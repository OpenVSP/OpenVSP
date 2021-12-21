//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//
//////////////////////////////////////////////////////////////////////

#include "ManageViewScreen.h"
#include "ScreenMgr.h"

#include "MainGLWindow.h"

#include "ManageCORScreen.h"

#include "GraphicEngine.h"
#include "Display.h"

ManageViewScreen::ManageViewScreen( ScreenMgr* mgr ) : BasicScreen( mgr, 250, 445 + 45 + 45, "Adjust View" )
{
    m_FLTK_Window->callback( staticCloseCB, this );
    m_MainLayout.SetGroupAndScreen( m_FLTK_Window, this );

    m_MainLayout.ForceNewLine();
    m_MainLayout.AddY(7);
    m_MainLayout.AddX(5);

    m_MainLayout.AddSubGroupLayout( m_BorderLayout, m_MainLayout.GetRemainX() - 5.0,
                                    m_MainLayout.GetRemainY() - 5.0);

    //===== Viewport Section =====//

    m_BorderLayout.AddDividerBox( "Viewport Size" );
    m_BorderLayout.AddYGap();
    m_BorderLayout.SetButtonWidth( 50 );
    m_BorderLayout.AddSlider( m_ViewportSizeX, "Width:", 250.0, "%7.0f" );
    m_BorderLayout.AddSlider( m_ViewportSizeY, "Height:", 250.0, "%7.0f" );
    m_BorderLayout.AddYGap();
    m_BorderLayout.AddButton(m_SetDefaultViewportSize, "Reset Viewport Size");
    m_BorderLayout.AddYGap();

    //===== Look At Point Section =====//

    m_BorderLayout.AddDividerBox( "Center of Rotation" );
    m_BorderLayout.AddYGap();

    m_BorderLayout.AddSlider( m_CORX, "X:", 10.0, "%7.3f" );
    m_BorderLayout.AddSlider( m_CORY, "Y:", 10.0, "%7.3f" );
    m_BorderLayout.AddSlider( m_CORZ, "Z:", 10.0, "%7.3f" );
    m_BorderLayout.AddYGap();

    m_BorderLayout.AddButton( m_PickLookAtBtn, "Pick Point" );
    m_BorderLayout.AddYGap();
    m_BorderLayout.AddButton( m_ResetOriginLookAtBtn, "Reset To Origin" );
    m_BorderLayout.AddYGap();

    //===== Camera Movement =====//

    m_BorderLayout.AddDividerBox( "Camera Movement" );
    m_BorderLayout.AddYGap();

    m_BorderLayout.SetButtonWidth( 50 );

    m_BorderLayout.AddSlider( m_PanXPos, "Pan X:", 10.0, "%7.3f" );
    m_BorderLayout.AddSlider( m_PanYPos, "Pan Y:", 10.0, "%7.3f" );
    m_BorderLayout.AddYGap();

    m_BorderLayout.AddSlider( m_Zoom, "Zoom:", 1, "%7.3g", 0, true );
    m_BorderLayout.AddYGap();

    //===== Attempt at Euler Angle Rotation =====//
    m_BorderLayout.AddSlider( m_XRotation, "X Rot:", 10.0, "%7.3f" );
    m_BorderLayout.AddSlider( m_YRotation, "Y Rot:", 10.0, "%7.3f" );
    m_BorderLayout.AddSlider( m_ZRotation, "Z Rot:", 10.0, "%7.3f" );
    m_BorderLayout.AddYGap();

    m_BorderLayout.AddButton( m_ResetCamera, "Reset Camera" );

    m_BorderLayout.AddYGap();
    m_BorderLayout.AddDividerBox( "Axis Marker Size" );
    m_BorderLayout.AddYGap();

    m_BorderLayout.AddSlider( m_AxisLenSlider, "Len", 10.0, "%7.3f" );

    m_BorderLayout.AddYGap();
    m_BorderLayout.AddDividerBox( "Text Label Size" );
    m_BorderLayout.AddYGap();

    m_BorderLayout.AddSlider( m_TextSizeSlider, "Size", 10.0, "%5.1f" );

}

