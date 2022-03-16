//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//
//
//////////////////////////////////////////////////////////////////////

#include "FuselageScreen.h"
#include "ScreenMgr.h"
#include "FuselageGeom.h"

using namespace vsp;



//==== Constructor ====//
FuselageScreen::FuselageScreen( ScreenMgr* mgr ) : ChevronScreen( mgr, 400, 715, "Fuselage" )
{
    m_CurrDisplayGroup = NULL;

    Fl_Group* design_tab = AddTab( "Design", 3 );

    Fl_Group* design_group = AddSubGroup( design_tab, 5 );

    m_DesignLayout.SetGroupAndScreen( design_group, this );
    m_DesignLayout.AddDividerBox( "Design" );
    m_DesignLayout.AddSlider( m_LengthSlider, "Length", 10, "%6.5f" );

    m_DesignLayout.AddYGap();
    m_DesignLayout.AddDividerBox( "Design Policy" );
    m_DesignPolicyChoice.AddItem( "MONOTONIC" );
    m_DesignPolicyChoice.AddItem( "LOOP" );
    m_DesignPolicyChoice.AddItem( "FREE" );
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
    m_XSecLayout.AddSlider( m_XSecXSlider, "X", 1.0, "%6.5f" );
    m_XSecLayout.AddSlider( m_XSecYSlider, "Y", 1.0, "%6.5f" );
    m_XSecLayout.AddSlider( m_XSecZSlider, "Z", 1.0, "%6.5f" );
    m_XSecLayout.AddYGap();

    m_XSecLayout.InitWidthHeightVals();
    m_XSecLayout.AddSlider( m_XSecXRotSlider, "Rot X", 90.0, "%6.5f" );
    m_XSecLayout.AddSlider( m_XSecYRotSlider, "Rot Y", 90.0, "%6.5f" );
    m_XSecLayout.AddSlider( m_XSecZRotSlider, "Rot Z", 90.0, "%6.5f" );
    m_XSecLayout.AddSlider( m_XSecSpinSlider, "Spin",  2.0, "%6.5f" );

    m_XSecLayout.AddYGap();

    AddXSecLayout();
}

//==== Show Pod Screen ====//
FuselageScreen::~FuselageScreen()
{
}

//==== Show Pod Screen ====//
void FuselageScreen::Show()
{
    if ( Update() )
    {
        m_FLTK_Window->show();
    }
}

