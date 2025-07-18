//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//

// CfdMeshScreen.h: interface for the CfdMeshScreen class.
//
//////////////////////////////////////////////////////////////////////

#ifndef CFDMESHSCREEN_H
#define CFDMESHSCREEN_H

#include "ScreenMgr.h"
#include "ScreenBase.h"
#include "Vehicle.h"
#include "GuiDevice.h"
#include "ProcessUtil.h"

using namespace std;

class CfdMeshScreen : public TabScreen
{
public:
    CfdMeshScreen( ScreenMgr* mgr );
    virtual ~CfdMeshScreen();

    bool Update();
    void Show();
    void Hide();

    void CallBack( Fl_Widget *w );
    virtual void CloseCallBack( Fl_Widget *w );
    static void staticScreenCB( Fl_Widget *w, void* data )
    {
        ( ( CfdMeshScreen* )data )->CallBack( w );
    }

    virtual void GuiDeviceCallBack( GuiDevice* device );

    void AddOutputText( const string &text );
    void LoadDrawObjs( vector< DrawObj* > &draw_obj_vec );
    virtual bool GetVisBndBox( BndBox &bbox );

    ProcessUtil* getCfdMeshProcess()
    {
        return &m_CFDMeshProcess;
    }

protected:

    GroupLayout m_GlobalTabLayout;
    GroupLayout m_DisplayTabLayout;
    GroupLayout m_OutputTabLayout;
    GroupLayout m_SourcesTabLayout;
    GroupLayout m_DomainTabLayout;
    GroupLayout m_WakesTabLayout;
    GroupLayout m_ConsoleLayout;
    GroupLayout m_BorderConsoleLayout;

    Fl_Group* m_WakesTab;

    //===== Global Tab Items =====//

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
    ToggleButton m_IntersectSubsurfaces;

    TriggerButton m_GlobSrcAdjustLenLftLft;
    TriggerButton m_GlobSrcAdjustLenLft;
    TriggerButton m_GlobSrcAdjustLenRht;
    TriggerButton m_GlobSrcAdjustLenRhtRht;
    TriggerButton m_GlobSrcAdjustRadLftLft;
    TriggerButton m_GlobSrcAdjustRadLft;
    TriggerButton m_GlobSrcAdjustRadRht;
    TriggerButton m_GlobSrcAdjustRadRhtRht;

    ToggleButton m_SetToggle;
    Choice m_UseSet;
    Choice m_UseDegenSet;

    ToggleRadioGroup m_ModeSetToggleGroup;
    ToggleButton m_ModeToggle;
    Choice m_ModeChoice;
    vector < string > m_ModeIDs;

    ToggleButton m_ToCubicToggle;
    SliderAdjRangeInput m_ToCubicTolSlider;

    ToggleButton m_ConvertToQuadsToggle;

    //===== Display Tab Items =====//

    ToggleButton m_ShowSourcesAndWakePreview;
    ToggleButton m_ShowFarFieldPreview;
    ToggleButton m_ShowMesh;
    ToggleButton m_ShowWake;
    ToggleButton m_ShowSymmetryPlane;
    ToggleButton m_ShowFarField;
    ToggleButton m_ShowBadEdgesAndTriangles;
    ToggleButton m_ColorFaces;

    ToggleButton m_ColorByTag;
    ToggleButton m_ColorByReason;
    ToggleRadioGroup m_ColorByToggleGroup;

    ToggleButton m_DrawIsect;
    ToggleButton m_DrawBorder;

    ToggleButton m_ShowRaw;
    ToggleButton m_ShowBinAdapt;
    SliderAdjRangeInput m_RelCurveTolSlider;

    ToggleButton m_ShowCurve;
    ToggleButton m_ShowPts;


    TriggerButton m_MaxLenConstraintLabel;

    TriggerButton m_MinLenCurvGapLabel;
    TriggerButton m_CurvGapLabel;
    TriggerButton m_GrowLimitCurvGapLabel;

    TriggerButton m_MinLenCurvNCircSegLabel;
    TriggerButton m_NCircSegLabel;
    TriggerButton m_GrowLimitNCircSegLabel;

    TriggerButton m_SourcesLabel;
    TriggerButton m_GrowLimitSourcesLabel;

    //===== Output Tab Items =====//

    ToggleButton m_StlFile;
    ToggleButton m_TaggedMultiSolid;
    ToggleButton m_PolyFile;
    ToggleButton m_TriFile;
    ToggleButton m_FacFile;
    ToggleButton m_ObjFile;
    ToggleButton m_MshFile;
    ToggleButton m_DatFile;
    ToggleButton m_KeyFile;
    ToggleButton m_TkeyFile;
    ToggleButton m_VspgeomFile;

