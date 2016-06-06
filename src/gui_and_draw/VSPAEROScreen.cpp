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

#include <utility>
#include <string>

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

VSPAEROScreen::VSPAEROScreen( ScreenMgr* mgr ) : TabScreen( mgr, 650, 340, "VSPAERO" )
{
    m_SolverPair = make_pair( this, VSPAERO_SOLVER );
    m_ViewerPair = make_pair( this, VSPAERO_VIEWER );
    m_SetupFlag = false;
    m_ADBFlag = false;
    
    //==== Overview Tab ====//
    Fl_Group* overview_tab = AddTab( "Overview" );
    Fl_Group* setup_tab = AddTab( "Setup" );
    Fl_Group* solver_tab = AddTab( "Solver" );
    Fl_Group* viewer_tab = AddTab( "Viewer" );

    overview_tab->show();


    Fl_Group* overview_group = AddSubGroup( overview_tab, 5 );
    m_OverviewLayout.SetGroupAndScreen( overview_group, this );

    m_OverviewLayout.AddSubGroupLayout( m_GeomLayout, 320 - 2, 80 );
    m_OverviewLayout.AddX( 320 + 2 );
    m_OverviewLayout.AddSubGroupLayout( m_FlowLayout, 320 - 2, 80 );
    m_OverviewLayout.ForceNewLine();
    m_OverviewLayout.AddY( 70 );

    m_OverviewLayout.AddSubGroupLayout( m_RefLayout, 320 - 2, 120 );
    m_OverviewLayout.AddX( 320 + 2 );
    m_OverviewLayout.AddSubGroupLayout( m_CGLayout, 320 - 2, 120 );
    m_OverviewLayout.ForceNewLine();
    m_OverviewLayout.AddY( 110 );

    m_GeomLayout.AddDividerBox( "Case Setup" );

    m_GeomLayout.SetSameLineFlag( true );
    m_GeomLayout.SetFitWidthFlag( false );

    m_GeomLayout.SetButtonWidth( 50 );
    m_GeomLayout.SetInputWidth( 318 - 50 - 25 );

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

    m_GeomLayout.SetSameLineFlag( false );
    m_GeomLayout.SetFitWidthFlag( true );

    m_GeomLayout.InitWidthHeightVals();
    m_GeomLayout.AddSlider( m_NCPUSlider, "Num CPU", 10.0, "%3.0f" );

    // Flow Condition
    m_FlowLayout.AddDividerBox( "Flow Condition" );
    m_FlowLayout.AddSlider( m_AlphaSlider, "Alpha", 10.0, "%7.3f" );
    m_FlowLayout.AddSlider( m_BetaSlider, "Beta", 10.0, "%7.3f" );
    m_FlowLayout.AddSlider( m_MachSlider, "Mach", 3.0, "%7.3f" );

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

    SetupDegenFile();

    string setupfile = m_DegenFile + string(".vspaero");
    m_SetupFlag = FileExist( setupfile );
    string adbfile = m_DegenFile + string(".adb");
    m_ADBFlag = FileExist( adbfile );

    VSPAEROMgr.Update();

    if( veh )
    {

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

        m_GeomSetChoice.SetVal( VSPAEROMgr.m_DegenGeomSet() );
        m_CGSetChoice.SetVal( VSPAEROMgr.m_CGGeomSet() );


        m_DegenFileName.Update( veh->getExportFileName( vsp::DEGEN_GEOM_CSV_TYPE ) );

        m_RefToggle.Update( VSPAEROMgr.m_RefFlag.GetID() );

        m_SrefSlider.Update( VSPAEROMgr.m_Sref.GetID() );
        m_brefSlider.Update( VSPAEROMgr.m_bref.GetID() );
        m_crefSlider.Update( VSPAEROMgr.m_cref.GetID() );

        m_NumSliceSlider.Update( VSPAEROMgr.m_NumMassSlice.GetID() );
        m_XcgSlider.Update( VSPAEROMgr.m_Xcg.GetID() );
        m_YcgSlider.Update( VSPAEROMgr.m_Ycg.GetID() );
        m_ZcgSlider.Update( VSPAEROMgr.m_Zcg.GetID() );
        m_AlphaSlider.Update( VSPAEROMgr.m_Alpha.GetID() );
        m_BetaSlider.Update( VSPAEROMgr.m_Beta.GetID() );
        m_MachSlider.Update( VSPAEROMgr.m_Mach.GetID() );

        m_NCPUSlider.Update( VSPAEROMgr.m_NCPU.GetID() );

        if( m_SolverProcess.IsRunning() )
        {
            m_KillSolverButton.Activate();
        }
        else
        {
            m_KillSolverButton.Deactivate();
        }

        if( veh->GetVSPAEROCmd().empty() || m_DegenFile.empty() || m_SolverProcess.IsRunning() )
        {
            m_SetupButton.Deactivate();
        }
        else
        {
            m_SetupButton.Activate();
        }

        if( veh->GetVSPAEROCmd().empty() || m_DegenFile.empty() || m_SolverProcess.IsRunning() || !m_SetupFlag )
        {
            m_SolverButton.Deactivate();
        }
        else
        {
            m_SolverButton.Activate();
        }

        if( veh->GetVIEWERCmd().empty() || m_DegenFile.empty() || m_ViewerProcess.IsRunning() || !m_SetupFlag || !m_ADBFlag )
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
    Update();
    m_FLTK_Window->show();

}

void VSPAEROScreen::Hide()
{
    m_FLTK_Window->hide();
}

void VSPAEROScreen::CallBack( Fl_Widget* w )
{
    m_ScreenMgr->SetUpdateFlag( true );
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

#ifdef WIN32
            DWORD nread = 1;
#else
            int nread = 1;
#endif

            bool runflag = pu->IsRunning();
            while( runflag || nread > 0)
            {
#ifdef WIN32
                ReadFile( pu->m_StdoutPipe[0], buf, bufsize, &nread, NULL);
#else
                nread = read( pu->m_StdoutPipe[0], buf, bufsize );
#endif

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
#ifdef WIN32
                    Sleep( 100 );
#else
                    usleep( 100000 );
#endif
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

void VSPAEROScreen::GuiDeviceCallBack( GuiDevice* device )
{
    assert( m_ScreenMgr );

    Vehicle *veh = VehicleMgr.GetVehicle();

    if( veh )
    {

        if ( device == &m_SetupButton )
        {
            if( veh->GetVSPAEROCmd().empty() || m_DegenFile.empty() || m_SolverProcess.IsRunning() )
            { /* Do nothing. Should not be reachable, button should be deactivated.*/ }
            else
            {
                vector<string> args;
                args.push_back( "-setup" );

                args.push_back( "-sref" );
                args.push_back( StringUtil::double_to_string( VSPAEROMgr.m_Sref(), "%f" ) );

                args.push_back( "-bref" );
                args.push_back( StringUtil::double_to_string( VSPAEROMgr.m_bref(), "%f" ) );

                args.push_back( "-cref" );
                args.push_back( StringUtil::double_to_string( VSPAEROMgr.m_cref(), "%f" ) );

                args.push_back( "-cg" );
                args.push_back( StringUtil::double_to_string( VSPAEROMgr.m_Xcg(), "%f" ) );
                args.push_back( StringUtil::double_to_string( VSPAEROMgr.m_Ycg(), "%f" ) );
                args.push_back( StringUtil::double_to_string( VSPAEROMgr.m_Zcg(), "%f" ) );

                args.push_back( "-aoa" );
                args.push_back( StringUtil::double_to_string( VSPAEROMgr.m_Alpha(), "%f" ) );

                args.push_back( "-beta" );
                args.push_back( StringUtil::double_to_string( VSPAEROMgr.m_Beta(), "%f" ) );

                args.push_back( "-mach" );
                args.push_back( StringUtil::double_to_string( VSPAEROMgr.m_Mach(), "%f" ) );

                args.push_back( m_DegenFile );

                m_SolverProcess.ForkCmd( veh->GetExePath(), veh->GetVSPAEROCmd(), args );

#ifdef WIN32
                Sleep( 100 );
#else
                usleep( 100000 );
#endif
                ReadSetup();
            }
        }
        else if ( device == &m_SolverButton )
        {
            if( veh->GetVSPAEROCmd().empty() || m_DegenFile.empty() || m_SolverProcess.IsRunning() )
            { /* Do nothing. Should not be reachable, button should be deactivated.*/ }
            else
            {
                vector<string> args;
                args.push_back( "-omp" );
                args.push_back( StringUtil::int_to_string( VSPAEROMgr.m_NCPU(), "%d" ) );
                args.push_back( m_DegenFile );

                m_SolverProcess.ForkCmd( veh->GetExePath(), veh->GetVSPAEROCmd(), args );

                m_SolverBuffer->text( "" );
                m_SolverMonitor.StartThread( monitorfun, (void*) &m_SolverPair );
            }
        }
        else if ( device == &m_ViewerButton )
        {
            if( veh->GetVIEWERCmd().empty() || m_DegenFile.empty() || m_ViewerProcess.IsRunning() )
            { /* Do nothing. Should not be reachable, button should be deactivated.*/ }
            else
            {
                vector<string> args;
                args.push_back( m_DegenFile );

                m_ViewerProcess.ForkCmd( veh->GetExePath(), veh->GetVIEWERCmd(), args );

                m_ViewerBuffer->text( "" );
                m_ViewerMonitor.StartThread( monitorfun, (void*) &m_ViewerPair );

            }
        }
        else if( device == &m_KillSolverButton )
        {
            m_SolverProcess.Kill();
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
            VSPAEROMgr.m_DegenGeomSet = m_GeomSetChoice.GetVal();
        }
        else if( device == &m_DegenGeomButton )
        {
            veh->CreateDegenGeom( VSPAEROMgr.m_DegenGeomSet() );

            bool exptMfile = veh->getExportDegenGeomMFile();
            bool exptCSVfile = veh->getExportDegenGeomCsvFile();
            veh->setExportDegenGeomMFile( false );
            veh->setExportDegenGeomCsvFile( true );

            veh->WriteDegenGeomFile();

            veh->setExportDegenGeomMFile( exptMfile );
            veh->setExportDegenGeomCsvFile( exptCSVfile );
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

void VSPAEROScreen::SetupDegenFile()
{
    Vehicle *veh = VehicleMgr.GetVehicle();

    string oldfile = m_DegenFile;

    if( veh )
    {
        m_DegenFile = veh->getExportFileName( vsp::DEGEN_GEOM_CSV_TYPE );

        FILE *fp = NULL;
        fp = fopen( m_DegenFile.c_str(), "r" );
        if( fp )
        {
            fclose( fp );
        }
        else
        {
            m_DegenFile = string();
        }

        if( !m_DegenFile.empty() )
        {
            int pos = m_DegenFile.find( ".csv" );
            if ( pos >= 0 )
            {
                m_DegenFile.erase( pos, m_DegenFile.length() - 1 );
            }
        }
    }
    else
    {
        m_DegenFile = string();
    }

    if( m_DegenFile.compare( oldfile ) != 0 )
    {
        if( !m_DegenFile.empty() )
        {
            ReadSetup();
        }
    }
}


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
        return &m_SolverProcess;
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
    string setupfile = m_DegenFile + string(".vspaero");
    m_SetupBuffer->loadfile( setupfile.c_str() );
}

void VSPAEROScreen::SaveSetup()
{
    string setupfile = m_DegenFile + string(".vspaero");
    m_SetupBuffer->savefile( setupfile.c_str() );
}
