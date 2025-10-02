//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//

// FeaPartEditScreen.cpp FeaPart editor screen.
// Justin Gravett
//
//////////////////////////////////////////////////////////////////////

#include "FeaPartEditScreen.h"
#include "StructureMgr.h"
#include "FeaMeshMgr.h"
#include "StlHelper.h"
#include "CurveEditScreen.h"

FeaPartEditScreen::FeaPartEditScreen( ScreenMgr* mgr ) : BasicScreen( mgr, 400, 475 + 80 + 60, "FEA Part Edit" )
{
    m_FLTK_Window->callback( staticCloseCB, this );

    m_CurFeaPartDispGroup = nullptr;

    int borderPaddingWidth = 5;
    int yPadding = 7;

    //=== FEA Parts ===//
    m_MainLayout.SetGroupAndScreen( m_FLTK_Window, this );

    //This sets position below heading with position at far left
    m_MainLayout.ForceNewLine();
    //Adds padding on left and top of position
    m_MainLayout.AddY( yPadding );
    m_MainLayout.AddX( borderPaddingWidth );

    m_MainLayout.AddSubGroupLayout( m_GenLayout, m_MainLayout.GetRemainX() - borderPaddingWidth, m_MainLayout.GetRemainY() - borderPaddingWidth);

    m_GenLayout.AddInput( m_FeaPartNameInput, "Part Name" );

    m_GenLayout.AddYGap();

    m_GenLayout.AddDividerBox( "Elements" );

    m_GenLayout.SetSameLineFlag( true );
    m_GenLayout.SetFitWidthFlag( false );

    m_GenLayout.SetButtonWidth( m_GenLayout.GetRemainX() / 2 );
    m_GenLayout.AddButton( m_KeepShellElementsToggle, "Keep Shell" );
    m_GenLayout.AddButton( m_DelShellElementsToggle, "Delete Shell" );

    m_KeepDelShellElementsToggleGroup.Init( this );
    m_KeepDelShellElementsToggleGroup.AddButton( m_KeepShellElementsToggle.GetFlButton() );
    m_KeepDelShellElementsToggleGroup.AddButton( m_DelShellElementsToggle.GetFlButton() );

    m_GenLayout.SetSameLineFlag( false );
    m_GenLayout.SetFitWidthFlag( true );
    m_GenLayout.ForceNewLine();

    m_GenLayout.AddButton( m_CreateBeamElementsToggle, "Create Beam Elements" );

    m_GenLayout.AddYGap();

    m_GenLayout.SetChoiceButtonWidth( m_GenLayout.GetRemainX() / 3 );

    m_GenLayout.AddChoice( m_ShellPropertyChoice, "Shell Property" );

    m_OrientationChoice.AddItem( "Global X", vsp::FEA_ORIENT_GLOBAL_X );
    m_OrientationChoice.AddItem( "Global Y", vsp::FEA_ORIENT_GLOBAL_Y );
    m_OrientationChoice.AddItem( "Global Z", vsp::FEA_ORIENT_GLOBAL_Z );
    m_OrientationChoice.AddItem( "Comp X", vsp::FEA_ORIENT_COMP_X );
    m_OrientationChoice.AddItem( "Comp Y", vsp::FEA_ORIENT_COMP_Y );
    m_OrientationChoice.AddItem( "Comp Z", vsp::FEA_ORIENT_COMP_Z );
    m_OrientationChoice.AddItem( "Part U", vsp::FEA_ORIENT_PART_U );
    m_OrientationChoice.AddItem( "Part V", vsp::FEA_ORIENT_PART_V );
    m_OrientationChoice.AddItem( "OML U", vsp::FEA_ORIENT_OML_U );
    m_OrientationChoice.AddItem( "OML V", vsp::FEA_ORIENT_OML_V );
    m_OrientationChoice.AddItem( "OML R", vsp::FEA_ORIENT_OML_R );
    m_OrientationChoice.AddItem( "OML S", vsp::FEA_ORIENT_OML_S );
    m_OrientationChoice.AddItem( "OML T", vsp::FEA_ORIENT_OML_T );
    m_OrientationChoice.UpdateItems();

    m_GenLayout.AddChoice( m_OrientationChoice, "Mat. Orientation" );
    m_GenLayout.AddChoice( m_CapPropertyChoice, "Cap Property" );

    m_GenLayout.AddYGap();

    // Individual FeaPart Parameters:
    int start_y = m_GenLayout.GetY();

    //==== FeaSkin ====//
    m_GenLayout.AddSubGroupLayout( m_SkinEditLayout, m_GenLayout.GetRemainX(), m_GenLayout.GetRemainY() );
    m_SkinEditLayout.SetY( start_y );

    m_SkinEditLayout.AddDividerBox( "Skin" );

    m_SkinEditLayout.AddButton( m_RemoveSkinToggle, "Remove Skin" );
    m_SkinEditLayout.AddButton( m_RemoveRootCapToggle, "Remove Root Cap" );
    m_SkinEditLayout.AddButton( m_RemoveTipCapToggle, "Remove Tip Cap" );

    //==== FeaSlice ====//
    m_GenLayout.AddSubGroupLayout( m_SliceEditLayout, m_GenLayout.GetRemainX(), m_GenLayout.GetRemainY() );
    m_SliceEditLayout.SetY( start_y );

    m_SliceEditLayout.AddDividerBox( "Slice" );

    m_SliceEditLayout.SetButtonWidth( m_SliceEditLayout.GetRemainX() / 3 );
    m_SliceEditLayout.SetChoiceButtonWidth( m_SliceEditLayout.GetRemainX() / 3 );

    m_SliceOrientationChoice.AddItem( "XY_Body" );
    m_SliceOrientationChoice.AddItem( "YZ_Body" );
    m_SliceOrientationChoice.AddItem( "XZ_Body" );
    m_SliceOrientationChoice.AddItem( "XY_Abs" );
    m_SliceOrientationChoice.AddItem( "YZ_Abs" );
    m_SliceOrientationChoice.AddItem( "XZ_Abs" );
    m_SliceOrientationChoice.AddItem( "Spine_Normal" );
    m_SliceEditLayout.AddChoice( m_SliceOrientationChoice, "Orientation" );

    m_SliceEditLayout.SetSameLineFlag( true );
    m_SliceEditLayout.SetFitWidthFlag( false );

    int button_width = m_SliceEditLayout.GetButtonWidth();

    m_SliceEditLayout.AddLabel( "Distance:", m_SliceEditLayout.GetRemainX() / 3 );
    m_SliceEditLayout.SetButtonWidth( m_SliceEditLayout.GetRemainX() / 2 );
    m_SliceEditLayout.AddButton( m_SlicePosRelToggle, "Relative" );
    m_SliceEditLayout.AddButton( m_SlicePosAbsToggle, "Model" );

    m_SlicePosTypeToggleGroup.Init( this );
    m_SlicePosTypeToggleGroup.AddButton( m_SlicePosAbsToggle.GetFlButton() );
    m_SlicePosTypeToggleGroup.AddButton( m_SlicePosRelToggle.GetFlButton() );

    m_SliceEditLayout.ForceNewLine();

    m_SliceEditLayout.SetSliderWidth( m_SliceEditLayout.GetRemainX() / 5 );

    m_SliceEditLayout.AddSlider( m_SliceCenterLocSlider, "Location", 0.5, "%5.3f" );

    m_SliceEditLayout.SetButtonWidth( m_SliceEditLayout.GetRemainX() );
    m_SliceEditLayout.AddButton( m_SlicePosUnit, " " );
    m_SlicePosUnit.GetFlButton()->box( FL_THIN_UP_BOX );
    m_SlicePosUnit.GetFlButton()->labelcolor( FL_BLACK );

    m_SliceEditLayout.ForceNewLine();
    m_SliceEditLayout.AddYGap();

    m_SliceEditLayout.InitWidthHeightVals();

    m_SliceEditLayout.AddLabel( "Rotation Axis", m_SliceEditLayout.GetRemainX() / 3 );

    m_SliceEditLayout.SetButtonWidth( m_SliceEditLayout.GetRemainX() / 3 );

    m_SliceEditLayout.AddButton( m_SliceXAxisToggle, "X Axis" );
    m_SliceEditLayout.AddButton( m_SliceYAxisToggle, "Y Axis" );
    m_SliceEditLayout.AddButton( m_SliceZAxisToggle, "Z Axis" );

    m_SliceRotAxisToggleGroup.Init( this );
    m_SliceRotAxisToggleGroup.AddButton( m_SliceXAxisToggle.GetFlButton() );
    m_SliceRotAxisToggleGroup.AddButton( m_SliceYAxisToggle.GetFlButton() );
    m_SliceRotAxisToggleGroup.AddButton( m_SliceZAxisToggle.GetFlButton() );

    m_SliceEditLayout.ForceNewLine();
    m_SliceEditLayout.SetSameLineFlag( false );
    m_SliceEditLayout.SetFitWidthFlag( true );

    m_SliceEditLayout.SetButtonWidth( button_width );

    m_SliceEditLayout.AddSlider( m_SliceXRotSlider, "XRot", 25, "%5.3f" );
    m_SliceEditLayout.AddSlider( m_SliceYRotSlider, "YRot", 25, "%5.3f" );
    m_SliceEditLayout.AddSlider( m_SliceZRotSlider, "ZRot", 25, "%5.3f" );

    //==== FeaRib ====//
    m_GenLayout.AddSubGroupLayout( m_RibEditLayout, m_GenLayout.GetRemainX(), m_GenLayout.GetRemainY() );
    m_RibEditLayout.SetY( start_y );

    m_RibEditLayout.AddDividerBox( "Rib" );

    m_RibEditLayout.SetSameLineFlag( true );
    m_RibEditLayout.SetFitWidthFlag( false );
    int label_width = 115;
    m_RibEditLayout.SetButtonWidth( ( m_RibEditLayout.GetRemainX() - label_width ) / 4 );

    m_RibEditLayout.AddLabel( "Normal Type:", label_width );
    m_RibEditLayout.AddButton( m_RibNoNormalToggle, "None" );
    m_RibEditLayout.AddButton( m_RibLENormalToggle, "LE" );
    m_RibEditLayout.AddButton( m_RibTENormalToggle, "TE" );
    m_RibEditLayout.AddButton( m_RibSparNormalToggle, "Spar" );

    m_RibNormalTypeGroup.Init( this );
    m_RibNormalTypeGroup.AddButton( m_RibNoNormalToggle.GetFlButton() );
    m_RibNormalTypeGroup.AddButton( m_RibLENormalToggle.GetFlButton() );
    m_RibNormalTypeGroup.AddButton( m_RibTENormalToggle.GetFlButton() );
    m_RibNormalTypeGroup.AddButton( m_RibSparNormalToggle.GetFlButton() );

    m_RibEditLayout.ForceNewLine();
    m_RibEditLayout.SetSameLineFlag( false );
    m_RibEditLayout.SetFitWidthFlag( true );
    m_RibEditLayout.SetButtonWidth( m_RibEditLayout.GetRemainX() / 2 );
    m_RibEditLayout.SetChoiceButtonWidth( m_RibEditLayout.GetRemainX() / 2 );

    m_RibEditLayout.AddChoice( m_RibPerpendicularSparChoice, "Perpendicular Spar" );

    m_RibEditLayout.AddYGap();

    m_RibEditLayout.AddButton( m_RibSectionLimitToggle, "Limit Rib to Section" );

    m_RibEditLayout.SetSameLineFlag( true );
    m_RibEditLayout.SetFitWidthFlag( false );

    button_width = m_RibEditLayout.GetButtonWidth();
    int input_width = m_RibEditLayout.GetInputWidth();

    m_RibEditLayout.SetButtonWidth( m_RibEditLayout.GetRemainX() / 11 );
    m_RibEditLayout.SetInputWidth( m_RibEditLayout.GetRemainX() / 12 );

    int labelwidth = m_RibEditLayout.GetRemainX() / 9;
    int gap = m_RibEditLayout.GetRemainX() / 34;

    m_RibEditLayout.AddLabel( "Start:", labelwidth );
    m_RibEditLayout.AddIndexSelector( m_RibStartSectIndexSelector );
    m_RibEditLayout.AddX( gap );
    m_RibEditLayout.AddLabel( "End:", labelwidth );
    m_RibEditLayout.AddIndexSelector( m_RibEndSectIndexSelector );

    m_RibEditLayout.ForceNewLine();
    m_RibEditLayout.AddYGap();

    m_RibEditLayout.AddLabel( "Distance:", m_RibEditLayout.GetRemainX() / 3 );
    m_RibEditLayout.SetButtonWidth( m_RibEditLayout.GetRemainX() / 2 );
    m_RibEditLayout.AddButton( m_RibPosRelToggle, "Relative" );
    m_RibEditLayout.AddButton( m_RibPosAbsToggle, "Model" );

    m_RibPosTypeToggleGroup.Init( this );
    m_RibPosTypeToggleGroup.AddButton( m_RibPosAbsToggle.GetFlButton() );
    m_RibPosTypeToggleGroup.AddButton( m_RibPosRelToggle.GetFlButton() );

    m_RibEditLayout.ForceNewLine();

    m_RibEditLayout.SetSliderWidth( m_RibEditLayout.GetRemainX() / 5 );
    m_RibEditLayout.SetInputWidth( input_width );

    m_RibEditLayout.AddSlider( m_RibPosSlider, "Location", 0.5, "%5.3f" );

    m_RibEditLayout.SetButtonWidth( m_RibEditLayout.GetRemainX() );
    m_RibEditLayout.AddButton( m_RibPosUnit, " " );
    m_RibPosUnit.GetFlButton()->box( FL_THIN_UP_BOX );
    m_RibPosUnit.GetFlButton()->labelcolor( FL_BLACK );

    m_RibEditLayout.ForceNewLine();
    m_RibEditLayout.AddYGap();

    m_RibEditLayout.InitWidthHeightVals();
    m_RibEditLayout.SetSameLineFlag( false );
    m_RibEditLayout.SetFitWidthFlag( true );

    m_RibEditLayout.SetButtonWidth( button_width );

    m_RibEditLayout.AddSlider( m_RibThetaSlider, "Rotation", 25, "%5.3f" );

    m_RibEditLayout.AddYGap();
    m_RibEditLayout.SetSameLineFlag( true );
    m_RibEditLayout.SetFitWidthFlag( false );
    m_RibEditLayout.SetButtonWidth( m_RibEditLayout.GetRemainX() / 2.0 );

    m_RibEditLayout.AddButton( m_RibTrimToBBoxToggle, "Trim to Bounding Box" );
    m_RibEditLayout.AddButton( m_RibRotateDihedralToggle, "Rotate to Match Dihedral" );

    m_RibEditLayout.ForceNewLine();
    m_RibEditLayout.SetSameLineFlag( false );
    m_RibEditLayout.SetFitWidthFlag( true );
    m_RibEditLayout.AddYGap();

    //==== FeaSpar ====//
    m_GenLayout.AddSubGroupLayout( m_SparEditLayout, m_GenLayout.GetRemainX(), m_GenLayout.GetRemainY() );
    m_SparEditLayout.SetY( start_y );

    m_SparEditLayout.AddDividerBox( "Spar" );

    m_SparEditLayout.SetButtonWidth( m_SparEditLayout.GetRemainX() / 2 );

    input_width = m_SparEditLayout.GetInputWidth();

    m_SparEditLayout.AddButton( m_SparSectionLimitToggle, "Limit Spar to Section" );

    m_SparEditLayout.SetSameLineFlag( true );
    m_SparEditLayout.SetFitWidthFlag( false );

    m_SparEditLayout.SetButtonWidth( m_SparEditLayout.GetRemainX() / 11 );
    m_SparEditLayout.SetInputWidth( m_SparEditLayout.GetRemainX() / 12 );

    labelwidth = m_SparEditLayout.GetRemainX() / 9;
    gap = m_SparEditLayout.GetRemainX() / 34;

    m_SparEditLayout.AddLabel( "Start:", labelwidth );
    m_SparEditLayout.AddIndexSelector( m_SparStartSectIndexSelector );
    m_SparEditLayout.AddX( gap );
    m_SparEditLayout.AddLabel( "End:", labelwidth );
    m_SparEditLayout.AddIndexSelector( m_SparEndSectIndexSelector );

    m_SparEditLayout.ForceNewLine();
    m_SparEditLayout.AddYGap();

    m_SparEditLayout.AddLabel( "Distance:", m_SparEditLayout.GetRemainX() / 3 );
    m_SparEditLayout.SetButtonWidth( m_SparEditLayout.GetRemainX() / 2 );
    m_SparEditLayout.AddButton( m_SparPosRelToggle, "Relative" );
    m_SparEditLayout.AddButton( m_SparPosAbsToggle, "Model" );

    m_SparPosTypeToggleGroup.Init( this );
    m_SparPosTypeToggleGroup.AddButton( m_SparPosAbsToggle.GetFlButton() );
    m_SparPosTypeToggleGroup.AddButton( m_SparPosRelToggle.GetFlButton() );

    m_SparEditLayout.ForceNewLine();
    m_SparEditLayout.SetSameLineFlag( false );
    m_SparEditLayout.SetFitWidthFlag( true );

    m_SparEditLayout.AddButton( m_SparSetPerChordToggle, "Parameterize by Chord" );

    m_SparEditLayout.AddYGap();
    m_SparEditLayout.SetSameLineFlag( true );
    m_SparEditLayout.SetFitWidthFlag( false );

    m_SparEditLayout.SetSliderWidth( m_SparEditLayout.GetRemainX() / 5 );
    m_SparEditLayout.SetInputWidth( input_width );

    m_SparEditLayout.AddSlider( m_SparPosSlider, "Location", 0.5, "%5.3f" );

    m_SparEditLayout.SetButtonWidth( m_SparEditLayout.GetRemainX() );
    m_SparEditLayout.AddButton( m_SparPosUnit, "" );
    m_SparPosUnit.GetFlButton()->box( FL_THIN_UP_BOX );
    m_SparPosUnit.GetFlButton()->labelcolor( FL_BLACK );

    m_SparEditLayout.ForceNewLine();
    m_SparEditLayout.SetSameLineFlag( false );
    m_SparEditLayout.SetFitWidthFlag( true );

    m_SparEditLayout.SetButtonWidth( m_SparEditLayout.GetRemainX() / 3 );

    m_SparEditLayout.AddSlider( m_SparThetaSlider, "Rotation", 25, "%5.3f" );

    m_SparEditLayout.AddYGap();

    m_SparEditLayout.AddSlider( m_SparRootChordSlider, "Root X/C", 0.5, "%5.3f" );
    m_SparEditLayout.AddSlider( m_SparTipChordSlider, "Tip X/C", 0.5, "%5.3f" );

    m_SparEditLayout.AddYGap();

    m_SparEditLayout.AddButton( m_SparTrimToBBoxToggle, "Trim to Bounding Box" );

    //==== FeaPolySpar ====//
    m_GenLayout.AddSubGroupLayout( m_PolySparEditLayout, m_GenLayout.GetRemainX(), m_GenLayout.GetRemainY() );
    m_PolySparEditLayout.SetY( start_y );

    m_PolySparEditLayout.AddDividerBox( "PolySpar" );

    int movebw = 20;
    int browser_h = 150;
    int start_x = m_PolySparEditLayout.GetX();
    start_y = m_PolySparEditLayout.GetY();

    m_PolySparEditLayout.AddSubGroupLayout( m_MovePointLayout, 20, browser_h );

    m_MovePointLayout.SetSameLineFlag( false );
    m_MovePointLayout.SetFitWidthFlag( false );

    m_MovePointLayout.SetButtonWidth( movebw );
    m_MovePointLayout.AddButton( m_MovePntTopButton, "@2<<" );
    m_MovePointLayout.AddYGap();
    m_MovePointLayout.AddButton( m_MovePntUpButton, "@2<" );
    m_MovePointLayout.AddY( browser_h - 4 * m_MovePointLayout.GetStdHeight() - 2 * m_MovePointLayout.GetGapHeight() );
    m_MovePointLayout.AddButton( m_MovePntDownButton, "@2>" );
    m_MovePointLayout.AddYGap();
    m_MovePointLayout.AddButton( m_MovePntBotButton, "@2>>" );

    m_PolySparEditLayout.SetY( start_y );
    m_PolySparEditLayout.AddX( movebw );
    m_PolySparEditLayout.SetFitWidthFlag( true );


    m_PolySparEditLayout.AddSubGroupLayout( m_PointBrowserLayout, m_PolySparEditLayout.GetRemainX(), browser_h );
    m_PolySparEditLayout.AddY( browser_h );

    // Pointer for the widths of each column in the browser to support resizing
    // Last column width must be 0
    static int col_widths[] = { 160, 85, 80, 85, 0 }; // widths for each column

    m_PolySparPointBrowser = m_PointBrowserLayout.AddColResizeBrowser( col_widths, 4, browser_h );
    m_PolySparPointBrowser->callback( staticScreenCB, this );
    m_PolySparPointBrowser->type( FL_MULTI_BROWSER );

    m_PolySparEditLayout.SetX( start_x );

    int bw = m_PolySparEditLayout.GetButtonWidth();

    m_PolySparEditLayout.SetButtonWidth( m_PolySparEditLayout.GetW() * 0.5 );

    m_PolySparEditLayout.SetFitWidthFlag( false );
    m_PolySparEditLayout.SetSameLineFlag( true );

    m_PolySparEditLayout.AddButton( m_AddPolySparPoint, "Add" );
    m_PolySparEditLayout.AddButton( m_DelPolySparPoint, "Delete" );

    m_PolySparEditLayout.ForceNewLine();

    m_PolySparEditLayout.AddButton( m_InsertPolySparPoint, "Insert Before" );
    m_PolySparEditLayout.AddButton( m_DelAllPolySparPoints, "Delete All" );

    m_PolySparEditLayout.SetButtonWidth( bw );

    m_PolySparEditLayout.ForceNewLine();
    m_PolySparEditLayout.AddYGap();

    m_PolySparEditLayout.SetFitWidthFlag( true );
    m_PolySparEditLayout.SetSameLineFlag( false );

    m_PolySparEditLayout.AddInput( m_PolySparPtNameInput, "Name" );
    m_PolySparEditLayout.AddYGap();

    int toggle_bw = 15;

    m_PolySparEditLayout.SetSameLineFlag( true );

    m_PolySparEditLayout.SetFitWidthFlag( false );
    m_PolySparEditLayout.SetButtonWidth( toggle_bw );
    m_PolySparEditLayout.AddButton( m_PolySparPointU01Toggle, "" );
    m_PolySparEditLayout.SetFitWidthFlag( true );
    m_PolySparEditLayout.SetButtonWidth( bw - toggle_bw );
    m_PolySparEditLayout.AddSlider( m_PolySparPointU01Slider, "U01", 1.0, "%5.3f" );
    m_PolySparEditLayout.ForceNewLine();

    m_PolySparEditLayout.SetFitWidthFlag( false );
    m_PolySparEditLayout.SetButtonWidth( toggle_bw );
    m_PolySparEditLayout.AddButton( m_PolySparPointU0NToggle, "" );
    m_PolySparEditLayout.SetFitWidthFlag( true );
    m_PolySparEditLayout.SetButtonWidth( bw - toggle_bw );
    m_PolySparEditLayout.AddSlider( m_PolySparPointU0NSlider, "U0N", 1.0, "%5.3f" );
    m_PolySparEditLayout.ForceNewLine();

    m_PolySparEditLayout.SetFitWidthFlag( false );
    m_PolySparEditLayout.SetButtonWidth( toggle_bw );
    m_PolySparEditLayout.AddButton( m_PolySparPointEtaToggle, "" );
    m_PolySparEditLayout.SetFitWidthFlag( true );
    m_PolySparEditLayout.SetButtonWidth( bw - toggle_bw );
    m_PolySparEditLayout.AddSlider( m_PolySparPointEtaSlider, "Eta", 1.0, "%5.3f" );
    m_PolySparEditLayout.ForceNewLine();

    m_PolySparPointTypeToggleGroup.Init( this );
    m_PolySparPointTypeToggleGroup.AddButton( m_PolySparPointU01Toggle.GetFlButton() );
    m_PolySparPointTypeToggleGroup.AddButton( m_PolySparPointU0NToggle.GetFlButton() );
    m_PolySparPointTypeToggleGroup.AddButton( m_PolySparPointEtaToggle.GetFlButton() );

    m_PolySparEditLayout.SetSameLineFlag( false );

    m_PolySparEditLayout.AddYGap();

    m_PolySparEditLayout.SetButtonWidth( bw );
    m_PolySparEditLayout.AddSlider( m_PolySparPointXoCSlider, "XoC", 1.0, "%5.3f" );


    //==== FeaFixPoint ====//
    m_GenLayout.AddSubGroupLayout( m_FixPointEditLayout, m_GenLayout.GetRemainX(), m_GenLayout.GetRemainY() );
    m_FixPointEditLayout.SetY( start_y );

    m_FixPointEditLayout.AddDividerBox( "Fixed Point" );

    m_FixPointEditLayout.SetChoiceButtonWidth( m_FixPointEditLayout.GetRemainX() / 3 );
    m_FixPointEditLayout.SetButtonWidth( m_FixPointEditLayout.GetRemainX() / 3 );

    m_FixPointEditLayout.AddChoice( m_FixPointTypeChoice, "Type" );
    m_FixPointTypeChoice.AddItem( "On Body", vsp::FEA_FIX_PT_ON_BODY );
    m_FixPointTypeChoice.AddItem( "Global XYZ", vsp::FEA_FIX_PT_GLOBAL_XYZ );
    m_FixPointTypeChoice.AddItem( "Delta XYZ", vsp::FEA_FIX_PT_DELTA_XYZ );
    m_FixPointTypeChoice.AddItem( "Delta UVN", vsp::FEA_FIX_PT_DELTA_UVN );
    m_FixPointTypeChoice.AddItem( "Geom Origin", vsp::FEA_FIX_PT_GEOM_ORIGIN );
    m_FixPointTypeChoice.AddItem( "Geom CG", vsp::FEA_FIX_PT_GEOM_CG );
    m_FixPointTypeChoice.UpdateItems();

    m_FixPointEditLayout.AddChoice( m_FixPointParentSurfChoice, "Parent Part" );

    // Not sure about this exclude list yet.
    m_FixPointOtherGeomPicker.AddExcludeType( MESH_GEOM_TYPE );
    m_FixPointOtherGeomPicker.AddExcludeType( HUMAN_GEOM_TYPE );
    m_FixPointOtherGeomPicker.AddExcludeType( PT_CLOUD_GEOM_TYPE );
    m_FixPointOtherGeomPicker.AddExcludeType( WIRE_FRAME_GEOM_TYPE );
    m_FixPointOtherGeomPicker.AddExcludeType( BLANK_GEOM_TYPE );
    m_FixPointEditLayout.AddGeomPicker( m_FixPointOtherGeomPicker, 0, "Parent Geom" );

    m_FixPointEditLayout.AddSlider( m_FixPointULocSlider, "U Location", 1, "%5.3f" );
    m_FixPointEditLayout.AddSlider( m_FixPointWLocSlider, "W Location", 1, "%5.3f" );
    m_FixPointEditLayout.AddSlider( m_FixPointAbsXSlider, "Abs X", 1, "%5.3f" );
    m_FixPointEditLayout.AddSlider( m_FixPointAbsYSlider, "Abs Y", 1, "%5.3f" );
    m_FixPointEditLayout.AddSlider( m_FixPointAbsZSlider, "Abs Z", 1, "%5.3f" );
    m_FixPointEditLayout.AddSlider( m_FixPointDeltaXSlider, "Delta X", 1, "%5.3f" );
    m_FixPointEditLayout.AddSlider( m_FixPointDeltaYSlider, "Delta Y", 1, "%5.3f" );
    m_FixPointEditLayout.AddSlider( m_FixPointDeltaZSlider, "Delta Z", 1, "%5.3f" );
    m_FixPointEditLayout.AddSlider( m_FixPointDeltaUSlider, "Delta U", 1, "%5.3f" );
    m_FixPointEditLayout.AddSlider( m_FixPointDeltaVSlider, "Delta V", 1, "%5.3f" );
    m_FixPointEditLayout.AddSlider( m_FixPointDeltaNSlider, "Delta N", 1, "%5.3f" );

    m_FixPointEditLayout.AddYGap();
    m_FixPointEditLayout.AddDividerBox( "Element" );

    m_FixPointEditLayout.SetSameLineFlag( true );
    m_FixPointEditLayout.SetFitWidthFlag( false );

    int togglew = m_FixPointEditLayout.GetRemainX() / 17;
    int gapw = 5.0;
    int massw = ( m_FixPointEditLayout.GetW() - gapw - togglew ) / 5.0;

    m_FixPointEditLayout.SetButtonWidth( 3 * massw + togglew );
    m_FixPointEditLayout.AddDividerBox( "Input" );
    m_FixPointEditLayout.AddX( gapw );
    m_FixPointEditLayout.SetButtonWidth( 2 * massw );
    m_FixPointEditLayout.AddDividerBox( "To FEM" );
    m_FixPointEditLayout.ForceNewLine();


    m_FixPointEditLayout.SetButtonWidth( togglew );
    m_FixPointEditLayout.AddButton( m_FixPointMassToggle, "" );

    m_FixPointEditLayout.SetButtonWidth( massw );
    m_FixPointEditLayout.SetSliderWidth( massw );
    m_FixPointEditLayout.SetInputWidth( massw );

    m_FixPointEditLayout.AddInput( m_FixPointMassInput, "Mass", "%5.3g" );

    m_FixPointEditLayout.SetChoiceButtonWidth( 0 );
    m_FixPointMassUnitChoice.AddItem( "lbm", vsp::MASS_UNIT_LBM );
    m_FixPointMassUnitChoice.AddItem( "slug", vsp::MASS_UNIT_SLUG );
    m_FixPointMassUnitChoice.AddItem( "lbf s^2/in", vsp::MASS_LBFSEC2IN );
    m_FixPointMassUnitChoice.AddItem( "g", vsp::MASS_UNIT_G );
    m_FixPointMassUnitChoice.AddItem( "kg", vsp::MASS_UNIT_KG );
    m_FixPointMassUnitChoice.AddItem( "tonne", vsp::MASS_UNIT_TONNE );
    m_FixPointEditLayout.AddChoice( m_FixPointMassUnitChoice, "" );

    m_FixPointEditLayout.AddX( gapw );

    m_FixPointEditLayout.SetButtonWidth( 0 );
    m_FixPointEditLayout.AddOutput( m_FixPointMass_FEMOutput, "", "%5.3g" );
    m_FixPointEditLayout.SetButtonWidth( massw );
    m_FixPointEditLayout.AddButton( m_FixPointMassUnit_FEM, "" );
    m_FixPointMassUnit_FEM.GetFlButton()->box( FL_THIN_UP_BOX );
    m_FixPointMassUnit_FEM.GetFlButton()->labelcolor( FL_BLACK );

    //==== FeaDome ====//
    m_GenLayout.AddSubGroupLayout( m_DomeEditLayout, m_GenLayout.GetRemainX(), m_GenLayout.GetRemainY() );
    m_DomeEditLayout.SetY( start_y );

    m_DomeEditLayout.AddDividerBox( "Dome" );

    m_DomeEditLayout.AddButton( m_DomeFlipDirButton, "Flip Direction" );

    m_DomeEditLayout.AddYGap();

    m_DomeEditLayout.AddButton( m_DomeSpineAttachButton, "Attach to Spine" );

    m_DomeEditLayout.SetButtonWidth( m_DomeEditLayout.GetRemainX() / 3 );

    m_DomeEditLayout.AddSlider( m_DomeUSpineSlider, "Spine U Loc", 1, "%5.3f" );

    m_DomeEditLayout.AddYGap();

    m_DomeEditLayout.SetSameLineFlag( true );
    m_DomeEditLayout.SetFitWidthFlag( false );

    m_DomeEditLayout.AddSlider( m_DomeARadSlider, "A Radius", 10, "%5.3f" );

    m_DomeEditLayout.SetButtonWidth( m_DomeEditLayout.GetRemainX() );
    m_DomeEditLayout.AddButton( m_DomeARadUnit, " " );
    m_DomeARadUnit.GetFlButton()->box( FL_THIN_UP_BOX );
    m_DomeARadUnit.GetFlButton()->labelcolor( FL_BLACK );

    m_DomeEditLayout.ForceNewLine();
    m_DomeEditLayout.SetButtonWidth( m_DomeEditLayout.GetRemainX() / 3 );

    m_DomeEditLayout.AddSlider( m_DomeBRadSlider, "B Radius", 10, "%5.3f" );

    m_DomeEditLayout.SetButtonWidth( m_DomeEditLayout.GetRemainX() );
    m_DomeEditLayout.AddButton( m_DomeBRadUnit, " " );
    m_DomeBRadUnit.GetFlButton()->box( FL_THIN_UP_BOX );
    m_DomeBRadUnit.GetFlButton()->labelcolor( FL_BLACK );

    m_DomeEditLayout.ForceNewLine();
    m_DomeEditLayout.SetButtonWidth( m_DomeEditLayout.GetRemainX() / 3 );

    m_DomeEditLayout.AddSlider( m_DomeCRadSlider, "C Radius", 10, "%5.3f" );

    m_DomeEditLayout.SetButtonWidth( m_DomeEditLayout.GetRemainX() );
    m_DomeEditLayout.AddButton( m_DomeCRadUnit, " " );
    m_DomeCRadUnit.GetFlButton()->box( FL_THIN_UP_BOX );
    m_DomeCRadUnit.GetFlButton()->labelcolor( FL_BLACK );

    m_DomeEditLayout.ForceNewLine();
    m_DomeEditLayout.AddYGap();
    m_DomeEditLayout.SetButtonWidth( m_DomeEditLayout.GetRemainX() / 3 );

    m_DomeEditLayout.AddSlider( m_DomeXSlider, "XLoc", 10, "%5.3f" );

    m_DomeEditLayout.SetButtonWidth( m_DomeEditLayout.GetRemainX() );
    m_DomeEditLayout.AddButton( m_DomeXUnit, " " );
    m_DomeXUnit.GetFlButton()->box( FL_THIN_UP_BOX );
    m_DomeXUnit.GetFlButton()->labelcolor( FL_BLACK );

    m_DomeEditLayout.ForceNewLine();
    m_DomeEditLayout.SetButtonWidth( m_DomeEditLayout.GetRemainX() / 3 );

    m_DomeEditLayout.AddSlider( m_DomeYSlider, "YLoc", 10, "%5.3f" );

    m_DomeEditLayout.SetButtonWidth( m_DomeEditLayout.GetRemainX() );
    m_DomeEditLayout.AddButton( m_DomeYUnit, " " );
    m_DomeYUnit.GetFlButton()->box( FL_THIN_UP_BOX );
    m_DomeYUnit.GetFlButton()->labelcolor( FL_BLACK );

    m_DomeEditLayout.ForceNewLine();
    m_DomeEditLayout.SetButtonWidth( m_DomeEditLayout.GetRemainX() / 3 );

    m_DomeEditLayout.AddSlider( m_DomeZSlider, "ZLoc", 10, "%5.3f" );

    m_DomeEditLayout.SetButtonWidth( m_DomeEditLayout.GetRemainX() );
    m_DomeEditLayout.AddButton( m_DomeZUnit, " " );
    m_DomeZUnit.GetFlButton()->box( FL_THIN_UP_BOX );
    m_DomeZUnit.GetFlButton()->labelcolor( FL_BLACK );

    m_DomeEditLayout.ForceNewLine();
    m_DomeEditLayout.AddYGap();

    m_DomeEditLayout.SetButtonWidth( m_DomeEditLayout.GetRemainX() / 3 );
    m_DomeEditLayout.SetSameLineFlag( false );
    m_DomeEditLayout.SetFitWidthFlag( true );

    m_DomeEditLayout.AddSlider( m_DomeXRotSlider, "XRot", 25, "%5.3f" );
    m_DomeEditLayout.AddSlider( m_DomeYRotSlider, "YRot", 25, "%5.3f" );
    m_DomeEditLayout.AddSlider( m_DomeZRotSlider, "ZRot", 25, "%5.3f" );

    //==== FeaRibArray ====//
    m_GenLayout.AddSubGroupLayout( m_RibArrayEditLayout, m_GenLayout.GetRemainX(), m_GenLayout.GetRemainY() );
    m_RibArrayEditLayout.SetY( start_y );

    m_RibArrayEditLayout.AddDividerBox( "RibArray" );

    m_RibArrayEditLayout.SetSameLineFlag( true );
    m_RibArrayEditLayout.SetFitWidthFlag( false );
    m_RibArrayEditLayout.SetButtonWidth( ( m_RibArrayEditLayout.GetRemainX() - label_width ) / 4 );

    m_RibArrayEditLayout.AddLabel( "Normal Type:", label_width );
    m_RibArrayEditLayout.AddButton( m_RibArrayNoNormalToggle, "None" );
    m_RibArrayEditLayout.AddButton( m_RibArrayLENormalToggle, "LE" );
    m_RibArrayEditLayout.AddButton( m_RibArrayTENormalToggle, "TE" );
    m_RibArrayEditLayout.AddButton( m_RibArraySparNormalToggle, "Spar" );

    m_RibArrayNormalTypeGroup.Init( this );
    m_RibArrayNormalTypeGroup.AddButton( m_RibArrayNoNormalToggle.GetFlButton() );
    m_RibArrayNormalTypeGroup.AddButton( m_RibArrayLENormalToggle.GetFlButton() );
    m_RibArrayNormalTypeGroup.AddButton( m_RibArrayTENormalToggle.GetFlButton() );
    m_RibArrayNormalTypeGroup.AddButton( m_RibArraySparNormalToggle.GetFlButton() );

    m_RibArrayEditLayout.ForceNewLine();
    m_RibArrayEditLayout.SetSameLineFlag( false );
    m_RibArrayEditLayout.SetFitWidthFlag( true );
    m_RibArrayEditLayout.SetButtonWidth( m_RibArrayEditLayout.GetRemainX() / 2 );
    m_RibArrayEditLayout.SetChoiceButtonWidth( m_RibArrayEditLayout.GetRemainX() / 2 );

    m_RibArrayEditLayout.AddChoice( m_RibArrayPerpendicularSparChoice, "Perpendicular Spar" );

    m_RibArrayEditLayout.AddYGap();

    m_RibArrayEditLayout.AddButton( m_RibArraySectionLimitToggle, "Limit Rib Array to Section" );

    m_RibArrayEditLayout.SetSameLineFlag( true );
    m_RibArrayEditLayout.SetFitWidthFlag( false );

    button_width = m_RibArrayEditLayout.GetButtonWidth();
    input_width = m_RibArrayEditLayout.GetInputWidth();

    m_RibArrayEditLayout.SetButtonWidth( m_RibArrayEditLayout.GetRemainX() / 11 );
    m_RibArrayEditLayout.SetInputWidth( m_RibArrayEditLayout.GetRemainX() / 12 );

    labelwidth = m_RibArrayEditLayout.GetRemainX() / 9;
    gap = m_RibArrayEditLayout.GetRemainX() / 34;

    m_RibArrayEditLayout.AddLabel( "Start:", labelwidth );
    m_RibArrayEditLayout.AddIndexSelector( m_RibArrayStartSectIndexSelector );
    m_RibArrayEditLayout.AddX( gap );
    m_RibArrayEditLayout.AddLabel( "End:", labelwidth );
    m_RibArrayEditLayout.AddIndexSelector( m_RibArrayEndSectIndexSelector );

    m_RibArrayEditLayout.AddYGap();
    m_RibArrayEditLayout.ForceNewLine();

    m_RibArrayEditLayout.AddLabel( "Distance:", m_RibArrayEditLayout.GetRemainX() / 3 );
    m_RibArrayEditLayout.SetButtonWidth( m_RibArrayEditLayout.GetRemainX() / 2 );
    m_RibArrayEditLayout.AddButton( m_RibArrayPosRelToggle, "Relative" );
    m_RibArrayEditLayout.AddButton( m_RibArrayPosAbsToggle, "Model" );

    m_RibArrayPosTypeToggleGroup.Init( this );
    m_RibArrayPosTypeToggleGroup.AddButton( m_RibArrayPosAbsToggle.GetFlButton() );
    m_RibArrayPosTypeToggleGroup.AddButton( m_RibArrayPosRelToggle.GetFlButton() );

    m_RibArrayEditLayout.ForceNewLine();

    m_RibArrayEditLayout.SetButtonWidth( m_RibArrayEditLayout.GetRemainX() / 3 );

    m_RibArrayEditLayout.SetSameLineFlag( true );
    m_RibArrayEditLayout.SetFitWidthFlag( false );

    m_RibArrayEditLayout.AddLabel( "Direction", m_RibArrayEditLayout.GetRemainX() / 3 );
    m_RibArrayEditLayout.AddButton( m_RibArrayPosDirToggle, "Positive" );
    m_RibArrayEditLayout.AddButton( m_RibArrayNegDirToggle, "Negative" );

    m_RibArrayPosNegDirToggleGroup.Init( this );
    m_RibArrayPosNegDirToggleGroup.AddButton( m_RibArrayNegDirToggle.GetFlButton() );
    m_RibArrayPosNegDirToggleGroup.AddButton( m_RibArrayPosDirToggle.GetFlButton() );

    m_RibArrayEditLayout.ForceNewLine();

    button_width = m_RibArrayEditLayout.GetButtonWidth();

    m_RibArrayEditLayout.SetSliderWidth( m_RibArrayEditLayout.GetRemainX() / 5 );
    m_RibArrayEditLayout.SetInputWidth( input_width );

    m_RibArrayEditLayout.AddSlider( m_RibArrayStartLocSlider, "Start Location", 0.5, "%5.3f" );

    m_RibArrayEditLayout.SetButtonWidth( m_RibArrayEditLayout.GetRemainX() );
    m_RibArrayEditLayout.AddButton( m_RibArrayStartLocUnit, " " );
    m_RibArrayStartLocUnit.GetFlButton()->box( FL_THIN_UP_BOX );
    m_RibArrayStartLocUnit.GetFlButton()->labelcolor( FL_BLACK );

    m_RibArrayEditLayout.ForceNewLine();
    m_RibArrayEditLayout.SetButtonWidth( m_RibArrayEditLayout.GetRemainX() / 3 );

    m_RibArrayEditLayout.AddSlider( m_RibArrayEndLocSlider, "End Location", 0.5, "%5.3f" );

    m_RibArrayEditLayout.SetButtonWidth( m_RibArrayEditLayout.GetRemainX() );
    m_RibArrayEditLayout.AddButton( m_RibArrayEndLocUnit, " " );
    m_RibArrayEndLocUnit.GetFlButton()->box( FL_THIN_UP_BOX );
    m_RibArrayEndLocUnit.GetFlButton()->labelcolor( FL_BLACK );

    m_RibArrayEditLayout.ForceNewLine();
    m_RibArrayEditLayout.SetButtonWidth( m_RibArrayEditLayout.GetRemainX() / 3 );

    m_RibArrayEditLayout.AddSlider( m_RibArraySpacingSlider, "Spacing", 0.5, "%5.3f" );

    m_RibArrayEditLayout.SetButtonWidth( m_RibArrayEditLayout.GetRemainX() );
    m_RibArrayEditLayout.AddButton( m_RibArrayPosUnit, " " );
    m_RibArrayPosUnit.GetFlButton()->box( FL_THIN_UP_BOX );
    m_RibArrayPosUnit.GetFlButton()->labelcolor( FL_BLACK );

    m_RibArrayEditLayout.ForceNewLine();
    m_RibArrayEditLayout.AddYGap();

    m_RibArrayEditLayout.InitWidthHeightVals();
    m_RibArrayEditLayout.SetSameLineFlag( false );
    m_RibArrayEditLayout.SetFitWidthFlag( true );

    m_RibArrayEditLayout.SetButtonWidth( button_width );

    m_RibArrayEditLayout.AddSlider( m_RibArrayThetaSlider, "Rotation", 25, "%5.3f" );

    m_RibArrayEditLayout.AddYGap();
    m_RibArrayEditLayout.SetSameLineFlag( true );
    m_RibArrayEditLayout.SetFitWidthFlag( false );
    m_RibArrayEditLayout.SetButtonWidth( m_RibArrayEditLayout.GetRemainX() / 2.0 );

    m_RibArrayEditLayout.AddButton( m_RibArrayTrimToBBoxToggle, "Trim to Bounding Box" );
    m_RibArrayEditLayout.AddButton( m_RibArrayRotateDihedralToggle, "Rotate to Match Dihedral" );

    m_RibArrayEditLayout.ForceNewLine();
    m_RibArrayEditLayout.SetSameLineFlag( false );
    m_RibArrayEditLayout.SetFitWidthFlag( true );
    m_RibArrayEditLayout.AddYGap();

    m_RibArrayEditLayout.AddButton( m_IndividualizeRibArrayButton, "Individualize Rib Array" );

    //==== FeaSliceArray ====//
    m_GenLayout.AddSubGroupLayout( m_SliceArrayEditLayout, m_GenLayout.GetRemainX(), m_GenLayout.GetRemainY() );
    m_SliceArrayEditLayout.SetY( start_y );

    m_SliceArrayEditLayout.AddDividerBox( "SliceArray" );

    m_SliceArrayEditLayout.SetButtonWidth( m_SliceArrayEditLayout.GetRemainX() / 3 );
    m_SliceArrayEditLayout.SetChoiceButtonWidth( m_SliceArrayEditLayout.GetRemainX() / 3 );

    m_SliceArrayOrientationChoice.AddItem( "XY_Body" );
    m_SliceArrayOrientationChoice.AddItem( "YZ_Body" );
    m_SliceArrayOrientationChoice.AddItem( "XZ_Body" );
    m_SliceArrayOrientationChoice.AddItem( "XY_Abs" );
    m_SliceArrayOrientationChoice.AddItem( "YZ_Abs" );
    m_SliceArrayOrientationChoice.AddItem( "XZ_Abs" );
    m_SliceArrayOrientationChoice.AddItem( "Spine_Normal" );
    m_SliceArrayEditLayout.AddChoice( m_SliceArrayOrientationChoice, "Orientation" );

    m_SliceArrayEditLayout.SetSameLineFlag( true );
    m_SliceArrayEditLayout.SetFitWidthFlag( false );

    m_SliceArrayEditLayout.AddLabel( "Distance:", m_SliceArrayEditLayout.GetRemainX() / 3 );
    m_SliceArrayEditLayout.SetButtonWidth( m_SliceArrayEditLayout.GetRemainX() / 2 );
    m_SliceArrayEditLayout.AddButton( m_SliceArrayPosRelToggle, "Relative" );
    m_SliceArrayEditLayout.AddButton( m_SliceArrayPosAbsToggle, "Model" );

    m_SliceArrayPosTypeToggleGroup.Init( this );
    m_SliceArrayPosTypeToggleGroup.AddButton( m_SliceArrayPosAbsToggle.GetFlButton() );
    m_SliceArrayPosTypeToggleGroup.AddButton( m_SliceArrayPosRelToggle.GetFlButton() );

    m_SliceArrayEditLayout.ForceNewLine();

    m_SliceArrayEditLayout.SetButtonWidth( m_SliceArrayEditLayout.GetRemainX() / 3 );

    m_SliceArrayEditLayout.AddLabel( "Direction", m_SliceArrayEditLayout.GetRemainX() / 3 );
    m_SliceArrayEditLayout.AddButton( m_SliceArrayPosDirToggle, "Positive" );
    m_SliceArrayEditLayout.AddButton( m_SliceArrayNegDirToggle, "Negative" );

    m_SliceArrayPosNegDirToggleGroup.Init( this );
    m_SliceArrayPosNegDirToggleGroup.AddButton( m_SliceArrayNegDirToggle.GetFlButton() );
    m_SliceArrayPosNegDirToggleGroup.AddButton( m_SliceArrayPosDirToggle.GetFlButton() );

    m_SliceArrayEditLayout.ForceNewLine();

    button_width = m_SliceArrayEditLayout.GetButtonWidth();

    m_SliceArrayEditLayout.SetSliderWidth( m_SliceArrayEditLayout.GetRemainX() / 5 );

    m_SliceArrayEditLayout.AddSlider( m_SliceArrayStartLocSlider, "Start Location", 0.5, "%5.3f" );

    m_SliceArrayEditLayout.SetButtonWidth( m_SliceArrayEditLayout.GetRemainX() );
    m_SliceArrayEditLayout.AddButton( m_SliceArrayStartLocUnit, " " );
    m_SliceArrayStartLocUnit.GetFlButton()->box( FL_THIN_UP_BOX );
    m_SliceArrayStartLocUnit.GetFlButton()->labelcolor( FL_BLACK );

    m_SliceArrayEditLayout.ForceNewLine();
    m_SliceArrayEditLayout.SetButtonWidth( m_SliceArrayEditLayout.GetRemainX() / 3 );

    m_SliceArrayEditLayout.AddSlider( m_SliceArrayEndLocSlider, "End Location", 0.5, "%5.3f" );

    m_SliceArrayEditLayout.SetButtonWidth( m_SliceArrayEditLayout.GetRemainX() );
    m_SliceArrayEditLayout.AddButton( m_SliceArrayEndLocUnit, " " );
    m_SliceArrayEndLocUnit.GetFlButton()->box( FL_THIN_UP_BOX );
    m_SliceArrayEndLocUnit.GetFlButton()->labelcolor( FL_BLACK );

    m_SliceArrayEditLayout.ForceNewLine();
    m_SliceArrayEditLayout.SetButtonWidth( m_SliceArrayEditLayout.GetRemainX() / 3 );

    m_SliceArrayEditLayout.AddSlider( m_SliceArraySpacingSlider, "Spacing", 0.5, "%5.3f" );

    m_SliceArrayEditLayout.SetButtonWidth( m_SliceArrayEditLayout.GetRemainX() );
    m_SliceArrayEditLayout.AddButton( m_SliceArrayPosUnit, " " );
    m_SliceArrayPosUnit.GetFlButton()->box( FL_THIN_UP_BOX );
    m_SliceArrayPosUnit.GetFlButton()->labelcolor( FL_BLACK );

    m_SliceArrayEditLayout.ForceNewLine();
    m_SliceArrayEditLayout.AddYGap();

    m_SliceArrayEditLayout.InitWidthHeightVals();
    m_SliceArrayEditLayout.SetSameLineFlag( true );
    m_SliceArrayEditLayout.SetFitWidthFlag( false );

    m_SliceArrayEditLayout.SetButtonWidth( button_width );

    m_SliceArrayEditLayout.AddLabel( "Rotation Axis", m_SliceArrayEditLayout.GetRemainX() / 3 );

    m_SliceArrayEditLayout.SetButtonWidth( m_SliceArrayEditLayout.GetRemainX() / 3 );

    m_SliceArrayEditLayout.AddButton( m_SliceArrayXAxisToggle, "X Axis" );
    m_SliceArrayEditLayout.AddButton( m_SliceArrayYAxisToggle, "Y Axis" );
    m_SliceArrayEditLayout.AddButton( m_SliceArrayZAxisToggle, "Z Axis" );

    m_SliceArrayRotAxisToggleGroup.Init( this );
    m_SliceArrayRotAxisToggleGroup.AddButton( m_SliceArrayXAxisToggle.GetFlButton() );
    m_SliceArrayRotAxisToggleGroup.AddButton( m_SliceArrayYAxisToggle.GetFlButton() );
    m_SliceArrayRotAxisToggleGroup.AddButton( m_SliceArrayZAxisToggle.GetFlButton() );

    m_SliceArrayEditLayout.ForceNewLine();
    m_SliceArrayEditLayout.SetSameLineFlag( false );
    m_SliceArrayEditLayout.SetFitWidthFlag( true );

    m_SliceArrayEditLayout.SetButtonWidth( button_width );

    m_SliceArrayEditLayout.AddSlider( m_SliceArrayXRotSlider, "XRot", 25, "%5.3f" );
    m_SliceArrayEditLayout.AddSlider( m_SliceArrayYRotSlider, "YRot", 25, "%5.3f" );
    m_SliceArrayEditLayout.AddSlider( m_SliceArrayZRotSlider, "ZRot", 25, "%5.3f" );

    m_SliceArrayEditLayout.AddYGap();

    m_SliceArrayEditLayout.AddButton( m_IndividualizeSliceArrayButton, "Individualize Slice Array" );

    //==== FeaTrimPart ====//
    m_GenLayout.AddSubGroupLayout( m_TrimEditLayout, m_GenLayout.GetRemainX(), m_GenLayout.GetRemainY() );
    m_TrimEditLayout.SetY( start_y );

    m_TrimEditLayout.AddDividerBox( "Trim" );

    m_TrimEditLayout.SetFitWidthFlag( true );
    m_TrimEditLayout.SetSameLineFlag( false );


    // Pointer for the widths of each column in the browser to support resizing
    // Last column width must be 0
    static int trim_part_col_widths[] = { m_TrimEditLayout.GetW()-75, 75, 0 }; // 2 columns

    m_TrimPartBrowser = m_TrimEditLayout.AddColResizeBrowser( trim_part_col_widths, 2, 200 );
    m_TrimPartBrowser->callback( staticScreenCB, this );

    m_TrimEditLayout.SetFitWidthFlag( true );
    m_TrimEditLayout.SetSameLineFlag( true );

    m_TrimEditLayout.SetChoiceButtonWidth( 75 );

    m_TrimEditLayout.AddChoice( m_TrimPartChoice, "Trim Part", 75 );

    m_TrimEditLayout.SetFitWidthFlag( false );
    m_TrimEditLayout.AddButton( m_FlipTrimDirButton, "Flip Dir" );

    m_TrimEditLayout.ForceNewLine();

    m_TrimEditLayout.SetButtonWidth( m_TrimEditLayout.GetW() * 0.5 );
    m_TrimEditLayout.AddButton( m_AddTrimPartButton, "Add" );
    m_TrimEditLayout.AddButton( m_DeleteTrimPartButton, "Delete" );

    //=== SubSurfaces ===//

    //==== SSLine ====//
    m_GenLayout.AddSubGroupLayout( m_FeaSSLineGroup, m_GenLayout.GetRemainX(), m_GenLayout.GetRemainY() );
    m_FeaSSLineGroup.SetY( start_y );

    m_FeaSSLineGroup.AddDividerBox( "Line Sub-Surface" );

    m_FeaSSLineGroup.SetFitWidthFlag( false );
    m_FeaSSLineGroup.SetSameLineFlag( true );
    m_FeaSSLineGroup.AddLabel( "Tag", m_FeaSSLineGroup.GetRemainX() / 3 );
    m_FeaSSLineGroup.SetButtonWidth( m_FeaSSLineGroup.GetRemainX() / 3 );
    m_FeaSSLineGroup.AddButton( m_FeaSSLineGreaterToggle, "Greater" );
    m_FeaSSLineGroup.AddButton( m_FeaSSLineLessToggle, "Less" );
    m_FeaSSLineGroup.AddButton( m_FeaSSLineNoneToggle, "None" );

    m_FeaSSLineTestToggleGroup.Init( this );
    m_FeaSSLineTestToggleGroup.AddButton( m_FeaSSLineGreaterToggle.GetFlButton() );
    m_FeaSSLineTestToggleGroup.AddButton( m_FeaSSLineLessToggle.GetFlButton() );
    m_FeaSSLineTestToggleGroup.AddButton( m_FeaSSLineNoneToggle.GetFlButton() );

    m_FeaSSLineGroup.ForceNewLine();

    m_FeaSSLineGroup.AddLabel( "Line Type", m_FeaSSLineGroup.GetRemainX() / 3 );
    m_FeaSSLineGroup.SetButtonWidth( m_FeaSSLineGroup.GetRemainX() / 2 );
    m_FeaSSLineGroup.AddButton( m_FeaSSLineConstUButton, "U" );
    m_FeaSSLineGroup.AddButton( m_FeaSSLineConstWButton, "W" );

    m_FeaSSLineConstToggleGroup.Init( this );
    m_FeaSSLineConstToggleGroup.AddButton( m_FeaSSLineConstUButton.GetFlButton() );
    m_FeaSSLineConstToggleGroup.AddButton( m_FeaSSLineConstWButton.GetFlButton() );

    m_FeaSSLineGroup.ForceNewLine();
    m_FeaSSLineGroup.SetFitWidthFlag( true );
    m_FeaSSLineGroup.SetSameLineFlag( false );

    m_FeaSSLineGroup.SetButtonWidth( m_FeaSSLineGroup.GetRemainX() / 3 );
    m_FeaSSLineGroup.SetChoiceButtonWidth( m_FeaSSLineGroup.GetRemainX() / 3 );

    m_FeaSSLineGroup.SetButtonWidth( m_FeaSSLineGroup.GetRemainX() / 3 );
    m_FeaSSLineGroup.AddSlider( m_FeaSSLineConstSlider, "Value", 1, "%5.4f" );

    //==== SSRectangle ====//
    m_GenLayout.AddSubGroupLayout( m_FeaSSRecGroup, m_GenLayout.GetRemainX(), m_GenLayout.GetRemainY() );
    m_FeaSSRecGroup.SetY( start_y );

    m_FeaSSRecGroup.AddDividerBox( "Rectangle Sub-Surface" );

    m_FeaSSRecGroup.SetFitWidthFlag( false );
    m_FeaSSRecGroup.SetSameLineFlag( true );
    m_FeaSSRecGroup.AddLabel( "Tag", m_FeaSSRecGroup.GetRemainX() / 3 );
    m_FeaSSRecGroup.SetButtonWidth( m_FeaSSRecGroup.GetRemainX() / 3 );
    m_FeaSSRecGroup.AddButton( m_FeaSSRecInsideButton, "Inside" );
    m_FeaSSRecGroup.AddButton( m_FeaSSRecOutsideButton, "Outside" );
    m_FeaSSRecGroup.AddButton( m_FeaSSRecNoneButton, "None" );

    m_FeaSSRecTestToggleGroup.Init( this );
    m_FeaSSRecTestToggleGroup.AddButton( m_FeaSSRecInsideButton.GetFlButton() );
    m_FeaSSRecTestToggleGroup.AddButton( m_FeaSSRecOutsideButton.GetFlButton() );
    m_FeaSSRecTestToggleGroup.AddButton( m_FeaSSRecNoneButton.GetFlButton() );

    m_FeaSSRecGroup.ForceNewLine();
    m_FeaSSRecGroup.SetFitWidthFlag( true );
    m_FeaSSRecGroup.SetSameLineFlag( false );

    m_FeaSSRecGroup.SetButtonWidth( m_FeaSSRecGroup.GetRemainX() / 3 );
    m_FeaSSRecGroup.SetChoiceButtonWidth( m_FeaSSRecGroup.GetRemainX() / 3 );

    m_FeaSSRecGroup.AddSlider( m_FeaSSRecCentUSlider, "Center U", 1, "%5.4f" );
    m_FeaSSRecGroup.AddSlider( m_FeaSSRecCentWSlider, "Center W", 1, "%5.4f" );
    m_FeaSSRecGroup.AddSlider( m_FeaSSRecULenSlider, "U Length", 1, "%5.4f" );
    m_FeaSSRecGroup.AddSlider( m_FeaSSRecWLenSlider, "W Length", 1, "%5.4f" );
    m_FeaSSRecGroup.AddSlider( m_FeaSSRecThetaSlider, "Theta", 25, "%5.4f" );

    //==== SS_Ellipse ====//
    m_GenLayout.AddSubGroupLayout( m_FeaSSEllGroup, m_GenLayout.GetRemainX(), m_GenLayout.GetRemainY() );
    m_FeaSSEllGroup.SetY( start_y );

    m_FeaSSEllGroup.AddDividerBox( "Ellipse Sub-Surface" );

    m_FeaSSEllGroup.SetFitWidthFlag( false );
    m_FeaSSEllGroup.SetSameLineFlag( true );
    m_FeaSSEllGroup.AddLabel( "Tag", m_FeaSSEllGroup.GetRemainX() / 3 );
    m_FeaSSEllGroup.SetButtonWidth( m_FeaSSEllGroup.GetRemainX() / 3 );
    m_FeaSSEllGroup.AddButton( m_FeaSSEllInsideButton, "Inside" );
    m_FeaSSEllGroup.AddButton( m_FeaSSEllOutsideButton, "Outside" );
    m_FeaSSEllGroup.AddButton( m_FeaSSEllNoneButton, "None" );

    m_FeaSSEllTestToggleGroup.Init( this );
    m_FeaSSEllTestToggleGroup.AddButton( m_FeaSSEllInsideButton.GetFlButton() );
    m_FeaSSEllTestToggleGroup.AddButton( m_FeaSSEllOutsideButton.GetFlButton() );
    m_FeaSSEllTestToggleGroup.AddButton( m_FeaSSEllNoneButton.GetFlButton() );

    m_FeaSSEllGroup.ForceNewLine();
    m_FeaSSEllGroup.SetFitWidthFlag( true );
    m_FeaSSEllGroup.SetSameLineFlag( false );

    m_FeaSSEllGroup.SetButtonWidth( m_FeaSSEllGroup.GetRemainX() / 3 );
    m_FeaSSEllGroup.SetChoiceButtonWidth( m_FeaSSEllGroup.GetRemainX() / 3 );

    m_FeaSSEllGroup.AddSlider( m_FeaSSEllTessSlider, "Num Points", 100, "%5.0f" );
    m_FeaSSEllGroup.AddSlider( m_FeaSSEllCentUSlider, "Center U", 1, "%5.4f" );
    m_FeaSSEllGroup.AddSlider( m_FeaSSEllCentWSlider, "Center W", 1, "%5.4f" );
    m_FeaSSEllGroup.AddSlider( m_FeaSSEllULenSlider, "U Length", 1, "%5.4f" );
    m_FeaSSEllGroup.AddSlider( m_FeaSSEllWLenSlider, "W Length", 1, "%5.4f" );
    m_FeaSSEllGroup.AddSlider( m_FeaSSEllThetaSlider, "Theta", 25, "%5.4f" );

    //===== SSXsecCurve =====//
    m_GenLayout.AddSubGroupLayout( m_FeaSSXSCGroup, m_GenLayout.GetW(), m_GenLayout.GetRemainY() );

    int remain_x = m_FeaSSEllGroup.GetRemainX();

    m_FeaSSXSCGroup.SetFitWidthFlag( false );
    m_FeaSSXSCGroup.SetSameLineFlag( true );
    m_FeaSSXSCGroup.AddLabel( "Tag", remain_x / 3 );
    m_FeaSSXSCGroup.SetButtonWidth( remain_x / 3 );
    m_FeaSSXSCGroup.AddButton( m_FeaSSXSCInsideButton, "Inside" );
    m_FeaSSXSCGroup.AddButton( m_FeaSSXSCOutsideButton, "Outside" );

    m_FeaSSXSCTestToggleGroup.Init( this );
    m_FeaSSXSCTestToggleGroup.AddButton( m_FeaSSXSCInsideButton.GetFlButton() );
    m_FeaSSXSCTestToggleGroup.AddButton( m_FeaSSXSCOutsideButton.GetFlButton() );

    m_FeaSSXSCGroup.SetFitWidthFlag( true );
    m_FeaSSXSCGroup.SetSameLineFlag( false );
    m_FeaSSXSCGroup.ForceNewLine();


    m_FeaSSXSCGroup.AddSlider( m_FeaSSXSCCentUSlider, "Center U", 1, "%7.6f" );
    m_FeaSSXSCGroup.AddSlider( m_FeaSSXSCCentWSlider, "Center W", 1, "%7.6f" );


    m_FeaSSXSCGroup.AddYGap();

    m_FeaSSXSCGroup.AddDividerBox( "Type" );

    m_FeaSSXSecTypeChoice.AddItem( "POINT", vsp::XS_POINT );
    m_FeaSSXSecTypeChoice.AddItem( "CIRCLE", vsp::XS_CIRCLE );
    m_FeaSSXSecTypeChoice.AddItem( "ELLIPSE", vsp::XS_ELLIPSE );
    m_FeaSSXSecTypeChoice.AddItem( "SUPER_ELLIPSE", vsp::XS_SUPER_ELLIPSE );
    m_FeaSSXSecTypeChoice.AddItem( "ROUNDED_RECTANGLE", vsp::XS_ROUNDED_RECTANGLE );
    m_FeaSSXSecTypeChoice.AddItem( "GENERAL_FUSE", vsp::XS_GENERAL_FUSE );
    m_FeaSSXSecTypeChoice.AddItem( "FUSE_FILE", vsp::XS_FILE_FUSE );
    m_FeaSSXSecTypeChoice.AddItem( "FOUR_SERIES", vsp::XS_FOUR_SERIES );
    m_FeaSSXSecTypeChoice.AddItem( "SIX_SERIES", vsp::XS_SIX_SERIES );
    m_FeaSSXSecTypeChoice.AddItem( "BICONVEX", vsp::XS_BICONVEX );
    m_FeaSSXSecTypeChoice.AddItem( "WEDGE", vsp::XS_WEDGE );
    m_FeaSSXSecTypeChoice.AddItem( "EDIT_CURVE", vsp::XS_EDIT_CURVE );
    m_FeaSSXSecTypeChoice.AddItem( "AF_FILE", vsp::XS_FILE_AIRFOIL );
    m_FeaSSXSecTypeChoice.AddItem( "CST_AIRFOIL", vsp::XS_CST_AIRFOIL );
    m_FeaSSXSecTypeChoice.AddItem( "KARMAN_TREFFTZ", vsp::XS_VKT_AIRFOIL );
    m_FeaSSXSecTypeChoice.AddItem( "FOUR_DIGIT_MOD", vsp::XS_FOUR_DIGIT_MOD );
    m_FeaSSXSecTypeChoice.AddItem( "FIVE_DIGIT", vsp::XS_FIVE_DIGIT );
    m_FeaSSXSecTypeChoice.AddItem( "FIVE_DIGIT_MOD", vsp::XS_FIVE_DIGIT_MOD );
    m_FeaSSXSecTypeChoice.AddItem( "16_SERIES", vsp::XS_ONE_SIX_SERIES );
    m_FeaSSXSecTypeChoice.AddItem( "AC25_773", vsp::XS_AC25_773 );

    m_FeaSSXSCGroup.SetFitWidthFlag( true );
    m_FeaSSXSCGroup.SetSameLineFlag( false );

    m_FeaSSXSCGroup.AddChoice( m_FeaSSXSecTypeChoice, "Choose Type:" );

    m_FeaSSXSCGroup.SetFitWidthFlag( false );
    m_FeaSSXSCGroup.SetSameLineFlag( true );

    m_FeaSSXSCGroup.SetButtonWidth( m_FeaSSXSCGroup.GetW() / 2 );
    m_FeaSSXSCGroup.AddButton( m_FeaSSXSCShowXSecButton, "Show" );

    m_FeaSSXSCGroup.AddSubGroupLayout( m_FeaSSXSCConvertCEDITGroup, m_FeaSSXSCGroup.GetW(), m_FeaSSXSCGroup.GetStdHeight() );
    m_FeaSSXSCConvertCEDITGroup.SetButtonWidth( m_FeaSSXSCGroup.GetW() / 2 );
    m_FeaSSXSCConvertCEDITGroup.SetFitWidthFlag( false );
    m_FeaSSXSCConvertCEDITGroup.AddButton( m_FeaSSXSCConvertCEDITButton, "Convert CEDIT" );

    m_FeaSSXSCGroup.AddSubGroupLayout( m_FeaSSXSCEditCEDITGroup, m_FeaSSXSCGroup.GetW(), m_FeaSSXSCGroup.GetStdHeight() );
    m_FeaSSXSCEditCEDITGroup.SetFitWidthFlag( false );
    m_FeaSSXSCEditCEDITGroup.SetButtonWidth( m_FeaSSXSCGroup.GetW() / 2 );
    m_FeaSSXSCEditCEDITGroup.AddButton( m_FeaSSXSCEditCEDITButton, "Edit Curve" );

    m_FeaSSXSCGroup.ForceNewLine();

    m_FeaSSXSCGroup.SetFitWidthFlag( true );
    m_FeaSSXSCGroup.SetSameLineFlag( false );

    //==== Circle XSec ====//
    m_FeaSSXSCGroup.AddSubGroupLayout( m_FeaSSXSCCircleGroup, m_FeaSSXSCGroup.GetW(), m_FeaSSXSCGroup.GetRemainY() );
    m_FeaSSXSCCircleGroup.AddSlider(  m_FeaSSXSCDiameterSlider, "Diameter", 10, "%6.5f" );

    //==== Ellipse XSec ====//
    m_FeaSSXSCGroup.AddSubGroupLayout( m_FeaSSXSCEllipseGroup, m_FeaSSXSCGroup.GetW(), m_FeaSSXSCGroup.GetRemainY() );
    m_FeaSSXSCEllipseGroup.AddSlider(  m_FeaSSXSCEllipseHeightSlider, "Height", 10, "%6.5f" );
    m_FeaSSXSCEllipseGroup.AddSlider(  m_FeaSSXSCEllipseWidthSlider, "Width", 10, "%6.5f" );

    //==== AC 25.773-1 XSec ====//
    m_FeaSSXSCGroup.AddSubGroupLayout( m_FeaSSXSCAC25773Group, m_FeaSSXSCGroup.GetW(), m_FeaSSXSCGroup.GetRemainY() );
    m_FeaSSXSCAC25773Group.AddChoice( m_FeaSSXSCAC25773SeatChoice, "Pilot Seat" );

    m_FeaSSXSCAC25773SeatChoice.AddItem( "Left", vsp::XSEC_LEFT_SIDE );
    m_FeaSSXSCAC25773SeatChoice.AddItem( "Right", vsp::XSEC_RIGHT_SIDE );
    m_FeaSSXSCAC25773SeatChoice.UpdateItems();

    //==== Super XSec ====//
    m_FeaSSXSCGroup.AddSubGroupLayout( m_FeaSSXSCSuperGroup, m_FeaSSXSCGroup.GetW(), m_FeaSSXSCGroup.GetRemainY() );
    m_FeaSSXSCSuperGroup.AddSlider( m_FeaSSXSCSuperHeightSlider, "Height", 10, "%6.5f" );
    m_FeaSSXSCSuperGroup.AddSlider( m_FeaSSXSCSuperWidthSlider,  "Width", 10, "%6.5f" );
    m_FeaSSXSCSuperGroup.AddYGap();
    m_FeaSSXSCSuperGroup.AddSlider( m_FeaSSXSCSuperMaxWidthLocSlider, "MaxWLoc", 2, "%6.5f" );
    m_FeaSSXSCSuperGroup.AddYGap();
    m_FeaSSXSCSuperGroup.AddSlider( m_FeaSSXSCSuperMSlider, "M", 10, "%6.5f" );
    m_FeaSSXSCSuperGroup.AddSlider( m_FeaSSXSCSuperNSlider, "N", 10, "%6.5f" );
    m_FeaSSXSCSuperGroup.AddYGap();
    m_FeaSSXSCSuperGroup.AddButton( m_FeaSSXSCSuperToggleSym, "T/B Symmetric Exponents" );
    m_FeaSSXSCSuperGroup.AddSlider( m_FeaSSXSCSuperM_botSlider, "M Bottom", 10, "%6.5f" );
    m_FeaSSXSCSuperGroup.AddSlider( m_FeaSSXSCSuperN_botSlider, "N Bottom", 10, "%6.5f" );

    //==== Rounded Rect ====//
    m_FeaSSXSCGroup.AddSubGroupLayout( m_FeaSSXSCRoundedRectGroup, m_FeaSSXSCGroup.GetW(), m_FeaSSXSCGroup.GetRemainY() );
    m_FeaSSXSCRoundedRectGroup.AddSlider( m_FeaSSXSCRRHeightSlider, "Height", 10, "%6.5f" );
    m_FeaSSXSCRoundedRectGroup.AddSlider( m_FeaSSXSCRRWidthSlider,  "Width", 10, "%6.5f" );
    m_FeaSSXSCRoundedRectGroup.AddYGap();
    m_FeaSSXSCRoundedRectGroup.AddSlider( m_FeaSSXSCRRSkewSlider, "Skew", 2, "%6.5f");
    m_FeaSSXSCRoundedRectGroup.AddSlider( m_FeaSSXSCRRVSkewSlider, "VSkew", 2, "%6.5f" );
    m_FeaSSXSCRoundedRectGroup.AddSlider( m_FeaSSXSCRRKeystoneSlider, "Keystone", 1, "%6.5f");

    m_FeaSSXSCRoundedRectGroup.AddYGap();
    m_FeaSSXSCRoundedRectGroup.SetSameLineFlag( true );
    m_FeaSSXSCRoundedRectGroup.SetFitWidthFlag( false );

    int oldbw = m_FeaSSXSCRoundedRectGroup.GetButtonWidth();

    m_FeaSSXSCRoundedRectGroup.AddLabel( "Symmetry:", oldbw );

    m_FeaSSXSCRoundedRectGroup.SetButtonWidth( m_FeaSSXSCRoundedRectGroup.GetRemainX() / 4 );
    m_FeaSSXSCRoundedRectGroup.AddButton( m_FeaSSXSCRRRadNoSymToggle, "None" );
    m_FeaSSXSCRoundedRectGroup.AddButton( m_FeaSSXSCRRRadRLSymToggle, "R//L" );
    m_FeaSSXSCRoundedRectGroup.AddButton( m_FeaSSXSCRRRadTBSymToggle, "T//B" );
    m_FeaSSXSCRoundedRectGroup.AddButton( m_FeaSSXSCRRRadAllSymToggle, "All" );

    m_FeaSSXSCRRRadSymRadioGroup.Init( this );
    m_FeaSSXSCRRRadSymRadioGroup.AddButton( m_FeaSSXSCRRRadNoSymToggle.GetFlButton() );
    m_FeaSSXSCRRRadSymRadioGroup.AddButton( m_FeaSSXSCRRRadRLSymToggle.GetFlButton() );
    m_FeaSSXSCRRRadSymRadioGroup.AddButton( m_FeaSSXSCRRRadTBSymToggle.GetFlButton() );
    m_FeaSSXSCRRRadSymRadioGroup.AddButton( m_FeaSSXSCRRRadAllSymToggle.GetFlButton() );

    m_FeaSSXSCRoundedRectGroup.ForceNewLine();
    m_FeaSSXSCRoundedRectGroup.SetSameLineFlag( false );
    m_FeaSSXSCRoundedRectGroup.SetFitWidthFlag( true );

    m_FeaSSXSCRoundedRectGroup.SetButtonWidth( oldbw );

    m_FeaSSXSCRoundedRectGroup.AddSlider( m_FeaSSXSCRRRadiusTRSlider, "TR Radius", 10, "%6.5f" );
    m_FeaSSXSCRoundedRectGroup.AddSlider( m_FeaSSXSCRRRadiusTLSlider, "TL Radius", 10, "%6.5f" );
    m_FeaSSXSCRoundedRectGroup.AddSlider( m_FeaSSXSCRRRadiusBLSlider, "BL Radius", 10, "%6.5f" );
    m_FeaSSXSCRoundedRectGroup.AddSlider( m_FeaSSXSCRRRadiusBRSlider, "BR Radius", 10, "%6.5f" );
    m_FeaSSXSCRoundedRectGroup.AddYGap();

    m_FeaSSXSCRoundedRectGroup.AddButton( m_FeaSSXSCRRKeyCornerButton, "Key Corner" );

    //==== General Fuse XSec ====//
    m_FeaSSXSCGroup.AddSubGroupLayout( m_FeaSSXSCGenGroup, m_FeaSSXSCGroup.GetW(), m_FeaSSXSCGroup.GetRemainY() );
    m_FeaSSXSCGenGroup.AddSlider( m_FeaSSXSCGenHeightSlider, "Height", 10, "%6.5f" );
    m_FeaSSXSCGenGroup.AddSlider( m_FeaSSXSCGenWidthSlider, "Width", 10, "%6.5f" );
    m_FeaSSXSCGenGroup.AddYGap();
    m_FeaSSXSCGenGroup.AddSlider( m_FeaSSXSCGenMaxWidthLocSlider, "MaxWLoc", 1, "%6.5f" );
    m_FeaSSXSCGenGroup.AddSlider( m_FeaSSXSCGenCornerRadSlider, "CornerRad", 1, "%6.5f" );
    m_FeaSSXSCGenGroup.AddYGap();
    m_FeaSSXSCGenGroup.AddSlider( m_FeaSSXSCGenTopTanAngleSlider, "TopTanAng", 90, "%7.5f" );
    m_FeaSSXSCGenGroup.AddSlider( m_FeaSSXSCGenBotTanAngleSlider, "BotTanAng", 90, "%7.5f" );
    m_FeaSSXSCGenGroup.AddYGap();
    m_FeaSSXSCGenGroup.AddSlider( m_FeaSSXSCGenTopStrSlider, "TopStr", 1, "%7.5f" );
    m_FeaSSXSCGenGroup.AddSlider( m_FeaSSXSCGenBotStrSlider, "BotStr", 1, "%7.5f" );
    m_FeaSSXSCGenGroup.AddSlider( m_FeaSSXSCGenUpStrSlider, "UpStr", 1, "%7.5f" );
    m_FeaSSXSCGenGroup.AddSlider( m_FeaSSXSCGenLowStrSlider, "LowStr", 1, "%7.5f" );

    //==== Four Series AF ====//
    m_FeaSSXSCGroup.AddSubGroupLayout( m_FeaSSXSCFourSeriesGroup, m_FeaSSXSCGroup.GetW(), m_FeaSSXSCGroup.GetRemainY() );
    m_FeaSSXSCFourSeriesGroup.AddOutput( m_FeaSSXSCFourNameOutput, "Name" );
    m_FeaSSXSCFourSeriesGroup.AddYGap();
    m_FeaSSXSCFourSeriesGroup.AddSlider( m_FeaSSXSCFourChordSlider, "Chord", 10, "%7.3f" );
    m_FeaSSXSCFourSeriesGroup.AddSlider( m_FeaSSXSCFourThickChordSlider, "T/C", 1, "%7.5f" );
    m_FeaSSXSCFourSeriesGroup.AddYGap();

    int actionToggleButtonWidth = 15;
    int actionSliderButtonWidth = m_FeaSSXSCFourSeriesGroup.GetButtonWidth() - actionToggleButtonWidth;

    m_FeaSSXSCFourSeriesGroup.SetSameLineFlag( true );

    m_FeaSSXSCFourSeriesGroup.SetFitWidthFlag( false );
    m_FeaSSXSCFourSeriesGroup.SetButtonWidth( actionToggleButtonWidth );
    m_FeaSSXSCFourSeriesGroup.AddButton( m_FeaSSXSCFourCamberButton, "" );
    m_FeaSSXSCFourSeriesGroup.SetButtonWidth( actionSliderButtonWidth );
    m_FeaSSXSCFourSeriesGroup.SetFitWidthFlag( true );
    m_FeaSSXSCFourSeriesGroup.AddSlider( m_FeaSSXSCFourCamberSlider, "Camber", 0.2, "%7.5f" );

    m_FeaSSXSCFourSeriesGroup.ForceNewLine();

    m_FeaSSXSCFourSeriesGroup.SetFitWidthFlag( false );
    m_FeaSSXSCFourSeriesGroup.SetButtonWidth( actionToggleButtonWidth );
    m_FeaSSXSCFourSeriesGroup.AddButton( m_FeaSSXSCFourCLiButton, "" );
    m_FeaSSXSCFourSeriesGroup.SetButtonWidth( actionSliderButtonWidth );
    m_FeaSSXSCFourSeriesGroup.SetFitWidthFlag( true );
    m_FeaSSXSCFourSeriesGroup.AddSlider( m_FeaSSXSCFourCLiSlider, "Ideal CL", 1, "%7.5f" );

    m_FeaSSXSCFourSeriesGroup.ForceNewLine();

    m_FeaSSXSCFourSeriesGroup.SetSameLineFlag( false );
    m_FeaSSXSCFourSeriesGroup.SetButtonWidth( actionSliderButtonWidth + actionToggleButtonWidth );

    m_FeaSSXSCFourCamberGroup.Init( this );
    m_FeaSSXSCFourCamberGroup.AddButton( m_FeaSSXSCFourCamberButton.GetFlButton() );
    m_FeaSSXSCFourCamberGroup.AddButton( m_FeaSSXSCFourCLiButton.GetFlButton() );

    vector< int > camb_val_map;
    camb_val_map.push_back( vsp::MAX_CAMB );
    camb_val_map.push_back( vsp::DESIGN_CL );
    m_FeaSSXSCFourCamberGroup.SetValMapVec( camb_val_map );

    m_FeaSSXSCFourSeriesGroup.AddSlider( m_FeaSSXSCFourCamberLocSlider, "CamberLoc", 1, "%7.5f" );
    m_FeaSSXSCFourSeriesGroup.AddYGap();
    m_FeaSSXSCFourSeriesGroup.AddButton( m_FeaSSXSCFourInvertButton, "Invert Airfoil" );
    m_FeaSSXSCFourSeriesGroup.AddYGap();
    m_FeaSSXSCFourSeriesGroup.AddButton( m_FeaSSXSCFourSharpTEButton, "Sharpen TE" );

    m_FeaSSXSCFourSeriesGroup.AddYGap();
    m_FeaSSXSCFourSeriesGroup.SetSameLineFlag( true );
    m_FeaSSXSCFourSeriesGroup.SetFitWidthFlag( false );
    m_FeaSSXSCFourSeriesGroup.SetButtonWidth( 125 );
    m_FeaSSXSCFourSeriesGroup.AddButton( m_FeaSSXSCFourFitCSTButton, "Fit CST" );
    m_FeaSSXSCFourSeriesGroup.InitWidthHeightVals();
    m_FeaSSXSCFourSeriesGroup.SetFitWidthFlag( true );
    m_FeaSSXSCFourSeriesGroup.AddCounter( m_FeaSSXSCFourDegreeCounter, "Degree", 125 );

    //==== Six Series AF ====//
    m_FeaSSXSCGroup.AddSubGroupLayout( m_FeaSSXSCSixSeriesGroup, m_FeaSSXSCGroup.GetW(), m_FeaSSXSCGroup.GetRemainY() );
    m_FeaSSXSCSixSeriesGroup.AddOutput( m_FeaSSXSCSixNameOutput, "Name" );
    m_FeaSSXSCSixSeriesGroup.AddYGap();

    m_FeaSSXSCSixSeriesChoice.AddItem( "63-" );
    m_FeaSSXSCSixSeriesChoice.AddItem( "64-" );
    m_FeaSSXSCSixSeriesChoice.AddItem( "65-" );
    m_FeaSSXSCSixSeriesChoice.AddItem( "66-" );
    m_FeaSSXSCSixSeriesChoice.AddItem( "67-" );
    m_FeaSSXSCSixSeriesChoice.AddItem( "63A" );
    m_FeaSSXSCSixSeriesChoice.AddItem( "64A" );
    m_FeaSSXSCSixSeriesChoice.AddItem( "65A" );
    m_FeaSSXSCSixSeriesGroup.AddChoice( m_FeaSSXSCSixSeriesChoice, "Series" );

    m_FeaSSXSCSixSeriesGroup.AddYGap();

    m_FeaSSXSCSixSeriesGroup.AddSlider( m_FeaSSXSCSixChordSlider, "Chord", 10, "%7.3f" );
    m_FeaSSXSCSixSeriesGroup.AddSlider( m_FeaSSXSCSixThickChordSlider, "T/C", 1, "%7.5f" );
    m_FeaSSXSCSixSeriesGroup.AddYGap();
    m_FeaSSXSCSixSeriesGroup.AddSlider( m_FeaSSXSCSixIdealClSlider, "Ideal CL", 1, "%7.5f" );
    m_FeaSSXSCSixSeriesGroup.AddSlider( m_FeaSSXSCSixASlider, "a", 1, "%7.5f" );
    m_FeaSSXSCSixSeriesGroup.AddYGap();
    m_FeaSSXSCSixSeriesGroup.AddButton( m_FeaSSXSCSixInvertButton, "Invert Airfoil" );
    m_FeaSSXSCSixSeriesGroup.AddYGap();
    m_FeaSSXSCSixSeriesGroup.SetSameLineFlag( true );
    m_FeaSSXSCSixSeriesGroup.SetFitWidthFlag( false );
    m_FeaSSXSCSixSeriesGroup.SetButtonWidth( 125 );
    m_FeaSSXSCSixSeriesGroup.AddButton( m_FeaSSXSCSixFitCSTButton, "Fit CST" );
    m_FeaSSXSCSixSeriesGroup.InitWidthHeightVals();
    m_FeaSSXSCSixSeriesGroup.SetFitWidthFlag( true );
    m_FeaSSXSCSixSeriesGroup.AddCounter( m_FeaSSXSCSixDegreeCounter, "Degree", 125 );

    //==== Biconvex AF ====//
    m_FeaSSXSCGroup.AddSubGroupLayout( m_FeaSSXSCBiconvexGroup, m_FeaSSXSCGroup.GetW(), m_FeaSSXSCGroup.GetRemainY() );
    m_FeaSSXSCBiconvexGroup.AddSlider( m_FeaSSXSCBiconvexChordSlider, "Chord", 10, "%7.3f" );
    m_FeaSSXSCBiconvexGroup.AddSlider( m_FeaSSXSCBiconvexThickChordSlider, "T/C", 1, "%7.5f" );

    //==== Wedge AF ====//
    m_FeaSSXSCGroup.AddSubGroupLayout( m_FeaSSXSCWedgeGroup, m_FeaSSXSCGroup.GetW(), m_FeaSSXSCGroup.GetRemainY() );
    m_FeaSSXSCWedgeGroup.AddSlider( m_FeaSSXSCWedgeChordSlider, "Chord", 10, "%7.3f" );
    m_FeaSSXSCWedgeGroup.AddSlider( m_FeaSSXSCWedgeThickChordSlider, "T/C", 1, "%7.5f" );
    m_FeaSSXSCWedgeGroup.AddYGap();
    m_FeaSSXSCWedgeGroup.AddSlider( m_FeaSSXSCWedgeThickLocSlider, "Thick X", 1, "%7.5f" );
    m_FeaSSXSCWedgeGroup.AddSlider( m_FeaSSXSCWedgeFlatUpSlider, "Flat Up", 1, "%7.5f" );
    m_FeaSSXSCWedgeGroup.AddYGap();
    m_FeaSSXSCWedgeGroup.AddButton( m_FeaSSXSCWedgeSymmThickButton, "Symm Thickness" );
    m_FeaSSXSCWedgeGroup.AddSlider( m_FeaSSXSCWedgeThickLocLowSlider, "Thick X Low", 1, "%7.5f" );
    m_FeaSSXSCWedgeGroup.AddSlider( m_FeaSSXSCWedgeFlatLowSlider, "Flat Low", 1, "%7.5f" );
    m_FeaSSXSCWedgeGroup.AddYGap();
    m_FeaSSXSCWedgeGroup.AddSlider( m_FeaSSXSCWedgeZCamberSlider, "Camber", 1, "%7.5f" );
    m_FeaSSXSCWedgeGroup.AddYGap();
    m_FeaSSXSCWedgeGroup.SetSameLineFlag( true );
    m_FeaSSXSCWedgeGroup.AddSlider( m_FeaSSXSCWedgeUForeUpSlider, "U Fwd Up", 1, "%7.5f", m_FeaSSXSCWedgeGroup.GetW() * 0.5 );
    m_FeaSSXSCWedgeGroup.AddSlider( m_FeaSSXSCWedgeDuUpSlider, "dU Flat Up", 1, "%7.5f" );
    m_FeaSSXSCWedgeGroup.ForceNewLine();
    m_FeaSSXSCWedgeGroup.AddSlider( m_FeaSSXSCWedgeUForeLowSlider, "U Fwd Low", 1, "%7.5f", m_FeaSSXSCWedgeGroup.GetW() * 0.5 );
    m_FeaSSXSCWedgeGroup.AddSlider( m_FeaSSXSCWedgeDuLowSlider, "dU Flat Low", 1, "%7.5f" );
    m_FeaSSXSCWedgeGroup.ForceNewLine();
    m_FeaSSXSCWedgeGroup.SetSameLineFlag( false );
    m_FeaSSXSCWedgeGroup.AddYGap();
    m_FeaSSXSCWedgeGroup.AddButton( m_FeaSSXSCWedgeInvertButton, "Invert Airfoil" );

    //==== Fuse File ====//
    m_FeaSSXSCGroup.AddSubGroupLayout( m_FeaSSXSCFuseFileGroup, m_FeaSSXSCGroup.GetW(), m_FeaSSXSCGroup.GetRemainY() );
    m_FeaSSXSCFuseFileGroup.AddButton( m_FeaSSXSCReadFuseFileButton, "Read File" );
    m_FeaSSXSCFuseFileGroup.AddYGap();
    m_FeaSSXSCFuseFileGroup.AddSlider( m_FeaSSXSCFileHeightSlider, "Height", 10, "%7.3f" );
    m_FeaSSXSCFuseFileGroup.AddSlider( m_FeaSSXSCFileWidthSlider, "Width", 10, "%7.3f" );

    //==== Airfoil File ====//
    m_FeaSSXSCGroup.AddSubGroupLayout( m_FeaSSXSCAfFileGroup, m_FeaSSXSCGroup.GetW(), m_FeaSSXSCGroup.GetRemainY() );
    m_FeaSSXSCAfFileGroup.AddButton( m_FeaSSXSCAfReadFileButton, "Read File" );
    m_FeaSSXSCAfFileGroup.AddYGap();
    m_FeaSSXSCAfFileGroup.AddOutput( m_FeaSSXSCAfFileNameOutput, "Name" );
    m_FeaSSXSCAfFileGroup.AddYGap();
    m_FeaSSXSCAfFileGroup.AddSlider( m_FeaSSXSCAfFileChordSlider, "Chord", 10, "%7.3f" );
    m_FeaSSXSCAfFileGroup.AddSlider( m_FeaSSXSCAfFileThickChordSlider, "T/C", 1, "%7.5f" );
    m_FeaSSXSCAfFileGroup.AddOutput( m_FeaSSXSCAfFileBaseThickChordOutput, "Base T/C", "%7.5f" );
    m_FeaSSXSCAfFileGroup.AddYGap();
    m_FeaSSXSCAfFileGroup.AddButton( m_FeaSSXSCAfFileInvertButton, "Invert Airfoil" );
    m_FeaSSXSCAfFileGroup.AddYGap();
    m_FeaSSXSCAfFileGroup.SetSameLineFlag( true );
    m_FeaSSXSCAfFileGroup.SetFitWidthFlag( false );
    m_FeaSSXSCAfFileGroup.SetButtonWidth( 125 );
    m_FeaSSXSCAfFileGroup.AddButton( m_FeaSSXSCAfFileFitCSTButton, "Fit CST" );
    m_FeaSSXSCAfFileGroup.InitWidthHeightVals();
    m_FeaSSXSCAfFileGroup.SetFitWidthFlag( true );
    m_FeaSSXSCAfFileGroup.AddCounter( m_FeaSSXSCAfFileDegreeCounter, "Degree", 125 );

    //==== CST Airfoil ====//
    m_FeaSSXSCGroup.AddSubGroupLayout( m_FeaSSXSCCSTAirfoilGroup, m_FeaSSXSCGroup.GetW(), m_FeaSSXSCGroup.GetRemainY() );

    m_FeaSSXSCCSTAirfoilGroup.AddOutput( m_FeaSSXSCCSTThickChordOutput, "T/C", "%7.5f" );

    m_FeaSSXSCCSTAirfoilGroup.SetSameLineFlag( true );
    m_FeaSSXSCCSTAirfoilGroup.AddButton( m_FeaSSXSCCSTContLERadButton, "Enforce Continuous LE Radius", m_FeaSSXSCCSTAirfoilGroup.GetW() * 0.5 );
    m_FeaSSXSCCSTAirfoilGroup.AddButton( m_FeaSSXSCCSTInvertButton, "Invert Airfoil", m_FeaSSXSCCSTAirfoilGroup.GetW() * 0.5 );
    m_FeaSSXSCCSTAirfoilGroup.ForceNewLine();
    m_FeaSSXSCCSTAirfoilGroup.SetSameLineFlag( false );

    m_FeaSSXSCCSTAirfoilGroup.AddSlider( m_FeaSSXSCCSTChordSlider, "Chord", 10, "%7.3f");

    m_FeaSSXSCCSTAirfoilGroup.AddYGap();
    m_FeaSSXSCCSTAirfoilGroup.AddDividerBox( "Upper Surface" );

    m_FeaSSXSCCSTAirfoilGroup.SetSameLineFlag( true );
    m_FeaSSXSCCSTAirfoilGroup.SetFitWidthFlag( true );

    m_FeaSSXSCCSTAirfoilGroup.AddOutput( m_FeaSSXSCUpDegreeOutput, "Degree", m_FeaSSXSCCSTAirfoilGroup.GetButtonWidth() * 2 );
    m_FeaSSXSCCSTAirfoilGroup.SetFitWidthFlag( false );
    m_FeaSSXSCCSTAirfoilGroup.AddButton( m_FeaSSXSCUpDemoteButton, "Demote" );
    m_FeaSSXSCCSTAirfoilGroup.AddButton( m_FeaSSXSCUpPromoteButton, "Promote" );

    m_FeaSSXSCCSTAirfoilGroup.ForceNewLine();

    m_FeaSSXSCCSTAirfoilGroup.SetSameLineFlag( false );
    m_FeaSSXSCCSTAirfoilGroup.SetFitWidthFlag( true );

    m_FeaSSXSCCSTUpCoeffScroll = m_FeaSSXSCCSTAirfoilGroup.AddFlScroll( 60 );

    m_FeaSSXSCCSTUpCoeffScroll->type( Fl_Scroll::VERTICAL_ALWAYS );
    m_FeaSSXSCCSTUpCoeffScroll->box( FL_BORDER_BOX );
    m_FeaSSXSCCSTUpCoeffLayout.SetGroupAndScreen( m_FeaSSXSCCSTUpCoeffScroll, this );

    m_FeaSSXSCCSTAirfoilGroup.AddYGap();

    m_FeaSSXSCCSTAirfoilGroup.AddDividerBox( "Lower Surface" );

    m_FeaSSXSCCSTAirfoilGroup.SetSameLineFlag( true );
    m_FeaSSXSCCSTAirfoilGroup.SetFitWidthFlag( true );

    m_FeaSSXSCCSTAirfoilGroup.AddOutput( m_FeaSSXSCLowDegreeOutput, "Degree", m_FeaSSXSCCSTAirfoilGroup.GetButtonWidth() * 2 );
    m_FeaSSXSCCSTAirfoilGroup.SetFitWidthFlag( false );
    m_FeaSSXSCCSTAirfoilGroup.AddButton( m_FeaSSXSCLowDemoteButton, "Demote" );
    m_FeaSSXSCCSTAirfoilGroup.AddButton( m_FeaSSXSCLowPromoteButton, "Promote" );

    m_FeaSSXSCCSTAirfoilGroup.ForceNewLine();

    m_FeaSSXSCCSTAirfoilGroup.SetSameLineFlag( false );
    m_FeaSSXSCCSTAirfoilGroup.SetFitWidthFlag( true );

    m_FeaSSXSCCSTLowCoeffScroll = m_FeaSSXSCCSTAirfoilGroup.AddFlScroll( 60 );
    m_FeaSSXSCCSTLowCoeffScroll->type( Fl_Scroll::VERTICAL_ALWAYS );
    m_FeaSSXSCCSTLowCoeffScroll->box( FL_BORDER_BOX );
    m_FeaSSXSCCSTLowCoeffLayout.SetGroupAndScreen( m_FeaSSXSCCSTLowCoeffScroll, this );

    //==== VKT AF ====//
    m_FeaSSXSCGroup.AddSubGroupLayout( m_FeaSSXSCVKTGroup, m_FeaSSXSCGroup.GetW(), m_FeaSSXSCGroup.GetRemainY() );
    m_FeaSSXSCVKTGroup.AddSlider( m_FeaSSXSCVKTChordSlider, "Chord", 10, "%7.3f" );
    m_FeaSSXSCVKTGroup.AddYGap();
    m_FeaSSXSCVKTGroup.AddSlider( m_FeaSSXSCVKTEpsilonSlider, "Epsilon", 1, "%7.5f" );
    m_FeaSSXSCVKTGroup.AddSlider( m_FeaSSXSCVKTKappaSlider, "Kappa", 1, "%7.5f" );
    m_FeaSSXSCVKTGroup.AddSlider( m_FeaSSXSCVKTTauSlider, "Tau", 10, "%7.5f" );
    m_FeaSSXSCVKTGroup.AddOutput( m_FeaSSXSCVKTThickChordOutput, "T/C", "%7.5f" );
    m_FeaSSXSCVKTGroup.AddYGap();
    m_FeaSSXSCVKTGroup.AddButton( m_FeaSSXSCVKTInvertButton, "Invert Airfoil" );
    m_FeaSSXSCVKTGroup.AddYGap();
    m_FeaSSXSCVKTGroup.SetSameLineFlag( true );
    m_FeaSSXSCVKTGroup.SetFitWidthFlag( false );
    m_FeaSSXSCVKTGroup.SetButtonWidth( 125 );
    m_FeaSSXSCVKTGroup.AddButton( m_FeaSSXSCVKTFitCSTButton, "Fit CST" );
    m_FeaSSXSCVKTGroup.InitWidthHeightVals();
    m_FeaSSXSCVKTGroup.SetFitWidthFlag( true );
    m_FeaSSXSCVKTGroup.AddCounter( m_FeaSSXSCVKTDegreeCounter, "Degree", 125 );

    //==== Four Series AF ====//
    m_FeaSSXSCGroup.AddSubGroupLayout( m_FeaSSXSCFourDigitModGroup, m_FeaSSXSCGroup.GetW(), m_FeaSSXSCGroup.GetRemainY() );
    m_FeaSSXSCFourDigitModGroup.AddOutput( m_FeaSSXSCFourModNameOutput, "Name" );
    m_FeaSSXSCFourDigitModGroup.AddYGap();
    m_FeaSSXSCFourDigitModGroup.AddSlider( m_FeaSSXSCFourModChordSlider, "Chord", 10, "%7.3f" );
    m_FeaSSXSCFourDigitModGroup.AddSlider( m_FeaSSXSCFourModThickChordSlider, "T/C", 1, "%7.5f" );
    m_FeaSSXSCFourDigitModGroup.AddYGap();

    m_FeaSSXSCFourDigitModGroup.SetSameLineFlag( true );

    m_FeaSSXSCFourDigitModGroup.SetFitWidthFlag( false );
    m_FeaSSXSCFourDigitModGroup.SetButtonWidth( actionToggleButtonWidth );
    m_FeaSSXSCFourDigitModGroup.AddButton( m_FeaSSXSCFourModCamberButton, "" );
    m_FeaSSXSCFourDigitModGroup.SetButtonWidth( actionSliderButtonWidth );
    m_FeaSSXSCFourDigitModGroup.SetFitWidthFlag( true );
    m_FeaSSXSCFourDigitModGroup.AddSlider( m_FeaSSXSCFourModCamberSlider, "Camber", 0.2, "%7.5f" );

    m_FeaSSXSCFourDigitModGroup.ForceNewLine();

    m_FeaSSXSCFourDigitModGroup.SetFitWidthFlag( false );
    m_FeaSSXSCFourDigitModGroup.SetButtonWidth( actionToggleButtonWidth );
    m_FeaSSXSCFourDigitModGroup.AddButton( m_FeaSSXSCFourModCLiButton, "" );
    m_FeaSSXSCFourDigitModGroup.SetButtonWidth( actionSliderButtonWidth );
    m_FeaSSXSCFourDigitModGroup.SetFitWidthFlag( true );
    m_FeaSSXSCFourDigitModGroup.AddSlider( m_FeaSSXSCFourModCLiSlider, "Ideal CL", 1, "%7.5f" );

    m_FeaSSXSCFourDigitModGroup.ForceNewLine();

    m_FeaSSXSCFourDigitModGroup.SetSameLineFlag( false );
    m_FeaSSXSCFourDigitModGroup.SetButtonWidth( actionSliderButtonWidth + actionToggleButtonWidth );

    m_FeaSSXSCFourModCamberGroup.Init( this );
    m_FeaSSXSCFourModCamberGroup.AddButton( m_FeaSSXSCFourModCamberButton.GetFlButton() );
    m_FeaSSXSCFourModCamberGroup.AddButton( m_FeaSSXSCFourModCLiButton.GetFlButton() );

    m_FeaSSXSCFourModCamberGroup.SetValMapVec( camb_val_map );

    m_FeaSSXSCFourDigitModGroup.AddSlider( m_FeaSSXSCFourModCamberLocSlider, "CamberLoc", 1, "%7.5f" );
    m_FeaSSXSCFourDigitModGroup.AddYGap();
    m_FeaSSXSCFourDigitModGroup.AddSlider( m_FeaSSXSCFourModThicknessLocSlider, "T/CLoc", 0.5, "%7.5f" );
    m_FeaSSXSCFourDigitModGroup.AddSlider( m_FeaSSXSCFourModLERadIndexSlider, "LERadIndx", 5, "%7.5f" );
    m_FeaSSXSCFourDigitModGroup.AddYGap();
    m_FeaSSXSCFourDigitModGroup.AddButton( m_FeaSSXSCFourModInvertButton, "Invert Airfoil" );
    m_FeaSSXSCFourDigitModGroup.AddYGap();
    m_FeaSSXSCFourDigitModGroup.AddButton( m_FeaSSXSCFourModSharpTEButton, "Sharpen TE" );
    m_FeaSSXSCFourDigitModGroup.AddYGap();
    m_FeaSSXSCFourDigitModGroup.SetSameLineFlag( true );
    m_FeaSSXSCFourDigitModGroup.SetFitWidthFlag( false );
    m_FeaSSXSCFourDigitModGroup.SetButtonWidth( 125 );
    m_FeaSSXSCFourDigitModGroup.AddButton( m_FeaSSXSCFourModFitCSTButton, "Fit CST" );
    m_FeaSSXSCFourDigitModGroup.InitWidthHeightVals();
    m_FeaSSXSCFourDigitModGroup.SetFitWidthFlag( true );
    m_FeaSSXSCFourDigitModGroup.AddCounter( m_FeaSSXSCFourModDegreeCounter, "Degree", 125 );

    //==== Five Digit AF ====//
    m_FeaSSXSCGroup.AddSubGroupLayout( m_FeaSSXSCFiveDigitGroup, m_FeaSSXSCGroup.GetW(), m_FeaSSXSCGroup.GetRemainY() );
    m_FeaSSXSCFiveDigitGroup.AddOutput( m_FeaSSXSCFiveNameOutput, "Name" );
    m_FeaSSXSCFiveDigitGroup.AddYGap();
    m_FeaSSXSCFiveDigitGroup.AddSlider( m_FeaSSXSCFiveChordSlider, "Chord", 10, "%7.3f" );
    m_FeaSSXSCFiveDigitGroup.AddSlider( m_FeaSSXSCFiveThickChordSlider, "T/C", 1, "%7.5f" );
    m_FeaSSXSCFiveDigitGroup.AddYGap();
    m_FeaSSXSCFiveDigitGroup.AddSlider( m_FeaSSXSCFiveCLiSlider, "Ideal CL", 1, "%7.5f" );
    m_FeaSSXSCFiveDigitGroup.AddSlider( m_FeaSSXSCFiveCamberLocSlider, "CamberLoc", 1, "%7.5f" );
    m_FeaSSXSCFiveDigitGroup.AddYGap();
    m_FeaSSXSCFiveDigitGroup.AddButton( m_FeaSSXSCFiveInvertButton, "Invert Airfoil" );
    m_FeaSSXSCFiveDigitGroup.AddYGap();
    m_FeaSSXSCFiveDigitGroup.AddButton( m_FeaSSXSCFiveSharpTEButton, "Sharpen TE" );
    m_FeaSSXSCFiveDigitGroup.AddYGap();
    m_FeaSSXSCFiveDigitGroup.SetSameLineFlag( true );
    m_FeaSSXSCFiveDigitGroup.SetFitWidthFlag( false );
    m_FeaSSXSCFiveDigitGroup.SetButtonWidth( 125 );
    m_FeaSSXSCFiveDigitGroup.AddButton( m_FeaSSXSCFiveFitCSTButton, "Fit CST" );
    m_FeaSSXSCFiveDigitGroup.InitWidthHeightVals();
    m_FeaSSXSCFiveDigitGroup.SetFitWidthFlag( true );
    m_FeaSSXSCFiveDigitGroup.AddCounter( m_FeaSSXSCFiveDegreeCounter, "Degree", 125 );

    //==== Five Digit Mod AF ====//
    m_FeaSSXSCGroup.AddSubGroupLayout( m_FeaSSXSCFiveDigitModGroup, m_FeaSSXSCGroup.GetW(), m_FeaSSXSCGroup.GetRemainY() );
    m_FeaSSXSCFiveDigitModGroup.AddOutput( m_FeaSSXSCFiveModNameOutput, "Name" );
    m_FeaSSXSCFiveDigitModGroup.AddYGap();
    m_FeaSSXSCFiveDigitModGroup.AddSlider( m_FeaSSXSCFiveModChordSlider, "Chord", 10, "%7.3f" );
    m_FeaSSXSCFiveDigitModGroup.AddSlider( m_FeaSSXSCFiveModThickChordSlider, "T/C", 1, "%7.5f" );
    m_FeaSSXSCFiveDigitModGroup.AddYGap();
    m_FeaSSXSCFiveDigitModGroup.AddSlider( m_FeaSSXSCFiveModCLiSlider, "Ideal CL", 1, "%7.5f" );
    m_FeaSSXSCFiveDigitModGroup.AddSlider( m_FeaSSXSCFiveModCamberLocSlider, "CamberLoc", 1, "%7.5f" );
    m_FeaSSXSCFiveDigitModGroup.AddYGap();
    m_FeaSSXSCFiveDigitModGroup.AddSlider( m_FeaSSXSCFiveModThicknessLocSlider, "T/CLoc", 0.5, "%7.5f" );
    m_FeaSSXSCFiveDigitModGroup.AddSlider( m_FeaSSXSCFiveModLERadIndexSlider, "LERadIndx", 5, "%7.5f" );
    m_FeaSSXSCFiveDigitModGroup.AddYGap();
    m_FeaSSXSCFiveDigitModGroup.AddButton( m_FeaSSXSCFiveModInvertButton, "Invert Airfoil" );
    m_FeaSSXSCFiveDigitModGroup.AddYGap();
    m_FeaSSXSCFiveDigitModGroup.AddButton( m_FeaSSXSCFiveModSharpTEButton, "Sharpen TE" );
    m_FeaSSXSCFiveDigitModGroup.AddYGap();
    m_FeaSSXSCFiveDigitModGroup.SetSameLineFlag( true );
    m_FeaSSXSCFiveDigitModGroup.SetFitWidthFlag( false );
    m_FeaSSXSCFiveDigitModGroup.SetButtonWidth( 125 );
    m_FeaSSXSCFiveDigitModGroup.AddButton( m_FeaSSXSCFiveModFitCSTButton, "Fit CST" );
    m_FeaSSXSCFiveDigitModGroup.InitWidthHeightVals();
    m_FeaSSXSCFiveDigitModGroup.SetFitWidthFlag( true );
    m_FeaSSXSCFiveDigitModGroup.AddCounter( m_FeaSSXSCFiveModDegreeCounter, "Degree", 125 );

    //==== 16 Series AF ====//
    m_FeaSSXSCGroup.AddSubGroupLayout( m_FeaSSXSCOneSixSeriesGroup, m_FeaSSXSCGroup.GetW(), m_FeaSSXSCGroup.GetRemainY() );
    m_FeaSSXSCOneSixSeriesGroup.AddOutput( m_FeaSSXSCOneSixSeriesNameOutput, "Name" );
    m_FeaSSXSCOneSixSeriesGroup.AddYGap();
    m_FeaSSXSCOneSixSeriesGroup.AddSlider( m_FeaSSXSCOneSixSeriesChordSlider, "Chord", 10, "%7.3f" );
    m_FeaSSXSCOneSixSeriesGroup.AddSlider( m_FeaSSXSCOneSixSeriesThickChordSlider, "T/C", 1, "%7.5f" );
    m_FeaSSXSCOneSixSeriesGroup.AddYGap();
    m_FeaSSXSCOneSixSeriesGroup.AddSlider( m_FeaSSXSCOneSixSeriesCLiSlider, "Ideal CL", 1, "%7.5f" );
    m_FeaSSXSCOneSixSeriesGroup.AddYGap();
    m_FeaSSXSCOneSixSeriesGroup.AddButton( m_FeaSSXSCOneSixSeriesInvertButton, "Invert Airfoil" );
    m_FeaSSXSCOneSixSeriesGroup.AddYGap();
    m_FeaSSXSCOneSixSeriesGroup.AddButton( m_FeaSSXSCOneSixSeriesSharpTEButton, "Sharpen TE" );
    m_FeaSSXSCOneSixSeriesGroup.AddYGap();
    m_FeaSSXSCOneSixSeriesGroup.SetSameLineFlag( true );
    m_FeaSSXSCOneSixSeriesGroup.SetFitWidthFlag( false );
    m_FeaSSXSCOneSixSeriesGroup.SetButtonWidth( 125 );
    m_FeaSSXSCOneSixSeriesGroup.AddButton( m_FeaSSXSCOneSixSeriesFitCSTButton, "Fit CST" );
    m_FeaSSXSCOneSixSeriesGroup.InitWidthHeightVals();
    m_FeaSSXSCOneSixSeriesGroup.SetFitWidthFlag( true );
    m_FeaSSXSCOneSixSeriesGroup.AddCounter( m_FeaSSXSCOneSixSeriesDegreeCounter, "Degree", 125 );

    m_FeaSSXSCCurrDisplayGroup = nullptr;

    SubSurfXSCDisplayGroup( &m_FeaSSXSCPointGroup );



    //===== SSControl ====//
    m_GenLayout.AddSubGroupLayout( m_FeaSSConGroup, m_GenLayout.GetRemainX(), m_GenLayout.GetRemainY() );
    m_FeaSSConGroup.SetY( start_y );

    m_FeaSSConGroup.AddDividerBox( "Control Sub-Surface" );

    m_FeaSSConGroup.SetFitWidthFlag( false );
    m_FeaSSConGroup.SetSameLineFlag( true );
    m_FeaSSConGroup.AddLabel( "Tag", m_FeaSSConGroup.GetRemainX() / 3 );
    m_FeaSSConGroup.SetButtonWidth( m_FeaSSConGroup.GetRemainX() / 3 );
    m_FeaSSConGroup.AddButton( m_FeaSSConInsideButton, "Inside" );
    m_FeaSSConGroup.AddButton( m_FeaSSConOutsideButton, "Outside" );
    m_FeaSSConGroup.AddButton( m_FeaSSConNoneButton, "None" );

    m_FeaSSConTestToggleGroup.Init( this );
    m_FeaSSConTestToggleGroup.AddButton( m_FeaSSConInsideButton.GetFlButton() );
    m_FeaSSConTestToggleGroup.AddButton( m_FeaSSConOutsideButton.GetFlButton() );
    m_FeaSSConTestToggleGroup.AddButton( m_FeaSSConNoneButton.GetFlButton() );

    m_FeaSSConGroup.ForceNewLine();

    m_FeaSSConGroup.SetChoiceButtonWidth( m_FeaSSConGroup.GetRemainX() / 3 );

    m_FeaSSConSurfTypeChoice.AddItem( "Upper" );
    m_FeaSSConSurfTypeChoice.AddItem( "Lower" );
    m_FeaSSConSurfTypeChoice.AddItem( "Both" );
    m_FeaSSConGroup.AddChoice( m_FeaSSConSurfTypeChoice, "Upper/Lower", m_FeaSSConGroup.GetRemainX() / 3 );

    m_FeaSSConGroup.SetButtonWidth( m_FeaSSConGroup.GetRemainX() );

    m_FeaSSConGroup.AddButton( m_FeaSSConLEFlagButton, "Leading Edge" );

    m_FeaSSConGroup.SetFitWidthFlag( true );
    m_FeaSSConGroup.SetSameLineFlag( false );
    m_FeaSSConGroup.ForceNewLine();

    m_FeaSSConGroup.AddYGap();
    m_FeaSSConGroup.AddDividerBox( "Spanwise" );

    m_FeaSSConGroup.SetButtonWidth( m_FeaSSConGroup.GetRemainX() / 3 );

    m_FeaSSConGroup.AddSlider( m_FeaSSConUSSlider, "Start U", 1, "%5.4f" );
    m_FeaSSConGroup.AddSlider( m_FeaSSConUESlider, "End U", 1, "%5.4f" );

    m_FeaSSConGroup.AddYGap();
    m_FeaSSConGroup.AddDividerBox( "Chordwise" );

    m_FeaSSConGroup.SetFitWidthFlag( false );
    m_FeaSSConGroup.SetSameLineFlag( true );

    m_FeaSSConGroup.SetButtonWidth( m_FeaSSConGroup.GetW() / 3 );

    m_FeaSSConGroup.AddButton( m_FeaSSConSAbsButton, "Length" );
    m_FeaSSConGroup.AddButton( m_FeaSSConSRelButton, "Length/C" );
    m_FeaSSConGroup.AddButton( m_FeaSSConSEConstButton, "Constant" );

    m_FeaSSConGroup.SetFitWidthFlag( true );
    m_FeaSSConGroup.SetSameLineFlag( false );
    m_FeaSSConGroup.ForceNewLine();

    m_FeaSSConSAbsRelToggleGroup.Init( this );
    m_FeaSSConSAbsRelToggleGroup.AddButton( m_FeaSSConSAbsButton.GetFlButton() );
    m_FeaSSConSAbsRelToggleGroup.AddButton( m_FeaSSConSRelButton.GetFlButton() );

    m_FeaSSConGroup.SetButtonWidth( m_FeaSSConGroup.GetRemainX() / 3 );

    m_FeaSSConGroup.AddSlider( m_FeaSSConSLenSlider, "Start Length", 10.0, "%5.4f" );
    m_FeaSSConGroup.AddSlider( m_FeaSSConSFracSlider, "Start Length/C", 1.0, "%5.4f" );
    m_FeaSSConGroup.AddSlider( m_FeaSSConELenSlider, "End Length", 10.0, "%5.4f" );
    m_FeaSSConGroup.AddSlider( m_FeaSSConEFracSlider, "End Length/C", 1.0, "%5.4f" );

    m_FeaSSConGroup.AddYGap();
    m_FeaSSConGroup.AddDividerBox( "Surface End Angle" );

    m_FeaSSConGroup.SetSameLineFlag( true );
    m_FeaSSConGroup.SetFitWidthFlag( false );

    m_FeaSSConGroup.AddButton( m_FeaSSConSAngleButton, "Start" );
    m_FeaSSConGroup.AddButton( m_FeaSSConEAngleButton, "End" );
    m_FeaSSConGroup.AddButton( m_FeaSSConSameAngleButton, "Same Angle" );

    m_FeaSSConGroup.SetSameLineFlag( false );
    m_FeaSSConGroup.SetFitWidthFlag( true );
    m_FeaSSConGroup.ForceNewLine();

    m_FeaSSConGroup.AddSlider( m_FeaSSConSAngleSlider, "Start Angle", 10.0, "%5.4f" );
    m_FeaSSConGroup.AddSlider( m_FeaSSConEAngleSlider, "End Angle", 10.0, "%5.4f" );

    m_FeaSSConGroup.AddSlider( m_FeaSSConTessSlider, "Num Points", 100, "%5.0f" );

    //==== SSLineArray ====//
    m_GenLayout.AddSubGroupLayout( m_FeaSSLineArrayGroup, m_GenLayout.GetRemainX(), m_GenLayout.GetRemainY() );
    m_FeaSSLineArrayGroup.SetY( start_y );

    m_FeaSSLineArrayGroup.AddDividerBox( "Line Array Sub-Surface" );

    m_FeaSSLineArrayGroup.SetSameLineFlag( true );
    m_FeaSSLineArrayGroup.SetFitWidthFlag( false );

    m_FeaSSLineArrayGroup.AddLabel( "Line Type", m_FeaSSLineArrayGroup.GetRemainX() / 3 );
    m_FeaSSLineArrayGroup.SetButtonWidth( m_FeaSSLineArrayGroup.GetRemainX() / 2 );
    m_FeaSSLineArrayGroup.AddButton( m_FeaSSLineArrayConstUButton, "U" );
    m_FeaSSLineArrayGroup.AddButton( m_FeaSSLineArrayConstWButton, "W" );

    m_FeaSSLineArrayConstToggleGroup.Init( this );
    m_FeaSSLineArrayConstToggleGroup.AddButton( m_FeaSSLineArrayConstUButton.GetFlButton() );
    m_FeaSSLineArrayConstToggleGroup.AddButton( m_FeaSSLineArrayConstWButton.GetFlButton() );

    m_FeaSSLineArrayGroup.ForceNewLine();

    m_FeaSSLineArrayGroup.SetButtonWidth( m_FeaSSLineArrayGroup.GetRemainX() / 3 );

    m_FeaSSLineArrayGroup.AddLabel( "Direction", m_FeaSSLineArrayGroup.GetRemainX() / 3 );
    m_FeaSSLineArrayGroup.AddButton( m_FeaSSLineArrayPosDirToggle, "Positive" );
    m_FeaSSLineArrayGroup.AddButton( m_FeaSSLineArrayNegDirToggle, "Negative" );

    m_FeaSSLineArrayPosNegDirToggleGroup.Init( this );
    m_FeaSSLineArrayPosNegDirToggleGroup.AddButton( m_FeaSSLineArrayNegDirToggle.GetFlButton() );
    m_FeaSSLineArrayPosNegDirToggleGroup.AddButton( m_FeaSSLineArrayPosDirToggle.GetFlButton() );

    m_FeaSSLineArrayGroup.ForceNewLine();
    m_FeaSSLineArrayGroup.SetSameLineFlag( false );
    m_FeaSSLineArrayGroup.SetFitWidthFlag( true );

    m_FeaSSLineArrayGroup.AddSlider( m_FeaSSLineArrayStartLocSlider, "Start Location", 0.5, "%5.4f" );
    m_FeaSSLineArrayGroup.AddSlider( m_FeaSSLineArrayEndLocSlider, "End Location", 0.5, "%5.4f" );
    m_FeaSSLineArrayGroup.AddSlider( m_FeaSSLineArraySpacingSlider, "Spacing", 0.5, "%5.4f" );

    m_FeaSSLineArrayGroup.AddYGap();

    m_FeaSSLineArrayGroup.AddButton( m_IndividualizeSSLineArrayButton, "Individualize SSLine Array" );

    //==== SSFiniteLine ====//
    m_GenLayout.AddSubGroupLayout( m_FeaSSFLineGroup, m_GenLayout.GetRemainX(), m_GenLayout.GetRemainY() );
    m_FeaSSFLineGroup.SetY( start_y );

    m_FeaSSFLineGroup.AddDividerBox( "Finite Line Sub-Surface" );

    m_FeaSSFLineGroup.ForceNewLine();
    m_FeaSSFLineGroup.SetFitWidthFlag( true );
    m_FeaSSFLineGroup.SetSameLineFlag( false );

    m_FeaSSFLineGroup.SetButtonWidth( m_FeaSSLineGroup.GetRemainX() / 3 );
    m_FeaSSFLineGroup.SetChoiceButtonWidth( m_FeaSSLineGroup.GetRemainX() / 3 );

    m_FeaSSFLineGroup.AddSlider( m_FeaSSFLineUStartSlider, "U Start", 1, "%5.4f" );
    m_FeaSSFLineGroup.AddSlider( m_FeaSSFLineUEndSlider, "U End", 1, "%5.4f" );
    m_FeaSSFLineGroup.AddSlider( m_FeaSSFLineWStartSlider, "W Start", 1, "%5.4f" );
    m_FeaSSFLineGroup.AddSlider( m_FeaSSFLineWEndSlider, "W End", 1, "%5.4f" );

}

