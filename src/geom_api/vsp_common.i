/* File : vsp_common.i */

/* Enable automatic docstrings for all wrapped functions. */
%feature("autodoc", 1);

/* Tell SWIG how to handle OpenVSP's language-specific Doxygen sections:
   - \forcpponly...\endforcpponly  : C++-only block; skip entirely.
   - \beginPythonOnly...\endPythonOnly : Python block; parse into docstring. */
%feature("doxygen:ignore:forcpponly", range="end");
%feature("doxygen:ignore:beginPythonOnly", range="end:endPythonOnly", contents="parse");

/* --- Warnings SWIG raises that are answered elsewhere, or are noise ------------------------------

   566: "Unexpected iterator value in DoxygenParser::parse", ~1800 of them.  SWIG's doxygen parser
   trips over the ends of the \forcpponly and \beginPythonOnly ranges that the features above ask
   it to skip.  The docstrings come out complete and correct -- description, the right language's
   example, params and return -- so this is the parser complaining about its own bookkeeping.

   401: ErrorMgrSingleton derives from MessageBase, which lives in MessageMgr.h and is not wrapped.
   The base contributes nothing to the API, so there is nothing to fix, only to say so.

   315: std::stack, from the "using std::stack" at the top of APIErrorMgr.h.  It is there for the
   private m_ErrorStack member, and nothing is generated for that.

   These three are suppressed with the nowarn pragma rather than %warnfilter.  A %warnfilter with no
   target applies to the next declaration, not to everything, so writing several in a row simply
   consumes them; the pragma is the global form.  509 does attach to a name, so it stays targeted.
   401 and 315 are each the only instance of their number in the whole build.

   509: poly_area is overloaded on vector<vec3d> and vector<vec2d>.  Those are both wrapped pointer
   types, so they tie on typecheck precedence and SWIG reports the second as shadowed.  The runtime
   dispatcher tries each in turn and gets it right -- both spellings are covered by the test suite. */
#pragma SWIG nowarn=566,401,315
%warnfilter(509) poly_area;

/* The operators are bound below as Python special methods with %extend, so the C++ declarations
   have nothing left to contribute.  Ignoring them is what silences warnings 503, 389 and 362:
   "operator+" is not a valid Python identifier, operator[] wants %extend -- which is exactly what it
   gets -- and there is no assignment operator to overload in Python at all. */
%ignore operator+;
%ignore operator-;
%ignore operator*;
%ignore operator/;
%ignore operator==;
%ignore operator!=;
%ignore vec3d::operator[];
%ignore vec2d::operator[];
%ignore vec3d::operator=;
%ignore vec2d::operator=;

/* cstdout is there to be read and handed to a C++ call, never assigned; without this SWIG generates
   a setter for it and warns that setting a pointer variable may leak memory (454). */
%immutable cstdout;

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
    // xformmat and xformnormmat take a grid of points; without this they cannot be called at all.
    %template(Vec3dVecVec) vector< vector<vec3d> >;
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

/* SWIG's Python keyword list still carries Python 2's "print" and renames these methods to _print.
   In Python 3 print is an ordinary name, so keep the name that Vec3d.h and Matrix4d.h document.
   %rename cannot do it -- the keyword rename wins over it -- so clear the keyword rule for just
   these two methods. */
%namewarn( "" ) vec3d::print;
%namewarn( "" ) Matrix4d::print;

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

/* getRotationAxis and toQuat report through double references.  Their parameters carry _out names
   so the typemap cannot catch an input by accident -- Matrix4d::rotate takes "const double &angle",
   and %apply matches on the name whatever the constness, which silently turned that input into an
   output and broke the wrapper.  Scoped to this include and cleared after, for the same reason. */
%apply ( double& OUTPUT ) { double &angle_out, double &qw_out, double &qx_out, double &qy_out, double &qz_out, double &tx_out, double &ty_out, double &tz_out };
%include "Matrix4d.h"
%clear double &angle_out, double &qw_out, double &qx_out, double &qy_out, double &qz_out, double &tx_out, double &ty_out, double &tz_out;
