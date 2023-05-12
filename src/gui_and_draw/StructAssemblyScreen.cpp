//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//

// StructAssemblyScreen.cpp: implementation of the StructAssemblyScreen class.
//
//////////////////////////////////////////////////////////////////////

#include "StructAssemblyScreen.h"
#include "FeaMeshMgr.h"
#include "StructureMgr.h"
#include "ManageViewScreen.h"
#include "GraphicEngine.h"
#include "Display.h"
#include "Camera.h"
#include "ParmMgr.h"
#include "StlHelper.h"

using namespace vsp;

//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

StructAssemblyScreen::StructAssemblyScreen( ScreenMgr* mgr ) : TabScreen( mgr, 430, 720, "FEA Assembly", 221 + 5 )
{
    m_FLTK_Window->callback( staticCloseCB, this );

    int border = 5;

    Fl_Group* assyTab = AddTab( "Assembly" );
    Fl_Group* assyTabGroup = AddSubGroup( assyTab, border );
    Fl_Group* structTab = AddTab( "Structures" );
    Fl_Group* structTabGroup = AddSubGroup( structTab, border );
    Fl_Group* conTab = AddTab( "Connections" );
    Fl_Group* conTabGroup = AddSubGroup( conTab, border );
    Fl_Group* femTab = AddTab( "FEM" );
    Fl_Group* femTabGroup = AddSubGroup( femTab, border );

    //=== Create Console Area ===//
    m_ConsoleLayout.SetGroupAndScreen( m_FLTK_Window, this );

    int textheight = 150;

    m_ConsoleLayout.AddY( m_ConsoleLayout.GetRemainY()
                        - textheight                            // 150
                        - 3 * m_ConsoleLayout.GetStdHeight()    // 3 * 20
                        - m_ConsoleLayout.GetGapHeight()        // 6
                        - border );                             // 5

    // 221 + 5 passed to TabScreen constructor above is this sum plus an additional border.
    // textheight + 2 * m_ConsoleLayout.GetStdHeight() + m_ConsoleLayout.GetGapHeight() + 2 * border
    // This is for the contents of m_BorderConsoleLayout.

    m_ConsoleLayout.AddX( border );

    m_ConsoleLayout.AddSubGroupLayout( m_BorderConsoleLayout, m_ConsoleLayout.GetRemainX() - border,
                                       m_ConsoleLayout.GetRemainY() - border );

    m_ConsoleDisplay = m_BorderConsoleLayout.AddFlTextDisplay( textheight );
    m_ConsoleBuffer = new Fl_Text_Buffer;
    m_ConsoleDisplay->buffer( m_ConsoleBuffer );
    m_FLTK_Window->resizable( m_ConsoleDisplay );

    m_BorderConsoleLayout.AddYGap();

    m_BorderConsoleLayout.AddButton( m_DrawAsMeshButton, "Draw as Mesh When Ready" );

    m_BorderConsoleLayout.SetSameLineFlag( true );
    m_BorderConsoleLayout.SetFitWidthFlag( false );

    m_BorderConsoleLayout.SetButtonWidth( m_BorderConsoleLayout.GetW() / 2 );


    m_BorderConsoleLayout.AddButton( m_FeaReMeshAllButton, "Re-Mesh All" );
    m_BorderConsoleLayout.AddButton( m_FeaMeshUnmeshedButton, "Mesh Unmeshed" );
    m_BorderConsoleLayout.ForceNewLine();

    m_BorderConsoleLayout.SetButtonWidth( m_BorderConsoleLayout.GetW() );

    m_BorderConsoleLayout.AddButton( m_FeaExportMeshButton, "Export FEM" );

    //=== Assembly Tab ===//
    assyTab->show();

    m_AssemblyTabLayout.SetGroupAndScreen( assyTabGroup, this );

    m_AssemblyTabLayout.AddDividerBox( "Assembly Selection" );

    // Pointer for the widths of each column in the browser to support resizing
    // Last column width must be 0
    static int assy_col_widths[] = { 400, 0 }; // widths for each column

    int browser_h = 150;
    m_AssemblySelectBrowser = m_AssemblyTabLayout.AddColResizeBrowser( assy_col_widths, 1, browser_h );
    m_AssemblySelectBrowser->callback( staticScreenCB, this );

    int buttonwidth = m_AssemblyTabLayout.GetButtonWidth();

    m_AssemblyTabLayout.SetChoiceButtonWidth( buttonwidth );
    m_AssemblyTabLayout.SetSliderWidth( ( m_AssemblyTabLayout.GetW() - ( 2 * m_AssemblyTabLayout.GetButtonWidth() ) ) / 2 );

    m_AssemblyTabLayout.SetSameLineFlag( true );
    m_AssemblyTabLayout.SetFitWidthFlag( false );

    m_AssemblyTabLayout.SetButtonWidth( m_AssemblyTabLayout.GetRemainX() / 2 );

    m_AssemblyTabLayout.AddButton( m_AddAssemblyButton, "Add Assembly" );
    m_AssemblyTabLayout.AddButton( m_DelAssemblyButton, "Delete Assembly" );

    m_AssemblyTabLayout.ForceNewLine();
    m_AssemblyTabLayout.AddYGap();

    m_AssemblyTabLayout.SetSameLineFlag( false );
    m_AssemblyTabLayout.SetFitWidthFlag( true );

    m_AssemblyTabLayout.AddInput( m_AssemblyNameInput, "Assembly Name" );


    //=== Structure Tab ===//

    m_StructureTabLayout.SetGroupAndScreen( structTabGroup, this );

    m_StructureTabLayout.AddDividerBox( "Structure Selection" );

    static int struct_col_widths[] = { 300, 100, 0 }; // widths for each column

    m_StructureSelectBrowser = m_StructureTabLayout.AddColResizeBrowser( struct_col_widths, 2, browser_h );
    m_StructureSelectBrowser->callback( staticScreenCB, this );

    m_StructureTabLayout.AddChoice( m_FeaStructureChoice, "Structure" );

    m_StructureTabLayout.SetSameLineFlag( false );
    m_StructureTabLayout.SetFitWidthFlag( true );

    m_StructureTabLayout.AddButton( m_AddFeaStructureButton, "Add Structure" );
    m_StructureTabLayout.AddButton( m_AddAllFeaStructureButton, "Add All Structures" );
    m_StructureTabLayout.AddButton( m_DelFeaStructureButton, "Delete Structure" );

    //=== Connection Tab ===//

    m_ConnectionsTabLayout.SetGroupAndScreen( conTabGroup, this );

    m_ConnectionsTabLayout.AddDividerBox( "Connection Selection" );

    static int con_col_widths[] = { 100, 75, 25, 100, 75, 25, 0 }; // widths for each column

    m_ConnectionSelectBrowser = m_ConnectionsTabLayout.AddColResizeBrowser( con_col_widths, 6, browser_h );
    m_ConnectionSelectBrowser->callback( staticScreenCB, this );

    m_ConnectionsTabLayout.SetSameLineFlag( true );
    m_ConnectionsTabLayout.SetFitWidthFlag( false );

    m_ConnectionsTabLayout.SetChoiceButtonWidth( m_ConnectionsTabLayout.GetRemainX() / 4 );
    m_ConnectionsTabLayout.SetSliderWidth( m_ConnectionsTabLayout.GetRemainX() / 4 );

    m_ConnectionsTabLayout.AddChoice( m_ConnectionStartChoice, "Start" );
    m_ConnectionsTabLayout.AddChoice( m_ConnectionStartSurfIndxChoice, "Index" );

    m_ConnectionsTabLayout.ForceNewLine();

    m_ConnectionsTabLayout.AddChoice( m_ConnectionEndChoice, "End" );
    m_ConnectionsTabLayout.AddChoice( m_ConnectionEndSurfIndxChoice, "Index" );

    m_ConnectionsTabLayout.ForceNewLine();

    m_ConnectionsTabLayout.SetButtonWidth( m_ConnectionsTabLayout.GetRemainX() / 2 );

    m_ConnectionsTabLayout.AddButton( m_AddConnectionButton, "Add Connection" );
    m_ConnectionsTabLayout.AddButton( m_DelConnectionButton, "Delete Connection" );
    m_ConnectionsTabLayout.ForceNewLine();

    m_ConnectionsTabLayout.SetSameLineFlag( false );
    m_ConnectionsTabLayout.SetFitWidthFlag( true );

    m_ConnectionsTabLayout.AddDividerBox( "Connection Properties" );

    m_ConnectionsTabLayout.AddChoice( m_FeaConnModeChoice, "Mode" );
    m_FeaConnModeChoice.AddItem( "User", vsp::FEA_BCM_USER );
    m_FeaConnModeChoice.AddItem( "All", vsp::FEA_BCM_ALL );
    m_FeaConnModeChoice.AddItem( "Pin", vsp::FEA_BCM_PIN );
    m_FeaConnModeChoice.UpdateItems();

    m_ConnectionsTabLayout.SetSameLineFlag( true );
    m_ConnectionsTabLayout.SetFitWidthFlag( false );

    m_ConnectionsTabLayout.SetButtonWidth( m_ConnectionsTabLayout.GetW() / 4 );

    m_ConnectionsTabLayout.AddLabel( "Translation:", m_ConnectionsTabLayout.GetButtonWidth() );

    m_ConnectionsTabLayout.AddButton( m_TxButton, "X", 1 << 0 );
    m_ConnectionsTabLayout.AddButton( m_TyButton, "Y", 1 << 1 );
    m_ConnectionsTabLayout.AddButton( m_TzButton, "Z", 1 << 2 );

    m_ConnectionsTabLayout.ForceNewLine();

    m_ConnectionsTabLayout.AddLabel( "Rotation:", m_ConnectionsTabLayout.GetButtonWidth() );
    m_ConnectionsTabLayout.AddButton( m_RxButton, "X", 1 << 3 );
    m_ConnectionsTabLayout.AddButton( m_RyButton, "Y", 1 << 4 );
    m_ConnectionsTabLayout.AddButton( m_RzButton, "Z", 1 << 5 );

    //=== Output Tab ===//
    m_FemTabLayout.SetGroupAndScreen( femTabGroup, this );

    m_FemTabLayout.AddDividerBox( "File Export" );

    m_FemTabLayout.SetFitWidthFlag( false );
    m_FemTabLayout.SetSameLineFlag( true );

    m_FemTabLayout.SetInputWidth( m_FemTabLayout.GetW() - 75 - 55 );

    m_FemTabLayout.SetButtonWidth( 75 );
    m_FemTabLayout.AddButton( m_StlFile, ".stl" );
    m_FemTabLayout.AddOutput( m_StlOutput );
    m_FemTabLayout.SetButtonWidth( m_FemTabLayout.GetRemainX() );
    m_FemTabLayout.AddButton( m_SelectStlFile, "..." );
    m_FemTabLayout.ForceNewLine();

    m_FemTabLayout.SetButtonWidth( 75 );
    m_FemTabLayout.AddButton( m_GmshFile, ".msh" );
    m_FemTabLayout.AddOutput( m_GmshOutput );
    m_FemTabLayout.SetButtonWidth( m_FemTabLayout.GetRemainX() );
    m_FemTabLayout.AddButton( m_SelectGmshFile, "..." );
    m_FemTabLayout.ForceNewLine();

    m_FemTabLayout.AddYGap();

    m_FemTabLayout.SetButtonWidth( 75 );
    m_FemTabLayout.AddButton( m_MassFile, "Mass" );
    m_FemTabLayout.AddOutput( m_MassOutput );
    m_FemTabLayout.SetButtonWidth( m_FemTabLayout.GetRemainX() );
    m_FemTabLayout.AddButton( m_SelectMassFile, "..." );
    m_FemTabLayout.ForceNewLine();

    m_FemTabLayout.AddYGap();

    m_FemTabLayout.SetButtonWidth( 75 );
    m_FemTabLayout.AddButton( m_NastFile, "Nastran" );
    m_FemTabLayout.AddOutput( m_NastOutput );
    m_FemTabLayout.SetButtonWidth( m_FemTabLayout.GetRemainX() );
    m_FemTabLayout.AddButton( m_SelectNastFile, "..." );
    m_FemTabLayout.ForceNewLine();

    m_FemTabLayout.SetButtonWidth( 75 );
    m_FemTabLayout.AddButton( m_NkeyFile, "Nkey" );
    m_FemTabLayout.AddOutput( m_NkeyOutput );
    m_FemTabLayout.SetButtonWidth( m_FemTabLayout.GetRemainX() );
    m_FemTabLayout.AddButton( m_SelectNkeyFile, "..." );
    m_FemTabLayout.ForceNewLine();

    m_FemTabLayout.AddYGap();

    m_FemTabLayout.SetButtonWidth( 75 );
    m_FemTabLayout.AddButton( m_CalcFile, "Calculix" );
    m_FemTabLayout.AddOutput( m_CalcOutput );
    m_FemTabLayout.SetButtonWidth( m_FemTabLayout.GetRemainX() );
    m_FemTabLayout.AddButton( m_SelectCalcFile, "..." );
    m_FemTabLayout.ForceNewLine();
    m_FemTabLayout.AddYGap();

    m_FemTabLayout.SetSameLineFlag( false );
    m_FemTabLayout.SetFitWidthFlag( true );

    m_FemTabLayout.AddDividerBox( "FEM Display" );

    m_FemTabLayout.SetSameLineFlag( true );
    m_FemTabLayout.SetFitWidthFlag( false );
    m_FemTabLayout.SetButtonWidth( m_FemTabLayout.GetW() / 2 );

    m_FemTabLayout.AddButton( m_DrawMeshButton, "Draw Mesh" );
    m_FemTabLayout.AddButton( m_ColorElementsButton, "Color Elements" );
    m_FemTabLayout.ForceNewLine();

    m_FemTabLayout.AddButton( m_DrawNodesToggle, "Draw Nodes" );
    m_FemTabLayout.AddButton( m_DrawElementOrientVecToggle, "Draw Element Orientation Vectors" );
    m_FemTabLayout.ForceNewLine();

    m_FemTabLayout.AddButton( m_DrawBCNodesToggle, "Draw BCs" );
    m_FemTabLayout.ForceNewLine();

    m_FemTabLayout.AddYGap();

    m_FemTabLayout.SetSameLineFlag( false );
    m_FemTabLayout.SetFitWidthFlag( true );

    m_FemTabLayout.AddDividerBox( "Element Sets" );

    m_DrawPartSelectBrowser = m_FemTabLayout.AddCheckBrowser( browser_h );
    m_DrawPartSelectBrowser->callback( staticScreenCB, this );

    m_FemTabLayout.AddY( 125 );
    m_FemTabLayout.AddYGap();

    m_FemTabLayout.SetSameLineFlag( true );
    m_FemTabLayout.SetFitWidthFlag( false );

    m_FemTabLayout.AddButton( m_DrawAllButton, "Draw All Elements" );
    m_FemTabLayout.AddButton( m_HideAllButton, "Hide All Elements" );
    m_FemTabLayout.ForceNewLine();


    m_StructureBrowserIndex = 0;
    m_ConnectionBrowserIndex = 0;

    m_StructureChoiceIndex = 0;

    m_ConnectionStartIndex = 0;
    m_ConnectionEndIndex = 0;

    m_ConnectionStartSurfIndex = 0;
    m_ConnectionEndSurfIndex = 0;

}

