//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//
//
//////////////////////////////////////////////////////////////////////

#include "AeroStructScreen.h"
#include "ScreenMgr.h"
#include "ScreenBase.h"
#include "ParmMgr.h"

#include "AeroStructMgr.h"
#include "VSPAEROScreen.h"
#include "StructScreen.h"
#include "StructureMgr.h"
#include "FeaStructure.h"
#include "FeaMeshMgr.h"
#include "FileUtil.h"

#include <FL/fl_ask.H>


//==== Constructor ====//
AeroStructScreen::AeroStructScreen( ScreenMgr* mgr ) : BasicScreen( mgr, 400, 600, "Aero Structure Coupled Analysis" )
{

    m_GlobalLayout.SetGroupAndScreen( m_FLTK_Window, this );
    m_GlobalLayout.AddY( 25 );

    m_GlobalLayout.AddDividerBox( "VSPAERO" );

    m_GlobalLayout.AddButton( m_ShowVSPAEROGUI, "Show VSPAERO GUI" );
    m_GlobalLayout.AddButton( m_ExecuteVSPAERO, "Execute VSPAERO" );

    m_GlobalLayout.AddYGap();
    m_GlobalLayout.AddDividerBox( "FEA Mesh" );

    m_GlobalLayout.AddChoice( m_StructureChoice, "Structure" );

    m_GlobalLayout.AddButton( m_ShowFEAMeshGUI, "Show FEA Mesh GUI" );
    m_GlobalLayout.AddButton( m_ExecuteFEAMesh, "Generate FEA Mesh" );

    m_GlobalLayout.AddYGap();
    m_GlobalLayout.AddDividerBox( "Loads" );

    m_GlobalLayout.AddButton( m_ExecuteLoads, "Apply Loads" );

    m_GlobalLayout.AddYGap();
    m_GlobalLayout.AddDividerBox( "CalculiX" );

    m_GlobalLayout.AddButton( m_ExecuteCalculiX, "Solve Structure" );

    m_GlobalLayout.AddYGap();
    m_GlobalLayout.AddDividerBox( "Viewer" );

    m_GlobalLayout.AddButton( m_ExecuteViewer, "Visualize Results" );

    m_GlobalLayout.AddYGap();

    m_ConsoleDisplay = m_GlobalLayout.AddFlTextDisplay( 100 );
    m_ConsoleBuffer = new Fl_Text_Buffer;
    m_ConsoleDisplay->buffer( m_ConsoleBuffer );
    m_FLTK_Window->resizable( m_ConsoleDisplay );


}

AeroStructScreen::~AeroStructScreen()
{
    m_ConsoleDisplay->buffer( NULL );
    delete m_ConsoleBuffer;
}

//==== Update Screen ====//
bool AeroStructScreen::Update()
{
    Vehicle *veh = VehicleMgr.GetVehicle();
    if ( !veh )
    {
        assert( false );
        return false;
    }

    AeroStructMgr.Update();

    VSPAEROScreen * AeroScreen = dynamic_cast < VSPAEROScreen* > ( m_ScreenMgr->GetScreen( ScreenMgr::VSP_VSPAERO_SCREEN ) );

    if ( AeroScreen )
    {
        if ( veh->GetVSPAEROFound() && !AeroScreen->m_SolverThreadIsRunning )
        {
            m_ExecuteVSPAERO.Activate();
        }
        else
        {
            m_ExecuteVSPAERO.Deactivate();
        }
    }

    if ( !AeroStructMgr.m_ADBFileFound )
    {
        m_ExecuteVSPAERO.SetColor( FL_RED );
    }
    else
    {
        m_ExecuteVSPAERO.SetColor( FL_BACKGROUND_COLOR );
    }

    if ( FeaMeshMgr.GetFeaMeshInProgress() )
    {
        m_ExecuteFEAMesh.Deactivate();
    }
    else
    {
        m_ExecuteFEAMesh.Activate();
    }

    if ( !AeroStructMgr.m_FEAMeshFileFound )
    {
        m_ExecuteFEAMesh.SetColor( FL_RED );
    }
    else
    {
        m_ExecuteFEAMesh.SetColor( FL_BACKGROUND_COLOR );
    }

    if ( veh->GetLOADSFound() && AeroStructMgr.m_ADBFileFound && AeroStructMgr.m_FEAMeshFileFound )
    {
        m_ExecuteLoads.Activate();
    }
    else
    {
        m_ExecuteLoads.Deactivate();
    }

    if ( !AeroStructMgr.m_FEAInputFileFound )
    {
        m_ExecuteLoads.SetColor( FL_RED );
    }
    else
    {
        m_ExecuteLoads.SetColor( FL_BACKGROUND_COLOR );
    }

    if ( AeroStructMgr.GetCalculiXFound() && AeroStructMgr.m_FEAInputFileFound )
    {
        m_ExecuteCalculiX.Activate();
    }
    else
    {
        m_ExecuteCalculiX.Deactivate();
    }

    if ( !AeroStructMgr.m_FEASolutionFileFound )
    {
        m_ExecuteCalculiX.SetColor( FL_RED );
    }
    else
    {
        m_ExecuteCalculiX.SetColor( FL_BACKGROUND_COLOR );
    }

    if ( veh->GetVIEWERFound() && AeroStructMgr.m_FEASolutionFileFound && AeroStructMgr.m_ADBFileFound )
    {
        m_ExecuteViewer.Activate();
    }
    else
    {
        m_ExecuteViewer.Deactivate();
    }


    m_StructureChoice.ClearItems();
    // Populate browser with added structures
    vector< FeaStructure* > structVec = StructureMgr.GetAllFeaStructs();
    if ( structVec.size() > 0 )
    {
        for ( int i = 0; i < (int)structVec.size(); i++ )
        {
            string struct_name = structVec[i]->GetName();
            int struct_surf_ind = structVec[i]->GetFeaStructMainSurfIndx();
            string parent_geom_name;
            char str[1000];

            Geom* parent = veh->FindGeom( structVec[i]->GetParentGeomID() );
            if ( parent )
            {
                parent_geom_name = parent->GetName();
            }

            sprintf( str, "%s:%s:Surf_%d", struct_name.c_str(), parent_geom_name.c_str(), struct_surf_ind );
            m_StructureChoice.AddItem( str );
        }
    }
    m_StructureChoice.UpdateItems();

    m_StructureChoice.Update( StructureMgr.m_CurrStructIndex.GetID() );




    m_FLTK_Window->redraw();
    return true;
}

