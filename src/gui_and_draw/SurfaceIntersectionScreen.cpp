//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//

// SurfaceIntersectionScreen.cpp: implementation of the SurfaceIntersectionScreen class.
//
//////////////////////////////////////////////////////////////////////

#ifdef WIN32
#undef _HAS_STD_BYTE
#define _HAS_STD_BYTE 0
#endif

#include "SurfaceIntersectionScreen.h"
#include "SurfaceIntersectionMgr.h"
#include "ModeMgr.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

SurfaceIntersectionScreen::SurfaceIntersectionScreen( ScreenMgr* mgr ) : TabScreen( mgr, 375, 545, "Trimmed Surfaces", "", 150 )
{
    m_Vehicle = m_ScreenMgr->GetVehiclePtr();

    m_FLTK_Window->callback( staticCloseCB, this );

    //=== Create Tabs ===//
    CreateGlobalTab();
    CreateDisplayTab();
    CreateOutputTab();
    CreateWakesTab();

    //=== Create Console Area ===//
    m_ConsoleLayout.SetGroupAndScreen( m_FLTK_Window, this );

    m_ConsoleLayout.AddY( m_ConsoleLayout.GetRemainY()
                        - 7 * m_ConsoleLayout.GetStdHeight()
                        - 2 * m_ConsoleLayout.GetGapHeight() );

    m_ConsoleLayout.AddYGap();
    m_ConsoleLayout.AddX(5);

    m_ConsoleLayout.AddSubGroupLayout( m_BorderConsoleLayout, m_ConsoleLayout.GetRemainX() - 5 ,
                                       m_ConsoleLayout.GetRemainY() - 5 );

    m_ConsoleDisplay = m_BorderConsoleLayout.AddFlTextDisplay( 115 );
    m_ConsoleBuffer = new Fl_Text_Buffer;
    m_ConsoleDisplay->buffer( m_ConsoleBuffer );
    m_FLTK_Window->resizable( m_ConsoleDisplay );

    m_BorderConsoleLayout.AddYGap();

    m_BorderConsoleLayout.AddButton( m_IntersectAndExport, "Intersect and Export" );
}

SurfaceIntersectionScreen::~SurfaceIntersectionScreen()
{
    m_ConsoleDisplay->buffer( nullptr );
    delete m_ConsoleBuffer;
}

void SurfaceIntersectionScreen::CreateGlobalTab()
{
    Fl_Group* globalTab = AddTab( "Global" );
    Fl_Group* globalTabGroup = AddSubGroup( globalTab, 5 );

    m_GlobalTabLayout.SetGroupAndScreen( globalTabGroup, this );

    //=== GLOBAL TAB INIT ===//

    m_GlobalTabLayout.SetButtonWidth( 175 );

    m_GlobalTabLayout.SetFitWidthFlag( true );
    m_GlobalTabLayout.SetSameLineFlag( false );

    m_GlobalTabLayout.AddDividerBox( "Geometry Control" );
    m_GlobalTabLayout.AddYGap();

    m_GlobalTabLayout.AddSlider( m_RelCurveTolSlider, "Curve Adaptation Tolerance", 0.01, "%7.5f" );
    m_GlobalTabLayout.AddYGap();

    m_GlobalTabLayout.AddButton( m_IntersectSubsurfaces, "Intersect Subsurfaces" );
    m_GlobalTabLayout.AddYGap();

    int bw = m_GlobalTabLayout.GetRemainX() / 2.0;
    m_GlobalTabLayout.SetButtonWidth( bw );
    m_GlobalTabLayout.SetChoiceButtonWidth( bw );

    m_GlobalTabLayout.SetSameLineFlag( true );
    m_GlobalTabLayout.SetChoiceButtonWidth( 0 );
    m_GlobalTabLayout.SetFitWidthFlag( false );
    m_GlobalTabLayout.AddButton( m_SetToggle, "Normal Set:" );
    m_GlobalTabLayout.SetFitWidthFlag( true );
    m_GlobalTabLayout.AddChoice(m_UseSet, "", bw);
    m_GlobalTabLayout.ForceNewLine();

    m_GlobalTabLayout.SetSameLineFlag( false );
    m_GlobalTabLayout.SetChoiceButtonWidth( bw );
    m_GlobalTabLayout.AddChoice(m_UseDegenSet, "Degen Set:" );

    m_GlobalTabLayout.SetSameLineFlag( true );
    m_GlobalTabLayout.SetChoiceButtonWidth( 0 );
    m_GlobalTabLayout.SetFitWidthFlag( false );
    m_GlobalTabLayout.AddButton( m_ModeToggle, "Mode:" );
    m_GlobalTabLayout.SetFitWidthFlag( true );
    m_GlobalTabLayout.AddChoice(m_ModeChoice, "", bw );
    m_GlobalTabLayout.ForceNewLine();

    m_ModeSetToggleGroup.Init( this );
    m_ModeSetToggleGroup.AddButton( m_SetToggle.GetFlButton() );
    m_ModeSetToggleGroup.AddButton( m_ModeToggle.GetFlButton() );

    m_GlobalTabLayout.SetSameLineFlag( false );

    m_GlobalTabLayout.AddYGap();
    m_GlobalTabLayout.AddButton( m_ToCubicToggle, "Demote Surfs to Cubic" );
    m_GlobalTabLayout.AddSlider( m_ToCubicTolSlider, "Cubic Tolerance", 10, "%5.4g", 0, true );

    globalTab->show();
}

