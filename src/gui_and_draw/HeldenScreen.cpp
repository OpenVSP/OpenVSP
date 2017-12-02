//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//
//
//////////////////////////////////////////////////////////////////////

#include "HeldenScreen.h"
#include "ProjectionMgr.h"
#include "StringUtil.h"
#include "FileUtil.h"

#include "HeldenMgr.h"

HeldenScreen::HeldenScreen( ScreenMgr* mgr ) : TabScreen( mgr, 600, 500, "Helden Mesh Setup" )
{
    m_FLTK_Window->callback( staticCloseCB, this );
    m_MainLayout.SetGroupAndScreen( m_FLTK_Window, this );

    m_HSurfPair = make_pair( &HeldenMgr, this );  //heldenpair type
    m_HPatchPair = make_pair( &HeldenMgr, this );  //heldenpair type
    m_HMeshPair = make_pair( &HeldenMgr, this );  //heldenpair type

    m_HSurfThreadIsRunning = false;
    m_HPatchThreadIsRunning = false;
    m_HMeshThreadIsRunning = false;

    Fl_Group* options_tab = AddTab( "Options" );
    Fl_Group* surf_file_tab = AddTab( "INPUT_HELDENSURFACE" );
    Fl_Group* patch_file_tab = AddTab( "INPUT_HELDENPATCH" );
    Fl_Group* mesh_file_tab = AddTab( "INPUT_HELDENMESH" );


    Fl_Group* options_group = AddSubGroup( options_tab, 5 );
    m_OptionsLayout.SetGroupAndScreen( options_group, this );

    int window_border_width = 5;
    int group_border_width = 2;

    int col_height = 220;

    m_OptionsLayout.AddSubGroupLayout( m_FirstColLayout, ( m_OptionsLayout.GetW()
                    - group_border_width ) / 2 - window_border_width, col_height );
    m_OptionsLayout.AddX( ( m_OptionsLayout.GetW() ) / 2 - window_border_width + group_border_width );
    m_OptionsLayout.AddSubGroupLayout( m_SecondColLayout, m_OptionsLayout.GetRemainX() - group_border_width / 2, col_height );
    m_OptionsLayout.ForceNewLine();
    m_OptionsLayout.AddY( col_height );


    m_FirstColLayout.AddDividerBox( "Case Setup" );
    m_FirstColLayout.AddChoice( m_SelectedSetChoice, "Geometry Set: " );

    m_FirstColLayout.AddYGap();

    m_FirstColLayout.AddDividerBox( "IGES Export Settings" );

    m_FirstColLayout.SetFitWidthFlag( false );
    m_FirstColLayout.SetSameLineFlag( true );

    m_FirstColLayout.SetButtonWidth( 30 );
    m_FirstColLayout.SetInputWidth( m_FirstColLayout.GetW() - 2 * m_FirstColLayout.GetButtonWidth() );
    m_FirstColLayout.AddOutput( m_IGESFileOutput, ".igs" );
    m_FirstColLayout.AddButton( m_IGESFileSelect, "..." );
    m_FirstColLayout.ForceNewLine();
    m_FirstColLayout.AddYGap();

    m_FirstColLayout.SetFitWidthFlag( true );
    m_FirstColLayout.SetSameLineFlag( false );


    m_FirstColLayout.InitWidthHeightVals();

    m_LenUnitChoice.AddItem( "MM" );
    m_LenUnitChoice.AddItem( "CM" );
    m_LenUnitChoice.AddItem( "M" );
    m_LenUnitChoice.AddItem( "IN" );
    m_LenUnitChoice.AddItem( "FT" );
    m_FirstColLayout.AddChoice( m_LenUnitChoice, "Length Unit" );
    m_FirstColLayout.AddYGap();

    m_FirstColLayout.AddButton( m_SplitSurfsToggle, "Split Surfaces" );
    m_FirstColLayout.AddYGap();
    m_FirstColLayout.AddButton( m_SplitSubSurfsToggle, "Split U/W-Const Sub-Surfaces" );
    m_FirstColLayout.AddYGap();
    m_FirstColLayout.AddButton( m_TrimTEToggle, "Omit TE Surfaces" );
    m_FirstColLayout.AddYGap();
    m_FirstColLayout.AddButton( m_ToCubicToggle, "Convert to Cubic" );
    m_FirstColLayout.AddSlider( m_ToCubicTolSlider, "Tolerance", 10, "%5.4g", 0, true );


    m_OptionsLayout.AddYGap();

    m_ConsoleDisplay = m_OptionsLayout.AddFlTextDisplay( m_OptionsLayout.GetRemainY() - 4 * m_OptionsLayout.GetStdHeight() );
    m_ConsoleBuffer = new Fl_Text_Buffer;
    m_ConsoleDisplay->buffer( m_ConsoleBuffer );

    m_OptionsLayout.SetButtonWidth( m_OptionsLayout.GetW() / 3 );

    m_OptionsLayout.SetSameLineFlag( true );
    m_OptionsLayout.SetFitWidthFlag( false );

    m_OptionsLayout.AddButton( m_ExportIGESButton, "Export IGES File" );
    m_OptionsLayout.ForceNewLine();

    m_OptionsLayout.AddButton( m_InitHSurfButton, "Init Helden Surf" );
    m_OptionsLayout.AddButton( m_HSurfButton, "Launch Helden Surf" );
    m_OptionsLayout.AddButton( m_KillHSurfButton, "Kill Helden Surf" );
    m_OptionsLayout.ForceNewLine();

    m_OptionsLayout.AddButton( m_InitHPatchButton, "Init Helden Patch" );
    m_OptionsLayout.AddButton( m_HPatchButton, "Launch Helden Patch" );
    m_OptionsLayout.AddButton( m_KillHPatchButton, "Kill Helden Patch" );
    m_OptionsLayout.ForceNewLine();

    m_OptionsLayout.AddButton( m_InitHMeshButton, "Init Helden Mesh" );
    m_OptionsLayout.AddButton( m_HMeshButton, "Launch Helden Mesh" );
    m_OptionsLayout.AddButton( m_KillHMeshButton, "Kill Helden Mesh" );


    // Set up Helden Surf File tab

    Fl_Group* surf_file_group = AddSubGroup( surf_file_tab, 5 );
    m_SurfFileLayout.SetGroupAndScreen( surf_file_group, this );

    m_SurfFileLayout.AddDividerBox( "Helden Surf INPUT_HELDENSURFACE File" );

    m_SurfFileEditor = m_SurfFileLayout.AddFlTextEditor( m_SurfFileLayout.GetRemainY() - m_SurfFileLayout.GetStdHeight() );

    m_SurfFileBuffer = new Fl_Text_Buffer;
    m_SurfFileEditor->buffer( m_SurfFileBuffer );
    m_SurfFileEditor->textfont( FL_COURIER );

    m_SurfFileLayout.SetButtonWidth( m_SurfFileLayout.GetW()/2 );
    m_SurfFileLayout.SetSameLineFlag( true );
    m_SurfFileLayout.SetFitWidthFlag( false );
    m_SurfFileLayout.AddButton( m_SaveSurfFile, "Save Surface File" );
    m_SurfFileLayout.AddButton( m_ReadSurfFile, "Read Surface File" );

    // Set up Helden Patch File tab

    Fl_Group* patch_file_group = AddSubGroup( patch_file_tab, 5 );
    m_PatchFileLayout.SetGroupAndScreen( patch_file_group, this );

    m_PatchFileLayout.AddDividerBox( "Helden Patch INPUT_HELDENPATCH File" );

    m_PatchFileEditor = m_PatchFileLayout.AddFlTextEditor( m_PatchFileLayout.GetRemainY() - m_PatchFileLayout.GetStdHeight() );

    m_PatchFileBuffer = new Fl_Text_Buffer;
    m_PatchFileEditor->buffer( m_PatchFileBuffer );
    m_PatchFileEditor->textfont( FL_COURIER );

    m_PatchFileLayout.SetButtonWidth( m_PatchFileLayout.GetW()/2 );
    m_PatchFileLayout.SetSameLineFlag( true );
    m_PatchFileLayout.SetFitWidthFlag( false );
    m_PatchFileLayout.AddButton( m_SavePatchFile, "Save Patch File" );
    m_PatchFileLayout.AddButton( m_ReadPatchFile, "Read Patch File" );

    // Set up Helden Mesh File tab

    Fl_Group* mesh_file_group = AddSubGroup( mesh_file_tab, 5 );
    m_MeshFileLayout.SetGroupAndScreen( mesh_file_group, this );

    m_MeshFileLayout.AddDividerBox( "Helden Mesh INPUT_HELDENMESH File" );

    m_MeshFileEditor = m_MeshFileLayout.AddFlTextEditor( m_MeshFileLayout.GetRemainY() - m_MeshFileLayout.GetStdHeight() );

    m_MeshFileBuffer = new Fl_Text_Buffer;
    m_MeshFileEditor->buffer( m_MeshFileBuffer );
    m_MeshFileEditor->textfont( FL_COURIER );

    m_MeshFileLayout.SetButtonWidth( m_MeshFileLayout.GetW()/2 );
    m_MeshFileLayout.SetSameLineFlag( true );
    m_MeshFileLayout.SetFitWidthFlag( false );
    m_MeshFileLayout.AddButton( m_SaveMeshFile, "Save Mesh File" );
    m_MeshFileLayout.AddButton( m_ReadMeshFile, "Read Mesh File" );

    options_tab->show();
}

