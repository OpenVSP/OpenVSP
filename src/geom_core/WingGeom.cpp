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
#include "APIDefines.h"
#include <assert.h>

using namespace vsp;


//==== Constructor ====//
WingDriverGroup::WingDriverGroup() : DriverGroup( NUM_WSECT_DRIVER, 3 )
{
    m_CurrChoices[0] = SPAN_WSECT_DRIVER;
    m_CurrChoices[1] = ROOTC_WSECT_DRIVER;
    m_CurrChoices[2] = TIPC_WSECT_DRIVER;
}

void WingDriverGroup::UpdateGroup( vector< string > parmIDs )
{
    Parm* AR = ParmMgr.FindParm( parmIDs[AR_WSECT_DRIVER] );
    Parm* span = ParmMgr.FindParm( parmIDs[SPAN_WSECT_DRIVER] );
    Parm* area = ParmMgr.FindParm( parmIDs[AREA_WSECT_DRIVER] );
    Parm* taper = ParmMgr.FindParm( parmIDs[TAPER_WSECT_DRIVER] );
    Parm* aveC = ParmMgr.FindParm( parmIDs[AVEC_WSECT_DRIVER] );
    Parm* rootC = ParmMgr.FindParm( parmIDs[ROOTC_WSECT_DRIVER] );
    Parm* tipC = ParmMgr.FindParm( parmIDs[TIPC_WSECT_DRIVER] );

    vector< bool > uptodate;
    uptodate.resize( m_Nvar );

    for( int i = 0; i < m_Nvar; i++ )
    {
        uptodate[i] = false;
    }
    for( int i = 0; i < m_Nchoice; i++ )
    {
        uptodate[m_CurrChoices[i]] = true;
    }

    int niter = 0;
    while( vector_contains_val( uptodate, false ) )
    {
        if( !uptodate[AR_WSECT_DRIVER] )
        {
            if( uptodate[SPAN_WSECT_DRIVER] && uptodate[AREA_WSECT_DRIVER] )
            {
                AR->Set( span->Get() * span->Get() / area->Get() );
                uptodate[AR_WSECT_DRIVER] = true;
            }
            else if( uptodate[SPAN_WSECT_DRIVER] && uptodate[AVEC_WSECT_DRIVER] )
            {
                AR->Set( span->Get() / aveC->Get() );
                uptodate[AR_WSECT_DRIVER] = true;
            }
            else if( uptodate[AREA_WSECT_DRIVER] && uptodate[AVEC_WSECT_DRIVER] )
            {
                AR->Set( area->Get() / ( aveC->Get() * aveC->Get() ) );
                uptodate[AR_WSECT_DRIVER] = true;
            }
        }

        if( !uptodate[SPAN_WSECT_DRIVER] )
        {
            if( uptodate[AVEC_WSECT_DRIVER] && uptodate[AREA_WSECT_DRIVER] )
            {
                span->Set( area->Get() / aveC->Get() );
                uptodate[SPAN_WSECT_DRIVER] = true;
            }
            else if( uptodate[AR_WSECT_DRIVER] && uptodate[AREA_WSECT_DRIVER] )
            {
                span->Set( sqrt( AR->Get() * area->Get() ) );
                uptodate[SPAN_WSECT_DRIVER] = true;
            }
            else if( uptodate[AR_WSECT_DRIVER] && uptodate[AVEC_WSECT_DRIVER] )
            {
                span->Set( AR->Get() * aveC->Get() );
                uptodate[SPAN_WSECT_DRIVER] = true;
            }
        }


        if( !uptodate[AREA_WSECT_DRIVER] )
        {
            if( uptodate[SPAN_WSECT_DRIVER] && uptodate[AVEC_WSECT_DRIVER] )
            {
                area->Set( span->Get() * aveC->Get() );
                uptodate[AREA_WSECT_DRIVER] = true;
            }
            else if( uptodate[SPAN_WSECT_DRIVER] && uptodate[AR_WSECT_DRIVER] )
            {
                area->Set( span->Get() * span->Get() / AR->Get() );
                uptodate[AREA_WSECT_DRIVER] = true;
            }
            else if( uptodate[AR_WSECT_DRIVER] && uptodate[AVEC_WSECT_DRIVER] )
            {
                area->Set( AR->Get() * aveC->Get() * aveC->Get() );
                uptodate[AREA_WSECT_DRIVER] = true;
            }
        }

        if( !uptodate[AVEC_WSECT_DRIVER] )
        {
            if( uptodate[TIPC_WSECT_DRIVER] && uptodate[ROOTC_WSECT_DRIVER] )
            {
                aveC->Set( ( tipC->Get() + rootC->Get() ) / 2.0 );
                uptodate[AVEC_WSECT_DRIVER] = true;
            }
            else if( uptodate[SPAN_WSECT_DRIVER] && uptodate[AREA_WSECT_DRIVER] )
            {
                aveC->Set( area->Get() / span->Get() );
                uptodate[AVEC_WSECT_DRIVER] = true;
            }
        }

        if( !uptodate[TIPC_WSECT_DRIVER] )
        {
            if( uptodate[TAPER_WSECT_DRIVER] && uptodate[ROOTC_WSECT_DRIVER] )
            {
                tipC->Set( taper->Get() * rootC->Get() );
                uptodate[TIPC_WSECT_DRIVER] = true;
            }
            else if( uptodate[TAPER_WSECT_DRIVER] && uptodate[AVEC_WSECT_DRIVER] )
            {
                tipC->Set( 2.0 * aveC->Get() / ( 1.0 + ( 1.0 / taper->Get() ) )  );
                uptodate[TIPC_WSECT_DRIVER] = true;
            }
            else if( uptodate[ROOTC_WSECT_DRIVER] && uptodate[AVEC_WSECT_DRIVER] )
            {
                tipC->Set( 2.0 * aveC->Get() - rootC->Get() );
                uptodate[TIPC_WSECT_DRIVER] = true;
            }
        }

        if( !uptodate[ROOTC_WSECT_DRIVER] )
        {
            if( uptodate[TAPER_WSECT_DRIVER] && uptodate[TIPC_WSECT_DRIVER] )
            {
                rootC->Set( tipC->Get() / taper->Get() );
                uptodate[ROOTC_WSECT_DRIVER] = true;
            }
            else if( uptodate[TAPER_WSECT_DRIVER] && uptodate[AVEC_WSECT_DRIVER] )
            {
                rootC->Set( 2.0 * aveC->Get() / ( 1.0 + taper->Get() )  );
                uptodate[ROOTC_WSECT_DRIVER] = true;
            }
            else if( uptodate[TIPC_WSECT_DRIVER] && uptodate[AVEC_WSECT_DRIVER] )
            {
                rootC->Set( 2.0 * aveC->Get() - tipC->Get() );
                uptodate[ROOTC_WSECT_DRIVER] = true;
            }
        }

        if( !uptodate[TAPER_WSECT_DRIVER] )
        {
            if( uptodate[TIPC_WSECT_DRIVER] && uptodate[ROOTC_WSECT_DRIVER] )
            {
                taper->Set( tipC->Get() / rootC->Get() );
                uptodate[TAPER_WSECT_DRIVER] = true;
            }
        }

        // Each pass through the loop should update at least one variable.
        // With m_Nvar variables and m_Nchoice initially known, all should
        // be updated in ( m_Nvar - m_Nchoice ) iterations.  If not, we're
        // in an infinite loop.
        assert( niter < ( m_Nvar - m_Nchoice ) );
        niter++;
    }
}

