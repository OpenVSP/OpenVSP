//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//

// StructAssemblyScreen.h:
// Rob McDonald
//////////////////////////////////////////////////////////////////////

#if !defined(VSP_STRUCTASSEMBLYSCREEN_H_INCLUDED_)
#define VSP_STRUCTASSEMBLYSCREEN_H_INCLUDED_

#include "ScreenBase.h"
#include "GuiDevice.h"
#include "Vehicle.h"
#include "ScreenMgr.h"
#include "FeaStructure.h"
#include "ProcessUtil.h"
#include "MainVSPScreen.h"
#include "MainGLWindow.h"

using namespace std;

class StructAssemblyScreen;

typedef std::pair< StructAssemblyScreen*, vector<string> > batchmeshpair;

class StructAssemblyScreen : public TabScreen
{
public:
    StructAssemblyScreen( ScreenMgr* mgr );
    virtual ~StructAssemblyScreen();

    virtual void Show();
    virtual void LaunchBatchFEAMesh( const vector < string > &idvec );

    virtual void GuiDeviceCallBack( GuiDevice* device );

    virtual void CallBack( Fl_Widget* w );
    virtual void CloseCallBack( Fl_Widget *w );

    static void staticScreenCB( Fl_Widget *w, void* data )
    {
        ( ( StructAssemblyScreen* )data )->CallBack( w );
    }

    virtual void AddOutputText( const string &text );

    virtual bool Update();
    virtual void UpdateAssemblyTab();
    virtual void UpdateStructTab();
    virtual void UpdateConnectionTab();
    virtual void UpdateDrawPartBrowser();

    virtual void LoadDrawObjs( vector< DrawObj* > &draw_obj_vec );

    ProcessUtil* getFeaMeshProcess()
    {
        return &m_FeaMeshProcess;
    }

private:

    GroupLayout m_ConsoleLayout;
    GroupLayout m_BorderConsoleLayout;
    GroupLayout m_AssemblyTabLayout;
    GroupLayout m_StructureTabLayout;
    GroupLayout m_ConnectionsTabLayout;
    GroupLayout m_OutputTabLayout;

    //===== Console Items =====//
    Fl_Text_Display* m_ConsoleDisplay;
    Fl_Text_Buffer* m_ConsoleBuffer;

    ProcessUtil m_FeaMeshProcess;
    ProcessUtil m_MonitorProcess;

    //===== Common buttons =====//
    TriggerButton m_FeaReMeshAllButton;
    TriggerButton m_FeaMeshUnmeshedButton;
    TriggerButton m_FeaExportMeshButton;


    //===== Assembly Tab Items =====//
    TriggerButton m_AddAssemblyButton;
    TriggerButton m_DelAssemblyButton;

    ColResizeBrowser* m_AssemblySelectBrowser;
    vector < string > m_AssemblyIDs;

    StringInput m_AssemblyNameInput;

    //===== Structure Tab Items =====//
    ColResizeBrowser* m_StructureSelectBrowser;
    int m_StructureBrowserIndex;

    Choice m_FeaStructureChoice;
    int m_StructureChoiceIndex;
    vector < string > m_StructIDs;

    TriggerButton m_AddFeaStructureButton;
    TriggerButton m_DelFeaStructureButton;

    //===== Connection Items =====//
    ColResizeBrowser* m_ConnectionSelectBrowser;
    int m_ConnectionBrowserIndex;

    Choice m_ConnectionStartChoice;
    Choice m_ConnectionEndChoice;
    int m_ConnectionStartIndex;
    int m_ConnectionEndIndex;
    Choice m_ConnectionStartSurfIndxChoice;
    Choice m_ConnectionEndSurfIndxChoice;
    int m_ConnectionStartSurfIndex;
    int m_ConnectionEndSurfIndex;

    vector < string > m_FixPtIDs;
    vector < string > m_FixPtStructIDs;

    TriggerButton m_AddConnectionButton;
    TriggerButton m_DelConnectionButton;

    //===== Output Items =====//
    ToggleButton m_StlFile;
    ToggleButton m_GmshFile;
    ToggleButton m_MassFile;
    ToggleButton m_NastFile;
    ToggleButton m_NkeyFile;
    ToggleButton m_CalcFile;

    TriggerButton m_SelectStlFile;
    TriggerButton m_SelectGmshFile;
    TriggerButton m_SelectMassFile;
    TriggerButton m_SelectNastFile;
    TriggerButton m_SelectNkeyFile;
    TriggerButton m_SelectCalcFile;

    StringOutput m_StlOutput;
    StringOutput m_GmshOutput;
    StringOutput m_MassOutput;
    StringOutput m_NastOutput;
    StringOutput m_NkeyOutput;
    StringOutput m_CalcOutput;

    //===== Draw Tab Items =====//
    ToggleButton m_DrawMeshButton;
    ToggleButton m_ColorElementsButton;
    ToggleButton m_DrawNodesToggle;
    ToggleButton m_DrawElementOrientVecToggle;

    Fl_Check_Browser* m_DrawPartSelectBrowser;

    TriggerButton m_DrawAllButton;
    TriggerButton m_HideAllButton;

    //==== Private Variables ====//

    vector < string > m_BatchIDs;

};

#endif