void SurfaceIntersectionScreen::CreateDisplayTab()
{
    Fl_Group* displayTab = AddTab( "Display" );
    Fl_Group* displayTabGroup = AddSubGroup( displayTab, 5 );

    m_DisplayTabLayout.SetGroupAndScreen( displayTabGroup, this );

    m_DisplayTabLayout.SetButtonWidth( 175 );

    m_DisplayTabLayout.AddYGap();
    m_DisplayTabLayout.AddButton( m_ShowWakePreview, "Show Wake Preview" );
    m_DisplayTabLayout.AddYGap();
    m_DisplayTabLayout.AddButton( m_DrawIsect, "Show Intersection Curves");
    m_DisplayTabLayout.AddButton( m_DrawBorder, "Show Border Curves");
    m_DisplayTabLayout.AddYGap();
    m_DisplayTabLayout.AddButton( m_ShowCurve, "Show Curves");
    m_DisplayTabLayout.AddButton( m_ShowPts, "Show Points");
    m_DisplayTabLayout.AddYGap();
    m_DisplayTabLayout.AddButton( m_ShowRaw, "Show Raw Curve");
    m_DisplayTabLayout.AddButton( m_ShowBinAdapt, "Show Binary Adapted");

    displayTab->show();
}

void SurfaceIntersectionScreen::CreateOutputTab()
{
    Fl_Group* outputTab = AddTab( "Output" );
    Fl_Group* outputTabGroup = AddSubGroup( outputTab, 5 );

    m_OutputTabLayout.SetGroupAndScreen( outputTabGroup, this );

    m_OutputTabLayout.AddDividerBox("Export Options");
    m_OutputTabLayout.AddYGap();

    m_OutputTabLayout.SetButtonWidth( 175 );

    m_OutputTabLayout.AddButton( m_ExportRaw, "Export Raw Points" );

    m_OutputTabLayout.AddYGap();

    m_OutputTabLayout.AddDividerBox("Export File Names");
    m_OutputTabLayout.AddYGap();

    m_OutputTabLayout.SetFitWidthFlag( false );
    m_OutputTabLayout.SetSameLineFlag( true );

    int button_width = 55;
    m_OutputTabLayout.SetButtonWidth( button_width );
    int input_width = 280;
    m_OutputTabLayout.SetInputWidth( input_width );

    m_OutputTabLayout.AddButton(m_CurvFile, ".curv");
    m_OutputTabLayout.AddOutput(m_CurvOutput);
    m_OutputTabLayout.SetButtonWidth( m_OutputTabLayout.GetRemainX() );
    m_OutputTabLayout.AddButton(m_SelectCurvFile, "...");

    m_OutputTabLayout.ForceNewLine();
    m_OutputTabLayout.SetButtonWidth( button_width );
    m_OutputTabLayout.AddButton(m_Plot3DFile, ".p3d");
    m_OutputTabLayout.AddOutput(m_Plot3DOutput);
    m_OutputTabLayout.SetButtonWidth( m_OutputTabLayout.GetRemainX() );
    m_OutputTabLayout.AddButton(m_SelectPlot3DFile, "...");

    m_OutputTabLayout.AddYGap();

    m_OutputTabLayout.ForceNewLine();
    m_OutputTabLayout.SetFitWidthFlag( true );
    m_OutputTabLayout.AddDividerBox("Surfaces and Intersection Curves");
    m_OutputTabLayout.ForceNewLine();
    m_OutputTabLayout.SetFitWidthFlag( false );

    m_OutputTabLayout.SetButtonWidth( button_width );
    m_OutputTabLayout.AddButton(m_SrfFile, ".srf");
    m_OutputTabLayout.AddOutput(m_SrfOutput);
    m_OutputTabLayout.SetButtonWidth( m_OutputTabLayout.GetRemainX() );
    m_OutputTabLayout.AddButton(m_SelectSrfFile, "...");
    m_OutputTabLayout.ForceNewLine();

    m_OutputTabLayout.SetFitWidthFlag( true );
    m_OutputTabLayout.AddButton( m_XYZIntCurves, "Include X,Y,Z Intersection Curves");
    m_OutputTabLayout.SetFitWidthFlag( false );
    m_OutputTabLayout.ForceNewLine();

    m_OutputTabLayout.AddYGap();
    m_OutputTabLayout.SetFitWidthFlag( true );
    m_OutputTabLayout.AddDividerBox( "Trimmed CAD Options" );
    m_OutputTabLayout.ForceNewLine();
    m_OutputTabLayout.SetFitWidthFlag( false );
    m_OutputTabLayout.InitWidthHeightVals();

    m_OutputTabLayout.SetButtonWidth( m_OutputTabLayout.GetRemainX() / 4 );

    m_OutputTabLayout.AddButton( m_LabelIDToggle, "Geom ID" );
    m_OutputTabLayout.AddButton( m_LabelNameToggle, "Geom Name" );
    m_OutputTabLayout.AddButton( m_LabelSurfNoToggle, "Surf Number" );
    m_OutputTabLayout.AddButton( m_LabelSplitNoToggle, "Split Number" );

    m_OutputTabLayout.ForceNewLine();
    m_OutputTabLayout.SetSliderWidth( m_OutputTabLayout.GetRemainX() / 4 );
    m_OutputTabLayout.SetChoiceButtonWidth( m_OutputTabLayout.GetRemainX() / 4 );

    m_LabelDelimChoice.AddItem( "Comma" );
    m_LabelDelimChoice.AddItem( "Underscore" );
    m_LabelDelimChoice.AddItem( "Space" );
    m_LabelDelimChoice.AddItem( "None" );
    m_OutputTabLayout.AddChoice( m_LabelDelimChoice, "Delimiter" );

    m_LenUnitChoice.AddItem( "MM" );
    m_LenUnitChoice.AddItem( "CM" );
    m_LenUnitChoice.AddItem( "M" );
    m_LenUnitChoice.AddItem( "IN" );
    m_LenUnitChoice.AddItem( "FT" );
    //m_LenUnitChoice.AddItem( "YD" ); // FIXME: YD is not supported by both STEP and IGES
    m_OutputTabLayout.AddChoice( m_LenUnitChoice, "Length Unit" );
    m_OutputTabLayout.ForceNewLine();

    m_OutputTabLayout.AddYGap();
    m_OutputTabLayout.SetInputWidth( input_width );

    m_OutputTabLayout.SetButtonWidth( button_width );
    m_OutputTabLayout.AddButton( m_IGESFile, ".igs" );
    m_OutputTabLayout.AddOutput( m_IGESOutput );
    m_OutputTabLayout.SetButtonWidth( m_OutputTabLayout.GetRemainX() );
    m_OutputTabLayout.AddButton( m_SelectIGESFile, "..." );
    m_OutputTabLayout.ForceNewLine();

    m_OutputTabLayout.AddYGap();

    m_OutputTabLayout.SetButtonWidth( button_width );
    m_OutputTabLayout.AddButton( m_STEPFile, ".stp" );
    m_OutputTabLayout.AddOutput( m_STEPOutput );
    m_OutputTabLayout.SetButtonWidth( m_OutputTabLayout.GetRemainX() );
    m_OutputTabLayout.AddButton( m_SelectSTEPFile, "..." );
    m_OutputTabLayout.ForceNewLine();

    m_OutputTabLayout.SetFitWidthFlag( true );
    m_OutputTabLayout.SetSameLineFlag( false );

    m_OutputTabLayout.InitWidthHeightVals();
    m_OutputTabLayout.SetButtonWidth( 175 );

    //m_OutputTabLayout.SetButtonWidth( m_OutputTabLayout.GetRemainX() / 3 );
    //m_OutputTabLayout.AddButton( m_STEPMergePointsToggle, "Merge Points" );
    //m_OutputTabLayout.SetFitWidthFlag( true );
    m_OutputTabLayout.AddSlider( m_STEPTolSlider, "STEP Tolerance", 10, "%5.4g", 0, true );
    //m_OutputTabLayout.SetFitWidthFlag( false );
    //m_OutputTabLayout.ForceNewLine();

    m_OutputTabLayout.SetFitWidthFlag( false );
    m_OutputTabLayout.SetSameLineFlag( true );

    m_OutputTabLayout.SetButtonWidth( m_OutputTabLayout.GetRemainX() / 2 );
    m_OutputTabLayout.AddButton( m_STEPShell, "Shell Representation" );
    m_OutputTabLayout.AddButton( m_STEPBREP, "BREP Solid Representation" );
    m_OutputTabLayout.ForceNewLine();

    m_STEPRepGroup.Init( this );
    m_STEPRepGroup.AddButton( m_STEPShell.GetFlButton() );
    m_STEPRepGroup.AddButton( m_STEPBREP.GetFlButton() );

    outputTab->show();
}

