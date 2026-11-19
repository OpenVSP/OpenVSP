//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//
// XY plots of Results data for the Results viewer.
//
//////////////////////////////////////////////////////////////////////

#include "ResultsPlotCanvas.h"
#include "GroupLayout.h"

#include "ResultsMgr.h"
#include "ScreenMgr.h"

#include <FL/fl_draw.H>

#include <algorithm>
#include <cmath>
#include <ctime>
#include <limits>
#include <set>

ResultsSeries::ResultsSeries()
{
    m_Color = FL_BLACK;
    m_LineStyle = FL_SOLID;
    m_Marker = CA_ROUND;
}

static bool PickElement( const vector < double > & d, int element, double & val )
{
    if ( d.empty() )
    {
        return false;
    }
    if ( element == vsp::RESULTS_PLOT_LAST )
    {
        val = d.back();
    }
    else
    {
        val = d.front();
    }
    return true;
}

bool BuildResultsSeries( const ResultsSeriesDef & def, ResultsSeries & series )
{
    series.m_Def = def;
    series.m_X.clear();
    series.m_Y.clear();
    series.m_Warning.clear();

    if ( def.m_Mode == vsp::RESULTS_PLOT_ACROSS_RESULTS )
    {
        int nmissing = 0;
        for ( int i = 0; i < ( int ) def.m_ResultIDs.size(); i++ )
        {
            Results* res = ResultsMgr.FindResultsPtr( def.m_ResultIDs[i] );
            vector < double > yd;
            double x, y;
            if ( !res || !GetResultsPlotVarData( res, def.m_YLabel, def.m_MatrixRows, yd ) || !PickElement( yd, def.m_Element, y ) )
            {
                nmissing++;
                continue;
            }

            if ( def.m_XLabel.empty() )
            {
                x = i;
            }
            else
            {
                vector < double > xd;
                if ( !GetResultsPlotVarData( res, def.m_XLabel, def.m_MatrixRows, xd ) || !PickElement( xd, def.m_Element, x ) )
                {
                    nmissing++;
                    continue;
                }
            }
            series.m_X.push_back( x );
            series.m_Y.push_back( y );
        }

        if ( nmissing > 0 )
        {
            char buf[256];
            snprintf( buf, sizeof( buf ), "%s: %d Result(s) lack the data.", def.m_YLabel.c_str(), nmissing );
            series.m_Warning = buf;
        }
        return !series.m_X.empty();
    }

    if ( def.m_ResultIDs.empty() )
    {
        return false;
    }

    Results* res = ResultsMgr.FindResultsPtr( def.m_ResultIDs[0] );
    if ( !res || !GetResultsPlotVarData( res, def.m_YLabel, def.m_MatrixRows, series.m_Y ) )
    {
        return false;
    }

    if ( def.m_XLabel.empty() )
    {
        series.m_X.resize( series.m_Y.size() );
        for ( int i = 0; i < ( int ) series.m_X.size(); i++ )
        {
            series.m_X[i] = i;
        }
    }
    else if ( !GetResultsPlotVarData( res, def.m_XLabel, def.m_MatrixRows, series.m_X ) )
    {
        return false;
    }

    if ( series.m_X.size() != series.m_Y.size() )
    {
        string xname = def.m_XLabel;
        if ( xname.empty() )
        {
            xname = "index";
        }
        char buf[512];
        snprintf( buf, sizeof( buf ), "%s: %d values against %d of %s; plotted %d.", def.m_YLabel.c_str(),
                  ( int ) series.m_Y.size(), ( int ) series.m_X.size(), xname.c_str(),
                  ( int ) std::min( series.m_X.size(), series.m_Y.size() ) );
        series.m_Warning = buf;

        size_t n = std::min( series.m_X.size(), series.m_Y.size() );
        series.m_X.resize( n );
        series.m_Y.resize( n );
    }
    return !series.m_X.empty();
}

// Name for a series' Results; repeated names get their times to tell them apart.
static string SourceName( const ResultsSeriesDef & def )
{
    if ( def.m_Mode == vsp::RESULTS_PLOT_ACROSS_RESULTS )
    {
        std::set < string > names;
        for ( int i = 0; i < ( int ) def.m_ResultIDs.size(); i++ )
        {
            names.insert( GetResultsDisplayName( ResultsMgr.FindResultsPtr( def.m_ResultIDs[i] ) ) );
        }
        string s;
        for ( std::set < string >::iterator it = names.begin(); it != names.end(); ++it )
        {
            if ( !s.empty() )
            {
                s += ", ";
            }
            s += *it;
        }
        char buf[64];
        snprintf( buf, sizeof( buf ), " (%d)", ( int ) def.m_ResultIDs.size() );
        return s + buf;
    }

    if ( def.m_ResultIDs.empty() )
    {
        return string();
    }
    return GetResultsDisplayName( ResultsMgr.FindResultsPtr( def.m_ResultIDs[0] ) );
}

