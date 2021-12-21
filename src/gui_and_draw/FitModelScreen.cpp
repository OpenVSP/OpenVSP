//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//

// FitModelScreen.cpp: implementation of the FitModelScreen class.
//
//////////////////////////////////////////////////////////////////////

#include "FitModelScreen.h"
#include "ParmMgr.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

FitModelScreen::FitModelScreen( ScreenMgr* mgr ) : TabScreen( mgr, 400, 469 + 107, "Fit Model", 107 )
{
    m_NVarLast = 0;

    m_FLTK_Window->callback( staticCloseCB, this );

    Fl_Group* pts_tab = AddTab( "Pick Points" );
    Fl_Group* var_tab = AddTab( "Pick Vars" );
    ( (Vsp_Group*) var_tab )->SetAllowDrop( true );
    var_tab->callback( staticScreenCB, this );

    Fl_Group* tree_tab = AddTab( "Var Tree" );
    ( (Vsp_Group*) tree_tab )->SetAllowDrop( true );
    tree_tab->callback( staticScreenCB, this );

    Fl_Group* fit_tab = AddTab( "Fit Model" );

    Fl_Group* saveLoad_tab = AddTab( "Save/Import" );

    pts_tab->show();

    // Set up pts tab

    Fl_Group* pts_group = AddSubGroup( pts_tab, 5 );
    m_PickPtsLayout.SetGroupAndScreen( pts_group, this );

    m_PickPtsLayout.AddDividerBox( "Point Selection" );

    m_PickPtsLayout.SetButtonWidth( 100 );
    m_PickPtsLayout.AddOutput( m_NSelOutput, "Num Selected" );

    m_PickPtsLayout.SetFitWidthFlag( false );
    m_PickPtsLayout.SetSameLineFlag( true );

    m_PickPtsLayout.SetButtonWidth( ( m_PickPtsLayout.GetRemainX() ) / 2 );
    m_PickPtsLayout.AddButton( m_SelOneButton, "Select One" );
    m_PickPtsLayout.AddButton( m_HideSelButton, "Hide Selection" );

    m_PickPtsLayout.ForceNewLine();

    m_PickPtsLayout.AddButton( m_SelBoxButton, "Select Region" );
    m_PickPtsLayout.AddButton( m_HideUnselButton, "Hide Unselected" );

    m_PickPtsLayout.ForceNewLine();

    m_PickPtsLayout.AddButton( m_SelAllButton, "Select All" );
    m_PickPtsLayout.AddButton( m_HideAllButton, "Hide All" );

    m_PickPtsLayout.ForceNewLine();

    m_PickPtsLayout.AddButton( m_SelNoneButton, "Select None" );
    m_PickPtsLayout.AddButton( m_HideNoneButton, "Show All" );

    m_PickPtsLayout.ForceNewLine();

    m_PickPtsLayout.AddButton( m_SelInvButton, "Invert Selection" );
    m_PickPtsLayout.AddButton( m_HideInvButton, "Invert Hidden" );

    m_PickPtsLayout.SetFitWidthFlag( true );
    m_PickPtsLayout.SetSameLineFlag( false );

    m_PickPtsLayout.ForceNewLine();

    m_PickPtsLayout.AddYGap();

    m_PickPtsLayout.AddDividerBox( "Target Points" );

    // Pointer for the widths of each column in the browser to support resizing
    // Last column width must be 0
    static int target_col_widths[] = { 90, 42, 42, 42, 42, 42, 42, 50, 0 }; // widths for each column

    int browser_h = 150;
    m_TargetPtBrowser = m_PickPtsLayout.AddColResizeBrowser( target_col_widths, 8, browser_h );
    m_TargetPtBrowser->callback( staticScreenCB, this );

    m_TargetGeomPicker.AddExcludeType( MESH_GEOM_TYPE );
    m_TargetGeomPicker.AddExcludeType( HUMAN_GEOM_TYPE );
    m_TargetGeomPicker.AddExcludeType( PT_CLOUD_GEOM_TYPE );
    m_TargetGeomPicker.AddExcludeType( WIRE_FRAME_GEOM_TYPE );
    m_TargetGeomPicker.AddExcludeType( BLANK_GEOM_TYPE );
    m_TargetGeomPicker.AddExcludeType( HINGE_GEOM_TYPE );
    m_PickPtsLayout.AddGeomPicker( m_TargetGeomPicker );

    m_PickPtsLayout.SetFitWidthFlag( false );
    m_PickPtsLayout.SetSameLineFlag( true );

    m_PickPtsLayout.SetButtonWidth( 50 );

    m_PickPtsLayout.AddButton( m_UFreeButton, "Free" );
    m_PickPtsLayout.AddButton( m_UFixButton, "Fix" );
    m_PickPtsLayout.SetFitWidthFlag( true );
    m_PickPtsLayout.AddSlider( m_USlider, "U", 1.0, "%7.3f" );
    m_PickPtsLayout.ForceNewLine();

    m_UToggleGroup.Init( this );
    m_UToggleGroup.AddButton( m_UFixButton.GetFlButton() );
    m_UToggleGroup.AddButton( m_UFreeButton.GetFlButton() );

    m_PickPtsLayout.SetFitWidthFlag( false );

    m_PickPtsLayout.AddButton( m_WFreeButton, "Free" );
    m_PickPtsLayout.AddButton( m_WFixButton, "Fix" );
    m_PickPtsLayout.SetFitWidthFlag( true );

    m_PickPtsLayout.AddSlider( m_WSlider, "W", 1.0, "%7.3f" );
    m_PickPtsLayout.ForceNewLine();

    m_WToggleGroup.Init( this );
    m_WToggleGroup.AddButton( m_WFixButton.GetFlButton() );
    m_WToggleGroup.AddButton( m_WFreeButton.GetFlButton() );

    m_PickPtsLayout.SetFitWidthFlag( false );

    m_PickPtsLayout.SetButtonWidth( ( m_PickPtsLayout.GetRemainX() ) / 3 );
    m_PickPtsLayout.AddButton( m_AddTargetPtButton, "Add Target" );
    m_PickPtsLayout.AddButton( m_DelTargetPtButton, "Delete Target" );
    m_PickPtsLayout.AddButton( m_ClearTargetPtButton, "Clear Target" );

    m_PickPtsLayout.ForceNewLine();
    m_PickPtsLayout.SetFitWidthFlag( true );
    m_PickPtsLayout.SetSameLineFlag( false );

    m_PickPtsLayout.SetButtonWidth( 100 );
    m_PickPtsLayout.AddOutput( m_NTgtOutput, "Num Target Pts." );

    // Set up vars tab

    Fl_Group* var_group = AddSubGroup( var_tab, 5 );
    m_PickVarLayout.SetGroupAndScreen( var_group, this );

    m_PickVarLayout.AddDividerBox( "Variable" );

    m_PickVarLayout.AddParmPicker( m_ParmPicker );

    m_PickVarLayout.AddYGap();

    m_PickVarLayout.SetFitWidthFlag( false );
    m_PickVarLayout.SetSameLineFlag( true );

    m_PickVarLayout.SetButtonWidth( ( m_PickVarLayout.GetRemainX() ) / 3 );
    m_PickVarLayout.AddButton( m_AddVarButton, "Add Variable" );
    m_PickVarLayout.AddButton( m_DelVarButton, "Delete Variable" );
    m_PickVarLayout.AddButton( m_ClearVarButton, "Clear Variables" );

    m_PickVarLayout.ForceNewLine();

    m_PickVarLayout.SetFitWidthFlag( true );
    m_PickVarLayout.SetSameLineFlag( false );

    m_PickVarLayout.SetButtonWidth( 100 );
    m_PickVarLayout.AddOutput( m_NVarOutput, "Num Variables" );

    m_PickVarLayout.AddYGap();

    m_PickVarLayout.AddDividerBox( "Variable List" );

    // Pointer for the widths of each column in the browser to support resizing
    // Last column width must be 0
    static int var_col_widths[] = { 120, 160, 120, 0 }; // widths for each column

    browser_h = 265;
    m_VarBrowser = m_PickVarLayout.AddColResizeBrowser( var_col_widths, 3, browser_h );
    m_VarBrowser->callback( staticScreenCB, this );

    // Set up tree tab

    Fl_Group* tree_group = AddSubGroup( tree_tab, 5 );
    m_PickTreeLayout.SetGroupAndScreen( tree_group, this );

    m_PickTreeLayout.AddDividerBox( "Variable Tree" );

    m_PickTreeLayout.AddParmTreePicker( m_ParmTreePicker, m_PickTreeLayout.GetW(), 370 );
    m_PickTreeLayout.AddButton( m_ClearVarButton2, "Clear Variables" );


    // Set up fit tab

    Fl_Group* fit_group = AddSubGroup( fit_tab, 5 );
    m_FitModelLayout.SetGroupAndScreen( fit_group, this );

    m_FitScrollGroup = AddSubScroll( fit_group, 5 );
    m_FitScrollGroup->type( Fl_Scroll::VERTICAL_ALWAYS );

    m_FitVarScrollLayout.SetGroupAndScreen( m_FitScrollGroup, this );


    // Set up optimization area at bottom of GUI.

    m_OptimLayout.SetGroupAndScreen( m_FLTK_Window, this );

    m_OptimLayout.AddY( m_OptimLayout.GetRemainY()
            - 4 * m_OptimLayout.GetStdHeight()
            - m_OptimLayout.GetGapHeight()
            - m_OptimLayout.GetDividerHeight() );

    m_OptimLayout.AddDividerBox( "Optimizer Control" );


    m_OptimLayout.SetFitWidthFlag( false );
    m_OptimLayout.SetSameLineFlag( true );

    m_OptimLayout.SetButtonWidth( ( m_OptimLayout.GetRemainX() ) / 4 );
    m_OptimLayout.SetInputWidth( ( m_OptimLayout.GetRemainX() ) / 4 );

    m_OptimLayout.AddOutput( m_DOFOutput, "DOF" );
    m_OptimLayout.AddOutput( m_CondOutput, "Conditions" );
    m_OptimLayout.ForceNewLine();



    m_OptimLayout.SetButtonWidth( ( m_OptimLayout.GetRemainX() ) / 2 );

    m_OptimLayout.AddButton( m_SearchUWButton, "Search UW" );
    m_OptimLayout.AddButton( m_RefineUWButton, "Refine UW" );

    m_OptimLayout.ForceNewLine();

    m_OptimLayout.AddButton( m_UpdateDistButton, "Update Distance" );
    m_OptimLayout.AddButton( m_OptimizeButton, "Fit" );

    m_OptimLayout.ForceNewLine();

    m_OptimLayout.SetFitWidthFlag( true );
    m_OptimLayout.SetSameLineFlag( false );

    m_OptimLayout.SetButtonWidth( 100 );
    m_OptimLayout.AddOutput( m_DistOutput, "Distance Metric" );

    //===== Save/Load Tab =====//
    Fl_Group* saveLoad_group = AddSubGroup( saveLoad_tab, 5 );
    m_FitModelLayout.SetGroupAndScreen( saveLoad_group, this );

    m_FitModelLayout.AddDividerBox("Save Targets & Variables");
    m_FitModelLayout.AddYGap();

    m_FitModelLayout.SetFitWidthFlag( false );
    m_FitModelLayout.SetSameLineFlag( true );

    m_FitModelLayout.SetButtonWidth( 75 );
    m_FitModelLayout.SetInputWidth( m_FitModelLayout.GetRemainX() - m_FitModelLayout.GetButtonWidth() - 50 );
    m_FitModelLayout.AddOutput(m_SaveOutput, "File Name:");
    m_FitModelLayout.SetButtonWidth( 50 );
    m_FitModelLayout.AddButton(m_SaveSelect, "...");
    m_FitModelLayout.ForceNewLine();
    m_FitModelLayout.AddYGap();

    m_FitModelLayout.SetFitWidthFlag( true );
    m_FitModelLayout.SetSameLineFlag( false );

    m_FitModelLayout.AddButton(m_Save, "Save");
    m_FitModelLayout.AddYGap();

    m_FitModelLayout.AddDividerBox("Import Targets & Variables");
    m_FitModelLayout.AddYGap();

    m_FitModelLayout.SetFitWidthFlag( false );
    m_FitModelLayout.SetSameLineFlag( true );

    m_FitModelLayout.SetButtonWidth( 75 );
    m_FitModelLayout.AddOutput(m_LoadOutput, "File Name:");
    m_FitModelLayout.SetButtonWidth( 50 );
    m_FitModelLayout.AddButton(m_LoadSelect, "...");
    m_FitModelLayout.ForceNewLine();
    m_FitModelLayout.AddYGap();

    m_FitModelLayout.SetFitWidthFlag( true );
    m_FitModelLayout.SetSameLineFlag( false );

    m_FitModelLayout.AddButton(m_Load, "Import");
}

