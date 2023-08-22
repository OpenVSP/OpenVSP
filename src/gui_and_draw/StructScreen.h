//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//

// StructScreen.h:
// Rob McDonald
//////////////////////////////////////////////////////////////////////

#if !defined(VSP_STRUCTSCREEN_H_INCLUDED_)
#define VSP_STRUCTSCREEN_H_INCLUDED_

#include "ScreenBase.h"
#include "GuiDevice.h"
#include "Vehicle.h"
#include "ScreenMgr.h"
#include "FeaStructure.h"
#include "ProcessUtil.h"
#include "MainVSPScreen.h"
#include "MainGLWindow.h"

using namespace std;

class StructScreen : public TabScreen
{
public:
    StructScreen( ScreenMgr* mgr );
    virtual ~StructScreen();

    virtual void FeaStructDispGroup( GroupLayout* group );
    virtual void FeaPropertyDispGroup( GroupLayout* group );
    virtual void BeamXSecDispGroup( GroupLayout* group );

    virtual void LoadGeomChoice();
    virtual void UpdateStructBrowser();
    virtual void UpdateFeaPartBrowser();
    virtual void UpdateDrawPartBrowser();
    virtual void UpdateMainSurfChoice();
    virtual void UpdateFeaPartChoice();
    virtual void UpdateFeaPropertyBrowser();
    virtual void UpdateFeaMaterialBrowser();
    virtual void UpdateFeaMaterialChoice();
    virtual void UpdateFeaBCBrowser();
    virtual void UpdateBCPartChoice();
    virtual void UpdateBCSubSurfChoice();

    virtual void UpdateLayerBrowser();

    virtual void UpdateUnitLabels();

    virtual void OrientWing();
    virtual void OrientStructure( VSPGraphic::Common::VSPenum type );

    virtual void Show();
    virtual void LaunchFEAMesh();

    virtual void GuiDeviceCallBack( GuiDevice* device );

    virtual void CallBack( Fl_Widget* w );
    virtual void CloseCallBack( Fl_Widget *w );

    static void staticScreenCB( Fl_Widget *w, void* data )
    {
        ( ( StructScreen* )data )->CallBack( w );
    }

    virtual void AddOutputText( const string &text );

    virtual bool Update();

    virtual void LoadDrawObjs( vector< DrawObj* > &draw_obj_vec );
    virtual bool GetVisBndBox( BndBox &bbox );

    ProcessUtil* getFeaMeshProcess()
    {
        return &m_FeaMeshProcess;
    }

private:

    GroupLayout m_ConsoleLayout;
    GroupLayout m_BorderConsoleLayout;
    GroupLayout m_StructureTabLayout;
    GroupLayout m_StructGroup;
    GroupLayout m_StructWingGroup;
    GroupLayout m_StructGeneralGroup;
    GroupLayout m_PartTabLayout;
    GroupLayout m_PartGroup;
    GroupLayout m_MovePartButtonLayout;
    GroupLayout m_FeaPartBrowserLayout;
    GroupLayout m_MaterialTabLayout;
    GroupLayout m_MaterialEditGroup;
    GroupLayout m_LaminateTabLayout;
    GroupLayout m_PropertyTabLayout;
    GroupLayout m_PropertyEditGroup;
    GroupLayout m_BCTabLayout;
    GroupLayout m_BCEditGroup;
    GroupLayout m_MeshTabLayout;
    GroupLayout m_CadTabLayout;
    GroupLayout m_FemTabLayout;

    GroupLayout* m_CurrDispGroup;

    //===== Console Items =====//
    Fl_Text_Display* m_ConsoleDisplay;
    Fl_Text_Buffer* m_ConsoleBuffer;

    ProcessUtil m_FeaMeshProcess;

    //===== Common buttons =====//
    Choice m_CurrFeaMeshChoice;

    TriggerButton m_ResetPartDisplayButton;
    TriggerButton m_ResetMeshDisplayButton;

    TriggerButton m_FeaIntersectMeshButton;
    TriggerButton m_FeaExportFEMButton;

    TriggerButton m_IntersectOnlyButton;
    TriggerButton m_ExportCADButton;

    //===== Structure Tab Items =====//
    TriggerButton m_WikiLinkButton;
    Choice m_ModelUnitChoice;
    Choice m_StructUnitChoice;
    Choice m_GeomChoice;
    Choice m_SurfSel;
    TriggerButton m_AddFeaStructButton;
    TriggerButton m_DelFeaStructButton;

    ColResizeBrowser* m_StructureSelectBrowser;
    vector < string > m_StructIDs;