ManageViewScreen::~ManageViewScreen()
{
}

void ManageViewScreen::Show()
{
    m_ScreenMgr->SetUpdateFlag( true );

    MainVSPScreen* main = dynamic_cast<MainVSPScreen*>( m_ScreenMgr->GetScreen( m_ScreenMgr->VSP_MAIN_SCREEN ) );
    if( main )
    {
        Vehicle *veh = VehicleMgr.GetVehicle();

        VSPGUI::VspGlWindow * glwin = main->GetGLWindow();

        // Get OpenGL Pixel width/height
        veh->m_ViewportSizeXValue.Set( glwin->pixel_w() );
        veh->m_ViewportSizeYValue.Set( glwin->pixel_h() );

        //===== Set current COR =====//
        glm::vec3 currentCOR = glwin->getCOR();
        veh->m_CORXValue.Set( currentCOR.x );
        veh->m_CORYValue.Set( currentCOR.y );
        veh->m_CORZValue.Set( currentCOR.z );

        glm::vec2 currentPan = glwin->getPanValues();
        veh->m_PanXPosValue.Set( currentPan.x );
        veh->m_PanYPosValue.Set( currentPan.y );

        veh->m_ZoomValue.Set( glwin->getRelativeZoomValue() );
        m_Zoom.Update( veh->m_ZoomValue.GetID() );

        UpdateRotations();

        // Save differences in FLTK pixels
        m_windowDX = main->GetFlWindow()->w() - glwin->w();
        m_windowDY = main->GetFlWindow()->h() - glwin->h();
    }

    m_FLTK_Window->show();
}

void ManageViewScreen::Hide()
{
    m_FLTK_Window->hide();
    m_ScreenMgr->SetUpdateFlag( true );
}

bool ManageViewScreen::Update()
{
    MainVSPScreen* main = dynamic_cast<MainVSPScreen*>( m_ScreenMgr->GetScreen( m_ScreenMgr->VSP_MAIN_SCREEN ) );
    if( !main )
    {
        return false;
    }
    Vehicle *veh = VehicleMgr.GetVehicle();

    VSPGUI::VspGlWindow * glwin = main->GetGLWindow();

    double factor = glwin->pixels_per_unit();

    // Added padding to screen
    if (veh->m_ViewportSizeXValue.Get()/factor > Fl::w() - 10)
    {
        veh->m_ViewportSizeXValue.Set( (Fl::w() - 10) * factor );
    }
    if (veh->m_ViewportSizeYValue.Get()/factor > Fl::h() - 10)
    {
        veh->m_ViewportSizeYValue.Set( (Fl::h() - 10) * factor );
    }

    m_AxisLenSlider.Update( veh->m_AxisLength.GetID() );
    m_TextSizeSlider.Update( veh->m_TextSize.GetID() );

    m_ViewportSizeX.Update( veh->m_ViewportSizeXValue.GetID() );
    m_ViewportSizeY.Update( veh->m_ViewportSizeYValue.GetID() );

    m_CORX.Update( veh->m_CORXValue.GetID() );
    m_CORY.Update( veh->m_CORYValue.GetID() );
    m_CORZ.Update( veh->m_CORZValue.GetID() );

    m_PanXPos.Update( veh->m_PanXPosValue.GetID() );
    m_PanYPos.Update( veh->m_PanYPosValue.GetID() );

    m_Zoom.Update( veh->m_ZoomValue.GetID() );

    //===== Euler Attempt =====//
    m_XRotation.Update( veh->m_XRotationValue.GetID() );
    m_YRotation.Update( veh->m_YRotationValue.GetID() );
    m_ZRotation.Update( veh->m_ZRotationValue.GetID() );

    //===== Do glwin functions here after the updates of the sliders =====//

    // Resize Viewport and window to your maximum screen size. Achieves any ratio.
    main->ResizeWindow( veh->m_ViewportSizeXValue.Get()/factor + m_windowDX, veh->m_ViewportSizeYValue.Get()/factor + m_windowDY );

    //===== Update Center of Rotation =====//
    glwin->setCOR( glm::vec3( veh->m_CORXValue.Get(), veh->m_CORYValue.Get(), veh->m_CORZValue.Get() ) );

    glwin->relativePan( veh->m_PanXPosValue.Get(), veh->m_PanYPosValue.Get() );

    glwin->relativeZoom( veh->m_ZoomValue.Get() );

    //===== LookAt Point Method =====//
    glwin->rotateSphere( veh->m_XRotationValue.Get() * ( M_PI / 180.0 ),
                         veh->m_YRotationValue.Get() * ( M_PI / 180.0 ),
                         veh->m_ZRotationValue.Get() * ( M_PI / 180.0 ) );

    m_FLTK_Window->redraw();
    return false;
}

