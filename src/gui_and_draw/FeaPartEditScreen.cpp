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

FeaPartEditScreen::FeaPartEditScreen( ScreenMgr* mgr ) : BasicScreen( mgr, 340, 475, "FEA Part Edit" )
{
    m_FLTK_Window->callback( staticCloseCB, this );

    //=== FEA Parts ===//
    m_GenLayout.SetGroupAndScreen( m_FLTK_Window, this );

    m_GenLayout.AddY( m_GenLayout.GetStdHeight() + m_GenLayout.GetGapHeight() );

    m_GenLayout.AddInput( m_FeaPartNameInput, "Part Name" );

    m_GenLayout.AddYGap();

    m_CurFeaPartDispGroup = NULL;

    // Individual FeaPart Parameters:
    int start_y = m_GenLayout.GetY();

    //==== FeaSkin ====//
    m_GenLayout.AddSubGroupLayout( m_SkinEditLayout, m_GenLayout.GetRemainX(), m_GenLayout.GetRemainY() );
    m_SkinEditLayout.SetY( start_y );

    m_SkinEditLayout.AddDividerBox( "Skin" );

    m_SkinEditLayout.AddButton( m_RemoveSkinToggle, "Remove Skin" );

    m_SkinEditLayout.AddYGap();

    m_SkinEditLayout.AddDividerBox( "Elements" );

    m_SkinEditLayout.AddChoice( m_SkinPropertyChoice, "Property" );

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
    m_SliceEditLayout.AddButton( m_SlicePosAbsToggle, "Absolute" );

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

    m_SliceEditLayout.AddYGap();

    m_SliceEditLayout.AddDividerBox( "Elements" );

    m_SliceEditLayout.SetSameLineFlag( true );
    m_SliceEditLayout.SetFitWidthFlag( false );

    m_SliceEditLayout.SetButtonWidth( m_SliceEditLayout.GetRemainX() / 3 );
    m_SliceEditLayout.AddButton( m_SliceShellToggle, "Shell" );
    m_SliceEditLayout.AddButton( m_SliceCapToggle, "Cap" );
    m_SliceEditLayout.AddButton( m_SliceShellCapToggle, "Shell and Cap" );

    m_SliceShellCapToggleGroup.Init( this );
    m_SliceShellCapToggleGroup.AddButton( m_SliceShellToggle.GetFlButton() );
    m_SliceShellCapToggleGroup.AddButton( m_SliceCapToggle.GetFlButton() );
    m_SliceShellCapToggleGroup.AddButton( m_SliceShellCapToggle.GetFlButton() );

    m_SliceEditLayout.SetSameLineFlag( false );
    m_SliceEditLayout.SetFitWidthFlag( true );
    m_SliceEditLayout.ForceNewLine();
    m_SliceEditLayout.SetChoiceButtonWidth( m_SliceEditLayout.GetRemainX() / 3 );

    m_SliceEditLayout.AddChoice( m_SlicePropertyChoice, "Property" );
    m_SliceEditLayout.AddChoice( m_SliceCapPropertyChoice, "Cap Property" );

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
    m_RibEditLayout.AddButton( m_RibPosAbsToggle, "Absolute" );

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

    m_RibEditLayout.AddDividerBox( "Elements" );

    m_RibEditLayout.SetSameLineFlag( true );
    m_RibEditLayout.SetFitWidthFlag( false );

    m_RibEditLayout.SetButtonWidth( m_RibEditLayout.GetRemainX() / 3 );
    m_RibEditLayout.AddButton( m_RibShellToggle, "Shell" );
    m_RibEditLayout.AddButton( m_RibCapToggle, "Cap" );
    m_RibEditLayout.AddButton( m_RibShellCapToggle, "Shell and Cap" );

    m_RibShellCapToggleGroup.Init( this );
    m_RibShellCapToggleGroup.AddButton( m_RibShellToggle.GetFlButton() );
    m_RibShellCapToggleGroup.AddButton( m_RibCapToggle.GetFlButton() );
    m_RibShellCapToggleGroup.AddButton( m_RibShellCapToggle.GetFlButton() );

    m_RibEditLayout.ForceNewLine();
    m_RibEditLayout.SetSameLineFlag( false );
    m_RibEditLayout.SetFitWidthFlag( true );
    m_RibEditLayout.SetChoiceButtonWidth( m_RibEditLayout.GetRemainX() / 3 );

    m_RibEditLayout.AddChoice( m_RibPropertyChoice, "Property" );
    m_RibEditLayout.AddChoice( m_RibCapPropertyChoice, "Cap Property" );

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
    m_SparEditLayout.AddButton( m_SparPosAbsToggle, "Absolute" );

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

    m_SparEditLayout.AddYGap();

    m_SparEditLayout.AddDividerBox( "Elements" );

    m_SparEditLayout.SetSameLineFlag( true );
    m_SparEditLayout.SetFitWidthFlag( false );

    m_SparEditLayout.SetButtonWidth( m_SparEditLayout.GetRemainX() / 3 );
    m_SparEditLayout.AddButton( m_SparShellToggle, "Shell" );
    m_SparEditLayout.AddButton( m_SparCapToggle, "Cap" );
    m_SparEditLayout.AddButton( m_SparShellCapToggle, "Shell and Cap" );

    m_SparShellCapToggleGroup.Init( this );
    m_SparShellCapToggleGroup.AddButton( m_SparShellToggle.GetFlButton() );
    m_SparShellCapToggleGroup.AddButton( m_SparCapToggle.GetFlButton() );
    m_SparShellCapToggleGroup.AddButton( m_SparShellCapToggle.GetFlButton() );

    m_SparEditLayout.SetSameLineFlag( false );
    m_SparEditLayout.SetFitWidthFlag( true );
    m_SparEditLayout.ForceNewLine();
    m_SparEditLayout.SetChoiceButtonWidth( m_SparEditLayout.GetRemainX() / 3 );

    m_SparEditLayout.AddChoice( m_SparPropertyChoice, "Property" );
    m_SparEditLayout.AddChoice( m_SparCapPropertyChoice, "Cap Property" );

    //==== FeaFixPoint ====//
    m_GenLayout.AddSubGroupLayout( m_FixPointEditLayout, m_GenLayout.GetRemainX(), m_GenLayout.GetRemainY() );
    m_FixPointEditLayout.SetY( start_y );

    m_FixPointEditLayout.AddDividerBox( "Fixed Point" );

    m_FixPointEditLayout.SetChoiceButtonWidth( m_FixPointEditLayout.GetRemainX() / 3 );
    m_FixPointEditLayout.SetButtonWidth( m_FixPointEditLayout.GetRemainX() / 3 );

    m_FixPointEditLayout.AddChoice( m_FixPointParentSurfChoice, "Parent Surface" );

    m_FixPointEditLayout.AddSlider( m_FixPointULocSlider, "U Location", 1, "%5.3f" );
    m_FixPointEditLayout.AddSlider( m_FixPointWLocSlider, "W Location", 1, "%5.3f" );

    m_FixPointEditLayout.AddYGap();
    m_FixPointEditLayout.AddDividerBox( "Element" );

    m_FixPointEditLayout.SetSameLineFlag( true );
    m_FixPointEditLayout.SetFitWidthFlag( false );

    m_FixPointEditLayout.SetButtonWidth( m_FixPointEditLayout.GetRemainX() / 17 );

    m_FixPointEditLayout.AddButton( m_FixPointMassToggle, "" );

    m_FixPointEditLayout.SetButtonWidth( ( m_FixPointEditLayout.GetW() / 3 ) - ( m_FixPointEditLayout.GetRemainX() / 16 ) );
    m_FixPointEditLayout.SetSliderWidth( m_FixPointEditLayout.GetW() / 4 );
    m_FixPointEditLayout.SetInputWidth(  m_FixPointEditLayout.GetRemainX() / 6 );

    m_FixPointEditLayout.AddSlider( m_FixPointMassSlider, "Mass", 100.0, "%5.3g" );

    m_FixPointEditLayout.AddButton( m_FixPointMassUnit, "" );
    m_FixPointMassUnit.GetFlButton()->box( FL_THIN_UP_BOX );
    m_FixPointMassUnit.GetFlButton()->labelcolor( FL_BLACK );
    m_FixPointMassUnit.SetWidth( ( m_FixPointEditLayout.GetW() / 5 ) );

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

    m_DomeEditLayout.AddYGap();

    m_DomeEditLayout.AddDividerBox( "Elements" );

    m_DomeEditLayout.SetSameLineFlag( true );
    m_DomeEditLayout.SetFitWidthFlag( false );

    m_DomeEditLayout.SetButtonWidth( m_DomeEditLayout.GetRemainX() / 3 );
    m_DomeEditLayout.AddButton( m_DomeShellToggle, "Shell" );
    m_DomeEditLayout.AddButton( m_DomeCapToggle, "Cap" );
    m_DomeEditLayout.AddButton( m_DomeShellCapToggle, "Shell and Cap" );

    m_DomeShellCapToggleGroup.Init( this );
    m_DomeShellCapToggleGroup.AddButton( m_DomeShellToggle.GetFlButton() );
    m_DomeShellCapToggleGroup.AddButton( m_DomeCapToggle.GetFlButton() );
    m_DomeShellCapToggleGroup.AddButton( m_DomeShellCapToggle.GetFlButton() );

    m_DomeEditLayout.SetSameLineFlag( false );
    m_DomeEditLayout.SetFitWidthFlag( true );
    m_DomeEditLayout.ForceNewLine();
    m_DomeEditLayout.SetChoiceButtonWidth( m_DomeEditLayout.GetRemainX() / 3 );

    m_DomeEditLayout.AddChoice( m_DomePropertyChoice, "Property" );
    m_DomeEditLayout.AddChoice( m_DomeCapPropertyChoice, "Cap Property" );

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
    m_RibArrayEditLayout.AddButton( m_RibArrayPosAbsToggle, "Absolute" );

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

    m_RibArrayEditLayout.AddYGap();

    m_RibArrayEditLayout.AddDividerBox( "Elements" );

    m_RibArrayEditLayout.SetSameLineFlag( true );
    m_RibArrayEditLayout.SetFitWidthFlag( false );

    m_RibArrayEditLayout.SetButtonWidth( m_RibArrayEditLayout.GetRemainX() / 3 );
    m_RibArrayEditLayout.AddButton( m_RibArrayShellToggle, "Shell" );
    m_RibArrayEditLayout.AddButton( m_RibArrayCapToggle, "Cap" );
    m_RibArrayEditLayout.AddButton( m_RibArrayShellCapToggle, "Shell and Cap" );

    m_RibArrayShellCapToggleGroup.Init( this );
    m_RibArrayShellCapToggleGroup.AddButton( m_RibArrayShellToggle.GetFlButton() );
    m_RibArrayShellCapToggleGroup.AddButton( m_RibArrayCapToggle.GetFlButton() );
    m_RibArrayShellCapToggleGroup.AddButton( m_RibArrayShellCapToggle.GetFlButton() );

    m_RibArrayEditLayout.SetSameLineFlag( false );
    m_RibArrayEditLayout.SetFitWidthFlag( true );
    m_RibArrayEditLayout.ForceNewLine();
    m_RibArrayEditLayout.SetChoiceButtonWidth( m_RibArrayEditLayout.GetRemainX() / 3 );

    m_RibArrayEditLayout.AddChoice( m_RibArrayPropertyChoice, "Property" );
    m_RibArrayEditLayout.AddChoice( m_RibArrayCapPropertyChoice, "Cap Property" );

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
    m_SliceArrayEditLayout.AddButton( m_SliceArrayPosAbsToggle, "Absolute" );

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

    m_SliceArrayEditLayout.AddYGap();

    m_SliceArrayEditLayout.AddDividerBox( "Elements" );

    m_SliceArrayEditLayout.SetSameLineFlag( true );
    m_SliceArrayEditLayout.SetFitWidthFlag( false );

    m_SliceArrayEditLayout.SetButtonWidth( m_SliceArrayEditLayout.GetRemainX() / 3 );
    m_SliceArrayEditLayout.AddButton( m_SliceArrayShellToggle, "Shell" );
    m_SliceArrayEditLayout.AddButton( m_SliceArrayCapToggle, "Cap" );
    m_SliceArrayEditLayout.AddButton( m_SliceArrayShellCapToggle, "Shell and Cap" );

    m_SliceArrayShellCapToggleGroup.Init( this );
    m_SliceArrayShellCapToggleGroup.AddButton( m_SliceArrayShellToggle.GetFlButton() );
    m_SliceArrayShellCapToggleGroup.AddButton( m_SliceArrayCapToggle.GetFlButton() );
    m_SliceArrayShellCapToggleGroup.AddButton( m_SliceArrayShellCapToggle.GetFlButton() );

    m_SliceArrayEditLayout.SetSameLineFlag( false );
    m_SliceArrayEditLayout.SetFitWidthFlag( true );
    m_SliceArrayEditLayout.ForceNewLine();
    m_SliceArrayEditLayout.SetChoiceButtonWidth( m_SliceArrayEditLayout.GetRemainX() / 3 );

    m_SliceArrayEditLayout.AddChoice( m_SliceArrayPropertyChoice, "Property" );
    m_SliceArrayEditLayout.AddChoice( m_SliceArrayCapPropertyChoice, "Cap Property" );

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

    m_FeaSSLineGroup.AddYGap();

    m_FeaSSLineGroup.AddDividerBox( "Elements" );

    m_FeaSSLineGroup.SetSameLineFlag( true );
    m_FeaSSLineGroup.SetFitWidthFlag( false );

    m_FeaSSLineGroup.SetButtonWidth( m_FeaSSLineGroup.GetRemainX() / 3 );
    m_FeaSSLineGroup.AddButton( m_FeaSSLineShellToggle, "Shell" );
    m_FeaSSLineGroup.AddButton( m_FeaSSLineCapToggle, "Cap (No Tris)" );
    m_FeaSSLineGroup.AddButton( m_FeaSSLineShellCapToggle, "Shell and Cap" );

    m_FeaSSLineShellCapToggleGroup.Init( this );
    m_FeaSSLineShellCapToggleGroup.AddButton( m_FeaSSLineShellToggle.GetFlButton() );
    m_FeaSSLineShellCapToggleGroup.AddButton( m_FeaSSLineCapToggle.GetFlButton() );
    m_FeaSSLineShellCapToggleGroup.AddButton( m_FeaSSLineShellCapToggle.GetFlButton() );

    m_FeaSSLineGroup.SetSameLineFlag( false );
    m_FeaSSLineGroup.SetFitWidthFlag( true );
    m_FeaSSLineGroup.ForceNewLine();

    m_FeaSSLineGroup.AddChoice( m_FeaSSLinePropertyChoice, "Property" );
    m_FeaSSLineGroup.AddChoice( m_FeaSSLineCapPropertyChoice, "Cap Property" );

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

    m_FeaSSRecGroup.AddYGap();

    m_FeaSSRecGroup.AddDividerBox( "Elements" );

    m_FeaSSRecGroup.SetSameLineFlag( true );
    m_FeaSSRecGroup.SetFitWidthFlag( false );

    m_FeaSSRecGroup.SetButtonWidth( m_FeaSSRecGroup.GetRemainX() / 3 );
    m_FeaSSRecGroup.AddButton( m_FeaSSRecShellToggle, "Shell" );
    m_FeaSSRecGroup.AddButton( m_FeaSSRecCapToggle, "Cap (No Tris)" );
    m_FeaSSRecGroup.AddButton( m_FeaSSRecShellCapToggle, "Shell and Cap" );

    m_FeaSSRecShellCapToggleGroup.Init( this );
    m_FeaSSRecShellCapToggleGroup.AddButton( m_FeaSSRecShellToggle.GetFlButton() );
    m_FeaSSRecShellCapToggleGroup.AddButton( m_FeaSSRecCapToggle.GetFlButton() );
    m_FeaSSRecShellCapToggleGroup.AddButton( m_FeaSSRecShellCapToggle.GetFlButton() );

    m_FeaSSRecGroup.SetSameLineFlag( false );
    m_FeaSSRecGroup.SetFitWidthFlag( true );
    m_FeaSSRecGroup.ForceNewLine();

    m_FeaSSRecGroup.AddChoice( m_FeaSSRecPropertyChoice, "Property" );
    m_FeaSSRecGroup.AddChoice( m_FeaSSRecCapPropertyChoice, "Cap Property" );

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

    m_FeaSSEllGroup.AddYGap();

    m_FeaSSEllGroup.AddDividerBox( "Elements" );

    m_FeaSSEllGroup.SetSameLineFlag( true );
    m_FeaSSEllGroup.SetFitWidthFlag( false );

    m_FeaSSEllGroup.SetButtonWidth( m_FeaSSEllGroup.GetRemainX() / 3 );
    m_FeaSSEllGroup.AddButton( m_FeaSSEllShellToggle, "Shell" );
    m_FeaSSEllGroup.AddButton( m_FeaSSEllCapToggle, "Cap (No Tris)" );
    m_FeaSSEllGroup.AddButton( m_FeaSSEllShellCapToggle, "Shell and Cap" );

    m_FeaSSEllShellCapToggleGroup.Init( this );
    m_FeaSSEllShellCapToggleGroup.AddButton( m_FeaSSEllShellToggle.GetFlButton() );
    m_FeaSSEllShellCapToggleGroup.AddButton( m_FeaSSEllCapToggle.GetFlButton() );
    m_FeaSSEllShellCapToggleGroup.AddButton( m_FeaSSEllShellCapToggle.GetFlButton() );

    m_FeaSSEllGroup.SetSameLineFlag( false );
    m_FeaSSEllGroup.SetFitWidthFlag( true );
    m_FeaSSEllGroup.ForceNewLine();

    m_FeaSSEllGroup.AddChoice( m_FeaSSEllPropertyChoice, "Property" );
    m_FeaSSEllGroup.AddChoice( m_FeaSSEllCapPropertyChoice, "Cap Property" );

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

    m_FeaSSConGroup.AddYGap();

    m_FeaSSConGroup.AddDividerBox( "Elements" );

    m_FeaSSConGroup.SetSameLineFlag( true );
    m_FeaSSConGroup.SetFitWidthFlag( false );

    m_FeaSSConGroup.SetButtonWidth( m_FeaSSConGroup.GetRemainX() / 3 );
    m_FeaSSConGroup.AddButton( m_FeaSSConShellToggle, "Shell" );
    m_FeaSSConGroup.AddButton( m_FeaSSConCapToggle, "Cap (No Tris)" );
    m_FeaSSConGroup.AddButton( m_FeaSSConShellCapToggle, "Shell and Cap" );

    m_FeaSSConShellCapToggleGroup.Init( this );
    m_FeaSSConShellCapToggleGroup.AddButton( m_FeaSSConShellToggle.GetFlButton() );
    m_FeaSSConShellCapToggleGroup.AddButton( m_FeaSSConCapToggle.GetFlButton() );
    m_FeaSSConShellCapToggleGroup.AddButton( m_FeaSSConShellCapToggle.GetFlButton() );

    m_FeaSSConGroup.SetSameLineFlag( false );
    m_FeaSSConGroup.SetFitWidthFlag( true );
    m_FeaSSConGroup.ForceNewLine();

    m_FeaSSConGroup.AddChoice( m_FeaSSConPropertyChoice, "Property" );
    m_FeaSSConGroup.AddChoice( m_FeaSSConCapPropertyChoice, "Cap Property" );

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

    m_FeaSSLineArrayGroup.AddYGap();

    m_FeaSSLineArrayGroup.AddDividerBox( "Elements" );

    m_FeaSSLineArrayGroup.AddChoice( m_FeaSSLineArrayCapPropertyChoice, "Cap Property" );
}

