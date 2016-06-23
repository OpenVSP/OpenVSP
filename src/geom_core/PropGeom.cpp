//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//
//
//////////////////////////////////////////////////////////////////////

#include "PropGeom.h"
#include "ParmMgr.h"
#include "VspSurf.h"
#include "Vehicle.h"
#include "StlHelper.h"
#include <float.h>

#include "APIDefines.h"
using namespace vsp;

// Fix thickness/height scaling for non-airfoil Xsec types.

// Remove control points
// Beautify control point editing GUI
// Renumbering of parameters when delete points.

// Change curve type from API
// Curve splitting from API
// Other API additions?

// Use unit circle surrogate for point XSec.  Or disable point type.

// Allow curve control of thickness.

// v2 import

// Nice default prop

// Import / Export blade element format.
// Support curved section shapes for Nick

// New section types
// Easy prop reversal
// Consider folding propeller

// Skinning options (PCHIP, Linear, etc.)
// Adaptive skinning


//==========================================================================//
//==========================================================================//
//==========================================================================//

PropPositioner::PropPositioner()
{
    m_ParentProp = NULL;;

    m_Chord = 1.0;

    m_Twist = 0.0;
    m_PropRot = 0.0;
    m_Feather = 0.0;
    m_XRotate = 0.0;
    m_ZRotate = 0.0;

    m_Radius = 0.0;
    m_Rake = 0.0;
    m_Skew = 0.0;

    m_NeedsUpdate = true;

}


void PropPositioner::Update()
{
    m_NeedsUpdate = false;

    m_TransformedCurve = m_Curve;

    Matrix4d mat;
    mat.scale( m_Chord );
    m_TransformedCurve.Transform( mat );

    if ( !m_ParentProp )
    {
        return;
    }

    // Basic transformation orients curve before other transformations.
    m_ParentProp->GetBasicTransformation( m_Chord, mat );
    m_TransformedCurve.Transform( mat );

    m_Transform.loadIdentity();

    // Propeller rotation first because order is reversed.
    m_Transform.rotateX( -m_PropRot );
    m_Transform.rotateY( m_Feather );

    m_Transform.translatef( 0, m_Radius, 0 );

    m_Transform.rotateY( m_Twist );

    m_Transform.rotateX( m_XRotate ); // About rake direction

    m_Transform.translatef( m_Rake, 0, m_Skew );

    m_Transform.rotateZ( m_ZRotate ); // About chord

    m_TransformedCurve.Transform( m_Transform );
}

void PropPositioner::SetCurve( const VspCurve &c )
{
    m_Curve = c;
    m_NeedsUpdate = true;
}

VspCurve PropPositioner::GetCurve()
{
    if ( m_NeedsUpdate )
    {
        Update();
    }
    return m_TransformedCurve;
}

//==========================================================================//
//==========================================================================//
//==========================================================================//

//==== Default Constructor ====//
PropXSec::PropXSec( XSecCurve *xsc ) : XSec( xsc )
{
    m_Type = XSEC_PROP;

    m_RadiusFrac.Init( "RadiusFrac", m_GroupName, this,  0.0, 0.0, 1.0 );
    m_RadiusFrac.SetDescript( "Radius of cross section as a fraction of prop radius" );

    m_RefLength.Init( "RefLength", m_GroupName, this, 1.0, 1e-8, 1e12 );
    m_RefLenVal = 1.0;
}

//==== Update ====//
void PropXSec::Update()
{
    m_LateUpdateFlag = false;

    //==== Apply Transform ====//
    m_TransformedCurve = GetUntransformedCurve();

    m_Transform.loadIdentity();

    m_Transform.translatef( 0, 0, m_RadiusFrac() );

    m_TransformedCurve.Transform( m_Transform );
}

//==== Set Ref Length ====//
void PropXSec::SetRefLength( double len )
{
    if ( fabs( len - m_RefLenVal ) < DBL_EPSILON )
    {
        return;
    }

    m_RefLength = len;
    m_LateUpdateFlag = true;

    m_RadiusFrac.SetRefVal( m_RefLength() );

    m_RefLenVal = m_RefLength();
}

