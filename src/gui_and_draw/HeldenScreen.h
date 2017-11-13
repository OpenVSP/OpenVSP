//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//
//
//////////////////////////////////////////////////////////////////////

#ifndef HELDENSCREEN_H
#define HELDENSCREEN_H

#include "ScreenBase.h"
#include "ScreenMgr.h"
#include "GuiDevice.h"
#include "ProcessUtil.h"
#include "HeldenMgr.h"

using namespace std;

#define HELDEN_SURF 0
#define HELDEN_PATCH 1
#define HELDEN_MESH 2

class HeldenScreen;
typedef std::pair< HeldenScreen*, int > heldenmonitorpair;
typedef std::pair< HeldenMgrSingleton*, HeldenScreen* > heldenpair;

class HeldenScreen : public TabScreen
{
public:
    HeldenScreen( ScreenMgr * mgr );
    virtual ~HeldenScreen();

    void Show();
    void Hide();
    bool Update();

    virtual void GuiDeviceCallBack( GuiDevice* device );

    virtual void CallBack( Fl_Widget* w );
    virtual void CloseCallBack( Fl_Widget *w );
    static void staticScreenCB( Fl_Widget *w, void* data )
    {
        ( ( HeldenScreen* )data )->CallBack( w );
    }

    void LoadSetChoice( Choice & c, int & index );

    void AddOutputText( Fl_Text_Display *display, const string &text );

    ProcessUtil *GetProcess( int id );
    Fl_Text_Display *GetDisplay( );

    // Solver thread kill flags (these must be public because they are accessed by the thread)
    bool m_HSurfThreadIsRunning;
    bool m_HPatchThreadIsRunning;
    bool m_HMeshThreadIsRunning;

protected:

    GroupLayout m_MainLayout;

    GroupLayout m_OptionsLayout;

    GroupLayout m_FirstColLayout;
    GroupLayout m_SecondColLayout;

    Choice m_SelectedSetChoice;

    StringOutput m_IGESFileOutput;
    TriggerButton m_IGESFileSelect;

    Choice m_LenUnitChoice;
    ToggleButton m_SplitSurfsToggle;
    ToggleButton m_SplitSubSurfsToggle;
    ToggleButton m_ToCubicToggle;
    SliderInput m_ToCubicTolSlider;
    ToggleButton m_TrimTEToggle;


    Fl_Text_Display *m_ConsoleDisplay;
    Fl_Text_Buffer *m_ConsoleBuffer;

    // Execute Layout
    TriggerButton m_InitHSurfButton;
    TriggerButton m_HSurfButton;
    TriggerButton m_KillHSurfButton;

    TriggerButton m_InitHPatchButton;
    TriggerButton m_HPatchButton;
    TriggerButton m_KillHPatchButton;

    TriggerButton m_InitHMeshButton;
    TriggerButton m_HMeshButton;
    TriggerButton m_KillHMeshButton;

    GroupLayout m_SurfFileLayout;

    Fl_Text_Editor* m_SurfFileEditor;
    Fl_Text_Buffer* m_SurfFileBuffer;

    TriggerButton m_SaveSurfFile;
    TriggerButton m_ReadSurfFile;

    GroupLayout m_PatchFileLayout;

    Fl_Text_Editor* m_PatchFileEditor;
    Fl_Text_Buffer* m_PatchFileBuffer;

    TriggerButton m_SavePatchFile;
    TriggerButton m_ReadPatchFile;

    GroupLayout m_MeshFileLayout;

    Fl_Text_Editor* m_MeshFileEditor;
    Fl_Text_Buffer* m_MeshFileBuffer;

    TriggerButton m_SaveMeshFile;
    TriggerButton m_ReadMeshFile;

protected:

    // Additional thread handling stuff
    ProcessUtil m_HPatchProcess;
    ProcessUtil m_HSurfProcess;
    ProcessUtil m_HMeshProcess;

    heldenpair m_HPatchPair;
    heldenpair m_HSurfPair;
    heldenpair m_HMeshPair;

    ProcessUtil m_HPatchMonitor;
    ProcessUtil m_HSurfMonitor;
    ProcessUtil m_HMeshMonitor;


};
#endif //HELDENSCREEN_H
