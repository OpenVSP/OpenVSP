//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//
// Created by Rob McDonald on 6/11/24.
//
//////////////////////////////////////////////////////////////////////

#include "ResultsViewer.h"

#include "ScreenMgr.h"
#include "ResultsMgr.h"
#include "ResultsPlotMgr.h"
#include "StlHelper.h"

#include <FL/fl_draw.H>
#include <FL/Fl_Image_Surface.H>
#include <FL/Fl_PNG_Image.H>
#include <FL/Fl_SVG_File_Surface.H>
#include <FL/fl_utf8.h>

#include <algorithm>
#include <ctime>
#include <limits>

// Text for a Result in lists: nickname and case name when it has them, type, time and description.
string GetResultsListText( Results* res )
{
    if ( !res )
    {
        return string();
    }

    char timestr[512];
    time_t r_time = res->GetTimestamp();
    struct tm *r_tp = localtime( &r_time );
    strftime( timestr, sizeof( timestr ), "%r %D", r_tp );

    string text;
    if ( !res->GetNickname().empty() )
    {
        text = res->GetNickname() + " - ";
    }
    if ( !GetResultsCaseName( res ).empty() )
    {
        text += GetResultsCaseName( res ) + " - ";
    }
    text += res->GetName() + " - " + string( timestr ) + " - " + res->GetDoc();
    return text;
}

ResultsViewer::ResultsViewer( ScreenMgr* mgr ) : TabScreen( mgr, 1000, 800, "Results" )
{
    m_CurrDisplayGroup = NULL;

    m_FLTK_Window->callback( staticCloseCB, this );

    Fl_Group* data_tab = AddTab( "Data" );
    Fl_Group* plot_tab = AddTab( "Plot" );
    data_tab->show();

    // Picking a tab updates it.
    m_MenuTabs->callback( staticScreenCB, this );

    Fl_Group* data_group = AddSubGroup( data_tab, 5 );
    m_MainLayout.SetGroupAndScreen( data_group, this );

    m_MainLayout.AddSubGroupLayout( m_BorderLayout, m_MainLayout.GetRemainX(), m_MainLayout.GetRemainY() );

    m_BorderLayout.AddChoice( m_ResultsChoice, "Result" );

    m_BorderLayout.SetSameLineFlag( true );
    m_BorderLayout.SetFitWidthFlag( false );

    m_BorderLayout.SetButtonWidth( m_BorderLayout.GetW() / 3.0 );

    m_BorderLayout.AddButton( m_ExportCSVButton, "Export CSV" );
    m_BorderLayout.AddButton( m_DeleteButton, "Delete" );
    m_BorderLayout.AddButton( m_DeleteAllButton, "Delete All" );
    m_BorderLayout.ForceNewLine();

    m_BorderLayout.SetSameLineFlag( false );
    m_BorderLayout.SetFitWidthFlag( true );

    m_BorderLayout.SetButtonWidth( m_BorderLayout.GetChoiceButtonWidth() );
    m_BorderLayout.AddInput( m_NicknameInput, "Nickname" );

    m_BorderLayout.AddYGap();

    m_BorderLayout.AddDividerBox( "Data" );

    // Pointer for the widths of each column in the browser to support resizing
    // Last column width must be 0
    static int data_col_widths[] = { 150, 100, 200, 0 }; // widths for each column

    m_DataBrowser = m_BorderLayout.AddColResizeBrowser( data_col_widths, 3, 200 );
    m_DataBrowser->callback( staticScreenCB, this );

    m_BorderLayout.AddYGap();

    m_BorderLayout.AddSubGroupLayout( m_IntVectorLayout, m_BorderLayout.GetW(), m_BorderLayout.GetRemainY() );
    m_IntVectorLayout.AddDividerBox( "Vector of Integers" );

    m_IntVectorSpreadColumn = m_IntVectorLayout.AddSpreadSheet < int > ( m_IntVectorLayout.GetRemainY(), " %d");
    m_IntVectorSpreadColumn->set_AllowPasteGrowCols( false );

    m_BorderLayout.AddSubGroupLayout( m_DoubleVectorLayout, m_BorderLayout.GetW(), m_BorderLayout.GetRemainY() );
    m_DoubleVectorLayout.AddDividerBox( "Vector of Doubles" );

    m_DoubleVectorSpreadColumn = m_DoubleVectorLayout.AddSpreadSheet < double > ( m_DoubleVectorLayout.GetRemainY(), " %7.5f");
    m_DoubleVectorSpreadColumn->set_AllowPasteGrowCols( false );

    m_BorderLayout.AddSubGroupLayout( m_StringVectorLayout, m_BorderLayout.GetW(), m_BorderLayout.GetRemainY() );
    m_StringVectorLayout.AddDividerBox( "Vector of Strings" );

    m_StringVectorSpreadColumn = m_StringVectorLayout.AddSpreadSheet < string > ( m_StringVectorLayout.GetRemainY(), " %s");
    m_StringVectorSpreadColumn->set_LabelAlign( FL_ALIGN_LEFT | FL_ALIGN_WRAP );
    m_StringVectorSpreadColumn->set_AllowPasteGrowCols( false );

    int sw = m_StringVectorSpreadColumn->scrollbar_size();
    if ( sw == 0 )  // Check size, if zero, global size is used.
    {
        sw = Fl::scrollbar_size();
    }
    m_StringVectorSpreadColumn->col_default_width( m_StringVectorLayout.GetW() - 70 - 2 - sw );


    m_BorderLayout.AddSubGroupLayout( m_Vec3dVectorLayout, m_BorderLayout.GetW(), m_BorderLayout.GetRemainY() );
    m_Vec3dVectorLayout.AddDividerBox( "Vector of Vec3d" );

    m_Vec3dVectorSpreadColumn = m_Vec3dVectorLayout.AddSpreadSheet < vec3d > ( m_Vec3dVectorLayout.GetRemainY(), " %7.5f");
    m_Vec3dVectorSpreadColumn->set_AllowPasteGrowCols( false );
    m_Vec3dVectorSpreadColumn->set_HeaderOffset( 'X' - 'A' ); // start columns at X.

    m_BorderLayout.AddSubGroupLayout( m_DoubleMatrixLayout, m_BorderLayout.GetW(), m_BorderLayout.GetRemainY() );
    m_DoubleMatrixLayout.AddDividerBox( "Matrix of Doubles" );

    m_DoubleMatrixSpreadSheet = m_DoubleMatrixLayout.AddSpreadSheet< vector < double > > ( m_DoubleMatrixLayout.GetRemainY(), " %7.5f");

    // A taller window grows the data shown, which lies in one place for every type
    SetGrowWidget( m_IntVectorSpreadColumn );
    SetGrowWidget( m_DoubleVectorSpreadColumn );
    SetGrowWidget( m_StringVectorSpreadColumn );
    SetGrowWidget( m_Vec3dVectorSpreadColumn );
    SetGrowWidget( m_DoubleMatrixSpreadSheet );


    m_ResultsViewerSelect = -1;
    m_DataBrowserSelect = -1;

    DisplayGroup( &m_IntVectorLayout );

    BuildPlotTab( plot_tab );
}

ResultsViewer::~ResultsViewer()
{
}

void ResultsViewer::Show()
{
    m_ScreenMgr->SetUpdateFlag( true );
    BasicScreen::Show();
}

void ResultsViewer::Hide()
{
    VspScreen::Hide();
    m_ScreenMgr->SetUpdateFlag( true );
}