static string SourceTime( const ResultsSeriesDef & def )
{
    if ( def.m_ResultIDs.empty() )
    {
        return string();
    }
    Results* res = ResultsMgr.FindResultsPtr( def.m_ResultIDs[0] );
    if ( !res )
    {
        return string();
    }
    char timestr[64];
    time_t r_time = res->GetTimestamp();
    struct tm *r_tp = localtime( &r_time );
    strftime( timestr, sizeof( timestr ), "%H:%M:%S", r_tp );
    return string( timestr );
}

static string SourceKey( const ResultsSeriesDef & def )
{
    string key;
    for ( int i = 0; i < ( int ) def.m_ResultIDs.size(); i++ )
    {
        key += def.m_ResultIDs[i] + ";";
    }
    return key;
}

static int LevelIndex( vector < string > & levels, const string & key )
{
    for ( int i = 0; i < ( int ) levels.size(); i++ )
    {
        if ( levels[i] == key )
        {
            return i;
        }
    }
    levels.push_back( key );
    return ( int ) levels.size() - 1;
}

void StyleResultsSeries( vector < ResultsSeries > & series )
{
    vector < string > sources;
    vector < string > yvars;
    vector < string > xvars;
    vector < int > isrc( series.size() );
    vector < int > iy( series.size() );

    for ( int i = 0; i < ( int ) series.size(); i++ )
    {
        isrc[i] = LevelIndex( sources, SourceKey( series[i].m_Def ) );
        iy[i] = LevelIndex( yvars, series[i].m_Def.m_YLabel );
        LevelIndex( xvars, series[i].m_Def.m_XLabel );
    }

    // Color tells apart whichever factor varies; with both varying, color goes to the
    // variable and dashes and markers to the Results.
    bool many_src = sources.size() > 1;
    bool many_y = yvars.size() > 1;

    // Results whose names repeat are told apart by their times.
    vector < string > src_names( sources.size() );
    vector < bool > src_done( sources.size(), false );
    for ( int i = 0; i < ( int ) series.size(); i++ )
    {
        if ( !src_done[ isrc[i] ] )
        {
            src_names[ isrc[i] ] = SourceName( series[i].m_Def );
            src_done[ isrc[i] ] = true;
        }
    }
    vector < string > src_full( src_names );
    for ( int i = 0; i < ( int ) series.size(); i++ )
    {
        int s = isrc[i];
        int nsame = 0;
        for ( int j = 0; j < ( int ) src_names.size(); j++ )
        {
            if ( src_names[j] == src_names[s] )
            {
                nsame++;
            }
        }
        if ( nsame > 1 && series[i].m_Def.m_Mode == vsp::RESULTS_PLOT_VECTORS )
        {
            src_full[s] = src_names[s] + " " + SourceTime( series[i].m_Def );
        }
    }

    // Results from the same second are numbered in order.
    for ( int s = 0; s < ( int ) src_full.size(); s++ )
    {
        int nbefore = 0;
        int nsame = 0;
        for ( int j = 0; j < ( int ) src_full.size(); j++ )
        {
            if ( src_full[j] == src_full[s] )
            {
                nsame++;
                if ( j < s )
                {
                    nbefore++;
                }
            }
        }
        if ( nsame > 1 )
        {
            char buf[32];
            snprintf( buf, sizeof( buf ), " #%d", nbefore + 1 );
            src_names[s] = src_full[s] + buf;
        }
        else
        {
            src_names[s] = src_full[s];
        }
    }
    src_full = src_names;

    for ( int i = 0; i < ( int ) series.size(); i++ )
    {
        ResultsSeries & s = series[i];

        if ( many_y && many_src )
        {
            SourceSeriesStyle( iy[i], isrc[i], ( int ) src_full.size(), s.m_Color, s.m_LineStyle, s.m_Marker );
        }
        else
        {
            // One factor: series are styled in order, as in other plots.
            int iseries = i;
            if ( many_y )
            {
                iseries = iy[i];
            }
            else if ( many_src )
            {
                iseries = isrc[i];
            }
            s.m_Color = SeriesColor( iseries );
            s.m_LineStyle = SeriesLineStyle( iseries );
            s.m_Marker = SeriesMarker( iseries );
        }

        s.m_Source = src_full[ isrc[i] ];

        // The legend names only what tells series apart.
        string name;
        if ( many_src || ( !many_y && xvars.size() <= 1 ) )
        {
            name = s.m_Source;
        }
        if ( many_y || ( !many_src && xvars.size() <= 1 ) )
        {
            if ( !name.empty() )
            {
                name += ": ";
            }
            name += s.m_Def.m_YLabel;
        }
        if ( xvars.size() > 1 )
        {
            if ( !name.empty() )
            {
                name += " ";
            }
            if ( s.m_Def.m_XLabel.empty() && s.m_Def.m_Mode == vsp::RESULTS_PLOT_ACROSS_RESULTS )
            {
                name += "vs Result Order";
            }
            else if ( s.m_Def.m_XLabel.empty() )
            {
                name += "vs index";
            }
            else
            {
                name += "vs " + s.m_Def.m_XLabel;
            }
        }
        s.m_Name = name;
    }
}

