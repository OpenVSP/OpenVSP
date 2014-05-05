//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//

// XSec.cpp: implementation of the XSec class.
//
//////////////////////////////////////////////////////////////////////

#include "XSec.h"
#include "Geom.h"
#include "XSecSurf.h"
#include "Parm.h"
#include "VehicleMgr.h"
#include "ParmMgr.h"
#include "StlHelper.h"
#include "APIDefines.h"
#include <float.h>
#include <stdio.h>

#include "Vehicle.h"

using std::string;
using namespace vsp;

//==== Default Constructor ====//
XSec::XSec( XSecCurve *xsc, bool use_left )
{
    m_XSCurve = xsc;

    if ( m_XSCurve  )
    {
        m_XSCurve->SetParentContainer( m_ID );
    }

    m_rotation.loadIdentity();
    m_center = false;

    m_Type = -1;

    m_GroupName = "XSec";
    m_GroupSuffix = -1;
}

//==== Destructor ====//
XSec::~XSec()
{
    delete m_XSCurve;
}

void XSec::ChangeID( string newid )
{
    string oldid = m_ID;
    ParmContainer::ChangeID( newid );

    XSecSurf* xssurf = ( XSecSurf* ) GetParentContainerPtr();

    xssurf->ChangeXSecID( oldid, newid );

    if ( m_XSCurve  )
    {
        m_XSCurve->SetParentContainer( newid );
    }
}

void XSec::SetGroupDisplaySuffix( int num )
{
    m_GroupSuffix = num;
    //==== Assign Group Suffix To All Parms ====//
    for ( int i = 0 ; i < ( int )m_ParmVec.size() ; i++ )
    {
        Parm* p = ParmMgr.FindParm( m_ParmVec[i] );
        if ( p )
        {
            p->SetGroupDisplaySuffix( num );
        }
    }

    if ( m_XSCurve )
    {
        m_XSCurve->SetGroupDisplaySuffix( num );
    }
}

string XSec::GetName()
{
    ParmContainer* pc = GetParentContainerPtr();

    if ( pc )
    {
        char str[256];
        sprintf( str, "_%d", m_GroupSuffix );
        return pc->GetName() + " " + m_GroupName + string(str);
    }
    return ParmContainer::GetName();
}

//==== Set Scale ====//
void XSec::SetScale( double scale )
{
    GetXSecCurve()->SetScale( scale );
}

//==== Parm Changed ====//
void XSec::ParmChanged( Parm* parm_ptr, int type )
{
    if ( type == Parm::SET )
    {
        m_LateUpdateFlag = true;
        return;
    }

    Update();

    //==== Notify Parent Container (XSecSurf) ====//
    ParmContainer* pc = GetParentContainerPtr();
    if ( pc )
    {
        pc->ParmChanged( parm_ptr, type );
    }
}

//==== Get Curve ====//
VspCurve& XSec::GetCurve()
{
    if ( m_LateUpdateFlag )
    {
        Update();
    }

    return m_TransformedCurve;
}

//==== Get Untransformed Curve ====//
VspCurve& XSec::GetUntransformedCurve()
{
    return m_XSCurve->GetCurve();
}

//==== Look Though All Parms and Load Linkable Ones ===//
void XSec::AddLinkableParms( vector< string > & parm_vec, const string & link_container_id )
{
    ParmContainer::AddLinkableParms( parm_vec, link_container_id );

    if ( m_XSCurve  )
    {
        m_XSCurve->AddLinkableParms( parm_vec, link_container_id );
    }
}


Matrix4d* XSec::GetTransform()
{
    if ( m_LateUpdateFlag )
    {
        Update();
    }

    return &m_Transform;
}


//==== Copy From XSec ====//
void XSec::CopyFrom( XSec* xs )
{
    ParmMgr.ResetRemapID();
    xmlNodePtr root = xmlNewNode( NULL, ( const xmlChar * )"Vsp_Geometry" );
    if ( xs->GetType() == GetType() && xs->GetXSecCurve()->GetType() == GetXSecCurve()->GetType() )
    {
        xs->EncodeXml( root );
        DecodeXml( root );
    }
    else
    {
        xs->XSec::EncodeXml( root );
        DecodeXml( root );

        m_XSCurve->SetWidthHeight( xs->GetXSecCurve()->GetWidth(), xs->GetXSecCurve()->GetHeight() );
    }
    xmlFreeNode( root );
    ParmMgr.ResetRemapID();
}

