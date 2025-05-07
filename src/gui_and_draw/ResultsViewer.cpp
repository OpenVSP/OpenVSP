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
#include "StlHelper.h"

#include <ctime>

ResultsViewer::ResultsViewer( ScreenMgr* mgr ) : BasicScreen( mgr, 800, 700, "Results" )
{
    m_CurrDisplayGroup = NULL;

    m_FLTK_Window->callback( staticCloseCB, this );
    m_MainLayout.SetGroupAndScreen( m_FLTK_Window, this );

    m_MainLayout.ForceNewLine();
    m_MainLayout.AddY( 7 );
    m_MainLayout.AddX( 5 );

    m_MainLayout.AddSubGroupLayout( m_BorderLayout, m_MainLayout.GetRemainX() - 5.0,
                                    m_MainLayout.GetRemainY() - 5.0 );

    m_BorderLayout.AddChoice( m_ResultsChoice, "Result" );

    m_BorderLayout.SetSameLineFlag( false );
    m_BorderLayout.SetFitWidthFlag( true );

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

    m_IntVectorSpreadColumn = m_IntVectorLayout.AddSpreadSheet < int > ( m_IntVectorLayout.GetRemainY() );
    m_IntVectorSpreadColumn->set_AllowPasteGrowCols( false );

    m_BorderLayout.AddSubGroupLayout( m_DoubleVectorLayout, m_BorderLayout.GetW(), m_BorderLayout.GetRemainY() );
    m_DoubleVectorLayout.AddDividerBox( "Vector of Doubles" );

    m_DoubleVectorSpreadColumn = m_DoubleVectorLayout.AddSpreadSheet < double > ( m_DoubleVectorLayout.GetRemainY() );
    m_DoubleVectorSpreadColumn->set_AllowPasteGrowCols( false );

    m_BorderLayout.AddSubGroupLayout( m_StringVectorLayout, m_BorderLayout.GetW(), m_BorderLayout.GetRemainY() );
    m_StringVectorLayout.AddDividerBox( "Vector of Strings" );

    m_StringVectorSpreadColumn = m_StringVectorLayout.AddSpreadSheet < string > ( m_StringVectorLayout.GetRemainY() );
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

    m_Vec3dVectorSpreadColumn = m_Vec3dVectorLayout.AddSpreadSheet < vec3d > ( m_Vec3dVectorLayout.GetRemainY() );
    m_Vec3dVectorSpreadColumn->set_AllowPasteGrowCols( false );
    m_Vec3dVectorSpreadColumn->set_HeaderOffset( 'X' - 'A' ); // start columns at X.

    m_BorderLayout.AddSubGroupLayout( m_DoubleMatrixLayout, m_BorderLayout.GetW(), m_BorderLayout.GetRemainY() );
    m_DoubleMatrixLayout.AddDividerBox( "Matrix of Doubles" );

    m_DoubleMatrixSpreadSheet = m_DoubleMatrixLayout.AddSpreadSheet< vector < double > > ( m_DoubleMatrixLayout.GetRemainY() );



    m_ResultsViewerSelect = -1;
    m_DataBrowserSelect = -1;

    DisplayGroup( &m_IntVectorLayout );
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
    m_FLTK_Window->hide();
    m_ScreenMgr->SetUpdateFlag( true );
}

bool ResultsViewer::Update()
{
    BasicScreen::Update();

    UpdateResultsChoice();

    UpdateDataBrowser();

    DisplayGroup( nullptr );
    if ( m_ResultsViewerSelect >= 0 && m_ResultsViewerSelect < m_ResIDvec.size() && m_ResIDvec.size() > 0 )
    {
        string resid = m_ResIDvec[ m_ResultsViewerSelect ];

        Results *res = ResultsMgr.FindResultsPtr( resid );

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
    return true;
}

void ResultsViewer::UpdateResultsChoice()
{
    char str[512];
    char timestr[512];

    m_ResultsChoice.ClearItems();
    m_ResIDvec.clear();

    vector < string > resvec = ResultsMgr.GetAllResultsIDs();

    vector < pair < time_t, string > > m_TimeIDVec;
    for ( int i = 0 ; i < (int)resvec.size() ; i++ )
    {
        Results * res = ResultsMgr.FindResultsPtr( resvec[i] );
        if ( res )
        {
            time_t r_time = res->GetTimestamp();
            m_TimeIDVec.emplace_back( make_pair( r_time, resvec[ i ] ) );
        }
    }
    sort( m_TimeIDVec.begin(), m_TimeIDVec.end() );

    for ( int i = 0 ; i < (int)m_TimeIDVec.size() ; i++ )
    {
        m_ResIDvec.emplace_back( m_TimeIDVec[i].second );
    }

    for ( int i = 0 ; i < (int)m_ResIDvec.size() ; i++ )
    {
        Results * res = ResultsMgr.FindResultsPtr( m_ResIDvec[i] );

        if ( res )
        {
            string r_name = res->GetName();
            string r_doc = res->GetDoc();
            time_t r_time = res->GetTimestamp();
            struct tm *r_tp = localtime( &r_time );

            strftime( timestr, sizeof( timestr ), "%r %D", r_tp );


            snprintf( str, sizeof( str ), "%s - %s - %s\n", r_name.c_str(), timestr, r_doc.c_str());
            m_ResultsChoice.AddItem( str, i );
        }
    }
    m_ResultsChoice.UpdateItems();

    if ( m_ResultsViewerSelect >= 0 && m_ResultsViewerSelect < (int)m_ResIDvec.size() )
    {
        m_ResultsChoice.SetVal( m_ResultsViewerSelect );
    }
}


void ResultsViewer::UpdateDataBrowser()
{
    char str[512];
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
