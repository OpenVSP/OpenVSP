//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//

// CfdMeshScreen.cpp: implementation of the CfdMeshScreen class.
//
//////////////////////////////////////////////////////////////////////

#ifdef WIN32
#undef _HAS_STD_BYTE
#define _HAS_STD_BYTE 0
#endif

#include "CfdMeshScreen.h"
#include "CfdMeshMgr.h"
#include "ModeMgr.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

#define CFD_CONSOLE_HEIGHT 200

CfdMeshScreen::CfdMeshScreen( ScreenMgr* mgr ) : TabScreen( mgr, 550, 472 + CFD_CONSOLE_HEIGHT, "CFD Mesh", "", CFD_CONSOLE_HEIGHT )
{
    m_Vehicle = m_ScreenMgr->GetVehiclePtr();

    m_FLTK_Window->callback( staticCloseCB, this );

    //=== Create Tabs ===//
    CreateGlobalTab();
    CreateDisplayTab();
    CreateOutputTab();
    CreateSourcesTab();
    CreateDomainTab();
    CreateWakesTab();

    //=== Create Console Area ===//
    m_ConsoleLayout.SetGroupAndScreen( m_FLTK_Window, this );

    m_ConsoleLayout.AddY( m_ConsoleLayout.GetRemainY() - CFD_CONSOLE_HEIGHT );

    m_ConsoleLayout.AddYGap();
    m_ConsoleLayout.AddX(5);

    m_ConsoleLayout.AddSubGroupLayout( m_BorderConsoleLayout, m_ConsoleLayout.GetRemainX() - 5,
                                       m_ConsoleLayout.GetRemainY() - 5 );

    m_ConsoleDisplay = m_BorderConsoleLayout.AddFlTerminal( m_BorderConsoleLayout.GetRemainY()
                                                               - m_BorderConsoleLayout.GetStdHeight()
                                                               - m_BorderConsoleLayout.GetGapHeight() );
    m_ConsoleDisplay->display_columns( 300 );
    m_ConsoleDisplay->history_lines( 1000 );

    m_FLTK_Window->resizable( m_ConsoleLayout.GetGroup() );
    m_ConsoleLayout.GetGroup()->resizable( m_BorderConsoleLayout.GetGroup() );
    m_BorderConsoleLayout.GetGroup()->resizable( m_ConsoleDisplay );

    m_BorderConsoleLayout.AddYGap();

    m_BorderConsoleLayout.AddButton(m_MeshAndExport, "Mesh and Export");
}

CfdMeshScreen::~CfdMeshScreen()
{
}

void CfdMeshScreen::CreateGlobalTab()
{
    Fl_Group* globalTab = AddTab( "Global" );
    Fl_Group* globalTabGroup = AddSubGroup( globalTab, 5 );

    m_GlobalTabLayout.SetGroupAndScreen( globalTabGroup, this );

    //=== GLOBAL TAB INIT ===//

    m_GlobalTabLayout.AddDividerBox("Global Mesh Control");

    m_GlobalTabLayout.AddYGap();

    int actionToggleButtonWidth = 45;
    int normalButtonWidth = 175;

    m_GlobalTabLayout.SetFitWidthFlag( false );
    m_GlobalTabLayout.SetSameLineFlag( true );

    m_GlobalTabLayout.SetButtonWidth( actionToggleButtonWidth );
    m_GlobalTabLayout.AddButton( m_MaxAbsToggle, "Abs" );
    m_GlobalTabLayout.AddButton( m_MaxRelToggle, "Rel" );

    m_GlobalTabLayout.SetFitWidthFlag( true );

    m_GlobalTabLayout.SetButtonWidth( normalButtonWidth - 2 * actionToggleButtonWidth );
    m_GlobalTabLayout.AddSlider( m_MaxEdgeLen, "Max Edge Len", 1.0, "%7.5f" );

    m_MaxEdgeLenToggleGroup.Init( this );
    m_MaxEdgeLenToggleGroup.AddButton( m_MaxAbsToggle.GetFlButton() );
    m_MaxEdgeLenToggleGroup.AddButton( m_MaxRelToggle.GetFlButton() );


    m_GlobalTabLayout.ForceNewLine();

    m_GlobalTabLayout.SetFitWidthFlag( false );
    m_GlobalTabLayout.SetSameLineFlag( true );

    m_GlobalTabLayout.SetButtonWidth( actionToggleButtonWidth );
    m_GlobalTabLayout.AddButton( m_MinAbsToggle, "Abs" );
    m_GlobalTabLayout.AddButton( m_MinRelToggle, "Rel" );

    m_GlobalTabLayout.SetFitWidthFlag( true );

    m_GlobalTabLayout.SetButtonWidth( normalButtonWidth - 2 * actionToggleButtonWidth );
    m_GlobalTabLayout.AddSlider( m_MinEdgeLen, "Min Edge Len", 1.0, "%7.5f" );

    m_MinEdgeLenToggleGroup.Init( this );
    m_MinEdgeLenToggleGroup.AddButton( m_MinAbsToggle.GetFlButton() );
    m_MinEdgeLenToggleGroup.AddButton( m_MinRelToggle.GetFlButton() );


    m_GlobalTabLayout.ForceNewLine();

    m_GlobalTabLayout.SetFitWidthFlag( false );
    m_GlobalTabLayout.SetSameLineFlag( true );

    m_GlobalTabLayout.SetButtonWidth( actionToggleButtonWidth );
    m_GlobalTabLayout.AddButton( m_GapAbsToggle, "Abs" );
    m_GlobalTabLayout.AddButton( m_GapRelToggle, "Rel" );

    m_GlobalTabLayout.SetFitWidthFlag( true );

    m_GlobalTabLayout.SetButtonWidth( normalButtonWidth - 2 * actionToggleButtonWidth );
    m_GlobalTabLayout.AddSlider( m_MaxGap, "Max Gap", 1.0, "%7.5f" );

    m_MaxGapToggleGroup.Init( this );
    m_MaxGapToggleGroup.AddButton( m_GapAbsToggle.GetFlButton() );
    m_MaxGapToggleGroup.AddButton( m_GapRelToggle.GetFlButton() );

    m_GlobalTabLayout.ForceNewLine();
    m_GlobalTabLayout.SetFitWidthFlag( true );
    m_GlobalTabLayout.SetSameLineFlag( false );

    m_GlobalTabLayout.SetButtonWidth( normalButtonWidth );

    m_GlobalTabLayout.AddSlider(m_NumCircleSegments, "Num Circle Segments", 100.0, "%7.5f");
    m_GlobalTabLayout.AddSlider(m_GrowthRatio, "Growth Ratio", 2.0, "%7.5f");

    m_GlobalTabLayout.AddYGap();
    m_GlobalTabLayout.AddButton(m_Rig3dGrowthLimit, "Rigorous 3D Growth Limiting");
    m_GlobalTabLayout.AddYGap();
    m_GlobalTabLayout.AddSlider( m_RelCurveTolSlider, "Curve Adaptation Tolerance", 0.01, "%7.5f" );
    m_GlobalTabLayout.AddYGap();
    m_GlobalTabLayout.AddDividerBox("Global Source Control");
    m_GlobalTabLayout.AddYGap();

    m_GlobalTabLayout.SetFitWidthFlag( false );
    m_GlobalTabLayout.SetSameLineFlag( true );

    m_GlobalTabLayout.SetButtonWidth(20.0);
    m_GlobalTabLayout.AddButton(m_GlobSrcAdjustLenLftLft, "<<");
    m_GlobalTabLayout.AddButton(m_GlobSrcAdjustLenLft, "<");
    m_GlobalTabLayout.AddLabel("Adjust Len", 100.0);
    m_GlobalTabLayout.AddButton(m_GlobSrcAdjustLenRht, ">");
    m_GlobalTabLayout.AddButton(m_GlobSrcAdjustLenRhtRht, ">>");
    m_GlobalTabLayout.AddX(45.0);
    m_GlobalTabLayout.AddButton(m_GlobSrcAdjustRadLftLft, "<<");
    m_GlobalTabLayout.AddButton(m_GlobSrcAdjustRadLft, "<");
    m_GlobalTabLayout.AddLabel("Adjust Rad", 100.0);
    m_GlobalTabLayout.AddButton(m_GlobSrcAdjustRadRht, ">");
    m_GlobalTabLayout.AddButton(m_GlobSrcAdjustRadRhtRht, ">>");
    m_GlobalTabLayout.ForceNewLine();
    m_GlobalTabLayout.AddYGap();

    m_GlobalTabLayout.SetFitWidthFlag( true );
    m_GlobalTabLayout.SetSameLineFlag( false );

    m_GlobalTabLayout.AddDividerBox("Geometry Control");
    m_GlobalTabLayout.AddYGap();
    m_GlobalTabLayout.AddButton(m_IntersectSubsurfaces, "Intersect Subsurfaces");
    m_GlobalTabLayout.AddYGap();

    int bw = m_GlobalTabLayout.GetRemainX() / 2.0;
    m_GlobalTabLayout.SetButtonWidth( bw );
    m_GlobalTabLayout.SetChoiceButtonWidth( bw );

    m_GlobalTabLayout.SetSameLineFlag( true );
    m_GlobalTabLayout.SetChoiceButtonWidth( 0 );
    m_GlobalTabLayout.SetFitWidthFlag( false );
    m_GlobalTabLayout.AddButton( m_SetToggle, "Normal Set:" );
    m_GlobalTabLayout.SetFitWidthFlag( true );
    m_GlobalTabLayout.AddChoice(m_UseSet, "", bw);
    m_GlobalTabLayout.ForceNewLine();

    m_GlobalTabLayout.SetSameLineFlag( false );
    m_GlobalTabLayout.SetChoiceButtonWidth( bw );
    m_GlobalTabLayout.AddChoice(m_UseDegenSet, "Degen Set:" );

    m_GlobalTabLayout.SetSameLineFlag( true );
    m_GlobalTabLayout.SetChoiceButtonWidth( 0 );
    m_GlobalTabLayout.SetFitWidthFlag( false );
    m_GlobalTabLayout.AddButton( m_ModeToggle, "Mode:" );
    m_GlobalTabLayout.SetFitWidthFlag( true );
    m_GlobalTabLayout.AddChoice(m_ModeChoice, "", bw );
    m_GlobalTabLayout.ForceNewLine();

    m_ModeSetToggleGroup.Init( this );
    m_ModeSetToggleGroup.AddButton( m_SetToggle.GetFlButton() );
    m_ModeSetToggleGroup.AddButton( m_ModeToggle.GetFlButton() );

    m_GlobalTabLayout.SetSameLineFlag( false );

    m_GlobalTabLayout.AddYGap();
    m_GlobalTabLayout.SetButtonWidth( 175.0 );

    m_GlobalTabLayout.AddButton( m_ToCubicToggle, "Demote Surfs to Cubic" );
    m_GlobalTabLayout.AddSlider( m_ToCubicTolSlider, "Cubic Tolerance", 10, "%5.4g", 0, true );

    m_GlobalTabLayout.AddYGap();
    m_GlobalTabLayout.AddButton( m_ConvertToQuadsToggle, "Convert to Quads" );

    globalTab->show();
}

