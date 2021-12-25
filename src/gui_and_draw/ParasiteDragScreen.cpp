//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//

// ParasiteDragScreen.cpp: implementation of the ParasiteDragScreen class.
//
//////////////////////////////////////////////////////////////////////

#include "ParasiteDragScreen.h"
#include "ParasiteDragMgr.h"
#include "StlHelper.h"

#include <numeric>
using namespace std;

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
#define DRAG_TAB_WIDTH 310          // width of the tab group
#define DRAG_TABLE_WIDTH 470        // width of the drag build-up table
#define DRAG_TABLE_PERSISTENT_WIDTH 180 // width of persistent sectino of drag table
#define TOTAL_WINDOW_HEIGHT 580     // Entire Window Height
#define EXECUTE_LAYOUT_HEIGHT 65    // height needed for dividerbox and two buttons
#define TYPICAL_INPUT_WIDTH 70      // input width for most table cells

#define OUT                         // Used to designate which variables are out variables in methods

ParasiteDragScreen::ParasiteDragScreen( ScreenMgr* mgr ) : TabScreen( mgr,
            DRAG_TAB_WIDTH + DRAG_TABLE_WIDTH + DRAG_TABLE_PERSISTENT_WIDTH + 10, TOTAL_WINDOW_HEIGHT,
            "Parasite Drag", EXECUTE_LAYOUT_HEIGHT + 5, DRAG_TABLE_WIDTH + DRAG_TABLE_PERSISTENT_WIDTH + 10 )
{
    // Used for Unit Labels
    string deg( 1, (char) 176 );
    string squared( 1, (char) 178 );
    string cubed( 1, (char) 179 );
    string temp;

    // Other Size Variables
    int drag_table_height = TOTAL_WINDOW_HEIGHT - 160;
    int documentation_display_height = TOTAL_WINDOW_HEIGHT - 165;
    int excrescence_browser_height = TOTAL_WINDOW_HEIGHT - 285;

    // Inputs
    stringstream strs;
    int layoutHeight = 600;

    // Add Tabs
    Fl_Group* overview_tab = AddTab( "Overview" );
    Fl_Group* excrescence_tab = AddTab( "Excrescence" );
    Fl_Group* documentation_tab = AddTab( "Documentation" );

    overview_tab->show();

    //==== Draw Out Outputs Tab ====//
    Fl_Group* outputs_group = AddSubGroup( overview_tab, 5 );
    m_OutputsLayout.SetGroupAndScreen( outputs_group, this );

    // Set up Options Layout on Left Side of GUI
    m_OutputsLayout.AddSubGroupLayout( m_OptionsLayout, DRAG_TAB_WIDTH - 10, layoutHeight - EXECUTE_LAYOUT_HEIGHT );

    // Setup persistent (always shown) sub-groups
    m_PersistenceLayout.SetGroupAndScreen( m_FLTK_Window, this );

    m_PersistenceLayout.AddX( m_OptionsLayout.GetW() + 15 );
    m_PersistenceLayout.AddY( m_OptionsLayout.GetY() );

    // Set up Slot for Component Label Layout in top left corner of Persistent Space to the Right
    m_PersistenceLayout.AddSubGroupLayout( m_ComponentLabelLayout, TYPICAL_INPUT_WIDTH * 2 + 20, m_PersistenceLayout.GetStdHeight() );

    // Create Scroll Group for Component Label
    m_ComponentLabelScrollGroup = m_ComponentLabelLayout.AddFlScroll( m_PersistenceLayout.GetStdHeight() * 2 );

    m_PersistenceLayout.AddX( m_ComponentLabelLayout.GetW() );

    // Set up Table Labels Layout in Persistent Space to the Right
    m_PersistenceLayout.AddSubGroupLayout( m_MainTableLabelsLayout, DRAG_TABLE_WIDTH - m_ComponentLabelLayout.GetW(), layoutHeight - EXECUTE_LAYOUT_HEIGHT );

    // Create Scroll Group for Table Labels
    m_MainTableLabelsScrollGroup = m_MainTableLabelsLayout.AddFlScroll( m_PersistenceLayout.GetStdHeight() * 2 );

    m_PersistenceLayout.AddX( m_MainTableLabelsLayout.GetW() );

    // Set up Const Table Labels Layout in Persistnet Space to the Right
    m_PersistenceLayout.AddSubGroupLayout( m_ConstantTableLabelsLayout, DRAG_TABLE_PERSISTENT_WIDTH, layoutHeight - EXECUTE_LAYOUT_HEIGHT );

    // Create Scroll Group for Const Labels
    m_ConstTableLabelsScrollGroup = m_ConstantTableLabelsLayout.AddFlScroll( m_PersistenceLayout.GetStdHeight() * 2 );

    m_PersistenceLayout.AddY( m_PersistenceLayout.GetStdHeight() + 2 );
    m_PersistenceLayout.AddX( -( m_MainTableLabelsLayout.GetW() + m_ComponentLabelLayout.GetW() ) );

    // Set up Table Labels Layout in Persistent Space to the Right
    m_PersistenceLayout.AddSubGroupLayout( m_TableCompNamesLayout, TYPICAL_INPUT_WIDTH * 2 + 20, layoutHeight - EXECUTE_LAYOUT_HEIGHT );

    // Create Scroll Group for Table Labels
    m_MainTableCompNamesScrollGroup = m_TableCompNamesLayout.AddFlScroll( drag_table_height - 15 );
    m_MainTableCompNamesScrollGroup->type( Fl_Scroll::VERTICAL );

    m_LabelScrollbarCB = m_MainTableCompNamesScrollGroup->scrollbar.callback();
    m_MainTableCompNamesScrollGroup->scrollbar.callback( staticScreenCB, this );

    m_PersistenceLayout.AddX( m_TableCompNamesLayout.GetW() );

    // Set up Table Layout in Persistent Space to the Right
    m_PersistenceLayout.AddSubGroupLayout( m_TableLayout, DRAG_TABLE_WIDTH - m_TableCompNamesLayout.GetW(), layoutHeight );

    // Set the Main Table as the Resizable Portion
    m_FLTK_Window->resizable( m_TableLayout.GetGroup() );
    m_FLTK_Window->size_range( m_FLTK_Window->w(), m_FLTK_Window->h() );
    m_TableLayout.SetSameLineFlag( false );

    // Create Scroll Group for Table Rows
    m_MainTableScrollGroup = m_TableLayout.AddFlScroll( drag_table_height );
    m_MainTableScrollGroup->type( Fl_Scroll::BOTH );


    m_TableLayout.SetFitWidthFlag( false );

    // Save Callbacks to Perform Update on Slide Event
    m_MainScrollbarCB = m_MainTableScrollGroup->scrollbar.callback();
    m_MainTableScrollGroup->scrollbar.callback( staticScreenCB, this );
    m_MainTableScrollGroup->scrollbar.when( FL_WHEN_CHANGED );
    m_HScrollbarCB = m_MainTableScrollGroup->hscrollbar.callback();
    m_MainTableScrollGroup->hscrollbar.callback( staticScreenCB, this );
    m_MainTableScrollGroup->hscrollbar.when( FL_WHEN_CHANGED );

    m_PersistenceLayout.AddX( m_TableLayout.GetW() );

    // Set up Constant View Layout
    m_PersistenceLayout.AddSubGroupLayout( m_ConstantViewLayout, DRAG_TABLE_PERSISTENT_WIDTH, layoutHeight - EXECUTE_LAYOUT_HEIGHT );

    // Create Scroll Group for Constant Table Rows
    m_ConstantTableScrollGroup = m_ConstantViewLayout.AddFlScroll( drag_table_height - 15 );
    m_ConstantTableScrollGroup->type( Fl_Scroll::VERTICAL );

    // Save Callback for Vertical Scrollbar to Perform Update on Slide Event
    m_ConstantScrollbarCB = m_ConstantTableScrollGroup->scrollbar.callback();
    m_ConstantTableScrollGroup->scrollbar.callback( staticScreenCB, this );
    m_ConstantTableScrollGroup->scrollbar.when( FL_WHEN_CHANGED );

    m_ConstantViewLayout.AddYGap();

    // Lower left execute and export area
    m_PersistenceLayout.SetY( TOTAL_WINDOW_HEIGHT - EXECUTE_LAYOUT_HEIGHT );
    m_PersistenceLayout.SetX( 5 );
    m_PersistenceLayout.AddSubGroupLayout( m_ExecuteLayout, DRAG_TAB_WIDTH, EXECUTE_LAYOUT_HEIGHT );

    //---- Execute ----//
    m_ExecuteLayout.SetSameLineFlag( false );
    m_ExecuteLayout.SetFitWidthFlag( true );
    m_ExecuteLayout.AddDividerBox( "Execute" );
    m_ExecuteLayout.AddButton( m_calc, "Calculate CD0" );
    m_ExecuteLayout.SetSameLineFlag( true );
    m_ExecuteLayout.SetFitWidthFlag( false );
    m_ExecuteLayout.SetButtonWidth( m_ExecuteLayout.GetW() / 2 );
    m_ExecuteLayout.AddButton( m_SubCompExportToggle, "Export Sub-Components" );
    m_ExecuteLayout.AddButton( m_export, "Export to *.csv" );
    m_ExecuteLayout.ForceNewLine();

    // Add Final Outputs
    m_ConstantViewLayout.SetSameLineFlag( true );
    m_ConstantViewLayout.SetFitWidthFlag( false );
    m_ConstantViewLayout.SetButtonWidth( TYPICAL_INPUT_WIDTH - 10 );
    m_ConstantViewLayout.AddButton( m_fTotalLabel, "" );
    m_fTotalLabel.GetFlButton()->box( FL_THIN_UP_BOX );
    m_fTotalLabel.GetFlButton()->labelcolor( FL_BLACK );
    m_ConstantViewLayout.AddLabel( "C_D", TYPICAL_INPUT_WIDTH - 10 );
    m_ConstantViewLayout.AddLabel( "% Total", TYPICAL_INPUT_WIDTH - 10 );
    m_ConstantViewLayout.SetSameLineFlag( true );
    m_ConstantViewLayout.SetFitWidthFlag( false );
    m_ConstantViewLayout.ForceNewLine();
    m_ConstantViewLayout.SetButtonWidth( 0 );
    m_ConstantViewLayout.SetInputWidth( TYPICAL_INPUT_WIDTH - 10 );
    m_ConstantViewLayout.AddX( -TYPICAL_INPUT_WIDTH );
    m_ConstantViewLayout.AddLabel( "Geom:", TYPICAL_INPUT_WIDTH );
    m_ConstantViewLayout.AddOutput( m_GeomfTotal, "" );
    m_ConstantViewLayout.AddOutput( m_GeomCdTotal, "" );
    m_ConstantViewLayout.AddOutput( m_GeomPercTotal, "" );
    m_ConstantViewLayout.ForceNewLine();

    m_ConstantViewLayout.AddX( -TYPICAL_INPUT_WIDTH );
    m_ConstantViewLayout.AddLabel( "Excres:", TYPICAL_INPUT_WIDTH );
    m_ConstantViewLayout.AddOutput( m_ExcresfTotal, "" );
    m_ConstantViewLayout.AddOutput( m_ExcresCdTotal, "" );
    m_ConstantViewLayout.AddOutput( m_ExcresPercTotal, "" );
    m_ConstantViewLayout.ForceNewLine();
    m_ConstantViewLayout.AddYGap();

    m_ConstantViewLayout.AddX( -TYPICAL_INPUT_WIDTH );
    m_ConstantViewLayout.AddLabel( "Total:", TYPICAL_INPUT_WIDTH );
    m_ConstantViewLayout.AddOutput( m_fTotal, "" );
    m_ConstantViewLayout.AddOutput( m_CDTotal, "" );
    m_ConstantViewLayout.AddOutput( m_PercTotal, "" );

    // Add GUI Elements to Options Layout
    //---- Geometry ----//
    m_OptionsLayout.AddDividerBox( "Geometry" );
    m_OptionsLayout.AddChoice( m_SetChoice, "Geometry Set: " );
    m_OptionsLayout.AddChoice( m_ModelLengthUnitChoice, "Model Length Unit" );
    m_ModelLengthUnitChoice.AddItem( "mm" );
    m_ModelLengthUnitChoice.AddItem( "cm" );
    m_ModelLengthUnitChoice.AddItem( "m" );
    m_ModelLengthUnitChoice.AddItem( "in" );
    m_ModelLengthUnitChoice.AddItem( "ft" );
    m_ModelLengthUnitChoice.AddItem( "yd" );
    m_ModelLengthUnitChoice.AddItem( "Unitless" );
    m_ModelLengthUnitChoice.UpdateItems();
    m_OptionsLayout.AddYGap();

    //---- Table Options ----//
    m_OptionsLayout.AddDividerBox( "Equation Selection" );
    m_OptionsLayout.AddChoice( m_LamCfEqnChoice, "Lam. Cf Eqn:" );
    m_LamCfEqnChoice.AddItem( "Blasius" );
    m_LamCfEqnChoice.UpdateItems();

    m_OptionsLayout.AddChoice( m_TurbCfEqnChoice, "Turb. Cf Eqn:" );
    m_TurbCfEqnChoice.AddItem( "Explicit Fit of Spalding" );
    m_TurbCfEqnChoice.AddItem( "Explicit Fit of Spalding-Chi" );
    m_TurbCfEqnChoice.AddItem( "Explicit Fit of Schoenherr" );
    m_TurbCfEqnChoice.AddItem( "Implicit Schoenherr" );
    m_TurbCfEqnChoice.AddItem( "Implicit Karman-Schoenherr" );
    m_TurbCfEqnChoice.AddItem( "Power Law Blasius" );
    m_TurbCfEqnChoice.AddItem( "Power Law Prandtl Low Re" );
    m_TurbCfEqnChoice.AddItem( "Power Law Prandtl Medium Re" );
    m_TurbCfEqnChoice.AddItem( "Power Law Prandtl High Re" );
    m_TurbCfEqnChoice.AddItem( "Schlichting Compressible" );
    m_TurbCfEqnChoice.AddItem( "_Schultz-Grunow Estimate of Schoenherr" );

    m_TurbCfEqnChoice.AddItem( "_Roughness Schlichting Avg" );

    m_TurbCfEqnChoice.AddItem( "_Roughness Schlichting Avg Compressible" );

    m_TurbCfEqnChoice.AddItem( "Heat Transfer White-Christoph" );
    m_TurbCfEqnChoice.UpdateItems();

    int tmp[] = { vsp::CF_TURB_EXPLICIT_FIT_SPALDING,
                  vsp::CF_TURB_EXPLICIT_FIT_SPALDING_CHI,
                  vsp::CF_TURB_EXPLICIT_FIT_SCHOENHERR,
                  vsp::CF_TURB_IMPLICIT_SCHOENHERR,
                  vsp::CF_TURB_IMPLICIT_KARMAN_SCHOENHERR,
                  vsp::CF_TURB_POWER_LAW_BLASIUS,
                  vsp::CF_TURB_POWER_LAW_PRANDTL_LOW_RE,
                  vsp::CF_TURB_POWER_LAW_PRANDTL_MEDIUM_RE,
                  vsp::CF_TURB_POWER_LAW_PRANDTL_HIGH_RE,
                  vsp::CF_TURB_SCHLICHTING_COMPRESSIBLE,
                  vsp::CF_TURB_SCHULTZ_GRUNOW_SCHOENHERR,
                  vsp::CF_TURB_ROUGHNESS_SCHLICHTING_AVG,
                  vsp::CF_TURB_ROUGHNESS_SCHLICHTING_AVG_FLOW_CORRECTION,
                  vsp::CF_TURB_HEATTRANSFER_WHITE_CHRISTOPH };
    m_TurbChoiceToEnum.insert( m_TurbChoiceToEnum.begin(), tmp, &tmp[ sizeof( tmp ) / sizeof( *tmp ) ] );

    m_OptionsLayout.AddYGap();

    //---- Reference Area ----//
    int button_width = 65;
    int slider_width = 70;
    int input_width = 70;
    int choice_button_width = 50;
    int choice_list_width = button_width + 10;
    m_OptionsLayout.AddDividerBox( "Reference Area" );
    m_OptionsLayout.SetSameLineFlag( true );
    m_OptionsLayout.SetFitWidthFlag( false );
    m_OptionsLayout.SetChoiceButtonWidth( choice_button_width );
    m_OptionsLayout.SetButtonWidth( m_OptionsLayout.GetW() / 2 );
    m_OptionsLayout.AddButton( m_RefManualToggle, "Manual" );
    m_OptionsLayout.AddButton( m_RefChoiceToggle, "From Model" );
    m_OptionsLayout.ForceNewLine();
    m_OptionsLayout.InitWidthHeightVals();
    m_OptionsLayout.SetSameLineFlag( false );
    m_OptionsLayout.SetFitWidthFlag( true );
    m_OptionsLayout.AddChoice( m_RefWingChoice, "Ref. Wing" );
    m_OptionsLayout.SetFitWidthFlag( false );
    m_OptionsLayout.SetSameLineFlag( true );
    m_OptionsLayout.SetButtonWidth( button_width );
    m_OptionsLayout.SetSliderWidth( slider_width );
    m_OptionsLayout.SetInputWidth( input_width );
    m_OptionsLayout.AddSlider( m_SrefSlider, "Sref", 1000.0, "%7.2f" );
    temp = "ft" + squared;
    m_OptionsLayout.AddButton( m_SrefUnitLabel, temp.c_str() );
    m_SrefUnitLabel.SetWidth( choice_list_width );
    m_SrefUnitLabel.GetFlButton()->labelcolor( FL_BLACK );
    m_OptionsLayout.ForceNewLine();
    m_OptionsLayout.SetFitWidthFlag( true );
    m_OptionsLayout.SetSameLineFlag( false );
    m_RefToggle.Init( this );
    m_RefToggle.AddButton( m_RefManualToggle.GetFlButton() );
    m_RefToggle.AddButton( m_RefChoiceToggle.GetFlButton() );
    m_OptionsLayout.AddYGap();

    //---- Flow Condition ----//
    choice_button_width = 80;
    m_OptionsLayout.SetChoiceButtonWidth( choice_button_width );
    m_OptionsLayout.AddDividerBox( "Flow Condition" );
    m_OptionsLayout.AddChoice( m_FreestreamTypeChoice, "Atmosphere" );
    m_FreestreamTypeChoice.AddItem( "US Standard Atmosphere 1976" );
    m_FreestreamTypeChoice.AddItem( "USAF 1966" );
    m_FreestreamTypeChoice.AddItem( "Pres + Density Control" );
    m_FreestreamTypeChoice.AddItem( "Pres + Temp Control" );
    m_FreestreamTypeChoice.AddItem( "Density + Temp Control" );
    m_FreestreamTypeChoice.AddItem( "Re\\/L + Mach Control" );
    // TODO: Add Total Control of All Parameters
    m_FreestreamTypeChoice.UpdateItems();
    m_OptionsLayout.AddYGap();

    // Atmosphere Properties
    choice_button_width = 0;
    m_OptionsLayout.SetFitWidthFlag( false );
    m_OptionsLayout.SetSameLineFlag( true );
    m_OptionsLayout.SetButtonWidth( button_width );
    m_OptionsLayout.SetSliderWidth( slider_width );
    m_OptionsLayout.SetInputWidth( input_width );
    m_OptionsLayout.SetChoiceButtonWidth( choice_button_width );

    // Velocity Input and Units
    m_OptionsLayout.AddSlider( m_FlowParmVinfSlider, "Vinf", 200.0, "%7.2f" );
    m_OptionsLayout.AddChoice( m_VinfUnitChoice, "" );
    m_VinfUnitChoice.AddItem( "ft\\/s" );
    m_VinfUnitChoice.AddItem( "m\\/s" );
    m_VinfUnitChoice.AddItem( "mph" );
    m_VinfUnitChoice.AddItem( "km\\/hr" );
    m_VinfUnitChoice.AddItem( "KEAS" );
    m_VinfUnitChoice.AddItem( "KTAS" );
    m_VinfUnitChoice.AddItem( "Mach" );
    m_VinfUnitChoice.UpdateItems();
    m_VinfUnitChoice.SetWidth( choice_list_width );
    m_OptionsLayout.ForceNewLine();

    // Altitude Input and Units
    m_OptionsLayout.AddSlider( m_FlowParmHinfSlider, "Alt", 10000.0, "%7.1f" );
    m_OptionsLayout.AddChoice( m_AltUnitChoice, "" );
    m_AltUnitChoice.AddItem( "ft" );
    m_AltUnitChoice.AddItem( "m" );
    m_AltUnitChoice.UpdateItems();
    m_AltUnitChoice.SetWidth( choice_list_width );
    m_OptionsLayout.ForceNewLine();

    // Temperature Input and Units
    m_OptionsLayout.AddSlider( m_FlowParmTempSlider, "Temp", 100.0, "%7.1f" );
    m_OptionsLayout.AddChoice( m_TempUnitChoice, "" );
    m_TempUnitChoice.AddItem( "K" );
    temp = deg + "C";
    m_TempUnitChoice.AddItem( temp );
    temp = deg + "F";
    m_TempUnitChoice.AddItem( temp );
    temp = deg + "R";
    m_TempUnitChoice.AddItem( temp );
    m_TempUnitChoice.UpdateItems();
    m_TempUnitChoice.SetWidth( choice_list_width );
    m_OptionsLayout.ForceNewLine();
    m_OptionsLayout.AddSlider( m_FlowParmDeltaTempSlider, "dTemp", 100.0, "%7.2f" );
    temp = deg + "F";
    m_OptionsLayout.AddButton( m_DeltaTempUnitLabel, temp.c_str() );
    m_DeltaTempUnitLabel.GetFlButton()->box( FL_THIN_UP_BOX );
    m_DeltaTempUnitLabel.SetWidth( choice_list_width );
    m_DeltaTempUnitLabel.GetFlButton()->labelcolor( FL_BLACK );
    m_OptionsLayout.ForceNewLine();

    // Pressure Input and Units
    m_OptionsLayout.AddSlider( m_FlowParmPresSlider, "Pres", 100.0, "%7.3f" );
    m_OptionsLayout.AddChoice( m_PresUnitChoice, "" );
    temp = "lbf\\/ft" + squared;
    m_PresUnitChoice.AddItem( temp );
    temp = "lbf\\/in" + squared;
    m_PresUnitChoice.AddItem( temp );
    m_PresUnitChoice.AddItem( "Ba" );
    m_PresUnitChoice.AddItem( "Pa" );
    m_PresUnitChoice.AddItem( "kPa" );
    m_PresUnitChoice.AddItem( "MPa" );
    m_PresUnitChoice.AddItem( "inchHg" );
    m_PresUnitChoice.AddItem( "mmHg" );
    m_PresUnitChoice.AddItem( "mmH20" );
    m_PresUnitChoice.AddItem( "mB" );
    m_PresUnitChoice.UpdateItems();
    m_PresUnitChoice.SetWidth( choice_list_width );
    m_OptionsLayout.ForceNewLine();

    // Density Input and Units
    m_OptionsLayout.AddSlider( m_FlowParmRhoSlider, "Density", 0.001, "%1.3e" );
    m_OptionsLayout.AddButton( m_RhoUnitLabel, "" );
    m_RhoUnitLabel.GetFlButton()->box( FL_THIN_UP_BOX );
    m_RhoUnitLabel.GetFlButton()->labelcolor( FL_BLACK );
    m_RhoUnitLabel.SetWidth( choice_list_width );
    m_OptionsLayout.ForceNewLine();

    // Specific Heat Ratio Input and Units
    m_OptionsLayout.AddSlider( m_FlowParmSpecificHeatRatioSlider, "Gamma", 1, "%1.3f" );
    m_OptionsLayout.ForceNewLine();

    // Dynamic Viscosity Input and Units
    m_OptionsLayout.AddSlider( m_FlowParmDynaViscSlider, "Dyn Visc", 0.01, "%1.3e" );
    m_OptionsLayout.AddButton( m_DynViscUnitLabel, "" );
    m_DynViscUnitLabel.GetFlButton()->box( FL_THIN_UP_BOX );
    m_DynViscUnitLabel.GetFlButton()->labelcolor( FL_BLACK );
    m_DynViscUnitLabel.SetWidth( choice_list_width );
    m_OptionsLayout.ForceNewLine();

    // Medium Selection
    //m_OptionsLayout.SetFitWidthFlag(true);
    //m_OptionsLayout.SetSameLineFlag(false);
    //m_OptionsLayout.SetChoiceButtonWidth(flowcond_choice_button_width);
    //m_OptionsLayout.AddChoice(m_flowParmMedium, "Medium");
    //m_flowParmMedium.AddItem("Std. Atmo. Air");
    //m_flowParmMedium.AddItem("Water");
    //m_flowParmMedium.UpdateItems();
    //m_OptionsLayout.SetChoiceButtonWidth(choice_button_width);
    //m_OptionsLayout.SetFitWidthFlag(false);
    //m_OptionsLayout.SetSameLineFlag(true);

    // Re/L Input and Units
    m_OptionsLayout.SetButtonWidth( button_width );
    m_OptionsLayout.SetSliderWidth( slider_width );
    m_OptionsLayout.SetInputWidth( input_width );
    m_OptionsLayout.AddYGap();
    m_OptionsLayout.AddSlider( m_FlowParmReqLSlider, "Re/L", 100000.0, "%1.3e" );
    m_OptionsLayout.AddButton( m_ReyqLUnitLabel, "1/ft" );
    m_ReyqLUnitLabel.GetFlButton()->box( FL_THIN_UP_BOX );
    m_ReyqLUnitLabel.SetWidth( choice_list_width );
    m_ReyqLUnitLabel.GetFlButton()->labelcolor( FL_BLACK );
    m_OptionsLayout.ForceNewLine();

    // Mach Input
    m_OptionsLayout.AddSlider( m_FlowParmMachSlider, "Mach", 1.0, "%7.3f" );
    m_OptionsLayout.ForceNewLine();
    m_OptionsLayout.SetFitWidthFlag( true );
    m_OptionsLayout.SetSameLineFlag( false );
    m_OptionsLayout.AddYGap();

    //==== Simple Excrescence ====//
    Fl_Group* excrescence_group = AddSubGroup( excrescence_tab, 5 );
    m_ExcrescenceLayout.SetGroupAndScreen( excrescence_group, this );
    m_ExcrescenceLayout.AddSubGroupLayout( m_ExcrescenceListLayout, DRAG_TAB_WIDTH - 10, excrescence_browser_height + 5 * m_ExcrescenceLayout.GetStdHeight() );

    // Add GUI Elements to Simple Excrescence Layout
    m_ExcrescenceListLayout.SetFitWidthFlag( true );
    m_ExcrescenceListLayout.SetButtonWidth( TYPICAL_INPUT_WIDTH );
    m_ExcrescenceListLayout.SetChoiceButtonWidth( TYPICAL_INPUT_WIDTH );
    m_ExcrescenceListLayout.AddDividerBox( "Excrescence List" );

    // Initial column widths
    static int col_widths[] = { 115, 111, 60, 0 }; // 3 columns

    m_ExcresBrowser = m_ExcrescenceListLayout.AddColResizeBrowser( col_widths, 3, excrescence_browser_height );
    m_ExcresBrowser->callback( staticScreenCB, this );

    m_ExcrescenceListLayout.AddYGap();

    m_ExcrescenceListLayout.AddButton( m_excresDelete, "Delete Excrescence" );
    m_ExcrescenceListLayout.AddYGap();

    m_ExcrescenceListLayout.SetChoiceButtonWidth( TYPICAL_INPUT_WIDTH );
    m_ExcrescenceListLayout.AddChoice( m_excresTypeChoice, "Type", 0 );
    m_excresTypeChoice.AddItem( "Drag Counts" );
    m_excresTypeChoice.AddItem( "CD" );
    m_excresTypeChoice.AddItem( "% of CD_Geom" );
    m_excresTypeChoice.AddItem( "% Margin" );
    m_excresTypeChoice.AddItem( "Drag Area, D\\/q" );
    m_excresTypeChoice.UpdateItems();
    m_ExcrescenceListLayout.AddButton( m_excresAdd, " Add Excrescence " );

    m_ExcrescenceLayout.AddY( m_ExcrescenceListLayout.GetH() - m_ExcrescenceListLayout.GetGapHeight() );
    m_ExcrescenceLayout.AddSubGroupLayout( m_ExcrescenceItemLayout, DRAG_TAB_WIDTH - 10, 3 * m_ExcrescenceLayout.GetStdHeight() );
    m_ExcrescenceItemLayout.AddDividerBox( "Excrescence Parameters" );
    m_ExcrescenceItemLayout.AddInput( m_excresNameInput, "Name" );
    m_ExcrescenceItemLayout.AddSlider( m_excresInput, "Value", 1e6, "%7.3f" );
    m_ExcrescenceItemLayout.ForceNewLine();

    //==== Eqn Based Excrescence Layout ====//
    //// Add GUI Elements to Simple Excrescence Layout
    //// Scroll Group Set up to Display Full List of Eqn Based Excrescence

    // Set up Documentation Tab
    Fl_Group* documentation_group = AddSubGroup( documentation_tab, 5 );
    m_DocumentationLayout.SetGroupAndScreen( documentation_group, this );

    char str[256];
    m_TextDisplay = m_DocumentationLayout.AddFlTextDisplay( documentation_display_height );
    m_TextDisplay->color( 23 );
    m_TextDisplay->textcolor( 32 );
    m_TextDisplay->textfont( 4 );
    m_TextDisplay->textsize( 12 );
    m_TextBuffer = new Fl_Text_Buffer;
    m_TextBuffer->append( "-- Component Table Quantities -- \n" );
    m_TextBuffer->append( "S_wet \t\t : \t Wetted Area of Geometry, Calculated through DegenGeom \n" );
    m_TextBuffer->append( "L_ref \t\t : \t Reference Length of Geometry, Calculated through Length of Bounding Box in X direction or Chord on a Wing\n" );
    m_TextBuffer->append( "t/c or l/d \t : \t Thickness to Chord Ratio or Fineness Ratio, respectively \n" );
    m_TextBuffer->append( "FF Equation \t : \t Chosen Form Factor Equation for Internal Calculations \n" );
    m_TextBuffer->append( "FF \t\t : \t Form Factor of Specific Geometry \n" );
    m_TextBuffer->append( "Re \t\t : \t Reynolds Number for the component \n" );
    m_TextBuffer->append( "% Lam \t\t : \t % Laminar of Flow over Specific Geometry \n" );
    m_TextBuffer->append( "C_f \t\t : \t Friction Coefficient Calculated Using Specified Laminar and Turbulent Equation Choices \n" );
    m_TextBuffer->append( "Q \t\t : \t Interference Factor for Specific Geometry \n" );
    m_TextBuffer->append( "f \t\t : \t Flat Plat Drag \n" );
    m_TextBuffer->append( "C_D \t\t : \t Drag Coefficient for Specific Geometry \n" );
    m_TextBuffer->append( "% Total \t : \t Total Percentage of the Drag Contribution of Specific Geometry \n" );
    m_TextBuffer->append( "\n" );
    m_TextBuffer->append( "-- Excrescence Table Quantities -- \n" );
    m_TextBuffer->append( "Excres. Type \t : \t Chosen type of excrescence (Drag Counts, CD, % of C_D Geom, Drag Area ) \n" );
    m_TextBuffer->append( "Excres. Input \t : \t User Input Value for Excrescence of Described Type \n" );
    m_TextBuffer->append( "\n" );
    m_TextBuffer->append( "-- User Input Options -- \n" );
    m_TextBuffer->append( "Lam. Cf Eqn \t : \t Chosen Laminar Equation from Documented List at Link Below \n" );
    m_TextBuffer->append( "Turb. Cf Eqn \t : \t Chosen Turbulent Equation from Documented List at Link Below \n" );
    m_TextBuffer->append( "Spec. Method \t : \t Which Inputs Are to be Used to Determine Re/L and Mach \n" );
    sprintf( str, "Vinf \t\t : \t %s \n", ParasiteDragMgr.m_Vinf.GetDescript().c_str() );
    m_TextBuffer->append( str );
    sprintf( str, "Altitude \t : \t %s \n", ParasiteDragMgr.m_Hinf.GetDescript().c_str() );
    m_TextBuffer->append( str );
    sprintf( str, "Temperature \t : \t %s \n", ParasiteDragMgr.m_Temp.GetDescript().c_str() );
    m_TextBuffer->append( str );
    sprintf( str, "Delta Temp. \t : \t %s \n", ParasiteDragMgr.m_DeltaT.GetDescript().c_str() );
    m_TextBuffer->append( str );
    sprintf( str, "Pressure \t : \t %s \n", ParasiteDragMgr.m_Pres.GetDescript().c_str() );
    m_TextBuffer->append( str );
    sprintf( str, "Density \t : \t %s \n", ParasiteDragMgr.m_Rho.GetDescript().c_str() );
    m_TextBuffer->append( str );
    sprintf( str, "Dynamic Visc. \t : \t %s \n", ParasiteDragMgr.m_DynaVisc.GetDescript().c_str() );
    m_TextBuffer->append( str );
    //sprintf(str, "Medium \t\t : \t %s \n", ParasiteDragMgr.m_MediumType.GetDescript().c_str());
    //m_TextBuffer->append(str);
    sprintf( str, "Re/L \t\t : \t %s \n", ParasiteDragMgr.m_ReqL.GetDescript().c_str() );
    m_TextBuffer->append( str );
    sprintf( str, "Mach \t\t : \t %s \n", ParasiteDragMgr.m_Mach.GetDescript().c_str() );
    m_TextBuffer->append( str );
    m_TextBuffer->append( "\n" );
    m_TextBuffer->append( "-- Input Unit Types -- \n" );

    temp = "";
    for ( size_t i = 0; i < m_AltUnitChoice.GetItems().size(); i++ )
    {
        if ( i != m_AltUnitChoice.GetItems().size() - 1 )
        {
            temp.append( m_AltUnitChoice.GetItems()[i] + "," );
        }
        else
        {
            temp.append( m_AltUnitChoice.GetItems()[i] );
        }
    }
    sprintf( str, "Alt Units \t : \t %s \n", temp.c_str() );
    m_TextBuffer->append( str );

    temp.clear();
    for ( size_t i = 0; i < m_ModelLengthUnitChoice.GetItems().size(); i++ )
    {
        if ( i != m_ModelLengthUnitChoice.GetItems().size() - 1 )
        {
            temp.append( m_ModelLengthUnitChoice.GetItems()[i] + "," );
        }
        else
        {
            temp.append( m_ModelLengthUnitChoice.GetItems()[i] );
        }
    }
    sprintf( str, "Length Units \t : \t %s \n", temp.c_str() );
    m_TextBuffer->append( str );

    temp.clear();
    for ( size_t i = 0; i < m_VinfUnitChoice.GetItems().size(); i++ )
    {
        if ( i != m_VinfUnitChoice.GetItems().size() - 1 )
        {
            temp.append( m_VinfUnitChoice.GetItems()[i] + "," );
        }
        else
        {
            temp.append( m_VinfUnitChoice.GetItems()[i] );
        }
    }
    sprintf( str, "Vinf Units \t : \t %s \n", temp.c_str() );
    m_TextBuffer->append( str );

    m_TextDisplay->buffer( m_TextBuffer );
    m_DocumentationLayout.AddYGap();

    temp.clear();
    for ( size_t i = 0; i < m_TempUnitChoice.GetItems().size(); i++ )
    {
        if ( i != m_TempUnitChoice.GetItems().size() - 1 )
        {
            temp.append( m_TempUnitChoice.GetItems()[i] + "," );
        }
        else
        {
            temp.append( m_TempUnitChoice.GetItems()[i] );
        }
    }
    sprintf( str, "Temp Units \t : \t %s \n", temp.c_str() );
    m_TextBuffer->append( str );

    temp.clear();
    for ( size_t i = 0; i < m_PresUnitChoice.GetItems().size(); i++ )
    {
        if ( i != m_PresUnitChoice.GetItems().size() - 1 )
        {
            temp.append( m_PresUnitChoice.GetItems()[i] + "," );
        }
        else
        {
            temp.append( m_PresUnitChoice.GetItems()[i] );
        }
    }
    sprintf( str, "Pres Units \t : \t %s \n", temp.c_str() );
    m_TextBuffer->append( str );
    m_TextBuffer->append( "\n" );

    m_TextBuffer->append( "-- Component Labels Legend --\n" );
    m_TextBuffer->append( "(+) \t : \t Indicates there are additional components contributing to wetted area\n" );
    m_TextBuffer->append( "v \t : \t Indicates the geometry list has been expanded to show all surfaces associated with this geometry\n" );
    m_TextBuffer->append( "[ss] \t : \t Indicates a subsurface on the parented geometry\n" );
    m_TextBuffer->append( "[B] \t : \t Indicates the body components of a custom geometry\n" );
    m_TextBuffer->append( "[W] \t : \t Indicates the wing components of a custom geometry\n" );
    m_TextBuffer->append( "\n" );

    m_TextDisplay->buffer( m_TextBuffer );

    m_DocumentationLayout.AddYGap();

    m_DocumentationLayout.AddButton( m_EqnDocumentation, "Link to Complete Wiki Parasite Drag Documentation" );
}