StructAssemblyScreen::~StructAssemblyScreen()
{
    m_ConsoleDisplay->buffer( NULL );
    delete m_ConsoleBuffer;
}

void StructAssemblyScreen::CloseCallBack( Fl_Widget *w )
{
    Hide();

    m_ScreenMgr->SetUpdateFlag( true );
}

void StructAssemblyScreen::Show()
{
    m_FLTK_Window->show();

    m_ScreenMgr->SetUpdateFlag( true );
}

bool StructAssemblyScreen::Update()
{
    FeaAssembly* curr_assy = StructureMgr.GetFeaAssembly( StructureMgr.GetCurrAssemblyIndex() );

    // This makes sure connection DO's are updated.
    // does not need to be called every time, but they aren't in an update path
    // otherwise.
    if ( curr_assy )
    {
        curr_assy->Update();
    }

    UpdateAssemblyTab();
    UpdateStructTab();
    UpdateConnectionTab();
    UpdateDrawPartBrowser();

    if ( !curr_assy )
    {
        return false;
    }

    m_DrawAsMeshButton.Update( curr_assy->m_AssemblySettings.m_DrawAsMeshFlag.GetID() );

    //===== Display Tab Toggle Update =====//
    m_DrawMeshButton.Update( curr_assy->m_AssemblySettings.m_DrawMeshFlag.GetID() );
    m_ColorElementsButton.Update( curr_assy->m_AssemblySettings.m_ColorFacesFlag.GetID() );
    m_DrawNodesToggle.Update( curr_assy->m_AssemblySettings.m_DrawNodesFlag.GetID() );
    m_DrawBCNodesToggle.Update( curr_assy->m_AssemblySettings.m_DrawBCNodesFlag.GetID() );
    m_DrawElementOrientVecToggle.Update( curr_assy->m_AssemblySettings.m_DrawElementOrientVecFlag.GetID() );

    FeaMeshMgr.UpdateAssemblyDisplaySettings( curr_assy->GetID() );

    string massname = curr_assy->m_AssemblySettings.GetExportFileName( vsp::FEA_MASS_FILE_NAME );
    m_MassOutput.Update( StringUtil::truncateFileName( massname, 40 ).c_str() );
    string nastranname = curr_assy->m_AssemblySettings.GetExportFileName( vsp::FEA_NASTRAN_FILE_NAME );
    m_NastOutput.Update( StringUtil::truncateFileName( nastranname, 40 ).c_str() );
    string nkeyname = curr_assy->m_AssemblySettings.GetExportFileName( vsp::FEA_NKEY_FILE_NAME );
    m_NkeyOutput.Update( StringUtil::truncateFileName( nkeyname, 40 ).c_str() );
    string calculixname = curr_assy->m_AssemblySettings.GetExportFileName( vsp::FEA_CALCULIX_FILE_NAME );
    m_CalcOutput.Update( StringUtil::truncateFileName( calculixname, 40 ).c_str() );
    string stlname = curr_assy->m_AssemblySettings.GetExportFileName( vsp::FEA_STL_FILE_NAME );
    m_StlOutput.Update( StringUtil::truncateFileName( stlname, 40 ).c_str() );
    string gmshname = curr_assy->m_AssemblySettings.GetExportFileName( vsp::FEA_GMSH_FILE_NAME );
    m_GmshOutput.Update( StringUtil::truncateFileName( gmshname, 40 ).c_str() );

    //==== Update File Output Flags ====//
    m_MassFile.Update( curr_assy->m_AssemblySettings.GetExportFileFlag( vsp::FEA_MASS_FILE_NAME )->GetID() );
    m_NastFile.Update( curr_assy->m_AssemblySettings.GetExportFileFlag( vsp::FEA_NASTRAN_FILE_NAME )->GetID() );
    m_NkeyFile.Update( curr_assy->m_AssemblySettings.GetExportFileFlag( vsp::FEA_NKEY_FILE_NAME )->GetID() );
    m_CalcFile.Update( curr_assy->m_AssemblySettings.GetExportFileFlag( vsp::FEA_CALCULIX_FILE_NAME )->GetID() );
    m_StlFile.Update( curr_assy->m_AssemblySettings.GetExportFileFlag( vsp::FEA_STL_FILE_NAME )->GetID() );
    m_GmshFile.Update( curr_assy->m_AssemblySettings.GetExportFileFlag( vsp::FEA_GMSH_FILE_NAME )->GetID() );

    if ( !curr_assy->m_AssemblySettings.GetExportFileFlag( vsp::FEA_NASTRAN_FILE_NAME )->Get() )
    {
        m_NkeyFile.Deactivate();
        m_NkeyOutput.Deactivate();
        m_SelectNkeyFile.Deactivate();
    }
    else
    {
        m_NkeyFile.Activate();
        m_NkeyOutput.Activate();
        m_SelectNkeyFile.Activate();
    }

    return true;
}

