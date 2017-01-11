//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//
//
//////////////////////////////////////////////////////////////////////

#ifndef COMPGEOMSCREEN_H
#define COMPGEOMSCREEN_H

#include "ScreenBase.h"
#include "ScreenMgr.h"
#include "GuiDevice.h"

using namespace std;

class CompGeomScreen : public BasicScreen
{
public:
    CompGeomScreen( ScreenMgr * mgr );
    virtual ~CompGeomScreen();

    void Show();
    void Hide();
    bool Update();

    virtual void GuiDeviceCallBack( GuiDevice* device );

    virtual void CallBack( Fl_Widget* w );
    virtual void CloseCallBack( Fl_Widget *w );
    static void staticScreenCB( Fl_Widget *w, void* data )
    {
        ( ( CompGeomScreen* )data )->CallBack( w );
    }

    void LoadSetChoice();

protected:

    int m_SelectedSetIndex;

    GroupLayout m_MainLayout;
    GroupLayout m_BorderLayout;

    ToggleButton m_CsvToggle;
    ToggleButton m_TsvToggle;

    StringOutput m_TxtOutput;
    StringOutput m_CsvOutput;
    StringOutput m_TsvOutput;

    TriggerButton m_TxtSelect;
    TriggerButton m_CsvSelect;
    TriggerButton m_TsvSelect;

    Choice m_UseSet;

    ToggleButton m_HalfMesh;
    ToggleButton m_Subsurfs;

    TriggerButton m_Execute;

    Fl_Text_Display* m_TextDisplay;
    Fl_Text_Buffer* m_TextBuffer;

};
#endif //COMPGEOMSCREEN_H