HeldenScreen::~HeldenScreen()
{
}

#ifdef WIN32
DWORD WINAPI hsurf_thread_fun( LPVOID data )
#else
void * hsurf_thread_fun(void *data)
#endif
{
    heldenpair *hp = ( heldenpair * ) data;

    HeldenMgrSingleton* hmgr = hp->first;
    HeldenScreen *hscreen = hp->second;

    if( hmgr && hscreen )
    {
        hscreen->m_HSurfThreadIsRunning = true;

        // EXECUTE SOLVER
        hmgr->ExecuteHSurf();

        hscreen->m_HSurfThreadIsRunning = false;

        hscreen->GetScreenMgr()->SetUpdateFlag( true );
    }

    return 0;
}

#ifdef WIN32
DWORD WINAPI hpatch_thread_fun( LPVOID data )
#else
void * hpatch_thread_fun(void *data)
#endif
{
    heldenpair *hp = ( heldenpair * ) data;

    HeldenMgrSingleton* hmgr = hp->first;
    HeldenScreen *hscreen = hp->second;

    if( hmgr && hscreen )
    {
        hscreen->m_HPatchThreadIsRunning = true;

        // EXECUTE SOLVER
        hmgr->ExecuteHPatch();

        hscreen->m_HPatchThreadIsRunning = false;

        hscreen->GetScreenMgr()->SetUpdateFlag( true );
    }

    return 0;
}

