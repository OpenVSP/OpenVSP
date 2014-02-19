//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//
//
//////////////////////////////////////////////////////////////////////

#include "WingGeom.h"
#include "ParmMgr.h"
#include "VspSurf.h"
#include "Vehicle.h"
#include "StlHelper.h"


//==== Constructor ====//
WingGeom::WingGeom( Vehicle* vehicle_ptr ) : GeomXSec( vehicle_ptr ), m_DoneConstructing( false )
{
    m_Name = "WingGeom";
    m_Type.m_Name = "Wing";
    m_Type.m_Type = MS_WING_GEOM_TYPE;

    m_Closed = false;

    Matrix4d mat;
    double *pm( mat.data() );
    bool center;

    // rotation                ; translation
    pm[0] = 0;
    pm[4] = 0;
    pm[ 8] = -1;
    pm[12] = 0;
    pm[1] = 1;
    pm[5] = 0;
    pm[ 9] = 0;
    pm[13] = 0;
    pm[2] = 0;
    pm[6] = 1;
    pm[10] = 0;
    pm[14] = 0;
    pm[3] = 0;
    pm[7] = 0;
    pm[11] = 0;
    pm[15] = 0;
    center = true;

    m_XSecSurf.SetTransformation( mat, true );
    m_XSecSurf.SetParentContainer( GetID() );

    //==== Init Parms ====//
    m_TessU = 16;
    m_TessW = 11;

    m_Length.Init( "Length", "Design", this, 30.0, 1.0e-8, 1.0e12 );
    m_Length.SetDescript( "Length of fuselage" );

// Revert back to MANUAL_CONNECT once get skinning fixed
#if 0
    m_XSecConnect.Init( "XSecConnect", "Design", this, MANUAL_CONNECT, 0, NUM_CONNECT );
#else
    m_XSecConnect.Init( "XSecConnect", "Design", this, LINEAR_CONNECT, 0, NUM_CONNECT );
#endif
    m_XSecConnect.SetDescript( "Method of connecting cross-sections" );

    m_ActiveXSec = 0;
    m_XSecSurf.AddXSec( XSec::POINT );
    m_XSecSurf.AddXSec( XSec::ELLIPSE );
    m_XSecSurf.AddXSec( XSec::ELLIPSE );
    m_XSecSurf.AddXSec( XSec::ELLIPSE );
    m_XSecSurf.AddXSec( XSec::POINT );

    VspJointInfo joint;
    double angle, strength, curvature;
    int i, j;
    XSec* xs;

    j = 0;
    xs = m_XSecSurf.FindXSec( j );
    xs->SetGroupDisplaySuffix( j );
    xs->m_XLocPercent = 0.0;
    angle = 90 * DEG_2_RAD;
    strength = 0.75;
    curvature = 2.0;
    joint.SetState( VspJointInfo::C1 );
    for ( i = 0; i < 4; ++i )
    {
        joint.SetRightParams( i, angle, strength, curvature );
    }
    xs->SetJointInfo( joint );

    ++j;
    xs = m_XSecSurf.FindXSec( j );
    xs->SetGroupDisplaySuffix( j );
    xs->m_XLocPercent = 0.25;
    dynamic_cast<EllipseXSec *>( xs )->SetWidthHeight( 3.0, 2.5 );
    angle = 0 * DEG_2_RAD;
    strength = 1.0;
    curvature = 0.0;
    joint.SetState( VspJointInfo::C2 );
    for ( i = 0; i < 4; ++i )
    {
        joint.SetLeftParams( i, angle, strength, curvature );
        joint.SetRightParams( i, angle, strength, curvature );
    }
    xs->SetJointInfo( joint );

    ++j;
    xs = m_XSecSurf.FindXSec( j );
    xs->SetGroupDisplaySuffix( j );
    xs->m_XLocPercent = 0.5;
    dynamic_cast<EllipseXSec *>( xs )->SetWidthHeight( 3.0, 2.5 );
    angle = 0 * DEG_2_RAD;
    strength = 1.0;
    curvature = 0.0;
    joint.SetState( VspJointInfo::C2 );
    for ( i = 0; i < 4; ++i )
    {
        joint.SetLeftParams( i, angle, strength, curvature );
        joint.SetRightParams( i, angle, strength, curvature );
    }
    xs->SetJointInfo( joint );

    ++j;
    xs = m_XSecSurf.FindXSec( j );
    xs->SetGroupDisplaySuffix( j );
    xs->m_XLocPercent = 0.75;
    dynamic_cast<EllipseXSec *>( xs )->SetWidthHeight( 3.0, 2.5 );
    angle = 0 * DEG_2_RAD;
    strength = 1.0;
    curvature = 0.0;
    joint.SetState( VspJointInfo::C2 );
    for ( i = 0; i < 4; ++i )
    {
        joint.SetLeftParams( i, angle, strength, curvature );
        joint.SetRightParams( i, angle, strength, curvature );
    }
    xs->SetJointInfo( joint );

    ++j;
    xs = m_XSecSurf.FindXSec( j );
    xs->SetGroupDisplaySuffix( j );
    xs->m_XLocPercent = 1.0;
    angle = -90 * DEG_2_RAD;
    strength = 0.75;
    curvature = 2.0;
    joint.SetState( VspJointInfo::C1 );
    for ( i = 0; i < 4; ++i )
    {
        joint.SetLeftParams( i, angle, strength, curvature );
    }
    xs->SetJointInfo( joint );

    m_DoneConstructing = true;
}

