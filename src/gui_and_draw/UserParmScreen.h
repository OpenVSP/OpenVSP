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
#include "userParmFlScreen.h"

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

    UserParmUI* m_UserParmUI;

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

    Fl_Browser* m_UserDefinedBrowser;

    GroupLayout m_EditLayout;

    GroupLayout m_AdjustLayout;
    Fl_Scroll* m_AdjustScroll;

    int m_NumEditSliders;

    vector< SliderAdjRangeInput > m_PredefSliderVec;
    vector < SliderAdjRangeInput > m_ParmSliderVec;
 
    vector< string > m_UserParmBrowserVec;
    deque< string > m_EditParmVec;

    string m_NameText;
    string m_GroupText;
    string m_DescText;
    Parm m_Val;
    Parm m_Min;
    Parm m_Max;

    int m_NumParmsLast;
    void RebuildAdjustGroup();

};


#endif
