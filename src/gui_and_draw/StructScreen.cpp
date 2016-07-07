//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//

// StructScreen.cpp: implementation of the StructScreen class.
//
//////////////////////////////////////////////////////////////////////

#include "StructScreen.h"
#include "FeaMeshMgr.h"
#include "APIDefines.h"
using namespace vsp;

//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

StructScreen::StructScreen( ScreenMgr* mgr ) : TabScreen( mgr, 415, 622, "FEA Mesh", 150 )
{
    m_FLTK_Window->callback( staticCloseCB, this );

    Fl_Group* globalTab = AddTab( "Global" );
    Fl_Group* globalTabGroup = AddSubGroup( globalTab, 5 );
    Fl_Group* outputTab = AddTab( "Output" );
    Fl_Group* outputTabGroup = AddSubGroup( outputTab, 5 );
    Fl_Group* structTab = AddTab( "Structure" );
    Fl_Group* structTabGroup = AddSubGroup( structTab, 5 );

    m_GlobalTabLayout.SetGroupAndScreen( globalTabGroup, this );

    //=== GLOBAL TAB INIT ===//

    m_GlobalTabLayout.AddDividerBox("Global Mesh Control");

    m_GlobalTabLayout.AddYGap();

    m_GlobalTabLayout.SetButtonWidth(175.0);
    m_GlobalTabLayout.AddSlider(m_MaxEdgeLen, "Max Edge Len", 1.0, "%7.5f");
    m_GlobalTabLayout.AddSlider(m_MinEdgeLen, "Min Edge Len", 1.0, "%7.5f");
    m_GlobalTabLayout.AddSlider(m_MaxGap, "Max Gap", 1.0, "%7.5f");
    m_GlobalTabLayout.AddSlider(m_NumCircleSegments, "Num Circle Segments", 100.0, "%7.5f");
    m_GlobalTabLayout.AddSlider(m_GrowthRatio, "Growth Ratio", 2.0, "%7.5f");

    m_GlobalTabLayout.AddYGap();
    m_GlobalTabLayout.AddButton(m_Rig3dGrowthLimit, "Rigorous 3D Growth Limiting");
    m_GlobalTabLayout.AddYGap();
    m_GlobalTabLayout.AddDividerBox("Global Source Control");
    m_GlobalTabLayout.AddYGap();

    m_GlobalTabLayout.SetFitWidthFlag( false );
    m_GlobalTabLayout.SetSameLineFlag( true );

    m_GlobalTabLayout.SetButtonWidth(20.0);
    m_GlobalTabLayout.AddButton(m_GlobSrcAdjustLenLftLft, "<<");
    m_GlobalTabLayout.AddButton(m_GlobSrcAdjustLenLft, "<");
    m_GlobalTabLayout.AddLabel("Adjust Len", 100.0);
    m_GlobalTabLayout.AddButton(m_GlobSrcAdjustLenRht, ">");
    m_GlobalTabLayout.AddButton(m_GlobSrcAdjustLenRhtRht, ">>");
    m_GlobalTabLayout.AddX(45.0);
    m_GlobalTabLayout.AddButton(m_GlobSrcAdjustRadLftLft, "<<");
    m_GlobalTabLayout.AddButton(m_GlobSrcAdjustRadLft, "<");
    m_GlobalTabLayout.AddLabel("Adjust Rad", 100.0);
    m_GlobalTabLayout.AddButton(m_GlobSrcAdjustRadRht, ">");
    m_GlobalTabLayout.AddButton(m_GlobSrcAdjustRadRhtRht, ">>");
    m_GlobalTabLayout.ForceNewLine();
    m_GlobalTabLayout.AddYGap();

    m_GlobalTabLayout.SetFitWidthFlag( true );
    m_GlobalTabLayout.SetSameLineFlag( false );

    m_GlobalTabLayout.AddDividerBox("Geometry Control");
    m_GlobalTabLayout.AddYGap();
    m_GlobalTabLayout.AddButton(m_IntersectSubsurfaces, "Intersect Subsurfaces");
    m_GlobalTabLayout.AddYGap();

    m_GlobalTabLayout.SetChoiceButtonWidth(m_GlobalTabLayout.GetRemainX() / 2.0);
    m_GlobalTabLayout.AddChoice(m_UseSet, "Use Set");

    globalTab->show();


    m_OutputTabLayout.SetGroupAndScreen( outputTabGroup, this );

    m_OutputTabLayout.AddDividerBox("Export File Names");
    m_OutputTabLayout.AddYGap();

    m_OutputTabLayout.SetFitWidthFlag( false );
    m_OutputTabLayout.SetSameLineFlag( true );

    m_OutputTabLayout.SetButtonWidth(50);
    m_OutputTabLayout.SetInputWidth(305);


    m_OutputTabLayout.AddButton(m_StlFile, ".stl");
    m_OutputTabLayout.AddOutput(m_StlOutput);
    m_OutputTabLayout.AddButton(m_SelectStlFile, "...");
    m_OutputTabLayout.ForceNewLine();

    m_OutputTabLayout.AddYGap();
    m_OutputTabLayout.SetFitWidthFlag( true );
    m_OutputTabLayout.AddDividerBox( "Mass" );
    m_OutputTabLayout.ForceNewLine();
    m_OutputTabLayout.SetFitWidthFlag( false );

    m_OutputTabLayout.AddButton(m_MassFile, ".mass");
    m_OutputTabLayout.AddOutput(m_MassOutput);
    m_OutputTabLayout.AddButton(m_SelectMassFile, "...");
    m_OutputTabLayout.ForceNewLine();

    m_OutputTabLayout.AddYGap();
    m_OutputTabLayout.SetFitWidthFlag( true );
    m_OutputTabLayout.AddDividerBox( "NASTRAN" );
    m_OutputTabLayout.ForceNewLine();
    m_OutputTabLayout.SetFitWidthFlag( false );

    m_OutputTabLayout.AddButton(m_NastFile, ".nast");
    m_OutputTabLayout.AddOutput(m_NastOutput);
    m_OutputTabLayout.AddButton(m_SelectNastFile, "...");
    m_OutputTabLayout.ForceNewLine();

    m_OutputTabLayout.AddYGap();
    m_OutputTabLayout.SetFitWidthFlag( true );
    m_OutputTabLayout.AddDividerBox( "Calculix" );
    m_OutputTabLayout.ForceNewLine();
    m_OutputTabLayout.SetFitWidthFlag( false );

    m_OutputTabLayout.AddButton(m_GeomFile, ".geom");
    m_OutputTabLayout.AddOutput(m_GeomOutput);
    m_OutputTabLayout.AddButton(m_SelectGeomFile, "...");
    m_OutputTabLayout.ForceNewLine();

    m_OutputTabLayout.AddButton(m_ThickFile, ".thick");
    m_OutputTabLayout.AddOutput(m_ThickOutput);
    m_OutputTabLayout.AddButton(m_SelectThickFile, "...");
    m_OutputTabLayout.ForceNewLine();

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

    m_BorderConsoleLayout.SetButtonWidth( m_BorderConsoleLayout.GetW()/3 );

    m_BorderConsoleLayout.SetFitWidthFlag( false );
    m_BorderConsoleLayout.SetSameLineFlag( true );

    m_BorderConsoleLayout.AddButton( m_ComputeFEAMeshButton, "Compute" );
    m_BorderConsoleLayout.AddButton( m_ExportFEAMeshButton, "Export" );
    m_BorderConsoleLayout.AddButton( m_DrawMeshButton, "Draw Mesh" );


    m_StructureTabLayout.SetGroupAndScreen( structTabGroup, this );

    m_StructureTabLayout.AddDividerBox( "Section" );
    m_StructureTabLayout.AddIndexSelector( m_SectSel );
    m_StructureTabLayout.AddYGap();



    m_StructureTabLayout.AddTabGroup( m_ComponentGroup, m_StructureTabLayout.GetRemainX(), m_StructureTabLayout.GetRemainY() );

    m_ComponentGroup.AddTabLayout( m_RibTabLayout, "Rib", 5 );
    m_RibTabLayout.AddDividerBox( "Rib" );


    m_RibTabLayout.AddIndexSelector( m_RibSel );

    m_RibTabLayout.AddButton( m_AddRib, "Add Rib" );
    m_RibTabLayout.AddButton( m_DelRib, "Delete Rib" );

    int ystart = m_RibTabLayout.GetY();

    m_RibTabLayout.AddSubGroupLayout( m_RibEditLayout, m_RibTabLayout.GetRemainX(), m_RibTabLayout.GetRemainY() );

    m_RibEditLayout.AddSlider( m_RibThickSlider, "Thickness", 1, "%5.3f" );
    m_RibEditLayout.AddSlider( m_RibDensitySlider, "Density", 1, "%5.3f" );

    m_RibEditLayout.AddSlider( m_RibPosSlider, "Position", 1, "%5.3f" );
    m_RibEditLayout.AddSlider( m_RibSweepSlider, "Sweep", 10, "%5.3f" );

    m_RibEditLayout.AddButton( m_RibTrimButton, "Trim at Border" );

    m_RibEditLayout.AddButton( m_RibSweepAbsButton, "Abs" );
    m_RibEditLayout.AddButton( m_RibSweepRelButton, "Rel" );

    m_RibSweepToggle.Init( this );
    m_RibSweepToggle.AddButton( m_RibSweepAbsButton.GetFlButton() );
    m_RibSweepToggle.AddButton( m_RibSweepRelButton.GetFlButton() );


    m_ComponentGroup.AddTabLayout( m_SparTabLayout, "Spar", 5 );
    m_SparTabLayout.AddDividerBox( "Spar" );


    m_ComponentGroup.AddTabLayout( m_UpSkinTabLayout, "Up Skin", 5 );
    m_UpSkinTabLayout.AddDividerBox( "Up Skin" );


    m_ComponentGroup.AddTabLayout( m_LowSkinTabLayout, "Low Skin", 5 );
    m_LowSkinTabLayout.AddDividerBox( "Low Skin" );


    m_ComponentGroup.AddTabLayout( m_PtMassTabLayout, "Pt Mass", 5 );
    m_PtMassTabLayout.AddDividerBox( "Pt Mass" );

    m_ComponentGroup.ShowTab( 0 );

}

