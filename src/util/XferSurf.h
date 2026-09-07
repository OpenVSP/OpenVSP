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

#include <algorithm>
#include <utility>
#include <string>
#include <vector>
#include <APIDefines.h>
#include <Vec3d.h>
using std::string;
using std::vector;
using std::pair;
using std::make_pair;

// One piece of a patch that came from one piece of the Geom's surface.
//
// A patch handed to the mesher is not always a plain sub-rectangle of the surface it came
// from.  The trailing edge patch is built by joining the strip at one end of w to the strip
// at the other, and an end cap by reversing one half and joining it to the other, so a
// single patch can cover two disjoint pieces of the original, and can cover them backwards.
//
// Every operation used to build them -- split, reverse, translate, join -- moves the
// parameters by an affine map of unit slope, so each piece is described by where it came
// from and which way round it now runs:
//
//     u_orig = m_USign * u_patch + m_UOff        m_USign is +1 or -1
//     w_orig = m_WSign * w_patch + m_WOff
//
// The extent is recorded in the ORIGINAL parameters, because that is what the things which
// care -- subsurfaces, tessellation lines, fixed points -- are written in.  The extent in
// the patch's own parameters is not stored because the map already gives it.
class UWRegion
{
public:

    UWRegion()
    {
        m_UMin = 0.0;
        m_UMax = 0.0;
        m_WMin = 0.0;
        m_WMax = 0.0;
        m_USign = 1.0;
        m_UOff = 0.0;
        m_WSign = 1.0;
        m_WOff = 0.0;
    }

    // Extent of this piece on the original surface.
    double m_UMin, m_UMax;
    double m_WMin, m_WMax;

    // Patch parameters to original parameters.
    double m_USign, m_UOff;
    double m_WSign, m_WOff;

    double ToOrigU( double u ) const  { return m_USign * u + m_UOff; }
    double ToOrigW( double w ) const  { return m_WSign * w + m_WOff; }

    double ToPatchU( double uo ) const  { return ( uo - m_UOff ) / m_USign; }
    double ToPatchW( double wo ) const  { return ( wo - m_WOff ) / m_WSign; }

    // Extent in the patch's own parameters, worked back through the map.  The sign flip
    // swaps which end is which, so they are put back in order.
    void PatchExtentU( double &lo, double &hi ) const
    {
        lo = ToPatchU( m_UMin );
        hi = ToPatchU( m_UMax );
        if ( lo > hi )
        {
            std::swap( lo, hi );
        }
    }

    void PatchExtentW( double &lo, double &hi ) const
    {
        lo = ToPatchW( m_WMin );
        hi = ToPatchW( m_WMax );
        if ( lo > hi )
        {
            std::swap( lo, hi );
        }
    }
};

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

    // The parent Geom's tessellation lines, in the parameter space of m_Surface and
    // clipped to it.  A surface is split along its feature lines on the way here, so
    // each piece carries only the lines that fall inside it.
    vector < double > m_UTess;
    vector < double > m_WTess;

    // Where this patch came from on the Geom's surface.  One entry for a patch that is a
    // plain piece of it, more than one for a patch built by joining pieces together.
    vector < UWRegion > m_UWRegions;

    // The lines where the surface this patch came from was joined to itself, as pairs of end
    // points in the GEOM's parameters -- ( u, w, 0 ), which is how a subsurface segment is
    // written.  Recorded at the join by VspSurf::FetchXFerSurf, which knows where it is:
    // join_u( a, b ) puts the seam at a's umax, join_v( a, b ) at a's vmax.
    //
    // Only ONE of each seam's two halves is here.  The two pieces meet along it, so each has
    // its own name for it and both come back to the same line of the patch; recording both
    // would build the same chain twice.
    //
    // The same list goes to every patch of the surface, and the clipping every subsurface
    // already gets decides which of them each patch keeps.
    vector < pair < vec3d, vec3d > > m_JoinLines;

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
