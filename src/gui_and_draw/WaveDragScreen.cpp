//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//
// WaveDragScreen.cpp
//
// Michael Waddington
//////////////////////////////////////////////////////////////////////


#include "WaveDragScreen.h"
#include "SubSurfaceMgr.h"
#include "StlHelper.h"
#include "VspUtil.h"

/////////////////// Construction/Destruction /////////////////////////

WaveDragScreen::WaveDragScreen( ScreenMgr* mgr ) : TabScreen( mgr, 310 + 470, 450 + 72, "Wave Drag", 80, 470 )
{
    Vehicle *veh = VehicleMgr.GetVehicle();

    m_FLTK_Window->callback( staticCloseCB, this );
    m_FLTK_Window->resizable( 0 );

    if (!veh) return;

    Fl_Group* run_tab = AddTab( "Setup" );
    Fl_Group* flow_tab = AddTab( "Inflow/Outflow" );
    Fl_Group* plot_tab = AddTab( "Plot" );

    run_tab->show();

    //==== Run Tab ====//
    Fl_Group* run_group = AddSubGroup( run_tab, 5 );
    m_RunLayout.SetGroupAndScreen( run_group, this );

    m_RunLayout.SetButtonWidth( 100 );
    m_RunLayout.SetChoiceButtonWidth( 100 );

    // Slice Number, Rotation, and Set
    m_RunLayout.AddDividerBox( "Case Setup" );

    m_RunLayout.AddChoice( m_SelectedSetChoice, "Geometry Set: " );
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
    m_canvas = m_JustPlotLayout.AddCanvas( 400, 400, 0, 1, 0, 1, "", "Axis Location", "Area" );

    m_canvas->callback( staticScreenCB, this );

    //==== Flow Tab ====//
    Fl_Group* flow_group = AddSubGroup( flow_tab, 5 );
    m_FlowLayout.SetGroupAndScreen( flow_group, this );

    m_FlowLayout.AddDividerBox( "Select Inflow/Outflow Subsurfaces" );
    m_SubSurfFlowBrowser = m_FlowLayout.AddCheckBrowser( m_FlowLayout.GetRemainY() );
    m_SubSurfFlowBrowser->callback( staticScreenCB, this );

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
}

WaveDragScreen::~WaveDragScreen()
{
}

