//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//
//
//////////////////////////////////////////////////////////////////////

#define _USE_MATH_DEFINES
#include <cmath>

#include "GearGeom.h"
#include "ParmMgr.h"
#include "LinkMgr.h"
#include "VspCurve.h"
#include "VspSurf.h"
#include "Vehicle.h"
#include "UnitConversion.h"

Bogie::Bogie()
{
    // Bogie
    m_Symmetrical.Init( "Symmetrical", "Bogie", this, false, false, true );

    m_DrawNominal.Init( "DrawNominal", "Bogie", this, true, false, true );

    m_NAcross.Init( "NumAcross", "Bogie", this, 1, 1, 100 );
    m_NTandem.Init( "NumTandem", "Bogie", this, 1, 1, 100 );

    m_SpacingType.Init( "SpacingType", "Bogie", this, vsp::BOGIE_GAP_FRAC, vsp::BOGIE_CENTER_DIST, vsp::NUM_BOGIE_SPACING_TYPE - 1 );
    m_Spacing.Init( "Spacing", "Bogie", this, 1.1, 0.0, 1e12 );
    m_SpacingFrac.Init( "SpacingFrac", "Bogie", this, 1.1, 1.0, 100 );
    m_SpacingGap.Init( "SpacingGap", "Bogie", this, 1.1, 0.0, 1e12 );
    m_SpacingGapFrac.Init( "SpacingGapFrac", "Bogie", this, 0.1, 0.0, 99 );

    m_PitchType.Init( "PitchType", "Bogie", this, vsp::BOGIE_GAP_FRAC, vsp::BOGIE_CENTER_DIST, vsp::NUM_BOGIE_SPACING_TYPE - 1 );
    m_Pitch.Init( "Pitch", "Bogie", this, 1.1, 0.0, 1e12 );
    m_PitchFrac.Init( "PitchFrac", "Bogie", this, 1.1, 1.0, 100 );
    m_PitchGap.Init( "PitchGap", "Bogie", this, 1.1, 0.0, 1e12 );
    m_PitchGapFrac.Init( "PitchGapFrac", "Bogie", this, 0.1, 0.0, 99 );

    m_XContactPt.Init( "XContactPt", "Bogie", this, 0.0, -1e12, 1e12 );
    m_YContactPt.Init( "YContactPt", "Bogie", this, 0.0, -1e12, 1e12 );
    m_ZAboveGround.Init( "ZAboveGround", "Bogie", this, 0.0, -1e12, 1e12 );

    m_TravelX.Init( "TravelX", "Bogie", this, 0.0, -1e12, 1e12 );
    m_TravelY.Init( "TravelY", "Bogie", this, 0.0, -1e12, 1e12 );
    m_TravelZ.Init( "TravelZ", "Bogie", this, 1.0, -1e12, 1e12 );

    m_Travel.Init( "Travel", "Bogie", this, 0.0, -1e12, 1e12 );
    m_TravelCompressed.Init( "TravelCompressed", "Bogie", this, 1.0, 0, 1e12 );
    m_TravelExtended.Init( "TravelExtended", "Bogie", this, 1.0, 0, 1e12 );

    m_BogieTheta.Init( "BogieTheta", "Bogie", this, 0.0, -180, 180 );
    m_BogieThetaMax.Init( "BogieThetaMax", "Bogie", this, 20.0, -180, 180 );
    m_BogieThetaMin.Init( "BogieThetaMin", "Bogie", this, -20.0, -180, 180 );

    m_SteeringAngle.Init( "SteeringAngle", "Bogie", this, 0.0, 0, 90 );

    //Tire
    m_DiameterMode.Init( "DiameterMode", "Tire", this, vsp::TIRE_DIM_IN, vsp::TIRE_DIM_IN, vsp::NUM_TIRE_DIM_MODES - 2 ); // TIRE_DIM_FRAC not allowed
    m_DiameterMode.SetDescript( "Mode to control diameter specification" );
    m_DiameterIn.Init( "DiameterIn", "Tire", this, 13.5, 0.0, 1.0e12 );
    m_DiameterIn.SetDescript( "Diameter of the tire in inches" );
    m_DiameterModel.Init( "DiameterModel", "Tire", this, 13.5/12., 0.0, 1.0e12 );
    m_DiameterModel.SetDescript( "Diameter of the tire in model units" );

    m_WidthMode.Init( "WidthMode", "Tire", this, vsp::TIRE_DIM_IN, vsp::TIRE_DIM_IN, vsp::NUM_TIRE_DIM_MODES - 2 ); // TIRE_DIM_FRAC not allowed
    m_WidthMode.SetDescript( "Mode to control width specification" );
    m_WidthIn.Init( "WidthIn", "Tire", this, 6.0, 0.0, 1.0e12 );
    m_WidthIn.SetDescript( "Width of the tire in inches" );
    m_WidthModel.Init( "WidthModel", "Tire", this, 6.0/12., 0.0, 1.0e12 );
    m_WidthModel.SetDescript( "Width of the tire in model units" );

    m_SLRMode.Init( "SLRMode", "Tire", this, vsp::TIRE_DIM_FRAC, vsp::TIRE_DIM_IN, vsp::NUM_TIRE_DIM_MODES - 1 );
    m_SLRMode.SetDescript( "Mode to control static loaded radius specification" );
    m_DeflectionPct.Init( "DeflectionPct", "Tire", this, 0.35, 0.0, 1.0 );
    m_DeflectionPct.SetDescript( "Static loaded deflection fraction" );
    m_StaticRadiusIn.Init( "StaticRadiusIn", "Tire", this, 0.75, 0.0, 1.0e12 );
    m_StaticRadiusIn.SetDescript( "Static loaded radius in inches" );
    m_StaticRadiusModel.Init( "StaticRadiusModel", "Tire", this, 0.75/12., 0.0, 1.0e12 );
    m_StaticRadiusModel.SetDescript( "Static loaded radius in in model units" );

    m_DrimMode.Init( "DrimMode", "Tire", this, vsp::TIRE_DIM_FRAC, vsp::TIRE_DIM_IN, vsp::NUM_TIRE_DIM_MODES - 1 );
    m_DrimMode.SetDescript( "Mode to control rim diameter specification" );
    m_DrimFrac.Init( "DrimFrac", "Tire", this, 0.29, 0.0, 1.0 );
    m_DrimFrac.SetDescript( "Wheel rim diameter fraction of wheel diameter" );
    m_DrimIn.Init( "DrimIn", "Tire", this, 4.0, 0.0, 1.0e12 );
    m_DrimIn.SetDescript( "Wheel rim diameter in inches" );
    m_DrimModel.Init( "DrimModel", "Tire", this, 4.0/12., 0.0, 1.0e12 );
    m_DrimModel.SetDescript( "Wheel rim diameter in model units" );

    m_WrimMode.Init( "WrimMode", "Tire", this, vsp::TIRE_DIM_FRAC, vsp::TIRE_DIM_IN, vsp::NUM_TIRE_DIM_MODES - 1 );
    m_WrimMode.SetDescript( "Mode to control wheel rim width specification" );
    m_WrimFrac.Init( "WrimFrac", "Tire", this, 0.77, 0.0, 1.0 );
    m_WrimFrac.SetDescript( "Wheel rim fraction of tire width" );
    m_WrimIn.Init( "WrimIn", "Tire", this, 0.88, 0.0, 1.0e12 );
    m_WrimIn.SetDescript( "Wheel rim width in inches" );
    m_WrimModel.Init( "WrimModel", "Tire", this, 0.88/12, 0.0, 1.0e12 );
    m_WrimModel.SetDescript( "Wheel rim width in model units" );

    m_WsMode.Init( "WsMode", "Tire", this, vsp::TIRE_DIM_FRAC, vsp::TIRE_DIM_IN, vsp::NUM_TIRE_DIM_MODES - 1 );
    m_WsMode.SetDescript( "Mode to control shoulder width specification" );
    m_WsFrac.Init( "WsFrac", "Tire", this, 0.88, 0.0, 1.0 );
    m_WsFrac.SetDescript( "Tire shoulder fraction of width" );
    m_WsIn.Init( "WsIn", "Tire", this, 0.88, 0.0, 1.0e12 );
    m_WsIn.SetDescript( "Tire shoulder width in inches" );
    m_WsModel.Init( "WsModel", "Tire", this, 0.88/12., 0.0, 1.0e12 );
    m_WsModel.SetDescript( "Tire shoulder width in model units" );

    m_HsMode.Init( "HsMode", "Tire", this, vsp::TIRE_DIM_FRAC, vsp::TIRE_DIM_IN, vsp::NUM_TIRE_DIM_MODES - 1 );
    m_HsMode.SetDescript( "Mode to control shoulder height specification" );
    m_HsFrac.Init( "HsFrac", "Tire", this, 0.82, 0.0, 1.0 );
    m_HsFrac.SetDescript( "Tire shoulder height fraction of tire height" );
    m_HsIn.Init( "HsIn", "Tire", this, 0.88, 0.0, 1.0e12 );
    m_HsIn.SetDescript( "Tire shoulder height in inches" );
    m_HsModel.Init( "HsModel", "Tire", this, 0.88/12., 0.0, 1.0e12 );
    m_HsModel.SetDescript( "Tire shoulder height in model units" );
}

