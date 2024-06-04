//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//

// VSPAEROMgr.cpp: VSPAERO Mgr Singleton.
//
//////////////////////////////////////////////////////////////////////

#ifdef WIN32
#undef _HAS_STD_BYTE
#define _HAS_STD_BYTE 0
#endif

#include "Defines.h"
#include "APIDefines.h"
#include "LinkMgr.h"
#include "MeshGeom.h"
#include "ParmMgr.h"
#include "StlHelper.h"
#include "Vehicle.h"
#include "VSPAEROMgr.h"
#include "WingGeom.h"
#include "PropGeom.h"
#include "FileUtil.h"
#include "SubSurfaceMgr.h"
#include "ModeMgr.h"
#include "NGonMeshGeom.h"

//==== Constructor ====//
VspAeroControlSurf::VspAeroControlSurf()
{
    isGrouped = false;
    iReflect = false;
}

//==== Constructor ====//
VSPAEROMgrSingleton::VSPAEROMgrSingleton() : ParmContainer()
{
    m_Name = "VSPAEROSettings";
    string groupname = "VSPAERO";

    m_GeomSet.Init( "GeomSet", groupname, this, DEFAULT_SET, vsp::SET_NONE, vsp::MAX_NUM_SETS );
    m_GeomSet.SetDescript( "Thick surface geometry set" );

    m_ThinGeomSet.Init( "ThinGeomSet", groupname, this, vsp::SET_NONE, vsp::SET_NONE, vsp::MAX_NUM_SETS );
    m_ThinGeomSet.SetDescript( "Thin surface geometry set" );

    m_UseMode.Init( "UseMode", groupname, this, false, false, true );
    m_UseMode.SetDescript( "Flag to control whether modes are used instead of sets." );

    m_CGUseMode.Init( "CGUseMode", groupname, this, false, false, true );
    m_CGUseMode.SetDescript( "Flag to control whether modes are used instead of sets or CG calc." );

    m_CullFrac.Init( "CullFrac", groupname, this, 0.03, 0.0, 1.0 );
    m_CullFrac.SetDescript( "Area fraction of thin orphan regions to cull." );

    m_CullFracFlag.Init( "CullFracFlag", groupname, this, false, false, true );
    m_CullFracFlag.SetDescript( "Flag to enable orphan culling." );

    m_ContinueCoPlanarWakesFlag.Init( "ContinueCoPlanarWakesFlag", groupname, this, true, false, true );
    m_ContinueCoPlanarWakesFlag.SetDescript( "Flag to continue coplanar wakes through bodies" );

    m_AnalysisMethod.Init( "AnalysisMethod", groupname, this, vsp::VORTEX_LATTICE, vsp::VORTEX_LATTICE, vsp::PANEL );
    m_AnalysisMethod.SetDescript( "Analysis method: 0=VLM, 1=Panel" );

    m_LastPanelMeshGeomId = string();

    m_Sref.Init( "Sref", groupname, this, 100.0, 0.0, 1e12 );
    m_Sref.SetDescript( "Reference area" );

    m_bref.Init( "bref", groupname, this, 1.0, 0.0, 1e6 );
    m_bref.SetDescript( "Reference span" );

    m_cref.Init( "cref", groupname, this, 1.0, 0.0, 1e6 );
    m_cref.SetDescript( "Reference chord" );

    m_RefFlag.Init( "RefFlag", groupname, this, vsp::MANUAL_REF, 0, vsp::NUM_REF_TYPES - 1 );
    m_RefFlag.SetDescript( "Reference quantity flag" );

    m_MACFlag.Init( "MACFlag", groupname, this, false, false, true );
    m_MACFlag.SetDescript( "Use MAC instead of Cave for cbar" );

    m_SCurveFlag.Init( "SCurveFlag", groupname, this, false, false, true );
    m_SCurveFlag.SetDescript( "Use Scurve instead of Stot for Sref" );

    m_CGGeomSet.Init( "MassSet", groupname, this, DEFAULT_SET, vsp::SET_NONE, vsp::MAX_NUM_SETS );
    m_CGGeomSet.SetDescript( "Mass property set" );

    m_CGDegenSet.Init( "MassDegenSet", groupname, this, vsp::SET_NONE, vsp::SET_NONE, vsp::MAX_NUM_SETS );
    m_CGDegenSet.SetDescript( "Mass property degen set" );

    m_NumMassSlice.Init( "NumMassSlice", groupname, this, 10, 10, 200 );
    m_NumMassSlice.SetDescript( "Number of mass property slices" );

    m_MassSliceDir.Init( "MassSliceDir", groupname, this, vsp::X_DIR, vsp::X_DIR, vsp::Z_DIR );
    m_MassSliceDir.SetDescript( "Slicing direction for mass property integration" );

    m_Xcg.Init( "Xcg", groupname, this, 0.0, -1.0e12, 1.0e12 );
    m_Xcg.SetDescript( "X Center of Gravity" );

    m_Ycg.Init( "Ycg", groupname, this, 0.0, -1.0e12, 1.0e12 );
    m_Ycg.SetDescript( "Y Center of Gravity" );

    m_Zcg.Init( "Zcg", groupname, this, 0.0, -1.0e12, 1.0e12 );
    m_Zcg.SetDescript( "Z Center of Gravity" );


    // Flow Condition
    m_AlphaStart.Init( "AlphaStart", groupname, this, 0.0, -180, 180 );
    m_AlphaStart.SetDescript( "Angle of attack (Start)" );
    m_AlphaEnd.Init( "AlphaEnd", groupname, this, 10.0, -180, 180 );
    m_AlphaEnd.SetDescript( "Angle of attack (End)" );
    m_AlphaNpts.Init( "AlphaNpts", groupname, this, 3, 1, 100 );
    m_AlphaNpts.SetDescript( "Angle of attack (Num Points)" );

    m_BetaStart.Init( "BetaStart", groupname, this, 0.0, -180, 180 );
    m_BetaStart.SetDescript( "Angle of sideslip (Start)" );
    m_BetaEnd.Init( "BetaEnd", groupname, this, 0.0, -180, 180 );
    m_BetaEnd.SetDescript( "Angle of sideslip (End)" );
    m_BetaNpts.Init( "BetaNpts", groupname, this, 1, 1, 100 );
    m_BetaNpts.SetDescript( "Angle of sideslip (Num Points)" );

    m_MachStart.Init( "MachStart", groupname, this, 0.0, 0.0, 5.0 );
    m_MachStart.SetDescript( "Freestream Mach number (Start)" );
    m_MachEnd.Init( "MachEnd", groupname, this, 0.0, 0.0, 5.0 );
    m_MachEnd.SetDescript( "Freestream Mach number (End)" );
    m_MachNpts.Init( "MachNpts", groupname, this, 1, 1, 100 );
    m_MachNpts.SetDescript( "Freestream Mach number (Num Points)" );

    m_ReCrefStart.Init( "ReCref", groupname, this, 1.0e7, 0, 1e12 ); // Note Parm nameed for compatibility with pre 3.25.0 models
    m_ReCrefStart.SetDescript( "Reynolds Number Along Reference Chord (Start)" );
    m_ReCrefEnd.Init( "ReCrefEnd", groupname, this, 2.0e7, 0, 1e12 );
    m_ReCrefEnd.SetDescript( "Reynolds Number Along Reference Chord (End)" );
    m_ReCrefNpts.Init( "ReCrefNpts", groupname, this, 1, 1, 100 );
    m_ReCrefNpts.SetDescript( "Reynolds Number Along Reference Chord (Num Points)" );

    // Case Setup
    m_NCPU.Init( "NCPU", groupname, this, 4, 1, 255 );
    m_NCPU.SetDescript( "Number of processors to use" );

    //    wake parameters
    m_FixedWakeFlag.Init( "FixedWakeFlag", groupname, this, false, false, true );
    m_FixedWakeFlag.SetDescript( "Flag to enable a fixed wake." );
    m_WakeNumIter.Init( "WakeNumIter", groupname, this, 3, 3, 255 );
    m_WakeNumIter.SetDescript( "Number of wake iterations to execute" );
    m_NumWakeNodes.SetPowShift( 2, 0 ); // Must come before Init
    m_NumWakeNodes.Init( "RootWakeNodes", groupname, this, 64, 0, 10e12 );
    m_NumWakeNodes.SetDescript( "Number of Wake Nodes (f(n^2))" );

    // This sets all the filename members to the appropriate value (for example: empty strings if there is no vehicle)
    UpdateFilenames();

    m_SolverProcessKill = false;

    // Plot limits
    m_ConvergenceXMinIsManual.Init( "m_ConvergenceXMinIsManual", groupname, this, 0, 0, 1 );
    m_ConvergenceXMaxIsManual.Init( "m_ConvergenceXMaxIsManual", groupname, this, 0, 0, 1 );
    m_ConvergenceYMinIsManual.Init( "m_ConvergenceYMinIsManual", groupname, this, 0, 0, 1 );
    m_ConvergenceYMaxIsManual.Init( "m_ConvergenceYMaxIsManual", groupname, this, 0, 0, 1 );
    m_ConvergenceXMin.Init( "m_ConvergenceXMin", groupname, this, -1, -1e12, 1e12 );
    m_ConvergenceXMax.Init( "m_ConvergenceXMax", groupname, this, 1, -1e12, 1e12 );
    m_ConvergenceYMin.Init( "m_ConvergenceYMin", groupname, this, -1, -1e12, 1e12 );
    m_ConvergenceYMax.Init( "m_ConvergenceYMax", groupname, this, 1, -1e12, 1e12 );

    m_LoadDistXMinIsManual.Init( "m_LoadDistXMinIsManual", groupname, this, 0, 0, 1 );
    m_LoadDistXMaxIsManual.Init( "m_LoadDistXMaxIsManual", groupname, this, 0, 0, 1 );
    m_LoadDistYMinIsManual.Init( "m_LoadDistYMinIsManual", groupname, this, 0, 0, 1 );
    m_LoadDistYMaxIsManual.Init( "m_LoadDistYMaxIsManual", groupname, this, 0, 0, 1 );
    m_LoadDistXMin.Init( "m_LoadDistXMin", groupname, this, -1, -1e12, 1e12 );
    m_LoadDistXMax.Init( "m_LoadDistXMax", groupname, this, 1, -1e12, 1e12 );
    m_LoadDistYMin.Init( "m_LoadDistYMin", groupname, this, -1, -1e12, 1e12 );
    m_LoadDistYMax.Init( "m_LoadDistYMax", groupname, this, 1, -1e12, 1e12 );

    m_SweepXMinIsManual.Init( "m_SweepXMinIsManual", groupname, this, 0, 0, 1 );
    m_SweepXMaxIsManual.Init( "m_SweepXMaxIsManual", groupname, this, 0, 0, 1 );
    m_SweepYMinIsManual.Init( "m_SweepYMinIsManual", groupname, this, 0, 0, 1 );
    m_SweepYMaxIsManual.Init( "m_SweepYMaxIsManual", groupname, this, 0, 0, 1 );
    m_SweepXMin.Init( "m_SweepXMin", groupname, this, -1, -1e12, 1e12 );
    m_SweepXMax.Init( "m_SweepXMax", groupname, this, 1, -1e12, 1e12 );
    m_SweepYMin.Init( "m_SweepYMin", groupname, this, -1, -1e12, 1e12 );
    m_SweepYMax.Init( "m_SweepYMax", groupname, this, 1, -1e12, 1e12 );

    m_CpSliceXMinIsManual.Init( "m_CpSliceXMinIsManual", groupname, this, 0, 0, 1 );
    m_CpSliceXMaxIsManual.Init( "m_CpSliceXMaxIsManual", groupname, this, 0, 0, 1 );
    m_CpSliceYMinIsManual.Init( "m_CpSliceYMinIsManual", groupname, this, 0, 0, 1 );
    m_CpSliceYMaxIsManual.Init( "m_CpSliceYMaxIsManual", groupname, this, 0, 0, 1 );
    m_CpSliceXMin.Init( "m_CpSliceXMin", groupname, this, -1, -1e12, 1e12 );
    m_CpSliceXMax.Init( "m_CpSliceXMax", groupname, this, 1, -1e12, 1e12 );
    m_CpSliceYMin.Init( "m_CpSliceYMin", groupname, this, -1, -1e12, 1e12 );
    m_CpSliceYMax.Init( "m_CpSliceYMax", groupname, this, 1, -1e12, 1e12 );

    m_CpSliceYAxisFlipFlag.Init( "CpSliceYAxisFlipFlag", groupname, this, false, false, true );
    m_CpSliceYAxisFlipFlag.SetDescript( "Flag to Flip Y Axis in Cp Slice Plot" );
    m_CpSlicePlotLinesFlag.Init( "CpSlicePlotLinesFlag", groupname, this, true, false, true );
    m_CpSlicePlotLinesFlag.SetDescript( "Flag to Plot Lines" );

    m_UnsteadyXMinIsManual.Init( "m_UnsteadyXMinIsManual", groupname, this, 0, 0, 1 );
    m_UnsteadyXMaxIsManual.Init( "m_UnsteadyXMaxIsManual", groupname, this, 0, 0, 1 );
    m_UnsteadyYMinIsManual.Init( "m_UnsteadyYMinIsManual", groupname, this, 0, 0, 1 );
    m_UnsteadyYMaxIsManual.Init( "m_UnsteadyYMaxIsManual", groupname, this, 0, 0, 1 );
    m_UnsteadyXMin.Init( "m_UnsteadyXMin", groupname, this, -1, -1e12, 1e12 );
    m_UnsteadyXMax.Init( "m_UnsteadyXMax", groupname, this, 1, -1e12, 1e12 );
    m_UnsteadyYMin.Init( "m_UnsteadyYMin", groupname, this, -1, -1e12, 1e12 );
    m_UnsteadyYMax.Init( "m_UnsteadyYMax", groupname, this, 1, -1e12, 1e12 );

    m_UnsteadyGroupSelectType.Init( "UnsteadyGroupSelectType", groupname, this, UNSTEADY_TYPE_SELECT::HISTORY_SELECT_TYPE, UNSTEADY_TYPE_SELECT::HISTORY_SELECT_TYPE, UNSTEADY_TYPE_SELECT::ROTOR_SELECT_TYPE );
    m_LoadDistSelectType.Init( "LoadDistSelectType", groupname, this, LOAD_TYPE_SELECT::LOAD_SELECT_TYPE, LOAD_TYPE_SELECT::LOAD_SELECT_TYPE, LOAD_TYPE_SELECT::BLADE_SELECT_TYPE );

    // Other Setup Parameters
    m_Vinf.Init( "Vinf", groupname, this, 100, 0, 1e6 );
    m_Vinf.SetDescript( "Freestream Velocity Through Propeller or Actuator Disk or for Stability Analysis" );
    m_Rho.Init( "Rho", groupname, this, 0.002377, 0, 1e3 );
    m_Rho.SetDescript( "Freestream Density. Used to Calculate Propeller or Actuator Disk Coefficients" );
    m_Vref.Init( "Vref", groupname, this, 100, 0, 1e12 );
    m_Vref.SetDescript( "Reference Velocity. Set to Rotor Tip Speed for Hover Analysis (Vinf = 0)" );
    m_ManualVrefFlag.Init( "ManualVrefFlag", groupname, this, false, false, true );
    m_ManualVrefFlag.SetDescript( "Flag to Set Vref Manually or Automatiacally Equal to Vinf" );

    m_Machref.Init( "Machref", groupname, this, 0.3, 0, 1e12 );
    m_Machref.SetDescript( "Reference Mach Number. Set to Rotor Tip Mach Number for Hover Analysis (Vinf = 0)" );
    m_Precondition.Init( "Precondition", groupname, this, vsp::PRECON_MATRIX, vsp::PRECON_MATRIX, vsp::PRECON_SSOR );
    m_Precondition.SetDescript( "Preconditioner Choice" );
    m_KTCorrection.Init( "KTCorrection", groupname, this, false, false, true );
    m_KTCorrection.SetDescript( "Activate 2nd Order Karman-Tsien Mach Number Correction" );
    m_Symmetry.Init( "Symmetry", groupname, this, false, false, true );
    m_Symmetry.SetDescript( "Toggle X-Z Symmetry to Improve Calculation Time" );
    m_Write2DFEMFlag.Init( "Write2DFEMFlag", groupname, this, false, false, true );
    m_Write2DFEMFlag.SetDescript( "Toggle File Write for 2D FEM" );
    m_ClMax.Init( "Clmax", groupname, this, -1, -1, 1e3 );
    m_ClMax.SetDescript( "Cl Max of Aircraft" );
    m_ClMaxToggle.Init( "ClmaxToggle", groupname, this, vsp::CLMAX_OFF, vsp::CLMAX_OFF, vsp::CLMAX_CARLSON );
    m_ClMaxToggle.SetDescript( "Stall Modeling Option" );
    m_MaxTurnAngle.Init( "MaxTurnAngle", groupname, this, -1, -1, 360 );
    m_MaxTurnAngle.SetDescript( "Max Turning Angle of Aircraft" );
    m_MaxTurnToggle.Init( "MaxTurnToggle", groupname, this, false, false, true );
    m_FarDist.Init( "FarDist", groupname, this, -1, -1, 1e6 );
    m_FarDist.SetDescript( "Far Field Distance for Wake Adaptation" );
    m_FarDistToggle.Init( "FarDistToggle", groupname, this, false, false, true );
    m_CpSliceFlag.Init( "CpSliceFlag", groupname, this, true, false, true );
    m_CpSliceFlag.SetDescript( "Flag to Calculate Cp Slices for Each Run Case" );
    m_GroundEffect.Init( "GroundEffect", groupname, this, -1, -1, 1e6 );
    m_GroundEffect.SetDescript( "Ground Effect Distance" );
    m_GroundEffectToggle.Init( "GroundEffectToggle", groupname, this, false, false, true );

    m_ActuatorDiskFlag.Init( "ActuatorDiskFlag", groupname, this, false, false, true );
    m_ActuatorDiskFlag.SetDescript( "Flag for VSPAERO to Analyze Actuator Disks (Disk tab)" );

    m_RotateBladesFlag.Init( "RotateBladesFlag", groupname, this, false, false, true );
    m_RotateBladesFlag.SetDescript( "Flag for VSPAERO to Analyze Unsteady Rotating Propellers (Propeller tab)" );

    m_StabilityType.Init( "UnsteadyType", groupname, this, vsp::STABILITY_OFF, vsp::STABILITY_OFF, vsp::STABILITY_NUM_TYPES - 1 );
    m_StabilityType.SetDescript( "Unsteady Calculation Type" );

    // Unsteady
    m_TimeStepSize.Init( "TimeStepSize", groupname, this, 1e-3, 0, 1e9 );
    m_TimeStepSize.SetDescript( "Size of Time Step for Unsteady Analysis" );

    m_NumTimeSteps.Init( "NumTimeSteps", groupname, this, 25, 0, 1e9 );
    m_NumTimeSteps.SetDescript( "Number of Time Steps for Unsteady Analysis" );

    m_AutoTimeStepFlag.Init( "AutoTimeStepFlag", groupname, this, true, false, true );
    m_AutoTimeStepFlag.SetDescript( "Flag for VSPAERO to Automatically Calculate the Time Step for the Slowest Rotor to Complete a Set Number of Revolutions" );

    m_AutoTimeNumRevs.Init( "AutoTimeNumRevs", groupname, this, 5, 0, 1e9 );
    m_AutoTimeNumRevs.SetDescript( "Number of Revolutions for the Slowest Rotor to Complete in Auto Time Step Mode" );

    m_HoverRampFlag.Init( "HoverRampFlag", groupname, this, false, false, true );
    m_HoverRampFlag.SetDescript( "Flag to Add Hoverramp" );

    m_HoverRamp.Init( "HoverRamp", groupname, this, 0, -1e12, 1e12 );
    m_HoverRamp.SetDescript( "Decay Freestream Velocity from V1 to Vinf" );

    m_FromSteadyState.Init( "FromSteadyState", groupname, this, false, false, true );
    m_FromSteadyState.SetDescript( "Flag to Indicate Steady State" );

    m_NoiseCalcFlag.Init( "NoiseCalcFlag", groupname, this, false, false, true );
    m_NoiseCalcFlag.SetDescript( "Do Calculations for Noise and Write PSU-WOPWOP Files" );

    m_NoiseCalcType.Init( "NoiseCalcType", groupname, this, vsp::NOISE_FLYBY, vsp::NOISE_FLYBY, vsp::NOISE_STEADY );
    m_NoiseCalcType.SetDescript( "Type of Noise Calculation" );

    m_NoiseUnits.Init( "NoiseUnits", groupname, this, vsp::NOISE_SI, vsp::NOISE_SI, vsp::NOISE_ENGLISH );
    m_NoiseUnits.SetDescript( "Model Units for Noise Calculation" );

    m_UniformPropRPMFlag.Init( "UniformPropRPMFlag", groupname, this, true, false, true );
    m_UniformPropRPMFlag.SetDescript( "Flag to Set RPM to the Same Value for All Unsteady Propellers" );

    m_CurrentCSGroupIndex = -1;
    m_CurrentRotorDiskIndex = -1;
    m_LastSelectedType = -1;
    m_CurrentCpSliceIndex = -1;
    m_CurrentUnsteadyGroupIndex = 0;

    m_Verbose = false;

    CpSlice* slice = AddCpSlice();
    slice->SetName( "Y = 0" );
    slice->m_CutType.Set( vsp::Y_DIR );
    slice->m_CutPosition.Set( 0.0 );

    m_StopBeforeRun = false;
}

VSPAEROMgrSingleton::~VSPAEROMgrSingleton()
{
    for ( int i = 0; i < m_CpSliceVec.size(); i++ )
    {
        delete m_CpSliceVec[i];
    }
    m_CpSliceVec.clear();

    for ( int i = 0; i < m_ControlSurfaceGroupVec.size(); i++ )
    {
        delete m_ControlSurfaceGroupVec[i];
    }
    m_ControlSurfaceGroupVec.clear();

    for ( int i = 0; i < m_UnsteadyGroupVec.size(); i++ )
    {
        delete m_UnsteadyGroupVec[i];
    }
    m_UnsteadyGroupVec.clear();
}

void VSPAEROMgrSingleton::ParmChanged( Parm* parm_ptr, int type )
{
    Vehicle* veh = VehicleMgr.GetVehicle();

    if ( veh )
    {
        veh->ParmChanged( parm_ptr, type );
    }

    // Only allow rotor blades or actuator disk representation, but not both
    if ( &m_RotateBladesFlag == parm_ptr && m_ActuatorDiskFlag() == true && parm_ptr->Get() == true )
    {
        m_ActuatorDiskFlag.Set( false );
    }
    else if ( &m_ActuatorDiskFlag == parm_ptr && m_RotateBladesFlag() == true && parm_ptr->Get() == true )
    {
        m_RotateBladesFlag.Set( false );
    }
}

void VSPAEROMgrSingleton::Renew()
{
    for(size_t i = 0; i < m_ControlSurfaceGroupVec.size(); ++i)
    {
        delete m_ControlSurfaceGroupVec[i];
    }
    m_ControlSurfaceGroupVec.clear();

    m_CompleteControlSurfaceVec.clear();
    m_ActiveControlSurfaceVec.clear();

    for(size_t i = 0; i < m_RotorDiskVec.size(); ++i)
    {
        delete m_RotorDiskVec[i];
    }
    m_RotorDiskVec.clear();

    ClearCpSliceVec();
    CpSlice* slice = AddCpSlice();
    slice->SetName( "Y = 0" );
    slice->m_CutType.Set( vsp::Y_DIR );
    slice->m_CutPosition.Set( 0.0 );

    ClearUnsteadyGroupVec();


    m_CullFrac = 0.03;
    m_CullFracFlag = false;
    m_ContinueCoPlanarWakesFlag = true;

    m_CurrentCSGroupIndex = -1;
    m_CurrentRotorDiskIndex = -1;
    m_LastSelectedType = -1;

    m_GeomSet.Set( DEFAULT_SET );
    m_ThinGeomSet.Set( vsp::SET_NONE );

    m_UseMode.Set( false );
    m_ModeID = "";

    m_CGUseMode.Set( false );
    m_CGModeID = "";

    m_RefGeomID = "";
    m_RefFlag.Set( vsp::MANUAL_REF );
    m_MACFlag.Set( false );
    m_SCurveFlag.Set( false );
    m_Sref.Set( 100 );
    m_bref.Set( 1.0 );
    m_cref.Set( 1.0 );

    m_CGGeomSet.Set( DEFAULT_SET );
    m_CGDegenSet.Set( vsp::SET_NONE );
    m_NumMassSlice.Set( 10 );
    m_MassSliceDir.Set( vsp::X_DIR );
    m_Xcg.Set( 0.0 );
    m_Ycg.Set( 0.0 );
    m_Zcg.Set( 0.0 );

    m_AlphaStart.Set( 1.0 ); m_AlphaEnd.Set( 10 ); m_AlphaNpts.Set( 3 );
    m_BetaStart.Set( 0.0 ); m_BetaEnd.Set( 0.0 ); m_BetaNpts.Set( 1 );
    m_MachStart.Set( 0.0 ); m_MachEnd.Set( 0.0 ); m_MachNpts.Set( 1 );

    m_Precondition.Set( vsp::PRECON_MATRIX );
    m_KTCorrection.Set( false );
    m_Symmetry.Set( false );
    m_StabilityType.Set( vsp::STABILITY_OFF );

    m_ActuatorDiskFlag.Set( false );
    m_RotateBladesFlag.Set( false );

    m_NCPU.Set( 4 );

    m_StopBeforeRun = false;

    m_WakeNumIter.Set( 5 );

    m_ClMaxToggle.Set( vsp::CLMAX_OFF );
    m_MaxTurnToggle.Set( false );
    m_FarDistToggle.Set( false );
    m_GroundEffectToggle.Set( false );
    m_FromSteadyState.Set( false );
    m_NumWakeNodes.Set( 64 );
}

xmlNodePtr VSPAEROMgrSingleton::EncodeXml( xmlNodePtr & node )
{
    xmlNodePtr VSPAEROsetnode = xmlNewChild( node, nullptr, BAD_CAST"VSPAEROSettings", nullptr );

    ParmContainer::EncodeXml( VSPAEROsetnode ); // Encode VSPAEROMgr Parms

    XmlUtil::AddStringNode( VSPAEROsetnode, "RefGeomID", m_RefGeomID );
    XmlUtil::AddStringNode( VSPAEROsetnode, "ModeID", m_ModeID );
    XmlUtil::AddStringNode( VSPAEROsetnode, "CGModeID", m_CGModeID );

    // Encode Control Surface Groups using Internal Encode Method
    XmlUtil::AddIntNode( VSPAEROsetnode, "ControlSurfaceGroupCount", m_ControlSurfaceGroupVec.size() );
    for ( size_t i = 0; i < m_ControlSurfaceGroupVec.size(); ++i )
    {
        xmlNodePtr csgnode = xmlNewChild( VSPAEROsetnode, nullptr, BAD_CAST "Control_Surface_Group", nullptr );
        m_ControlSurfaceGroupVec[i]->EncodeXml( csgnode );
    }

    // Encode Rotor Disks using Internal Encode Method
    XmlUtil::AddIntNode( VSPAEROsetnode, "RotorDiskCount", m_RotorDiskVec.size() );
    for ( size_t i = 0; i < m_RotorDiskVec.size(); ++i )
    {
        xmlNodePtr rotornode = xmlNewChild( VSPAEROsetnode, nullptr, BAD_CAST "Rotor", nullptr );
        m_RotorDiskVec[i]->EncodeXml( rotornode );
    }

    // Encode CpSlices using Internal Encode Method
    XmlUtil::AddIntNode( VSPAEROsetnode, "CpSliceCount", m_CpSliceVec.size() );
    for ( size_t i = 0; i < m_CpSliceVec.size(); ++i )
    {
        xmlNodePtr cpslicenode = xmlNewChild( VSPAEROsetnode, nullptr, BAD_CAST "CpSlice", nullptr );
        m_CpSliceVec[i]->EncodeXml( cpslicenode );
    }

    // Encode Unsteady Groups using Internal Encode Method
    XmlUtil::AddIntNode( VSPAEROsetnode, "UnsteadyGroupCount", m_UnsteadyGroupVec.size() );
    for ( size_t i = 0; i < m_UnsteadyGroupVec.size(); ++i )
    {
        xmlNodePtr unsteady_node = xmlNewChild( VSPAEROsetnode, nullptr, BAD_CAST "Unsteady_Group", nullptr );
        m_UnsteadyGroupVec[i]->EncodeXml( unsteady_node );
    }

    return VSPAEROsetnode;
}

xmlNodePtr VSPAEROMgrSingleton::DecodeXml( xmlNodePtr & node )
{
    xmlNodePtr VSPAEROsetnode = XmlUtil::GetNode( node, "VSPAEROSettings", 0 );
    if ( VSPAEROsetnode )
    {
        ParmContainer::DecodeXml( VSPAEROsetnode ); // Decode VSPAEROMgr Parms

        m_RefGeomID = ParmMgr.RemapID( XmlUtil::FindString( VSPAEROsetnode, "RefGeomID", m_RefGeomID ) );
        m_ModeID = ParmMgr.RemapID( XmlUtil::FindString( VSPAEROsetnode, "ModeID", m_ModeID ) );
        m_CGModeID = ParmMgr.RemapID( XmlUtil::FindString( VSPAEROsetnode, "CGModeID", m_CGModeID ) );

        // Decode Control Surface Groups using Internal Decode Method
        int num_groups = XmlUtil::FindInt( VSPAEROsetnode, "ControlSurfaceGroupCount", 0 );
        for ( size_t i = 0; i < num_groups; ++i )
        {
            xmlNodePtr csgnode = XmlUtil::GetNode( VSPAEROsetnode, "Control_Surface_Group", i );
            if ( csgnode )
            {
                AddControlSurfaceGroup();
                m_ControlSurfaceGroupVec.back()->DecodeXml( csgnode );
            }
        }

        // Decode Rotor Disks using Internal Decode Method
        int num_rotor = XmlUtil::FindInt( VSPAEROsetnode, "RotorDiskCount", 0 );
        for ( size_t i = 0; i < num_rotor; ++i )
        {
            xmlNodePtr rotornode = XmlUtil::GetNode( VSPAEROsetnode, "Rotor", i );
            if ( rotornode )
            {
                AddRotorDisk();
                m_RotorDiskVec.back()->DecodeXml( rotornode );
            }
        }

        // One slice added automatically -- remove before adding more.  This prevents
        // adding more Y=0 slices every time a file is load/saved.
        ClearCpSliceVec();
        // Decode CpSlices using Internal Decode Method
        int num_slice = XmlUtil::FindInt( VSPAEROsetnode, "CpSliceCount", 0 );
        for ( size_t i = 0; i < num_slice; ++i )
        {
            xmlNodePtr cpslicenode = XmlUtil::GetNode( VSPAEROsetnode, "CpSlice", i );
            if ( cpslicenode )
            {
                AddCpSlice();
                m_CpSliceVec.back()->DecodeXml( cpslicenode );
            }
        }

        // Decode Unsteady Groups using Internal Decode Method
        int num__unsteady_groups = XmlUtil::FindInt( VSPAEROsetnode, "UnsteadyGroupCount", 0 );
        for ( size_t i = 0; i < num__unsteady_groups; ++i )
        {
            xmlNodePtr unsteady_node = XmlUtil::GetNode( VSPAEROsetnode, "Unsteady_Group", i );
            if ( unsteady_node )
            {
                AddUnsteadyGroup();
                m_UnsteadyGroupVec.back()->DecodeXml( unsteady_node );
            }
        }
    }

    UpdateControlSurfaceGroupSuffix();
    UpdateRotorDiskSuffix();

    return VSPAEROsetnode;
}


void VSPAEROMgrSingleton::Update()
{
    UpdateSref();

    UpdateFilenames();

    UpdateRotorDisks();

    UpdateCompleteControlSurfVec();

    UpdateControlSurfaceGroups();

    UpdateActiveControlSurfVec();

    UpdateSetupParmLimits();

    UpdateUnsteadyGroups();

    UpdateParmRestrictions();
}

void VSPAEROMgrSingleton::UpdateSref()
{
    if( m_RefFlag() == vsp::MANUAL_REF )
    {
        m_Sref.Activate();
        m_bref.Activate();
        m_cref.Activate();
    }
    else
    {
        Geom* refgeom = VehicleMgr.GetVehicle()->FindGeom( m_RefGeomID );

        if( refgeom )
        {
            if( refgeom->GetType().m_Type == MS_WING_GEOM_TYPE )
            {
                WingGeom* refwing = ( WingGeom* ) refgeom;

                if ( m_SCurveFlag() )
                {
                    m_Sref.Set( refwing->m_CurvedArea() );
                }
                else
                {
                    m_Sref.Set( refwing->m_TotalArea() );
                }

                m_bref.Set( refwing->m_TotalSpan() );

                if ( m_MACFlag() )
                {
                    m_cref.Set( refwing->m_MAC() );
                }
                else
                {
                    m_cref.Set( refwing->m_TotalChord() );
                }

                m_Sref.Deactivate();
                m_bref.Deactivate();
                m_cref.Deactivate();
            }
        }
        else
        {
            m_RefGeomID = string();
        }
    }
}

