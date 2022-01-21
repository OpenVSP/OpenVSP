//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//

// XSec.cpp: implementation of the XSec class.
//
//////////////////////////////////////////////////////////////////////

#include "XSec.h"
#include "Geom.h"
#include "ParmMgr.h"
#include "StlHelper.h"
#include <float.h>

using std::string;
using namespace vsp;

//==== Default Constructor ====//
XSec::XSec( XSecCurve *xsc )
{
    if ( xsc == NULL )
    {
        //==== Create A Default Curve ====//
        xsc = new CircleXSec();
    }

    m_XSCurve = xsc;

    if ( m_XSCurve  )
    {
        m_XSCurve->SetParentContainer( m_ID );
    }

    m_Type = -1;

    m_GroupName = "XSec";
    m_GroupSuffix = -1;

    m_SectTessU.Init( "SectTess_U", m_GroupName, this, 6, 2,  1000 );
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

        //==== Notify Parent Container (XSecSurf) ====//
        ParmContainer* pc = GetParentContainerPtr();
        if ( pc )
        {
            pc->ParmChanged( parm_ptr, type );
        }

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

vector< vec3d > XSec::GetDrawLines( Matrix4d &transMat  )
{
    vector< vec3d > lines;

    VspCurve curve = GetCurve();
    curve.Transform( transMat );
    curve.TessAdapt( lines, 1e-2, 10 );

    return lines;
}

void XSec::SetXSecCurve( XSecCurve* xs_crv )
{
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
    string lastreset = ParmMgr.ResetRemapID();
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
    ParmMgr.ResetRemapID( lastreset );
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

// This is the principal direction component of a principal direction unit vector transformed by m_Transform.
// This will give the ratio of a component's area when projected into the principal direction.
double XSec::GetProjectionCosine()
{
    XSecSurf* xsecsurf = (XSecSurf*) GetParentContainerPtr();
    int pdir = xsecsurf->GetPrincipalDirection();

    // vec3d pvec;
    // pvec.v[pdir] = 1;
    // vec3d xvec = m_Transform.xformnorm( pvec );
    // return xvec.v[pdir];

    // The below is equivalent to the above.  The above follows from the definition and is hopefully more intuitive.
    // The below is clearly the most efficient way to get this information.
    return m_Transform.data()[ pdir * 5 ];
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

    double tmin(m_TransformedCurve.GetCurve().get_parameter_min()), tmax(m_TransformedCurve.GetCurve().get_parameter_max());

    // Rotate basis around principal direction to point along curve
    // according to parameter and assumed circle.
    Matrix4d rmat;
    rmat.rotate( 2.0*PI*(t-tmin)/(tmax-tmin), pdir );
    basis.postMult( rmat.data() );
}

// Given a position along a curve t, and a desired surface angle theta, calculate
// the tangent and normal unit vectors that will be required by the surface
// skinning algorithm.

// TODO: XSec::GetAngStrCrv and XSec::GetTanNormVec should be refactored
// such that the core math performed by each is isolated (like GetTanNormVec) and they
// could be unit tested to behave as perfect inverses of one another.
//
// Currently, they appear to give wrong results in cases with nonzero
// theta and phi.

void XSec::GetTanNormVec( double t, double theta, double phi, vec3d &tangent, vec3d &normal )
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
    rmat.loadIdentity();
    rmat.rotate( phi, wdir );
    basis.postMult( rmat.data() );

    // Pull out desired normal and tangent directions.
    basis.getBasis( normal, updir, tangent);
}

// Given a vector of positions along a curve ts, desired surface angles thetas,
// angle strengths angstr, and curvature strengths crvstr, calculate the tangent
// and normal vector curves that will be required by the skinning algorithm.
void XSec::GetTanNormCrv( const vector< double > &ts, const vector< double > &thetas,
        const vector< double > &phis,
        const vector< double > &angstr, const vector< double > &crvstr,
        piecewise_curve_type &tangentcrv, piecewise_curve_type &normcrv )
{
    unsigned int nts = ts.size();

    // Set up cubic spline of desired controls
    piecewise_cubic_spline_creator_type pcc( nts - 1 );
    pcc.set_t0( ts[0] );
    for ( unsigned int i = 0; i < nts - 1; ++i)
    {
        pcc.set_segment_dt( ts[i+1] - ts[i], i );
    }

    vector< curve_point_type > pts(nts), phipts(nts);
    for( unsigned int i = 0; i < nts; i++ )
    {
        pts[i] << thetas[i], angstr[i], crvstr[i];
        phipts[i] << phis[i], 0.0, 0.0;
    }
    pcc.set_closed_cubic_spline( pts.begin() );

    // Build control curve.
    piecewise_curve_type crvcntrl, crvphi;
    pcc.create( crvcntrl );

    pcc.set_closed_cubic_spline( phipts.begin() );
    pcc.create( crvphi );


    // Evaluate control curve at all the piecewise endpoints
    // used to define the XSecCurve.

    // Parameters that define the XSecCurve
    vector< double > crvts;

    GetCurve().GetCurve().get_pmap( crvts );

    int ntcrv = crvts.size();

    // Evaluate controls and build tan & norm vectors at piecewise endpoints
    vector< curve_point_type > tanpts(ntcrv), nrmpts(ntcrv);
    for( unsigned int i = 0; i < ntcrv; i++ )
    {
        curve_point_type crvparm = crvcntrl.f( crvts[i] );
        curve_point_type crvphiparm = crvphi.f( crvts[i] );

        vec3d tangent, normal;

        GetTanNormVec( crvts[i], -crvparm.x(), -crvphiparm.x(), tangent, normal );

        tangent = tangent * crvparm.y();
        normal = normal * crvparm.z();

        tanpts[i] << tangent.v[0], tangent.v[1], tangent.v[2];
        nrmpts[i] << normal.v[0], normal.v[1], normal.v[2];
    }

    pcc.set_number_segments( ntcrv - 1 );
    pcc.set_t0( crvts[0] );
    for ( unsigned int i = 0; i < ntcrv - 1; ++i)
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
        const vector< double > &phis,
        const vector< double > &angstr, const vector< double > &crvstr,
        piecewise_curve_type &tangentcrv, piecewise_curve_type &normcrv )
{
    vector< double > ts;
    for ( int i = 0 ; i < 5 ; i++ )
        ts.push_back( (double)i );

    assert( thetas.size() == 5 );
    assert( phis.size() == 5 );
    assert( angstr.size() == 5 );
    assert( crvstr.size() == 5 );

    GetTanNormCrv( ts, thetas, phis, angstr, crvstr, tangentcrv, normcrv );
}

void XSec::GetAngStrCrv( double t, int irib,
        double &thetaL, double &phiL, double &strengthL, double &curvatureL,
        double &thetaR, double &phiR, double &strengthR, double &curvatureR,
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

    if( irib < surf.GetUMax() ) uribR = irib + tol;
    else uribR = irib;

    vec3d tanL = surf.CompTanU( uribL, t );
    vec3d tanR = surf.CompTanU( uribR, t );

    thetaL =  (PI/2.0) - signed_angle( tanL, wdir, updir );
    phiL = signed_angle( updir, tanL, wdir ) - (PI/2.0);
    strengthL = tanL.mag();

    thetaR = (PI/2.0) - signed_angle( tanR, wdir, updir );
    phiR = signed_angle( updir, tanR, wdir ) - (PI/2.0);
    strengthR = tanR.mag();

    vec3d uuL = surf.CompTanUU( uribL, t );
    vec3d uuR = surf.CompTanUU( uribR, t );

    Matrix4d basisL, basisR;
    basisL.initMat( basis.data() );
    basisR.initMat( basis.data() );

    // Rotate basis to specified slope.
    rmat.rotate( thetaL, updir );
    basisL.postMult( rmat.data() );
    rmat.loadIdentity();
    rmat.rotate( phiL, wdir );
    basisL.postMult( rmat.data() );

    rmat.loadIdentity();
    rmat.rotate( thetaR, updir );
    basisR.postMult( rmat.data() );
    rmat.loadIdentity();
    rmat.rotate( phiR, wdir );
    basisR.postMult( rmat.data() );

    // Pull out desired normal and tangent directions.
    vec3d nL, uL, tL;
    vec3d nR, uR, tR;
    basisL.getBasis( nL, uL, tL);
    basisR.getBasis( nR, uR, tR);

    curvatureL = uuL.mag() * sgn( dot( uuL, nL ));
    curvatureR = uuR.mag() * sgn( dot( uuR, nR ));
}

void XSec::ReadV2FileFuse2( xmlNodePtr &root )
{
    m_SectTessU  = XmlUtil::FindInt( root, "Num_Sect_Interp_1", m_SectTessU() );

    double numSectInterp2 = 0;
    numSectInterp2  = XmlUtil::FindInt( root, "Num_Sect_Interp_2", numSectInterp2 );

    m_XSCurve->ReadV2FileFuse2( root );
}

void XSec::ReadV2FileFuse1( xmlNodePtr &root )
{
    m_SectTessU  = XmlUtil::FindInt( root, "Num_Sect_Interp_1", m_SectTessU() );

    double numSectInterp2 = 0;
    numSectInterp2  = XmlUtil::FindInt( root, "Num_Sect_Interp_2", numSectInterp2 );

    xmlNodePtr omlNode = XmlUtil::GetNode( root, "OML_Parms", 0 );

    m_XSCurve->ReadV2FileFuse2( omlNode );
}

EditCurveXSec* XSec::ConvertToEdit()
{
    EditCurveXSec* xscrv_ptr = m_XSCurve->ConvertToEdit();

    if ( xscrv_ptr && xscrv_ptr != m_XSCurve )
    {
        SetXSecCurve( xscrv_ptr );
    }

    return xscrv_ptr;
}

//==========================================================================//
//==========================================================================//
//==========================================================================//

//==== Default Constructor ====//
SkinXSec::SkinXSec( XSecCurve *xsc ) : XSec( xsc)
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
    m_TopLSlewSet.Init( "TopLSlewSet", m_GroupName, this, 0, 0, 1 );
    m_TopLStrengthSet.Init( "TopLStrengthSet", m_GroupName, this, 0, 0, 1 );
    m_TopLCurveSet.Init( "TopLCurveSet", m_GroupName, this, 0, 0, 1 );

    m_TopRAngleSet.Init( "TopRAngleSet", m_GroupName, this, 0, 0, 1 );
    m_TopRSlewSet.Init( "TopRSlewSet", m_GroupName, this, 0, 0, 1 );
    m_TopRStrengthSet.Init( "TopRStrengthSet", m_GroupName, this, 0, 0, 1 );
    m_TopRCurveSet.Init( "TopRCurveSet", m_GroupName, this, 0, 0, 1 );

    m_TopLRAngleEq.Init( "TopLRAngleEq", m_GroupName, this, 0, 0, 1 );
    m_TopLRSlewEq.Init( "TopLRSlewEq", m_GroupName, this, 0, 0, 1 );
    m_TopLRStrengthEq.Init( "TopLRStrengthEq", m_GroupName, this, 0, 0, 1 );
    m_TopLRCurveEq.Init( "TopLRCurveEq", m_GroupName, this, 0, 0, 1 );

    m_TopLAngle.Init( "TopLAngle", m_GroupName, this,  0.0, -360.0, 360.0 );
    m_TopLSlew.Init( "TopLSlew", m_GroupName, this,  0.0, -360.0, 360.0 );
    m_TopLStrength.Init( "TopLStrength", m_GroupName, this,  1.0, 0.0, 1e12 );
    m_TopLCurve.Init( "TopLCurve", m_GroupName, this,  0.0, -1e12, 1e12 );

    m_TopRAngle.Init( "TopRAngle", m_GroupName, this,  0.0, -360.0, 360.0 );
    m_TopRSlew.Init( "TopRSlew", m_GroupName, this,  0.0, -360.0, 360.0 );
    m_TopRStrength.Init( "TopRStrength", m_GroupName, this,  1.0, 0.0, 1e12 );
    m_TopRCurve.Init( "TopRCurve", m_GroupName, this,  0.0, -1e12, 1e12 );

    // Right spline parameters
    m_RightCont.Init( "ContinuityRight", m_GroupName, this, 0, 0, 2 );
    m_RightCont.SetDescript( "Skinning continuity enforced on right of curve" );

    m_RightLAngleSet.Init( "RightLAngleSet", m_GroupName, this, 0, 0, 1 );
    m_RightLSlewSet.Init( "RightLSlewSet", m_GroupName, this, 0, 0, 1 );
    m_RightLStrengthSet.Init( "RightLStrengthSet", m_GroupName, this, 0, 0, 1 );
    m_RightLCurveSet.Init( "RightLCurveSet", m_GroupName, this, 0, 0, 1 );

    m_RightRAngleSet.Init( "RightRAngleSet", m_GroupName, this, 0, 0, 1 );
    m_RightRSlewSet.Init( "RightRSlewSet", m_GroupName, this, 0, 0, 1 );
    m_RightRStrengthSet.Init( "RightRStrengthSet", m_GroupName, this, 0, 0, 1 );
    m_RightRCurveSet.Init( "RightRCurveSet", m_GroupName, this, 0, 0, 1 );

    m_RightLRAngleEq.Init( "RightLRAngleEq", m_GroupName, this, 0, 0, 1 );
    m_RightLRSlewEq.Init( "RightLRSlewEq", m_GroupName, this, 0, 0, 1 );
    m_RightLRStrengthEq.Init( "RightLRStrengthEq", m_GroupName, this, 0, 0, 1 );
    m_RightLRCurveEq.Init( "RightLRCurveEq", m_GroupName, this, 0, 0, 1 );

    m_RightLAngle.Init( "RightLAngle", m_GroupName, this,  0.0, -360.0, 360.0 );
    m_RightLSlew.Init( "RightLSlew", m_GroupName, this,  0.0, -360.0, 360.0 );
    m_RightLStrength.Init( "RightLStrength", m_GroupName, this,  1.0, 0.0, 1e12 );
    m_RightLCurve.Init( "RightLCurve", m_GroupName, this,  0.0, -1e12, 1e12 );

    m_RightRAngle.Init( "RightRAngle", m_GroupName, this,  0.0, -360.0, 360.0 );
    m_RightRSlew.Init( "RightRSlew", m_GroupName, this,  0.0, -360.0, 360.0 );
    m_RightRStrength.Init( "RightRStrength", m_GroupName, this,  1.0, 0.0, 1e12 );
    m_RightRCurve.Init( "RightRCurve", m_GroupName, this,  0.0, -1e12, 1e12 );

    // Bottom spline parameters
    m_BottomCont.Init( "ContinuityBottom", m_GroupName, this, 0, 0, 2 );
    m_BottomCont.SetDescript( "Skinning continuity enforced on bottom of curve" );

    m_BottomLAngleSet.Init( "BottomLAngleSet", m_GroupName, this, 0, 0, 1 );
    m_BottomLSlewSet.Init( "BottomLSlewSet", m_GroupName, this, 0, 0, 1 );
    m_BottomLStrengthSet.Init( "BottomLStrengthSet", m_GroupName, this, 0, 0, 1 );
    m_BottomLCurveSet.Init( "BottomLCurveSet", m_GroupName, this, 0, 0, 1 );

    m_BottomRAngleSet.Init( "BottomRAngleSet", m_GroupName, this, 0, 0, 1 );
    m_BottomRSlewSet.Init( "BottomRSlewSet", m_GroupName, this, 0, 0, 1 );
    m_BottomRStrengthSet.Init( "BottomRStrengthSet", m_GroupName, this, 0, 0, 1 );
    m_BottomRCurveSet.Init( "BottomRCurveSet", m_GroupName, this, 0, 0, 1 );

    m_BottomLRAngleEq.Init( "BottomLRAngleEq", m_GroupName, this, 0, 0, 1 );
    m_BottomLRSlewEq.Init( "BottomLRSlewEq", m_GroupName, this, 0, 0, 1 );
    m_BottomLRStrengthEq.Init( "BottomLRStrengthEq", m_GroupName, this, 0, 0, 1 );
    m_BottomLRCurveEq.Init( "BottomLRCurveEq", m_GroupName, this, 0, 0, 1 );

    m_BottomLAngle.Init( "BottomLAngle", m_GroupName, this,  0.0, -360.0, 360.0 );
    m_BottomLSlew.Init( "BottomLSlew", m_GroupName, this,  0.0, -360.0, 360.0 );
    m_BottomLStrength.Init( "BottomLStrength", m_GroupName, this,  1.0, 0.0, 1e12 );
    m_BottomLCurve.Init( "BottomLCurve", m_GroupName, this,  0.0, -1e12, 1e12 );

    m_BottomRAngle.Init( "BottomRAngle", m_GroupName, this,  0.0, -360.0, 360.0 );
    m_BottomRSlew.Init( "BottomRSlew", m_GroupName, this,  0.0, -360.0, 360.0 );
    m_BottomRStrength.Init( "BottomRStrength", m_GroupName, this,  1.0, 0.0, 1e12 );
    m_BottomRCurve.Init( "BottomRCurve", m_GroupName, this,  0.0, -1e12, 1e12 );

    // Left spline parameters
    m_LeftCont.Init( "ContinuityLeft", m_GroupName, this, 0, 0, 2 );
    m_LeftCont.SetDescript( "Skinning continuity enforced on left of curve" );

    m_LeftLAngleSet.Init( "LeftLAngleSet", m_GroupName, this, 0, 0, 1 );
    m_LeftLSlewSet.Init( "LeftLSlewSet", m_GroupName, this, 0, 0, 1 );
    m_LeftLStrengthSet.Init( "LeftLStrengthSet", m_GroupName, this, 0, 0, 1 );
    m_LeftLCurveSet.Init( "LeftLCurveSet", m_GroupName, this, 0, 0, 1 );

    m_LeftRAngleSet.Init( "LeftRAngleSet", m_GroupName, this, 0, 0, 1 );
    m_LeftRSlewSet.Init( "LeftRSlewSet", m_GroupName, this, 0, 0, 1 );
    m_LeftRStrengthSet.Init( "LeftRStrengthSet", m_GroupName, this, 0, 0, 1 );
    m_LeftRCurveSet.Init( "LeftRCurveSet", m_GroupName, this, 0, 0, 1 );

    m_LeftLRAngleEq.Init( "LeftLRAngleEq", m_GroupName, this, 0, 0, 1 );
    m_LeftLRSlewEq.Init( "LeftLRSlewEq", m_GroupName, this, 0, 0, 1 );
    m_LeftLRStrengthEq.Init( "LeftLRStrengthEq", m_GroupName, this, 0, 0, 1 );
    m_LeftLRCurveEq.Init( "LeftLRCurveEq", m_GroupName, this, 0, 0, 1 );

    m_LeftLAngle.Init( "LeftLAngle", m_GroupName, this,  0.0, -360.0, 360.0 );
    m_LeftLSlew.Init( "LeftLSlew", m_GroupName, this,  0.0, -360.0, 360.0 );
    m_LeftLStrength.Init( "LeftLStrength", m_GroupName, this,  1.0, 0.0, 1e12 );
    m_LeftLCurve.Init( "LeftLCurve", m_GroupName, this,  0.0, -1e12, 1e12 );

    m_LeftRAngle.Init( "LeftRAngle", m_GroupName, this,  0.0, -360.0, 360.0 );
    m_LeftRSlew.Init( "LeftRSlew", m_GroupName, this,  0.0, -360.0, 360.0 );
    m_LeftRStrength.Init( "LeftRStrength", m_GroupName, this,  1.0, 0.0, 1e12 );
    m_LeftRCurve.Init( "LeftRCurve", m_GroupName, this,  0.0, -1e12, 1e12 );
}



