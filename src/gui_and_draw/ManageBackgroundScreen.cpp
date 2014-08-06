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

ManageBackgroundScreen::ManageBackgroundScreen( ScreenMgr * mgr ) : VspScreen( mgr )
{
    m_BackgroundUI = new BackgroundUI();
    m_FLTK_Window = m_BackgroundUI->UIWindow;

    m_BackgroundUI->colorBackButton->callback( staticCB, this );
    m_BackgroundUI->jpegBackButton->callback( staticCB, this );

    m_BackgroundUI->preserveAspectButton->callback( staticCB, this );

    m_BackgroundUI->redSlider->callback( staticCB, this );
    m_BackgroundUI->greenSlider->callback( staticCB, this );
    m_BackgroundUI->blueSlider->callback( staticCB, this );

    m_BackgroundUI->wScaleSlider->range( 0.5, 1.5 );
    m_BackgroundUI->wScaleSlider->callback( staticCB, this );
    m_BackgroundUI->wScaleInput->callback( staticCB, this );

    m_BackgroundUI->hScaleSlider->range( 0.5, 1.5 );
    m_BackgroundUI->hScaleSlider->callback( staticCB, this );
    m_BackgroundUI->hScaleInput->callback( staticCB, this );

    m_BackgroundUI->xOffsetSlider->range( -0.5, 0.5 );
    m_BackgroundUI->xOffsetSlider->callback( staticCB, this );
    m_BackgroundUI->xOffsetInput->callback( staticCB, this );

    m_BackgroundUI->yOffsetSlider->range( -0.5, 0.5 );
    m_BackgroundUI->yOffsetSlider->callback( staticCB, this );
    m_BackgroundUI->yOffsetInput->callback( staticCB, this );

    m_BackgroundUI->resetDefaultsButton->callback( staticCB, this );
}
ManageBackgroundScreen::~ManageBackgroundScreen()
{
    delete m_BackgroundUI;
}

void ManageBackgroundScreen::Show()
{
    if( Update() )
    {
        m_FLTK_Window->show();
    }
}

void ManageBackgroundScreen::Hide()
{
    m_FLTK_Window->hide();
}

bool ManageBackgroundScreen::Update()
{
    MainVSPScreen * main =
        dynamic_cast<MainVSPScreen*>( m_ScreenMgr->GetScreen( m_ScreenMgr->VSP_MAIN_SCREEN ) );
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
    switch( mode )
    {
    case VSPGraphic::Common::VSP_BACKGROUND_COLOR:
        m_BackgroundUI->colorBackButton->value( 1 );
        m_BackgroundUI->jpegBackButton->value( 0 );
        break;

    case VSPGraphic::Common::VSP_BACKGROUND_IMAGE:
        m_BackgroundUI->colorBackButton->value( 0 );
        m_BackgroundUI->jpegBackButton->value( 1 );
        break;

    default:
        break;
    }

    char str[256];

    VSPGraphic::Background * background = viewport->getBackground();

    m_BackgroundUI->preserveAspectButton->value( background->getARFlag() ? 1 : 0 );

    m_BackgroundUI->redSlider->value( background->getRed() * 255 );
    m_BackgroundUI->greenSlider->value( background->getGreen() * 255 );
    m_BackgroundUI->blueSlider->value( background->getBlue() * 255 );

    sprintf( str, "%6.3f", background->getScaleW() );
    m_BackgroundUI->wScaleInput->value( str );
    m_BackgroundUI->wScaleSlider->value( background->getScaleW() );

    sprintf( str, "%6.3f", background->getScaleH() );
    m_BackgroundUI->hScaleInput->value( str );
    m_BackgroundUI->hScaleSlider->value( background->getScaleH() );

    sprintf( str, "%6.3f", background->getOffsetX() );
    m_BackgroundUI->xOffsetInput->value( str );
    m_BackgroundUI->xOffsetSlider->value( background->getOffsetX() );

    sprintf( str, "%6.3f", background->getOffsetY() );
    m_BackgroundUI->yOffsetInput->value( str );
    m_BackgroundUI->yOffsetSlider->value( background->getOffsetY() );

    return true;
}

