//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//

// STEPStructureOptionsScreen.cpp: implementation of the STEPOptionsScreen class.
//
//////////////////////////////////////////////////////////////////////

#include "STEPStructureOptionsScreen.h"
#include "StructureMgr.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

STEPStructureOptionsScreen::STEPStructureOptionsScreen( ScreenMgr* mgr ) : BasicScreen( mgr, 250, 290, "STEP Structure Options" )
{
    m_FLTK_Window->callback( staticCloseCB, this );

    m_OkFlag = false;
    m_PrevStructureChoice = 0;
    m_PrevTol = 1e-6;
    m_PrevSplit = false;
    m_PrevMerge = true;
    m_PrevCubic = false;
    m_PrevToCubicTol = 1e-6;

    m_GenLayout.SetGroupAndScreen( m_FLTK_Window, this );
    m_GenLayout.AddY( 25 );

    m_GenLayout.AddYGap();

    m_GenLayout.AddChoice( m_StructureChoice, "Structure" );
    m_GenLayout.AddYGap();

    m_GenLayout.AddDividerBox( "Options" );
    m_GenLayout.AddSlider( m_TolSlider, "Tolerance", 10, "%5.4g", 0, true );
    m_GenLayout.AddYGap();
    m_GenLayout.AddButton( m_SplitSurfsToggle, "Split Surfaces" );
    m_GenLayout.AddYGap();
    m_GenLayout.AddButton( m_MergePointsToggle, "Merge Points" );
    m_GenLayout.AddYGap();
    m_GenLayout.AddButton( m_ToCubicToggle, "Convert to Cubic" );
    m_GenLayout.AddSlider( m_ToCubicTolSlider, "Tolerance", 10, "%5.4g", 0, true );
    m_GenLayout.AddYGap();

    m_GenLayout.SetFitWidthFlag( false );
    m_GenLayout.SetSameLineFlag( true );
    m_GenLayout.SetButtonWidth( 100 );

    m_GenLayout.AddX( 20 );
    m_GenLayout.AddButton( m_OkButton, "OK" );
    m_GenLayout.AddX( 10 );
    m_GenLayout.AddButton( m_CancelButton, "Cancel" );
    m_GenLayout.ForceNewLine();
}

STEPStructureOptionsScreen::~STEPStructureOptionsScreen()
{
}

bool STEPStructureOptionsScreen::Update()
{
    Vehicle *veh = VehicleMgr.GetVehicle();

    if( veh )
    {
        m_StructureChoice.ClearItems();
        vector < FeaStructure * > fea_struct_vec = StructureMgr.GetAllFeaStructs();
        for ( int i = 0; i < fea_struct_vec.size(); i++ )
        {
            m_StructureChoice.AddItem( fea_struct_vec[i]->GetName() );
        }
        m_StructureChoice.UpdateItems();
        m_StructureChoice.SetVal( veh->m_STEPStructureExportIndex() );

        m_TolSlider.Update( veh->m_STEPStructureTol.GetID() );
        m_SplitSurfsToggle.Update( veh->m_STEPStructureSplitSurfs.GetID() );
        m_MergePointsToggle.Update( veh->m_STEPStructureMergePoints.GetID() );
        m_ToCubicToggle.Update( veh->m_STEPStructureToCubic.GetID() );
        m_ToCubicTolSlider.Update( veh->m_STEPStructureToCubicTol.GetID() );

        if ( !veh->m_STEPToCubic() )
        {
            m_ToCubicTolSlider.Deactivate();
        }
    }

    m_FLTK_Window->redraw();

    return false;
}

void STEPStructureOptionsScreen::Show()
{
    Update();
    m_FLTK_Window->show();
}

void STEPStructureOptionsScreen::CallBack( Fl_Widget* w )
{
    m_ScreenMgr->SetUpdateFlag( true );
}

void STEPStructureOptionsScreen::GuiDeviceCallBack( GuiDevice* device )
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
            veh->m_STEPStructureExportIndex.Set( m_PrevStructureChoice );
            veh->m_STEPStructureTol.Set( m_PrevTol );
            veh->m_STEPStructureSplitSurfs.Set( m_PrevSplit );
            veh->m_STEPStructureMergePoints.Set( m_PrevMerge );
            veh->m_STEPStructureToCubic.Set( m_PrevCubic );
            veh->m_STEPStructureToCubicTol.Set( m_PrevToCubicTol );

        }
        Hide();
    }
    else if ( device == &m_StructureChoice )
    {
        Vehicle *veh = VehicleMgr.GetVehicle();

        if( veh )
        {
            veh->m_STEPStructureExportIndex.Set( m_StructureChoice.GetVal() );
        }
    }

    m_ScreenMgr->SetUpdateFlag( true );
}

bool STEPStructureOptionsScreen::ShowSTEPOptionsScreen()
{
    Show();

    m_OkFlag = false;

    Vehicle *veh = VehicleMgr.GetVehicle();

    if( veh )
    {
        m_PrevStructureChoice = veh->m_STEPStructureExportIndex();
        m_PrevTol = veh->m_STEPStructureTol();
        m_PrevSplit = veh->m_STEPStructureSplitSurfs();
        m_PrevMerge = veh->m_STEPStructureMergePoints();
        m_PrevCubic = veh->m_STEPStructureToCubic();
        m_PrevToCubicTol = veh->m_STEPStructureToCubicTol();
    }

    while( m_FLTK_Window->shown() )
    {
        Fl::wait();
    }

    return m_OkFlag;
}

void STEPStructureOptionsScreen::CloseCallBack( Fl_Widget *w )
{
    assert( m_ScreenMgr );

    Vehicle *veh = VehicleMgr.GetVehicle();

    if( veh )
    {
        veh->m_STEPStructureExportIndex.Set( m_PrevStructureChoice );
        veh->m_STEPStructureTol.Set( m_PrevTol );
        veh->m_STEPStructureSplitSurfs.Set( m_PrevSplit );
        veh->m_STEPStructureMergePoints.Set( m_PrevMerge );
        veh->m_STEPStructureToCubic.Set( m_PrevCubic );
        veh->m_STEPStructureToCubicTol.Set( m_PrevToCubicTol );
    }

    Hide();
}
