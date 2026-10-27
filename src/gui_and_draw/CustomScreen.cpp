//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//
//
//////////////////////////////////////////////////////////////////////

#include "CustomScreen.h"
#include "ScreenMgr.h"
#include "ParmMgr.h"

#include <algorithm>
#include <cstdlib>


//==== Constructor ====//
CustomScreen::CustomScreen( ScreenMgr* mgr ) : GeomScreen( mgr, 400, 800, "Custom" )
{
    m_FLTK_Window->copy_label( "Custom Geom" );
    m_SameLineFlag = false;
    m_SameLinePlacedFlag = false;
    m_LastChoiceGui = nullptr;
    m_LastRadioGroup = nullptr;
    m_ForceWidthFlag = false;
    m_ForceWidth = 10;
    m_Format = "%7.3f";

 
}

void CustomScreen::InitGui( Geom* geom_ptr )
{
    //==== Cast To Custom Geom ====//
    CustomGeom* custom_geom = dynamic_cast<CustomGeom*>( geom_ptr );
    if ( !custom_geom )
        return;

    string custom_type_name = custom_geom->GetScriptModuleName();

    //==== Check The Gui Is Already Defined For This Type ====//
    unordered_map< string, vector< GuiDevice* > >::iterator iter = m_DeviceVecMap.find( custom_type_name );

    if ( iter == m_DeviceVecMap.end() )
    {
        InitGuiDeviceVec( geom_ptr );
    }
}

void CustomScreen::InitGuiDeviceVec( Geom* geom_ptr )
{
    //==== Cast To Custom Geom ====//
    CustomGeom* custom_geom = dynamic_cast<CustomGeom*>( geom_ptr );
    if ( !custom_geom )
        return;

    vector< GuiDef > gui_def_vec = CustomGeomMgr.GetGuiDefVec( geom_ptr->GetID() );

    for ( int i = 0 ; i < ( int )gui_def_vec.size() ; i++ )
    {
        GuiDevice* gd = AddGuiItem( gui_def_vec[i], i );
        if ( gd )
            gd->SetIndex( i );

        string custom_type_name = custom_geom->GetScriptModuleName();
        m_DeviceVecMap[custom_type_name].push_back( gd );
    }
}

GuiDevice* CustomScreen::FindGuiDevice( const string & custom_type_name, int index )
{
    unordered_map< string, vector< GuiDevice* > >::iterator iter = m_DeviceVecMap.find( custom_type_name );

    if ( iter != m_DeviceVecMap.end() )
    {
        if ( index >= 0 && index < ( int )iter->second.size() )
        {
            return iter->second.at( index );
        }
    }
    return nullptr;
}



// The integer a choice item's value string holds, when it holds one and nothing else.
static bool ChoiceItemValue( const string & str, int & val )
{
    if ( str.empty() )
    {
        return false;
    }
    char* end = nullptr;
    long v = strtol( str.c_str(), &end, 10 );
    if ( end == str.c_str() || *end != '\0' )
    {
        return false;
    }
    val = ( int )v;
    return true;
}