void SkinXSec::CopySetValidate( IntParm &Cont,
    BoolParm &LAngleSet,
    BoolParm &LSlewSet,
    BoolParm &LStrengthSet,
    BoolParm &LCurveSet,
    BoolParm &RAngleSet,
    BoolParm &RSlewSet,
    BoolParm &RStrengthSet,
    BoolParm &RCurveSet,
    BoolParm &LRAngleEq,
    BoolParm &LRSlewEq,
    BoolParm &LRStrengthEq,
    BoolParm &LRCurveEq )
{
    Cont = m_TopCont();
    LAngleSet = m_TopLAngleSet();
    LSlewSet = m_TopLSlewSet();
    LStrengthSet = m_TopLStrengthSet();
    LCurveSet = m_TopLCurveSet();
    RAngleSet = m_TopRAngleSet();
    RSlewSet = m_TopRSlewSet();
    RStrengthSet = m_TopRStrengthSet();
    RCurveSet = m_TopRCurveSet();

    ValidateParms( Cont,
        LAngleSet,
        LSlewSet,
        LStrengthSet,
        LCurveSet,
        RAngleSet,
        RSlewSet,
        RStrengthSet,
        RCurveSet,
        LRAngleEq,
        LRSlewEq,
        LRStrengthEq,
        LRCurveEq );
}