FitModelScreen::~FitModelScreen()
{
}

bool FitModelScreen::Update()
{
    int i;
    int index;
    char str[256];

    Vehicle * veh = VehicleMgr.GetVehicle();

    // Update the number of selected points.
    sprintf( str, "%d", FitModelMgr.GetNumSelected() );
    m_NSelOutput.Update( str );

    m_TargetGeomPicker.Update();

    m_UToggleGroup.Update( veh->m_UType.GetID() );
    m_USlider.Update( veh->m_UTargetPt.GetID() );

    m_WToggleGroup.Update( veh->m_WType.GetID() );
    m_WSlider.Update( veh->m_WTargetPt.GetID() );

    m_SelOneButton.Update( veh->m_SelectOneFlag.GetID() );
    m_SelBoxButton.Update( veh->m_SelectBoxFlag.GetID() );

    // Update Fixed target point browser
    int h_pos = m_TargetPtBrowser->hposition();
    int v_pos = m_TargetPtBrowser->position();
    m_TargetPtBrowser->clear();

    m_TargetPtBrowser->column_char( ':' );         // use : as the column character

    sprintf( str, "@b@.GEOM:@b@c@.X:@b@c@.Y:@b@c@.Z:@b@c@.U:@b@c@.Type:@b@c@.W:@b@.Type" );
    m_TargetPtBrowser->add( str );

    int num_fix = FitModelMgr.GetNumTargetPt();
    for ( i = 0 ; i < num_fix ; i++ )
    {
        TargetPt* tpt = FitModelMgr.GetTargetPt( i );
        if( tpt )
        {
            Geom* g = veh->FindGeom( tpt->GetMatchGeom() );
            if( g )
            {
                string ut;
                string wt;

                if( tpt->GetUType() == TargetPt::FIXED )
                {
                    ut = string( "fix" );
                }
                else
                {
                    ut = string( "free" );
                }

                if( tpt->GetWType() == TargetPt::FIXED )
                {
                    wt = string( "fix" );
                }
                else
                {
                    wt = string( "free" );
                }

                sprintf( str, "%s:%4.2f:%4.2f:%4.2f:%4.2f:%s:%4.2f:%s", g->GetName().c_str(), tpt->GetPt().x(), tpt->GetPt().y(), tpt->GetPt().z(), tpt->GetUW().x(), ut.c_str(), tpt->GetUW().y(), wt.c_str() );
                m_TargetPtBrowser->add( str );
            }
        }
    }

    index = FitModelMgr.GetCurrTargetPtIndex();
    if ( index >= 0 && index < num_fix )
    {
        m_TargetPtBrowser->select( index + 2 );
    }

    m_TargetPtBrowser->hposition( h_pos );
    m_TargetPtBrowser->position( v_pos );

    sprintf( str, "%d", num_fix );
    m_NTgtOutput.Update( str );

    // Check that all Parms exist.  Needed in case a Geom with DesVars is
    // deleted.
    FitModelMgr.CheckVars();

    // Re-sort DesVars.  Needed in case a Geom's name is changed.
    if ( !FitModelMgr.SortVars() )
    {
        RebuildAdjustTab();
    }

    //==== Update Parm Picker ====//
    m_ParmPicker.Update();

    //==== Update Parm Tree Picker ====//
    m_ParmTreePicker.Update( FitModelMgr.GetVarVec() );

    //==== Update Parm Browser ====//
    h_pos = m_VarBrowser->hposition();
    v_pos = m_VarBrowser->position();
    m_VarBrowser->clear();

    m_VarBrowser->column_char( ':' );         // use : as the column character

    sprintf( str, "@b@.COMP_A:@b@.GROUP:@b@.PARM" );
    m_VarBrowser->add( str );

    int num_vars = FitModelMgr.GetNumVars();
    for ( i = 0 ; i < num_vars ; i++ )
    {
        string c_name, g_name, p_name;
        ParmMgr.GetNames( FitModelMgr.GetVar( i ), c_name, g_name, p_name );

        sprintf( str, "%s:%s:%s", c_name.c_str(), g_name.c_str(), p_name.c_str() );
        m_VarBrowser->add( str );
    }

    index = FitModelMgr.GetCurrVarIndex();
    if ( index >= 0 && index < num_vars )
    {
        m_VarBrowser->select( index + 2 );
    }

    m_VarBrowser->hposition( h_pos );
    m_VarBrowser->position( v_pos );

    sprintf( str, "%d", num_vars );
    m_NVarOutput.Update( str );

    // Parameter GUI got out of sync.  Probably from File->New or similar.
    if ( m_NVarLast != num_vars )
    {
        RebuildAdjustTab();
    }

    //==== Update Parm Adjust Tab ====//
    for ( i = 0 ; i < num_vars ; i++ )
    {
        m_ParmSliderVec[i].Update( FitModelMgr.GetVar( i ) );
    }

    FitModelMgr.UpdateNumOptVars();
    sprintf( str, "%d", FitModelMgr.GetNumOptVars() );
    m_DOFOutput.Update( str );
    sprintf( str, "%d", num_fix * 3 );
    m_CondOutput.Update( str );

    m_DistOutput.Update( std::to_string( static_cast<long double> (FitModelMgr.m_DistMetric) ) );

    //===== Save/Load =====//
    m_SaveOutput.Update( StringUtil::truncateFileName( FitModelMgr.GetSaveFitFileName(), 40 ) );
    m_LoadOutput.Update( StringUtil::truncateFileName( FitModelMgr.GetLoadFitFileName(), 40 ) );

    if ( m_SaveOutput.GetString() == "" )
    {
        m_Save.Deactivate();
    }
    else
    {
        m_Save.Activate();
    }
    if ( m_LoadOutput.GetString() == "" )
    {
        m_Load.Deactivate();
    }
    else
    {
        m_Load.Activate();
    }

    m_FLTK_Window->redraw();

    return false;
}