void CfdMeshScreen::CreateDisplayTab()
{
    Fl_Group* displayTab = AddTab( "Display" );
    Fl_Group* displayTabGroup = AddSubGroup( displayTab, 5 );

    m_DisplayTabLayout.SetGroupAndScreen( displayTabGroup, this );
    m_DisplayTabLayout.SetButtonWidth( 175 );

    m_DisplayTabLayout.AddDividerBox( "Mesh Display" );
    m_DisplayTabLayout.AddYGap();
    m_DisplayTabLayout.AddButton(m_ShowMesh, "Show Mesh");

    m_DisplayTabLayout.SetButtonWidth( m_DisplayTabLayout.GetW() / 3.0 );

    m_DisplayTabLayout.SetFitWidthFlag( false );
    m_DisplayTabLayout.SetSameLineFlag( true );

    m_DisplayTabLayout.AddButton( m_ColorFaces, "Color Mesh");
    m_DisplayTabLayout.AddButton( m_ColorByTag, "By Tag" );
    m_DisplayTabLayout.AddButton( m_ColorByReason, "By Reason" );
    m_DisplayTabLayout.ForceNewLine();

    m_ColorByToggleGroup.Init( this );
    m_ColorByToggleGroup.AddButton( m_ColorByTag.GetFlButton() );
    m_ColorByToggleGroup.AddButton( m_ColorByReason.GetFlButton() );

    m_DisplayTabLayout.SetFitWidthFlag( true );
    m_DisplayTabLayout.SetSameLineFlag( false );

    m_DisplayTabLayout.AddButton(m_ShowBadEdgesAndTriangles, "Show Bad Edges and Triangles");
    m_DisplayTabLayout.AddYGap();

    m_DisplayTabLayout.SetFitWidthFlag( false );
    m_DisplayTabLayout.SetSameLineFlag( true );

    m_DisplayTabLayout.SetButtonWidth( m_DisplayTabLayout.GetW() / 2.0 );

    m_DisplayTabLayout.AddButton(m_ShowSourcesAndWakePreview, "Show Sources & Wake Preview");
    m_DisplayTabLayout.AddButton(m_ShowWake, "Show Wake");
    m_DisplayTabLayout.ForceNewLine();

    m_DisplayTabLayout.AddButton(m_ShowFarFieldPreview, "Show Far Field Preview");
    m_DisplayTabLayout.AddButton(m_ShowFarField, "Show Far Field");
    m_DisplayTabLayout.ForceNewLine();

    m_DisplayTabLayout.SetFitWidthFlag( true );
    m_DisplayTabLayout.SetSameLineFlag( false );
    m_DisplayTabLayout.AddButton(m_ShowSymmetryPlane, "Show Symmetry Plane");

    m_DisplayTabLayout.AddYGap();

    m_DisplayTabLayout.AddDividerBox( "Intersection Curve Display" );

    m_DisplayTabLayout.AddYGap();

    m_DisplayTabLayout.SetFitWidthFlag( false );
    m_DisplayTabLayout.SetSameLineFlag( true );

    m_DisplayTabLayout.AddButton( m_DrawIsect, "Show Intersection Curves");
    m_DisplayTabLayout.AddButton( m_DrawBorder, "Show Border Curves");
    m_DisplayTabLayout.ForceNewLine();
    m_DisplayTabLayout.AddButton( m_ShowCurve, "Show Curves");
    m_DisplayTabLayout.AddButton( m_ShowPts, "Show Points");
    m_DisplayTabLayout.ForceNewLine();
    m_DisplayTabLayout.AddButton( m_ShowRaw, "Show Raw Curve");
    m_DisplayTabLayout.AddButton( m_ShowBinAdapt, "Show Binary Adapted");
    m_DisplayTabLayout.ForceNewLine();

    m_DisplayTabLayout.SetFitWidthFlag( true );
    m_DisplayTabLayout.SetSameLineFlag( false );

    m_DisplayTabLayout.AddYGap();

    m_DisplayTabLayout.AddDividerBox( "Mesh Edge Length Reason Key" );

    m_DisplayTabLayout.AddYGap();

    vec3d c;


    m_DisplayTabLayout.AddButton( m_MaxLenConstraintLabel, "Max Edge Len" );
    c = 255 * DrawObj::Color( DrawObj::reasonColorMap( vsp::MAX_LEN_CONSTRAINT ) );
    m_MaxLenConstraintLabel.SetColor( fl_rgb_color( c.x(), c.y(), c.z() ) );


    m_DisplayTabLayout.SetFitWidthFlag( false );
    m_DisplayTabLayout.SetSameLineFlag( true );

    m_DisplayTabLayout.SetButtonWidth( m_DisplayTabLayout.GetW() / 3.0 );

    m_DisplayTabLayout.AddButton( m_GrowLimitCurvGapLabel, "Growth Ratio" );
    c = 255 * DrawObj::Color( DrawObj::reasonColorMap( vsp::GROW_LIMIT_CURV_GAP ) );
    m_GrowLimitCurvGapLabel.SetColor( fl_rgb_color( c.x(), c.y(), c.z() ) );

    m_DisplayTabLayout.AddButton( m_GrowLimitNCircSegLabel, "Growth Ratio" );
    c = 255 * DrawObj::Color( DrawObj::reasonColorMap( vsp::GROW_LIMIT_CURV_NCIRCSEG ) );
    m_GrowLimitNCircSegLabel.SetColor( fl_rgb_color( c.x(), c.y(), c.z() ) );

    m_DisplayTabLayout.AddButton( m_GrowLimitSourcesLabel, "Growth Ratio" );
    c = 255 * DrawObj::Color( DrawObj::reasonColorMap( vsp::GROW_LIMIT_SOURCES ) );
    m_GrowLimitSourcesLabel.SetColor( fl_rgb_color( c.x(), c.y(), c.z() ) );

    m_DisplayTabLayout.ForceNewLine();

    m_DisplayTabLayout.AddButton( m_CurvGapLabel, "Max Gap" );
    c = 255 * DrawObj::Color( DrawObj::reasonColorMap( vsp::CURV_GAP ) );
    m_CurvGapLabel.SetColor( fl_rgb_color( c.x(), c.y(), c.z() ) );

    m_DisplayTabLayout.AddButton( m_NCircSegLabel, "Num Circle Segments" );
    c = 255 * DrawObj::Color( DrawObj::reasonColorMap( vsp::CURV_NCIRCSEG ) );
    m_NCircSegLabel.SetColor( fl_rgb_color( c.x(), c.y(), c.z() ) );

    m_DisplayTabLayout.AddButton( m_SourcesLabel, "Sources" );
    c = 255 * DrawObj::Color( DrawObj::reasonColorMap( vsp::SOURCES ) );
    m_SourcesLabel.SetColor( fl_rgb_color( c.x(), c.y(), c.z() ) );

    m_DisplayTabLayout.ForceNewLine();

    m_DisplayTabLayout.AddButton( m_MinLenCurvGapLabel, "Min Edge Len" );
    c = 255 * DrawObj::Color( DrawObj::reasonColorMap( vsp::MIN_LEN_CONSTRAINT_CURV_GAP ) );
    m_MinLenCurvGapLabel.SetColor( fl_rgb_color( c.x(), c.y(), c.z() ) );
    m_MinLenCurvGapLabel.SetLabelColor( FL_WHITE );

    m_DisplayTabLayout.AddButton( m_MinLenCurvNCircSegLabel, "Min Edge Len" );
    c = 255 * DrawObj::Color( DrawObj::reasonColorMap( vsp::MIN_LEN_CONSTRAINT_CURV_NCIRCSEG ) );
    m_MinLenCurvNCircSegLabel.SetColor( fl_rgb_color( c.x(), c.y(), c.z() ) );
    m_MinLenCurvNCircSegLabel.SetLabelColor( FL_WHITE );


    displayTab->show();
}

