//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//
//
//////////////////////////////////////////////////////////////////////

#include "MdcProtubScreen.h"
#include "ScreenMgr.h"

//==== Constructor ====//
MdcProtubScreen::MdcProtubScreen( ScreenMgr* mgr ) : GeomScreen( mgr, 400, 800, "MDC_PROTUB" )
{
    Fl_Group* design_tab = AddTab( "Design" );
    Fl_Group* design_group = AddSubGroup( design_tab, 5 );

    m_DesignLayout.SetGroupAndScreen( design_group, this );

    // Choices' labels as wide as sliders', so they line up.
    m_DesignLayout.SetChoiceButtonWidth( m_DesignLayout.GetButtonWidth() );

    m_DesignLayout.AddYGap();
    m_DesignLayout.AddDividerBox( "Protuberance" );

    // The type's default members, set on demand so a choice of type keeps the members as they are.
    m_DesignLayout.SetSameLineFlag( true );
    m_DesignLayout.SetFitWidthFlag( true );
    m_DesignLayout.AddChoice( m_PTypeChoice, "PTYPE", m_DesignLayout.GetButtonWidth() );
    m_DesignLayout.SetFitWidthFlag( false );
    m_DesignLayout.AddButton( m_SetDefaultButton, "Set Default" );
    m_DesignLayout.ForceNewLine();
    m_DesignLayout.SetSameLineFlag( false );
    m_DesignLayout.SetFitWidthFlag( true );
    m_PTypeChoice.AddItem( "VCYL", MdcProtubGeom::VCYL );
    m_PTypeChoice.AddItem( "HCYL", MdcProtubGeom::HCYL );
    m_PTypeChoice.AddItem( "LUG", MdcProtubGeom::LUG );
    m_PTypeChoice.AddItem( "SHOE", MdcProtubGeom::SHOE );
    m_PTypeChoice.AddItem( "BLOCK", MdcProtubGeom::BLOCK );
    m_PTypeChoice.AddItem( "FAIRING", MdcProtubGeom::FAIRING );
    m_PTypeChoice.UpdateItems();
    m_DesignLayout.AddSlider( m_XProtSlider, "XPROT", 10, "%7.3f" );
    m_DesignLayout.AddSlider( m_RBodySlider, "RBODY", 10, "%7.3f" );

    // PHIPRO of the protuberance the counter shows.
    m_DesignLayout.AddYGap();
    m_DesignLayout.AddDividerBox( "Locations" );
    m_DesignLayout.AddSlider( m_NLocSlider, "NLOC", 20, "%5.0f" );
    m_DesignLayout.AddButton( m_PhiProFlagToggle, "PHIPRO given" );
    m_DesignLayout.AddIndexSelector( m_LocIndexSelector );
    m_DesignLayout.AddSlider( m_PhiProSlider, "PHIPRO", 180.0, "%7.3f" );

    // Member 1 is a LUG's or SHOE's base plate, 2 a SHOE's web or a LUG's first post, 3 a SHOE's top plate or
    // a LUG's second post, and 4 a LUG's top plate.
    for ( int m = 0; m < MdcProtubGeom::MAX_MEM; m++ )
    {
        m_DesignLayout.AddYGap();
        m_DesignLayout.AddDividerBox( "Member " + to_string( m + 1 ) );
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

    m_PTypeChoice.Update( protub->m_PType.GetID() );
    m_XProtSlider.Update( protub->m_XRelLoc.GetID() );
    m_RBodySlider.Update( protub->m_RBody.GetID() );

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
    if ( protub && device == &m_SetDefaultButton )
    {
        protub->SetDefaults();
        protub->Update();
    }

    GeomScreen::GuiDeviceCallBack( device );
}
