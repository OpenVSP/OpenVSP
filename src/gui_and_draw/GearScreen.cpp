//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//
//
//////////////////////////////////////////////////////////////////////

#include "GearScreen.h"
#include "ScreenMgr.h"
#include "GearGeom.h"
#include "Vehicle.h"
#include "ParmMgr.h"

#include <assert.h>


//==== Constructor ====//
Gearcreen::Gearcreen( ScreenMgr* mgr ) : GeomScreen( mgr, 400, 525, "Gear" )
{
    Fl_Group* tire_tab = AddTab( "Tire" );
    Fl_Group* tire_group = AddSubGroup( tire_tab, 5 );

    m_TireGroup.SetGroupAndScreen( tire_group, this );
    m_TireGroup.AddDividerBox( "Tire" );

    m_TireGroup.AddSlider( m_TireXSlider, "X", 10, "%6.5f" );
    m_TireGroup.AddSlider( m_TireYSlider, "Y", 10, "%6.5f" );
    m_TireGroup.AddSlider( m_TireZSlider, "Z", 10, "%6.5f" );


    m_TireGroup.AddSlider( m_TireDiameterSlider, "Diameter", 10, "%6.5f" );
    m_TireGroup.AddSlider( m_TireWidthSlider, "Width", 10, "%6.5f" );

    int toggleButtonWidth = 35;
    int bw = m_TireGroup.GetButtonWidth();

    m_TireGroup.AddYGap();
    m_TireGroup.SetSameLineFlag( true );

    m_TireGroup.SetFitWidthFlag( false );
    m_TireGroup.SetButtonWidth( toggleButtonWidth );
    m_TireGroup.AddButton( m_DrimFracToggleButton, "01" );
    m_TireGroup.AddButton( m_DrimToggleButton, "0D" );
    m_TireGroup.SetFitWidthFlag( true );
    m_TireGroup.SetButtonWidth( bw );
    m_TireGroup.AddSlider( m_DrimSlider, "Drim", 1, "%6.5f" );

    m_DrimToggleGroup.Init( this );
    m_DrimToggleGroup.AddButton( m_DrimFracToggleButton.GetFlButton() ); // false
    m_DrimToggleGroup.AddButton( m_DrimToggleButton.GetFlButton() ); // true


    m_TireGroup.ForceNewLine();


    m_TireGroup.SetFitWidthFlag( false );
    m_TireGroup.SetButtonWidth( toggleButtonWidth );
    m_TireGroup.AddButton( m_WrimFracToggleButton, "01" );
    m_TireGroup.AddButton( m_WrimToggleButton, "0W" );
    m_TireGroup.SetFitWidthFlag( true );
    m_TireGroup.SetButtonWidth( bw );
    m_TireGroup.AddSlider( m_WrimSlider, "Wrim", 1, "%6.5f" );

    m_WrimToggleGroup.Init( this );
    m_WrimToggleGroup.AddButton( m_WrimFracToggleButton.GetFlButton() ); // false
    m_WrimToggleGroup.AddButton( m_WrimToggleButton.GetFlButton() ); // true

    m_TireGroup.ForceNewLine();
    m_TireGroup.AddYGap();


    m_TireGroup.SetFitWidthFlag( false );
    m_TireGroup.SetButtonWidth( toggleButtonWidth );
    m_TireGroup.AddButton( m_HsFracToggleButton, "01" );
    m_TireGroup.AddButton( m_HsToggleButton, "0H" );
    m_TireGroup.SetFitWidthFlag( true );
    m_TireGroup.SetButtonWidth( bw );
    m_TireGroup.AddSlider( m_HsSlider, "Hs", 1, "%6.5f" );

    m_HsToggleGroup.Init( this );
    m_HsToggleGroup.AddButton( m_HsFracToggleButton.GetFlButton() ); // false
    m_HsToggleGroup.AddButton( m_HsToggleButton.GetFlButton() ); // true

    m_TireGroup.ForceNewLine();


    m_TireGroup.SetFitWidthFlag( false );
    m_TireGroup.SetButtonWidth( toggleButtonWidth );
    m_TireGroup.AddButton( m_WsFracToggleButton, "01" );
    m_TireGroup.AddButton( m_WsToggleButton, "0W" );
    m_TireGroup.SetFitWidthFlag( true );
    m_TireGroup.SetButtonWidth( bw );
    m_TireGroup.AddSlider( m_WsSlider, "Ws", 1, "%6.5f" );

    m_WsToggleGroup.Init( this );
    m_WsToggleGroup.AddButton( m_WsFracToggleButton.GetFlButton() ); // false
    m_WsToggleGroup.AddButton( m_WsToggleButton.GetFlButton() ); // true

    m_TireGroup.ForceNewLine();

}


