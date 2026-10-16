//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//
//
//////////////////////////////////////////////////////////////////////

#include <cmath>
#include <vector>
#include <algorithm>

#include "PropGeom.h"
#include "ParmMgr.h"
#include "Vehicle.h"
#include "VehicleMgr.h"
#include <float.h>

#include <eli/mutil/nls/newton_raphson_system_method.hpp>

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
    m_ParentProp = nullptr;

    m_Chord = 1.0;

    m_Construct = 0.5;
    m_FeatherOffset = 0.0;
    m_FeatherAxis = 0.5;
    m_RootChord = 1.0;
    m_RootTwist = 0.0;

    m_Twist = 0.0;
    m_ZRotate = 0.0;

    m_CurveSection = false;

    m_Radius = 0.0;
    m_Rake = 0.0;
    m_Skew = 0.0;
    m_Sweep = 0.0;
    m_Axial = 0.0;
    m_Tangential = 0.0;

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
    mat.translatef( xb * sin( m_RootTwist * M_PI / 180.0), 0, m_Reverse * xb * cos( m_RootTwist * M_PI / 180.0) );

    mat.rotateX( m_Reverse * m_Sweep ); // About axis of rotation

    mat.translatef( 0, m_Radius, 0 );

    double x = -m_RootChord * ( 0.5 - m_Construct );
    mat.translatef( x * sin( m_RootTwist * M_PI / 180.0), 0, m_Reverse * x * cos( m_RootTwist * M_PI / 180.0) );

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

    mat.loadIdentity();

    // Offset the blade from its feather axis.  Feathering itself is rigid motion of the blade.
    mat.translatef( 0, 0, m_Reverse * m_RootChord * m_FeatherOffset );

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

// Blade axes: x aft (toward the trailing edge), y radial, z toward the thrust.  With reverse
// rotation (rev -1) the map to propeller axes is a reflection, so anything given in blade axes
// mirrors with the propeller.
static vec3d BladeToProp( const vec3d & b, double rev )
{
    return vec3d( -b.z(), b.y(), rev * b.x() );
}

// An axis of rotation also changes sign under the reflection.
static vec3d BladeAxisToProp( const vec3d & a, double rev )
{
    return BladeToProp( a, rev ) * rev;
}