//==== Show Screen ====//
void AeroStructScreen::Show()
{
    m_ScreenMgr->SetUpdateFlag( true );
    m_FLTK_Window->show();
}


//==== Hide Screen ====//
void AeroStructScreen::Hide()
{
    m_FLTK_Window->hide();
    m_ScreenMgr->SetUpdateFlag( true );
}

void AeroStructScreen::AddOutputText( const string &text )
{
    Fl::lock();
    m_ConsoleBuffer->append( text.c_str() );
    m_ConsoleDisplay->insert_position( m_ConsoleDisplay->buffer()->length() );
    m_ConsoleDisplay->show_insert_position();
    Fl::unlock();
}

//==== Callbacks ====//
void AeroStructScreen::CallBack( Fl_Widget *w )
{

//    if ( w == m_Foo )
    {

    }
    // else
    {
        return;
    }

    m_ScreenMgr->SetUpdateFlag( true );
}

#ifdef WIN32
DWORD WINAPI asmonitorfun( LPVOID data )
#else
void * asmonitorfun( void *data )
#endif
{

    AeroStructScreen *as = (AeroStructScreen *) data;

    if( as )
    {
        Fl_Text_Display *display = as->GetDisplay();
        ProcessUtil *pu = as->GetProcess();
        if( pu && display )
        {
            int bufsize = 1000;
            char *buf;
            buf = ( char* ) malloc( sizeof( char ) * ( bufsize + 1 ) );

            BUF_READ_TYPE nread = 1;

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
                    as->AddOutputText( buf );
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
            as->GetScreenMgr()->SetUpdateFlag( true );

            free( buf );
        }
    }
    return 0;
}

//==== Gui Device CallBacks ====//
void AeroStructScreen::GuiDeviceCallBack( GuiDevice* gui_device )
{
    assert( m_ScreenMgr );

    Vehicle* veh = VehicleMgr.GetVehicle();
    if ( !veh )
    {
        return;
    }

    if ( gui_device == &m_ShowVSPAEROGUI )
    {
        m_ScreenMgr->ShowScreen( ScreenMgr::VSP_VSPAERO_SCREEN );
    }
    else if( gui_device == &m_ExecuteVSPAERO )
    {
        VSPAEROScreen * AeroScreen = dynamic_cast < VSPAEROScreen* > ( m_ScreenMgr->GetScreen( ScreenMgr::VSP_VSPAERO_SCREEN ) );
        if ( AeroScreen )
        {
            // Clear the console
            m_ConsoleBuffer->text( "" );

            AeroScreen->LaunchVSPAERO();
        }
    }
    else if ( gui_device == &m_ShowFEAMeshGUI )
    {
        m_ScreenMgr->ShowScreen( ScreenMgr::VSP_STRUCT_SCREEN );
    }
    else if ( gui_device == &m_ExecuteFEAMesh )
    {
        StructScreen * structscreen = dynamic_cast < StructScreen* > ( m_ScreenMgr->GetScreen( ScreenMgr::VSP_STRUCT_SCREEN ) );
        if ( structscreen )
        {
            // Clear the console
            m_ConsoleBuffer->text( "" );

            structscreen->LaunchFEAMesh();
        }
    }
    else if ( gui_device == &m_ExecuteLoads )
    {
        // Clear the console
        m_ConsoleBuffer->text( "" );

        AeroStructMgr.TransferLoads();
    }
    else if ( gui_device == &m_ExecuteCalculiX )
    {
        // Clear the console
        m_ConsoleBuffer->text( "" );

        AeroStructMgr.ComputeStructure();
    }
    else if ( gui_device == &m_ExecuteViewer )
    {
        // Clear the console
        m_ConsoleBuffer->text( "" );

        if( !veh->GetVIEWERFound() || m_ViewerProcess.IsRunning() ||
                !FileExist( AeroStructMgr.m_FEASolutionFile ) ||
                !FileExist( AeroStructMgr.m_ADBFile ) )
        { /* Do nothing. Should not be reachable, button should be deactivated.*/ }
        else
        {
            vector<string> args;
            args.push_back( "-calculix" );

            args.push_back( GetBasename( AeroStructMgr.m_FEASolutionFile ) );

            args.push_back( GetBasename( AeroStructMgr.m_ADBFile ) );

            string command = m_ViewerProcess.PrettyCmd( veh->GetVSPAEROPath(), veh->GetVIEWERCmd(), args );

            AddOutputText( command );

            m_ViewerProcess.ForkCmd( veh->GetVSPAEROPath(), veh->GetVIEWERCmd(), args );

            m_ViewerMonitor.StartThread( asmonitorfun, ( void* ) this );

        }
    }
    else
    {
        return;
    }

    m_ScreenMgr->SetUpdateFlag( true );
}

ProcessUtil* AeroStructScreen::GetProcess()
{
    return &m_ViewerProcess;
}

Fl_Text_Display* AeroStructScreen::GetDisplay()
{
    return m_ConsoleDisplay;
}
