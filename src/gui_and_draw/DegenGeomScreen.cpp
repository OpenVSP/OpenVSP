//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//
//
//////////////////////////////////////////////////////////////////////

#include "DegenGeomScreen.h"
#include "CfdMeshMgr.h"
#include "StringUtil.h"
#include "MeshGeom.h"

DegenGeomScreen::DegenGeomScreen( ScreenMgr* mgr ) : BasicScreen( mgr, 375, 365, "Degen Geom - Compute Models, File IO" )
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

    m_BorderLayout.AddButton(m_CsvToggle, ".csv");
    m_BorderLayout.AddOutput(m_CsvOutput);
    m_BorderLayout.AddButton(m_CsvSelect, "...");
    m_BorderLayout.ForceNewLine();
    m_BorderLayout.AddYGap();

    m_BorderLayout.AddButton(m_MToggle, ".m");
    m_BorderLayout.AddOutput(m_MOutput);
    m_BorderLayout.AddButton(m_MSelect, "...");
    m_BorderLayout.ForceNewLine();
    m_BorderLayout.AddYGap();

    m_BorderLayout.SetFitWidthFlag( true );
    m_BorderLayout.SetSameLineFlag( false );

    m_TextDisplay = m_BorderLayout.AddFlTextDisplay( 235 );
    m_TextDisplay->color( 23 );
    m_TextDisplay->textcolor( 32 );
    m_TextDisplay->textfont( 4 );
    m_TextDisplay->textsize( 12 );
    m_TextBuffer = new Fl_Text_Buffer;
    m_TextDisplay->buffer( m_TextBuffer );
    m_BorderLayout.AddYGap();

    m_BorderLayout.SetFitWidthFlag( true );
    m_BorderLayout.SetSameLineFlag( true );

    m_BorderLayout.SetButtonWidth(m_BorderLayout.GetRemainX() / 3.0 - 5.0);
    m_BorderLayout.SetChoiceButtonWidth(50);
    m_BorderLayout.AddChoice(m_UseSet, "Set:", 2.0 * m_BorderLayout.GetRemainX() / 3.0);
    m_BorderLayout.AddX(5);
    m_BorderLayout.SetFitWidthFlag( false );
    m_BorderLayout.AddButton(m_Execute, "Execute");
    m_BorderLayout.AddButton(m_MakeDegenMeshGeom, "Make MeshGeom");

    m_SelectedSetIndex = DEFAULT_SET;
}

DegenGeomScreen::~DegenGeomScreen()
{
    m_TextDisplay->buffer( NULL );
    delete m_TextBuffer;
}

void DegenGeomScreen::LoadSetChoice()
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

void DegenGeomScreen::Show()
{
    m_ScreenMgr->SetUpdateFlag( true );
    m_FLTK_Window->show();
}

void DegenGeomScreen::Hide()
{
    m_FLTK_Window->hide();
    m_ScreenMgr->SetUpdateFlag( true );
}

bool DegenGeomScreen::Update()
{
    Vehicle* vehiclePtr = m_ScreenMgr->GetVehiclePtr();

    LoadSetChoice();

    //===== Update File Toggle Buttons =====//
    m_CsvToggle.Update( vehiclePtr->m_exportDegenGeomCsvFile.GetID() );
    m_MToggle.Update( vehiclePtr->m_exportDegenGeomMFile.GetID() );

    //===== Update File Output Text =====//
    string csvName = vehiclePtr->getExportFileName( vsp::DEGEN_GEOM_CSV_TYPE );
    string mName = vehiclePtr->getExportFileName( vsp::DEGEN_GEOM_M_TYPE );
    m_CsvOutput.Update( StringUtil::truncateFileName( csvName, 40 ).c_str() );
    m_MOutput.Update( StringUtil::truncateFileName( mName, 40 ).c_str() );

    m_FLTK_Window->redraw();
    return false;
}

// Callback for Link Browser
void DegenGeomScreen::CallBack( Fl_Widget* w )
{
    assert( m_ScreenMgr );

    m_ScreenMgr->SetUpdateFlag( true );
}

void DegenGeomScreen::CloseCallBack( Fl_Widget *w )
{
    Hide();
}

// Callback for all other GUI Devices
void DegenGeomScreen::GuiDeviceCallBack( GuiDevice* device )
{
    Vehicle* vehiclePtr = m_ScreenMgr->GetVehiclePtr();

    assert( m_ScreenMgr );

    if ( device == &m_CsvSelect )
    {
        vehiclePtr->setExportFileName( vsp::DEGEN_GEOM_CSV_TYPE,
                                       m_ScreenMgr->GetSelectFileScreen()->FileChooser(
                                               "Select degen geom CSV output file.", "*.csv" ) );
    }
    else if ( device == &m_MSelect )
    {
        vehiclePtr->setExportFileName( vsp::DEGEN_GEOM_M_TYPE,
                                       m_ScreenMgr->GetSelectFileScreen()->FileChooser(
                                               "Select degen geom Matlab output file.", "*.m" ) );
    }
    else if ( device == &m_UseSet )
    {
        m_SelectedSetIndex = m_UseSet.GetVal();
    }
    else if ( device == &m_Execute )
    {
        m_TextDisplay->buffer()->text("");
        m_TextDisplay->buffer()->append("Computing degenerate geometry...\n");
        Fl::flush();

        vehiclePtr->CreateDegenGeom( m_SelectedSetIndex );
        m_TextDisplay->buffer()->append("Done!\n");

        if ( vehiclePtr->getExportDegenGeomCsvFile() || vehiclePtr->getExportDegenGeomMFile() )
        {
            m_TextDisplay->buffer()->append("--------------------------------\n");
            m_TextDisplay->buffer()->append("\nWriting output...\n");
            Fl::flush();

            m_TextDisplay->buffer()->append( vehiclePtr->WriteDegenGeomFile().c_str() );
        }
    }
    else if ( device == & m_MakeDegenMeshGeom )
    {
        string id = vehiclePtr->AddMeshGeom( vsp::SET_NONE, m_SelectedSetIndex );
        if ( id.compare( "NONE" ) != 0 )
        {
            Geom* geom_ptr = vehiclePtr->FindGeom( id );
            if ( geom_ptr )
            {
                MeshGeom* mg = dynamic_cast<MeshGeom*>( geom_ptr );
                mg->SubTagTris( true );
                geom_ptr->Update();
            }
            vehiclePtr->HideAllExcept( id );
        }
    }

    m_ScreenMgr->SetUpdateFlag( true );
}