void StructAssemblyScreen::UpdateAssemblyTab()
{
    int scroll_pos = m_AssemblySelectBrowser->position();
    int h_pos = m_AssemblySelectBrowser->hposition();

    m_AssemblySelectBrowser->clear();
    m_AssemblyIDs.clear();

    m_AssemblySelectBrowser->column_char( ':' );
    m_AssemblySelectBrowser->add( "@b@.NAME" );


    vector< FeaAssembly* > assy_vec = StructureMgr.GetFeaAssemblyVec();
    if ( assy_vec.size() > 0 )
    {
        for ( int i = 0; i < (int)assy_vec.size(); i++ )
        {
            string assy_name = assy_vec[i]->GetName();

            char str[256];
            snprintf( str, sizeof( str ),  "%s", assy_name.c_str() );
            m_AssemblySelectBrowser->add( str );

            m_AssemblyIDs.push_back( assy_vec[i]->GetID() );
        }

        if ( StructureMgr.ValidFeaAssemblyInd( StructureMgr.GetCurrAssemblyIndex() ) )
        {
            m_AssemblySelectBrowser->select( StructureMgr.GetCurrAssemblyIndex() + 2 );
        }
    }

    m_AssemblySelectBrowser->position( scroll_pos );
    m_AssemblySelectBrowser->hposition( h_pos );


    if ( StructureMgr.ValidFeaAssemblyInd( StructureMgr.GetCurrAssemblyIndex() ) )
    {
        vector < FeaAssembly * > assyVec = StructureMgr.GetFeaAssemblyVec();
        FeaAssembly *curr_assy = assyVec[ StructureMgr.GetCurrAssemblyIndex() ];

        m_AssemblyNameInput.Update( curr_assy->GetName() );

    }
}