bool WingDriverGroup::ValidDrivers( vector< int > choices )
{
    // Check for duplicate selections.
    for( int i = 0; i < (int)choices.size() - 1; i++ )
    {
        for( int j = i + 1; j < (int)choices.size(); j++ )
        {
            if( choices[i] == choices[j] )
            {
                return false;
            }
        }
    }

    // Check for algebraically nonsense selections.
    if( vector_contains_val( choices, ( int ) AR_WSECT_DRIVER ) &&
            vector_contains_val( choices, ( int ) SPAN_WSECT_DRIVER ) &&
            vector_contains_val( choices, ( int ) AREA_WSECT_DRIVER ) )
    {
        return false;
    }

    if( vector_contains_val( choices, ( int ) AR_WSECT_DRIVER ) &&
            vector_contains_val( choices, ( int ) SPAN_WSECT_DRIVER ) &&
            vector_contains_val( choices, ( int ) AVEC_WSECT_DRIVER ) )
    {
        return false;
    }

    if( vector_contains_val( choices, ( int ) AR_WSECT_DRIVER ) &&
            vector_contains_val( choices, ( int ) AREA_WSECT_DRIVER ) &&
            vector_contains_val( choices, ( int ) AVEC_WSECT_DRIVER ) )
    {
        return false;
    }

    if( vector_contains_val( choices, ( int ) AREA_WSECT_DRIVER ) &&
            vector_contains_val( choices, ( int ) SPAN_WSECT_DRIVER ) &&
            vector_contains_val( choices, ( int ) AVEC_WSECT_DRIVER ) )
    {
        return false;
    }

    if( vector_contains_val( choices, ( int ) TAPER_WSECT_DRIVER ) &&
            vector_contains_val( choices, ( int ) ROOTC_WSECT_DRIVER ) &&
            vector_contains_val( choices, ( int ) TIPC_WSECT_DRIVER ) )
    {
        return false;
    }

    if( vector_contains_val( choices, ( int ) AVEC_WSECT_DRIVER ) &&
            vector_contains_val( choices, ( int ) ROOTC_WSECT_DRIVER ) &&
            vector_contains_val( choices, ( int ) TIPC_WSECT_DRIVER ) )
    {
        return false;
    }

    if( vector_contains_val( choices, ( int ) AVEC_WSECT_DRIVER ) &&
            vector_contains_val( choices, ( int ) TAPER_WSECT_DRIVER ) &&
            vector_contains_val( choices, ( int ) TIPC_WSECT_DRIVER ) )
    {
        return false;
    }

    if( vector_contains_val( choices, ( int ) AVEC_WSECT_DRIVER ) &&
            vector_contains_val( choices, ( int ) ROOTC_WSECT_DRIVER ) &&
            vector_contains_val( choices, ( int ) TAPER_WSECT_DRIVER ) )
    {
        return false;
    }

    return true;
}

//=========================================================================================================//
//=========================================================================================================//
//=========================================================================================================//

