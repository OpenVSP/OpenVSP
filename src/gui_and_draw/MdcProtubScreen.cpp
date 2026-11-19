//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//
//
//////////////////////////////////////////////////////////////////////

#include "MdcProtubScreen.h"
#include "ScreenMgr.h"

//==== Constructor ====//
MdcProtubScreen::MdcProtubScreen( ScreenMgr* mgr ) : GeomScreen( mgr, 400, 800, "MDC_PROTUB", "MdcProtub.html" )
{
    Fl_Group* design_tab = AddTab( "Design" );
    Fl_Group* design_group = AddSubGroup( design_tab, 5 );

    m_DesignLayout.SetGroupAndScreen( design_group, this );

    // Choices' labels as wide as sliders', so they line up.
    m_DesignLayout.SetChoiceButtonWidth( m_DesignLayout.GetButtonWidth() );

    m_DesignLayout.AddYGap();
    m_DesignLayout.AddDividerBox( "Protuberance" );

    // A preset shape, set on demand so a choice keeps the shape as it is.
    m_DesignLayout.SetSameLineFlag( true );
    m_DesignLayout.SetFitWidthFlag( true );
    m_DesignLayout.AddChoice( m_PresetChoice, "Preset", m_DesignLayout.GetButtonWidth() );
    m_DesignLayout.SetFitWidthFlag( false );
    m_DesignLayout.AddButton( m_SetPresetButton, "Set" );
    m_DesignLayout.ForceNewLine();
    m_DesignLayout.SetSameLineFlag( false );
    m_DesignLayout.SetFitWidthFlag( true );
    m_PresetChoice.AddItem( "Default", vsp::MDC_PROTUB_PRESET_DEFAULT );
    m_PresetChoice.AddItem( "VCYL", vsp::MDC_PROTUB_PRESET_VCYL );
    m_PresetChoice.AddItem( "HCYL", vsp::MDC_PROTUB_PRESET_HCYL );
    m_PresetChoice.AddItem( "LUG", vsp::MDC_PROTUB_PRESET_LUG );
    m_PresetChoice.AddItem( "SHOE", vsp::MDC_PROTUB_PRESET_SHOE );
    m_PresetChoice.AddItem( "BLOCK", vsp::MDC_PROTUB_PRESET_BLOCK );
    m_PresetChoice.AddItem( "FAIRING", vsp::MDC_PROTUB_PRESET_FAIRING );
    m_PresetChoice.AddItem( "Small Lug", vsp::MDC_PROTUB_PRESET_SMALL_LUG );
    m_PresetChoice.AddItem( "Medium Lug", vsp::MDC_PROTUB_PRESET_MEDIUM_LUG );
    m_PresetChoice.AddItem( "Large Lug", vsp::MDC_PROTUB_PRESET_LARGE_LUG );
    m_PresetChoice.UpdateItems();

    // A choice of type keeps the members as they are.
    m_DesignLayout.AddChoice( m_PTypeChoice, "PTYPE" );
    m_PTypeChoice.AddItem( "VCYL", MdcProtubGeom::VCYL );
    m_PTypeChoice.AddItem( "HCYL", MdcProtubGeom::HCYL );
    m_PTypeChoice.AddItem( "LUG", MdcProtubGeom::LUG );
    m_PTypeChoice.AddItem( "SHOE", MdcProtubGeom::SHOE );
    m_PTypeChoice.AddItem( "BLOCK", MdcProtubGeom::BLOCK );
    m_PTypeChoice.AddItem( "FAIRING", MdcProtubGeom::FAIRING );
    m_PTypeChoice.UpdateItems();

    m_DesignLayout.AddYGap();
    m_DesignLayout.AddDividerBox( "Placement" );
    m_DesignLayout.AddSlider( m_XProtSlider, "XPROT", 10, "%7.3f" );
    m_DesignLayout.AddSlider( m_NLocSlider, "NLOC", 20, "%5.0f" );
    m_DesignLayout.AddButton( m_PhiProFlagToggle, "PHIPRO given" );
    m_DesignLayout.AddIndexSelector( m_LocIndexSelector );
    // PHIPRO of the protuberance the counter shows.
    m_DesignLayout.AddSlider( m_PhiProSlider, "PHIPRO", 180.0, "%7.3f" );
    m_DesignLayout.AddSlider( m_RBodySlider, "RBODY", 10, "%7.3f" );
    m_DesignLayout.AddButton( m_BuryRootToggle, "Bury Root" );

    // Each member's header names its part in the type; see MemberRole.
    for ( int m = 0; m < MdcProtubGeom::MAX_MEM; m++ )
    {
        m_DesignLayout.AddYGap();
        m_MemberDivider[m] = m_DesignLayout.AddDividerBox( "Member " + to_string( m + 1 ) );
        m_DesignLayout.AddSlider( m_LProtSlider[m], "LPROT", 10, "%7.3f" );
        m_DesignLayout.AddSlider( m_WProtSlider[m], "WPROT", 10, "%7.3f" );
        m_DesignLayout.AddSlider( m_HProtSlider[m], "HPROT", 10, "%7.3f" );
        m_DesignLayout.AddSlider( m_OProtSlider[m], "OPROT", 10, "%7.3f" );
    }
}