void StructAssemblyScreen::UpdateStructTab()
{
    m_FeaStructureChoice.ClearItems();
    m_StructIDs.clear();

    vector < string > names;

    vector< FeaStructure* > structVec = StructureMgr.GetAllFeaStructs();
    for ( int i = 0; i < (int)structVec.size(); i++ )
    {
        string struct_name = structVec[i]->GetName();

        m_FeaStructureChoice.AddItem( struct_name, i );
        m_StructIDs.push_back( structVec[i]->GetID() );
        names.push_back( struct_name );
    }
    m_FeaStructureChoice.UpdateItems();

    m_FeaStructureChoice.SetVal( m_StructureChoiceIndex );


    //==== Structure Browser ====//
    int scroll_pos = m_StructureSelectBrowser->position();
    int h_pos = m_StructureSelectBrowser->hposition();
    m_StructureSelectBrowser->clear();

    FeaAssembly* curr_assy = StructureMgr.GetFeaAssembly( StructureMgr.GetCurrAssemblyIndex() );

    if ( !curr_assy )
    {
        return;
    }

    m_StructureSelectBrowser->column_char( ':' );
    m_StructureSelectBrowser->add( "@b@.NAME:@b@.MESH READY:" );

    vector < string > idvec = curr_assy->m_StructIDVec;
    for ( int i = 0; i < idvec.size(); i++ )
    {
        int indx = vector_find_val( m_StructIDs, idvec[i] );

        if ( indx >= 0 )
        {
            string ready( "N" );

            FeaStructure *fea_struct = StructureMgr.GetFeaStruct( idvec[ i ] );

            if ( fea_struct )
            {
                FeaMesh *mesh = FeaMeshMgr.GetMeshPtr( idvec[ i ] );
                if ( mesh )
                {
                    if ( mesh->m_MeshReady )
                    {
                        ready = "Y";
                    }
                }
            }

            char str[256];
            snprintf( str, sizeof( str ),  "%s:%s:", names[indx].c_str(), ready.c_str() );


            m_StructureSelectBrowser->add( str );
        }
    }

    m_StructureSelectBrowser->select( m_StructureBrowserIndex + 2 );

    m_StructureSelectBrowser->position( scroll_pos );
    m_StructureSelectBrowser->hposition( h_pos );
}

