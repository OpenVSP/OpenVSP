//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//

// IGESOptionsScreen.cpp: implementation of the IGESOptionsScreen class.
//
//////////////////////////////////////////////////////////////////////

#include "IGESOptionsScreen.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

IGESOptionsScreen::IGESOptionsScreen( ScreenMgr* mgr ) : BasicScreen( mgr, 250, 174, "IGES Options" )
{
    m_FLTK_Window->callback( staticCloseCB, this );

    m_OkFlag = false;
    m_PrevUnit = 0;
    m_PrevSplit = false;
    m_PrevCubic = false;;
    m_PrevToCubicTol = 1e-6;

    m_GenLayout.SetGroupAndScreen( m_FLTK_Window, this );
    m_GenLayout.AddY( 25 );

    m_GenLayout.AddYGap();

    m_LenUnitChoice.AddItem( "MM" );
    m_LenUnitChoice.AddItem( "CM" );
    m_LenUnitChoice.AddItem( "M" );
    m_LenUnitChoice.AddItem( "IN" );
    m_LenUnitChoice.AddItem( "FT" );
    m_GenLayout.AddChoice( m_LenUnitChoice, "Length Unit" );
    m_GenLayout.AddYGap();

    m_GenLayout.AddButton( m_SplitSurfsToggle, "Split Surfaces" );
    m_GenLayout.AddYGap();
    m_GenLayout.AddButton( m_ToCubicToggle, "Convert to Cubic" );
    m_GenLayout.AddSlider( m_ToCubicTolSlider, "Tolerance", 10, "%5.4g", true );

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

IGESOptionsScreen::~IGESOptionsScreen()
{
}

bool IGESOptionsScreen::Update()
{
    Vehicle *veh = VehicleMgr.GetVehicle();

    if( veh )
    {
        m_LenUnitChoice.Update( veh->m_IGESLenUnit.GetID() );
        m_SplitSurfsToggle.Update( veh->m_IGESSplitSurfs.GetID() );
        m_ToCubicToggle.Update( veh->m_IGESToCubic.GetID() );
        m_ToCubicTolSlider.Update( veh->m_IGESToCubicTol.GetID() );

        if ( !veh->m_IGESToCubic() )
        {
            m_ToCubicTolSlider.Deactivate();
        }
    }

    m_FLTK_Window->redraw();

    return false;
}


void IGESOptionsScreen::Show()
{
    Update();
    m_FLTK_Window->show();
}

void IGESOptionsScreen::CallBack( Fl_Widget* w )
{
    m_ScreenMgr->SetUpdateFlag( true );
}

void IGESOptionsScreen::GuiDeviceCallBack( GuiDevice* device )
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
            veh->m_IGESLenUnit.Set( m_PrevUnit );
            veh->m_IGESSplitSurfs.Set( m_PrevSplit );
            veh->m_IGESToCubic.Set( m_PrevCubic );
            veh->m_IGESToCubicTol.Set( m_PrevToCubicTol );
        }
        Hide();
    }

    m_ScreenMgr->SetUpdateFlag( true );
}

bool IGESOptionsScreen::ShowIGESOptionsScreen()
{
    Show();

    m_OkFlag = false;

    Vehicle *veh = VehicleMgr.GetVehicle();

    if( veh )
    {
        m_PrevUnit = veh->m_IGESLenUnit();
        m_PrevSplit = veh->m_IGESSplitSurfs();
        m_PrevCubic = veh->m_IGESToCubic();
        m_PrevToCubicTol = veh->m_IGESToCubicTol();
    }

    while( m_FLTK_Window->shown() )
    {
        Fl::wait();
    }

    return m_OkFlag;
}

void IGESOptionsScreen::CloseCallBack( Fl_Widget *w )
{
    assert( m_ScreenMgr );

    Vehicle *veh = VehicleMgr.GetVehicle();

    if( veh )
    {
        veh->m_IGESLenUnit.Set( m_PrevUnit );
        veh->m_IGESSplitSurfs.Set( m_PrevSplit );
        veh->m_IGESToCubic.Set( m_PrevCubic );
        veh->m_IGESToCubicTol.Set( m_PrevToCubicTol );
    }

    Hide();
}