//==== Copy position from base class ====//
// May be possible to do this using ParmContainer::EncodeXML, but all
// we want to do is copy the values in the XSec (not XSecCurve) class
// that control position.
void PropXSec::CopyBasePos( XSec* xs )
{
    if ( xs )
    {
        PropXSec* pxs = ( PropXSec* ) xs;

        m_RadiusFrac = pxs->m_RadiusFrac();

        m_RefLength = pxs->m_RefLength();
    }
}

vector< vec3d > PropXSec::GetDrawLines( Matrix4d &transMat  )
{
    vector< vec3d > lines;

    VspCurve curve = GetCurve();

    curve.Scale( 1.0 / m_PropPos.m_Chord );

    vec3d v = curve.CompPnt( 0 );
    curve.OffsetZ( -v.z() );

    m_PropPos.SetCurve( curve );
    m_PropPos.Update();
    curve = m_PropPos.GetCurve();

    curve.Transform( transMat );
    curve.TessAdapt( lines, 1e-2, 10 );

    return lines;
}

//==========================================================================//
//==========================================================================//
//==========================================================================//

//==== Constructor ====//
PropGeom::PropGeom( Vehicle* vehicle_ptr ) : GeomXSec( vehicle_ptr )
{
    m_Name = "PropGeom";
    m_Type.m_Name = "Propeller";
    m_Type.m_Type = PROP_GEOM_TYPE;

    m_XSecSurf.SetBasicOrientation( Y_DIR, Z_DIR, XS_SHIFT_MID, true );

    m_XSecSurf.SetParentContainer( GetID() );

    //==== Init Parms ====//
    m_TessU = 12;
    m_TessW = 17;
    m_TessW.SetMultShift( 8, 1 );

    m_Diameter.Init( "Diameter", "Design", this, 30.0, 1.0e-8, 1.0e12 );
    m_Diameter.SetDescript( "Propeller diameter" );

    m_Nblade.Init( "NumBlade", "Design", this, 3, 1, 1000 );
    m_Nblade.SetDescript( "Number of propeller blades" );

    m_Rotate.Init( "Rotate", "Design", this, 0.0, -360.0, 360.0 );
    m_Rotate.SetDescript( "Rotation of first propeller blade." );

    m_Beta34.Init( "Beta34", "Design", this, 20.0, -400.0, 400.0 );
    m_Beta34.SetDescript( "Blade pitch at 3/4 of radius" );

    m_Feather.Init( "Feather", "Design", this, 0.0, -400.0, 400.0 );
    m_Feather.SetDescript( "Blade feather angle" );

    m_UseBeta34Flag.Init( "UseBeta34Flag", "Design", this, 1, 0, 1 );
    m_UseBeta34Flag.SetDescript( "Flag to use Beta34 or Beta0 as driver" );

    m_AFLimit.Init( "AFLimit", "Design", this, 0.2, 0, 1 );
    m_AFLimit.SetDescript( "Lower limit of activity factor integration" );

    m_AF.Init( "AF", "Design", this, 100, 0, 1e12 );
    m_AF.SetDescript( "Propeller activity factor" );

    m_LECluster.Init( "LECluster", "Design", this, 0.2, 0.0, 2.0 );
    m_LECluster.SetDescript( "LE Tess Cluster Control" );

    m_TECluster.Init( "TECluster", "Design", this, 1.0, 0.0, 2.0 );
    m_TECluster.SetDescript( "TE Tess Cluster Control" );

    m_RootCluster.Init( "InCluster", "Design", this, 1.0, 0.0, 2.0 );
    m_RootCluster.SetDescript( "Inboard Tess Cluster Control" );

    m_TipCluster.Init( "OutCluster", "Design", this, 1.0, 0.0, 2.0 );
    m_TipCluster.SetDescript( "Outboard Tess Cluster Control" );

    //==== rename capping controls for wing specific terminology ====//
    m_CapUMinOption.SetDescript( "Type of End Cap on Propeller Root" );
    m_CapUMinOption.Parm::Set( FLAT_END_CAP );
    m_CapUMinTess.SetDescript( "Number of tessellated curves on Propeller Root and Tip" );
    m_CapUMaxOption.SetDescript( "Type of End Cap on Propeller Tip" );
    m_CapUMaxOption.Parm::Set( ROUND_END_CAP );

    m_ActiveXSec = 0;

    m_XSecSurf.SetXSecType( XSEC_PROP );

    m_XSecSurf.AddXSec( XS_CIRCLE );
    m_XSecSurf.AddXSec( XS_FOUR_SERIES );
    m_XSecSurf.AddXSec( XS_FOUR_SERIES );
    m_XSecSurf.AddXSec( XS_FOUR_SERIES );
    m_XSecSurf.AddXSec( XS_FOUR_SERIES );

    int j;
    PropXSec* xs;

    j = 0;
    xs = ( PropXSec* ) m_XSecSurf.FindXSec( j );
    xs->SetGroupDisplaySuffix( j );
    xs->m_RadiusFrac = 0.2;

    ++j;
    xs = ( PropXSec* ) m_XSecSurf.FindXSec( j );
    xs->SetGroupDisplaySuffix( j );
    xs->m_RadiusFrac = 0.4;

    ++j;
    xs = ( PropXSec* ) m_XSecSurf.FindXSec( j );
    xs->SetGroupDisplaySuffix( j );
    xs->m_RadiusFrac = 0.6;

    ++j;
    xs = ( PropXSec* ) m_XSecSurf.FindXSec( j );
    xs->SetGroupDisplaySuffix( j );
    xs->m_RadiusFrac = 0.8;

    ++j;
    xs = ( PropXSec* ) m_XSecSurf.FindXSec( j );
    xs->SetGroupDisplaySuffix( j );
    xs->m_RadiusFrac = 1.0;


    m_ChordCurve.SetParentContainer( GetID() );
    m_ChordCurve.SetDispNames( "r/R", "Chord/R" );
    m_ChordCurve.SetParmNames( "r", "crd" );
    m_ChordCurve.SetCurveName( "Chord" );
    m_ChordCurve.InitParms();
    m_ChordCurve.m_CurveType = PCurve::CEDIT;
    static const double t1[] = {0.2, 0.2 + 0.55 / 3.0, 0.75 - 0.55 / 3.0, 0.75,
                             0.75 + 0.25 / 3.0, 1.0 - 0.25 / 3.0, 1.0};
    static const double v1[] = {0.08, 0.15, 0.21, 0.21,
                             0.21, 0.25, 0.01};
    vector < double > tv1( t1, t1 + sizeof( t1 ) / sizeof( t1[0] ) );
    vector < double > vv1( v1, v1 + sizeof( v1 ) / sizeof( v1[0] ) );
    m_ChordCurve.InitCurve( tv1, vv1 );

    m_TwistCurve.SetParentContainer( GetID() );
    m_TwistCurve.SetDispNames( "r/R", "Twist" );
    m_TwistCurve.SetParmNames( "r", "tw" );
    m_TwistCurve.SetCurveName( "Twist" );
    m_TwistCurve.InitParms();
    m_TwistCurve.m_CurveType = PCurve::PCHIP;
    static const double t2[] = {0.2, 0.75, 1.0};
    static const double v2[] = {46.75, 20.0, 13.0};
    vector < double > tv2( t2, t2 + sizeof( t2 ) / sizeof( t2[0] ) );
    vector < double > vv2( v2, v2 + sizeof( v2 ) / sizeof( v2[0] ) );
    m_TwistCurve.InitCurve( tv2, vv2 );

    m_XRotateCurve.SetParentContainer( GetID() );
    m_XRotateCurve.SetDispNames( "r/R", "X Rotate" );
    m_XRotateCurve.SetParmNames( "r", "xr" );
    m_XRotateCurve.SetCurveName( "XRotate" );
    m_XRotateCurve.InitParms();
    m_XRotateCurve.m_CurveType = PCurve::LINEAR;
    static const double t3[] = {0.2, 1.0};
    static const double v3[] = {0.0, 0.0};
    vector < double > tv3( t3, t3 + sizeof( t3 ) / sizeof( t3[0] ) );
    vector < double > vv3( v3, v3 + sizeof( v3 ) / sizeof( v3[0] ) );
    m_XRotateCurve.InitCurve( tv3, vv3 );

    m_ZRotateCurve.SetParentContainer( GetID() );
    m_ZRotateCurve.SetDispNames( "r/R", "Z Rotate" );
    m_ZRotateCurve.SetParmNames( "r", "zr" );
    m_ZRotateCurve.SetCurveName( "ZRotate" );
    m_ZRotateCurve.InitParms();
    m_ZRotateCurve.m_CurveType = PCurve::LINEAR;
    m_ZRotateCurve.InitCurve( tv3, vv3 );

    m_RakeCurve.SetParentContainer( GetID() );
    m_RakeCurve.SetDispNames( "r/R", "Rake/R" );
    m_RakeCurve.SetParmNames( "r", "rak" );
    m_RakeCurve.SetCurveName( "Rake" );
    m_RakeCurve.InitParms();
    m_RakeCurve.m_CurveType = PCurve::LINEAR;
    m_RakeCurve.InitCurve( tv3, vv3 );

    m_SkewCurve.SetParentContainer( GetID() );
    m_SkewCurve.SetDispNames( "r/R", "Skew/R" );
    m_SkewCurve.SetParmNames( "r", "skw" );
    m_SkewCurve.SetCurveName( "Skew" );
    m_SkewCurve.InitParms();
    m_SkewCurve.m_CurveType = PCurve::LINEAR;
    m_SkewCurve.InitCurve( tv3, vv3 );

    // Set up vector to allow treatment as a group.
    m_pcurve_vec.resize( 6 );
    m_pcurve_vec[0] = &m_ChordCurve;
    m_pcurve_vec[1] = &m_TwistCurve;
    m_pcurve_vec[2] = &m_XRotateCurve;
    m_pcurve_vec[3] = &m_ZRotateCurve;
    m_pcurve_vec[4] = &m_RakeCurve;
    m_pcurve_vec[5] = &m_SkewCurve;

}