void SurfaceIntersectionScreen::CreateWakesTab()
{
    m_WakesTab = AddTab( "Wakes" );
    Fl_Group* wakesTabGroup = AddSubGroup( m_WakesTab, 5 );

    m_WakesTabLayout.SetGroupAndScreen( wakesTabGroup, this );

    m_WakesTabLayout.AddYGap();

    m_WakesTabLayout.SetButtonWidth( 175.0 );
    m_WakesTabLayout.AddSlider( m_ScaleWake, "Scale Wake", 10.0, "%7.5f" );
    m_WakesTabLayout.AddYGap();
    m_WakesTabLayout.AddSlider( m_WakeAngle, "Wake Angle", 10.0, "%7.5f" );

    m_WakesTabLayout.ForceNewLine();

    m_WakesTabLayout.SetFitWidthFlag( true );
    m_WakesTabLayout.SetSameLineFlag( true );

    m_WakesTabLayout.SetChoiceButtonWidth( 100 );
    m_WakesTabLayout.AddChoice( m_Comp, "Comp", 100 );

    m_WakesTabLayout.SetFitWidthFlag( false );
    m_WakesTabLayout.SetButtonWidth( 100 );
    m_WakesTabLayout.AddButton( m_AddWake, "Add Wake" );

    m_WakesTab->show();
}

