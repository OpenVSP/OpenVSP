//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//
//////////////////////////////////////////////////////////////////////

#ifndef MASSPROPSCREEN_H_
#define MASSPROPSCREEN_H_

#include "ScreenBase.h"
#include "ScreenMgr.h"

class MassPropScreen : public BasicScreen
{
public:

    MassPropScreen( ScreenMgr* mgr );
    virtual ~MassPropScreen()                          {}

    void Show();
    void Hide();
    bool Update();

    virtual void GuiDeviceCallBack( GuiDevice* device );
    virtual void CallBack( Fl_Widget* w );
    virtual void CloseCallBack( Fl_Widget* w );
    
    void LoadDrawObjs( vector< DrawObj* >& draw_obj_vec );

protected:

    GroupLayout m_MainLayout;
    GroupLayout m_BorderLayout;

    SliderAdjRangeInput m_NumSlicesInput;
    Choice m_SliceDirChoice;

    ToggleButton m_SetToggle;

    Choice m_SetChoice;
    Choice m_DegenSet;

    ToggleRadioGroup m_ModeSetToggleGroup;
    ToggleButton m_ModeToggle;
    Choice m_ModeChoice;

    DrawObj m_CGDrawObj;
    ToggleButton m_DrawCgButton;

    StringOutput m_TotalMassOutput;
    StringOutput m_XCgOutput;
    StringOutput m_YCgOutput;
    StringOutput m_ZCgOutput;
    StringOutput m_IxxOutput;
    StringOutput m_IyyOutput;
    StringOutput m_IzzOutput;
    StringOutput m_IxyOutput;
    StringOutput m_IxzOutput;
    StringOutput m_IyzOutput;
    StringOutput m_FileSelect;

    TriggerButton m_FileExportButton;
    TriggerButton m_ComputeButton;
    TriggerButton m_FileTrigger;

private :
    int m_SelectedSetIndex;
    int m_DegenSelectedSetIndex;
    int m_SelectedModeChoice;
    vector < string > m_ModeIDs;
};

#endif /* MASSPROPSCREEN_H_ */
