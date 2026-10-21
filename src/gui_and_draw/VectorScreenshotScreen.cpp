//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//
//
//////////////////////////////////////////////////////////////////////

#include "VectorScreenshotScreen.h"
#include "ScreenMgr.h"
#include "MainVSPScreen.h"
#include "MainGLWindow.h"
#include "UnitConversion.h"

VectorScreenshotScreen::VectorScreenshotScreen( ScreenMgr* mgr ) : BasicScreen( mgr, 300, 320, "Vector Screenshot" )
{
    m_FLTK_Window->callback( staticCloseCB, this );
    m_MainLayout.SetGroupAndScreen( m_FLTK_Window, this );

    m_MainLayout.ForceNewLine();
    m_MainLayout.AddY( 7 );
    m_MainLayout.AddX( 5 );

    m_MainLayout.AddSubGroupLayout( m_BorderLayout, m_MainLayout.GetRemainX() - 5.0,
                                    m_MainLayout.GetRemainY() - 5.0 );

    m_BorderLayout.AddDividerBox( "Window Size" );
    m_BorderLayout.AddOutput( m_WindowWidth, "Width (px)" );
    m_BorderLayout.AddOutput( m_WindowHeight, "Height (px)" );
    m_BorderLayout.AddYGap();

    m_BorderLayout.AddDividerBox( "File" );
    m_FileTypeChoice.AddItem( "SVG", vsp::VEC_SVG );
    m_FileTypeChoice.AddItem( "PDF", vsp::VEC_PDF );
    m_FileTypeChoice.UpdateItems();
    m_BorderLayout.AddChoice( m_FileTypeChoice, "Format" );
    m_BorderLayout.AddButton( m_AutoCropToggle, "Auto Crop" );
    m_BorderLayout.AddYGap();

    m_BorderLayout.AddDividerBox( "Page" );
    m_BorderLayout.AddButton( m_ScaleToPageToggle, "Scale to Page" );
    m_BorderLayout.AddSlider( m_PageWidthSlider, "Width", 10.0, "%7.2f" );
    m_BorderLayout.AddSlider( m_PageHeightSlider, "Height", 10.0, "%7.2f" );
    m_PageUnitsChoice.AddItem( "mm", vsp::LEN_MM );
    m_PageUnitsChoice.AddItem( "cm", vsp::LEN_CM );
    m_PageUnitsChoice.AddItem( "m", vsp::LEN_M );
    m_PageUnitsChoice.AddItem( "in", vsp::LEN_IN );
    m_PageUnitsChoice.UpdateItems();
    m_BorderLayout.AddChoice( m_PageUnitsChoice, "Units" );
    m_BorderLayout.AddOutput( m_DrawingSize, "Drawing" );
    m_BorderLayout.AddYGap();

    m_BorderLayout.AddButton( m_CaptureButton, "Capture" );
}

VectorScreenshotScreen::~VectorScreenshotScreen()
{
}

void VectorScreenshotScreen::Show()
{
    // Filled in before it appears, not at the next update of the screens
    Update();
    BasicScreen::Show();
}

bool VectorScreenshotScreen::Update()
{
    BasicScreen::Update();

    MainVSPScreen* main = dynamic_cast<MainVSPScreen*>( m_ScreenMgr->GetScreen( vsp::VSP_MAIN_SCREEN ) );
    Vehicle * veh = m_ScreenMgr->GetVehiclePtr();
    if ( !main || !veh )
    {
        return false;
    }

    VSPGUI::VspGlWindow * glwin = main->GetGLWindow();
    int w = glwin->pixel_w();
    int h = glwin->pixel_h();

    m_WindowWidth.Update( std::to_string( w ) );
    m_WindowHeight.Update( std::to_string( h ) );

    m_FileTypeChoice.Update( veh->m_VecFileType.GetID() );
    m_AutoCropToggle.Update( veh->m_VecAutoCropFlag.GetID() );
    m_ScaleToPageToggle.Update( veh->m_VecScaleToPageFlag.GetID() );
    m_PageWidthSlider.Update( veh->m_VecPageWidth.GetID() );
    m_PageHeightSlider.Update( veh->m_VecPageHeight.GetID() );
    m_PageUnitsChoice.Update( veh->m_VecPageUnits.GetID() );

    // The size the whole window is drawn at: a pixel is a point, or the window fits the page.
    int units = veh->m_VecPageUnits();
    double scale = MainVSPScreen::VectorPageScale( w, h, veh->m_VecScaleToPageFlag(), veh->m_VecPageWidth(),
                                                   veh->m_VecPageHeight(), units );
    double dw = ConvertLength( w * scale / 72.0, vsp::LEN_IN, units );
    double dh = ConvertLength( h * scale / 72.0, vsp::LEN_IN, units );
    char str[256];
    snprintf( str, sizeof( str ), "%.2f x %.2f", dw, dh );
    m_DrawingSize.Update( str );

    if ( veh->m_VecScaleToPageFlag() )
    {
        m_PageWidthSlider.Activate();
        m_PageHeightSlider.Activate();
    }
    else
    {
        m_PageWidthSlider.Deactivate();
        m_PageHeightSlider.Deactivate();
    }

    m_FLTK_Window->redraw();
    return false;
}

void VectorScreenshotScreen::CallBack( Fl_Widget* w )
{
    assert( m_ScreenMgr );

    m_ScreenMgr->SetUpdateFlag( true );
}

void VectorScreenshotScreen::CloseCallBack( Fl_Widget *w )
{
    Hide();
}

void VectorScreenshotScreen::GuiDeviceCallBack( GuiDevice* device )
{
    assert( m_ScreenMgr );

    MainVSPScreen* main = dynamic_cast<MainVSPScreen*>( m_ScreenMgr->GetScreen( vsp::VSP_MAIN_SCREEN ) );
    Vehicle * veh = m_ScreenMgr->GetVehiclePtr();

    if ( device == &m_PageUnitsChoice && veh )
    {
        // A change of units keeps the size of the page.
        int from = ( int ) veh->m_VecPageUnits.GetLastVal();
        int to = veh->m_VecPageUnits();
        veh->m_VecPageWidth.Set( ConvertLength( veh->m_VecPageWidth(), from, to ) );
        veh->m_VecPageHeight.Set( ConvertLength( veh->m_VecPageHeight(), from, to ) );
    }
    else if ( device == &m_CaptureButton && main && veh )
    {
        int type = veh->m_VecFileType();
        string filter = "*.svg";
        if ( type == vsp::VEC_PDF )
        {
            filter = "*.pdf";
        }

        string fname = m_ScreenMgr->FileChooser( "Create or Select a Vector File", filter, vsp::SAVE );
        if ( !fname.empty() )
        {
            if ( !main->VectorScreenGrab( fname, type, veh->m_VecAutoCropFlag(), veh->m_VecScaleToPageFlag(),
                                          veh->m_VecPageWidth(), veh->m_VecPageHeight(), veh->m_VecPageUnits() ) )
            {
                m_ScreenMgr->Alert( "Could not write the vector file " + fname );
            }
        }
    }

    m_ScreenMgr->SetUpdateFlag( true );
}
