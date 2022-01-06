//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//

// IGESStructureOptionsScreen.cpp: implementation of the IGESOptionsScreen class.
//
//////////////////////////////////////////////////////////////////////

#include "IGESStructureOptionsScreen.h"
#include "StructureMgr.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

IGESStructureOptionsScreen::IGESStructureOptionsScreen( ScreenMgr* mgr ) : BasicScreen( mgr, 250, 310, "Untrimmed IGES Struct Options" )
{
    m_FLTK_Window->callback( staticCloseCB, this );

    m_OkFlag = false;
    m_PrevStructureChoice = 0;
    m_PrevSplit = false;
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

    m_GenLayout.AddChoice( m_StructureChoice, "Structure" );
    m_GenLayout.AddYGap();

    m_GenLayout.AddDividerBox( "Options" );
    m_GenLayout.AddButton( m_SplitSurfsToggle, "Split Surfaces" );
    m_GenLayout.AddYGap();
    m_GenLayout.AddButton( m_ToCubicToggle, "Demote Surfs to Cubic" );
    m_GenLayout.AddSlider( m_ToCubicTolSlider, "Tolerance", 10, "%5.4g", 0, true );
    m_GenLayout.AddYGap();

    m_GenLayout.AddDividerBox( "Surface Name" );
    m_GenLayout.AddButton( m_LabelIDToggle, "Geom ID" );
    m_GenLayout.AddButton( m_LabelNameToggle, "Geom Name" );
    m_GenLayout.AddButton( m_LabelSurfNoToggle, "Surface Number" );
    m_GenLayout.AddButton( m_LabelSplitNoToggle, "Split Number" );

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

IGESStructureOptionsScreen::~IGESStructureOptionsScreen()
{
}

bool IGESStructureOptionsScreen::Update()
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
        m_StructureChoice.SetVal( veh->m_IGESStructureExportIndex() );

        m_SplitSurfsToggle.Update( veh->m_IGESStructureSplitSurfs.GetID() );
        m_ToCubicToggle.Update( veh->m_IGESStructureToCubic.GetID() );
        m_ToCubicTolSlider.Update( veh->m_IGESStructureToCubicTol.GetID() );

        m_LabelIDToggle.Update( veh->m_IGESStructureLabelID.GetID() );
        m_LabelNameToggle.Update( veh->m_IGESStructureLabelName.GetID() );
        m_LabelSurfNoToggle.Update( veh->m_IGESStructureLabelSurfNo.GetID() );
        m_LabelSplitNoToggle.Update( veh->m_IGESStructureLabelSplitNo.GetID() );
        m_LabelDelimChoice.Update( veh->m_IGESStructureLabelDelim.GetID() );

        if ( !veh->m_IGESStructureToCubic() )
        {
            m_ToCubicTolSlider.Deactivate();
        }
    }

    m_FLTK_Window->redraw();

    return false;
}

void IGESStructureOptionsScreen::Show()
{
    m_ScreenMgr->SetUpdateFlag( true );
    m_FLTK_Window->show();
}

void IGESStructureOptionsScreen::CallBack( Fl_Widget* w )
{
    m_ScreenMgr->SetUpdateFlag( true );
}

void IGESStructureOptionsScreen::GuiDeviceCallBack( GuiDevice* device )
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
            veh->m_IGESStructureExportIndex.Set( m_PrevStructureChoice );
            veh->m_IGESStructureSplitSurfs.Set( m_PrevSplit );
            veh->m_IGESStructureToCubic.Set( m_PrevCubic );
            veh->m_IGESStructureToCubicTol.Set( m_PrevToCubicTol );

            veh->m_IGESStructureLabelID.Set( m_PrevLabelID );
            veh->m_IGESStructureLabelName.Set( m_PrevLabelName );
            veh->m_IGESStructureLabelSurfNo.Set( m_PrevLabelSurfNo );
            veh->m_IGESStructureLabelSplitNo.Set( m_PrevLabelSplitNo );
            veh->m_IGESStructureLabelDelim.Set( m_PrevLabelDelim );

        }
        Hide();
    }
    else if ( device == &m_StructureChoice )
    {
        Vehicle *veh = VehicleMgr.GetVehicle();

        if( veh )
        {
            veh->m_IGESStructureExportIndex.Set( m_StructureChoice.GetVal() );
        }
    }

    m_ScreenMgr->SetUpdateFlag( true );
}

bool IGESStructureOptionsScreen::ShowIGESOptionsScreen()
{
    Show();

    m_OkFlag = false;

    Vehicle *veh = VehicleMgr.GetVehicle();

    if( veh )
    {
        m_PrevStructureChoice = veh->m_IGESStructureExportIndex();
        m_PrevSplit = veh->m_IGESStructureSplitSurfs();
        m_PrevCubic = veh->m_IGESStructureToCubic();
        m_PrevToCubicTol = veh->m_IGESStructureToCubicTol();

        m_PrevLabelID = veh->m_IGESStructureLabelID();
        m_PrevLabelName = veh->m_IGESStructureLabelName();
        m_PrevLabelSurfNo = veh->m_IGESStructureLabelSurfNo();
        m_PrevLabelSplitNo = veh->m_IGESStructureLabelSplitNo();
        m_PrevLabelDelim = veh->m_IGESStructureLabelDelim();
    }

    while( m_FLTK_Window->shown() )
    {
        Fl::wait();
    }

    return m_OkFlag;
}

void IGESStructureOptionsScreen::CloseCallBack( Fl_Widget *w )
{
    assert( m_ScreenMgr );

    Vehicle *veh = VehicleMgr.GetVehicle();

    if( veh )
    {
        veh->m_IGESStructureExportIndex.Set( m_PrevStructureChoice );
        veh->m_IGESStructureSplitSurfs.Set( m_PrevSplit );
        veh->m_IGESStructureToCubic.Set( m_PrevCubic );
        veh->m_IGESStructureToCubicTol.Set( m_PrevToCubicTol );

        veh->m_IGESStructureLabelID.Set( m_PrevLabelID );
        veh->m_IGESStructureLabelName.Set( m_PrevLabelName );
        veh->m_IGESStructureLabelSurfNo.Set( m_PrevLabelSurfNo );
        veh->m_IGESStructureLabelSplitNo.Set( m_PrevLabelSplitNo );
        veh->m_IGESStructureLabelDelim.Set( m_PrevLabelDelim );
    }

    Hide();
}
