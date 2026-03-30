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
#include "StlHelper.h"
#include "HingeGeom.h"
#include "WingGeom.h"

Bogie::Bogie()
{
    // Bogie
    m_Symmetrical.Init( "Symmetrical", "Bogie", this, false, false, true );

    // m_DrawNominal.Init( "DrawNominal", "Bogie", this, true, false, true );

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

    // m_Travel.Init( "Travel", "Bogie", this, 0.0, -1e12, 1e12 );
    m_TravelCompressed.Init( "TravelCompressed", "Bogie", this, 1.0, 0, 1e12 );
    m_TravelExtended.Init( "TravelExtended", "Bogie", this, 1.0, 0, 1e12 );

    // m_BogieTheta.Init( "BogieTheta", "Bogie", this, 0.0, -180, 180 );
    m_BogieThetaMax.Init( "BogieThetaMax", "Bogie", this, 20.0, -180, 180 );
    m_BogieThetaMin.Init( "BogieThetaMin", "Bogie", this, -20.0, -180, 180 );

    m_SteeringAngle.Init( "SteeringAngle", "Bogie", this, 0.0, 0, 90 );

    //Tire
    m_TireMode.Init( "TireMode", "Tire", this, vsp::TIRE_TRA, vsp::TIRE_TRA, vsp::NUM_TIRE_MODES - 1 );
    m_TireMode.SetDescript( "Mode to control tire representation" );

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
    m_WrimFrac.Init( "WrimFrac", "Tire", this, 0.775, 0.0, 1.0 );
    m_WrimFrac.SetDescript( "Wheel rim width between flanges fraction of tire width" );
    m_WrimIn.Init( "WrimIn", "Tire", this, 0.88, 0.0, 1.0e12 );
    m_WrimIn.SetDescript( "Wheel rim width between flanges in inches" );
    m_WrimModel.Init( "WrimModel", "Tire", this, 0.88/12, 0.0, 1.0e12 );
    m_WrimModel.SetDescript( "Wheel rim width between flanges in model units" );

    m_PlyRating.Init( "PlyRating", "Tire", this, 20.0, 5.0, 100 );
    m_PlyRating.SetDescript( "Tire ply rating" );

    m_SpeedRating.Init( "SpeedRating", "Tire", this, 160.0, 0.0, 1000.0 );
    m_SpeedRating.SetDescript( "Speed Rating for tire (MPH)" );

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

    m_DFlangeModel.Init( "DFlangeModel", "Tire", this, 0.0, 0.0, 1.0e12 );
    m_DFlangeModel.SetDescript( "Wheel diameter at flanges in model units" );

    m_WGModel.Init( "WGModel", "Tire", this, 0.0, 0.0, 1.0e12 );
    m_WGModel.SetDescript( "Grown tire width in model units" );
    m_DGModel.Init( "DGModel", "Tire", this, 0.0, 0.0, 1.0e12 );
    m_DGModel.SetDescript( "Grown tire diameter in model units" );
    m_WsGModel.Init( "WsGModel", "Tire", this, 0.0, 0.0, 1.0e12 );
    m_WsGModel.SetDescript( "Grown tire shoulder width in model units" );
    m_DsGModel.Init( "DsGModel", "Tire", this, 0.0, 0.0, 1.0e12 );
    m_DsGModel.SetDescript( "Grown tire shoulder diameter in model units" );

    // Retracted
    m_RetMode.Init( "RetMode", "Retract", this, vsp::GEAR_STOWED_POSITION, vsp::GEAR_STOWED_POSITION, vsp::NUM_GEAR_RETRACT_MODES - 1 );
    m_RetMode.SetDescript( "Retract mode" );

    m_StowXLoc.Init( "StowXLoc", "Retract", this, 0.0, -1.0e12, 1.0e12 );

    m_StowXLoc.SetDescript( "Global X Location" );
    m_StowYLoc.Init( "StowYLoc", "Retract", this, 0.0, -1.0e12, 1.0e12 );
    m_StowYLoc.SetDescript( "Global Y Location" );
    m_StowZLoc.Init( "StowZLoc", "Retract", this, 0.0, -1.0e12, 1.0e12 );
    m_StowZLoc.SetDescript( "Global Z Location" );
    m_StowXRot.Init( "StowXRot", "Retract", this, 0.0, -180, 180 );
    m_StowXRot.SetDescript( "Global X Rotation" );
    m_StowYRot.Init( "StowYRot", "Retract", this, 0.0, -180, 180 );
    m_StowYRot.SetDescript( "Global Y Rotation" );
    m_StowZRot.Init( "StowZRot", "Retract", this, 0.0,  -180, 180 );
    m_StowZRot.SetDescript( "Global Z Rotation" );

    m_StowXRelLoc.Init( "StowXRelLoc", "Retract", this, 0.0, -1.0e12, 1.0e12 );
    m_StowXRelLoc.SetDescript( "X Location Relative to Parent" );
    m_StowYRelLoc.Init( "StowYRelLoc", "Retract", this, 0.0, -1.0e12, 1.0e12 );
    m_StowYRelLoc.SetDescript( "Y Location Relative to Parent" );
    m_StowZRelLoc.Init( "StowZRelLoc", "Retract", this, 0.0, -1.0e12, 1.0e12 );
    m_StowZRelLoc.SetDescript( "Z Location Relative to Parent" );
    m_StowXRelRot.Init( "StowXRelRot", "Retract", this, 0.0, -180, 180 );
    m_StowXRelRot.SetDescript( "X Rotation Relative to Parent" );
    m_StowYRelRot.Init( "StowYRelRot", "Retract", this, 0.0, -180, 180 );
    m_StowYRelRot.SetDescript( "Y Rotation Relative to Parent" );
    m_StowZRelRot.Init( "StowZRelRot", "Retract", this, 0.0, -180, 180 );
    m_StowZRelRot.SetDescript( "Z Rotation Relative to Parent" );

    // Attachment Parms
    m_StowAbsRelFlag.Init( "StowAbsRelFlag", "Retract", this, vsp::REL, vsp::ABS, vsp::REL );
    m_StowTransAttachFlag.Init( "StowTransAttachFlag", "StowAttach", this, vsp::ATTACH_TRANS_NONE, vsp::ATTACH_TRANS_NONE, vsp::ATTACH_TRANS_NUM_TYPES - 1 );
    m_StowTransAttachFlag.SetDescript( "Determines relative translation coordinate system" );
    m_StowRotAttachFlag.Init( "StowRotAttachFlag", "StowAttach", this, vsp::ATTACH_ROT_NONE, vsp::ATTACH_ROT_NONE, vsp::ATTACH_ROT_NUM_TYPES - 1 );
    m_StowRotAttachFlag.SetDescript( "Determines relative rotation axes" );
    m_StowULoc.Init( "StowULoc", "StowAttach", this, 0.0, 0.0, 1.0 );
    m_StowULoc.SetDescript( "U Location on parent's surface" );
    m_StowU01.Init( "StowU01", "StowAttach", this, true, false, true );
    m_StowU01.SetDescript( "The U value is specified in [0, 1] basis or [0, N] basis." );
    m_StowU0NLoc.Init( "StowU0NLoc", "StowAttach", this, 0, 0, 1e12 );
    m_StowU0NLoc.SetDescript( "U Location on parent's surface on [0,N] basis." );
    m_StowWLoc.Init( "StowWLoc", "StowAttach", this, 0.0, 0.0, 1.0 );
    m_StowWLoc.SetDescript( "W Location on parent's surface" );

    m_StowRLoc.Init( "StowRLoc", "StowAttach", this, 0.0, 0.0, 1.0 );
    m_StowRLoc.SetDescript( "R Location in parent's volume" );
    m_StowR01.Init( "StowR01", "StowAttach", this, true, false, true );
    m_StowR01.SetDescript( "The R value is specified in [0, 1] basis or [0, N] basis." );
    m_StowR0NLoc.Init( "StowR0NLoc", "StowAttach", this, 0, 0, 1e12 );
    m_StowR0NLoc.SetDescript( "R Location in parent's volume on [0,N] basis." );
    m_StowSLoc.Init( "StowSLoc", "StowAttach", this, 0.5, 0.0, 1.0 );
    m_StowSLoc.SetDescript( "S Location in parent's volume" );
    m_StowTLoc.Init( "StowTLoc", "StowAttach", this, 0.5, 0.0, 1.0 );
    m_StowTLoc.SetDescript( "T Location in parent's volume" );

    m_StowLLoc.Init( "StowLLoc", "StowAttach", this, 0.0, 0.0, 1.0 );
    m_StowLLoc.SetDescript( "L Location in parent's volume" );
    m_StowL01.Init( "StowL01", "StowAttach", this, true, false, true );
    m_StowL01.SetDescript( "The L value is specified in [0, 1] basis or dimensional basis." );
    m_StowL0LenLoc.Init( "StowL0LenLoc", "StowAttach", this, 0, 0, 1e12 );
    m_StowL0LenLoc.SetDescript( "L Location in parent's volume on [0,Len] basis." );
    m_StowMLoc.Init( "StowMLoc", "StowAttach", this, 0.5, 0.0, 1.0 );
    m_StowMLoc.SetDescript( "M Location in parent's volume" );
    m_StowNLoc.Init( "StowNLoc", "StowAttach", this, 0.5, 0.0, 1.0 );
    m_StowNLoc.SetDescript( "N Location in parent's volume" );

    m_StowEtaLoc.Init( "StowEtaLoc", "StowAttach", this, 0.0, 0.0, 1.0 );
    m_StowEtaLoc.SetDescript( "Eta Location in parent's volume" );

    m_StowSurfIndx.Init( "StowSurfIndx", "StowAttach", this, 0, 0, 1e6 );

    // Mechanism
    m_MechKRetract.Init( "MechKRetract", "Retract", this, 0.0, 0.0, 1.0 );

    m_MechXLoc.Init( "Mech_XLoc", "Retract", this, 0.0, -1.0e12, 1.0e12 );
    m_MechYLoc.Init( "Mech_YLoc", "Retract", this, 0.0, -1.0e12, 1.0e12 );
    m_MechZLoc.Init( "Mech_ZLoc", "Retract", this, 0.0, -1.0e12, 1.0e12 );

    m_MechXAxis.Init( "Mech_X_Axis", "Retract", this, 0.0, -1.0, 1.0 );
    m_MechYAxis.Init( "Mech_Y_Axis", "Retract", this, 1.0, -1.0, 1.0 );
    m_MechZAxis.Init( "Mech_Z_Axis", "Retract", this, 0.0, -1.0, 1.0 );

    m_MechXRelLoc.Init( "MechXRelLoc", "Retract", this, 0.0, -1.0e12, 1.0e12 );
    m_MechXRelLoc.SetDescript( "X Location Relative to Parent" );
    m_MechYRelLoc.Init( "MechYRelLoc", "Retract", this, 0.0, -1.0e12, 1.0e12 );
    m_MechYRelLoc.SetDescript( "Y Location Relative to Parent" );
    m_MechZRelLoc.Init( "MechZRelLoc", "Retract", this, 0.0, -1.0e12, 1.0e12 );
    m_MechZRelLoc.SetDescript( "Z Location Relative to Parent" );

    // Attachment Parms
    m_MechAbsRelFlag.Init( "MechAbsRelFlag", "Retract", this, vsp::REL, vsp::ABS, vsp::REL );
    m_MechTransAttachFlag.Init( "MechTransAttachFlag", "MechAttach", this, vsp::ATTACH_TRANS_NONE, vsp::ATTACH_TRANS_NONE, vsp::ATTACH_TRANS_NUM_TYPES - 1 );
    m_MechTransAttachFlag.SetDescript( "Determines relative translation coordinate system" );
    m_MechULoc.Init( "MechULoc", "MechAttach", this, 0.0, 0.0, 1.0 );
    m_MechULoc.SetDescript( "U Location on parent's surface" );
    m_MechU01.Init( "MechU01", "MechAttach", this, true, false, true );
    m_MechU01.SetDescript( "The U value is specified in [0, 1] basis or [0, N] basis." );
    m_MechU0NLoc.Init( "MechU0NLoc", "MechAttach", this, 0, 0, 1e12 );
    m_MechU0NLoc.SetDescript( "U Location on parent's surface on [0,N] basis." );
    m_MechWLoc.Init( "MechWLoc", "MechAttach", this, 0.0, 0.0, 1.0 );
    m_MechWLoc.SetDescript( "W Location on parent's surface" );

    m_MechRLoc.Init( "MechRLoc", "MechAttach", this, 0.0, 0.0, 1.0 );
    m_MechRLoc.SetDescript( "R Location in parent's volume" );
    m_MechR01.Init( "MechR01", "MechAttach", this, true, false, true );
    m_MechR01.SetDescript( "The R value is specified in [0, 1] basis or [0, N] basis." );
    m_MechR0NLoc.Init( "MechR0NLoc", "MechAttach", this, 0, 0, 1e12 );
    m_MechR0NLoc.SetDescript( "R Location in parent's volume on [0,N] basis." );
    m_MechSLoc.Init( "MechSLoc", "MechAttach", this, 0.5, 0.0, 1.0 );
    m_MechSLoc.SetDescript( "S Location in parent's volume" );
    m_MechTLoc.Init( "MechTLoc", "MechAttach", this, 0.5, 0.0, 1.0 );
    m_MechTLoc.SetDescript( "T Location in parent's volume" );

    m_MechLLoc.Init( "MechLLoc", "MechAttach", this, 0.0, 0.0, 1.0 );
    m_MechLLoc.SetDescript( "L Location in parent's volume" );
    m_MechL01.Init( "MechL01", "MechAttach", this, true, false, true );
    m_MechL01.SetDescript( "The L value is specified in [0, 1] basis or dimensional basis." );
    m_MechL0LenLoc.Init( "MechL0LenLoc", "MechAttach", this, 0, 0, 1e12 );
    m_MechL0LenLoc.SetDescript( "L Location in parent's volume on [0,Len] basis." );
    m_MechMLoc.Init( "MechMLoc", "MechAttach", this, 0.5, 0.0, 1.0 );
    m_MechMLoc.SetDescript( "M Location in parent's volume" );
    m_MechNLoc.Init( "MechNLoc", "MechAttach", this, 0.5, 0.0, 1.0 );
    m_MechNLoc.SetDescript( "N Location in parent's volume" );

    m_MechEtaLoc.Init( "MechEtaLoc", "MechAttach", this, 0.0, 0.0, 1.0 );
    m_MechEtaLoc.SetDescript( "Eta Location in parent's volume" );

    m_MechSurfIndx.Init( "MechSurfIndx", "MechAttach", this, 0, 0, 1e6 );

    m_MechKneePos.Init( "MechKneePos", "Retract", this, 0.0, 0, 1.0 );
    m_MechKneeAngle.Init( "MechKneeAngle", "Retract", this, 0.0, -360.0, 360.0 );
    m_MechKneeAzimuthAngle.Init( "MechKneeAzimuthAngle", "Retract", this, 0.0, -180.0, 180.0 );
    m_MechKneeElevationAngle.Init( "MechKneeElevationAngle", "Retract", this, 0.0, -90.0, 90.0 );
    m_MechKneeDownAngle.Init( "MechKneeDownAngle", "Retract", this, 180.0, 0.0, 360.0 );

    m_MechRetAngle.Init( "Mech_RetAngle", "Retract", this, 90.0, -360.0, 360.0 );
    m_MechTwistAngle.Init( "Mech_TwistAngle", "Retract", this, 0.0, -360.0, 360.0 );
    m_MechRollAngle.Init( "m_MechRollAngle", "Retract", this, 0.0, -360.0, 360.0 );
    m_MechBogieAngle.Init( "Mech_BogieAngle", "Retract", this, 0.0, -360.0, 360.0 );

    m_MechStrutDL.Init( "Mech_StrutDL", "Retract", this, 0.0, -1.0e12, 1.0e12 );

    m_TireDirty = true;
    m_TireTessDirty = true;
}