void VSPAEROMgrSingleton::UpdateSetupParmLimits()
{
    if ( m_ClMaxToggle.Get() == vsp::CLMAX_2D )
    {
        m_ClMax.SetLowerLimit( 0.0 );
        m_ClMax.Activate();
    }
    else if ( m_ClMaxToggle.Get() == vsp::CLMAX_OFF )
    {
        m_ClMax.SetLowerLimit( -1.0 );
        m_ClMax.Set( -1.0 );
        m_ClMax.Deactivate();
    }
    else if (m_ClMaxToggle.Get() == vsp::CLMAX_CARLSON)
    {
        m_ClMax.SetLowerLimit( -999 );
        m_ClMax.Set( -999 );
        m_ClMax.Deactivate();
    }

    if ( m_MaxTurnToggle() )
    {
        m_MaxTurnAngle.SetLowerLimit( 0.0 );
        m_MaxTurnAngle.Activate();
    }
    else
    {
        m_MaxTurnAngle.SetLowerLimit( -1.0 );
        m_MaxTurnAngle.Set( -1.0 );
        m_MaxTurnAngle.Deactivate();
    }

    if ( m_FarDistToggle() )
    {
        m_FarDist.SetLowerLimit( 0.0 );
        m_FarDist.Activate();
    }
    else
    {
        m_FarDist.SetLowerLimit( -1.0 );
        m_FarDist.Set( -1.0 );
        m_FarDist.Deactivate();
    }

    if ( m_GroundEffectToggle() )
    {
        m_GroundEffect.SetLowerLimit( 0.0 );
        m_GroundEffect.Activate();
    }
    else
    {
        m_GroundEffect.SetLowerLimit( -1.0 );
        m_GroundEffect.Set( -1.0 );
        m_GroundEffect.Deactivate();
    }
}

void VSPAEROMgrSingleton::UpdateFilenames()    //A.K.A. SetupDegenFile()
{
    // Initialize these to blanks.  if any of the checks fail the variables will at least contain an empty string
    m_ModelNameBase     = string();
    m_VSPGeomFileFull   = string();
    m_SetupFile         = string();
    m_AdbFile           = string();
    m_HistoryFile       = string();
    m_PolarFile         = string();
    m_LoadFile          = string();
    m_StabFile          = string();
    m_CutsFile          = string();
    m_SliceFile         = string();
    m_GroupsFile        = string();
    
    m_GroupResFiles.clear();
    m_GroupResFiles.resize( m_UnsteadyGroupVec.size() );

    m_RotorResFiles.clear();
    m_RotorResFiles.resize( NumUnsteadyRotorGroups() );

    m_UnsteadyGroupResNames.clear();
    m_UnsteadyGroupResNames.resize( m_UnsteadyGroupVec.size() );

    Vehicle *veh = VehicleMgr.GetVehicle();
    if( veh )
    {
        // Generate the base name based on the vsp3filename without the extension
        int pos = -1;

        m_VSPGeomFileFull = veh->getExportFileName( vsp::VSPAERO_VSPGEOM_TYPE );

        m_ModelNameBase = m_VSPGeomFileFull;
        pos = m_ModelNameBase.find( ".vspgeom" );
        if ( pos >= 0 )
        {
            m_ModelNameBase.erase( pos, m_ModelNameBase.length() - 1 );
        }

        m_SetupFile         = m_ModelNameBase + string( ".vspaero" );
        m_AdbFile           = m_ModelNameBase + string( ".adb" );
        m_HistoryFile       = m_ModelNameBase + string( ".history" );
        m_PolarFile         = m_ModelNameBase + string( ".polar" );
        m_LoadFile          = m_ModelNameBase + string( ".lod" );

        if ( m_StabilityType() == vsp::STABILITY_P_ANALYSIS )
        {
            m_StabFile = m_ModelNameBase + string( ".pstab" );
        }
        else if ( m_StabilityType() == vsp::STABILITY_Q_ANALYSIS )
        {
            m_StabFile = m_ModelNameBase + string( ".qstab" );
        }
        else if ( m_StabilityType() == vsp::STABILITY_R_ANALYSIS )
        {
            m_StabFile = m_ModelNameBase + string( ".rstab" );
        }
        else if ( m_StabilityType() == vsp::STABILITY_PITCH )
        {
            m_StabFile = m_ModelNameBase + string( ".aerocenter.stab" );
        }
        else
        {
            m_StabFile = m_ModelNameBase + string( ".stab" );
        }

        m_CutsFile          = m_ModelNameBase + string( ".cuts" );
        m_SliceFile         = m_ModelNameBase + string( ".slc" );
        m_GroupsFile        = m_ModelNameBase + string( ".groups" );

        for ( size_t i = 0; i < m_GroupResFiles.size(); i++ )
        {
            m_GroupResFiles[i] = m_ModelNameBase + string( ".group." ) + to_string( i + 1 );
        }

        for ( size_t i = 0; i < m_RotorResFiles.size(); i++ )
        {
            m_RotorResFiles[i] = m_ModelNameBase + string( ".rotor." ) + to_string( i + 1 );
        }

        for ( size_t i = 0; i < m_UnsteadyGroupResNames.size(); i++ )
        {
            if ( i == 0 && m_UnsteadyGroupVec[i]->m_GeomPropertyType() == m_UnsteadyGroupVec[i]->GEOM_FIXED )
            {
                m_UnsteadyGroupResNames[i] = m_UnsteadyGroupVec[i]->GetName();
            }
            else
            {
                m_UnsteadyGroupResNames[i] = m_UnsteadyGroupVec[i]->GetName() + "_Surf_" + to_string( m_UnsteadyGroupVec[i]->GetCompSurfPairVec()[0].second );
            }
        }
    }
}

void VSPAEROMgrSingleton::UpdateRotorDisks()
{
    int set = m_GeomSet();
    int degenset = m_ThinGeomSet();

    if ( m_UseMode() )
    {
        Mode *m = ModeMgr.GetMode( m_ModeID );
        if ( m )
        {
            m->ApplySettings();
            set = m->m_NormalSet();
            degenset = m->m_DegenSet();
        }
    }

    Vehicle * veh = VehicleMgr.GetVehicle();
    char str[256];

    if ( veh )
    {
        vector < RotorDisk* > temp;
        bool contained = false;

        // Get both sets.
        vector <string> currgeomvec = veh->GetGeomSet( set );
        vector <string> thingeomvec = veh->GetGeomSet( degenset );

        // Append them and make sure each is included only once.
        currgeomvec.insert( currgeomvec.end(), thingeomvec.begin(), thingeomvec.end() );
        sort( currgeomvec.begin(), currgeomvec.end() );
        currgeomvec.erase( std::unique( currgeomvec.begin(), currgeomvec.end() ), currgeomvec.end() );

        for ( size_t i = 0; i < currgeomvec.size(); ++i )
        {
            // Ensure that a deleted component is still not in the DegenGeom vector
            Geom* geom = veh->FindGeom(currgeomvec[i]);
            if (geom)
            {
                for (size_t iSubsurf = 0; iSubsurf < geom->GetNumTotalSurfs(); ++iSubsurf)
                {
                    contained = false;
                    if (geom->GetSurfType( iSubsurf ) == vsp::DISK_SURF)
                    {
                        for (size_t j = 0; j < m_RotorDiskVec.size(); ++j)
                        {
                            // If Rotor Disk and Corresponding Surface Num Already Exists within m_RotorDiskVec
                            if (m_RotorDiskVec[j]->m_ParentGeomId == currgeomvec[i] && m_RotorDiskVec[j]->GetSurfNum() == iSubsurf)
                            {
                                contained = true;
                                temp.push_back(m_RotorDiskVec[j]);

                                snprintf( str, sizeof( str ),  "%s_%zu", geom->GetName().c_str(), iSubsurf);
                                temp.back()->SetName(str);
                            }
                        }

                        // If Rotor Disk and Corresponding Surface Num Do NOT Exist within m_RotorDiskVec
                        // Create New Rotor Disk Parm Container
                        if (!contained)
                        {
                            RotorDisk *rotor = new RotorDisk();
                            temp.push_back(rotor);
                            temp.back()->m_ParentGeomId = currgeomvec[i];
                            temp.back()->m_ParentGeomSurfNdx = iSubsurf;
                            snprintf( str, sizeof( str ),  "%s_%zu", geom->GetName().c_str(), iSubsurf);
                            temp.back()->SetName(str);
                        }

                        string dia_id = geom->FindParm("Diameter", "Design");
                        temp.back()->m_Diameter.Set(ParmMgr.FindParm(dia_id)->Get());

                        temp.back()->m_XYZ = geom->CompPnt01( iSubsurf, 0, 0 );

                        // Get flag to flip normal vector but don't actually flip the normal vector. Instead flip the sign of RPM
                        temp.back()->m_FlipNormalFlag = geom->GetFlipNormal( iSubsurf );

                        // Identify normal vector before it has been flipped.
                        // Alternatively, we could get the normal vector from degen_vec[indxToSearch].getDegenDisk().nvec
                        // and flip it. Note, for unsteady propellers we flip the normal vector but keep the sign of RPM
                        // if the Prop is reversed
                        vector < Matrix4d > trans_mat_vec = geom->GetTransMatVec();
                        Matrix4d trans_mat = trans_mat_vec[iSubsurf]; // Translations for the specific symmetric copy

                        vec3d rotdir( 1, 0, 0 );

                        vec3d r_vec = trans_mat.xform( rotdir ) - temp.back()->m_XYZ;
                        temp.back()->m_Normal = r_vec;


                        // Set hub diameter from geometry
                        bool hub_set = false;
                        XSecSurf* xsecsurf = geom->GetXSecSurf( 0 );
                        if ( xsecsurf )
                        {
                            XSec* xsec = xsecsurf->FindXSec( 0 );
                            if ( xsec && xsec->GetType() == vsp::XSEC_PROP )
                            {
                                if ( temp.back()->m_AutoHubDiaFlag() )
                                {
                                    PropXSec* prop_xsec = dynamic_cast <PropXSec*> ( xsec );
                                    temp.back()->m_HubDiameter.Set( 2 * prop_xsec->m_RadiusFrac.GetResult() ); // radius to diameter
                                    temp.back()->m_HubDiameter.Deactivate();
                                    hub_set = true;
                                }
                            }
                            else
                            {
                                temp.back()->m_AutoHubDiaFlag.Set( false );
                                temp.back()->m_AutoHubDiaFlag.Deactivate();
                            }

                        }

                        if ( !hub_set )
                        {
                            temp.back()->m_HubDiameter.Activate();
                        }

                        if (temp.back()->m_HubDiameter() > temp.back()->m_Diameter())
                        {
                            temp.back()->m_HubDiameter.Set(temp.back()->m_Diameter());
                        }

                        temp.back()->Update( m_Vinf(), m_Rho() );

                    }
                }
            }
        }

        // Check for and delete any disks that no longer exist
        for ( int i = m_RotorDiskVec.size() - 1; i >= 0; i-- ) // Iterate in reverse as vector is changing size.
        {
            bool delete_flag = true;
            for ( size_t j = 0; j < temp.size(); ++j )
            {
                if ( temp[j] == m_RotorDiskVec[i] )
                {
                    delete_flag = false;
                }
            }
            if ( delete_flag )
            {
                delete m_RotorDiskVec[i];
                m_RotorDiskVec.erase( m_RotorDiskVec.begin() + i );
            }
        }
        m_RotorDiskVec.clear();
        m_RotorDiskVec = temp;
    }

    UpdateRotorDiskSuffix();
}

void VSPAEROMgrSingleton::UpdateControlSurfaceGroups()
{
    for ( size_t i = 0; i < m_ControlSurfaceGroupVec.size(); ++i )
    {
        for ( size_t k = 0; k < m_ControlSurfaceGroupVec[i]->m_ControlSurfVec.size(); ++k )
        {
            for ( size_t j = 0; j < m_CompleteControlSurfaceVec.size(); ++j )
            {
                // If Control Surface ID AND Reflection Number Match - Replace Subsurf within Control Surface Group
                if ( m_ControlSurfaceGroupVec[i]->m_ControlSurfVec[k].SSID.compare( m_CompleteControlSurfaceVec[j].SSID ) == 0 &&
                        m_ControlSurfaceGroupVec[i]->m_ControlSurfVec[k].iReflect == m_CompleteControlSurfaceVec[j].iReflect )
                {
                    m_ControlSurfaceGroupVec[i]->m_ControlSurfVec[k].fullName = m_CompleteControlSurfaceVec[j].fullName;
                    m_CompleteControlSurfaceVec[j].isGrouped = true;
                    m_ControlSurfaceGroupVec[i]->m_ControlSurfVec[k].isGrouped = true;
                }
            }
            // Remove Deleted Sub Surfaces and Sub Surfaces with Parent Geoms That No Longer Exist
            Geom* parent = VehicleMgr.GetVehicle()->FindGeom( m_ControlSurfaceGroupVec[i]->m_ControlSurfVec[k].parentGeomId );

            SubSurface* ss = nullptr;
            if ( parent ) ss = parent->GetSubSurf( m_ControlSurfaceGroupVec[i]->m_ControlSurfVec[k].SSID );

            if ( !parent || !ss || ( ss && parent->GetNumSymmCopies() <= m_ControlSurfaceGroupVec[i]->m_ControlSurfVec[k].iReflect ) )
            {
                m_ControlSurfaceGroupVec[i]->RemoveSubSurface( m_ControlSurfaceGroupVec[i]->m_ControlSurfVec[k].SSID,
                        m_ControlSurfaceGroupVec[i]->m_ControlSurfVec[k].iReflect );
                k--;
            }
        }
    }
    UpdateControlSurfaceGroupSuffix();
}

void VSPAEROMgrSingleton::CleanCompleteControlSurfVec()
{
    Vehicle* veh = VehicleMgr.GetVehicle();

    if ( veh )
    {
        // Clean Out No Longer Existing Control Surfaces (Due to Geom Changes)
        for ( int i = m_CompleteControlSurfaceVec.size() - 1; i >= 0; i-- ) // Iterate in reverse as vector is changing size.
        {
            Geom* geom = veh->FindGeom( m_CompleteControlSurfaceVec[i].parentGeomId );
            if ( !geom )
            {
                m_CompleteControlSurfaceVec.erase( m_CompleteControlSurfaceVec.begin() + i );
            }
            else if ( !geom->GetSubSurf( m_CompleteControlSurfaceVec[i].SSID ) )
            {
                m_CompleteControlSurfaceVec.erase( m_CompleteControlSurfaceVec.begin() + i );
            }
        }
    }
}

void VSPAEROMgrSingleton::UpdateCompleteControlSurfVec()
{
    m_CompleteControlSurfaceVec.clear();

    Vehicle* veh = VehicleMgr.GetVehicle();
    if ( veh )
    {
        vector< string > geom_vec = veh->GetGeomVec();
        for ( size_t i = 0; i < geom_vec.size(); ++i )
        {
            Geom *g = veh->FindGeom( geom_vec[i] );
            if ( g )
            {
                vector < SubSurface* > sub_surf_vec = g->GetSubSurfVec();
                for ( size_t j = 0; j < sub_surf_vec.size(); ++j )
                {
                    SubSurface *ssurf = sub_surf_vec[j];
                    if ( ssurf )
                    {
                        for ( size_t iReflect = 0; iReflect < g->GetNumSymmCopies(); ++iReflect )
                        {
                            if ( ssurf->GetType() == vsp::SS_CONTROL || ssurf->GetType() == vsp::SS_RECTANGLE )
                            {
                                // Create New CS Parm Container
                                VspAeroControlSurf newSurf;
                                newSurf.SSID = ssurf->GetID();
                                char str[256];
                                snprintf( str, sizeof( str ),  "%s_Surf%zu_%s", g->GetName().c_str(), iReflect, ssurf->GetName().c_str() );
                                newSurf.fullName = string( str );
                                newSurf.parentGeomId = ssurf->GetParentContainer();
                                newSurf.iReflect = iReflect;

                                m_CompleteControlSurfaceVec.push_back( newSurf );
                            }
                        }
                    }
                }
            }
        }

        CleanCompleteControlSurfVec();
    }
}

void VSPAEROMgrSingleton::UpdateActiveControlSurfVec()
{
    m_ActiveControlSurfaceVec.clear();
    if ( m_CurrentCSGroupIndex != -1 )
    {
        vector < VspAeroControlSurf > sub_surf_vec = m_ControlSurfaceGroupVec[ m_CurrentCSGroupIndex ]->m_ControlSurfVec;
        for ( size_t j = 0; j < sub_surf_vec.size(); ++j )
        {
            m_ActiveControlSurfaceVec.push_back( sub_surf_vec[j] );
        }
    }
}

void VSPAEROMgrSingleton::AddLinkableParms( vector < string > & linkable_parm_vec, const string & link_container_id )
{
    ParmContainer::AddLinkableParms( linkable_parm_vec );

    for ( size_t i = 0; i < m_ControlSurfaceGroupVec.size(); ++i )
    {
        m_ControlSurfaceGroupVec[i]->AddLinkableParms( linkable_parm_vec, m_ID );
    }

    for ( size_t i = 0; i < m_RotorDiskVec.size(); ++i )
    {
        m_RotorDiskVec[i]->AddLinkableParms( linkable_parm_vec, m_ID );
    }

    for ( size_t i = 0; i < m_CpSliceVec.size(); ++i )
    {
        m_CpSliceVec[i]->AddLinkableParms( linkable_parm_vec, m_ID );
    }
}

// InitControlSurfaceGroups - creates the initial default grouping for the control surfaces
//  The initial grouping collects all surface copies of the subsurface into a single group.
//  For example if a wing is defined with an aileron and that wing is symmetrical about the
//  xz plane there will be a surface copy of the master wing surface as well as a copy of
//  the subsurface. The two subsurfaces may get deflected differently during analysis
//  routines and can be identified uniquely by the control_surf.fullname.
//  The initial grouping routine implemented here finds all copies of that subsurface
//  that have the same sub surf ID and places them into a single control group.
void VSPAEROMgrSingleton::InitControlSurfaceGroups()
{
    Vehicle * veh = VehicleMgr.GetVehicle();
    if ( !veh )
    {
        return;
    }

    ControlSurfaceGroup * csg;
    char str [256];

    for ( size_t i = 0 ; i < m_CompleteControlSurfaceVec.size(); ++i )
    {
        // Only Autogroup Ungrouped Control Surfaces
        if ( !m_CompleteControlSurfaceVec[i].isGrouped )
        {
            bool exists = false;

            // Has CS been placed into init group?
            // --> No create group with any reflected groups
            // --> Yes Skip
            for ( size_t j = 0; j < m_ControlSurfaceGroupVec.size(); ++j )
            {
                // Check if the control surface is available
                m_CurrentCSGroupIndex = j;
                UpdateActiveControlSurfVec();
                vector < VspAeroControlSurf > ungrouped_vec = GetAvailableCSVec();
                bool is_available = false;

                for ( size_t k = 0; k < ungrouped_vec.size(); k++ )
                {
                    if ( ( m_CompleteControlSurfaceVec[i].fullName == ungrouped_vec[k].fullName ) &&
                        ( m_CompleteControlSurfaceVec[i].parentGeomId == ungrouped_vec[k].parentGeomId ) &&
                        ( m_CompleteControlSurfaceVec[i].SSID == ungrouped_vec[k].SSID ) &&
                        ( m_CompleteControlSurfaceVec[i].isGrouped == ungrouped_vec[k].isGrouped ) &&
                        ( m_CompleteControlSurfaceVec[i].iReflect == ungrouped_vec[k].iReflect ) )
                    {
                        is_available = true;
                        break;
                    }
                }

                if ( m_ControlSurfaceGroupVec[j]->m_ControlSurfVec.size() > 0 && is_available )
                {
                    // Construct a default group name
                    string curr_csg_id = m_CompleteControlSurfaceVec[i].parentGeomId + "_" + m_CompleteControlSurfaceVec[i].SSID;

                    snprintf( str, sizeof( str ),  "%s_%s", m_ControlSurfaceGroupVec[j]->m_ParentGeomBaseID.c_str(),
                        m_ControlSurfaceGroupVec[j]->m_ControlSurfVec[0].SSID.c_str() );
                    if ( curr_csg_id == str ) // Update Existing Control Surface Group
                    {
                        csg = m_ControlSurfaceGroupVec[j];
                        csg->AddSubSurface( m_CompleteControlSurfaceVec[i] );
                        m_ControlSurfaceGroupVec.back() = csg;
                        exists = true;
                        break;
                    }
                }
            }

            if ( !exists ) // Create New Control Surface Group
            {
                Geom* geom = veh->FindGeom( m_CompleteControlSurfaceVec[i].parentGeomId );
                if ( geom )
                {
                    csg = new ControlSurfaceGroup;
                    csg->AddSubSurface( m_CompleteControlSurfaceVec[i] );
                    snprintf( str, sizeof( str ),  "%s_%s", geom->GetName().c_str(),
                        geom->GetSubSurf( m_CompleteControlSurfaceVec[i].SSID )->GetName().c_str() );
                    csg->SetName( str );
                    csg->m_ParentGeomBaseID = m_CompleteControlSurfaceVec[i].parentGeomId;
                    m_ControlSurfaceGroupVec.push_back( csg );
                }
            }
        }
    }

    UpdateControlSurfaceGroupSuffix();
}

string VSPAEROMgrSingleton::ComputeGeometry()
{
    Vehicle *veh = VehicleMgr.GetVehicle();
    if ( !veh )
    {
        fprintf( stderr, "ERROR: Unable to get vehicle \n\tFile: %s \tLine:%d\n", __FILE__, __LINE__ );
        return string();
    }

    int set = m_GeomSet();
    int degenset = m_ThinGeomSet();

    if ( m_UseMode() )
    {
        Mode *m = ModeMgr.GetMode( m_ModeID );
        if ( m )
        {
            m->ApplySettings();
            set = m->m_NormalSet();
            degenset = m->m_DegenSet();
        }
    }

    // Cleanup previously created meshGeom IDs created from VSPAEROMgr
    Geom* last_mesh = veh->FindGeom( m_LastPanelMeshGeomId );

    vector < string > geom_vec = veh->GetGeomSet( set );

    if ( last_mesh )
    {
        // Remove the previous mesh, which has been updated
        veh->DeleteGeomVec( vector< string >{ m_LastPanelMeshGeomId } );
        last_mesh = nullptr;
        m_LastPanelMeshGeomId = "";
    }

    //Update information derived from the degenerate geometry
    UpdateRotorDisks();
    UpdateCompleteControlSurfVec();

    UpdateFilenames();

    int halfFlag = 0;

    if ( m_Symmetry() )
    {
        halfFlag = 1;
    }

    int mesh_set = set;

    // Generate *.vspgeom geometry file for analysis
    // Compute intersected and trimmed geometry
    string mesh_geom_id = veh->CompGeomAndFlatten( set, halfFlag, 1 /*subsFlag*/, degenset, false, true );

    MeshGeom* mesh_geom = ( MeshGeom * ) veh->FindGeom( mesh_geom_id );

    if ( mesh_geom )
    {
        m_LastPanelMeshGeomId = mesh_geom->CreateNGonMeshGeom( m_CullFracFlag(), m_CullFrac(), m_ContinueCoPlanarWakesFlag() );

        NGonMeshGeom* ngon_mesh_geom = ( NGonMeshGeom * ) veh->FindGeom( m_LastPanelMeshGeomId );


        if ( ngon_mesh_geom )
        {
            veh->DeleteGeomVec( vector< string >{ mesh_geom_id } );

            vector < string > all_fnames;
            ngon_mesh_geom->WriteVSPGEOM( m_VSPGeomFileFull, all_fnames );

            // Write out mesh to *.vspgeom file. Only the MeshGeom is shown
            WaitForFiles( all_fnames );
            for ( int ifile = 0; ifile < all_fnames.size(); ifile++ )
            {
                if ( !FileExist( all_fnames[ifile] ) )
                {
                    fprintf( stderr, "WARNING: File not found: %s\n\tFile: %s \tLine:%d\n", m_VSPGeomFileFull.c_str(), __FILE__, __LINE__ );
                }
            }

            veh->NoShowSet( vsp::SET_ALL );

            ngon_mesh_geom->SetSetFlag( vsp::SET_SHOWN, true );
            ngon_mesh_geom->SetSetFlag( vsp::SET_NOT_SHOWN, false );
        }
    }

    // Clear previous results
    while ( ResultsMgr.GetNumResults( "VSPAERO_Geom" ) > 0 )
    {
        ResultsMgr.DeleteResult( ResultsMgr.FindResultsID( "VSPAERO_Geom",  0 ) );
    }
    // Write out new results
    Results* res = ResultsMgr.CreateResults( "VSPAERO_Geom", "VSPAERO Geometry results." );
    if ( !res )
    {
        fprintf( stderr, "ERROR: Unable to create result in result manager \n\tFile: %s \tLine:%d\n", __FILE__, __LINE__ );
        return string();
    }
    res->Add( new NameValData( "GeometrySet", set, "Thick surface geometry Set for analysis." ) );
    res->Add( new NameValData( "ThinGeometrySet", degenset, "Thin surface geometry Set for analysis." ) );
    res->Add( new NameValData( "VSPGeomFileName", m_VSPGeomFileFull, "CompGeom *.vspgeom file name." ) );
    res->Add( new NameValData( "Mesh_GeomID", m_LastPanelMeshGeomId, "MeshGeom GeomID of mesh created in process." ) );

    return res->GetID();

}

string VSPAEROMgrSingleton::CreateSetupFile()
{
    string retStr = string();

    Update(); // Ensure correct control surface and rotor groups when this function is called through the API

    Vehicle *veh = VehicleMgr.GetVehicle();
    if ( !veh )
    {
        fprintf( stderr, "ERROR %d: Unable to get vehicle \n\tFile: %s \tLine:%d\n", vsp::VSP_INVALID_PTR, __FILE__, __LINE__ );
        return retStr;
    }

    // Clear existing setup file
    if ( FileExist( m_SetupFile ) )
    {
        remove( m_SetupFile.c_str() );
    }


    FILE * case_file = fopen( m_SetupFile.c_str(), "w" );
    if ( case_file == nullptr )
    {
        fprintf( stderr, "ERROR %d: Unable to create case file: %s\n\tFile: %s \tLine:%d\n", vsp::VSP_INVALID_PTR, m_SetupFile.c_str(), __FILE__, __LINE__ );
        return retStr;
    }
    fprintf( case_file, "Sref = %lf \n", m_Sref() );
    fprintf( case_file, "Cref = %lf \n", m_cref() );
    fprintf( case_file, "Bref = %lf \n", m_bref() );
    fprintf( case_file, "X_cg = %lf \n", m_Xcg() );
    fprintf( case_file, "Y_cg = %lf \n", m_Ycg() );
    fprintf( case_file, "Z_cg = %lf \n", m_Zcg() );

    vector<double> alphaVec;
    vector<double> betaVec;
    vector<double> machVec;
    vector<double> recrefVec;
    GetSweepVectors( alphaVec, betaVec, machVec, recrefVec );

    unsigned int i;
    // Mach vector
    fprintf( case_file, "Mach = " );
    for ( i = 0; i < machVec.size() - 1; i++ )
    {
        fprintf( case_file, "%lf, ", machVec[i] );
    }
    fprintf( case_file, "%lf \n", machVec[i++] );

    // Alpha vector
    fprintf( case_file, "AoA = " );
    for ( i = 0; i < alphaVec.size() - 1; i++ )
    {
        fprintf( case_file, "%lf, ", alphaVec[i] );
    }
    fprintf( case_file, "%lf \n", alphaVec[i++] );

    // Beta vector
    fprintf( case_file, "Beta = " );
    for ( i = 0; i < betaVec.size() - 1; i++ )
    {
        fprintf( case_file, "%lf, ", betaVec[i] );
    }
    fprintf( case_file, "%lf \n", betaVec[i++] );

    string sym;
    if ( m_Symmetry() )
    { sym = "Y"; }
    else
    { sym = "NO"; }

    fprintf( case_file, "Vinf = %lf \n", m_Vinf() );

    if ( m_ManualVrefFlag() )
    {
        fprintf( case_file, "Vref = %lf \n", m_Vref() );
        fprintf( case_file, "Machref = %lf \n", m_Machref() );
    }

    fprintf( case_file, "Rho = %lf \n", m_Rho() );

    fprintf( case_file, "ReCref = " );
    for ( i = 0; i < recrefVec.size() - 1; i++ )
    {
        fprintf( case_file, "%lf, ", recrefVec[i] );
    }
    fprintf( case_file, "%lf \n", recrefVec[i++] );

    fprintf( case_file, "ClMax = %lf \n", m_ClMax() );
    fprintf( case_file, "MaxTurningAngle = %lf \n", m_MaxTurnAngle() );
    fprintf( case_file, "Symmetry = %s \n", sym.c_str() );
    fprintf( case_file, "FarDist = %lf \n", m_FarDist() );
    fprintf( case_file, "NumWakeNodes = %d \n", m_NumWakeNodes() );

    if ( m_FixedWakeFlag() )
    {
        fprintf( case_file, "WakeIters = 0 \n" );
    }
    else
    {
        fprintf( case_file, "WakeIters = %d \n", m_WakeNumIter.Get() );
    }

    // RotorDisks
    if ( m_ActuatorDiskFlag() )
    {
        unsigned int numRotors = m_RotorDiskVec.size();
        fprintf( case_file, "NumberOfRotors = %u \n", numRotors );
        int iPropElement = 0;
        for ( unsigned int iRotor = 0; iRotor < m_RotorDiskVec.size(); iRotor++ )
        {
            iPropElement++;
            fprintf( case_file, "PropElement_%d\n", iPropElement );     //read in by, but not used, in vspaero and begins at 1
            fprintf( case_file, "%d\n", iPropElement );                 //read in by, but not used, in vspaero
            m_RotorDiskVec[iRotor]->Write_STP_Data( case_file );
        }
    }

    // ControlSurfaceGroups
    unsigned int numUsedCSGs = 0;
    for ( size_t iCSG = 0; iCSG < m_ControlSurfaceGroupVec.size(); iCSG++ )
    {
        if ( m_ControlSurfaceGroupVec[iCSG]->m_IsUsed() && m_ControlSurfaceGroupVec[iCSG]->m_ControlSurfVec.size() > 0 )
        {
            // Don't "use" if no control surfaces are assigned to the group
            numUsedCSGs++;
        }
    }

    fprintf( case_file, "NumberOfControlGroups = %u \n", numUsedCSGs );
    for ( size_t iCSG = 0; iCSG < m_ControlSurfaceGroupVec.size(); iCSG++ )
    {
        if ( m_ControlSurfaceGroupVec[iCSG]->m_IsUsed() && m_ControlSurfaceGroupVec[iCSG]->m_ControlSurfVec.size() > 0 )
        {
            m_ControlSurfaceGroupVec[iCSG]->Write_STP_Data( case_file );
        }
    }

    if ( m_RotateBladesFlag() )
    {
        if ( m_AutoTimeStepFlag() )
        {
            fprintf( case_file, "TimeStep = -1 \n" );
            fprintf( case_file, "NumberOfTimeSteps = %d \n", ( -1 * m_AutoTimeNumRevs.Get() ) );
        }
        else
        {
            fprintf( case_file, "TimeStep = %lf \n", m_TimeStepSize() );
            fprintf( case_file, "NumberOfTimeSteps = %d \n", m_NumTimeSteps.Get() );
        }
    }

    // The following additions to the setup file are not read by VSPAERO, and therefore must 
    // be placed at the end of the file.

    // Preconditioner
    string precon;
    if ( m_Precondition() == vsp::PRECON_MATRIX )
    {
        precon = "Matrix";
    }
    else if ( m_Precondition() == vsp::PRECON_JACOBI )
    {
        precon = "Jacobi";
    }
    else if ( m_Precondition() == vsp::PRECON_SSOR )
    {
        precon = "SSOR";
    }
    fprintf( case_file, "Preconditioner = %s \n", precon.c_str() );

    // 2nd Order Karman-Tsien Mach Number Correction
    string ktcorrect;
    if ( m_KTCorrection() )
    {
        ktcorrect = "Y";
    }
    else
    {
        ktcorrect = "N";
    }
    fprintf( case_file, "Karman-Tsien Correction = %s \n", ktcorrect.c_str() );

    fprintf( case_file, "Stability Type = %d \n", m_StabilityType() ); // Stability Type

    if ( m_RotateBladesFlag() )
    {
        fprintf( case_file, "Num Unsteady Groups = %d \n", NumUnsteadyGroups() ); // Number of unsteady groups
        fprintf( case_file, "Num Unsteady Props = %d \n", NumUnsteadyRotorGroups() ); // number of unsteady propellers
    }

    if ( m_CpSliceFlag() && m_CpSliceVec.size() > 0 )
    {
        fprintf( case_file, "NumberOfQuadTrees = %zu \n", m_CpSliceVec.size() );

        for ( i = 0; i < m_CpSliceVec.size(); i++ )
        {
            CpSlice* slice = m_CpSliceVec[i];
            if ( slice )
            {
                fprintf( case_file, "%d %d %lf \n", i + 1, slice->m_CutType() + 1, slice->m_CutPosition() );
            }
        }
    }

    //Finish up by closing the file and making sure that it appears in the file system
    fclose( case_file );

    // Wait until the setup file shows up on the file system
    WaitForFile( m_SetupFile );

    // Add and return a result
    Results* res = ResultsMgr.CreateResults( "VSPAERO_Setup", "VSPAERO setup file results." );

    if ( !FileExist( m_SetupFile ) )
    {
        // shouldn't be able to get here but create a setup file with the correct settings
        fprintf( stderr, "ERROR %d: setup file not found, file %s\n\tFile: %s \tLine:%d\n", vsp::VSP_FILE_DOES_NOT_EXIST, m_SetupFile.c_str(), __FILE__, __LINE__ );
        retStr = string();
    }
    else if ( !res )
    {
        fprintf( stderr, "ERROR: Unable to create result in result manager \n\tFile: %s \tLine:%d\n", __FILE__, __LINE__ );
        retStr = string();
    }
    else
    {
        res->Add( new NameValData( "SetupFile", m_SetupFile, "Setup file name for case." ) );
        retStr = res->GetID();
    }

    // Send the message to update the screens
    MessageData data;
    data.m_String = "UpdateAllScreens";
    MessageMgr::getInstance().Send( "ScreenMgr", nullptr, data );

    return retStr;

}

