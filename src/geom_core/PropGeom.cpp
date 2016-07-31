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

// Change curve type from API
// Curve splitting from API
// Other API additions?

// Allow curve control of thickness.

// v2 import

// Import / Export blade element format.

// New section types

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

    // Propeller rotation first because order is reversed.
    mat.rotateX( -m_Reverse * m_PropRot );
    mat.rotateY( m_Feather );

    mat.translatef( 0, m_Radius, 0 );

    mat.rotateY( m_Reverse * m_Twist );

    mat.rotateX( m_XRotate ); // About rake direction

    mat.translatef( m_Rake, 0, m_Reverse * m_Skew );

    mat.rotateZ( m_ZRotate ); // About chord

    m_TransformedCurve.Transform( mat );

    Matrix4d fold, foldrot;

    fold.loadIdentity();
    fold.translatef( m_FoldOrigin.x(), m_FoldOrigin.y(), m_FoldOrigin.z() );
    foldrot.rotate( m_FoldAngle * PI / 180.0, m_FoldDirection );
    fold.matMult( foldrot.data() );
    fold.translatef( -m_FoldOrigin.x(), -m_FoldOrigin.y(), -m_FoldOrigin.z() );

    m_TransformedCurve.Transform( fold );

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

    m_ReverseFlag.Init( "ReverseFlag", "Design", this, false, 0, 1 );
    m_ReverseFlag.SetDescript( "Flag to reverse propeller rotation direction" );

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

    int j;
    PropXSec* xs;

    j = 0;
    xs = ( PropXSec* ) m_XSecSurf.FindXSec( j );
    xs->SetGroupDisplaySuffix( j );
    xs->m_RadiusFrac = 0.2;

    ++j;
    xs = ( PropXSec* ) m_XSecSurf.FindXSec( j );
    xs->SetGroupDisplaySuffix( j );
    xs->m_RadiusFrac = 0.45;
    FourSeries* af = dynamic_cast< FourSeries* >( xs->GetXSecCurve() );
    if ( af )
    {
        af->m_ThickChord.Set( 0.12 );
        af->m_Camber.Set( .05 );
        af->m_CamberLoc.Set( 0.4 );
    }

    ++j;
    xs = ( PropXSec* ) m_XSecSurf.FindXSec( j );
    xs->SetGroupDisplaySuffix( j );
    xs->m_RadiusFrac = 0.7;
    af = dynamic_cast< FourSeries* >( xs->GetXSecCurve() );
    if ( af )
    {
        af->m_ThickChord.Set( 0.05 );
        af->m_Camber.Set( .05 );
        af->m_CamberLoc.Set( 0.4 );
    }

    ++j;
    xs = ( PropXSec* ) m_XSecSurf.FindXSec( j );
    xs->SetGroupDisplaySuffix( j );
    xs->m_RadiusFrac = 1.0;
    af = dynamic_cast< FourSeries* >( xs->GetXSecCurve() );
    if ( af )
    {
        af->m_ThickChord.Set( 0.03 );
        af->m_Camber.Set( .01 );
        af->m_CamberLoc.Set( 0.4 );
    }


    m_ChordCurve.SetParentContainer( GetID() );
    m_ChordCurve.SetDispNames( "r/R", "Chord/R" );
    m_ChordCurve.SetParmNames( "r", "crd" );
    m_ChordCurve.SetCurveName( "Chord" );
    m_ChordCurve.InitParms();
    m_ChordCurve.m_CurveType = PCurve::CEDIT;
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

    static const double t3[] = {0.2, 1.0};
    static const double v3[] = {0.0, 0.0};
    vector < double > tv3( t3, t3 + sizeof( t3 ) / sizeof( t3[0] ) );
    vector < double > vv3( v3, v3 + sizeof( v3 ) / sizeof( v3[0] ) );

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
    m_pcurve_vec.resize( 4 );
    m_pcurve_vec[0] = &m_ChordCurve;
    m_pcurve_vec[1] = &m_TwistCurve;
    m_pcurve_vec[2] = &m_RakeCurve;
    m_pcurve_vec[3] = &m_SkewCurve;

}

//==== Destructor ====//
PropGeom::~PropGeom()
{

}