//==== Parm Changed ====//
void Bogie::ParmChanged( Parm* parm_ptr, int type )
{
    if ( type == Parm::SET )
    {
        m_LateUpdateFlag = true;
    }
    else
    {
        Update();
    }

    //==== Notify Parent Container (XSec) ====//
    ParmContainer* pc = GetParentContainerPtr();
    if ( pc )
    {
        pc->ParmChanged( parm_ptr, type );
    }
}

int Bogie::GetNumSurf() const
{
    int sym = 1;
    if ( m_Symmetrical() )
    {
        sym = 2;
    }

    return m_NAcross() * m_NTandem() * sym;
}

void Bogie:: UpdateParms()
{
    ParmContainer* pc = GetParentContainerPtr();
    GearGeom *gear_geom = dynamic_cast < GearGeom * > ( pc );

    double in2model = 1.0;
    if ( gear_geom )
    {
        in2model = ConvertLength( 1, vsp::LEN_IN, gear_geom->m_ModelLenUnits() );
    }
    double model2in = 1.0 / in2model;


    if ( m_DiameterMode() == vsp::TIRE_DIM_IN )
    {
        m_DiameterModel = m_DiameterIn() * in2model;
    }
    else // vsp::TIRE_DIM_MODEL
    {
        m_DiameterIn = m_DiameterModel() * model2in;
    }

    if ( m_WidthMode() == vsp::TIRE_DIM_IN )
    {
        m_WidthModel = m_WidthIn() * in2model;
    }
    else // vsp::TIRE_DIM_MODEL
    {
        m_WidthIn = m_WidthModel() * model2in;
    }

    // Tire dimensions
    double Do = m_DiameterIn();
    double W = m_WidthIn();

    // Rim
    if ( m_WrimMode() == vsp::TIRE_DIM_IN )
    {
        m_WrimFrac = m_WrimIn() / W;
        m_WrimModel = m_WrimIn() * in2model;
    }
    else if ( m_WrimMode() == vsp::TIRE_DIM_MODEL )
    {
        m_WrimIn = m_WrimModel() * model2in;
        m_WrimFrac = m_WrimIn() / W;
    }
    else // TIRE_DIM_FRAC
    {
        m_WrimIn = m_WrimFrac() * W;
        m_WrimModel = m_WrimIn() * in2model;
    }

    if ( m_DrimMode() == vsp::TIRE_DIM_IN )
    {
        m_DrimFrac = m_DrimIn() / Do;
        m_DrimModel = m_DrimIn() * in2model;
    }
    else if ( m_DrimMode() == vsp::TIRE_DIM_MODEL )
    {
        m_DrimIn = m_DrimModel() * model2in;
        m_DrimFrac = m_DrimIn() / Do;
    }
    else // TIRE_DIM_FRAC
    {
        m_DrimIn = m_DrimFrac() * Do;
        m_DrimModel = m_DrimIn() * in2model;
    }

    // Rim dimensions
    double Drim = m_DrimIn();
    double Wrim = m_WrimIn();
    // double Hflange = 0.55;

    double H = 0.5 * ( Do - Drim );



    if ( m_WsMode() == vsp::TIRE_DIM_IN )
    {
        m_WsFrac = m_WsIn() / W;
        m_WsModel = m_WsIn() * in2model;
    }
    else if ( m_WsMode() == vsp::TIRE_DIM_IN )
    {
        m_WsIn = m_WsModel() * model2in;
        m_WsFrac = m_WsIn() / W;
    }
    else // TIRE_DIM_FRAC
    {
        m_WsIn = m_WsFrac() * W;
        m_WsModel = m_WsIn() * in2model;
    }

    if ( m_HsMode() == vsp::TIRE_DIM_IN )
    {
        m_HsFrac = m_HsIn() / H;
        m_HsModel = m_HsIn() * in2model;
    }
    else if ( m_HsMode() == vsp::TIRE_DIM_IN )
    {
        m_HsIn = m_HsModel() * model2in;
        m_HsFrac = m_HsIn() / H;
    }
    else // TIRE_DIM_FRAC
    {
        m_HsIn = m_HsFrac() * H;
        m_HsModel = m_HsIn() * in2model;
    }

    m_StaticRadiusIn.SetLowerUpperLimits( 0.5 * Drim, 0.5 * Do );
    m_StaticRadiusModel.SetLowerUpperLimits( 0.5 * Drim * in2model, 0.5 * Do * in2model );
    if ( m_SLRMode() == vsp::TIRE_DIM_IN )
    {
        m_DeflectionPct = ( 0.5 * Do - m_StaticRadiusIn() ) / H ;
        m_StaticRadiusModel = m_StaticRadiusIn() * in2model;
    }
    else if ( m_SLRMode() == vsp::TIRE_DIM_MODEL )
    {
        m_StaticRadiusIn = m_StaticRadiusModel() * model2in;
        m_DeflectionPct = ( 0.5 * Do - m_StaticRadiusIn() ) / H ;
    }
    else // TIRE_DIM_FRAC
    {
        m_StaticRadiusIn = 0.5 * Do - m_DeflectionPct() * H;
        m_StaticRadiusModel = m_StaticRadiusIn() * in2model;
    }

    double Wmodel = m_WidthModel();
    double Dmodel = m_DiameterModel();

    switch ( m_SpacingType() )
    {
        case vsp::BOGIE_CENTER_DIST:
            m_SpacingFrac = m_Spacing() / Wmodel;
            m_SpacingGap = m_Spacing() - Wmodel;
            m_SpacingGapFrac = m_SpacingFrac() - 1;
            break;
        case vsp::BOGIE_CENTER_DIST_FRAC:
            m_Spacing = m_SpacingFrac() * Wmodel;
            m_SpacingGap = m_Spacing() - Wmodel;
            m_SpacingGapFrac = m_SpacingFrac() - 1;
            break;
        case vsp::BOGIE_GAP:
            m_Spacing = m_SpacingGap() + Wmodel;
            m_SpacingFrac = m_Spacing() / Wmodel;
            m_SpacingGapFrac = m_SpacingFrac() - 1;
            break;
        case vsp::BOGIE_GAP_FRAC:
        default:
            m_SpacingFrac = m_SpacingGapFrac() + 1;
            m_Spacing = m_SpacingFrac() * Wmodel;
            m_SpacingGap = m_SpacingGapFrac() * Wmodel;
            break;
    }

    switch ( m_PitchType() )
    {
        case vsp::BOGIE_CENTER_DIST:
            m_PitchFrac = m_Pitch() / Dmodel;
            m_PitchGap = m_Pitch() - Dmodel;
            m_PitchGapFrac = m_PitchFrac() - 1;
            break;
        case vsp::BOGIE_CENTER_DIST_FRAC:
            m_Pitch = m_PitchFrac() * Dmodel;
            m_PitchGap = m_Pitch() - Dmodel;
            m_PitchGapFrac = m_PitchFrac() - 1;
            break;
        case vsp::BOGIE_GAP:
            m_Pitch = m_PitchGap() + Dmodel;
            m_PitchFrac = m_Pitch() / Dmodel;
            m_PitchGapFrac = m_PitchFrac() - 1;
            break;
        case vsp::BOGIE_GAP_FRAC:
        default:
            m_PitchFrac = m_PitchGapFrac() + 1;
            m_Pitch = m_PitchFrac() * Dmodel;
            m_PitchGap = m_PitchGapFrac() * Dmodel;
            break;
    }


}

