//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//

// StructScreen.cpp: implementation of the StructScreen class.
//
//////////////////////////////////////////////////////////////////////

#include "StructScreen.h"
#include "FeaMeshMgr.h"
#include "APIDefines.h"
#include "StructureMgr.h"
#include "ManageViewScreen.h"
#include "GraphicEngine.h"
#include "Display.h"
#include "Camera.h"

using namespace vsp;

//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

StructScreen::StructScreen( ScreenMgr* mgr ) : TabScreen( mgr, 415, 625, "FEA Mesh", 150 )
{
    m_FLTK_Window->callback( staticCloseCB, this );

    Fl_Group* structTab = AddTab( "Structure" );
    Fl_Group* structTabGroup = AddSubGroup( structTab, 5 );
    Fl_Group* partTab = AddTab( "Part" );
    Fl_Group* partTabGroup = AddSubGroup( partTab, 5 );
    Fl_Group* subTab = AddTab( "Sub" );
    Fl_Group* subTabGroup = AddSubGroup( subTab, 5 );
    Fl_Group* matTab = AddTab( "Material" );
    Fl_Group* matTabGroup = AddSubGroup( matTab, 5 );
    Fl_Group* propTab = AddTab( "Property" );
    Fl_Group* propTabGroup = AddSubGroup( propTab, 5 );
    Fl_Group* globalTab = AddTab( "Global" );
    Fl_Group* globalTabGroup = AddSubGroup( globalTab, 5 );
    Fl_Group* displayTab = AddTab( "Display" );
    Fl_Group* displayTabGroup = AddSubGroup( displayTab, 5 );

    //=== Create Console Area ===//
    m_ConsoleLayout.SetGroupAndScreen( m_FLTK_Window, this );

    m_ConsoleLayout.AddY( m_ConsoleLayout.GetRemainY()
                        - 7 * m_ConsoleLayout.GetStdHeight()
                        - 2 * m_ConsoleLayout.GetGapHeight() );

    m_ConsoleLayout.AddYGap();
    m_ConsoleLayout.AddX(5);

    m_ConsoleLayout.AddSubGroupLayout( m_BorderConsoleLayout, m_ConsoleLayout.GetRemainX() - 5,
                                       m_ConsoleLayout.GetRemainY() - 5);

    m_ConsoleDisplay = m_BorderConsoleLayout.AddFlTextDisplay( 115 );
    m_ConsoleBuffer = new Fl_Text_Buffer;
    m_ConsoleDisplay->buffer( m_ConsoleBuffer );

    m_BorderConsoleLayout.AddYGap();

    m_BorderConsoleLayout.AddButton( m_FeaMeshExportButton, "Mesh and Export" );

    //=== Structures Tab ===//
    structTab->show();

    m_StructureTabLayout.SetGroupAndScreen( structTabGroup, this );

    m_StructureTabLayout.AddDividerBox( "Structure Selection" );

    int buttonwidth = m_StructureTabLayout.GetButtonWidth();

    m_StructureTabLayout.SetChoiceButtonWidth( buttonwidth );
    m_StructureTabLayout.SetSliderWidth( ( m_StructureTabLayout.GetW() - ( 2 * m_StructureTabLayout.GetButtonWidth() ) ) / 2 );

    m_StructureTabLayout.SetSameLineFlag( true );
    m_StructureTabLayout.SetFitWidthFlag( false );

    m_StructureTabLayout.AddChoice( m_GeomChoice, "Geom" );
    m_StructureTabLayout.AddChoice( m_SurfSel, "Surface" );
    m_StructureTabLayout.ForceNewLine();

    m_StructureTabLayout.SetSameLineFlag( false );
    m_StructureTabLayout.SetFitWidthFlag( true );

    m_StructureTabLayout.AddButton( m_AddFeaStructButton, "Add Structure" );

    m_StructGroup.SetGroupAndScreen( AddSubGroup( structTab, 5 ), this );
    m_StructGroup.SetY( m_StructureTabLayout.GetY() );

    int browser_h = 100;

    m_StructureSelectBrowser = m_StructGroup.AddFlBrowser( browser_h );
    m_StructureSelectBrowser->type( FL_HOLD_BROWSER );
    m_StructureSelectBrowser->labelfont( 13 );
    m_StructureSelectBrowser->labelsize( 12 );
    m_StructureSelectBrowser->textsize( 12 );
    m_StructureSelectBrowser->callback( staticScreenCB, this );

    m_StructGroup.AddButton( m_DelFeaStructButton, "Delete Structure" );

    m_StructGroup.AddYGap();

    buttonwidth *= 2;

    m_StructGroup.SetButtonWidth( buttonwidth );
    m_StructGroup.SetChoiceButtonWidth( buttonwidth );

    m_StructGroup.AddInput( m_FeaStructNameInput, "Struct Name" );
    m_StructGroup.AddChoice( m_SkinPropertyChoice, "Skin Property" );

    m_StructGroup.AddYGap();

    m_StructGeneralGroup.SetGroupAndScreen( AddSubGroup( structTab, 5 ), this );
    m_StructGeneralGroup.SetY( m_StructGroup.GetY() );

    m_StructGeneralGroup.AddDividerBox( "Add Spaced Slices" );

    m_StructGeneralGroup.SetSameLineFlag( true );
    m_StructGeneralGroup.SetFitWidthFlag( false );

    m_StructGeneralGroup.SetButtonWidth( m_StructGeneralGroup.GetRemainX() / 3 );
    m_StructGeneralGroup.SetChoiceButtonWidth( m_StructGeneralGroup.GetRemainX() / 3 );
    m_StructGeneralGroup.SetSliderWidth( m_StructGeneralGroup.GetRemainX() / 3 );

    m_StructGeneralGroup.AddButton( m_MultSliceIncludeTrisToggle, "Include Tris" );
    m_StructGeneralGroup.AddChoice( m_MultSlicePropChoice, "Tri Property" );

    m_StructGeneralGroup.ForceNewLine();

    m_StructGeneralGroup.AddButton( m_MultSliceCapToggle, "Cap Intersections" );
    m_StructGeneralGroup.AddChoice( m_MultSliceCapPropChoice, "Cap Property" );

    m_StructGeneralGroup.ForceNewLine();

    m_StructGeneralGroup.SetSameLineFlag( false );
    m_StructGeneralGroup.SetFitWidthFlag( true );

    m_StructGeneralGroup.AddSlider( m_SpacedPartsInput, "Spacing", 10, "%4.3f" );

    m_StructGeneralGroup.SetSameLineFlag( true );
    m_StructGeneralGroup.SetFitWidthFlag( false );

    m_StructGeneralGroup.SetButtonWidth( m_StructGeneralGroup.GetRemainX() / 3 );
    m_StructGeneralGroup.SetChoiceButtonWidth( m_StructGeneralGroup.GetRemainX() / 3 );

    m_MultSliceOrientationChoice.AddItem( "XY Plane" );
    m_MultSliceOrientationChoice.AddItem( "YZ Plane" );
    m_MultSliceOrientationChoice.AddItem( "XZ Plane" );
    m_StructGeneralGroup.AddChoice( m_MultSliceOrientationChoice, "Orientation" );

    m_StructGeneralGroup.SetButtonWidth( m_StructGeneralGroup.GetRemainX() );

    m_StructGeneralGroup.AddButton( m_AddSpacedPartsButton, "Add" );
    
    m_StructGeneralGroup.ForceNewLine();
    m_StructGeneralGroup.AddYGap();

    m_StructGeneralGroup.SetSameLineFlag( false );
    m_StructGeneralGroup.SetFitWidthFlag( true );

    m_StructGeneralGroup.AddDividerBox( "Orientation" );

    m_StructGeneralGroup.SetSameLineFlag( true );
    m_StructGeneralGroup.SetFitWidthFlag( false );

    m_StructGeneralGroup.SetButtonWidth( m_StructGeneralGroup.GetRemainX() / 3 );

    m_StructGeneralGroup.AddButton( m_OrientFrontButton, "Front" );
    m_StructGeneralGroup.AddButton( m_OrientTopButton, "Top" );
    m_StructGeneralGroup.AddButton( m_OrientSideButton, "Side" );

    m_StructGeneralGroup.ForceNewLine();

    m_StructWingGroup.SetGroupAndScreen( AddSubGroup( structTab, 5 ), this );
    m_StructWingGroup.SetY( m_StructGroup.GetY() );

    m_StructWingGroup.AddDividerBox( "Wing Functions" );

    m_StructWingGroup.AddSlider( m_NumEvenlySpacedRibsInput, "Num Ribs", 10, "%5.0f" );
    m_StructWingGroup.AddButton( m_AddEvenlySpacedRibsButton, "Add" );

    m_StructWingGroup.AddYGap();

    m_StructWingGroup.AddDividerBox( "Orientation" );

    m_StructWingGroup.AddButton( m_OrientWingButton, "Orient Wing" );

    m_StructWingGroup.ForceNewLine();

    //=== Parts Tab ===//
    m_PartTabLayout.SetGroupAndScreen( partTabGroup, this );

    m_PartTabLayout.AddDividerBox( "Fea Part Selection" );

    m_FeaPartSelectBrowser = m_PartTabLayout.AddFlBrowser( browser_h );
    m_FeaPartSelectBrowser->type( FL_HOLD_BROWSER );
    m_FeaPartSelectBrowser->labelfont( 13 );
    m_FeaPartSelectBrowser->labelsize( 12 );
    m_FeaPartSelectBrowser->textsize( 12 );
    m_FeaPartSelectBrowser->callback( staticScreenCB, this );

    m_PartTabLayout.SetChoiceButtonWidth( buttonwidth );

    m_PartTabLayout.AddChoice( m_FeaPartChoice, "Type" );

    m_PartTabLayout.SetSameLineFlag( true );
    m_PartTabLayout.SetFitWidthFlag( false );

    m_PartTabLayout.SetButtonWidth( m_StructGroup.GetRemainX() / 2 );

    m_PartTabLayout.AddButton( m_AddFeaPartButton, "Add Part" );
    m_PartTabLayout.AddButton( m_DelFeaPartButton, "Delete Part" );
    m_PartTabLayout.ForceNewLine();

    m_PartTabLayout.SetSameLineFlag( false );
    m_PartTabLayout.SetFitWidthFlag( true );

    m_PartTabLayout.AddYGap();

    m_PartGroup.SetGroupAndScreen( AddSubGroup( partTab, 5 ), this );
    m_PartGroup.SetY( m_PartTabLayout.GetY() );

    m_PartGroup.SetButtonWidth( buttonwidth );

    m_PartGroup.AddInput( m_FeaPartNameInput, "Part Name" );
    m_PartGroup.AddYGap();
    
    m_CurFeaPartDispGroup = NULL;

    // Indivdual FeaPart Parameters:
    int start_y = m_PartGroup.GetY();

    //==== FeaSlice ====//
    m_SliceEditLayout.SetGroupAndScreen( AddSubGroup( partTab, 5 ), this );
    m_SliceEditLayout.SetY( start_y );

    m_SliceEditLayout.AddDividerBox( "Slice" );

    m_SliceEditLayout.AddButton( m_SliceIncludeTrisToggle, "Include Interior Tris" );

    m_SliceEditLayout.SetButtonWidth( buttonwidth );
    m_SliceEditLayout.SetChoiceButtonWidth( buttonwidth );

    m_SliceEditLayout.AddChoice( m_SlicePropertyChoice, "Property" );

    m_SliceOrientationChoice.AddItem( "XY Plane" );
    m_SliceOrientationChoice.AddItem( "YZ Plane" );
    m_SliceOrientationChoice.AddItem( "XZ Plane" );
    m_SliceEditLayout.AddChoice( m_SliceOrientationChoice, "Orientation" );

    m_SliceEditLayout.AddSlider( m_SliceCenterLocSlider, "Position", 1, "%5.3f" );
    m_SliceEditLayout.AddSlider( m_SliceThetaSlider, "Theta", 25, "%5.3f" );
    m_SliceEditLayout.AddSlider( m_SliceAlphaSlider, "Alpha", 25, "%5.3f" );

    m_SliceEditLayout.AddYGap();

    m_SliceEditLayout.AddButton( m_SliceCapToggle, "Cap Intersections" );
    m_SliceEditLayout.AddChoice( m_SliceCapPropertyChoice, "Cap Property" );

    //==== FeaRib ====//
    m_RibEditLayout.SetGroupAndScreen( AddSubGroup( partTab, 5 ), this );
    m_RibEditLayout.SetY( start_y );

    m_RibEditLayout.AddDividerBox( "Rib" );

    m_RibEditLayout.SetButtonWidth( buttonwidth );
    m_RibEditLayout.SetChoiceButtonWidth( buttonwidth );

    m_RibEditLayout.AddChoice( m_RibPropertyChoice, "Property" );

    m_RibEditLayout.AddChoice( m_RibPerpendicularEdgeChoice, "Perpendicular Edge" );

    m_RibEditLayout.AddSlider( m_RibPosSlider, "Position", 1, "%5.3f" );
    m_RibEditLayout.AddSlider( m_RibThetaSlider, "Theta", 25, "%5.3f" );

    m_RibEditLayout.AddYGap();

    m_RibEditLayout.AddButton( m_RibCapToggle, "Cap Intersections" );
    m_RibEditLayout.AddChoice( m_RibCapPropertyChoice, "Cap Property" );

    //==== FeaSpar ====//
    m_SparEditLayout.SetGroupAndScreen( AddSubGroup( partTab, 5 ), this );
    m_SparEditLayout.SetY( start_y );

    m_SparEditLayout.AddDividerBox( "Spar" );

    m_SparEditLayout.SetButtonWidth( buttonwidth );
    m_SparEditLayout.SetChoiceButtonWidth( buttonwidth );

    m_SparEditLayout.AddChoice( m_SparPropertyChoice, "Property" );

    m_SparEditLayout.AddSlider( m_SparPosSlider, "Position", 1, "%5.3f" );
    m_SparEditLayout.AddSlider( m_SparThetaSlider, "Theta", 25, "%5.3f" );

    m_SparEditLayout.AddYGap();

    m_SparEditLayout.AddButton( m_SparCapToggle, "Cap Intersections" );
    m_SparEditLayout.AddChoice( m_SparCapPropertyChoice, "Cap Property" );

    //==== FeaFixPoint ====//
    m_FixPointEditLayout.SetGroupAndScreen( AddSubGroup( partTab, 5 ), this );

    m_FixPointEditLayout.SetY( start_y );

    m_FixPointEditLayout.AddDividerBox( "Fixed Point" );

    m_FixPointEditLayout.SetChoiceButtonWidth( buttonwidth );
    m_FixPointEditLayout.SetButtonWidth( buttonwidth );

    m_FixPointEditLayout.AddChoice( m_FixPointParentSurfChoice, "Parent Surface" );

    m_FixPointEditLayout.AddSlider( m_FixPointULocSlider, "U Location", 1, "%5.3f" );
    m_FixPointEditLayout.AddSlider( m_FixPointWLocSlider, "W Location", 1, "%5.3f" );

    //=== SubSurface Tab ===//
    m_CurFeaSubDispGroup = NULL;

    m_SubSurfTabLayout.SetGroupAndScreen( subTabGroup, this );

    m_SubSurfTabLayout.AddDividerBox( "Sub-Surface Selection" );

    int start_x = m_SubSurfTabLayout.GetX();

    m_SubSurfTabLayout.AddSubGroupLayout( m_SubSurfButtonLayout, 20, browser_h );
    m_SubSurfTabLayout.AddX( 20 );
    m_SubSurfTabLayout.AddSubGroupLayout( m_SubSurfBrowserLayout, m_SubSurfTabLayout.GetRemainX(), browser_h );
    m_SubSurfTabLayout.GetGroup()->resizable( m_SubSurfBrowserLayout.GetGroup() );
    m_SubSurfTabLayout.AddY( browser_h );

    m_SubSurfButtonLayout.SetSameLineFlag( false );
    m_SubSurfButtonLayout.SetFitWidthFlag( false );

    m_SubSurfButtonLayout.SetStdHeight( 20 );
    m_SubSurfButtonLayout.SetButtonWidth( 20 );
    m_SubSurfButtonLayout.AddButton( m_MoveSSTopButton, "@2<<" );
    m_SubSurfButtonLayout.AddYGap();
    m_SubSurfButtonLayout.AddButton( m_MoveSSUpButton, "@2<" );
    m_SubSurfButtonLayout.AddY( browser_h - 95 - m_SubSurfTabLayout.GetStdHeight() );
    m_SubSurfButtonLayout.AddResizeBox();
    m_SubSurfButtonLayout.AddButton( m_MoveSSDownButton, "@2>" );
    m_SubSurfButtonLayout.AddYGap();
    m_SubSurfButtonLayout.AddButton( m_MoveSSBotButton, "@2>>" );

    m_FeaSubSurfBrowser = m_SubSurfBrowserLayout.AddFlBrowser( browser_h );
    m_FeaSubSurfBrowser->type( 1 );
    m_FeaSubSurfBrowser->labelfont( 13 );
    m_FeaSubSurfBrowser->labelsize( 12 );
    m_FeaSubSurfBrowser->textsize( 12 );
    m_FeaSubSurfBrowser->callback( staticCB, this );

    m_SubSurfTabLayout.SetX( start_x );

    m_SubSurfTabLayout.SetChoiceButtonWidth( buttonwidth );

    m_SubSurfTabLayout.AddChoice( m_FeaSubSurfChoice, "Type" );

    m_SubSurfTabLayout.SetSameLineFlag( true );
    m_SubSurfTabLayout.SetFitWidthFlag( false );

    m_SubSurfTabLayout.SetButtonWidth( m_StructGroup.GetRemainX() / 2 );

    m_SubSurfTabLayout.AddButton( m_AddFeaSubSurfButton, "Add Sub-Surface" );
    m_SubSurfTabLayout.AddButton( m_DelFeaSubSurfButton, "Delete Sub-Surface" );
    m_SubSurfTabLayout.ForceNewLine();

    m_SubSurfTabLayout.SetSameLineFlag( false );
    m_SubSurfTabLayout.SetFitWidthFlag( true );

    m_SubSurfTabLayout.AddYGap();

    m_FeaSSCommonGroup.SetGroupAndScreen( AddSubGroup( subTab, 5 ), this );
    m_FeaSSCommonGroup.SetY( m_SubSurfTabLayout.GetY() );

    m_FeaSSCommonGroup.SetButtonWidth( buttonwidth );
    m_FeaSSCommonGroup.AddInput( m_FeaSubNameInput, "Sub-Surface Name" );

    m_FeaSSCommonGroup.AddYGap();

    // Indivdual SubSurface Parameters
    start_y = m_FeaSSCommonGroup.GetY();

    //==== SSLine ====//
    m_FeaSSLineGroup.SetGroupAndScreen( AddSubGroup( subTab, 5 ), this );
    m_FeaSSLineGroup.SetY( start_y );

    m_FeaSSLineGroup.AddDividerBox( "Line Sub-Surface" );

    m_FeaSSLineGroup.SetFitWidthFlag( false );
    m_FeaSSLineGroup.SetSameLineFlag( true );
    m_FeaSSLineGroup.AddLabel( "Tag", buttonwidth );
    m_FeaSSLineGroup.SetButtonWidth( m_FeaSSLineGroup.GetRemainX() / 3 );
    m_FeaSSLineGroup.AddButton( m_FeaSSLineGreaterToggle, "Greater" );
    m_FeaSSLineGroup.AddButton( m_FeaSSLineLessToggle, "Less" );
    m_FeaSSLineGroup.AddButton( m_FeaSSLineNoneToggle, "None" );

    m_FeaSSLineTestToggleGroup.Init( this );
    m_FeaSSLineTestToggleGroup.AddButton( m_FeaSSLineGreaterToggle.GetFlButton() );
    m_FeaSSLineTestToggleGroup.AddButton( m_FeaSSLineLessToggle.GetFlButton() );
    m_FeaSSLineTestToggleGroup.AddButton( m_FeaSSLineNoneToggle.GetFlButton() );

    m_FeaSSLineGroup.ForceNewLine();
    m_FeaSSLineGroup.SetFitWidthFlag( true );
    m_FeaSSLineGroup.SetSameLineFlag( false );

    m_FeaSSLineGroup.SetButtonWidth( buttonwidth );
    m_FeaSSLineGroup.SetChoiceButtonWidth( buttonwidth );

    m_FeaSSLineGroup.AddButton( m_FeaSSLineRemoveTrisToggle, "RemoveSubSurfaceTris" );

    m_FeaSSLineGroup.AddChoice( m_FeaSSLinePropertyChoice, "Property" );

    m_FeaSSLineGroup.SetFitWidthFlag( false );
    m_FeaSSLineGroup.SetSameLineFlag( true );
    m_FeaSSLineGroup.AddLabel( "Line Type", buttonwidth );
    m_FeaSSLineGroup.SetButtonWidth( m_FeaSSLineGroup.GetRemainX() / 2 );
    m_FeaSSLineGroup.AddButton( m_FeaSSLineConstUButton, "U" );
    m_FeaSSLineGroup.AddButton( m_FeaSSLineConstWButton, "W" );

    m_FeaSSLineConstToggleGroup.Init( this );
    m_FeaSSLineConstToggleGroup.AddButton( m_FeaSSLineConstUButton.GetFlButton() );
    m_FeaSSLineConstToggleGroup.AddButton( m_FeaSSLineConstWButton.GetFlButton() );

    m_FeaSSLineGroup.SetFitWidthFlag( true );
    m_FeaSSLineGroup.SetSameLineFlag( false );
    m_FeaSSLineGroup.ForceNewLine();
    m_FeaSSLineGroup.SetButtonWidth( buttonwidth );
    m_FeaSSLineGroup.AddSlider( m_FeaSSLineConstSlider, "Value", 1, "%5.4f" );

    m_FeaSSLineGroup.AddYGap();

    m_FeaSSLineGroup.AddButton( m_FeaSSLineCapToggle, "Cap Intersections" );
    m_FeaSSLineGroup.AddChoice( m_FeaSSLineCapPropertyChoice, "Cap Property" );

    //==== SSRectangle ====//
    m_FeaSSRecGroup.SetGroupAndScreen( AddSubGroup( subTab, 5 ), this );
    m_FeaSSRecGroup.SetY( start_y );

    m_FeaSSRecGroup.AddDividerBox( "Rectangle Sub-Surface" );

    m_FeaSSRecGroup.SetFitWidthFlag( false );
    m_FeaSSRecGroup.SetSameLineFlag( true );
    m_FeaSSRecGroup.AddLabel( "Tag", buttonwidth );
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

    m_FeaSSRecGroup.SetButtonWidth( buttonwidth );
    m_FeaSSRecGroup.SetChoiceButtonWidth( buttonwidth );

    m_FeaSSRecGroup.AddButton( m_FeaSSRecRemoveTrisToggle, "RemoveSubSurfaceTris" );

    m_FeaSSRecGroup.AddChoice( m_FeaSSRecPropertyChoice, "Property" );

    m_FeaSSRecGroup.SetFitWidthFlag( true );
    m_FeaSSRecGroup.SetSameLineFlag( false );

    m_FeaSSRecGroup.SetButtonWidth( buttonwidth );

    m_FeaSSRecGroup.AddSlider( m_FeaSSRecCentUSlider, "Center U", 1, "%5.4f" );
    m_FeaSSRecGroup.AddSlider( m_FeaSSRecCentWSlider, "Center W", 1, "%5.4f" );
    m_FeaSSRecGroup.AddSlider( m_FeaSSRecULenSlider, "U Length", 1, "%5.4f" );
    m_FeaSSRecGroup.AddSlider( m_FeaSSRecWLenSlider, "W Length", 1, "%5.4f" );
    m_FeaSSRecGroup.AddSlider( m_FeaSSRecThetaSlider, "Theta", 25, "%5.4f" );

    m_FeaSSRecGroup.AddYGap();

    m_FeaSSRecGroup.AddButton( m_FeaSSRecCapToggle, "Cap Intersections" );
    m_FeaSSRecGroup.AddChoice( m_FeaSSRecCapPropertyChoice, "Cap Property" );

    //==== SS_Ellipse ====//
    m_FeaSSEllGroup.SetGroupAndScreen( AddSubGroup( subTab, 5 ), this );
    m_FeaSSEllGroup.SetY( start_y );

    m_FeaSSEllGroup.AddDividerBox( "Ellipse Sub-Surface" );

    m_FeaSSEllGroup.SetFitWidthFlag( false );
    m_FeaSSEllGroup.SetSameLineFlag( true );
    m_FeaSSEllGroup.AddLabel( "Tag", buttonwidth );
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

    m_FeaSSEllGroup.SetButtonWidth( buttonwidth );
    m_FeaSSEllGroup.SetChoiceButtonWidth( buttonwidth );

    m_FeaSSEllGroup.AddButton( m_FeaSSEllRemoveTrisToggle, "RemoveSubSurfaceTris" );

    m_FeaSSEllGroup.AddChoice( m_FeaSSEllPropertyChoice, "Property" );

    m_FeaSSEllGroup.SetFitWidthFlag( true );
    m_FeaSSEllGroup.SetSameLineFlag( false );

    m_FeaSSEllGroup.SetButtonWidth( buttonwidth );

    m_FeaSSEllGroup.AddSlider( m_FeaSSEllTessSlider, "Num Points", 100, "%5.0f" );
    m_FeaSSEllGroup.AddSlider( m_FeaSSEllCentUSlider, "Center U", 1, "%5.4f" );
    m_FeaSSEllGroup.AddSlider( m_FeaSSEllCentWSlider, "Center W", 1, "%5.4f" );
    m_FeaSSEllGroup.AddSlider( m_FeaSSEllULenSlider, "U Length", 1, "%5.4f" );
    m_FeaSSEllGroup.AddSlider( m_FeaSSEllWLenSlider, "W Length", 1, "%5.4f" );
    m_FeaSSEllGroup.AddSlider( m_FeaSSEllThetaSlider, "Theta", 25, "%5.4f" );

    m_FeaSSEllGroup.AddYGap();

    m_FeaSSEllGroup.AddButton( m_FeaSSEllCapToggle, "Cap Intersections" );
    m_FeaSSEllGroup.AddChoice( m_FeaSSEllCapPropertyChoice, "Cap Property" );

    //===== SSControl ====//
    m_FeaSSConGroup.SetGroupAndScreen( AddSubGroup( subTab, 5 ), this );
    m_FeaSSConGroup.SetY( start_y );

    m_FeaSSConGroup.AddDividerBox( "Control Sub-Surface" );

    m_FeaSSConGroup.SetFitWidthFlag( false );
    m_FeaSSConGroup.SetSameLineFlag( true );
    m_FeaSSConGroup.AddLabel( "Tag", buttonwidth );
    m_FeaSSConGroup.SetButtonWidth( m_FeaSSConGroup.GetRemainX() / 3 );
    m_FeaSSConGroup.AddButton( m_FeaSSConInsideButton, "Inside" );
    m_FeaSSConGroup.AddButton( m_FeaSSConOutsideButton, "Outside" );
    m_FeaSSConGroup.AddButton( m_FeaSSConNoneButton, "None" );

    m_FeaSSConTestToggleGroup.Init( this );
    m_FeaSSConTestToggleGroup.AddButton( m_FeaSSConInsideButton.GetFlButton() );
    m_FeaSSConTestToggleGroup.AddButton( m_FeaSSConOutsideButton.GetFlButton() );
    m_FeaSSConTestToggleGroup.AddButton( m_FeaSSConNoneButton.GetFlButton() );

    m_FeaSSConGroup.ForceNewLine();
    m_FeaSSConGroup.SetFitWidthFlag( true );
    m_FeaSSConGroup.SetSameLineFlag( false );

    m_FeaSSConGroup.SetButtonWidth( buttonwidth );
    m_FeaSSConGroup.SetChoiceButtonWidth( buttonwidth );

    m_FeaSSConGroup.AddButton( m_FeaSSConRemoveTrisToggle, "RemoveSubSurfaceTris" );

    m_FeaSSConGroup.AddChoice( m_FeaSSConPropertyChoice, "Property" );

    m_FeaSSConGroup.SetFitWidthFlag( false );
    m_FeaSSConGroup.SetSameLineFlag( true );

    m_FeaSSConGroup.SetChoiceButtonWidth( buttonwidth );

    m_FeaSSConSurfTypeChoice.AddItem( "Upper" );
    m_FeaSSConSurfTypeChoice.AddItem( "Lower" );
    m_FeaSSConSurfTypeChoice.AddItem( "Both" );
    m_FeaSSConGroup.AddChoice( m_FeaSSConSurfTypeChoice, "Upper/Lower", buttonwidth );

    m_FeaSSConGroup.SetButtonWidth( m_FeaSSConGroup.GetRemainX() );

    m_FeaSSConGroup.AddButton( m_FeaSSConLEFlagButton, "Leading Edge" );

    m_FeaSSConGroup.SetFitWidthFlag( true );
    m_FeaSSConGroup.SetSameLineFlag( false );
    m_FeaSSConGroup.ForceNewLine();

    m_FeaSSConGroup.AddYGap();
    m_FeaSSConGroup.AddDividerBox( "Spanwise" );

    m_FeaSSConGroup.SetButtonWidth( buttonwidth );

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

    m_FeaSSConGroup.SetButtonWidth( buttonwidth );

    m_FeaSSConGroup.AddSlider( m_FeaSSConSLenSlider, "Start Length", 10.0, "%5.4f" );
    m_FeaSSConGroup.AddSlider( m_FeaSSConSFracSlider, "Start Length/C", 1.0, "%5.4f" );
    m_FeaSSConGroup.AddSlider( m_FeaSSConELenSlider, "End Length", 10.0, "%5.4f" );
    m_FeaSSConGroup.AddSlider( m_FeaSSConEFracSlider, "End Length/C", 1.0, "%5.4f" );

    m_FeaSSConGroup.AddYGap();

    m_FeaSSConGroup.AddButton( m_FeaSSConCapToggle, "Cap Intersections" );
    m_FeaSSConGroup.AddChoice( m_FeaSSConCapPropertyChoice, "Cap Property" );

    //=== Material Tab ===//
    m_MaterialTabLayout.SetGroupAndScreen( matTabGroup, this );

    m_MaterialTabLayout.AddDividerBox( "Material Selection" );

    m_MaterialEditGroup.SetGroupAndScreen( AddSubGroup( matTabGroup, 5 ), this );
    m_MaterialEditGroup.SetY( m_MaterialTabLayout.GetY() );

    m_FeaMaterialSelectBrowser = m_MaterialEditGroup.AddFlBrowser( browser_h );
    m_FeaMaterialSelectBrowser->type( FL_HOLD_BROWSER );
    m_FeaMaterialSelectBrowser->labelfont( 13 );
    m_FeaMaterialSelectBrowser->labelsize( 12 );
    m_FeaMaterialSelectBrowser->textsize( 12 );
    m_FeaMaterialSelectBrowser->callback( staticScreenCB, this );

    m_MaterialEditGroup.SetSameLineFlag( true );
    m_MaterialEditGroup.SetFitWidthFlag( false );

    m_MaterialEditGroup.SetButtonWidth( m_MaterialEditGroup.GetRemainX() / 2 );

    m_MaterialEditGroup.AddButton( m_AddFeaMaterialButton, "Add Material" );
    m_MaterialEditGroup.AddButton( m_DelFeaMaterialButton, "Delete Material" );
    m_MaterialEditGroup.ForceNewLine();

    m_MaterialEditSubGroup.SetGroupAndScreen( AddSubGroup( matTab, 5 ), this );
    m_MaterialEditSubGroup.SetY( m_MaterialEditGroup.GetY() );

    m_MaterialEditSubGroup.SetButtonWidth( buttonwidth );

    m_MaterialEditSubGroup.AddYGap();

    m_MaterialEditSubGroup.AddInput( m_FeaMaterialNameInput, "Material Name" );

    m_MaterialEditSubGroup.AddYGap();

    m_MaterialEditSubGroup.AddDividerBox( "Material Properties" );

    m_MaterialEditSubGroup.AddSlider( m_MatDensitySlider, "Mass Density", 1, "%5.3f" );
    m_MaterialEditSubGroup.AddSlider( m_MatElasticModSlider, "Elestic Modulus", 10e5, "%g" );
    m_MaterialEditSubGroup.AddSlider( m_MatPoissonSlider, "Poisson Ratio", 1, "%5.3f" );
    m_MaterialEditSubGroup.AddSlider( m_MatThermalExCoeffSlider, "Thermal Expansion Coeff", 10e-5, "%g" );

    m_MaterialEditSubGroup.AddYGap();

    //=== Property Tab ===//
    m_PropertyTabLayout.SetGroupAndScreen( propTabGroup, this );

    m_PropertyTabLayout.AddDividerBox( "Property Selection" );

    m_PropertyEditGroup.SetGroupAndScreen( AddSubGroup( propTabGroup, 5 ), this );
    m_PropertyEditGroup.SetY( m_PropertyTabLayout.GetY() );

    m_FeaPropertySelectBrowser = m_PropertyEditGroup.AddFlBrowser( browser_h );
    m_FeaPropertySelectBrowser->type( FL_HOLD_BROWSER );
    m_FeaPropertySelectBrowser->labelfont( 13 );
    m_FeaPropertySelectBrowser->labelsize( 12 );
    m_FeaPropertySelectBrowser->textsize( 12 );
    m_FeaPropertySelectBrowser->callback( staticScreenCB, this );

    m_PropertyEditGroup.SetChoiceButtonWidth( buttonwidth );

    m_FeaPropertyType.AddItem( "Shell Property" );
    m_FeaPropertyType.AddItem( "Beam Property" );

    m_PropertyEditGroup.AddChoice( m_FeaPropertyType, "Type" );

    m_PropertyEditGroup.SetSameLineFlag( true );
    m_PropertyEditGroup.SetFitWidthFlag( false );

    m_PropertyEditGroup.SetButtonWidth( m_PropertyEditGroup.GetRemainX() / 2 );

    m_PropertyEditGroup.AddButton( m_AddFeaPropertyButton, "Add Property" );
    m_PropertyEditGroup.AddButton( m_DelFeaPropertyButton, "Delete Property" );
    m_PropertyEditGroup.ForceNewLine();

    m_FeaPropertyCommonGroup.SetGroupAndScreen( AddSubGroup( propTab, 5 ), this );
    m_FeaPropertyCommonGroup.SetY( m_PropertyEditGroup.GetY() );

    m_FeaPropertyCommonGroup.SetButtonWidth( buttonwidth );

    m_FeaPropertyCommonGroup.AddYGap();

    m_FeaPropertyCommonGroup.AddInput( m_FeaPropertyNameInput, "Property Name" );

    m_FeaPropertyCommonGroup.AddYGap();

    m_FeaPropertyShellGroup.SetGroupAndScreen( AddSubGroup( propTab, 5 ), this );
    m_FeaPropertyShellGroup.SetY( m_FeaPropertyCommonGroup.GetY() );

    m_FeaPropertyShellGroup.AddDividerBox( "Shell Properties" );

    m_FeaPropertyShellGroup.SetButtonWidth( buttonwidth );
    m_FeaPropertyShellGroup.SetChoiceButtonWidth( buttonwidth );

    m_FeaPropertyShellGroup.AddChoice( m_FeaShellMaterialChoice, "Material" );

    m_FeaPropertyShellGroup.AddSlider( m_PropThickSlider, "Thickness", 1, "%5.3f" );

    m_FeaPropertyBeamGroup.SetGroupAndScreen( AddSubGroup( propTab, 5 ), this );
    m_FeaPropertyBeamGroup.SetY( m_FeaPropertyCommonGroup.GetY() );

    m_FeaPropertyBeamGroup.AddDividerBox( "Beam Properties" );

    m_FeaPropertyBeamGroup.SetButtonWidth( buttonwidth );
    m_FeaPropertyBeamGroup.SetChoiceButtonWidth( buttonwidth );

    m_FeaPropertyBeamGroup.AddChoice( m_FeaBeamMaterialChoice, "Material" );

    m_FeaPropertyBeamGroup.AddSlider( m_PropAreaSlider, "Cross-Sectional Area", 1, "%5.3f" );
    m_FeaPropertyBeamGroup.AddSlider( m_PropIzzSlider, "Izz", 100, "%5.3f" );
    m_FeaPropertyBeamGroup.AddSlider( m_PropIyySlider, "Iyy", 100, "%5.3f" );
    m_FeaPropertyBeamGroup.AddSlider( m_PropIzySlider, "Izy", 100, "%5.3f" );
    m_FeaPropertyBeamGroup.AddSlider( m_PropIxxSlider, "Ixx", 100, "%5.3f" );

    m_PropertyEditGroup.AddYGap();


    //=== GLOBAL TAB ===//
    m_GlobalTabLayout.SetGroupAndScreen( globalTabGroup, this );

    m_GlobalTabLayout.AddDividerBox( "Global Mesh Control" );

    m_GlobalTabLayout.AddYGap();

    m_GlobalTabLayout.SetButtonWidth( 175 );
    m_GlobalTabLayout.AddSlider( m_MaxEdgeLen, "Max Edge Len", 1.0, "%7.5f" );
    m_GlobalTabLayout.AddSlider( m_MinEdgeLen, "Min Edge Len", 1.0, "%7.5f" );
    m_GlobalTabLayout.AddSlider( m_MaxGap, "Max Gap", 1.0, "%7.5f" );
    m_GlobalTabLayout.AddSlider( m_NumCircleSegments, "Num Circle Segments", 100.0, "%7.5f" );
    m_GlobalTabLayout.AddSlider( m_GrowthRatio, "Growth Ratio", 2.0, "%7.5f" );

    m_GlobalTabLayout.AddYGap();
    m_GlobalTabLayout.AddButton( m_Rig3dGrowthLimit, "Rigorous 3D Growth Limiting" );
    m_GlobalTabLayout.AddYGap();

    m_GlobalTabLayout.SetFitWidthFlag( true );
    m_GlobalTabLayout.SetSameLineFlag( false );

    m_GlobalTabLayout.AddDividerBox( "Geometry Control" );
    m_GlobalTabLayout.AddYGap();
    m_GlobalTabLayout.AddButton( m_IntersectSubsurfaces, "Intersect Subsurfaces" );
    m_GlobalTabLayout.AddYGap();

    m_GlobalTabLayout.AddButton( m_HalfMeshButton, "Generate Half Mesh" );
    m_GlobalTabLayout.AddYGap();

    // TODO: Add more CFD Mesh Export file options?

    m_GlobalTabLayout.AddDividerBox( "File Export" );
    m_GlobalTabLayout.AddYGap();

    m_GlobalTabLayout.SetFitWidthFlag( false );
    m_GlobalTabLayout.SetSameLineFlag( true );

    m_GlobalTabLayout.SetButtonWidth( 75 );
    m_GlobalTabLayout.SetInputWidth( m_GlobalTabLayout.GetW() - 125 );

    m_GlobalTabLayout.AddButton( m_StlFile, ".stl" );
    m_GlobalTabLayout.AddOutput( m_StlOutput );
    m_GlobalTabLayout.SetButtonWidth( m_GlobalTabLayout.GetRemainX() );
    m_GlobalTabLayout.AddButton( m_SelectStlFile, "..." );
    m_GlobalTabLayout.ForceNewLine();

    m_GlobalTabLayout.SetButtonWidth( 75 );
    m_GlobalTabLayout.AddButton( m_GmshFile, ".msh" );
    m_GlobalTabLayout.AddOutput( m_GmshOutput );
    m_GlobalTabLayout.SetButtonWidth( m_GlobalTabLayout.GetRemainX() );
    m_GlobalTabLayout.AddButton( m_SelectGmshFile, "..." );
    m_GlobalTabLayout.ForceNewLine();

    m_GlobalTabLayout.AddYGap();

    m_GlobalTabLayout.SetButtonWidth( 75 );
    m_GlobalTabLayout.AddButton( m_MassFile, "Mass" );
    m_GlobalTabLayout.AddOutput( m_MassOutput );
    m_GlobalTabLayout.SetButtonWidth( m_GlobalTabLayout.GetRemainX() );
    m_GlobalTabLayout.AddButton( m_SelectMassFile, "..." );
    m_GlobalTabLayout.ForceNewLine();

    m_GlobalTabLayout.AddYGap();

    m_GlobalTabLayout.SetButtonWidth( 75 );
    m_GlobalTabLayout.AddButton( m_NastFile, "NASTRAN" );
    m_GlobalTabLayout.AddOutput( m_NastOutput );
    m_GlobalTabLayout.SetButtonWidth( m_GlobalTabLayout.GetRemainX() );
    m_GlobalTabLayout.AddButton( m_SelectNastFile, "..." );
    m_GlobalTabLayout.ForceNewLine();

    m_GlobalTabLayout.AddYGap();

    m_GlobalTabLayout.SetButtonWidth( 75 );
    m_GlobalTabLayout.AddButton( m_CalcFile, "Calculix" );
    m_GlobalTabLayout.AddOutput( m_CalcOutput );
    m_GlobalTabLayout.SetButtonWidth( m_GlobalTabLayout.GetRemainX() );
    m_GlobalTabLayout.AddButton( m_SelectCalcFile, "..." );
    m_GlobalTabLayout.ForceNewLine();

    //=== Display Tab ===//
    m_DisplayTabLayout.SetGroupAndScreen( displayTabGroup, this );

    m_DisplayTabLayout.AddDividerBox( "Display" );

    m_DisplayTabLayout.AddYGap();
    m_DisplayTabLayout.AddButton( m_DrawFeaPartsButton, "Draw FeaParts" );

    m_DisplayTabLayout.AddYGap();
    m_DisplayTabLayout.AddButton( m_DrawMeshButton, "Draw Mesh" );

    m_DisplayTabLayout.AddYGap();
    m_DisplayTabLayout.AddButton( m_ColorElementsButton, "Color Elements" );

    m_DisplayTabLayout.AddYGap();
    m_DisplayTabLayout.AddButton( m_DrawNodesToggle, "Draw Nodes" );

    m_DisplayTabLayout.AddYGap();
    m_DisplayTabLayout.AddButton( m_DrawElementOrientVecToggle, "Draw Element Orientation Vectors" );

    m_DisplayTabLayout.AddYGap();
    m_DisplayTabLayout.AddButton( m_ShowBadEdgeTriButton, "Show Bad Edges and Triangles" );

    m_DisplayTabLayout.AddYGap();

    m_DisplayTabLayout.AddDividerBox( "Display FeaElement Sets" );

    m_DrawPartSelectBrowser = m_DisplayTabLayout.AddCheckBrowser( 150 );
    m_DrawPartSelectBrowser->callback( staticScreenCB, this );

    m_DisplayTabLayout.AddY( 125 );
    m_DisplayTabLayout.AddYGap();

    m_DisplayTabLayout.SetSameLineFlag( true );
    m_DisplayTabLayout.SetFitWidthFlag( false );

    m_DisplayTabLayout.SetButtonWidth( m_DisplayTabLayout.GetW() / 2 );

    m_DisplayTabLayout.AddButton( m_DrawAllButton, "Draw All Elements" );
    m_DisplayTabLayout.AddButton( m_HideAllButton, "Hide All Elements" );
    m_DisplayTabLayout.ForceNewLine();

    // Set initial values
    m_FeaCurrMainSurfIndx = 0;
    m_SelectedStructIndex = -1;
    m_SelectedPartIndex = -1;
    m_SelectedSubSurfIndex = -1;
    m_SelectedFeaPartChoice = 0;
    m_SelectedSubSurfChoice = 0;
    m_SelectedMaterialIndex = -1;
    m_SelectedPropertyIndex = -1;
}

