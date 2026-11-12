//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//
// XY plots of Results data for the Results viewer.
//
//////////////////////////////////////////////////////////////////////

#ifndef VSP_TOP_RESULTSPLOTCANVAS_H
#define VSP_TOP_RESULTSPLOTCANVAS_H

#include "GuiDevice.h"
#include "ResultsPlotMgr.h"

#include <string>
#include <vector>

using std::string;
using std::vector;

//==== A series ready to draw ====//
class ResultsSeries
{
public:
    ResultsSeries();

    ResultsSeriesDef m_Def;
    vector < double > m_X;
    vector < double > m_Y;
    string m_Source;        // Results the series came from, for the legend
    string m_Name;          // Legend text
    string m_Warning;       // Why points were dropped, empty when none were

    Fl_Color m_Color;
    int m_LineStyle;
    int m_Marker;
};

// Fill in the series' points; false when a Result or variable is missing.
bool BuildResultsSeries( const ResultsSeriesDef & def, ResultsSeries & series );

// Colors, dashes, markers and legend names for a set of series.
void StyleResultsSeries( vector < ResultsSeries > & series );

// Axis label from the variable labels in use.
string MakeResultsAxisLabel( const vector < string > & labels, const string & empty_label );

// Rounded limits that hold [dmin, dmax].  Logarithmic limits are whole decades.
void NiceAxisLimits( double dmin, double dmax, bool log_axis, double & amin, double & amax );

// Whether axis limits can be drawn: increasing, positive on a log axis, and far enough apart for ticks.
bool UsableAxisLimits( double amin, double amax, bool log_axis );

#endif //VSP_TOP_RESULTSPLOTCANVAS_H
