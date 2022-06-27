//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//

// VSPAEROScreen.h: interface for launching VSPAERO.
//
//////////////////////////////////////////////////////////////////////

#ifndef VSPAEROSCREEN_H
#define VSPAEROSCREEN_H

#include "ScreenMgr.h"
#include "ScreenBase.h"
#include "Vehicle.h"
#include "GuiDevice.h"
#include "ProcessUtil.h"
#include "VSPAEROMgr.h"

#include "SelectFileScreen.h"

#include <FL/Fl_Text_Buffer.H>
#include <FL/Fl_Text_Display.H>

using namespace std;

#define VSPAERO_SOLVER 0
#define VSPAERO_VIEWER 1
#define VSPAERO_SOLVER_MONITOR 2

class VSPAEROScreen;
typedef std::pair< VSPAEROScreen*, int > monitorpair;
typedef std::pair< VSPAEROMgrSingleton*, VSPAEROScreen* > solverpair;

class VSPAEROScreen : public TabScreen
{
public:
    VSPAEROScreen( ScreenMgr* mgr );
    virtual ~VSPAEROScreen();

    void Show();
    void Hide();
    bool Update();

    void CallBack( Fl_Widget *w );
    void CloseCallBack( Fl_Widget* w );
    static void staticScreenCB( Fl_Widget *w, void* data )
    {
        ( ( VSPAEROScreen* )data )->CallBack( w );
    }

    virtual void GuiDeviceCallBack( GuiDevice* device );

    void PropElemBrowserCallback();
    void ControlSurfaceGroupBrowserCallback();
    void UngroupedCSBrowserCallback();
    void GroupedCSBrowserCallback();
    void CpSliceBrowserCallback();
    void UnsteadyGroupBrowserCallback();

    void SelectPropBrowser( int cur_index );
    void SelectControlSurfaceBrowser( int cur_index );
    void SelectUngroupedListBrowser( int cur_index );
    void SelectGroupedListBrowser( int cur_index );
    void SelectCpSliceBrowser( int cur_index );
    void SelectUnsteadyGroupBrowser( int cur_index );

    static void AddOutputText( Fl_Text_Display *display, const string &text );

    ProcessUtil *GetProcess( int id );
    Fl_Text_Display *GetDisplay( int id );

    // Solver thread kill flags (these must be public because they are accessed by the thread)
    bool m_SolverThreadIsRunning;

    void UpdateRefWing();
    void UpdateSetChoiceLists();
    void UpdateCaseSetupDevices();
    void UpdateReferenceQuantitiesDevices();
    void UpdateCGDevices();
    void UpdateAdvancedTabDevices();
    void UpdateFlowConditionDevices();
    void UpdateVSPAEROButtons();
    void UpdatePropElemDevices();
    void UpdatePropElemBrowser();
    void UpdateControlSurfaceBrowsers();
    void UpdateOtherSetupParms();
    void UpdateDeflectionAngleScrollGroup();
    void UpdateControlSurfaceGroupNames();
    void UpdateDeflectionGainScrollGroup();
    void UpdateCpSlices();
    void UpdateCpSliceBrowser();
    void UpdateUnsteadyGroups();
    void UpdateUnsteadyGroupBrowser();

    void DisplayDegenVLMPreview();

    void LoadDrawObjs( vector < DrawObj* > & draw_obj_vec );

protected:

    int m_NumVarAngle;
    int m_NumVarDeflection;

    //==== Constant Console Area ====//
    GroupLayout m_ConstantAreaLayout;
    GroupLayout m_ConsoleLayout;
    GroupLayout m_ExecuteLayout;

    //==== Overview Tab ====//
    GroupLayout m_OverviewLayout;

    GroupLayout m_LeftColumnLayout;
    GroupLayout m_RightColumnLayout;

    GroupLayout m_CaseSetupLayout;
    GroupLayout m_RefLengthLayout;
    GroupLayout m_MomentRefLayout;
    GroupLayout m_FlowCondLayout;
    GroupLayout m_ExecuteOptionsLayout;

    // Case Setup Layout
    //  Radio buttons for analysis method selection VLM OR Panel
    ToggleButton m_AeroMethodToggleVLM;
    ToggleButton m_AeroMethodTogglePanel;
    ToggleRadioGroup m_AeroMethodToggleGroup;
    Choice m_GeomSetChoice;
    // Additional options
    TriggerButton m_PreviewDegenButton;

