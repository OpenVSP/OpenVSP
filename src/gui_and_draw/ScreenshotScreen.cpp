//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//
//
//////////////////////////////////////////////////////////////////////

#include "ScreenshotScreen.h"
#include "ScreenMgr.h"

#include "MainVSPScreen.h"
#include "MainGLWindow.h"

#include "GraphicEngine.h"
#include "Display.h"
#include "Viewport.h"
#include "TextureRepo.h"
#include "GraphicSingletons.h"
#include "Common.h"

#include "FL/Fl_File_Chooser.H"

ScreenshotScreen::ScreenshotScreen( ScreenMgr* mgr ) : BasicScreen( mgr, 270, 233, "Screenshot" )
{
    m_FLTK_Window->callback( staticCloseCB, this );
    m_MainLayout.SetGroupAndScreen( m_FLTK_Window, this );

    m_MainLayout.ForceNewLine();
    m_MainLayout.AddY(7);
    m_MainLayout.AddX(5);

    m_MainLayout.AddSubGroupLayout( m_BorderLayout, m_MainLayout.GetRemainX() - 5.0,
                                    m_MainLayout.GetRemainY() - 5.0);

    m_BorderLayout.AddDividerBox("Current Size");
    m_BorderLayout.AddYGap();
    m_BorderLayout.SetButtonWidth( 60 );
    m_BorderLayout.AddOutput( m_CurrentWidth, "Width" );
    m_BorderLayout.AddOutput( m_CurrentHeight, "Height" );

    m_BorderLayout.AddYGap();

    m_BorderLayout.AddDividerBox("New Size");
    m_BorderLayout.AddYGap();

    m_BorderLayout.SetFitWidthFlag( false );
    m_BorderLayout.SetSameLineFlag( true );

    m_BorderLayout.SetSliderWidth( 100 );
    m_BorderLayout.SetButtonWidth( 20 );
    m_BorderLayout.AddButton(m_SelectRatio, "", 1);
    m_BorderLayout.SetButtonWidth( 60 );
    m_BorderLayout.AddSlider(m_NewRatio, "Ratio", 1.0, "%7.3f");
    m_BorderLayout.ForceNewLine();

    m_BorderLayout.SetButtonWidth( 20 );
    m_BorderLayout.AddButton(m_SelectWidth, "", -1);
    m_BorderLayout.SetButtonWidth( 60 );
    m_BorderLayout.AddSlider(m_NewWidth, "Width", 1.0, "%7.0f");
    m_BorderLayout.ForceNewLine();

    m_BorderLayout.SetButtonWidth( 20 );
    m_BorderLayout.AddButton(m_SelectHeight, "", -1);
    m_BorderLayout.SetButtonWidth( 60 );
    m_BorderLayout.AddSlider(m_NewHeight, "Height", 1.0, "%7.0f");
    m_BorderLayout.ForceNewLine();
    m_BorderLayout.AddYGap();

    m_BorderLayout.SetFitWidthFlag( true );
    m_BorderLayout.SetSameLineFlag( false );

    m_BorderLayout.AddButton(m_SetToCurrentSize, "Set to Current Size");
    m_BorderLayout.AddYGap();
    m_BorderLayout.AddButton(m_CaptureScreenshot, "Capture Screenshot");

    //Initialize Width and Height Values
    m_SelectRatio.GetFlButton()->value( 1 );

    MainVSPScreen* main = dynamic_cast<MainVSPScreen*>( m_ScreenMgr->GetScreen( m_ScreenMgr->VSP_MAIN_SCREEN ) );
    if( main )
    {
        VSPGUI::VspGlWindow * glwin = main->GetGLWindow();
        m_NewRatioValue.Init("Ratio", "Screenshot", NULL, 1.0, 0.0, 1.0e12);
        m_NewWidthValue.Init("Width", "Screenshot", NULL, glwin->w(), 0.0, 1.0e12);
        m_NewHeightValue.Init("Height", "Screenshot", NULL, glwin->h(), 0.0, 1.0e12);
    }
    else
    {
        m_NewRatioValue.Init("Ratio", "Screenshot", NULL, 1.0, 0.0, 1.0e12);
        m_NewWidthValue.Init("Width", "Screenshot", NULL, 1.0, 0.0, 1.0e12);
        m_NewHeightValue.Init("Height", "Screenshot", NULL, 1.0, 0.0, 1.0e12);
    }
    m_NewWidth.SetRange( 10.0 );
    m_NewHeight.SetRange( 10.0 );
}

ScreenshotScreen::~ScreenshotScreen()
{
}

void ScreenshotScreen::Show()
{
    m_ScreenMgr->SetUpdateFlag( true );
    m_FLTK_Window->show();
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

    //==== Update Current Width and Height ====//
    m_CurrentWidth.Update( std::to_string( glwin->w() ) );
    m_CurrentHeight.Update( std::to_string( glwin->h() ) );

    //==== First Update Width/Height/Ratio ====//
    m_NewRatio.Update( m_NewRatioValue.GetID() );
    m_NewWidth.Update( m_NewWidthValue.GetID() );
    m_NewHeight.Update( m_NewHeightValue.GetID() );

    if ( m_SelectRatio.GetFlButton()->value() )
    {
        m_NewWidthValue.Set( glwin->w() * m_NewRatioValue.Get() );
        m_NewHeightValue.Set( glwin->h() * m_NewRatioValue.Get() );
    }
    else if ( m_SelectWidth.GetFlButton()->value() )
    {
        m_NewRatioValue.Set( ( (float) m_NewWidthValue.Get() ) / ( (float) glwin->w() ) );
        m_NewHeightValue.Set( glwin->h() * m_NewRatioValue.Get() );
    }
    else
    {
        m_NewRatioValue.Set( ( (float) m_NewHeightValue.Get() ) / ( (float) glwin->h() ) );
        m_NewWidthValue.Set( glwin->w() * m_NewRatioValue.Get() );
    }

    //==== Second Update Width/Height/Ratio ====//
    m_NewRatio.Update( m_NewRatioValue.GetID() );
    m_NewWidth.Update( m_NewWidthValue.GetID() );
    m_NewHeight.Update( m_NewHeightValue.GetID() );

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
        m_NewRatioValue.Set( 1.0 );
        m_NewWidthValue.Set( glwin->w() );
        m_NewHeightValue.Set( glwin->h() );
    }
    else if ( device == &m_CaptureScreenshot )
    {
        std::string fileName = m_ScreenMgr->GetSelectFileScreen()->FileChooser(
                "Create or Select a JPG File", "*.jpg" );

        if( !fileName.empty() )
        {
            std::string::size_type extIndex = fileName.find_last_of( '.' );

            if( extIndex == std::string::npos )
            {
                fileName += ".jpg";
            }
            else
            {
                std::string ext = fileName.substr( extIndex, fileName.size() - extIndex );
                std::transform( ext.begin(), ext.end(), ext.begin(), ::tolower );
                if( ext != ".jpg" )
                {
                    fileName += ".jpg";
                }
            }
            glwin->getGraphicEngine()->dumpScreenJPEG( fileName, m_NewWidthValue.Get(), m_NewHeightValue.Get() );
        }
    }

    m_ScreenMgr->SetUpdateFlag( true );
}

string ScreenshotScreen::truncateFileName( const string &fn, int len )
{
    string trunc( fn );
    if ( (int)trunc.length() > len )
    {
        trunc.erase( 0, trunc.length() - len );
        trunc.replace( 0, 3, "..." );
    }
    return trunc;
}