//==== Constructor ====//
WingSect::WingSect( XSecCurve *xsc, bool use_left ) : XSec( xsc, use_left)
{
    m_Type = vsp::XSEC_WING;

    m_ProjectedSpan = 0.0;

    m_XDelta  = m_YDelta  = m_ZDelta  = 0;
    m_XRotate = m_YRotate = m_ZRotate = 0;
    m_XCenterRot = m_YCenterRot = m_ZCenterRot = 0;

    m_Aspect.Init( "Aspect", m_GroupName, this, 1.0, 0.001, 1000.0 );
    m_Aspect.SetDescript( "Aspect Ratio of Wing Section" );
    m_Taper.Init( "Taper", m_GroupName, this, 1.0, 0.001, 1000.0 );
    m_Taper.SetDescript( "Taper Ratio of Wing Section" );
    m_Area.Init( "Area", m_GroupName, this, 1.0, 0.0001, 1000000.0 );
    m_Area.SetDescript( "Area of Wing Section" );
    m_Span.Init( "Span", m_GroupName, this, 1.0, 0.0001, 1000000.0 );
    m_Span.SetDescript( "Span of Wing Section" );
    m_AvgChord.Init( "Avg_Chord", m_GroupName, this, 1.0, 0.0001, 1000000.0 );
    m_AvgChord.SetDescript( "Avg Chord of Wing Section" );
    m_TipChord.Init( "Tip_Chord", m_GroupName, this, 1.0, 0.0001, 1000000.0 );
    m_TipChord.SetDescript( "Tip Chord of Wing Section" );
    m_RootChord.Init( "Root_Chord", m_GroupName, this, 1.0, 0.0001, 1000000.0 );
    m_RootChord.SetDescript( "Root Chord of Wing Section" );

    m_Sweep.Init( "Sweep", m_GroupName, this, 0.0, -89.0, 89.0 );
    m_Sweep.SetDescript( "Sweep of Wing Section" );
    m_SweepLoc.Init( "Sweep_Location", m_GroupName, this, 0.0, 0.0, 1.0 );
    m_SweepLoc.SetDescript( "Location Along Chord That Sweep is Measured For Wing Section" );
    m_Twist.Init( "Twist", m_GroupName, this, 0.0, -45.0, 45.0 );
    m_Twist.SetDescript( "Twist of Wing Section" );
    m_TwistLoc.Init( "Twist_Location", m_GroupName, this, 0.25, 0.0, 1.0 );
    m_TwistLoc.SetDescript( "Location Along Chord That Airfoil is Rotated" );
    m_Dihedral.Init( "Dihedral", m_GroupName, this, 0.0, -360.0, 360.0 );
    m_Dihedral.SetDescript( "Dihedral of Wing Section" );

}

//==== Set Scale ====//
void WingSect::SetScale( double scale )
{
    XSec::SetScale( scale );
}

//==== Update ====//
void WingSect::Update()
{
    m_LateUpdateFlag = false;

    XSecSurf* xsecsurf = (XSecSurf*) GetParentContainerPtr();

    // apply the needed transformation to get section into body orientation
    Matrix4d mat;
    xsecsurf->GetBasicTransformation( m_XSCurve->GetWidth(), mat );

    VspCurve baseCurve = GetUntransformedCurve();

    baseCurve.Transform( mat );

    //==== Apply Transform ====//
    m_TransformedCurve = baseCurve;

    Matrix4d tran_mat;
    tran_mat.translatef( m_XDelta, m_YDelta, m_ZDelta );

    Matrix4d rotate_mat;
    rotate_mat.rotateX( m_XRotate );
    rotate_mat.rotateY( m_YRotate );
    rotate_mat.rotateZ( m_ZRotate );

    Matrix4d cent_mat;
    cent_mat.translatef( -m_XCenterRot, -m_YCenterRot, -m_ZCenterRot );

    Matrix4d inv_cent_mat;
    inv_cent_mat.translatef( m_XCenterRot, m_YCenterRot, m_ZCenterRot );

    m_Transform.loadIdentity();

    m_Transform.postMult( tran_mat.data() );
    m_Transform.postMult( cent_mat.data() );
    m_Transform.postMult( rotate_mat.data() );
    m_Transform.postMult( inv_cent_mat.data() );

    m_Transform.postMult( xsecsurf->GetGlobalXForm().data() );

    m_TransformedCurve.Transform( m_Transform );


    //==== Inform Outboard Section of Change ====//
    int indx = xsecsurf->FindXSecIndex( m_ID );
    if( indx < xsecsurf->NumXSec() - 1 )
    {
        WingSect* nextxs = (WingSect*) xsecsurf->FindXSec( indx + 1);
        if( nextxs )
        {
            nextxs->SetLateUpdateFlag( true );
        }
    }

}


//==== Copy position from base class ====//
void WingSect::CopyBasePos( XSec* xs )
{
    //if ( xs )
    //{
    //    WingSect* sxs = ( WingSect* ) xs;

    //    m_XDelta = sxs->m_XDelta;
    //    m_YDelta = sxs->m_YDelta;
    //    m_ZDelta = sxs->m_ZDelta;

    //    m_XRotate = sxs->m_XRotate;
    //    m_YRotate = sxs->m_YRotate;
    //    m_ZRotate = sxs->m_ZRotate;
    //}
}