void CfdMeshScreen::CreateOutputTab()
{
    Fl_Group* outputTab = AddTab( "Output" );
    Fl_Group* outputTabGroup = AddSubGroup( outputTab, 5 );

    m_OutputTabLayout.SetGroupAndScreen( outputTabGroup, this );

    m_OutputTabLayout.AddDividerBox("Export File Names");
    m_OutputTabLayout.AddYGap();

    m_OutputTabLayout.SetFitWidthFlag( false );
    m_OutputTabLayout.SetSameLineFlag( true );


    int typebuttonw = 80;
    m_OutputTabLayout.SetButtonWidth(typebuttonw);
    m_OutputTabLayout.SetInputWidth( m_OutputTabLayout.GetW() - typebuttonw - 30 );

    m_OutputTabLayout.AddButton(m_StlFile, ".stl");
    m_OutputTabLayout.AddOutput(m_StlOutput);
    m_OutputTabLayout.SetButtonWidth( m_OutputTabLayout.GetRemainX() );
    m_OutputTabLayout.AddButton(m_SelectStlFile, "...");
    m_OutputTabLayout.ForceNewLine();

    m_OutputTabLayout.SetFitWidthFlag( true );
    m_OutputTabLayout.AddButton(m_TaggedMultiSolid, "Tagged Multi Sold STL (Non-Standard)");
    m_OutputTabLayout.SetFitWidthFlag( false );
    m_OutputTabLayout.ForceNewLine();
    m_OutputTabLayout.AddYGap();

    m_OutputTabLayout.SetButtonWidth( typebuttonw );
    m_OutputTabLayout.AddButton(m_PolyFile, ".poly");
    m_OutputTabLayout.AddOutput(m_PolyOutput);
    m_OutputTabLayout.SetButtonWidth( m_OutputTabLayout.GetRemainX() );
    m_OutputTabLayout.AddButton(m_SelectPolyFile, "...");
    m_OutputTabLayout.ForceNewLine();
    m_OutputTabLayout.SetButtonWidth( typebuttonw );
    m_OutputTabLayout.AddButton(m_TriFile, ".tri");
    m_OutputTabLayout.AddOutput(m_TriOutput);
    m_OutputTabLayout.SetButtonWidth( m_OutputTabLayout.GetRemainX() );
    m_OutputTabLayout.AddButton(m_SelectTriFile, "...");
    m_OutputTabLayout.ForceNewLine();
    m_OutputTabLayout.SetButtonWidth( typebuttonw );
    m_OutputTabLayout.AddButton( m_FacFile, ".facet" );
    m_OutputTabLayout.AddOutput( m_FacOutput );
    m_OutputTabLayout.SetButtonWidth( m_OutputTabLayout.GetRemainX() );
    m_OutputTabLayout.AddButton( m_SelectFacFile, "..." );
    m_OutputTabLayout.ForceNewLine();
    m_OutputTabLayout.SetButtonWidth( typebuttonw );
    m_OutputTabLayout.AddButton(m_ObjFile, ".obj");
    m_OutputTabLayout.AddOutput(m_ObjOutput);
    m_OutputTabLayout.SetButtonWidth( m_OutputTabLayout.GetRemainX() );
    m_OutputTabLayout.AddButton(m_SelectObjFile, "...");
    m_OutputTabLayout.ForceNewLine();
    m_OutputTabLayout.SetButtonWidth( typebuttonw );
    m_OutputTabLayout.AddButton(m_MshFile, ".msh");
    m_OutputTabLayout.AddOutput(m_MshOutput);
    m_OutputTabLayout.SetButtonWidth( m_OutputTabLayout.GetRemainX() );
    m_OutputTabLayout.AddButton(m_SelectMshFile, "...");
    m_OutputTabLayout.ForceNewLine();
    m_OutputTabLayout.SetButtonWidth( typebuttonw );
    m_OutputTabLayout.AddButton(m_VspgeomFile, ".vspgeom");
    m_OutputTabLayout.AddOutput(m_VspgeomOutput);
    m_OutputTabLayout.SetButtonWidth( m_OutputTabLayout.GetRemainX() );
    m_OutputTabLayout.AddButton(m_SelectVspgeomFile, "...");
    m_OutputTabLayout.ForceNewLine();
    m_OutputTabLayout.AddYGap();

    m_OutputTabLayout.SetFitWidthFlag( true );
    m_OutputTabLayout.AddDividerBox("NASCART");
    m_OutputTabLayout.ForceNewLine();
    m_OutputTabLayout.SetFitWidthFlag( false );

    m_OutputTabLayout.SetButtonWidth( typebuttonw );
    m_OutputTabLayout.AddButton(m_DatFile, ".dat");
    m_OutputTabLayout.AddOutput(m_DatOutput);
    m_OutputTabLayout.SetButtonWidth( m_OutputTabLayout.GetRemainX() );
    m_OutputTabLayout.AddButton(m_SelectDatFile, "...");
    m_OutputTabLayout.ForceNewLine();
    m_OutputTabLayout.SetButtonWidth( typebuttonw );
    m_OutputTabLayout.AddButton(m_KeyFile, ".key");
    m_OutputTabLayout.AddOutput(m_KeyOutput);
    m_OutputTabLayout.SetButtonWidth( m_OutputTabLayout.GetRemainX() );
    m_OutputTabLayout.AddButton(m_SelectKeyFile, "...");
    m_OutputTabLayout.ForceNewLine();
    m_OutputTabLayout.AddYGap();

    m_OutputTabLayout.SetFitWidthFlag( true );
    m_OutputTabLayout.AddDividerBox("Surface and Subsurface Key File");
    m_OutputTabLayout.ForceNewLine();
    m_OutputTabLayout.SetFitWidthFlag( false );

    m_OutputTabLayout.SetButtonWidth( typebuttonw );
    m_OutputTabLayout.AddButton(m_TkeyFile, ".tkey");
    m_OutputTabLayout.AddOutput(m_TkeyOutput);
    m_OutputTabLayout.SetButtonWidth( m_OutputTabLayout.GetRemainX() );
    m_OutputTabLayout.AddButton(m_SelectTkeyFile, "...");
    m_OutputTabLayout.ForceNewLine();
    m_OutputTabLayout.AddYGap();

    outputTab->show();
}

void CfdMeshScreen::CreateSourcesTab()
{
    Fl_Group* sourcesTab = AddTab( "Sources" );
    Fl_Group* sourcesTabGroup = AddSubGroup( sourcesTab, 5 );

    m_SourcesTabLayout.SetGroupAndScreen( sourcesTabGroup, this );

    m_SourcesTabLayout.AddYGap();

    m_SourcesTabLayout.SetFitWidthFlag( true );
    m_SourcesTabLayout.SetSameLineFlag( true );

    m_SourcesTabLayout.SetChoiceButtonWidth(100);
    m_SourcesTabLayout.AddChoice(m_SourcesSelectComp, "Select Comp", 140);
    m_SourcesTabLayout.SetFitWidthFlag( false );
    m_SourcesTabLayout.SetButtonWidth(140);
    m_SourcesTabLayout.AddButton(m_AddDefaultSources, "Add Default Sources");
    m_SourcesTabLayout.ForceNewLine();
    m_SourcesTabLayout.AddYGap();

    m_SourcesTabLayout.SetFitWidthFlag( true );
    m_SourcesTabLayout.AddChoice(m_SourcesSelectSurface, "Select Surface");
    m_SourcesTabLayout.ForceNewLine();
    m_SourcesTabLayout.AddYGap();

    m_SourcesTabLayout.AddDividerBox("Sources");
    m_SourcesTabLayout.ForceNewLine();

    m_SourcesTabLayout.AddSubGroupLayout(m_SourcesLeft, m_SourcesTabLayout.GetRemainX() / 2.0 - 5.0, 300);
    m_SourcesTabLayout.AddX(m_SourcesTabLayout.GetRemainX() / 2.0 + 5.0);
    m_SourcesTabLayout.AddSubGroupLayout(m_SourcesRight, m_SourcesTabLayout.GetRemainX() - 5.0, 300);

    m_SourcesLeft.SetFitWidthFlag( true );
    m_SourcesLeft.SetSameLineFlag( false );

    m_SourcesLeft.SetChoiceButtonWidth(50);
    m_SourcesType.AddItem("Point");
    m_SourcesType.AddItem("Line");
    m_SourcesType.AddItem("Box");
    m_SourcesType.AddItem("U Line");
    m_SourcesType.AddItem("W Line");
    m_SourcesLeft.AddChoice(m_SourcesType, "Type");
    m_SourcesLeft.AddYGap();
    m_SourcesLeft.AddButton(m_AddSource, "Add Source");
    m_SourcesLeft.AddYGap();
    m_SourcesLeft.AddButton(m_DeleteSource, "Delete Source");

    m_SourceBrowser = m_SourcesRight.AddFlBrowser(75);
    m_SourceBrowser->callback( staticScreenCB, this );

    m_SourcesTabLayout.ForceNewLine();
    m_SourcesTabLayout.ForceNewLine();
    m_SourcesTabLayout.ForceNewLine();
    m_SourcesTabLayout.ForceNewLine();

    m_SourcesTabLayout.AddDividerBox("Edit Source");

    m_SourcesTabLayout.ForceNewLine();

    m_SourcesTabLayout.AddInput(m_SourceName, "Name:");

    m_SourcesTabLayout.ForceNewLine();
    m_SourcesTabLayout.AddYGap();

    m_SourcesTabLayout.AddSubGroupLayout(m_UWPosition1, m_SourcesTabLayout.GetRemainX() / 2.0 - 5.0, 400);
    m_SourcesTabLayout.AddX(m_SourcesTabLayout.GetRemainX() / 2.0 + 5.0);
    m_SourcesTabLayout.AddSubGroupLayout(m_UWPosition2, m_SourcesTabLayout.GetRemainX() - 5.0, 400);

    m_UWPosition1.AddDividerBox("UW Position 1");
    m_UWPosition1.AddYGap();
    m_UWPosition1.SetButtonWidth(40);
    m_UWPosition1.AddSlider(m_SourceU1, "U", 1.0, "%7.3f");
    m_UWPosition1.AddSlider(m_SourceW1, "W", 1.0, "%7.3f");
    m_UWPosition1.AddYGap();
    m_UWPosition1.AddSlider(m_SourceRad1, "Rad", 1.0, "%7.2g");
    m_UWPosition1.AddSlider(m_SourceLen1, "Len", 1.0, "%7.2g");

    m_UWPosition2.AddDividerBox("UW Position 2");
    m_UWPosition2.AddYGap();
    m_UWPosition2.SetButtonWidth(40);
    m_UWPosition2.AddSlider(m_SourceU2, "U", 1.0, "%7.3f");
    m_UWPosition2.AddSlider(m_SourceW2, "W", 1.0, "%7.3f");
    m_UWPosition2.AddYGap();
    m_UWPosition2.AddSlider(m_SourceRad2, "Rad", 1.0, "%7.2g");
    m_UWPosition2.AddSlider(m_SourceLen2, "Len", 1.0, "%7.2g");

    sourcesTab->show();
}