void VSPAEROMgrSingleton::ReadSetupFile()
{
    // Initialize member variables to identify Parms values from previous results
    // Note, this function will only be able to find some information necessary to load
    // previous results if the VSPAERO setup file was created after the additional 
    // information was added.
    m_PreviousStabilityType = -1;
    m_PreviousNumUnsteadyGroups = -1;
    m_PreviousNumUnsteadyProps = -1;

    FILE* case_file = fopen( m_SetupFile.c_str(), "r" );
    if ( case_file == nullptr )
    {
        fprintf( stderr, "ERROR %d: Unable to find existing VSPAERO setup file: %s\n\tFile: %s \tLine:%d\n", vsp::VSP_FILE_DOES_NOT_EXIST, m_SetupFile.c_str(), __FILE__, __LINE__ );
        return;
    }

    char strbuff[1024]; // buffer for entire line in file
    while ( fgets( strbuff, 1024, case_file ) != nullptr )
    {
        if ( string( strbuff ).find( "Stability Type" ) != string::npos )
        {
            sscanf( strbuff, "Stability Type = %d \n", &m_PreviousStabilityType );
        }
        else if ( string( strbuff ).find( "Num Unsteady Groups" ) != string::npos )
        {
            sscanf( strbuff, "Num Unsteady Groups = %d \n", &m_PreviousNumUnsteadyGroups );
        }
        else if ( string( strbuff ).find( "Num Unsteady Props" ) != string::npos )
        {
            sscanf( strbuff, "Num Unsteady Props = %d \n", &m_PreviousNumUnsteadyProps );
        }
    }

    fclose( case_file );
}

string VSPAEROMgrSingleton::LoadExistingVSPAEROResults()
{
    // The current base VSPAERO file name and analysis method is used as the basis
    // to identify previous VSPAERO results from.
    ClearAllPreviousResults();

    UpdateFilenames();

    ReadSetupFile();

    vector < string > res_id_vec;

    MessageData data;
    data.m_String = "VSPAEROSolverMessage";
    bool no_errors = true;

    if ( FileExist( m_HistoryFile ) )
    {
        ReadHistoryFile( m_HistoryFile, res_id_vec, m_ReCrefStart() );
    }
    else
    {
        data.m_StringVec = vector < string >{ "Error: VSPAERO History file " + m_HistoryFile + " not found \n" };
        MessageMgr::getInstance().Send( "ScreenMgr", nullptr, data );
        no_errors = false;
    }

    if ( m_PreviousStabilityType == vsp::STABILITY_OFF )
    {
        if ( FileExist( m_PolarFile ) )
        {
            ReadPolarFile( m_PolarFile, res_id_vec, m_ReCrefStart() );
        }
        else
        {
            data.m_StringVec = vector < string >{ "Error: VSPAERO Polar file " + m_PolarFile + " not found \n" };
            MessageMgr::getInstance().Send( "ScreenMgr", nullptr, data );
            no_errors = false;
        }
    }

    if ( FileExist( m_LoadFile ) )
    {
        ReadLoadFile( m_LoadFile, res_id_vec );
    }
    else
    {
        data.m_StringVec = vector < string >{ "Error: VSPAERO Load file " + m_LoadFile + " not found \n" };
        MessageMgr::getInstance().Send( "ScreenMgr", nullptr, data );
        no_errors = false;
    }

    if ( m_PreviousStabilityType > vsp::VSPAERO_STABILITY_TYPE::STABILITY_OFF )
    {
        string stab_file;

        if ( m_PreviousStabilityType == vsp::STABILITY_P_ANALYSIS )
        {
            stab_file = m_ModelNameBase + string( ".pstab" );
        }
        else if ( m_PreviousStabilityType == vsp::STABILITY_Q_ANALYSIS )
        {
            stab_file = m_ModelNameBase + string( ".qstab" );
        }
        else if ( m_PreviousStabilityType == vsp::STABILITY_R_ANALYSIS )
        {
            stab_file = m_ModelNameBase + string( ".rstab" );
        }
        else
        {
            stab_file = m_ModelNameBase + string( ".stab" );
        }

        if ( FileExist( stab_file ) )
        {
            ReadStabFile( stab_file, res_id_vec, ( vsp::VSPAERO_STABILITY_TYPE )m_PreviousStabilityType );
        }
        else
        {
            data.m_StringVec = vector < string >{ "Error: VSPAERO Stab file " + stab_file + " not found \n" };
            MessageMgr::getInstance().Send( "ScreenMgr", nullptr, data );
            no_errors = false;
        }
    }

    if ( m_PreviousNumUnsteadyGroups > 0 && m_PreviousNumUnsteadyGroups == m_GroupResFiles.size() )
    {
        for ( size_t i = 0; i < m_PreviousNumUnsteadyGroups; i++ )
        {
            if ( FileExist( m_GroupResFiles[i] ) )
            {
                ReadGroupResFile( m_GroupResFiles[i], res_id_vec, ( "Group_" + to_string(i) ) );
            }
            else
            {
                data.m_StringVec = vector < string >{ "Error: VSPAERO Group file " + m_GroupResFiles[i] + " not found \n" };
                MessageMgr::getInstance().Send( "ScreenMgr", nullptr, data );
                no_errors = false;
            }
        }
    }

    if ( m_PreviousNumUnsteadyProps > 0 && m_PreviousNumUnsteadyProps == m_RotorResFiles.size() )
    {
        for ( size_t i = 0; i < m_PreviousNumUnsteadyProps; i++ )
        {
            if ( FileExist( m_RotorResFiles[i] ) )
            {
                ReadRotorResFile( m_RotorResFiles[i], res_id_vec, ( "Rotor_" + to_string( i ) ) );
            }
            else
            {
                data.m_StringVec = vector < string >{ "Error: VSPAERO Rotor file " + m_RotorResFiles[i] + " not found \n" };
                MessageMgr::getInstance().Send( "ScreenMgr", nullptr, data );
                no_errors = false;
            }
        }
    }

    if ( no_errors )
    {
        data.m_StringVec = vector < string >{ "All previous VSPAERO results successfully read \n" };
        MessageMgr::getInstance().Send( "ScreenMgr", nullptr, data );
    }

    // Create "wrapper" result to contain a vector of result IDs (this maintains compatibility to return a single result after computation)
    Results *res = ResultsMgr.CreateResults( "VSPAERO_Wrapper", "VSPAERO read existing solution results." );
    if ( !res )
    {
        return string();
    }

    res->Add( new NameValData( "ResultsVec", res_id_vec, "ID's of VSPAERO analysis results." ) );
    return res->GetID();
}

void VSPAEROMgrSingleton::ClearAllPreviousResults()
{
    ClearCpSliceResults();

    while ( ResultsMgr.GetNumResults( "VSPAERO_History" ) > 0 )
    {
        ResultsMgr.DeleteResult( ResultsMgr.FindResultsID( "VSPAERO_History",  0 ) );
    }
    while ( ResultsMgr.GetNumResults( "VSPAERO_Polar" ) > 0 )
    {
        ResultsMgr.DeleteResult( ResultsMgr.FindResultsID( "VSPAERO_Polar", 0 ) );
    }
    while ( ResultsMgr.GetNumResults( "VSPAERO_Load" ) > 0 )
    {
        ResultsMgr.DeleteResult( ResultsMgr.FindResultsID( "VSPAERO_Load",  0 ) );
    }
    while ( ResultsMgr.GetNumResults( "VSPAERO_Stab" ) > 0 )
    {
        ResultsMgr.DeleteResult( ResultsMgr.FindResultsID( "VSPAERO_Stab",  0 ) );
    }
    while ( ResultsMgr.GetNumResults( "VSPAERO_Group" ) > 0 )
    {
        ResultsMgr.DeleteResult( ResultsMgr.FindResultsID( "VSPAERO_Group", 0 ) );
    }
    while ( ResultsMgr.GetNumResults( "VSPAERO_Rotor" ) > 0 )
    {
        ResultsMgr.DeleteResult( ResultsMgr.FindResultsID( "VSPAERO_Rotor", 0 ) );
    }
    while ( ResultsMgr.GetNumResults( "VSPAERO_Blade_Avg" ) > 0 )
    {
        ResultsMgr.DeleteResult( ResultsMgr.FindResultsID( "VSPAERO_Blade_Avg", 0 ) );
    }
    while ( ResultsMgr.GetNumResults( "VSPAERO_Blade_Last_Rev" ) > 0 )
    {
        ResultsMgr.DeleteResult( ResultsMgr.FindResultsID( "VSPAERO_Blade_Last_Rev", 0 ) );
    }
    while ( ResultsMgr.GetNumResults( "VSPAERO_Wrapper" ) > 0 )
    {
        ResultsMgr.DeleteResult( ResultsMgr.FindResultsID( "VSPAERO_Wrapper",  0 ) );
    }
}

void VSPAEROMgrSingleton::GetSweepVectors( vector<double> &alphaVec, vector<double> &betaVec, vector<double> &machVec, vector<double> &recrefVec )
{
    // grab current parm values
    double alphaStart = m_AlphaStart.Get();
    double alphaEnd = m_AlphaEnd.Get();
    int alphaNpts = m_AlphaNpts.Get();

    double betaStart = m_BetaStart.Get();
    double betaEnd = m_BetaEnd.Get();
    int betaNpts = m_BetaNpts.Get();

    double machStart = m_MachStart.Get();
    double machEnd = m_MachEnd.Get();
    int machNpts = m_MachNpts.Get();

    double recrefStart = m_ReCrefStart.Get();
    double recrefEnd = m_ReCrefEnd.Get();
    int recrefNpts = m_ReCrefNpts.Get();

    // Calculate spacing
    double alphaDelta = 0.0;
    if ( alphaNpts > 1 )
    {
        alphaDelta = ( alphaEnd - alphaStart ) / ( alphaNpts - 1.0 );
    }
    for ( int iAlpha = 0; iAlpha < alphaNpts; iAlpha++ )
    {
        //Set current alpha value
        alphaVec.push_back( alphaStart + double( iAlpha ) * alphaDelta );
    }

    double betaDelta = 0.0;
    if ( betaNpts > 1 )
    {
        betaDelta = ( betaEnd - betaStart ) / ( betaNpts - 1.0 );
    }
    for ( int iBeta = 0; iBeta < betaNpts; iBeta++ )
    {
        //Set current beta value
        betaVec.push_back( betaStart + double( iBeta ) * betaDelta );
    }

    double machDelta = 0.0;
    if ( machNpts > 1 )
    {
        machDelta = ( machEnd - machStart ) / ( machNpts - 1.0 );
    }
    for ( int iMach = 0; iMach < machNpts; iMach++ )
    {
        //Set current Mach value
        machVec.push_back( machStart + double( iMach ) * machDelta );
    }

    double recrefDelta = 0.0;
    if ( recrefNpts > 1 )
    {
        recrefDelta = ( recrefEnd - recrefStart ) / ( recrefNpts - 1.0 );
    }
    for ( int iReCref = 0; iReCref < recrefNpts; iReCref++ )
    {
        //Set current ReCref value
        recrefVec.push_back( recrefStart + double( iReCref ) * recrefDelta );
    }
}

/* ComputeSolver(FILE * logFile)
Returns a result with a vector of results id's under the name ResultVec
Optional input of logFile allows outputting to a log file or the console
*/
string VSPAEROMgrSingleton::ComputeSolver( FILE * logFile )
{
    Update(); // Force update to ensure correct number of unstead groups, actuator disks, etc when run though the API.
    UpdateFilenames(); // Do we really need this? is also called by Update() moments before

    return ComputeSolverBatch( logFile );
}

/* ComputeSolverBatch(FILE * logFile)
*/
string VSPAEROMgrSingleton::ComputeSolverBatch( FILE * logFile )
{
    std::vector <string> res_id_vector;

    Vehicle *veh = VehicleMgr.GetVehicle();

    if ( veh )
    {

        string adbFileName = m_AdbFile;
        string historyFileName = m_HistoryFile;
        string polarFileName = m_PolarFile;
        string loadFileName = m_LoadFile;
        string stabFileName = m_StabFile;
        string modelNameBase = m_ModelNameBase;

        vector < string > group_res_vec = m_GroupResFiles;
        vector < string > rotor_res_vec = m_RotorResFiles;
        vector < string > unsteady_group_name_vec = m_UnsteadyGroupResNames;

        bool unsteady_flag = m_RotateBladesFlag.Get();
        vsp::VSPAERO_STABILITY_TYPE stabilityType = ( vsp::VSPAERO_STABILITY_TYPE )m_StabilityType.Get();
        bool noise_flag = m_NoiseCalcFlag.Get();
        int noise_type = m_NoiseCalcType.Get();
        int noise_unit = m_NoiseUnits.Get();

        double recref = m_ReCrefStart.Get();

        int ncpu = m_NCPU.Get();


        //====== Modify/Update the setup file ======//
        if ( m_Verbose ) { printf( "Writing vspaero setup file: %s\n", m_SetupFile.c_str() ); }
        CreateSetupFile();

        //====== Modify/Update the groups file for unsteady analysis ======//
        if ( m_RotateBladesFlag() )
        {
            if ( m_Verbose ) { printf( "Writing vspaero groups file: %s\n", m_GroupsFile.c_str() ); }
            CreateGroupsFile();
        }

        //====== Clear VSPAERO output files ======//
        if ( FileExist( adbFileName ) )
        {
            remove( adbFileName.c_str() );
        }
        if ( FileExist( historyFileName ) )
        {
            remove( historyFileName.c_str() );
        }
        if ( FileExist( polarFileName ) )
        {
            remove( polarFileName.c_str() );
        }
        if ( FileExist( loadFileName ) )
        {
            remove( loadFileName.c_str() );
        }
        if ( FileExist( stabFileName ) )
        {
            remove( stabFileName.c_str() );
        }

        for ( size_t j = 0; j < group_res_vec.size(); j++ )
        {
            if ( FileExist( group_res_vec[j] ) )
            {
                remove( group_res_vec[j].c_str() );
            }
        }

        for ( size_t j = 0; j < rotor_res_vec.size(); j++ )
        {
            if ( FileExist( rotor_res_vec[j] ) )
            {
                remove( rotor_res_vec[j].c_str() );
            }
        }

        //====== generate batch mode command to be executed by the system at the command prompt ======//
        vector<string> args;

        // Set number of openmp threads
        args.push_back( "-omp" );
        args.push_back( StringUtil::int_to_string( m_NCPU.Get(), "%d" ) );

        // Set stability run arguments
        if ( stabilityType != vsp::STABILITY_OFF )
        {
// Disable "enumeration values not handled in switch" warning
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wswitch"
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wswitch"
            switch ( stabilityType )
            {
                case vsp::STABILITY_DEFAULT:
                    args.push_back( "-stab" );
                    break;

                case vsp::STABILITY_P_ANALYSIS:
                    args.push_back( "-pstab" );
                    break;

                case vsp::STABILITY_Q_ANALYSIS:
                    args.push_back( "-qstab" );
                    break;

                case vsp::STABILITY_R_ANALYSIS:
                    args.push_back( "-rstab" );
                    break;

                // === To Be Implemented ===
                //case vsp::STABILITY_HEAVE:
                //    AnalysisType = "HEAVE";
                //    break;

                //case vsp::STABILITY_IMPULSE:
                //    AnalysisType = "IMPULSE";
                //    break;

                //case vsp::STABILITY_UNSTEADY:
                //    args.push_back( "-unsteady" );
                //    break;

                case vsp::STABILITY_PITCH:
                    args.push_back( "-acstab" );
                    break;
            }
#pragma GCC diagnostic pop
#pragma clang diagnostic pop
        }

        if ( m_FromSteadyState() )
        {
            args.push_back( "-fromsteadystate" );
        }

        if ( m_GroundEffectToggle() )
        {
            args.push_back( "-groundheight" );
            args.push_back( StringUtil::double_to_string( m_GroundEffect(), "%f" ) );
        }

        if( m_Write2DFEMFlag() )
        {
            args.push_back( "-write2dfem" );
        }

        if ( m_Precondition() == vsp::PRECON_JACOBI )
        {
            args.push_back( "-jacobi" );
        }
        else if ( m_Precondition() == vsp::PRECON_SSOR )
        {
            args.push_back( "-ssor" );
        }

        if ( m_KTCorrection() )
        {
            args.push_back( "-dokt" );
        }

        if ( m_RotateBladesFlag() )
        {
            args.push_back( "-unsteady" );

            if ( m_HoverRampFlag() )
            {
                args.push_back( "-hoverramp" );
                args.push_back( StringUtil::double_to_string( m_HoverRamp(), "%f" ) );
            }
        }

        // Add model file name
        args.push_back( modelNameBase );

        //Print out execute command
        string cmdStr = ProcessUtil::PrettyCmd( veh->GetVSPAEROPath(), veh->GetVSPAEROCmd(), args );
        if( logFile )
        {
            fprintf( logFile, "%s", cmdStr.c_str() );
        }
        else
        {
            MessageData data;
            data.m_String = "VSPAEROSolverMessage";
            data.m_StringVec.push_back( cmdStr );
            MessageMgr::getInstance().Send( "ScreenMgr", nullptr, data );
        }

        if ( m_StopBeforeRun )
        {
            m_StopBeforeRun = false;
            return string();
        }

        // Execute VSPAero
        m_SolverProcess.ForkCmd( veh->GetVSPAEROPath(), veh->GetVSPAEROCmd(), args );

        // ==== MonitorSolverProcess ==== //
        MonitorProcess( logFile, &m_SolverProcess, "VSPAEROSolverMessage" );

        // Check if the kill solver flag has been raised, if so clean up and return
        //  note: we could have exited the IsRunning loop if the process was killed
        if( m_SolverProcessKill )
        {
            m_SolverProcessKill = false;    //reset kill flag

            return string();    //return empty result ID vector
        }

        //====== Read in all of the results ======//
        ReadHistoryFile( historyFileName, res_id_vector, recref );

        if ( stabilityType == vsp::STABILITY_OFF )
        {
            ReadPolarFile( polarFileName, res_id_vector, recref ); // Must be after *.history file is read to generate results for multiple ReCref values
        }

        ReadLoadFile( loadFileName, res_id_vector );

        if ( stabilityType != vsp::STABILITY_OFF )
        {
            ReadStabFile( stabFileName, res_id_vector, stabilityType );      //*.STAB stability coeff file
        }

        // CpSlice *.adb File and slices are defined
        if ( m_CpSliceFlag() && m_CpSliceVec.size() > 0 )
        {
            string slice_res_id = ComputeCpSlices();
            res_id_vector.push_back( slice_res_id );
        }

        if ( unsteady_flag )
        {
            for ( size_t j = 0; j < group_res_vec.size(); j++ )
            {
                ReadGroupResFile( group_res_vec[j], res_id_vector, unsteady_group_name_vec[j] );
            }

            int offset = group_res_vec.size() - rotor_res_vec.size();

            for ( size_t j = 0; j < rotor_res_vec.size(); j++ )
            {
                ReadRotorResFile( rotor_res_vec[j], res_id_vector, unsteady_group_name_vec[j + offset] );
            }

            if ( noise_flag )
            {
                ExecuteNoiseAnalysis( logFile, noise_type, noise_unit );
            }
        }

        // Send the message to update the screens
        MessageData data;
        data.m_String = "UpdateAllScreens";
        MessageMgr::getInstance().Send( "ScreenMgr", nullptr, data );

    }

    // Create "wrapper" result to contain a vector of result IDs (this maintains compatibility to return a single result after computation)
    Results *res = ResultsMgr.CreateResults( "VSPAERO_Wrapper", "VSPAERO sweep analysis results." );
    if( !res )
    {
        return string();
    }
    else
    {
        res->Add( new NameValData( "ResultsVec", res_id_vector, "ID's of VSPAERO sweep analysis results." ) );
        return res->GetID();
    }
}

void VSPAEROMgrSingleton::AddResultHeader( const string &res_id, double mach, double alpha, double beta )
{
    // Add Flow Condition header to each result
    Results * res = ResultsMgr.FindResultsPtr( res_id );
    if ( res )
    {

    }
}

// helper thread functions for VSPAERO GUI interface and multi-threaded impleentation
bool VSPAEROMgrSingleton::IsSolverRunning()
{
    return m_SolverProcess.IsRunning();
}

void VSPAEROMgrSingleton::KillSolver()
{
    // Raise flag to break the compute solver thread
    m_SolverProcessKill = true;
    return m_SolverProcess.Kill();
}

ProcessUtil* VSPAEROMgrSingleton::GetSolverProcess()
{
    return &m_SolverProcess;
}

/*******************************************************
Read .HISTORY file output from VSPAERO
See: VSP_Solver.C in vspaero project
line 4351 - void VSP_SOLVER::OutputStatusFile(int Type)
line 4407 - void VSP_SOLVER::OutputZeroLiftDragToStatusFile(void)
TODO:
- Update this function to use the generic table read as used in: string VSPAEROMgrSingleton::ReadStabFile()
*******************************************************/
void VSPAEROMgrSingleton::ReadHistoryFile( const string &filename, vector <string> &res_id_vector, double recref )
{
    //TODO return success or failure
    FILE *fp = nullptr;
    //size_t result;
    bool read_success = false;

    //HISTORY file
    WaitForFile( filename );
    fp = fopen( filename.c_str(), "r" );
    if ( fp == nullptr )
    {
        fprintf( stderr, "ERROR %d: Could not open History file: %s\n\tFile: %s \tLine:%d\n", vsp::VSP_FILE_DOES_NOT_EXIST, m_HistoryFile.c_str(), __FILE__, __LINE__ );
        return;
    }

    Results* res = nullptr;
    std::vector<string> data_string_array;

    char seps[]   = " :,\t\n";
    while ( !feof( fp ) )
    {
        data_string_array = ReadDelimLine( fp, seps ); //this is also done in some of the embedded loops below

        if ( CheckForCaseHeader( data_string_array ) )
        {
            res = ResultsMgr.CreateResults( "VSPAERO_History", "VSPAERO History file results." );
            res_id_vector.push_back( res->GetID() );

            if ( ReadVSPAEROCaseHeader( res, fp ) != 0 )
            {
                // Failed to read the case header
                fprintf( stderr, "ERROR %d: Could not read case header in VSPAERO file: %s\n\tFile: %s \tLine:%d\n", vsp::VSP_FILE_READ_FAILURE, m_StabFile.c_str(), __FILE__, __LINE__ );
                return;
            }

            res->Add( new NameValData( "FC_ReCref_", recref, "Reynolds number." ) );
        }

        //READ wake iteration table
        /* Example wake iteration table
      Iter             Mach             AoA              Beta              CLo             CLi            CLtot              CDo              CDi             CDtot              CDt            CDtot_t           CSo              CSi            CStot               L/D              E               CFxo             CFyo             CFzo             CFxi             CFyi             CFzi             CFxtot           CFytot           CFztot           CMxo             CMyo             CMzo             CMxi             CMyi             CMzi             CMxtot           CMytot           CMztot          T/QS
               1   0.001000000000  10.000000000000   0.000000000000  -0.002045979010   1.014947981124   1.012902002114   0.012476073687   0.051479459485   0.063955533173   0.046529859616   0.059005933303   0.000017342556  -0.000483613705  -0.000466271149  15.869588302617   0.884649651914   0.012641814621   0.000017342556   0.000151551469  -0.125546496527  -0.000483613705   1.008467955042  -0.112904681906  -0.000466271149   1.008619506511   0.000000066648  -0.002024085230   0.000019540874  -0.001310492462  -2.099444030482  -0.000281526352  -0.001310425814  -2.101468115712  -0.000261985478   0.000000000000
               2   0.001000000000  10.000000000000   0.000000000000  -0.002046524971   1.013995626911   1.011949101941   0.012472078545   0.051266503048   0.063738581593   0.046141207128   0.058613285674   0.000000421857  -0.000084496014  -0.000084074158  15.908663192154   0.886658109538   0.012637974980   0.000000421857   0.000150320053  -0.125590843104  -0.000084496014   1.007493089733  -0.112952868125  -0.000084074158   1.007643409786  -0.000000020250  -0.002020759170   0.000001308592  -0.000114914460  -2.099867836603  -0.000052447483  -0.000114934710  -2.101888595773  -0.000051138891   0.000000000000
        ...
        */
        int wake_iter_table_columns = 36;
        int num_unsteady_pqr_col = 37;
        bool unsteady_flag = false;
        bool unsteady_h = false;
        bool unsteady_pqr = false;

        if ( data_string_array.size() == num_unsteady_pqr_col )
        {
            if ( strcmp( data_string_array[ 0 ].c_str(), "Time" ) == 0 )
            {
                unsteady_flag = true;

                if ( strcmp( data_string_array.back().c_str(), "H" ) == 0 )
                {
                    unsteady_h = true;
                }

                if ( strcmp( data_string_array.back().c_str(), "UnstdyAng" ) == 0 )
                {
                    unsteady_pqr = true;
                }
            }
        }

        if( data_string_array.size() >= wake_iter_table_columns )
        {
            //discard the header row and read the next line assuming that it is numeric
            data_string_array = ReadDelimLine( fp, seps );

            // create new vectors for this set of results information
            std::vector<int> i;
            std::vector<double> time;
            std::vector<double> Mach;
            std::vector<double> Alpha;
            std::vector<double> Beta;
            std::vector<double> CLo;
            std::vector<double> CLi;
            std::vector<double> CLtot;
            std::vector<double> CDo;
            std::vector<double> CDi;
            std::vector<double> CDtot;
            std::vector<double> CDt;
            std::vector<double> CDtott;
            std::vector<double> CSo;
            std::vector<double> CSi;
            std::vector<double> CStot;
            std::vector<double> LoD;
            std::vector<double> E;
            std::vector<double> CFxo;
            std::vector<double> CFyo;
            std::vector<double> CFzo;
            std::vector<double> CFxi;
            std::vector<double> CFyi;
            std::vector<double> CFzi;
            std::vector<double> CFxtot;
            std::vector<double> CFytot;
            std::vector<double> CFztot;
            std::vector<double> CMxo;
            std::vector<double> CMyo;
            std::vector<double> CMzo;
            std::vector<double> CMxi;
            std::vector<double> CMyi;
            std::vector<double> CMzi;
            std::vector<double> CMxtot;
            std::vector<double> CMytot;
            std::vector<double> CMztot;
            std::vector<double> ToQS;
            std::vector<double> UnstdAng;
            std::vector<double> H;

            while ( data_string_array.size() >= wake_iter_table_columns )
            {
                int icol = 0;
                if ( unsteady_flag )
                {
                    time.push_back( std::stod( data_string_array[icol] ) ); icol++;
                }
                else
                {
                    i.push_back( std::stoi( data_string_array[icol] ) ); icol++;
                }

                Mach.push_back(     std::stod( data_string_array[icol] ) ); icol++;
                Alpha.push_back(    std::stod( data_string_array[icol] ) ); icol++;
                Beta.push_back(     std::stod( data_string_array[icol] ) ); icol++;

                CLo.push_back(      std::stod( data_string_array[icol] ) ); icol++;
                CLi.push_back(      std::stod( data_string_array[icol] ) ); icol++;
                CLtot.push_back(    std::stod( data_string_array[icol] ) ); icol++;
                CDo.push_back(      std::stod( data_string_array[icol] ) ); icol++;
                CDi.push_back(      std::stod( data_string_array[icol] ) ); icol++;
                CDtot.push_back(    std::stod( data_string_array[icol] ) ); icol++;
                CDt.push_back(      std::stod( data_string_array[icol] ) ); icol++;
                CDtott.push_back(   std::stod( data_string_array[icol] ) ); icol++;
                CSo.push_back(      std::stod( data_string_array[icol] ) ); icol++;
                CSi.push_back(      std::stod( data_string_array[icol] ) ); icol++;
                CStot.push_back(    std::stod( data_string_array[icol] ) ); icol++;

                LoD.push_back(      std::stod( data_string_array[icol] ) ); icol++;
                E.push_back(        std::stod( data_string_array[icol] ) ); icol++;

                CFxo.push_back(     std::stod( data_string_array[icol] ) ); icol++;
                CFyo.push_back(     std::stod( data_string_array[icol] ) ); icol++;
                CFzo.push_back(     std::stod( data_string_array[icol] ) ); icol++;
                CFxi.push_back(     std::stod( data_string_array[icol] ) ); icol++;
                CFyi.push_back(     std::stod( data_string_array[icol] ) ); icol++;
                CFzi.push_back(     std::stod( data_string_array[icol] ) ); icol++;
                CFxtot.push_back(   std::stod( data_string_array[icol] ) ); icol++;
                CFytot.push_back(   std::stod( data_string_array[icol] ) ); icol++;
                CFztot.push_back(   std::stod( data_string_array[icol] ) ); icol++;

                CMxo.push_back(     std::stod( data_string_array[icol] ) ); icol++;
                CMyo.push_back(     std::stod( data_string_array[icol] ) ); icol++;
                CMzo.push_back(     std::stod( data_string_array[icol] ) ); icol++;
                CMxi.push_back(     std::stod( data_string_array[icol] ) ); icol++;
                CMyi.push_back(     std::stod( data_string_array[icol] ) ); icol++;
                CMzi.push_back(     std::stod( data_string_array[icol] ) ); icol++;
                CMxtot.push_back(   std::stod( data_string_array[icol] ) ); icol++;
                CMytot.push_back(   std::stod( data_string_array[icol] ) ); icol++;
                CMztot.push_back(   std::stod( data_string_array[icol] ) ); icol++;

                ToQS.push_back(      std::stod( data_string_array[icol] ) ); icol++;

                if ( unsteady_pqr ) // Additional columns for pqr analysis
                {
                    UnstdAng.push_back( std::stod( data_string_array[icol] ) ); icol++;
                }

                if ( unsteady_h ) // Additional columns for h analysis
                {
                    H.push_back( std::stod( data_string_array[icol] ) ); icol++;
                }

                data_string_array = ReadDelimLine( fp, seps );
            }

            //add to the results manager
            if ( res )
            {
                if ( unsteady_flag || unsteady_pqr )
                {
                    res->Add( new NameValData( "Time", time, "Time in unsteady analysis." ) );
                }
                else
                {
                    res->Add( new NameValData( "WakeIter", i, "Wake relaxation iteration." ) );
                }
                res->Add( new NameValData( "Mach", Mach, "Mach number." ) );
                res->Add( new NameValData( "Alpha", Alpha, "Angle of attack." ) );
                res->Add( new NameValData( "Beta", Beta, "Angle of sideslip." ) );
                res->Add( new NameValData( "CLo", CLo, "Parasite component of lift coefficient." ) );
                res->Add( new NameValData( "CLi", CLi, "Inviscid component of lift coefficient." ) );
                res->Add( new NameValData( "CL", CLtot, "Lift coefficient." ) );
                res->Add( new NameValData( "CDo", CDo, "Parasite drag coefficient." ) );
                res->Add( new NameValData( "CDi", CDi, "Induced drag coefficient." ) );
                res->Add( new NameValData( "CDtot", CDtot, "Total drag coefficient." ) );
                res->Add( new NameValData( "CDt", CDt, "Induced drag coefficient from Trefftz-like calculation." ) );
                res->Add( new NameValData( "CDtott", CDtott, "Total drag coefficient from Trefftz-like calculation." ) );
                res->Add( new NameValData( "CSo", CSo, "Parasite component of side force coefficient." ) );
                res->Add( new NameValData( "CSi", CSi, "Inviscid component of side force coefficient." ) );
                res->Add( new NameValData( "CS", CStot, "Side force coefficient." ) );
                res->Add( new NameValData( "L/D", LoD, "Lift to drag ratio." ) );
                res->Add( new NameValData( "E", E, "Oswald efficiency factor." ) );
                res->Add( new NameValData( "CFxo", CFxo, "Parasite component of X force coefficient." ) );
                res->Add( new NameValData( "CFyo", CFyo, "Parasite component of Y force coefficient." ) );
                res->Add( new NameValData( "CFzo", CFzo, "Parasite component of Z force coefficient." ) );
                res->Add( new NameValData( "CFxi", CFxi, "Inviscid component of X force coefficient." ) );
                res->Add( new NameValData( "CFyi", CFyi, "Inviscid component of Y force coefficient." ) );
                res->Add( new NameValData( "CFzi", CFzi, "Inviscid component of Z force coefficient." ) );
                res->Add( new NameValData( "CFx", CFxtot, "X force coefficient." ) );
                res->Add( new NameValData( "CFy", CFytot, "Y force coefficient." ) );
                res->Add( new NameValData( "CFz", CFztot, "Z force coefficient." ) );
                res->Add( new NameValData( "CMxo", CMxo, "Parasite component of X moment coefficient." ) );
                res->Add( new NameValData( "CMyo", CMyo, "Parasite component of Y moment coefficient." ) );
                res->Add( new NameValData( "CMzo", CMzo, "Parasite component of Z moment coefficient." ) );
                res->Add( new NameValData( "CMxi", CMxi, "Inviscid component of X moment coefficient." ) );
                res->Add( new NameValData( "CMyi", CMyi, "Inviscid component of Y moment coefficient." ) );
                res->Add( new NameValData( "CMzi", CMzi, "Inviscid component of Z moment coefficient." ) );
                res->Add( new NameValData( "CMx", CMxtot, "X moment coefficient." ) );
                res->Add( new NameValData( "CMy", CMytot, "Y moment coefficient." ) );
                res->Add( new NameValData( "CMz", CMztot, "Z moment coefficient." ) );
                res->Add( new NameValData( "T/QS", ToQS, "Thrust coefficient." ) );

                if ( unsteady_pqr )
                {
                    res->Add( new NameValData( "UnstdyAng", UnstdAng, "Unsteady rotor angle." ) );
                }

                if ( unsteady_h )
                {
                    res->Add( new NameValData( "H", H, "Unsteady heave." ) );
                }

            }

        } // end of wake iteration

    } //end feof loop to read entire history file

    fclose ( fp );

    return;
}