//==== Destructor ====//
PropGeom::~PropGeom()
{

}

void PropGeom::ChangeID( string id )
{
    Geom::ChangeID( id );
    m_XSecSurf.SetParentContainer( GetID() );
}

//==== Update Fuselage And Cross Section Placement ====//
void PropGeom::UpdateSurf()
{
    int nxsec = m_XSecSurf.NumXSec();

    double radius = m_Diameter() / 2.0;

    double rfirst = 0.0;
    double rlast = 1.0;

    vector < double > uvec( nxsec );
    vector < double > rvec( nxsec );

    //==== Update XSec Location/Rotation ====//
    for ( int i = 0 ; i < nxsec ; i++ )
    {
        PropXSec* xs = ( PropXSec* ) m_XSecSurf.FindXSec( i );

        if ( xs )
        {
            //==== Reset Group Names ====//
            xs->SetGroupDisplaySuffix( i );

            //==== Set X Limits ====//
            EnforceOrder( xs, i );

            xs->SetRefLength( radius );

            bool first = false;
            bool last = false;

            if( i == 0 ) first = true;
            else if( i == (nxsec-1) ) last = true;

            if ( first )
            {
                rfirst = xs->m_RadiusFrac();
            }
            if ( last )
            {
                rlast = xs->m_RadiusFrac();
            }

            uvec[i] = i;
            rvec[i] = xs->m_RadiusFrac();
        }
    }

    Vsp1DCurve rtou;
    rtou.InterpolateLinear( uvec, rvec, false );

    EnforcePCurveOrder( rfirst, rlast );

    // Set lower limit for activity factor integration limit
    m_AFLimit.SetLowerLimit( rfirst );
    // Integrate activity factor.
    m_AF.Set( m_ChordCurve.IntegrateAF( m_AFLimit() ) );
    m_AF.Deactivate();

    if ( m_UseBeta34Flag() == 1 )
    {
        if ( rfirst <= 0.75 )
        {
            double theta34 = m_TwistCurve.Comp( 0.75 );
            m_Feather = m_Beta34() - theta34;
        }
        else
        {
            m_Feather = m_Beta34();
        }
        m_Beta34.Activate();
        m_Feather.Deactivate();
    }
    else
    {
        if ( rfirst <= 0.75 )
        {
            double theta34 = m_TwistCurve.Comp( 0.75 );
            m_Beta34 = m_Feather() + theta34;
        }
        else
        {
            m_Beta34 = m_Feather();
        }
        m_Beta34.Deactivate();
        m_Feather.Activate();
    }


    //==== Cross Section Curves & joint info ====//
    vector< rib_data_type > rib_vec;
    vector< VspCurve > crv_vec;
    rib_vec.resize( nxsec );
    crv_vec.resize( nxsec );

    //==== Update XSec Location/Rotation ====//
    for ( int i = 0 ; i < nxsec ; i++ )
    {
        PropXSec* xs = ( PropXSec* ) m_XSecSurf.FindXSec( i );

        if ( xs )
        {
            //==== Find Width Parm ====//
            string width_id = xs->GetXSecCurve()->GetWidthParmID();
            Parm* width_parm = ParmMgr.FindParm( width_id );

            double r = xs->m_RadiusFrac();
            double w = m_ChordCurve.Comp( r ) * radius;

            piecewise_curve_type pwc;
            if ( width_parm )
            {
                width_parm->Deactivate();
                width_parm->Set( 1.0 );
                pwc = xs->GetCurve().GetCurve();
                width_parm->Set( w );
            }
            else
            {
                pwc = xs->GetCurve().GetCurve();
            }

            // Set up prop positioner for highlight curves - not lofting.
            xs->m_PropPos.m_ParentProp = GetXSecSurf( 0 );
            xs->m_PropPos.m_Radius = r * radius;
            xs->m_PropPos.m_Chord = w;
            xs->m_PropPos.m_Twist = m_TwistCurve.Comp( r );
            xs->m_PropPos.m_XRotate = m_XRotateCurve.Comp( r );
            xs->m_PropPos.m_ZRotate = m_ZRotateCurve.Comp( r );
            xs->m_PropPos.m_Rake = m_RakeCurve.Comp( r ) * radius;
            xs->m_PropPos.m_Skew = m_SkewCurve.Comp( r ) * radius;
            xs->m_PropPos.m_PropRot = m_Rotate();
            xs->m_PropPos.m_Feather = m_Feather();

            rib_vec[i].set_f( pwc );
            crv_vec[i].SetCurve( pwc );
        }
    }




    // This surface linearly interpolates the airfoil sections without
    // any other transformations.
    // These sections can be extracted (as u-const curves) and then
    // transformed to their final position before skinning.
    VspSurf foilsurf;
    foilsurf.SkinC0( crv_vec, false );

    crv_vec.clear();
    rib_vec.clear();

    // Pseudo cross sections
    // Not directly user-controlled, but an intermediate step in lofting the
    // surface.
    int npseudo = 31;


    crv_vec.resize( npseudo );
    rib_vec.resize( npseudo );



    for ( int i = 0; i < npseudo; i++ )
    {
        // Assume linear interpolation means linear u/r relationship.
        double r = rfirst + ( rlast - rfirst ) * (double) i/ (double) ( npseudo - 1 );
        double u = rtou.CompPnt( r );

        VspCurve c;
        foilsurf.GetUConstCurve( c, u );
        vec3d v = c.CompPnt( 0 );
        c.OffsetZ( -v.z() );

        PropPositioner pp;

        pp.m_ParentProp = this->GetXSecSurf( 0 );
        pp.m_Radius = r * radius;

        pp.m_Chord = m_ChordCurve.Comp( r ) * radius;
        pp.m_Twist = m_TwistCurve.Comp( r );

        pp.m_XRotate = m_XRotateCurve.Comp( r );
        pp.m_ZRotate = m_ZRotateCurve.Comp( r );

        pp.m_Rake = m_RakeCurve.Comp( r ) * radius;
        pp.m_Skew = m_SkewCurve.Comp( r ) * radius;

        pp.m_PropRot = m_Rotate();
        pp.m_Feather = m_Feather();

        // Set a bunch of other pp variables.
        pp.SetCurve( c );
        pp.Update();

        crv_vec[i] = pp.GetCurve();
        rib_vec[i].set_f( crv_vec[i].GetCurve() );
    }




    m_MainSurfVec.resize( m_Nblade() );

//    m_MainSurfVec[0].SkinC0( crv_vec, false );
    m_MainSurfVec[0].SetMagicVParm( false );
    m_MainSurfVec[0].SkinCubicSpline( rib_vec, false );

    m_MainSurfVec[0].SetMagicVParm( true );
    m_MainSurfVec[0].SetSurfType( PROP_SURF );
    m_MainSurfVec[0].SetClustering( m_LECluster(), m_TECluster() );

    if ( m_XSecSurf.GetFlipUD() )
    {
        m_MainSurfVec[0].FlipNormal();
    }

    Matrix4d rot;
    for ( int i = 1; i < m_Nblade(); i++ )
    {
        double theta = 360.0 * i / ( double )m_Nblade();
        rot.loadIdentity();
        rot.rotateX( theta );

        m_MainSurfVec[i] = m_MainSurfVec[0];
        m_MainSurfVec[i].Transform( rot );
    }
}