void FitModelScreen::RebuildAdjustTab()
{
    m_FitScrollGroup->clear();
    m_FitVarScrollLayout.SetGroup( m_FitScrollGroup );
    m_FitVarScrollLayout.InitWidthHeightVals();

    m_ParmSliderVec.clear();

    unsigned int num_vars = FitModelMgr.GetNumVars();
    m_ParmSliderVec.resize( num_vars );

    string lastContID;

    for ( int i = 0 ; i < num_vars ; i++ )
    {
        string pID = FitModelMgr.GetVar( i );

        Parm* p = ParmMgr.FindParm( pID );

        string contID = p->GetContainerID();

        if ( contID.compare( lastContID ) != 0 )
        {
            lastContID = contID;
            m_FitVarScrollLayout.AddDividerBox( ParmMgr.FindParmContainer( contID )->GetName() );
        }

        m_FitVarScrollLayout.AddSlider( m_ParmSliderVec[i], "AUTO_UPDATE", 10, "%7.3f" );
        m_ParmSliderVec[i].Update( pID );
    }

    m_NVarLast = num_vars;
}

void FitModelScreen::Show()
{
    Update();
    m_FLTK_Window->show();
    FitModelMgr.SetGUIShown( true );
    m_ScreenMgr->SetUpdateFlag( true );
}

