#include "MultTransScreen.h"
#include "ScreenMgr.h"
#include "LightMgr.h"
#include "../geom_core/Vehicle.h"

#include <assert.h>

MultTransScreen::MultTransScreen( ScreenMgr * mgr ) : BasicScreen( mgr, 250, 180, "Multiple Transformation" )
{
    m_GenLayout.SetGroupAndScreen( m_FLTK_Window, this );
    m_GenLayout.AddY( 25 );

    m_GenLayout.AddSlider( m_XLoc, "XLoc", 10.0, "%7.3f" );
    m_GenLayout.AddSlider( m_YLoc, "YLoc", 10.0, "%7.3f" );
    m_GenLayout.AddSlider( m_ZLoc, "ZLoc", 10.0, "%7.3f" );
    m_GenLayout.AddYGap();

    m_GenLayout.AddSlider( m_XRot, "XRot", 10.0, "%7.3f" );
    m_GenLayout.AddSlider( m_YRot, "YRot", 10.0, "%7.3f" );
    m_GenLayout.AddSlider( m_ZRot, "ZRot", 10.0, "%7.3f" );
    m_GenLayout.AddYGap();

    m_GenLayout.AddSlider(m_Scale, "Scale", 1, " %5.4f" );
}

MultTransScreen::~MultTransScreen()
{
}

void MultTransScreen::Show()
{
    assert( m_ScreenMgr );
    Vehicle* veh = m_ScreenMgr->GetVehiclePtr();
    assert( veh );

    // Reset the vehicle group transformation variables
    veh->SetActiveGeomVarVals();
    veh->ResetGroupVars();

    // Update and show if applicable
    if ( Update() )
    {
        BasicScreen::Show();
    }
    m_ScreenMgr->SetUpdateFlag( true );
}

bool MultTransScreen::Update()
{
    assert( m_ScreenMgr );
    Vehicle* veh = m_ScreenMgr->GetVehiclePtr();
    assert( veh );

    // Check that more than one Geom is active
    if ( veh->GetActiveGeomVec().size() <= 1 )
    {
        Hide();
        return false;
    }
    m_XLoc.Update(veh->m_GroupXLoc.GetID());
    m_YLoc.Update(veh->m_GroupYLoc.GetID());
    m_ZLoc.Update(veh->m_GroupZLoc.GetID());
    m_XRot.Update( veh->m_GroupXRot.GetID() );
    m_YRot.Update( veh->m_GroupYRot.GetID() );
    m_ZRot.Update( veh->m_GroupZRot.GetID() );
    m_Scale.Update(veh->m_GroupScale.GetID());

    return true;
}

void MultTransScreen::GuiDeviceCallBack( GuiDevice* device )
{
    assert( m_ScreenMgr );

//    if ( device == &m_LightChoice )
//    {
//        m_CurrentSelected = m_LightChoice.GetVal();
//    }
//    m_ScreenMgr->SetUpdateFlag( true );
}

void MultTransScreen::LoadDrawObjs(vector< DrawObj* > & draw_obj_vec)
{
//    Vehicle* veh = m_ScreenMgr->GetVehiclePtr();
//    LightMgr * lightMgr = veh->getVGuiDraw()->getLightMgr();
//
//    // Create DrawObj that provides Global Lighting Setting.
//    m_LightingDO.m_Type = DrawObj::VSP_SETTING;
//    m_LightingDO.m_Screen = DrawObj::VSP_MAIN_SCREEN;
//
//    // Clear Light info, redo list on every GetDrawObjs().
//    m_LightingDO.m_LightingInfos.clear();
//
//    std::vector< Light* > lightList = lightMgr->GetVec();
//
//    // Load Lighting Info.
//    for ( int i = 0; i < ( int )lightList.size() ; i++ )
//    {
//        DrawObj::LightSourceInfo info;
//
//        info.Active = lightList[i]->m_Active.Get();
//        info.X = ( float )lightList[i]->m_X.Get();
//        info.Y = ( float )lightList[i]->m_Y.Get();
//        info.Z = ( float )lightList[i]->m_Z.Get();
//        info.Amb = ( float )lightList[i]->m_Amb.Get();
//        info.Diff = ( float )lightList[i]->m_Diff.Get();
//        info.Spec = ( float )lightList[i]->m_Spec.Get();
//
//        m_LightingDO.m_LightingInfos.push_back( info );
//    }
//    draw_obj_vec.push_back( &m_LightingDO );
}