ParasiteDragScreen::~ParasiteDragScreen()
{
    m_TextDisplay->buffer( NULL );
    delete m_TextBuffer;
}

bool ParasiteDragScreen::Update()
{
    Vehicle *veh = VehicleMgr.GetVehicle();
    if ( veh )
    {
        UpdateSetChoice();

        UpdateSrefChoice();

        ParasiteDragMgr.Update();     // Update Atmos, Excres & Activate Correct GUI Elements

        RebuildBuildUpTable();
        UpdateIncorporateDropDowns();

        UpdateExcresTab();

        // Update GUI Devices
        UpdateDependentUnitLabels();
        UpdateChoiceDevices();     // Set Correct Options for Options
        UpdateSliderDevices();

        m_SubCompExportToggle.Update( ParasiteDragMgr.m_ExportSubCompFlag.GetID() );
    }

    return false;
}

void ParasiteDragScreen::UpdateTableLabels()
{
    string squared( 1, (char) 178 );
    string temp;

    // Length
    switch ( ParasiteDragMgr.m_LengthUnit() )
    {
    case vsp::LEN_MM:
        temp = "S_wet (mm" + squared + ")";
        m_SwetUnitLabel.GetFlButton()->copy_label( temp.c_str() );
        m_LrefUnitLabel.GetFlButton()->copy_label( "L_ref (mm)" );
        temp = "f (mm" + squared + ")";
        m_fLabel.GetFlButton()->copy_label( temp.c_str() );
        m_fTotalLabel.GetFlButton()->copy_label( temp.c_str() );
        break;

    case vsp::LEN_CM:
        temp = "S_wet (cm" + squared + ")";
        m_SwetUnitLabel.GetFlButton()->copy_label( temp.c_str() );
        m_LrefUnitLabel.GetFlButton()->copy_label( "L_ref (cm)" );
        temp = "f (cm" + squared + ")";
        m_fLabel.GetFlButton()->copy_label( temp.c_str() );
        m_fTotalLabel.GetFlButton()->copy_label( temp.c_str() );
        break;

    case vsp::LEN_M:
        temp = "S_wet (m" + squared + ")";
        m_SwetUnitLabel.GetFlButton()->copy_label( temp.c_str() );
        m_LrefUnitLabel.GetFlButton()->copy_label( "L_ref (m)" );
        temp = "f (m" + squared + ")";
        m_fLabel.GetFlButton()->copy_label( temp.c_str() );
        m_fTotalLabel.GetFlButton()->copy_label( temp.c_str() );
        break;

    case vsp::LEN_IN:
        temp = "S_wet (in" + squared + ")";
        m_SwetUnitLabel.GetFlButton()->copy_label( temp.c_str() );
        m_LrefUnitLabel.GetFlButton()->copy_label( "L_ref (in)" );
        temp = "f (in" + squared + ")";
        m_fLabel.GetFlButton()->copy_label( temp.c_str() );
        m_fTotalLabel.GetFlButton()->copy_label( temp.c_str() );
        break;

    case vsp::LEN_FT:
        temp = "S_wet (ft" + squared + ")";
        m_SwetUnitLabel.GetFlButton()->copy_label( temp.c_str() );
        m_LrefUnitLabel.GetFlButton()->copy_label( "L_ref (ft)" );
        temp = "f (ft" + squared + ")";
        m_fLabel.GetFlButton()->copy_label( temp.c_str() );
        m_fTotalLabel.GetFlButton()->copy_label( temp.c_str() );
        break;

    case vsp::LEN_YD:
        temp = "S_wet (yd" + squared + ")";
        m_SwetUnitLabel.GetFlButton()->copy_label( temp.c_str() );
        m_LrefUnitLabel.GetFlButton()->copy_label( "L_ref (yd)" );
        temp = "f (yd" + squared + ")";
        m_fLabel.GetFlButton()->copy_label( temp.c_str() );
        m_fTotalLabel.GetFlButton()->copy_label( temp.c_str() );
        break;

    case vsp::LEN_UNITLESS:
        temp = "S_wet (LU" + squared + ")";
        m_SwetUnitLabel.GetFlButton()->copy_label( temp.c_str() );
        m_LrefUnitLabel.GetFlButton()->copy_label( "L_ref (LU)" );
        temp = "f (LU" + squared + ")";
        m_fLabel.GetFlButton()->copy_label( temp.c_str() );
        m_fTotalLabel.GetFlButton()->copy_label( temp.c_str() );
        break;
    }

    ParasiteDragMgr.UpdateExportLabels();
}