StructScreen::~StructScreen()
{
}

void StructScreen::LoadGeomChoice()
{
    //==== Geom Choice ====//
    m_GeomChoice.ClearItems();
    m_GeomIDVec.clear();

    Vehicle* veh = m_ScreenMgr->GetVehiclePtr();
    if ( veh )
    {
        vector< Geom* > geom_vec = veh->FindGeomVec( veh->GetGeomVec( false ) );

        map <string, int> CompIDMap;
        int icomp = 0;
        for ( int i = 0; i < (int)geom_vec.size(); ++i )
        {
            string disp_geom_name = std::to_string( i ) + "_" + geom_vec[i]->GetName();
            int geom_type = geom_vec[i]->GetType().m_Type;

            if ( geom_type != BLANK_GEOM_TYPE && geom_type != PT_CLOUD_GEOM_TYPE && geom_type != HINGE_GEOM_TYPE && geom_type != MESH_GEOM_TYPE )
            {
                m_GeomChoice.AddItem( disp_geom_name );
                CompIDMap[geom_vec[i]->GetID()] = icomp;
                m_GeomIDVec.push_back( geom_vec[i]->GetID() );
                icomp++;
            }
        }
        m_GeomChoice.UpdateItems();

        Geom* currgeom = veh->FindGeom( m_SelectedGeomID );

        if ( m_SelectedGeomID.length() == 0 && m_GeomIDVec.size() > 0 )
        {
            // Handle default case
            m_SelectedGeomID = m_GeomIDVec[0];
        }
        else if ( !currgeom && m_GeomIDVec.size() > 0 )
        {
            m_SelectedGeomID = m_GeomIDVec[0];
        }
        
        m_GeomChoice.SetVal( CompIDMap[m_SelectedGeomID] );
    }
}