void PropGeom::UpdateDrawObj()
{
    GeomXSec::UpdateDrawObj();

    DrawObj rotAxis;

    double rev = 1.0;
    if ( m_ReverseFlag() )
    {
        rev = -1.0;
    }

    double data[16];
    m_ModelMatrix.getMat( data );

    Matrix4d mat;
    mat.loadIdentity();
    mat.rotateX( -rev * m_Rotate() );
    mat.postMult( data );

    vec3d ptstart = mat.xform( m_FoldAxOrigin + m_FoldAxDirection );
    vec3d ptend = mat.xform( m_FoldAxOrigin - m_FoldAxDirection );

    rotAxis.m_PntVec.push_back( ptstart );
    rotAxis.m_PntVec.push_back( ptend );
    rotAxis.m_GeomChanged = true;

    m_FeatureDrawObj_vec.push_back( rotAxis );


    vec3d dir = ptend - ptstart;
    double len = dir.mag();
    dir.normalize();

    m_ArrowDO.m_PntVec.clear();
    m_ArrowDO.m_Type = DrawObj::VSP_SHADED_TRIS;
    m_ArrowDO.m_GeomID = m_ID + string( "_arrow" );
    m_ArrowDO.m_GeomChanged = true;

    for ( int i = 0; i < 4; i++ )
    {
        m_ArrowDO.m_MaterialInfo.Ambient[i] = 0.2;
        m_ArrowDO.m_MaterialInfo.Diffuse[i] = 0.1;
        m_ArrowDO.m_MaterialInfo.Specular[i] = 0.7;
        m_ArrowDO.m_MaterialInfo.Emission[i] = 0.0;
    }
    m_ArrowDO.m_MaterialInfo.Diffuse[3] = 0.5;
    m_ArrowDO.m_MaterialInfo.Shininess = 5.0;

    MakeArrowhead( ptend, dir, 0.15 * len, m_ArrowDO.m_PntVec );

    m_ArrowDO.m_NormVec.clear();
    m_ArrowDO.m_NormVec.resize( m_ArrowDO.m_PntVec.size(), vec3d() );
}

