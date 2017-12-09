//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//

// SurfaceIntersectionScreen.cpp: implementation of the SurfaceIntersectionScreen class.
//
//////////////////////////////////////////////////////////////////////

#include "SurfaceIntersectionScreen.h"
#include "SurfaceIntersectionMgr.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

SurfaceIntersectionScreen::SurfaceIntersectionScreen( ScreenMgr* mgr ) : TabScreen( mgr, 415, 622, "Surface Intersection", 150 )
{
    m_Vehicle = m_ScreenMgr->GetVehiclePtr();

    m_FLTK_Window->callback( staticCloseCB, this );

    //=== Create Tabs ===//
    CreateGlobalTab();
    CreateDisplayTab();
    CreateOutputTab();

    //=== Create Console Area ===//
    m_ConsoleLayout.SetGroupAndScreen( m_FLTK_Window, this );

    m_ConsoleLayout.AddY( m_ConsoleLayout.GetRemainY()
                        - 7 * m_ConsoleLayout.GetStdHeight()
                        - 2.0 * m_ConsoleLayout.GetGapHeight() );

    m_ConsoleLayout.AddYGap();
    m_ConsoleLayout.AddX(5);

    m_ConsoleLayout.AddSubGroupLayout( m_BorderConsoleLayout, m_ConsoleLayout.GetRemainX() - 5.0,
                                       m_ConsoleLayout.GetRemainY() - 5.0);

    m_ConsoleDisplay = m_BorderConsoleLayout.AddFlTextDisplay( 115 );
    m_ConsoleBuffer = new Fl_Text_Buffer;
    m_ConsoleDisplay->buffer( m_ConsoleBuffer );

    m_BorderConsoleLayout.AddYGap();

    m_BorderConsoleLayout.AddButton(m_MeshAndExport, "Mesh and Export");
}

SurfaceIntersectionScreen::~SurfaceIntersectionScreen()
{
}

void SurfaceIntersectionScreen::CreateGlobalTab()
{
    Fl_Group* globalTab = AddTab( "Global" );
    Fl_Group* globalTabGroup = AddSubGroup( globalTab, 5 );

    m_GlobalTabLayout.SetGroupAndScreen( globalTabGroup, this );

    //=== GLOBAL TAB INIT ===//

    m_GlobalTabLayout.AddDividerBox("Global Mesh Control");

    m_GlobalTabLayout.AddYGap();

    m_GlobalTabLayout.SetButtonWidth(175.0);

    m_GlobalTabLayout.AddYGap();
    m_GlobalTabLayout.AddYGap();
    m_GlobalTabLayout.AddDividerBox("Global Source Control");
    m_GlobalTabLayout.AddYGap();

    m_GlobalTabLayout.SetFitWidthFlag( false );
    m_GlobalTabLayout.SetSameLineFlag( true );

    m_GlobalTabLayout.SetFitWidthFlag( true );
    m_GlobalTabLayout.SetSameLineFlag( false );

    m_GlobalTabLayout.AddDividerBox("Geometry Control");
    m_GlobalTabLayout.AddYGap();
    m_GlobalTabLayout.AddButton(m_IntersectSubsurfaces, "Intersect Subsurfaces");
    m_GlobalTabLayout.AddYGap();

    m_GlobalTabLayout.SetChoiceButtonWidth(m_GlobalTabLayout.GetRemainX() / 2.0);
    m_GlobalTabLayout.AddChoice(m_UseSet, "Use Set");

    globalTab->show();
}

void SurfaceIntersectionScreen::CreateDisplayTab()
{
    Fl_Group* displayTab = AddTab( "Display" );
    Fl_Group* displayTabGroup = AddSubGroup( displayTab, 5 );

    m_DisplayTabLayout.SetGroupAndScreen( displayTabGroup, this );

    displayTab->show();
}

