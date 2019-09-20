//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//
//
//////////////////////////////////////////////////////////////////////

#include "FuselageScreen.h"
#include "ScreenMgr.h"
#include "FuselageGeom.h"
#include "ParmMgr.h"
using namespace vsp;



//==== Constructor ====//
FuselageScreen::FuselageScreen( ScreenMgr* mgr ) : SkinScreen( mgr, 400, 630, "Fuselage" )
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


    Fl_Group* xsec_tab = AddTab( "XSec" );
    Fl_Group* xsec_group = AddSubGroup( xsec_tab, 5 );

    m_XSecLayout.SetGroupAndScreen( xsec_group, this );
    m_XSecLayout.AddDividerBox( "Cross Section" );

    m_XSecLayout.AddIndexSelector( m_XSecIndexSelector );
    m_XSecLayout.AddYGap();

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

    m_XSecLayout.AddDividerBox( "Type" );

    m_XSecTypeChoice.AddItem( "POINT" );
    m_XSecTypeChoice.AddItem( "CIRCLE" );
    m_XSecTypeChoice.AddItem( "ELLIPSE" );
    m_XSecTypeChoice.AddItem( "SUPER_ELLIPSE" );
    m_XSecTypeChoice.AddItem( "ROUNDED_RECTANGLE" );
    m_XSecTypeChoice.AddItem( "GENERAL_FUSE" );
    m_XSecTypeChoice.AddItem( "FUSE_FILE" );
    m_XSecTypeChoice.AddItem( "FOUR_SERIES" );
    m_XSecTypeChoice.AddItem( "SIX_SERIES" );
    m_XSecTypeChoice.AddItem( "BICONVEX" );
    m_XSecTypeChoice.AddItem( "WEDGE" );
    m_XSecTypeChoice.AddItem( "EDIT_CURVE" );
    m_XSecTypeChoice.AddItem( "AF_FILE" );
    m_XSecTypeChoice.AddItem( "CST_AIRFOIL" );
    m_XSecTypeChoice.AddItem( "KARMAN_TREFFTZ" );
    m_XSecTypeChoice.AddItem( "FOUR_DIGIT_MOD" );
    m_XSecTypeChoice.AddItem( "FIVE_DIGIT" );
    m_XSecTypeChoice.AddItem( "FIVE_DIGIT_MOD" );
    m_XSecTypeChoice.AddItem( "16_SERIES" );

    m_XSecLayout.SetChoiceButtonWidth( tess_w );
    int show_w = 50;
    int convert_w = 100;

    m_XSecLayout.SetSameLineFlag( true );
    m_XSecLayout.AddChoice( m_XSecTypeChoice, "Choose Type:", ( show_w + convert_w ) );
    m_XSecLayout.SetFitWidthFlag( false );
    m_XSecLayout.SetButtonWidth( show_w );
    m_XSecLayout.AddButton( m_ShowXSecButton, "Show" );

    m_ConvertCEDITGroup.SetGroupAndScreen( AddSubGroup( xsec_tab, 5 ), this );
    m_ConvertCEDITGroup.SetY( m_XSecLayout.GetY() );
    m_ConvertCEDITGroup.SetX( m_XSecLayout.GetX() );
    m_ConvertCEDITGroup.SetButtonWidth( convert_w );
    m_ConvertCEDITGroup.SetFitWidthFlag( false );
    m_ConvertCEDITGroup.AddButton( m_ConvertCEDITButton, "Convert CEDIT" );

    m_EditCEDITGroup.SetGroupAndScreen( AddSubGroup( xsec_tab, 5 ), this );
    m_EditCEDITGroup.SetFitWidthFlag( false );
    m_EditCEDITGroup.SetY( m_XSecLayout.GetY() );
    m_EditCEDITGroup.SetX( m_XSecLayout.GetX() );
    m_EditCEDITGroup.SetButtonWidth( convert_w );
    m_EditCEDITGroup.AddButton( m_EditCEDITButton, "Edit Curve" );

    m_XSecLayout.ForceNewLine();

    m_XSecLayout.InitWidthHeightVals();
    m_XSecLayout.SetFitWidthFlag( true );
    m_XSecLayout.SetSameLineFlag( false );

    //==== Location To Start XSec Layouts ====//
    int start_y = m_XSecLayout.GetY();

    //==== Super XSec ====//
    m_SuperGroup.SetGroupAndScreen( AddSubGroup( xsec_tab, 5 ), this );
    m_SuperGroup.SetY( start_y );
    m_SuperGroup.AddYGap();
    m_SuperGroup.AddSlider( m_SuperHeightSlider, "Height", 10, "%6.5f" );
    m_SuperGroup.AddSlider( m_SuperWidthSlider,  "Width", 10, "%6.5f" );
    m_SuperGroup.AddYGap();
    m_SuperGroup.AddSlider( m_SuperMaxWidthLocSlider, "MaxWLoc", 2, "%6.5f" );
    m_SuperGroup.AddYGap();
    m_SuperGroup.AddSlider( m_SuperMSlider, "M", 10, "%6.5f" );
    m_SuperGroup.AddSlider( m_SuperNSlider, "N", 10, "%6.5f" );
    m_SuperGroup.AddYGap();
    m_SuperGroup.AddButton( m_SuperToggleSym, "T/B Symmetric Exponents" );
    m_SuperGroup.AddSlider( m_SuperM_botSlider, "M Bottom", 10, "%6.5f" );
    m_SuperGroup.AddSlider( m_SuperN_botSlider, "N Bottom", 10, "%6.5f" );

    //==== Circle XSec ====//
    m_CircleGroup.SetGroupAndScreen( AddSubGroup( xsec_tab, 5 ), this );
    m_CircleGroup.SetY( start_y );
    m_CircleGroup.AddYGap();
    m_CircleGroup.AddSlider(  m_DiameterSlider, "Diameter", 10, "%6.5f" );

    //==== Ellipse XSec ====//
    m_EllipseGroup.SetGroupAndScreen( AddSubGroup( xsec_tab, 5 ), this );
    m_EllipseGroup.SetY( start_y );
    m_EllipseGroup.AddYGap();
    m_EllipseGroup.AddSlider(  m_EllipseHeightSlider, "Height", 10, "%6.5f" );
    m_EllipseGroup.AddSlider(  m_EllipseWidthSlider, "Width", 10, "%6.5f" );

    //==== Rounded Rect ====//
    m_RoundedRectGroup.SetGroupAndScreen( AddSubGroup( xsec_tab, 5 ), this );
    m_RoundedRectGroup.SetY( start_y );
    m_RoundedRectGroup.AddYGap();
    m_RoundedRectGroup.AddSlider( m_RRHeightSlider, "Height", 10, "%6.5f" );
    m_RoundedRectGroup.AddSlider( m_RRWidthSlider,  "Width", 10, "%6.5f" );
    m_RoundedRectGroup.AddYGap();
    m_RoundedRectGroup.AddSlider( m_RRSkewSlider, "Skew", 2, "%6.5f");
    m_RoundedRectGroup.AddSlider( m_RRKeystoneSlider, "Keystone", 1, "%6.5f");
    m_RoundedRectGroup.AddYGap();
    m_RoundedRectGroup.AddSlider( m_RRRadiusSlider, "Radius", 10, "%6.5f" );
    m_RoundedRectGroup.AddYGap();
    m_RoundedRectGroup.AddButton( m_RRKeyCornerButton, "Key Corner" );

    //==== General Fuse XSec ====//
    m_GenGroup.SetGroupAndScreen( AddSubGroup( xsec_tab, 5 ), this );
    m_GenGroup.SetY( start_y );
    m_GenGroup.AddSlider( m_GenHeightSlider, "Height", 10, "%6.5f" );
    m_GenGroup.AddSlider( m_GenWidthSlider, "Width", 10, "%6.5f" );
    m_GenGroup.AddYGap();
    m_GenGroup.AddSlider( m_GenMaxWidthLocSlider, "MaxWLoc", 1, "%6.5f" );
    m_GenGroup.AddSlider( m_GenCornerRadSlider, "CornerRad", 1, "%6.5f" );
    m_GenGroup.AddYGap();
    m_GenGroup.AddSlider( m_GenTopTanAngleSlider, "TopTanAng", 90, "%7.5f" );
    m_GenGroup.AddSlider( m_GenBotTanAngleSlider, "BotTanAng", 90, "%7.5f" );
    m_GenGroup.AddYGap();
    m_GenGroup.AddSlider( m_GenTopStrSlider, "TopStr", 1, "%7.5f" );
    m_GenGroup.AddSlider( m_GenBotStrSlider, "BotStr", 1, "%7.5f" );
    m_GenGroup.AddSlider( m_GenUpStrSlider, "UpStr", 1, "%7.5f" );
    m_GenGroup.AddSlider( m_GenLowStrSlider, "LowStr", 1, "%7.5f" );

    //==== Four Series AF ====//
    m_FourSeriesGroup.SetGroupAndScreen( AddSubGroup( xsec_tab, 5 ), this );
    m_FourSeriesGroup.SetY( start_y );
    m_FourSeriesGroup.AddYGap();
    m_FourSeriesGroup.AddOutput( m_FourNameOutput, "Name" );
    m_FourSeriesGroup.AddYGap();
    m_FourSeriesGroup.AddSlider( m_FourChordSlider, "Chord", 10, "%7.3f" );
    m_FourSeriesGroup.AddSlider( m_FourThickChordSlider, "T/C", 1, "%7.5f" );
    m_FourSeriesGroup.AddYGap();

    int actionToggleButtonWidth = 15;
    int actionSliderButtonWidth = m_FourSeriesGroup.GetButtonWidth() - actionToggleButtonWidth;

    m_FourSeriesGroup.SetSameLineFlag( true );

    m_FourSeriesGroup.SetFitWidthFlag( false );
    m_FourSeriesGroup.SetButtonWidth( actionToggleButtonWidth );
    m_FourSeriesGroup.AddButton( m_FourCamberButton, "" );
    m_FourSeriesGroup.SetButtonWidth( actionSliderButtonWidth );
    m_FourSeriesGroup.SetFitWidthFlag( true );
    m_FourSeriesGroup.AddSlider( m_FourCamberSlider, "Camber", 0.2, "%7.5f" );

    m_FourSeriesGroup.ForceNewLine();

    m_FourSeriesGroup.SetFitWidthFlag( false );
    m_FourSeriesGroup.SetButtonWidth( actionToggleButtonWidth );
    m_FourSeriesGroup.AddButton( m_FourCLiButton, "" );
    m_FourSeriesGroup.SetButtonWidth( actionSliderButtonWidth );
    m_FourSeriesGroup.SetFitWidthFlag( true );
    m_FourSeriesGroup.AddSlider( m_FourCLiSlider, "Ideal CL", 1, "%7.5f" );

    m_FourSeriesGroup.ForceNewLine();

    m_FourSeriesGroup.SetSameLineFlag( false );
    m_FourSeriesGroup.SetButtonWidth( actionSliderButtonWidth + actionToggleButtonWidth );

    m_FourCamberGroup.Init( this );
    m_FourCamberGroup.AddButton( m_FourCamberButton.GetFlButton() );
    m_FourCamberGroup.AddButton( m_FourCLiButton.GetFlButton() );

    vector< int > camb_val_map;
    camb_val_map.push_back( vsp::MAX_CAMB );
    camb_val_map.push_back( vsp::DESIGN_CL );
    m_FourCamberGroup.SetValMapVec( camb_val_map );

    m_FourSeriesGroup.AddSlider( m_FourCamberLocSlider, "CamberLoc", 1, "%7.5f" );
    m_FourSeriesGroup.AddYGap();
    m_FourSeriesGroup.AddButton( m_FourInvertButton, "Invert Airfoil" );
    m_FourSeriesGroup.AddYGap();
    m_FourSeriesGroup.AddButton( m_FourSharpTEButton, "Sharpen TE" );

    //==== Six Series AF ====//
    m_SixSeriesGroup.SetGroupAndScreen( AddSubGroup( xsec_tab, 5 ), this );
    m_SixSeriesGroup.SetY( start_y );
    m_SixSeriesGroup.AddYGap();
    m_SixSeriesGroup.AddOutput( m_SixNameOutput, "Name" );
    m_SixSeriesGroup.AddYGap();

    m_SixSeriesChoice.AddItem( "63" );
    m_SixSeriesChoice.AddItem( "64" );
    m_SixSeriesChoice.AddItem( "65" );
    m_SixSeriesChoice.AddItem( "66" );
    m_SixSeriesChoice.AddItem( "67" );
    m_SixSeriesChoice.AddItem( "63a" );
    m_SixSeriesChoice.AddItem( "64a" );
    m_SixSeriesChoice.AddItem( "65a" );
    m_SixSeriesGroup.AddChoice( m_SixSeriesChoice, "Series" );

    m_SixSeriesGroup.AddYGap();

    m_SixSeriesGroup.AddSlider( m_SixChordSlider, "Chord", 10, "%7.3f" );
    m_SixSeriesGroup.AddSlider( m_SixThickChordSlider, "T/C", 1, "%7.5f" );
    m_SixSeriesGroup.AddYGap();
    m_SixSeriesGroup.AddSlider( m_SixIdealClSlider, "Ideal CL", 1, "%7.5f" );
    m_SixSeriesGroup.AddSlider( m_SixASlider, "A", 1, "%7.5f" );
    m_SixSeriesGroup.AddYGap();
    m_SixSeriesGroup.AddButton( m_SixInvertButton, "Invert Airfoil" );

    //==== Biconvex AF ====//
    m_BiconvexGroup.SetGroupAndScreen( AddSubGroup( xsec_tab, 5 ), this );
    m_BiconvexGroup.SetY( start_y );
    m_BiconvexGroup.AddYGap();
    m_BiconvexGroup.AddSlider( m_BiconvexChordSlider, "Chord", 10, "%7.3f" );
    m_BiconvexGroup.AddSlider( m_BiconvexThickChordSlider, "T/C", 1, "%7.5f" );

    //==== Wedge AF ====//
    m_WedgeGroup.SetGroupAndScreen(  AddSubGroup( xsec_tab, 5 ), this );
    m_WedgeGroup.SetY( start_y );
    m_WedgeGroup.AddYGap();
    m_WedgeGroup.AddSlider( m_WedgeChordSlider, "Chord", 10, "%7.3f" );
    m_WedgeGroup.AddSlider( m_WedgeThickChordSlider, "T/C", 1, "%7.5f" );
    m_WedgeGroup.AddYGap();
    m_WedgeGroup.AddSlider( m_WedgeThickLocSlider, "Thick_Loc", 1, "%7.5f" );

    //==== Fuse File ====//
    m_FuseFileGroup.SetGroupAndScreen( AddSubGroup( xsec_tab, 5 ), this );
    m_FuseFileGroup.SetY( start_y );
    m_FuseFileGroup.AddYGap();
    m_FuseFileGroup.AddButton( m_ReadFuseFileButton, "Read File" );
    m_FuseFileGroup.AddYGap();
    m_FuseFileGroup.AddSlider( m_FileHeightSlider, "Height", 10, "%7.3f" );
    m_FuseFileGroup.AddSlider( m_FileWidthSlider, "Width", 10, "%7.3f" );

    //==== Airfoil File ====//
    m_AfFileGroup.SetGroupAndScreen( AddSubGroup( xsec_tab, 5 ), this );
    m_AfFileGroup.SetY( start_y );
    m_AfFileGroup.AddYGap();
    m_AfFileGroup.AddButton( m_AfReadFileButton, "Read File" );
    m_AfFileGroup.AddYGap();
    m_AfFileGroup.AddOutput( m_AfFileNameOutput, "Name" );
    m_AfFileGroup.AddYGap();
    m_AfFileGroup.AddSlider( m_AfFileChordSlider, "Chord", 10, "%7.3f" );
    m_AfFileGroup.AddSlider( m_AfFileThickChordSlider, "T/C", 1, "%7.5f" );
    m_AfFileGroup.AddOutput( m_AfFileBaseThickChordOutput, "Base T/C", "%7.5f" );
    m_AfFileGroup.AddYGap();
    m_AfFileGroup.AddButton( m_AfFileInvertButton, "Invert Airfoil" );
    
    //==== CST Airfoil ====//
    m_CSTAirfoilGroup.SetGroupAndScreen( AddSubGroup( xsec_tab, 5 ), this );
    m_CSTAirfoilGroup.SetY( start_y );
    m_CSTAirfoilGroup.AddYGap();
    m_CSTAirfoilGroup.AddButton( m_CSTContLERadButton, "Enforce Continuous LE Radius" );
    m_CSTAirfoilGroup.AddButton( m_CSTInvertButton, "Invert Airfoil" );

    m_CSTAirfoilGroup.AddYGap();
    m_CSTAirfoilGroup.AddSlider( m_CSTChordSlider, "Chord", 10, "%7.3f");
    
    m_CSTAirfoilGroup.AddYGap();
    m_CSTAirfoilGroup.AddDividerBox( "Upper Surface" );

    m_CSTAirfoilGroup.SetSameLineFlag( true );
    m_CSTAirfoilGroup.SetFitWidthFlag( true );

    m_CSTAirfoilGroup.AddOutput( m_UpDegreeOutput, "Degree", m_CSTAirfoilGroup.GetButtonWidth() * 2 );
    m_CSTAirfoilGroup.SetFitWidthFlag( false );
    m_CSTAirfoilGroup.AddButton( m_UpDemoteButton, "Demote" );
    m_CSTAirfoilGroup.AddButton( m_UpPromoteButton, "Promote" );

    m_CSTAirfoilGroup.ForceNewLine();

    m_CSTAirfoilGroup.SetSameLineFlag( false );
    m_CSTAirfoilGroup.SetFitWidthFlag( true );

    m_CSTUpCoeffScroll = m_CSTAirfoilGroup.AddFlScroll( 60 );

    m_CSTUpCoeffScroll->type( Fl_Scroll::VERTICAL_ALWAYS );
    m_CSTUpCoeffScroll->box( FL_BORDER_BOX );
    m_CSTUpCoeffLayout.SetGroupAndScreen( m_CSTUpCoeffScroll, this );

    m_CSTAirfoilGroup.AddYGap();

    m_CSTAirfoilGroup.AddDividerBox( "Lower Surface" );

    m_CSTAirfoilGroup.SetSameLineFlag( true );
    m_CSTAirfoilGroup.SetFitWidthFlag( true );

    m_CSTAirfoilGroup.AddOutput( m_LowDegreeOutput, "Degree", m_CSTAirfoilGroup.GetButtonWidth() * 2 );
    m_CSTAirfoilGroup.SetFitWidthFlag( false );
    m_CSTAirfoilGroup.AddButton( m_LowDemoteButton, "Demote" );
    m_CSTAirfoilGroup.AddButton( m_LowPromoteButton, "Promote" );

    m_CSTAirfoilGroup.ForceNewLine();

    m_CSTAirfoilGroup.SetSameLineFlag( false );
    m_CSTAirfoilGroup.SetFitWidthFlag( true );

    m_CSTLowCoeffScroll = m_CSTAirfoilGroup.AddFlScroll( 60 );
    m_CSTLowCoeffScroll->type( Fl_Scroll::VERTICAL_ALWAYS );
    m_CSTLowCoeffScroll->box( FL_BORDER_BOX );
    m_CSTLowCoeffLayout.SetGroupAndScreen( m_CSTLowCoeffScroll, this );

    //==== VKT AF ====//
    m_VKTGroup.SetGroupAndScreen( AddSubGroup( xsec_tab, 5 ), this );
    m_VKTGroup.SetY( start_y );
    m_VKTGroup.AddYGap();
    m_VKTGroup.AddSlider( m_VKTChordSlider, "Chord", 10, "%7.3f" );
    m_VKTGroup.AddYGap();
    m_VKTGroup.AddSlider( m_VKTEpsilonSlider, "Epsilon", 1, "%7.5f" );
    m_VKTGroup.AddSlider( m_VKTKappaSlider, "Kappa", 1, "%7.5f" );
    m_VKTGroup.AddSlider( m_VKTTauSlider, "Tau", 10, "%7.5f" );
    m_VKTGroup.AddYGap();
    m_VKTGroup.AddButton( m_VKTInvertButton, "Invert Airfoil" );

    //==== Four Series AF ====//
    m_FourDigitModGroup.SetGroupAndScreen( AddSubGroup( xsec_tab, 5 ), this );
    m_FourDigitModGroup.SetY( start_y );
    m_FourDigitModGroup.AddYGap();
    m_FourDigitModGroup.AddOutput( m_FourModNameOutput, "Name" );
    m_FourDigitModGroup.AddYGap();
    m_FourDigitModGroup.AddSlider( m_FourModChordSlider, "Chord", 10, "%7.3f" );
    m_FourDigitModGroup.AddSlider( m_FourModThickChordSlider, "T/C", 1, "%7.5f" );
    m_FourDigitModGroup.AddYGap();

    m_FourDigitModGroup.SetSameLineFlag( true );

    m_FourDigitModGroup.SetFitWidthFlag( false );
    m_FourDigitModGroup.SetButtonWidth( actionToggleButtonWidth );
    m_FourDigitModGroup.AddButton( m_FourModCamberButton, "" );
    m_FourDigitModGroup.SetButtonWidth( actionSliderButtonWidth );
    m_FourDigitModGroup.SetFitWidthFlag( true );
    m_FourDigitModGroup.AddSlider( m_FourModCamberSlider, "Camber", 0.2, "%7.5f" );

    m_FourDigitModGroup.ForceNewLine();

    m_FourDigitModGroup.SetFitWidthFlag( false );
    m_FourDigitModGroup.SetButtonWidth( actionToggleButtonWidth );
    m_FourDigitModGroup.AddButton( m_FourModCLiButton, "" );
    m_FourDigitModGroup.SetButtonWidth( actionSliderButtonWidth );
    m_FourDigitModGroup.SetFitWidthFlag( true );
    m_FourDigitModGroup.AddSlider( m_FourModCLiSlider, "Ideal CL", 1, "%7.5f" );

    m_FourDigitModGroup.ForceNewLine();

    m_FourDigitModGroup.SetSameLineFlag( false );
    m_FourDigitModGroup.SetButtonWidth( actionSliderButtonWidth + actionToggleButtonWidth );

    m_FourModCamberGroup.Init( this );
    m_FourModCamberGroup.AddButton( m_FourModCamberButton.GetFlButton() );
    m_FourModCamberGroup.AddButton( m_FourModCLiButton.GetFlButton() );

    m_FourModCamberGroup.SetValMapVec( camb_val_map );

    m_FourDigitModGroup.AddSlider( m_FourModCamberLocSlider, "CamberLoc", 1, "%7.5f" );
    m_FourDigitModGroup.AddYGap();
    m_FourDigitModGroup.AddSlider( m_FourModThicknessLocSlider, "T/CLoc", 0.5, "%7.5f" );
    m_FourDigitModGroup.AddSlider( m_FourModLERadIndexSlider, "LERadIndx", 5, "%7.5f" );
    m_FourDigitModGroup.AddYGap();
    m_FourDigitModGroup.AddButton( m_FourModInvertButton, "Invert Airfoil" );
    m_FourDigitModGroup.AddYGap();
    m_FourDigitModGroup.AddButton( m_FourModSharpTEButton, "Sharpen TE" );

    //==== Five Digit AF ====//
    m_FiveDigitGroup.SetGroupAndScreen( AddSubGroup( xsec_tab, 5 ), this );
    m_FiveDigitGroup.SetY( start_y );
    m_FiveDigitGroup.AddYGap();
    m_FiveDigitGroup.AddOutput( m_FiveNameOutput, "Name" );
    m_FiveDigitGroup.AddYGap();
    m_FiveDigitGroup.AddSlider( m_FiveChordSlider, "Chord", 10, "%7.3f" );
    m_FiveDigitGroup.AddSlider( m_FiveThickChordSlider, "T/C", 1, "%7.5f" );
    m_FiveDigitGroup.AddYGap();
    m_FiveDigitGroup.AddSlider( m_FiveCLiSlider, "Ideal CL", 1, "%7.5f" );
    m_FiveDigitGroup.AddSlider( m_FiveCamberLocSlider, "CamberLoc", 1, "%7.5f" );
    m_FiveDigitGroup.AddYGap();
    m_FiveDigitGroup.AddButton( m_FiveInvertButton, "Invert Airfoil" );
    m_FiveDigitGroup.AddYGap();
    m_FiveDigitGroup.AddButton( m_FiveSharpTEButton, "Sharpen TE" );

    //==== Five Digit Mod AF ====//
    m_FiveDigitModGroup.SetGroupAndScreen( AddSubGroup( xsec_tab, 5 ), this );
    m_FiveDigitModGroup.SetY( start_y );
    m_FiveDigitModGroup.AddYGap();
    m_FiveDigitModGroup.AddOutput( m_FiveModNameOutput, "Name" );
    m_FiveDigitModGroup.AddYGap();
    m_FiveDigitModGroup.AddSlider( m_FiveModChordSlider, "Chord", 10, "%7.3f" );
    m_FiveDigitModGroup.AddSlider( m_FiveModThickChordSlider, "T/C", 1, "%7.5f" );
    m_FiveDigitModGroup.AddYGap();
    m_FiveDigitModGroup.AddSlider( m_FiveModCLiSlider, "Ideal CL", 1, "%7.5f" );
    m_FiveDigitModGroup.AddSlider( m_FiveModCamberLocSlider, "CamberLoc", 1, "%7.5f" );
    m_FiveDigitModGroup.AddYGap();
    m_FiveDigitModGroup.AddSlider( m_FiveModThicknessLocSlider, "T/CLoc", 0.5, "%7.5f" );
    m_FiveDigitModGroup.AddSlider( m_FiveModLERadIndexSlider, "LERadIndx", 5, "%7.5f" );
    m_FiveDigitModGroup.AddYGap();
    m_FiveDigitModGroup.AddButton( m_FiveModInvertButton, "Invert Airfoil" );
    m_FiveDigitModGroup.AddYGap();
    m_FiveDigitModGroup.AddButton( m_FiveModSharpTEButton, "Sharpen TE" );

    //==== 16 Series AF ====//
    m_OneSixSeriesGroup.SetGroupAndScreen( AddSubGroup( xsec_tab, 5 ), this );
    m_OneSixSeriesGroup.SetY( start_y );
    m_OneSixSeriesGroup.AddYGap();
    m_OneSixSeriesGroup.AddOutput( m_OneSixSeriesNameOutput, "Name" );
    m_OneSixSeriesGroup.AddYGap();
    m_OneSixSeriesGroup.AddSlider( m_OneSixSeriesChordSlider, "Chord", 10, "%7.3f" );
    m_OneSixSeriesGroup.AddSlider( m_OneSixSeriesThickChordSlider, "T/C", 1, "%7.5f" );
    m_OneSixSeriesGroup.AddYGap();
    m_OneSixSeriesGroup.AddSlider( m_OneSixSeriesCLiSlider, "Ideal CL", 1, "%7.5f" );
    m_OneSixSeriesGroup.AddYGap();
    m_OneSixSeriesGroup.AddButton( m_OneSixSeriesInvertButton, "Invert Airfoil" );
    m_OneSixSeriesGroup.AddYGap();
    m_OneSixSeriesGroup.AddButton( m_OneSixSeriesSharpTEButton, "Sharpen TE" );

    DisplayGroup( &m_PointGroup );

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

    SkinScreen::Update();
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
    int xsid = fuselage_ptr->GetActiveXSecIndex();
    m_XSecIndexSelector.SetIndex( xsid );

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


        XSecCurve* xsc = xs->GetXSecCurve();
        if ( xsc )
        {
            m_XSecTypeChoice.SetVal( xsc->GetType() );

            if ( lastxs && fuselage_ptr->m_OrderPolicy() == FuselageGeom::FUSE_LOOP )
            {
                m_XSecTypeChoice.Deactivate();
            }
            else
            {
                m_XSecTypeChoice.Activate();
            }

            if ( lastxs && fuselage_ptr->m_OrderPolicy() == FuselageGeom::FUSE_LOOP )
            {
                DisplayGroup ( NULL);
            }
            else if ( xsc->GetType() == XS_POINT )
            {
                DisplayGroup( NULL );
            }
            else if ( xsc->GetType() == XS_SUPER_ELLIPSE )
            {
                DisplayGroup( &m_SuperGroup );

                SuperXSec* super_xs = dynamic_cast< SuperXSec* >( xsc );
                assert( super_xs );
                m_SuperHeightSlider.Update( super_xs->m_Height.GetID() );
                m_SuperWidthSlider.Update( super_xs->m_Width.GetID() );
                m_SuperMSlider.Update( super_xs->m_M.GetID() );
                m_SuperNSlider.Update( super_xs->m_N.GetID() );
                m_SuperToggleSym.Update( super_xs->m_TopBotSym.GetID() );
                m_SuperM_botSlider.Update( super_xs->m_M_bot.GetID() );
                m_SuperN_botSlider.Update( super_xs->m_N_bot.GetID() );
                m_SuperMaxWidthLocSlider.Update( super_xs->m_MaxWidthLoc.GetID() );

                if ( super_xs->m_TopBotSym() )
                {
                    m_SuperM_botSlider.Deactivate();
                    m_SuperN_botSlider.Deactivate();
                }
                else if ( !super_xs->m_TopBotSym() )
                {
                    m_SuperM_botSlider.Activate();
                    m_SuperN_botSlider.Activate();
                }
            }
            else if ( xsc->GetType() == XS_CIRCLE )
            {
                DisplayGroup( &m_CircleGroup );
                CircleXSec* circle_xs = dynamic_cast< CircleXSec* >( xsc );
                assert( circle_xs );

                m_DiameterSlider.Update( circle_xs->m_Diameter.GetID() );
            }
            else if ( xsc->GetType() == XS_ELLIPSE )
            {
                DisplayGroup( & m_EllipseGroup );

                EllipseXSec* ellipse_xs = dynamic_cast< EllipseXSec* >( xsc );
                m_EllipseHeightSlider.Update( ellipse_xs->m_Height.GetID() );
                m_EllipseWidthSlider.Update( ellipse_xs->m_Width.GetID() );
            }
            else if ( xsc->GetType() == XS_ROUNDED_RECTANGLE )
            {
                DisplayGroup( & m_RoundedRectGroup );
                RoundedRectXSec* rect_xs = dynamic_cast< RoundedRectXSec* >( xsc );
                assert( rect_xs );

                m_RRHeightSlider.Update( rect_xs->m_Height.GetID() );
                m_RRWidthSlider.Update( rect_xs->m_Width.GetID() );
                m_RRRadiusSlider.Update( rect_xs->m_Radius.GetID() );
                m_RRKeyCornerButton.Update( rect_xs->m_KeyCornerParm.GetID() );
                m_RRSkewSlider.Update( rect_xs->m_Skew.GetID() );
                m_RRKeystoneSlider.Update( rect_xs->m_Keystone.GetID() );
            }
            else if ( xsc->GetType() == XS_GENERAL_FUSE )
            {
                DisplayGroup( &m_GenGroup );
                GeneralFuseXSec* gen_xs = dynamic_cast< GeneralFuseXSec* >( xsc );
                assert( gen_xs );

                m_GenHeightSlider.Update( gen_xs->m_Height.GetID() );
                m_GenWidthSlider.Update( gen_xs->m_Width.GetID() );
                m_GenMaxWidthLocSlider.Update( gen_xs->m_MaxWidthLoc.GetID() );
                m_GenCornerRadSlider.Update( gen_xs->m_CornerRad.GetID() );
                m_GenTopTanAngleSlider.Update( gen_xs->m_TopTanAngle.GetID() );
                m_GenBotTanAngleSlider.Update( gen_xs->m_BotTanAngle.GetID() );
                m_GenTopStrSlider.Update( gen_xs->m_TopStr.GetID() );
                m_GenBotStrSlider.Update( gen_xs->m_BotStr.GetID() );
                m_GenUpStrSlider.Update( gen_xs->m_UpStr.GetID() );
                m_GenLowStrSlider.Update( gen_xs->m_LowStr.GetID() );
            }
            else if ( xsc->GetType() == XS_FOUR_SERIES )
            {
                DisplayGroup( &m_FourSeriesGroup );
                FourSeries* fs_xs = dynamic_cast< FourSeries* >( xsc );
                assert( fs_xs );

                m_FourChordSlider.Update( fs_xs->m_Chord.GetID() );
                m_FourThickChordSlider.Update( fs_xs->m_ThickChord.GetID() );
                m_FourCamberSlider.Update( fs_xs->m_Camber.GetID() );
                m_FourCLiSlider.Update( fs_xs->m_IdealCl.GetID() );
                m_FourCamberGroup.Update( fs_xs->m_CamberInputFlag.GetID() );
                if ( fs_xs->m_CamberInputFlag() == MAX_CAMB )
                {
                    m_FourCamberSlider.Activate();
                    m_FourCLiSlider.Deactivate();
                }
                else
                {
                    m_FourCamberSlider.Deactivate();
                    m_FourCLiSlider.Activate();
                }
                m_FourCamberLocSlider.Update( fs_xs->m_CamberLoc.GetID() );
                m_FourInvertButton.Update( fs_xs->m_Invert.GetID() );
                m_FourNameOutput.Update( fs_xs->GetAirfoilName() );
                m_FourSharpTEButton.Update( fs_xs->m_SharpTE.GetID() );
            }
            else if ( xsc->GetType() == XS_SIX_SERIES )
            {
                DisplayGroup( &m_SixSeriesGroup );
                SixSeries* ss_xs = dynamic_cast< SixSeries* >( xsc );
                assert( ss_xs );

                m_SixChordSlider.Update( ss_xs->m_Chord.GetID() );
                m_SixThickChordSlider.Update( ss_xs->m_ThickChord.GetID() );
                m_SixIdealClSlider.Update( ss_xs->m_IdealCl.GetID() );
                m_SixASlider.Update( ss_xs->m_A.GetID() );

                m_SixInvertButton.Update( ss_xs->m_Invert.GetID() );
                m_SixNameOutput.Update( ss_xs->GetAirfoilName() );
                m_SixSeriesChoice.Update( ss_xs->m_Series.GetID() );
            }
            else if ( xsc->GetType() == XS_BICONVEX )
            {
                DisplayGroup( &m_BiconvexGroup );
                Biconvex* bi_xs = dynamic_cast< Biconvex* >( xsc );
                assert( bi_xs );

                m_BiconvexChordSlider.Update( bi_xs->m_Chord.GetID() );
                m_BiconvexThickChordSlider.Update( bi_xs->m_ThickChord.GetID() );
            }
            else if ( xsc->GetType() == XS_WEDGE )
            {
                DisplayGroup( &m_WedgeGroup );
                Wedge* we_xs = dynamic_cast< Wedge* >( xsc );
                assert( we_xs );

                m_WedgeChordSlider.Update( we_xs->m_Chord.GetID() );
                m_WedgeThickChordSlider.Update( we_xs->m_ThickChord.GetID() );
                m_WedgeThickLocSlider.Update( we_xs->m_ThickLoc.GetID() );
            }
            else if ( xsc->GetType() == XS_FILE_FUSE )
            {
                DisplayGroup( &m_FuseFileGroup );
                FileXSec* file_xs = dynamic_cast< FileXSec* >( xsc );
                assert( file_xs );
                m_FileHeightSlider.Update( file_xs->m_Height.GetID() );
                m_FileWidthSlider.Update( file_xs->m_Width.GetID() );
            }
            else if ( xsc->GetType() == XS_FILE_AIRFOIL )
            {
                DisplayGroup( &m_AfFileGroup );
                FileAirfoil* affile_xs = dynamic_cast< FileAirfoil* >( xsc );
                assert( affile_xs );

                m_AfFileChordSlider.Update( affile_xs->m_Chord.GetID() );
                m_AfFileThickChordSlider.Update( affile_xs->m_ThickChord.GetID() );
                m_AfFileBaseThickChordOutput.Update( affile_xs->m_BaseThickness.GetID() );
                m_AfFileInvertButton.Update( affile_xs->m_Invert.GetID() );
                m_AfFileNameOutput.Update( affile_xs->GetAirfoilName() );
            }
            else if ( xsc->GetType() == XS_CST_AIRFOIL )
            {
                DisplayGroup( &m_CSTAirfoilGroup );
                CSTAirfoil* cst_xs = dynamic_cast< CSTAirfoil* >( xsc );
                assert( cst_xs );
                
                int num_up = cst_xs->m_UpDeg() + 1;
                int num_low = cst_xs->m_LowDeg() + 1;

                char str[255];
                sprintf( str, "%d", cst_xs->m_UpDeg() );
                m_UpDegreeOutput.Update( str );
                sprintf( str, "%d", cst_xs->m_LowDeg() );
                m_LowDegreeOutput.Update( str );

                m_CSTChordSlider.Update(cst_xs->m_Chord.GetID());
                m_CSTInvertButton.Update( cst_xs->m_Invert.GetID() );
                m_CSTContLERadButton.Update( cst_xs->m_ContLERad.GetID() );

                if ( ( m_UpCoeffSliderVec.size() != num_up ) || ( m_LowCoeffSliderVec.size() != num_low ) )
                {
                    RebuildCSTGroup( cst_xs );
                }

                for ( int i = 0; i < num_up; i++ )
                {
                    Parm *p = cst_xs->m_UpCoeffParmVec[i];
                    if ( p )
                    {
                        m_UpCoeffSliderVec[i].Update( p->GetID() );
                    }
                }

                for ( int i = 0; i < num_low; i++ )
                {
                    Parm *p = cst_xs->m_LowCoeffParmVec[i];
                    if ( p )
                    {
                        m_LowCoeffSliderVec[i].Update( p->GetID() );
                    }
                }

                if ( cst_xs->m_ContLERad() && num_low > 0 )
                {
                    m_LowCoeffSliderVec[0].Deactivate();
                }
            }
            else if ( xsc->GetType() == XS_VKT_AIRFOIL )
            {
                DisplayGroup( &m_VKTGroup );
                VKTAirfoil* vkt_xs = dynamic_cast< VKTAirfoil* >( xsc );
                assert( vkt_xs );

                m_VKTChordSlider.Update( vkt_xs->m_Chord.GetID() );
                m_VKTEpsilonSlider.Update( vkt_xs->m_Epsilon.GetID() );
                m_VKTKappaSlider.Update( vkt_xs->m_Kappa.GetID() );
                m_VKTTauSlider.Update( vkt_xs->m_Tau.GetID() );
                m_VKTInvertButton.Update( vkt_xs->m_Invert.GetID() );
            }
            else if ( xsc->GetType() == XS_FOUR_DIGIT_MOD )
            {
                DisplayGroup( &m_FourDigitModGroup );
                FourDigMod* fs_xs = dynamic_cast< FourDigMod* >( xsc );
                assert( fs_xs );

                m_FourModChordSlider.Update( fs_xs->m_Chord.GetID() );
                m_FourModThickChordSlider.Update( fs_xs->m_ThickChord.GetID() );
                m_FourModCamberSlider.Update( fs_xs->m_Camber.GetID() );
                m_FourModCLiSlider.Update( fs_xs->m_IdealCl.GetID() );
                m_FourModCamberGroup.Update( fs_xs->m_CamberInputFlag.GetID() );
                if ( fs_xs->m_CamberInputFlag() == MAX_CAMB )
                {
                    m_FourModCamberSlider.Activate();
                    m_FourModCLiSlider.Deactivate();
                }
                else
                {
                    m_FourModCamberSlider.Deactivate();
                    m_FourModCLiSlider.Activate();
                }
                m_FourModCamberLocSlider.Update( fs_xs->m_CamberLoc.GetID() );
                m_FourModInvertButton.Update( fs_xs->m_Invert.GetID() );
                m_FourModNameOutput.Update( fs_xs->GetAirfoilName() );
                m_FourModThicknessLocSlider.Update( fs_xs->m_ThickLoc.GetID() );
                m_FourModLERadIndexSlider.Update( fs_xs->m_LERadIndx.GetID() );
                m_FourModSharpTEButton.Update( fs_xs->m_SharpTE.GetID() );
            }
            else if ( xsc->GetType() == XS_FIVE_DIGIT )
            {
                DisplayGroup( &m_FiveDigitGroup );
                FiveDig* fs_xs = dynamic_cast< FiveDig* >( xsc );
                assert( fs_xs );

                m_FiveChordSlider.Update( fs_xs->m_Chord.GetID() );
                m_FiveThickChordSlider.Update( fs_xs->m_ThickChord.GetID() );
                m_FiveCLiSlider.Update( fs_xs->m_IdealCl.GetID() );
                m_FiveCamberLocSlider.Update( fs_xs->m_CamberLoc.GetID() );
                m_FiveInvertButton.Update( fs_xs->m_Invert.GetID() );
                m_FiveNameOutput.Update( fs_xs->GetAirfoilName() );
                m_FiveSharpTEButton.Update( fs_xs->m_SharpTE.GetID() );
            }
            else if ( xsc->GetType() == XS_FIVE_DIGIT_MOD )
            {
                DisplayGroup( &m_FiveDigitModGroup );
                FiveDigMod* fs_xs = dynamic_cast< FiveDigMod* >( xsc );
                assert( fs_xs );

                m_FiveModChordSlider.Update( fs_xs->m_Chord.GetID() );
                m_FiveModThickChordSlider.Update( fs_xs->m_ThickChord.GetID() );
                m_FiveModCLiSlider.Update( fs_xs->m_IdealCl.GetID() );
                m_FiveModCamberLocSlider.Update( fs_xs->m_CamberLoc.GetID() );
                m_FiveModInvertButton.Update( fs_xs->m_Invert.GetID() );
                m_FiveModNameOutput.Update( fs_xs->GetAirfoilName() );
                m_FiveModThicknessLocSlider.Update( fs_xs->m_ThickLoc.GetID() );
                m_FiveModLERadIndexSlider.Update( fs_xs->m_LERadIndx.GetID() );
                m_FiveModSharpTEButton.Update( fs_xs->m_SharpTE.GetID() );
            }
            else if ( xsc->GetType() == XS_ONE_SIX_SERIES )
            {
                DisplayGroup( &m_OneSixSeriesGroup );
                OneSixSeries* fs_xs = dynamic_cast< OneSixSeries* >( xsc );
                assert( fs_xs );

                m_OneSixSeriesChordSlider.Update( fs_xs->m_Chord.GetID() );
                m_OneSixSeriesThickChordSlider.Update( fs_xs->m_ThickChord.GetID() );
                m_OneSixSeriesCLiSlider.Update( fs_xs->m_IdealCl.GetID() );
                m_OneSixSeriesInvertButton.Update( fs_xs->m_Invert.GetID() );
                m_OneSixSeriesNameOutput.Update( fs_xs->GetAirfoilName() );
                m_OneSixSeriesSharpTEButton.Update( fs_xs->m_SharpTE.GetID() );
            }
            else if ( xsc->GetType() == XS_EDIT_CURVE )
            {
                m_EditCEDITGroup.Show();
                m_ConvertCEDITGroup.Hide();
                DisplayGroup( NULL );
            }

            if ( xsc->GetType() != XS_EDIT_CURVE )
            {
                m_EditCEDITGroup.Hide();
                m_ConvertCEDITGroup.Show();
            }
        }
    }
    return true;
}

