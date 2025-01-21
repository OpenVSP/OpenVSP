//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//
//
//////////////////////////////////////////////////////////////////////

#ifndef DEGENGEOMSCREEN_H
#define DEGENGEOMSCREEN_H

#include "ScreenBase.h"
#include "ScreenMgr.h"
#include "GuiDevice.h"

using namespace std;

class DegenGeomScreen : public BasicScreen
{
public:
    DegenGeomScreen( ScreenMgr * mgr );
    virtual ~DegenGeomScreen();

    void Show();
    void Hide();
    bool Update();

    virtual void GuiDeviceCallBack( GuiDevice* device );

    virtual void CallBack( Fl_Widget* w );
    virtual void CloseCallBack( Fl_Widget *w );
    static void staticScreenCB( Fl_Widget *w, void* data )
    {
        ( ( DegenGeomScreen* )data )->CallBack( w );
    }

protected:

    int m_SelectedSetIndex;
    int m_SelectedModeChoice;
    vector < string > m_ModeIDs;

    GroupLayout m_MainLayout;
    GroupLayout m_BorderLayout;

    ToggleButton m_CsvToggle;
    ToggleButton m_MToggle;

    StringOutput m_CsvOutput;
    StringOutput m_MOutput;

    TriggerButton m_CsvSelect;
    TriggerButton m_MSelect;

    Fl_Text_Display* m_TextDisplay;
    Fl_Text_Buffer* m_TextBuffer;

    ToggleButton m_SetToggle;
    Choice m_UseSet;

    ToggleRadioGroup m_ModeSetToggleGroup;
    ToggleButton m_ModeToggle;
    Choice m_ModeChoice;

    TriggerButton m_Execute;

    Choice m_MeshTypeChoice;
    TriggerButton m_MakeDegenMeshGeom;
};
#endif //DEGENGEOMSCREEN_H