//==== Get Driver Parms ====//
vector< string > WingSect::GetDriverParms()
{
    vector< string > parm_ids;
    parm_ids.resize( WingDriverGroup::NUM_WSECT_DRIVER );
    parm_ids[ WingDriverGroup::AR_WSECT_DRIVER ] = m_Aspect.GetID();
    parm_ids[ WingDriverGroup::SPAN_WSECT_DRIVER ] = m_Span.GetID();
    parm_ids[ WingDriverGroup::AREA_WSECT_DRIVER ] = m_Area.GetID();
    parm_ids[ WingDriverGroup::TAPER_WSECT_DRIVER ] = m_Taper.GetID();
    parm_ids[ WingDriverGroup::AVEC_WSECT_DRIVER ] = m_AvgChord.GetID();
    parm_ids[ WingDriverGroup::ROOTC_WSECT_DRIVER ] = m_RootChord.GetID();
    parm_ids[ WingDriverGroup::TIPC_WSECT_DRIVER ] = m_TipChord.GetID();

    return parm_ids;
}

void WingSect::ForceChordVal( double val, bool root_chord_flag )
{
    vector<int> span_rc_tc;
    span_rc_tc.push_back( WingDriverGroup::SPAN_WSECT_DRIVER );
    span_rc_tc.push_back( WingDriverGroup::ROOTC_WSECT_DRIVER );
    span_rc_tc.push_back( WingDriverGroup::TIPC_WSECT_DRIVER );

    vector< int > save_choice_vec = m_DriverGroup.GetChoices();
    m_DriverGroup.SetChoices( span_rc_tc );
    if ( root_chord_flag )
        m_RootChord = val;
    else
        m_TipChord = val;

    m_DriverGroup.UpdateGroup( GetDriverParms() );
    m_DriverGroup.SetChoices( save_choice_vec );
    m_DriverGroup.UpdateGroup( GetDriverParms() );

    Update();
}


void WingSect::ForceSpanRcTc(  double span, double rc, double tc )
{
    vector<int> span_rc_tc;
    span_rc_tc.push_back( WingDriverGroup::SPAN_WSECT_DRIVER );
    span_rc_tc.push_back( WingDriverGroup::ROOTC_WSECT_DRIVER );
    span_rc_tc.push_back( WingDriverGroup::TIPC_WSECT_DRIVER );

    vector< int > save_choice_vec = m_DriverGroup.GetChoices();
    m_DriverGroup.SetChoices( span_rc_tc );

    m_Span = span;
    m_RootChord = rc;
    m_TipChord = tc;

    m_DriverGroup.UpdateGroup( GetDriverParms() );
    m_DriverGroup.SetChoices( save_choice_vec );
    m_DriverGroup.UpdateGroup( GetDriverParms() );

    Update();
}

void WingSect::ForceAspectTaperArea( double aspect, double taper, double area )
{
    vector<int> aspect_taper_area;
    aspect_taper_area.push_back( WingDriverGroup::AR_WSECT_DRIVER );
    aspect_taper_area.push_back( WingDriverGroup::TAPER_WSECT_DRIVER );
    aspect_taper_area.push_back( WingDriverGroup::AREA_WSECT_DRIVER );

    vector< int > save_choice_vec = m_DriverGroup.GetChoices();
    m_DriverGroup.SetChoices( aspect_taper_area );

    m_Aspect = aspect;
    m_Taper  = taper;
    m_Area   = area;

    m_DriverGroup.UpdateGroup( GetDriverParms() );
    m_DriverGroup.SetChoices( save_choice_vec );
    m_DriverGroup.UpdateGroup( GetDriverParms() );

    Update();
}


double WingSect::GetTanSweepAt( double sweep, double loc  )
{
  double tan_sweep = tan(sweep*DEG_2_RAD);
  double tan_sweep_at = tan_sweep - (2.0/m_Aspect())*
            ( (loc-m_SweepLoc()) * ((1.0-m_Taper())/(1.0+m_Taper())) );

  return ( tan_sweep_at );
}

//==== Encode XML ====//
xmlNodePtr WingSect::EncodeXml(  xmlNodePtr & node  )
{
    ParmContainer::EncodeXml( node );
    xmlNodePtr xsec_node = xmlNewChild( node, NULL, BAD_CAST "XSec", NULL );
    if ( xsec_node )
    {
        XmlUtil::AddIntNode( xsec_node, "Type", m_Type );
        XmlUtil::AddStringNode( xsec_node, "GroupName", m_GroupName );

        m_DriverGroup.EncodeXml( xsec_node );

        xmlNodePtr xscrv_node = xmlNewChild( xsec_node, NULL, BAD_CAST "XSecCurve", NULL );
        if ( xscrv_node )
        {
            m_XSCurve->EncodeXml( xscrv_node );
        }
    }
    return xsec_node;
}

//==== Decode XML ====//
// Called from XSec::DecodeXSec, XSec::CopyFrom, and overridden calls to ParmContainer::DecodeXml --
// i.e. during DecodeXml entire Geom, but also for in-XSecSurf copy/paste/insert.
xmlNodePtr WingSect::DecodeXml(  xmlNodePtr & node  )
{
    ParmContainer::DecodeXml( node );

    xmlNodePtr child_node = XmlUtil::GetNode( node, "XSec", 0 );
    if ( child_node )
    {
        m_GroupName = XmlUtil::FindString( child_node, "GroupName", m_GroupName );

         m_DriverGroup.DecodeXml( child_node );

        xmlNodePtr xscrv_node = XmlUtil::GetNode( child_node, "XSecCurve", 0 );
        if ( xscrv_node )
        {
            m_XSCurve->DecodeXml( xscrv_node );
        }
    }
    return child_node;
}

