//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//

// CustomScreen.h: UI for Custom Geom
// J.R Gloudemans
//
//////////////////////////////////////////////////////////////////////

#if !defined(CUSTOMSCREEN__INCLUDED_)
#define CUSTOMSCREEN__INCLUDED_

#include "ScreenBase.h"
#include "GuiDevice.h"
#include "GuiDeviceEnums.h"
#include "CustomGeom.h"

class CustomScreen : public GeomScreen
{
public:
    CustomScreen( ScreenMgr* mgr );
    virtual ~CustomScreen()                         {}

    virtual void Show();
    virtual bool Update();
    virtual void InitGui( Geom* geom_ptr );
    virtual void InitGuiDeviceVec( Geom* geom_ptr );
    virtual GuiDevice* AddGuiItem( GuiDef & def, int id );
    virtual void ShowTabs( const string & custom_type_name );

    virtual void CallBack( Fl_Widget *w );

protected:

    GuiDevice* FindGuiDevice( const string & custom_type_name, int index );

    GroupLayout m_Layout;
    SliderAdjRangeInput m_Slider;
    map< string, vector< GuiDevice* > > m_DeviceVecMap;

    string m_CurrTabLayoutName;

};


#endif // !defined(CUSTOMSCREEN__INCLUDED_)