void SkinXSec::ValidateParms( IntParm &Cont,
    BoolParm &LAngleSet,
    BoolParm &LSlewSet,
    BoolParm &LStrengthSet,
    BoolParm &LCurveSet,
    BoolParm &RAngleSet,
    BoolParm &RSlewSet,
    BoolParm &RStrengthSet,
    BoolParm &RCurveSet,
    BoolParm &LRAngleEq,
    BoolParm &LRSlewEq,
    BoolParm &LRStrengthEq,
    BoolParm &LRCurveEq )
{
    LStrengthSet = LAngleSet();
    RStrengthSet = RAngleSet();
    LSlewSet = LAngleSet();
    RSlewSet = RAngleSet();

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

        if ( LSlewSet() )
        {
            LRSlewEq = true;
            RSlewSet = true;
        }
        else
        {
            LRSlewEq = false;
            RSlewSet = false;
        }
    }

    if ( LRAngleEq() )
    {
        RAngleSet = true;
        RSlewSet = true;
        RStrengthSet = true;
        LAngleSet = true;
    }
    if ( LRSlewEq() )
    {
        RSlewSet = true;
        LSlewSet = true;
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
        BoolParm &topLSet,
        bool CX )
{
    if( !CX )
    {
        if ( topEq() || rightEq() || bottomEq() || leftEq() )
        {
            topRSet = true;
            topLSet = true;
        }
    }
    else
    {
        topRSet = topLSet();
        topEq = topLSet();
        rightEq = topLSet();
        bottomEq = topLSet();
        leftEq = topLSet();
    }
}


