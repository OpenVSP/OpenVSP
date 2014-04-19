//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//
//
//////////////////////////////////////////////////////////////////////

#include "StackGeom.h"
#include "ParmMgr.h"
#include "VspSurf.h"
#include "Vehicle.h"
#include "StlHelper.h"
#include "APIDefines.h"

using namespace vsp;

//==== Constructor ====//
StackGeom::StackGeom( Vehicle* vehicle_ptr ) : GeomXSec( vehicle_ptr )
{
    m_Name = "StackGeom";
    m_Type.m_Name = "Stack";
    m_Type.m_Type = STACK_GEOM_TYPE;

    m_Closed = false;

    m_XSecSurf.SetParentContainer( GetID() );

    m_XSecSurf.SetBasicOrientation( X_DIR, Y_DIR, XS_SHIFT_MID, false );

    //==== Init Parms ====//
    m_TessU = 16;
    m_TessW = 17;
    m_TessW.SetMultShift( 8, 1 );

    m_ActiveXSec = 0;

    m_XSecSurf.SetXSecType( XSEC_STACK );

    m_XSecSurf.AddXSec( XS_POINT );
    m_XSecSurf.AddXSec( XS_ELLIPSE );
    m_XSecSurf.AddXSec( XS_ELLIPSE );
    m_XSecSurf.AddXSec( XS_ELLIPSE );
    m_XSecSurf.AddXSec( XS_POINT );

    int j;
    StackXSec* xs;

    j = 0;
    xs = ( StackXSec* ) m_XSecSurf.FindXSec( j );
    xs->SetGroupDisplaySuffix( j );
    xs->m_XDelta = 0.0;

    ++j;
    xs = ( StackXSec* ) m_XSecSurf.FindXSec( j );
    xs->SetGroupDisplaySuffix( j );
    xs->m_XDelta = 1.0;
    dynamic_cast<EllipseXSec *>( xs->GetXSecCurve() )->SetWidthHeight( 3.0, 2.5 );

    ++j;
    xs = ( StackXSec* ) m_XSecSurf.FindXSec( j );
    xs->SetGroupDisplaySuffix( j );
    xs->m_XDelta = 2.0;
    dynamic_cast<EllipseXSec *>( xs->GetXSecCurve() )->SetWidthHeight( 3.0, 2.5 );

    ++j;
    xs = ( StackXSec* ) m_XSecSurf.FindXSec( j );
    xs->SetGroupDisplaySuffix( j );
    xs->m_XDelta = 1.0;
    dynamic_cast<EllipseXSec *>( xs->GetXSecCurve() )->SetWidthHeight( 3.0, 2.5 );

    ++j;
    xs = ( StackXSec* ) m_XSecSurf.FindXSec( j );
    xs->SetGroupDisplaySuffix( j );
    xs->m_XDelta = 0.5;

}

//==== Destructor ====//
StackGeom::~StackGeom()
{

}

void StackGeom::ChangeID( string id )
{
    ParmContainer::ChangeID( id );
    m_XSecSurf.SetParentContainer( GetID() );
}

//==== Update Fuselage And Cross Section Placement ====//
void StackGeom::UpdateSurf()
{
    m_TessUVec.clear();

    //==== Cross Section Curves & joint info ====//
    vector< VspCurve > crv_vec;
    crv_vec.resize( m_XSecSurf.NumXSec() );

    //==== Update XSec Location/Rotation ====//
    for ( int i = 0 ; i < m_XSecSurf.NumXSec() ; i++ )
    {
        StackXSec* xs = ( StackXSec* ) m_XSecSurf.FindXSec( i );

        if ( xs )
        {
            if ( i == 0 )
            {
                xs->m_XDelta.SetLowerUpperLimits( 0.0, 0.0 );
                xs->m_YDelta.SetLowerUpperLimits( 0.0, 0.0 );
                xs->m_ZDelta.SetLowerUpperLimits( 0.0, 0.0 );

                xs->m_XRotate.SetLowerUpperLimits( 0.0, 0.0 );
                xs->m_YRotate.SetLowerUpperLimits( 0.0, 0.0 );
                xs->m_ZRotate.SetLowerUpperLimits( 0.0, 0.0 );
            }
            else
            {
                xs->m_XDelta.SetLowerUpperLimits( 0.0, 1.0e12 );
                xs->m_YDelta.SetLowerUpperLimits( -1.0e12, 1.0e12 );
                xs->m_ZDelta.SetLowerUpperLimits( -1.0e12, 1.0e12 );

                xs->m_XRotate.SetLowerUpperLimits( -180.0, 180.0 );
                xs->m_YRotate.SetLowerUpperLimits( -180.0, 180.0 );
                xs->m_ZRotate.SetLowerUpperLimits( -180.0, 180.0 );
            }

            //==== Reset Group Names ====//
            xs->SetGroupDisplaySuffix( i );

            crv_vec[i] =  xs->GetCurve();

            if ( i > 0 )
            {
                m_TessUVec.push_back( xs->m_SectTessU() );
            }
        }
    }


    m_SurfVec[0].InterpolateLinear( crv_vec, false );
    if ( m_XSecSurf.GetFlipUD() )
    {
        m_SurfVec[0].FlipNormal();
    }
}


void StackGeom::UpdateTesselate( int indx, vector< vector< vec3d > > &pnts, vector< vector< vec3d > > &norms )
{
    m_SurfVec[indx].Tesselate( m_TessUVec, m_TessW(), pnts, norms );
}