void ParasiteDragScreen::UpdateSetChoice()
{
    // Clear All Items
    m_SetChoice.ClearItems();

    // Adding Items to Set Choice
    vector <string> setnames = VehicleMgr.GetVehicle()->GetSetNameVec();
    for ( int i = 0; i < setnames.size(); i++ )
    {
        m_SetChoice.AddItem( setnames[i] );
    }
    m_SetChoice.UpdateItems();
    m_SetChoice.SetVal( ParasiteDragMgr.m_SetChoice() );

    ParasiteDragMgr.RefreshBaseDataVectors();
}

void ParasiteDragScreen::UpdateChoiceDevices()
{
    m_SortToggle.Update( ParasiteDragMgr.m_SortByFlag.GetID() );
    m_RefToggle.Update( ParasiteDragMgr.m_RefFlag.GetID() );
    m_LamCfEqnChoice.SetVal( ParasiteDragMgr.m_LamCfEqnType() );

    ParasiteDragMgr.CorrectTurbEquation();

    int indx = vector_find_val( m_TurbChoiceToEnum, ParasiteDragMgr.m_TurbCfEqnType() );
    m_TurbCfEqnChoice.SetVal( indx );

    m_FreestreamTypeChoice.SetVal( ParasiteDragMgr.m_FreestreamType() );
    m_ModelLengthUnitChoice.SetVal( ParasiteDragMgr.m_LengthUnit() );
    m_AltUnitChoice.SetVal( ParasiteDragMgr.m_AltLengthUnit() );
    m_VinfUnitChoice.SetVal( ParasiteDragMgr.m_VinfUnitType() );
    m_TempUnitChoice.SetVal( ParasiteDragMgr.m_TempUnit() );
    m_PresUnitChoice.SetVal( ParasiteDragMgr.m_PresUnit() );
}

