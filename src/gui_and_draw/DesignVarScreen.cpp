//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//

// ParmDebugScreen.cpp: implementation of the ParmDebugScreen class.
//
//////////////////////////////////////////////////////////////////////

#include "DesignVarScreen.h"
#include "ParmMgr.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

DesignVarScreen::DesignVarScreen( ScreenMgr* mgr ) : TabScreen( mgr, 300, 463, "Design Variables" )
{
    m_NVarLast = 0;

    Fl_Group* pick_tab = AddTab( "Pick" );
    ( (Vsp_Group*) pick_tab )->SetAllowDrop( true );
    pick_tab->callback( staticCB, this );

    Fl_Group* adj_tab = AddTab( "Adjust" );
    Fl_Group* pick_group = AddSubGroup( pick_tab, 5 );
    m_AdjustGroup = AddSubScroll( adj_tab, 5 );
    m_AdjustGroup->type( Fl_Scroll::VERTICAL_ALWAYS );

    m_PickLayout.SetGroupAndScreen( pick_group, this );

    m_PickLayout.AddDividerBox( "Variable" );

    m_PickLayout.AddParmPicker( m_ParmPicker );

    m_PickLayout.SetFitWidthFlag( false );
    m_PickLayout.SetSameLineFlag( true );

    m_PickLayout.AddLabel( "XDDM Type:", 100 );
    m_PickLayout.SetButtonWidth( m_PickLayout.GetRemainX() / 2 );

    m_PickLayout.AddButton( m_XDDMVarButton, "Variable" );
    m_PickLayout.AddButton( m_XDDMConstButton, "Constant" );
    m_PickLayout.ForceNewLine();

    m_XDDMGroup.Init( this );
    m_XDDMGroup.AddButton( m_XDDMVarButton.GetFlButton() );
    m_XDDMGroup.AddButton( m_XDDMConstButton.GetFlButton() );

    vector< int > dv_val_map;
    dv_val_map.push_back( vsp::XDDM_VAR );
    dv_val_map.push_back( vsp::XDDM_CONST );
    m_XDDMGroup.SetValMapVec( dv_val_map );

    m_PickLayout.AddYGap();

    m_PickLayout.SetButtonWidth( ( m_PickLayout.GetRemainX() ) / 2 );
    m_PickLayout.AddButton( m_AddVarButton, "Add Variable" );
    m_PickLayout.AddButton( m_DelVarButton, "Delete Variable" );

    m_PickLayout.ForceNewLine();

    m_PickLayout.SetFitWidthFlag( true );
    m_PickLayout.SetSameLineFlag( false );

    m_PickLayout.AddYGap();

    m_PickLayout.AddDividerBox( "Variable List" );

    int browser_h = 200;
    varBrowser = new Fl_Browser( m_PickLayout.GetX(), m_PickLayout.GetY(), m_PickLayout.GetW(), browser_h );
    varBrowser->type( 1 );
    varBrowser->labelfont( 13 );
    varBrowser->labelsize( 12 );
    varBrowser->textsize( 12 );
    varBrowser->callback( staticScreenCB, this );

    pick_group->add( varBrowser );
    m_PickLayout.AddY( browser_h );

    m_PickLayout.AddYGap();

    m_PickLayout.AddDividerBox( "Design File" );

    m_FileTypeChoice.AddItem( "Design file *.des" );
    m_FileTypeChoice.AddItem( "Cart3D XDDM file *.xddm" );
    m_PickLayout.AddChoice( m_FileTypeChoice, "File Type:" );

    m_PickLayout.SetFitWidthFlag( false );
    m_PickLayout.SetSameLineFlag( true );

    m_PickLayout.AddButton( m_SaveButton, "Save" );
    m_PickLayout.AddButton( m_LoadButton, "Load" );
    m_PickLayout.ForceNewLine();

    pick_tab->show();

    m_AdjustLayout.SetGroupAndScreen( m_AdjustGroup, this );
}

DesignVarScreen::~DesignVarScreen()
{
}

bool DesignVarScreen::Update()
{
    int i;
    char str[256];

    // Check that all Parms exist.  Needed in case a Geom with DesVars is
    // deleted.
    DesignVarMgr.CheckVars();

    // Re-sort DesVars.  Needed in case a Geom's name is changed.
    if ( !DesignVarMgr.SortVars() )
    {
        RebuildAdjustTab();
    }

    //==== Update Parm Picker ====//
    m_ParmPicker.Update();

    m_XDDMGroup.Update( DesignVarMgr.m_WorkingXDDMType.GetID() );

    //==== Update Parm Browser ====//
    varBrowser->clear();

    static int widths[] = { 75, 75, 90, 20 }; // widths for each column
    varBrowser->column_widths( widths );    // assign array to widget
    varBrowser->column_char( ':' );         // use : as the column character

    sprintf( str, "@b@.COMP_A:@b@.GROUP:@b@.PARM:@b@.V/C" );
    varBrowser->add( str );

    int num_vars = DesignVarMgr.GetNumVars();
    for ( i = 0 ; i < num_vars ; i++ )
    {
        DesignVar* dv = DesignVarMgr.GetVar( i );
        char vtype;
        if( dv->m_XDDM_Type == false )
        {
            vtype = 'V';
        }
        else
        {
            vtype = 'C';
        }

        string c_name, g_name, p_name;
        ParmMgr.GetNames( dv->m_ParmID, c_name, g_name, p_name );

        sprintf( str, "%s:%s:%s:%c", c_name.c_str(), g_name.c_str(), p_name.c_str(), vtype );
        varBrowser->add( str );
    }

    int index = DesignVarMgr.GetCurrVarIndex();
    if ( index >= 0 && index < num_vars )
    {
        varBrowser->select( index + 2 );
    }

    // Parameter GUI got out of sync.  Probably from File->New or similar.
    if ( m_NVarLast != num_vars )
    {
        RebuildAdjustTab();
    }

    //==== Update Parm Adjust Tab ====//
    for ( int i = 0 ; i < num_vars ; i++ )
    {
        m_ParmSliderVec[i].Update( DesignVarMgr.GetVar( i )->m_ParmID );
    }

    m_FLTK_Window->redraw();

    return false;
}