/*******************************************************
Read .polar file output from VSPAERO
See: VSP_Solver.C in vspaero project
*******************************************************/
void VSPAEROMgrSingleton::ReadPolarFile( const string &filename, vector <string> &res_id_vector, double recref )
{
    FILE *fp = nullptr;
    bool read_success = false;
    WaitForFile( filename );
    fp = fopen( filename.c_str(), "r" );
    if ( fp == nullptr )
    {
        fprintf( stderr, "ERROR %d: Could not open Polar file: %s\n\tFile: %s \tLine:%d\n", vsp::VSP_FILE_DOES_NOT_EXIST, m_PolarFile.c_str(), __FILE__, __LINE__ );
        return;
    }

    Results* res = nullptr;

    std::vector<string> data_string_array;

    int num_polar_col = 23; // number of columns in the file

    double tol = 1e-8; // tolerance for comparing values to account for machine precision errors
    int num_history_res = ResultsMgr.GetNumResults( "VSPAERO_History" );

    // Read in all of the data into the results manager
    char seps[] = " :,\t\n";
    while ( !feof( fp ) )
    {
        data_string_array = ReadDelimLine( fp, seps ); //this is also done in some of the embedded loops below

        if ( num_polar_col == data_string_array.size() )
        {
            if ( data_string_array[0].find( "Beta" ) != std::string::npos )
            {
                res = ResultsMgr.CreateResults( "VSPAERO_Polar", "VSPAERO polar file results." );

                if ( res )
                {
                    data_string_array = ReadDelimLine( fp, seps );

                    std::vector<double> Beta;
                    std::vector<double> Mach;
                    std::vector<double> Alpha;
                    std::vector<double> Re_1e6;
                    std::vector<double> CL;
                    std::vector<double> CDo;
                    std::vector<double> CDi;
                    std::vector<double> CDtot;
                    std::vector<double> CDt;
                    std::vector<double> CDtott;
                    std::vector<double> CS;
                    std::vector<double> L_D;
                    std::vector<double> E;
                    std::vector<double> CFx;
                    std::vector<double> CFy;
                    std::vector<double> CFz;
                    std::vector<double> CMx;
                    std::vector<double> CMy;
                    std::vector<double> CMz;
                    std::vector<double> CMl;
                    std::vector<double> CMm;
                    std::vector<double> CMn;
                    std::vector<double> Fopt;

                    while ( num_polar_col == data_string_array.size() )
                    {
                        int icol = 0;
                        Beta.push_back(   std::stod( data_string_array[icol] ) ); icol++;
                        Mach.push_back(   std::stod( data_string_array[icol] ) ); icol++;
                        Alpha.push_back(  std::stod( data_string_array[icol] ) ); icol++;
                        Re_1e6.push_back( std::stod( data_string_array[icol] ) ); icol++;
                        CL.push_back(     std::stod( data_string_array[icol] ) ); icol++;
                        CDo.push_back(    std::stod( data_string_array[icol] ) ); icol++;
                        CDi.push_back(    std::stod( data_string_array[icol] ) ); icol++;
                        CDtot.push_back(  std::stod( data_string_array[icol] ) ); icol++;
                        CDt.push_back(    std::stod( data_string_array[icol] ) ); icol++;
                        CDtott.push_back( std::stod( data_string_array[icol] ) ); icol++;
                        CS.push_back(     std::stod( data_string_array[icol] ) ); icol++;
                        L_D.push_back(    std::stod( data_string_array[icol] ) ); icol++;
                        E.push_back(      std::stod( data_string_array[icol] ) ); icol++;
                        CFx.push_back(    std::stod( data_string_array[icol] ) ); icol++;
                        CFy.push_back(    std::stod( data_string_array[icol] ) ); icol++;
                        CFz.push_back(    std::stod( data_string_array[icol] ) ); icol++;
                        CMx.push_back(    std::stod( data_string_array[icol] ) ); icol++;
                        CMy.push_back(    std::stod( data_string_array[icol] ) ); icol++;
                        CMz.push_back(    std::stod( data_string_array[icol] ) ); icol++;
                        CMl.push_back(    std::stod( data_string_array[icol] ) ); icol++;
                        CMm.push_back(    std::stod( data_string_array[icol] ) ); icol++;
                        CMn.push_back(    std::stod( data_string_array[icol] ) ); icol++;
                        Fopt.push_back(   std::stod( data_string_array[icol] ) ); icol++;

                        if ( ( abs( ( 1e6 * Re_1e6.back() ) - recref ) > tol ) && num_history_res > 0 )
                        {
                            // Find history result with matching mach, beta, and alpha
                            for ( size_t i = 0; i < num_history_res; i++ )
                            {
                                Results* history_res = ResultsMgr.FindResults( "VSPAERO_History", i );

                                if ( !history_res )
                                {
                                    continue;
                                }

                                NameValData* mach_ptr = history_res->FindPtr( "FC_Mach_" );
                                NameValData* alpha_ptr = history_res->FindPtr( "Alpha" );
                                NameValData* beta_ptr = history_res->FindPtr( "FC_Beta_" );

                                if ( !mach_ptr || !alpha_ptr || !beta_ptr )
                                {
                                    continue;
                                }

                                double mach = mach_ptr->GetDouble( 0 );
                                double alpha = alpha_ptr->GetDouble( 0 );
                                double beta = beta_ptr->GetDouble( 0 );

                                if ( mach <= ( 0.001 + tol ) )
                                {
                                    // Mach is reported as 0 in the polar but 0.001 in the history file
                                    mach = 0;
                                }

                                if ( ( abs( mach - Mach.back() ) < tol ) && ( abs( alpha - Alpha.back() ) < tol ) && ( abs( beta - Beta.back() ) < tol ) )
                                {
                                    // Generate new *.history results for multiple ReCref inputs since VSPAERO only outputs a result for the first ReCref
                                    Results* new_history_res = ResultsMgr.CreateResults( "VSPAERO_History", "VSPAERO additional history results to capture ReCref variation." );
                                    res_id_vector.push_back( new_history_res->GetID() );

                                    new_history_res->Add( new NameValData( "FC_ReCref_", ( 1e6 * Re_1e6.back() ), "Reynolds number." ) );

                                    int num_wake = (int)alpha_ptr->GetDoubleData().size();

                                    NameValData* cdo_ptr = history_res->FindPtr( "CDo" );
                                    NameValData* cdtot_ptr = history_res->FindPtr( "CDtot" );
                                    NameValData* l_d_ptr = history_res->FindPtr( "L/D" );

                                    vector < string > data_names = history_res->GetAllDataNames();

                                    // Copy ReCref dependent results from polar to history file. Copy non-dependent results from 
                                    // history case that matches alpha, beta, and mach
                                    for ( size_t j = 0; j < data_names.size(); j++ )
                                    {
                                        // Calculate wake iteration convergence differences - ReCref scales CDo, CDtot, and L_D
                                        if ( cdo_ptr && strcmp( data_names[j].c_str(), "CDo" ) == 0 )
                                        {
                                            vector < double > history_cdo_vec = cdo_ptr->GetDoubleData();
                                            vector < double > cdo_vec( num_wake, CDo.back() );

                                            for ( size_t k = 0; k < history_cdo_vec.size() - 1; k++ )
                                            {
                                                cdo_vec[k] = cdo_vec[k] - ( history_cdo_vec.back() - history_cdo_vec[k] );
                                            }

                                            new_history_res->Add( new NameValData( data_names[j].c_str(), cdo_vec, "Parasite drag coefficient." ) );
                                        }
                                        else if ( cdtot_ptr && strcmp( data_names[j].c_str(), "CDtot" ) == 0 )
                                        {
                                            vector < double > history_ctot_vec = cdtot_ptr->GetDoubleData();
                                            vector < double > ctot_vec( num_wake, CDtot.back() );

                                            for ( size_t k = 0; k < history_ctot_vec.size() - 1; k++ )
                                            {
                                                ctot_vec[k] = ctot_vec[k] - ( history_ctot_vec.back() - history_ctot_vec[k] );
                                            }

                                            new_history_res->Add( new NameValData( data_names[j].c_str(), ctot_vec, "Total drag coefficient." ) );
                                        }
                                        else if ( l_d_ptr && strcmp( data_names[j].c_str(), "L/D" ) == 0 )
                                        {
                                            vector < double > history_l_d_vec = l_d_ptr->GetDoubleData();
                                            vector < double > ld_vec( num_wake, L_D.back() );

                                            for ( size_t k = 0; k < history_l_d_vec.size() - 1; k++ )
                                            {
                                                ld_vec[k] = ld_vec[k] - ( history_l_d_vec.back() - history_l_d_vec[k] );
                                            }

                                            new_history_res->Add( new NameValData( data_names[j].c_str(), ld_vec, "Lift to drag ratio." ) );
                                        }
                                        else if ( strcmp( data_names[j].c_str(), "FC_ReCref_" ) != 0 )
                                        {
                                            NameValData* nvd = history_res->FindPtr( data_names[j] );
                                            if ( !nvd )
                                            {
                                                continue;
                                            }

                                            NameValData* nvdcopy = new NameValData();
                                            nvdcopy->CopyFrom( nvd );

                                            new_history_res->Add( nvdcopy ); // q: do we need a copy here?
                                        }
                                    }

                                    break;
                                }
                            }
                        }

                        data_string_array = ReadDelimLine( fp, seps );
                    }

                    res->Add( new NameValData( "Beta", Beta, "Angle of sideslip." ) );
                    res->Add( new NameValData( "Mach", Mach, "Mach number." ) );
                    res->Add( new NameValData( "Alpha", Alpha, "Angle of attack." ) );
                    res->Add( new NameValData( "Re_1e6", Re_1e6, "Reynolds number in millions." ) );
                    res->Add( new NameValData( "CL", CL, "Lift coefficient." ) );
                    res->Add( new NameValData( "CDo", CDo, "Parasite drag coefficient." ) );
                    res->Add( new NameValData( "CDi", CDi, "Induced drag coefficient." ) );
                    res->Add( new NameValData( "CDtot", CDtot, "Total drag coefficient." ) );
                    res->Add( new NameValData( "CDt", CDt, "Induced drag coefficient from Trefftz-like calculation." ) );
                    res->Add( new NameValData( "CDtott", CDtott, "Total drag coefficient from Trefftz-like calculation." ) );
                    res->Add( new NameValData( "CS", CS, "Side force coefficient." ) );
                    res->Add( new NameValData( "L_D", L_D, "Lift to drag ratio." ) );
                    res->Add( new NameValData( "E", E, "Oswald efficiency factor." ) );
                    res->Add( new NameValData( "CFx", CFx, "X force coefficient." ) );
                    res->Add( new NameValData( "CFy", CFy, "Y force coefficient." ) );
                    res->Add( new NameValData( "CFz", CFz, "Z force coefficient." ) );
                    res->Add( new NameValData( "CMx", CMx, "X moment coefficient." ) );
                    res->Add( new NameValData( "CMy", CMy, "Y moment coefficient." ) );
                    res->Add( new NameValData( "CMz", CMz, "Z moment coefficient." ) );
                    res->Add( new NameValData( "CMl", CMl, "L roll moment coefficient." ) );
                    res->Add( new NameValData( "CMm", CMm, "M pitch moment coefficient." ) );
                    res->Add( new NameValData( "CMn", CMn, "N yaw moment coefficient." ) );
                    res->Add( new NameValData( "Fopt", Fopt, "Objective function value." ) );

                    // Add results at the end to keep new VSPAERO_HIstory results together in the CSV export
                    res_id_vector.push_back( res->GetID() );
                }
            }
        }

    } //end for while !feof(fp)

    std::fclose( fp );

    return;
}

/*******************************************************
Read .LOD file output from VSPAERO
See: VSP_Solver.C in vspaero project
line 2851 - void VSP_SOLVER::CalculateSpanWiseLoading(void)
TODO:
- Update this function to use the generic table read as used in: string VSPAEROMgrSingleton::ReadStabFile()
*******************************************************/
void VSPAEROMgrSingleton::ReadLoadFile( const string &filename, vector <string> &res_id_vector )
{
    FILE *fp = nullptr;
    bool read_success = false;

    //LOAD file
    WaitForFile( filename );
    fp = fopen( filename.c_str(), "r" );
    if ( fp == nullptr )
    {
        fprintf( stderr, "ERROR %d: Could not open Load file: %s\n\tFile: %s \tLine:%d\n", vsp::VSP_FILE_DOES_NOT_EXIST, m_LoadFile.c_str(), __FILE__, __LINE__ );
        return;
    }

    Results* res = nullptr;
    std::vector< std::string > data_string_array;
    bool sectional_data_complete = false; // flag indicating if the sectional data section of the Lod file has been read
    bool remnant_data_complete = false;

    double cref = 1.0;

    char seps[]   = " :,\t\n";
    while ( !feof( fp ) )
    {
        data_string_array = ReadDelimLine( fp, seps ); //this is also done in some of the embedded loops below

        if ( CheckForCaseHeader( data_string_array ) )
        {
            res = ResultsMgr.CreateResults( "VSPAERO_Load", "VSPAERO load distribution lod file results." );
            res_id_vector.push_back( res->GetID() );

            if ( ReadVSPAEROCaseHeader( res, fp ) != 0 )
            {
                // Failed to read the case header
                fprintf( stderr, "ERROR %d: Could not read case header in VSPAERO file: %s\n\tFile: %s \tLine:%d\n", vsp::VSP_FILE_READ_FAILURE, m_StabFile.c_str(), __FILE__, __LINE__ );
                return;
            }

            cref = res->FindPtr( "FC_Cref_" )->GetDouble( 0 );

        }

        // Sectional distribution table
        int nSectionalDataTableCols = 36;
        int nCompDataTableCols = 15;
        if ( data_string_array.size() == nSectionalDataTableCols && ( !sectional_data_complete || !remnant_data_complete ) && !isdigit( data_string_array[0][0] ) )
        {

            if ( sectional_data_complete )
            {
                res = ResultsMgr.CreateResults( "VSPAERO_Remnant_Load", "VSPAERO remnant loads from lod file results." );
                res_id_vector.push_back( res->GetID());
            }

            //discard the header row and read the next line assuming that it is numeric
            data_string_array = ReadDelimLine( fp, seps );

            // Raw data vectors
            std::vector<int> SpanLoadSet;
            std::vector<int> Surface;
            std::vector<double> S;
            std::vector<double> Xavg;
            std::vector<double> Yavg;
            std::vector<double> Zavg;
            std::vector<double> Area;
            std::vector<double> Chord;
            std::vector<double> VoVref;
            std::vector<double> Cl;
            std::vector<double> Cd;
            std::vector<double> Cs;
            std::vector<double> Clo;
            std::vector<double> Cdo;
            std::vector<double> Cso;
            std::vector<double> Cli;
            std::vector<double> Cdi;
            std::vector<double> Csi;
            std::vector<double> Cx;
            std::vector<double> Cy;
            std::vector<double> Cz;
            std::vector<double> Cxo;
            std::vector<double> Cyo;
            std::vector<double> Czo;
            std::vector<double> Cxi;
            std::vector<double> Cyi;
            std::vector<double> Czi;
            std::vector<double> Cmx;
            std::vector<double> Cmy;
            std::vector<double> Cmz;
            std::vector<double> Cmxo;
            std::vector<double> Cmyo;
            std::vector<double> Cmzo;
            std::vector<double> Cmxi;
            std::vector<double> Cmyi;
            std::vector<double> Cmzi;

            //normalized by local chord
            std::vector<double> Clc_cref;
            std::vector<double> Cdc_cref;
            std::vector<double> Csc_cref;
            std::vector<double> Cloc_cref;
            std::vector<double> Cdoc_cref;
            std::vector<double> Csoc_cref;
            std::vector<double> Clic_cref;
            std::vector<double> Cdic_cref;
            std::vector<double> Csic_cref;
            std::vector<double> Cxc_cref;
            std::vector<double> Cyc_cref;
            std::vector<double> Czc_cref;
            std::vector<double> Cxoc_cref;
            std::vector<double> Cyoc_cref;
            std::vector<double> Czoc_cref;
            std::vector<double> Cxic_cref;
            std::vector<double> Cyic_cref;
            std::vector<double> Czic_cref;
            std::vector<double> Cmxc_cref;
            std::vector<double> Cmyc_cref;
            std::vector<double> Cmzc_cref;
            std::vector<double> Cmxoc_cref;
            std::vector<double> Cmyoc_cref;
            std::vector<double> Cmzoc_cref;
            std::vector<double> Cmxic_cref;
            std::vector<double> Cmyic_cref;
            std::vector<double> Cmzic_cref;

            double chordRatio;

            // read the data rows
            while ( data_string_array.size() == nSectionalDataTableCols )
            {
                // Store the raw data
                SpanLoadSet.push_back( std::stoi( data_string_array[0] ) );
                Surface.push_back(     std::stoi( data_string_array[1] ) );
                S.push_back(           std::stod( data_string_array[2] ) );
                Xavg.push_back(        std::stod( data_string_array[3] ) );
                Yavg.push_back(        std::stod( data_string_array[4] ) );
                Zavg.push_back(        std::stod( data_string_array[5] ) );
                Area.push_back(        std::stod( data_string_array[6] ) );
                Chord.push_back(       std::stod( data_string_array[7] ) );
                VoVref.push_back(      std::stod( data_string_array[8] ) );
                Cl.push_back(          std::stod( data_string_array[9] ) );
                Cd.push_back(          std::stod( data_string_array[10] ) );
                Cs.push_back(          std::stod( data_string_array[11] ) );
                Clo.push_back(         std::stod( data_string_array[12] ) );
                Cdo.push_back(         std::stod( data_string_array[13] ) );
                Cso.push_back(         std::stod( data_string_array[14] ) );
                Cli.push_back(         std::stod( data_string_array[15] ) );
                Cdi.push_back(         std::stod( data_string_array[16] ) );
                Csi.push_back(         std::stod( data_string_array[17] ) );
                Cx.push_back(          std::stod( data_string_array[18] ) );
                Cy.push_back(          std::stod( data_string_array[19] ) );
                Cz.push_back(          std::stod( data_string_array[20] ) );
                Cxo.push_back(         std::stod( data_string_array[21] ) );
                Cyo.push_back(         std::stod( data_string_array[22] ) );
                Czo.push_back(         std::stod( data_string_array[23] ) );
                Cxi.push_back(         std::stod( data_string_array[24] ) );
                Cyi.push_back(         std::stod( data_string_array[25] ) );
                Czi.push_back(         std::stod( data_string_array[26] ) );
                Cmx.push_back(         std::stod( data_string_array[27] ) );
                Cmy.push_back(         std::stod( data_string_array[28] ) );
                Cmz.push_back(         std::stod( data_string_array[29] ) );
                Cmxo.push_back(        std::stod( data_string_array[30] ) );
                Cmyo.push_back(        std::stod( data_string_array[31] ) );
                Cmzo.push_back(        std::stod( data_string_array[32] ) );
                Cmxi.push_back(        std::stod( data_string_array[33] ) );
                Cmyi.push_back(        std::stod( data_string_array[34] ) );
                Cmzi.push_back(        std::stod( data_string_array[35] ) );

                chordRatio = Chord.back() / cref;

                // Normalized by local chord
                Clc_cref.push_back( Cl.back() * chordRatio );
                Cdc_cref.push_back( Cd.back() * chordRatio );
                Csc_cref.push_back( Cs.back() * chordRatio );
                Cloc_cref.push_back( Clo.back() * chordRatio );
                Cdoc_cref.push_back( Cdo.back() * chordRatio );
                Csoc_cref.push_back( Cso.back() * chordRatio );
                Clic_cref.push_back( Cli.back() * chordRatio );
                Cdic_cref.push_back( Cdi.back() * chordRatio );
                Csic_cref.push_back( Csi.back() * chordRatio );
                Cxc_cref.push_back( Cx.back() * chordRatio );
                Cyc_cref.push_back( Cy.back() * chordRatio );
                Czc_cref.push_back( Cz.back() * chordRatio );
                Cxoc_cref.push_back( Cxo.back() * chordRatio );
                Cyoc_cref.push_back( Cyo.back() * chordRatio );
                Czoc_cref.push_back( Czo.back() * chordRatio );
                Cxic_cref.push_back( Cxi.back() * chordRatio );
                Cyic_cref.push_back( Cyi.back() * chordRatio );
                Czic_cref.push_back( Czi.back() * chordRatio );
                Cmxc_cref.push_back( Cmx.back() * chordRatio );
                Cmyc_cref.push_back( Cmy.back() * chordRatio );
                Cmzc_cref.push_back( Cmz.back() * chordRatio );
                Cmxoc_cref.push_back( Cmxo.back() * chordRatio );
                Cmyoc_cref.push_back( Cmyo.back() * chordRatio );
                Cmzoc_cref.push_back( Cmzo.back() * chordRatio );
                Cmxic_cref.push_back( Cmxi.back() * chordRatio );
                Cmyic_cref.push_back( Cmyi.back() * chordRatio );
                Cmzic_cref.push_back( Cmzi.back() * chordRatio );

                // Read the next line and loop
                data_string_array = ReadDelimLine( fp, seps );
            }

            // Finish up by adding the data to the result res
            res->Add( new NameValData( "SpanLoadSet", SpanLoadSet, "Span load distribution set." ) );
            res->Add( new NameValData( "Surface", Surface, "Surface for span load set." ) );
            res->Add( new NameValData( "S", S, "Non-dimensional spanwise coordinate." ) );
            res->Add( new NameValData( "Xavg", Xavg, "Section X coordinate." ) );
            res->Add( new NameValData( "Yavg", Yavg, "Section Y coordinate." ) );
            res->Add( new NameValData( "Zavg", Zavg, "Section Z coordinate." ) );
            res->Add( new NameValData( "Area", Area, "Section area." ) );
            res->Add( new NameValData( "Chord", Chord, "Section chord." ) );
            res->Add( new NameValData( "V/Vref", VoVref, "Local velocity ratio." ) );
            res->Add( new NameValData( "cl", Cl, "Section lift coefficient." ) );
            res->Add( new NameValData( "cd", Cd, "Section drag coefficient." ) );
            res->Add( new NameValData( "cs", Cs, "Section side force coefficient." ) );
            res->Add( new NameValData( "clo", Clo, "Section lift coefficient.  Parasite part." ) );
            res->Add( new NameValData( "cdo", Cdo, "Section drag coefficient.  Parasite part." ) );
            res->Add( new NameValData( "cso", Cso, "Section side force coefficient.  Parasite part." ) );
            res->Add( new NameValData( "cli", Cli, "Section lift coefficient.  Induced part." ) );
            res->Add( new NameValData( "cdi", Cdi, "Section drag coefficient.  Induced part." ) );
            res->Add( new NameValData( "csi", Csi, "Section side force coefficient.  Induced part." ) );
            res->Add( new NameValData( "cx", Cx, "Section X force coefficient." ) );
            res->Add( new NameValData( "cy", Cy, "Section Y force coefficient." ) );
            res->Add( new NameValData( "cz", Cz, "Section Z force coefficient." ) );
            res->Add( new NameValData( "cxo", Cxo, "Section X force coefficient.  Parasite part." ) );
            res->Add( new NameValData( "cyo", Cyo, "Section Y force coefficient.  Parasite part." ) );
            res->Add( new NameValData( "czo", Czo, "Section Z force coefficient.  Parasite part." ) );
            res->Add( new NameValData( "cxi", Cxi, "Section X force coefficient.  Induced part." ) );
            res->Add( new NameValData( "cyi", Cyi, "Section Y force coefficient.  Induced part." ) );
            res->Add( new NameValData( "czi", Czi, "Section Z force coefficient.  Induced part." ) );
            res->Add( new NameValData( "cmx", Cmx, "Section X moment coefficient." ) );
            res->Add( new NameValData( "cmy", Cmy, "Section Y moment coefficient." ) );
            res->Add( new NameValData( "cmz", Cmz, "Section Z moment coefficient." ) );
            res->Add( new NameValData( "cmxo", Cmxo, "Section X moment coefficient.  Parasite part." ) );
            res->Add( new NameValData( "cmyo", Cmyo, "Section Y moment coefficient.  Parasite part." ) );
            res->Add( new NameValData( "cmzo", Cmzo, "Section Z moment coefficient.  Parasite part." ) );
            res->Add( new NameValData( "cmxi", Cmxi, "Section X moment coefficient.  Induced part." ) );
            res->Add( new NameValData( "cmyi", Cmyi, "Section Y moment coefficient.  Induced part." ) );
            res->Add( new NameValData( "cmzi", Cmzi, "Section Z moment coefficient.  Induced part." ) );

            res->Add( new NameValData( "cl*c/cref", Clc_cref, "Section lift scaled load." ) );
            res->Add( new NameValData( "cd*c/cref", Cdc_cref, "Section drag scaled load." ) );
            res->Add( new NameValData( "cs*c/cref", Csc_cref, "Section side scaled load." ) );
            res->Add( new NameValData( "clo*c/cref", Cloc_cref, "Section lift scaled load.  Parasite part." ) );
            res->Add( new NameValData( "cdo*c/cref", Cdoc_cref, "Section drag scaled load.  Parasite part." ) );
            res->Add( new NameValData( "cso*c/cref", Csoc_cref, "Section side scaled load.  Parasite part." ) );
            res->Add( new NameValData( "cli*c/cref", Clic_cref, "Section lift scaled load.  Induced part." ) );
            res->Add( new NameValData( "cdi*c/cref", Cdic_cref, "Section drag scaled load.  Induced part." ) );
            res->Add( new NameValData( "csi*c/cref", Csic_cref, "Section side scaled load.  Induced part." ) );
            res->Add( new NameValData( "cx*c/cref", Cxc_cref, "Section X scaled load." ) );
            res->Add( new NameValData( "cy*c/cref", Cyc_cref, "Section Y scaled load." ) );
            res->Add( new NameValData( "cz*c/cref", Czc_cref, "Section Z scaled load." ) );
            res->Add( new NameValData( "cxo*c/cref", Cxoc_cref, "Section X scaled load.  Parasite part." ) );
            res->Add( new NameValData( "cyo*c/cref", Cyoc_cref, "Section Y scaled load.  Parasite part." ) );
            res->Add( new NameValData( "czo*c/cref", Czoc_cref, "Section Z scaled load.  Parasite part." ) );
            res->Add( new NameValData( "cxi*c/cref", Cxic_cref, "Section X scaled load.  Induced part." ) );
            res->Add( new NameValData( "cyi*c/cref", Cyic_cref, "Section Y scaled load.  Induced part." ) );
            res->Add( new NameValData( "czi*c/cref", Czic_cref, "Section Z scaled load.  Induced part." ) );
            res->Add( new NameValData( "cmx*c/cref", Cmxc_cref, "Section X scaled moment." ) );
            res->Add( new NameValData( "cmy*c/cref", Cmyc_cref, "Section Y scaled moment." ) );
            res->Add( new NameValData( "cmz*c/cref", Cmzc_cref, "Section Z scaled moment." ) );
            res->Add( new NameValData( "cmxo*c/cref", Cmxoc_cref, "Section X scaled moment.  Parasite part." ) );
            res->Add( new NameValData( "cmyo*c/cref", Cmyoc_cref, "Section Y scaled moment.  Parasite part." ) );
            res->Add( new NameValData( "cmzo*c/cref", Cmzoc_cref, "Section Z scaled moment.  Parasite part." ) );
            res->Add( new NameValData( "cmxi*c/cref", Cmxic_cref, "Section X scaled moment.  Induced part." ) );
            res->Add( new NameValData( "cmyi*c/cref", Cmyic_cref, "Section Y scaled moment.  Induced part." ) );
            res->Add( new NameValData( "cmzi*c/cref", Cmzic_cref, "Section Z scaled moment.  Induced part." ) );

            if ( !sectional_data_complete )
            {
                // First time through.
                sectional_data_complete = true;
            }
            else
            {
                // Second time through
                remnant_data_complete = true;
            }

        } // end sectional and remnant table read
        else if ( data_string_array.size() == nCompDataTableCols && sectional_data_complete && remnant_data_complete )
        {
            // "Comp" section of *.lod file
            res = ResultsMgr.CreateResults( "VSPAERO_Comp_Load", "VSPAERO component loads from lod file results." );
            res_id_vector.push_back( res->GetID() );

            //discard the header row and read the next line assuming that it is numeric
            data_string_array = ReadDelimLine( fp, seps );

            // Raw data vectors
            std::vector<int> SpanLoadSet;
            std::vector<int> Surface;
            std::vector<string> Comp_Name;
            std::vector<double> Mach;
            std::vector<double> AoA;
            std::vector<double> Beta;
            std::vector<double> CL;
            std::vector<double> CDi;
            std::vector<double> Cs;
            std::vector<double> CFx;
            std::vector<double> CFy;
            std::vector<double> CFz;
            std::vector<double> Cmx;
            std::vector<double> Cmy;
            std::vector<double> Cmz;

            // read the data rows
            while ( ( data_string_array.size() >= nCompDataTableCols - 1 ) && ( data_string_array.size() <= nCompDataTableCols ) )
            {
                // Store the raw data
                size_t j = 0;
                SpanLoadSet.push_back( std::stoi( data_string_array[j++] ) );
                Surface.push_back( std::stoi( data_string_array[j++] ) );

                if ( data_string_array.size() == nCompDataTableCols - 1 )
                {
                    // Condition if no body-type components in *.vspgeom input
                    Comp_Name.push_back( "NONE" );
                }
                else
                {
                    Comp_Name.push_back( data_string_array[j++] );
                }

                Mach.push_back( std::stod( data_string_array[j++] ) );
                AoA.push_back( std::stod( data_string_array[j++] ) );
                Beta.push_back( std::stod( data_string_array[j++] ) );
                CL.push_back( std::stod( data_string_array[j++] ) );
                CDi.push_back( std::stod( data_string_array[j++] ) );
                Cs.push_back( std::stod( data_string_array[j++] ) );
                CFx.push_back( std::stod( data_string_array[j++] ) );
                CFy.push_back( std::stod( data_string_array[j++] ) );
                CFz.push_back( std::stod( data_string_array[j++] ) );
                Cmx.push_back( std::stod( data_string_array[j++] ) );
                Cmy.push_back( std::stod( data_string_array[j++] ) );
                Cmz.push_back( std::stod( data_string_array[j++] ) );

                // Read the next line and loop
                data_string_array = ReadDelimLine( fp, seps );
            }

            // Finish up by adding the data to the result res
            res->Add( new NameValData( "SpanLoadSet", SpanLoadSet, "Span load distribution set." ) );
            res->Add( new NameValData( "Surface", Surface, "Surface for span load set." ) );
            res->Add( new NameValData( "Comp_Name", Comp_Name, "Component name." ) );
            res->Add( new NameValData( "Mach", Mach, "Mach number." ) );
            res->Add( new NameValData( "AoA", AoA, "Angle of attack." ) );
            res->Add( new NameValData( "Beta", Beta, "Angle of sideslip." ) );
            res->Add( new NameValData( "CL", CL, "Lift coefficient." ) );
            res->Add( new NameValData( "CDi", CDi, "Induced drag coefficient." ) );
            res->Add( new NameValData( "Cs", Cs, "Side force coefficient." ) );
            res->Add( new NameValData( "CFx", CFx, "X force coefficient." ) );
            res->Add( new NameValData( "CFy", CFy, "Y force coefficient." ) );
            res->Add( new NameValData( "CFz", CFz, "Z force coefficient." ) );
            res->Add( new NameValData( "Cmx", Cmx, "X moment coefficient." ) );
            res->Add( new NameValData( "Cmy", Cmy, "Y moment coefficient." ) );
            res->Add( new NameValData( "Cmz", Cmz, "Z moment coefficient." ) );

            sectional_data_complete = false;
        } // end total component table read

    } // end file loop

    std::fclose ( fp );

    return;
}