void Bogie::UpdateTireCurve()
{
    // Tire dimensions
    double Do = m_DiameterModel();
    double W = m_WidthModel();

    // Rim dimensions
    double Drim = m_DrimModel();
    double Wrim = m_WrimModel();
    // double Hflange = 0.55;

    double H = 0.5 * ( Do - Drim );


    // Tire shoulder
    double Ws = m_WsModel();
    double Ds = 2 * m_HsModel() + Drim;


    double Cr = 0;
    double Cw = 0;
    double Cside = 0.25;

    m_TireProfile.CreateTire( Do, W, Ds, Ws, Drim, Wrim );
}

void Bogie::Update()
{
    UpdateParms();
    UpdateTireCurve();

    m_TireSurface.CreateBodyRevolution( m_TireProfile, true, 1 );
    m_TireSurface.SetMagicVParm( false );
    m_TireSurface.SetHalfBOR( true );
    m_TireSurface.FlipNormal();

}

void Bogie::UpdateDrawObj( const Matrix4d &relTrans )
{
    m_SuspensionTravelLinesDO.m_PntVec.clear();
    m_SuspensionTravelPointsDO.m_PntVec.clear();

    m_SuspensionTravelLinesDO.m_GeomID = m_ID + "LSuspension";
    m_SuspensionTravelLinesDO.m_Screen = DrawObj::VSP_MAIN_SCREEN;
    m_SuspensionTravelLinesDO.m_LineWidth = 2.0;
    m_SuspensionTravelLinesDO.m_Type = DrawObj::VSP_LINES;
    m_SuspensionTravelLinesDO.m_LineColor = vec3d( 0, 1, 0 );
    m_SuspensionTravelLinesDO.m_GeomChanged = true;

    m_SuspensionTravelPointsDO.m_GeomID = m_ID + "PSuspension";
    m_SuspensionTravelPointsDO.m_Screen = DrawObj::VSP_MAIN_SCREEN;
    m_SuspensionTravelPointsDO.m_PointSize = 12.0;
    m_SuspensionTravelPointsDO.m_Type = DrawObj::VSP_POINTS;
    m_SuspensionTravelPointsDO.m_PointColor = vec3d( 0, 0, 0 );
    m_SuspensionTravelPointsDO.m_GeomChanged = true;


    int isymm = 0;

    m_SuspensionTravelLinesDO.m_PntVec.push_back( relTrans.xform( GetPivotPoint( 0, vsp::GEAR_SUSPENSION_EXTENDED ) ) );
    m_SuspensionTravelLinesDO.m_PntVec.push_back( relTrans.xform( GetPivotPoint( 0, vsp::GEAR_SUSPENSION_COMPRESSED ) ) );

    m_SuspensionTravelPointsDO.m_PntVec.push_back( relTrans.xform( GetPivotPoint( 0, vsp::GEAR_SUSPENSION_EXTENDED ) ) );
    m_SuspensionTravelPointsDO.m_PntVec.push_back( relTrans.xform( GetPivotPoint( 0, vsp::GEAR_SUSPENSION_NOMINAL ) ) );
    m_SuspensionTravelPointsDO.m_PntVec.push_back( relTrans.xform( GetPivotPoint( 0, vsp::GEAR_SUSPENSION_COMPRESSED ) ) );


    m_SuspensionTravelPointsDO.m_PntVec.push_back( relTrans.xform( GetMeanContactPoint( 0,  vsp::TIRE_STATIC_LODED_CONTACT, vsp::GEAR_SUSPENSION_NOMINAL, m_BogieTheta() * M_PI / 180.0 ) ) );
    if ( m_Symmetrical() )
    {
        m_SuspensionTravelPointsDO.m_PntVec.push_back( relTrans.xform( GetMeanContactPoint( 1,  vsp::TIRE_STATIC_LODED_CONTACT, vsp::GEAR_SUSPENSION_NOMINAL, m_BogieTheta() * M_PI / 180.0 ) ) );
    }

    if ( m_Symmetrical() )
    {
        isymm++;
    }
}

void Bogie::LoadDrawObjs( vector< DrawObj* > & draw_obj_vec )
{
    draw_obj_vec.push_back( &m_SuspensionTravelLinesDO );
    draw_obj_vec.push_back( &m_SuspensionTravelPointsDO );
}

string Bogie::GetAcrossDesignation()
{
    switch ( m_NAcross() )
    {
        case 1:
            return string( "S" );
        break;
        case 2:
            return string( "D" );
        break;
        case 3:
            return string( "T" );
        break;
        case 4:
            return string( "Q" );
        break;
        default:
            return "," + to_string( m_NAcross() );
    }
}

string Bogie::GetConfigDesignation()
{
    return to_string( m_NTandem() ) + GetAcrossDesignation();
}

string Bogie::GetTireDesignation()
{
    double k = 1000;
    char buf[255];

    snprintf( buf, sizeof( buf ), "%gx%g-%g", round( k * m_DiameterIn() ) / k,
                                              round( k * m_WidthIn() ) / k,
                                              round( k * m_DrimIn() ) / k );
    return string( buf );
}

string Bogie::GetDesignation( const char* sep )
{
    return GetName() + sep + GetConfigDesignation() + sep + GetTireDesignation();
}

double Bogie::GetTireRadius( int tiremode ) const
{
    switch ( tiremode )
    {
        case vsp::TIRE_NOMINAL_CONTACT:
            return m_DiameterModel() * 0.5;
            break;
        case vsp::TIRE_GROWTH_CONTACT:  // Not correct at this time.  Need to do growth calculation based on speed.
            return m_DiameterModel() * 0.5;
            break;
        case vsp::TIRE_FLAT_CONTACT:
            return m_DrimModel() * 0.5;
            break;
        case vsp::TIRE_STATIC_LODED_CONTACT:
        default:
            return m_StaticRadiusModel();
    }
}

vec3d Bogie::GetTireDeflection( int tiremode ) const
{
    return vec3d( 0, 0, m_StaticRadiusModel() - GetTireRadius( tiremode ) );
}

vec3d Bogie::GetCompressionUnitDirection( int isymm ) const
{
    vec3d dir( m_TravelX(), m_TravelY(), m_TravelZ() );
    dir.normalize();

    if ( isymm > 0 )
    {
        dir.scale_y( -1.0 );
    }
    return dir;
}

vec3d Bogie::GetSuspensionDeflection( int isymm, int suspensionmode ) const
{
    switch ( suspensionmode )
    {
        case vsp::GEAR_SUSPENSION_COMPRESSED:
            return GetCompressionUnitDirection( isymm ) * m_TravelCompressed();
        break;
        case vsp::GEAR_SUSPENSION_EXTENDED:
            return -1.0 * GetCompressionUnitDirection( isymm ) * m_TravelExtended();
        break;
        case vsp::GEAR_SUSPENSION_NOMINAL:
        default:
            return vec3d();
    }
}

// Contact point in ground plane coordinate system.
vec3d Bogie::GetNominalMeanContactPoint( int isymm ) const
{
    vec3d con = vec3d( m_XContactPt(), m_YContactPt(), m_ZAboveGround() );
    if ( isymm > 0 )
    {
        con.scale_y( -1.0 );
    }
    return con;
}

vec3d Bogie::GetMeanContactPoint( int isymm, int tiremode, int suspensionmode, double thetabogie ) const
{
    vec3d v( 0, 0, -GetTireRadius( tiremode ) );
    if ( m_NTandem() > 1 )
    {
        v.rotate_y( thetabogie );
    }
    return GetPivotPoint( isymm, suspensionmode ) + v;
}