void FitModelScreen::Hide()
{
    m_FLTK_Window->hide();
    FitModelMgr.SetGUIShown( false );
    m_ScreenMgr->SetUpdateFlag( true );
}

void FitModelScreen::CallBack( Fl_Widget* w )
{
    assert( m_ScreenMgr );
    Vehicle * veh = VehicleMgr.GetVehicle();

    if( Fl::event() == FL_PASTE || Fl::event() == FL_DND_RELEASE )
    {
        string ParmID( Fl::event_text() );
        FitModelMgr.AddVar( ParmID );
    }
    else if (  w == m_TargetPtBrowser )
    {
        int sel = m_TargetPtBrowser->value();
        FitModelMgr.SetCurrTargetPtIndex( sel - 2 );
        TargetPt* tpt = FitModelMgr.GetCurrTargetPt();

        if ( tpt )
        {
            veh->m_UType = tpt->GetUType();
            veh->m_WType = tpt->GetWType();

            veh->m_UTargetPt = tpt->GetUW().x();
            veh->m_WTargetPt = tpt->GetUW().y();

            m_TargetGeomPicker.SetGeomChoice( tpt->GetMatchGeom() );
        }
        else
        {
            m_TargetGeomPicker.SetGeomChoice( string() );
        }
    }
    else if (  w == m_VarBrowser )
    {
        int sel = m_VarBrowser->value();
        FitModelMgr.SetCurrVarIndex( sel - 2 );

        m_ParmPicker.SetParmChoice( FitModelMgr.GetCurrVar() );
    }

    m_ScreenMgr->SetUpdateFlag( true );
}