void StructScreen::UpdateStructBrowser()
{
    //==== Structure Browser ====//
    int scroll_pos = m_StructureSelectBrowser->position();
    m_StructureSelectBrowser->clear();

    static int widths[] = { 220, 220 };
    m_StructureSelectBrowser->column_widths( widths );
    m_StructureSelectBrowser->column_char( ':' );

    char str[256]; // TODO: Use to_string function

    sprintf( str, "@b@.NAME:@b@.SURFACE" );
    m_StructureSelectBrowser->add( str );

    string struct_name;
    int struct_surf_ind;

    // Populate browser with added structures
    vector< FeaStructure* > structVec = StructureMgr.GetAllFeaStructs();
    if ( structVec.size() > 0 )
    {
        for ( int i = 0; i < (int)structVec.size(); i++ )
        {
            struct_name = structVec[i]->GetFeaStructName();
            struct_surf_ind = structVec[i]->GetFeaStructMainSurfIndx();
            sprintf( str, "%s:Surf_%d", struct_name.c_str(), struct_surf_ind );
            m_StructureSelectBrowser->add( str );
        }

        if ( StructureMgr.ValidTotalFeaStructInd( m_SelectedStructIndex ) )
        {
            m_StructureSelectBrowser->select( m_SelectedStructIndex + 2 );
        }
    }

    m_StructureSelectBrowser->position( scroll_pos );
}

void StructScreen::UpdateFeaPartBrowser()
{
    //==== FeaPart Browser ====//
    int scroll_pos = m_FeaPartSelectBrowser->position();
    m_FeaPartSelectBrowser->clear();
    static int widths[] = { 115, 93, 93, 93 };
    m_FeaPartSelectBrowser->column_widths( widths );
    m_FeaPartSelectBrowser->column_char( ':' );

    char str[256]; // TODO: Use to_string function

    sprintf( str, "@b@.NAME:@b@.TYPE:@b@.MAT:@b@.PROP" );
    m_FeaPartSelectBrowser->add( str );

    string fea_name, fea_type, fea_mat, fea_prop;

    if ( StructureMgr.ValidTotalFeaStructInd( m_SelectedStructIndex ) )
    {
        vector< FeaStructure* > structVec = StructureMgr.GetAllFeaStructs();
        vector<FeaPart*> feaprt_vec = structVec[m_SelectedStructIndex]->GetFeaPartVec();

        for ( int i = 0; i < (int)feaprt_vec.size(); i++ )
        {
            if ( feaprt_vec[i]->GetType() != vsp::FEA_SKIN ) // Do not add skins to list of FeaParts
            {
                fea_name = feaprt_vec[i]->GetName();
                fea_type = FeaPart::GetTypeName( feaprt_vec[i]->GetType() );

                FeaMaterial* mat = StructureMgr.GetFeaMaterial( feaprt_vec[i]->GetFeaMaterialIndex() );
                if ( mat )
                {
                    fea_mat = mat->GetName();
                }

                FeaProperty* prop = StructureMgr.GetFeaProperty( feaprt_vec[i]->GetFeaPropertyIndex() );
                if ( prop )
                {
                    fea_prop = prop->GetName();
                }

                sprintf( str, "%s:%s:%s:%s:", fea_name.c_str(), fea_type.c_str(), fea_mat.c_str(), fea_prop.c_str() );

                m_FeaPartSelectBrowser->add( str );
            }
        }

        if ( structVec[m_SelectedStructIndex]->ValidFeaPartInd( m_SelectedPartIndex ) )
        {
            // Check number of skins, which should be always equal to 1 unless the skin has not been initialized
            int num_skin = structVec[m_SelectedStructIndex]->GetNumFeaSkin();
            m_FeaPartSelectBrowser->select( m_SelectedPartIndex + 2 - num_skin );
        }
    }

    m_FeaPartSelectBrowser->position( scroll_pos );
}

void StructScreen::UpdateDrawPartBrowser()
{
    //==== Draw Part Browser ====//
    int scroll_pos = m_DrawPartSelectBrowser->position();
    m_DrawPartSelectBrowser->clear();

    vector < int > draw_browser_index_vec = FeaMeshMgr.GetDrawBrowserIndexVec();
    vector < string > draw_browser_name_vec = FeaMeshMgr.GetDrawBrowserNameVec();
    vector < bool > draw_element_flag_vec = FeaMeshMgr.GetDrawElementFlagVec();
    vector < bool > draw_cap_flag_vec = FeaMeshMgr.GetDrawCapFlagVec();

    for ( unsigned int i = 0; i < draw_browser_name_vec.size(); i++ )
    {
        if ( draw_browser_name_vec[i].find( "CAP" ) != std::string::npos )
        {
            m_DrawPartSelectBrowser->add( draw_browser_name_vec[i].c_str(), draw_cap_flag_vec[draw_browser_index_vec[i]] );
        }
        else
        {
            m_DrawPartSelectBrowser->add( draw_browser_name_vec[i].c_str(), draw_element_flag_vec[draw_browser_index_vec[i]] );
        }
    }

    m_DrawPartSelectBrowser->position( scroll_pos );
}

