//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//

// SVGUtil.h
// Justin Gravett
//
//////////////////////////////////////////////////////////////////////

#if !defined(VSP_SVGUtil_h)
#define VSP_SVGUtil_h

#include "Vec3d.h"
#include "APIDefines.h"
#include "BndBox.h"
#include "XmlUtil.h"

#include <vector>
#include <string>
#include <algorithm>

using std::vector;
using std::string;
using namespace XmlUtil;

void WriteSVGHeader( xmlNodePtr root, const BndBox &svgbox );
void WriteSVGScaleBar( xmlNodePtr root, const int &View, const BndBox &svgbox, const int &LenUnit, const double &scale );
void WriteSVGPolylines2D( xmlNodePtr root, const vector < vector < vec3d > > &allflines, const BndBox &svgbox );
xmlNodePtr AddDefaultScalBarProps( xmlNodePtr & node, const double &convert_scale );

#endif
