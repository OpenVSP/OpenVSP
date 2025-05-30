//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//
//
//////////////////////////////////////////////////////////////////////

#include "CompGeomScreen.h"

#include "ModeMgr.h"

CompGeomScreen::CompGeomScreen( ScreenMgr* mgr ) : BasicScreen( mgr, 375, 470, "Comp Geom - Mesh, Intersect, Trim" )
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

    int bw = 100;
    m_BorderLayout.SetButtonWidth( bw );

    m_BorderLayout.SetSameLineFlag( true );
    m_BorderLayout.SetChoiceButtonWidth( 0 );
    m_BorderLayout.SetFitWidthFlag( false );
    m_BorderLayout.AddButton( m_SetToggle, "Normal Set:" );
    m_BorderLayout.SetFitWidthFlag( true );
    m_BorderLayout.AddChoice(m_UseSet, "", bw);
    m_BorderLayout.ForceNewLine();

    m_BorderLayout.SetSameLineFlag( false );
    m_BorderLayout.SetChoiceButtonWidth( bw );
    m_BorderLayout.AddChoice(m_DegenSet, "Degen Set:" );

    m_BorderLayout.SetSameLineFlag( true );
    m_BorderLayout.SetChoiceButtonWidth( 0 );
    m_BorderLayout.SetFitWidthFlag( false );
    m_BorderLayout.AddButton( m_ModeToggle, "Mode:" );
    m_BorderLayout.SetFitWidthFlag( true );
    m_BorderLayout.AddChoice(m_ModeChoice, "", bw );
    m_BorderLayout.ForceNewLine();

    m_ModeSetToggleGroup.Init( this );
    m_ModeSetToggleGroup.AddButton( m_SetToggle.GetFlButton() );
    m_ModeSetToggleGroup.AddButton( m_ModeToggle.GetFlButton() );

    m_BorderLayout.AddYGap();
    m_BorderLayout.SetButtonWidth(( m_BorderLayout.GetRemainX() - 5 ) / 2.0 );

    m_BorderLayout.SetFitWidthFlag( false );
    m_BorderLayout.SetSameLineFlag( true );
    m_BorderLayout.AddButton(m_HalfMesh, "Half Mesh");
    m_BorderLayout.AddX(5);
    m_BorderLayout.AddButton(m_Subsurfs, "Subsurfs");
    m_BorderLayout.ForceNewLine();
    m_BorderLayout.AddYGap();

    m_BorderLayout.SetFitWidthFlag( true );
    m_BorderLayout.SetSameLineFlag( false );

    m_BorderLayout.AddButton(m_Execute, "Execute");

    m_SelectedSetIndex = DEFAULT_SET;
    m_DegenSelectedSetIndex = vsp::SET_NONE;
    m_SelectedModeChoice = 0;

    m_Subsurfs.GetFlButton()->value( 1 );
}

CompGeomScreen::~CompGeomScreen()
{
    m_TextDisplay->buffer( nullptr );
    delete m_TextBuffer;
}

void CompGeomScreen::Show()
{
    m_ScreenMgr->SetUpdateFlag( true );
    BasicScreen::Show();
}

void CompGeomScreen::Hide()
{
    m_FLTK_Window->hide();
    m_ScreenMgr->SetUpdateFlag( true );
}

bool CompGeomScreen::Update()
{
    BasicScreen::Update();

    Vehicle* vehiclePtr = m_ScreenMgr->GetVehiclePtr();

    m_ScreenMgr->LoadSetChoice( {&m_UseSet, &m_DegenSet}, {m_SelectedSetIndex, m_DegenSelectedSetIndex}, true );

    m_ScreenMgr->LoadModeChoice( m_ModeChoice, m_ModeIDs, m_SelectedModeChoice );

    //===== Update File Toggle Buttons =====//
    m_CsvToggle.Update( vehiclePtr->m_exportCompGeomCsvFile.GetID() );

    //===== Update File Output Text =====//
    m_TxtOutput.Update( vehiclePtr->getExportFileName( vsp::COMP_GEOM_TXT_TYPE ).c_str() );
    m_CsvOutput.Update( vehiclePtr->getExportFileName( vsp::COMP_GEOM_CSV_TYPE ).c_str() );

    m_ModeSetToggleGroup.Update( vehiclePtr->m_UseModeCompGeomFlag.GetID() );

    if ( ModeMgr.GetNumModes() == 0 )
    {
        if ( vehiclePtr->m_UseModeCompGeomFlag() )
        {
            vehiclePtr->m_UseModeCompGeomFlag.Set( false );
            m_ScreenMgr->SetUpdateFlag( true );
        }
        m_ModeToggle.Deactivate();
    }
    else
    {
        m_ModeToggle.Activate();
    }

    if ( vehiclePtr->m_UseModeCompGeomFlag() )
    {
        m_ModeChoice.Activate();
        m_UseSet.Deactivate();
        m_DegenSet.Deactivate();

        Mode *m = ModeMgr.GetMode( m_ModeIDs[m_SelectedModeChoice] );
        if ( m )
        {
            if ( m_SelectedSetIndex != m->m_NormalSet() ||
                 m_DegenSelectedSetIndex != m->m_DegenSet() )
            {
                m_SelectedSetIndex = m->m_NormalSet();
                m_DegenSelectedSetIndex = m->m_DegenSet();
                m_ScreenMgr->SetUpdateFlag( true );
            }
        }
    }
    else
    {
        m_ModeChoice.Deactivate();
        m_UseSet.Activate();
        m_DegenSet.Activate();
    }

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
    assert( m_ScreenMgr );
    Vehicle* vehiclePtr = m_ScreenMgr->GetVehiclePtr();

    if ( device == &m_TxtSelect )
    {
        vehiclePtr->setExportFileName( vsp::COMP_GEOM_TXT_TYPE, m_ScreenMgr->FileChooser( "Select comp_geom output file.", "*.txt", vsp::SAVE ) );
    }
    else if ( device == &m_CsvSelect )
    {
        vehiclePtr->setExportFileName( vsp::COMP_GEOM_CSV_TYPE, m_ScreenMgr->FileChooser( "Select comp_geom output file.", "*.csv", vsp::SAVE ) );
    }
    else if ( device == &m_ModeChoice )
    {
        m_SelectedModeChoice = m_ModeChoice.GetVal();
    }
    else if ( device == &m_UseSet )
    {
        m_SelectedSetIndex = m_UseSet.GetVal();
    }
    else if ( device == &m_DegenSet )
    {
        m_DegenSelectedSetIndex = m_DegenSet.GetVal();
    }
    else if ( device == &m_Execute )
    {
        bool useMode = vehiclePtr->m_UseModeCompGeomFlag();
        string modeID;
        if ( m_SelectedModeChoice >= 0 && m_SelectedModeChoice < m_ModeIDs.size() )
        {
            modeID = m_ModeIDs[ m_SelectedModeChoice ];
        }

        bool hideset = true;
        bool suppressdisks = false;

        string geom = vehiclePtr->CompGeomAndFlatten( m_SelectedSetIndex, m_HalfMesh.GetFlButton()->value(),
                      m_Subsurfs.GetFlButton()->value(), m_DegenSelectedSetIndex, hideset, suppressdisks, useMode, modeID );
        if ( geom.compare( "NONE" ) != 0 )
        {
            m_TextDisplay->buffer()->loadfile( vehiclePtr->getExportFileName( vsp::COMP_GEOM_TXT_TYPE ).c_str() );
        }
    }

    m_ScreenMgr->SetUpdateFlag( true );
}