void SkinXSec::ValidateParms( )
{
    CrossValidateParms( m_TopLRAngleEq,
            m_RightLRAngleEq,
            m_BottomLRAngleEq,
            m_LeftLRAngleEq,
            m_TopRAngleSet,
            m_TopLAngleSet,
            m_TopCont() >= 1 );

    CrossValidateParms( m_TopLRSlewEq,
            m_RightLRSlewEq,
            m_BottomLRSlewEq,
            m_LeftLRSlewEq,
            m_TopRSlewSet,
            m_TopLSlewSet,
            m_TopCont() >= 1 );

    CrossValidateParms( m_TopLRStrengthEq,
            m_RightLRStrengthEq,
            m_BottomLRStrengthEq,
            m_LeftLRStrengthEq,
            m_TopRAngleSet,
            m_TopLAngleSet,
            m_TopCont() >= 1 );

    CrossValidateParms( m_TopLRCurveEq,
            m_RightLRCurveEq,
            m_BottomLRCurveEq,
            m_LeftLRCurveEq,
            m_TopRCurveSet,
            m_TopLCurveSet,
            m_TopCont() >= 2 );

    ValidateParms( m_TopCont,
               m_TopLAngleSet,
               m_TopLSlewSet,
               m_TopLStrengthSet,
               m_TopLCurveSet,
               m_TopRAngleSet,
               m_TopRSlewSet,
               m_TopRStrengthSet,
               m_TopRCurveSet,
               m_TopLRAngleEq,
               m_TopLRSlewEq,
               m_TopLRStrengthEq,
               m_TopLRCurveEq );

    CopySetValidate( m_RightCont,
               m_RightLAngleSet,
               m_RightLSlewSet,
               m_RightLStrengthSet,
               m_RightLCurveSet,
               m_RightRAngleSet,
               m_RightRSlewSet,
               m_RightRStrengthSet,
               m_RightRCurveSet,
               m_RightLRAngleEq,
               m_RightLRSlewEq,
               m_RightLRStrengthEq,
               m_RightLRCurveEq );

    CopySetValidate( m_BottomCont,
               m_BottomLAngleSet,
               m_BottomLSlewSet,
               m_BottomLStrengthSet,
               m_BottomLCurveSet,
               m_BottomRAngleSet,
               m_BottomRSlewSet,
               m_BottomRStrengthSet,
               m_BottomRCurveSet,
               m_BottomLRAngleEq,
               m_BottomLRSlewEq,
               m_BottomLRStrengthEq,
               m_BottomLRCurveEq );

    CopySetValidate( m_LeftCont,
               m_LeftLAngleSet,
               m_LeftLSlewSet,
               m_LeftLStrengthSet,
               m_LeftLCurveSet,
               m_LeftRAngleSet,
               m_LeftRSlewSet,
               m_LeftRStrengthSet,
               m_LeftRCurveSet,
               m_LeftLRAngleEq,
               m_LeftLRSlewEq,
               m_LeftLRStrengthEq,
               m_LeftLRCurveEq );

    if ( m_TopLRAngleEq() ) m_TopRAngle = m_TopLAngle();
    if ( m_TopLRSlewEq() ) m_TopRSlew = m_TopLSlew();
    if ( m_TopLRStrengthEq() ) m_TopRStrength = m_TopLStrength();
    if ( m_TopLRCurveEq() ) m_TopRCurve = m_TopLCurve();

    if ( m_RightLRAngleEq() ) m_RightRAngle = m_RightLAngle();
    if ( m_RightLRSlewEq() ) m_RightRSlew = m_RightLSlew();
    if ( m_RightLRStrengthEq() ) m_RightRStrength = m_RightLStrength();
    if ( m_RightLRCurveEq() ) m_RightRCurve = m_RightLCurve();

    if ( m_BottomLRAngleEq() ) m_BottomRAngle = m_BottomLAngle();
    if ( m_BottomLRSlewEq() ) m_BottomRSlew = m_BottomLSlew();
    if ( m_BottomLRStrengthEq() ) m_BottomRStrength = m_BottomLStrength();
    if ( m_BottomLRCurveEq() ) m_BottomRCurve = m_BottomLCurve();

    if ( m_LeftLRAngleEq() ) m_LeftRAngle = m_LeftLAngle();
    if ( m_LeftLRSlewEq() ) m_LeftRSlew = m_LeftLSlew();
    if ( m_LeftLRStrengthEq() ) m_LeftRStrength = m_LeftLStrength();
    if ( m_LeftLRCurveEq() ) m_LeftRCurve = m_LeftLCurve();

    if ( m_AllSymFlag() )
    {
        m_RightCont = m_TopCont();
        m_RightLAngleSet = m_TopLAngleSet();
        m_RightLSlewSet = m_TopLSlewSet();
        m_RightLStrengthSet = m_TopLStrengthSet();
        m_RightLCurveSet = m_TopLCurveSet();
        m_RightRAngleSet = m_TopRAngleSet();
        m_RightRSlewSet = m_TopRSlewSet();
        m_RightRStrengthSet = m_TopRStrengthSet();
        m_RightRCurveSet = m_TopRCurveSet();
        m_RightLRAngleEq = m_TopLRAngleEq();
        m_RightLRSlewEq = m_TopLRSlewEq();
        m_RightLRStrengthEq = m_TopLRStrengthEq();
        m_RightLRCurveEq = m_TopLRCurveEq();
        m_RightLAngle = m_TopLAngle();
        m_RightLSlew = m_TopLSlew();
        m_RightLStrength = m_TopLStrength();
        m_RightLCurve = m_TopLCurve();
        m_RightRAngle = m_TopRAngle();
        m_RightRSlew = m_TopRSlew();
        m_RightRStrength = m_TopRStrength();
        m_RightRCurve = m_TopRCurve();

        m_BottomCont = m_TopCont();
        m_BottomLAngleSet = m_TopLAngleSet();
        m_BottomLSlewSet = m_TopLSlewSet();
        m_BottomLStrengthSet = m_TopLStrengthSet();
        m_BottomLCurveSet = m_TopLCurveSet();
        m_BottomRAngleSet = m_TopRAngleSet();
        m_BottomRSlewSet = m_TopRSlewSet();
        m_BottomRStrengthSet = m_TopRStrengthSet();
        m_BottomRCurveSet = m_TopRCurveSet();
        m_BottomLRAngleEq = m_TopLRAngleEq();
        m_BottomLRSlewEq = m_TopLRSlewEq();
        m_BottomLRStrengthEq = m_TopLRStrengthEq();
        m_BottomLRCurveEq = m_TopLRCurveEq();
        m_BottomLAngle = m_TopLAngle();
        m_BottomLSlew = m_TopLSlew();
        m_BottomLStrength = m_TopLStrength();
        m_BottomLCurve = m_TopLCurve();
        m_BottomRAngle = m_TopRAngle();
        m_BottomRSlew = m_TopRSlew();
        m_BottomRStrength = m_TopRStrength();
        m_BottomRCurve = m_TopRCurve();

        m_LeftCont = m_TopCont();
        m_LeftLAngleSet = m_TopLAngleSet();
        m_LeftLSlewSet = m_TopLSlewSet();
        m_LeftLStrengthSet = m_TopLStrengthSet();
        m_LeftLCurveSet = m_TopLCurveSet();
        m_LeftRAngleSet = m_TopRAngleSet();
        m_LeftRSlewSet = m_TopRSlewSet();
        m_LeftRStrengthSet = m_TopRStrengthSet();
        m_LeftRCurveSet = m_TopRCurveSet();
        m_LeftLRAngleEq = m_TopLRAngleEq();
        m_LeftLRSlewEq = m_TopLRSlewEq();
        m_LeftLRStrengthEq = m_TopLRStrengthEq();
        m_LeftLRCurveEq = m_TopLRCurveEq();
        m_LeftLAngle = m_TopLAngle();
        m_LeftLSlew = m_TopLSlew();
        m_LeftLStrength = m_TopLStrength();
        m_LeftLCurve = m_TopLCurve();
        m_LeftRAngle = m_TopRAngle();
        m_LeftRSlew = m_TopRSlew();
        m_LeftRStrength = m_TopRStrength();
        m_LeftRCurve = m_TopRCurve();
    }

    if ( m_TBSymFlag() )
    {
        m_BottomCont = m_TopCont();
        m_BottomLAngleSet = m_TopLAngleSet();
        m_BottomLSlewSet = m_TopLSlewSet();
        m_BottomLStrengthSet = m_TopLStrengthSet();
        m_BottomLCurveSet = m_TopLCurveSet();
        m_BottomRAngleSet = m_TopRAngleSet();
        m_BottomRSlewSet = m_TopRSlewSet();
        m_BottomRStrengthSet = m_TopRStrengthSet();
        m_BottomRCurveSet = m_TopRCurveSet();
        m_BottomLRAngleEq = m_TopLRAngleEq();
        m_BottomLRSlewEq = m_TopLRSlewEq();
        m_BottomLRStrengthEq = m_TopLRStrengthEq();
        m_BottomLRCurveEq = m_TopLRCurveEq();
        m_BottomLAngle = m_TopLAngle();
        m_BottomLSlew = m_TopLSlew();
        m_BottomLStrength = m_TopLStrength();
        m_BottomLCurve = m_TopLCurve();
        m_BottomRAngle = m_TopRAngle();
        m_BottomRSlew = m_TopRSlew();
        m_BottomRStrength = m_TopRStrength();
        m_BottomRCurve = m_TopRCurve();
    }

    if ( m_RLSymFlag() )
    {
        m_LeftCont = m_RightCont();
        m_LeftLAngleSet = m_RightLAngleSet();
        m_LeftLSlewSet = m_RightLSlewSet();
        m_LeftLStrengthSet = m_RightLStrengthSet();
        m_LeftLCurveSet = m_RightLCurveSet();
        m_LeftRAngleSet = m_RightRAngleSet();
        m_LeftRSlewSet = m_RightRSlewSet();
        m_LeftRStrengthSet = m_RightRStrengthSet();
        m_LeftRCurveSet = m_RightRCurveSet();
        m_LeftLRAngleEq = m_RightLRAngleEq();
        m_LeftLRSlewEq = m_RightLRSlewEq();
        m_LeftLRStrengthEq = m_RightLRStrengthEq();
        m_LeftLRCurveEq = m_RightLRCurveEq();
        m_LeftLAngle = m_RightLAngle();
        m_LeftLSlew = m_RightLSlew();
        m_LeftLStrength = m_RightLStrength();
        m_LeftLCurve = m_RightLCurve();
        m_LeftRAngle = m_RightRAngle();
        m_LeftRSlew = m_RightRSlew();
        m_LeftRStrength = m_RightRStrength();
        m_LeftRCurve = m_RightRCurve();
    }
}

