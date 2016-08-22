//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//

// ManageGeomScreen: Create/Delete Geom Screen
// J.R Gloudemans
//
//////////////////////////////////////////////////////////////////////

#if !defined(VSPMANAGEGEOMSCREEN__INCLUDED_)
#define VSPMANAGEGEOMSCREEN__INCLUDED_

#include "ScreenBase.h"
#include "GuiDevice.h"

#include <FL/Fl.H>
#include "PodScreen.h"
#include "FuselageScreen.h"
#include "WingScreen.h"
#include "BlankScreen.h"
#include "MeshScreen.h"
#include "StackScreen.h"
#include "CustomScreen.h"
#include "PtCloudScreen.h"
#include "PropScreen.h"
#include "HingeScreen.h"
#include "DrawObj.h"
#include "MultTransScreen.h"

using std::string;
using std::vector;

class ManageGeomScreen : public BasicScreen
{
public:
    ManageGeomScreen( ScreenMgr* mgr );
    virtual ~ManageGeomScreen();

    enum { POD_GEOM_SCREEN, FUSELAGE_GEOM_SCREEN, MS_WING_GEOM_SCREEN, BLANK_GEOM_SCREEN,
           MESH_GEOM_SCREEN, STACK_GEOM_SCREEN, CUSTOM_GEOM_SCREEN, PT_CLOUD_GEOM_SCREEN,
           PROP_GEOM_SCREEN, HINGE_GEOM_SCREEN, MULT_GEOM_SCREEN, NUM_GEOM_SCREENS
         };

    void Show();
    void Hide();
    bool Update();

    void CallBack( Fl_Widget *w );
    static void staticScreenCB( Fl_Widget *w, void* data )
    {
        ( ( ManageGeomScreen* )data )->CallBack( w );
    }

    virtual void GuiDeviceCallBack( GuiDevice* device );

    void CreateScreens();
    void ShowHideGeomScreens();

    void UpdateGeomScreens();

    /*
    * Get Feedback Group's name.
    */
    virtual std::string getFeedbackGroupName();

    /*!
    * Set current geom to geom with specific ID.
    */
    void Set(std::string geomID);

    /*!
    * Push Pick button once.
    */
    void TriggerPickSwitch();

    void LoadDrawObjs( vector< DrawObj* > & draw_obj_vec );

protected:

    GroupLayout m_MainLayout;

    GroupLayout m_HeadLayout;

    Choice m_GeomTypeChoice;
    TriggerButton m_AddGeomButton;
    StringInput m_ActiveGeomInput;

    GroupLayout m_BodyLayout;

    GroupLayout m_LeftLayout;
    GroupLayout m_MidLayout;
    GroupLayout m_RightLayout;

    TriggerButton m_MoveTopButton;
    TriggerButton m_MoveUpButton;
    TriggerButton m_MoveDownButton;
    TriggerButton m_MoveBotButton;

    Fl_Browser * m_GeomBrowser;

    TriggerButton m_DeleteButton;
    TriggerButton m_CopyButton;
    TriggerButton m_PasteButton;
    TriggerButton m_CutButton;

    TriggerButton m_SelectAllButton;
    ToggleButton m_PickButton;
    TriggerButton m_ShowButton;
    TriggerButton m_ShowOnlyButton;
    TriggerButton m_NoShowButton;

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
    int m_TypeIndex;
    bool m_CollapseFlag;
    string m_LastSelectedGeomID;
    Vehicle* m_VehiclePtr;
    vector< VspScreen* > m_GeomScreenVec;

    vector< string > m_DisplayedGeomVec;

    std::vector<DrawObj> m_PickList;

    void AddGeom();
    void LoadBrowser();
    void LoadActiveGeomOutput();
    void LoadSetChoice();
    void LoadTypeChoice();
    void UpdateDrawType();
    void GeomBrowserCallback();
    void SelectGeomBrowser( string geom_id );
    bool IsParentSelected( string geom_id, vector< string > & selVec );
    void NoShowActiveGeoms( bool flag );
    void SelectAll();
    void SelectSet( int set );
    void SetGeomDisplayType( int type );
    void EditName( string name );
    void SetSubDrawFlag( bool f );
    void SetFeatureDrawFlag( bool f );

    vector< string > GetActiveGeoms();

    vector< string > GetSelectedBrowserItems();

    void UpdateDrawObjs();
    
};


#endif // !defined(MAINSCREEN__INCLUDED_)