//==== Compute Rotation Center ====//
void PropGeom::ComputeCenter()
{
}

//==== Encode Data Into XML Data Struct ====//
xmlNodePtr PropGeom::EncodeXml( xmlNodePtr & node )
{
    Geom::EncodeXml( node );
    xmlNodePtr propeller_node = xmlNewChild( node, NULL, BAD_CAST "PropellerGeom", NULL );
    if ( propeller_node )
    {
        m_XSecSurf.EncodeXml( propeller_node );
        m_ChordCurve.EncodeXml( propeller_node );
        m_TwistCurve.EncodeXml( propeller_node );
        m_XRotateCurve.EncodeXml( propeller_node );
        m_ZRotateCurve.EncodeXml( propeller_node );
        m_RakeCurve.EncodeXml( propeller_node );
        m_SkewCurve.EncodeXml( propeller_node );
    }
    return propeller_node;
}

//==== Encode Data Into XML Data Struct ====//
xmlNodePtr PropGeom::DecodeXml( xmlNodePtr & node )
{
    Geom::DecodeXml( node );

    xmlNodePtr propeller_node = XmlUtil::GetNode( node, "PropellerGeom", 0 );
    if ( propeller_node )
    {
        m_XSecSurf.DecodeXml( propeller_node );
        m_ChordCurve.DecodeXml( propeller_node );
        m_TwistCurve.DecodeXml( propeller_node );
        m_XRotateCurve.DecodeXml( propeller_node );
        m_ZRotateCurve.DecodeXml( propeller_node );
        m_RakeCurve.DecodeXml( propeller_node );
        m_SkewCurve.DecodeXml( propeller_node );
    }

    return propeller_node;
}

