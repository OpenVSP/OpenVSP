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
Gearcreen::Gearcreen( ScreenMgr* mgr ) : GeomScreen( mgr, 400, 600, "Gear" )
{
    Fl_Group* design_tab = AddTab( "Design" );
    Fl_Group* design_group = AddSubGroup( design_tab, 5 );

    Fl_Group* tire_tab = AddTab( "Tire" );
    Fl_Group* tire_group = AddSubGroup( tire_tab, 5 );

    m_DesignLayout.SetGroupAndScreen( design_group, this );

    m_BogieBrowser = m_DesignLayout.AddFlBrowser( 100 );
    m_BogieBrowser->callback( staticScreenCB, this );
    m_DesignLayout.AddInput( m_BogieNameInput, "Name:" );

    m_DesignLayout.SetSameLineFlag( true );
    m_DesignLayout.SetFitWidthFlag( false );
    m_DesignLayout.SetButtonWidth( m_DesignLayout.GetW() / 2.0 );

    m_DesignLayout.AddButton( m_AddBogieButton, "Add" );
    m_DesignLayout.AddButton( m_RemoveBogieButton, "Delete" );

    m_DesignLayout.ForceNewLine();

    m_DesignLayout.AddButton( m_RenameBogieButton, "Rename" );
    m_DesignLayout.AddButton( m_RemoveAllBogiesButton, "Delete All" );

    m_DesignLayout.ForceNewLine();

    m_DesignLayout.AddButton( m_ShowAllBogiesButton, "Show All" );
    m_DesignLayout.AddButton( m_HideAllBogiesButton, "Hide All" );

    m_DesignLayout.ForceNewLine();

    m_DesignLayout.SetSameLineFlag( false );
    m_DesignLayout.SetFitWidthFlag( true );


    m_DesignLayout.AddYGap();

    m_DesignLayout.AddDividerBox( "Configuration" );

    m_DesignLayout.AddButton( m_SymmetricalButton, "Symmetrical" );

    m_DesignLayout.AddYGap();
    m_DesignLayout.AddSlider( m_NAcrossSlider, "Num Across", 10, "%6.0f" );

    m_DesignLayout.AddChoice( m_SpacingTypeChoice, "Mode" );
    m_SpacingTypeChoice.AddItem( "Center Distance", vsp::BOGIE_CENTER_DIST );
    m_SpacingTypeChoice.AddItem( "Center Distance Fraction", vsp::BOGIE_CENTER_DIST_FRAC );
    m_SpacingTypeChoice.AddItem( "Gap", vsp::BOGIE_GAP );
    m_SpacingTypeChoice.AddItem( "Gap Fraction", vsp::BOGIE_GAP_FRAC );
    m_SpacingTypeChoice.UpdateItems();

    m_DesignLayout.AddSlider( m_SpacingSlider, "Spacing", 10, "%6.5f" );
    m_DesignLayout.AddSlider( m_SpacingGapSlider, "Spacing Gap", 10, "%6.5f" );

    m_DesignLayout.AddYGap();
    m_DesignLayout.AddSlider( m_NTandemSlider, "Num Tandem", 10, "%6.0f" );

    m_DesignLayout.AddChoice( m_PitchTypeChoice, "Mode" );
    m_PitchTypeChoice.AddItem( "Center Distance", vsp::BOGIE_CENTER_DIST );
    m_PitchTypeChoice.AddItem( "Center Distance Fraction", vsp::BOGIE_CENTER_DIST_FRAC );
    m_PitchTypeChoice.AddItem( "Gap", vsp::BOGIE_GAP );
    m_PitchTypeChoice.AddItem( "Gap Fraction", vsp::BOGIE_GAP_FRAC );
    m_PitchTypeChoice.UpdateItems();

    m_DesignLayout.AddSlider( m_PitchSlider, "Pitch", 10, "%6.5f" );
    m_DesignLayout.AddSlider( m_PitchGapSlider, "Pitch Gap", 10, "%6.5f" );

    m_DesignLayout.AddYGap();
    m_DesignLayout.AddDividerBox( "Contact Point" );

    m_DesignLayout.AddSlider( m_TireXSlider, "X Contact", 10, "%6.5f" );
    m_DesignLayout.AddSlider( m_TireYSlider, "Y Contact", 10, "%6.5f" );
    m_DesignLayout.AddSlider( m_TireZSlider, "Z Above Ground", 10, "%6.5f" );


    m_TireGroup.SetGroupAndScreen( tire_group, this );

    m_TireGroup.AddDividerBox( "Tire" );

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

    m_TireGroup.AddYGap();

    m_TireGroup.SetFitWidthFlag( false );
    m_TireGroup.SetButtonWidth( toggleButtonWidth );
    m_TireGroup.AddButton( m_DeflectionToggleButton, "" );
    m_TireGroup.SetFitWidthFlag( true );
    m_TireGroup.SetButtonWidth( bw );
    m_TireGroup.AddSlider( m_DeflectionSlider, "Deflection", 1, "%6.5f" );

    m_TireGroup.ForceNewLine();

    m_TireGroup.SetFitWidthFlag( false );
    m_TireGroup.SetButtonWidth( toggleButtonWidth );
    m_TireGroup.AddButton( m_SLRToggleButton, "" );
    m_TireGroup.SetFitWidthFlag( true );
    m_TireGroup.SetButtonWidth( bw );
    m_TireGroup.AddSlider( m_SLRSlider, "SLR", 1, "%6.5f" );

    m_SLRToggleGroup.Init( this );
    m_SLRToggleGroup.AddButton( m_DeflectionToggleButton.GetFlButton() ); // false
    m_SLRToggleGroup.AddButton( m_SLRToggleButton.GetFlButton() ); // true

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

    std::vector < Bogie * > bogies = gear_ptr->GetBogieVec();

    int h_pos = m_BogieBrowser->hposition();
    int v_pos = m_BogieBrowser->vposition();
    m_BogieBrowser->clear();
    for( int i = 0; i < ( int )bogies.size(); i++ )
    {
        if ( bogies[i] )
        {
            string bogieName = bogies[i]->GetName();

            m_BogieBrowser->add( bogieName.c_str() );
        }
    }

    int index = gear_ptr->GetCurrBogieIndex();
    if ( index >= 0 && index < ( int )bogies.size() )
    {
        m_BogieBrowser->select( index + 1 );
    }
    m_BogieBrowser->hposition( h_pos );
    m_BogieBrowser->vposition( v_pos );



    Bogie* bogie_ptr = gear_ptr->GetCurrentBogie();

    if ( bogie_ptr )
    {

        m_SymmetricalButton.Update( bogie_ptr->m_Symmetrical.GetID() );

        m_NAcrossSlider.Update( bogie_ptr->m_NAcross.GetID() );

        m_SpacingTypeChoice.Update( bogie_ptr->m_SpacingType.GetID() );

        if ( bogie_ptr->m_SpacingType() == vsp::BOGIE_CENTER_DIST || bogie_ptr->m_SpacingType() == vsp::BOGIE_GAP )
        {
            m_SpacingSlider.Update( 1, bogie_ptr->m_Spacing.GetID(), bogie_ptr->m_SpacingFrac.GetID() );
            m_SpacingGapSlider.Update( 1, bogie_ptr->m_SpacingGap.GetID(), bogie_ptr->m_SpacingGapFrac.GetID() );
        }
        else
        {
            m_SpacingSlider.Update( 2, bogie_ptr->m_Spacing.GetID(), bogie_ptr->m_SpacingFrac.GetID() );
            m_SpacingGapSlider.Update( 2, bogie_ptr->m_SpacingGap.GetID(), bogie_ptr->m_SpacingGapFrac.GetID() );
        }


        if ( bogie_ptr->m_NAcross() > 1 )
        {
            m_SpacingTypeChoice.Activate();

            if ( bogie_ptr->m_SpacingType() == vsp::BOGIE_CENTER_DIST || bogie_ptr->m_SpacingType() == vsp::BOGIE_CENTER_DIST_FRAC )
            {
                m_SpacingSlider.Activate();
                m_SpacingGapSlider.Deactivate();
            }
            else
            {
                m_SpacingSlider.Deactivate();
                m_SpacingGapSlider.Activate();
            }
        }
        else
        {
            m_SpacingTypeChoice.Deactivate();
            m_SpacingSlider.Deactivate();
            m_SpacingGapSlider.Deactivate();
        }



        m_NTandemSlider.Update( bogie_ptr->m_NTandem.GetID() );

        m_PitchTypeChoice.Update( bogie_ptr->m_PitchType.GetID() );

        if ( bogie_ptr->m_PitchType() == vsp::BOGIE_CENTER_DIST || bogie_ptr->m_PitchType() == vsp::BOGIE_GAP )
        {
            m_PitchSlider.Update( 1, bogie_ptr->m_Pitch.GetID(), bogie_ptr->m_PitchFrac.GetID() );
            m_PitchGapSlider.Update( 1, bogie_ptr->m_PitchGap.GetID(), bogie_ptr->m_PitchGapFrac.GetID() );
        }
        else
        {
            m_PitchSlider.Update( 2, bogie_ptr->m_Pitch.GetID(), bogie_ptr->m_PitchFrac.GetID() );
            m_PitchGapSlider.Update( 2, bogie_ptr->m_PitchGap.GetID(), bogie_ptr->m_PitchGapFrac.GetID() );
        }

        if ( bogie_ptr->m_NTandem() > 1 )
        {
            m_PitchTypeChoice.Activate();

            if ( bogie_ptr->m_PitchType() == vsp::BOGIE_CENTER_DIST || bogie_ptr->m_PitchType() == vsp::BOGIE_CENTER_DIST_FRAC )
            {
                m_PitchSlider.Activate();
                m_PitchGapSlider.Deactivate();
            }
            else
            {
                m_PitchSlider.Deactivate();
                m_PitchGapSlider.Activate();
            }
        }
        else
        {
            m_PitchTypeChoice.Deactivate();
            m_PitchSlider.Deactivate();
            m_PitchGapSlider.Deactivate();
        }

        m_TireXSlider.Update( bogie_ptr->m_XContactPt.GetID() );
        m_TireYSlider.Update( bogie_ptr->m_YContactPt.GetID() );
        m_TireZSlider.Update( bogie_ptr->m_ZAboveGround.GetID() );

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


        m_SLRToggleGroup.Update( bogie_ptr->m_SLRFlag.GetID() );
        m_DeflectionSlider.Update( bogie_ptr->m_DeflectionPct.GetID() );
        m_SLRSlider.Update( bogie_ptr->m_StaticRadius.GetID() );


        if ( bogie_ptr->m_SLRFlag() )
        {
            m_SLRSlider.Activate();
            m_DeflectionSlider.Deactivate();
        }
        else
        {
            m_SLRSlider.Deactivate();
            m_DeflectionSlider.Activate();
        }

    }

    return true;
}


