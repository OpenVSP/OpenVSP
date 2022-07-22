//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//
//
//////////////////////////////////////////////////////////////////////

#include "PropGeom.h"
#include "ParmMgr.h"
#include "Vehicle.h"
#include <float.h>
#include "Cluster.h"
#include "SubSurfaceMgr.h"

using namespace vsp;

// Allow curve control of thickness.

// v2 import

// New section types

//==========================================================================//
//==========================================================================//
//==========================================================================//

PropPositioner::PropPositioner()
{
    m_ParentProp = NULL;

    m_Chord = 1.0;

    m_Construct = 0.5;
    m_FeatherOffset = 0.0;
    m_FeatherAxis = 0.5;
    m_RootChord = 1.0;
    m_RootTwist = 0.0;

    m_Twist = 0.0;
    m_PropRot = 0.0;
    m_Feather = 0.0;
    m_ZRotate = 0.0;

    m_CurveSection = false;

    m_Radius = 0.0;
    m_Rake = 0.0;
    m_Skew = 0.0;
    m_Sweep = 0.0;

    m_FoldOrigin = vec3d( 0, 0, 0 );
    m_FoldDirection = vec3d( 0, 0, 1 );
    m_FoldAngle = 0.0;

    m_Reverse = 1.0;

    m_NeedsUpdate = true;

}


void PropPositioner::Update()
{
    m_NeedsUpdate = false;

    m_TransformedCurve = m_Curve;

    Matrix4d mat;
    mat.scale( m_Chord );

    if ( m_Reverse < 0 )
    {
        mat.translatef( 1.0, 0.0, 0.0 );
        mat.flipx();
    }

    m_TransformedCurve.Transform( mat );

    if ( !m_ParentProp )
    {
        return;
    }

    // Basic transformation orients curve before other transformations.
    m_ParentProp->GetBasicTransformation( m_Chord, mat );
    m_TransformedCurve.Transform( mat );

    mat.loadIdentity();


    // Position section for prop shaping.

    double xb = m_RootChord * ( 0.5 - m_FeatherAxis );
    mat.translatef( xb * sin( m_RootTwist * PI / 180.0), 0, m_Reverse * xb * cos( m_RootTwist * PI / 180.0) );

    mat.rotateX( m_Reverse * m_Sweep ); // About axis of rotation

    mat.translatef( 0, m_Radius, 0 );

    double x = -m_RootChord * ( 0.5 - m_Construct );
    mat.translatef( x * sin( m_RootTwist * PI / 180.0), 0, m_Reverse * x * cos( m_RootTwist * PI / 180.0) );

    mat.translatef( m_Axial, 0, m_Reverse * m_Tangential );

    mat.rotateY( m_Reverse * m_Twist );

    mat.translatef( m_Rake, 0, m_Reverse * m_Skew );

    mat.rotateZ( m_ZRotate ); // About chord

    mat.translatef( 0, 0, m_Reverse * m_Chord * ( 0.5 - m_Construct ) );

    m_TransformedCurve.Transform( mat );

    // Project prop curve onto cylinder surface in current location.  After blade lofting,
    // but before rigid body motion positioning.
    if ( m_CurveSection )
    {
        m_TransformedCurve.ProjectOntoCylinder( m_Radius, true, 1e-6 * m_Chord );
    }

    mat.loadIdentity();  // Reset to handle rigid body motion of lofted prop sections.

    // Propeller rotation first because order is reversed.
    mat.rotateX( -m_Reverse * m_PropRot );

    mat.rotateZ( m_Precone );

    mat.translatef( m_FoldOrigin.x(), m_FoldOrigin.y(), m_FoldOrigin.z() );
    mat.rotate( m_FoldAngle * PI / 180.0, m_FoldDirection );
    mat.translatef( -m_FoldOrigin.x(), -m_FoldOrigin.y(), -m_FoldOrigin.z() );


    mat.translatef( 0, 0, m_RootChord * m_FeatherOffset );

    mat.rotateY( m_Reverse * m_Feather );

    m_TransformedCurve.Transform( mat );
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
}

