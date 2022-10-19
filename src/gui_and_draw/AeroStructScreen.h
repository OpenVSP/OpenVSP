//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//

// Manage coupled aero / structural analyses
// Rob McDonald
//
//////////////////////////////////////////////////////////////////////

#if !defined(VSPAEROSTRUCTSCREEN__INCLUDED_)
#define VSPAEROSTRUCTSCREEN__INCLUDED_

#include "ScreenBase.h"
#include "GuiDevice.h"
#include "ProcessUtil.h"

#include <FL/Fl.H>
#include <FL/Fl_Text_Buffer.H>

using std::string;
using std::vector;

class AeroStructScreen : public BasicScreen
{
public:
    AeroStructScreen( ScreenMgr* mgr );
    virtual ~AeroStructScreen();
    void Show();
    void Hide();
    bool Update();

    void AddOutputText( const string &text );

    void CallBack( Fl_Widget *w );
    static void staticScreenCB( Fl_Widget *w, void* data )
    {
        ( static_cast <AeroStructScreen*> ( data ) )->CallBack( w );
    }

    void GuiDeviceCallBack( GuiDevice* gui_device );

    vector < string > MakeStructIDVec();

    ProcessUtil *GetProcess();
    Fl_Text_Display *GetDisplay();

    void LaunchBatchFEAMesh( const vector < string > &idvec );


protected:

    GroupLayout m_GlobalLayout;

    TriggerButton m_ShowVSPAEROGUI;
    TriggerButton m_ExecuteVSPAERO;

    Choice m_StructureChoice;
    TriggerButton m_ShowFEAMeshGUI;
    TriggerButton m_ShowFEAAssemblyGUI;
    TriggerButton m_ExecuteRemeshAllFEAMesh;
    TriggerButton m_ExecuteMeshUnmeshedFEAMesh;
    TriggerButton m_ExportFEAMesh;

    SliderAdjRangeInput m_DynPressSlider;
    TriggerButton m_ExecuteLoads;

    TriggerButton m_ExecuteCalculiX;

    TriggerButton m_ExecuteViewer;

    //===== Console Items =====//
    Fl_Text_Display* m_ConsoleDisplay;
    Fl_Text_Buffer* m_ConsoleBuffer;

    // Viewer thread handling
    ProcessUtil m_ViewerProcess;

    ProcessUtil m_ViewerMonitor;
    ProcessUtil m_FeaMeshProcess;

    vector < string > m_BatchIDs;
};


#endif