vec3d Bogie::GetNominalPivotPoint( int isymm ) const
{
    return GetNominalMeanContactPoint( isymm ) + vec3d( 0, 0, m_StaticRadiusModel() );
}

vec3d Bogie::GetPivotPoint( int isymm, int suspensionmode ) const
{
    return GetNominalPivotPoint( isymm ) + GetSuspensionDeflection( isymm, suspensionmode );
}

double Bogie::GetAxleArm() const
{
    return 0.5 * ( m_NTandem() - 1 ) * m_Pitch();
}

vec3d Bogie::GetAxleDisplacement( double thetabogie ) const
{
    vec3d v( GetAxleArm(), 0, 0 );
    if ( m_NTandem() > 1 )
    {
        v.rotate_y( thetabogie );
    }
    return v;
}

vec3d Bogie::GetFwdAxle( int isymm, int suspensionmode, double thetabogie ) const
{
    return GetPivotPoint( isymm, suspensionmode ) - GetAxleDisplacement( thetabogie );
}

vec3d Bogie::GetAftAxle( int isymm, int suspensionmode, double thetabogie ) const
{
    return GetPivotPoint( isymm, suspensionmode ) + GetAxleDisplacement( thetabogie );
}

vec3d Bogie::GetFwdContactPoint( int isymm, int suspensionmode, int tiremode, double thetabogie, double thetawheel ) const
{
    vec3d v( 0, 0, -GetTireRadius( tiremode ) );
    v.rotate_y( thetawheel + thetabogie );
    return GetFwdAxle( isymm, suspensionmode, thetabogie ) + v;
}

vec3d Bogie::GetAftContactPoint( int isymm, int suspensionmode, int tiremode, double thetabogie, double thetawheel ) const
{
    vec3d v( 0, 0, -GetTireRadius( tiremode ) );
    v.rotate_y( thetawheel + thetabogie );
    return GetAftAxle( isymm, suspensionmode, thetabogie ) + v;
}

void Bogie::AppendMainSurf( vector < VspSurf > &surfvec ) const
{
    TireToBogie( m_TireSurface, surfvec );
}


//===============================================================================//
//===============================================================================//
//===============================================================================//

//==== Constructor ====//
GearGeom::GearGeom( Vehicle* vehicle_ptr ) : Geom( vehicle_ptr )
{
    m_Name = "GearGeom";
    m_Type.m_Name = "Gear";
    m_Type.m_Type = GEAR_GEOM_TYPE;

    m_ModelLenUnits.Init( "m_ModelLenUnits", "Gear", this, vsp::LEN_FT, vsp::LEN_MM, vsp::NUM_LEN_UNIT - 2 ); // Do not allow LEN_UNITLESS

    m_PlaneSize.Init( "PlaneSize", "GroundPlane", this, 10.0, 0.0, 1e12 );
    m_AutoPlaneFlag.Init( "AutoPlaneFlag", "GroundPlane", this, true, false, true );

    m_IncludeNominalGroundPlane.Init( "ShowNominalGroundPlane", "GroundPlane", this, true, false, true );


    m_CGLocalFlag.Init( "CGLocalFlag", "GroundPlane", this, true, false, true );

    m_XCGMinLocal.Init( "XCGMinLocal", "GroundPlane", this, 0, -1e12, 1e12 );
    m_XCGMaxLocal.Init( "XCGMaxLocal", "GroundPlane", this, 0, -1e12, 1e12 );
    m_XCGNominalLocal.Init( "XCGNominalLocal", "GroundPlane", this, 0, -1e12, 1e12 );
    m_YCGMinLocal.Init( "YCGMinLocal", "GroundPlane", this, 0, -1e12, 1e12 );
    m_YCGMaxLocal.Init( "YCGMaxLocal", "GroundPlane", this, 0, -1e12, 1e12 );
    m_YCGNominalLocal.Init( "YCGNominalLocal", "GroundPlane", this, 0, -1e12, 1e12 );
    m_ZCGMinLocal.Init( "ZCGMinLocal", "GroundPlane", this, 0, -1e12, 1e12 );
    m_ZCGMaxLocal.Init( "ZCGMaxLocal", "GroundPlane", this, 0, -1e12, 1e12 );
    m_ZCGNominalLocal.Init( "ZCGNominalLocal", "GroundPlane", this, 0, -1e12, 1e12 );

    m_XCGMinGlobal.Init( "XCGMinGlobal", "GroundPlane", this, 0, -1e12, 1e12 );
    m_XCGMaxGlobal.Init( "XCGMaxGlobal", "GroundPlane", this, 0, -1e12, 1e12 );
    m_XCGNominalGlobal.Init( "XCGNominalGlobal", "GroundPlane", this, 0, -1e12, 1e12 );
    m_YCGMinGlobal.Init( "YCGMinGlobal", "GroundPlane", this, 0, -1e12, 1e12 );
    m_YCGMaxGlobal.Init( "YCGMaxGlobal", "GroundPlane", this, 0, -1e12, 1e12 );
    m_YCGNominalGlobal.Init( "YCGNominalGlobal", "GroundPlane", this, 0, -1e12, 1e12 );
    m_ZCGMinGlobal.Init( "ZCGMinGlobal", "GroundPlane", this, 0, -1e12, 1e12 );
    m_ZCGMaxGlobal.Init( "ZCGMaxGlobal", "GroundPlane", this, 0, -1e12, 1e12 );
    m_ZCGNominalGlobal.Init( "ZCGNominalGlobal", "GroundPlane", this, 0, -1e12, 1e12 );

    //==== Init Parms ====//
    m_TessU = 10;
    m_TessW = 8;

    // Bogie * mg =  CreateAndAddBogie();

    m_MainSurfVec.clear();
}

//==== Destructor ====//
GearGeom::~GearGeom()
{

}

