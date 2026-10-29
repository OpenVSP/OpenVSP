//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//

// XSecSurfGeom.h: A Geom whose surfaces are skinned through XSecSurfs placed in space.
// J.R Gloudemans
//
//////////////////////////////////////////////////////////////////////

#if !defined(VSPXSECSURFGEOM__INCLUDED_)
#define VSPXSECSURFGEOM__INCLUDED_

#include "Geom.h"
#include "XSec.h"
#include "XSecSurf.h"

//==================================================================================================//
//==================================================================================================//
//==================================================================================================//
//==================================================================================================//

//==== XSecSurf Geom ====//
/*
    A Geom of any number of surfaces, each skinned through an XSecSurf of BareXSecs that are placed in space with
    SetLoc.  A custom component's script builds them, and a built in component's UpdateSurf.
*/
class XSecSurfGeom : public Geom
{
public:
    XSecSurfGeom( Vehicle* vehicle_ptr );
    virtual ~XSecSurfGeom();

    virtual void ClearXSecSurfs();

    virtual int GetNumXSecSurfs() const                   { return ( int )m_XSecSurfVec.size(); }
    virtual XSecSurf* GetXSecSurf( int index );

    //==== Skin XSecs ====//
    virtual void SkinXSecSurf( bool closed_flag );
    virtual void CloneSurf( int index, Matrix4d & mat );
    virtual void TransformSurf( int index, Matrix4d & mat );

    //==== Set VSP Surf Type ====//
    // The type every surface is skinned and flagged as.
    virtual void SetVspSurfType( int type )               { m_VspSurfType = type; }
    virtual int GetVspSurfType( int surf_id ) const       { return m_VspSurfType; }
    virtual void SetVspSurfCfdType( int type )            { m_VspSurfCfdType = type; }

    //==== Offset XSecs For Conformal =====//
    virtual void OffsetXSecs( double off );

protected:

    // A new surface, its pointer rather than its ID, of cross sections of type xsec_type.
    XSecSurf* NewXSecSurf( int xsec_type = vsp::XSEC_BARE );

    // A cross section of type type, appended to the surface and put at loc, with no continuity to its neighbors.
    BareXSec* AppendXSec( XSecSurf* xsec_surf, int type, const vec3d & loc );

    // A linear section at x through the points ( ys[i], zs[i] ), closed back to the first, its points spaced
    // evenly in its parameter.
    void AddPolySection( XSecSurf* surf, double x, const vector< double > & ys, const vector< double > & zs );

    // A cross section at loc that is a point, an ellipse w across and h high, a circle d across, or a rectangle w
    // across and h high with square corners.
    BareXSec* AppendPointXSec( XSecSurf* xsec_surf, const vec3d & loc );
    BareXSec* AppendEllipseXSec( XSecSurf* xsec_surf, const vec3d & loc, double w, double h );
    BareXSec* AppendCircleXSec( XSecSurf* xsec_surf, const vec3d & loc, double d );
    BareXSec* AppendRectXSec( XSecSurf* xsec_surf, const vec3d & loc, double w, double h );

    vector< XSecSurf* > m_XSecSurfVec;

    int m_VspSurfType;
    int m_VspSurfCfdType;

    virtual void UpdateFlags();

    bool m_ConformalFlag;
    double m_ConformalOffset;
    virtual void ApplyConformalOffset( double off );
};


#endif // !defined(VSPXSECSURFGEOM__INCLUDED_)