//==== Set Active XSec Type ====//
void PropGeom::SetActiveXSecType( int type )
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

//==== Override Geom Cut/Copy/Insert/Paste ====//
void PropGeom::CutXSec( int index )
{
    m_XSecSurf.CutXSec( index );
    SetActiveXSecIndex( GetActiveXSecIndex() );
    Update();
}
void PropGeom::CopyXSec( int index )
{
    m_XSecSurf.CopyXSec( index );
}
void PropGeom::PasteXSec( int index )
{
    m_XSecSurf.PasteXSec( index );
    Update();
}
void PropGeom::InsertXSec( int index, int type )
{
    SetActiveXSecIndex( index );
    InsertXSec( type );
}

//==== Cut Active XSec ====//
void PropGeom::CutActiveXSec()
{
    CutXSec( m_ActiveXSec );
}

//==== Copy Active XSec ====//
void PropGeom::CopyActiveXSec()
{
    CopyXSec( m_ActiveXSec );
}

//==== Paste Cut/Copied XSec To Active XSec ====//
void PropGeom::PasteActiveXSec()
{
    PasteXSec( m_ActiveXSec );
}

//==== Insert XSec ====//
void PropGeom::InsertXSec( )
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
void PropGeom::InsertXSec( int type )
{
    if ( m_ActiveXSec >= NumXSec() - 1 )
    {
        return;
    }

    PropXSec* xs = ( PropXSec* ) GetXSec( m_ActiveXSec );
    PropXSec* xs_1 = ( PropXSec* ) GetXSec( m_ActiveXSec + 1 );

    double y_loc_0 = xs->m_RadiusFrac();
    double y_loc_1 = xs_1->m_RadiusFrac();

    m_XSecSurf.InsertXSec( type, m_ActiveXSec );
    m_ActiveXSec++;

    PropXSec* inserted_xs = ( PropXSec* ) GetXSec( m_ActiveXSec );

    if ( inserted_xs )
    {
        inserted_xs->SetRefLength( m_Diameter() / 2.0 );
        inserted_xs->CopyFrom( xs );
        inserted_xs->m_RadiusFrac = ( y_loc_0 + y_loc_1 ) * 0.5;
    }

    Update();
}


