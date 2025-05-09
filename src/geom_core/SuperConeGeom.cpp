//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//
//
//////////////////////////////////////////////////////////////////////

#include "SuperConeGeom.h"
#include "Vehicle.h"

typedef eli::geom::curve::piecewise_ellipse_creator<double, 3, curve_tolerance_type> piecewise_ellipse_creator;

//==== Constructor ====//
SuperConeGeom::SuperConeGeom( Vehicle* vehicle_ptr ) : Geom( vehicle_ptr )
{
    m_Name = "SuperConeGeom";
    m_Type.m_Name = "SuperCone";
    m_Type.m_Type = SUPER_CONE_GEOM_TYPE;

    //==== Init Parms ====//
    m_TessU = 5;
    m_TessW = 8;

    m_Distance.Init( "Distance", "Design", this, 2.0, 0.0, 1.0e12 );
    m_Distance.SetDescript( "Distance of SuperCone" );

    m_Xoff = 0.0;

    m_XSCurve = NULL;
    SetXSecCurveType( vsp::XS_CIRCLE );

}

//==== Destructor ====//
SuperConeGeom::~SuperConeGeom()
{

}

void SuperConeGeom::UpdateSurf()
{
    if ( !m_XSCurve )
    {
        return;
    }

    m_XSCurve->Update(); // May not need to force Update here()

    double w = m_XSCurve->GetWidth();

    vector< VspCurve > crv_vec;
    crv_vec.resize( 3 );

    crv_vec[0].MakePoint();

    VspCurve c = m_XSCurve->GetCurve();

    Matrix4d basicmat;
    basicmat.translatef( -w * 0.5, 0, 0 );
    c.Transform( basicmat );

    c.Scale( M_PI / 180.0 );

    Matrix4d mat;
    mat.translatef( 0, 0, m_Distance() );

    c.Transform( mat );

    c.EvaluateOnSphere( false );
    crv_vec[1] = c;

    crv_vec[2].MakePoint();
    crv_vec[2].Transform( mat );


    // Revolve to unit sphere
    m_MainSurfVec[0].SkinC0( crv_vec, false );

    // Shift so nose is at origin
    m_MainSurfVec[0].OffsetX( m_Xoff );
}

//==== Change IDs =====//
void SuperConeGeom::ChangeID( string id )
{
    Geom::ChangeID( id );
    if ( m_XSCurve )
    {
        m_XSCurve->SetParentContainer( GetID() );
    }
}

void SuperConeGeom::UpdatePreTess()
{
}

//==== Compute Rotation Center ====//
void SuperConeGeom::ComputeCenter()
{
    m_Center.set_x( m_XSCurve->GetWidth() * m_Origin() );
}

//==== Scale ====//
void SuperConeGeom::Scale()
{
    double currentScale = m_Scale() / m_LastScale();

    m_Distance *= currentScale;

    m_XSCurve->SetScale( currentScale );

    m_LastScale = m_Scale();
}

void SuperConeGeom::AddDefaultSources( double base_len )
{
}

void SuperConeGeom::OffsetXSecs( double off )
{
    m_XSCurve->OffsetCurve( off );

    m_Xoff = off;
}

void SuperConeGeom::SetXSecCurveType( int type )
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

int SuperConeGeom::GetXSecCurveType()
{
    return m_XSCurve->GetType();
}

void SuperConeGeom::UpdateDrawObj()
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

void SuperConeGeom::LoadDrawObjs( vector< DrawObj* > & draw_obj_vec )
{
    Geom::LoadDrawObjs( draw_obj_vec );

    m_CurrentXSecDrawObj.m_Screen = DrawObj::VSP_XSEC_SCREEN;
    m_CurrentXSecDrawObj.m_GeomID = XSECHEADER + m_ID + "CURRENT";
    m_CurrentXSecDrawObj.m_Visible = m_Vehicle->IsGeomActive( m_ID );
    draw_obj_vec.push_back( &m_CurrentXSecDrawObj );
}

//==== Encode XML ====//
xmlNodePtr SuperConeGeom::EncodeXml(  xmlNodePtr & node  )
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
xmlNodePtr SuperConeGeom::DecodeXml(  xmlNodePtr & node  )
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
void SuperConeGeom::AddLinkableParms( vector< string > & parm_vec, const string & link_container_id )
{
    Geom::AddLinkableParms( parm_vec );

    if ( m_XSCurve  )
    {
        m_XSCurve->AddLinkableParms( parm_vec, m_ID );
    }
}

EditCurveXSec* SuperConeGeom::ConvertToEdit()
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
