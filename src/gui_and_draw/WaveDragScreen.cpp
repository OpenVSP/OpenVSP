//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//
// WaveDragScreen.cpp
//
// Michael Waddington
//////////////////////////////////////////////////////////////////////

#include <cmath>

#include "WaveDragScreen.h"
#include "ResultsViewer.h"
#include "SubSurfaceMgr.h"
#include "StlHelper.h"
#include "VspUtil.h"
#include "ModeMgr.h"

/////////////////// Construction/Destruction /////////////////////////

WaveDragScreen::WaveDragScreen( ScreenMgr* mgr ) : TabScreen( mgr, 310 + 470, 450 + 72, "Wave Drag", "", 80, 470 )
{
    Vehicle *veh = VehicleMgr.GetVehicle();

    m_FLTK_Window->callback( staticCloseCB, this );
    m_FLTK_Window->resizable( 0 );

    if (!veh) return;

    Fl_Group* run_tab = AddTab( "Setup" );
    Fl_Group* flow_tab = AddTab( "Inflow/Outflow" );
    Fl_Group* plot_tab = AddTab( "Plot" );
    Fl_Group* runs_tab = AddTab( "Runs" );

    run_tab->show();

    //==== Run Tab ====//
    Fl_Group* run_group = AddSubGroup( run_tab, 5 );
    m_RunLayout.SetGroupAndScreen( run_group, this );

    m_RunLayout.SetButtonWidth( 100 );
    m_RunLayout.SetChoiceButtonWidth( 100 );

    // Slice Number, Rotation, and Set
    m_RunLayout.AddDividerBox( "Case Setup" );

    m_RunLayout.SetFitWidthFlag( false );
    m_RunLayout.SetSameLineFlag( true );

    int bw = 100;
    m_RunLayout.SetButtonWidth( bw );

    m_RunLayout.SetSameLineFlag( true );
    m_RunLayout.SetChoiceButtonWidth( 0 );
    m_RunLayout.SetFitWidthFlag( false );
    m_RunLayout.AddButton( m_SetToggle, "Geometry Set:" );
    m_RunLayout.SetFitWidthFlag( true );
    m_RunLayout.AddChoice(m_SelectedSetChoice, "", bw);
    m_RunLayout.ForceNewLine();

    m_RunLayout.SetSameLineFlag( true );
    m_RunLayout.SetChoiceButtonWidth( 0 );
    m_RunLayout.SetFitWidthFlag( false );
    m_RunLayout.AddButton( m_ModeToggle, "Mode:" );
    m_RunLayout.SetFitWidthFlag( true );
    m_RunLayout.AddChoice(m_ModeChoice, "", bw );
    m_RunLayout.ForceNewLine();

    m_ModeSetToggleGroup.Init( this );
    m_ModeSetToggleGroup.AddButton( m_SetToggle.GetFlButton() );
    m_ModeSetToggleGroup.AddButton( m_ModeToggle.GetFlButton() );

    m_RunLayout.SetSameLineFlag( false );

    m_RunLayout.AddSlider( m_NumSlicesSlider, "Num Slices", 80, "%7.0f" );
    m_RunLayout.AddSlider( m_NumRotSectsSlider, "Num Rotations", 20, "%7.0f" );
    m_RunLayout.AddButton( m_SymmToggle, "Run with X-Z Symmetry" );

    // Mach
    m_RunLayout.AddYGap();
    m_RunLayout.AddDividerBox( "Flow Condition" );

    m_RunLayout.AddSlider( m_MachNumberSlider, "Mach", 5.0, "%7.3f" );

    // Reference Area
    m_RunLayout.AddYGap();
    m_RunLayout.AddDividerBox( "Reference Area" );

    m_RunLayout.SetSameLineFlag( true );
    m_RunLayout.SetFitWidthFlag( false );

    m_RunLayout.SetButtonWidth( m_RunLayout.GetW()/2 );

    m_RunLayout.AddButton( m_RefManualToggle, "Manual" );
    m_RunLayout.AddButton( m_RefChoiceToggle, "From Model" );
    m_RunLayout.ForceNewLine();

    m_RunLayout.InitWidthHeightVals();

    m_RunLayout.SetSameLineFlag( false );
    m_RunLayout.SetFitWidthFlag( true );

    m_RunLayout.AddChoice( m_RefWingChoice, "Ref. Wing" );

    m_RunLayout.AddSlider( m_SrefSlider, "Sref", 1000.0, "%7.3f" );

    m_RefToggle.Init( this );
    m_RefToggle.AddButton( m_RefManualToggle.GetFlButton() );
    m_RefToggle.AddButton( m_RefChoiceToggle.GetFlButton() );

    // Output
    m_RunLayout.AddYGap();
    m_RunLayout.AddDividerBox( "Output File" );

    m_RunLayout.SetSameLineFlag( true );
    m_RunLayout.SetFitWidthFlag( false );

    m_RunLayout.SetButtonWidth( 50 );
    m_RunLayout.SetInputWidth( 300 - 50 - 25 );
    m_RunLayout.AddOutput( m_FileSelect, "File" );

    m_RunLayout.SetButtonWidth( 25 );
    m_RunLayout.AddButton( m_FileTrigger, "..." );

    //==== Plot Tab ====//
    Fl_Group* plot_group = AddSubGroup( plot_tab, 5 );
    m_PlotLayout.SetGroupAndScreen( plot_group, this );

    m_PlotLayout.SetDividerHeight( m_PlotLayout.GetStdHeight() );

    m_PlotLayout.SetSameLineFlag( true );
    m_PlotLayout.SetFitWidthFlag( true );
    // Rotation and theta
    m_PlotLayout.AddDividerBox( "Displayed Rotation", m_PlotLayout.GetInputWidth() );

    m_PlotLayout.SetFitWidthFlag( false );

    m_PlotLayout.AddOutput( m_ThetaDegValResult );
    m_PlotLayout.ForceNewLine();

    m_PlotLayout.SetSameLineFlag( false );
    m_PlotLayout.SetFitWidthFlag( true );

    m_PlotLayout.AddIndexSelector( m_ThetaIndexSelector );

    // Reference location
    m_PlotLayout.AddYGap();

    m_PlotLayout.SetButtonWidth( 60 );

    m_PlotLayout.SetSameLineFlag( true );
    m_PlotLayout.SetFitWidthFlag( true );
    m_PlotLayout.AddDividerBox( "Slice Reference", m_PlotLayout.GetButtonWidth() );

    m_PlotLayout.SetFitWidthFlag( false );
    m_PlotLayout.AddButton( m_PlaneToggle, "Plane" );

    m_PlotLayout.ForceNewLine();

    m_PlotLayout.SetSameLineFlag( true );
    m_PlotLayout.SetFitWidthFlag( true );
    m_PlotLayout.AddSlider( m_SlicingLocSlider, "X Loc", 10, "%5.3f", 100 );

    m_PlotLayout.SetFitWidthFlag( false );

    m_PlotLayout.SetButtonWidth( m_PlotLayout.GetRemainX()/2 );
    m_PlotLayout.AddButton( m_LocDragTrigger, "X" );
    m_PlotLayout.AddButton( m_GlobalDragTrigger, "X, Rot" );
    m_PlotLayout.ForceNewLine();

    m_PlotLayout.SetSameLineFlag( false );
    m_PlotLayout.SetFitWidthFlag( true );

    // Bodies of revolution
    m_PlotLayout.AddYGap();
    m_PlotLayout.AddDividerBox( "Optimal Distribution" );
    m_PlotLayout.SetChoiceButtonWidth( 60 );
    m_SelectedBodyRev.AddItem( "None" );
    m_SelectedBodyRev.AddItem( "Sears-Haack Body (Len-Vol)" );
    m_SelectedBodyRev.AddItem( "Von Karman Ogive (Len-Diam)" );
    m_SelectedBodyRev.AddItem( "Lighthill Body (Len-Diam)" );
    m_PlotLayout.AddChoice( m_SelectedBodyRev, "Select:" );

    // Visualizer
    m_PlotLayout.AddYGap();


    m_PlotLayout.AddDividerBox( "Plot Style" );

    m_PlotLayout.SetFitWidthFlag( false );
    m_PlotLayout.SetSameLineFlag( true );

    m_PlotLayout.AddLabel( "Plot:", 60 );
    m_PlotLayout.SetButtonWidth( m_PlotLayout.GetRemainX() / 4 );

    m_PlotLayout.AddButton( m_AreaTotalButton, "Total" );
    m_PlotLayout.AddButton( m_AreaComponentsButton, "Parts" );
    m_PlotLayout.AddButton( m_AreaBuildupButton, "Buildup" );
    m_PlotLayout.AddButton( m_PointToggle, "Points" );

    m_PlotLayout.ForceNewLine();

    m_AreaGroup.Init( this );
    m_AreaGroup.AddButton( m_AreaTotalButton.GetFlButton() );
    m_AreaGroup.AddButton( m_AreaComponentsButton.GetFlButton() );
    m_AreaGroup.AddButton( m_AreaBuildupButton.GetFlButton() );

    vector< int > area_plot_val_map;
    area_plot_val_map.push_back( WaveDragSingleton::AREA_TOTAL );
    area_plot_val_map.push_back( WaveDragSingleton::AREA_COMPONENTS );
    area_plot_val_map.push_back( WaveDragSingleton::AREA_BUILDUP );
    m_AreaGroup.SetValMapVec( area_plot_val_map );

    m_PlotLayout.SetSameLineFlag( false );
    m_PlotLayout.SetFitWidthFlag( true );

    m_PlotLayout.AddYGap();
    m_PlotLayout.AddDividerBox( "Legend" );

    m_LegendGroup = m_PlotLayout.AddFlScroll( m_PlotLayout.GetRemainY() );
    m_LegendGroup->type( Fl_Scroll::VERTICAL_ALWAYS );
    m_LegendLayout.SetGroupAndScreen( m_LegendGroup, this );

    //==== Plot Area, Title Only ====//
    Fl_Group* plottitle_group = new Fl_Group( 310, 25, 465, 490 );
    // plottitle_group->box( Fl_Boxtype::FL_BORDER_BOX );
    plottitle_group->show();
    m_FLTK_Window->add( plottitle_group );
    m_JustPlotLayout.SetGroupAndScreen( plottitle_group, this );

    m_JustPlotLayout.AddDividerBox( "Cross-Sectional Area Plot" );

    m_JustPlotLayout.AddY( 50 );
    m_JustPlotLayout.AddX( 25 );
    m_canvas = new ResultsPlotCanvas( 0, 0, 10, 10 );
    m_JustPlotLayout.AddCanvas( 400, 400, 0, 1, 0, 1, "", "Axis Location", "Area", m_canvas );
    m_canvas->SetLimitsCallback( staticLimitsCB, this );
    m_canvas->SetPressCallback( staticPressCB, this );

    //==== Flow Tab ====//
    Fl_Group* flow_group = AddSubGroup( flow_tab, 5 );
    m_FlowLayout.SetGroupAndScreen( flow_group, this );

    m_FlowLayout.AddDividerBox( "Select Inflow/Outflow Subsurfaces" );
    m_SubSurfFlowBrowser = m_FlowLayout.AddCheckBrowser( m_FlowLayout.GetRemainY() );
    m_SubSurfFlowBrowser->callback( staticScreenCB, this );

    //==== Runs Tab ====//
    Fl_Group* runs_group = AddSubGroup( runs_tab, 5 );
    m_RunsLayout.SetGroupAndScreen( runs_group, this );

    m_RunsLayout.AddDividerBox( "Runs to Plot" );
    static int runs_col_widths[] = { 24, 50, 100, 45, 60, 0 };
    m_RunsBrowser = new ColCheckBrowser( 0, 0, 10, 10 );
    m_RunsLayout.AddColResizeBrowser( runs_col_widths, 5, m_RunsLayout.GetRemainY(), m_RunsBrowser );
    m_RunsBrowser->Init( this, runs_group );

    //==== Execute Area ====//
    Fl_Group* execute_group = new Fl_Group( 5, 455, 300, 70 );
    execute_group->show();
    m_FLTK_Window->add( execute_group );
    m_ExecuteLayout.SetGroupAndScreen( execute_group, this );

    m_ExecuteLayout.AddDividerBox( "Execute Wave Drag" );
    m_ExecuteLayout.AddButton( m_StartSlicingTrigger, "START" );

    // Drag Results
    m_ExecuteLayout.AddYGap();
    m_ExecuteLayout.SetButtonWidth( 50 );

    m_ExecuteLayout.AddOutput( m_CDwResult, "CD0_w" );

    m_SetMaxDragFlag = false;
    m_Dragging = false;

    m_Zoomed = false;
    m_ZoomXMin = 0;
    m_ZoomXMax = 1;
    m_ZoomYMin = 0;
    m_ZoomYMax = 1;
}

