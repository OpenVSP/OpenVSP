/* File : vsp_common.i */
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
    %template(Matrix4dVec) vector<Matrix4d>;
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

/* Let's just grab the original header file here */
%include "APIDefines.h"
%include "APIErrorMgr.h"
%include "VSP_Geom_API.h"
%include "SWIGDefines.h"
%include "Vec3d.h"
%include "Vec2d.h"
%include "Matrix4d.h"