StructScreen::~StructScreen()
{
}

void StructScreen::CloseCallBack( Fl_Widget *w )
{
    Hide();
/*
    FeaMeshMgr.SetDrawMeshFlag( false );
    FeaMeshMgr.SetDrawFlag( false );

    FeaMeshMgr.SaveData();

    vector< string > geomVec = m_Vehicle->GetGeomVec();
    for ( int i = 0 ; i < ( int )geomVec.size() ; i++ )
    {
        Geom *g = m_Vehicle->FindGeom( geomVec[i] );
        if ( g )
        {
            g->SetSetFlag( SET_SHOWN, false );
            g->SetSetFlag( SET_NOT_SHOWN, true );
        }
    }

    m_FLTK_Window->hide();
    m_ScreenMgr->SetUpdateFlag( true );
*/
}

void StructScreen::Show()
{
    FeaMeshMgr.SetDrawFlag( true );

    FeaMeshMgr.LoadSurfaces();
    Update();

    m_FLTK_Window->show();

    Vehicle*  veh = m_ScreenMgr->GetVehiclePtr();


    vector< string > geomVec = veh->GetGeomVec();
    for ( int i = 0 ; i < ( int )geomVec.size() ; i++ )
    {
        Geom *g = veh->FindGeom( geomVec[i] );
        if ( g )
        {
            g->SetSetFlag( SET_SHOWN, true );
            g->SetSetFlag( SET_NOT_SHOWN, false );
        }
    }
    m_ScreenMgr->SetUpdateFlag( true );

}

