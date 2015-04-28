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

    GroupLayout m_OverviewLayout;

    GroupLayout m_GeomLayout;
    GroupLayout m_FlowLayout;
    GroupLayout m_RefLayout;
    GroupLayout m_CGLayout;


    Choice m_GeomSetChoice;
    TriggerButton m_DegenGeomButton;
    StringOutput m_DegenFileName;
    TriggerButton m_DegenFileButton;

    SliderAdjRangeInput m_AlphaSlider;
    SliderAdjRangeInput m_BetaSlider;
    SliderAdjRangeInput m_MachSlider;


    Choice m_RefWingChoice;
    map <string, int> m_WingCompIDMap;
    vector <string> m_WingGeomVec;

    ToggleButton m_RefManualToggle;
    ToggleButton m_RefChoiceToggle;
    ToggleRadioGroup m_RefToggle;
    SliderAdjRangeInput m_SrefSlider;
    SliderAdjRangeInput m_brefSlider;
    SliderAdjRangeInput m_crefSlider;


    Choice m_CGSetChoice;
    TriggerButton m_MassPropButton;
    SliderInput m_NumSliceSlider;
    SliderAdjRangeInput m_XcgSlider;
    SliderAdjRangeInput m_YcgSlider;
    SliderAdjRangeInput m_ZcgSlider;



    TriggerButton m_SetupButton;
    TriggerButton m_SolverButton;
    TriggerButton m_ViewerButton;


    SliderInput m_NCPUSlider;

    GroupLayout m_SetupLayout;

    Fl_Text_Editor* m_SetupEditor;
    Fl_Text_Buffer* m_SetupBuffer;

    TriggerButton m_SaveSetup;
    TriggerButton m_ReadSetup;


    GroupLayout m_SolverLayout;

    TriggerButton m_KillSolverButton;
    Fl_Text_Display *m_SolverDisplay;
    Fl_Text_Buffer *m_SolverBuffer;

    GroupLayout m_ViewerLayout;

    Fl_Text_Display *m_ViewerDisplay;
    Fl_Text_Buffer *m_ViewerBuffer;

    string m_DegenFile;

    void SetupDegenFile();

    ProcessUtil m_SolverProcess;
    ProcessUtil m_ViewerProcess;

    monitorpair m_SolverPair;
    monitorpair m_ViewerPair;

    ProcessUtil m_SolverMonitor;
    ProcessUtil m_ViewerMonitor;
};

#endif  // VSPAEROSCREEN_H