//==== Parm Changed ====//
void Bogie::ParmChanged( Parm* parm_ptr, int type )
{
    if ( parm_ptr )
    {
        SetDirtyFlags( parm_ptr );
    }

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

void Bogie::SetDirtyFlags( Parm* parm_ptr )
{
    if ( !parm_ptr )
    {
        return;
    }

    string gname = parm_ptr->GetGroupName();
    string pname = parm_ptr->GetName();

    if ( gname == string("Tire") )
    {
        m_TireDirty = true;
        m_TireTessDirty = true;
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

// Flange height equation from Tire and Rim ASsociation Aircraft Engineering Design Information Book AC-30-B.
// PR - Ply rating
// H  - Tire height (in)
// LR - Lift ratio
double FlangeHeight( double PR, double H, double LR )
{
    double a = 0.54959;
    double b = 0.0053275;
    double c = 0.00034202;
    double d = 0.0000034138;
    double e = -0.05286;
    double f = 0.0024187;
    double g = -0.0041179;
    double h = 0.00023336;
    double i = -0.14303;
    double j = 0.0079038;
    double k = -0.00017005;

    double PRp4 = PR + 4.0;
    double HoLR = H / LR;

    return ( a + b * PRp4 + c * pow( PRp4, 2 ) + d * pow( PRp4,3 ) + e * HoLR + f * pow(HoLR, 2 ) ) /
           ( 1.0 + g * PRp4 + h * pow( PRp4,2 ) + i * HoLR + j * pow( HoLR,2 ) + k * pow( HoLR,3 ) );
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

    // Tire height
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

    // Lift ratio
    double LR = Do / Drim;

    // Flange height (in)
    double Hflange = FlangeHeight( m_PlyRating(), H, LR );

    // Diameter to flanges.
    double Dflange = Drim + 2.0 * Hflange;
    m_DFlangeModel = Dflange * in2model;

    // Tire height above flanges.
    double Haboveflange = 0.5 * ( Do - Dflange );

    m_StaticRadiusIn.SetLowerUpperLimits( 0.5 * Dflange, 0.5 * Do );
    m_StaticRadiusModel.SetLowerUpperLimits( 0.5 * Dflange * in2model, 0.5 * Do * in2model );
    if ( m_SLRMode() == vsp::TIRE_DIM_IN )
    {
        m_DeflectionPct = ( 0.5 * Do - m_StaticRadiusIn() ) / Haboveflange ;
        m_StaticRadiusModel = m_StaticRadiusIn() * in2model;
    }
    else if ( m_SLRMode() == vsp::TIRE_DIM_MODEL )
    {
        m_StaticRadiusIn = m_StaticRadiusModel() * model2in;
        m_DeflectionPct = ( 0.5 * Do - m_StaticRadiusIn() ) / Haboveflange ;
    }
    else // TIRE_DIM_FRAC
    {
        m_StaticRadiusIn = 0.5 * Do - m_DeflectionPct() * Haboveflange;
        m_StaticRadiusModel = m_StaticRadiusIn() * in2model;
    }

    // Tire aspect ratio.
    double AR = H / W;

    // Section height growth factor
    double GH = 1.115 - ( 0.075 * AR );

    // Section width growth factor
    double GW = 1.04;

    // Grown width
    double WG = GW * W;
    // Grown diameter
    double DG = Drim + 2.0 * GH * H;
    // Grown width at shoulder
    double WSG = GW * m_WsIn();
    // Grown diameter at shoulder
    double DSG = Drim + 2.0 * GH * m_HsIn();

    m_WGModel = WG * in2model;
    m_DGModel = DG * in2model;
    m_WsGModel = WSG * in2model;
    m_DsGModel = DSG * in2model;

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
    ParmContainer* pc = GetParentContainerPtr();
    GearGeom *gear_geom = dynamic_cast < GearGeom * > ( pc );

    double in2model = 1.0;
    if ( gear_geom )
    {
        in2model = ConvertLength( 1, vsp::LEN_IN, gear_geom->m_ModelLenUnits() );
    }

    // Tire dimensions
    double Do = m_DiameterModel();
    double W = m_WidthModel();

    // Rim dimensions
    double Drim = m_DrimModel();
    double Wrim = m_WrimModel(); // Width between flanges

    // Diameter at flanges
    double Dflange = m_DFlangeModel();
    // Flange height in model units.
    double Hflange = 0.5 * ( Dflange - Drim );

    // Tire shoulder
    double Ws = m_WsModel();
    double Ds = 2 * m_HsModel() + Drim;


    m_TireProfile.CreateTire( Do, W, Ds, Ws, Drim, Wrim, Hflange, m_TireMode() );

    if ( m_TireMode() == vsp::TIRE_TRA ||
         m_TireMode() == vsp::TIRE_FAIR_FLANGE ||
         m_TireMode() == vsp::TIRE_FAIR_WHEEL )
    {
        double WG = m_WGModel();
        double DG = m_DGModel();
        double WSG = m_WsGModel();
        double DSG = m_DsGModel();

        m_GrownTireProfile.CreateTire( DG, WG, DSG, WSG, Drim, Wrim, Hflange, m_TireMode() );

        double Hg = 0.5 * ( DG - Drim );

        // Flange radius
        double Rflange = 0.5 * Hflange;
        // Flange width
        double Bmin = 1.3 * Rflange;

        double WGin = WG / in2model;

        double SPDmph = m_SpeedRating();

        // Bias ply clearance alowance.  Calculations in inches.
        double Crin = 0.001 * ( 17.02 + 2.61 * pow( SPDmph * .01, 3.348 ) ) * WGin + 0.4;
        double Cwin = 0.019 * WGin + 0.23;
        double Wmarginin = 0.25;

        double Cr = Crin * in2model;
        double Cw = Cwin * in2model;
        double Wmargin = Wmarginin * in2model;
        double Sx = ( Cw + Cr ) * 0.5;
        m_ClearanceProfile.CreateTRAClearance( WG, DG, WSG, DSG, Hg, Drim, Wrim, Bmin, Hflange, Cw, Cr, Sx, Wmargin );
    }
    else
    {
        m_GrownTireProfile = m_TireProfile;
        m_ClearanceProfile = m_TireProfile;
    }
}

void Bogie::UpdateStowAttachParms()
{
    Vehicle *veh = VehicleMgr.GetVehicle();
    Geom* parent = veh->FindGeom( GetStowParentID() );

    if ( parent )
    {
        if ( m_StowSurfIndx() >= parent->GetNumTotalSurfs() )
        {
            m_StowSurfIndx.Set( 0 );
        }

        if ( parent->isNonSurfaceType() )
        {
            return;
        }

        WingGeom* wing_parent = dynamic_cast < WingGeom * > ( parent );

        double umax = parent->GetUMapMax( m_StowSurfIndx() );
        double lmax = parent->GetSurfPtr( m_StowSurfIndx() )->GetLMax();

        m_StowU0NLoc.SetUpperLimit( umax );
        m_StowR0NLoc.SetUpperLimit( umax );
        m_StowL0LenLoc.SetUpperLimit( lmax );

        if ( m_StowU01.Get() )
        {
            m_StowU0NLoc.Set( m_StowULoc() * umax );
        }
        else
        {
            double val = clamp( m_StowU0NLoc(), 0.0, umax );
            m_StowU0NLoc.Set( val );
            m_StowULoc.Set( val / umax );
        }

        if ( m_StowR01.Get() )
        {
            m_StowR0NLoc.Set( m_StowRLoc() * umax );
        }
        else
        {
            double val = clamp( m_StowR0NLoc(), 0.0, umax );
            m_StowR0NLoc.Set( val );
            m_StowRLoc.Set( val / umax );
        }

        if ( m_StowL01.Get() )
        {
            m_StowL0LenLoc.Set( m_StowLLoc() * lmax );
        }
        else
        {
            double val = clamp( m_StowL0LenLoc(), 0.0, lmax );
            m_StowL0LenLoc.Set( val );
            m_StowLLoc.Set( val / lmax );
        }

        if ( wing_parent )
        {
            if ( m_StowTransAttachFlag() == vsp::ATTACH_TRANS_EtaMN || m_StowRotAttachFlag() == vsp::ATTACH_ROT_EtaMN ) // Eta is active.
            {
                if ( m_StowTransAttachFlag() != vsp::ATTACH_TRANS_UV && m_StowRotAttachFlag() != vsp::ATTACH_ROT_UV ) // U is not active.
                {
                    double u = wing_parent->EtatoU( m_StowEtaLoc() ) / umax;

                    double r;
                    r = u;

                    double l;
                    parent->ConvertRtoL( m_StowSurfIndx(), r, l );

                    double s, t;
                    parent->ConvertLMNtoRST( m_StowSurfIndx(), l, m_StowMLoc(), m_StowNLoc(), r, s, t );

                    double w;
                    if ( t < 0.5 )
                    {
                        w = 0.5 * s;
                    }
                    else
                    {
                        w = 1.0 - 0.5 * s;
                    }

                    m_StowULoc.Set( u );
                    m_StowU0NLoc.Set( m_StowULoc() * umax );
                    m_StowWLoc.Set( w );
                }

                if ( m_StowTransAttachFlag() != vsp::ATTACH_TRANS_RST && m_StowRotAttachFlag() != vsp::ATTACH_ROT_RST ) // R is not active
                {
                    double u, w;
                    u = wing_parent->EtatoU( m_StowEtaLoc() ) / umax;
                    w = m_StowWLoc();
                    double r, s, t;
                    r = u;

                    double l;
                    parent->ConvertRtoL( m_StowSurfIndx(), r, l );

                    parent->ConvertLMNtoRST( m_StowSurfIndx(), m_StowLLoc(), m_StowMLoc(), m_StowNLoc(), r, s, t );
                    m_StowRLoc.Set( r );
                    m_StowR0NLoc.Set( m_StowRLoc() * umax );
                    m_StowSLoc.Set( s );
                    m_StowTLoc.Set( t );
                }

                if ( m_StowTransAttachFlag() != vsp::ATTACH_TRANS_LMN && m_StowRotAttachFlag() != vsp::ATTACH_ROT_LMN ) // L is not active
                {
                    double u = wing_parent->EtatoU( m_StowEtaLoc() ) / umax;

                    double r;
                    r = u;

                    double l;
                    parent->ConvertRtoL( m_StowSurfIndx(), r, l );

                    m_StowLLoc.Set( l );
                    m_StowL0LenLoc.Set( m_StowLLoc() * lmax );
                }

            }
            else // Eta is not active
            {
                if ( m_StowTransAttachFlag() == vsp::ATTACH_TRANS_UV || m_StowRotAttachFlag() == vsp::ATTACH_ROT_UV ) // UV is active
                {
                    m_StowEtaLoc = wing_parent->UtoEta( m_StowULoc() * umax );
                }
                else if ( m_StowTransAttachFlag() == vsp::ATTACH_TRANS_RST || m_StowRotAttachFlag() == vsp::ATTACH_ROT_RST ) // RST is active
                {
                    double r = m_StowRLoc();
                    double u = r;
                    m_StowEtaLoc = wing_parent->UtoEta( u * umax );
                }
                else if ( m_StowTransAttachFlag() == vsp::ATTACH_TRANS_LMN || m_StowRotAttachFlag() == vsp::ATTACH_ROT_LMN ) // LMN is active
                {
                    double l = m_StowLLoc();
                    double r;
                    parent->ConvertLtoR( m_StowSurfIndx(), l, r );
                    double u = r;
                    m_StowEtaLoc = wing_parent->UtoEta( u * umax );
                }
                else // Nothing is active, use U value anyway.
                {
                    m_StowEtaLoc = wing_parent->UtoEta( m_StowULoc() * umax );
                }
            }
        }

        // If UV is active in either way and RST is not active in any way, compute RST from UV.
        if ( ( m_StowTransAttachFlag() == vsp::ATTACH_TRANS_UV || m_StowRotAttachFlag() == vsp::ATTACH_ROT_UV ) &&
             ( m_StowTransAttachFlag() != vsp::ATTACH_TRANS_RST && m_StowRotAttachFlag() != vsp::ATTACH_ROT_RST ) )
        {
            double u, w;
            u = m_StowULoc();
            w = m_StowWLoc();
            double r, s, t;
            r = u;
            s = 2.0 * w;
            t = 0.0;
            if ( w > 0.5 )
            {
                s = 2.0 * ( 1.0 - w );
                t = 1.0;
            }

            m_StowRLoc.Set( r );
            m_StowR0NLoc.Set( m_StowRLoc() * umax );
            m_StowSLoc.Set( s );
            m_StowTLoc.Set( t );
        }

        // If UV is active in either way and LMN is not active in any way, compute LMN from UV.
        if ( ( m_StowTransAttachFlag() == vsp::ATTACH_TRANS_UV || m_StowRotAttachFlag() == vsp::ATTACH_ROT_UV ) &&
             ( m_StowTransAttachFlag() != vsp::ATTACH_TRANS_LMN && m_StowRotAttachFlag() != vsp::ATTACH_ROT_LMN ) )
        {
            double u, w;
            u = m_StowULoc();
            w = m_StowWLoc();
            double r, s, t;
            r = u;
            s = 2.0 * w;
            t = 0.0;
            if ( w > 0.5 )
            {
                s = 2.0 * ( 1.0 - w );
                t = 1.0;
            }


            double l, m, n;

            parent->ConvertRSTtoLMN( m_StowSurfIndx(), r, s, t, l, m, n );
            m_StowLLoc.Set( l );
            m_StowL0LenLoc.Set( m_StowLLoc() * lmax );
            m_StowMLoc.Set( m );
            m_StowNLoc.Set( n );
        }

        // If RST is active in either way and LMN is not active in any way, compute LMN from RST.
        if ( ( m_StowTransAttachFlag() == vsp::ATTACH_TRANS_RST || m_StowRotAttachFlag() == vsp::ATTACH_ROT_RST ) &&
             ( m_StowTransAttachFlag() != vsp::ATTACH_TRANS_LMN && m_StowRotAttachFlag() != vsp::ATTACH_ROT_LMN ) )
        {
            double l, m, n;
            parent->ConvertRSTtoLMN( m_StowSurfIndx(), m_StowRLoc(), m_StowSLoc(), m_StowTLoc(), l, m, n );
            m_StowLLoc.Set( l );
            m_StowL0LenLoc.Set( m_StowLLoc() * lmax );
            m_StowMLoc.Set( m );
            m_StowNLoc.Set( n );
        }

        // If LMN is active in either way and RST is not active in any way, compute RST from LMN.
        if ( ( m_StowTransAttachFlag() == vsp::ATTACH_TRANS_LMN || m_StowRotAttachFlag() == vsp::ATTACH_ROT_LMN ) &&
             ( m_StowTransAttachFlag() != vsp::ATTACH_TRANS_RST && m_StowRotAttachFlag() != vsp::ATTACH_ROT_RST ) )
        {
            double r, s, t;
            parent->ConvertLMNtoRST( m_StowSurfIndx(), m_StowLLoc(), m_StowMLoc(), m_StowNLoc(), r, s, t );
            m_StowRLoc.Set( r );
            m_StowR0NLoc.Set( m_StowRLoc() * umax );
            m_StowSLoc.Set( s );
            m_StowTLoc.Set( t );
        }

        // If RST is active in either way and UV is not active in any way, compute UV from RST.
        if ( ( m_StowTransAttachFlag() == vsp::ATTACH_TRANS_RST || m_StowRotAttachFlag() == vsp::ATTACH_ROT_RST ) &&
             ( m_StowTransAttachFlag() != vsp::ATTACH_TRANS_UV && m_StowRotAttachFlag() != vsp::ATTACH_ROT_UV ) )
        {
            double u, w;
            double r = m_StowRLoc();
            double s = m_StowSLoc();
            double t = m_StowTLoc();

            u = r;
            if ( t < 0.5 )
            {
                w = 0.5 * s;
            }
            else
            {
                w = 1.0 - 0.5 * s;
            }

            m_StowULoc.Set( u );
            m_StowU0NLoc.Set( m_StowULoc() * umax );
            m_StowWLoc.Set( w );
        }

        // If LMN is active in either way and UV is not active in any way, compute UV from LMN.
        if ( ( m_StowTransAttachFlag() == vsp::ATTACH_TRANS_LMN || m_StowRotAttachFlag() == vsp::ATTACH_ROT_LMN ) &&
             ( m_StowTransAttachFlag() != vsp::ATTACH_TRANS_UV && m_StowRotAttachFlag() != vsp::ATTACH_ROT_UV ) )
        {
            double u, w;
            double r, s, t;

            parent->ConvertLMNtoRST( m_StowSurfIndx(), m_StowLLoc(), m_StowMLoc(), m_StowNLoc(), r, s, t );

            u = r;
            if ( t < 0.5 )
            {
                w = 0.5 * s;
            }
            else
            {
                w = 1.0 - 0.5 * s;
            }

            m_StowULoc.Set( u );
            m_StowU0NLoc.Set( m_StowULoc() * umax );
            m_StowWLoc.Set( w );
        }
    }
}

void Bogie::UpdateMechAttachParms()
{
    Vehicle *veh = VehicleMgr.GetVehicle();
    Geom* parent = veh->FindGeom( GetMechParentID() );

    if ( parent )
    {
        if ( m_MechSurfIndx() >= parent->GetNumTotalSurfs() )
        {
            m_MechSurfIndx.Set( 0 );
        }

        if ( parent->isNonSurfaceType() )
        {
            return;
        }

        WingGeom* wing_parent = dynamic_cast < WingGeom * > ( parent );

        double umax = parent->GetUMapMax( m_MechSurfIndx() );
        double lmax = parent->GetSurfPtr( m_MechSurfIndx() )->GetLMax();

        m_MechU0NLoc.SetUpperLimit( umax );
        m_MechR0NLoc.SetUpperLimit( umax );
        m_MechL0LenLoc.SetUpperLimit( lmax );

        if ( m_MechU01.Get() )
        {
            m_MechU0NLoc.Set( m_MechULoc() * umax );
        }
        else
        {
            double val = clamp( m_MechU0NLoc(), 0.0, umax );
            m_MechU0NLoc.Set( val );
            m_MechULoc.Set( val / umax );
        }

        if ( m_MechR01.Get() )
        {
            m_MechR0NLoc.Set( m_MechRLoc() * umax );
        }
        else
        {
            double val = clamp( m_MechR0NLoc(), 0.0, umax );
            m_MechR0NLoc.Set( val );
            m_MechRLoc.Set( val / umax );
        }

        if ( m_MechL01.Get() )
        {
            m_MechL0LenLoc.Set( m_MechLLoc() * lmax );
        }
        else
        {
            double val = clamp( m_MechL0LenLoc(), 0.0, lmax );
            m_MechL0LenLoc.Set( val );
            m_MechLLoc.Set( val / lmax );
        }

        if ( wing_parent )
        {
            if ( m_MechTransAttachFlag() == vsp::ATTACH_TRANS_EtaMN ) // Eta is active.
            {
                double u = wing_parent->EtatoU( m_MechEtaLoc() ) / umax;

                double r;
                r = u;

                double l;
                parent->ConvertRtoL( m_MechSurfIndx(), r, l );

                double s, t;
                parent->ConvertLMNtoRST( m_MechSurfIndx(), l, m_MechMLoc(), m_MechNLoc(), r, s, t );

                double w;
                if ( t < 0.5 )
                {
                    w = 0.5 * s;
                }
                else
                {
                    w = 1.0 - 0.5 * s;
                }

                m_MechULoc.Set( u );
                m_MechU0NLoc.Set( m_MechULoc() * umax );
                m_MechWLoc.Set( w );

                m_MechRLoc.Set( r );
                m_MechR0NLoc.Set( m_MechRLoc() * umax );
                m_MechSLoc.Set( s );
                m_MechTLoc.Set( t );

                m_MechLLoc.Set( l );
                m_MechL0LenLoc.Set( m_MechLLoc() * lmax );
            }
            else // Eta is not active
            {
                if ( m_MechTransAttachFlag() == vsp::ATTACH_TRANS_UV ) // UV is active
                {
                    m_MechEtaLoc = wing_parent->UtoEta( m_MechULoc() * umax );
                }
                else if ( m_MechTransAttachFlag() == vsp::ATTACH_TRANS_RST ) // RST is active
                {
                    double r = m_MechRLoc();
                    double u = r;
                    m_MechEtaLoc = wing_parent->UtoEta( u * umax );
                }
                else if ( m_MechTransAttachFlag() == vsp::ATTACH_TRANS_LMN ) // LMN is active
                {
                    double l = m_MechLLoc();
                    double r;
                    parent->ConvertLtoR( m_MechSurfIndx(), l, r );
                    double u = r;
                    m_MechEtaLoc = wing_parent->UtoEta( u * umax );
                }
                else // Nothing is active, use U value anyway.
                {
                    m_MechEtaLoc = wing_parent->UtoEta( m_MechULoc() * umax );
                }
            }
        }

        // If UV is active in either way and RST is not active in any way, compute RST from UV.
        // If UV is active in either way and LMN is not active in any way, compute LMN from UV.
        if ( m_MechTransAttachFlag() == vsp::ATTACH_TRANS_UV )
        {
            double u, w;
            u = m_MechULoc();
            w = m_MechWLoc();
            double r, s, t;
            r = u;
            s = 2.0 * w;
            t = 0.0;
            if ( w > 0.5 )
            {
                s = 2.0 * ( 1.0 - w );
                t = 1.0;
            }

            m_MechRLoc.Set( r );
            m_MechR0NLoc.Set( m_MechRLoc() * umax );
            m_MechSLoc.Set( s );
            m_MechTLoc.Set( t );

            double l, m, n;

            parent->ConvertRSTtoLMN( m_MechSurfIndx(), r, s, t, l, m, n );
            m_MechLLoc.Set( l );
            m_MechL0LenLoc.Set( m_MechLLoc() * lmax );
            m_MechMLoc.Set( m );
            m_MechNLoc.Set( n );

        }


        // If RST is active in either way and LMN is not active in any way, compute LMN from RST.
        // If RST is active in either way and UV is not active in any way, compute UV from RST.
        if ( m_MechTransAttachFlag() == vsp::ATTACH_TRANS_RST  )
        {
            double l, m, n;
            double r = m_MechRLoc();
            double s = m_MechSLoc();
            double t = m_MechTLoc();
            parent->ConvertRSTtoLMN( m_MechSurfIndx(), r, s, t, l, m, n );
            m_MechLLoc.Set( l );
            m_MechL0LenLoc.Set( m_MechLLoc() * lmax );
            m_MechMLoc.Set( m );
            m_MechNLoc.Set( n );

            double u, w;
            u = r;
            if ( t < 0.5 )
            {
                w = 0.5 * s;
            }
            else
            {
                w = 1.0 - 0.5 * s;
            }

            m_MechULoc.Set( u );
            m_MechU0NLoc.Set( m_MechULoc() * umax );
            m_MechWLoc.Set( w );
        }

        // If LMN is active in either way and RST is not active in any way, compute RST from LMN.
        // If LMN is active in either way and UV is not active in any way, compute UV from LMN.
        if ( m_MechTransAttachFlag() == vsp::ATTACH_TRANS_LMN  )
        {
            double r, s, t;
            parent->ConvertLMNtoRST( m_MechSurfIndx(), m_MechLLoc(), m_MechMLoc(), m_MechNLoc(), r, s, t );
            m_MechRLoc.Set( r );
            m_MechR0NLoc.Set( m_MechRLoc() * umax );
            m_MechSLoc.Set( s );
            m_MechTLoc.Set( t );

            double u, w;

            u = r;
            if ( t < 0.5 )
            {
                w = 0.5 * s;
            }
            else
            {
                w = 1.0 - 0.5 * s;
            }

            m_MechULoc.Set( u );
            m_MechU0NLoc.Set( m_MechULoc() * umax );
            m_MechWLoc.Set( w );
        }
    }
}

//==== Set Rel or Abs Flag ====//
void Bogie::DeactivateStowXForms()
{
    // Deactivate non driving parms and Activate driving parms
    if ( m_StowAbsRelFlag() ==  vsp::REL  )
    {
        m_StowXLoc.Deactivate();
        m_StowYLoc.Deactivate();
        m_StowZLoc.Deactivate();
        m_StowXRot.Deactivate();
        m_StowYRot.Deactivate();
        m_StowZRot.Deactivate();

        m_StowXRelLoc.Activate();
        m_StowYRelLoc.Activate();
        m_StowZRelLoc.Activate();
        m_StowXRelRot.Activate();
        m_StowYRelRot.Activate();
        m_StowZRelRot.Activate();
    }
    else
    {
        m_StowXRelLoc.Deactivate();
        m_StowYRelLoc.Deactivate();
        m_StowZRelLoc.Deactivate();
        m_StowXRelRot.Deactivate();
        m_StowYRelRot.Deactivate();
        m_StowZRelRot.Deactivate();

        m_StowXLoc.Activate();
        m_StowYLoc.Activate();
        m_StowZLoc.Activate();
        m_StowXRot.Activate();
        m_StowYRot.Activate();
        m_StowZRot.Activate();
    }

    m_StowULoc.Activate();
    m_StowU0NLoc.Activate();
    m_StowWLoc.Activate();
    m_StowRLoc.Activate();
    m_StowR0NLoc.Activate();
    m_StowSLoc.Activate();
    m_StowTLoc.Activate();
    m_StowLLoc.Activate();
    m_StowL0LenLoc.Activate();
    m_StowMLoc.Activate();
    m_StowNLoc.Activate();
    m_StowTransAttachFlag.Activate();
    m_StowRotAttachFlag.Activate();

    if ( IsStowParentJoint() )
    {
        m_StowULoc.Deactivate();
        m_StowU0NLoc.Deactivate();
        m_StowU01.Deactivate();
        m_StowWLoc.Deactivate();
        m_StowRLoc.Deactivate();
        m_StowR01.Deactivate();
        m_StowR0NLoc.Deactivate();
        m_StowSLoc.Deactivate();
        m_StowTLoc.Deactivate();
        m_StowLLoc.Deactivate();
        m_StowL01.Deactivate();
        m_StowL0LenLoc.Deactivate();
        m_StowMLoc.Deactivate();
        m_StowNLoc.Deactivate();
        m_StowEtaLoc.Deactivate();
        m_StowTransAttachFlag.Deactivate();
        m_StowRotAttachFlag.Deactivate();
    }

    if ( m_StowU01() )
    {
        m_StowU0NLoc.Deactivate();
    }
    else
    {
        m_StowULoc.Deactivate();
    }

    if ( m_StowR01() )
    {
        m_StowR0NLoc.Deactivate();
    }
    else
    {
        m_StowRLoc.Deactivate();
    }

    if ( m_StowL01() )
    {
        m_StowL0LenLoc.Deactivate();
    }
    else
    {
        m_StowLLoc.Deactivate();
    }

}

//==== Set Rel or Abs Flag ====//
void Bogie::DeactivateMechXForms()
{
    ParmContainer* pc = GetParentContainerPtr();
    if ( pc )
    {
        GearGeom * gg = dynamic_cast< GearGeom* >( pc );
        if ( gg )
        {
            int gear_config = gg->m_GearConfigMode();

            if ( gear_config == vsp::GEAR_CONFIGURATION_INTERMEDIATE ||
                 gear_config == vsp::GEAR_CONFIGURATION_ALL )
            {
                m_MechKRetract.Activate();
            }
            else
            {
                m_MechKRetract.Deactivate();
            }
        }
    }

    // Deactivate non driving parms and Activate driving parms
    if ( m_MechAbsRelFlag() ==  vsp::REL  )
    {
        m_MechXLoc.Deactivate();
        m_MechYLoc.Deactivate();
        m_MechZLoc.Deactivate();

        m_MechXRelLoc.Activate();
        m_MechYRelLoc.Activate();
        m_MechZRelLoc.Activate();
    }
    else
    {
        m_MechXRelLoc.Deactivate();
        m_MechYRelLoc.Deactivate();
        m_MechZRelLoc.Deactivate();

        m_MechXLoc.Activate();
        m_MechYLoc.Activate();
        m_MechZLoc.Activate();
    }

    m_MechULoc.Activate();
    m_MechU0NLoc.Activate();
    m_MechWLoc.Activate();
    m_MechRLoc.Activate();
    m_MechR0NLoc.Activate();
    m_MechSLoc.Activate();
    m_MechTLoc.Activate();
    m_MechLLoc.Activate();
    m_MechL0LenLoc.Activate();
    m_MechMLoc.Activate();
    m_MechNLoc.Activate();
    m_MechTransAttachFlag.Activate();

    if ( IsMechParentJoint() )
    {
        m_MechULoc.Deactivate();
        m_MechU0NLoc.Deactivate();
        m_MechU01.Deactivate();
        m_MechWLoc.Deactivate();
        m_MechRLoc.Deactivate();
        m_MechR01.Deactivate();
        m_MechR0NLoc.Deactivate();
        m_MechSLoc.Deactivate();
        m_MechTLoc.Deactivate();
        m_MechLLoc.Deactivate();
        m_MechL01.Deactivate();
        m_MechL0LenLoc.Deactivate();
        m_MechMLoc.Deactivate();
        m_MechNLoc.Deactivate();
        m_MechEtaLoc.Deactivate();
        m_MechTransAttachFlag.Deactivate();
    }

    if ( m_MechU01() )
    {
        m_MechU0NLoc.Deactivate();
    }
    else
    {
        m_MechULoc.Deactivate();
    }

    if ( m_MechR01() )
    {
        m_MechR0NLoc.Deactivate();
    }
    else
    {
        m_MechRLoc.Deactivate();
    }

    if ( m_MechL01() )
    {
        m_MechL0LenLoc.Deactivate();
    }
    else
    {
        m_MechLLoc.Deactivate();
    }

}

void Bogie::ComposeStowAttachMatrix()
{
    m_StowAttachMatrix.loadIdentity();

    Vehicle *veh = VehicleMgr.GetVehicle();
    Geom* parent = veh->FindGeom( GetStowParentID() );

    if ( parent )
    {
        HingeGeom* hingeparent = dynamic_cast < HingeGeom* > ( parent );
        if ( hingeparent )
        {
            m_StowAttachMatrix = hingeparent->GetJointMatrix();

            Matrix4d gmm = m_GearModelMatrix;
            gmm.affineInverse();

            m_StowAttachMatrix.postMult( gmm.data() );
            return;
        }
    }

    // If both attachment flags set to none, return identity
    if ( m_StowTransAttachFlag() == vsp::ATTACH_TRANS_NONE && m_StowRotAttachFlag() == vsp::ATTACH_ROT_NONE )
    {
        m_StowAttachMatrix.translatev( GetNominalPivotPoint( 0 ) );
        return;
    }

    if ( parent )
    {
        Matrix4d transMat;
        Matrix4d rotMat;

        Matrix4d parentMat;
        parentMat = parent->getModelMatrix();

        double tempMat[16];
        parentMat.getMat( tempMat );

        bool revertCompTrans = false;
        bool revertCompRot = false;

        WingGeom* wing_parent = dynamic_cast < WingGeom * > ( parent );

        // Parent CompXXXCoordSys methods query the positioned m_RetSurfVec[0] surface,
        // not m_RetMainSurfVec[0].  Consequently, m_ModelMatrix is already implied in
        // these calculations and does not need to be applied again.
        if ( m_StowTransAttachFlag() == vsp::ATTACH_TRANS_UV )
        {
            if ( !( parent->CompTransCoordSys( m_StowSurfIndx(), m_StowULoc(), m_StowWLoc(), transMat )) )
            {
                revertCompTrans = true; // Any Geom without a surface reverts to the component matrix.
            }
        }

        if ( m_StowTransAttachFlag() == vsp::ATTACH_TRANS_RST )
        {
            if ( !( parent->CompTransCoordSysRST( m_StowSurfIndx(), m_StowRLoc(), m_StowSLoc(), m_StowTLoc(), transMat )) )
            {
                revertCompTrans = true; // Any Geom without a surface reverts to the component matrix.
            }
        }

        if ( m_StowTransAttachFlag() == vsp::ATTACH_TRANS_LMN )
        {
            if ( !( parent->CompTransCoordSysLMN( m_StowSurfIndx(), m_StowLLoc(), m_StowMLoc(), m_StowNLoc(), transMat )) )
            {
                revertCompTrans = true; // Any Geom without a surface reverts to the component matrix.
            }
        }

        if ( m_StowTransAttachFlag() == vsp::ATTACH_TRANS_EtaMN )
        {
            double l = m_StowEtaLoc();

            if ( wing_parent )
            {
                double umax = parent->GetUMapMax( m_StowSurfIndx() );
                double u = wing_parent->EtatoU( m_StowEtaLoc() ) / umax;

                double r = u;
                parent->ConvertRtoL( m_StowSurfIndx(), r, l );
            }

            if ( !( parent->CompTransCoordSysLMN( m_StowSurfIndx(), l, m_StowMLoc(), m_StowNLoc(), transMat )) )
            {
                revertCompTrans = true; // Any Geom without a surface reverts to the component matrix.
            }
        }

        if ( m_StowTransAttachFlag() == vsp::ATTACH_TRANS_COMP || revertCompTrans )
        {
            transMat.translatef( tempMat[12], tempMat[13], tempMat[14] );
        }

        if ( m_StowTransAttachFlag() == vsp::ATTACH_TRANS_NONE )
        {
            transMat.translatev( GetNominalPivotPoint( 0 ) );

            double tempMat2[16];
            m_GearModelMatrix.getMat( tempMat2 );

            transMat.translatef( tempMat2[12], tempMat2[13], tempMat2[14] );
        }

        if ( m_StowRotAttachFlag() == vsp::ATTACH_ROT_UV )
        {
            if ( !( parent->CompRotCoordSys( m_StowSurfIndx(), m_StowULoc(), m_StowWLoc(), rotMat )) )
            {
                revertCompRot = true; // Any Geom without a surface reverts to the component matrix.
            }
        }

        if ( m_StowRotAttachFlag() == vsp::ATTACH_ROT_RST )
        {
            if ( !( parent->CompRotCoordSysRST( m_StowSurfIndx(), m_StowRLoc(), m_StowSLoc(), m_StowTLoc(), rotMat )) )
            {
                revertCompRot = true; // Any Geom without a surface reverts to the component matrix.
            }
        }

        if ( m_StowRotAttachFlag() == vsp::ATTACH_ROT_LMN )
        {
            if ( !( parent->CompRotCoordSysLMN( m_StowSurfIndx(), m_StowLLoc(), m_StowMLoc(), m_StowNLoc(), rotMat )) )
            {
                revertCompRot = true; // Any Geom without a surface reverts to the component matrix.
            }
        }

        if ( m_StowRotAttachFlag() == vsp::ATTACH_ROT_EtaMN )
        {
            double l = m_StowEtaLoc();

            if ( wing_parent )
            {
                double umax = parent->GetUMapMax( m_StowSurfIndx() );
                double u = wing_parent->EtatoU( m_StowEtaLoc() ) / umax;

                double r = u;
                parent->ConvertRtoL( m_StowSurfIndx(), r, l );
            }

            if ( !( parent->CompRotCoordSysLMN( m_StowSurfIndx(), l, m_StowMLoc(), m_StowNLoc(), rotMat )) )
            {
                revertCompRot = true; // Any Geom without a surface reverts to the component matrix.
            }
        }

        if ( m_StowRotAttachFlag() == vsp::ATTACH_ROT_COMP || revertCompRot )
        {
            // Only take rotation matrix from parent so set translation part to zero
            tempMat[12] = tempMat[13] = tempMat[14] = 0;
            rotMat.initMat( tempMat );
        }

        if ( m_StowRotAttachFlag() == vsp::ATTACH_ROT_NONE )
        {
            double tempMat2[16];
            m_GearModelMatrix.getMat( tempMat2 );

            tempMat2[12] = tempMat2[13] = tempMat2[14] = 0;
            rotMat.initMat( tempMat2 );
        }

        transMat.matMult( rotMat.data() );
        m_StowAttachMatrix = transMat;
    }

    Matrix4d gmm = m_GearModelMatrix;
    gmm.affineInverse();

    m_StowAttachMatrix.postMult( gmm.data() );
}

void Bogie::ComposeMechAttachMatrix()
{
    m_MechAttachMatrix.loadIdentity();

    Vehicle *veh = VehicleMgr.GetVehicle();
    Geom* parent = veh->FindGeom( GetMechParentID() );

    if ( parent )
    {
        HingeGeom* hingeparent = dynamic_cast < HingeGeom* > ( parent );
        if ( hingeparent )
        {
            m_MechAttachMatrix = hingeparent->GetJointMatrix();

            Matrix4d gmm = m_GearModelMatrix;
            gmm.affineInverse();

            m_MechAttachMatrix.postMult( gmm.data() );
            return;
        }
    }

    // If both attachment flags set to none, return identity
    if ( m_MechTransAttachFlag() == vsp::ATTACH_TRANS_NONE )
    {
        m_MechAttachMatrix.translatev( GetNominalPivotPoint( 0 ) );
        return;
    }

    if ( parent )
    {
        Matrix4d transMat;

        Matrix4d parentMat;
        parentMat = parent->getModelMatrix();

        double tempMat[16];
        parentMat.getMat( tempMat );

        bool revertCompTrans = false;

        WingGeom* wing_parent = dynamic_cast < WingGeom * > ( parent );

        // Parent CompXXXCoordSys methods query the positioned m_RetSurfVec[0] surface,
        // not m_RetMainSurfVec[0].  Consequently, m_ModelMatrix is already implied in
        // these calculations and does not need to be applied again.
        if ( m_MechTransAttachFlag() == vsp::ATTACH_TRANS_UV )
        {
            if ( !( parent->CompTransCoordSys( m_MechSurfIndx(), m_MechULoc(), m_MechWLoc(), transMat )) )
            {
                revertCompTrans = true; // Any Geom without a surface reverts to the component matrix.
            }
        }

        if ( m_MechTransAttachFlag() == vsp::ATTACH_TRANS_RST )
        {
            if ( !( parent->CompTransCoordSysRST( m_MechSurfIndx(), m_MechRLoc(), m_MechSLoc(), m_MechTLoc(), transMat )) )
            {
                revertCompTrans = true; // Any Geom without a surface reverts to the component matrix.
            }
        }

        if ( m_MechTransAttachFlag() == vsp::ATTACH_TRANS_LMN )
        {
            if ( !( parent->CompTransCoordSysLMN( m_MechSurfIndx(), m_MechLLoc(), m_MechMLoc(), m_MechNLoc(), transMat )) )
            {
                revertCompTrans = true; // Any Geom without a surface reverts to the component matrix.
            }
        }

        if ( m_MechTransAttachFlag() == vsp::ATTACH_TRANS_EtaMN )
        {
            double l = m_MechEtaLoc();

            if ( wing_parent )
            {
                double umax = parent->GetUMapMax( m_MechSurfIndx() );
                double u = wing_parent->EtatoU( m_MechEtaLoc() ) / umax;

                double r = u;
                parent->ConvertRtoL( m_MechSurfIndx(), r, l );
            }

            if ( !( parent->CompTransCoordSysLMN( m_MechSurfIndx(), l, m_MechMLoc(), m_MechNLoc(), transMat )) )
            {
                revertCompTrans = true; // Any Geom without a surface reverts to the component matrix.
            }
        }

        if ( m_MechTransAttachFlag() == vsp::ATTACH_TRANS_COMP || revertCompTrans )
        {
            transMat.translatef( tempMat[12], tempMat[13], tempMat[14] );
        }

        m_MechAttachMatrix = transMat;
    }

    Matrix4d gmm = m_GearModelMatrix;
    gmm.affineInverse();

    m_MechAttachMatrix.postMult( gmm.data() );
}

void Bogie::BuildRetractMatrix( Matrix4d &ret_mat, vec3d &knee_pt, vec3d &knee_ax, double k, int isymm ) const
{
    vec3d axis = vec3d( m_MechXAxis(), m_MechYAxis(), m_MechZAxis() );
    axis.normalize();

    vec3d trunnion = vec3d( m_MechXLoc(), m_MechYLoc(), m_MechZLoc() );

    vec3d pivot = GetNominalPivotPoint( 0 );

    vec3d u = trunnion - pivot;

    vec3d u_dir = u;
    u_dir.normalize();
    vec3d n = cross( axis, u_dir );
    n.normalize();

    vec3d v = cross( u_dir, n );
    v.normalize();

    Matrix4d orientKnee;
    orientKnee.rotate( m_MechKneeAzimuthAngle() * M_PI / 180.0, u_dir );
    orientKnee.rotate( -m_MechKneeElevationAngle() * M_PI / 180.0, n );
    vec3d w = orientKnee.xformnorm( v );

    // SSA calculation for knee position
    double alpha = m_MechKneeDownAngle() * M_PI / 180.0;
    double beta = ( 1.0 - m_MechKneePos() ) * std::sin( alpha );
    double gamma = M_PI - alpha - std::asin( beta );

    Matrix4d knee_bend;
    knee_bend.translatev( trunnion );
    knee_bend.rotate( -gamma, axis ); // Negative is to match right hand rule
    knee_bend.translatev( -trunnion );

    vec3d k_pt = knee_bend.xform( pivot + u * m_MechKneePos() );
    vec3d k_ax = knee_bend.xformnorm( w );

    ret_mat.translatev( trunnion );
    ret_mat.rotate( -k * m_MechRetAngle() * M_PI / 180.0, axis ); // Negative is to match right hand rule
    ret_mat.translatev( -trunnion );

    knee_pt = ret_mat.xform( k_pt );
    knee_ax = ret_mat.xformnorm( k_ax );

    ret_mat.translatev( k_pt );
    ret_mat.rotate( -k * m_MechKneeAngle() * M_PI / 180.0, w ); // Negative is to match right hand rule
    ret_mat.translatev( -k_pt );

    ret_mat.translatev( GetContinuiousPivotPoint( 0, -k * m_MechStrutDL() ) );

    ret_mat.rotateX( k * m_MechRollAngle() );

    ret_mat.rotateY( -k * m_MechBogieAngle() );

    ret_mat.rotateZ( k * m_MechTwistAngle() );

    if ( isymm > 0 )
    {
        ret_mat.mirrory();
    }
}

void Bogie::UpdateRetract()
{
    m_StowTransform.loadIdentity();
    m_MechTransform.loadIdentity();

    if ( m_RetMode() == vsp::GEAR_STOWED_POSITION )
    {
        // These transformations get applied in reverse order.
        if (  m_StowAbsRelFlag() ==  vsp::REL )
        {
            // Apply normal translations
            m_StowTransform.translatef( m_StowXRelLoc(), m_StowYRelLoc(), m_StowZRelLoc() );

            // Apply rotations
            m_StowTransform.rotateX( m_StowXRelRot() );
            m_StowTransform.rotateY( m_StowYRelRot() );
            m_StowTransform.rotateZ( m_StowZRelRot() );

            // Apply Attached Matrix to Relative Matrix
            m_StowTransform.postMult( m_StowAttachMatrix.data() );

            // Update Absolute Parameters
            double tempMat[16];
            m_StowTransform.getMat( tempMat );
            m_StowXLoc = tempMat[12];
            m_StowYLoc = tempMat[13];
            m_StowZLoc = tempMat[14];

            vec3d angles = m_StowTransform.getAngles();
            m_StowXRot = angles.x();
            m_StowYRot = angles.y();
            m_StowZRot = angles.z();
        }
        else if ( m_StowAbsRelFlag() ==  vsp::ABS )
        {
            // Apply normal translations
            m_StowTransform.translatef( m_StowXLoc(), m_StowYLoc(), m_StowZLoc() );

            // Apply rotations
            m_StowTransform.rotateX( m_StowXRot() );
            m_StowTransform.rotateY( m_StowYRot() );
            m_StowTransform.rotateZ( m_StowZRot() );

            // Update Relative Parameters
            Matrix4d attachedMat = m_StowAttachMatrix;
            double tempMat[16];
            attachedMat.affineInverse();
            attachedMat.matMult( m_StowTransform.data() );
            attachedMat.getMat( tempMat );
            m_StowXRelLoc = tempMat[12];
            m_StowYRelLoc = tempMat[13];
            m_StowZRelLoc = tempMat[14];

            vec3d angles = attachedMat.getAngles();
            m_StowXRelRot = angles.x();
            m_StowYRelRot = angles.y();
            m_StowZRelRot = angles.z();
        }

        m_PivotPtVec.clear();
    }
    else // vsp::GEAR_MECHANISM
    {
        // These transformations get applied in reverse order.
        if (  m_MechAbsRelFlag() ==  vsp::REL )
        {
            // Apply normal translations
            m_MechTransform.translatef( m_MechXRelLoc(), m_MechYRelLoc(), m_MechZRelLoc() );

            // Apply Attached Matrix to Relative Matrix
            m_MechTransform.postMult( m_MechAttachMatrix.data() );

            // Update Absolute Parameters
            double tempMat[16];
            m_MechTransform.getMat( tempMat );
            m_MechXLoc = tempMat[12];
            m_MechYLoc = tempMat[13];
            m_MechZLoc = tempMat[14];
        }
        else if ( m_MechAbsRelFlag() ==  vsp::ABS )
        {
            // Apply normal translations
            m_MechTransform.translatef( m_MechXLoc(), m_MechYLoc(), m_MechZLoc() );

            // Update Relative Parameters
            Matrix4d attachedMat = m_MechAttachMatrix;
            double tempMat[16];
            attachedMat.affineInverse();
            attachedMat.matMult( m_MechTransform.data() );
            attachedMat.getMat( tempMat );
            m_MechXRelLoc = tempMat[12];
            m_MechYRelLoc = tempMat[13];
            m_MechZRelLoc = tempMat[14];
        }

        m_Axis.set_xyz( m_MechXAxis(), m_MechYAxis(), m_MechZAxis() );
        m_Axis.normalize();

        m_StrutAttachPt.set_xyz( m_MechXLoc(), m_MechYLoc(), m_MechZLoc() );

        m_PivotPtVec.clear();
        m_KneePtVec.clear();
        m_KneeAxVec.clear();

        vector < double > kvec;

        ParmContainer* pc = GetParentContainerPtr();
        if ( pc )
        {
            GearGeom * gg = dynamic_cast< GearGeom* >( pc );
            if ( gg )
            {
                int gear_config = gg->m_GearConfigMode();

                if ( gear_config == vsp::GEAR_CONFIGURATION_DOWN ||
                     gear_config == vsp::GEAR_CONFIGURATION_UP_AND_DOWN ||
                     gear_config == vsp::GEAR_CONFIGURATION_ALL )
                {
                    kvec.push_back( 0.0 );
                }

                if ( gear_config == vsp::GEAR_CONFIGURATION_UP ||
                     gear_config == vsp::GEAR_CONFIGURATION_UP_AND_DOWN ||
                     gear_config == vsp::GEAR_CONFIGURATION_ALL )
                {
                    kvec.push_back( 1.0 );
                }

                if ( gear_config == vsp::GEAR_CONFIGURATION_INTERMEDIATE ||
                     gear_config == vsp::GEAR_CONFIGURATION_ALL )
                {
                    kvec.push_back( m_MechKRetract() );
                }
            }
        }

        for ( int i = 0; i < kvec.size(); i++ )
        {
            Matrix4d retractMat;
            vec3d knee_pt, knee_ax;
            BuildRetractMatrix( retractMat, knee_pt, knee_ax, kvec[i], 0 );

            m_PivotPtVec.push_back( retractMat.xform( vec3d() ) );
            m_KneePtVec.push_back( knee_pt );
            m_KneeAxVec.push_back( knee_ax );
        }
    }


    m_StowSymmTransform = m_StowTransform;
    m_StowSymmTransform.mirrory();

}

void Bogie::BackCalculateRetract()
{
    if ( m_RetMode() == vsp::GEAR_STOWED_POSITION )
    {
        m_MechParentID = "";
        m_MechSurfIndx = 0;

        m_MechAbsRelFlag = vsp::REL;
        m_MechTransAttachFlag = vsp::ATTACH_TRANS_NONE;

        m_MechULoc = 0.0;
        m_MechU0NLoc = 0.0;
        m_MechU01 = true;
        m_MechWLoc = 0.0;

        m_MechRLoc = 0.0;
        m_MechR01 = true;
        m_MechR0NLoc = 0.0;
        m_MechSLoc = 0.5;
        m_MechTLoc = 0.5;

        m_MechLLoc = 0.0;
        m_MechL01 = true;
        m_MechL0LenLoc = 0.0;
        m_MechMLoc = 0.5;
        m_MechNLoc = 0.5;

        m_MechEtaLoc = 0.0;

        m_MechKneePos = 0.0;
        m_MechKneeAngle = 0.0;
        m_MechKneeAzimuthAngle = 0.0;
        m_MechKneeElevationAngle = 0.0;
        m_MechKneeDownAngle = 180.0;

        m_MechTwistAngle = 0.0;
        m_MechRollAngle = 0.0;
        m_MechBogieAngle = 0.0;

        m_MechStrutDL = 0.0;



        SimpleFeatureTess sample_pts;
        sample_pts.m_ptline.push_back( { vec3d(0, 0, 0), vec3d( 0, 1, 0) } );
        vector <SimpleFeatureTess> out;
        TireToBogie( sample_pts, out, vsp::GEAR_CONFIGURATION_UP_AND_DOWN );

        vec3d a = out[0].m_ptline[0][0];
        vec3d b = out[0].m_ptline[0][1];
        vec3d aprm = out[1].m_ptline[0][0];
        vec3d bprm = out[1].m_ptline[0][1];

        vec3d u = b - a;
        vec3d uprm = bprm - aprm;

        vec3d k_general = cross( a - aprm, u - uprm );
        vec3d k;
        if ( k_general.mag() > 1e-10 )
        {
            k = k_general;
        }
        else
        {
            k = cross( u, uprm );
        }
        k.normalize();

        m_MechXAxis = k.x();
        m_MechYAxis = k.y();
        m_MechZAxis = k.z();

        vec3d u_perp = u - dot( u, k ) * k;
        vec3d uprm_perp = uprm - dot( uprm, k ) * k;
        double L2 = u_perp.magsq();

        double theta;
        if ( L2 > 1e-20 )
        {
            double cos_theta = dot( u_perp, uprm_perp ) / L2;
            double sin_theta = dot( cross( u_perp, uprm_perp ), k ) / L2;
            theta = atan2( sin_theta, cos_theta );
        }
        else
        {
            theta = angle( u, uprm );
        }


        if ( theta != 0.0 && theta != M_PI )
        {
            m_MechRetAngle = theta * 180.0 / M_PI;

            vec3d c = 0.5 * ( aprm + a );

            vec3d n = aprm - a;

            vec3d p = c + ( 1.0 / ( 2.0 * tan( theta * 0.5 ) ) ) * cross( k, n );

            m_MechXLoc = p.x();
            m_MechYLoc = p.y();
            m_MechZLoc = p.z();

            // Only for vsp::ATTACH_TRANS_NONE && vsp::ATTACH_ROT_NONE
            Matrix4d mechAttachMat;
            mechAttachMat.translatev( GetNominalPivotPoint( 0 ) );

            Matrix4d mechMat;
            mechMat.translatev( p );

            // Update Relative Parameters
            Matrix4d attachedMat = mechAttachMat;

            attachedMat.affineInverse();
            attachedMat.matMult( mechMat.data() );
            double tempMat[16];
            attachedMat.getMat( tempMat );
            m_MechXRelLoc = tempMat[12];
            m_MechYRelLoc = tempMat[13];
            m_MechZRelLoc = tempMat[14];
        }
        else
        {
            m_MechYAxis = 1.0;

            m_MechRetAngle = theta * 180.0 / M_PI;

            m_MechXRelLoc = 0.0;
            m_MechYRelLoc = 0.0;
            m_MechZRelLoc = 0.0;

            // Apply normal translations
            Matrix4d mechAttachMat;
            mechAttachMat.translatef( m_MechXRelLoc(), m_MechYRelLoc(), m_MechZRelLoc() );

            // Apply Attached Matrix to Relative Matrix
            Matrix4d attachedMat;
            attachedMat.translatev( GetNominalPivotPoint( 0 ) );
            mechAttachMat.postMult( attachedMat.data() );

            // Update Absolute Parameters
            double tempMat[16];
            mechAttachMat.getMat( tempMat );
            m_MechXLoc = tempMat[12];
            m_MechYLoc = tempMat[13];
            m_MechZLoc = tempMat[14];
        }
    }
    else // vsp::GEAR_MECHANISM
    {
        m_StowParentID = "";
        m_StowSurfIndx = 0;

        m_StowAbsRelFlag = vsp::REL;
        m_StowTransAttachFlag = vsp::ATTACH_TRANS_NONE;
        m_StowRotAttachFlag = vsp::ATTACH_ROT_NONE;

        m_StowULoc = 0.0;
        m_StowU01 = true;
        m_StowU0NLoc = 0;
        m_StowWLoc = 0.0;

        m_StowRLoc = 0.0;
        m_StowR01 = true;
        m_StowR0NLoc = 0;
        m_StowSLoc = 0.5;
        m_StowTLoc = 0.5;

        m_StowLLoc = 0.0;
        m_StowL01 = true;
        m_StowL0LenLoc = 0;
        m_StowMLoc = 0.5;
        m_StowNLoc = 0.5;

        m_StowEtaLoc = 0.0;

        // Only for vsp::ATTACH_TRANS_NONE && vsp::ATTACH_ROT_NONE
        Matrix4d stowAttachMat;
        stowAttachMat.translatev( GetNominalPivotPoint( 0 ) );

        // Apply normal translations
        Matrix4d stowMat;
        vec3d knee_pt, knee_ax;
        BuildRetractMatrix( stowMat, knee_pt, knee_ax, /* up */ 1.0, 0 );

        double tempMat[16];
        stowMat.getMat( tempMat );

        m_StowXLoc = tempMat[12];
        m_StowYLoc = tempMat[13];
        m_StowZLoc = tempMat[14];

        vec3d angles = stowMat.getAngles();
        m_StowXRot = angles.x();
        m_StowYRot = angles.y();
        m_StowZRot = angles.z();

        // Update Relative Parameters
        Matrix4d attachedMat = stowAttachMat;

        attachedMat.affineInverse();
        attachedMat.matMult( stowMat.data() );
        attachedMat.getMat( tempMat );
        m_StowXRelLoc = tempMat[12];
        m_StowYRelLoc = tempMat[13];
        m_StowZRelLoc = tempMat[14];

        angles = attachedMat.getAngles();
        m_StowXRelRot = angles.x();
        m_StowYRelRot = angles.y();
        m_StowZRelRot = angles.z();
    }
}

void Bogie::Update()
{
    UpdateParms();

    UpdateStowAttachParms();
    UpdateMechAttachParms();

    DeactivateStowXForms();
    DeactivateMechXForms();

    UpdateRetractAttach();

    UpdateRetract();

    BackCalculateRetract();

    if ( m_TireDirty )
    {
        UpdateTireCurve();

        m_TireSurface.CreateBodyRevolution( m_TireProfile, true, 1 );
        m_TireSurface.SetMagicVParm( false );
        m_TireSurface.SetHalfBOR( true );
        m_TireSurface.FlipNormal();

        m_TireSurface.BuildFeatureLines( false );

        if ( m_TireMode() == vsp::TIRE_TRA ||
             m_TireMode() == vsp::TIRE_FAIR_FLANGE ||
             m_TireMode() == vsp::TIRE_FAIR_WHEEL )
        {
            m_GrownTireSurface.CreateBodyRevolution( m_GrownTireProfile, true, 1 );
            m_GrownTireSurface.SetMagicVParm( false );
            m_GrownTireSurface.SetHalfBOR( true );
            m_GrownTireSurface.FlipNormal();

            m_GrownTireSurface.BuildFeatureLines( false );

            m_ClearanceSurface.CreateBodyRevolution( m_ClearanceProfile, true, 1 );
            m_ClearanceSurface.SetMagicVParm( false );
            m_ClearanceSurface.SetHalfBOR( true );
            m_ClearanceSurface.FlipNormal();

            m_ClearanceSurface.BuildFeatureLines( false );
        }
        else
        {
            m_GrownTireSurface = m_TireSurface;
            m_ClearanceSurface = m_TireSurface;
        }
    }

    m_TireDirty = false;
}

void Bogie::UpdateRetractAttach()
{
    ComposeStowAttachMatrix();
    ComposeMechAttachMatrix();
}

void Bogie::UpdateTess()
{
    if ( m_TireTessDirty )
    {
        ParmContainer* pc = GetParentContainerPtr();
        if ( pc )
        {
            GearGeom * gg = dynamic_cast< GearGeom* >( pc );
            if ( gg )
            {
                gg->UpdateTess( m_TireSurface, false, false, m_TireTess, m_TireFeatureTess );

                if ( m_TireMode() == vsp::TIRE_TRA ||
                     m_TireMode() == vsp::TIRE_FAIR_FLANGE ||
                     m_TireMode() == vsp::TIRE_FAIR_WHEEL )
                {
                    m_GrownTireSurface.CopyNonSurfaceData( m_TireSurface );
                    gg->UpdateTess( m_GrownTireSurface, false, false, m_GrownTireTess, m_GrownTireFeatureTess );
                    m_ClearanceSurface.CopyNonSurfaceData( m_TireSurface );
                    gg->UpdateTess( m_ClearanceSurface, false, false, m_ClearanceTess, m_ClearanceFeatureTess );
                }
                else
                {
                    m_GrownTireTess = m_TireTess;
                    m_GrownTireFeatureTess = m_TireFeatureTess;
                    m_ClearanceTess = m_TireTess;
                    m_ClearanceFeatureTess = m_TireFeatureTess;
                }
            }
        }
    }
    m_TireTessDirty = false;
}

void Bogie::UpdateDrawObj( const Matrix4d &relTrans )
{
    m_SuspensionTravelLinesDO.m_PntVec.clear();
    m_SuspensionTravelPointsDO.m_PntVec.clear();

    m_AxisDO.m_PntVec.clear();
    m_AxisArrowDO.m_PntVec.clear();
    m_AxisArrowDO.m_NormVec.clear();
    m_AxisCircleDO.m_PntVec.clear();
    m_StrutDO.m_PntVec.clear();

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

    m_AxisDO.m_GeomID = m_ID + "MAxis";
    m_AxisDO.m_Screen = DrawObj::VSP_MAIN_SCREEN;
    m_AxisDO.m_LineWidth = 2.0;
    m_AxisDO.m_Type = DrawObj::VSP_LINES;
    m_AxisDO.m_LineColor = vec3d( 0, 0, 0 );
    m_AxisDO.m_GeomChanged = true;

    m_AxisCircleDO.m_GeomID = m_ID + "MCircle";
    m_AxisCircleDO.m_Screen = DrawObj::VSP_MAIN_SCREEN;
    m_AxisCircleDO.m_LineWidth = 2.0;
    m_AxisCircleDO.m_Type = DrawObj::VSP_LINES;
    m_AxisCircleDO.m_LineColor = vec3d( 0, 0, 0 );
    m_AxisCircleDO.m_GeomChanged = true;

    m_AxisArrowDO.m_GeomID = m_ID + "MArrow";
    m_AxisArrowDO.m_Screen = DrawObj::VSP_MAIN_SCREEN;
    m_AxisArrowDO.m_LineWidth = 1.0;
    m_AxisArrowDO.m_Type = DrawObj::VSP_SHADED_TRIS;
    m_AxisArrowDO.m_GeomChanged = true;

    for ( int i = 0; i < 4; i++ )
    {
        m_AxisArrowDO.m_MaterialInfo.Ambient[i] = 0.2f;
        m_AxisArrowDO.m_MaterialInfo.Diffuse[i] = 0.1f;
        m_AxisArrowDO.m_MaterialInfo.Specular[i] = 0.7f;
        m_AxisArrowDO.m_MaterialInfo.Emission[i] = 0.0f;
    }
    m_AxisArrowDO.m_MaterialInfo.Diffuse[3] = 0.5f;
    m_AxisArrowDO.m_MaterialInfo.Shininess = 5.0f;


    m_StrutDO.m_GeomID = m_ID + "MStrut";
    m_StrutDO.m_Screen = DrawObj::VSP_MAIN_SCREEN;
    m_StrutDO.m_LineWidth = 2.0;
    m_StrutDO.m_Type = DrawObj::VSP_LINES;
    m_StrutDO.m_LineColor = vec3d( 0, 0, 0 );
    m_StrutDO.m_GeomChanged = true;

    int isymm = 0;

    m_SuspensionTravelLinesDO.m_PntVec.push_back( relTrans.xform( GetPivotPoint( 0, vsp::GEAR_SUSPENSION_EXTENDED ) ) );
    m_SuspensionTravelLinesDO.m_PntVec.push_back( relTrans.xform( GetPivotPoint( 0, vsp::GEAR_SUSPENSION_COMPRESSED ) ) );

    m_SuspensionTravelPointsDO.m_PntVec.push_back( relTrans.xform( GetPivotPoint( 0, vsp::GEAR_SUSPENSION_EXTENDED ) ) );
    m_SuspensionTravelPointsDO.m_PntVec.push_back( relTrans.xform( GetPivotPoint( 0, vsp::GEAR_SUSPENSION_NOMINAL ) ) );
    m_SuspensionTravelPointsDO.m_PntVec.push_back( relTrans.xform( GetPivotPoint( 0, vsp::GEAR_SUSPENSION_COMPRESSED ) ) );

    if ( !m_PivotPtVec.empty() )
    {
        double axlen = 1.0;

        Vehicle *veh = VehicleMgr.GetVehicle();
        if ( veh )
        {
            axlen = veh->m_AxisLength();
        }

        vec3d axstart = relTrans.xform( m_StrutAttachPt );
        vec3d axend = relTrans.xform( m_StrutAttachPt + axlen * m_Axis );

        vec3d u = axend - axstart;
        MakeCircleArrow(axstart + 0.6 * u, u, 0.5 * axlen, 0.5 * axlen, m_AxisCircleDO, m_AxisArrowDO );
        m_AxisArrowDO.m_NormVec = vector <vec3d> ( m_AxisArrowDO.m_PntVec.size() );

        MakeDashedLine( axstart,  axend, 4, m_AxisDO.m_PntVec );

        for ( int i = 0; i < m_PivotPtVec.size(); i++ )
        {
            m_StrutDO.m_PntVec.push_back( axstart );
            m_StrutDO.m_PntVec.push_back( relTrans.xform( m_KneePtVec[i] ) );
            m_StrutDO.m_PntVec.push_back( relTrans.xform( m_KneePtVec[i] ) );
            m_StrutDO.m_PntVec.push_back( relTrans.xform( m_PivotPtVec[i] ) );

            vec3d knee_axstart = relTrans.xform( m_KneePtVec[i] );
            vec3d knee_axend = relTrans.xform( m_KneePtVec[i] + axlen * m_KneeAxVec[i] );

            MakeDashedLine( knee_axstart,  knee_axend, 4, m_AxisDO.m_PntVec );
        }
    }

    // Visualize gear contact points.
    // double bogietheta = 0;
    // // if ( !m_DrawNominal() )
    // // {
    // //     bogietheta = m_BogieTheta();
    // // }
    // m_SuspensionTravelPointsDO.m_PntVec.push_back( relTrans.xform( GetMeanContactPoint( 0,  vsp::TIRE_STATIC_LODED_CONTACT, vsp::GEAR_SUSPENSION_NOMINAL, bogietheta * M_PI / 180.0 ) ) );
    // if ( m_Symmetrical() )
    // {
    //     m_SuspensionTravelPointsDO.m_PntVec.push_back( relTrans.xform( GetMeanContactPoint( 1,  vsp::TIRE_STATIC_LODED_CONTACT, vsp::GEAR_SUSPENSION_NOMINAL, bogietheta * M_PI / 180.0 ) ) );
    // }

    if ( m_Symmetrical() )
    {
        isymm++;
    }
}

void Bogie::LoadDrawObjs( vector< DrawObj* > & draw_obj_vec )
{
    draw_obj_vec.push_back( &m_AxisDO );
    draw_obj_vec.push_back( &m_AxisCircleDO );
    draw_obj_vec.push_back( &m_AxisArrowDO );
    draw_obj_vec.push_back( &m_StrutDO );
    draw_obj_vec.push_back( &m_SuspensionTravelLinesDO );
    draw_obj_vec.push_back( &m_SuspensionTravelPointsDO );
}

xmlNodePtr Bogie::EncodeXml( xmlNodePtr & node )
{
    xmlNodePtr pcnode = ParmContainer::EncodeXml( node );

    xmlNodePtr bogie_node = xmlNewChild( node, nullptr, BAD_CAST"Bogie", nullptr );

    if ( bogie_node )
    {
        XmlUtil::AddStringNode( bogie_node, "StowParentID", m_StowParentID );
        XmlUtil::AddStringNode( bogie_node, "MechParentID", m_MechParentID );
    }

    return pcnode;
}

xmlNodePtr Bogie::DecodeXml( xmlNodePtr & node )
{
    xmlNodePtr pcnode = ParmContainer::DecodeXml( node );

    xmlNodePtr bogie_node = XmlUtil::GetNode( node, "Bogie", 0 );

    if ( bogie_node )
    {
        m_StowParentID = ParmMgr.RemapID( XmlUtil::FindString( bogie_node, "StowParentID", m_StowParentID ) );
        m_MechParentID = ParmMgr.RemapID( XmlUtil::FindString( bogie_node, "MechParentID", m_MechParentID ) );
    }

    return pcnode;
}

void Bogie::SetStowParentID( const string &id )
{
    m_StowParentID = id ;

    ParmContainer* pc = GetParentContainerPtr();
    if ( pc )
    {
        GearGeom * gg = dynamic_cast< GearGeom* >( pc );
        if ( gg )
        {
            gg->UpdateParents();
            gg->Update();
        }
    }
}

void Bogie::SetMechParentID( const string &id )
{
    m_MechParentID = id ;

    ParmContainer* pc = GetParentContainerPtr();
    if ( pc )
    {
        GearGeom * gg = dynamic_cast< GearGeom* >( pc );
        if ( gg )
        {
            gg->UpdateParents();
            gg->Update();
        }
    }
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
        case vsp::TIRE_GROWTH_CONTACT:
            return m_DGModel() * 0.5;
            break;
        case vsp::TIRE_FLAT_CONTACT:
            return m_DFlangeModel() * 0.5;
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

vec3d Bogie::GetContinuiousSuspensionDeflection( int isymm, double d ) const
{
    return GetCompressionUnitDirection( isymm ) * d;
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

vec3d Bogie::GetContinuiousPivotPoint( int isymm, double d ) const
{
    return GetNominalPivotPoint( isymm ) + GetContinuiousSuspensionDeflection( isymm, d );
}

double Bogie::GetAxleArm() const
{
    return 0.5 * ( m_NTandem() - 1 ) * m_Pitch();
}

double Bogie::GetBogieSemiWidth() const
{
    return 0.5 * ( m_NAcross() - 1 ) * m_Spacing();
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

vec3d Bogie::GetSideContactPoint( int isymm, int suspensionmode, int tiremode, double thetabogie, double thetawheel, int ysign ) const
{
    vec3d v( 0, ysign * GetBogieSemiWidth(), 0 );
    vec3d origin;
    if ( thetawheel > 0 )
    {
        origin = GetAftContactPoint( isymm, suspensionmode, tiremode, thetabogie, thetawheel );
    }
    else if ( thetawheel < 0 )
    {
        origin = GetFwdContactPoint( isymm, suspensionmode, tiremode, thetabogie, thetawheel );
    }
    else
    {
        origin = GetMeanContactPoint( isymm, tiremode, suspensionmode, thetabogie );
    }
    return origin + v;
}

void Bogie::AppendMainSurf( vector < VspSurf > &surfvec, int gear_config ) const
{
    TireToBogie( m_TireSurface, surfvec, gear_config );
}

bool Bogie::IsStowParentJoint()
{
    Vehicle *veh = VehicleMgr.GetVehicle();
    GeomBase* parentPtr = veh->FindGeom( m_StowParentID );
    if ( parentPtr )
    {
        HingeGeom* hingeParentPtr = dynamic_cast < HingeGeom* > ( parentPtr );
        if ( hingeParentPtr )
        {
            return true;
        }
    }
    return false;
}

bool Bogie::IsMechParentJoint()
{
    Vehicle *veh = VehicleMgr.GetVehicle();
    GeomBase* parentPtr = veh->FindGeom( m_MechParentID );
    if ( parentPtr )
    {
        HingeGeom* hingeParentPtr = dynamic_cast < HingeGeom* > ( parentPtr );
        if ( hingeParentPtr )
        {
            return true;
        }
    }
    return false;
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

    m_GearConfigMode.Init( "GearConfigMode", "Gear", this, vsp::GEAR_CONFIGURATION_DOWN, vsp::GEAR_CONFIGURATION_DOWN, vsp::NUM_GEAR_CONFIGURATION_MODES - 1 );

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

void GearGeom::SetDirtyFlags( Parm* parm_ptr )
{
    if ( !parm_ptr )
    {
        return;
    }

    Geom::SetDirtyFlags( parm_ptr );

    string gname = parm_ptr->GetGroupName();
    string pname = parm_ptr->GetName();

    bool tessdirty = false;
    bool tiredirty = false;

    if ( ( pname == string("Tess_U") || pname == string("Tess_W") ) )
    {
        tessdirty = true;

    }
    else if ( parm_ptr == &m_ModelLenUnits )
    {
        tiredirty = true;
    }


    if ( tessdirty )
    {
        int nbogies = m_Bogies.size();

        for ( int i = 0; i < nbogies; i++ )
        {
            if ( m_Bogies[i] )
            {
                m_Bogies[i]->m_TireTessDirty = true;
            }
        }
    }

    if ( tiredirty )
    {
        int nbogies = m_Bogies.size();

        for ( int i = 0; i < nbogies; i++ )
        {
            if ( m_Bogies[i] )
            {
                m_Bogies[i]->m_TireDirty = true;
            }
        }
    }
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

    int multiplier = 1;
    if ( m_GearConfigMode() == vsp::GEAR_CONFIGURATION_UP_AND_DOWN )
    {
        multiplier = 2;
    }

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
            nsurf += multiplier * m_Bogies[i]->GetNumSurf();
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
                m_Bogies[i]->AppendMainSurf( m_MainSurfVec, m_GearConfigMode() );
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

void GearGeom::UpdateXForm()
{
    Geom::UpdateXForm();

    int nbogies = m_Bogies.size();

    for ( int i = 0; i < nbogies; i++ )
    {
        if ( m_Bogies[i] )
        {
            m_Bogies[i]->m_GearModelMatrix = m_ModelMatrix;
        }
    }
}

void GearGeom::UpdateFeatureLines()
{
    // Tire Feature Lines are kept up to date.
    if ( m_IncludeNominalGroundPlane() )
    {
        m_MainSurfVec[0].BuildFeatureLines( m_ForceXSecFlag );
    }
}

void GearGeom::UpdateMainTessVec()
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


                m_Bogies[i]->UpdateTess();

                m_Bogies[i]->TireToBogie( m_Bogies[i]->m_TireTess, m_MainTessVec, m_GearConfigMode() );
                m_Bogies[i]->TireToBogie( m_Bogies[i]->m_TireFeatureTess, m_MainFeatureTessVec, m_GearConfigMode() );
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
                CreateDegenGeom( m_Bogies[i]->m_TireSurface, 0, degenGeom, true, 1 );

                m_Bogies[i]->TireToBogie( degenGeom, m_MainDegenGeomPreviewVec, m_GearConfigMode() );
            }
        }
    }

    if ( nmain >= 1 )
    {
        if ( m_IncludeNominalGroundPlane() )
        {
            // Update degen preview for ground plane
            CreateDegenGeom( m_MainSurfVec[0], 0, m_MainDegenGeomPreviewVec[0], true, 1 );
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

    UpdateParents();
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

    UpdateParents();
    return label_root_node;
}

void GearGeom::AddLinkableParms( vector< string > & linkable_parm_vec, const string & link_container_id )
{
    Geom::AddLinkableParms( linkable_parm_vec );

    for ( int i = 0 ; i < ( int )m_Bogies.size() ; i++ )
    {
        m_Bogies[i]->AddLinkableParms( linkable_parm_vec, m_ID );
    }
}

void GearGeom::ChangeID( const string &id )
{
    Geom::ChangeID( id );

    for ( int i = 0; i < m_Bogies.size(); i++ )
    {
        m_Bogies[i]->SetParentContainer( id );
    }
}

void GearGeom::UpdateParents()
{
    m_SurfDirty = true;

    Vehicle* veh = VehicleMgr.GetVehicle();

    if ( !veh )
    {
        return;
    }

    vector < string > parent_vec;
    parent_vec.reserve( m_Bogies.size() );
    for ( int i = 0; i < m_Bogies.size(); i++ )
    {
        parent_vec.push_back( m_Bogies[i]->GetStowParentID() );
        parent_vec.push_back( m_Bogies[i]->GetMechParentID() );
    }

    std::sort( parent_vec.begin(), parent_vec.end() );
    parent_vec.erase(std::unique( parent_vec.begin(), parent_vec.end()), parent_vec.end() );

    // Serialize m_ParmIDs into single long string.
    string str = string_vec_serialize( parent_vec );
    // Calculate hash to detect changes in m_ParmIDs
    std::size_t str_hash = std::hash < std::string >{}( str );

    // Relies on currency of m_ParmIDs by UpdateVarBrowser()
    if ( str_hash != m_ParentHash )
    {
        // Remove parents.
        for ( int i = 0; i < m_ParentVec.size(); i++ )
        {
            Geom * g = veh->FindGeom( m_ParentVec[i] );
            if ( g )
            {
                g->RemoveStepChildID( m_ID );
            }
        }

        // Add to parents.
        for ( int i = 0; i < parent_vec.size(); i++ )
        {
            Geom * g = veh->FindGeom( parent_vec[i] );
            if ( g )
            {
                g->AddStepChildID( m_ID );
            }
        }

        m_ParentVec = parent_vec;
        m_ParentHash = str_hash;
    }
}

void GearGeom::DelAllBogies()
{
    for( int i = 0; i < ( int )m_Bogies.size(); i++ )
    {
        delete m_Bogies[i];
    }
    m_Bogies.clear();
    m_CurrBogieIndex = -1;
    UpdateParents();
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

    if ( i > m_Bogies.size() - 1 )
    {
        SetCurrBogieIndex( m_Bogies.size() - 1 );
    }

    UpdateParents();
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

void GearGeom::BuildOnePtBasis( const string &cp1, int isymm1, int suspension1, int tire1,
                                double thetabogie, double thetawheel, double thetaroll, Matrix4d &mat, vec3d &p1 )
{
    mat.loadIdentity();

    vec3d axis, normal;
    int ysign;
    if ( GetOnePtSideContactPtAxisNormal( cp1, isymm1, suspension1, tire1, thetabogie, thetawheel, thetaroll, p1, axis, normal, ysign) )
    {
        vec3d y = cross( normal, axis );
        y.normalize();

        mat.translatev( p1 );
        mat.setBasis( axis, y, normal );
    }
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
        if ( z[ 2 ] < 0 ) // Make sure normal is generally pointing 'up'.
        {
            z = -z;
        }

        vec3d y( 0, 1, 0 );

        if ( std::abs( dot( y, z ) ) < 1.0 ) // z is not parallel to y (good).
        {
        }
        else
        {
            y.set_xyz( 0, 0, 1 );
        }


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
        if ( z[ 2 ] < 0 ) // Make sure normal is generally pointing 'up'.
        {
            z = -z;
        }

        vec3d y( 0, 1, 0 );

        if ( std::abs( dot( y, z ) ) < 1.0 ) // z is not parallel to y (good).
        {
        }
        else
        {
            y.set_xyz( 0, 0, 1 );
        }

        vec3d x = cross( y, z );
        x.normalize();
        y = cross( z, x );
        y.normalize();

        mat.translatev( pcen );
        mat.setBasis( x, y, z );
    }
}

void GearGeom::BuildThreePtOffAxisBasis( const string &cp1, int isymm1, int suspension1, int tire1,
                                const string &cp2, int isymm2, int suspension2, int tire2,
                                const string &cp3, int isymm3, int suspension3, int tire3,
                                double mainoffset,
                                Matrix4d &mat )
{
    mat.loadIdentity();

    const Bogie *b1 = GetBogie( cp1 );
    const Bogie *b2 = GetBogie( cp2 );
    const Bogie *b3 = GetBogie( cp3 );

    if ( b1 && b2 && b3 )
    {
        const vec3d p1 = b1->GetMeanContactPoint( isymm1, tire1, suspension1, 0.0 );
        const vec3d p2 = b2->GetMeanContactPoint( isymm2, tire2, suspension2, 0.0 );
        const vec3d p3 = b3->GetMeanContactPoint( isymm3, tire3, suspension3, 0.0 );

        const vec3d v12 = p2 - p1;
        const vec3d v13 = p3 - p1;

        vec3d normal = cross( v12, v13 );
        normal.normalize();

        // Check that plane points mostly 'up' in GearGeom coordinate system.  Nominal ground plane will point
        // straight up in these coordinates.  Changing the order of the contact points can change the orientation
        // of the normal vector.  This sign check prevents us from requiring cw/ccw ordering by the user.
        if ( normal.z() < 0 )
        {
            normal = -normal;
        }

        // Find vector connecting main gear contact points.
        vec3d v23 = p3 - p2;
        v23.normalize();

        // Find mean of main gear contact points.
        vec3d pmaincen = ( p2 + p3 ) * 0.5;

        // Find wheelbase
        double wb = dist( p1, pmaincen );

        // Trig to resolve specified offset measured perpendicular to nominal track.
        // delta should be slightly larger than mainoffset
        // double delta = wb * sin( atan( mainoffset / wb ) );
        // Alternate equivalent formulation with less trig.
        double o = mainoffset / wb;
        double delta = mainoffset / sqrt( 1.0 + o * o );

        // Offset mean main point along the line between the main gear contact points.
        vec3d pmainoff = pmaincen + v23 * delta;

        // X is in the ground plane by construction.
        // All points on line between main gear contact points are on ground plane.
        vec3d x = pmainoff - p1;
        x.normalize();

        // Find rotated y direction.
        vec3d y = cross( normal, x );
        y.normalize();

        // Translate so nose gear is origin
        mat.translatev( p1 );
        // Set rotated basis
        mat.setBasis( x, y, normal );
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
        p1 = b1->GetAftContactPoint( isymm1, suspension1, tire1, thetabogie, thetawheel );
        p2 = b2->GetAftContactPoint( isymm2, suspension2, tire2, thetabogie, thetawheel );
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
        p1 = b1->GetFwdContactPoint( isymm1, suspension1, tire1, thetabogie, thetawheel );
        p2 = b2->GetFwdContactPoint( isymm2, suspension2, tire2, thetabogie, thetawheel );
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

bool GearGeom::GetTwoPtSideContactPtsNormal( const string &cp1, int isymm1, int suspension1, int tire1,
                                             const string &cp2, int isymm2, int suspension2, int tire2,
                                             vec3d &p1, vec3d &p2, vec3d &normal ) const
{
    const Bogie *b1 = GetBogie( cp1 );
    const Bogie *b2 = GetBogie( cp2 );
    if ( b1 && b2 )
    {
        // Grab mean contact points
        p1 = b1->GetMeanContactPoint( isymm1, tire1, suspension1, /* thetabogie */ 0.0 );
        p2 = b2->GetMeanContactPoint( isymm2, tire2, suspension2, /* thetabogie */ 0.0 );

        // Assign ysign to match wheel furthest from center line.
        int ysign = 0;
        if ( std::abs( p1.y() ) > std::abs( p2.y() ) )
        {
            ysign = sgn( p1.y() );
        }
        else
        {
            ysign = sgn( p2.y() );
        }

        p1 = b1->GetSideContactPoint( isymm1, suspension1, tire1, /* thetabogie */ 0.0, /* thetawheel */ 0.0, ysign);
        p2 = b2->GetSideContactPoint( isymm2, suspension2, tire2, /* thetabogie */ 0.0, /* thetawheel */ 0.0, ysign);


        vec3d v12 = p2 - p1;
        const vec3d nnom( 0, 0, 1 );
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

        return true;
    }
    return false;
}

bool GearGeom::GetOnePtSideContactPtAxisNormal( const string &cp1, int isymm1, int suspension1, int tire1,
                                                double thetabogie, double thetawheel, double thetaroll, vec3d &p1, vec3d &axis, vec3d &normal, int &ysign ) const
{
    const Bogie *b1 = GetBogie( cp1 );
    if ( b1 )
    {
        // Grab mean contact point to determine ysign
        p1 = b1->GetMeanContactPoint( isymm1, tire1, suspension1, 0 );

        // Assign ysign to match wheel position relative to center line.
        ysign = sgn( p1.y() );

        p1 = b1->GetSideContactPoint( isymm1, suspension1, tire1, thetabogie, thetawheel, ysign);

        vec3d y( 0, 1, 0 );
        axis.set_xyz( 1, 0, 0 );
        normal.set_xyz( 0, 0, 1 );

        Matrix4d mat;
        if ( b1->m_NTandem() > 1 )
        {
            mat.rotate( thetabogie, y );
        }
        mat.rotate( thetawheel, y );
        mat.rotate( thetaroll, axis );

        axis = mat.xformnorm( axis );
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

bool GearGeom::GetSteerAngle( const string &cp1, const string &cp2, const string &cp3, int &isteer, double &steerangle ) const
{
    const Bogie *b1 = GetBogie( cp1 );
    const Bogie *b2 = GetBogie( cp2 );
    const Bogie *b3 = GetBogie( cp3 );

    if ( b1 && b2 && b3 )
    {
        vector < double > turnvec( 3 );
        turnvec[0] = b1->m_SteeringAngle();
        turnvec[1] = b2->m_SteeringAngle();
        turnvec[2] = b3->m_SteeringAngle();

        isteer = vector_find_maximum( turnvec );
        steerangle = turnvec[ isteer ] * M_PI / 180.0;
        return true;
    }
    isteer = -1;
    steerangle = 0;
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

bool GearGeom::GetTwoPtSideContactPtsNormalInWorld( const string &cp1, int isymm1, int suspension1, int tire1,
                                                    const string &cp2, int isymm2, int suspension2, int tire2,
                                                    vec3d &p1, vec3d &p2, vec3d &normal ) const
{
    bool ret = GetTwoPtSideContactPtsNormal( cp1, isymm1, suspension1, tire1, cp2, isymm2, suspension2, tire2, p1, p2, normal );
    p1 = m_ModelMatrix.xform( p1 );
    p2 = m_ModelMatrix.xform( p2 );
    normal = m_ModelMatrix.xformnorm( normal );
    return ret;
}

bool GearGeom::GetOnePtSideContactPtAxisNormalInWorld( const string &cp1, int isymm1, int suspension1, int tire1,
                                                       double thetabogie, double thetawheel, double thetaroll, vec3d &p1, vec3d &axis, vec3d &normal, int &ysign ) const
{
    bool ret = GetOnePtSideContactPtAxisNormal( cp1, isymm1, suspension1, tire1, thetabogie, thetawheel, thetaroll, p1, axis, normal, ysign);
    p1 = m_ModelMatrix.xform( p1 );
    axis = m_ModelMatrix.xformnorm( axis );
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

bool GearGeom::GetContactPointVecNormal( const string &cp1, int isymm1, int suspension1, int tire1,
                                         const string &cp2, int isymm2, int suspension2, int tire2,
                                         const string &cp3, int isymm3, int suspension3, int tire3,
                                         vector < vec3d > &ptvec, vec3d &normal ) const
{
    const Bogie *b1 = GetBogie( cp1 );
    const Bogie *b2 = GetBogie( cp2 );
    const Bogie *b3 = GetBogie( cp3 );

    ptvec.resize( 3 );

    if ( b1 && b2 && b3 )
    {
        ptvec[0] = b1->GetMeanContactPoint( isymm1, tire1, suspension1, 0.0 );
        ptvec[1] = b2->GetMeanContactPoint( isymm2, tire2, suspension2, 0.0 );
        ptvec[2] = b3->GetMeanContactPoint( isymm3, tire3, suspension3, 0.0 );

        const vec3d v12 = ptvec[1]- ptvec[0] ;
        const vec3d v13 = ptvec[2] - ptvec[0] ;

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

bool GearGeom::GetContactPointVecNormalInWorld( const string &cp1, int isymm1, int suspension1, int tire1,
                                                const string &cp2, int isymm2, int suspension2, int tire2,
                                                const string &cp3, int isymm3, int suspension3, int tire3,
                                                vector < vec3d > &ptvec, vec3d &normal ) const
{
    bool ret = GetContactPointVecNormal( cp1,  isymm1,  suspension1,  tire1,
                                         cp2,  isymm2,  suspension2, tire2,
                                         cp3,  isymm3,  suspension3, tire3,
                                         ptvec, normal );

    m_ModelMatrix.xformvec( ptvec );
    normal = m_ModelMatrix.xformnorm( normal );

    return ret;
}
