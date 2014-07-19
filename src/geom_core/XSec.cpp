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
    if ( xsc == NULL )
    {
        //==== Create A Default Curve ====//
        xsc = new XSecCurve();
    }

    m_XSCurve = xsc;

    if ( m_XSCurve  )
    {
        m_XSCurve->SetParentContainer( m_ID );
    }

    m_Type = -1;

    m_GroupName = "XSec";
    m_GroupSuffix = -1;

    m_SectTessU.Init( "SectTess_U", m_GroupName, this, 3, 2,  100 );
    m_SectTessU.SetDescript( "Number of tessellated curves in the U direction for this section" );
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

void XSec::SetXSecCurve( XSecCurve* xs_crv )
{
    if ( m_XSCurve )
        delete m_XSCurve;

    m_XSCurve = xs_crv;
    m_XSCurve->SetParentContainer( m_ID );
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

// Given a position along a curve t, and a desired surfce angle theta, calculate
// the tangent and normal unit vectors that will be required by the surface
// skinning algorithm.
void XSec::GetTanNormVec( double t, double theta, vec3d &tangent, vec3d &normal )
{
    Matrix4d basis;

    // Get primary orientation of this XSecSurf
    XSecSurf* xsecsurf = (XSecSurf*) GetParentContainerPtr();
    xsecsurf->GetBasicTransformation( 0.0, basis );

    // Transform primary orientation to orientation of this XSec
    basis.postMult( GetTransform()->data() );

    // Pull out width, up, and principal directions.
    vec3d wdir, updir, pdir;
    basis.getBasis( wdir, updir, pdir );

    double tmin, tmax;
    m_TransformedCurve.GetCurve().get_parameter_min( tmin );
    m_TransformedCurve.GetCurve().get_parameter_max( tmax );

    // Rotate basis around principal direction to point along curve
    // according to parameter and assumed circle.
    Matrix4d rmat;
    rmat.rotate( 2.0*PI*(t-tmin)/(tmax-tmin), pdir );
    basis.postMult( rmat.data() );

    // Pull out rotated directions.
    basis.getBasis( wdir, updir, pdir );

    // Rotate basis to specified slope.
    rmat.rotate( theta, updir );
    basis.postMult( rmat.data() );

    // Pull out desired normal and tangent directions.
    basis.getBasis( normal, updir, tangent);
}

// Given a vector of positions along a curve ts, desired surface angles thetas,
// angle strengths angstr, and curvature strengths crvstr, calculate the tangent
// and normal vector curves that will be required by the skinning algorithm.
void XSec::GetTanNormCrv( const vector< double > &ts, const vector< double > &thetas,
        const vector< double > &angstr, const vector< double > &crvstr,
        piecewise_curve_type &tangentcrv, piecewise_curve_type &normcrv )
{
    int nts = ts.size();

    // Set up cubic spline of desired controls
    piecewise_cubic_spline_creator_type pcc( nts - 1 );
    pcc.set_t0( ts[0] );
    for ( int i = 0; i < nts - 1; ++i)
    {
        pcc.set_segment_dt( ts[i+1] - ts[i], i );
    }

    vector< curve_point_type > pts(nts);
    for( int i = 0; i < nts; i++ )
    {
        pts[i] << thetas[i], angstr[i], crvstr[i];
    }
    pcc.set_closed_cubic_spline( pts.begin() );

    // Build control curve.
    piecewise_curve_type crvcntrl;
    pcc.create( crvcntrl );

    // Evaluate control curve at all the piecewise endpoints
    // used to define the XSecCurve.

    // Parameters that define the XSecCurve
    vector< double > crvts;
    // Work around non-constness of get_pmap.
    piecewise_curve_type crv = m_TransformedCurve.GetCurve();
    crv.get_pmap( crvts );

    int ntcrv = crvts.size();

    // Evaluate controls and build tan & norm vectors at piecewise endpoints
    vector< curve_point_type > tanpts(ntcrv), nrmpts(ntcrv);
    for( int i = 0; i < ntcrv; i++ )
    {
        curve_point_type crvparm = crvcntrl.f( crvts[i] );

        vec3d tangent, normal;

        GetTanNormVec( crvts[i], crvparm.x(), tangent, normal );

        tangent = tangent * crvparm.y();
        normal = normal * crvparm.z();

        tanpts[i] << tangent.v[0], tangent.v[1], tangent.v[2];
        nrmpts[i] << normal.v[0], normal.v[1], normal.v[2];
    }

    pcc.set_number_segments( ntcrv - 1 );
    pcc.set_t0( crvts[0] );
    for ( int i = 0; i < ntcrv - 1; ++i)
    {
        pcc.set_segment_dt( crvts[i+1] - crvts[i], i );
    }
    // Build tangent and normal vector curves.
    pcc.set_closed_cubic_spline( tanpts.begin() );
    pcc.create( tangentcrv );

    pcc.set_closed_cubic_spline( nrmpts.begin() );
    pcc.create( normcrv );
}

void XSec::GetTanNormCrv( const vector< double > &thetas,
        const vector< double > &angstr, const vector< double > &crvstr,
        piecewise_curve_type &tangentcrv, piecewise_curve_type &normcrv )
{
    vector< double > ts = { 0.0, 1.0, 2.0, 3.0, 4.0 };

    assert( thetas.size() == 5 );
    assert( angstr.size() == 5 );
    assert( crvstr.size() == 5 );

    GetTanNormCrv( ts, thetas, angstr, crvstr, tangentcrv, normcrv );
}

void XSec::GetTanNormCrv( double theta, double angstr, double crvstr,
        piecewise_curve_type &tangentcrv, piecewise_curve_type &normcrv )
{
    double tmin, tmax;
    m_TransformedCurve.GetCurve().get_parameter_min( tmin );
    m_TransformedCurve.GetCurve().get_parameter_max( tmax );

    vector< double > ts = { tmin, tmax };
    vector< double > thetas( 2, theta );
    vector< double > angstrs( 2, angstr );
    vector< double > crvstrs( 2, crvstr );

    GetTanNormCrv( ts, thetas, angstrs, crvstrs, tangentcrv, normcrv );
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

        m_RefLength = fxs->m_RefLength();
    }
}