void ParasiteDragScreen::UpdateSliderDevices()
{
    m_SrefSlider.Update( ParasiteDragMgr.m_Sref.GetID() );
    m_FlowParmVinfSlider.Update( ParasiteDragMgr.m_Vinf.GetID() );
    m_FlowParmHinfSlider.Update( ParasiteDragMgr.m_Hinf.GetID() );
    m_FlowParmDeltaTempSlider.Update( ParasiteDragMgr.m_DeltaT.GetID() );
    m_FlowParmTempSlider.Update( ParasiteDragMgr.m_Temp.GetID() );
    m_FlowParmPresSlider.Update( ParasiteDragMgr.m_Pres.GetID() );
    m_FlowParmRhoSlider.Update( ParasiteDragMgr.m_Rho.GetID() );
    m_FlowParmSpecificHeatRatioSlider.Update( ParasiteDragMgr.m_SpecificHeatRatio.GetID() );
    m_FlowParmDynaViscSlider.Update( ParasiteDragMgr.m_DynaVisc.GetID() );
    //m_flowParmMedium.Update(ParasiteDragMgr.m_MediumType.GetID());
    m_FlowParmReqLSlider.Update( ParasiteDragMgr.m_ReqL.GetID() );
    m_FlowParmMachSlider.Update( ParasiteDragMgr.m_Mach.GetID() );

    UpdateAltitudeSliderLimits();
}

void ParasiteDragScreen::UpdateSrefChoice()
{
    Vehicle* veh = VehicleMgr.GetVehicle();

    // Adding Items to Rerefence Wing Choice Setting Val if Necessary
    m_RefWingChoice.ClearItems();
    m_WingGeomVec.clear();
    map <string, int> WingCompIDMap;
    int iwing = 0;

    // Find Correct Geoms from Active Set
    vector <string> g_IDs = veh->GetGeomSet( ParasiteDragMgr.m_SetChoice() );

    if ( !ParasiteDragMgr.m_RefFlag() )
    {
        m_RefWingChoice.Deactivate();
    }
    else
    {
        m_RefWingChoice.Activate();
        for ( int i = 0; i < ( int )g_IDs.size(); i++ )
        {
            char str[256];
            Geom* geom = veh->FindGeom( g_IDs[i] );
            if ( geom )
            {
                sprintf( str, "%d_%s", i, geom->GetName().c_str() );

                if ( geom->GetType().m_Type == MS_WING_GEOM_TYPE )
                {
                    m_RefWingChoice.AddItem( str );
                    WingCompIDMap[g_IDs[i]] = iwing;
                    m_WingGeomVec.push_back( g_IDs[i] );
                    iwing++;
                }
            }
        }
        m_RefWingChoice.UpdateItems();
    }

    // Setting the Wing to Pull Reference Area from
    string refGeomID = ParasiteDragMgr.m_RefGeomID;
    if ( refGeomID.length() == 0 && m_WingGeomVec.size() > 0 )
    {
        // Handle case default case.
        refGeomID = m_WingGeomVec[0];
        ParasiteDragMgr.m_RefGeomID = refGeomID;
    }
    m_RefWingChoice.SetVal( WingCompIDMap[refGeomID] );
}

void ParasiteDragScreen::UpdateExcrescenceSliderLimits()
{
    if ( ParasiteDragMgr.GetCurrentExcresType() == vsp::EXCRESCENCE_CD )
    {
        ParasiteDragMgr.m_ExcresValue.SetLowerUpperLimits( 0.0, 0.2 );
        m_excresInput.SetMinBound( 0.0 );
        m_excresInput.SetMaxBound( 0.2 );
        m_excresInput.SetFormat( "%5.5f" );
    }
    else if ( ParasiteDragMgr.GetCurrentExcresType() == vsp::EXCRESCENCE_COUNT )
    {
        ParasiteDragMgr.m_ExcresValue.SetLowerUpperLimits( 0.0, 2000.0 );
        m_excresInput.SetMinBound( 0.0 );
        m_excresInput.SetMaxBound( 2000.0 );
        m_excresInput.SetFormat( "%5.1f" );
    }
    else if ( ParasiteDragMgr.GetCurrentExcresType() == vsp::EXCRESCENCE_PERCENT_GEOM )
    {
        ParasiteDragMgr.m_ExcresValue.SetLowerUpperLimits( 0.0, 100.0 );
        m_excresInput.SetMinBound( 0.0 );
        m_excresInput.SetMaxBound( 100.0 );
        m_excresInput.SetFormat( "%5.1f" );
    }
    else if ( ParasiteDragMgr.GetCurrentExcresType() == vsp::EXCRESCENCE_MARGIN )
    {
        ParasiteDragMgr.m_ExcresValue.SetLowerUpperLimits( 0.0, 100.0 );
        m_excresInput.SetMinBound( 0.0 );
        m_excresInput.SetMaxBound( 100.0 );
        m_excresInput.SetFormat( "%5.1f" );
    }
    else if ( ParasiteDragMgr.GetCurrentExcresType() == vsp::EXCRESCENCE_DRAGAREA )
    {
        ParasiteDragMgr.m_ExcresValue.SetLowerUpperLimits( 0.0, 10.0 );
        m_excresInput.SetMinBound( 0.0 );
        m_excresInput.SetMaxBound( 10.0 );
        m_excresInput.SetFormat( "%5.3f" );
    }
}

void ParasiteDragScreen::UpdateAltitudeSliderLimits()
{
    if ( ParasiteDragMgr.m_AltLengthUnit() == vsp::PD_UNITS_IMPERIAL )
    {
        ParasiteDragMgr.m_Hinf.SetLowerUpperLimits( 0.0, 913339.33 );
        m_FlowParmHinfSlider.SetFormat( "%7.2f" );
    }
    else if ( ParasiteDragMgr.m_AltLengthUnit() == vsp::PD_UNITS_METRIC )
    {
        ParasiteDragMgr.m_Hinf.SetLowerUpperLimits( 0.0, 278385.83 );
        m_FlowParmHinfSlider.SetFormat( "%7.2f" );
    }
}

void ParasiteDragScreen::UpdateExcresTab()
{
    // Grab Current Excres Name and Value
    m_excresNameInput.Update( ParasiteDragMgr.GetCurrentExcresLabel() );
    m_excresInput.Update( ParasiteDragMgr.m_ExcresValue.GetID() );

    // Update Excres Browser
    char str[256];
    int h_pos = m_ExcresBrowser->hposition();
    int v_pos = m_ExcresBrowser->position();
    m_ExcresBrowser->clear();
    m_ExcresBrowser->column_char( ':' );

    sprintf( str, "@b@.NAME:@b@.TYPE:@b@.VALUE" );
    m_ExcresBrowser->add( str );

    string excres_name, excres_type, formatString;
    double excres_val;

    vector < ExcrescenceTableRow > excresVec = ParasiteDragMgr.GetExcresVec();
    for ( size_t i = 0; i < excresVec.size(); ++i )
    {
        excres_name = excresVec[i].Label;
        excres_type = excresVec[i].TypeString;
        excres_val = excresVec[i].Input;

        if ( excresVec[i].Type == vsp::EXCRESCENCE_CD )
        {
            formatString = "%s:%s:%5.5f";
        }
        else if ( excresVec[i].Type == vsp::EXCRESCENCE_COUNT )
        {
            formatString = "%s:%s:%5.1f";
        }
        else if ( excresVec[i].Type == vsp::EXCRESCENCE_PERCENT_GEOM )
        {
            formatString = "%s:%s:%5.1f";
        }
        else if ( excresVec[i].Type == vsp::EXCRESCENCE_MARGIN )
        {
            formatString = "%s:%s:%5.1f";
        }
        else if ( excresVec[i].Type == vsp::EXCRESCENCE_DRAGAREA )
        {
            formatString = "%s:%s:%5.3f";
        }

        sprintf( str, formatString.c_str(), excres_name.c_str(), excres_type.c_str(), excres_val );
        m_ExcresBrowser->add( str );
    }

    m_ExcresBrowser->hposition( h_pos );
    m_ExcresBrowser->position( v_pos );

    for ( size_t i = 0; i < excresVec.size(); ++i )
    {
        if ( excresVec[i].Type == vsp::EXCRESCENCE_MARGIN )
        {
            m_excresTypeChoice.SetFlag( vsp::EXCRESCENCE_MARGIN, FL_MENU_INACTIVE );
            break;
        }
        m_excresTypeChoice.SetFlag( vsp::EXCRESCENCE_MARGIN, 0 );
    }

    if ( excresVec.size() == 0 )
    {
        m_excresTypeChoice.SetFlag( vsp::EXCRESCENCE_MARGIN, 0 );
    }

    m_excresTypeChoice.UpdateItems();
    m_excresTypeChoice.SetVal( ParasiteDragMgr.m_ExcresType() );

    // Select Current Excres
    if ( ParasiteDragMgr.GetCurrExcresIndex() != -1 )
    {
        SelectExcresBrowser( ParasiteDragMgr.GetCurrExcresIndex() + 2 );
        m_ExcrescenceItemLayout.Show();
    }
    else
    {
        m_ExcrescenceItemLayout.Hide();
    }
}

void ParasiteDragScreen::UpdateDependentUnitLabels()
{
    string deg( 1, (char) 176 );
    string squared( 1, (char) 178 );
    string cubed( 1, (char) 179 );
    string temp;

    switch ( ParasiteDragMgr.m_LengthUnit() )
    {
    case vsp::LEN_MM:
        temp = "mm" + squared;
        m_SrefUnitLabel.GetFlButton()->copy_label( temp.c_str() );
        m_ReyqLUnitLabel.GetFlButton()->copy_label( "1/mm" );
        break;

    case vsp::LEN_CM:
        temp = "cm" + squared;
        m_SrefUnitLabel.GetFlButton()->copy_label( temp.c_str() );
        m_ReyqLUnitLabel.GetFlButton()->copy_label( "1/cm" );
        break;

    case vsp::LEN_M:
        temp = "m" + squared;
        m_SrefUnitLabel.GetFlButton()->copy_label( temp.c_str() );
        m_ReyqLUnitLabel.GetFlButton()->copy_label( "1/m" );
        break;

    case vsp::LEN_IN:
        temp = "in" + squared;
        m_SrefUnitLabel.GetFlButton()->copy_label( temp.c_str() );
        m_ReyqLUnitLabel.GetFlButton()->copy_label( "1/in" );
        break;

    case vsp::LEN_FT:
        temp = "ft" + squared;
        m_SrefUnitLabel.GetFlButton()->copy_label( temp.c_str() );
        m_ReyqLUnitLabel.GetFlButton()->copy_label( "1/ft" );
        break;

    case vsp::LEN_YD:
        temp = "yd" + squared;
        m_SrefUnitLabel.GetFlButton()->copy_label( temp.c_str() );
        m_ReyqLUnitLabel.GetFlButton()->copy_label( "1/yd" );
        break;

    case vsp::LEN_UNITLESS:
        temp = "Unitless" + squared;
        m_SrefUnitLabel.GetFlButton()->copy_label( temp.c_str() );
        m_ReyqLUnitLabel.GetFlButton()->copy_label( "1/Unitless" );
        break;
    }

    switch ( ParasiteDragMgr.m_AltLengthUnit() )
    {
    case vsp::PD_UNITS_IMPERIAL:
        temp = "slug/ft" + cubed;
        m_RhoUnitLabel.GetFlButton()->copy_label( temp.c_str() );
        m_DynViscUnitLabel.GetFlButton()->copy_label( "slug/ft-s" );
        break;

    case vsp::PD_UNITS_METRIC:
        temp = "kg/m" + cubed;
        m_RhoUnitLabel.GetFlButton()->copy_label( temp.c_str() );
        m_DynViscUnitLabel.GetFlButton()->copy_label( "kg/m-s" );
        break;
    }

    switch ( ParasiteDragMgr.m_TempUnit() )
    {
    case vsp::TEMP_UNIT_K:
        temp = "K";
        break;

    case vsp::TEMP_UNIT_C:
        temp = deg + "C";
        break;

    case vsp::TEMP_UNIT_F:
        temp = deg + "F";
        break;

    case vsp::TEMP_UNIT_R:
        temp = deg + "R";
        break;
    }

    m_DeltaTempUnitLabel.GetFlButton()->copy_label( temp.c_str() );
}

void ParasiteDragScreen::Show()
{
    m_ScreenMgr->SetUpdateFlag( true );
    m_FLTK_Window->show();
}

void ParasiteDragScreen::Hide()
{
    m_FLTK_Window->hide();
    m_ScreenMgr->SetUpdateFlag( true );
}

void ParasiteDragScreen::CallBack( Fl_Widget* w )
{
    assert( m_ScreenMgr );

    if ( w == m_ExcresBrowser )
    {
        ExresBrowserCallback();

        if ( ParasiteDragMgr.GetCurrExcresIndex() >= 0 )
        {
            UpdateExcrescenceSliderLimits();
            ParasiteDragMgr.UpdateCurrentExcresVal();
        }
    }
    else if ( w == &m_MainTableCompNamesScrollGroup->scrollbar )
    {
        m_MainTableCompNamesScrollGroup->scrollbar.callback( m_LabelScrollbarCB );
        m_MainTableCompNamesScrollGroup->scrollbar.do_callback();

        m_MainTableScrollGroup->scroll_to( main_XScrollPosition, m_MainTableCompNamesScrollGroup->yposition() );
        m_ConstantTableScrollGroup->scroll_to( 0, m_MainTableCompNamesScrollGroup->yposition() );

        m_MainTableCompNamesScrollGroup->scrollbar.callback( staticScreenCB, this );
        return;
    }
    else if ( w == &m_MainTableScrollGroup->scrollbar )
    {
        m_MainTableScrollGroup->scrollbar.callback( m_MainScrollbarCB );
        m_MainTableScrollGroup->scrollbar.do_callback();

        m_ConstantTableScrollGroup->scroll_to( 0, m_MainTableScrollGroup->yposition() );
        m_MainTableCompNamesScrollGroup->scroll_to( 0, m_MainTableScrollGroup->yposition() );

        m_MainTableScrollGroup->scrollbar.callback( staticScreenCB, this );
        return;
    }
    else if ( w == &m_ConstantTableScrollGroup->scrollbar )
    {
        m_ConstantTableScrollGroup->scrollbar.callback( m_ConstantScrollbarCB );
        m_ConstantTableScrollGroup->scrollbar.do_callback();

        m_MainTableScrollGroup->scroll_to( main_XScrollPosition, m_ConstantTableScrollGroup->yposition() );
        m_MainTableCompNamesScrollGroup->scroll_to( 0, m_ConstantTableScrollGroup->yposition() );

        m_ConstantTableScrollGroup->scrollbar.callback( staticScreenCB, this );
        return;
    }
    else if ( w == &m_MainTableScrollGroup->hscrollbar )
    {
        m_MainTableScrollGroup->hscrollbar.callback( m_HScrollbarCB );
        m_MainTableScrollGroup->hscrollbar.do_callback();

        m_MainTableLabelsScrollGroup->scroll_to( m_MainTableScrollGroup->xposition(), 0 );

        m_MainTableLabelsScrollGroup->hscrollbar.hide();
        m_MainTableScrollGroup->scrollbar.hide();

        m_MainTableScrollGroup->hscrollbar.callback( staticScreenCB, this );
        return;
    }

    m_ScreenMgr->SetUpdateFlag( true );
}