bool SurfaceIntersectionScreen::Update()
{
    TabScreen::Update();

    m_ScreenMgr->LoadSetChoice( {&m_UseSet, &m_UseDegenSet},
        {m_Vehicle->GetISectSettingsPtr()->m_SelectedSetIndex.GetID(),
        m_Vehicle->GetISectSettingsPtr()->m_SelectedDegenSetIndex.GetID()}, true );

    m_ScreenMgr->LoadModeChoice( m_ModeChoice, m_ModeIDs, m_Vehicle->GetISectSettingsPtr()->m_ModeID );

    m_ModeSetToggleGroup.Update( m_Vehicle->GetISectSettingsPtr()->m_UseMode.GetID() );

    if ( ModeMgr.GetNumModes() == 0 )
    {
        if ( m_Vehicle->GetISectSettingsPtr()->m_UseMode() )
        {
            m_Vehicle->GetISectSettingsPtr()->m_UseMode.Set( false );
            m_ScreenMgr->SetUpdateFlag( true );
        }
        m_ModeToggle.Deactivate();
    }
    else
    {
        m_ModeToggle.Activate();
    }

    if ( m_Vehicle->GetISectSettingsPtr()->m_UseMode() )
    {
        m_ModeChoice.Activate();
        m_UseSet.Deactivate();
        m_UseDegenSet.Deactivate();

        Mode *m = ModeMgr.GetMode( m_Vehicle->GetISectSettingsPtr()->m_ModeID );
        if ( m )
        {
            if ( m_Vehicle->GetISectSettingsPtr()->m_SelectedSetIndex() != m->m_NormalSet() ||
                 m_Vehicle->GetISectSettingsPtr()->m_SelectedDegenSetIndex() != m->m_DegenSet() )
            {
                m_Vehicle->GetISectSettingsPtr()->m_SelectedSetIndex = m->m_NormalSet();
                m_Vehicle->GetISectSettingsPtr()->m_SelectedDegenSetIndex = m->m_DegenSet();
                m_ScreenMgr->SetUpdateFlag( true );
            }
        }
    }
    else
    {
        m_ModeChoice.Deactivate();
        m_UseSet.Activate();
        m_UseDegenSet.Activate();
    }


    if ( SurfaceIntersectionMgr.GetMeshInProgress() )
    {
        m_IntersectAndExport.Deactivate();
    }
    else
    {
        m_IntersectAndExport.Activate();
    }

    UpdateGlobalTab();
    UpdateDisplayTab();
    UpdateOutputTab();
    UpdateWakesTab();

    if ( !SurfaceIntersectionMgr.GetMeshInProgress() )
    {
        SurfaceIntersectionMgr.UpdateWakes();
    }

    m_FLTK_Window->redraw();

    return true;
}