FeaPartEditScreen::~FeaPartEditScreen()
{
}

void FeaPartEditScreen::Show()
{
    if ( Update() )
    {
        BasicScreen::Show();
    }
}

bool FeaPartEditScreen::Update()
{
    assert( m_ScreenMgr );

    BasicScreen::Update();

    // Close the screen if StructScreen is not open
    VspScreen* struct_screen = m_ScreenMgr->GetScreen( vsp::VSP_STRUCT_SCREEN );
    if ( !struct_screen )
    {
        Hide();
        return false;
    }
    else if ( !struct_screen->IsShown() )
    {
        Hide();
        return false;
    }

    Vehicle* veh = m_ScreenMgr->GetVehiclePtr();

    if ( veh )
    {
        //==== Update Perpendicular Rib Choice ====//
        UpdatePerpendicularRibChoice();

        //==== Update FixPoint Parent Surf Choice ====//
        UpdateFixPointParentChoice();

        UpdateTrimPartChoice();

        //===== FeaProperty Update =====//
        UpdateFeaPropertyChoice();

        UpdateUnitLabels();

        // Activate by default, only need to de-activate in following code.
        m_ShellPropertyChoice.Activate();
        m_OrientationChoice.Activate();
        m_CapPropertyChoice.Activate();
        m_KeepDelShellElementsToggleGroup.Activate();
        m_CreateBeamElementsToggle.Activate();

        if ( StructureMgr.ValidTotalFeaStructInd( StructureMgr.m_CurrStructIndex() ) )
        {
            vector< FeaStructure* > structVec = StructureMgr.GetAllFeaStructs();


            // Update Current FeaPart
            if ( StructureMgr.GetCurrPartIndex() < structVec[StructureMgr.m_CurrStructIndex()]->NumFeaParts() )
            {
                FeaPart* feaprt = structVec[StructureMgr.m_CurrStructIndex()]->GetFeaPart( StructureMgr.GetCurrPartIndex() );
                if ( feaprt )
                {
                    m_FeaPartNameInput.Update( feaprt->GetName() );

                    if ( !feaprt->m_CreateBeamElements() )
                    {
                        m_CapPropertyChoice.Deactivate();
                    }

                    if ( feaprt->m_KeepDelShellElements() == vsp::FEA_DELETE )
                    {
                        m_ShellPropertyChoice.Deactivate();
                        m_OrientationChoice.Deactivate();
                    }

                    if ( feaprt->GetType() == vsp::FEA_SKIN )
                    {
                        FeaSkin* skin = dynamic_cast<FeaSkin*>( feaprt );
                        assert( skin );

                        m_RemoveSkinToggle.Update( skin->m_RemoveSkinFlag.GetID() );
                        m_RemoveRootCapToggle.Update( skin->m_RemoveRootCapFlag.GetID() );
                        m_RemoveTipCapToggle.Update( skin->m_RemoveTipCapFlag.GetID() );

                        m_KeepDelShellElementsToggleGroup.Deactivate();
                        m_CreateBeamElementsToggle.Deactivate();
                        m_CapPropertyChoice.Deactivate();

                        if ( skin->m_RemoveSkinFlag() )
                        {
                            m_ShellPropertyChoice.Deactivate();
                            m_RemoveRootCapToggle.Deactivate();
                            m_RemoveTipCapToggle.Deactivate();
                        }

                        FeaPartDispGroup( &m_SkinEditLayout );
                    }
                    else if ( feaprt->GetType() == vsp::FEA_SLICE )
                    {
                        FeaSlice* slice = dynamic_cast<FeaSlice*>( feaprt );
                        assert( slice );

                        m_SliceOrientationChoice.Update( slice->m_OrientationPlane.GetID() );
                        m_SlicePosTypeToggleGroup.Update( slice->m_AbsRelParmFlag.GetID() );

                        if ( slice->m_AbsRelParmFlag() == vsp::REL )
                        {
                            slice->m_RelCenterLocation.Activate();
                            slice->m_AbsCenterLocation.Deactivate();
                            m_SliceCenterLocSlider.Update( 1, slice->m_RelCenterLocation.GetID(), slice->m_AbsCenterLocation.GetID() );
                        }
                        else
                        {
                            slice->m_RelCenterLocation.Deactivate();
                            slice->m_AbsCenterLocation.Activate();
                            m_SliceCenterLocSlider.Update( 2, slice->m_RelCenterLocation.GetID(), slice->m_AbsCenterLocation.GetID() );
                        }

                        m_SliceRotAxisToggleGroup.Update( slice->m_RotationAxis.GetID() );
                        m_CreateBeamElementsToggle.Update( slice->m_CreateBeamElements.GetID() );
                        m_KeepDelShellElementsToggleGroup.Update( slice->m_KeepDelShellElements.GetID() );

                        m_SliceXRotSlider.Update( slice->m_XRot.GetID() );
                        m_SliceYRotSlider.Update( slice->m_YRot.GetID() );
                        m_SliceZRotSlider.Update( slice->m_ZRot.GetID() );

                        if ( slice->m_RotationAxis() == vsp::X_DIR )
                        {
                            m_SliceXRotSlider.Activate();
                            m_SliceYRotSlider.Deactivate();
                            m_SliceZRotSlider.Deactivate();
                        }
                        else if ( slice->m_RotationAxis() == vsp::Y_DIR )
                        {
                            m_SliceXRotSlider.Deactivate();
                            m_SliceYRotSlider.Activate();
                            m_SliceZRotSlider.Deactivate();
                        }
                        else if ( slice->m_RotationAxis() == vsp::Z_DIR )
                        {
                            m_SliceXRotSlider.Deactivate();
                            m_SliceYRotSlider.Deactivate();
                            m_SliceZRotSlider.Activate();
                        }

                        FeaPartDispGroup( &m_SliceEditLayout );
                    }
                    else if ( feaprt->GetType() == vsp::FEA_RIB )
                    {
                        FeaRib* rib = dynamic_cast<FeaRib*>( feaprt );
                        assert( rib );

                        m_RibNormalTypeGroup.Update( rib->m_PerpendicularEdgeType.GetID() );

                        if ( m_PerpendicularEdgeVec.size() == 0 )
                        {
                            m_RibSparNormalToggle.Deactivate();
                        }
                        else
                        {
                            m_RibSparNormalToggle.Activate();
                        }

                        if ( rib->m_PerpendicularEdgeType.Get() != vsp::SPAR_NORMAL || m_PerpendicularEdgeVec.size() == 0 )
                        {
                            m_RibPerpendicularSparChoice.Deactivate();
                        }
                        else
                        {
                            m_RibPerpendicularSparChoice.Activate();
                        }

                        m_RibSectionLimitToggle.Update( rib->m_LimitRibToSectionFlag.GetID() );
                        m_RibStartSectIndexSelector.Update( rib->m_StartWingSection.GetID() );
                        m_RibEndSectIndexSelector.Update( rib->m_EndWingSection.GetID() );

                        if ( rib->m_LimitRibToSectionFlag() )
                        {
                            m_RibStartSectIndexSelector.Activate();
                            m_RibEndSectIndexSelector.Activate();
                        }
                        else
                        {
                            m_RibStartSectIndexSelector.Deactivate();
                            m_RibEndSectIndexSelector.Deactivate();
                        }

                        m_RibPosTypeToggleGroup.Update( rib->m_AbsRelParmFlag.GetID() );

                        if ( rib->m_AbsRelParmFlag() == vsp::REL )
                        {
                            rib->m_RelCenterLocation.Activate();
                            rib->m_AbsCenterLocation.Deactivate();
                            m_RibPosSlider.Update( 1, rib->m_RelCenterLocation.GetID(), rib->m_AbsCenterLocation.GetID() );
                        }
                        else
                        {
                            rib->m_RelCenterLocation.Deactivate();
                            rib->m_AbsCenterLocation.Activate();
                            m_RibPosSlider.Update( 2, rib->m_RelCenterLocation.GetID(), rib->m_AbsCenterLocation.GetID() );
                        }

                        m_RibThetaSlider.Update( rib->m_Theta.GetID() );
                        m_RibTrimToBBoxToggle.Update( rib->m_BndBoxTrimFlag.GetID() );
                        m_RibRotateDihedralToggle.Update( rib->m_MatchDihedralFlag.GetID() );
                        m_CreateBeamElementsToggle.Update( rib->m_CreateBeamElements.GetID() );
                        m_KeepDelShellElementsToggleGroup.Update( rib->m_KeepDelShellElements.GetID() );

                        FeaPartDispGroup( &m_RibEditLayout );
                    }
                    else if ( feaprt->GetType() == vsp::FEA_SPAR )
                    {
                        FeaSpar* spar = dynamic_cast<FeaSpar*>( feaprt );
                        assert( spar );

                        m_SparPosTypeToggleGroup.Update( spar->m_AbsRelParmFlag.GetID() );

                        m_SparSectionLimitToggle.Update( spar->m_LimitSparToSectionFlag.GetID() );
                        m_SparStartSectIndexSelector.Update( spar->m_StartWingSection.GetID() );
                        m_SparEndSectIndexSelector.Update( spar->m_EndWingSection.GetID() );

                        if ( spar->m_LimitSparToSectionFlag() )
                        {
                            m_SparStartSectIndexSelector.Activate();
                            m_SparEndSectIndexSelector.Activate();
                        }
                        else
                        {
                            m_SparStartSectIndexSelector.Deactivate();
                            m_SparEndSectIndexSelector.Deactivate();
                        }

                        m_SparSetPerChordToggle.Update( spar->m_UsePercentChord.GetID() );
                        m_SparRootChordSlider.Update( spar->m_PercentRootChord.GetID() );
                        m_SparTipChordSlider.Update( spar->m_PercentTipChord.GetID() );
                        m_SparThetaSlider.Update( spar->m_Theta.GetID() );

                        if ( spar->m_AbsRelParmFlag() == vsp::REL )
                        {
                            if ( !spar->m_UsePercentChord() )
                            {
                                m_SparPosSlider.Activate();
                                spar->m_RelCenterLocation.Activate();
                                spar->m_AbsCenterLocation.Deactivate();
                            }
                            m_SparPosSlider.Update( 1, spar->m_RelCenterLocation.GetID(), spar->m_AbsCenterLocation.GetID() );
                        }
                        else
                        {
                            if ( !spar->m_UsePercentChord() )
                            {
                                m_SparPosSlider.Activate();
                                spar->m_AbsCenterLocation.Activate();
                                spar->m_RelCenterLocation.Deactivate();
                            }
                            m_SparPosSlider.Update( 2, spar->m_RelCenterLocation.GetID(), spar->m_AbsCenterLocation.GetID() );
                        }

                        if ( spar->m_UsePercentChord() )
                        {
                            m_SparPosSlider.Deactivate();
                            m_SparThetaSlider.Deactivate();
                            m_SparRootChordSlider.Activate();
                            m_SparTipChordSlider.Activate();
                        }
                        else
                        {
                            m_SparThetaSlider.Activate();
                            m_SparRootChordSlider.Deactivate();
                            m_SparTipChordSlider.Deactivate();
                        }

                        m_SparTrimToBBoxToggle.Update( spar->m_BndBoxTrimFlag.GetID() );
                        m_CreateBeamElementsToggle.Update( spar->m_CreateBeamElements.GetID() );
                        m_KeepDelShellElementsToggleGroup.Update( spar->m_KeepDelShellElements.GetID() );

                        FeaPartDispGroup( &m_SparEditLayout );
                    }
                    else if ( feaprt->GetType() == vsp::FEA_POLY_SPAR )
                    {
                        FeaPolySpar* polyspar = dynamic_cast<FeaPolySpar*>( feaprt );
                        assert( polyspar );

                        m_CreateBeamElementsToggle.Update( polyspar->m_CreateBeamElements.GetID() );
                        m_KeepDelShellElementsToggleGroup.Update( polyspar->m_KeepDelShellElements.GetID() );

                        UpdatePolySparPointBrowser( polyspar );

                        PolySparPoint* spt = polyspar->GetPt( m_PolySparPointBrowserSelect );

                        if ( spt )
                        {
                            m_PolySparPtNameInput.Update( spt->GetName() );

                            m_PolySparPointTypeToggleGroup.Update( spt->m_SpanMode.GetID() );

                            m_PolySparPointU01Slider.Update( spt->m_U01.GetID() );
                            m_PolySparPointU0NSlider.Update( spt->m_U0N.GetID() );
                            m_PolySparPointEtaSlider.Update( spt->m_Eta.GetID() );

                            m_PolySparPointXoCSlider.Update( spt->m_XoC.GetID() );
                            m_PolySparPointXoCSlider.Activate();

                            m_PolySparPointU01Toggle.Activate();
                            m_PolySparPointU0NToggle.Activate();
                            m_PolySparPointEtaToggle.Activate();

                            m_DelPolySparPoint.Activate();

                            m_PolySparPtNameInput.Activate();

                            if ( spt->m_SpanMode() == vsp::POLY_SPAR_POINT_U01 )
                            {
                                m_PolySparPointU01Slider.Activate();
                                m_PolySparPointU0NSlider.Deactivate();
                                m_PolySparPointEtaSlider.Deactivate();
                            }
                            else if ( spt->m_SpanMode() == vsp::POLY_SPAR_POINT_U0N )
                            {
                                m_PolySparPointU01Slider.Deactivate();
                                m_PolySparPointU0NSlider.Activate();
                                m_PolySparPointEtaSlider.Deactivate();
                            }
                            else // if ( spt->m_SpanMode() == vsp::POLY_SPAR_POINT_ETA )
                            {
                                m_PolySparPointU01Slider.Deactivate();
                                m_PolySparPointU0NSlider.Deactivate();
                                m_PolySparPointEtaSlider.Activate();
                            }
                        }
                        else
                        {
                            m_PolySparPtNameInput.Update( "" );
                            m_PolySparPtNameInput.Deactivate();

                            m_PolySparPointXoCSlider.Deactivate();

                            m_PolySparPointU01Slider.Deactivate();
                            m_PolySparPointU0NSlider.Deactivate();
                            m_PolySparPointEtaSlider.Deactivate();

                            m_PolySparPointU01Toggle.Deactivate();
                            m_PolySparPointU0NToggle.Deactivate();
                            m_PolySparPointEtaToggle.Deactivate();

                            m_DelPolySparPoint.Deactivate();
                        }

                        FeaPartDispGroup( &m_PolySparEditLayout );
                    }
                    else if ( feaprt->GetType() == vsp::FEA_FIX_POINT )
                    {
                        FeaFixPoint* fixpt = dynamic_cast<FeaFixPoint*>( feaprt );
                        assert( fixpt );

                        m_FixPointTypeChoice.Update( fixpt->m_FixedPointType.GetID() );

                        if ( fixpt->m_FixedPointType() == vsp::FEA_FIX_PT_GEOM_ORIGIN ||
                             fixpt->m_FixedPointType() == vsp::FEA_FIX_PT_GEOM_CG )
                        {
                            m_FixPointOtherGeomPicker.SetGeomChoice( fixpt->m_OtherGeomID );
                        }
                        else
                        {
                            m_FixPointOtherGeomPicker.SetGeomChoice( "" );
                        }
                        m_FixPointOtherGeomPicker.Update();

                        m_FixPointULocSlider.Update( fixpt->m_PosU.GetID() );
                        m_FixPointWLocSlider.Update( fixpt->m_PosW.GetID() );
                        m_FixPointAbsXSlider.Update( fixpt->m_AbsX.GetID() );
                        m_FixPointAbsYSlider.Update( fixpt->m_AbsY.GetID() );
                        m_FixPointAbsZSlider.Update( fixpt->m_AbsZ.GetID() );
                        m_FixPointDeltaXSlider.Update( fixpt->m_DeltaX.GetID() );
                        m_FixPointDeltaYSlider.Update( fixpt->m_DeltaY.GetID() );
                        m_FixPointDeltaZSlider.Update( fixpt->m_DeltaZ.GetID() );
                        m_FixPointDeltaUSlider.Update( fixpt->m_DeltaU.GetID() );
                        m_FixPointDeltaVSlider.Update( fixpt->m_DeltaV.GetID() );
                        m_FixPointDeltaNSlider.Update( fixpt->m_DeltaN.GetID() );

                        m_FixPointMassToggle.Update( fixpt->m_FixPointMassFlag.GetID() );
                        m_FixPointMassUnitChoice.Update( fixpt->m_MassUnit.GetID() );
                        m_FixPointMassInput.Update( fixpt->m_FixPointMass.GetID() );

                        m_FixPointMass_FEMOutput.Update( fixpt->m_FixPointMass_FEM.GetID() );

                        if ( fixpt->m_FixPointMassFlag() )
                        {
                            m_FixPointMassInput.Activate();
                            m_FixPointMassUnitChoice.Activate();
                            m_FixPointMass_FEMOutput.Activate();
                            m_FixPointMassUnit_FEM.Activate();
                        }
                        else
                        {
                            m_FixPointMassInput.Deactivate();
                            m_FixPointMassUnitChoice.Deactivate();
                            m_FixPointMass_FEMOutput.Deactivate();
                            m_FixPointMassUnit_FEM.Deactivate();
                        }

                        if ( fixpt->m_FixedPointType() == vsp::FEA_FIX_PT_ON_BODY )
                        {
                            m_FixPointAbsXSlider.Deactivate();
                            m_FixPointAbsYSlider.Deactivate();
                            m_FixPointAbsZSlider.Deactivate();

                            m_FixPointDeltaXSlider.Deactivate();
                            m_FixPointDeltaYSlider.Deactivate();
                            m_FixPointDeltaZSlider.Deactivate();

                            m_FixPointDeltaUSlider.Deactivate();
                            m_FixPointDeltaVSlider.Deactivate();
                            m_FixPointDeltaNSlider.Deactivate();

                            m_FixPointParentSurfChoice.Activate();
                            m_FixPointULocSlider.Activate();
                            m_FixPointWLocSlider.Activate();

                            m_FixPointOtherGeomPicker.Deactivate();
                        }
                        else if ( fixpt->m_FixedPointType() == vsp::FEA_FIX_PT_GLOBAL_XYZ )
                        {
                            m_FixPointAbsXSlider.Activate();
                            m_FixPointAbsYSlider.Activate();
                            m_FixPointAbsZSlider.Activate();

                            m_FixPointDeltaXSlider.Deactivate();
                            m_FixPointDeltaYSlider.Deactivate();
                            m_FixPointDeltaZSlider.Deactivate();

                            m_FixPointDeltaUSlider.Deactivate();
                            m_FixPointDeltaVSlider.Deactivate();
                            m_FixPointDeltaNSlider.Deactivate();

                            m_FixPointParentSurfChoice.Deactivate();
                            m_FixPointULocSlider.Deactivate();
                            m_FixPointWLocSlider.Deactivate();

                            m_FixPointOtherGeomPicker.Deactivate();
                        }
                        else if ( fixpt->m_FixedPointType() == vsp::FEA_FIX_PT_DELTA_XYZ ||
                                  fixpt->m_FixedPointType() == vsp::FEA_FIX_PT_GEOM_ORIGIN ||
                                  fixpt->m_FixedPointType() == vsp::FEA_FIX_PT_GEOM_CG )
                        {

                            m_FixPointAbsXSlider.Deactivate();
                            m_FixPointAbsYSlider.Deactivate();
                            m_FixPointAbsZSlider.Deactivate();

                            m_FixPointDeltaXSlider.Activate();
                            m_FixPointDeltaYSlider.Activate();
                            m_FixPointDeltaZSlider.Activate();

                            m_FixPointDeltaUSlider.Deactivate();
                            m_FixPointDeltaVSlider.Deactivate();
                            m_FixPointDeltaNSlider.Deactivate();

                            if ( fixpt->m_FixedPointType() == vsp::FEA_FIX_PT_DELTA_XYZ )
                            {
                                m_FixPointParentSurfChoice.Activate();
                                m_FixPointULocSlider.Activate();
                                m_FixPointWLocSlider.Activate();

                                m_FixPointOtherGeomPicker.Deactivate();
                            }
                            else
                            {
                                m_FixPointParentSurfChoice.Deactivate();
                                m_FixPointULocSlider.Deactivate();
                                m_FixPointWLocSlider.Deactivate();

                                m_FixPointOtherGeomPicker.Activate();
                            }

                        }
                        else if ( fixpt->m_FixedPointType() == vsp::FEA_FIX_PT_DELTA_UVN )
                        {
                            m_FixPointAbsXSlider.Deactivate();
                            m_FixPointAbsYSlider.Deactivate();
                            m_FixPointAbsZSlider.Deactivate();

                            m_FixPointDeltaXSlider.Deactivate();
                            m_FixPointDeltaYSlider.Deactivate();
                            m_FixPointDeltaZSlider.Deactivate();

                            m_FixPointDeltaUSlider.Activate();
                            m_FixPointDeltaVSlider.Activate();
                            m_FixPointDeltaNSlider.Activate();

                            m_FixPointParentSurfChoice.Activate();
                            m_FixPointULocSlider.Activate();
                            m_FixPointWLocSlider.Activate();

                            m_FixPointOtherGeomPicker.Deactivate();
                        }

                        m_ShellPropertyChoice.Deactivate();
                        m_OrientationChoice.Deactivate();
                        m_CapPropertyChoice.Deactivate();
                        m_KeepDelShellElementsToggleGroup.Deactivate();
                        m_CreateBeamElementsToggle.Deactivate();

                        FeaPartDispGroup( &m_FixPointEditLayout );
                    }
                    else if ( feaprt->GetType() == vsp::FEA_DOME )
                    {
                        FeaDome* dome = dynamic_cast<FeaDome*>( feaprt );
                        assert( dome );

                        m_DomeFlipDirButton.Update( dome->m_FlipDirectionFlag.GetID() );
                        m_DomeSpineAttachButton.Update( dome->m_SpineAttachFlag.GetID() );
                        m_DomeUSpineSlider.Update( dome->m_USpineLoc.GetID() );
                        m_DomeARadSlider.Update( dome->m_Aradius.GetID() );
                        m_DomeBRadSlider.Update( dome->m_Bradius.GetID() );
                        m_DomeCRadSlider.Update( dome->m_Cradius.GetID() );
                        m_DomeXSlider.Update( dome->m_XLoc.GetID() );
                        m_DomeYSlider.Update( dome->m_YLoc.GetID() );
                        m_DomeZSlider.Update( dome->m_ZLoc.GetID() );
                        m_DomeXRotSlider.Update( dome->m_XRot.GetID() );
                        m_DomeYRotSlider.Update( dome->m_YRot.GetID() );
                        m_DomeZRotSlider.Update( dome->m_ZRot.GetID() );

                        m_CreateBeamElementsToggle.Update( dome->m_CreateBeamElements.GetID() );
                        m_KeepDelShellElementsToggleGroup.Update( dome->m_KeepDelShellElements.GetID() );

                        if ( dome->m_SpineAttachFlag() )
                        {
                            m_DomeUSpineSlider.Activate();
                        }
                        else
                        {
                            m_DomeUSpineSlider.Deactivate();
                        }

                        FeaPartDispGroup( &m_DomeEditLayout );
                    }
                    else if ( feaprt->GetType() == vsp::FEA_RIB_ARRAY )
                    {
                        FeaRibArray* rib_array = dynamic_cast<FeaRibArray*>( feaprt );
                        assert( rib_array );

                        m_RibArrayNormalTypeGroup.Update( rib_array->m_PerpendicularEdgeType.GetID() );

                        if ( m_PerpendicularEdgeVec.size() == 0 )
                        {
                            m_RibArraySparNormalToggle.Deactivate();
                        }
                        else
                        {
                            m_RibArraySparNormalToggle.Activate();
                        }

                        if ( rib_array->m_PerpendicularEdgeType.Get() != vsp::SPAR_NORMAL || m_PerpendicularEdgeVec.size() == 0 )
                        {
                            m_RibArrayPerpendicularSparChoice.Deactivate();
                        }
                        else
                        {
                            m_RibArrayPerpendicularSparChoice.Activate();
                        }

                        m_RibArraySectionLimitToggle.Update( rib_array->m_LimitArrayToSectionFlag.GetID() );
                        m_RibArrayStartSectIndexSelector.Update( rib_array->m_StartWingSection.GetID() );
                        m_RibArrayEndSectIndexSelector.Update( rib_array->m_EndWingSection.GetID() );

                        if ( rib_array->m_LimitArrayToSectionFlag() )
                        {
                            m_RibArrayStartSectIndexSelector.Activate();
                            m_RibArrayEndSectIndexSelector.Activate();
                        }
                        else
                        {
                            m_RibArrayStartSectIndexSelector.Deactivate();
                            m_RibArrayEndSectIndexSelector.Deactivate();
                        }

                        m_RibArrayPosTypeToggleGroup.Update( rib_array->m_AbsRelParmFlag.GetID() );

                        if ( rib_array->m_AbsRelParmFlag() == vsp::REL )
                        {
                            rib_array->m_RelStartLocation.Activate();
                            rib_array->m_AbsStartLocation.Deactivate();
                            rib_array->m_RelEndLocation.Activate();
                            rib_array->m_AbsEndLocation.Deactivate();
                            rib_array->m_RibRelSpacing.Activate();
                            rib_array->m_RibAbsSpacing.Deactivate();
                            m_RibArrayStartLocSlider.Update( 1, rib_array->m_RelStartLocation.GetID(), rib_array->m_AbsStartLocation.GetID() );
                            m_RibArrayEndLocSlider.Update( 1, rib_array->m_RelEndLocation.GetID(), rib_array->m_AbsEndLocation.GetID() );
                            m_RibArraySpacingSlider.Update( 1, rib_array->m_RibRelSpacing.GetID(), rib_array->m_RibAbsSpacing.GetID() );
                        }
                        else
                        {
                            rib_array->m_RelStartLocation.Deactivate();
                            rib_array->m_AbsStartLocation.Activate();
                            rib_array->m_RelEndLocation.Deactivate();
                            rib_array->m_AbsEndLocation.Activate();
                            rib_array->m_RibRelSpacing.Deactivate();
                            rib_array->m_RibAbsSpacing.Activate();
                            m_RibArrayStartLocSlider.Update( 2, rib_array->m_RelStartLocation.GetID(), rib_array->m_AbsStartLocation.GetID() );
                            m_RibArrayEndLocSlider.Update( 2, rib_array->m_RelEndLocation.GetID(), rib_array->m_AbsEndLocation.GetID() );
                            m_RibArraySpacingSlider.Update( 2, rib_array->m_RibRelSpacing.GetID(), rib_array->m_RibAbsSpacing.GetID() );
                        }

                        m_RibArrayThetaSlider.Update( rib_array->m_Theta.GetID() );
                        m_RibArrayTrimToBBoxToggle.Update( rib_array->m_BndBoxTrimFlag.GetID() );
                        m_CreateBeamElementsToggle.Update( rib_array->m_CreateBeamElements.GetID() );
                        m_KeepDelShellElementsToggleGroup.Update( rib_array->m_KeepDelShellElements.GetID() );
                        m_RibArrayPosNegDirToggleGroup.Update( rib_array->m_PositiveDirectionFlag.GetID() );
                        m_RibArrayRotateDihedralToggle.Update( rib_array->m_MatchDihedralFlag.GetID() );

                        FeaPartDispGroup( &m_RibArrayEditLayout );
                    }
                    else if ( feaprt->GetType() == vsp::FEA_SLICE_ARRAY )
                    {
                        FeaSliceArray* slice_array = dynamic_cast<FeaSliceArray*>( feaprt );
                        assert( slice_array );

                        m_SliceArrayOrientationChoice.Update( slice_array->m_OrientationPlane.GetID() );
                        m_SliceArrayPosTypeToggleGroup.Update( slice_array->m_AbsRelParmFlag.GetID() );

                        if ( slice_array->m_AbsRelParmFlag() == vsp::REL )
                        {
                            slice_array->m_RelStartLocation.Activate();
                            slice_array->m_AbsStartLocation.Deactivate();
                            slice_array->m_RelEndLocation.Activate();
                            slice_array->m_AbsEndLocation.Deactivate();
                            slice_array->m_SliceRelSpacing.Activate();
                            slice_array->m_SliceAbsSpacing.Deactivate();
                            m_SliceArrayStartLocSlider.Update( 1, slice_array->m_RelStartLocation.GetID(), slice_array->m_AbsStartLocation.GetID() );
                            m_SliceArrayEndLocSlider.Update( 1, slice_array->m_RelEndLocation.GetID(), slice_array->m_AbsEndLocation.GetID() );
                            m_SliceArraySpacingSlider.Update( 1, slice_array->m_SliceRelSpacing.GetID(), slice_array->m_SliceAbsSpacing.GetID() );
                        }
                        else
                        {
                            slice_array->m_RelStartLocation.Deactivate();
                            slice_array->m_AbsStartLocation.Activate();
                            slice_array->m_RelEndLocation.Deactivate();
                            slice_array->m_AbsEndLocation.Activate();
                            slice_array->m_SliceRelSpacing.Deactivate();
                            slice_array->m_SliceAbsSpacing.Activate();
                            m_SliceArrayStartLocSlider.Update( 2, slice_array->m_RelStartLocation.GetID(), slice_array->m_AbsStartLocation.GetID() );
                            m_SliceArrayEndLocSlider.Update( 2, slice_array->m_RelEndLocation.GetID(), slice_array->m_AbsEndLocation.GetID() );
                            m_SliceArraySpacingSlider.Update( 2, slice_array->m_SliceRelSpacing.GetID(), slice_array->m_SliceAbsSpacing.GetID() );
                        }

                        m_SliceArrayPosNegDirToggleGroup.Update( slice_array->m_PositiveDirectionFlag.GetID() );
                        m_SliceArrayRotAxisToggleGroup.Update( slice_array->m_RotationAxis.GetID() );
                        m_SliceArrayXRotSlider.Update( slice_array->m_XRot.GetID() );
                        m_SliceArrayYRotSlider.Update( slice_array->m_YRot.GetID() );
                        m_SliceArrayZRotSlider.Update( slice_array->m_ZRot.GetID() );
                        m_CreateBeamElementsToggle.Update( slice_array->m_CreateBeamElements.GetID() );
                        m_KeepDelShellElementsToggleGroup.Update( slice_array->m_KeepDelShellElements.GetID() );

                        if ( slice_array->m_RotationAxis() == vsp::X_DIR )
                        {
                            m_SliceArrayXRotSlider.Activate();
                            m_SliceArrayYRotSlider.Deactivate();
                            m_SliceArrayZRotSlider.Deactivate();
                        }
                        else if ( slice_array->m_RotationAxis() == vsp::Y_DIR )
                        {
                            m_SliceArrayXRotSlider.Deactivate();
                            m_SliceArrayYRotSlider.Activate();
                            m_SliceArrayZRotSlider.Deactivate();
                        }
                        else if ( slice_array->m_RotationAxis() == vsp::Z_DIR )
                        {
                            m_SliceArrayXRotSlider.Deactivate();
                            m_SliceArrayYRotSlider.Deactivate();
                            m_SliceArrayZRotSlider.Activate();
                        }

                        FeaPartDispGroup( &m_SliceArrayEditLayout );
                    }
                    else if ( feaprt->GetType() == vsp::FEA_TRIM )
                    {
                        FeaPartTrim* trim = dynamic_cast<FeaPartTrim*>( feaprt );
                        assert( trim );

                        m_ShellPropertyChoice.Deactivate();
                        m_OrientationChoice.Deactivate();
                        m_CapPropertyChoice.Deactivate();
                        m_KeepDelShellElementsToggleGroup.Deactivate();
                        m_CreateBeamElementsToggle.Deactivate();

                        FeaPartDispGroup( &m_TrimEditLayout );
                    }
                    else
                    {
                        Hide();
                        return false;
                    }
                }
                else
                {
                    Hide();
                    return false;
                }
            }
            else if ( StructureMgr.GetCurrPartIndex() >= structVec[StructureMgr.m_CurrStructIndex()]->NumFeaParts() )
            {
                //===== SubSurface =====//
                SubSurface* subsurf = structVec[StructureMgr.m_CurrStructIndex()]->GetFeaSubSurf( StructureMgr.GetCurrPartIndex() - structVec[StructureMgr.m_CurrStructIndex()]->NumFeaParts() );
                if ( subsurf )
                {
                    m_FeaPartNameInput.Update( subsurf->GetName() );

                    if ( !subsurf->m_CreateBeamElements() )
                    {
                        m_CapPropertyChoice.Deactivate();
                    }

                    if ( subsurf->m_KeepDelShellElements() == vsp::FEA_DELETE )
                    {
                        m_ShellPropertyChoice.Deactivate();
                        m_OrientationChoice.Deactivate();
                    }

                    if ( subsurf->GetType() == vsp::SS_LINE )
                    {
                        SSLine* ssline = dynamic_cast<SSLine*>( subsurf );
                        assert( ssline );

                        m_FeaSSLineConstToggleGroup.Update( ssline->m_ConstType.GetID() );
                        m_FeaSSLineTestToggleGroup.Update( ssline->m_TestType.GetID() );
                        m_FeaSSLineConstSlider.Update( ssline->m_ConstVal.GetID() );
                        m_CreateBeamElementsToggle.Update( ssline->m_CreateBeamElements.GetID() );
                        m_KeepDelShellElementsToggleGroup.Update( ssline->m_KeepDelShellElements.GetID() );

                        if ( ssline->m_TestType() == vsp::NONE )
                        {
                            m_ShellPropertyChoice.Deactivate();
                        }

                        FeaPartDispGroup( &m_FeaSSLineGroup );
                    }
                    else if ( subsurf->GetType() == vsp::SS_RECTANGLE )
                    {
                        SSRectangle* ssrec = dynamic_cast<SSRectangle*>( subsurf );
                        assert( ssrec );

                        m_FeaSSRecTestToggleGroup.Update( ssrec->m_TestType.GetID() );
                        m_FeaSSRecCentUSlider.Update( ssrec->m_CenterU.GetID() );
                        m_FeaSSRecCentWSlider.Update( ssrec->m_CenterW.GetID() );
                        m_FeaSSRecULenSlider.Update( ssrec->m_ULength.GetID() );
                        m_FeaSSRecWLenSlider.Update( ssrec->m_WLength.GetID() );
                        m_FeaSSRecThetaSlider.Update( ssrec->m_Theta.GetID() );
                        m_CreateBeamElementsToggle.Update( ssrec->m_CreateBeamElements.GetID() );
                        m_KeepDelShellElementsToggleGroup.Update( ssrec->m_KeepDelShellElements.GetID() );

                        if ( ssrec->m_TestType() == vsp::NONE )
                        {
                            m_ShellPropertyChoice.Deactivate();
                        }

                        FeaPartDispGroup( &m_FeaSSRecGroup );
                    }
                    else if ( subsurf->GetType() == vsp::SS_ELLIPSE )
                    {
                        SSEllipse* ssell = dynamic_cast<SSEllipse*>( subsurf );
                        assert( ssell );

                        m_FeaSSEllTestToggleGroup.Update( ssell->m_TestType.GetID() );
                        m_FeaSSEllTessSlider.Update( ssell->m_Tess.GetID() );
                        m_FeaSSEllCentUSlider.Update( ssell->m_CenterU.GetID() );
                        m_FeaSSEllCentWSlider.Update( ssell->m_CenterW.GetID() );
                        m_FeaSSEllULenSlider.Update( ssell->m_ULength.GetID() );
                        m_FeaSSEllWLenSlider.Update( ssell->m_WLength.GetID() );
                        m_FeaSSEllThetaSlider.Update( ssell->m_Theta.GetID() );
                        m_CreateBeamElementsToggle.Update( ssell->m_CreateBeamElements.GetID() );
                        m_KeepDelShellElementsToggleGroup.Update( ssell->m_KeepDelShellElements.GetID() );

                        if ( ssell->m_TestType() == vsp::NONE )
                        {
                            m_ShellPropertyChoice.Deactivate();
                        }

                        FeaPartDispGroup( &m_FeaSSEllGroup );
                    }
                    else if ( subsurf->GetType() == vsp::SS_XSEC_CURVE )
                    {
                        SSXSecCurve* ssxsc = dynamic_cast< SSXSecCurve* >( subsurf );
                        assert( ssxsc );

                        m_FeaSSXSCTestToggleGroup.Update( ssxsc->m_TestType.GetID() );
                        // m_FeaSSXSCTessSlider.Update( ssxsc->m_Tess.GetID() );
                        m_FeaSSXSCCentUSlider.Update( ssxsc->m_CenterU.GetID() );
                        m_FeaSSXSCCentWSlider.Update( ssxsc->m_CenterW.GetID() );
                        // m_FeaSSXSCULenSlider.Update( ssxsc->m_ULength.GetID() );
                        // m_FeaSSXSCWLenSlider.Update( ssxsc->m_WLength.GetID() );
                        // m_FeaSSXSCThetaSlider.Update( ssxsc->m_Theta.GetID() );
                        FeaPartDispGroup( & m_FeaSSXSCGroup );

                        // update attribute pointer to SSXSCAttrEditor

                        XSecCurve* xsc = ssxsc->GetXSecCurve();

                        vector < string > empty_coll_ids;
                        empty_coll_ids.push_back( subsurf->m_AttrCollection.GetID() );

                        if ( xsc )
                        {
                            XSecViewScreen *xsscreen = dynamic_cast < XSecViewScreen* > ( m_ScreenMgr->GetScreen( vsp::VSP_XSEC_SCREEN ) );

                            if ( xsscreen )
                            {
                                xsscreen->SetXSecCurve( xsc );
                            }

                            empty_coll_ids.push_back( xsc->GetAttrCollection()->GetID() );

                            m_FeaSSXSecTypeChoice.SetVal( xsc->GetType() );

                            if ( xsc->GetType() == vsp::XS_POINT )
                            {
                                SubSurfXSCDisplayGroup( NULL );
                            }
                            else if ( xsc->GetType() == vsp::XS_SUPER_ELLIPSE )
                            {
                                SubSurfXSCDisplayGroup( &m_FeaSSXSCSuperGroup );

                                SuperXSec* super_xs = dynamic_cast< SuperXSec* >( xsc );
                                assert( super_xs );
                                m_FeaSSXSCSuperHeightSlider.Update( super_xs->m_Height.GetID() );
                                m_FeaSSXSCSuperWidthSlider.Update( super_xs->m_Width.GetID() );
                                m_FeaSSXSCSuperMSlider.Update( super_xs->m_M.GetID() );
                                m_FeaSSXSCSuperNSlider.Update( super_xs->m_N.GetID() );
                                m_FeaSSXSCSuperToggleSym.Update( super_xs->m_TopBotSym.GetID() );
                                m_FeaSSXSCSuperM_botSlider.Update( super_xs->m_M_bot.GetID() );
                                m_FeaSSXSCSuperN_botSlider.Update( super_xs->m_N_bot.GetID() );
                                m_FeaSSXSCSuperMaxWidthLocSlider.Update( super_xs->m_MaxWidthLoc.GetID() );

                                if ( super_xs->m_TopBotSym() )
                                {
                                    m_FeaSSXSCSuperM_botSlider.Deactivate();
                                    m_FeaSSXSCSuperN_botSlider.Deactivate();
                                }
                                else if ( !super_xs->m_TopBotSym() )
                                {
                                    m_FeaSSXSCSuperM_botSlider.Activate();
                                    m_FeaSSXSCSuperN_botSlider.Activate();
                                }
                            }
                            else if ( xsc->GetType() == vsp::XS_CIRCLE )
                            {
                                SubSurfXSCDisplayGroup( &m_FeaSSXSCCircleGroup );
                                CircleXSec* circle_xs = dynamic_cast< CircleXSec* >( xsc );
                                assert( circle_xs );

                                m_FeaSSXSCDiameterSlider.Update( circle_xs->m_Diameter.GetID() );
                            }
                            else if ( xsc->GetType() == vsp::XS_ELLIPSE )
                            {
                                SubSurfXSCDisplayGroup( & m_FeaSSXSCEllipseGroup );

                                EllipseXSec* ellipse_xs = dynamic_cast< EllipseXSec* >( xsc );
                                m_FeaSSXSCEllipseHeightSlider.Update( ellipse_xs->m_Height.GetID() );
                                m_FeaSSXSCEllipseWidthSlider.Update( ellipse_xs->m_Width.GetID() );
                            }
                            else if ( xsc->GetType() == vsp::XS_AC25_773 )
                            {
                                SubSurfXSCDisplayGroup( & m_FeaSSXSCAC25773Group );

                                AC25_773XSec* pilotview_xs = dynamic_cast< AC25_773XSec* >( xsc );
                                m_FeaSSXSCAC25773SeatChoice.Update( pilotview_xs->m_CockpitSide.GetID() );
                            }
                            else if ( xsc->GetType() == vsp::XS_ROUNDED_RECTANGLE )
                            {
                                SubSurfXSCDisplayGroup( & m_FeaSSXSCRoundedRectGroup );
                                RoundedRectXSec* rect_xs = dynamic_cast< RoundedRectXSec* >( xsc );
                                assert( rect_xs );

                                m_FeaSSXSCRRHeightSlider.Update( rect_xs->m_Height.GetID() );
                                m_FeaSSXSCRRWidthSlider.Update( rect_xs->m_Width.GetID() );
                                m_FeaSSXSCRRRadSymRadioGroup.Update( rect_xs->m_RadiusSymmetryType.GetID() );
                                m_FeaSSXSCRRRadiusBRSlider.Update( rect_xs->m_RadiusBR.GetID() );
                                m_FeaSSXSCRRRadiusBLSlider.Update( rect_xs->m_RadiusBL.GetID() );
                                m_FeaSSXSCRRRadiusTLSlider.Update( rect_xs->m_RadiusTL.GetID() );
                                m_FeaSSXSCRRRadiusTRSlider.Update( rect_xs->m_RadiusTR.GetID() );
                                m_FeaSSXSCRRKeyCornerButton.Update( rect_xs->m_KeyCornerParm.GetID() );
                                m_FeaSSXSCRRSkewSlider.Update( rect_xs->m_Skew.GetID() );
                                m_FeaSSXSCRRKeystoneSlider.Update( rect_xs->m_Keystone.GetID() );
                                m_FeaSSXSCRRVSkewSlider.Update( rect_xs->m_VSkew.GetID() );

                                if ( rect_xs->m_RadiusSymmetryType.Get() == vsp::SYM_NONE )
                                {
                                    m_FeaSSXSCRRRadiusBRSlider.Activate();
                                    m_FeaSSXSCRRRadiusBLSlider.Activate();
                                    m_FeaSSXSCRRRadiusTLSlider.Activate();
                                }
                                else if ( rect_xs->m_RadiusSymmetryType.Get() == vsp::SYM_RL )
                                {
                                    m_FeaSSXSCRRRadiusBRSlider.Activate();
                                    m_FeaSSXSCRRRadiusBLSlider.Deactivate();
                                    m_FeaSSXSCRRRadiusTLSlider.Deactivate();
                                }
                                else if ( rect_xs->m_RadiusSymmetryType.Get() == vsp::SYM_TB )
                                {
                                    m_FeaSSXSCRRRadiusBRSlider.Deactivate();
                                    m_FeaSSXSCRRRadiusTLSlider.Activate();
                                    m_FeaSSXSCRRRadiusBLSlider.Deactivate();
                                }
                                else if ( rect_xs->m_RadiusSymmetryType.Get() == vsp::SYM_ALL )
                                {
                                    m_FeaSSXSCRRRadiusBRSlider.Deactivate();
                                    m_FeaSSXSCRRRadiusBLSlider.Deactivate();
                                    m_FeaSSXSCRRRadiusTLSlider.Deactivate();
                                }
                            }
                            else if ( xsc->GetType() == vsp::XS_GENERAL_FUSE )
                            {
                                SubSurfXSCDisplayGroup( &m_FeaSSXSCGenGroup );
                                GeneralFuseXSec* gen_xs = dynamic_cast< GeneralFuseXSec* >( xsc );
                                assert( gen_xs );

                                m_FeaSSXSCGenHeightSlider.Update( gen_xs->m_Height.GetID() );
                                m_FeaSSXSCGenWidthSlider.Update( gen_xs->m_Width.GetID() );
                                m_FeaSSXSCGenMaxWidthLocSlider.Update( gen_xs->m_MaxWidthLoc.GetID() );
                                m_FeaSSXSCGenCornerRadSlider.Update( gen_xs->m_CornerRad.GetID() );
                                m_FeaSSXSCGenTopTanAngleSlider.Update( gen_xs->m_TopTanAngle.GetID() );
                                m_FeaSSXSCGenBotTanAngleSlider.Update( gen_xs->m_BotTanAngle.GetID() );
                                m_FeaSSXSCGenTopStrSlider.Update( gen_xs->m_TopStr.GetID() );
                                m_FeaSSXSCGenBotStrSlider.Update( gen_xs->m_BotStr.GetID() );
                                m_FeaSSXSCGenUpStrSlider.Update( gen_xs->m_UpStr.GetID() );
                                m_FeaSSXSCGenLowStrSlider.Update( gen_xs->m_LowStr.GetID() );
                            }
                            else if ( xsc->GetType() == vsp::XS_FOUR_SERIES )
                            {
                                SubSurfXSCDisplayGroup( &m_FeaSSXSCFourSeriesGroup );
                                FourSeries* fs_xs = dynamic_cast< FourSeries* >( xsc );
                                assert( fs_xs );

                                m_FeaSSXSCFourChordSlider.Update( fs_xs->m_Chord.GetID() );
                                m_FeaSSXSCFourThickChordSlider.Update( fs_xs->m_ThickChord.GetID() );
                                m_FeaSSXSCFourCamberSlider.Update( fs_xs->m_Camber.GetID() );
                                m_FeaSSXSCFourCLiSlider.Update( fs_xs->m_IdealCl.GetID() );
                                m_FeaSSXSCFourCamberGroup.Update( fs_xs->m_CamberInputFlag.GetID() );
                                if ( fs_xs->m_CamberInputFlag() == vsp::MAX_CAMB )
                                {
                                    m_FeaSSXSCFourCamberSlider.Activate();
                                    m_FeaSSXSCFourCLiSlider.Deactivate();
                                }
                                else
                                {
                                    m_FeaSSXSCFourCamberSlider.Deactivate();
                                    m_FeaSSXSCFourCLiSlider.Activate();
                                }
                                m_FeaSSXSCFourCamberLocSlider.Update( fs_xs->m_CamberLoc.GetID() );
                                m_FeaSSXSCFourInvertButton.Update( fs_xs->m_Invert.GetID() );
                                m_FeaSSXSCFourNameOutput.Update( fs_xs->GetAirfoilName() );
                                m_FeaSSXSCFourSharpTEButton.Update( fs_xs->m_SharpTE.GetID() );
                                m_FeaSSXSCFourDegreeCounter.Update( fs_xs->m_FitDegree.GetID() );
                            }
                            else if ( xsc->GetType() == vsp::XS_SIX_SERIES )
                            {
                                SubSurfXSCDisplayGroup( &m_FeaSSXSCSixSeriesGroup );
                                SixSeries* ss_xs = dynamic_cast< SixSeries* >( xsc );
                                assert( ss_xs );

                                m_FeaSSXSCSixChordSlider.Update( ss_xs->m_Chord.GetID() );
                                m_FeaSSXSCSixThickChordSlider.Update( ss_xs->m_ThickChord.GetID() );
                                m_FeaSSXSCSixIdealClSlider.Update( ss_xs->m_IdealCl.GetID() );
                                m_FeaSSXSCSixASlider.Update( ss_xs->m_A.GetID() );

                                m_FeaSSXSCSixInvertButton.Update( ss_xs->m_Invert.GetID() );
                                m_FeaSSXSCSixNameOutput.Update( ss_xs->GetAirfoilName() );
                                m_FeaSSXSCSixSeriesChoice.Update( ss_xs->m_Series.GetID() );
                                m_FeaSSXSCSixDegreeCounter.Update( ss_xs->m_FitDegree.GetID() );
                            }
                            else if ( xsc->GetType() == vsp::XS_BICONVEX )
                            {
                                SubSurfXSCDisplayGroup( &m_FeaSSXSCBiconvexGroup );
                                Biconvex* bi_xs = dynamic_cast< Biconvex* >( xsc );
                                assert( bi_xs );

                                m_FeaSSXSCBiconvexChordSlider.Update( bi_xs->m_Chord.GetID() );
                                m_FeaSSXSCBiconvexThickChordSlider.Update( bi_xs->m_ThickChord.GetID() );
                            }
                            else if ( xsc->GetType() == vsp::XS_WEDGE )
                            {
                                SubSurfXSCDisplayGroup( &m_FeaSSXSCWedgeGroup );
                                Wedge* we_xs = dynamic_cast< Wedge* >( xsc );
                                assert( we_xs );

                                m_FeaSSXSCWedgeChordSlider.Update( we_xs->m_Chord.GetID() );
                                m_FeaSSXSCWedgeThickChordSlider.Update( we_xs->m_ThickChord.GetID() );
                                m_FeaSSXSCWedgeThickLocSlider.Update( we_xs->m_ThickLoc.GetID() );
                                m_FeaSSXSCWedgeZCamberSlider.Update( we_xs->m_ZCamber.GetID() );
                                m_FeaSSXSCWedgeSymmThickButton.Update( we_xs->m_SymmThick.GetID() );
                                m_FeaSSXSCWedgeThickLocLowSlider.Update( we_xs->m_ThickLocLow.GetID() );
                                m_FeaSSXSCWedgeFlatUpSlider.Update( we_xs->m_FlatUp.GetID() );
                                m_FeaSSXSCWedgeFlatLowSlider.Update( we_xs->m_FlatLow.GetID() );
                                m_FeaSSXSCWedgeUForeUpSlider.Update( we_xs->m_UForeUp.GetID() );
                                m_FeaSSXSCWedgeUForeLowSlider.Update( we_xs->m_UForeLow.GetID() );
                                m_FeaSSXSCWedgeDuUpSlider.Update( we_xs->m_DuUp.GetID() );
                                m_FeaSSXSCWedgeDuLowSlider.Update( we_xs->m_DuLow.GetID() );
                                m_FeaSSXSCWedgeInvertButton.Update( we_xs->m_Invert.GetID() );

                                if ( we_xs->m_SymmThick() )
                                {
                                    m_FeaSSXSCWedgeThickLocLowSlider.Deactivate();
                                    m_FeaSSXSCWedgeFlatLowSlider.Deactivate();
                                }
                            }
                            else if ( xsc->GetType() == vsp::XS_FILE_FUSE )
                            {
                                SubSurfXSCDisplayGroup( &m_FeaSSXSCFuseFileGroup );
                                FileXSec* file_xs = dynamic_cast< FileXSec* >( xsc );
                                assert( file_xs );
                                m_FeaSSXSCFileHeightSlider.Update( file_xs->m_Height.GetID() );
                                m_FeaSSXSCFileWidthSlider.Update( file_xs->m_Width.GetID() );
                            }
                            else if ( xsc->GetType() == vsp::XS_FILE_AIRFOIL )
                            {
                                SubSurfXSCDisplayGroup( &m_FeaSSXSCAfFileGroup );
                                FileAirfoil* affile_xs = dynamic_cast< FileAirfoil* >( xsc );
                                assert( affile_xs );

                                m_FeaSSXSCAfFileChordSlider.Update( affile_xs->m_Chord.GetID() );
                                m_FeaSSXSCAfFileThickChordSlider.Update( affile_xs->m_ThickChord.GetID() );
                                m_FeaSSXSCAfFileBaseThickChordOutput.Update( affile_xs->m_BaseThickness.GetID() );
                                m_FeaSSXSCAfFileInvertButton.Update( affile_xs->m_Invert.GetID() );
                                m_FeaSSXSCAfFileNameOutput.Update( affile_xs->GetAirfoilName() );
                                m_FeaSSXSCAfFileDegreeCounter.Update( affile_xs->m_FitDegree.GetID() );
                            }
                            else if ( xsc->GetType() == vsp::XS_CST_AIRFOIL )
                            {
                                SubSurfXSCDisplayGroup( &m_FeaSSXSCCSTAirfoilGroup );
                                CSTAirfoil* cst_xs = dynamic_cast< CSTAirfoil* >( xsc );
                                assert( cst_xs );

                                int num_up = cst_xs->m_UpDeg() + 1;
                                int num_low = cst_xs->m_LowDeg() + 1;

                                char str[255];
                                snprintf( str, sizeof( str ),  "%d", cst_xs->m_UpDeg() );
                                m_FeaSSXSCUpDegreeOutput.Update( str );
                                snprintf( str, sizeof( str ),  "%d", cst_xs->m_LowDeg() );
                                m_FeaSSXSCLowDegreeOutput.Update( str );

                                m_FeaSSXSCCSTChordSlider.Update(cst_xs->m_Chord.GetID());
                                m_FeaSSXSCCSTInvertButton.Update( cst_xs->m_Invert.GetID() );
                                m_FeaSSXSCCSTContLERadButton.Update( cst_xs->m_ContLERad.GetID() );
                                m_FeaSSXSCCSTThickChordOutput.Update( cst_xs->m_ThickChord.GetID() );

                                if ( ( m_FeaSSXSCUpCoeffSliderVec.size() != num_up ) || ( m_FeaSSXSCLowCoeffSliderVec.size() != num_low ) )
                                {
                                    RebuildSSCSTGroup( cst_xs );
                                }

                                for ( int i = 0; i < num_up; i++ )
                                {
                                    Parm *p = cst_xs->m_UpCoeffParmVec[i];
                                    if ( p )
                                    {
                                        m_FeaSSXSCUpCoeffSliderVec[i].Update( p->GetID() );
                                    }
                                }

                                for ( int i = 0; i < num_low; i++ )
                                {
                                    Parm *p = cst_xs->m_LowCoeffParmVec[i];
                                    if ( p )
                                    {
                                        m_FeaSSXSCLowCoeffSliderVec[i].Update( p->GetID() );
                                    }
                                }

                                if ( cst_xs->m_ContLERad() && num_low > 0 )
                                {
                                    m_FeaSSXSCLowCoeffSliderVec[0].Deactivate();
                                }
                            }
                            else if ( xsc->GetType() == vsp::XS_VKT_AIRFOIL )
                            {
                                SubSurfXSCDisplayGroup( &m_FeaSSXSCVKTGroup );
                                VKTAirfoil* vkt_xs = dynamic_cast< VKTAirfoil* >( xsc );
                                assert( vkt_xs );

                                m_FeaSSXSCVKTChordSlider.Update( vkt_xs->m_Chord.GetID() );
                                m_FeaSSXSCVKTEpsilonSlider.Update( vkt_xs->m_Epsilon.GetID() );
                                m_FeaSSXSCVKTKappaSlider.Update( vkt_xs->m_Kappa.GetID() );
                                m_FeaSSXSCVKTTauSlider.Update( vkt_xs->m_Tau.GetID() );
                                m_FeaSSXSCVKTInvertButton.Update( vkt_xs->m_Invert.GetID() );
                                m_FeaSSXSCVKTDegreeCounter.Update( vkt_xs->m_FitDegree.GetID() );
                                m_FeaSSXSCVKTThickChordOutput.Update( vkt_xs->m_ThickChord.GetID() );
                            }
                            else if ( xsc->GetType() == vsp::XS_FOUR_DIGIT_MOD )
                            {
                                SubSurfXSCDisplayGroup( &m_FeaSSXSCFourDigitModGroup );
                                FourDigMod* fs_xs = dynamic_cast< FourDigMod* >( xsc );
                                assert( fs_xs );

                                m_FeaSSXSCFourModChordSlider.Update( fs_xs->m_Chord.GetID() );
                                m_FeaSSXSCFourModThickChordSlider.Update( fs_xs->m_ThickChord.GetID() );
                                m_FeaSSXSCFourModCamberSlider.Update( fs_xs->m_Camber.GetID() );
                                m_FeaSSXSCFourModCLiSlider.Update( fs_xs->m_IdealCl.GetID() );
                                m_FeaSSXSCFourModCamberGroup.Update( fs_xs->m_CamberInputFlag.GetID() );
                                if ( fs_xs->m_CamberInputFlag() == vsp::MAX_CAMB )
                                {
                                    m_FeaSSXSCFourModCamberSlider.Activate();
                                    m_FeaSSXSCFourModCLiSlider.Deactivate();
                                }
                                else
                                {
                                    m_FeaSSXSCFourModCamberSlider.Deactivate();
                                    m_FeaSSXSCFourModCLiSlider.Activate();
                                }
                                m_FeaSSXSCFourModCamberLocSlider.Update( fs_xs->m_CamberLoc.GetID() );
                                m_FeaSSXSCFourModInvertButton.Update( fs_xs->m_Invert.GetID() );
                                m_FeaSSXSCFourModNameOutput.Update( fs_xs->GetAirfoilName() );
                                m_FeaSSXSCFourModThicknessLocSlider.Update( fs_xs->m_ThickLoc.GetID() );
                                m_FeaSSXSCFourModLERadIndexSlider.Update( fs_xs->m_LERadIndx.GetID() );
                                m_FeaSSXSCFourModSharpTEButton.Update( fs_xs->m_SharpTE.GetID() );
                                m_FeaSSXSCFourModDegreeCounter.Update( fs_xs->m_FitDegree.GetID() );
                            }
                            else if ( xsc->GetType() == vsp::XS_FIVE_DIGIT )
                            {
                                SubSurfXSCDisplayGroup( &m_FeaSSXSCFiveDigitGroup );
                                FiveDig* fs_xs = dynamic_cast< FiveDig* >( xsc );
                                assert( fs_xs );

                                m_FeaSSXSCFiveChordSlider.Update( fs_xs->m_Chord.GetID() );
                                m_FeaSSXSCFiveThickChordSlider.Update( fs_xs->m_ThickChord.GetID() );
                                m_FeaSSXSCFiveCLiSlider.Update( fs_xs->m_IdealCl.GetID() );
                                m_FeaSSXSCFiveCamberLocSlider.Update( fs_xs->m_CamberLoc.GetID() );
                                m_FeaSSXSCFiveInvertButton.Update( fs_xs->m_Invert.GetID() );
                                m_FeaSSXSCFiveNameOutput.Update( fs_xs->GetAirfoilName() );
                                m_FeaSSXSCFiveSharpTEButton.Update( fs_xs->m_SharpTE.GetID() );
                                m_FeaSSXSCFiveDegreeCounter.Update( fs_xs->m_FitDegree.GetID() );
                            }
                            else if ( xsc->GetType() == vsp::XS_FIVE_DIGIT_MOD )
                            {
                                SubSurfXSCDisplayGroup( &m_FeaSSXSCFiveDigitModGroup );
                                FiveDigMod* fs_xs = dynamic_cast< FiveDigMod* >( xsc );
                                assert( fs_xs );

                                m_FeaSSXSCFiveModChordSlider.Update( fs_xs->m_Chord.GetID() );
                                m_FeaSSXSCFiveModThickChordSlider.Update( fs_xs->m_ThickChord.GetID() );
                                m_FeaSSXSCFiveModCLiSlider.Update( fs_xs->m_IdealCl.GetID() );
                                m_FeaSSXSCFiveModCamberLocSlider.Update( fs_xs->m_CamberLoc.GetID() );
                                m_FeaSSXSCFiveModInvertButton.Update( fs_xs->m_Invert.GetID() );
                                m_FeaSSXSCFiveModNameOutput.Update( fs_xs->GetAirfoilName() );
                                m_FeaSSXSCFiveModThicknessLocSlider.Update( fs_xs->m_ThickLoc.GetID() );
                                m_FeaSSXSCFiveModLERadIndexSlider.Update( fs_xs->m_LERadIndx.GetID() );
                                m_FeaSSXSCFiveModSharpTEButton.Update( fs_xs->m_SharpTE.GetID() );
                                m_FeaSSXSCFiveModDegreeCounter.Update( fs_xs->m_FitDegree.GetID() );
                            }
                            else if ( xsc->GetType() == vsp::XS_ONE_SIX_SERIES )
                            {
                                SubSurfXSCDisplayGroup( &m_FeaSSXSCOneSixSeriesGroup );
                                OneSixSeries* fs_xs = dynamic_cast< OneSixSeries* >( xsc );
                                assert( fs_xs );

                                m_FeaSSXSCOneSixSeriesChordSlider.Update( fs_xs->m_Chord.GetID() );
                                m_FeaSSXSCOneSixSeriesThickChordSlider.Update( fs_xs->m_ThickChord.GetID() );
                                m_FeaSSXSCOneSixSeriesCLiSlider.Update( fs_xs->m_IdealCl.GetID() );
                                m_FeaSSXSCOneSixSeriesInvertButton.Update( fs_xs->m_Invert.GetID() );
                                m_FeaSSXSCOneSixSeriesNameOutput.Update( fs_xs->GetAirfoilName() );
                                m_FeaSSXSCOneSixSeriesSharpTEButton.Update( fs_xs->m_SharpTE.GetID() );
                                m_FeaSSXSCOneSixSeriesDegreeCounter.Update( fs_xs->m_FitDegree.GetID() );
                            }
                            else if ( xsc->GetType() == vsp::XS_EDIT_CURVE )
                            {
                                CurveEditScreen *ceditcreen = dynamic_cast < CurveEditScreen* > ( m_ScreenMgr->GetScreen( vsp::VSP_CURVE_EDIT_SCREEN ) );

                                if ( ceditcreen )
                                {
                                    ceditcreen->SetXSecCurve( xsc );
                                }

                                m_FeaSSXSCEditCEDITGroup.Show();
                                m_FeaSSXSCConvertCEDITGroup.Hide();
                                SubSurfXSCDisplayGroup( nullptr );
                            }

                            if ( xsc->GetType() != vsp::XS_EDIT_CURVE )
                            {
                                m_FeaSSXSCEditCEDITGroup.Hide();
                                m_FeaSSXSCConvertCEDITGroup.Show();
                            }
                        }
//                        m_FeaSSXSCAttrEditor.SetEditorCollID( empty_coll_ids );
//                        m_FeaSSXSCAttrEditor.Update();

                    }
                    else if ( subsurf->GetType() == vsp::SS_CONTROL )
                    {
                        SSControlSurf* sscon = dynamic_cast<SSControlSurf*>( subsurf );
                        assert( sscon );

                        m_FeaSSConTestToggleGroup.Update( sscon->m_TestType.GetID() );
                        m_FeaSSConUESlider.Update( sscon->m_UEnd.GetID() );
                        m_FeaSSConUSSlider.Update( sscon->m_UStart.GetID() );
                        m_FeaSSConSFracSlider.Update( sscon->m_StartLenFrac.GetID() );
                        m_FeaSSConSLenSlider.Update( sscon->m_StartLength.GetID() );
                        m_FeaSSConEFracSlider.Update( sscon->m_EndLenFrac.GetID() );
                        m_FeaSSConELenSlider.Update( sscon->m_EndLength.GetID() );
                        m_FeaSSConSAbsRelToggleGroup.Update( sscon->m_AbsRelFlag.GetID() );
                        m_FeaSSConSEConstButton.Update( sscon->m_ConstFlag.GetID() );
                        m_FeaSSConLEFlagButton.Update( sscon->m_LEFlag.GetID() );
                        m_CreateBeamElementsToggle.Update( sscon->m_CreateBeamElements.GetID() );
                        m_KeepDelShellElementsToggleGroup.Update( sscon->m_KeepDelShellElements.GetID() );
                        m_FeaSSConSAngleButton.Update( sscon->m_StartAngleFlag.GetID() );
                        m_FeaSSConEAngleButton.Update( sscon->m_EndAngleFlag.GetID() );
                        m_FeaSSConSAngleSlider.Update( sscon->m_StartAngle.GetID() );
                        m_FeaSSConEAngleSlider.Update( sscon->m_EndAngle.GetID() );

                        m_FeaSSConTessSlider.Update( sscon->m_Tess.GetID() );

                        if ( sscon->m_TestType() == vsp::NONE )
                        {
                            m_ShellPropertyChoice.Deactivate();
                        }

                        if ( sscon->m_StartAngleFlag() )
                        {
                            m_FeaSSConSAngleSlider.Activate();
                        }
                        else
                        {
                            m_FeaSSConSAngleSlider.Deactivate();
                        }

                        m_FeaSSConSameAngleButton.Update( sscon->m_SameAngleFlag.GetID() );

                        if ( sscon->m_StartAngleFlag() && sscon->m_EndAngleFlag() )
                        {
                            m_FeaSSConSameAngleButton.Activate();
                        }
                        else
                        {
                            m_FeaSSConSameAngleButton.Deactivate();
                        }

                        if ( sscon->m_EndAngleFlag() && ( !sscon->m_SameAngleFlag() || ( !sscon->m_StartAngleFlag() && sscon->m_SameAngleFlag() ) ) )
                        {
                            m_FeaSSConEAngleSlider.Activate();
                        }
                        else
                        {
                            m_FeaSSConEAngleSlider.Deactivate();
                        }

                        m_FeaSSConSFracSlider.Activate();
                        m_FeaSSConSLenSlider.Activate();

                        m_FeaSSConEFracSlider.Activate();
                        m_FeaSSConELenSlider.Activate();

                        if ( sscon->m_AbsRelFlag() == vsp::ABS )
                        {
                            m_FeaSSConSFracSlider.Deactivate();

                            if ( sscon->m_ConstFlag() )
                            {
                                m_FeaSSConELenSlider.Deactivate();
                            }
                        }
                        else
                        {
                            m_FeaSSConSLenSlider.Deactivate();

                            if ( sscon->m_ConstFlag() )
                            {
                                m_FeaSSConEFracSlider.Deactivate();
                            }
                        }

                        m_FeaSSConSurfTypeChoice.Update( sscon->m_SurfType.GetID() );
                        FeaPartDispGroup( &m_FeaSSConGroup );
                    }
                    else if ( subsurf->GetType() == vsp::SS_LINE_ARRAY )
                    {
                        SSLineArray* sslinearray = dynamic_cast<SSLineArray*>( subsurf );
                        assert( sslinearray );

                        m_FeaSSLineArrayConstToggleGroup.Update( sslinearray->m_ConstType.GetID() );
                        m_FeaSSLineArrayPosNegDirToggleGroup.Update( sslinearray->m_PositiveDirectionFlag.GetID() );
                        m_FeaSSLineArraySpacingSlider.Update( sslinearray->m_Spacing.GetID() );
                        m_FeaSSLineArrayStartLocSlider.Update( sslinearray->m_StartLocation.GetID() );
                        m_FeaSSLineArrayEndLocSlider.Update( sslinearray->m_EndLocation.GetID() );
                        m_CreateBeamElementsToggle.Update( sslinearray->m_CreateBeamElements.GetID() );
                        m_KeepDelShellElementsToggleGroup.Update( sslinearray->m_KeepDelShellElements.GetID() );


                        m_ShellPropertyChoice.Deactivate();
                        m_OrientationChoice.Deactivate();
                        m_KeepDelShellElementsToggleGroup.Deactivate();

                        FeaPartDispGroup( &m_FeaSSLineArrayGroup );
                    }
                    else if ( subsurf->GetType() == vsp::SS_FINITE_LINE )
                    {
                        SSFiniteLine* ssfline = dynamic_cast<SSFiniteLine*>( subsurf );
                        assert( ssfline );

                        m_FeaSSFLineUStartSlider.Update( ssfline->m_UStart.GetID() );
                        m_FeaSSFLineUEndSlider.Update( ssfline->m_UEnd.GetID() );
                        m_FeaSSFLineWStartSlider.Update( ssfline->m_WStart.GetID() );
                        m_FeaSSFLineWEndSlider.Update( ssfline->m_WEnd.GetID() );
                        m_CreateBeamElementsToggle.Update( ssfline->m_CreateBeamElements.GetID() );
                        m_KeepDelShellElementsToggleGroup.Update( ssfline->m_KeepDelShellElements.GetID() );

                        m_ShellPropertyChoice.Deactivate();
                        m_OrientationChoice.Deactivate();
                        m_KeepDelShellElementsToggleGroup.Deactivate();

                        FeaPartDispGroup( &m_FeaSSFLineGroup );
                    }
                    else
                    {
                        Hide();
                        return false;
                    }
                }
                else
                {
                    Hide();
                    return false;
                }
            }
            else
            {
                Hide();
                return false;
            }
        }
        else
        {
            Hide();
            return false;
        }
    }

    return true;
}