    StringInput m_FeaStructNameInput;

    // Orientation
    TriggerButton m_OrientWingButton;

    TriggerButton m_OrientFrontButton;
    TriggerButton m_OrientSideButton;
    TriggerButton m_OrientTopButton;

    //===== FeaPart Tab Items =====//
    ColResizeBrowser* m_FeaPartSelectBrowser;

    TriggerButton m_MovePrtUpButton;
    TriggerButton m_MovePrtDownButton;
    TriggerButton m_MovePrtTopButton;
    TriggerButton m_MovePrtBotButton;

    Choice m_FeaPartChoice;
    StringInput m_FeaPartNameInput;
    TriggerButton m_EditFeaPartButton;
    TriggerButton m_AddFeaPartButton;
    TriggerButton m_DelFeaPartButton;

    ToggleButton m_ShowFeaPartButton;
    ToggleButton m_HideFeaPartButton;
    ToggleRadioGroup m_DispFeaPartGroup;

    //===== Material Tab =====//
    TriggerButton m_AddFeaMaterialButton;
    TriggerButton m_DelFeaMaterialButton;

    TriggerButton m_EditLaminateButton;
    TriggerButton m_EditLaminaButton;

    Fl_Browser* m_FeaMaterialSelectBrowser;
    vector < string > m_FeaMaterialIDVec;

    GroupLayout m_MaterialEditSubGroup;

    Choice m_FeaMaterialTypeChoice;

    StringInput m_FeaMaterialNameInput;
    StringInput m_FeaMaterialDescriptionInput;

    GroupLayout m_IsoSubGroup;
    GroupLayout m_OrthoSubGroup;
    GroupLayout m_LaminateSubGroup;

    Input m_MatDensityInput;
    Choice m_IsoMatDensityUnitChoice;
    Output m_MatDensity_FEMOutput;
    TriggerButton m_IsoMatDensityUnit_FEM;

    Input m_MatElasticModInput;
    Choice m_IsoMatElasticModUnitChoice;
    Output m_MatElasticMod_FEMOutput;
    TriggerButton m_IsoMatElasticModUnit_FEM;

    Input m_MatPoissonInput;

    Input m_MatThermalExCoeffInput;
    Choice m_IsoMatThermalExCoeffUnitChoice;
    Output m_MatThermalExCoeff_FEMOutput;
    TriggerButton m_IsoMatThermalExCoeffUnit_FEM;

    StringOutput m_MatShearModOutput;
    TriggerButton m_IsoMatShearModUnit;
    StringOutput m_MatShearMod_FEMOutput;
    TriggerButton m_IsoMatShearModUnit_FEM;

    Input m_MatE1Input;
    Input m_MatE2Input;
    Input m_MatE3Input;
    Input m_Matnu12Input;
    Input m_Matnu13Input;
    Input m_Matnu23Input;
    Input m_MatG12Input;
    Input m_MatG13Input;
    Input m_MatG23Input;
    Input m_MatA1Input;
    Input m_MatA2Input;
    Input m_MatA3Input;

    Output m_MatE1Output_FEM;
    Output m_MatE2Output_FEM;
    Output m_MatE3Output_FEM;
    Output m_MatG12Output_FEM;
    Output m_MatG13Output_FEM;
    Output m_MatG23Output_FEM;
    Output m_MatA1Output_FEM;
    Output m_MatA2Output_FEM;
    Output m_MatA3Output_FEM;

    Input m_OrthoMatDensityInput;
    Output m_OrthoMatDensity_FEMOutput;
    Choice m_OrthoMatDensityUnitChoice;
    TriggerButton m_OrthoMatDensityUnit_FEM;

    Choice m_OrthoMatElasticModUnitChoice;
    TriggerButton m_OrthoMatElasticModUnit_FEM;

    Choice m_OrthoMatThermalExCoeffUnitChoice;
    TriggerButton m_OrthoMatThermalExCoeffUnit_FEM;

    TriggerButton m_OrthoMatShearModUnit;
    TriggerButton m_OrthoMatShearModUnit_FEM;


    Choice m_LaminateLengthUnitChoice;

    Output m_LaminateMatDensity_FEMOutput;
    Output m_LaminateMatThickness_FEMOutput;

