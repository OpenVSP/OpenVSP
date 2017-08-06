//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//
//
//////////////////////////////////////////////////////////////////////

#include "BORGeom.h"
#include "ParmMgr.h"
#include "LinkMgr.h"
#include "Vehicle.h"

typedef eli::geom::curve::piecewise_ellipse_creator<double, 3, curve_tolerance_type> piecewise_ellipse_creator;

//==== Constructor ====//
BORGeom::BORGeom( Vehicle* vehicle_ptr ) : Geom( vehicle_ptr )
{
    m_Name = "BORGeom";
    m_Type.m_Name = "BodyOfRevolution";
    m_Type.m_Type = BOR_GEOM_TYPE;

    //==== Init Parms ====//
    m_TessU = 5;
    m_TessW = 8;

    m_Aradius.Init( "A_Radius", "Design", this, 1.0, 0.0, 1.0e12 );
    m_Aradius.SetDescript( "A (x) radius of ellipsoid" );

    m_Bradius.Init( "B_Radius", "Design", this, 1.0, 0.0, 1.0e12 );
    m_Bradius.SetDescript( "B (y) radius of ellipsoid" );

    m_Cradius.Init( "C_Radius", "Design", this, 1.0, 0.0, 1.0e12 );
    m_Cradius.SetDescript( "C (z) radius of ellipsoid" );

    m_Xoff = 0.0;

}

//==== Destructor ====//
BORGeom::~BORGeom()
{

}

void BORGeom::UpdateSurf()
{
    // Build unit circle
    piecewise_curve_type c, c1, c2;
    piecewise_ellipse_creator pec( 4 );
    curve_point_type origin, normal;

    origin << 0, 0, 0;
    normal << 0, 1, 0;

    pec.set_origin( origin );
    pec.set_x_axis_radius( 1.0 );
    pec.set_y_axis_radius( 1.0 );

    // set circle params, make sure that entire curve goes from 0 to 4
    pec.set_t0( 0 );
    pec.set_segment_dt( 1, 0 );
    pec.set_segment_dt( 1, 1 );
    pec.set_segment_dt( 1, 2 );
    pec.set_segment_dt( 1, 3 );

    pec.create( c );

    c.split( c1, c2, 2.0 );
    c2.scale_y( -1.0 );

    VspCurve stringer;
    stringer.SetCurve( c2 );

    // Revolve to unit sphere
    m_MainSurfVec[0].CreateBodyRevolution( stringer );

    // Scale to ellipsoid
    m_MainSurfVec[0].ScaleX( m_Aradius() );
    m_MainSurfVec[0].ScaleY( m_Bradius() );
    m_MainSurfVec[0].ScaleZ( m_Cradius() );

    // Shift so nose is at origin
    m_MainSurfVec[0].OffsetX( m_Aradius() + m_Xoff );

    m_MainSurfVec[0].SetMagicVParm( false );
}

//==== Compute Rotation Center ====//
void BORGeom::ComputeCenter()
{
    m_Center.set_x( m_Aradius() * m_Origin() * 2.0 );
}

//==== Scale ====//
void BORGeom::Scale()
{
    double currentScale = m_Scale() / m_LastScale();

    m_Aradius *= currentScale;
    m_Bradius *= currentScale;
    m_Cradius *= currentScale;

    m_LastScale = m_Scale();
}

void BORGeom::AddDefaultSources( double base_len )
{
}

void BORGeom::OffsetXSecs( double off )
{
    double a = m_Aradius();
    double b = m_Bradius();
    double c = m_Cradius();

    double aoff = a - off;
    double boff = b - off;
    double coff = c - off;

    if ( aoff < 0.0 )
    {
        aoff = 0.0;
    }

    if ( boff < 0.0 )
    {
        boff = 0.0;
    }

    if ( coff < 0.0 )
    {
        coff = 0.0;
    }

    m_Aradius = aoff;
    m_Bradius = boff;
    m_Cradius = coff;

    m_Xoff = off;
}