//=========================================================================================================//
//=========================================================================================================//
//=========================================================================================================//


//==== Constructor ====//
WingGeom::WingGeom( Vehicle* vehicle_ptr ) : GeomXSec( vehicle_ptr )
{
    m_Name = "WingGeom";
    m_Type.m_Name = "Wing";
    m_Type.m_Type = MS_WING_GEOM_TYPE;

    m_Closed = false;

    m_XSecSurf.SetParentContainer( GetID() );
    m_XSecSurf.SetBasicOrientation( vsp::Y_DIR, X_DIR, XS_SHIFT_LE, true );

    m_RelativeDihedralFlag.Init("RelativeDihedralFlag", m_Name, this, 0, 0, 1, false );
    m_RelativeDihedralFlag.SetDescript( "Relative or Absolute Dihedral" );

    m_RelativeTwistFlag.Init("RelativeTwistFlag", m_Name, this, 0, 0, 1, false );
    m_RelativeTwistFlag.SetDescript( "Relative or Absolute Twist" );

    m_RotateAirfoilMatchDiedralFlag.Init("RotateAirfoilMatchDideralFlag", m_Name, this, 0, 0, 1, false );
    m_RotateAirfoilMatchDiedralFlag.SetDescript( "Rotate Airfoil To Stay Tangent To Dihedral (or Not)" );

    m_TotalSpan.Init( "TotalSpan", m_Name, this, 1.0, 0.0001, 1000000.0 );
    m_TotalSpan.SetDescript( "Total Planform Span" );

    m_TotalProjSpan.Init( "TotalProjectedSpan", m_Name, this, 1.0, 0.0001, 1000000.0 );
    m_TotalProjSpan.SetDescript( "Total Projected Planform Span" );

    m_TotalChord.Init( "m_TotalChord", m_Name, this, 1.0, 0.0001, 1000000.0 );
    m_TotalChord.SetDescript( "Total Planform Chord" );

    m_TotalArea.Init( "TotalArea", m_Name, this, 1.0, 0.0001, 1000000.0 );
    m_TotalArea.SetDescript( "Total Planform Area" );

    //==== Init Parms ====//
    m_TessU = 16;
    m_TessW = 31;
    m_ActiveXSec = 1;
    m_ActiveAirfoil = 0;
    m_SymPlanFlag = SYM_XZ;

    //==== Wing XSecs ====//
    m_XSecSurf.SetXSecType( XSEC_WING );
    m_XSecSurf.SetCutMinNumXSecs( 2 );

    m_XSecSurf.AddXSec( vsp::XS_FOUR_SERIES );
    m_XSecSurf.AddXSec( vsp::XS_FOUR_SERIES );

    WingSect* ws;

    ws = ( WingSect* ) m_XSecSurf.FindXSec( 0 );
    ws->SetGroupDisplaySuffix( 0 );

    ws = ( WingSect* ) m_XSecSurf.FindXSec( 1 );
    ws->SetGroupDisplaySuffix( 1 );

    UpdateSurf();
}

//==== Destructor ====//
WingGeom::~WingGeom()
{

}

//==== Change IDs =====//
void WingGeom::ChangeID( string id )
{
    Geom::ChangeID( id );
    m_XSecSurf.SetParentContainer( GetID() );
}

//==== Look Though All Parms and Load Linkable Ones ===//
void WingGeom::AddLinkableParms( vector< string > & linkable_parm_vec, const string & link_container_id )
{
    Geom::AddLinkableParms( linkable_parm_vec );

    m_XSecSurf.AddLinkableParms( linkable_parm_vec, m_ID  );
}

//==== Scale ====//
void WingGeom::Scale()
{
    //double currentScale = m_Scale() / m_LastScale;
    //for ( int i = 0 ; i < m_XSecSurf.NumXSec() ; i++ )
    //{
    //    XSec* xs = m_XSecSurf.FindXSec( i );
    //    if ( xs )
    //    {
    //        xs->SetScale( currentScale );
    //    }
    //}

    //m_LastScale = m_Scale();
}

//==== Drag Parameters ====//
void WingGeom::LoadDragFactors( DragFactors& drag_factors )
{


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
        m_XSecSurf.DecodeXml( wing_node );
    }

    return wing_node;
}

//==== Compute Rotation Center ====//
void WingGeom::ComputeCenter()
{
    m_Center.set_x( 0.0 );
    m_Center.set_y( 0.0 );
    m_Center.set_z( 0.0 );
}

//==== Set Index For Active Wing Sect ====//
int WingGeom::GetActiveXSecIndex()
{
    return m_ActiveXSec;
}

//==== Set Index For Active Wing Sect ====//
void WingGeom::SetActiveXSecIndex( int index )
{
    index = Clamp<int>( index, 1, m_XSecSurf.NumXSec() - 1 );
    m_ActiveXSec = index;
}