void ManageViewScreen::UpdateViewport()
{
    MainVSPScreen* main = dynamic_cast<MainVSPScreen*>( m_ScreenMgr->GetScreen( m_ScreenMgr->VSP_MAIN_SCREEN ) );
    if( main )
    {
        Vehicle *veh = VehicleMgr.GetVehicle();
        veh->m_ViewportSizeXValue.Set( main->GetGLWindow()->pixel_w() );
        veh->m_ViewportSizeYValue.Set( main->GetGLWindow()->pixel_h() );

        m_ViewportSizeX.Update( veh->m_ViewportSizeXValue.GetID() );
        m_ViewportSizeY.Update( veh->m_ViewportSizeYValue.GetID() );
    }
}

void ManageViewScreen::UpdateCOR()
{
    MainVSPScreen* main = dynamic_cast<MainVSPScreen*>( m_ScreenMgr->GetScreen( m_ScreenMgr->VSP_MAIN_SCREEN ) );
    Vehicle *veh = VehicleMgr.GetVehicle();
    if( main )
    {
        VSPGUI::VspGlWindow * glwin = main->GetGLWindow();

        glm::vec3 center = glwin->getCOR();

        veh->m_CORXValue.Set( center.x );
        veh->m_CORYValue.Set( center.y );
        veh->m_CORZValue.Set( center.z );

        m_CORX.Update( veh->m_CORXValue.GetID() );
        m_CORY.Update( veh->m_CORYValue.GetID() );
        m_CORZ.Update( veh->m_CORZValue.GetID() );
    }
}

void ManageViewScreen::UpdatePan()
{
    MainVSPScreen* main = dynamic_cast<MainVSPScreen*>( m_ScreenMgr->GetScreen( m_ScreenMgr->VSP_MAIN_SCREEN ) );
    Vehicle *veh = VehicleMgr.GetVehicle();
    if( main )
    {
        VSPGUI::VspGlWindow * glwin = main->GetGLWindow();

        glm::vec2 currentPan = glwin->getPanValues();
        veh->m_PanXPosValue.Set( currentPan.x );
        veh->m_PanYPosValue.Set( currentPan.y );

        m_PanXPos.Update( veh->m_PanXPosValue.GetID() );
        m_PanYPos.Update( veh->m_PanYPosValue.GetID() );
    }
}

void ManageViewScreen::UpdateZoom()
{
    MainVSPScreen* main = dynamic_cast<MainVSPScreen*>( m_ScreenMgr->GetScreen( m_ScreenMgr->VSP_MAIN_SCREEN ) );
    Vehicle *veh = VehicleMgr.GetVehicle();
    if( main )
    {
        VSPGUI::VspGlWindow * glwin = main->GetGLWindow();

        veh->m_ZoomValue.Set( glwin->getRelativeZoomValue() );
        m_Zoom.Update( veh->m_ZoomValue.GetID() );
    }
}

//===== Attempt at Euler Angle Rotation =====//
void ManageViewScreen::UpdateRotations()
{
    MainVSPScreen* main = dynamic_cast<MainVSPScreen*>( m_ScreenMgr->GetScreen( m_ScreenMgr->VSP_MAIN_SCREEN ) );
    Vehicle *veh = VehicleMgr.GetVehicle();
    if( main )
    {
        VSPGUI::VspGlWindow * glwin = main->GetGLWindow();

        glm::vec3 eulerValues = glwin->getRotationEulerAngles();

        veh->m_XRotationValue.Set( eulerValues[0] * ( 180.0 / M_PI ) );
        veh->m_YRotationValue.Set( eulerValues[1] * ( 180.0 / M_PI ) );
        veh->m_ZRotationValue.Set( eulerValues[2] * ( 180.0 / M_PI ) );

        m_XRotation.Update( veh->m_XRotationValue.GetID() );
        m_YRotation.Update( veh->m_YRotationValue.GetID() );
        m_ZRotation.Update( veh->m_ZRotationValue.GetID() );
    }
}