WaveDragScreen::~WaveDragScreen()
{
}

bool WaveDragScreen::Update()
{
    TabScreen::Update();

    UpdateRuns();

    char str[256];
    WaveDragMgr.Update();

    Vehicle *veh = VehicleMgr.GetVehicle();
    if( veh )
    {
        vector <string> geomVec = veh->GetGeomVec();

        m_RefWingChoice.ClearItems();
        m_WingGeomVec.clear();
        unordered_map <string, int> WingCompIDMap;
        int iwing = 0;

        if (!WaveDragMgr.m_RefFlag())
        {
            m_RefWingChoice.Deactivate();
        }
        else
        {
            m_RefWingChoice.Activate();
            for (int i = 0; i < (int)geomVec.size(); i++)
            {
                Geom* g = veh->FindGeom(geomVec[i]);
                if (g)
                {
                    snprintf(str, sizeof( str ), "%d_%s", i, g->GetName().c_str());

                    if (g->GetBehaviorType() == MS_WING_GEOM_TYPE)
                    {
                        m_RefWingChoice.AddItem(str);
                        WingCompIDMap[geomVec[i]] = iwing;
                        m_WingGeomVec.push_back(geomVec[i]);
                        iwing++;
                    }
                }
            }
            m_RefWingChoice.UpdateItems();
        }

        string refGeomID = WaveDragMgr.m_RefGeomID;
        if( refGeomID.length() == 0 && m_WingGeomVec.size() > 0 )
        {
            // Handle case default case.
            refGeomID = m_WingGeomVec[0];
            WaveDragMgr.m_RefGeomID = refGeomID;
            // Re-trigger reference quantity update with default component.
            WaveDragMgr.Update();
        }
        m_RefWingChoice.SetVal( WingCompIDMap[ refGeomID ] );

        m_RefToggle.Update( WaveDragMgr.m_RefFlag.GetID() );
        m_SrefSlider.Update( WaveDragMgr.m_Sref.GetID() );

        m_FileSelect.Update( veh->getExportFileName( vsp::WAVE_DRAG_TXT_TYPE ) );


        m_NumSlicesSlider.Update( WaveDragMgr.m_NumSlices.GetID() );
        m_NumRotSectsSlider.Update( WaveDragMgr.m_NumRotSects.GetID() );
        m_MachNumberSlider.Update( WaveDragMgr.m_MachNumber.GetID() );
        m_SymmToggle.Update( WaveDragMgr.m_SymmFlag.GetID() );
        m_SlicingLocSlider.Update( WaveDragMgr.m_SlicingLoc.GetID() );
        m_PlaneToggle.Update( WaveDragMgr.m_PlaneFlag.GetID() );

        m_PointToggle.Update( WaveDragMgr.m_PointFlag.GetID() );

        m_AreaGroup.Update( WaveDragMgr.m_AreaPlotType.GetID() );

        m_ThetaIndexSelector.SetMinMaxLimits( 1, WaveDragMgr.m_NTheta );

        m_ThetaIndexSelector.Update( WaveDragMgr.m_ThetaIndex.GetID() );

        int itheta = WaveDragMgr.m_ThetaIndex() - 1;
        if ( itheta < 0 )
        {
            itheta = 0;
        }

        double tdval = 0.0;
        if ( itheta < WaveDragMgr.m_ThetaRad.size() )
        {
            tdval = WaveDragMgr.m_ThetaRad[ itheta ] * 180.0 / M_PI;
        }

        snprintf( str, sizeof( str ), "%.1f", tdval );
        m_ThetaDegValResult.Update( str );

        snprintf( str, sizeof( str ), "%7.4f", WaveDragMgr.m_CDWave );
        m_CDwResult.Update( string( str ) );

        m_SelectedBodyRev.Update( WaveDragMgr.m_IdealBodyType.GetID() );


        m_ScreenMgr->LoadSetChoice( {&m_SelectedSetChoice}, {WaveDragMgr.m_SelectedSetIndex.GetID()} );

        m_ScreenMgr->LoadModeChoice( m_ModeChoice, m_ModeIDs, WaveDragMgr.m_ModeID );

        m_ModeSetToggleGroup.Update( WaveDragMgr.m_UseMode.GetID() );

        if ( ModeMgr.GetNumModes() == 0 )
        {
            if ( WaveDragMgr.m_UseMode() )
            {
                WaveDragMgr.m_UseMode.Set( false );
                m_ScreenMgr->SetUpdateFlag( true );
            }
            m_ModeToggle.Deactivate();
        }
        else
        {
            m_ModeToggle.Activate();
        }

        if ( WaveDragMgr.m_UseMode() )
        {
            m_ModeChoice.Activate();
            m_SelectedSetChoice.Deactivate();

            // An unset or deleted mode falls back to the first, as the choice shows
            if ( !ModeMgr.GetMode( WaveDragMgr.m_ModeID ) && !m_ModeIDs.empty() )
            {
                WaveDragMgr.m_ModeID = m_ModeIDs[0];
            }

            Mode *m = ModeMgr.GetMode( WaveDragMgr.m_ModeID );
            if ( m )
            {
                if ( WaveDragMgr.m_SelectedSetIndex() != m->m_NormalSet() )
                {
                    WaveDragMgr.m_SelectedSetIndex = m->m_NormalSet();
                    m_ScreenMgr->SetUpdateFlag( true );
                }
            }
        }
        else
        {
            m_ModeChoice.Deactivate();
            m_SelectedSetChoice.Activate();
        }

        // Removes any subsurf IDs from m_SSFlow_vec that don't exist in the model
        for ( int i = (int)WaveDragMgr.m_SSFlow_vec.size()-1; i >= 0 ; i--)
        {
            SubSurface* subsurf = SubSurfaceMgr.GetSubSurf( WaveDragMgr.m_SSFlow_vec[i] );
            if ( subsurf == nullptr )
            {
                vector_remove_val( WaveDragMgr.m_SSFlow_vec, WaveDragMgr.m_SSFlow_vec[i] );
            }
        }

        //==== Flow Tab Update ====//
        vector< SubSurface* > subsurf_vec = SubSurfaceMgr.GetSubSurfs();

        int scroll_pos = m_SubSurfFlowBrowser->vposition();

        m_SubSurfFlowBrowser->clear();
        // Load Subsurface Names
        for ( int i = 0; i < ( int )subsurf_vec.size() ; i++ )
        {
            string geomid = subsurf_vec[i]->GetCompID();
            Geom* gptr = veh->FindGeom( geomid );
            if ( gptr )
            {
                snprintf( str, sizeof( str ), "%s:  %s", gptr->GetName().c_str(), subsurf_vec[i]->GetName().c_str() );

                if ( vector_contains_val( WaveDragMgr.m_SSFlow_vec, subsurf_vec[i]->GetID() ) )
                {
                    m_SubSurfFlowBrowser->add( str, 1 );
                }
                else
                {
                    m_SubSurfFlowBrowser->add( str, 0  );
                }
            }
        }
        m_SubSurfFlowBrowser->vposition( scroll_pos );
    }

    RedrawPlot();

    m_FLTK_Window->redraw();

    return false;
}

