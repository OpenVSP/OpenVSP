//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//
//
//////////////////////////////////////////////////////////////////////

#include "StackScreen.h"
#include "ScreenMgr.h"
#include "StackGeom.h"

using namespace vsp;

//==== Constructor ====//
StackScreen::StackScreen( ScreenMgr* mgr ) : SkinScreen( mgr, 400, 640+75, "Stack" )
{
    m_CurrDisplayGroup = NULL;

    Fl_Group* design_tab = AddTab( "Design", 3 );

    Fl_Group* design_group = AddSubGroup( design_tab, 5 );

    m_DesignLayout.SetGroupAndScreen( design_group, this );
    m_DesignLayout.AddDividerBox( "Design" );

    m_DesignLayout.AddYGap();
    m_DesignLayout.AddDividerBox( "Design Policy" );
    m_DesignPolicyChoice.AddItem( "FREE" );
    m_DesignPolicyChoice.AddItem( "LOOP" );
    m_DesignLayout.AddChoice( m_DesignPolicyChoice, "XSec Order: " );

    m_DesignLayout.AddYGap();
    m_DesignLayout.AddDividerBox( "Tip Treatment" );

    m_NoseCapTypeChoice.AddItem("None");
    m_NoseCapTypeChoice.AddItem("Flat");
    m_NoseCapTypeChoice.AddItem("Round");
    m_NoseCapTypeChoice.AddItem("Edge");
    m_NoseCapTypeChoice.AddItem("Sharp");
    m_DesignLayout.AddChoice(m_NoseCapTypeChoice, "Nose Cap Type");

    m_DesignLayout.AddSlider( m_NoseCapLenSlider, "Length", 1, "%6.5f" );
    m_DesignLayout.AddSlider( m_NoseCapOffsetSlider, "Offset", 1, "%6.5f" );
    m_DesignLayout.AddSlider( m_NoseCapStrengthSlider, "Strength", 1, "%6.5f" );
    m_DesignLayout.AddButton( m_NoseCapSweepFlagButton, "Sweep Stretch" );

    m_DesignLayout.AddYGap();

    m_TailCapTypeChoice.AddItem("None");
    m_TailCapTypeChoice.AddItem("Flat");
    m_TailCapTypeChoice.AddItem("Round");
    m_TailCapTypeChoice.AddItem("Edge");
    m_TailCapTypeChoice.AddItem("Sharp");
    m_DesignLayout.AddChoice(m_TailCapTypeChoice, "Tail Cap Type");

    m_DesignLayout.AddSlider( m_TailCapLenSlider, "Length", 1, "%6.5f" );
    m_DesignLayout.AddSlider( m_TailCapOffsetSlider, "Offset", 1, "%6.5f" );
    m_DesignLayout.AddSlider( m_TailCapStrengthSlider, "Strength", 1, "%6.5f" );
    m_DesignLayout.AddButton( m_TailCapSweepFlagButton, "Sweep Stretch" );

    m_DesignLayout.AddYGap();

    m_DesignLayout.AddSlider( m_CapTessSlider, "Cap Tess", 10, "%3.0f" );

    // Xsec layout after index selector has been defined in base class
    m_XSecLayout.SetFitWidthFlag( false );
    m_XSecLayout.SetSameLineFlag( true );
    m_XSecLayout.SetButtonWidth( ( m_XFormLayout.GetRemainX() - 30 ) / 4 );
    m_XSecLayout.AddButton( m_InsertXSec, "Insert" );
    m_XSecLayout.AddX( 10 );
    m_XSecLayout.AddButton( m_CutXSec, "Cut" );
    m_XSecLayout.AddX( 10 );
    m_XSecLayout.AddButton( m_CopyXSec, "Copy" );
    m_XSecLayout.AddX( 10 );
    m_XSecLayout.AddButton( m_PasteXSec, "Paste" );
    m_XSecLayout.ForceNewLine();
    m_XSecLayout.AddYGap();

    m_XSecLayout.SetFitWidthFlag( true );
    m_XSecLayout.SetSameLineFlag( false );

    m_XSecLayout.AddYGap();
    int tess_w = m_XSecLayout.GetButtonWidth();
    m_XSecLayout.AddSlider( m_SectUTessSlider, "Num U", 20, " %5.0f" );

    m_XSecLayout.SetButtonWidth( 50 );
    m_XSecLayout.AddSlider( m_XSecXDeltaSlider, "Delta X", 10.0, "%6.5f" );
    m_XSecLayout.AddSlider( m_XSecYDeltaSlider, "Delta Y", 10.0, "%6.5f" );
    m_XSecLayout.AddSlider( m_XSecZDeltaSlider, "Delta Z", 10.0, "%6.5f" );
    m_XSecLayout.AddYGap();

    m_XSecLayout.InitWidthHeightVals();
    m_XSecLayout.AddSlider( m_XSecXRotSlider, "Rot X", 90.0, "%6.5f" );
    m_XSecLayout.AddSlider( m_XSecYRotSlider, "Rot Y", 90.0, "%6.5f" );
    m_XSecLayout.AddSlider( m_XSecZRotSlider, "Rot Z", 90.0, "%6.5f" );
    m_XSecLayout.AddSlider( m_XSecSpinSlider, "Spin",  2.0, "%6.5f" );
    m_XSecLayout.AddYGap();

    AddXSecLayout();

    //==== XSec Modifications ====//

    Fl_Group* modify_tab = AddTab( "Modify" );
    Fl_Group* modify_group = AddSubGroup( modify_tab, 5 );

    m_ModifyLayout.SetButtonWidth( 70 );

    m_ModifyLayout.SetGroupAndScreen( modify_group, this );
    m_ModifyLayout.AddDividerBox( "XSec" );

    m_ModifyLayout.AddIndexSelector( m_XsecModIndexSelector );

    m_ModifyLayout.AddYGap();

    m_ModifyLayout.InitWidthHeightVals();
    m_ModifyLayout.SetChoiceButtonWidth( m_ModifyLayout.GetButtonWidth() );

    m_ModifyLayout.AddYGap();
    m_ModifyLayout.AddDividerBox( "Chevron" );

    m_ChevronModeChoice.AddItem( "NONE", vsp::CHEVRON_NONE );
    m_ChevronModeChoice.AddItem( "PARTIAL", vsp::CHEVRON_PARTIAL );
    m_ChevronModeChoice.AddItem( "FULL", vsp::CHEVRON_FULL );
    m_ModifyLayout.AddChoice( m_ChevronModeChoice, "Type:" );

    m_ModifyLayout.AddSlider( m_ChevNumberSlider, "Number", 10, " %5.0f" );

    m_ModifyLayout.AddYGap();
    m_ModifyLayout.AddDividerBox( "Corner Rounding Radius" );

    m_ModifyLayout.SetButtonWidth( m_ModifyLayout.GetButtonWidth() * 0.6 );

    m_ModifyLayout.SetSameLineFlag( true );
    m_ModifyLayout.AddSlider( m_ChevPeakRadSlider, "Peak", 1.0, "%6.5f", m_ModifyLayout.GetW() * 0.5 + 5 );
    m_ModifyLayout.AddX( 5 );
    m_ModifyLayout.AddSlider( m_ChevValleyRadSlider, "Valley", 1.0, "%6.5f" );
    m_ModifyLayout.ForceNewLine();
    m_ModifyLayout.SetSameLineFlag( false );

    m_ModifyLayout.AddYGap();
    m_ModifyLayout.AddDividerBox( "Waveform" );

    m_ModifyLayout.SetSameLineFlag( true );
    m_ModifyLayout.AddSlider( m_ChevOnDutySlider, "\% On", 1, "%6.5f", m_ModifyLayout.GetW() * 0.5 + 5 );
    m_ModifyLayout.AddX( 5 );
    m_ModifyLayout.AddSlider( m_ChevOffDutySlider, "\% Off", 1, "%6.5f" );
    m_ModifyLayout.ForceNewLine();
    m_ModifyLayout.SetSameLineFlag( false );

    m_ModifyLayout.InitWidthHeightVals();
    m_ModifyLayout.SetChoiceButtonWidth( m_ModifyLayout.GetButtonWidth() );

    m_ModifyLayout.AddYGap();
    m_ModifyLayout.AddDividerBox( "Extents" );

    m_ChevronExtentModeChoice.AddItem( "Start \\/ End", vsp::CHEVRON_W01_SE );
    m_ChevronExtentModeChoice.AddItem( "Center \\/ Width", vsp::CHEVRON_W01_CW );
    m_ModifyLayout.AddChoice( m_ChevronExtentModeChoice, "Mode:" );

    m_ModifyLayout.AddSlider( m_ChevW01StartSlider, "W Start", 1, "%6.5f" );
    m_ModifyLayout.AddSlider( m_ChevW01EndSlider, "W End", 1, "%6.5f" );
    m_ModifyLayout.AddSlider( m_ChevW01CenterSlider, "W Center", 1, "%6.5f" );
    m_ModifyLayout.AddSlider( m_ChevW01WidthSlider, "W Width", 1, "%6.5f" );

    m_ModifyLayout.AddYGap();

    m_ModifyLayout.SetDividerHeight( m_ModifyLayout.GetStdHeight() );

    m_ModifyLayout.SetSameLineFlag( true );
    m_ModifyLayout.AddDividerBox( "Top Side", m_ModifyLayout.GetButtonWidth() );
    m_ModifyLayout.SetFitWidthFlag( false );
    m_ModifyLayout.AddButton( m_ChevAngleAllSymButton, "All Sym" );
    m_ModifyLayout.ForceNewLine();
    m_ModifyLayout.SetFitWidthFlag( true );
    m_ModifyLayout.SetSameLineFlag( false );

    m_ModifyLayout.AddSlider( m_ChevTopAmpSlider, "Amplitude", 10, "%6.5f" );
    m_ModifyLayout.AddSlider( m_ChevDirTopAngleSlider, "Angle", 90.0, "%5.2f" );
    m_ModifyLayout.AddSlider( m_ChevDirTopSlewSlider, "Slew", 90.0, "%5.2f" );

    m_ModifyLayout.AddYGap();
    m_ModifyLayout.AddDividerBox( "Right Side" );

    m_ModifyLayout.AddSlider( m_ChevRightAmpSlider, "Amplitude", 10, "%6.5f" );
    m_ModifyLayout.AddSlider( m_ChevDirRightAngleSlider, "Angle", 90.0, "%5.2f" );
    m_ModifyLayout.AddSlider( m_ChevDirRightSlewSlider, "Slew", 90.0, "%5.2f" );

    m_ModifyLayout.AddYGap();
    m_ModifyLayout.SetSameLineFlag( true );
    m_ModifyLayout.AddDividerBox( "Bottom Side", m_ModifyLayout.GetButtonWidth() );
    m_ModifyLayout.SetFitWidthFlag( false );
    m_ModifyLayout.AddButton( m_ChevAngleTBSymButton, "T/B Sym" );
    m_ModifyLayout.ForceNewLine();
    m_ModifyLayout.SetFitWidthFlag( true );
    m_ModifyLayout.SetSameLineFlag( false );

    m_ModifyLayout.AddSlider( m_ChevBottomAmpSlider, "Amplitude", 10, "%6.5f" );
    m_ModifyLayout.AddSlider( m_ChevDirBottomAngleSlider, "Angle", 90.0, "%5.2f" );
    m_ModifyLayout.AddSlider( m_ChevDirBottomSlewSlider, "Slew", 90.0, "%5.2f" );

    m_ModifyLayout.AddYGap();
    m_ModifyLayout.SetSameLineFlag( true );
    m_ModifyLayout.AddDividerBox( "Left Side", m_ModifyLayout.GetButtonWidth() );
    m_ModifyLayout.SetFitWidthFlag( false );
    m_ModifyLayout.AddButton( m_ChevAngleRLSymButton, "R/L Sym" );
    m_ModifyLayout.ForceNewLine();
    m_ModifyLayout.SetFitWidthFlag( true );
    m_ModifyLayout.SetSameLineFlag( false );

    m_ModifyLayout.AddSlider( m_ChevLeftAmpSlider, "Amplitude", 10, "%6.5f" );
    m_ModifyLayout.AddSlider( m_ChevDirLeftAngleSlider, "Angle", 90.0, "%5.2f" );
    m_ModifyLayout.AddSlider( m_ChevDirLeftSlewSlider, "Slew", 90.0, "%5.2f" );

}