void FitModelScreen::CloseCallBack( Fl_Widget* w )
{
    Hide();
}

void FitModelScreen::GuiDeviceCallBack( GuiDevice* device )
{
    assert( m_ScreenMgr );

    if ( device == &m_SelOneButton )
    {
        FitModelMgr.SetSelectOne();
    }
    else if ( device == &m_SelBoxButton )
    {
        FitModelMgr.SetSelectBox();
    }
    else if ( device == &m_SelAllButton )
    {
        FitModelMgr.SelectAllShown();
    }
    else if ( device == &m_SelNoneButton )
    {
        FitModelMgr.SelectNone();
    }
    else if ( device == &m_SelInvButton )
    {
        FitModelMgr.SelectInv();
    }
    else if ( device == &m_HideSelButton )
    {
        FitModelMgr.HideSelection();
    }
    else if ( device == &m_HideUnselButton )
    {
        FitModelMgr.HideUnselected();
    }
    else if ( device == &m_HideAllButton )
    {
        FitModelMgr.HideAll();
    }
    else if ( device == &m_HideNoneButton )
    {
        FitModelMgr.ShowAll();
    }
    else if ( device == &m_HideInvButton )
    {
        FitModelMgr.HideInv();
    }
    else if ( device == &m_AddTargetPtButton )
    {
        FitModelMgr.AddSelectedPts( m_TargetGeomPicker.GetGeomChoice() );
        FitModelMgr.UpdateDist();
    }
    else if ( device == &m_DelTargetPtButton )
    {
        FitModelMgr.DelCurrTargetPt( );
        FitModelMgr.UpdateDist();
    }
    else if ( device == &m_ClearTargetPtButton )
    {
        FitModelMgr.DelAllTargetPts( );
        FitModelMgr.UpdateDist();
    }
    else if ( device == &m_AddVarButton )
    {
        FitModelMgr.AddCurrVar( );
        RebuildAdjustTab();
    }
    else if ( device == &m_DelVarButton )
    {
        FitModelMgr.DelCurrVar( );
        RebuildAdjustTab();
    }
    else if ( device == &m_ClearVarButton || device == &m_ClearVarButton2 )
    {
        FitModelMgr.DelAllVars( );
        RebuildAdjustTab();
    }
    else if ( device == &m_ParmPicker )
    {
        FitModelMgr.SetWorkingParmID( m_ParmPicker.GetParmChoice() );
        FitModelMgr.SetCurrVarIndex( -1 );
    }
    else if ( device == &m_SearchUWButton )
    {
        FitModelMgr.SearchTargetUW();
        FitModelMgr.UpdateDist();
    }
    else if ( device == &m_RefineUWButton )
    {
        FitModelMgr.RefineTargetUW();
        FitModelMgr.UpdateDist();
    }
    else if ( device == &m_UpdateDistButton )
    {
        FitModelMgr.UpdateDist();
    }
    else if ( device == &m_OptimizeButton )
    {
        m_OptimizeButton.SetColor( FL_RED );
        Fl::check();

        int info = FitModelMgr.Optimize();

        m_OptimizeButton.SetColor( FL_BACKGROUND_COLOR );
        Fl::check();

        if( info == 0 )
        {
            m_ScreenMgr->Alert( "Invalid inputs to least squares optimizer.\n"
                "Variables may not be independent, or may not change metric.");
        }
        FitModelMgr.UpdateDist();
    }
    else if ( device == &m_ParmTreePicker )
    {
        int eventtype = m_ParmTreePicker.GetEventType();
        if ( eventtype == ParmTreePicker::SELECT )
        {
            string ParmID  = m_ParmTreePicker.GetEventParm();
            FitModelMgr.AddVar( ParmID );
        }
        else if ( eventtype == ParmTreePicker::UNSELECT )
        {
            string ParmID  = m_ParmTreePicker.GetEventParm();
            FitModelMgr.DelVar( ParmID );
        }
    }
    else if ( device == &m_SaveSelect )
    {
        string newfile = m_ScreenMgr->GetSelectFileScreen()->FileChooser( "Create or Select .fit file.", "*.fit" );
        if ( newfile.compare( "" ) != 0 )
        {
            FitModelMgr.SetSaveFitFileName( newfile );
        }
    }
    else if ( device == &m_LoadSelect )
    {
        string newfile = m_ScreenMgr->GetSelectFileScreen()->FileChooser( "Select .fit file.", "*.fit" );
        if ( newfile.compare( "" ) != 0 )
        {
            FitModelMgr.SetLoadFitFileName( newfile );
        }
    }
    else if ( device == &m_Save )
    {
        FitModelMgr.Save();
    }
    else if ( device == &m_Load )
    {
        FitModelMgr.Load();
    }

    m_ScreenMgr->SetUpdateFlag( true );
}

bool FitModelScreen::IsVisible()
{
    return m_FLTK_Window->visible();
}

void FitModelScreen::LoadDrawObjs( vector< DrawObj* > & draw_obj_vec )
{
    // GUI not visible.
    if ( !IsVisible() )
    {
        return;
    }

    FitModelMgr.LoadDrawObjs( draw_obj_vec );
}
