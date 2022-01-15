#pragma once
//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//

// CurveEditScreen.h: Based on OpenVSP v2 Curve Edit GUI and v3 propeller blade editor
// Justin Gravett, ESAero, 7/10/19
//////////////////////////////////////////////////////////////////////

#ifndef CURVE_EDIT_H
#define CURVE_EDIT_H

#include "ScreenMgr.h"
#include "ScreenBase.h"
#include "MainGLWindow.h"

class CurveEditScreen : public TabScreen
{
public:
    CurveEditScreen( ScreenMgr* mgr );
    virtual ~CurveEditScreen();

    virtual void Show();
    virtual bool Update();
    virtual void CloseCallBack( Fl_Widget* w );
    virtual void GuiDeviceCallBack( GuiDevice* d );

    XSecCurve* GetXSecCurve();

    void UpdateDrawObj();
    void LoadDrawObjs( vector< DrawObj* >& draw_obj_vec );

    void SetDeleteActive( bool flag )                   { m_DeleteActive = flag; }
    bool GetDeleteActive()                              { return m_DeleteActive; }

    void SetSplitActive( bool flag )                    { m_SplitActive = flag; }
    bool GetSplitActive()                               { return m_SplitActive; }

    void SetFreezeAxis( bool flag )                     { m_FreezeAxis = flag; }

    // Restricts the index selecttor from cycling through intermediate control 
    // points. If an intermediate control point is selected, the delete button
    // is deactivated
    void UpdateIndexSelector( int index, bool skip_intermediate = true );

private:

    void UpdateAxisLimits(); // Update axis display to match zoom, scale, pan, etc.
    void RedrawXYSliders( int num_pts, int curve_type );

    GroupLayout m_MainLayout;
    GroupLayout m_XSecLayout;
    GroupLayout m_DrawLayout;
    GroupLayout m_BackgroundImageLayout;
    GroupLayout m_PtLayout;

    // XSec editor area GUI elements
    Choice m_ViewChoice;
    VSPGUI::EditXSecWindow* m_XSecGlWin;
    Ca_X_Axis* m_XAxis;
    Ca_Y_Axis* m_YAxis;

    Fl_Scroll* m_PtScroll;

    // XSec Tab GUI Elements
    Choice m_ShapeChoice;
    TriggerButton m_InitShapeButton;
    ToggleButton m_SymToggle;
    ToggleButton m_ClosedCurveToggle;

    TriggerButton m_ReparameterizeButton;

    HWXSecCurveDriverGroup m_DefaultXSecDriverGroup;
    DriverGroupBank m_XSecDriverGroupBank;

    SliderAdjRangeInput m_DepthSlider;

    SliderAdjRangeInput m_SplitPtSlider;
    TriggerButton m_SplitButton;
    TriggerButton m_DelButton;
    ToggleButton m_DelPickButton;
    ToggleButton m_SplitPickButton;
    IndexSelector m_PntSelector;
    Choice m_ConvertChoice;
    TriggerButton m_ConvertButton;
    StringOutput m_CurveType;

    ToggleButton m_AbsDimToggle;

    // Draw Tab GUI Elements
    ColorPicker m_ColorPicker;
    SliderAdjRangeInput m_PointSizeSlider;
    SliderAdjRangeInput m_LineThicknessSlider;
    CheckButton m_PointColorCheck;
    SliderAdjRangeInput m_PointColorWheelSlider;

    ToggleButton m_BorderToggle;
    ToggleButton m_AxisToggle;
    ToggleButton m_GridToggle;

    // GEU controls for background image
    ToggleButton m_ImageToggle;
    ToggleButton m_LockImageToggle;
    StringOutput m_ImageFileOutput;
    TriggerButton m_ImageFileSelect;

    SliderAdjRangeInput m_ImageWScale;
    SliderAdjRangeInput m_ImageHScale;
    ToggleButton m_PreserveImageAspect;
    ToggleButton m_FlipImageToggle;

    SliderAdjRangeInput m_ImageXOffset;
    SliderAdjRangeInput m_ImageYOffset;

    TriggerButton m_CopyDrawToAllXSec;
    TriggerButton m_ResetDefaultBackground;
    TriggerButton m_ResetViewButton;

    // Draw Objects for XSec
    DrawObj m_XSecCurveDrawObj;
    DrawObj m_XSecCtrlPntsDrawObj;
    vector < DrawObj > m_CEDITTangentLineDrawObj;
    DrawObj m_CEDITTangentPntDrawObj;
    DrawObj m_CurrentPntDrawObj;

    // Vector of sliders to match the curve parameter vectors
    vector < vector < Input > > m_InputVecVec;
    vector < CheckButton > m_EnforceG1Vec;
    vector < CheckButton > m_FixedUCheckVec;

    bool m_FreezeAxis; // Restrict the canvas from resizing when performing a click and drag operation

    // Flags to delete or split after the curve is clicked on
    bool m_DeleteActive;
    bool m_SplitActive;

    // Keeps track of previous curve type. If changed, redraw sliders
    int m_PrevCurveType;

    // Variables to help with CEDIT point selection restrictions
    int m_PrevIndex; // Maintains the previously selected point index

    int m_GlWinWidth;

    // Variables for fixed image option - scale and position is updated
    // to follow zoom and pan events
    double m_ImageZoomOffset;
    glm::vec2 m_ImagePanOffset;
    double m_ImageWOrig;
    double m_ImageHOrig;
    double m_ImageXOffsetOrig;
    double m_ImageYOffsetOrig;

};

#endif // !defined(CURVE_EDIT_H)