void FeaPartEditScreen::GuiDeviceCallBack( GuiDevice* device )
{
    assert( m_ScreenMgr );

    Vehicle*  veh = m_ScreenMgr->GetVehiclePtr();

    if ( !veh )
    {
        return;
    }

    if ( device == &m_SliceOrientationChoice )
    {
        StructureMgr.SetFeaSliceOrientIndex( m_SliceOrientationChoice.GetVal() );
    }
    else if ( device == &m_TrimPartChoice )
    {
        m_SelectedTrimPartChoice = m_TrimPartChoice.GetVal();
    }

    if ( StructureMgr.ValidTotalFeaStructInd( StructureMgr.m_CurrStructIndex() ) )
    {
        vector < FeaStructure* > structvec = StructureMgr.GetAllFeaStructs();
        FeaStructure *structure = structvec[ StructureMgr.m_CurrStructIndex() ];

        FeaPart* feaprt = nullptr;
        SubSurface* subsurf = nullptr;

        if ( StructureMgr.GetCurrPartIndex() < structure->NumFeaParts() )
        {
            feaprt = structure->GetFeaPart( StructureMgr.GetCurrPartIndex() );
        }
        else if ( StructureMgr.GetCurrPartIndex() >= structure->NumFeaParts() )
        {
            subsurf = structure->GetFeaSubSurf( StructureMgr.GetCurrPartIndex() - structure->NumFeaParts() );
        }


        if ( device == &m_FeaPartNameInput )
        {
            if ( feaprt )
            {
                feaprt->SetName( m_FeaPartNameInput.GetString() );
            }
            else if ( subsurf )
            {
                subsurf->SetName( m_FeaPartNameInput.GetString() );
            }
        }
        else if ( device == &m_ShellPropertyChoice )
        {
            if ( feaprt )
            {
                feaprt->m_FeaPropertyID = m_FeaPropertyIDVec[ m_ShellPropertyChoice.GetVal() ];
            }
            else if ( subsurf )
            {
                subsurf->m_FeaPropertyID = m_FeaPropertyIDVec[ m_ShellPropertyChoice.GetVal() ];
            }
        }
        else if ( device == &m_CapPropertyChoice )
        {
            if ( feaprt )
            {
                feaprt->m_CapFeaPropertyID = m_FeaPropertyIDVec[ m_CapPropertyChoice.GetVal() ];
            }
            else if ( subsurf )
            {
                subsurf->m_CapFeaPropertyID = m_FeaPropertyIDVec[ m_CapPropertyChoice.GetVal() ];
            }
        }
        else if ( device == &m_RibPerpendicularSparChoice )
        {
            if ( feaprt && feaprt->GetType() == vsp::FEA_RIB )
            {
                FeaRib *rib = dynamic_cast<FeaRib *>( feaprt );
                assert( rib );

                rib->SetPerpendicularEdgeID( m_PerpendicularEdgeVec[ m_RibPerpendicularSparChoice.GetVal() ] );
            }
        }
        else if ( device == &m_RibNormalTypeGroup )
        {
            if ( feaprt && feaprt->GetType() == vsp::FEA_RIB )
            {
                FeaRib *rib = dynamic_cast<FeaRib *>( feaprt );
                assert( rib );

                if ( rib->m_PerpendicularEdgeType.Get() == vsp::SPAR_NORMAL )
                {
                    rib->SetPerpendicularEdgeID( m_PerpendicularEdgeVec[ m_RibPerpendicularSparChoice.GetVal() ] );
                }
            }
        }
        else if ( device == &m_FixPointParentSurfChoice )
        {
            FeaPart *parent_feaprt = StructureMgr.GetFeaPart( m_FixPointParentIDVec[ m_FixPointParentSurfChoice.GetVal() ] );

            if ( feaprt && parent_feaprt )
            {
                if ( feaprt->GetType() == vsp::FEA_FIX_POINT )
                {
                    FeaFixPoint *fixpt = dynamic_cast<FeaFixPoint *>( feaprt );
                    assert( fixpt );

                    fixpt->m_ParentFeaPartID = parent_feaprt->GetID();
                    fixpt->m_SurfDirty = true;
                }
            }
        }
        else if ( device == & m_FixPointOtherGeomPicker )
        {
            if ( feaprt )
            {
                if ( feaprt->GetType() == vsp::FEA_FIX_POINT )
                {
                    FeaFixPoint *fixpt = dynamic_cast<FeaFixPoint *>( feaprt );
                    assert( fixpt );

                    fixpt->m_OtherGeomID = m_FixPointOtherGeomPicker.GetGeomChoice();
                    fixpt->m_SurfDirty = true;
                }
            }
        }
        else if ( device == &m_RibArrayPerpendicularSparChoice )
        {
            if ( feaprt && feaprt->GetType() == vsp::FEA_RIB_ARRAY )
            {
                FeaRibArray *rib_array = dynamic_cast<FeaRibArray *>( feaprt );
                assert( rib_array );

                rib_array->SetPerpendicularEdgeID( m_PerpendicularEdgeVec[ m_RibArrayPerpendicularSparChoice.GetVal() ] );
            }
        }
        else if ( device == &m_RibArrayNormalTypeGroup )
        {
            if ( feaprt && feaprt->GetType() == vsp::FEA_RIB_ARRAY )
            {
                FeaRibArray *rib_array = dynamic_cast<FeaRibArray *>( feaprt );
                assert( rib_array );

                if ( rib_array->m_PerpendicularEdgeType.Get() == vsp::SPAR_NORMAL )
                {
                    rib_array->SetPerpendicularEdgeID( m_PerpendicularEdgeVec[ m_RibArrayPerpendicularSparChoice.GetVal() ] );
                }
            }
        }
        else if ( device == &m_IndividualizeRibArrayButton )
        {
            if ( feaprt )
            {
                if ( feaprt->GetType() == vsp::FEA_RIB_ARRAY )
                {
                    structure->IndividualizeRibArray( StructureMgr.GetCurrPartIndex() );
                }
            }
        }
        else if ( device == &m_IndividualizeSliceArrayButton )
        {
            if ( feaprt )
            {
                if ( feaprt->GetType() == vsp::FEA_SLICE_ARRAY )
                {
                    structure->IndividualizeSliceArray( StructureMgr.GetCurrPartIndex() );
                }
            }
        }
        else if ( device == &m_IndividualizeSSLineArrayButton )
        {
            if ( subsurf )
            {
                if ( subsurf->GetType() == vsp::SS_LINE_ARRAY )
                {
                    structure->IndividualizeSSLineArray( StructureMgr.GetCurrPartIndex() - structure->NumFeaParts() );
                }
            }
        }
        else if ( device == &m_FeaSSXSecTypeChoice )
        {
            SSXSecCurve* ssxsc = dynamic_cast< SSXSecCurve* >( subsurf );
            if ( ssxsc )
            {
                int t = m_FeaSSXSecTypeChoice.GetVal();

                ssxsc->SetXSecCurveType( t );

                if ( t == vsp::XS_EDIT_CURVE )
                {
                    m_ScreenMgr->ShowScreen( vsp::VSP_CURVE_EDIT_SCREEN );
                }
            }
        }
        else if ( device == &m_FeaSSXSCShowXSecButton )
        {
            m_ScreenMgr->ShowScreen( vsp::VSP_XSEC_SCREEN );
        }
        else if ( device == &m_FeaSSXSCConvertCEDITButton )
        {
            SSXSecCurve* ssxsc = dynamic_cast< SSXSecCurve* >( subsurf );
            if ( ssxsc )
            {
                EditCurveXSec* edit_xsec = ssxsc->ConvertToEdit();

                if ( edit_xsec )
                {
                    CurveEditScreen *ceditcreen = dynamic_cast < CurveEditScreen* > ( m_ScreenMgr->GetScreen( vsp::VSP_CURVE_EDIT_SCREEN ) );

                    if ( ceditcreen )
                    {
                        ceditcreen->SetXSecCurve( edit_xsec );
                    }

                    m_ScreenMgr->ShowScreen( vsp::VSP_CURVE_EDIT_SCREEN );
                }
            }
        }
        else if ( device == &m_FeaSSXSCEditCEDITButton )
        {
            m_ScreenMgr->ShowScreen( vsp::VSP_CURVE_EDIT_SCREEN );
        }
        else if ( device == &m_FeaSSXSCReadFuseFileButton  )
        {
            SSXSecCurve* ssxsc = dynamic_cast< SSXSecCurve* >( subsurf );
            if ( ssxsc )
            {
                XSecCurve* xsc = ssxsc->GetXSecCurve();
                if ( xsc )
                {
                    if ( xsc->GetType() == vsp::XS_FILE_FUSE  )
                    {
                        FileXSec* file_xs = dynamic_cast< FileXSec* >( xsc );
                        assert( file_xs );
                        string newfile = m_ScreenMgr->FileChooser( "Fuselage Cross Section", "*.fxs" );

                        file_xs->ReadXsecFile( newfile );
                        file_xs->Update();
                        ssxsc->Update();
                    }
                }
            }
        }
        else if ( device == &m_FeaSSXSCAfReadFileButton   )
        {
            SSXSecCurve* ssxsc = dynamic_cast< SSXSecCurve* >( subsurf );
            if ( ssxsc )
            {
                XSecCurve* xsc = ssxsc->GetXSecCurve();
                if ( xsc )
                {
                    if ( xsc->GetType() == vsp::XS_FILE_AIRFOIL  )
                    {
                        FileAirfoil* affile_xs = dynamic_cast< FileAirfoil* >( xsc );
                        assert( affile_xs );
                        string newfile = m_ScreenMgr->FileChooser( "Airfoil File", "*.{af,dat}"  );

                        affile_xs->ReadFile( newfile );
                        affile_xs->Update();
                        ssxsc->Update();
                    }
                }
            }
        }
        else if ( device == &m_FeaSSXSCUpPromoteButton )
        {
            SSXSecCurve* ssxsc = dynamic_cast< SSXSecCurve* >( subsurf );
            if ( ssxsc )
            {
                XSecCurve* xsc = ssxsc->GetXSecCurve();
                if ( xsc )
                {
                    if ( xsc->GetType() == vsp::XS_CST_AIRFOIL )
                    {
                        CSTAirfoil* cst_xs = dynamic_cast<CSTAirfoil*>( xsc );
                        assert( cst_xs );

                        cst_xs->PromoteUpper();
                        cst_xs->Update();
                        ssxsc->Update();
                    }
                }
            }
        }
        else if ( device == &m_FeaSSXSCLowPromoteButton )
        {
            SSXSecCurve* ssxsc = dynamic_cast< SSXSecCurve* >( subsurf );
            if ( ssxsc )
            {
                XSecCurve* xsc = ssxsc->GetXSecCurve();
                if ( xsc )
                {
                    if ( xsc->GetType() == vsp::XS_CST_AIRFOIL )
                    {
                        CSTAirfoil* cst_xs = dynamic_cast<CSTAirfoil*>( xsc );
                        assert( cst_xs );

                        cst_xs->PromoteLower();
                        cst_xs->Update();
                        ssxsc->Update();
                    }
                }
            }
        }
        else if ( device == &m_FeaSSXSCUpDemoteButton )
        {
            SSXSecCurve* ssxsc = dynamic_cast< SSXSecCurve* >( subsurf );
            if ( ssxsc )
            {
                XSecCurve* xsc = ssxsc->GetXSecCurve();
                if ( xsc )
                {
                    if ( xsc->GetType() == vsp::XS_CST_AIRFOIL )
                    {
                        CSTAirfoil* cst_xs = dynamic_cast<CSTAirfoil*>( xsc );
                        assert( cst_xs );

                        cst_xs->DemoteUpper();
                        cst_xs->Update();
                        ssxsc->Update();
                    }
                }
            }
        }
        else if ( device == &m_FeaSSXSCLowDemoteButton )
        {
            SSXSecCurve* ssxsc = dynamic_cast< SSXSecCurve* >( subsurf );
            if ( ssxsc )
            {
                XSecCurve* xsc = ssxsc->GetXSecCurve();
                if ( xsc )
                {
                    if ( xsc->GetType() == vsp::XS_CST_AIRFOIL )
                    {
                        CSTAirfoil* cst_xs = dynamic_cast<CSTAirfoil*>( xsc );
                        assert( cst_xs );

                        cst_xs->DemoteLower();
                        cst_xs->Update();
                        ssxsc->Update();
                    }
                }
            }
        }
        else if ( ( device == &m_FeaSSXSCFourFitCSTButton ) ||
                ( device == &m_FeaSSXSCSixFitCSTButton ) ||
                ( device == &m_FeaSSXSCAfFileFitCSTButton ) ||
                ( device == &m_FeaSSXSCVKTFitCSTButton ) ||
                ( device == &m_FeaSSXSCFourModFitCSTButton ) ||
                ( device == &m_FeaSSXSCFiveFitCSTButton ) ||
                ( device == &m_FeaSSXSCFiveModFitCSTButton ) ||
                ( device == &m_FeaSSXSCOneSixSeriesFitCSTButton ) )
        {
            SSXSecCurve* ssxsc = dynamic_cast< SSXSecCurve* >( subsurf );
            if ( ssxsc )
            {
                XSecCurve* xsc = ssxsc->GetXSecCurve();
                if ( xsc )
                {
                    Airfoil* af_xs = dynamic_cast<Airfoil*>( xsc );

                    if ( af_xs )
                    {
                        VspCurve c = af_xs->GetOrigCurve();
                        int deg = af_xs->m_FitDegree();

                        //bor_ptr->SetActiveAirfoilType( XS_CST_AIRFOIL );
                        ssxsc->SetXSecCurveType( vsp::XS_CST_AIRFOIL );

                        XSecCurve* newxsc = ssxsc->GetXSecCurve();
                        if ( newxsc )
                        {
                            if ( newxsc->GetType() == vsp::XS_CST_AIRFOIL )
                            {
                                CSTAirfoil* cst_xs = dynamic_cast<CSTAirfoil*>( newxsc );
                                assert( cst_xs );

                                cst_xs->FitCurve( c, deg );

                                cst_xs->Update();
                                ssxsc->Update();
                            }
                        }
                    }
                }
            }
        }
        else if ( device == &m_AddTrimPartButton )
        {
            if ( feaprt && feaprt->GetType() == vsp::FEA_TRIM )
            {
                FeaPartTrim *trim = dynamic_cast<FeaPartTrim *>( feaprt );
                assert( trim );

                if ( trim )
                {
                    trim->AddTrimPart( m_TrimPartChoiceIDVec[ m_SelectedTrimPartChoice ] );
                    m_ActiveTrimPartIndex = trim->m_TrimFeaPartIDVec.size() - 1;
                    trim->Update();
                }
            }
        }
        else if ( device == &m_DeleteTrimPartButton )
        {
            if ( feaprt && feaprt->GetType() == vsp::FEA_TRIM )
            {
                FeaPartTrim *trim = dynamic_cast<FeaPartTrim *>( feaprt );
                assert( trim );

                if ( trim )
                {
                    trim->DeleteTrimPart( m_ActiveTrimPartIndex );
                    m_ActiveTrimPartIndex = -1;
                    trim->Update();
                }
            }
        }


        FeaPolySpar* polyspar = dynamic_cast<FeaPolySpar*>( feaprt );
        if ( polyspar )
        {
            if ( device == &m_AddPolySparPoint )
            {
                polyspar->AddPt();
                m_PolySparPointBrowserSelect = polyspar->GetNumPt() - 1;
                polyspar->m_SurfDirty = true;
            }
            else if ( device == &m_InsertPolySparPoint )
            {
                if ( m_PolySparPointBrowserSelect < 0 )
                {
                    m_PolySparPointBrowserSelect = 0;
                }
                polyspar->InsertPt( m_PolySparPointBrowserSelect );
                polyspar->m_SurfDirty = true;
            }
            else if ( device == &m_DelPolySparPoint )
            {
                polyspar->DelPt( m_PolySparPointBrowserSelect );
                polyspar->m_SurfDirty = true;
            }
            else if ( device == &m_DelAllPolySparPoints )
            {
                polyspar->DelAllPt();
                polyspar->m_SurfDirty = true;
                m_PolySparPointBrowserSelect = -1;
            }
            else if ( device == &m_PolySparPtNameInput )
            {
                PolySparPoint* spt = polyspar->GetPt( m_PolySparPointBrowserSelect );

                if ( spt )
                {
                    spt->SetName( m_PolySparPtNameInput.GetString() );
                }
            }
            else if ( device == &m_MovePntTopButton )
            {
                int npt = polyspar->GetNumPt();
                if ( m_PolySparPointBrowserSelect >= 0 && m_PolySparPointBrowserSelect < npt )
                {
                    m_PolySparPointBrowserSelect = polyspar->MovePt( m_PolySparPointBrowserSelect, vsp::REORDER_MOVE_TOP );
                    polyspar->m_SurfDirty = true;
                }
            }
            else if ( device == &m_MovePntUpButton )
            {
                int npt = polyspar->GetNumPt();
                if ( m_PolySparPointBrowserSelect >= 0 && m_PolySparPointBrowserSelect < npt )
                {
                    m_PolySparPointBrowserSelect = polyspar->MovePt( m_PolySparPointBrowserSelect, vsp::REORDER_MOVE_UP );
                    polyspar->m_SurfDirty = true;
                }
            }
            else if ( device == &m_MovePntDownButton )
            {
                int npt = polyspar->GetNumPt();
                if ( m_PolySparPointBrowserSelect >= 0 && m_PolySparPointBrowserSelect < npt )
                {
                    m_PolySparPointBrowserSelect = polyspar->MovePt( m_PolySparPointBrowserSelect, vsp::REORDER_MOVE_DOWN );
                    polyspar->m_SurfDirty = true;
                }
            }
            else if ( device == &m_MovePntBotButton )
            {
                int npt = polyspar->GetNumPt();
                if ( m_PolySparPointBrowserSelect >= 0 && m_PolySparPointBrowserSelect < npt )
                {
                    m_PolySparPointBrowserSelect = polyspar->MovePt( m_PolySparPointBrowserSelect, vsp::REORDER_MOVE_BOTTOM );
                    polyspar->m_SurfDirty = true;
                }
            }
        }
    }

    m_ScreenMgr->SetUpdateFlag( true );
}

