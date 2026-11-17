//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//
//
//////////////////////////////////////////////////////////////////////

#include "MdcBodScreen.h"
#include "ScreenMgr.h"
#include "MdcBodGeom.h"
#include "ManageGeomScreen.h"

//==== Constructor ====//
MdcBodScreen::MdcBodScreen( ScreenMgr* mgr ) : GeomScreen( mgr, 400, 800, "MDC_BOD", "MdcBod.html" )
{
    Fl_Group* design_tab = AddTab( "Design" );
    Fl_Group* design_group = AddSubGroup( design_tab, 5 );

    m_DesignLayout.SetGroupAndScreen( design_group, this );

    // Choices' labels as wide as sliders', so they line up.
    m_DesignLayout.SetChoiceButtonWidth( m_DesignLayout.GetButtonWidth() );

    m_DesignLayout.AddButton( m_ConvertToStackButton, "Convert to Stack" );

    m_DesignLayout.AddYGap();
    m_DesignLayout.AddDividerBox( "Body" );
    // A preset shape, set on demand so a choice keeps the shape as it is.
    m_DesignLayout.SetSameLineFlag( true );
    m_DesignLayout.SetFitWidthFlag( true );
    m_DesignLayout.AddChoice( m_PresetChoice, "Preset", m_DesignLayout.GetButtonWidth() );
    m_DesignLayout.SetFitWidthFlag( false );
    m_DesignLayout.AddButton( m_SetPresetButton, "Set" );
    m_DesignLayout.ForceNewLine();
    m_DesignLayout.SetSameLineFlag( false );
    m_DesignLayout.SetFitWidthFlag( true );
    m_PresetChoice.AddItem( "Default", vsp::MDC_BOD_PRESET_DEFAULT );
    m_PresetChoice.AddItem( "Generic Rocket", vsp::MDC_BOD_PRESET_GENERIC_ROCKET );
    m_PresetChoice.UpdateItems();
    m_DesignLayout.AddChoice( m_NamelistChoice, "NAMELIST" );
    m_NamelistChoice.AddItem( "AXIBOD" );
    m_NamelistChoice.AddItem( "ELLBOD" );
    m_NamelistChoice.UpdateItems();
    m_DesignLayout.AddSlider( m_XOSlider, "XO", 10, "%7.3f" );

    m_DesignLayout.AddYGap();
    m_DesignLayout.AddDividerBox( "Nose" );
    m_DesignLayout.AddChoice( m_TNoseChoice, "TNOSE" );
    m_TNoseChoice.AddItem( "CONE" );
    m_TNoseChoice.AddItem( "OGIVE" );
    m_TNoseChoice.AddItem( "POWER" );
    m_TNoseChoice.AddItem( "HAACK" );
    m_TNoseChoice.AddItem( "KARMAN" );
    m_TNoseChoice.UpdateItems();
    m_DesignLayout.AddSlider( m_PowerSlider, "POWER", 10, "%7.3f" );
    m_DesignLayout.AddSlider( m_LNoseSlider, "LNOSE", 10, "%7.3f" );
    m_DesignLayout.AddSlider( m_DNoseSlider, "DNOSE", 10, "%7.3f" );
    m_DesignLayout.AddSlider( m_WNoseSlider, "WNOSE", 10, "%7.3f" );
    m_DesignLayout.AddSlider( m_ENoseSlider, "ENOSE", 10, "%7.3f" );
    m_DesignLayout.AddSlider( m_BNoseSlider, "BNOSE", 10, "%7.3f" );
    m_DesignLayout.AddButton( m_TruncToggle, "TRUNC" );
    m_DesignLayout.AddSlider( m_SlpNosSlider, "SLPNOS", 10, "%7.3f" );
    m_DesignLayout.AddSlider( m_ZTipSlider, "ZTIP", 10, "%7.3f" );
    m_DesignLayout.AddSlider( m_ZNoseSlider, "ZNOSE", 10, "%7.3f" );

    m_DesignLayout.AddYGap();
    m_DesignLayout.AddDividerBox( "Centerbody" );
    m_DesignLayout.AddSlider( m_LCentrSlider, "LCENTR", 10, "%7.3f" );
    m_DesignLayout.AddButton( m_CentrDefaultToggle, "Nose's size (DCENTR = DNOSE)" );
    m_DesignLayout.AddSlider( m_DCentrSlider, "DCENTR", 10, "%7.3f" );
    m_DesignLayout.AddSlider( m_WCentrSlider, "WCENTR", 10, "%7.3f" );
    m_DesignLayout.AddSlider( m_ECentrSlider, "ECENTR", 10, "%7.3f" );
    m_DesignLayout.AddSlider( m_ZCentrSlider, "ZCENTR", 10, "%7.3f" );

    m_DesignLayout.AddYGap();
    m_DesignLayout.AddDividerBox( "Aft Body" );
    m_DesignLayout.AddChoice( m_TAftChoice, "TAFT" );
    m_TAftChoice.AddItem( "CONE" );
    m_TAftChoice.AddItem( "OGIVE" );
    m_TAftChoice.UpdateItems();
    m_DesignLayout.AddSlider( m_LAftSlider, "LAFT", 10, "%7.3f" );
    m_DesignLayout.AddSlider( m_DAftSlider, "DAFT", 10, "%7.3f" );
    m_DesignLayout.AddSlider( m_WAftSlider, "WAFT", 10, "%7.3f" );
    m_DesignLayout.AddSlider( m_EAftSlider, "EAFT", 10, "%7.3f" );
    m_DesignLayout.AddSlider( m_ZAftSlider, "ZAFT", 10, "%7.3f" );
    m_DesignLayout.AddSlider( m_SlpTalSlider, "SLPTAL", 10, "%7.3f" );

    m_DesignLayout.AddYGap();
    m_DesignLayout.AddDividerBox( "Base" );
    m_DesignLayout.AddSlider( m_DExitSlider, "DEXIT", 10, "%7.3f" );
}