//==== Set Index For Active Airfoil ====//
void WingGeom::SetActiveAirfoilIndex( int index )
{
    index = Clamp<int>( index, 0, m_XSecSurf.NumXSec() - 1 );
    m_ActiveAirfoil = index;
}


//==== Get XSec ====//
XSec* WingGeom::GetXSec( int index )
{
    return m_XSecSurf.FindXSec( index );
}

//==== Set Active XSec Type ====//
void WingGeom::SetActiveAirfoilType( int type )
{
    XSec* xs = GetXSec( m_ActiveAirfoil );

    if ( !xs )
    {
        return;
    }

    if ( type == xs->GetXSecCurve()->GetType() )
    {
        return;
    }

    m_XSecSurf.ChangeXSecShape( m_ActiveAirfoil, type );

    Update();
}


bool WingGeom::IsClosed() const
{
    return m_Closed;
}


WingSect* WingGeom::GetWingSect( int index )
{
    XSec* xs = m_XSecSurf.FindXSec( index );
    if ( xs )
    {
        WingSect* ws = dynamic_cast< WingSect* >(xs);
        return ws;
    }
    return NULL;
}

//==== Insert Wing Sect ====//
void WingGeom::SplitWingSect( int index  )
{
    WingSect* ws = GetWingSect( index );
    if ( ws )
    {
        double span = ws->m_Span();
        double rc = ws->m_RootChord();
        double ac = ws->m_AvgChord();
        double tc = ws->m_TipChord();
        double twist = 0.5*ws->m_Twist();

        int curve_type = ws->GetXSecCurve()->GetType();

        string ins_id = m_XSecSurf.InsertXSec( curve_type, index );

        ws->ForceSpanRcTc( span*0.5, rc, ac );
        ws->m_Twist = twist;
        ws->Update();

        XSec* xs = m_XSecSurf.FindXSec( ins_id );
        if ( xs )
        {
            WingSect* ins_ws = dynamic_cast< WingSect* >(xs);
            ins_ws->CopyFrom( ws );
            ins_ws->ForceSpanRcTc( span*0.5, ac, tc );
            ins_ws->m_Twist = twist;
            ins_ws->Update();
        }
    }

}

//==== Cut Wing Sect ====//
void WingGeom::CutWingSect( int index  )
{
    if ( index > 0 && index < m_XSecSurf.NumXSec() )
    {
        m_XSecSurf.CutXSec( index );

        //==== Reset Active Indeices ====//
        SetActiveXSecIndex( GetActiveXSecIndex() );
        SetActiveXSecIndex( GetActiveAirfoilIndex() );
    }
}


//==== Copy Wing Sect ====//
void WingGeom::CopyWingSect( int index  )
{
    if ( index > 0 && index < m_XSecSurf.NumXSec() )
    {
        m_XSecSurf.CopyXSec(index);
    }
}

//==== Paste Wing Sect ====//
void WingGeom::PasteWingSect( int index  )
{
    if ( index > 0 && index < m_XSecSurf.NumXSec() )
    {
        m_XSecSurf.PasteXSec(index);
        XSec* xs = m_XSecSurf.FindXSec( index );
        if ( xs )
            xs->SetLateUpdateFlag( true );
        Update();
    }
}

//==== Copy Wing Sect ====//
void WingGeom::CopyAirfoil( int index  )
{
    if ( index >= 0 && index < m_XSecSurf.NumXSec() )
    {
        m_XSecSurf.CopyXSecCurve( index );
    }
}

//==== Paste Wing Sect ====//
void WingGeom::PasteAirfoil( int index  )
{
    if ( index >= 0 && index < m_XSecSurf.NumXSec() )
    {
        m_XSecSurf.PasteXSecCurve(index);
        XSec* xs = m_XSecSurf.FindXSec( index );
        if ( xs )
            xs->SetLateUpdateFlag( true );
        Update();
    }
}

//==== Insert Wing Sect ====//
void WingGeom::InsertWingSect( int index  )
{
    if ( index > 0 && index < m_XSecSurf.NumXSec() )
    {
        XSec* xs = m_XSecSurf.FindXSec( index );
        int type = xs->GetXSecCurve()->GetType();

        string ins_id = m_XSecSurf.InsertXSec(type, index);
        SetActiveXSecIndex( index + 1 );
        PasteWingSect( GetActiveXSecIndex() );
    }
}

