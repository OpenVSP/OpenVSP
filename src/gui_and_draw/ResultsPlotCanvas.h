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

//==== One legend entry ====//
class ResultsLegendEntry
{
public:
    string m_Text;
    Fl_Color m_Color;
    int m_LineStyle;
    int m_Marker;
    bool m_ShowLine;
    bool m_ShowPoint;
};

//==== Plot canvas with a legend, zoom box, wheel zoom, pan and a point readout ====//
class ResultsPlotCanvas : public Vsp_Canvas
{
public:
    ResultsPlotCanvas( int x, int y, int w, int h );

    virtual void draw();
    virtual int handle( int event );

    void SetLegend( const vector < ResultsLegendEntry > & entries, int placement );

    // The points under the legend's corner choice: x and y in data units per series, and each
    // point's index in its data, which the readout shows.
    void SetSeriesPoints( const vector < vector < double > > & x, const vector < vector < double > > & y, const vector < string > & names,
                          const vector < vector < int > > & index );

    // Called with the new limits after a zoom or pan, and with reset true on a double click.
    typedef void ( *LimitsCB )( void* data, double xmin, double xmax, double ymin, double ymax, bool reset );
    void SetLimitsCallback( LimitsCB cb, void* data )
    {
        m_LimitsCB = cb;
        m_LimitsData = data;
    }

    // Called when the readout text changes.
    typedef void ( *ReadoutCB )( void* data, const string & text );
    void SetReadoutCallback( ReadoutCB cb, void* data )
    {
        m_ReadoutCB = cb;
        m_ReadoutData = data;
    }

protected:

    void DrawLegend();
    void LegendSize( int & lw, int & lh );
    void LegendCorner( int corner, int lw, int lh, int & lx, int & ly );
    int CountPointsIn( int x0, int y0, int w, int h );
    void UpdateReadout( int mx, int my );
    bool AxisValue( Ca_Axis_* axis, int pix, double & val );
    void ZoomAbout( int mx, int my, double factor );
    void SendLimits( double x0, double x1, double y0, double y1 );
    void ClearHighlight();
    double PanScale( Ca_Axis_* axis, double amin, double amax );
    void PanShift( Ca_Axis_* axis, double k, int dpix, double & amin, double & amax );

    vector < ResultsLegendEntry > m_Legend;
    int m_LegendPlacement;

    vector < vector < double > > m_PtX;
    vector < vector < double > > m_PtY;
    vector < string > m_PtNames;
    vector < vector < int > > m_PtIndex;  // Data index of each point; none means its place
    int m_PtGen;                    // Counts point changes

    vector < double > m_BestKey;    // Limits, size and points the best corner was found for
    int m_BestCorner;

    bool m_Dragging;
    bool m_Panning;
    int m_PushButton;               // Button that started the drag
    int m_PushX, m_PushY;
    int m_DragX, m_DragY;
    double m_PanXMin, m_PanXMax, m_PanYMin, m_PanYMax;
    double m_PanKx, m_PanKy;

    int m_HighSeries;
    int m_HighIndex;

    LimitsCB m_LimitsCB;
    void* m_LimitsData;
    ReadoutCB m_ReadoutCB;
    void* m_ReadoutData;
};

// A pixel position as an int, held within a range that drawing and int casts can take.
int ResultsPixel( double p );

#endif //VSP_TOP_RESULTSPLOTCANVAS_H