void WaveDragScreen::Show()
{
    m_ScreenMgr->SetUpdateFlag( true );
    TabScreen::Show();
}

void WaveDragScreen::Hide()
{
    VspScreen::Hide();
    m_ScreenMgr->SetUpdateFlag( true );
}

void WaveDragScreen::CallBack( Fl_Widget* w )
{
    assert( m_ScreenMgr );

    if ( w == m_RunsBrowser )
    {
        m_RunsBrowser->HidePopupInput();

        int reason = m_RunsBrowser->GetCBReason();
        if ( reason == BROWSER_CALLBACK_POPUP_ENTER )
        {
            Results* run = ResultsMgr.FindResultsPtr( m_NicknameRunID );
            if ( run )
            {
                run->SetNickname( m_RunsBrowser->GetPopupValue() );
            }
            m_NicknameRunID.clear();
        }
        else if ( reason == BROWSER_CALLBACK_CHECK )
        {
            m_RunIDs.clear();
            for ( int i = 0; i < ( int ) m_ListedRunIDs.size(); i++ )
            {
                if ( m_RunsBrowser->IsChecked( i + 2 ) )
                {
                    m_RunIDs.push_back( m_ListedRunIDs[i] );
                }
            }
        }
        else if ( reason == BROWSER_CALLBACK_POPUP_OPEN )
        {
            int line = m_RunsBrowser->value();
            if ( line >= 2 && line - 2 < ( int ) m_ListedRunIDs.size() )
            {
                m_NicknameRunID = m_ListedRunIDs[ line - 2 ];
                Results* run = ResultsMgr.FindResultsPtr( m_NicknameRunID );
                if ( run )
                {
                    m_RunsBrowser->InsertPopupInput( run->GetNickname(), line, 2 );
                }
            }
        }
    }

    Vehicle *veh = VehicleMgr.GetVehicle();

    if ( w == m_SubSurfFlowBrowser )
    {
        int curr_index = m_SubSurfFlowBrowser->value();
        vector< SubSurface* > subsurf_vec = SubSurfaceMgr.GetSubSurfs();

        if ( (int)subsurf_vec.size() > 0 && curr_index >= 1 && curr_index <= (int)subsurf_vec.size() )
        {

            bool flag = !!m_SubSurfFlowBrowser->checked( curr_index );

            string curr_ID = subsurf_vec[curr_index - 1]->GetID();

            if ( flag && !vector_contains_val( WaveDragMgr.m_SSFlow_vec, curr_ID ) )
            {
                WaveDragMgr.m_SSFlow_vec.push_back( curr_ID );
            }
            else
            {
                vector_remove_val( WaveDragMgr.m_SSFlow_vec, curr_ID );
            }
        }
    }

    m_ScreenMgr->SetUpdateFlag( true );
}

