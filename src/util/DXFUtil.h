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
void DXFManipulate( vector < vector < vec3d > > &inlines, BndBox dxfbox, int Parm1 );
vector < vector < vec3d > > DXFRot( vector < vector < vec3d > > allflines, int ang );
vector < vector < vec3d > > DXFShift( vector < vector < vec3d > > allflines, vec3d shiftvec, int Parm1, int Parm2, int Parm3 );
void WriteDXFPolylines3D( FILE* dxf_file, const vector < vector < vec3d > > &allflines, string layer );
void WriteDXFPolylines2D( FILE* dxf_file, const vector < vector < vec3d > > &allflines, string layer );
void WriteDXFClose( FILE* dxf_file );

#endif