void ParasiteDragScreen::GuiDeviceCallBack( GuiDevice* device )
{
    Vehicle *vptr = VehicleMgr.GetVehicle();

    assert( m_ScreenMgr );

    //==== Overview Tab GUI Callback ====//
    if ( device == &m_RefWingChoice )
    {
        // Reference Wing Choice
        int id = m_RefWingChoice.GetVal();
        ParasiteDragMgr.m_RefGeomID = m_WingGeomVec[id];
    }
    else if ( device == &m_SetChoice )
    {
        // Set Choice
        ParasiteDragMgr.m_SetChoice.Set( m_SetChoice.GetVal() );
    }
    else if ( device == &m_LamCfEqnChoice )
    {
        ParasiteDragMgr.m_LamCfEqnType.Set( m_LamCfEqnChoice.GetVal() );
    }
    else if ( device == &m_TurbCfEqnChoice )
    {
        ParasiteDragMgr.m_TurbCfEqnType.Set( m_TurbChoiceToEnum[ m_TurbCfEqnChoice.GetVal() ] );
    }
    else if ( device == &m_EqnDocumentation )
    {
#if     defined(__APPLE__)
        system( "open http://www.openvsp.org/wiki/doku.php?id=parasitedrag" );
#elif   defined(_WIN32) || defined(WIN32) 
        ShellExecute( NULL, "open", "http://www.openvsp.org/wiki/doku.php?id=parasitedrag",
                      NULL, NULL, SW_SHOWNORMAL );
#else
        system( "xdg-open http://www.openvsp.org/wiki/doku.php?id=parasitedrag" );
#endif
    }
    else if ( device == &m_FreestreamTypeChoice )
    {
        ParasiteDragMgr.m_FreestreamType.Set( m_FreestreamTypeChoice.GetVal() );
    }
    else if ( device == &m_ModelLengthUnitChoice )
    {
        ParasiteDragMgr.m_LengthUnit.Set( m_ModelLengthUnitChoice.GetVal() );
    }
    else if ( device == &m_VinfUnitChoice )
    {
        ParasiteDragMgr.UpdateVinf( m_VinfUnitChoice.GetVal() );
    }
    else if ( device == &m_AltUnitChoice )
    {
        ParasiteDragMgr.UpdateAlt( m_AltUnitChoice.GetVal() );
    }
    else if ( device == &m_PresUnitChoice )
    {
        ParasiteDragMgr.UpdatePres( m_PresUnitChoice.GetVal() );
    }
    else if ( device == &m_TempUnitChoice )
    {
        ParasiteDragMgr.UpdateTemp( m_TempUnitChoice.GetVal() );
    }

    //==== Main Output Table GUI Callback ====//
    else if ( device == &m_calc )
    {
        // Calculate DegenGeom and Save User Parms Before Running Calculate_ALL
        ParasiteDragMgr.SetupFullCalculation();
    }
    else if ( device == &m_export )
    {
        // Export to CSV
        if ( ParasiteDragMgr.GetMainTableVec().size() > 0 )
        {
            vptr->setExportFileName( vsp::DRAG_BUILD_CSV_TYPE,
                                     m_ScreenMgr->GetSelectFileScreen()->FileChooser(
                                         "Select ParasiteBuildUp output file.", "*.csv" ) );

            ParasiteDragMgr.ExportToCSV( vptr->getExportFileName( vsp::DRAG_BUILD_CSV_TYPE ) );
        }
    }

    //==== Excrescence Tab GUI Callback ====//
    else if ( device == &m_excresAdd )
    {
        // Add Excrescence to Table
        ParasiteDragMgr.AddExcrescence();
        // Add Excres Under With Default Name ( e.g. EXCRES_0 )
        UpdateExcrescenceSliderLimits();
    }
    else if ( device == &m_excresDelete )
    {
        // Delete Excrescence from Table
        ParasiteDragMgr.DeleteExcrescence();
        UpdateExcrescenceSliderLimits();
    }
    else if ( device == &m_excresNameInput )
    {
        ParasiteDragMgr.SetExcresLabel( m_excresNameInput.GetString() );
    }
    else if ( device == &m_excresTypeChoice )
    {
        // Selecting Excres Type for New Excres
        ParasiteDragMgr.m_ExcresType.Set( m_excresTypeChoice.GetVal() );
    }

    // Clause for Updating Table Values on FF Choice Switch ( Must go last )
    else if ( !m_ffType.empty() )
    {
        // Setting FF Eqn Choice for Selected Row
        for ( int i = 0; i < m_ffType.size(); i++ )
        {
            if ( device->GetType() == GDEV_CHOICE || device->GetType() == GDEV_TRIGGER_BUTTON ||
                    device->GetType() == GDEV_STRING_INPUT )
            {
                break;
            }
            else
            {
                return;
            }
        }

        vector < ParasiteDragTableRow > rowVec = ParasiteDragMgr.GetMainTableVec();
        for ( size_t i = 0; i < rowVec.size(); ++i )
        {
            if ( device == &m_ffType[i] )
            {
                if ( rowVec[i].SubSurfID.compare( "" ) == 0 )
                {
                    if ( rowVec[i].GeomShapeType == vsp::NORMAL_SURF )
                    {
                        vptr->FindGeom( rowVec[i].GeomID )->m_FFBodyEqnType.Set( m_ffType[i].GetVal() );
                    }
                    else if ( rowVec[i].GeomShapeType == vsp::WING_SURF )
                    {
                        vptr->FindGeom( rowVec[i].GeomID )->m_FFWingEqnType.Set( m_ffType[i].GetVal() );
                    }
                }
                else
                {
                    if ( rowVec[i].GeomShapeType == vsp::NORMAL_SURF )
                    {
                        vptr->FindGeom( rowVec[i].GeomID )->GetSubSurf( rowVec[i].SubSurfID )->m_FFBodyEqnType.Set( m_ffType[i].GetVal() );
                    }
                    else if ( rowVec[i].GeomShapeType == vsp::WING_SURF )
                    {
                        vptr->FindGeom( rowVec[i].GeomID )->GetSubSurf( rowVec[i].SubSurfID )->m_FFWingEqnType.Set( m_ffType[i].GetVal() );
                    }
                }
            }
            if ( device == &m_grouped[i] )
            {
                vptr->FindGeom( rowVec[i].GeomID )->m_GroupedAncestorGen.Set( m_grouped[i].GetVal() ); // Zero Value is SELF
            }
            if ( device == &m_labels[i] )
            {
                bool expandflag = vptr->FindGeom( rowVec[i].GeomID )->m_ExpandedListFlag();
                vptr->FindGeom( rowVec[i].GeomID )->m_ExpandedListFlag.Set( !expandflag );
            }
            if ( device == &m_subsurfinclude[i] )
            {
                if ( m_subsurfinclude[i].GetVal() == vsp::SS_INC_SEPARATE_TREATMENT && ParasiteDragMgr.IsCaclualted() )
                {
                    vptr->FindGeom( rowVec[i].GeomID )->GetSubSurf( rowVec[i].SubSurfID )->m_IncludeType.Set( m_subsurfinclude[i].GetVal() );
                    UpdateSetChoice();
                    RebuildBuildUpTable();
                    ParasiteDragMgr.RenewDegenGeomVec();
                }

                if ( vptr->FindGeom( rowVec[i].GeomID )->GetSubSurf( rowVec[i].SubSurfID )->m_IncludeType() == vsp::SS_INC_SEPARATE_TREATMENT &&
                        ParasiteDragMgr.IsCaclualted() )
                {
                    vptr->FindGeom( rowVec[i].GeomID )->GetSubSurf( rowVec[i].SubSurfID )->m_IncludeType.Set( m_subsurfinclude[i].GetVal() );
                    UpdateSetChoice();
                    RebuildBuildUpTable();
                    ParasiteDragMgr.RenewDegenGeomVec();
                }
                else
                {
                    vptr->FindGeom(rowVec[i].GeomID)->GetSubSurf(rowVec[i].SubSurfID)->m_IncludeType.Set(m_subsurfinclude[i].GetVal());
                }
            }
            if ( device == &m_ffIn[i] )
            {
                if ( rowVec[i].SubSurfID.compare( "" ) == 0 )
                {
                    try
                    {
                        vptr->FindGeom( rowVec[i].GeomID )->m_FFUser.Set( stod( m_ffIn[i].GetString() ) );
                    }
                    catch ( const std::exception& )
                    {
                    }
                }
                else
                {
                    try
                    {
                        vptr->FindGeom( rowVec[i].GeomID )->GetSubSurf( rowVec[i].SubSurfID )->m_FFUser.Set( stod( m_ffIn[i].GetString() ) );
                    }
                    catch ( const std::exception& )
                    {
                    }
                }
            }
            if ( device == &m_percLam[i] )
            {
                if ( rowVec[i].SubSurfID.compare( "" ) == 0 )
                {
                    try
                    {
                        vptr->FindGeom( rowVec[i].GeomID )->m_PercLam.Set( stod( m_percLam[i].GetString() ) );
                    }
                    catch ( const std::exception& )
                    {
                    }
                }
                else
                {
                    try
                    {
                        vptr->FindGeom( rowVec[i].GeomID )->GetSubSurf( rowVec[i].SubSurfID )->m_PercLam.Set( stod( m_percLam[i].GetString() ) );
                    }
                    catch ( const std::exception& )
                    {
                    }
                }
            }
            if ( device == &m_Q[i] )
            {
                if ( rowVec[i].SubSurfID.compare( "" ) == 0 )
                {
                    try
                    {
                        vptr->FindGeom( rowVec[i].GeomID )->m_Q.Set( stod( m_Q[i].GetString() ) );
                    }
                    catch ( const std::exception& )
                    {
                    }
                }
                else
                {
                    try
                    {
                        vptr->FindGeom( rowVec[i].GeomID )->GetSubSurf( rowVec[i].SubSurfID )->m_Q.Set( stod( m_Q[i].GetString() ) );
                    }
                    catch ( const std::exception& )
                    {
                    }
                }
            }
            if ( device == &m_TawTwRatio[i] )
            {
                if ( rowVec[i].SubSurfID.compare( "" ) == 0 )
                {
                    try
                    {
                        vptr->FindGeom( rowVec[i].GeomID )->m_TawTwRatio.Set( stod( m_TawTwRatio[i].GetString() ) );
                    }
                    catch ( const std::exception& )
                    {
                    }
                }
                else
                {
                    try
                    {
                        vptr->FindGeom( rowVec[i].GeomID )->GetSubSurf( rowVec[i].SubSurfID )->m_TawTwRatio.Set( stod( m_TawTwRatio[i].GetString() ) );
                    }
                    catch ( const std::exception& )
                    {
                    }
                }
            }
            if ( device == &m_TeTwRatio[i] )
            {
                if ( rowVec[i].SubSurfID.compare( "" ) == 0 )
                {
                    try
                    {
                        vptr->FindGeom( rowVec[i].GeomID )->m_TeTwRatio.Set( stod( m_TeTwRatio[i].GetString() ) );
                    }
                    catch ( const std::exception& )
                    {
                    }
                }
                else
                {
                    try
                    {
                        vptr->FindGeom( rowVec[i].GeomID )->GetSubSurf( rowVec[i].SubSurfID )->m_TeTwRatio.Set( stod( m_TeTwRatio[i].GetString() ) );
                    }
                    catch ( const std::exception& )
                    {
                    }
                }
            }
            if ( device == &m_Roughness[i] )
            {
                if ( rowVec[i].SubSurfID.compare( "" ) == 0 )
                {
                    try
                    {
                        vptr->FindGeom( rowVec[i].GeomID )->m_Roughness.Set( stod( m_Roughness[i].GetString() ) );
                    }
                    catch ( const std::exception& )
                    {
                    }
                }
                else
                {
                    try
                    {
                        vptr->FindGeom( rowVec[i].GeomID )->GetSubSurf( rowVec[i].SubSurfID )->m_Roughness.Set( stod( m_Roughness[i].GetString() ) );
                    }
                    catch ( const std::exception& )
                    {
                    }
                }
            }
        }
    }
    else
    {
        return;
    }

    assert( m_ScreenMgr );
    m_ScreenMgr->SetUpdateFlag( true );
}

