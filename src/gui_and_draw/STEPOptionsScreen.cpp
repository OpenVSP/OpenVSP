//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//

// STEPOptionsScreen.cpp: implementation of the STEPOptionsScreen class.
//
//////////////////////////////////////////////////////////////////////

#include "STEPOptionsScreen.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

STEPOptionsScreen::STEPOptionsScreen( ScreenMgr* mgr ) : BasicScreen( mgr, 250, 170, "STEP Options" )
{
    m_FLTK_Window->callback( staticCloseCB, this );

    m_OkFlag = false;
    m_PrevSplit = false;
    m_PrevMerge = false;
    m_PrevCubic = false;;
    m_PrevTol = 1e-6;

    m_GenLayout.SetGroupAndScreen( m_FLTK_Window, this );
    m_GenLayout.AddY( 25 );

    m_GenLayout.AddYGap();

    m_GenLayout.AddButton( m_SplitSurfsToggle, "Split Surfaces" );
    m_GenLayout.AddYGap();
    m_GenLayout.AddButton( m_MergePointsToggle, "Merge Points" );
    m_GenLayout.AddYGap();
    m_GenLayout.AddButton( m_ToCubicToggle, "Convert to Cubic" );
    m_GenLayout.AddSlider( m_TolSlider, "Tolerance", 10, "%5.4g", true );

    m_GenLayout.AddY( 25 );
    m_GenLayout.SetFitWidthFlag( false );
    m_GenLayout.SetSameLineFlag( true );
    m_GenLayout.SetButtonWidth( 100 );

    m_GenLayout.AddX( 20 );
    m_GenLayout.AddButton( m_OkButton, "OK" );
    m_GenLayout.AddX( 10 );
    m_GenLayout.AddButton( m_CancelButton, "Cancel" );
    m_GenLayout.ForceNewLine();
}

STEPOptionsScreen::~STEPOptionsScreen()
{
}

bool STEPOptionsScreen::Update()
{
    Vehicle *veh = VehicleMgr.GetVehicle();

    if( veh )
    {
        m_SplitSurfsToggle.Update( veh->m_STEPSplitSurfs.GetID() );
        m_MergePointsToggle.Update( veh->m_STEPMergePoints.GetID() );
        m_ToCubicToggle.Update( veh->m_STEPToCubic.GetID() );
        m_TolSlider.Update( veh->m_STEPTolerance.GetID() );

        if ( !veh->m_STEPToCubic() )
        {
            m_TolSlider.Deactivate();
        }
    }

    m_FLTK_Window->redraw();

    return false;
}


void STEPOptionsScreen::Show()
{
    Update();
    m_FLTK_Window->show();
}

void STEPOptionsScreen::CallBack( Fl_Widget* w )
{
    m_ScreenMgr->SetUpdateFlag( true );
}

void STEPOptionsScreen::GuiDeviceCallBack( GuiDevice* device )
{
    assert( m_ScreenMgr );

    if ( device == &m_OkButton )
    {
        m_OkFlag = true;
        Hide();
    }
    else if ( device == &m_CancelButton )
    {
        Vehicle *veh = VehicleMgr.GetVehicle();

        if( veh )
        {
            veh->m_STEPSplitSurfs.Set( m_PrevSplit );
            veh->m_STEPMergePoints.Set( m_PrevMerge );
            veh->m_STEPToCubic.Set( m_PrevCubic );
            veh->m_STEPTolerance.Set( m_PrevTol );
        }
        Hide();
    }

    m_ScreenMgr->SetUpdateFlag( true );
}

bool STEPOptionsScreen::ShowSTEPOptionsScreen()
{
    Show();

    m_OkFlag = false;

    Vehicle *veh = VehicleMgr.GetVehicle();

    if( veh )
    {
        m_PrevSplit = veh->m_STEPSplitSurfs();
        m_PrevMerge = veh->m_STEPMergePoints();
        m_PrevCubic = veh->m_STEPToCubic();
        m_PrevTol = veh->m_STEPTolerance();
    }

    while( m_FLTK_Window->shown() )
    {
        Fl::wait();
    }

    return m_OkFlag;
}

void STEPOptionsScreen::CloseCallBack( Fl_Widget *w )
{
    assert( m_ScreenMgr );

    Vehicle *veh = VehicleMgr.GetVehicle();

    if( veh )
    {
        veh->m_STEPSplitSurfs.Set( m_PrevSplit );
        veh->m_STEPMergePoints.Set( m_PrevMerge );
        veh->m_STEPToCubic.Set( m_PrevCubic );
        veh->m_STEPTolerance.Set( m_PrevTol );
    }

    Hide();
}