//==== Destructor ====//
WingGeom::~WingGeom()
{

}

void WingGeom::ChangeID( string id )
{
    ParmContainer::ChangeID( id );
    m_XSecSurf.SetParentContainer( GetID() );
}

//==== Update Wing And Cross Section Placement ====//
void WingGeom::UpdateSurf()
{
    double len = m_Length();
    int connection_type( m_XSecConnect() );

    //==== Cross Section Curves & joint info ====//
    vector< VspCurve > crv_vec;
    vector<VspJointInfo> joint_info;
    crv_vec.resize( m_XSecSurf.NumXSec() );
    joint_info.resize( m_XSecSurf.NumXSec() );

    //==== Update XSec Location/Rotation ====//
    char str[256];
    for ( int i = 0 ; i < m_XSecSurf.NumXSec() ; i++ )
    {
        XSec* xs = m_XSecSurf.FindXSec( i );

        if ( xs )
        {
            //==== Reset Group Names ====//
            sprintf( str, "XSec_%d", i );

            //==== Set X Limits ====//
#if 0
            // NOTE: This code breaks cross section insertion and deletion because Update()
            //       gets called when a new cross section is created but before the cross section
            //       has been fully initialized. For example, trace the code through a cross section
            //       insert and see that XSecSurf::AddXSec() needs to call ParmChanged() to let
            //       geometry know that the number of u-curves changed because the cross section
            //       count changed and not the Gen tab changing the count. Also, this logic should
            //       probably be in the cross section code (perhaps XSecSurf) so that when the cross
            //       section x-location is changed this is checked.
            int policy = FUSE_MONOTONIC;
            int duct_ile = 2;  // Only needed for FUSE_DUCT
            EnforceOrder( xs, i, duct_ile, policy );
#endif

            xs->SetRefLength( m_Length() );

            crv_vec[i] =  xs->GetCurve();
            joint_info[i] = xs->GetJointInfo();
        }
    }

#if 0
    int idx = 0;
    std::cout << "<===== in  ======>" << std::endl;
    std::cout << "\t segment " << idx;
    std::cout << "\tright curvature=" << joint_info[idx].GetRightCurvature( 0 );
    std::cout << std::endl;
#endif

    // construct the surface
    switch ( connection_type )
    {
        // linear interpolate between cross-sections
    case( LINEAR_CONNECT ):
    {
        m_SurfVec[0].InterpolateLinear( crv_vec, false );
        break;
    }
    // C1 PCHIP interpolate between cross-sections
    case( PCHIP_CONNECT ):
    {
        m_SurfVec[0].InterpolatePCHIP( crv_vec, false );
        break;
    }
    // C2 cubic spline interpolate between cross-sections
    case( CSPLINE_CONNECT ):
    {
        m_SurfVec[0].InterpolateCSpline( crv_vec, false );
        break;
    }
    case( MANUAL_CONNECT ):
    {
        m_SurfVec[0].InterpolateManual( crv_vec, joint_info, false );
        break;
    }
    default:
    {
        std::cerr << "Invalid cross-section type" << std::endl;
    }
    }

    // get the joint info into the cross sections
    for ( int i = 0 ; i < m_XSecSurf.NumXSec() ; i++ )
    {
        XSec* xs = m_XSecSurf.FindXSec( i );

        if ( xs )
        {
            VspJointInfo joint;

            m_SurfVec[0].CompJointParams( i, joint );

            switch ( connection_type )
            {
            case( LINEAR_CONNECT ):
            {
                joint.SetState( VspJointInfo::C0 );
                break;
            }
            case( PCHIP_CONNECT ):
            {
                joint.SetState( VspJointInfo::C1 );
                break;
            }
            case( CSPLINE_CONNECT ):
            {
                joint.SetState( VspJointInfo::C2 );
                break;
            }
            case( MANUAL_CONNECT ):
            {
                joint.SetState( joint_info[i].GetState() );
                break;
            }
            default:
            {
                std::cerr << "Invalid connection type" << std::endl;
                break;
            }
            }

            xs->SetJointInfo( joint );
        }
    }

#if 0
    VspJointInfo joint;
    m_SurfVec[0].CompJointParams( idx, joint );
    std::cout << "<===== out ======>" << std::endl;
    std::cout << "\t segment " << idx;
    std::cout << "\tright curvature=" << joint.GetRightCurvature( 0 );
    std::cout << std::endl;
    std::cout << "<================>" << std::endl;
#endif
}