void FeaPartEditScreen::CloseCallBack( Fl_Widget *w )
{
    assert( m_ScreenMgr );

    Hide();
}

void FeaPartEditScreen::CallBack( Fl_Widget* w )
{
    assert( m_ScreenMgr );

    if ( w == m_TrimPartBrowser )
    {
        m_ActiveTrimPartIndex = m_TrimPartBrowser->value() - 2;

        vector < FeaStructure* > structVec = StructureMgr.GetAllFeaStructs();
        if ( StructureMgr.GetCurrPartIndex() < structVec[StructureMgr.m_CurrStructIndex()]->NumFeaParts() )
        {
            FeaPart* feaprt = structVec[StructureMgr.m_CurrStructIndex()]->GetFeaPart( StructureMgr.GetCurrPartIndex() );

            if ( feaprt && feaprt->GetType() == vsp::FEA_TRIM )
            {
                FeaPartTrim* trim = dynamic_cast<FeaPartTrim*>( feaprt );
                assert( trim );

                if ( trim )
                {
                    if ( m_ActiveTrimPartIndex >= 0 && m_ActiveTrimPartIndex < trim->m_TrimFeaPartIDVec.size() )
                    {
                        int selid = vector_find_val( m_TrimPartChoiceIDVec, trim->m_TrimFeaPartIDVec[ m_ActiveTrimPartIndex ] );

                        if ( selid >= 0 && selid < m_TrimPartChoiceIDVec.size() )
                        {
                            m_SelectedTrimPartChoice = selid;
                        }
                    }
                }
            }
        }
    }
    else if ( w == m_PolySparPointBrowser )
    {
        m_PolySparPointBrowserSelect = m_PolySparPointBrowser->value() - 2;
    }

    m_ScreenMgr->SetUpdateFlag( true );
}

