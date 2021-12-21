//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//
//
//////////////////////////////////////////////////////////////////////

#include "ScreenshotScreen.h"
#include "ScreenMgr.h"

#include "MainGLWindow.h"

#include "GraphicEngine.h"
#include "Viewport.h"

#include "FL/Fl_File_Chooser.H"

ScreenshotScreen::ScreenshotScreen( ScreenMgr* mgr ) : BasicScreen( mgr, 270, 233+48, "Screenshot" )
{
    m_FLTK_Window->callback( staticCloseCB, this );
    m_MainLayout.SetGroupAndScreen( m_FLTK_Window, this );

    m_MainLayout.ForceNewLine();
    m_MainLayout.AddY(7);
    m_MainLayout.AddX(5);

    m_MainLayout.AddSubGroupLayout( m_BorderLayout, m_MainLayout.GetRemainX() - 5.0,
                                    m_MainLayout.GetRemainY() - 5.0);

    int hcurrview = m_CurrentViewportSizeLayout.GetDividerHeight() +
    2 * m_CurrentViewportSizeLayout.GetStdHeight() +
    m_CurrentViewportSizeLayout.GetGapHeight();

    m_BorderLayout.AddSubGroupLayout( m_CurrentViewportSizeLayout, m_BorderLayout.GetRemainX(), hcurrview );
    m_BorderLayout.AddY( hcurrview );

    m_CurrentViewportSizeLayout.AddDividerBox("Viewport Size");
    m_CurrentViewportSizeLayout.AddYGap();
    m_CurrentViewportSizeLayout.SetButtonWidth( 60 );
    m_CurrentViewportSizeLayout.AddOutput( m_CurrentWidth, "Width" );
    m_CurrentViewportSizeLayout.AddOutput( m_CurrentHeight, "Height" );

    int hviewport = m_ViewportSizeLayout.GetDividerHeight() +
    4 * m_ViewportSizeLayout.GetStdHeight() +
    2 * m_ViewportSizeLayout.GetGapHeight();

    m_BorderLayout.AddYGap();
    m_BorderLayout.AddSubGroupLayout( m_ViewportSizeLayout, m_BorderLayout.GetRemainX(), hviewport );
    m_BorderLayout.AddY( hviewport );

    m_ViewportSizeLayout.AddDividerBox("Output Size");
    m_ViewportSizeLayout.AddYGap();

    m_ViewportSizeLayout.SetFitWidthFlag( false );
    m_ViewportSizeLayout.SetSameLineFlag( true );

    m_ViewportSizeLayout.SetSliderWidth( 100 );
    m_ViewportSizeLayout.SetButtonWidth( 20 );
    m_ViewportSizeLayout.AddButton(m_SelectRatio, "", 1);
    m_ViewportSizeLayout.SetButtonWidth( 60 );
    m_ViewportSizeLayout.AddSlider(m_NewRatio, "Ratio", 1.0, "%7.3f");
    m_ViewportSizeLayout.ForceNewLine();

    m_ViewportSizeLayout.SetButtonWidth( 20 );
    m_ViewportSizeLayout.AddButton(m_SelectWidth, "", -1);
    m_ViewportSizeLayout.SetButtonWidth( 60 );
    m_ViewportSizeLayout.AddSlider(m_NewWidth, "Width", 1.0, "%7.0f");
    m_ViewportSizeLayout.ForceNewLine();

    m_ViewportSizeLayout.SetButtonWidth( 20 );
    m_ViewportSizeLayout.AddButton(m_SelectHeight, "", -1);
    m_ViewportSizeLayout.SetButtonWidth( 60 );
    m_ViewportSizeLayout.AddSlider(m_NewHeight, "Height", 1.0, "%7.0f");
    m_ViewportSizeLayout.ForceNewLine();
    m_ViewportSizeLayout.AddYGap();

    m_ViewportSizeLayout.SetFitWidthFlag( true );
    m_ViewportSizeLayout.SetSameLineFlag( false );

    m_ViewportSizeLayout.AddButton(m_SetToCurrentSize, "Reset to Viewport Size");

    int htransparent = m_TransparentLayout.GetDividerHeight() +
                    1 * m_TransparentLayout.GetStdHeight() +
                    1 * m_TransparentLayout.GetGapHeight();

    m_BorderLayout.AddYGap();
    m_BorderLayout.AddSubGroupLayout( m_TransparentLayout, m_BorderLayout.GetRemainX(), hviewport );
    m_BorderLayout.AddY( htransparent );

    m_TransparentLayout.AddDividerBox( "Transparency" );
    m_TransparentLayout.AddYGap();
    m_TransparentLayout.AddButton( m_TransparentBG, "Transparent Background" );

    m_BorderLayout.AddYGap();
    m_BorderLayout.AddButton(m_CapturePNG, "Capture PNG");

    //Initialize Width and Height Values
    m_SelectRatio.GetFlButton()->value( 1 );

    MainVSPScreen* main = dynamic_cast<MainVSPScreen*>( m_ScreenMgr->GetScreen( m_ScreenMgr->VSP_MAIN_SCREEN ) );

    m_NewWidth.SetRange( 10.0 );
    m_NewHeight.SetRange( 10.0 );

    m_framebufferSupported = true;
    m_showedOpenGLErrorMessage = false;
}

