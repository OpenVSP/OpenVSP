//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//

// VehNotesScreen.h: GUI Screen for displaying Vehicle Notes attribute only- with bool parm for autoshow setting
// Tim Cuatt
//
//////////////////////////////////////////////////////////////////////

#if !defined(VSP_NOTESCREEN_INCLUDED_)
#define VSP_NOTESCREEN_INCLUDED_

#include "ScreenBase.h"
#include "GuiDevice.h"
#include "AttributeEditor.h"
#include "ScreenMgr.h"
#include "ResultsMgr.h"

#include <FL/Fl.H>

using std::string;
using std::vector;

class VehNotesScreen: public BasicScreen
{
public:
    VehNotesScreen( ScreenMgr* mgr );
    virtual ~VehNotesScreen();

    bool Update( );

    virtual void Show();

    virtual void CallBack( Fl_Widget *w );
    void GuiDeviceCallBack( GuiDevice* gui_device );
    static void staticScreenCB( Fl_Widget *w, void* data )
    {
        ( ( VehNotesScreen* )data )->CallBack( w );
    }

protected:

    ScreenMgr* m_ScreenMgr;

    GroupLayout m_NotesScreenLayout;
    GroupLayout m_NotesLayout;
    GroupLayout m_ToggleLayout;
    GroupLayout m_ResizableLayout;

    Fl_Box* m_DataLabel;

    BoolParm* m_ShowBoolParmPtr;
    StringOutput m_ShowToggleLabel;
    ToggleButton m_ShowDataToggleIn;
    StringOutput m_ShowToggleField;

    VspTextEditor* m_DataText;
    Fl_Text_Buffer* m_DataBuffer;

};

#endif