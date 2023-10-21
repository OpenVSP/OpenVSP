//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//
//
//////////////////////////////////////////////////////////////////////

#include "ConformalScreen.h"
#include "ScreenMgr.h"
#include "ConformalGeom.h"
#include "APIDefines.h"

//==== Constructor ====//
ConformalScreen::ConformalScreen( ScreenMgr* mgr ) : GeomScreen( mgr, 400, 657 + 25, "Conformal" )
{
    Fl_Group* design_tab = AddTab( "Design" );
    Fl_Group* design_group = AddSubGroup( design_tab, 5 );

    m_DesignLayout.SetGroupAndScreen( design_group, this );
    m_DesignLayout.AddDividerBox( "Design" );

    //==== Design ====//
    m_DesignLayout.AddSlider( m_OffsetSlider, "Offset", 0.1, "%7.3f" );
    m_DesignLayout.AddButton( m_OffsetEndsToggle, "Offset Ends" );

    m_DesignLayout.AddYGap();

    m_DesignLayout.AddDividerBox( "U Trim" );

    m_DesignLayout.AddSubGroupLayout( m_TrimGroup, m_DesignLayout.GetW(), m_DesignLayout.GetH() );

    m_DesignLayout.SetButtonWidth( m_DesignLayout.GetButtonWidth() + 15 );
    int buttonW = m_DesignLayout.GetButtonWidth();
    int buttonSMW = buttonW - 25;
    int toggleW = 35;
    int blankW = 20;
    int buttonLGW = buttonSMW + 2 * toggleW;

    int start_y = m_TrimGroup.GetY();

    m_TrimGroup.SetSameLineFlag( false );
    m_TrimGroup.SetFitWidthFlag( true );
    m_TrimGroup.AddButton( m_UTrimToggle, "Enable U Trimming" );

    m_TrimGroup.SetSameLineFlag( true );
    m_TrimGroup.SetFitWidthFlag( false );

    m_TrimGroup.SetButtonWidth( blankW );
    m_TrimGroup.AddButton( m_UMinUTrimButton, "" );

    m_TrimGroup.SetFitWidthFlag( true );

    m_TrimGroup.SetButtonWidth( buttonLGW );
    m_TrimGroup.AddSlider( m_UTrimMinSlider, "U Min", 1.0, "%5.4f" );

    m_TrimGroup.ForceNewLine();

    m_TrimGroup.SetFitWidthFlag( false );

    m_TrimGroup.SetButtonWidth( blankW );
    m_TrimGroup.AddButton( m_UminLTrimButton, "" );
    m_TrimGroup.SetButtonWidth( toggleW );
    m_TrimGroup.AddButton( m_L01MinTrimToggle, "01" );
    m_TrimGroup.AddButton( m_L0LenMinTrimToggle, "0D" );

    m_TrimGroup.SetFitWidthFlag( true );

    m_TrimGroup.SetButtonWidth( buttonSMW );
    m_TrimGroup.AddSlider( m_LTrimMinSlider, "L Min", 1.0, "%5.4f" );


    m_UMinTrimToggleGroup.Init( this );
    m_UMinTrimToggleGroup.AddButton( m_UMinUTrimButton.GetFlButton() );
    m_UMinTrimToggleGroup.AddButton( m_UminLTrimButton.GetFlButton() );

    m_LMinScaleTrimToggleGroup.Init( this );
    m_LMinScaleTrimToggleGroup.AddButton( m_L0LenMinTrimToggle.GetFlButton() ); // 0 false added first
    m_LMinScaleTrimToggleGroup.AddButton( m_L01MinTrimToggle.GetFlButton() ); // 1 true added second

    m_TrimGroup.ForceNewLine();

    m_TrimGroup.SetSameLineFlag( true );
    m_TrimGroup.SetFitWidthFlag( false );
    m_TrimGroup.SetButtonWidth( blankW );

    m_TrimGroup.AddButton( m_UMaxUTrimButton, "" );

    m_TrimGroup.SetFitWidthFlag( true );

    m_TrimGroup.SetButtonWidth( buttonLGW );
    m_TrimGroup.AddSlider( m_UTrimMaxSlider, "U Max", 1.0, "%5.4f" );

    m_TrimGroup.ForceNewLine();

    m_TrimGroup.SetFitWidthFlag( false );

    m_TrimGroup.SetButtonWidth( blankW );
    m_TrimGroup.AddButton( m_UmaxLTrimButton, "" );
    m_TrimGroup.SetButtonWidth( toggleW );
    m_TrimGroup.AddButton( m_L01MaxTrimToggle, "01" );
    m_TrimGroup.AddButton( m_L0LenMaxTrimToggle, "0D" );

    m_TrimGroup.SetFitWidthFlag( true );

    m_TrimGroup.SetButtonWidth( buttonSMW );
    m_TrimGroup.AddSlider( m_LTrimMaxSlider, "L Max", 1.0, "%5.4f" );


    m_UMaxTrimToggleGroup.Init( this );
    m_UMaxTrimToggleGroup.AddButton( m_UMaxUTrimButton.GetFlButton() );
    m_UMaxTrimToggleGroup.AddButton( m_UmaxLTrimButton.GetFlButton() );

    m_LMaxScaleTrimToggleGroup.Init( this );
    m_LMaxScaleTrimToggleGroup.AddButton( m_L0LenMaxTrimToggle.GetFlButton() ); // 0 false added first
    m_LMaxScaleTrimToggleGroup.AddButton( m_L01MaxTrimToggle.GetFlButton() ); // 1 true added second

    m_TrimGroup.ForceNewLine();

    m_TrimGroup.SetFitWidthFlag( true );
    m_TrimGroup.SetSameLineFlag( false );
    m_TrimGroup.SetButtonWidth( buttonW );


    toggleW = 20;

    m_TrimGroup.AddYGap();

    m_TrimGroup.AddDividerBox( "U Trim Tip Treatment" );

    m_NoseCapTypeChoice.AddItem( "None", vsp::NO_END_CAP );
    m_NoseCapTypeChoice.AddItem( "Flat", vsp::FLAT_END_CAP );
    m_NoseCapTypeChoice.AddItem( "Round", vsp::ROUND_END_CAP );
    m_NoseCapTypeChoice.AddItem( "Edge", vsp::EDGE_END_CAP );
    m_NoseCapTypeChoice.AddItem( "Sharp", vsp::SHARP_END_CAP );
    m_TrimGroup.AddChoice( m_NoseCapTypeChoice, "Nose Cap Type" );

    m_TrimGroup.AddSlider( m_NoseCapLenSlider, "Length", 1, "%6.5f" );
    m_TrimGroup.AddSlider( m_NoseCapOffsetSlider, "Offset", 1, "%6.5f" );
    m_TrimGroup.AddSlider( m_NoseCapStrengthSlider, "Strength", 1, "%6.5f" );
    m_TrimGroup.AddButton( m_NoseCapSweepFlagButton, "Sweep Stretch" );

    m_TrimGroup.AddYGap();

    m_TailCapTypeChoice.AddItem( "None", vsp::NO_END_CAP );
    m_TailCapTypeChoice.AddItem( "Flat", vsp::FLAT_END_CAP );
    m_TailCapTypeChoice.AddItem( "Round", vsp::ROUND_END_CAP );
    m_TailCapTypeChoice.AddItem( "Edge", vsp::EDGE_END_CAP );
    m_TailCapTypeChoice.AddItem( "Sharp", vsp::SHARP_END_CAP );
    m_TrimGroup.AddChoice( m_TailCapTypeChoice, "Tail Cap Type" );

    m_TrimGroup.AddSlider( m_TailCapLenSlider, "Length", 1, "%6.5f" );
    m_TrimGroup.AddSlider( m_TailCapOffsetSlider, "Offset", 1, "%6.5f" );
    m_TrimGroup.AddSlider( m_TailCapStrengthSlider, "Strength", 1, "%6.5f" );
    m_TrimGroup.AddButton( m_TailCapSweepFlagButton, "Sweep Stretch" );

    m_TrimGroup.AddYGap();

    m_TrimGroup.AddDividerBox( "V Trim" );

    m_TrimGroup.SetSameLineFlag( true );
    m_TrimGroup.SetFitWidthFlag( false );
    m_TrimGroup.SetButtonWidth( toggleW );
    m_TrimGroup.AddButton( m_V1TrimToggle, "" );
    m_TrimGroup.SetFitWidthFlag( true );
    m_TrimGroup.SetButtonWidth( buttonW - toggleW );
    m_TrimGroup.AddSlider( m_V1TrimMinSlider, "V1 Begin", 1.0, "%5.4f" );
    m_TrimGroup.ForceNewLine();
    m_TrimGroup.SetSameLineFlag( false );
    m_TrimGroup.SetButtonWidth( buttonW );
    m_TrimGroup.AddSlider( m_V1TrimMaxSlider, "V1 End", 1.0, "%5.4f" );
    m_TrimGroup.AddYGap();

    m_TrimGroup.SetSameLineFlag( true );
    m_TrimGroup.SetFitWidthFlag( false );
    m_TrimGroup.SetButtonWidth( toggleW );
    m_TrimGroup.AddButton( m_V2TrimToggle, "" );
    m_TrimGroup.SetFitWidthFlag( true );
    m_TrimGroup.SetButtonWidth( buttonW - toggleW );
    m_TrimGroup.AddSlider( m_V2TrimMinSlider, "V2 Begin", 1.0, "%5.4f" );
    m_TrimGroup.ForceNewLine();
    m_TrimGroup.SetSameLineFlag( false );
    m_TrimGroup.SetButtonWidth( buttonW );
    m_TrimGroup.AddSlider( m_V2TrimMaxSlider, "V2 End", 1.0, "%5.4f" );
    m_TrimGroup.AddYGap();

    int offset_y = m_TrimGroup.GetY() - start_y;

    m_DesignLayout.AddY( offset_y );

    m_DesignLayout.AddSubGroupLayout( m_WingGroup, m_DesignLayout.GetW(), 200 );
    m_WingGroup.AddYGap();
    m_WingGroup.AddDividerBox( "Wing Trim" );

    //m_WingGroup.AddButton( m_SpanTrimToggle, "Trim Span" );
    //m_WingGroup.AddSlider( m_SpanTrimMinSlider, "Span Min", 1.0, "%5.4f" );
    //m_WingGroup.AddSlider( m_SpanTrimMaxSlider, "Span Max", 1.0, "%5.4f" );
    //m_WingGroup.AddYGap();

    m_WingGroup.SetSameLineFlag( true );
    m_WingGroup.SetFitWidthFlag( false );
    m_WingGroup.SetButtonWidth( toggleW );
    m_WingGroup.AddButton( m_ChordTrimToggle, "" );
    m_WingGroup.SetFitWidthFlag( true );
    m_WingGroup.SetButtonWidth( buttonW - toggleW );
    m_WingGroup.AddSlider( m_ChordTrimMinSlider, "Chord Min", 1.0, "%5.4f" );
    m_WingGroup.ForceNewLine();
    m_WingGroup.SetSameLineFlag( false );
    m_WingGroup.SetButtonWidth( buttonW );
    m_WingGroup.AddSlider( m_ChordTrimMaxSlider, "Chord Max", 1.0, "%5.4f" );
    m_WingGroup.AddYGap();

    m_WingGroup.SetFitWidthFlag( false );
    m_WingGroup.SetSameLineFlag( true );

    m_WingGroup.SetButtonWidth( toggleW );
    m_WingGroup.AddButton( m_UMinEtaTrimButton, "" );
    m_UMinTrimToggleGroup.AddButton( m_UMinEtaTrimButton.GetFlButton() );

    m_WingGroup.SetFitWidthFlag( true );

    m_WingGroup.SetButtonWidth( buttonW - toggleW );
    m_WingGroup.AddSlider( m_EtaTrimMinSlider, "Eta Min", 1.0, "%5.4f" );

    m_WingGroup.SetFitWidthFlag( false );
    m_WingGroup.ForceNewLine();

    m_WingGroup.SetButtonWidth( toggleW );
    m_WingGroup.AddButton( m_UmaxEtaTrimButton, "" );
    m_UMaxTrimToggleGroup.AddButton( m_UmaxEtaTrimButton.GetFlButton() );

    m_WingGroup.SetFitWidthFlag( true );

    m_WingGroup.SetButtonWidth( buttonW - toggleW );
    m_WingGroup.AddSlider( m_EtaTrimMaxSlider, "Eta Max", 1.0, "%5.4f" );


    m_DesignLayout.AddSubGroupLayout( m_SideGroup, m_DesignLayout.GetW(), 200 );
    m_SideGroup.AddYGap();
    m_SideGroup.AddDividerBox( "Side Trim" );

    m_SideGroup.SetSameLineFlag( true );
    m_SideGroup.SetFitWidthFlag( false );
    m_SideGroup.SetButtonWidth( toggleW );
    m_SideGroup.AddButton( m_Side1TrimToggle, "" );
    m_SideGroup.SetFitWidthFlag( true );
    m_SideGroup.SetButtonWidth( buttonW - toggleW );
    m_SideGroup.AddSlider( m_Side1TrimSlider, "Side 1", 1.0, "%5.4f" );
    m_SideGroup.ForceNewLine();
    m_SideGroup.AddYGap();

    m_SideGroup.SetSameLineFlag( true );
    m_SideGroup.SetFitWidthFlag( false );
    m_SideGroup.SetButtonWidth( toggleW );
    m_SideGroup.AddButton( m_Side2TrimToggle, "" );
    m_SideGroup.SetFitWidthFlag( true );
    m_SideGroup.SetButtonWidth( buttonW - toggleW );
    m_SideGroup.AddSlider( m_Side2TrimSlider, "Side 2", 1.0, "%5.4f" );
    m_SideGroup.ForceNewLine();
    m_SideGroup.AddYGap();

    m_SideGroup.SetSameLineFlag( true );
    m_SideGroup.SetFitWidthFlag( false );
    m_SideGroup.SetButtonWidth( toggleW );
    m_SideGroup.AddButton( m_Side3TrimToggle, "" );
    m_SideGroup.SetFitWidthFlag( true );
    m_SideGroup.SetButtonWidth( buttonW - toggleW );
    m_SideGroup.AddSlider( m_Side3TrimSlider, "Side 3", 1.0, "%5.4f" );
    m_SideGroup.ForceNewLine();
    m_SideGroup.AddYGap();

    m_SideGroup.SetSameLineFlag( true );
    m_SideGroup.SetFitWidthFlag( false );
    m_SideGroup.SetButtonWidth( toggleW );
    m_SideGroup.AddButton( m_Side4TrimToggle, "" );
    m_SideGroup.SetFitWidthFlag( true );
    m_SideGroup.SetButtonWidth( buttonW - toggleW );
    m_SideGroup.AddSlider( m_Side4TrimSlider, "Side 4", 1.0, "%5.4f" );
    m_SideGroup.ForceNewLine();
    m_SideGroup.AddYGap();
}