ScreenshotScreen::~ScreenshotScreen()
{
}

void ScreenshotScreen::Show()
{
    if ( !glewIsSupported( "GL_ARB_framebuffer_object" ) )
    {
        m_ViewportSizeLayout.GetGroup()->deactivate();
        m_framebufferSupported = false;
    }

    m_ScreenMgr->SetUpdateFlag( true );
    m_FLTK_Window->show();

    //===== We only need OpenGL version 2.1. Framebuffer objects offered as an extension. =====//
    //===== GLEW associates the framebuffer calls to their appropriate extension calls. =====//
    if ( !glewIsSupported( "GL_ARB_framebuffer_object" ) && !m_showedOpenGLErrorMessage )
    {
        fl_message( "Some features may be disabled due to a lower version of OpenGL.\nUpgrade to OpenGL 2.1 or higher." );
        m_showedOpenGLErrorMessage = true;
    }
}

void ScreenshotScreen::Hide()
{
    m_FLTK_Window->hide();
    m_ScreenMgr->SetUpdateFlag( true );
}

bool ScreenshotScreen::Update()
{
    MainVSPScreen* main = dynamic_cast<MainVSPScreen*>( m_ScreenMgr->GetScreen( m_ScreenMgr->VSP_MAIN_SCREEN ) );
    if( !main )
    {
        return false;
    }

    VSPGUI::VspGlWindow * glwin = main->GetGLWindow();
    Vehicle * veh = m_ScreenMgr->GetVehiclePtr();

    //==== Update Current Width and Height ====//
    m_CurrentWidth.Update( std::to_string( glwin->pixel_w() ) );
    m_CurrentHeight.Update( std::to_string( glwin->pixel_h() ) );

    //==== First Update Width/Height/Ratio ====//
    m_NewRatio.Update( veh->m_NewRatioValue.GetID() );
    m_NewWidth.Update( veh->m_NewWidthValue.GetID() );
    m_NewHeight.Update( veh->m_NewHeightValue.GetID() );

    if ( m_SelectRatio.GetFlButton()->value() )
    {
        veh->m_NewWidthValue.Set( glwin->pixel_w() * veh->m_NewRatioValue.Get() );
        veh->m_NewHeightValue.Set( glwin->pixel_h() * veh->m_NewRatioValue.Get() );
    }
    else if ( m_SelectWidth.GetFlButton()->value() )
    {
        veh->m_NewRatioValue.Set( ( (float) veh->m_NewWidthValue.Get() ) / ( (float) glwin->pixel_w() ) );
        veh->m_NewHeightValue.Set( glwin->pixel_h() * veh->m_NewRatioValue.Get() );
    }
    else
    {
        veh->m_NewRatioValue.Set( ( (float) veh->m_NewHeightValue.Get() ) / ( (float) glwin->pixel_h() ) );
        veh->m_NewWidthValue.Set( glwin->pixel_w() * veh->m_NewRatioValue.Get() );
    }

    //==== Second Update Width/Height/Ratio ====//
    m_NewRatio.Update( veh->m_NewRatioValue.GetID() );
    m_NewWidth.Update( veh->m_NewWidthValue.GetID() );
    m_NewHeight.Update( veh->m_NewHeightValue.GetID() );

    //==== Must do deactivation after updating the values ====//
    m_NewRatio.Deactivate();
    m_NewWidth.Deactivate();
    m_NewHeight.Deactivate();

    if ( m_SelectRatio.GetFlButton()->value() )
    {
        m_NewRatio.Activate();
    }
    else if ( m_SelectWidth.GetFlButton()->value() )
    {
        m_NewWidth.Activate();
    }
    else
    {
        m_NewHeight.Activate();
    }

    m_TransparentBG.Update( veh->m_TransparentBGFlag.GetID() );

    m_FLTK_Window->redraw();
    return false;
}