rib_data_type SkinXSec::GetRib( bool first, bool last )
{
    if( first || last )
    {
        m_TopCont = 0;
        m_RightCont = 0;
        m_BottomCont = 0;
        m_LeftCont = 0;
    }

    ValidateParms( );

    rib_data_type rib;

    // First GetCurve() forces Update() call if needed;
    rib.set_f( GetCurve().GetCurve() );

    if( !first && !last )
        rib.set_continuity( ( rib_data_type::connection_continuity ) m_TopCont() );

    piecewise_curve_type tangentcrv;
    piecewise_curve_type normcrv;

    vector< double > angles( 5, 0.0 );
    vector< double > slews( 5, 0.0 );
    vector< double > strengths( 5, 0.0 );
    vector< double > curves( 5, 0.0 );

    double scale =  GetScale();

    // Use 'wrong' side of first cross section to set right side.
    if ( first && ( m_TopLAngleSet() || m_TopLCurveSet() ) )
    {
        angles[0] = m_RightLAngle()*PI/180.0;
        angles[1] = m_BottomLAngle()*PI/180.0;
        angles[2] = m_LeftLAngle()*PI/180.0;
        angles[3] = m_TopLAngle()*PI/180.0;
        angles[4] = angles[0];

        slews[0] = m_RightLSlew()*PI/180.0;
        slews[1] = -m_BottomLSlew()*PI/180.0;
        slews[2] = -m_LeftLSlew()*PI/180.0;
        slews[3] = m_TopLSlew()*PI/180.0;
        slews[4] = slews[0];

        strengths[0] = m_RightLStrength() * scale;
        strengths[1] = m_BottomLStrength() * scale;
        strengths[2] = m_LeftLStrength() * scale;
        strengths[3] = m_TopLStrength() * scale;
        strengths[4] = strengths[0];

        curves[0] = m_RightLCurve() * scale;
        curves[1] = m_BottomLCurve() * scale;
        curves[2] = m_LeftLCurve() * scale;
        curves[3] = m_TopLCurve() * scale;
        curves[4] = curves[0];

        GetTanNormCrv( angles, slews, strengths, curves, tangentcrv, normcrv );

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

        slews[0] = m_RightLSlew()*PI/180.0;
        slews[1] = -m_BottomLSlew()*PI/180.0;
        slews[2] = -m_LeftLSlew()*PI/180.0;
        slews[3] = m_TopLSlew()*PI/180.0;
        slews[4] = slews[0];

        strengths[0] = m_RightLStrength() * scale;
        strengths[1] = m_BottomLStrength() * scale;
        strengths[2] = m_LeftLStrength() * scale;
        strengths[3] = m_TopLStrength() * scale;
        strengths[4] = strengths[0];

        curves[0] = m_RightLCurve() * scale;
        curves[1] = m_BottomLCurve() * scale;
        curves[2] = m_LeftLCurve() * scale;
        curves[3] = m_TopLCurve() * scale;
        curves[4] = curves[0];

        GetTanNormCrv( angles, slews, strengths, curves, tangentcrv, normcrv );

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

        slews[0] = m_RightRSlew()*PI/180.0;
        slews[1] = -m_BottomRSlew()*PI/180.0;
        slews[2] = -m_LeftRSlew()*PI/180.0;
        slews[3] = m_TopRSlew()*PI/180.0;
        slews[4] = slews[0];

        strengths[0] = m_RightRStrength() * scale;
        strengths[1] = m_BottomRStrength() * scale;
        strengths[2] = m_LeftRStrength() * scale;
        strengths[3] = m_TopRStrength() * scale;
        strengths[4] = strengths[0];

        curves[0] = m_RightRCurve() * scale;
        curves[1] = m_BottomRCurve() * scale;
        curves[2] = m_LeftRCurve() * scale;
        curves[3] = m_TopRCurve() * scale;
        curves[4] = curves[0];

        GetTanNormCrv( angles, slews, strengths, curves, tangentcrv, normcrv );

        if( m_TopRAngleSet() ) rib.set_right_fp( tangentcrv );
        if( m_TopRCurveSet() ) rib.set_right_fpp( normcrv );
    }

    return rib;
}

void SkinXSec::SetUnsetParms( int irib, const VspSurf &surf )
{
    SetUnsetParms( 0.0, false, irib, surf,
             m_RightLAngleSet,
             m_RightLSlewSet,
             m_RightLStrengthSet,
             m_RightLCurveSet,
             m_RightRAngleSet,
             m_RightRSlewSet,
             m_RightRStrengthSet,
             m_RightRCurveSet,
             m_RightLAngle,
             m_RightLSlew,
             m_RightLStrength,
             m_RightLCurve,
             m_RightRAngle,
             m_RightRSlew,
             m_RightRStrength,
             m_RightRCurve );

    SetUnsetParms( 1.0, true, irib, surf,
             m_BottomLAngleSet,
             m_BottomLSlewSet,
             m_BottomLStrengthSet,
             m_BottomLCurveSet,
             m_BottomRAngleSet,
             m_BottomRSlewSet,
             m_BottomRStrengthSet,
             m_BottomRCurveSet,
             m_BottomLAngle,
             m_BottomLSlew,
             m_BottomLStrength,
             m_BottomLCurve,
             m_BottomRAngle,
             m_BottomRSlew,
             m_BottomRStrength,
             m_BottomRCurve );

    SetUnsetParms( 2.0, true, irib, surf,
             m_LeftLAngleSet,
             m_LeftLSlewSet,
             m_LeftLStrengthSet,
             m_LeftLCurveSet,
             m_LeftRAngleSet,
             m_LeftRSlewSet,
             m_LeftRStrengthSet,
             m_LeftRCurveSet,
             m_LeftLAngle,
             m_LeftLSlew,
             m_LeftLStrength,
             m_LeftLCurve,
             m_LeftRAngle,
             m_LeftRSlew,
             m_LeftRStrength,
             m_LeftRCurve );

    SetUnsetParms( 3.0, false, irib, surf,
             m_TopLAngleSet,
             m_TopLSlewSet,
             m_TopLStrengthSet,
             m_TopLCurveSet,
             m_TopRAngleSet,
             m_TopRSlewSet,
             m_TopRStrengthSet,
             m_TopRCurveSet,
             m_TopLAngle,
             m_TopLSlew,
             m_TopLStrength,
             m_TopLCurve,
             m_TopRAngle,
             m_TopRSlew,
             m_TopRStrength,
             m_TopRCurve );

}

void SkinXSec::SetUnsetParms( double t, bool flipslew, int irib, const VspSurf &surf,
        BoolParm &LAngleSet,
        BoolParm &LSlewSet,
        BoolParm &LStrengthSet,
        BoolParm &LCurveSet,
        BoolParm &RAngleSet,
        BoolParm &RSlewSet,
        BoolParm &RStrengthSet,
        BoolParm &RCurveSet,
        Parm &LAngle,
        Parm &LSlew,
        Parm &LStrength,
        Parm &LCurve,
        Parm &RAngle,
        Parm &RSlew,
        Parm &RStrength,
        Parm &RCurve )
{
    double thetaL, phiL, strengthL, curvatureL;
    double thetaR, phiR, strengthR, curvatureR;

    GetAngStrCrv(  t,  irib,
        thetaL, phiL, strengthL, curvatureL,
        thetaR, phiR, strengthR, curvatureR,
        surf );

    double scale =  GetScale();

    if ( flipslew )
    {
        phiL = -phiL;
        phiR = -phiR;
    }

    if( !LAngleSet() ) LAngle = thetaL*180.0/PI;
    if( !LSlewSet() ) LSlew = phiL*180.0/PI;
    if( !LStrengthSet() ) LStrength = strengthL/scale;
    if( !LCurveSet() ) LCurve = curvatureL/scale;

    if( !RAngleSet() ) RAngle = thetaR*180.0/PI;
    if( !RSlewSet() ) RSlew = phiR*180.0/PI;
    if( !RStrengthSet() ) RStrength = strengthR/scale;
    if( !RCurveSet() ) RCurve = curvatureR/scale;
}