#ifdef WIN32
DWORD WINAPI hpatch_thread_fun( LPVOID data )
#else
void * hmesh_thread_fun(void *data)
#endif
{
    heldenpair *hp = ( heldenpair * ) data;

    HeldenMgrSingleton* hmgr = hp->first;
    HeldenScreen *hscreen = hp->second;

    if( hmgr && hscreen )
    {
        hscreen->m_HMeshThreadIsRunning = true;

        // EXECUTE SOLVER
        hmgr->ExecuteHMesh();

        hscreen->m_HMeshThreadIsRunning = false;

        hscreen->GetScreenMgr()->SetUpdateFlag( true );
    }

    return 0;
}


void HeldenScreen::LoadSetChoice( Choice & c, int & index )
{
    c.ClearItems();

    Vehicle* veh = m_ScreenMgr->GetVehiclePtr();
    vector< string > set_name_vec = veh->GetSetNameVec();

    for ( int i = 0 ; i < ( int )set_name_vec.size() ; ++i )
    {
        c.AddItem( set_name_vec[i].c_str() );
    }

    c.UpdateItems();
    c.SetVal( index );
}

void HeldenScreen::Show()
{
    m_ScreenMgr->SetUpdateFlag( true );
    m_FLTK_Window->show();
}

void HeldenScreen::Hide()
{
    m_FLTK_Window->hide();
    m_ScreenMgr->SetUpdateFlag( true );
}