void CfdMeshScreen::CreateDomainTab()
{
    Fl_Group* domainTab = AddTab( "Domain" );
    Fl_Group* domainTabGroup = AddSubGroup( domainTab, 5 );

    m_DomainTabLayout.SetGroupAndScreen( domainTabGroup, this );

    m_DomainTabLayout.AddYGap();

    m_DomainTabLayout.AddButton(m_GenerateHalfMesh, "Generate Half Mesh");
    m_DomainTabLayout.AddYGap();
    m_DomainTabLayout.AddButton(m_GenerateFarFieldMesh, "Generate Far Field Mesh");
    m_DomainTabLayout.AddYGap();

    m_DomainTabLayout.AddSubGroupLayout(m_FarParametersLayout, m_DomainTabLayout.GetRemainX(),
                                        m_DomainTabLayout.GetRemainY());

    m_FarParametersLayout.AddDividerBox("Far Field Mesh Control");
    m_FarParametersLayout.AddYGap();
    m_FarParametersLayout.SetButtonWidth(m_DomainTabLayout.GetRemainX() / 2.0);

    int actionToggleButtonWidth = 45;
    int normalButtonWidth = 175;

    m_FarParametersLayout.SetFitWidthFlag( false );
    m_FarParametersLayout.SetSameLineFlag( true );

    m_FarParametersLayout.SetButtonWidth( actionToggleButtonWidth );
    m_FarParametersLayout.AddButton( m_DomainMaxAbsToggle, "Abs" );
    m_FarParametersLayout.AddButton( m_DomainMaxRelToggle, "Rel" );

    m_FarParametersLayout.SetFitWidthFlag( true );

    m_FarParametersLayout.SetButtonWidth( normalButtonWidth - 2 * actionToggleButtonWidth );
    m_FarParametersLayout.AddSlider( m_DomainMaxEdgeLen, "Max Edge Len", 1.0, "%7.5f" );

    m_DomainMaxEdgeLenToggleGroup.Init( this );
    m_DomainMaxEdgeLenToggleGroup.AddButton( m_DomainMaxAbsToggle.GetFlButton() );
    m_DomainMaxEdgeLenToggleGroup.AddButton( m_DomainMaxRelToggle.GetFlButton() );


    m_FarParametersLayout.ForceNewLine();

    m_FarParametersLayout.SetFitWidthFlag( false );
    m_FarParametersLayout.SetSameLineFlag( true );

    m_FarParametersLayout.SetButtonWidth( actionToggleButtonWidth );
    m_FarParametersLayout.AddButton( m_DomainGapAbsToggle, "Abs" );
    m_FarParametersLayout.AddButton( m_DomainGapRelToggle, "Rel" );

    m_FarParametersLayout.SetFitWidthFlag( true );

    m_FarParametersLayout.SetButtonWidth( normalButtonWidth - 2 * actionToggleButtonWidth );
    m_FarParametersLayout.AddSlider( m_DomainMaxGap, "Max Gap", 1.0, "%7.5f" );

    m_DomainMaxGapToggleGroup.Init( this );
    m_DomainMaxGapToggleGroup.AddButton( m_DomainGapAbsToggle.GetFlButton() );
    m_DomainMaxGapToggleGroup.AddButton( m_DomainGapRelToggle.GetFlButton() );


    m_FarParametersLayout.ForceNewLine();
    m_FarParametersLayout.SetFitWidthFlag( true );
    m_FarParametersLayout.SetSameLineFlag( false );

    m_FarParametersLayout.SetButtonWidth( normalButtonWidth );


    m_FarParametersLayout.AddSlider(m_DomainNumCircleSegments, "Num Circle Segments", 100.0, "%7.5f");
    m_FarParametersLayout.AddYGap();

    //=== Far Field Type Area ===//
    m_FarParametersLayout.AddDividerBox("Far Field Type");
    m_FarParametersLayout.AddYGap();

    m_FarParametersLayout.SetFitWidthFlag( false );
    m_FarParametersLayout.SetSameLineFlag( true );

    m_FarParametersLayout.SetButtonWidth(100);
    m_FarParametersLayout.AddButton(m_FarFieldTypeComponent, "Component:", -1);
    m_FarParametersLayout.SetFitWidthFlag( true );
    m_FarParametersLayout.SetChoiceButtonWidth(0);
    m_FarParametersLayout.AddChoice(m_ComponentChoice, "", 100);

    m_FarParametersLayout.ForceNewLine();
    m_FarParametersLayout.SetFitWidthFlag( false );
    m_FarParametersLayout.AddButton(m_FarFieldTypeBox, "Box", 1);
    m_FarFieldTypeBox.GetFlButton()->value( 1 );

    m_FarParametersLayout.ForceNewLine();

    m_FarParametersLayout.AddSubGroupLayout(m_FarBoxLayout, m_FarParametersLayout.GetRemainX(),
                                            m_FarParametersLayout.GetRemainY());

    //=== Symmetry Plane Splitting Toggle Button ===//
    m_FarBoxLayout.AddButton(m_SymPlaneSplit, "Symmetry Plane Splitting");
    m_FarBoxLayout.AddYGap();

    //=== Size Area ===//
    m_FarBoxLayout.SetFitWidthFlag( true );
    m_FarBoxLayout.SetSameLineFlag( true );

    m_FarBoxLayout.AddDividerBox("Size", 120);

    m_FarBoxLayout.SetFitWidthFlag( false );

    m_FarBoxLayout.SetButtonWidth(60.0);
    m_FarBoxLayout.AddButton(m_DomainRel, "Rel");
    m_FarBoxLayout.AddButton(m_DomainAbs, "Abs");
    m_DomainRel.GetFlButton()->value( 1 );

    m_FarBoxLayout.ForceNewLine();

    m_FarBoxLayout.SetFitWidthFlag( true );
    m_FarBoxLayout.SetSameLineFlag( false );

    m_FarBoxLayout.AddYGap();
    m_FarBoxLayout.SetButtonWidth(m_DomainTabLayout.GetRemainX() / 3.0);
    m_FarBoxLayout.AddSlider(m_DomainLength, "Length", 10.0, "%7.5f");
    m_FarBoxLayout.AddSlider(m_DomainWidth, "Width", 10.0, "%7.5f");
    m_FarBoxLayout.AddSlider(m_DomainHeight, "Height", 10.0, "%7.5f");
    m_FarBoxLayout.AddYGap();

    //=== Location Area ===//
    m_FarBoxLayout.SetFitWidthFlag( true );
    m_FarBoxLayout.SetSameLineFlag( true );

    m_FarBoxLayout.AddDividerBox("Location", 120);

    m_FarBoxLayout.SetFitWidthFlag( false );

    m_FarBoxLayout.SetButtonWidth(60.0);
    m_FarBoxLayout.AddButton(m_DomainCen, "Cen");
    m_FarBoxLayout.AddButton(m_DomainMan, "Man");
    m_DomainCen.GetFlButton()->value( 1 );

    m_FarBoxLayout.ForceNewLine();

    m_FarBoxLayout.SetFitWidthFlag( true );
    m_FarBoxLayout.SetSameLineFlag( false );

    m_FarBoxLayout.AddYGap();

    m_FarBoxLayout.AddSubGroupLayout(m_FarXYZLocationLayout, m_FarBoxLayout.GetRemainX(),
                                     m_FarBoxLayout.GetRemainY());

    m_FarXYZLocationLayout.SetButtonWidth(m_DomainTabLayout.GetRemainX() / 3.0);
    m_FarXYZLocationLayout.AddSlider(m_DomainXLoc, "X Loc", 5.0, "%7.5f");
    m_FarXYZLocationLayout.AddSlider(m_DomainYLoc, "Y Loc", 5.0, "%7.5f");
    m_FarXYZLocationLayout.AddSlider(m_DomainZLoc, "Z Loc", 5.0, "%7.5f");
    m_FarXYZLocationLayout.AddYGap();

    domainTab->show();
}

void CfdMeshScreen::CreateWakesTab()
{
    m_WakesTab = AddTab( "Wakes" );
    Fl_Group* wakesTabGroup = AddSubGroup( m_WakesTab, 5 );

    m_WakesTabLayout.SetGroupAndScreen( wakesTabGroup, this );

    m_WakesTabLayout.AddYGap();

    m_WakesTabLayout.SetButtonWidth(175.0);
    m_WakesTabLayout.AddSlider(m_ScaleWake, "Scale Wake", 10.0, "%7.5f");
    m_WakesTabLayout.AddYGap();
    m_WakesTabLayout.AddSlider(m_WakeAngle, "Wake Angle", 10.0, "%7.5f");

    m_WakesTabLayout.ForceNewLine();

    m_WakesTabLayout.SetFitWidthFlag( true );
    m_WakesTabLayout.SetSameLineFlag( true );

    m_WakesTabLayout.SetChoiceButtonWidth(100);
    m_WakesTabLayout.AddChoice(m_Comp, "Comp", 100);

    m_WakesTabLayout.SetFitWidthFlag(false);
    m_WakesTabLayout.SetButtonWidth(100);
    m_WakesTabLayout.AddButton(m_AddWake, "Add Wake");

    m_WakesTab->show();
}