void FuselageScreen::DisplayGroup( GroupLayout* group )
{
    if ( m_CurrDisplayGroup == group )
    {
        return;
    }

    m_SuperGroup.Hide();
    m_CircleGroup.Hide();
    m_EllipseGroup.Hide();
    m_RoundedRectGroup.Hide();
    m_GenGroup.Hide();
    m_FourSeriesGroup.Hide();
    m_SixSeriesGroup.Hide();
    m_BiconvexGroup.Hide();
    m_WedgeGroup.Hide();
    m_FuseFileGroup.Hide();
    m_AfFileGroup.Hide();
    m_CSTAirfoilGroup.Hide();
    m_VKTGroup.Hide();
    m_FourDigitModGroup.Hide();
    m_FiveDigitGroup.Hide();
    m_FiveDigitModGroup.Hide();
    m_OneSixSeriesGroup.Hide();

    m_CurrDisplayGroup = group;

    if ( group )
    {
        group->Show();
    }
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

    if ( gui_device == &m_XSecIndexSelector )
    {
        fuselage_ptr->SetActiveXSecIndex( m_XSecIndexSelector.GetIndex() );
        fuselage_ptr->Update();
    }
    else if ( gui_device == &m_XSecTypeChoice )
    {
        int t = m_XSecTypeChoice.GetVal();
        fuselage_ptr->SetActiveXSecType( t );

        if ( t == XS_EDIT_CURVE )
        {
            m_ScreenMgr->ShowScreen( ScreenMgr::VSP_CURVE_EDIT_SCREEN );
        }

    }
    else if ( gui_device == &m_ShowXSecButton )
    {
        m_ScreenMgr->ShowScreen( ScreenMgr::VSP_XSEC_SCREEN );
    }
    else if ( gui_device == &m_ConvertCEDITButton )
    {
        XSec* xs = fuselage_ptr->GetXSec( fuselage_ptr->GetActiveXSecIndex() );

        if ( xs )
        {
            EditCurveXSec* edit_xsec = xs->ConvertToEdit();

            if ( edit_xsec )
            {
                m_ScreenMgr->ShowScreen( ScreenMgr::VSP_CURVE_EDIT_SCREEN );
            }
        }
    }
    else if ( gui_device == &m_EditCEDITButton )
    {
        m_ScreenMgr->ShowScreen( ScreenMgr::VSP_CURVE_EDIT_SCREEN );
    }
    else if ( gui_device == &m_CutXSec )
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
    else if ( gui_device == &m_ReadFuseFileButton  )
    {
        int xsid = fuselage_ptr->GetActiveXSecIndex();
        XSec* xs = fuselage_ptr->GetXSec( xsid );
        if ( xs )
        {
            XSecCurve* xsc = xs->GetXSecCurve();
            if ( xsc )
            {
                if ( xsc->GetType() == XS_FILE_FUSE  )
                {
                    FileXSec* file_xs = dynamic_cast< FileXSec* >( xsc );
                    assert( file_xs );
                    string newfile = m_ScreenMgr->GetSelectFileScreen()->FileChooser( "Fuselage Cross Section", "*.fxs" );

                    file_xs->ReadXsecFile( newfile );
                    file_xs->Update();
                    xs->Update();
                    fuselage_ptr->Update();
                }
            }
        }
    }
    else if ( gui_device == &m_AfReadFileButton   )
    {
        int xsid = fuselage_ptr->GetActiveXSecIndex();
        XSec* xs = fuselage_ptr->GetXSec( xsid );
        if ( xs )
        {
            XSecCurve* xsc = xs->GetXSecCurve();
            if ( xsc )
            {
                if ( xsc->GetType() == XS_FILE_AIRFOIL  )
                {
                    FileAirfoil* affile_xs = dynamic_cast< FileAirfoil* >( xsc );
                    assert( affile_xs );
                    string newfile = m_ScreenMgr->GetSelectFileScreen()->FileChooser( "Airfoil File", "*.{af,dat}", false );

                    affile_xs->ReadFile( newfile );
                    affile_xs->Update();
                    xs->Update();
                    fuselage_ptr->Update();
                }
            }
        }
    }
    else if ( gui_device == &m_DesignPolicyChoice )
    {
        // This is a hack to get the XSecXSlider to update its ranges.  This
        // requires setting the ID to another valid FractionParm's ID.  In this
        // case, m_YLocPercent of the same XSec.  It will get set back to
        // m_XLocPercent in Update() before anyone notices the change.
        int xsid = fuselage_ptr->GetActiveXSecIndex();
        FuseXSec* xs = (FuseXSec*) fuselage_ptr->GetXSec( xsid );
        if ( xs )
        {
            m_XSecXSlider.Update( xs->m_YLocPercent.GetID() );
        }
    }
    else if ( gui_device == &m_UpPromoteButton )
    {
        int xsid = fuselage_ptr->GetActiveXSecIndex();
        XSec* xs = fuselage_ptr->GetXSec( xsid );
        if ( xs )
        {
            XSecCurve* xsc = xs->GetXSecCurve();
            if ( xsc )
            {
                if ( xsc->GetType() == XS_CST_AIRFOIL  )
                {
                    CSTAirfoil* cst_xs = dynamic_cast< CSTAirfoil* >( xsc );
                    assert( cst_xs );

                    cst_xs->PromoteUpper();
                    cst_xs->Update();
                    xs->Update();
                    fuselage_ptr->Update();
                }
            }
        }
    }
    else if ( gui_device == &m_LowPromoteButton )
    {
        int xsid = fuselage_ptr->GetActiveXSecIndex();
        XSec* xs = fuselage_ptr->GetXSec( xsid );
        if ( xs )
        {
            XSecCurve* xsc = xs->GetXSecCurve();
            if ( xsc )
            {
                if ( xsc->GetType() == XS_CST_AIRFOIL  )
                {
                    CSTAirfoil* cst_xs = dynamic_cast< CSTAirfoil* >( xsc );
                    assert( cst_xs );

                    cst_xs->PromoteLower();
                    cst_xs->Update();
                    xs->Update();
                    fuselage_ptr->Update();
                }
            }
        }

    }
    else if ( gui_device == &m_UpDemoteButton )
    {
        int xsid = fuselage_ptr->GetActiveXSecIndex();
        XSec* xs = fuselage_ptr->GetXSec( xsid );
        if ( xs )
        {
            XSecCurve* xsc = xs->GetXSecCurve();
            if ( xsc )
            {
                if ( xsc->GetType() == XS_CST_AIRFOIL  )
                {
                    CSTAirfoil* cst_xs = dynamic_cast< CSTAirfoil* >( xsc );
                    assert( cst_xs );

                    cst_xs->DemoteUpper();
                    cst_xs->Update();
                    xs->Update();
                    fuselage_ptr->Update();
                }
            }
        }

    }
    else if ( gui_device == &m_LowDemoteButton )
    {
        int xsid = fuselage_ptr->GetActiveXSecIndex();
        XSec* xs = fuselage_ptr->GetXSec( xsid );
        if ( xs )
        {
            XSecCurve* xsc = xs->GetXSecCurve();
            if ( xsc )
            {
                if ( xsc->GetType() == XS_CST_AIRFOIL  )
                {
                    CSTAirfoil* cst_xs = dynamic_cast< CSTAirfoil* >( xsc );
                    assert( cst_xs );

                    cst_xs->DemoteLower();
                    cst_xs->Update();
                    xs->Update();
                    fuselage_ptr->Update();
                }
            }
        }

    }

    SkinScreen::GuiDeviceCallBack( gui_device );
}

