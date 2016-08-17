#include "MultTransScreen.h"
#include "ScreenMgr.h"
#include "LightMgr.h"
#include "Vehicle.h"

#include <assert.h>

MultTransScreen::MultTransScreen( ScreenMgr * mgr ) : BasicScreen( mgr, 250, 225, "Multiple Transformation" )
{
    m_MainLayout.SetGroupAndScreen( m_FLTK_Window, this );
    m_MainLayout.AddX( 5 );
    m_MainLayout.AddY( 25 );
    m_MainLayout.AddSubGroupLayout( m_GenLayout, m_MainLayout.GetRemainX() - 5, m_MainLayout.GetRemainY() );

    m_GenLayout.AddSlider( m_XLoc, "XLoc", 10.0, "%7.3f" );
    m_GenLayout.AddSlider( m_YLoc, "YLoc", 10.0, "%7.3f" );
    m_GenLayout.AddSlider( m_ZLoc, "ZLoc", 10.0, "%7.3f" );
    m_GenLayout.AddYGap();

    m_GenLayout.AddSlider( m_XRot, "XRot", 10.0, "%7.3f" );
    m_GenLayout.AddSlider( m_YRot, "YRot", 10.0, "%7.3f" );
    m_GenLayout.AddSlider( m_ZRot, "ZRot", 10.0, "%7.3f" );
    m_GenLayout.AddYGap();

    m_GenLayout.AddButton( m_ApplyScaleToTranslations, "Scale Translations" );
    m_GenLayout.AddSlider(m_Scale, "Scale", 1, " %5.4f" );

    m_GenLayout.AddYGap();

    m_GenLayout.SetButtonWidth( m_GenLayout.GetRemainX() / 2.0 );
    m_GenLayout.SetFitWidthFlag( false );
    m_GenLayout.SetSameLineFlag( true );
    m_GenLayout.AddButton( m_AcceptButton, "Accept" );
    m_GenLayout.AddButton( m_ResetButton, "Reset" );
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
    veh->GetGroupTransformationsPtr()->ReInitialize();

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

    // Get the group transformations class
    GroupTransformations* group_trans = veh->GetGroupTransformationsPtr();
    m_XLoc.Update( group_trans->m_GroupXLoc.GetID());
    m_YLoc.Update( group_trans->m_GroupYLoc.GetID());
    m_ZLoc.Update( group_trans->m_GroupZLoc.GetID());
    m_XRot.Update( group_trans->m_GroupXRot.GetID() );
    m_YRot.Update( group_trans->m_GroupYRot.GetID() );
    m_ZRot.Update( group_trans->m_GroupZRot.GetID() );
    m_Scale.Update( group_trans->m_GroupScale.GetID());
    m_ApplyScaleToTranslations.Update( group_trans->m_scaleGroupTranslations.GetID());

    return true;
}

void MultTransScreen::GuiDeviceCallBack( GuiDevice* device )
{
    assert( m_ScreenMgr );
    Vehicle* veh = m_ScreenMgr->GetVehiclePtr();
    assert( veh );
    GroupTransformations* trans = veh->GetGroupTransformationsPtr();
    assert( trans );

    // If the device is the accept button,
    // then accept the values
    if ( device == &m_AcceptButton )
    {
        trans->Accept();
    }
    else if ( device == &m_ResetButton )
    {
        trans->Reset();
    }

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
