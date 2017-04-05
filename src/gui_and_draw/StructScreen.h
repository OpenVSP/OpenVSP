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

    virtual void FeaPartDispGroup( GroupLayout* group );
    virtual void FeaSubSurfDispGroup( GroupLayout* group );
    virtual void FeaPropertyDispGroup( GroupLayout* group );

    virtual void LoadGeomChoice();
    virtual void UpdateStructBrowser();
    virtual void UpdateFeaPartBrowser();
    virtual void UpdateDrawPartBrowser();
    virtual void UpdateFeaSubSurfBrowser();
    virtual void UpdateMainSurfChoice();
    virtual void UpdateFeaPartChoice();
    virtual void UpdateSubSurfChoice();
    virtual void UpdateFeaPropertyBrowser();
    virtual void UpdateFeaPropertyChoice();
    virtual void UpdateFeaMaterialBrowser();
    virtual void UpdateFeaMaterialChoice();

    virtual void UpdateFeaPartPropertyIndex( Choice* property_choice );
    virtual void UpdateCapPropertyIndex( Choice* property_choice );
    virtual void UpdateFeaSubSurfPropertyIndex( Choice* property_choice );

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
    virtual void UpdateDrawObjs( vector< DrawObj* > &draw_obj_vec );

    ProcessUtil* getFeaMeshProcess()
    {
        return &m_FeaMeshProcess;
    }