bool StructScreen::Update()
{
    char str[256];

    //==== Default Elem Size ====//
    m_MaxEdgeLen.Update( FeaMeshMgr.GetGridDensityPtr()->m_BaseLen.GetID() );
    m_MinEdgeLen.Update( FeaMeshMgr.GetGridDensityPtr()->m_MinLen.GetID() );
    m_MaxGap.Update( FeaMeshMgr.GetGridDensityPtr()->m_MaxGap.GetID() );
    m_NumCircleSegments.Update( FeaMeshMgr.GetGridDensityPtr()->m_NCircSeg.GetID() );
    m_GrowthRatio.Update( FeaMeshMgr.GetGridDensityPtr()->m_GrowRatio.GetID() );
    m_ThickScale.Update( FeaMeshMgr.m_ThickScale.GetID() );

    //==== SectID ====//
    m_SectSel.SetIndex( FeaMeshMgr.GetCurrSectID() );

    //==== Rib ====//
    m_RibSel.SetIndex( FeaMeshMgr.GetCurrRibID() );


    FeaRib* rib = FeaMeshMgr.GetCurrRib();
    if ( rib )
    {
        m_RibEditLayout.Show();

        m_RibThickSlider.Update( rib->m_Thick.GetID() );
        m_RibDensitySlider.Update( rib->m_Density.GetID() );
        m_RibPosSlider.Update( rib->m_PerSpan.GetID()  );
        m_RibSweepSlider.Update( rib->m_Sweep.GetID()  );

        m_RibTrimButton.Update( rib->m_TrimFlag.GetID() );

        m_RibSweepToggle.Update( rib->m_AbsSweepFlag.GetID() );

    }
    else
    {
        m_RibEditLayout.Hide();
    }



    m_DrawMeshButton.Update( FeaMeshMgr.GetStructSettingsPtr()->m_DrawMeshFlag.GetID() );


    string massname = FeaMeshMgr.GetStructSettingsPtr()->GetExportFileName( vsp::MASS_FILE_NAME );
    m_MassOutput.Update( truncateFileName( massname, 40 ).c_str() );
    string nastranname = FeaMeshMgr.GetStructSettingsPtr()->GetExportFileName( vsp::NASTRAN_FILE_NAME );
    m_NastOutput.Update( truncateFileName( nastranname, 40 ).c_str() );
    string geomname = FeaMeshMgr.GetStructSettingsPtr()->GetExportFileName( vsp::GEOM_FILE_NAME );
    m_GeomOutput.Update( truncateFileName( geomname, 40 ).c_str() );
    string thickname = FeaMeshMgr.GetStructSettingsPtr()->GetExportFileName( vsp::THICK_FILE_NAME );
    m_ThickOutput.Update( truncateFileName( thickname, 40 ).c_str() );
    string stlname = FeaMeshMgr.GetStructSettingsPtr()->GetExportFileName( vsp::STL_FEA_NAME );
    m_StlOutput.Update( truncateFileName( stlname, 40 ).c_str() );

    //==== Update File Output Flags ====//
    m_MassFile.Update( FeaMeshMgr.GetStructSettingsPtr()->GetExportFileFlag( vsp::MASS_FILE_NAME )->GetID() );
    m_NastFile.Update( FeaMeshMgr.GetStructSettingsPtr()->GetExportFileFlag( vsp::NASTRAN_FILE_NAME )->GetID() );
    m_GeomFile.Update( FeaMeshMgr.GetStructSettingsPtr()->GetExportFileFlag( vsp::GEOM_FILE_NAME )->GetID() );
    m_ThickFile.Update( FeaMeshMgr.GetStructSettingsPtr()->GetExportFileFlag( vsp::THICK_FILE_NAME )->GetID() );
    m_StlFile.Update( FeaMeshMgr.GetStructSettingsPtr()->GetExportFileFlag( vsp::STL_FEA_NAME )->GetID() );

    return true;
}

