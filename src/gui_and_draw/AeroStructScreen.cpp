//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//
//
//////////////////////////////////////////////////////////////////////

#ifdef WIN32
#undef _HAS_STD_BYTE
#define _HAS_STD_BYTE 0
#endif

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
    m_MainLayout.SetGroupAndScreen( m_FLTK_Window, this );

    m_MainLayout.ForceNewLine();
    m_MainLayout.AddY(7);
    m_MainLayout.AddX(5);

    m_MainLayout.AddSubGroupLayout( m_BorderLayout, m_MainLayout.GetRemainX() - 5.0,
                                    m_MainLayout.GetRemainY() - 5.0 );

    m_BorderLayout.AddDividerBox( "VSPAERO" );

    m_BorderLayout.AddButton( m_ShowVSPAEROGUI, "Show VSPAERO GUI" );
    m_BorderLayout.AddButton( m_ExecuteVSPAERO, "Execute VSPAERO" );

    m_BorderLayout.AddYGap();
    m_BorderLayout.AddDividerBox( "FEA Mesh" );

    m_BorderLayout.AddChoice( m_StructureChoice, "Structure" );

    m_BorderLayout.SetFitWidthFlag( false );
    m_BorderLayout.SetSameLineFlag( true );

    m_BorderLayout.SetButtonWidth( m_BorderLayout.GetW() / 2 );

    m_BorderLayout.AddButton( m_ShowFEAMeshGUI, "Show FEA Structure GUI" );
    m_BorderLayout.AddButton( m_ShowFEAAssemblyGUI, "Show FEA Assembly GUI" );
    m_BorderLayout.ForceNewLine();


    m_BorderLayout.AddButton( m_ExecuteRemeshAllFEAMesh, "Remesh All" );
    m_BorderLayout.AddButton( m_ExecuteMeshUnmeshedFEAMesh, "Mesh Unmeshed" );
    m_BorderLayout.ForceNewLine();

    m_BorderLayout.SetFitWidthFlag( true );
    m_BorderLayout.SetSameLineFlag( false );

    m_BorderLayout.AddButton( m_ExportFEAMesh, "Export FEA Mesh" );

    m_BorderLayout.AddButton( m_ExecuteCGXMesh, "Visualize Mesh in cgx" );

    m_BorderLayout.AddYGap();
    m_BorderLayout.AddDividerBox( "Loads" );

    m_BorderLayout.SetButtonWidth( m_BorderLayout.GetW() / 3 );

    m_BorderLayout.AddSlider( m_DynPressSlider, "Dynamic Pressure", 100, "%7.3f"  );
    m_BorderLayout.AddButton( m_ExecuteLoads, "Apply Loads" );

    m_BorderLayout.AddButton( m_ExecuteCGXInput, "Visualize Input in cgx" );

    m_BorderLayout.AddYGap();
    m_BorderLayout.AddDividerBox( "CalculiX" );

    m_BorderLayout.AddButton( m_ExecuteCalculiX, "Solve Structure" );

    m_BorderLayout.AddYGap();
    m_BorderLayout.AddDividerBox( "Viewer" );

    m_BorderLayout.AddButton( m_ExecuteViewer, "Visualize Results in vspviewer" );
    m_BorderLayout.AddButton( m_ExecuteCGXSolution, "Visualize Results in cgx" );

    m_BorderLayout.ForceNewLine();

    m_ConsoleDisplay = m_BorderLayout.AddFlTextDisplay( m_BorderLayout.GetH() - m_BorderLayout.GetY() + 25 );
    m_ConsoleBuffer = new Fl_Text_Buffer;
    m_ConsoleDisplay->buffer( m_ConsoleBuffer );
    m_FLTK_Window->resizable( m_ConsoleDisplay );
}