//==========================================================================//
//==========================================================================//
//==========================================================================//

//==== Default Constructor ====//
StackXSec::StackXSec( XSecCurve *xsc, bool use_left ) : XSec( xsc, use_left)
{
    m_Type = XSEC_STACK;

    m_XDelta.Init( "XDelta", m_GroupName, this,  0.0, -1.0e12, 1.0e12 );
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

    m_XCenterRot.Init( "m_XCenterRot", m_GroupName, this,  0.0, -1.0e12, 1.0e12 );
    m_XCenterRot.SetDescript( "X Center Of Rotation" );
    m_YCenterRot.Init( "m_YCenterRot", m_GroupName, this,  0.0, -1.0e12, 1.0e12 );
    m_YCenterRot.SetDescript( "Y Center Of Rotation" );
    m_ZCenterRot.Init( "m_ZCenterRot", m_GroupName, this,  0.0, -1.0e12, 1.0e12 );
    m_ZCenterRot.SetDescript( "Z Center Of Rotation" );


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

    m_TransformedCurve.OffsetX( m_XCenterRot() );
    m_TransformedCurve.OffsetY( m_YCenterRot() );
    m_TransformedCurve.OffsetZ( m_ZCenterRot() );

    m_TransformedCurve.Transform( m_Transform );

    m_TransformedCurve.OffsetX( -m_XCenterRot() );
    m_TransformedCurve.OffsetY( -m_YCenterRot() );
    m_TransformedCurve.OffsetZ( -m_ZCenterRot() );

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