void WingGeom::UpdateTesselate( int indx, vector< vector< vec3d > > &pnts, vector< vector< vec3d > > &norms )
{
    vector<int> nu( NumXSec() - 1 );
    int nv( m_TessW() );

    // fill the nu vector
    for ( size_t i = 0; i < nu.size(); ++i )
    {
        XSec *xs = GetXSec( i );
        nu[i] = xs->m_NRightSecs();
    }

    // if closed in u- or w-directions then need to add another coordinate line
    // to see desired number
    if ( m_SurfVec[indx].IsClosedU() )
    {
        // FIX: Should actually increase size of nu by one and use the last xs's m_NRightSecs() for
        // that value. The question is whether surface's tesselate can handle that.
        std::cerr << "Cannot handle closed surface in v-direction" << std::endl;
    }
    if ( m_SurfVec[indx].IsClosedW() )
    {
        ++nv;
    }

    m_SurfVec[indx].Tesselate( nu, nv, pnts, norms );
}

//==== Compute Rotation Center ====//
void WingGeom::ComputeCenter()
{
    m_Center.set_x( m_Length()*m_Origin() );
}

//==== Encode Data Into XML Data Struct ====//
xmlNodePtr WingGeom::EncodeXml( xmlNodePtr & node )
{
    Geom::EncodeXml( node );
    xmlNodePtr wing_node = xmlNewChild( node, NULL, BAD_CAST "WingGeom", NULL );
    if ( wing_node )
    {
        m_XSecSurf.EncodeXml( wing_node );
    }
    return wing_node;
}

//==== Encode Data Into XML Data Struct ====//
xmlNodePtr WingGeom::DecodeXml( xmlNodePtr & node )
{
    Geom::DecodeXml( node );

    xmlNodePtr wing_node = XmlUtil::GetNode( node, "WingGeom", 0 );
    if ( wing_node )
    {
        m_DoneConstructing = false;
        m_XSecSurf.DecodeXml( wing_node );
        m_DoneConstructing = true;
    }

    return wing_node;
}

void WingGeom::ParmChanged( Parm* parm_ptr, int type )
{
    // catch special case where an XSec num U changed
    if ( ( parm_ptr == 0 ) && ( type == -1001 ) )
    {
        m_TessU.Set( m_XSecSurf.GetNumTess() );
    }

    // if the total number of tesselation in the v-direction change, then need to make sure the
    // cross sections and the m_TessU values are in sync
    if ( ( parm_ptr != 0 ) && ( parm_ptr->GetID() == m_TessU.GetID() ) )
    {
        int nTess( m_TessU() ), nXSec( m_XSecSurf.GetNumTess() );

        // if have cross sections and the numbers don't match it means that m_TessU was updated
        // and the cross sections need to be updated
        if ( ( nXSec > 0 ) && ( nTess != nXSec ) )
        {
            m_TessU.Set( m_XSecSurf.UpdateNumTess( nTess ) );
        }
    }

    // call base class method if have enough cross sections
    if ( m_DoneConstructing )
    {
        GeomXSec::ParmChanged( parm_ptr, type );
    }
}

//==== Set Index For Active XSec ====//
void WingGeom::SetActiveXSecIndex( int index )
{
    index = Clamp<int>( index, 0, m_XSecSurf.NumXSec() - 1 );

    m_ActiveXSec = index;
}

//==== Get XSec ====//
XSec* WingGeom::GetXSec( int index )
{
    return m_XSecSurf.FindXSec( index );
}