AeroStructScreen::~AeroStructScreen()
{
    m_ConsoleDisplay->buffer( nullptr );
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

    BasicScreen::Update();

    AeroStructMgr.Update();

    bool structselected = AeroStructMgr.m_CurrStructAssyIndex() >= 0;

    m_DynPressSlider.Update( AeroStructMgr.m_DynPress.GetID() );

    VSPAEROScreen * AeroScreen = dynamic_cast < VSPAEROScreen* > ( m_ScreenMgr->GetScreen( vsp::VSP_VSPAERO_SCREEN ) );

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

    if ( FeaMeshMgr.GetMeshPtr() )
    {
        if ( FeaMeshMgr.GetMeshPtr()->m_MeshReady )
        {
            m_ExecuteRemeshAllFEAMesh.SetColor( FL_BACKGROUND_COLOR );
            m_ExecuteMeshUnmeshedFEAMesh.SetColor( FL_BACKGROUND_COLOR );
        }
        else
        {
            m_ExecuteRemeshAllFEAMesh.SetColor( FL_RED );
            m_ExecuteMeshUnmeshedFEAMesh.SetColor( FL_RED );
        }
    }
    else
    {
        m_ExecuteRemeshAllFEAMesh.SetColor( FL_RED );
        m_ExecuteMeshUnmeshedFEAMesh.SetColor( FL_RED );
    }

    if ( !structselected || FeaMeshMgr.GetFeaMeshInProgress() )
    {
        m_ExecuteRemeshAllFEAMesh.Deactivate();
        m_ExecuteMeshUnmeshedFEAMesh.Deactivate();
    }
    else
    {
        m_ExecuteRemeshAllFEAMesh.Activate();
        m_ExecuteMeshUnmeshedFEAMesh.Activate();
    }

    if ( FeaMeshMgr.GetMeshPtr() && !FeaMeshMgr.GetFeaMeshInProgress() )
    {
        if ( FeaMeshMgr.GetMeshPtr()->m_MeshReady )
        {
            m_ExportFEAMesh.Activate();
        }
        else
        {
            m_ExportFEAMesh.Deactivate();
        }
    }
    else
    {
        m_ExportFEAMesh.Deactivate();
    }

    if ( !AeroStructMgr.m_FEAMeshFileFound )
    {
        m_ExportFEAMesh.SetColor( FL_RED );
    }
    else
    {
        m_ExportFEAMesh.SetColor( FL_BACKGROUND_COLOR );
    }

    if ( AeroStructMgr.GetCGXFound() && AeroStructMgr.m_FEAMeshFileFound && !m_ViewerProcess.IsRunning() )
    {
        m_ExecuteCGXMesh.Activate();
    }
    else
    {
        m_ExecuteCGXMesh.Deactivate();
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

    if ( AeroStructMgr.GetCGXFound() && AeroStructMgr.m_FEAInputFileFound && !m_ViewerProcess.IsRunning() )
    {
        m_ExecuteCGXInput.Activate();
    }
    else
    {
        m_ExecuteCGXInput.Deactivate();
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

    if ( veh->GetVIEWERFound() && AeroStructMgr.m_FEASolutionFileFound && AeroStructMgr.m_ADBFileFound && !m_ViewerProcess.IsRunning() )
    {
        m_ExecuteViewer.Activate();
    }
    else
    {
        m_ExecuteViewer.Deactivate();
    }

    if ( AeroStructMgr.GetCGXFound() && AeroStructMgr.m_FEASolutionFileFound && !m_ViewerProcess.IsRunning() )
    {
        m_ExecuteCGXSolution.Activate();
    }
    else
    {
        m_ExecuteCGXSolution.Deactivate();
    }

    m_StructureChoice.ClearItems();

    for ( int i = 0; i < AeroStructMgr.m_StructAssyIDVec.size(); i++ )
    {
        string id = AeroStructMgr.m_StructAssyIDVec[i];

        if ( AeroStructMgr.m_StructAssyFlagVec[i] )        // Current is a structure.
        {
            FeaStructure* fea_struct = StructureMgr.GetFeaStruct( id );

            if ( fea_struct )
            {
                string struct_name = fea_struct->GetName();
                int struct_surf_ind = fea_struct->GetFeaStructMainSurfIndx();
                string parent_geom_name;
                char str[1000];

                Geom *parent = veh->FindGeom( fea_struct->GetParentGeomID());
                if ( parent )
                {
                    parent_geom_name = parent->GetName();
                }

                snprintf( str, sizeof( str ),  "%s:%s:Surf_%d", struct_name.c_str(), parent_geom_name.c_str(), struct_surf_ind );
                m_StructureChoice.AddItem( str );
            }
        }
        else                                               // Current is an assembly.
        {
            FeaAssembly* fea_assy = StructureMgr.GetFeaAssembly( id );

            if ( fea_assy )
            {
                string assy_name = fea_assy->GetName();

                m_StructureChoice.AddItem( assy_name );
            }
        }
    }

    m_StructureChoice.UpdateItems();

    m_StructureChoice.Update( AeroStructMgr.m_CurrStructAssyIndex.GetID() );



    m_FLTK_Window->redraw();
    return true;
}

//==== Show Screen ====//
void AeroStructScreen::Show()
{
    m_ScreenMgr->SetUpdateFlag( true );
    BasicScreen::Show();
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

            Fl::lock();
            // Any FL calls must occur between Fl::lock() and Fl::unlock().
            as->AddOutputText( "Done\n" );
            Fl::unlock();

#ifdef WIN32
            CloseHandle( pu->m_StdoutPipe[0] );
            pu->m_StdoutPipe[0] = nullptr;
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
        m_ScreenMgr->ShowScreen( vsp::VSP_VSPAERO_SCREEN );
    }
    else if( gui_device == &m_ExecuteVSPAERO )
    {
        VSPAEROScreen * AeroScreen = dynamic_cast < VSPAEROScreen* > ( m_ScreenMgr->GetScreen( vsp::VSP_VSPAERO_SCREEN ) );
        if ( AeroScreen )
        {
            // Clear the console
            m_ConsoleBuffer->text( "" );

            AeroScreen->LaunchVSPAERO();
        }
    }
    else if ( gui_device == &m_ShowFEAMeshGUI )
    {
        m_ScreenMgr->ShowScreen( vsp::VSP_STRUCT_SCREEN );
    }
    else if ( gui_device == &m_ShowFEAAssemblyGUI )
    {
        m_ScreenMgr->ShowScreen( vsp::VSP_STRUCT_ASSEMBLY_SCREEN );
    }
    else if ( gui_device == &m_ExecuteRemeshAllFEAMesh )
    {
        // Clear the console
        m_ConsoleBuffer->text( "" );

        vector < string > structid;
        structid = MakeStructIDVec();

        if ( !structid.empty() )
        {
            FeaMeshMgr.CleanupMeshes( structid );
            LaunchBatchFEAMesh( structid );
        }
    }
    else if ( gui_device == &m_ExecuteMeshUnmeshedFEAMesh )
    {
        // Clear the console
        m_ConsoleBuffer->text( "" );

        vector < string > structid;
        structid = MakeStructIDVec();

        if ( !structid.empty() )
        {
            LaunchBatchFEAMesh( structid );
        }
    }
    else if ( gui_device == &m_ExportFEAMesh )
    {
        // Clear the console
        m_ConsoleBuffer->text( "" );

        if ( AeroStructMgr.m_CurrStructAssyIndex() >= 0 ) // Non-negative test is sufficient because of tests elsewhere.
        {
            string id = AeroStructMgr.m_StructAssyIDVec[ AeroStructMgr.m_CurrStructAssyIndex() ];

            if ( AeroStructMgr.m_StructAssyFlagVec[ AeroStructMgr.m_CurrStructAssyIndex() ] )         // Currently a structure
            {
                FeaMeshMgr.addOutputText( "Exporting Mesh Files\n" );
                FeaMeshMgr.ExportFeaMesh( id );
            }
            else                                                                                      // Currently an assembly.
            {
                FeaMeshMgr.addOutputText( "Exporting Mesh Files\n" );
                FeaMeshMgr.ExportAssemblyMesh( id );
            }
            FeaMeshMgr.addOutputText( "Done\n" );
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

            string command = ProcessUtil::PrettyCmd( veh->GetVSPAEROPath(), veh->GetVIEWERCmd(), args );

            AddOutputText( command );

            m_ViewerProcess.ForkCmd( veh->GetVSPAEROPath(), veh->GetVIEWERCmd(), args );

            m_ViewerMonitor.StartThread( asmonitorfun, ( void* ) this );

        }
    }
    else if ( gui_device == &m_ExecuteCGXMesh )
    {
        // Clear the console
        m_ConsoleBuffer->text( "" );

        if( !AeroStructMgr.GetCGXFound() || m_ViewerProcess.IsRunning() ||
            !FileExist( AeroStructMgr.m_FEAMeshFile ) )
        { /* Do nothing. Should not be reachable, button should be deactivated.*/ }
        else
        {
            vector<string> args;
            args.push_back( "-c" );

            args.push_back( AeroStructMgr.m_FEAMeshFile );

            string command = ProcessUtil::PrettyCmd( AeroStructMgr.GetCGXPath(), AeroStructMgr.GetCGXCmd(), args );

            AddOutputText( command );

            m_ViewerProcess.ForkCmd( AeroStructMgr.GetCGXPath(), AeroStructMgr.GetCGXCmd(), args );

            m_ViewerMonitor.StartThread( asmonitorfun, ( void* ) this );

        }
    }
    else if ( gui_device == &m_ExecuteCGXInput )
    {
        // Clear the console
        m_ConsoleBuffer->text( "" );

        if( !AeroStructMgr.GetCGXFound() || m_ViewerProcess.IsRunning() ||
            !FileExist( AeroStructMgr.m_FEAInputFile ) )
        { /* Do nothing. Should not be reachable, button should be deactivated.*/ }
        else
        {
            vector<string> args;
            args.push_back( "-c" );

            args.push_back( AeroStructMgr.m_FEAInputFile );

            string command = ProcessUtil::PrettyCmd( AeroStructMgr.GetCGXPath(), AeroStructMgr.GetCGXCmd(), args );

            AddOutputText( command );

            m_ViewerProcess.ForkCmd( AeroStructMgr.GetCGXPath(), AeroStructMgr.GetCGXCmd(), args );

            m_ViewerMonitor.StartThread( asmonitorfun, ( void* ) this );

        }
    }
    else if ( gui_device == &m_ExecuteCGXSolution )
    {
        // Clear the console
        m_ConsoleBuffer->text( "" );

        if( !AeroStructMgr.GetCGXFound() || m_ViewerProcess.IsRunning() ||
            !FileExist( AeroStructMgr.m_FEASolutionFile ) ||
            !FileExist( AeroStructMgr.m_FEAInputFile ) )
        { /* Do nothing. Should not be reachable, button should be deactivated.*/ }
        else
        {
            vector<string> args;

            args.push_back( AeroStructMgr.m_FEASolutionFile );

            args.push_back( AeroStructMgr.m_FEAInputFile );

            string command = ProcessUtil::PrettyCmd( AeroStructMgr.GetCGXPath(), AeroStructMgr.GetCGXCmd(), args );

            AddOutputText( command );

            m_ViewerProcess.ForkCmd( AeroStructMgr.GetCGXPath(), AeroStructMgr.GetCGXCmd(), args );

            m_ViewerMonitor.StartThread( asmonitorfun, ( void* ) this );

        }
    }
    else
    {
        return;
    }

    m_ScreenMgr->SetUpdateFlag( true );
}

vector < string > AeroStructScreen::MakeStructIDVec()
{
    vector < string > structid;

    if ( AeroStructMgr.m_CurrStructAssyIndex() >= 0 ) // Non-negative test is sufficient because of tests elsewhere.
    {
        string id = AeroStructMgr.m_StructAssyIDVec[ AeroStructMgr.m_CurrStructAssyIndex() ];

        if ( AeroStructMgr.m_StructAssyFlagVec[ AeroStructMgr.m_CurrStructAssyIndex() ] )         // Currently a structure
        {
            structid.push_back( id );
        }
        else                                                                                      // Currently an assembly.
        {
            FeaAssembly *fea_assy = StructureMgr.GetFeaAssembly( id );

            if ( fea_assy )
            {
                structid = fea_assy->m_StructIDVec;
            }
        }
    }
    return structid;
}

ProcessUtil* AeroStructScreen::GetProcess()
{
    return &m_ViewerProcess;
}

Fl_Text_Display* AeroStructScreen::GetDisplay()
{
    return m_ConsoleDisplay;
}

#ifdef WIN32
DWORD WINAPI aerostruct_feamesh_thread_fun( LPVOID data )
#else
void * aerostruct_feamesh_thread_fun( void *data )
#endif
{
    vector < string > *idvec = ( vector < string > * ) data;

    FeaMeshMgr.MeshUnMeshed( *idvec );

    return 0;
}

void AeroStructScreen::LaunchBatchFEAMesh( const vector < string > &idvec )
{
    // Set m_FeaMeshInProgress to ensure m_MonitorProcess does not terminate prematurely
    FeaMeshMgr.SetFeaMeshInProgress( true );

    // Copy vector to memory that will persist through duration of meshing process.
    m_BatchIDs = idvec;

    m_FeaMeshProcess.StartThread( aerostruct_feamesh_thread_fun, ( void* ) &m_BatchIDs  );
}
