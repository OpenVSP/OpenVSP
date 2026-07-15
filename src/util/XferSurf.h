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

    // Destructor and copy/move operations are intentionally left implicit so vector<XferSurf>
    // can move elements instead of deep-copying them.

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

#include <type_traits>
// Guard the rule-of-zero cleanup: a user-declared destructor or copy operation would silently
// suppress the implicit move operations that vector<XferSurf> relies on to avoid deep copies.
// MSVC's std::map move operations are not noexcept (sentinel node allocation), so the
// nothrow guarantee cannot hold for types holding Code-Eli piecewise members there --
// vector reallocation copies these types on MSVC and moves them elsewhere.
#if defined(_MSC_VER)
static_assert( std::is_move_constructible< XferSurf >::value, "XferSurf must be move constructible" );
static_assert( std::is_move_assignable< XferSurf >::value, "XferSurf must be move assignable" );
#else
static_assert( std::is_nothrow_move_constructible< XferSurf >::value, "XferSurf must be nothrow move constructible" );
static_assert( std::is_nothrow_move_assignable< XferSurf >::value, "XferSurf must be nothrow move assignable" );
#endif

#endif
