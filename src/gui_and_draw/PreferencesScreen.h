//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//

// PreferencesScreen.h: interface for setting preferences.
//
//////////////////////////////////////////////////////////////////////

#ifndef PREFERENCESSCREEN_H
#define PREFERENCESSCREEN_H

#include "ScreenMgr.h"
#include "ScreenBase.h"
#include "Vehicle.h"
#include "GuiDevice.h"
#include "NavController.h"

#include <FL/Fl_Multiline_Output.H>

using namespace std;

class NavPressBox;
class NavAxisMeter;

class PreferencesScreen : public TabScreen
{
public:
    PreferencesScreen( ScreenMgr* mgr );
    virtual ~PreferencesScreen();

    void Show();
    void Hide();
    bool Update();

    void CallBack( Fl_Widget *w );
    static void staticScreenCB( Fl_Widget *w, void* data )
    {
        ( ( PreferencesScreen* )data )->CallBack( w );
    }

    virtual void GuiDeviceCallBack( GuiDevice* device );

    int GetFCType();
    void SetFCType( int fc );

    // The main window's toolbar size, a ToolbarSize.
    int GetToolbarSize();
    void SetToolbarSize( int size );

    // Where the Geom Browser's panel is, a BrowserPlace.
    int GetBrowserPlace();
    void SetBrowserPlace( int place );

    // Keeps the window mode, a vsp::WINDOW_MODE, in the preferences and shows the screens that way
    void SetWindowMode( int mode );

protected:

    // Show m_Nav in the controls.
    void ShowNav();

    // Grey the controls that do nothing with m_Nav's other settings.
    void ShowBallSpeed();
    void ShowActive();

    // Read a changed control into m_Nav.  False if it is not one of m_Nav's.
    bool ReadNav( GuiDevice * device );

    // Read a changed control of the selected binding, or of the bindings as a whole, into
    // m_Nav.  False if it is not one of those.
    bool ReadBinding( GuiDevice * device );

    // Set m_Nav to a NavPreset, or keep it for Custom, and use it.
    void ApplyScheme( int scheme );

    // Show which preset m_Nav matches, or Custom, and what its buttons do in the tooltip.
    void ShowScheme();

    // Show the binding list, the selected binding and the notes on the bindings.
    void ShowBindings();

    // Show what the last press in the try box does with the bindings as they are.
    void ShowTriedPress();

    // Save m_Nav and use it in the main window now.
    void ApplyNav();

    // Show and use the preferences as they are stored, as after loading a preferences file.
    void ApplyStored();

    // Shows the screens as the window mode kept in the preferences says
    static void StaticWindowModeCB( void* data );

    // While the window is shown, show the SpaceMouse's axes as they move.
    static void StaticMeterCB( void* data );
    void ShowMeters();

    // True if a six-axis device is open, so its buttons can be bound and its rows are shown.
    bool HasDevice();

    // The binding before (step -1) or after (step 1) the selected one that is of the same kind,
    // mouse or device, or -1.
    int NeighborBinding( int step );

    // The 3D interaction settings being edited.
    VSPGUI::NavSettings m_Nav;

    // The settings when the window opened, for Revert.
    VSPGUI::NavSettings m_NavAtShow;
    int m_FCTypeAtShow;
    int m_ToolbarAtShow;
    int m_WindowModeAtShow;
    int m_BrowserPlaceAtShow;

    // True while the screens move to another window mode, which shows this window again.
    bool m_ModeChanging;

    // Index of the selected binding, or -1.
    int m_SelBinding;

    // The binding shown on each row of the bindings list, after its header.  Device button
    // rows are left out while no device is open.
    std::vector < int > m_BindingRows;

    // Whether device rows were shown, to show the list again when a device comes or goes.
    bool m_ShowedDevice;

    // The device buttons offered in m_ButtonChoice, after the mouse's, as values from
    // DEVICE_BUTTON_VAL on.
    std::vector < std::string > m_DeviceButtonNames;

    // General tab.
    GroupLayout m_GenLayout;
    Choice m_FCTypeChoice;
    Choice m_ToolbarChoice;
    Choice m_WindowModeChoice;
    Choice m_BrowserPlaceChoice;

    // 3D View tab.
    GroupLayout m_ViewLayout;

    Choice m_ProfileChoice;
    Choice m_RotateModeChoice;
    Choice m_BallTypeChoice;
    Choice m_BallSizeChoice;
    VspButton * m_BallSpeedLabel;
    VspSlider * m_BallSpeedSlider;
    VspDragInput * m_BallSpeedInput;
    Choice m_TurntableSpeedChoice;
    Choice m_BallOutlineChoice;
    Choice m_PivotChoice;

    Choice m_WheelDirChoice;
    Choice m_WheelAnchorChoice;
    Choice m_WheelStepChoice;
    Choice m_DragZoomDirChoice;
    Choice m_DragZoomAnchorChoice;
    Choice m_DragZoomLawChoice;
    Choice m_ZoomSpeedChoice;

    Choice m_DeviceChoice;
    StringOutput m_DeviceStatus;

    std::string m_ProfileTip;

    // Mouse Buttons tab.
    GroupLayout m_ButtonsLayout;

    Choice m_ButtonsProfileChoice;
    ColResizeBrowser * m_BindingBrowser;
    TriggerButton m_AddBindingButton;
    TriggerButton m_DeleteBindingButton;
    TriggerButton m_UpBindingButton;
    TriggerButton m_DownBindingButton;

    Choice m_ActionChoice;
    Choice m_TriggerChoice;
    Choice m_ButtonChoice;
    Choice m_WithButtonChoice;
    Choice m_ModChoice[4];
    Choice m_OtherButtonsChoice;
    StringInput m_HeldKeyInput;
    Choice m_WhereChoice;
    Choice m_PickFirstChoice;

    NavPressBox * m_PressBox;
    TriggerButton m_UsePressButton;

    Choice m_ClickDistanceChoice;
    Choice m_ChordChoice;
    Choice m_EmulateMiddleChoice;

    Fl_Multiline_Output * m_NotesOutput;

    // SpaceMouse tab.
    GroupLayout m_SpaceMouseLayout;
    Choice m_SpaceMouseChoice;
    Choice m_SpaceMouseCursorChoice;
    StringOutput m_SpaceMouseStatus;
    Choice m_SpaceMouseSpeedChoice;
    Choice m_SpaceMouseRotateChoice;
    Choice m_SpaceMouseMoveChoice;
    Choice m_SpaceMouseDominantChoice;
    Choice m_AxisChoice[ VSPGUI::NAV_NUM_AXES ];
    Choice m_AxisReverseChoice[ VSPGUI::NAV_NUM_AXES ];
    NavAxisMeter * m_AxisMeter[ VSPGUI::NAV_NUM_AXES ];
    TriggerButton m_AxisDefaultsButton;

    // Bottom of the window.
    GroupLayout m_BottomLayout;
    TriggerButton m_SaveButton;
    TriggerButton m_LoadButton;
    TriggerButton m_DefaultsButton;
    TriggerButton m_RevertButton;
    TriggerButton m_CloseButton;
};

#endif  // PREFERENCESSCREEN_H