void FeaPartEditScreen::UpdateFeaPropertyChoice()
{
    //==== Property Choice ====//
    m_ShellPropertyChoice.ClearItems();
    m_CapPropertyChoice.ClearItems();
    m_FeaPropertyIDVec.clear();

    Vehicle*  veh = m_ScreenMgr->GetVehiclePtr();

    if ( veh )
    {
        vector< FeaProperty* > property_vec = StructureMgr.GetFeaPropertyVec();

        for ( int i = 0; i < property_vec.size(); ++i )
        {
            m_FeaPropertyIDVec.push_back( property_vec[i]->GetID() );
            if ( property_vec[i]->m_FeaPropertyType() == vsp::FEA_SHELL )
            {
                m_ShellPropertyChoice.AddItem( string( property_vec[i]->GetName() ), i );
            }
            else // vsp::FEA_BEAM
            {
                m_CapPropertyChoice.AddItem( string( property_vec[i]->GetName() ), i );
            }
        }

        m_ShellPropertyChoice.UpdateItems();
        m_CapPropertyChoice.UpdateItems();

        if ( StructureMgr.ValidTotalFeaStructInd( StructureMgr.m_CurrStructIndex() ) )
        {
            vector < FeaStructure* > structvec = StructureMgr.GetAllFeaStructs();

            if ( StructureMgr.GetCurrPartIndex() < structvec[StructureMgr.m_CurrStructIndex()]->NumFeaParts() )
            {
                FeaPart* feaprt = structvec[StructureMgr.m_CurrStructIndex()]->GetFeaPart( StructureMgr.GetCurrPartIndex() );

                if ( feaprt )
                {
                    // Update all FeaPart Property Choices ( Only Selected Part Visible )
                    m_ShellPropertyChoice.SetVal( vector_find_val( m_FeaPropertyIDVec, feaprt->m_FeaPropertyID ) );
                    m_OrientationChoice.Update( feaprt->m_OrientationType.GetID() );
                    m_CapPropertyChoice.SetVal( vector_find_val( m_FeaPropertyIDVec, feaprt->m_CapFeaPropertyID ) );

                }
            }
            else if ( StructureMgr.GetCurrPartIndex() >= structvec[StructureMgr.m_CurrStructIndex()]->NumFeaParts() )
            {
                SubSurface* subsurf = structvec[StructureMgr.m_CurrStructIndex()]->GetFeaSubSurf( StructureMgr.GetCurrPartIndex() - structvec[StructureMgr.m_CurrStructIndex()]->NumFeaParts() );

                if ( subsurf )
                {
                    // Update all FeaSubSurface Property Choices ( Only Selected Part Visible )
                    m_ShellPropertyChoice.SetVal( vector_find_val( m_FeaPropertyIDVec, subsurf->m_FeaPropertyID ) );
                    m_OrientationChoice.Update( subsurf->m_FeaOrientationType.GetID() );
                    m_CapPropertyChoice.SetVal( vector_find_val( m_FeaPropertyIDVec, subsurf->m_CapFeaPropertyID ) );

                }
            }
        }
    }
}