//==== Update Wing And Cross Section Placement ====//
void WingGeom::UpdateSurf()
{
    //==== Check If Total Span/Chord/Area Has Changed ====//
    if ( UpdatedParm( m_TotalSpan.GetID() ) )
        UpdateTotalSpan();

    if ( UpdatedParm( m_TotalProjSpan.GetID() ) )
        UpdateTotalProjSpan();

    if ( UpdatedParm( m_TotalChord.GetID() ) )
        UpdateTotalChord();

    if ( UpdatedParm( m_TotalArea.GetID() ) )
        UpdateTotalArea();

    //==== Make Sure Chord Match For Adjacent Wing Sections ====//
    MatchWingSections();

    //==== Cross Section Curves & joint info ====//
    vector< VspCurve > crv_vec;
    crv_vec.resize( m_XSecSurf.NumXSec() );

    m_TessUVec.clear();

    //==== Compute Parameters For Each Section ====//
    double total_span = 0.0;
    double total_sweep_offset = 0.0;
    double total_dihed_offset = 0.0;
    double total_twist = 0.0;

    //==== Load End Points for Each Section ====//
    for ( int i = 0 ; i < m_XSecSurf.NumXSec() ; i++ )
    {
        WingSect* ws = ( WingSect* ) m_XSecSurf.FindXSec( i );
        if ( ws )
        {

            double rad = ws->m_Span();
            double ty = rad*cos(GetSumDihedral(i)*DEG_2_RAD);
            double tz = rad*sin(GetSumDihedral(i)*DEG_2_RAD);

            double tan_le  = ws->GetTanSweepAt( ws->m_Sweep(), 0.0 );
            double toff    = tan_le*rad;                    // Tip X Offset

            if ( i == 0 )
            {
                ty = 0;
                tz = 0;
                toff = 0;
            }

            total_dihed_offset += tz;
            total_span += ty;
            total_sweep_offset += toff;

            if ( m_RelativeTwistFlag() )
                total_twist += ws->m_Twist();
            else
                total_twist = ws->m_Twist();

            ws->SetProjectedSpan( ty );

            //==== Find Width Parm ====//
            string width_id = ws->GetXSecCurve()->GetWidthParmID();
            Parm* width_parm = ParmMgr.FindParm( width_id );

            if ( width_parm )
            {
                width_parm->Deactivate();
                width_parm->Set( ws->m_TipChord() );
            }

            double dihead_rot = 0.0;
            if ( m_RotateAirfoilMatchDiedralFlag() )
            {
                if ( i == 0 )
                {
                    dihead_rot = GetSumDihedral( i+1 );
                }
                else if ( i ==  m_XSecSurf.NumXSec()-1 )
                {
                    dihead_rot = GetSumDihedral( i );
                }
                else
                {
                    dihead_rot = 0.5*( GetSumDihedral( i ) + GetSumDihedral( i+1 ) );
                }
            }

            //==== Load Transformations =====//
            ws->m_YDelta = total_span;
            ws->m_XDelta = total_sweep_offset;
            ws->m_ZDelta = total_dihed_offset;

            ws->m_YRotate = total_twist;
            ws->m_XRotate = dihead_rot;

            ws->m_XCenterRot = ws->m_XDelta + ws->m_TwistLoc()*ws->m_TipChord();
            ws->m_YCenterRot = ws->m_YDelta;
            ws->m_ZCenterRot = ws->m_ZDelta;

            crv_vec[i] =  ws->GetCurve();

            if ( i > 0 )
            {
                m_TessUVec.push_back( ws->m_SectTessU() );
            }

        }
    }


    m_MainSurfVec[0].SkinC0( crv_vec, false );
    if ( m_XSecSurf.GetFlipUD() )
    {
        m_MainSurfVec[0].FlipNormal();
    }
    m_MainSurfVec[0].SetSurfType( VspSurf::WING_SURF );

    //==== Load Totals ====//
    m_TotalSpan = ComputeTotalSpan();
    m_TotalProjSpan = ComputeTotalProjSpan();
    m_TotalChord = ComputeTotalChord();
    m_TotalArea = ComputeTotalArea();

}

void WingGeom::UpdateTesselate( int indx, vector< vector< vec3d > > &pnts, vector< vector< vec3d > > &norms, vector< vector< vec3d > > &uw_pnts  )
{
    m_SurfVec[indx].Tesselate( m_TessUVec, m_TessW(), pnts, norms, uw_pnts );
}

//==== Get All WingSections ====//
vector< WingSect* > WingGeom::GetWingSectVec()
{
    vector< WingSect* > ws_vec;
    for ( int i = 0 ; i < m_XSecSurf.NumXSec() ; i++ )
    {
        WingSect* ws = ( WingSect* ) m_XSecSurf.FindXSec( i );
        if ( ws )
        {
            ws_vec.push_back( ws );
        }
    }
    return ws_vec;
}

//==== Compute Total Span ====//
double WingGeom::ComputeTotalSpan()
{
    double ts = 0.0;
    vector< WingSect* > ws_vec = GetWingSectVec();
    for ( int i = 1 ; i < (int)ws_vec.size() ; i++ )
    {
            ts += ws_vec[i]->m_Span();
    }

    if ( GetSymFlag() != 0 )
    {
        ts *= 2.0;
    }

    return ts;
}



//==== Compute Total Proj Span ====//
double WingGeom::ComputeTotalProjSpan()
{
    double ts = 0.0;
    vector< WingSect* > ws_vec = GetWingSectVec();
    for ( int i = 1 ; i < (int)ws_vec.size() ; i++ )
    {
            ts += ws_vec[i]->m_Span()*cos( ws_vec[i]->m_Dihedral() * DEG_2_RAD);
    }

    if ( GetSymFlag() != 0 )
    {
        ts *= 2.0;
    }

    return ts;
}

//==== Compute Total Chord ====//
double WingGeom::ComputeTotalChord()
{
    double tc = 0.0;
    vector< WingSect* > ws_vec = GetWingSectVec();
    for ( int i = 1 ; i < (int)ws_vec.size() ; i++ )
    {
        tc += ws_vec[i]->m_AvgChord();
    }
    tc = tc/(double)(ws_vec.size()-1);
    return tc;

}

