/* File : vsp_common.i */

/* Enable automatic docstrings for all wrapped functions. */
%feature("autodoc", 1);

/* Tell SWIG how to handle OpenVSP's language-specific Doxygen sections:
   - \forcpponly...\endforcpponly  : C++-only block; skip entirely.
   - \beginPythonOnly...\endPythonOnly : Python block; parse into docstring. */
%feature("doxygen:ignore:forcpponly", range="end");
%feature("doxygen:ignore:beginPythonOnly", range="end:endPythonOnly", contents="parse");

%include typemaps.i
%{

#include "APIDefines.h"
#include "APIErrorMgr.h"
#include "VSP_Geom_API.h"
#include "SWIGDefines.h"
#include "Vec3d.h"
#include "Vec2d.h"
#include "Matrix4d.h"
%}

%include stl.i
/* instantiate the required template specializations */
namespace std {
    %template(IntVector)    vector<int>;
    %template(DoubleVector) vector<double>;
    %template(StringVector) vector<string>;
    %template(BoolVector) vector<bool>;
    %template(StringVecVec) vector< vector<string> >;
    %template(IntVecVec) vector< vector<int> >;
    %template(DoubleVecVec)  vector< vector<double> >;
    %template(Vec3dVec) vector<vec3d>;
    %template(Vec2dVec) vector<vec2d>;
    %template(Matrix4dVec) vector<Matrix4d>;
}

/* Vec2d.h has a handful of functions that hand their answer back through a reference.  Without a
   typemap they are wrapped as required *input* arguments of a type Python has no way to build, so
   seg_seg_intersect, bi_lin_interp and inverse_bi_lin_interp were all reachable and all uncallable.
   typemaps.i only covers the primitives, so vec2d needs one of its own. */
%typemap(in, numinputs=0) vec2d &OUTPUT ( vec2d temp ) {
    $1 = &temp;
}
%typemap(argout) vec2d &OUTPUT {
    %append_output( SWIG_NewPointerObj( new vec2d( *$1 ), SWIGTYPE_p_vec2d, SWIG_POINTER_OWN ) );
}

%apply std::vector<vec3d> &INPUT { std::vector<vec3d> & pnt_vec };
%apply ( double& OUTPUT ) { double & xmin_out, double & ymin_out, double & zmin_out, double & xlen_out, double & ylen_out, double & zlen_out };
%apply ( double& OUTPUT ) { double& u_out, double& w_out, double &k1_out, double &k2_out, double &ka_out, double &kg_out };
%apply ( double& OUTPUT ) { double& r_out, double& s_out,  double& t_out };
%apply ( double& OUTPUT ) { double& l_out, double& m_out,  double& n_out };
%apply ( double& OUTPUT ) { double& eta_out };
%apply ( double& OUTPUT ) { double& temp, double& pres, double& pres_ratio,  double& rho_ratio };
%apply ( int& OUTPUT ) { int &surf_indx_out };
%apply ( std::vector<double> &OUTPUT ) { std::vector < double > &k1_out_vec };
%apply ( std::vector<double> &OUTPUT ) { std::vector < double > &k2_out_vec };
%apply ( std::vector<double> &OUTPUT ) { std::vector < double > &ka_out_vec };
%apply ( std::vector<double> &OUTPUT ) { std::vector < double > &kg_out_vec };
%apply ( std::vector<double> &OUTPUT ) { std::vector < double > &u_out_vec };
%apply ( std::vector<double> &OUTPUT ) { std::vector < double > &w_out_vec };
%apply ( std::vector<double> &OUTPUT ) { std::vector < double > &d_out_vec };
%apply ( std::vector<double> &OUTPUT ) { std::vector < double > &s_out_vec };
%apply ( std::vector<double> &OUTPUT ) { std::vector < double > &r_out_vec };
%apply ( std::vector<double> &OUTPUT ) { std::vector < double > &t_out_vec };
%apply ( std::vector<double> &OUTPUT ) { std::vector < double > &l_out_vec };
%apply ( std::vector<double> &OUTPUT ) { std::vector < double > &m_out_vec };
%apply ( std::vector<double> &OUTPUT ) { std::vector < double > &n_out_vec };

/* The arithmetic on vec3d and vec2d is written as free operator functions, which SWIG cannot wrap:
   "operator+" is not a valid Python identifier, so it is skipped with a warning and the operators
   simply do not reach Python.  They are documented in Vec3d.h with examples that do "vec3d c = a +
   b", so the documentation promised arithmetic that raised TypeError, and "a == b" quietly compared
   identity instead of value -- two equal points came back unequal.  Nothing caught it because the
   examples in these headers were never generated into the test suite.

   Bind them here as the corresponding Python special methods.  %pythonmaybecall makes a comparison
   against an unrelated type return NotImplemented rather than raise, so "v == None" still works. */
