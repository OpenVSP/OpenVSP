//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//
//
//////////////////////////////////////////////////////////////////////

#include "WingScreen.h"
#include "ScreenMgr.h"
#include "WingGeom.h"
#include "EventMgr.h"
#include "Vehicle.h"
#include "ParmMgr.h"
#include "StlHelper.h"
#include "APIDefines.h"

#include <assert.h>

using namespace vsp;

//==== Constructor ====//
WingScreen::WingScreen( ScreenMgr* mgr ) : GeomScreen( mgr, 335, 640, "Wing" )
{
    m_CurrDisplayGroup = NULL;

    Fl_Group* plan_tab = AddTab( "Plan" );
    Fl_Group* plan_group = AddSubGroup( plan_tab, 5 );

    m_PlanLayout.SetGroupAndScreen( plan_group, this );

    m_PlanLayout.AddDividerBox( "Total Planform" );

    m_PlanLayout.AddSlider( m_PlanSpanSlider, "Span", 10, "%6.5f" );
    m_PlanLayout.AddSlider( m_PlanProjSpanSlider, "Proj Span", 10, "%6.5f" );
    m_PlanLayout.AddSlider( m_PlanChordSlider, "Chord", 10, "%6.5f" );
    m_PlanLayout.AddSlider( m_PlanAreaSlider, "Area", 10, "%6.5f" );

    m_PlanLayout.SetButtonWidth( 100 );
    m_PlanLayout.AddOutput( m_PlanAROutput, "Aspect Ratio" );

    m_PlanLayout.AddYGap();
    
    m_PlanLayout.AddDividerBox( "Tip Treatment" );

    m_RootCapTypeChoice.AddItem("None");
    m_RootCapTypeChoice.AddItem("Flat");
    m_PlanLayout.AddChoice(m_RootCapTypeChoice, "Root Cap Type");
    m_TipCapTypeChoice.AddItem("None");
    m_TipCapTypeChoice.AddItem("Flat");
    m_PlanLayout.AddChoice(m_TipCapTypeChoice, "Tip Cap Type");
    m_PlanLayout.AddYGap();
    m_PlanLayout.AddSlider( m_CapTessSlider, "Cap Tess", 10, "%3.0f" );

    m_PlanLayout.AddYGap();
    m_PlanLayout.AddDividerBox( "Root Incidence" );
    m_PlanLayout.AddSlider( m_IncidenceSlider, "Incidence", 10, "%6.5f" );
    m_PlanLayout.AddSlider( m_IncidenceLocSlider, "Incidence Loc", 10, "%6.5f" );

    Fl_Group* sect_tab = AddTab( "Sect" );
    Fl_Group* sect_group = AddSubGroup( sect_tab, 5 );

    m_SectionLayout.SetGroupAndScreen( sect_group, this );
    m_SectionLayout.AddDividerBox( "Wing Section" );

    m_SectionLayout.AddIndexSelector( m_SectIndexSelector );
    m_SectionLayout.AddYGap();

    m_SectionLayout.SetFitWidthFlag( false );
    m_SectionLayout.SetSameLineFlag( true );

    m_SectionLayout.SetButtonWidth( m_SectionLayout.GetRemainX() / 5 );

    m_SectionLayout.AddButton( m_SplitSectButton, "Split" );
    m_SectionLayout.AddButton( m_CutSectButton, "Cut" );
    m_SectionLayout.AddButton( m_CopySectButton, "Copy" );
    m_SectionLayout.AddButton( m_PasteSectButton, "Paste" );
    m_SectionLayout.AddButton( m_InsertSectButton, "Insert" );

    m_SectionLayout.ForceNewLine();
    m_SectionLayout.AddYGap();

    m_SectionLayout.SetFitWidthFlag( true );
    m_SectionLayout.SetSameLineFlag( false );

    m_SectionLayout.SetButtonWidth( 4*m_SectionLayout.GetRemainX()/5 );
    m_SectionLayout.AddOutput( m_NumSectOutput, "Num Sections" );
    m_SectionLayout.AddYGap();

    m_SectionLayout.SetButtonWidth( 74 );

    m_SectionLayout.AddDividerBox( "Num Interpolated XSecs" );
    m_SectionLayout.AddSlider( m_SectUTessSlider, "Num U", 20, " %5.0f" );

    m_SectionLayout.AddYGap();

    m_SectionLayout.AddDividerBox( "Section Planform" );

    m_SectionLayout.SetFitWidthFlag( true );
    m_SectionLayout.SetSameLineFlag( false );

    vector < string > wsect_driver_labels;
    wsect_driver_labels.resize( vsp::NUM_WSECT_DRIVER );

    wsect_driver_labels[vsp::AR_WSECT_DRIVER] = string( "AR" );
    wsect_driver_labels[vsp::SPAN_WSECT_DRIVER] = "Span";
    wsect_driver_labels[vsp::AREA_WSECT_DRIVER] = "Area";
    wsect_driver_labels[vsp::TAPER_WSECT_DRIVER] = "Taper";
    wsect_driver_labels[vsp::AVEC_WSECT_DRIVER] = "Ave C";
    wsect_driver_labels[vsp::ROOTC_WSECT_DRIVER] = "Root C";
    wsect_driver_labels[vsp::TIPC_WSECT_DRIVER] = "Tip C";
    wsect_driver_labels[vsp::SECSWEEP_WSECT_DRIVER] = "Sec SW";

    m_SectionLayout.SetButtonWidth( 50 );

    m_WingDriverGroupBank.SetDriverGroup( &m_DefaultWingDriverGroup );
    m_SectionLayout.AddDriverGroupBank( m_WingDriverGroupBank, wsect_driver_labels, 10, "%6.5f" );

    m_SectionLayout.SetButtonWidth( 4*m_SectionLayout.GetRemainX()/5 );

    m_SectionLayout.AddOutput( m_SectProjSpanOutput, "Projected Span" );

    m_SectionLayout.AddYGap();
    m_SectionLayout.SetButtonWidth( 74 );
    m_SectionLayout.AddDividerBox( "Sweep" );

    m_SectionLayout.AddSlider( m_SweepSlider, "Sweep", 10, "%6.5f" );
    m_SectionLayout.AddSlider( m_SweepLocSlider, "Sweep Loc", 10, "%6.5f" );
    m_SectionLayout.AddSlider( m_SecSweepLocSlider, "Sec SW Loc", 10, "%6.5f" );

    m_SectionLayout.AddYGap();
    m_SectionLayout.AddDividerBox( "Twist" );
    m_SectionLayout.AddSlider( m_TwistSlider, "Twist", 10, "%6.5f" );
    m_SectionLayout.AddSlider( m_TwistLocSlider, "Twist Loc", 10, "%6.5f" );

    m_SectionLayout.SetFitWidthFlag( false );
    m_SectionLayout.SetSameLineFlag( true );
    m_SectionLayout.AddLabel( "Reference:", 170 );
    m_SectionLayout.SetButtonWidth( m_SectionLayout.GetRemainX() / 2 );
    m_SectionLayout.AddButton( m_TwistRelativeToggle, "Rel" );
    m_SectionLayout.AddButton( m_TwistAbsoluteToggle, "Abs" );
    m_SectionLayout.ForceNewLine();

    m_TwistAbsRelToggle.Init( this );
    m_TwistAbsRelToggle.AddButton( m_TwistAbsoluteToggle.GetFlButton() );
    m_TwistAbsRelToggle.AddButton( m_TwistRelativeToggle.GetFlButton() );

    m_SectionLayout.SetFitWidthFlag( true );
    m_SectionLayout.SetSameLineFlag( false );

    m_SectionLayout.AddYGap();
    m_SectionLayout.AddDividerBox( "Dihedral" );
    m_SectionLayout.AddSlider( m_DihedralSlider, "Dihedral", 10, "%6.5f" );


    m_SectionLayout.SetFitWidthFlag( false );
    m_SectionLayout.SetSameLineFlag( true );
    m_SectionLayout.AddLabel( "Reference:", 170 );
    m_SectionLayout.SetButtonWidth( m_SectionLayout.GetRemainX() / 2 );
    m_SectionLayout.AddButton( m_DihedralRelativeToggle, "Rel" );
    m_SectionLayout.AddButton( m_DihedralAbsoluteToggle, "Abs" );
    m_SectionLayout.ForceNewLine();

    m_DihedralAbsRelToggle.Init( this );
    m_DihedralAbsRelToggle.AddButton( m_DihedralAbsoluteToggle.GetFlButton() );
    m_DihedralAbsRelToggle.AddButton( m_DihedralRelativeToggle.GetFlButton() );

    m_SectionLayout.SetButtonWidth( m_SectionLayout.GetRemainX() );
    m_SectionLayout.AddYGap();
    m_SectionLayout.AddButton( m_RotateFoilMatchDihedral, "Rotate Foil To Match Dihedral" );
    m_SectionLayout.SetButtonWidth( 74 );


    Fl_Group* af_tab = AddTab( "Airfoil" );
    Fl_Group* af_group = AddSubGroup( af_tab, 5 );

    m_AfLayout.SetGroupAndScreen( af_group, this );
    m_AfLayout.AddDividerBox( "Airfoil Section" );

    m_AfLayout.AddIndexSelector( m_AfIndexSelector );
    m_AfLayout.AddYGap();

    m_AfLayout.SetFitWidthFlag( false );
    m_AfLayout.SetSameLineFlag( true );
    m_AfLayout.SetButtonWidth( ( m_AfLayout.GetRemainX() - 10 ) / 2 );
    m_AfLayout.AddButton( m_CopyAfButton, "Copy" );
    m_AfLayout.AddX( 10 );
    m_AfLayout.AddButton( m_PasteAfButton, "Paste" );
    m_AfLayout.ForceNewLine();
    m_AfLayout.AddYGap();

    m_AfLayout.SetFitWidthFlag( true );
    m_AfLayout.SetSameLineFlag( false );

    m_AfLayout.AddDividerBox( "Type" );

    m_AfTypeChoice.AddItem( "POINT" );
    m_AfTypeChoice.AddItem( "CIRCLE" );
    m_AfTypeChoice.AddItem( "ELLIPSE" );
    m_AfTypeChoice.AddItem( "SUPER_ELLIPSE" );
    m_AfTypeChoice.AddItem( "ROUNDED_RECTANGLE" );
    m_AfTypeChoice.AddItem( "GENERAL_FUSE" );
    m_AfTypeChoice.AddItem( "FUSE_FILE" );
    m_AfTypeChoice.AddItem( "FOUR_SERIES" );
    m_AfTypeChoice.AddItem( "SIX_SERIES" );
    m_AfTypeChoice.AddItem( "BICONVEX" );
    m_AfTypeChoice.AddItem( "WEDGE" );
    m_AfTypeChoice.AddItem( "BEZIER" );
    m_AfTypeChoice.AddItem( "AF_FILE" );

    m_AfLayout.SetChoiceButtonWidth( 85 );
    m_AfLayout.SetButtonWidth( 40 );
    m_AfLayout.SetSameLineFlag( true );
    m_AfLayout.AddChoice( m_AfTypeChoice, "Choose Type:", m_AfLayout.GetButtonWidth() );
    m_AfLayout.SetFitWidthFlag( false );

    m_AfLayout.AddButton( m_ShowXSecButton, "Show" );
    m_AfLayout.ForceNewLine();

    m_AfLayout.SetFitWidthFlag( true );
    m_AfLayout.SetSameLineFlag( false );

    //==== Location To Start XSec Layouts ====//
    int start_y = m_AfLayout.GetY();

    //==== Super XSec ====//
    m_SuperGroup.SetGroupAndScreen( AddSubGroup( af_tab, 5 ), this );
    m_SuperGroup.SetY( start_y );
    m_SuperGroup.AddYGap();
    m_SuperGroup.AddSlider( m_SuperHeightSlider, "Height", 10, "%6.5f" );
    m_SuperGroup.AddSlider( m_SuperWidthSlider,  "Width", 10, "%6.5f" );
    m_SuperGroup.AddYGap();
    m_SuperGroup.AddSlider( m_SuperMSlider, "M", 10, "%6.5f" );
    m_SuperGroup.AddSlider( m_SuperNSlider, "N", 10, "%6.5f" );

    //==== Circle XSec ====//
    m_CircleGroup.SetGroupAndScreen( AddSubGroup( af_tab, 5 ), this );
    m_CircleGroup.SetY( start_y );
    m_CircleGroup.AddYGap();
    m_CircleGroup.AddSlider(  m_DiameterSlider, "Diameter", 10, "%6.5f" );

    //==== Ellipse XSec ====//
    m_EllipseGroup.SetGroupAndScreen( AddSubGroup( af_tab, 5 ), this );
    m_EllipseGroup.SetY( start_y );
    m_EllipseGroup.AddYGap();
    m_EllipseGroup.AddSlider(  m_EllipseHeightSlider, "Height", 10, "%6.5f" );
    m_EllipseGroup.AddSlider(  m_EllipseWidthSlider, "Width", 10, "%6.5f" );

    //==== Rounded Rect ====//
    m_RoundedRectGroup.SetGroupAndScreen( AddSubGroup( af_tab, 5 ), this );
    m_RoundedRectGroup.SetY( start_y );
    m_RoundedRectGroup.AddYGap();
    m_RoundedRectGroup.AddSlider( m_RRHeightSlider, "Height", 10, "%6.5f" );
    m_RoundedRectGroup.AddSlider( m_RRWidthSlider,  "Width", 10, "%6.5f" );
    m_RoundedRectGroup.AddYGap();
    m_RoundedRectGroup.AddSlider( m_RRRadiusSlider, "Radius", 10, "%6.5f" );

    //==== General Fuse XSec ====//
    m_GenGroup.SetGroupAndScreen( AddSubGroup( af_tab, 5 ), this );
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
    m_FourSeriesGroup.SetGroupAndScreen( AddSubGroup( af_tab, 5 ), this );
    m_FourSeriesGroup.SetY( start_y );
    m_FourSeriesGroup.AddYGap();
    m_FourSeriesGroup.AddOutput( m_FourNameOutput, "Name" );
    m_FourSeriesGroup.AddYGap();
    m_FourSeriesGroup.AddSlider( m_FourChordSlider, "Chord", 10, "%7.3f" );
    m_FourSeriesGroup.AddSlider( m_FourThickChordSlider, "T/C", 1, "%7.5f" );
    m_FourSeriesGroup.AddYGap();
    m_FourSeriesGroup.AddSlider( m_FourCamberSlider, "Camber", 0.2, "%7.5f" );
    m_FourSeriesGroup.AddSlider( m_FourCamberLocSlider, "CamberLoc", 1, "%7.5f" );
    m_FourSeriesGroup.AddYGap();
    m_FourSeriesGroup.AddButton( m_FourInvertButton, "Invert Airfoil" );

    //==== Sex Series AF ====//
    m_SixSeriesGroup.SetGroupAndScreen( AddSubGroup( af_tab, 5 ), this );
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
    m_BiconvexGroup.SetGroupAndScreen( AddSubGroup( af_tab, 5 ), this );
    m_BiconvexGroup.SetY( start_y );
    m_BiconvexGroup.AddYGap();
    m_BiconvexGroup.AddSlider( m_BiconvexChordSlider, "Chord", 10, "%7.3f" );
    m_BiconvexGroup.AddSlider( m_BiconvexThickChordSlider, "T/C", 1, "%7.5f" );

    //==== Wedge AF ====//
    m_WedgeGroup.SetGroupAndScreen(  AddSubGroup( af_tab, 5 ), this );
    m_WedgeGroup.SetY( start_y );
    m_WedgeGroup.AddYGap();
    m_WedgeGroup.AddSlider( m_WedgeChordSlider, "Chord", 10, "%7.3f" );
    m_WedgeGroup.AddSlider( m_WedgeThickChordSlider, "T/C", 1, "%7.5f" );
    m_WedgeGroup.AddYGap();
    m_WedgeGroup.AddSlider( m_WedgeThickLocSlider, "Thick_Loc", 1, "%7.5f" );

    //==== Fuse File ====//
    m_FuseFileGroup.SetGroupAndScreen( AddSubGroup( af_tab, 5 ), this );
    m_FuseFileGroup.SetY( start_y );
    m_FuseFileGroup.AddYGap();
    m_FuseFileGroup.AddButton( m_ReadFuseFileButton, "Read File" );
    m_FuseFileGroup.AddYGap();
    m_FuseFileGroup.AddSlider( m_FileHeightSlider, "Height", 10, "%7.3f" );
    m_FuseFileGroup.AddSlider( m_FileWidthSlider, "Width", 10, "%7.3f" );

    //==== Airfoil File ====//
    m_AfFileGroup.SetGroupAndScreen( AddSubGroup( af_tab, 5 ), this );
    m_AfFileGroup.SetY( start_y );
    m_AfFileGroup.AddYGap();
    m_AfFileGroup.AddButton( m_AfReadFileButton, "Read File" );
    m_AfFileGroup.AddYGap();
    m_AfFileGroup.AddOutput( m_AfFileNameOutput, "Name" );
    m_AfFileGroup.AddYGap();
    m_AfFileGroup.AddSlider( m_AfFileChordSlider, "Chord", 10, "%7.3f" );
    m_AfFileGroup.AddYGap();
    m_AfFileGroup.AddButton( m_AfFileInvertButton, "Invert Airfoil" );

    DisplayGroup( &m_PointGroup );

    //==== TE Trim ====//

    Fl_Group* modify_tab = AddTab( "Modify" );
    Fl_Group* modify_group = AddSubGroup( modify_tab, 5 );

    m_ModifyLayout.SetGroupAndScreen( modify_group, this );
    m_ModifyLayout.AddDividerBox( "Airfoil Section" );

    m_ModifyLayout.AddIndexSelector( m_AfModIndexSelector );

    m_ModifyLayout.AddYGap();

    m_ModifyLayout.InitWidthHeightVals();
    m_ModifyLayout.AddDividerBox( "Shift, Rotate, & Scale" );

    m_ModifyLayout.AddSlider( m_AFDeltaXSlider, "Delta X/C", 1, "%7.3f" );
    m_ModifyLayout.AddSlider( m_AFDeltaYSlider, "Delta Y/C", 1, "%7.3f" );
    m_ModifyLayout.AddSlider( m_AFThetaSlider, "Theta", 20, "%7.3f" );
    m_ModifyLayout.AddSlider( m_AFScaleSlider, "Scale", 1, "%7.3f" );
    m_ModifyLayout.AddSlider( m_AFShiftLESlider, "Shift LE", 1, "%7.3f" );

    m_ModifyLayout.SetChoiceButtonWidth( 80 );
    m_ModifyLayout.SetButtonWidth( 40 );

    m_ModifyLayout.AddYGap();
    m_ModifyLayout.AddDividerBox( "TE Closure" );

    m_TECloseChoice.AddItem( "NONE" );
    m_TECloseChoice.AddItem( "SKEW LOWER" );
    m_TECloseChoice.AddItem( "SKEW UPPER" );
    m_TECloseChoice.AddItem( "SKEW BOTH" );
    m_TECloseChoice.AddItem( "EXTRAPOLATE" );

    m_ModifyLayout.SetFitWidthFlag( true );
    m_ModifyLayout.SetSameLineFlag( true );

    m_ModifyLayout.AddChoice( m_TECloseChoice, "Type:", m_ModifyLayout.GetButtonWidth() * 2 );

    m_ModifyLayout.SetFitWidthFlag( false );
    m_ModifyLayout.AddButton( m_TECloseABSButton, "Abs" );
    m_ModifyLayout.AddButton( m_TECloseRELButton, "Rel" );

    m_TECloseGroup.Init( this );
    m_TECloseGroup.AddButton( m_TECloseABSButton.GetFlButton() );
    m_TECloseGroup.AddButton( m_TECloseRELButton.GetFlButton() );

    vector< int > close_val_map;
    close_val_map.push_back( vsp::ABS );
    close_val_map.push_back( vsp::REL );
    m_TECloseGroup.SetValMapVec( close_val_map );

    m_ModifyLayout.ForceNewLine();

    m_ModifyLayout.SetFitWidthFlag( true );
    m_ModifyLayout.SetSameLineFlag( false );

    m_ModifyLayout.AddSlider( m_CloseThickSlider, "T", 10.0, "%6.5f" );
    m_ModifyLayout.AddSlider( m_CloseThickChordSlider, "T/C", 1.0, "%6.5f" );

    m_ModifyLayout.AddYGap();
    m_ModifyLayout.AddDividerBox( "TE Trim" );

    m_TETrimChoice.AddItem( "NONE" );
    m_TETrimChoice.AddItem( "X" );
    m_TETrimChoice.AddItem( "THICK" );

    m_ModifyLayout.SetFitWidthFlag( true );
    m_ModifyLayout.SetSameLineFlag( true );

    m_ModifyLayout.AddChoice( m_TETrimChoice, "Type:", m_ModifyLayout.GetButtonWidth() * 2 );

    m_ModifyLayout.SetFitWidthFlag( false );
    m_ModifyLayout.AddButton( m_TETrimABSButton, "Abs" );
    m_ModifyLayout.AddButton( m_TETrimRELButton, "Rel" );

    m_TETrimGroup.Init( this );
    m_TETrimGroup.AddButton( m_TETrimABSButton.GetFlButton() );
    m_TETrimGroup.AddButton( m_TETrimRELButton.GetFlButton() );

    vector< int > trim_val_map;
    trim_val_map.push_back( vsp::ABS );
    trim_val_map.push_back( vsp::REL );
    m_TETrimGroup.SetValMapVec( trim_val_map );

    m_ModifyLayout.ForceNewLine();

    m_ModifyLayout.SetFitWidthFlag( true );
    m_ModifyLayout.SetSameLineFlag( false );

    m_ModifyLayout.AddSlider( m_TrimXSlider, "X", 10.0, "%6.5f" );
    m_ModifyLayout.AddSlider( m_TrimXChordSlider, "X/C", 10.0, "%6.5f" );
    m_ModifyLayout.AddSlider( m_TrimThickSlider, "T", 10.0, "%6.5f" );
    m_ModifyLayout.AddSlider( m_TrimThickChordSlider, "T/C", 1.0, "%6.5f" );

    m_SubSurfChoice.AddItem( SubSurface::GetTypeName( vsp::SS_CONTROL) );
    m_SubSurfChoice.UpdateItems();
}