// Callback for Link Browser
void ScreenshotScreen::CallBack( Fl_Widget* w )
{
    assert( m_ScreenMgr );

    m_ScreenMgr->SetUpdateFlag( true );
}

void ScreenshotScreen::CloseCallBack( Fl_Widget *w )
{
    Hide();
}

// Callback for all other GUI Devices
void ScreenshotScreen::GuiDeviceCallBack( GuiDevice* device )
{
    assert( m_ScreenMgr );

    MainVSPScreen* main = dynamic_cast<MainVSPScreen*>( m_ScreenMgr->GetScreen( m_ScreenMgr->VSP_MAIN_SCREEN ) );
    if( !main )
    {
        return;
    }

    Vehicle * veh = m_ScreenMgr->GetVehiclePtr();

    VSPGUI::VspGlWindow * glwin = main->GetGLWindow();

    if ( device == &m_SelectRatio )
    {
        m_SelectRatio.GetFlButton()->value( 1 );
        m_SelectWidth.GetFlButton()->value( 0 );
        m_SelectHeight.GetFlButton()->value( 0 );
    }
    else if ( device == &m_SelectWidth )
    {
        m_SelectRatio.GetFlButton()->value( 0 );
        m_SelectWidth.GetFlButton()->value( 1 );
        m_SelectHeight.GetFlButton()->value( 0 );
    }
    else if ( device == &m_SelectHeight )
    {
        m_SelectRatio.GetFlButton()->value( 0 );
        m_SelectWidth.GetFlButton()->value( 0 );
        m_SelectHeight.GetFlButton()->value( 1 );
    }
    else if ( device == &m_SetToCurrentSize )
    {
        veh->m_NewRatioValue.Set( 1.0 );
        if ( glwin )
        {
            veh->m_NewWidthValue.Set( glwin->pixel_w() );
            veh->m_NewHeightValue.Set( glwin->pixel_h() );
        }
    }
    else if ( device == &m_CapturePNG )
    {
        std::string fileName = m_ScreenMgr->GetSelectFileScreen()->FileChooser(
                "Create or Select a PNG File", "*.png" );

        if( !fileName.empty() && glwin )
        {
            glwin->getGraphicEngine()->dumpScreenImage( fileName, veh->m_NewWidthValue.Get(), veh->m_NewHeightValue.Get(), veh->m_TransparentBGFlag.Get(), m_framebufferSupported, VSPGraphic::GraphicEngine::PNG );
        }
    }
    m_ScreenMgr->SetUpdateFlag( true );
}