void ParasiteDragScreen::RebuildBuildUpTableLabels()
{
    // Everything Relevant to Table Section
    m_MainTableLabelsLayout.SetFitWidthFlag( false );
    m_MainTableLabelsLayout.SetSameLineFlag( true );
    m_MainTableLabelsLayout.SetButtonWidth( TYPICAL_INPUT_WIDTH );

    m_ConstantTableLabelsLayout.SetFitWidthFlag( false );
    m_ConstantTableLabelsLayout.SetSameLineFlag( true );
    m_ConstantTableLabelsLayout.SetButtonWidth( TYPICAL_INPUT_WIDTH );

    m_ComponentLabelLayout.SetButtonWidth( TYPICAL_INPUT_WIDTH * 2 + 20 );
    m_ComponentLabelLayout.AddButton( m_CompLabel, "Component" );
    m_CompLabel.GetFlButton()->labelcolor( FL_BLACK );
    m_CompLabel.GetFlButton()->copy_tooltip( "Geometry Name" );

    //  Changing Units - therefore isButton
    m_MainTableLabelsLayout.SetButtonWidth( TYPICAL_INPUT_WIDTH + 20 );
    m_MainTableLabelsLayout.AddButton( m_SwetUnitLabel, "S_wet (ft2)" );
    m_SwetUnitLabel.GetFlButton()->labelcolor( FL_BLACK );
    m_SwetUnitLabel.GetFlButton()->copy_tooltip( "Wetted Area of Geometry" );

    m_MainTableLabelsLayout.SetButtonWidth( TYPICAL_INPUT_WIDTH );
    m_MainTableLabelsLayout.AddButton( m_GroupedGeomLabel, "Group" );
    m_GroupedGeomLabel.GetFlButton()->box( FL_THIN_UP_BOX );
    m_GroupedGeomLabel.GetFlButton()->labelcolor( FL_BLACK );
    m_GroupedGeomLabel.GetFlButton()->copy_tooltip( "Option to blend individual component with ancestor" );

    m_MainTableLabelsLayout.SetButtonWidth( TYPICAL_INPUT_WIDTH + 30 );
    m_MainTableLabelsLayout.AddButton( m_FFEqnChoiceLabel, "FF Equation" );
    m_FFEqnChoiceLabel.GetFlButton()->box( FL_THIN_UP_BOX );
    m_FFEqnChoiceLabel.GetFlButton()->labelcolor( FL_BLACK );
    m_FFEqnChoiceLabel.GetFlButton()->copy_tooltip( "Form Factor Equation Choice \n One per Geometry" );

    m_MainTableLabelsLayout.SetButtonWidth( TYPICAL_INPUT_WIDTH - 20 );
    m_MainTableLabelsLayout.AddButton( m_FFLabel, "FF" );
    m_FFLabel.GetFlButton()->box( FL_THIN_UP_BOX );
    m_FFLabel.GetFlButton()->labelcolor( FL_BLACK );
    m_FFLabel.GetFlButton()->copy_tooltip( "Manual or Calculated Form Factor" );

    m_MainTableLabelsLayout.SetButtonWidth( TYPICAL_INPUT_WIDTH + 5 );
    m_MainTableLabelsLayout.AddButton( m_LrefUnitLabel, "L_ref (ft)" );
    m_LrefUnitLabel.GetFlButton()->box( FL_THIN_UP_BOX );
    m_LrefUnitLabel.GetFlButton()->labelcolor( FL_BLACK );
    m_LrefUnitLabel.GetFlButton()->copy_tooltip( "Reference Length of Geometry" );

    m_MainTableLabelsLayout.SetButtonWidth( TYPICAL_INPUT_WIDTH - 10 );
    m_MainTableLabelsLayout.AddButton( m_FineRatorToCLabel, "t/c or l/d" );
    m_FineRatorToCLabel.GetFlButton()->box( FL_THIN_UP_BOX );
    m_FineRatorToCLabel.GetFlButton()->labelcolor( FL_BLACK );
    m_FineRatorToCLabel.GetFlButton()->copy_tooltip( "Thickness over Chord \n \t or \n Fineness Ratio" );

    m_MainTableLabelsLayout.SetButtonWidth( TYPICAL_INPUT_WIDTH );
    m_MainTableLabelsLayout.AddButton( m_ReLabel, "Re (1e6)" );
    m_ReLabel.GetFlButton()->box( FL_THIN_UP_BOX );
    m_ReLabel.GetFlButton()->labelcolor( FL_BLACK );
    m_ReLabel.GetFlButton()->copy_tooltip( "Unitless Reynolds Number" );

    if ( ParasiteDragMgr.m_TurbCfEqnType() == vsp::CF_TURB_HEATTRANSFER_WHITE_CHRISTOPH )
    {
        m_MainTableLabelsLayout.SetButtonWidth( TYPICAL_INPUT_WIDTH - 20 );
        m_MainTableLabelsLayout.AddButton( m_TeTwLabel, "Te/Tw" );
        m_TeTwLabel.GetFlButton()->box( FL_THIN_UP_BOX );
        m_TeTwLabel.GetFlButton()->labelcolor( FL_BLACK );
        m_TeTwLabel.GetFlButton()->copy_tooltip( "Freestream Temperature over Wall Temperature" );

        m_MainTableLabelsLayout.SetButtonWidth( TYPICAL_INPUT_WIDTH - 10 );
        m_MainTableLabelsLayout.AddButton( m_TawTwLabel, "Taw/Tw" );
        m_TawTwLabel.GetFlButton()->box( FL_THIN_UP_BOX );
        m_TawTwLabel.GetFlButton()->labelcolor( FL_BLACK );
        m_TawTwLabel.GetFlButton()->copy_tooltip( "Adiabatic Wall Temperature over Wall Temperature" );
    }
    else if ( ParasiteDragMgr.m_TurbCfEqnType() >= vsp::CF_TURB_ROUGHNESS_SCHLICHTING_AVG &&
              ParasiteDragMgr.m_TurbCfEqnType() <= vsp::CF_TURB_ROUGHNESS_SCHLICHTING_AVG_FLOW_CORRECTION )
    {
        AddRoughnessLabel();
    }

    m_MainTableLabelsLayout.SetButtonWidth( TYPICAL_INPUT_WIDTH - 20 );
    m_MainTableLabelsLayout.AddButton( m_PercLamLabel, "% Lam" );
    m_PercLamLabel.GetFlButton()->box( FL_THIN_UP_BOX );
    m_PercLamLabel.GetFlButton()->labelcolor( FL_BLACK );
    m_PercLamLabel.GetFlButton()->copy_tooltip( "Percentage of Flow that is Turbulent \n Values should be input from 0 - 100" );

    m_MainTableLabelsLayout.SetButtonWidth( TYPICAL_INPUT_WIDTH );
    m_MainTableLabelsLayout.AddButton( m_CfLabel, "C_f (1e-3)" );
    m_CfLabel.GetFlButton()->box( FL_THIN_UP_BOX );
    m_CfLabel.GetFlButton()->labelcolor( FL_BLACK );
    m_CfLabel.GetFlButton()->copy_tooltip( "Friction Coefficient Calculated from Selected Equations" );

    m_MainTableLabelsLayout.SetButtonWidth( TYPICAL_INPUT_WIDTH - 20 );
    m_MainTableLabelsLayout.AddButton( m_QLabel, "Q" );
    m_QLabel.GetFlButton()->box( FL_THIN_UP_BOX );
    m_QLabel.GetFlButton()->labelcolor( FL_BLACK );
    m_QLabel.GetFlButton()->copy_tooltip( "Interference Factor" );

    m_ConstantTableLabelsLayout.SetButtonWidth( TYPICAL_INPUT_WIDTH - 10 );
    m_ConstantTableLabelsLayout.AddButton( m_fLabel, "f" );
    m_fLabel.GetFlButton()->box( FL_THIN_UP_BOX );
    m_fLabel.GetFlButton()->labelcolor( FL_BLACK );
    m_fLabel.GetFlButton()->copy_tooltip( "Flat Plate Friction Drag" );

    m_ConstantTableLabelsLayout.AddButton( m_CdLabel, "C_D" );
    m_CdLabel.GetFlButton()->box( FL_THIN_UP_BOX );
    m_CdLabel.GetFlButton()->labelcolor( FL_BLACK );
    m_CdLabel.GetFlButton()->copy_tooltip( "Coefficient of Parasite Drag Per Component" );

    m_ConstantTableLabelsLayout.AddButton( m_PercTotalLabel, "% Total" );
    m_PercTotalLabel.GetFlButton()->labelcolor( FL_BLACK );
    m_PercTotalLabel.GetFlButton()->copy_tooltip( "Percentage Total Contributed to Final Cd0" );

    InitSortToggleGroup();
}

void ParasiteDragScreen::InitSortToggleGroup()
{
    m_SortToggle.Init( this );
    m_SortToggle.ClearButtons();
    m_SortToggle.AddButton( m_CompLabel.GetFlButton() );
    m_SortToggle.AddButton( m_SwetUnitLabel.GetFlButton() );
    m_SortToggle.AddButton( m_PercTotalLabel.GetFlButton() );
}

void ParasiteDragScreen::UpdateIncorporateDropDowns()
{
    Vehicle* veh = VehicleMgr.GetVehicle();
    if ( veh )
    {
        char str[256];
        vector < string > ancestorNames;
        vector < ParasiteDragTableRow > rowVec = ParasiteDragMgr.GetMainTableVec();
        for ( size_t i = 0; i < rowVec.size(); ++i )
        {
            if ( m_grouped[i].GetFlChoice() )
            {
                // Incorporate Drop Downs
                m_grouped[i].ClearItems();

                m_grouped[i].AddItem( "SELF" );
                veh->FindGeom( rowVec[i].GeomID )->BuildAncestorList( ancestorNames );
                for ( size_t j = 1; j < ancestorNames.size(); ++j )
                {
                    sprintf( str, "%u %s", j, ancestorNames[j].c_str() );
                    m_grouped[i].AddItem( str );
                }
                ancestorNames.clear();

                for ( size_t j = 1; j < m_grouped[i].GetItems().size(); ++j )
                {
                    if ( veh->FindGeom( veh->FindGeom( rowVec[i].GeomID )->GetAncestorID( j ) )->GetType().m_Type != HINGE_GEOM_TYPE &&
                        veh->FindGeom( veh->FindGeom( rowVec[i].GeomID )->GetAncestorID( j ) )->GetType().m_Type != BLANK_GEOM_TYPE)
                    {
                        if ( rowVec[i].GeomShapeType !=
                                veh->FindGeom( veh->FindGeom( rowVec[i].GeomID )->GetAncestorID( j ) )->GetSurfType(0) )
                        {
                            m_grouped[i].SetFlag( j, FL_MENU_INACTIVE );
                        }
                    }
                    else
                    {
                        m_grouped[i].SetFlag( j, FL_MENU_INACTIVE );
                    }
                }
                m_grouped[i].UpdateItems();
                m_grouped[i].SetVal( rowVec[i].GroupedAncestorGen );
            }
        }
    }
}

void ParasiteDragScreen::RebuildBuildUpTable()
{
    SaveMainTableScrollPosition();

    ClearTableScrollGroups();

    ScrollToInitPosition(); // To 0,0 to build everything from default position

    RedrawTableScrollGroups();

    RebuildBuildUpTableLabels();

    ClearGUIElements();

    ResizeDeviceVectors( ParasiteDragMgr.CalcRowSize(),
                         ParasiteDragMgr.GetExcresVec().size() );

    ParasiteDragMgr.Calculate_ALL();

    UpdateReynoldsLabel();

    AddGeomsToTable();

    AddSimpleExcresToTable();

    UpdateFinalValues();

    UpdateTableLabels();

    RevertScrollbarPositions();
}

void ParasiteDragScreen::SaveMainTableScrollPosition()
{
    main_XScrollPosition = m_MainTableScrollGroup->xposition();
    main_YScrollPosition = m_MainTableScrollGroup->yposition();
}

void ParasiteDragScreen::RevertScrollbarPositions()
{
    m_MainTableScrollGroup->scroll_to( main_XScrollPosition, main_YScrollPosition );
    m_MainTableCompNamesScrollGroup->scroll_to( 0, main_YScrollPosition );
    m_MainTableLabelsScrollGroup->scroll_to( main_XScrollPosition, 0 );
    m_ConstantTableScrollGroup->scroll_to( 0, main_YScrollPosition );
}

void ParasiteDragScreen::RedrawTableScrollGroups()
{
    m_ComponentLabelScrollGroup->redraw();
    m_MainTableScrollGroup->redraw();
    m_MainTableCompNamesScrollGroup->redraw();
    m_MainTableLabelsScrollGroup->redraw();
    m_ConstTableLabelsScrollGroup->redraw();
    m_ConstantTableScrollGroup->redraw();
}

void ParasiteDragScreen::ScrollToInitPosition()
{
    m_MainTableScrollGroup->scroll_to( 0, 0 );
    m_MainTableCompNamesScrollGroup->scroll_to( 0, 0 );
    m_MainTableLabelsScrollGroup->scroll_to( 0, 0 );
    m_ConstantTableScrollGroup->scroll_to( 0, 0 );
}

void ParasiteDragScreen::ClearTableScrollGroups()
{
    m_ComponentLabelScrollGroup->clear();
    m_ComponentLabelLayout.SetGroup( m_ComponentLabelScrollGroup );

    m_MainTableLabelsScrollGroup->clear();
    m_MainTableLabelsLayout.SetGroup( m_MainTableLabelsScrollGroup );
    m_MainTableLabelsLayout.SetInputWidth( TYPICAL_INPUT_WIDTH );

    m_ConstTableLabelsScrollGroup->clear();
    m_ConstantTableLabelsLayout.SetGroup( m_ConstTableLabelsScrollGroup );

    m_MainTableCompNamesScrollGroup->clear();
    m_TableCompNamesLayout.SetGroup( m_MainTableCompNamesScrollGroup );

    m_MainTableScrollGroup->clear();
    m_TableLayout.SetGroup( m_MainTableScrollGroup );
    m_TableLayout.SetChoiceButtonWidth( 0 );
    m_TableLayout.SetInputWidth( TYPICAL_INPUT_WIDTH );

    m_ConstantTableScrollGroup->clear();
    m_ConstantViewLayout.SetGroup( m_ConstantTableScrollGroup );
    m_ConstantViewLayout.SetInputWidth( TYPICAL_INPUT_WIDTH );
}

void ParasiteDragScreen::ClearGUIElements()
{
    m_labels.clear();
    m_swet.clear();
    m_grouped.clear();
    m_lref.clear();
    m_Re.clear();
    m_Roughness.clear();
    m_TeTwRatio.clear();
    m_TawTwRatio.clear();
    m_percLam.clear();
    m_Cf.clear();
    m_fineRatorToC.clear();
    m_ffType.clear();
    m_ffIn.clear();
    m_ffOut.clear();
    m_Q.clear();
    m_f.clear();
    m_cd.clear();
    m_percTotal.clear();
    m_subsurfinclude.clear();

    m_GeomfTotal.Update( "" );
    m_GeomCdTotal.Update( "" );
    m_GeomPercTotal.Update( "" );
    m_ExcresfTotal.Update( "" );
    m_ExcresCdTotal.Update( "" );
    m_ExcresPercTotal.Update( "" );
    m_fTotal.Update( "" );
    m_CDTotal.Update( "" );
    m_PercTotal.Update( "" );

    m_excresType.clear();
    m_excresOutput.clear();
}

