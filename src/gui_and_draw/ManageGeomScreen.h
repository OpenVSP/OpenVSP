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
#include "geomBrowserScreen.h"
#include "PodScreen.h"
#include "FuselageScreen.h"
#include "WingScreen.h"
#include "BlankScreen.h"
#include "MeshScreen.h"
#include "StackScreen.h"

using std::string;
using std::vector;

class ManageGeomScreen : public VspScreen
{
public:
    ManageGeomScreen( ScreenMgr* mgr );
    virtual ~ManageGeomScreen();

    enum { POD_GEOM_SCREEN, FUSELAGE_GEOM_SCREEN, MS_WING_GEOM_SCREEN, BLANK_GEOM_SCREEN,
        MESH_GEOM_SCREEN, STACK_GEOM_SCREEN, NUM_GEOM_SCREENS };

    void Show();
    void Hide();
    bool Update();

    void CallBack( Fl_Widget *w );
    static void staticScreenCB( Fl_Widget *w, void* data )
    {
        ( ( ManageGeomScreen* )data )->CallBack( w );
    }

    void CreateScreens();
    void ShowHideGeomScreens();

protected:

    GeomUI* m_GeomUI;
    int m_LastTopLine;
    int m_SetIndex;
    int m_TypeIndex;
    bool m_CollapseFlag;
    string m_LastSelectedGeomID;
    Vehicle* m_VehiclePtr;
    vector< VspScreen* > m_GeomScreenVec;

    vector< string > m_DisplayedGeomVec;

    void AddGeom();
    void LoadBrowser();
    void LoadActiveGeomOutput();
    void LoadSetChoice();
    void LoadTypeChoice();
    void GeomBrowserCallback();
    void SelectGeomBrowser( string geom_id );
    bool IsParentSelected( string geom_id, vector< string > & selVec );
    void NoShowActiveGeoms( bool flag );
    void SelectAll();
    void SetGeomDisplayType( int type );
    void EditName( string name );

    vector< string > GetSelectedBrowserItems();


};


#endif // !defined(MAINSCREEN__INCLUDED_)
