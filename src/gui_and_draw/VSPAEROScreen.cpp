//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//

// VSPAEROScreen.cpp: implementation of the VSPAEROScreen class.
//
//////////////////////////////////////////////////////////////////////

#include "VSPAEROScreen.h"
#include "VSPAEROPlotScreen.h"
#include "APIDefines.h"
#include "StringUtil.h"
#include "FileUtil.h"
#include "float.h"
#include <FL/fl_ask.H>

#include <utility>
#include <string>
#include <algorithm>

#include <iostream>

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
#define VSPAERO_SCREEN_WIDTH 850
#define VSPAERO_SCREEN_HEIGHT 750

VSPAEROScreen::VSPAEROScreen( ScreenMgr* mgr ) : TabScreen( mgr, VSPAERO_SCREEN_WIDTH, VSPAERO_SCREEN_HEIGHT, "VSPAERO" )
{
    m_FLTK_Window->callback( staticCloseCB, this );

    m_SolverPair = make_pair( &VSPAEROMgr, this );  //solverpair type
    m_ViewerPair = make_pair( this, VSPAERO_VIEWER );   //monitorpair type

    int window_border_width = 5;
    int group_border_width = 2;

    int total_width = VSPAERO_SCREEN_WIDTH - 2 * window_border_width;
    int total_height = VSPAERO_SCREEN_HEIGHT - 2 * window_border_width;
    int row_height = 20;
    int action_button_height = 0 * row_height; //space reserved for action buttons at the bottom

    //==== Overview Tab ====//
    Fl_Group* overview_tab = AddTab( "Overview" );
    Fl_Group* overview_group = AddSubGroup( overview_tab, window_border_width );
    m_OverviewLayout.SetGroupAndScreen( overview_group, this );

    // Column layout
    GroupLayout left_col_layout;
    int left_col_width = 350 - 2 * group_border_width;
    int col_height = m_OverviewLayout.GetH() - group_border_width - action_button_height;
    m_OverviewLayout.AddSubGroupLayout( left_col_layout, left_col_width, col_height );

    m_OverviewLayout.AddX( left_col_layout.GetW() + 2 * group_border_width );

    GroupLayout right_col_layout;
    int right_col_width = m_OverviewLayout.GetRemainX();
    m_OverviewLayout.AddSubGroupLayout( right_col_layout, right_col_width, col_height );

    m_OverviewLayout.ForceNewLine();
    m_OverviewLayout.AddY( right_col_layout.GetH() );   //add Y for Execute divider box

    // Case Setup
    left_col_layout.AddSubGroupLayout( m_GeomLayout, left_col_layout.GetW() - 2 * group_border_width, 8 * row_height );
    left_col_layout.AddY( m_GeomLayout.GetH() );

    m_GeomLayout.AddDividerBox( "Case Setup" );

    m_GeomLayout.SetSameLineFlag( true );
    m_GeomLayout.SetFitWidthFlag( false );

    // Analysis method radio button group setup
    m_GeomLayout.SetButtonWidth( m_GeomLayout.GetW() / 2 );
    m_GeomLayout.AddButton( m_AeroMethodToggleVLM, "Vortex Lattice (VLM)" );
    m_GeomLayout.AddButton( m_AeroMethodTogglePanel, "Panel Method" );

    m_AeroMethodToggleGroup.Init( this );
    m_AeroMethodToggleGroup.AddButton( m_AeroMethodToggleVLM.GetFlButton() );
    m_AeroMethodToggleGroup.AddButton( m_AeroMethodTogglePanel.GetFlButton() );

    vector< int > val_map;
    val_map.push_back( vsp::VSPAERO_ANALYSIS_METHOD::VORTEX_LATTICE );
    val_map.push_back( vsp::VSPAERO_ANALYSIS_METHOD::PANEL );
    m_AeroMethodToggleGroup.SetValMapVec( val_map );

    m_GeomLayout.ForceNewLine();

    //  Degengeom output file selection, used for VLM & Panel methods
    int labelButtonWidth = 60;
    int fileButtonWidth = 25;
    int inputWidth = m_GeomLayout.GetW() - labelButtonWidth - fileButtonWidth;

    m_GeomLayout.SetButtonWidth( labelButtonWidth );
    m_GeomLayout.SetInputWidth( inputWidth );

    m_GeomLayout.AddOutput( m_DegenFileName, "Degen" );

    m_GeomLayout.SetButtonWidth( fileButtonWidth );

    m_GeomLayout.AddButton( m_DegenFileButton, "..." );
    m_GeomLayout.ForceNewLine();

    //  CompGeom output file selection, used for Panel method only
    m_GeomLayout.SetButtonWidth( labelButtonWidth );
    m_GeomLayout.SetInputWidth( inputWidth );

    m_GeomLayout.AddOutput( m_CompGeomFileName, "Panel" );

    m_GeomLayout.SetButtonWidth( fileButtonWidth );

    m_GeomLayout.AddButton( m_CompGeomFileButton, "..." );
    m_GeomLayout.ForceNewLine();

    m_GeomLayout.InitWidthHeightVals();
    m_GeomLayout.SetSameLineFlag( false );
    m_GeomLayout.SetFitWidthFlag( true );
    m_GeomLayout.AddChoice( m_GeomSetChoice, "Geometry Set:" );
    m_GeomLayout.AddSlider( m_NCPUSlider, "Num CPU", 10.0, "%3.0f" );
    m_GeomLayout.AddButton( m_StabilityCalcToggle, "Stability Calculation" );
    m_GeomLayout.AddButton( m_BatchCalculationToggle, "Batch Calculation" );

    left_col_layout.AddY( group_border_width );

    // Wake
    left_col_layout.AddSubGroupLayout( m_WakeLayout, left_col_layout.GetW() - 2 * group_border_width, 4 * row_height );
    left_col_layout.AddY( m_WakeLayout.GetH() );

    m_WakeLayout.AddDividerBox( "Wake" );
    m_WakeLayout.SetSameLineFlag( false );
    m_WakeLayout.SetFitWidthFlag( true );
    m_WakeLayout.AddSlider( m_WakeNumIterSlider, "Num It.", 10, "%3.0f" );
    m_WakeLayout.AddSlider( m_WakeAvgStartIterSlider, "Avg Start It.", 11, "%3.0f" );
    m_WakeLayout.AddSlider( m_WakeSkipUntilIterSlider, "Skip Until It.", 11, "%3.0f" );

    left_col_layout.AddY( group_border_width );

    // Reference Quantities
    left_col_layout.AddSubGroupLayout( m_RefLayout, left_col_layout.GetW() - 2 * group_border_width, 6 * row_height );
    left_col_layout.AddY( m_RefLayout.GetH() );

    m_RefLayout.AddDividerBox( "Reference Area, Lengths" );

    m_RefLayout.SetSameLineFlag( true );
    m_RefLayout.SetFitWidthFlag( false );

    m_RefLayout.SetButtonWidth( m_RefLayout.GetW() / 2 );

    m_RefLayout.AddButton( m_RefManualToggle, "Manual" );
    m_RefLayout.AddButton( m_RefChoiceToggle, "From Model" );
    m_RefLayout.ForceNewLine();

    m_RefLayout.InitWidthHeightVals();

    m_RefLayout.SetSameLineFlag( false );
    m_RefLayout.SetFitWidthFlag( true );

    m_RefLayout.AddChoice( m_RefWingChoice, "Ref. Wing" );

    m_RefLayout.AddSlider( m_SrefSlider, "Sref", 1000.0, "%7.3f" );
    m_RefLayout.AddSlider( m_brefSlider, "bref", 100.0, "%7.3f" );
    m_RefLayout.AddSlider( m_crefSlider, "cref", 100.0, "%7.3f" );

    m_RefToggle.Init( this );
    m_RefToggle.AddButton( m_RefManualToggle.GetFlButton() );
    m_RefToggle.AddButton( m_RefChoiceToggle.GetFlButton() );

    left_col_layout.AddY( group_border_width );

    // CG
    left_col_layout.AddSubGroupLayout( m_CGLayout, left_col_layout.GetW() - 2 * group_border_width, 6 * row_height );
    left_col_layout.AddY( m_CGLayout.GetH() );

    m_CGLayout.AddDividerBox( "Moment Reference Position" );

    m_CGLayout.SetButtonWidth( 125 );

    m_CGLayout.SetSameLineFlag( true );
    m_CGLayout.AddChoice( m_CGSetChoice, "Mass Set:",  m_CGLayout.GetButtonWidth() );
    m_CGLayout.SetFitWidthFlag( false );
    m_CGLayout.AddButton( m_MassPropButton, "Calc CG" );
    m_CGLayout.ForceNewLine();

    m_CGLayout.SetSameLineFlag( false );
    m_CGLayout.SetFitWidthFlag( true );

    m_CGLayout.InitWidthHeightVals();

    m_CGLayout.AddSlider( m_NumSliceSlider, "Num Slices", 100, "%4.0f" );
    m_CGLayout.AddSlider( m_XcgSlider, "Xref", 100.0, "%7.3f" );
    m_CGLayout.AddSlider( m_YcgSlider, "Yref", 100.0, "%7.3f" );
    m_CGLayout.AddSlider( m_ZcgSlider, "Zref", 100.0, "%7.3f" );

    left_col_layout.AddY( group_border_width );

    // Flow Condition
    left_col_layout.AddSubGroupLayout( m_FlowLayout, left_col_layout.GetW() - 2 * group_border_width, 4 * row_height );
    left_col_layout.AddY( m_FlowLayout.GetH() );

    m_FlowLayout.AddDividerBox( "Flow Condition" );
    m_FlowLayout.SetSameLineFlag( false );
    m_FlowLayout.SetFitWidthFlag( true );

    m_FlowLayout.AddInputEvenSpacedVector( m_AlphaStartInput, m_AlphaEndInput, m_AlphaNptsInput, "Alpha", "%7.3f" );
    m_FlowLayout.AddInputEvenSpacedVector( m_BetaStartInput, m_BetaEndInput, m_BetaNptsInput, "Beta", "%7.3f" );
    m_FlowLayout.AddInputEvenSpacedVector( m_MachStartInput, m_MachEndInput, m_MachNptsInput, "Mach", "%7.3f" );

    left_col_layout.AddY( group_border_width );

    // Execute
    left_col_layout.AddSubGroupLayout( m_ExecuteLayout, left_col_layout.GetW() - 2 * group_border_width, 7 * row_height );
    left_col_layout.AddY( m_ExecuteLayout.GetH() );

    m_ExecuteLayout.AddDividerBox( "Execute" );

    m_ExecuteLayout.AddButton( m_ComputeGeometryButton, "Generate Geometry" ); //This calls VSAERO.ComputeGeometry()

    m_ExecuteLayout.SetButtonWidth( m_ExecuteLayout.GetW() / 2 );

    m_ExecuteLayout.SetSameLineFlag( true );
    m_ExecuteLayout.SetFitWidthFlag( false );

    m_ExecuteLayout.AddButton( m_SetupButton, "Create New Setup" );
    m_ExecuteLayout.AddButton( m_KillSolverSetupButton, "Kill Setup" );

    m_ExecuteLayout.ForceNewLine();

    m_ExecuteLayout.AddButton( m_SolverButton, "Launch Solver" );
    m_ExecuteLayout.AddButton( m_KillSolverButton, "Kill Solver" );

    m_ExecuteLayout.ForceNewLine();

    m_ExecuteLayout.AddButton( m_PlotButton, "Show Results Mgr" );
    m_ExecuteLayout.AddButton( m_ViewerButton, "Launch Viewer" );

    m_ExecuteLayout.ForceNewLine();

    //m_ExecuteLayout.AddButton( m_ExportResultsToMatlabButton, "Export to *.m" );
    m_ExecuteLayout.AddButton( m_ExportResultsToCsvButton, "Export to *.csv" );
    m_ExportResultsToCsvButton.Deactivate();

    // Setup File
    right_col_layout.AddSubGroupLayout( m_SetupLayout, right_col_layout.GetW() - 2 * group_border_width, right_col_layout.GetH() - 2 * group_border_width );

    m_SetupDividerBox = m_SetupLayout.AddDividerBox( "Setup File: *.vspaero" );

    m_SetupEditor = m_SetupLayout.AddFlTextEditor( m_SetupLayout.GetRemainY() - m_SetupLayout.GetStdHeight() );

    m_SetupBuffer = new Fl_Text_Buffer;
    m_SetupEditor->buffer( m_SetupBuffer );
    m_SetupEditor->textfont( FL_COURIER );

    m_SetupLayout.SetButtonWidth( m_SetupLayout.GetW() / 2 );
    m_SetupLayout.SetSameLineFlag( true );
    m_SetupLayout.SetFitWidthFlag( false );
    m_SetupLayout.AddButton( m_ReadSetup, "Read Setup" );
    m_SetupLayout.AddButton( m_SaveSetup, "Save Setup" );


    //==== Solver Tab ====//
    Fl_Group* solver_tab = AddTab( "Solver Console" );
    Fl_Group* solver_group = AddSubGroup( solver_tab, window_border_width );
    m_SolverLayout.SetGroupAndScreen( solver_group, this );

    m_SolverLayout.AddDividerBox( "VSPAERO Solver Console" );

    m_SolverDisplay = m_SolverLayout.AddFlTextDisplay( m_SolverLayout.GetRemainY() - m_SolverLayout.GetStdHeight() );
    m_SolverBuffer = new Fl_Text_Buffer;
    m_SolverDisplay->buffer( m_SolverBuffer );


    //==== Viewer Tab ====//
    Fl_Group* viewer_tab = AddTab( "Viewer Console" );
    Fl_Group* viewer_group = AddSubGroup( viewer_tab, window_border_width );
    m_ViewerLayout.SetGroupAndScreen( viewer_group, this );

    m_ViewerLayout.AddDividerBox( "VSPAERO Viewer Console" );

    m_ViewerDisplay = m_ViewerLayout.AddFlTextDisplay( m_ViewerLayout.GetRemainY() - m_ViewerLayout.GetStdHeight() );
    m_ViewerBuffer = new Fl_Text_Buffer;
    m_ViewerDisplay->buffer( m_ViewerBuffer );


    // Show the starting tab
    overview_tab->show();

    // Flags to control Kill thread functionality
    m_SolverSetupThreadIsRunning = false;
    m_SolverThreadIsRunning = false;

    // String to enable detection of degen file name changes
    m_ModelNameBasePrevious = string();

}

