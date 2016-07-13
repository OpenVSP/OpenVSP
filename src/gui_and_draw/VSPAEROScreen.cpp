//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//

// VSPAEROScreen.cpp: implementation of the VSPAEROScreen class.
//
//////////////////////////////////////////////////////////////////////

#include "VSPAEROScreen.h"
#include "APIDefines.h"
#include "StringUtil.h"
#include "FileUtil.h"
#include "float.h"

#include <utility>
#include <string>
#include <algorithm>

#include <iostream>

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

VSPAEROScreen::VSPAEROScreen( ScreenMgr* mgr ) : TabScreen( mgr, 650, 390, "VSPAERO" )
{
    m_SolverPair = make_pair( this, VSPAERO_SOLVER );
    m_ViewerPair = make_pair( this, VSPAERO_VIEWER );
    
    //==== Overview Tab ====//
    Fl_Group* overview_tab = AddTab( "Overview" );
    Fl_Group* setup_tab = AddTab( "Setup" );
    Fl_Group* solver_tab = AddTab( "Solver Console" );
    Fl_Group* viewer_tab = AddTab( "Viewer Console" );

    overview_tab->show();

    int window_boarder_width = 5;
    Fl_Group* overview_group = AddSubGroup( overview_tab, window_boarder_width );
    m_OverviewLayout.SetGroupAndScreen( overview_group, this );

    int group_boarder_width = 2;
    int column_width = m_OverviewLayout.GetW()/2 - group_boarder_width;
    int row_height = 20;
    int n_rows = 8;
    m_OverviewLayout.AddSubGroupLayout( m_GeomLayout, column_width, n_rows*row_height );
    m_OverviewLayout.AddX( column_width + 2*group_boarder_width );
    m_OverviewLayout.AddSubGroupLayout( m_FlowLayout, column_width, n_rows*row_height );
    m_OverviewLayout.ForceNewLine();
    m_OverviewLayout.AddY( (n_rows-1)*row_height );
    n_rows = 6;
    m_OverviewLayout.AddSubGroupLayout( m_RefLayout, column_width, n_rows*row_height);
    m_OverviewLayout.AddX( column_width + 2*group_boarder_width );
    m_OverviewLayout.AddSubGroupLayout( m_CGLayout, column_width, n_rows*row_height);
    m_OverviewLayout.ForceNewLine();
    m_OverviewLayout.AddY( (n_rows-1)*row_height);

    m_GeomLayout.AddDividerBox( "Case Setup" );

    m_GeomLayout.SetSameLineFlag( true );
    m_GeomLayout.SetFitWidthFlag( false );

    m_GeomLayout.SetButtonWidth( 50 );
    m_GeomLayout.SetInputWidth( column_width - 50 - 25 );

    m_GeomLayout.AddOutput( m_DegenFileName, "File" );

    m_GeomLayout.SetButtonWidth( 25 );

    m_GeomLayout.AddButton( m_DegenFileButton, "..." );
    m_GeomLayout.ForceNewLine();

    m_GeomLayout.SetButtonWidth( 125 );

    m_GeomLayout.SetFitWidthFlag( true );
    m_GeomLayout.AddChoice( m_GeomSetChoice, "Geometry Set:", m_GeomLayout.GetButtonWidth() );
    m_GeomLayout.SetFitWidthFlag( false );
    m_GeomLayout.AddButton( m_DegenGeomButton, "Generate Geometry" );
    m_GeomLayout.ForceNewLine();

    m_GeomLayout.InitWidthHeightVals();
    m_GeomLayout.SetButtonWidth( 125 );
    m_GeomLayout.SetSameLineFlag( false );
    m_GeomLayout.SetFitWidthFlag( true );
    m_GeomLayout.AddSlider( m_NCPUSlider, "Num CPU", 10.0, "%3.0f" );
    m_GeomLayout.AddSlider( m_WakeNumIterSlider, "Wake Num Iter", 10, "%3.0f" ); 
    m_GeomLayout.AddSlider( m_WakeAvgStartIterSlider, "Wake Avg Start Iter", 11, "%3.0f" );
    m_GeomLayout.AddSlider( m_WakeSkipUntilIterSlider, "Wake Skip Until Iter", 11, "%3.0f" );
    m_GeomLayout.AddButton( m_StabilityCalcToggle, "Stability Calculation" );

    // Flow Condition
    m_FlowLayout.AddDividerBox( "Flow Condition" );
    m_FlowLayout.SetSameLineFlag( false );
    m_FlowLayout.SetFitWidthFlag( true );

    m_FlowLayout.AddInputEvenSpacedVector(m_AlphaStartInput,m_AlphaEndInput,m_AlphaNptsInput,"Alpha","%7.3f");
    m_FlowLayout.AddInputEvenSpacedVector(m_BetaStartInput,m_BetaEndInput,m_BetaNptsInput,"Beta","%7.3f");
    m_FlowLayout.AddInputEvenSpacedVector(m_MachStartInput,m_MachEndInput,m_MachNptsInput,"Mach","%7.3f");


    // Reference Quantities
    m_RefLayout.AddDividerBox( "Reference Quantities" );

    m_RefLayout.SetSameLineFlag( true );
    m_RefLayout.SetFitWidthFlag( false );

    m_RefLayout.SetButtonWidth( m_RefLayout.GetW()/2 );

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


    // CG
    m_CGLayout.AddDividerBox( "CG Position" );

    m_CGLayout.SetButtonWidth( 125 );

    m_CGLayout.SetSameLineFlag( true );
    m_CGLayout.AddChoice( m_CGSetChoice, "Mass Prop Set:",  m_CGLayout.GetButtonWidth() );
    m_CGLayout.SetFitWidthFlag( false );
    m_CGLayout.AddButton( m_MassPropButton, "Calc Mass Prop" );
    m_CGLayout.ForceNewLine();

    m_CGLayout.SetSameLineFlag( false );
    m_CGLayout.SetFitWidthFlag( true );

    m_CGLayout.InitWidthHeightVals();

    m_CGLayout.AddSlider( m_NumSliceSlider, "Num Slices", 100, "%4.0f" );
    m_CGLayout.AddSlider( m_XcgSlider, "Xcg", 100.0, "%7.3f" );
    m_CGLayout.AddSlider( m_YcgSlider, "Ycg", 100.0, "%7.3f" );
    m_CGLayout.AddSlider( m_ZcgSlider, "Zcg", 100.0, "%7.3f" );


    // Execute
    m_OverviewLayout.AddDividerBox( "Execute VSPAERO" );

    m_OverviewLayout.SetButtonWidth( m_OverviewLayout.GetW()/2 );

    m_OverviewLayout.SetSameLineFlag( true );
    m_OverviewLayout.SetFitWidthFlag( false );

    m_OverviewLayout.AddButton( m_SetupButton, "Setup Input File" );
    m_OverviewLayout.AddButton( m_ViewerButton, "Launch Viewer" );

    m_OverviewLayout.ForceNewLine();

    m_OverviewLayout.AddButton( m_SolverButton, "Launch Solver" );
    m_OverviewLayout.AddButton( m_KillSolverButton, "Kill Solver" );


    //==== Setup Tab ====//
    Fl_Group* setup_group = AddSubGroup( setup_tab, 5 );
    m_SetupLayout.SetGroupAndScreen( setup_group, this );

    m_SetupLayout.AddDividerBox( "VSPAERO Setup *.vspaero File" );

    m_SetupEditor = m_SetupLayout.AddFlTextEditor( 245 );

    m_SetupBuffer = new Fl_Text_Buffer;
    m_SetupEditor->buffer( m_SetupBuffer );
    m_SetupEditor->textfont( FL_COURIER );

    m_SetupLayout.SetButtonWidth( m_SetupLayout.GetW()/2 );
    m_SetupLayout.SetSameLineFlag( true );
    m_SetupLayout.SetFitWidthFlag( false );
    m_SetupLayout.AddButton( m_SaveSetup, "Save Setup" );
    m_SetupLayout.AddButton( m_ReadSetup, "Read Setup" );

    //==== Solver Tab ====//
    Fl_Group* solver_group = AddSubGroup( solver_tab, 5 );
    m_SolverLayout.SetGroupAndScreen( solver_group, this );

    m_SolverLayout.AddDividerBox( "VSPAERO Solver Console" );
    
    m_SolverDisplay = m_SolverLayout.AddFlTextDisplay( 260 );
    m_SolverBuffer = new Fl_Text_Buffer;
    m_SolverDisplay->buffer( m_SolverBuffer );


    //==== Viewer Tab ====//
    Fl_Group* viewer_group = AddSubGroup( viewer_tab, 5 );
    m_ViewerLayout.SetGroupAndScreen( viewer_group, this );

    m_ViewerLayout.AddDividerBox( "VSPAERO Viewer Console" );
    
    m_ViewerDisplay = m_ViewerLayout.AddFlTextDisplay( 260 );
    m_ViewerBuffer = new Fl_Text_Buffer;
    m_ViewerDisplay->buffer( m_ViewerBuffer );

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


        m_GeomSetChoice.ClearItems();
        m_CGSetChoice.ClearItems();

        vector <string> setVec = veh->GetSetNameVec();
        for ( int i = 0; i < setVec.size(); i++ )
        {
            m_GeomSetChoice.AddItem( setVec[i] );
            m_CGSetChoice.AddItem( setVec[i] );
        }
        m_GeomSetChoice.UpdateItems();
        m_CGSetChoice.UpdateItems();

        m_GeomSetChoice.SetVal( VSPAEROMgr.m_GeomSet() );
        m_CGSetChoice.SetVal( VSPAEROMgr.m_CGGeomSet() );
        
        m_DegenFileName.Update( veh->getExportFileName( vsp::DEGEN_GEOM_CSV_TYPE ) );

        m_NCPUSlider.Update( VSPAEROMgr.m_NCPU.GetID() );
        m_WakeNumIterSlider.Update( VSPAEROMgr.m_WakeNumIter.GetID() );
        m_WakeAvgStartIterSlider.Update( VSPAEROMgr.m_WakeAvgStartIter.GetID() );
        m_WakeSkipUntilIterSlider.Update( VSPAEROMgr.m_WakeSkipUntilIter.GetID() );
        m_StabilityCalcToggle.Update( VSPAEROMgr.m_StabilityCalcFlag.GetID() );


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
        if ( VSPAEROMgr.m_AlphaNpts.Get()==1 )
        {
            m_AlphaEndInput.Deactivate();
        }
        else if ( VSPAEROMgr.m_AlphaNpts.Get()>1 )
        {
            m_AlphaEndInput.Activate();
        }
        // Beta
        m_BetaStartInput.Update( VSPAEROMgr.m_BetaStart.GetID() );
        m_BetaEndInput.Update( VSPAEROMgr.m_BetaEnd.GetID() );
        m_BetaNptsInput.Update( VSPAEROMgr.m_BetaNpts.GetID() );
        if ( VSPAEROMgr.m_BetaNpts.Get()==1 )
        {
            m_BetaEndInput.Deactivate();
        }
        else if ( VSPAEROMgr.m_BetaNpts.Get()>1 )
        {
            m_BetaEndInput.Activate();
        }
        // Mach
        m_MachStartInput.Update( VSPAEROMgr.m_MachStart.GetID() );
        m_MachEndInput.Update( VSPAEROMgr.m_MachEnd.GetID() );
        m_MachNptsInput.Update( VSPAEROMgr.m_MachNpts.GetID() );
        if ( VSPAEROMgr.m_MachNpts.Get()==1 )
        {
            m_MachEndInput.Deactivate();
        } 
        else if ( VSPAEROMgr.m_MachNpts.Get()>1 )
        {
            m_MachEndInput.Activate();
        } 

        // Kill Solver Button
        if( VSPAEROMgr.IsSolverRunning() )
        {
            m_KillSolverButton.Activate();
        }
        else
        {
            m_KillSolverButton.Deactivate();
        }

        // Setup Button
        if( veh->GetVSPAEROCmd().empty() || VSPAEROMgr.m_DegenFile.empty() || VSPAEROMgr.IsSolverRunning() )
        {
            m_SetupButton.Deactivate();
        }
        else
        {
            m_SetupButton.Activate();
        }

        // Solver Button
        if( veh->GetVSPAEROCmd().empty() || VSPAEROMgr.m_DegenFile.empty() || VSPAEROMgr.IsSolverRunning() || !FileExist(VSPAEROMgr.m_SetupFile) )
        {
            m_SolverButton.Deactivate();
        }
        else
        {
            m_SolverButton.Activate();
        }

        // Viewer Button
        if( veh->GetVIEWERCmd().empty() || m_ViewerProcess.IsRunning() || !FileExist(VSPAEROMgr.m_AdbFile))
        {
            m_ViewerButton.Deactivate();
        }
        else
        {
            m_ViewerButton.Activate();
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
    m_ScreenMgr->SetUpdateFlag( false );
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
    monitorpair *mp = (monitorpair *) data;

    VSPAEROScreen *vs = mp->first;
    int id = mp->second;

    if(vs)
    {
        Fl_Text_Display *display = vs->GetDisplay( id );
        ProcessUtil *pu = vs->GetProcess( id );
        if( pu && display )
        {
            int bufsize = 1000;
            char *buf;
            buf = (char*) malloc( sizeof( char ) * (bufsize + 1) );

            unsigned long nread = 1;

            bool runflag = pu->IsRunning();
            while( runflag || nread > 0)
            {
                pu->ReadStdoutPipe( buf, bufsize, &nread);

                if( nread > 0 )
                {
                    buf[nread] = 0;
                    StringUtil::change_from_to( buf, '\r', '\n' );

                    Fl::lock();
                    // Any FL calls must occur between Fl::lock() and Fl::unlock().
                    vs->AddOutputText( display, buf );
                    Fl::unlock();
                    // printf( "%s", buf );
                }

                if( runflag )
                {
                    SleepForMilliseconds(200);
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
DWORD WINAPI solver_monitor_thread_fun( LPVOID data )
#else
void * solver_monitor_thread_fun( void *data )
#endif
{
    printf("Monitor: The solver monitor has started\n");

    VSPAEROMgrSingleton* vsmgr = (VSPAEROMgrSingleton*) data;

    int bufsize = 1000;
    char *buf;
    buf = (char*) malloc( sizeof( char ) * (bufsize + 1) );
    unsigned long nread = 1;
    while (1)
    {
        vsmgr->m_SolverProcess.ReadStdoutPipe( buf, bufsize, &nread);

        if( nread > 0 )
        {
            buf[nread] = 0;
            StringUtil::change_from_to( buf, '\r', '\n' );

            printf( "%s", buf );
            
            /*
            //TODO printing to the VSPAERO GUI back, need to pass in VSPAEROScreen object
            Fl::lock();
            // Any FL calls must occur between Fl::lock() and Fl::unlock().
            vs->AddOutputText( display, buf );
            Fl::unlock();
            */
        }
        SleepForMilliseconds(200);
    }

    return 0;
}

#ifdef WIN32
DWORD WINAPI solver_thread_fun( LPVOID data )
#else
void * solver_thread_fun( void *data )
#endif
{
    printf("Solver: The solver has started\n");

    VSPAEROMgrSingleton* vsmgr = (VSPAEROMgrSingleton*) data;

    // Start Monitor
    ProcessUtil monitor;
    monitor.StartThread( solver_monitor_thread_fun, data );

    vsmgr->ComputeSolver();

    // Kill Monitor
    monitor.Kill();

    printf("Solver: The solver is finished\n");

    return 0;
}


void VSPAEROScreen::GuiDeviceCallBack( GuiDevice* device )
{
    assert( m_ScreenMgr );

    Vehicle *veh = VehicleMgr.GetVehicle();

    if( veh )
    {

        if ( device == &m_SetupButton )
        {
            if( veh->GetVSPAEROCmd().empty() || VSPAEROMgr.m_DegenFile.empty() || VSPAEROMgr.IsSolverRunning() )
            { /* Do nothing. Should not be reachable, button should be deactivated.*/ }
            else
            {
                VSPAEROMgr.CreateSetupFile();
                
                ReadSetup();    // reads file into GUI "Setup" tab for additional editing
            }
        }
        else if ( device == &m_SolverButton )
        {
            if( veh->GetVSPAEROCmd().empty() || VSPAEROMgr.m_DegenFile.empty() || VSPAEROMgr.IsSolverRunning() )
            { /* Do nothing. Should not be reachable, button should be deactivated.*/ }
            else
            {
                // Clear out previous results
                VSPAEROMgr.ClearAllPreviousResults();

                m_SolverProcess.StartThread( solver_thread_fun, (void*) &VSPAEROMgr );

                /*
                // kick off some threads to compute and monitor
                std::thread solver_thread(solver_thread_fun,this);
                
                // detach so this function can end asynchronously, when the thread finished it releases hardware resources
                solver_thread.detach();
                */
            }
        }
        else if ( device == &m_ViewerButton )
        {
            if( veh->GetVIEWERCmd().empty() || VSPAEROMgr.m_DegenFile.empty() || m_ViewerProcess.IsRunning() )
            { /* Do nothing. Should not be reachable, button should be deactivated.*/ }
            else
            {
                vector<string> args;
                args.push_back( VSPAEROMgr.m_DegenFile );

                m_ViewerProcess.ForkCmd( veh->GetExePath(), veh->GetVIEWERCmd(), args );

                m_ViewerBuffer->text( "" );
                m_ViewerMonitor.StartThread( monitorfun, (void*) &m_ViewerPair );

            }
        }
        else if( device == &m_KillSolverButton )
        {
            VSPAEROMgr.KillSolver();

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
        else if( device == &m_DegenGeomButton )
        {
            VSPAEROMgr.ComputeGeometry();
        }
        else if( device == &m_DegenFileButton )
        {
            veh->setExportFileName( vsp::DEGEN_GEOM_CSV_TYPE, m_ScreenMgr->GetSelectFileScreen()->FileChooser( "Select degen geom CSV output file.", "*.csv" ) );
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
    }
    m_ScreenMgr->SetUpdateFlag( true );
}

// VSPAEROScreen::AddOutputText( Fl_Text_Display *display, const char *text )
//     This is used for the Solver tab to show the current results of the solver in the GUI
void VSPAEROScreen::AddOutputText( Fl_Text_Display *display, const char *text )
{
    if ( display )
    {
        display->buffer()->append( text );
        while( display->move_down() ) {}

        display->show_insert_position();
    }
}

ProcessUtil* VSPAEROScreen::GetProcess( int id )
{
    if( id == VSPAERO_SOLVER )
        return VSPAEROMgr.GetSolverProcess();
    else if( id == VSPAERO_VIEWER )
        return &m_ViewerProcess;
    else
        return NULL;
}

Fl_Text_Display* VSPAEROScreen::GetDisplay( int id )
{
    if( id == VSPAERO_SOLVER )
        return m_SolverDisplay;
    else if( id == VSPAERO_VIEWER )
        return m_ViewerDisplay;
    else
        return NULL;
}

void VSPAEROScreen::ReadSetup()
{
    m_SetupBuffer->loadfile( VSPAEROMgr.m_SetupFile.c_str() );
    //TODO update the divider box title to show the filename
}

void VSPAEROScreen::SaveSetup()
{
    m_SetupBuffer->savefile( VSPAEROMgr.m_SetupFile.c_str() );
}