//==== Set Ref Length ====//
void PropXSec::SetRefLength( double len )
{
    if ( std::abs( len - m_RefLenVal ) < DBL_EPSILON )
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

    m_ExportMainSurf = false;

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

    m_PropMode.Init( "PropMode", "Design", this, PROP_BLADES, PROP_BLADES, PROP_DISK );
    m_PropMode.SetDescript( "Propeller model mode." );

    m_Rotate.Init( "Rotate", "Design", this, 0.0, -360.0, 360.0 );
    m_Rotate.SetDescript( "Rotation of first propeller blade." );

    m_Construct.Init( "ConstructXoC", "Design", this, 0.5, 0.0, 1.0 );
    m_Construct.SetDescript( "X/C of construction line." );

    m_FeatherAxis.Init( "FeatherAxisXoC", "Design", this, 0.5, -100.0, 100.0 );
    m_FeatherAxis.SetDescript( "Location of feather axis along chord." );

    m_FeatherOffset.Init( "FeatherOffsetXoC", "Design", this, 0.0, -100.0, 100.0 );
    m_FeatherOffset.SetDescript( "Offset of feather axis line." );

    m_RadFoldAxis.Init( "RFoldAx", "Design", this, 0.2, 0.0, 1.0 );
    m_RadFoldAxis.SetDescript( "Radial position of fold axis as fraction of radius" );

    m_AxialFoldAxis.Init( "AxFoldAx", "Design", this, 0, -1.0, 1.0 );
    m_AxialFoldAxis.SetDescript( "Axial position of fold axis as fraction of radius" );

    m_OffsetFoldAxis.Init( "OffFoldAx", "Design", this, 0, -1.0, 1.0 );
    m_OffsetFoldAxis.SetDescript( "Offset position of fold axis as fraction of radius" );

    m_AzimuthFoldDir.Init( "AzFoldDir", "Design", this, 0.0, -90.0, 90.0 );
    m_AzimuthFoldDir.SetDescript( "Azimuth angle of fold axis direction vector" );

    m_ElevationFoldDir.Init( "ElFoldDir", "Design", this, 0.0, -90.0, 90.0 );
    m_ElevationFoldDir.SetDescript( "Elevation angle of fold axis direction vector" );

    m_FoldAngle.Init( "FoldAngle", "Design", this, 0.0, -180.0, 180.0 );
    m_FoldAngle.SetDescript( "Propeller fold angle" );

    m_Beta34.Init( "Beta34", "Design", this, 20.0, -400.0, 400.0 );
    m_Beta34.SetDescript( "Blade pitch at 3/4 of radius" );

    m_Feather.Init( "Feather", "Design", this, 0.0, -400.0, 400.0 );
    m_Feather.SetDescript( "Blade feather angle" );

    m_UseBeta34Flag.Init( "UseBeta34Flag", "Design", this, 1, 0, 1 );
    m_UseBeta34Flag.SetDescript( "Flag to use Beta34 or Beta0 as driver" );

    m_Precone.Init( "Precone", "Design", this, 0.0, -90.0, 90.0 );
    m_Precone.SetDescript( "Blade pre cone angle" );

    m_ReverseFlag.Init( "ReverseFlag", "Design", this, false, 0, 1 );
    m_ReverseFlag.SetDescript( "Flag to reverse propeller rotation direction" );

    m_CylindricalSectionsFlag.Init( "CylindricalSectionsFlag", "Design", this, false, 0, 1 );
    m_CylindricalSectionsFlag.SetDescript( "Flag to project airfoil sections onto cylinder of rotation" );

    m_AFLimit.Init( "AFLimit", "Design", this, 0.2, 0, 1 );
    m_AFLimit.SetDescript( "Lower limit of activity factor integration" );

    m_AF.Init( "AF", "Design", this, 100, 0, 1e12 );
    m_AF.SetDescript( "Propeller activity factor" );

    m_CLi.Init( "CLi", "Design", this, 0.5, 0, 1e12 );
    m_CLi.SetDescript( "Integrated design lift coefficient" );

    m_Solidity.Init( "Solidity", "Design", this, 0.5, 0, 1.0 );
    m_Solidity.SetDescript( "Geometric blade solidity" );

    m_TSolidity.Init( "TSolidity", "Design", this, 0.5, 0, 1.0 );
    m_TSolidity.SetDescript( "Thrust weighted blade solidity" );

    m_PSolidity.Init( "PSolidity", "Design", this, 0.5, 0, 1.0 );
    m_PSolidity.SetDescript( "Power weighted blade solidity" );

    m_Chord.Init( "Chord", "Design", this, 0.5, 0, 1.0 );
    m_Chord.SetDescript( "Geometric blade chord" );

    m_TChord.Init( "TChord", "Design", this, 0.5, 0, 1.0 );
    m_TChord.SetDescript( "Thrust weighted blade chord" );

    m_PChord.Init( "PChord", "Design", this, 0.5, 0, 1.0 );
    m_PChord.SetDescript( "Power weighted blade chord" );

    m_LECluster.Init( "LECluster", "Design", this, 0.25, 1e-4, 10.0 );
    m_LECluster.SetDescript( "LE Tess Cluster Control" );

    m_TECluster.Init( "TECluster", "Design", this, 0.25, 1e-4, 10.0 );
    m_TECluster.SetDescript( "TE Tess Cluster Control" );

    m_RootCluster.Init( "InCluster", "Design", this, 1.0, 1e-4, 10.0 );
    m_RootCluster.SetDescript( "Inboard Tess Cluster Control" );

    m_TipCluster.Init( "OutCluster", "Design", this, 1.0, 1e-4, 10.0 );
    m_TipCluster.SetDescript( "Outboard Tess Cluster Control" );

    m_SmallPanelW.Init( "SmallPanelW", m_Name, this, 0.0, 0.0, 1e12 );
    m_SmallPanelW.SetDescript( "Smallest LE/TE panel width");

    m_MaxGrowth.Init( "MaxGrowth", m_Name, this, 1.0, 1.0, 1e12);
    m_MaxGrowth.SetDescript( "Maximum chordwise panel growth ratio" );

    //==== rename capping controls for wing specific terminology ====//
    m_CapUMinOption.SetDescript( "Type of End Cap on Propeller Root" );
    m_CapUMinOption.Parm::Set( FLAT_END_CAP );
    m_CapUMinTess.SetDescript( "Number of tessellated curves on Propeller Root and Tip" );
    m_CapUMaxOption.SetDescript( "Type of End Cap on Propeller Tip" );
    m_CapUMaxOption.Parm::Set( ROUND_END_CAP );

    m_ActiveXSec = 0;

    m_XSecSurf.SetXSecType( XSEC_PROP );

    m_XSecSurf.AddXSec( XS_ELLIPSE );
    m_XSecSurf.AddXSec( XS_ONE_SIX_SERIES );
    m_XSecSurf.AddXSec( XS_ONE_SIX_SERIES );

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
    xs->m_RadiusFrac = 1.0;

    m_ChordCurve.SetParentContainer( GetID() );
    m_ChordCurve.SetDispNames( "r/R", "Chord/R" );
    m_ChordCurve.SetParmNames( "r", "crd" );
    m_ChordCurve.SetCurveName( "Chord" );
    m_ChordCurve.InitParms();
    m_ChordCurve.m_CurveType = vsp::CEDIT;
    static const double t1[] = {0.2, 0.2 + 0.4 / 3.0,
               0.6 - 0.4 / 3.0, 0.6, 0.6 + 0.35 / 3.0,
               0.95 - 0.35 / 3.0, 0.95, 0.95 + 0.05 / 3.0,
               1.0 - 0.05 / 3.0, 1.0};
    static const double v1[] = {0.08, 0.15,
                           0.2, 0.2, 0.2,
                           0.2, 0.2, 0.2,
                           0.2, 0.13};
    vector < double > tv1( t1, t1 + sizeof( t1 ) / sizeof( t1[0] ) );
    vector < double > vv1( v1, v1 + sizeof( v1 ) / sizeof( v1[0] ) );
    m_ChordCurve.InitCurve( tv1, vv1 );

    // Prevent negative values in m_Chord
    for ( size_t i = 0; i < m_ChordCurve.m_ValParmVec.size(); i++ )
    {
        m_ChordCurve.m_ValParmVec[i]->SetLowerLimit( 1e-8 );
    }

    m_TwistCurve.SetParentContainer( GetID() );
    m_TwistCurve.SetDispNames( "r/R", "Twist" );
    m_TwistCurve.SetParmNames( "r", "tw" );
    m_TwistCurve.SetCurveName( "Twist" );
    m_TwistCurve.InitParms();
    m_TwistCurve.m_CurveType = vsp::PCHIP;
    static const double t2[] = {0.2, 0.75, 1.0};
    static const double v2[] = {46.75, 20.0, 13.0};
    vector < double > tv2( t2, t2 + sizeof( t2 ) / sizeof( t2[0] ) );
    vector < double > vv2( v2, v2 + sizeof( v2 ) / sizeof( v2[0] ) );
    m_TwistCurve.InitCurve( tv2, vv2 );

    static const double t3[] = {0.2, 1.0};
    static const double v3[] = {0.0, 0.0};
    vector < double > tv3( t3, t3 + sizeof( t3 ) / sizeof( t3[0] ) );
    vector < double > vv3( v3, v3 + sizeof( v3 ) / sizeof( v3[0] ) );

    m_RakeCurve.SetParentContainer( GetID() );
    m_RakeCurve.SetDispNames( "r/R", "Rake/R" );
    m_RakeCurve.SetParmNames( "r", "rak" );
    m_RakeCurve.SetCurveName( "Rake" );
    m_RakeCurve.InitParms();
    m_RakeCurve.m_CurveType = vsp::LINEAR;
    m_RakeCurve.InitCurve( tv3, vv3 );

    m_SkewCurve.SetParentContainer( GetID() );
    m_SkewCurve.SetDispNames( "r/R", "Skew/R" );
    m_SkewCurve.SetParmNames( "r", "skw" );
    m_SkewCurve.SetCurveName( "Skew" );
    m_SkewCurve.InitParms();
    m_SkewCurve.m_CurveType = vsp::LINEAR;
    m_SkewCurve.InitCurve( tv3, vv3 );

    m_SweepCurve.SetParentContainer( GetID() );
    m_SweepCurve.SetDispNames( "r/R", "Sweep" );
    m_SweepCurve.SetParmNames( "r", "sw" );
    m_SweepCurve.SetCurveName( "Sweep" );
    m_SweepCurve.InitParms();
    m_SweepCurve.m_CurveType = vsp::LINEAR;
    m_SweepCurve.InitCurve( tv3, vv3 );

    m_ThickCurve.SetParentContainer( GetID() );
    m_ThickCurve.SetDispNames( "r/R", "Thick/C" );
    m_ThickCurve.SetParmNames( "r", "toc" );
    m_ThickCurve.SetCurveName( "Thick" );
    m_ThickCurve.InitParms();
    m_ThickCurve.m_CurveType = vsp::CEDIT;
    static const double t4[] =                    {0.2, 0.2 + 0.1 / 3.0,
                                  0.3 - 0.1 / 3.0, 0.3, 0.3 + 0.3 / 3.0,
                                  0.6 - 0.3 / 3.0, 0.6, 0.6 + 0.4 / 3.0,
                                  1.0 - 0.4 / 3.0, 1.0};
    static const double v4[] =        {0.50, 0.300,
                                0.225, 0.20, 0.125,
                                0.115, 0.10, 0.075,
                                0.055, 0.03};
    vector < double > tv4( t4, t4 + sizeof( t4 ) / sizeof( t4[0] ) );
    vector < double > vv4( v4, v4 + sizeof( v4 ) / sizeof( v4[0] ) );
    m_ThickCurve.InitCurve( tv4, vv4 );

    m_CLICurve.SetParentContainer( GetID() );
    m_CLICurve.SetDispNames( "r/R", "CLi" );
    m_CLICurve.SetParmNames( "r", "cli" );
    m_CLICurve.SetCurveName( "CLI" );
    m_CLICurve.InitParms();
    m_CLICurve.m_CurveType = vsp::CEDIT;
    static const double t5[] =                    {0.2, 0.2 + 0.2 / 3.0,
                                  0.4 - 0.2 / 3.0, 0.4, 0.4 + 0.2 / 3.0,
                                  0.6 - 0.2 / 3.0, 0.6, 0.6 + 0.4 / 3.0,
                                  1.0 - 0.4 / 3.0, 1.0};
    static const double v5[] =       {0.0, 0.7,
                                 0.7, 0.7, 0.7,
                                 0.7, 0.7, 0.7,
                                 0.7, 0.2};
    vector < double > tv5( t5, t5 + sizeof( t5 ) / sizeof( t5[0] ) );
    vector < double > vv5( v5, v5 + sizeof( v5 ) / sizeof( v5[0] ) );
    m_CLICurve.InitCurve( tv5, vv5 );

    m_AxialCurve.SetParentContainer( GetID() );
    m_AxialCurve.SetDispNames( "r/R", "Axial/R" );
    m_AxialCurve.SetParmNames( "r", "ax" );
    m_AxialCurve.SetCurveName( "Axial" );
    m_AxialCurve.InitParms();
    m_AxialCurve.m_CurveType = vsp::LINEAR;
    m_AxialCurve.InitCurve( tv3, vv3 );

    m_TangentialCurve.SetParentContainer(GetID() );
    m_TangentialCurve.SetDispNames("r/R", "Tan/R" );
    m_TangentialCurve.SetParmNames("r", "tan" );
    m_TangentialCurve.SetCurveName("Tangential" );
    m_TangentialCurve.InitParms();
    m_TangentialCurve.m_CurveType = vsp::LINEAR;
    m_TangentialCurve.InitCurve(tv3, vv3 );

    // Set up vector to allow treatment as a group.
    m_pcurve_vec.resize( NUM_PROP_PCURVE );
    m_pcurve_vec[ PROP_CHORD ] = &m_ChordCurve;
    m_pcurve_vec[ PROP_TWIST ] = &m_TwistCurve;
    m_pcurve_vec[ PROP_RAKE ] = &m_RakeCurve;
    m_pcurve_vec[ PROP_SKEW ] = &m_SkewCurve;
    m_pcurve_vec[ PROP_SWEEP ] = &m_SweepCurve;
    m_pcurve_vec[ PROP_THICK ] = &m_ThickCurve;
    m_pcurve_vec[ PROP_CLI ] = &m_CLICurve;
    m_pcurve_vec[ PROP_AXIAL ] = &m_AxialCurve;
    m_pcurve_vec[ PROP_TANGENTIAL ] = &m_TangentialCurve;

}

//==== Destructor ====//
PropGeom::~PropGeom()
{

}

