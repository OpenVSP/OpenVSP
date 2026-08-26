//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//

// XSec.cpp: implementation of the XSec class.
//
//////////////////////////////////////////////////////////////////////

#include <algorithm>
#include <cmath>

#include "XSec.h"
#include "Geom.h"
#include "ParmMgr.h"
#include "IDMgr.h"
#include "StlHelper.h"
#include "XmlUtil.h"
#include <float.h>

using std::string;
using namespace vsp;

//==== Default Constructor ====//
XSec::XSec( XSecCurve *xsc )
{
    m_Name = "XSec";

    if ( xsc == nullptr )
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

    SetParmContainerType( vsp::ATTROBJ_SEC );
    AttachAttrCollection();
}

//==== Destructor ====//
XSec::~XSec()
{
    delete m_XSCurve;
}

void XSec::ChangeID( const string &newid )
{
    string oldid = m_ID;
    ParmContainer::ChangeID( newid );

    // The XSec kept as the copy buffer has no XSecSurf holding it.
    XSecSurf* xssurf = ( XSecSurf* ) GetParentContainerPtr();

    if ( xssurf )
    {
        xssurf->ChangeXSecID( oldid, newid );
    }

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

void XSec::SetGroupAlias( const string & alias )
{
    m_GroupAlias = alias;
    //==== Assign Group Suffix To All Parms ====//
    for ( int i = 0 ; i < ( int )m_ParmVec.size() ; i++ )
    {
        Parm* p = ParmMgr.FindParm( m_ParmVec[i] );
        if ( p )
        {
            p->SetGroupAlias( alias );
        }
    }
}

string XSec::GetDisplayGroupName( const string & group_name_overwrite )
{
    string group_name = group_name_overwrite.empty()? m_GroupName : group_name_overwrite;
    return Parm::GetDisplayGroupName( group_name, m_GroupAlias, m_GroupSuffix );
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
    }
    else
    {
        Update();
    }

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

void XSec::GetDrawLines( Matrix4d &transMat, vector< vec3d > &lines )
{
    // Out-parameter form so repeated callers reuse the point vector's allocation.
    lines.clear();

    VspCurve curve = GetCurve();
    curve.Transform( transMat );
    curve.TessAdapt( lines, 1e-2, 10 );
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
    string lastreset = IDMgr.ResetRemapID();
    xmlNodePtr root = xmlNewNode( nullptr, ( const xmlChar * )"Vsp_Geometry" );
    if ( xs->GetType() == GetType() && xs->GetXSecCurve()->GetType() == GetXSecCurve()->GetType() )
    {
        xs->EncodeXml( root );
        IDMgr.PreRegisterIDs( root );
        DecodeXml( root );
    }
    else
    {
        xs->XSec::EncodeXml( root );
        DecodeXml( root );

        m_XSCurve->SetWidthHeight( xs->GetXSecCurve()->GetWidth(), xs->GetXSecCurve()->GetHeight() );
    }
    xmlFreeNode( root );
    IDMgr.ResetRemapID( lastreset );
}

//==== Encode XML ====//
xmlNodePtr XSec::EncodeXml(  xmlNodePtr & node  )
{
    ParmContainer::EncodeXml( node );
    xmlNodePtr xsec_node = xmlNewChild( node, nullptr, BAD_CAST "XSec", nullptr );
    if ( xsec_node )
    {
        XmlUtil::AddIntNode( xsec_node, "Type", m_Type );
        XmlUtil::AddStringNode( xsec_node, "GroupName", m_GroupName );

        if ( !m_GroupAlias.empty() )
        {
            XmlUtil::AddStringNode( xsec_node, "GroupAlias", m_GroupAlias );
        }

        xmlNodePtr xscrv_node = xmlNewChild( xsec_node, nullptr, BAD_CAST "XSecCurve", nullptr );
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
        string group_alias = XmlUtil::FindString( child_node, "GroupAlias", m_GroupAlias );
        SetGroupAlias( group_alias );

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
    xmlNodePtr xsec_node = xmlNewChild( node, nullptr, BAD_CAST "XSec", nullptr );
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

void XSec::GetSimpleBasis( vec3d &xdir, vec3d &ydir, vec3d &zdir )
{
    Matrix4d basis;

    // Get primary orientation of this XSecSurf
    XSecSurf* xsecsurf = (XSecSurf*) GetParentContainerPtr();
    xsecsurf->GetBasicTransformation( 0.0, basis );

    // Transform primary orientation to orientation of this XSec
    basis.postMult( GetTransform()->data() );

    basis.getBasis( xdir, ydir, zdir );
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
    rmat.rotate( 2.0*M_PI*(t-tmin)/(tmax-tmin), pdir );
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

    // Sample at the stations as well as at the XSec curve's own joints.
    //
    // The controls are laid out at the stations, but what the rib actually carries is a
    // curve of tangent and normal vectors, built by evaluating the controls at these
    // parameters and fitting through the results.  A station falling between two joints
    // never gets sampled, so its value is smeared across the gap rather than enforced: on a
    // default Stack the joints fall on half integers, and a spine at W=2.5 holds its 40
    // degrees exactly while one at W=2.25 achieves 24 of them.  Which is to say the control
    // resolution was the cross section curve's segmentation, not the user's station layout.
    // Matches SkinXSec::GetMinStationGap: two stations closer than this are one station, so
    // two sample parameters that close are one sample.
    double sampgap = 1.0e-3;

    // Take the bounds before inserting anything.  Reading them from the back of crvts inside
    // the loop would read whatever was last pushed instead of the largest parameter, and
    // every station beyond it would be rejected as out of range -- which left the first
    // station sampled and the rest of them not.
    double tfirst = crvts[0];
    double tlast = crvts[crvts.size() - 1];

    for ( int i = 0; i < ( int )ts.size(); i++ )
    {
        if ( ts[i] < tfirst || ts[i] > tlast )
        {
            continue;
        }

        bool have = false;
        for ( int j = 0; j < ( int )crvts.size(); j++ )
        {
            if ( std::abs( crvts[j] - ts[i] ) < sampgap )
            {
                have = true;
                break;
            }
        }

        if ( !have )
        {
            crvts.push_back( ts[i] );
        }
    }
    std::sort( crvts.begin(), crvts.end() );

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
    vector< double > ts( 5 );
    for ( int i = 0 ; i < 5 ; i++ )
    {
        ts[i] = (double) i;
    }

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

    thetaL =  (M_PI/2.0) - signed_angle( tanL, wdir, updir );
    phiL = signed_angle( updir, tanL, wdir ) - (M_PI/2.0);
    strengthL = tanL.mag();

    thetaR = (M_PI/2.0) - signed_angle( tanR, wdir, updir );
    phiR = signed_angle( updir, tanR, wdir ) - (M_PI/2.0);
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
    m_Name = "SkinXSec";

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

SkinXSec::~SkinXSec()
{
    DelAllSpines();
}

xmlNodePtr SkinXSec::EncodeXml( xmlNodePtr & node )
{
    xmlNodePtr xsec_node = XSec::EncodeXml( node );

    if ( xsec_node )
    {
        xmlNodePtr spine_root = xmlNewChild( xsec_node, nullptr, BAD_CAST "SkinSpines", nullptr );
        if ( spine_root )
        {
            XmlUtil::AddIntNode( spine_root, "NumSpines", m_SpineVec.size() );

            for ( int i = 0; i < ( int )m_SpineVec.size(); i++ )
            {
                char name[256];
                snprintf( name, sizeof( name ), "SkinSpine_%d", i );
                xmlNodePtr spine_node = xmlNewChild( spine_root, nullptr, BAD_CAST name, nullptr );
                if ( spine_node )
                {
                    XmlUtil::AddStringNode( spine_node, "SpineID", m_SpineVec[i]->GetSpineID() );
                    m_SpineVec[i]->EncodeXml( spine_node );
                }
            }
        }
    }

    return xsec_node;
}

xmlNodePtr SkinXSec::DecodeXml( xmlNodePtr & node )
{
    xmlNodePtr xsec_node = XSec::DecodeXml( node );

    DelAllSpines();

    if ( xsec_node )
    {
        xmlNodePtr spine_root = XmlUtil::GetNode( xsec_node, "SkinSpines", 0 );
        if ( spine_root )
        {
            int nspine = XmlUtil::FindInt( spine_root, "NumSpines", 0 );

            for ( int i = 0; i < nspine; i++ )
            {
                char name[256];
                snprintf( name, sizeof( name ), "SkinSpine_%d", i );
                xmlNodePtr spine_node = XmlUtil::GetNode( spine_root, name, 0 );
                if ( spine_node )
                {
                    // Not AddSpine: every value it would set is about to be overwritten by
                    // the file a line later.
                    SkinSpine* sp = new SkinSpine();
                    if ( sp )
                    {
                        // Older files carry no tag; give those a fresh one so the sync has
                        // something to match on, which for a file written in step is the same
                        // as matching by position.
                        string sid = XmlUtil::FindString( spine_node, "SpineID", "" );
                        if ( sid.empty() )
                        {
                            sid = GenerateRandomID( vsp::ID_LENGTH_PARMCONTAINER );
                        }
                        sp->SetSpineID( sid );
                        sp->SetParentContainer( m_ID );
                        m_SpineVec.push_back( sp );
                        sp->DecodeXml( spine_node );
                    }
                }
            }
        }
    }

    return xsec_node;
}

void SkinXSec::AddLinkableParms( vector< string > & parm_vec, const string & link_container_id )
{
    XSec::AddLinkableParms( parm_vec, link_container_id );

    for ( int i = 0; i < ( int )m_SpineVec.size(); i++ )
    {
        if ( m_SpineVec[i] )
        {
            m_SpineVec[i]->AddLinkableParms( parm_vec, link_container_id );
        }
    }
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
    // Continuity stays uniform around the cross section.  A C1 or C2 joint requires the
    // left and right derivative curves to agree over the span that enforces it, but the
    // values come from one periodic spline through all four stations, so the two curves
    // only coincide at a station -- never across a whole span -- unless all four sides
    // agree.  The Set flags below are per side; only continuity is copied.
    Cont = m_TopCont();

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
    // Settle the angle flags before deriving anything from them.  Equality forces its pair
    // on, and that used to happen at the bottom -- after slew and strength had been derived
    // from the angle flag's earlier value.  One pass then left LAngleSet on with LStrengthSet
    // off, which the line below says cannot happen, and the surface was built from that state
    // while the next validation quietly produced a different one.
    if ( LRAngleEq() )
    {
        LAngleSet = true;
        RAngleSet = true;
    }

    // Slew and strength are parts of the same tangent as the angle, so they follow it.
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

void SkinXSec::ValidateParms( )
{
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

// Fill one station from a side's parameters.  Slew carries a sign convention per side so
// that a positive value reads the same way around the cross section; SetUnsetParms applies
// the matching flip when reading achieved values back.
//===============================================================================//
//========================        SkinSpine        ==============================//
//===============================================================================//

SkinSpine::SkinSpine() : ParmContainer()
{
    m_Name = "Spine";

    string group = "SkinSpine";

    // Position on a [0, 1] basis, the way OpenVSP shows U and W everywhere else.  The
    // skinning works in the cross section curve's own parameter, so this is scaled up
    // wherever it is read -- 01 in the name to keep which basis is which unmistakable.
    m_W01.Init( "W01", group, this, 0.125, 0.0, 1.0 );
    m_W01.SetDescript( "Position of this spine around the cross section, on a [0, 1] basis" );

    m_LRSymFlag.Init( "LRSym", group, this, false, 0, 1 );
    m_LRSymFlag.SetDescript( "Mirror this spine to the other side" );
    m_TBSymFlag.Init( "TBSym", group, this, false, 0, 1 );
    m_TBSymFlag.SetDescript( "Mirror this spine to the other half" );

    m_LAngleSet.Init( "LAngleSet", group, this, 0, 0, 1 );
    m_LSlewSet.Init( "LSlewSet", group, this, 0, 0, 1 );
    m_LStrengthSet.Init( "LStrengthSet", group, this, 0, 0, 1 );
    m_LCurveSet.Init( "LCurveSet", group, this, 0, 0, 1 );
    m_RAngleSet.Init( "RAngleSet", group, this, 0, 0, 1 );
    m_RSlewSet.Init( "RSlewSet", group, this, 0, 0, 1 );
    m_RStrengthSet.Init( "RStrengthSet", group, this, 0, 0, 1 );
    m_RCurveSet.Init( "RCurveSet", group, this, 0, 0, 1 );

    m_LRAngleEq.Init( "LRAngleEq", group, this, 0, 0, 1 );
    m_LRSlewEq.Init( "LRSlewEq", group, this, 0, 0, 1 );
    m_LRStrengthEq.Init( "LRStrengthEq", group, this, 0, 0, 1 );
    m_LRCurveEq.Init( "LRCurveEq", group, this, 0, 0, 1 );

    m_LAngle.Init( "LAngle", group, this, 0.0, -180.0, 180.0 );
    m_LSlew.Init( "LSlew", group, this, 0.0, -180.0, 180.0 );
    m_LStrength.Init( "LStrength", group, this, 0.0, 0.0, 1.0e12 );
    m_LCurve.Init( "LCurve", group, this, 0.0, -1.0e12, 1.0e12 );
    m_RAngle.Init( "RAngle", group, this, 0.0, -180.0, 180.0 );
    m_RSlew.Init( "RSlew", group, this, 0.0, -180.0, 180.0 );
    m_RStrength.Init( "RStrength", group, this, 0.0, 0.0, 1.0e12 );
    m_RCurve.Init( "RCurve", group, this, 0.0, -1.0e12, 1.0e12 );
}

void SkinSpine::ParmChanged( Parm* parm_ptr, int type )
{
    ParmContainer* pc = GetParentContainerPtr();
    if ( pc )
    {
        pc->ParmChanged( parm_ptr, type );
    }
}

void SkinSpine::SetGroupDisplaySuffix( int num )
{
    for ( int i = 0; i < ( int )m_ParmVec.size(); i++ )
    {
        Parm* p = ParmMgr.FindParm( m_ParmVec[i] );
        if ( p )
        {
            p->SetGroupDisplaySuffix( num );
        }
    }
}

void SkinSpine::SetGroupAlias( const string & alias )
{
    for ( int i = 0; i < ( int )m_ParmVec.size(); i++ )
    {
        Parm* p = ParmMgr.FindParm( m_ParmVec[i] );
        if ( p )
        {
            p->SetGroupAlias( alias );
        }
    }
}

// The spine itself plus a mirror for each symmetry asked for.  A left/right mirror sends
// W to 2 - W, which fixes Bottom and Top and swaps Left and Right; a top/bottom mirror
// sends W to -W, which fixes Right and Left and swaps Top and Bottom.  Asking for both
// gives the third, W + 2.  Each reflection reverses the sense of slew, so the mirrored
// copies carry a flag telling the caller to negate it.
void SkinSpine::GetMirrorW( vector< double > &ws, vector< bool > &flipslew, double period )
{
    ws.clear();
    flipslew.clear();

    // Up into the cross section curve's parameter, where the reflections are expressed.
    double w = m_W01() * period;

    ws.push_back( w );
    flipslew.push_back( false );

    // Half a period is the Left station, which is the axis a left/right mirror reflects in.
    double half = 0.5 * period;

    if ( m_LRSymFlag() )
    {
        ws.push_back( half - w );
        flipslew.push_back( true );
    }

    if ( m_TBSymFlag() )
    {
        ws.push_back( -w );
        flipslew.push_back( true );
    }

    if ( m_LRSymFlag() && m_TBSymFlag() )
    {
        // Both reflections compose into a half turn, which reverses slew twice.
        ws.push_back( half + w );
        flipslew.push_back( false );
    }

    for ( int i = 0; i < ( int )ws.size(); i++ )
    {
        while ( ws[i] < 0.0 )
        {
            ws[i] += period;
        }
        while ( ws[i] >= period )
        {
            ws[i] -= period;
        }
    }
}

//===============================================================================//

void SkinXSec::SetGroupDisplaySuffix( int num )
{
    XSec::SetGroupDisplaySuffix( num );

    for ( int i = 0; i < ( int )m_SpineVec.size(); i++ )
    {
        if ( m_SpineVec[i] )
        {
            m_SpineVec[i]->SetGroupDisplaySuffix( num );
        }
    }
}

// Seed each spine's name with its index and push it down as the parms' group alias.  Two
// spines on one XSec would otherwise share a display group name; a user is free to rename
// from here, and the browser shows whatever they choose.
// The lowest numbered Spine_ name no spine here is using.  Numbering by the array index or
// by the current count repeats a name as soon as one has been deleted -- with Spine_0 and
// Spine_1, removing Spine_0 and adding gives a second Spine_1 -- and the name is the parm
// group alias, so the two then collide in the Parm Link and Design Variable pickers.
string SkinXSec::UnusedSpineName() const
{
    char str[256];

    for ( int n = 0; ; n++ )
    {
        snprintf( str, sizeof( str ), "Spine_%d", n );

        bool taken = false;
        for ( int i = 0; i < ( int )m_SpineVec.size() && !taken; i++ )
        {
            if ( m_SpineVec[i] && m_SpineVec[i]->GetName() == string( str ) )
            {
                taken = true;
            }
        }

        if ( !taken )
        {
            return string( str );
        }
    }
}

void SkinXSec::RenumberSpines()
{
    for ( int i = 0; i < ( int )m_SpineVec.size(); i++ )
    {
        SkinSpine* sp = m_SpineVec[i];
        if ( !sp )
        {
            continue;
        }

        // An empty name gets the lowest unused number rather than the array index, which
        // would collide with a spine already carrying that name.
        if ( sp->GetName().empty() )
        {
            sp->SetName( UnusedSpineName() );
        }

        sp->SetGroupAlias( sp->GetName() );
    }
}

// Put this cross section's spines in the same order as another's, matching by tag.
//
// Syncing matches a copy to its master by tag and so does not care what order the arrays are
// in, but everything downstream reaches a spine by its index: the Skinning tab, DelSkinSpine,
// GetSkinSpineID and the active spine highlight.  An array that disagrees with the master's
// therefore makes one index mean different spines on different cross sections, which a paste
// from another Geom produces -- it rebuilds the list in the source's order.
void SkinXSec::OrderSpinesLike( const SkinXSec* other )
{
    if ( !other )
    {
        return;
    }

    vector< SkinSpine* > ordered;
    ordered.reserve( m_SpineVec.size() );

    for ( int k = 0; k < ( int )other->m_SpineVec.size(); k++ )
    {
        const SkinSpine* o = other->m_SpineVec[k];
        if ( !o )
        {
            continue;
        }

        for ( int j = 0; j < ( int )m_SpineVec.size(); j++ )
        {
            if ( m_SpineVec[j] && m_SpineVec[j]->GetSpineID() == o->GetSpineID() )
            {
                ordered.push_back( m_SpineVec[j] );
                break;
            }
        }
    }

    // A spine the other does not have keeps its place at the end.  This routine orders; it
    // does not decide what belongs here.
    for ( int j = 0; j < ( int )m_SpineVec.size(); j++ )
    {
        if ( !m_SpineVec[j] )
        {
            continue;
        }

        if ( std::find( ordered.begin(), ordered.end(), m_SpineVec[j] ) == ordered.end() )
        {
            ordered.push_back( m_SpineVec[j] );
        }
    }

    m_SpineVec = ordered;
}

// Where to put a spine when the user just presses Add: the middle of the widest gap in the
// station layout as it stands.
//
// Any fixed choice eventually lands on a station that is already there, and two stations at
// the same parameter are one station -- the newcomer is merged away and appears in the
// browser doing nothing at all.  A fixed 0.25 would sit on Bottom; adding twice would put the
// second on top of the first.  The widest gap is always clear, and always the most useful
// place to be offered.
double SkinXSec::SuggestSpineW01()
{
    vector< SkinStation > stations;
    GetStations( stations );

    double t0 = GetCurve().GetCurve().get_t0();
    double period = GetCurve().GetCurve().get_tmax() - t0;

    int n = stations.size();
    if ( n < 1 )
    {
        return 0.125;
    }

    double best = stations[0].m_W + 0.5 * period;
    double bestgap = -1.0;

    for ( int i = 0; i < n; i++ )
    {
        int j = i + 1;
        double wj;
        if ( j < n )
        {
            wj = stations[j].m_W;
        }
        else
        {
            // The last gap closes back onto the first station, the long way round.
            wj = stations[0].m_W + period;
        }

        double gap = wj - stations[i].m_W;
        if ( gap > bestgap )
        {
            bestgap = gap;
            best = stations[i].m_W + 0.5 * gap;
        }
    }

    while ( best >= t0 + period )
    {
        best -= period;
    }

    return ( best - t0 ) / period;
}

// A ParmContainer's children hold their parent's ID, so changing it has to reach them.
// XSec::ChangeID reparents the XSecCurve; the spines are children too, and leaving them
// pointing at an ID that no longer exists would stop SkinSpine::ParmChanged finding the Geom
// to dirty, after which a spine edit would no longer rebuild the surface.
void SkinXSec::ChangeID( const string &newid )
{
    XSec::ChangeID( newid );

    for ( int i = 0; i < ( int )m_SpineVec.size(); i++ )
    {
        if ( m_SpineVec[i] )
        {
            m_SpineVec[i]->SetParentContainer( newid );
        }
    }
}

SkinSpine* SkinXSec::AddSpine( double w01 )
{
    SkinSpine* sp = new SkinSpine();
    if ( !sp )
    {
        return nullptr;
    }

    sp->m_W01 = w01;
    sp->SetSpineID( GenerateRandomID( vsp::ID_LENGTH_PARMCONTAINER ) );
    sp->SetParentContainer( m_ID );

    sp->SetName( UnusedSpineName() );

    m_SpineVec.push_back( sp );

    RenumberSpines();

    m_LateUpdateFlag = true;

    return sp;
}

void SkinXSec::DelSpine( int index )
{
    if ( index < 0 || index >= ( int )m_SpineVec.size() )
    {
        return;
    }

    delete m_SpineVec[index];
    m_SpineVec.erase( m_SpineVec.begin() + index );

    m_LateUpdateFlag = true;
}

void SkinXSec::DelAllSpines()
{
    for ( int i = 0; i < ( int )m_SpineVec.size(); i++ )
    {
        delete m_SpineVec[i];
    }
    m_SpineVec.clear();

    m_LateUpdateFlag = true;
}

SkinSpine* SkinXSec::GetSpine( int index )
{
    if ( index < 0 || index >= ( int )m_SpineVec.size() )
    {
        return nullptr;
    }
    return m_SpineVec[index];
}

void SkinXSec::GetSideStation( int side, SkinStation &st )
{
    st.m_W = side;
    st.m_IsSide = true;

    if ( side == SKIN_SIDE_RIGHT )
    {
        st.m_LAngleSet = m_RightLAngleSet();   st.m_LSlewSet = m_RightLSlewSet();
        st.m_LStrengthSet = m_RightLStrengthSet(); st.m_LCurveSet = m_RightLCurveSet();
        st.m_RAngleSet = m_RightRAngleSet();   st.m_RSlewSet = m_RightRSlewSet();
        st.m_RStrengthSet = m_RightRStrengthSet(); st.m_RCurveSet = m_RightRCurveSet();

        st.m_LAngle = m_RightLAngle();  st.m_LSlew = m_RightLSlew();
        st.m_LStrength = m_RightLStrength(); st.m_LCurve = m_RightLCurve();
        st.m_RAngle = m_RightRAngle();  st.m_RSlew = m_RightRSlew();
        st.m_RStrength = m_RightRStrength(); st.m_RCurve = m_RightRCurve();
    }
    else if ( side == SKIN_SIDE_BOTTOM )
    {
        st.m_LAngleSet = m_BottomLAngleSet();   st.m_LSlewSet = m_BottomLSlewSet();
        st.m_LStrengthSet = m_BottomLStrengthSet(); st.m_LCurveSet = m_BottomLCurveSet();
        st.m_RAngleSet = m_BottomRAngleSet();   st.m_RSlewSet = m_BottomRSlewSet();
        st.m_RStrengthSet = m_BottomRStrengthSet(); st.m_RCurveSet = m_BottomRCurveSet();

        st.m_LAngle = m_BottomLAngle();  st.m_LSlew = -m_BottomLSlew();
        st.m_LStrength = m_BottomLStrength(); st.m_LCurve = m_BottomLCurve();
        st.m_RAngle = m_BottomRAngle();  st.m_RSlew = -m_BottomRSlew();
        st.m_RStrength = m_BottomRStrength(); st.m_RCurve = m_BottomRCurve();
    }
    else if ( side == SKIN_SIDE_LEFT )
    {
        st.m_LAngleSet = m_LeftLAngleSet();   st.m_LSlewSet = m_LeftLSlewSet();
        st.m_LStrengthSet = m_LeftLStrengthSet(); st.m_LCurveSet = m_LeftLCurveSet();
        st.m_RAngleSet = m_LeftRAngleSet();   st.m_RSlewSet = m_LeftRSlewSet();
        st.m_RStrengthSet = m_LeftRStrengthSet(); st.m_RCurveSet = m_LeftRCurveSet();

        st.m_LAngle = m_LeftLAngle();  st.m_LSlew = -m_LeftLSlew();
        st.m_LStrength = m_LeftLStrength(); st.m_LCurve = m_LeftLCurve();
        st.m_RAngle = m_LeftRAngle();  st.m_RSlew = -m_LeftRSlew();
        st.m_RStrength = m_LeftRStrength(); st.m_RCurve = m_LeftRCurve();
    }
    else
    {
        st.m_LAngleSet = m_TopLAngleSet();   st.m_LSlewSet = m_TopLSlewSet();
        st.m_LStrengthSet = m_TopLStrengthSet(); st.m_LCurveSet = m_TopLCurveSet();
        st.m_RAngleSet = m_TopRAngleSet();   st.m_RSlewSet = m_TopRSlewSet();
        st.m_RStrengthSet = m_TopRStrengthSet(); st.m_RCurveSet = m_TopRCurveSet();

        st.m_LAngle = m_TopLAngle();  st.m_LSlew = m_TopLSlew();
        st.m_LStrength = m_TopLStrength(); st.m_LCurve = m_TopLCurve();
        st.m_RAngle = m_TopRAngle();  st.m_RSlew = m_TopRSlew();
        st.m_RStrength = m_TopRStrength(); st.m_RCurve = m_TopRCurve();
    }
}

// Every station around the cross section: the four sides, plus each spine and whatever
// mirrors its symmetry flags ask for, ordered by W.
//
// A station lands on the ordered list only if it clears the minimum gap from the ones
// already there.  Two stations at the same parameter would give the control spline a zero
// length segment, and a spine dragged onto a side is better ignored than fatal.  The four
// sides go in first, so they always win a collision.
void SkinXSec::GetStations( vector< SkinStation > &stations )
{
    stations.clear();
    stations.reserve( NUM_SKIN_SIDES + 4 * m_SpineVec.size() );

    for ( int i = 0; i < NUM_SKIN_SIDES; i++ )
    {
        SkinStation st;
        GetSideStation( i, st );
        stations.push_back( st );
    }

    double t0 = GetCurve().GetCurve().get_t0();
    double period = GetCurve().GetCurve().get_tmax() - t0;
    double gap = GetMinStationGap();

    for ( int i = 0; i < ( int )m_SpineVec.size(); i++ )
    {
        SkinSpine* sp = m_SpineVec[i];
        if ( !sp )
        {
            continue;
        }

        vector< double > ws;
        vector< bool > flipslew;
        sp->GetMirrorW( ws, flipslew, period );

        for ( int k = 0; k < ( int )ws.size(); k++ )
        {
            double w = t0 + ws[k];

            bool clear = true;
            for ( int j = 0; j < ( int )stations.size(); j++ )
            {
                double d = std::abs( stations[j].m_W - w );
                if ( d > 0.5 * period )
                {
                    d = period - d;
                }
                if ( d < gap )
                {
                    clear = false;
                    break;
                }
            }

            if ( !clear )
            {
                continue;
            }

            double sgn = 1.0;
            if ( flipslew[k] )
            {
                sgn = -1.0;
            }

            SkinStation st;
            st.m_W = w;
            st.m_IsSide = false;

            st.m_LAngleSet = sp->m_LAngleSet();
            st.m_LSlewSet = sp->m_LSlewSet();
            st.m_LStrengthSet = sp->m_LStrengthSet();
            st.m_LCurveSet = sp->m_LCurveSet();
            st.m_RAngleSet = sp->m_RAngleSet();
            st.m_RSlewSet = sp->m_RSlewSet();
            st.m_RStrengthSet = sp->m_RStrengthSet();
            st.m_RCurveSet = sp->m_RCurveSet();

            st.m_LAngle = sp->m_LAngle();
            st.m_LSlew = sgn * sp->m_LSlew();
            st.m_LStrength = sp->m_LStrength();
            st.m_LCurve = sp->m_LCurve();
            st.m_RAngle = sp->m_RAngle();
            st.m_RSlew = sgn * sp->m_RSlew();
            st.m_RStrength = sp->m_RStrength();
            st.m_RCurve = sp->m_RCurve();

            stations.push_back( st );
        }
    }

    std::sort( stations.begin(), stations.end(),
               []( const SkinStation &a, const SkinStation &b ) { return a.m_W < b.m_W; } );
}

void SkinXSec::GetStationW( vector< double > &ws )
{
    vector< SkinStation > stations;
    GetStations( stations );

    ws.resize( stations.size() );
    for ( int i = 0; i < ( int )stations.size(); i++ )
    {
        ws[i] = stations[i].m_W;
    }
}

// Build the tangent (fp) and normal (fpp) curves that control the loft on one side of this
// XSec.  'left' selects the parameters that control the loft before this XSec, otherwise
// the parameters controlling the loft after it are used.
//
// The control values are laid out at the station parameters and closed back onto the first
// station, so the periodic spline through them does not care how many stations there are
// or how they are spaced.
void SkinXSec::GetSkinCrvs( bool left, piecewise_curve_type &tangentcrv, piecewise_curve_type &normcrv )
{
    vector< SkinStation > stations;
    GetStations( stations );

    GetSkinCrvs( left, stations, tangentcrv, normcrv );
}

void SkinXSec::GetSkinCrvs( bool left, const vector< SkinStation > &stations,
                            piecewise_curve_type &tangentcrv, piecewise_curve_type &normcrv )
{
    int n = stations.size();

    vector< double > ts( n + 1 );
    vector< double > angles( n + 1 );
    vector< double > slews( n + 1 );
    vector< double > strengths( n + 1 );
    vector< double > curves( n + 1 );

    double scale = GetScale();

    double t0 = GetCurve().GetCurve().get_t0();
    double tmax = GetCurve().GetCurve().get_tmax();

    for ( int i = 0; i < n; i++ )
    {
        const SkinStation &st = stations[i];

        ts[i] = st.m_W;

        if ( left )
        {
            angles[i] = st.m_LAngle * M_PI / 180.0;
            slews[i] = st.m_LSlew * M_PI / 180.0;
            strengths[i] = st.m_LStrength * scale;
            curves[i] = st.m_LCurve * scale;
        }
        else
        {
            angles[i] = st.m_RAngle * M_PI / 180.0;
            slews[i] = st.m_RSlew * M_PI / 180.0;
            strengths[i] = st.m_RStrength * scale;
            curves[i] = st.m_RCurve * scale;
        }
    }

    // Close the periodic control spline back onto the first station.
    ts[n] = t0 + ( tmax - t0 );
    angles[n] = angles[0];
    slews[n] = slews[0];
    strengths[n] = strengths[0];
    curves[n] = curves[0];

    GetTanNormCrv( ts, angles, slews, strengths, curves, tangentcrv, normcrv );
}

bool SkinXSec::AnyAngleSet( bool left, const vector< SkinStation > &stations )
{
    for ( int i = 0; i < ( int )stations.size(); i++ )
    {
        if ( left && stations[i].m_LAngleSet )
        {
            return true;
        }
        if ( !left && stations[i].m_RAngleSet )
        {
            return true;
        }
    }
    return false;
}

bool SkinXSec::AnyCurveSet( bool left, const vector< SkinStation > &stations )
{
    for ( int i = 0; i < ( int )stations.size(); i++ )
    {
        if ( left && stations[i].m_LCurveSet )
        {
            return true;
        }
        if ( !left && stations[i].m_RCurveSet )
        {
            return true;
        }
    }
    return false;
}

// One rib per station.  Every rib carries the same tangent and normal curves -- their
// values come from one periodic spline through all the stations -- but enforces only the
// conditions its own station asks for.  The surface is skinned once per rib set and the
// results blended, which keeps it continuous where a station stops enforcing something.
// Settle the parms before anything reads them.  A staged solve calls this once per XSec and
// then builds each pass from its own station list, so validation must not be buried in the
// rib build itself.
void SkinXSec::PrepRibs( bool first, bool last )
{
    if( first || last )
    {
        m_TopCont = 0;
        m_RightCont = 0;
        m_BottomCont = 0;
        m_LeftCont = 0;
    }

    ValidateParms( );
}

// Build the single rib one blending pass uses.
//
// A pass owns some stations and knows nothing about the rest.  Two things follow from that,
// and together they mean the pass never has to invent a number.
//
// It enforces only where it has something to say.  Its solution is multiplied by a blend
// weight that is one at its own stations and zero at every other, so what it does outside the
// spans touching its stations is discarded anyway.  Confining the conditions to those spans
// costs nothing and lets the loft run free everywhere else, which is exactly where the pass
// has no information.
//
// And its control values come only from its own stations.  Every station in a pass enforces
// the same things, so those values are all the user's.  Letting a station outside the pass
// contribute would put a number nobody chose into the interpolant, and a PCHIP carries a
// station's influence one span either way -- far enough to reach the spans this pass does
// enforce on.  Each station therefore takes the values of the nearest member.
void SkinXSec::GetGroupRib( bool first, bool last, const vector< SkinStation > &stations,
                            const vector< bool > &ingroup, rib_data_type &rib )
{
    int n = stations.size();

    int rep = -1;
    for ( int k = 0; k < n; k++ )
    {
        if ( k < ( int )ingroup.size() && ingroup[k] )
        {
            rep = k;
            break;
        }
    }

    if ( rep < 0 )
    {
        return;
    }

    double t0 = GetCurve().GetCurve().get_t0();
    double period = GetCurve().GetCurve().get_tmax() - t0;

    // Every station takes the values of the nearest member, measured around the section.
    vector< SkinStation > vals( n );
    for ( int k = 0; k < n; k++ )
    {
        int best = rep;
        double bestd = 2.0 * period;

        for ( int m = 0; m < n; m++ )
        {
            if ( m >= ( int )ingroup.size() || !ingroup[m] )
            {
                continue;
            }

            double d = std::abs( stations[m].m_W - stations[k].m_W );
            if ( d > 0.5 * period )
            {
                d = period - d;
            }

            if ( d < bestd )
            {
                bestd = d;
                best = m;
            }
        }

        vals[k] = stations[best];
        vals[k].m_W = stations[k].m_W;
    }

    piecewise_curve_type ltan, lnrm, rtan, rnrm;

    if ( AnyAngleSet( true, vals ) || AnyCurveSet( true, vals ) )
    {
        GetSkinCrvs( true, vals, ltan, lnrm );
    }

    if ( AnyAngleSet( false, vals ) || AnyCurveSet( false, vals ) )
    {
        GetSkinCrvs( false, vals, rtan, rnrm );
    }

    rib.set_f( GetCurve().GetCurve() );

    if( !first && !last )
    {
        rib.set_continuity( ( rib_data_type::connection_continuity ) m_TopCont() );
    }

    const SkinStation &st = stations[rep];

    // Use 'wrong' side of first cross section to set right side.
    if ( first )
    {
        if( st.m_LAngleSet ) rib.set_right_fp( ltan );
        if( st.m_LCurveSet ) rib.set_right_fpp( lnrm );
    }
    else
    {
        if( st.m_LAngleSet ) rib.set_left_fp( ltan );
        if( st.m_LCurveSet ) rib.set_left_fpp( lnrm );

        if ( !last )
        {
            if( st.m_RAngleSet ) rib.set_right_fp( rtan );
            if( st.m_RCurveSet ) rib.set_right_fpp( rnrm );
        }
    }

    // One region per span between neighboring stations, on where the span touches a station
    // this pass owns.  The stations are sorted and the section is periodic, so the last span
    // closes back onto the first station.
    vector< double > breaks;
    vector< unsigned int > masks;

    breaks.push_back( t0 );
    for ( int k = 1; k < n; k++ )
    {
        breaks.push_back( stations[k].m_W );
    }
    breaks.push_back( t0 + period );

    for ( int k = 0; k < n; k++ )
    {
        int a = k;
        int b = k + 1;
        if ( b >= n )
        {
            b = 0;
        }

        bool touches = false;
        if ( a < ( int )ingroup.size() && ingroup[a] ) touches = true;
        if ( b < ( int )ingroup.size() && ingroup[b] ) touches = true;

        if ( touches )
        {
            masks.push_back( ~0u );
        }
        else
        {
            masks.push_back( rib_data_type::CONNECTION_SET );
        }
    }

    rib.set_condition_regions( breaks, masks );
}

void SkinXSec::GetRibs( bool first, bool last, vector< rib_data_type > &ribs )
{
    PrepRibs( first, last );

    vector< SkinStation > stations;
    GetStations( stations );

    GetRibs( first, last, ribs, stations );
}

// The station list is supplied rather than read from the parms, so a caller can build the
// ribs from stations it has adjusted -- GetGroupRib hands each pass its own.
void SkinXSec::GetRibs( bool first, bool last, vector< rib_data_type > &ribs,
                        const vector< SkinStation > &stations )
{
    piecewise_curve_type ltan, lnrm, rtan, rnrm;

    if ( AnyAngleSet( true, stations ) || AnyCurveSet( true, stations ) )
    {
        GetSkinCrvs( true, stations, ltan, lnrm );
    }

    if ( AnyAngleSet( false, stations ) || AnyCurveSet( false, stations ) )
    {
        GetSkinCrvs( false, stations, rtan, rnrm );
    }

    ribs.resize( stations.size() );

    for ( int s = 0; s < ( int )stations.size(); s++ )
    {
        rib_data_type &rib = ribs[s];

        // First GetCurve() forces Update() call if needed;
        rib.set_f( GetCurve().GetCurve() );

        if( !first && !last )
            rib.set_continuity( ( rib_data_type::connection_continuity ) m_TopCont() );

        const SkinStation &st = stations[s];

        // Use 'wrong' side of first cross section to set right side.
        if ( first )
        {
            if( st.m_LAngleSet ) rib.set_right_fp( ltan );
            if( st.m_LCurveSet ) rib.set_right_fpp( lnrm );
        }
        else
        {
            if( st.m_LAngleSet ) rib.set_left_fp( ltan );
            if( st.m_LCurveSet ) rib.set_left_fpp( lnrm );

            if ( !last )
            {
                if( st.m_RAngleSet ) rib.set_right_fp( rtan );
                if( st.m_RCurveSet ) rib.set_right_fpp( rnrm );
            }
        }
    }
}

rib_data_type SkinXSec::GetRib( bool first, bool last )
{
    vector< rib_data_type > ribs;
    GetRibs( first, last, ribs );

    // Find Top by its parameter, not by counting.  GetRibs returns one rib per station in W
    // order, so index SKIN_SIDE_TOP is the Top side only while the stations are exactly the
    // four sides -- anything inserted below it shifts Top along and this quietly returned a
    // neighbour's rib instead.  Nothing else can sit at Top's parameter: a station landing
    // within GetMinStationGap of a side is merged into it.
    vector< SkinStation > stations;
    GetStations( stations );

    for ( int i = 0; i < ( int )stations.size() && i < ( int )ribs.size(); i++ )
    {
        if ( stations[i].m_W == SKIN_SIDE_TOP )
        {
            return ribs[i];
        }
    }

    return ribs[ ribs.size() - 1 ];
}

// Every quantity a station does not enforce is replaced by what surf actually did there.
//
// These are exactly the values that carry no authority: the user never set them, so the
// only meaningful thing to say about them is what the loft chose.  A pass that enforces a
// quantity somewhere still needs a value for it everywhere, and taking it from a solution
// that left it free is what keeps that pass from inventing one.
//
// Slew needs no sign flip here.  The station list is already in station space -- the sides
// negate on the way in -- and GetAngStrCrv reports in that same space.  It is SetUnsetParms
// writing back to the parms that has to flip, because the parms are in GUI space.
void SkinXSec::FillUnsetFromSurf( int irib, const VspSurf &surf, vector< SkinStation > &stations )
{
    double scale = GetScale();

    for ( int i = 0; i < ( int )stations.size(); i++ )
    {
        SkinStation &st = stations[i];

        double thetaL, phiL, strengthL, curvatureL;
        double thetaR, phiR, strengthR, curvatureR;

        GetAngStrCrv( st.m_W, irib,
                      thetaL, phiL, strengthL, curvatureL,
                      thetaR, phiR, strengthR, curvatureR,
                      surf );

        if ( !st.m_LAngleSet ) st.m_LAngle = thetaL * 180.0 / M_PI;
        if ( !st.m_LSlewSet ) st.m_LSlew = phiL * 180.0 / M_PI;
        if ( !st.m_LStrengthSet ) st.m_LStrength = strengthL / scale;
        if ( !st.m_LCurveSet ) st.m_LCurve = curvatureL / scale;

        if ( !st.m_RAngleSet ) st.m_RAngle = thetaR * 180.0 / M_PI;
        if ( !st.m_RSlewSet ) st.m_RSlew = phiR * 180.0 / M_PI;
        if ( !st.m_RStrengthSet ) st.m_RStrength = strengthR / scale;
        if ( !st.m_RCurveSet ) st.m_RCurve = curvatureR / scale;
    }
}

void SkinXSec::SetUnsetParms( int irib, const VspSurf &surf )
{
    double t0 = GetCurve().GetCurve().get_t0();
    double period = GetCurve().GetCurve().get_tmax() - t0;

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

    // Spines read back the same way, at their own W.  Unlike the four sides they carry no
    // sign convention on slew, so nothing is flipped here.  A spine whose mirrors are
    // switched on is read at its own position only: the mirrors are generated from it, so
    // reading them back would just overwrite it with a reflection of itself.
    for ( int i = 0; i < ( int )m_SpineVec.size(); i++ )
    {
        SkinSpine* sp = m_SpineVec[i];
        if ( !sp )
        {
            continue;
        }

        SetUnsetParms( t0 + sp->m_W01() * period, false, irib, surf,
                 sp->m_LAngleSet,
                 sp->m_LSlewSet,
                 sp->m_LStrengthSet,
                 sp->m_LCurveSet,
                 sp->m_RAngleSet,
                 sp->m_RSlewSet,
                 sp->m_RStrengthSet,
                 sp->m_RCurveSet,
                 sp->m_LAngle,
                 sp->m_LSlew,
                 sp->m_LStrength,
                 sp->m_LCurve,
                 sp->m_RAngle,
                 sp->m_RSlew,
                 sp->m_RStrength,
                 sp->m_RCurve );
    }
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

    if( !LAngleSet() ) LAngle = thetaL*180.0/M_PI;
    if( !LSlewSet() ) LSlew = phiL*180.0/M_PI;
    if( !LStrengthSet() ) LStrength = strengthL/scale;
    if( !LCurveSet() ) LCurve = curvatureL/scale;

    if( !RAngleSet() ) RAngle = thetaR*180.0/M_PI;
    if( !RSlewSet() ) RSlew = phiR*180.0/M_PI;
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

vector < double > SkinXSec::GetTanAngles( int side ) const
{
    vector < double > ret;

    if ( side == vsp::XSEC_LEFT_SIDE )
    {
        ret.push_back( m_TopLAngle() );
        ret.push_back( m_RightLAngle() );
        ret.push_back( m_BottomLAngle() );
        ret.push_back( m_LeftLAngle() );
    }
    else if ( side == vsp::XSEC_RIGHT_SIDE )
    {
        ret.push_back( m_TopRAngle() );
        ret.push_back( m_RightRAngle() );
        ret.push_back( m_BottomRAngle() );
        ret.push_back( m_LeftRAngle() );
    }

    return ret;
}

vector < double > SkinXSec::GetTanSlews( int side ) const
{
    vector < double > ret;

    if ( side == vsp::XSEC_LEFT_SIDE )
    {
        ret.push_back( m_TopLSlew() );
        ret.push_back( m_RightLSlew() );
        ret.push_back( m_BottomLSlew() );
        ret.push_back( m_LeftLSlew() );
    }
    else if ( side == vsp::XSEC_RIGHT_SIDE )
    {
        ret.push_back( m_TopRSlew() );
        ret.push_back( m_RightRSlew() );
        ret.push_back( m_BottomRSlew() );
        ret.push_back( m_LeftRSlew() );
    }

    return ret;
}

vector < double > SkinXSec::GetTanStrengths( int side ) const
{
    vector < double > ret;

    if ( side == vsp::XSEC_LEFT_SIDE )
    {
        ret.push_back( m_TopLStrength() );
        ret.push_back( m_RightLStrength() );
        ret.push_back( m_BottomLStrength() );
        ret.push_back( m_LeftLStrength() );
    }
    else if ( side == vsp::XSEC_RIGHT_SIDE )
    {
        ret.push_back( m_TopRStrength() );
        ret.push_back( m_RightRStrength() );
        ret.push_back( m_BottomRStrength() );
        ret.push_back( m_LeftRStrength() );
    }

    return ret;
}

vector < double > SkinXSec::GetCurvatures( int side ) const
{
    vector < double > ret;

    if ( side == vsp::XSEC_LEFT_SIDE )
    {
        ret.push_back( m_TopLCurve() );
        ret.push_back( m_RightLCurve() );
        ret.push_back( m_BottomLCurve() );
        ret.push_back( m_LeftLCurve() );
    }
    else if ( side == vsp::XSEC_RIGHT_SIDE )
    {
        ret.push_back( m_TopRCurve() );
        ret.push_back( m_RightRCurve() );
        ret.push_back( m_BottomRCurve() );
        ret.push_back( m_LeftRCurve() );
    }

    return ret;
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
        else                            { m_RightLAngle = m_TopLAngle(); }

        if ( bottom > XSEC_NO_VAL )     { m_BottomLAngle = bottom; }
        else                            { m_BottomLAngle = m_TopLAngle(); }

        if ( left > XSEC_NO_VAL )       { m_LeftLAngle = left; }
        else                            { m_LeftLAngle = m_RightLAngle(); }
    }

    if ( side == XSEC_BOTH_SIDES || side == XSEC_RIGHT_SIDE )
    {
        m_TopRAngleSet = true;
        m_RightRAngleSet = true;
        m_BottomRAngleSet = true;
        m_LeftRAngleSet = true;

        m_TopRAngle = top;
        if ( right > XSEC_NO_VAL )      { m_RightRAngle = right; }
        else                            { m_RightRAngle = m_TopRAngle(); }

        if ( bottom > XSEC_NO_VAL )     { m_BottomRAngle = bottom; }
        else                            { m_BottomRAngle = m_TopRAngle(); }

        if ( left > XSEC_NO_VAL )       { m_LeftRAngle = left; }
        else                            { m_LeftRAngle = m_RightRAngle(); }
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
        else                            { m_RightLSlew = m_TopLSlew(); }

        if ( bottom > XSEC_NO_VAL )     { m_BottomLSlew = bottom; }
        else                            { m_BottomLSlew = m_TopLSlew(); }

        if ( left > XSEC_NO_VAL )       { m_LeftLSlew = left; }
        else                            { m_LeftLSlew = m_RightLSlew(); }
    }

    if ( side == XSEC_BOTH_SIDES || side == XSEC_RIGHT_SIDE )
    {
        m_TopRSlewSet = true;
        m_RightRSlewSet = true;
        m_BottomRSlewSet = true;
        m_LeftRSlewSet = true;

        m_TopRSlew = top;
        if ( right > XSEC_NO_VAL )      { m_RightRSlew = right; }
        else                            { m_RightRSlew = m_TopRSlew(); }

        if ( bottom > XSEC_NO_VAL )     { m_BottomRSlew = bottom; }
        else                            { m_BottomRSlew = m_TopRSlew(); }

        if ( left > XSEC_NO_VAL )       { m_LeftRSlew = left; }
        else                            { m_LeftRSlew = m_RightRSlew(); }
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
        else                            { m_RightLStrength = m_TopLStrength(); }

        if ( bottom > XSEC_NO_VAL )     { m_BottomLStrength = bottom; }
        else                            { m_BottomLStrength = m_TopLStrength(); }

        if ( left > XSEC_NO_VAL )       { m_LeftLStrength = left; }
        else                            { m_LeftLStrength = m_RightLStrength(); }
    }

    if ( side == XSEC_BOTH_SIDES || side == XSEC_RIGHT_SIDE )
    {
        m_TopRStrengthSet = true;
        m_RightRStrengthSet = true;
        m_BottomRStrengthSet = true;
        m_LeftRStrengthSet = true;

        m_TopRStrength = top;
        if ( right > XSEC_NO_VAL )      { m_RightRStrength = right; }
        else                            { m_RightRStrength = m_TopRStrength(); }

        if ( bottom > XSEC_NO_VAL )     { m_BottomRStrength = bottom; }
        else                            { m_BottomRStrength = m_TopRStrength(); }

        if ( left > XSEC_NO_VAL )       { m_LeftRStrength = left; }
        else                            { m_LeftRStrength = m_RightRStrength(); }
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
        else                            { m_RightLCurve = m_TopLCurve(); }

        if ( bottom > XSEC_NO_VAL )     { m_BottomLCurve = bottom; }
        else                            { m_BottomLCurve = m_TopLCurve(); }

        if ( left > XSEC_NO_VAL)        { m_LeftLCurve = left; }
        else                            { m_LeftLCurve = m_RightLCurve(); }
    }

    if ( side == XSEC_BOTH_SIDES || side == XSEC_RIGHT_SIDE )
    {
        m_TopRCurveSet = true;
        m_RightRCurveSet = true;
        m_BottomRCurveSet = true;
        m_LeftRCurveSet = true;

        m_TopRCurve = top;
        if ( right > XSEC_NO_VAL )      { m_RightRCurve = right; }
        else                            { m_RightRCurve = m_TopRCurve(); }

        if ( bottom > XSEC_NO_VAL )     { m_BottomRCurve = bottom; }
        else                            { m_BottomRCurve = m_TopRCurve(); }

        if ( left > XSEC_NO_VAL )       { m_LeftRCurve = left; }
        else                            { m_LeftRCurve = m_RightRCurve(); }
    }
}

void SkinXSec::FlipLRSkinning()
{
    double ang, slew, str, curv;

    // Remap Top
    ang  = m_TopLAngle();
    slew = m_TopLSlew();
    str  = m_TopLStrength();
    curv = m_TopLCurve();
    m_TopLAngle    = m_TopRAngle();
    m_TopLSlew     = m_TopRSlew();
    m_TopLStrength = m_TopRStrength();
    m_TopLCurve    = m_TopRCurve();
    m_TopRAngle    = ang;
    m_TopRSlew     = slew;
    m_TopRStrength = str;
    m_TopRCurve    = curv;

    // Remap Right
    ang  = m_RightLAngle();
    slew = m_RightLSlew();
    str  = m_RightLStrength();
    curv = m_RightLCurve();
    m_RightLAngle    = m_RightRAngle();
    m_RightLSlew     = m_RightRSlew();
    m_RightLStrength = m_RightRStrength();
    m_RightLCurve    = m_RightRCurve();
    m_RightRAngle    = ang;
    m_RightRSlew     = slew;
    m_RightRStrength = str;
    m_RightRCurve    = curv;

    // Remap Bottom
    ang  = m_BottomLAngle();
    slew = m_BottomLSlew();
    str  = m_BottomLStrength();
    curv = m_BottomLCurve();
    m_BottomLAngle    = m_BottomRAngle();
    m_BottomLSlew     = m_BottomRSlew();
    m_BottomLStrength = m_BottomRStrength();
    m_BottomLCurve    = m_BottomRCurve();
    m_BottomRAngle    = ang;
    m_BottomRSlew     = slew;
    m_BottomRStrength = str;
    m_BottomRCurve    = curv;

    // Remap Left
    ang  = m_LeftLAngle();
    slew = m_LeftLSlew();
    str  = m_LeftLStrength();
    curv = m_LeftLCurve();
    m_LeftLAngle    = m_LeftRAngle();
    m_LeftLSlew     = m_LeftRSlew();
    m_LeftLStrength = m_LeftRStrength();
    m_LeftLCurve    = m_LeftRCurve();
    m_LeftRAngle    = ang;
    m_LeftRSlew     = slew;
    m_LeftRStrength = str;
    m_LeftRCurve    = curv;
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

void SkinXSec::ClearSkinning()
{
    m_AllSymFlag = true;
    m_TopLRAngleEq = false;
    m_TopLRStrengthEq = false;
    m_TopLRSlewEq = false;
    m_TopLRCurveEq = false;

    // necessary to validate parms here, as disabling skinning requires multiple operations in order
    // AllSym = true, and all LR equal = false in order to permit disabling the setter bool parms.
    ValidateParms();

    m_TopLAngleSet = false;
    m_TopLSlewSet = false;
    m_TopLStrengthSet = false;
    m_TopLCurveSet = false;
    m_TopRAngleSet = false;
    m_TopRSlewSet = false;
    m_TopRStrengthSet = false;
    m_TopRCurveSet = false;

    // And the equality flags again.  From C1 up, validation derives each of them from the
    // matching left hand Set flag -- LRAngleEq from LAngleSet -- so the pass above turns
    // back on exactly what the four lines before it turned off.  The next validation then
    // reads those equality flags and forces the Set flags back on with them, which left
    // Clear Skinning clearing nothing whatever at C1 or C2.
    m_TopLRAngleEq = false;
    m_TopLRStrengthEq = false;
    m_TopLRSlewEq = false;
    m_TopLRCurveEq = false;
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
    m_TopLAngle = atan( XmlUtil::FindDouble( root, "Profile_Tan_Ang", 0.0 ) ) * 180.0 / M_PI;
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

    m_FwdCluster.Init( "FwdCluster", m_GroupName, this, 1.0, 1e-4, 10.0 );
    m_FwdCluster.SetDescript( "Forward Tess Cluster Control" );
    m_AftCluster.Init( "AftCluster", m_GroupName, this, 1.0, 1e-4, 10.0 );
    m_AftCluster.SetDescript( "Aft Tess Cluster Control" );

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

    m_FwdCluster.Init( "FwdCluster", m_GroupName, this, 1.0, 1e-4, 10.0 );
    m_FwdCluster.SetDescript( "Forward Tess Cluster Control" );
    m_AftCluster.Init( "AftCluster", m_GroupName, this, 1.0, 1e-4, 10.0 );
    m_AftCluster.SetDescript( "Aft Tess Cluster Control" );

    m_VAlign.Init( "VAlign", m_GroupName, this, 0.0, -10.0, 10.0 );
    m_VAlign.SetDescript( "Vertical alignment of cross section" );

    m_XDelta.Init( "XDelta", m_GroupName, this,  0.0, -1.0e12, 1.0e12 );
    m_XDelta.SetDescript( "X distance of cross section from prior cross section" );
    m_YDelta.Init( "YDelta", m_GroupName, this,  0.0, -1.0e12, 1.0e12 );
    m_YDelta.SetDescript( "Y distance of cross section from prior cross section" );
    m_ZDelta.Init( "ZDelta", m_GroupName, this,  0.0, -1.0e12, 1.0e12 );
    m_ZDelta.SetDescript( "Z distance of cross section from prior cross section" );

    m_XSAbsRelFlag.Init( "XSAbsRelFlag", m_GroupName, this, vsp::REL, vsp::ABS, vsp::REL );

    m_XAbs.Init( "XAbs", m_GroupName, this,  0.0, -1.0e12, 1.0e12 );
    m_XAbs.SetDescript( "Absolute X position of cross section" );
    m_YAbs.Init( "YAbs", m_GroupName, this,  0.0, -1.0e12, 1.0e12 );
    m_YAbs.SetDescript( "Absolute Y position of cross section" );
    m_ZAbs.Init( "ZAbs", m_GroupName, this,  0.0, -1.0e12, 1.0e12 );
    m_ZAbs.SetDescript( "Absolute Z position of cross section" );

    m_XRotate.Init( "XRotate", m_GroupName, this,  0.0, -180.0, 180.0 );
    m_XRotate.SetDescript( "Rotation about x-axis of cross section" );
    m_YRotate.Init( "YRotate", m_GroupName, this,  0.0, -180.0, 180.0 );
    m_YRotate.SetDescript( "Rotation about y-axis of cross section" );
    m_ZRotate.Init( "ZRotate", m_GroupName, this,  0.0, -180.0, 180.0 );
    m_ZRotate.SetDescript( "Rotation about z-axis of cross section" );

    m_XRotateAbs.Init( "XRotateAbs", m_GroupName, this,  0.0, -180.0, 180.0 );
    m_XRotateAbs.SetDescript( "Absolute rotation about x-axis of cross section" );
    m_YRotateAbs.Init( "YRotateAbs", m_GroupName, this,  0.0, -180.0, 180.0 );
    m_YRotateAbs.SetDescript( "Absolute rotation about y-axis of cross section" );
    m_ZRotateAbs.Init( "ZRotateAbs", m_GroupName, this,  0.0, -180.0, 180.0 );
    m_ZRotateAbs.SetDescript( "Absolute rotation about z-axis of cross section" );

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
    // Deactivate non driving parms and Activate driving parms
    if ( m_XSAbsRelFlag() ==  vsp::REL  )
    {
        m_XAbs.Deactivate();
        m_YAbs.Deactivate();
        m_ZAbs.Deactivate();

        m_XDelta.Activate();
        m_YDelta.Activate();
        m_ZDelta.Activate();

        m_XRotateAbs.Deactivate();
        m_YRotateAbs.Deactivate();
        m_ZRotateAbs.Deactivate();

        m_XRotate.Activate();
        m_YRotate.Activate();
        m_ZRotate.Activate();
    }
    else
    {
        m_XDelta.Deactivate();
        m_YDelta.Deactivate();
        m_ZDelta.Deactivate();

        m_XAbs.Activate();
        m_YAbs.Activate();
        m_ZAbs.Activate();

        m_XRotate.Deactivate();
        m_YRotate.Deactivate();
        m_ZRotate.Deactivate();

        m_XRotateAbs.Activate();
        m_YRotateAbs.Activate();
        m_ZRotateAbs.Activate();
    }


    m_LateUpdateFlag = false;

    XSecSurf* xsecsurf = (XSecSurf*) GetParentContainerPtr();
    int indx = xsecsurf->FindXSecIndex( m_ID );

    // apply the needed transformation to get section into body orientation

    Matrix4d mat;
    xsecsurf->GetBasicTransformation( m_XSCurve->GetWidth(), mat );

    VspCurve baseCurve = GetUntransformedCurve();

    // This is really a Z translation for the Stack, but it is applied before the
    // BasicTransformation is applied.  Consequently, it is a y transformation
    // (to the untransformed curve).
    mat.translatef( 0, -m_VAlign() * 0.5 * m_XSCurve->GetHeight(), 0 );

    baseCurve.Transform( mat );

    //==== Apply Transform ====//
    m_TransformedCurve = baseCurve;

    m_TransformedCurve.Spin01( m_Spin() );

    m_Transform.loadIdentity();

    if ( m_XSAbsRelFlag() ==  vsp::REL  )
    {

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

        vec3d trans = m_Transform.getTranslation();
        m_XAbs = trans.x();
        m_YAbs = trans.y();
        m_ZAbs = trans.z();

        vec3d angles = m_Transform.getAngles();
        m_XRotateAbs = angles.x();
        m_YRotateAbs = angles.y();
        m_ZRotateAbs = angles.z();
    }
    else
    {
        Matrix4d prevmat;
        if( indx > 0 )
        {
            StackXSec* prevxs = (StackXSec*) xsecsurf->FindXSec( indx - 1);
            if( prevxs )
            {
                prevmat.matMult( prevxs->GetTransform()->data() );
            }
        }

        m_Transform.translatef( m_XAbs(), m_YAbs(), m_ZAbs() );

        m_Transform.rotateX( m_XRotateAbs() );
        m_Transform.rotateY( m_YRotateAbs() );
        m_Transform.rotateZ( m_ZRotateAbs() );

        m_TransformedCurve.Transform( m_Transform );

        prevmat.affineInverse();
        prevmat.matMult( m_Transform.data() );

        vec3d trans = prevmat.getTranslation();
        m_XDelta = trans.x();
        m_YDelta = trans.y();
        m_ZDelta = trans.z();

        vec3d angles = prevmat.getAngles();
        m_XRotate = angles.x();
        m_YRotate = angles.y();
        m_ZRotate = angles.z();
    }


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

        m_VAlign = sxs->m_VAlign();

        m_XDelta = sxs->m_XDelta();
        m_YDelta = sxs->m_YDelta();
        m_ZDelta = sxs->m_ZDelta();

        m_XSAbsRelFlag = sxs->m_XSAbsRelFlag();
        m_XAbs = sxs->m_XAbs();
        m_YAbs = sxs->m_YAbs();
        m_ZAbs = sxs->m_ZAbs();

        m_Spin = sxs->m_Spin();

        m_XRotate = sxs->m_XRotate();
        m_YRotate = sxs->m_YRotate();
        m_ZRotate = sxs->m_ZRotate();

        m_XRotateAbs = sxs->m_XRotateAbs();
        m_YRotateAbs = sxs->m_YRotateAbs();
        m_ZRotateAbs = sxs->m_ZRotateAbs();

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