void StructAssemblyScreen::UpdateConnectionTab()
{
    int scroll_pos = m_ConnectionSelectBrowser->position();
    int h_pos = m_ConnectionSelectBrowser->hposition();

    m_ConnectionSelectBrowser->clear();
    m_ConnectionStartChoice.ClearItems();
    m_ConnectionEndChoice.ClearItems();
    m_ConnectionStartSurfIndxChoice.ClearItems();
    m_ConnectionEndSurfIndxChoice.ClearItems();
    m_FixPtIDs.clear();
    m_FixPtStructIDs.clear();

    FeaAssembly* curr_assy = StructureMgr.GetFeaAssembly( StructureMgr.GetCurrAssemblyIndex() );

    if ( !curr_assy )
    {
        return;
    }

    vector < FeaPart* > allfp;
    vector < string > fpstructids;

    curr_assy->GetAllFixPts( allfp, fpstructids );

    for ( int i = 0; i < allfp.size(); i++ )
    {
        string st_name;

        FeaStructure * fea_struct = StructureMgr.GetFeaStruct( fpstructids[i] );
        if ( fea_struct )
        {
            st_name = fea_struct->GetName();
            st_name += "_";
        }

        FeaPart* fp = allfp[i];
        if ( fp )
        {
            st_name += fp->GetName();

            m_FixPtIDs.push_back( fp->GetID() );
            m_FixPtStructIDs.push_back( fea_struct->GetID() );

            m_ConnectionStartChoice.AddItem( st_name, i );
            m_ConnectionEndChoice.AddItem( st_name, i );
        }
    }
    m_ConnectionStartChoice.UpdateItems();
    m_ConnectionEndChoice.UpdateItems();

    m_ConnectionStartChoice.SetVal( m_ConnectionStartIndex );
    m_ConnectionEndChoice.SetVal( m_ConnectionEndIndex );

    if ( m_FixPtIDs.size() > 0 )
    {
        FeaPart* fpstart = StructureMgr.GetFeaPart( m_FixPtIDs[ m_ConnectionStartIndex ] );
        if ( fpstart )
        {
            int num = fpstart->NumFeaPartSurfs();

            for ( int i = 0; i < num; i ++ )
            {
                m_ConnectionStartSurfIndxChoice.AddItem( to_string( i ), i );
            }
        }
        m_ConnectionStartSurfIndxChoice.UpdateItems();
        m_ConnectionStartSurfIndxChoice.SetVal( m_ConnectionStartSurfIndex );


        FeaPart* fpend = StructureMgr.GetFeaPart( m_FixPtIDs[ m_ConnectionEndIndex ] );
        if ( fpend )
        {
            int num = fpend->NumFeaPartSurfs();

            for ( int i = 0; i < num; i ++ )
            {
                m_ConnectionEndSurfIndxChoice.AddItem( to_string( i ), i );
            }
        }
        m_ConnectionEndSurfIndxChoice.UpdateItems();
        m_ConnectionEndSurfIndxChoice.SetVal( m_ConnectionEndSurfIndex );
    }

    m_ConnectionSelectBrowser->column_char( ':' );
    m_ConnectionSelectBrowser->add( "@b@.START STRUCT:@b@.PT:@b@.INDEX:@b@.END STRUCT:@b@.PT:@b@.INDEX:" );

    vector < FeaConnection* > convec = curr_assy->m_ConnectionVec;
    for ( int i = 0; i < convec.size(); i++ )
    {
        m_ConnectionSelectBrowser->add( convec[i]->MakeLabel().c_str() );

        convec[i]->SetDrawObjHighlight( false );
    }

    if ( m_ConnectionBrowserIndex >= 0 && convec.size() > 0 && m_ConnectionBrowserIndex < convec.size() )
    {
        FeaConnection* curr_conn = convec[ m_ConnectionBrowserIndex ];
        if ( curr_conn )
        {
            curr_conn->SetDrawObjHighlight( true );

            m_FeaConnModeChoice.Update( curr_conn->m_ConMode.GetID() );

            m_TxButton.Update( curr_conn->m_Constraints.GetID() );
            m_TyButton.Update( curr_conn->m_Constraints.GetID() );
            m_TzButton.Update( curr_conn->m_Constraints.GetID() );
            m_RxButton.Update( curr_conn->m_Constraints.GetID() );
            m_RyButton.Update( curr_conn->m_Constraints.GetID() );
            m_RzButton.Update( curr_conn->m_Constraints.GetID() );
        }
    }

    m_ConnectionSelectBrowser->select( m_ConnectionBrowserIndex + 2 );

    m_ConnectionSelectBrowser->position( scroll_pos );
    m_ConnectionSelectBrowser->hposition( h_pos );
}