bool ResultsViewer::Update()
{
    TabScreen::Update();

    UpdateResultsChoice();

    UpdateDataBrowser();

    DisplayGroup( nullptr );
    if ( m_ResultsViewerSelect >= 0 && m_ResultsViewerSelect < m_ResIDvec.size() && m_ResIDvec.size() > 0 )
    {
        string resid = m_ResIDvec[ m_ResultsViewerSelect ];

        Results *res = ResultsMgr.FindResultsPtr( resid );

        if ( res )
        {
            m_ExportCSVButton.Activate();
            m_DeleteButton.Activate();
            m_NicknameInput.Update( res->GetNickname() );
            m_NicknameInput.Activate();
        }
        else
        {
            m_ExportCSVButton.Deactivate();
            m_DeleteButton.Deactivate();
            m_NicknameInput.Update( "" );
            m_NicknameInput.Deactivate();
        }

        if ( res && m_DataBrowserSelect >= 0 && m_DataBrowserSelect < m_DataNameIndexVec.size() && m_DataNameIndexVec.size() > 0 )
        {
            pair < string, int > name_index = m_DataNameIndexVec[ m_DataBrowserSelect ];
            NameValData *nvd = res->FindPtr( name_index.first, name_index.second );

            if ( nvd )
            {
                switch ( nvd->GetType())
                {
                    case vsp::INVALID_TYPE:
                    {
                        break;
                    }
                    case vsp::INT_DATA:
                    {
                        DisplayGroup( &m_IntVectorLayout );

                        m_IntVectorSpreadColumn->set_data( &( nvd->GetIntData() ) );

                        break;
                    }
                    case vsp::DOUBLE_DATA:
                    {
                        DisplayGroup( &m_DoubleVectorLayout );

                        m_DoubleVectorSpreadColumn->set_data( &( nvd->GetDoubleData() ) );

                        break;
                    }
                    case vsp::STRING_DATA:
                    {
                        DisplayGroup( &m_StringVectorLayout );

                        m_StringVectorSpreadColumn->set_data( &( nvd->GetStringData() ) );

                        break;
                    }
                    case vsp::VEC3D_DATA:
                    {
                        DisplayGroup( &m_Vec3dVectorLayout );

                        m_Vec3dVectorSpreadColumn->set_data( &( nvd->GetVec3dData() ) );

                        break;
                    }
                    case vsp::DOUBLE_MATRIX_DATA:
                    {
                        DisplayGroup( &m_DoubleMatrixLayout );

                        m_DoubleMatrixSpreadSheet->set_data( &( nvd->GetDoubleMatData() ) );

                        break;
                    }
                    default:
                    {
                        break;
                    }
                }
            }
        }
    }
    else
    {
        m_ExportCSVButton.Deactivate();
        m_DeleteButton.Deactivate();
        m_NicknameInput.Update( "" );
        m_NicknameInput.Deactivate();
    }

    // The plot is kept up to date only while it shows.
    if ( m_PlotTab->visible() )
    {
        UpdatePlotTab();
    }

    return true;
}

void ResultsViewer::UpdateResultsChoice()
{
    char str[2048];

    m_ResultsChoice.ClearItems();
    m_ResIDvec.clear();

    vector < string > resvec = ResultsMgr.GetAllResultsIDs();

    vector < pair < int, string > > count_ids;
    for ( int i = 0 ; i < (int)resvec.size() ; i++ )
    {
        Results * res = ResultsMgr.FindResultsPtr( resvec[i] );
        if ( res )
        {
            count_ids.emplace_back( make_pair( res->GetCreationCount(), resvec[ i ] ) );
        }
    }
    sort( count_ids.begin(), count_ids.end() );

    for ( int i = 0 ; i < (int)count_ids.size() ; i++ )
    {
        m_ResIDvec.emplace_back( count_ids[i].second );
    }

    for ( int i = 0 ; i < (int)m_ResIDvec.size() ; i++ )
    {
        Results * res = ResultsMgr.FindResultsPtr( m_ResIDvec[i] );

        if ( res )
        {
            snprintf( str, sizeof( str ), "%s\n", GetResultsListText( res ).c_str() );
            m_ResultsChoice.AddItem( str, i );
        }
    }
    m_ResultsChoice.UpdateItems();

    // With nothing picked, pick the newest Result.
    if ( ( m_ResultsViewerSelect < 0 || m_ResultsViewerSelect >= (int)m_ResIDvec.size() ) && !m_ResIDvec.empty() )
    {
        m_ResultsViewerSelect = (int)m_ResIDvec.size() - 1;
        m_DataBrowserSelect = 0;
    }

    if ( m_ResultsViewerSelect >= 0 && m_ResultsViewerSelect < (int)m_ResIDvec.size() )
    {
        m_ResultsChoice.SetVal( m_ResultsViewerSelect );
    }
}


void ResultsViewer::UpdateDataBrowser()
{
    char str[2048];
    char timestr[512];

    //==== Input and Output Browsers ====//
    int input_h_pos = m_DataBrowser->hposition();
    int input_v_pos = m_DataBrowser->vposition();

    m_DataBrowser->clear();
    m_DataNameIndexVec.clear();

    if ( m_ResultsViewerSelect >= 0 && m_ResultsViewerSelect < (int)m_ResIDvec.size() )
    {
        string resid = m_ResIDvec[ m_ResultsViewerSelect ];

        Results *res = ResultsMgr.FindResultsPtr( resid );

        if ( res )
        {
            m_DataBrowser->column_char( '|' );

            snprintf( str, sizeof( str ), "@b@.NAME|@b@.TYPE|@b@.DESCRIPTION" ); // @b@.VALUE|
            m_DataBrowser->add( str );


            vector< string > names = res->GetAllDataNames();

            for ( int i = 0; i < names.size(); i++ )
            {
                string dname = names[ i ];

                int ndata = res->GetNumData( dname );

                for ( int j = 0; j < ndata; j++ )
                {
                    NameValData * nvd = res->FindPtr( dname, j );

                    if ( nvd )
                    {
                        m_DataNameIndexVec.emplace_back( std::make_pair( dname, j ) );

                        string d_name = nvd->GetName();
                        string d_type = nvd->GetTypeName();
                        // string d_val = nvd->GetAsString();
                        string d_doc = nvd->GetDoc();

                        snprintf( str, sizeof( str ), "%s|%s|%s\n", d_name.c_str(), d_type.c_str(), d_doc.c_str() ); // |%s  d_val.c_str(),
                        m_DataBrowser->add( str );
                    }
                }
            }

            if ( m_DataBrowserSelect >= 0 && m_DataBrowserSelect < ( int ) m_DataNameIndexVec.size() )
            {
                m_DataBrowser->select( m_DataBrowserSelect + 2 );
            }

            m_DataBrowser->hposition( input_h_pos );
            m_DataBrowser->vposition( input_v_pos );
        }
    }
}


void ResultsViewer::CallBack( Fl_Widget* w )
{
    assert( m_ScreenMgr );

    if ( w == m_DataBrowser )
    {
        int sel = m_DataBrowser->value();
        m_DataBrowserSelect = sel - 2;
    }
    else
    {
        ResultsPlot* plot = ResultsPlotMgr.GetCurrPlot();
        if ( plot && plot->GetEditCurve() )
        {
            ResultsCurve* curve = plot->GetEditCurve();

            if ( w == m_CurveBrowser )
            {
                int sel = m_CurveBrowser->value();
                if ( sel > 0 )
                {
                    plot->SetEditIndex( sel - 1 );
                }
            }
            else if ( w == m_PlotResultsBrowser )
            {
                curve->m_Sources.clear();
                for ( int i = 0; i < ( int ) m_PlotSources.size(); i++ )
                {
                    if ( m_PlotResultsBrowser->selected( i + 1 ) )
                    {
                        curve->m_Sources.push_back( m_PlotSources[i] );
                    }
                }
            }
            else if ( w == m_PlotYBrowser )
            {
                curve->m_YLabels.clear();
                for ( int i = 0; i < ( int ) m_PlotYLabels.size(); i++ )
                {
                    if ( m_PlotYBrowser->selected( i + 1 ) )
                    {
                        curve->m_YLabels.push_back( m_PlotYLabels[i] );
                    }
                }
            }
        }
    }

    m_ScreenMgr->SetUpdateFlag( true );
}

void ResultsViewer::CloseCallBack( Fl_Widget *w )
{
    Hide();
}