void StructScreen::UpdateFeaSubSurfBrowser()
{
    //==== SubSurfBrowser ====//
    int scroll_pos = m_FeaSubSurfBrowser->position();
    m_FeaSubSurfBrowser->clear();
    static int widths[] = { 150, 150, 150 };
    m_FeaSubSurfBrowser->column_widths( widths );
    m_FeaSubSurfBrowser->column_char( ':' );

    char str[256]; // TODO: Use to_string function

    sprintf( str, "@b@.NAME:@b@.TYPE:@b@.SURFACE" );
    m_FeaSubSurfBrowser->add( str );

    string ss_name, ss_type;
    int ss_surf_ind;

    if ( StructureMgr.ValidTotalFeaStructInd( m_SelectedStructIndex ) )
    {
        vector< FeaStructure* > structVec = StructureMgr.GetAllFeaStructs();
        vector<SubSurface*> subsurf_vec = structVec[m_SelectedStructIndex]->GetFeaSubSurfVec();

        for ( int i = 0; i < (int)subsurf_vec.size(); i++ )
        {
            ss_name = subsurf_vec[i]->GetName();
            ss_type = SubSurface::GetTypeName( subsurf_vec[i]->GetType() );
            ss_surf_ind = subsurf_vec[i]->m_MainSurfIndx.Get();
            sprintf( str, "%s:%s:Surf_%d", ss_name.c_str(), ss_type.c_str(), ss_surf_ind );
            m_FeaSubSurfBrowser->add( str );
        }

        if ( structVec[m_SelectedStructIndex]->ValidFeaSubSurfInd( m_SelectedSubSurfIndex ) )
        {
            m_FeaSubSurfBrowser->select( m_SelectedSubSurfIndex + 2 );
        }
    }

    m_FeaSubSurfBrowser->position( scroll_pos );
}

void StructScreen::UpdateMainSurfChoice()
{
    //==== Main Surf Choice ====//
    m_SurfSel.ClearItems();

    Vehicle*  veh = m_ScreenMgr->GetVehiclePtr();

    if ( veh )
    {
        Geom* currgeom = veh->FindGeom( m_SelectedGeomID );

        if ( currgeom )
        {
            int nmain = currgeom->GetNumMainSurfs();
            for ( int i = 0; i < nmain; ++i )
            {
                m_SurfSel.AddItem( string( "Surf_" + std::to_string( i ) ) );
            }
            m_SurfSel.UpdateItems();

            if ( m_FeaCurrMainSurfIndx < 0 || m_FeaCurrMainSurfIndx >= nmain )
            {
                m_FeaCurrMainSurfIndx = 0;
            }
            m_SurfSel.SetVal( m_FeaCurrMainSurfIndx );
        }
    }
}

void StructScreen::UpdateFeaPartChoice()
{
    //==== FeaPart Type Choice ====//
    m_FeaPartChoice.ClearItems();

    Vehicle*  veh = m_ScreenMgr->GetVehiclePtr();

    if ( veh )
    {
        if ( StructureMgr.ValidTotalFeaStructInd( m_SelectedStructIndex ) )
        {
            FeaStructure* curr_struct = StructureMgr.GetAllFeaStructs()[m_SelectedStructIndex];

            if ( curr_struct )
            {
                Geom* currgeom = veh->FindGeom( curr_struct->GetParentGeomID() );

                if ( currgeom )
                {
                    m_FeaPartChoice.AddItem( FeaPart::GetTypeName( vsp::FEA_SLICE ) );
                    m_FeaPartChoice.AddItem( FeaPart::GetTypeName( vsp::FEA_RIB ) );
                    m_FeaPartChoice.AddItem( FeaPart::GetTypeName( vsp::FEA_SPAR ) );
                    m_FeaPartChoice.AddItem( FeaPart::GetTypeName( vsp::FEA_FIX_POINT ) );

                    if ( currgeom->GetType().m_Type == MS_WING_GEOM_TYPE )
                    {
                        m_FeaPartChoice.SetFlag( 1, 0 ); // FEA_RIB
                        m_FeaPartChoice.SetFlag( 2, 0 ); // FEA_SPAR
                    }
                    else
                    {
                        m_FeaPartChoice.SetFlag( 1, FL_MENU_INACTIVE );
                        m_FeaPartChoice.SetFlag( 2, FL_MENU_INACTIVE );
                    }

                    m_FeaPartChoice.UpdateItems();

                    m_FeaPartChoice.SetVal( m_SelectedFeaPartChoice );
                }
            }
        }
    }
}

void StructScreen::UpdateFixPointParentChoice()
{
    //==== FixPoint Parent Surf Choice ====//
    m_FixPointParentSurfChoice.ClearItems();
    m_FixPointParentIDVec.clear();

    if ( StructureMgr.ValidTotalFeaStructInd( m_SelectedStructIndex ) )
    {
        vector< FeaStructure* > structVec = StructureMgr.GetAllFeaStructs();
        vector<FeaPart*> feaprt_vec = structVec[m_SelectedStructIndex]->GetFeaPartVec();

        for ( size_t i = 0; i < feaprt_vec.size(); i++ )
        {

            if ( !structVec[m_SelectedStructIndex]->FeaPartIsFixPoint( i ) )
            {
                m_FixPointParentSurfChoice.AddItem( string( feaprt_vec[i]->GetName() + "_Surf" ) );
                m_FixPointParentIDVec.push_back( feaprt_vec[i]->GetID() );
            }
        }

        m_FixPointParentSurfChoice.UpdateItems();

        FeaPart* feaprt = structVec[m_SelectedStructIndex]->GetFeaPart( m_SelectedPartIndex );

        if ( feaprt )
        {
            if ( feaprt->GetType() == vsp::FEA_FIX_POINT )
            {
                FeaFixPoint* fixpt = dynamic_cast<FeaFixPoint*>( feaprt );
                assert( fixpt );

                FeaPart* parent_feaprt = StructureMgr.GetFeaPart( fixpt->m_ParentFeaPartID );

                if ( parent_feaprt )
                {
                    int parent_index = structVec[m_SelectedStructIndex]->GetFeaPartIndex( parent_feaprt );

                    if ( structVec[m_SelectedStructIndex]->ValidFeaPartInd( parent_index ) )
                    {
                        m_FixPointParentSurfChoice.SetVal( parent_index );
                    }
                }
                else
                {
                    // Set skin as parent surface if undefined
                    if ( structVec[m_SelectedStructIndex]->GetFeaSkin() )
                    {
                        fixpt->m_ParentFeaPartID = structVec[m_SelectedStructIndex]->GetFeaSkin()->GetID();
                    }
                }
            }
        }
    }
}

void StructScreen::UpdateSubSurfChoice()
{
    //==== SubSurf Type Choice ====//
    m_FeaSubSurfChoice.ClearItems();

    Vehicle*  veh = m_ScreenMgr->GetVehiclePtr();

    if ( veh )
    {
        if ( StructureMgr.ValidTotalFeaStructInd( m_SelectedStructIndex ) )
        {
            FeaStructure* curr_struct = StructureMgr.GetAllFeaStructs()[m_SelectedStructIndex];

            if ( curr_struct )
            {
                Geom* currgeom = veh->FindGeom( curr_struct->GetParentGeomID() );

                if ( currgeom )
                {
                    m_FeaSubSurfChoice.AddItem( SubSurface::GetTypeName( vsp::SS_LINE ) );
                    m_FeaSubSurfChoice.AddItem( SubSurface::GetTypeName( vsp::SS_RECTANGLE ) );
                    m_FeaSubSurfChoice.AddItem( SubSurface::GetTypeName( vsp::SS_ELLIPSE ) );

                    int nchoice = 3;

                    if ( currgeom->GetType().m_Type == MS_WING_GEOM_TYPE )
                    {
                        m_FeaSubSurfChoice.AddItem( SubSurface::GetTypeName( vsp::SS_CONTROL ) );
                        nchoice++;
                    }

                    m_FeaSubSurfChoice.UpdateItems();

                    if ( m_SelectedSubSurfChoice < 0 || m_SelectedSubSurfChoice >= nchoice )
                    {
                        m_SelectedSubSurfChoice = 0;
                    }
                    m_FeaSubSurfChoice.SetVal( m_SelectedSubSurfChoice );
                }
            }
        }
    }
}

void StructScreen::UpdateFeaPropertyBrowser()
{
    //==== FeaPart Browser ====//
    int scroll_pos = m_FeaPropertySelectBrowser->position();
    m_FeaPropertySelectBrowser->clear();

    static int widths[] = { 200, 190 };
    m_FeaPropertySelectBrowser->column_widths( widths );
    m_FeaPropertySelectBrowser->column_char( ':' );

    char str[256]; // TODO: Use to_string function

    sprintf( str, "@b@.NAME:@b@.TYPE:" );
    m_FeaPropertySelectBrowser->add( str );

    string prop_name, prop_type;

    vector< FeaProperty* > property_vec = StructureMgr.GetFeaPropertyVec();

    for ( int i = 0; i < (int)property_vec.size(); i++ )
    {
        prop_name = property_vec[i]->GetName();
        prop_type = property_vec[i]->GetTypeName();

        sprintf( str, "%s:%s:", prop_name.c_str(), prop_type.c_str() );

        m_FeaPropertySelectBrowser->add( str );
    }

    if ( StructureMgr.ValidFeaPropertyInd( m_SelectedPropertyIndex ) )
    {
        m_FeaPropertySelectBrowser->select( m_SelectedPropertyIndex + 2 );
    }

    m_FeaPropertySelectBrowser->position( scroll_pos );
}

void StructScreen::UpdateFeaPropertyChoice()
{
    //==== Property Choice ====//
    m_SkinPropertyChoice.ClearItems();
    m_MultSlicePropChoice.ClearItems();
    m_SlicePropertyChoice.ClearItems();
    m_RibPropertyChoice.ClearItems();
    m_SparPropertyChoice.ClearItems();
    m_FeaSSLinePropertyChoice.ClearItems();
    m_FeaSSRecPropertyChoice.ClearItems();
    m_FeaSSEllPropertyChoice.ClearItems();
    m_FeaSSConPropertyChoice.ClearItems();

    m_MultSliceCapPropChoice.ClearItems();
    m_SparCapPropertyChoice.ClearItems();
    m_SliceCapPropertyChoice.ClearItems();
    m_RibCapPropertyChoice.ClearItems();

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
            m_MultSlicePropChoice.AddItem( string( property_vec[i]->GetName() ) );
            m_SlicePropertyChoice.AddItem( string( property_vec[i]->GetName() ) );
            m_RibPropertyChoice.AddItem( string( property_vec[i]->GetName() ) );
            m_SparPropertyChoice.AddItem( string( property_vec[i]->GetName() ) );
            m_FeaSSLinePropertyChoice.AddItem( string( property_vec[i]->GetName() ) );
            m_FeaSSRecPropertyChoice.AddItem( string( property_vec[i]->GetName() ) );
            m_FeaSSEllPropertyChoice.AddItem( string( property_vec[i]->GetName() ) );
            m_FeaSSConPropertyChoice.AddItem( string( property_vec[i]->GetName() ) );

            m_MultSliceCapPropChoice.AddItem( string( property_vec[i]->GetName() ) );
            m_SparCapPropertyChoice.AddItem( string( property_vec[i]->GetName() ) );
            m_SliceCapPropertyChoice.AddItem( string( property_vec[i]->GetName() ) );
            m_RibCapPropertyChoice.AddItem( string( property_vec[i]->GetName() ) );

            m_FeaSSLineCapPropertyChoice.AddItem( string( property_vec[i]->GetName() ) );
            m_FeaSSRecCapPropertyChoice.AddItem( string( property_vec[i]->GetName() ) );
            m_FeaSSEllCapPropertyChoice.AddItem( string( property_vec[i]->GetName() ) );
            m_FeaSSConCapPropertyChoice.AddItem( string( property_vec[i]->GetName() ) );

            if ( property_vec[i]->m_FeaPropertyType() == SHELL_PROPERTY )
            {
                m_SkinPropertyChoice.SetFlag( i, 0 );
                m_MultSlicePropChoice.SetFlag( i, 0 );
                m_SlicePropertyChoice.SetFlag( i, 0 );
                m_RibPropertyChoice.SetFlag( i, 0 );
                m_SparPropertyChoice.SetFlag( i, 0 );
                m_FeaSSLinePropertyChoice.SetFlag( i, 0 );
                m_FeaSSRecPropertyChoice.SetFlag( i, 0 );
                m_FeaSSEllPropertyChoice.SetFlag( i, 0 );
                m_FeaSSConPropertyChoice.SetFlag( i, 0 );

                m_MultSliceCapPropChoice.SetFlag( i, FL_MENU_INACTIVE );
                m_SparCapPropertyChoice.SetFlag( i, FL_MENU_INACTIVE );
                m_SliceCapPropertyChoice.SetFlag( i, FL_MENU_INACTIVE );
                m_RibCapPropertyChoice.SetFlag( i, FL_MENU_INACTIVE );

                m_FeaSSLineCapPropertyChoice.SetFlag( i, FL_MENU_INACTIVE );
                m_FeaSSRecCapPropertyChoice.SetFlag( i, FL_MENU_INACTIVE );
                m_FeaSSEllCapPropertyChoice.SetFlag( i, FL_MENU_INACTIVE );
                m_FeaSSConCapPropertyChoice.SetFlag( i, FL_MENU_INACTIVE );
            }
            else if ( property_vec[i]->m_FeaPropertyType() == BEAM_PROPERTY )
            {
                m_SkinPropertyChoice.SetFlag( i, FL_MENU_INACTIVE );
                m_MultSlicePropChoice.SetFlag( i, FL_MENU_INACTIVE );
                m_SlicePropertyChoice.SetFlag( i, FL_MENU_INACTIVE );
                m_RibPropertyChoice.SetFlag( i, FL_MENU_INACTIVE );
                m_SparPropertyChoice.SetFlag( i, FL_MENU_INACTIVE );
                m_FeaSSLinePropertyChoice.SetFlag( i, FL_MENU_INACTIVE );
                m_FeaSSRecPropertyChoice.SetFlag( i, FL_MENU_INACTIVE );
                m_FeaSSEllPropertyChoice.SetFlag( i, FL_MENU_INACTIVE );
                m_FeaSSConPropertyChoice.SetFlag( i, FL_MENU_INACTIVE );

                m_MultSliceCapPropChoice.SetFlag( i, 0 );
                m_SparCapPropertyChoice.SetFlag( i, 0 );
                m_SliceCapPropertyChoice.SetFlag( i, 0 );
                m_RibCapPropertyChoice.SetFlag( i, 0 );

                m_FeaSSLineCapPropertyChoice.SetFlag( i, 0 );
                m_FeaSSRecCapPropertyChoice.SetFlag( i, 0 );
                m_FeaSSEllCapPropertyChoice.SetFlag( i, 0 );
                m_FeaSSConCapPropertyChoice.SetFlag( i, 0 );
            }
        }

        m_SkinPropertyChoice.UpdateItems();
        m_MultSlicePropChoice.UpdateItems();
        m_SlicePropertyChoice.UpdateItems();
        m_RibPropertyChoice.UpdateItems();
        m_SparPropertyChoice.UpdateItems();
        m_FeaSSLinePropertyChoice.UpdateItems();
        m_FeaSSRecPropertyChoice.UpdateItems();
        m_FeaSSEllPropertyChoice.UpdateItems();
        m_FeaSSConPropertyChoice.UpdateItems();

        m_MultSliceCapPropChoice.UpdateItems();
        m_SparCapPropertyChoice.UpdateItems();
        m_SliceCapPropertyChoice.UpdateItems();
        m_RibCapPropertyChoice.UpdateItems();

        m_FeaSSLineCapPropertyChoice.UpdateItems();
        m_FeaSSRecCapPropertyChoice.UpdateItems();
        m_FeaSSEllCapPropertyChoice.UpdateItems();
        m_FeaSSConCapPropertyChoice.UpdateItems();

        if ( StructureMgr.ValidTotalFeaStructInd( m_SelectedStructIndex ) )
        {
            vector < FeaStructure* > structvec = StructureMgr.GetAllFeaStructs();
            FeaPart* feaskin = structvec[m_SelectedStructIndex]->GetFeaSkin();

            if ( feaskin )
            {
                m_SkinPropertyChoice.SetVal( feaskin->GetFeaPropertyIndex() );
            }

            m_MultSlicePropChoice.SetVal( structvec[m_SelectedStructIndex]->GetStructSettingsPtr()->GetMultPropertyIndex() );
            m_MultSliceCapPropChoice.SetVal( structvec[m_SelectedStructIndex]->GetStructSettingsPtr()->GetMultCapPropertyIndex() );

            FeaPart* feaprt = structvec[m_SelectedStructIndex]->GetFeaPart( m_SelectedPartIndex );

            if ( feaprt )
            {
                // Update all FeaPart Property Choices ( Only Selected Part Visible )
                m_SlicePropertyChoice.SetVal( feaprt->GetFeaPropertyIndex() );
                m_RibPropertyChoice.SetVal( feaprt->GetFeaPropertyIndex() );
                m_SparPropertyChoice.SetVal( feaprt->GetFeaPropertyIndex() );

                m_SparCapPropertyChoice.SetVal( feaprt->GetCapFeaPropertyIndex() );
                m_SliceCapPropertyChoice.SetVal( feaprt->GetCapFeaPropertyIndex() );
                m_RibCapPropertyChoice.SetVal( feaprt->GetCapFeaPropertyIndex() );
            }

            SubSurface* subsurf = structvec[m_SelectedStructIndex]->GetFeaSubSurf( m_SelectedSubSurfIndex );

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

void StructScreen::UpdateFeaMaterialBrowser()
{
    //==== FeaMaterial Browser ====//
    int scroll_pos = m_FeaMaterialSelectBrowser->position();
    m_FeaMaterialSelectBrowser->clear();

    vector< FeaMaterial* > material_vec = StructureMgr.GetFeaMaterialVec();

    for ( int i = 0; i < (int)material_vec.size(); i++ )
    {
        string mat_name = material_vec[i]->GetName();
        m_FeaMaterialSelectBrowser->add( mat_name.c_str() );
    }

    if ( StructureMgr.ValidFeaMaterialInd( m_SelectedMaterialIndex ) )
    {
        m_FeaMaterialSelectBrowser->select( m_SelectedMaterialIndex + 1 );
    }

    if ( StructureMgr.NumFeaMaterials() <= 0 )
    {
        m_MaterialEditSubGroup.Hide();
    }
    else
    {
        m_MaterialEditSubGroup.Show();
    }

    m_FeaMaterialSelectBrowser->position( scroll_pos );
}

void StructScreen::UpdateFeaMaterialChoice()
{
    //==== Material Choice ====//
    m_FeaShellMaterialChoice.ClearItems();
    m_FeaBeamMaterialChoice.ClearItems();

    Vehicle*  veh = m_ScreenMgr->GetVehiclePtr();

    if ( veh )
    {
        vector< FeaMaterial* > material_vec = StructureMgr.GetFeaMaterialVec();

        for ( int i = 0; i < material_vec.size(); ++i )
        {
            m_FeaShellMaterialChoice.AddItem( string( material_vec[i]->GetName() ) );
            m_FeaBeamMaterialChoice.AddItem( string( material_vec[i]->GetName() ) );
        }
        m_FeaShellMaterialChoice.UpdateItems();
        m_FeaBeamMaterialChoice.UpdateItems();

        if ( StructureMgr.ValidFeaPropertyInd( m_SelectedPropertyIndex ) )
        {
            FeaProperty* fea_prop = StructureMgr.GetFeaPropertyVec()[m_SelectedPropertyIndex];

            if ( fea_prop )
            {
                // Update all FeaPart Material Choices ( Only Selected Property Visible )
                m_FeaShellMaterialChoice.SetVal( fea_prop->GetFeaMaterialIndex() );
                m_FeaBeamMaterialChoice.SetVal( fea_prop->GetFeaMaterialIndex() );
            }
        }
    }
}

void StructScreen::UpdatePerpendicularRibChoice()
{
    //==== Perpendicular Rib Choice ====//
    m_RibPerpendicularEdgeChoice.ClearItems();
    m_PerpendicularEdgeVec.clear();

    m_RibPerpendicularEdgeChoice.AddItem( "None" );
    m_PerpendicularEdgeVec.push_back( "None" );
    m_RibPerpendicularEdgeChoice.AddItem( "Leading Edge" );
    m_PerpendicularEdgeVec.push_back( "Leading Edge" );
    m_RibPerpendicularEdgeChoice.AddItem( "Trailing Edge" );
    m_PerpendicularEdgeVec.push_back( "Trailing Edge" );

    Vehicle*  veh = m_ScreenMgr->GetVehiclePtr();

    if ( veh )
    {
        if ( StructureMgr.ValidTotalFeaStructInd( m_SelectedStructIndex ) )
        {
            FeaStructure* curr_struct = StructureMgr.GetAllFeaStructs()[m_SelectedStructIndex];

            if ( curr_struct )
            {
                vector < FeaPart* > part_vec = curr_struct->GetFeaPartVec();

                for ( size_t i = 1; i < part_vec.size(); i++ )
                {
                    if ( part_vec[i]->GetType() == vsp::FEA_SPAR )
                    {
                        m_RibPerpendicularEdgeChoice.AddItem( part_vec[i]->GetName() );
                        m_PerpendicularEdgeVec.push_back( part_vec[i]->GetID() );
                    }
                }

                m_RibPerpendicularEdgeChoice.UpdateItems();

                FeaPart* feaprt = curr_struct->GetFeaPart( m_SelectedPartIndex );

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
                                rib->SetPerpendicularEdgeIndex( k );
                                break;
                            }
                        }
                    }
                }
            }
        }
    }
}