void StructAssemblyScreen::UpdateDrawPartBrowser()
{
    //==== Draw Part Browser ====//
    int scroll_pos = m_DrawPartSelectBrowser->position();
    m_DrawPartSelectBrowser->clear();

    m_DrawBrowserMeshIDVec.clear();
    m_DrawBrowserIndexVec.clear();

    FeaAssembly* curr_assy = StructureMgr.GetFeaAssembly( StructureMgr.GetCurrAssemblyIndex() );

    if ( !curr_assy )
    {
        return;
    }

    vector < string > idvec = curr_assy->m_StructIDVec;
    for ( int i = 0; i < idvec.size(); i++ )
    {
        int indx = vector_find_val( m_StructIDs, idvec[i] );

        FeaMesh *mesh = FeaMeshMgr.GetMeshPtr( idvec[ i ] );

        if ( mesh )
        {
            vector < int > draw_browser_index_vec = mesh->GetDrawBrowserIndexVec();
            vector < string > draw_browser_name_vec = mesh->GetDrawBrowserNameVec();
            vector < bool > draw_element_flag_vec = mesh->GetDrawElementFlagVec();
            vector < bool > draw_cap_flag_vec = mesh->GetDrawCapFlagVec();

            for ( unsigned int j = 0; j < draw_browser_name_vec.size(); j++ )
            {
                m_DrawBrowserMeshIDVec.push_back( idvec[ i ] );
                m_DrawBrowserIndexVec.push_back( j );

                if ( draw_browser_name_vec[j].find( "CAP" ) != std::string::npos )
                {
                    m_DrawPartSelectBrowser->add( draw_browser_name_vec[j].c_str(), draw_cap_flag_vec[draw_browser_index_vec[j]] );
                }
                else
                {
                    m_DrawPartSelectBrowser->add( draw_browser_name_vec[j].c_str(), draw_element_flag_vec[draw_browser_index_vec[j]] );
                }
            }
        }
    }

    m_DrawPartSelectBrowser->position( scroll_pos );
}

void StructAssemblyScreen::AddOutputText( const string &text )
{
    Fl::lock();
    m_ConsoleBuffer->append( text.c_str() );
    m_ConsoleDisplay->insert_position( m_ConsoleDisplay->buffer()->length() );
    m_ConsoleDisplay->show_insert_position();
    Fl::unlock();
}

void StructAssemblyScreen::CallBack( Fl_Widget* w )
{
    assert( m_ScreenMgr );

    Vehicle*  veh = m_ScreenMgr->GetVehiclePtr();

    if ( veh )
    {
    }

    if ( w == m_AssemblySelectBrowser )
    {
        StructureMgr.SetCurrAssemblyIndex( m_AssemblySelectBrowser->value() - 2 );
    }
    else if ( w == m_StructureSelectBrowser )
    {
        m_StructureBrowserIndex = m_StructureSelectBrowser->value() - 2;
    }
    else if ( w == m_ConnectionSelectBrowser )
    {
        m_ConnectionBrowserIndex = m_ConnectionSelectBrowser->value() - 2;
    }
    else if ( w == m_DrawPartSelectBrowser )
    {
        int selected_index = m_DrawPartSelectBrowser->value();
        bool flag = !!m_DrawPartSelectBrowser->checked( selected_index );
        selected_index--;

        if ( selected_index >= 0 && selected_index < m_DrawBrowserMeshIDVec.size() && m_DrawBrowserMeshIDVec.size() > 0 )
        {
            string id = m_DrawBrowserMeshIDVec[ selected_index ];
            int index = m_DrawBrowserIndexVec[ selected_index ];

            FeaMesh *mesh = FeaMeshMgr.GetMeshPtr( id );

            if ( mesh )
            {
                vector < string > draw_browser_name_vec = mesh->GetDrawBrowserNameVec();
                vector < int > draw_browser_index_vec = mesh->GetDrawBrowserIndexVec();

                if ( draw_browser_name_vec[index].find( "CAP" ) != std::string::npos )
                {
                    mesh->SetDrawCapFlag( draw_browser_index_vec[index], flag );
                }
                else
                {
                    mesh->SetDrawElementFlag( draw_browser_index_vec[index], flag );
                }
            }
        }
    }

    m_ScreenMgr->SetUpdateFlag( true );
}



#ifdef WIN32
DWORD WINAPI feaassy_thread_fun( LPVOID data )
#else
void * feaassy_thread_fun( void *data )
#endif
{
    vector < string > *idvec = ( vector < string > * ) data;

    FeaMeshMgr.MeshUnMeshed( *idvec );

    return 0;
}

void StructAssemblyScreen::LaunchBatchFEAMesh( const vector < string > &idvec )
{
    FeaMeshMgr.SetFeaMeshInProgress( true );

    // Copy vector to memory that will persist through duration of meshing process.
    m_BatchIDs = idvec;

    m_FeaMeshProcess.StartThread( feaassy_thread_fun, ( void* ) &m_BatchIDs );
}

