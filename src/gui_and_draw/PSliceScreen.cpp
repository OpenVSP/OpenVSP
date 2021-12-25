//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//
//////////////////////////////////////////////////////////////////////

#include "PSliceScreen.h"
#include "ScreenMgr.h"

PSliceScreen::PSliceScreen( ScreenMgr *mgr ) : BasicScreen( mgr, 300, 450, "Planar Slicing" )
{
    m_FLTK_Window->callback( staticCloseCB, this );
    m_MainLayout.SetGroupAndScreen( m_FLTK_Window, this );

    int borderPaddingWidth = 5;
    int yPadding = 7;
    int smallButtonWidth = 100;
    int smallGap = 25;

    m_Norm = vec3d( 1, 0, 0 );

    m_SelectedSetIndex = DEFAULT_SET;

    m_MainLayout.ForceNewLine();
    m_MainLayout.AddY( yPadding );
    m_MainLayout.AddX( borderPaddingWidth );

    m_MainLayout.AddSubGroupLayout( m_BorderLayout, m_MainLayout.GetRemainX() - borderPaddingWidth,
                                        m_MainLayout.GetRemainY() - borderPaddingWidth );

    m_BorderLayout.SetButtonWidth( smallButtonWidth );
    m_BorderLayout.AddSlider( m_NumSlicesInput, "Num Slice:", 100, "%6.0f" );
    m_BorderLayout.AddYGap();

    m_AxisChoice.AddItem( "X-Axis" );
    m_AxisChoice.AddItem( "Y-Axis" );
    m_AxisChoice.AddItem( "Z-Axis" );

    m_BorderLayout.AddChoice( m_AxisChoice, "Normal Axis" );
    m_BorderLayout.AddYGap();

    m_BorderLayout.AddDividerBox( "Slicing Bounds" );

    m_BorderLayout.AddButton( m_AutoButton, "Auto" );
    m_BorderLayout.AddYGap();

    m_BorderLayout.AddSlider( m_StartLocSlider, "Start Location", 10, "%6.3f" );

    m_BorderLayout.AddSlider( m_EndLocSlider, "End Location", 10, "%6.3f" );
    m_BorderLayout.AddYGap();

    m_BorderLayout.AddDividerBox( "Output File" );

    m_BorderLayout.SetSameLineFlag( true );
    m_BorderLayout.SetFitWidthFlag( true );

    m_BorderLayout.SetButtonWidth( 0 );
    m_BorderLayout.AddOutput( m_FileSelect, "", smallGap );

    m_BorderLayout.SetFitWidthFlag( false );
    m_BorderLayout.SetButtonWidth( smallGap );
    m_BorderLayout.AddButton( m_FileTrigger, "..." );
    m_BorderLayout.AddYGap();

    m_BorderLayout.ForceNewLine();
    m_BorderLayout.AddYGap();

    m_BorderLayout.SetFitWidthFlag( true );
    m_BorderLayout.SetSameLineFlag( false );

    m_TextDisplay = m_BorderLayout.AddFlTextDisplay( 180 );
    m_TextDisplay->color( 23 );
    m_TextDisplay->textcolor( 32 );
    m_TextDisplay->textfont( 4 );
    m_TextDisplay->textsize( 12 );
    m_TextBuffer = new Fl_Text_Buffer;
    m_TextDisplay->buffer( m_TextBuffer );
    m_BorderLayout.AddYGap();

    m_BorderLayout.AddChoice( m_SetChoice, "Set" );
    m_BorderLayout.AddYGap();

    m_BorderLayout.AddButton( m_StartSlicingTrigger, "Start Slicing" );
    m_BorderLayout.AddYGap();
}

PSliceScreen::~PSliceScreen()
{
    m_TextDisplay->buffer( NULL );
    delete m_TextBuffer;
}

