//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//

// ParasiteDragScreen.h: interface for launching VSPAERO.
//
//////////////////////////////////////////////////////////////////////

#ifndef PARASITEDRAGSCREEN_H
#define PARASITEDRAGSCREEN_H

#include "ScreenMgr.h"
#include "ScreenBase.h"
#include "Vehicle.h"

class ParasiteDragScreen;

class ParasiteDragScreen : public TabScreen
{
public:
    ParasiteDragScreen( ScreenMgr* mgr );
    virtual ~ParasiteDragScreen();

    void Show();
    void Hide();
    bool Update();

    void CallBack( Fl_Widget *w );
    static void staticScreenCB( Fl_Widget *w, void* data )
    {
        ( ( ParasiteDragScreen* )data )->CallBack( w );
    }

    virtual void GuiDeviceCallBack( GuiDevice* device );

    int main_XScrollPosition;
    int main_YScrollPosition;

protected:
    //==== Screen Functions ====//
    // Browser Callback
    void ExresBrowserCallback();
    void SelectExcresBrowser( int index );

    // General Functions
    void UpdateTableLabels();
    void UpdateSrefChoice();
    void UpdateSetChoice();
    void UpdateChoiceDevices();
    void UpdateSliderDevices();
    void UpdateExcrescenceSliderLimits();
    void UpdateAltitudeSliderLimits();
    void UpdateExcresTab();
    void UpdateDependentUnitLabels();

    // Table Functions
    void RebuildBuildUpTableLabels();
    void InitSortToggleGroup();
    void UpdateIncorporateDropDowns();
    void AddRoughnessLabel();
    void UpdateReynoldsLabel();
    void RebuildBuildUpTable();
    void SaveMainTableScrollPosition();
    void RevertScrollbarPositions();
    void RedrawTableScrollGroups();
    void ScrollToInitPosition();
    void ClearTableScrollGroups();
    void AddGeomsToTable();
    void RebuildExcresTableLabels();
    void AddSimpleExcresToTable();
    void ClearGUIElements();
    void DeactivateRow( int index, int grayFlag );
    void ResizeDeviceVectors( int geomSize, int excresSize );
    void UpdateFinalValues();

    void GetTableFlags( int index, const string &lastID, const string &lastIncorporatedID, int lastShape,
                        int &skipFlag, int &subsurfFlag, int &grayFlag );
    string GetComponentTableLabel( int subsurfFlag, const string &lastID, int index );

    void SetupRowLabel( int subsurfFlag, const string &lastID, int index );
    void SetupSwet( int index, double swet );
    void SetupLref( int index, double lref );
    void SetupFineRat( int index, double finerat );
    void SetupFFType( int index, int shapetype, int eqnchoice );
    void SetupFFValue( int index, double formfactor, int eqnchoice );
    void SetupReyNum( int index, double reynum );
    void SetupRoughness( int index, double roughness );
    void SetupHeatTransfer( int index, double tetwratio, double tawtwratio );
    void SetupPercLaminar( int index, double perclam );
    void SetupCf( int index, double cf );
    void SetupQ( int index, double Q );
    void Setupf( int index, double f );
    void SetupCD( int index, double CD );
    void SetupPercCD( int index, double percCD );
    void SetupSubSurfInclude( int index, const string geomid, const string subsurfid );

    void SetupExcresType( int index, const string type );
    void SetupExcresInput( int index, double input );

    // Screen Variables
    vector < string > m_WingGeomVec;
    vector < string > m_GeomOrderInTableVec;

    // Tab Layouts
    GroupLayout m_OutputsLayout;
    GroupLayout m_ExcrescenceLayout;
    GroupLayout m_DocumentationLayout;

    //==== Outputs FLTK ====//
    // Outputs Layout
    GroupLayout m_OptionsLayout;

    //---- Geometry ----//
    Choice m_SetChoice;

    //---- Table Options ----//
    Choice m_LamCfEqnChoice;
    Choice m_TurbCfEqnChoice;

    //---- Reference Area ----//
    ToggleButton m_RefManualToggle;
    ToggleButton m_RefChoiceToggle;
    ToggleRadioGroup m_RefToggle;
    Choice m_RefWingChoice;
    SliderAdjRangeInput m_SrefSlider;
    TriggerButton m_SrefUnit;

    //---- Flow Parameters ----//
    Choice m_FreestreamTypeChoice;

    // Std. Atmos. + Delta Temp. Inputs
    SliderAdjRangeInput m_FlowParmVinfSlider;
    SliderAdjRangeInput m_FlowParmHinfSlider;
    SliderAdjRangeInput m_FlowParmReqLSlider;
    SliderAdjRangeInput m_FlowParmMachSlider;

    // Manual Inputs
    SliderAdjRangeInput m_FlowParmTempSlider;
    SliderAdjRangeInput m_FlowParmPresSlider;
    SliderAdjRangeInput m_FlowParmRhoSlider;
    SliderAdjRangeInput m_FlowParmSpecificHeatRatioSlider;
    SliderAdjRangeInput m_FlowParmDynaViscSlider;
    //Choice m_flowParmMedium;