%extend vec3d {
    vec3d __add__( const vec3d &b )      { return ( *$self ) + b; }
    vec3d __sub__( const vec3d &b )      { return ( *$self ) - b; }
    vec3d __mul__( double b )            { return ( *$self ) * b; }
    vec3d __mul__( const vec3d &b )      { return ( *$self ) * b; }
    vec3d __rmul__( double b )           { return ( *$self ) * b; }
    vec3d __truediv__( double b )        { return ( *$self ) / b; }
    vec3d __neg__()                      { return -( *$self ); }

    %pythonmaybecall __eq__;
    bool __eq__( const vec3d &b )        { return ( *$self ) == b; }
    %pythonmaybecall __ne__;
    bool __ne__( const vec3d &b )        { return ( *$self ) != b; }

    /* vec3d::operator[] does not range check, so the bounds test has to live somewhere.  It is done
       on the Python side because a C++ throw out of an %extend method does not become a Python
       exception -- %exception does not attach to these, so the wrapper returns normally with the
       error still set and the interpreter reports "returned a result with an exception set".
       Raising IndexError properly is also what lets list( v ) and tuple( v ) terminate. */
    double _getitem( int i )             { return ( *$self )[i]; }
    void _setitem( int i, double val )   { ( *$self )[i] = val; }
    int __len__()                        { return 3; }

    %pythoncode %{
        def __getitem__( self, i ):
            if i < 0:
                i += 3
            if i < 0 or i > 2:
                raise IndexError( "vec3d index out of range" )
            return self._getitem( i )

        def __setitem__( self, i, val ):
            if i < 0:
                i += 3
            if i < 0 or i > 2:
                raise IndexError( "vec3d index out of range" )
            self._setitem( i, val )
    %}

    std::string __repr__() {
        char buf[128];
        snprintf( buf, sizeof( buf ), "vec3d( %g, %g, %g )", $self->x(), $self->y(), $self->z() );
        return std::string( buf );
    }

    /* Defining __eq__ drops the inherited __hash__ in Python 3.  Put the identity hash back so a
       vec3d can still go in a set or be used as a dict key, as it could before. */
    %pythoncode %{
        __hash__ = object.__hash__
    %}
}

%extend vec2d {
    vec2d __add__( const vec2d &b )      { return ( *$self ) + b; }
    vec2d __sub__( const vec2d &b )      { return ( *$self ) - b; }
    vec2d __mul__( double b )            { return ( *$self ) * b; }
    vec2d __mul__( const vec2d &b )      { return ( *$self ) * b; }
    vec2d __rmul__( double b )           { return ( *$self ) * b; }
    vec2d __truediv__( double b )        { return ( *$self ) / b; }

    /* vec2d::operator[] does not range check, so the bounds test has to live somewhere.  It is done
       on the Python side because a C++ throw out of an %extend method does not become a Python
       exception -- %exception does not attach to these, so the wrapper returns normally with the
       error still set and the interpreter reports "returned a result with an exception set".
       Raising IndexError properly is also what lets list( v ) and tuple( v ) terminate. */
    double _getitem( int i )             { return ( *$self )[i]; }
    void _setitem( int i, double val )   { ( *$self )[i] = val; }
    int __len__()                        { return 2; }

    %pythoncode %{
        def __getitem__( self, i ):
            if i < 0:
                i += 2
            if i < 0 or i > 1:
                raise IndexError( "vec2d index out of range" )
            return self._getitem( i )

        def __setitem__( self, i, val ):
            if i < 0:
                i += 2
            if i < 0 or i > 1:
                raise IndexError( "vec2d index out of range" )
            self._setitem( i, val )
    %}

    std::string __repr__() {
        char buf[128];
        snprintf( buf, sizeof( buf ), "vec2d( %g, %g )", $self->x(), $self->y() );
        return std::string( buf );
    }
}

/* Let's just grab the original header file here */
%include "APIDefines.h"
%include "APIErrorMgr.h"
%include "VSP_Geom_API.h"
%include "SWIGDefines.h"
%include "Vec3d.h"

/* Not expressible in Python: data() hands back a bare pointer, transform() wants a 3x3 C array, and
   the two clipping helpers report through a C array or through arguments they read as well as
   write.  Leaving them wrapped puts entries in the documentation that cannot be called. */
%ignore vec2d::data;
%ignore vec2d::transform;
%ignore encode( double, double, double, double, const vec2d&, int[4] );
%ignore clip_seg_rect;

%apply vec2d &OUTPUT { vec2d &int_pnt, vec2d &p_out };
%apply ( double& OUTPUT ) { double &t1, double &t2, double &s, double &t, double &s2 };
%include "Vec2d.h"
%clear vec2d &int_pnt, vec2d &p_out;
%clear double &t1, double &t2, double &s, double &t, double &s2;

%include "Matrix4d.h"
