//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//
//
//////////////////////////////////////////////////////////////////////

#include "ManageBackgroundScreen.h"
#include "ScreenMgr.h"

#include "MainGLWindow.h"

#include "GraphicEngine.h"
#include "Display.h"
#include "Viewport.h"
#include "Background.h"
#include "GraphicSingletons.h"

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

    m_BorderLayout.AddButton(m_Image, "Image");
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

    Vehicle * veh = m_ScreenMgr->GetVehiclePtr();

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
    m_WScale.Update( veh->m_BGWidthScaleValue.GetID() );
    m_HScale.Update( veh->m_BGHeightScaleValue.GetID() );

    viewport->getBackground()->scaleW( (float) veh->m_BGWidthScaleValue.Get() );
    if (m_PreserveAspect.GetFlButton()->value())
    {
        veh->m_BGHeightScaleValue.Set(viewport->getBackground()->getScaleH() );
    }
    else
    {
        viewport->getBackground()->scaleH( (float) veh->m_BGHeightScaleValue.Get() );
    }

    m_XOffset.Update( veh->m_BGXOffsetValue.GetID() );
    m_YOffset.Update( veh->m_BGYOffsetValue.GetID() );

    viewport->getBackground()->offsetX( (float) veh->m_BGXOffsetValue.Get() );
    viewport->getBackground()->offsetY( (float) veh->m_BGYOffsetValue.Get() );

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
        viewport->getBackground()->setMeshColor( (float)( colorChooser->r() ), (float)( colorChooser->g() ), (float)( colorChooser->b() ) );
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

    Vehicle *veh = m_ScreenMgr->GetVehiclePtr();
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
        std::string fileName = m_ScreenMgr->GetSelectFileScreen()->FileChooser(
                "Select Image File", "*.{jpg,png,tga,bmp,gif}", false );

        if( !fileName.empty() )
        {
            viewport->getBackground()->removeImage();
            viewport->getBackground()->attachImage( VSPGraphic::GlobalTextureRepo()->get2DTexture( fileName.c_str() ) );
            m_ImageFile = fileName;
            m_FileOutput.Update( StringUtil::truncateFileName( fileName, 40 ).c_str() );
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
        veh->m_BGWidthScaleValue.Set(viewport->getBackground()->getScaleW() );
        veh->m_BGHeightScaleValue.Set(viewport->getBackground()->getScaleH() );

        veh->m_BGXOffsetValue.Set(viewport->getBackground()->getOffsetX() );
        veh->m_BGYOffsetValue.Set(viewport->getBackground()->getOffsetY() );
    }

    m_ScreenMgr->SetUpdateFlag( true );
}