// Callback for all other GUI Devices
void ResultsViewer::GuiDeviceCallBack( GuiDevice* device )
{
    if ( device == & m_ResultsChoice )
    {
        int sel = m_ResultsChoice.GetVal();
        m_ResultsViewerSelect = sel;
        m_DataBrowserSelect = 0;
        m_DataBrowser->vposition( 0 );
    }
    else if ( device == &m_ExportCSVButton)
    {
        if ( m_ResultsViewerSelect >= 0 && m_ResultsViewerSelect < (int)m_ResIDvec.size() )
        {
            string resid = m_ResIDvec[ m_ResultsViewerSelect ];

            Results *res = ResultsMgr.FindResultsPtr( resid );

            if ( res )
            {
                string fileName = m_ScreenMgr->FileChooser( "Select CSV File", "*.csv", vsp::SAVE );
                if ( !fileName.empty() )
                {
                    vector < string > resvec = { resid };
                    ResultsMgr.WriteCSVFile( fileName, resvec );
                }
            }
        }
    }
    else if ( device == & m_DeleteButton )
    {
        if ( m_ResultsViewerSelect >= 0 && m_ResultsViewerSelect < (int)m_ResIDvec.size() )
        {
            string resid = m_ResIDvec[ m_ResultsViewerSelect ];

            ResultsMgr.DeleteResult( resid );
        }
    }
    else if ( device == & m_DeleteAllButton )
    {
        ResultsMgr.DeleteAllResults();
    }
    else if ( device == &m_NicknameInput )
    {
        if ( m_ResultsViewerSelect >= 0 && m_ResultsViewerSelect < (int)m_ResIDvec.size() )
        {
            Results *res = ResultsMgr.FindResultsPtr( m_ResIDvec[ m_ResultsViewerSelect ] );
            if ( res )
            {
                res->SetNickname( m_NicknameInput.GetString() );
            }
        }
    }
    else if ( device == &m_PlotChoice )
    {
        ResultsPlotMgr.SetCurrIndex( m_PlotChoice.GetVal() );
    }
    else if ( device == &m_AddPlotButton )
    {
        ResultsPlotMgr.AddPlot();
    }
    else if ( device == &m_CopyPlotButton )
    {
        ResultsPlotMgr.CopyPlot( ResultsPlotMgr.GetCurrIndex() );
    }
    else if ( device == &m_DeletePlotButton )
    {
        ResultsPlotMgr.DeletePlot( ResultsPlotMgr.GetCurrIndex() );
    }
    else if ( device == &m_SavePlotFileButton )
    {
        string fname = m_ScreenMgr->FileChooser( "Save Results Plot", "*.vspplot", vsp::SAVE );
        if ( !fname.empty() && !ResultsPlotMgr.WritePlotFile( ResultsPlotMgr.GetCurrIndex(), fname ) )
        {
            m_ScreenMgr->Alert( "The plot was not written to " + fname );
        }
    }
    else if ( device == &m_LoadPlotFileButton )
    {
        string fname = m_ScreenMgr->FileChooser( "Load Results Plot", "*.vspplot" );
        if ( !fname.empty() && !ResultsPlotMgr.ReadPlotFile( fname ) )
        {
            m_ScreenMgr->Alert( fname + " holds no Results plot" );
        }
    }
    else if ( device == &m_AutoScaleButton )
    {
        LimitsCallBack( 0, 0, 0, 0, true );
    }
    else if ( device == &m_ExportPlotCSVButton )
    {
        ExportPlotCSV( m_ScreenMgr->FileChooser( "Export Plotted Data", "*.csv", vsp::SAVE ) );
    }
    else if ( device == &m_SavePlotButton )
    {
        SavePlotImage( m_ScreenMgr->FileChooser( "Save Plot Image", "*.{png,svg}", vsp::SAVE ) );
    }
    else if ( ResultsPlotMgr.GetCurrPlot() )
    {
        ResultsPlot* plot = ResultsPlotMgr.GetCurrPlot();
        ResultsCurve* curve = plot->GetEditCurve();

        if ( device == &m_PlotNameInput )
        {
            plot->SetName( m_PlotNameInput.GetString() );
        }
        else if ( device == &m_PlotXChoice && curve )
        {
            int sel = m_PlotXChoice.GetVal();
            if ( sel > 0 && sel <= ( int ) m_PlotXLabels.size() )
            {
                curve->m_XLabel = m_PlotXLabels[ sel - 1 ];
            }
            else
            {
                curve->m_XLabel.clear();
            }
        }
        else if ( ( device == &m_PlotRunChoice || device == &m_ChooseRunsButton ) && curve )
        {
            if ( device == &m_ChooseRunsButton )
            {
                curve->m_Run.Set( vsp::RESULTS_RUN_CHOSEN );
            }

            if ( curve->m_Run() == vsp::RESULTS_RUN_CHOSEN )
            {
                m_ScreenMgr->ShowScreen( vsp::VSP_RESULTS_CHOOSER_SCREEN );
            }
        }
        else if ( device == &m_AddCurveButton )
        {
            // Starting from the curve being edited makes a variation, such as another run, quick to set up.
            plot->AddCurve( plot->GetEditCurve() );
        }
        else if ( device == &m_DeleteCurveButton )
        {
            plot->DeleteCurve( plot->GetEditIndex() );
        }
        else if ( device == &m_DeleteAllCurvesButton )
        {
            plot->DeleteAllCurves();
        }
    }


    m_ScreenMgr->SetUpdateFlag( true );
}

void ResultsViewer::DisplayGroup( GroupLayout* group )
{
    if ( m_CurrDisplayGroup == group )
    {
        return;
    }

    m_IntVectorLayout.Hide();
    m_DoubleVectorLayout.Hide();
    m_StringVectorLayout.Hide();
    m_Vec3dVectorLayout.Hide();
    m_DoubleMatrixLayout.Hide();

    m_CurrDisplayGroup = group;

    if (group)
    {
        group->Show();
    }
}

void ResultsViewer::SetSelectedResult( string rid )
{
    // Make sure m_ResIDvec is up-to-date.
    UpdateResultsChoice();
    m_ResultsViewerSelect = vector_find_val( m_ResIDvec, rid );
}

void ResultsViewer::SetLastResult()
{
    // Make sure m_ResIDvec is up-to-date.
    UpdateResultsChoice();
    m_ResultsViewerSelect = m_ResIDvec.size() - 1;
}

//==== Plot Tab ====//

