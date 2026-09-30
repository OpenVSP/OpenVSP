//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//

// SplitStitchOptionsScreen.cpp: implementation of the SplitStitchOptionsScreen class.
//
//////////////////////////////////////////////////////////////////////

#include "SplitStitchOptionsScreen.h"

SplitStitchOptionsScreen::SplitStitchOptionsScreen( ScreenMgr* mgr ) : BasicScreen( mgr, 300, 400, "Split and Stitch Options" )
{
    m_FLTK_Window->callback( staticCloseCB, this );

    m_STEPFlag = true;
    m_OkFlag = false;

    m_PrevUnit = vsp::LEN_FT;
    m_PrevTol = 1e-6;
    m_PrevRep = vsp::STEP_BREP;
    m_PrevCubic = false;
    m_PrevToCubicTol = 1e-6;
    m_PrevLabelID = true;
    m_PrevLabelName = true;
    m_PrevLabelSurfNo = true;
    m_PrevLabelSplitNo = true;
    m_PrevLabelDelim = vsp::DELIM_COMMA;

    m_GenLayout.SetGroupAndScreen( m_FLTK_Window, this );
    m_GenLayout.AddY( 25 );
    m_GenLayout.AddYGap();

    m_LenUnitChoice.AddItem( "MM" );
    m_LenUnitChoice.AddItem( "CM" );
    m_LenUnitChoice.AddItem( "M" );
    m_LenUnitChoice.AddItem( "IN" );
    m_LenUnitChoice.AddItem( "FT" );
    m_GenLayout.AddChoice( m_LenUnitChoice, "Length Unit" );
    m_GenLayout.AddSlider( m_TolSlider, "Tolerance", 10, "%5.4g", 0, true );
    m_GenLayout.AddYGap();

    m_GenLayout.SetFitWidthFlag( false );
    m_GenLayout.SetSameLineFlag( true );
    m_GenLayout.SetButtonWidth( m_GenLayout.GetRemainX() / 2 );
    m_GenLayout.AddButton( m_STEPShell, "Shell" );
    m_GenLayout.AddButton( m_STEPBREP, "BREP Solid" );
    m_GenLayout.ForceNewLine();
    m_GenLayout.SetFitWidthFlag( true );
    m_GenLayout.SetSameLineFlag( false );

    m_STEPRepGroup.Init( this );
    m_STEPRepGroup.AddButton( m_STEPShell.GetFlButton() );
    m_STEPRepGroup.AddButton( m_STEPBREP.GetFlButton() );
    m_GenLayout.AddYGap();

    m_GenLayout.SetButtonWidth( 100 );
    m_GenLayout.AddButton( m_ToCubicToggle, "Demote Surfs to Cubic" );
    m_GenLayout.AddSlider( m_ToCubicTolSlider, "Tolerance", 10, "%5.4g", 0, true );
    m_GenLayout.AddYGap();

    m_GenLayout.AddDividerBox( "Surface Name" );
    m_GenLayout.AddButton( m_LabelIDToggle, "Geom ID" );
    m_GenLayout.AddButton( m_LabelNameToggle, "Geom Name" );
    m_GenLayout.AddButton( m_LabelSurfNoToggle, "Surface Number" );
    m_GenLayout.AddButton( m_LabelSplitNoToggle, "Split Number" );

    m_LabelDelimChoice.AddItem( "Comma", vsp::DELIM_COMMA );
    m_LabelDelimChoice.AddItem( "Underscore", vsp::DELIM_USCORE );
    m_LabelDelimChoice.AddItem( "Space", vsp::DELIM_SPACE );
    m_LabelDelimChoice.AddItem( "None", vsp::DELIM_NONE );
    m_GenLayout.AddChoice( m_LabelDelimChoice, "Delimiter" );

    m_GenLayout.AddY( 25 );
    m_GenLayout.SetFitWidthFlag( false );
    m_GenLayout.SetSameLineFlag( true );
    m_GenLayout.SetButtonWidth( 100 );

    m_GenLayout.AddX( 45 );
    m_GenLayout.AddButton( m_OkButton, "OK" );
    m_GenLayout.AddX( 10 );
    m_GenLayout.AddButton( m_CancelButton, "Cancel" );
    m_GenLayout.ForceNewLine();
}

SplitStitchOptionsScreen::~SplitStitchOptionsScreen()
{
}