string MakeResultsAxisLabel( const vector < string > & labels, const string & empty_label )
{
    vector < string > uniq;
    for ( int i = 0; i < ( int ) labels.size(); i++ )
    {
        string l = labels[i];
        if ( l.empty() )
        {
            l = empty_label;
        }
        if ( std::find( uniq.begin(), uniq.end(), l ) == uniq.end() )
        {
            uniq.push_back( l );
        }
    }

    string s;
    for ( int i = 0; i < ( int ) uniq.size(); i++ )
    {
        if ( i == 3 )
        {
            s += ", ...";
            break;
        }
        if ( i > 0 )
        {
            s += ", ";
        }
        s += uniq[i];
    }
    return s;
}

// Heckbert's nice number: 1, 2, 5 or 10 times a power of ten.
static double NiceNum( double x, bool round )
{
    double e = floor( log10( x ) );
    double f = x / pow( 10.0, e );
    double nf;
    if ( round )
    {
        if ( f < 1.5 )
        {
            nf = 1;
        }
        else if ( f < 3 )
        {
            nf = 2;
        }
        else if ( f < 7 )
        {
            nf = 5;
        }
        else
        {
            nf = 10;
        }
    }
    else
    {
        if ( f <= 1 )
        {
            nf = 1;
        }
        else if ( f <= 2 )
        {
            nf = 2;
        }
        else if ( f <= 5 )
        {
            nf = 5;
        }
        else
        {
            nf = 10;
        }
    }
    return nf * pow( 10.0, e );
}

void NiceAxisLimits( double dmin, double dmax, bool log_axis, double & amin, double & amax )
{
    if ( log_axis )
    {
        if ( !( dmin > 0 ) || !( dmax > 0 ) )
        {
            amin = 0.1;
            amax = 10;
            return;
        }
        amin = pow( 10.0, floor( log10( dmin ) ) );
        amax = pow( 10.0, ceil( log10( dmax ) ) );
        if ( !std::isfinite( amax ) )
        {
            amax = std::numeric_limits < double >::max();
        }
        if ( amax <= amin )
        {
            amax = amin * 10.0;
        }
        return;
    }

    if ( !std::isfinite( dmin ) || !std::isfinite( dmax ) )
    {
        amin = 0;
        amax = 1;
        return;
    }

    if ( !UsableAxisLimits( dmin, dmax, false ) )
    {
        double pad = 0.1 * fabs( dmin );
        if ( pad < 1e-200 )
        {
            pad = 1;
        }
        dmin -= pad;
        dmax += pad;
    }

    double range = NiceNum( dmax - dmin, false );
    double step = NiceNum( range / 8.0, true );
    // Data a rounding error past a tick stays inside it.
    double tol = 1e-6;
    amin = floor( dmin / step + tol ) * step;
    amax = ceil( dmax / step - tol ) * step;

    // Ranges near the largest double round past it.
    if ( !std::isfinite( amin ) || !std::isfinite( amax ) )
    {
        amin = dmin;
        amax = dmax;
    }
}

bool UsableAxisLimits( double amin, double amax, bool log_axis )
{
    if ( !std::isfinite( amin ) || !std::isfinite( amax ) || !( amin < amax ) )
    {
        return false;
    }
    if ( log_axis && amin <= 0 )
    {
        return false;
    }
    // Cartesian's tick loop stops advancing when the span is near round-off of the values.
    double scale = std::max( std::max( fabs( amin ), fabs( amax ) ), 1e-200 );
    return amax - amin > 1e-12 * scale;
}

void AddLegendPlacementItems( Choice & choice )
{
    choice.AddItem( "Best", vsp::RESULTS_LEGEND_BEST );
    choice.AddItem( "Upper Right", vsp::RESULTS_LEGEND_UPPER_RIGHT );
    choice.AddItem( "Upper Left", vsp::RESULTS_LEGEND_UPPER_LEFT );
    choice.AddItem( "Lower Left", vsp::RESULTS_LEGEND_LOWER_LEFT );
    choice.AddItem( "Lower Right", vsp::RESULTS_LEGEND_LOWER_RIGHT );
    choice.AddItem( "Off", vsp::RESULTS_LEGEND_OFF );
}