VSPAEROScreen::~VSPAEROScreen()
{
}

bool VSPAEROScreen::Update()
{
    Vehicle *veh = VehicleMgr.GetVehicle();

    VSPAEROMgr.Update();

    if( veh )
    {
        //check if the degenfile name has changed
        string t_ModelNameBase = VSPAEROMgr.m_ModelNameBase;  //m_ModelNameBase is built from calling veh->getExportFileName();
        if( !t_ModelNameBase.empty() && strcmp( m_ModelNameBasePrevious.c_str(), t_ModelNameBase.c_str() ) != 0 )
        {
            ReadSetup();
        }
        m_ModelNameBasePrevious = t_ModelNameBase;

        // Reference Wing Choice
        //    find & list all Wing type geometries
        vector <string> geomVec = veh->GetGeomVec();

        m_RefWingChoice.ClearItems();
        m_WingGeomVec.clear();
        map <string, int> WingCompIDMap;
        int iwing = 0;
        for ( int i = 0 ; i < ( int )geomVec.size() ; i++ )
        {
            char str[256];
            Geom* g = veh->FindGeom( geomVec[i] );
            if ( g )
            {
                sprintf( str, "%d_%s", i, g->GetName().c_str() );

                if( g->GetType().m_Type == MS_WING_GEOM_TYPE )
                {
                    m_RefWingChoice.AddItem( str );
                    WingCompIDMap[ geomVec[i] ] = iwing;
                    m_WingGeomVec.push_back( geomVec[i] );
                    iwing ++;
                }
            }
        }
        m_RefWingChoice.UpdateItems();
        //    Update selected value
        string refGeomID = VSPAEROMgr.m_RefGeomID;
        if( refGeomID.length() == 0 && m_WingGeomVec.size() > 0 )
        {
            // Handle case default case.
            refGeomID = m_WingGeomVec[0];
            VSPAEROMgr.m_RefGeomID = refGeomID;
            // Re-trigger reference quantity update with default component.
            VSPAEROMgr.Update();
        }
        m_RefWingChoice.SetVal( WingCompIDMap[ refGeomID ] );


        // Update available set choices
        m_GeomSetChoice.ClearItems();
        m_CGSetChoice.ClearItems();

        vector <string> setVec = veh->GetSetNameVec();
        for ( int iSet = 0; iSet < setVec.size(); iSet++ )
        {
            m_GeomSetChoice.AddItem( setVec[iSet] );
            m_CGSetChoice.AddItem( setVec[iSet] );
        }
        m_GeomSetChoice.UpdateItems();
        m_CGSetChoice.UpdateItems();

        m_GeomSetChoice.SetVal( VSPAEROMgr.m_GeomSet() );
        m_CGSetChoice.SetVal( VSPAEROMgr.m_CGGeomSet() );


        // Case Setup
        m_AeroMethodToggleGroup.Update( VSPAEROMgr.m_AnalysisMethod.GetID() );
        switch ( VSPAEROMgr.m_AnalysisMethod.Get() )
        {
        case vsp::VSPAERO_ANALYSIS_METHOD::VORTEX_LATTICE:

            m_DegenFileName.Activate();
            m_DegenFileButton.Activate();

            m_CompGeomFileName.Deactivate();
            m_CompGeomFileButton.Deactivate();

            break;

        case vsp::VSPAERO_ANALYSIS_METHOD::PANEL:

            m_DegenFileName.Deactivate();
            m_DegenFileButton.Deactivate();

            m_CompGeomFileName.Activate();
            m_CompGeomFileButton.Activate();

            break;

        default:
            //do nothing; this should not be reachable
            break;
        }

        m_DegenFileName.Update( veh->getExportFileName( vsp::DEGEN_GEOM_CSV_TYPE ) );
        m_CompGeomFileName.Update( veh->getExportFileName( vsp::VSPAERO_PANEL_TRI_TYPE ) );

        m_NCPUSlider.Update( VSPAEROMgr.m_NCPU.GetID() );
        m_StabilityCalcToggle.Update( VSPAEROMgr.m_StabilityCalcFlag.GetID() );
        m_BatchCalculationToggle.Update( VSPAEROMgr.m_BatchModeFlag.GetID() );
        //printf("m_SolverProcess.m_ThreadID = %lu\n", m_SolverProcess.m_ThreadID);
        if( m_SolverThreadIsRunning )
        {
            m_ComputeGeometryButton.Deactivate();
        }
        else
        {
            m_ComputeGeometryButton.Activate();
        }


        // Wake Options
        m_WakeNumIterSlider.Update( VSPAEROMgr.m_WakeNumIter.GetID() );
        m_WakeAvgStartIterSlider.Update( VSPAEROMgr.m_WakeAvgStartIter.GetID() );
        m_WakeSkipUntilIterSlider.Update( VSPAEROMgr.m_WakeSkipUntilIter.GetID() );


        // Reference Quantities
        m_RefToggle.Update( VSPAEROMgr.m_RefFlag.GetID() );
        m_SrefSlider.Update( VSPAEROMgr.m_Sref.GetID() );
        m_brefSlider.Update( VSPAEROMgr.m_bref.GetID() );
        m_crefSlider.Update( VSPAEROMgr.m_cref.GetID() );


        // CG Position
        m_NumSliceSlider.Update( VSPAEROMgr.m_NumMassSlice.GetID() );
        m_XcgSlider.Update( VSPAEROMgr.m_Xcg.GetID() );
        m_YcgSlider.Update( VSPAEROMgr.m_Ycg.GetID() );
        m_ZcgSlider.Update( VSPAEROMgr.m_Zcg.GetID() );


        // Flow Condition
        // Alpha
        m_AlphaStartInput.Update( VSPAEROMgr.m_AlphaStart.GetID() );
        m_AlphaEndInput.Update( VSPAEROMgr.m_AlphaEnd.GetID() );
        m_AlphaNptsInput.Update( VSPAEROMgr.m_AlphaNpts.GetID() );
        if ( VSPAEROMgr.m_AlphaNpts.Get() == 1 )
        {
            m_AlphaEndInput.Deactivate();
        }
        else if ( VSPAEROMgr.m_AlphaNpts.Get() > 1 )
        {
            m_AlphaEndInput.Activate();
        }
        // Beta
        m_BetaStartInput.Update( VSPAEROMgr.m_BetaStart.GetID() );
        m_BetaEndInput.Update( VSPAEROMgr.m_BetaEnd.GetID() );
        m_BetaNptsInput.Update( VSPAEROMgr.m_BetaNpts.GetID() );
        if ( VSPAEROMgr.m_BetaNpts.Get() == 1 )
        {
            m_BetaEndInput.Deactivate();
        }
        else if ( VSPAEROMgr.m_BetaNpts.Get() > 1 )
        {
            m_BetaEndInput.Activate();
        }
        // Mach
        m_MachStartInput.Update( VSPAEROMgr.m_MachStart.GetID() );
        m_MachEndInput.Update( VSPAEROMgr.m_MachEnd.GetID() );
        m_MachNptsInput.Update( VSPAEROMgr.m_MachNpts.GetID() );
        if ( VSPAEROMgr.m_MachNpts.Get() == 1 )
        {
            m_MachEndInput.Deactivate();
        }
        else if ( VSPAEROMgr.m_MachNpts.Get() > 1 )
        {
            m_MachEndInput.Activate();
        }

        // Create Setup Button
        if( veh->GetVSPAEROCmd().empty()             ||
                !FileExist( VSPAEROMgr.m_DegenFileFull ) ||
                m_SolverThreadIsRunning                  ||
                m_SolverSetupThreadIsRunning             ||
                ( ( VSPAEROMgr.m_AnalysisMethod.Get() == vsp::VSPAERO_ANALYSIS_METHOD::PANEL ) && ( !FileExist( VSPAEROMgr.m_CompGeomFileFull ) ) ) )
        {
            m_SetupButton.Deactivate();
        }
        else
        {
            m_SetupButton.Activate();
        }
        // Kill Solver Setup Button
        if( m_SolverSetupThreadIsRunning )
        {
            m_KillSolverSetupButton.Activate();
        }
        else
        {
            m_KillSolverSetupButton.Deactivate();
        }

        // Setup Text Display
        m_SetupDividerBox->copy_label( std::string( "Setup File: " + GetFilename( VSPAEROMgr.m_SetupFile ) ).c_str() );
        // Read Setup Button
        if( !FileExist( VSPAEROMgr.m_SetupFile ) )
        {
            m_ReadSetup.Deactivate();
        }
        else
        {
            m_ReadSetup.Activate();
        }
        // Save Setup Button
        if( veh->GetVSPAEROCmd().empty() || m_SolverThreadIsRunning || m_SolverSetupThreadIsRunning )
        {
            m_SaveSetup.Deactivate();
        }
        else
        {
            m_SaveSetup.Activate();
        }

        // Solver Button
        if( veh->GetVSPAEROCmd().empty()             ||
                !FileExist( VSPAEROMgr.m_DegenFileFull ) ||
                !FileExist( VSPAEROMgr.m_SetupFile )     ||
                m_SolverThreadIsRunning                  ||
                ( VSPAEROMgr.m_AnalysisMethod.Get() == vsp::VSPAERO_ANALYSIS_METHOD::PANEL && !FileExist( VSPAEROMgr.m_CompGeomFileFull ) ) )
        {
            m_SolverButton.Deactivate();
        }
        else
        {
            m_SolverButton.Activate();
        }
        // Kill Solver Button
        if( m_SolverThreadIsRunning )
        {
            m_KillSolverButton.Activate();
        }
        else
        {
            m_KillSolverButton.Deactivate();
        }

        // Plot Window Button
        if( veh->GetVSPAEROCmd().empty()             ||
                !FileExist( VSPAEROMgr.m_DegenFileFull ) ||
                !FileExist( VSPAEROMgr.m_SetupFile )     ||
                ( VSPAEROMgr.m_AnalysisMethod.Get() == vsp::VSPAERO_ANALYSIS_METHOD::PANEL && !FileExist( VSPAEROMgr.m_CompGeomFileFull ) ) )
        {
            m_PlotButton.Deactivate();
        }
        else
        {
            m_PlotButton.Activate();
        }

        // Viewer Button
        if( veh->GetVIEWERCmd().empty() || m_SolverThreadIsRunning || m_ViewerProcess.IsRunning() || !FileExist( VSPAEROMgr.m_AdbFile ) )
        {
            m_ViewerButton.Deactivate();
        }
        else
        {
            m_ViewerButton.Activate();
        }

        // Export Button
        if ( ResultsMgr.GetNumResults( "VSPAERO_Wrapper" ) == 0 )
        {
            m_ExportResultsToCsvButton.Deactivate();
        }
        else
        {
            m_ExportResultsToCsvButton.Activate();
        }

    }

    m_FLTK_Window->redraw();

    return false;
}