//==== Fltk  Callbacks ====//
void FuselageScreen::CallBack( Fl_Widget *w )
{
    SkinScreen::CallBack( w );
}

void FuselageScreen::RebuildCSTGroup( CSTAirfoil* cst_xs)
{
    if ( !cst_xs )
    {
        return;
    }

    if ( !m_CSTUpCoeffScroll || !m_CSTLowCoeffScroll )
    {
         return;
    }

    m_CSTUpCoeffScroll->clear();
    m_CSTUpCoeffLayout.SetGroup( m_CSTUpCoeffScroll );
    m_CSTUpCoeffLayout.InitWidthHeightVals();

    m_UpCoeffSliderVec.clear();

    unsigned int num_up = cst_xs->m_UpDeg() + 1;

    m_UpCoeffSliderVec.resize( num_up );

    for ( int i = 0; i < num_up; i++ )
    {
        m_CSTUpCoeffLayout.AddSlider( m_UpCoeffSliderVec[i], "AUTO_UPDATE", 2, "%9.5f" );
    }




    m_CSTLowCoeffScroll->clear();
    m_CSTLowCoeffLayout.SetGroup( m_CSTLowCoeffScroll );
    m_CSTLowCoeffLayout.InitWidthHeightVals();

    m_LowCoeffSliderVec.clear();

    unsigned int num_low = cst_xs->m_LowDeg() + 1;

    m_LowCoeffSliderVec.resize( num_low );


    for ( int i = 0; i < num_low; i++ )
    {
        m_CSTLowCoeffLayout.AddSlider( m_LowCoeffSliderVec[i], "AUTO_UPDATE", 2, "%9.5f" );
    }
}