    Output m_LaminateMatE1Output_FEM;
    Output m_LaminateMatE2Output_FEM;
    Output m_LaminateMatE3Output_FEM;
    Output m_LaminateMatnu12Output;
    Output m_LaminateMatnu13Output;
    Output m_LaminateMatnu23Output;
    Output m_LaminateMatG12Output_FEM;
    Output m_LaminateMatG13Output_FEM;
    Output m_LaminateMatG23Output_FEM;
    Output m_LaminateMatA1Output_FEM;
    Output m_LaminateMatA2Output_FEM;
    Output m_LaminateMatA3Output_FEM;

    TriggerButton m_LaminateMatDensityUnit_FEM;
    TriggerButton m_LaminateThicknessUnit_FEM;

    TriggerButton m_LaminateMatElasticModUnit_FEM;

    TriggerButton m_LaminateMatThermalExCoeffUnit_FEM;

    TriggerButton m_LaminateMatShearModUnit_FEM;

    //===== Laminate Tab =====//

    Choice m_LaminateChoice;

    StringInput m_LaminateNameInput;
    StringInput m_LaminateDescriptionInput;

    ColResizeBrowser* m_FeaLayerSelectBrowser;

    TriggerButton m_AddFeaLayerToLaminateButton;
    TriggerButton m_RemoveFeaLayerFromLaminateButton;

    Choice m_LayerChoice;

    Input m_LayerThickInput;
    TriggerButton m_LayerThickUnit;
    Output m_LayerThick_FEMOutput;
    TriggerButton m_LayerThickUnit_FEM;

    Input m_LayerThetaInput;

    //===== Property Tab =====//
    GroupLayout* m_CurFeaPropDispGroup;
    TriggerButton m_AddFeaPropertyButton;
    TriggerButton m_DelFeaPropertyButton;

    ColResizeBrowser* m_FeaPropertySelectBrowser;

    Choice m_FeaPropertyType;

    GroupLayout m_FeaPropertyCommonGroup;
    StringInput m_FeaPropertyNameInput;

    // Shell Property
    GroupLayout m_FeaPropertyShellGroup;

    Choice m_FeaShellMaterialChoice;
    Choice m_FeaShellLengthUnitChoice;

    Input m_PropThickInput;
    TriggerButton m_PropThickUnit;

    Output m_PropThick_FEMOutput;
    TriggerButton m_PropThickUnit_FEM;

    // Beam Property
    GroupLayout m_FeaPropertyBeamGroup;
    GroupLayout* m_CurBeamXSecDispGroup;
    GroupLayout m_GenXSecGroup;
    GroupLayout m_CircXSecGroup;
    GroupLayout m_PipeXSecGroup;
    GroupLayout m_IXSecGroup;
    GroupLayout m_RectXSecGroup;
    GroupLayout m_BoxXSecGroup;

    Choice m_FeaBeamMaterialChoice;
    Choice m_FeaBeamLengthUnitChoice;
    Choice m_FeaBeamXSecChoice;
    TriggerButton m_ShowFeaBeamXSecButton;
    // General XSec
    Input m_PropAreaInput;
    TriggerButton m_PropAreaUnit;
    Input m_PropIzzInput;
    TriggerButton m_PropIzzUnit;
    Input m_PropIyyInput;
    TriggerButton m_PropIyyUnit;
    Input m_PropIzyInput;
    TriggerButton m_PropIzyUnit;
    Input m_PropIxxInput;
    TriggerButton m_PropIxxUnit;

    Output m_PropArea_FEMOutput;
    TriggerButton m_PropAreaUnit_FEM;
    Output m_PropIzz_FEMOutput;
    TriggerButton m_PropIzzUnit_FEM;
    Output m_PropIyy_FEMOutput;
    TriggerButton m_PropIyyUnit_FEM;
    Output m_PropIzy_FEMOutput;
    TriggerButton m_PropIzyUnit_FEM;
    Output m_PropIxx_FEMOutput;
    TriggerButton m_PropIxxUnit_FEM;

    // Circ XSec
    Input m_CircDim1Input;
    TriggerButton m_CircDim1Unit;

    Output m_CircDim1_FEMOutput;
    TriggerButton m_CircDim1Unit_FEM;

    // Pipe/Tube XSec
    Input m_PipeDim1Input;
    TriggerButton m_PipeDim1Unit;
    Input m_PipeDim2Input;
    TriggerButton m_PipeDim2Unit;

    Output m_PipeDim1_FEMOutput;
    TriggerButton m_PipeDim1Unit_FEM;
    Output m_PipeDim2_FEMOutput;
    TriggerButton m_PipeDim2Unit_FEM;