private:

    Fl_Text_Buffer m_TextBuffer;

    GroupLayout m_GlobalTabLayout;
    GroupLayout m_ConsoleLayout;
    GroupLayout m_BorderConsoleLayout;
    GroupLayout m_DisplayTabLayout;
    GroupLayout m_StructureTabLayout;
    GroupLayout m_StructGroup;
    GroupLayout m_StructWingGroup;
    GroupLayout m_StructGeneralGroup;
    GroupLayout m_PartTabLayout;
    GroupLayout* m_CurFeaPartDispGroup;
    GroupLayout m_PartGroup;
    GroupLayout m_FullDepthEditLayout;
    GroupLayout m_RibEditLayout;
    GroupLayout m_SparEditLayout;
    GroupLayout m_UpSkinEditLayout;
    GroupLayout m_LowSkinEditLayout;
    GroupLayout m_StiffenerPlaneEditLayout;
    GroupLayout m_StiffenerSubSurfEditLayout;
    GroupLayout m_FixPointEditLayout;
    GroupLayout m_SubSurfTabLayout;
    GroupLayout m_PropertyTabLayout;
    GroupLayout m_PropertyEditGroup;
    GroupLayout m_MaterialTabLayout;
    GroupLayout m_MaterialEditGroup;

    //===== Console Items =====//
    Fl_Text_Display *m_ConsoleDisplay;
    Fl_Text_Buffer *m_ConsoleBuffer;

    ProcessUtil m_FeaMeshProcess;
    ProcessUtil m_MonitorProcess;

    //===== Global Tab Items =====//
    SliderAdjRangeInput m_MaxEdgeLen;
    SliderAdjRangeInput m_MinEdgeLen;
    SliderAdjRangeInput m_MaxGap;
    SliderAdjRangeInput m_NumCircleSegments;
    SliderAdjRangeInput m_GrowthRatio;

    ToggleButton m_Rig3dGrowthLimit;
    ToggleButton m_IntersectSubsurfaces;

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

    //===== Draw Tab Items =====//
    ToggleButton m_DrawFeaPartsButton;
    ToggleButton m_DrawMeshButton;
    ToggleButton m_ColorTagsButton;
    ToggleButton m_ShowBadEdgeTriButton;

    Fl_Check_Browser * m_DrawPartSelectBrowser;

    ToggleButton m_DrawNodesToggle;
    ToggleButton m_DrawElementsToggle;

    TriggerButton m_DrawAllButton;
    TriggerButton m_HideAllButton;

    //===== Structure Tab Items =====//
    Choice m_GeomChoice;
    TriggerButton m_AddFeaStructButton;
    TriggerButton m_DelFeaStructButton;

    Fl_Browser * m_StructureSelectBrowser;

    StringInput m_FeaStructNameInput;

    Choice m_SkinPropertyChoice;

    // Wing Functions
    SliderAdjRangeInput m_NumEvenlySpacedRibsInput;
    TriggerButton m_AddEvenlySpacedRibsButton;
    TriggerButton m_OrientWingButton;

    // General Functions
    TriggerButton m_OrientFrontButton;
    TriggerButton m_OrientSideButton;
    TriggerButton m_OrientTopButton;

    //===== FeaPart Tab Items =====//

    Fl_Browser * m_FeaPartSelectBrowser;

    Choice m_FeaPartChoice;
    Choice m_SurfSel;
    StringInput m_FeaPartNameInput;

    TriggerButton m_AddFeaPartButton;
    TriggerButton m_DelFeaPartButton;

    //===== FullDepth =====//
    Choice m_FullDepthPropertyChoice;

    Choice m_FullDepthOrientationChoice;
    SliderAdjRangeInput m_FullDepthCenterLocSlider;
    SliderAdjRangeInput m_FullDepthThetaSlider;

    ToggleButton m_FullDepthCapToggle;
    Choice m_FullDepthCapPropertyChoice;

    //===== Rib =====//
    Choice m_RibPropertyChoice;

    Choice m_RibPerpendicularEdgeChoice;
    SliderAdjRangeInput m_RibPosSlider;
    //SliderAdjRangeInput m_RibAlphaSlider;
    SliderAdjRangeInput m_RibThetaSlider;

    ToggleButton m_RibCapToggle;
    //ToggleButton m_RibTrimButton;

    //SliderAdjRangeInput m_RibLengthScaleSlider;
    //SliderAdjRangeInput m_RibWidthScaleSlider;
    Choice m_RibCapPropertyChoice;

    //===== Spar =====//
    Choice m_SparPropertyChoice;

    SliderAdjRangeInput m_SparPosSlider;
    //SliderAdjRangeInput m_SparAlphaSlider;
    SliderAdjRangeInput m_SparThetaSlider;

    ToggleButton m_SparCapToggle;
    //ToggleButton m_SparTrimButton;

    //SliderAdjRangeInput m_SparLengthScaleSlider;
    //SliderAdjRangeInput m_SparWidthScaleSlider;
    Choice m_SparCapPropertyChoice;

    //===== StiffenerPlane =====//
    Choice m_StiffenerPlanePropertyChoice;

    Choice m_StiffenerOrientationChoice;
    SliderAdjRangeInput m_StiffenerCenterLocSlider;
    SliderAdjRangeInput m_StiffenerThetaSlider;

    //===== StiffenerSubSurf =====//
    Choice m_StiffenerSubSurfPropertyChoice;

    SliderInput m_StiffenerConstSlider; // Either Constant U or W
    ToggleButton m_StiffenerConstUButton;
    ToggleButton m_StiffenerConstVButton;
    ToggleRadioGroup m_StiffenerConstToggleGroup;

    //===== Fix Point =====//
    SliderAdjRangeInput m_FixPointULocSlider;
    SliderAdjRangeInput m_FixPointWLocSlider;

    //===== SubSurface Tab =====//
    GroupLayout* m_CurFeaSubDispGroup;
    Fl_Browser* m_FeaSubSurfBrowser;
    TriggerButton m_DelFeaSubSurfButton;
    TriggerButton m_AddFeaSubSurfButton;
    Choice m_FeaSubSurfChoice;

    GroupLayout m_FeaSSCommonGroup;
    StringInput m_FeaSubNameInput;

    // SS_Line
    GroupLayout m_FeaSSLineGroup;
    Choice m_FeaSSLinePropertyChoice;
    SliderInput m_FeaSSLineConstSlider; // Either Constant U or W
    ToggleButton m_FeaSSLineConstUButton;
    ToggleButton m_FeaSSLineConstWButton;
    ToggleRadioGroup m_FeaSSLineConstToggleGroup;

    ToggleButton m_FeaSSLineGreaterToggle;
    ToggleButton m_FeaSSLineLessToggle;
    ToggleRadioGroup m_FeaSSLineTestToggleGroup;

    // SS_Rectangle
    GroupLayout m_FeaSSRecGroup;
    Choice m_FeaSSRecPropertyChoice;
    SliderAdjRangeInput m_FeaSSRecCentUSlider;
    SliderAdjRangeInput m_FeaSSRecCentWSlider;
    SliderAdjRangeInput m_FeaSSRecULenSlider;
    SliderAdjRangeInput m_FeaSSRecWLenSlider;
    SliderAdjRangeInput m_FeaSSRecThetaSlider;
    ToggleButton m_FeaSSRecInsideButton;
    ToggleButton m_FeaSSRecOutsideButton;
    ToggleRadioGroup m_FeaSSRecTestToggleGroup;

    // SS_Ellipse
    GroupLayout m_FeaSSEllGroup;
    Choice m_FeaSSEllPropertyChoice;
    SliderAdjRangeInput m_FeaSSEllCentUSlider;
    SliderAdjRangeInput m_FeaSSEllCentWSlider;
    SliderAdjRangeInput m_FeaSSEllULenSlider;
    SliderAdjRangeInput m_FeaSSEllWLenSlider;
    SliderAdjRangeInput m_FeaSSEllTessSlider;
    SliderAdjRangeInput m_FeaSSEllThetaSlider;
    ToggleButton m_FeaSSEllInsideButton;
    ToggleButton m_FeaSSEllOutsideButton;
    ToggleRadioGroup m_FeaSSEllTestToggleGroup;

    // SS_Control
    GroupLayout m_FeaSSConGroup;
    Choice m_FeaSSConPropertyChoice;
    SliderAdjRangeInput m_FeaSSConUSSlider;
    SliderAdjRangeInput m_FeaSSConUESlider;
    SliderAdjRangeInput m_FeaSSConSFracSlider;
    SliderAdjRangeInput m_FeaSSConSLenSlider;
    SliderAdjRangeInput m_FeaSSConEFracSlider;
    SliderAdjRangeInput m_FeaSSConELenSlider;

    ToggleButton m_FeaSSConSAbsButton;
    ToggleButton m_FeaSSConSRelButton;
    ToggleRadioGroup m_FeaSSConSAbsRelToggleGroup;

    ToggleButton m_FeaSSConSEConstButton;

    ToggleButton m_FeaSSConLEFlagButton;

    ToggleButton m_FeaSSConInsideButton;
    ToggleButton m_FeaSSConOutsideButton;
    ToggleRadioGroup m_FeaSSConTestToggleGroup;
    Choice m_FeaSSConSurfTypeChoice;

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

    // Beam Property
    GroupLayout m_FeaPropertyBeamGroup;

    Choice m_FeaBeamMaterialChoice;
    SliderAdjRangeInput m_PropAreaSlider;
    SliderAdjRangeInput m_PropIzzSlider;
    SliderAdjRangeInput m_PropIyySlider;
    SliderAdjRangeInput m_PropIzySlider;
    SliderAdjRangeInput m_PropIxxSlider;

    //===== Material Tab =====//
    TriggerButton m_AddFeaMaterialButton;
    TriggerButton m_DelFeaMaterialButton;

    Fl_Browser* m_FeaMaterialSelectBrowser;

    GroupLayout m_MaterialEditSubGroup;
    StringInput m_FeaMaterialNameInput;

    SliderAdjRangeInput m_MatDensitySlider;
    SliderAdjRangeInput m_MatElasticModSlider;
    SliderAdjRangeInput m_MatPoissonSlider;
    //SliderAdjRangeInput m_MatShearModSlider;
    SliderAdjRangeInput m_MatThermalExCoeffSlider;

    //===== Common buttons =====//
    TriggerButton m_FeaMeshExportButton;

    //==== Private Variables ====//
    string m_SelectedGeomID;
    vector < string > m_GeomIDVec;
    vector < string > m_SubSurfIDVec;
    vector < string > m_DrawBrowserNameVec;
    vector < int > m_DrawBrowserPartIndexVec;

    int m_FeaCurrMainSurfIndx;
    int m_SelectedStructIndex;
    int m_SelectedPartIndex;
    int m_SelectedSubSurfIndex;
    int m_SelectedFeaPartChoice;
    int m_SelectedSubSurfChoice;
    int m_SelectedMaterialIndex;
    int m_SelectedPropertyIndex;
};

#endif