void SurfaceIntersectionScreen::UpdateGlobalTab()
{
    m_RelCurveTolSlider.Update( m_Vehicle->GetISectSettingsPtr()->m_RelCurveTol.GetID() );

    //===== Geometry Control =====//
    m_IntersectSubsurfaces.Update( m_Vehicle->GetISectSettingsPtr()->m_IntersectSubSurfs.GetID() );

    m_ToCubicToggle.Update( m_Vehicle->GetISectSettingsPtr()->m_DemoteSurfsCubicFlag.GetID() );
    m_ToCubicTolSlider.Update( m_Vehicle->GetISectSettingsPtr()->m_CubicSurfTolerance.GetID() );

    if ( m_Vehicle->GetISectSettingsPtr()->m_DemoteSurfsCubicFlag.Get() )
    {
        m_ToCubicTolSlider.Activate();
    }
    else
    {
        m_ToCubicTolSlider.Deactivate();
    }
}

void SurfaceIntersectionScreen::UpdateDisplayTab()
{
    //===== Display Tab Toggle Update =====//

    m_ShowWakePreview.Update( m_Vehicle->GetISectSettingsPtr()->m_DrawSourceWakeFlag.GetID() );

    m_DrawIsect.Update( m_Vehicle->GetISectSettingsPtr()->m_DrawIsectFlag.GetID() );
    m_DrawBorder.Update( m_Vehicle->GetISectSettingsPtr()->m_DrawBorderFlag.GetID() );

    m_ShowRaw.Update( m_Vehicle->GetISectSettingsPtr()->m_DrawRawFlag.GetID() );
    m_ShowBinAdapt.Update( m_Vehicle->GetISectSettingsPtr()->m_DrawBinAdaptFlag.GetID() );

    m_ShowCurve.Update( m_Vehicle->GetISectSettingsPtr()->m_DrawCurveFlag.GetID() );
    m_ShowPts.Update( m_Vehicle->GetISectSettingsPtr()->m_DrawPntsFlag.GetID() );

    SurfaceIntersectionMgr.UpdateDisplaySettings();
}