int ResultsPixel( double p )
{
    if ( !( p > -1e6 ) )
    {
        return -1000000;
    }
    if ( p > 1e6 )
    {
        return 1000000;
    }
    return ( int ) p;
}

//==== Canvas ====//
ResultsPlotCanvas::ResultsPlotCanvas( int x, int y, int w, int h ) : Vsp_Canvas( x, y, w, h )
{
    m_LegendPlacement = vsp::RESULTS_LEGEND_BEST;
    m_Dragging = false;
    m_Panning = false;
    m_PushButton = 0;
    m_PushX = m_PushY = m_DragX = m_DragY = 0;
    m_PanXMin = m_PanXMax = m_PanYMin = m_PanYMax = 0;
    m_PanKx = m_PanKy = 0;
    m_HighSeries = -1;
    m_HighIndex = -1;
    m_PtGen = 0;
    m_BestCorner = vsp::RESULTS_LEGEND_UPPER_RIGHT;
    m_LimitsCB = nullptr;
    m_LimitsData = nullptr;
    m_ReadoutCB = nullptr;
    m_ReadoutData = nullptr;

    m_PressCB = nullptr;
    m_PressData = nullptr;
    m_PressTaken = false;
    m_NumNamed = 0;
}

void ResultsPlotCanvas::SetLegend( const vector < ResultsLegendEntry > & entries, int placement )
{
    m_Legend = entries;
    m_LegendPlacement = placement;
    redraw();
}

void ResultsPlotCanvas::ClearLegend()
{
    m_Legend.clear();
    redraw();
}

void ResultsPlotCanvas::AddLegendEntry( const string & text, Fl_Color c, bool show_line, int linestyle, int marker )
{
    ResultsLegendEntry e;
    e.m_Text = text;
    e.m_Color = c;
    e.m_LineStyle = linestyle;
    e.m_Marker = marker;
    e.m_ShowLine = show_line;
    e.m_ShowPoint = marker != CA_NO_POINT;
    m_Legend.push_back( e );
    redraw();
}

void ResultsPlotCanvas::SetSeriesPoints( const vector < vector < double > > & x, const vector < vector < double > > & y, const vector < string > & names,
                                         const vector < vector < int > > & index )
{
    m_PtX = x;
    m_PtY = y;
    m_PtNames = names;
    m_PtIndex = index;
    m_PtGen++;
    m_HighSeries = -1;
    m_HighIndex = -1;
}

void ResultsPlotCanvas::ClearSeriesPoints()
{
    m_PtX.clear();
    m_PtY.clear();
    m_PtNames.clear();
    m_PtIndex.clear();
    m_NumNamed = 0;
    m_PtGen++;
    m_HighSeries = -1;
    m_HighIndex = -1;
}

void ResultsPlotCanvas::AddSeriesPoints( const vector < double > & x, const vector < double > & y )
{
    size_t n = std::min( x.size(), y.size() );
    m_PtX.push_back( vector < double > ( x.begin(), x.begin() + n ) );
    m_PtY.push_back( vector < double > ( y.begin(), y.begin() + n ) );
    m_PtNames.push_back( string() );
    m_PtGen++;
}

void ResultsPlotCanvas::NameNewSeries( const string & name )
{
    for ( int i = m_NumNamed; i < ( int ) m_PtNames.size(); i++ )
    {
        m_PtNames[i] = name;
    }
    m_NumNamed = ( int ) m_PtNames.size();
}

void ResultsPlotCanvas::draw()
{
    // Always draw everything: the overlays are not Cartesian objects.
    damage( FL_DAMAGE_ALL );
    Ca_Canvas::draw();

    fl_push_clip( x(), y(), w(), h() );

    // Highlighted point under the mouse.
    if ( m_HighSeries >= 0 && m_HighSeries < ( int ) m_PtX.size() && m_HighIndex >= 0 &&
         m_HighIndex < ( int ) m_PtX[ m_HighSeries ].size() && current_x() && current_y() )
    {
        int px = ResultsPixel( current_x()->position( m_PtX[ m_HighSeries ][ m_HighIndex ] ) );
        int py = ResultsPixel( current_y()->position( m_PtY[ m_HighSeries ][ m_HighIndex ] ) );
        fl_color( FL_BLACK );
        fl_line_style( FL_SOLID, 2 );
        fl_arc( px - 6, py - 6, 13, 13, 0, 360 );
        fl_line_style( 0 );
    }

    DrawLegend();
    DrawReadout();

    // Zoom box.
    if ( m_Dragging )
    {
        int x0 = std::min( m_PushX, m_DragX );
        int y0 = std::min( m_PushY, m_DragY );
        int dw = abs( m_DragX - m_PushX );
        int dh = abs( m_DragY - m_PushY );
        fl_color( FL_BLACK );
        fl_line_style( FL_DASH, 1 );
        fl_rect( x0, y0, dw + 1, dh + 1 );
        fl_line_style( 0 );
    }

    fl_pop_clip();
}