void StructAssemblyScreen::GuiDeviceCallBack( GuiDevice* device )
{
    assert( m_ScreenMgr );

    Vehicle*  veh = m_ScreenMgr->GetVehiclePtr();

    if ( !veh )
    {
        return;
    }

    FeaAssembly *curr_assy = NULL;
    if ( StructureMgr.ValidFeaAssemblyInd( StructureMgr.GetCurrAssemblyIndex() ) )
    {
        curr_assy = StructureMgr.GetFeaAssembly( StructureMgr.GetCurrAssemblyIndex() );
    }

    if ( device == &m_FeaReMeshAllButton )
    {
        if ( curr_assy )
        {
            FeaMeshMgr.CleanupMeshes( curr_assy->m_StructIDVec );
            LaunchBatchFEAMesh( curr_assy->m_StructIDVec );
        }
    }
    else if ( device == &m_FeaMeshUnmeshedButton )
    {
        if ( curr_assy )
        {
            LaunchBatchFEAMesh( curr_assy->m_StructIDVec );
        }
    }
    else if ( device == &m_FeaExportMeshButton )
    {
        if ( curr_assy )
        {
            FeaMeshMgr.ExportAssemblyMesh( curr_assy->GetID() );
        }
    }
    else if ( device == &m_AddAssemblyButton )
    {
        StructureMgr.AddFeaAssembly();
        StructureMgr.SetCurrAssemblyIndex( StructureMgr.NumFeaAssembly() - 1 );
    }
    else if ( device == &m_DelAssemblyButton )
    {
        if ( StructureMgr.ValidFeaAssemblyInd( StructureMgr.GetCurrAssemblyIndex() ) )
        {
            StructureMgr.DeleteFeaAssembly( StructureMgr.GetCurrAssemblyIndex() );

            StructureMgr.SetCurrAssemblyIndex( StructureMgr.GetCurrAssemblyIndex() - 1 );
        }
        else
        {
            StructureMgr.SetCurrAssemblyIndex( -1 );
        }
    }
    else if ( device == &m_AssemblyNameInput )
    {
        if ( curr_assy )
        {
            curr_assy->SetName( m_AssemblyNameInput.GetString() );
            curr_assy->ResetExportFileNames();
        }
    }
    else if ( device == &m_FeaStructureChoice )
    {
        m_StructureChoiceIndex = m_FeaStructureChoice.GetVal();
    }
    else if ( device == &m_AddFeaStructureButton )
    {
        if ( curr_assy )
        {
            curr_assy->AddStructure( m_StructIDs[m_StructureChoiceIndex] );
        }
    }
    else if ( device == &m_AddAllFeaStructureButton )
    {
        if ( curr_assy )
        {
            for ( int i = 0; i < m_StructIDs.size(); i++ )
            {
                curr_assy->AddStructure( m_StructIDs[i] );

                // Overwrite structure names to match Geom names.  Useful when debugging and creating a large number
                // of structures.
                // FeaStructure* fea_struct = StructureMgr.GetFeaStruct( m_StructIDs[i] );
                // string geomid = fea_struct->GetParentGeomID();
                // Geom *g = veh->FindGeom( geomid );
                // fea_struct->SetName( g->GetName() );
            }
        }
    }
    else if ( device == &m_DelFeaStructureButton )
    {
        if ( curr_assy )
        {
            if ( m_StructureBrowserIndex < curr_assy->m_StructIDVec.size() )
            {
                curr_assy->DelStructure( curr_assy->m_StructIDVec[ m_StructureBrowserIndex ] );
            }
        }
    }
    else if ( device == &m_ConnectionStartChoice )
    {
        m_ConnectionStartIndex = m_ConnectionStartChoice.GetVal();
    }
    else if ( device == &m_ConnectionEndChoice )
    {
        m_ConnectionEndIndex = m_ConnectionEndChoice.GetVal();
    }
    else if ( device == &m_ConnectionStartSurfIndxChoice )
    {
        m_ConnectionStartSurfIndex = m_ConnectionStartSurfIndxChoice.GetVal();
    }
    else if ( device == &m_ConnectionEndSurfIndxChoice )
    {
        m_ConnectionEndSurfIndex = m_ConnectionEndSurfIndxChoice.GetVal();
    }
    else if ( device == &m_AddConnectionButton )
    {
        if ( curr_assy )
        {
            if ( m_ConnectionStartIndex < m_FixPtIDs.size() &&
                 m_ConnectionStartIndex < m_FixPtStructIDs.size() &&
                 m_ConnectionEndIndex < m_FixPtIDs.size() &&
                 m_ConnectionEndIndex < m_FixPtStructIDs.size() )
            {
                curr_assy->AddConnection( m_FixPtIDs[m_ConnectionStartIndex], m_FixPtStructIDs[m_ConnectionStartIndex], m_ConnectionStartSurfIndex,
                                          m_FixPtIDs[m_ConnectionEndIndex], m_FixPtStructIDs[m_ConnectionEndIndex], m_ConnectionEndSurfIndex );
            }
        }
    }
    else if ( device == &m_DelConnectionButton )
    {
        if ( curr_assy )
        {
            curr_assy->DelConnection( m_ConnectionBrowserIndex );
        }
    }
    else if ( device == &m_SelectStlFile )
    {
        if ( curr_assy )
        {
            string newfile = m_ScreenMgr->GetSelectFileScreen()->FileChooser( "Select .stl file.", "*.stl" );
            if ( newfile.compare( "" ) != 0 )
            {
                curr_assy->m_AssemblySettings.SetExportFileName( newfile, vsp::FEA_STL_FILE_NAME );
            }
        }
    }
    else if ( device == &m_SelectMassFile )
    {
        if ( curr_assy )
        {
            string newfile = m_ScreenMgr->GetSelectFileScreen()->FileChooser( "Select mass .txt file.", "*.txt" );
            if ( newfile.compare( "" ) != 0 )
            {
                curr_assy->m_AssemblySettings.SetExportFileName( newfile, vsp::FEA_MASS_FILE_NAME );
            }
        }
    }
    else if ( device == &m_SelectNastFile )
    {
        if ( curr_assy )
        {
            string newfile = m_ScreenMgr->GetSelectFileScreen()->FileChooser( "Select NASTRAN .dat file.", "*.dat" );
            if ( newfile.compare( "" ) != 0 )
            {
                curr_assy->m_AssemblySettings.SetExportFileName( newfile, vsp::FEA_NASTRAN_FILE_NAME );
            }
        }
    }
    else if ( device == &m_SelectNkeyFile )
    {
        if ( curr_assy )
        {
            string newfile = m_ScreenMgr->GetSelectFileScreen()->FileChooser( "Select NASTRAN key file.", "*.nkey" );
            if ( newfile.compare( "" ) != 0 )
            {
                curr_assy->m_AssemblySettings.SetExportFileName( newfile, vsp::FEA_NKEY_FILE_NAME );
            }
        }
    }
    else if ( device == &m_SelectCalcFile  )
    {
        if ( curr_assy )
        {
            string newfile = m_ScreenMgr->GetSelectFileScreen()->FileChooser( "Select Calculix .dat file.", "*.dat" );
            if ( newfile.compare( "" ) != 0 )
            {
                curr_assy->m_AssemblySettings.SetExportFileName( newfile, vsp::FEA_CALCULIX_FILE_NAME );
            }
        }
    }
    else if ( device == &m_SelectGmshFile )
    {
        if ( curr_assy )
        {
            string newfile = m_ScreenMgr->GetSelectFileScreen()->FileChooser( "Select .msh file.", "*.msh" );
            if ( newfile.compare( "" ) != 0 )
            {
                curr_assy->m_AssemblySettings.SetExportFileName( newfile, vsp::FEA_GMSH_FILE_NAME );
            }
        }
    }

    m_ScreenMgr->SetUpdateFlag( true );
}

