//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//

// XRotorOptionsScreen.cpp: implementation of the XRotorOptionsScreen class.
//
//////////////////////////////////////////////////////////////////////

#include "XRotorOptionsScreen.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

XRotorOptionsScreen::XRotorOptionsScreen( ScreenMgr* mgr ) : BasicScreen( mgr, 300, 25 + 21 * 20 + 4 * 6 + 30, "XRotor Options" )
{
    m_OkFlag = false;

    m_GenLayout.SetGroupAndScreen( m_FLTK_Window, this );
    m_GenLayout.AddY( 25 );

    m_GenLayout.AddYGap();

    m_PropGeomPicker.AddIncludeType( PROP_GEOM_TYPE );
    m_GenLayout.AddGeomPicker( m_PropGeomPicker );
    m_GenLayout.AddYGap();

    m_GenLayout.AddDividerBox( "Operating Condition" );
    m_LenUnitChoice.AddItem( "MM", vsp::LEN_MM );
    m_LenUnitChoice.AddItem( "CM", vsp::LEN_CM );
    m_LenUnitChoice.AddItem( "M", vsp::LEN_M );
    m_LenUnitChoice.AddItem( "IN", vsp::LEN_IN );
    m_LenUnitChoice.AddItem( "FT", vsp::LEN_FT );
    m_LenUnitChoice.AddItem( "YD", vsp::LEN_YD );
    m_GenLayout.AddChoice( m_LenUnitChoice, "Model Unit" );
    m_GenLayout.AddSlider( m_VinfSlider, "V (m/s)", 100, "%7.3g" );
    m_GenLayout.AddSlider( m_RPMSlider, "RPM", 1000, "%7.5g" );
    m_GenLayout.AddSlider( m_AltSlider, "Alt (m)", 1000, "%7.5g" );
    m_GenLayout.AddButton( m_FreeWakeToggle, "Self-Deforming Wake" );
    m_GenLayout.AddYGap();

    m_GenLayout.AddDividerBox( "Aero Sections" );
    m_GenLayout.AddSlider( m_NumAeroSlider, "Num", 10, "%3.0f" );
    m_GenLayout.AddSlider( m_dCLdASlider, "dCL/dA", 1, "%7.4g" );
    m_GenLayout.AddSlider( m_CLmaxSlider, "CLmax", 1, "%7.4g" );
    m_GenLayout.AddSlider( m_CLminSlider, "CLmin", 1, "%7.4g" );
    m_GenLayout.AddSlider( m_dCLdAStallSlider, "dCL/dA Stall", 1, "%7.4g" );
    m_GenLayout.AddSlider( m_dCLStallSlider, "dCL Stall", 1, "%7.4g" );
    m_GenLayout.AddSlider( m_CmSlider, "Cm", 1, "%7.4g" );
    m_GenLayout.AddSlider( m_McritSlider, "Mcrit", 1, "%7.4g" );
    m_GenLayout.AddSlider( m_CDminSlider, "CDmin", 0.1, "%7.4g" );
    m_GenLayout.AddSlider( m_CLCDminSlider, "CL at CDmin", 1, "%7.4g" );
    m_GenLayout.AddSlider( m_dCDdCL2Slider, "dCD/dCL^2", 0.1, "%7.4g" );
    m_GenLayout.AddSlider( m_ReRefSlider, "Re Ref", 1e6, "%7.4g" );
    m_GenLayout.AddSlider( m_ReExpSlider, "Re Exponent", 1, "%7.4g" );
    m_GenLayout.AddYGap();

    m_GenLayout.SetFitWidthFlag( false );
    m_GenLayout.SetSameLineFlag( true );
    m_GenLayout.SetButtonWidth( 100 );

    m_GenLayout.AddX( 45 );
    m_GenLayout.AddButton( m_OkButton, "OK" );
    m_GenLayout.AddX( 10 );
    m_GenLayout.AddButton( m_CancelButton, "Cancel" );
    m_GenLayout.ForceNewLine();
}

XRotorOptionsScreen::~XRotorOptionsScreen()
{
}

bool XRotorOptionsScreen::Update()
{
    BasicScreen::Update();

    m_PropGeomPicker.Update();

    Vehicle *veh = VehicleMgr.GetVehicle();

    if ( veh )
    {
        m_LenUnitChoice.Update( veh->m_XRotorLenUnit.GetID() );
        m_VinfSlider.Update( veh->m_XRotorVinf.GetID() );
        m_RPMSlider.Update( veh->m_XRotorRPM.GetID() );
        m_AltSlider.Update( veh->m_XRotorAlt.GetID() );
        m_FreeWakeToggle.Update( veh->m_XRotorFreeWakeFlag.GetID() );

        m_NumAeroSlider.Update( veh->m_XRotorNumAero.GetID() );
        m_dCLdASlider.Update( veh->m_XRotordCLdA.GetID() );
        m_CLmaxSlider.Update( veh->m_XRotorCLmax.GetID() );
        m_CLminSlider.Update( veh->m_XRotorCLmin.GetID() );
        m_dCLdAStallSlider.Update( veh->m_XRotordCLdAStall.GetID() );
        m_dCLStallSlider.Update( veh->m_XRotordCLStall.GetID() );
        m_CmSlider.Update( veh->m_XRotorCm.GetID() );
        m_McritSlider.Update( veh->m_XRotorMcrit.GetID() );
        m_CDminSlider.Update( veh->m_XRotorCDmin.GetID() );
        m_CLCDminSlider.Update( veh->m_XRotorCLCDmin.GetID() );
        m_dCDdCL2Slider.Update( veh->m_XRotordCDdCL2.GetID() );
        m_ReRefSlider.Update( veh->m_XRotorReRef.GetID() );
        m_ReExpSlider.Update( veh->m_XRotorReExp.GetID() );
    }

    m_FLTK_Window->redraw();

    return false;
}

void XRotorOptionsScreen::Show()
{
    m_ScreenMgr->SetUpdateFlag( true );
    BasicScreen::Show();
}

void XRotorOptionsScreen::CallBack( Fl_Widget* w )
{
    m_ScreenMgr->SetUpdateFlag( true );
}

void XRotorOptionsScreen::GuiDeviceCallBack( GuiDevice* device )
{
    assert( m_ScreenMgr );

    if ( device == &m_OkButton )
    {
        m_OkFlag = true;
        Vehicle *veh = VehicleMgr.GetVehicle();

        if ( veh )
        {
            veh->m_BEMPropID = m_PropGeomPicker.GetGeomChoice();
        }
        Hide();
    }
    else if ( device == &m_CancelButton )
    {
        Hide();
    }

    m_ScreenMgr->SetUpdateFlag( true );
}

bool XRotorOptionsScreen::ShowXRotorOptionsScreen()
{
    Show();

    m_OkFlag = false;

    while ( m_FLTK_Window->shown() )
    {
        Fl::wait();
    }

    return m_OkFlag;
}