void ResultsViewer::BuildPlotTab( Fl_Group* tab )
{
    m_PlotTab = tab;
    m_PlotCanvas = nullptr;
    m_PlotGroup = nullptr;

    Fl_Group* plot_group = AddSubGroup( tab, 5 );
    m_PlotTabLayout.SetGroupAndScreen( plot_group, this );

    int control_w = 300;
    int gap = 5;

    //==== Controls ====//
    m_PlotTabLayout.AddSubGroupLayout( m_PlotControlLayout, control_w, m_PlotTabLayout.GetH() );
    Fl_Group* control_group = m_PlotControlLayout.GetGroup();

    // Rows besides the two variable browsers, which share what is left.
    int curve_h = 75;
    int nrows = 12;
    int ndiv = 5;
    int ngap = 5;
    int fixed_h = nrows * m_PlotControlLayout.GetStdHeight() + ndiv * m_PlotControlLayout.GetDividerHeight() + curve_h + ngap * m_PlotControlLayout.GetGapHeight();
    int browse_h = ( m_PlotControlLayout.GetH() - fixed_h ) / 2;

    m_PlotControlLayout.AddDividerBox( "Curves" );
    m_CurveBrowser = m_PlotControlLayout.AddVspBrowser( curve_h );
    m_CurveBrowser->type( FL_HOLD_BROWSER );
    m_CurveBrowser->callback( staticScreenCB, this );

    m_PlotControlLayout.SetSameLineFlag( true );
    m_PlotControlLayout.SetFitWidthFlag( false );
    m_PlotControlLayout.SetButtonWidth( m_PlotControlLayout.GetW() / 3 );
    m_PlotControlLayout.AddButton( m_AddCurveButton, "Add" );
    m_PlotControlLayout.AddButton( m_DeleteCurveButton, "Delete" );
    m_PlotControlLayout.AddButton( m_DeleteAllCurvesButton, "Delete All" );
    m_PlotControlLayout.ForceNewLine();
    m_PlotControlLayout.SetSameLineFlag( false );
    m_PlotControlLayout.SetFitWidthFlag( true );
    m_PlotControlLayout.AddYGap();

    m_PlotControlLayout.AddDividerBox( "Results" );
    m_PlotResultsBrowser = m_PlotControlLayout.AddVspBrowser( browse_h );
    m_PlotResultsBrowser->type( FL_MULTI_BROWSER );
    m_PlotResultsBrowser->callback( staticScreenCB, this );

    m_PlotControlLayout.SetChoiceButtonWidth( 70 );

    int choose_w = 70;
    m_PlotControlLayout.SetSameLineFlag( true );
    m_PlotControlLayout.AddChoice( m_PlotRunChoice, "Runs", choose_w );
    m_PlotControlLayout.SetFitWidthFlag( false );
    m_PlotControlLayout.SetButtonWidth( choose_w );
    m_PlotControlLayout.AddButton( m_ChooseRunsButton, "Choose..." );
    m_PlotControlLayout.ForceNewLine();
    m_PlotControlLayout.SetSameLineFlag( false );
    m_PlotControlLayout.SetFitWidthFlag( true );

    m_PlotModeChoice.AddItem( "Vectors", vsp::RESULTS_PLOT_VECTORS );
    m_PlotModeChoice.AddItem( "Across Results", vsp::RESULTS_PLOT_ACROSS_RESULTS );
    m_PlotControlLayout.AddChoice( m_PlotModeChoice, "Plot" );

    m_PlotElementChoice.AddItem( "First", vsp::RESULTS_PLOT_FIRST );
    m_PlotElementChoice.AddItem( "Last", vsp::RESULTS_PLOT_LAST );
    m_PlotControlLayout.AddChoice( m_PlotElementChoice, "Element" );

    m_PlotMatrixChoice.AddItem( "Columns", 0 );
    m_PlotMatrixChoice.AddItem( "Rows", 1 );
    m_PlotControlLayout.AddChoice( m_PlotMatrixChoice, "Matrix" );

    m_PlotControlLayout.AddChoice( m_PlotXChoice, "X" );
    m_PlotControlLayout.AddYGap();

    m_PlotControlLayout.AddDividerBox( "Y" );
    m_PlotYBrowser = m_PlotControlLayout.AddVspBrowser( browse_h );
    m_PlotYBrowser->type( FL_MULTI_BROWSER );
    m_PlotYBrowser->callback( staticScreenCB, this );
    m_PlotControlLayout.AddYGap();

    m_PlotControlLayout.AddDividerBox( "Axes" );

    int toggle_w = 15;
    int label_w = 40;
    int input_w = ( m_PlotControlLayout.GetW() - 2 * ( toggle_w + label_w ) ) / 2;

    m_PlotControlLayout.SetSameLineFlag( true );
    m_PlotControlLayout.SetFitWidthFlag( false );
    m_PlotControlLayout.SetInputWidth( input_w );

    m_PlotControlLayout.SetButtonWidth( toggle_w );
    m_PlotControlLayout.AddButton( m_XMinToggle, "" );
    m_PlotControlLayout.SetButtonWidth( label_w );
    m_PlotControlLayout.AddInput( m_XMinInput, "X Min", "%g", 0, 1.0 );
    m_PlotControlLayout.SetButtonWidth( toggle_w );
    m_PlotControlLayout.AddButton( m_XMaxToggle, "" );
    m_PlotControlLayout.SetButtonWidth( label_w );
    m_PlotControlLayout.AddInput( m_XMaxInput, "Max", "%g", 0, 1.0 );
    m_PlotControlLayout.ForceNewLine();

    m_PlotControlLayout.SetButtonWidth( toggle_w );
    m_PlotControlLayout.AddButton( m_YMinToggle, "" );
    m_PlotControlLayout.SetButtonWidth( label_w );
    m_PlotControlLayout.AddInput( m_YMinInput, "Y Min", "%g", 0, 1.0 );
    m_PlotControlLayout.SetButtonWidth( toggle_w );
    m_PlotControlLayout.AddButton( m_YMaxToggle, "" );
    m_PlotControlLayout.SetButtonWidth( label_w );
    m_PlotControlLayout.AddInput( m_YMaxInput, "Max", "%g", 0, 1.0 );
    m_PlotControlLayout.ForceNewLine();

    m_PlotControlLayout.SetButtonWidth( m_PlotControlLayout.GetW() / 3 );
    m_PlotControlLayout.AddButton( m_XLogToggle, "Log X" );
    m_PlotControlLayout.AddButton( m_YLogToggle, "Log Y" );
    m_PlotControlLayout.AddButton( m_AutoScaleButton, "Auto" );
    m_PlotControlLayout.ForceNewLine();
    m_PlotControlLayout.AddYGap();

    m_PlotControlLayout.SetFitWidthFlag( true );
    m_PlotControlLayout.SetSameLineFlag( false );

    m_PlotControlLayout.AddDividerBox( "Style" );

    m_PlotControlLayout.SetSameLineFlag( true );
    m_PlotControlLayout.SetFitWidthFlag( false );
    m_PlotControlLayout.SetButtonWidth( m_PlotControlLayout.GetW() / 2 );
    m_PlotControlLayout.AddButton( m_ShowLinesToggle, "Lines" );
    m_PlotControlLayout.AddButton( m_ShowPointsToggle, "Points" );
    m_PlotControlLayout.ForceNewLine();
    m_PlotControlLayout.SetSameLineFlag( false );
    m_PlotControlLayout.SetFitWidthFlag( true );

    m_LegendChoice.AddItem( "Best", vsp::RESULTS_LEGEND_BEST );
    m_LegendChoice.AddItem( "Upper Right", vsp::RESULTS_LEGEND_UPPER_RIGHT );
    m_LegendChoice.AddItem( "Upper Left", vsp::RESULTS_LEGEND_UPPER_LEFT );
    m_LegendChoice.AddItem( "Lower Left", vsp::RESULTS_LEGEND_LOWER_LEFT );
    m_LegendChoice.AddItem( "Lower Right", vsp::RESULTS_LEGEND_LOWER_RIGHT );
    m_LegendChoice.AddItem( "Off", vsp::RESULTS_LEGEND_OFF );
    m_PlotControlLayout.AddChoice( m_LegendChoice, "Legend" );
    m_PlotControlLayout.AddYGap();

    m_PlotControlLayout.SetSameLineFlag( true );
    m_PlotControlLayout.SetFitWidthFlag( false );
    m_PlotControlLayout.SetButtonWidth( m_PlotControlLayout.GetW() / 2 );
    m_PlotControlLayout.AddButton( m_ExportPlotCSVButton, "Export CSV" );
    m_PlotControlLayout.AddButton( m_SavePlotButton, "Save Image" );
    m_PlotControlLayout.ForceNewLine();
    m_PlotControlLayout.SetSameLineFlag( false );
    m_PlotControlLayout.SetFitWidthFlag( true );

    // The Results browser takes the control column's growth.
    control_group->resizable( m_PlotResultsBrowser );

    //==== Plot ====//
    m_PlotTabLayout.AddX( control_w + gap );
    m_PlotTabLayout.AddSubGroupLayout( m_PlotAreaLayout, m_PlotTabLayout.GetW() - control_w - gap, m_PlotTabLayout.GetH() );
    Fl_Group* area_group = m_PlotAreaLayout.GetGroup();

    //==== Plots of the model ====//
    int choice_w = 180;
    int plot_button_w = 55;
    m_PlotAreaLayout.SetSameLineFlag( true );
    m_PlotAreaLayout.SetFitWidthFlag( false );
    m_PlotAreaLayout.SetChoiceButtonWidth( 40 );
    m_PlotAreaLayout.SetSliderWidth( choice_w );
    m_PlotAreaLayout.AddChoice( m_PlotChoice, "Plot" );
    m_PlotAreaLayout.AddX( gap );
    m_PlotAreaLayout.SetButtonWidth( 45 );
    m_PlotAreaLayout.SetInputWidth( m_PlotAreaLayout.GetW() - 40 - choice_w - 45 - 5 * plot_button_w - 2 * gap );
    m_PlotAreaLayout.AddInput( m_PlotNameInput, "Name" );
    m_PlotAreaLayout.AddX( gap );
    m_PlotAreaLayout.SetButtonWidth( plot_button_w );
    m_PlotAreaLayout.AddButton( m_AddPlotButton, "Add" );
    m_PlotAreaLayout.AddButton( m_CopyPlotButton, "Copy" );
    m_PlotAreaLayout.AddButton( m_DeletePlotButton, "Delete" );
    m_PlotAreaLayout.AddButton( m_SavePlotFileButton, "Save..." );
    m_PlotAreaLayout.AddButton( m_LoadPlotFileButton, "Load..." );
    m_PlotAreaLayout.ForceNewLine();
    m_PlotAreaLayout.SetSameLineFlag( false );
    m_PlotAreaLayout.SetFitWidthFlag( true );
    m_PlotAreaLayout.AddYGap();

    int head_h = m_PlotAreaLayout.GetY() - area_group->y();
    int readout_h = 2 * m_PlotAreaLayout.GetStdHeight();
    int margin = 10;

    GroupLayout canvas_layout;
    m_PlotAreaLayout.AddSubGroupLayout( canvas_layout, m_PlotAreaLayout.GetW(), m_PlotAreaLayout.GetH() - head_h - readout_h );
    m_PlotGroup = canvas_layout.GetGroup();
    m_PlotGroup->box( FL_FLAT_BOX );
    m_PlotGroup->color( FL_WHITE );

    // Room above the plot for the Y axis label.
    int top = 30;
    canvas_layout.AddY( top );
    canvas_layout.AddX( margin );
    m_PlotCanvas = new ResultsPlotCanvas( 0, 0, 10, 10 );
    canvas_layout.AddCanvas( canvas_layout.GetW() - 2 * margin, canvas_layout.GetH() - top - margin - 10, 0, 1, 0, 1, "", "X", "Y", m_PlotCanvas );
    m_PlotCanvas->current_x()->label_format( "%g" );
    m_PlotCanvas->current_y()->label_format( "%g" );
    // A long Y label runs right over the plot, not left out of it.
    m_PlotCanvas->current_y()->align( Fl_Align( FL_ALIGN_TOP_LEFT ) );
    m_PlotCanvas->SetLimitsCallback( staticLimitsCB, this );
    m_PlotCanvas->SetReadoutCallback( staticReadoutCB, this );
    m_PlotGroup->resizable( m_PlotCanvas );

    m_PlotAreaLayout.AddY( m_PlotAreaLayout.GetH() - head_h - readout_h );
    m_ReadoutBox = m_PlotAreaLayout.AddDividerBox( "", 0, FL_BACKGROUND_COLOR );
    m_ReadoutBox->box( FL_NO_BOX );
    m_ReadoutBox->labelcolor( FL_BLACK );
    m_ReadoutBox->labelfont( FL_HELVETICA );
    m_ReadoutBox->align( FL_ALIGN_LEFT | FL_ALIGN_INSIDE | FL_ALIGN_CLIP );
    m_WarningBox = m_PlotAreaLayout.AddDividerBox( "", 0, FL_BACKGROUND_COLOR );
    m_WarningBox->box( FL_NO_BOX );
    m_WarningBox->labelcolor( FL_DARK_RED );
    m_WarningBox->labelfont( FL_HELVETICA );
    m_WarningBox->align( FL_ALIGN_LEFT | FL_ALIGN_INSIDE | FL_ALIGN_CLIP );

    area_group->resizable( m_PlotGroup );
    plot_group->resizable( area_group );

    // The window shrinks only until the Results browser is 40 high and the plot area 300 wide.
    int min_browse_h = 40;
    int min_area_w = 300;
    int shrink_h = std::max( 0, browse_h - min_browse_h );
    int shrink_w = std::max( 0, area_group->w() - min_area_w );
    m_FLTK_Window->size_range( m_FLTK_Window->w() - shrink_w, m_FLTK_Window->h() - shrink_h );
}