bool WaveDragScreen::Update()
{
    char str[256];
    WaveDragMgr.Update();

    Vehicle *veh = VehicleMgr.GetVehicle();
    if( veh )
    {
        vector <string> geomVec = veh->GetGeomVec();

        m_RefWingChoice.ClearItems();
        m_WingGeomVec.clear();
        map <string, int> WingCompIDMap;
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
                    sprintf(str, "%d_%s", i, g->GetName().c_str());

                    if (g->GetType().m_Type == MS_WING_GEOM_TYPE)
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
            tdval = WaveDragMgr.m_ThetaRad[ itheta ] * 180.0 / PI;
        }

        sprintf( str, "%.1f", tdval );
        m_ThetaDegValResult.Update( str );

        sprintf( str, "%7.4f", WaveDragMgr.m_CDWave );
        m_CDwResult.Update( string( str ) );

        m_SelectedBodyRev.Update( WaveDragMgr.m_IdealBodyType.GetID() );

        vector< string > set_name_vec = veh->GetSetNameVec();

        m_SelectedSetChoice.ClearItems();
        for ( int i = 0 ; i < ( int )set_name_vec.size() ; i++ )
        {
            m_SelectedSetChoice.AddItem( set_name_vec[i].c_str() );
        }
        m_SelectedSetChoice.UpdateItems();

        m_SelectedSetChoice.Update( WaveDragMgr.m_SelectedSetIndex.GetID() );

        // Removes any subsurf IDs from m_SSFlow_vec that don't exist in the model
        for ( int i = (int)WaveDragMgr.m_SSFlow_vec.size()-1; i >= 0 ; i--)
        {
            SubSurface* subsurf = SubSurfaceMgr.GetSubSurf( WaveDragMgr.m_SSFlow_vec[i] );
            if ( subsurf == NULL )
            {
                vector_remove_val( WaveDragMgr.m_SSFlow_vec, WaveDragMgr.m_SSFlow_vec[i] );
            }
        }

        //==== Flow Tab Update ====//
        vector< SubSurface* > subsurf_vec = SubSurfaceMgr.GetSubSurfs();

        m_SubSurfFlowBrowser->clear();
        // Load Subsurface Names
        for ( int i = 0; i < ( int )subsurf_vec.size() ; i++ )
        {
            string geomid = subsurf_vec[i]->GetCompID();
            Geom* gptr = veh->FindGeom( geomid );
            if ( gptr )
            {
                sprintf( str, "%s:  %s", gptr->GetName().c_str(), subsurf_vec[i]->GetName().c_str() );

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
    }

    RedrawPlot();

    m_FLTK_Window->redraw();

    return false;
}

void WaveDragScreen::Show()
{
    m_ScreenMgr->SetUpdateFlag( true );
    m_FLTK_Window->show();
}

void WaveDragScreen::Hide()
{
    m_FLTK_Window->hide();
    m_ScreenMgr->SetUpdateFlag( true );
}

void WaveDragScreen::CallBack( Fl_Widget* w )
{
    assert( m_ScreenMgr );

    Vehicle *veh = VehicleMgr.GetVehicle();

    if ( w == m_SubSurfFlowBrowser )
    {
        int curr_index = m_SubSurfFlowBrowser->value();
        vector< SubSurface* > subsurf_vec = SubSurfaceMgr.GetSubSurfs();

        if ( (int)subsurf_vec.size() > 0 && curr_index >= 1 && curr_index <= (int)subsurf_vec.size() )
        {

            bool flag = !!m_SubSurfFlowBrowser->checked( curr_index );

            string curr_ID = subsurf_vec[curr_index - 1]->GetID();

            if ( flag )
            {
                WaveDragMgr.m_SSFlow_vec.push_back( curr_ID );
            }
            else
            {
                vector_remove_val( WaveDragMgr.m_SSFlow_vec, curr_ID );
            }
        }
    }
    else if ( w == m_canvas )
    {
        int x = Fl::event_x();
        int y = Fl::event_y();

        if ( Fl::event() == FL_PUSH )
        {
            m_Dragging = hittest( x, WaveDragMgr.m_SlicingLoc(), 5 );
        }

        if ( Fl::event() == FL_DRAG && m_Dragging )
        {
            WaveDragMgr.m_SlicingLoc = m_canvas->current_x()->value( x );
        }

        if ( Fl::event() == FL_RELEASE )
        {
            m_Dragging = false;
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
        string newfile = m_ScreenMgr->GetSelectFileScreen()->FileChooser( "Choose Wave Drag Output File", "*.txt" );
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

    m_ScreenMgr->SetUpdateFlag( true );
}

void WaveDragScreen::RedrawPlot()
{
    if ( WaveDragMgr.m_lastmeshgeomID.empty() )
    {
        return;
    }

    Vehicle* veh = m_ScreenMgr->GetVehiclePtr();

    m_LegendGroup->clear();
    m_LegendLayout.SetGroup( m_LegendGroup );
    m_LegendLayout.InitWidthHeightVals();
    m_LegendLayout.SetButtonWidth( m_LegendLayout.GetW() * 0.8 );


    int itheta = WaveDragMgr.m_ThetaIndex() - 1;

    // Sets x-axis limits to rounded down/up to about 1/10th the body length.
    double xscale = pow( 10.0, mag( WaveDragMgr.m_EndX[itheta] - WaveDragMgr.m_StartX[itheta] ) - 1 );
    double xmin = xscale * floor( WaveDragMgr.m_StartX[itheta] / xscale - 0.1 );
    double xmax = xscale * ceil( WaveDragMgr.m_EndX[itheta] / xscale + 0.1 );

    // Sets max y-axis value to a round number greater than maximum area value
    double yscale = pow( 10.0, mag( WaveDragMgr.m_MaxMaxArea ) );
    double ymax = yscale * ceil( WaveDragMgr.m_MaxMaxArea / yscale );

    Vsp_Canvas::current( m_canvas );
    m_canvas->clear();

    m_canvas->current_x()->minimum( xmin );
    m_canvas->current_x()->maximum( xmax );

    m_canvas->current_y()->minimum( 0 );
    m_canvas->current_y()->maximum( ymax );

    unsigned int numpts = WaveDragMgr.m_NumPtsFit;
    vector< double > xfit(numpts);
    for ( unsigned int i = 0; i < numpts; i++ )
    {
        xfit[i] = WaveDragMgr.m_StartX[itheta] + WaveDragMgr.m_XNormFit[i] * ( WaveDragMgr.m_EndX[itheta] - WaveDragMgr.m_StartX[itheta] );
    }

    vector < double > xvec( WaveDragMgr.m_NSlice );
    for ( unsigned int i = 0; i < WaveDragMgr.m_NSlice; i++ )
    {
        xvec[i] = WaveDragMgr.m_StartX[itheta] + WaveDragMgr.m_XNorm[i] * ( WaveDragMgr.m_EndX[itheta] - WaveDragMgr.m_StartX[itheta] );
    }

    unsigned int nstyle = 5; // Number of CA line styles.

    if ( WaveDragMgr.m_AreaPlotType() == WaveDragSingleton::AREA_TOTAL )
    {
        AddPointLine( xfit, WaveDragMgr.m_FitAreaDistFlow[itheta], 2, FL_BLACK );

        m_LegendLayout.AddLegendEntry( "Total", FL_BLACK );

        if ( WaveDragMgr.m_PointFlag() )
        {
            AddPoint( xvec, WaveDragMgr.m_SliceAreaDistFlow[itheta], FL_BLACK, 3 );
        }
    }
    else if ( WaveDragMgr.m_AreaPlotType() == WaveDragSingleton::AREA_COMPONENTS )
    {
        m_LegendLayout.AddLegendEntry( "Total", FL_BLACK );

        // Components
        for ( int icomp = 0; icomp < WaveDragMgr.m_NComp; icomp++ )
        {
            Fl_Color c = ColorWheel( icomp, WaveDragMgr.m_NComp );
            AddPointLine( xfit, WaveDragMgr.m_CompFitAreaDist[itheta][icomp], 2, c );

            Geom *g = veh->FindGeom( WaveDragMgr.m_CompIDVec[icomp] );
            if ( g )
            {
                m_LegendLayout.AddLegendEntry( g->GetName(), c );
            }

            if ( WaveDragMgr.m_PointFlag() )
            {
                AddPoint( xvec, WaveDragMgr.m_CompSliceAreaDist[itheta][icomp], c, 3, StyleWheel( icomp ) );
            }
        }
        AddPointLine( xfit, WaveDragMgr.m_FitAreaDist[itheta], 2, FL_BLACK );
        if ( WaveDragMgr.m_PointFlag() )
        {
            AddPoint( xvec, WaveDragMgr.m_SliceAreaDist[itheta], FL_BLACK, 3 );
        }
    }
    else // AREA_BUILDUP
    {
        for ( int icomp = 0; icomp < WaveDragMgr.m_NComp; icomp++ )
        {
            Fl_Color c = ColorWheel( icomp, WaveDragMgr.m_NComp );
            AddPointLine( xfit, WaveDragMgr.m_BuildupFitAreaDist[itheta][icomp], 2, c );

            Geom *g = veh->FindGeom( WaveDragMgr.m_CompIDVec[icomp] );
            if ( g )
            {
                m_LegendLayout.AddLegendEntry( g->GetName(), c );
            }

            if ( WaveDragMgr.m_PointFlag() )
            {
                AddPoint( xvec, WaveDragMgr.m_BuildupAreaDist[itheta][icomp], c, 3, StyleWheel( icomp ) );
            }
        }
    }

    //==== Bodies of Revolution ====//
    // Index 0 = None
    // Index 1 = Sears-Haack
    // Index 2 = von Karman Ogive
    // Index 3 = Lighthill's Body

    double Smax = WaveDragMgr.m_MaxArea[itheta];
    double Vol = WaveDragMgr.m_Volume[itheta];
    double len = WaveDragMgr.m_Length[itheta];

    if ( WaveDragMgr.m_IdealBodyType.Get() == 0 ) // None
    {
        // Do nothing.
    }
    else
    {
        vector<double> S_ideal;

        if ( WaveDragMgr.m_IdealBodyType.Get() == 1 ) // Sears-Haack curve
        {
            WaveDragMgr.SearsHaack( WaveDragMgr.m_XNormFit, S_ideal, Vol, len );
            m_LegendLayout.AddLegendEntry( "Sears-Haack Body", FL_BLUE );
        }
        else if ( WaveDragMgr.m_IdealBodyType.Get() == 2 ) // von Karman curve
        {
            WaveDragMgr.vonKarman( WaveDragMgr.m_XNormFit, S_ideal, Smax );
            m_LegendLayout.AddLegendEntry( "Von Karman Ogive", FL_BLUE );
        }
        else if ( WaveDragMgr.m_IdealBodyType.Get() == 3 ) // Lighthill curve
        {
            WaveDragMgr.Lighthill( WaveDragMgr.m_XNormFit, S_ideal, Smax );
            m_LegendLayout.AddLegendEntry( "Lighthill Body", FL_BLUE );
        }

        if ( WaveDragMgr.m_AreaPlotType() != WaveDragSingleton::AREA_TOTAL )
        {
            for ( int i = 0; i < WaveDragMgr.m_NumPtsFit; i++ )
            {
                S_ideal[i] = S_ideal[i] + WaveDragMgr.m_InletArea * (1.0 - WaveDragMgr.m_XNormFit[i]) + WaveDragMgr.m_ExitArea * WaveDragMgr.m_XNormFit[i];
            }
        }

        AddPointLine( xfit, S_ideal, 2, FL_BLUE );
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
    if( m_FLTK_Window->visible() )
    {
        if ( WaveDragMgr.m_PlaneFlag.Get() )
        {
            WaveDragMgr.LoadDrawObjs( draw_obj_vec );
        }
    }
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