/*******************************************************
Read .STAB file output from VSPAERO
See: VSP_Solver.C in vspaero project
*******************************************************/
void VSPAEROMgrSingleton::ReadStabFile( const string &filename, vector <string> &res_id_vector, vsp::VSPAERO_STABILITY_TYPE stabilityType )
{
    FILE *fp = nullptr;
    bool read_success = false;
    WaitForFile( filename );
    fp = fopen( filename.c_str() , "r" );
    if ( fp == nullptr )
    {
        fprintf( stderr, "ERROR %d: Could not open Stab file: %s\n\tFile: %s \tLine:%d\n", vsp::VSP_FILE_DOES_NOT_EXIST, m_StabFile.c_str(), __FILE__, __LINE__ );
        return;
    }

    Results* res = nullptr;

    std::vector<string> table_column_names;
    std::vector<string> data_string_array;

    // Read in all of the data into the results manager
    char seps[] = " :,\t\n()";
    while ( !feof( fp ) )
    {
        data_string_array = ReadDelimLine( fp, seps ); //this is also done in some of the embedded loops below

        if ( CheckForCaseHeader( data_string_array ) )
        {
            res = ResultsMgr.CreateResults( "VSPAERO_Stab", "VSPAERO stability run results." );
            res->Add( new NameValData( "StabilityType", stabilityType, "Stability and control mode enum." ) );
            res_id_vector.push_back( res->GetID() );

            if ( ReadVSPAEROCaseHeader( res, fp ) != 0 )
            {
                // Failed to read the case header
                fprintf( stderr, "ERROR %d: Could not read case header in VSPAERO file: %s\n\tFile: %s \tLine:%d\n", vsp::VSP_FILE_READ_FAILURE, m_StabFile.c_str(), __FILE__, __LINE__ );
                std::fclose ( fp );
                return;
            }
        }
        else if ( res && CheckForResultHeader( data_string_array ) )
        {
            data_string_array = ReadDelimLine( fp, seps );

            // Read result table
            double value;

            // Parse if this is not a comment line
            while ( !feof( fp ) && strncmp( data_string_array[0].c_str(), "#", 1 ) != 0 )
            {
                if ( ( data_string_array.size() == 3 ) )
                {
                    // assumption that the 2nd entry is a number
                    if ( sscanf( data_string_array[1].c_str(), "%lf", &value ) == 1 )
                    {
                        res->Add( new NameValData( data_string_array[0], value, "#Auto parsed stab result." ) );
                    }
                }

                // read the next line
                data_string_array = ReadDelimLine( fp, seps );
            } // end while
        }
        else if ( data_string_array.size() > 0 )
        {
            // Parse if this is not a comment line
            if ( res && strncmp( data_string_array[0].c_str(), "#", 1 ) != 0 )
            {
                if ( stabilityType != vsp::STABILITY_DEFAULT )
                {
                    // Support file format for P, Q, or R uinsteady analysis types
                    // Also works for pitch stability run.
                    string name = data_string_array[0];

                    for ( unsigned int i_field = 1; i_field < data_string_array.size(); i_field++ )
                    {
                        //attempt to read a double if that fails then treat it as a string result and add to result name to account for spaces
                        double temp_val = 0;
                        int result = 0;
                        result = sscanf( data_string_array[i_field].c_str(), "%lf", &temp_val );

                        if ( result == 1 )
                        {
                            res->Add( new NameValData( name, temp_val, "#Auto parsed stab result." ) );
                        }
                        else
                        {
                            name += data_string_array[i_field];
                        }
                    }
                }
                else
                {
                    //================ Table Data ================//
                    // Checks for table header format
                    if ( ( data_string_array.size() != table_column_names.size() ) || ( table_column_names.size() == 0 ) )
                    {
                        //Indicator that the data table has changed or has not been initialized.
                        table_column_names.clear();
                        table_column_names = data_string_array;

                        // map control group names to full control surface group names
                        int i_field_offset = -1;
                        for ( unsigned int i_field = 0; i_field < data_string_array.size(); i_field++ )
                        {
                            if ( strstr( table_column_names[i_field].c_str(), "ConGrp_" ) )
                            {
                                //  Set field offset based on the first ConGrp_ found
                                if ( i_field_offset == -1 )
                                {
                                    i_field_offset = i_field;
                                }

                                if ( m_Verbose )
                                {
                                    printf( "\tMapping table col name to CSG name: \n" );
                                }
                                if ( m_Verbose )
                                {
                                    printf( "\ti_field = %u --> i_field_offset = %u\n", i_field, i_field - i_field_offset );
                                }
                                if ( ( i_field - i_field_offset ) < m_ControlSurfaceGroupVec.size() )
                                {
                                    if ( m_Verbose )
                                    {
                                        printf( "\t%s --> %s\n", table_column_names[i_field].c_str(), m_ControlSurfaceGroupVec[i_field - i_field_offset]->GetName().c_str() );
                                    }
                                    table_column_names[i_field] = m_ControlSurfaceGroupVec[i_field - i_field_offset]->GetName();
                                }
                                else
                                {
                                    printf( "\tERROR (i_field - i_field_offset) > m_ControlSurfaceGroupVec.size()\n" );
                                    printf( "\t      (  %u    -    %d         ) >            %zu             \n", i_field, i_field_offset, m_ControlSurfaceGroupVec.size() );
                                }

                            }
                        }

                    }
                    else
                    {
                        //This is a continuation of the current table and add this row to the results manager
                        for ( unsigned int i_field = 1; i_field < data_string_array.size(); i_field++ )
                        {
                            //attempt to read a double if that fails then treat it as a string result
                            double temp_val = 0;
                            int result = 0;
                            result = sscanf( data_string_array[i_field].c_str(), "%lf", &temp_val );
                            if ( result == 1 )
                            {
                                res->Add( new NameValData( data_string_array[0] + "_" + table_column_names[i_field], temp_val, "#Auto parsed stab result." ) );
                            }
                            else
                            {
                                res->Add( new NameValData( data_string_array[0] + "_" + table_column_names[i_field], data_string_array[i_field], "#Auto parsed stab result." ) );
                            }
                        }
                    } //end new table check

                } // end unsteady check

            } // end comment line check

        } // end data_string_array.size()>0 check

    } //end for while !feof(fp)

    std::fclose ( fp );

    return;
}

vector <string> VSPAEROMgrSingleton::ReadDelimLine( FILE * fp, char * delimiters )
{

    vector <string> dataStringVector;
    dataStringVector.clear();

    char strbuff[1024];                // buffer for entire line in file
    if ( fgets( strbuff, 1024, fp ) != nullptr )
    {
        char * pch = strtok ( strbuff, delimiters );
        while ( pch != nullptr )
        {
            dataStringVector.push_back( pch );
            pch = strtok ( nullptr, delimiters );
        }
    }

    return dataStringVector;
}

bool VSPAEROMgrSingleton::CheckForCaseHeader( const std::vector<string> &headerStr )
{
    if ( headerStr.size() == 1 )
    {
        if ( strcmp( headerStr[0].c_str(), "*****************************************************************************************************************************************************************************************" ) == 0 )
        {
            return true;
        }
    }

    return false;
}

bool VSPAEROMgrSingleton::CheckForResultHeader( const std::vector<string> &headerStr )
{
    if ( headerStr.size() == 4 )
    {
        if ( strcmp( headerStr[0].c_str(), "#" ) == 0 && strcmp( headerStr[1].c_str(), "Result" ) == 0 )
        {
            return true;
        }
    }

    return false;
}

int VSPAEROMgrSingleton::ReadVSPAEROCaseHeader( Results * res, FILE * fp )
{
    // check input arguments
    if ( res == nullptr )
    {
        // Bad pointer
        fprintf( stderr, "ERROR %d: Invalid results pointer\n\tFile: %s \tLine:%d\n", vsp::VSP_INVALID_PTR, __FILE__, __LINE__ );
        return -1;
    }
    if ( fp == nullptr )
    {
        // Bad pointer
        fprintf( stderr, "ERROR %d: Invalid file pointer\n\tFile: %s \tLine:%d\n", vsp::VSP_INVALID_PTR, __FILE__, __LINE__ );
        return -2;
    }

    char seps[]   = " :,\t\n";
    std::vector<string> data_string_array;

    //skip any blank lines before the header
    while ( !feof( fp ) && data_string_array.size() == 0 )
    {
        data_string_array = ReadDelimLine( fp, seps ); //this is also done in some of the embedded loops below
    }

    // Read header table
    bool needs_header = true;
    bool mach_found = false;
    bool alpha_found = false;
    bool beta_found = false;
    double current_mach = -FLT_MAX;
    double current_alpha = -FLT_MAX;
    double current_beta = -FLT_MAX;
    double value;
    while ( !feof( fp ) && data_string_array.size() > 0 )
    {
        // Parse if this is not a comment line
        if ( ( strncmp( data_string_array[0].c_str(), "#", 1 ) != 0 ) && ( data_string_array.size() == 3 ) )
        {
            // assumption that the 2nd entry is a number
            if ( sscanf( data_string_array[1].c_str(), "%lf", &value ) == 1 )
            {
                res->Add( new NameValData( "FC_" + data_string_array[0], value, "#Auto parsed case flight condition." ) );

                // save flow condition information to be added to the header later
                if ( strcmp( data_string_array[0].c_str(), "Mach_" ) == 0 )
                {
                    current_mach = value;
                    mach_found = true;
                }
                if ( strcmp( data_string_array[0].c_str(), "AoA_" ) == 0 )
                {
                    current_alpha = value;
                    alpha_found = true;
                }
                if ( strcmp( data_string_array[0].c_str(), "Beta_" ) == 0 )
                {
                    current_beta = value;
                    beta_found = true;
                }

                // check if the information needed for the result header has been read in
                if ( mach_found && alpha_found && beta_found && needs_header )
                {
                    AddResultHeader( res->GetID(), current_mach, current_alpha, current_beta );
                    needs_header = false;
                }
            }
        }

        // read the next line
        data_string_array = ReadDelimLine( fp, seps );

    } // end while

    if ( needs_header )
    {
        fprintf( stderr, "WARNING: Case header incomplete \n\tFile: %s \tLine:%d\n", __FILE__, __LINE__ );
        return -3;
    }

    return 0; // no errors
}

//Export Results to CSV
int VSPAEROMgrSingleton::ExportResultsToCSV( const string &fileName )
{
    int retVal = vsp::VSP_FILE_WRITE_FAILURE;

    // Get the results
    string resId = ResultsMgr.FindLatestResultsID( "VSPAERO_Wrapper" );
    if ( resId == string() )
    {
        retVal = vsp::VSP_CANT_FIND_NAME;
        fprintf( stderr, "ERROR %d: Unable to find ""VSPAERO_Wrapper"" result \n\tFile: %s \tLine:%d\n", retVal, __FILE__, __LINE__ );
        return retVal;
    }

    Results* resptr = ResultsMgr.FindResultsPtr( resId );
    if ( !resptr )
    {
        retVal = vsp::VSP_INVALID_PTR;
        fprintf( stderr, "ERROR %d: Unable to get pointer to ""VSPAERO_Wrapper"" result \n\tFile: %s \tLine:%d\n", retVal, __FILE__, __LINE__ );
        return retVal;
    }

    // Get all the child results and write out to csv using a vector of results
    vector <string> resIdVector = ResultsMgr.GetStringResults( resId, "ResultsVec" );
    if ( resIdVector.size() == 0 )
    {
        fprintf( stderr, "WARNING %d: ""VSPAERO_Wrapper"" result contains no child results \n\tFile: %s \tLine:%d\n", retVal, __FILE__, __LINE__ );
    }

    // Get CP Slice results if available
    string CPresId = ResultsMgr.FindLatestResultsID( "CpSlice_Wrapper" );
    if ( CPresId != string() )
    {
        vector < string > caseIdVector = ResultsMgr.GetStringResults( CPresId, "CpSlice_Case_ID_Vec" );

        for ( size_t i = 0; i < caseIdVector.size(); i++ )
        {
            resIdVector.push_back( caseIdVector[i] );
        }
    }

    // Export to CSV file
    retVal = ResultsMgr.WriteCSVFile( fileName, resIdVector );

    // Check that the file made it to the file system and return status
    return WaitForFile( fileName );
}

void VSPAEROMgrSingleton::AddRotorDisk()
{
    RotorDisk* new_rd = new RotorDisk;
    new_rd->SetParentContainer( GetID() );
    m_RotorDiskVec.push_back( new_rd );
}

bool VSPAEROMgrSingleton::ValidRotorDiskIndex( int index )
{
    if ( ( index >= 0 ) && ( index < m_RotorDiskVec.size() ) && m_RotorDiskVec.size() > 0 )
    {
        return true;
    }

    return false;
}

void VSPAEROMgrSingleton::UpdateRotorDiskSuffix()
{
    for (int i = 0 ; i < (int) m_RotorDiskVec.size(); ++i)
    {
        m_RotorDiskVec[i]->SetParentContainer( GetID() );
        m_RotorDiskVec[i]->SetGroupDisplaySuffix( i );
    }
}

RotorDisk* VSPAEROMgrSingleton::GetRotorDisk( int index )
{
    if ( ValidRotorDiskIndex( index ) )
    {
        return m_RotorDiskVec[index];
    }
    return nullptr; // error
}

void VSPAEROMgrSingleton::UpdateControlSurfaceGroupSuffix()
{
    for (int i = 0 ; i < (int) m_ControlSurfaceGroupVec.size(); ++i)
    {
        m_ControlSurfaceGroupVec[i]->SetParentContainer( GetID() );
        m_ControlSurfaceGroupVec[i]->SetGroupDisplaySuffix( i );
    }
}

void VSPAEROMgrSingleton::AddControlSurfaceGroup()
{
    ControlSurfaceGroup* new_cs = new ControlSurfaceGroup;
    new_cs->SetParentContainer( GetID() );
    m_ControlSurfaceGroupVec.push_back( new_cs );

    m_CurrentCSGroupIndex = m_ControlSurfaceGroupVec.size() - 1;

    m_SelectedGroupedCS.clear();
    UpdateActiveControlSurfVec();

    HighlightSelected( CONTROL_SURFACE );
}

void VSPAEROMgrSingleton::RemoveControlSurfaceGroup()
{
    if ( m_CurrentCSGroupIndex != -1 )
    {
        for ( size_t i = 0; i < m_ActiveControlSurfaceVec.size(); ++i )
        {
            for ( size_t j = 0; j < m_CompleteControlSurfaceVec.size(); ++j )
            {
                if ( m_CompleteControlSurfaceVec[j].SSID.compare( m_ActiveControlSurfaceVec[i].SSID ) == 0 )
                {
                    m_CompleteControlSurfaceVec[j].isGrouped = false;
                }
            }
        }

        delete m_ControlSurfaceGroupVec[m_CurrentCSGroupIndex];
        m_ControlSurfaceGroupVec.erase( m_ControlSurfaceGroupVec.begin() + m_CurrentCSGroupIndex );

        if ( m_ControlSurfaceGroupVec.size() > 0 )
        {
            m_CurrentCSGroupIndex = 0;
        }
        else
        {
            m_CurrentCSGroupIndex = -1;
        }
    }
    m_SelectedGroupedCS.clear();
    UpdateActiveControlSurfVec();
    UpdateControlSurfaceGroupSuffix();
}

void VSPAEROMgrSingleton::AddSelectedToCSGroup()
{
    vector < int > selected = m_SelectedUngroupedCS;
    if ( m_CurrentCSGroupIndex != -1 )
    {
        vector < VspAeroControlSurf > ungrouped_vec = GetAvailableCSVec();

        for ( size_t i = 0; i < selected.size(); ++i )
        {
            m_ControlSurfaceGroupVec[ m_CurrentCSGroupIndex ]->AddSubSurface( ungrouped_vec[ selected[ i ] - 1 ] );
        }
    }
    m_SelectedUngroupedCS.clear();
    m_SelectedGroupedCS.clear();
    UpdateActiveControlSurfVec();
}

void VSPAEROMgrSingleton::AddAllToCSGroup()
{
    if ( m_CurrentCSGroupIndex != -1 )
    {
        vector < VspAeroControlSurf > ungrouped_vec = GetAvailableCSVec();
        for ( size_t i = 0; i < ungrouped_vec.size(); ++i )
        {
            m_ControlSurfaceGroupVec[ m_CurrentCSGroupIndex ]->AddSubSurface( ungrouped_vec[ i ] );
        }
    }
    m_SelectedUngroupedCS.clear();
    m_SelectedGroupedCS.clear();
    UpdateActiveControlSurfVec();
}

void VSPAEROMgrSingleton::RemoveSelectedFromCSGroup()
{
    vector < int > selected = m_SelectedGroupedCS;
    if ( m_CurrentCSGroupIndex != -1 )
    {
        for ( size_t i = 0; i < selected.size(); ++i )
        {
            m_ControlSurfaceGroupVec[ m_CurrentCSGroupIndex ]->RemoveSubSurface( m_ActiveControlSurfaceVec[selected[i] - 1].SSID,
                    m_ActiveControlSurfaceVec[selected[i] - 1].iReflect );
            for ( size_t j = 0; j < m_CompleteControlSurfaceVec.size(); ++j )
            {
                if ( m_ActiveControlSurfaceVec[selected[i] - 1].SSID.compare( m_CompleteControlSurfaceVec[j].SSID ) == 0 )
                {
                    if ( m_ActiveControlSurfaceVec[selected[i] - 1].iReflect == m_CompleteControlSurfaceVec[j].iReflect )
                    {
                        m_CompleteControlSurfaceVec[ j ].isGrouped = false;
                    }
                }
            }
        }
    }
    m_SelectedGroupedCS.clear();
    UpdateActiveControlSurfVec();
}

void VSPAEROMgrSingleton::RemoveAllFromCSGroup()
{
    if ( m_CurrentCSGroupIndex != -1 )
    {
        for ( size_t i = 0; i < m_ActiveControlSurfaceVec.size(); ++i )
        {
            m_ControlSurfaceGroupVec[ m_CurrentCSGroupIndex ]->RemoveSubSurface( m_ActiveControlSurfaceVec[i].SSID, m_ActiveControlSurfaceVec[i].iReflect );
            for ( size_t j = 0; j < m_CompleteControlSurfaceVec.size(); ++j )
            {
                if ( m_ActiveControlSurfaceVec[i].SSID.compare( m_CompleteControlSurfaceVec[j].SSID ) == 0 )
                {
                    if ( m_ActiveControlSurfaceVec[i].iReflect == m_CompleteControlSurfaceVec[j].iReflect )
                    {
                        m_CompleteControlSurfaceVec[ j ].isGrouped = false;
                    }
                }
            }
        }
    }
    m_SelectedGroupedCS.clear();
    UpdateActiveControlSurfVec();
}

string VSPAEROMgrSingleton::GetCurrentCSGGroupName()
{
    if ( m_CurrentCSGroupIndex != -1 )
    {
        return m_ControlSurfaceGroupVec[ m_CurrentCSGroupIndex ]->GetName();
    }
    else
    {
        return "";
    }
}

vector < VspAeroControlSurf > VSPAEROMgrSingleton::GetAvailableCSVec()
{
    vector < VspAeroControlSurf > ungrouped_cs;

    for ( size_t i = 0; i < m_CompleteControlSurfaceVec.size(); i++ )
    {
        bool grouped = false;

        for ( size_t j = 0; j < m_ActiveControlSurfaceVec.size(); j++ )
        {
            if ( ( strcmp( m_CompleteControlSurfaceVec[i].SSID.c_str(), m_ActiveControlSurfaceVec[j].SSID.c_str() ) == 0 ) && 
                 ( m_CompleteControlSurfaceVec[i].iReflect == m_ActiveControlSurfaceVec[j].iReflect ) &&
                 ( strcmp( m_CompleteControlSurfaceVec[i].fullName.c_str(), m_ActiveControlSurfaceVec[j].fullName.c_str() ) == 0 ) )
            {
                grouped = true;
                break;
            }
        }

        if ( !grouped )
        {
            ungrouped_cs.push_back( m_CompleteControlSurfaceVec[i] );
        }
    }

    return ungrouped_cs;
}

void VSPAEROMgrSingleton::SetCurrentCSGroupName( const string & name )
{
    if ( m_CurrentCSGroupIndex != -1 )
    {
        m_ControlSurfaceGroupVec[ m_CurrentCSGroupIndex ]->SetName( name );
    }
}

void VSPAEROMgrSingleton::HighlightSelected( int type )
{
    Vehicle* veh = VehicleMgr.GetVehicle();

    veh->ClearActiveGeom();

    if ( type == ROTORDISK )
    {
        VSPAEROMgr.SetCurrentType( ROTORDISK );
    }
    else if ( type == CONTROL_SURFACE )
    {
        VSPAEROMgr.SetCurrentType( CONTROL_SURFACE );
    }
    else if ( type == UNSTEADY_GROUP )
    {
        VSPAEROMgr.SetCurrentType( UNSTEADY_GROUP );
    }
    else
    {
        return;
    }
}

void VSPAEROMgrSingleton::LoadDrawObjs( vector < DrawObj* > & draw_obj_vec )
{
    Vehicle* veh = VehicleMgr.GetVehicle();
    if ( !veh )
    {
        return;
    }

    if ( m_LastSelectedType == ROTORDISK )
    {
        UpdateBBox( draw_obj_vec );
    }
    else if ( m_LastSelectedType == CONTROL_SURFACE )
    {
        UpdateHighlighted( draw_obj_vec );
    }
    else if ( m_LastSelectedType == UNSTEADY_GROUP )
    {
        HighlightUnsteadyGroup( draw_obj_vec );
    }

    for ( size_t i = 0; i < m_CpSliceVec.size(); i++ )
    {
        bool highlight = false;
        if ( m_CurrentCpSliceIndex == i )
        {
            highlight = true;
        }

        m_CpSliceVec[i]->LoadDrawObj( draw_obj_vec, i, highlight );
    }
}

void VSPAEROMgrSingleton::UpdateBBox( vector < DrawObj* > & draw_obj_vec )
{
    Vehicle* veh = VehicleMgr.GetVehicle();
    if ( !veh )
    {
        return;
    }

    //==== Load Bounding Box ====//
    m_BBox.Reset();
    BndBox bb;

    // If there is no selected rotor size is zero ( like blank geom )
    // set bbox to zero size
    if ( m_CurrentRotorDiskIndex == -1 )
    {
        m_BBox.Update( vec3d( 0, 0, 0 ) );
    }
    else
    {
        Geom* geom = veh->FindGeom( m_RotorDiskVec[m_CurrentRotorDiskIndex]->GetParentID() );
        if ( geom )
        {
            geom->GetSurfPtr( m_RotorDiskVec[m_CurrentRotorDiskIndex]->GetSurfNum() )->GetBoundingBox( bb );
            m_BBox.Update( bb );
        }
        else
        { m_CurrentRotorDiskIndex = -1; }
    }

    //==== Bounding Box ====//
    m_HighlightDrawObj.m_Screen = DrawObj::VSP_MAIN_SCREEN;
    m_HighlightDrawObj.m_GeomID = BBOXHEADER + m_ID;
    m_HighlightDrawObj.m_LineWidth = 2.0;
    m_HighlightDrawObj.m_LineColor = vec3d( 1.0, 0., 0.0 );
    m_HighlightDrawObj.m_Type = DrawObj::VSP_LINES;

    m_HighlightDrawObj.m_PntVec = m_BBox.GetBBoxDrawLines();
    m_HighlightDrawObj.m_GeomChanged = true;

    draw_obj_vec.push_back( &m_HighlightDrawObj );
}

void VSPAEROMgrSingleton::UpdateHighlighted( vector < DrawObj* > & draw_obj_vec )
{
    Vehicle* veh = VehicleMgr.GetVehicle();
    if ( !veh )
    {
        return;
    }

    string parentID = "";
    string ssid = "";
    int sub_surf_indx;
    if ( m_CurrentCSGroupIndex != -1 )
    {
        vector < VspAeroControlSurf > cont_surf_vec = m_ActiveControlSurfaceVec;
        vector < VspAeroControlSurf > cont_surf_vec_ungrouped = GetAvailableCSVec();
        if ( m_SelectedGroupedCS.size() == 0 && m_SelectedUngroupedCS.size() == 0 )
        {
            for ( size_t i = 0; i < cont_surf_vec.size(); ++i )
            {
                vec3d color( 0, 1, 0 ); // Green
                parentID = cont_surf_vec[i].parentGeomId;
                sub_surf_indx = cont_surf_vec[i].iReflect;
                ssid = cont_surf_vec[i].SSID;
                Geom* geom = veh->FindGeom( parentID );
                if ( geom )
                {
                    SubSurface* subsurf = geom->GetSubSurf( ssid );
                    if ( subsurf )
                    {
                        subsurf->LoadPartialColoredDrawObjs( ssid, sub_surf_indx, draw_obj_vec, color );
                    }
                }
            }
        }
        else
        {
            if ( cont_surf_vec_ungrouped.size() > 0 )
            {
                for ( size_t i = 0; i < m_SelectedUngroupedCS.size(); ++i )
                {
                    vec3d color( 1, 0, 0 ); // Red
                    parentID = cont_surf_vec_ungrouped[m_SelectedUngroupedCS[i] - 1].parentGeomId;
                    sub_surf_indx = cont_surf_vec_ungrouped[m_SelectedUngroupedCS[i] - 1].iReflect;
                    ssid = cont_surf_vec_ungrouped[m_SelectedUngroupedCS[i] - 1].SSID;
                    Geom* geom = veh->FindGeom( parentID );
                    SubSurface* subsurf = geom->GetSubSurf( ssid );
                    if ( subsurf )
                    {
                        subsurf->LoadPartialColoredDrawObjs( ssid, sub_surf_indx, draw_obj_vec, color );
                    }
                }
            }

            if ( cont_surf_vec.size() > 0 )
            {
                for ( size_t i = 0; i < m_SelectedGroupedCS.size(); ++i )
                {
                    vec3d color( 0, 1, 0 ); // Green
                    parentID = cont_surf_vec[m_SelectedGroupedCS[i] - 1].parentGeomId;
                    sub_surf_indx = cont_surf_vec[m_SelectedGroupedCS[i] - 1].iReflect;
                    ssid = cont_surf_vec[m_SelectedGroupedCS[i] - 1].SSID;
                    Geom* geom = veh->FindGeom( parentID );
                    SubSurface* subsurf = geom->GetSubSurf( ssid );
                    if ( subsurf )
                    {
                        subsurf->LoadPartialColoredDrawObjs( ssid, sub_surf_indx, draw_obj_vec, color );
                    }
                }
            }
        }
    }
}

string VSPAEROMgrSingleton::ComputeCpSlices( FILE * logFile )
{
    string resID = string();

    UpdateFilenames();

    if ( !FileExist( m_AdbFile ) )
    {
        fprintf( stderr, "\nError: Aerothermal database (*.adb) file not found. "
            "Execute VSPAERO before running the CpSlicer\n" );
        return resID;
    }

    CreateCutsFile();

    resID = ExecuteCpSlicer( logFile );

    vector < string > resIDvec;
    ReadSliceFile( m_SliceFile, resIDvec );

    // Add Case Result IDs to CpSlice Wrapper Result
    Results* res = ResultsMgr.FindResultsPtr( resID );
    if ( res )
    {
        res->Add( new NameValData( "CpSlice_Case_ID_Vec", resIDvec, "ID's of Cp slice results." ) );
    }

    return resID;
}

string VSPAEROMgrSingleton::ExecuteCpSlicer( FILE * logFile )
{
    Vehicle* veh = VehicleMgr.GetVehicle();
    if ( !veh )
    {
        return string();
    }

    WaitForFile( m_AdbFile );
    if ( !FileExist( m_AdbFile ) )
    {
        fprintf( stderr, "WARNING: Aerothermal database file not found: %s\n\tFile: %s \tLine:%d\n", m_AdbFile.c_str(), __FILE__, __LINE__ );
    }

    WaitForFile( m_CutsFile );
    if ( !FileExist( m_CutsFile ) )
    {
        fprintf( stderr, "WARNING: Cuts file not found: %s\n\tFile: %s \tLine:%d\n", m_CutsFile.c_str(), __FILE__, __LINE__ );
    }

    //====== Send command to be executed by the system at the command prompt ======//
    vector<string> args;

    // Add model file name
    args.push_back( "-slice" );
    args.push_back( m_ModelNameBase );

    //Print out execute command
    string cmdStr = ProcessUtil::PrettyCmd( veh->GetVSPAEROPath(), veh->GetLOADSCmd(), args );
    if( logFile )
    {
        fprintf( logFile, "%s", cmdStr.c_str() );
    }
    else
    {
        MessageData data;
        data.m_String = "VSPAEROSolverMessage";
        data.m_StringVec.push_back( cmdStr );
        MessageMgr::getInstance().Send( "ScreenMgr", nullptr, data );
    }

    //====== Execute VSPAERO Slicer ======//
    m_SlicerThread.ForkCmd( veh->GetVSPAEROPath(), veh->GetLOADSCmd(), args );

    // ==== MonitorSolverProcess ==== //
    MonitorProcess( logFile, &m_SlicerThread, "VSPAEROSolverMessage" );

    // Write out new results
    Results* res = ResultsMgr.CreateResults( "CpSlice_Wrapper", "VSPAERO Cp slicer setup results." );
    if ( !res )
    {
        fprintf( stderr, "ERROR: Unable to create result in result manager \n\tFile: %s \tLine:%d\n", __FILE__, __LINE__ );
        return string();
    }
    else
    {
        int num_slice = m_CpSliceVec.size();
        res->Add( new NameValData( "Num_Cuts", num_slice, "Number of slices." ) );
    }

    return res->GetID();
}

void VSPAEROMgrSingleton::ComputeQuadTreeSlices( FILE * logFile )
{
    UpdateFilenames();

    if ( !FileExist( m_AdbFile ) )
    {
        fprintf( stderr, "\nError: Aerothermal database (*.adb) file not found. "
                         "Execute VSPAERO before running the quad tree slicer\n" );
        return;
    }

    // Setup file needs to be re-written to update desired slices.
    CreateSetupFile();

    ExecuteQuadTreeSlicer( logFile );
}

void VSPAEROMgrSingleton::ExecuteQuadTreeSlicer( FILE * logFile )
{
    Vehicle* veh = VehicleMgr.GetVehicle();
    if ( !veh )
    {
        return;
    }

    WaitForFile( m_AdbFile );
    if ( !FileExist( m_AdbFile ) )
    {
        fprintf( stderr, "WARNING: Aerothermal database file not found: %s\n\tFile: %s \tLine:%d\n", m_AdbFile.c_str(), __FILE__, __LINE__ );
        return;
    }

    //====== Send command to be executed by the system at the command prompt ======//
    vector<string> args;

    // Add model file name
    args.push_back( "-interrogate" );

    if ( m_RotateBladesFlag() ||
       ( m_StabilityType.Get() > vsp::STABILITY_DEFAULT && m_StabilityType.Get() < vsp::STABILITY_PITCH ) )
    {
        args.push_back( "-unsteady" );
    }

    args.push_back( m_ModelNameBase );

    //Print out execute command
    string cmdStr = ProcessUtil::PrettyCmd( veh->GetVSPAEROPath(), veh->GetVSPAEROCmd(), args );
    if( logFile )
    {
        fprintf( logFile, "%s", cmdStr.c_str() );
    }
    else
    {
        MessageData data;
        data.m_String = "VSPAEROSolverMessage";
        data.m_StringVec.push_back( cmdStr );
        MessageMgr::getInstance().Send( "ScreenMgr", nullptr, data );
    }

    //====== Execute VSPAERO Slicer ======//
    m_SolverProcess.ForkCmd( veh->GetVSPAEROPath(), veh->GetVSPAEROCmd(), args );

    // ==== MonitorSolverProcess ==== //
    MonitorProcess( logFile, &m_SolverProcess, "VSPAEROSolverMessage" );
}

void VSPAEROMgrSingleton::ClearCpSliceResults()
{
    // Clear previous results
    while ( ResultsMgr.GetNumResults( "CpSlicer_Case" ) > 0 )
    {
        ResultsMgr.DeleteResult( ResultsMgr.FindResultsID( "CpSlicer_Case", 0 ) );
    }
    while ( ResultsMgr.GetNumResults( "CpSlice_Wrapper" ) > 0 )
    {
        ResultsMgr.DeleteResult( ResultsMgr.FindResultsID( "CpSlice_Wrapper", 0 ) );
    }
}

void VSPAEROMgrSingleton::CreateCutsFile()
{
    Vehicle *veh = VehicleMgr.GetVehicle();
    if ( !veh )
    {
        fprintf( stderr, "ERROR %d: Unable to get vehicle \n\tFile: %s \tLine:%d\n", vsp::VSP_INVALID_PTR, __FILE__, __LINE__ );
        return ;
    }

    // Clear existing cuts file
    if ( FileExist( m_CutsFile ) )
    {
        remove( m_CutsFile.c_str() );
    }

    FILE * cut_file = fopen( m_CutsFile.c_str(), "w" );
    if ( cut_file == nullptr )
    {
        fprintf( stderr, "ERROR %d: Unable to create cuts file: %s\n\tFile: %s \tLine:%d\n", vsp::VSP_INVALID_PTR, m_CutsFile.c_str(), __FILE__, __LINE__ );
        return;
    }

    int numcuts = m_CpSliceVec.size();

    fprintf( cut_file, "%d\n", numcuts );

    for ( size_t i = 0; i < numcuts; i++ )
    {
        fprintf( cut_file, "%c %f\n", 120 + m_CpSliceVec[i]->m_CutType(),
                 m_CpSliceVec[i]->m_CutPosition() );
    }

    //Finish up by closing the file and making sure that it appears in the file system
    fclose( cut_file );

    // Wait until the setup file shows up on the file system
    WaitForFile( m_SetupFile );

}

void VSPAEROMgrSingleton::AddCpSliceVec( int cut_type, const vector< double > &cut_vec )
{
    for ( size_t i = 0; i < cut_vec.size(); i++ )
    {
        CpSlice* slice = AddCpSlice();

        if ( slice )
        {
            slice->m_CutType.Set( cut_type );
            slice->m_CutPosition.Set( cut_vec[i] );
        }
    }
}

vector < double > VSPAEROMgrSingleton::GetCpSlicePosVec( int type )
{
    vector < double > cut_pos_vec;

    for ( size_t i = 0; i < m_CpSliceVec.size(); i++ )
    {
        if ( m_CpSliceVec[i]->m_CutType() == type )
        {
            cut_pos_vec.push_back( m_CpSliceVec[i]->m_CutPosition() );
        }
    }
    return cut_pos_vec;
}

bool VSPAEROMgrSingleton::ValidCpSliceInd( int ind )
{
    if ( (int)m_CpSliceVec.size() > 0 && ind >= 0 && ind < (int)m_CpSliceVec.size() )
    {
        return true;
    }
    else
    {
        return false;
    }
}

void VSPAEROMgrSingleton::DelCpSlice( int ind )
{
    if ( ValidCpSliceInd( ind ) )
    {
        delete m_CpSliceVec[ind];
        m_CpSliceVec.erase( m_CpSliceVec.begin() + ind );
    }
}

CpSlice* VSPAEROMgrSingleton::AddCpSlice( )
{
    CpSlice* slice = nullptr;
    slice = new CpSlice();

    if ( slice )
    {
        slice->SetName( string( "CpSlice_" + to_string( (long long)m_CpSliceVec.size() ) ) );
        slice->SetParentContainer( GetID() );
        AddCpSlice( slice );
    }

    return slice;
}

CpSlice* VSPAEROMgrSingleton::GetCpSlice( int ind )
{
    if ( ValidCpSliceInd( ind ) )
    {
        return m_CpSliceVec[ind];
    }
    return nullptr;
}

int VSPAEROMgrSingleton::GetCpSliceIndex( const string & id )
{
    for ( int i = 0; i < (int)m_CpSliceVec.size(); i++ )
    {
        if ( m_CpSliceVec[i]->GetID() == id && ValidCpSliceInd( i ) )
        {
            return i;
        }
    }
    return -1;
}

void VSPAEROMgrSingleton::ClearCpSliceVec()
{
    for ( size_t i = 0; i < m_CpSliceVec.size(); ++i )
    {
        delete m_CpSliceVec[i];
    }
    m_CpSliceVec.clear();
}