void FeaPartEditScreen::UpdateFixPointParentChoice()
{
    //==== FixPoint Parent Surf Choice ====//
    m_FixPointParentSurfChoice.ClearItems();
    m_FixPointParentIDVec.clear();

    if ( StructureMgr.ValidTotalFeaStructInd( StructureMgr.m_CurrStructIndex() ) )
    {
        vector< FeaStructure* > structVec = StructureMgr.GetAllFeaStructs();
        vector<FeaPart*> feaprt_vec = structVec[StructureMgr.m_CurrStructIndex()]->GetFeaPartVec();

        for ( size_t i = 0; i < feaprt_vec.size(); i++ )
        {
            if ( !structVec[StructureMgr.m_CurrStructIndex()]->FeaPartIsFixPoint( i ) )
            {
                m_FixPointParentSurfChoice.AddItem( string( feaprt_vec[i]->GetName() ) );
                m_FixPointParentIDVec.push_back( feaprt_vec[i]->GetID() );
            }
        }

        m_FixPointParentSurfChoice.UpdateItems();

        if ( StructureMgr.GetCurrPartIndex() < structVec[StructureMgr.m_CurrStructIndex()]->NumFeaParts() )
        {
            FeaPart* feaprt = structVec[StructureMgr.m_CurrStructIndex()]->GetFeaPart( StructureMgr.GetCurrPartIndex() );

            if ( feaprt )
            {
                if ( feaprt->GetType() == vsp::FEA_FIX_POINT )
                {
                    FeaFixPoint* fixpt = dynamic_cast<FeaFixPoint*>( feaprt );
                    assert( fixpt );

                    FeaPart* parent_feaprt = StructureMgr.GetFeaPart( fixpt->m_ParentFeaPartID );

                    if ( parent_feaprt )
                    {
                        int parent_index = structVec[StructureMgr.m_CurrStructIndex()]->GetFeaPartIndex( parent_feaprt );
                        int num_fix_pnts_prior = 0;

                        for ( size_t i = 0; i < parent_index; i++ )
                        {
                            if ( structVec[StructureMgr.m_CurrStructIndex()]->FeaPartIsFixPoint( i ) )
                            {
                                num_fix_pnts_prior++;
                            }
                        }

                        if ( structVec[StructureMgr.m_CurrStructIndex()]->ValidFeaPartInd( parent_index - num_fix_pnts_prior ) )
                        {
                            m_FixPointParentSurfChoice.SetVal( parent_index - num_fix_pnts_prior );
                        }
                    }
                    else
                    {
                        // Set skin as parent surface if undefined
                        if ( structVec[StructureMgr.m_CurrStructIndex()]->GetFeaSkin() )
                        {
                            fixpt->m_ParentFeaPartID = structVec[StructureMgr.m_CurrStructIndex()]->GetFeaSkin()->GetID();
                            fixpt->m_SurfDirty = true;
                        }
                    }
                }
            }
        }
    }
}