    // Ref Length Layout
    // Reference lengths & areas
    vector <string> m_WingGeomVec;
    Choice m_RefWingChoice;
    ToggleButton m_RefManualToggle;
    ToggleButton m_RefChoiceToggle;
    ToggleRadioGroup m_RefToggle;
    SliderAdjRangeInput m_SrefSlider;
    SliderAdjRangeInput m_brefSlider;
    SliderAdjRangeInput m_crefSlider;

    // Moment Ref Layout
    // Moment reference location
    Choice m_CGSetChoice;
    TriggerButton m_MassPropButton;
    SliderAdjRangeInput m_NumSliceSlider;
    SliderAdjRangeInput m_XcgSlider;
    SliderAdjRangeInput m_YcgSlider;
    SliderAdjRangeInput m_ZcgSlider;

    // Flow Condition Layout
    // Freestream settings
    Input m_AlphaStartInput, m_AlphaEndInput, m_AlphaNptsInput;
    Input m_BetaStartInput, m_BetaEndInput, m_BetaNptsInput;
    Input m_MachStartInput, m_MachEndInput, m_MachNptsInput;
    Input m_ReCrefStartInput, m_ReCrefEndInput, m_ReCrefNptsInput;

    // Execute Layout
    TriggerButton m_SolverButton;
    TriggerButton m_KillSolverButton;
    TriggerButton m_PlotButton;
    TriggerButton m_ViewerButton;
    TriggerButton m_ExportResultsToCsvButton;

    //==== Advanced Tab ====//
    GroupLayout m_AdvancedLayout;
    GroupLayout m_AdvancedLeftLayout;
    GroupLayout m_AdvancedRightLayout;
    GroupLayout m_AdvancedCaseSetupLayout;
    GroupLayout m_WakeLayout;
    GroupLayout m_OtherParmsLayout;
    GroupLayout m_PropAndStabLayout;
    GroupLayout m_AdvancedFlowLayout;
    GroupLayout m_CpSlicerLayout;
    GroupLayout m_CpSlicerSubLayout;

    // Advanced Case Setup Layout
    //  Degengeom (VLM & Panel)
    StringOutput m_DegenFileName;
    TriggerButton m_DegenFileButton;
    //  CompGeom (Panel only)
    StringOutput m_CompGeomFileName;
    TriggerButton m_CompGeomFileButton;
    SliderAdjRangeInput m_NCPUSlider;
    ToggleButton m_BatchCalculationToggle;
    ToggleButton m_SymmetryToggle;
    ToggleButton m_Write2DFEMToggle;
    ToggleButton m_EnableAlternateFormat;
    TriggerButton m_LoadExistingResultsButton;
    Choice m_PreconditionChoice;
    ToggleButton m_KTCorrectionToggle;
    ToggleButton m_FromSteadyStateToggle;

    // Wake calculation options
    ToggleButton m_FixedWakeToggle;
    SliderAdjRangeInput m_WakeNumIterSlider;
    SliderAdjRangeInput m_NumWakeNodeSlider;

    // Other Setup Parms Setup
    Choice m_ClmaxChoice;
    ToggleButton m_MaxTurningToggle;
    ToggleButton m_FarDistToggle;
    ToggleButton m_GroundEffectToggle;
    SliderAdjRangeInput m_ClmaxSlider;
    SliderAdjRangeInput m_MaxTurningSlider;
    SliderAdjRangeInput m_FarDistSlider;
    SliderAdjRangeInput m_GroundEffectSlider;

    ToggleButton m_ActuatorDiskToggle;
    ToggleButton m_RotateBladesToggle;

    // Stability Setup
    Choice m_StabilityTypeChoice;

    // Cp Slice Setup
    ToggleButton m_CpSlicerActivateToggle;
    TriggerButton m_CpSliceLastADBButton;
    ColResizeBrowser* m_CpSliceBrowser;
    TriggerButton m_AddCpSliceButton;
    TriggerButton m_DeleteCpSliceButton;
    TriggerButton m_DeleteAllCpSliceButton;
    Choice m_CpSliceTypeChoice;
    SliderAdjRangeInput m_CpSliceLocation;
    StringInput m_CpSliceNameInput;
    ToggleButton m_CpSliceShowToggle;