void VSPAEROScreen::Show()
{
    m_ScreenMgr->SetUpdateFlag( true );
    m_FLTK_Window->show();
}

void VSPAEROScreen::Hide()
{
    m_FLTK_Window->hide();
    m_ScreenMgr->SetUpdateFlag( true );
}


void VSPAEROScreen::CallBack( Fl_Widget* w )
{
    m_ScreenMgr->SetUpdateFlag( true );
}

void VSPAEROScreen::CloseCallBack( Fl_Widget* w )
{
    Hide();
}

#ifdef WIN32
DWORD WINAPI monitorfun( LPVOID data )
#else
void * monitorfun( void *data )
#endif
{
    monitorpair *mp = ( monitorpair * ) data;

    VSPAEROScreen *vs = mp->first;
    int id = mp->second;

    if( vs )
    {
        Fl_Text_Display *display = vs->GetDisplay( id );
        ProcessUtil *pu = vs->GetProcess( id );
        if( pu && display )
        {
            int bufsize = 1000;
            char *buf;
            buf = ( char* ) malloc( sizeof( char ) * ( bufsize + 1 ) );

            unsigned long nread = 1;

            bool runflag = pu->IsRunning();
            while( runflag || nread > 0 )
            {
                nread = 0;
                pu->ReadStdoutPipe( buf, bufsize, &nread );

                if( nread > 0 )
                {
                    buf[nread] = 0;
                    StringUtil::change_from_to( buf, '\r', '\n' );

                    Fl::lock();
                    // Any FL calls must occur between Fl::lock() and Fl::unlock().
                    vs->AddOutputText( display, buf );
                    Fl::unlock();
                }

                if( runflag )
                {
                    SleepForMilliseconds( 100 );
                }
                runflag = pu->IsRunning();
            }

#ifdef WIN32
            CloseHandle( pu->m_StdoutPipe[0] );
            pu->m_StdoutPipe[0] = NULL;
#else
            close( pu->m_StdoutPipe[0] );
            pu->m_StdoutPipe[0] = -1;
#endif
            vs->GetScreenMgr()->SetUpdateFlag( true );

            free( buf );
        }
    }
    return 0;
}