void ParasiteDragScreen::DeactivateRow( int index, int grayFlag )
{
    if ( grayFlag == 1 )
    {
        m_swet[index].Deactivate();
        m_lref[index].Deactivate();
        m_grouped[index].Deactivate();
        m_Re[index].Deactivate();
        m_Roughness[index].Deactivate();
        m_TeTwRatio[index].Deactivate();
        m_TawTwRatio[index].Deactivate();
        m_percLam[index].Deactivate();
        m_Cf[index].Deactivate();
        m_fineRatorToC[index].Deactivate();
        m_ffType[index].Deactivate();
        m_ffOut[index].Deactivate();
        m_ffIn[index].Deactivate();
        m_Q[index].Deactivate();
        m_f[index].Deactivate();
        m_cd[index].Deactivate();
        m_percTotal[index].Deactivate();
        m_subsurfinclude[index].Deactivate();
    }
}

void ParasiteDragScreen::ResizeDeviceVectors( int geomSize, int excresSize )
{
    m_labels.resize( geomSize );
    m_swet.resize( geomSize );
    m_grouped.resize( geomSize );
    m_lref.resize( geomSize );
    m_Re.resize( geomSize );
    m_Roughness.resize( geomSize );
    m_TeTwRatio.resize( geomSize );
    m_TawTwRatio.resize( geomSize );
    m_percLam.resize( geomSize );
    m_Cf.resize( geomSize );
    m_fineRatorToC.resize( geomSize );
    m_ffType.resize( geomSize );
    m_ffIn.resize( geomSize );
    m_ffOut.resize( geomSize );
    m_Q.resize( geomSize );
    m_f.resize( geomSize + excresSize );
    m_cd.resize( geomSize + excresSize );
    m_percTotal.resize( geomSize + excresSize );
    m_subsurfinclude.resize( geomSize );

    m_excresType.resize( excresSize );
    m_excresOutput.resize( excresSize );
}

void ParasiteDragScreen::AddGeomsToTable()
{
    Vehicle* veh = VehicleMgr.GetVehicle();
    int grayFlag = 0, lastShape = -1, skipFlag = 0, subsurfFlag = 0;
    string lastID = "";
    string lastIncorporatedID = "";

    ParasiteDragMgr.SortMap();

    vector < ParasiteDragTableRow > rowVec = ParasiteDragMgr.GetMainTableVec();

    m_GeomOrderInTableVec.clear();
    for ( size_t i = 0; i < rowVec.size(); ++i )
    {
        m_GeomOrderInTableVec.push_back( rowVec[i].GeomID );

        GetTableFlags( i, lastID, lastIncorporatedID, lastShape,
                       OUT skipFlag, OUT subsurfFlag, OUT grayFlag );

        m_TableLayout.SetSameLineFlag( true );
        m_ConstantViewLayout.SetSameLineFlag( true );

        if ( !skipFlag )
        {
            // ---- These two components are the same regardless of subsurfFlag ---- //
            SetupRowLabel( subsurfFlag, lastID, i );
            SetupSwet( i, rowVec[i].Swet );

            if ( !subsurfFlag )
            {
                // Set Up of Incorporate Choice
                m_TableLayout.SetSliderWidth( TYPICAL_INPUT_WIDTH ); // Sets Drop Down Menu Width
                m_TableLayout.AddChoice( m_grouped[i], "" ); // Is updated later in UpdateIncorporatedDropDowns()

                // Setup Rest of Row
                SetupFFType( i, rowVec[i].GeomShapeType, rowVec[i].FFEqnChoice );
                SetupFFValue( i, rowVec[i].FF, rowVec[i].FFEqnChoice );
                SetupLref( i, rowVec[i].Lref );
                SetupFineRatorToC( i, rowVec[i].fineRatorToC );
                SetupReyNum( i, rowVec[i].Re );
                SetupRoughness( i, rowVec[i].Roughness );
                SetupHeatTransfer( i, rowVec[i].TeTwRatio, rowVec[i].TawTwRatio );
                SetupPercLaminar( i, rowVec[i].PercLam );
                SetupCf( i, rowVec[i].Cf );
                SetupQ( i, rowVec[i].Q, rowVec[i].GeomShapeType, rowVec[i].FFEqnChoice );
                Setupf( i, rowVec[i].f );
                SetupCD( i, rowVec[i].CD );
                SetupPercCD( i, rowVec[i].PercTotalCD );

                m_TableLayout.SetSameLineFlag( false );
                m_TableLayout.ForceNewLine();

                m_ConstantViewLayout.SetSameLineFlag( false );
                m_ConstantViewLayout.ForceNewLine();

                DeactivateRow( i, grayFlag );

                if ( rowVec[i].MasterRow )
                {
                    m_grouped[i].Activate();
                }
                lastID = rowVec[i].GeomID;
                lastShape = rowVec[i].GeomShapeType;

                if ( m_grouped[i].GetVal() == 0 )
                {
                    lastIncorporatedID = lastID;
                }
                else
                {
                    lastIncorporatedID = VehicleMgr.GetVehicle()->FindGeom( rowVec[i].GeomID )->GetAncestorID( rowVec[i].GroupedAncestorGen );
                }
            }
            else
            {
                int subsurfflag = 1;
                if ( veh->FindGeom( rowVec[i].GeomID )->m_ExpandedListFlag() ||
                        veh->FindGeom( rowVec[i].GeomID )->GetSubSurf( rowVec[i].SubSurfID )->m_IncludeType() == vsp::SS_INC_SEPARATE_TREATMENT )
                {
                    SetupSubSurfInclude( i, rowVec[i].GeomID, rowVec[i].SubSurfID );
                    if ( veh->FindGeom( rowVec[i].GeomID )->GetSubSurf( rowVec[i].SubSurfID )->m_IncludeType() == vsp::SS_INC_SEPARATE_TREATMENT )
                    {
                        SetupFFType( i, rowVec[i].GeomShapeType, rowVec[i].FFEqnChoice );
                        SetupFFValue( i, rowVec[i].FF, rowVec[i].FFEqnChoice );
                        SetupLref( i, rowVec[i].Lref );
                        SetupFineRatorToC( i, rowVec[i].fineRatorToC );
                        SetupReyNum( i, rowVec[i].Re );
                        SetupRoughness( i, rowVec[i].Roughness );
                        SetupHeatTransfer( i, rowVec[i].TeTwRatio, rowVec[i].TawTwRatio );
                        SetupPercLaminar( i, rowVec[i].PercLam );
                        SetupCf( i, rowVec[i].Cf );
                        SetupQ( i, rowVec[i].Q, rowVec[i].GeomShapeType, rowVec[i].FFEqnChoice );
                    }
                    Setupf( i, rowVec[i].f );
                    SetupCD( i, rowVec[i].CD );
                    SetupPercCD( i, rowVec[i].PercTotalCD );

                    m_TableLayout.SetSameLineFlag( false );
                    m_TableLayout.ForceNewLine();

                    m_ConstantViewLayout.SetSameLineFlag( false );
                    m_ConstantViewLayout.ForceNewLine();

                    DeactivateRow( i, grayFlag );
                    lastID = rowVec[i].GeomID;
                }
            }
        }
    }

    // This solves a UI issues causing the geometries to start much lower than immediately below labels
    if ( rowVec.size() > 0 )
    {
        m_TableLayout.AddYGap();
        m_TableCompNamesLayout.AddYGap();
        m_ConstantViewLayout.AddYGap();
    }

}

void ParasiteDragScreen::GetTableFlags( int index, const string &lastID, const string &lastIncorporatedID, int lastShape,
                                        int &skipFlag, int &subsurfFlag, int &grayFlag )
{
    Vehicle* veh = VehicleMgr.GetVehicle();

    vector <ParasiteDragTableRow> rowVec = ParasiteDragMgr.GetMainTableVec();

    // Check if shape is copy
    if ( index > 0 )
    {
        if ( rowVec[index].SubSurfID.compare( "" ) != 0 ) // If this row is a subsurface
        {
            if ( veh->FindGeom( rowVec[index].GeomID )->GetSubSurf( rowVec[index].SubSurfID )->m_IncludeType() == vsp::SS_INC_SEPARATE_TREATMENT &&
                    rowVec[index].MasterRow )
            {
                skipFlag = 0;
            }
            else
            {
                skipFlag = !veh->FindGeom( rowVec[index].GeomID )->m_ExpandedListFlag();
            }
            subsurfFlag = 1;
            // Gray out any rows that aren't the master subsurface
            if ( rowVec[index].MasterRow ||
                    ( rowVec[index].SurfNum == 0 && veh->FindGeom( rowVec[index].GeomID )->GetSubSurf( rowVec[index].SubSurfID )->m_IncludeType() != vsp::SS_INC_SEPARATE_TREATMENT ) )
            {
                grayFlag = 0;
            }
            else
            {
                grayFlag = 1;
            }
        }
        else if ( rowVec[index].GroupedAncestorGen > 0 && rowVec[index].MasterRow )
        {
            skipFlag = 0;
            grayFlag = 1;
            subsurfFlag = 0;
        }
        else if ( lastID == rowVec[index].GeomID && lastShape == rowVec[index].GeomShapeType )
        {
            skipFlag = !veh->FindGeom( rowVec[index].GeomID )->m_ExpandedListFlag();
            grayFlag = 1;
            subsurfFlag = 0;
        }
        else
        {
            skipFlag = 0;
            grayFlag = 0;
            subsurfFlag = 0;
        }

        if ( lastID.compare( rowVec[index].GeomID ) != 0 &&
                lastIncorporatedID.compare( veh->FindGeom( rowVec[index].GeomID )->GetAncestorID( rowVec[index].GroupedAncestorGen ) ) != 0 )
        {
            m_TableLayout.AddYGap();
            m_TableCompNamesLayout.AddYGap();
            m_ConstantViewLayout.AddYGap();
        }
    }
}


string ParasiteDragScreen::GetComponentTableLabel( int subsurfFlag, const string &lastID, int index )
{
    Vehicle* veh = VehicleMgr.GetVehicle();
    char str[256];

    vector <ParasiteDragTableRow> rowVec = ParasiteDragMgr.GetMainTableVec();

    if ( !subsurfFlag )
    {
        if ( rowVec[index].MasterRow )
        {
            if ( veh->FindGeom( rowVec[index].GeomID )->GetType().m_Type == CUSTOM_GEOM_TYPE )
            {
                sprintf( str, "(+) %s", rowVec[index].Label.c_str() );
            }
            else
            {
                if ( rowVec[index].ExpandedList )
                {
                    sprintf( str, "@2> %s", rowVec[index].Label.c_str() );
                }
                else
                {
                    sprintf( str, "(+) %s", veh->FindGeom( rowVec[index].GeomID )->GetName().c_str() );
                }
            }
        }
        else
        {
            // Expanded List
            sprintf( str, "%s", rowVec[index].Label.c_str() ); // All other Surfaces (not including subsurfaces)
        }
    }
    else
    {
        sprintf( str, "%s", rowVec[index].Label.c_str() ); // is subsurface
    }

    return string( str );
}

void ParasiteDragScreen::SetupRowLabel( int subsurfFlag, const string &lastID, int index )
{
    m_TableCompNamesLayout.SetButtonWidth( TYPICAL_INPUT_WIDTH * 2 + 20 );
    string label = GetComponentTableLabel( subsurfFlag, lastID, index );
    m_TableCompNamesLayout.AddButton( m_labels[index], label.c_str() );
    m_labels[index].GetFlButton()->align( Fl_Align( FL_ALIGN_LEFT | FL_ALIGN_INSIDE | FL_ALIGN_WRAP ) );
}

void ParasiteDragScreen::SetupSwet( int index, double swet )
{
    m_TableLayout.SetButtonWidth( 0 );
    ostringstream strs;
    int precision = 2;
    m_TableLayout.SetInputWidth( TYPICAL_INPUT_WIDTH + 20 );
    m_TableLayout.AddOutput( m_swet[index], "" );
    if ( swet != -1 )
    {
        strs << setprecision( precision ) << fixed << swet;
        m_swet[index].Update( strs.str() );
    }
}

void ParasiteDragScreen::SetupLref( int index, double lref )
{
    ostringstream strs;
    int precision = ParasiteDragMgr.GetLrefSigFig();
    m_TableLayout.SetInputWidth( TYPICAL_INPUT_WIDTH + 5 );
    m_TableLayout.AddOutput( m_lref[index], "" );
    if ( lref != -1 )
    {
        strs << setprecision( precision ) << fixed << lref;
        m_lref[index].Update( strs.str() );
    }
}

void ParasiteDragScreen::SetupFineRatorToC( int index, double finerat )
{
    ostringstream strs;
    int precision = 3;
    m_TableLayout.SetInputWidth( TYPICAL_INPUT_WIDTH - 10 );
    m_TableLayout.AddOutput( m_fineRatorToC[index], "" );
    if ( finerat != -1 )
    {
        strs << setprecision( precision ) << fixed << finerat;
        m_fineRatorToC[index].Update( strs.str() );
    }
}

void ParasiteDragScreen::SetupFFType( int index, int shapetype, int eqnchoice )
{
    m_TableLayout.SetSliderWidth( TYPICAL_INPUT_WIDTH + 30 );
    m_TableLayout.AddChoice( m_ffType[index], "" );
    m_ffType[index].ClearItems();
    if ( shapetype == vsp::NORMAL_SURF )
    {
        m_ffType[index].AddItem( "Manual" );
        m_ffType[index].AddItem( "Schemensky DATCOM RAND Fuselage" );
        m_ffType[index].AddItem( "Schemensky DATCOM RAND Nacelle" );
        m_ffType[index].AddItem( "Hoerner Streamlined Body" );
        m_ffType[index].AddItem( "Torenbeek" );
        m_ffType[index].AddItem( "Shevell" );
        m_ffType[index].AddItem( "Covert" );
        m_ffType[index].AddItem( "Jenkinson Fuselage" );
        m_ffType[index].AddItem( "Jenkinson Wing Mounted Nacelle" );
        m_ffType[index].AddItem( "Jenkinson Aft Fuse Mounted Nacelle" );
    }
    else
    {
        m_ffType[index].AddItem( "Manual" );
        m_ffType[index].AddItem( "EDET Conventional" );
        m_ffType[index].AddItem( "EDET Advanced" );
        m_ffType[index].AddItem( "Hoerner" );
        m_ffType[index].AddItem( "Covert" );
        m_ffType[index].AddItem( "Williams" );
        m_ffType[index].AddItem( "Kroo" );
        m_ffType[index].AddItem( "Torenbeek" );
        m_ffType[index].AddItem( "DATCOM" );
        m_ffType[index].AddItem( "Schemensky 6 Series AF" );
        m_ffType[index].AddItem( "Schemensky 4 Series AF" );
        m_ffType[index].AddItem( "Jenkinson Wing" );
        m_ffType[index].AddItem( "Jenkinson Tail" );
    }
    m_ffType[index].UpdateItems();
    m_ffType[index].SetVal( eqnchoice );
}