void PropGeom::LoadDrawObjs( vector< DrawObj* > & draw_obj_vec )
{
    GeomXSec::LoadDrawObjs( draw_obj_vec );

    if ( m_GuiDraw.GetDispFeatureFlag() && !m_GuiDraw.GetNoShowFlag() )
    {
        draw_obj_vec.push_back( &m_ArrowDO );
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
    return fabs( a - b ) < 0.001;
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

    // Set up fold axis & store for visualization.
    Matrix4d fold, foldrot;
    fold.loadIdentity();
    fold.rotateY( m_AzimuthFoldDir() );
    fold.rotateX( m_ElevationFoldDir() );
    m_FoldAxDirection = fold.xform( vec3d( 0, 0, 1 ) );
    m_FoldAxOrigin = vec3d( m_AxialFoldAxis() * radius, m_RadFoldAxis() * radius, m_OffsetFoldAxis() * radius );

    //==== Cross Section Curves & joint info ====//
    vector< VspCurve > crv_vec;
    crv_vec.resize( nxsec );

    //==== Update XSec Location/Rotation ====//
    for ( int i = 0 ; i < nxsec ; i++ )
    {
        PropXSec* xs = ( PropXSec* ) m_XSecSurf.FindXSec( i );

        if ( xs )
        {
            XSecCurve* xsc = xs->GetXSecCurve();

            double r = xs->m_RadiusFrac();
            double w = m_ChordCurve.Comp( r ) * radius;

            if ( xsc )
            {
                //==== Find Width Parm ====//
                string width_id = xsc->GetWidthParmID();
                Parm* width_parm = ParmMgr.FindParm( width_id );

                piecewise_curve_type pwc;

                if ( width_parm )
                {
                    width_parm->Deactivate();

                    Airfoil* af = dynamic_cast < Airfoil* > ( xsc );
                    if ( af )
                    {
                        width_parm->Set( 1.0 );
                        xs->GetXSecCurve()->SetFakeWidth( w );
                        xs->GetXSecCurve()->SetUseFakeWidth( true );
                        pwc = xs->GetCurve().GetCurve();
                        xs->GetXSecCurve()->SetUseFakeWidth( false );
                        width_parm->Set( w );
                    }
                    else
                    {
                        CircleXSec* cir = dynamic_cast < CircleXSec* > ( xsc );
                        if ( cir )
                        {
                            width_parm->Set( 1.0 );
                            pwc = xs->GetCurve().GetCurve();
                            width_parm->Set( w );
                        }
                        else
                        {
                            double h = xs->GetXSecCurve()->GetHeight();
                            xsc->SetWidthHeight( 1.0, h/w );
                            pwc = xs->GetCurve().GetCurve();
                            xsc->SetWidthHeight( w, h );
                        }
                    }

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
                xs->m_PropPos.m_XRotate = 0.0;
                xs->m_PropPos.m_ZRotate = atan( -m_RakeCurve.Compdt( r ) ) * 180.0 / PI;

                xs->m_PropPos.m_Rake = m_RakeCurve.Comp( r ) * radius;
                xs->m_PropPos.m_Skew = m_SkewCurve.Comp( r ) * radius;
                xs->m_PropPos.m_PropRot = m_Rotate();
                xs->m_PropPos.m_Feather = m_Feather();

                xs->m_PropPos.m_FoldOrigin = m_FoldAxOrigin;
                xs->m_PropPos.m_FoldDirection = m_FoldAxDirection;
                xs->m_PropPos.m_FoldAngle = m_FoldAngle();

                xs->m_PropPos.m_Reverse = rev;

                crv_vec[i].SetCurve( pwc );
            }
        }
    }

    // This surface linearly interpolates the airfoil sections without
    // any other transformations.
    // These sections can be extracted (as u-const curves) and then
    // transformed to their final position before skinning.
    m_FoilSurf = VspSurf();
    m_FoilSurf.SkinC0( crv_vec, false );

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

    // Pseudo cross sections
    // Not directly user-controlled, but an intermediate step in lofting the
    // surface.
    int npseudo = tmap.size();

    vector < rib_data_type > rib_vec( npseudo );
    vector < double > u_pseudo( npseudo );
    for ( int i = 0; i < npseudo; i++ )
    {
        // Assume linear interpolation means linear u/r relationship.
        double r = tmap[i];
        double u = m_rtou.CompPnt( r );

        VspCurve c;
        m_FoilSurf.GetUConstCurve( c, u );
        vec3d v = c.CompPnt( 0 );
        c.OffsetZ( -v.z() );

        PropPositioner pp;

        pp.m_ParentProp = this->GetXSecSurf( 0 );
        pp.m_Radius = r * radius;

        pp.m_Chord = m_ChordCurve.Comp( r ) * radius;
        pp.m_Twist = m_TwistCurve.Comp( r );

        pp.m_XRotate = 0.0;
        pp.m_ZRotate = atan( -m_RakeCurve.Compdt( r ) ) * 180.0 / PI;

        pp.m_Rake = m_RakeCurve.Comp( r ) * radius;
        pp.m_Skew = m_SkewCurve.Comp( r ) * radius;

        pp.m_PropRot = m_Rotate();
        pp.m_Feather = m_Feather();

        pp.m_FoldOrigin = m_FoldAxOrigin;
        pp.m_FoldDirection = m_FoldAxDirection;
        pp.m_FoldAngle = m_FoldAngle();

        pp.m_Reverse = rev;

        // Set a bunch of other pp variables.
        pp.SetCurve( c );
        pp.Update();

        rib_vec[i].set_f( pp.GetCurve().GetCurve() );
        u_pseudo[i] = ( r - rfirst ) / ( rlast - rfirst );
    }

    m_MainSurfVec.resize( m_Nblade() );

    m_MainSurfVec[0].SetMagicVParm( false );
    m_MainSurfVec[0].SkinCubicSpline( rib_vec, u_pseudo, tdisc, false );

    m_MainSurfVec[0].SetMagicVParm( true );
    m_MainSurfVec[0].SetSurfType( PROP_SURF );
    m_MainSurfVec[0].SetClustering( m_LECluster(), m_TECluster() );

    if ( m_XSecSurf.GetFlipUD() )
    {
        m_MainSurfVec[0].FlipNormal();
    }

    if ( this->m_ReverseFlag() )
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

    double rspan = rlast - rfirst;
    for ( int i = 0; i < n; i++ )
    {
        double t = static_cast < double > ( i ) / ( n - 1 );
        double r = rfirst + rspan * Cluster( t, m_RootCluster(), m_TipCluster() );
        double u = m_rtou.CompPnt( r );

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
        sprintf( str, "%03d", i );
        res->Add( NameValData( "XSection_" + string( str ), xpts ) );
        res->Add( NameValData( "YSection_" + string( str ), ypts ) );

        r_vec[i] = r;
        chord_vec[i] = m_ChordCurve.Comp( r );
        twist_vec[i] = m_TwistCurve.Comp( r );
        rake_vec[i] = m_RakeCurve.Comp( r );
        skew_vec[i] = m_SkewCurve.Comp( r );
    }

    res->Add( NameValData( "Radius", r_vec ) );
    res->Add( NameValData( "Chord", chord_vec ) );
    res->Add( NameValData( "Twist", twist_vec ) );
    res->Add( NameValData( "Rake", rake_vec ) );
    res->Add( NameValData( "Skew", skew_vec ) );

    return res->GetID();
}
