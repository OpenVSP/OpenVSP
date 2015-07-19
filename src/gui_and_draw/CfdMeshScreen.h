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

using namespace std;

class CfdMeshScreen : public TabScreen
{
public:
    CfdMeshScreen( ScreenMgr* mgr );
    virtual ~CfdMeshScreen();

    bool Update();
    void Show();
    void Hide();

    void LoadSetChoice();

    void CallBack( Fl_Widget *w );
    virtual void CloseCallBack( Fl_Widget *w );
    static void staticScreenCB( Fl_Widget *w, void* data )
    {
        ( ( CfdMeshScreen* )data )->CallBack( w );
    }

    virtual void GuiDeviceCallBack( GuiDevice* device );

    void parm_changed( Parm* parm ) {}
    void AddOutputText( const string &text );
    string truncateFileName( const string &fn, int len );
    void LoadDrawObjs( vector< DrawObj* > &draw_obj_vec );

protected:

    GroupLayout m_GlobalTabLayout;
    GroupLayout m_DisplayTabLayout;
    GroupLayout m_OutputTabLayout;
    GroupLayout m_SourcesTabLayout;
    GroupLayout m_DomainTabLayout;
    GroupLayout m_WakesTabLayout;
    GroupLayout m_ConsoleLayout;
    GroupLayout m_BorderConsoleLayout;

    //===== Global Tab Items =====//

    SliderAdjRangeInput m_MaxEdgeLen;
    SliderAdjRangeInput m_MinEdgeLen;
    SliderAdjRangeInput m_MaxGap;
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

    Choice m_UseSet;

    //===== Display Tab Items =====//

    ToggleButton m_ShowSourcesAndWakePreview;
    ToggleButton m_ShowFarFieldPreview;
    ToggleButton m_ShowMesh;
    ToggleButton m_ShowWake;
    ToggleButton m_ShowSymmetryPlane;
    ToggleButton m_ShowFarField;
    ToggleButton m_ShowBadEdgesAndTriangles;
    ToggleButton m_ColorTags;

    //===== Output Tab Items =====//

    ToggleButton m_StlFile;
    ToggleButton m_TaggedMultiSolid;
    ToggleButton m_PolyFile;
    ToggleButton m_TriFile;
    ToggleButton m_ObjFile;
    ToggleButton m_MshFile;
    ToggleButton m_DatFile;
    ToggleButton m_KeyFile;
    ToggleButton m_SrfFile;
    ToggleButton m_TkeyFile;

    TriggerButton m_SelectStlFile;
    TriggerButton m_SelectPolyFile;
    TriggerButton m_SelectTriFile;
    TriggerButton m_SelectObjFile;
    TriggerButton m_SelectMshFile;
    TriggerButton m_SelectDatFile;
    TriggerButton m_SelectKeyFile;
    TriggerButton m_SelectSrfFile;
    TriggerButton m_SelectTkeyFile;

    StringOutput m_StlOutput;
    StringOutput m_PolyOutput;
    StringOutput m_TriOutput;
    StringOutput m_ObjOutput;
    StringOutput m_MshOutput;
    StringOutput m_DatOutput;
    StringOutput m_KeyOutput;
    StringOutput m_SrfOutput;
    StringOutput m_TkeyOutput;

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

    SliderInput m_SourceU1;
    SliderInput m_SourceW1;
    SliderAdjRangeInput m_SourceRad1;
    SliderAdjRangeInput m_SourceLen1;

    SliderInput m_SourceU2;
    SliderInput m_SourceW2;
    SliderAdjRangeInput m_SourceRad2;
    SliderAdjRangeInput m_SourceLen2;

    //===== Domain Tab Items =====//

    GroupLayout m_FarParametersLayout;
    GroupLayout m_FarBoxLayout;
    GroupLayout m_FarXYZLocationLayout;

    ToggleButton m_GenerateHalfMesh;
    ToggleButton m_GenerateFarFieldMesh;

    SliderAdjRangeInput m_DomainMaxEdgeLen;
    SliderAdjRangeInput m_DomainMaxGap;
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

    Fl_Text_Display *m_ConsoleDisplay;
    Fl_Text_Buffer *m_ConsoleBuffer;

    TriggerButton m_MeshAndExport;

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
    void GuiDeviceDisplayTabCallback( GuiDevice* device );
    void GuiDeviceOutputTabCallback( GuiDevice* device );
    void GuiDeviceSourcesTabCallback( GuiDevice* device );
    void GuiDeviceDomainTabCallback( GuiDevice* device );
    void GuiDeviceWakesTabCallback( GuiDevice* device );

    Vehicle* m_Vehicle;

    vector< string > m_GeomVec;
    vector< string > m_WingGeomVec;

};

#endif //CFDMESHSCREEN_H