void ParasiteDragScreen::SetupFFValue( int index, double formfactor, int eqnchoice )
{
    ostringstream strs;
    int precision = 2;
    m_TableLayout.SetInputWidth( TYPICAL_INPUT_WIDTH - 20 );
    if ( eqnchoice == 0 )
    {
        m_TableLayout.AddInput( m_ffIn[index], "" );
        if ( formfactor != -1 )
        {
            strs << setprecision( precision ) << fixed << formfactor;
            m_ffIn[index].Update( strs.str() );
        }
    }
    else
    {
        m_TableLayout.AddOutput( m_ffOut[index], "" );
        if ( formfactor != -1 )
        {
            strs << setprecision( precision ) << fixed << formfactor;
            m_ffOut[index].Update( strs.str() );
        }
    }
}

void ParasiteDragScreen::SetupReyNum( int index, double reynum )
{
    ostringstream strs;
    int precision = 2;
    m_TableLayout.SetInputWidth( TYPICAL_INPUT_WIDTH );
    m_TableLayout.AddOutput( m_Re[index], "" );
    if ( reynum != -1 )
    {
        strs << setprecision( precision ) << fixed << reynum / pow( 10.0, ParasiteDragMgr.GetReynoldsDivisor() );
        m_Re[index].Update( strs.str() );
    }
}

void ParasiteDragScreen::SetupRoughness( int index, double roughness )
{
    ostringstream strs;
    int precision = 2;
    if ( ParasiteDragMgr.m_TurbCfEqnType() >= vsp::CF_TURB_ROUGHNESS_SCHLICHTING_AVG && ParasiteDragMgr.m_TurbCfEqnType() < vsp::CF_TURB_HEATTRANSFER_WHITE_CHRISTOPH )
    {
        m_TableLayout.SetInputWidth( TYPICAL_INPUT_WIDTH + 30 );
        m_TableLayout.AddInput( m_Roughness[index], "" );
        if ( roughness != -1 )
        {
            strs << setprecision( precision ) << fixed << roughness;
            m_Roughness[index].Update( strs.str() );
        }
    }
}

void ParasiteDragScreen::SetupHeatTransfer( int index, double tetwratio, double tawtwratio )
{
    int precision = 2;
    if ( ParasiteDragMgr.m_TurbCfEqnType() == vsp::CF_TURB_HEATTRANSFER_WHITE_CHRISTOPH )
    {
        // Set up of Te/Tw Input
        m_TableLayout.SetInputWidth( TYPICAL_INPUT_WIDTH - 20 );
        m_TableLayout.AddInput( m_TeTwRatio[index], "" );
        if ( tetwratio != -1 )
        {
            ostringstream strs;
            strs << setprecision( precision ) << fixed << tetwratio;
            m_TeTwRatio[index].Update( strs.str() );
        }

        // Set up of Taw/Tw Input
        m_TableLayout.SetInputWidth( TYPICAL_INPUT_WIDTH - 10 );
        m_TableLayout.AddInput( m_TawTwRatio[index], "" );
        if ( tawtwratio != -1 )
        {
            ostringstream strs;
            strs << setprecision( precision ) << fixed << tawtwratio;
            m_TawTwRatio[index].Update( strs.str() );
        }
    }
}

void ParasiteDragScreen::SetupPercLaminar( int index, double perclam )
{
    ostringstream strs;
    int precision = 1;
    m_TableLayout.SetInputWidth( TYPICAL_INPUT_WIDTH - 20 );
    m_TableLayout.AddInput( m_percLam[index], "" );
    if ( perclam != -1 )
    {
        strs << setprecision( precision ) << fixed << perclam;
        m_percLam[index].Update( strs.str() );
    }
}

void ParasiteDragScreen::SetupCf( int index, double cf )
{
    ostringstream strs;
    int precision = 2;
    m_TableLayout.SetInputWidth( TYPICAL_INPUT_WIDTH );
    m_TableLayout.AddOutput( m_Cf[index], "" );
    if ( cf != -1 )
    {
        strs << setprecision( precision ) << fixed << cf / 1e-3;
        m_Cf[index].Update( strs.str() );
    }
}

void ParasiteDragScreen::SetupQ( int index, double Q, int shapetype, int eqnchoice )
{
    // TODO: Update to use output GUI element for Jenkinson Tail
    ostringstream strs;
    int precision = 2;
    m_TableLayout.SetInputWidth( TYPICAL_INPUT_WIDTH - 20 );
    m_TableLayout.AddInput( m_Q[index], "" );
    if ( Q != -1 )
    {
        if ( shapetype == vsp::WING_SURF && eqnchoice == vsp::FF_W_JENKINSON_TAIL )
        {
            Q = 1.2;
        }

        strs << setprecision( precision ) << fixed << Q;
        m_Q[index].Update( strs.str() );
    }
}

void ParasiteDragScreen::Setupf( int index, double f )
{
    ostringstream strs;
    int precision = 4;
    m_ConstantViewLayout.SetInputWidth( TYPICAL_INPUT_WIDTH - 10 );
    m_ConstantViewLayout.AddOutput( m_f[index], "" );
    if ( f != -1 )
    {
        strs << setprecision( precision ) << fixed << f;
        m_f[index].Update( strs.str() );
    }
}

void ParasiteDragScreen::SetupCD( int index, double CD )
{
    ostringstream strs;
    int precision = 5;
    m_ConstantViewLayout.AddOutput( m_cd[index], "" );
    if ( CD != -1 )
    {
        strs << setprecision( precision ) << fixed << CD;
        m_cd[index].Update( strs.str() );
    }
}

void ParasiteDragScreen::SetupPercCD( int index, double percCD )
{
    ostringstream strs;
    int precision = 2;
    m_ConstantViewLayout.AddOutput( m_percTotal[index], "" );
    if ( percCD != -1 )
    {
        strs << setprecision( precision ) << fixed << percCD * 100;
        m_percTotal[index].Update( strs.str() );
    }
}

void ParasiteDragScreen::SetupSubSurfInclude( int index, const string geomid, const string subsurfid )
{
    Vehicle* veh = VehicleMgr.GetVehicle();
    m_TableLayout.SetSliderWidth( TYPICAL_INPUT_WIDTH );
    m_TableLayout.AddChoice( m_subsurfinclude[index], "" );
    m_subsurfinclude[index].ClearItems();

    m_subsurfinclude[index].AddItem( "Same as Parent" );
    m_subsurfinclude[index].AddItem( "Separate Treatment" );
    m_subsurfinclude[index].AddItem( "Zero Drag" );
    m_subsurfinclude[index].UpdateItems();
    m_subsurfinclude[index].SetVal( veh->FindGeom( geomid )->GetSubSurf( subsurfid )->m_IncludeType() );
}

void ParasiteDragScreen::SetupExcresType( int index, const string type )
{
    ostringstream strs;
    m_TableLayout.SetInputWidth( TYPICAL_INPUT_WIDTH + 45 );
    m_TableLayout.AddOutput( m_excresType[index], "" );
    strs << type;
    m_excresType[index].Update( strs.str() );
}

void ParasiteDragScreen::SetupExcresInput( int index, double input )
{
    ostringstream strs;
    int precision = 5;
    m_TableLayout.SetInputWidth( TYPICAL_INPUT_WIDTH );
    m_TableLayout.AddOutput( m_excresOutput[index], "" );
    strs << setprecision( precision ) << fixed << input;
    m_excresOutput[index].Update( strs.str() );
}

void ParasiteDragScreen::AddSimpleExcresToTable()
{
    RebuildExcresTableLabels();

    ParasiteDragMgr.UpdateExcres();

    vector < ExcrescenceTableRow > excresRowVec = ParasiteDragMgr.GetExcresVec();

    for ( size_t i = 0; i < excresRowVec.size(); ++i )
    {
        m_TableLayout.SetButtonWidth( 0 );
        m_TableLayout.SetSameLineFlag( true );
        m_ConstantViewLayout.SetSameLineFlag( true );

        m_TableCompNamesLayout.AddLabel( excresRowVec[i].Label.c_str(), TYPICAL_INPUT_WIDTH * 2 - 15 );

        SetupExcresType( i, excresRowVec[i].TypeString );
        SetupExcresInput( i, excresRowVec[i].Input );

        Setupf( i, excresRowVec[i].f );
        SetupCD( i, excresRowVec[i].Amount );
        SetupPercCD( i, excresRowVec[i].PercTotalCD );

        m_TableCompNamesLayout.ForceNewLine();

        m_TableLayout.SetSameLineFlag( false );
        m_TableLayout.ForceNewLine();

        m_ConstantViewLayout.SetSameLineFlag( false );
        m_ConstantViewLayout.ForceNewLine();

        m_CurrentRowIndex++;
    }
}

void ParasiteDragScreen::RebuildExcresTableLabels()
{
    m_TableLayout.SetSameLineFlag( true );
    m_TableLayout.SetFitWidthFlag( false );

    m_TableCompNamesLayout.AddLabel( "Excrescence", TYPICAL_INPUT_WIDTH * 2 + 20 - m_MainTableCompNamesScrollGroup->scrollbar.w() );
    m_TableLayout.AddLabel( "Type", TYPICAL_INPUT_WIDTH + 45 );
    m_TableLayout.AddLabel( "Input", TYPICAL_INPUT_WIDTH );

    m_ConstantViewLayout.AddLabel( "", ( TYPICAL_INPUT_WIDTH - 10 ) * 3 );

    m_TableCompNamesLayout.ForceNewLine();
    m_TableLayout.ForceNewLine();
    m_ConstantViewLayout.ForceNewLine();
}

void ParasiteDragScreen::UpdateFinalValues()
{
    Vehicle* veh = VehicleMgr.GetVehicle();
    ostringstream strs;

    int fprecision = 4;
    int cdprecision = 5;
    int percprecision = 1;


    // Geometry Relevant
    strs << setprecision( fprecision ) << fixed << ParasiteDragMgr.GetGeomfTotal();
    m_GeomfTotal.Update( strs.str() );
    strs.str( "" );

    strs << setprecision( cdprecision ) << fixed << ParasiteDragMgr.GetGeometryCD();
    m_GeomCdTotal.Update( strs.str() );
    strs.str( "" );

    strs << setprecision( percprecision ) << fixed << ParasiteDragMgr.GetGeomPercTotal() * 100;
    m_GeomPercTotal.Update( strs.str() );
    strs.str( "" );

    // Excrescence Relevant
    strs << setprecision( fprecision ) << fixed << ParasiteDragMgr.GetExcresfTotal();
    m_ExcresfTotal.Update( strs.str() );
    strs.str( "" );

    strs << setprecision( cdprecision ) << fixed << ParasiteDragMgr.GetTotalExcresCD();
    m_ExcresCdTotal.Update( strs.str() );
    strs.str( "" );

    strs << setprecision( percprecision ) << fixed << ParasiteDragMgr.GetExcresPercTotal() * 100;
    m_ExcresPercTotal.Update( strs.str() );
    strs.str( "" );

    // Absolute Totals
    strs << setprecision( fprecision ) << fixed << ParasiteDragMgr.GetfTotal();
    m_fTotal.Update( strs.str() );
    strs.str( "" );

    strs << setprecision( cdprecision ) << fixed << ParasiteDragMgr.GetTotalCD();
    m_CDTotal.Update( strs.str() );
    strs.str( "" );

    strs << setprecision( percprecision ) << fixed << ParasiteDragMgr.GetPercTotal() * 100;
    m_PercTotal.Update( strs.str() );
    strs.str( "" );
}

void ParasiteDragScreen::AddRoughnessLabel()
{
    m_MainTableLabelsLayout.SetButtonWidth( TYPICAL_INPUT_WIDTH + 30 );
    if ( ParasiteDragMgr.m_LengthUnit() == vsp::LEN_MM )
    {
        m_MainTableLabelsLayout.AddButton( m_RoughnessLabel, "Rough. Ht. (mm)" );
    }
    else if ( ParasiteDragMgr.m_LengthUnit() == vsp::LEN_CM )
    {
        m_MainTableLabelsLayout.AddButton( m_RoughnessLabel, "Rough. Ht. (cm)" );
    }
    else if ( ParasiteDragMgr.m_LengthUnit() == vsp::LEN_M )
    {
        m_MainTableLabelsLayout.AddButton( m_RoughnessLabel, "Rough. Ht. (m)" );
    }
    else if ( ParasiteDragMgr.m_LengthUnit() == vsp::LEN_IN )
    {
        m_MainTableLabelsLayout.AddButton( m_RoughnessLabel, "Rough. Ht. (in)" );
    }
    else if ( ParasiteDragMgr.m_LengthUnit() == vsp::LEN_FT )
    {
        m_MainTableLabelsLayout.AddButton( m_RoughnessLabel, "Rough. Ht. (ft)" );
    }
    else if ( ParasiteDragMgr.m_LengthUnit() == vsp::LEN_YD )
    {
        m_MainTableLabelsLayout.AddButton( m_RoughnessLabel, "Rough. Ht. (yd)" );
    }
    else if ( ParasiteDragMgr.m_LengthUnit() == vsp::LEN_UNITLESS )
    {
        m_MainTableLabelsLayout.AddButton( m_RoughnessLabel, "Rough. Ht." );
    }
    m_RoughnessLabel.GetFlButton()->labelcolor( FL_BLACK );
    m_RoughnessLabel.GetFlButton()->copy_tooltip( "Roughness Height" );
}

void ParasiteDragScreen::UpdateReynoldsLabel()
{
    if ( ParasiteDragMgr.GetReynoldsDivisor() > 1 )
    {
        string relabel;
        relabel = "Re (1e" + to_string( ParasiteDragMgr.GetReynoldsDivisor() ) + ")";
        m_ReLabel.GetFlButton()->copy_label( relabel.c_str() );
    }
    else
    {
        m_ReLabel.GetFlButton()->label( "Re (1e6)" );
    }
}

void ParasiteDragScreen::ExresBrowserCallback()
{
    int last = m_ExcresBrowser->value();
    if ( last >= 2 )
    {
        if ( ParasiteDragMgr.GetCurrExcresIndex() != last - 2 )
        {
            ParasiteDragMgr.SetCurrExcresIndex( last - 2 );
        }
    }
}

void ParasiteDragScreen::SelectExcresBrowser( int index )
{
    if ( index > 1 )
    {
        //==== Select if Match ====//
        m_ExcresBrowser->select( index );

        //==== Position Browser ====//
        m_ExcresBrowser->topline( index );
    }
}
