//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//
// A check list of the Results a Results plot curve plots when its runs are chosen.
//
//////////////////////////////////////////////////////////////////////

#include "ResultsChooserScreen.h"

#include "ResultsMgr.h"
#include "ResultsPlotMgr.h"
#include "ResultsViewer.h"
#include "ScreenMgr.h"

#include <algorithm>

ResultsChooserScreen::ResultsChooserScreen( ScreenMgr* mgr ) : BasicScreen( mgr, 500, 400, "Choose Results" )
{
    m_MainLayout.SetGroupAndScreen( m_FLTK_Window, this );

    m_MainLayout.ForceNewLine();
    m_MainLayout.AddY( 7 );
    m_MainLayout.AddX( 5 );

    m_MainLayout.AddSubGroupLayout( m_BorderLayout, m_MainLayout.GetRemainX() - 5, m_MainLayout.GetRemainY() - 5 );

    m_CurveBox = m_BorderLayout.AddDividerBox( "" );

    // AddCheckBrowser moves down one row, whatever the browser's height.
    int browse_h = m_BorderLayout.GetRemainY() - m_BorderLayout.GetStdHeight() - m_BorderLayout.GetGapHeight();
    m_ResultsBrowser = m_BorderLayout.AddCheckBrowser( browse_h );
    m_ResultsBrowser->callback( staticScreenCB, this );
    m_BorderLayout.AddY( browse_h - m_BorderLayout.GetStdHeight() );

    m_BorderLayout.AddYGap();

    m_BorderLayout.SetSameLineFlag( true );
    m_BorderLayout.SetFitWidthFlag( false );
    m_BorderLayout.SetButtonWidth( m_BorderLayout.GetW() / 2 );
    m_BorderLayout.AddButton( m_CheckAllButton, "Check All" );
    m_BorderLayout.AddButton( m_CheckNoneButton, "Check None" );
    m_BorderLayout.ForceNewLine();

    m_FLTK_Window->resizable( m_ResultsBrowser );
}

ResultsChooserScreen::~ResultsChooserScreen()
{
}

void ResultsChooserScreen::Show()
{
    m_ScreenMgr->SetUpdateFlag( true );
    BasicScreen::Show();
}

ResultsCurve* ResultsChooserScreen::GetChoosingCurve()
{
    ResultsPlot* plot = ResultsPlotMgr.GetCurrPlot();
    if ( !plot )
    {
        return nullptr;
    }

    ResultsCurve* curve = plot->GetEditCurve();
    if ( !curve || curve->m_Run() != vsp::RESULTS_RUN_CHOSEN )
    {
        return nullptr;
    }
    return curve;
}

bool ResultsChooserScreen::Update()
{
    BasicScreen::Update();

    int vpos = m_ResultsBrowser->vposition();
    m_ResultsBrowser->clear();
    m_ListedIDs.clear();

    ResultsCurve* curve = GetChoosingCurve();
    if ( !curve )
    {
        m_CurveBox->copy_label( "The curve being edited does not plot chosen Results" );
        m_ResultsBrowser->deactivate();
        m_CheckAllButton.Deactivate();
        m_CheckNoneButton.Deactivate();
        m_FLTK_Window->redraw();
        return false;
    }

    m_CurveBox->copy_label( curve->GetText().c_str() );
    m_ResultsBrowser->activate();
    m_CheckAllButton.Activate();
    m_CheckNoneButton.Activate();

    ListResultsOfSources( curve->m_Sources, m_ListedIDs );

    for ( int i = 0; i < ( int ) m_ListedIDs.size(); i++ )
    {
        bool checked = std::find( curve->m_UncheckedIDs.begin(), curve->m_UncheckedIDs.end(), m_ListedIDs[i] ) == curve->m_UncheckedIDs.end();
        m_ResultsBrowser->add( GetResultsListText( ResultsMgr.FindResultsPtr( m_ListedIDs[i] ) ).c_str(), checked );
    }
    m_ResultsBrowser->vposition( vpos );

    m_FLTK_Window->redraw();
    return true;
}

void ResultsChooserScreen::CallBack( Fl_Widget *w )
{
    ResultsCurve* curve = GetChoosingCurve();

    if ( w == m_ResultsBrowser && curve )
    {
        // A drag can toggle several lines, so every line is read.
        for ( int i = 0; i < ( int ) m_ListedIDs.size() && i < m_ResultsBrowser->nitems(); i++ )
        {
            const string & id = m_ListedIDs[i];
            curve->m_UncheckedIDs.erase( std::remove( curve->m_UncheckedIDs.begin(), curve->m_UncheckedIDs.end(), id ), curve->m_UncheckedIDs.end() );
            if ( !m_ResultsBrowser->checked( i + 1 ) )
            {
                curve->m_UncheckedIDs.push_back( id );
            }
        }
    }

    m_ScreenMgr->SetUpdateFlag( true );
}

void ResultsChooserScreen::GuiDeviceCallBack( GuiDevice* device )
{
    ResultsCurve* curve = GetChoosingCurve();

    if ( curve && ( device == &m_CheckAllButton || device == &m_CheckNoneButton ) )
    {
        // Results of other kinds the curve left out before stay left out.
        for ( int i = 0; i < ( int ) m_ListedIDs.size(); i++ )
        {
            curve->m_UncheckedIDs.erase( std::remove( curve->m_UncheckedIDs.begin(), curve->m_UncheckedIDs.end(), m_ListedIDs[i] ), curve->m_UncheckedIDs.end() );
        }

        if ( device == &m_CheckNoneButton )
        {
            curve->m_UncheckedIDs.insert( curve->m_UncheckedIDs.end(), m_ListedIDs.begin(), m_ListedIDs.end() );
        }
    }

    m_ScreenMgr->SetUpdateFlag( true );
}