bool SplitStitchOptionsScreen::Update()
{
    BasicScreen::Update();

    Vehicle *veh = VehicleMgr.GetVehicle();

    if ( veh )
    {
        IntersectSettings* settings = veh->GetISectSettingsPtr();

        m_LenUnitChoice.Update( settings->m_CADLenUnit.GetID() );
        m_TolSlider.Update( settings->m_STEPTol.GetID() );
        m_STEPRepGroup.Update( settings->m_STEPRepresentation.GetID() );
        m_ToCubicToggle.Update( settings->m_DemoteSurfsCubicFlag.GetID() );
        m_ToCubicTolSlider.Update( settings->m_CubicSurfTolerance.GetID() );

        m_LabelIDToggle.Update( settings->m_CADLabelID.GetID() );
        m_LabelNameToggle.Update( settings->m_CADLabelName.GetID() );
        m_LabelSurfNoToggle.Update( settings->m_CADLabelSurfNo.GetID() );
        m_LabelSplitNoToggle.Update( settings->m_CADLabelSplitNo.GetID() );
        m_LabelDelimChoice.Update( settings->m_CADLabelDelim.GetID() );

        // The representation is STEP's alone
        if ( m_STEPFlag )
        {
            m_STEPRepGroup.Activate();
        }
        else
        {
            m_STEPRepGroup.Deactivate();
        }

        if ( settings->m_DemoteSurfsCubicFlag() )
        {
            m_ToCubicTolSlider.Activate();
        }
        else
        {
            m_ToCubicTolSlider.Deactivate();
        }
    }

    m_FLTK_Window->redraw();

    return false;
}

void SplitStitchOptionsScreen::Show()
{
    m_ScreenMgr->SetUpdateFlag( true );
    BasicScreen::Show();
}

void SplitStitchOptionsScreen::CallBack( Fl_Widget* w )
{
    m_ScreenMgr->SetUpdateFlag( true );
}

void SplitStitchOptionsScreen::RestorePrev()
{
    Vehicle *veh = VehicleMgr.GetVehicle();

    if ( veh )
    {
        IntersectSettings* settings = veh->GetISectSettingsPtr();

        settings->m_CADLenUnit.Set( m_PrevUnit );
        settings->m_STEPTol.Set( m_PrevTol );
        settings->m_STEPRepresentation.Set( m_PrevRep );
        settings->m_DemoteSurfsCubicFlag.Set( m_PrevCubic );
        settings->m_CubicSurfTolerance.Set( m_PrevToCubicTol );
        settings->m_CADLabelID.Set( m_PrevLabelID );
        settings->m_CADLabelName.Set( m_PrevLabelName );
        settings->m_CADLabelSurfNo.Set( m_PrevLabelSurfNo );
        settings->m_CADLabelSplitNo.Set( m_PrevLabelSplitNo );
        settings->m_CADLabelDelim.Set( m_PrevLabelDelim );
    }
}

void SplitStitchOptionsScreen::GuiDeviceCallBack( GuiDevice* device )
{
    assert( m_ScreenMgr );

    if ( device == &m_OkButton )
    {
        m_OkFlag = true;
        Hide();
    }
    else if ( device == &m_CancelButton )
    {
        RestorePrev();
        Hide();
    }

    m_ScreenMgr->SetUpdateFlag( true );
}

bool SplitStitchOptionsScreen::ShowSplitStitchOptionsScreen( bool step_flag )
{
    m_STEPFlag = step_flag;

    if ( step_flag )
    {
        SetTitle( "Split and Stitch STEP Options" );
    }
    else
    {
        SetTitle( "Split and Stitch IGES Options" );
    }

    Show();

    m_OkFlag = false;

    Vehicle *veh = VehicleMgr.GetVehicle();

    if ( veh )
    {
        IntersectSettings* settings = veh->GetISectSettingsPtr();

        m_PrevUnit = settings->m_CADLenUnit();
        m_PrevTol = settings->m_STEPTol();
        m_PrevRep = settings->m_STEPRepresentation();
        m_PrevCubic = settings->m_DemoteSurfsCubicFlag();
        m_PrevToCubicTol = settings->m_CubicSurfTolerance();
        m_PrevLabelID = settings->m_CADLabelID();
        m_PrevLabelName = settings->m_CADLabelName();
        m_PrevLabelSurfNo = settings->m_CADLabelSurfNo();
        m_PrevLabelSplitNo = settings->m_CADLabelSplitNo();
        m_PrevLabelDelim = settings->m_CADLabelDelim();
    }

    while ( m_FLTK_Window->shown() )
    {
        Fl::wait();
    }

    return m_OkFlag;
}

void SplitStitchOptionsScreen::CloseCallBack( Fl_Widget *w )
{
    assert( m_ScreenMgr );

    RestorePrev();

    Hide();
}