void WaveDragScreen::CloseCallBack( Fl_Widget* w )
{
    Hide();
}

void WaveDragScreen::GuiDeviceCallBack( GuiDevice* gui_device )
{
    assert( m_ScreenMgr );

    Vehicle* veh = m_ScreenMgr->GetVehiclePtr();

    if ( gui_device == &m_FileTrigger  )
    {
        string newfile = m_ScreenMgr->FileChooser( "Choose Wave Drag Output File", "*.txt", vsp::SAVE );
        veh->setExportFileName( vsp::WAVE_DRAG_TXT_TYPE, newfile );
    }
    else if ( gui_device == &m_StartSlicingTrigger )
    {
        WaveDragMgr.SliceAndAnalyze();
    }
    else if ( gui_device == &m_LocDragTrigger )
    {
        m_SetMaxDragFlag = true;
    }
    else if ( gui_device == &m_GlobalDragTrigger )
    {
        WaveDragMgr.m_ThetaIndex = WaveDragMgr.m_iMaxDrag + 1;
        m_SetMaxDragFlag = true;
    }
    else if( gui_device == &m_RefWingChoice )
    {
        int id = m_RefWingChoice.GetVal();
        WaveDragMgr.m_RefGeomID = m_WingGeomVec[id];
    }
    else if ( gui_device == &m_ModeChoice )
    {
        int indx = m_ModeChoice.GetVal();
        if ( indx >= 0 && indx < ( int )m_ModeIDs.size() )
        {
            WaveDragMgr.m_ModeID = m_ModeIDs[ indx ];
        }
        else
        {
            WaveDragMgr.m_ModeID = "";
        }
    }

    m_ScreenMgr->SetUpdateFlag( true );
}