//==== Encode XML ====//
xmlNodePtr XSec::EncodeXml(  xmlNodePtr & node  )
{
    ParmContainer::EncodeXml( node );
    xmlNodePtr xsec_node = xmlNewChild( node, NULL, BAD_CAST "XSec", NULL );
    if ( xsec_node )
    {
        XmlUtil::AddIntNode( xsec_node, "Type", m_Type );
        XmlUtil::AddStringNode( xsec_node, "GroupName", m_GroupName );

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
xmlNodePtr XSec::DecodeXml(  xmlNodePtr & node  )
{
    ParmContainer::DecodeXml( node );

    xmlNodePtr child_node = XmlUtil::GetNode( node, "XSec", 0 );
    if ( child_node )
    {
        m_GroupName = XmlUtil::FindString( child_node, "GroupName", m_GroupName );

        xmlNodePtr xscrv_node = XmlUtil::GetNode( child_node, "XSecCurve", 0 );
        if ( xscrv_node )
        {
            m_XSCurve->DecodeXml( xscrv_node );
        }
    }
    return child_node;
}

//==== Encode XSec ====//
xmlNodePtr XSec::EncodeXSec(  xmlNodePtr & node  )
{
    xmlNodePtr xsec_node = xmlNewChild( node, NULL, BAD_CAST "XSec", NULL );
    if ( xsec_node )
    {
        EncodeXml( xsec_node );
    }
    return xsec_node;
}

//==== Decode XSec ====//
// Called only from XSecSurf::DecodeXml -- i.e. when DecodeXml'ing entire Geom.
xmlNodePtr XSec::DecodeXSec(  xmlNodePtr & node   )
{
    if ( node )
    {
        DecodeXml( node );
    }
    return node;
}

//==== Compute Area ====//
double XSec::ComputeArea( int num_pnts )
{
    VspCurve curve = GetCurve();
    vector<vec3d> pnts;
    curve.Tesselate( num_pnts, pnts );
    vec3d zero;
    return poly_area( pnts, zero );
}

void XSec::SetTransformation( const Matrix4d &mat, bool center )
{
    m_rotation = mat;
    m_center = center;
}

//==========================================================================//
//==========================================================================//
//==========================================================================//

//==== Default Constructor ====//
FuseXSec::FuseXSec( XSecCurve *xsc, bool use_left ) : XSec( xsc, use_left)
{
    m_Type = XSEC_FUSE;

    m_RefLength = 1.0;

    m_XLocPercent.Init( "XLocPercent", m_GroupName, this,  0.0, 0.0, 1.0 );
    m_XLocPercent.SetDescript( "X distance of cross section as a percent of fuselage length" );
    m_YLocPercent.Init( "YLocPercent", m_GroupName, this,  0.0, -1.0, 1.0 );
    m_YLocPercent.SetDescript( "Y distance of cross section as a percent of fuselage length" );
    m_ZLocPercent.Init( "ZLocPercent", m_GroupName, this,  0.0, -1.0, 1.0 );
    m_ZLocPercent.SetDescript( "Z distance of cross section as a percent of fuselage length" );

    m_XRotate.Init( "XRotate", m_GroupName, this,  0.0, -180.0, 180.0 );
    m_XRotate.SetDescript( "Rotation about x-axis of cross section" );
    m_YRotate.Init( "YRotate", m_GroupName, this,  0.0, -180.0, 180.0 );
    m_YRotate.SetDescript( "Rotation about y-axis of cross section" );
    m_ZRotate.Init( "ZRotate", m_GroupName, this,  0.0, -180.0, 180.0 );
    m_YRotate.SetDescript( "Rotation about z-axis of cross section" );

    m_Spin.Init( "Spin", m_GroupName, this, 0.0, -180.0, 180.0 );

    m_RefLength.Init( "RefLength", m_GroupName, this, 1.0, 1e-8, 1e12, false );
}

//==== Update ====//
void FuseXSec::Update()
{
    m_LateUpdateFlag = false;

    XSecSurf* xsecsurf = (XSecSurf*) GetParentContainerPtr();

    Matrix4d mat;
    xsecsurf->GetBasicTransformation( m_XSCurve->GetWidth(), mat );

    VspCurve baseCurve = GetUntransformedCurve();

    baseCurve.Transform( mat );

    //==== Apply Transform ====//
    m_TransformedCurve = baseCurve;
    if ( fabs( m_Spin() ) > DBL_EPSILON )
    {
        std::cerr << "XSec spin not implemented." << std::endl;
// NOTE: Not implementing spin. Also, this implementation doesn't spin first or last segments
//      double val = 0.0;
//      if ( m_Spin() > 0 )         val = 0.5*(m_Spin()/180.0);
//      else if ( m_Spin() < 0 )    val = 1.0 + 0.5*(m_Spin()/180.0);
//      val = Clamp( val, 0.0, 0.99999999 );
//
//      vector< vec3d > ctl_pnts = m_Curve.GetControlPnts();
//      int split = (int)(val*(((int)ctl_pnts.size()-1)/3)+1);
//
//      vector< vec3d > spin_ctl_pnts;
//      for ( int i = split*3 ; i < (int)ctl_pnts.size()-1 ; i++ )
//          spin_ctl_pnts.push_back( ctl_pnts[i] );
//
//      for ( int i = 0 ; i <= split*3 ; i++ )
//          spin_ctl_pnts.push_back( ctl_pnts[i] );
//
//      m_TransformedCurve.SetControlPnts( spin_ctl_pnts );
    }

    m_Transform.loadIdentity();

    m_Transform.translatef( m_XLocPercent()*m_RefLength(), m_YLocPercent()*m_RefLength(), m_ZLocPercent()*m_RefLength() );

    m_Transform.rotateX( m_XRotate() );
    m_Transform.rotateY( m_YRotate() );
    m_Transform.rotateZ( m_ZRotate() );

    m_TransformedCurve.Transform( m_Transform );
}

//==== Set Ref Length ====//
void FuseXSec::SetRefLength( double len )
{
    if ( fabs( len - m_RefLength() ) < DBL_EPSILON )
    {
        return;
    }

    m_RefLength = len;
    m_LateUpdateFlag = true;

    m_XLocPercent.SetRefVal( m_RefLength() );
    m_YLocPercent.SetRefVal( m_RefLength() );
    m_ZLocPercent.SetRefVal( m_RefLength() );
}

//==== Copy position from base class ====//
// May be possible to do this using ParmContainer::EncodeXML, but all
// we want to do is copy the values in the XSec (not XSecCurve) class
// that control position.
void FuseXSec::CopyBasePos( XSec* xs )
{
    if ( xs )
    {
        FuseXSec* fxs = ( FuseXSec* ) xs;

        m_XLocPercent = fxs->m_XLocPercent();
        m_YLocPercent = fxs->m_YLocPercent();
        m_ZLocPercent = fxs->m_ZLocPercent();

        m_RefLength = fxs->m_RefLength;
    }
}

//==========================================================================//
//==========================================================================//
//==========================================================================//

//==== Default Constructor ====//
StackXSec::StackXSec( XSecCurve *xsc, bool use_left ) : XSec( xsc, use_left)
{
    m_Type = XSEC_STACK;

    m_XDelta.Init( "XDelta", m_GroupName, this,  1.0, 0.0, 1.0e12 );
    m_XDelta.SetDescript( "X distance of cross section from prior cross section" );
    m_YDelta.Init( "YDelta", m_GroupName, this,  0.0, -1.0e12, 1.0e12 );
    m_YDelta.SetDescript( "Y distance of cross section from prior cross section" );
    m_ZDelta.Init( "ZDelta", m_GroupName, this,  0.0, -1.0e12, 1.0e12 );
    m_ZDelta.SetDescript( "Z distance of cross section from prior cross section" );

    m_XRotate.Init( "XRotate", m_GroupName, this,  0.0, -180.0, 180.0 );
    m_XRotate.SetDescript( "Rotation about x-axis of cross section" );
    m_YRotate.Init( "YRotate", m_GroupName, this,  0.0, -180.0, 180.0 );
    m_YRotate.SetDescript( "Rotation about y-axis of cross section" );
    m_ZRotate.Init( "ZRotate", m_GroupName, this,  0.0, -180.0, 180.0 );
    m_YRotate.SetDescript( "Rotation about z-axis of cross section" );
}

//==== Set Scale ====//
void StackXSec::SetScale( double scale )
{
    XSec::SetScale( scale );
    m_XDelta = m_XDelta() * scale;
    m_YDelta = m_YDelta() * scale;
    m_ZDelta = m_ZDelta() * scale;
}

//==== Update ====//
void StackXSec::Update()
{
    m_LateUpdateFlag = false;

    XSecSurf* xsecsurf = (XSecSurf*) GetParentContainerPtr();
    int indx = xsecsurf->FindXSecIndex( m_ID );

    // apply the needed transformation to get section into body orientation

    Matrix4d mat;
    xsecsurf->GetBasicTransformation( m_XSCurve->GetWidth(), mat );

    VspCurve baseCurve = GetUntransformedCurve();

    baseCurve.Transform( mat );

    //==== Apply Transform ====//
    m_TransformedCurve = baseCurve;

    m_Transform.loadIdentity();

    if( indx > 0 )
    {
        StackXSec* prevxs = (StackXSec*) xsecsurf->FindXSec( indx - 1);
        if( prevxs )
        {
            m_Transform.matMult( prevxs->GetTransform()->data() );
        }
    }

    m_Transform.translatef( m_XDelta(), m_YDelta(), m_ZDelta() );

    m_Transform.rotateX( m_XRotate() );
    m_Transform.rotateY( m_YRotate() );
    m_Transform.rotateZ( m_ZRotate() );

    m_TransformedCurve.Transform( m_Transform );


    if( indx < xsecsurf->NumXSec() - 1 )
    {
        StackXSec* nextxs = (StackXSec*) xsecsurf->FindXSec( indx + 1);
        if( nextxs )
        {
            nextxs->SetLateUpdateFlag( true );
        }
    }

}

//==== Copy position from base class ====//
// May be possible to do this using ParmContainer::EncodeXML, but all
// we want to do is copy the values in the XSec (not XSecCurve) class
// that control position.
void StackXSec::CopyBasePos( XSec* xs )
{
    if ( xs )
    {
        StackXSec* sxs = ( StackXSec* ) xs;

        m_XDelta = sxs->m_XDelta();
        m_YDelta = sxs->m_YDelta();
        m_ZDelta = sxs->m_ZDelta();

        m_XRotate = sxs->m_XRotate();
        m_YRotate = sxs->m_YRotate();
        m_ZRotate = sxs->m_ZRotate();
    }
}

