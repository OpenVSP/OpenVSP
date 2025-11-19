//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//
//
//////////////////////////////////////////////////////////////////////

#include "CobraScreen.h"
#include "ScreenMgr.h"
#include "CobraGeom.h"

//==== Constructor ====//
CobraScreen::CobraScreen( ScreenMgr* mgr ) : GeomScreen( mgr, 400, 800, "Cobra" )
{
    Fl_Group* design_tab = AddTab( "Design" );
    Fl_Group* design_group = AddSubGroup( design_tab, 5 );

    m_DesignLayout.SetGroupAndScreen( design_group, this );
    m_DesignLayout.AddDividerBox( "Design" );

    m_DesignLayout.AddYGap();
    m_DesignLayout.AddDividerBox( "Nose" );

    m_DesignLayout.AddSlider( m_XradNSlider, "Length", 10, "%7.3f" );
    m_DesignLayout.AddSlider( m_YradNSlider, "Width", 10, "%7.3f" );
    m_DesignLayout.AddSlider( m_PowXSlider, "PowX", 5, "%7.3f" );

    m_DesignLayout.AddSlider( m_ZradNUSlider, "Upper Height", 10, "%7.3f" );
    m_DesignLayout.AddSlider( m_PowNUSlider, "PowNU", 5, "%7.3f" );
    m_DesignLayout.AddButton( m_NoseULSymmToggle, "Nose ULSymm" );
    m_DesignLayout.AddSlider( m_ZradNLSlider, "Lower Height", 10, "%7.3f" );
    m_DesignLayout.AddSlider( m_PowNLSlider, "PowNL", 5, "%7.3f" );

    m_DesignLayout.AddYGap();
    m_DesignLayout.AddDividerBox( "Aft" );

    m_DesignLayout.AddSlider( m_XlenASlider, "Length", 10, "%7.3f" );
    m_DesignLayout.AddSlider( m_YradASlider, "Width", 10, "%7.3f" );

    m_DesignLayout.AddSlider( m_ZradAUSlider, "Upper Height", 10, "%7.3f" );
    m_DesignLayout.AddSlider( m_PowAUSlider, "PowAU", 5, "%7.3f" );
    m_DesignLayout.AddButton( m_AftULSymmToggle, "Aft ULSymm" );
    m_DesignLayout.AddSlider( m_ZradALSlider, "Lower Height", 10, "%7.3f" );
    m_DesignLayout.AddSlider( m_PowALSlider, "PowAL", 5, "%7.3f" );

    m_DesignLayout.AddYGap();

}


//==== Show Pod Screen ====//
void CobraScreen::Show()
{
    if ( Update() )
    {
        m_FLTK_Window->show();
    }
}

//==== Update Disk Screen ====//
bool CobraScreen::Update()
{
    assert( m_ScreenMgr );

    Geom* geom_ptr = m_ScreenMgr->GetCurrGeom();
    if ( !geom_ptr || geom_ptr->GetType().m_Type != COBRA_GEOM_TYPE )
    {
        Hide();
        return false;
    }

    GeomScreen::Update();

    //==== Update Cobra Specific Parms ====//
    CobraGeom* cobra_ptr = dynamic_cast< CobraGeom* >( geom_ptr );
    assert( cobra_ptr );

    m_XradNSlider.Update( cobra_ptr->m_XradN.GetID() );
    m_YradNSlider.Update( cobra_ptr->m_YradN.GetID() );
    m_PowXSlider.Update( cobra_ptr->m_PowX.GetID() );
    m_ZradNLSlider.Update( cobra_ptr->m_ZradNL.GetID() );
    m_PowNLSlider.Update( cobra_ptr->m_PowNL.GetID() );
    m_NoseULSymmToggle.Update( cobra_ptr->m_NoseULSymm.GetID() );
    m_ZradNUSlider.Update( cobra_ptr->m_ZradNU.GetID() );
    m_PowNUSlider.Update( cobra_ptr->m_PowNU.GetID() );

    m_XlenASlider.Update( cobra_ptr->m_XlenA.GetID() );
    m_YradASlider.Update( cobra_ptr->m_YradA.GetID() );
    m_ZradALSlider.Update( cobra_ptr->m_ZradAL.GetID() );
    m_PowALSlider.Update( cobra_ptr->m_PowAL.GetID() );
    m_AftULSymmToggle.Update( cobra_ptr->m_AftULSymm.GetID() );
    m_ZradAUSlider.Update( cobra_ptr->m_ZradAU.GetID() );
    m_PowAUSlider.Update( cobra_ptr->m_PowAU.GetID() );

    if ( cobra_ptr->m_NoseULSymm() )
    {
        m_ZradNLSlider.Deactivate();
        m_PowNLSlider.Deactivate();
    }

    if ( cobra_ptr->m_AftULSymm() )
    {
        m_ZradALSlider.Deactivate();
        m_PowALSlider.Deactivate();
    }

    return true;
}


//==== Non Menu Callbacks ====//
void CobraScreen::CallBack( Fl_Widget *w )
{
    GeomScreen::CallBack( w );
}