//==== Set Active XSec Type ====//
void WingGeom::SetActiveXSecType( int type )
{
    XSec* xs = GetXSec( m_ActiveXSec );

    if ( !xs )
    {
        return;
    }

    if ( type == xs->GetType() )
    {
        return;
    }

    m_XSecSurf.ChangeXSecType( m_ActiveXSec, type );

    Update();
}

//==== Cut Active XSec ====//
void WingGeom::CutActiveXSec()
{
    m_XSecSurf.CutXSec( m_ActiveXSec );
    Update();
}

//==== Copy Active XSec ====//
void WingGeom::CopyActiveXSec()
{
    m_XSecSurf.CopyXSec( m_ActiveXSec );
}

//==== Paste Cut/Copied XSec To Active XSec ====//
void WingGeom::PasteActiveXSec()
{
    m_XSecSurf.PasteXSec( m_ActiveXSec );

    Update();
}

//==== Insert XSec ====//
void WingGeom::InsertXSec( )
{
    if ( m_ActiveXSec >= NumXSec() - 1 )
    {
        return;
    }

    XSec* xs = GetXSec( m_ActiveXSec );
    if ( xs )
    {
        InsertXSec( xs->GetType() );
    }
}

//==== Insert XSec ====//
void WingGeom::InsertXSec( int type )
{
    if ( m_ActiveXSec >= NumXSec() - 1 )
    {
        return;
    }

    XSec* xs = GetXSec( m_ActiveXSec );

    double x_loc_0 = xs->m_XLocPercent();
    double x_loc_1 = GetXSec( m_ActiveXSec + 1 )->m_XLocPercent();

    m_XSecSurf.InsertXSec( type, m_ActiveXSec );
    m_ActiveXSec++;

    XSec* inserted_xs = GetXSec( m_ActiveXSec );

    if ( inserted_xs )
    {
        inserted_xs->CopyFrom( xs );
        GetXSec( m_ActiveXSec )->m_XLocPercent = ( x_loc_0 + x_loc_1 ) * 0.5;
    }

    Update();
}


//==== Look Though All Parms and Load Linkable Ones ===//
void WingGeom::AddLinkableParms( vector< string > & linkable_parm_vec, const string & link_container_id )
{
    ParmContainer::AddLinkableParms( linkable_parm_vec );

    m_XSecSurf.AddLinkableParms( linkable_parm_vec, m_ID  );
}

//==== Scale ====//
void WingGeom::Scale()
{
    double currentScale = m_Scale() / m_LastScale;
    m_Length *= currentScale;
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
void WingGeom::LoadDragFactors( DragFactors& drag_factors )
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

    drag_factors.m_Length = m_Length();
    drag_factors.m_MaxXSecArea = max_xsec_area;
    drag_factors.m_LengthToDia = m_Length() / dia;
}

void WingGeom::GetJointParams( int joint, VspJointInfo &jointInfo ) const
{
    m_SurfVec[0].CompJointParams( joint, jointInfo );
}

bool WingGeom::IsClosed() const
{
    return m_Closed;
}

void WingGeom::EnforceOrder( XSec* xs, int indx, int ile, int policy )
{
    if( policy == FUSE_MONOTONIC )
    {
        if ( indx == 0 )
        {
            xs->m_XLocPercent.SetLowerUpperLimits( 0.0, 0.0 );
        }
        else if ( indx ==  m_XSecSurf.NumXSec() - 1 )
        {
            xs->m_XLocPercent.SetLowerUpperLimits( 1.0, 1.0 );
        }
        else
        {
            double lower = m_XSecSurf.FindXSec( indx - 1 )->m_XLocPercent();
            double upper = m_XSecSurf.FindXSec( indx + 1 )->m_XLocPercent();
            xs->m_XLocPercent.SetLowerUpperLimits( lower , upper );
        }
    }
    else if( policy == FUSE_DUCT )
    {
        if ( indx == 0 )
        {
            xs->m_XLocPercent.SetLowerUpperLimits( 1.0, 1.0 );
        }
        else if ( indx ==  m_XSecSurf.NumXSec() - 1 )
        {
            xs->m_XLocPercent.SetLowerUpperLimits( 1.0, 1.0 );
        }
        else if ( indx == ile )
        {
            xs->m_XLocPercent.SetLowerUpperLimits( 0.0, 0.0 );
        }
        else
        {
            xs->m_XLocPercent.SetLowerUpperLimits( 0.0, 1.0 );
        }

    }
    else if( policy == FUSE_FREE )
    {
        xs->m_XLocPercent.SetLowerUpperLimits( 0.0, 1.0 );
    }
}
