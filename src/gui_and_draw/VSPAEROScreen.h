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

#include <FL/Fl_Text_Buffer.H>
#include <FL/Fl_Text_Display.H>

using namespace std;

#define VSPAERO_SOLVER 0
#define VSPAERO_VIEWER 1

class VSPAEROScreen;
typedef std::pair< VSPAEROScreen*, int > monitorpair;

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

    void AddOutputText( Fl_Text_Display *display, const char *text );

    ProcessUtil *GetProcess( int id );
    Fl_Text_Display *GetDisplay( int id );

protected:

    void ReadSetup();
    void SaveSetup();

    //==== Overview Tab ====//
    GroupLayout m_OverviewLayout;
    
    // Case Setup
    GroupLayout m_GeomLayout;
    Choice m_GeomSetChoice;
    TriggerButton m_DegenGeomButton;
    StringOutput m_DegenFileName;
    TriggerButton m_DegenFileButton;
    SliderAdjRangeInput m_NCPUSlider;
    SliderAdjRangeInput m_WakeNumIterSlider;
    SliderAdjRangeInput m_WakeAvgStartIterSlider;
    SliderAdjRangeInput m_WakeSkipUntilIterSlider;
    ToggleButton m_StabilityCalcToggle;

    // Freestream settings
    GroupLayout m_FlowLayout;
    Input m_AlphaStartInput, m_AlphaEndInput, m_AlphaNptsInput;
    Input m_BetaStartInput, m_BetaEndInput, m_BetaNptsInput;
    Input m_MachStartInput, m_MachEndInput, m_MachNptsInput;
    
    // Reference lengths & areas
    GroupLayout m_RefLayout;
    Choice m_RefWingChoice;
    ToggleButton m_RefManualToggle;
    ToggleButton m_RefChoiceToggle;
    ToggleRadioGroup m_RefToggle;
    SliderAdjRangeInput m_SrefSlider;
    SliderAdjRangeInput m_brefSlider;
    SliderAdjRangeInput m_crefSlider;

    vector <string> m_WingGeomVec;

    // Moment reference location
    GroupLayout m_CGLayout;
    Choice m_CGSetChoice;
    TriggerButton m_MassPropButton;
    SliderInput m_NumSliceSlider;
    SliderAdjRangeInput m_XcgSlider;
    SliderAdjRangeInput m_YcgSlider;
    SliderAdjRangeInput m_ZcgSlider;

    // Execute
    TriggerButton m_SetupButton;
    TriggerButton m_SolverButton;
    TriggerButton m_ViewerButton;

    //==== Setup Tab ====//
    GroupLayout m_SetupLayout;

    Fl_Text_Editor* m_SetupEditor;
    Fl_Text_Buffer* m_SetupBuffer;

    TriggerButton m_SaveSetup;
    TriggerButton m_ReadSetup;


    //==== Solver Tab ====//
    GroupLayout m_SolverLayout;

    TriggerButton m_KillSolverButton;
    Fl_Text_Display *m_SolverDisplay;
    Fl_Text_Buffer *m_SolverBuffer;


    //==== Viewer Tab ====//
    GroupLayout m_ViewerLayout;

    Fl_Text_Display *m_ViewerDisplay;
    Fl_Text_Buffer *m_ViewerBuffer;



    // Additional thread handling stuff
    ProcessUtil m_SolverProcess;
    ProcessUtil m_ViewerProcess;

    monitorpair m_SolverPair;
    monitorpair m_ViewerPair;

    ProcessUtil m_SolverMonitor;
    ProcessUtil m_ViewerMonitor;
};

#endif  // VSPAEROSCREEN_H