// A label with FLTK's symbol character escaped.
static string PlainLabel( const string & s )
{
    string out;
    for ( int i = 0; i < ( int ) s.size(); i++ )
    {
        out += s[i];
        if ( s[i] == '@' )
        {
            out += '@';
        }
    }
    return out;
}

// Scroll a browser to its first selected line.
static void ShowFirstSelected( VspBrowser* browser )
{
    for ( int i = 1; i <= browser->size(); i++ )
    {
        if ( browser->selected( i ) )
        {
            browser->topline( i );
            return;
        }
    }
    browser->topline( 1 );
}

void ResultsViewer::UpdatePlotTab()
{
    if ( !m_PlotCanvas )
    {
        return;
    }

    UpdatePlotChoice();

    ResultsPlot* plot = ResultsPlotMgr.GetCurrPlot();
    if ( !plot )
    {
        m_PlotControlLayout.GetGroup()->deactivate();
        m_PlotNameInput.Update( "" );
        m_PlotNameInput.Deactivate();
        m_CopyPlotButton.Deactivate();
        m_DeletePlotButton.Deactivate();
        m_SavePlotFileButton.Deactivate();
        m_CurveBrowser->clear();
        m_PlotResultsBrowser->clear();
        m_PlotYBrowser->clear();
    }
    else
    {
        m_PlotControlLayout.GetGroup()->activate();
        m_PlotNameInput.Update( plot->GetName() );
        m_PlotNameInput.Activate();
        m_CopyPlotButton.Activate();
        m_DeletePlotButton.Activate();
        m_SavePlotFileButton.Activate();

        plot->Update();
        UpdateCurveBrowser( plot );

        ResultsCurve* curve = plot->GetEditCurve();
        if ( curve )
        {
            m_PlotResultsBrowser->activate();
            m_PlotYBrowser->activate();
            m_PlotXChoice.Activate();

            UpdatePlotResultsBrowser( curve );
            UpdatePlotRunChoice( curve );

            UpdatePlotVarLists( curve );

            // A curve picked for editing shows its picks.
            if ( curve->GetID() != m_PlotEditCurveID )
            {
                m_PlotEditCurveID = curve->GetID();
                ShowFirstSelected( m_PlotResultsBrowser );
                ShowFirstSelected( m_PlotYBrowser );
            }

            m_PlotModeChoice.Update( curve->m_Mode.GetID() );
            m_PlotElementChoice.Update( curve->m_Element.GetID() );
            m_PlotMatrixChoice.Update( curve->m_MatrixRows.GetID() );

            if ( curve->m_Mode() == vsp::RESULTS_PLOT_ACROSS_RESULTS )
            {
                m_PlotElementChoice.Activate();
            }
            else
            {
                m_PlotElementChoice.Deactivate();
            }
        }
        else
        {
            // Nothing to edit until a curve is added.
            m_PlotResultsBrowser->clear();
            m_PlotYBrowser->clear();
            m_PlotResultsBrowser->deactivate();
            m_PlotYBrowser->deactivate();
            m_PlotRunChoice.Update( "" );
            m_PlotModeChoice.Update( "" );
            m_PlotElementChoice.Update( "" );
            m_PlotMatrixChoice.Update( "" );
            m_PlotXChoice.Deactivate();
        }

        m_LegendChoice.Update( plot->m_Legend.GetID() );
        m_ShowLinesToggle.Update( plot->m_ShowLines.GetID() );
        m_ShowPointsToggle.Update( plot->m_ShowPoints.GetID() );
        m_XLogToggle.Update( plot->m_XLog.GetID() );
        m_YLogToggle.Update( plot->m_YLog.GetID() );
    }

    vector < ResultsSeriesDef > defs;
    string sig;
    if ( plot )
    {
        plot->GetSeriesDefs( defs );

        char buf[256];
        snprintf( buf, sizeof( buf ), "%d %d %d %d %d", ( int ) plot->m_ShowLines(), ( int ) plot->m_ShowPoints(),
                  ( int ) plot->m_XLog(), ( int ) plot->m_YLog(), plot->m_Legend() );
        sig = plot->GetID() + " " + buf + "\n";

        for ( int i = 0; i < plot->GetNumCurves(); i++ )
        {
            sig += plot->GetCurve( i )->GetText() + "\n";
        }
    }
    else
    {
        // Differs from the empty signature the tab starts with, so the first update draws.
        sig = "No plot";
    }

    // Rebuild the series only when what they come from changed.
    for ( int i = 0; i < ( int ) defs.size(); i++ )
    {
        char dbuf[64];
        snprintf( dbuf, sizeof( dbuf ), "%d %d %d|", defs[i].m_Mode, defs[i].m_Element, ( int ) defs[i].m_MatrixRows );
        sig += dbuf + defs[i].m_XLabel + "|" + defs[i].m_YLabel + "|";
        // The legend names each Result by its display name, which a nickname changes.
        for ( int j = 0; j < ( int ) defs[i].m_ResultIDs.size(); j++ )
        {
            sig += defs[i].m_ResultIDs[j] + " " + GetResultsDisplayName( ResultsMgr.FindResultsPtr( defs[i].m_ResultIDs[j] ) ) + ",";
        }
        sig += "\n";
    }

    if ( sig != m_PlotSignature )
    {
        m_PlotSignature = sig;
        RedrawPlot( plot );
    }

    UpdatePlotAxes( plot );
}

void ResultsViewer::UpdatePlotChoice()
{
    m_PlotChoice.ClearItems();
    for ( int i = 0; i < ResultsPlotMgr.GetNumPlots(); i++ )
    {
        m_PlotChoice.AddItem( ResultsPlotMgr.GetPlot( i )->GetName(), i );
    }
    if ( ResultsPlotMgr.GetNumPlots() == 0 )
    {
        m_PlotChoice.AddItem( "None", -1 );
    }
    m_PlotChoice.UpdateItems();
    m_PlotChoice.SetVal( ResultsPlotMgr.GetCurrIndex() );

    if ( ResultsPlotMgr.GetNumPlots() == 0 )
    {
        m_PlotChoice.Deactivate();
    }
    else
    {
        m_PlotChoice.Activate();
    }
}