//==== Show Pod Screen ====//
StackScreen::~StackScreen()
{
}

//==== Show Pod Screen ====//
void StackScreen::Show()
{
    if ( Update() )
    {
        m_FLTK_Window->show();
    }
}

//==== Update Pod Screen ====//
bool StackScreen::Update()
{
    assert( m_ScreenMgr );

    Geom* geom_ptr = m_ScreenMgr->GetCurrGeom();
    if ( !geom_ptr || geom_ptr->GetType().m_Type != STACK_GEOM_TYPE )
    {
        Hide();
        return false;
    }

    SkinScreen::Update();
    m_NumUSlider.Deactivate();

    StackGeom* stackgeom_ptr = dynamic_cast< StackGeom* >( geom_ptr );
    assert( stackgeom_ptr );

    m_DesignPolicyChoice.Update( stackgeom_ptr->m_OrderPolicy.GetID() );

    m_NoseCapTypeChoice.Update( stackgeom_ptr->m_CapUMinOption.GetID() );
    m_TailCapTypeChoice.Update( stackgeom_ptr->m_CapUMaxOption.GetID() );

    m_CapTessSlider.Update( stackgeom_ptr->m_CapUMinTess.GetID() );

    m_NoseCapLenSlider.Update( stackgeom_ptr->m_CapUMinLength.GetID() );
    m_NoseCapOffsetSlider.Update( stackgeom_ptr->m_CapUMinOffset.GetID() );
    m_NoseCapStrengthSlider.Update( stackgeom_ptr->m_CapUMinStrength.GetID() );
    m_NoseCapSweepFlagButton.Update( stackgeom_ptr->m_CapUMinSweepFlag.GetID() );

    m_NoseCapLenSlider.Deactivate();
    m_NoseCapOffsetSlider.Deactivate();
    m_NoseCapStrengthSlider.Deactivate();
    m_NoseCapSweepFlagButton.Deactivate();

    switch( stackgeom_ptr->m_CapUMinOption() ){
        case NO_END_CAP:
            break;
        case FLAT_END_CAP:
            break;
        case ROUND_END_CAP:
            m_NoseCapLenSlider.Activate();
            m_NoseCapOffsetSlider.Activate();
            m_NoseCapSweepFlagButton.Activate();
            break;
        case EDGE_END_CAP:
            m_NoseCapLenSlider.Activate();
            m_NoseCapOffsetSlider.Activate();
            m_NoseCapSweepFlagButton.Activate();
            break;
        case SHARP_END_CAP:
            m_NoseCapLenSlider.Activate();
            m_NoseCapOffsetSlider.Activate();
            m_NoseCapStrengthSlider.Activate();
            m_NoseCapSweepFlagButton.Activate();
            break;
    }

    m_TailCapLenSlider.Update( stackgeom_ptr->m_CapUMaxLength.GetID() );
    m_TailCapOffsetSlider.Update( stackgeom_ptr->m_CapUMaxOffset.GetID() );
    m_TailCapStrengthSlider.Update( stackgeom_ptr->m_CapUMaxStrength.GetID() );
    m_TailCapSweepFlagButton.Update( stackgeom_ptr->m_CapUMaxSweepFlag.GetID() );

    m_TailCapLenSlider.Deactivate();
    m_TailCapOffsetSlider.Deactivate();
    m_TailCapStrengthSlider.Deactivate();
    m_TailCapSweepFlagButton.Deactivate();

    switch( stackgeom_ptr->m_CapUMaxOption() ){
        case NO_END_CAP:
            break;
        case FLAT_END_CAP:
            break;
        case ROUND_END_CAP:
            m_TailCapLenSlider.Activate();
            m_TailCapOffsetSlider.Activate();
            m_TailCapSweepFlagButton.Activate();
            break;
        case EDGE_END_CAP:
            m_TailCapLenSlider.Activate();
            m_TailCapOffsetSlider.Activate();
            m_TailCapSweepFlagButton.Activate();
            break;
        case SHARP_END_CAP:
            m_TailCapLenSlider.Activate();
            m_TailCapOffsetSlider.Activate();
            m_TailCapStrengthSlider.Activate();
            m_TailCapSweepFlagButton.Activate();
            break;
    }

    //==== XSec Index Display ===//
    int xsid = stackgeom_ptr->m_ActiveXSec();
    m_XSecIndexSelector.Update( stackgeom_ptr->m_ActiveXSec.GetID() );
    m_XsecModIndexSelector.Update( stackgeom_ptr->m_ActiveXSec.GetID() );

    StackXSec* xs = ( StackXSec* ) stackgeom_ptr->GetXSec( xsid );
    if ( xs )
    {
        bool firstxs = xsid == 0;
        bool lastxs = xsid == ( stackgeom_ptr->GetXSecSurf( 0 )->NumXSec() - 1 );
        bool nextlastxs = xsid == ( stackgeom_ptr->GetXSecSurf( 0 )->NumXSec() - 2 );

        m_SectUTessSlider.Update( xs->m_SectTessU.GetID() );
        m_XSecXDeltaSlider.Update( xs->m_XDelta.GetID() );
        m_XSecYDeltaSlider.Update( xs->m_YDelta.GetID() );
        m_XSecZDeltaSlider.Update( xs->m_ZDelta.GetID() );
        m_XSecXRotSlider.Update( xs->m_XRotate.GetID() );
        m_XSecYRotSlider.Update( xs->m_YRotate.GetID() );
        m_XSecZRotSlider.Update( xs->m_ZRotate.GetID() );
        m_XSecSpinSlider.Update( xs->m_Spin.GetID() );

        if ( firstxs )
        {
            m_SectUTessSlider.Deactivate();
            m_XSecXDeltaSlider.Deactivate();
            m_XSecYDeltaSlider.Deactivate();
            m_XSecZDeltaSlider.Deactivate();
            m_XSecXRotSlider.Deactivate();
            m_XSecYRotSlider.Deactivate();
            m_XSecZRotSlider.Deactivate();
            m_XSecSpinSlider.Deactivate();
        }
        else
        {
            m_SectUTessSlider.Activate();
            m_XSecXDeltaSlider.Activate();
            m_XSecYDeltaSlider.Activate();
            m_XSecZDeltaSlider.Activate();
            m_XSecXRotSlider.Activate();
            m_XSecYRotSlider.Activate();
            m_XSecZRotSlider.Activate();
            m_XSecSpinSlider.Activate();
        }

        if ( lastxs && stackgeom_ptr->m_OrderPolicy() == StackGeom::STACK_LOOP)
        {
            m_XSecXDeltaSlider.Deactivate();
            m_XSecYDeltaSlider.Deactivate();
            m_XSecZDeltaSlider.Deactivate();
            m_XSecXRotSlider.Deactivate();
            m_XSecYRotSlider.Deactivate();
            m_XSecZRotSlider.Deactivate();
            m_XSecSpinSlider.Deactivate();
        }

        XSecCurve* xsc = xs->GetXSecCurve();
        if ( xsc )
        {
            // Special considerations for loop policy
            if ( lastxs && stackgeom_ptr->m_OrderPolicy() == StackGeom::STACK_LOOP )
            {
                m_XSecTypeChoice.Deactivate();
                m_ShowXSecButton.Deactivate();
                m_ConvertCEDITButton.Deactivate();
            }
            else
            {
                m_XSecTypeChoice.Activate();
                m_ShowXSecButton.Activate();
                m_ConvertCEDITButton.Activate();
            }

            if ( lastxs && stackgeom_ptr->m_OrderPolicy() == StackGeom::STACK_LOOP )
            {
                DisplayGroup ( NULL);
            }

            m_ChevronModeChoice.Update( xsc->m_ChevronType.GetID() );

            m_ChevTopAmpSlider.Update( xsc->m_ChevTopAmplitude.GetID() );
            m_ChevBottomAmpSlider.Update( xsc->m_ChevBottomAmplitude.GetID() );
            m_ChevLeftAmpSlider.Update( xsc->m_ChevLeftAmplitude.GetID() );
            m_ChevRightAmpSlider.Update( xsc->m_ChevRightAmplitude.GetID() );

            m_ChevNumberSlider.Update( xsc->m_ChevNumber.GetID() );

            m_ChevOnDutySlider.Update( xsc->m_ChevOnDuty.GetID() );
            m_ChevOffDutySlider.Update( xsc->m_ChevOffDuty.GetID() );

            m_ChevronExtentModeChoice.Update( xsc->m_ChevronExtentMode.GetID() );

            m_ChevW01StartSlider.Update( xsc->m_ChevW01Start.GetID() );
            m_ChevW01EndSlider.Update( xsc->m_ChevW01End.GetID() );
            m_ChevW01CenterSlider.Update( xsc->m_ChevW01Center.GetID() );
            m_ChevW01WidthSlider.Update( xsc->m_ChevW01Width.GetID() );

            m_ChevAngleAllSymButton.Update( xsc->m_ChevDirAngleAllSymFlag.GetID() );
            m_ChevAngleTBSymButton.Update( xsc->m_ChevDirAngleTBSymFlag.GetID() );
            m_ChevAngleRLSymButton.Update( xsc->m_ChevDirAngleRLSymFlag.GetID() );

            m_ChevDirTopAngleSlider.Update( xsc->m_ChevTopAngle.GetID() );
            m_ChevDirBottomAngleSlider.Update( xsc->m_ChevBottomAngle.GetID() );
            m_ChevDirRightAngleSlider.Update( xsc->m_ChevRightAngle.GetID() );
            m_ChevDirLeftAngleSlider.Update( xsc->m_ChevLeftAngle.GetID() );

            m_ChevDirTopSlewSlider.Update( xsc->m_ChevTopSlew.GetID() );
            m_ChevDirBottomSlewSlider.Update( xsc->m_ChevBottomSlew.GetID() );
            m_ChevDirRightSlewSlider.Update( xsc->m_ChevRightSlew.GetID() );
            m_ChevDirLeftSlewSlider.Update( xsc->m_ChevLeftSlew.GetID() );

            m_ChevValleyRadSlider.Update( xsc->m_ValleyRad.GetID() );
            m_ChevPeakRadSlider.Update( xsc->m_PeakRad.GetID() );
        }
    }
    return true;
}

void StackScreen::GuiDeviceCallBack( GuiDevice* gui_device )
{
    //==== Find Fuselage Ptr ====//
    Geom* geom_ptr = m_ScreenMgr->GetCurrGeom();
    if ( !geom_ptr || geom_ptr->GetType().m_Type != STACK_GEOM_TYPE )
    {
        return;
    }
    StackGeom* stackgeom_ptr = dynamic_cast< StackGeom* >( geom_ptr );
    assert( stackgeom_ptr );

    if ( gui_device == &m_CutXSec )
    {
        stackgeom_ptr->CutActiveXSec();
    }
    else if ( gui_device == &m_CopyXSec   )
    {
        stackgeom_ptr->CopyActiveXSec();
    }
    else if ( gui_device == &m_PasteXSec  )
    {
        stackgeom_ptr->PasteActiveXSec();
    }
    else if ( gui_device == &m_InsertXSec  )
    {
        stackgeom_ptr->InsertXSec( );
    }

    SkinScreen::GuiDeviceCallBack( gui_device );
}

//==== Fltk  Callbacks ====//
void StackScreen::CallBack( Fl_Widget *w )
{
    SkinScreen::CallBack( w );
}

