//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//
//
//////////////////////////////////////////////////////////////////////

#include "ManageBackgroundScreen.h"
#include "ScreenMgr.h"

#include "MainVSPScreen.h"
#include "MainGLWindow.h"

#include "GraphicEngine.h"
#include "Display.h"
#include "Viewport.h"
#include "Background.h"
#include "TextureRepo.h"
#include "GraphicSingletons.h"
#include "Common.h"

#include "FL/Fl_File_Chooser.H"

ManageBackgroundScreen::ManageBackgroundScreen( ScreenMgr* mgr ) : BasicScreen( mgr, 210, 347, "Background" )
{
    m_FLTK_Window->callback( staticCloseCB, this );
    m_MainLayout.SetGroupAndScreen( m_FLTK_Window, this );

    m_MainLayout.ForceNewLine();
    m_MainLayout.AddY(7);
    m_MainLayout.AddX(5);

    m_MainLayout.AddSubGroupLayout( m_BorderLayout, m_MainLayout.GetRemainX() - 5.0,
                                    m_MainLayout.GetRemainY() - 5.0);

    m_BorderLayout.AddButton(m_Color, "Color");
    m_BorderLayout.AddYGap();

    m_BorderLayout.AddSubGroupLayout(m_ColorLayout, m_BorderLayout.GetRemainX(), 100);
    m_BorderLayout.AddY(100);

    colorChooser = m_ColorLayout.AddFlColorChooser( 95 );
    colorChooser->callback( staticScreenCB, this );

    m_BorderLayout.AddButton(m_Image, "JPEG Image");
    m_BorderLayout.AddYGap();

    m_BorderLayout.AddSubGroupLayout(m_ImageLayout, m_BorderLayout.GetRemainX(), m_BorderLayout.GetRemainY() - 20);

    m_ImageLayout.SetFitWidthFlag( false );
    m_ImageLayout.SetSameLineFlag( true );

    m_ImageLayout.SetInputWidth( m_ImageLayout.GetRemainX() - 70 );
    m_ImageLayout.SetButtonWidth( 40 );
    m_ImageLayout.AddOutput( m_FileOutput, "File:");
    m_ImageLayout.SetButtonWidth( 30 );
    m_ImageLayout.AddButton( m_FileSelect, "..." );
    m_ImageLayout.ForceNewLine();
    m_ImageLayout.AddYGap();

    m_ImageLayout.SetFitWidthFlag( true );
    m_ImageLayout.SetSameLineFlag( false );

    m_ImageLayout.AddButton(m_PreserveAspect, "Preserve Aspect");
    m_ImageLayout.AddYGap();
    m_ImageLayout.SetButtonWidth( 60 );
    m_ImageLayout.SetInputWidth( 50 );
    m_ImageLayout.AddSlider(m_WScale, "W Scale", 1.0, "%7.3f");
    m_ImageLayout.AddSubGroupLayout(m_PreserveAspectLayout, m_ImageLayout.GetRemainX(), 25);
    m_ImageLayout.AddY(25);
    m_PreserveAspectLayout.SetButtonWidth( 60 );
    m_PreserveAspectLayout.SetInputWidth( 50 );
    m_PreserveAspectLayout.AddSlider(m_HScale, "H Scale", 1.0, "%7.3f");

    m_ImageLayout.AddSlider(m_XOffset, "X Offset", 0.500, "%7.3f");
    m_ImageLayout.AddSlider(m_YOffset, "Y Offset", 0.500, "%7.3f");
    m_ImageLayout.AddYGap();

    m_BorderLayout.AddY( 143 );
    m_BorderLayout.AddButton(m_ResetDefaults, "Reset Defaults");

    //Initialize Variables
    colorChooser->rgb(242.0/255.0, 242.0/255.0, 242.0/255.0);
    m_Color.GetFlButton()->value( 1 );
    m_Image.GetFlButton()->value( 0 );
    m_PreserveAspect.GetFlButton()->value( 1 );

    m_WidthScaleValue.Init("WidthScale", "Background", NULL, 1.0, -1.0e12, 1.0e12);
    m_HeightScaleValue.Init("HeightScale", "Background", NULL, 1.0, -1.0e12, 1.0e12);

    m_XOffsetValue.Init("XOffset", "Background", NULL, 0.0, -1.0e12, 1.0e12);
    m_YOffsetValue.Init("YOffset", "Background", NULL, 0.0, -1.0e12, 1.0e12);
}

ManageBackgroundScreen::~ManageBackgroundScreen()
{
}

void ManageBackgroundScreen::Show()
{
    m_ScreenMgr->SetUpdateFlag( true );
    m_FLTK_Window->show();
}

void ManageBackgroundScreen::Hide()
{
    m_FLTK_Window->hide();
    m_ScreenMgr->SetUpdateFlag( true );
}