void GearGeom::UpdateSurf()
{

    m_MainNominalCGPointVec.resize( 1 );

    Matrix4d relTrans;

    relTrans = m_AttachMatrix;
    relTrans.affineInverse();
    relTrans.matMult( m_ModelMatrix.data() );
    relTrans.postMult( m_AttachMatrix.data() );

    // Set local based on global.
    if ( !m_CGLocalFlag() )
    {
        Matrix4d invRelTrans = relTrans;
        invRelTrans.affineInverse();

        vec3d cgminglobal( m_XCGMinGlobal(), m_YCGMinGlobal(), m_ZCGMinGlobal() );
        vec3d cgglobal( m_XCGNominalGlobal(), m_YCGNominalGlobal(), m_ZCGNominalGlobal() );
        vec3d cgmaxglobal( m_XCGMaxGlobal(), m_YCGMaxGlobal(), m_ZCGMaxGlobal() );

        m_MainMinCGPoint = invRelTrans.xform( cgminglobal );
        m_MainNominalCGPointVec[0] = invRelTrans.xform( cgglobal );
        m_MainMaxCGPoint = invRelTrans.xform( cgmaxglobal );

        m_XCGNominalLocal = m_MainNominalCGPointVec[0].x();
        m_YCGNominalLocal = m_MainNominalCGPointVec[0].y();
        m_ZCGNominalLocal = m_MainNominalCGPointVec[0].z();

        m_XCGMinLocal = m_MainMinCGPoint.x();
        m_YCGMinLocal = m_MainMinCGPoint.y();
        m_ZCGMinLocal = m_MainMinCGPoint.z();

        m_XCGMaxLocal = m_MainMaxCGPoint.x();
        m_YCGMaxLocal = m_MainMaxCGPoint.y();
        m_ZCGMaxLocal = m_MainMaxCGPoint.z();
    }

    // Make sure local obeys limits.
    m_XCGMinLocal.SetUpperLimit( m_XCGNominalLocal() );
    m_XCGMaxLocal.SetLowerLimit( m_XCGNominalLocal() );

    m_YCGMinLocal.SetUpperLimit( m_YCGNominalLocal() );
    m_YCGMaxLocal.SetLowerLimit( m_YCGNominalLocal() );

    m_ZCGMinLocal.SetUpperLimit( m_ZCGNominalLocal() );
    m_ZCGMaxLocal.SetLowerLimit( m_ZCGNominalLocal() );

    // Grab points
    m_MainMinCGPoint = vec3d( m_XCGMinLocal(), m_YCGMinLocal(), m_ZCGMinLocal() );
    m_MainNominalCGPointVec[0] = vec3d( m_XCGNominalLocal(), m_YCGNominalLocal(), m_ZCGNominalLocal() );
    m_MainMaxCGPoint = vec3d( m_XCGMaxLocal(), m_YCGMaxLocal(), m_ZCGMaxLocal() );

    // Set global from local (obeying local limits)
    vec3d cgminglobal = relTrans.xform( m_MainMinCGPoint );
    vec3d cgglobal = relTrans.xform( m_MainNominalCGPointVec[0] );
    vec3d cgmaxglobal = relTrans.xform( m_MainMaxCGPoint );

    m_XCGMinGlobal = cgminglobal.x();
    m_YCGMinGlobal = cgminglobal.y();
    m_ZCGMinGlobal = cgminglobal.z();

    m_XCGNominalGlobal = cgglobal.x();
    m_YCGNominalGlobal = cgglobal.y();
    m_ZCGNominalGlobal = cgglobal.z();

    m_XCGMaxGlobal = cgmaxglobal.x();
    m_YCGMaxGlobal = cgmaxglobal.y();
    m_ZCGMaxGlobal = cgmaxglobal.z();


    int nbogies = m_Bogies.size();

    int nsurf = 0;
    if ( m_IncludeNominalGroundPlane() )
    {
        nsurf++;
    }
    for ( int i = 0; i < nbogies; i++ )
    {
        if ( m_Bogies[i] )
        {
            if ( !m_GlobalScaleDirty )
            {
                m_Bogies[i]->Update();
            }
            nsurf += m_Bogies[i]->GetNumSurf();
        }
    }

    if ( m_MainSurfVec.size() != nsurf || !m_GlobalScaleDirty )
    {
        m_MainSurfVec.clear();
        m_MainSurfVec.reserve( nsurf );

        if ( m_IncludeNominalGroundPlane() )
        {
            m_MainSurfVec.resize( 1 );
        }

        m_BogieMainSurfIndex.resize( nbogies );

        for ( int i = 0; i < nbogies; i++ )
        {
            if ( m_Bogies[i] )
            {
                m_BogieMainSurfIndex[i] = m_MainSurfVec.size();
                m_Bogies[i]->AppendMainSurf( m_MainSurfVec );
            }
        }
    }


    if ( m_AutoPlaneFlag() )
    {
        BndBox bbox = VehicleMgr.GetVehicle()->GetScaleIndependentBndBox();
        double diag = bbox.DiagDist();

        if ( !bbox.IsEmpty() && diag != 0 )
        {
            m_PlaneSize = bbox.DiagDist();
        }
    }

    if ( m_IncludeNominalGroundPlane() )
    {
        double d = m_PlaneSize();
        m_MainSurfVec[0].CreatePlane( -d, d, -d, d );
        m_MainSurfVec[0].SetSurfCfdType( vsp::CFD_TRANSPARENT );
    }
}

void GearGeom::UpdateMainTessVec( bool firstonly )
{
    int nmain = GetNumMainSurfs();

    if ( m_MainTessVec.size() != nmain || !m_GlobalScaleDirty )
    {
        m_MainTessVec.clear();
        m_MainFeatureTessVec.clear();
        m_MainTessVec.reserve( nmain );
        m_MainFeatureTessVec.reserve( nmain );

        if ( m_IncludeNominalGroundPlane() )
        {
            m_MainTessVec.resize( 1 );
            m_MainFeatureTessVec.resize( 1 );
        }

        int nbogies = m_Bogies.size();
        for ( int i = 0; i < nbogies; i++ )
        {
            if ( m_Bogies[i] )
            {
                // Copy non-surface data from m_MainSurfVec.  Geom::Update() does various 'things' to m_MainSurfVec
                // between UpdateSurf() (when it is populated from m_TireSurface) and here (UpdateMainTessVec).
                // Some of these need to be applied to m_TireSurface before the calls to UpdateSplitTesselate and
                // TessU/WFeatureLine.  These include: VspSurf::InitUMapping(); and
                // VspSurf::BuildFeatureLines( m_ForceXSecFlag );.  Rather than attempting to only copy exactly the
                // required information, CopyNonSurfaceData takes an everything-but-the-kitchen-sink approach.
                m_Bogies[i]->m_TireSurface.CopyNonSurfaceData( m_MainSurfVec[ m_BogieMainSurfIndex[i] ] );


                SimpleTess tireTess;
                SimpleFeatureTess tireFeatureTess;

                UpdateTess( m_Bogies[i]->m_TireSurface, false, false, tireTess, tireFeatureTess );

                m_Bogies[i]->TireToBogie( tireTess, m_MainTessVec );
                m_Bogies[i]->TireToBogie( tireFeatureTess, m_MainFeatureTessVec );
            }
        }
    }

    if ( m_IncludeNominalGroundPlane() )
    {
        // Update MTV for ground plane.
        UpdateTess( m_MainSurfVec[0], false, false, m_MainTessVec[0], m_MainFeatureTessVec[0] );
    }
}

void GearGeom::UpdateTessVec()
{
    Geom::UpdateTessVec();
    ApplySymm( m_MainNominalCGPointVec, m_NominalCGPointVec );


    BndBox cgbox;
    cgbox.Update( m_MainMinCGPoint );
    cgbox.Update( m_MainMaxCGPoint );
    vector < SimpleFeatureTess > tessvec(1);
    tessvec[0].m_ptline.push_back( cgbox.GetBBoxDrawLines() );

    ApplySymm( tessvec, m_LimitsCGPointVec );
}

void GearGeom::UpdateMainDegenGeomPreview()
{
    int nmain = GetNumMainSurfs();

    if ( m_MainDegenGeomPreviewVec.size() != nmain || !m_GlobalScaleDirty )
    {
        m_MainDegenGeomPreviewVec.clear();
        m_MainDegenGeomPreviewVec.reserve( nmain );

        if ( m_IncludeNominalGroundPlane() )
        {
            m_MainDegenGeomPreviewVec.resize( 1 );
        }

        int nbogies = m_Bogies.size();
        for ( int i = 0; i < nbogies; i++ )
        {
            if ( m_Bogies[i] )
            {
                DegenGeom degenGeom;
                CreateDegenGeom( m_Bogies[i]->m_TireSurface, 0, degenGeom, true );

                m_Bogies[i]->TireToBogie( degenGeom, m_MainDegenGeomPreviewVec );
            }
        }
    }

    if ( nmain >= 1 )
    {
        if ( m_IncludeNominalGroundPlane() )
        {
            // Update degen preview for ground plane
            CreateDegenGeom( m_MainSurfVec[0], 0, m_MainDegenGeomPreviewVec[0], true );
        }
    }
}

void GearGeom::UpdateDrawObj()
{
    Geom::UpdateDrawObj();

    Matrix4d relTrans = m_AttachMatrix;
    relTrans.affineInverse();
    relTrans.matMult( m_ModelMatrix.data() );
    relTrans.postMult( m_AttachMatrix.data() );

    int nbogies = m_Bogies.size();

    for ( int i = 0; i < nbogies; i++ )
    {
        if ( m_Bogies[i] )
        {
            if ( !m_GlobalScaleDirty )
            {
                m_Bogies[i]->UpdateDrawObj( relTrans );
            }
        }
    }


    m_CGNominalDrawObj.m_PntVec = m_NominalCGPointVec;
    m_CGNominalDrawObj.m_GeomChanged = true;


    m_CGLimitsDrawObj.m_PntVec.clear();
    for ( int i = 0 ; i < m_LimitsCGPointVec.size() ; i++ )
    {
        for( int j = 0; j < m_LimitsCGPointVec[i].m_ptline.size(); j++ )
        {
            m_CGLimitsDrawObj.m_PntVec.insert( m_CGLimitsDrawObj.m_PntVec.end(), m_LimitsCGPointVec[i].m_ptline[j].begin(), m_LimitsCGPointVec[i].m_ptline[j].end() );
        }
    }
    m_CGLimitsDrawObj.m_GeomChanged = true;

}