// A run's areas at one theta, read from its WaveDrag Result.
struct WaveDragRunAreas
{
    double m_ThetaDeg;
    vector < double > m_X;          // Slice coordinates
    vector < double > m_XNormFit;
    vector < double > m_XFit;       // Fitted curve coordinates
    vector < double > m_SliceArea;
    vector < double > m_SliceAreaFlow;
    vector < double > m_FitArea;
    vector < double > m_FitAreaFlow;
    vector < string > m_CompIDs;
    vector < string > m_CompNames;  // Unique within the run
    vector < vector < double > > m_CompSliceArea;
    vector < vector < double > > m_CompFitArea;
    vector < vector < double > > m_BuildupArea;
    vector < vector < double > > m_BuildupFitArea;
    double m_StartX;
    double m_EndX;
    double m_Volume;
    double m_Length;
    double m_MaxArea;
    double m_MaxMaxArea;    // Over every theta, as the y axis is scaled
    double m_InletArea;
    double m_ExitArea;
};

static const vector < double > & ResultsDoubles( Results* res, const string & name, int index, bool & ok )
{
    static const vector < double > empty;
    NameValData* nvd = res->FindPtr( name, index );
    if ( !nvd )
    {
        ok = false;
        return empty;
    }
    return nvd->GetDoubleData();
}

// Reads the run's theta nearest theta_deg.
static bool ReadWaveDragRun( Results* res, double theta_deg, WaveDragRunAreas & run )
{
    if ( !res || !res->FindPtr( "Num_Cone_Sections" ) || !res->FindPtr( "Num_Comps" ) || !res->FindPtr( "Comp_Name" ) ||
         !res->FindPtr( "Comp_ID" ) )
    {
        return false;
    }

    bool ok = true;
    const vector < double > & theta = ResultsDoubles( res, "Theta", 0, ok );
    int itheta = -1;
    for ( int i = 0; i < ( int ) theta.size(); i++ )
    {
        if ( itheta < 0 || std::abs( theta[i] - theta_deg ) < std::abs( theta[ itheta ] - theta_deg ) )
        {
            itheta = i;
        }
    }
    if ( itheta < 0 || itheta >= res->FindPtr( "Num_Cone_Sections" )->GetInt( 0 ) )
    {
        return false;
    }
    run.m_ThetaDeg = theta[ itheta ];

    const vector < double > & start = ResultsDoubles( res, "Start_X", 0, ok );
    const vector < double > & end = ResultsDoubles( res, "End_X", 0, ok );
    const vector < double > & xnorm = ResultsDoubles( res, "X_Norm", 0, ok );
    run.m_XNormFit = ResultsDoubles( res, "X_Norm_Fit", 0, ok );
    run.m_SliceArea = ResultsDoubles( res, "Slice_Area", itheta, ok );
    run.m_SliceAreaFlow = ResultsDoubles( res, "Slice_Area_Flow", itheta, ok );
    run.m_FitArea = ResultsDoubles( res, "Fit_Area", itheta, ok );
    run.m_FitAreaFlow = ResultsDoubles( res, "Fit_Area_Flow", itheta, ok );
    const vector < double > & volume = ResultsDoubles( res, "Volume", 0, ok );
    const vector < double > & length = ResultsDoubles( res, "Length", 0, ok );
    const vector < double > & max_area = ResultsDoubles( res, "MaxArea", 0, ok );
    const vector < double > & first_area = ResultsDoubles( res, "Slice_Area", 0, ok );
    const vector < double > & inlet = ResultsDoubles( res, "Inlet_Area", 0, ok );
    const vector < double > & exit = ResultsDoubles( res, "Exit_Area", 0, ok );
    if ( !ok || itheta >= ( int ) start.size() || itheta >= ( int ) end.size() || itheta >= ( int ) volume.size() ||
         itheta >= ( int ) length.size() || itheta >= ( int ) max_area.size() || first_area.empty() || inlet.empty() || exit.empty() )
    {
        return false;
    }

    run.m_StartX = start[ itheta ];
    run.m_EndX = end[ itheta ];
    run.m_Volume = volume[ itheta ];
    run.m_Length = length[ itheta ];
    run.m_MaxArea = max_area[ itheta ];
    run.m_InletArea = inlet[0];
    run.m_ExitArea = exit[0];

    run.m_MaxMaxArea = first_area[0];
    for ( int i = 0; i < ( int ) max_area.size(); i++ )
    {
        run.m_MaxMaxArea = std::max( run.m_MaxMaxArea, max_area[i] );
    }

    run.m_X.clear();
    for ( int i = 0; i < ( int ) xnorm.size(); i++ )
    {
        run.m_X.push_back( run.m_StartX + xnorm[i] * ( run.m_EndX - run.m_StartX ) );
    }
    run.m_XFit.clear();
    for ( int i = 0; i < ( int ) run.m_XNormFit.size(); i++ )
    {
        run.m_XFit.push_back( run.m_StartX + run.m_XNormFit[i] * ( run.m_EndX - run.m_StartX ) );
    }

    run.m_CompIDs = res->FindPtr( "Comp_ID" )->GetStringData();
    run.m_CompNames = res->FindPtr( "Comp_Name" )->GetStringData();
    for ( int icomp = 1; icomp < ( int ) run.m_CompNames.size(); icomp++ )
    {
        int count = 1;
        for ( int j = 0; j < icomp; j++ )
        {
            if ( res->FindPtr( "Comp_Name" )->GetString( j ) == res->FindPtr( "Comp_Name" )->GetString( icomp ) )
            {
                count++;
            }
        }
        if ( count > 1 )
        {
            run.m_CompNames[icomp] += " " + std::to_string( count );
        }
    }
    int ncomp = res->FindPtr( "Num_Comps" )->GetInt( 0 );
    run.m_CompSliceArea.resize( ncomp );
    run.m_CompFitArea.resize( ncomp );
    run.m_BuildupArea.resize( ncomp );
    run.m_BuildupFitArea.resize( ncomp );
    for ( int icomp = 0; icomp < ncomp; icomp++ )
    {
        int index = itheta * ncomp + icomp;
        run.m_CompSliceArea[icomp] = ResultsDoubles( res, "Comp_Slice_Area", index, ok );
        run.m_CompFitArea[icomp] = ResultsDoubles( res, "Comp_Fit_Area", index, ok );
        run.m_BuildupArea[icomp] = ResultsDoubles( res, "Buildup_Area", index, ok );
        run.m_BuildupFitArea[icomp] = ResultsDoubles( res, "Buildup_Fit_Area", index, ok );
    }
    return ok && ( int ) run.m_CompNames.size() == ncomp && ( int ) run.m_CompIDs.size() == ncomp;
}