string StructScreen::truncateFileName( const string &fn, int len )
{
    string trunc( fn );
    if ( (int)trunc.length() > len )
    {
        trunc.erase( 0, trunc.length() - len );
        trunc.replace( 0, 3, "..." );
    }
    return trunc;
}

void StructScreen::AddOutputText( const string &text )
{
    m_ConsoleBuffer->append( text.c_str() );
    m_ConsoleDisplay->move_down();
    m_ConsoleDisplay->show_insert_position();
    Fl::flush();
}

void StructScreen::CallBack( Fl_Widget* w )
{


}

void StructScreen::GuiDeviceCallBack( GuiDevice* device )
{
    assert( m_ScreenMgr );

    if ( device == &m_ExportFEAMeshButton )
    {
        FeaMeshMgr.Build();
        FeaMeshMgr.Export();
        FeaMeshMgr.GetStructSettingsPtr()->m_DrawMeshFlag = true;
    }
    else if ( device == &m_ComputeFEAMeshButton )
    {
        FeaMeshMgr.Build();
        FeaMeshMgr.GetStructSettingsPtr()->m_DrawMeshFlag = true;
    }
    else if ( device == &m_SectSel )
    {
        FeaMeshMgr.SetCurrSectID( m_SectSel.GetIndex() );
    }
    else if ( device == &m_RibSel )
    {
        FeaMeshMgr.SetCurrRibID( m_RibSel.GetIndex() );
    }
    else if ( device == &m_AddRib )
    {
        FeaMeshMgr.AddRib();
    }
    else if ( device == &m_DelRib )
    {
        FeaMeshMgr.DelCurrRib();
    }
    else if ( device == &m_SelectStlFile )
    {
        string newfile = m_ScreenMgr->GetSelectFileScreen()->FileChooser( "Select .stl file.", "*.stl" );
        if ( newfile.compare( "" ) != 0 )
        {
            FeaMeshMgr.GetStructSettingsPtr()->SetExportFileName( newfile, vsp::STL_FEA_NAME );
        }
    }
    else if ( device == &m_SelectMassFile )
    {
        string newfile = m_ScreenMgr->GetSelectFileScreen()->FileChooser( "Select mass .dat file.", "*.dat" );
        if ( newfile.compare( "" ) != 0 )
        {
            FeaMeshMgr.GetStructSettingsPtr()->SetExportFileName( newfile, vsp::MASS_FILE_NAME );
        }
    }
    else if ( device == &m_SelectNastFile )
    {
        string newfile = m_ScreenMgr->GetSelectFileScreen()->FileChooser( "Select NASTRAN .dat file.", "*.dat" );
        if ( newfile.compare( "" ) != 0 )
        {
            FeaMeshMgr.GetStructSettingsPtr()->SetExportFileName( newfile, vsp::NASTRAN_FILE_NAME );
        }
    }
    else if ( device == &m_SelectGeomFile  )
    {
        string newfile = m_ScreenMgr->GetSelectFileScreen()->FileChooser( "Select Calculix geom .dat file.", "*.dat" );
        if ( newfile.compare( "" ) != 0 )
        {
            FeaMeshMgr.GetStructSettingsPtr()->SetExportFileName( newfile, vsp::GEOM_FILE_NAME );
        }
    }
    else if ( device == &m_SelectThickFile )
    {
        string newfile = m_ScreenMgr->GetSelectFileScreen()->FileChooser( "Select Calculix thick .dat file.", "*.dat" );
        if ( newfile.compare( "" ) != 0 )
        {
            FeaMeshMgr.GetStructSettingsPtr()->SetExportFileName( newfile, vsp::THICK_FILE_NAME );
        }
    }

    m_ScreenMgr->SetUpdateFlag( true );
}

void StructScreen::LoadDrawObjs( vector< DrawObj* > &draw_obj_vec )
{
    if ( IsShown() )
    {
        FeaMeshMgr.LoadDrawObjs( draw_obj_vec );
    }
}