void SurfaceIntersectionScreen::UpdateOutputTab()
{
    string curvname = m_Vehicle->GetISectSettingsPtr()->GetExportFileName( vsp::INTERSECT_CURV_FILE_NAME );
    m_CurvOutput.Update( StringUtil::truncateFileName( curvname, 40 ).c_str() );
    string plot3dname = m_Vehicle->GetISectSettingsPtr()->GetExportFileName( vsp::INTERSECT_PLOT3D_FILE_NAME );
    m_Plot3DOutput.Update( StringUtil::truncateFileName( plot3dname, 40 ).c_str() );
    string srfname = m_Vehicle->GetISectSettingsPtr()->GetExportFileName( vsp::INTERSECT_SRF_FILE_NAME );
    m_SrfOutput.Update( StringUtil::truncateFileName( srfname, 40 ).c_str() );
    string igsname = m_Vehicle->GetISectSettingsPtr()->GetExportFileName( vsp::INTERSECT_IGES_FILE_NAME );
    m_IGESOutput.Update( StringUtil::truncateFileName( igsname, 40 ).c_str() );
    string stpname = m_Vehicle->GetISectSettingsPtr()->GetExportFileName( vsp::INTERSECT_STEP_FILE_NAME );
    m_STEPOutput.Update( StringUtil::truncateFileName( stpname, 40 ).c_str() );

    //==== Update File Output Flags ====//
    m_CurvFile.Update( m_Vehicle->GetISectSettingsPtr()->GetExportFileFlag( vsp::INTERSECT_CURV_FILE_NAME )->GetID() );
    m_Plot3DFile.Update( m_Vehicle->GetISectSettingsPtr()->GetExportFileFlag( vsp::INTERSECT_PLOT3D_FILE_NAME )->GetID() );
    m_SrfFile.Update( m_Vehicle->GetISectSettingsPtr()->GetExportFileFlag( vsp::INTERSECT_SRF_FILE_NAME )->GetID() );
    m_IGESFile.Update( m_Vehicle->GetISectSettingsPtr()->GetExportFileFlag( vsp::INTERSECT_IGES_FILE_NAME )->GetID() );
    m_STEPFile.Update( m_Vehicle->GetISectSettingsPtr()->GetExportFileFlag( vsp::INTERSECT_STEP_FILE_NAME )->GetID() );

    m_ExportRaw.Update( m_Vehicle->GetISectSettingsPtr()->m_ExportRawFlag.GetID() );
    m_XYZIntCurves.Update( m_Vehicle->GetISectSettingsPtr()->m_XYZIntCurveFlag.GetID() );

    //m_STEPMergePointsToggle.Update( m_Vehicle->GetISectSettingsPtr()->m_STEPMergePoints.GetID() );
    m_STEPTolSlider.Update( m_Vehicle->GetISectSettingsPtr()->m_STEPTol.GetID() );
    m_STEPRepGroup.Update( m_Vehicle->GetISectSettingsPtr()->m_STEPRepresentation.GetID() );
    m_LenUnitChoice.Update( m_Vehicle->GetISectSettingsPtr()->m_CADLenUnit.GetID() );
    m_LabelIDToggle.Update( m_Vehicle->GetISectSettingsPtr()->m_CADLabelID.GetID() );
    m_LabelNameToggle.Update( m_Vehicle->GetISectSettingsPtr()->m_CADLabelName.GetID() );
    m_LabelSurfNoToggle.Update( m_Vehicle->GetISectSettingsPtr()->m_CADLabelSurfNo.GetID() );
    m_LabelSplitNoToggle.Update( m_Vehicle->GetISectSettingsPtr()->m_CADLabelSplitNo.GetID() );
    m_LabelDelimChoice.Update( m_Vehicle->GetISectSettingsPtr()->m_CADLabelDelim.GetID() );

    if ( !m_Vehicle->GetISectSettingsPtr()->GetExportFileFlag( vsp::INTERSECT_STEP_FILE_NAME )->Get() )
    {
        //m_STEPMergePointsToggle.Deactivate();
        m_STEPTolSlider.Deactivate();
        m_STEPRepGroup.Deactivate();
    }
    else
    {
        //m_STEPMergePointsToggle.Activate();
        m_STEPTolSlider.Activate();
        m_STEPRepGroup.Activate();
    }

    if ( !m_Vehicle->GetISectSettingsPtr()->GetExportFileFlag( vsp::INTERSECT_STEP_FILE_NAME )->Get() &&
         !m_Vehicle->GetISectSettingsPtr()->GetExportFileFlag( vsp::INTERSECT_IGES_FILE_NAME )->Get() )
    {
        m_LabelIDToggle.Deactivate();
        m_LabelNameToggle.Deactivate();
        m_LabelSurfNoToggle.Deactivate();
        m_LenUnitChoice.Deactivate();
        m_LabelSplitNoToggle.Deactivate();
        m_LabelDelimChoice.Deactivate();
    }
    else
    {
        m_LabelIDToggle.Activate();
        m_LabelNameToggle.Activate();
        m_LabelSurfNoToggle.Activate();
        m_LenUnitChoice.Activate();
        m_LabelSplitNoToggle.Activate();
        m_LabelDelimChoice.Activate();
    }
}