void DesignVarScreen::RebuildAdjustTab()
{
    m_AdjustGroup->clear();
    m_AdjustLayout.SetGroup( m_AdjustGroup );
    m_AdjustLayout.InitWidthHeightVals();

    m_ParmSliderVec.clear();

    int num_vars = DesignVarMgr.GetNumVars();
    m_ParmSliderVec.resize( num_vars );

    string lastContID;

    for ( int i = 0 ; i < num_vars ; i++ )
    {
        string pID = DesignVarMgr.GetVar( i )->m_ParmID;

        Parm* p = ParmMgr.FindParm( pID );

        string contID = p->GetContainerID();

        if ( contID.compare( lastContID ) != 0 )
        {
            lastContID = contID;
            m_AdjustLayout.AddDividerBox( ParmMgr.FindParmContainer( contID )->GetName() );
        }

        m_AdjustLayout.AddSlider( m_ParmSliderVec[i], "AUTO_UPDATE", 10, "%7.3f" );
        m_ParmSliderVec[i].Update( pID );
    }

    m_NVarLast = num_vars;
}

void DesignVarScreen::Show()
{
    Update();
    m_FLTK_Window->show();
}

void DesignVarScreen::Hide()
{
    m_FLTK_Window->hide();
}

void DesignVarScreen::CallBack( Fl_Widget* w )
{
    assert( m_ScreenMgr );

    if( Fl::event() == FL_PASTE )
    {
        string ParmID( Fl::event_text() );
        DesignVarMgr.AddVar( ParmID, DesignVarMgr.m_WorkingXDDMType.Get() );
        RebuildAdjustTab();
    }
    else if (  w == varBrowser )
    {
        int sel = varBrowser->value();
        DesignVarMgr.SetCurrVarIndex( sel - 2 );

        DesignVar *dv = DesignVarMgr.GetCurrVar();

        if ( dv )
        {
            m_ParmPicker.SetParmChoice( dv->m_ParmID );
            DesignVarMgr.m_WorkingXDDMType = dv->m_XDDM_Type;
        }
        else
        {
            m_ParmPicker.SetParmChoice( string() );
            DesignVarMgr.m_WorkingXDDMType = vsp::XDDM_VAR;
        }
    }

    m_ScreenMgr->SetUpdateFlag( true );
}

void DesignVarScreen::GuiDeviceCallBack( GuiDevice* device )
{
    assert( m_ScreenMgr );

    if ( device == &m_AddVarButton )
    {
        DesignVarMgr.AddCurrVar( );
        RebuildAdjustTab();
    }
    else if ( device == &m_DelVarButton )
    {
        DesignVarMgr.DelCurrVar( );
        RebuildAdjustTab();
    }
    else if ( device == &m_ParmPicker )
    {
        DesignVarMgr.SetWorkingParmID( m_ParmPicker.GetParmChoice() );
        DesignVarMgr.SetCurrVarIndex( -1 );
    }
    else if ( device == &m_XDDMGroup )
    {
        DesignVarMgr.SetCurrVarIndex( -1 );
    }
    else if ( device == &m_SaveButton )
    {
        if ( m_FileTypeChoice.GetVal() == 0 )
        {
            string newfile = m_ScreenMgr->GetSelectFileScreen()->FileChooser( "Design File", "*.des" );
            if ( newfile.size() != 0 && newfile[ newfile.size() - 1] != '/' )
            {
                DesignVarMgr.WriteDesVarsDES( newfile );
            }
        }
        else
        {
            string newfile = m_ScreenMgr->GetSelectFileScreen()->FileChooser( "Cart3D XDDM File", "*.xddm" );
            if ( newfile.size() != 0 && newfile[ newfile.size() - 1] != '/' )
            {
                DesignVarMgr.WriteDesVarsXDDM( newfile );
            }
        }
    }
    else if ( device == &m_LoadButton )
    {
        if ( m_FileTypeChoice.GetVal() == 0 )
        {
            string newfile = m_ScreenMgr->GetSelectFileScreen()->FileChooser( "Design File", "*.des" );
            if ( newfile.size() != 0 && newfile[ newfile.size() - 1] != '/' )
            {
                DesignVarMgr.ReadDesVarsDES( newfile );
                RebuildAdjustTab();
            }
        }
        else
        {
            string newfile = m_ScreenMgr->GetSelectFileScreen()->FileChooser( "Cart3D XDDM File", "*.xddm" );
            if ( newfile.size() != 0 && newfile[ newfile.size() - 1] != '/' )
            {
                DesignVarMgr.ReadDesVarsXDDM( newfile );
                RebuildAdjustTab();
            }
        }
    }

    m_ScreenMgr->SetUpdateFlag( true );
}
