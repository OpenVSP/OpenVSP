//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//
// A check list of the Results a Results plot curve plots when its runs are chosen.
//
//////////////////////////////////////////////////////////////////////

#ifndef VSP_TOP_RESULTSCHOOSERSCREEN_H
#define VSP_TOP_RESULTSCHOOSERSCREEN_H

#include "ScreenBase.h"

class ResultsCurve;

class ResultsChooserScreen : public BasicScreen
{
public:
    ResultsChooserScreen( ScreenMgr* mgr );
    virtual ~ResultsChooserScreen();

    void Show();
    bool Update();

    void CallBack( Fl_Widget *w );
    static void staticScreenCB( Fl_Widget *w, void* data )
    {
        ( ( ResultsChooserScreen* )data )->CallBack( w );
    }

    virtual void GuiDeviceCallBack( GuiDevice* device );

protected:

    // The curve being edited in the current plot, when its runs are chosen.
    ResultsCurve* GetChoosingCurve();

    GroupLayout m_MainLayout;
    GroupLayout m_BorderLayout;

    Fl_Box* m_CurveBox;
    Fl_Check_Browser* m_ResultsBrowser;
    TriggerButton m_CheckAllButton;
    TriggerButton m_CheckNoneButton;

    vector < string > m_ListedIDs;      // Browser lines
};

#endif //VSP_TOP_RESULTSCHOOSERSCREEN_H