void GearGeom::LoadDrawObjs( vector< DrawObj* > & draw_obj_vec )
{
    Geom::LoadDrawObjs( draw_obj_vec );

    vector< DrawObj* > bogie_draw_obj_vec;
    int nbogies = m_Bogies.size();
    for ( int i = 0; i < nbogies; i++ )
    {
        if ( m_Bogies[i] )
        {
            m_Bogies[i]->LoadDrawObjs( bogie_draw_obj_vec );
        }
    }

    for ( int i = 0; i < bogie_draw_obj_vec.size(); i++ )
    {
        bogie_draw_obj_vec[i]->m_Visible = ( m_GuiDraw.GetDispFeatureFlag() && GetSetFlag( vsp::SET_SHOWN ) ) || m_Vehicle->IsGeomActive( m_ID );
        draw_obj_vec.push_back( bogie_draw_obj_vec[i] );
    }

    m_CGNominalDrawObj.m_Screen = DrawObj::VSP_MAIN_SCREEN;
    m_CGNominalDrawObj.m_GeomID = m_ID + string( "cgnominal" );
    m_CGNominalDrawObj.m_Visible = GetSetFlag( vsp::SET_SHOWN );
    m_CGNominalDrawObj.m_PointSize = 10.0;
    m_CGNominalDrawObj.m_PointColor = vec3d( 0.5, 0.5, 0.5 );
    m_CGNominalDrawObj.m_Type = DrawObj::VSP_POINTS;
    draw_obj_vec.push_back( &m_CGNominalDrawObj );

    m_CGLimitsDrawObj.m_Screen = DrawObj::VSP_MAIN_SCREEN;
    m_CGLimitsDrawObj.m_GeomID = m_ID + string( "cgrange" );
    m_CGLimitsDrawObj.m_Visible = GetSetFlag( vsp::SET_SHOWN );
    m_CGLimitsDrawObj.m_LineWidth = 4.0;
    m_CGLimitsDrawObj.m_LineColor = vec3d( 0.5, 0.5, 0.5 );
    m_CGLimitsDrawObj.m_Type = DrawObj::VSP_LINES;
    draw_obj_vec.push_back( &m_CGLimitsDrawObj );
}

//==== Compute Rotation Center ====//
void GearGeom::ComputeCenter()
{

}

//==== Scale ====//
void GearGeom::Scale()
{
    double currentScale = m_Scale() / m_LastScale();
    // m_Length *= currentScale;
    m_LastScale = m_Scale();
}

void GearGeom::AddDefaultSources( double base_len )
{
}

Bogie * GearGeom::CreateAndAddBogie()
{
    Bogie * bogie = new Bogie();

    bogie->SetParentContainer( m_ID );

    m_Bogies.push_back( bogie );

    m_CurrBogieIndex = m_Bogies.size() - 1;

    m_SurfDirty = true;
    return bogie;
}

string GearGeom::CreateAndAddBogie( int foo )
{
    Bogie * bogie = CreateAndAddBogie();

    return bogie->GetID();
}

Bogie * GearGeom::GetCurrentBogie()
{
    if ( m_CurrBogieIndex < 0 || m_CurrBogieIndex >= ( int )m_Bogies.size() )
    {
        return NULL;
    }

    return m_Bogies[ m_CurrBogieIndex ];
}

std::vector < Bogie * > GearGeom::GetBogieVec()
{
    return m_Bogies;
}

xmlNodePtr GearGeom::EncodeXml( xmlNodePtr & node )
{
    Geom::EncodeXml( node );

    char labelName[256];

    xmlNodePtr child_node = xmlNewChild( node, NULL, BAD_CAST "Gear", NULL );

    XmlUtil::AddIntNode( child_node, "Num_of_Bogies", m_Bogies.size() );

    for ( int i = 0; i < ( int )m_Bogies.size(); i++ )
    {
        snprintf( labelName, sizeof( labelName ), "Bogie_%d", i );
        xmlNodePtr label_node = xmlNewChild( child_node, NULL, BAD_CAST labelName, NULL );
        m_Bogies[i]->EncodeXml( label_node );
    }

    return child_node;
}

xmlNodePtr GearGeom::DecodeXml( xmlNodePtr & node )
{
    Geom::DecodeXml( node );

    char labelName[256];

    xmlNodePtr label_root_node = XmlUtil::GetNode( node, "Gear", 0 );

    int numofLabels = XmlUtil::FindInt( label_root_node, "Num_of_Bogies", 0 );
    for ( int i = 0; i < numofLabels; i++ )
    {
        snprintf( labelName, sizeof( labelName ), "Bogie_%d", i );
        xmlNodePtr label_node = XmlUtil::GetNode( label_root_node, labelName, 0 );
        if( label_node )
        {
            xmlNodePtr parmcontain_node = XmlUtil::GetNode( label_node, "ParmContainer", 0 );
            if ( parmcontain_node )
            {
                Bogie * bogie = CreateAndAddBogie();
                if ( bogie )
                {
                    bogie->DecodeXml( label_node );
                }
            }
        }
    }

    return label_root_node;
}

void GearGeom::DelAllBogies()
{
    for( int i = 0; i < ( int )m_Bogies.size(); i++ )
    {
        delete m_Bogies[i];
    }
    m_Bogies.clear();
    m_SurfDirty = true;
    m_CurrBogieIndex = -1;
    Update();
}

void GearGeom::ShowAllBogies()
{
    for( int i = 0; i < ( int )m_Bogies.size(); i++ )
    {
        m_Bogies[i]->m_Visible = true;
    }
}

void GearGeom::HideAllBogies()
{
    for( int i = 0; i < ( int )m_Bogies.size(); i++ )
    {
        m_Bogies[i]->m_Visible = false;
    }
}

Bogie * GearGeom::GetBogie( const string &id ) const
{
    for( int i = 0; i < ( int )m_Bogies.size(); i++ )
    {
        if ( m_Bogies[i]->GetID() == id )
        {
            return m_Bogies[i];
        }
    }

    return NULL;
}

vector < string > GearGeom::GetAllBogies()
{
    vector < string > rulerList( m_Bogies.size() );

    for( int i = 0; i < ( int )m_Bogies.size(); i++ )
    {
        rulerList[i] = m_Bogies[i]->GetID();
    }

    return rulerList;
}

void GearGeom::DelBogie( const int & i )
{
    if ( i < 0 || i >= ( int )m_Bogies.size() )
    {
        return;
    }

    Bogie* ruler = m_Bogies[i];

    m_Bogies.erase( m_Bogies.begin() +  i );
    m_SurfDirty = true;
    Update();
    delete ruler;
}

void GearGeom::DelBogie( const string &id )
{
    int idel = -1;
    for( int i = 0; i < ( int )m_Bogies.size(); i++ )
    {
        if ( m_Bogies[i]->GetID() == id )
        {
            idel = i;
            break;
        }
    }

    DelBogie( idel );
}

void GearGeom::UpdateBBox( )
{
    // Add GearGeom origin point to represent ground plane without scale.
    BndBox gnd_box;
    for ( int isymm = 0; isymm < m_SymmTransMatVec.size(); isymm++ )
    {
        vec3d origin;
        origin.Transform( m_SymmTransMatVec[ isymm ] );
        gnd_box.Update( origin );
    }

    // Fill m_BBox and m_ScaleIndependentBBox while skipping ground plane.
    // Call at the end so m_Bb*Len and m_Bb*Min are updated correctly.
    int istart = 0;
    if ( m_IncludeNominalGroundPlane() )
    {
        istart = 1;
    }
    Geom::UpdateBBox( istart, gnd_box );
}