#ifdef WIN32
DWORD WINAPI solver_setup_thread_fun( LPVOID data )
#else
void * solver_setup_thread_fun( void *data )
#endif
{
    solverpair *sp = ( solverpair * ) data;

    VSPAEROMgrSingleton* vsmgr = sp->first;
    VSPAEROScreen *vsscreen = sp->second;

    if( vsmgr && vsscreen )
    {
        vsscreen->m_SolverSetupThreadIsRunning = true;

        // EXECUTE SOLVER
        vsmgr->CreateSetupFile();

        // Read setup file
        vsscreen->ReadSetup();

        vsscreen->m_SolverSetupThreadIsRunning = false;

        vsscreen->GetScreenMgr()->SetUpdateFlag( true );
    }

    return 0;
}

#ifdef WIN32
DWORD WINAPI solver_thread_fun( LPVOID data )
#else
void * solver_thread_fun( void *data )
#endif
{
    solverpair *sp = ( solverpair * ) data;

    VSPAEROMgrSingleton* vsmgr = sp->first;
    VSPAEROScreen *vsscreen = sp->second;

    if( vsmgr && vsscreen )
    {
        vsscreen->m_SolverThreadIsRunning = true;

        // EXECUTE SOLVER
        vsmgr->ComputeSolver();

        vsscreen->m_SolverThreadIsRunning = false;

        vsscreen->GetScreenMgr()->SetUpdateFlag( true );
    }

    return 0;
}