//==== Show Pod Screen ====//
void Gearcreen::Show()
{
    if ( Update() )
    {
        m_FLTK_Window->show();
    }
}

//==== Update Disk Screen ====//
bool Gearcreen::Update()
{
    assert( m_ScreenMgr );

    Geom* geom_ptr = m_ScreenMgr->GetCurrGeom();
    if ( !geom_ptr || geom_ptr->GetType().m_Type != GEAR_GEOM_TYPE )
    {
        Hide();
        return false;
    }

    GeomScreen::Update();

    //==== Update Pod Specific Parms ====//
    GearGeom* gear_ptr = dynamic_cast< GearGeom* >( geom_ptr );
    assert( gear_ptr );

    Bogie* bogie_ptr = gear_ptr->GetCurrentBogie();

    if ( bogie_ptr )
    {
        m_TireXSlider.Update( bogie_ptr->m_X.GetID() );
        m_TireYSlider.Update( bogie_ptr->m_Y.GetID() );
        m_TireZSlider.Update( bogie_ptr->m_Z.GetID() );

        m_TireDiameterSlider.Update( bogie_ptr->m_Diameter.GetID() );
        m_TireWidthSlider.Update( bogie_ptr->m_Width.GetID() );

        m_DrimToggleGroup.Update( bogie_ptr->m_DrimFlag.GetID() );
        if ( bogie_ptr->m_DrimFlag() )
        {
            m_DrimSlider.Update( 2, bogie_ptr->m_DrimFrac.GetID(), bogie_ptr->m_Drim.GetID());
        }
        else
        {
            m_DrimSlider.Update( 1, bogie_ptr->m_DrimFrac.GetID(), bogie_ptr->m_Drim.GetID());
        }

        m_WrimToggleGroup.Update( bogie_ptr->m_WrimFlag.GetID() );
        if ( bogie_ptr->m_WrimFlag() )
        {
            m_WrimSlider.Update( 2, bogie_ptr->m_WrimFrac.GetID(), bogie_ptr->m_Wrim.GetID());
        }
        else
        {
            m_WrimSlider.Update( 1, bogie_ptr->m_WrimFrac.GetID(), bogie_ptr->m_Wrim.GetID());
        }

        m_HsToggleGroup.Update( bogie_ptr->m_HsFlag.GetID() );
        if ( bogie_ptr->m_HsFlag() )
        {
            m_HsSlider.Update( 2, bogie_ptr->m_HsFrac.GetID(), bogie_ptr->m_Hs.GetID());
        }
        else
        {
            m_HsSlider.Update( 1, bogie_ptr->m_HsFrac.GetID(), bogie_ptr->m_Hs.GetID());
        }

        m_WsToggleGroup.Update( bogie_ptr->m_WsFlag.GetID() );
        if ( bogie_ptr->m_WsFlag() )
        {
            m_WsSlider.Update( 2, bogie_ptr->m_WsFrac.GetID(), bogie_ptr->m_Ws.GetID());
        }
        else
        {
            m_WsSlider.Update( 1, bogie_ptr->m_WsFrac.GetID(), bogie_ptr->m_Ws.GetID());
        }
    }

    return true;
}


//==== Non Menu Callbacks ====//
void Gearcreen::CallBack( Fl_Widget *w )
{
    GeomScreen::CallBack( w );
}