//==== Compute Total Chord ====//
double WingGeom::ComputeTotalArea()
{
    double ta = 0.0;
    vector< WingSect* > ws_vec = GetWingSectVec();
    for ( int i = 1 ; i < (int)ws_vec.size() ; i++ )
    {
        ta += ws_vec[i]->m_Area();
    }

    if ( GetSymFlag() != 0 )
    {
        ta *= 2.0;
    }

    return ta;
}

//==== Update Total Span ====//
void WingGeom::UpdateTotalSpan()
{
     //==== Compute Totals ====//
    double ts = ComputeTotalSpan();

    double fract = 1.0;
    if ( ts > 1.0e-08 )
        fract = m_TotalSpan()/ts;

    //==== Adjust Sections Total Span ====//
    if ( fract > 1.0e-08 )
    {
        vector< WingSect* > ws_vec = GetWingSectVec();
        for ( int i = 1 ; i < (int)ws_vec.size() ; i++ )
        {
            double span = ws_vec[i]->m_Span()*fract;
            ws_vec[i]->ForceSpanRcTc( span, ws_vec[i]->m_RootChord(), ws_vec[i]->m_TipChord() );
        }
    }
}
//==== Update Total Proj Span ====//
void WingGeom::UpdateTotalProjSpan()
{
     //==== Compute Totals ====//
    double ts = ComputeTotalProjSpan();

    double fract = 1.0;
    if ( ts > 1.0e-08 )
        fract = m_TotalProjSpan()/ts;

    //==== Adjust Sections Total Span ====//
    if ( fract > 1.0e-08 )
    {
        vector< WingSect* > ws_vec = GetWingSectVec();
        for ( int i = 1 ; i < (int)ws_vec.size() ; i++ )
        {
            double span = ws_vec[i]->m_Span()*fract;
            ws_vec[i]->ForceSpanRcTc( span, ws_vec[i]->m_RootChord(), ws_vec[i]->m_TipChord() );
        }
    }
}

//==== Update Total Chord ====//
void WingGeom::UpdateTotalChord()
{
    double tc = ComputeTotalChord();

    double fract = 1.0;
    if ( tc > 1.0e-08 )
        fract = m_TotalChord()/tc;

    //==== Adjust Sections Chord ====//
    if ( fract > 1.0e-08 )
    {
        vector< WingSect* > ws_vec = GetWingSectVec();

        vector< double > new_tc_vals;
        for ( int i = 0 ; i < (int)ws_vec.size() ; i++ )
        {
            new_tc_vals.push_back( ws_vec[i]->m_TipChord()*fract );
        }

        for ( int i = 1 ; i < (int)ws_vec.size() ; i++ )
        {
            double rc = new_tc_vals[i-1];
            double tc = new_tc_vals[i];
            ws_vec[i]->ForceSpanRcTc( ws_vec[i]->m_Span(), rc, tc );
        }
    }
}

//==== Update Total Area ====//
void WingGeom::UpdateTotalArea()
{
    double ta = ComputeTotalArea();

    double fract = 1.0;
    if ( ta > 1.0e-08 )
        fract = m_TotalArea()/ta;

    //==== Adjust Sections Area ====//
    if ( fract > 1.0e-08 )
    {
        vector< WingSect* > ws_vec = GetWingSectVec();
        for ( int i = 1 ; i < (int)ws_vec.size() ; i++ )
        {
            double area = ws_vec[i]->m_Area()*fract;
            ws_vec[i]->ForceAspectTaperArea( ws_vec[i]->m_Aspect(), ws_vec[i]->m_Taper(), area );
        }
    }
}


//==== Match Wing Sections =====//
void WingGeom::MatchWingSections()
{
    //==== Match Section Root/Tip ====//
    WingSect* active_ws = ( WingSect* ) m_XSecSurf.FindXSec( m_ActiveXSec );
    if ( active_ws )
    {
        active_ws->m_DriverGroup.UpdateGroup( active_ws->GetDriverParms() );
        double active_rc = active_ws->m_RootChord();
        double active_tc = active_ws->m_TipChord();

        if ( m_ActiveXSec > 0 )
        {
            WingSect* inboard_ws = ( WingSect* ) m_XSecSurf.FindXSec( m_ActiveXSec-1 );
            inboard_ws->ForceChordVal( active_rc, false );
        }
        if ( m_ActiveXSec < m_XSecSurf.NumXSec()-1 )
        {
            WingSect* outboard_ws = ( WingSect* ) m_XSecSurf.FindXSec( m_ActiveXSec+1 );
            outboard_ws->ForceChordVal( active_tc, true );
        }
    }
}

//==== Get Sum Dihedral ====//
double WingGeom::GetSumDihedral( int sect_id )
{
    if ( sect_id < 1 || sect_id >= (int)m_XSecSurf.NumXSec() )
        return 0.0;

    if ( m_RelativeDihedralFlag() == false )
    {
        WingSect* ws = ( WingSect* ) m_XSecSurf.FindXSec( sect_id );
        return ws->m_Dihedral();
    }

    double sum_dihedral = 0.0;
    for ( int i = 1 ; i <= sect_id ; i++ )
    {
        WingSect* ws = ( WingSect* ) m_XSecSurf.FindXSec( i );
        sum_dihedral += ws->m_Dihedral();
    }

    return sum_dihedral;
}