FeaPartEditScreen::~FeaPartEditScreen()
{
}

void FeaPartEditScreen::Show()
{
    if ( Update() )
    {
        m_FLTK_Window->show();
    }
}

bool FeaPartEditScreen::Update()
{
    assert( m_ScreenMgr );

    // Close the screen if StructScreen is not open
    VspScreen* struct_screen = m_ScreenMgr->GetScreen( ScreenMgr::VSP_STRUCT_SCREEN );
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

        //===== FeaProperty Update =====//
        UpdateFeaPropertyChoice();

        UpdateUnitLabels();

        if ( StructureMgr.ValidTotalFeaStructInd( StructureMgr.GetCurrStructIndex() ) )
        {
            vector< FeaStructure* > structVec = StructureMgr.GetAllFeaStructs();

            // Update FeaParts and SubSurfaces if FeaMesh is not in progress.
            //  Note: This update is mainly added to ensure GUI elements are upt to date (relative/absolute values are calculated and set in Update() )
            if ( !FeaMeshMgr.GetFeaMeshInProgress() )
            {
                structVec[StructureMgr.GetCurrStructIndex()]->Update();
            }

            // Update Current FeaPart
            if ( StructureMgr.GetCurrPartIndex() < structVec[StructureMgr.GetCurrStructIndex()]->NumFeaParts() )
            {
                FeaPart* feaprt = structVec[StructureMgr.GetCurrStructIndex()]->GetFeaPart( StructureMgr.GetCurrPartIndex() );
                if ( feaprt )
                {
                    m_FeaPartNameInput.Update( feaprt->GetName() );

                    if ( feaprt->GetType() == vsp::FEA_SKIN )
                    {
                        FeaSkin* skin = dynamic_cast<FeaSkin*>( feaprt );
                        assert( skin );

                        m_RemoveSkinToggle.Update( skin->m_RemoveSkinFlag.GetID() );

                        if ( skin->m_RemoveSkinFlag() )
                        {
                            m_SkinPropertyChoice.Deactivate();
                        }
                        else
                        {
                            m_SkinPropertyChoice.Activate();
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
                        m_SliceShellCapToggleGroup.Update( slice->m_IncludedElements.GetID() );

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

                        if ( slice->m_IncludedElements() == vsp::FEA_SHELL_AND_BEAM )
                        {
                            m_SlicePropertyChoice.Activate();
                            m_SliceCapPropertyChoice.Activate();
                        }
                        else if ( slice->m_IncludedElements() == vsp::FEA_SHELL )
                        {
                            m_SlicePropertyChoice.Activate();
                            m_SliceCapPropertyChoice.Deactivate();
                        }
                        else if ( slice->m_IncludedElements() == vsp::FEA_BEAM )
                        {
                            m_SlicePropertyChoice.Deactivate();
                            m_SliceCapPropertyChoice.Activate();
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
                        m_RibShellCapToggleGroup.Update( rib->m_IncludedElements.GetID() );

                        if ( rib->m_IncludedElements() == vsp::FEA_SHELL_AND_BEAM )
                        {
                            m_RibPropertyChoice.Activate();
                            m_RibCapPropertyChoice.Activate();
                        }
                        else if ( rib->m_IncludedElements() == vsp::FEA_SHELL )
                        {
                            m_RibPropertyChoice.Activate();
                            m_RibCapPropertyChoice.Deactivate();
                        }
                        else if ( rib->m_IncludedElements() == vsp::FEA_BEAM )
                        {
                            m_RibPropertyChoice.Deactivate();
                            m_RibCapPropertyChoice.Activate();
                        }

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
                        m_SparShellCapToggleGroup.Update( spar->m_IncludedElements.GetID() );

                        if ( spar->m_IncludedElements() == vsp::FEA_SHELL_AND_BEAM )
                        {
                            m_SparPropertyChoice.Activate();
                            m_SparCapPropertyChoice.Activate();
                        }
                        else if ( spar->m_IncludedElements() == vsp::FEA_SHELL )
                        {
                            m_SparPropertyChoice.Activate();
                            m_SparCapPropertyChoice.Deactivate();
                        }
                        else if ( spar->m_IncludedElements() == vsp::FEA_BEAM )
                        {
                            m_SparPropertyChoice.Deactivate();
                            m_SparCapPropertyChoice.Activate();
                        }

                        FeaPartDispGroup( &m_SparEditLayout );
                    }
                    else if ( feaprt->GetType() == vsp::FEA_FIX_POINT )
                    {
                        FeaFixPoint* fixpt = dynamic_cast<FeaFixPoint*>( feaprt );
                        assert( fixpt );

                        m_FixPointULocSlider.Update( fixpt->m_PosU.GetID() );
                        m_FixPointWLocSlider.Update( fixpt->m_PosW.GetID() );
                        m_FixPointMassToggle.Update( fixpt->m_FixPointMassFlag.GetID() );
                        m_FixPointMassSlider.Update( fixpt->m_FixPointMass.GetID() );

                        if ( fixpt->m_FixPointMassFlag() )
                        {
                            m_FixPointMassSlider.Activate();
                            m_FixPointMassUnit.Activate();
                        }
                        else
                        {
                            m_FixPointMassSlider.Deactivate();
                            m_FixPointMassUnit.Deactivate();
                        }

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

                        m_DomeShellCapToggleGroup.Update( dome->m_IncludedElements.GetID() );

                        if ( dome->m_SpineAttachFlag() )
                        {
                            m_DomeUSpineSlider.Activate();
                        }
                        else
                        {
                            m_DomeUSpineSlider.Deactivate();
                        }

                        if ( dome->m_IncludedElements() == vsp::FEA_SHELL_AND_BEAM )
                        {
                            m_DomePropertyChoice.Activate();
                            m_DomeCapPropertyChoice.Activate();
                        }
                        else if ( dome->m_IncludedElements() == vsp::FEA_SHELL )
                        {
                            m_DomePropertyChoice.Activate();
                            m_DomeCapPropertyChoice.Deactivate();
                        }
                        else if ( dome->m_IncludedElements() == vsp::FEA_BEAM )
                        {
                            m_DomePropertyChoice.Deactivate();
                            m_DomeCapPropertyChoice.Activate();
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
                        m_RibArrayShellCapToggleGroup.Update( rib_array->m_IncludedElements.GetID() );
                        m_RibArrayPosNegDirToggleGroup.Update( rib_array->m_PositiveDirectionFlag.GetID() );
                        m_RibArrayRotateDihedralToggle.Update( rib_array->m_MatchDihedralFlag.GetID() );

                        if ( rib_array->m_IncludedElements() == vsp::FEA_SHELL_AND_BEAM )
                        {
                            m_RibArrayPropertyChoice.Activate();
                            m_RibArrayCapPropertyChoice.Activate();
                        }
                        else if ( rib_array->m_IncludedElements() == vsp::FEA_SHELL )
                        {
                            m_RibArrayPropertyChoice.Activate();
                            m_RibArrayCapPropertyChoice.Deactivate();
                        }
                        else if ( rib_array->m_IncludedElements() == vsp::FEA_BEAM )
                        {
                            m_RibArrayPropertyChoice.Deactivate();
                            m_RibArrayCapPropertyChoice.Activate();
                        }

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
                        m_SliceArrayShellCapToggleGroup.Update( slice_array->m_IncludedElements.GetID() );

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

                        if ( slice_array->m_IncludedElements() == vsp::FEA_SHELL_AND_BEAM )
                        {
                            m_SliceArrayPropertyChoice.Activate();
                            m_SliceArrayCapPropertyChoice.Activate();
                        }
                        else if ( slice_array->m_IncludedElements() == vsp::FEA_SHELL )
                        {
                            m_SliceArrayPropertyChoice.Activate();
                            m_SliceArrayCapPropertyChoice.Deactivate();
                        }
                        else if ( slice_array->m_IncludedElements() == vsp::FEA_BEAM )
                        {
                            m_SliceArrayPropertyChoice.Deactivate();
                            m_SliceArrayCapPropertyChoice.Activate();
                        }

                        FeaPartDispGroup( &m_SliceArrayEditLayout );
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
            else if ( StructureMgr.GetCurrPartIndex() >= structVec[StructureMgr.GetCurrStructIndex()]->NumFeaParts() )
            {
                //===== SubSurface =====//
                SubSurface* subsurf = structVec[StructureMgr.GetCurrStructIndex()]->GetFeaSubSurf( StructureMgr.GetCurrPartIndex() - structVec[StructureMgr.GetCurrStructIndex()]->NumFeaParts() );
                if ( subsurf )
                {
                    m_FeaPartNameInput.Update( subsurf->GetName() );

                    if ( subsurf->GetType() == vsp::SS_LINE )
                    {
                        SSLine* ssline = dynamic_cast<SSLine*>( subsurf );
                        assert( ssline );

                        m_FeaSSLineConstToggleGroup.Update( ssline->m_ConstType.GetID() );
                        m_FeaSSLineTestToggleGroup.Update( ssline->m_TestType.GetID() );
                        m_FeaSSLineConstSlider.Update( ssline->m_ConstVal.GetID() );
                        m_FeaSSLineShellCapToggleGroup.Update( ssline->m_IncludedElements.GetID() );

                        if ( ssline->m_IncludedElements() == vsp::FEA_BEAM )
                        {
                            m_FeaSSLineCapPropertyChoice.Activate();
                            m_FeaSSLinePropertyChoice.Deactivate();
                        }
                        else if ( ssline->m_IncludedElements() == vsp::FEA_SHELL )
                        {
                            m_FeaSSLineCapPropertyChoice.Deactivate();
                            m_FeaSSLinePropertyChoice.Activate();
                        }
                        else if ( ssline->m_IncludedElements() == vsp::FEA_SHELL_AND_BEAM )
                        {
                            m_FeaSSLineCapPropertyChoice.Activate();
                            m_FeaSSLinePropertyChoice.Activate();
                        }

                        if ( ssline->m_TestType() == vsp::NONE )
                        {
                            m_FeaSSLinePropertyChoice.Deactivate();
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
                        m_FeaSSRecShellCapToggleGroup.Update( ssrec->m_IncludedElements.GetID() );

                        if ( ssrec->m_IncludedElements() == vsp::FEA_BEAM )
                        {
                            m_FeaSSRecCapPropertyChoice.Activate();
                            m_FeaSSRecPropertyChoice.Deactivate();
                            m_FeaSSRecTestToggleGroup.Deactivate();
                            ssrec->m_TestType.Set( vsp::INSIDE ); // Inside tris must be set to be removed. Skin tris are saved/removed using FeaSkin parms
                        }
                        else if ( ssrec->m_IncludedElements() == vsp::FEA_SHELL )
                        {
                            m_FeaSSRecCapPropertyChoice.Deactivate();
                            m_FeaSSRecPropertyChoice.Activate();
                            m_FeaSSRecTestToggleGroup.Activate();
                        }
                        else if ( ssrec->m_IncludedElements() == vsp::FEA_SHELL_AND_BEAM )
                        {
                            m_FeaSSRecCapPropertyChoice.Activate();
                            m_FeaSSRecPropertyChoice.Activate();
                            m_FeaSSRecTestToggleGroup.Activate();
                        }

                        if ( ssrec->m_TestType() == vsp::NONE )
                        {
                            m_FeaSSRecPropertyChoice.Deactivate();
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
                        m_FeaSSEllShellCapToggleGroup.Update( ssell->m_IncludedElements.GetID() );

                        if ( ssell->m_IncludedElements() == vsp::FEA_BEAM )
                        {
                            m_FeaSSEllCapPropertyChoice.Activate();
                            m_FeaSSEllPropertyChoice.Deactivate();
                            m_FeaSSEllTestToggleGroup.Deactivate();
                            ssell->m_TestType.Set( vsp::INSIDE ); // Inside tris must be set to be removed. Skin tris are saved/removed using FeaSkin parms
                        }
                        else if ( ssell->m_IncludedElements() == vsp::FEA_SHELL )
                        {
                            m_FeaSSEllCapPropertyChoice.Deactivate();
                            m_FeaSSEllPropertyChoice.Activate();
                            m_FeaSSEllTestToggleGroup.Activate();
                        }
                        else if ( ssell->m_IncludedElements() == vsp::FEA_SHELL_AND_BEAM )
                        {
                            m_FeaSSEllCapPropertyChoice.Activate();
                            m_FeaSSEllPropertyChoice.Activate();
                            m_FeaSSEllTestToggleGroup.Activate();
                        }

                        if ( ssell->m_TestType() == vsp::NONE )
                        {
                            m_FeaSSEllPropertyChoice.Deactivate();
                        }

                        FeaPartDispGroup( &m_FeaSSEllGroup );
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
                        m_FeaSSConShellCapToggleGroup.Update( sscon->m_IncludedElements.GetID() );
                        m_FeaSSConSAngleButton.Update( sscon->m_StartAngleFlag.GetID() );
                        m_FeaSSConEAngleButton.Update( sscon->m_EndAngleFlag.GetID() );
                        m_FeaSSConSAngleSlider.Update( sscon->m_StartAngle.GetID() );
                        m_FeaSSConEAngleSlider.Update( sscon->m_EndAngle.GetID() );

                        m_FeaSSConTessSlider.Update( sscon->m_Tess.GetID() );

                        if ( sscon->m_IncludedElements() == vsp::FEA_BEAM )
                        {
                            m_FeaSSConCapPropertyChoice.Activate();
                            m_FeaSSConPropertyChoice.Deactivate();
                            m_FeaSSConTestToggleGroup.Deactivate();
                            sscon->m_TestType.Set( vsp::INSIDE ); // Inside tris must be set to be removed. Skin tris are saved/removed using FeaSkin parms
                        }
                        else if ( sscon->m_IncludedElements() == vsp::FEA_SHELL )
                        {
                            m_FeaSSConCapPropertyChoice.Deactivate();
                            m_FeaSSConPropertyChoice.Activate();
                            m_FeaSSConTestToggleGroup.Activate();
                        }
                        else if ( sscon->m_IncludedElements() == vsp::FEA_SHELL_AND_BEAM )
                        {
                            m_FeaSSConCapPropertyChoice.Activate();
                            m_FeaSSConPropertyChoice.Activate();
                            m_FeaSSConTestToggleGroup.Activate();
                        }

                        if ( sscon->m_TestType() == vsp::NONE )
                        {
                            m_FeaSSConPropertyChoice.Deactivate();
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

                        m_FeaSSConSFracSlider.Deactivate();
                        m_FeaSSConSLenSlider.Deactivate();

                        m_FeaSSConEFracSlider.Deactivate();
                        m_FeaSSConELenSlider.Deactivate();

                        if ( sscon->m_AbsRelFlag() == vsp::ABS )
                        {
                            m_FeaSSConSLenSlider.Activate();

                            if ( !sscon->m_ConstFlag() )
                            {
                                m_FeaSSConELenSlider.Activate();
                            }
                        }
                        else
                        {
                            m_FeaSSConSFracSlider.Activate();

                            if ( !sscon->m_ConstFlag() )
                            {
                                m_FeaSSConEFracSlider.Activate();
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

                        FeaPartDispGroup( &m_FeaSSLineArrayGroup );
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

    if ( device == &m_FeaPartNameInput )
    {
        if ( StructureMgr.ValidTotalFeaStructInd( StructureMgr.GetCurrStructIndex() ) )
        {
            vector < FeaStructure* > structvec = StructureMgr.GetAllFeaStructs();

            if ( StructureMgr.GetCurrPartIndex() < structvec[StructureMgr.GetCurrStructIndex()]->NumFeaParts() )
            {
                FeaPart* feaprt = structvec[StructureMgr.GetCurrStructIndex()]->GetFeaPart( StructureMgr.GetCurrPartIndex() );

                if ( feaprt )
                {
                    feaprt->SetName( m_FeaPartNameInput.GetString() );
                }
            }
            else if ( StructureMgr.GetCurrPartIndex() >= structvec[StructureMgr.GetCurrStructIndex()]->NumFeaParts() )
            {
                SubSurface* subsurf = structvec[StructureMgr.GetCurrStructIndex()]->GetFeaSubSurf( StructureMgr.GetCurrPartIndex() - structvec[StructureMgr.GetCurrStructIndex()]->NumFeaParts() );

                if ( subsurf )
                {
                    subsurf->SetName( m_FeaPartNameInput.GetString() );
                }
            }
        }
    }
    else if ( device == &m_SliceOrientationChoice )
    {
        StructureMgr.SetFeaSliceOrientIndex( m_SliceOrientationChoice.GetVal() );
    }
    else if ( device == &m_SkinPropertyChoice || device == &m_SlicePropertyChoice || device == &m_RibPropertyChoice || device == &m_SparPropertyChoice
              || device == &m_DomePropertyChoice || device == &m_RibArrayPropertyChoice || device == &m_FeaSSLinePropertyChoice || device == &m_FeaSSRecPropertyChoice
              || device == &m_FeaSSEllPropertyChoice || device == &m_FeaSSConPropertyChoice || device == &m_SliceArrayPropertyChoice )
    {
        Choice* selected_choice = dynamic_cast<Choice*>( device );
        assert( selected_choice );

        SetFeaPartPropertyIndex( selected_choice );
    }
    else if ( device == &m_SliceCapPropertyChoice || device == &m_RibCapPropertyChoice || device == &m_SparCapPropertyChoice || device == &m_DomeCapPropertyChoice
              || device == &m_RibArrayCapPropertyChoice || device == &m_SliceArrayCapPropertyChoice || device == &m_FeaSSLineCapPropertyChoice
              || device == &m_FeaSSRecCapPropertyChoice || device == &m_FeaSSEllCapPropertyChoice || device == &m_FeaSSConCapPropertyChoice || device == &m_FeaSSLineArrayCapPropertyChoice )
    {
        Choice* selected_choice = dynamic_cast<Choice*>( device );
        assert( selected_choice );

        SetCapPropertyIndex( selected_choice );
    }
    else if ( device == &m_RibPerpendicularSparChoice && StructureMgr.ValidTotalFeaStructInd( StructureMgr.GetCurrStructIndex() ) )
    {
        vector < FeaStructure* > structVec = StructureMgr.GetAllFeaStructs();

        if ( StructureMgr.GetCurrPartIndex() < structVec[StructureMgr.GetCurrStructIndex()]->NumFeaParts() )
        {
            FeaPart* feaprt = structVec[StructureMgr.GetCurrStructIndex()]->GetFeaPart( StructureMgr.GetCurrPartIndex() );

            if ( feaprt && feaprt->GetType() == vsp::FEA_RIB )
            {
                FeaRib* rib = dynamic_cast<FeaRib*>( feaprt );
                assert( rib );

                rib->SetPerpendicularEdgeID( m_PerpendicularEdgeVec[m_RibPerpendicularSparChoice.GetVal()] );
            }
        }
    }
    else if ( device == &m_RibNormalTypeGroup && StructureMgr.ValidTotalFeaStructInd( StructureMgr.GetCurrStructIndex() ) )
    {
        vector < FeaStructure* > structVec = StructureMgr.GetAllFeaStructs();

        if ( StructureMgr.GetCurrPartIndex() < structVec[StructureMgr.GetCurrStructIndex()]->NumFeaParts() )
        {
            FeaPart* feaprt = structVec[StructureMgr.GetCurrStructIndex()]->GetFeaPart( StructureMgr.GetCurrPartIndex() );

            if ( feaprt && feaprt->GetType() == vsp::FEA_RIB )
            {
                FeaRib* rib = dynamic_cast<FeaRib*>( feaprt );
                assert( rib );

                if ( rib->m_PerpendicularEdgeType.Get() == vsp::SPAR_NORMAL )
                {
                    rib->SetPerpendicularEdgeID( m_PerpendicularEdgeVec[m_RibPerpendicularSparChoice.GetVal()] );
                }
            }
        }
    }
    else if ( device == &m_FixPointParentSurfChoice )
    {
        if ( StructureMgr.ValidTotalFeaStructInd( StructureMgr.GetCurrStructIndex() ) )
        {
            vector < FeaStructure* > structVec = StructureMgr.GetAllFeaStructs();

            FeaPart* parent_feaprt = StructureMgr.GetFeaPart( m_FixPointParentIDVec[m_FixPointParentSurfChoice.GetVal()] );

            if ( StructureMgr.GetCurrPartIndex() < structVec[StructureMgr.GetCurrStructIndex()]->NumFeaParts() )
            {
                FeaPart* feaprt = structVec[StructureMgr.GetCurrStructIndex()]->GetFeaPart( StructureMgr.GetCurrPartIndex() );

                if ( feaprt && parent_feaprt )
                {
                    if ( feaprt->GetType() == vsp::FEA_FIX_POINT )
                    {
                        FeaFixPoint* fixpt = dynamic_cast<FeaFixPoint*>( feaprt );
                        assert( fixpt );

                        fixpt->m_ParentFeaPartID = parent_feaprt->GetID();
                    }
                }
            }
        }
    }
    else if ( device == &m_RibArrayPerpendicularSparChoice && StructureMgr.ValidTotalFeaStructInd( StructureMgr.GetCurrStructIndex() ) )
    {
        vector < FeaStructure* > structVec = StructureMgr.GetAllFeaStructs();

        if ( StructureMgr.GetCurrPartIndex() < structVec[StructureMgr.GetCurrStructIndex()]->NumFeaParts() )
        {
            FeaPart* feaprt = structVec[StructureMgr.GetCurrStructIndex()]->GetFeaPart( StructureMgr.GetCurrPartIndex() );

            if ( feaprt && feaprt->GetType() == vsp::FEA_RIB_ARRAY )
            {
                FeaRibArray* rib_array = dynamic_cast<FeaRibArray*>( feaprt );
                assert( rib_array );

                rib_array->SetPerpendicularEdgeID( m_PerpendicularEdgeVec[m_RibArrayPerpendicularSparChoice.GetVal()] );
            }
        }
    }
    else if ( device == &m_RibArrayNormalTypeGroup && StructureMgr.ValidTotalFeaStructInd( StructureMgr.GetCurrStructIndex() ) )
    {
        vector < FeaStructure* > structVec = StructureMgr.GetAllFeaStructs();

        if ( StructureMgr.GetCurrPartIndex() < structVec[StructureMgr.GetCurrStructIndex()]->NumFeaParts() )
        {
            FeaPart* feaprt = structVec[StructureMgr.GetCurrStructIndex()]->GetFeaPart( StructureMgr.GetCurrPartIndex() );

            if ( feaprt && feaprt->GetType() == vsp::FEA_RIB_ARRAY )
            {
                FeaRibArray* rib_array = dynamic_cast<FeaRibArray*>( feaprt );
                assert( rib_array );

                if ( rib_array->m_PerpendicularEdgeType.Get() == vsp::SPAR_NORMAL )
                {
                    rib_array->SetPerpendicularEdgeID( m_PerpendicularEdgeVec[m_RibArrayPerpendicularSparChoice.GetVal()] );
                }
            }
        }
    }
    else if ( device == &m_IndividualizeRibArrayButton )
    {
        if ( StructureMgr.ValidTotalFeaStructInd( StructureMgr.GetCurrStructIndex() ) )
        {
            vector < FeaStructure* > structVec = StructureMgr.GetAllFeaStructs();

            if ( StructureMgr.GetCurrPartIndex() < structVec[StructureMgr.GetCurrStructIndex()]->NumFeaParts() )
            {
                FeaPart* feaprt = structVec[StructureMgr.GetCurrStructIndex()]->GetFeaPart( StructureMgr.GetCurrPartIndex() );

                if ( feaprt )
                {
                    if ( feaprt->GetType() == vsp::FEA_RIB_ARRAY )
                    {
                        structVec[StructureMgr.GetCurrStructIndex()]->IndividualizeRibArray( StructureMgr.GetCurrPartIndex() );
                    }
                }
            }
        }
    }
    else if ( device == &m_IndividualizeSliceArrayButton )
    {
        if ( StructureMgr.ValidTotalFeaStructInd( StructureMgr.GetCurrStructIndex() ) )
        {
            vector < FeaStructure* > structVec = StructureMgr.GetAllFeaStructs();

            if ( StructureMgr.GetCurrPartIndex() < structVec[StructureMgr.GetCurrStructIndex()]->NumFeaParts() )
            {
                FeaPart* feaprt = structVec[StructureMgr.GetCurrStructIndex()]->GetFeaPart( StructureMgr.GetCurrPartIndex() );

                if ( feaprt )
                {
                    if ( feaprt->GetType() == vsp::FEA_SLICE_ARRAY )
                    {
                        structVec[StructureMgr.GetCurrStructIndex()]->IndividualizeSliceArray( StructureMgr.GetCurrPartIndex() );
                    }
                }
            }
        }
    }
    else if ( device == &m_IndividualizeSSLineArrayButton )
    {
        if ( StructureMgr.ValidTotalFeaStructInd( StructureMgr.GetCurrStructIndex() ) )
        {
            vector < FeaStructure* > structVec = StructureMgr.GetAllFeaStructs();

            if ( StructureMgr.GetCurrPartIndex() >= structVec[StructureMgr.GetCurrStructIndex()]->NumFeaParts() )
            {
                SubSurface* subsurf = structVec[StructureMgr.GetCurrStructIndex()]->GetFeaSubSurf( StructureMgr.GetCurrPartIndex() - structVec[StructureMgr.GetCurrStructIndex()]->NumFeaParts() );
                
                if ( subsurf )
                {
                    if ( subsurf->GetType() == vsp::SS_LINE_ARRAY )
                    {
                        structVec[StructureMgr.GetCurrStructIndex()]->IndividualizeSSLineArray( StructureMgr.GetCurrPartIndex() - structVec[StructureMgr.GetCurrStructIndex()]->NumFeaParts() );
                    }
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

void FeaPartEditScreen::UpdateFeaPropertyChoice()
{
    //==== Property Choice ====//
    m_SkinPropertyChoice.ClearItems();
    m_SlicePropertyChoice.ClearItems();
    m_RibPropertyChoice.ClearItems();
    m_SparPropertyChoice.ClearItems();
    m_DomePropertyChoice.ClearItems();
    m_RibArrayPropertyChoice.ClearItems();
    m_SliceArrayPropertyChoice.ClearItems();
    m_FeaSSLinePropertyChoice.ClearItems();
    m_FeaSSRecPropertyChoice.ClearItems();
    m_FeaSSEllPropertyChoice.ClearItems();
    m_FeaSSConPropertyChoice.ClearItems();

    m_SparCapPropertyChoice.ClearItems();
    m_SliceCapPropertyChoice.ClearItems();
    m_RibCapPropertyChoice.ClearItems();
    m_DomeCapPropertyChoice.ClearItems();
    m_RibArrayCapPropertyChoice.ClearItems();
    m_SliceArrayCapPropertyChoice.ClearItems();

    m_FeaSSLineCapPropertyChoice.ClearItems();
    m_FeaSSRecCapPropertyChoice.ClearItems();
    m_FeaSSEllCapPropertyChoice.ClearItems();
    m_FeaSSConCapPropertyChoice.ClearItems();
    m_FeaSSLineArrayCapPropertyChoice.ClearItems();

    Vehicle*  veh = m_ScreenMgr->GetVehiclePtr();

    if ( veh )
    {
        vector< FeaProperty* > property_vec = StructureMgr.GetFeaPropertyVec();

        for ( int i = 0; i < property_vec.size(); ++i )
        {
            m_SkinPropertyChoice.AddItem( string( property_vec[i]->GetName() ) );
            m_SlicePropertyChoice.AddItem( string( property_vec[i]->GetName() ) );
            m_RibPropertyChoice.AddItem( string( property_vec[i]->GetName() ) );
            m_SparPropertyChoice.AddItem( string( property_vec[i]->GetName() ) );
            m_DomePropertyChoice.AddItem( string( property_vec[i]->GetName() ) );
            m_RibArrayPropertyChoice.AddItem( string( property_vec[i]->GetName() ) );
            m_SliceArrayPropertyChoice.AddItem( string( property_vec[i]->GetName() ) );
            m_FeaSSLinePropertyChoice.AddItem( string( property_vec[i]->GetName() ) );
            m_FeaSSRecPropertyChoice.AddItem( string( property_vec[i]->GetName() ) );
            m_FeaSSEllPropertyChoice.AddItem( string( property_vec[i]->GetName() ) );
            m_FeaSSConPropertyChoice.AddItem( string( property_vec[i]->GetName() ) );

            m_SparCapPropertyChoice.AddItem( string( property_vec[i]->GetName() ) );
            m_SliceCapPropertyChoice.AddItem( string( property_vec[i]->GetName() ) );
            m_RibCapPropertyChoice.AddItem( string( property_vec[i]->GetName() ) );
            m_DomeCapPropertyChoice.AddItem( string( property_vec[i]->GetName() ) );
            m_RibArrayCapPropertyChoice.AddItem( string( property_vec[i]->GetName() ) );
            m_SliceArrayCapPropertyChoice.AddItem( string( property_vec[i]->GetName() ) );

            m_FeaSSLineCapPropertyChoice.AddItem( string( property_vec[i]->GetName() ) );
            m_FeaSSRecCapPropertyChoice.AddItem( string( property_vec[i]->GetName() ) );
            m_FeaSSEllCapPropertyChoice.AddItem( string( property_vec[i]->GetName() ) );
            m_FeaSSConCapPropertyChoice.AddItem( string( property_vec[i]->GetName() ) );
            m_FeaSSLineArrayCapPropertyChoice.AddItem( string( property_vec[i]->GetName() ) );

            if ( property_vec[i]->m_FeaPropertyType() == vsp::FEA_SHELL )
            {
                m_SkinPropertyChoice.SetFlag( i, 0 );
                m_SlicePropertyChoice.SetFlag( i, 0 );
                m_RibPropertyChoice.SetFlag( i, 0 );
                m_SparPropertyChoice.SetFlag( i, 0 );
                m_DomePropertyChoice.SetFlag( i, 0 );
                m_RibArrayPropertyChoice.SetFlag( i, 0 );
                m_SliceArrayPropertyChoice.SetFlag( i, 0 );
                m_FeaSSLinePropertyChoice.SetFlag( i, 0 );
                m_FeaSSRecPropertyChoice.SetFlag( i, 0 );
                m_FeaSSEllPropertyChoice.SetFlag( i, 0 );
                m_FeaSSConPropertyChoice.SetFlag( i, 0 );

                m_SparCapPropertyChoice.SetFlag( i, FL_MENU_INACTIVE );
                m_SliceCapPropertyChoice.SetFlag( i, FL_MENU_INACTIVE );
                m_RibCapPropertyChoice.SetFlag( i, FL_MENU_INACTIVE );
                m_DomeCapPropertyChoice.SetFlag( i, FL_MENU_INACTIVE );
                m_RibArrayCapPropertyChoice.SetFlag( i, FL_MENU_INACTIVE );
                m_SliceArrayCapPropertyChoice.SetFlag( i, FL_MENU_INACTIVE );

                m_FeaSSLineCapPropertyChoice.SetFlag( i, FL_MENU_INACTIVE );
                m_FeaSSRecCapPropertyChoice.SetFlag( i, FL_MENU_INACTIVE );
                m_FeaSSEllCapPropertyChoice.SetFlag( i, FL_MENU_INACTIVE );
                m_FeaSSConCapPropertyChoice.SetFlag( i, FL_MENU_INACTIVE );
                m_FeaSSLineArrayCapPropertyChoice.SetFlag( i, FL_MENU_INACTIVE );
            }
            else if ( property_vec[i]->m_FeaPropertyType() == vsp::FEA_BEAM )
            {
                m_SkinPropertyChoice.SetFlag( i, FL_MENU_INACTIVE );
                m_SlicePropertyChoice.SetFlag( i, FL_MENU_INACTIVE );
                m_RibPropertyChoice.SetFlag( i, FL_MENU_INACTIVE );
                m_SparPropertyChoice.SetFlag( i, FL_MENU_INACTIVE );
                m_DomePropertyChoice.SetFlag( i, FL_MENU_INACTIVE );
                m_RibArrayPropertyChoice.SetFlag( i, FL_MENU_INACTIVE );
                m_SliceArrayPropertyChoice.SetFlag( i, FL_MENU_INACTIVE );
                m_FeaSSLinePropertyChoice.SetFlag( i, FL_MENU_INACTIVE );
                m_FeaSSRecPropertyChoice.SetFlag( i, FL_MENU_INACTIVE );
                m_FeaSSEllPropertyChoice.SetFlag( i, FL_MENU_INACTIVE );
                m_FeaSSConPropertyChoice.SetFlag( i, FL_MENU_INACTIVE );

                m_SparCapPropertyChoice.SetFlag( i, 0 );
                m_SliceCapPropertyChoice.SetFlag( i, 0 );
                m_RibCapPropertyChoice.SetFlag( i, 0 );
                m_DomeCapPropertyChoice.SetFlag( i, 0 );
                m_RibArrayCapPropertyChoice.SetFlag( i, 0 );
                m_SliceArrayCapPropertyChoice.SetFlag( i, 0 );

                m_FeaSSLineCapPropertyChoice.SetFlag( i, 0 );
                m_FeaSSRecCapPropertyChoice.SetFlag( i, 0 );
                m_FeaSSEllCapPropertyChoice.SetFlag( i, 0 );
                m_FeaSSConCapPropertyChoice.SetFlag( i, 0 );
                m_FeaSSLineArrayCapPropertyChoice.SetFlag( i, 0 );
            }
        }

        m_SkinPropertyChoice.UpdateItems();
        m_SlicePropertyChoice.UpdateItems();
        m_RibPropertyChoice.UpdateItems();
        m_SparPropertyChoice.UpdateItems();
        m_DomePropertyChoice.UpdateItems();
        m_RibArrayPropertyChoice.UpdateItems();
        m_SliceArrayPropertyChoice.UpdateItems();
        m_FeaSSLinePropertyChoice.UpdateItems();
        m_FeaSSRecPropertyChoice.UpdateItems();
        m_FeaSSEllPropertyChoice.UpdateItems();
        m_FeaSSConPropertyChoice.UpdateItems();

        m_SparCapPropertyChoice.UpdateItems();
        m_SliceCapPropertyChoice.UpdateItems();
        m_RibCapPropertyChoice.UpdateItems();
        m_DomeCapPropertyChoice.UpdateItems();
        m_RibArrayCapPropertyChoice.UpdateItems();
        m_SliceArrayCapPropertyChoice.UpdateItems();

        m_FeaSSLineCapPropertyChoice.UpdateItems();
        m_FeaSSRecCapPropertyChoice.UpdateItems();
        m_FeaSSEllCapPropertyChoice.UpdateItems();
        m_FeaSSConCapPropertyChoice.UpdateItems();
        m_FeaSSLineArrayCapPropertyChoice.UpdateItems();

        if ( StructureMgr.ValidTotalFeaStructInd( StructureMgr.GetCurrStructIndex() ) )
        {
            vector < FeaStructure* > structvec = StructureMgr.GetAllFeaStructs();

            if ( StructureMgr.GetCurrPartIndex() < structvec[StructureMgr.GetCurrStructIndex()]->NumFeaParts() )
            {
                FeaPart* feaprt = structvec[StructureMgr.GetCurrStructIndex()]->GetFeaPart( StructureMgr.GetCurrPartIndex() );

                if ( feaprt )
                {
                    // Update all FeaPart Property Choices ( Only Selected Part Visible )
                    m_SkinPropertyChoice.SetVal( feaprt->m_FeaPropertyIndex() );
                    m_SlicePropertyChoice.SetVal( feaprt->m_FeaPropertyIndex() );
                    m_RibPropertyChoice.SetVal( feaprt->m_FeaPropertyIndex() );
                    m_SparPropertyChoice.SetVal( feaprt->m_FeaPropertyIndex() );
                    m_DomePropertyChoice.SetVal( feaprt->m_FeaPropertyIndex() );
                    m_RibArrayPropertyChoice.SetVal( feaprt->m_FeaPropertyIndex() );
                    m_SliceArrayPropertyChoice.SetVal( feaprt->m_FeaPropertyIndex() );

                    m_SparCapPropertyChoice.SetVal( feaprt->m_CapFeaPropertyIndex() );
                    m_SliceCapPropertyChoice.SetVal( feaprt->m_CapFeaPropertyIndex() );
                    m_RibCapPropertyChoice.SetVal( feaprt->m_CapFeaPropertyIndex() );
                    m_DomeCapPropertyChoice.SetVal( feaprt->m_CapFeaPropertyIndex() );
                    m_RibArrayCapPropertyChoice.SetVal( feaprt->m_CapFeaPropertyIndex() );
                    m_SliceArrayCapPropertyChoice.SetVal( feaprt->m_CapFeaPropertyIndex() );
                }
            }
            else if ( StructureMgr.GetCurrPartIndex() >= structvec[StructureMgr.GetCurrStructIndex()]->NumFeaParts() )
            {
                SubSurface* subsurf = structvec[StructureMgr.GetCurrStructIndex()]->GetFeaSubSurf( StructureMgr.GetCurrPartIndex() - structvec[StructureMgr.GetCurrStructIndex()]->NumFeaParts() );

                if ( subsurf )
                {
                    // Update all FeaSubSurface Property Choices ( Only Selected Part Visible )
                    m_FeaSSLinePropertyChoice.SetVal( subsurf->m_FeaPropertyIndex() );
                    m_FeaSSRecPropertyChoice.SetVal( subsurf->m_FeaPropertyIndex() );
                    m_FeaSSEllPropertyChoice.SetVal( subsurf->m_FeaPropertyIndex() );
                    m_FeaSSConPropertyChoice.SetVal( subsurf->m_FeaPropertyIndex() );

                    m_FeaSSLineCapPropertyChoice.SetVal( subsurf->m_CapFeaPropertyIndex() );
                    m_FeaSSRecCapPropertyChoice.SetVal( subsurf->m_CapFeaPropertyIndex() );
                    m_FeaSSEllCapPropertyChoice.SetVal( subsurf->m_CapFeaPropertyIndex() );
                    m_FeaSSConCapPropertyChoice.SetVal( subsurf->m_CapFeaPropertyIndex() );
                    m_FeaSSLineArrayCapPropertyChoice.SetVal( subsurf->m_CapFeaPropertyIndex() );
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

    if ( StructureMgr.ValidTotalFeaStructInd( StructureMgr.GetCurrStructIndex() ) )
    {
        vector< FeaStructure* > structVec = StructureMgr.GetAllFeaStructs();
        vector<FeaPart*> feaprt_vec = structVec[StructureMgr.GetCurrStructIndex()]->GetFeaPartVec();

        for ( size_t i = 0; i < feaprt_vec.size(); i++ )
        {
            if ( !structVec[StructureMgr.GetCurrStructIndex()]->FeaPartIsFixPoint( i ) )
            {
                m_FixPointParentSurfChoice.AddItem( string( feaprt_vec[i]->GetName() ) );
                m_FixPointParentIDVec.push_back( feaprt_vec[i]->GetID() );
            }
        }

        m_FixPointParentSurfChoice.UpdateItems();

        if ( StructureMgr.GetCurrPartIndex() < structVec[StructureMgr.GetCurrStructIndex()]->NumFeaParts() )
        {
            FeaPart* feaprt = structVec[StructureMgr.GetCurrStructIndex()]->GetFeaPart( StructureMgr.GetCurrPartIndex() );

            if ( feaprt )
            {
                if ( feaprt->GetType() == vsp::FEA_FIX_POINT )
                {
                    FeaFixPoint* fixpt = dynamic_cast<FeaFixPoint*>( feaprt );
                    assert( fixpt );

                    FeaPart* parent_feaprt = StructureMgr.GetFeaPart( fixpt->m_ParentFeaPartID );

                    if ( parent_feaprt )
                    {
                        int parent_index = structVec[StructureMgr.GetCurrStructIndex()]->GetFeaPartIndex( parent_feaprt );
                        int num_fix_pnts_prior = 0;

                        for ( size_t i = 0; i < parent_index; i++ )
                        {
                            if ( structVec[StructureMgr.GetCurrStructIndex()]->FeaPartIsFixPoint( i ) )
                            {
                                num_fix_pnts_prior++;
                            }
                        }

                        if ( structVec[StructureMgr.GetCurrStructIndex()]->ValidFeaPartInd( parent_index - num_fix_pnts_prior ) )
                        {
                            m_FixPointParentSurfChoice.SetVal( parent_index - num_fix_pnts_prior );
                        }
                    }
                    else
                    {
                        // Set skin as parent surface if undefined
                        if ( structVec[StructureMgr.GetCurrStructIndex()]->GetFeaSkin() )
                        {
                            fixpt->m_ParentFeaPartID = structVec[StructureMgr.GetCurrStructIndex()]->GetFeaSkin()->GetID();
                        }
                    }
                }
            }
        }
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
        if ( StructureMgr.ValidTotalFeaStructInd( StructureMgr.GetCurrStructIndex() ) )
        {
            FeaStructure* curr_struct = StructureMgr.GetAllFeaStructs()[StructureMgr.GetCurrStructIndex()];

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
    m_FixPointEditLayout.Hide();
    m_DomeEditLayout.Hide();
    m_RibArrayEditLayout.Hide();
    m_SliceArrayEditLayout.Hide();

    m_FeaSSLineGroup.Hide();
    m_FeaSSRecGroup.Hide();
    m_FeaSSEllGroup.Hide();
    m_FeaSSConGroup.Hide();
    m_FeaSSLineArrayGroup.Hide();

    m_CurFeaPartDispGroup = group;

    if ( group )
    {
        group->Show();
    }
}

void FeaPartEditScreen::SetFeaPartPropertyIndex( Choice* property_choice )
{
    if ( StructureMgr.ValidTotalFeaStructInd( StructureMgr.GetCurrStructIndex() ) )
    {
        vector < FeaStructure* > structvec = StructureMgr.GetAllFeaStructs();

        if ( StructureMgr.GetCurrPartIndex() < structvec[StructureMgr.GetCurrStructIndex()]->NumFeaParts() )
        {
            FeaPart* feaprt = structvec[StructureMgr.GetCurrStructIndex()]->GetFeaPart( StructureMgr.GetCurrPartIndex() );

            if ( feaprt )
            {
                feaprt->m_FeaPropertyIndex.Set( property_choice->GetVal() );
            }
        }
        else if ( StructureMgr.GetCurrPartIndex() >= structvec[StructureMgr.GetCurrStructIndex()]->NumFeaParts() )
        {
            SubSurface* subsurf = structvec[StructureMgr.GetCurrStructIndex()]->GetFeaSubSurf( StructureMgr.GetCurrPartIndex() - structvec[StructureMgr.GetCurrStructIndex()]->NumFeaParts() );

            if ( subsurf )
            {
                subsurf->m_FeaPropertyIndex.Set( property_choice->GetVal() );
            }
        }
    }
}

void FeaPartEditScreen::SetCapPropertyIndex( Choice* property_choice )
{
    if ( StructureMgr.ValidTotalFeaStructInd( StructureMgr.GetCurrStructIndex() ) )
    {
        vector < FeaStructure* > structvec = StructureMgr.GetAllFeaStructs();

        if ( StructureMgr.GetCurrPartIndex() < structvec[StructureMgr.GetCurrStructIndex()]->NumFeaParts() )
        {
            FeaPart* feaprt = structvec[StructureMgr.GetCurrStructIndex()]->GetFeaPart( StructureMgr.GetCurrPartIndex() );

            if ( feaprt )
            {
                feaprt->m_CapFeaPropertyIndex.Set( property_choice->GetVal() );
            }
        }
        else if ( StructureMgr.GetCurrPartIndex() >= structvec[StructureMgr.GetCurrStructIndex()]->NumFeaParts() )
        {
            SubSurface* subsurf = structvec[StructureMgr.GetCurrStructIndex()]->GetFeaSubSurf( StructureMgr.GetCurrPartIndex() - structvec[StructureMgr.GetCurrStructIndex()]->NumFeaParts() );

            if ( subsurf )
            {
                subsurf->m_CapFeaPropertyIndex.Set( property_choice->GetVal() );
            }
        }
    }
}

void FeaPartEditScreen::UpdateUnitLabels()
{
    Vehicle* veh = m_ScreenMgr->GetVehiclePtr();

    if ( veh )
    {
        string mass_unit, dist_unit;
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
        mass_unit = "lbf*sec" + squared + "/in";
        dist_unit = "in";
        break;
        }

        m_FixPointMassUnit.GetFlButton()->copy_label( mass_unit.c_str() );

        m_DomeARadUnit.GetFlButton()->copy_label( dist_unit.c_str() );
        m_DomeBRadUnit.GetFlButton()->copy_label( dist_unit.c_str() );
        m_DomeCRadUnit.GetFlButton()->copy_label( dist_unit.c_str() );
        m_DomeXUnit.GetFlButton()->copy_label( dist_unit.c_str() );
        m_DomeYUnit.GetFlButton()->copy_label( dist_unit.c_str() );
        m_DomeZUnit.GetFlButton()->copy_label( dist_unit.c_str() );

        if ( StructureMgr.ValidTotalFeaStructInd( StructureMgr.GetCurrStructIndex() ) )
        {
            vector< FeaStructure* > structVec = StructureMgr.GetAllFeaStructs();

            if ( StructureMgr.GetCurrPartIndex() < structVec[StructureMgr.GetCurrStructIndex()]->NumFeaParts() )
            {
                FeaPart* feaprt = structVec[StructureMgr.GetCurrStructIndex()]->GetFeaPart( StructureMgr.GetCurrPartIndex() );
                if ( feaprt )
                {
                    if ( feaprt->GetType() == vsp::FEA_SLICE || feaprt->GetType() == vsp::FEA_RIB || feaprt->GetType() == vsp::FEA_SPAR || feaprt->GetType() == vsp::FEA_RIB_ARRAY || feaprt->GetType() == vsp::FEA_SLICE_ARRAY ) // TODO: Switch to different check
                    {
                        m_SlicePosUnit.GetFlButton()->copy_label( dist_unit.c_str() );
                        m_SparPosUnit.GetFlButton()->copy_label( dist_unit.c_str() );
                        m_RibPosUnit.GetFlButton()->copy_label( dist_unit.c_str() );
                        m_RibArrayStartLocUnit.GetFlButton()->copy_label( dist_unit.c_str() );
                        m_RibArrayEndLocUnit.GetFlButton()->copy_label( dist_unit.c_str() );
                        m_RibArrayPosUnit.GetFlButton()->copy_label( dist_unit.c_str() );
                        m_SliceArrayStartLocUnit.GetFlButton()->copy_label( dist_unit.c_str() );
                        m_SliceArrayEndLocUnit.GetFlButton()->copy_label( dist_unit.c_str() );
                        m_SliceArrayPosUnit.GetFlButton()->copy_label( dist_unit.c_str() );

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