void SkinXSec::Reset()
{
    m_AllSymFlag = 1;
    m_TBSymFlag = 1;
    m_RLSymFlag = 1;

    m_TopLAngleSet = false;
    m_TopLSlewSet = false;
    m_TopLStrengthSet = false;
    m_TopLCurveSet = false;
    m_TopRAngleSet = false;
    m_TopRSlewSet = false;
    m_TopRStrengthSet = false;
    m_TopRCurveSet = false;
    m_TopLRAngleEq = false;
    m_TopLRSlewEq = false;
    m_TopLRStrengthEq = false;
    m_TopLRCurveEq = false;
    m_TopLAngle = 0.0;
    m_TopLSlew = 0.0;
    m_TopLStrength = 0.0;
    m_TopLCurve = 0.0;
    m_TopRAngle = 0.0;
    m_TopRSlew = 0.0;
    m_TopRStrength = 0.0;
    m_TopRCurve = 0.0;

    m_RightLAngleSet = false;
    m_RightLSlewSet = false;
    m_RightLStrengthSet = false;
    m_RightLCurveSet = false;
    m_RightRAngleSet = false;
    m_RightRSlewSet = false;
    m_RightRStrengthSet = false;
    m_RightRCurveSet = false;
    m_RightLRAngleEq = false;
    m_RightLRSlewEq = false;
    m_RightLRStrengthEq = false;
    m_RightLRCurveEq = false;
    m_RightLAngle = 0.0;
    m_RightLSlew = 0.0;
    m_RightLStrength = 0.0;
    m_RightLCurve = 0.0;
    m_RightRAngle = 0.0;
    m_RightRSlew = 0.0;
    m_RightRStrength = 0.0;
    m_RightRCurve = 0.0;

    m_BottomLAngleSet = false;
    m_BottomLSlewSet = false;
    m_BottomLStrengthSet = false;
    m_BottomLCurveSet = false;
    m_BottomRAngleSet = false;
    m_BottomRSlewSet = false;
    m_BottomRStrengthSet = false;
    m_BottomRCurveSet = false;
    m_BottomLRAngleEq = false;
    m_BottomLRSlewEq = false;
    m_BottomLRStrengthEq = false;
    m_BottomLRCurveEq = false;
    m_BottomLAngle = 0.0;
    m_BottomLSlew = 0.0;
    m_BottomLStrength = 0.0;
    m_BottomLCurve = 0.0;
    m_BottomRAngle = 0.0;
    m_BottomRSlew = 0.0;
    m_BottomRStrength = 0.0;
    m_BottomRCurve = 0.0;

    m_LeftLAngleSet = false;
    m_LeftLSlewSet = false;
    m_LeftLStrengthSet = false;
    m_LeftLCurveSet = false;
    m_LeftRAngleSet = false;
    m_LeftRSlewSet = false;
    m_LeftRStrengthSet = false;
    m_LeftRCurveSet = false;
    m_LeftLRAngleEq = false;
    m_LeftLRSlewEq = false;
    m_LeftLRStrengthEq = false;
    m_LeftLRCurveEq = false;
    m_LeftLAngle = 0.0;
    m_LeftLSlew = 0.0;
    m_LeftLStrength = 0.0;
    m_LeftLCurve = 0.0;
    m_LeftRAngle = 0.0;
    m_LeftRSlew = 0.0;
    m_LeftRStrength = 0.0;
    m_LeftRCurve = 0.0;

}

void SkinXSec::SetContinuity( int cx )
{
    m_TopCont = cx;
}

void SkinXSec::SetTanAngles( int side, double top, double right, double bottom, double left )
{
    m_AllSymFlag = false;
    m_TBSymFlag = false;
    m_RLSymFlag = false;
    if ( side == XSEC_BOTH_SIDES || side == XSEC_LEFT_SIDE )
    {
        m_TopLAngleSet = true;
        m_RightLAngleSet = true;
        m_BottomLAngleSet = true;
        m_LeftLAngleSet = true;

        m_TopLAngle = top;
        if ( right > XSEC_NO_VAL )      { m_RightLAngle = right; }
        else                            { m_RightLAngle = m_TopLAngle; }

        if ( bottom > XSEC_NO_VAL )     { m_BottomLAngle = bottom; }
        else                            { m_BottomLAngle = m_TopLAngle; }

        if ( left > XSEC_NO_VAL )       { m_LeftLAngle = left; }
        else                            { m_LeftLAngle = m_RightLAngle; }
    }

    if ( side == XSEC_BOTH_SIDES || side == XSEC_RIGHT_SIDE )
    {
        m_TopRAngleSet = true;
        m_RightRAngleSet = true;
        m_BottomRAngleSet = true;
        m_LeftRAngleSet = true;

        m_TopRAngle = top;
        if ( right > XSEC_NO_VAL )      { m_RightRAngle = right; }
        else                            { m_RightRAngle = m_TopRAngle; }

        if ( bottom > XSEC_NO_VAL )     { m_BottomRAngle = bottom; }
        else                            { m_BottomRAngle = m_TopRAngle; }

        if ( left > XSEC_NO_VAL )       { m_LeftRAngle = left; }
        else                            { m_LeftRAngle = m_RightRAngle; }
    }
}

void SkinXSec::SetTanSlews( int side, double top, double right, double bottom, double left )
{
    m_AllSymFlag = false;
    m_TBSymFlag = false;
    m_RLSymFlag = false;
    if ( side == XSEC_BOTH_SIDES || side == XSEC_LEFT_SIDE )
    {
        m_TopLSlewSet = true;
        m_RightLSlewSet = true;
        m_BottomLSlewSet = true;
        m_LeftLSlewSet = true;

        m_TopLSlew = top;
        if ( right > XSEC_NO_VAL )      { m_RightLSlew = right; }
        else                            { m_RightLSlew = m_TopLSlew; }

        if ( bottom > XSEC_NO_VAL )     { m_BottomLSlew = bottom; }
        else                            { m_BottomLSlew = m_TopLSlew; }

        if ( left > XSEC_NO_VAL )       { m_LeftLSlew = left; }
        else                            { m_LeftLSlew = m_RightLSlew; }
    }

    if ( side == XSEC_BOTH_SIDES || side == XSEC_RIGHT_SIDE )
    {
        m_TopRSlewSet = true;
        m_RightRSlewSet = true;
        m_BottomRSlewSet = true;
        m_LeftRSlewSet = true;

        m_TopRSlew = top;
        if ( right > XSEC_NO_VAL )      { m_RightRSlew = right; }
        else                            { m_RightRSlew = m_TopRSlew; }

        if ( bottom > XSEC_NO_VAL )     { m_BottomRSlew = bottom; }
        else                            { m_BottomRSlew = m_TopRSlew; }

        if ( left > XSEC_NO_VAL )       { m_LeftRSlew = left; }
        else                            { m_LeftRSlew = m_RightRSlew; }
    }
}

void SkinXSec::SetTanStrengths( int side, double top, double right, double bottom, double left )
{
    m_AllSymFlag = false;
    m_TBSymFlag = false;
    m_RLSymFlag = false;
    if ( side == XSEC_BOTH_SIDES || side == XSEC_LEFT_SIDE )
    {
        m_TopLStrengthSet = true;
        m_RightLStrengthSet = true;
        m_BottomLStrengthSet = true;
        m_LeftLStrengthSet = true;

        m_TopLStrength = top;
        if ( right > XSEC_NO_VAL )      { m_RightLStrength = right; }
        else                            { m_RightLStrength = m_TopLStrength; }

        if ( bottom > XSEC_NO_VAL )     { m_BottomLStrength = bottom; }
        else                            { m_BottomLStrength = m_TopLStrength; }

        if ( left > XSEC_NO_VAL )       { m_LeftLStrength = left; }
        else                            { m_LeftLStrength = m_RightLStrength; }
    }

    if ( side == XSEC_BOTH_SIDES || side == XSEC_RIGHT_SIDE )
    {
        m_TopRStrengthSet = true;
        m_RightRStrengthSet = true;
        m_BottomRStrengthSet = true;
        m_LeftRStrengthSet = true;

        m_TopRStrength = top;
        if ( right > XSEC_NO_VAL )      { m_RightRStrength = right; }
        else                            { m_RightRStrength = m_TopRStrength; }

        if ( bottom > XSEC_NO_VAL )     { m_BottomRStrength = bottom; }
        else                            { m_BottomRStrength = m_TopRStrength; }

        if ( left > XSEC_NO_VAL )       { m_LeftRStrength = left; }
        else                            { m_LeftRStrength = m_RightRStrength; }
    }
}