void ResultsPlotCanvas::LegendSize( int & lw, int & lh )
{
    fl_font( FL_HELVETICA, 12 );
    int tw = 0;
    for ( int i = 0; i < ( int ) m_Legend.size(); i++ )
    {
        tw = std::max( tw, ( int ) fl_width( m_Legend[i].m_Text.c_str() ) );
    }
    int row = 16;
    lw = 8 + 30 + 6 + tw + 8;
    lh = 6 + row * ( int ) m_Legend.size() + 4;
}

void ResultsPlotCanvas::LegendCorner( int corner, int lw, int lh, int & lx, int & ly )
{
    int pad = 10;
    int left = x() + pad;
    int top = y() + pad;
    // A legend bigger than the plot starts at its top left corner.
    int right = std::max( left, x() + w() - lw - pad );
    int bottom = std::max( top, y() + h() - lh - pad );

    switch ( corner )
    {
        case vsp::RESULTS_LEGEND_UPPER_LEFT:
            lx = left;
            ly = top;
            break;
        case vsp::RESULTS_LEGEND_LOWER_LEFT:
            lx = left;
            ly = bottom;
            break;
        case vsp::RESULTS_LEGEND_LOWER_RIGHT:
            lx = right;
            ly = bottom;
            break;
        default:
            lx = right;
            ly = top;
            break;
    }
}

// Points and samples along the lines between them that fall in a box.
int ResultsPlotCanvas::CountPointsIn( int x0, int y0, int bw, int bh )
{
    if ( !current_x() || !current_y() )
    {
        return 0;
    }

    int count = 0;
    for ( int i = 0; i < ( int ) m_PtX.size(); i++ )
    {
        double lastx = 0;
        double lasty = 0;
        for ( int j = 0; j < ( int ) m_PtX[i].size(); j++ )
        {
            double px = current_x()->position( m_PtX[i][j] );
            double py = current_y()->position( m_PtY[i][j] );

            // A segment clear of the box adds nothing.
            if ( j > 0 && ( std::max( px, lastx ) < x0 || std::min( px, lastx ) > x0 + bw ||
                            std::max( py, lasty ) < y0 || std::min( py, lasty ) > y0 + bh ) )
            {
                lastx = px;
                lasty = py;
                continue;
            }

            int nsample = 1;
            if ( j > 0 )
            {
                nsample = 1 + ( int ) ( std::min( 1000.0, sqrt( ( px - lastx ) * ( px - lastx ) + ( py - lasty ) * ( py - lasty ) ) ) / 4.0 );
            }
            for ( int k = 0; k < nsample; k++ )
            {
                double t = ( k + 1.0 ) / nsample;
                double sx = px;
                double sy = py;
                if ( j > 0 )
                {
                    sx = lastx + t * ( px - lastx );
                    sy = lasty + t * ( py - lasty );
                }
                if ( sx >= x0 && sx <= x0 + bw && sy >= y0 && sy <= y0 + bh )
                {
                    count++;
                }
            }
            lastx = px;
            lasty = py;
        }
    }
    return count;
}

