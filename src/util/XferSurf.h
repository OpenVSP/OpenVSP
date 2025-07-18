//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//

#ifndef VSP_XFER_SURF_H
#define VSP_XFER_SURF_H

#include "eli/code_eli.hpp"

#include "eli/geom/surface/bezier.hpp"
#include "eli/geom/surface/piecewise.hpp"

typedef eli::geom::surface::bezier<double, 3> surface_patch_type;
typedef eli::geom::surface::piecewise<eli::geom::surface::bezier, double, 3> piecewise_surface_type;

#include <string>
#include <APIDefines.h>
#include <Vec3d.h>
using std::string;

class XferSurf
{
public:

    XferSurf()
    {
        m_FlipNormal = false;
        m_SplitNum = 0;
        m_CompIndx = 0;
        m_SurfIndx = 0;
        m_FeaPartSurfNum = -1;
        m_FeaSymmIndex = -1;
        m_SurfType = vsp::NORMAL_SURF;
        m_SurfCfdType = vsp::CFD_NORMAL;
        m_FeaOrientationType = vsp::FEA_ORIENT_OML_U;
        m_FeaOrientation = vec3d();
        m_ThickSurf = true;
        m_PlateNum = -1;
        m_CopyIndex = -1;
        m_PlanarUWAspect = -1;
    };

    ~XferSurf()
    {
    };

    bool m_FlipNormal;

    string m_GeomID;
    string m_Name;
    int m_SplitNum;
    int m_CompIndx;
    int m_SurfIndx;
    int m_FeaPartSurfNum;
    int m_FeaSymmIndex;
    int m_SurfType;
    int m_SurfCfdType;
    int m_FeaOrientationType;
    vec3d m_FeaOrientation;
    bool m_ThickSurf;
    int m_PlateNum;
    int m_CopyIndex;
    double m_PlanarUWAspect;

    piecewise_surface_type m_Surface;
};

#endif