// A part keeps its place across runs: the same Geom, else the same name.
static int FindPart( const vector < string > & part_ids, const vector < string > & part_names, const string & id, const string & name )
{
    for ( int i = 0; i < ( int ) part_ids.size(); i++ )
    {
        if ( part_ids[i] == id )
        {
            return i;
        }
    }
    for ( int i = 0; i < ( int ) part_names.size(); i++ )
    {
        if ( part_names[i] == name )
        {
            return i;
        }
    }
    return -1;
}

void WaveDragScreen::RedrawPlot()
{
    Vsp_Canvas::current( m_canvas );
    m_canvas->clear();
    m_canvas->ClearSeriesPoints();

    m_LegendGroup->clear();
    m_LegendLayout.SetGroup( m_LegendGroup );
    m_LegendLayout.InitWidthHeightVals();
    m_LegendLayout.SetButtonWidth( m_LegendLayout.GetW() * 0.65 );

    int itheta = WaveDragMgr.m_ThetaIndex() - 1;
    double theta_deg = 0.0;
    if ( itheta >= 0 && itheta < ( int ) WaveDragMgr.m_ThetaRad.size() )
    {
        theta_deg = WaveDragMgr.m_ThetaRad[ itheta ] * 180.0 / M_PI;
    }

    // Runs are drawn at the selected angle, or the nearest they have, which the label then gives.
    vector < WaveDragRunAreas > runs;
    vector < string > labels;
    for ( int i = 0; i < ( int ) m_RunIDs.size(); i++ )
    {
        WaveDragRunAreas run;
        if ( ReadWaveDragRun( ResultsMgr.FindResultsPtr( m_RunIDs[i] ), theta_deg, run ) )
        {
            string label = RunLabel( m_RunIDs[i] );
            if ( std::abs( run.m_ThetaDeg - theta_deg ) > 1.0e-6 )
            {
                char buf[64];
                snprintf( buf, sizeof( buf ), " (theta %.1f)", run.m_ThetaDeg );
                label += buf;
            }
            runs.push_back( run );
            labels.push_back( label );
        }
    }
    int nrun = ( int ) runs.size();
    if ( nrun == 0 )
    {
        return;
    }

    // Limits that hold every run, each rounded as when it is drawn alone: x to about a tenth of
    // the body length, y up to a round number above the largest area.
    double xmin = 0;
    double xmax = 0;
    double ymax = 0;
    for ( int irun = 0; irun < nrun; irun++ )
    {
        const WaveDragRunAreas & r = runs[irun];
        double xscale = pow( 10.0, mag( r.m_EndX - r.m_StartX ) - 1 );
        double rxmin = xscale * floor( r.m_StartX / xscale - 0.1 );
        double rxmax = xscale * ceil( r.m_EndX / xscale + 0.1 );
        double yscale = pow( 10.0, mag( r.m_MaxMaxArea ) );
        double rymax = yscale * ceil( r.m_MaxMaxArea / yscale );
        if ( irun == 0 )
        {
            xmin = rxmin;
            xmax = rxmax;
            ymax = rymax;
        }
        else
        {
            xmin = std::min( xmin, rxmin );
            xmax = std::max( xmax, rxmax );
            ymax = std::max( ymax, rymax );
        }
    }

    m_canvas->current_x()->minimum( xmin );
    m_canvas->current_x()->maximum( xmax );

    m_canvas->current_y()->minimum( 0 );
    m_canvas->current_y()->maximum( ymax );

    if ( m_Zoomed )
    {
        m_canvas->current_x()->minimum( m_ZoomXMin );
        m_canvas->current_x()->maximum( m_ZoomXMax );
        m_canvas->current_y()->minimum( m_ZoomYMin );
        m_canvas->current_y()->maximum( m_ZoomYMax );
    }

    int plot_type = WaveDragMgr.m_AreaPlotType();
    bool points = WaveDragMgr.m_PointFlag();

    // Parts keep one color across runs.
    vector < string > part_ids;
    vector < string > part_names;
    vector < vector < int > > part_of_comp( nrun );
    for ( int irun = 0; irun < nrun; irun++ )
    {
        for ( int icomp = 0; icomp < ( int ) runs[irun].m_CompNames.size(); icomp++ )
        {
            int ipart = FindPart( part_ids, part_names, runs[irun].m_CompIDs[icomp], runs[irun].m_CompNames[icomp] );
            if ( ipart < 0 )
            {
                ipart = ( int ) part_ids.size();
                part_ids.push_back( runs[irun].m_CompIDs[icomp] );
                part_names.push_back( runs[irun].m_CompNames[icomp] );
            }
            part_of_comp[irun].push_back( ipart );
        }
    }

    for ( int irun = 0; irun < nrun; irun++ )
    {
        const WaveDragRunAreas & r = runs[irun];

        string prefix;
        if ( nrun > 1 )
        {
            prefix = labels[irun] + ": ";
        }

        // The total is black, dashed by run beside parts; alone, runs take colors of their own.
        Fl_Color total_color = FL_BLACK;
        int total_dash = FL_SOLID;
        int total_marker = CA_ROUND;
        if ( nrun > 1 && plot_type == WaveDragSingleton::AREA_TOTAL )
        {
            SeriesStyle( 0, 1, irun, nrun, total_color, total_dash, total_marker );
        }
        else if ( nrun > 1 )
        {
            Fl_Color run_color;
            SourceSeriesStyle( 0, irun, nrun, run_color, total_dash, total_marker );
        }

        // Legends show the markers of the slice points when they are drawn.
        int total_legend_marker = CA_NO_POINT;
        if ( points )
        {
            total_legend_marker = total_marker;
        }

        if ( plot_type == WaveDragSingleton::AREA_TOTAL )
        {
            AddPointLine( r.m_XFit, r.m_FitAreaFlow, 2, total_color, 0, 1, total_dash );
            m_canvas->AddSeriesPoints( r.m_XFit, r.m_FitAreaFlow );

            m_LegendLayout.AddLegendEntry( prefix + "Total", total_color, true, total_dash, total_legend_marker );
            m_canvas->NameNewSeries( prefix + "Total" );

            if ( points )
            {
                AddPoint( r.m_X, r.m_SliceAreaFlow, total_color, 3, total_marker );
            }
            continue;
        }

        if ( plot_type == WaveDragSingleton::AREA_COMPONENTS )
        {
            m_LegendLayout.AddLegendEntry( prefix + "Total", total_color, true, total_dash, total_legend_marker );
        }

        for ( int icomp = 0; icomp < ( int ) r.m_CompNames.size(); icomp++ )
        {
            int ipart = part_of_comp[irun][icomp];
            Fl_Color c;
            int dash;
            int marker;
            SeriesStyle( ipart, ( int ) part_ids.size(), irun, nrun, c, dash, marker );

            const vector < double > * fit = &r.m_CompFitArea[icomp];
            const vector < double > * pts = &r.m_CompSliceArea[icomp];
            if ( plot_type == WaveDragSingleton::AREA_BUILDUP )
            {
                fit = &r.m_BuildupFitArea[icomp];
                pts = &r.m_BuildupArea[icomp];
            }

            AddPointLine( r.m_XFit, *fit, 2, c, 0, 1, dash );
            m_canvas->AddSeriesPoints( r.m_XFit, *fit );

            int legend_marker = CA_NO_POINT;
            if ( points )
            {
                legend_marker = marker;
            }
            m_LegendLayout.AddLegendEntry( prefix + r.m_CompNames[icomp], c, true, dash, legend_marker );
            m_canvas->NameNewSeries( prefix + r.m_CompNames[icomp] );

            if ( points )
            {
                AddPoint( r.m_X, *pts, c, 3, marker );
            }
        }

        if ( plot_type == WaveDragSingleton::AREA_COMPONENTS )
        {
            AddPointLine( r.m_XFit, r.m_FitArea, 2, total_color, 0, 1, total_dash );
            m_canvas->AddSeriesPoints( r.m_XFit, r.m_FitArea );
            m_canvas->NameNewSeries( prefix + "Total" );
            if ( points )
            {
                AddPoint( r.m_X, r.m_SliceArea, total_color, 3, total_marker );
            }
        }
    }

    //==== Bodies of Revolution ====//
    // Index 0 = None
    // Index 1 = Sears-Haack
    // Index 2 = von Karman Ogive
    // Index 3 = Lighthill's Body
    // Drawn for the newest run plotted.

    const WaveDragRunAreas & last = runs.back();
    string last_prefix;
    if ( nrun > 1 )
    {
        last_prefix = labels.back() + ": ";
    }

    if ( WaveDragMgr.m_IdealBodyType.Get() == 0 ) // None
    {
        // Do nothing.
    }
    else
    {
        vector<double> S_ideal;
        string body;

        if ( WaveDragMgr.m_IdealBodyType.Get() == 1 ) // Sears-Haack curve
        {
            WaveDragMgr.SearsHaack( last.m_XNormFit, S_ideal, last.m_Volume, last.m_Length );
            body = "Sears-Haack Body";
        }
        else if ( WaveDragMgr.m_IdealBodyType.Get() == 2 ) // von Karman curve
        {
            WaveDragMgr.vonKarman( last.m_XNormFit, S_ideal, last.m_MaxArea );
            body = "Von Karman Ogive";
        }
        else if ( WaveDragMgr.m_IdealBodyType.Get() == 3 ) // Lighthill curve
        {
            WaveDragMgr.Lighthill( last.m_XNormFit, S_ideal, last.m_MaxArea );
            body = "Lighthill Body";
        }

        if ( plot_type != WaveDragSingleton::AREA_TOTAL )
        {
            for ( int i = 0; i < ( int ) S_ideal.size() && i < ( int ) last.m_XNormFit.size(); i++ )
            {
                S_ideal[i] = S_ideal[i] + last.m_InletArea * ( 1.0 - last.m_XNormFit[i] ) + last.m_ExitArea * last.m_XNormFit[i];
            }
        }

        m_LegendLayout.AddLegendEntry( last_prefix + body, FL_BLUE );
        AddPointLine( last.m_XFit, S_ideal, 2, FL_BLUE );
        m_canvas->AddSeriesPoints( last.m_XFit, S_ideal );
        m_canvas->NameNewSeries( last_prefix + body );
    }

    // The slice location belongs to the newest run, whose slices the view plane shows.
    if ( itheta >= ( int ) WaveDragMgr.m_StartX.size() || WaveDragMgr.m_lastmeshgeomID.empty() )
    {
        return;
    }

    if( m_SetMaxDragFlag )
    {
        m_SetMaxDragFlag = false;
        WaveDragMgr.m_SlicingLoc = WaveDragMgr.m_StartX[itheta] + WaveDragMgr.m_XMaxDrag[itheta] * ( WaveDragMgr.m_EndX[itheta] - WaveDragMgr.m_StartX[itheta] );
    }

    // Create points for vertical slice location bar
    vector <double> barx( 2, WaveDragMgr.m_SlicingLoc() );
    vector <double> bary;
    bary.push_back( 0 );
    bary.push_back( ymax );

    // Plot vertical slice location bar
    AddPointLine( barx, bary, 2, FL_BLACK );

    // Adjust slicing location slider
    WaveDragMgr.m_SlicingLoc.SetLowerLimit( WaveDragMgr.m_StartX[itheta] );
    WaveDragMgr.m_SlicingLoc.SetUpperLimit( WaveDragMgr.m_EndX[itheta] );
    m_SlicingLocSlider.SetRange( (WaveDragMgr.m_EndX[itheta] - WaveDragMgr.m_StartX[itheta])/2 );

}

