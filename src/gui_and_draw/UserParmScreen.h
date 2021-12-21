//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//

// Screen and edit user parms
// J.R Gloudemans
//
//////////////////////////////////////////////////////////////////////

#if !defined(VSPTYPEUSERPARMSCREEN__INCLUDED_)
#define VSPTYPEUSERPARMSCREEN__INCLUDED_

#include "ScreenBase.h"
#include "GuiDevice.h"

#include <FL/Fl.H>

using std::string;
using std::vector;

class UserParmScreen : public TabScreen
{
public:
    UserParmScreen( ScreenMgr* mgr );
    virtual ~UserParmScreen()                         {}
    void Show();
    void Hide();
    bool Update();

    void CallBack( Fl_Widget *w );
    static void staticScreenCB( Fl_Widget *w, void* data )
    {
        ( ( UserParmScreen* )data )->CallBack( w );
    }

    void GuiDeviceCallBack( GuiDevice* gui_device );

protected:

    GroupLayout m_PredefGroup;
    GroupLayout m_CreateGroup;
    Choice m_ParmTypeChoice;
    StringInput m_ParmNameInput;
    StringInput m_ParmGroupInput;
    StringInput m_ParmDescInput;
    Input m_ParmValueInput;
    Input m_ParmMinInput;
    Input m_ParmMaxInput;
    TriggerButton m_CreateParm;
    TriggerButton m_DeleteParm;
    TriggerButton m_DeleteAllParm;

    StringInput m_EditParmNameInput;
    StringInput m_EditParmGroupInput;
    StringInput m_EditParmDescInput;
    StringInput m_EditParmMinInput;
    StringInput m_EditParmMaxInput;

    Fl_Browser* m_UserDefinedBrowser;

    GroupLayout m_AdjustLayout;
    Fl_Scroll* m_AdjustScroll;

    vector< SliderAdjRangeInput > m_PredefSliderVec;
    vector < SliderAdjRangeInput > m_ParmSliderVec;
 
    vector< string > m_UserParmBrowserVec;

    string m_NameText;
    string m_GroupText;
    string m_DescText;

    int m_UserBrowserSelection;

    int m_NumParmsLast;
    void RebuildAdjustGroup();

};


#endif