void VSPAEROMgrSingleton::ReadSliceFile( const string &filename, vector <string> &res_id_vector )
{
    FILE *fp = nullptr;
    bool read_success = false;
    WaitForFile( filename );
    fp = fopen( filename.c_str(), "r" );
    if ( fp == nullptr )
    {
        fprintf( stderr, "ERROR %d: Could not open Slice file: %s\n\tFile: %s \tLine:%d\n", vsp::VSP_FILE_DOES_NOT_EXIST, m_SliceFile.c_str(), __FILE__, __LINE__ );
        return;
    }

    Results* res = nullptr;
    std::vector<string> data_string_array;
    int num_table_columns = 4;

    // Read in all of the data into the results manager
    char seps[] = " :,_\t\n";
    bool skip = false;

    while ( !feof( fp ) )
    {
        if ( !skip )
        {
            data_string_array = ReadDelimLine( fp, seps ); //this is also done in some of the embedded loops below
        }
        skip = false;

        if ( data_string_array.size() > 0 )
        {
            if ( strcmp( data_string_array[0].c_str(), "BLOCK" ) == 0 )
            {
                res = ResultsMgr.CreateResults( "CpSlicer_Case", "VSPAERO Cp slicer results." );
                res_id_vector.push_back( res->GetID() );

                res->Add( new NameValData( "Cut_Type", (int)( data_string_array[4][0] - 88 ), "Cut type (X,Y,Z)." ) ); // ASCII X: 88; Y: 89; Z: 90
                res->Add( new NameValData( "Cut_Loc", std::stod( data_string_array[5] ), "Cut location." ) );
                res->Add( new NameValData( "Cut_Num", std::stoi( data_string_array[2] ), "Cut number." ) );
            }
            else if ( res && strcmp( data_string_array[0].c_str(), "Case" ) == 0 )
            {
                res->Add( new NameValData( "Case", std::stoi( data_string_array[1] ), "Case number." ) );
                res->Add( new NameValData( "Mach", std::stod( data_string_array[4] ), "Mach number." ) );
                res->Add( new NameValData( "Alpha", std::stod( data_string_array[7] ), "Angle of attack." ) );
                res->Add( new NameValData( "Beta", std::stod( data_string_array[10] ), "Angle of sideslip." ) );
            }
            //READ slc table
            /* Example slc table
            BLOCK Cut_1_at_X:_2.000000
            Case: 1 ... Mach: 0.001000 ... Alpha: 1.000000 ... Beta: 0.000000 ...     Case: 1 ...
            x          y          z         dCp/Cp
            2.0000     0.0000    -0.6063    -0.0000
            2.0000     0.0000    -0.5610    -0.0000
            2.0000     0.0000    -0.4286    -0.0000
            2.0000     0.0000    -0.1093    -0.0000
            */
            else if ( res && data_string_array.size() == num_table_columns && strcmp( data_string_array[0].c_str(), "x" ) != 0 )
            {
                // create new vectors for this set of results information
                vector < double > x_data_vec, y_data_vec, z_data_vec, Cp_data_vec;

                while ( data_string_array.size() == num_table_columns )
                {
                    x_data_vec.push_back( std::stod( data_string_array[0] ) );
                    y_data_vec.push_back( std::stod( data_string_array[1] ) );
                    z_data_vec.push_back( std::stod( data_string_array[2] ) );
                    Cp_data_vec.push_back( std::stod( data_string_array[3] ) );

                    data_string_array = ReadDelimLine( fp, seps );
                }

                skip = true;

                //Add to the results manager
                res->Add( new NameValData( "X_Loc", x_data_vec, "Slice data X vector." ) );
                res->Add( new NameValData( "Y_Loc", y_data_vec, "Slice data Y vector." ) );
                res->Add( new NameValData( "Z_Loc", z_data_vec, "Slice data Z vector." ) );

                res->Add( new NameValData( "Cp", Cp_data_vec, "Slice Cp or delta Cp." ) );
            } // end of cut data
        }
    }

    std::fclose( fp );

    return;
}

bool VSPAEROMgrSingleton::ValidUnsteadyGroupInd( int index )
{
    if ( (int)m_UnsteadyGroupVec.size() > 0 && index >= 0 && index < (int)m_UnsteadyGroupVec.size() )
    {
        return true;
    }
    else
    {
        return false;
    }
}

void VSPAEROMgrSingleton::DeleteUnsteadyGroup( const vector <int> &ind_vec )
{
    vector < UnsteadyGroup* > tempvec;

    for ( int i = 0; i < m_UnsteadyGroupVec.size(); i++ )
    {
        if ( vector_contains_val( ind_vec, i ) )
        {
            delete m_UnsteadyGroupVec[i];
        }
        else
        {
            tempvec.push_back( m_UnsteadyGroupVec[i] );
        }
    }
    m_UnsteadyGroupVec = tempvec;
}

UnsteadyGroup* VSPAEROMgrSingleton::AddUnsteadyGroup()
{
    UnsteadyGroup* group = new UnsteadyGroup();

    if ( group )
    {
        group->SetName( string( "UnsteadyGroup_" + to_string( (long long)m_UnsteadyGroupVec.size() ) ) );
        group->SetParentContainer( GetID() );
        AddUnsteadyGroup( group );
        m_CurrentUnsteadyGroupIndex = m_UnsteadyGroupVec.size() - 1;
    }

    return group;
}

UnsteadyGroup* VSPAEROMgrSingleton::GetUnsteadyGroup( int index )
{
    if ( ValidUnsteadyGroupInd( index ) )
    {
        return m_UnsteadyGroupVec[index];
    }
    return nullptr;
}

int VSPAEROMgrSingleton::GetUnsteadyGroupIndex( const string& id )
{
    for ( int i = 0; i < (int)m_UnsteadyGroupVec.size(); i++ )
    {
        if ( m_UnsteadyGroupVec[i]->GetID() == id && ValidUnsteadyGroupInd( i ) )
        {
            return i;
        }
    }
    return -1;
}

string VSPAEROMgrSingleton::GetUnsteadyGroupID( int index )
{
    if ( ValidUnsteadyGroupInd( index ) )
    {
        return m_UnsteadyGroupVec[index]->GetID();
    }
    return string();
}

void VSPAEROMgrSingleton::ClearUnsteadyGroupVec()
{
    for ( size_t i = 0; i < m_UnsteadyGroupVec.size(); ++i )
    {
        delete m_UnsteadyGroupVec[i];
    }
    m_UnsteadyGroupVec.clear();
}

int VSPAEROMgrSingleton::NumUnsteadyRotorGroups()
{
    int num_rot = 0;

    for ( size_t i = 0; i < m_UnsteadyGroupVec.size(); i++ )
    {
        if ( m_UnsteadyGroupVec[i]->m_GeomPropertyType.Get() == UnsteadyGroup::GEOM_ROTOR )
        {
            num_rot++;
        }
    }

    return num_rot;
}

//==== Set Index For Active Unsteady Group ====//
void VSPAEROMgrSingleton::SetCurrentUnsteadyGroupIndex( int index )
{
    if ( ValidUnsteadyGroupInd( index ) )
    {
        m_CurrentUnsteadyGroupIndex = index;
    }
}

void VSPAEROMgrSingleton::SetCurrentUnsteadyGroupIndex( const string& id )
{
    int index = GetUnsteadyGroupIndex( id );
    if ( index >= 0 ) // Valid if not -1
    {
        m_CurrentUnsteadyGroupIndex = index;
    }
}

map < pair < string, int >, vector < int > > VSPAEROMgrSingleton::GetVSPAEROGeomIndexMap( int set_index, int thin_set )
{
    map < pair < string, int >, vector < int > > geom_index_map;

    Vehicle* veh = VehicleMgr.GetVehicle();
    if ( !veh )
    {
        return geom_index_map;
    }

    vector < pair < string, int > > surface_geoms;
    int surf_cnt = 1;

    // Get both sets.
    vector < string > all_geom_vec = veh->GetGeomSet( set_index );
    vector <string> thingeomvec = veh->GetGeomSet( thin_set );
    // Append them.
    all_geom_vec.insert( all_geom_vec.end(), thingeomvec.begin(), thingeomvec.end() );

    vector < int > degen_type_vec = veh->GetDegenGeomTypeVec( set_index );
    vector < int > thin_degen_type_vec = veh->GetDegenGeomTypeVec( thin_set );
    // Append them.
    degen_type_vec.insert( degen_type_vec.end(), thin_degen_type_vec.begin(), thin_degen_type_vec.end() );

    for ( size_t i = 0; i < all_geom_vec.size(); i++ )
    {
        Geom* geom = veh->FindGeom( all_geom_vec[i] );
        if ( !geom )
        {
            continue;
        }

        if ( geom->GetType().m_Type == BLANK_GEOM_TYPE ||
             geom->GetType().m_Type == HINGE_GEOM_TYPE ||
             geom->GetType().m_Type == PT_CLOUD_GEOM_TYPE ) // Skip these types.
        {
            continue;
        }

        if ( geom->GetType().m_Type == MESH_GEOM_TYPE && ( i == all_geom_vec.size() - 1 ) )
        {
            // Do not include the panel method mesh of the entire VSPAERO set 
            // This is done to prevent the the computed geometry form being included
            // as VSPAERO components
            MeshGeom* mesh = dynamic_cast<MeshGeom*>( geom );
            assert( mesh );

            if ( mesh->GetTMeshPtrIDs().size() == ( all_geom_vec.size() - 1 ) )
            {
                continue;
            }
        }

        int num_sym = geom->GetNumSymmCopies();

        for ( size_t s = 1; s <= num_sym; s++ )
        {
            // Human and Mesh types will run in VSPAERO panel method... support accordingly
            size_t num_surf = 0;
            if ( geom->GetType().m_Type == HUMAN_GEOM_TYPE )
            {
                num_surf = 1;
            }
            else if ( geom->GetType().m_Type == MESH_GEOM_TYPE )
            {
                MeshGeom* mesh = dynamic_cast<MeshGeom*>( geom );
                assert( mesh );

                num_surf = mesh->GetNumIndexedParts();
            }
            else
            {
                num_surf = geom->GetNumMainSurfs();
            }

            for ( size_t j = 0; j < num_surf; j++ )
            {
                geom_index_map[std::make_pair( all_geom_vec[i], s )].push_back( surf_cnt );
                surf_cnt++;
            }

        }

    }

    // Note: DISK_TYPE and MESH_TYPE are ignored since they are not supported as unsteady components

    for ( size_t i = 0; i < surface_geoms.size(); i++ )
    {
        geom_index_map[surface_geoms[i]].push_back( i + 1 );
    }

    return geom_index_map;
}

void VSPAEROMgrSingleton::UpdateAutoTimeStep()
{
    if ( NumUnsteadyRotorGroups() == 0 )
    {
        return;
    }

    double max_rpm = -1.e9;
    double min_rpm = 1.e9;
    int num_dt = m_AutoTimeNumRevs();
    double dt = 0;

    for ( size_t c = 0; c < NumUnsteadyGroups(); c++ )
    {
        if ( m_UnsteadyGroupVec[c]->m_GeomPropertyType() == UnsteadyGroup::GEOM_ROTOR )
        {
            if ( abs( m_UnsteadyGroupVec[c]->m_RPM() ) < min_rpm )
            {
                min_rpm = abs( m_UnsteadyGroupVec[c]->m_RPM() );
            }

            if ( abs( m_UnsteadyGroupVec[c]->m_RPM() ) > max_rpm )
            {
                max_rpm = abs( m_UnsteadyGroupVec[c]->m_RPM() );
            }
        }
    }

    if ( max_rpm > 0. )
    {
        dt = 2.5 / max_rpm;
    }

   // Calculate the number of time steps if user did not set it
    // Slowest rotor, at least 1 rotation
    double Period = 60 / max_rpm;

    double NumSteps_1 = 2. * Period / dt + 1;

    // Fastest rotor does ABS(NumberOfTimeSteps_) revolutions
    double NumSteps_2 = std::abs( num_dt ) * 20;

    if ( NumSteps_1 > NumSteps_2 )
    {
        num_dt = NumSteps_1;
    }
    else
    {
        num_dt = NumSteps_2;
    }

    if ( m_AutoTimeStepFlag.Get() )
    {
        m_TimeStepSize.Set( dt );
        m_NumTimeSteps.Set( num_dt );
    }
    else
    {
        m_AutoTimeNumRevs.Set( int( ( m_NumTimeSteps() / 20.0 ) * m_TimeStepSize() / dt ) );
    }
}

void VSPAEROMgrSingleton::UpdateParmRestrictions()
{
    if ( !m_ManualVrefFlag() )
    {
        m_Vref.Set( m_Vinf() );
    }

    if ( m_RotorDiskVec.size() == 0 )
    {
        m_ActuatorDiskFlag.Set( false );
    }

    if ( NumUnsteadyRotorGroups() == 0 )
    {
        m_RotateBladesFlag.Set( false );
    }

    if ( m_RotateBladesFlag() || m_GroundEffectToggle() )
    {
        // Only 1 flow condition supported for unsteady analysis and ground effect calculations
        m_AlphaNpts.Set( 1 );
        m_BetaNpts.Set( 1 );
        m_MachNpts.Set( 1 );
        m_ReCrefNpts.Set( 1 );
        m_StabilityType.Set( vsp::STABILITY_OFF );
    }
}

// TODO:  Not sure how this works.  It references m_GeomSet, but not yet m_ThinGeomSet.
// it is also the last place that accesses m_AnalysisMethod() - if you include the GetVSPAEROGeomIndexMap()
// that is only called from here.
void VSPAEROMgrSingleton::UpdateUnsteadyGroups()
{
    int set = m_GeomSet();
    int degenset = m_ThinGeomSet();

    if ( m_UseMode() )
    {
        Mode *m = ModeMgr.GetMode( m_ModeID );
        if ( m )
        {
            m->ApplySettings();
            set = m->m_NormalSet();
            degenset = m->m_DegenSet();
        }
    }

    Vehicle* veh = VehicleMgr.GetVehicle();
    if ( !veh )
    {
        return;
    }

    vector < int > del_vec;

    map < pair < string, int >, vector < int > > vspaero_geom_index_map = GetVSPAEROGeomIndexMap( set, degenset );

    // For the current VSPAERO set, identify if there are any props or fixed components 
    // that are not placed in an unsteady group

    // Note, this function will delete unsteady groups if the prop is no longer in the set or
    // changed to disk mode, causing the parms to be lost. This can be improved, but is the

    // same behavior for rotor disks.

    // Get both sets.
    vector < string > geom_set_vec = veh->GetGeomSet( set );
    vector <string> thingeomvec = veh->GetGeomSet( degenset );

    // Append them.
    geom_set_vec.insert( geom_set_vec.end(), thingeomvec.begin(), thingeomvec.end() );

    vector < pair < string, int > > ungrouped_props, ungrouped_comps; // ungrouped Geom ID and symmetric surf index

    for ( size_t i = 0; i < geom_set_vec.size(); ++i )
    {
        Geom* geom = veh->FindGeom( geom_set_vec[i] );
        if ( !geom )
        {
            continue;
        }

        int num_sym = geom->GetNumSymmCopies();

        for ( size_t s = 1; s <= num_sym; s++ )
        {
            bool grouped = false;

            // Check if this geom and symmetric copy is in an unsteady group
            for ( size_t j = 0; j < m_UnsteadyGroupVec.size(); j++ )
            {
                vector < pair < string, int > > comp_id_surf_ind_vec = m_UnsteadyGroupVec[j]->GetCompSurfPairVec();

                for ( size_t k = 0; k < comp_id_surf_ind_vec.size(); k++ )
                {
                    if ( ( strcmp( geom_set_vec[i].c_str(), comp_id_surf_ind_vec[k].first.c_str() ) == 0 ) && ( comp_id_surf_ind_vec[k].second == s ) )
                    {
                        grouped = true;
                        break;
                    }
                }
            }

            if ( !grouped )
            {
                if ( geom->GetType().m_Type == PROP_GEOM_TYPE )
                {
                    PropGeom* prop = dynamic_cast<PropGeom*>( geom );
                    assert( prop );
                    if ( prop->m_PropMode() != vsp::PROP_DISK )
                    {
                        ungrouped_props.push_back( std::make_pair( geom_set_vec[i], s ) );
                    }
                }
                else if ( vspaero_geom_index_map[std::make_pair( geom_set_vec[i], s )].size() > 0 && geom->GetType().m_Type != BLANK_GEOM_TYPE &&
                          geom->GetType().m_Type != PT_CLOUD_GEOM_TYPE && geom->GetType().m_Type != HINGE_GEOM_TYPE ) // TODO: Check if point cloud works in panel method?
                {
                    // Allow mesh and human types for panel method
                    if ( m_AnalysisMethod() == vsp::PANEL || ( m_AnalysisMethod() == vsp::VORTEX_LATTICE && geom->GetType().m_Type != HUMAN_GEOM_TYPE && geom->GetType().m_Type != MESH_GEOM_TYPE ) )
                    {
                        ungrouped_comps.push_back( std::make_pair( geom_set_vec[i], s ) );
                    }
                }
            }
        }
    }

    for ( size_t i = 0; i < m_UnsteadyGroupVec.size(); ++i )
    {
        vector < pair < string, int > > new_comp_pair_vec;
        vector < pair < string, int > > comp_id_surf_ind_vec = m_UnsteadyGroupVec[i]->GetCompSurfPairVec();
        vector < int > vspaero_index_vec;

        for ( size_t j = 0; j < comp_id_surf_ind_vec.size(); j++ )
        {
            // Check if Geom still exists and is in the current set
            Geom* parent = veh->FindGeom( comp_id_surf_ind_vec[j].first );
            if ( parent && parent->GetSetFlag( set ) )
            {
                if ( parent->GetType().m_Type == PROP_GEOM_TYPE )
                {
                    PropGeom* prop = dynamic_cast<PropGeom*>( parent );
                    assert( prop );
                    if ( prop->m_PropMode() == vsp::PROP_DISK )
                    {
                        break;
                    }
                }
                else if ( parent->GetType().m_Type == BLANK_GEOM_TYPE || parent->GetType().m_Type == HINGE_GEOM_TYPE || parent->GetType().m_Type == PT_CLOUD_GEOM_TYPE )
                {
                    continue; // TODO: Check if point cloud works in panel method?
                }
                else if ( m_AnalysisMethod() == vsp::VORTEX_LATTICE && ( parent->GetType().m_Type == HUMAN_GEOM_TYPE || parent->GetType().m_Type == MESH_GEOM_TYPE ) )
                {
                    continue; // Allow human and mesh types for panel method
                }

                if ( vspaero_geom_index_map[comp_id_surf_ind_vec[j]].size() > 0 )
                {
                    new_comp_pair_vec.push_back( std::make_pair( comp_id_surf_ind_vec[j].first, comp_id_surf_ind_vec[j].second ) );
                    vspaero_index_vec.insert( vspaero_index_vec.end(), vspaero_geom_index_map[comp_id_surf_ind_vec[j]].begin(), vspaero_geom_index_map[comp_id_surf_ind_vec[j]].end() );
                }
            }
        }

        if ( m_UnsteadyGroupVec[i]->m_GeomPropertyType() != UnsteadyGroup::GEOM_ROTOR )
        {
            for ( size_t j = 0; j < ungrouped_comps.size(); j++ )
            {
                new_comp_pair_vec.push_back( std::make_pair( ungrouped_comps[j].first, ungrouped_comps[j].second ) );
                vspaero_index_vec.insert( vspaero_index_vec.end(), vspaero_geom_index_map[ungrouped_comps[j]].begin(), vspaero_geom_index_map[ungrouped_comps[j]].end() );
            }
            ungrouped_comps.clear();
        }

        if ( new_comp_pair_vec.size() == 0 )
        {
            del_vec.push_back( i );
        }

        m_UnsteadyGroupVec[i]->SetCompSurfPairVec( new_comp_pair_vec );
        m_UnsteadyGroupVec[i]->SetVSPAEROIndexVec( vspaero_index_vec );
        m_UnsteadyGroupVec[i]->Update();
    }


    DeleteUnsteadyGroup( del_vec );

    // Create a fixed component group with all ungrouped components
    if ( ungrouped_comps.size() > 0 )
    {
        UnsteadyGroup* group = AddUnsteadyGroup();

        group->SetCompSurfPairVec( ungrouped_comps );

        vector < int > vspaero_index_vec;
        for ( size_t j = 0; j < ungrouped_comps.size(); j++ )
        {
            vspaero_index_vec.insert( vspaero_index_vec.end(), vspaero_geom_index_map[ungrouped_comps[j]].begin(), vspaero_geom_index_map[ungrouped_comps[j]].end() );
        }

        group->SetVSPAEROIndexVec( vspaero_index_vec );
        group->m_GeomPropertyType.Set( group->GEOM_FIXED );
        group->SetName( "FixedGroup" );
        group->Update();
    }

    // Create a prop component group for each ungrouped prop
    for ( size_t i = 0; i < ungrouped_props.size(); i++ )
    {
        UnsteadyGroup* group = AddUnsteadyGroup();

        group->SetVSPAEROIndexVec( vspaero_geom_index_map[ungrouped_props[i]] );
        group->m_GeomPropertyType.Set( group->GEOM_ROTOR );
        group->AddComp( ungrouped_props[i].first, ungrouped_props[i].second );
        group->Update();
    }

    for ( size_t i = 0; i < m_UnsteadyGroupVec.size(); ++i )
    {
        vector < pair < string, int > > cspv = m_UnsteadyGroupVec[i]->GetCompSurfPairVec();
        // A vector of <geomid, s> pairs -- where s is the symmetry copy number.
        // We want to find the unique Geom's from this.

        vector < string > gidv( cspv.size() );
        for ( size_t j = 0; j < cspv.size(); j++ )
        {
            // Find index (position in vector) corresponding to GeomID.
            gidv[j] = cspv[j].first;
        }

        // Make vector unique.
        std::vector<string>::iterator it;
        it = std::unique( gidv.begin(), gidv.end() );
        gidv.resize( std::distance( gidv.begin(), it ) );

        m_UnsteadyGroupVec[i]->SetGeomIDsInGroup( gidv );
    }

    // Make sure the fixed compomnet group is always first
    if ( m_UnsteadyGroupVec.size() > NumUnsteadyRotorGroups() && m_UnsteadyGroupVec[0]->m_GeomPropertyType() != UnsteadyGroup::GEOM_FIXED )
    {
        UnsteadyGroup* fixed_group = nullptr;
        int fixed_group_index = -1;
        for ( size_t i = 0; i < m_UnsteadyGroupVec.size(); ++i )
        {
            if ( m_UnsteadyGroupVec[i]->m_GeomPropertyType() == UnsteadyGroup::GEOM_FIXED )
            {
                fixed_group = m_UnsteadyGroupVec[i];
                fixed_group_index = i;
                break;
            }
        }

        if ( fixed_group && fixed_group_index >= 0 )
        {
            m_UnsteadyGroupVec.erase( m_UnsteadyGroupVec.begin() + fixed_group_index );
            m_UnsteadyGroupVec.insert( m_UnsteadyGroupVec.begin(), fixed_group );
        }
    }

    UpdateAutoTimeStep();

    if ( m_UniformPropRPMFlag() )
    {
        UnsteadyGroup* current_group = GetUnsteadyGroup( m_CurrentUnsteadyGroupIndex );
        if ( current_group && current_group->m_GeomPropertyType() == UnsteadyGroup::GEOM_ROTOR )
        {
            for ( size_t i = 0; i < m_UnsteadyGroupVec.size(); ++i )
            {
                if ( m_UnsteadyGroupVec[i]->m_GeomPropertyType() == UnsteadyGroup::GEOM_ROTOR )
                {
                    m_UnsteadyGroupVec[i]->m_RPM.Set( current_group->m_RPM.Get() );
                }
            }
        }
    }
}

void VSPAEROMgrSingleton::HighlightUnsteadyGroup( vector < DrawObj* >& draw_obj_vec )
{
    Vehicle* veh = VehicleMgr.GetVehicle();
    if ( !veh )
    {
        return;
    }

    //==== Load Bounding Box ====//
    m_BBox.Reset();
    BndBox bb;

    // If there is no selected rotor size is zero ( like blank geom )
    // set bbox to zero size
    if ( m_CurrentUnsteadyGroupIndex < 0 || m_CurrentUnsteadyGroupIndex > m_UnsteadyGroupVec.size() - 1 )
    {
        m_BBox.Update( vec3d( 0, 0, 0 ) );
    }
    else
    {
        UnsteadyGroup* select_group = m_UnsteadyGroupVec[m_CurrentUnsteadyGroupIndex];

        if ( select_group )
        {
            vector < pair < string, int > > comp_vec = select_group->GetCompSurfPairVec();

            for ( size_t i = 0; i < comp_vec.size(); i++ )
            {
                Geom* geom = veh->FindGeom( comp_vec[i].first );

                if ( !geom )
                {
                    continue;
                }

                int num_main_surf = geom->GetNumMainSurfs();

                for ( size_t j = 0; j < num_main_surf; j++ )
                {
                    geom->GetSurfPtr(j + num_main_surf * ( comp_vec[i].second - 1 ))->GetBoundingBox( bb );
                    m_BBox.Update( bb );
                }
            }
        }
        else
        {
            m_CurrentRotorDiskIndex = -1;
        }
    }

    //==== Bounding Box ====//
    m_HighlightDrawObj.m_Screen = DrawObj::VSP_MAIN_SCREEN;
    m_HighlightDrawObj.m_GeomID = BBOXHEADER + m_ID;
    m_HighlightDrawObj.m_LineWidth = 2.0;
    m_HighlightDrawObj.m_LineColor = vec3d( 1.0, 0., 0.0 );
    m_HighlightDrawObj.m_Type = DrawObj::VSP_LINES;

    m_HighlightDrawObj.m_PntVec = m_BBox.GetBBoxDrawLines();
    m_HighlightDrawObj.m_GeomChanged = true;

    draw_obj_vec.push_back( &m_HighlightDrawObj );
}

int VSPAEROMgrSingleton::CreateGroupsFile()
{
    Vehicle* veh = VehicleMgr.GetVehicle();
    if ( !veh )
    {
        fprintf( stderr, "ERROR %d: Unable to get vehicle \n\tFile: %s \tLine:%d\n", vsp::VSP_INVALID_PTR, __FILE__, __LINE__ );
        return vsp::VSP_INVALID_PTR;
    }

    // Clear existing group file
    if ( FileExist( m_GroupsFile ) )
    {
        remove( m_GroupsFile.c_str() );
    }

    FILE* group_file = fopen( m_GroupsFile.c_str(), "w" );
    if ( !group_file )
    {
        fprintf( stderr, "ERROR %d: Unable to create groups file: %s\n\tFile: %s \tLine:%d\n", vsp::VSP_INVALID_PTR, m_GroupsFile.c_str(), __FILE__, __LINE__ );
        return vsp::VSP_FILE_WRITE_FAILURE;
    }

    int numgroups = m_UnsteadyGroupVec.size();

    fprintf( group_file, "%d\n", numgroups );

    for ( size_t i = 0; i < numgroups; i++ )
    {
        m_UnsteadyGroupVec[ i ]->WriteGroup( group_file );
    }

    //Finish up by closing the file and making sure that it appears in the file system
    fclose( group_file );

    // Wait until the setup file shows up on the file system
    return WaitForFile( m_GroupsFile );
}

string VSPAEROMgrSingleton::ExecuteNoiseAnalysis( FILE* logFile, int noise_type, int noise_unit )
{
    Vehicle* veh = VehicleMgr.GetVehicle();
    if ( !veh )
    {
        return string();
    }

    WaitForFile( m_AdbFile );
    if ( !FileExist( m_AdbFile ) )
    {
        fprintf( stderr, "WARNING: Aerothermal database file not found: %s\n\tFile: %s \tLine:%d\n", m_AdbFile.c_str(), __FILE__, __LINE__ );
        return string();
    }

    for ( size_t i = 0; i < m_GroupResFiles.size(); i++ )
    {
        WaitForFile( m_GroupResFiles[i] );
        if ( !FileExist( m_GroupResFiles[i] ) )
        {
            fprintf( stderr, "WARNING: Group result file not found: %s\n\tFile: %s \tLine:%d\n", m_GroupResFiles[i].c_str(), __FILE__, __LINE__ );
            return string();
        }
    }

    for ( size_t i = 0; i < m_RotorResFiles.size(); i++ )
    {
        WaitForFile( m_RotorResFiles[i] );
        if ( !FileExist( m_RotorResFiles[i] ) )
        {
            fprintf( stderr, "WARNING: Rotor result file not found: %s\n\tFile: %s \tLine:%d\n", m_RotorResFiles[i].c_str(), __FILE__, __LINE__ );
            return string();
        }
    }

    //====== Send command to be executed by the system at the command prompt ======//
    vector<string> args;

    args.push_back( "-noise" );

    if ( noise_type == vsp::NOISE_FLYBY )
    {
        args.push_back( "-flyby" );
    }
    else if ( noise_type == vsp::NOISE_FOOTPRINT )
    {
        args.push_back( "-footprint" );
    }
    else if ( noise_type == vsp::NOISE_STEADY )
    {
        args.push_back( "-steady" );
    }

    if ( noise_unit == vsp::NOISE_ENGLISH )
    {
        args.push_back( "-english" );
    }

    // Add model file name
    args.push_back( m_ModelNameBase );

    //Print out execute command
    string cmdStr = ProcessUtil::PrettyCmd( veh->GetVSPAEROPath(), veh->GetVSPAEROCmd(), args );
    if ( logFile )
    {
        fprintf( logFile, "%s", cmdStr.c_str() );
    }
    else
    {
        MessageData data;
        data.m_String = "VSPAEROSolverMessage";
        data.m_StringVec.push_back( cmdStr );
        MessageMgr::getInstance().Send( "ScreenMgr", nullptr, data );
    }

    // Execute VSPAero
    m_SolverProcess.ForkCmd( veh->GetVSPAEROPath(), veh->GetVSPAEROCmd(), args );

    // ==== MonitorSolverProcess ==== //
    MonitorProcess( logFile, &m_SolverProcess, "VSPAEROSolverMessage" );

    // Check if the kill solver flag has been raised, if so clean up and return
    //  note: we could have exited the IsRunning loop if the process was killed
    if ( m_SolverProcessKill )
    {
        m_SolverProcessKill = false;    //reset kill flag

        return string();    //return empty result ID vector
    }

    return string(); // TODO: Read noise results
}

/*******************************************************
Read *.group.* file output from VSPAERO
See: VSP_Solver.C in vspaero project
*******************************************************/
void VSPAEROMgrSingleton::ReadGroupResFile( const string &filename, vector <string> &res_id_vector, const string &group_name )
{
    FILE *fp = nullptr;
    bool read_success = false;
    WaitForFile( filename );
    fp = fopen( filename.c_str(), "r" );
    if ( fp == nullptr )
    {
        fprintf( stderr, "ERROR %d: Could not open Group file: %s\n\tFile: %s \tLine:%d\n", vsp::VSP_FILE_DOES_NOT_EXIST, filename.c_str(), __FILE__, __LINE__ );
        return;
    }

    Results* res = nullptr;

    // Get group index
    size_t last_index = filename.find_last_not_of( "0123456789" );
    int group_num = stoi( filename.substr( last_index + 1 ) );

    // Time        Cx         Cy         Cz        Cxo        Cyo        Czo        Cxi        Cyi        Czi        Cmx        Cmy        Cmz        Cmxo       Cmyo       Cmzo       Cmxi       Cmyi       Cmzi        CL         CD         CS        CLo        CDo        CSo        CLi        CDi        CSi
    std::vector<string> data_string_array;
    int num_data_col = 28;

    // Read in all of the data into the results manager
    char seps[] = " :,\t\n";
    while ( !feof( fp ) )
    {
        data_string_array = ReadDelimLine( fp, seps ); //this is also done in some of the embedded loops below
        if ( data_string_array.size() == 0 )
        {
            continue;
        }

        if ( strcmp( data_string_array[0].c_str(), "Time" ) == 0 )
        {
            res = ResultsMgr.CreateResults( "VSPAERO_Group", "VSPAERO Group file results." );

            res->Add( new NameValData( "Group_Num", group_num, "Unsteady group number." ) );
            res->Add( new NameValData( "Group_Name", group_name, "Unsteady group name." ) );

            res_id_vector.push_back( res->GetID() );
        }

        // discard the header row and read the next line assuming that it is numeric
        data_string_array = ReadDelimLine( fp, seps );

        if ( res && data_string_array.size() == num_data_col )
        {
            // Raw data vectors
            std::vector < double > Time, Cx, Cy, Cz, Cxo, Cyo, Czo, Cxi, Cyi, Czi;
            std::vector < double > Cmx, Cmy, Cmz, Cmxo, Cmyo, Cmzo, Cmxi, Cmyi, Cmzi;
            std::vector < double > CL, CD, CLo, CDo, CLi, CDi, CS, CSo, CSi;

            // read the data rows
            while ( data_string_array.size() == num_data_col ) //&& data_string_array[0].find( "Comp" ) == std::string::npos
            {
                // Store the raw data
                Time.push_back( std::stod( data_string_array[0] ) );
                Cx.push_back( std::stod( data_string_array[1] ) );
                Cy.push_back( std::stod( data_string_array[2] ) );
                Cz.push_back( std::stod( data_string_array[3] ) );
                Cxo.push_back( std::stod( data_string_array[4] ) );
                Cyo.push_back( std::stod( data_string_array[5] ) );
                Czo.push_back( std::stod( data_string_array[6] ) );
                Cxi.push_back( std::stod( data_string_array[7] ) );
                Cyi.push_back( std::stod( data_string_array[8] ) );
                Czi.push_back( std::stod( data_string_array[9] ) );
                Cmx.push_back( std::stod( data_string_array[10] ) );
                Cmy.push_back( std::stod( data_string_array[11] ) );
                Cmz.push_back( std::stod( data_string_array[12] ) );
                Cmxo.push_back( std::stod( data_string_array[13] ) );
                Cmyo.push_back( std::stod( data_string_array[14] ) );
                Cmzo.push_back( std::stod( data_string_array[15] ) );
                Cmxi.push_back( std::stod( data_string_array[16] ) );
                Cmyi.push_back( std::stod( data_string_array[17] ) );
                Cmzi.push_back( std::stod( data_string_array[18] ) );
                CL.push_back( std::stod( data_string_array[19] ) );
                CD.push_back( std::stod( data_string_array[20] ) );
                CS.push_back( std::stod( data_string_array[21] ) );
                CLo.push_back( std::stod( data_string_array[22] ) );
                CDo.push_back( std::stod( data_string_array[23] ) );
                CSo.push_back( std::stod( data_string_array[24] ) );
                CLi.push_back( std::stod( data_string_array[25] ) );
                CDi.push_back( std::stod( data_string_array[26] ) );
                CSi.push_back( std::stod( data_string_array[27] ) );

                // Read the next line and loop
                data_string_array = ReadDelimLine( fp, seps );
            }

            // Finish up by adding the data to the result res
            res->Add( new NameValData( "Time", Time, "Time in unsteady analysis." ) );
            res->Add( new NameValData( "Cx", Cx, "X force coefficient." ) );
            res->Add( new NameValData( "Cy", Cy, "Y force coefficient." ) );
            res->Add( new NameValData( "Cz", Cz, "Z force coefficient." ) );
            res->Add( new NameValData( "Cxo", Cxo, "X viscous force coefficient." ) );
            res->Add( new NameValData( "Cyo", Cyo, "Y viscous force coefficient." ) );
            res->Add( new NameValData( "Czo", Czo, "Z viscous force coefficient." ) );
            res->Add( new NameValData( "Cxi", Cxi, "X inviscid force coefficient." ) );
            res->Add( new NameValData( "Cyi", Cyi, "Y inviscid force coefficient." ) );
            res->Add( new NameValData( "Czi", Czi, "Z inviscid force coefficient." ) );
            res->Add( new NameValData( "Cmx", Cmx, "X moment coefficient." ) );
            res->Add( new NameValData( "Cmy", Cmy, "Y moment coefficient." ) );
            res->Add( new NameValData( "Cmz", Cmz, "Z moment coefficient." ) );
            res->Add( new NameValData( "Cmxo", Cmxo, "X viscous moment coefficient." ) );
            res->Add( new NameValData( "Cmyo", Cmyo, "Y viscous moment coefficient." ) );
            res->Add( new NameValData( "Cmzo", Cmzo, "Z viscous moment coefficient." ) );
            res->Add( new NameValData( "Cmxi", Cmxi, "X inviscid moment coefficient." ) );
            res->Add( new NameValData( "Cmyi", Cmyi, "Y inviscid moment coefficient." ) );
            res->Add( new NameValData( "Cmzi", Cmzi, "Z inviscid moment coefficient." ) );
            res->Add( new NameValData( "CL", CL, "Lift coefficient." ) );
            res->Add( new NameValData( "CD", CD, "Drag coefficient." ) );
            res->Add( new NameValData( "CS", CS, "Side force coefficient." ) );
            res->Add( new NameValData( "CLo", CLo, "Viscous contribution to lift coefficient." ) );
            res->Add( new NameValData( "CDo", CDo, "Viscous contribution to drag coefficient.") );
            res->Add( new NameValData( "CSo", CSo, "Viscous contribution to side force coefficient." ) );
            res->Add( new NameValData( "CLi", CLi, "Inviscid contribution to lift coefficient." ) );
            res->Add( new NameValData( "CDi", CDi, "Inviscid contribution to drag coefficient." ) );
            res->Add( new NameValData( "CDi", CSi, "Inviscid contribution to side force coefficient." ) );

        } //end for while !feof(fp)
    }

    std::fclose( fp );

    return;
}