void VSPAEROScreen::GuiDeviceCallBack( GuiDevice* device )
{
    assert( m_ScreenMgr );

    Vehicle *veh = VehicleMgr.GetVehicle();

    if( veh )
    {
        //TODO add callback to determine if the setup file text has been edited

        if ( device == &m_SetupButton )
        {
            if( veh->GetVSPAEROCmd().empty() || !FileExist( VSPAEROMgr.m_DegenFileFull ) || VSPAEROMgr.IsSolverRunning() )
            { /* Do nothing. Should not be reachable, button should be deactivated.*/ }
            else
            {
                if ( FileExist( VSPAEROMgr.m_SetupFile ) )
                {
                    switch( fl_choice( "Overwrite any existing setup file?", "Cancel", "Abort", "Overwrite" ) )
                    {
                    case( 0 ):
                    case ( 1 ):
                        break;
                    case( 2 ):
                        // Clear the solver console
                        m_SolverBuffer->text( "" );

                        m_SolverProcess.StartThread( solver_setup_thread_fun, ( void* ) &m_SolverPair );
                        break;
                    }
                }
                else
                {
                    // Clear the solver console
                    m_SolverBuffer->text( "" );

                    m_SolverProcess.StartThread( solver_setup_thread_fun, ( void* ) &m_SolverPair );
                }
            }
        }
        else if ( device == &m_SolverButton )
        {
            if( veh->GetVSPAEROCmd().empty() || !FileExist( VSPAEROMgr.m_DegenFileFull ) || VSPAEROMgr.IsSolverRunning() )
            { /* Do nothing. Should not be reachable, button should be deactivated.*/ }
            else
            {
                // Clear out previous results
                VSPAEROMgr.ClearAllPreviousResults();

                // Clear the solver console
                m_SolverBuffer->text( "" );

                //Show the plot screen
                m_ScreenMgr->m_ShowPlotScreenOnce = true;   //deferred show of plot screen

                VSPAEROPlotScreen * vspapscreen = ( VSPAEROPlotScreen * )m_ScreenMgr->GetScreen( ScreenMgr::VSP_VSPAERO_PLOT_SCREEN );
                if( vspapscreen )
                {
                    vspapscreen->SetDefaultView();
                    vspapscreen->Update();
                }

                m_SolverProcess.StartThread( solver_thread_fun, ( void* ) &m_SolverPair );

            }
        }
        else if ( device == &m_ViewerButton )
        {
            if( veh->GetVIEWERCmd().empty() || !FileExist( VSPAEROMgr.m_DegenFileFull ) || m_ViewerProcess.IsRunning() )
            { /* Do nothing. Should not be reachable, button should be deactivated.*/ }
            else
            {
                vector<string> args;
                args.push_back( VSPAEROMgr.m_ModelNameBase );

                m_ViewerProcess.ForkCmd( veh->GetExePath(), veh->GetVIEWERCmd(), args );

                m_ViewerBuffer->text( "" );
                m_ViewerMonitor.StartThread( monitorfun, ( void* ) &m_ViewerPair );

            }
        }
        else if( device == &m_KillSolverSetupButton )
        {
            VSPAEROMgr.KillSolver();
        }
        else if( device == &m_KillSolverButton )
        {
            VSPAEROMgr.KillSolver();
        }
        else if( device == &m_PlotButton )
        {
            m_ScreenMgr->m_ShowPlotScreenOnce = true;   //deferred show of plot screen
        }
        else if( device == &m_SaveSetup )
        {
            SaveSetup();
        }
        else if( device == &m_ReadSetup )
        {
            ReadSetup();
        }
        else if( device == &m_RefWingChoice )
        {
            int id = m_RefWingChoice.GetVal();
            VSPAEROMgr.m_RefGeomID = m_WingGeomVec[id];
        }
        else if( device == &m_GeomSetChoice )
        {
            VSPAEROMgr.m_GeomSet = m_GeomSetChoice.GetVal();
        }
        else if( device == &m_ComputeGeometryButton )
        {
            VSPAEROMgr.ComputeGeometry();
        }
        else if( device == &m_DegenFileButton )
        {
            veh->setExportFileName( vsp::DEGEN_GEOM_CSV_TYPE, m_ScreenMgr->GetSelectFileScreen()->FileChooser( "Select degen geom CSV output file.", "*.csv" ) );
        }
        else if( device == &m_CompGeomFileButton )
        {
            veh->setExportFileName( vsp::VSPAERO_PANEL_TRI_TYPE, m_ScreenMgr->GetSelectFileScreen()->FileChooser( "Select comp geom TRI output file.", "*.tri" ) );
        }
        else if( device == &m_CGSetChoice )
        {
            VSPAEROMgr.m_CGGeomSet = m_CGSetChoice.GetVal();
        }
        else if( device == &m_MassPropButton )
        {
            string id = veh->MassPropsAndFlatten( m_CGSetChoice.GetVal(), VSPAEROMgr.m_NumMassSlice(), false, false );
            veh->DeleteGeom( id );

            VSPAEROMgr.m_Xcg = veh->m_CG.x();
            VSPAEROMgr.m_Ycg = veh->m_CG.y();
            VSPAEROMgr.m_Zcg = veh->m_CG.z();
        }
        else if( device == &m_ExportResultsToCsvButton )
        {
            string fileName = m_ScreenMgr->GetSelectFileScreen()->FileChooser( "Select CSV File", "*.csv" );
            if ( fileName.size() > 0 )
            {
                int status = VSPAEROMgr.ExportResultsToCSV( fileName );
                if ( status != vsp::VSP_OK )
                {
                    char strBuf[1000];
                    sprintf( strBuf, "File export failed\nFile: %s", fileName.c_str() );
                    fl_alert( strBuf );
                }
            }
        }
    }
    m_ScreenMgr->SetUpdateFlag( true );
}