bool CfdMeshScreen::Update()
{
    TabScreen::Update();

    m_ScreenMgr->LoadSetChoice( {&m_UseSet, &m_UseDegenSet},
        {m_Vehicle->GetCfdSettingsPtr()->m_SelectedSetIndex.GetID(),
        m_Vehicle->GetCfdSettingsPtr()->m_SelectedDegenSetIndex.GetID()}, true );

    m_ScreenMgr->LoadModeChoice( m_ModeChoice, m_ModeIDs, m_Vehicle->GetCfdSettingsPtr()->m_ModeID );

    m_ModeSetToggleGroup.Update( m_Vehicle->GetCfdSettingsPtr()->m_UseMode.GetID() );

    if ( ModeMgr.GetNumModes() == 0 )
    {
        if ( m_Vehicle->GetCfdSettingsPtr()->m_UseMode() )
        {
            m_Vehicle->GetCfdSettingsPtr()->m_UseMode.Set( false );
            m_ScreenMgr->SetUpdateFlag( true );
        }
        m_ModeToggle.Deactivate();
    }
    else
    {
        m_ModeToggle.Activate();
    }

    if ( m_Vehicle->GetCfdSettingsPtr()->m_UseMode() )
    {
        m_ModeChoice.Activate();
        m_UseSet.Deactivate();
        m_UseDegenSet.Deactivate();

        Mode *m = ModeMgr.GetMode( m_Vehicle->GetCfdSettingsPtr()->m_ModeID );
        if ( m )
        {
            if ( m_Vehicle->GetCfdSettingsPtr()->m_SelectedSetIndex() != m->m_NormalSet() ||
                 m_Vehicle->GetCfdSettingsPtr()->m_SelectedDegenSetIndex() != m->m_DegenSet() )
            {
                m_Vehicle->GetCfdSettingsPtr()->m_SelectedSetIndex = m->m_NormalSet();
                m_Vehicle->GetCfdSettingsPtr()->m_SelectedDegenSetIndex = m->m_DegenSet();
                m_ScreenMgr->SetUpdateFlag( true );
            }
        }
        else
        {
            if ( m_ModeIDs.size() > 0 )
            {
                m_Vehicle->GetCfdSettingsPtr()->m_ModeID = m_ModeIDs[ 0 ];
            }
        }
    }
    else
    {
        m_ModeChoice.Deactivate();
        m_UseSet.Activate();
        m_UseDegenSet.Activate();
    }


    if ( CfdMeshMgr.GetMeshInProgress() )
    {
        m_MeshAndExport.Deactivate();
    }
    else
    {
        m_MeshAndExport.Activate();
    }

    CfdMeshMgr.Update();

    //==== Load Geom Choice ====//
    m_GeomVec = m_Vehicle->GetGeomVec();

    m_SourcesSelectComp.ClearItems();
    m_ComponentChoice.ClearItems();
    m_Comp.ClearItems();

    unordered_map< string, int > compIDMap;
    unordered_map< string, int > wingCompIDMap;
    m_WingGeomVec.clear();


    int iwing = 0;
    for ( int i = 0 ; i < ( int )m_GeomVec.size() ; ++i )
    {
        char str[256];
        Geom* g = m_Vehicle->FindGeom( m_GeomVec[i] );
        if ( g )
        {
            snprintf( str, sizeof( str ),  "%d_%s", i, g->GetName().c_str() );
            m_SourcesSelectComp.AddItem( str );
            if( g->HasWingTypeSurfs() )
            {
                m_Comp.AddItem( str );
                wingCompIDMap[ m_GeomVec[i] ] = iwing++;
                m_WingGeomVec.push_back( m_GeomVec[i] );
            }
            m_ComponentChoice.AddItem( str );
            compIDMap[ m_GeomVec[i] ] = i;
        }
    }
    m_SourcesSelectComp.UpdateItems();
    m_Comp.UpdateItems();
    m_ComponentChoice.UpdateItems();

    //===== Set Current Source and component for Sources =====//
    string currSourceGeomID = CfdMeshMgr.GetCurrSourceGeomID();
    if( currSourceGeomID.length() == 0 && m_GeomVec.size() > 0 )
    {
        // Handle case default case.
        currSourceGeomID = m_GeomVec[0];
        CfdMeshMgr.SetCurrSourceGeomID( currSourceGeomID );
    }
    Geom* currGeom = m_Vehicle->FindGeom( currSourceGeomID );
    m_SourcesSelectComp.SetVal( compIDMap[ currSourceGeomID ] );

    //===== Set FarGeomID and Far Component Selection for Domain Tab =====//
    string farGeomID = m_Vehicle->GetCfdSettingsPtr()->GetFarGeomID();
    if( farGeomID.length() == 0 && m_GeomVec.size() > 0 )
    {
        // Handle case default case.
        farGeomID = m_GeomVec[0];
        m_Vehicle->GetCfdSettingsPtr()->SetFarGeomID( farGeomID );
    }
    m_ComponentChoice.SetVal( compIDMap[ farGeomID ] );

    //===== Set WakeGeomID and wake component selection for wake tab =====//
    string wakeGeomID = CfdMeshMgr.GetWakeGeomID();
    Geom* wakeGeom = m_Vehicle->FindGeom( wakeGeomID );

    if( ( !wakeGeom || wakeGeomID.length() == 0 ) && m_WingGeomVec.size() > 0 )
    {
        // Handle case default case.
        wakeGeomID = m_WingGeomVec[0];
        CfdMeshMgr.SetWakeGeomID( wakeGeomID );
        wakeGeom = m_Vehicle->FindGeom( wakeGeomID );
    }

    m_Comp.SetVal( wingCompIDMap[ wakeGeomID ] );

    //===== Update Sources =====//
    BaseSource* source = CfdMeshMgr.GetCurrSource();

    //==== Load Up Source Browser ====//
    int currSourceID = -1;

    m_SourceBrowser->clear();
    m_SourcesSelectSurface.ClearItems();

    if( currGeom )
    {
        vector< BaseSource* > sVec = currGeom->GetCfdMeshMainSourceVec();
        for ( int i = 0 ; i < ( int )sVec.size() ; ++i )
        {
            if ( source == sVec[i] )
            {
                currSourceID = i;
            }
            m_SourceBrowser->add( sVec[i]->GetName().c_str() );
        }
        if ( currSourceID >= 0 && currSourceID < ( int )sVec.size() )
        {
            m_SourceBrowser->select( currSourceID + 1 );
        }

        int nmain = currGeom->GetNumMainSurfs();
        for ( int i = 0; i < nmain; ++i )
        {
            char str[256];
            snprintf( str, sizeof( str ),  "Surf_%d", i );
            m_SourcesSelectSurface.AddItem( str );
        }
        m_SourcesSelectSurface.UpdateItems();

        int currMainSurfID = CfdMeshMgr.GetCurrMainSurfIndx();
        if( currMainSurfID >= 0 && currMainSurfID < nmain )
        {
            m_SourcesSelectSurface.SetVal( currMainSurfID );
        }
    }

    UpdateGlobalTab();
    UpdateDisplayTab();
    UpdateOutputTab();
    UpdateSourcesTab(source);
    UpdateDomainTab();

    if ( m_WingGeomVec.size() == 0 )
    {
        m_WakesTab->deactivate();
    }
    else
    {
        m_WakesTab->activate();
        UpdateWakesTab();
    }

    m_FLTK_Window->redraw();

    return false;
}

void CfdMeshScreen::UpdateGlobalTab()
{
    //===== Global Mesh Control =====//
    m_MaxEdgeLenToggleGroup.Update( m_Vehicle->GetCfdGridDensityPtr()->m_BaseAbsRel.GetID() );
    if ( m_Vehicle->GetCfdGridDensityPtr()->m_BaseAbsRel() == vsp::ABS )
    {
        m_MaxEdgeLen.Update( 1, m_Vehicle->GetCfdGridDensityPtr()->m_BaseLen.GetID(), m_Vehicle->GetCfdGridDensityPtr()->m_BaseFrac.GetID() );
    }
    else
    {
        m_MaxEdgeLen.Update( 2, m_Vehicle->GetCfdGridDensityPtr()->m_BaseLen.GetID(), m_Vehicle->GetCfdGridDensityPtr()->m_BaseFrac.GetID() );
    }

    m_MinEdgeLenToggleGroup.Update( m_Vehicle->GetCfdGridDensityPtr()->m_MinAbsRel.GetID() );
    if ( m_Vehicle->GetCfdGridDensityPtr()->m_MinAbsRel() == vsp::ABS )
    {
        m_MinEdgeLen.Update( 1, m_Vehicle->GetCfdGridDensityPtr()->m_MinLen.GetID(), m_Vehicle->GetCfdGridDensityPtr()->m_MinFrac.GetID() );
    }
    else
    {
        m_MinEdgeLen.Update( 2, m_Vehicle->GetCfdGridDensityPtr()->m_MinLen.GetID(), m_Vehicle->GetCfdGridDensityPtr()->m_MinFrac.GetID() );
    }

    m_MaxGapToggleGroup.Update( m_Vehicle->GetCfdGridDensityPtr()->m_MaxGapAbsRel.GetID() );
    if ( m_Vehicle->GetCfdGridDensityPtr()->m_MaxGapAbsRel() == vsp::ABS )
    {
        m_MaxGap.Update( 1, m_Vehicle->GetCfdGridDensityPtr()->m_MaxGap.GetID(), m_Vehicle->GetCfdGridDensityPtr()->m_MaxGapFrac.GetID() );
    }
    else
    {
        m_MaxGap.Update( 2, m_Vehicle->GetCfdGridDensityPtr()->m_MaxGap.GetID(), m_Vehicle->GetCfdGridDensityPtr()->m_MaxGapFrac.GetID() );
    }

    m_NumCircleSegments.Update( m_Vehicle->GetCfdGridDensityPtr()->m_NCircSeg.GetID() );
    m_GrowthRatio.Update( m_Vehicle->GetCfdGridDensityPtr()->m_GrowRatio.GetID() );
    m_Rig3dGrowthLimit.Update( m_Vehicle->GetCfdGridDensityPtr()->m_RigorLimit.GetID() );

    m_RelCurveTolSlider.Update( m_Vehicle->GetCfdSettingsPtr()->m_RelCurveTol.GetID() );

    //===== Geometry Control =====//
    m_IntersectSubsurfaces.Update( m_Vehicle->GetCfdSettingsPtr()->m_IntersectSubSurfs.GetID() );

    m_ToCubicToggle.Update( m_Vehicle->GetCfdSettingsPtr()->m_DemoteSurfsCubicFlag.GetID() );
    m_ToCubicTolSlider.Update( m_Vehicle->GetCfdSettingsPtr()->m_CubicSurfTolerance.GetID() );

    if ( m_Vehicle->GetCfdSettingsPtr()->m_DemoteSurfsCubicFlag.Get() )
    {
        m_ToCubicTolSlider.Activate();
    }
    else
    {
        m_ToCubicTolSlider.Deactivate();
    }

    m_ConvertToQuadsToggle.Update( m_Vehicle->GetCfdSettingsPtr()->m_ConvertToQuadsFlag.GetID() );
}