void SkinXSec::SetCurvatures( int side, double top, double right, double bottom, double left )
{
    m_AllSymFlag = false;
    m_TBSymFlag = false;
    m_RLSymFlag = false;
    if ( side == XSEC_BOTH_SIDES || side == XSEC_LEFT_SIDE )
    {
        m_TopLCurveSet = true;
        m_RightLCurveSet = true;
        m_BottomLCurveSet = true;
        m_LeftLCurveSet = true;

        m_TopLCurve = top;
        if ( right > XSEC_NO_VAL )      { m_RightLCurve = right; }
        else                            { m_RightLCurve = m_TopLCurve; }

        if ( bottom > XSEC_NO_VAL )     { m_BottomLCurve = bottom; }
        else                            { m_BottomLCurve = m_TopLCurve; }

        if ( left > XSEC_NO_VAL)        { m_LeftLCurve = left; }
        else                            { m_LeftLCurve = m_RightLCurve; }
    }

    if ( side == XSEC_BOTH_SIDES || side == XSEC_RIGHT_SIDE )
    {
        m_TopRCurveSet = true;
        m_RightRCurveSet = true;
        m_BottomRCurveSet = true;
        m_LeftRCurveSet = true;

        m_TopRCurve = top;
        if ( right > XSEC_NO_VAL )      { m_RightRCurve = right; }
        else                            { m_RightRCurve = m_TopRCurve; }

        if ( bottom > XSEC_NO_VAL )     { m_BottomRCurve = bottom; }
        else                            { m_BottomRCurve = m_TopRCurve; }

        if ( left > XSEC_NO_VAL )       { m_LeftRCurve = left; }
        else                            { m_LeftRCurve = m_RightRCurve; }
    }
}

// Set skinning behavior to be similar to VSP v2's Fuse2.
void SkinXSec::SetV2DefaultBehavior()
{
    m_AllSymFlag = 0;

    m_TopLAngleSet = 1;
    m_TopLSlewSet = 1;
    m_TopLStrengthSet = 1;
    m_TopRAngleSet = 1;
    m_TopRSlewSet = 1;
    m_TopRStrengthSet = 1;
    m_TopLRAngleEq = 1;

    m_RightLAngleSet = 1;
    m_RightLSlewSet = 1;
    m_RightLStrengthSet = 1;
    m_RightRAngleSet = 1;
    m_RightRSlewSet = 1;
    m_RightRStrengthSet = 1;
    m_RightLRAngleEq = 1;

    m_BottomLAngleSet = 1;
    m_BottomLSlewSet = 1;
    m_BottomLStrengthSet = 1;
    m_BottomRAngleSet = 1;
    m_BottomRSlewSet = 1;
    m_BottomRStrengthSet = 1;
    m_BottomLRAngleEq = 1;

    m_LeftLAngleSet = 1;
    m_LeftLSlewSet = 1;
    m_LeftLStrengthSet = 1;
    m_LeftRAngleSet = 1;
    m_LeftRSlewSet = 1;
    m_LeftRStrengthSet = 1;
    m_LeftLRAngleEq = 1;
}

void SkinXSec::ReadV2FileFuse2( xmlNodePtr &root )
{
    XSec::ReadV2FileFuse2( root );

    SetV2DefaultBehavior();

    m_TBSymFlag = XmlUtil::FindInt( root, "Top_Sym_Flag", m_TBSymFlag() );
    m_RLSymFlag = XmlUtil::FindInt( root, "Side_Sym_Flag", m_RLSymFlag() );

    m_TopLAngle = -XmlUtil::FindDouble( root, "Top_Tan_Ang", -m_TopLAngle() );
    m_TopLStrength = 3.0 * XmlUtil::FindDouble( root, "Top_Tan_Str_1", m_TopLStrength() / 3.0 );
    m_TopRStrength = 3.0 * XmlUtil::FindDouble( root, "Top_Tan_Str_2", m_TopRStrength() / 3.0 );

    m_BottomLAngle = -XmlUtil::FindDouble( root, "Bot_Tan_Ang", -m_BottomLAngle() );
    m_BottomLStrength = 3.0 * XmlUtil::FindDouble( root, "Bot_Tan_Str_1", m_BottomLStrength() / 3.0 );
    m_BottomRStrength = 3.0 * XmlUtil::FindDouble( root, "Bot_Tan_Str_2", m_BottomRStrength() / 3.0 );

    m_LeftLAngle = -XmlUtil::FindDouble( root, "Left_Tan_Ang", -m_LeftLAngle() );
    m_LeftLStrength = 3.0 * XmlUtil::FindDouble( root, "Left_Tan_Str_1", m_LeftLStrength() / 3.0 );
    m_LeftRStrength = 3.0 * XmlUtil::FindDouble( root, "Left_Tan_Str_2", m_LeftRStrength() / 3.0 );

    m_RightLAngle = -XmlUtil::FindDouble( root, "Right_Tan_Ang", -m_RightLAngle() );
    m_RightLStrength = 3.0 * XmlUtil::FindDouble( root, "Right_Tan_Str_1", m_RightLStrength() / 3.0 );
    m_RightRStrength = 3.0 * XmlUtil::FindDouble( root, "Right_Tan_Str_2", m_RightRStrength() / 3.0 );
}

void SkinXSec::ReadV2FileFuse1( xmlNodePtr &root )
{
    XSec::ReadV2FileFuse1( root );

    SetV2DefaultBehavior();

    m_AllSymFlag = 1;

    //==== Profile Stuff ====//
    m_TopLAngle = atan( XmlUtil::FindDouble( root, "Profile_Tan_Ang", 0.0 ) ) * 180.0 / PI;
    m_TopLStrength = 3.0 * XmlUtil::FindDouble( root, "Profile_Tan_Str_1", m_TopLStrength() / 3.0 );
    m_TopRStrength = 3.0 * XmlUtil::FindDouble( root, "Profile_Tan_Str_2", m_TopRStrength() / 3.0 );

}

//==========================================================================//
//==========================================================================//
//==========================================================================//