void ResultsPlotCanvas::DrawLegend()
{
    if ( m_Legend.empty() || m_LegendPlacement == vsp::RESULTS_LEGEND_OFF )
    {
        return;
    }

    int lw, lh;
    LegendSize( lw, lh );

    int lx, ly;
    int corner = m_LegendPlacement;
    if ( corner == vsp::RESULTS_LEGEND_BEST && current_x() && current_y() )
    {
        // Found again only when the limits, size, legend or points change.
        double key_vals[] = { current_x()->minimum(), current_x()->maximum(), current_y()->minimum(), current_y()->maximum(),
                              ( double ) x(), ( double ) y(), ( double ) w(), ( double ) h(), ( double ) lw, ( double ) lh, ( double ) m_PtGen };
        vector < double > key( key_vals, key_vals + sizeof( key_vals ) / sizeof( key_vals[0] ) );

        if ( key != m_BestKey )
        {
            // The corner that hides the fewest points; ties keep the earlier corner.
            int corners[] = { vsp::RESULTS_LEGEND_UPPER_RIGHT, vsp::RESULTS_LEGEND_UPPER_LEFT, vsp::RESULTS_LEGEND_LOWER_LEFT, vsp::RESULTS_LEGEND_LOWER_RIGHT };
            int best = -1;
            for ( int i = 0; i < 4; i++ )
            {
                int cx, cy;
                LegendCorner( corners[i], lw, lh, cx, cy );
                int n = CountPointsIn( cx, cy, lw, lh );
                if ( best < 0 || n < best )
                {
                    best = n;
                    m_BestCorner = corners[i];
                }
            }
            m_BestKey = key;
        }
        corner = m_BestCorner;
    }
    LegendCorner( corner, lw, lh, lx, ly );

    fl_color( FL_WHITE );
    fl_rectf( lx, ly, lw, lh );
    fl_color( fl_gray_ramp( 12 ) );
    fl_rect( lx, ly, lw, lh );

    fl_font( FL_HELVETICA, 12 );
    int row = 16;
    for ( int i = 0; i < ( int ) m_Legend.size(); i++ )
    {
        const ResultsLegendEntry & e = m_Legend[i];
        int cy = ly + 6 + row * i + row / 2;
        int sx = lx + 8;

        if ( e.m_ShowLine )
        {
            fl_color( e.m_Color );
            fl_line_style( e.m_LineStyle, 2 );
            fl_line( sx, cy, sx + 30, cy );
            fl_line_style( 0 );
        }
        if ( e.m_ShowPoint )
        {
            DrawPlotMarker( e.m_Marker, sx + 15, cy, 6, e.m_Color );
            fl_line_style( 0 );
        }

        fl_color( FL_BLACK );
        fl_draw( e.m_Text.c_str(), sx + 36, cy + fl_height() / 2 - fl_descent() );
    }
}

bool ResultsPlotCanvas::AxisValue( Ca_Axis_* axis, int pix, double & val )
{
    if ( !axis )
    {
        return false;
    }
    val = axis->value( pix );
    return std::isfinite( val );
}

void ResultsPlotCanvas::UpdateReadout( int mx, int my )
{
    int old_series = m_HighSeries;
    int old_index = m_HighIndex;
    m_HighSeries = -1;
    m_HighIndex = -1;

    if ( !current_x() || !current_y() )
    {
        return;
    }

    // Nearest point within a few pixels.
    double best = 8.0 * 8.0;
    for ( int i = 0; i < ( int ) m_PtX.size(); i++ )
    {
        for ( int j = 0; j < ( int ) m_PtX[i].size(); j++ )
        {
            double dx = current_x()->position( m_PtX[i][j] ) - mx;
            double dy = current_y()->position( m_PtY[i][j] ) - my;
            double d2 = dx * dx + dy * dy;
            if ( d2 < best )
            {
                best = d2;
                m_HighSeries = i;
                m_HighIndex = j;
            }
        }
    }

    char buf[1024];
    if ( m_HighSeries >= 0 )
    {
        string name;
        if ( m_HighSeries < ( int ) m_PtNames.size() )
        {
            name = m_PtNames[ m_HighSeries ];
        }
        int index = m_HighIndex;
        if ( m_HighSeries < ( int ) m_PtIndex.size() && m_HighIndex < ( int ) m_PtIndex[ m_HighSeries ].size() )
        {
            index = m_PtIndex[ m_HighSeries ][ m_HighIndex ];
        }
        snprintf( buf, sizeof( buf ), "%s  [%d]  x = %.8g  y = %.8g", name.c_str(), index,
                  m_PtX[ m_HighSeries ][ m_HighIndex ], m_PtY[ m_HighSeries ][ m_HighIndex ] );
    }
    else
    {
        double vx, vy;
        if ( AxisValue( current_x(), mx, vx ) && AxisValue( current_y(), my, vy ) )
        {
            snprintf( buf, sizeof( buf ), "x = %.6g  y = %.6g", vx, vy );
        }
        else
        {
            buf[0] = '\0';
        }
    }

    SetReadout( string( buf ) );

    if ( old_series != m_HighSeries || old_index != m_HighIndex )
    {
        redraw();
    }
}

void ResultsPlotCanvas::ZoomAbout( int mx, int my, double factor )
{
    Ca_Axis_* xa = current_x();
    Ca_Axis_* ya = current_y();
    if ( !xa || !ya || !m_LimitsCB )
    {
        return;
    }

    // Zoom in pixel space so logarithmic axes zoom evenly.
    double x0 = xa->value( mx + ( xa->position( xa->minimum() ) - mx ) * factor );
    double x1 = xa->value( mx + ( xa->position( xa->maximum() ) - mx ) * factor );
    double y0 = ya->value( my + ( ya->position( ya->minimum() ) - my ) * factor );
    double y1 = ya->value( my + ( ya->position( ya->maximum() ) - my ) * factor );

    SendLimits( x0, x1, y0, y1 );
}