    // I XSec
    Input m_IDim1Input;
    TriggerButton m_IDim1Unit;
    Input m_IDim2Input;
    TriggerButton m_IDim2Unit;
    Input m_IDim3Input;
    TriggerButton m_IDim3Unit;
    Input m_IDim4Input;
    TriggerButton m_IDim4Unit;
    Input m_IDim5Input;
    TriggerButton m_IDim5Unit;
    Input m_IDim6Input;
    TriggerButton m_IDim6Unit;

    Output m_IDim1_FEMOutput;
    TriggerButton m_IDim1Unit_FEM;
    Output m_IDim2_FEMOutput;
    TriggerButton m_IDim2Unit_FEM;
    Output m_IDim3_FEMOutput;
    TriggerButton m_IDim3Unit_FEM;
    Output m_IDim4_FEMOutput;
    TriggerButton m_IDim4Unit_FEM;
    Output m_IDim5_FEMOutput;
    TriggerButton m_IDim5Unit_FEM;
    Output m_IDim6_FEMOutput;
    TriggerButton m_IDim6Unit_FEM;

    // Rect XSec
    Input m_RectDim1Input;
    TriggerButton m_RectDim1Unit;
    Input m_RectDim2Input;
    TriggerButton m_RectDim2Unit;

    Output m_RectDim1_FEMOutput;
    TriggerButton m_RectDim1Unit_FEM;
    Output m_RectDim2_FEMOutput;
    TriggerButton m_RectDim2Unit_FEM;

    // Box XSec
    Input m_BoxDim1Input;
    TriggerButton m_BoxDim1Unit;
    Input m_BoxDim2Input;
    TriggerButton m_BoxDim2Unit;
    Input m_BoxDim3Input;
    TriggerButton m_BoxDim3Unit;
    Input m_BoxDim4Input;
    TriggerButton m_BoxDim4Unit;

    Output m_BoxDim1_FEMOutput;
    TriggerButton m_BoxDim1Unit_FEM;
    Output m_BoxDim2_FEMOutput;
    TriggerButton m_BoxDim2Unit_FEM;
    Output m_BoxDim3_FEMOutput;
    TriggerButton m_BoxDim3Unit_FEM;
    Output m_BoxDim4_FEMOutput;
    TriggerButton m_BoxDim4Unit_FEM;

    //===== BC Tab Items =====//
    ColResizeBrowser* m_FeaBCSelectBrowser;

    TriggerButton m_AddFeaBCButton;
    TriggerButton m_DelFeaBCButton;

    Choice m_FeaBCTypeChoice;

    GroupLayout m_FeaBCCommonGroup;

    Choice m_FeaBCModeChoice;

    CheckButtonBit m_TxButton;
    CheckButtonBit m_TyButton;
    CheckButtonBit m_TzButton;
    CheckButtonBit m_RxButton;
    CheckButtonBit m_RyButton;
    CheckButtonBit m_RzButton;

    Choice m_FeaBCPartChoice;
    vector < string > m_FeaBCPartChoiceIDVec;
    int m_SelectedBCPartChoice;

    Choice m_FeaBCSubSurfChoice;
    vector < string > m_FeaBCSubSurfChoiceIDVec;
    int m_SelectedBCSubSurfChoice;

    ToggleButton m_XLTFlagButton;
    ToggleButton m_XGTFlagButton;

    ToggleButton m_YLTFlagButton;
    ToggleButton m_YGTFlagButton;

    ToggleButton m_ZLTFlagButton;
    ToggleButton m_ZGTFlagButton;

    SliderAdjRangeInput m_XLTValSlider;
    SliderAdjRangeInput m_XGTValSlider;

    SliderAdjRangeInput m_YLTValSlider;
    SliderAdjRangeInput m_YGTValSlider;

    SliderAdjRangeInput m_ZLTValSlider;
    SliderAdjRangeInput m_ZGTValSlider;

    //===== Mesh Tab Items =====//

    ToggleButton m_MaxAbsToggle;
    ToggleButton m_MaxRelToggle;
    ToggleRadioGroup m_MaxEdgeLenToggleGroup;

    ToggleButton m_MinAbsToggle;
    ToggleButton m_MinRelToggle;
    ToggleRadioGroup m_MinEdgeLenToggleGroup;

    ToggleButton m_GapAbsToggle;
    ToggleButton m_GapRelToggle;
    ToggleRadioGroup m_MaxGapToggleGroup;

    SliderAdjRange2Input m_MaxEdgeLen;
    SliderAdjRange2Input m_MinEdgeLen;
    SliderAdjRange2Input m_MaxGap;
    SliderAdjRangeInput m_NumCircleSegments;
    SliderAdjRangeInput m_GrowthRatio;