//==== Show Wing Screen ====//
WingScreen::~WingScreen()
{
}

//==== Show Wing Screen ====//
void WingScreen::Show()
{
    if ( Update() )
    {
        m_FLTK_Window->show();
    }
}

//==== Update Wing Screen ====//
bool WingScreen::Update()
{
    assert( m_ScreenMgr );
    char str[256];

    Geom* geom_ptr = m_ScreenMgr->GetCurrGeom();
    if ( !geom_ptr || geom_ptr->GetType().m_Type != MS_WING_GEOM_TYPE )
    {
        Hide();
        return false;
    }

    GeomScreen::Update();
    m_NumUSlider.Deactivate();

    WingGeom* wing_ptr = dynamic_cast< WingGeom* >( geom_ptr );
    assert( wing_ptr );

    //==== Plan Parms ===//
    m_PlanSpanSlider.Update( wing_ptr->m_TotalSpan.GetID() );
    m_PlanProjSpanSlider.Update( wing_ptr->m_TotalProjSpan.GetID() );
    m_PlanChordSlider.Update( wing_ptr->m_TotalChord.GetID() );
    m_PlanAreaSlider.Update( wing_ptr->m_TotalArea.GetID() );

    sprintf( str, "%6.4f", wing_ptr->m_TotalProjSpan() * wing_ptr->m_TotalProjSpan() / wing_ptr->m_TotalArea() );
    m_PlanAROutput.Update( str );

    m_RootCapTypeChoice.Update( wing_ptr->m_CapUMinOption.GetID() );
    m_TipCapTypeChoice.Update( wing_ptr->m_CapUMaxOption.GetID() );
    if ( wing_ptr->m_CapUMinOption() == VspSurf::NO_END_CAP &&
         wing_ptr->m_CapUMaxOption() == VspSurf::NO_END_CAP )
    {
        m_CapTessSlider.Deactivate();
    }
    else
    {
        m_CapTessSlider.Update( wing_ptr->m_CapUMinTess.GetID() );
    }

    WingSect* root_sect = dynamic_cast<WingSect*>(wing_ptr->GetXSec( 0 ));

    if ( root_sect )
    {
        m_IncidenceSlider.Update( root_sect->m_Twist.GetID() );
        m_IncidenceLocSlider.Update( root_sect->m_TwistLoc.GetID() );
    }

    sprintf( str, "       %d", wing_ptr->NumXSec()-1 );
    m_NumSectOutput.Update(  str );

    ////==== Wing Section Index Display ====//
    int ws_index = wing_ptr->GetActiveXSecIndex();
    m_SectIndexSelector.SetIndex( ws_index );

    WingSect* wing_sect = dynamic_cast<WingSect*>(wing_ptr->GetXSec( ws_index ));

    if ( wing_sect )
    {
        m_SectUTessSlider.Update( wing_sect->m_SectTessU.GetID() );

        m_WingDriverGroupBank.SetDriverGroup( &wing_sect->m_DriverGroup );
        vector< string > parm_ids = wing_sect->GetDriverParms();
        wing_sect->m_DriverGroup.UpdateGroup( parm_ids );
        m_WingDriverGroupBank.Update( parm_ids );

        m_SweepSlider.Update( wing_sect->m_Sweep.GetID() );
        m_SweepLocSlider.Update( wing_sect->m_SweepLoc.GetID() );
        m_SecSweepLocSlider.Update( wing_sect->m_SecSweepLoc.GetID() );
        m_TwistSlider.Update( wing_sect->m_Twist.GetID() );
        m_TwistLocSlider.Update( wing_sect->m_TwistLoc.GetID() );

        m_DihedralSlider.Update( wing_sect->m_Dihedral.GetID() );
        m_DihedralAbsRelToggle.Update( wing_ptr->m_RelativeDihedralFlag.GetID() );
        m_TwistAbsRelToggle.Update( wing_ptr->m_RelativeTwistFlag.GetID() );

        m_RotateFoilMatchDihedral.Update( wing_ptr->m_RotateAirfoilMatchDiedralFlag.GetID() );

        sprintf( str, " %6.4f", wing_sect->GetProjectedSpan() );
        m_SectProjSpanOutput.Update(  str );

    }

    //==== XSec Index Display ===//
    int xsid = wing_ptr->GetActiveAirfoilIndex();
    m_AfIndexSelector.SetIndex( xsid );
    m_AfModIndexSelector.SetIndex( xsid );

    WingSect* ws = ( WingSect* ) wing_ptr->GetXSec( xsid );
    if ( ws )
    {
        XSecCurve* xsc = ws->GetXSecCurve();
        if ( xsc )
        {
            m_AfTypeChoice.SetVal( xsc->GetType() );

            if ( xsc->GetType() == XS_POINT )
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
                m_FourCamberLocSlider.Update( fs_xs->m_CamberLoc.GetID() );
                m_FourInvertButton.Update( fs_xs->m_Invert.GetID() );
                m_FourNameOutput.Update( fs_xs->GetAirfoilName() );
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
                m_AfFileInvertButton.Update( affile_xs->m_Invert.GetID() );
                m_AfFileNameOutput.Update( affile_xs->GetAirfoilName() );
            }


            m_TECloseChoice.Update( xsc->m_TECloseType.GetID() );
            m_TECloseGroup.Update( xsc->m_TECloseAbsRel.GetID() );

            m_CloseThickSlider.Update( xsc->m_TECloseThick.GetID() );
            m_CloseThickChordSlider.Update( xsc->m_TECloseThickChord.GetID() );

            if ( xsc->m_TECloseType() != CLOSE_NONE )
            {
                m_TECloseABSButton.Activate();
                m_TECloseRELButton.Activate();

                if ( xsc->m_TECloseAbsRel() == ABS )
                {
                    m_CloseThickSlider.Activate();
                    m_CloseThickChordSlider.Deactivate();
                }
                else
                {
                    m_CloseThickSlider.Deactivate();
                    m_CloseThickChordSlider.Activate();
                }
            }
            else
            {
                m_CloseThickSlider.Deactivate();
                m_CloseThickChordSlider.Deactivate();

                m_TECloseABSButton.Deactivate();
                m_TECloseRELButton.Deactivate();
            }

            m_TETrimChoice.Update( xsc->m_TETrimType.GetID() );
            m_TETrimGroup.Update( xsc->m_TETrimAbsRel.GetID() );


            m_TrimXSlider.Update( xsc->m_TETrimX.GetID() );
            m_TrimXChordSlider.Update( xsc->m_TETrimXChord.GetID() );
            m_TrimThickSlider.Update( xsc->m_TETrimThick.GetID() );
            m_TrimThickChordSlider.Update( xsc->m_TETrimThickChord.GetID() );

            m_TrimXSlider.Deactivate();
            m_TrimXChordSlider.Deactivate();
            m_TrimThickSlider.Deactivate();
            m_TrimThickChordSlider.Deactivate();
            m_TETrimABSButton.Deactivate();
            m_TETrimRELButton.Deactivate();

            if ( xsc->m_TETrimType() != TRIM_NONE )
            {
                m_TETrimABSButton.Activate();
                m_TETrimRELButton.Activate();
            }

            if ( xsc->m_TETrimType() == TRIM_X )
            {
                if ( xsc->m_TETrimAbsRel() == ABS )
                {
                    m_TrimXSlider.Activate();
                }
                else
                {
                    m_TrimXChordSlider.Activate();
                }
            }
            else if ( xsc->m_TETrimType() == TRIM_THICK )
            {
                if ( xsc->m_TETrimAbsRel() == ABS )
                {
                    m_TrimThickSlider.Activate();
                }
                else
                {
                    m_TrimThickChordSlider.Activate();
                }
            }

            m_AFThetaSlider.Update( xsc->m_Theta.GetID() );
            m_AFScaleSlider.Update( xsc->m_Scale.GetID() );
            m_AFDeltaXSlider.Update( xsc->m_DeltaX.GetID() );
            m_AFDeltaYSlider.Update( xsc->m_DeltaY.GetID() );
            m_AFShiftLESlider.Update( xsc->m_ShiftLE.GetID() );
        }
    }
    return true;
}

