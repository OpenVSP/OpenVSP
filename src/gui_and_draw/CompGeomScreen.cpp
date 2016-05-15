//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//
//
//////////////////////////////////////////////////////////////////////

#include "CompGeomScreen.h"
#include "CfdMeshMgr.h"
#include "Vehicle.h"

CompGeomScreen::CompGeomScreen( ScreenMgr* mgr ) : BasicScreen( mgr, 375, 430, "Comp Geom - Mesh, Intersect, Trim" )
{
    m_FLTK_Window->callback( staticCloseCB, this );
    m_MainLayout.SetGroupAndScreen( m_FLTK_Window, this );

    m_MainLayout.ForceNewLine();
    m_MainLayout.AddY(7);
    m_MainLayout.AddX(5);

    m_MainLayout.AddSubGroupLayout( m_BorderLayout, m_MainLayout.GetRemainX() - 5.0,
                                       m_MainLayout.GetRemainY() - 5.0);

    m_BorderLayout.AddDividerBox("File Export");
    m_BorderLayout.AddYGap();

    m_BorderLayout.SetFitWidthFlag( false );
    m_BorderLayout.SetSameLineFlag( true );

    m_BorderLayout.SetButtonWidth(50);
    m_BorderLayout.SetInputWidth(265);
    m_BorderLayout.AddOutput(m_TxtOutput, ".txt");
    m_BorderLayout.AddButton(m_TxtSelect, "...");
    m_BorderLayout.ForceNewLine();
    m_BorderLayout.AddYGap();

    m_BorderLayout.AddButton(m_CsvToggle, ".csv");
    m_BorderLayout.AddOutput(m_CsvOutput);
    m_BorderLayout.AddButton(m_CsvSelect, "...");
    m_BorderLayout.ForceNewLine();
    m_BorderLayout.AddYGap();

    m_BorderLayout.SetFitWidthFlag( true );
    m_BorderLayout.SetSameLineFlag( false );

    m_BorderLayout.AddDividerBox("Parasite Drag Output");
    m_BorderLayout.AddYGap();

    m_BorderLayout.SetFitWidthFlag( false );
    m_BorderLayout.SetSameLineFlag( true );

    m_BorderLayout.AddButton(m_TsvToggle, ".tsv");
    m_BorderLayout.AddOutput(m_TsvOutput);
    m_BorderLayout.AddButton(m_TsvSelect, "...");
    m_BorderLayout.ForceNewLine();
    m_BorderLayout.AddYGap();

    m_BorderLayout.SetFitWidthFlag( true );
    m_BorderLayout.SetSameLineFlag( false );

    m_TextDisplay = m_BorderLayout.AddFlTextDisplay( 225 );
    m_TextDisplay->color( 23 );
    m_TextDisplay->textcolor( 32 );
    m_TextDisplay->textfont( 4 );
    m_TextDisplay->textsize( 12 );
    m_TextBuffer = new Fl_Text_Buffer;
    m_TextDisplay->buffer( m_TextBuffer );
    m_BorderLayout.AddYGap();

    m_BorderLayout.SetFitWidthFlag( false );
    m_BorderLayout.SetSameLineFlag( true );

    m_BorderLayout.SetChoiceButtonWidth(50);
    m_BorderLayout.SetButtonWidth(95.0);
    m_BorderLayout.SetFitWidthFlag( true );
    m_BorderLayout.AddChoice(m_UseSet, "Set:", 200);
    m_BorderLayout.SetFitWidthFlag( false );
    m_BorderLayout.AddX(5);
    m_BorderLayout.AddButton(m_HalfMesh, "Half Mesh");
    m_BorderLayout.AddX(5);
    m_BorderLayout.AddButton(m_Subsurfs, "Subsurfs");
    m_BorderLayout.ForceNewLine();
    m_BorderLayout.AddYGap();

    m_BorderLayout.SetFitWidthFlag( true );
    m_BorderLayout.SetSameLineFlag( false );

    m_BorderLayout.AddButton(m_Execute, "Execute");

    m_SelectedSetIndex = 0;
    m_Subsurfs.GetFlButton()->value( 1 );
}