//==== Show Screen ====//
void MdcProtubScreen::Show()
{
    if ( Update() )
    {
        m_FLTK_Window->show();
    }
}

// The part member m plays in a protuberance of type ptype, or empty where the type has no such member.
static string MemberRole( int ptype, int m )
{
    if ( ptype == MdcProtubGeom::LUG )
    {
        if ( m == 0 )
        {
            return "Base Plate";
        }
        else if ( m == 1 )
        {
            return "First Post";
        }
        else if ( m == 2 )
        {
            return "Second Post";
        }
        return "Top Plate";
    }
    if ( ptype == MdcProtubGeom::SHOE )
    {
        if ( m == 0 )
        {
            return "Base Plate";
        }
        else if ( m == 1 )
        {
            return "Web";
        }
        else if ( m == 2 )
        {
            return "Top Plate";
        }
        return string();
    }
    if ( m > 0 )
    {
        return string();
    }
    if ( ptype == MdcProtubGeom::VCYL )
    {
        return "Vertical Cylinder";
    }
    else if ( ptype == MdcProtubGeom::HCYL )
    {
        return "Horizontal Cylinder";
    }
    else if ( ptype == MdcProtubGeom::BLOCK )
    {
        return "Block";
    }
    return "Fairing";
}

//==== Update Screen ====//
bool MdcProtubScreen::Update()
{
    assert( m_ScreenMgr );

    Geom* geom_ptr = m_ScreenMgr->GetCurrGeom();
    if ( !geom_ptr || geom_ptr->GetType().m_Type != MDC_PROTUB_GEOM_TYPE )
    {
        Hide();
        return false;
    }

    GeomScreen::Update();

    MdcProtubGeom* protub = dynamic_cast< MdcProtubGeom* >( geom_ptr );
    assert( protub );

    m_PresetChoice.Update( protub->m_Preset.GetID() );
    m_PTypeChoice.Update( protub->m_PType.GetID() );
    m_XProtSlider.Update( protub->m_XRelLoc.GetID() );
    m_RBodySlider.Update( protub->m_RBody.GetID() );
    m_BuryRootToggle.Update( protub->m_BuryRoot.GetID() );

    m_NLocSlider.Update( protub->m_NLoc.GetID() );
    m_PhiProFlagToggle.Update( protub->m_PhiProFlag.GetID() );
    m_LocIndexSelector.Update( protub->m_LocIndex.GetID() );

    // PHIPRO of the protuberance the counter shows, given only if PHIPRO is.
    m_PhiProSlider.Update( XSecSurfGeom::ParmIDAt( protub->m_PhiPro, protub->m_LocIndex() - 1 ) );
    if ( !protub->m_PhiProFlag() )
    {
        m_PhiProSlider.Deactivate();
    }

    // The members the type has, and a FAIRING's OPROT.
    int ptype = protub->m_PType();
    for ( int m = 0; m < MdcProtubGeom::MAX_MEM; m++ )
    {
        m_LProtSlider[m].Update( XSecSurfGeom::ParmIDAt( protub->m_LProt, m ) );
        m_WProtSlider[m].Update( XSecSurfGeom::ParmIDAt( protub->m_WProt, m ) );
        m_HProtSlider[m].Update( XSecSurfGeom::ParmIDAt( protub->m_HProt, m ) );
        m_OProtSlider[m].Update( XSecSurfGeom::ParmIDAt( protub->m_OProt, m ) );

        string header = "Member " + to_string( m + 1 );
        string role = MemberRole( ptype, m );
        if ( !role.empty() )
        {
            header += ": " + role;
        }
        m_MemberDivider[m]->copy_label( header.c_str() );

        if ( ptype == MdcProtubGeom::FAIRING )
        {
            m_OProtSlider[m].Deactivate();
        }
    }

    return true;
}

//==== Non Menu Callbacks ====//
void MdcProtubScreen::CallBack( Fl_Widget *w )
{
    GeomScreen::CallBack( w );
}

void MdcProtubScreen::GuiDeviceCallBack( GuiDevice* device )
{
    MdcProtubGeom* protub = dynamic_cast< MdcProtubGeom* >( m_ScreenMgr->GetCurrGeom() );
    if ( protub && device == &m_SetPresetButton )
    {
        protub->SetPreset();
        protub->Update();
    }

    GeomScreen::GuiDeviceCallBack( device );
}
