//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//

// ManageGeomScreen: Create/Delete Geom Screen
// J.R Gloudemans
//
//////////////////////////////////////////////////////////////////////

#if !defined(VSPNERFMANAGEGEOMSCREEN__INCLUDED_)
#define VSPNERFMANAGEGEOMSCREEN__INCLUDED_

#include "ScreenBase.h"
#include "GuiDevice.h"

#include <FL/Fl.H>
#include "BlankScreen.h"
#include "BORScreen.h"
#include "ConformalScreen.h"
#include "CustomScreen.h"
#include "DrawObj.h"
#include "EllipsoidScreen.h"
#include "FuselageScreen.h"
#include "HingeScreen.h"
#include "HumanGeomScreen.h"
#include "MeshScreen.h"
#include "MultTransScreen.h"
#include "PodScreen.h"
#include "PropScreen.h"
#include "PtCloudScreen.h"
#include "StackScreen.h"
#include "TreeIconWidget.h"
#include "WingScreen.h"
#include "WireScreen.h"

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
    virtual void GetCollIDs( vector < string > &collIDVec );

    void CallBack( Fl_Widget *w );
    void CloseCallBack( Fl_Widget *w );
    static void staticScreenCB( Fl_Widget *w, void* data )
    {
        ( ( NerfManageGeomScreen* )data )->CallBack( w );
    }

    virtual void GuiDeviceCallBack( GuiDevice* device );

    void ShowHideGeomScreens();

    void SetNeedsShowHideGeoms()
    {
        m_NeedsShowHideGeoms = true;
    }

    void SetRedrawFlag()
    {
        m_RedrawFlag = true;
    }
    void ClearRedrawFlag()
    {
        m_RedrawFlag = false;
    }

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

    GroupLayout m_MidLayout;
    GroupLayout m_RightLayout;

    TreeWithIcons* m_GeomBrowser;

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

    int m_SetIndex;
    Vehicle* m_VehiclePtr;

    std::vector<DrawObj> m_PickList;

    bool m_VehSelected;
    bool m_VehOpen;
    bool m_RedrawFlag;
    bool m_NeedsShowHideGeoms;

    vector < string > m_SelVec;

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
