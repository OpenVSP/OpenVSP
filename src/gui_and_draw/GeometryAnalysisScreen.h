//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//

// Create Edit Geometry Analyses
// Rob McDonald
//
//////////////////////////////////////////////////////////////////////

#if !defined(VSPTYPEGEOMETRYANALYSISSCREEN__INCLUDED_)
#define VSPTYPEGEOMETRYANALYSISSCREEN__INCLUDED_

#include "ScreenBase.h"
#include "GuiDevice.h"
#include "GeometryAnalysisMgr.h"

#include <FL/Fl.H>
#include <FL/Fl_Text_Buffer.H>

using std::string;
using std::vector;

class GeometryAnalysisScreen : public BasicScreen
{
public:
    GeometryAnalysisScreen( ScreenMgr* mgr );
    virtual ~GeometryAnalysisScreen();
    void Show();
    void Hide();
    bool Update();
    void UpdateGeometryAnalysisBrowser();
    void UpdateWindowSubSurfBrowser();
    void LoadDrawObjs( vector< DrawObj* > & draw_obj_vec );
    void MarkDOChanged();
    bool GetVisBndBox( BndBox &bbox );

    void CallBack( Fl_Widget *w );
    static void staticScreenCB( Fl_Widget *w, void* data )
    {
        ( static_cast <GeometryAnalysisScreen*> ( data ) )->CallBack( w );
    }

    void GuiDeviceCallBack( GuiDevice* gui_device );


protected:

    GroupLayout* m_OptionsCurrDisplayGroup;
    void OptionsDisplayGroup( GroupLayout* group );


    GroupLayout m_GenLayout;
    GroupLayout m_BorderLayout;
    GroupLayout m_GCaseLayout;
    GroupLayout m_PrimaryLayout;
    GroupLayout m_SecondaryLayout;
    GroupLayout m_OptionsLayout;

    GroupLayout m_RotateOptionsLayout;
    GroupLayout m_VisibilityOptionsLayout;
    GroupLayout m_MotionOptionsLayout;
    GroupLayout m_LookAtVisibilityOptionsLayout;

    ColResizeBrowser* m_GeometryAnalysisBrowser;
    int m_GeometryBrowserSelect;


    TriggerButton m_AddGeometryAnalysis;
    TriggerButton m_DelGeometryAnalysis;
    TriggerButton m_DelAllGeometryAnalyses;

    TriggerButton m_ShowBoth;
    TriggerButton m_ShowOnlyBoth;

    TriggerButton m_EvaluateAllGeometryAnalyses;

    StringInput m_GANameInput;

    Choice m_GeometryAnalysisTypeChoice;

    ToggleRadioGroup m_PrimaryToggleGroup;
    ToggleButton m_PrimarySetToggle;
    Choice m_PrimarySetChoice;

    ToggleButton m_PrimaryModeToggle;
    Choice m_PrimaryModeChoice;
    vector < string > m_ModeIDs;

    ToggleButton m_PrimaryGeomToggle;
    GeomPicker m_PrimaryGeomPicker;
    vector < string > m_GeomIDs;

    ToggleRadioGroup m_SecondaryToggleGroup;
    ToggleButton m_SecondarySetToggle;
    Choice m_SecondarySetChoice;

    ToggleButton m_SecondaryGeomToggle;
    GeomPicker m_SecondaryGeomPicker;

    ToggleButton m_SecondaryUseZGroundToggle;
    SliderAdjRangeInput m_SecondaryZGroundSlider;

    ToggleButton m_CCWToggle;
    ToggleButton m_CWToggle;
    ToggleRadioGroup m_CCWToggleGroup;

    ToggleButton m_PolyVisibleToggle;
    ToggleButton m_PolyOccludedToggle;
    ToggleRadioGroup m_PolyVisibleToggleGroup;

    Fl_Check_Browser* m_SubSurfCutoutBrowser;

    ToggleButton m_SecondaryUsePointToggle;
    SliderAdjRangeInput m_SecondaryXSlider;
    SliderAdjRangeInput m_SecondaryYSlider;
    SliderAdjRangeInput m_SecondaryZSlider;

    Choice m_ExtentChoice;
    SliderAdjRangeInput m_DispXSlider;
    SliderAdjRangeInput m_DispYSlider;
    SliderAdjRangeInput m_DispZSlider;

    ToggleButton m_SymmRotXToggle;
    ToggleButton m_SymmRotYToggle;
    ToggleButton m_SymmRotZToggle;

    SliderAdjRangeInput m_RotXpSlider;
    SliderAdjRangeInput m_RotYpSlider;
    SliderAdjRangeInput m_RotZpSlider;

    SliderAdjRangeInput m_RotXnSlider;
    SliderAdjRangeInput m_RotYnSlider;
    SliderAdjRangeInput m_RotZnSlider;

    SliderAdjRangeInput m_AzimuthSlider;
    SliderAdjRangeInput m_ElevationSlider;
    SliderAdjRangeInput m_N2RefractionIndexSlider;

    TriggerButton m_Evaluate;
    TriggerButton m_ShowResultsViewer;

    TriggerButton m_ApplyRotation;

    Output m_ResultOutput;

};


#endif