// Delete the parms past the first n.
static void TrimParmVec( vector< Parm* > & vec, int n )
{
    while ( ( int )vec.size() > n )
    {
        delete vec.back();
        vec.pop_back();
    }
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

void PropXSec::GetDrawLines( Matrix4d &transMat, vector< vec3d > &lines )
{
    lines.clear();

    VspCurve curve = GetCurve();

    curve.Scale( 1.0 / m_PropPos.m_Chord );

    vec3d v = curve.CompPnt( 0 );
    curve.OffsetZ( -v.z() );

    m_PropPos.SetCurve( curve );
    m_PropPos.Update();
    curve = m_PropPos.GetCurve();

    curve.Transform( transMat );
    curve.TessAdapt( lines, 1e-2, 10 );
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

    m_ActiveBlade.Init( "ActiveBlade", "Index", this, 1, 1, 1000 );
    m_ActiveBlade.SetDescript( "Active blade" );

    m_BladeAzimuthMode.Init( "BladeAzimuthMode", "Design", this, PROP_AZI_UNIFORM, PROP_AZI_UNIFORM, NUM_PROP_AZI - 1 );
    m_BladeAzimuthMode.SetDescript( "Blade angle mode control" );

    m_BladeAzimuthDeltaFlag.Init( "BladeAzimuthDeltaFlag", "Design", this, 0, 0, 1 );
    m_BladeAzimuthDeltaFlag.SetDescript( "Flag to determine whether blade azimuth is absolute or deltas from uniform" );

    m_BalanceX1.Init( "BalanceX1", "Design", this, 0, -1, 1 );
    m_BalanceX1.SetDescript( "Normalized rotor balance point in blade 1 direction." );

    m_BalanceX2.Init( "BalanceX2", "Design", this, 0, -1, 1 );
    m_BalanceX2.SetDescript( "Normalized rotor balance point in direction perpendicular to blade 1." );

    m_TipMarkerScaleFlag.Init( "TipMarkerScaleFlag", "Attach", this, 0, 0, 1 );
    m_TipMarkerScaleFlag.SetDescript( "Flag for rendering the propeller vector." );

    m_TipMarkerScale.Init( "TipMarkerScale", "Attach", this, 1, 0, 2 );
    m_TipMarkerScale.SetDescript( "Set propeller render vector as ratio of diameter." );

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

    m_RadFeatherAxis.Init( "RFeatherAx", "Articulation", this, 0.15, 0.0, 1.0 );
    m_RadFeatherAxis.SetDescript( "Radial position of feather bearing as fraction of radius; orders it among the hinges" );

    m_RadFlapAxis.Init( "RFlapAx", "Articulation", this, 0.05, 0.0, 1.0 );
    m_RadFlapAxis.SetDescript( "Radial position of flap axis as fraction of radius" );

    m_AxialFlapAxis.Init( "AxFlapAx", "Articulation", this, 0, -1.0, 1.0 );
    m_AxialFlapAxis.SetDescript( "Axial position of flap axis as fraction of radius" );

    m_OffsetFlapAxis.Init( "OffFlapAx", "Articulation", this, 0, -1.0, 1.0 );
    m_OffsetFlapAxis.SetDescript( "Offset position of flap axis as fraction of radius" );

    m_AzimuthFlapDir.Init( "AzFlapDir", "Articulation", this, 0.0, -90.0, 90.0 );
    m_AzimuthFlapDir.SetDescript( "Azimuth angle of flap axis direction vector, theta_F" );

    m_ElevationFlapDir.Init( "ElFlapDir", "Articulation", this, 0.0, -90.0, 90.0 );
    m_ElevationFlapDir.SetDescript( "Elevation angle of flap axis direction vector, delta_3" );

    m_FlapAngle.Init( "FlapAngle", "Articulation", this, 0.0, -1.0e12, 1.0e12 );
    m_FlapAngle.SetDescript( "Blade flap angle" );

    m_FlapMean.Init( "FlapMean", "Articulation", this, 0.0, -180.0, 180.0 );
    m_FlapMean.SetDescript( "Mean flap angle of the harmonic blade motion" );

    m_RadLagAxis.Init( "RLagAx", "Articulation", this, 0.1, 0.0, 1.0 );
    m_RadLagAxis.SetDescript( "Radial position of lag axis as fraction of radius" );

    m_AxialLagAxis.Init( "AxLagAx", "Articulation", this, 0, -1.0, 1.0 );
    m_AxialLagAxis.SetDescript( "Axial position of lag axis as fraction of radius" );

    m_OffsetLagAxis.Init( "OffLagAx", "Articulation", this, 0, -1.0, 1.0 );
    m_OffsetLagAxis.SetDescript( "Offset position of lag axis as fraction of radius" );

    m_AzimuthLagDir.Init( "AzLagDir", "Articulation", this, 0.0, -90.0, 90.0 );
    m_AzimuthLagDir.SetDescript( "Azimuth angle of lag axis direction vector, theta_L" );

    m_CantLagDir.Init( "CantLagDir", "Articulation", this, 0.0, -90.0, 90.0 );
    m_CantLagDir.SetDescript( "Cant angle of lag axis direction vector, alpha_1" );

    m_LagAngle.Init( "LagAngle", "Articulation", this, 0.0, -1.0e12, 1.0e12 );
    m_LagAngle.SetDescript( "Blade lag angle" );

    m_LagMean.Init( "LagMean", "Articulation", this, 0.0, -180.0, 180.0 );
    m_LagMean.SetDescript( "Mean lag angle of the harmonic blade motion" );

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

    m_IndividualBladeFoldFlag.Init( "IndividualBladeFoldFlag", "Design", this, false, 0, 1 );
    m_IndividualBladeFoldFlag.SetDescript( "Flag to enable individual blade fold angles" );

    m_PsiRef.Init( "PsiRef", "Articulation", this, 0.0, -360.0, 360.0 );
    m_PsiRef.SetDescript( "Azimuth where psi is zero, in the direction of rotation; Rotate turns blade 1 from it" );

    m_BladeMotionMode.Init( "BladeMotionMode", "Articulation", this, vsp::PROP_BLADE_COLLECTIVE, vsp::PROP_BLADE_COLLECTIVE,
                            vsp::NUM_PROP_BLADE_MOTION - 1 );
    m_BladeMotionMode.SetDescript( "How the blades take the flap, lag and feather angles" );

    m_NumHarmonics.Init( "NumHarmonics", "Articulation", this, 1, 0, 20 );
    m_NumHarmonics.SetDescript( "Number of harmonics of the flap, lag and feather angles" );

    m_FeatherMean.Init( "FeatherMean", "Articulation", this, 0.0, -400.0, 400.0 );
    m_FeatherMean.SetDescript( "Mean feather angle of the harmonic blade motion" );

    m_Beta34.Init( "Beta34", "Design", this, 20.0, -1.0e12, 1.0e12 );
    m_Beta34.SetDescript( "Blade pitch at 3/4 of radius" );

    m_Feather.Init( "Feather", "Design", this, 0.0, -1.0e12, 1.0e12 );
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

    m_Solidity.Init( "Solidity", "Design", this, 0.5, 0, 1.0e12 );
    m_Solidity.SetDescript( "Geometric blade solidity" );

    m_TSolidity.Init( "TSolidity", "Design", this, 0.5, 0, 1.0e12 );
    m_TSolidity.SetDescript( "Thrust weighted blade solidity" );

    m_PSolidity.Init( "PSolidity", "Design", this, 0.5, 0, 1.0e12 );
    m_PSolidity.SetDescript( "Power weighted blade solidity" );

    m_Chord.Init( "Chord", "Design", this, 0.5, 0, 1.0e12 );
    m_Chord.SetDescript( "Geometric blade chord" );

    m_TChord.Init( "TChord", "Design", this, 0.5, 0, 1.0e12 );
    m_TChord.SetDescript( "Thrust weighted blade chord" );

    m_PChord.Init( "PChord", "Design", this, 0.5, 0, 1.0e12 );
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

    m_Twist34 = 0.0;
    m_BladeRootChord = 0.0;
    m_BladeCapUMinSuccess = false;
    m_BladeCapUMaxSuccess = false;
    m_BladeTessDirty = true;
    m_BladeDegenDirty = true;

    m_XSecSurf.SetXSecType( XSEC_PROP );

    m_ChordCurve.SetParentContainer( GetID() );
    m_ChordCurve.SetDispNames( "r/R", "Chord/R" );
    m_ChordCurve.SetParmNames( "r", "crd" );
    m_ChordCurve.SetCurveName( "Chord" );
    m_ChordCurve.InitParms();

    m_TwistCurve.SetParentContainer( GetID() );
    m_TwistCurve.SetDispNames( "r/R", "Twist" );
    m_TwistCurve.SetParmNames( "r", "tw" );
    m_TwistCurve.SetCurveName( "Twist" );
    m_TwistCurve.InitParms();

    m_RakeCurve.SetParentContainer( GetID() );
    m_RakeCurve.SetDispNames( "r/R", "Rake/R" );
    m_RakeCurve.SetParmNames( "r", "rak" );
    m_RakeCurve.SetCurveName( "Rake" );
    m_RakeCurve.InitParms();

    m_SkewCurve.SetParentContainer( GetID() );
    m_SkewCurve.SetDispNames( "r/R", "Skew/R" );
    m_SkewCurve.SetParmNames( "r", "skw" );
    m_SkewCurve.SetCurveName( "Skew" );
    m_SkewCurve.InitParms();

    m_SweepCurve.SetParentContainer( GetID() );
    m_SweepCurve.SetDispNames( "r/R", "Sweep" );
    m_SweepCurve.SetParmNames( "r", "sw" );
    m_SweepCurve.SetCurveName( "Sweep" );
    m_SweepCurve.InitParms();

    m_ThickCurve.SetParentContainer( GetID() );
    m_ThickCurve.SetDispNames( "r/R", "Thick/C" );
    m_ThickCurve.SetParmNames( "r", "toc" );
    m_ThickCurve.SetCurveName( "Thick" );
    m_ThickCurve.InitParms();

    m_CLICurve.SetParentContainer( GetID() );
    m_CLICurve.SetDispNames( "r/R", "CLi" );
    m_CLICurve.SetParmNames( "r", "cli" );
    m_CLICurve.SetCurveName( "CLI" );
    m_CLICurve.InitParms();

    m_AxialCurve.SetParentContainer( GetID() );
    m_AxialCurve.SetDispNames( "r/R", "Axial/R" );
    m_AxialCurve.SetParmNames( "r", "ax" );
    m_AxialCurve.SetCurveName( "Axial" );
    m_AxialCurve.InitParms();

    m_TangentialCurve.SetParentContainer(GetID() );
    m_TangentialCurve.SetDispNames("r/R", "Tan/R" );
    m_TangentialCurve.SetParmNames("r", "tan" );
    m_TangentialCurve.SetCurveName("Tangential" );
    m_TangentialCurve.InitParms();

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

    InitPreset( PROP_PRESET_PROPELLER );
}

//==== Replace the sections, the blade curves, the pitch and the Y rotation with a starting design ====//
void PropGeom::InitPreset( int preset )
{
    m_XSecSurf.DeleteAllXSecs();
    m_ActiveXSec = 0;

    vector < int > xstype;
    vector < double > xsrad;

    vector < double > t3 = { 0.2, 1.0 };
    vector < double > zero = { 0.0, 0.0 };
    double yrot = 0.0;

    if ( preset == PROP_PRESET_HELICOPTER )
    {
        // A rotor blade: NACA 0012 from root to tip, constant chord, linear twist.
        xstype = { XS_FOUR_SERIES, XS_FOUR_SERIES };
        xsrad = { 0.2, 1.0 };

        m_ChordCurve.SetCurve( t3, { 0.06, 0.06 }, vsp::LINEAR );
        m_TwistCurve.SetCurve( t3, { 0.0, -8.0 }, vsp::LINEAR );
        m_ThickCurve.SetCurve( t3, { 0.12, 0.12 }, vsp::LINEAR );
        m_CLICurve.SetCurve( t3, zero, vsp::LINEAR );

        m_UseBeta34Flag.Set( 0 );
        m_Feather.Set( 8.0 );
        m_PsiRef.Set( 270.0 );

        // Thrust up.
        yrot = 90.0;
    }
    else
    {
        xstype = { XS_ELLIPSE, XS_ONE_SIX_SERIES, XS_ONE_SIX_SERIES };
        xsrad = { 0.2, 0.4, 1.0 };

        vector < double > t1 = { 0.2, 0.2 + 0.4 / 3.0,
                                 0.6 - 0.4 / 3.0, 0.6, 0.6 + 0.35 / 3.0,
                                 0.95 - 0.35 / 3.0, 0.95, 0.95 + 0.05 / 3.0,
                                 1.0 - 0.05 / 3.0, 1.0 };
        vector < double > v1 = { 0.08, 0.15,
                                 0.2, 0.2, 0.2,
                                 0.2, 0.2, 0.2,
                                 0.2, 0.13 };
        m_ChordCurve.SetCurve( t1, v1, vsp::CEDIT );

        m_TwistCurve.SetCurve( { 0.2, 0.75, 1.0 }, { 46.75, 20.0, 13.0 }, vsp::PCHIP );

        vector < double > t4 = { 0.2, 0.2 + 0.1 / 3.0,
                                 0.3 - 0.1 / 3.0, 0.3, 0.3 + 0.3 / 3.0,
                                 0.6 - 0.3 / 3.0, 0.6, 0.6 + 0.4 / 3.0,
                                 1.0 - 0.4 / 3.0, 1.0 };
        vector < double > v4 = { 0.50, 0.300,
                                 0.225, 0.20, 0.125,
                                 0.115, 0.10, 0.075,
                                 0.055, 0.03 };
        m_ThickCurve.SetCurve( t4, v4, vsp::CEDIT );

        vector < double > t5 = { 0.2, 0.2 + 0.2 / 3.0,
                                 0.4 - 0.2 / 3.0, 0.4, 0.4 + 0.2 / 3.0,
                                 0.6 - 0.2 / 3.0, 0.6, 0.6 + 0.4 / 3.0,
                                 1.0 - 0.4 / 3.0, 1.0 };
        vector < double > v5 = { 0.0, 0.7,
                                 0.7, 0.7, 0.7,
                                 0.7, 0.7, 0.7,
                                 0.7, 0.2 };
        m_CLICurve.SetCurve( t5, v5, vsp::CEDIT );

        m_UseBeta34Flag.Set( 1 );
        m_Beta34.Set( 20.0 );
        m_Feather.Set( 0.0 );
        m_PsiRef.Set( 0.0 );
    }

    m_RakeCurve.SetCurve( t3, zero, vsp::LINEAR );
    m_SkewCurve.SetCurve( t3, zero, vsp::LINEAR );
    m_SweepCurve.SetCurve( t3, zero, vsp::LINEAR );
    m_AxialCurve.SetCurve( t3, zero, vsp::LINEAR );
    m_TangentialCurve.SetCurve( t3, zero, vsp::LINEAR );

    // Prevent negative values in m_Chord
    for ( size_t i = 0; i < m_ChordCurve.m_ValParmVec.size(); i++ )
    {
        m_ChordCurve.m_ValParmVec[i]->SetLowerLimit( 1e-8 );
    }

    // The Y rotation in the frame the propeller is placed in.
    if ( m_AbsRelFlag() == vsp::REL )
    {
        m_YRelRot.Set( yrot );
    }
    else
    {
        m_YRot.Set( yrot );
    }

    for ( int j = 0; j < (int) xstype.size(); j++ )
    {
        m_XSecSurf.AddXSec( xstype[ j ] );
        PropXSec* xs = ( PropXSec* ) m_XSecSurf.FindXSec( j );
        xs->SetGroupDisplaySuffix( j );
        xs->m_RadiusFrac = xsrad[ j ];
    }
}

//==== Destructor ====//
PropGeom::~PropGeom()
{
    TrimParmVec( m_FlapAngleParmVec, 0 );
    TrimParmVec( m_LagAngleParmVec, 0 );
    TrimParmVec( m_FoldAngleParmVec, 0 );
    TrimParmVec( m_FeatherParmVec, 0 );
    TrimParmVec( m_FlapCosParmVec, 0 );
    TrimParmVec( m_FlapSinParmVec, 0 );
    TrimParmVec( m_LagCosParmVec, 0 );
    TrimParmVec( m_LagSinParmVec, 0 );
    TrimParmVec( m_FeatherCosParmVec, 0 );
    TrimParmVec( m_FeatherSinParmVec, 0 );
    TrimParmVec( m_BladeAzimuthParmVec, 0 );
    TrimParmVec( m_BladeDeltaAzimuthParmVec, 0 );
}

void PropGeom::UpdateDrawObj()
{
    GeomXSec::UpdateDrawObj();

    Matrix4d relTrans = m_AttachMatrix;
    relTrans.affineInverse();
    relTrans.matMult( m_ModelMatrix.data() );
    relTrans.postMult( m_AttachMatrix.data() );

    // The flip goes innermost.
    relTrans.matMult( GetFlipMat().data() );

    Matrix4d invRelTrans = relTrans;
    invRelTrans.affineInverse();

    Matrix4d rigid;
    RigidBladeMotion( rigid, 0, m_Rotate() );
    for ( int i = 0 ; i < m_XSecDrawObj_vec.size() ; i++ )
    {
        invRelTrans.xformvec( m_XSecDrawObj_vec[ i ].m_PntVec );
        rigid.xformvec( m_XSecDrawObj_vec[ i ].m_PntVec );
        relTrans.xformvec( m_XSecDrawObj_vec[ i ].m_PntVec );
    }

    if ( m_TipMarkerScaleFlag.Get() )
    {
        m_TipMarkerScale.Activate();
    }
    else
    {
        m_TipMarkerScale.Deactivate();
    }
}

// The thrust, rotation direction and blade hinges at each of placer's symmetric copies.  The
// rotation direction follows each copy's orientation, flip included.
void PropGeom::BuildMarkerDrawObjs( Geom* placer, vector< DrawObj > &marker_vec )
{
    if ( marker_vec.size() != NUM_PROP_MARKERS )
    {
        marker_vec.clear();
        marker_vec.resize( NUM_PROP_MARKERS );
    }

    DrawObj &arrow_lines = marker_vec[ PROP_MARKER_LINES ];
    DrawObj &arrow_heads = marker_vec[ PROP_MARKER_HEADS ];

    arrow_lines.m_PntVec.clear();
    arrow_heads.m_PntVec.clear();
    arrow_heads.m_NormVec.clear();

    arrow_lines.m_GeomChanged = true;
    arrow_heads.m_GeomChanged = true;

    for ( int i = PROP_MARKER_HINGES; i < NUM_PROP_MARKERS; i++ )
    {
        marker_vec[i].m_PntVec.clear();
        marker_vec[i].m_NormVec.clear();
        marker_vec[i].m_GeomChanged = true;
    }

    double axlen = 1.0;
    double rot_axlen = 1.0;



    Vehicle *veh = VehicleMgr.GetVehicle();
    if ( veh )
    {
        rot_axlen = axlen = veh->m_AxisLength();
    }

    if ( m_TipMarkerScaleFlag.Get() )
    {
        rot_axlen = m_TipMarkerScale() * m_Diameter() / 2;
    }

    vector< Matrix4d > trans_vec = placer->GetTransMatVec();
    int nmain = placer->GetNumMainSurfs();

    for ( int i = 0; i < placer->GetNumSymmCopies() && nmain > 0 && i * nmain < ( int )trans_vec.size(); i++)
    {
        Matrix4d trans_mat = trans_vec[i * nmain]; // Translations for the specific symmetric copy

        vec3d cen( 0, 0, 0 );
        vec3d rotdir( -1, 0, 0 );
        vec3d thrustdir( -1, 0, 0 );
        vec3d refdir( 0, 1, 0 );

        double rev = 1.0;
        if ( !placer->GetFlipNormal( i * nmain ) )
        {
            // Note inverse of m_FipNormalVec is used because Props are flipped by 
            // default (m_XSecSurf.GetFlipUD() in UpdateSurf())
            rev = -1.0;
        }

        rotdir = rotdir * rev;

        cen = trans_mat.xform( cen );
        rotdir = trans_mat.xform( rotdir ) - cen;
        thrustdir = trans_mat.xform( thrustdir ) - cen;
        refdir = trans_mat.xform( refdir ) - cen;

        // Each hinge of the active blade where the hinges inboard of it carry it.
        if ( m_PropMode() <= PROP_MODE::PROP_BOTH )
        {
            // A Clone may be turned apart from its original.
            double rotate = m_Rotate();
            RotorRole* rotor = Geom::CastTo< RotorRole >( placer );
            if ( rotor )
            {
                rotate = rotor->GetRotorRotate();
            }

            int iblade = m_ActiveBlade() - 1;
            Matrix4d mat;
            BladeRootMotion( mat, iblade, rotate );

            // The hinge axes already mirror with reverse rotation, so their arrows turn with the
            // copy's own mirror alone.
            double hrev = rev;
            if ( m_ReverseFlag() )
            {
                hrev = -rev;
            }

            vector< BladeHinge > hinges = GetBladeHinges( iblade, rotate );
            for ( int j = 0; j < ( int )hinges.size(); j++ )
            {
                const BladeHinge & h = hinges[j];
                DrawObj &hinge_lines = marker_vec[ PROP_MARKER_HINGES + 2 * h.m_Type ];
                DrawObj &hinge_heads = marker_vec[ PROP_MARKER_HINGES + 2 * h.m_Type + 1 ];

                Matrix4d hmat = mat;
                hmat.postMult( trans_mat );

                vec3d pmid = hmat.xform( h.m_Origin );
                vec3d ptstart = hmat.xform( h.m_Origin - h.m_Axis * hrev * axlen / 2.0 );
                vec3d ptend = hmat.xform( h.m_Origin + h.m_Axis * hrev * axlen / 2.0 );

                vec3d dir = ptend - ptstart;
                dir.normalize();

                hinge_lines.m_PntVec.push_back( ptstart );
                hinge_lines.m_PntVec.push_back( ptend );
                MakeCircleArrow( pmid, dir, 0.5 * axlen, 0.5 * axlen, hinge_lines, hinge_heads );

                mat.rotateAbout( h.m_Angle, h.m_Axis, h.m_Origin );
            }
        }

        arrow_lines.m_PntVec.push_back( cen );
        arrow_lines.m_PntVec.push_back( cen + refdir * axlen );
        arrow_lines.m_PntVec.push_back( cen );
        arrow_lines.m_PntVec.push_back( cen + thrustdir * axlen );
        MakeArrowhead( cen + thrustdir * axlen, thrustdir, 0.25 * axlen, arrow_heads.m_PntVec, arrow_heads.m_NormVec );
        MakeCircleArrow( cen, rotdir, rot_axlen, axlen, arrow_lines, arrow_heads );



    }

    arrow_heads.m_GeomID = placer->GetID() + "Arrows";
    arrow_heads.m_Screen = DrawObj::VSP_MAIN_SCREEN;
    arrow_heads.m_LineWidth = 1.0;
    arrow_heads.m_Type = DrawObj::VSP_SHADED_TRIS;

    for ( int i = 0; i < 4; i++ )
    {
        arrow_heads.m_MaterialInfo.Ambient[i] = 0.2f;
        arrow_heads.m_MaterialInfo.Diffuse[i] = 0.1f;
        arrow_heads.m_MaterialInfo.Specular[i] = 0.7f;
        arrow_heads.m_MaterialInfo.Emission[i] = 0.0f;
    }
    arrow_heads.m_MaterialInfo.Diffuse[3] = 0.5f;
    arrow_heads.m_MaterialInfo.Shininess = 5.0f;

    arrow_lines.m_GeomID = placer->GetID() + "ALines";
    arrow_lines.m_Screen = DrawObj::VSP_MAIN_SCREEN;
    arrow_lines.m_LineWidth = 2.0;
    arrow_lines.m_Type = DrawObj::VSP_LINES;

    // Each hinge type in its own colour, heads shaded to match their lines.
    for ( int t = 0; t < NUM_HINGE_TYPES; t++ )
    {
        DrawObj &hinge_lines = marker_vec[ PROP_MARKER_HINGES + 2 * t ];
        DrawObj &hinge_heads = marker_vec[ PROP_MARKER_HINGES + 2 * t + 1 ];
        vec3d c = DrawObj::Color( HingeDrawColor( t ) );

        char str[32];
        snprintf( str, sizeof( str ), "%d", t );

        hinge_lines.m_GeomID = placer->GetID() + "HingeLines" + str;
        hinge_lines.m_Screen = DrawObj::VSP_MAIN_SCREEN;
        hinge_lines.m_LineWidth = 2.0;
        hinge_lines.m_LineColor = c;
        hinge_lines.m_Type = DrawObj::VSP_LINES;

        hinge_heads.m_GeomID = placer->GetID() + "HingeArrows" + str;
        hinge_heads.m_Screen = DrawObj::VSP_MAIN_SCREEN;
        hinge_heads.m_LineWidth = 1.0;
        hinge_heads.m_Type = DrawObj::VSP_SHADED_TRIS;

        for ( int i = 0; i < 3; i++ )
        {
            hinge_heads.m_MaterialInfo.Ambient[i] = 0.2f * ( float )c.v[i];
            hinge_heads.m_MaterialInfo.Diffuse[i] = ( float )c.v[i];
            hinge_heads.m_MaterialInfo.Specular[i] = 0.7f;
            hinge_heads.m_MaterialInfo.Emission[i] = 0.0f;
        }
        hinge_heads.m_MaterialInfo.Ambient[3] = 0.2f;
        hinge_heads.m_MaterialInfo.Diffuse[3] = 0.5f;
        hinge_heads.m_MaterialInfo.Specular[3] = 0.7f;
        hinge_heads.m_MaterialInfo.Emission[3] = 0.0f;
        hinge_heads.m_MaterialInfo.Shininess = 5.0f;
    }
}

int PropGeom::HingeDrawColor( int type )
{
    switch ( type )
    {
        case FLAP_HINGE:
            return DrawObj::DODGER_BLUE;
        case LAG_HINGE:
            return DrawObj::ORANGE_RED;
        case FOLD_HINGE:
            return DrawObj::MEDIUM_ORCHID;
        case FEATHER_HINGE:
            return DrawObj::LIME_GREEN;
    }

    return DrawObj::BLACK;
}

void PropGeom::SetMarkerVisibility( Geom* placer, vector< DrawObj > &marker_vec )
{
    bool visible = placer->ShowsMarkers();
    for ( int i = 0; i < ( int )marker_vec.size(); i++ )
    {
        marker_vec[i].m_Visible = visible;
    }
}

void PropGeom::UpdateHighlightDrawObj()
{
    GeomXSec::UpdateHighlightDrawObj();

    Matrix4d relTrans = m_AttachMatrix;
    relTrans.affineInverse();
    relTrans.matMult( m_ModelMatrix.data() );
    relTrans.postMult( m_AttachMatrix.data() );

    // The flip goes innermost.
    relTrans.matMult( GetFlipMat().data() );

    Matrix4d invRelTrans = relTrans;
    invRelTrans.affineInverse();

    Matrix4d rigid;
    RigidBladeMotion( rigid, 0, m_Rotate() );

    invRelTrans.xformvec( m_HighlightXSecDrawObj.m_PntVec );
    rigid.xformvec( m_HighlightXSecDrawObj.m_PntVec );
    relTrans.xformvec( m_HighlightXSecDrawObj.m_PntVec );

    m_HighlightBladeDrawObj.m_PntVec = m_MainBladeBBox.GetBBoxDrawLines();

    RigidBladeMotion( rigid, m_ActiveBlade() - 1, m_Rotate() );

    rigid.xformvec( m_HighlightBladeDrawObj.m_PntVec );

    relTrans.xformvec( m_HighlightBladeDrawObj.m_PntVec );


    m_HighlightBladeDrawObj.m_GeomChanged = true;
}

void PropGeom::LoadDrawObjs( vector< DrawObj* > & draw_obj_vec )
{

    // Get all geom DO's
    GeomXSec::LoadDrawObjs( draw_obj_vec );

    // Bypass XSec drawing for disk
    if ( m_PropMode() == PROP_MODE::PROP_DISK )
    {
        // Force GeomXSec specific DO's to non-visible.
        for ( int i = 0 ; i < m_XSecDrawObj_vec.size() ; i++ )
        {
            m_XSecDrawObj_vec[ i ].m_Visible = false;
        }
        m_HighlightXSecDrawObj.m_Visible = false;
    }

    m_HighlightBladeDrawObj.m_Screen = DrawObj::VSP_MAIN_SCREEN;
    m_HighlightBladeDrawObj.m_GeomID = BBOXHEADER + m_ID + "ACTIVE_BLADE";
    m_HighlightBladeDrawObj.m_Visible = m_Vehicle->IsGeomActive( m_ID ) && m_PropMode() != PROP_MODE::PROP_DISK;
    m_HighlightBladeDrawObj.m_LineWidth = 4.0;
    m_HighlightBladeDrawObj.m_LineColor = vec3d( 0.0, 1.0, 0.0 );
    m_HighlightBladeDrawObj.m_Type = DrawObj::VSP_LINES;
    draw_obj_vec.push_back( &m_HighlightBladeDrawObj );

}

void PropGeom::ChangeID( const string &id )
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

    // Beta34 less the feather angle.
    m_Twist34 = 0.0;
    if ( rfirst <= 0.75 )
    {
        m_Twist34 = m_TwistCurve.Comp( 0.75 );
    }


    //==== Cross Section Curves & joint info ====//
    vector< XSecCurve* > xsc_vec;
    xsc_vec.resize( nxsec, nullptr );
    vector< double > w_vec;
    w_vec.resize( nxsec, 0.0 );

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
            w_vec[i] = w;

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
            }
        }
    }

    // The flaps are sized from the section widths just set.
    NormalizeFlaps();

    for ( int i = 0 ; i < nxsec ; i++ )
    {
        if ( xsc_vec[i] )
        {
            base_crv_vec[i] = xsc_vec[i]->GetCurve();
            if ( w_vec[i] != 0 )
            {
                base_crv_vec[i].Scale( 1.0 / w_vec[i] );
            }
        }
    }

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

        m_BladeRootChord = croot;

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
                xs->m_PropPos.m_ZRotate = atan( -m_RakeCurve.Compdt( r ) - m_AxialCurve.Compdt( r ) ) * 180.0 / M_PI;

                xs->m_PropPos.m_Rake = m_RakeCurve.Comp( r ) * radius;
                xs->m_PropPos.m_Skew = m_SkewCurve.Comp( r ) * radius;
                xs->m_PropPos.m_Sweep = m_SweepCurve.Comp( r );
                xs->m_PropPos.m_Axial = m_AxialCurve.Comp( r ) * radius;
                xs->m_PropPos.m_Tangential = m_TangentialCurve.Comp( r ) * radius;

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

            pp.m_ZRotate = atan( -m_RakeCurve.Compdt( r ) - m_AxialCurve.Compdt( r ) ) * 180.0 / M_PI;

            pp.m_Rake = m_RakeCurve.Comp( r ) * radius;
            pp.m_Skew = m_SkewCurve.Comp( r ) * radius;

            pp.m_Sweep = m_SweepCurve.Comp( r );

            pp.m_Axial = m_AxialCurve.Comp( r ) * radius;
            pp.m_Tangential = m_TangentialCurve.Comp( r ) * radius;

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
        m_FoilSurf.SkinC0Uniform(crv_vec, m_UPseudo, false );

        // Build the one blade.  UpdatePostSurf makes a copy of it for each blade.
        m_BladeSurf = VspSurf();

        m_BladeSurf.SetMagicVParm( false );
        m_BladeSurf.SkinCubicSpline(rib_vec, m_UPseudo, tdisc, false );

        m_BladeSurf.SetMagicVParm( true );
        m_BladeSurf.SetSurfType( WING_SURF );
        m_BladeSurf.SetSurfCfdType( vsp::CFD_NORMAL );  // Make sure set to default, can be updated later.

        m_FoilSurf.SetMagicVParm( true );

        m_BladeSurf.SetFoilSurf( &m_FoilSurf );

        if ( m_XSecSurf.GetFlipUD() )
        {
            m_BladeSurf.FlipNormal();
        }

        if ( this->m_ReverseFlag() )
        {
            m_BladeSurf.FlipNormal();
        }

        // Cap the one blade; the copies inherit its caps.
        UpdateEndCaps();

        m_MainBladeBBox.Reset();
        m_BladeSurf.GetBoundingBox( m_MainBladeBBox );
    }

    // Build disk surface.
    if ( m_PropMode() >= PROP_MODE::PROP_BOTH )
    {
        m_DiskSurf = VspSurf();
        m_DiskSurf.CreateDisk( m_Diameter(), Y_DIR, Z_DIR );
        m_DiskSurf.SetSurfType( vsp::DISK_SURF );
        m_DiskSurf.SetSurfCfdType( vsp::CFD_TRANSPARENT );
        m_DiskSurf.SetMagicVParm( false );

        if ( !m_ReverseFlag() )
        {
            m_DiskSurf.FlipNormal();
        }

        if ( m_PropMode() == PROP_MODE::PROP_DISK )
        {
            // Provide fake mesh metric values.
            m_SmallPanelW = 0.0;
            m_MaxGrowth = 1.0;

            m_CappingDone = true;
        }
    }
}