void FeaPartEditScreen::UpdateTrimPartChoice()
{
    //==== FixPoint Parent Surf Choice ====//
    m_TrimPartChoice.ClearItems();
    m_TrimPartChoiceIDVec.clear();

    int istruct = StructureMgr.m_CurrStructIndex();

    if ( StructureMgr.ValidTotalFeaStructInd( istruct ) )
    {
        vector < FeaStructure * > structVec = StructureMgr.GetAllFeaStructs();
        FeaStructure * fea_struct = structVec[ istruct ];
        vector < FeaPart * > feaprt_vec = fea_struct->GetFeaPartVec(); // Does not include subsurfaces

        // Loop over all parts in current structure
        for ( size_t i = 0; i < feaprt_vec.size(); i++ )
        {
            if ( !fea_struct->FeaPartIsFixPoint( i ) &&
                 !fea_struct->FeaPartIsArray( i ) &&
                 !fea_struct->FeaPartIsTrim( i ) &&
                 !fea_struct->FeaPartIsSkin( i ) ) // Possibly could be done differently
            {
                m_TrimPartChoice.AddItem( string( feaprt_vec[ i ]->GetName() ) );
                m_TrimPartChoiceIDVec.push_back( feaprt_vec[ i ]->GetID() );
            }
        }

        m_TrimPartChoice.UpdateItems();
        m_TrimPartChoice.SetVal( m_SelectedTrimPartChoice );


        int h_pos = m_TrimPartBrowser->hposition();
        int v_pos = m_TrimPartBrowser->vposition();

        m_TrimPartBrowser->clear();
        m_TrimPartBrowser->column_char( ':' );
        char str[256];
        snprintf( str, sizeof( str ),  "@b@.PART:@b@.FLIPDIR" );
        m_TrimPartBrowser->add( str );

        int ipart = StructureMgr.GetCurrPartIndex();
        if (  ipart < fea_struct->NumFeaParts() )
        {
            FeaPart * feaprt = fea_struct->GetFeaPart( ipart ) ;

            if ( feaprt )
            {
                FeaPartTrim * trim = dynamic_cast < FeaPartTrim* > ( feaprt );

                if ( trim )
                {
                    vector < BoolParm* > flagvec = trim->m_FlipFlagVec;
                    vector < string > partids = trim->m_TrimFeaPartIDVec;

                    for ( int itrim = 0; itrim < partids.size(); itrim++ )
                    {
                        FeaPart* trim_feaprt = StructureMgr.GetFeaPart( partids[itrim] );

                        if ( trim_feaprt )
                        {
                            string flagstr( " " );
                            if ( flagvec[ itrim ]->Get())
                            {
                                flagstr = string( "X" );
                            }

                            snprintf( str, sizeof( str ),  "%s:%s", trim_feaprt->GetName().c_str(), flagstr.c_str());
                            m_TrimPartBrowser->add( str );
                        }
                        else
                        {
                            snprintf( str, sizeof( str ),  "Unset: " );
                            m_TrimPartBrowser->add( str );
                        }
                    }

                    if ( m_ActiveTrimPartIndex >= 0 && m_ActiveTrimPartIndex < partids.size() )
                    {
                        m_TrimPartBrowser->select( m_ActiveTrimPartIndex + 2 );
                    }

                    if ( m_ActiveTrimPartIndex >= 0 && m_ActiveTrimPartIndex < trim->m_FlipFlagVec.size() )
                    {
                        m_FlipTrimDirButton.Update( trim->m_FlipFlagVec[ m_ActiveTrimPartIndex ]->GetID());
                    }
                }
            }
        }

        m_TrimPartBrowser->hposition( h_pos );
        m_TrimPartBrowser->vposition( v_pos );


    }
}