//==== Compute Rotation Center ====//
void StackGeom::ComputeCenter()
{
    m_Center.set_x( 0.0 );
    m_Center.set_y( 0.0 );
    m_Center.set_z( 0.0 );
}

//==== Encode Data Into XML Data Struct ====//
xmlNodePtr StackGeom::EncodeXml( xmlNodePtr & node )
{
    Geom::EncodeXml( node );
    xmlNodePtr fuselage_node = xmlNewChild( node, NULL, BAD_CAST "FuselageGeom", NULL );
    if ( fuselage_node )
    {
        m_XSecSurf.EncodeXml( fuselage_node );
    }
    return fuselage_node;
}

//==== Encode Data Into XML Data Struct ====//
xmlNodePtr StackGeom::DecodeXml( xmlNodePtr & node )
{
    Geom::DecodeXml( node );

    xmlNodePtr fuselage_node = XmlUtil::GetNode( node, "FuselageGeom", 0 );
    if ( fuselage_node )
    {
        m_XSecSurf.DecodeXml( fuselage_node );
    }

    return fuselage_node;
}

//==== Set Index For Active XSec ====//
void StackGeom::SetActiveXSecIndex( int index )
{
    index = Clamp<int>( index, 0, m_XSecSurf.NumXSec() - 1 );

    m_ActiveXSec = index;
}

//==== Get XSec ====//
XSec* StackGeom::GetXSec( int index )
{
    return m_XSecSurf.FindXSec( index );
}

//==== Set Active XSec Type ====//
void StackGeom::SetActiveXSecType( int type )
{
    XSec* xs = GetXSec( m_ActiveXSec );

    if ( !xs )
    {
        return;
    }

    if ( type == xs->GetXSecCurve()->GetType() )
    {
        return;
    }

    m_XSecSurf.ChangeXSecShape( m_ActiveXSec, type );

    Update();
}

//==== Cut Active XSec ====//
void StackGeom::CutActiveXSec()
{
    m_XSecSurf.CutXSec( m_ActiveXSec );
    m_XSecSurf.FindXSec( m_ActiveXSec )->SetLateUpdateFlag( true );
    Update();
}

//==== Copy Active XSec ====//
void StackGeom::CopyActiveXSec()
{
    m_XSecSurf.CopyXSec( m_ActiveXSec );
}

//==== Paste Cut/Copied XSec To Active XSec ====//
void StackGeom::PasteActiveXSec()
{
    m_XSecSurf.PasteXSec( m_ActiveXSec );
    m_XSecSurf.FindXSec( m_ActiveXSec )->SetLateUpdateFlag( true );
    Update();
}

//==== Insert XSec ====//
void StackGeom::InsertXSec( )
{
    if ( m_ActiveXSec >= NumXSec() - 1 )
    {
        return;
    }

    XSec* xs = GetXSec( m_ActiveXSec );
    if ( xs )
    {
        InsertXSec( xs->GetXSecCurve()->GetType() );
    }
}

//==== Insert XSec ====//
void StackGeom::InsertXSec( int type )
{
    if ( m_ActiveXSec >= NumXSec() - 1 )
    {
        return;
    }

    StackXSec* xs = ( StackXSec* ) GetXSec( m_ActiveXSec );

    m_XSecSurf.InsertXSec( type, m_ActiveXSec );
    m_ActiveXSec++;

    StackXSec* inserted_xs = ( StackXSec* ) GetXSec( m_ActiveXSec );

    if ( inserted_xs )
    {
        inserted_xs->CopyFrom( xs );
    }

    inserted_xs->SetLateUpdateFlag( true );
    Update();
}


//==== Look Though All Parms and Load Linkable Ones ===//
void StackGeom::AddLinkableParms( vector< string > & linkable_parm_vec, const string & link_container_id )
{
    ParmContainer::AddLinkableParms( linkable_parm_vec );

    m_XSecSurf.AddLinkableParms( linkable_parm_vec, m_ID  );
}

//==== Scale ====//
void StackGeom::Scale()
{
    double currentScale = m_Scale() / m_LastScale;
    for ( int i = 0 ; i < m_XSecSurf.NumXSec() ; i++ )
    {
        XSec* xs = m_XSecSurf.FindXSec( i );
        if ( xs )
        {
            xs->SetScale( currentScale );
        }
    }

    m_LastScale = m_Scale();
}

//==== Drag Parameters ====//
void StackGeom::LoadDragFactors( DragFactors& drag_factors )
{
    double max_xsec_area = 0.000000000001;
    for ( int i = 0 ; i < ( int )m_XSecSurf.NumXSec() ; i++ )
    {
        XSec* xs = m_XSecSurf.FindXSec( i );
        double a = xs->ComputeArea( 100 );
        if ( a > max_xsec_area )
        {
            max_xsec_area = a;
        }
    }

    double dia = 2.0 * sqrt( max_xsec_area / PI );

//    drag_factors.m_Length = m_Length();
    drag_factors.m_MaxXSecArea = max_xsec_area;
//    drag_factors.m_LengthToDia = m_Length() / dia;
}

void StackGeom::GetJointParams( int joint, VspJointInfo &jointInfo ) const
{
    m_SurfVec[0].CompJointParams( joint, jointInfo );
}

bool StackGeom::IsClosed() const
{
    return m_Closed;
}