void StructScreen::CloseCallBack( Fl_Widget *w )
{
    Hide();

    m_ScreenMgr->SetUpdateFlag( true );
}

void StructScreen::Show()
{
    m_FLTK_Window->show();

    m_ScreenMgr->SetUpdateFlag( true );
}

void StructScreen::FeaPartDispGroup( GroupLayout* group )
{
    if ( m_CurFeaPartDispGroup == group && group )
    {
        return;
    }

    if ( StructureMgr.ValidTotalFeaStructInd( m_SelectedStructIndex ) )
    {
        m_StructGroup.Show();

        FeaStructure* curr_struct = StructureMgr.GetAllFeaStructs()[m_SelectedStructIndex];

        Vehicle*  veh = m_ScreenMgr->GetVehiclePtr();

        if ( veh && curr_struct )
        {
            Geom* currgeom = veh->FindGeom( curr_struct->GetParentGeomID() );

            if ( currgeom )
            {
                if ( currgeom->GetType().m_Type == MS_WING_GEOM_TYPE )
                {
                    m_StructWingGroup.Show();
                    m_StructGeneralGroup.Hide();
                }
                else
                {
                    m_StructGeneralGroup.Show();
                    m_StructWingGroup.Hide();
                }
            }
        }
    }
    else
    {
        m_StructGroup.Hide();
        m_StructWingGroup.Hide();
        m_StructGeneralGroup.Hide();
    }

    m_SliceEditLayout.Hide();
    m_RibEditLayout.Hide();
    m_SparEditLayout.Hide();
    m_FixPointEditLayout.Hide();
    m_PartGroup.Hide();

    m_CurFeaPartDispGroup = group;

    if ( group )
    {
        group->Show();
        m_PartGroup.Show(); // Always show the Part Group if any other FeaPart is being displayed.
    }
}

void StructScreen::FeaSubSurfDispGroup( GroupLayout* group )
{
    if ( m_CurFeaSubDispGroup == group && group )
    {
        return;
    }

    m_FeaSSLineGroup.Hide();
    m_FeaSSRecGroup.Hide();
    m_FeaSSCommonGroup.Hide();
    m_FeaSSEllGroup.Hide();
    m_FeaSSConGroup.Hide();

    m_CurFeaSubDispGroup = group;

    if ( group )
    {
        group->Show();
        m_FeaSSCommonGroup.Show(); // Always show the Common Group if any other subsurface group is being displayed.
    }
}

void StructScreen::FeaPropertyDispGroup( GroupLayout* group )
{
    if ( m_CurFeaPropDispGroup == group && group )
    {
        return;
    }

    m_FeaPropertyShellGroup.Hide();
    m_FeaPropertyBeamGroup.Hide();
    m_FeaPropertyCommonGroup.Hide();

    m_CurFeaPropDispGroup = group;

    if ( group )
    {
        group->Show();
        m_FeaPropertyCommonGroup.Show(); // Always show the Common Group if any other subsurface group is being displayed.
    }
}

void StructScreen::UpdateFeaPartPropertyIndex( Choice* property_choice )
{
    if ( StructureMgr.ValidTotalFeaStructInd( m_SelectedStructIndex ) )
    {
        vector < FeaStructure* > structvec = StructureMgr.GetAllFeaStructs();
        FeaPart* feaprt = structvec[m_SelectedStructIndex]->GetFeaPart( m_SelectedPartIndex );

        if ( feaprt )
        {
            feaprt->SetFeaPropertyIndex( property_choice->GetVal() );
        }
    }
}

void StructScreen::UpdateCapPropertyIndex( Choice* property_choice )
{
    if ( StructureMgr.ValidTotalFeaStructInd( m_SelectedStructIndex ) )
    {
        vector < FeaStructure* > structvec = StructureMgr.GetAllFeaStructs();
        FeaPart* feaprt = structvec[m_SelectedStructIndex]->GetFeaPart( m_SelectedPartIndex );

        if ( feaprt )
        {
            feaprt->SetCapFeaPropertyIndex( property_choice->GetVal() );
        }
    }
}

void StructScreen::UpdateFeaSubSurfPropertyIndex( Choice* property_choice )
{
    if ( StructureMgr.ValidTotalFeaStructInd( m_SelectedStructIndex ) )
    {
        vector < FeaStructure* > structvec = StructureMgr.GetAllFeaStructs();
        SubSurface* subsurf = structvec[m_SelectedStructIndex]->GetFeaSubSurf( m_SelectedSubSurfIndex );

        if ( subsurf )
        {
            subsurf->SetFeaPropertyIndex( property_choice->GetVal() );
        }
    }
}

void StructScreen::UpdateFeaSubSurfCapPropertyIndex( Choice* property_choice )
{
    if ( StructureMgr.ValidTotalFeaStructInd( m_SelectedStructIndex ) )
    {
        vector < FeaStructure* > structvec = StructureMgr.GetAllFeaStructs();
        SubSurface* subsurf = structvec[m_SelectedStructIndex]->GetFeaSubSurf( m_SelectedSubSurfIndex );

        if ( subsurf )
        {
            subsurf->SetCapFeaPropertyIndex( property_choice->GetVal() );
        }
    }
}