//==== Show Screen ====//
void MdcBodScreen::Show()
{
    if ( Update() )
    {
        m_FLTK_Window->show();
    }
}

//==== Update Screen ====//
bool MdcBodScreen::Update()
{
    assert( m_ScreenMgr );

    Geom* geom_ptr = m_ScreenMgr->GetCurrGeom();
    if ( !geom_ptr || geom_ptr->GetType().m_Type != MDC_BOD_GEOM_TYPE )
    {
        Hide();
        return false;
    }

    GeomScreen::Update();

    MdcBodGeom* body = dynamic_cast< MdcBodGeom* >( geom_ptr );
    assert( body );

    m_PresetChoice.Update( body->m_Preset.GetID() );
    m_NamelistChoice.Update( body->m_Namelist.GetID() );
    m_XOSlider.Update( body->m_XRelLoc.GetID() );

    m_TNoseChoice.Update( body->m_TNose.GetID() );
    m_PowerSlider.Update( body->m_Power.GetID() );
    m_LNoseSlider.Update( body->m_LNose.GetID() );
    m_DNoseSlider.Update( body->m_DNose.GetID() );
    m_WNoseSlider.Update( body->m_WNose.GetID() );
    m_ENoseSlider.Update( body->m_ENose.GetID() );
    m_BNoseSlider.Update( body->m_BNose.GetID() );
    m_TruncToggle.Update( body->m_Trunc.GetID() );
    m_SlpNosSlider.Update( body->m_SlpNos.GetID() );
    m_ZTipSlider.Update( body->m_ZTip.GetID() );
    m_ZNoseSlider.Update( body->m_ZNose.GetID() );

    m_LCentrSlider.Update( body->m_LCentr.GetID() );
    m_CentrDefaultToggle.Update( body->m_CentrDefault.GetID() );
    m_DCentrSlider.Update( body->m_DCentr.GetID() );
    m_WCentrSlider.Update( body->m_WCentr.GetID() );
    m_ECentrSlider.Update( body->m_ECentr.GetID() );
    m_ZCentrSlider.Update( body->m_ZCentr.GetID() );

    m_TAftChoice.Update( body->m_TAft.GetID() );
    m_LAftSlider.Update( body->m_LAft.GetID() );
    m_DAftSlider.Update( body->m_DAft.GetID() );
    m_WAftSlider.Update( body->m_WAft.GetID() );
    m_EAftSlider.Update( body->m_EAft.GetID() );
    m_ZAftSlider.Update( body->m_ZAft.GetID() );
    m_SlpTalSlider.Update( body->m_SlpTal.GetID() );
    m_DExitSlider.Update( body->m_DExit.GetID() );

    // Deactivate the inputs that shape nothing: the other namelist's sizes, a centerbody size that takes the nose's,
    // POWER but for a power law nose, SLPNOS but for an ogive, BNOSE for a power law or Haack nose that is not
    // truncated, a centerbody's or aft body's inputs where it has no length, and SLPTAL but for an ogive aft body.
    bool ell = body->m_Namelist() == MdcBodGeom::ELLBOD;
    int tnose = body->m_TNose();
    bool profile = tnose == MdcBodGeom::POWER || tnose == MdcBodGeom::HAACK || tnose == MdcBodGeom::KARMAN;
    bool centr = body->m_LCentr() > 0.0;
    bool aft = body->m_LAft() > 0.0;

    if ( tnose != MdcBodGeom::POWER )
    {
        m_PowerSlider.Deactivate();
    }
    if ( tnose != MdcBodGeom::OGIVE )
    {
        m_SlpNosSlider.Deactivate();
    }
    if ( profile && !body->m_Trunc() )
    {
        m_BNoseSlider.Deactivate();
    }

    if ( ell )
    {
        m_DNoseSlider.Deactivate();
        m_DCentrSlider.Deactivate();
        m_DAftSlider.Deactivate();
    }
    else
    {
        m_WNoseSlider.Deactivate();
        m_ENoseSlider.Deactivate();
        m_WCentrSlider.Deactivate();
        m_ECentrSlider.Deactivate();
        m_WAftSlider.Deactivate();
        m_EAftSlider.Deactivate();
    }

    if ( body->m_CentrDefault() )
    {
        m_DCentrSlider.Deactivate();
        m_WCentrSlider.Deactivate();
    }

    if ( !centr )
    {
        m_CentrDefaultToggle.Deactivate();
        m_DCentrSlider.Deactivate();
        m_WCentrSlider.Deactivate();
        m_ECentrSlider.Deactivate();
        m_ZCentrSlider.Deactivate();
    }

    if ( !aft )
    {
        m_TAftChoice.Deactivate();
        m_DAftSlider.Deactivate();
        m_WAftSlider.Deactivate();
        m_EAftSlider.Deactivate();
        m_ZAftSlider.Deactivate();
        m_SlpTalSlider.Deactivate();
    }
    else if ( body->m_TAft() != MdcBodGeom::OGIVE )
    {
        m_SlpTalSlider.Deactivate();
    }

    // A nose alone has no base to cut a nozzle into.
    if ( !centr && !aft )
    {
        m_DExitSlider.Deactivate();
    }

    return true;
}

