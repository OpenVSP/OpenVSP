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
#include "VspCurve.h"

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

    m_LECluster.Init( "LECluster", m_Name, this, 0.25, 1e-4, 10.0 );
    m_LECluster.SetDescript( "LE Tess Cluster Control" );

    m_TECluster.Init( "TECluster", m_Name, this, 0.25, 1e-4, 10.0 );
    m_TECluster.SetDescript( "TE Tess Cluster Control" );

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

    stringer.OffsetY( m_Diameter() );

    // Revolve to unit sphere
    m_MainSurfVec[0].CreateBodyRevolution( stringer, true );

    m_MainSurfVec[0].SwapUWDirections();
    m_MainSurfVec[0].ReverseWDirection();

    // Shift so nose is at origin
    m_MainSurfVec[0].OffsetX( m_Xoff );

    m_MainSurfVec[0].SetSurfType( vsp::WING_SURF );
    m_MainSurfVec[0].SetMagicVParm( true );

    m_MainSurfVec[0].SetClustering( m_LECluster(), m_TECluster() );

}

//==== Compute Rotation Center ====//
void BORGeom::ComputeCenter()
{
    m_Center.set_x( m_XSCurve->GetWidth() * m_Origin() );
}

//==== Scale ====//
void BORGeom::Scale()
{
    double currentScale = m_Scale() / m_LastScale();

    m_Diameter *= currentScale;

    m_XSCurve->SetScale( currentScale );

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

    XSecCurve *oldXSCurve = NULL;

    if ( m_XSCurve )
    {
        if ( type == m_XSCurve->GetType() )
        {
            return;
        }

        w = m_XSCurve->GetWidth();
        h = m_XSCurve->GetHeight();

        oldXSCurve = m_XSCurve;
    }

    m_XSCurve = XSecSurf::CreateXSecCurve( type ) ;
    m_XSCurve->SetParentContainer( m_ID );

    if ( oldXSCurve )
    {
        m_XSCurve->CopyFrom( oldXSCurve );
        delete oldXSCurve;
    }

    m_XSCurve->SetWidthHeight( w, h );

    Update();
}

void BORGeom::UpdateDrawObj()
{
    Geom::UpdateDrawObj();


    double w = m_XSCurve->GetWidth();

    Matrix4d mat;
    XSecSurf::GetBasicTransformation( vsp::Z_DIR, vsp::X_DIR, vsp::XS_SHIFT_MID, false, 1.0, mat );

    mat.scale( 1.0/w );
    VspCurve crv = m_XSCurve->GetCurve();
    crv.Transform( mat );

    vector< vec3d > pts;
    crv.TessAdapt( pts, 1e-4, 10 );

    m_CurrentXSecDrawObj.m_PntVec = pts;
    m_CurrentXSecDrawObj.m_LineWidth = 3;
    m_CurrentXSecDrawObj.m_LineColor = vec3d( 0.0, 0.0, 0.0 );
    m_CurrentXSecDrawObj.m_Type = DrawObj::VSP_LINES;
    m_CurrentXSecDrawObj.m_GeomChanged = true;

}

void BORGeom::LoadDrawObjs( vector< DrawObj* > & draw_obj_vec )
{
    Geom::LoadDrawObjs( draw_obj_vec );

    if ( m_Vehicle->IsGeomActive( m_ID ) && m_GuiDraw.GetDisplayType() == GeomGuiDraw::DISPLAY_BEZIER )
    {
        m_CurrentXSecDrawObj.m_Screen = DrawObj::VSP_XSEC_SCREEN;
        m_CurrentXSecDrawObj.m_GeomID = XSECHEADER + m_ID + "CURRENT";
        draw_obj_vec.push_back( &m_CurrentXSecDrawObj );
    }
}