void GearGeom::BuildTwoPtBasis( const string &cp1, int isymm1, int suspension1, int tire1,
                                const string &cp2, int isymm2, int suspension2, int tire2,
                                double thetabogie, Matrix4d &mat, vec3d &p1, vec3d &p2 )
{
    mat.loadIdentity();

    vec3d pcen, z;
    bool usepivot = false;
    double mintheta, maxtheta;

    if ( GetTwoPtMeanContactPtNormal( cp1, isymm1, suspension1, tire1,
                                      cp2, isymm2, suspension2, tire2,
                                      thetabogie, pcen, z, p1, p2, usepivot, mintheta, maxtheta ) )
    {
        const int iminor = z.minor_comp();

        vec3d y;
        y[ iminor ] = 1;

        vec3d x = cross( y, z );
        x.normalize();
        y = cross( z, x );
        y.normalize();

        mat.translatev( pcen );
        mat.setBasis( x, y, z );
    }
}

void GearGeom::BuildThreePtBasis( const string &cp1, int isymm1, int suspension1, int tire1,
                                  const string &cp2, int isymm2, int suspension2, int tire2,
                                  const string &cp3, int isymm3, int suspension3, int tire3,
                                  Matrix4d &mat )
{
    mat.loadIdentity();

    vec3d pcen, z;

    if ( GetPtNormal( cp1, isymm1, suspension1, tire1,
                      cp2, isymm2, suspension2, tire2,
                      cp3, isymm3, suspension3, tire3,
                      pcen, z ) )
    {
        const int iminor = z.minor_comp();

        vec3d y;
        y[ iminor ] = 1;

        vec3d x = cross( y, z );
        x.normalize();
        y = cross( z, x );
        y.normalize();

        mat.translatev( pcen );
        mat.setBasis( x, y, z );
    }
}

bool GearGeom::GetTwoPtPivot( const string &cp1, int isymm1, int suspension1,
                              const string &cp2, int isymm2, int suspension2,
                              vec3d &ptaxis, vec3d &axis ) const
{
    const Bogie *b1 = GetBogie( cp1 );
    const Bogie *b2 = GetBogie( cp2 );

    if ( b1 && b2 )
    {
        const vec3d p1 = b1->GetPivotPoint( isymm1, suspension1 );
        const vec3d p2 = b2->GetPivotPoint( isymm2, suspension2 );
        ptaxis = ( p1 + p2 ) * 0.5;
        axis = p2 - p1;
        axis.normalize();

        if ( axis.y() < 0 )
        {
            axis = -axis;
        }

        return true;
    }
    return false;
}

bool GearGeom::GetTwoPtAftAxleAxis( const string &cp1, int isymm1, int suspension1,
                                    const string &cp2, int isymm2, int suspension2,
                                    double thetabogie, vec3d &ptaxis, vec3d &axis ) const
{
    const Bogie *b1 = GetBogie( cp1 );
    const Bogie *b2 = GetBogie( cp2 );

    if ( b1 && b2 )
    {
        const vec3d p1 = b1->GetAftAxle( isymm1, suspension1, thetabogie );
        const vec3d p2 = b2->GetAftAxle( isymm2, suspension2, thetabogie );
        ptaxis = ( p1 + p2 ) * 0.5;
        axis = p2 - p1;
        axis.normalize();

        if ( axis.y() < 0 )
        {
            axis = -axis;
        }

        return true;
    }
    return false;
}

bool GearGeom::GetTwoPtFwdAxleAxis( const string &cp1, int isymm1, int suspension1,
                                    const string &cp2, int isymm2, int suspension2,
                                    double thetabogie, vec3d &ptaxis, vec3d &axis ) const
{
    const Bogie *b1 = GetBogie( cp1 );
    const Bogie *b2 = GetBogie( cp2 );

    if ( b1 && b2 )
    {
        const vec3d p1 = b1->GetFwdAxle( isymm1, suspension1, thetabogie );
        const vec3d p2 = b2->GetFwdAxle( isymm2, suspension2, thetabogie );
        ptaxis = ( p1 + p2 ) * 0.5;
        axis = p2 - p1;
        axis.normalize();

        if ( axis.y() < 0 )
        {
            axis = -axis;
        }

        return true;
    }
    return false;
}

bool GearGeom::GetTwoPtMeanContactPtNormal( const string &cp1, int isymm1, int suspension1, int tire1,
                                            const string &cp2, int isymm2, int suspension2, int tire2,
                                            double thetabogie, vec3d &pt, vec3d &normal, vec3d &p1, vec3d &p2, bool &usepivot, double &mintheta, double &maxtheta ) const
{
    const Bogie *b1 = GetBogie( cp1 );
    const Bogie *b2 = GetBogie( cp2 );

    if ( b1 && b2 )
    {
        const vec3d nnom( 0, 0, 1 );
        p1 = b1->GetMeanContactPoint( isymm1, tire1, suspension1, thetabogie );
        p2 = b2->GetMeanContactPoint( isymm2, tire2, suspension2, thetabogie );
        pt = ( p1 + p2 ) * 0.5;

        // Make sure axis points generally to the right.
        vec3d v12 = p2 - p1;
        if ( v12.y() < 0 )
        {
            v12 = -v12;
        }

        const vec3d u = cross( nnom, v12 );

        normal = cross( v12, u );
        normal.normalize();

        // Check that plane points mostly 'up' in GearGeom coordinate system.  Nominal ground plane will point
        // straight up in these coordinates.  Changing the order of the contact points can change the orientation
        // of the normal vector.  This sign check prevents us from requiring cw/ccw ordering by the user.
        if ( normal.z() < 0 )
        {
            normal = -normal;
        }

        usepivot = false;
        Matrix4d mat;
        if ( b1->m_NTandem() > 1 || b2->m_NTandem() > 1 )
        {
            mat.rotate( thetabogie, v12 );
            usepivot = true;
        }
        normal = mat.xformnorm( normal );

        mintheta = min( b1->m_BogieThetaMin(), b2->m_BogieThetaMin() ) * M_PI / 180;
        maxtheta = max( b1->m_BogieThetaMax(), b2->m_BogieThetaMax() ) * M_PI / 180;

        return true;
    }
    return false;
}

bool GearGeom::GetTwoPtAftContactPtNormal( const string &cp1, int isymm1, int suspension1, int tire1,
                                           const string &cp2, int isymm2, int suspension2, int tire2,
                                           double thetabogie, double thetawheel, vec3d &pt, vec3d &normal, vec3d &p1, vec3d &p2 ) const
{
    const Bogie *b1 = GetBogie( cp1 );
    const Bogie *b2 = GetBogie( cp2 );

    if ( b1 && b2 )
    {
        const vec3d nnom( 0, 0, 1 );
        p1 = b1->GetAftContactPoint( isymm1, tire1, suspension1, thetabogie, thetawheel );
        p2 = b2->GetAftContactPoint( isymm2, tire2, suspension2, thetabogie, thetawheel );
        pt = ( p1 + p2 ) * 0.5;

        // Make sure axis points generally to the right.
        vec3d v12 = p2 - p1;
        if ( v12.y() < 0 )
        {
            v12 = -v12;
        }

        const vec3d u = cross( nnom, v12 );

        normal = cross( v12, u );
        normal.normalize();

        // Check that plane points mostly 'up' in GearGeom coordinate system.  Nominal ground plane will point
        // straight up in these coordinates.  Changing the order of the contact points can change the orientation
        // of the normal vector.  This sign check prevents us from requiring cw/ccw ordering by the user.
        if ( normal.z() < 0 )
        {
            normal = -normal;
        }

        Matrix4d mat;
        if ( b1->m_NTandem() > 1 || b2->m_NTandem() > 1 )
        {
            mat.rotate( thetabogie, v12 );
        }
        normal = mat.xformnorm( normal );

        return true;
    }
    return false;
}