GuiDevice* CustomScreen::AddGuiItem( GuiDef & def, int id )
{
    GuiDevice* gui_dev = nullptr;

    if ( def.m_Type == vsp::GDEV_TAB )
    {
        Fl_Group* tab_group = AddTab( def.m_Label );
        Fl_Group* design_group = AddSubGroup( tab_group, 5 );
        m_Layout.SetGroupAndScreen( design_group, this );

        Tab* t = new Tab();
        t->Init( tab_group );
        gui_dev = t;
    }
    else if ( def.m_Type == vsp::GDEV_SCROLL_TAB )
    {
        Fl_Group* tab_group = AddTab( def.m_Label );
        Fl_Scroll* design_scroll = AddSubScroll( tab_group, 5 );
        design_scroll->type( Fl_Scroll::VERTICAL_ALWAYS );
        m_Layout.SetGroupAndScreen( design_scroll, this );

        Tab* t = new Tab();
        t->Init( tab_group );
        gui_dev = t;
    }
    else if ( def.m_Type == vsp::GDEV_SLIDER_ADJ_RANGE_INPUT )
    {
        SliderAdjRangeInput* slider = new SliderAdjRangeInput();
        m_Layout.AddSlider( *slider, ( const char* )def.m_Label.c_str(), def.m_Range, m_Format.c_str() );
        gui_dev = slider;
    }
    else if ( def.m_Type == vsp::GDEV_SLIDER_ADJ_RANGE_TWO_INPUT )
    {
        SliderAdjRange2Input* slider = new SliderAdjRange2Input();
        m_Layout.AddSlider( *slider, ( const char* )def.m_Label.c_str(), def.m_Range, m_Format.c_str() );
        gui_dev = slider;
    }
    else if ( def.m_Type == vsp::GDEV_SLIDER_INPUT )
    {
        SliderInput* slider = new SliderInput();
        m_Layout.AddSlider( *slider, ( const char* )def.m_Label.c_str(), def.m_Range, m_Format.c_str() );
        gui_dev = slider;
    }
    else if ( def.m_Type == vsp::GDEV_INPUT )
    {
        Input* input = new Input();
        m_Layout.AddInput( *input, ( const char* )def.m_Label.c_str(), m_Format.c_str() );
        gui_dev = input;
    }
    else if ( def.m_Type == vsp::GDEV_OUTPUT )
    {
        Output* output = new Output();
        m_Layout.AddOutput( *output, ( const char* )def.m_Label.c_str(), m_Format.c_str() );
        gui_dev = output;
    }
    else if ( def.m_Type == vsp::GDEV_CHECK_BUTTON )
    {
        CheckButton* cb = new CheckButton();
        m_Layout.AddButton( *cb, ( const char* )def.m_Label.c_str() );
        gui_dev = cb;
    }
    else if ( def.m_Type == vsp::GDEV_PARM_BUTTON )
    {
        ParmButton* pb = new ParmButton();
        m_Layout.AddButton( *pb, ( const char* )def.m_Label.c_str() );
        gui_dev = pb;
    }
   else if ( def.m_Type == vsp::GDEV_YGAP )
    {
        m_Layout.AddYGap();
    }
    else if ( def.m_Type == vsp::GDEV_DIVIDER_BOX )
    {
        m_Layout.AddDividerBox( ( const char* )def.m_Label.c_str() );
    }
    else if ( def.m_Type == vsp::GDEV_TOGGLE_BUTTON )
    {
        ToggleButton* tb = new ToggleButton();
        m_Layout.AddButton( *tb, ( const char* )def.m_Label.c_str() );
        gui_dev = tb;
    }
    else if ( def.m_Type == vsp::GDEV_TRIGGER_BUTTON )
    {
        TriggerButton* tb = new TriggerButton();
        m_Layout.AddButton( *tb, ( const char* )def.m_Label.c_str() );
        gui_dev = tb;
    }
    else if ( def.m_Type == vsp::GDEV_INDEX_SELECTOR )
    {
        IndexSelector* tb = new IndexSelector();
        m_Layout.AddIndexSelector( *tb );
        gui_dev = tb;
    }
    else if ( def.m_Type == vsp::GDEV_COUNTER )
    {
        Counter* tb = new Counter();
        m_Layout.AddCounter( *tb,  ( const char* )def.m_Label.c_str() );
        gui_dev = tb;
    }
    else if ( def.m_Type == vsp::GDEV_CHOICE )
    {
        Choice* ch = new Choice();
        m_LastChoiceGui = ch;
        m_LastRadioGroup = nullptr;
        m_Layout.AddChoice( *ch,  ( const char* )def.m_Label.c_str() );
        gui_dev = ch;
    }
    else if ( def.m_Type == vsp::GDEV_TOGGLE_RADIO_GROUP )
    {
        // Its toggle buttons are the items added to it.
        ToggleRadioGroup* rg = new ToggleRadioGroup();
        rg->Init( this );
        m_LastRadioGroup = rg;
        m_LastRadioVals.clear();
        m_LastChoiceGui = nullptr;
        gui_dev = rg;
    }
    else if ( def.m_Type == vsp::GDEV_ADD_CHOICE_ITEM )
    {
        // An item's value is the integer in AddGui's arg5, where one is given, or else its place in the list.
        int val = -1;
        bool given = ChoiceItemValue( def.m_ParmName2, val );
        if ( m_LastChoiceGui )
        {
            if ( given )
            {
                m_LastChoiceGui->AddItem( def.m_Label, val );
            }
            else
            {
                m_LastChoiceGui->AddItem( def.m_Label );
            }
            m_LastChoiceGui->UpdateItems();
        }
        else if ( m_LastRadioGroup )
        {
            if ( !given )
            {
                val = ( int )m_LastRadioVals.size();
            }

            ToggleButton* tb = new ToggleButton();
            m_Layout.AddButton( *tb, ( const char* )def.m_Label.c_str() );
            m_LastRadioGroup->AddButton( tb->GetFlButton() );
            m_LastRadioVals.push_back( val );
            m_LastRadioGroup->SetValMapVec( m_LastRadioVals );
            gui_dev = tb;
        }
    }
    else if ( def.m_Type == vsp::GDEV_BEGIN_SAME_LINE )
    {
        m_Layout.SetFitWidthFlag( false );
        m_Layout.SetSameLineFlag( true );
        m_SameLineVec.clear();
        m_SameLineFlag = true;
        m_SameLinePlacedFlag = false;
    }
    else if ( def.m_Type == vsp::GDEV_END_SAME_LINE )
    {
        m_Layout.SetFitWidthFlag( true );
        m_Layout.SetSameLineFlag( false );
        m_Layout.ForceNewLine();
        m_SameLineFlag = false;

        // A line placed with GDEV_ADD_X keeps its widths; any other is stretched to fill the line.
        int num_same_line = (int)m_SameLineVec.size();
        if ( num_same_line && !m_SameLinePlacedFlag )
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
    else if ( def.m_Type == vsp::GDEV_FORCE_WIDTH )
    {
        int w = std::stoi( def.m_Label );
        if ( w > 0 && w < 1000 )
        {
            m_ForceWidthFlag = true;
            m_ForceWidth = std::stoi( def.m_Label );
        }
    }
    else if ( def.m_Type == vsp::GDEV_SET_FORMAT )
    {
        m_Format = def.m_Label;
    }
    else if ( def.m_Type == vsp::GDEV_ADD_X )
    {
        m_Layout.AddX( LayoutWidth( def.m_Label ) );
        if ( m_SameLineFlag )
        {
            m_SameLinePlacedFlag = true;
        }
    }
    else if ( def.m_Type == vsp::GDEV_SET_BUTTON_WIDTH )
    {
        m_Layout.SetButtonWidth( LayoutWidth( def.m_Label ) );
    }
    else if ( def.m_Type == vsp::GDEV_SET_CHOICE_BUTTON_WIDTH )
    {
        m_Layout.SetChoiceButtonWidth( LayoutWidth( def.m_Label ) );
    }


    //==== Add To Same Line Vector, Where A Radio Group's Buttons Stand For It ====//
    if ( m_SameLineFlag && gui_dev && def.m_Type != vsp::GDEV_TOGGLE_RADIO_GROUP )
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

// A width in pixels from expr, a sum of terms, each an optional count and '*' and then a number of pixels or one
// of the layout's widths: W, the whole line, INPUT, BUTTON, CHOICE_BUTTON or RANGE_BUTTON.  "W-2*INPUT" is
// the line less two inputs.
int CustomScreen::LayoutWidth( const string & expr )
{
    int total = 0;
    size_t i = 0;
    while ( i < expr.size() )
    {
        int sign = 1;
        while ( i < expr.size() && ( expr[i] == ' ' || expr[i] == '+' || expr[i] == '-' ) )
        {
            if ( expr[i] == '-' )
            {
                sign = -sign;
            }
            i++;
        }

        size_t end = expr.find_first_of( "+-", i );
        if ( end == string::npos )
        {
            end = expr.size();
        }
        string term = expr.substr( i, end - i );
        i = end;

        int count = 1;
        size_t star = term.find( '*' );
        if ( star != string::npos )
        {
            count = atoi( term.substr( 0, star ).c_str() );
            term = term.substr( star + 1 );
        }
        term.erase( std::remove( term.begin(), term.end(), ' ' ), term.end() );

        int w = 0;
        if ( term == "W" )
        {
            w = m_Layout.GetW();
        }
        else if ( term == "INPUT" )
        {
            w = m_Layout.GetInputWidth();
        }
        else if ( term == "BUTTON" )
        {
            w = m_Layout.GetButtonWidth();
        }
        else if ( term == "CHOICE_BUTTON" )
        {
            w = m_Layout.GetChoiceButtonWidth();
        }
        else if ( term == "RANGE_BUTTON" )
        {
            w = m_Layout.GetRangeButtonWidth();
        }
        else
        {
            w = atoi( term.c_str() );
        }
        total += sign * count * w;
    }
    return total;
}

//==== Show Tabs of Custom Geom Type (Hide All Others) ====//
void CustomScreen::ShowTabs( const string & custom_type_name )
{

    if ( m_CurrTabLayoutName == custom_type_name )
    {
        return;
    }

    unordered_map< string, vector< GuiDevice* > >::iterator iter;

    //==== Remove Add Tabs ====//
    for ( iter = m_DeviceVecMap.begin() ; iter != m_DeviceVecMap.end() ; iter++ )
    {
        for ( unsigned int i = 0 ; i < ( int )iter->second.size() ; i++ )
        {
            GuiDevice* gd = iter->second.at( i );
            if ( gd && gd->GetType() == vsp::GDEV_TAB )
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
        for ( unsigned int i = 0 ; i < ( int )iter->second.size() ; i++ )
        {
            GuiDevice* gd = iter->second.at( i );
            if ( gd && gd->GetType() == vsp::GDEV_TAB )
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
        GeomScreen::Show();
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

    //==== Cast To Custom Geom ====//
    CustomGeom* custom_geom = dynamic_cast<CustomGeom*>( geom_ptr );
    if ( !custom_geom )
    {
        return false;
    }

    //==== Set Title Tabs Window ====//
    string custom_type_name = custom_geom->GetScriptModuleName();
    string display_name = custom_geom->GetDisplayName();
    TabScreen::SetTitle( display_name );

    GeomScreen::Update();

    //==== Update Custom GUI ====//
    ShowTabs( custom_type_name );
    CustomGeomMgr.SetCurrCustomGeom( geom_ptr->GetID() );

    vector< GuiUpdate > update_vec = CustomGeomMgr.GetGuiUpdateVec();

    for ( int i = 0 ; i < ( int )update_vec.size() ; i++ )
    {
        int gui_index = update_vec[i].m_GuiID;
        GuiDevice* gui_dev = FindGuiDevice( custom_type_name, gui_index );

        SliderAdjRange2Input* two = dynamic_cast< SliderAdjRange2Input* >( gui_dev );
        if ( two )
        {
            if ( update_vec[i].m_ParmID.empty() || update_vec[i].m_ParmID2.empty() )
            {
                two->Deactivate();
            }
            else if ( update_vec[i].m_Input == 2 )
            {
                two->Update( 2, update_vec[i].m_ParmID, update_vec[i].m_ParmID2 );
                two->ActivateInput2();
            }
            else
            {
                two->Update( 1, update_vec[i].m_ParmID, update_vec[i].m_ParmID2 );
                two->ActivateInput1();
            }
        }
        else if ( gui_dev )
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




