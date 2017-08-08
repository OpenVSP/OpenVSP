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
#include "SubSurface.h"
#include "FeaMeshMgr.h"

FeaPartEditScreen::FeaPartEditScreen( ScreenMgr* mgr ) : BasicScreen( mgr, 310, 365, "FEA Part Edit" )
{
    m_FLTK_Window->callback( staticCloseCB, this );

    //=== FEA Parts ===//
    m_GenLayout.SetGroupAndScreen( m_FLTK_Window, this );

    m_GenLayout.AddY( m_GenLayout.GetStdHeight() + m_GenLayout.GetGapHeight() );

    m_GenLayout.AddInput( m_FeaPartNameInput, "Part Name" );

    m_GenLayout.AddYGap();

    m_CurFeaPartDispGroup = NULL;

    // Indivdual FeaPart Parameters:
    int start_y = m_GenLayout.GetY();

    //==== FeaSlice ====//
    m_GenLayout.AddSubGroupLayout( m_SkinEditLayout, m_GenLayout.GetW(), m_GenLayout.GetH() );
    m_SkinEditLayout.SetY( start_y );

    m_SkinEditLayout.AddDividerBox( "Skin" );

    m_SkinEditLayout.AddButton( m_RemoveSkinTrisToggle, "Remove Skin Tris" );

    m_SkinEditLayout.AddYGap();

    m_SkinEditLayout.AddChoice( m_SkinPropertyChoice, "Property" );

    //==== FeaSlice ====//
    m_GenLayout.AddSubGroupLayout( m_SliceEditLayout, m_GenLayout.GetW(), m_GenLayout.GetH() );
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
    m_SliceEditLayout.AddChoice( m_SliceOrientationChoice, "Orientation" );

    m_SliceEditLayout.SetSameLineFlag( true );
    m_SliceEditLayout.SetFitWidthFlag( false );

    int button_width = m_SliceEditLayout.GetButtonWidth();
    int slider_width = m_SliceEditLayout.GetSliderWidth();

    m_SliceEditLayout.SetSliderWidth( button_width );
    m_SliceEditLayout.SetButtonWidth( 0 );
    m_SliceEditLayout.SetChoiceButtonWidth( 0 );

    m_SlicePosTypeChoice.AddItem( "Percent" );
    m_SlicePosTypeChoice.AddItem( "Distance" );
    m_SliceEditLayout.AddChoice( m_SlicePosTypeChoice, "" );

    m_SliceEditLayout.SetSliderWidth( slider_width - 5 );

    m_SliceEditLayout.AddSlider( m_SliceCenterLocSlider, " ", 50, "%5.3f" );

    m_SliceEditLayout.AddButton( m_SlicePosUnit, "" );
    m_SlicePosUnit.GetFlButton()->box( FL_THIN_UP_BOX );
    m_SlicePosUnit.GetFlButton()->labelcolor( FL_BLACK );
    m_SlicePosUnit.SetWidth( m_SliceEditLayout.GetRemainX() );

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

    m_SliceEditLayout.AddChoice( m_SlicePropertyChoice, "Property" );
    m_SliceEditLayout.AddChoice( m_SliceCapPropertyChoice, "Cap Property" );

    //==== FeaRib ====//
    m_GenLayout.AddSubGroupLayout( m_RibEditLayout, m_GenLayout.GetW(), m_GenLayout.GetH() );
    m_RibEditLayout.SetY( start_y );

    m_RibEditLayout.AddDividerBox( "Rib" );

    m_RibEditLayout.SetButtonWidth( m_RibEditLayout.GetRemainX() / 3 );
    m_RibEditLayout.SetChoiceButtonWidth( m_RibEditLayout.GetRemainX() / 3 );

    m_RibEditLayout.AddChoice( m_RibPerpendicularEdgeChoice, "Edge Normal" );

    m_RibEditLayout.SetSameLineFlag( true );
    m_RibEditLayout.SetFitWidthFlag( false );

    button_width = m_RibEditLayout.GetButtonWidth();
    slider_width = m_RibEditLayout.GetSliderWidth();

    m_RibEditLayout.SetSliderWidth( button_width );
    m_RibEditLayout.SetButtonWidth( 0 );
    m_RibEditLayout.SetChoiceButtonWidth( 0 );

    m_RibPosTypeChoice.AddItem( "% Span" );
    m_RibPosTypeChoice.AddItem( "Dist Span" );
    m_RibEditLayout.AddChoice( m_RibPosTypeChoice, "" );

    m_RibEditLayout.SetSliderWidth( slider_width - 5 );

    m_RibEditLayout.AddSlider( m_RibPosSlider, " ", 50, "%5.3f" );

    m_RibEditLayout.AddButton( m_RibPosUnit, "" );
    m_RibPosUnit.GetFlButton()->box( FL_THIN_UP_BOX );
    m_RibPosUnit.GetFlButton()->labelcolor( FL_BLACK );
    m_RibPosUnit.SetWidth( m_RibEditLayout.GetRemainX() );

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

    m_RibEditLayout.AddChoice( m_RibPropertyChoice, "Property" );
    m_RibEditLayout.AddChoice( m_RibCapPropertyChoice, "Cap Property" );

    //==== FeaSpar ====//
    m_GenLayout.AddSubGroupLayout( m_SparEditLayout, m_GenLayout.GetW(), m_GenLayout.GetH() );
    m_SparEditLayout.SetY( start_y );

    m_SparEditLayout.AddDividerBox( "Spar" );

    m_SparEditLayout.SetButtonWidth( m_SparEditLayout.GetRemainX() / 2 );

    int input_width = m_SparEditLayout.GetInputWidth();

    m_SparEditLayout.SetSameLineFlag( true );
    m_SparEditLayout.SetFitWidthFlag( false );

    m_SparEditLayout.AddButton( m_SparSectionLimitToggle, "Limit Spar to Section" );

    m_SparEditLayout.SetButtonWidth( m_SparEditLayout.GetRemainX() / 4 );
    m_SparEditLayout.SetInputWidth( m_SparEditLayout.GetRemainX() / 5 );

    m_SparEditLayout.AddIndexSelector( m_SparSectIndexSelector );

    m_SparEditLayout.ForceNewLine();

    button_width = m_SparEditLayout.GetRemainX() / 3;
    slider_width = m_SparEditLayout.GetSliderWidth();

    m_SparEditLayout.SetSliderWidth( button_width );
    m_SparEditLayout.SetInputWidth( input_width );
    m_SparEditLayout.SetButtonWidth( 0 );
    m_SparEditLayout.SetChoiceButtonWidth( 0 );

    m_SparPosTypeChoice.AddItem( "% Chord" );
    m_SparPosTypeChoice.AddItem( "Dist Chord" );
    m_SparEditLayout.AddChoice( m_SparPosTypeChoice, "" );

    m_SparEditLayout.SetSliderWidth( slider_width - 5 );

    m_SparEditLayout.AddSlider( m_SparPosSlider, " ", 50, "%5.3f" );

    m_SparEditLayout.AddButton( m_SparPosUnit, "" );
    m_SparPosUnit.GetFlButton()->box( FL_THIN_UP_BOX );
    m_SparPosUnit.GetFlButton()->labelcolor( FL_BLACK );
    m_SparPosUnit.SetWidth( m_SparEditLayout.GetRemainX() );

    m_SparEditLayout.ForceNewLine();
    m_SparEditLayout.AddYGap();

    m_SparEditLayout.InitWidthHeightVals();
    m_SparEditLayout.SetSameLineFlag( false );
    m_SparEditLayout.SetFitWidthFlag( true );

    m_SparEditLayout.SetButtonWidth( button_width );

    m_SparEditLayout.AddSlider( m_SparThetaSlider, "Rotation", 25, "%5.3f" );

    m_SparEditLayout.AddYGap();

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

    m_SparEditLayout.AddChoice( m_SparPropertyChoice, "Property" );
    m_SparEditLayout.AddChoice( m_SparCapPropertyChoice, "Cap Property" );

    //==== FeaFixPoint ====//
    m_GenLayout.AddSubGroupLayout( m_FixPointEditLayout, m_GenLayout.GetW(), m_GenLayout.GetH() );
    m_FixPointEditLayout.SetY( start_y );

    m_FixPointEditLayout.AddDividerBox( "Fixed Point" );

    m_FixPointEditLayout.SetChoiceButtonWidth( m_FixPointEditLayout.GetRemainX() / 3 );
    m_FixPointEditLayout.SetButtonWidth( m_FixPointEditLayout.GetRemainX() / 3 );

    m_FixPointEditLayout.AddChoice( m_FixPointParentSurfChoice, "Parent Surface" );

    m_FixPointEditLayout.AddSlider( m_FixPointULocSlider, "U Location", 1, "%5.3f" );
    m_FixPointEditLayout.AddSlider( m_FixPointWLocSlider, "W Location", 1, "%5.3f" );

    m_FixPointEditLayout.AddYGap();

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

    //==== FeaBulkhead ====//
    m_GenLayout.AddSubGroupLayout( m_BulkheadEditLayout, m_GenLayout.GetW(), m_GenLayout.GetH() );
    m_BulkheadEditLayout.SetY( start_y );

    m_BulkheadEditLayout.AddDividerBox( "Bulkhead" );

    m_BulkheadEditLayout.SetSameLineFlag( true );
    m_BulkheadEditLayout.SetFitWidthFlag( false );

    m_BulkheadEditLayout.SetButtonWidth( m_BulkheadEditLayout.GetRemainX() / 3 );
    m_BulkheadEditLayout.SetSliderWidth( m_BulkheadEditLayout.GetSliderWidth() - 5 );

    m_BulkheadEditLayout.AddSlider( m_BHARadSlider, "A Radius", 10, "%5.3f" );

    m_BulkheadEditLayout.AddButton( m_BHARadUnit, " " );
    m_BHARadUnit.GetFlButton()->box( FL_THIN_UP_BOX );
    m_BHARadUnit.GetFlButton()->labelcolor( FL_BLACK );
    m_BHARadUnit.SetWidth( m_BulkheadEditLayout.GetW() / 14 );

    m_BulkheadEditLayout.ForceNewLine();

    m_BulkheadEditLayout.AddSlider( m_BHBRadSlider, "B Radius", 10, "%5.3f" );

    m_BulkheadEditLayout.AddButton( m_BHBRadUnit, " " );
    m_BHBRadUnit.GetFlButton()->box( FL_THIN_UP_BOX );
    m_BHBRadUnit.GetFlButton()->labelcolor( FL_BLACK );
    m_BHBRadUnit.SetWidth( m_BulkheadEditLayout.GetW() / 14 );

    m_BulkheadEditLayout.ForceNewLine();

    m_BulkheadEditLayout.AddSlider( m_BHCRadSlider, "C Radius", 10, "%5.3f" );

    m_BulkheadEditLayout.AddButton( m_BHCRadUnit, " " );
    m_BHCRadUnit.GetFlButton()->box( FL_THIN_UP_BOX );
    m_BHCRadUnit.GetFlButton()->labelcolor( FL_BLACK );
    m_BHCRadUnit.SetWidth( m_BulkheadEditLayout.GetW() / 14 );

    m_BulkheadEditLayout.ForceNewLine();
    m_BulkheadEditLayout.AddYGap();

    m_BulkheadEditLayout.AddSlider( m_BHXSlider, "XLoc", 50, "%5.3f" );

    m_BulkheadEditLayout.AddButton( m_BHXUnit, " " );
    m_BHXUnit.GetFlButton()->box( FL_THIN_UP_BOX );
    m_BHXUnit.GetFlButton()->labelcolor( FL_BLACK );
    m_BHXUnit.SetWidth( m_BulkheadEditLayout.GetW() / 14 );

    m_BulkheadEditLayout.ForceNewLine();

    m_BulkheadEditLayout.AddSlider( m_BHYSlider, "YLoc", 50, "%5.3f" );

    m_BulkheadEditLayout.AddButton( m_BHYUnit, " " );
    m_BHYUnit.GetFlButton()->box( FL_THIN_UP_BOX );
    m_BHYUnit.GetFlButton()->labelcolor( FL_BLACK );
    m_BHYUnit.SetWidth( m_BulkheadEditLayout.GetW() / 14 );

    m_BulkheadEditLayout.ForceNewLine();

    m_BulkheadEditLayout.AddSlider( m_BHZSlider, "ZLoc", 50, "%5.3f" );

    m_BulkheadEditLayout.AddButton( m_BHZUnit, " " );
    m_BHZUnit.GetFlButton()->box( FL_THIN_UP_BOX );
    m_BHZUnit.GetFlButton()->labelcolor( FL_BLACK );
    m_BHZUnit.SetWidth( m_BulkheadEditLayout.GetW() / 14 );

    m_BulkheadEditLayout.ForceNewLine();
    m_BulkheadEditLayout.AddYGap();

    m_BulkheadEditLayout.SetSameLineFlag( false );
    m_BulkheadEditLayout.SetFitWidthFlag( true );

    m_BulkheadEditLayout.AddSlider( m_BHXRotSlider, "XRot", 25, "%5.3f" );
    m_BulkheadEditLayout.AddSlider( m_BHYRotSlider, "YRot", 25, "%5.3f" );
    m_BulkheadEditLayout.AddSlider( m_BHZRotSlider, "ZRot", 25, "%5.3f" );

    m_BulkheadEditLayout.AddYGap();

    m_BulkheadEditLayout.SetSameLineFlag( true );
    m_BulkheadEditLayout.SetFitWidthFlag( false );

    m_BulkheadEditLayout.SetButtonWidth( m_BulkheadEditLayout.GetRemainX() / 3 );
    m_BulkheadEditLayout.AddButton( m_BulkheadShellToggle, "Shell" );
    m_BulkheadEditLayout.AddButton( m_BulkheadCapToggle, "Cap" );
    m_BulkheadEditLayout.AddButton( m_BulkheadShellCapToggle, "Shell and Cap" );

    m_BulkheadShellCapToggleGroup.Init( this );
    m_BulkheadShellCapToggleGroup.AddButton( m_BulkheadShellToggle.GetFlButton() );
    m_BulkheadShellCapToggleGroup.AddButton( m_BulkheadCapToggle.GetFlButton() );
    m_BulkheadShellCapToggleGroup.AddButton( m_BulkheadShellCapToggle.GetFlButton() );

    m_BulkheadEditLayout.SetSameLineFlag( false );
    m_BulkheadEditLayout.SetFitWidthFlag( true );
    m_BulkheadEditLayout.ForceNewLine();

    m_BulkheadEditLayout.AddChoice( m_BHPropertyChoice, "Property" );
    m_BulkheadEditLayout.AddChoice( m_BHCapPropertyChoice, "Cap Property" );

    //==== FeaRibArray ====//
    m_GenLayout.AddSubGroupLayout( m_RibArrayEditLayout, m_GenLayout.GetW(), m_GenLayout.GetH() );
    m_RibArrayEditLayout.SetY( start_y );

    m_RibArrayEditLayout.AddDividerBox( "RibArray" );

    m_RibArrayEditLayout.AddChoice( m_RibArrayPerpEdgeChoice, "Edge Normal" );

    m_RibArrayEditLayout.AddYGap();

    m_RibArrayPosTypeChoice.AddItem( "% Span" );
    m_RibArrayPosTypeChoice.AddItem( "Dist Span" );
    m_RibArrayEditLayout.AddChoice( m_RibArrayPosTypeChoice, "Parameterization" );

    m_RibArrayEditLayout.SetButtonWidth( m_RibArrayEditLayout.GetRemainX() / 3 );

    m_RibArrayEditLayout.SetSameLineFlag( true );
    m_RibArrayEditLayout.SetFitWidthFlag( false );

    button_width = m_RibArrayEditLayout.GetButtonWidth();
    slider_width = m_RibArrayEditLayout.GetSliderWidth();

    m_RibArrayEditLayout.SetSliderWidth( slider_width - 5 );

    m_RibArrayEditLayout.AddSlider( m_RibArrayStartLocSlider, "Start Location", 50, "%5.3f" );

    m_RibArrayEditLayout.AddButton( m_RibArrayStartLocUnit, " " );
    m_RibArrayStartLocUnit.GetFlButton()->box( FL_THIN_UP_BOX );
    m_RibArrayStartLocUnit.GetFlButton()->labelcolor( FL_BLACK );
    m_RibArrayStartLocUnit.SetWidth( m_RibArrayEditLayout.GetW() / 14 );

    m_RibArrayEditLayout.ForceNewLine();

    m_RibArrayEditLayout.AddSlider( m_RibArraySpacingSlider, "Spacing", 50, "%5.3f" );

    m_RibArrayEditLayout.AddButton( m_RibArrayPosUnit, " " );
    m_RibArrayPosUnit.GetFlButton()->box( FL_THIN_UP_BOX );
    m_RibArrayPosUnit.GetFlButton()->labelcolor( FL_BLACK );
    m_RibArrayPosUnit.SetWidth( m_RibArrayEditLayout.GetW() / 14 );

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

    m_RibArrayEditLayout.AddChoice( m_RibArrayPropertyChoice, "Property" );
    m_RibArrayEditLayout.AddChoice( m_RibArrayCapPropertyChoice, "Cap Property" );

    m_RibArrayEditLayout.AddYGap();

    m_RibArrayEditLayout.AddButton( m_IndividualizeRibArrayButton, "Individualize Rib Array" );

    //=== SubSurfaces ===//

    //==== SSLine ====//
    m_GenLayout.AddSubGroupLayout( m_FeaSSLineGroup, m_GenLayout.GetW(), m_GenLayout.GetH() );
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

    m_FeaSSLineGroup.SetSameLineFlag( true );
    m_FeaSSLineGroup.SetFitWidthFlag( false );

    m_FeaSSLineGroup.SetButtonWidth( m_FeaSSLineGroup.GetRemainX() / 3 );
    m_FeaSSLineGroup.AddButton( m_FeaSSLineShellToggle, "Shell" );
    m_FeaSSLineGroup.AddButton( m_FeaSSLineCapToggle, "Cap" );
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
    m_GenLayout.AddSubGroupLayout( m_FeaSSRecGroup, m_GenLayout.GetW(), m_GenLayout.GetH() );
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

    m_FeaSSRecGroup.SetSameLineFlag( true );
    m_FeaSSRecGroup.SetFitWidthFlag( false );

    m_FeaSSRecGroup.SetButtonWidth( m_FeaSSRecGroup.GetRemainX() / 3 );
    m_FeaSSRecGroup.AddButton( m_FeaSSRecShellToggle, "Shell" );
    m_FeaSSRecGroup.AddButton( m_FeaSSRecCapToggle, "Cap" );
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
    m_GenLayout.AddSubGroupLayout( m_FeaSSEllGroup, m_GenLayout.GetW(), m_GenLayout.GetH() );
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

    m_FeaSSEllGroup.SetSameLineFlag( true );
    m_FeaSSEllGroup.SetFitWidthFlag( false );

    m_FeaSSEllGroup.SetButtonWidth( m_FeaSSEllGroup.GetRemainX() / 3 );
    m_FeaSSEllGroup.AddButton( m_FeaSSEllShellToggle, "Shell" );
    m_FeaSSEllGroup.AddButton( m_FeaSSEllCapToggle, "Cap" );
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
    m_GenLayout.AddSubGroupLayout( m_FeaSSConGroup, m_GenLayout.GetW(), m_GenLayout.GetH() );
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

    m_FeaSSConGroup.SetSameLineFlag( true );
    m_FeaSSConGroup.SetFitWidthFlag( false );

    m_FeaSSConGroup.SetButtonWidth( m_FeaSSConGroup.GetRemainX() / 3 );
    m_FeaSSConGroup.AddButton( m_FeaSSConShellToggle, "Shell" );
    m_FeaSSConGroup.AddButton( m_FeaSSConCapToggle, "Cap" );
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

}