//==== Non Menu Callbacks ====//
void Gearcreen::CallBack( Fl_Widget *w )
{
    Geom* geom_ptr = m_ScreenMgr->GetCurrGeom();

    GearGeom* gear_ptr = dynamic_cast< GearGeom* >( geom_ptr );

    if ( w == m_BogieBrowser )
    {
        int sel = m_BogieBrowser->value();
        gear_ptr->SetCurrBogieIndex( sel - 1 );

        Bogie * bogie = gear_ptr->GetCurrentBogie();
        if ( bogie )
        {
            m_BogieNameInput.Update( bogie->GetName() );
        }
    }

    GeomScreen::CallBack( w );
}

void Gearcreen::GuiDeviceCallBack( GuiDevice* device )
{
    Geom* geom_ptr = m_ScreenMgr->GetCurrGeom();

    GearGeom* gear_ptr = dynamic_cast< GearGeom* >( geom_ptr );

    if ( device == &m_AddBogieButton )
    {
        Bogie * bogie = gear_ptr->CreateAndAddBogie();
        if ( bogie )
        {
            bogie->SetName( m_BogieNameInput.GetString() );
        }
    }
    else if ( device == &m_RemoveBogieButton )
    {
        gear_ptr->DelBogie( gear_ptr->GetCurrBogieIndex() );
    }
    else if ( device == &m_RemoveAllBogiesButton )
    {
        gear_ptr->DelAllBogies();
    }
    else if ( device == &m_ShowAllBogiesButton )
    {
        gear_ptr->ShowAllBogies();
    }
    else if ( device == &m_HideAllBogiesButton )
    {
        gear_ptr->HideAllBogies();
    }
    else if ( device == &m_RenameBogieButton )
    {
        Bogie * bogie = gear_ptr->GetCurrentBogie();
        if ( bogie )
        {
            bogie->SetName( m_BogieNameInput.GetString() );
        }
    }

    GeomScreen::GuiDeviceCallBack( device );
}