//==== Look Though All Parms and Load Linkable Ones ===//
void PropGeom::AddLinkableParms( vector< string > & linkable_parm_vec, const string & link_container_id )
{
    Geom::AddLinkableParms( linkable_parm_vec );

    m_XSecSurf.AddLinkableParms( linkable_parm_vec, m_ID  );

    m_ChordCurve.AddLinkableParms( linkable_parm_vec, m_ID  );
    m_TwistCurve.AddLinkableParms( linkable_parm_vec, m_ID  );
    m_XRotateCurve.AddLinkableParms( linkable_parm_vec, m_ID  );
    m_ZRotateCurve.AddLinkableParms( linkable_parm_vec, m_ID  );
    m_RakeCurve.AddLinkableParms( linkable_parm_vec, m_ID  );
    m_SkewCurve.AddLinkableParms( linkable_parm_vec, m_ID  );
}

//==== Scale ====//
void PropGeom::Scale()
{
    double currentScale = m_Scale() / m_LastScale();
    m_Diameter *= currentScale;
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

void PropGeom::AddDefaultSources( double base_len )
{
    AddDefaultSourcesXSec( base_len, m_Diameter(), 0 );
    AddDefaultSourcesXSec( base_len, m_Diameter(), m_XSecSurf.NumXSec() - 1 );
}

//==== Drag Parameters ====//
void PropGeom::LoadDragFactors( DragFactors& drag_factors )
{
}

void PropGeom::EnforceOrder( PropXSec* xs, int indx )
{
    if ( indx == 0 )
    {
        PropXSec* nextxs = ( PropXSec* ) m_XSecSurf.FindXSec( indx + 1 );
        double upper = nextxs->m_RadiusFrac();
        xs->m_RadiusFrac.SetLowerUpperLimits( 0.0, upper );
    }
    else if ( indx ==  m_XSecSurf.NumXSec() - 1 )
    {
        xs->m_RadiusFrac.SetLowerUpperLimits( 1.0, 1.0 );
        xs->m_RadiusFrac.Set( 1.0 );
    }
    else
    {
        PropXSec* priorxs = ( PropXSec* ) m_XSecSurf.FindXSec( indx - 1 );
        PropXSec* nextxs = ( PropXSec* ) m_XSecSurf.FindXSec( indx + 1 );
        double lower = priorxs->m_RadiusFrac();
        double upper = nextxs->m_RadiusFrac();
        xs->m_RadiusFrac.SetLowerUpperLimits( lower , upper );
    }
}

void PropGeom::EnforcePCurveOrder( double rfirst, double rlast )
{
    double offset = 1e-4;

    for ( int i = 0; i < m_pcurve_vec.size(); i++ )
    {
        if ( m_pcurve_vec[i] )
        {
            m_pcurve_vec[i]->EnforcePtOrder( rfirst, rlast );
        }
    }
}

void PropGeom::UpdateTesselate( int indx, vector< vector< vec3d > > &pnts, vector< vector< vec3d > > &norms, vector< vector< vec3d > > &uw_pnts, bool degen )
{
    vector < int > tessvec;
    vector < double > rootc;
    vector < double > tipc;
    vector < int > umerge;

    int nmerge = m_SurfVec[indx].GetNumSectU();

    if (m_CapUMinOption()!=NO_END_CAP && m_CapUMinSuccess[ m_SurfIndxVec[indx] ] )
    {
        tessvec.push_back( m_CapUMinTess() );
        rootc.push_back( 1.0 );
        tipc.push_back( 1.0 );
        umerge.push_back( 1 );
        nmerge--;
    }

    if (m_CapUMaxOption()!=NO_END_CAP && m_CapUMaxSuccess[ m_SurfIndxVec[indx] ] )
    {
        nmerge--;
    }

    tessvec.push_back( m_TessU() );
    rootc.push_back( m_RootCluster() );
    tipc.push_back( m_TipCluster() );
    umerge.push_back( nmerge );

    if (m_CapUMaxOption()!=NO_END_CAP && m_CapUMaxSuccess[ m_SurfIndxVec[indx] ] )
    {
        tessvec.push_back( m_CapUMinTess() );
        rootc.push_back( 1.0 );
        tipc.push_back( 1.0 );
        umerge.push_back( 1 );
    }

    m_SurfVec[indx].SetRootTipClustering( rootc, tipc );
    m_SurfVec[indx].Tesselate( tessvec, m_TessW(), pnts, norms, uw_pnts, m_CapUMinTess(), degen, umerge );
}

void PropGeom::UpdateSplitTesselate( int indx, vector< vector< vector< vec3d > > > &pnts, vector< vector< vector< vec3d > > > &norms )
{
    vector < int > tessvec;
    vector < double > rootc;
    vector < double > tipc;
    vector < int > umerge;

    int nmerge = m_SurfVec[indx].GetNumSectU();

    if (m_CapUMinOption()!=NO_END_CAP && m_CapUMinSuccess[ m_SurfIndxVec[indx] ] )
    {
        tessvec.push_back( m_CapUMinTess() );
        rootc.push_back( 1.0 );
        tipc.push_back( 1.0 );
        umerge.push_back( 1 );
        nmerge--;
    }

    if (m_CapUMaxOption()!=NO_END_CAP && m_CapUMaxSuccess[ m_SurfIndxVec[indx] ] )
    {
        nmerge--;
    }

    tessvec.push_back( m_TessU() );
    rootc.push_back( m_RootCluster() );
    tipc.push_back( m_TipCluster() );
    umerge.push_back( nmerge );

    if (m_CapUMaxOption()!=NO_END_CAP && m_CapUMaxSuccess[ m_SurfIndxVec[indx] ] )
    {
        tessvec.push_back( m_CapUMinTess() );
        rootc.push_back( 1.0 );
        tipc.push_back( 1.0 );
        umerge.push_back( 1 );
    }

    m_SurfVec[indx].SetRootTipClustering( rootc, tipc );
    m_SurfVec[indx].SplitTesselate( tessvec, m_TessW(), pnts, norms, m_CapUMinTess(), umerge );
}