bool StructScreen::Update()
{
    Vehicle* veh = m_ScreenMgr->GetVehiclePtr();

    if ( veh )
    {
        //===== Geom Choice Update =====//
        LoadGeomChoice();

        //==== Main Surf Choice Update ====//
        UpdateMainSurfChoice();

        //===== Structure Browser Update =====//
        UpdateStructBrowser();

        //===== Draw Part Browser Update =====//
        UpdateDrawPartBrowser();

        //===== FeaPart Browser Update =====//
        UpdateFeaPartBrowser();

        //==== Update FeaPart Choice ====//
        UpdateFeaPartChoice();

        //==== Update Perpendicular Rib Choice ====//
        UpdatePerpendicularRibChoice();

        //==== Update FixPoint Parent Surf Choice ====//
        UpdateFixPointParentChoice();

        //==== Update SubSurface Choice ====//
        UpdateSubSurfChoice();

        //===== SubSurf Browser Update =====//
        UpdateFeaSubSurfBrowser();

        if ( StructureMgr.ValidTotalFeaStructInd( m_SelectedStructIndex ) )
        {
            vector< FeaStructure* > structVec = StructureMgr.GetAllFeaStructs();

            //==== Default Elem Size ====//
            m_MaxEdgeLen.Update( structVec[m_SelectedStructIndex]->GetFeaGridDensityPtr()->m_BaseLen.GetID() );
            m_MinEdgeLen.Update( structVec[m_SelectedStructIndex]->GetFeaGridDensityPtr()->m_MinLen.GetID() );
            m_MaxGap.Update( structVec[m_SelectedStructIndex]->GetFeaGridDensityPtr()->m_MaxGap.GetID() );
            m_NumCircleSegments.Update( structVec[m_SelectedStructIndex]->GetFeaGridDensityPtr()->m_NCircSeg.GetID() );
            m_GrowthRatio.Update( structVec[m_SelectedStructIndex]->GetFeaGridDensityPtr()->m_GrowRatio.GetID() );
            m_Rig3dGrowthLimit.Update( structVec[m_SelectedStructIndex]->GetFeaGridDensityPtr()->m_RigorLimit.GetID() );

            //===== Geometry Control =====//
            m_IntersectSubsurfaces.Update( structVec[m_SelectedStructIndex]->GetStructSettingsPtr()->m_IntersectSubSurfs.GetID() );
            m_HalfMeshButton.Update( structVec[m_SelectedStructIndex]->GetStructSettingsPtr()->m_HalfMeshFlag.GetID() );

            //===== Display Tab Toggle Update =====//
            m_DrawFeaPartsButton.Update( structVec[m_SelectedStructIndex]->GetStructSettingsPtr()->m_DrawFeaPartsFlag.GetID() );
            m_DrawMeshButton.Update( structVec[m_SelectedStructIndex]->GetStructSettingsPtr()->m_DrawMeshFlag.GetID() );
            m_ShowBadEdgeTriButton.Update( structVec[m_SelectedStructIndex]->GetStructSettingsPtr()->m_DrawBadFlag.GetID() );
            m_ColorElementsButton.Update( structVec[m_SelectedStructIndex]->GetStructSettingsPtr()->m_ColorTagsFlag.GetID() );
            m_DrawNodesToggle.Update( structVec[m_SelectedStructIndex]->GetStructSettingsPtr()->m_DrawNodesFlag.GetID() );
            m_DrawElementOrientVecToggle.Update( structVec[m_SelectedStructIndex]->GetStructSettingsPtr()->m_DrawElementOrientVecFlag.GetID() );

            if ( FeaMeshMgr.GetStructSettingsPtr() )
            {
                FeaMeshMgr.UpdateDisplaySettings();
            }

            string massname = structVec[m_SelectedStructIndex]->GetStructSettingsPtr()->GetExportFileName( vsp::MASS_FILE_NAME );
            m_MassOutput.Update( truncateFileName( massname, 40 ).c_str() );
            string nastranname = structVec[m_SelectedStructIndex]->GetStructSettingsPtr()->GetExportFileName( vsp::NASTRAN_FILE_NAME );
            m_NastOutput.Update( truncateFileName( nastranname, 40 ).c_str() );
            string calculixname = structVec[m_SelectedStructIndex]->GetStructSettingsPtr()->GetExportFileName( vsp::CALCULIX_FILE_NAME );
            m_CalcOutput.Update( truncateFileName( calculixname, 40 ).c_str() );
            string stlname = structVec[m_SelectedStructIndex]->GetStructSettingsPtr()->GetExportFileName( vsp::STL_FEA_NAME );
            m_StlOutput.Update( truncateFileName( stlname, 40 ).c_str() );
            string gmshname = structVec[m_SelectedStructIndex]->GetStructSettingsPtr()->GetExportFileName( vsp::GMSH_FEA_NAME );
            m_GmshOutput.Update( truncateFileName( gmshname, 40 ).c_str() );

            //==== Update File Output Flags ====//
            m_MassFile.Update( structVec[m_SelectedStructIndex]->GetStructSettingsPtr()->GetExportFileFlag( vsp::MASS_FILE_NAME )->GetID() );
            m_NastFile.Update( structVec[m_SelectedStructIndex]->GetStructSettingsPtr()->GetExportFileFlag( vsp::NASTRAN_FILE_NAME )->GetID() );
            m_CalcFile.Update( structVec[m_SelectedStructIndex]->GetStructSettingsPtr()->GetExportFileFlag( vsp::CALCULIX_FILE_NAME )->GetID() );
            m_StlFile.Update( structVec[m_SelectedStructIndex]->GetStructSettingsPtr()->GetExportFileFlag( vsp::STL_FEA_NAME )->GetID() );
            m_GmshFile.Update( structVec[m_SelectedStructIndex]->GetStructSettingsPtr()->GetExportFileFlag( vsp::GMSH_FEA_NAME )->GetID() );

            // Update Structure Name
            m_FeaStructNameInput.Update( structVec[m_SelectedStructIndex]->GetFeaStructName() );

            m_NumEvenlySpacedRibsInput.Update( structVec[m_SelectedStructIndex]->GetStructSettingsPtr()->m_NumEvenlySpacedPart.GetID() );

            m_SpacedPartsInput.Update( structVec[m_SelectedStructIndex]->GetStructSettingsPtr()->m_MultiSliceSpacing.GetID() );
            m_MultSliceIncludeTrisToggle.Update( structVec[m_SelectedStructIndex]->GetStructSettingsPtr()->m_MultSliceIncludeTrisFlag.GetID() );
            m_MultSliceCapToggle.Update( structVec[m_SelectedStructIndex]->GetStructSettingsPtr()->m_MultSliceCapFlag.GetID() );

            if ( structVec[m_SelectedStructIndex]->GetStructSettingsPtr()->m_MultSliceIncludeTrisFlag() )
            {
                m_MultSlicePropChoice.Activate();
            }
            else
            {
                m_MultSlicePropChoice.Deactivate();
            }

            if ( structVec[m_SelectedStructIndex]->GetStructSettingsPtr()->m_MultSliceCapFlag() )
            {
                m_MultSliceCapPropChoice.Activate();
            }
            else
            {
                m_MultSliceCapPropChoice.Deactivate();
            }

            // Update Current FeaPart
            FeaPart* feaprt = structVec[m_SelectedStructIndex]->GetFeaPart( m_SelectedPartIndex );
            if ( feaprt )
            {
                m_FeaPartNameInput.Update( feaprt->GetName() );
                if ( feaprt->GetType() == vsp::FEA_SLICE )
                {
                    FeaSlice* slice = dynamic_cast<FeaSlice*>( feaprt );
                    assert( slice );

                    m_SliceIncludeTrisToggle.Update( slice->m_IncludeTrisFlag.GetID() );
                    m_SliceOrientationChoice.Update( slice->m_OrientationPlane.GetID() );
                    m_SliceCenterLocSlider.Update( slice->m_CenterPerBBoxLocation.GetID() );
                    m_SliceThetaSlider.Update( slice->m_Theta.GetID() );
                    m_SliceAlphaSlider.Update( slice->m_Alpha.GetID() );
                    m_SliceCapToggle.Update( slice->m_IntersectionCapFlag.GetID() );

                    if ( !slice->m_IncludeTrisFlag() )
                    {
                        m_SlicePropertyChoice.Deactivate();
                    }
                    else
                    {
                        m_SlicePropertyChoice.Activate();
                    }

                    if ( slice->m_IntersectionCapFlag() )
                    {
                        m_SliceCapPropertyChoice.Activate();
                    }
                    else
                    {
                        m_SliceCapPropertyChoice.Deactivate();
                    }

                    FeaPartDispGroup( &m_SliceEditLayout );
                }
                else if ( feaprt->GetType() == vsp::FEA_RIB )
                {
                    FeaRib* rib = dynamic_cast<FeaRib*>( feaprt );
                    assert( rib );

                    m_RibPosSlider.Update( rib->m_PerU.GetID() );
                    m_RibThetaSlider.Update( rib->m_Theta.GetID() );
                    m_RibCapToggle.Update( rib->m_IntersectionCapFlag.GetID() );

                    if ( rib->m_IntersectionCapFlag() )
                    {
                        m_RibCapPropertyChoice.Activate();
                    }
                    else
                    {
                        m_RibCapPropertyChoice.Deactivate();
                    }

                    FeaPartDispGroup( &m_RibEditLayout );
                }
                else if ( feaprt->GetType() == vsp::FEA_SPAR )
                {
                    FeaSpar* spar = dynamic_cast<FeaSpar*>( feaprt );
                    assert( spar );

                    m_SparPosSlider.Update( spar->m_PerV.GetID() );
                    m_SparThetaSlider.Update( spar->m_Theta.GetID() );
                    m_SparCapToggle.Update( spar->m_IntersectionCapFlag.GetID() );

                    if ( spar->m_IntersectionCapFlag() )
                    {
                        m_SparCapPropertyChoice.Activate();
                    }
                    else
                    {
                        m_SparCapPropertyChoice.Deactivate();
                    }

                    FeaPartDispGroup( &m_SparEditLayout );
                }
                else if ( feaprt->GetType() == vsp::FEA_FIX_POINT )
                {
                    FeaFixPoint* fixpt = dynamic_cast<FeaFixPoint*>( feaprt );
                    assert( fixpt );

                    m_FixPointULocSlider.Update( fixpt->m_PosU.GetID() );
                    m_FixPointWLocSlider.Update( fixpt->m_PosW.GetID() );

                    FeaPartDispGroup( &m_FixPointEditLayout );
                }
                else
                {
                    FeaPartDispGroup( NULL );
                }

                // Do not update FeaParts if mesh is in progress
                if ( FeaMeshMgr.GetFeaMeshInProgress() == false )
                {
                    structVec[m_SelectedStructIndex]->Update();
                }
            }
            else
            {
                FeaPartDispGroup( NULL );
            }

            //===== SubSurface =====//
            SubSurface* subsurf = structVec[m_SelectedStructIndex]->GetFeaSubSurf( m_SelectedSubSurfIndex );
            if ( subsurf )
            {
                m_FeaSubNameInput.Update( subsurf->GetName() );
                if ( subsurf->GetType() == vsp::SS_LINE )
                {
                    SSLine* ssline = dynamic_cast< SSLine* >( subsurf );
                    assert( ssline );

                    m_FeaSSLineConstToggleGroup.Update( ssline->m_ConstType.GetID() );
                    m_FeaSSLineTestToggleGroup.Update( ssline->m_TestType.GetID() );
                    m_FeaSSLineConstSlider.Update( ssline->m_ConstVal.GetID() );
                    m_FeaSSLineCapToggle.Update( ssline->m_IntersectionCapFlag.GetID() );
                    m_FeaSSLineRemoveTrisToggle.Update( ssline->m_RemoveSubSurfTrisFlag.GetID() );

                    m_FeaSSLinePropertyChoice.Activate();
                    m_FeaSSLineRemoveTrisToggle.Activate();

                    if ( ssline->m_TestType() == vsp::NONE )
                    {
                        m_FeaSSLinePropertyChoice.Deactivate();
                        ssline->m_RemoveSubSurfTrisFlag.Set( false );
                        m_FeaSSLineRemoveTrisToggle.Deactivate();
                    }

                    if ( ssline->m_RemoveSubSurfTrisFlag.Get() )
                    {
                        m_FeaSSLinePropertyChoice.Deactivate();
                    }

                    if ( ssline->m_IntersectionCapFlag() )
                    {
                        m_FeaSSLineCapPropertyChoice.Activate();
                    }
                    else
                    {
                        m_FeaSSLineCapPropertyChoice.Deactivate();
                    }

                    FeaSubSurfDispGroup( &m_FeaSSLineGroup );
                }
                else if ( subsurf->GetType() == vsp::SS_RECTANGLE )
                {
                    SSRectangle* ssrec = dynamic_cast< SSRectangle* >( subsurf );
                    assert( subsurf );

                    m_FeaSSRecTestToggleGroup.Update( ssrec->m_TestType.GetID() );
                    m_FeaSSRecCentUSlider.Update( ssrec->m_CenterU.GetID() );
                    m_FeaSSRecCentWSlider.Update( ssrec->m_CenterW.GetID() );
                    m_FeaSSRecULenSlider.Update( ssrec->m_ULength.GetID() );
                    m_FeaSSRecWLenSlider.Update( ssrec->m_WLength.GetID() );
                    m_FeaSSRecThetaSlider.Update( ssrec->m_Theta.GetID() );
                    m_FeaSSRecCapToggle.Update( ssrec->m_IntersectionCapFlag.GetID() );
                    m_FeaSSRecRemoveTrisToggle.Update( ssrec->m_RemoveSubSurfTrisFlag.GetID() );

                    m_FeaSSRecPropertyChoice.Activate();
                    m_FeaSSRecRemoveTrisToggle.Activate();

                    if ( ssrec->m_TestType() == vsp::NONE )
                    {
                        m_FeaSSRecPropertyChoice.Deactivate();
                        ssrec->m_RemoveSubSurfTrisFlag.Set( false );
                        m_FeaSSRecRemoveTrisToggle.Deactivate();
                    }

                    if ( ssrec->m_RemoveSubSurfTrisFlag.Get() )
                    {
                        m_FeaSSRecPropertyChoice.Deactivate();
                    }

                    if ( ssrec->m_IntersectionCapFlag() )
                    {
                        m_FeaSSRecCapPropertyChoice.Activate();
                    }
                    else
                    {
                        m_FeaSSRecCapPropertyChoice.Deactivate();
                    }

                    FeaSubSurfDispGroup( &m_FeaSSRecGroup );
                }
                else if ( subsurf->GetType() == vsp::SS_ELLIPSE )
                {
                    SSEllipse* ssell = dynamic_cast< SSEllipse* >( subsurf );
                    assert( ssell );

                    m_FeaSSEllTestToggleGroup.Update( ssell->m_TestType.GetID() );
                    m_FeaSSEllTessSlider.Update( ssell->m_Tess.GetID() );
                    m_FeaSSEllCentUSlider.Update( ssell->m_CenterU.GetID() );
                    m_FeaSSEllCentWSlider.Update( ssell->m_CenterW.GetID() );
                    m_FeaSSEllULenSlider.Update( ssell->m_ULength.GetID() );
                    m_FeaSSEllWLenSlider.Update( ssell->m_WLength.GetID() );
                    m_FeaSSEllThetaSlider.Update( ssell->m_Theta.GetID() );
                    m_FeaSSEllCapToggle.Update( ssell->m_IntersectionCapFlag.GetID() );
                    m_FeaSSEllRemoveTrisToggle.Update( ssell->m_RemoveSubSurfTrisFlag.GetID() );

                    m_FeaSSEllPropertyChoice.Activate();
                    m_FeaSSEllRemoveTrisToggle.Activate();

                    if ( ssell->m_TestType() == vsp::NONE )
                    {
                        m_FeaSSEllPropertyChoice.Deactivate();
                        ssell->m_RemoveSubSurfTrisFlag.Set( false );
                        m_FeaSSEllRemoveTrisToggle.Deactivate();
                    }

                    if ( ssell->m_RemoveSubSurfTrisFlag.Get() )
                    {
                        m_FeaSSEllPropertyChoice.Deactivate();
                    }

                    if ( ssell->m_IntersectionCapFlag() )
                    {
                        m_FeaSSEllCapPropertyChoice.Activate();
                    }
                    else
                    {
                        m_FeaSSEllCapPropertyChoice.Deactivate();
                    }

                    FeaSubSurfDispGroup( &m_FeaSSEllGroup );
                }
                else if ( subsurf->GetType() == vsp::SS_CONTROL )
                {
                    SSControlSurf* sscon = dynamic_cast< SSControlSurf* >( subsurf );
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
                    m_FeaSSConCapToggle.Update( sscon->m_IntersectionCapFlag.GetID() );
                    m_FeaSSConRemoveTrisToggle.Update( sscon->m_RemoveSubSurfTrisFlag.GetID() );

                    m_FeaSSConPropertyChoice.Activate();
                    m_FeaSSConRemoveTrisToggle.Activate();

                    if ( sscon->m_TestType() == vsp::NONE )
                    {
                        m_FeaSSConPropertyChoice.Deactivate();
                        sscon->m_RemoveSubSurfTrisFlag.Set( false );
                        m_FeaSSConRemoveTrisToggle.Deactivate();
                    }

                    if ( sscon->m_RemoveSubSurfTrisFlag.Get() )
                    {
                        m_FeaSSConPropertyChoice.Deactivate();
                    }

                    if ( sscon->m_IntersectionCapFlag() )
                    {
                        m_FeaSSConCapPropertyChoice.Activate();
                    }
                    else
                    {
                        m_FeaSSConCapPropertyChoice.Deactivate();
                    }

                    m_FeaSSConSFracSlider.Deactivate();
                    m_FeaSSConSLenSlider.Deactivate();
                    m_FeaSSConEFracSlider.Deactivate();
                    m_FeaSSConELenSlider.Deactivate();

                    if ( sscon->m_AbsRelFlag() == ABS )
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
                    FeaSubSurfDispGroup( &m_FeaSSConGroup );
                }
            }
            else
            {
                FeaSubSurfDispGroup( NULL );
            }
        }
        else if ( StructureMgr.ValidTotalFeaStructInd( 0 ) )
        {
            FeaSubSurfDispGroup( NULL );
            FeaPartDispGroup( NULL );
            m_SelectedStructIndex = 0;
        }
        else
        {
            FeaSubSurfDispGroup( NULL );
            FeaPartDispGroup( NULL );
            m_SelectedStructIndex = -1;
        }

        //===== FeaProperty Update =====//
        UpdateFeaPropertyBrowser();
        UpdateFeaPropertyChoice();

        if ( StructureMgr.ValidFeaPropertyInd( m_SelectedPropertyIndex ) )
        {
            FeaProperty* fea_prop = StructureMgr.GetFeaPropertyVec()[m_SelectedPropertyIndex];
            if ( fea_prop )
            {
                m_FeaPropertyNameInput.Update( fea_prop->GetName() );

                if ( fea_prop->m_FeaPropertyType() == SHELL_PROPERTY )
                {
                    m_PropThickSlider.Update( fea_prop->m_Thickness.GetID() );

                    FeaPropertyDispGroup( &m_FeaPropertyShellGroup );
                }
                else if ( fea_prop->m_FeaPropertyType() == BEAM_PROPERTY )
                {
                    m_PropAreaSlider.Update( fea_prop->m_CrossSecArea.GetID() );
                    m_PropIzzSlider.Update( fea_prop->m_Izz.GetID() );
                    m_PropIyySlider.Update( fea_prop->m_Iyy.GetID() );
                    m_PropIzySlider.Update( fea_prop->m_Izy.GetID() );
                    m_PropIxxSlider.Update( fea_prop->m_Ixx.GetID() );

                    FeaPropertyDispGroup( &m_FeaPropertyBeamGroup );
                }
            }
            else
            {
                FeaPropertyDispGroup( NULL );
            }
        }
        else
        {
            FeaPropertyDispGroup( NULL );
        }

        //===== FeaMaterial Update =====//
        UpdateFeaMaterialBrowser();
        UpdateFeaMaterialChoice();

        if ( StructureMgr.ValidFeaMaterialInd( m_SelectedMaterialIndex ) )
        {
            FeaMaterial* fea_mat = StructureMgr.GetFeaMaterialVec()[m_SelectedMaterialIndex];
            if ( fea_mat )
            {
                m_FeaMaterialNameInput.Update( fea_mat->GetName() );
                m_MatDensitySlider.Update( fea_mat->m_MassDensity.GetID() );
                m_MatElasticModSlider.Update( fea_mat->m_ElasticModulus.GetID() );
                m_MatPoissonSlider.Update( fea_mat->m_PoissonRatio.GetID() );
                m_MatThermalExCoeffSlider.Update( fea_mat->m_ThermalExpanCoeff.GetID() );

                if ( fea_mat->m_UserFeaMaterial )
                {
                    m_FeaMaterialNameInput.Activate();
                    m_MatDensitySlider.Activate();
                    m_MatElasticModSlider.Activate();
                    m_MatPoissonSlider.Activate();
                    m_MatThermalExCoeffSlider.Activate();
                }
                else
                {
                    m_FeaMaterialNameInput.Deactivate();
                    m_MatDensitySlider.Deactivate();
                    m_MatElasticModSlider.Deactivate();
                    m_MatPoissonSlider.Deactivate();
                    m_MatThermalExCoeffSlider.Deactivate();
                }
            }
        }

        // Update Draw Objects for FeaParts
        UpdateDrawObjs();
    }

    if ( FeaMeshMgr.GetFeaMeshInProgress() )
    {
        m_FeaMeshExportButton.Deactivate();
    }
    else
    {
        m_FeaMeshExportButton.Activate();
    }

    return true;
}

string StructScreen::truncateFileName( const string &fn, int len )
{
    string trunc( fn );
    if ( (int)trunc.length() > len )
    {
        trunc.erase( 0, trunc.length() - len );
        trunc.replace( 0, 3, "..." );
    }
    return trunc;
}

void StructScreen::AddOutputText( const string &text )
{
    m_ConsoleBuffer->append( text.c_str() );
    m_ConsoleDisplay->insert_position( m_ConsoleDisplay->buffer()->length() );
    m_ConsoleDisplay->show_insert_position();
}

void StructScreen::CallBack( Fl_Widget* w )
{
    assert( m_ScreenMgr );

    Vehicle*  veh = m_ScreenMgr->GetVehiclePtr();

    if ( veh )
    {
        if ( w == m_FeaPartSelectBrowser )
        {
            if ( StructureMgr.ValidTotalFeaStructInd( m_SelectedStructIndex ) )
            {
                vector< FeaStructure* > structVec = StructureMgr.GetAllFeaStructs();

                // Check number of skins, which should be always equal to 1 unless the skin has not been initialized
                int num_skin = structVec[m_SelectedStructIndex]->GetNumFeaSkin();

                m_SelectedPartIndex = m_FeaPartSelectBrowser->value() - 2 + num_skin;

                FeaPart* feaprt = structVec[m_SelectedStructIndex]->GetFeaPart( m_SelectedPartIndex );

                if ( feaprt )
                {
                    m_FeaPartChoice.SetVal( feaprt->GetType() );
                    m_FeaCurrMainSurfIndx = feaprt->m_MainSurfIndx();
                }
            }
        }
        else if ( w == m_StructureSelectBrowser )
        {
            for ( unsigned int iCase = 1; iCase <= m_StructureSelectBrowser->size(); iCase++ )
            {
                if ( m_StructureSelectBrowser->selected( iCase ) )
                {
                    m_SelectedStructIndex = iCase - 2;
                    break;
                }
            }
        }
        else if ( w == m_DrawPartSelectBrowser )
        {
            int selected_index = m_DrawPartSelectBrowser->value();
            bool flag = !!m_DrawPartSelectBrowser->checked( selected_index );

            vector < string > draw_browser_name_vec = FeaMeshMgr.GetDrawBrowserNameVec();
            vector < int > draw_browser_index_vec = FeaMeshMgr.GetDrawBrowserIndexVec();

            if ( draw_browser_name_vec[selected_index - 1].find( "CAP" ) != std::string::npos )
            {
                FeaMeshMgr.SetDrawCapFlag( draw_browser_index_vec[selected_index - 1], flag );
            }
            else
            {
                FeaMeshMgr.SetDrawElementFlag( draw_browser_index_vec[selected_index - 1], flag );
            }
        }
        else if ( w == m_FeaSubSurfBrowser )
        {
            if ( StructureMgr.ValidTotalFeaStructInd( m_SelectedStructIndex ) )
            {
                vector< FeaStructure* > structVec = StructureMgr.GetAllFeaStructs();

                m_SelectedSubSurfIndex = m_FeaSubSurfBrowser->value() - 2;
                SubSurface* sub_surf = structVec[m_SelectedStructIndex]->GetFeaSubSurf( m_SelectedSubSurfIndex );
                if ( sub_surf )
                {
                    m_SelectedSubSurfChoice = sub_surf->GetType();
                }
            }
        }
        else if ( w == m_FeaPropertySelectBrowser )
        {
            for ( unsigned int iCase = 1; iCase <= m_FeaPropertySelectBrowser->size(); iCase++ )
            {
                if ( m_FeaPropertySelectBrowser->selected( iCase ) )
                {
                    m_SelectedPropertyIndex = iCase - 2;
                    break;
                }
            }
        }
        else if ( w == m_FeaMaterialSelectBrowser )
        {
            for ( unsigned int iCase = 1; iCase <= m_FeaMaterialSelectBrowser->size(); iCase++ )
            {
                if ( m_FeaMaterialSelectBrowser->selected( iCase ) )
                {
                    m_SelectedMaterialIndex = iCase - 1;
                    break;
                }
            }
        }
    }

    m_ScreenMgr->SetUpdateFlag( true );
}

