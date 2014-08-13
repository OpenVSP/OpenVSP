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


//==== Constructor ====//
CustomScreen::CustomScreen( ScreenMgr* mgr ) : GeomScreen( mgr, 300, 500, "Custom" )
{
    m_FLTK_Window->copy_label( "Custom Geom" );
    m_SameLineFlag = false;
    m_LastChoiceGui = NULL;
    m_ForceWidthFlag = false;
    m_ForceWidth = 10;
    m_Format = "%7.3f";

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
        if ( gd )
            gd->SetIndex( i );

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
    GuiDevice* gui_dev = NULL;

    if ( def.m_Type == GDEV_TAB )
    {
        Fl_Group* tab_group = AddTab( def.m_Label );
        Fl_Group* design_group = AddSubGroup( tab_group, 5 );
        m_Layout.SetGroupAndScreen( design_group, this );

        Tab* t = new Tab();
        t->Init( tab_group );
        gui_dev = t;
    }
    else if ( def.m_Type == GDEV_SLIDER_ADJ_RANGE_INPUT )
    {
        SliderAdjRangeInput* slider = new SliderAdjRangeInput();
        m_Layout.AddSlider( *slider, ( const char* )def.m_Label.c_str(), def.m_Range, m_Format.c_str() );
        gui_dev = slider;
    }
    else if ( def.m_Type == GDEV_SLIDER_INPUT )
    {
        SliderInput* slider = new SliderInput();
        m_Layout.AddSlider( *slider, ( const char* )def.m_Label.c_str(), def.m_Range, m_Format.c_str() );
        gui_dev = slider;
    }
    else if ( def.m_Type == GDEV_INPUT )
    {
        Input* input = new Input();
        m_Layout.AddInput( *input, ( const char* )def.m_Label.c_str(), m_Format.c_str() );
        gui_dev = input;
    }
    else if ( def.m_Type == GDEV_PARM_BUTTON )
    {
        ParmButton* pb = new ParmButton();
        m_Layout.AddButton( *pb, ( const char* )def.m_Label.c_str() );
        gui_dev = pb;
    }
   else if ( def.m_Type == GDEV_YGAP )
    {
        m_Layout.AddYGap();
    }
    else if ( def.m_Type == GDEV_DIVIDER_BOX )
    {
        m_Layout.AddDividerBox( ( const char* )def.m_Label.c_str() );
    }
    else if ( def.m_Type == GDEV_TOGGLE_BUTTON )
    {
        ToggleButton* tb = new ToggleButton();
        m_Layout.AddButton( *tb, ( const char* )def.m_Label.c_str() );
        gui_dev = tb;
    }
    else if ( def.m_Type == GDEV_TRIGGER_BUTTON )
    {
        TriggerButton* tb = new TriggerButton();
        m_Layout.AddButton( *tb, ( const char* )def.m_Label.c_str() );
        gui_dev = tb;
    }
    else if ( def.m_Type == GDEV_INDEX_SELECTOR )
    {
        IndexSelector* tb = new IndexSelector();
        m_Layout.AddIndexSelector( *tb );
        gui_dev = tb;
    }
    else if ( def.m_Type == GDEV_COUNTER )
    {
        Counter* tb = new Counter();
        m_Layout.AddCounter( *tb,  ( const char* )def.m_Label.c_str() );
        gui_dev = tb;
    }
    else if ( def.m_Type == GDEV_CHOICE )
    {
        Choice* ch = new Choice();
        m_LastChoiceGui = ch;
        m_Layout.AddChoice( *ch,  ( const char* )def.m_Label.c_str() );
        gui_dev = ch;
    }
    else if ( def.m_Type == GDEV_ADD_CHOICE_ITEM )
    {
        if ( m_LastChoiceGui )
        {
            m_LastChoiceGui->AddItem( def.m_Label );
        }
        m_LastChoiceGui->UpdateItems();
    }
    else if ( def.m_Type == GDEV_BEGIN_SAME_LINE )
    {
        m_Layout.SetFitWidthFlag( false );
        m_Layout.SetSameLineFlag( true );
        m_SameLineVec.clear();
        m_SameLineFlag = true;
    }
    else if ( def.m_Type == GDEV_END_SAME_LINE )
    {
        m_Layout.SetFitWidthFlag( true );
        m_Layout.SetSameLineFlag( false );
        m_Layout.ForceNewLine();
        m_SameLineFlag = false;

        int num_same_line = (int)m_SameLineVec.size();
        if ( num_same_line )
        {
            int dev_x = m_Layout.GetStartX();

            int total_w = 1;
            for ( int i = 0 ; i < (int)m_SameLineVec.size() ; i++ )
            {
                total_w += m_SameLineVec[i]->GetWidth();
            }
            for ( int i = 0 ; i < (int)m_SameLineVec.size() ; i++ )
            {
                int dev_w = (m_Layout.GetW()*m_SameLineVec[i]->GetWidth())/total_w;
                m_SameLineVec[i]->SetWidth( dev_w );
                m_SameLineVec[i]->SetX( dev_x );
                dev_x += dev_w;
            }
        }
    }
    else if ( def.m_Type == GDEV_FORCE_WIDTH )
    {
        int w = std::stoi( def.m_Label );
        if ( w > 0 && w < 1000 )
        {
            m_ForceWidthFlag = true;
            m_ForceWidth = std::stoi( def.m_Label );
        }
    }
    else if ( def.m_Type == GDEV_SET_FORMAT )
    {
        m_Format = def.m_Label;
    }


    //==== Add To Same Line Vector ====//
    if ( m_SameLineFlag && gui_dev )
    {
        m_SameLineVec.push_back( gui_dev );
    }

    //==== Force Width ====//
    if ( m_ForceWidthFlag && gui_dev )
    {
        gui_dev->SetWidth( m_ForceWidth );
        m_ForceWidthFlag = false;
    }

    return gui_dev;

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
    if ( iter != m_DeviceVecMap.end() )
    {
        for ( int i = 0 ; i < ( int )iter->second.size() ; i++ )
        {
            GuiDevice* gd = iter->second.at( i );
            if ( gd && gd->GetType() == GDEV_TAB )
            {
                Tab* gd_tab = dynamic_cast< Tab* > ( gd );
                TabScreen::AddTab( gd_tab->GetGroup() );
            }
        }
    }

    m_CurrTabLayoutName = custom_type_name;
}



//==== Show Custom Screen ====//
void CustomScreen::Show()
{
    if ( Update() )
    {
        m_FLTK_Window->show();
    }
}

//==== Update Custom Screen ====//
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

//==== Gui Device Callbacks ====//
void CustomScreen::GuiDeviceCallBack( GuiDevice* device )
{
    GeomScreen::GuiDeviceCallBack( device );

    //==== Find Curr Geom ====//
    Geom* geom_ptr = m_ScreenMgr->GetCurrGeom();
    if ( !geom_ptr || geom_ptr->GetType().m_Type != CUSTOM_GEOM_TYPE )
        return;

    //==== Cast To Custom Geom ====//
    CustomGeom* custom_geom = dynamic_cast<CustomGeom*>( geom_ptr );
    if ( !custom_geom )
        return;

    custom_geom->AddGuiTriggerEvent( device->GetIndex() );


}



//==== Non Menu Callbacks ====//
void CustomScreen::CallBack( Fl_Widget *w )
{
    GeomScreen::CallBack( w );
}