//==== Default Constructor ====//
FuseXSec::FuseXSec( XSecCurve *xsc ) : SkinXSec( xsc)
{
    m_Type = XSEC_FUSE;

    m_RefLength = 1.0;
    m_RefLenVal = 1.0;

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
    m_ZRotate.SetDescript( "Rotation about z-axis of cross section" );

    m_Spin.Init( "Spin", m_GroupName, this, 0.0, -1.0, 1.0 );
    m_Spin.SetDescript( "Shift curve parameterization" );

    m_RefLength.Init( "RefLength", m_GroupName, this, 1.0, 1e-8, 1e12 );

    SetV2DefaultBehavior();
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

    m_TransformedCurve.Spin01( m_Spin() );

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
    if ( std::abs( len - m_RefLenVal ) < DBL_EPSILON )
    {
        return;
    }

    m_RefLength = len;
    m_LateUpdateFlag = true;

    m_XLocPercent.SetRefVal( m_RefLength() );
    m_YLocPercent.SetRefVal( m_RefLength() );
    m_ZLocPercent.SetRefVal( m_RefLength() );

    m_RefLenVal = m_RefLength();
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


//==== Copy FuseXSec parameters only ====//
// This is used by the 'loop' capability to make first/last XSecs
// match.  We need to copy all of the FuseXSec parms without
// copying the skinning parameters.
// XSecCurve parms are handled separately.
void FuseXSec::CopyFuseXSParms( XSec* xs )
{
    if ( xs )
    {
        FuseXSec* fxs = ( FuseXSec* ) xs;

        m_XLocPercent = fxs->m_XLocPercent();
        m_YLocPercent = fxs->m_YLocPercent();
        m_ZLocPercent = fxs->m_ZLocPercent();

        m_RefLength = fxs->m_RefLength();

        m_Spin = fxs->m_Spin();

        m_XRotate = fxs->m_XRotate();
        m_YRotate = fxs->m_YRotate();
        m_ZRotate = fxs->m_ZRotate();
    }
}

double FuseXSec::GetScale()
{
    XSecSurf* xsecsurf = (XSecSurf*) GetParentContainerPtr();
    int indx = xsecsurf->FindXSecIndex( m_ID );

    double dx(0), dy(0), dz(0), dr(0), dw(0);

    double scaleL = 1e12;
    double scaleR = 1e12;
    double scale = 1.0;

    if( indx > 0 )
    {
        FuseXSec* prevxs = (FuseXSec*) xsecsurf->FindXSec( indx - 1);
        if( prevxs )
        {
            dx = ( m_XLocPercent() - prevxs->m_XLocPercent() ) * m_RefLength();
            dy = ( m_YLocPercent() - prevxs->m_YLocPercent() ) * m_RefLength();
            dz = ( m_ZLocPercent() - prevxs->m_ZLocPercent() ) * m_RefLength();
            dr = ( GetXSecCurve()->GetHeight() - prevxs->GetXSecCurve()->GetHeight() ) * 0.5;
            dr = dr * dr;
            dw = ( GetXSecCurve()->GetWidth() - prevxs->GetXSecCurve()->GetWidth() ) * 0.5;
            dw = dw * dw;
            if ( dw > dr )
            {
                dr = dw;
            }
            scaleL = sqrt( dx*dx + dy*dy + dz*dz + dr );
        }
    }

    if( indx < ( xsecsurf->NumXSec() - 1 ) )
    {
        FuseXSec* nxtxs = (FuseXSec*) xsecsurf->FindXSec( indx + 1);
        if( nxtxs )
        {
            dx = ( nxtxs->m_XLocPercent() - m_XLocPercent() ) * m_RefLength();
            dy = ( nxtxs->m_YLocPercent() - m_YLocPercent() ) * m_RefLength();
            dz = ( nxtxs->m_ZLocPercent() - m_ZLocPercent() ) * m_RefLength();
            dr = ( nxtxs->GetXSecCurve()->GetHeight() - GetXSecCurve()->GetHeight() ) * 0.5;
            dr = dr * dr;
            dw = ( nxtxs->GetXSecCurve()->GetWidth() - GetXSecCurve()->GetWidth() ) * 0.5;
            dw = dw * dw;
            if ( dw > dr)
            {
                dr = dw;
            }
            scaleR = sqrt( dx*dx + dy*dy + dz*dz + dr );
        }
    }

    if ( scaleL < scaleR )
    {
        scale = scaleL;
    }
    else
    {
        scale = scaleR;
    }

    if ( scale < 1e-4 )
    {
        scale = 1e-4;
    }

    return scale;
}

void FuseXSec::ReadV2FileFuse2( xmlNodePtr &root )
{
    SkinXSec::ReadV2FileFuse2( root );

    m_XLocPercent = XmlUtil::FindDouble( root, "Spine_Location", m_XLocPercent() );
    m_YLocPercent.SetResult( XmlUtil::FindDouble( root, "Y_Offset", m_YLocPercent.GetResult() ) );
    m_ZLocPercent.SetResult( XmlUtil::FindDouble( root, "Z_Offset", m_ZLocPercent.GetResult() ) );
}

void FuseXSec::ReadV2FileFuse1( xmlNodePtr &root )
{
    SkinXSec::ReadV2FileFuse1( root );

//    num_pnts = xmlFindInt( root, "Num_Pnts", num_pnts );

    m_XLocPercent = XmlUtil::FindDouble( root, "Spine_Location", m_XLocPercent() );
    m_ZLocPercent.SetResult( XmlUtil::FindDouble( root, "Z_Offset", m_ZLocPercent.GetResult() ) );

//    topThick = XmlUtil::FindDouble( root, "Top_Thick", topThick() );
//    botThick = XmlUtil::FindDouble( root, "Bot_Thick", botThick() );
//    sideThick = XmlUtil::FindDouble( root, "Side_Thick", sideThick() );
//
//    actTopThick = XmlUtil::FindDouble( root, "Act_Top_Thick", actTopThick );
//    actBotThick = XmlUtil::FindDouble( root, "Act_Bot_Thick", actBotThick );
//    actSideThick = XmlUtil::FindDouble( root, "Act_Side_Thick", actSideThick );
//
//    imlXOff = XmlUtil::FindDouble( root, "IML_X_Offset", imlXOff );
//    imlZOff = XmlUtil::FindDouble( root, "IML_Z_Offset", imlZOff );
//
//    imlFlag = XmlUtil::FindInt( root, "IML_Flag", imlFlag );
//    mlType = XmlUtil::FindInt( root, "ML_Type", mlType );


}

//==========================================================================//
//==========================================================================//
//==========================================================================//

//==== Default Constructor ====//
StackXSec::StackXSec( XSecCurve *xsc ) : SkinXSec( xsc)
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
    m_ZRotate.SetDescript( "Rotation about z-axis of cross section" );

    m_XCenterRot.Init( "m_XCenterRot", m_GroupName, this,  0.0, -1.0e12, 1.0e12 );
    m_XCenterRot.SetDescript( "X Center Of Rotation" );
    m_YCenterRot.Init( "m_YCenterRot", m_GroupName, this,  0.0, -1.0e12, 1.0e12 );
    m_YCenterRot.SetDescript( "Y Center Of Rotation" );
    m_ZCenterRot.Init( "m_ZCenterRot", m_GroupName, this,  0.0, -1.0e12, 1.0e12 );
    m_ZCenterRot.SetDescript( "Z Center Of Rotation" );

    m_Spin.Init( "Spin", m_GroupName, this, 0.0, -1.0, 1.0 );
    m_Spin.SetDescript( "Shift curve parameterization" );

    SetV2DefaultBehavior();

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

    m_TransformedCurve.Spin01( m_Spin() );

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

double StackXSec::GetScale()
{
    XSecSurf* xsecsurf = (XSecSurf*) GetParentContainerPtr();
    int indx = xsecsurf->FindXSecIndex( m_ID );

    double dx(0), dy(0), dz(0), dw(0), dr(0);

    double scaleL = 1e12;
    double scaleR = 1e12;
    double scale = 1.0;

    if( indx > 0 )
    {
        dx = m_XDelta();
        dy = m_YDelta();
        dz = m_ZDelta();
        FuseXSec* prevxs = (FuseXSec*) xsecsurf->FindXSec( indx - 1 );
        if( prevxs )
        {
            dr = ( GetXSecCurve()->GetHeight() - prevxs->GetXSecCurve()->GetHeight() ) * 0.5;
            dr = dr * dr;
            dw = (  GetXSecCurve()->GetWidth() - prevxs->GetXSecCurve()->GetWidth() ) * 0.5;
            dw = dw * dw;
            if ( dw > dr )
            {
                dr = dw;
            }
        }
        scaleL = sqrt( dx*dx + dy*dy + dz*dz + dr );
    }

    if( indx < (xsecsurf->NumXSec() - 1) )
    {
        StackXSec* nxtxs = (StackXSec*) xsecsurf->FindXSec( indx + 1 );
        if( nxtxs )
        {
            dx = nxtxs->m_XDelta();
            dy = nxtxs->m_YDelta();
            dz = nxtxs->m_ZDelta();
            dr = ( nxtxs->GetXSecCurve()->GetHeight() - GetXSecCurve()->GetHeight() ) * 0.5;
            dr = dr * dr;
            dw = ( nxtxs->GetXSecCurve()->GetWidth() - GetXSecCurve()->GetWidth() ) * 0.5;
            dw = dw * dw;
            if ( dw > dr)
            {
                dr = dw;
            }
            scaleR = sqrt( dx*dx + dy*dy + dz*dz + dr );
        }
    }

    if ( scaleL < scaleR )
    {
        scale = scaleL;
    }
    else
    {
        scale = scaleR;
    }

    if ( scale < 1e-4 )
    {
        scale = 1e-4;
    }

    return scale;
}