void WaveDragScreen::LoadDrawObjs( vector< DrawObj* > & draw_obj_vec )
{
    if( IsShown() )
    {
        if ( WaveDragMgr.m_PlaneFlag.Get() )
        {
            WaveDragMgr.LoadDrawObjs( draw_obj_vec );
        }
    }
}

void WaveDragScreen::LimitsCallBack( double xmin, double xmax, double ymin, double ymax, bool reset )
{
    m_Zoomed = !reset;
    if ( !reset )
    {
        m_ZoomXMin = xmin;
        m_ZoomXMax = xmax;
        m_ZoomYMin = ymin;
        m_ZoomYMax = ymax;
    }
    m_ScreenMgr->SetUpdateFlag( true );
}

// A press on the slice location bar drags the bar; any other press zooms or pans.
bool WaveDragScreen::PressCallBack( int event, int mx, int my )
{
    if ( event == FL_PUSH )
    {
        m_Dragging = Fl::event_button() == FL_LEFT_MOUSE && hittest( mx, WaveDragMgr.m_SlicingLoc(), 5 );
        return m_Dragging;
    }
    if ( event == FL_DRAG && m_Dragging )
    {
        WaveDragMgr.m_SlicingLoc = m_canvas->current_x()->value( mx );
        m_ScreenMgr->SetUpdateFlag( true );
    }
    if ( event == FL_RELEASE )
    {
        m_Dragging = false;
    }
    return true;
}

