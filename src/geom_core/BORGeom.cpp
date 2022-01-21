//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//
//
//////////////////////////////////////////////////////////////////////

#include "BORGeom.h"
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

    m_Diameter.Init( "Diameter", "Design", this, 2.0, 0.0, 1.0e12 );
    m_Diameter.SetDescript( "Diameter of BOR" );

    m_Angle.Init( "Angle", "Design", this, 0.0, -180.0, 180.0 );
    m_Angle.SetDescript( "Section angle");

    m_Mode.Init( "Mode", "Design", this, vsp::BOR_FLOWTHROUGH, vsp::BOR_FLOWTHROUGH, vsp::BOR_NUM_MODES-1 );
    m_Mode.SetDescript( "Mode control, flowthrough, upper, or lower surface" );

    m_LECluster.Init( "LECluster", "Design", this, 0.25, 1e-4, 10.0 );
    m_LECluster.SetDescript( "LE Tess Cluster Control" );

    m_TECluster.Init( "TECluster", "Design", this, 0.25, 1e-4, 10.0 );
    m_TECluster.SetDescript( "TE Tess Cluster Control" );

    m_A0.Init( "A0", "Design", this, 1.0, -1.0e12, 1.0e12 );
    m_A0.SetDescript( "Area of BOR leading edge" );

    m_Ae.Init( "Ae", "Design", this, 1.0, -1.0e12, 1.0e12 );
    m_Ae.SetDescript( "Area of BOR trailing edge" );

    m_Amin.Init( "Amin", "Design", this, 1.0, -1.0e12, 1.0e12 );
    m_Amin.SetDescript( "Minimum area of BOR" );

    m_AminW.Init( "AminW", "Design", this, 0.5, 0.0, 1.0 );
    m_AminW.SetDescript( "W parameter for minimum area" );

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

    // Store m_FoilSurf for later use.
    vector< VspCurve > crv_vec;
    crv_vec.resize( 2 );

    crv_vec[0] = stringer;
    crv_vec[0].Scale( 1.0 / m_XSCurve->GetWidth() );
    crv_vec[1] = crv_vec[0];

    m_FoilSurf = VspSurf();
    m_FoilSurf.SkinC0( crv_vec, false );

    if ( m_Mode() == vsp::BOR_FLOWTHROUGH )
    {
        stringer.RotateZ(m_Angle() * PI / 180.0);
        stringer.OffsetY( m_Diameter() * 0.5 ); // Offset is a radius

        double r = stringer.CompPnt01( 0.0 ).y();
        m_Ae.Set( PI * r * r );
        m_Ae.Deactivate();

        double tmin;
        r = stringer.FindMinimumDimension( tmin, vsp::Y_DIR );
        m_AminW.Set( tmin / 4.0 );
        m_AminW.Deactivate();
        m_Amin.Set( PI * r * r );
        m_Amin.Deactivate();

        r = stringer.CompPnt01( 0.5 ).y();
        m_A0.Set( PI * r * r );
        m_A0.Deactivate();
    }
    else if ( m_Mode() == vsp::BOR_LOWER || m_Mode() == vsp::BOR_UPPER )
    {
        piecewise_curve_type c, c1, c2;
        double tmin, tmax, tsplit;

        c = stringer.GetCurve();
        tmin = c.get_parameter_min();
        tmax = c.get_parameter_max();
        tsplit = ( tmin + tmax ) * 0.5;
        c.split( c1, c2, tsplit );

        if ( m_Mode() == vsp::BOR_LOWER )
        {
            c1.set_t0( tmin );
            c1.scale_y( -1.0 );
            stringer.SetCurve( c1 );
        }
        else if ( m_Mode() == vsp::BOR_UPPER )
        {
            c2.reverse();
            c2.set_t0( tmin );
            stringer.SetCurve( c2 );
        }

        m_Ae.Set( 0.0 );
        m_Ae.Deactivate();

        m_A0.Set( 0.0 );
        m_A0.Deactivate();

        m_Amin.Set( 0.0 );
        m_Amin.Deactivate();

        m_AminW.Set( 0.0 );
        m_AminW.Deactivate();
    }

    // Revolve to unit sphere
    m_MainSurfVec[0].CreateBodyRevolution( stringer, true );

    m_MainSurfVec[0].SwapUWDirections();

    if ( m_Mode() == vsp::BOR_FLOWTHROUGH )
    {
        m_MainSurfVec[0].SetHalfBOR( false );
        m_MainSurfVec[0].FlipNormal();
    }
    else
    {
        m_MainSurfVec[0].SetHalfBOR( true );
    }

    // Shift so nose is at origin
    m_MainSurfVec[0].OffsetX( m_Xoff );

    m_MainSurfVec[0].SetSurfType( vsp::WING_SURF );
    m_MainSurfVec[0].SetMagicVParm( true );
    m_MainSurfVec[0].SetFoilSurf( &m_FoilSurf );
}