void SurfaceIntersectionScreen::CreateOutputTab()
{
    Fl_Group* outputTab = AddTab( "Output" );
    Fl_Group* outputTabGroup = AddSubGroup( outputTab, 5 );

    m_OutputTabLayout.SetGroupAndScreen( outputTabGroup, this );

    m_OutputTabLayout.AddDividerBox("Export File Names");
    m_OutputTabLayout.AddYGap();

    m_OutputTabLayout.SetFitWidthFlag( false );
    m_OutputTabLayout.SetSameLineFlag( true );

    m_OutputTabLayout.SetButtonWidth(55);
    m_OutputTabLayout.SetInputWidth(300);

    m_OutputTabLayout.SetFitWidthFlag( true );
    m_OutputTabLayout.AddDividerBox("Surfaces and Intersection Curves");
    m_OutputTabLayout.ForceNewLine();
    m_OutputTabLayout.SetFitWidthFlag( false );

    m_OutputTabLayout.AddButton(m_SrfFile, ".srf");
    m_OutputTabLayout.AddOutput(m_SrfOutput);
    m_OutputTabLayout.AddButton(m_SelectSrfFile, "...");
    m_OutputTabLayout.ForceNewLine();

    m_OutputTabLayout.SetFitWidthFlag( true );
    m_OutputTabLayout.AddButton( m_XYZIntCurves, "Include X,Y,Z Intersection Curves");
    m_OutputTabLayout.SetFitWidthFlag( false );
    m_OutputTabLayout.ForceNewLine();
    m_OutputTabLayout.AddYGap();

    outputTab->show();
}

bool SurfaceIntersectionScreen::Update()
{
    LoadSetChoice();

    if ( SurfaceIntersectionMgr.GetMeshInProgress() )
    {
        m_MeshAndExport.Deactivate();
    }
    else
    {
        m_MeshAndExport.Activate();
    }

    //==== Load Geom Choice ====//
    m_GeomVec = m_Vehicle->GetGeomVec();

    map< string, int > compIDMap;
    map< string, int > wingCompIDMap;
    m_WingGeomVec.clear();


    int iwing = 0;
    for ( int i = 0 ; i < ( int )m_GeomVec.size() ; ++i )
    {
        char str[256];
        Geom* g = m_Vehicle->FindGeom( m_GeomVec[i] );
        if ( g )
        {
            sprintf( str, "%d_%s", i, g->GetName().c_str() );
            if( g->HasWingTypeSurfs() )
            {
                wingCompIDMap[ m_GeomVec[i] ] = iwing++;
                m_WingGeomVec.push_back( m_GeomVec[i] );
            }
            compIDMap[ m_GeomVec[i] ] = i;
        }
    }

    //===== Set FarGeomID and Far Component Selection for Domain Tab =====//
    string farGeomID = m_Vehicle->GetCfdSettingsPtr()->GetFarGeomID();
    if( farGeomID.length() == 0 && m_GeomVec.size() > 0 )
    {
        // Handle case default case.
        farGeomID = m_GeomVec[0];
        m_Vehicle->GetCfdSettingsPtr()->SetFarGeomID( farGeomID );
    }

    UpdateGlobalTab();
    UpdateDisplayTab();
    UpdateOutputTab();

    m_FLTK_Window->redraw();

    return false;
}

void SurfaceIntersectionScreen::UpdateGlobalTab()
{
    //===== Geometry Control =====//
    m_IntersectSubsurfaces.Update( m_Vehicle->GetCfdSettingsPtr()->m_IntersectSubSurfs.GetID() );
}

void SurfaceIntersectionScreen::UpdateDisplayTab()
{
}

void SurfaceIntersectionScreen::UpdateOutputTab()
{
    string srfname = m_Vehicle->GetCfdSettingsPtr()->GetExportFileName( vsp::CFD_SRF_FILE_NAME );
    m_SrfOutput.Update( truncateFileName( srfname, 40 ).c_str() );

    //==== Update File Output Flags ====//
    m_SrfFile.Update( m_Vehicle->GetCfdSettingsPtr()->GetExportFileFlag( vsp::CFD_SRF_FILE_NAME )->GetID() );
    m_XYZIntCurves.Update( m_Vehicle->GetCfdSettingsPtr()->m_XYZIntCurveFlag.GetID() );
}

void SurfaceIntersectionScreen::LoadSetChoice()
{
    m_UseSet.ClearItems();

    vector< string > set_name_vec = m_Vehicle->GetSetNameVec();

    for ( int i = 0 ; i < ( int )set_name_vec.size() ; ++i )
    {
        m_UseSet.AddItem( set_name_vec[i].c_str() );
    }

    m_UseSet.UpdateItems();
    m_UseSet.SetVal( m_Vehicle->GetCfdSettingsPtr()->m_SelectedSetIndex() );
}