// Cap the one blade; its copies take its caps.
void PropGeom::UpdateEndCaps()
{
    if ( m_CappingDone )
    {
        return;
    }
    m_CappingDone = true;

    CapSurf( m_BladeSurf, m_BladeCapUMinSuccess, m_BladeCapUMaxSuccess );
    UpdateCapParms();
}

void PropGeom::UpdateFeatureLines()
{
    if ( m_PropMode() <= PROP_MODE::PROP_BOTH )
    {
        m_BladeSurf.BuildFeatureLines( m_ForceXSecFlag );
    }
    if ( m_PropMode() >= PROP_MODE::PROP_BOTH )
    {
        m_DiskSurf.BuildFeatureLines( m_ForceXSecFlag );
    }
}

void PropGeom::UpdateLCurve()
{
    if ( m_PropMode() <= PROP_MODE::PROP_BOTH )
    {
        m_BladeSurf.InitUMapping();
        m_BladeSurf.BuildLCurve();
    }
    if ( m_PropMode() >= PROP_MODE::PROP_BOTH )
    {
        m_DiskSurf.InitUMapping();
        m_DiskSurf.BuildLCurve();
    }
}

void PropGeom::UpdateFlags()
{
    if ( m_PropMode() <= PROP_MODE::PROP_BOTH )
    {
        UpdateSurfFlags( m_BladeSurf );
    }
    if ( m_PropMode() >= PROP_MODE::PROP_BOTH )
    {
        UpdateSurfFlags( m_DiskSurf );
    }
}