// Report new limits, in either order, when they can be drawn.
void ResultsPlotCanvas::SendLimits( double x0, double x1, double y0, double y1 )
{
    if ( !m_LimitsCB || !current_x() || !current_y() )
    {
        return;
    }

    double xmin = std::min( x0, x1 );
    double xmax = std::max( x0, x1 );
    double ymin = std::min( y0, y1 );
    double ymax = std::max( y0, y1 );

    if ( !UsableAxisLimits( xmin, xmax, current_x()->scale() & CA_LOG ) || !UsableAxisLimits( ymin, ymax, current_y()->scale() & CA_LOG ) )
    {
        return;
    }

    // The highlighted point moves with the limits; the next mouse move finds it again.
    ClearHighlight();
    m_LimitsCB( m_LimitsData, xmin, xmax, ymin, ymax, false );
}

void ResultsPlotCanvas::ClearHighlight()
{
    m_HighSeries = -1;
    m_HighIndex = -1;
    SetReadout( string() );
    redraw();
}

void ResultsPlotCanvas::SetReadout( const string & text )
{
    if ( m_ReadoutCB )
    {
        m_ReadoutCB( m_ReadoutData, text );
    }
    else if ( text != m_Readout )
    {
        m_Readout = text;
        redraw();
    }
}

// The readout in the canvas's top right corner, when no callback shows it.
void ResultsPlotCanvas::DrawReadout()
{
    if ( m_ReadoutCB || m_Readout.empty() )
    {
        return;
    }
    fl_font( FL_HELVETICA, 11 );
    int tw = ( int ) fl_width( m_Readout.c_str() );
    int th = fl_height();
    int bx = std::max( x() + 2, x() + w() - tw - 10 );
    int by = y() + 2;
    fl_color( FL_WHITE );
    fl_rectf( bx, by, tw + 8, th + 4 );
    fl_color( fl_gray_ramp( 12 ) );
    fl_rect( bx, by, tw + 8, th + 4 );
    fl_color( FL_BLACK );
    fl_draw( m_Readout.c_str(), bx + 4, by + 2 + th - fl_descent() );
}

// Pixels per unit, or per unit of log, at the limits the axis was last drawn with.
double ResultsPlotCanvas::PanScale( Ca_Axis_* axis, double amin, double amax )
{
    double span = axis->position( amax ) - axis->position( amin );
    if ( axis->scale() & CA_LOG )
    {
        return span / ( log( amax ) - log( amin ) );
    }
    return span / ( amax - amin );
}

// Move limits by a mouse movement of dpix pixels at scale k.
void ResultsPlotCanvas::PanShift( Ca_Axis_* axis, double k, int dpix, double & amin, double & amax )
{
    if ( !std::isfinite( k ) || k == 0 )
    {
        return;
    }
    double d = -dpix / k;
    if ( axis->scale() & CA_LOG )
    {
        amin = exp( log( amin ) + d );
        amax = exp( log( amax ) + d );
    }
    else
    {
        amin += d;
        amax += d;
    }
}