CompGeomScreen::~CompGeomScreen()
{
}

void CompGeomScreen::LoadSetChoice()
{
    m_UseSet.ClearItems();

    Vehicle* veh = m_ScreenMgr->GetVehiclePtr();
    vector< string > set_name_vec = veh->GetSetNameVec();

    for ( int i = 0 ; i < ( int )set_name_vec.size() ; ++i )
    {
        m_UseSet.AddItem( set_name_vec[i].c_str() );
    }

    m_UseSet.UpdateItems();
    m_UseSet.SetVal( m_SelectedSetIndex );
}

void CompGeomScreen::Show()
{
    m_ScreenMgr->SetUpdateFlag( true );
    m_FLTK_Window->show();
}

void CompGeomScreen::Hide()
{
    m_FLTK_Window->hide();
    m_ScreenMgr->SetUpdateFlag( true );
}

bool CompGeomScreen::Update()
{
    Vehicle* vehiclePtr = m_ScreenMgr->GetVehiclePtr();

    LoadSetChoice();

    //===== Update File Toggle Buttons =====//
    m_CsvToggle.Update( vehiclePtr->m_exportCompGeomCsvFile.GetID() );
    m_TsvToggle.Update( vehiclePtr->m_exportDragBuildTsvFile.GetID() );

    //===== Update File Output Text =====//
    m_TxtOutput.Update( vehiclePtr->getExportFileName( vsp::COMP_GEOM_TXT_TYPE ).c_str() );
    m_CsvOutput.Update( vehiclePtr->getExportFileName( vsp::COMP_GEOM_CSV_TYPE ).c_str() );
    m_TsvOutput.Update( vehiclePtr->getExportFileName( vsp::DRAG_BUILD_TSV_TYPE ).c_str() );

    m_FLTK_Window->redraw();
    return false;
}

// Callback for Link Browser
void CompGeomScreen::CallBack( Fl_Widget* w )
{
    assert( m_ScreenMgr );

    m_ScreenMgr->SetUpdateFlag( true );

}

void CompGeomScreen::CloseCallBack( Fl_Widget *w )
{
    Hide();
}

// Callback for all other GUI Devices
void CompGeomScreen::GuiDeviceCallBack( GuiDevice* device )
{
    Vehicle* vehiclePtr = m_ScreenMgr->GetVehiclePtr();

    assert( m_ScreenMgr );

    if ( device == &m_TxtSelect )
    {
        vehiclePtr->setExportFileName( vsp::COMP_GEOM_TXT_TYPE,
                                       m_ScreenMgr->GetSelectFileScreen()->FileChooser(
                                               "Select comp_geom output file.", "*.txt" ) );
    }
    else if ( device == &m_CsvSelect )
    {
        vehiclePtr->setExportFileName( vsp::COMP_GEOM_CSV_TYPE,
                                       m_ScreenMgr->GetSelectFileScreen()->FileChooser(
                                               "Select comp_geom output file.", "*.csv" ) );
    }
    else if ( device == &m_TsvSelect )
    {
        vehiclePtr->setExportFileName( vsp::DRAG_BUILD_TSV_TYPE,
                                       m_ScreenMgr->GetSelectFileScreen()->FileChooser(
                                               "Select comp_geom output file.", "*.tsv" ) );
    }
    else if ( device == &m_UseSet )
    {
        m_SelectedSetIndex = m_UseSet.GetVal();
    }
    else if ( device == &m_Execute )
    {
        string geom = vehiclePtr->CompGeomAndFlatten( m_SelectedSetIndex, m_HalfMesh.GetFlButton()->value(),
                      m_Subsurfs.GetFlButton()->value() );
        if ( geom.compare( "NONE" ) != 0 )
        {
            m_TextDisplay->buffer()->loadfile( vehiclePtr->getExportFileName( vsp::COMP_GEOM_TXT_TYPE ).c_str() );
        }
    }

    m_ScreenMgr->SetUpdateFlag( true );
}