void BORGeom::UpdatePreTess()
{
    // Update clustering before symmetry is applied for m_SurfVec
    m_FoilSurf.SetClustering( m_LECluster(), m_TECluster() );
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
    m_XSCurve->OffsetCurve( off );

    m_Xoff = off;
}

void BORGeom::SetXSecCurveType( int type )
{
    double w = 1;
    double h = 1;

    XSecCurve *oldXSCurve = m_XSCurve;

    if ( m_XSCurve )
    {
        if ( type == m_XSCurve->GetType() )
        {
            return;
        }

        w = m_XSCurve->GetWidth();
        h = m_XSCurve->GetHeight();
    }

    m_XSCurve = XSecSurf::CreateXSecCurve( type ) ;

    if ( m_XSCurve )
    {
        m_XSCurve->SetParentContainer( m_ID );

        if ( oldXSCurve )
        {
            m_XSCurve->CopyFrom( oldXSCurve );
            delete oldXSCurve;
        }

        m_XSCurve->SetWidthHeight( w, h );
    }
    else  // Failed to create new curve, revert to saved.
    {
        m_XSCurve = oldXSCurve;
    }

    Update();
}

int BORGeom::GetXSecCurveType()
{
    return m_XSCurve->GetType();
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

    if ( m_Vehicle->IsGeomActive( m_ID ) && m_GuiDraw.GetDisplayType() == vsp::DISPLAY_TYPE::DISPLAY_BEZIER )
    {
        m_CurrentXSecDrawObj.m_Screen = DrawObj::VSP_XSEC_SCREEN;
        m_CurrentXSecDrawObj.m_GeomID = XSECHEADER + m_ID + "CURRENT";
        draw_obj_vec.push_back( &m_CurrentXSecDrawObj );
    }
}

//==== Encode XML ====//
xmlNodePtr BORGeom::EncodeXml(  xmlNodePtr & node  )
{
    Geom::EncodeXml( node );

    xmlNodePtr xscrv_node = xmlNewChild( node, NULL, BAD_CAST "XSecCurve", NULL );
    if ( xscrv_node )
    {
        m_XSCurve->EncodeXml( xscrv_node );
    }

    return xscrv_node;
}

//==== Decode XML ====//
xmlNodePtr BORGeom::DecodeXml(  xmlNodePtr & node  )
{
    Geom::DecodeXml( node );

    xmlNodePtr xscrv_node = XmlUtil::GetNode( node, "XSecCurve", 0 );
    if ( xscrv_node )
    {

        xmlNodePtr xscrv_node2 = XmlUtil::GetNode( xscrv_node, "XSecCurve", 0 );
        if ( xscrv_node2 )
        {

            int xsc_type = XmlUtil::FindInt( xscrv_node2, "Type", vsp::XS_CIRCLE );

            if ( m_XSCurve )
            {
                if ( m_XSCurve->GetType() != xsc_type )
                {
                    delete m_XSCurve;

                    m_XSCurve = XSecSurf::CreateXSecCurve( xsc_type );
                    m_XSCurve->SetParentContainer( m_ID );
                }
            }
        }

        if ( m_XSCurve )
        {
            m_XSCurve->DecodeXml( xscrv_node );
        }
    }
    return xscrv_node;
}

//==== Look Though All Parms and Load Linkable Ones ===//
void BORGeom::AddLinkableParms( vector< string > & parm_vec, const string & link_container_id )
{
    Geom::AddLinkableParms( parm_vec );

    if ( m_XSCurve  )
    {
        m_XSCurve->AddLinkableParms( parm_vec, m_ID );
    }
}

EditCurveXSec* BORGeom::ConvertToEdit()
{
    EditCurveXSec* xscrv_ptr = m_XSCurve->ConvertToEdit();

    if ( xscrv_ptr && xscrv_ptr != m_XSCurve )
    {
        delete m_XSCurve;

        m_XSCurve = xscrv_ptr;
        m_XSCurve->SetParentContainer( m_ID );
    }

    return xscrv_ptr;
}