// VSPAEROScreen::AddOutputText( Fl_Text_Display *display, const char *text )
//     This is used for the Solver tab to show the current results of the solver in the GUI
void VSPAEROScreen::AddOutputText( Fl_Text_Display *display, const string &text )
{
    if ( display )
    {
        // Added lock(), unlock() calls to avoid heap corruption while updating the text and rapidly scrolling with the mouse wheel inside the text display
        Fl::lock();

        display->buffer()->append( text.c_str() );
        display->insert_position( display->buffer()->length() );

        display->show_insert_position();

        Fl::unlock();
    }
}

ProcessUtil* VSPAEROScreen::GetProcess( int id )
{
    if( id == VSPAERO_SOLVER )
    {
        return &m_SolverProcess;
    }
    else if( id == VSPAERO_VIEWER )
    {
        return &m_ViewerProcess;
    }
    else if( id == VSPAERO_SOLVER_MONITOR )
    {
        return &m_SolverMonitor;
    }
    else
    {
        return NULL;
    }
}

Fl_Text_Display* VSPAEROScreen::GetDisplay( int id )
{
    if( id == VSPAERO_SOLVER )
    {
        return m_SolverDisplay;
    }
    else if( id == VSPAERO_VIEWER )
    {
        return m_ViewerDisplay;
    }
    else
    {
        return NULL;
    }
}

void VSPAEROScreen::ReadSetup()
{
    int loadStatus = m_SetupBuffer->loadfile( VSPAEROMgr.m_SetupFile.c_str() );
}

void VSPAEROScreen::SaveSetup()
{
    m_SetupBuffer->savefile( VSPAEROMgr.m_SetupFile.c_str() );
}
