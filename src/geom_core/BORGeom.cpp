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
#include "APIDefines.h"

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

    m_Diameter.Init( "Diameter", "Design", this, 1.0, 0.0, 1.0e12 );
    m_Diameter.SetDescript( "Diameter of BOR" );

    m_Length.Init( "Length", "Design", this, 1.0, 0.0, 1.0e12 );
    m_Length.SetDescript( "Length of BOR" );

    m_Xoff = 0.0;

    m_XSCurve = NULL;
    SetXSecCurveType( vsp::XS_CIRCLE );

}

//==== Destructor ====//
BORGeom::~BORGeom()
{

}

void BORGeom::UpdateSurf()
{
    if ( !m_XSCurve )
    {
        return;
    }

    m_XSCurve->Update(); // May not need to force Update here()

    VspCurve stringer;

    stringer = m_XSCurve->GetCurve();

    stringer.Scale( m_Length() );

    stringer.OffsetY( m_Diameter() );

    // Revolve to unit sphere
    m_MainSurfVec[0].CreateBodyRevolution( stringer );

    // Shift so nose is at origin
    m_MainSurfVec[0].OffsetX( m_Xoff );

    m_MainSurfVec[0].SetMagicVParm( false );
}

//==== Compute Rotation Center ====//
void BORGeom::ComputeCenter()
{
    m_Center.set_x( m_Length() * m_Origin() );
}

//==== Scale ====//
void BORGeom::Scale()
{
    double currentScale = m_Scale() / m_LastScale();

    m_Diameter *= currentScale;
    m_Length *= currentScale;

    m_LastScale = m_Scale();
}

void BORGeom::AddDefaultSources( double base_len )
{
}

void BORGeom::OffsetXSecs( double off )
{
}

void BORGeom::SetXSecCurveType( int type )
{
    double w = 1;
    double h = 1;

    if ( m_XSCurve )
    {
        if ( type == m_XSCurve->GetType() )
        {
            return;
        }

        w = m_XSCurve->GetWidth();
        h = m_XSCurve->GetHeight();

        delete m_XSCurve;
    }

    m_XSCurve = XSecSurf::CreateXSecCurve( type ) ;
    m_XSCurve->SetParentContainer( m_ID );

    m_XSCurve->SetWidthHeight( w, h );

    Update();
}