void ResultsViewer::UpdateCurveBrowser( ResultsPlot* plot )
{
    int vpos = m_CurveBrowser->vposition();
    m_CurveBrowser->clear();

    int ncurve = plot->GetNumCurves();
    for ( int i = 0; i < ncurve; i++ )
    {
        string line = "@." + plot->GetCurve( i )->GetText();
        m_CurveBrowser->add( line.c_str() );
    }

    int edit = plot->GetEditIndex();
    if ( edit >= 0 )
    {
        m_CurveBrowser->select( edit + 1 );
    }
    m_CurveBrowser->vposition( vpos );

    if ( ncurve > 0 )
    {
        m_DeleteCurveButton.Activate();
        m_DeleteAllCurvesButton.Activate();
    }
    else
    {
        m_DeleteCurveButton.Deactivate();
        m_DeleteAllCurvesButton.Deactivate();
    }
}

void ResultsViewer::UpdatePlotResultsBrowser( ResultsCurve* curve )
{
    vector < int > counts;
    ListResultsSources( m_PlotSources, counts );

    // Kinds the curve plots stay listed while none of their Results exist.
    for ( int i = 0; i < ( int ) curve->m_Sources.size(); i++ )
    {
        if ( std::find( m_PlotSources.begin(), m_PlotSources.end(), curve->m_Sources[i] ) == m_PlotSources.end() )
        {
            m_PlotSources.push_back( curve->m_Sources[i] );
            counts.push_back( 0 );
        }
    }

    int vpos = m_PlotResultsBrowser->vposition();
    m_PlotResultsBrowser->clear();

    for ( int i = 0; i < ( int ) m_PlotSources.size(); i++ )
    {
        char buf[64];
        if ( counts[i] > 0 )
        {
            snprintf( buf, sizeof( buf ), "  (%d)", counts[i] );
        }
        else
        {
            snprintf( buf, sizeof( buf ), "  (none)" );
        }
        string line = "@." + m_PlotSources[i].GetText() + buf;
        m_PlotResultsBrowser->add( line.c_str() );

        if ( std::find( curve->m_Sources.begin(), curve->m_Sources.end(), m_PlotSources[i] ) != curve->m_Sources.end() )
        {
            m_PlotResultsBrowser->select( i + 1 );
        }
    }
    m_PlotResultsBrowser->vposition( vpos );
}

void ResultsViewer::UpdatePlotRunChoice( ResultsCurve* curve )
{
    // As many runs as the curve's most run kind of Result has.
    vector < ResultsSource > sources;
    vector < int > counts;
    ListResultsSources( sources, counts );

    int nrun = std::max( curve->m_Run(), 1 );
    for ( int i = 0; i < ( int ) sources.size(); i++ )
    {
        if ( std::find( curve->m_Sources.begin(), curve->m_Sources.end(), sources[i] ) != curve->m_Sources.end() )
        {
            nrun = std::max( nrun, counts[i] );
        }
    }

    m_PlotRunChoice.ClearItems();
    m_PlotRunChoice.AddItem( "All", vsp::RESULTS_RUN_ALL );
    m_PlotRunChoice.AddItem( "Chosen", vsp::RESULTS_RUN_CHOSEN );
    m_PlotRunChoice.AddItem( "Newest", vsp::RESULTS_RUN_NEWEST );
    for ( int k = 2; k <= nrun; k++ )
    {
        char buf[64];
        snprintf( buf, sizeof( buf ), "Newest - %d", k - 1 );
        m_PlotRunChoice.AddItem( buf, k );
    }
    m_PlotRunChoice.UpdateItems();
    m_PlotRunChoice.Update( curve->m_Run.GetID() );
}

void ResultsViewer::UpdatePlotVarLists( ResultsCurve* curve )
{
    bool rows = curve->m_MatrixRows();

    // Variables of every run of the curve's kinds of Result, in the order first met.
    vector < string > labels;
    vector < int > lens;
    for ( int i = 0; i < ( int ) m_ResIDvec.size(); i++ )
    {
        Results* res = ResultsMgr.FindResultsPtr( m_ResIDvec[i] );

        bool use = false;
        for ( int s = 0; s < ( int ) curve->m_Sources.size(); s++ )
        {
            if ( curve->m_Sources[s].Matches( res ) )
            {
                use = true;
            }
        }
        if ( !use )
        {
            continue;
        }

        vector < ResultsPlotVar > vars;
        ListResultsPlotVars( res, rows, vars );
        for ( int j = 0; j < ( int ) vars.size(); j++ )
        {
            if ( std::find( labels.begin(), labels.end(), vars[j].m_Label ) == labels.end() )
            {
                labels.push_back( vars[j].m_Label );
                lens.push_back( vars[j].m_Len );
            }
        }
    }

    // Variables the curve plots stay listed while no Result has them.
    vector < string > picked = curve->m_YLabels;
    if ( !curve->m_XLabel.empty() )
    {
        picked.push_back( curve->m_XLabel );
    }
    for ( int i = 0; i < ( int ) picked.size(); i++ )
    {
        if ( std::find( labels.begin(), labels.end(), picked[i] ) == labels.end() )
        {
            labels.push_back( picked[i] );
            lens.push_back( -1 );
        }
    }

    //==== X ====//
    m_PlotXLabels = labels;
    m_PlotXChoice.ClearItems();
    if ( curve->m_Mode() == vsp::RESULTS_PLOT_ACROSS_RESULTS )
    {
        m_PlotXChoice.AddItem( "Result Order", 0 );
    }
    else
    {
        m_PlotXChoice.AddItem( "Index", 0 );
    }
    int xsel = 0;
    for ( int i = 0; i < ( int ) labels.size(); i++ )
    {
        m_PlotXChoice.AddItem( labels[i], i + 1 );
        if ( labels[i] == curve->m_XLabel )
        {
            xsel = i + 1;
        }
    }
    m_PlotXChoice.UpdateItems();
    m_PlotXChoice.SetVal( xsel );

    //==== Y ====//
    m_PlotYLabels = labels;

    int vpos = m_PlotYBrowser->vposition();
    m_PlotYBrowser->clear();
    for ( int i = 0; i < ( int ) labels.size(); i++ )
    {
        char buf[64];
        if ( lens[i] >= 0 )
        {
            snprintf( buf, sizeof( buf ), "  (%d)", lens[i] );
        }
        else
        {
            snprintf( buf, sizeof( buf ), "  (none)" );
        }
        string line = "@." + labels[i] + buf;
        m_PlotYBrowser->add( line.c_str() );
        if ( std::find( curve->m_YLabels.begin(), curve->m_YLabels.end(), labels[i] ) != curve->m_YLabels.end() )
        {
            m_PlotYBrowser->select( i + 1 );
        }
    }
    m_PlotYBrowser->vposition( vpos );
}

// Whether a point can be drawn on axes of these scales.
static bool PlotablePoint( double x, double y, bool xlog, bool ylog )
{
    if ( !std::isfinite( x ) || !std::isfinite( y ) )
    {
        return false;
    }
    if ( xlog && x <= 0 )
    {
        return false;
    }
    if ( ylog && y <= 0 )
    {
        return false;
    }
    return true;
}

//==== One series drawn on a Cartesian canvas ====//
// Pixels within this of the origin fit the 16-bit coordinates X11 draws with.
static const double PIXEL_LIMIT = 16000.0;

// Clips the segment from p0 to p1 to the square of PIXEL_LIMIT (Liang-Barsky).  Returns whether
// any of it is left, and whether each end was moved.
static bool ClipPixelSegment( double & x0, double & y0, double & x1, double & y1, bool & moved0, bool & moved1 )
{
    double t0 = 0.0;
    double t1 = 1.0;
    double dx = x1 - x0;
    double dy = y1 - y0;
    double p[4] = { -dx, dx, -dy, dy };
    double q[4] = { x0 + PIXEL_LIMIT, PIXEL_LIMIT - x0, y0 + PIXEL_LIMIT, PIXEL_LIMIT - y0 };
    for ( int i = 0; i < 4; i++ )
    {
        if ( p[i] == 0.0 )
        {
            if ( q[i] < 0.0 )
            {
                return false;
            }
        }
        else
        {
            double r = q[i] / p[i];
            if ( p[i] < 0.0 )
            {
                t0 = std::max( t0, r );
            }
            else
            {
                t1 = std::min( t1, r );
            }
        }
    }
    if ( t0 > t1 )
    {
        return false;
    }
    moved0 = t0 > 0.0;
    moved1 = t1 < 1.0;
    double ox = x0;
    double oy = y0;
    x0 = ox + t0 * dx;
    y0 = oy + t0 * dy;
    x1 = ox + t1 * dx;
    y1 = oy + t1 * dy;
    return true;
}

