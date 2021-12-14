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

    void LoadSetChoice( Choice & choice, int selectedindex );

protected:

    int m_SelectedSetIndex;
    int m_DegenSelectedSetIndex;

    GroupLayout m_MainLayout;
    GroupLayout m_BorderLayout;

    ToggleButton m_CsvToggle;

    StringOutput m_TxtOutput;
    StringOutput m_CsvOutput;

    TriggerButton m_TxtSelect;
    TriggerButton m_CsvSelect;

    Choice m_UseSet;
    Choice m_DegenSet;

    ToggleButton m_HalfMesh;
    ToggleButton m_Subsurfs;

    TriggerButton m_Execute;

    Fl_Text_Display* m_TextDisplay;
    Fl_Text_Buffer* m_TextBuffer;

};
#endif //COMPGEOMSCREEN_H