void SurfaceIntersectionScreen::UpdateWakesTab()
{
    m_Comp.ClearItems();

    map< string, int > compIDMap;
    map< string, int > wingCompIDMap;
    m_WingGeomVec.clear();

    vector < string > m_GeomVec = m_Vehicle->GetGeomVec();

    int iwing = 0;
    for ( int i = 0; i < (int)m_GeomVec.size(); ++i )
    {
        char str[256];
        Geom* g = m_Vehicle->FindGeom( m_GeomVec[i] );
        if ( g )
        {
            snprintf( str, sizeof( str ), "%d_%s", i, g->GetName().c_str() );
            if ( g->HasWingTypeSurfs() )
            {
                m_Comp.AddItem( str );
                wingCompIDMap[m_GeomVec[i]] = iwing++;
                m_WingGeomVec.push_back( m_GeomVec[i] );
            }
            compIDMap[m_GeomVec[i]] = i;
        }
    }

    m_Comp.UpdateItems();

    if ( m_WingGeomVec.size() == 0 )
    {
        m_WakesTab->deactivate();
        return;
    }
    else
    {
        m_WakesTab->activate();
    }

    //===== Set WakeGeomID and wake component selection for wake tab =====//
    string wakeGeomID = SurfaceIntersectionMgr.GetWakeGeomID();
    Geom* wakeGeom = m_Vehicle->FindGeom( wakeGeomID );

    if ( ( !wakeGeom || wakeGeomID.length() == 0 ) && m_WingGeomVec.size() > 0 )
    {
        // Handle case default case.
        wakeGeomID = m_WingGeomVec[0];
        SurfaceIntersectionMgr.SetWakeGeomID( wakeGeomID );
        wakeGeom = m_Vehicle->FindGeom( wakeGeomID );
    }

    m_Comp.SetVal( wingCompIDMap[wakeGeomID] );

    //===== Update Wake Parms =====//
    if ( wakeGeom )
    {
        m_AddWake.Update( wakeGeom->m_WakeActiveFlag.GetID() );
        m_ScaleWake.Update( wakeGeom->m_WakeScale.GetID() );
        m_WakeAngle.Update( wakeGeom->m_WakeAngle.GetID() );

        if ( wakeGeom->m_WakeActiveFlag() )
        {
            m_ScaleWake.Activate();
            m_WakeAngle.Activate();
        }
        else
        {
            m_ScaleWake.Deactivate();
            m_WakeAngle.Deactivate();
        }
    }
}

void SurfaceIntersectionScreen::AddOutputText( const string &text )
{
    Fl::lock();
    m_ConsoleBuffer->append( text.c_str() );
    m_ConsoleDisplay->insert_position( m_ConsoleDisplay->buffer()->length() );
    m_ConsoleDisplay->show_insert_position();
    Fl::unlock();
}

void SurfaceIntersectionScreen::LoadDrawObjs( vector< DrawObj* > &draw_obj_vec )
{
    if ( IsShown() )
    {
        SurfaceIntersectionMgr.LoadDrawObjs( draw_obj_vec );
    }
}

bool SurfaceIntersectionScreen::GetVisBndBox( BndBox &bbox )
{
    if ( IsShown() )
    {
        return SurfaceIntersectionMgr.GetVisBndBox( bbox );
    }
    return false;
}