#ifdef WIN32
DWORD WINAPI feamonitorfun( LPVOID data )
#else
void * feamonitorfun( void *data )
#endif
{
    StructScreen *cs = (StructScreen *)data;

    Vehicle* veh = VehicleMgr.GetVehicle();

    if ( cs && veh )
    {
        unsigned long nread = 1;

        bool running = true;

        while ( running || nread > 0 )
        {
            running = FeaMeshMgr.GetFeaMeshInProgress();
            nread = 0;

            std::streamoff ig = FeaMeshMgr.m_OutStream.tellg();
            FeaMeshMgr.m_OutStream.seekg( 0, FeaMeshMgr.m_OutStream.end );
            nread = (int)( FeaMeshMgr.m_OutStream.tellg() ) - ig;
            FeaMeshMgr.m_OutStream.seekg( ig );

            if ( nread > 0 )
            {
                char * buffer = new char[nread + 1];

                FeaMeshMgr.m_OutStream.read( buffer, nread );
                buffer[nread] = 0;

                Fl::lock();
                // Any FL calls must occur between Fl::lock() and Fl::unlock().
                cs->AddOutputText( buffer );
                Fl::unlock();

                delete[] buffer;
            }
            SleepForMilliseconds( 100 );
        }
        
        cs->GetScreenMgr()->SetUpdateFlag( true ); // FeaParts will not be updated when mesh is in progress
    }

    return 0;
}

#ifdef WIN32
DWORD WINAPI feamesh_thread_fun( LPVOID data )
#else
void * feamesh_thread_fun( void *data )
#endif
{
    FeaMeshMgr.GenerateFeaMesh();

    return 0;
}

