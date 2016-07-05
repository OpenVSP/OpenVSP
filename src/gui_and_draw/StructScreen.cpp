//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//

// StructScreen.cpp: implementation of the StructScreen class.
//
//////////////////////////////////////////////////////////////////////

#include "StructScreen.h"
#include "FeaMeshMgr.h"
#include "APIDefines.h"
using namespace vsp;

//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

StructScreen::StructScreen( ScreenMgr* mgr ) : TabScreen( mgr, 415, 622, "FEA Mesh", 150 )
{
    m_FLTK_Window->callback( staticCloseCB, this );

    Fl_Group* globalTab = AddTab( "Global" );
    Fl_Group* globalTabGroup = AddSubGroup( globalTab, 5 );

    m_GlobalTabLayout.SetGroupAndScreen( globalTabGroup, this );

    //=== GLOBAL TAB INIT ===//

    m_GlobalTabLayout.AddDividerBox("Global Mesh Control");

    m_GlobalTabLayout.AddYGap();

    m_GlobalTabLayout.SetButtonWidth(175.0);
    m_GlobalTabLayout.AddSlider(m_MaxEdgeLen, "Max Edge Len", 1.0, "%7.5f");
    m_GlobalTabLayout.AddSlider(m_MinEdgeLen, "Min Edge Len", 1.0, "%7.5f");
    m_GlobalTabLayout.AddSlider(m_MaxGap, "Max Gap", 1.0, "%7.5f");
    m_GlobalTabLayout.AddSlider(m_NumCircleSegments, "Num Circle Segments", 100.0, "%7.5f");
    m_GlobalTabLayout.AddSlider(m_GrowthRatio, "Growth Ratio", 2.0, "%7.5f");

    m_GlobalTabLayout.AddYGap();
    m_GlobalTabLayout.AddButton(m_Rig3dGrowthLimit, "Rigorous 3D Growth Limiting");
    m_GlobalTabLayout.AddYGap();
    m_GlobalTabLayout.AddDividerBox("Global Source Control");
    m_GlobalTabLayout.AddYGap();

    m_GlobalTabLayout.SetFitWidthFlag( false );
    m_GlobalTabLayout.SetSameLineFlag( true );

    m_GlobalTabLayout.SetButtonWidth(20.0);
    m_GlobalTabLayout.AddButton(m_GlobSrcAdjustLenLftLft, "<<");
    m_GlobalTabLayout.AddButton(m_GlobSrcAdjustLenLft, "<");
    m_GlobalTabLayout.AddLabel("Adjust Len", 100.0);
    m_GlobalTabLayout.AddButton(m_GlobSrcAdjustLenRht, ">");
    m_GlobalTabLayout.AddButton(m_GlobSrcAdjustLenRhtRht, ">>");
    m_GlobalTabLayout.AddX(45.0);
    m_GlobalTabLayout.AddButton(m_GlobSrcAdjustRadLftLft, "<<");
    m_GlobalTabLayout.AddButton(m_GlobSrcAdjustRadLft, "<");
    m_GlobalTabLayout.AddLabel("Adjust Rad", 100.0);
    m_GlobalTabLayout.AddButton(m_GlobSrcAdjustRadRht, ">");
    m_GlobalTabLayout.AddButton(m_GlobSrcAdjustRadRhtRht, ">>");
    m_GlobalTabLayout.ForceNewLine();
    m_GlobalTabLayout.AddYGap();

    m_GlobalTabLayout.SetFitWidthFlag( true );
    m_GlobalTabLayout.SetSameLineFlag( false );

    m_GlobalTabLayout.AddDividerBox("Geometry Control");
    m_GlobalTabLayout.AddYGap();
    m_GlobalTabLayout.AddButton(m_IntersectSubsurfaces, "Intersect Subsurfaces");
    m_GlobalTabLayout.AddYGap();

    m_GlobalTabLayout.SetChoiceButtonWidth(m_GlobalTabLayout.GetRemainX() / 2.0);
    m_GlobalTabLayout.AddChoice(m_UseSet, "Use Set");

    globalTab->show();
}

StructScreen::~StructScreen()
{
}

void StructScreen::CloseCallBack( Fl_Widget *w )
{
    Hide();
/*
    FeaMeshMgr.SetDrawMeshFlag( false );
    FeaMeshMgr.SetDrawFlag( false );

    FeaMeshMgr.SaveData();

    vector< string > geomVec = m_Vehicle->GetGeomVec();
    for ( int i = 0 ; i < ( int )geomVec.size() ; i++ )
    {
        Geom *g = m_Vehicle->FindGeom( geomVec[i] );
        if ( g )
        {
            g->SetSetFlag( SET_SHOWN, false );
            g->SetSetFlag( SET_NOT_SHOWN, true );
        }
    }

    m_FLTK_Window->hide();
    m_ScreenMgr->SetUpdateFlag( true );
*/
}

void StructScreen::Show()
{
    FeaMeshMgr.SetDrawFlag( true );

    FeaMeshMgr.LoadSurfaces();
    Update();

    m_FLTK_Window->show();

    Vehicle*  veh = m_ScreenMgr->GetVehiclePtr();


    vector< string > geomVec = veh->GetGeomVec();
    for ( int i = 0 ; i < ( int )geomVec.size() ; i++ )
    {
        Geom *g = veh->FindGeom( geomVec[i] );
        if ( g )
        {
            g->SetSetFlag( SET_SHOWN, true );
            g->SetSetFlag( SET_NOT_SHOWN, false );
        }
    }
    m_ScreenMgr->SetUpdateFlag( true );

}

bool StructScreen::Update()
{
    char str[256];

    //==== Default Elem Size ====//
    m_MaxEdgeLen.Update( FeaMeshMgr.GetGridDensityPtr()->m_BaseLen.GetID() );
    m_MinEdgeLen.Update( FeaMeshMgr.GetGridDensityPtr()->m_MinLen.GetID() );
    m_MaxGap.Update( FeaMeshMgr.GetGridDensityPtr()->m_MaxGap.GetID() );
    m_NumCircleSegments.Update( FeaMeshMgr.GetGridDensityPtr()->m_NCircSeg.GetID() );
    m_GrowthRatio.Update( FeaMeshMgr.GetGridDensityPtr()->m_GrowRatio.GetID() );
    m_ThickScale.Update( FeaMeshMgr.m_ThickScale.GetID() );


    return true;
}

string StructScreen::truncateFileName( const string &fn, int len )
{
    string trunc( fn );
    if ( (int)trunc.length() > len )
    {
        trunc.erase( 0, trunc.length() - len );
        trunc.replace( 0, 3, "..." );
    }
    return trunc;
}

void StructScreen::AddOutputText( const string &text )
{
    m_ConsoleBuffer->append( text.c_str() );
    m_ConsoleDisplay->move_down();
    m_ConsoleDisplay->show_insert_position();
    Fl::flush();
}

void StructScreen::CallBack( Fl_Widget* w )
{


}

void StructScreen::LoadDrawObjs( vector< DrawObj* > &draw_obj_vec )
{
    if ( IsShown() )
    {
        FeaMeshMgr.LoadDrawObjs( draw_obj_vec );
    }
}