void PropGeom::UpdateDrawObj()
{
    GeomXSec::UpdateDrawObj();

    m_ArrowLinesDO.m_PntVec.clear();
    m_ArrowHeadDO.m_PntVec.clear();

    double axlen = 1.0;

    Vehicle *veh = VehicleMgr.GetVehicle();
    if ( veh )
    {
        axlen = veh->m_AxisLength();
    }

    for ( int i = 0; i < GetNumSymmCopies(); i++)
    {
        double data[16];
        m_ModelMatrix.getMat( data );

        Matrix4d trans_mat = m_TransMatVec[i * GetNumMainSurfs()]; // Translations for the specific symmetric copy

        vec3d cen( 0, 0, 0 );
        vec3d rotdir( -1, 0, 0 );
        vec3d thrustdir( -1, 0, 0 );

        double rev = 1.0;
        if ( !m_FlipNormalVec[i * GetNumMainSurfs()] )
        {
            // Note inverse of m_FipNormalVec is used because Props are flipped by 
            // default (m_XSecSurf.GetFlipUD() in UpdateSurf())
            rev = -1.0;
        }

        rotdir = rotdir * rev;

        cen = trans_mat.xform( cen );
        rotdir = trans_mat.xform( rotdir ) - cen;
        thrustdir = trans_mat.xform( thrustdir ) - cen;

        Matrix4d mat;
        mat.loadIdentity();
        mat.rotateX( -1 * m_Rotate() );
        mat.rotateZ( m_Precone() );
        mat.postMult( trans_mat );

        vec3d pmid = mat.xform( m_FoldAxOrigin );
        vec3d ptstart = mat.xform( m_FoldAxOrigin + rev * m_FoldAxDirection * axlen / 2.0 );
        vec3d ptend = mat.xform( m_FoldAxOrigin - rev * m_FoldAxDirection * axlen / 2.0 );

        vec3d dir = ptend - ptstart;
        dir.normalize();

        if ( m_PropMode() <= PROP_MODE::PROP_BOTH )
        {
            m_ArrowLinesDO.m_PntVec.push_back( ptstart );
            m_ArrowLinesDO.m_PntVec.push_back( ptend );
        }

        m_ArrowLinesDO.m_PntVec.push_back( cen );
        m_ArrowLinesDO.m_PntVec.push_back( cen + thrustdir * axlen );

        MakeArrowhead( cen + thrustdir * axlen, thrustdir, 0.25 * axlen, m_ArrowHeadDO.m_PntVec );
        MakeCircleArrow( cen, rotdir, 0.5 * axlen, m_ArrowLinesDO, m_ArrowHeadDO );

        if ( m_PropMode() <= PROP_MODE::PROP_BOTH )
        {
            MakeCircleArrow( pmid, dir, 0.5 * axlen, m_ArrowLinesDO, m_ArrowHeadDO );
        }
    }
}

void PropGeom::LoadDrawObjs( vector< DrawObj* > & draw_obj_vec )
{
    if ( m_PropMode() == PROP_MODE::PROP_DISK )
    {
        // Bypass XSec drawing for disk
        Geom::LoadDrawObjs( draw_obj_vec );
        // TODO: Don't draw subsurfaces for disk mode?
    }
    else
    {
        GeomXSec::LoadDrawObjs( draw_obj_vec );
    }

    if ( ( m_GuiDraw.GetDispFeatureFlag() && GetSetFlag( vsp::SET_SHOWN ) ) || m_Vehicle->IsGeomActive( m_ID ) )
    {
        m_ArrowHeadDO.m_GeomID = m_ID + "Arrows";
        m_ArrowHeadDO.m_LineWidth = 1.0;
        m_ArrowHeadDO.m_Type = DrawObj::VSP_SHADED_TRIS;
        m_ArrowHeadDO.m_NormVec = vector <vec3d> ( m_ArrowHeadDO.m_PntVec.size() );

        for ( int i = 0; i < 4; i++ )
        {
            m_ArrowHeadDO.m_MaterialInfo.Ambient[i] = 0.2f;
            m_ArrowHeadDO.m_MaterialInfo.Diffuse[i] = 0.1f;
            m_ArrowHeadDO.m_MaterialInfo.Specular[i] = 0.7f;
            m_ArrowHeadDO.m_MaterialInfo.Emission[i] = 0.0f;
        }
        m_ArrowHeadDO.m_MaterialInfo.Diffuse[3] = 0.5f;
        m_ArrowHeadDO.m_MaterialInfo.Shininess = 5.0f;

        m_ArrowLinesDO.m_GeomID = m_ID + "ALines";
        m_ArrowLinesDO.m_Screen = DrawObj::VSP_MAIN_SCREEN;
        m_ArrowLinesDO.m_LineWidth = 2.0;
        m_ArrowLinesDO.m_Type = DrawObj::VSP_LINES;

        draw_obj_vec.push_back( &m_ArrowLinesDO );
        draw_obj_vec.push_back( &m_ArrowHeadDO );
    }
}

void PropGeom::ChangeID( string id )
{
    Geom::ChangeID( id );
    m_XSecSurf.SetParentContainer( GetID() );

    for ( int i = 0; i < m_pcurve_vec.size(); i++ )
    {
        m_pcurve_vec[i]->SetParentContainer( GetID() );
    }
}

bool aboutcomp(const double &a, const double &b)
{
    return ( a + 0.001 ) < b;
}

bool abouteq(const double &a, const double &b)
{
    return std::abs( a - b ) < 0.001;
}

void InterpXSecCurve( VspCurve & cout, XSecCurve *c1, XSecCurve *c2, const double & frac, const double & w, const double & t, const double & cli )
{
    // Shortcut for exact match.
    double tol = 1e-6;
    if ( std::abs( frac ) <= tol )
    {
        c1->SetLateUpdateFlag(true);
        cout = c1->GetCurve();

        double wc = c1->GetWidth();
        if ( wc != 0 )
        {
            cout.Scale( 1.0 / wc );
        }

        return;
    }

    if ( std::abs( frac - 1.0 ) <= tol )
    {
        c2->SetLateUpdateFlag(true);
        cout = c2->GetCurve();

        double wc = c1->GetWidth();
        if ( wc != 0 )
        {
            cout.Scale( 1.0 / wc );
        }

        return;
    }


    if ( c1->GetType() == c2->GetType() )
    {
        if ( c1->GetType() != XS_FILE_AIRFOIL &&
             c1->GetType() != XS_CST_AIRFOIL &&
             c1->GetType() != XS_FILE_FUSE &&
             c1->GetType() != XS_EDIT_CURVE )
        {
            XSecCurve *c3 = XSecSurf::CreateXSecCurve( c1->GetType() );

            c3->CopyVals( c1 );

            c3->Interp( c1, c2, frac );

            c3->SetWidthHeight( w, t * w );
            c3->SetDesignLiftCoeff( cli );
            c3->SetForceWingType( true );

            c3->SetLateUpdateFlag( true );

            cout = c3->GetCurve();

            delete c3;

            if ( w != 0 )
            {
                cout.Scale( 1.0 / w );
            }

            return;
        }
    }

    // Code path for mismatched XSecs and XSecs that don't interpolate well.
    InterpXSec c3;

    c3.CopyVals( c1 );

    c3.Interp( c1, c2, frac );

    c3.SetWidthHeight( w, t * w );
    c3.SetForceWingType( true );

    c3.SetLateUpdateFlag( true );

    cout = c3.GetCurve();

    if ( w != 0 )
    {
        cout.Scale( 1.0 / w );
    }

    return;
}

