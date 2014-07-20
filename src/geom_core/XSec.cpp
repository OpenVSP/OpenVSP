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

    m_SectTessU.Init( "SectTess_U", m_GroupName, this, 6, 2,  100 );
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

void XSec::GetBasis( double t, Matrix4d &basis )
{
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
}

// Given a position along a curve t, and a desired surfce angle theta, calculate
// the tangent and normal unit vectors that will be required by the surface
// skinning algorithm.
void XSec::GetTanNormVec( double t, double theta, vec3d &tangent, vec3d &normal )
{
    Matrix4d basis;
    vec3d wdir, updir, pdir;
    Matrix4d rmat;

    GetBasis( t, basis );

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

        GetTanNormVec( crvts[i], -crvparm.x(), tangent, normal );

        tangent = tangent * crvparm.y();
        normal = normal * ( -crvparm.z() );

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

void XSec::GetAngStrCrv( double t, int irib,
        double &thetaL, double &strengthL, double &curvatureL,
        double &thetaR, double &strengthR, double &curvatureR,
        const VspSurf &surf )
{
    Matrix4d basis;
    vec3d wdir, updir, pdir;
    Matrix4d rmat;

    GetBasis( t, basis );

    // Pull out basis directions.
    basis.getBasis( wdir, updir, pdir );

    double uribL, uribR;
    double tol = 1.0e-9;

    if( irib > 0 ) uribL = irib - tol;
    else uribL = irib;

    if( irib < surf.GetNumSectU() ) uribR = irib + tol;
    else uribR = irib;

    vec3d tanL = surf.CompTanU( uribL, t );
    vec3d tanR = surf.CompTanU( uribR, t );

    thetaL = angle( pdir, tanL );
    strengthL = tanL.mag();

    thetaR = angle( pdir, tanR );
    strengthR = tanR.mag();

    vec3d uuL = surf.CompTanUU( uribL, t );
    vec3d uuR = surf.CompTanUU( uribR, t );

    Matrix4d basisL, basisR;
    basisL.initMat( basis.data() );
    basisR.initMat( basis.data() );

    // Rotate basis to specified slope.
    rmat.rotate( thetaL, updir );
    basisL.postMult( rmat.data() );

    rmat.rotate( thetaR, updir );
    basisR.postMult( rmat.data() );

    // Pull out desired normal and tangent directions.
    vec3d nL, uL, tL;
    vec3d nR, uR, tR;
    basisL.getBasis( nL, uL, tL);
    basisR.getBasis( nR, uR, tR);

    curvatureL = uuL.mag() * sgn( dot( uuL, nL ));
    curvatureR = uuR.mag() * sgn( dot( uuR, nR ));
}

//==========================================================================//
//==========================================================================//
//==========================================================================//

//==== Default Constructor ====//
SkinXSec::SkinXSec( XSecCurve *xsc, bool use_left ) : XSec( xsc, use_left)
{

    m_AllSymFlag.Init( "AllSym", m_GroupName, this, 1, 0, 1 );
    m_AllSymFlag.SetDescript( "Set all skinning parameters equal." );
    m_TBSymFlag.Init( "TBSym", m_GroupName, this, 1, 0, 1 );
    m_TBSymFlag.SetDescript( "Set top/bottom skinning parameters equal." );
    m_RLSymFlag.Init( "RLSym", m_GroupName, this, 1, 0, 1 );
    m_RLSymFlag.SetDescript( "Set left/right skinning parameters equal." );

    // Top spline parameters
    m_TopCont.Init( "ContinuityTop", m_GroupName, this, 0, 0, 2 );
    m_TopCont.SetDescript( "Skinning continuity enforced on top of curve" );

    m_TopLAngleSet.Init( "TopLAngleSet", m_GroupName, this, 0, 0, 1 );
    m_TopLStrengthSet.Init( "TopLStrengthSet", m_GroupName, this, 0, 0, 1 );
    m_TopLCurveSet.Init( "TopLCurveSet", m_GroupName, this, 0, 0, 1 );

    m_TopRAngleSet.Init( "TopRAngleSet", m_GroupName, this, 0, 0, 1 );
    m_TopRStrengthSet.Init( "TopRStrengthSet", m_GroupName, this, 0, 0, 1 );
    m_TopRCurveSet.Init( "TopRCurveSet", m_GroupName, this, 0, 0, 1 );

    m_TopLRAngleEq.Init( "TopLRAngleEq", m_GroupName, this, 0, 0, 1 );
    m_TopLRStrengthEq.Init( "TopLRStrengthEq", m_GroupName, this, 0, 0, 1 );
    m_TopLRCurveEq.Init( "TopLRCurveEq", m_GroupName, this, 0, 0, 1 );

    m_TopLAngle.Init( "TopLAngle", m_GroupName, this,  0.0, -180.0, 180.0 );
    m_TopLStrength.Init( "TopLStrength", m_GroupName, this,  1.0, 0.0, 10.0 );
    m_TopLCurve.Init( "TopLCurve", m_GroupName, this,  0.0, -10.0, 10.0 );

    m_TopRAngle.Init( "TopRAngle", m_GroupName, this,  0.0, -180.0, 180.0 );
    m_TopRStrength.Init( "TopRStrength", m_GroupName, this,  1.0, 0.0, 10.0 );
    m_TopRCurve.Init( "TopRCurve", m_GroupName, this,  0.0, -10.0, 10.0 );

    // Right spline parameters
    m_RightCont.Init( "ContinuityRight", m_GroupName, this, 0, 0, 2 );
    m_RightCont.SetDescript( "Skinning continuity enforced on right of curve" );

    m_RightLAngleSet.Init( "RightLAngleSet", m_GroupName, this, 0, 0, 1 );
    m_RightLStrengthSet.Init( "RightLStrengthSet", m_GroupName, this, 0, 0, 1 );
    m_RightLCurveSet.Init( "RightLCurveSet", m_GroupName, this, 0, 0, 1 );

    m_RightRAngleSet.Init( "RightRAngleSet", m_GroupName, this, 0, 0, 1 );
    m_RightRStrengthSet.Init( "RightRStrengthSet", m_GroupName, this, 0, 0, 1 );
    m_RightRCurveSet.Init( "RightRCurveSet", m_GroupName, this, 0, 0, 1 );

    m_RightLRAngleEq.Init( "RightLRAngleEq", m_GroupName, this, 0, 0, 1 );
    m_RightLRStrengthEq.Init( "RightLRStrengthEq", m_GroupName, this, 0, 0, 1 );
    m_RightLRCurveEq.Init( "RightLRCurveEq", m_GroupName, this, 0, 0, 1 );

    m_RightLAngle.Init( "RightLAngle", m_GroupName, this,  0.0, -180.0, 180.0 );
    m_RightLStrength.Init( "RightLStrength", m_GroupName, this,  1.0, 0.0, 10.0 );
    m_RightLCurve.Init( "RightLCurve", m_GroupName, this,  0.0, -10.0, 10.0 );

    m_RightRAngle.Init( "RightRAngle", m_GroupName, this,  0.0, -180.0, 180.0 );
    m_RightRStrength.Init( "RightRStrength", m_GroupName, this,  1.0, 0.0, 10.0 );
    m_RightRCurve.Init( "RightRCurve", m_GroupName, this,  0.0, -10.0, 10.0 );

    // Bottom spline parameters
    m_BottomCont.Init( "ContinuityBottom", m_GroupName, this, 0, 0, 2 );
    m_BottomCont.SetDescript( "Skinning continuity enforced on bottom of curve" );

    m_BottomLAngleSet.Init( "BottomLAngleSet", m_GroupName, this, 0, 0, 1 );
    m_BottomLStrengthSet.Init( "BottomLStrengthSet", m_GroupName, this, 0, 0, 1 );
    m_BottomLCurveSet.Init( "BottomLCurveSet", m_GroupName, this, 0, 0, 1 );

    m_BottomRAngleSet.Init( "BottomRAngleSet", m_GroupName, this, 0, 0, 1 );
    m_BottomRStrengthSet.Init( "BottomRStrengthSet", m_GroupName, this, 0, 0, 1 );
    m_BottomRCurveSet.Init( "BottomRCurveSet", m_GroupName, this, 0, 0, 1 );

    m_BottomLRAngleEq.Init( "BottomLRAngleEq", m_GroupName, this, 0, 0, 1 );
    m_BottomLRStrengthEq.Init( "BottomLRStrengthEq", m_GroupName, this, 0, 0, 1 );
    m_BottomLRCurveEq.Init( "BottomLRCurveEq", m_GroupName, this, 0, 0, 1 );

    m_BottomLAngle.Init( "BottomLAngle", m_GroupName, this,  0.0, -180.0, 180.0 );
    m_BottomLStrength.Init( "BottomLStrength", m_GroupName, this,  1.0, 0.0, 10.0 );
    m_BottomLCurve.Init( "BottomLCurve", m_GroupName, this,  0.0, -10.0, 10.0 );

    m_BottomRAngle.Init( "BottomRAngle", m_GroupName, this,  0.0, -180.0, 180.0 );
    m_BottomRStrength.Init( "BottomRStrength", m_GroupName, this,  1.0, 0.0, 10.0 );
    m_BottomRCurve.Init( "BottomRCurve", m_GroupName, this,  0.0, -10.0, 10.0 );

    // Left spline parameters
    m_LeftCont.Init( "ContinuityLeft", m_GroupName, this, 0, 0, 2 );
    m_LeftCont.SetDescript( "Skinning continuity enforced on left of curve" );

    m_LeftLAngleSet.Init( "LeftLAngleSet", m_GroupName, this, 0, 0, 1 );
    m_LeftLStrengthSet.Init( "LeftLStrengthSet", m_GroupName, this, 0, 0, 1 );
    m_LeftLCurveSet.Init( "LeftLCurveSet", m_GroupName, this, 0, 0, 1 );

    m_LeftRAngleSet.Init( "LeftRAngleSet", m_GroupName, this, 0, 0, 1 );
    m_LeftRStrengthSet.Init( "LeftRStrengthSet", m_GroupName, this, 0, 0, 1 );
    m_LeftRCurveSet.Init( "LeftRCurveSet", m_GroupName, this, 0, 0, 1 );

    m_LeftLRAngleEq.Init( "LeftLRAngleEq", m_GroupName, this, 0, 0, 1 );
    m_LeftLRStrengthEq.Init( "LeftLRStrengthEq", m_GroupName, this, 0, 0, 1 );
    m_LeftLRCurveEq.Init( "LeftLRCurveEq", m_GroupName, this, 0, 0, 1 );

    m_LeftLAngle.Init( "LeftLAngle", m_GroupName, this,  0.0, -180.0, 180.0 );
    m_LeftLStrength.Init( "LeftLStrength", m_GroupName, this,  1.0, 0.0, 10.0 );
    m_LeftLCurve.Init( "LeftLCurve", m_GroupName, this,  0.0, -10.0, 10.0 );

    m_LeftRAngle.Init( "LeftRAngle", m_GroupName, this,  0.0, -180.0, 180.0 );
    m_LeftRStrength.Init( "LeftRStrength", m_GroupName, this,  1.0, 0.0, 10.0 );
    m_LeftRCurve.Init( "LeftRCurve", m_GroupName, this,  0.0, -10.0, 10.0 );
}



void SkinXSec::CopySetValidate( IntParm &Cont,
    BoolParm &LAngleSet,
    BoolParm &LStrengthSet,
    BoolParm &LCurveSet,
    BoolParm &RAngleSet,
    BoolParm &RStrengthSet,
    BoolParm &RCurveSet,
    BoolParm &LRAngleEq,
    BoolParm &LRStrengthEq,
    BoolParm &LRCurveEq )
{
    Cont = m_TopCont();
    LAngleSet = m_TopLAngleSet();
    LStrengthSet = m_TopLStrengthSet();
    LCurveSet = m_TopLCurveSet();
    RAngleSet = m_TopRAngleSet();
    RStrengthSet = m_TopRStrengthSet();
    RCurveSet = m_TopRCurveSet();

    ValidateParms( Cont,
        LAngleSet,
        LStrengthSet,
        LCurveSet,
        RAngleSet,
        RStrengthSet,
        RCurveSet,
        LRAngleEq,
        LRStrengthEq,
        LRCurveEq );
}

void SkinXSec::ValidateParms( IntParm &Cont,
    BoolParm &LAngleSet,
    BoolParm &LStrengthSet,
    BoolParm &LCurveSet,
    BoolParm &RAngleSet,
    BoolParm &RStrengthSet,
    BoolParm &RCurveSet,
    BoolParm &LRAngleEq,
    BoolParm &LRStrengthEq,
    BoolParm &LRCurveEq )
{
    if ( LAngleSet() ) LStrengthSet = true;
    if ( !LAngleSet() ) LStrengthSet = false;

    if ( RAngleSet() ) RStrengthSet = true;
    if ( !RAngleSet() ) RStrengthSet = false;

    if ( Cont() == 2 )
    {
        if ( LCurveSet() )
        {
            LRCurveEq = true;
            RCurveSet = true;
        }
        else
        {
            LRCurveEq = false;
            RCurveSet = false;
        }
    }

    if ( Cont() >= 1 )
    {
        if ( LAngleSet() )
        {
            LRAngleEq = true;
            RAngleSet = true;
        }
        else
        {
            LRAngleEq = false;
            RAngleSet = false;
        }

        if ( LStrengthSet() )
        {
            LRStrengthEq = true;
            RStrengthSet = true;
        }
        else
        {
            LRStrengthEq = false;
            RStrengthSet = false;
        }

    }

    if ( LRAngleEq() )
    {
        RAngleSet = true;
        RStrengthSet = true;
        LAngleSet = true;
    }
    if ( LRStrengthEq() )
    {
        RStrengthSet = true;
        LStrengthSet = true;
    }
    if ( LRCurveEq() )
    {
        RCurveSet = true;
        LCurveSet = true;
    }
}

void SkinXSec::CrossValidateParms( BoolParm &topEq,
        BoolParm &rightEq,
        BoolParm &bottomEq,
        BoolParm &leftEq,
        BoolParm &topRSet,
        BoolParm &topLSet )
{
    if ( topEq() || rightEq() || bottomEq() || leftEq() )
    {
        topRSet = true;
        topLSet = true;
    }
}


void SkinXSec::ValidateParms( )
{
    CrossValidateParms( m_TopLRAngleEq,
            m_RightLRAngleEq,
            m_BottomLRAngleEq,
            m_LeftLRAngleEq,
            m_TopRAngleSet,
            m_TopLAngleSet );

    CrossValidateParms( m_TopLRStrengthEq,
            m_RightLRStrengthEq,
            m_BottomLRStrengthEq,
            m_LeftLRStrengthEq,
            m_TopRAngleSet,
            m_TopLAngleSet );

    CrossValidateParms( m_TopLRCurveEq,
            m_RightLRCurveEq,
            m_BottomLRCurveEq,
            m_LeftLRCurveEq,
            m_TopRCurveSet,
            m_TopLCurveSet );

    ValidateParms( m_TopCont,
               m_TopLAngleSet,
               m_TopLStrengthSet,
               m_TopLCurveSet,
               m_TopRAngleSet,
               m_TopRStrengthSet,
               m_TopRCurveSet,
               m_TopLRAngleEq,
               m_TopLRStrengthEq,
               m_TopLRCurveEq );

    CopySetValidate( m_RightCont,
               m_RightLAngleSet,
               m_RightLStrengthSet,
               m_RightLCurveSet,
               m_RightRAngleSet,
               m_RightRStrengthSet,
               m_RightRCurveSet,
               m_RightLRAngleEq,
               m_RightLRStrengthEq,
               m_RightLRCurveEq );

    CopySetValidate( m_BottomCont,
               m_BottomLAngleSet,
               m_BottomLStrengthSet,
               m_BottomLCurveSet,
               m_BottomRAngleSet,
               m_BottomRStrengthSet,
               m_BottomRCurveSet,
               m_BottomLRAngleEq,
               m_BottomLRStrengthEq,
               m_BottomLRCurveEq );

    CopySetValidate( m_LeftCont,
               m_LeftLAngleSet,
               m_LeftLStrengthSet,
               m_LeftLCurveSet,
               m_LeftRAngleSet,
               m_LeftRStrengthSet,
               m_LeftRCurveSet,
               m_LeftLRAngleEq,
               m_LeftLRStrengthEq,
               m_LeftLRCurveEq );

    if ( m_TopLRAngleEq() ) m_TopRAngle = m_TopLAngle();
    if ( m_TopLRStrengthEq() ) m_TopRStrength = m_TopLStrength();
    if ( m_TopLRCurveEq() ) m_TopRCurve = m_TopLCurve();

    if ( m_RightLRAngleEq() ) m_RightRAngle = m_RightLAngle();
    if ( m_RightLRStrengthEq() ) m_RightRStrength = m_RightLStrength();
    if ( m_RightLRCurveEq() ) m_RightRCurve = m_RightLCurve();

    if ( m_BottomLRAngleEq() ) m_BottomRAngle = m_BottomLAngle();
    if ( m_BottomLRStrengthEq() ) m_BottomRStrength = m_BottomLStrength();
    if ( m_BottomLRCurveEq() ) m_BottomRCurve = m_BottomLCurve();

    if ( m_RightLRAngleEq() ) m_RightRAngle = m_RightLAngle();
    if ( m_RightLRStrengthEq() ) m_RightRStrength = m_RightLStrength();
    if ( m_RightLRCurveEq() ) m_RightRCurve = m_RightLCurve();

    if ( m_AllSymFlag() )
    {
        m_RightCont = m_TopCont();
        m_RightLAngleSet = m_TopLAngleSet();
        m_RightLStrengthSet = m_TopLStrengthSet();
        m_RightLCurveSet = m_TopLCurveSet();
        m_RightRAngleSet = m_TopRAngleSet();
        m_RightRStrengthSet = m_TopRStrengthSet();
        m_RightRCurveSet = m_TopRCurveSet();
        m_RightLRAngleEq = m_TopLRAngleEq();
        m_RightLRStrengthEq = m_TopLRStrengthEq();
        m_RightLRCurveEq = m_TopLRCurveEq();
        m_RightLAngle = m_TopLAngle();
        m_RightLStrength = m_TopLStrength();
        m_RightLCurve = m_TopLCurve();
        m_RightRAngle = m_TopRAngle();
        m_RightRStrength = m_TopRStrength();
        m_RightRCurve = m_TopRCurve();

        m_BottomCont = m_TopCont();
        m_BottomLAngleSet = m_TopLAngleSet();
        m_BottomLStrengthSet = m_TopLStrengthSet();
        m_BottomLCurveSet = m_TopLCurveSet();
        m_BottomRAngleSet = m_TopRAngleSet();
        m_BottomRStrengthSet = m_TopRStrengthSet();
        m_BottomRCurveSet = m_TopRCurveSet();
        m_BottomLRAngleEq = m_TopLRAngleEq();
        m_BottomLRStrengthEq = m_TopLRStrengthEq();
        m_BottomLRCurveEq = m_TopLRCurveEq();
        m_BottomLAngle = m_TopLAngle();
        m_BottomLStrength = m_TopLStrength();
        m_BottomLCurve = m_TopLCurve();
        m_BottomRAngle = m_TopRAngle();
        m_BottomRStrength = m_TopRStrength();
        m_BottomRCurve = m_TopRCurve();

        m_LeftCont = m_TopCont();
        m_LeftLAngleSet = m_TopLAngleSet();
        m_LeftLStrengthSet = m_TopLStrengthSet();
        m_LeftLCurveSet = m_TopLCurveSet();
        m_LeftRAngleSet = m_TopRAngleSet();
        m_LeftRStrengthSet = m_TopRStrengthSet();
        m_LeftRCurveSet = m_TopRCurveSet();
        m_LeftLRAngleEq = m_TopLRAngleEq();
        m_LeftLRStrengthEq = m_TopLRStrengthEq();
        m_LeftLRCurveEq = m_TopLRCurveEq();
        m_LeftLAngle = m_TopLAngle();
        m_LeftLStrength = m_TopLStrength();
        m_LeftLCurve = m_TopLCurve();
        m_LeftRAngle = m_TopRAngle();
        m_LeftRStrength = m_TopRStrength();
        m_LeftRCurve = m_TopRCurve();
    }

    if ( m_TBSymFlag() )
    {
        m_BottomCont = m_TopCont();
        m_BottomLAngleSet = m_TopLAngleSet();
        m_BottomLStrengthSet = m_TopLStrengthSet();
        m_BottomLCurveSet = m_TopLCurveSet();
        m_BottomRAngleSet = m_TopRAngleSet();
        m_BottomRStrengthSet = m_TopRStrengthSet();
        m_BottomRCurveSet = m_TopRCurveSet();
        m_BottomLRAngleEq = m_TopLRAngleEq();
        m_BottomLRStrengthEq = m_TopLRStrengthEq();
        m_BottomLRCurveEq = m_TopLRCurveEq();
        m_BottomLAngle = m_TopLAngle();
        m_BottomLStrength = m_TopLStrength();
        m_BottomLCurve = m_TopLCurve();
        m_BottomRAngle = m_TopRAngle();
        m_BottomRStrength = m_TopRStrength();
        m_BottomRCurve = m_TopRCurve();
    }

    if ( m_RLSymFlag() )
    {
        m_LeftCont = m_RightCont();
        m_LeftLAngleSet = m_RightLAngleSet();
        m_LeftLStrengthSet = m_RightLStrengthSet();
        m_LeftLCurveSet = m_RightLCurveSet();
        m_LeftRAngleSet = m_RightRAngleSet();
        m_LeftRStrengthSet = m_RightRStrengthSet();
        m_LeftRCurveSet = m_RightRCurveSet();
        m_LeftLRAngleEq = m_RightLRAngleEq();
        m_LeftLRStrengthEq = m_RightLRStrengthEq();
        m_LeftLRCurveEq = m_RightLRCurveEq();
        m_LeftLAngle = m_RightLAngle();
        m_LeftLStrength = m_RightLStrength();
        m_LeftLCurve = m_RightLCurve();
        m_LeftRAngle = m_RightRAngle();
        m_LeftRStrength = m_RightRStrength();
        m_LeftRCurve = m_RightRCurve();
    }
}

rib_data_type SkinXSec::GetRib( bool first, bool last )
{
    ValidateParms( );

    rib_data_type rib;

    // First GetCurve() forces Update() call if needed;
    rib.set_f( GetCurve().GetCurve() );

    if( !first && !last )
        rib.set_continuity( ( rib_data_type::connection_continuity ) m_TopCont() );

    piecewise_curve_type tangentcrv;
    piecewise_curve_type normcrv;

    vector< double > angles( 5, 0.0 );
    vector< double > strengths( 5, 0.0 );
    vector< double > curves( 5, 0.0 );

    double scaleL =  GetLScale();
    double scaleR =  GetRScale();

    // Use 'wrong' side of first cross section to set right side.
    if ( first && ( m_TopLAngleSet() || m_TopLCurveSet() ) )
    {
        angles[0] = m_RightLAngle()*PI/180.0;
        angles[1] = m_BottomLAngle()*PI/180.0;
        angles[2] = m_LeftLAngle()*PI/180.0;
        angles[3] = m_TopLAngle()*PI/180.0;
        angles[4] = angles[0];

        strengths[0] = m_RightLStrength() * scaleR;
        strengths[1] = m_BottomLStrength() * scaleR;
        strengths[2] = m_LeftLStrength() * scaleR;
        strengths[3] = m_TopLStrength() * scaleR;
        strengths[4] = strengths[0];

        curves[0] = m_RightLCurve() * scaleR;
        curves[1] = m_BottomLCurve() * scaleR;
        curves[2] = m_LeftLCurve() * scaleR;
        curves[3] = m_TopLCurve() * scaleR;
        curves[4] = curves[0];

        GetTanNormCrv( angles, strengths, curves, tangentcrv, normcrv );

        if( m_TopLAngleSet() ) rib.set_right_fp( tangentcrv );
        if( m_TopLCurveSet() ) rib.set_right_fpp( normcrv );
    }

    // Set the left side of a rib.
    if( !first && ( m_TopLAngleSet() || m_TopLCurveSet() ) )
    {

        angles[0] = m_RightLAngle()*PI/180.0;
        angles[1] = m_BottomLAngle()*PI/180.0;
        angles[2] = m_LeftLAngle()*PI/180.0;
        angles[3] = m_TopLAngle()*PI/180.0;
        angles[4] = angles[0];

        strengths[0] = m_RightLStrength() * scaleL;
        strengths[1] = m_BottomLStrength() * scaleL;
        strengths[2] = m_LeftLStrength() * scaleL;
        strengths[3] = m_TopLStrength() * scaleL;
        strengths[4] = strengths[0];

        curves[0] = m_RightLCurve() * scaleL;
        curves[1] = m_BottomLCurve() * scaleL;
        curves[2] = m_LeftLCurve() * scaleL;
        curves[3] = m_TopLCurve() * scaleL;
        curves[4] = curves[0];

        GetTanNormCrv( angles, strengths, curves, tangentcrv, normcrv );

        if( m_TopLAngleSet() ) rib.set_left_fp( tangentcrv );
        if( m_TopLCurveSet() ) rib.set_left_fpp( normcrv );

    }

    // Set the right side of a rib.
    if( !first && !last && ( m_TopRAngleSet() || m_TopRCurveSet() ) )
    {
        angles[0] = m_RightRAngle()*PI/180.0;
        angles[1] = m_BottomRAngle()*PI/180.0;
        angles[2] = m_LeftRAngle()*PI/180.0;
        angles[3] = m_TopRAngle()*PI/180.0;
        angles[4] = angles[0];

        strengths[0] = m_RightRStrength() * scaleR;
        strengths[1] = m_BottomRStrength() * scaleR;
        strengths[2] = m_LeftRStrength() * scaleR;
        strengths[3] = m_TopRStrength() * scaleR;
        strengths[4] = strengths[0];

        curves[0] = m_RightRCurve() * scaleR;
        curves[1] = m_BottomRCurve() * scaleR;
        curves[2] = m_LeftRCurve() * scaleR;
        curves[3] = m_TopRCurve() * scaleR;
        curves[4] = curves[0];

        GetTanNormCrv( angles, strengths, curves, tangentcrv, normcrv );

        if( m_TopRAngleSet() ) rib.set_right_fp( tangentcrv );
        if( m_TopRCurveSet() ) rib.set_right_fpp( normcrv );
    }

    return rib;
}

void SkinXSec::SetUnsetParms( int irib, const VspSurf &surf )
{
    SetUnsetParms( 0.0, irib, surf,
             m_RightLAngleSet,
             m_RightLStrengthSet,
             m_RightLCurveSet,
             m_RightRAngleSet,
             m_RightRStrengthSet,
             m_RightRCurveSet,
             m_RightLAngle,
             m_RightLStrength,
             m_RightLCurve,
             m_RightRAngle,
             m_RightRStrength,
             m_RightRCurve );

    SetUnsetParms( 1.0, irib, surf,
             m_BottomLAngleSet,
             m_BottomLStrengthSet,
             m_BottomLCurveSet,
             m_BottomRAngleSet,
             m_BottomRStrengthSet,
             m_BottomRCurveSet,
             m_BottomLAngle,
             m_BottomLStrength,
             m_BottomLCurve,
             m_BottomRAngle,
             m_BottomRStrength,
             m_BottomRCurve );

    SetUnsetParms( 2.0, irib, surf,
             m_LeftLAngleSet,
             m_LeftLStrengthSet,
             m_LeftLCurveSet,
             m_LeftRAngleSet,
             m_LeftRStrengthSet,
             m_LeftRCurveSet,
             m_LeftLAngle,
             m_LeftLStrength,
             m_LeftLCurve,
             m_LeftRAngle,
             m_LeftRStrength,
             m_LeftRCurve );

    SetUnsetParms( 3.0, irib, surf,
             m_TopLAngleSet,
             m_TopLStrengthSet,
             m_TopLCurveSet,
             m_TopRAngleSet,
             m_TopRStrengthSet,
             m_TopRCurveSet,
             m_TopLAngle,
             m_TopLStrength,
             m_TopLCurve,
             m_TopRAngle,
             m_TopRStrength,
             m_TopRCurve );

}

void SkinXSec::SetUnsetParms( double t, int irib, const VspSurf &surf,
        BoolParm &LAngleSet,
        BoolParm &LStrengthSet,
        BoolParm &LCurveSet,
        BoolParm &RAngleSet,
        BoolParm &RStrengthSet,
        BoolParm &RCurveSet,
        Parm &LAngle,
        Parm &LStrength,
        Parm &LCurve,
        Parm &RAngle,
        Parm &RStrength,
        Parm &RCurve )
{
    double thetaL, strengthL, curvatureL;
    double thetaR, strengthR, curvatureR;

    GetAngStrCrv(  t,  irib,
        thetaL, strengthL, curvatureL,
        thetaR, strengthR, curvatureR,
        surf );

    double scaleL =  GetLScale();
    double scaleR =  GetRScale();

    if( !LAngleSet() ) LAngle = thetaL*180.0/PI;
    if( !LStrengthSet() ) LStrength = strengthL/scaleL;
    if( !LCurveSet() ) LCurve = curvatureL/scaleL;

    if( !RAngleSet() ) RAngle = thetaR*180.0/PI;
    if( !RStrengthSet() ) RStrength = strengthR/scaleR;
    if( !RCurveSet() ) RCurve = curvatureR/scaleR;
}

//==========================================================================//
//==========================================================================//
//==========================================================================//

//==== Default Constructor ====//
FuseXSec::FuseXSec( XSecCurve *xsc, bool use_left ) : SkinXSec( xsc, use_left)
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

double FuseXSec::GetLScale()
{
	return m_RefLength();
}

double FuseXSec::GetRScale()
{
	return m_RefLength();
}

//==========================================================================//
//==========================================================================//
//==========================================================================//

//==== Default Constructor ====//
StackXSec::StackXSec( XSecCurve *xsc, bool use_left ) : SkinXSec( xsc, use_left)
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

double StackXSec::GetLScale()
{
    XSecSurf* xsecsurf = (XSecSurf*) GetParentContainerPtr();
    int indx = xsecsurf->FindXSecIndex( m_ID );

    double dx, dy, dz;

    if( indx == 0 )
    {
        StackXSec* prevxs = (StackXSec*) xsecsurf->FindXSec( indx + 1);
        if( prevxs )
        {
            dx = prevxs->m_XDelta();
            dy = prevxs->m_YDelta();
            dz = prevxs->m_ZDelta();
        }
    }
    else
    {
        dx = m_XDelta();
        dy = m_YDelta();
        dz = m_ZDelta();
    }

    return sqrt( dx*dx + dy*dy + dz*dz );
}

double StackXSec::GetRScale()
{
    XSecSurf* xsecsurf = (XSecSurf*) GetParentContainerPtr();
    int indx = xsecsurf->FindXSecIndex( m_ID );

    double dx, dy, dz;

    if( indx < (xsecsurf->NumXSec() - 1) )
    {
        StackXSec* prevxs = (StackXSec*) xsecsurf->FindXSec( indx + 1);
        if( prevxs )
        {
            dx = prevxs->m_XDelta();
            dy = prevxs->m_YDelta();
            dz = prevxs->m_ZDelta();
        }
    }
    else
    {
        dx = m_XDelta();
        dy = m_YDelta();
        dz = m_ZDelta();
    }

    return sqrt( dx*dx + dy*dy + dz*dz );
}