    //---- Units ----//
    Choice m_ModelLengthUnitChoice;
    TriggerButton m_SrefUnitLabel;
    Choice m_VinfUnitChoice;
    Choice m_AltUnitChoice;
    Choice m_TempUnitChoice;
    Choice m_PresUnitChoice;
    TriggerButton m_RhoUnitLabel;
    TriggerButton m_DynViscUnitLabel;
    TriggerButton m_ReyqLUnitLabel;

    //==== Execute Layout ====//
    GroupLayout m_ExecuteLayout;
    TriggerButton m_calc;
    TriggerButton m_export;

    //==== Table and Constant View Layout ====//
    GroupLayout m_ComponentLabelLayout;
    GroupLayout m_MainTableLabelsLayout;
    GroupLayout m_ConstantTableLabelsLayout;
    GroupLayout m_TableCompNamesLayout;
    GroupLayout m_TableLayout;
    GroupLayout m_ConstantViewLayout;

    // Scroll Group
    Fl_Scroll* m_ComponentLabelScrollGroup;
    Fl_Scroll* m_MainTableLabelsScrollGroup;
    Fl_Scroll* m_ConstTableLabelsScrollGroup;
    Fl_Scroll* m_MainTableCompNamesScrollGroup;
    Fl_Scroll* m_MainTableScrollGroup;
    Fl_Scroll* m_ConstantTableScrollGroup;
    int m_CurrentRowIndex;

    // Table Layout Final Outputs
    StringOutput m_GeomfTotal;
    StringOutput m_GeomCdTotal;
    StringOutput m_GeomPercTotal;
    StringOutput m_ExcresfTotal;
    StringOutput m_ExcresCdTotal;
    StringOutput m_ExcresPercTotal;
    TriggerButton m_fTotalLabel;
    StringOutput m_fTotal;
    StringOutput m_CDTotal;
    StringOutput m_PercTotal;

    // Table Layout Variable Labels
    ToggleButton m_CompLabel;
    ToggleButton m_SwetUnitLabel;
    TriggerButton m_GroupedGeomLabel;
    TriggerButton m_LrefUnitLabel;
    TriggerButton m_FineRatLabel;
    TriggerButton m_FFEqnChoiceLabel;
    TriggerButton m_FFLabel;
    TriggerButton m_ReLabel;
    TriggerButton m_RoughnessLabel;
    TriggerButton m_TeTwLabel;
    TriggerButton m_TawTwLabel;
    TriggerButton m_PercLamLabel;
    TriggerButton m_CfLabel;
    TriggerButton m_QLabel;
    TriggerButton m_fLabel;
    TriggerButton m_CdLabel;
    ToggleButton m_PercTotalLabel;
    ToggleRadioGroup m_SortToggle;

    // Table Layout I/O Row Cells
    vector <TriggerButton> m_labels;
    vector <StringOutput> m_swet;
    vector <Choice> m_grouped;
    vector <StringOutput> m_lref;
    vector <StringOutput> m_Re;
    vector <StringInput> m_Roughness;
    vector <StringInput> m_TeTwRatio;
    vector <StringInput> m_TawTwRatio;
    vector <StringInput> m_percLam;
    vector <StringOutput> m_Cf;
    vector <StringOutput> m_fineRat;
    vector <Choice> m_ffType;
    vector <StringOutput> m_ffOut;
    vector <StringInput> m_ffIn;
    vector <StringInput> m_Q;
    vector <StringOutput> m_f;
    vector <StringOutput> m_cd;
    vector <StringOutput> m_percTotal;
    vector <Choice> m_subsurfinclude;

    //==== Excrescence Tab ====//
    GroupLayout m_ExcrescenceListLayout;
    GroupLayout m_ExcrescenceItemLayout;
    Fl_Browser* m_ExcresBrowser;

    // Excrescence Inputs
    StringInput m_excresNameInput;
    Choice m_excresTypeChoice;
    SliderAdjRangeInput m_excresInput;

    // Excres Labels
    TriggerButton m_excresNameLabel;
    TriggerButton m_excresTypeLabel;
    TriggerButton m_excresInputLabel;
    TriggerButton m_excresAmountLabel;

    // Excrescence Buttons
    TriggerButton m_excresAdd;
    TriggerButton m_excresDelete;

    // Simple Excrescence Table I/O Row Cells
    vector <StringOutput> m_excresType;
    vector <StringOutput> m_excresOutput;

    //==== Documentation Tab ====//
    TriggerButton m_EqnDocumentation;
    Fl_Text_Display* m_TextDisplay;
    Fl_Text_Buffer* m_TextBuffer;

    Fl_Callback_p m_LabelScrollbarCB;
    Fl_Callback_p m_MainScrollbarCB;
    Fl_Callback_p m_ConstantScrollbarCB;
    Fl_Callback_p m_HScrollbarCB;
};

#endif  // ParasiteDragScreen_H