bool HeldenScreen::Update()
{
    Vehicle* vehiclePtr = m_ScreenMgr->GetVehiclePtr();


    vector< string > set_name_vec = vehiclePtr->GetSetNameVec();


    m_SelectedSetChoice.ClearItems();
    for ( int i = 0 ; i < ( int )set_name_vec.size() ; i++ )
    {
        m_SelectedSetChoice.AddItem( set_name_vec[i].c_str() );
    }
    m_SelectedSetChoice.UpdateItems();
    m_SelectedSetChoice.Update( HeldenMgr.m_SelectedSetIndex.GetID() );

    m_IGESFileOutput.Update( vehiclePtr->getExportFileName( vsp::HELDEN_IGES_TYPE ).c_str() );

    m_LenUnitChoice.Update( HeldenMgr.m_IGESLenUnit.GetID() );
    m_SplitSurfsToggle.Update( HeldenMgr.m_IGESSplitSurfs.GetID() );
    m_SplitSubSurfsToggle.Update( HeldenMgr.m_IGESSplitSubSurfs.GetID() );
    m_ToCubicToggle.Update( HeldenMgr.m_IGESToCubic.GetID() );
    m_ToCubicTolSlider.Update( HeldenMgr.m_IGESToCubicTol.GetID() );
    m_TrimTEToggle.Update( HeldenMgr.m_IGESTrimTE.GetID() );

    if ( !vehiclePtr->m_IGESToCubic() )
    {
        m_ToCubicTolSlider.Deactivate();
    }

    // Hendensurf Button
    if ( vehiclePtr->GetHeldenSurfCmd().empty() || m_HSurfThreadIsRunning )
    {
        m_HSurfButton.Deactivate();
    }
    else
    {
        m_HSurfButton.Activate();
    }
    // Kill Heldensurf Button
    if ( m_HSurfThreadIsRunning )
    {
        m_KillHSurfButton.Activate();
    }
    else
    {
        m_KillHSurfButton.Deactivate();
    }

    // Hendensurf Button
    if ( vehiclePtr->GetHeldenPatchCmd().empty() || m_HPatchThreadIsRunning )
    {
        m_HPatchButton.Deactivate();
    }
    else
    {
        m_HPatchButton.Activate();
    }
    // Kill Heldensurf Button
    if ( m_HPatchThreadIsRunning )
    {
        m_KillHPatchButton.Activate();
    }
    else
    {
        m_KillHPatchButton.Deactivate();
    }

    // Hendenmesh Button
    if ( vehiclePtr->GetHeldenMeshCmd().empty() || m_HMeshThreadIsRunning )
    {
        m_HMeshButton.Deactivate();
    }
    else
    {
        m_HMeshButton.Activate();
    }
    // Kill Heldensurf Button
    if ( m_HMeshThreadIsRunning )
    {
        m_KillHMeshButton.Activate();
    }
    else
    {
        m_KillHMeshButton.Deactivate();
    }

    m_FLTK_Window->redraw();
    return false;
}

// Callback for Link Browser
void HeldenScreen::CallBack( Fl_Widget* w )
{
    assert( m_ScreenMgr );

    m_ScreenMgr->SetUpdateFlag( true );

}

void HeldenScreen::CloseCallBack( Fl_Widget *w )
{
    Hide();
}

