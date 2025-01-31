//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//

// NerfManageGeomScreen: Create/Delete Geom Screen
// J.R Gloudemans
//
//////////////////////////////////////////////////////////////////////

#if !defined(VSPNERFMANAGEGEOMSCREEN__INCLUDED_)
#define VSPNERFMANAGEGEOMSCREEN__INCLUDED_

#include "ScreenBase.h"
#include "GuiDevice.h"

#include <FL/Fl.H>
#include "PodScreen.h"
#include "FuselageScreen.h"
#include "WingScreen.h"
#include "BlankScreen.h"
#include "BORScreen.h"
#include "MeshScreen.h"
#include "StackScreen.h"
#include "CustomScreen.h"
#include "PtCloudScreen.h"
#include "PropScreen.h"
#include "HingeScreen.h"
#include "DrawObj.h"
#include "MultTransScreen.h"
#include "ConformalScreen.h"
#include "EllipsoidScreen.h"
#include "WireScreen.h"
#include "HumanGeomScreen.h"

using std::string;
using std::vector;

class NerfManageGeomScreen : public BasicScreen
{
public:
    NerfManageGeomScreen( ScreenMgr* mgr );
    virtual ~NerfManageGeomScreen();

    void Show();
    void Hide();
    bool Update();

    void CallBack( Fl_Widget *w );
    static void staticScreenCB( Fl_Widget *w, void* data )
    {
        ( ( NerfManageGeomScreen* )data )->CallBack( w );
    }

    virtual void GuiDeviceCallBack( GuiDevice* device );

    void ShowHideGeomScreens();

    void UpdateGeomScreens();

    /*
    * Get Feedback Group's name.
    */
    virtual std::string getFeedbackGroupName();

    /*!
    * Set current geom to geom with specific ID.
    */
    void Set( const std::string &geomID);

    /*!
    * Push Pick button once.
    */
    void TriggerPickSwitch();

    void LoadDrawObjs( vector< DrawObj* > & draw_obj_vec );

protected:

    GroupLayout m_MainLayout;

    GroupLayout m_HeadLayout;

    StringOutput m_ActiveGeomOutput;

    GroupLayout m_BodyLayout;

    GroupLayout m_LeftLayout;
    GroupLayout m_MidLayout;
    GroupLayout m_RightLayout;

    Fl_Browser * m_GeomBrowser;

    TriggerButton m_SelectAllButton;
    ToggleButton m_PickButton;
    TriggerButton m_ShowButton;
    TriggerButton m_ShowOnlyButton;
    TriggerButton m_NoShowButton;

    Choice m_DisplayChoice;

    TriggerButton m_WireGeomButton;
    TriggerButton m_HiddenGeomButton;
    TriggerButton m_ShadeGeomButton;
    TriggerButton m_TextureGeomButton;
    TriggerButton m_NoneGeomButton;

    ToggleButton m_ShowSubToggle;
    ToggleButton m_ShowFeatureToggle;

    Choice m_SetChoice;
    TriggerButton m_ShowOnlySetButton;
    TriggerButton m_ShowSetButton;
    TriggerButton m_NoShowSetButton;
    TriggerButton m_SelectSetButton;


    int m_LastTopLine;
    int m_SetIndex;
    bool m_CollapseFlag;
    string m_LastSelectedGeomID;
    Vehicle* m_VehiclePtr;

    vector< string > m_DisplayedGeomVec;

    std::vector<DrawObj> m_PickList;

    bool m_VehSelected;

    void LoadBrowser();
    void LoadActiveGeomOutput();
    void LoadDisplayChoice();
    void UpdateDrawType();
    void GeomBrowserCallback();
    void SelectGeomBrowser( const string &geom_id = "NONE" );
    bool IsParentSelected( const string &geom_id, const vector< string > & selVec );
    void NoShowActiveGeoms( bool flag );
    void SelectAll();
    void SelectSet( int set );
    void SetGeomDisplayChoice( int type );
    void SetGeomDisplayType( int type );
    void EditName( const string &name );
    void SetSubDrawFlag( bool f );
    void SetFeatureDrawFlag( bool f );

    vector< string > GetActiveGeoms();

    vector< string > GetSelectedBrowserItems();

    void UpdateDrawObjs();
    
};


#endif // !defined(VSPNERFMANAGEGEOMSCREEN__INCLUDED_)
