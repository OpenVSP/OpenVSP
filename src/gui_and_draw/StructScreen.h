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
    virtual void UpdateFeaPropertyChoice();
    virtual void UpdateFeaMaterialBrowser();
    virtual void UpdateFeaMaterialChoice();

    virtual void UpdateGenPropertyIndex( Choice* property_choice );
    virtual void UpdateGenCapPropertyIndex( Choice* property_choice );

    virtual void UpdateUnitLabels();

    virtual void OrientWing();
    virtual void OrientStructure( VSPGraphic::Common::VSPenum type );

    virtual void Show();

    virtual void GuiDeviceCallBack( GuiDevice* device );

    virtual void CallBack( Fl_Widget* w );
    virtual void CloseCallBack( Fl_Widget *w );

    static void staticScreenCB( Fl_Widget *w, void* data )
    {
        ( ( StructScreen* )data )->CallBack( w );
    }

    virtual void AddOutputText( const string &text );

    string truncateFileName( const string &fn, int len );

    virtual bool Update();

    virtual void LoadDrawObjs( vector< DrawObj* > &draw_obj_vec );
    virtual void UpdateDrawObjs();

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
    GroupLayout m_PropertyTabLayout;
    GroupLayout m_PropertyEditGroup;
    GroupLayout m_MeshTabLayout;
    GroupLayout m_OutputTabLayout;
    GroupLayout m_DisplayTabLayout;

    GroupLayout* m_CurrDispGroup;

    //===== Console Items =====//
    Fl_Text_Display* m_ConsoleDisplay;
    Fl_Text_Buffer* m_ConsoleBuffer;

    ProcessUtil m_FeaMeshProcess;
    ProcessUtil m_MonitorProcess;

    //===== Common buttons =====//
    StringOutput m_CurrStructOutput;
    TriggerButton m_ResetDisplayButton;
    TriggerButton m_FeaMeshExportButton;

    //===== Structure Tab Items =====//
    TriggerButton m_WikiLinkButton;
    Choice m_StructUnitChoice;
    Choice m_GeomChoice;
    Choice m_SurfSel;
    TriggerButton m_AddFeaStructButton;
    TriggerButton m_DelFeaStructButton;

    Fl_Browser* m_StructureSelectBrowser;

    StringInput m_FeaStructNameInput;

    // Orientation
    TriggerButton m_OrientWingButton;

    TriggerButton m_OrientFrontButton;
    TriggerButton m_OrientSideButton;
    TriggerButton m_OrientTopButton;

    //===== FeaPart Tab Items =====//
    Fl_Browser* m_FeaPartSelectBrowser;

    TriggerButton m_MovePrtUpButton;
    TriggerButton m_MovePrtDownButton;
    TriggerButton m_MovePrtTopButton;
    TriggerButton m_MovePrtBotButton;

    Choice m_FeaPartChoice;
    StringInput m_FeaPartNameInput;
    TriggerButton m_EditFeaPartButton;
    TriggerButton m_AddFeaPartButton;
    TriggerButton m_DelFeaPartButton;
    ToggleButton m_ShellToggle;
    ToggleButton m_CapToggle;
    ToggleButton m_ShellCapToggle;
    ToggleRadioGroup m_ShellCapToggleGroup;

    Choice m_GenPropertyChoice;
    Choice m_GenCapPropertyChoice;

    ToggleButton m_ShowFeaPartButton;
    ToggleButton m_HideFeaPartButton;
    ToggleRadioGroup m_DispFeaPartGroup;

    //===== Material Tab =====//
    TriggerButton m_AddFeaMaterialButton;
    TriggerButton m_DelFeaMaterialButton;

    Fl_Browser* m_FeaMaterialSelectBrowser;

    GroupLayout m_MaterialEditSubGroup;
    StringInput m_FeaMaterialNameInput;

    SliderAdjRangeInput m_MatDensitySlider;
    TriggerButton m_MatDensityUnit;
    SliderAdjRangeInput m_MatElasticModSlider;
    TriggerButton m_MatElasticModUnit;
    SliderAdjRangeInput m_MatPoissonSlider;
    SliderAdjRangeInput m_MatThermalExCoeffSlider;
    TriggerButton m_MatThermalExCoeffUnit;

    //===== Property Tab =====//
    GroupLayout* m_CurFeaPropDispGroup;
    TriggerButton m_AddFeaPropertyButton;
    TriggerButton m_DelFeaPropertyButton;

    Fl_Browser* m_FeaPropertySelectBrowser;

    Choice m_FeaPropertyType;

    GroupLayout m_FeaPropertyCommonGroup;
    StringInput m_FeaPropertyNameInput;

    // Shell Property
    GroupLayout m_FeaPropertyShellGroup;

    Choice m_FeaShellMaterialChoice;
    SliderAdjRangeInput m_PropThickSlider;
    TriggerButton m_PropThickUnit;

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
    Choice m_FeaBeamXSecChoice;
    TriggerButton m_ShowFeaBeamXSecButton;
    // General XSec
    SliderAdjRangeInput m_PropAreaSlider;
    TriggerButton m_PropAreaUnit;
    SliderAdjRangeInput m_PropIzzSlider;
    TriggerButton m_PropIzzUnit;
    SliderAdjRangeInput m_PropIyySlider;
    TriggerButton m_PropIyyUnit;
    SliderAdjRangeInput m_PropIzySlider;
    TriggerButton m_PropIzyUnit;
    SliderAdjRangeInput m_PropIxxSlider;
    TriggerButton m_PropIxxUnit;
    // Circ XSec
    SliderAdjRangeInput m_CircDim1Slider;
    TriggerButton m_CircDim1Unit;
    // Pipe/Tube XSec
    SliderAdjRangeInput m_PipeDim1Slider;
    TriggerButton m_PipeDim1Unit;
    SliderAdjRangeInput m_PipeDim2Slider;
    TriggerButton m_PipeDim2Unit;
    // I XSec
    SliderAdjRangeInput m_IDim1Slider;
    TriggerButton m_IDim1Unit;
    SliderAdjRangeInput m_IDim2Slider;
    TriggerButton m_IDim2Unit;
    SliderAdjRangeInput m_IDim3Slider;
    TriggerButton m_IDim3Unit;
    SliderAdjRangeInput m_IDim4Slider;
    TriggerButton m_IDim4Unit;
    SliderAdjRangeInput m_IDim5Slider;
    TriggerButton m_IDim5Unit;
    SliderAdjRangeInput m_IDim6Slider;
    TriggerButton m_IDim6Unit;
    // Rect XSec
    SliderAdjRangeInput m_RectDim1Slider;
    TriggerButton m_RectDim1Unit;
    SliderAdjRangeInput m_RectDim2Slider;
    TriggerButton m_RectDim2Unit;
    // Box XSec
    SliderAdjRangeInput m_BoxDim1Slider;
    TriggerButton m_BoxDim1Unit;
    SliderAdjRangeInput m_BoxDim2Slider;
    TriggerButton m_BoxDim2Unit;
    SliderAdjRangeInput m_BoxDim3Slider;
    TriggerButton m_BoxDim3Unit;
    SliderAdjRangeInput m_BoxDim4Slider;
    TriggerButton m_BoxDim4Unit;

    //===== Mesh Tab Items =====//
    SliderAdjRangeInput m_MaxEdgeLen;
    SliderAdjRangeInput m_MinEdgeLen;
    SliderAdjRangeInput m_MaxGap;
    SliderAdjRangeInput m_NumCircleSegments;
    SliderAdjRangeInput m_GrowthRatio;

    ToggleButton m_Rig3dGrowthLimit;
    ToggleButton m_HalfMeshButton;

    //===== Output Items =====//
    ToggleButton m_StlFile;
    ToggleButton m_GmshFile;
    ToggleButton m_MassFile;
    ToggleButton m_NastFile;
    ToggleButton m_CalcFile;

    TriggerButton m_SelectStlFile;
    TriggerButton m_SelectGmshFile;
    TriggerButton m_SelectMassFile;
    TriggerButton m_SelectNastFile;
    TriggerButton m_SelectCalcFile;

    StringOutput m_StlOutput;
    StringOutput m_GmshOutput;
    StringOutput m_MassOutput;
    StringOutput m_NastOutput;
    StringOutput m_CalcOutput;

    ToggleButton m_SrfFile;
    ToggleButton m_XYZIntCurves;
    TriggerButton m_SelectSrfFile;
    StringOutput m_SrfOutput;

    ToggleButton m_CurvFile;
    ToggleButton m_Plot3DFile;

    TriggerButton m_SelectCurvFile;
    TriggerButton m_SelectPlot3DFile;

    StringOutput m_CurvOutput;
    StringOutput m_Plot3DOutput;

    ToggleButton m_ExportRaw;
    SliderAdjRangeInput m_ExportRelCurveTolSlider;

    //===== Draw Tab Items =====//
    ToggleButton m_DrawMeshButton;
    ToggleButton m_ColorElementsButton;
    ToggleButton m_DrawNodesToggle;
    ToggleButton m_DrawElementOrientVecToggle;

    ToggleButton m_DrawIsect;
    ToggleButton m_DrawBorder;

    ToggleButton m_ShowRaw;
    ToggleButton m_ShowBinAdapt;
    SliderAdjRangeInput m_DrawRelCurveTolSlider;

    ToggleButton m_ShowCurve;
    ToggleButton m_ShowPts;

    Fl_Check_Browser* m_DrawPartSelectBrowser;

    TriggerButton m_DrawAllButton;
    TriggerButton m_HideAllButton;

    //==== Private Variables ====//
    string m_SelectedGeomID;
    vector < string > m_GeomIDVec;
    vector < string > m_SubSurfIDVec;
    vector < string > m_FixPointParentIDVec;
    vector < string > m_PerpendicularEdgeVec;

    int m_FeaCurrMainSurfIndx;
    vector < int > m_SelectedPartIndexVec;
    int m_SelectedFeaPartChoice;
    int m_NumFeaPartChoices;
};

#endif
