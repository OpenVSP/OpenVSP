#if !defined(VSP_DXFUtil_h)
#define VSP_DXFUtil_h

#include "Vec3d.h"
#include "APIDefines.h"
#include "BndBox.h"

#include <vector>
#include <string>
#include <algorithm>

using std::vector;
using std::string;

void WriteDXFHeader( FILE* dxf_file, int LenUnitChoice );
void DXFManipulate( vector < vector < vec3d > > &allflines, const BndBox &dxfbox, int view, int ang );
void DXFShift( vector < vector < vec3d > > &allflines, vec3d shiftvec, int shift, int ang1, int ang2 );
void WriteDXFPolylines3D( FILE* dxf_file, const vector < vector < vec3d > > &allflines, string layer );
void WriteDXFPolylines2D( FILE* dxf_file, const vector < vector < vec3d > > &allflines, string layer );
void WriteDXFClose( FILE* dxf_file );

#endif