FeaPartEditScreen::~FeaPartEditScreen()
{
}

bool FeaPartEditScreen::Update()
{
    assert( m_ScreenMgr );

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

                        m_RemoveSkinTrisToggle.Update( skin->m_RemoveSkinTrisFlag.GetID() );

                        if ( skin->m_RemoveSkinTrisFlag() )
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
                        m_SlicePosTypeChoice.Update( slice->m_LocationParmType.GetID() );
                        m_SliceCenterLocSlider.Update( slice->m_CenterLocation.GetID() );
                        m_SliceRotAxisToggleGroup.Update( slice->m_RotationAxis.GetID() );
                        m_SliceXRotSlider.Update( slice->m_XRot.GetID() );
                        m_SliceYRotSlider.Update( slice->m_YRot.GetID() );
                        m_SliceZRotSlider.Update( slice->m_ZRot.GetID() );
                        m_SliceShellCapToggleGroup.Update( slice->m_IncludedElements.GetID() );

                        if ( slice->m_RotationAxis() == vsp::X_DIR )
                        {
                            m_SliceXRotSlider.Activate();
                            m_SliceYRotSlider.Deactivate();
                            m_SliceZRotSlider.Deactivate();
                            slice->m_YRot.Set( 0.0 );
                            slice->m_ZRot.Set( 0.0 );
                        }
                        else if ( slice->m_RotationAxis() == vsp::Y_DIR )
                        {
                            m_SliceXRotSlider.Deactivate();
                            m_SliceYRotSlider.Activate();
                            m_SliceZRotSlider.Deactivate();
                            slice->m_XRot.Set( 0.0 );
                            slice->m_ZRot.Set( 0.0 );
                        }
                        else if ( slice->m_RotationAxis() == vsp::Z_DIR )
                        {
                            m_SliceXRotSlider.Deactivate();
                            m_SliceYRotSlider.Deactivate();
                            m_SliceZRotSlider.Activate();
                            slice->m_XRot.Set( 0.0 );
                            slice->m_YRot.Set( 0.0 );
                        }

                        if ( slice->m_IncludedElements() == BOTH_ELEMENTS )
                        {
                            m_SlicePropertyChoice.Activate();
                            m_SliceCapPropertyChoice.Activate();
                        }
                        else if ( slice->m_IncludedElements() == TRIS )
                        {
                            m_SlicePropertyChoice.Activate();
                            m_SliceCapPropertyChoice.Deactivate();
                        }
                        else if ( slice->m_IncludedElements() == BEAM )
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

                        m_RibPosTypeChoice.Update( rib->m_LocationParmType.GetID() );
                        m_RibPosSlider.Update( rib->m_CenterLocation.GetID() );
                        m_RibThetaSlider.Update( rib->m_Theta.GetID() );
                        m_RibShellCapToggleGroup.Update( rib->m_IncludedElements.GetID() );

                        if ( rib->m_IncludedElements() == BOTH_ELEMENTS )
                        {
                            m_RibPropertyChoice.Activate();
                            m_RibCapPropertyChoice.Activate();
                        }
                        else if ( rib->m_IncludedElements() == TRIS )
                        {
                            m_RibPropertyChoice.Activate();
                            m_RibCapPropertyChoice.Deactivate();
                        }
                        else if ( rib->m_IncludedElements() == BEAM )
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

                        m_SparPosTypeChoice.Update( spar->m_LocationParmType.GetID() );
                        m_SparSectionLimitToggle.Update( spar->m_LimitSparToSectionFlag.GetID() );
                        m_SparSectIndexSelector.Update( spar->m_CurrWingSection.GetID() );

                        if ( spar->m_LimitSparToSectionFlag() )
                        {
                            m_SparSectIndexSelector.Activate();
                        }
                        else
                        {
                            m_SparSectIndexSelector.Deactivate();
                        }

                        m_SparPosSlider.Update( spar->m_CenterLocation.GetID() );
                        m_SparThetaSlider.Update( spar->m_Theta.GetID() );
                        m_SparShellCapToggleGroup.Update( spar->m_IncludedElements.GetID() );

                        if ( spar->m_IncludedElements() == BOTH_ELEMENTS )
                        {
                            m_SparPropertyChoice.Activate();
                            m_SparCapPropertyChoice.Activate();
                        }
                        else if ( spar->m_IncludedElements() == TRIS )
                        {
                            m_SparPropertyChoice.Activate();
                            m_SparCapPropertyChoice.Deactivate();
                        }
                        else if ( spar->m_IncludedElements() == BEAM )
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
                    else if ( feaprt->GetType() == vsp::FEA_BULKHEAD )
                    {
                        FeaBulkhead* bulkhead = dynamic_cast<FeaBulkhead*>( feaprt );
                        assert( bulkhead );

                        //m_SlicePosTypeChoice.Update( slice->m_LocationParmType.GetID() );
                        m_BHARadSlider.Update( bulkhead->m_Aradius.GetID() );
                        m_BHBRadSlider.Update( bulkhead->m_Bradius.GetID() );
                        m_BHCRadSlider.Update( bulkhead->m_Cradius.GetID() );

                        m_BHXSlider.Update( bulkhead->m_XLoc.GetID() );
                        m_BHYSlider.Update( bulkhead->m_YLoc.GetID() );
                        m_BHZSlider.Update( bulkhead->m_ZLoc.GetID() );

                        m_BHXRotSlider.Update( bulkhead->m_XRot.GetID() );
                        m_BHYRotSlider.Update( bulkhead->m_YRot.GetID() );
                        m_BHZRotSlider.Update( bulkhead->m_ZRot.GetID() );

                        m_BulkheadShellCapToggleGroup.Update( bulkhead->m_IncludedElements.GetID() );

                        if ( bulkhead->m_IncludedElements() == BOTH_ELEMENTS )
                        {
                            m_BHPropertyChoice.Activate();
                            m_BHCapPropertyChoice.Activate();
                        }
                        else if ( bulkhead->m_IncludedElements() == TRIS )
                        {
                            m_BHPropertyChoice.Activate();
                            m_BHCapPropertyChoice.Deactivate();
                        }
                        else if ( bulkhead->m_IncludedElements() == BEAM )
                        {
                            m_BHPropertyChoice.Deactivate();
                            m_BHCapPropertyChoice.Activate();
                        }

                        FeaPartDispGroup( &m_BulkheadEditLayout );
                    }
                    else if ( feaprt->GetType() == vsp::FEA_RIB_ARRAY )
                    {
                        FeaRibArray* rib_array = dynamic_cast<FeaRibArray*>( feaprt );
                        assert( rib_array );

                        m_RibArrayPosTypeChoice.Update( rib_array->m_LocationParmType.GetID() );
                        m_RibArrayStartLocSlider.Update( rib_array->m_StartLocation.GetID() );
                        m_RibArraySpacingSlider.Update( rib_array->m_RibSpacing.GetID() );
                        m_RibArrayThetaSlider.Update( rib_array->m_Theta.GetID() );
                        m_RibArrayShellCapToggleGroup.Update( rib_array->m_IncludedElements.GetID() );

                        if ( rib_array->m_IncludedElements() == BOTH_ELEMENTS )
                        {
                            m_RibArrayPropertyChoice.Activate();
                            m_RibArrayCapPropertyChoice.Activate();
                        }
                        else if ( rib_array->m_IncludedElements() == TRIS )
                        {
                            m_RibArrayPropertyChoice.Activate();
                            m_RibArrayCapPropertyChoice.Deactivate();
                        }
                        else if ( rib_array->m_IncludedElements() == BEAM )
                        {
                            m_RibArrayPropertyChoice.Deactivate();
                            m_RibArrayCapPropertyChoice.Activate();
                        }

                        FeaPartDispGroup( &m_RibArrayEditLayout );
                    }
                    else
                    {
                        FeaPartDispGroup( NULL );
                    }
                }
                else
                {
                    FeaPartDispGroup( NULL );
                }
            }
            else if ( StructureMgr.GetCurrPartIndex() >= structVec[StructureMgr.GetCurrStructIndex()]->NumFeaParts() )
            {
                //===== SubSurface =====//
                SubSurface* subsurf = structVec[StructureMgr.GetCurrStructIndex()]->GetFeaSubSurf( StructureMgr.GetCurrPartIndex() - structVec[StructureMgr.GetCurrStructIndex()]->NumFeaParts() );
                if ( subsurf )
                {
                    m_FeaPartNameInput.Update( subsurf->GetName() );
                    m_ShellCapToggleGroup.Update( subsurf->m_IncludedElements.GetID() );

                    if ( subsurf->GetType() == vsp::SS_LINE )
                    {
                        SSLine* ssline = dynamic_cast<SSLine*>( subsurf );
                        assert( ssline );

                        m_FeaSSLineConstToggleGroup.Update( ssline->m_ConstType.GetID() );
                        m_FeaSSLineTestToggleGroup.Update( ssline->m_TestType.GetID() );
                        m_FeaSSLineConstSlider.Update( ssline->m_ConstVal.GetID() );
                        m_FeaSSLineShellCapToggleGroup.Update( ssline->m_IncludedElements.GetID() );

                        if ( ssline->m_IncludedElements() == BEAM )
                        {
                            m_FeaSSLineCapPropertyChoice.Activate();
                            m_FeaSSLinePropertyChoice.Deactivate();
                        }
                        else if ( ssline->m_IncludedElements() == TRIS )
                        {
                            m_FeaSSLineCapPropertyChoice.Deactivate();
                            m_FeaSSLinePropertyChoice.Activate();
                        }
                        else if ( ssline->m_IncludedElements() == BOTH_ELEMENTS )
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

                        if ( ssrec->m_IncludedElements() == BEAM )
                        {
                            m_FeaSSRecCapPropertyChoice.Activate();
                            m_FeaSSRecPropertyChoice.Deactivate();
                            m_FeaSSRecTestToggleGroup.Deactivate();
                            ssrec->m_TestType.Set( vsp::INSIDE ); // Inside tris must be set to be removed. Skin tris are saved/removed using FeaSkin parms
                        }
                        else if ( ssrec->m_IncludedElements() == TRIS  )
                        {
                            m_FeaSSRecCapPropertyChoice.Deactivate();
                            m_FeaSSRecPropertyChoice.Activate();
                            m_FeaSSRecTestToggleGroup.Activate();
                        }
                        else if ( ssrec->m_IncludedElements() == BOTH_ELEMENTS )
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

                        if ( ssell->m_IncludedElements() == BEAM )
                        {
                            m_FeaSSEllCapPropertyChoice.Activate();
                            m_FeaSSEllPropertyChoice.Deactivate();
                            m_FeaSSEllTestToggleGroup.Deactivate();
                            ssell->m_TestType.Set( vsp::INSIDE ); // Inside tris must be set to be removed. Skin tris are saved/removed using FeaSkin parms
                        }
                        else if ( ssell->m_IncludedElements() == TRIS )
                        {
                            m_FeaSSEllCapPropertyChoice.Deactivate();
                            m_FeaSSEllPropertyChoice.Activate();
                            m_FeaSSEllTestToggleGroup.Activate();
                        }
                        else if ( ssell->m_IncludedElements() == BOTH_ELEMENTS )
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

                        if ( sscon->m_IncludedElements() == BEAM )
                        {
                            m_FeaSSConCapPropertyChoice.Activate();
                            m_FeaSSConPropertyChoice.Deactivate();
                            m_FeaSSConTestToggleGroup.Deactivate();
                            sscon->m_TestType.Set( vsp::INSIDE ); // Inside tris must be set to be removed. Skin tris are saved/removed using FeaSkin parms
                        }
                        else if ( sscon->m_IncludedElements() == TRIS )
                        {
                            m_FeaSSConCapPropertyChoice.Deactivate();
                            m_FeaSSConPropertyChoice.Activate();
                            m_FeaSSConTestToggleGroup.Activate();
                        }
                        else if ( sscon->m_IncludedElements() == BOTH_ELEMENTS )
                        {
                            m_FeaSSConCapPropertyChoice.Activate();
                            m_FeaSSConPropertyChoice.Activate();
                            m_FeaSSConTestToggleGroup.Activate();
                        }

                        if ( sscon->m_TestType() == vsp::NONE )
                        {
                            m_FeaSSConPropertyChoice.Deactivate();
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
                }
                else
                {
                    FeaPartDispGroup( NULL );
                }
            }

            // Update FeaParts and SubSurfaces if FeaMesh is not in progress
            if ( !FeaMeshMgr.GetFeaMeshInProgress() )
            {
                structVec[StructureMgr.GetCurrStructIndex()]->Update();
            }
        }
        else
        {
            FeaPartDispGroup( NULL );
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
    else if ( device == &m_SkinPropertyChoice )
    {
        UpdateFeaPartPropertyIndex( &m_SkinPropertyChoice );
    }
    else if ( device == &m_SliceOrientationChoice )
    {
        StructureMgr.SetFeaSliceOrientIndex( m_SliceOrientationChoice.GetVal() );
    }
    else if ( device == &m_SlicePropertyChoice )
    {
        UpdateFeaPartPropertyIndex( &m_SlicePropertyChoice );
    }
    else if ( device == &m_SliceCapPropertyChoice )
    {
        UpdateCapPropertyIndex( &m_SliceCapPropertyChoice );
    }
    else if ( device == &m_RibPropertyChoice )
    {
        UpdateFeaPartPropertyIndex( &m_RibPropertyChoice );
    }
    else if ( device == &m_RibCapPropertyChoice )
    {
        UpdateCapPropertyIndex( &m_RibCapPropertyChoice );
    }
    else if ( device == &m_SparPropertyChoice )
    {
        UpdateFeaPartPropertyIndex( &m_SparPropertyChoice );
    }
    else if ( device == &m_SparCapPropertyChoice )
    {
        UpdateCapPropertyIndex( &m_SparCapPropertyChoice );
    }
    else if ( device == &m_BHPropertyChoice )
    {
        UpdateFeaPartPropertyIndex( &m_BHPropertyChoice );
    }
    else if ( device == &m_BHCapPropertyChoice )
    {
        UpdateCapPropertyIndex( &m_BHCapPropertyChoice );
    }
    else if ( device == &m_RibArrayPropertyChoice )
    {
        UpdateFeaPartPropertyIndex( &m_RibArrayPropertyChoice );
    }
    else if ( device == &m_RibArrayCapPropertyChoice )
    {
        UpdateCapPropertyIndex( &m_RibArrayCapPropertyChoice );
    }
    else if ( device == &m_FeaSSLinePropertyChoice )
    {
        UpdateFeaPartPropertyIndex( &m_FeaSSLinePropertyChoice );
    }
    else if ( device == &m_FeaSSRecPropertyChoice )
    {
        UpdateFeaPartPropertyIndex( &m_FeaSSRecPropertyChoice );
    }
    else if ( device == &m_FeaSSEllPropertyChoice )
    {
        UpdateFeaPartPropertyIndex( &m_FeaSSEllPropertyChoice );
    }
    else if ( device == &m_FeaSSConPropertyChoice )
    {
        UpdateFeaPartPropertyIndex( &m_FeaSSConPropertyChoice );
    }
    else if ( device == &m_FeaSSLineCapPropertyChoice )
    {
        UpdateCapPropertyIndex( &m_FeaSSLineCapPropertyChoice );
    }
    else if ( device == &m_FeaSSRecCapPropertyChoice )
    {
        UpdateCapPropertyIndex( &m_FeaSSRecCapPropertyChoice );
    }
    else if ( device == &m_FeaSSEllCapPropertyChoice )
    {
        UpdateCapPropertyIndex( &m_FeaSSEllCapPropertyChoice );
    }
    else if ( device == &m_FeaSSConCapPropertyChoice )
    {
        UpdateCapPropertyIndex( &m_FeaSSConCapPropertyChoice );
    }
    else if ( device == &m_RibPerpendicularEdgeChoice )
    {
        if ( StructureMgr.ValidTotalFeaStructInd( StructureMgr.GetCurrStructIndex() ) )
        {
            vector < FeaStructure* > structVec = StructureMgr.GetAllFeaStructs();

            if ( StructureMgr.GetCurrPartIndex() < structVec[StructureMgr.GetCurrStructIndex()]->NumFeaParts() )
            {
                FeaPart* feaprt = structVec[StructureMgr.GetCurrStructIndex()]->GetFeaPart( StructureMgr.GetCurrPartIndex() );

                if ( feaprt )
                {
                    if ( feaprt->GetType() == vsp::FEA_RIB )
                    {
                        FeaRib* rib = dynamic_cast<FeaRib*>( feaprt );
                        assert( rib );

                        rib->SetPerpendicularEdgeID( m_PerpendicularEdgeVec[m_RibPerpendicularEdgeChoice.GetVal()] );
                    }
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
    else if ( device == &m_RibArrayPerpEdgeChoice )
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
                        FeaRibArray* rib_array = dynamic_cast<FeaRibArray*>( feaprt );
                        assert( rib_array );

                        rib_array->SetPerpendicularEdgeID( m_PerpendicularEdgeVec[m_RibArrayPerpEdgeChoice.GetVal()] );
                    }
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
                        StructureMgr.SetCurrPartIndex( structVec[StructureMgr.GetCurrStructIndex()]->NumFeaParts() - 1 );
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
    m_BHPropertyChoice.ClearItems();
    m_RibArrayPropertyChoice.ClearItems();
    m_FeaSSLinePropertyChoice.ClearItems();
    m_FeaSSRecPropertyChoice.ClearItems();
    m_FeaSSEllPropertyChoice.ClearItems();
    m_FeaSSConPropertyChoice.ClearItems();

    m_SparCapPropertyChoice.ClearItems();
    m_SliceCapPropertyChoice.ClearItems();
    m_RibCapPropertyChoice.ClearItems();
    m_BHCapPropertyChoice.ClearItems();
    m_RibArrayCapPropertyChoice.ClearItems();

    m_FeaSSLineCapPropertyChoice.ClearItems();
    m_FeaSSRecCapPropertyChoice.ClearItems();
    m_FeaSSEllCapPropertyChoice.ClearItems();
    m_FeaSSConCapPropertyChoice.ClearItems();

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
            m_BHPropertyChoice.AddItem( string( property_vec[i]->GetName() ) );
            m_RibArrayPropertyChoice.AddItem( string( property_vec[i]->GetName() ) );
            m_FeaSSLinePropertyChoice.AddItem( string( property_vec[i]->GetName() ) );
            m_FeaSSRecPropertyChoice.AddItem( string( property_vec[i]->GetName() ) );
            m_FeaSSEllPropertyChoice.AddItem( string( property_vec[i]->GetName() ) );
            m_FeaSSConPropertyChoice.AddItem( string( property_vec[i]->GetName() ) );

            m_SparCapPropertyChoice.AddItem( string( property_vec[i]->GetName() ) );
            m_SliceCapPropertyChoice.AddItem( string( property_vec[i]->GetName() ) );
            m_RibCapPropertyChoice.AddItem( string( property_vec[i]->GetName() ) );
            m_BHCapPropertyChoice.AddItem( string( property_vec[i]->GetName() ) );
            m_RibArrayCapPropertyChoice.AddItem( string( property_vec[i]->GetName() ) );

            m_FeaSSLineCapPropertyChoice.AddItem( string( property_vec[i]->GetName() ) );
            m_FeaSSRecCapPropertyChoice.AddItem( string( property_vec[i]->GetName() ) );
            m_FeaSSEllCapPropertyChoice.AddItem( string( property_vec[i]->GetName() ) );
            m_FeaSSConCapPropertyChoice.AddItem( string( property_vec[i]->GetName() ) );

            if ( property_vec[i]->m_FeaPropertyType() == SHELL_PROPERTY )
            {
                m_SkinPropertyChoice.SetFlag( i, 0 );
                m_SlicePropertyChoice.SetFlag( i, 0 );
                m_RibPropertyChoice.SetFlag( i, 0 );
                m_SparPropertyChoice.SetFlag( i, 0 );
                m_BHPropertyChoice.SetFlag( i, 0 );
                m_RibArrayPropertyChoice.SetFlag( i, 0 );
                m_FeaSSLinePropertyChoice.SetFlag( i, 0 );
                m_FeaSSRecPropertyChoice.SetFlag( i, 0 );
                m_FeaSSEllPropertyChoice.SetFlag( i, 0 );
                m_FeaSSConPropertyChoice.SetFlag( i, 0 );

                m_SparCapPropertyChoice.SetFlag( i, FL_MENU_INACTIVE );
                m_SliceCapPropertyChoice.SetFlag( i, FL_MENU_INACTIVE );
                m_RibCapPropertyChoice.SetFlag( i, FL_MENU_INACTIVE );
                m_BHCapPropertyChoice.SetFlag( i, FL_MENU_INACTIVE );
                m_RibArrayCapPropertyChoice.SetFlag( i, FL_MENU_INACTIVE );

                m_FeaSSLineCapPropertyChoice.SetFlag( i, FL_MENU_INACTIVE );
                m_FeaSSRecCapPropertyChoice.SetFlag( i, FL_MENU_INACTIVE );
                m_FeaSSEllCapPropertyChoice.SetFlag( i, FL_MENU_INACTIVE );
                m_FeaSSConCapPropertyChoice.SetFlag( i, FL_MENU_INACTIVE );
            }
            else if ( property_vec[i]->m_FeaPropertyType() == BEAM_PROPERTY )
            {
                m_SkinPropertyChoice.SetFlag( i, FL_MENU_INACTIVE );
                m_SlicePropertyChoice.SetFlag( i, FL_MENU_INACTIVE );
                m_RibPropertyChoice.SetFlag( i, FL_MENU_INACTIVE );
                m_SparPropertyChoice.SetFlag( i, FL_MENU_INACTIVE );
                m_BHPropertyChoice.SetFlag( i, FL_MENU_INACTIVE );
                m_RibArrayPropertyChoice.SetFlag( i, FL_MENU_INACTIVE );
                m_FeaSSLinePropertyChoice.SetFlag( i, FL_MENU_INACTIVE );
                m_FeaSSRecPropertyChoice.SetFlag( i, FL_MENU_INACTIVE );
                m_FeaSSEllPropertyChoice.SetFlag( i, FL_MENU_INACTIVE );
                m_FeaSSConPropertyChoice.SetFlag( i, FL_MENU_INACTIVE );

                m_SparCapPropertyChoice.SetFlag( i, 0 );
                m_SliceCapPropertyChoice.SetFlag( i, 0 );
                m_RibCapPropertyChoice.SetFlag( i, 0 );
                m_BHCapPropertyChoice.SetFlag( i, 0 );
                m_RibArrayCapPropertyChoice.SetFlag( i, 0 );

                m_FeaSSLineCapPropertyChoice.SetFlag( i, 0 );
                m_FeaSSRecCapPropertyChoice.SetFlag( i, 0 );
                m_FeaSSEllCapPropertyChoice.SetFlag( i, 0 );
                m_FeaSSConCapPropertyChoice.SetFlag( i, 0 );
            }
        }

        m_SkinPropertyChoice.UpdateItems();
        m_SlicePropertyChoice.UpdateItems();
        m_RibPropertyChoice.UpdateItems();
        m_SparPropertyChoice.UpdateItems();
        m_BHPropertyChoice.UpdateItems();
        m_RibArrayPropertyChoice.UpdateItems();
        m_FeaSSLinePropertyChoice.UpdateItems();
        m_FeaSSRecPropertyChoice.UpdateItems();
        m_FeaSSEllPropertyChoice.UpdateItems();
        m_FeaSSConPropertyChoice.UpdateItems();

        m_SparCapPropertyChoice.UpdateItems();
        m_SliceCapPropertyChoice.UpdateItems();
        m_RibCapPropertyChoice.UpdateItems();
        m_BHCapPropertyChoice.UpdateItems();
        m_RibArrayCapPropertyChoice.UpdateItems();

        m_FeaSSLineCapPropertyChoice.UpdateItems();
        m_FeaSSRecCapPropertyChoice.UpdateItems();
        m_FeaSSEllCapPropertyChoice.UpdateItems();
        m_FeaSSConCapPropertyChoice.UpdateItems();

        if ( StructureMgr.ValidTotalFeaStructInd( StructureMgr.GetCurrStructIndex() ) )
        {
            vector < FeaStructure* > structvec = StructureMgr.GetAllFeaStructs();

            if ( StructureMgr.GetCurrPartIndex() < structvec[StructureMgr.GetCurrStructIndex()]->NumFeaParts() )
            {
                FeaPart* feaprt = structvec[StructureMgr.GetCurrStructIndex()]->GetFeaPart( StructureMgr.GetCurrPartIndex() );

                if ( feaprt )
                {
                    // Update all FeaPart Property Choices ( Only Selected Part Visible )
                    m_SkinPropertyChoice.SetVal( feaprt->GetFeaPropertyIndex() );
                    m_SlicePropertyChoice.SetVal( feaprt->GetFeaPropertyIndex() );
                    m_RibPropertyChoice.SetVal( feaprt->GetFeaPropertyIndex() );
                    m_SparPropertyChoice.SetVal( feaprt->GetFeaPropertyIndex() );
                    m_BHPropertyChoice.SetVal( feaprt->GetFeaPropertyIndex() );
                    m_RibArrayPropertyChoice.SetVal( feaprt->GetFeaPropertyIndex() );

                    m_SparCapPropertyChoice.SetVal( feaprt->GetCapFeaPropertyIndex() );
                    m_SliceCapPropertyChoice.SetVal( feaprt->GetCapFeaPropertyIndex() );
                    m_RibCapPropertyChoice.SetVal( feaprt->GetCapFeaPropertyIndex() );
                    m_BHCapPropertyChoice.SetVal( feaprt->GetCapFeaPropertyIndex() );
                    m_RibArrayCapPropertyChoice.SetVal( feaprt->GetCapFeaPropertyIndex() );
                }
            }
            else if ( StructureMgr.GetCurrPartIndex() >= structvec[StructureMgr.GetCurrStructIndex()]->NumFeaParts() )
            {
                SubSurface* subsurf = structvec[StructureMgr.GetCurrStructIndex()]->GetFeaSubSurf( StructureMgr.GetCurrPartIndex() - structvec[StructureMgr.GetCurrStructIndex()]->NumFeaParts() );

                if ( subsurf )
                {
                    // Update all FeaSubSurface Property Choices ( Only Selected Part Visible )
                    m_FeaSSLinePropertyChoice.SetVal( subsurf->GetFeaPropertyIndex() );
                    m_FeaSSRecPropertyChoice.SetVal( subsurf->GetFeaPropertyIndex() );
                    m_FeaSSEllPropertyChoice.SetVal( subsurf->GetFeaPropertyIndex() );
                    m_FeaSSConPropertyChoice.SetVal( subsurf->GetFeaPropertyIndex() );

                    m_FeaSSLineCapPropertyChoice.SetVal( subsurf->GetCapFeaPropertyIndex() );
                    m_FeaSSRecCapPropertyChoice.SetVal( subsurf->GetCapFeaPropertyIndex() );
                    m_FeaSSEllCapPropertyChoice.SetVal( subsurf->GetCapFeaPropertyIndex() );
                    m_FeaSSConCapPropertyChoice.SetVal( subsurf->GetCapFeaPropertyIndex() );
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
                m_FixPointParentSurfChoice.AddItem( string( feaprt_vec[i]->GetName() + "_Surf" ) );
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

                        if ( structVec[StructureMgr.GetCurrStructIndex()]->ValidFeaPartInd( parent_index ) )
                        {
                            m_FixPointParentSurfChoice.SetVal( parent_index );
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
    m_RibPerpendicularEdgeChoice.ClearItems();
    m_RibArrayPerpEdgeChoice.ClearItems();
    m_PerpendicularEdgeVec.clear();

    m_RibPerpendicularEdgeChoice.AddItem( "None" );
    m_RibArrayPerpEdgeChoice.AddItem( "None" );
    m_PerpendicularEdgeVec.push_back( "None" );
    m_RibPerpendicularEdgeChoice.AddItem( "Leading Edge" );
    m_RibArrayPerpEdgeChoice.AddItem( "Leading Edge" );
    m_PerpendicularEdgeVec.push_back( "Leading Edge" );
    m_RibPerpendicularEdgeChoice.AddItem( "Trailing Edge" );
    m_RibArrayPerpEdgeChoice.AddItem( "Trailing Edge" );
    m_PerpendicularEdgeVec.push_back( "Trailing Edge" );

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
                        m_RibPerpendicularEdgeChoice.AddItem( part_vec[i]->GetName() );
                        m_RibArrayPerpEdgeChoice.AddItem( part_vec[i]->GetName() );
                        m_PerpendicularEdgeVec.push_back( part_vec[i]->GetID() );
                    }
                }

                m_RibPerpendicularEdgeChoice.UpdateItems();
                m_RibArrayPerpEdgeChoice.UpdateItems();

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
                                    m_RibPerpendicularEdgeChoice.SetVal( k );
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
                                    m_RibArrayPerpEdgeChoice.SetVal( k );
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
    m_BulkheadEditLayout.Hide();
    m_RibArrayEditLayout.Hide();

    m_FeaSSLineGroup.Hide();
    m_FeaSSRecGroup.Hide();
    m_FeaSSEllGroup.Hide();
    m_FeaSSConGroup.Hide();

    m_CurFeaPartDispGroup = group;

    if ( group )
    {
        group->Show();
    }
}

void FeaPartEditScreen::UpdateFeaPartPropertyIndex( Choice* property_choice )
{
    if ( StructureMgr.ValidTotalFeaStructInd( StructureMgr.GetCurrStructIndex() ) )
    {
        vector < FeaStructure* > structvec = StructureMgr.GetAllFeaStructs();

        if ( StructureMgr.GetCurrPartIndex() < structvec[StructureMgr.GetCurrStructIndex()]->NumFeaParts() )
        {
            FeaPart* feaprt = structvec[StructureMgr.GetCurrStructIndex()]->GetFeaPart( StructureMgr.GetCurrPartIndex() );

            if ( feaprt )
            {
                feaprt->SetFeaPropertyIndex( property_choice->GetVal() );
            }
        }
        else if ( StructureMgr.GetCurrPartIndex() >= structvec[StructureMgr.GetCurrStructIndex()]->NumFeaParts() )
        {
            SubSurface* subsurf = structvec[StructureMgr.GetCurrStructIndex()]->GetFeaSubSurf( StructureMgr.GetCurrPartIndex() - structvec[StructureMgr.GetCurrStructIndex()]->NumFeaParts() );

            if ( subsurf )
            {
                subsurf->SetFeaPropertyIndex( property_choice->GetVal() );
            }
        }
    }
}

void FeaPartEditScreen::UpdateCapPropertyIndex( Choice* property_choice )
{
    if ( StructureMgr.ValidTotalFeaStructInd( StructureMgr.GetCurrStructIndex() ) )
    {
        vector < FeaStructure* > structvec = StructureMgr.GetAllFeaStructs();

        if ( StructureMgr.GetCurrPartIndex() < structvec[StructureMgr.GetCurrStructIndex()]->NumFeaParts() )
        {
            FeaPart* feaprt = structvec[StructureMgr.GetCurrStructIndex()]->GetFeaPart( StructureMgr.GetCurrPartIndex() );

            if ( feaprt )
            {
                feaprt->SetCapFeaPropertyIndex( property_choice->GetVal() );
            }
        }
        else if ( StructureMgr.GetCurrPartIndex() >= structvec[StructureMgr.GetCurrStructIndex()]->NumFeaParts() )
        {
            SubSurface* subsurf = structvec[StructureMgr.GetCurrStructIndex()]->GetFeaSubSurf( StructureMgr.GetCurrPartIndex() - structvec[StructureMgr.GetCurrStructIndex()]->NumFeaParts() );

            if ( subsurf )
            {
                subsurf->SetCapFeaPropertyIndex( property_choice->GetVal() );
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
        string squared( 1, 178 );

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

        m_BHARadUnit.GetFlButton()->copy_label( dist_unit.c_str() );
        m_BHBRadUnit.GetFlButton()->copy_label( dist_unit.c_str() );
        m_BHCRadUnit.GetFlButton()->copy_label( dist_unit.c_str() );
        m_BHXUnit.GetFlButton()->copy_label( dist_unit.c_str() );
        m_BHYUnit.GetFlButton()->copy_label( dist_unit.c_str() );
        m_BHZUnit.GetFlButton()->copy_label( dist_unit.c_str() );

        if ( StructureMgr.ValidTotalFeaStructInd( StructureMgr.GetCurrStructIndex() ) )
        {
            vector< FeaStructure* > structVec = StructureMgr.GetAllFeaStructs();

            if ( StructureMgr.GetCurrPartIndex() < structVec[StructureMgr.GetCurrStructIndex()]->NumFeaParts() )
            {
                FeaPart* feaprt = structVec[StructureMgr.GetCurrStructIndex()]->GetFeaPart( StructureMgr.GetCurrPartIndex() );
                if ( feaprt )
                {
                    if ( feaprt->GetType() == vsp::FEA_SLICE || feaprt->GetType() == vsp::FEA_RIB || feaprt->GetType() == vsp::FEA_SPAR || feaprt->GetType() == vsp::FEA_RIB_ARRAY ) // TODO: Switch to different check
                    {
                        if ( feaprt->m_LocationParmType() == LENGTH )
                        {
                            m_SlicePosUnit.GetFlButton()->copy_label( dist_unit.c_str() );
                            m_SparPosUnit.GetFlButton()->copy_label( dist_unit.c_str() );
                            m_RibPosUnit.GetFlButton()->copy_label( dist_unit.c_str() );
                            m_RibArrayStartLocUnit.GetFlButton()->copy_label( dist_unit.c_str() );
                            m_RibArrayPosUnit.GetFlButton()->copy_label( dist_unit.c_str() );
                        }
                        else if( feaprt->m_LocationParmType() == PERCENT )
                        {
                            m_SlicePosUnit.GetFlButton()->copy_label( "%" );
                            m_SparPosUnit.GetFlButton()->copy_label( "%" );
                            m_RibPosUnit.GetFlButton()->copy_label( "%" );
                            m_RibArrayStartLocUnit.GetFlButton()->copy_label( "%" );
                            m_RibArrayPosUnit.GetFlButton()->copy_label( "%" );
                        }
                    }
                }
            }
        }
    }
}