    TriggerButton m_SelectStlFile;
    TriggerButton m_SelectPolyFile;
    TriggerButton m_SelectTriFile;
    TriggerButton m_SelectFacFile;
    TriggerButton m_SelectObjFile;
    TriggerButton m_SelectMshFile;
    TriggerButton m_SelectDatFile;
    TriggerButton m_SelectKeyFile;
    TriggerButton m_SelectTkeyFile;
    TriggerButton m_SelectVspgeomFile;

    StringOutput m_StlOutput;
    StringOutput m_PolyOutput;
    StringOutput m_TriOutput;
    StringOutput m_FacOutput;
    StringOutput m_ObjOutput;
    StringOutput m_MshOutput;
    StringOutput m_DatOutput;
    StringOutput m_KeyOutput;
    StringOutput m_TkeyOutput;
    StringOutput m_VspgeomOutput;

    //===== Sources Tab Items =====//

    GroupLayout m_SourcesLeft;
    GroupLayout m_SourcesRight;
    GroupLayout m_UWPosition1;
    GroupLayout m_UWPosition2;

    Choice m_SourcesSelectComp;
    Choice m_SourcesSelectSurface;
    Choice m_SourcesType;

    TriggerButton m_AddDefaultSources;
    TriggerButton m_AddSource;
    TriggerButton m_DeleteSource;

    Fl_Browser* m_SourceBrowser;

    StringInput m_SourceName;

    SliderAdjRangeInput m_SourceU1;
    SliderAdjRangeInput m_SourceW1;
    SliderAdjRangeInput m_SourceRad1;
    SliderAdjRangeInput m_SourceLen1;

    SliderAdjRangeInput m_SourceU2;
    SliderAdjRangeInput m_SourceW2;
    SliderAdjRangeInput m_SourceRad2;
    SliderAdjRangeInput m_SourceLen2;

    //===== Domain Tab Items =====//

    GroupLayout m_FarParametersLayout;
    GroupLayout m_FarBoxLayout;
    GroupLayout m_FarXYZLocationLayout;

    ToggleButton m_GenerateHalfMesh;
    ToggleButton m_GenerateFarFieldMesh;

    ToggleButton m_DomainMaxAbsToggle;
    ToggleButton m_DomainMaxRelToggle;
    ToggleRadioGroup m_DomainMaxEdgeLenToggleGroup;

    ToggleButton m_DomainGapAbsToggle;
    ToggleButton m_DomainGapRelToggle;
    ToggleRadioGroup m_DomainMaxGapToggleGroup;

    SliderAdjRange2Input m_DomainMaxEdgeLen;
    SliderAdjRange2Input m_DomainMaxGap;
    SliderAdjRangeInput m_DomainNumCircleSegments;

    //Far Field Type Items
    RadioButton m_FarFieldTypeComponent;
    RadioButton m_FarFieldTypeBox;

    Choice m_ComponentChoice;

    //Symmetry Plane Splitting
    ToggleButton m_SymPlaneSplit;

    //Size
    ToggleButton m_DomainRel;
    ToggleButton m_DomainAbs;

    SliderAdjRange2Input m_DomainLength;
    SliderAdjRange2Input m_DomainWidth;
    SliderAdjRange2Input m_DomainHeight;

    //Location
    ToggleButton m_DomainCen;
    ToggleButton m_DomainMan;

    SliderAdjRangeInput m_DomainXLoc;
    SliderAdjRangeInput m_DomainYLoc;
    SliderAdjRangeInput m_DomainZLoc;

    //===== Wake Tab Items =====//

    SliderAdjRangeInput m_ScaleWake;
    SliderAdjRangeInput m_WakeAngle;

    Choice m_Comp;

    ToggleButton m_AddWake;

    //===== Console Items =====//

    Fl_Terminal *m_ConsoleDisplay;

    TriggerButton m_MeshAndExport;

    ProcessUtil m_CFDMeshProcess;
    ProcessUtil m_MonitorProcess;

private:

    void CreateGlobalTab();
    void CreateDisplayTab();
    void CreateOutputTab();
    void CreateSourcesTab();
    void CreateDomainTab();
    void CreateWakesTab();

    void UpdateGlobalTab();
    void UpdateDisplayTab();
    void UpdateOutputTab();
    void UpdateSourcesTab(BaseSource* source);
    void UpdateDomainTab();
    void UpdateWakesTab();

    void GuiDeviceGlobalTabCallback( GuiDevice* device );
    void GuiDeviceOutputTabCallback( GuiDevice* device );
    void GuiDeviceSourcesTabCallback( GuiDevice* device );
    void GuiDeviceDomainTabCallback( GuiDevice* device );
    void GuiDeviceWakesTabCallback( GuiDevice* device );

    Vehicle* m_Vehicle;

    vector< string > m_GeomVec;
    vector< string > m_WingGeomVec;

};

#endif //CFDMESHSCREEN_H