    //==== Rotor Disk Tab ====//
    Fl_Group* m_RotorDiskTab;
    GroupLayout m_PropGeneralLayout;
    GroupLayout m_PropElemLayout;
    GroupLayout m_DeflectionAngleLayout;

    // Prop General Layout
    SliderAdjRangeInput m_VinfSlider;
    SliderAdjRangeInput m_RhoSlider;
    SliderAdjRangeInput m_MachRefSlider;
    SliderAdjRangeInput m_VRefSlider;
    ToggleButton m_ActivateVRefToggle;

    // Prop Elem Layout
    ColResizeBrowser* m_PropElemBrowser;
    StringOutput m_PropElemDia;
    ToggleButton m_PropAutoHubDia;
    SliderAdjRangeInput m_PropElemHubDia;
    SliderAdjRangeInput m_PropElemRPM;
    SliderAdjRangeInput m_PropElemCT;
    SliderAdjRangeInput m_PropElemCP;

    // Deflection Angle Layout
    Fl_Scroll* m_DeflectionAngleScroll;

    vector < ToggleButton > m_DeflectionAngleToggleVec;
    vector < SliderAdjRangeInput > m_DeflectionAngleSliderVec;

    //==== Control Surface Grouping Tab ====//
    GroupLayout m_ControlSurfaceLayout;
    GroupLayout m_CSGroupingLayout;
    GroupLayout m_CSGroupDetailsLayout;
    GroupLayout m_CSGroupGainScrollLayout;
    Fl_Browser* m_CSGroupBrowser;
    Fl_Browser* m_UngroupedCSBrowser;
    Fl_Browser* m_GroupedCSBrowser;

    Fl_Scroll* m_DeflectionGainScroll;
    vector < SliderAdjRangeInput > m_DeflectionGainSliderVec;

    TriggerButton m_AutoGroupTrigger;

    StringInput m_GroupEditNameInput;

    StringInput m_CSGroupInput;
    TriggerButton m_AddCSGroupButton;
    TriggerButton m_RemoveCSGroupButton;

    TriggerButton m_AddSelectedCSButton;
    TriggerButton m_AddAllCSButton;
    TriggerButton m_RemoveSelectedCSButton;
    TriggerButton m_RemoveAllCSButton;

    SliderAdjRangeInput m_DeflectionGainSlider;

    ToggleButton m_HoverRampToggle;
    SliderAdjRangeInput m_HoverRampSlider;

    ToggleButton m_NoiseCalcToggle;
    TriggerButton m_NoiseCalcTrigger;
    Choice m_NoiseCalcChoice;
    Choice m_NoiseUnitChoice;

    //==== Solver Tab ====//
    GroupLayout m_SolverLayout;

    Fl_Text_Display *m_SolverDisplay;
    Fl_Text_Buffer *m_SolverBuffer;

    //====  Propeller Tab ====//
    Fl_Group* m_PropellerTab;

    // Unsteady Setup
    ToggleButton m_AutoTimeStepToggle;
    SliderAdjRangeInput m_AutoTimeNumRevSlider;
    SliderAdjRangeInput m_TimeStepSizeSlider;
    SliderAdjRangeInput m_NumTimeStepSlider;

    GroupLayout m_UnsteadyGroupLayout;
    GroupLayout m_UnsteadyGroupLeftLayout;
    GroupLayout m_UnsteadyGroupRightLayout;

    ColResizeBrowser* m_UnsteadyGroupBrowser;
    SliderAdjRangeInput m_RPMSlider;
    ToggleButton m_UniformRPMToggle;

    //==== Viewer Tab ====//
    GroupLayout m_ViewerLayout;

    Fl_Text_Display *m_ViewerDisplay;
    Fl_Text_Buffer *m_ViewerBuffer;


    // Additional thread handling stuff
    ProcessUtil m_SolverProcess;
    ProcessUtil m_ViewerProcess;

    solverpair m_SolverPair;
    monitorpair m_ViewerPair;

    ProcessUtil m_SolverMonitor;
    ProcessUtil m_ViewerMonitor;
};

#endif  // VSPAEROSCREEN_H
