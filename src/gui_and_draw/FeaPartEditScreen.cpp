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

FeaPartEditScreen::FeaPartEditScreen( ScreenMgr* mgr ) : BasicScreen( mgr, 340, 475, "FEA Part Edit" )
{
    m_FLTK_Window->callback( staticCloseCB, this );

    m_CurFeaPartDispGroup = NULL;

    //=== FEA Parts ===//
    m_GenLayout.SetGroupAndScreen( m_FLTK_Window, this );

    m_GenLayout.AddY( m_GenLayout.GetStdHeight() + m_GenLayout.GetGapHeight() );

    m_GenLayout.AddInput( m_FeaPartNameInput, "Part Name" );

    m_GenLayout.AddYGap();

    m_GenLayout.AddDividerBox( "Elements" );

    m_GenLayout.SetSameLineFlag( true );
    m_GenLayout.SetFitWidthFlag( false );

    m_GenLayout.SetButtonWidth( m_GenLayout.GetRemainX() / 3 );
    m_GenLayout.AddButton( m_ShellToggle, "Shell" );
    m_GenLayout.AddButton( m_CapToggle, "Cap" );
    m_GenLayout.AddButton( m_ShellCapToggle, "Shell and Cap" );

    m_ShellCapToggleGroup.Init( this );
    m_ShellCapToggleGroup.AddButton( m_ShellToggle.GetFlButton() );
    m_ShellCapToggleGroup.AddButton( m_CapToggle.GetFlButton() );
    m_ShellCapToggleGroup.AddButton( m_ShellCapToggle.GetFlButton() );

    m_GenLayout.SetSameLineFlag( false );
    m_GenLayout.SetFitWidthFlag( true );
    m_GenLayout.ForceNewLine();
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

    m_GenLayout.AddChoice( m_OrientationChoice, "Orientation" );
    m_GenLayout.AddChoice( m_CapPropertyChoice, "Cap Property" );

    m_GenLayout.AddYGap();

    // Individual FeaPart Parameters:
    int start_y = m_GenLayout.GetY();

    //==== FeaSkin ====//
    m_GenLayout.AddSubGroupLayout( m_SkinEditLayout, m_GenLayout.GetRemainX(), m_GenLayout.GetRemainY() );
    m_SkinEditLayout.SetY( start_y );

    m_SkinEditLayout.AddDividerBox( "Skin" );

    m_SkinEditLayout.AddButton( m_RemoveSkinToggle, "Remove Skin" );

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

        UpdateTrimPartChoice();

        //===== FeaProperty Update =====//
        UpdateFeaPropertyChoice();

        UpdateUnitLabels();

        // Activate by default, only need to de-activate in following code.
        m_ShellPropertyChoice.Activate();
        m_OrientationChoice.Activate();
        m_CapPropertyChoice.Activate();
        m_ShellCapToggleGroup.Activate();

        if ( StructureMgr.ValidTotalFeaStructInd( StructureMgr.GetCurrStructIndex() ) )
        {
            vector< FeaStructure* > structVec = StructureMgr.GetAllFeaStructs();


            // Update Current FeaPart
            if ( StructureMgr.GetCurrPartIndex() < structVec[StructureMgr.GetCurrStructIndex()]->NumFeaParts() )
            {
                FeaPart* feaprt = structVec[StructureMgr.GetCurrStructIndex()]->GetFeaPart( StructureMgr.GetCurrPartIndex() );
                if ( feaprt )
                {
                    m_FeaPartNameInput.Update( feaprt->GetName() );

                    if ( feaprt->m_IncludedElements() == vsp::FEA_SHELL )
                    {
                        m_CapPropertyChoice.Deactivate();
                    }
                    else if ( feaprt->m_IncludedElements() == vsp::FEA_BEAM )
                    {
                        m_ShellPropertyChoice.Deactivate();
                        m_OrientationChoice.Deactivate();
                    }

                    if ( feaprt->GetType() == vsp::FEA_SKIN )
                    {
                        FeaSkin* skin = dynamic_cast<FeaSkin*>( feaprt );
                        assert( skin );

                        m_RemoveSkinToggle.Update( skin->m_RemoveSkinFlag.GetID() );

                        m_ShellCapToggleGroup.Deactivate();
                        m_CapPropertyChoice.Deactivate();

                        if ( skin->m_RemoveSkinFlag() )
                        {
                            m_ShellPropertyChoice.Deactivate();
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
                        m_ShellCapToggleGroup.Update( slice->m_IncludedElements.GetID() );

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
                        m_ShellCapToggleGroup.Update( rib->m_IncludedElements.GetID() );

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
                        m_ShellCapToggleGroup.Update( spar->m_IncludedElements.GetID() );

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

                        m_ShellPropertyChoice.Deactivate();
                        m_OrientationChoice.Deactivate();
                        m_CapPropertyChoice.Deactivate();
                        m_ShellCapToggleGroup.Deactivate();

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

                        m_ShellCapToggleGroup.Update( dome->m_IncludedElements.GetID() );

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
                        m_ShellCapToggleGroup.Update( rib_array->m_IncludedElements.GetID() );
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
                        m_ShellCapToggleGroup.Update( slice_array->m_IncludedElements.GetID() );

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
                        m_ShellCapToggleGroup.Deactivate();

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
            else if ( StructureMgr.GetCurrPartIndex() >= structVec[StructureMgr.GetCurrStructIndex()]->NumFeaParts() )
            {
                //===== SubSurface =====//
                SubSurface* subsurf = structVec[StructureMgr.GetCurrStructIndex()]->GetFeaSubSurf( StructureMgr.GetCurrPartIndex() - structVec[StructureMgr.GetCurrStructIndex()]->NumFeaParts() );
                if ( subsurf )
                {
                    m_FeaPartNameInput.Update( subsurf->GetName() );

                    if ( subsurf->m_IncludedElements() == vsp::FEA_BEAM )
                    {
                        m_ShellPropertyChoice.Deactivate();
                        m_OrientationChoice.Deactivate();
                    }
                    else if ( subsurf->m_IncludedElements() == vsp::FEA_SHELL )
                    {
                        m_CapPropertyChoice.Deactivate();
                    }

                    if ( subsurf->GetType() == vsp::SS_LINE )
                    {
                        SSLine* ssline = dynamic_cast<SSLine*>( subsurf );
                        assert( ssline );

                        m_FeaSSLineConstToggleGroup.Update( ssline->m_ConstType.GetID() );
                        m_FeaSSLineTestToggleGroup.Update( ssline->m_TestType.GetID() );
                        m_FeaSSLineConstSlider.Update( ssline->m_ConstVal.GetID() );
                        m_ShellCapToggleGroup.Update( ssline->m_IncludedElements.GetID() );

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
                        m_ShellCapToggleGroup.Update( ssrec->m_IncludedElements.GetID() );

                        if ( ssrec->m_IncludedElements() == vsp::FEA_BEAM )
                        {
                            m_FeaSSRecTestToggleGroup.Deactivate();
                            ssrec->m_TestType.Set( vsp::INSIDE ); // Inside tris must be set to be removed. Skin tris are saved/removed using FeaSkin parms
                        }
                        else if ( ssrec->m_IncludedElements() == vsp::FEA_SHELL )
                        {
                            m_FeaSSRecTestToggleGroup.Activate();
                        }
                        else if ( ssrec->m_IncludedElements() == vsp::FEA_SHELL_AND_BEAM )
                        {
                            m_FeaSSRecTestToggleGroup.Activate();
                        }

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
                        m_ShellCapToggleGroup.Update( ssell->m_IncludedElements.GetID() );

                        if ( ssell->m_IncludedElements() == vsp::FEA_BEAM )
                        {
                            m_FeaSSEllTestToggleGroup.Deactivate();
                            ssell->m_TestType.Set( vsp::INSIDE ); // Inside tris must be set to be removed. Skin tris are saved/removed using FeaSkin parms
                        }
                        else if ( ssell->m_IncludedElements() == vsp::FEA_SHELL )
                        {
                            m_FeaSSEllTestToggleGroup.Activate();
                        }
                        else if ( ssell->m_IncludedElements() == vsp::FEA_SHELL_AND_BEAM )
                        {
                            m_FeaSSEllTestToggleGroup.Activate();
                        }

                        if ( ssell->m_TestType() == vsp::NONE )
                        {
                            m_ShellPropertyChoice.Deactivate();
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
                        m_ShellCapToggleGroup.Update( sscon->m_IncludedElements.GetID() );
                        m_FeaSSConSAngleButton.Update( sscon->m_StartAngleFlag.GetID() );
                        m_FeaSSConEAngleButton.Update( sscon->m_EndAngleFlag.GetID() );
                        m_FeaSSConSAngleSlider.Update( sscon->m_StartAngle.GetID() );
                        m_FeaSSConEAngleSlider.Update( sscon->m_EndAngle.GetID() );

                        m_FeaSSConTessSlider.Update( sscon->m_Tess.GetID() );

                        if ( sscon->m_IncludedElements() == vsp::FEA_BEAM )
                        {
                            m_FeaSSConTestToggleGroup.Deactivate();
                            sscon->m_TestType.Set( vsp::INSIDE ); // Inside tris must be set to be removed. Skin tris are saved/removed using FeaSkin parms
                        }
                        else if ( sscon->m_IncludedElements() == vsp::FEA_SHELL )
                        {
                            m_FeaSSConTestToggleGroup.Activate();
                        }
                        else if ( sscon->m_IncludedElements() == vsp::FEA_SHELL_AND_BEAM )
                        {
                            m_FeaSSConTestToggleGroup.Activate();
                        }

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

                        m_ShellPropertyChoice.Deactivate();
                        m_OrientationChoice.Deactivate();
                        m_ShellCapToggleGroup.Deactivate();

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

                        m_ShellPropertyChoice.Deactivate();
                        m_OrientationChoice.Deactivate();
                        m_ShellCapToggleGroup.Deactivate();

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

    if ( StructureMgr.ValidTotalFeaStructInd( StructureMgr.GetCurrStructIndex() ) )
    {
        vector < FeaStructure* > structvec = StructureMgr.GetAllFeaStructs();
        FeaStructure *structure = structvec[ StructureMgr.GetCurrStructIndex() ];

        FeaPart* feaprt = NULL;
        SubSurface* subsurf = NULL;

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
        if ( StructureMgr.GetCurrPartIndex() < structVec[StructureMgr.GetCurrStructIndex()]->NumFeaParts() )
        {
            FeaPart* feaprt = structVec[StructureMgr.GetCurrStructIndex()]->GetFeaPart( StructureMgr.GetCurrPartIndex() );

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

    m_ScreenMgr->SetUpdateFlag( true );
}

void FeaPartEditScreen::UpdateFeaPropertyChoice()
{
    //==== Property Choice ====//
    m_ShellPropertyChoice.ClearItems();
    m_CapPropertyChoice.ClearItems();

    Vehicle*  veh = m_ScreenMgr->GetVehiclePtr();

    if ( veh )
    {
        vector< FeaProperty* > property_vec = StructureMgr.GetFeaPropertyVec();

        for ( int i = 0; i < property_vec.size(); ++i )
        {
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

        if ( StructureMgr.ValidTotalFeaStructInd( StructureMgr.GetCurrStructIndex() ) )
        {
            vector < FeaStructure* > structvec = StructureMgr.GetAllFeaStructs();

            if ( StructureMgr.GetCurrPartIndex() < structvec[StructureMgr.GetCurrStructIndex()]->NumFeaParts() )
            {
                FeaPart* feaprt = structvec[StructureMgr.GetCurrStructIndex()]->GetFeaPart( StructureMgr.GetCurrPartIndex() );

                if ( feaprt )
                {
                    // Update all FeaPart Property Choices ( Only Selected Part Visible )
                    m_ShellPropertyChoice.Update( feaprt->m_FeaPropertyIndex.GetID() );
                    m_OrientationChoice.Update( feaprt->m_OrientationType.GetID() );
                    m_CapPropertyChoice.Update( feaprt->m_CapFeaPropertyIndex.GetID() );

                }
            }
            else if ( StructureMgr.GetCurrPartIndex() >= structvec[StructureMgr.GetCurrStructIndex()]->NumFeaParts() )
            {
                SubSurface* subsurf = structvec[StructureMgr.GetCurrStructIndex()]->GetFeaSubSurf( StructureMgr.GetCurrPartIndex() - structvec[StructureMgr.GetCurrStructIndex()]->NumFeaParts() );

                if ( subsurf )
                {
                    // Update all FeaSubSurface Property Choices ( Only Selected Part Visible )
                    m_ShellPropertyChoice.Update( subsurf->m_FeaPropertyIndex.GetID() );
                    m_OrientationChoice.Update( subsurf->m_FeaOrientationType.GetID() );
                    m_CapPropertyChoice.Update( subsurf->m_CapFeaPropertyIndex.GetID() );
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

void FeaPartEditScreen::UpdateTrimPartChoice()
{
    //==== FixPoint Parent Surf Choice ====//
    m_TrimPartChoice.ClearItems();
    m_TrimPartChoiceIDVec.clear();

    int istruct = StructureMgr.GetCurrStructIndex();

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
        int v_pos = m_TrimPartBrowser->position();

        m_TrimPartBrowser->clear();
        m_TrimPartBrowser->column_char( ':' );
        char str[256];
        sprintf( str, "@b@.PART:@b@.FLIPDIR" );
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

                            sprintf( str, "%s:%s", trim_feaprt->GetName().c_str(), flagstr.c_str());
                            m_TrimPartBrowser->add( str );
                        }
                        else
                        {
                            sprintf( str, "Unset: " );
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
        m_TrimPartBrowser->position( v_pos );


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
    m_TrimEditLayout.Hide();

    m_FeaSSLineGroup.Hide();
    m_FeaSSRecGroup.Hide();
    m_FeaSSEllGroup.Hide();
    m_FeaSSConGroup.Hide();
    m_FeaSSLineArrayGroup.Hide();
    m_FeaSSFLineGroup.Hide();

    m_CurFeaPartDispGroup = group;

    if ( group )
    {
        group->Show();
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