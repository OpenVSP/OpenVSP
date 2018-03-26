//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//

// AirfoilExportScreen.cpp: implementation of the AirfoilExportScreen class.
//
//////////////////////////////////////////////////////////////////////

#include "AirfoilExportScreen.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

SeligAirfoilExportScreen::SeligAirfoilExportScreen( ScreenMgr* mgr ) : BasicScreen( mgr, 250, 174, "Selig Airfoil Options" )
{
    m_OkFlag = false;

    m_GenLayout.SetGroupAndScreen( m_FLTK_Window, this );
    m_GenLayout.AddY( 25 );

    m_GenLayout.AddYGap();

    m_GenLayout.AddButton( m_AppendGeomIDToggle, "Append Geom ID to Airfoil File Names" );

    m_GenLayout.AddYGap();

    m_GenLayout.SetButtonWidth( 5 * m_GenLayout.GetW() / 12 );
    m_GenLayout.AddSlider( m_WTessFactorSlider, "W Tess Factor", 9, "%6.2f" );

    m_GenLayout.AddY( 70 );
    m_GenLayout.SetSameLineFlag( true );
    m_GenLayout.SetFitWidthFlag( false );
    m_GenLayout.SetButtonWidth( 100 );

    m_GenLayout.AddX( 20 );
    m_GenLayout.AddButton( m_OkButton, "OK" );
    m_GenLayout.AddX( 10 );
    m_GenLayout.AddButton( m_CancelButton, "Cancel" );
    m_GenLayout.ForceNewLine();
}

SeligAirfoilExportScreen::~SeligAirfoilExportScreen()
{
}

bool SeligAirfoilExportScreen::Update()
{
    Vehicle* veh = m_ScreenMgr->GetVehiclePtr();
    if ( !veh )
    {
        return false;
    }

    m_WTessFactorSlider.Update( veh->m_AFWTessFactor.GetID() );
    m_AppendGeomIDToggle.Update( veh->m_AFAppendGeomIDFlag.GetID() );

    m_FLTK_Window->redraw();

    return true;
}

void SeligAirfoilExportScreen::CallBack( Fl_Widget* w )
{
    m_ScreenMgr->SetUpdateFlag( true );
}

void SeligAirfoilExportScreen::GuiDeviceCallBack( GuiDevice* device )
{
    assert( m_ScreenMgr );

    if ( device == &m_OkButton )
    {
        m_OkFlag = true;
        Vehicle *veh = VehicleMgr.GetVehicle();

        Hide();
    }
    else if ( device == &m_CancelButton )
    {
        m_OkFlag = false;
        Hide();
    }

    m_ScreenMgr->SetUpdateFlag( true );
}

void SeligAirfoilExportScreen::Show()
{
    Update();
    m_FLTK_Window->show();
}

bool SeligAirfoilExportScreen::ShowAirfoilExportScreen()
{
    Show();

    m_OkFlag = false;

    while ( m_FLTK_Window->shown() )
    {
        Fl::wait();
    }

    return m_OkFlag;
}

BezierAirfoilExportScreen::BezierAirfoilExportScreen( ScreenMgr* mgr ) : BasicScreen( mgr, 250, 174, "Bezier Airfoil Options" )
{
    m_OkFlag = false;

    m_GenLayout.SetGroupAndScreen( m_FLTK_Window, this );
    m_GenLayout.AddY( 25 );

    m_GenLayout.AddYGap();

    m_GenLayout.AddButton( m_AppendGeomIDToggle, "Append Geom ID to Airfoil File Names" );

    m_GenLayout.AddY( 95 );
    m_GenLayout.SetSameLineFlag( true );
    m_GenLayout.SetFitWidthFlag( false );
    m_GenLayout.SetButtonWidth( 100 );

    m_GenLayout.AddX( 20 );
    m_GenLayout.AddButton( m_OkButton, "OK" );
    m_GenLayout.AddX( 10 );
    m_GenLayout.AddButton( m_CancelButton, "Cancel" );
    m_GenLayout.ForceNewLine();
}

BezierAirfoilExportScreen::~BezierAirfoilExportScreen()
{
}

bool BezierAirfoilExportScreen::Update()
{
    Vehicle* veh = m_ScreenMgr->GetVehiclePtr();
    if ( !veh )
    {
        return false;
    }

    m_AppendGeomIDToggle.Update( veh->m_AFAppendGeomIDFlag.GetID() );

    m_FLTK_Window->redraw();

    return true;
}

void BezierAirfoilExportScreen::CallBack( Fl_Widget* w )
{
    m_ScreenMgr->SetUpdateFlag( true );
}

void BezierAirfoilExportScreen::GuiDeviceCallBack( GuiDevice* device )
{
    assert( m_ScreenMgr );

    if ( device == &m_OkButton )
    {
        m_OkFlag = true;
        Vehicle *veh = VehicleMgr.GetVehicle();

        Hide();
    }
    else if ( device == &m_CancelButton )
    {
        m_OkFlag = false;
        Hide();
    }

    m_ScreenMgr->SetUpdateFlag( true );
}

void BezierAirfoilExportScreen::Show()
{
    Update();
    m_FLTK_Window->show();
}

bool BezierAirfoilExportScreen::ShowAirfoilExportScreen()
{
    Show();

    m_OkFlag = false;

    while ( m_FLTK_Window->shown() )
    {
        Fl::wait();
    }

    return m_OkFlag;
}