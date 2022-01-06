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

STEPOptionsScreen::STEPOptionsScreen( ScreenMgr* mgr ) : BasicScreen( mgr, 250, 370, "Untrimmed STEP Options" )
{
    m_FLTK_Window->callback( staticCloseCB, this );

    m_OkFlag = false;
    m_PrevUnit = 0;
    m_PrevTol = 1e-6;
    m_PrevSplit = false;
    m_PrevSplitSub = false;
    //m_PrevMerge = true;
    m_PrevCubic = false;
    m_PrevToCubicTol = 1e-6;
    m_PrevPropExportOrigin = false;
    m_PrevLabelID = true;
    m_PrevLabelName = true;
    m_PrevLabelSurfNo = true;
    m_PrevLabelDelim = vsp::DELIM_COMMA;

    m_GenLayout.SetGroupAndScreen( m_FLTK_Window, this );
    m_GenLayout.AddY( 25 );

    m_GenLayout.AddYGap();

    m_LenUnitChoice.AddItem( "MM" );
    m_LenUnitChoice.AddItem( "CM" );
    m_LenUnitChoice.AddItem( "M" );
    m_LenUnitChoice.AddItem( "IN" );
    m_LenUnitChoice.AddItem( "FT" );
    m_LenUnitChoice.AddItem( "YD" );
    m_GenLayout.AddChoice( m_LenUnitChoice, "Length Unit" );
    m_GenLayout.AddSlider( m_TolSlider, "Tolerance", 10, "%5.4g", 0, true );
    m_GenLayout.AddYGap();

    m_GenLayout.AddButton( m_SplitSurfsToggle, "Split Surfaces" );
    m_GenLayout.AddYGap();
    m_GenLayout.AddButton( m_SplitSubSurfsToggle, "Split U/W-Const Sub-Surfaces" );
    m_GenLayout.AddYGap();
    m_GenLayout.AddButton( m_TrimTEToggle, "Omit TE Surfaces" );
    ////m_GenLayout.AddYGap();
    //m_GenLayout.AddButton( m_MergePointsToggle, "Merge Points" );
    m_GenLayout.AddYGap();
    m_GenLayout.AddButton( m_ToCubicToggle, "Demote Surfs to Cubic" );
    m_GenLayout.AddSlider( m_ToCubicTolSlider, "Tolerance", 10, "%5.4g", 0, true );
    m_GenLayout.AddYGap();
    m_GenLayout.AddButton( m_PropExportOriginToggle, "Export Props At Origin" );

    m_GenLayout.AddYGap();

    m_GenLayout.AddDividerBox( "Surface Name" );
    m_GenLayout.AddButton( m_LabelIDToggle, "Geom ID" );
    m_GenLayout.AddButton( m_LabelNameToggle, "Geom Name" );
    m_GenLayout.AddButton( m_LabelSurfNoToggle, "Surface Number" );

    m_LabelDelimChoice.AddItem( "Comma" );
    m_LabelDelimChoice.AddItem( "Underscore" );
    m_LabelDelimChoice.AddItem( "Space" );
    m_LabelDelimChoice.AddItem( "None" );
    m_GenLayout.AddChoice( m_LabelDelimChoice, "Delimiter" );

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
        m_LenUnitChoice.Update( veh->m_STEPLenUnit.GetID() );
        m_TolSlider.Update( veh->m_STEPTol.GetID() );
        m_SplitSurfsToggle.Update( veh->m_STEPSplitSurfs.GetID() );
        m_SplitSubSurfsToggle.Update( veh->m_STEPSplitSubSurfs.GetID() );
        //m_MergePointsToggle.Update( veh->m_STEPMergePoints.GetID() );
        m_ToCubicToggle.Update( veh->m_STEPToCubic.GetID() );
        m_ToCubicTolSlider.Update( veh->m_STEPToCubicTol.GetID() );
        m_TrimTEToggle.Update( veh->m_STEPTrimTE.GetID() );
        m_PropExportOriginToggle.Update( veh->m_STEPExportPropMainSurf.GetID() );

        m_LabelIDToggle.Update( veh->m_STEPLabelID.GetID() );
        m_LabelNameToggle.Update( veh->m_STEPLabelName.GetID() );
        m_LabelSurfNoToggle.Update( veh->m_STEPLabelSurfNo.GetID() );
        m_LabelDelimChoice.Update( veh->m_STEPLabelDelim.GetID() );

        if ( !veh->m_STEPToCubic() )
        {
            m_ToCubicTolSlider.Deactivate();
        }
    }

    m_FLTK_Window->redraw();

    return false;
}


void STEPOptionsScreen::Show()
{
    m_ScreenMgr->SetUpdateFlag( true );
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
            veh->m_STEPLenUnit.Set( m_PrevUnit );
            veh->m_STEPTol.Set( m_PrevTol );
            veh->m_STEPSplitSurfs.Set( m_PrevSplit );
            veh->m_STEPSplitSubSurfs.Set( m_PrevSplitSub );
            //veh->m_STEPMergePoints.Set( m_PrevMerge );
            veh->m_STEPToCubic.Set( m_PrevCubic );
            veh->m_STEPToCubicTol.Set( m_PrevToCubicTol );
            veh->m_STEPTrimTE.Set( m_PrevTrimTE );
            veh->m_STEPExportPropMainSurf.Set( m_PrevPropExportOrigin );

            veh->m_STEPLabelID.Set( m_PrevLabelID );
            veh->m_STEPLabelName.Set( m_PrevLabelName );
            veh->m_STEPLabelSurfNo.Set( m_PrevLabelSurfNo );
            veh->m_STEPLabelDelim.Set( m_PrevLabelDelim );

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
        m_PrevUnit = veh->m_STEPLenUnit();
        m_PrevTol = veh->m_STEPTol();
        m_PrevSplit = veh->m_STEPSplitSurfs();
        m_PrevSplitSub = veh->m_STEPSplitSubSurfs();
        //m_PrevMerge = veh->m_STEPMergePoints();
        m_PrevCubic = veh->m_STEPToCubic();
        m_PrevToCubicTol = veh->m_STEPToCubicTol();
        m_PrevTrimTE = veh->m_STEPTrimTE();
        m_PrevPropExportOrigin = veh->m_STEPExportPropMainSurf();

        m_PrevLabelID = veh->m_STEPLabelID();
        m_PrevLabelName = veh->m_STEPLabelName();
        m_PrevLabelSurfNo = veh->m_STEPLabelSurfNo();
        m_PrevLabelDelim = veh->m_STEPLabelDelim();
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
        veh->m_STEPLenUnit.Set( m_PrevUnit );
        veh->m_STEPTol.Set( m_PrevTol );
        veh->m_STEPSplitSurfs.Set( m_PrevSplit );
        veh->m_STEPSplitSubSurfs.Set( m_PrevSplitSub );
        //veh->m_STEPMergePoints.Set( m_PrevMerge );
        veh->m_STEPToCubic.Set( m_PrevCubic );
        veh->m_STEPToCubicTol.Set( m_PrevToCubicTol );
        veh->m_STEPTrimTE.Set( m_PrevTrimTE );
        veh->m_STEPExportPropMainSurf.Set( m_PrevPropExportOrigin );

        veh->m_STEPLabelID.Set( m_PrevLabelID );
        veh->m_STEPLabelName.Set( m_PrevLabelName );
        veh->m_STEPLabelSurfNo.Set( m_PrevLabelSurfNo );
        veh->m_STEPLabelDelim.Set( m_PrevLabelDelim );
    }

    Hide();
}