/*******************************************************
Read *.rotor.* file output from VSPAERO
See: VSP_Solver.C in vspaero project
*******************************************************/
void VSPAEROMgrSingleton::ReadRotorResFile( const string &filename, vector <string> &res_id_vector, const string &group_name )
{
    FILE *fp = nullptr;
    bool read_success = false;
    WaitForFile( filename );
    fp = fopen( filename.c_str(), "r" );
    if ( fp == nullptr )
    {
        fprintf( stderr, "ERROR %d: Could not open Rotor file: %s\n\tFile: %s \tLine:%d\n", vsp::VSP_FILE_DOES_NOT_EXIST, filename.c_str(), __FILE__, __LINE__ );
        return;
    }

    Results* res = nullptr;

    // Get group index
    size_t last_index = filename.find_last_not_of( "0123456789" );
    int rotor_num = stoi( filename.substr( last_index + 1 ) );

    // Time       Diameter     RPM       Thrust    Thrusto    Thrusti     Power      Powero     Poweri     Moment     Momento    Momenti      J          CT         CQ         CP        EtaP       CT_H       CQ_H       CP_H       FOM        Angle
    int num_tot_history_data_col = 22;

    // Station     S       Chord     Area      V/Vref   Diameter    RPM      TipVel       CNo_H         CSo_H         CTo_H         CQo_H         CPo_H         CN_H          CS_H          CT_H          CQ_H          CP_H
    int num_load_avg_data_col = 18;

    // Station    Time       Angle     Xqc       Yqc       Zqc       S       Chord     Area      V/Vref   Diameter    RPM      TipVel       CNo_H         CSo_H         CTo_H         CQo_H         CPo_H         CN_H          CS_H          CT_H          CQ_H          CP_H
    int num_load_last_rev_data_col = 23;

    std::vector<string> data_string_array;
    string prev_start_str;
    int blade_load_avg_ind = 1; // Current rotor blade index for the average load data
    int blade_load_last_rev_ind = 1; // Current rotor blade index for the last revolution load data

    // TODO: Read Average over last full revolution

    // Read in all of the data into the results manager
    char seps[] = " :,\t\n";
    while ( !feof( fp ) )
    {
        data_string_array = ReadDelimLine( fp, seps ); //this is also done in some of the embedded loops below
        if ( data_string_array.size() == 0 )
        {
            continue;
        }

        if ( strcmp( data_string_array[0].c_str(), "Time" ) == 0 && strcmp( data_string_array[1].c_str(), "Diameter" ) == 0 )
        {
            res = ResultsMgr.CreateResults( "VSPAERO_Rotor", "VSPAERO Rotor results." );

            res->Add( new NameValData( "Rotor_Num", rotor_num, "Rotor number." ) );
            res->Add( new NameValData( "Group_Name", group_name, "Unsteady group name." ) );

            res_id_vector.push_back( res->GetID() );
        }
        else if ( strcmp( data_string_array[0].c_str(), "Station" ) == 0 && strcmp( prev_start_str.c_str(), "Average" ) == 0 )
        {
            res = ResultsMgr.CreateResults( "VSPAERO_Blade_Avg", "VSPAERO blade average rotor results." );

            res->Add( new NameValData( "Rotor_Num", rotor_num, "Rotor number." ) );
            res->Add( new NameValData( "Blade_Num", blade_load_avg_ind, "Blade number." ) );
            res->Add( new NameValData( "Group_Name", group_name, "Unsteady group name." ) );

            res_id_vector.push_back( res->GetID() );
            blade_load_avg_ind++;
        }
        else if ( strcmp( data_string_array[0].c_str(), "Station" ) == 0 && strcmp( prev_start_str.c_str(), "Time" ) == 0 )
        {
            res = ResultsMgr.CreateResults( "VSPAERO_Blade_Last_Rev", "VSPAERO blade final revolution results." );

            res->Add( new NameValData( "Rotor_Num", rotor_num, "Rotor number." ) );
            res->Add( new NameValData( "Blade_Num", blade_load_last_rev_ind, "Blade number." ) );
            res->Add( new NameValData( "Group_Name", group_name, "Unsteady group name." ) );

            res_id_vector.push_back( res->GetID() );
            blade_load_last_rev_ind++;
        }

        prev_start_str = data_string_array[0];

        // discard the header row and read the next line assuming that it is numeric
        data_string_array = ReadDelimLine( fp, seps );

        if ( res && ( data_string_array.size() == num_tot_history_data_col ||
                      data_string_array.size() == num_load_avg_data_col ||
                      data_string_array.size() == num_load_last_rev_data_col ) )
        {
            if ( data_string_array.size() == num_tot_history_data_col )
            {
                // Raw data vectors
                std::vector < double > Time, Diameter, RPM, Thrust, Thrusto, Thrusti, Moment, Momento, Momenti, Power, Powero, Poweri;
                std::vector < double > J, CT, CQ, CP, EtaP; // Prop coefficients
                std::vector < double > CT_H, CQ_H, CP_H, FOM, Angle; // Rotor coefficients

                // read the data rows
                while ( data_string_array.size() == num_tot_history_data_col )
                {
                    // Store the raw data
                    Time.push_back( std::stod( data_string_array[0] ) );
                    Diameter.push_back( std::stod( data_string_array[1] ) );
                    RPM.push_back( std::stod( data_string_array[2] ) );
                    Thrust.push_back( std::stod( data_string_array[3] ) );
                    Thrusto.push_back( std::stod( data_string_array[4] ) );
                    Thrusti.push_back( std::stod( data_string_array[5] ) );
                    Power.push_back( std::stod( data_string_array[6] ) );
                    Powero.push_back( std::stod( data_string_array[7] ) );
                    Poweri.push_back( std::stod( data_string_array[8] ) );
                    Moment.push_back( std::stod( data_string_array[9] ) );
                    Momento.push_back( std::stod( data_string_array[10] ) );
                    Momenti.push_back( std::stod( data_string_array[11] ) );
                    J.push_back( std::stod( data_string_array[12] ) );
                    CT.push_back( std::stod( data_string_array[13] ) );
                    CQ.push_back( std::stod( data_string_array[14] ) );
                    CP.push_back( std::stod( data_string_array[15] ) );
                    EtaP.push_back( std::stod( data_string_array[16] ) );
                    CT_H.push_back( std::stod( data_string_array[17] ) );
                    CQ_H.push_back( std::stod( data_string_array[18] ) );
                    CP_H.push_back( std::stod( data_string_array[19] ) );
                    FOM.push_back( std::stod( data_string_array[20] ) );
                    Angle.push_back( std::stod( data_string_array[21] ) );

                    // Read the next line and loop
                    data_string_array = ReadDelimLine( fp, seps );
                }

                // Finish up by adding the data to the result res
                res->Add( new NameValData( "Time", Time, "Time in unsteady analysis." ) );
                res->Add( new NameValData( "Diameter", Diameter, "Prop diameter." ) );
                res->Add( new NameValData( "RPM", RPM, "Rotation speed, revolutions per minute." ) );
                res->Add( new NameValData( "Thrust", Thrust, "Rotor thrust." ) );
                res->Add( new NameValData( "Thrusto", Thrusto, "Viscous contribution to thrust." ) );
                res->Add( new NameValData( "Thrusti", Thrusti, "Inviscid contribution to thrust." ) );
                res->Add( new NameValData( "Power", Power, "Rotor power." ) );
                res->Add( new NameValData( "Powero", Powero, "Viscous contribution to power." ) );
                res->Add( new NameValData( "Poweri", Poweri, "Inviscid contribution to power." ) );
                res->Add( new NameValData( "Moment", Moment, "Rotor torque." ) );
                res->Add( new NameValData( "Momento", Momento, "Viscous contribution to torque." ) );
                res->Add( new NameValData( "Momenti", Momenti, "Inviscid contribution to torque." ) );
                res->Add( new NameValData( "J", J, "Advance ratio J=V/(n*D)." ) );
                res->Add( new NameValData( "CT", CT, "Thrust coefficient, propeller convention." ) );
                res->Add( new NameValData( "CQ", CQ, "Torque coefficient, propeller convention." ) );
                res->Add( new NameValData( "CP", CP, "Power coefficient, propeller convention." ) );
                res->Add( new NameValData( "EtaP", EtaP, "Propeller efficiency." ) );
                res->Add( new NameValData( "CT_H", CT_H, "Thrust coefficient, helicopter convention." ) );
                res->Add( new NameValData( "CQ_H", CQ_H, "Torque coefficient, helicopter convention." ) );
                res->Add( new NameValData( "CP_H", CP_H, "Power coefficient, helicopter convention." ) );
                res->Add( new NameValData( "FOM", FOM, "Figure of merit." ) );
                res->Add( new NameValData( "Angle", Angle, "Rotation angle." ) );
            }
            else if ( data_string_array.size() == num_load_avg_data_col )
            {
                // Raw data vectors
                std::vector < int > Station;
                std::vector < double > S, Chord, Area, V_Vref, Diameter, RPM, TipVel;
                std::vector < double > CNo_H, CSo_H, CTo_H, CQo_H, CPo_H, CN_H, CS_H, CT_H, CQ_H, CP_H;

                // read the data rows
                while ( data_string_array.size() == num_load_avg_data_col )
                {
                    // Store the raw data
                    Station.push_back( std::stoi( data_string_array[0] ) );
                    S.push_back( std::stod( data_string_array[1] ) );
                    Chord.push_back( std::stod( data_string_array[2] ) );
                    Area.push_back( std::stod( data_string_array[3] ) );
                    V_Vref.push_back( std::stod( data_string_array[4] ) );
                    Diameter.push_back( std::stod( data_string_array[5] ) );
                    RPM.push_back( std::stod( data_string_array[6] ) );
                    TipVel.push_back( std::stod( data_string_array[7] ) );
                    CNo_H.push_back( std::stod( data_string_array[8] ) );
                    CSo_H.push_back( std::stod( data_string_array[9] ) );
                    CTo_H.push_back( std::stod( data_string_array[10] ) );
                    CQo_H.push_back( std::stod( data_string_array[11] ) );
                    CPo_H.push_back( std::stod( data_string_array[12] ) );
                    CN_H.push_back( std::stod( data_string_array[13] ) );
                    CS_H.push_back( std::stod( data_string_array[14] ) );
                    CT_H.push_back( std::stod( data_string_array[15] ) );
                    CQ_H.push_back( std::stod( data_string_array[16] ) );
                    CP_H.push_back( std::stod( data_string_array[17] ) );

                    // Read the next line and loop
                    data_string_array = ReadDelimLine( fp, seps );
                }

                // Finish up by adding the data to the result res
                res->Add( new NameValData( "Station", Station, "Blade station index." ) );
                res->Add( new NameValData( "S", S, "Non-dimensional blade station coordinate." ) );
                res->Add( new NameValData( "Chord", Chord, "Section chord." ) );
                res->Add( new NameValData( "Area", Area, "Section area." ) );
                res->Add( new NameValData( "V_Vref", V_Vref, "Local velocity ratio." ) );
                res->Add( new NameValData( "Diameter", Diameter, "Rotor diameter." ) );
                res->Add( new NameValData( "RPM", RPM, "Rotation speed, revolutions per minute." ) );
                res->Add( new NameValData( "TipVel", TipVel, "Rotor tip speed." ) );
                res->Add( new NameValData( "CNo_H", CNo_H, "Viscous contribution to normal force coefficient, helicoter convention." ) );
                res->Add( new NameValData( "CSo_H", CSo_H, "Viscous contribution to side force coefficient, helicoter convention." ) );
                res->Add( new NameValData( "CTo_H", CTo_H, "Viscous contribution to thrust coefficient, helicoter convention." ) );
                res->Add( new NameValData( "CQo_H", CQo_H, "Viscous contribution to torque coefficient, helicoter convention." ) );
                res->Add( new NameValData( "CPo_H", CPo_H, "Viscous contribution to power coefficient, helicoter convention." ) );
                res->Add( new NameValData( "CN_H", CN_H, "Normal force coefficient, helicoter convention." ) );
                res->Add( new NameValData( "CS_H", CS_H, "Side force coefficient, helicoter convention." ) );
                res->Add( new NameValData( "CT_H", CT_H, "Thrust coefficient, helicoter convention." ) );
                res->Add( new NameValData( "CQ_H", CQ_H, "Torque coefficient, helicoter convention." ) );
                res->Add( new NameValData( "CP_H", CP_H, "Power coefficient, helicoter convention." ) );
            }
            else if ( data_string_array.size() == num_load_last_rev_data_col )
            {
                // Raw data vectors
                std::vector < int > Station;
                std::vector < double > Time, Angle, Xqc, Yqc, Zqc, S, Chord, Area, V_Vref, Diameter, RPM, TipVel;
                std::vector < double > CNo_H, CSo_H, CTo_H, CQo_H, CPo_H, CN_H, CS_H, CT_H, CQ_H, CP_H;

                // read the data rows
                while ( data_string_array.size() == num_load_last_rev_data_col )
                {
                    if ( strcmp( data_string_array[0].c_str(), "Station" ) == 0 && strcmp( prev_start_str.c_str(), "Time" ) != 0 )
                    {
                        // Skip this line
                        data_string_array = ReadDelimLine( fp, seps );

                        if ( data_string_array.size() != num_load_last_rev_data_col )
                        {
                            break;
                        }
                    }

                    // Store the raw data
                    Station.push_back( std::stoi( data_string_array[0] ) );
                    Time.push_back( std::stod( data_string_array[1] ) );
                    Angle.push_back( std::stod( data_string_array[2] ) );
                    Xqc.push_back( std::stod( data_string_array[3] ) );
                    Yqc.push_back( std::stod( data_string_array[4] ) );
                    Zqc.push_back( std::stod( data_string_array[5] ) );
                    S.push_back( std::stod( data_string_array[6] ) );
                    Chord.push_back( std::stod( data_string_array[7] ) );
                    Area.push_back( std::stod( data_string_array[8] ) );
                    V_Vref.push_back( std::stod( data_string_array[9] ) );
                    Diameter.push_back( std::stod( data_string_array[10] ) );
                    RPM.push_back( std::stod( data_string_array[11] ) );
                    TipVel.push_back( std::stod( data_string_array[12] ) );
                    CNo_H.push_back( std::stod( data_string_array[13] ) );
                    CSo_H.push_back( std::stod( data_string_array[14] ) );
                    CTo_H.push_back( std::stod( data_string_array[15] ) );
                    CQo_H.push_back( std::stod( data_string_array[16] ) );
                    CPo_H.push_back( std::stod( data_string_array[17] ) );
                    CN_H.push_back( std::stod( data_string_array[18] ) );
                    CS_H.push_back( std::stod( data_string_array[19] ) );
                    CT_H.push_back( std::stod( data_string_array[20] ) );
                    CQ_H.push_back( std::stod( data_string_array[21] ) );
                    CP_H.push_back( std::stod( data_string_array[22] ) );

                    // Read the next line and loop
                    data_string_array = ReadDelimLine( fp, seps );
                }

                // Finish up by adding the data to the result res
                res->Add( new NameValData( "Station", Station, "Blade station index." ) );
                res->Add( new NameValData( "Time", Time, "Time in unsteady analysis." ) );
                res->Add( new NameValData( "Angle", Angle, "Rotation angle." ) );
                res->Add( new NameValData( "Xqc", Xqc, "X coordinate of section quarter chord." ) );
                res->Add( new NameValData( "Yqc", Yqc, "Y coordinate of section quarter chord." ) );
                res->Add( new NameValData( "Zqc", Zqc, "Z coordinate of section quarter chord." ) );
                res->Add( new NameValData( "S", S, "Non dimensional blade station coordinate." ) );
                res->Add( new NameValData( "Chord", Chord, "Section chord." ) );
                res->Add( new NameValData( "Area", Area, "Section area." ) );
                res->Add( new NameValData( "V_Vref", V_Vref, "Local velocity ratio." ) );
                res->Add( new NameValData( "Diameter", Diameter, "Rotor diameter" ) );
                res->Add( new NameValData( "RPM", RPM, "Rotation speed, revolutions per minute." ) );
                res->Add( new NameValData( "TipVel", TipVel, "Rotor tip speed." ) );
                res->Add( new NameValData( "CNo_H", CNo_H, "Viscous contribution to normal force coefficient, helicoter convention." ) );
                res->Add( new NameValData( "CSo_H", CSo_H, "Viscous contribution to side force coefficient, helicoter convention." ) );
                res->Add( new NameValData( "CTo_H", CTo_H, "Viscous contribution to thrust coefficient, helicoter convention." ) );
                res->Add( new NameValData( "CQo_H", CQo_H, "Viscous contribution to torque coefficient, helicoter convention." ) );
                res->Add( new NameValData( "CPo_H", CPo_H, "Viscous contribution to power coefficient, helicoter convention." ) );
                res->Add( new NameValData( "CN_H", CN_H, "Normal force coefficient, helicoter convention." ) );
                res->Add( new NameValData( "CS_H", CS_H, "Side force coefficient, helicoter convention." ) );
                res->Add( new NameValData( "CT_H", CT_H, "Thrust coefficient, helicoter convention." ) );
                res->Add( new NameValData( "CQ_H", CQ_H, "Torque coefficient, helicoter convention." ) );
                res->Add( new NameValData( "CP_H", CP_H, "Power coefficient, helicoter convention." ) );
            }
        }
    } //end for while !feof(fp)

    std::fclose( fp );

    return;
}

/*##############################################################################
#                                                                              #
#                               CpSlice                                        #
#                                                                              #
##############################################################################*/

CpSlice::CpSlice() : ParmContainer()
{
    m_Name = "CpSlice";

    m_CutType.Init( "CutType", "CpSlice", this, vsp::Y_DIR, vsp::X_DIR, vsp::Z_DIR );
    m_CutType.SetDescript( "Perpendicular Axis for the Cut" );

    m_CutPosition.Init( "CutPosition", "CpSlice", this, 0.0, -1e12, 1e12 );
    m_CutPosition.SetDescript( "Position of the Cut from Orgin Along Perpendicular Axis" );

    m_DrawCutFlag.Init( "DrawCutFlag", "CpSlice", this, true, false, true );
    m_DrawCutFlag.SetDescript( "Flag to Draw the CpSlice Cutting Plane" );
}

CpSlice::~CpSlice( )
{

}

void CpSlice::ParmChanged( Parm* parm_ptr, int type )
{
    if ( type == Parm::SET )
    {
        m_LateUpdateFlag = true;
        return;
    }

    Vehicle* veh = VehicleMgr.GetVehicle();
    if ( veh )
    {
        veh->ParmChanged( parm_ptr, type );
    }
}

VspSurf CpSlice::CreateSurf()
{
    VspSurf slice_surf = VspSurf();

    Vehicle* veh = VehicleMgr.GetVehicle();
    if ( veh )
    {
        vec3d pnt0, pnt1, pnt2, pnt3;

        vec3d max_pnt = veh->GetBndBox().GetMax();
        vec3d min_pnt = veh->GetBndBox().GetMin();
        double del_x = ( max_pnt.x() - min_pnt.x() ) / 2;
        double del_y = ( max_pnt.y() - min_pnt.y() ) / 2;
        double del_z = ( max_pnt.z() - min_pnt.z() ) / 2;

        vec3d veh_center = veh->GetBndBox().GetCenter();

        // Center at vehicle bounding box center
        if ( m_CutType() == vsp::X_DIR )
        {
            pnt0 = vec3d( m_CutPosition(), del_y + veh_center.y(), del_z + veh_center.z() );
            pnt1 = vec3d( m_CutPosition(), -1 * del_y + veh_center.y(), del_z + veh_center.z() );
            pnt2 = vec3d( m_CutPosition(), del_y + veh_center.y(), -1 * del_z + veh_center.z() );
            pnt3 = vec3d( m_CutPosition(), -1 * del_y + veh_center.y(), -1 * del_z + veh_center.z() );
        }
        else if ( m_CutType() == vsp::Y_DIR )
        {
            pnt0 = vec3d(del_x + veh_center.x(), m_CutPosition(), del_z + veh_center.z() );
            pnt1 = vec3d( -1 * del_x + veh_center.x(), m_CutPosition(), del_z + veh_center.z() );
            pnt2 = vec3d(del_x + veh_center.x(), m_CutPosition(), -1 * del_z + veh_center.z() );
            pnt3 = vec3d( -1 * del_x + veh_center.x(), m_CutPosition(), -1 * del_z + veh_center.z() );
        }
        else if ( m_CutType() == vsp::Z_DIR )
        {
            pnt0 = vec3d(del_x + veh_center.x(), del_y + veh_center.y(), m_CutPosition() );
            pnt1 = vec3d( -1 * del_x + veh_center.x(), del_y + veh_center.y(), m_CutPosition() );
            pnt2 = vec3d(del_x + veh_center.x(), -1 * del_y + veh_center.y(), m_CutPosition() );
            pnt3 = vec3d( -1 * del_x + veh_center.x(), -1 * del_y + veh_center.y(), m_CutPosition() );
        }

        slice_surf.MakePlaneSurf( pnt0, pnt1, pnt2, pnt3 );
    }

    return slice_surf;
}

void CpSlice::LoadDrawObj( vector < DrawObj* > &draw_obj_vec, int id, bool highlight )
{
    // One DrawObj for plane and one for border. This is done to avoid DrawObj ordering transparency issues
    m_CpSliceDOVec.clear();
    m_CpSliceDOVec.resize( 2 );

    if ( m_DrawCutFlag() )
    {
        VspSurf slice_surf = CreateSurf();

        m_CpSliceDOVec[0].m_GeomID = m_Name + "_Plane_" + std::to_string( id );
        m_CpSliceDOVec[0].m_Screen = DrawObj::VSP_MAIN_SCREEN;

        m_CpSliceDOVec[1].m_GeomID = m_Name + "_Border_" + std::to_string( id );
        m_CpSliceDOVec[1].m_Screen = DrawObj::VSP_MAIN_SCREEN;

        if ( highlight )
        {
            m_CpSliceDOVec[1].m_LineColor = vec3d( 1.0, 0.0, 0.0 );
            m_CpSliceDOVec[1].m_LineWidth = 3.0;
        }
        else
        {
            m_CpSliceDOVec[1].m_LineColor = vec3d( 96.0 / 255.0, 96.0 / 255.0, 96.0 / 255.0 );
            m_CpSliceDOVec[1].m_LineWidth = 1.0;
        }

        m_CpSliceDOVec[0].m_Type = DrawObj::VSP_SHADED_QUADS;
        m_CpSliceDOVec[1].m_Type = DrawObj::VSP_LINE_LOOP;

        vec3d p00 = slice_surf.CompPnt01( 0, 0 );
        vec3d p10 = slice_surf.CompPnt01( 1, 0 );
        vec3d p11 = slice_surf.CompPnt01( 1, 1 );
        vec3d p01 = slice_surf.CompPnt01( 0, 1 );

        m_CpSliceDOVec[0].m_PntVec.push_back( p00 );
        m_CpSliceDOVec[0].m_PntVec.push_back( p10 );
        m_CpSliceDOVec[0].m_PntVec.push_back( p11 );
        m_CpSliceDOVec[0].m_PntVec.push_back( p01 );

        m_CpSliceDOVec[1].m_PntVec.push_back( p00 );
        m_CpSliceDOVec[1].m_PntVec.push_back( p10 );
        m_CpSliceDOVec[1].m_PntVec.push_back( p11 );
        m_CpSliceDOVec[1].m_PntVec.push_back( p01 );

        // Get new normal and set plane color to medium glass
        vec3d quadnorm = cross( p10 - p00, p01 - p00 );
        quadnorm.normalize();

        for ( size_t i = 0; i < 4; i++ )
        {
            m_CpSliceDOVec[0].m_MaterialInfo.Ambient[i] = 0.2f;
            m_CpSliceDOVec[0].m_MaterialInfo.Diffuse[i] = 0.1f;
            m_CpSliceDOVec[0].m_MaterialInfo.Specular[i] = 0.7f;
            m_CpSliceDOVec[0].m_MaterialInfo.Emission[i] = 0.0f;

            m_CpSliceDOVec[0].m_NormVec.push_back( quadnorm );
        }

        if ( highlight )
        {
            m_CpSliceDOVec[0].m_MaterialInfo.Diffuse[3] = 0.67f;
        }
        else
        {
            m_CpSliceDOVec[0].m_MaterialInfo.Diffuse[3] = 0.33f;
        }

        m_CpSliceDOVec[0].m_MaterialInfo.Shininess = 5.0f;

        m_CpSliceDOVec[0].m_GeomChanged = true;
        draw_obj_vec.push_back( &m_CpSliceDOVec[0] );

        m_CpSliceDOVec[1].m_GeomChanged = true;
        draw_obj_vec.push_back( &m_CpSliceDOVec[1] );
    }
}

/*##############################################################################
#                                                                              #
#                              PropDriverGroup                                 #
#                                                                              #
##############################################################################*/

PropDriverGroup::PropDriverGroup() : DriverGroup( vsp::NUM_PROP_DRIVER, 3 )
{
    m_CurrChoices[0] = vsp::RPM_PROP_DRIVER;
    m_CurrChoices[1] = vsp::CT_PROP_DRIVER;
    m_CurrChoices[2] = vsp::CP_PROP_DRIVER;
}

void PropDriverGroup::UpdateGroup( vector< string > parmIDs )
{
    Parm* rpm = ParmMgr.FindParm( parmIDs[ vsp::RPM_PROP_DRIVER ] );
    Parm* J = ParmMgr.FindParm( parmIDs[ vsp::J_PROP_DRIVER ] );
    Parm* CT = ParmMgr.FindParm( parmIDs[ vsp::CT_PROP_DRIVER ] );
    Parm* thrust = ParmMgr.FindParm( parmIDs[ vsp::T_PROP_DRIVER ] );
    Parm* CP = ParmMgr.FindParm( parmIDs[ vsp::CP_PROP_DRIVER ] );
    Parm* power = ParmMgr.FindParm( parmIDs[ vsp::P_PROP_DRIVER ] );
    Parm* CQ = ParmMgr.FindParm( parmIDs[ vsp::CQ_PROP_DRIVER ] );
    Parm* torque = ParmMgr.FindParm( parmIDs[ vsp::Q_PROP_DRIVER ] );
    Parm* eta = ParmMgr.FindParm( parmIDs[ vsp::ETA_PROP_DRIVER ] );

    vector< bool > uptodate( m_Nvar, false );

    for( int i = 0; i < m_Nchoice; i++ )
    {
        uptodate[ m_CurrChoices[ i ] ] = true;
    }

//    for( int i = 0; i < m_Nvar; i++ )
//    {
//        if ( !uptodate[ i ] )
//        {
//            Parm * p = ParmMgr.FindParm( parmIDs[ i ] );
//            if ( p )
//            {
//                p->Set( 0.0 );
//            }
//        }
//    }


    double D4 = D * D * D * D;
    double D5 = D4 * D;

    int niter = 0;
    while( vector_contains_val( uptodate, false ) ) //  && niter < ( m_Nvar - m_Nchoice ) )
    {

        if ( !uptodate[ vsp::J_PROP_DRIVER ] )
        {
            if ( uptodate[ vsp::RPM_PROP_DRIVER ] )
            {
                double nrps = rpm->Get() / 60.0;

                J->Set( Vinf / ( nrps * D ) );
                uptodate[ vsp::J_PROP_DRIVER ] = true;
            }
            else if ( uptodate[ vsp::CT_PROP_DRIVER ] && uptodate[ vsp::CP_PROP_DRIVER ] && uptodate[ vsp::ETA_PROP_DRIVER ] )
            {
                J->Set( eta->Get() * CP->Get() / CT->Get() );
                uptodate[ vsp::J_PROP_DRIVER ] = true;
            }
        }

        if ( !uptodate[ vsp::RPM_PROP_DRIVER ] )
        {
            if ( uptodate[ vsp::J_PROP_DRIVER ] )
            {
                double nrps = Vinf / ( J->Get() * D );

                rpm->Set( nrps * 60.0 );
                uptodate[ vsp::RPM_PROP_DRIVER ] = true;
            }
            else if ( uptodate[ vsp::CP_PROP_DRIVER ] && uptodate[ vsp::P_PROP_DRIVER ] )
            {
                double nrps = pow( power->Get() / ( CP->Get() * rho * D5 ), 1.0 / 3.0 );
                rpm->Set( nrps * 60.0 );
                uptodate[ vsp::RPM_PROP_DRIVER ] = true;
            }
            else if ( uptodate[ vsp::CQ_PROP_DRIVER ] && uptodate[ vsp::Q_PROP_DRIVER ] )
            {
                double nrps = sqrt( torque->Get() / ( CQ->Get() * rho * D5 ) );
                rpm->Set( nrps * 60.0 );
                uptodate[ vsp::RPM_PROP_DRIVER ] = true;
            }
            else if ( uptodate[ vsp::CT_PROP_DRIVER ] && uptodate[ vsp::T_PROP_DRIVER ] )
            {
                double nrps = sqrt( thrust->Get() / ( CT->Get() * rho * D4 ) );
                rpm->Set( nrps * 60.0 );
                uptodate[ vsp::RPM_PROP_DRIVER ] = true;
            }
        }

        if ( !uptodate[ vsp::CQ_PROP_DRIVER ] )
        {
            if ( uptodate[ vsp::CP_PROP_DRIVER ] )
            {
                CQ->Set( CP->Get() / ( 2.0 * PI ) );
                uptodate[ vsp::CQ_PROP_DRIVER ] = true;
            }
            else if ( uptodate[ vsp::Q_PROP_DRIVER] && uptodate[ vsp::RPM_PROP_DRIVER ] )
            {
                double nrps = rpm->Get() / 60.0;
                double n2 = nrps * nrps;
                CQ->Set( torque->Get() / ( rho * n2 * D5 ) );
                uptodate[ vsp::CQ_PROP_DRIVER ] = true;
            }
        }

        if ( !uptodate[ vsp::CP_PROP_DRIVER ] )
        {
            if ( uptodate[ vsp::CQ_PROP_DRIVER ] )
            {
                CP->Set( CQ->Get() * 2.0 * PI );
                uptodate[ vsp::CP_PROP_DRIVER ] = true;
            }
            else if ( uptodate[ vsp::P_PROP_DRIVER] && uptodate[ vsp::RPM_PROP_DRIVER ] )
            {
                double nrps = rpm->Get() / 60.0;
                double n3 = nrps * nrps * nrps;
                CP->Set( power->Get() / ( rho * n3 * D5 ) );
                uptodate[ vsp::CP_PROP_DRIVER ] = true;
            }
            else if ( uptodate[ vsp::J_PROP_DRIVER] && uptodate[ vsp::CT_PROP_DRIVER ] && uptodate[ vsp::ETA_PROP_DRIVER ] )
            {
                CP->Set( J->Get() * CT->Get() / eta->Get() );
                uptodate[ vsp::CP_PROP_DRIVER ] = true;
            }
        }

        if ( !uptodate[ vsp::CT_PROP_DRIVER ] )
        {
            if ( uptodate[ vsp::T_PROP_DRIVER] && uptodate[ vsp::RPM_PROP_DRIVER ] )
            {
                double nrps = rpm->Get() / 60.0;
                double n2 = nrps * nrps;
                CT->Set( thrust->Get() / ( rho * n2 * D4 ) );
                uptodate[ vsp::CT_PROP_DRIVER ] = true;
            }
            else if ( uptodate[ vsp::J_PROP_DRIVER] && uptodate[ vsp::CP_PROP_DRIVER ] && uptodate[ vsp::ETA_PROP_DRIVER ] )
            {
                CT->Set( eta->Get() * CP->Get() / J->Get() );
                uptodate[ vsp::CT_PROP_DRIVER ] = true;
            }
        }

        if ( !uptodate[ vsp::Q_PROP_DRIVER ] )
        {
            if ( uptodate[ vsp::CQ_PROP_DRIVER] && uptodate[ vsp::RPM_PROP_DRIVER ] )
            {
                double nrps = rpm->Get() / 60.0;
                double n2 = nrps * nrps;
                torque->Set( CQ->Get() * rho * n2 * D5 );
                uptodate[ vsp::Q_PROP_DRIVER ] = true;
            }
            else if ( uptodate[ vsp::P_PROP_DRIVER] && uptodate[ vsp::RPM_PROP_DRIVER ] )
            {
                double nrps = rpm->Get() / 60.0;
                torque->Set( power->Get() / ( nrps * 2.0 * PI ) );
                uptodate[ vsp::Q_PROP_DRIVER ] = true;
            }
        }

        if ( !uptodate[ vsp::P_PROP_DRIVER ] )
        {
            if ( uptodate[ vsp::CP_PROP_DRIVER] && uptodate[ vsp::RPM_PROP_DRIVER ] )
            {
                double nrps = rpm->Get() / 60.0;
                double n3 = nrps * nrps * nrps;
                power->Set( CP->Get() * rho * n3 * D5 );
                uptodate[ vsp::P_PROP_DRIVER ] = true;
            }
            else if ( uptodate[ vsp::Q_PROP_DRIVER] && uptodate[ vsp::RPM_PROP_DRIVER ] )
            {
                double nrps = rpm->Get() / 60.0;
                power->Set( torque->Get() * nrps * 2.0 * PI );
                uptodate[ vsp::P_PROP_DRIVER ] = true;
            }
            else if ( uptodate[ vsp::ETA_PROP_DRIVER ] && uptodate[ vsp::T_PROP_DRIVER ] )
            {
                power->Set( thrust->Get() * Vinf / eta->Get() );
                uptodate[ vsp::P_PROP_DRIVER ] = true;
            }
        }

        if ( !uptodate[ vsp::T_PROP_DRIVER ] )
        {
            if ( uptodate[ vsp::CT_PROP_DRIVER] && uptodate[ vsp::RPM_PROP_DRIVER ] )
            {
                double nrps = rpm->Get() / 60.0;
                double n2 = nrps * nrps;
                thrust->Set( CT->Get() * rho * n2 * D4 );
                uptodate[ vsp::T_PROP_DRIVER ] = true;
            }
            else if ( uptodate[ vsp::ETA_PROP_DRIVER ] && uptodate[ vsp::P_PROP_DRIVER ] )
            {
                thrust->Set( eta->Get() * power->Get() / Vinf );
                uptodate[ vsp::T_PROP_DRIVER ] = true;
            }
        }

        if ( !uptodate[ vsp::ETA_PROP_DRIVER ] )
        {
            if ( uptodate[ vsp::J_PROP_DRIVER] && uptodate[ vsp::CT_PROP_DRIVER ] && uptodate[ vsp::CP_PROP_DRIVER ] )
            {
                eta->Set( J->Get() * CT->Get() / CP->Get() );
                uptodate[ vsp::ETA_PROP_DRIVER ] = true;
            }
            else if ( uptodate[ vsp::T_PROP_DRIVER ] && uptodate[ vsp::P_PROP_DRIVER ] )
            {
                eta->Set( Vinf * thrust->Get() / power->Get() );
                uptodate[ vsp::ETA_PROP_DRIVER ] = true;
            }
        }


        // Each pass through the loop should update at least one variable.
        // With m_Nvar variables and m_Nchoice initially known, all should
        // be updated in ( m_Nvar - m_Nchoice ) iterations.  If not, we're
        // in an infinite loop.
        assert( niter < ( m_Nvar - m_Nchoice ) );
        niter++;
    }
}

