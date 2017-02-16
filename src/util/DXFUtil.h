//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//

// DXFUtil.h
// Justin Gravett
//
//////////////////////////////////////////////////////////////////////

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
void WriteDXFPolylines3D( FILE* dxf_file, const vector < vector < vec3d > > &allflines, string layer );
void WriteDXFPolylines2D( FILE* dxf_file, const vector < vector < vec3d > > &allflines, string layer );
void FeatureLinesManipulate( vector < vector < vec3d > > &allflines, int view, int ang, vec3d shiftvec );
void FeatureLinesShift( vector < vector < vec3d > > &allflines, vec3d shiftvec, int shift, int ang1, int ang2 );
void WriteDXFClose( FILE* dxf_file );

#endif