//==== Show Pod Screen ====//
void ConformalScreen::Show()
{
    if ( Update() )
    {
        GeomScreen::Show();
    }
}

//==== Update Pod Screen ====//
bool ConformalScreen::Update()
{
    assert( m_ScreenMgr );

    Geom* geom_ptr = m_ScreenMgr->GetCurrGeom();
    if ( !geom_ptr || geom_ptr->GetType().m_Type != CONFORMAL_GEOM_TYPE )
    {
        Hide();
        return false;
    }

    GeomScreen::Update();

    m_SymmLayout.GetGroup()->deactivate();
    m_AttachLayout.GetGroup()->deactivate();

    //==== Update Conformal Specific Parms ====//
    ConformalGeom* conformal_ptr = dynamic_cast< ConformalGeom* >( geom_ptr );
    assert( conformal_ptr );

    m_OffsetSlider.Update( conformal_ptr->m_Offset.GetID() );

    m_OffsetEndsToggle.Update( conformal_ptr->m_OffsetEnds.GetID() );

    m_UTrimToggle.Update( conformal_ptr->m_UTrimFlag.GetID() );
    m_UTrimMinSlider.Update( conformal_ptr->m_UTrimMin.GetID() );
    m_UTrimMaxSlider.Update( conformal_ptr->m_UTrimMax.GetID() );

    m_EtaTrimMinSlider.Update( conformal_ptr->m_EtaTrimMin.GetID() );
    m_EtaTrimMaxSlider.Update( conformal_ptr->m_EtaTrimMax.GetID() );

    m_UMinTrimToggleGroup.Update( conformal_ptr->m_UMinTrimTypeFlag.GetID() );
    m_LMinScaleTrimToggleGroup.Update( conformal_ptr->m_L01Min.GetID() );

    if ( conformal_ptr->m_L01Min() )
    {
        m_LTrimMinSlider.Update( 1, conformal_ptr->m_LTrimMin.GetID(), conformal_ptr->m_L0LenTrimMin.GetID());
    }
    else
    {
        m_LTrimMinSlider.Update( 2, conformal_ptr->m_LTrimMin.GetID(), conformal_ptr->m_L0LenTrimMin.GetID());
    }

    m_UMaxTrimToggleGroup.Update( conformal_ptr->m_UMaxTrimTypeFlag.GetID() );
    m_LMaxScaleTrimToggleGroup.Update( conformal_ptr->m_L01Max.GetID() );

    if ( conformal_ptr->m_L01Max() )
    {
        m_LTrimMaxSlider.Update( 1, conformal_ptr->m_LTrimMax.GetID(), conformal_ptr->m_L0LenTrimMax.GetID());
    }
    else
    {
        m_LTrimMaxSlider.Update( 2, conformal_ptr->m_LTrimMax.GetID(), conformal_ptr->m_L0LenTrimMax.GetID());
    }

    m_NoseCapTypeChoice.Update( conformal_ptr->m_CapUMinTrimOption.GetID() );
    m_TailCapTypeChoice.Update( conformal_ptr->m_CapUMaxTrimOption.GetID() );

    m_NoseCapLenSlider.Update( conformal_ptr->m_CapUMinTrimLength.GetID() );
    m_NoseCapOffsetSlider.Update( conformal_ptr->m_CapUMinTrimOffset.GetID() );
    m_NoseCapStrengthSlider.Update( conformal_ptr->m_CapUMinTrimStrength.GetID() );
    m_NoseCapSweepFlagButton.Update( conformal_ptr->m_CapUMinTrimSweepFlag.GetID() );

    m_NoseCapLenSlider.Deactivate();
    m_NoseCapOffsetSlider.Deactivate();
    m_NoseCapStrengthSlider.Deactivate();
    m_NoseCapSweepFlagButton.Deactivate();

    switch( conformal_ptr->m_CapUMinOption() ){
        case vsp::NO_END_CAP:
            break;
        case vsp::FLAT_END_CAP:
            break;
        case vsp::ROUND_END_CAP:
            m_NoseCapLenSlider.Activate();
            m_NoseCapOffsetSlider.Activate();
            m_NoseCapSweepFlagButton.Activate();
            break;
        case vsp::EDGE_END_CAP:
            m_NoseCapLenSlider.Activate();
            m_NoseCapOffsetSlider.Activate();
            m_NoseCapSweepFlagButton.Activate();
            break;
        case vsp::SHARP_END_CAP:
            m_NoseCapLenSlider.Activate();
            m_NoseCapOffsetSlider.Activate();
            m_NoseCapStrengthSlider.Activate();
            m_NoseCapSweepFlagButton.Activate();
            break;
    }

    m_TailCapLenSlider.Update( conformal_ptr->m_CapUMaxTrimLength.GetID() );
    m_TailCapOffsetSlider.Update( conformal_ptr->m_CapUMaxTrimOffset.GetID() );
    m_TailCapStrengthSlider.Update( conformal_ptr->m_CapUMaxTrimStrength.GetID() );
    m_TailCapSweepFlagButton.Update( conformal_ptr->m_CapUMaxTrimSweepFlag.GetID() );

    m_TailCapLenSlider.Deactivate();
    m_TailCapOffsetSlider.Deactivate();
    m_TailCapStrengthSlider.Deactivate();
    m_TailCapSweepFlagButton.Deactivate();

    switch( conformal_ptr->m_CapUMaxOption() ){
        case vsp::NO_END_CAP:
            break;
        case vsp::FLAT_END_CAP:
            break;
        case vsp::ROUND_END_CAP:
            m_TailCapLenSlider.Activate();
            m_TailCapOffsetSlider.Activate();
            m_TailCapSweepFlagButton.Activate();
            break;
        case vsp::EDGE_END_CAP:
            m_TailCapLenSlider.Activate();
            m_TailCapOffsetSlider.Activate();
            m_TailCapSweepFlagButton.Activate();
            break;
        case vsp::SHARP_END_CAP:
            m_TailCapLenSlider.Activate();
            m_TailCapOffsetSlider.Activate();
            m_TailCapStrengthSlider.Activate();
            m_TailCapSweepFlagButton.Activate();
            break;
    }

    if ( conformal_ptr->m_UTrimFlag() )
    {
        m_UMinTrimToggleGroup.Activate();

        if ( conformal_ptr->m_UMinTrimTypeFlag() == vsp::U_TRIM ) // Trim based on U
        {
            m_UTrimMinSlider.Activate();

            m_LMinScaleTrimToggleGroup.Deactivate();
            m_LTrimMinSlider.Deactivate();

            m_EtaTrimMinSlider.Deactivate();
        }
        else if ( conformal_ptr->m_UMinTrimTypeFlag() == vsp::L_TRIM ) // Trim based on L
        {
            m_UTrimMinSlider.Deactivate();

            m_LMinScaleTrimToggleGroup.Activate();
            m_LTrimMinSlider.Activate();

            m_EtaTrimMinSlider.Deactivate();

            if ( conformal_ptr->m_L01Min() )
            {
                m_LTrimMinSlider.ActivateInput1();
            }
            else
            {
                m_LTrimMinSlider.ActivateInput2();
            }
        }
        else // Trim based on eta
        {
            m_UTrimMinSlider.Deactivate();

            m_LMinScaleTrimToggleGroup.Deactivate();
            m_LTrimMinSlider.Deactivate();

            m_EtaTrimMinSlider.Activate();
        }


        m_UMaxTrimToggleGroup.Activate();

        if ( conformal_ptr->m_UMaxTrimTypeFlag() == vsp::U_TRIM ) // Trim based on U
        {
            m_UTrimMaxSlider.Activate();

            m_LMaxScaleTrimToggleGroup.Deactivate();
            m_LTrimMaxSlider.Deactivate();

            m_EtaTrimMaxSlider.Deactivate();
        }
        else if ( conformal_ptr->m_UMaxTrimTypeFlag() == vsp::L_TRIM ) // Trim based on L
        {
            m_UTrimMaxSlider.Deactivate();

            m_LMaxScaleTrimToggleGroup.Activate();
            m_LTrimMaxSlider.Activate();

            m_EtaTrimMaxSlider.Deactivate();

            if ( conformal_ptr->m_L01Max() )
            {
                m_LTrimMaxSlider.ActivateInput1();
            }
            else
            {
                m_LTrimMaxSlider.ActivateInput2();
            }
        }
        else // Trim based on eta
        {
            m_UTrimMaxSlider.Deactivate();

            m_LMaxScaleTrimToggleGroup.Deactivate();
            m_LTrimMaxSlider.Deactivate();

            m_EtaTrimMaxSlider.Activate();
        }

        m_NoseCapTypeChoice.Activate();
        m_NoseCapLenSlider.Activate();
        m_NoseCapOffsetSlider.Activate();
        m_NoseCapStrengthSlider.Activate();
        m_NoseCapSweepFlagButton.Activate();

        m_TailCapTypeChoice.Activate();
        m_TailCapLenSlider.Activate();
        m_TailCapOffsetSlider.Activate();
        m_TailCapStrengthSlider.Activate();
        m_TailCapSweepFlagButton.Activate();
    }
    else
    {
        m_UTrimMinSlider.Deactivate();
        m_UTrimMaxSlider.Deactivate();

        m_UMinTrimToggleGroup.Deactivate();
        m_LMinScaleTrimToggleGroup.Deactivate();
        m_LTrimMinSlider.Deactivate();

        m_UMaxTrimToggleGroup.Deactivate();
        m_LMaxScaleTrimToggleGroup.Deactivate();
        m_LTrimMaxSlider.Deactivate();

        m_EtaTrimMinSlider.Deactivate();
        m_EtaTrimMaxSlider.Deactivate();

        m_NoseCapTypeChoice.Deactivate();
        m_NoseCapLenSlider.Deactivate();
        m_NoseCapOffsetSlider.Deactivate();
        m_NoseCapStrengthSlider.Deactivate();
        m_NoseCapSweepFlagButton.Deactivate();

        m_TailCapTypeChoice.Deactivate();
        m_TailCapLenSlider.Deactivate();
        m_TailCapOffsetSlider.Deactivate();
        m_TailCapStrengthSlider.Deactivate();
        m_TailCapSweepFlagButton.Deactivate();
    }

    m_V1TrimToggle.Update( conformal_ptr->m_V1TrimFlag.GetID() );
    m_V1TrimMinSlider.Update( conformal_ptr->m_V1TrimBegin.GetID() );
    m_V1TrimMaxSlider.Update( conformal_ptr->m_V1TrimEnd.GetID() );

    if ( conformal_ptr->m_V1TrimFlag() )
    {
        m_V1TrimMinSlider.Activate();
        m_V1TrimMaxSlider.Activate();
    }
    else
    {
        m_V1TrimMinSlider.Deactivate();
        m_V1TrimMaxSlider.Deactivate();
    }

    m_V2TrimToggle.Update( conformal_ptr->m_V2TrimFlag.GetID() );
    m_V2TrimMinSlider.Update( conformal_ptr->m_V2TrimBegin.GetID() );
    m_V2TrimMaxSlider.Update( conformal_ptr->m_V2TrimEnd.GetID() );

    if ( conformal_ptr->m_V2TrimFlag() )
    {
        m_V2TrimMinSlider.Activate();
        m_V2TrimMaxSlider.Activate();
    }
    else
    {
        m_V2TrimMinSlider.Deactivate();
        m_V2TrimMaxSlider.Deactivate();
    }

    if ( !conformal_ptr->IsWingParent() )
    {
        m_WingGroup.Hide();
        m_SideGroup.Show();

        m_Side1TrimToggle.Update( conformal_ptr->m_Side1TrimFlag.GetID() );
        m_Side1TrimSlider.Update( conformal_ptr->m_Side1Trim.GetID() );

        m_Side2TrimToggle.Update( conformal_ptr->m_Side2TrimFlag.GetID() );
        m_Side2TrimSlider.Update( conformal_ptr->m_Side2Trim.GetID() );

        m_Side3TrimToggle.Update( conformal_ptr->m_Side3TrimFlag.GetID() );
        m_Side3TrimSlider.Update( conformal_ptr->m_Side3Trim.GetID() );

        m_Side4TrimToggle.Update( conformal_ptr->m_Side4TrimFlag.GetID() );
        m_Side4TrimSlider.Update( conformal_ptr->m_Side4Trim.GetID() );

        if ( conformal_ptr->m_Side1TrimFlag() )
        {
            m_Side1TrimSlider.Activate();
        }
        else
        {
            m_Side1TrimSlider.Deactivate();
        }

        if ( conformal_ptr->m_Side2TrimFlag() )
        {
            m_Side2TrimSlider.Activate();
        }
        else
        {
            m_Side2TrimSlider.Deactivate();
        }

        if ( conformal_ptr->m_Side3TrimFlag() )
        {
            m_Side3TrimSlider.Activate();
        }
        else
        {
            m_Side3TrimSlider.Deactivate();
        }

        if ( conformal_ptr->m_Side4TrimFlag() )
        {
            m_Side4TrimSlider.Activate();
        }
        else
        {
            m_Side4TrimSlider.Deactivate();
        }
    }
    else
    {
        m_WingGroup.Show();
        m_SideGroup.Hide();

        m_ChordTrimToggle.Update( conformal_ptr->m_ChordTrimFlag.GetID() );
        m_ChordTrimMinSlider.Update( conformal_ptr->m_ChordTrimMin.GetID() );
        m_ChordTrimMaxSlider.Update( conformal_ptr->m_ChordTrimMax.GetID() );

        if ( conformal_ptr->m_ChordTrimFlag() )
        {
            m_ChordTrimMinSlider.Activate();
            m_ChordTrimMaxSlider.Activate();

            m_V1TrimToggle.Deactivate();
            m_V1TrimMinSlider.Deactivate();
            m_V1TrimMaxSlider.Deactivate();
            m_V2TrimToggle.Deactivate();
            m_V2TrimMinSlider.Deactivate();
            m_V2TrimMaxSlider.Deactivate();
        }
        else
        {
            m_ChordTrimMinSlider.Deactivate();
            m_ChordTrimMaxSlider.Deactivate();
        }
    }

    return true;
}


//==== Non Menu Callbacks ====//
void ConformalScreen::CallBack( Fl_Widget *w )
{
    GeomScreen::CallBack( w );
}