void ManageBackgroundScreen::CallBack( Fl_Widget * w )
{
    MainVSPScreen * main =
        dynamic_cast<MainVSPScreen*>( m_ScreenMgr->GetScreen( m_ScreenMgr->VSP_MAIN_SCREEN ) );
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

    if( w == m_BackgroundUI->redSlider )
    {
        viewport->getBackground()->setRed( ( float )( m_BackgroundUI->redSlider->value() / 255 ) );
    }
    else if( w == m_BackgroundUI->greenSlider )
    {
        viewport->getBackground()->setGreen( ( float )( m_BackgroundUI->greenSlider->value() / 255 ) );
    }
    else if( w == m_BackgroundUI->blueSlider )
    {
        viewport->getBackground()->setBlue( ( float )( m_BackgroundUI->blueSlider->value() / 255 ) );
    }
    else if( w == m_BackgroundUI->wScaleSlider )
    {
        viewport->getBackground()->scaleW( ( float )( m_BackgroundUI->wScaleSlider->value() ) );
    }
    else if( w == m_BackgroundUI->wScaleInput )
    {
        double inputValue = atof( m_BackgroundUI->wScaleInput->value() );

        viewport->getBackground()->scaleW( ( float )inputValue );

        m_BackgroundUI->wScaleSlider->range( inputValue - 0.5, inputValue + 0.5 );
    }
    else if( w == m_BackgroundUI->hScaleSlider )
    {
        viewport->getBackground()->scaleH( ( float )( m_BackgroundUI->hScaleSlider->value() ) );
    }
    else if( w == m_BackgroundUI->hScaleInput )
    {
        double inputValue = atof( m_BackgroundUI->hScaleInput->value() );

        viewport->getBackground()->scaleH( ( float )inputValue );

        m_BackgroundUI->hScaleSlider->range( inputValue - 0.5, inputValue + 0.5 );
    }
    else if( w == m_BackgroundUI->colorBackButton )
    {
        viewport->getBackground()->removeImage();
        viewport->getBackground()->setBackgroundMode( VSPGraphic::Common::VSP_BACKGROUND_COLOR );
    }
    else if( w == m_BackgroundUI->jpegBackButton )
    {
        Fl_File_Chooser fc( ".", "TGA, JPG Files (*.{tga,jpg})", Fl_File_Chooser::SINGLE, "Read Texture?" );
        fc.show();

        while( fc.shown() )
        {
            Fl::wait();
        }

        viewport->getBackground()->removeImage();

        if( fc.value() == NULL )
        {
            viewport->getBackground()->setBackgroundMode( VSPGraphic::Common::VSP_BACKGROUND_COLOR );
        }
        else
        {
            viewport->getBackground()->attachImage( VSPGraphic::GlobalTextureRepo()->get2DTexture( fc.value() ) );
            viewport->getBackground()->setBackgroundMode( VSPGraphic::Common::VSP_BACKGROUND_IMAGE );
        }
    }
    else if( w == m_BackgroundUI->preserveAspectButton )
    {
        if( m_BackgroundUI->preserveAspectButton->value() == 1 )
        {
            viewport->getBackground()->preserveAR( true );
        }
        else
        {
            viewport->getBackground()->preserveAR( false );
        }
    }
    else if( w == m_BackgroundUI->xOffsetSlider )
    {
        viewport->getBackground()->offsetX( ( float )( m_BackgroundUI->xOffsetSlider->value() ) );
    }
    else if( w == m_BackgroundUI->xOffsetInput )
    {
        double inputValue = atof( m_BackgroundUI->xOffsetInput->value() );

        viewport->getBackground()->offsetX( ( float )inputValue );

        m_BackgroundUI->xOffsetSlider->range( inputValue - 0.5, inputValue + 0.5 );
    }
    else if( w == m_BackgroundUI->yOffsetSlider )
    {
        viewport->getBackground()->offsetY( ( float )( m_BackgroundUI->yOffsetSlider->value() ) );
    }
    else if( w == m_BackgroundUI->yOffsetInput )
    {
        double inputValue = atof( m_BackgroundUI->yOffsetInput->value() );

        viewport->getBackground()->offsetY( ( float )inputValue );

        m_BackgroundUI->yOffsetSlider->range( inputValue - 0.5, inputValue + 0.5 );
    }
    else if( w == m_BackgroundUI->resetDefaultsButton )
    {
        viewport->getBackground()->reset();
    }
    m_ScreenMgr->SetUpdateFlag( true );
}