bool PropDriverGroup::ValidDrivers( vector< int > choices )
{
    // Check for duplicate selections.
    for( int i = 0; i < (int)choices.size() - 1; i++ )
    {
        for( int j = i + 1; j < (int)choices.size(); j++ )
        {
            if( choices[i] == choices[j] )
            {
                return false;
            }
        }
    }

    // Check for algebraically nonsense selections.
    if( vector_contains_val( choices, ( int ) vsp::RPM_PROP_DRIVER ) &&
        vector_contains_val( choices, ( int ) vsp::J_PROP_DRIVER ) )
    {
        return false;
    }

    if( vector_contains_val( choices, ( int ) vsp::CP_PROP_DRIVER ) &&
        vector_contains_val( choices, ( int ) vsp::CQ_PROP_DRIVER ) )
    {
        return false;
    }

    if( vector_contains_val( choices, ( int ) vsp::P_PROP_DRIVER ) &&
        vector_contains_val( choices, ( int ) vsp::Q_PROP_DRIVER ) )
    {
        return false;
    }

    if( vector_contains_val( choices, ( int ) vsp::RPM_PROP_DRIVER ) &&
        vector_contains_val( choices, ( int ) vsp::CT_PROP_DRIVER ) &&
        vector_contains_val( choices, ( int ) vsp::T_PROP_DRIVER ) )
    {
        return false;
    }

    if( vector_contains_val( choices, ( int ) vsp::J_PROP_DRIVER ) &&
        vector_contains_val( choices, ( int ) vsp::CT_PROP_DRIVER ) &&
        vector_contains_val( choices, ( int ) vsp::T_PROP_DRIVER ) )
    {
        return false;
    }

    if( vector_contains_val( choices, ( int ) vsp::RPM_PROP_DRIVER ) &&
        vector_contains_val( choices, ( int ) vsp::CP_PROP_DRIVER ) &&
        vector_contains_val( choices, ( int ) vsp::P_PROP_DRIVER ) )
    {
        return false;
    }

    if( vector_contains_val( choices, ( int ) vsp::J_PROP_DRIVER ) &&
        vector_contains_val( choices, ( int ) vsp::CP_PROP_DRIVER ) &&
        vector_contains_val( choices, ( int ) vsp::P_PROP_DRIVER ) )
    {
        return false;
    }

    if( vector_contains_val( choices, ( int ) vsp::RPM_PROP_DRIVER ) &&
        vector_contains_val( choices, ( int ) vsp::CQ_PROP_DRIVER ) &&
        vector_contains_val( choices, ( int ) vsp::Q_PROP_DRIVER ) )
    {
        return false;
    }

    if( vector_contains_val( choices, ( int ) vsp::J_PROP_DRIVER ) &&
        vector_contains_val( choices, ( int ) vsp::CQ_PROP_DRIVER ) &&
        vector_contains_val( choices, ( int ) vsp::Q_PROP_DRIVER ) )
    {
        return false;
    }

    if( vector_contains_val( choices, ( int ) vsp::RPM_PROP_DRIVER ) &&
        vector_contains_val( choices, ( int ) vsp::CQ_PROP_DRIVER ) &&
        vector_contains_val( choices, ( int ) vsp::P_PROP_DRIVER ) )
    {
        return false;
    }

    if( vector_contains_val( choices, ( int ) vsp::J_PROP_DRIVER ) &&
        vector_contains_val( choices, ( int ) vsp::CQ_PROP_DRIVER ) &&
        vector_contains_val( choices, ( int ) vsp::P_PROP_DRIVER ) )
    {
        return false;
    }

    if( vector_contains_val( choices, ( int ) vsp::RPM_PROP_DRIVER ) &&
        vector_contains_val( choices, ( int ) vsp::CP_PROP_DRIVER ) &&
        vector_contains_val( choices, ( int ) vsp::Q_PROP_DRIVER ) )
    {
        return false;
    }

    if( vector_contains_val( choices, ( int ) vsp::J_PROP_DRIVER ) &&
        vector_contains_val( choices, ( int ) vsp::CP_PROP_DRIVER ) &&
        vector_contains_val( choices, ( int ) vsp::Q_PROP_DRIVER ) )
    {
        return false;
    }

    if( vector_contains_val( choices, ( int ) vsp::T_PROP_DRIVER ) &&
        vector_contains_val( choices, ( int ) vsp::P_PROP_DRIVER ) &&
        vector_contains_val( choices, ( int ) vsp::ETA_PROP_DRIVER ) )
    {
        return false;
    }

    if( vector_contains_val( choices, ( int ) vsp::T_PROP_DRIVER ) &&
        vector_contains_val( choices, ( int ) vsp::Q_PROP_DRIVER ) &&
        vector_contains_val( choices, ( int ) vsp::ETA_PROP_DRIVER ) )
    {
        return false;
    }

    if( vector_contains_val( choices, ( int ) vsp::CT_PROP_DRIVER ) &&
        vector_contains_val( choices, ( int ) vsp::Q_PROP_DRIVER ) &&
        vector_contains_val( choices, ( int ) vsp::ETA_PROP_DRIVER ) )
    {
        return false;
    }

    return true;
}

/*##############################################################################
#                                                                              #
#                              RotorDisk                                       #
#                                                                              #
##############################################################################*/

RotorDisk::RotorDisk( void ) : ParmContainer()
{
    m_Name = "RotorDisk";
    m_GroupName = "Rotor";

    m_XYZ.set_xyz( 0, 0, 0 );           // RotorXYZ_
    m_Normal.set_xyz( 0, 0, 0 );        // RotorNormal_

    m_Diameter.Init( "RotorDiameter", m_GroupName, this, 10.0, 0.0, 1e12 );       // RotorDiameter_
    m_Diameter.SetDescript( "Rotor Diameter" );

    m_HubDiameter.Init( "RotorHubDiameter", m_GroupName, this, 0.0, 0.0, 1e12 );    // RotorHubDiameter_
    m_HubDiameter.SetDescript( "Rotor Hub Diameter" );

    m_RPM.Init( "RotorRPM", m_GroupName, this, 2000.0, -1e12, 1e12 );       // RotorRPM_
    m_RPM.SetDescript( "Rotor RPM" );

    m_AutoHubDiaFlag.Init( "AutoHubDiaFlag", m_GroupName, this, true, false, true );
    m_AutoHubDiaFlag.SetDescript( "Flag to Automatically Set Hub Diameter from Prop Geom" );

    m_CT.Init( "RotorCT", m_GroupName, this, 0.4, -1e3, 1e3 );       // Rotor_CT_
    m_CT.SetDescript( "Rotor Coefficient of Thrust" );

    m_CP.Init( "RotorCP", m_GroupName, this, 0.6, -1e3, 1e3 );        // Rotor_CP_
    m_CP.SetDescript( "Rotor Coefficient of Power" );

    m_CQ.Init( "RotorCQ", m_GroupName, this, 0.6, -1e3, 1e3 );
    m_CQ.SetDescript( "Rotor Coefficient of Torque" );

    m_T.Init( "RotorThrust", m_GroupName, this,  1.0, -1e12, 1e12 );
    m_T.SetDescript( "Rotor thrust" );

    m_P.Init( "RotorPower", m_GroupName, this,  1.0, -1e12, 1e12 );
    m_P.SetDescript( "Rotor power" );

    m_Q.Init( "RotorTorque", m_GroupName, this,  1.0, -1e12, 1e12 );
    m_Q.SetDescript( "Rotor torque" );

    m_J.Init( "RotorJ", m_GroupName, this, 0.6, -1e6, 1e6 );
    m_J.SetDescript( "Rotor advance ratio" );

    m_eta.Init( "RotorEta", m_GroupName, this, 0.9, -1e3, 1e3 );
    m_eta.SetDescript( "Rotor efficiency" );

    m_ParentGeomId = "";
    m_ParentGeomSurfNdx = -1;
    m_FlipNormalFlag = false;
}


RotorDisk::~RotorDisk( void )
{
}

void RotorDisk::Update( double V, double rho )
{
    m_DriverGroup.rho = rho;
    m_DriverGroup.Vinf = V;
    m_DriverGroup.D = m_Diameter();

    m_DriverGroup.UpdateGroup( GetDriverParms() );

//    double D = m_Diameter();
//    double D2 = D * D;
//    double D4 = D2 * D2;
//    double D5 = D4 * D;
//
//    double nrps = std::abs( m_RPM() ) / 60.0;
//    double n2 = nrps * nrps;
//    double n3 = n2 * nrps;
//
//    double CT = m_CT();
//    double CP = m_CP();
//
//    double J = V / ( nrps * D );
//
//    double eta = J * CT / CP;
//
//    double T = CT * rho * n2 * D4;
//    double P = CP * rho * n3 * D5;
//    double CQ = CP / ( 2.0 * PI );
//    double Q = CQ * rho * n2 * D5;
//
//    m_T = T;
//    m_J = J;
//    m_eta = eta;
}

void RotorDisk::Write_STP_Data( FILE *InputFile )
{

    // Write out RotorDisk to file

    fprintf( InputFile, "%lf %lf %lf \n", m_XYZ.x(), m_XYZ.y(), m_XYZ.z() );

    fprintf( InputFile, "%lf %lf %lf \n", m_Normal.x(), m_Normal.y(), m_Normal.z() );

    fprintf( InputFile, "%lf \n", m_Diameter() / 2.0 );

    fprintf( InputFile, "%lf \n", m_HubDiameter() / 2.0 );

    if ( m_FlipNormalFlag )
    {
        fprintf( InputFile, "%lf \n", -m_RPM() );
    }
    else
    {
        fprintf( InputFile, "%lf \n", m_RPM() );
    }

    fprintf( InputFile, "%lf \n", m_CT() );

    fprintf( InputFile, "%lf \n", m_CP() );

}

//==== Get Driver Parms ====//
vector< string > RotorDisk::GetDriverParms()
{
    vector< string > parm_ids;
    parm_ids.resize( vsp::NUM_PROP_DRIVER );
    parm_ids[ vsp::RPM_PROP_DRIVER ] = m_RPM.GetID();
    parm_ids[ vsp::J_PROP_DRIVER ] = m_J.GetID();
    parm_ids[ vsp::CT_PROP_DRIVER ] = m_CT.GetID();
    parm_ids[ vsp::T_PROP_DRIVER ] = m_T.GetID();
    parm_ids[ vsp::CP_PROP_DRIVER ] = m_CP.GetID();
    parm_ids[ vsp::P_PROP_DRIVER ] = m_P.GetID();
    parm_ids[ vsp::CQ_PROP_DRIVER ] = m_CQ.GetID();
    parm_ids[ vsp::Q_PROP_DRIVER ] = m_Q.GetID();
    parm_ids[ vsp::ETA_PROP_DRIVER ] = m_eta.GetID();

    return parm_ids;
}

xmlNodePtr RotorDisk::EncodeXml( xmlNodePtr & node )
{
    if ( node )
    {
        ParmContainer::EncodeXml( node );
        XmlUtil::AddStringNode( node, "ParentID", m_ParentGeomId.c_str() );
        XmlUtil::AddIntNode( node, "SurfIndex", m_ParentGeomSurfNdx );
    }

    return node;
}

xmlNodePtr RotorDisk::DecodeXml( xmlNodePtr & node )
{
    string defstr = "";
    int defint = 0;
    if ( node )
    {
        ParmContainer::DecodeXml( node );
        m_ParentGeomId = ParmMgr.RemapID( XmlUtil::FindString( node, "ParentID", defstr ) );
        m_ParentGeomSurfNdx = XmlUtil::FindInt( node, "SurfIndex", defint );
    }

    return node;
}

void RotorDisk::SetGroupDisplaySuffix(int num)
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
}

/*##############################################################################
#                                                                              #
#                        ControlSurfaceGroup                                   #
#                                                                              #
##############################################################################*/

ControlSurfaceGroup::ControlSurfaceGroup( void ) : ParmContainer()
{
    m_Name = "Unnamed Control Group";
    m_ParentGeomBaseID = "";

    m_GroupName = "ControlSurfaceGroup";

    m_IsUsed.Init( "ActiveFlag", m_GroupName, this, true, false, true );
    m_IsUsed.SetDescript( "Flag to determine whether or not this group will be used in VSPAero" );

    m_DeflectionAngle.Init( "DeflectionAngle", m_GroupName, this, 0.0, -1.0e12, 1.0e12 );
    m_DeflectionAngle.SetDescript( "Angle of deflection for the control group" );
}

ControlSurfaceGroup::~ControlSurfaceGroup( void )
{
    for ( int i = 0 ; i < (int)m_DeflectionGainVec.size() ; i++ )
    {
        delete m_DeflectionGainVec[i];
    }
    m_DeflectionGainVec.clear();
}

void ControlSurfaceGroup::Write_STP_Data( FILE *InputFile )
{
    Vehicle* veh = VehicleMgr.GetVehicle();
    if ( !veh )
    {
        return;
    }

    string nospacename;

    // Write out Control surface group to .vspaero file
    nospacename = m_Name;
    StringUtil::change_space_to_underscore( nospacename );
    fprintf( InputFile, "%s\n", nospacename.c_str() );

    // surface names ( Cannot have trailing commas )
    unsigned int i = 0;
    for ( i = 0; i < m_ControlSurfVec.size() - 1; i++ )
    {
        nospacename = m_ControlSurfVec[i].fullName;
        StringUtil::change_space_to_underscore( nospacename );
        fprintf( InputFile, "%s,", nospacename.c_str() );
    }
    nospacename = m_ControlSurfVec[i++].fullName;
    StringUtil::change_space_to_underscore( nospacename );
    fprintf( InputFile, "%s\n", nospacename.c_str() );

    // deflection mixing gains ( Cannot have trailing commas )
    for ( i = 0; i < m_DeflectionGainVec.size() - 1; i++ )
    {
        fprintf( InputFile, "%lg, ", m_DeflectionGainVec[i]->Get() );
    }
    fprintf( InputFile, "%lg\n", m_DeflectionGainVec[i++]->Get() );

    // group deflection angle
    fprintf( InputFile, "%lg\n", m_DeflectionAngle() );

}

void ControlSurfaceGroup::Load_STP_Data( FILE *InputFile )
{
    //TODO - need to write function to load data from .vspaero file
}

xmlNodePtr ControlSurfaceGroup::EncodeXml( xmlNodePtr & node )
{
    if ( node )
    {
        XmlUtil::AddStringNode( node, "ParentGeomBase", m_ParentGeomBaseID.c_str() );

        XmlUtil::AddIntNode( node, "NumberOfControlSubSurfaces", m_ControlSurfVec.size() );
        for ( size_t i = 0; i < m_ControlSurfVec.size(); ++i )
        {
            xmlNodePtr csnode = xmlNewChild( node, nullptr, BAD_CAST "Control_Surface" , nullptr );

            XmlUtil::AddStringNode( csnode, "SSID", m_ControlSurfVec[i].SSID.c_str() );
            XmlUtil::AddStringNode( csnode, "ParentGeomID", m_ControlSurfVec[i].parentGeomId.c_str() );
            XmlUtil::AddIntNode( csnode, "iReflect", m_ControlSurfVec[i].iReflect );
        }

        ParmContainer::EncodeXml( node );
    }

    return node;
}

xmlNodePtr ControlSurfaceGroup::DecodeXml( xmlNodePtr & node )
{
    unsigned int nControlSubSurfaces = 0;
    string ParentGeomID;
    string SSID;

    int iReflect = 0;
    VspAeroControlSurf newSurf;

    if ( node )
    {
        m_ParentGeomBaseID = ParmMgr.RemapID( XmlUtil::FindString( node, "ParentGeomBase", ParentGeomID ) );

        nControlSubSurfaces = XmlUtil::FindInt( node, "NumberOfControlSubSurfaces", nControlSubSurfaces );
        for ( size_t i = 0; i < nControlSubSurfaces; ++i )
        {
            xmlNodePtr csnode = XmlUtil::GetNode( node, "Control_Surface", i );

            newSurf.SSID = ParmMgr.RemapID( XmlUtil::FindString( csnode, "SSID", SSID ) );
            newSurf.parentGeomId = ParmMgr.RemapID( XmlUtil::FindString( csnode, "ParentGeomID", ParentGeomID ) );
            newSurf.iReflect = XmlUtil::FindInt( csnode, "iReflect", iReflect );
            AddSubSurface( newSurf );
        }

        ParmContainer::DecodeXml( node ); // Comes after AddSubSurface() to prevent overwriting of newly initialized Parms
    }

    return node;
}

void ControlSurfaceGroup::AddSubSurface( const VspAeroControlSurf &control_surf )
{
    // Add deflection gain parm to ControlSurfaceGroup container
    Parm* p = ParmMgr.CreateParm( vsp::PARM_DOUBLE_TYPE );
    char str[256];

    if ( p )
    {
        //  parm name: control_surf->fullName (example: MainWing_Surf1_Aileron)
        //  group: "ControlSurfaceGroup"
        //  initial value: control_surf->deflection_gain
        snprintf( str, sizeof( str ),  "Surf_%s_%u_Gain", control_surf.SSID.c_str(), control_surf.iReflect );
        p->Init( str, m_GroupName, this, 1.0, -1.0e6, 1.0e6 );
        p->SetDescript( "Deflection gain for the individual sub surface to be used for control mixing and allocation within the control surface group" );
        m_DeflectionGainVec.push_back( p );
    }

    m_ControlSurfVec.push_back( control_surf );
}

void ControlSurfaceGroup::RemoveSubSurface( const string & ssid, int reflec_num )
{
    for ( int i = m_ControlSurfVec.size() - 1; i >= 0; i-- ) // Iterate in reverse as vector is changing size.
    {
        if ( m_ControlSurfVec[i].SSID.compare( ssid ) == 0 && m_ControlSurfVec[i].iReflect == reflec_num )
        {
            m_ControlSurfVec.erase( m_ControlSurfVec.begin() + i );
            delete m_DeflectionGainVec[i];
            m_DeflectionGainVec.erase( m_DeflectionGainVec.begin() + i );
            return;
        }
    }
}

void ControlSurfaceGroup::SetGroupDisplaySuffix( int num )
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
}

/*##############################################################################
#                                                                              #
#                           UnsteadyGroup                                      #
#                                                                              #
##############################################################################*/

UnsteadyGroup::UnsteadyGroup( void ) : ParmContainer()
{
    m_Name = "Unnamed Unsteady Group";

    m_GroupName = "UnsteadyGroup";

    m_GeomPropertyType.Init( "GeomPropertyType", m_GroupName, this, GEOM_DYNAMIC, GEOM_FIXED, GEOM_ROTOR );
    m_GeomPropertyType.SetDescript( "Flag indicating if the geometry is fixed" );

    m_RotorDia.Init( "RotorDia", m_GroupName, this, 0, 0, 1e12 );
    m_RotorDia.SetDescript( "Rotor diameter if geometry is a rotor" );

    m_Ox.Init( "Ox", m_GroupName, this, 0, -1e12, 1e12 );
    m_Ox.SetDescript( "X component of unsteady group origin of rotation" );

    m_Oy.Init( "Oy", m_GroupName, this, 0, -1e12, 1e12 );
    m_Oy.SetDescript( "Y component of unsteady group origin of rotation" );

    m_Oz.Init( "Oz", m_GroupName, this, 0, -1e12, 1e12 );
    m_Oz.SetDescript( "Y component of unsteady group origin of rotation" );

    m_Rx.Init( "Rx", m_GroupName, this, 0, -1e12, 1e12 );
    m_Rx.SetDescript( "X component of unsteady group direction of rotation axis" );

    m_Ry.Init( "Ry", m_GroupName, this, 0, -1e12, 1e12 );
    m_Ry.SetDescript( "Y component of unsteady group direction of rotation axis" );

    m_Rz.Init( "Rz", m_GroupName, this, 0, -1e12, 1e12 );
    m_Rz.SetDescript( "Z component of unsteady group direction of rotation axis" );

    m_Vx.Init( "Vx", m_GroupName, this, 0, -1e12, 1e12 );
    m_Vx.SetDescript( "X component of unsteady group velocity vector" );

    m_Vy.Init( "Vy", m_GroupName, this, 0, -1e12, 1e12 );
    m_Vy.SetDescript( "Y component of unsteady group velocity vector" );

    m_Vz.Init( "Vz", m_GroupName, this, 0, -1e12, 1e12 );
    m_Vz.SetDescript( "Z component of unsteady group velocity vector" );

    m_Ax.Init( "Ax", m_GroupName, this, 0, -1e12, 1e12 );
    m_Ax.SetDescript( "X component of unsteady group acceleration vector" );

    m_Ay.Init( "Ay", m_GroupName, this, 0, -1e12, 1e12 );
    m_Ay.SetDescript( "Y component of unsteady group acceleration vector" );

    m_Az.Init( "Az", m_GroupName, this, 0, -1e12, 1e12 );
    m_Az.SetDescript( "Z component of unsteady group acceleration vector" );

    m_RPM.Init( "RPM", m_GroupName, this, 2000, -1e12, 1e12 );
    m_RPM.SetDescript( "RPM of unsteady group" );

    m_Mass.Init( "Mass", m_GroupName, this, 0, 0, 1e12 );
    m_Mass.SetDescript( "Mass of unsteady group" );

    m_Ixx.Init( "Ixx", m_GroupName, this, 0, 0, 1e12 );
    m_Ixx.SetDescript( "Ixx of unsteady group" );

    m_Iyy.Init( "Iyy", m_GroupName, this, 0, 0, 1e12 );
    m_Iyy.SetDescript( "Iyy of unsteady group" );

    m_Izz.Init( "Izz", m_GroupName, this, 0, 0, 1e12 );
    m_Izz.SetDescript( "Izz of unsteady group" );

    m_Ixy.Init( "Ixy", m_GroupName, this, 0, 0, 1e12 );
    m_Ixy.SetDescript( "Ixy of unsteady group" );

    m_Ixz.Init( "Ixz", m_GroupName, this, 0, 0, 1e12 );
    m_Ixz.SetDescript( "Ixz of unsteady group" );

    m_Iyz.Init( "Iyz", m_GroupName, this, 0, 0, 1e12 );
    m_Iyz.SetDescript( "Iyz of unsteady group" );

    m_ReverseFlag = false;
}

UnsteadyGroup::~UnsteadyGroup( void )
{
}

xmlNodePtr UnsteadyGroup::EncodeXml( xmlNodePtr& node )
{
    if ( node )
    {
        XmlUtil::AddIntNode( node, "NumberOfComponents", m_ComponentSurfPairVec.size() );

        for ( size_t i = 0; i < m_ComponentSurfPairVec.size(); ++i )
        {
            xmlNodePtr csnode = xmlNewChild( node, nullptr, BAD_CAST "Component", nullptr );
            XmlUtil::AddStringNode( csnode, "CompID", m_ComponentSurfPairVec[i].first.c_str() );
            XmlUtil::AddIntNode( csnode, "SurfIndex", m_ComponentSurfPairVec[i].second );
        }

        ParmContainer::EncodeXml( node );
    }

    return node;
}

xmlNodePtr UnsteadyGroup::DecodeXml( xmlNodePtr& node )
{
    if ( node )
    {
        unsigned int nComponents = XmlUtil::FindInt( node, "NumberOfComponents", 0 );

        for ( size_t i = 0; i < nComponents; ++i )
        {
            xmlNodePtr csnode = XmlUtil::GetNode( node, "Component", i );

            string compID = ParmMgr.RemapID( XmlUtil::FindString( csnode, "CompID", "" ) );
            int surf_index = XmlUtil::FindInt( csnode, "SurfIndex", 1 );
            AddComp( compID, surf_index );
        }

        ParmContainer::DecodeXml( node );
    }

    return node;
}

void UnsteadyGroup::ParmChanged( Parm* parm_ptr, int type )
{
    if ( type == Parm::SET )
    {
        m_LateUpdateFlag = true;
        return;
    }

    // Identify if unsteady prop RPM is changed. If so, update it to be the "master" that 
    // all other unsteady prop RPM will be set to
    if ( VSPAEROMgr.m_RotateBladesFlag() && VSPAEROMgr.m_UniformPropRPMFlag() &&
         &m_RPM == parm_ptr && m_GeomPropertyType() == UnsteadyGroup::GEOM_ROTOR )
    {
        VSPAEROMgr.SetCurrentUnsteadyGroupIndex( m_ID );
    }

    Update();

    Vehicle* veh = VehicleMgr.GetVehicle();
    if ( veh )
    {
        veh->ParmChanged( parm_ptr, type );
    }
}

void UnsteadyGroup::Update()
{
    Vehicle* veh = VehicleMgr.GetVehicle();
    if ( !veh )
    {
        return;
    }

    bool is_rotor = false;
    double rotor_dia = 0;
    vec3d o_vec = vec3d( 0, 0, 0 );
    vec3d r_vec = vec3d( 0, 0, 0 );

    if ( m_ComponentSurfPairVec.size() == 1 )
    {
        Geom* geom = veh->FindGeom( m_ComponentSurfPairVec[0].first );

        if ( geom )
        {
            if ( geom->GetType().m_Type == PROP_GEOM_TYPE )
            {
                PropGeom* prop = dynamic_cast<PropGeom*>( geom );
                assert( prop );

                is_rotor = true;

                int surf_index = m_ComponentSurfPairVec[0].second;
                int num_main_surf = geom->GetNumMainSurfs();
                vector < Matrix4d > trans_mat_vec = geom->GetTransMatVec();
                Matrix4d trans_mat = trans_mat_vec[( surf_index - 1 ) * num_main_surf]; // Translations for the specific symmetric copy

                vec3d cen( 0, 0, 0 );
                vec3d rotdir( -1, 0, 0 );

                // Identify if the normal vector is flipped and use it to flip the sign of RPM but calculate 
                // the normal vector from the transformation matrix (which will consider symmetry)
                // Note inverse of GetFlipNormal is used because Props are flipped by default
                // see (m_XSecSurf.GetFlipUD() in UpdateSurf() in PropGeom)
                m_ReverseFlag = !geom->GetFlipNormal( ( surf_index - 1 ) * num_main_surf );

                o_vec = trans_mat.xform( cen );
                r_vec = trans_mat.xform( rotdir ) - o_vec;

                rotor_dia = prop->m_Diameter.Get();

                // Set group name
                m_Name = prop->GetName();
            }
        }
    }

    if ( is_rotor )
    {
        m_GeomPropertyType.Set( GEOM_ROTOR );
    }
    else
    {
        m_Name = "Fixed_Group";
        m_RPM.Set( 0 );
    }

    m_RotorDia.Set( rotor_dia );

    m_Ox.Set( o_vec.x() );
    m_Oy.Set( o_vec.y() );
    m_Oz.Set( o_vec.z() );

    r_vec.normalize();

    m_Rx.Set( r_vec.x() );
    m_Ry.Set( r_vec.y() );
    m_Rz.Set( r_vec.z() );
}

int UnsteadyGroup::WriteGroup( FILE *group_file )
{
    if ( !group_file )
    {
        fprintf( stderr, "ERROR: Failed to open *.group file \n\tFile: %s \tLine:%d\n", __FILE__, __LINE__ );
        return vsp::VSP_FILE_WRITE_FAILURE;
    }

    fprintf( group_file, "#\n" );

    // Remove spaces from name
    string name = m_Name;
    name.erase( remove_if( name.begin(), name.end(), ::isspace ), name.end() );

    fprintf( group_file, "GroupName = %s\n", name.c_str() );

    // 'Old' way of setting up number of components.
    // fprintf( group_file, "NumberOfComponents = %d\n", m_ComponentVSPAEROIndexVec.size() );
    // for ( size_t i = 0; i < m_ComponentVSPAEROIndexVec.size(); i++ )
    // {
    //     fprintf( group_file, "%d\n", m_ComponentVSPAEROIndexVec[i] );
    // }

    fprintf( group_file, "NumberOfComponents = %d\n", m_GeomIDsInGroup.size() );
    for ( size_t i = 0; i < m_GeomIDsInGroup.size(); i++ )
    {
        int gnum = SubSurfaceMgr.FindGNum( m_GeomIDsInGroup[i] );
        fprintf( group_file, "%d\n", gnum + 1 );
    }

    bool geom_fixed = false;
    bool geom_dynamic = false;
    bool geom_rotor = false;

    if ( m_GeomPropertyType() == GEOM_FIXED )
    {
        geom_fixed = true;
    }
    else if ( m_GeomPropertyType() == GEOM_DYNAMIC )
    {
        geom_dynamic = true;
    }
    else if ( m_GeomPropertyType() == GEOM_ROTOR )
    {
        geom_rotor = true;
    }

    double omega = m_RPM() * PI / 30;
    if ( m_ReverseFlag )
    {
        omega *= -1;
    }

    fprintf( group_file, "GeometryIsFixed = %d\n", geom_fixed );
    fprintf( group_file, "GeometryIsDynamic = %d\n", geom_dynamic );
    fprintf( group_file, "GeometryIsARotor = %d\n", geom_rotor );
    fprintf( group_file, "RotorDiameter = %lf\n", m_RotorDia() );
    fprintf( group_file, "OVec = %lf %lf %lf\n", m_Ox(), m_Oy(), m_Oz() );
    fprintf( group_file, "RVec = %lf %lf %lf\n", m_Rx(), m_Ry(), m_Rz() );
    fprintf( group_file, "Velocity = %lf %lf %lf\n", m_Vx(), m_Vy(), m_Vz() );
    fprintf( group_file, "Acceleration = %lf %lf %lf\n", m_Ax(), m_Ay(), m_Az() );
    fprintf( group_file, "Omega = %lf\n", omega );
    fprintf( group_file, "Mass = %lf\n", m_Mass() );
    fprintf( group_file, "Ixx = %lf\n", m_Ixx() );
    fprintf( group_file, "Iyy = %lf\n", m_Iyy() );
    fprintf( group_file, "Izz = %lf\n", m_Izz() );
    fprintf( group_file, "Ixy = %lf\n", m_Ixy() );
    fprintf( group_file, "Ixz = %lf\n", m_Ixz() );
    fprintf( group_file, "Iyz = %lf\n", m_Iyz() );

    return vsp::VSP_OK;
}
