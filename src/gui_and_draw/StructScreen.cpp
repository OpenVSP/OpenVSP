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
#include "FeaPartEditScreen.h"
#include "ParmMgr.h"

using namespace vsp;

//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

StructScreen::StructScreen( ScreenMgr* mgr ) : TabScreen( mgr, 430, 625, "FEA Mesh", 150 )
{
    m_FLTK_Window->callback( staticCloseCB, this );

    int border = 5;
    int labelfont = 13;
    int textsize = 12;
    int labelsize = 12;

    Fl_Group* structTab = AddTab( "Structure" );
    Fl_Group* structTabGroup = AddSubGroup( structTab, border );
    Fl_Group* partTab = AddTab( "Part" );
    Fl_Group* partTabGroup = AddSubGroup( partTab, border );
    Fl_Group* matTab = AddTab( "Material" );
    Fl_Group* matTabGroup = AddSubGroup( matTab, border );
    Fl_Group* propTab = AddTab( "Property" );
    Fl_Group* propTabGroup = AddSubGroup( propTab, border );
    Fl_Group* meshTab = AddTab( "Mesh" );
    Fl_Group* meshTabGroup = AddSubGroup( meshTab, border );
    Fl_Group* outputTab = AddTab( "Output" );
    Fl_Group* outputTabGroup = AddSubGroup( outputTab, border );
    Fl_Group* displayTab = AddTab( "Display" );
    Fl_Group* displayTabGroup = AddSubGroup( displayTab, border );

    //=== Create Console Area ===//
    m_ConsoleLayout.SetGroupAndScreen( m_FLTK_Window, this );

    m_ConsoleLayout.AddY( m_ConsoleLayout.GetRemainY()
                        - 7 * m_ConsoleLayout.GetStdHeight()
                        - 2 * m_ConsoleLayout.GetGapHeight() );

    m_ConsoleLayout.AddYGap();
    m_ConsoleLayout.AddX( border );

    m_ConsoleLayout.AddSubGroupLayout( m_BorderConsoleLayout, m_ConsoleLayout.GetRemainX() - border,
                                       m_ConsoleLayout.GetRemainY() - border );

    m_ConsoleDisplay = m_BorderConsoleLayout.AddFlTextDisplay( 115 );
    m_ConsoleBuffer = new Fl_Text_Buffer;
    m_ConsoleDisplay->buffer( m_ConsoleBuffer );

    m_BorderConsoleLayout.AddYGap();

    m_BorderConsoleLayout.SetSameLineFlag( true );
    m_BorderConsoleLayout.SetFitWidthFlag( false );

    m_BorderConsoleLayout.SetButtonWidth( m_BorderConsoleLayout.GetW() / 4 );
    m_BorderConsoleLayout.SetInputWidth( m_BorderConsoleLayout.GetW() / 4 );

    m_BorderConsoleLayout.AddOutput( m_CurrStructOutput, "Current Structure" );
    m_BorderConsoleLayout.AddButton( m_FeaMeshExportButton, "Mesh and Export" );
    m_BorderConsoleLayout.AddButton( m_ResetDisplayButton, "Reset Display" );

    //=== Structures Tab ===//
    structTab->show();

    m_StructureTabLayout.SetGroupAndScreen( structTabGroup, this );

    m_StructureTabLayout.AddDividerBox( "General" );

    m_StructureTabLayout.AddButton( m_WikiLinkButton, "Link to FEA Mesh Wiki Documentation" );

    m_StructureTabLayout.AddYGap();

    m_StructureTabLayout.SetSameLineFlag( true );
    m_StructureTabLayout.SetFitWidthFlag( false );
    m_StructureTabLayout.SetChoiceButtonWidth( m_StructureTabLayout.GetRemainX() / 2 );
    m_StructureTabLayout.SetSliderWidth( m_StructureTabLayout.GetRemainX() / 2 );

    m_StructUnitChoice.AddItem( "SI (kg, m)" );
    m_StructUnitChoice.AddItem( "CGS (g, cm)" );
    m_StructUnitChoice.AddItem( "MPA (tonne, mm)" );
    m_StructUnitChoice.AddItem( "BFT (slug, ft)" );
    string squared( 1, 178 );
    string bin_name = "BIN (lbf*sec" + squared + "\\/in, in)";
    m_StructUnitChoice.AddItem( bin_name );
    m_StructureTabLayout.AddChoice( m_StructUnitChoice, "Unit System (Mass, Length)" );

    m_StructureTabLayout.SetSameLineFlag( false );
    m_StructureTabLayout.SetFitWidthFlag( true );

    m_StructureTabLayout.ForceNewLine();
    m_StructureTabLayout.AddYGap();

    m_StructureTabLayout.AddDividerBox( "Structure Selection" );

    int browser_h = 150;

    m_StructureSelectBrowser = m_StructureTabLayout.AddFlBrowser( browser_h );
    m_StructureSelectBrowser->type( FL_HOLD_BROWSER );
    m_StructureSelectBrowser->labelfont( labelfont );
    m_StructureSelectBrowser->labelsize( labelsize );
    m_StructureSelectBrowser->textsize( textsize );
    m_StructureSelectBrowser->callback( staticScreenCB, this );

    int buttonwidth = m_StructureTabLayout.GetButtonWidth();

    m_StructureTabLayout.SetChoiceButtonWidth( buttonwidth );
    m_StructureTabLayout.SetSliderWidth( ( m_StructureTabLayout.GetW() - ( 2 * m_StructureTabLayout.GetButtonWidth() ) ) / 2 );

    m_StructureTabLayout.SetSameLineFlag( true );
    m_StructureTabLayout.SetFitWidthFlag( false );

    m_StructureTabLayout.AddChoice( m_GeomChoice, "Geom" );
    m_StructureTabLayout.AddChoice( m_SurfSel, "Surface" );

    m_StructureTabLayout.ForceNewLine();

    m_StructureTabLayout.SetButtonWidth( m_StructureTabLayout.GetRemainX() / 2 );

    m_StructureTabLayout.AddButton( m_AddFeaStructButton, "Add Structure" );
    m_StructureTabLayout.AddButton( m_DelFeaStructButton, "Delete Structure" );

    m_StructureTabLayout.ForceNewLine();
    m_StructureTabLayout.AddYGap();

    m_StructGroup.SetGroupAndScreen( AddSubGroup( structTab, 5 ), this );
    m_StructGroup.SetY( m_StructureTabLayout.GetY() );

    buttonwidth *= 2;

    m_StructGroup.SetButtonWidth( buttonwidth );
    m_StructGroup.SetChoiceButtonWidth( buttonwidth );

    m_StructGroup.AddInput( m_FeaStructNameInput, "Struct Name" );

    m_StructGroup.AddYGap();

    m_StructGeneralGroup.SetGroupAndScreen( AddSubGroup( structTab, 5 ), this );
    m_StructGeneralGroup.SetY( m_StructGroup.GetY() );

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

    m_StructWingGroup.AddDividerBox( "Orientation" );

    m_StructWingGroup.AddButton( m_OrientWingButton, "Orient Wing" );

    m_StructWingGroup.ForceNewLine();

    //=== Parts Tab ===//
    m_PartTabLayout.SetGroupAndScreen( partTabGroup, this );

    m_PartTabLayout.AddDividerBox( "FEA Part Selection" );

    int start_x = m_PartTabLayout.GetX();
    int start_y = m_PartTabLayout.GetY();

    m_PartTabLayout.AddSubGroupLayout( m_MovePartButtonLayout, 20, browser_h );
    m_PartTabLayout.AddY( browser_h );

    m_MovePartButtonLayout.SetSameLineFlag( false );
    m_MovePartButtonLayout.SetFitWidthFlag( false );

    m_MovePartButtonLayout.SetStdHeight( 20 );
    m_MovePartButtonLayout.SetButtonWidth( 20 );
    m_MovePartButtonLayout.AddButton( m_MovePrtTopButton, "@2<<" );
    m_MovePartButtonLayout.AddYGap();
    m_MovePartButtonLayout.AddButton( m_MovePrtUpButton, "@2<" );
    m_MovePartButtonLayout.AddY( browser_h - 75 - m_PartTabLayout.GetStdHeight() );
    m_MovePartButtonLayout.AddButton( m_MovePrtDownButton, "@2>" );
    m_MovePartButtonLayout.AddYGap();
    m_MovePartButtonLayout.AddButton( m_MovePrtBotButton, "@2>>" );

    m_PartTabLayout.SetY( start_y );
    m_PartTabLayout.AddX( 20 );
    m_PartTabLayout.SetFitWidthFlag( true );

    m_PartTabLayout.AddSubGroupLayout( m_FeaPartBrowserLayout, m_PartTabLayout.GetRemainX(), browser_h );
    m_PartTabLayout.AddY( browser_h );

    m_FeaPartSelectBrowser = m_FeaPartBrowserLayout.AddFlBrowser( browser_h );
    m_FeaPartSelectBrowser->type( FL_MULTI_BROWSER );
    m_FeaPartSelectBrowser->labelfont( labelfont );
    m_FeaPartSelectBrowser->labelsize( labelsize );
    m_FeaPartSelectBrowser->textsize( textsize );
    m_FeaPartSelectBrowser->callback( staticScreenCB, this );

    m_PartTabLayout.SetX( start_x );

    m_PartTabLayout.SetSameLineFlag( true );
    m_PartTabLayout.SetFitWidthFlag( false );

    m_PartTabLayout.SetButtonWidth( m_StructGroup.GetRemainX() / 4 );
    m_PartTabLayout.SetChoiceButtonWidth( m_StructGroup.GetRemainX() / 4 );
    m_PartTabLayout.SetSliderWidth( m_StructGroup.GetRemainX() / 4 );

    m_PartTabLayout.AddChoice( m_FeaPartChoice, "Type" );
    m_PartTabLayout.AddButton( m_AddFeaPartButton, "Add Part" );
    m_PartTabLayout.AddButton( m_DelFeaPartButton, "Delete Part" );

    m_PartTabLayout.ForceNewLine();
    m_PartTabLayout.AddYGap();

    //==== General FeaPart Parameters ====//
    m_PartTabLayout.AddSubGroupLayout( m_PartGroup, m_PartTabLayout.GetRemainX(), m_PartTabLayout.GetRemainY() );

    m_PartGroup.SetSameLineFlag( true );
    m_PartGroup.SetFitWidthFlag( false );

    m_PartGroup.SetButtonWidth( m_PartGroup.GetRemainX() / 3 );
    m_PartGroup.SetInputWidth( m_PartGroup.GetRemainX() / 3 );

    m_PartGroup.AddInput( m_FeaPartNameInput, "Part Name" );
    m_PartGroup.AddButton( m_EditFeaPartButton, "Edit Part" );

    m_PartGroup.ForceNewLine();
    m_PartGroup.AddYGap();

    m_PartGroup.SetSameLineFlag( false );
    m_PartGroup.SetFitWidthFlag( true );

    m_PartGroup.AddDividerBox( "General" );

    m_PartGroup.SetSameLineFlag( true );
    m_PartGroup.SetFitWidthFlag( false );

    m_PartGroup.SetButtonWidth( m_PartTabLayout.GetRemainX() / 3 );
    m_PartGroup.AddButton( m_ShellToggle, "Shell" );
    m_PartGroup.AddButton( m_CapToggle, "Cap" );
    m_PartGroup.AddButton( m_ShellCapToggle, "Shell and Cap" );

    m_ShellCapToggleGroup.Init( this );
    m_ShellCapToggleGroup.AddButton( m_ShellToggle.GetFlButton() );
    m_ShellCapToggleGroup.AddButton( m_CapToggle.GetFlButton() );
    m_ShellCapToggleGroup.AddButton( m_ShellCapToggle.GetFlButton() );

    m_PartGroup.SetSameLineFlag( false );
    m_PartGroup.SetFitWidthFlag( true );

    m_PartGroup.ForceNewLine();
    m_PartGroup.AddYGap();

    m_PartGroup.SetSliderWidth( m_PartGroup.GetRemainX() / 2 );
    m_PartGroup.SetChoiceButtonWidth( m_PartGroup.GetRemainX() / 2 );

    m_PartGroup.AddChoice( m_GenPropertyChoice, "Property" );
    m_PartGroup.AddChoice( m_GenCapPropertyChoice, "Cap Property" );

    m_PartGroup.AddYGap();

    m_PartGroup.SetSameLineFlag( true );
    m_PartGroup.SetFitWidthFlag( false );

    m_PartGroup.SetButtonWidth( m_PartGroup.GetRemainX() / 2 );

    m_PartGroup.AddButton( m_ShowFeaPartButton, "Show" );
    m_PartGroup.AddButton( m_HideFeaPartButton, "Hide" );

    m_DispFeaPartGroup.Init( this );
    m_DispFeaPartGroup.AddButton( m_HideFeaPartButton.GetFlButton() );
    m_DispFeaPartGroup.AddButton( m_ShowFeaPartButton.GetFlButton() );

    //=== Material Tab ===//
    m_MaterialTabLayout.SetGroupAndScreen( matTabGroup, this );

    m_MaterialTabLayout.AddDividerBox( "Material Selection" );

    m_MaterialEditGroup.SetGroupAndScreen( AddSubGroup( matTabGroup, 5 ), this );

    m_MaterialEditGroup.SetY( m_MaterialTabLayout.GetY() );

    m_FeaMaterialSelectBrowser = m_MaterialEditGroup.AddFlBrowser( browser_h );
    m_FeaMaterialSelectBrowser->type( FL_HOLD_BROWSER );
    m_FeaMaterialSelectBrowser->labelfont( labelfont );
    m_FeaMaterialSelectBrowser->labelsize( labelsize );
    m_FeaMaterialSelectBrowser->textsize( textsize );
    m_FeaMaterialSelectBrowser->callback( staticScreenCB, this );

    m_MaterialEditGroup.SetSameLineFlag( true );
    m_MaterialEditGroup.SetFitWidthFlag( false );

    m_MaterialEditGroup.SetButtonWidth( m_MaterialEditGroup.GetRemainX() / 2 );

    m_MaterialEditGroup.AddButton( m_AddFeaMaterialButton, "Add Material" );
    m_MaterialEditGroup.AddButton( m_DelFeaMaterialButton, "Delete Material" );
    m_MaterialEditGroup.ForceNewLine();

    m_MaterialEditGroup.AddSubGroupLayout( m_MaterialEditSubGroup, m_MaterialEditGroup.GetRemainX(), m_MaterialEditGroup.GetRemainY() );

    m_MaterialEditSubGroup.SetButtonWidth( m_MaterialEditSubGroup.GetRemainX() / 3 );

    m_MaterialEditSubGroup.AddYGap();

    m_MaterialEditSubGroup.AddInput( m_FeaMaterialNameInput, "Material Name" );

    m_MaterialEditSubGroup.AddYGap();

    m_MaterialEditSubGroup.AddDividerBox( "Material Properties" );

    m_MaterialEditSubGroup.SetSameLineFlag( true );
    m_MaterialEditSubGroup.SetFitWidthFlag( false );

    m_MaterialEditSubGroup.SetButtonWidth( m_MaterialEditSubGroup.GetRemainX() / 3 );
    m_MaterialEditSubGroup.SetSliderWidth( 6 * m_MaterialEditSubGroup.GetW() / 24 );
    m_MaterialEditSubGroup.SetInputWidth( m_MaterialEditSubGroup.GetW() / 6 );

    m_MaterialEditSubGroup.AddSlider( m_MatDensitySlider, "Mass Density", 1e3, "%5.3g" );
    m_MaterialEditSubGroup.SetButtonWidth( m_MaterialEditSubGroup.GetRemainX() );
    m_MaterialEditSubGroup.AddButton( m_MatDensityUnit, "" );
    m_MatDensityUnit.GetFlButton()->box( FL_THIN_UP_BOX );
    m_MatDensityUnit.GetFlButton()->labelcolor( FL_BLACK );

    m_MaterialEditSubGroup.ForceNewLine();
    m_MaterialEditSubGroup.SetButtonWidth( m_MaterialEditSubGroup.GetRemainX() / 3 );
    m_MaterialEditSubGroup.SetSliderWidth( 6 * m_MaterialEditSubGroup.GetW() / 24 );

    m_MaterialEditSubGroup.AddSlider( m_MatElasticModSlider, "Elastic Modulus", 1e4, "%5.3g" );
    m_MaterialEditSubGroup.SetButtonWidth( m_MaterialEditSubGroup.GetRemainX() );
    m_MaterialEditSubGroup.AddButton( m_MatElasticModUnit, "" );
    m_MatElasticModUnit.GetFlButton()->box( FL_THIN_UP_BOX );
    m_MatElasticModUnit.GetFlButton()->labelcolor( FL_BLACK );

    m_MaterialEditSubGroup.ForceNewLine();
    m_MaterialEditSubGroup.SetButtonWidth( m_MaterialEditSubGroup.GetRemainX() / 3 );
    m_MaterialEditSubGroup.SetSliderWidth( 6 * m_MaterialEditSubGroup.GetW() / 24 );

    m_MaterialEditSubGroup.AddSlider( m_MatPoissonSlider, "Poisson Ratio", 1, "%5.3g" );

    m_MaterialEditSubGroup.ForceNewLine();

    m_MaterialEditSubGroup.AddSlider( m_MatThermalExCoeffSlider, "Thermal Expan Coeff", 10e-5, "%5.3g" );
    m_MaterialEditSubGroup.SetButtonWidth( m_MaterialEditSubGroup.GetRemainX() );
    m_MaterialEditSubGroup.AddButton( m_MatThermalExCoeffUnit, "" );
    m_MatThermalExCoeffUnit.GetFlButton()->box( FL_THIN_UP_BOX );
    m_MatThermalExCoeffUnit.GetFlButton()->labelcolor( FL_BLACK );

    //=== Property Tab ===//
    m_PropertyTabLayout.SetGroupAndScreen( propTabGroup, this );

    m_PropertyTabLayout.AddDividerBox( "Property Selection" );

    m_PropertyEditGroup.SetGroupAndScreen( AddSubGroup( propTabGroup, 5 ), this );
    m_PropertyEditGroup.SetY( m_PropertyTabLayout.GetY() );

    m_FeaPropertySelectBrowser = m_PropertyEditGroup.AddFlBrowser( browser_h - 20 );
    m_FeaPropertySelectBrowser->type( FL_HOLD_BROWSER );
    m_FeaPropertySelectBrowser->labelfont( labelfont );
    m_FeaPropertySelectBrowser->labelsize( labelsize );
    m_FeaPropertySelectBrowser->textsize( textsize );
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

    m_PropertyEditGroup.AddSubGroupLayout( m_FeaPropertyCommonGroup, m_PropertyEditGroup.GetRemainX(), m_PropertyEditGroup.GetRemainY() );

    m_FeaPropertyCommonGroup.SetButtonWidth( m_FeaPropertyCommonGroup.GetRemainX() / 3 );

    m_FeaPropertyCommonGroup.AddYGap();

    m_FeaPropertyCommonGroup.AddInput( m_FeaPropertyNameInput, "Property Name" );

    m_FeaPropertyCommonGroup.AddYGap();

    m_FeaPropertyCommonGroup.AddSubGroupLayout( m_FeaPropertyShellGroup, m_FeaPropertyCommonGroup.GetRemainX(), m_FeaPropertyCommonGroup.GetRemainY() );

    m_FeaPropertyShellGroup.AddDividerBox( "Shell Properties" );

    m_FeaPropertyShellGroup.SetButtonWidth( m_FeaPropertyShellGroup.GetRemainX() / 3 );
    m_FeaPropertyShellGroup.SetChoiceButtonWidth( m_FeaPropertyShellGroup.GetRemainX() / 3 );

    m_FeaPropertyShellGroup.AddChoice( m_FeaShellMaterialChoice, "Material" );

    m_FeaPropertyShellGroup.SetSameLineFlag( true );
    m_FeaPropertyShellGroup.SetFitWidthFlag( false );

    m_FeaPropertyShellGroup.SetSliderWidth( m_FeaPropertyShellGroup.GetW() / 3 );
    m_FeaPropertyShellGroup.SetInputWidth( m_FeaPropertyShellGroup.GetW() / 6 );

    m_FeaPropertyShellGroup.AddSlider( m_PropThickSlider, "Thickness", 100.0, "%5.3g" );
    m_FeaPropertyShellGroup.SetButtonWidth( m_FeaPropertyShellGroup.GetRemainX() );
    m_FeaPropertyShellGroup.AddButton( m_PropThickUnit, "" );
    m_PropThickUnit.GetFlButton()->box( FL_THIN_UP_BOX );
    m_PropThickUnit.GetFlButton()->labelcolor( FL_BLACK );

    m_FeaPropertyShellGroup.ForceNewLine();

    m_FeaPropertyCommonGroup.AddSubGroupLayout( m_FeaPropertyBeamGroup, m_FeaPropertyCommonGroup.GetRemainX(), m_FeaPropertyCommonGroup.GetRemainY() );

    m_FeaPropertyBeamGroup.AddDividerBox( "Beam Properties" );

    m_FeaPropertyBeamGroup.SetButtonWidth( m_FeaPropertyBeamGroup.GetRemainX() / 3 );
    m_FeaPropertyBeamGroup.SetChoiceButtonWidth( m_FeaPropertyBeamGroup.GetRemainX() / 3 );

    m_FeaPropertyBeamGroup.AddChoice( m_FeaBeamMaterialChoice, "Material" );

    m_FeaPropertyBeamGroup.SetSameLineFlag( true );
    m_FeaPropertyBeamGroup.SetFitWidthFlag( false );

    m_FeaPropertyBeamGroup.SetSliderWidth( m_FeaPropertyBeamGroup.GetRemainX() / 3 );

    m_FeaBeamXSecChoice.AddItem( "General" );
    m_FeaBeamXSecChoice.AddItem( "Circle" );
    m_FeaBeamXSecChoice.AddItem( "Pipe" );
    m_FeaBeamXSecChoice.AddItem( "I" );
    m_FeaBeamXSecChoice.AddItem( "Rectangle" );
    m_FeaBeamXSecChoice.AddItem( "Box" );
    m_FeaPropertyBeamGroup.AddChoice( m_FeaBeamXSecChoice, "Cross Section Type" );

    m_FeaPropertyBeamGroup.AddButton( m_ShowFeaBeamXSecButton, "Show XSec" );

    m_FeaPropertyBeamGroup.ForceNewLine();

    m_FeaPropertyBeamGroup.AddYGap();

    // General XSec
    m_FeaPropertyBeamGroup.AddSubGroupLayout( m_GenXSecGroup, m_FeaPropertyBeamGroup.GetRemainX(), m_FeaPropertyBeamGroup.GetRemainY() );

    m_GenXSecGroup.AddDividerBox( "General XSec" );

    m_GenXSecGroup.SetSameLineFlag( true );
    m_GenXSecGroup.SetFitWidthFlag( false );

    m_GenXSecGroup.SetButtonWidth( m_GenXSecGroup.GetRemainX() / 3 );
    m_GenXSecGroup.SetSliderWidth( m_GenXSecGroup.GetRemainX() / 3 );
    m_GenXSecGroup.SetInputWidth( m_GenXSecGroup.GetRemainX() / 6 );

    m_GenXSecGroup.AddSlider( m_PropAreaSlider, "Cross-Sect Area", 100.0, "%5.3g" );
    m_GenXSecGroup.SetButtonWidth( m_GenXSecGroup.GetRemainX() );
    m_GenXSecGroup.AddButton( m_PropAreaUnit, "" );
    m_PropAreaUnit.GetFlButton()->box( FL_THIN_UP_BOX );
    m_PropAreaUnit.GetFlButton()->labelcolor( FL_BLACK );

    m_GenXSecGroup.ForceNewLine();
    m_GenXSecGroup.SetButtonWidth( m_GenXSecGroup.GetRemainX() / 3 );

    m_GenXSecGroup.AddSlider( m_PropIzzSlider, "Izz", 100.0, "%5.3g" );
    m_GenXSecGroup.SetButtonWidth( m_GenXSecGroup.GetRemainX() );
    m_GenXSecGroup.AddButton( m_PropIzzUnit, "" );
    m_PropIzzUnit.GetFlButton()->box( FL_THIN_UP_BOX );
    m_PropIzzUnit.GetFlButton()->labelcolor( FL_BLACK );

    m_GenXSecGroup.ForceNewLine();
    m_GenXSecGroup.SetButtonWidth( m_GenXSecGroup.GetRemainX() / 3 );

    m_GenXSecGroup.AddSlider( m_PropIyySlider, "Iyy", 100.0, "%5.3g" );
    m_GenXSecGroup.SetButtonWidth( m_GenXSecGroup.GetRemainX() );
    m_GenXSecGroup.AddButton( m_PropIyyUnit, "" );
    m_PropIyyUnit.GetFlButton()->box( FL_THIN_UP_BOX );
    m_PropIyyUnit.GetFlButton()->labelcolor( FL_BLACK );

    m_GenXSecGroup.ForceNewLine();
    m_GenXSecGroup.SetButtonWidth( m_GenXSecGroup.GetRemainX() / 3 );

    m_GenXSecGroup.AddSlider( m_PropIzySlider, "Izy", 100.0, "%5.3g" );
    m_GenXSecGroup.SetButtonWidth( m_GenXSecGroup.GetRemainX() );
    m_GenXSecGroup.AddButton( m_PropIzyUnit, "" );
    m_PropIzyUnit.GetFlButton()->box( FL_THIN_UP_BOX );
    m_PropIzyUnit.GetFlButton()->labelcolor( FL_BLACK );

    m_GenXSecGroup.ForceNewLine();
    m_GenXSecGroup.SetButtonWidth( m_GenXSecGroup.GetRemainX() / 3 );

    m_GenXSecGroup.AddSlider( m_PropIxxSlider, "Ixx", 100.0, "%5.3g" );
    m_GenXSecGroup.SetButtonWidth( m_GenXSecGroup.GetRemainX() );
    m_GenXSecGroup.AddButton( m_PropIxxUnit, "" );
    m_PropIxxUnit.GetFlButton()->box( FL_THIN_UP_BOX );
    m_PropIxxUnit.GetFlButton()->labelcolor( FL_BLACK );

    m_GenXSecGroup.ForceNewLine();

    // Circle XSec
    m_FeaPropertyBeamGroup.AddSubGroupLayout( m_CircXSecGroup, m_FeaPropertyBeamGroup.GetRemainX(), m_FeaPropertyBeamGroup.GetRemainY() );

    m_CircXSecGroup.AddDividerBox( "Circle XSec" );

    m_CircXSecGroup.SetSameLineFlag( true );
    m_CircXSecGroup.SetFitWidthFlag( false );

    m_CircXSecGroup.SetButtonWidth( m_CircXSecGroup.GetRemainX() / 3 );
    m_CircXSecGroup.SetSliderWidth( m_CircXSecGroup.GetRemainX() / 3 );
    m_CircXSecGroup.SetInputWidth( m_CircXSecGroup.GetRemainX() / 6 );

    m_CircXSecGroup.AddSlider( m_CircDim1Slider, "Radius", 100.0, "%5.3f" );
    m_CircXSecGroup.SetButtonWidth( m_CircXSecGroup.GetRemainX() );
    m_CircXSecGroup.AddButton( m_CircDim1Unit, "" );
    m_CircDim1Unit.GetFlButton()->box( FL_THIN_UP_BOX );
    m_CircDim1Unit.GetFlButton()->labelcolor( FL_BLACK );

    // Pipe/Tube XSec
    m_FeaPropertyBeamGroup.AddSubGroupLayout( m_PipeXSecGroup, m_FeaPropertyBeamGroup.GetRemainX(), m_FeaPropertyBeamGroup.GetRemainY() );

    m_PipeXSecGroup.AddDividerBox( "Pipe XSec" );

    m_PipeXSecGroup.SetSameLineFlag( true );
    m_PipeXSecGroup.SetFitWidthFlag( false );

    m_PipeXSecGroup.SetButtonWidth( m_PipeXSecGroup.GetRemainX() / 3 );
    m_PipeXSecGroup.SetSliderWidth( m_PipeXSecGroup.GetRemainX() / 3 );
    m_PipeXSecGroup.SetInputWidth( m_PipeXSecGroup.GetRemainX() / 6 );

    m_PipeXSecGroup.AddSlider( m_PipeDim1Slider, "R_outer", 100.0, "%5.3f" );
    m_PipeXSecGroup.SetButtonWidth( m_PipeXSecGroup.GetRemainX() );
    m_PipeXSecGroup.AddButton( m_PipeDim1Unit, "" );
    m_PipeDim1Unit.GetFlButton()->box( FL_THIN_UP_BOX );
    m_PipeDim1Unit.GetFlButton()->labelcolor( FL_BLACK );

    m_PipeXSecGroup.ForceNewLine();
    m_PipeXSecGroup.SetButtonWidth( m_PipeXSecGroup.GetW() / 3 );

    m_PipeXSecGroup.AddSlider( m_PipeDim2Slider, "R_inner", 100.0, "%5.3f" );
    m_PipeXSecGroup.SetButtonWidth( m_PipeXSecGroup.GetRemainX() );
    m_PipeXSecGroup.AddButton( m_PipeDim2Unit, "" );
    m_PipeDim2Unit.GetFlButton()->box( FL_THIN_UP_BOX );
    m_PipeDim2Unit.GetFlButton()->labelcolor( FL_BLACK );

    // I XSec
    m_FeaPropertyBeamGroup.AddSubGroupLayout( m_IXSecGroup, m_FeaPropertyBeamGroup.GetRemainX(), m_FeaPropertyBeamGroup.GetRemainY() );

    m_IXSecGroup.AddDividerBox( "I XSec" );

    m_IXSecGroup.SetSameLineFlag( true );
    m_IXSecGroup.SetFitWidthFlag( false );

    m_IXSecGroup.SetButtonWidth( m_IXSecGroup.GetRemainX() / 3 );
    m_IXSecGroup.SetSliderWidth( m_IXSecGroup.GetRemainX() / 3 );
    m_IXSecGroup.SetInputWidth( m_IXSecGroup.GetRemainX() / 6 );

    m_IXSecGroup.AddSlider( m_IDim1Slider, "Dim1", 100.0, "%5.3f" );
    m_IXSecGroup.SetButtonWidth( m_IXSecGroup.GetRemainX() );
    m_IXSecGroup.AddButton( m_IDim1Unit, "" );
    m_IDim1Unit.GetFlButton()->box( FL_THIN_UP_BOX );
    m_IDim1Unit.GetFlButton()->labelcolor( FL_BLACK );

    m_IXSecGroup.ForceNewLine();
    m_IXSecGroup.SetButtonWidth( m_IXSecGroup.GetW() / 3 );

    m_IXSecGroup.AddSlider( m_IDim2Slider, "Dim2", 100.0, "%5.3f" );
    m_IXSecGroup.SetButtonWidth( m_IXSecGroup.GetRemainX() );
    m_IXSecGroup.AddButton( m_IDim2Unit, "" );
    m_IDim2Unit.GetFlButton()->box( FL_THIN_UP_BOX );
    m_IDim2Unit.GetFlButton()->labelcolor( FL_BLACK );

    m_IXSecGroup.ForceNewLine();
    m_IXSecGroup.SetButtonWidth( m_IXSecGroup.GetRemainX() / 3 );

    m_IXSecGroup.AddSlider( m_IDim3Slider, "Dim3", 100.0, "%5.3f" );
    m_IXSecGroup.SetButtonWidth( m_IXSecGroup.GetRemainX() );
    m_IXSecGroup.AddButton( m_IDim3Unit, "" );
    m_IDim3Unit.GetFlButton()->box( FL_THIN_UP_BOX );
    m_IDim3Unit.GetFlButton()->labelcolor( FL_BLACK );

    m_IXSecGroup.ForceNewLine();
    m_IXSecGroup.SetButtonWidth( m_IXSecGroup.GetRemainX() / 3 );

    m_IXSecGroup.AddSlider( m_IDim4Slider, "Dim4", 100.0, "%5.3f" );
    m_IXSecGroup.SetButtonWidth( m_IXSecGroup.GetRemainX() );
    m_IXSecGroup.AddButton( m_IDim4Unit, "" );
    m_IDim4Unit.GetFlButton()->box( FL_THIN_UP_BOX );
    m_IDim4Unit.GetFlButton()->labelcolor( FL_BLACK );

    m_IXSecGroup.ForceNewLine();
    m_IXSecGroup.SetButtonWidth( m_IXSecGroup.GetRemainX() / 3 );

    m_IXSecGroup.AddSlider( m_IDim5Slider, "Dim5", 100.0, "%5.3f" );
    m_IXSecGroup.SetButtonWidth( m_IXSecGroup.GetRemainX() );
    m_IXSecGroup.AddButton( m_IDim5Unit, "" );
    m_IDim5Unit.GetFlButton()->box( FL_THIN_UP_BOX );
    m_IDim5Unit.GetFlButton()->labelcolor( FL_BLACK );

    m_IXSecGroup.ForceNewLine();
    m_IXSecGroup.SetButtonWidth( m_IXSecGroup.GetRemainX() / 3 );

    m_IXSecGroup.AddSlider( m_IDim6Slider, "Dim6", 100.0, "%5.3f" );
    m_IXSecGroup.SetButtonWidth( m_IXSecGroup.GetRemainX() );
    m_IXSecGroup.AddButton( m_IDim6Unit, "" );
    m_IDim6Unit.GetFlButton()->box( FL_THIN_UP_BOX );
    m_IDim6Unit.GetFlButton()->labelcolor( FL_BLACK );

    // Rectangle XSec
    m_FeaPropertyBeamGroup.AddSubGroupLayout( m_RectXSecGroup, m_FeaPropertyBeamGroup.GetRemainX(), m_FeaPropertyBeamGroup.GetRemainY() );

    m_RectXSecGroup.AddDividerBox( "Rectangle XSec" );

    m_RectXSecGroup.SetSameLineFlag( true );
    m_RectXSecGroup.SetFitWidthFlag( false );

    m_RectXSecGroup.SetButtonWidth( m_RectXSecGroup.GetRemainX() / 3 );
    m_RectXSecGroup.SetSliderWidth( m_RectXSecGroup.GetRemainX() / 3 );
    m_RectXSecGroup.SetInputWidth( m_RectXSecGroup.GetRemainX() / 6 );

    m_RectXSecGroup.AddSlider( m_RectDim1Slider, "Width", 100.0, "%5.3f" );
    m_RectXSecGroup.SetButtonWidth( m_RectXSecGroup.GetRemainX() );
    m_RectXSecGroup.AddButton( m_RectDim1Unit, "" );
    m_RectDim1Unit.GetFlButton()->box( FL_THIN_UP_BOX );
    m_RectDim1Unit.GetFlButton()->labelcolor( FL_BLACK );

    m_RectXSecGroup.ForceNewLine();
    m_RectXSecGroup.SetButtonWidth( m_RectXSecGroup.GetRemainX() / 3 );

    m_RectXSecGroup.AddSlider( m_RectDim2Slider, "Height", 100.0, "%5.3f" );
    m_RectXSecGroup.SetButtonWidth( m_RectXSecGroup.GetRemainX() );
    m_RectXSecGroup.AddButton( m_RectDim2Unit, "" );
    m_RectDim2Unit.GetFlButton()->box( FL_THIN_UP_BOX );
    m_RectDim2Unit.GetFlButton()->labelcolor( FL_BLACK );

    // Box XSec
    m_FeaPropertyBeamGroup.AddSubGroupLayout( m_BoxXSecGroup, m_FeaPropertyBeamGroup.GetRemainX(), m_FeaPropertyBeamGroup.GetRemainY() );

    m_BoxXSecGroup.AddDividerBox( "Box XSec" );

    m_BoxXSecGroup.SetSameLineFlag( true );
    m_BoxXSecGroup.SetFitWidthFlag( false );

    m_BoxXSecGroup.SetButtonWidth( m_BoxXSecGroup.GetRemainX() / 3 );
    m_BoxXSecGroup.SetSliderWidth( m_BoxXSecGroup.GetRemainX() / 3 );
    m_BoxXSecGroup.SetInputWidth( m_BoxXSecGroup.GetRemainX() / 6 );

    m_BoxXSecGroup.AddSlider( m_BoxDim1Slider, "Dim1", 100.0, "%5.3f" );
    m_BoxXSecGroup.SetButtonWidth( m_BoxXSecGroup.GetRemainX() );
    m_BoxXSecGroup.AddButton( m_BoxDim1Unit, "" );
    m_BoxDim1Unit.GetFlButton()->box( FL_THIN_UP_BOX );
    m_BoxDim1Unit.GetFlButton()->labelcolor( FL_BLACK );

    m_BoxXSecGroup.ForceNewLine();
    m_BoxXSecGroup.SetButtonWidth( m_BoxXSecGroup.GetRemainX() / 3 );

    m_BoxXSecGroup.AddSlider( m_BoxDim2Slider, "Dim2", 100.0, "%5.3f" );
    m_BoxXSecGroup.SetButtonWidth( m_BoxXSecGroup.GetRemainX() );
    m_BoxXSecGroup.AddButton( m_BoxDim2Unit, "" );
    m_BoxDim2Unit.GetFlButton()->box( FL_THIN_UP_BOX );
    m_BoxDim2Unit.GetFlButton()->labelcolor( FL_BLACK );

    m_BoxXSecGroup.ForceNewLine();
    m_BoxXSecGroup.SetButtonWidth( m_BoxXSecGroup.GetRemainX() / 3 );

    m_BoxXSecGroup.AddSlider( m_BoxDim3Slider, "Dim3", 100.0, "%5.3f" );
    m_BoxXSecGroup.SetButtonWidth( m_BoxXSecGroup.GetRemainX() );
    m_BoxXSecGroup.AddButton( m_BoxDim3Unit, "" );
    m_BoxDim3Unit.GetFlButton()->box( FL_THIN_UP_BOX );
    m_BoxDim3Unit.GetFlButton()->labelcolor( FL_BLACK );

    m_BoxXSecGroup.ForceNewLine();
    m_BoxXSecGroup.SetButtonWidth( m_BoxXSecGroup.GetRemainX() / 3 );

    m_BoxXSecGroup.AddSlider( m_BoxDim4Slider, "Dim4", 100.0, "%5.3f" );
    m_BoxXSecGroup.SetButtonWidth( m_BoxXSecGroup.GetRemainX() );
    m_BoxXSecGroup.AddButton( m_BoxDim4Unit, "" );
    m_BoxDim4Unit.GetFlButton()->box( FL_THIN_UP_BOX );
    m_BoxDim4Unit.GetFlButton()->labelcolor( FL_BLACK );

    //=== MESH TAB ===//
    m_MeshTabLayout.SetGroupAndScreen( meshTabGroup, this );

    m_MeshTabLayout.AddDividerBox( "Mesh Control" );

    m_MeshTabLayout.AddYGap();

    m_MeshTabLayout.SetButtonWidth( 175 );
    m_MeshTabLayout.AddSlider( m_MaxEdgeLen, "Max Edge Len", 1.0, "%7.5f" );
    m_MeshTabLayout.AddSlider( m_MinEdgeLen, "Min Edge Len", 1.0, "%7.5f" );
    m_MeshTabLayout.AddSlider( m_MaxGap, "Max Gap", 1.0, "%7.5f" );
    m_MeshTabLayout.AddSlider( m_NumCircleSegments, "Num Circle Segments", 100.0, "%7.5f" );
    m_MeshTabLayout.AddSlider( m_GrowthRatio, "Growth Ratio", 2.0, "%7.5f" );

    m_MeshTabLayout.AddYGap();
    m_MeshTabLayout.AddButton( m_Rig3dGrowthLimit, "Rigorous 3D Growth Limiting" );
    m_MeshTabLayout.AddYGap();
    m_MeshTabLayout.AddSlider( m_RelCurveTolSlider, "Curve Adaptation Tolerance", 1.0, "%7.5f" );
    m_MeshTabLayout.AddYGap();
    m_MeshTabLayout.AddButton( m_HalfMeshButton, "Generate Half Mesh" );
    m_MeshTabLayout.AddYGap();

    m_OutputTabLayout.SetGroupAndScreen( outputTabGroup, this );
    // TODO: Add more CFD Mesh Export file options?

    m_OutputTabLayout.AddDividerBox( "File Export" );
    m_OutputTabLayout.AddYGap();

    m_OutputTabLayout.SetFitWidthFlag( false );
    m_OutputTabLayout.SetSameLineFlag( true );

    m_OutputTabLayout.SetInputWidth( m_OutputTabLayout.GetW() - 75 - 55 );

    m_OutputTabLayout.SetButtonWidth( 75 );
    m_OutputTabLayout.AddButton( m_StlFile, ".stl" );
    m_OutputTabLayout.AddOutput( m_StlOutput );
    m_OutputTabLayout.SetButtonWidth( m_OutputTabLayout.GetRemainX() );
    m_OutputTabLayout.AddButton( m_SelectStlFile, "..." );
    m_OutputTabLayout.ForceNewLine();

    m_OutputTabLayout.SetButtonWidth( 75 );
    m_OutputTabLayout.AddButton( m_GmshFile, ".msh" );
    m_OutputTabLayout.AddOutput( m_GmshOutput );
    m_OutputTabLayout.SetButtonWidth( m_OutputTabLayout.GetRemainX() );
    m_OutputTabLayout.AddButton( m_SelectGmshFile, "..." );
    m_OutputTabLayout.ForceNewLine();

    m_OutputTabLayout.AddYGap();

    m_OutputTabLayout.SetButtonWidth( 75 );
    m_OutputTabLayout.AddButton( m_MassFile, "Mass" );
    m_OutputTabLayout.AddOutput( m_MassOutput );
    m_OutputTabLayout.SetButtonWidth( m_OutputTabLayout.GetRemainX() );
    m_OutputTabLayout.AddButton( m_SelectMassFile, "..." );
    m_OutputTabLayout.ForceNewLine();

    m_OutputTabLayout.AddYGap();

    m_OutputTabLayout.SetButtonWidth( 75 );
    m_OutputTabLayout.AddButton( m_NastFile, "Nastran" );
    m_OutputTabLayout.AddOutput( m_NastOutput );
    m_OutputTabLayout.SetButtonWidth( m_OutputTabLayout.GetRemainX() );
    m_OutputTabLayout.AddButton( m_SelectNastFile, "..." );
    m_OutputTabLayout.ForceNewLine();

    m_OutputTabLayout.SetButtonWidth( 75 );
    m_OutputTabLayout.AddButton( m_NkeyFile, "Nkey" );
    m_OutputTabLayout.AddOutput( m_NkeyOutput );
    m_OutputTabLayout.SetButtonWidth( m_OutputTabLayout.GetRemainX() );
    m_OutputTabLayout.AddButton( m_SelectNkeyFile, "..." );
    m_OutputTabLayout.ForceNewLine();

    m_OutputTabLayout.AddYGap();

    m_OutputTabLayout.SetButtonWidth( 75 );
    m_OutputTabLayout.AddButton( m_CalcFile, "Calculix" );
    m_OutputTabLayout.AddOutput( m_CalcOutput );
    m_OutputTabLayout.SetButtonWidth( m_OutputTabLayout.GetRemainX() );
    m_OutputTabLayout.AddButton( m_SelectCalcFile, "..." );
    m_OutputTabLayout.ForceNewLine();

    m_OutputTabLayout.AddYGap();

    m_OutputTabLayout.SetFitWidthFlag( true );
    m_OutputTabLayout.SetSameLineFlag( false );
    m_OutputTabLayout.InitWidthHeightVals();

    m_OutputTabLayout.AddDividerBox("Surfaces and Intersection Curves");

    m_OutputTabLayout.AddYGap();

    m_OutputTabLayout.AddButton( m_ExportRaw, "Export Raw Points" );

    m_OutputTabLayout.InitWidthHeightVals();
    m_OutputTabLayout.SetInputWidth( m_OutputTabLayout.GetW() - 75 - 55 );
    m_OutputTabLayout.SetFitWidthFlag( false );
    m_OutputTabLayout.SetSameLineFlag( true );

    m_OutputTabLayout.SetButtonWidth( 75 );
    m_OutputTabLayout.AddButton(m_CurvFile, ".curv");
    m_OutputTabLayout.AddOutput(m_CurvOutput);
    m_OutputTabLayout.SetButtonWidth( m_OutputTabLayout.GetRemainX() );
    m_OutputTabLayout.AddButton(m_SelectCurvFile, "...");

    m_OutputTabLayout.ForceNewLine();
    m_OutputTabLayout.SetButtonWidth( 75 );
    m_OutputTabLayout.AddButton(m_Plot3DFile, ".p3d");
    m_OutputTabLayout.AddOutput(m_Plot3DOutput);
    m_OutputTabLayout.SetButtonWidth( m_OutputTabLayout.GetRemainX() );
    m_OutputTabLayout.AddButton(m_SelectPlot3DFile, "...");

    m_OutputTabLayout.AddYGap();
    m_OutputTabLayout.ForceNewLine();
    m_OutputTabLayout.SetButtonWidth( 75 );
    m_OutputTabLayout.AddButton( m_SrfFile, ".srf" );
    m_OutputTabLayout.AddOutput( m_SrfOutput );
    m_OutputTabLayout.SetButtonWidth( m_OutputTabLayout.GetRemainX() );
    m_OutputTabLayout.AddButton( m_SelectSrfFile, "..." );
    m_OutputTabLayout.ForceNewLine();

    m_OutputTabLayout.SetSameLineFlag( false );
    m_OutputTabLayout.SetFitWidthFlag( true );
    m_OutputTabLayout.AddButton( m_XYZIntCurves, "Include X,Y,Z Intersection Curves" );

    m_OutputTabLayout.AddYGap();

    //=== Display Tab ===//
    m_DisplayTabLayout.SetGroupAndScreen( displayTabGroup, this );

    m_DisplayTabLayout.AddDividerBox( "Display" );

    m_DisplayTabLayout.AddYGap();
    m_DisplayTabLayout.AddButton( m_DrawMeshButton, "Draw Mesh" );
    m_DisplayTabLayout.AddButton( m_ColorElementsButton, "Color Elements" );
    m_DisplayTabLayout.AddButton( m_DrawNodesToggle, "Draw Nodes" );
    m_DisplayTabLayout.AddButton( m_DrawElementOrientVecToggle, "Draw Element Orientation Vectors" );

    m_DisplayTabLayout.AddYGap();

    m_DisplayTabLayout.AddDividerBox( "Intersection Curve Display" );

    m_DisplayTabLayout.SetFitWidthFlag( false );
    m_DisplayTabLayout.SetSameLineFlag( true );

    m_DisplayTabLayout.SetButtonWidth( m_DisplayTabLayout.GetW() / 2 );

    m_DisplayTabLayout.AddYGap();
    m_DisplayTabLayout.AddButton( m_DrawIsect, "Show Intersection Curves");
    m_DisplayTabLayout.AddButton( m_DrawBorder, "Show Border Curves");
    m_DisplayTabLayout.ForceNewLine();
    m_DisplayTabLayout.AddYGap();
    m_DisplayTabLayout.AddButton( m_ShowCurve, "Show Curves");
    m_DisplayTabLayout.AddButton( m_ShowPts, "Show Points");
    m_DisplayTabLayout.ForceNewLine();
    m_DisplayTabLayout.AddYGap();
    m_DisplayTabLayout.AddButton( m_ShowRaw, "Show Raw Curve");
    m_DisplayTabLayout.AddButton( m_ShowBinAdapt, "Show Binary Adapted");
    m_DisplayTabLayout.ForceNewLine();

    m_DisplayTabLayout.SetFitWidthFlag( true );
    m_DisplayTabLayout.SetSameLineFlag( false );
    m_DisplayTabLayout.AddYGap();

    m_DisplayTabLayout.AddDividerBox( "Display Element Sets" );

    m_DrawPartSelectBrowser = m_DisplayTabLayout.AddCheckBrowser( browser_h );
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
    m_SelectedFeaPartChoice = 0;
    m_CurrDispGroup = NULL;
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
        vector< Geom* > geom_vec = veh->FindGeomVec( veh->GetGeomVec() );

        // TODO:  This looks like it would be much easier to use a GeomPicker and the AddExcludeType() method.
        map <string, int> CompIDMap;
        int icomp = 0;
        for ( int i = 0; i < (int)geom_vec.size(); ++i )
        {
            string disp_geom_name = std::to_string( icomp + 1 ) + ". " + geom_vec[i]->GetName();
            int geom_type = geom_vec[i]->GetType().m_Type;

            if ( geom_type != BLANK_GEOM_TYPE &&
                 geom_type != PT_CLOUD_GEOM_TYPE &&
                 geom_type != HINGE_GEOM_TYPE &&
                 geom_type != MESH_GEOM_TYPE &&
                 geom_type != HUMAN_GEOM_TYPE )
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

    Vehicle* veh = m_ScreenMgr->GetVehiclePtr();
    if ( !veh )
    {
        return;
    }

    static int widths[] = { 170, 140, 130 }; // TODO: Set spacing as function of browser width
    m_StructureSelectBrowser->column_widths( widths );
    m_StructureSelectBrowser->column_char( ':' );

    char str[256];

    m_StructureSelectBrowser->add( "@b@.NAME:@b@.GEOM:@b@.SURFACE" );

    string struct_name, parent_geom_name;
    int struct_surf_ind;

    // Populate browser with added structures
    vector< FeaStructure* > structVec = StructureMgr.GetAllFeaStructs();
    if ( structVec.size() > 0 )
    {
        for ( int i = 0; i < (int)structVec.size(); i++ )
        {
            struct_name = structVec[i]->GetName();
            struct_surf_ind = structVec[i]->GetFeaStructMainSurfIndx();

            Geom* parent = veh->FindGeom( structVec[i]->GetParentGeomID() );
            if ( parent )
            {
                parent_geom_name = parent->GetName();
            }

            sprintf( str, "%s:%s:Surf_%d", struct_name.c_str(), parent_geom_name.c_str(), struct_surf_ind );
            m_StructureSelectBrowser->add( str );
        }

        if ( StructureMgr.ValidTotalFeaStructInd( StructureMgr.GetCurrStructIndex() ) )
        {
            m_StructureSelectBrowser->select( StructureMgr.GetCurrStructIndex() + 2 );
        }
    }

    m_StructureSelectBrowser->position( scroll_pos );
}

void StructScreen::UpdateFeaPartBrowser()
{
    //==== FeaPart Browser ====//
    int scroll_pos = m_FeaPartSelectBrowser->position();
    m_FeaPartSelectBrowser->clear();
    static int widths[] = { 85, 65, 45, 85, 35, 85 }; // TODO: Set spacing as function of browser width
    m_FeaPartSelectBrowser->column_widths( widths );
    m_FeaPartSelectBrowser->column_char( ':' );

    char str[256];

    m_FeaPartSelectBrowser->add( "@b@.NAME:@b@.TYPE:@b@.SHELL:@b@.PROPERTY:@b@.CAP:@b@.PROPERTY" );

    string fea_name, fea_type, shell, shell_prop, cap, cap_prop;

    if ( StructureMgr.ValidTotalFeaStructInd( StructureMgr.GetCurrStructIndex() ) )
    {
        vector< FeaStructure* > structVec = StructureMgr.GetAllFeaStructs();
        vector<FeaPart*> feaprt_vec = structVec[StructureMgr.GetCurrStructIndex()]->GetFeaPartVec();

        for ( int i = 0; i < (int)feaprt_vec.size(); i++ )
        {
            fea_name = feaprt_vec[i]->GetName();
            fea_type = FeaPart::GetTypeName( feaprt_vec[i]->GetType() );

            if ( feaprt_vec[i]->m_IncludedElements() == vsp::FEA_SHELL || feaprt_vec[i]->m_IncludedElements() == vsp::FEA_SHELL_AND_BEAM )
            {
                shell = "     X";

                FeaProperty* prop = StructureMgr.GetFeaProperty( feaprt_vec[i]->m_FeaPropertyIndex() );
                if ( prop )
                {
                    shell_prop = prop->GetName();
                }
            }
            else
            {
                shell = "";
                shell_prop = "N/A";
            }

            if ( feaprt_vec[i]->m_IncludedElements() == vsp::FEA_BEAM || feaprt_vec[i]->m_IncludedElements() == vsp::FEA_SHELL_AND_BEAM )
            {
                cap = "   X";

                FeaProperty* prop = StructureMgr.GetFeaProperty( feaprt_vec[i]->m_CapFeaPropertyIndex() );
                if ( prop )
                {
                    cap_prop = prop->GetName();
                }
            }
            else
            {
                cap = "";
                cap_prop = "N/A";
            }

            if ( feaprt_vec[i]->GetType() == vsp::FEA_FIX_POINT )
            {
                shell = "";
                shell_prop = "N/A";
                cap = "";
                cap_prop = "N/A";
            }
            else if ( feaprt_vec[i]->GetType() == vsp::FEA_SKIN )
            {
                FeaSkin* skin = dynamic_cast<FeaSkin*>( feaprt_vec[i] );
                assert( skin );

                if ( skin->m_RemoveSkinTrisFlag() )
                {
                    shell = "";
                    shell_prop = "N/A";
                }
            }

            sprintf( str, "%s:%s:%s:%s:%s:%s:", fea_name.c_str(), fea_type.c_str(), shell.c_str(), shell_prop.c_str(), cap.c_str(), cap_prop.c_str() );
            m_FeaPartSelectBrowser->add( str );
        }

        vector<SubSurface*> subsurf_vec = structVec[StructureMgr.GetCurrStructIndex()]->GetFeaSubSurfVec();

        for ( int i = 0; i < (int)subsurf_vec.size(); i++ )
        {
            fea_name = subsurf_vec[i]->GetName();
            fea_type = SubSurface::GetTypeName( subsurf_vec[i]->GetType() );

            if ( subsurf_vec[i]->m_IncludedElements() == vsp::FEA_SHELL || subsurf_vec[i]->m_IncludedElements() == vsp::FEA_SHELL_AND_BEAM )
            {
                shell = "     X";

                FeaProperty* prop = StructureMgr.GetFeaProperty( subsurf_vec[i]->m_FeaPropertyIndex() );
                if ( prop )
                {
                    shell_prop = prop->GetName();
                }
            }
            else
            {
                shell = "";
                shell_prop = "N/A";
            }

            if ( subsurf_vec[i]->m_IncludedElements() == vsp::FEA_BEAM || subsurf_vec[i]->m_IncludedElements() == vsp::FEA_SHELL_AND_BEAM )
            {
                cap = "   X";

                FeaProperty* prop = StructureMgr.GetFeaProperty( subsurf_vec[i]->m_CapFeaPropertyIndex() );
                if ( prop )
                {
                    cap_prop = prop->GetName();
                }
            }
            else
            {
                cap = "";
                cap_prop = "N/A";
            }

            sprintf( str, "%s:%s:%s:%s:%s:%s:", fea_name.c_str(), fea_type.c_str(), shell.c_str(), shell_prop.c_str(), cap.c_str(), cap_prop.c_str() );
            m_FeaPartSelectBrowser->add( str );
        }

        for ( size_t i = 0; i < m_SelectedPartIndexVec.size(); i++ )
        {
            if ( structVec[StructureMgr.GetCurrStructIndex()]->ValidFeaPartInd( m_SelectedPartIndexVec[i] ) ||
                 structVec[StructureMgr.GetCurrStructIndex()]->ValidFeaSubSurfInd( m_SelectedPartIndexVec[i] - structVec[StructureMgr.GetCurrStructIndex()]->NumFeaParts() ) )
            {
                m_FeaPartSelectBrowser->select( m_SelectedPartIndexVec[i] + 2 );
            }
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
        if ( StructureMgr.ValidTotalFeaStructInd( StructureMgr.GetCurrStructIndex() ) )
        {
            FeaStructure* curr_struct = StructureMgr.GetAllFeaStructs()[StructureMgr.GetCurrStructIndex()];

            if ( curr_struct )
            {
                Geom* currgeom = veh->FindGeom( curr_struct->GetParentGeomID() );

                if ( currgeom )
                {
                    m_FeaPartChoice.AddItem( FeaPart::GetTypeName( vsp::FEA_SLICE ) );
                    m_FeaPartChoice.AddItem( FeaPart::GetTypeName( vsp::FEA_RIB ) );
                    m_FeaPartChoice.AddItem( FeaPart::GetTypeName( vsp::FEA_SPAR ) );
                    m_FeaPartChoice.AddItem( FeaPart::GetTypeName( vsp::FEA_FIX_POINT ) );
                    m_FeaPartChoice.AddItem( FeaPart::GetTypeName( vsp::FEA_DOME ) );
                    m_FeaPartChoice.AddItem( FeaPart::GetTypeName( vsp::FEA_RIB_ARRAY ) );
                    m_FeaPartChoice.AddItem( FeaPart::GetTypeName( vsp::FEA_SLICE_ARRAY ) );

                    m_FeaPartChoice.AddItem( SubSurface::GetTypeName( vsp::SS_LINE ) );
                    m_FeaPartChoice.AddItem( SubSurface::GetTypeName( vsp::SS_RECTANGLE ) );
                    m_FeaPartChoice.AddItem( SubSurface::GetTypeName( vsp::SS_ELLIPSE ) );
                    m_FeaPartChoice.AddItem( SubSurface::GetTypeName( vsp::SS_CONTROL ) );
                    m_FeaPartChoice.AddItem( SubSurface::GetTypeName( vsp::SS_LINE_ARRAY ) );

                    if ( currgeom->GetType().m_Type == MS_WING_GEOM_TYPE )
                    {
                        m_FeaPartChoice.SetFlag( 1, 0 ); // FEA_RIB
                        m_FeaPartChoice.SetFlag( 2, 0 ); // FEA_SPAR
                        m_FeaPartChoice.SetFlag( 5, 0 ); // FEA_RIB_ARRAY
                        m_FeaPartChoice.SetFlag( 10, 0 ); // SS_CONTROL
                    }
                    else
                    {
                        m_FeaPartChoice.SetFlag( 1, FL_MENU_INACTIVE );
                        m_FeaPartChoice.SetFlag( 2, FL_MENU_INACTIVE );
                        m_FeaPartChoice.SetFlag( 5, FL_MENU_INACTIVE );
                        m_FeaPartChoice.SetFlag( 10, FL_MENU_INACTIVE );
                    }

                    if ( currgeom->GetType().m_Type == FUSELAGE_GEOM_TYPE || currgeom->GetType().m_Type == POD_GEOM_TYPE || currgeom->GetType().m_Type == STACK_GEOM_TYPE ) //TODO: Improve
                    {
                        m_FeaPartChoice.SetFlag( 4, 0 ); // FEA_DOME
                    }
                    else
                    {
                        m_FeaPartChoice.SetFlag( 4, FL_MENU_INACTIVE );
                    }

                    m_NumFeaPartChoices = 7;

                    m_FeaPartChoice.UpdateItems();

                    m_FeaPartChoice.SetVal( m_SelectedFeaPartChoice );
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

    static int widths[] = { 130, 70, 80, 130 }; // TODO: Set spacing as function of browser width
    m_FeaPropertySelectBrowser->column_widths( widths );
    m_FeaPropertySelectBrowser->column_char( ':' );

    char str[256];

    m_FeaPropertySelectBrowser->add( "@b@.NAME:@b@.TYPE:@b@.XSEC:@b@.MATERIAL:" );

    string prop_name, prop_type, prop_xsec, prop_mat;

    vector< FeaProperty* > property_vec = StructureMgr.GetFeaPropertyVec();

    for ( int i = 0; i < (int)property_vec.size(); i++ )
    {
        prop_name = property_vec[i]->GetName();
        prop_type = property_vec[i]->GetTypeName();

        if ( property_vec[i]->m_FeaPropertyType() == vsp::FEA_SHELL )
        {
            prop_xsec = "N/A";
        }
        else
        {
            prop_xsec = property_vec[i]->GetXSecName();
        }

        FeaMaterial* fea_mat = StructureMgr.GetFeaMaterial( property_vec[i]->m_FeaMaterialIndex() );
        if ( fea_mat )
        {
            prop_mat = fea_mat->GetName();
        }

        sprintf( str, "%s:%s:%s:%s:", prop_name.c_str(), prop_type.c_str(), prop_xsec.c_str(), prop_mat.c_str() );

        m_FeaPropertySelectBrowser->add( str );
    }

    if ( StructureMgr.ValidFeaPropertyInd( StructureMgr.GetCurrPropertyIndex() ) )
    {
        m_FeaPropertySelectBrowser->select( StructureMgr.GetCurrPropertyIndex() + 2 );
    }

    m_FeaPropertySelectBrowser->position( scroll_pos );
}

void StructScreen::UpdateFeaPropertyChoice()
{
    //==== Property Choice ====//
    m_GenPropertyChoice.ClearItems();
    m_GenCapPropertyChoice.ClearItems();

    Vehicle*  veh = m_ScreenMgr->GetVehiclePtr();

    if ( veh )
    {
        vector< FeaProperty* > property_vec = StructureMgr.GetFeaPropertyVec();

        for ( int i = 0; i < property_vec.size(); ++i )
        {
            m_GenPropertyChoice.AddItem( string( property_vec[i]->GetName() ) );
            m_GenCapPropertyChoice.AddItem( string( property_vec[i]->GetName() ) );

            if ( property_vec[i]->m_FeaPropertyType() == vsp::FEA_SHELL )
            {
                m_GenPropertyChoice.SetFlag( i, 0 );
                m_GenCapPropertyChoice.SetFlag( i, FL_MENU_INACTIVE );
            }
            else if ( property_vec[i]->m_FeaPropertyType() == vsp::FEA_BEAM )
            {
                m_GenPropertyChoice.SetFlag( i, FL_MENU_INACTIVE );
                m_GenCapPropertyChoice.SetFlag( i, 0 );
            }
        }

        m_GenPropertyChoice.UpdateItems();
        m_GenCapPropertyChoice.UpdateItems();

        if ( StructureMgr.ValidTotalFeaStructInd( StructureMgr.GetCurrStructIndex() ) )
        {
            vector < FeaStructure* > structvec = StructureMgr.GetAllFeaStructs();

            if ( m_SelectedPartIndexVec.size() == 1 )
            {
                if ( m_SelectedPartIndexVec[0] < structvec[StructureMgr.GetCurrStructIndex()]->NumFeaParts() )
                {
                    FeaPart* feaprt = structvec[StructureMgr.GetCurrStructIndex()]->GetFeaPart( m_SelectedPartIndexVec[0] );

                    if ( feaprt )
                    {
                        m_GenPropertyChoice.SetVal( feaprt->m_FeaPropertyIndex() );
                        m_GenCapPropertyChoice.SetVal( feaprt->m_CapFeaPropertyIndex() );
                    }
                }
                else if ( m_SelectedPartIndexVec[0] >= structvec[StructureMgr.GetCurrStructIndex()]->NumFeaParts() )
                {
                    SubSurface* subsurf = structvec[StructureMgr.GetCurrStructIndex()]->GetFeaSubSurf( m_SelectedPartIndexVec[0] - structvec[StructureMgr.GetCurrStructIndex()]->NumFeaParts() );

                    if ( subsurf )
                    {
                        m_GenPropertyChoice.SetVal( subsurf->m_FeaPropertyIndex() );
                        m_GenCapPropertyChoice.SetVal( subsurf->m_CapFeaPropertyIndex() );
                    }
                }
            }
            else if ( m_SelectedPartIndexVec.size() > 1 )
            {
                int prop_index = -1;
                int cap_index = -1;
                bool mult_prop = false;
                bool mult_cap_prop = false;

                for ( size_t i = 0; i < m_SelectedPartIndexVec.size(); i++ )
                {
                    if ( m_SelectedPartIndexVec[i] < structvec[StructureMgr.GetCurrStructIndex()]->NumFeaParts() )
                    {
                        FeaPart* feaprt = structvec[StructureMgr.GetCurrStructIndex()]->GetFeaPart( m_SelectedPartIndexVec[i] );

                        if ( feaprt )
                        {
                            if ( i == 0 )
                            {
                                prop_index = feaprt->m_FeaPropertyIndex();
                                cap_index = feaprt->m_CapFeaPropertyIndex();
                            }

                            if ( feaprt->m_FeaPropertyIndex() != prop_index )
                            {
                                mult_prop = true;
                            }
                            if ( feaprt->m_CapFeaPropertyIndex() != cap_index )
                            {
                                mult_cap_prop = true;
                            }
                        }
                    }
                    else if ( m_SelectedPartIndexVec[i] >= structvec[StructureMgr.GetCurrStructIndex()]->NumFeaParts() )
                    {
                        SubSurface* subsurf = structvec[StructureMgr.GetCurrStructIndex()]->GetFeaSubSurf( m_SelectedPartIndexVec[i] - structvec[StructureMgr.GetCurrStructIndex()]->NumFeaParts() );

                        if ( subsurf )
                        {
                            if ( i == 0 )
                            {
                                prop_index = subsurf->m_FeaPropertyIndex();
                                cap_index = subsurf->m_CapFeaPropertyIndex();
                            }

                            if ( subsurf->m_FeaPropertyIndex() != prop_index )
                            {
                                mult_prop = true;
                            }
                            if ( subsurf->m_CapFeaPropertyIndex() != cap_index )
                            {
                                mult_cap_prop = true;
                            }
                        }
                    }
                }

                if ( mult_prop )
                {
                    m_GenPropertyChoice.AddItem( "Multiple" );
                    m_GenPropertyChoice.UpdateItems();
                    m_GenPropertyChoice.SetVal( m_GenPropertyChoice.GetItems().size() - 1 );
                }
                else
                {
                    m_GenPropertyChoice.SetVal( prop_index );
                }

                if ( mult_cap_prop )
                {
                    m_GenCapPropertyChoice.AddItem( "Multiple" );
                    m_GenCapPropertyChoice.UpdateItems();
                    m_GenCapPropertyChoice.SetVal( m_GenCapPropertyChoice.GetItems().size() - 1 );
                }
                else
                {
                    m_GenCapPropertyChoice.SetVal( cap_index );
                }
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

    if ( StructureMgr.ValidFeaMaterialInd( StructureMgr.GetCurrMaterialIndex() ) )
    {
        m_MaterialEditSubGroup.Show();
        m_FeaMaterialSelectBrowser->select( StructureMgr.GetCurrMaterialIndex() + 1 );
    }
    else
    {
        m_MaterialEditSubGroup.Hide();
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

        if ( StructureMgr.ValidFeaPropertyInd( StructureMgr.GetCurrPropertyIndex() ) )
        {
            FeaProperty* fea_prop = StructureMgr.GetFeaPropertyVec()[StructureMgr.GetCurrPropertyIndex()];

            if ( fea_prop )
            {
                // Update all FeaPart Material Choices ( Only Selected Property Visible )
                m_FeaShellMaterialChoice.SetVal( fea_prop->m_FeaMaterialIndex() );
                m_FeaBeamMaterialChoice.SetVal( fea_prop->m_FeaMaterialIndex() );
            }
        }
    }
}

void StructScreen::BeamXSecDispGroup( GroupLayout* group )
{
    if ( m_CurBeamXSecDispGroup == group && group )
    {
        return;
    }

    m_GenXSecGroup.Hide();
    m_CircXSecGroup.Hide();
    m_PipeXSecGroup.Hide();
    m_IXSecGroup.Hide();
    m_RectXSecGroup.Hide();
    m_BoxXSecGroup.Hide();

    m_CurBeamXSecDispGroup = group;

    if ( group )
    {
        group->Show();
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

void StructScreen::UpdateGenPropertyIndex( Choice* property_choice )
{
    FeaProperty* prop = StructureMgr.GetFeaProperty( property_choice->GetVal() );

    if ( prop->m_FeaPropertyType() != vsp::FEA_SHELL || !StructureMgr.ValidFeaPropertyInd( property_choice->GetVal() ) )
    {
        return;
    }

    if ( StructureMgr.ValidTotalFeaStructInd( StructureMgr.GetCurrStructIndex() ) )
    {
        vector < FeaStructure* > structvec = StructureMgr.GetAllFeaStructs();

        for ( size_t i = 0; i < m_SelectedPartIndexVec.size(); i++ )
        {
            if ( m_SelectedPartIndexVec[i] < structvec[StructureMgr.GetCurrStructIndex()]->NumFeaParts() )
            {
                FeaPart* feaprt = structvec[StructureMgr.GetCurrStructIndex()]->GetFeaPart( m_SelectedPartIndexVec[i] );

                if ( feaprt )
                {
                    if ( ( feaprt->m_IncludedElements() == vsp::FEA_SHELL || feaprt->m_IncludedElements() == vsp::FEA_SHELL_AND_BEAM ) && feaprt->GetType() != vsp::FEA_FIX_POINT )
                    {
                        feaprt->m_FeaPropertyIndex.Set( property_choice->GetVal() );
                    }
                }
            }
            else if ( m_SelectedPartIndexVec[i] >= structvec[StructureMgr.GetCurrStructIndex()]->NumFeaParts() )
            {
                SubSurface* subsurf = structvec[StructureMgr.GetCurrStructIndex()]->GetFeaSubSurf( m_SelectedPartIndexVec[i] - structvec[StructureMgr.GetCurrStructIndex()]->NumFeaParts() );

                if ( subsurf )
                {
                    if ( subsurf->m_IncludedElements() == vsp::FEA_SHELL || subsurf->m_IncludedElements() == vsp::FEA_SHELL_AND_BEAM )
                    {
                        subsurf->m_FeaPropertyIndex.Set( property_choice->GetVal() );
                    }
                }
            }
        }
    }
}

void StructScreen::UpdateGenCapPropertyIndex( Choice* property_choice )
{
    FeaProperty* prop = StructureMgr.GetFeaProperty( property_choice->GetVal() );

    if ( prop->m_FeaPropertyType() != vsp::FEA_BEAM || !StructureMgr.ValidFeaPropertyInd( property_choice->GetVal() ) )
    {
        return;
    }

    if ( StructureMgr.ValidTotalFeaStructInd( StructureMgr.GetCurrStructIndex() ) )
    {
        vector < FeaStructure* > structvec = StructureMgr.GetAllFeaStructs();

        for ( size_t i = 0; i < m_SelectedPartIndexVec.size(); i++ )
        {
            if ( m_SelectedPartIndexVec[i] < structvec[StructureMgr.GetCurrStructIndex()]->NumFeaParts() )
            {
                FeaPart* feaprt = structvec[StructureMgr.GetCurrStructIndex()]->GetFeaPart( m_SelectedPartIndexVec[i] );

                if ( feaprt )
                {
                    if ( ( feaprt->m_IncludedElements() == vsp::FEA_BEAM || feaprt->m_IncludedElements() == vsp::FEA_SHELL_AND_BEAM ) && feaprt->GetType() != vsp::FEA_FIX_POINT )
                    {
                        feaprt->m_CapFeaPropertyIndex.Set( property_choice->GetVal() );
                    }
                }
            }
            else if ( m_SelectedPartIndexVec[i] >= structvec[StructureMgr.GetCurrStructIndex()]->NumFeaParts() )
            {
                SubSurface* subsurf = structvec[StructureMgr.GetCurrStructIndex()]->GetFeaSubSurf( m_SelectedPartIndexVec[i] - structvec[StructureMgr.GetCurrStructIndex()]->NumFeaParts() );

                if ( subsurf )
                {
                    if ( subsurf->m_IncludedElements() == vsp::FEA_BEAM || subsurf->m_IncludedElements() == vsp::FEA_SHELL_AND_BEAM )
                    {
                        subsurf->m_CapFeaPropertyIndex.Set( property_choice->GetVal() );
                    }
                }
            }
        }
    }
}

void StructScreen::FeaStructDispGroup( GroupLayout* group )
{
    if ( m_CurrDispGroup == group && group )
    {
        return;
    }

    m_StructGroup.Hide();
    m_PartGroup.Hide();
    m_StructGeneralGroup.Hide();
    m_StructWingGroup.Hide();

    if ( StructureMgr.ValidTotalFeaStructInd( StructureMgr.GetCurrStructIndex() ) )
    {
        m_StructGroup.Show();

        FeaStructure* curr_struct = StructureMgr.GetAllFeaStructs()[StructureMgr.GetCurrStructIndex()];

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

    m_CurrDispGroup = group;

    if ( group )
    {
        group->Show();
    }
}

void StructScreen::UpdateUnitLabels()
{
    Vehicle* veh = m_ScreenMgr->GetVehiclePtr();

    if ( veh )
    {
        string squared( 1, 178 );
        string cubed( 1, 179 );
        string deg( 1, 176 );
        string thick_unit, area_unit, area_moment_inertia_unit, young_mod_unit, density_unit, temp_unit;

        switch ( veh->m_StructUnit() )
        {
        case vsp::SI_UNIT:
        density_unit = "kg/m" + cubed;
        thick_unit = "m";
        area_unit = "m" + squared;
        area_moment_inertia_unit = "m^4";
        young_mod_unit = "Pa";
        temp_unit = "1/K";
        break;

        case vsp::CGS_UNIT:
        density_unit = "g/cm" + cubed;
        thick_unit = "cm";
        area_unit = "cm" + squared;
        area_moment_inertia_unit = "cm^4";
        young_mod_unit = "Ba";
        temp_unit = "1/K";
        break;

        case vsp::MPA_UNIT:
        density_unit = "t/mm" + cubed; // or Mg/
        thick_unit = "mm";
        area_unit = "mm" + squared;
        area_moment_inertia_unit = "mm^4";
        young_mod_unit = "MPa";
        temp_unit = "1/K";
        break;

        case vsp::BFT_UNIT:
        density_unit = "slug/ft" + cubed;
        thick_unit = "ft";
        area_unit = "ft" + squared;
        area_moment_inertia_unit = "ft^4";
        young_mod_unit = "lbf/ft" + squared;
        temp_unit = "1/" + deg + "R";
        break;

        case vsp::BIN_UNIT:
        density_unit = "lbf*sec" + squared + "/in^4";
        thick_unit = "in";
        area_unit = "in" + squared;
        area_moment_inertia_unit = "in^4";
        young_mod_unit = "psi";
        temp_unit = "1/" + deg + "R";
        break;
        }

        m_MatDensityUnit.GetFlButton()->copy_label( density_unit.c_str() );
        m_MatElasticModUnit.GetFlButton()->copy_label( young_mod_unit.c_str() );
        m_MatThermalExCoeffUnit.GetFlButton()->copy_label( temp_unit.c_str() );

        m_PropThickUnit.GetFlButton()->copy_label( thick_unit.c_str() );

        m_PropAreaUnit.GetFlButton()->copy_label( area_unit.c_str() );
        m_PropIzzUnit.GetFlButton()->copy_label( area_moment_inertia_unit.c_str() );
        m_PropIyyUnit.GetFlButton()->copy_label( area_moment_inertia_unit.c_str() );
        m_PropIzyUnit.GetFlButton()->copy_label( area_moment_inertia_unit.c_str() );
        m_PropIxxUnit.GetFlButton()->copy_label( area_moment_inertia_unit.c_str() );

        m_CircDim1Unit.GetFlButton()->copy_label( thick_unit.c_str() );

        m_PipeDim1Unit.GetFlButton()->copy_label( thick_unit.c_str() );
        m_PipeDim2Unit.GetFlButton()->copy_label( thick_unit.c_str() );

        m_IDim1Unit.GetFlButton()->copy_label( thick_unit.c_str() );
        m_IDim2Unit.GetFlButton()->copy_label( thick_unit.c_str() );
        m_IDim3Unit.GetFlButton()->copy_label( thick_unit.c_str() );
        m_IDim4Unit.GetFlButton()->copy_label( thick_unit.c_str() );
        m_IDim5Unit.GetFlButton()->copy_label( thick_unit.c_str() );
        m_IDim6Unit.GetFlButton()->copy_label( thick_unit.c_str() );

        m_RectDim1Unit.GetFlButton()->copy_label( thick_unit.c_str() );
        m_RectDim2Unit.GetFlButton()->copy_label( thick_unit.c_str() );

        m_BoxDim1Unit.GetFlButton()->copy_label( thick_unit.c_str() );
        m_BoxDim2Unit.GetFlButton()->copy_label( thick_unit.c_str() );
        m_BoxDim3Unit.GetFlButton()->copy_label( thick_unit.c_str() );
        m_BoxDim4Unit.GetFlButton()->copy_label( thick_unit.c_str() );
    }
}

bool StructScreen::Update()
{
    Vehicle* veh = m_ScreenMgr->GetVehiclePtr();

    if ( veh )
    {
        FeaStructDispGroup( NULL ); // Hide all initially

        m_StructUnitChoice.Update( veh->m_StructUnit.GetID() );

        //==== Update Unit Labels ====//
        UpdateUnitLabels();

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

        //===== FeaProperty Update =====//
        UpdateFeaPropertyBrowser();
        UpdateFeaPropertyChoice();

        if ( StructureMgr.ValidFeaPropertyInd( StructureMgr.GetCurrPropertyIndex() ) )
        {
            FeaProperty* fea_prop = StructureMgr.GetFeaPropertyVec()[StructureMgr.GetCurrPropertyIndex()];
            if ( fea_prop )
            {
                m_FeaPropertyNameInput.Update( fea_prop->GetName() );

                if ( fea_prop->m_FeaPropertyType() == vsp::FEA_SHELL )
                {
                    m_PropThickSlider.Update( fea_prop->m_Thickness.GetID() );

                    FeaPropertyDispGroup( &m_FeaPropertyShellGroup );
                }
                else if ( fea_prop->m_FeaPropertyType() == vsp::FEA_BEAM )
                {
                    m_FeaBeamXSecChoice.Update( fea_prop->m_CrossSectType.GetID() );

                    FeaPropertyDispGroup( &m_FeaPropertyBeamGroup );

                    m_ShowFeaBeamXSecButton.Activate();

                    if ( fea_prop->m_CrossSectType() == vsp::FEA_XSEC_GENERAL )
                    {
                        m_ShowFeaBeamXSecButton.Deactivate();

                        m_PropAreaSlider.Update( fea_prop->m_CrossSecArea.GetID() );
                        m_PropIzzSlider.Update( fea_prop->m_Izz.GetID() );
                        m_PropIyySlider.Update( fea_prop->m_Iyy.GetID() );
                        m_PropIzySlider.Update( fea_prop->m_Izy.GetID() );
                        m_PropIxxSlider.Update( fea_prop->m_Ixx.GetID() );

                        BeamXSecDispGroup( &m_GenXSecGroup );
                    }
                    else if ( fea_prop->m_CrossSectType() == vsp::FEA_XSEC_CIRC )
                    {
                        m_CircDim1Slider.Update( fea_prop->m_Dim1.GetID() );

                        BeamXSecDispGroup( &m_CircXSecGroup );
                    }
                    else if ( fea_prop->m_CrossSectType() == vsp::FEA_XSEC_PIPE )
                    {
                        m_PipeDim1Slider.Update( fea_prop->m_Dim1.GetID() );
                        m_PipeDim2Slider.Update( fea_prop->m_Dim2.GetID() );

                        BeamXSecDispGroup( &m_PipeXSecGroup );
                    }
                    else if ( fea_prop->m_CrossSectType() == vsp::FEA_XSEC_I )
                    {
                        m_IDim1Slider.Update( fea_prop->m_Dim1.GetID() );
                        m_IDim2Slider.Update( fea_prop->m_Dim2.GetID() );
                        m_IDim3Slider.Update( fea_prop->m_Dim3.GetID() );
                        m_IDim4Slider.Update( fea_prop->m_Dim4.GetID() );
                        m_IDim5Slider.Update( fea_prop->m_Dim5.GetID() );
                        m_IDim6Slider.Update( fea_prop->m_Dim6.GetID() );

                        BeamXSecDispGroup( &m_IXSecGroup );
                    }
                    else if ( fea_prop->m_CrossSectType() == vsp::FEA_XSEC_RECT )
                    {
                        m_RectDim1Slider.Update( fea_prop->m_Dim1.GetID() );
                        m_RectDim2Slider.Update( fea_prop->m_Dim2.GetID() );

                        BeamXSecDispGroup( &m_RectXSecGroup );
                    }
                    else if ( fea_prop->m_CrossSectType() == vsp::FEA_XSEC_BOX )
                    {
                        m_BoxDim1Slider.Update( fea_prop->m_Dim1.GetID() );
                        m_BoxDim2Slider.Update( fea_prop->m_Dim2.GetID() );
                        m_BoxDim3Slider.Update( fea_prop->m_Dim3.GetID() );
                        m_BoxDim4Slider.Update( fea_prop->m_Dim4.GetID() );

                        BeamXSecDispGroup( &m_BoxXSecGroup );
                    }
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

        if ( StructureMgr.ValidFeaMaterialInd( StructureMgr.GetCurrMaterialIndex() ) )
        {
            FeaMaterial* fea_mat = StructureMgr.GetFeaMaterialVec()[StructureMgr.GetCurrMaterialIndex()];
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

        if ( StructureMgr.ValidTotalFeaStructInd( StructureMgr.GetCurrStructIndex() ) )
        {
            vector< FeaStructure* > structVec = StructureMgr.GetAllFeaStructs();
            FeaStructure* curr_struct = structVec[StructureMgr.GetCurrStructIndex()];

            m_CurrStructOutput.Update( curr_struct->GetName() );

            //==== Default Elem Size ====//
            m_MaxEdgeLen.Update( curr_struct->GetFeaGridDensityPtr()->m_BaseLen.GetID() );
            m_MinEdgeLen.Update( curr_struct->GetFeaGridDensityPtr()->m_MinLen.GetID() );
            m_MaxGap.Update( curr_struct->GetFeaGridDensityPtr()->m_MaxGap.GetID() );
            m_NumCircleSegments.Update( curr_struct->GetFeaGridDensityPtr()->m_NCircSeg.GetID() );
            m_GrowthRatio.Update( curr_struct->GetFeaGridDensityPtr()->m_GrowRatio.GetID() );
            m_Rig3dGrowthLimit.Update( curr_struct->GetFeaGridDensityPtr()->m_RigorLimit.GetID() );

            m_RelCurveTolSlider.Update( curr_struct->GetStructSettingsPtr()->m_RelCurveTol.GetID() );

            if ( curr_struct->GetStructSettingsPtr()->m_DrawBinAdaptFlag() )
            {
                m_RelCurveTolSlider.Activate();
            }
            else
            {
                m_RelCurveTolSlider.Deactivate();
            }

            //===== Geometry Control =====//
            m_HalfMeshButton.Update( curr_struct->GetStructSettingsPtr()->m_HalfMeshFlag.GetID() );

            //===== Display Tab Toggle Update =====//
            m_DrawMeshButton.Update( curr_struct->GetStructSettingsPtr()->m_DrawMeshFlag.GetID() );
            m_ColorElementsButton.Update( curr_struct->GetStructSettingsPtr()->m_ColorTagsFlag.GetID() );
            m_DrawNodesToggle.Update( curr_struct->GetStructSettingsPtr()->m_DrawNodesFlag.GetID() );
            m_DrawElementOrientVecToggle.Update( curr_struct->GetStructSettingsPtr()->m_DrawElementOrientVecFlag.GetID() );

            m_DrawIsect.Update( curr_struct->GetStructSettingsPtr()->m_DrawIsectFlag.GetID() );
            m_DrawBorder.Update( curr_struct->GetStructSettingsPtr()->m_DrawBorderFlag.GetID() );

            m_ShowRaw.Update( curr_struct->GetStructSettingsPtr()->m_DrawRawFlag.GetID() );
            m_ShowBinAdapt.Update( curr_struct->GetStructSettingsPtr()->m_DrawBinAdaptFlag.GetID() );

            m_ShowCurve.Update( curr_struct->GetStructSettingsPtr()->m_DrawCurveFlag.GetID() );
            m_ShowPts.Update( curr_struct->GetStructSettingsPtr()->m_DrawPntsFlag.GetID() );

            if ( FeaMeshMgr.GetStructSettingsPtr() )
            {
                FeaMeshMgr.UpdateDisplaySettings();
            }

            string massname = curr_struct->GetStructSettingsPtr()->GetExportFileName( vsp::FEA_MASS_FILE_NAME );
            m_MassOutput.Update( truncateFileName( massname, 40 ).c_str() );
            string nastranname = curr_struct->GetStructSettingsPtr()->GetExportFileName( vsp::FEA_NASTRAN_FILE_NAME );
            m_NastOutput.Update( truncateFileName( nastranname, 40 ).c_str() );
            string nkeyname = curr_struct->GetStructSettingsPtr()->GetExportFileName( vsp::FEA_NKEY_FILE_NAME );
            m_NkeyOutput.Update( truncateFileName( nkeyname, 40 ).c_str() );
            string calculixname = curr_struct->GetStructSettingsPtr()->GetExportFileName( vsp::FEA_CALCULIX_FILE_NAME );
            m_CalcOutput.Update( truncateFileName( calculixname, 40 ).c_str() );
            string stlname = curr_struct->GetStructSettingsPtr()->GetExportFileName( vsp::FEA_STL_FILE_NAME );
            m_StlOutput.Update( truncateFileName( stlname, 40 ).c_str() );
            string gmshname = curr_struct->GetStructSettingsPtr()->GetExportFileName( vsp::FEA_GMSH_FILE_NAME );
            m_GmshOutput.Update( truncateFileName( gmshname, 40 ).c_str() );

            //==== Update File Output Flags ====//
            m_MassFile.Update( curr_struct->GetStructSettingsPtr()->GetExportFileFlag( vsp::FEA_MASS_FILE_NAME )->GetID() );
            m_NastFile.Update( curr_struct->GetStructSettingsPtr()->GetExportFileFlag( vsp::FEA_NASTRAN_FILE_NAME )->GetID() );
            m_NkeyFile.Update( curr_struct->GetStructSettingsPtr()->GetExportFileFlag( vsp::FEA_NKEY_FILE_NAME )->GetID() );
            m_CalcFile.Update( curr_struct->GetStructSettingsPtr()->GetExportFileFlag( vsp::FEA_CALCULIX_FILE_NAME )->GetID() );
            m_StlFile.Update( curr_struct->GetStructSettingsPtr()->GetExportFileFlag( vsp::FEA_STL_FILE_NAME )->GetID() );
            m_GmshFile.Update( curr_struct->GetStructSettingsPtr()->GetExportFileFlag( vsp::FEA_GMSH_FILE_NAME )->GetID() );

            if ( !curr_struct->GetStructSettingsPtr()->GetExportFileFlag( vsp::FEA_NASTRAN_FILE_NAME )->Get() )
            {
                m_NkeyFile.Deactivate();
                m_NkeyOutput.Deactivate();
                m_SelectNkeyFile.Deactivate();
            }
            else
            {
                m_NkeyFile.Activate();
                m_NkeyOutput.Activate();
                m_SelectNkeyFile.Activate();
            }

            string srfname = curr_struct->GetStructSettingsPtr()->GetExportFileName( vsp::FEA_SRF_FILE_NAME );
            m_SrfOutput.Update( truncateFileName( srfname, 40 ).c_str() );

            m_SrfFile.Update( curr_struct->GetStructSettingsPtr()->GetExportFileFlag( vsp::FEA_SRF_FILE_NAME )->GetID() );
            m_XYZIntCurves.Update( curr_struct->GetStructSettingsPtr()->m_XYZIntCurveFlag.GetID() );

            string curvname = curr_struct->GetStructSettingsPtr()->GetExportFileName( vsp::FEA_CURV_FILE_NAME );
            m_CurvOutput.Update( truncateFileName( curvname, 40 ).c_str() );
            string plot3dname = curr_struct->GetStructSettingsPtr()->GetExportFileName( vsp::FEA_PLOT3D_FILE_NAME );
            m_Plot3DOutput.Update( truncateFileName( plot3dname, 40 ).c_str() );

            //==== Update File Output Flags ====//
            m_CurvFile.Update( curr_struct->GetStructSettingsPtr()->GetExportFileFlag( vsp::FEA_CURV_FILE_NAME )->GetID() );
            m_Plot3DFile.Update( curr_struct->GetStructSettingsPtr()->GetExportFileFlag( vsp::FEA_PLOT3D_FILE_NAME )->GetID() );

            m_ExportRaw.Update( curr_struct->GetStructSettingsPtr()->m_ExportRawFlag.GetID() );

            // Update Structure Name
            m_FeaStructNameInput.Update( curr_struct->GetName() );

            if ( m_SelectedPartIndexVec.size() > 0 )
            {
                FeaStructDispGroup( &m_PartGroup );
                m_GenPropertyChoice.Activate();
                m_GenCapPropertyChoice.Activate();
                m_FeaPartNameInput.Deactivate();
                m_EditFeaPartButton.Deactivate();
                m_DispFeaPartGroup.Activate();
                m_ShellCapToggleGroup.Activate();
            }
            else
            {
                FeaStructDispGroup( NULL );
            }

            if ( m_SelectedPartIndexVec.size() == 1 )
            {
                m_FeaPartNameInput.Activate();
                m_EditFeaPartButton.Activate();

                if ( m_SelectedPartIndexVec[0] < curr_struct->NumFeaParts() )
                {
                    FeaPart* prt = curr_struct->GetFeaPart( m_SelectedPartIndexVec[0] );

                    if ( prt )
                    {
                        m_FeaPartNameInput.Update( prt->GetName() );
                        m_ShellCapToggleGroup.Update( prt->m_IncludedElements.GetID() );
                        m_DispFeaPartGroup.Update( prt->m_DrawFeaPartFlag.GetID() );
                        m_DispFeaPartGroup.Activate();
                        m_ShellCapToggleGroup.Activate();

                        if ( prt->m_IncludedElements() == vsp::FEA_SHELL )
                        {
                            m_GenPropertyChoice.Activate();
                            m_GenCapPropertyChoice.Deactivate();
                        }
                        else if ( prt->m_IncludedElements() == vsp::FEA_BEAM )
                        {
                            m_GenPropertyChoice.Deactivate();
                            m_GenCapPropertyChoice.Activate();
                        }

                        if ( prt->GetType() == vsp::FEA_FIX_POINT )
                        {
                            m_GenPropertyChoice.Deactivate();
                            m_GenCapPropertyChoice.Deactivate();
                            m_ShellCapToggleGroup.Deactivate();
                        }
                        else if ( prt->GetType() == vsp::FEA_SKIN )
                        {
                            m_DispFeaPartGroup.Deactivate();
                            m_ShellCapToggleGroup.Deactivate();

                            FeaSkin* skin = dynamic_cast<FeaSkin*>( prt );
                            assert( skin );

                            if ( skin->m_RemoveSkinTrisFlag() )
                            {
                                m_GenPropertyChoice.Deactivate();
                            }
                            else
                            {
                                m_GenPropertyChoice.Activate();
                            }
                        }
                    }
                    else
                    {
                        FeaStructDispGroup( NULL );
                    }
                }
                else if ( m_SelectedPartIndexVec[0] >= curr_struct->NumFeaParts() )
                {
                    SubSurface* subsurf = curr_struct->GetFeaSubSurf( m_SelectedPartIndexVec[0] - curr_struct->NumFeaParts() );

                    if ( subsurf )
                    {
                        m_FeaPartNameInput.Update( subsurf->GetName() );
                        m_ShellCapToggleGroup.Update( subsurf->m_IncludedElements.GetID() );
                        m_DispFeaPartGroup.Update( subsurf->m_DrawFeaPartFlag.GetID() );

                        if ( subsurf->m_IncludedElements() == vsp::FEA_SHELL )
                        {
                            m_GenPropertyChoice.Activate();
                            m_GenCapPropertyChoice.Deactivate();
                        }
                        else if ( subsurf->m_IncludedElements() == vsp::FEA_BEAM )
                        {
                            m_GenPropertyChoice.Deactivate();
                            m_GenCapPropertyChoice.Activate();
                        }

                        if ( subsurf->GetType() == vsp::SS_LINE_ARRAY )
                        {
                            m_ShellCapToggleGroup.Deactivate();
                        }
                        else
                        {
                            m_ShellCapToggleGroup.Activate();
                        }
                    }
                    else
                    {
                        FeaStructDispGroup( NULL );
                    }
                }
                else
                {
                    FeaStructDispGroup( NULL );
                }
            }

            // Update FeaParts and SubSurfaces if FeaMesh is not in progress
            if ( !FeaMeshMgr.GetFeaMeshInProgress() )
            {
                // Only update if FeaPartEditScreen is not open, since the structure is updated there otherwise
                VspScreen* edit_screen = m_ScreenMgr->GetScreen( ScreenMgr::VSP_FEA_PART_EDIT_SCREEN );
                if ( edit_screen )
                {
                    if ( !edit_screen->IsShown() )
                    {
                        structVec[StructureMgr.GetCurrStructIndex()]->Update();
                    }
                }
            }
        }
        else
        {
            m_CurrStructOutput.Update( "" );
        }

        // Update Draw Objects for FeaParts
        UpdateDrawObjs();

        if ( FeaMeshMgr.GetFeaMeshInProgress() )
        {
            m_FeaMeshExportButton.Deactivate();
        }
        else
        {
            m_FeaMeshExportButton.Activate();
        }

        if ( StructureMgr.ValidTotalFeaStructInd( StructureMgr.GetCurrStructIndex() ) && FeaMeshMgr.FeaDataAvailable() )
        {
            m_ResetDisplayButton.Activate();
        }
        else
        {
            m_ResetDisplayButton.Deactivate();
        }
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
            if ( StructureMgr.ValidTotalFeaStructInd( StructureMgr.GetCurrStructIndex() ) )
            {
                vector< FeaStructure* > structVec = StructureMgr.GetAllFeaStructs();

                m_SelectedPartIndexVec.clear();

                for ( size_t i = 2; i <= m_FeaPartSelectBrowser->size(); i++ )
                {
                    if ( m_FeaPartSelectBrowser->selected( i ) )
                    {
                        m_SelectedPartIndexVec.push_back( i - 2 );
                    }
                }

                if ( m_SelectedPartIndexVec.size() == 1 )
                {
                    StructureMgr.SetCurrPartIndex( m_SelectedPartIndexVec[0] );

                    if ( m_SelectedPartIndexVec[0] < structVec[StructureMgr.GetCurrStructIndex()]->NumFeaParts() )
                    {
                        FeaPart* feaprt = structVec[StructureMgr.GetCurrStructIndex()]->GetFeaPart( m_SelectedPartIndexVec[0] );

                        if ( feaprt )
                        {
                            if ( feaprt->GetType() == vsp::FEA_SKIN )
                            {
                                m_SelectedFeaPartChoice = 0; // No dropdown available
                            }
                            else
                            {
                                m_SelectedFeaPartChoice = feaprt->GetType();
                            }
                        }
                    }
                    else if ( m_SelectedPartIndexVec[0] >= structVec[StructureMgr.GetCurrStructIndex()]->NumFeaParts() )
                    {
                        SubSurface* subsurf = structVec[StructureMgr.GetCurrStructIndex()]->GetFeaSubSurf( m_SelectedPartIndexVec[0] - structVec[StructureMgr.GetCurrStructIndex()]->NumFeaParts() );

                        if ( subsurf )
                        {
                            m_SelectedFeaPartChoice = subsurf->GetType() + m_NumFeaPartChoices;
                        }
                    }
                }
                else
                {
                    StructureMgr.SetCurrPartIndex( -1 );
                }

                if ( Fl::event_clicks() != 0 ) // Indicates a double click
                {
                    if ( m_SelectedPartIndexVec.size() == 1 )
                    {
                        StructureMgr.SetCurrPartIndex( m_SelectedPartIndexVec[0] );
                        m_ScreenMgr->ShowScreen( ScreenMgr::VSP_FEA_PART_EDIT_SCREEN );
                    }
                }
            }
        }
        else if ( w == m_StructureSelectBrowser )
        {
            for ( unsigned int iCase = 1; iCase <= m_StructureSelectBrowser->size(); iCase++ )
            {
                if ( m_StructureSelectBrowser->selected( iCase ) )
                {
                    StructureMgr.SetCurrStructIndex( iCase - 2 );
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
        else if ( w == m_FeaPropertySelectBrowser )
        {
            for ( unsigned int iCase = 1; iCase <= m_FeaPropertySelectBrowser->size(); iCase++ )
            {
                if ( m_FeaPropertySelectBrowser->selected( iCase ) )
                {
                    StructureMgr.SetCurrPropertyIndex( iCase - 2 );
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
                    StructureMgr.SetCurrMaterialIndex( iCase - 1 );
                    break;
                }
            }
        }
    }

    m_ScreenMgr->SetUpdateFlag( true );
}

#ifdef WIN32
DWORD WINAPI feamesh_thread_fun( LPVOID data )
#else
void * feamesh_thread_fun( void *data )
#endif
{
    FeaMeshMgr.GenerateFeaMesh();

    StructScreen *cs = (StructScreen *)data;
    if ( cs )
    {
        cs->GetScreenMgr()->SetUpdateFlag( true ); // FeaParts will not be updated when mesh is in progress
    }

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
        FeaMeshMgr.SetFeaMeshStructIndex( StructureMgr.GetCurrStructIndex() );

        m_FeaMeshProcess.StartThread( feamesh_thread_fun, ( void* ) this );

        if ( StructureMgr.ValidTotalFeaStructInd( StructureMgr.GetCurrStructIndex() ) )
        {
            vector < FeaStructure* > structvec = StructureMgr.GetAllFeaStructs();

            structvec[StructureMgr.GetCurrStructIndex()]->GetStructSettingsPtr()->m_DrawMeshFlag = true;
            structvec[StructureMgr.GetCurrStructIndex()]->SetDrawFlag( false );
        }
    }
    else if ( device == &m_ResetDisplayButton )
    {
        if ( StructureMgr.ValidTotalFeaStructInd( StructureMgr.GetCurrStructIndex() ) && FeaMeshMgr.FeaDataAvailable() )
        {
            StructureMgr.ShowAllParts();
            FeaMeshMgr.SetAllDisplayFlags( false );
        }
    }
    else if ( device == &m_WikiLinkButton )
    {
#ifdef  __APPLE__
        system( "open http://www.openvsp.org/wiki/doku.php?id=feamesh" );
#elif   LINUX
        system( "xdg-open http://www.openvsp.org/wiki/doku.php?id=feamesh" );
#else
        system( "start /max /wait http://www.openvsp.org/wiki/doku.php?id=feamesh" );
#endif
    }
    else if ( device == &m_StructUnitChoice )
    {
        StructureMgr.UpdateStructUnit( m_StructUnitChoice.GetVal() );
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
                StructureMgr.InitFeaProperties(); // Add default FeaProperties if none available

                StructureMgr.SetCurrStructIndex( StructureMgr.GetTotFeaStructIndex( newstruct ) );
            }
        }
    }
    else if ( device == &m_DelFeaStructButton )
    {
        vector < FeaStructure* > structvec = StructureMgr.GetAllFeaStructs();

        if ( StructureMgr.ValidTotalFeaStructInd( StructureMgr.GetCurrStructIndex() ) )
        {
            FeaStructure* delstruct = structvec[StructureMgr.GetCurrStructIndex()];

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

        StructureMgr.SetCurrStructIndex( StructureMgr.GetCurrStructIndex() - 1 );

        if ( !StructureMgr.ValidTotalFeaStructInd( StructureMgr.GetCurrStructIndex() ) )
        {
            StructureMgr.SetCurrStructIndex( -1 );
        }
    }
    else if ( device == &m_FeaStructNameInput )
    {
        if ( StructureMgr.ValidTotalFeaStructInd( StructureMgr.GetCurrStructIndex() ) )
        {
            vector < FeaStructure* > structvec = StructureMgr.GetAllFeaStructs();

            FeaStructure* feastruct = structvec[StructureMgr.GetCurrStructIndex()];

            if ( feastruct )
            {
                feastruct->SetName( m_FeaStructNameInput.GetString() );

                if ( feastruct->GetStructSettingsPtr() )
                {
                    feastruct->GetStructSettingsPtr()->ResetExportFileNames( m_FeaStructNameInput.GetString() );
                }
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

        if ( m_SelectedPartIndexVec.size() == 1 )
        {
            if ( m_SelectedPartIndexVec[0] < structvec[StructureMgr.GetCurrStructIndex()]->NumFeaParts() )
            {
                FeaPart* feaprt = structvec[StructureMgr.GetCurrStructIndex()]->GetFeaPart( m_SelectedPartIndexVec[0] );

                if ( feaprt )
                {
                    feaprt->SetName( m_FeaPartNameInput.GetString() );
                }
            }
            else if ( m_SelectedPartIndexVec[0] >= structvec[StructureMgr.GetCurrStructIndex()]->NumFeaParts() )
            {
                SubSurface* subsurf = structvec[StructureMgr.GetCurrStructIndex()]->GetFeaSubSurf( m_SelectedPartIndexVec[0] - structvec[StructureMgr.GetCurrStructIndex()]->NumFeaParts() );

                if ( subsurf )
                {
                    subsurf->SetName( m_FeaPartNameInput.GetString() );
                }
            }
        }
    }
    else if ( device == &m_EditFeaPartButton )
    {
        if ( m_SelectedPartIndexVec.size() == 1 )
        {
            StructureMgr.SetCurrPartIndex( m_SelectedPartIndexVec[0] );

            m_ScreenMgr->ShowScreen( ScreenMgr::VSP_FEA_PART_EDIT_SCREEN );
        }
        else
        {
            AddOutputText( "Error: Select A Single FEA Part\n" );
        }
    }
    else if ( device == &m_FeaPartChoice )
    {
        m_SelectedFeaPartChoice = m_FeaPartChoice.GetVal();
    }
    else if ( device == &m_AddFeaPartButton )
    {
        if ( StructureMgr.ValidTotalFeaStructInd( StructureMgr.GetCurrStructIndex() ) )
        {
            vector < FeaStructure* > structvec = StructureMgr.GetAllFeaStructs();

            m_SelectedPartIndexVec.clear();

            if ( m_FeaPartChoice.GetVal() < m_NumFeaPartChoices )
            {
                FeaPart* feaprt = NULL;

                if ( m_FeaPartChoice.GetVal() == vsp::FEA_SLICE )
                {
                    feaprt = structvec[StructureMgr.GetCurrStructIndex()]->AddFeaPart( vsp::FEA_SLICE );

                    FeaSlice* slice = dynamic_cast<FeaSlice*>( feaprt );
                    assert( slice );

                    slice->m_OrientationPlane.Set( StructureMgr.GetFeaSliceOrientIndex() );
                }
                else if ( m_FeaPartChoice.GetVal() == vsp::FEA_RIB )
                {
                    feaprt = structvec[StructureMgr.GetCurrStructIndex()]->AddFeaPart( vsp::FEA_RIB );
                }
                else if ( m_FeaPartChoice.GetVal() == vsp::FEA_SPAR )
                {
                    feaprt = structvec[StructureMgr.GetCurrStructIndex()]->AddFeaPart( vsp::FEA_SPAR );
                }
                else if ( m_FeaPartChoice.GetVal() == vsp::FEA_FIX_POINT )
                {
                    feaprt = structvec[StructureMgr.GetCurrStructIndex()]->AddFeaPart( vsp::FEA_FIX_POINT );
                }
                else if ( m_FeaPartChoice.GetVal() == vsp::FEA_DOME )
                {
                    feaprt = structvec[StructureMgr.GetCurrStructIndex()]->AddFeaPart( vsp::FEA_DOME );
                }
                else if ( m_FeaPartChoice.GetVal() == vsp::FEA_RIB_ARRAY )
                {
                    feaprt = structvec[StructureMgr.GetCurrStructIndex()]->AddFeaPart( vsp::FEA_RIB_ARRAY );
                }
                else if ( m_FeaPartChoice.GetVal() == vsp::FEA_SLICE_ARRAY )
                {
                    feaprt = structvec[StructureMgr.GetCurrStructIndex()]->AddFeaPart( vsp::FEA_SLICE_ARRAY );
                }

                if ( feaprt )
                {
                    m_SelectedPartIndexVec.push_back( structvec[StructureMgr.GetCurrStructIndex()]->NumFeaParts() - 1 );
                    StructureMgr.SetCurrPartIndex( structvec[StructureMgr.GetCurrStructIndex()]->NumFeaParts() - 1 );
                }
            }
            else
            {
                SubSurface* ssurf = NULL;

                if ( m_FeaPartChoice.GetVal() - m_NumFeaPartChoices == vsp::SS_LINE )
                {
                    ssurf = structvec[StructureMgr.GetCurrStructIndex()]->AddFeaSubSurf( vsp::SS_LINE );
                }
                else if ( m_FeaPartChoice.GetVal() - m_NumFeaPartChoices == vsp::SS_RECTANGLE )
                {
                    ssurf = structvec[StructureMgr.GetCurrStructIndex()]->AddFeaSubSurf( vsp::SS_RECTANGLE );
                }
                else if ( m_FeaPartChoice.GetVal() - m_NumFeaPartChoices == vsp::SS_ELLIPSE )
                {
                    ssurf = structvec[StructureMgr.GetCurrStructIndex()]->AddFeaSubSurf( vsp::SS_ELLIPSE );
                }
                else if ( m_FeaPartChoice.GetVal() - m_NumFeaPartChoices == vsp::SS_CONTROL )
                {
                    ssurf = structvec[StructureMgr.GetCurrStructIndex()]->AddFeaSubSurf( vsp::SS_CONTROL );
                }
                else if ( m_FeaPartChoice.GetVal() - m_NumFeaPartChoices == vsp::SS_LINE_ARRAY )
                {
                    ssurf = structvec[StructureMgr.GetCurrStructIndex()]->AddFeaSubSurf( vsp::SS_LINE_ARRAY );
                }

                if ( ssurf )
                {
                    m_SelectedPartIndexVec.push_back( structvec[StructureMgr.GetCurrStructIndex()]->NumFeaParts() + structvec[StructureMgr.GetCurrStructIndex()]->NumFeaSubSurfs() - 1 );
                    StructureMgr.SetCurrPartIndex( structvec[StructureMgr.GetCurrStructIndex()]->NumFeaParts() + structvec[StructureMgr.GetCurrStructIndex()]->NumFeaSubSurfs() - 1 );
                }
            }
        }
        else
        {
            AddOutputText( "Error: No Structure Selected\n" );
        }
    }
    else if ( device == &m_DelFeaPartButton )
    {
        if ( StructureMgr.ValidTotalFeaStructInd( StructureMgr.GetCurrStructIndex() ) )
        {
            vector < FeaStructure* > structvec = StructureMgr.GetAllFeaStructs();

            if ( m_SelectedPartIndexVec.size() > 0 )
            {
                for ( size_t i = 0; i < m_SelectedPartIndexVec.size(); i++ )
                {
                    FeaPart* prt = structvec[StructureMgr.GetCurrStructIndex()]->GetFeaPart( m_SelectedPartIndexVec[i] );

                    if ( prt )
                    {
                        if ( prt->GetType() == vsp::FEA_SKIN )
                        {
                            m_SelectedPartIndexVec.erase( m_SelectedPartIndexVec.begin() + i ); // Do not include FeaSkin
                            break;
                        }
                    }
                }

                int first_selection = m_SelectedPartIndexVec[0];

                while ( m_SelectedPartIndexVec.size() > 0 )
                {
                    if ( m_SelectedPartIndexVec[0] < structvec[StructureMgr.GetCurrStructIndex()]->NumFeaParts() )
                    {
                        structvec[StructureMgr.GetCurrStructIndex()]->DelFeaPart( m_SelectedPartIndexVec[0] );
                    }
                    else if ( m_SelectedPartIndexVec[0] >= structvec[StructureMgr.GetCurrStructIndex()]->NumFeaParts() )
                    {
                        structvec[StructureMgr.GetCurrStructIndex()]->DelFeaSubSurf( m_SelectedPartIndexVec[0] - structvec[StructureMgr.GetCurrStructIndex()]->NumFeaParts() );
                    }

                    vector < int > temp_index_vec;

                    for ( size_t j = 1; j < m_SelectedPartIndexVec.size(); j++ )
                    {
                        temp_index_vec.push_back( m_SelectedPartIndexVec[j] - 1 );
                    }

                    m_SelectedPartIndexVec = temp_index_vec;
                }

                if ( structvec[StructureMgr.GetCurrStructIndex()]->ValidFeaPartInd( first_selection - 1 ) )
                {
                    m_SelectedPartIndexVec.clear();
                    m_SelectedPartIndexVec.push_back( first_selection - 1 );
                    StructureMgr.SetCurrPartIndex( m_SelectedPartIndexVec[0] );
                }
            }
        }
    }
    else if ( device == &m_DispFeaPartGroup )
    {
        string curr_parm_id = m_DispFeaPartGroup.GetParmID();
        Parm* curr_parm = ParmMgr.FindParm( curr_parm_id );
        bool curr_val = curr_parm->Get();

        if ( StructureMgr.ValidTotalFeaStructInd( StructureMgr.GetCurrStructIndex() ) && m_SelectedPartIndexVec.size() > 1 )
        {
            vector < FeaStructure* > structvec = StructureMgr.GetAllFeaStructs();

            for ( size_t i = 0; i < m_SelectedPartIndexVec.size(); i++ )
            {
                if ( m_SelectedPartIndexVec[i] < structvec[StructureMgr.GetCurrStructIndex()]->NumFeaParts() )
                {
                    FeaPart* prt = structvec[StructureMgr.GetCurrStructIndex()]->GetFeaPart( m_SelectedPartIndexVec[i] );

                    if ( prt )
                    {
                        if ( prt->GetType() != vsp::FEA_SKIN )
                        {
                            prt->m_DrawFeaPartFlag.Set( curr_val );
                        }
                        else
                        {
                            prt->m_DrawFeaPartFlag.Set( true );
                        }
                    }
                }
                else if ( m_SelectedPartIndexVec[i] >= structvec[StructureMgr.GetCurrStructIndex()]->NumFeaParts() )
                {
                    SubSurface* ssurf = structvec[StructureMgr.GetCurrStructIndex()]->GetFeaSubSurf( m_SelectedPartIndexVec[i] - structvec[StructureMgr.GetCurrStructIndex()]->NumFeaParts() );

                    if ( ssurf )
                    {
                        ssurf->m_DrawFeaPartFlag.Set( curr_val );
                    }
                }
            }
        }
    }
    else if ( device == &m_ShellCapToggleGroup )
    {
        string curr_parm_id = m_ShellCapToggleGroup.GetParmID();
        Parm* curr_parm = ParmMgr.FindParm( curr_parm_id );
        double curr_val = curr_parm->Get();

        if ( StructureMgr.ValidTotalFeaStructInd( StructureMgr.GetCurrStructIndex() ) && m_SelectedPartIndexVec.size() > 1 )
        {
            vector < FeaStructure* > structvec = StructureMgr.GetAllFeaStructs();

            for ( size_t i = 0; i < m_SelectedPartIndexVec.size(); i++ )
            {
                if ( m_SelectedPartIndexVec[i] < structvec[StructureMgr.GetCurrStructIndex()]->NumFeaParts() )
                {
                    FeaPart* prt = structvec[StructureMgr.GetCurrStructIndex()]->GetFeaPart( m_SelectedPartIndexVec[i] );

                    if ( prt )
                    {
                        if ( prt->GetType() == vsp::FEA_SKIN )
                        {
                            prt->m_IncludedElements.Set( vsp::FEA_SHELL );
                        }
                        else if ( prt->GetType() != vsp::FEA_FIX_POINT )
                        {
                            prt->m_IncludedElements.Set( (int)curr_val );
                        }
                    }
                }
                else if ( m_SelectedPartIndexVec[i] >= structvec[StructureMgr.GetCurrStructIndex()]->NumFeaParts() )
                {
                    SubSurface* ssurf = structvec[StructureMgr.GetCurrStructIndex()]->GetFeaSubSurf( m_SelectedPartIndexVec[i] - structvec[StructureMgr.GetCurrStructIndex()]->NumFeaParts() );

                    if ( ssurf )
                    {
                        if ( ssurf->GetType() == vsp::SS_LINE_ARRAY  )
                        {
                            ssurf->m_IncludedElements.Set( vsp::FEA_BEAM );
                        }
                        else
                        {
                            ssurf->m_IncludedElements.Set( (int)curr_val );
                        }
                    }
                }
            }
        }
    }
    else if ( device == &m_DrawAllButton )
    {
        FeaMeshMgr.SetAllDisplayFlags( true );
    }
    else if ( device == &m_HideAllButton )
    {
        FeaMeshMgr.SetAllDisplayFlags( false );
    }
    else if ( device == &m_MovePrtUpButton )
    {
        if ( StructureMgr.ValidTotalFeaStructInd( StructureMgr.GetCurrStructIndex() ) && m_SelectedPartIndexVec.size() == 1 )
        {
            vector < FeaStructure* > structvec = StructureMgr.GetAllFeaStructs();

            if ( m_SelectedPartIndexVec[0] < structvec[StructureMgr.GetCurrStructIndex()]->NumFeaParts() )
            {
                structvec[StructureMgr.GetCurrStructIndex()]->ReorderFeaPart( m_SelectedPartIndexVec[0], Vehicle::REORDER_MOVE_UP );

                if ( m_SelectedPartIndexVec[0] != 0 )
                {
                    m_SelectedPartIndexVec[0]--;
                    StructureMgr.SetCurrPartIndex( m_SelectedPartIndexVec[0] );
                }
            }
            else if ( m_SelectedPartIndexVec[0] >= structvec[StructureMgr.GetCurrStructIndex()]->NumFeaParts() )
            {
                structvec[StructureMgr.GetCurrStructIndex()]->ReorderFeaSubSurf( m_SelectedPartIndexVec[0] - structvec[StructureMgr.GetCurrStructIndex()]->NumFeaParts(), Vehicle::REORDER_MOVE_UP );

                if ( m_SelectedPartIndexVec[0] != structvec[StructureMgr.GetCurrStructIndex()]->NumFeaParts() )
                {
                    m_SelectedPartIndexVec[0]--;
                    StructureMgr.SetCurrPartIndex( m_SelectedPartIndexVec[0] );
                }
            }
        }
    }
    else if ( device == &m_MovePrtDownButton )
    {
        if ( StructureMgr.ValidTotalFeaStructInd( StructureMgr.GetCurrStructIndex() ) && m_SelectedPartIndexVec.size() == 1 )
        {
            vector < FeaStructure* > structvec = StructureMgr.GetAllFeaStructs();

            if ( m_SelectedPartIndexVec[0] < structvec[StructureMgr.GetCurrStructIndex()]->NumFeaParts() )
            {
                structvec[StructureMgr.GetCurrStructIndex()]->ReorderFeaPart( m_SelectedPartIndexVec[0], Vehicle::REORDER_MOVE_DOWN );

                if ( m_SelectedPartIndexVec[0] != structvec[StructureMgr.GetCurrStructIndex()]->NumFeaParts() - 1 )
                {
                    m_SelectedPartIndexVec[0]++;
                    StructureMgr.SetCurrPartIndex( m_SelectedPartIndexVec[0] );
                }
            }
            else if ( m_SelectedPartIndexVec[0] >= structvec[StructureMgr.GetCurrStructIndex()]->NumFeaParts() )
            {
                structvec[StructureMgr.GetCurrStructIndex()]->ReorderFeaSubSurf( m_SelectedPartIndexVec[0] - structvec[StructureMgr.GetCurrStructIndex()]->NumFeaParts(), Vehicle::REORDER_MOVE_DOWN );

                if ( m_SelectedPartIndexVec[0] != structvec[StructureMgr.GetCurrStructIndex()]->NumFeaSubSurfs() + structvec[StructureMgr.GetCurrStructIndex()]->NumFeaParts() - 1 )
                {
                    m_SelectedPartIndexVec[0]++;
                    StructureMgr.SetCurrPartIndex( m_SelectedPartIndexVec[0] );
                }
            }
        }
    }
    else if ( device == &m_MovePrtTopButton )
    {
        if ( StructureMgr.ValidTotalFeaStructInd( StructureMgr.GetCurrStructIndex() ) && m_SelectedPartIndexVec.size() == 1 )
        {
            vector < FeaStructure* > structvec = StructureMgr.GetAllFeaStructs();

            if ( m_SelectedPartIndexVec[0] < structvec[StructureMgr.GetCurrStructIndex()]->NumFeaParts() )
            {
                structvec[StructureMgr.GetCurrStructIndex()]->ReorderFeaPart( m_SelectedPartIndexVec[0], Vehicle::REORDER_MOVE_TOP );
                m_SelectedPartIndexVec[0] = 0;
                StructureMgr.SetCurrPartIndex( m_SelectedPartIndexVec[0] );
            }
            else if ( m_SelectedPartIndexVec[0] >= structvec[StructureMgr.GetCurrStructIndex()]->NumFeaParts() )
            {
                structvec[StructureMgr.GetCurrStructIndex()]->ReorderFeaSubSurf( m_SelectedPartIndexVec[0] - structvec[StructureMgr.GetCurrStructIndex()]->NumFeaParts(), Vehicle::REORDER_MOVE_TOP );
                m_SelectedPartIndexVec[0] = structvec[StructureMgr.GetCurrStructIndex()]->NumFeaParts();
                StructureMgr.SetCurrPartIndex( m_SelectedPartIndexVec[0] );
            }
        }
    }
    else if ( device == &m_MovePrtBotButton )
    {
        if ( StructureMgr.ValidTotalFeaStructInd( StructureMgr.GetCurrStructIndex() ) && m_SelectedPartIndexVec.size() == 1 )
        {
            vector < FeaStructure* > structvec = StructureMgr.GetAllFeaStructs();

            if ( m_SelectedPartIndexVec[0] < structvec[StructureMgr.GetCurrStructIndex()]->NumFeaParts() )
            {
                structvec[StructureMgr.GetCurrStructIndex()]->ReorderFeaPart( m_SelectedPartIndexVec[0], Vehicle::REORDER_MOVE_BOTTOM );
                m_SelectedPartIndexVec[0] = structvec[StructureMgr.GetCurrStructIndex()]->NumFeaParts() - 1;
                StructureMgr.SetCurrPartIndex( m_SelectedPartIndexVec[0] );
            }
            else if ( m_SelectedPartIndexVec[0] >= structvec[StructureMgr.GetCurrStructIndex()]->NumFeaParts() )
            {
                structvec[StructureMgr.GetCurrStructIndex()]->ReorderFeaSubSurf( m_SelectedPartIndexVec[0] - structvec[StructureMgr.GetCurrStructIndex()]->NumFeaParts(), Vehicle::REORDER_MOVE_BOTTOM );
                m_SelectedPartIndexVec[0] = structvec[StructureMgr.GetCurrStructIndex()]->NumFeaParts() + structvec[StructureMgr.GetCurrStructIndex()]->NumFeaSubSurfs() - 1;
                StructureMgr.SetCurrPartIndex( m_SelectedPartIndexVec[0] );
            }
        }
    }
    else if ( device == &m_AddFeaPropertyButton )
    {
        StructureMgr.AddFeaProperty( m_FeaPropertyType.GetVal() );
        StructureMgr.SetCurrPropertyIndex( StructureMgr.NumFeaProperties() - 1 );
    }
    else if ( device == &m_DelFeaPropertyButton )
    {
        if ( StructureMgr.ValidFeaPropertyInd( StructureMgr.GetCurrPropertyIndex() ) )
        {
            StructureMgr.DeleteFeaProperty( StructureMgr.GetCurrPropertyIndex() );

            StructureMgr.SetCurrPropertyIndex( StructureMgr.GetCurrPropertyIndex() - 1 );
        }
        else
        {
            StructureMgr.SetCurrPropertyIndex( -1 );
        }
    }
    else if ( device == &m_FeaPropertyNameInput )
    {
        if ( StructureMgr.ValidFeaPropertyInd( StructureMgr.GetCurrPropertyIndex() ) )
        {
            vector < FeaProperty* > property_vec = StructureMgr.GetFeaPropertyVec();
            FeaProperty* fea_prop = property_vec[StructureMgr.GetCurrPropertyIndex()];

            if ( fea_prop )
            {
                fea_prop->SetName( m_FeaPropertyNameInput.GetString() );
            }
        }
    }
    else if ( device == &m_GenPropertyChoice )
    {
        UpdateGenPropertyIndex( &m_GenPropertyChoice );
    }
    else if ( device == &m_GenCapPropertyChoice )
    {
        UpdateGenCapPropertyIndex( &m_GenCapPropertyChoice );
    }
    else if ( device == &m_AddFeaMaterialButton )
    {
        StructureMgr.AddFeaMaterial();
        StructureMgr.SetCurrMaterialIndex( StructureMgr.NumFeaMaterials() - 1 );
    }
    else if ( device == &m_DelFeaMaterialButton )
    {
        if ( StructureMgr.ValidFeaMaterialInd( StructureMgr.GetCurrMaterialIndex() ) )
        {
            StructureMgr.DeleteFeaMaterial( StructureMgr.GetCurrMaterialIndex() );

            StructureMgr.SetCurrMaterialIndex( StructureMgr.GetCurrMaterialIndex() - 1 );
        }
        else
        {
            StructureMgr.SetCurrMaterialIndex( -1 );
        }
    }
    else if ( device == &m_FeaMaterialNameInput )
    {
        if ( StructureMgr.ValidFeaMaterialInd( StructureMgr.GetCurrMaterialIndex() ) )
        {
            vector < FeaMaterial* > material_vec = StructureMgr.GetFeaMaterialVec();
            FeaMaterial* fea_mat = material_vec[StructureMgr.GetCurrMaterialIndex()];

            if ( fea_mat )
            {
                fea_mat->SetName( m_FeaMaterialNameInput.GetString() );
            }
        }
    }
    else if ( device == &m_FeaShellMaterialChoice )
    {
        if ( StructureMgr.ValidFeaPropertyInd( StructureMgr.GetCurrPropertyIndex() ) )
        {
            FeaProperty* fea_prop = StructureMgr.GetFeaPropertyVec()[StructureMgr.GetCurrPropertyIndex()];

            if ( fea_prop )
            {
                fea_prop->m_FeaMaterialIndex.Set( m_FeaShellMaterialChoice.GetVal() );
            }
        }
    }
    else if ( device == &m_FeaBeamMaterialChoice )
    {
        if ( StructureMgr.ValidFeaPropertyInd( StructureMgr.GetCurrPropertyIndex() ) )
        {
            FeaProperty* fea_prop = StructureMgr.GetFeaPropertyVec()[StructureMgr.GetCurrPropertyIndex()];

            if ( fea_prop )
            {
                fea_prop->m_FeaMaterialIndex.Set( m_FeaBeamMaterialChoice.GetVal() );
            }
        }
    }
    else if ( device == &m_ShowFeaBeamXSecButton )
    {
        // Show XSec Diagram with Dimensions
        m_ScreenMgr->ShowScreen( ScreenMgr::VSP_FEA_XSEC_SCREEN );
    }
    else if ( device == &m_SelectStlFile )
    {
        if ( StructureMgr.ValidTotalFeaStructInd( StructureMgr.GetCurrStructIndex() ) )
        {
            vector < FeaStructure* > structvec = StructureMgr.GetAllFeaStructs();

            string newfile = m_ScreenMgr->GetSelectFileScreen()->FileChooser( "Select .stl file.", "*.stl" );
            if ( newfile.compare( "" ) != 0 )
            {
                structvec[StructureMgr.GetCurrStructIndex()]->GetStructSettingsPtr()->SetExportFileName( newfile, vsp::FEA_STL_FILE_NAME );
            }
        }
    }
    else if ( device == &m_SelectMassFile )
    {
        if ( StructureMgr.ValidTotalFeaStructInd( StructureMgr.GetCurrStructIndex() ) )
        {
            vector < FeaStructure* > structvec = StructureMgr.GetAllFeaStructs();

            string newfile = m_ScreenMgr->GetSelectFileScreen()->FileChooser( "Select mass .txt file.", "*.txt" );
            if ( newfile.compare( "" ) != 0 )
            {
                structvec[StructureMgr.GetCurrStructIndex()]->GetStructSettingsPtr()->SetExportFileName( newfile, vsp::FEA_MASS_FILE_NAME );
            }
        }
    }
    else if ( device == &m_SelectNastFile )
    {
        if ( StructureMgr.ValidTotalFeaStructInd( StructureMgr.GetCurrStructIndex() ) )
        {
            vector < FeaStructure* > structvec = StructureMgr.GetAllFeaStructs();

            string newfile = m_ScreenMgr->GetSelectFileScreen()->FileChooser( "Select NASTRAN .dat file.", "*.dat" );
            if ( newfile.compare( "" ) != 0 )
            {
                structvec[StructureMgr.GetCurrStructIndex()]->GetStructSettingsPtr()->SetExportFileName( newfile, vsp::FEA_NASTRAN_FILE_NAME );
            }
        }
    }
    else if ( device == &m_SelectNkeyFile )
    {
        if ( StructureMgr.ValidTotalFeaStructInd( StructureMgr.GetCurrStructIndex() ) )
        {
            vector < FeaStructure* > structvec = StructureMgr.GetAllFeaStructs();

            string newfile = m_ScreenMgr->GetSelectFileScreen()->FileChooser( "Select NASTRAN key file.", "*.nkey" );
            if ( newfile.compare( "" ) != 0 )
            {
                structvec[StructureMgr.GetCurrStructIndex()]->GetStructSettingsPtr()->SetExportFileName( newfile, vsp::FEA_NKEY_FILE_NAME );
            }
        }
    }
    else if ( device == &m_SelectCalcFile  )
    {
        if ( StructureMgr.ValidTotalFeaStructInd( StructureMgr.GetCurrStructIndex() ) )
        {
            vector < FeaStructure* > structvec = StructureMgr.GetAllFeaStructs();

            string newfile = m_ScreenMgr->GetSelectFileScreen()->FileChooser( "Select Calculix .dat file.", "*.dat" );
            if ( newfile.compare( "" ) != 0 )
            {
                structvec[StructureMgr.GetCurrStructIndex()]->GetStructSettingsPtr()->SetExportFileName( newfile, vsp::FEA_CALCULIX_FILE_NAME );
            }
        }
    }
    else if ( device == &m_SelectGmshFile )
    {
        if ( StructureMgr.ValidTotalFeaStructInd( StructureMgr.GetCurrStructIndex() ) )
        {
            vector < FeaStructure* > structvec = StructureMgr.GetAllFeaStructs();

            string newfile = m_ScreenMgr->GetSelectFileScreen()->FileChooser( "Select .msh file.", "*.msh" );
            if ( newfile.compare( "" ) != 0 )
            {
                structvec[StructureMgr.GetCurrStructIndex()]->GetStructSettingsPtr()->SetExportFileName( newfile, vsp::FEA_GMSH_FILE_NAME );
            }
        }
    }
    else if ( device == &m_SelectSrfFile )
    {
        if ( StructureMgr.ValidTotalFeaStructInd( StructureMgr.GetCurrStructIndex() ) )
        {
            vector < FeaStructure* > structvec = StructureMgr.GetAllFeaStructs();

            string newfile = m_ScreenMgr->GetSelectFileScreen()->FileChooser( "Select .srf file.", "*.srf" );
            if ( newfile.compare( "" ) != 0 )
            {
                structvec[StructureMgr.GetCurrStructIndex()]->GetStructSettingsPtr()->SetExportFileName( newfile, vsp::FEA_SRF_FILE_NAME );
            }
        }
    }
    else if ( device == &m_SelectCurvFile )
    {
        if ( StructureMgr.ValidTotalFeaStructInd( StructureMgr.GetCurrStructIndex() ) )
        {
            vector < FeaStructure* > structvec = StructureMgr.GetAllFeaStructs();

            string newfile = m_ScreenMgr->GetSelectFileScreen()->FileChooser( "Select GridTool .curv file.", "*.curv" );
            if ( newfile.compare( "" ) != 0 )
            {
                structvec[StructureMgr.GetCurrStructIndex()]->GetStructSettingsPtr()->SetExportFileName( newfile, vsp::FEA_CURV_FILE_NAME );
            }
        }
    }
    else if ( device == &m_SelectPlot3DFile )
    {
        if ( StructureMgr.ValidTotalFeaStructInd( StructureMgr.GetCurrStructIndex() ) )
        {
            vector < FeaStructure* > structvec = StructureMgr.GetAllFeaStructs();

            string newfile = m_ScreenMgr->GetSelectFileScreen()->FileChooser( "Select Plot3D .p3d file.", "*.p3d" );
            if ( newfile.compare( "" ) != 0 )
            {
                structvec[StructureMgr.GetCurrStructIndex()]->GetStructSettingsPtr()->SetExportFileName( newfile, vsp::FEA_PLOT3D_FILE_NAME );
            }
        }
    }

    m_ScreenMgr->SetUpdateFlag( true );
}

void StructScreen::OrientStructure( VSPGraphic::Common::VSPenum type )
{
    if ( !StructureMgr.ValidTotalFeaStructInd( StructureMgr.GetCurrStructIndex() ) )
    {
        return;
    }

    FeaStructure* curr_struct = StructureMgr.GetAllFeaStructs()[StructureMgr.GetCurrStructIndex()];
    Vehicle* veh = m_ScreenMgr->GetVehiclePtr();

    if ( veh && curr_struct )
    {
        Geom* curr_geom = veh->FindGeom( curr_struct->GetParentGeomID() );

        MainVSPScreen* main = dynamic_cast<MainVSPScreen*>( m_ScreenMgr->GetScreen( m_ScreenMgr->VSP_MAIN_SCREEN ) );

        if ( curr_geom && main )
        {
            if ( type == VSPGraphic::Common::VSP_CAM_FRONT )
            {
                curr_geom->m_GuiDraw.SetDisplayType( DISPLAY_TYPE::DISPLAY_DEGEN_SURF );
            }
            else
            {
                curr_geom->m_GuiDraw.SetDisplayType( DISPLAY_TYPE::DISPLAY_DEGEN_CAMBER );
            }

            curr_geom->m_GuiDraw.SetDrawType( vsp::DRAW_TYPE::GEOM_DRAW_SHADE );
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

            glwin->getGraphicEngine()->getDisplay()->setCOR( (float)-p.x(), (float)-p.y(), (float)-p.z() );
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
    if ( !StructureMgr.ValidTotalFeaStructInd( StructureMgr.GetCurrStructIndex() ) )
    {
        return;
    }

    FeaStructure* curr_struct = StructureMgr.GetAllFeaStructs()[StructureMgr.GetCurrStructIndex()];
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
        if ( !FeaMeshMgr.GetFeaMeshInProgress() && StructureMgr.ValidTotalFeaStructInd( StructureMgr.GetCurrStructIndex() ) )
        {
            vector < FeaStructure* > totalstructvec = StructureMgr.GetAllFeaStructs();

            FeaStructure* curr_struct = totalstructvec[StructureMgr.GetCurrStructIndex()];

            if ( !curr_struct )
            {
                return;
            }

            Update(); // Updating right before drawing ensures the structure is up to date

            vector < FeaPart* > partvec = curr_struct->GetFeaPartVec();

            for ( unsigned int i = 0; i < (int)partvec.size(); i++ )
            {
                if ( partvec[i]->m_DrawFeaPartFlag() )
                {
                    partvec[i]->LoadDrawObjs( draw_obj_vec );
                }
            }

            vector < SubSurface* > subsurf_vec = curr_struct->GetFeaSubSurfVec();
            vector < int > active_index_vec;

            for ( size_t i = 0; i < m_SelectedPartIndexVec.size(); i++ )
            {
                if ( m_SelectedPartIndexVec[i] >= curr_struct->NumFeaParts() )
                {
                    active_index_vec.push_back( m_SelectedPartIndexVec[i] - curr_struct->NumFeaParts() );
                }
            }

            curr_struct->RecolorFeaSubSurfs( active_index_vec );

            for ( unsigned int i = 0; i < (int)subsurf_vec.size(); i++ )
            {
                if ( subsurf_vec[i]->m_DrawFeaPartFlag() )
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

    if ( StructureMgr.ValidTotalFeaStructInd( StructureMgr.GetCurrStructIndex() ) )
    {
        vector < FeaStructure* > totalstructvec = StructureMgr.GetAllFeaStructs();

        FeaStructure* curr_struct = totalstructvec[StructureMgr.GetCurrStructIndex()];

        if ( !curr_struct )
        {
            return;
        }

        int k = 0;

        vector < FeaPart* > curr_part_vec;

        for ( size_t i = 0; i < m_SelectedPartIndexVec.size(); i++ )
        {
            curr_part_vec.push_back( curr_struct->GetFeaPart( m_SelectedPartIndexVec[i] ) );
        }

        vector < FeaPart* > partvec = curr_struct->GetFeaPartVec();

        for ( unsigned int i = 0; i < (int)partvec.size(); i++ )
        {
            bool match = false;

            for ( size_t j = 0; j < curr_part_vec.size(); j++ )
            {
                if ( partvec[i] == curr_part_vec[j] )
                {
                    match = true;
                }
            }

            if ( partvec[i]->m_DrawFeaPartFlag() )
            {
                partvec[i]->UpdateDrawObjs( k, match );
                k++;
            }
        }
    }
}