void CfdMeshScreen::UpdateDisplayTab()
{
    //===== Display Tab Toggle Update =====//
    m_ShowSourcesAndWakePreview.Update( m_Vehicle->GetCfdSettingsPtr()->m_DrawSourceWakeFlag.GetID() );
    m_ShowFarFieldPreview.Update( m_Vehicle->GetCfdSettingsPtr()->m_DrawFarPreFlag.GetID() );
    m_ShowMesh.Update( m_Vehicle->GetCfdSettingsPtr()->m_DrawMeshFlag.GetID() );
    m_ShowWake.Update( m_Vehicle->GetCfdSettingsPtr()->m_DrawWakeFlag.GetID() );
    m_ShowSymmetryPlane.Update( m_Vehicle->GetCfdSettingsPtr()->m_DrawSymmFlag.GetID() );
    m_ShowFarField.Update( m_Vehicle->GetCfdSettingsPtr()->m_DrawFarFlag.GetID() );
    m_ShowBadEdgesAndTriangles.Update( m_Vehicle->GetCfdSettingsPtr()->m_DrawBadFlag.GetID() );
    m_ColorFaces.Update( m_Vehicle->GetCfdSettingsPtr()->m_ColorFacesFlag.GetID() );
    m_ColorByToggleGroup.Update( m_Vehicle->GetCfdSettingsPtr()->m_ColorTagReason.GetID() );

    m_ColorByToggleGroup.Deactivate();
    if ( m_Vehicle->GetCfdSettingsPtr()->m_ColorFacesFlag() )
    {
        m_ColorByToggleGroup.Activate();
    }

    m_DrawIsect.Update( m_Vehicle->GetCfdSettingsPtr()->m_DrawIsectFlag.GetID() );
    m_DrawBorder.Update( m_Vehicle->GetCfdSettingsPtr()->m_DrawBorderFlag.GetID() );

    m_ShowRaw.Update( m_Vehicle->GetCfdSettingsPtr()->m_DrawRawFlag.GetID() );
    m_ShowBinAdapt.Update( m_Vehicle->GetCfdSettingsPtr()->m_DrawBinAdaptFlag.GetID() );

    m_ShowCurve.Update( m_Vehicle->GetCfdSettingsPtr()->m_DrawCurveFlag.GetID() );
    m_ShowPts.Update( m_Vehicle->GetCfdSettingsPtr()->m_DrawPntsFlag.GetID() );

    if ( CfdMeshMgr.GetCfdSettingsPtr() )
    {
        CfdMeshMgr.UpdateDisplaySettings();
    }
}

void CfdMeshScreen::UpdateOutputTab()
{
    //===== Update File Output Strings =====//
    string stlname = m_Vehicle->GetCfdSettingsPtr()->GetExportFileName( vsp::CFD_STL_FILE_NAME );
    m_StlOutput.Update( StringUtil::truncateFileName( stlname, 40 ).c_str() );
    string polyname = m_Vehicle->GetCfdSettingsPtr()->GetExportFileName( vsp::CFD_POLY_FILE_NAME );
    m_PolyOutput.Update( StringUtil::truncateFileName( polyname, 40 ).c_str() );
    string triname = m_Vehicle->GetCfdSettingsPtr()->GetExportFileName( vsp::CFD_TRI_FILE_NAME );
    m_TriOutput.Update( StringUtil::truncateFileName( triname, 40 ).c_str() );
    string facname = m_Vehicle->GetCfdSettingsPtr()->GetExportFileName( vsp::CFD_FACET_FILE_NAME );
    m_FacOutput.Update( StringUtil::truncateFileName( facname, 40 ).c_str() );
    string objname = m_Vehicle->GetCfdSettingsPtr()->GetExportFileName( vsp::CFD_OBJ_FILE_NAME );
    m_ObjOutput.Update( StringUtil::truncateFileName( objname, 40 ).c_str() );
    string gmshname = m_Vehicle->GetCfdSettingsPtr()->GetExportFileName( vsp::CFD_GMSH_FILE_NAME );
    m_MshOutput.Update( StringUtil::truncateFileName( gmshname, 40 ).c_str() );
    string datname = m_Vehicle->GetCfdSettingsPtr()->GetExportFileName( vsp::CFD_DAT_FILE_NAME );
    m_DatOutput.Update( StringUtil::truncateFileName( datname, 40 ).c_str() );
    string keyname = m_Vehicle->GetCfdSettingsPtr()->GetExportFileName( vsp::CFD_KEY_FILE_NAME );
    m_KeyOutput.Update( StringUtil::truncateFileName( keyname, 40 ).c_str() );
    string tkeyname = m_Vehicle->GetCfdSettingsPtr()->GetExportFileName( vsp::CFD_TKEY_FILE_NAME );
    m_TkeyOutput.Update( StringUtil::truncateFileName( tkeyname, 40).c_str() );
    string vspgeomname = m_Vehicle->GetCfdSettingsPtr()->GetExportFileName( vsp::CFD_VSPGEOM_FILE_NAME );
    m_VspgeomOutput.Update( StringUtil::truncateFileName( vspgeomname, 40 ).c_str() );

    //==== Update File Output Flags ====//
    m_StlFile.Update( m_Vehicle->GetCfdSettingsPtr()->GetExportFileFlag( vsp::CFD_STL_FILE_NAME )->GetID() );
    m_TaggedMultiSolid.Update( m_Vehicle->m_STLMultiSolid.GetID() );
    m_PolyFile.Update( m_Vehicle->GetCfdSettingsPtr()->GetExportFileFlag( vsp::CFD_POLY_FILE_NAME )->GetID() );
    m_TriFile.Update( m_Vehicle->GetCfdSettingsPtr()->GetExportFileFlag( vsp::CFD_TRI_FILE_NAME )->GetID() );
    m_FacFile.Update( m_Vehicle->GetCfdSettingsPtr()->GetExportFileFlag( vsp::CFD_FACET_FILE_NAME )->GetID() );
    m_ObjFile.Update( m_Vehicle->GetCfdSettingsPtr()->GetExportFileFlag( vsp::CFD_OBJ_FILE_NAME )->GetID() );
    m_MshFile.Update( m_Vehicle->GetCfdSettingsPtr()->GetExportFileFlag( vsp::CFD_GMSH_FILE_NAME )->GetID() );
    m_DatFile.Update( m_Vehicle->GetCfdSettingsPtr()->GetExportFileFlag( vsp::CFD_DAT_FILE_NAME )->GetID() );
    m_KeyFile.Update( m_Vehicle->GetCfdSettingsPtr()->GetExportFileFlag( vsp::CFD_KEY_FILE_NAME )->GetID() );
    m_TkeyFile.Update( m_Vehicle->GetCfdSettingsPtr()->GetExportFileFlag( vsp::CFD_TKEY_FILE_NAME)->GetID() );
    m_VspgeomFile.Update( m_Vehicle->GetCfdSettingsPtr()->GetExportFileFlag( vsp::CFD_VSPGEOM_FILE_NAME )->GetID() );

}

void CfdMeshScreen::UpdateSourcesTab( BaseSource* source )
{
    if ( source )
    {
        m_SourceLen1.Activate();
        m_SourceRad1.Activate();
        m_SourceName.Activate();

        m_SourceLen1.Update( source->m_Len.GetID() );
        m_SourceRad1.Update( source->m_Rad.GetID() );

        m_SourceName.Update( source->GetName().c_str() );

        if ( source->GetType() == vsp::POINT_SOURCE )
        {
            m_SourceU1.Activate();
            m_SourceW1.Activate();

            PointSource* ps = ( PointSource* )source;

            m_SourceU1.Update( ps->m_ULoc.GetID() );
            m_SourceW1.Update( ps->m_WLoc.GetID() );

            m_SourceU2.Deactivate();
            m_SourceW2.Deactivate();
            m_SourceLen2.Deactivate();
            m_SourceRad2.Deactivate();
        }
        else if ( source->GetType() == vsp::LINE_SOURCE )
        {
            m_SourceU1.Activate();
            m_SourceW1.Activate();
            m_SourceU2.Activate();
            m_SourceW2.Activate();
            m_SourceLen2.Activate();
            m_SourceRad2.Activate();

            LineSource* ps = ( LineSource* )source;

            m_SourceU1.Update( ps->m_ULoc1.GetID() );
            m_SourceW1.Update( ps->m_WLoc1.GetID() );

            m_SourceU2.Update( ps->m_ULoc2.GetID() );
            m_SourceW2.Update( ps->m_WLoc2.GetID() );

            m_SourceLen2.Update( ps->m_Len2.GetID() );
            m_SourceRad2.Update( ps->m_Rad2.GetID() );
        }
        else if ( source->GetType() == vsp::BOX_SOURCE )
        {
            m_SourceU1.Activate();
            m_SourceW1.Activate();
            m_SourceU2.Activate();
            m_SourceW2.Activate();

            BoxSource* ps = ( BoxSource* )source;

            m_SourceU1.Update( ps->m_ULoc1.GetID() );
            m_SourceW1.Update( ps->m_WLoc1.GetID() );

            m_SourceU2.Update( ps->m_ULoc2.GetID() );
            m_SourceW2.Update( ps->m_WLoc2.GetID() );

            m_SourceLen2.Deactivate();
            m_SourceRad2.Deactivate();
        }
        else if ( source->GetType() == vsp::ULINE_SOURCE )
        {
            m_SourceU1.Activate();

            ULineSource* ps = ( ULineSource* )source;

            m_SourceU1.Update( ps->m_Val.GetID() );
            m_SourceW1.Deactivate();

            m_SourceU2.Deactivate();
            m_SourceW2.Deactivate();

            m_SourceLen2.Deactivate();
            m_SourceRad2.Deactivate();
        }
        else if ( source->GetType() == vsp::WLINE_SOURCE )
        {
            m_SourceW1.Activate();

            WLineSource* ps = ( WLineSource* )source;

            m_SourceU1.Deactivate();
            m_SourceW1.Update( ps->m_Val.GetID() );

            m_SourceU2.Deactivate();
            m_SourceW2.Deactivate();

            m_SourceLen2.Deactivate();
            m_SourceRad2.Deactivate();
        }
    }
    else
    {
        m_SourceLen1.Deactivate();
        m_SourceRad1.Deactivate();
        m_SourceLen2.Deactivate();
        m_SourceRad2.Deactivate();
        m_SourceU1.Deactivate();
        m_SourceW1.Deactivate();
        m_SourceU2.Deactivate();
        m_SourceW2.Deactivate();
        m_SourceName.Deactivate();
    }
}

