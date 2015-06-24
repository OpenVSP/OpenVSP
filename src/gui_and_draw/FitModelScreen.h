//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//

// FitModelScreen.h: interface for the fit model menu.
//
//////////////////////////////////////////////////////////////////////

#ifndef FITMODELSCREEN_H
#define FITMODELSCREEN_H

#include "ScreenMgr.h"
#include "ScreenBase.h"
#include "Vehicle.h"
#include "GuiDevice.h"
#include "FitModelMgr.h"

using namespace std;

class FitModelScreen : public TabScreen
{
public:
    FitModelScreen( ScreenMgr* mgr );
    virtual ~FitModelScreen();

    bool Update();
    virtual void RebuildAdjustTab();

    void Show();
    void Hide();

    void CallBack( Fl_Widget *w );
    static void staticScreenCB( Fl_Widget *w, void* data )
    {
        ( ( FitModelScreen* )data )->CallBack( w );
    }

    void CloseCallBack( Fl_Widget *w );

    virtual void GuiDeviceCallBack( GuiDevice* device );

    bool IsVisible();

    virtual void LoadDrawObjs( vector< DrawObj* > & draw_obj_vec );

protected:

    int m_NVarLast;

    // Pts tab variables

    GroupLayout m_PickPtsLayout;

    StringOutput m_NSelOutput;

    ToggleButton m_SelOneButton;
    ToggleButton m_SelBoxButton;
    TriggerButton m_SelAllButton;
    TriggerButton m_SelNoneButton;
    TriggerButton m_SelInvButton;

    TriggerButton m_HideSelButton;
    TriggerButton m_HideUnselButton;
    TriggerButton m_HideAllButton;
    TriggerButton m_HideNoneButton;
    TriggerButton m_HideInvButton;

    Fl_Browser* m_TargetPtBrowser;
    GeomPicker m_TargetGeomPicker;

    ToggleButton m_UFixButton;
    ToggleButton m_UFreeButton;
    ToggleRadioGroup m_UToggleGroup;
    ToggleButton m_WFixButton;
    ToggleButton m_WFreeButton;
    ToggleRadioGroup m_WToggleGroup;
    SliderInput m_USlider;
    SliderInput m_WSlider;

    TriggerButton m_AddTargetPtButton;
    TriggerButton m_DelTargetPtButton;
    TriggerButton m_ClearTargetPtButton;

    StringOutput m_NTgtOutput;

    // Var tab variables
    GroupLayout m_PickVarLayout;

    ParmPicker m_ParmPicker;

    TriggerButton m_AddVarButton;
    TriggerButton m_DelVarButton;
    TriggerButton m_ClearVarButton;

    StringOutput m_NVarOutput;

    Vsp_Browser* varBrowser;

    // Var tree tab variables
    GroupLayout m_PickTreeLayout;

    ParmTreePicker m_ParmTreePicker;
    TriggerButton m_ClearVarButton2;

    // Fit tab variables
    GroupLayout m_FitModelLayout;

    Fl_Scroll* m_FitScrollGroup;
    GroupLayout m_FitVarScrollLayout;

    vector < SliderAdjRangeInput > m_ParmSliderVec;

    // Optimizer layout
    GroupLayout m_OptimLayout;

    StringOutput m_DOFOutput;
    StringOutput m_CondOutput;
    TriggerButton m_SearchUWButton;
    TriggerButton m_RefineUWButton;
    TriggerButton m_UpdateDistButton;
    TriggerButton m_OptimizeButton;
    StringOutput m_DistOutput;

    vector< DrawObj > m_PickList;

};

#endif