bool ManageBackgroundScreen::Update()
{
    MainVSPScreen* main = dynamic_cast<MainVSPScreen*>( m_ScreenMgr->GetScreen( m_ScreenMgr->VSP_MAIN_SCREEN ) );
    if( !main )
    {
        return false;
    }

    VSPGUI::VspGlWindow * glwin = main->GetGLWindow();

    VSPGraphic::Viewport * viewport = glwin->getGraphicEngine()->getDisplay()->getViewport();
    if( !viewport )
    {
        return false;
    }

    VSPGraphic::Common::VSPenum mode = viewport->getBackground()->getBackgroundMode();
    if ( mode == VSPGraphic::Common::VSP_BACKGROUND_COLOR )
    {
        m_ColorLayout.GetGroup()->activate();
        m_ImageLayout.GetGroup()->deactivate();
    }
    else
    {
        m_ColorLayout.GetGroup()->deactivate();
        m_ImageLayout.GetGroup()->activate();
    }

    if ( m_PreserveAspect.GetFlButton()->value() )
    {
        m_PreserveAspectLayout.GetGroup()->deactivate();
    }
    else
    {
        m_PreserveAspectLayout.GetGroup()->activate();
    }

    //Update Scale and Offset in Background
    m_WScale.Update( m_WidthScaleValue.GetID() );
    m_HScale.Update( m_HeightScaleValue.GetID() );

    viewport->getBackground()->scaleW( (float) m_WidthScaleValue.Get() );
    if (m_PreserveAspect.GetFlButton()->value())
    {
        m_HeightScaleValue.Set( viewport->getBackground()->getScaleH() );
    }
    else
    {
        viewport->getBackground()->scaleH( (float) m_HeightScaleValue.Get() );
    }

    m_XOffset.Update( m_XOffsetValue.GetID() );
    m_YOffset.Update( m_YOffsetValue.GetID() );

    viewport->getBackground()->offsetX( (float) m_XOffsetValue.Get() );
    viewport->getBackground()->offsetY( (float) m_YOffsetValue.Get() );

    m_FLTK_Window->redraw();
    return false;
}

// Callback for Link Browser
void ManageBackgroundScreen::CallBack( Fl_Widget* w )
{
    assert( m_ScreenMgr );

    MainVSPScreen* main = dynamic_cast<MainVSPScreen*>( m_ScreenMgr->GetScreen( m_ScreenMgr->VSP_MAIN_SCREEN ) );
    if( !main )
    {
        return;
    }

    VSPGUI::VspGlWindow * glwin = main->GetGLWindow();

    VSPGraphic::Viewport * viewport = glwin->getGraphicEngine()->getDisplay()->getViewport();
    if( !viewport )
    {
        return;
    }

    if ( w == colorChooser )
    {
        viewport->getBackground()->setRed( ( float )( colorChooser->r() ) );
        viewport->getBackground()->setGreen( ( float )( colorChooser->g() ) );
        viewport->getBackground()->setBlue( ( float )( colorChooser->b() ) );
    }

    m_ScreenMgr->SetUpdateFlag( true );
}

void ManageBackgroundScreen::CloseCallBack( Fl_Widget *w )
{
    Hide();
}

// Callback for all other GUI Devices
void ManageBackgroundScreen::GuiDeviceCallBack( GuiDevice* device )
{
    assert( m_ScreenMgr );

    MainVSPScreen* main = dynamic_cast<MainVSPScreen*>( m_ScreenMgr->GetScreen( m_ScreenMgr->VSP_MAIN_SCREEN ) );
    if( !main )
    {
        return;
    }

    VSPGUI::VspGlWindow * glwin = main->GetGLWindow();

    VSPGraphic::Viewport * viewport = glwin->getGraphicEngine()->getDisplay()->getViewport();
    if( !viewport )
    {
        return;
    }

    if ( device == &m_Color )
    {
        m_Color.GetFlButton()->value( 1 );
        m_Image.GetFlButton()->value( 0 );
        viewport->getBackground()->removeImage();
        viewport->getBackground()->setBackgroundMode( VSPGraphic::Common::VSP_BACKGROUND_COLOR );
    }
    else if ( device == &m_Image )
    {
        m_Color.GetFlButton()->value( 0 );
        m_Image.GetFlButton()->value( 1 );
        viewport->getBackground()->setBackgroundMode( VSPGraphic::Common::VSP_BACKGROUND_IMAGE );

        if ( m_ImageFile.compare( "" ) != 0 )
        {
            viewport->getBackground()->attachImage( VSPGraphic::GlobalTextureRepo()->get2DTexture( m_ImageFile.c_str() ) );
        }
    }
    else if ( device == &m_FileSelect )
    {
        Fl_File_Chooser fc( ".", "TGA, JPG Files (*.{tga,jpg})", Fl_File_Chooser::SINGLE, "Read Texture?" );
        fc.show();

        while( fc.shown() )
        {
            Fl::wait();
        }
        if ( fc.value() != NULL )
        {
            viewport->getBackground()->removeImage();
            viewport->getBackground()->attachImage( VSPGraphic::GlobalTextureRepo()->get2DTexture( fc.value() ) );
            m_ImageFile = fc.value();
            m_FileOutput.Update( truncateFileName( fc.value(), 40 ).c_str() );
        }
    }
    else if ( device == &m_PreserveAspect )
    {
        if ( m_PreserveAspect.GetFlButton()->value() == 1 )
        {
            viewport->getBackground()->preserveAR( true );
        }
        else
        {
            viewport->getBackground()->preserveAR( false );
        }
    }
    else if ( device == &m_ResetDefaults )
    {
        viewport->getBackground()->reset();
        colorChooser->rgb( viewport->getBackground()->getRed(), viewport->getBackground()->getGreen(),
                           viewport->getBackground()->getBlue() );
        m_FileOutput.Update("");
        m_ImageFile = "";

        //Reset Scale & Offset
        m_WidthScaleValue.Set( viewport->getBackground()->getScaleW() );
        m_HeightScaleValue.Set( viewport->getBackground()->getScaleH() );

        m_XOffsetValue.Set( viewport->getBackground()->getOffsetX() );
        m_YOffsetValue.Set( viewport->getBackground()->getOffsetY() );
    }

    m_ScreenMgr->SetUpdateFlag( true );
}

string ManageBackgroundScreen::truncateFileName( const string &fn, int len )
{
    string trunc( fn );
    if ( (int)trunc.length() > len )
    {
        trunc.erase( 0, trunc.length() - len );
        trunc.replace( 0, 3, "..." );
    }
    return trunc;
}