// Callback for all other GUI Devices
void HeldenScreen::GuiDeviceCallBack( GuiDevice* device )
{
    Vehicle* veh = m_ScreenMgr->GetVehiclePtr();

    assert( m_ScreenMgr );

    if( device == &m_SaveSurfFile )
    {
        string setupfile = HeldenMgr.FileInPathOf( "INPUT_HELDENSURFACE" );
        m_SurfFileBuffer->savefile( setupfile.c_str() );
    }
    else if( device == &m_ReadSurfFile )
    {
        string setupfile = HeldenMgr.FileInPathOf( "INPUT_HELDENSURFACE" );
        m_SurfFileBuffer->loadfile( setupfile.c_str() );
    }
    else if( device == &m_SavePatchFile )
    {
        string setupfile = HeldenMgr.FileInPathOf( "INPUT_HELDENPATCH" );
        m_PatchFileBuffer->savefile( setupfile.c_str() );
    }
    else if( device == &m_ReadPatchFile )
    {
        string hpfname = HeldenMgr.FileInPathOf( "INPUT_HELDENPATCH" );
        m_PatchFileBuffer->loadfile( hpfname.c_str() );
    }
    else if( device == &m_SaveMeshFile )
    {
        string setupfile = HeldenMgr.FileInPathOf( "INPUT_HELDENMESH" );
        m_MeshFileBuffer->savefile( setupfile.c_str() );
    }
    else if( device == &m_ReadMeshFile )
    {
        string setupfile = HeldenMgr.FileInPathOf( "INPUT_HELDENMESH" );
        m_MeshFileBuffer->loadfile( setupfile.c_str() );
    }
    else if ( device == & m_ExportIGESButton )
    {
        HeldenMgr.ExportIGESConvertRST();
    }
    else if ( device == &m_IGESFileSelect )
    {
        veh->setExportFileName( vsp::HELDEN_IGES_TYPE,
                                       m_ScreenMgr->GetSelectFileScreen()->FileChooser(
                                               "Select Helden IGES output file.", "*.igs" ) );
    }
    else if ( device == &m_InitHSurfButton )
    {
        HeldenMgr.InitHSurf();

        string setupfile = HeldenMgr.FileInPathOf( "INPUT_HELDENSURFACE" );
        m_SurfFileBuffer->loadfile( setupfile.c_str() );
    }
    else if ( device == &m_HSurfButton )
    {
        if( veh->GetHeldenSurfCmd().empty() || HeldenMgr.IsHSurfRunning() )
        { /* Do nothing. Should not be reachable, button should be deactivated.*/ }
        else
        {

            // Clear the solver console
            m_ConsoleBuffer->text( "" );


            m_HSurfProcess.StartThread( hsurf_thread_fun, (void *) &m_HSurfPair );

        }
    }
    else if( device == &m_KillHSurfButton )
    {
        HeldenMgr.KillHSurf();
    }
    else if ( device == &m_InitHPatchButton )
    {
        HeldenMgr.InitHPatch();

        string setupfile = HeldenMgr.FileInPathOf( "INPUT_HELDENPATCH" );
        m_PatchFileBuffer->loadfile( setupfile.c_str() );
    }
    else if ( device == &m_HPatchButton )
    {
        if( veh->GetHeldenPatchCmd().empty() || HeldenMgr.IsHPatchRunning() )
        { /* Do nothing. Should not be reachable, button should be deactivated.*/ }
        else
        {

            // Clear the solver console
            m_ConsoleBuffer->text( "" );


            m_HPatchProcess.StartThread( hpatch_thread_fun, (void *) &m_HPatchPair );

        }
    }
    else if( device == &m_KillHPatchButton )
    {
        HeldenMgr.KillHPatch();
    }
    else if ( device == &m_InitHMeshButton )
    {
        HeldenMgr.InitHMesh();

        string setupfile = HeldenMgr.FileInPathOf( "INPUT_HELDENMESH" );
        m_MeshFileBuffer->loadfile( setupfile.c_str() );
    }
    else if ( device == &m_HMeshButton )
    {
        if( veh->GetHeldenMeshCmd().empty() || HeldenMgr.IsHMeshRunning() )
        { /* Do nothing. Should not be reachable, button should be deactivated.*/ }
        else
        {

            // Clear the solver console
            m_ConsoleBuffer->text( "" );


            m_HMeshProcess.StartThread( hmesh_thread_fun, (void *) &m_HMeshPair );

        }
    }
    else if( device == &m_KillHMeshButton )
    {
        HeldenMgr.KillHMesh();
    }

    m_ScreenMgr->SetUpdateFlag( true );
}

// VSPAEROScreen::AddOutputText( Fl_Text_Display *display, const char *text )
//     This is used for the Solver tab to show the current results of the solver in the GUI
void HeldenScreen::AddOutputText( Fl_Text_Display *display, const string &text )
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

ProcessUtil* HeldenScreen::GetProcess( int id )
{
    if( id == HELDEN_SURF )
    {
        return &m_HSurfProcess;
    }
    else if( id == HELDEN_PATCH )
    {
        return &m_HPatchProcess;
    }
    else if( id == HELDEN_MESH )
    {
        return &m_HMeshProcess;
    }
    else
    {
        return NULL;
    }
}

Fl_Text_Display* HeldenScreen::GetDisplay( )
{
    return m_ConsoleDisplay;
}