bool WaveDragScreen::hittest( int mx, double datax, int r )
{
    if ( m_canvas )
    {
        int dx = std::abs( m_canvas->current_x()->position( datax ) - mx );

        if ( dx < r )
        {
            return true;
        }
    }
    return false;
}

void WaveDragScreen::UpdateRuns()
{
    vector < string > all;
    int n = ResultsMgr.GetNumResults( "WaveDrag" );
    for ( int i = 0; i < n; i++ )
    {
        all.push_back( ResultsMgr.FindResultsID( "WaveDrag", i ) );
    }

    PickRuns( all, m_KnownRunIDs, m_RunIDs );

    int vpos = m_RunsBrowser->vposition();
    int sel = m_RunsBrowser->value();
    m_RunsBrowser->clear();
    m_RunsBrowser->AddHeader( "@b@.Run\t@b@.Name\t@b@.Mach\t@b@.Set\t@b@.Made" );
    m_ListedRunIDs = all;
    for ( int i = 0; i < ( int ) all.size(); i++ )
    {
        Results* run = ResultsMgr.FindResultsPtr( all[i] );
        bool checked = std::find( m_RunIDs.begin(), m_RunIDs.end(), all[i] ) != m_RunIDs.end();

        char mach[64];
        mach[0] = '\0';
        if ( run->FindPtr( "Mach" ) )
        {
            snprintf( mach, sizeof( mach ), "%g", run->FindPtr( "Mach" )->GetDouble( 0 ) );
        }
        string set;
        if ( run->FindPtr( "Set_Name" ) )
        {
            set = run->FindPtr( "Set_Name" )->GetString( 0 );
        }

        string text = "Run " + std::to_string( i + 1 ) + "\t@." + run->GetNickname() + "\t" + mach + "\t@." + set + "\t" + GetResultsTimeText( run );
        m_RunsBrowser->AddLine( text, checked );
    }
    if ( sel > 0 && sel <= m_RunsBrowser->size() )
    {
        m_RunsBrowser->select( sel );
    }
    m_RunsBrowser->vposition( vpos );
}

string WaveDragScreen::RunLabel( const string & run_id )
{
    Results* res = ResultsMgr.FindResultsPtr( run_id );
    if ( res && !res->GetNickname().empty() )
    {
        return res->GetNickname();
    }

    vector < string >::const_iterator it = std::find( m_ListedRunIDs.begin(), m_ListedRunIDs.end(), run_id );
    return "Run " + std::to_string( ( int ) ( it - m_ListedRunIDs.begin() ) + 1 );
}