    ToggleButton m_Rig3dGrowthLimit;
    ToggleButton m_HalfMeshButton;
    ToggleButton m_ToCubicToggle;
    SliderAdjRangeInput m_ToCubicTolSlider;

    ToggleButton m_ConvertToQuadsToggle;
    ToggleButton m_HighOrderElementToggle;
    ToggleButton m_BeamPerElementNormalToggle;

    SliderAdjRangeInput m_NodeOffset;
    SliderAdjRangeInput m_ElementOffset;

    //===== FEM Items =====//
    ToggleButton m_StlFile;
    ToggleButton m_GmshFile;
    ToggleButton m_MassFile;
    ToggleButton m_NastFile;
    ToggleButton m_NkeyFile;
    ToggleButton m_CalcFile;

    TriggerButton m_SelectStlFile;
    TriggerButton m_SelectGmshFile;
    TriggerButton m_SelectMassFile;
    TriggerButton m_SelectNastFile;
    TriggerButton m_SelectNkeyFile;
    TriggerButton m_SelectCalcFile;

    StringOutput m_StlOutput;
    StringOutput m_GmshOutput;
    StringOutput m_MassOutput;
    StringOutput m_NastOutput;
    StringOutput m_NkeyOutput;
    StringOutput m_CalcOutput;

    ToggleButton m_DrawMeshButton;
    ToggleButton m_ColorElementsButton;

    ToggleButton m_ColorByTag;
    ToggleButton m_ColorByReason;
    ToggleRadioGroup m_ColorByToggleGroup;

    ToggleButton m_DrawNodesToggle;
    ToggleButton m_DrawBCNodesToggle;
    ToggleButton m_DrawElementOrientVecToggle;

    Fl_Check_Browser* m_DrawPartSelectBrowser;

    TriggerButton m_DrawAllButton;
    TriggerButton m_HideAllButton;

    TriggerButton m_MaxLenConstraintLabel;

    TriggerButton m_MinLenCurvGapLabel;
    TriggerButton m_CurvGapLabel;
    TriggerButton m_GrowLimitCurvGapLabel;

    TriggerButton m_MinLenCurvNCircSegLabel;
    TriggerButton m_NCircSegLabel;
    TriggerButton m_GrowLimitNCircSegLabel;

    TriggerButton m_SourcesLabel;
    TriggerButton m_GrowLimitSourcesLabel;

    //===== CAD Items =====//
    ToggleButton m_SrfFile;
    ToggleButton m_XYZIntCurves;
    TriggerButton m_SelectSrfFile;
    StringOutput m_SrfOutput;

    ToggleButton m_CurvFile;
    ToggleButton m_Plot3DFile;
    ToggleButton m_IGESFile;
    ToggleButton m_STEPFile;

    TriggerButton m_SelectCurvFile;
    TriggerButton m_SelectPlot3DFile;
    TriggerButton m_SelectIGESFile;
    TriggerButton m_SelectSTEPFile;

    StringOutput m_CurvOutput;
    StringOutput m_Plot3DOutput;
    StringOutput m_IGESOutput;
    StringOutput m_STEPOutput;

    ToggleButton m_ExportRaw;

    SliderAdjRangeInput m_STEPTolSlider;
    //ToggleButton m_STEPMergePointsToggle;
    ToggleRadioGroup m_STEPRepGroup;
    ToggleButton m_STEPShell;
    ToggleButton m_STEPBREP;

    Choice m_LenUnitChoice;
    ToggleButton m_LabelIDToggle;
    ToggleButton m_LabelNameToggle;
    ToggleButton m_LabelSurfNoToggle;
    ToggleButton m_LabelSplitNoToggle;
    Choice m_LabelDelimChoice;

    ToggleButton m_DrawIsect;
    ToggleButton m_DrawBorder;

    ToggleButton m_ShowRaw;
    ToggleButton m_ShowBinAdapt;
    SliderAdjRangeInput m_RelCurveTolSlider;

    ToggleButton m_ShowCurve;
    ToggleButton m_ShowPts;

    //==== Private Variables ====//
    string m_SelectedGeomID;
    vector < string > m_GeomIDVec;
    vector < string > m_SubSurfIDVec;
    vector < string > m_FixPointParentIDVec;
    vector < string > m_PerpendicularEdgeVec;

    int m_FeaCurrMainSurfIndx;
    vector < int > m_SelectedPartIndexVec;
    int m_SelectedFeaPartChoice;
    int m_FeaPartChoiceSubSurfOffset;
};

#endif
