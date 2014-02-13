/* File : vsp_common.i */
%include typemaps.i
%{
#include "Defines.h"
#include "APIDefines.h"
#include "APIErrorMgr.h"
#include "VSP_Geom_API.h"
#include "SWIGDefines.h"
#include "Vec3d.h"
%}

%include stl.i
/* instantiate the required template specializations */
namespace std {
    %template(IntVector)    vector<int>;
    %template(DoubleVector) vector<double>;
    %template(StringVector) vector<string>;
	%template(DoubleVecVec)  vector< vector<double> >;
	%template(Vec3dVec) vector<vec3d>;
}

%apply std::vector<vec3d> &INPUT { std::vector<vec3d> & pnt_vec };
/* Let's just grab the original header file here */
%include "APIDefines.h"
%include "APIErrorMgr.h"
%include "VSP_Geom_API.h"
%include "SWIGDefines.h"
%include "Vec3d.h"