//==== Update Pod Screen ====//
bool FuselageScreen::Update()
{
    assert( m_ScreenMgr );

    Geom* geom_ptr = m_ScreenMgr->GetCurrGeom();
    if ( !geom_ptr || geom_ptr->GetType().m_Type != FUSELAGE_GEOM_TYPE )
    {
        Hide();
        return false;
    }

    ChevronScreen::Update();
    m_NumUSlider.Deactivate();

    FuselageGeom* fuselage_ptr = dynamic_cast< FuselageGeom* >( geom_ptr );
    assert( fuselage_ptr );

    //==== Design ====//
    m_LengthSlider.Update( fuselage_ptr->m_Length.GetID() );

    m_DesignPolicyChoice.Update( fuselage_ptr->m_OrderPolicy.GetID() );


    m_NoseCapTypeChoice.Update( fuselage_ptr->m_CapUMinOption.GetID() );
    m_TailCapTypeChoice.Update( fuselage_ptr->m_CapUMaxOption.GetID() );

    m_CapTessSlider.Update( fuselage_ptr->m_CapUMinTess.GetID() );

    m_NoseCapLenSlider.Update( fuselage_ptr->m_CapUMinLength.GetID() );
    m_NoseCapOffsetSlider.Update( fuselage_ptr->m_CapUMinOffset.GetID() );
    m_NoseCapStrengthSlider.Update( fuselage_ptr->m_CapUMinStrength.GetID() );
    m_NoseCapSweepFlagButton.Update( fuselage_ptr->m_CapUMinSweepFlag.GetID() );

    m_NoseCapLenSlider.Deactivate();
    m_NoseCapOffsetSlider.Deactivate();
    m_NoseCapStrengthSlider.Deactivate();
    m_NoseCapSweepFlagButton.Deactivate();

    switch( fuselage_ptr->m_CapUMinOption() ){
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

    m_TailCapLenSlider.Update( fuselage_ptr->m_CapUMaxLength.GetID() );
    m_TailCapOffsetSlider.Update( fuselage_ptr->m_CapUMaxOffset.GetID() );
    m_TailCapStrengthSlider.Update( fuselage_ptr->m_CapUMaxStrength.GetID() );
    m_TailCapSweepFlagButton.Update( fuselage_ptr->m_CapUMaxSweepFlag.GetID() );

    m_TailCapLenSlider.Deactivate();
    m_TailCapOffsetSlider.Deactivate();
    m_TailCapStrengthSlider.Deactivate();
    m_TailCapSweepFlagButton.Deactivate();

    switch( fuselage_ptr->m_CapUMaxOption() ){
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


    //==== Skin & XSec Index Display ===//
    int xsid = fuselage_ptr->m_ActiveXSec();
    m_XSecIndexSelector.Update( fuselage_ptr->m_ActiveXSec.GetID() );

    FuseXSec* xs = ( FuseXSec* ) fuselage_ptr->GetXSec( xsid );
    if ( xs )
    {
        bool firstxs = xsid == 0;
        bool lastxs = xsid == ( fuselage_ptr->GetXSecSurf( 0 )->NumXSec() - 1 );

        //==== XSec ====//
        m_SectUTessSlider.Update( xs->m_SectTessU.GetID() );
        if ( firstxs )
        {
            m_SectUTessSlider.Deactivate();
        }

        m_XSecXSlider.Update( xs->m_XLocPercent.GetID() );
        m_XSecYSlider.Update( xs->m_YLocPercent.GetID() );
        m_XSecZSlider.Update( xs->m_ZLocPercent.GetID() );
        m_XSecXRotSlider.Update( xs->m_XRotate.GetID() );
        m_XSecYRotSlider.Update( xs->m_YRotate.GetID() );
        m_XSecZRotSlider.Update( xs->m_ZRotate.GetID() );
        m_XSecSpinSlider.Update( xs->m_Spin.GetID() );

        if ( firstxs && ( fuselage_ptr->m_OrderPolicy() == FuselageGeom::FUSE_MONOTONIC || fuselage_ptr->m_OrderPolicy() == FuselageGeom::FUSE_LOOP ) )
        {
            m_XSecXSlider.Deactivate();
        }

        if ( lastxs && fuselage_ptr->m_OrderPolicy() == FuselageGeom::FUSE_MONOTONIC )
        {
            m_XSecXSlider.Deactivate();
        }

        if ( lastxs && fuselage_ptr->m_OrderPolicy() == FuselageGeom::FUSE_LOOP )
        {
            m_XSecXSlider.Deactivate();
            m_XSecYSlider.Deactivate();
            m_XSecZSlider.Deactivate();
            m_XSecXRotSlider.Deactivate();
            m_XSecYRotSlider.Deactivate();
            m_XSecZRotSlider.Deactivate();
            m_XSecSpinSlider.Deactivate();
        }

        // Special considerations for loop policy
        XSecCurve* xsc = xs->GetXSecCurve();
        if ( xsc )
        {
            m_XSecTypeChoice.SetVal( xsc->GetType() );

            if ( lastxs && fuselage_ptr->m_OrderPolicy() == FuselageGeom::FUSE_LOOP )
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

            if ( lastxs && fuselage_ptr->m_OrderPolicy() == FuselageGeom::FUSE_LOOP )
            {
                DisplayGroup( NULL );
            }
        }

    }
    return true;
}

void FuselageScreen::GuiDeviceCallBack( GuiDevice* gui_device )
{
    //==== Find Fuselage Ptr ====//
    Geom* geom_ptr = m_ScreenMgr->GetCurrGeom();
    if ( !geom_ptr || geom_ptr->GetType().m_Type != FUSELAGE_GEOM_TYPE )
    {
        return;
    }
    FuselageGeom* fuselage_ptr = dynamic_cast< FuselageGeom* >( geom_ptr );
    assert( fuselage_ptr );

    if ( gui_device == &m_CutXSec )
    {
        fuselage_ptr->CutActiveXSec();
    }
    else if ( gui_device == &m_CopyXSec   )
    {
        fuselage_ptr->CopyActiveXSec();
    }
    else if ( gui_device == &m_PasteXSec  )
    {
        fuselage_ptr->PasteActiveXSec();
    }
    else if ( gui_device == &m_InsertXSec  )
    {
        fuselage_ptr->InsertXSec( );
    }
    else if ( gui_device == &m_DesignPolicyChoice )
    {
        // This is a hack to get the XSecXSlider to update its ranges.  This
        // requires setting the ID to another valid FractionParm's ID.  In this
        // case, m_YLocPercent of the same XSec.  It will get set back to
        // m_XLocPercent in Update() before anyone notices the change.
        int xsid = fuselage_ptr->m_ActiveXSec();
        FuseXSec* xs = (FuseXSec*) fuselage_ptr->GetXSec( xsid );
        if ( xs )
        {
            m_XSecXSlider.Update( xs->m_YLocPercent.GetID() );
        }
    }

    ChevronScreen::GuiDeviceCallBack( gui_device );
}

//==== Fltk  Callbacks ====//
void FuselageScreen::CallBack( Fl_Widget *w )
{
    ChevronScreen::CallBack( w );
}
