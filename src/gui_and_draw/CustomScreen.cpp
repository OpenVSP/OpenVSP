//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//
//
//////////////////////////////////////////////////////////////////////

#include "CustomScreen.h"
#include "ScreenMgr.h"
#include "PodGeom.h"
#include "EventMgr.h"
#include "Vehicle.h"
#include "ParmMgr.h"

#include <assert.h>

//
//
// Must match parms to gui_devices
// gui_device_vec[i].Update( geom_ptr->GetParm(i).GetID() );
//


//==== Constructor ====//
CustomScreen::CustomScreen( ScreenMgr* mgr ) : GeomScreen( mgr, 300, 500, "Custom" )
{
    m_FLTK_Window->copy_label( "Custom Geom" );
}

void CustomScreen::InitGui( Geom* geom_ptr )
{
    string custom_type_name =  geom_ptr->GetType().m_Name;

    //==== Check The Gui Is Already Defined For This Type ====//
    map< string, vector< GuiDevice* > >::iterator iter = m_DeviceVecMap.find( custom_type_name );

    if ( iter == m_DeviceVecMap.end() )
    {
        InitGuiDeviceVec( geom_ptr );
    }
}

void CustomScreen::InitGuiDeviceVec( Geom* geom_ptr )
{
    vector< GuiDef > gui_def_vec = CustomGeomMgr.GetGuiDefVec( geom_ptr->GetID() );

    for ( int i = 0 ; i < ( int )gui_def_vec.size() ; i++ )
    {
        GuiDevice* gd = AddGuiItem( gui_def_vec[i], i );

        string custom_type_name =  geom_ptr->GetType().m_Name;
        m_DeviceVecMap[custom_type_name].push_back( gd );
    }
}

GuiDevice* CustomScreen::FindGuiDevice( const string & custom_type_name, int index )
{
    map< string, vector< GuiDevice* > >::iterator iter = m_DeviceVecMap.find( custom_type_name );

    if ( iter != m_DeviceVecMap.end() )
    {
        if ( index >= 0 && index < ( int )iter->second.size() )
        {
            return iter->second.at( index );
        }
    }
    return NULL;
}



GuiDevice* CustomScreen::AddGuiItem( GuiDef & def, int id )
{
    if ( def.m_Type == GDEV_TAB )
    {
        Fl_Group* tab_group = AddTab( def.m_Label );
        Fl_Group* design_group = AddSubGroup( tab_group, 5 );
        m_Layout.SetGroupAndScreen( design_group, this );

        Tab* t = new Tab();
        t->Init( tab_group );

        return t;
    }
    else if ( def.m_Type == GDEV_SLIDER_ADJ_RANGE_INPUT )
    {
        SliderAdjRangeInput* slider = new SliderAdjRangeInput();
        m_Layout.AddSlider( *slider, ( const char* )def.m_Label.c_str(), 1, "%7.3f" );
        return slider;
    }
    else if ( def.m_Type == GDEV_YGAP )
    {
        m_Layout.AddYGap();
        return NULL;
    }
    else if ( def.m_Type == GDEV_DIVIDER_BOX )
    {
        m_Layout.AddDividerBox( ( const char* )def.m_Label.c_str() );
        return NULL;
    }
    else if ( def.m_Type == GDEV_TOGGLE_BUTTON )
    {
        ToggleButton* tb = new ToggleButton();
        m_Layout.AddButton( *tb, ( const char* )def.m_Label.c_str() );
        return tb;
    }
    return NULL;

}

//==== Show Tabs of Custom Geom Type (Hide All Others) ====//
void CustomScreen::ShowTabs( const string & custom_type_name )
{

    if ( m_CurrTabLayoutName == custom_type_name )
    {
        return;
    }

    map< string, vector< GuiDevice* > >::iterator iter;

    //==== Remove Add Tabs ====//
    for ( iter = m_DeviceVecMap.begin() ; iter != m_DeviceVecMap.end() ; iter++ )
    {
        for ( int i = 0 ; i < ( int )iter->second.size() ; i++ )
        {
            GuiDevice* gd = iter->second.at( i );
            if ( gd && gd->GetType() == GDEV_TAB )
            {
                Tab* gd_tab = dynamic_cast< Tab* > ( gd );

                TabScreen::RemoveTab( gd_tab->GetGroup() );
            }
        }
    }

    //===== Add Tabs Back In For This Type ====//
    iter = m_DeviceVecMap.find( custom_type_name );
    for ( int i = 0 ; i < ( int )iter->second.size() ; i++ )
    {
        GuiDevice* gd = iter->second.at( i );
        if ( gd && gd->GetType() == GDEV_TAB )
        {
            Tab* gd_tab = dynamic_cast< Tab* > ( gd );
            TabScreen::AddTab( gd_tab->GetGroup() );
        }
    }

    m_CurrTabLayoutName = custom_type_name;
}



//==== Show Pod Screen ====//
void CustomScreen::Show()
{
    if ( Update() )
    {
        m_FLTK_Window->show();
    }
}

//==== Update Pod Screen ====//
bool CustomScreen::Update()
{
    assert( m_ScreenMgr );

    Geom* geom_ptr = m_ScreenMgr->GetCurrGeom();
    if ( !geom_ptr || geom_ptr->GetType().m_Type != CUSTOM_GEOM_TYPE )
    {
        Hide();
        return false;
    }

    //==== Make Sure Gui Devices Are Created - Only Once ====//
    InitGui( geom_ptr );

    //==== Set Title Tabs Window ====//
    string custom_type_name = geom_ptr->GetType().m_Name;
    TabScreen::SetTitle( custom_type_name );

    GeomScreen::Update();

    //==== Update Custom GUI ====//
    ShowTabs( custom_type_name );
    CustomGeomMgr.SetCurrCustomGeom( geom_ptr->GetID() );

    vector< GuiUpdate > update_vec = CustomGeomMgr.GetGuiUpdateVec();

    for ( int i = 0 ; i < ( int )update_vec.size() ; i++ )
    {
        int gui_index = update_vec[i].m_GuiID;
        GuiDevice* gui_dev = FindGuiDevice( geom_ptr->GetType().m_Name, gui_index );

        if ( gui_dev )
        {
            gui_dev->Update( update_vec[i].m_ParmID );
        }

    }

    return true;
}


//==== Non Menu Callbacks ====//
void CustomScreen::CallBack( Fl_Widget *w )
{
    GeomScreen::CallBack( w );
}