void SurfaceIntersectionScreen::Show()
{
    if ( m_Vehicle->GetISectSettingsPtr()->m_UseMode() )
    {
        Mode *m = ModeMgr.GetMode( m_Vehicle->GetISectSettingsPtr()->m_ModeID );
        if ( m )
        {
            m->ApplySettings();
        }
    }

    m_ScreenMgr->SetUpdateFlag( true );
    TabScreen::Show();
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
    GuiDeviceWakesTabCallback( device );

    if ( device == &m_IntersectAndExport )
    {
        SurfaceIntersectionMgr.SetMeshInProgress( true );
        m_IntersectProcess.StartThread( surfint_thread_fun, nullptr );
    }

    m_ScreenMgr->SetUpdateFlag( true );
}

void SurfaceIntersectionScreen::GuiDeviceGlobalTabCallback( GuiDevice* device )
{
    if ( device == &m_ModeChoice || device == &m_ModeSetToggleGroup )
    {
        if ( m_Vehicle->GetISectSettingsPtr()->m_UseMode() )
        {
            int indx = m_ModeChoice.GetVal();
            if ( indx >= 0  && indx < m_ModeIDs.size() )
            {
                m_Vehicle->GetISectSettingsPtr()->m_ModeID = m_ModeIDs[ indx ];
            }
            else
            {
                m_Vehicle->GetISectSettingsPtr()->m_ModeID = "";
            }

            Mode *m = ModeMgr.GetMode( m_Vehicle->GetISectSettingsPtr()->m_ModeID );
            if ( m )
            {
                m->ApplySettings();
            }
        }
    }
}

void SurfaceIntersectionScreen::GuiDeviceOutputTabCallback( GuiDevice* device )
{
    if ( device == &m_SelectSrfFile )
    {
        string newfile = m_ScreenMgr->FileChooser( "Select .srf file.", "*.srf", vsp::SAVE );
        if ( newfile.compare( "" ) != 0 )
        {
            m_Vehicle->GetISectSettingsPtr()->SetExportFileName( newfile, vsp::INTERSECT_SRF_FILE_NAME );
        }
    }
    else if ( device == &m_SelectCurvFile )
    {
        string newfile = m_ScreenMgr->FileChooser( "Select GridTool .curv file.", "*.curv", vsp::SAVE );
        if ( newfile.compare( "" ) != 0 )
        {
            m_Vehicle->GetISectSettingsPtr()->SetExportFileName( newfile, vsp::INTERSECT_CURV_FILE_NAME );
        }
    }
    else if ( device == &m_SelectPlot3DFile )
    {
        string newfile = m_ScreenMgr->FileChooser( "Select Plot3D .p3d file.", "*.p3d", vsp::SAVE );
        if ( newfile.compare( "" ) != 0 )
        {
            m_Vehicle->GetISectSettingsPtr()->SetExportFileName( newfile, vsp::INTERSECT_PLOT3D_FILE_NAME );
        }
    }
    else if ( device == &m_SelectIGESFile )
    {
        string newfile = m_ScreenMgr->FileChooser( "Select IGES .igs file.", "*.igs", vsp::SAVE );
        if ( newfile.compare( "" ) != 0 )
        {
            m_Vehicle->GetISectSettingsPtr()->SetExportFileName( newfile, vsp::INTERSECT_IGES_FILE_NAME );
        }
    }
    else if ( device == &m_SelectSTEPFile )
    {
        string newfile = m_ScreenMgr->FileChooser( "Select STEP .stp file.", "*.stp", vsp::SAVE );
        if ( newfile.compare( "" ) != 0 )
        {
            m_Vehicle->GetISectSettingsPtr()->SetExportFileName( newfile, vsp::INTERSECT_STEP_FILE_NAME );
        }
    }
}

void SurfaceIntersectionScreen::GuiDeviceWakesTabCallback( GuiDevice* device )
{
    if ( device == &m_Comp )
    {
        SurfaceIntersectionMgr.SetWakeGeomID( m_WingGeomVec[m_Comp.GetVal()] );
    }
}