void StructScreen::GuiDeviceCallBack( GuiDevice* device )
{
    assert( m_ScreenMgr );

    Vehicle*  veh = m_ScreenMgr->GetVehiclePtr();

    if ( !veh )
    {
        return;
    }

    if ( device == &m_FeaMeshExportButton )
    {
        // Set m_FeaMeshInProgress to ensure m_MonitorProcess does not terminate prematurely
        FeaMeshMgr.SetFeaMeshInProgress( true );

        // Identify which structure to mesh
        FeaMeshMgr.SetFeaMeshStructIndex( m_SelectedStructIndex );

        m_FeaMeshProcess.StartThread( feamesh_thread_fun, NULL );

        m_MonitorProcess.StartThread( feamonitorfun, ( void* ) this );

        if ( StructureMgr.ValidTotalFeaStructInd( m_SelectedStructIndex ) )
        {
            vector < FeaStructure* > structvec = StructureMgr.GetAllFeaStructs();

            structvec[m_SelectedStructIndex]->GetStructSettingsPtr()->m_DrawMeshFlag = true;
            structvec[m_SelectedStructIndex]->GetStructSettingsPtr()->m_DrawFeaPartsFlag = false;
        }
    }
    else if ( device == &m_GeomChoice )
    {
        m_SelectedGeomID = m_GeomIDVec[m_GeomChoice.GetVal()];
    }
    else if ( device == &m_SurfSel )
    {
        m_FeaCurrMainSurfIndx = m_SurfSel.GetVal();
    }
    else if ( device == &m_AddFeaStructButton )
    {
        Geom* currgeom = veh->FindGeom( m_SelectedGeomID );
        if ( currgeom )
        {
            FeaStructure* newstruct = currgeom->AddFeaStruct( true, m_FeaCurrMainSurfIndx );

            if ( newstruct )
            {
                m_SelectedStructIndex = StructureMgr.GetFeaStructIndex( newstruct );
            }
        }
    }
    else if ( device == &m_DelFeaStructButton )
    {
        vector < FeaStructure* > structvec = StructureMgr.GetAllFeaStructs();

        if ( StructureMgr.ValidTotalFeaStructInd( m_SelectedStructIndex ) )
        {
            FeaStructure* delstruct = structvec[m_SelectedStructIndex];

            if ( delstruct )
            {
                Geom* currgeom = veh->FindGeom( delstruct->GetParentGeomID() );

                if ( currgeom )
                {
                    vector < FeaStructure* > geomstructvec = currgeom->GetFeaStructVec();

                    for ( unsigned int i = 0; i < geomstructvec.size(); i++ )
                    {
                        if ( geomstructvec[i] == delstruct )
                        {
                            currgeom->DeleteFeaStruct( i );
                        }
                    }
                }
            }
        }

        m_SelectedStructIndex -= 1;

        if ( !StructureMgr.ValidTotalFeaStructInd( m_SelectedStructIndex ) )
        {
            m_SelectedStructIndex = -1;
        }
    }
    else if ( device == &m_FeaStructNameInput )
    {
        if ( StructureMgr.ValidTotalFeaStructInd( m_SelectedStructIndex ) )
        {
            vector < FeaStructure* > structvec = StructureMgr.GetAllFeaStructs();

            FeaStructure* feastruct = structvec[m_SelectedStructIndex];

            if ( feastruct )
            {
                feastruct->SetFeaStructName( m_FeaStructNameInput.GetString() );
            }
        }
    }
    else if ( device == &m_AddSpacedPartsButton )
    {
        if ( StructureMgr.ValidTotalFeaStructInd( m_SelectedStructIndex ) )
        {
            vector < FeaStructure* > structvec = StructureMgr.GetAllFeaStructs();

            FeaStructure* feastruct = structvec[m_SelectedStructIndex];

            if ( feastruct )
            {
                string message = feastruct->AddSpacedSlices( m_MultSliceOrientationChoice.GetVal() );
                AddOutputText( message );
            }
        }
    }
    else if ( device == &m_AddEvenlySpacedRibsButton )
    {
        if ( StructureMgr.ValidTotalFeaStructInd( m_SelectedStructIndex ) )
        {
            vector < FeaStructure* > structvec = StructureMgr.GetAllFeaStructs();

            FeaStructure* feastruct = structvec[m_SelectedStructIndex];

            if ( feastruct )
            {
                feastruct->AddEvenlySpacedRibs( feastruct->GetStructSettingsPtr()->m_NumEvenlySpacedPart.Get() );
            }
        }
    }
    else if ( device == &m_OrientWingButton )
    {
        OrientWing();
    }
    else if ( device == &m_OrientFrontButton )
    {
        OrientStructure( VSPGraphic::Common::VSP_CAM_FRONT );
    }
    else if ( device == &m_OrientSideButton )
    {
        OrientStructure( VSPGraphic::Common::VSP_CAM_LEFT );
    }
    else if ( device == &m_OrientTopButton )
    {
        OrientStructure( VSPGraphic::Common::VSP_CAM_TOP );
    }
    else if ( device == &m_FeaPartNameInput )
    {
        vector < FeaStructure* > structvec = StructureMgr.GetAllFeaStructs();

        FeaPart* feaprt = structvec[m_SelectedStructIndex]->GetFeaPart( m_SelectedPartIndex );

        if ( feaprt )
        {
            feaprt->SetName( m_FeaPartNameInput.GetString() );
        }
    }
    else if ( device == &m_FeaPartChoice )
    {
        m_SelectedFeaPartChoice = m_FeaPartChoice.GetVal();
    }
    else if ( device == &m_AddFeaPartButton )
    {
        if ( StructureMgr.ValidTotalFeaStructInd( m_SelectedStructIndex ) )
        {
            vector < FeaStructure* > structvec = StructureMgr.GetAllFeaStructs();

            FeaPart* feaprt = NULL;

            if ( m_FeaPartChoice.GetVal() == vsp::FEA_SLICE )
            {
                feaprt = structvec[m_SelectedStructIndex]->AddFeaPart( vsp::FEA_SLICE );
            }
            else if ( m_FeaPartChoice.GetVal() == vsp::FEA_RIB )
            {
                feaprt = structvec[m_SelectedStructIndex]->AddFeaPart( vsp::FEA_RIB );
            }
            else if ( m_FeaPartChoice.GetVal() == vsp::FEA_SPAR )
            {
                feaprt = structvec[m_SelectedStructIndex]->AddFeaPart( vsp::FEA_SPAR );
            }
            else if ( m_FeaPartChoice.GetVal() == vsp::FEA_FIX_POINT )
            {
                feaprt = structvec[m_SelectedStructIndex]->AddFeaPart( vsp::FEA_FIX_POINT );
            }

            structvec[m_SelectedStructIndex]->Update();

            m_SelectedPartIndex = structvec[m_SelectedStructIndex]->NumFeaParts() - 1;
        }
        else
        {
            AddOutputText( "Error: No Structure Selected\n" );
        }
    }
    else if ( device == &m_DelFeaPartButton )
    {
        if ( StructureMgr.ValidTotalFeaStructInd( m_SelectedStructIndex ) )
        {
            vector < FeaStructure* > structvec = StructureMgr.GetAllFeaStructs();

            structvec[m_SelectedStructIndex]->DelFeaPart( m_SelectedPartIndex );

            m_SelectedPartIndex -= 1;

            if ( !structvec[m_SelectedStructIndex]->ValidFeaPartInd( m_SelectedPartIndex ) )
            {
                m_SelectedPartIndex = -1;
            }
        }
    }
    else if ( device == &m_DrawAllButton )
    {
        vector < bool > draw_element_flag_vec = FeaMeshMgr.GetDrawElementFlagVec();
        for ( unsigned int i = 0; i < draw_element_flag_vec.size(); i++ )
        {
            FeaMeshMgr.SetDrawElementFlag( i, true );
        }

        vector < bool > draw_cap_flag_vec = FeaMeshMgr.GetDrawCapFlagVec();
        for ( unsigned int i = 0; i < draw_cap_flag_vec.size(); i++ )
        {
            FeaMeshMgr.SetDrawCapFlag( i, true );
        }
    }
    else if ( device == &m_HideAllButton )
    {
        vector < bool > draw_element_flag_vec = FeaMeshMgr.GetDrawElementFlagVec();
        for ( unsigned int i = 0; i < draw_element_flag_vec.size(); i++ )
        {
            FeaMeshMgr.SetDrawElementFlag( i, false );
        }

        vector < bool > draw_cap_flag_vec = FeaMeshMgr.GetDrawCapFlagVec();
        for ( unsigned int i = 0; i < draw_cap_flag_vec.size(); i++ )
        {
            FeaMeshMgr.SetDrawCapFlag( i, false );
        }
    }
    else if ( device == &m_RibPerpendicularEdgeChoice )
    {
        if ( StructureMgr.ValidTotalFeaStructInd( m_SelectedStructIndex ) )
        {
            vector < FeaStructure* > structVec = StructureMgr.GetAllFeaStructs();
            FeaPart* feaprt = structVec[m_SelectedStructIndex]->GetFeaPart( m_SelectedPartIndex );

            if ( feaprt )
            {
                if ( feaprt->GetType() == vsp::FEA_RIB )
                {
                    FeaRib* rib = dynamic_cast< FeaRib* >( feaprt );
                    assert( rib );

                    rib->SetPerpendicularEdgeID( m_PerpendicularEdgeVec[m_RibPerpendicularEdgeChoice.GetVal()] );
                }
            }
        }
    }
    else if ( device == &m_FixPointParentSurfChoice )
    {
        if ( StructureMgr.ValidTotalFeaStructInd( m_SelectedStructIndex ) )
        {
            vector < FeaStructure* > structVec = StructureMgr.GetAllFeaStructs();

            FeaPart* parent_feaprt = StructureMgr.GetFeaPart( m_FixPointParentIDVec[m_FixPointParentSurfChoice.GetVal()] );
            FeaPart* feaprt = structVec[m_SelectedStructIndex]->GetFeaPart( m_SelectedPartIndex );

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
    else if ( device == &m_FeaSubSurfChoice )
    {
        m_SelectedSubSurfChoice = m_FeaSubSurfChoice.GetVal();
    }
    else if ( device == &m_AddFeaSubSurfButton )
    {
        if ( StructureMgr.ValidTotalFeaStructInd( m_SelectedStructIndex ) )
        {
            vector < FeaStructure* > structvec = StructureMgr.GetAllFeaStructs();

            SubSurface* ssurf = NULL;

            if ( m_FeaSubSurfChoice.GetVal() == vsp::SS_LINE )
            {
                ssurf = structvec[m_SelectedStructIndex]->AddFeaSubSurf( vsp::SS_LINE );
            }
            else if ( m_FeaSubSurfChoice.GetVal() == vsp::SS_RECTANGLE )
            {
                ssurf = structvec[m_SelectedStructIndex]->AddFeaSubSurf( vsp::SS_RECTANGLE );
            }
            else if ( m_FeaSubSurfChoice.GetVal() == vsp::SS_ELLIPSE )
            {
                ssurf = structvec[m_SelectedStructIndex]->AddFeaSubSurf( vsp::SS_ELLIPSE );
            }
            else if ( m_FeaSubSurfChoice.GetVal() == vsp::SS_CONTROL )
            {
                ssurf = structvec[m_SelectedStructIndex]->AddFeaSubSurf( vsp::SS_CONTROL );
            }

            if ( ssurf )
            {
                ssurf->Update();
            }
            m_SelectedSubSurfIndex = structvec[m_SelectedStructIndex]->NumFeaSubSurfs() - 1;
        }
        else
        {
            AddOutputText( "Error: No Structure Selected\n" );
        }
    }
    else if ( device == &m_DelFeaSubSurfButton )
    {
        if ( StructureMgr.ValidTotalFeaStructInd( m_SelectedStructIndex ) )
        {
            vector < FeaStructure* > structvec = StructureMgr.GetAllFeaStructs();
            structvec[m_SelectedStructIndex]->DelFeaSubSurf( m_SelectedSubSurfIndex );

            m_SelectedSubSurfIndex -= 1;

            if ( !structvec[m_SelectedStructIndex]->ValidFeaSubSurfInd( m_SelectedSubSurfIndex ) )
            {
                m_SelectedSubSurfIndex = -1;
            }
        }
    }
    else if ( device == &m_MoveSSUpButton )
    {
        if ( StructureMgr.ValidTotalFeaStructInd( m_SelectedStructIndex ) )
        {
            vector < FeaStructure* > structvec = StructureMgr.GetAllFeaStructs();
            structvec[m_SelectedStructIndex]->ReorderFeaSubSurf( m_SelectedSubSurfIndex, Vehicle::REORDER_MOVE_UP );
            m_SelectedSubSurfIndex--;
        }
    }
    else if ( device == &m_MoveSSDownButton )
    {
        if ( StructureMgr.ValidTotalFeaStructInd( m_SelectedStructIndex ) )
        {
            vector < FeaStructure* > structvec = StructureMgr.GetAllFeaStructs();
            structvec[m_SelectedStructIndex]->ReorderFeaSubSurf( m_SelectedSubSurfIndex, Vehicle::REORDER_MOVE_DOWN );
            m_SelectedSubSurfIndex++;
        }
    }
    else if ( device == &m_MoveSSTopButton )
    {
        if ( StructureMgr.ValidTotalFeaStructInd( m_SelectedStructIndex ) )
        {
            vector < FeaStructure* > structvec = StructureMgr.GetAllFeaStructs();
            structvec[m_SelectedStructIndex]->ReorderFeaSubSurf( m_SelectedSubSurfIndex, Vehicle::REORDER_MOVE_TOP );
            m_SelectedSubSurfIndex = 0;
        }
    }
    else if ( device == &m_MoveSSBotButton )
    {
        if ( StructureMgr.ValidTotalFeaStructInd( m_SelectedStructIndex ) )
        {
            vector < FeaStructure* > structvec = StructureMgr.GetAllFeaStructs();
            structvec[m_SelectedStructIndex]->ReorderFeaSubSurf( m_SelectedSubSurfIndex, Vehicle::REORDER_MOVE_BOTTOM );
            m_SelectedSubSurfIndex = structvec[m_SelectedStructIndex]->NumFeaSubSurfs() - 1;;
        }
    }
    else if ( device == &m_FeaSubNameInput )
    {
        if ( StructureMgr.ValidTotalFeaStructInd( m_SelectedStructIndex ) )
        {
            vector < FeaStructure* > structvec = StructureMgr.GetAllFeaStructs();
            SubSurface* sub_surf = structvec[m_SelectedStructIndex]->GetFeaSubSurf( m_SelectedSubSurfIndex );

            if ( sub_surf )
            {
                sub_surf->SetName( m_FeaSubNameInput.GetString() );
            }
        }
    }
    else if ( device == &m_AddFeaPropertyButton )
    {
        StructureMgr.AddFeaProperty( m_FeaPropertyType.GetVal() );
        m_SelectedPropertyIndex = StructureMgr.NumFeaProperties() - 1;
    }
    else if ( device == &m_DelFeaPropertyButton )
    {
        if ( StructureMgr.ValidFeaPropertyInd( m_SelectedPropertyIndex ) )
        {
            StructureMgr.DeleteFeaProperty( m_SelectedPropertyIndex );

            m_SelectedPropertyIndex -= 1;

            if ( !StructureMgr.ValidFeaPropertyInd( m_SelectedPropertyIndex ) )
            {
                m_SelectedPropertyIndex = -1;
            }
        }
    }
    else if ( device == &m_FeaPropertyNameInput )
    {
        if ( StructureMgr.ValidFeaPropertyInd( m_SelectedPropertyIndex ) )
        {
            vector < FeaProperty* > property_vec = StructureMgr.GetFeaPropertyVec();
            FeaProperty* fea_prop = property_vec[m_SelectedPropertyIndex];

            if ( fea_prop )
            {
                fea_prop->SetName( m_FeaPropertyNameInput.GetString() );
            }
        }
    }
    else if ( device == &m_SkinPropertyChoice )
    {
        if ( StructureMgr.ValidTotalFeaStructInd( m_SelectedStructIndex ) )
        {
            vector < FeaStructure* > structvec = StructureMgr.GetAllFeaStructs();
            FeaPart* feaskin = structvec[m_SelectedStructIndex]->GetFeaSkin();

            if ( feaskin )
            {
                feaskin->SetFeaPropertyIndex( m_SkinPropertyChoice.GetVal() );
            }
        }
    }
    else if ( device == &m_MultSlicePropChoice )
    {
        if ( StructureMgr.ValidTotalFeaStructInd( m_SelectedStructIndex ) )
        {
            vector < FeaStructure* > structvec = StructureMgr.GetAllFeaStructs();

            if ( structvec[m_SelectedStructIndex]->GetStructSettingsPtr() )
            {
                structvec[m_SelectedStructIndex]->GetStructSettingsPtr()->SetMultPropertyIndex( m_MultSlicePropChoice.GetVal() );
            }
        }
    }
    else if ( device == &m_MultSliceCapPropChoice )
    {
        if ( StructureMgr.ValidTotalFeaStructInd( m_SelectedStructIndex ) )
        {
            vector < FeaStructure* > structvec = StructureMgr.GetAllFeaStructs();

            if ( structvec[m_SelectedStructIndex]->GetStructSettingsPtr() )
            {
                structvec[m_SelectedStructIndex]->GetStructSettingsPtr()->SetMultCapPropertyIndex( m_MultSliceCapPropChoice.GetVal() );
            }
        }
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
    else if ( device == &m_FeaSSLinePropertyChoice )
    {
        UpdateFeaSubSurfPropertyIndex( &m_FeaSSLinePropertyChoice );
    }
    else if ( device == &m_FeaSSRecPropertyChoice )
    {
        UpdateFeaSubSurfPropertyIndex( &m_FeaSSRecPropertyChoice );
    }
    else if ( device == &m_FeaSSEllPropertyChoice )
    {
        UpdateFeaSubSurfPropertyIndex( &m_FeaSSEllPropertyChoice );
    }
    else if ( device == &m_FeaSSConPropertyChoice )
    {
        UpdateFeaSubSurfPropertyIndex( &m_FeaSSConPropertyChoice );
    }
    else if ( device == &m_FeaSSLineCapPropertyChoice )
    {
        UpdateFeaSubSurfCapPropertyIndex( &m_FeaSSLineCapPropertyChoice );
    }
    else if ( device == &m_FeaSSRecCapPropertyChoice )
    {
        UpdateFeaSubSurfCapPropertyIndex( &m_FeaSSRecCapPropertyChoice );
    }
    else if ( device == &m_FeaSSEllCapPropertyChoice )
    {
        UpdateFeaSubSurfCapPropertyIndex( &m_FeaSSEllCapPropertyChoice );
    }
    else if ( device == &m_FeaSSConCapPropertyChoice )
    {
        UpdateFeaSubSurfCapPropertyIndex( &m_FeaSSConCapPropertyChoice );
    }
    else if ( device == &m_AddFeaMaterialButton )
    {
        StructureMgr.AddFeaMaterial();
        m_SelectedMaterialIndex = StructureMgr.NumFeaMaterials() - 1;
    }
    else if ( device == &m_DelFeaMaterialButton )
    {
        if ( StructureMgr.ValidFeaMaterialInd( m_SelectedMaterialIndex ) )
        {
            StructureMgr.DeleteFeaMaterial( m_SelectedMaterialIndex );

            m_SelectedMaterialIndex -= 1;

            if ( !StructureMgr.ValidFeaMaterialInd( m_SelectedMaterialIndex ) )
            {
                m_SelectedMaterialIndex = -1;
            }
        }
    }
    else if ( device == &m_FeaMaterialNameInput )
    {
        if ( StructureMgr.ValidFeaMaterialInd( m_SelectedMaterialIndex ) )
        {
            vector < FeaMaterial* > material_vec = StructureMgr.GetFeaMaterialVec();
            FeaMaterial* fea_mat = material_vec[m_SelectedMaterialIndex];

            if ( fea_mat )
            {
                fea_mat->SetName( m_FeaMaterialNameInput.GetString() );
            }
        }
    }
    else if ( device == &m_FeaShellMaterialChoice )
    {
        if ( StructureMgr.ValidFeaPropertyInd( m_SelectedPropertyIndex ) )
        {
            FeaProperty* fea_prop = StructureMgr.GetFeaPropertyVec()[m_SelectedPropertyIndex];

            if ( fea_prop )
            {
                fea_prop->SetFeaMaterialIndex( m_FeaShellMaterialChoice.GetVal() );
            }
        }
    }
    else if ( device == &m_FeaBeamMaterialChoice )
    {
        if ( StructureMgr.ValidFeaPropertyInd( m_SelectedPropertyIndex ) )
        {
            FeaProperty* fea_prop = StructureMgr.GetFeaPropertyVec()[m_SelectedPropertyIndex];

            if ( fea_prop )
            {
                fea_prop->SetFeaMaterialIndex( m_FeaBeamMaterialChoice.GetVal() );
            }
        }
    }
    else if ( device == &m_SelectStlFile )
    {
        if ( StructureMgr.ValidTotalFeaStructInd( m_SelectedStructIndex ) )
        {
            vector < FeaStructure* > structvec = StructureMgr.GetAllFeaStructs();

            string newfile = m_ScreenMgr->GetSelectFileScreen()->FileChooser( "Select .stl file.", "*.stl" );
            if ( newfile.compare( "" ) != 0 )
            {
                structvec[m_SelectedStructIndex]->GetStructSettingsPtr()->SetExportFileName( newfile, vsp::STL_FEA_NAME );
            }
        }
    }
    else if ( device == &m_SelectMassFile )
    {
        if ( StructureMgr.ValidTotalFeaStructInd( m_SelectedStructIndex ) )
        {
            vector < FeaStructure* > structvec = StructureMgr.GetAllFeaStructs();

            string newfile = m_ScreenMgr->GetSelectFileScreen()->FileChooser( "Select mass .dat file.", "*.dat" );
            if ( newfile.compare( "" ) != 0 )
            {
                structvec[m_SelectedStructIndex]->GetStructSettingsPtr()->SetExportFileName( newfile, vsp::MASS_FILE_NAME );
            }
        }
    }
    else if ( device == &m_SelectNastFile )
    {
        if ( StructureMgr.ValidTotalFeaStructInd( m_SelectedStructIndex ) )
        {
            vector < FeaStructure* > structvec = StructureMgr.GetAllFeaStructs();

            string newfile = m_ScreenMgr->GetSelectFileScreen()->FileChooser( "Select NASTRAN .dat file.", "*.dat" );
            if ( newfile.compare( "" ) != 0 )
            {
                structvec[m_SelectedStructIndex]->GetStructSettingsPtr()->SetExportFileName( newfile, vsp::NASTRAN_FILE_NAME );
            }
        }
    }
    else if ( device == &m_SelectCalcFile  )
    {
        if ( StructureMgr.ValidTotalFeaStructInd( m_SelectedStructIndex ) )
        {
            vector < FeaStructure* > structvec = StructureMgr.GetAllFeaStructs();

            string newfile = m_ScreenMgr->GetSelectFileScreen()->FileChooser( "Select Calculix .dat file.", "*.dat" );
            if ( newfile.compare( "" ) != 0 )
            {
                structvec[m_SelectedStructIndex]->GetStructSettingsPtr()->SetExportFileName( newfile, vsp::CALCULIX_FILE_NAME );
            }
        }
    }
    else if ( device == &m_SelectGmshFile )
    {
        if ( StructureMgr.ValidTotalFeaStructInd( m_SelectedStructIndex ) )
        {
            vector < FeaStructure* > structvec = StructureMgr.GetAllFeaStructs();

            string newfile = m_ScreenMgr->GetSelectFileScreen()->FileChooser( "Select .msh file.", "*.msh" );
            if ( newfile.compare( "" ) != 0 )
            {
                structvec[m_SelectedStructIndex]->GetStructSettingsPtr()->SetExportFileName( newfile, vsp::GMSH_FEA_NAME );
            }
        }
    }

    m_ScreenMgr->SetUpdateFlag( true );
}

void StructScreen::OrientStructure( VSPGraphic::Common::VSPenum type )
{
    if ( !StructureMgr.ValidTotalFeaStructInd( m_SelectedStructIndex ) )
    {
        return;
    }

    FeaStructure* curr_struct = StructureMgr.GetAllFeaStructs()[m_SelectedStructIndex];
    Vehicle* veh = m_ScreenMgr->GetVehiclePtr();

    if ( veh && curr_struct )
    {
        Geom* curr_geom = veh->FindGeom( curr_struct->GetParentGeomID() );

        MainVSPScreen* main = dynamic_cast<MainVSPScreen*>( m_ScreenMgr->GetScreen( m_ScreenMgr->VSP_MAIN_SCREEN ) );

        if ( curr_geom && main )
        {
            if ( type == VSPGraphic::Common::VSP_CAM_FRONT )
            {
                curr_geom->m_GuiDraw.SetDisplayType( GeomGuiDraw::DISPLAY_DEGEN_SURF );
            }
            else
            {
                curr_geom->m_GuiDraw.SetDisplayType( GeomGuiDraw::DISPLAY_DEGEN_CAMBER );
            }

            curr_geom->m_GuiDraw.SetDrawType( GeomGuiDraw::GEOM_DRAW_SHADE );
            curr_geom->m_GuiDraw.SetDispSubSurfFlag( true );

            veh->ShowSet( 0 );
            veh->HideAllExcept( curr_struct->GetParentGeomID() );
            veh->ClearActiveGeom();

            VSPGUI::VspGlWindow * glwin = main->GetGLWindow();

            if ( !glwin )
            {
                return;
            }

            glwin->setView( type );

            BndBox bbox = curr_geom->GetBndBox();

            vec3d p = bbox.GetCenter();
            double d = bbox.DiagDist();
            int wid = glwin->pixel_w();
            int ht = glwin->pixel_h();

            float z = d * ( wid < ht ? 1.f / wid : 1.f / ht );

            // Validate pointers
            if ( !glwin->getGraphicEngine() )
            {
                return;
            }
            if ( !glwin->getGraphicEngine()->getDisplay() )
            {
                return;
            }
            if ( !glwin->getGraphicEngine()->getDisplay()->getCamera() )
            {
                return;
            }

            glwin->getGraphicEngine()->getDisplay()->setCOR( -p.x(), -p.y(), -p.z() );
            glwin->getGraphicEngine()->getDisplay()->relativePan( 0.0f, 0.0f );
            glwin->getGraphicEngine()->getDisplay()->getCamera()->relativeZoom( z );

            ManageViewScreen * viewScreen = dynamic_cast<ManageViewScreen*>( m_ScreenMgr->GetScreen( ScreenMgr::VSP_VIEW_SCREEN ) );

            if ( viewScreen )
            {
                viewScreen->UpdateCOR();
                viewScreen->UpdatePan();
                viewScreen->UpdateZoom();
            }
        }
    }
}

void StructScreen::OrientWing()
{
    if ( !StructureMgr.ValidTotalFeaStructInd( m_SelectedStructIndex ) )
    {
        return;
    }

    FeaStructure* curr_struct = StructureMgr.GetAllFeaStructs()[m_SelectedStructIndex];
    Vehicle* veh = m_ScreenMgr->GetVehiclePtr();

    if ( veh && curr_struct )
    {
        Geom* current_wing = veh->FindGeom( curr_struct->GetParentGeomID() );

        if ( !current_wing )
        {
            return;
        }

        if ( current_wing->GetType().m_Type == MS_WING_GEOM_TYPE )
        {
            BndBox wing_bbox = current_wing->GetBndBox();

            double del_y = wing_bbox.GetMax( 1 ) - wing_bbox.GetMin( 1 );
            double del_z = wing_bbox.GetMax( 2 ) - wing_bbox.GetMin( 2 );

            VSPGraphic::Common::VSPenum type;

            // Select a side view if the wing geom is a vertical stabilizer
            if ( del_z > del_y )
            {
                type = VSPGraphic::Common::VSP_CAM_LEFT;
            }
            else
            {
                type = VSPGraphic::Common::VSP_CAM_TOP;
            }

            OrientStructure( type );
        }
    }
}

void StructScreen::LoadDrawObjs( vector< DrawObj* > &draw_obj_vec )
{
    Vehicle* veh = m_ScreenMgr->GetVehiclePtr();

    if ( !veh )
    {
        return;
    }

    if ( IsShown() )
    {
        Update(); // Updating right before drawing ensures the correct structure is highlighted

        if ( !FeaMeshMgr.GetFeaMeshInProgress() && StructureMgr.ValidTotalFeaStructInd( m_SelectedStructIndex ) )
        {
            vector < FeaStructure* > totalstructvec = StructureMgr.GetAllFeaStructs();

            FeaStructure* curr_struct = totalstructvec[m_SelectedStructIndex];

            if ( !curr_struct )
            {
                return;
            }

            if ( curr_struct->GetStructSettingsPtr()->m_DrawFeaPartsFlag() )
            {
                vector < FeaPart* > partvec = curr_struct->GetFeaPartVec();

                for ( unsigned int i = 0; i < (int)partvec.size(); i++ )
                {
                    partvec[i]->LoadDrawObjs( draw_obj_vec );
                }

                vector < SubSurface* > subsurf_vec = curr_struct->GetFeaSubSurfVec();
                curr_struct->RecolorFeaSubSurfs( m_SelectedSubSurfIndex );

                for ( unsigned int i = 0; i < (int)subsurf_vec.size(); i++ )
                {
                    subsurf_vec[i]->LoadDrawObjs( draw_obj_vec );
                }
            }
        }

        // Load Draw Objects for FeaMesh
        FeaMeshMgr.LoadDrawObjs( draw_obj_vec );
    }
}

void StructScreen::UpdateDrawObjs()
{
    if ( FeaMeshMgr.GetFeaMeshInProgress() == true )
    {
        return;
    }

    Vehicle* veh = m_ScreenMgr->GetVehiclePtr();

    if ( !veh )
    {
        return;
    }

    if ( StructureMgr.ValidTotalFeaStructInd( m_SelectedStructIndex ) )
    {
        vector < FeaStructure* > totalstructvec = StructureMgr.GetAllFeaStructs();

        FeaStructure* curr_struct = totalstructvec[m_SelectedStructIndex];

        if ( !curr_struct )
        {
            return;
        }

        if ( curr_struct->GetStructSettingsPtr()->m_DrawFeaPartsFlag() )
        {
            int k = 0;

            FeaPart* curr_part = curr_struct->GetFeaPart( m_SelectedPartIndex );

            vector < FeaPart* > partvec = curr_struct->GetFeaPartVec();

            for ( unsigned int i = 0; i < (int)partvec.size(); i++ )
            {
                if ( partvec[i] == curr_part )
                    partvec[i]->UpdateDrawObjs( k, true );
                else
                    partvec[i]->UpdateDrawObjs( k, false );
                k++;
            }
        }
    }
}
