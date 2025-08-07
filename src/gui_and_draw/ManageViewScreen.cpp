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

    m_BorderLayout.AddButton( m_PickNormalBtn, "View Normal To" );
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

    BasicScreen::Show();
}

void ManageViewScreen::Hide()
{
    m_FLTK_Window->hide();
    m_ScreenMgr->SetUpdateFlag( true );
}

bool ManageViewScreen::Update()
{
    BasicScreen::Update();

    Vehicle *veh = VehicleMgr.GetVehicle();
    if( !veh )
    {
        return false;
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

    m_FLTK_Window->redraw();
    return false;
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
        MainVSPScreen* main = dynamic_cast<MainVSPScreen*>( m_ScreenMgr->GetScreen( vsp::VSP_MAIN_SCREEN ) );
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
        ( m_ScreenMgr->GetScreen( vsp::VSP_COR_SCREEN ) );
        if( corScreen )
        {
            corScreen->EnableCORSelection();
        }
    }
    else if ( device == &m_PickNormalBtn )
    {
        ManageCORScreen* corScreen = dynamic_cast<ManageCORScreen *>
        ( m_ScreenMgr->GetScreen( vsp::VSP_COR_SCREEN ) );
        if( corScreen )
        {
            corScreen->EnableVNSelection();
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

        MainVSPScreen* main = dynamic_cast<MainVSPScreen*>( m_ScreenMgr->GetScreen( vsp::VSP_MAIN_SCREEN ) );
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