bool GearGeom::GetTwoPtFwdContactPtNormal( const string &cp1, int isymm1, int suspension1, int tire1,
                                           const string &cp2, int isymm2, int suspension2, int tire2,
                                           double thetabogie, double thetawheel, vec3d &pt, vec3d &normal, vec3d &p1, vec3d &p2 ) const
{
    const Bogie *b1 = GetBogie( cp1 );
    const Bogie *b2 = GetBogie( cp2 );

    if ( b1 && b2 )
    {
        const vec3d nnom( 0, 0, 1 );
        p1 = b1->GetFwdContactPoint( isymm1, tire1, suspension1, thetabogie, thetawheel );
        p2 = b2->GetFwdContactPoint( isymm2, tire2, suspension2, thetabogie, thetawheel );
        pt = ( p1 + p2 ) * 0.5;

        // Make sure axis points generally to the right.
        vec3d v12 = p2 - p1;
        if ( v12.y() < 0 )
        {
            v12 = -v12;
        }

        const vec3d u = cross( nnom, v12 );

        normal = cross( v12, u );
        normal.normalize();

        // Check that plane points mostly 'up' in GearGeom coordinate system.  Nominal ground plane will point
        // straight up in these coordinates.  Changing the order of the contact points can change the orientation
        // of the normal vector.  This sign check prevents us from requiring cw/ccw ordering by the user.
        if ( normal.z() < 0 )
        {
            normal = -normal;
        }

        Matrix4d mat;
        if ( b1->m_NTandem() > 1 || b2->m_NTandem() > 1 )
        {
            mat.rotate( thetabogie, v12 );
        }
        normal = mat.xformnorm( normal );

        return true;
    }
    return false;
}

bool GearGeom::GetPtNormal( const string &cp1, int isymm1, int suspension1, int tire1,
                            const string &cp2, int isymm2, int suspension2, int tire2,
                            const string &cp3, int isymm3, int suspension3, int tire3,
                            vec3d &pt, vec3d &normal ) const
{
    const Bogie *b1 = GetBogie( cp1 );
    const Bogie *b2 = GetBogie( cp2 );
    const Bogie *b3 = GetBogie( cp3 );

    if ( b1 && b2 && b3 )
    {
        const vec3d p1 = b1->GetMeanContactPoint( isymm1, tire1, suspension1, 0.0 );
        const vec3d p2 = b2->GetMeanContactPoint( isymm2, tire2, suspension2, 0.0 );
        const vec3d p3 = b3->GetMeanContactPoint( isymm3, tire3, suspension3, 0.0 );
        pt = ( p1 + p2 + p3 ) / 3.0;

        const vec3d v12 = p2 - p1;
        const vec3d v13 = p3 - p1;

        normal = cross( v12, v13 );
        normal.normalize();

        // Check that plane points mostly 'up' in GearGeom coordinate system.  Nominal ground plane will point
        // straight up in these coordinates.  Changing the order of the contact points can change the orientation
        // of the normal vector.  This sign check prevents us from requiring cw/ccw ordering by the user.
        if ( normal.z() < 0 )
        {
            normal = -normal;
        }

        return true;
    }
    return false;
}

bool GearGeom::GetTwoPtPivotInWorld( const string &cp1, int isymm1, int suspension1,
                                     const string &cp2, int isymm2, int suspension2,
                                     vec3d &ptaxis, vec3d &axis ) const
{
    bool ret = GetTwoPtPivot( cp1, isymm1, suspension1, cp2, isymm2, suspension2, ptaxis, axis );
    ptaxis = m_ModelMatrix.xform( ptaxis );
    axis = m_ModelMatrix.xformnorm( axis );
    return ret;
}

bool GearGeom::GetTwoPtAftAxleAxisInWorld( const string &cp1, int isymm1, int suspension1,
                                           const string &cp2, int isymm2, int suspension2,
                                           double thetabogie, vec3d &ptaxis, vec3d &axis ) const
{
    bool ret = GetTwoPtAftAxleAxis( cp1, isymm1, suspension1, cp2, isymm2, suspension2, thetabogie, ptaxis, axis );
    ptaxis = m_ModelMatrix.xform( ptaxis );
    axis = m_ModelMatrix.xformnorm( axis );
    return ret;
}

bool GearGeom::GetTwoPtFwdAxleAxisInWorld( const string &cp1, int isymm1, int suspension1,
                                           const string &cp2, int isymm2, int suspension2,
                                           double thetabogie, vec3d &ptaxis, vec3d &axis ) const
{
    bool ret = GetTwoPtFwdAxleAxis( cp1, isymm1, suspension1, cp2, isymm2, suspension2, thetabogie, ptaxis, axis );
    ptaxis = m_ModelMatrix.xform( ptaxis );
    axis = m_ModelMatrix.xformnorm( axis );
    return ret;
}

bool GearGeom::GetTwoPtMeanContactPtNormalInWorld( const string &cp1, int isymm1, int suspension1, int tire1,
                                                   const string &cp2, int isymm2, int suspension2, int tire2,
                                                   double thetabogie, vec3d &pt, vec3d &normal, bool &usepivot, double &mintheta, double &maxtheta ) const
{
    vec3d p1, p2;
    bool ret = GetTwoPtMeanContactPtNormal( cp1, isymm1, suspension1, tire1, cp2, isymm2, suspension2, tire2, thetabogie, pt, normal, p1, p2, usepivot, mintheta, maxtheta );
    pt = m_ModelMatrix.xform( pt );
    normal = m_ModelMatrix.xformnorm( normal );
    return ret;
}

bool GearGeom::GetTwoPtAftContactPtNormalInWorld( const string &cp1, int isymm1, int suspension1, int tire1,
                                                  const string &cp2, int isymm2, int suspension2, int tire2,
                                                  double thetabogie, double thetawheel, vec3d &pt, vec3d &normal ) const
{
    vec3d p1, p2;
    bool ret = GetTwoPtAftContactPtNormal( cp1, isymm1, suspension1, tire1, cp2, isymm2, suspension2, tire2, thetabogie, thetawheel, pt, normal, p1, p2 );
    pt = m_ModelMatrix.xform( pt );
    normal = m_ModelMatrix.xformnorm( normal );
    return ret;
}

bool GearGeom::GetTwoPtFwdContactPtNormalInWorld( const string &cp1, int isymm1, int suspension1, int tire1,
                                                  const string &cp2, int isymm2, int suspension2, int tire2,
                                                  double thetabogie, double thetawheel, vec3d &pt, vec3d &normal ) const
{
    vec3d p1, p2;
    bool ret = GetTwoPtFwdContactPtNormal( cp1, isymm1, suspension1, tire1, cp2, isymm2, suspension2, tire2, thetabogie, thetawheel, pt, normal, p1, p2 );
    pt = m_ModelMatrix.xform( pt );
    normal = m_ModelMatrix.xformnorm( normal );
    return ret;
}

bool GearGeom::GetPtNormalInWorld( const string &cp1, int isymm1, int suspension1, int tire1,
                                   const string &cp2, int isymm2, int suspension2, int tire2,
                                   const string &cp3, int isymm3, int suspension3, int tire3,
                                   vec3d &pt, vec3d &normal ) const
{
    bool ret = GetPtNormal( cp1, isymm1, suspension1, tire1, cp2, isymm2, suspension2, tire2, cp3, isymm3, suspension3, tire3, pt, normal );
    pt = m_ModelMatrix.xform( pt );
    normal = m_ModelMatrix.xformnorm( normal );
    return ret;
}

void GearGeom::GetNominalPtNormalInWorld( vec3d &pt, vec3d &normal ) const
{
    pt = m_ModelMatrix.xform( vec3d() );
    normal = m_ModelMatrix.xformnorm( vec3d( 0, 0, 1 ) );
}

void GearGeom::GetCGInWorld( vec3d &cgnom, vector < vec3d > &cgbounds ) const
{
    cgnom = m_ModelMatrix.xform( m_MainNominalCGPointVec[0] );

    BndBox cgbox;
    cgbox.Update( m_MainMinCGPoint );
    cgbox.Update( m_MainMaxCGPoint );

    cgbounds = cgbox.GetCornerPnts();
    m_ModelMatrix.xformvec( cgbounds );
}
