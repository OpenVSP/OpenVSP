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