void FeaPartEditScreen::UpdatePerpendicularRibChoice()
{
    //==== Perpendicular Rib Choice ====//
    m_RibPerpendicularSparChoice.ClearItems();
    m_RibArrayPerpendicularSparChoice.ClearItems();
    m_PerpendicularEdgeVec.clear();

    Vehicle*  veh = m_ScreenMgr->GetVehiclePtr();

    if ( veh )
    {
        if ( StructureMgr.ValidTotalFeaStructInd( StructureMgr.m_CurrStructIndex() ) )
        {
            FeaStructure* curr_struct = StructureMgr.GetAllFeaStructs()[StructureMgr.m_CurrStructIndex()];

            if ( curr_struct )
            {
                vector < FeaPart* > part_vec = curr_struct->GetFeaPartVec();

                for ( size_t i = 1; i < part_vec.size(); i++ )
                {
                    if ( part_vec[i]->GetType() == vsp::FEA_SPAR )
                    {
                        m_RibPerpendicularSparChoice.AddItem( part_vec[i]->GetName() );
                        m_RibArrayPerpendicularSparChoice.AddItem( part_vec[i]->GetName() );
                        m_PerpendicularEdgeVec.push_back( part_vec[i]->GetID() );
                    }
                }

                m_RibPerpendicularSparChoice.UpdateItems();
                m_RibArrayPerpendicularSparChoice.UpdateItems();

                if ( StructureMgr.GetCurrPartIndex() < curr_struct->NumFeaParts() )
                {
                    FeaPart* feaprt = curr_struct->GetFeaPart( StructureMgr.GetCurrPartIndex() );

                    if ( feaprt )
                    {
                        if ( feaprt->GetType() == vsp::FEA_RIB )
                        {
                            FeaRib* rib = dynamic_cast<FeaRib*>( feaprt );
                            assert( rib );

                            for ( size_t k = 0; k < m_PerpendicularEdgeVec.size(); k++ )
                            {
                                if ( rib->GetPerpendicularEdgeID() == m_PerpendicularEdgeVec[k] )
                                {
                                    m_RibPerpendicularSparChoice.SetVal( k );
                                    break;
                                }
                            }
                        }
                        else if ( feaprt->GetType() == vsp::FEA_RIB_ARRAY )
                        {
                            FeaRibArray* rib_array = dynamic_cast<FeaRibArray*>( feaprt );
                            assert( rib_array );

                            for ( size_t k = 0; k < m_PerpendicularEdgeVec.size(); k++ )
                            {
                                if ( rib_array->GetPerpendicularEdgeID() == m_PerpendicularEdgeVec[k] )
                                {
                                    m_RibArrayPerpendicularSparChoice.SetVal( k );
                                    break;
                                }
                            }
                        }
                    }
                }
            }
        }
    }
}

void FeaPartEditScreen::FeaPartDispGroup( GroupLayout* group )
{
    if ( m_CurFeaPartDispGroup == group && group )
    {
        return;
    }

    m_SkinEditLayout.Hide();
    m_SliceEditLayout.Hide();
    m_SliceEditLayout.Hide();
    m_RibEditLayout.Hide();
    m_SparEditLayout.Hide();
    m_PolySparEditLayout.Hide();
    m_FixPointEditLayout.Hide();
    m_DomeEditLayout.Hide();
    m_RibArrayEditLayout.Hide();
    m_SliceArrayEditLayout.Hide();
    m_TrimEditLayout.Hide();

    m_FeaSSLineGroup.Hide();
    m_FeaSSRecGroup.Hide();
    m_FeaSSEllGroup.Hide();
    m_FeaSSConGroup.Hide();
    m_FeaSSLineArrayGroup.Hide();
    m_FeaSSFLineGroup.Hide();
    m_FeaSSXSCGroup.Hide();

    m_CurFeaPartDispGroup = group;

    if ( group )
    {
        group->Show();
    }
}

void FeaPartEditScreen::SubSurfXSCDisplayGroup( GroupLayout* group )
{
    if ( m_FeaSSXSCCurrDisplayGroup == group )
    {
        return;
    }

    m_FeaSSXSCSuperGroup.Hide();
    m_FeaSSXSCCircleGroup.Hide();
    m_FeaSSXSCEllipseGroup.Hide();
    m_FeaSSXSCAC25773Group.Hide();
    m_FeaSSXSCRoundedRectGroup.Hide();
    m_FeaSSXSCGenGroup.Hide();
    m_FeaSSXSCFourSeriesGroup.Hide();
    m_FeaSSXSCSixSeriesGroup.Hide();
    m_FeaSSXSCBiconvexGroup.Hide();
    m_FeaSSXSCWedgeGroup.Hide();
    m_FeaSSXSCFuseFileGroup.Hide();
    m_FeaSSXSCAfFileGroup.Hide();
    m_FeaSSXSCCSTAirfoilGroup.Hide();
    m_FeaSSXSCVKTGroup.Hide();
    m_FeaSSXSCFourDigitModGroup.Hide();
    m_FeaSSXSCFiveDigitGroup.Hide();
    m_FeaSSXSCFiveDigitModGroup.Hide();
    m_FeaSSXSCOneSixSeriesGroup.Hide();

    m_FeaSSXSCCurrDisplayGroup = group;

    if ( group )
    {
        group->Show();
    }
}

void FeaPartEditScreen::RebuildSSCSTGroup( CSTAirfoil* cst_xs)
{
    if ( !cst_xs )
    {
        return;
    }

    if ( !m_FeaSSXSCCSTUpCoeffScroll || !m_FeaSSXSCCSTLowCoeffScroll )
    {
        return;
    }

    m_FeaSSXSCCSTUpCoeffScroll->clear();
    m_FeaSSXSCCSTUpCoeffLayout.SetGroup( m_FeaSSXSCCSTUpCoeffScroll );
    m_FeaSSXSCCSTUpCoeffLayout.InitWidthHeightVals();

    m_FeaSSXSCUpCoeffSliderVec.clear();

    unsigned int num_up = cst_xs->m_UpDeg() + 1;

    m_FeaSSXSCUpCoeffSliderVec.resize( num_up );

    for ( int i = 0; i < num_up; i++ )
    {
        m_FeaSSXSCCSTUpCoeffLayout.AddSlider( m_FeaSSXSCUpCoeffSliderVec[i], "AUTO_UPDATE", 2, "%9.5f" );
    }




    m_FeaSSXSCCSTLowCoeffScroll->clear();
    m_FeaSSXSCCSTLowCoeffLayout.SetGroup( m_FeaSSXSCCSTLowCoeffScroll );
    m_FeaSSXSCCSTLowCoeffLayout.InitWidthHeightVals();

    m_FeaSSXSCLowCoeffSliderVec.clear();

    unsigned int num_low = cst_xs->m_LowDeg() + 1;

    m_FeaSSXSCLowCoeffSliderVec.resize( num_low );


    for ( int i = 0; i < num_low; i++ )
    {
        m_FeaSSXSCCSTLowCoeffLayout.AddSlider( m_FeaSSXSCLowCoeffSliderVec[i], "AUTO_UPDATE", 2, "%9.5f" );
    }
}

void FeaPartEditScreen::UpdateUnitLabels()
{
    Vehicle* veh = m_ScreenMgr->GetVehiclePtr();

    if ( veh )
    {
        string mass_unit, dist_unit, model_dist_unit;
        string squared( 1, (char) 178 );

        switch ( veh->m_StructUnit() )
        {
            case vsp::SI_UNIT:
                mass_unit = "kg";
                dist_unit = "m";
                break;

            case vsp::CGS_UNIT:
                mass_unit = "g";
                dist_unit = "cm";
                break;

            case vsp::MPA_UNIT:
                mass_unit = "tonne"; // or Mg/
                dist_unit = "mm";
                break;

            case vsp::BFT_UNIT:
                mass_unit = "slug";
                dist_unit = "ft";
                break;

            case vsp::BIN_UNIT:
                mass_unit = "lbf sec" + squared + " / in";
                dist_unit = "in";
                break;
        }

        switch ( veh->m_StructModelUnit() )
        {
            case vsp::LEN_MM:
                model_dist_unit = "mm";
                break;

            case vsp::LEN_CM:
                model_dist_unit = "cm";
                break;

            case vsp::LEN_M:
                model_dist_unit = "m";
                break;

            case vsp::LEN_IN:
                model_dist_unit = "in";
                break;

            case vsp::LEN_FT:
                model_dist_unit = "ft";
                break;

            case vsp::LEN_YD:
                model_dist_unit = "yd";
                break;

            case vsp::LEN_UNITLESS:
                model_dist_unit = "-";
                break;
        }

        m_FixPointMassUnit_FEM.GetFlButton()->copy_label( mass_unit.c_str() );

        m_DomeARadUnit.GetFlButton()->copy_label( model_dist_unit.c_str() );
        m_DomeBRadUnit.GetFlButton()->copy_label( model_dist_unit.c_str() );
        m_DomeCRadUnit.GetFlButton()->copy_label( model_dist_unit.c_str() );
        m_DomeXUnit.GetFlButton()->copy_label( model_dist_unit.c_str() );
        m_DomeYUnit.GetFlButton()->copy_label( model_dist_unit.c_str() );
        m_DomeZUnit.GetFlButton()->copy_label( model_dist_unit.c_str() );






        if ( StructureMgr.ValidTotalFeaStructInd( StructureMgr.m_CurrStructIndex() ) )
        {
            vector< FeaStructure* > structVec = StructureMgr.GetAllFeaStructs();

            if ( StructureMgr.GetCurrPartIndex() < structVec[StructureMgr.m_CurrStructIndex()]->NumFeaParts() )
            {
                FeaPart* feaprt = structVec[StructureMgr.m_CurrStructIndex()]->GetFeaPart( StructureMgr.GetCurrPartIndex() );
                if ( feaprt )
                {
                    if ( feaprt->GetType() == vsp::FEA_SLICE ||
                         feaprt->GetType() == vsp::FEA_RIB ||
                         feaprt->GetType() == vsp::FEA_SPAR ||
                         feaprt->GetType() == vsp::FEA_POLY_SPAR ||
                         feaprt->GetType() == vsp::FEA_RIB_ARRAY ||
                         feaprt->GetType() == vsp::FEA_SLICE_ARRAY ) // TODO: Switch to different check
                    {
                        m_SlicePosUnit.GetFlButton()->copy_label( model_dist_unit.c_str() );
                        m_SparPosUnit.GetFlButton()->copy_label( model_dist_unit.c_str() );
                        m_RibPosUnit.GetFlButton()->copy_label( model_dist_unit.c_str() );
                        m_RibArrayStartLocUnit.GetFlButton()->copy_label( model_dist_unit.c_str() );
                        m_RibArrayEndLocUnit.GetFlButton()->copy_label( model_dist_unit.c_str() );
                        m_RibArrayPosUnit.GetFlButton()->copy_label( model_dist_unit.c_str() );
                        m_SliceArrayStartLocUnit.GetFlButton()->copy_label( model_dist_unit.c_str() );
                        m_SliceArrayEndLocUnit.GetFlButton()->copy_label( model_dist_unit.c_str() );
                        m_SliceArrayPosUnit.GetFlButton()->copy_label( model_dist_unit.c_str() );

                        if ( feaprt->m_AbsRelParmFlag() == vsp::ABS )
                        {
                            m_SlicePosUnit.Activate();
                            m_SparPosUnit.Activate();
                            m_RibPosUnit.Activate();
                            m_RibArrayStartLocUnit.Activate();
                            m_RibArrayEndLocUnit.Activate();
                            m_RibArrayPosUnit.Activate();
                            m_SliceArrayStartLocUnit.Activate();
                            m_SliceArrayEndLocUnit.Activate();
                            m_SliceArrayPosUnit.Activate();
                        }
                        else if( feaprt->m_AbsRelParmFlag() == vsp::REL )
                        {
                            m_SlicePosUnit.Deactivate();
                            m_SparPosUnit.Deactivate();
                            m_RibPosUnit.Deactivate();
                            m_RibArrayStartLocUnit.Deactivate();
                            m_RibArrayEndLocUnit.Deactivate();
                            m_RibArrayPosUnit.Deactivate();
                            m_SliceArrayStartLocUnit.Deactivate();
                            m_SliceArrayEndLocUnit.Deactivate();
                            m_SliceArrayPosUnit.Deactivate();
                        }
                    }
                }
            }
        }
    }
}

void FeaPartEditScreen::UpdatePolySparPointBrowser( FeaPolySpar* polyspar )
{
    if ( polyspar )
    {
        char str[255];

        int input_h_pos = m_PolySparPointBrowser->hposition();
        int input_v_pos = m_PolySparPointBrowser->vposition();

        m_PolySparPointBrowser->clear();

        m_PolySparPointBrowser->column_char( ':' );

        snprintf( str, sizeof( str ),  "@b@.PT_NAME:@b@.PARM:@b@.GROUP:@b@.CONTAINER" );
        m_PolySparPointBrowser->add( str );
        m_PolySparPointBrowser->addCopyText( "header" );

        vector < PolySparPoint* > pt_vec = polyspar->GetAllPt();

        for ( int i = 0 ; i < (int)pt_vec.size() ; i++ )
        {
            snprintf( str, sizeof( str ),  "%s:::\n", pt_vec[i]->GetName().c_str() );
            m_PolySparPointBrowser->add( str );
        }

        if ( m_PolySparPointBrowserSelect >= 0 && m_PolySparPointBrowserSelect < (int)pt_vec.size() )
        {
            m_PolySparPointBrowser->select( m_PolySparPointBrowserSelect + 2 );
        }

        m_PolySparPointBrowser->hposition( input_h_pos );
        m_PolySparPointBrowser->vposition( input_v_pos );
    }
}