void SurfaceIntersectionScreen::AddOutputText( const string &text )
{
    m_ConsoleBuffer->append( text.c_str() );
    m_ConsoleDisplay->insert_position( m_ConsoleDisplay->buffer()->length() );
    m_ConsoleDisplay->show_insert_position();
}

string SurfaceIntersectionScreen::truncateFileName( const string &fn, int len )
{
    string trunc( fn );
    if ( (int)trunc.length() > len )
    {
        trunc.erase( 0, trunc.length() - len );
        trunc.replace( 0, 3, "..." );
    }
    return trunc;
}

void SurfaceIntersectionScreen::LoadDrawObjs( vector< DrawObj* > &draw_obj_vec )
{
    if ( IsShown() )
    {
        SurfaceIntersectionMgr.LoadDrawObjs( draw_obj_vec );
    }
}

void SurfaceIntersectionScreen::Show()
{
    m_ScreenMgr->SetUpdateFlag( true );
    m_FLTK_Window->show();
}

void SurfaceIntersectionScreen::Hide()
{
    m_FLTK_Window->hide();
    m_ScreenMgr->SetUpdateFlag( true );
}

void SurfaceIntersectionScreen::CallBack( Fl_Widget* w )
{
    assert( m_ScreenMgr );

    m_ScreenMgr->SetUpdateFlag( true );
}

void SurfaceIntersectionScreen::CloseCallBack( Fl_Widget *w )
{
    Hide();
}

#ifdef WIN32
DWORD WINAPI surfintmonitorfun( LPVOID data )
#else
void * surfintmonitorfun( void *data )
#endif
{
    SurfaceIntersectionScreen *cs = ( SurfaceIntersectionScreen * ) data;

    if( cs )
    {
        unsigned long nread = 1;

        bool running = true;

        while( running || nread > 0 )
        {
            running = SurfaceIntersectionMgr.GetMeshInProgress();
            nread = 0;

            int ig = SurfaceIntersectionMgr.m_OutStream.tellg();
            SurfaceIntersectionMgr.m_OutStream.seekg( 0, SurfaceIntersectionMgr.m_OutStream.end );
            nread = (int)(SurfaceIntersectionMgr.m_OutStream.tellg()) - ig;
            SurfaceIntersectionMgr.m_OutStream.seekg( ig );

            if( nread > 0 )
            {
                char * buffer = new char [nread+1];

                SurfaceIntersectionMgr.m_OutStream.read( buffer, nread );
                buffer[nread]=0;

                Fl::lock();
                // Any FL calls must occur between Fl::lock() and Fl::unlock().
                cs->AddOutputText( buffer );
                Fl::unlock();

                delete[] buffer;
            }
            SleepForMilliseconds( 100 );
        }
        cs->GetScreenMgr()->SetUpdateFlag( true );
    }

    return 0;
}

#ifdef WIN32
DWORD WINAPI surfint_thread_fun( LPVOID data )
#else
void * surfint_thread_fun( void *data )
#endif
{
    SurfaceIntersectionMgr.IntersectSurfaces();

    return 0;
}

void SurfaceIntersectionScreen::GuiDeviceCallBack( GuiDevice* device )
{
    assert( m_ScreenMgr );

    GuiDeviceGlobalTabCallback( device );
    GuiDeviceOutputTabCallback( device );

    if ( device == &m_MeshAndExport )
    {
        SurfaceIntersectionMgr.SetMeshInProgress( true );
        m_CFDMeshProcess.StartThread( surfint_thread_fun, NULL );

        m_MonitorProcess.StartThread( surfintmonitorfun, ( void* ) this );
    }

    m_ScreenMgr->SetUpdateFlag( true );
}

void SurfaceIntersectionScreen::GuiDeviceGlobalTabCallback( GuiDevice* device )
{
    //Use Set
    if ( device == &m_UseSet )
    {
        m_Vehicle->GetCfdSettingsPtr()->m_SelectedSetIndex = m_UseSet.GetVal();
    }
}

void SurfaceIntersectionScreen::GuiDeviceOutputTabCallback( GuiDevice* device )
{
    if ( device == &m_SelectSrfFile )
    {
        string newfile = m_ScreenMgr->GetSelectFileScreen()->FileChooser( "Select .srf file.", "*.srf" );
        if ( newfile.compare( "" ) != 0 )
        {
            m_Vehicle->GetCfdSettingsPtr()->SetExportFileName( newfile, vsp::CFD_SRF_FILE_NAME );
        }
    }
}