bool PSliceScreen::Update()
{
    assert( m_ScreenMgr );
    Vehicle* veh = m_ScreenMgr->GetVehiclePtr();

    LoadSetChoice();

    m_Norm.set_xyz( 0, 0, 0 );
    m_Norm[veh->m_PlanarAxisType.Get()] = 1;

    if ( veh->m_AutoBoundsFlag() )
    {
        vector< Geom* > geom_vec = veh->FindGeomVec( veh->GetGeomVec() );
        BndBox bbox;

        for ( int i = 0 ; i < ( int )geom_vec.size() ; i++ )
        {
            if ( geom_vec[i] )
            {
                if ( geom_vec[i]->GetSetFlag( m_SelectedSetIndex ) )
                {
                    bbox.Update( geom_vec[i]->GetBndBox() );
                }
            }
        }

        // Set Start and End Locations
        vec3d minBBox = bbox.GetMin();
        vec3d maxBBox = bbox.GetMax();
        double min = minBBox[veh->m_PlanarAxisType.Get()];
        double max = maxBBox[veh->m_PlanarAxisType.Get()];

        veh->m_PlanarStartLocation = min ;
        veh->m_PlanarEndLocation = max;
    }

    m_NumSlicesInput.Update( veh->m_NumPlanerSlices.GetID() );

    m_AutoButton.Update( veh->m_AutoBoundsFlag.GetID() );

    m_FileSelect.Update( veh->getExportFileName( vsp::SLICE_TXT_TYPE ).c_str() );

    m_AxisChoice.Update( veh->m_PlanarAxisType.GetID() );

    m_StartLocSlider.Update( veh->m_PlanarStartLocation.GetID() );
    m_EndLocSlider.Update( veh->m_PlanarEndLocation.GetID() );

    if ( veh->m_AutoBoundsFlag.Get() == false )
    {
        m_StartLocSlider.Activate();
        m_EndLocSlider.Activate();   
    }
    else
    {
        m_StartLocSlider.Deactivate();
        m_EndLocSlider.Deactivate();
    }

    if ( veh->m_NumPlanerSlices() == 1 )
    {
        m_EndLocSlider.Deactivate();
    }

    m_FLTK_Window->redraw();
    return false;
}

void PSliceScreen::Show()
{
    m_ScreenMgr->SetUpdateFlag( true );
    m_FLTK_Window->show();
}

void PSliceScreen::Hide()
{
    m_FLTK_Window->hide();
    m_ScreenMgr->SetUpdateFlag( true );
}

void PSliceScreen::LoadSetChoice()
{
    m_SetChoice.ClearItems();

    Vehicle* veh = m_ScreenMgr->GetVehiclePtr();
    vector< string > set_name_vec = veh->GetSetNameVec();

    for ( int i = 0 ; i < ( int )set_name_vec.size() ; i++ )
    {
        m_SetChoice.AddItem( set_name_vec[i].c_str() );
    }

    m_SetChoice.UpdateItems();
    m_SetChoice.SetVal( m_SelectedSetIndex );
}

void PSliceScreen::GuiDeviceCallBack( GuiDevice* device )
{
    assert( m_ScreenMgr );
    Vehicle* veh = m_ScreenMgr->GetVehiclePtr();

    if ( device == &m_FileTrigger )
    {
         veh->setExportFileName( vsp::SLICE_TXT_TYPE,
                                       m_ScreenMgr->GetSelectFileScreen()->FileChooser(
                                               "Select Mass Prop output file.", "*.txt" ) );
    }
    else if ( device == &m_StartSlicingTrigger )
    {
        string id = veh->PSliceAndFlatten( m_SelectedSetIndex, veh->m_NumPlanerSlices.Get(), m_Norm,
                                           !!veh->m_AutoBoundsFlag.Get(), veh->m_PlanarStartLocation.Get(), veh->m_PlanarEndLocation.Get() );
        if ( id.compare( "NONE" ) != 0 )
        {
            m_TextBuffer->loadfile( veh->getExportFileName( vsp::SLICE_TXT_TYPE ).c_str() );
        }
    }
    else if ( device == &m_SetChoice )
    {
        m_SelectedSetIndex = m_SetChoice.GetVal();
    }

    m_ScreenMgr->SetUpdateFlag( true );
}

void PSliceScreen::CallBack( Fl_Widget* w )
{
    assert( m_ScreenMgr );
    m_ScreenMgr->SetUpdateFlag( true );
}

void PSliceScreen::CloseCallBack( Fl_Widget* w )
{
    Hide();
}