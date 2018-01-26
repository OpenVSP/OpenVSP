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

AirfoilExportScreen::AirfoilExportScreen( ScreenMgr* mgr ) : BasicScreen( mgr, 250, 174, "Airfoil Export Options" )
{
    m_OkFlag = false;

    m_GenLayout.SetGroupAndScreen( m_FLTK_Window, this );
    m_GenLayout.AddY( 25 );

    m_GenLayout.AddYGap();

    m_GenLayout.AddDividerBox( "File Format" );

    m_GenLayout.SetSameLineFlag( true );
    m_GenLayout.SetFitWidthFlag( false );
    m_GenLayout.SetButtonWidth( m_GenLayout.GetW() / 2 );

    m_GenLayout.AddButton( m_SeligToggle, "Selig (.dat)" );
    m_GenLayout.AddButton( m_BezierToggle, "Bezier (.bz)" );

    m_AirfoilTypeGroup.Init( this );
    m_AirfoilTypeGroup.AddButton( m_SeligToggle.GetFlButton() );
    m_AirfoilTypeGroup.AddButton( m_BezierToggle.GetFlButton() );

    m_GenLayout.ForceNewLine();
    m_GenLayout.AddYGap();
    m_GenLayout.SetSameLineFlag( false );
    m_GenLayout.SetFitWidthFlag( true );

    m_GenLayout.AddDividerBox( "Options" );

    m_GenLayout.AddButton( m_AppendGeomIDToggle, "Append Geom ID to Airfoil File Names" );

    m_GenLayout.SetButtonWidth( 5 * m_GenLayout.GetW() / 12 );
    m_GenLayout.AddSlider( m_WTessFactorSlider, "W Tess Factor", 9, "%6.2f" );

    m_GenLayout.AddY( 20 );
    m_GenLayout.SetSameLineFlag( true );
    m_GenLayout.SetFitWidthFlag( false );
    m_GenLayout.SetButtonWidth( 100 );

    m_GenLayout.AddX( 20 );
    m_GenLayout.AddButton( m_OkButton, "OK" );
    m_GenLayout.AddX( 10 );
    m_GenLayout.AddButton( m_CancelButton, "Cancel" );
    m_GenLayout.ForceNewLine();
}

AirfoilExportScreen::~AirfoilExportScreen()
{
}

bool AirfoilExportScreen::Update()
{
    Vehicle* veh = m_ScreenMgr->GetVehiclePtr();
    if ( !veh )
    {
        return false;
    }

    m_AirfoilTypeGroup.Update( veh->m_AFExportType.GetID() );

    m_WTessFactorSlider.Update( veh->m_AFWTessFactor.GetID() );

    if ( veh->m_AFExportType() == vsp::BEZIER_AF_EXPORT )
    {
        m_WTessFactorSlider.Deactivate();
    }
    else
    {
        m_WTessFactorSlider.Activate();
    }

    m_AppendGeomIDToggle.Update( veh->m_AFAppendGeomIDFlag.GetID() );

    m_FLTK_Window->redraw();

    return true;
}

void AirfoilExportScreen::CallBack( Fl_Widget* w )
{
    m_ScreenMgr->SetUpdateFlag( true );
}

void AirfoilExportScreen::GuiDeviceCallBack( GuiDevice* device )
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

void AirfoilExportScreen::Show()
{
    Update();
    m_FLTK_Window->show();
}

bool AirfoilExportScreen::ShowAirfoilExportScreen()
{
    Show();

    m_OkFlag = false;

    while ( m_FLTK_Window->shown() )
    {
        Fl::wait();
    }

    return m_OkFlag;
}