int ResultsPlotCanvas::handle( int event )
{
    switch ( event )
    {
        case FL_ENTER:
            return 1;
        case FL_LEAVE:
            ClearHighlight();
            return 1;
        case FL_MOVE:
            UpdateReadout( Fl::event_x(), Fl::event_y() );
            return 1;
        case FL_PUSH:
            // A second button during a drag is ignored.
            if ( m_Dragging || m_Panning || m_PressTaken )
            {
                return 1;
            }
            m_PushButton = Fl::event_button();
            if ( m_PressCB && m_PressCB( m_PressData, FL_PUSH, Fl::event_x(), Fl::event_y() ) )
            {
                m_PressTaken = true;
                return 1;
            }
            if ( Fl::event_clicks() > 0 && Fl::event_button() == FL_LEFT_MOUSE )
            {
                // Double click: back to automatic limits.
                Fl::event_clicks( 0 );
                m_Dragging = false;
                ClearHighlight();
                if ( m_LimitsCB )
                {
                    m_LimitsCB( m_LimitsData, 0, 0, 0, 0, true );
                }
                return 1;
            }
            m_PushX = m_DragX = Fl::event_x();
            m_PushY = m_DragY = Fl::event_y();
            if ( Fl::event_button() == FL_LEFT_MOUSE && !Fl::event_shift() )
            {
                m_Dragging = true;
            }
            else if ( current_x() && current_y() )
            {
                m_Panning = true;
                m_PanXMin = current_x()->minimum();
                m_PanXMax = current_x()->maximum();
                m_PanYMin = current_y()->minimum();
                m_PanYMax = current_y()->maximum();
                m_PanKx = PanScale( current_x(), m_PanXMin, m_PanXMax );
                m_PanKy = PanScale( current_y(), m_PanYMin, m_PanYMax );
            }
            return 1;
        case FL_DRAG:
            if ( m_PressTaken )
            {
                m_PressCB( m_PressData, FL_DRAG, Fl::event_x(), Fl::event_y() );
                return 1;
            }
            m_DragX = Fl::event_x();
            m_DragY = Fl::event_y();
            if ( m_Dragging )
            {
                redraw();
            }
            else if ( m_Panning && m_LimitsCB && current_x() && current_y() )
            {
                double x0 = m_PanXMin;
                double x1 = m_PanXMax;
                double y0 = m_PanYMin;
                double y1 = m_PanYMax;
                PanShift( current_x(), m_PanKx, m_DragX - m_PushX, x0, x1 );
                PanShift( current_y(), m_PanKy, m_DragY - m_PushY, y0, y1 );
                SendLimits( x0, x1, y0, y1 );
            }
            return 1;
        case FL_RELEASE:
            // Only the button that started a drag ends it.
            if ( Fl::event_button() != m_PushButton )
            {
                return 1;
            }
            if ( m_PressTaken )
            {
                m_PressTaken = false;
                m_PressCB( m_PressData, FL_RELEASE, Fl::event_x(), Fl::event_y() );
                return 1;
            }
            if ( m_Dragging )
            {
                m_Dragging = false;
                int dw = abs( m_DragX - m_PushX );
                int dh = abs( m_DragY - m_PushY );
                if ( dw > 4 && dh > 4 && m_LimitsCB && current_x() && current_y() )
                {
                    double x0 = current_x()->value( m_PushX );
                    double x1 = current_x()->value( m_DragX );
                    double y0 = current_y()->value( m_PushY );
                    double y1 = current_y()->value( m_DragY );
                    SendLimits( x0, x1, y0, y1 );
                }
                redraw();
            }
            m_Panning = false;
            return 1;
        case FL_MOUSEWHEEL:
        {
            // Sideways scrolling goes on to the parent.
            if ( Fl::event_dy() == 0 )
            {
                break;
            }
            double factor = 1.2;
            if ( Fl::event_dy() < 0 )
            {
                factor = 1.0 / 1.2;
            }
            ZoomAbout( Fl::event_x(), Fl::event_y(), factor );
            return 1;
        }
    }
    return Ca_Canvas::handle( event );
}

PlotLimitParms::PlotLimitParms()
{
    m_ScreenMgr = nullptr;
    m_XMinManual = nullptr;
    m_XMaxManual = nullptr;
    m_YMinManual = nullptr;
    m_YMaxManual = nullptr;
    m_XMin = nullptr;
    m_XMax = nullptr;
    m_YMin = nullptr;
    m_YMax = nullptr;
}

void PlotLimitParms::Link( ScreenMgr* mgr, BoolParm & xmin_manual, BoolParm & xmax_manual, BoolParm & ymin_manual, BoolParm & ymax_manual,
                           Parm & xmin, Parm & xmax, Parm & ymin, Parm & ymax )
{
    m_ScreenMgr = mgr;
    m_XMinManual = &xmin_manual;
    m_XMaxManual = &xmax_manual;
    m_YMinManual = &ymin_manual;
    m_YMaxManual = &ymax_manual;
    m_XMin = &xmin;
    m_XMax = &xmax;
    m_YMin = &ymin;
    m_YMax = &ymax;
}

void SetPlotLimitParms( void* data, double xmin, double xmax, double ymin, double ymax, bool reset )
{
    PlotLimitParms* p = ( PlotLimitParms* ) data;
    if ( !p || !p->m_XMinManual || !p->m_XMaxManual || !p->m_YMinManual || !p->m_YMaxManual ||
         !p->m_XMin || !p->m_XMax || !p->m_YMin || !p->m_YMax )
    {
        return;
    }

    if ( reset )
    {
        p->m_XMinManual->Set( false );
        p->m_XMaxManual->Set( false );
        p->m_YMinManual->Set( false );
        p->m_YMaxManual->Set( false );
    }
    else
    {
        p->m_XMin->Set( xmin );
        p->m_XMax->Set( xmax );
        p->m_YMin->Set( ymin );
        p->m_YMax->Set( ymax );
        p->m_XMinManual->Set( true );
        p->m_XMaxManual->Set( true );
        p->m_YMinManual->Set( true );
        p->m_YMaxManual->Set( true );
    }

    if ( p->m_ScreenMgr )
    {
        p->m_ScreenMgr->SetUpdateFlag( true );
    }
}