void ManageViewScreen::UpdateAll()
{
    UpdateCOR();
    UpdatePan();
    UpdateZoom();
    UpdateRotations();
}

// Callback for Link Browser
void ManageViewScreen::CallBack( Fl_Widget* w )
{
    assert( m_ScreenMgr );

    m_ScreenMgr->SetUpdateFlag( true );
}

void ManageViewScreen::CloseCallBack( Fl_Widget *w )
{
    Hide();
}

// Callback for all other GUI Devices
void ManageViewScreen::GuiDeviceCallBack( GuiDevice* device )
{
    assert( m_ScreenMgr );
    Vehicle *veh = VehicleMgr.GetVehicle();

    //===== Trigger Buttons =====//
    if ( device == &m_SetDefaultViewportSize )
    {
        MainVSPScreen* main = dynamic_cast<MainVSPScreen*>( m_ScreenMgr->GetScreen( m_ScreenMgr->VSP_MAIN_SCREEN ) );
        if( main )
        {
            VSPGUI::VspGlWindow * glwin = main->GetGLWindow();

            veh->m_ViewportSizeXValue.Set( glwin->getGraphicEngine()->getDisplay()->getDefaultGlWindowWidth( ) );
            veh->m_ViewportSizeYValue.Set( glwin->getGraphicEngine()->getDisplay()->getDefaultGlWindowHeight( ) );
        }
    }
    else if ( device == &m_PickLookAtBtn )
    {
        ManageCORScreen* corScreen = dynamic_cast<ManageCORScreen *>
        ( m_ScreenMgr->GetScreen( ScreenMgr::VSP_COR_SCREEN ) );
        if( corScreen )
        {
            corScreen->EnableSelection();
        }
    }
    else if ( device == &m_ResetOriginLookAtBtn )
    {
        veh->m_CORXValue.Set( 0.0 );
        veh->m_CORYValue.Set( 0.0 );
        veh->m_CORZValue.Set( 0.0 );

        m_CORX.Update( veh->m_CORXValue.GetID() );
        m_CORY.Update( veh->m_CORYValue.GetID() );
        m_CORZ.Update( veh->m_CORZValue.GetID() );

        MainVSPScreen* main = dynamic_cast<MainVSPScreen*>( m_ScreenMgr->GetScreen( m_ScreenMgr->VSP_MAIN_SCREEN ) );
        if( main )
        {
            VSPGUI::VspGlWindow * glwin = main->GetGLWindow();

            glwin->setCOR( glm::vec3( veh->m_CORXValue.Get(), veh->m_CORYValue.Get(), veh->m_CORZValue.Get() ) );
        }
    }
    else if ( device == &m_ResetCamera )
    {
        //===== Reset the values to default =====//
        veh->m_PanXPosValue.Set( 0.0 );
        veh->m_PanYPosValue.Set( 0.0 );

        veh->m_ZoomValue.Set( 0.018 );

        veh->m_XRotationValue.Set( 0.0 );
        veh->m_YRotationValue.Set( 0.0 );
        veh->m_ZRotationValue.Set( 0.0 );

        //===== Update The Values =====//
        m_PanXPos.Update( veh->m_PanXPosValue.GetID() );
        m_PanYPos.Update( veh->m_PanYPosValue.GetID() );

        m_Zoom.Update( veh->m_ZoomValue.GetID() );

        m_XRotation.Update( veh->m_XRotationValue.GetID() );
        m_YRotation.Update( veh->m_YRotationValue.GetID() );
        m_ZRotation.Update( veh->m_ZRotationValue.GetID() );
    }

    m_ScreenMgr->SetUpdateFlag( true );
}