void StructAssemblyScreen::LoadDrawObjs( vector< DrawObj* > &draw_obj_vec )
{
    Vehicle* veh = m_ScreenMgr->GetVehiclePtr();

    if ( !veh )
    {
        return;
    }

    if ( IsShown() )
    {
        FeaAssembly *curr_assy = NULL;
        if ( StructureMgr.ValidFeaAssemblyInd( StructureMgr.GetCurrAssemblyIndex() ) )
        {
            curr_assy = StructureMgr.GetFeaAssembly( StructureMgr.GetCurrAssemblyIndex() );
        }

        if ( curr_assy )
        {
            // This makes sure connection DO's are updated.
            // does not need to be called every time, but they aren't in an update path
            // otherwise.
            curr_assy->Update();

            for ( int i = 0 ; i < ( int )curr_assy->m_ConnectionVec.size() ; i++ )
            {
                FeaConnection* conn = curr_assy->m_ConnectionVec[i];
                if ( conn )
                {
                    if ( FeaMeshMgr.GetAssemblySettingsPtr()->m_DrawAsMeshFlag )
                    {
                        std::vector < DrawObj * > connDO;

                        // Grab version to potentially modify.
                        conn->LoadDrawObjs( connDO );

                        // Modify each point if mesh is ready.
                        FeaMeshMgr.ModifyConnDO( conn, connDO );
                    }

                    // Grab pointers for graphics engine.
                    conn->LoadDrawObjs( draw_obj_vec );
                }
            }

            vector < string > idvec = curr_assy->m_StructIDVec;
            for ( int iid = 0; iid < idvec.size(); iid++ )
            {
                FeaMesh *mesh = FeaMeshMgr.GetMeshPtr( idvec[ iid ] );

                bool drewmesh = false;

                if ( mesh )
                {
                    if ( mesh->m_MeshReady && FeaMeshMgr.GetAssemblySettingsPtr()->m_DrawAsMeshFlag )
                    {
                        mesh->LoadDrawObjs( draw_obj_vec, FeaMeshMgr.GetAssemblySettingsPtr() );

                        drewmesh = true;
                    }
                }

                if ( !drewmesh )
                {
                    FeaStructure *fea_struct = StructureMgr.GetFeaStruct( idvec[ iid ] );

                    if ( fea_struct )
                    {
                        vector < FeaPart * > partvec = fea_struct->GetFeaPartVec();
                        for ( unsigned int i = 0; i < ( int ) partvec.size(); i++ )
                        {
                            partvec[ i ]->LoadDrawObjs( draw_obj_vec );
                        }

                        vector < SubSurface * > subsurf_vec = fea_struct->GetFeaSubSurfVec();
                        for ( unsigned int i = 0; i < ( int ) subsurf_vec.size(); i++ )
                        {
                            subsurf_vec[ i ]->LoadDrawObjs( draw_obj_vec );
                        }
                    }
                }
            }
        }
    }
}

bool StructAssemblyScreen::GetVisBndBox( BndBox &bbox )
{
    bool anyvisible = false;

    vector< DrawObj* > draw_obj_vec;
    LoadDrawObjs( draw_obj_vec );

    for(int j = 0; j < (int)draw_obj_vec.size(); j++)
    {
        if(draw_obj_vec[j]->m_Visible)
        {
            bbox.Update( draw_obj_vec[j]->m_PntVec );
            bbox.Update( draw_obj_vec[j]->m_PntMesh );
            anyvisible = true;
        }
    }

    return anyvisible;
}