//==== Update Fuselage And Cross Section Placement ====//
void PropGeom::UpdateSurf()
{
    unsigned int nxsec = m_XSecSurf.NumXSec();

    double radius = m_Diameter() / 2.0;

    double rfirst = 0.0;
    double rlast = 1.0;

    vector < double > uvec( nxsec );
    vector < double > rvec( nxsec );

    double rev = 1.0;
    if ( m_ReverseFlag() )
    {
        rev = -1.0;
    }

    //==== Update XSec Location/Rotation ====//
    for ( int i = 0 ; i < nxsec ; i++ )
    {
        PropXSec* xs = ( PropXSec* ) m_XSecSurf.FindXSec( i );

        if ( xs )
        {
            if ( xs->GetXSecCurve()->GetType() == XS_POINT )
            {
                printf( "Warning: XS_POINT type not valid for propellers\n" );
                // Propellers are constructed in two phases.  The first phase stacks
                // all the XSecs with unit chord.  Intermediate curves are extracted
                // from that surface and are scaled to match the required chord.
                // Since XS_POINT XSecs already have zero chord, they mess up this
                // process.
            }

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

    m_rtou = Vsp1DCurve();
    m_rtou.InterpolateLinear( uvec, rvec, false );

    EnforcePCurveOrder( rfirst, rlast );

    // Set lower limit for activity factor integration limit
    m_AFLimit.SetLowerLimit( rfirst );
    // Integrate activity factor.
    m_AF.Set( ( 100000.0 / 16.0 ) * 0.5 * m_ChordCurve.IntegrateCrv_rcub( m_AFLimit() ) );
    m_CLi.Set( 4.0 * m_CLICurve.IntegrateCrv_rcub( m_AFLimit() ) );

    double r03 = rfirst * rfirst * rfirst;
    double r04 = r03 * rfirst;
    m_Chord.Set( m_ChordCurve.IntegrateCrv( rfirst ) / ( 1.0 - rfirst ) );
    m_TChord.Set( 3.0 * m_ChordCurve.IntegrateCrv_rsq( rfirst ) / ( 1.0 - r03 ) );
    m_PChord.Set( 4.0 * m_ChordCurve.IntegrateCrv_rcub( rfirst ) / ( 1.0 - r04 ) );

    m_Solidity.Set( m_Chord() * m_Nblade() / PI );
    m_TSolidity.Set( m_TChord() * m_Nblade() / PI );
    m_PSolidity.Set( m_PChord() * m_Nblade() / PI );

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

    // Set up fold axis & store for visualization.
    Matrix4d fold, foldrot;
    fold.loadIdentity();
    fold.rotateY( m_AzimuthFoldDir() );
    fold.rotateX( m_ElevationFoldDir() );
    m_FoldAxDirection = fold.xform( vec3d( 0, 0, 1 ) );
    m_FoldAxOrigin = vec3d( m_AxialFoldAxis() * radius, m_RadFoldAxis() * radius, m_OffsetFoldAxis() * radius );


    //==== Cross Section Curves & joint info ====//
    vector< XSecCurve* > xsc_vec;
    xsc_vec.resize( nxsec, NULL );

    vector< VspCurve > base_crv_vec;
    base_crv_vec.resize( nxsec );

    //==== Update XSec Location/Rotation ====//
    for ( int i = 0 ; i < nxsec ; i++ )
    {
        PropXSec* xs = ( PropXSec* ) m_XSecSurf.FindXSec( i );

        if ( xs )
        {
            XSecCurve* xsc = xs->GetXSecCurve();

            double r = xs->m_RadiusFrac();
            double w = m_ChordCurve.Comp( r ) * radius;
            double t = m_ThickCurve.Comp( r );
            double cli = m_CLICurve.Comp( r );

            xsc_vec[i] = xsc;

            if ( xsc )
            {
                string height_id = xsc->GetHeightParmID();
                Parm* height_parm = ParmMgr.FindParm( height_id );

                if ( height_parm )
                {
                    height_parm->Deactivate();
                }

                //==== Find Width Parm ====//
                string width_id = xsc->GetWidthParmID();
                Parm* width_parm = ParmMgr.FindParm( width_id );

                xsc->SetDesignLiftCoeff( cli );

                if ( width_parm )
                {
                    width_parm->Deactivate();

                    Airfoil* af = dynamic_cast < Airfoil* > ( xsc );
                    if ( af )
                    {
                        width_parm->Set( w );
                        af->m_ThickChord = t;

                        vector < string > cambids;
                        af->GetLiftCamberParmID( cambids );

                        for ( int j = 0; j < cambids.size(); j++ )
                        {
                            Parm * p = ParmMgr.FindParm( cambids[j] );
                            if ( p )
                            {
                                p->Deactivate();
                            }
                        }
                    }
                    else
                    {
                        CircleXSec* cir = dynamic_cast < CircleXSec* > ( xsc );
                        if ( cir )
                        {
                            width_parm->Set( w );
                        }
                        else
                        {
                            xsc->SetWidthHeight( w, t * w );
                        }
                    }

                }

                base_crv_vec[i] = xsc->GetCurve();
                if ( w != 0 )
                {
                    base_crv_vec[i].Scale( 1.0 / w );
                }
            }
        }
    }

    m_MainSurfVec.clear();

    if ( m_PropMode() <= PROP_MODE::PROP_BOTH )
    {
        // Find the union of stations required to approximate the blade parameters
        // with cubic functions.
        vector < double > tmap = rvec;  // Initialize with specified XSecs.
        vector < double > tdisc;
        tdisc.push_back( rfirst );
        tdisc.push_back( rlast );
        for ( int i = 0; i < m_pcurve_vec.size(); i++ )
        {
            vector < double > tm;
            vector < double > tmout;
            vector < double > td;
            m_pcurve_vec[i]->GetTMap( tm, td );

            std::set_union( tmap.begin(), tmap.end(), tm.begin(), tm.end(), std::back_inserter(tmout), &aboutcomp );
            std::swap( tmout, tmap );
        }

        // Not sure why above set_union leaves duplicate entries, but
        // sort and force unique just to be sure.
        std::sort( tmap.begin(), tmap.end() );
        auto tmit = std::unique( tmap.begin(), tmap.end(), &abouteq );
        tmap.erase( tmit, tmap.end() );

        // Treat all control points as possible discontinuities.
        tdisc = tmap;

        // Refine by adding two intermediate points to each cubic section
        // this is needed because the adaptive algorithm above uses derivatives
        // while our later reconstruction does not.
        vector < double > tref( ( tmap.size() - 1 ) * 3 + 1 );
        for ( int i = 0; i < tmap.size() - 1; i++ )
        {
            int iref = 3*i;
            double t = tmap[i];
            double tnxt = tmap[i+1];
            double dt = (tnxt-t)/3.0;

            tref[iref] = t;
            tref[iref+1] = t + dt;
            tref[iref+2] = t + 2 * dt;
        }
        tref.back() = tmap.back();
        std::swap( tmap, tref );

        // Convert tdisc to final parameterization.
        for ( int i = 0; i < tdisc.size(); i++ )
        {
            tdisc[i] = ( tdisc[i] - rfirst ) / ( rlast - rfirst );
        }


        // Find blade root chord and twist, for later calculations.
        double croot = 0.0;
        double twroot = 0.0;

        PropXSec* xs = ( PropXSec* ) m_XSecSurf.FindXSec( 0 );

        if ( xs )
        {
            double r = xs->m_RadiusFrac();
            double w = m_ChordCurve.Comp( r ) * radius;

            croot = w;
            twroot = m_TwistCurve.Comp( r );
        }

        //==== Update XSec Prop Positioner for highlight curves ====//
        for ( int i = 0 ; i < nxsec ; i++ )
        {
            xs = ( PropXSec* ) m_XSecSurf.FindXSec( i );

            if ( xs )
            {
                double r = xs->m_RadiusFrac();
                double w = m_ChordCurve.Comp( r ) * radius;

                xs->m_PropPos.m_CurveSection = m_CylindricalSectionsFlag();

                // Set up prop positioner for highlight curves - not lofting.
                xs->m_PropPos.m_ParentProp = GetXSecSurf( 0 );
                xs->m_PropPos.m_Radius = r * radius;
                xs->m_PropPos.m_Chord = w;

                xs->m_PropPos.m_Construct = m_Construct();
                xs->m_PropPos.m_FeatherOffset = m_FeatherOffset();
                xs->m_PropPos.m_FeatherAxis = m_FeatherAxis();
                xs->m_PropPos.m_RootChord = croot;
                xs->m_PropPos.m_RootTwist = twroot;

                xs->m_PropPos.m_Twist = m_TwistCurve.Comp( r );
                xs->m_PropPos.m_ZRotate = atan( -m_RakeCurve.Compdt( r ) - m_AxialCurve.Compdt( r ) ) * 180.0 / PI;

                xs->m_PropPos.m_Rake = m_RakeCurve.Comp( r ) * radius;
                xs->m_PropPos.m_Skew = m_SkewCurve.Comp( r ) * radius;
                xs->m_PropPos.m_Sweep = m_SweepCurve.Comp( r );
                xs->m_PropPos.m_Axial = m_AxialCurve.Comp( r ) * radius;
                xs->m_PropPos.m_Tangential = m_TangentialCurve.Comp( r ) * radius;
                xs->m_PropPos.m_PropRot = m_Rotate();
                xs->m_PropPos.m_Feather = m_Feather();

                xs->m_PropPos.m_Precone = m_Precone();

                xs->m_PropPos.m_FoldOrigin = m_FoldAxOrigin;
                xs->m_PropPos.m_FoldDirection = m_FoldAxDirection;
                xs->m_PropPos.m_FoldAngle = m_FoldAngle();

                xs->m_PropPos.m_Reverse = rev;
            }
        }

        // Pseudo cross sections
        // Not directly user-controlled, but an intermediate step in lofting the
        // surface.
        unsigned int npseudo = tmap.size();


        vector< VspCurve > crv_vec( npseudo );
        vector < rib_data_type > rib_vec( npseudo );
        m_UPseudo.clear();
        m_UPseudo.resize( npseudo );
        for ( int i = 0; i < npseudo; i++ )
        {
            // Assume linear interpolation means linear u/r relationship.
            double r = tmap[i];

            // u is a floating point coordinate where integers correspond to XSec indices
            double u = m_rtou.CompPnt( r );

            int istart = std::floor( u );
            int iend = istart + 1;

            double frac = 0;
            if ( iend >= nxsec ) // Make sure index doesn't go off the end.
            {
                iend = istart;
            }
            else
            {
                // Try to be safe to overlapping sections.
                double denom = ( rvec[iend] - rvec[istart] );

                if ( denom <= 1e-6 && iend < nxsec - 1 )
                {
                    istart = iend;
                    iend = istart + 1;
                    denom = ( rvec[iend] - rvec[istart] );
                }
                frac = ( r - rvec[istart] ) / denom;
            }

            double t = m_ThickCurve.Comp( r );
            double cli = m_CLICurve.Comp( r );
            double w = m_ChordCurve.Comp( r ) * radius;

            // Interpolate foil as needed here.
            InterpXSecCurve( crv_vec[i], xsc_vec[ istart ], xsc_vec[ iend ], frac, w, t, cli );


            PropPositioner pp;

            pp.m_ParentProp = this->GetXSecSurf( 0 );
            pp.m_Radius = r * radius;

            pp.m_CurveSection = m_CylindricalSectionsFlag();

            pp.m_Chord = w;
            pp.m_Twist = m_TwistCurve.Comp( r );

            pp.m_Construct = m_Construct();
            pp.m_FeatherOffset = m_FeatherOffset();
            pp.m_FeatherAxis = m_FeatherAxis();
            pp.m_RootChord = croot;
            pp.m_RootTwist = twroot;

            pp.m_ZRotate = atan( -m_RakeCurve.Compdt( r ) - m_AxialCurve.Compdt( r ) ) * 180.0 / PI;

            pp.m_Rake = m_RakeCurve.Comp( r ) * radius;
            pp.m_Skew = m_SkewCurve.Comp( r ) * radius;

            pp.m_Sweep = m_SweepCurve.Comp( r );

            pp.m_Axial = m_AxialCurve.Comp( r ) * radius;
            pp.m_Tangential = m_TangentialCurve.Comp( r ) * radius;

            pp.m_PropRot = m_Rotate();
            pp.m_Feather = m_Feather();

            pp.m_Precone = m_Precone();

            pp.m_FoldOrigin = m_FoldAxOrigin;
            pp.m_FoldDirection = m_FoldAxDirection;
            pp.m_FoldAngle = m_FoldAngle();

            pp.m_Reverse = rev;

            // Set a bunch of other pp variables.
            pp.SetCurve( crv_vec[i] );
            pp.Update();

            rib_vec[i].set_f( pp.GetCurve().GetCurve() );
            // m_UPseudo is 0-1 out the span of the blade.
            m_UPseudo[i] = (r - rfirst ) / (rlast - rfirst );
        }

        // This surface linearly interpolates the airfoil sections without
        // any other transformations.
        // These sections can be extracted (as u-const curves) and then
        // transformed to their final position before skinning.
        m_FoilSurf = VspSurf();
        m_FoilSurf.SkinC0(crv_vec, m_UPseudo, false );

        m_MainSurfVec.reserve( m_Nblade() + 1 );
        m_MainSurfVec.resize( 1 );

        m_MainSurfVec[0].SetMagicVParm( false );
        m_MainSurfVec[0].SkinCubicSpline(rib_vec, m_UPseudo, tdisc, false );

        m_MainSurfVec[0].SetMagicVParm( true );
        m_MainSurfVec[0].SetSurfType( PROP_SURF );
        m_MainSurfVec[0].SetSurfCfdType( vsp::CFD_NORMAL );  // Make sure set to default, can be updated later.

        m_FoilSurf.SetMagicVParm( true );

        m_MainSurfVec[0].SetFoilSurf( &m_FoilSurf );

        if ( m_XSecSurf.GetFlipUD() )
        {
            m_MainSurfVec[0].FlipNormal();
        }

        if ( this->m_ReverseFlag() )
        {
            m_MainSurfVec[0].FlipNormal();
        }

        // UpdateEndCaps here so we only have to cap one blade.
        UpdateEndCaps();

        m_MainSurfVec.resize( m_Nblade(), m_MainSurfVec[0] );

        // Duplicate capping variables
        m_CapUMinSuccess.resize( m_Nblade(), m_CapUMinSuccess[0] );
        m_CapUMaxSuccess.resize( m_Nblade(), m_CapUMaxSuccess[0] );

        Matrix4d rot;
        for ( int i = 1; i < m_Nblade(); i++ )
        {
            double theta = 360.0 * i / ( double )m_Nblade();
            rot.loadIdentity();
            rot.rotateX( theta );

            m_MainSurfVec[i].Transform( rot );
        }
    }

    // Build disk surface.
    if ( m_PropMode() >= PROP_MODE::PROP_BOTH )
    {
        unsigned int nsurf = m_MainSurfVec.size() + 1;
        unsigned int idisk = nsurf - 1;

        m_MainSurfVec.resize( nsurf );
        m_CapUMinSuccess.resize( nsurf );
        m_CapUMaxSuccess.resize( nsurf );

        m_MainSurfVec[ idisk ].CreateDisk( m_Diameter(), Y_DIR, Z_DIR );
        m_MainSurfVec[ idisk ].SetSurfType( vsp::DISK_SURF );
        m_MainSurfVec[ idisk ].SetSurfCfdType( vsp::CFD_TRANSPARENT );
        m_MainSurfVec[ idisk ].SetMagicVParm( false );

        m_CapUMinSuccess[ idisk ] = false;
        m_CapUMaxSuccess[ idisk ] = false;

        if ( m_ReverseFlag() )
        {
            m_MainSurfVec[idisk].FlipNormal();
        }

        if ( m_PropMode() == PROP_MODE::PROP_DISK )
        {
            // Provide fake mesh metric values.
            m_SmallPanelW = 0.0;
            m_MaxGrowth = 1.0;

            m_CappingDone = true;
        }
    }
    
    //When props blades are removed we need to remove its sub-surface from list of sub-surfaces
    //If sub-surface index is greater then amount of GetNumMainSurfs(), we remove sub-surface 
    int count = 0;
    vector<SubSurface*> subsurf_vec_temp = GetSubSurfVec();
    for ( int i = 0; i < subsurf_vec_temp.size(); i++ )
    {
        if ( subsurf_vec_temp[i]->m_MainSurfIndx.Get() >= GetNumMainSurfs() )
        {
            DelSubSurf( i - count );
            count++; // Keep track of deleted sub-surface index offset
            SubSurfaceMgr.SetCurrSubSurfInd( -1 ); // unselect current sub-surface
        }
    }
}

void PropGeom::UpdateMainTessVec()
{
    Geom::UpdateMainTessVec( true );

    int nmain = GetNumMainSurfs();

    m_MainTessVec.resize( nmain, m_MainTessVec[0] );
    m_MainFeatureTessVec.resize( nmain, m_MainFeatureTessVec[0] );

    Matrix4d rot;
    for ( int i = 1; i < m_Nblade(); i++ )
    {
        double theta = 360.0 * i / ( double )m_Nblade();
        rot.loadIdentity();
        rot.rotateX( theta );

        m_MainTessVec[i].Transform( rot );
        m_MainFeatureTessVec[i].Transform( rot );
    }
}

void PropGeom::CalculateMeshMetrics()
{
    std::vector<double> vcheck( 8 );

    double vmin, vmax, vle, vlelow, vleup, vtruemax;

    vmin = 0.0;
    vmax = GetMainWMax(0);
    vtruemax = vmax;

    vle = ( vmin + vmax ) * 0.5;

    vmin += TMAGIC;
    vmax -= TMAGIC;

    vlelow = vle - TMAGIC;
    vleup = vle + TMAGIC;

    double dj = 2.0 / ( m_TessW() - 1 );

    // Calculate lower surface tessellation check points.
    vcheck[0] = ( vmin );
    vcheck[1] = ( vmin + ( vlelow - vmin ) * Cluster( dj, m_TECluster(), m_LECluster() ) );
    vcheck[2] = ( vmin + ( vlelow - vmin ) * Cluster( 1.0 - dj, m_TECluster(), m_LECluster() ) );
    vcheck[3] = ( vlelow );

    // Upper surface constructed as:  vupper = m_Surface.get_vmax() - vlower;
    vcheck[4] = vtruemax - vcheck[0];
    vcheck[5] = vtruemax - vcheck[1];
    vcheck[6] = vtruemax - vcheck[2];
    vcheck[7] = vtruemax - vcheck[3];

    // Loop over points checking for minimum panel width.
    double mind = std::numeric_limits < double >::max();
    for ( int i = 0; i < vcheck.size() - 1; i += 2 )
    {
        double v1 = vcheck[ i ];
        double v2 = vcheck[ i + 1 ];

        for ( int j = 0; j < m_UPseudo.size(); j++ )
        {
            double u = m_UPseudo[ j ];

            double d = dist( m_MainSurfVec[0].CompPnt( u, v1 ), m_MainSurfVec[0].CompPnt( u, v2 ) );
            mind = min( mind, d );
        }
    }
    m_SmallPanelW = mind;

    // Check theoretical growth ratio assuming arclength parameterization is correct.  No need to check actual
    // actual realized surface.  Also, no need to check all airfoil sections.
    double maxrat = 1.0;

    int jle = ( m_TessW() - 1 ) / 2;
    int j = 0;

    double t0 = Cluster( static_cast<double>( j ) / jle, m_TECluster(), m_LECluster() );
    j++;
    double t1 = Cluster( static_cast<double>( j ) / jle, m_TECluster(), m_LECluster() );
    double dt1 = t1 - t0;
    j++;
    double t2;
    for ( ; j <= jle; ++j )
    {
        t2 = Cluster( static_cast<double>( j ) / jle, m_TECluster(), m_LECluster() );

        double dt2 = t2 - t1;

        maxrat = max( maxrat, dt1 / dt2 );
        maxrat = max( maxrat, dt2 / dt1 );

        t0 = t1;
        t1 = t2;
        dt1 = dt2;

    }
    m_MaxGrowth = maxrat;
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
        m_RakeCurve.EncodeXml( propeller_node );
        m_SkewCurve.EncodeXml( propeller_node );
        m_SweepCurve.EncodeXml( propeller_node );
        m_ThickCurve.EncodeXml( propeller_node );
        m_CLICurve.EncodeXml( propeller_node );
        m_AxialCurve.EncodeXml( propeller_node );
        m_TangentialCurve.EncodeXml(propeller_node );
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
        m_RakeCurve.DecodeXml( propeller_node );
        m_SkewCurve.DecodeXml( propeller_node );
        m_SweepCurve.DecodeXml( propeller_node );

        xmlNodePtr thick_curve_node = XmlUtil::GetNode( propeller_node, m_ThickCurve.GetCurveName().c_str(), 0 );
        if ( thick_curve_node )
        {
            m_ThickCurve.DecodeXml( propeller_node );
        }
        else
        {
            unsigned int nxsec = m_XSecSurf.NumXSec();

            vector < double > rvec( nxsec, 0.0 );
            vector < double > tvec( nxsec, 0.0 );

            for ( int i = 0; i < nxsec; i++ )
            {
                PropXSec *xs = ( PropXSec * ) m_XSecSurf.FindXSec( i );

                if ( xs )
                {
                    XSecCurve* xsc = xs->GetXSecCurve();

                    double t = 1.0;
                    if ( xsc )
                    {
                        double w = xsc->GetWidth();
                        double h = xsc->GetHeight();

                        t = h;
                        if ( w > 0 )
                        {
                            t = h / w;
                        }
                    }

                    rvec[i] = xs->m_RadiusFrac();
                    tvec[i] = t;
                }
            }
            m_ThickCurve.m_CurveType = vsp::LINEAR;
            m_ThickCurve.InitCurve( rvec, tvec );
        }

        xmlNodePtr cli_curve_node = XmlUtil::GetNode( propeller_node, m_CLICurve.GetCurveName().c_str(), 0 );
        if ( cli_curve_node )
        {
            m_CLICurve.DecodeXml( propeller_node );
        }
        else
        {
            unsigned int nxsec = m_XSecSurf.NumXSec();

            vector < double > rvec( nxsec, 0.0 );
            vector < double > clivec( nxsec, 0.0 );

            for ( int i = 0; i < nxsec; i++ )
            {
                PropXSec *xs = ( PropXSec * ) m_XSecSurf.FindXSec( i );

                if ( xs )
                {
                    XSecCurve* xsc = xs->GetXSecCurve();

                    rvec[i] = xs->m_RadiusFrac();

                    if ( xsc )
                    {
                        clivec[i] = xsc->GetDesignLiftCoeff();
                    }
                }
            }
            m_CLICurve.m_CurveType = vsp::LINEAR;
            m_CLICurve.InitCurve( rvec, clivec );
        }

        m_AxialCurve.DecodeXml( propeller_node );
        m_TangentialCurve.DecodeXml(propeller_node );
    }

    return propeller_node;
}

//==== Override Geom Cut/Copy/Insert/Paste ====//
void PropGeom::CutXSec( int index )
{
    m_XSecSurf.CutXSec( index );
    // Set up flag so Update() knows to regenerate surface.
    // Insert / split cases don't need this because Parms are added,
    // which implicitly triggers this flag.
    // However, cut deletes Parms - requiring an explicit flag.
    m_SurfDirty = true;
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
    m_ActiveXSec = index;
    InsertXSec( type );
}

//==== Cut Active XSec ====//
void PropGeom::CutActiveXSec()
{
    CutXSec( m_ActiveXSec() );
}

//==== Copy Active XSec ====//
void PropGeom::CopyActiveXSec()
{
    CopyXSec( m_ActiveXSec() );
}

//==== Paste Cut/Copied XSec To Active XSec ====//
void PropGeom::PasteActiveXSec()
{
    PasteXSec( m_ActiveXSec() );
}

//==== Insert XSec ====//
void PropGeom::InsertXSec( )
{
    if ( m_ActiveXSec() >= NumXSec() - 1 || m_ActiveXSec() < 0 )
    {
        return;
    }

    XSec* xs = GetXSec( m_ActiveXSec() );
    if ( xs )
    {
        InsertXSec( xs->GetXSecCurve()->GetType() );
    }
}

//==== Insert XSec ====//
void PropGeom::InsertXSec( int type )
{
    if ( m_ActiveXSec() >= NumXSec() - 1 || m_ActiveXSec() < 0 )
    {
        return;
    }

    PropXSec* xs = ( PropXSec* ) GetXSec( m_ActiveXSec() );
    PropXSec* xs_1 = ( PropXSec* ) GetXSec( m_ActiveXSec() + 1 );

    double y_loc_0 = xs->m_RadiusFrac();
    double y_loc_1 = xs_1->m_RadiusFrac();

    m_XSecSurf.InsertXSec( type, m_ActiveXSec() );
    m_ActiveXSec = m_ActiveXSec() + 1;

    PropXSec* inserted_xs = ( PropXSec* ) GetXSec( m_ActiveXSec() );

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
    m_RakeCurve.AddLinkableParms( linkable_parm_vec, m_ID  );
    m_SkewCurve.AddLinkableParms( linkable_parm_vec, m_ID  );
    m_SweepCurve.AddLinkableParms( linkable_parm_vec, m_ID  );
    m_ThickCurve.AddLinkableParms( linkable_parm_vec, m_ID  );
    m_CLICurve.AddLinkableParms( linkable_parm_vec, m_ID  );
    m_AxialCurve.AddLinkableParms( linkable_parm_vec, m_ID  );
    m_TangentialCurve.AddLinkableParms(linkable_parm_vec, m_ID  );
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
        double tol = 1e-6;
        PropXSec* priorxs = ( PropXSec* ) m_XSecSurf.FindXSec( indx - 1 );
        PropXSec* nextxs = ( PropXSec* ) m_XSecSurf.FindXSec( indx + 1 );
        // Apply tolerance to avoid degenerate surfaces
        double lower = priorxs->m_RadiusFrac() + tol;
        double upper = nextxs->m_RadiusFrac() - tol;
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

void PropGeom::UpdateTesselate( const vector<VspSurf> &surf_vec, int indx, vector< vector< vec3d > > &pnts, vector< vector< vec3d > > &norms, vector< vector< vec3d > > &uw_pnts, bool degen ) const
{
    vector < int > tessvec;
    vector < double > rootc;
    vector < double > tipc;
    vector < int > umerge;

    int nmerge = surf_vec[indx].GetNumSectU();

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

    surf_vec[indx].SetRootTipClustering( rootc, tipc );
    surf_vec[indx].Tesselate( tessvec, m_TessW(), pnts, norms, uw_pnts, m_CapUMinTess(), degen, umerge );
}

void PropGeom::UpdateSplitTesselate( const vector<VspSurf> &surf_vec, int indx, vector< vector< vector< vec3d > > > &pnts, vector< vector< vector< vec3d > > > &norms ) const
{
    vector < int > tessvec;
    vector < double > rootc;
    vector < double > tipc;
    vector < int > umerge;

    int nmerge = surf_vec[indx].GetNumSectU();

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

    surf_vec[indx].SetRootTipClustering( rootc, tipc );
    surf_vec[indx].SplitTesselate( tessvec, m_TessW(), pnts, norms, m_CapUMinTess(), umerge );
}

void PropGeom::UpdatePreTess()
{
    // Update clustering before symmetry is applied for m_SurfVec
    m_FoilSurf.SetClustering( m_LECluster(), m_TECluster() );

    int nsurf = GetNumMainSurfs();
    for ( int i = 0; i < nsurf; i++ )
    {
        m_MainSurfVec[i].SetClustering( m_LECluster(), m_TECluster() );
    }

    CalculateMeshMetrics();
}

string PropGeom::BuildBEMResults()
{
    // Calculate prop center and normal vector
    vec3d cen = m_ModelMatrix.xform( vec3d( 0, 0, 0 ) );
    vec3d norm = m_ModelMatrix.xform( vec3d( -1.0, 0, 0 ) ) - cen;

    int n = m_TessU();

    //==== Create Results ====//
    Results* res = ResultsMgr.CreateResults( "PropBEM" );
    res->Add( NameValData( "Num_Sections", n ) );
    res->Add( NameValData( "Num_Blade", m_Nblade() ) );
    res->Add( NameValData( "Diameter", m_Diameter() ) );
    res->Add( NameValData( "Beta34", m_Beta34() ) );
    res->Add( NameValData( "Feather", m_Feather() ) );
    res->Add( NameValData( "Pre_Cone", m_Precone() ) );
    res->Add( NameValData( "Center", cen ) );
    res->Add( NameValData( "Normal", norm ) );

    double rfirst = m_ChordCurve.GetRFirst();
    double rlast = m_ChordCurve.GetRLast();

    // Establish points to evaluate sections at.
    vector < double > vtess;
    m_MainSurfVec[0].MakeVTess( m_TessW(), vtess, m_CapUMinTess(), false );

    vector < double > r_vec(n);
    vector < double > chord_vec(n);
    vector < double > twist_vec(n);
    vector < double > rake_vec(n);
    vector < double > skew_vec(n);
    vector < double > sweep_vec(n);
    vector < double > thick_vec(n);
    vector < double > cli_vec(n);
    vector < double > axial_vec(n);
    vector < double > tangential_vec(n);

    double rspan = rlast - rfirst;
    for ( int i = 0; i < n; i++ )
    {
        double t = static_cast < double > ( i ) / ( n - 1 );
        double r = rfirst + rspan * Cluster( t, m_RootCluster(), m_TipCluster() );

        VspCurve c;
        m_FoilSurf.GetUConstCurve( c, t );
        vec3d v = c.CompPnt( 0 );
        c.OffsetZ( -v.z() );

        vector < vec3d > pts;
        c.Tesselate( vtess, pts );

        vector < double > xpts( pts.size() );
        vector < double > ypts( pts.size() );

        for ( int j = 0; j < pts.size(); j++ )
        {
            xpts[j] = pts[j].x();
            ypts[j] = pts[j].y();
        }

        char str[255];
        sprintf( str, "%03d", i );
        res->Add( NameValData( "XSection_" + string( str ), xpts ) );
        res->Add( NameValData( "YSection_" + string( str ), ypts ) );

        r_vec[i] = r;
        chord_vec[i] = m_ChordCurve.Comp( r );
        twist_vec[i] = m_TwistCurve.Comp( r );
        rake_vec[i] = m_RakeCurve.Comp( r );
        skew_vec[i] = m_SkewCurve.Comp( r );
        sweep_vec[i] = m_SweepCurve.Comp( r );
        thick_vec[i] = m_ThickCurve.Comp( r );
        cli_vec[i] = m_CLICurve.Comp( r );
        axial_vec[i] = m_AxialCurve.Comp( r );
        tangential_vec[i] = m_TangentialCurve.Comp( r );
    }

    res->Add( NameValData( "Radius", r_vec ) );
    res->Add( NameValData( "Chord", chord_vec ) );
    res->Add( NameValData( "Twist", twist_vec ) );
    res->Add( NameValData( "Rake", rake_vec ) );
    res->Add( NameValData( "Skew", skew_vec ) );
    res->Add( NameValData( "Sweep", sweep_vec ) );
    res->Add( NameValData( "Thick", thick_vec ) );
    res->Add( NameValData( "CLi", cli_vec ) );
    res->Add( NameValData( "Axial", axial_vec ) );
    res->Add( NameValData( "Tangential", tangential_vec ) );

    return res->GetID();
}

int PropGeom::ReadBEM( const string &file_name )
{
    unsigned int num_sect;
    unsigned int num_blade;
    double diam;
    double beta34;
    double feather;
    double precone;
    vec3d cen;
    vec3d norm;

    vector < double > r_vec;
    vector < double > chord_vec;
    vector < double > twist_vec;
    vector < double > rake_vec;
    vector < double > skew_vec;
    vector < double > sweep_vec;
    vector < double > thick_vec;
    vector < double > cli_vec;
    vector < double > axial_vec;
    vector < double > tangential_vec;

    FILE* fid = fopen( file_name.c_str(), "r" );

    if ( !fid )
    {
        return 0;
    }
    else
    {
        char buf[255];
        fgets( buf, 255, fid );  // Advance past "...BEM Propeller..."

        fscanf( fid, "Num_Sections: %u\n", &num_sect );
        fscanf( fid, "Num_Blade: %u\n", &num_blade );
        fscanf( fid, "Diameter: %lf\n", &diam );
        fscanf( fid, "Beta 3/4 (deg): %lf\n", &beta34 );
        fscanf( fid, "Feather (deg): %lf\n", &feather );
        fscanf( fid, "Pre_Cone (deg): %lf\n", &precone );
        double x, y, z;
        fscanf( fid, "Center: %lf, %lf, %lf\n", &x, &y, &z );
        cen.set_xyz( x, y, z );
        fscanf( fid, "Normal: %lf, %lf, %lf\n", &x, &y, &z );
        norm.set_xyz( x, y, z );

        r_vec.resize( num_sect );
        chord_vec.resize( num_sect );
        twist_vec.resize( num_sect );
        rake_vec.resize( num_sect );
        skew_vec.resize( num_sect );
        sweep_vec.resize( num_sect );
        thick_vec.resize( num_sect );
        cli_vec.resize( num_sect );
        axial_vec.resize( num_sect );
        tangential_vec.resize( num_sect );

        fgets( buf, 255, fid );  // Advance past "Radius/R, Chord/R, Twist (deg), Rake/R, Skew/R, Sweep, t/c, CLi, Axial, Tangential"

        for ( int i = 0; i < num_sect; i++ )
        {
            fscanf( fid, "%lf, %lf, %lf, %lf, %lf, %lf, %lf, %lf, %lf, %lf\n", &r_vec[i], &chord_vec[i], &twist_vec[i], &rake_vec[i], &skew_vec[i], &sweep_vec[i], &thick_vec[i], &cli_vec[i], &axial_vec[i], &tangential_vec[i] );
        }

        fclose( fid );
    }

    // Echo BEM file as-read back to stdout.
    if ( false )
    {
        printf( "Num_Sections: %u\n", num_sect );
        printf( "Num_Blade: %u\n", num_blade );
        printf( "Diameter: %.8f\n", diam );
        printf( "Beta 3/4 (deg): %.8f\n", beta34 );
        printf( "Feather (deg): %.8f\n", feather );
        printf( "Pre_Cone (deg): %.8f\n", precone );
        printf( "Center: %.8f, %.8f, %.8f\n", cen.x(), cen.y(), cen.z() );
        printf( "Normal: %.8f, %.8f, %.8f\n", norm.x(), norm.y(), norm.z() );

        printf( "\nRadius/R, Chord/R, Twist (deg), Rake/R, Skew/R, Sweep, t/c, CLi, Axial, Tangential\n" );
        for ( int i = 0; i < num_sect; i++ )
        {
            printf( "%.8f, %.8f, %.8f, %.8f, %.8f, %.8f, %.8f, %.8f, %.8f, %.8f\n", r_vec[i], chord_vec[i], twist_vec[i], rake_vec[i], skew_vec[i], sweep_vec[i], thick_vec[i], cli_vec[i], axial_vec[i], tangential_vec[i] );
        }
    }

    double rfirst = r_vec[ 0 ];
    double rlast = r_vec[ r_vec.size() - 1 ];

    int nxsec = m_XSecSurf.NumXSec();

    PropXSec* xs;
    m_XSecSurf.ChangeXSecShape( 0, XS_ONE_SIX_SERIES );
    xs = ( PropXSec* ) m_XSecSurf.FindXSec( 0 );
    if ( xs )
    {
        xs->m_RadiusFrac = rfirst;
    }

    m_XSecSurf.ChangeXSecShape( nxsec - 1, XS_ONE_SIX_SERIES );
    xs = ( PropXSec* ) m_XSecSurf.FindXSec( nxsec - 1 );
    if ( xs )
    {
        xs->m_RadiusFrac = rlast;
    }

    for ( int i = nxsec - 2; i > 0; i-- )
    {
        m_XSecSurf.CutXSec( i );
    }

    m_Diameter = diam;
    m_Nblade = num_blade;
    m_Beta34 = beta34;
    m_Feather = feather;
    m_Precone = precone;

    m_XRelLoc = cen.x();
    m_YRelLoc = cen.y();
    m_ZRelLoc = cen.z();

    norm = norm * -1.0;
    norm.normalize();
    vec3d minor;
    minor.v[ norm.minor_comp() ] = 1.0;
    vec3d counter = cross( norm, minor );
    counter.normalize();
    minor = cross( counter, norm );
    minor.normalize();

    Matrix4d mat;
    mat.setBasis( norm, minor, counter );

    vec3d angles = mat.getAngles();
    m_XRelRot = angles.x();
    m_YRelRot = angles.y();
    m_ZRelRot = angles.z();

    m_ChordCurve.SetCurve( r_vec, chord_vec, vsp::PCHIP );
    m_TwistCurve.SetCurve( r_vec, twist_vec, vsp::PCHIP );
    m_RakeCurve.SetCurve( r_vec, rake_vec, vsp::PCHIP );
    m_SkewCurve.SetCurve( r_vec, skew_vec, vsp::PCHIP );
    m_SweepCurve.SetCurve( r_vec, sweep_vec, vsp::PCHIP );
    m_ThickCurve.SetCurve( r_vec, thick_vec, vsp::PCHIP );
    m_CLICurve.SetCurve( r_vec, cli_vec, vsp::PCHIP );
    m_AxialCurve.SetCurve( r_vec, axial_vec, vsp::PCHIP );
    m_TangentialCurve.SetCurve( r_vec, tangential_vec, vsp::PCHIP );

    return 1;
}

PCurve* PropGeom::GetPCurve( int curveid )
{
    if ( curveid >= 0 && curveid < NUM_PROP_PCURVE )
    {
        return m_pcurve_vec[ curveid ];
    }
    return NULL;
}

void PropGeom::WriteAirfoilFiles( FILE* meta_fid )
{
    // This function writes out the coordinate/control point data for all untwisted unit length airfoils.
    //  Special considerations need to be taken for PropGeoms because the interpolated airfoils do
    //  not start and/or end at each XSec. Sectional U tessellation is not considered for PropGeoms. 

    Vehicle* veh = VehicleMgr.GetVehicle();
    if ( !veh || !meta_fid || m_MainSurfVec.size() == 0 )
    {
        return;
    }

    // Adjust Tessellation
    if ( veh->m_AFExportType() == vsp::SELIG_AF_EXPORT && std::abs( veh->m_AFWTessFactor() - 1.0 ) >= FLT_EPSILON )
    {
        m_TessW.Set( m_TessW() * veh->m_AFWTessFactor() );
    }

    string xsecsurf_id;
    int numXsurf = GetNumXSecSurfs(); // Should always be 1 for wing and prop geoms

    for ( size_t i = 0; i < numXsurf; i++ )
    {
        XSecSurf* sec_surf = GetXSecSurf( i );
        if ( sec_surf )
        {
            xsecsurf_id = sec_surf->GetID();
        }
    }

    // Get the untwisted wing surface
    VspSurf* foil_surf = m_MainSurfVec[0].GetFoilSurf();

    unsigned int numUsec = foil_surf->GetNumSectU();

    // Identify starting and ending U values from end cap options
    double Umin = 0;
    if ( m_CapUMinOption() != NO_END_CAP && m_CapUMinSuccess[0] )
    {
        Umin = 1.0;
    }

    double Umax = m_MainSurfVec[0].GetUMax();
    if ( m_CapUMaxOption() != NO_END_CAP && m_CapUMaxSuccess[0] )
    {
        Umax -= 1.0;
    }

    double ustep = ( ( Umax - Umin ) / numUsec ) / m_MainSurfVec[0].GetUMax();
    double umin = Umin / m_MainSurfVec[0].GetUMax();
    double umax = Umax / m_MainSurfVec[0].GetUMax();

    double Vmin = 0.0;
    double Vmax = GetMainWMax(0);
    double Vle = ( Vmin + Vmax ) * 0.5;

    // First and last XSec accounted for by tessellated airfoils. Get additional XSec u locations 
    vector < double > XSec_u_vec;
    XSec_u_vec.resize( numUsec - 1 );

    for ( size_t i = 1; i < numUsec; i++ )
    {
        XSec_u_vec[i - 1] = umin + ( i * ustep );
    }

    int XSec_index = 0;
    int foil_cnt = 0;
    int xsec_count = 0;

    for ( size_t j = 0; j < m_TessU(); j++ )
    {
        string af_file_name = m_Name + "_";

        if ( veh->m_AFAppendGeomIDFlag() )
        {
            af_file_name += ( m_ID + "_" );
        }

        af_file_name += to_string( foil_cnt );

        if ( veh->m_AFExportType() == vsp::SELIG_AF_EXPORT )
        {
            af_file_name += ".dat";
        }
        else if ( veh->m_AFExportType() == vsp::BEZIER_AF_EXPORT )
        {
            af_file_name += ".bz";
        }

        double u = (double)j / ( m_TessU() - 1 );
        double u_global = umin + ( ( umax - umin ) * ( (double)j / ( m_TessU() - 1.0 ) ) ); // Get u value on main surface

        bool xsec_flag = false;
        if ( j == 0 || j == m_TessU() - 1 || u_global == XSec_u_vec[XSec_index] )
        {
            xsec_flag = true;
        }

        fprintf( meta_fid, "########################################\n" );
        fprintf( meta_fid, "Airfoil File Name, %s\n", af_file_name.c_str() );
        fprintf( meta_fid, "Geom Name, %s\n", m_Name.c_str() );
        fprintf( meta_fid, "Geom ID, %s\n", m_ID.c_str() );
        fprintf( meta_fid, "Airfoil Index, %d\n", foil_cnt );
        fprintf( meta_fid, "XSec Flag, %d\n", xsec_flag );

        if ( xsec_flag )
        {
            fprintf( meta_fid, "XSec Index, %d\n", xsec_count );
            fprintf( meta_fid, "XSecSurf ID, %s\n", xsecsurf_id.c_str() );
        }

        vec3d le_pnt = m_MainSurfVec[0].CompPnt01( u_global, ( Vle / Vmax ) );
        vec3d te_pnt = m_MainSurfVec[0].CompPnt01( u_global, ( Vmin / Vmax ) );
        double chord = dist( le_pnt, te_pnt );

        fprintf( meta_fid, "FoilSurf u Value, %f\n", u );
        fprintf( meta_fid, "Global u Value, %f\n", u_global );
        fprintf( meta_fid, "Leading Edge Point, %f, %f, %f\n", le_pnt.x(), le_pnt.y(), le_pnt.z() );
        fprintf( meta_fid, "Trailing Edge Point, %f, %f, %f\n", te_pnt.x(), te_pnt.y(), te_pnt.z() );
        fprintf( meta_fid, "Chord, %f\n", chord );
        fprintf( meta_fid, "########################################\n\n" );

        if ( veh->m_AFExportType() == vsp::SELIG_AF_EXPORT )
        {
            WriteSeligAirfoil( ( veh->m_AFFileDir + af_file_name ), u );
        }
        else if ( veh->m_AFExportType() == vsp::BEZIER_AF_EXPORT )
        {
            WriteBezierAirfoil( ( veh->m_AFFileDir + af_file_name ), u );
        }

        foil_cnt++;

        if ( xsec_flag )
        {
            xsec_count++;
        }

        // Add XSec airfoils between tessellated interpolated airfoils
        if ( XSec_u_vec.size() > 0 )
        {
            double u_main = u_global;
            double u_next = umin + ( (double)( j + 1 ) * ( ustep * numUsec ) / ( m_TessU() - 1 ) );

            if ( XSec_u_vec[XSec_index] > u_main && XSec_u_vec[XSec_index] < u_next )
            {
                u = ( ( XSec_index + 1 ) / (double)numUsec );

                af_file_name = m_Name + "_";

                if ( veh->m_AFAppendGeomIDFlag() )
                {
                    af_file_name += ( m_ID + "_" );
                }

                af_file_name += to_string( foil_cnt );

                if ( veh->m_AFExportType() == vsp::SELIG_AF_EXPORT )
                {
                    af_file_name += ".dat";
                }
                else if ( veh->m_AFExportType() == vsp::BEZIER_AF_EXPORT )
                {
                    af_file_name += ".bz";
                }

                fprintf( meta_fid, "########################################\n" );
                fprintf( meta_fid, "Airfoil File Name, %s\n", af_file_name.c_str() );
                fprintf( meta_fid, "Geom Name, %s\n", m_Name.c_str() );
                fprintf( meta_fid, "Geom ID, %s\n", m_ID.c_str() );
                fprintf( meta_fid, "Airfoil Index, %d\n", foil_cnt );
                fprintf( meta_fid, "XSec Flag, %d\n", true );
                fprintf( meta_fid, "XSec Index, %d\n", xsec_count );
                fprintf( meta_fid, "XSecSurf ID, %s\n", xsecsurf_id.c_str() );

                u_global = XSec_u_vec[XSec_index]; // Get u value on main surface

                le_pnt = m_MainSurfVec[0].CompPnt01( u_global, ( Vle / Vmax ) );
                te_pnt = m_MainSurfVec[0].CompPnt01( u_global, ( Vmin / Vmax ) );
                chord = dist( le_pnt, te_pnt );

                fprintf( meta_fid, "FoilSurf u Value, %f\n", u );
                fprintf( meta_fid, "Global u Value, %f\n", u_global );
                fprintf( meta_fid, "Leading Edge Point, %f, %f, %f\n", le_pnt.x(), le_pnt.y(), le_pnt.z() );
                fprintf( meta_fid, "Trailing Edge Point, %f, %f, %f\n", te_pnt.x(), te_pnt.y(), te_pnt.z() );
                fprintf( meta_fid, "Chord, %f\n", chord );
                fprintf( meta_fid, "########################################\n\n" );

                if ( veh->m_AFExportType() == vsp::SELIG_AF_EXPORT )
                {
                    WriteSeligAirfoil( ( veh->m_AFFileDir + af_file_name ), u );
                }
                else if ( veh->m_AFExportType() == vsp::BEZIER_AF_EXPORT )
                {
                    WriteBezierAirfoil( ( veh->m_AFFileDir + af_file_name ), u );
                }

                foil_cnt++;
                xsec_count++;

                if ( XSec_index == XSec_u_vec.size() - 1 )
                {
                    XSec_u_vec.clear();
                }
                else
                {
                    XSec_index++;
                }
            }
        }
    }

    // Restore Tessellation
    if ( veh->m_AFExportType() == vsp::SELIG_AF_EXPORT && abs( veh->m_AFWTessFactor() - 1.0 ) >= FLT_EPSILON )
    {
        m_TessW.Set( m_TessW.GetLastVal() );
    }
}

vector< TMesh* > PropGeom::CreateTMeshVec() const
{
    vector< TMesh* > TMeshVec;

    if ( m_ExportMainSurf )
    {
        vector<VspSurf> surf_vec;
        surf_vec = GetMainSurfVecConstRef();

        TMeshVec = Geom::CreateTMeshVec( surf_vec );
    }
    else
    {
        TMeshVec = Geom::CreateTMeshVec();
    }

    return TMeshVec;
}

const vector<VspSurf> & PropGeom::GetSurfVecConstRef() const
{
    if ( m_ExportMainSurf )
    {
        return GetMainSurfVecConstRef();
    }
    else
    {
        return Geom::GetSurfVecConstRef();
    }
}


void PropGeom::ApproxCubicAllPCurves()
{
    for ( int i = 0; i < NUM_PROP_PCURVE; i++ )
    {
        if ( m_pcurve_vec[i] )
        {
            m_pcurve_vec[i]->Approximate();
        }
    }
}

void PropGeom::ResetThickness()
{
    unsigned int nxsec = m_XSecSurf.NumXSec();

    vector < double > rvec( nxsec );
    vector < double > tcvec( nxsec );

    //==== Update XSec Location/Rotation ====//
    for ( int i = 0 ; i < nxsec ; i++ )
    {
        PropXSec* xs = ( PropXSec* ) m_XSecSurf.FindXSec( i );

        if ( xs )
        {
            rvec[i] = xs->m_RadiusFrac();

            XSecCurve *xsc = xs->GetXSecCurve();
            if ( xsc )
            {
                Airfoil* af = dynamic_cast < Airfoil* > ( xsc );
                if ( af )
                {
                    FileAirfoil* faf = dynamic_cast < FileAirfoil* > ( af );
                    if ( faf )
                    {
                        tcvec[i] = faf->m_BaseThickness();
                    }
                    else
                    {
                        tcvec[i] = af->m_ThickChord();
                    }
                }
                else
                {
                    string height_id = xsc->GetHeightParmID();
                    Parm* height_parm = ParmMgr.FindParm( height_id );

                    string width_id = xsc->GetWidthParmID();
                    Parm* width_parm = ParmMgr.FindParm( width_id );

                    tcvec[i] = height_parm->Get() / width_parm->Get();
                }
            }
        }
    }

    m_ThickCurve.SetCurve( rvec, tcvec, PCHIP );
}
