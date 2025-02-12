//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//

// Create Edit Interference Checks
// Rob McDonald
//
//////////////////////////////////////////////////////////////////////

#if !defined(VSPTYPEINTERFERENCESCREEN__INCLUDED_)
#define VSPTYPEINTERFERENCESCREEN__INCLUDED_

#include "ScreenBase.h"
#include "GuiDevice.h"
#include "InterferenceMgr.h"

#include <FL/Fl.H>
#include <FL/Fl_Text_Buffer.H>

using std::string;
using std::vector;

class InterferenceScreen : public BasicScreen
{
public:
    InterferenceScreen( ScreenMgr* mgr );
    virtual ~InterferenceScreen();
    void Show();
    void Hide();
    bool Update();
    void UpdateInterferenceCheckBrowser();
    void LoadDrawObjs( vector< DrawObj* > & draw_obj_vec );
    void MarkDOChanged();
    bool GetVisBndBox( BndBox &bbox );

    void CallBack( Fl_Widget *w );
    static void staticScreenCB( Fl_Widget *w, void* data )
    {
        ( static_cast <InterferenceScreen*> ( data ) )->CallBack( w );
    }

    void GuiDeviceCallBack( GuiDevice* gui_device );


protected:

    GroupLayout m_GenLayout;
    GroupLayout m_BorderLayout;
    GroupLayout m_ICaseLayout;

    ColResizeBrowser* m_InterferenceCheckBrowser;
    int m_InterferenceBrowserSelect;


    TriggerButton m_AddInterferenceCheck;
    TriggerButton m_DelInterferenceCheck;
    TriggerButton m_DelAllInterferenceChecks;

    TriggerButton m_EvaluateAllInterferenceChecks;

    StringInput m_ICNameInput;

    Choice m_InterferenceTypeChoice;

    ToggleRadioGroup m_PrimaryToggleGroup;
    ToggleButton m_PrimarySetToggle;
    Choice m_PrimarySetChoice;

    ToggleButton m_PrimaryModeToggle;
    Choice m_PrimaryModeChoice;
    vector < string > m_ModeIDs;

    ToggleButton m_PrimaryGeomToggle;
    GeomPicker m_PrimaryGeomPicker;
    vector < string > m_GeomIDs;


    ToggleRadioGroup m_SecondaryToggleGroup;
    ToggleButton m_SecondarySetToggle;
    Choice m_SecondarySetChoice;

    ToggleButton m_SecondaryGeomToggle;
    GeomPicker m_SecondaryGeomPicker;

    TriggerButton m_Evaluate;

    Output m_ResultOutput;

};


#endif