class ResultsSeriesObject : public Ca_Object_
{
public:
    ResultsSeriesObject( const ResultsSeries & series, bool lines, bool points, bool xlog, bool ylog ) : Ca_Object_()
    {
        m_X = series.m_X;
        m_Y = series.m_Y;
        m_Color = series.m_Color;
        m_LineStyle = series.m_LineStyle;
        m_Marker = series.m_Marker;
        m_Lines = lines;
        m_Points = points;
        m_XLog = xlog;
        m_YLog = ylog;
    }

protected:

    virtual void draw()
    {
        int n = ( int ) m_X.size();
        int nvalid = 0;
        for ( int i = 0; i < n; i++ )
        {
            if ( PlotablePoint( m_X[i], m_Y[i], m_XLog, m_YLog ) )
            {
                nvalid++;
            }
        }

        if ( m_Lines )
        {
            // Lines break at points that cannot be drawn, and are clipped far off the plot.
            fl_color( m_Color );
            fl_line_style( m_LineStyle, 2 );
            bool open = false;
            bool have_prev = false;
            double px = 0.0;
            double py = 0.0;
            for ( int i = 0; i < n; i++ )
            {
                if ( !PlotablePoint( m_X[i], m_Y[i], m_XLog, m_YLog ) )
                {
                    if ( open )
                    {
                        fl_end_line();
                        open = false;
                    }
                    have_prev = false;
                    continue;
                }

                double qx = x_axis_->position( m_X[i] );
                double qy = y_axis_->position( m_Y[i] );
                if ( have_prev )
                {
                    double ax = px;
                    double ay = py;
                    double bx = qx;
                    double by = qy;
                    bool moved0 = false;
                    bool moved1 = false;
                    if ( ClipPixelSegment( ax, ay, bx, by, moved0, moved1 ) )
                    {
                        if ( open && moved0 )
                        {
                            fl_end_line();
                            open = false;
                        }
                        if ( !open )
                        {
                            fl_begin_line();
                            fl_vertex( ax, ay );
                            open = true;
                        }
                        fl_vertex( bx, by );
                        if ( moved1 )
                        {
                            fl_end_line();
                            open = false;
                        }
                    }
                    else if ( open )
                    {
                        fl_end_line();
                        open = false;
                    }
                }
                px = qx;
                py = qy;
                have_prev = true;
            }
            if ( open )
            {
                fl_end_line();
            }
            fl_line_style( 0 );
        }

        // A lone point shows even without markers.
        if ( m_Points || nvalid == 1 )
        {
            for ( int i = 0; i < n; i++ )
            {
                if ( PlotablePoint( m_X[i], m_Y[i], m_XLog, m_YLog ) )
                {
                    double qx = x_axis_->position( m_X[i] );
                    double qy = y_axis_->position( m_Y[i] );
                    if ( std::abs( qx ) < PIXEL_LIMIT && std::abs( qy ) < PIXEL_LIMIT )
                    {
                        DrawPlotMarker( m_Marker, ( int ) qx, ( int ) qy, 6, m_Color );
                    }
                }
            }
            fl_line_style( 0 );
        }
    }

    vector < double > m_X;
    vector < double > m_Y;
    Fl_Color m_Color;
    int m_LineStyle;
    int m_Marker;
    bool m_Lines;
    bool m_Points;
    bool m_XLog;
    bool m_YLog;
};

void ResultsViewer::RedrawPlot( ResultsPlot* plot )
{
    bool xlog = false;
    bool ylog = false;
    bool lines = true;
    bool points = true;
    int legend_place = vsp::RESULTS_LEGEND_BEST;

    vector < ResultsSeriesDef > defs;
    vector < string > warnings;

    if ( plot )
    {
        xlog = plot->m_XLog();
        ylog = plot->m_YLog();
        lines = plot->m_ShowLines();
        points = plot->m_ShowPoints();
        legend_place = plot->m_Legend();

        plot->GetSeriesDefs( defs );

        // Curves whose Results are gone, as after the model is read back.
        for ( int i = 0; i < plot->GetNumCurves(); i++ )
        {
            ResultsCurve* curve = plot->GetCurve( i );

            vector < ResultsSeriesDef > cdefs;
            curve->GetSeriesDefs( cdefs );
            if ( cdefs.empty() && !curve->m_Sources.empty() && !curve->m_YLabels.empty() )
            {
                warnings.push_back( "No Results for " + curve->GetText() + "." );
            }
        }
    }

    m_Series.clear();
    for ( int i = 0; i < ( int ) defs.size(); i++ )
    {
        ResultsSeries s;
        if ( BuildResultsSeries( defs[i], s ) )
        {
            m_Series.push_back( s );
        }
        else
        {
            warnings.push_back( "No data for " + defs[i].m_YLabel + "." );
        }
        if ( !s.m_Warning.empty() )
        {
            warnings.push_back( s.m_Warning );
        }
    }
    StyleResultsSeries( m_Series );

    //==== Limits from the data ====//
    double big = std::numeric_limits < double >::max();
    double xmin = big, xmax = -big, ymin = big, ymax = -big;
    int nskip = 0;
    vector < vector < double > > ptx( m_Series.size() );
    vector < vector < double > > pty( m_Series.size() );
    vector < vector < int > > ptindex( m_Series.size() );
    vector < string > names( m_Series.size() );
    for ( int i = 0; i < ( int ) m_Series.size(); i++ )
    {
        names[i] = m_Series[i].m_Name;
        for ( int j = 0; j < ( int ) m_Series[i].m_X.size(); j++ )
        {
            double x = m_Series[i].m_X[j];
            double y = m_Series[i].m_Y[j];
            if ( !PlotablePoint( x, y, xlog, ylog ) )
            {
                nskip++;
                continue;
            }
            ptx[i].push_back( x );
            pty[i].push_back( y );
            ptindex[i].push_back( j );
            xmin = std::min( xmin, x );
            xmax = std::max( xmax, x );
            ymin = std::min( ymin, y );
            ymax = std::max( ymax, y );
        }
    }
    if ( nskip > 0 )
    {
        char buf[256];
        if ( xlog || ylog )
        {
            snprintf( buf, sizeof( buf ), "%d points not finite or not positive on a log axis left out.", nskip );
        }
        else
        {
            snprintf( buf, sizeof( buf ), "%d points not finite left out.", nskip );
        }
        warnings.push_back( buf );
    }

    m_AutoLimits.assign( 4, 0.0 );
    if ( xmin <= xmax )
    {
        NiceAxisLimits( xmin, xmax, xlog, m_AutoLimits[0], m_AutoLimits[1] );
        NiceAxisLimits( ymin, ymax, ylog, m_AutoLimits[2], m_AutoLimits[3] );
    }
    else
    {
        NiceAxisLimits( 1, 1, xlog, m_AutoLimits[0], m_AutoLimits[1] );
        NiceAxisLimits( 1, 1, ylog, m_AutoLimits[2], m_AutoLimits[3] );
    }

    //==== Axis labels ====//
    vector < string > xl, yl;
    for ( int i = 0; i < ( int ) m_Series.size(); i++ )
    {
        const ResultsSeriesDef & def = m_Series[i].m_Def;
        if ( !def.m_XLabel.empty() )
        {
            xl.push_back( def.m_XLabel );
        }
        else if ( def.m_Mode == vsp::RESULTS_PLOT_ACROSS_RESULTS )
        {
            xl.push_back( "Result Order" );
        }
        else
        {
            xl.push_back( "Index" );
        }
        yl.push_back( def.m_YLabel );
    }
    m_PlotCanvas->current_x()->copy_label( PlainLabel( MakeResultsAxisLabel( xl, "" ) ).c_str() );
    m_PlotCanvas->current_y()->copy_label( PlainLabel( MakeResultsAxisLabel( yl, "" ) ).c_str() );

    if ( xlog )
    {
        m_PlotCanvas->current_x()->scale( CA_LOG );
    }
    else
    {
        m_PlotCanvas->current_x()->scale( CA_LIN );
    }
    if ( ylog )
    {
        m_PlotCanvas->current_y()->scale( CA_LOG );
    }
    else
    {
        m_PlotCanvas->current_y()->scale( CA_LIN );
    }

    //==== Series ====//
    Ca_Canvas::current( m_PlotCanvas );
    m_PlotCanvas->clear();

    vector < ResultsLegendEntry > legend;
    for ( int i = 0; i < ( int ) m_Series.size(); i++ )
    {
        new ResultsSeriesObject( m_Series[i], lines, points, xlog, ylog );

        ResultsLegendEntry e;
        e.m_Text = m_Series[i].m_Name;
        e.m_Color = m_Series[i].m_Color;
        e.m_LineStyle = m_Series[i].m_LineStyle;
        e.m_Marker = m_Series[i].m_Marker;
        e.m_ShowLine = lines;
        e.m_ShowPoint = points || ptx[i].size() == 1;
        legend.push_back( e );
    }
    m_PlotCanvas->SetLegend( legend, legend_place );

    // Only drawn series are read out; with neither lines nor points, a series of more than one point is not drawn.
    if ( !lines && !points )
    {
        for ( int i = 0; i < ( int ) ptx.size(); i++ )
        {
            if ( ptx[i].size() > 1 )
            {
                ptx[i].clear();
                pty[i].clear();
                ptindex[i].clear();
            }
        }
    }
    m_PlotCanvas->SetSeriesPoints( ptx, pty, names, ptindex );

    //==== Warnings ====//
    string warn;
    if ( !warnings.empty() )
    {
        warn = warnings[0];
        if ( warnings.size() > 1 )
        {
            char buf[64];
            snprintf( buf, sizeof( buf ), "  (+%d more)", ( int ) warnings.size() - 1 );
            warn += buf;
        }
    }
    else if ( !plot )
    {
        warn = "Add a plot to start.";
    }
    else if ( plot->GetNumCurves() == 0 )
    {
        warn = "Add a curve to start.";
    }
    else if ( m_Series.empty() )
    {
        warn = "Pick Results and Y data to plot.";
    }
    m_WarningBox->copy_label( PlainLabel( warn ).c_str() );

    if ( m_Series.empty() )
    {
        m_ExportPlotCSVButton.Deactivate();
    }
    else
    {
        m_ExportPlotCSVButton.Activate();
    }

    m_PlotGroup->redraw();
}