// Make and place the blades: the per-blade values, the hinges, then each blade's rigid motion.
void PropGeom::UpdatePostSurf()
{
    ReserveBlades( m_Nblade() - 1 );
    ReserveHarmonics();

    m_ActiveBlade.SetUpperLimit( m_Nblade() );

    m_Solidity.Set( m_Chord() * m_Nblade() / M_PI );
    m_TSolidity.Set( m_TChord() * m_Nblade() / M_PI );
    m_PSolidity.Set( m_PChord() * m_Nblade() / M_PI );

    UpdateBladeAzimuth();

    CheckBalance();

    UpdateBladeMotion( m_BladeMotionMode(), m_FlapAngle, m_FlapAngleParmVec, m_FlapMean, m_FlapCosParmVec,
                       m_FlapSinParmVec );

    UpdateBladeMotion( m_BladeMotionMode(), m_LagAngle, m_LagAngleParmVec, m_LagMean, m_LagCosParmVec,
                       m_LagSinParmVec );

    if ( m_BladeMotionMode() == vsp::PROP_BLADE_HARMONIC )
    {
        m_NumHarmonics.Activate();
    }
    else
    {
        m_NumHarmonics.Deactivate();
    }

    if ( !m_IndividualBladeFoldFlag() )
    {
        for ( int i = 0; i < m_FoldAngleParmVec.size(); i++ )
        {
            m_FoldAngleParmVec[i]->Set( m_FoldAngle() );
        }
    }

    if ( m_UseBeta34Flag() == 1 )
    {
        m_Feather = m_Beta34() - m_Twist34;
    }

    // While Beta34 drives every blade takes its feather, but blades given their own keep them.
    int feather_mode = FeatherMode();
    if ( m_BladeMotionMode() == vsp::PROP_BLADE_INDIVIDUAL )
    {
        feather_mode = vsp::PROP_BLADE_INDIVIDUAL;
    }
    UpdateBladeMotion( feather_mode, m_Feather, m_FeatherParmVec, m_FeatherMean, m_FeatherCosParmVec,
                       m_FeatherSinParmVec );

    if ( m_UseBeta34Flag() == 1 )
    {
        m_Beta34.Activate();
        m_Feather.Deactivate();
        for ( int i = 0; i < ( int )m_FeatherParmVec.size(); i++ )
        {
            m_FeatherParmVec[i]->Deactivate();
        }
    }
    else
    {
        m_Beta34 = m_Feather() + m_Twist34;
        m_Beta34.Deactivate();
    }

    // The Blade counter goes back to the first blade while no blade is given its own angles.
    bool individual = m_IndividualBladeFoldFlag();
    individual = individual || m_BladeMotionMode() == vsp::PROP_BLADE_INDIVIDUAL;
    if ( !individual )
    {
        m_ActiveBlade.Set( 1 );
    }

    double radius = m_Diameter() / 2.0;

    double rev = 1.0;
    if ( m_ReverseFlag() )
    {
        rev = -1.0;
    }

    // Flap hinge.  Its vector starts aft (+x); azimuth then elevation turn it, as for the fold
    // hinge.
    Matrix4d flap;
    flap.rotateY( m_AzimuthFlapDir() );
    flap.rotateZ( -m_ElevationFlapDir() );
    m_FlapAxDirection = BladeAxisToProp( flap.xform( vec3d( 1, 0, 0 ) ), rev );
    m_FlapAxOrigin = BladeToProp( vec3d( m_OffsetFlapAxis(), m_RadFlapAxis(), -m_AxialFlapAxis() ) * radius, rev );

    // Lag hinge.  Its vector starts away from the thrust (-z); azimuth turns it about the radial
    // axis, cant about the chord.
    Matrix4d lag;
    lag.rotateY( m_AzimuthLagDir() );
    lag.rotateX( -m_CantLagDir() );
    m_LagAxDirection = BladeAxisToProp( lag.xform( vec3d( 0, 0, -1 ) ), rev );
    m_LagAxOrigin = BladeToProp( vec3d( m_OffsetLagAxis(), m_RadLagAxis(), -m_AxialLagAxis() ) * radius, rev );

    // Fold hinge.  Its vector starts forward (-x).
    Matrix4d fold;
    fold.rotateY( m_AzimuthFoldDir() );
    fold.rotateZ( -m_ElevationFoldDir() );
    m_FoldAxDirection = BladeAxisToProp( fold.xform( vec3d( -1, 0, 0 ) ), rev );
    m_FoldAxOrigin = BladeToProp( vec3d( m_OffsetFoldAxis(), m_RadFoldAxis(), -m_AxialFoldAxis() ) * radius, rev );

    // The feather axis is radial, at the feather offset from the blade; the bearing sits on it
    // at its radial place.  Its vector is +y.
    m_FeatherAxOrigin = BladeToProp( vec3d( m_BladeRootChord * m_FeatherOffset(), m_RadFeatherAxis() * radius, 0 ), rev );
    m_FeatherAxDirection = BladeAxisToProp( vec3d( 0, 1, 0 ), rev );

    GetMainSurfVecAtRotate( m_Rotate(), m_MainSurfVec );

    // The blades take the blade's caps; the disk has none.
    int nblade = NumPlacedBlades();
    int nsurf = m_MainSurfVec.size();
    m_CapUMinSuccess.resize( nsurf );
    m_CapUMaxSuccess.resize( nsurf );
    for ( int i = 0; i < nsurf; i++ )
    {
        m_CapUMinSuccess[i] = i < nblade && m_BladeCapUMinSuccess;
        m_CapUMaxSuccess[i] = i < nblade && m_BladeCapUMaxSuccess;
    }

    // Remove the sub-surfaces of blades that are gone.
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

// The parms that only move the finished blades.
bool PropGeom::MovesBladesOnly( Parm* parm_ptr )
{
    // The articulation parms have their own group.
    if ( parm_ptr->GetGroupName() == string( "Articulation" ) )
    {
        return true;
    }

    vector< Parm* > movers = { &m_Nblade, &m_Rotate, &m_Precone,
                               &m_Feather, &m_Beta34, &m_UseBeta34Flag,
                               &m_FoldAngle, &m_IndividualBladeFoldFlag, &m_RadFoldAxis, &m_AxialFoldAxis,
                               &m_OffsetFoldAxis, &m_AzimuthFoldDir, &m_ElevationFoldDir,
                               &m_BladeAzimuthMode, &m_BladeAzimuthDeltaFlag, &m_BalanceX1, &m_BalanceX2 };
    movers.insert( movers.end(), m_FoldAngleParmVec.begin(), m_FoldAngleParmVec.end() );
    movers.insert( movers.end(), m_BladeAzimuthParmVec.begin(), m_BladeAzimuthParmVec.end() );
    movers.insert( movers.end(), m_BladeDeltaAzimuthParmVec.begin(), m_BladeDeltaAzimuthParmVec.end() );

    return std::find( movers.begin(), movers.end(), parm_ptr ) != movers.end();
}

void PropGeom::SetDirtyFlags( Parm* parm_ptr )
{
    if ( parm_ptr && MovesBladesOnly( parm_ptr ) )
    {
        m_PostSurfDirty = true;
        return;
    }

    // The hinge markers are drawn on the active blade.
    if ( parm_ptr == &m_ActiveBlade )
    {
        m_MarkerDirty = true;
    }

    GeomXSec::SetDirtyFlags( parm_ptr );
}

// The blades placed as main surfaces: none in disk-only mode.
int PropGeom::NumPlacedBlades()
{
    if ( m_PropMode() <= PROP_MODE::PROP_BOTH )
    {
        return m_Nblade();
    }
    return 0;
}

// A copy of the blade for each blade, turned into place, then the disk, if any.  The surfaces
// that stay keep their storage, so copying into them does not allocate.
void PropGeom::GetMainSurfVecAtRotate( double rotate, vector< VspSurf > &surfs )
{
    int nblade = NumPlacedBlades();
    int nsurf = nblade;
    if ( m_PropMode() >= PROP_MODE::PROP_BOTH )
    {
        nsurf++;
    }

    surfs.resize( nsurf );

    Matrix4d rigid;
    for ( int i = 0; i < nblade; i++ )
    {
        surfs[i] = m_BladeSurf;
        RigidBladeMotion( rigid, i, rotate );
        surfs[i].Transform( rigid );
    }

    if ( nsurf > nblade )
    {
        surfs[ nblade ] = m_DiskSurf;
    }
}

// The blade's tessellation copied and turned into place for each blade, then the disk's, assigned
// into the existing elements so their buffers are reused.
void PropGeom::GetMainTessVecAtRotate( double rotate, vector< SimpleTess > &tess, vector< SimpleFeatureTess > &ftess )
{
    int nblade = NumPlacedBlades();
    int nmain = nblade;
    if ( m_PropMode() >= PROP_MODE::PROP_BOTH )
    {
        nmain++;
    }

    tess.resize( nmain );
    ftess.resize( nmain );

    Matrix4d rigid;
    for ( int i = 0; i < nblade; i++ )
    {
        tess[i] = m_BladeTess;
        ftess[i] = m_BladeFeatureTess;
        RigidBladeMotion( rigid, i, rotate );
        tess[i].Transform( rigid );
        ftess[i].Transform( rigid );
    }

    if ( nmain > nblade )
    {
        tess[ nblade ] = m_DiskTess;
        ftess[ nblade ] = m_DiskFeatureTess;
    }
}

// The blade's preview copied and turned into place for each blade.  The disk has none.
void PropGeom::GetMainDegenGeomPreviewAtRotate( double rotate, vector< DegenGeom > &degen )
{
    int nblade = NumPlacedBlades();
    degen.resize( nblade );

    Matrix4d rigid;
    for ( int i = 0; i < nblade; i++ )
    {
        degen[i] = m_BladeDegenGeom;
        RigidBladeMotion( rigid, i, rotate );
        degen[i].Transform( rigid );
    }
}

// Moving the blades leaves the blade's and the disk's own tessellations as they were.
void PropGeom::UpdateMainTessVec()
{
    if ( m_BladeTessDirty )
    {
        if ( m_PropMode() <= PROP_MODE::PROP_BOTH )
        {
            UpdateTess( m_BladeSurf, m_BladeCapUMinSuccess, m_BladeCapUMaxSuccess, m_BladeTess, m_BladeFeatureTess );
        }
        if ( m_PropMode() >= PROP_MODE::PROP_BOTH )
        {
            UpdateTess( m_DiskSurf, false, false, m_DiskTess, m_DiskFeatureTess );
        }
        m_BladeTessDirty = false;
    }

    GetMainTessVecAtRotate( m_Rotate(), m_MainTessVec, m_MainFeatureTessVec );
}

// Moving the blades leaves the blade's own preview as it was.
void PropGeom::UpdateMainDegenGeomPreview()
{
    if ( m_PropMode() <= PROP_MODE::PROP_BOTH && m_BladeDegenDirty )
    {
        m_BladeDegenGeom = DegenGeom();
        CreateDegenGeom( m_BladeSurf, 0, m_BladeDegenGeom, true, 1 );
        m_BladeDegenDirty = false;
    }

    GetMainDegenGeomPreviewAtRotate( m_Rotate(), m_MainDegenGeomPreviewVec );
}

void PropGeom::UpdateBladeAzimuth()
{
    for ( int i = 1; i < m_Nblade(); i++ )
    {
        m_BladeAzimuthParmVec[ i - 1 ]->Activate();
        m_BladeDeltaAzimuthParmVec[ i - 1 ]->Activate();
    }
    m_BladeAzimuthDeltaFlag.Activate();

    if ( m_BladeAzimuthMode() == PROP_AZI_UNIFORM )
    {
        for ( int i = 1; i < m_Nblade(); i++ )
        {
            double theta_nom = 360.0 * i / ( double )m_Nblade();

            m_BladeAzimuthParmVec[ i - 1 ]->Set( theta_nom );
            m_BladeDeltaAzimuthParmVec[ i - 1 ]->Set( 0.0 );

            m_BladeAzimuthParmVec[ i - 1 ]->Deactivate();
            m_BladeDeltaAzimuthParmVec[ i - 1 ]->Deactivate();
        }
        m_BladeAzimuthDeltaFlag.Deactivate();
    }
    else if ( m_BladeAzimuthMode() == PROP_AZI_FREE )
    {
        for ( int i = 1; i < m_Nblade(); i++ )
        {
            double theta_nom = 360.0 * i / ( double )m_Nblade();

            if ( !m_BladeAzimuthDeltaFlag() )
            {
                m_BladeDeltaAzimuthParmVec[ i - 1 ]->Set( m_BladeAzimuthParmVec[ i - 1 ]->Get() - theta_nom );
                m_BladeDeltaAzimuthParmVec[ i - 1 ]->Deactivate();
            }
            else
            {
                m_BladeAzimuthParmVec[ i - 1 ]->Set( theta_nom + m_BladeDeltaAzimuthParmVec[ i - 1 ]->Get() );
                m_BladeAzimuthParmVec[ i - 1 ]->Deactivate();
            }
        }
    }
    else // PROP_AZI_BALANCED
    {
        if ( m_Nblade() > 3 )
        {
            vector < double > thetavec( m_Nblade(), 0.0 );

            for ( int i = 1; i < m_Nblade(); i++ )
            {
                double theta_nom = 360.0 * i / ( double )m_Nblade();

                if ( !m_BladeAzimuthDeltaFlag() )
                {
                    m_BladeDeltaAzimuthParmVec[ i - 1 ]->Set( m_BladeAzimuthParmVec[ i - 1 ]->Get() - theta_nom );
                    m_BladeDeltaAzimuthParmVec[ i - 1 ]->Deactivate();
                }
                else
                {
                    m_BladeAzimuthParmVec[ i - 1 ]->Set( theta_nom + m_BladeDeltaAzimuthParmVec[ i - 1 ]->Get() );
                    m_BladeAzimuthParmVec[ i - 1 ]->Deactivate();
                }
                thetavec[i] = m_BladeAzimuthParmVec[ i - 1 ]->Get() * M_PI / 180.0;
            }

            BalanceBladesAnalytic( thetavec );


            for ( int i = m_Nblade() - 2; i < m_Nblade(); i++ )
            {
                double theta_nom = 360.0 * i / ( double )m_Nblade();

                m_BladeAzimuthParmVec[ i - 1 ]->Set( thetavec[i] * 180.0 / M_PI );
                m_BladeDeltaAzimuthParmVec[ i - 1 ]->Set( m_BladeAzimuthParmVec[ i - 1 ]->Get() - theta_nom );

                m_BladeDeltaAzimuthParmVec[ i - 1 ]->Deactivate();
                m_BladeAzimuthParmVec[ i - 1 ]->Deactivate();
            }
        }
        else // Uniorm spacing for 3 or less blades.
        {
            for ( int i = 1; i < m_Nblade(); i++ )
            {
                double theta_nom = 360.0 * i / ( double )m_Nblade();

                m_BladeAzimuthParmVec[ i - 1 ]->Set( theta_nom );
                m_BladeDeltaAzimuthParmVec[ i - 1 ]->Set( 0.0 );

                m_BladeAzimuthParmVec[ i - 1 ]->Deactivate();
                m_BladeDeltaAzimuthParmVec[ i - 1 ]->Deactivate();
            }
            m_BladeAzimuthDeltaFlag.Deactivate();
        }
    }
}

struct balance_g_gp_functor
{
    double s, c;

    typedef typename Eigen::Matrix < double, 2, 1 > vec;
    typedef typename Eigen::Matrix < double, 2, 2 > mat;

    void operator()( vec &g, mat &gp, const vec &theta ) const
    {
        double s0 = sin( theta[0] );
        double s1 = sin( theta[1] );
        double c0 = cos( theta[0] );
        double c1 = cos( theta[1] );

        g(0) = s + s0 + s1;
        g(1) = c + c0 + c1;

        gp(0,0) = c0; // d g0 / d0
        gp(0,1) = c1; // d g0 / d1

        gp(1,0) = -s0; // d g1 / d0
        gp(1,1) = -s1; // d g1 / d1
    }
};


void PropGeom::BalanceBlades(vector < double > &thetavec)
{
    typedef eli::mutil::nls::newton_raphson_system_method < double, 2, 1 > nonlinear_solver_type;
    nonlinear_solver_type nrm;
    balance_g_gp_functor fun;
    typedef typename Eigen::Matrix < double, 2, 1 > vec;
    vec theta0, theta;
    vec f0;


    double s = 0;
    double c = 0;
    int n = thetavec.size();

    for ( int i = 0; i < n - 2; i++ )
    {
        s += sin( thetavec[i] );
        c += cos( thetavec[i] );
    }

    fun.s = s;
    fun.c = c;

    theta0[ 0 ] = thetavec[ n - 2 ];
    theta0[ 1 ] = thetavec[ n - 1 ];

    f0[0] = 0;
    f0[1] = 0;

    nrm.set_initial_guess( theta0 );

    nrm.find_root( theta, fun, f0 );

    thetavec[ n - 2 ] = theta[ 0 ];
    thetavec[ n - 1 ] = theta[ 1 ];
}

void PropGeom::BalanceBladesAnalytic( vector < double > & thetavec )
{
    double s = 0;
    double c = 0;
    int n = thetavec.size();

    for ( int i = 0; i < n - 2; i++ )
    {
        s += sin( thetavec[i] );
        c += cos( thetavec[i] );
    }

    const double r = std::sqrt( c * c + s * s );

    // Existence condition
    if ( r > 2.0 )
    {
        printf( "Static balance impossible: r > 2.0\n" );
    }
    else
    {
        // Closed-form solution
        const double phi = atan2( -s, -c );
        const double delta = acos( r / 2.0 );

        double theta0 = phi + delta;
        double theta1 = phi - delta;

        // Enforce [ 0, 2 * pi )
        theta0 = clampCyclic( theta0, 0.0, 2.0 * M_PI );
        theta1 = clampCyclic( theta1, 0.0, 2.0 * M_PI );

        // Enforce increasing order
        if ( theta1 < theta0 )
        {
            std::swap( theta0, theta1 );
        }

        thetavec[ n - 2 ] = theta0;
        thetavec[ n - 1 ] = theta1;
    }
}

// Notional code to allow cancellation of higher order modes by adjusting blades.  This requires a nonlinear solve
// of a matrix whose size will only be known at runtime.  The nrm solver in Code-Eli is only set up to be sized
// at compile time.
// struct fourier_balance_functor
// {
//     int M;                      // number of modes
//     Eigen::VectorXd theta_fixed;
//
//     typedef Eigen::VectorXd vec;
//     typedef Eigen::MatrixXd mat;
//
//     void operator()( vec& g, mat& gp, const vec& theta_adj ) const
//     {
//         const int Na = theta_adj.size();
//
//         g.setZero( 2 * M );
//         gp.setZero( 2 * M, Na );
//
//         for ( int m = 1; m <= M; ++m )
//         {
//             double Re = 0.0;
//             double Im = 0.0;
//
//             // Fixed blades
//             for ( int k = 0; k < theta_fixed.size(); ++k )
//             {
//                 Re += cos( m * theta_fixed[k] );
//                 Im += sin( m * theta_fixed[k] );
//             }
//
//             // Adjustable blades
//             for ( int j = 0; j < Na; ++j )
//             {
//                 const double mj = m * theta_adj[ j ];
//                 Re += cos( mj );
//                 Im += sin( mj );
//
//                 // Jacobian contributions
//                 gp( 2 * m - 2, j ) = -m * sin( mj );  // d Re / d theta_j
//                 gp( 2 * m - 1, j ) =  m * cos( mj );  // d Im / d theta_j
//             }
//
//             g( 2 * m - 2 ) = Re;
//             g( 2 * m - 1 ) = Im;
//         }
//     }
// };
//
// void PropGeom::BalanceBladesFourier( std::vector<double>& thetavec, const std::vector<int>& fixed_idx, int M )
// {
//     typedef Eigen::VectorXd vec;
//     typedef Eigen::MatrixXd mat;
//
//     const int N = static_cast < int > ( thetavec.size() );
//
//     std::vector< int > adj_idx;
//     for ( int i = 0; i < N; ++i )
//     {
//         if ( std::find( fixed_idx.begin(), fixed_idx.end(), i) == fixed_idx.end() )
//         {
//             adj_idx.push_back( i );
//         }
//     }
//
//     const int Na = static_cast< int > ( adj_idx.size() );
//
//     if ( Na < 2 * M )
//     {
//         printf( "Insufficient DOF to cancel requested modes\n" );
//     }
//
//     // Build fixed-angle vector
//     vec theta_fixed( fixed_idx.size() );
//     for ( int i = 0; i < fixed_idx.size(); ++i )
//     {
//         theta_fixed[i] = thetavec[ fixed_idx[ i ] ];
//     }
//
//     // Initial guess for adjustable angles
//     vec theta0( Na );
//     for ( int i = 0; i < Na; ++i )
//     {
//         theta0[i] = thetavec[ adj_idx[ i ] ];
//     }
//
//     // Functor
//     fourier_balance_functor fun;
//     fun.M = M;
//     fun.theta_fixed = theta_fixed;
//
//     // Newton–Raphson solver
//     // Need to dynamically size the solver here, but the solver is not written to allow dynamic problem sizing.
//     typedef eli::mutil::nls::newton_raphson_system_method< double, Eigen::Dynamic, 1 > nonlinear_solver_type;
//
//     nonlinear_solver_type nrm;
//     nrm.set_initial_guess( theta0 );
//
//     vec theta_adj;
//     vec f0 = vec::Zero( 2 * M );
//
//     nrm.find_root( theta_adj, fun, f0 );
//
//     for (int i = 0; i < Na; ++i)
//     {
//         thetavec[ adj_idx[ i ] ] = clampCyclic( theta_adj[i], 0.0, 2.0 * M_PI );
//     }
//     std::sort( thetavec.begin(), thetavec.end() );
// }

void PropGeom::CheckBalance()
{
    int n = m_Nblade();

    double rev = 1.0;
    if ( m_ReverseFlag() )
    {
        rev = -1.0;
    }

    // Initialize with blade zero values.
    double s = 0.0;
    double c = 1.0;
    // Add rest of blades.
    for ( int i = 1; i < n; i++ )
    {
        double th = -rev * m_BladeAzimuthParmVec[ i - 1 ]->Get() * M_PI / 180.0;
        s += sin( th );
        c += cos( th );
    }

    m_BalanceX1.Set( c / n );
    m_BalanceX2.Set( s / n );
}

// Blade iblade's own value with individual on, counting blades from zero; master otherwise.
static double GetBladeValue( const vector< Parm* > & vec, bool individual, int iblade, double master )
{
    if ( individual && iblade > 0 && iblade <= ( int )vec.size() && vec[ iblade - 1 ] )
    {
        return vec[ iblade - 1 ]->Get();
    }
    return master;
}

// mean plus the harmonics at azimuth psi, degrees: cos_n cos( n psi ) + sin_n sin( n psi ).
static double HarmonicValue( double mean, const vector< Parm* > & cosvec, const vector< Parm* > & sinvec, double psi )
{
    double val = mean;
    double p = psi * M_PI / 180.0;
    for ( int n = 1; n <= ( int )cosvec.size() && n <= ( int )sinvec.size(); n++ )
    {
        val += cosvec[ n - 1 ]->Get() * cos( n * p ) + sinvec[ n - 1 ]->Get() * sin( n * p );
    }
    return val;
}

// Blade iblade's azimuth psi with the propeller turned to rotate: rotate plus the blade's azimuth, in
// the direction of rotation.  PsiRef turns psi's zero with the blades.
double PropGeom::GetBladePsi( int iblade, double rotate )
{
    double psi = rotate;
    if ( iblade > 0 && iblade <= ( int )m_BladeAzimuthParmVec.size() )
    {
        psi += m_BladeAzimuthParmVec[ iblade - 1 ]->Get();
    }
    return psi;
}

// Set the angles a mode makes outputs: in harmonic mode blade 1's and the other blades', from the
// harmonics at Rotate, greyed; in collective mode the other blades', to blade 1's.  The harmonic parms
// are greyed in the other modes.
void PropGeom::UpdateBladeMotion( int mode, Parm & angle, vector< Parm* > & bladevec, Parm & mean,
                                  vector< Parm* > & cosvec, vector< Parm* > & sinvec )
{
    bool harmonic = ( mode == vsp::PROP_BLADE_HARMONIC );
    if ( harmonic )
    {
        angle.Set( HarmonicValue( mean(), cosvec, sinvec, GetBladePsi( 0, m_Rotate() ) ) );
    }

    for ( int i = 0; i < ( int )bladevec.size(); i++ )
    {
        if ( harmonic )
        {
            bladevec[i]->Set( HarmonicValue( mean(), cosvec, sinvec, GetBladePsi( i + 1, m_Rotate() ) ) );
        }
        else if ( mode == vsp::PROP_BLADE_COLLECTIVE )
        {
            bladevec[i]->Set( angle() );
        }
    }

    vector< Parm* > outputs = bladevec;
    outputs.push_back( &angle );

    vector< Parm* > inputs = cosvec;
    inputs.insert( inputs.end(), sinvec.begin(), sinvec.end() );
    inputs.push_back( &mean );

    for ( int i = 0; i < ( int )outputs.size(); i++ )
    {
        if ( harmonic )
        {
            outputs[i]->Deactivate();
        }
        else
        {
            outputs[i]->Activate();
        }
    }

    for ( int i = 0; i < ( int )inputs.size(); i++ )
    {
        if ( harmonic )
        {
            inputs[i]->Activate();
        }
        else
        {
            inputs[i]->Deactivate();
        }
    }
}

double PropGeom::GetBladeFlapAngle( int iblade, double rotate )
{
    if ( m_BladeMotionMode() == vsp::PROP_BLADE_HARMONIC )
    {
        return HarmonicValue( m_FlapMean(), m_FlapCosParmVec, m_FlapSinParmVec, GetBladePsi( iblade, rotate ) );
    }
    return GetBladeValue( m_FlapAngleParmVec, m_BladeMotionMode() == vsp::PROP_BLADE_INDIVIDUAL, iblade, m_FlapAngle() );
}

double PropGeom::GetBladeLagAngle( int iblade, double rotate )
{
    if ( m_BladeMotionMode() == vsp::PROP_BLADE_HARMONIC )
    {
        return HarmonicValue( m_LagMean(), m_LagCosParmVec, m_LagSinParmVec, GetBladePsi( iblade, rotate ) );
    }
    return GetBladeValue( m_LagAngleParmVec, m_BladeMotionMode() == vsp::PROP_BLADE_INDIVIDUAL, iblade, m_LagAngle() );
}

double PropGeom::GetBladeFoldAngle( int iblade )
{
    return GetBladeValue( m_FoldAngleParmVec, m_IndividualBladeFoldFlag(), iblade, m_FoldAngle() );
}

// The feather's mode in use: collective while Beta34 drives, whatever the mode is set to.
int PropGeom::FeatherMode()
{
    if ( m_UseBeta34Flag() )
    {
        return vsp::PROP_BLADE_COLLECTIVE;
    }
    return m_BladeMotionMode();
}

double PropGeom::GetBladeFeather( int iblade, double rotate )
{
    if ( FeatherMode() == vsp::PROP_BLADE_HARMONIC )
    {
        return HarmonicValue( m_FeatherMean(), m_FeatherCosParmVec, m_FeatherSinParmVec, GetBladePsi( iblade, rotate ) );
    }
    return GetBladeValue( m_FeatherParmVec, FeatherMode() == vsp::PROP_BLADE_INDIVIDUAL, iblade, m_Feather() );
}

// The blade's azimuth, the propeller rotation from PsiRef and the precone: the motion inboard of the hinges.
void PropGeom::BladeRootMotion( Matrix4d & mat, int iblade, double rotate )
{
    double rev = 1.0;
    if ( m_ReverseFlag() )
    {
        rev = -1.0;
    }

    mat.loadIdentity();

    if ( iblade > 0 && iblade <= ( int )m_BladeAzimuthParmVec.size() )
    {
        mat.rotateX( -rev * m_BladeAzimuthParmVec[ iblade - 1 ]->Get() );
    }

    mat.rotateX( -rev * m_PsiRef() );
    mat.rotateX( -rev * rotate );

    mat.rotateZ( m_Precone() );
}

// The blade's hinges, hub to tip by radial place; hinges at one place go in the order of their types.
vector< BladeHinge > PropGeom::GetBladeHinges( int iblade, double rotate )
{
    vector< BladeHinge > hinges;
    hinges.push_back( { m_RadFlapAxis(), FLAP_HINGE, m_FlapAxOrigin, m_FlapAxDirection, GetBladeFlapAngle( iblade, rotate ) } );
    hinges.push_back( { m_RadLagAxis(), LAG_HINGE, m_LagAxOrigin, m_LagAxDirection, GetBladeLagAngle( iblade, rotate ) } );
    hinges.push_back( { m_RadFoldAxis(), FOLD_HINGE, m_FoldAxOrigin, m_FoldAxDirection, GetBladeFoldAngle( iblade ) } );
    hinges.push_back( { m_RadFeatherAxis(), FEATHER_HINGE, m_FeatherAxOrigin, m_FeatherAxDirection, GetBladeFeather( iblade, rotate ) } );

    std::sort( hinges.begin(), hinges.end(), []( const BladeHinge & a, const BladeHinge & b )
    {
        if ( a.m_Radius != b.m_Radius )
        {
            return a.m_Radius < b.m_Radius;
        }
        return a.m_Type < b.m_Type;
    } );

    return hinges;
}

Matrix4d PropGeom::GetBladePlacement( int iblade, double rotate )
{
    Matrix4d mat;
    RigidBladeMotion( mat, iblade, rotate );
    return mat;
}

void PropGeom::RigidBladeMotion( Matrix4d & mat, int iblade, double rotate )
{
    BladeRootMotion( mat, iblade, rotate );

    vector< BladeHinge > hinges = GetBladeHinges( iblade, rotate );
    for ( int i = 0; i < ( int )hinges.size(); i++ )
    {
        mat.rotateAbout( hinges[i].m_Angle, hinges[i].m_Axis, hinges[i].m_Origin );
    }
}

void PropGeom::ReserveBlades( int n ) // Note:  n is m_Nblade() - 1
{
    if ( n == m_FoldAngleParmVec.size() )
    {
        return;
    }

    if ( n < m_FoldAngleParmVec.size() )
    {
        TrimParmVec( m_FlapAngleParmVec, n );
        TrimParmVec( m_LagAngleParmVec, n );
        TrimParmVec( m_FoldAngleParmVec, n );
        TrimParmVec( m_FeatherParmVec, n );
        TrimParmVec( m_BladeAzimuthParmVec, n );
        TrimParmVec( m_BladeDeltaAzimuthParmVec, n );
    }
    else
    {
        while ( m_FoldAngleParmVec.size() < n )
        {
            AddBlade();
        }
    }

    bool flag_before = m_BladeAzimuthDeltaFlag();
    m_BladeAzimuthDeltaFlag.Set( true );
    UpdateBladeAzimuth();
    m_BladeAzimuthDeltaFlag.Set( flag_before );
}

// Add the parm name_num, in group, to vec.
void PropGeom::AddBladeParm( vector< Parm* > & vec, const string & name, const string & group, int num, double val, double lower, double upper, const string & descript )
{
    Parm *p = ParmMgr.CreateParm( vsp::PARM_DOUBLE_TYPE );
    if ( p )
    {
        char str[255];
        snprintf( str, sizeof( str ), "%s_%d", name.c_str(), num );
        p->Init( string( str ), group, this, val, lower, upper );
        p->SetDescript( descript );
        vec.push_back( p );
    }
}

// The coefficient parms each harmonic count asks for.
void PropGeom::ReserveHarmonics()
{
    ReserveHarmonicParms( m_FlapCosParmVec, m_FlapSinParmVec, "Flap", "flap angle", m_NumHarmonics() );
    ReserveHarmonicParms( m_LagCosParmVec, m_LagSinParmVec, "Lag", "lag angle", m_NumHarmonics() );
    ReserveHarmonicParms( m_FeatherCosParmVec, m_FeatherSinParmVec, "Feather", "feather angle", m_NumHarmonics() );
}

// Make or delete the parms nameCos_k and nameSin_k so there are n harmonics.
void PropGeom::ReserveHarmonicParms( vector< Parm* > & cosvec, vector< Parm* > & sinvec, const string & name,
                                     const string & angle, int n )
{
    TrimParmVec( cosvec, n );
    TrimParmVec( sinvec, n );
    while ( ( int )cosvec.size() < n )
    {
        int k = cosvec.size() + 1;
        char str[255];
        snprintf( str, sizeof( str ), " of harmonic %d of the %s", k, angle.c_str() );
        AddBladeParm( cosvec, name + "Cos", "Articulation", k, 0.0, -180.0, 180.0, string( "Cosine coefficient" ) + str );
        AddBladeParm( sinvec, name + "Sin", "Articulation", k, 0.0, -180.0, 180.0, string( "Sine coefficient" ) + str );
        if ( ( int )cosvec.size() < k || ( int )sinvec.size() < k )
        {
            break;
        }
    }
}

void PropGeom::AddBlade()
{
    int i = m_FoldAngleParmVec.size();

    // The parms of blade i + 2, named _( i + 1 ) except for the azimuths.
    char str[255];
    snprintf( str, sizeof( str ), " of blade %d", i + 2 );
    string blade( str );

    AddBladeParm( m_FlapAngleParmVec, "FlapAngle", "Articulation", i + 1, m_FlapAngle(), -1.0e12, 1.0e12, "Flap angle" + blade );
    AddBladeParm( m_LagAngleParmVec, "LagAngle", "Articulation", i + 1, m_LagAngle(), -1.0e12, 1.0e12, "Lag angle" + blade );
    AddBladeParm( m_FoldAngleParmVec, "FoldAngle", "Design", i + 1, m_FoldAngle(), -180, 180, "Fold angle" + blade );
    AddBladeParm( m_FeatherParmVec, "Feather", "Articulation", i + 1, m_Feather(), -1.0e12, 1.0e12, "Feather angle" + blade );
    AddBladeParm( m_BladeAzimuthParmVec, "BladeAz", "Design", i + 2, 360.0 * i / ( double )m_Nblade(), 0, 360, "Propeller blade azimuth" );
    AddBladeParm( m_BladeDeltaAzimuthParmVec, "DeltaAz", "Design", i + 2, 0, -180, 180, "Propeller blade azimuth delta" );
}

void PropGeom::CalculateMeshMetrics()
{
    // The blade, or the disk in disk-only mode.
    const VspSurf * surf = &m_BladeSurf;
    if ( m_PropMode() == PROP_MODE::PROP_DISK )
    {
        surf = &m_DiskSurf;
    }

    std::vector<double> vcheck( 8 );

    double vmin, vmax, vle, vlelow, vleup, vtruemax;

    vmin = 0.0;
    vmax = surf->GetWMax();
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

            double d = dist( surf->CompPnt( u, v1 ), surf->CompPnt( u, v2 ) );
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
    xmlNodePtr propeller_node = xmlNewChild( node, nullptr, BAD_CAST "PropellerGeom", nullptr );
    if ( propeller_node )
    {
        // m_VecViz.EncodeXml( propeller_node );
        // m_VecVizDia.EncodeXml( propeller_node );

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
    xmlNodePtr child_node = XmlUtil::GetNode( node, "ParmContainer", 0 );
    if ( child_node )
    {
        xmlNodePtr gnode = XmlUtil::GetNode( child_node, "Design", 0 );
        if ( gnode )
        {
            xmlNodePtr n = XmlUtil::GetNode( gnode, "NumBlade", 0 );
            if ( n )
            {
                double nblade = XmlUtil::FindDoubleProp( n, "Value", m_Nblade() );
                m_Nblade.Set( nblade );
                ReserveBlades( (int) nblade - 1 );
            }
        }

        // The harmonic count, so the coefficient parms exist to be read.
        gnode = XmlUtil::GetNode( child_node, "Articulation", 0 );
        if ( gnode )
        {
            xmlNodePtr n = XmlUtil::GetNode( gnode, "NumHarmonics", 0 );
            if ( n )
            {
                m_NumHarmonics.Set( XmlUtil::FindDoubleProp( n, "Value", m_NumHarmonics() ) );
            }
        }
        ReserveHarmonics();
    }

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
    m_SurfDirty = true;
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
void PropGeom::ApplyScale( double currentScale )
{
    m_Diameter *= currentScale;
    for ( int i = 0 ; i < m_XSecSurf.NumXSec() ; i++ )
    {
        XSec* xs = m_XSecSurf.FindXSec( i );
        if ( xs )
        {
            xs->SetScale( currentScale );
        }
    }

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

void PropGeom::GetUWTess( const VspSurf &surf, bool capUMinSuccess, bool capUMaxSuccess, bool degen, vector< double > &utess, vector< double > &vtess, const int & n_ref ) const
{
    vector < int > tessvec;
    vector < double > rootc;
    vector < double > tipc;
    vector < int > umerge;

    int nmerge = surf.GetNumSectU();

    if (m_CapUMinOption()!=NO_END_CAP && capUMinSuccess )
    {
        tessvec.push_back( m_CapUMinTess() );
        rootc.push_back( 1.0 );
        tipc.push_back( 1.0 );

        if ( m_CapUMinOption() <= POINT_END_CAP )
        {
            umerge.push_back( 1 );
        }
        else
        {
            umerge.push_back( 2 );
            nmerge--;
        }
        nmerge--;
    }

    if (m_CapUMaxOption()!=NO_END_CAP && capUMaxSuccess )
    {
        nmerge--;

        if ( m_CapUMaxOption() >= ROUND_EXT_END_CAP_NONE )
        {
            nmerge--;
        }
    }

    tessvec.push_back( m_TessU() );
    rootc.push_back( m_RootCluster() );
    tipc.push_back( m_TipCluster() );
    umerge.push_back( nmerge );

    if (m_CapUMaxOption()!=NO_END_CAP && capUMaxSuccess )
    {
        tessvec.push_back( m_CapUMinTess() );
        rootc.push_back( 1.0 );
        tipc.push_back( 1.0 );

        if ( m_CapUMaxOption() <= POINT_END_CAP )
        {
            umerge.push_back( 1 );
        }
        else
        {
            umerge.push_back( 2 );
        }
    }

    surf.SetRootTipClustering( rootc, tipc );
    surf.GetUWTess( utess, vtess, tessvec, m_TessW(), m_CapUMinTess(), m_TessU(), degen, umerge, n_ref );
}

void PropGeom::UpdateTesselate( const VspSurf &surf, bool capUMinSuccess, bool capUMaxSuccess, bool degen, vector< vector< vec3d > > &pnts, vector< vector< vec3d > > &norms, vector< vector< vec3d > > &uw_pnts, const int & n_ref ) const
{
    vector < int > tessvec;
    vector < double > rootc;
    vector < double > tipc;
    vector < int > umerge;

    int nmerge = surf.GetNumSectU();

    if (m_CapUMinOption()!=NO_END_CAP && capUMinSuccess )
    {
        tessvec.push_back( m_CapUMinTess() );
        rootc.push_back( 1.0 );
        tipc.push_back( 1.0 );

        if ( m_CapUMinOption() <= POINT_END_CAP )
        {
            umerge.push_back( 1 );
        }
        else
        {
            umerge.push_back( 2 );
            nmerge--;
        }
        nmerge--;
    }

    if (m_CapUMaxOption()!=NO_END_CAP && capUMaxSuccess )
    {
        nmerge--;

        if ( m_CapUMaxOption() >= ROUND_EXT_END_CAP_NONE )
        {
            nmerge--;
        }
    }

    tessvec.push_back( m_TessU() );
    rootc.push_back( m_RootCluster() );
    tipc.push_back( m_TipCluster() );
    umerge.push_back( nmerge );

    if (m_CapUMaxOption()!=NO_END_CAP && capUMaxSuccess )
    {
        tessvec.push_back( m_CapUMinTess() );
        rootc.push_back( 1.0 );
        tipc.push_back( 1.0 );

        if ( m_CapUMaxOption() <= POINT_END_CAP )
        {
            umerge.push_back( 1 );
        }
        else
        {
            umerge.push_back( 2 );
        }
    }

    surf.SetRootTipClustering( rootc, tipc );
    surf.Tesselate( tessvec, m_TessW(), pnts, norms, uw_pnts, m_CapUMinTess(), m_TessU(), degen, umerge, n_ref );
}

void PropGeom::UpdateSplitTesselate( const VspSurf &surf, bool
                                     capUMinSuccess, bool capUMaxSuccess, vector< vector< vector< vec3d > > > &pnts, vector< vector< vector< vec3d > > > &norms ) const
{
    vector < int > tessvec;
    vector < double > rootc;
    vector < double > tipc;
    vector < int > umerge;

    int nmerge = surf.GetNumSectU();

    if (m_CapUMinOption()!=NO_END_CAP && capUMinSuccess )
    {
        tessvec.push_back( m_CapUMinTess() );
        rootc.push_back( 1.0 );
        tipc.push_back( 1.0 );

        if ( m_CapUMinOption() <= POINT_END_CAP )
        {
            umerge.push_back( 1 );
        }
        else
        {
            umerge.push_back( 2 );
            nmerge--;
        }
        nmerge--;
    }

    if (m_CapUMaxOption()!=NO_END_CAP && capUMaxSuccess )
    {
        nmerge--;

        if ( m_CapUMaxOption() >= ROUND_EXT_END_CAP_NONE )
        {
            nmerge--;
        }
    }

    tessvec.push_back( m_TessU() );
    rootc.push_back( m_RootCluster() );
    tipc.push_back( m_TipCluster() );
    umerge.push_back( nmerge );

    if (m_CapUMaxOption()!=NO_END_CAP && capUMaxSuccess )
    {
        tessvec.push_back( m_CapUMinTess() );
        rootc.push_back( 1.0 );
        tipc.push_back( 1.0 );

        if ( m_CapUMaxOption() <= POINT_END_CAP )
        {
            umerge.push_back( 1 );
        }
        else
        {
            umerge.push_back( 2 );
        }
    }

    surf.SetRootTipClustering( rootc, tipc );
    surf.SplitTesselate( tessvec, m_TessW(), pnts, norms, m_CapUMinTess(), m_TessU(), umerge );
}

void PropGeom::UpdatePreTess()
{
    // The surface or its tessellation changed, so the blade is tessellated again.
    m_BladeTessDirty = true;
    m_BladeDegenDirty = true;

    // Update clustering before symmetry is applied for m_SurfVec
    m_FoilSurf.SetClustering( m_LECluster(), m_TECluster() );
    m_BladeSurf.SetClustering( m_LECluster(), m_TECluster() );
    m_DiskSurf.SetClustering( m_LECluster(), m_TECluster() );

    int nsurf = GetNumMainSurfs();
    for ( int i = 0; i < nsurf; i++ )
    {
        m_MainSurfVec[i].SetClustering( m_LECluster(), m_TECluster() );
    }

    CalculateMeshMetrics();
}

string PropGeom::BuildBEMResults()
{
    // Calculate prop center and normal vector, where the shape is
    Matrix4d shape_mat = GetShapeMatrix();
    vec3d cen = shape_mat.xform( vec3d( 0, 0, 0 ) );
    vec3d norm = shape_mat.xform( vec3d( -1.0, 0, 0 ) ) - cen;

    int n = m_TessU();

    //==== Create Results ====//
    Results* res = ResultsMgr.CreateResults( "PropBEM", "Propeller BEM representation results." );
    res->Add( new NameValData( "Num_Sections", n, "Number of defining sections." ) );
    res->Add( new NameValData( "Num_Blade", m_Nblade(), "Number of propeller blades." ) );
    res->Add( new NameValData( "Diameter", m_Diameter(), "Propeller diameter." ) );
    res->Add( new NameValData( "Beta34", m_Beta34(), "Blade angle measured at 0.75R." ) );
    res->Add( new NameValData( "Feather", m_Feather(), "Rotation of blade about feather axis." ) );
    res->Add( new NameValData( "Pre_Cone", m_Precone(), "Propeller pre-cone angle." ) );
    res->Add( new NameValData( "Center", cen, "Center of rotation vector." ) );
    res->Add( new NameValData( "Normal", norm, "Axis of rotation vector." ) );

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
        // The foil surface's u runs linearly from the root to the tip, so the section at radius r is at u.
        double t = static_cast < double > ( i ) / ( n - 1 );
        double u = Cluster( t, m_RootCluster(), m_TipCluster() );
        double r = rfirst + rspan * u;

        VspCurve c;
        m_FoilSurf.GetUConstCurve( c, u );
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
        snprintf( str, sizeof( str ),  "%03d", i );
        res->Add( new NameValData( "XSection_" + string( str ), xpts, "X coordinates of airfoil section." ) );
        res->Add( new NameValData( "YSection_" + string( str ), ypts, "Y coordinates of airfoil section." ) );

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

    res->Add( new NameValData( "Radius", r_vec, "Vector of radii where prop data is provided." ) );
    res->Add( new NameValData( "Chord", chord_vec, "Blade chord as c/R." ) );
    res->Add( new NameValData( "Twist", twist_vec, "Blade twist angle." ) );
    res->Add( new NameValData( "Rake", rake_vec, "Offset perpendicular to local chord as Rake/R." ) );
    res->Add( new NameValData( "Skew", skew_vec, "Offset parallel to local chord as Skew/R." ) );
    res->Add( new NameValData( "Sweep", sweep_vec, "Blade sweep angle." ) );
    res->Add( new NameValData( "Thick", thick_vec, "Airfoil section t/c." ) );
    res->Add( new NameValData( "CLi", cli_vec, "Airfoil ideal (design) lift coefficient." ) );
    res->Add( new NameValData( "Axial", axial_vec, "Offset in direction of prop axis as Axial/R." ) );
    res->Add( new NameValData( "Tangential", tangential_vec, "Offset tangent to prop rotation as Tangential/R" ) );

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
    return nullptr;
}

void PropGeom::WriteAirfoilFiles( FILE* meta_fid, const string &name, const string &id )
{
    // This function writes out the coordinate/control point data for all untwisted unit length airfoils.
    //  Special considerations need to be taken for PropGeoms because the interpolated airfoils do
    //  not start and/or end at each XSec. Sectional U tessellation is not considered for PropGeoms. 

    Vehicle* veh = VehicleMgr.GetVehicle();
    if ( !veh || !meta_fid || m_MainSurfVec.size() == 0 )
    {
        return;
    }

    // File names come from name and id, so a Clone names the files after itself.
    string geom_name = name;
    string geom_id = id;
    if ( geom_name.empty() )
    {
        geom_name = m_Name;
    }
    if ( geom_id.empty() )
    {
        geom_id = m_ID;
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
        string af_file_name = geom_name + "_";

        if ( veh->m_AFAppendGeomIDFlag() )
        {
            af_file_name += ( geom_id + "_" );
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
        fprintf( meta_fid, "Geom Name, %s\n", geom_name.c_str() );
        fprintf( meta_fid, "Geom ID, %s\n", geom_id.c_str() );
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

// The innermost cross section is the hub.
bool PropGeom::GetRotorHubDiameter( double &hubdia ) const
{
    XSecSurf* xsecsurf = const_cast< PropGeom* >( this )->GetXSecSurf( 0 );
    if ( !xsecsurf )
    {
        return false;
    }

    XSec* xsec = xsecsurf->FindXSec( 0 );
    if ( !xsec || xsec->GetType() != vsp::XSEC_PROP )
    {
        return false;
    }

    PropXSec* prop_xsec = dynamic_cast< PropXSec* >( xsec );
    if ( !prop_xsec )
    {
        return false;
    }

    hubdia = 2.0 * prop_xsec->m_RadiusFrac.GetResult();  // radius to diameter
    return true;
}

// A blade's chord curve is c/R over r/R, so its MAC is R integral( c^2 ) / integral( c ) from the root
double PropGeom::ComputeMAC()
{
    double rfirst = GetR0();

    Vsp1DCurve csq;
    csq.product( *m_ChordCurve.GetCurve(), *m_ChordCurve.GetCurve() );

    double cint = m_ChordCurve.IntegrateCrv( rfirst );
    if ( cint <= 0 )
    {
        return 0.0;
    }
    return 0.5 * m_Diameter() * csq.IntegrateCrv( rfirst ) / cint;
}

double PropGeom::GetR0()
{
    if (  PropXSec* xs = ( PropXSec* ) m_XSecSurf.FindXSec( 0 ) )
    {
        return xs->m_RadiusFrac();
    }
    return 0.0;
}

// For a propeller, intrepret 'Eta' as r/R
double PropGeom::UtoEta( const double &u, bool ignoreCap )
{
    double rfirst = GetR0();

    double t = u;

    int indx = 0;
    if ( !ignoreCap && m_CapUMinOption() != NO_END_CAP && m_CapUMinSuccess.size() > 0 && m_CapUMinSuccess[ m_MainSurfIndxVec[indx] ] )
    {
        t = t - 1.0;
    }

    double rt = rfirst + t * ( 1.0 - rfirst );

    return rt;
}

double PropGeom::EtatoU( const double &eta, bool ignoreCap )
{
    double rfirst = GetR0();

    double rt = eta;

    double t = ( rt - rfirst ) / ( 1.0 - rfirst );

    double u = t;

    int indx = 0;
    if ( !ignoreCap && m_CapUMinOption() != NO_END_CAP && m_CapUMinSuccess.size() > 0 && m_CapUMinSuccess[ m_MainSurfIndxVec[indx] ] )
    {
        u = u + 1.0;
    }

    return u;
}