void CfdMeshScreen::UpdateDomainTab()
{
    //===== Update Generate Buttons =====//
    m_GenerateHalfMesh.Update( m_Vehicle->GetCfdSettingsPtr()->m_HalfMeshFlag.GetID() );
    m_GenerateFarFieldMesh.Update( m_Vehicle->GetCfdSettingsPtr()->m_FarMeshFlag.GetID() );

    //===== Update Domain Sliders =====//
    m_DomainMaxEdgeLenToggleGroup.Update( m_Vehicle->GetCfdGridDensityPtr()->m_FarAbsRel.GetID() );
    if ( m_Vehicle->GetCfdGridDensityPtr()->m_FarAbsRel() == vsp::ABS )
    {
        m_DomainMaxEdgeLen.Update( 1, m_Vehicle->GetCfdGridDensityPtr()->m_FarMaxLen.GetID(), m_Vehicle->GetCfdGridDensityPtr()->m_FarFrac.GetID() );
    }
    else
    {
        m_DomainMaxEdgeLen.Update( 2, m_Vehicle->GetCfdGridDensityPtr()->m_FarMaxLen.GetID(), m_Vehicle->GetCfdGridDensityPtr()->m_FarFrac.GetID() );
    }

    m_DomainMaxGapToggleGroup.Update( m_Vehicle->GetCfdGridDensityPtr()->m_FarMaxGapAbsRel.GetID() );
    if ( m_Vehicle->GetCfdGridDensityPtr()->m_FarMaxGapAbsRel() == vsp::ABS )
    {
        m_DomainMaxGap.Update( 1, m_Vehicle->GetCfdGridDensityPtr()->m_FarMaxGap.GetID(), m_Vehicle->GetCfdGridDensityPtr()->m_FarMaxGapFrac.GetID() );
    }
    else
    {
        m_DomainMaxGap.Update( 2, m_Vehicle->GetCfdGridDensityPtr()->m_FarMaxGap.GetID(), m_Vehicle->GetCfdGridDensityPtr()->m_FarMaxGapFrac.GetID() );
    }

    m_DomainNumCircleSegments.Update( m_Vehicle->GetCfdGridDensityPtr()->m_FarNCircSeg.GetID() );

    //===== Symmetry Plane Toggle Update =====//
    m_SymPlaneSplit.Update( m_Vehicle->GetCfdSettingsPtr()->m_SymSplittingOnFlag.GetID() );

    //Last parameter set to null because Abs functionality currently not available
    m_DomainLength.Update( 1, m_Vehicle->GetCfdSettingsPtr()->m_FarXScale.GetID(), "" );
    m_DomainWidth.Update( 1, m_Vehicle->GetCfdSettingsPtr()->m_FarYScale.GetID(), "" );
    m_DomainHeight.Update( 1, m_Vehicle->GetCfdSettingsPtr()->m_FarZScale.GetID(), "" );

    m_DomainXLoc.Update( m_Vehicle->GetCfdSettingsPtr()->m_FarXLocation.GetID() );
    m_DomainYLoc.Update( m_Vehicle->GetCfdSettingsPtr()->m_FarYLocation.GetID() );
    m_DomainZLoc.Update( m_Vehicle->GetCfdSettingsPtr()->m_FarZLocation.GetID() );

    //=== Domain tab GUI active areas ===//
    if ( m_Vehicle->GetCfdSettingsPtr()->m_FarMeshFlag() )
    {
        m_FarParametersLayout.GetGroup()->activate();

        if( m_Vehicle->GetCfdSettingsPtr()->m_FarCompFlag() )
        {
            m_FarBoxLayout.GetGroup()->deactivate();
            m_ComponentChoice.Activate();
        }
        else
        {
            m_FarBoxLayout.GetGroup()->activate();
            m_ComponentChoice.Deactivate();

            if( m_Vehicle->GetCfdSettingsPtr()->m_FarManLocFlag() )
            {
                m_FarXYZLocationLayout.GetGroup()->activate();
            }
            else
            {
                m_FarXYZLocationLayout.GetGroup()->deactivate();
            }
        }
    }
    else
    {
        m_FarParametersLayout.GetGroup()->deactivate();
    }
}

void CfdMeshScreen::UpdateWakesTab()
{
    //===== Update Wake Parms =====//
    string wakeGeomID = CfdMeshMgr.GetWakeGeomID();
    Geom* g = m_Vehicle->FindGeom( wakeGeomID );
    if ( g )
    {
        m_AddWake.Update( g->m_WakeActiveFlag.GetID() );
        m_ScaleWake.Update( g->m_WakeScale.GetID() );
        m_WakeAngle.Update( g->m_WakeAngle.GetID() );

        if ( g->m_WakeActiveFlag() )
        {
            m_ScaleWake.Activate();
            m_WakeAngle.Activate();
        }
        else
        {
            m_ScaleWake.Deactivate();
            m_WakeAngle.Deactivate();
        }
    }
}

void CfdMeshScreen::AddOutputText( const string &text )
{
    Fl::lock();
    if ( text == "CLEAR_TERMINAL" )
    {
        m_ConsoleDisplay->clear();
        m_ConsoleDisplay->clear_history();
    }
    else
    {
        m_ConsoleDisplay->append( text.c_str() );
    }
    Fl::unlock();
}

void CfdMeshScreen::LoadDrawObjs( vector< DrawObj* > &draw_obj_vec )
{
    if ( IsShown() )
    {
        CfdMeshMgr.LoadDrawObjs( draw_obj_vec );
    }
}

bool CfdMeshScreen::GetVisBndBox( BndBox &bbox )
{
    if ( IsShown() )
    {
        return CfdMeshMgr.GetVisBndBox( bbox );
    }
    return false;
}

void CfdMeshScreen::Show()
{
    if ( m_Vehicle->GetCfdSettingsPtr()->m_UseMode() )
    {
        Mode *m = ModeMgr.GetMode( m_Vehicle->GetCfdSettingsPtr()->m_ModeID );
        if ( m )
        {
            m->ApplySettings();
        }
    }

    m_ScreenMgr->SetUpdateFlag( true );
    TabScreen::Show();
}

void CfdMeshScreen::Hide()
{
    m_FLTK_Window->hide();
    m_ScreenMgr->SetUpdateFlag( true );
}

void CfdMeshScreen::CallBack( Fl_Widget* w )
{
    assert( m_ScreenMgr );

    if ( w == m_SourceBrowser )
    {
        CfdMeshMgr.GUI_Val( "SourceID", m_SourceBrowser->value() - 1 );
    }

    m_ScreenMgr->SetUpdateFlag( true );
}

void CfdMeshScreen::CloseCallBack( Fl_Widget *w )
{
    Hide();
}

#ifdef WIN32
DWORD WINAPI cfdmesh_thread_fun( LPVOID data )
#else
void * cfdmesh_thread_fun( void *data )
#endif
{
    CfdMeshMgr.GenerateMesh();
    return 0;
}

void CfdMeshScreen::GuiDeviceCallBack( GuiDevice* device )
{
    assert( m_ScreenMgr );

    GuiDeviceGlobalTabCallback( device );
    GuiDeviceOutputTabCallback( device );
    GuiDeviceSourcesTabCallback( device );
    GuiDeviceDomainTabCallback( device );
    GuiDeviceWakesTabCallback( device );

    if ( device == &m_MeshAndExport )
    {
        CfdMeshMgr.SetMeshInProgress( true );
        m_CFDMeshProcess.StartThread( cfdmesh_thread_fun, nullptr );
    }

    m_ScreenMgr->SetUpdateFlag( true );
}