//==== Non Menu Callbacks ====//
void MdcBodScreen::CallBack( Fl_Widget *w )
{
    GeomScreen::CallBack( w );
}

void MdcBodScreen::GuiDeviceCallBack( GuiDevice* device )
{
    MdcBodGeom* preset_geom = dynamic_cast< MdcBodGeom* >( m_ScreenMgr->GetCurrGeom() );
    if ( preset_geom && device == &m_SetPresetButton )
    {
        preset_geom->SetPreset();
        preset_geom->Update();
    }

    Geom* geom_ptr = m_ScreenMgr->GetCurrGeom();
    if ( device == &m_ConvertToStackButton && geom_ptr && geom_ptr->GetType().m_Type == MDC_BOD_GEOM_TYPE )
    {
        int choice = fl_choice( "Convert MDC_BOD to Stack Geom.\nReplace the MDC_BOD (can not be un-done), or add a Stack beside it?",
                                "Cancel", "Replace", "Add" );
        if ( choice == 1 || choice == 2 )
        {
            bool replace_flag = ( choice == 1 );

            // Replacing deletes this Geom, so nothing may touch it afterwards.
            Vehicle* veh = m_ScreenMgr->GetVehiclePtr();
            string stack_id = veh->ConvertMdcBodToStack( geom_ptr->GetID(), replace_flag );
            if ( stack_id.empty() )
            {
                fl_alert( "The MDC_BOD could not be converted." );
                return;
            }

            if ( replace_flag )
            {
                veh->SetActiveGeom( stack_id );
            }

            ManageGeomScreen* mgs = dynamic_cast< ManageGeomScreen* >( m_ScreenMgr->GetScreen( vsp::VSP_MANAGE_GEOM_SCREEN ) );
            if ( mgs )
            {
                mgs->SetNeedsShowHideGeoms();
            }

            m_ScreenMgr->SetUpdateFlag( true );
            return;
        }
        return;
    }

    GeomScreen::GuiDeviceCallBack( device );
}