void ResultsViewer::UpdatePlotAxes( ResultsPlot* plot )
{
    if ( m_AutoLimits.size() != 4 )
    {
        return;
    }

    if ( !plot )
    {
        return;
    }

    ResultsPlot & ps = *plot;
    bool xlog = ps.m_XLog();
    bool ylog = ps.m_YLog();

    // Automatic limits show in the inputs; manual ones come from them.
    ps.ShowAutoLimits( m_AutoLimits[0], m_AutoLimits[1], m_AutoLimits[2], m_AutoLimits[3] );

    double x0 = ps.m_XMin();
    double x1 = ps.m_XMax();
    double y0 = ps.m_YMin();
    double y1 = ps.m_YMax();

    // Unusable limits fall back to the automatic ones.
    if ( !UsableAxisLimits( x0, x1, xlog ) )
    {
        x0 = m_AutoLimits[0];
        x1 = m_AutoLimits[1];
    }
    if ( !UsableAxisLimits( y0, y1, ylog ) )
    {
        y0 = m_AutoLimits[2];
        y1 = m_AutoLimits[3];
    }

    Ca_Axis_* xa = m_PlotCanvas->current_x();
    Ca_Axis_* ya = m_PlotCanvas->current_y();
    if ( xa->minimum() != x0 || xa->maximum() != x1 || ya->minimum() != y0 || ya->maximum() != y1 )
    {
        xa->minimum( x0 );
        xa->maximum( x1 );
        ya->minimum( y0 );
        ya->maximum( y1 );
        m_PlotGroup->redraw();
    }

    m_XMinToggle.Update( ps.m_XMinManual.GetID() );
    m_XMaxToggle.Update( ps.m_XMaxManual.GetID() );
    m_YMinToggle.Update( ps.m_YMinManual.GetID() );
    m_YMaxToggle.Update( ps.m_YMaxManual.GetID() );
    m_XMinInput.Update( ps.m_XMin.GetID() );
    m_XMaxInput.Update( ps.m_XMax.GetID() );
    m_YMinInput.Update( ps.m_YMin.GetID() );
    m_YMaxInput.Update( ps.m_YMax.GetID() );
}

void ResultsViewer::LimitsCallBack( double xmin, double xmax, double ymin, double ymax, bool reset )
{
    ResultsPlot* plot = ResultsPlotMgr.GetCurrPlot();
    if ( !plot )
    {
        return;
    }

    // One notice for all the limits, rather than one screen update per Parm.
    if ( reset )
    {
        plot->SetAutoLimits();
    }
    else
    {
        plot->SetManualLimits( xmin, xmax, ymin, ymax );
    }
    UpdatePlotAxes( plot );
    m_ScreenMgr->SetUpdateFlag( true );
}

void ResultsViewer::ReadoutCallBack( const string & text )
{
    m_ReadoutBox->copy_label( PlainLabel( text ).c_str() );
    m_ReadoutBox->redraw();
}

// A CSV field, quoted when it holds a comma, quote or line break.
static string CSVField( const string & s )
{
    if ( s.find_first_of( ",\"\n" ) == string::npos )
    {
        return s;
    }
    string out = "\"";
    for ( int i = 0; i < ( int ) s.size(); i++ )
    {
        if ( s[i] == '"' )
        {
            out += '"';
        }
        out += s[i];
    }
    return out + "\"";
}

void ResultsViewer::ExportPlotCSV( const string & fname )
{
    if ( m_Series.empty() || fname.empty() )
    {
        return;
    }

    FILE* fp = fl_fopen( fname.c_str(), "w" );
    if ( !fp )
    {
        return;
    }

    // Two columns per series, as long as the longest.
    size_t nrow = 0;
    for ( int i = 0; i < ( int ) m_Series.size(); i++ )
    {
        if ( i > 0 )
        {
            fprintf( fp, "," );
        }
        string xname = m_Series[i].m_Def.m_XLabel;
        if ( xname.empty() && m_Series[i].m_Def.m_Mode == vsp::RESULTS_PLOT_ACROSS_RESULTS )
        {
            xname = "Result Order";
        }
        else if ( xname.empty() )
        {
            xname = "Index";
        }
        fprintf( fp, "%s,%s", CSVField( m_Series[i].m_Name + " " + xname ).c_str(),
                 CSVField( m_Series[i].m_Name + " " + m_Series[i].m_Def.m_YLabel ).c_str() );
        nrow = std::max( nrow, m_Series[i].m_X.size() );
    }
    fprintf( fp, "\n" );

    for ( size_t r = 0; r < nrow; r++ )
    {
        for ( int i = 0; i < ( int ) m_Series.size(); i++ )
        {
            if ( i > 0 )
            {
                fprintf( fp, "," );
            }
            if ( r < m_Series[i].m_X.size() )
            {
                fprintf( fp, "%.17g,%.17g", m_Series[i].m_X[r], m_Series[i].m_Y[r] );
            }
            else
            {
                fprintf( fp, "," );
            }
        }
        fprintf( fp, "\n" );
    }
    fclose( fp );
}

void ResultsViewer::SavePlotImage( string fname )
{
    if ( fname.empty() )
    {
        return;
    }

    string ext;
    size_t dot = fname.rfind( '.' );
    if ( dot != string::npos )
    {
        ext = fname.substr( dot );
        std::transform( ext.begin(), ext.end(), ext.begin(), ::tolower );
    }

    int pw = m_PlotGroup->w();
    int ph = m_PlotGroup->h();

    if ( ext == ".svg" )
    {
        FILE* fp = fl_fopen( fname.c_str(), "w" );
        if ( !fp )
        {
            return;
        }
        Fl_SVG_File_Surface surf( pw, ph, fp );
        Fl_Surface_Device::push_current( &surf );
        fl_color( FL_WHITE );
        fl_rectf( 0, 0, pw, ph );
        surf.draw( m_PlotGroup, 0, 0 );
        Fl_Surface_Device::pop_current();
        surf.close();
    }
    else
    {
        if ( ext != ".png" )
        {
            fname += ".png";
        }
        Fl_Image_Surface surf( pw, ph, 1 );
        Fl_Surface_Device::push_current( &surf );
        fl_color( FL_WHITE );
        fl_rectf( 0, 0, pw, ph );
        surf.draw( m_PlotGroup, 0, 0 );
        Fl_RGB_Image* img = surf.image();
        Fl_Surface_Device::pop_current();
        if ( img )
        {
            fl_write_png( fname.c_str(), img );
            delete img;
        }
    }
}