void CfdMeshScreen::GuiDeviceGlobalTabCallback( GuiDevice* device )
{
    //===== Global Source Control =====//
    if ( device == &m_GlobSrcAdjustLenLftLft )
    {
        CfdMeshMgr.AdjustAllSourceLen( 1.0 / 1.5 );
    }
    else if ( device == &m_GlobSrcAdjustLenLft )
    {
        CfdMeshMgr.AdjustAllSourceLen( 1.0 / 1.1 );
    }
    else if ( device == &m_GlobSrcAdjustLenRht )
    {
        CfdMeshMgr.AdjustAllSourceLen( 1.1 );
    }
    else if ( device == &m_GlobSrcAdjustLenRhtRht )
    {
        CfdMeshMgr.AdjustAllSourceLen( 1.5 );
    }
    else if ( device == &m_GlobSrcAdjustRadLftLft )
    {
        CfdMeshMgr.AdjustAllSourceRad( 1.0 / 1.5 );
    }
    else if ( device == &m_GlobSrcAdjustRadLft )
    {
        CfdMeshMgr.AdjustAllSourceRad( 1.0 / 1.1 );
    }
    else if ( device == &m_GlobSrcAdjustRadRht )
    {
        CfdMeshMgr.AdjustAllSourceRad( 1.1 );
    }
    else if ( device == &m_GlobSrcAdjustRadRhtRht )
    {
        CfdMeshMgr.AdjustAllSourceRad( 1.5 );
    }
    else if ( device == &m_ModeChoice || device == &m_ModeSetToggleGroup )
    {
        if ( m_Vehicle->GetCfdSettingsPtr()->m_UseMode() )
        {
            int indx = m_ModeChoice.GetVal();
            if ( indx >= 0  && indx < m_ModeIDs.size() )
            {
                m_Vehicle->GetCfdSettingsPtr()->m_ModeID = m_ModeIDs[ indx ];
            }
            else
            {
                m_Vehicle->GetCfdSettingsPtr()->m_ModeID = "";
            }

            Mode *m = ModeMgr.GetMode( m_Vehicle->GetCfdSettingsPtr()->m_ModeID );
            if ( m )
            {
                m->ApplySettings();
            }
        }
    }
}

void CfdMeshScreen::GuiDeviceOutputTabCallback( GuiDevice* device )
{
    if ( device == &m_SelectStlFile )
    {
        string newfile = m_ScreenMgr->FileChooser( "Select .stl file.", "*.stl", vsp::SAVE );
        if ( newfile.compare( "" ) != 0 )
        {
            m_Vehicle->GetCfdSettingsPtr()->SetExportFileName( newfile, vsp::CFD_STL_FILE_NAME );
        }
    }
    else if ( device == &m_SelectPolyFile )
    {
        string newfile = m_ScreenMgr->FileChooser( "Select .poly file.", "*.poly", vsp::SAVE );
        if ( newfile.compare( "" ) != 0 )
        {
            m_Vehicle->GetCfdSettingsPtr()->SetExportFileName( newfile, vsp::CFD_POLY_FILE_NAME );
        }
    }
    else if ( device == &m_SelectTriFile )
    {
        string newfile = m_ScreenMgr->FileChooser( "Select .tri file.", "*.tri", vsp::SAVE );
        if ( newfile.compare( "" ) != 0 )
        {
            m_Vehicle->GetCfdSettingsPtr()->SetExportFileName( newfile, vsp::CFD_TRI_FILE_NAME );
        }
    }
    else if ( device == &m_SelectFacFile )
    {
        string newfile = m_ScreenMgr->FileChooser( "Select .facet file.", "*.facet", vsp::SAVE );
        if ( newfile.compare( "" ) != 0 )
        {
            m_Vehicle->GetCfdSettingsPtr()->SetExportFileName( newfile, vsp::CFD_FACET_FILE_NAME );
        }
    }
    else if ( device == &m_SelectObjFile  )
    {
        string newfile = m_ScreenMgr->FileChooser( "Select .obj file.", "*.obj", vsp::SAVE );
        if ( newfile.compare( "" ) != 0 )
        {
            m_Vehicle->GetCfdSettingsPtr()->SetExportFileName( newfile, vsp::CFD_OBJ_FILE_NAME );
        }
    }
    else if ( device == &m_SelectMshFile )
    {
        string newfile = m_ScreenMgr->FileChooser( "Select .msh file.", "*.msh", vsp::SAVE );
        if ( newfile.compare( "" ) != 0 )
        {
            m_Vehicle->GetCfdSettingsPtr()->SetExportFileName( newfile, vsp::CFD_GMSH_FILE_NAME );
        }
    }
    else if ( device == &m_SelectDatFile )
    {
        string newfile = m_ScreenMgr->FileChooser( "Select NASCART .dat file.", "*.dat", vsp::SAVE );
        if ( newfile.compare( "" ) != 0 )
        {
            m_Vehicle->GetCfdSettingsPtr()->SetExportFileName( newfile, vsp::CFD_DAT_FILE_NAME );
        }
    }
    else if ( device == &m_SelectKeyFile )
    {
        string newfile = m_ScreenMgr->FileChooser( "Select NASCART .key file.", "*.key", vsp::SAVE );
        if ( newfile.compare( "" ) != 0 )
        {
            m_Vehicle->GetCfdSettingsPtr()->SetExportFileName( newfile, vsp::CFD_KEY_FILE_NAME );
        }
    }
    else if ( device == &m_SelectTkeyFile )
    {
        string newfile = m_ScreenMgr->FileChooser( "Select .tkey file.", "*.tkey", vsp::SAVE );
        if ( newfile.compare( "" ) != 0 )
        {
            m_Vehicle->GetCfdSettingsPtr()->SetExportFileName( newfile, vsp::CFD_TKEY_FILE_NAME );
        }
    }
    else if ( device == &m_SelectVspgeomFile )
    {
        string newfile = m_ScreenMgr->FileChooser( "Select .vspgeom file.", "*.vspgeom", vsp::SAVE );
        if ( newfile.compare( "" ) != 0 )
        {
            m_Vehicle->GetCfdSettingsPtr()->SetExportFileName( newfile, vsp::CFD_VSPGEOM_FILE_NAME );
        }
    }

}

void CfdMeshScreen::GuiDeviceSourcesTabCallback( GuiDevice* device )
{
    if ( device == &m_AddDefaultSources )
    {
        CfdMeshMgr.AddDefaultSourcesCurrGeom();
    }
    else if ( device == &m_AddSource )
    {
        int type = m_SourcesType.GetVal();
        if ( type >= 0 && type < vsp::NUM_SOURCE_TYPES )
        {
            CfdMeshMgr.AddSource( type );
        }
    }
    else if ( device == &m_DeleteSource )
    {
        CfdMeshMgr.DeleteCurrSource();
    }
    else if ( device == &m_SourcesSelectComp )
    {
        //==== Load List of Parts for Comp ====//
        int id = m_SourcesSelectComp.GetVal();
        CfdMeshMgr.SetCurrSourceGeomID( m_GeomVec[ id ] );
        CfdMeshMgr.SetCurrMainSurfIndx( 0 );
    }
    else if ( device == &m_SourcesSelectSurface )
    {
        int id = m_SourcesSelectSurface.GetVal();
        CfdMeshMgr.SetCurrMainSurfIndx( id );
    }
    else if ( device == &m_SourceName )
    {
        //===== Update Sources =====//
        BaseSource* source = CfdMeshMgr.GetCurrSource();
        source->SetName( m_SourceName.GetString() );
    }
}

void CfdMeshScreen::GuiDeviceDomainTabCallback( GuiDevice* device )
{
    if ( device == &m_FarFieldTypeComponent )
    {
        m_Vehicle->GetCfdSettingsPtr()->m_FarCompFlag = true;

        if ( m_FarFieldTypeBox.GetValue() )
        {
            m_FarFieldTypeComponent.GetFlButton()->value( -1 );
            m_FarFieldTypeBox.GetFlButton()->value( 0 );
        }
        else
        {
            m_FarFieldTypeComponent.GetFlButton()->value( -1 );
        }
    }
    else if ( device == &m_FarFieldTypeBox )
    {
        m_Vehicle->GetCfdSettingsPtr()->m_FarCompFlag = false;

        if ( m_FarFieldTypeComponent.GetValue() )
        {
            m_FarFieldTypeBox.GetFlButton()->value( -1 );
            m_FarFieldTypeComponent.GetFlButton()->value( 0 );
        }
        else
        {
            m_FarFieldTypeBox.GetFlButton()->value( -1 );
        }
    }
    else if ( device == &m_DomainRel )
    {
        m_Vehicle->GetCfdSettingsPtr()->m_FarAbsSizeFlag = false;

        if ( (int)m_DomainAbs.GetFlButton()->value() )
        {
            m_DomainRel.GetFlButton()->value( 1 );
            m_DomainAbs.GetFlButton()->value( 0 );
        }
        else
        {
            m_DomainRel.GetFlButton()->value( 1 );
        }
    }
    else if ( device == &m_DomainAbs )
    {
        m_Vehicle->GetCfdSettingsPtr()->m_FarAbsSizeFlag = true;

        if ( (int)m_DomainRel.GetFlButton()->value() )
        {
            m_DomainAbs.GetFlButton()->value( 1 );
            m_DomainRel.GetFlButton()->value( 0 );
        }
        else
        {
            m_DomainAbs.GetFlButton()->value( 1 );
        }
    }
    else if ( device == &m_DomainCen )
    {
        m_Vehicle->GetCfdSettingsPtr()->m_FarManLocFlag = false;

        if ( (int)m_DomainMan.GetFlButton()->value() )
        {
            m_DomainCen.GetFlButton()->value( 1 );
            m_DomainMan.GetFlButton()->value( 0 );
        }
        else
        {
            m_DomainCen.GetFlButton()->value( 1 );
        }
    }
    else if ( device == &m_DomainMan )
    {
        m_Vehicle->GetCfdSettingsPtr()->m_FarManLocFlag = true;

        if ( (int)m_DomainCen.GetFlButton()->value() )
        {
            m_DomainMan.GetFlButton()->value( 1 );
            m_DomainCen.GetFlButton()->value( 0 );
        }
        else
        {
            m_DomainMan.GetFlButton()->value( 1 );
        }
    }
    else if ( device == &m_ComponentChoice )
    {
        int id = m_ComponentChoice.GetVal();
        m_Vehicle->GetCfdSettingsPtr()->SetFarGeomID( m_GeomVec[ id ] );
    }
}

void CfdMeshScreen::GuiDeviceWakesTabCallback( GuiDevice* device )
{
    if ( device == &m_Comp )
    {
        CfdMeshMgr.SetWakeGeomID( m_WingGeomVec[ m_Comp.GetVal() ] );
    }
}