void WingScreen::DisplayGroup( GroupLayout* group )
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

    m_CurrDisplayGroup = group;

    if ( group )
    {
        group->Show();
    }
}

void WingScreen::GuiDeviceCallBack( GuiDevice* gui_device )
{
    //==== Find Fuselage Ptr ====//
    Geom* geom_ptr = m_ScreenMgr->GetCurrGeom();
    if ( !geom_ptr || geom_ptr->GetType().m_Type != MS_WING_GEOM_TYPE )
    {
        return;
    }
    WingGeom* wing_ptr = dynamic_cast< WingGeom* >( geom_ptr );
    assert( wing_ptr );

    if ( gui_device == &m_AfIndexSelector )
    {
        wing_ptr->SetActiveAirfoilIndex( m_AfIndexSelector.GetIndex() );
        wing_ptr->Update();
    }
    else if ( gui_device == &m_AfModIndexSelector )
    {
        wing_ptr->SetActiveAirfoilIndex( m_AfModIndexSelector.GetIndex() );
        wing_ptr->Update();
    }
    else if ( gui_device == &m_SectIndexSelector )
    {
        wing_ptr->SetActiveXSecIndex( m_SectIndexSelector.GetIndex() );
        wing_ptr->Update();
    }
    else if ( gui_device == &m_SplitSectButton )
    {
        int wsid = wing_ptr->GetActiveXSecIndex();
        wing_ptr->SplitWingSect( wsid );
        wing_ptr->Update();
    }
    else if ( gui_device == &m_CutSectButton )
    {
        int wsid = wing_ptr->GetActiveXSecIndex();
        wing_ptr->CutWingSect( wsid );
        wing_ptr->Update();
    }
    else if ( gui_device == &m_CopySectButton )
    {
        int wsid = wing_ptr->GetActiveXSecIndex();
        wing_ptr->CopyWingSect( wsid );
        wing_ptr->Update();
    }
    else if ( gui_device == &m_PasteSectButton )
    {
        int wsid = wing_ptr->GetActiveXSecIndex();
        wing_ptr->PasteWingSect( wsid );
        wing_ptr->Update();
    }
    else if ( gui_device == &m_InsertSectButton )
    {
        int wsid = wing_ptr->GetActiveXSecIndex();
        wing_ptr->InsertWingSect( wsid );
        wing_ptr->Update();
    }
    else if ( gui_device == &m_AfTypeChoice )
    {
        int t = m_AfTypeChoice.GetVal();
        wing_ptr->SetActiveAirfoilType( t );
        wing_ptr->Update();
    }
    else if ( gui_device == &m_ShowXSecButton )
    {
        m_ScreenMgr->ShowScreen( ScreenMgr::VSP_XSEC_SCREEN );
    }
    else if ( gui_device == &m_CopyAfButton   )
    {
        int afid = wing_ptr->GetActiveAirfoilIndex();
        wing_ptr->CopyAirfoil( afid );
    }
    else if ( gui_device == &m_PasteAfButton  )
    {
        int afid = wing_ptr->GetActiveAirfoilIndex();
        wing_ptr->PasteAirfoil(afid);
        wing_ptr->Update();
    }
    else if ( gui_device == &m_ReadFuseFileButton  )
    {
        int xsid = wing_ptr->GetActiveAirfoilIndex();
        XSec* xs = wing_ptr->GetXSec( xsid );
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
                    wing_ptr->Update();
                }
            }
        }
    }
    else if ( gui_device == &m_AfReadFileButton   )
    {
        int xsid = wing_ptr->GetActiveAirfoilIndex();
        XSec* xs = wing_ptr->GetXSec( xsid );
        if ( xs )
        {
            XSecCurve* xsc = xs->GetXSecCurve();
            if ( xsc )
            {
                if ( xsc->GetType() == XS_FILE_AIRFOIL  )
                {
                    FileAirfoil* affile_xs = dynamic_cast< FileAirfoil* >( xsc );
                    assert( affile_xs );
                    string newfile = m_ScreenMgr->GetSelectFileScreen()->FileChooser( "Airfoil File", "*.{af,dat}" );

                    affile_xs->ReadFile( newfile );
                    affile_xs->Update();
                    xs->Update();
                    wing_ptr->Update();
                }
            }
        }
    }
    else if ( gui_device == &m_TestDriverGroupButton )
    {
        int wsid = wing_ptr->GetActiveXSecIndex();

        WingSect* wing_sect = dynamic_cast<WingSect*>(wing_ptr->GetXSec( wsid ));

        if ( wing_sect )
        {
            vector< string > parm_ids = wing_sect->GetDriverParms();
            wing_sect->m_DriverGroup.Test( parm_ids, 1e-4 );
        }
    }

    GeomScreen::GuiDeviceCallBack( gui_device );
}

//==== Fltk  Callbacks ====//
void WingScreen::CallBack( Fl_Widget *w )
{
    GeomScreen::CallBack( w );
}
