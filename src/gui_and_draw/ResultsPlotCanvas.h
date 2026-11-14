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

    // Legend entries one at a time, in the placement the canvas has; a marker of CA_NO_POINT draws no point.
    void ClearLegend();
    void SetLegendPlacement( int placement );
    void AddLegendEntry( const string & text, Fl_Color c, bool show_line = true, int linestyle = FL_SOLID, int marker = CA_NO_POINT );

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

    // Called when the readout text changes.  Without it, the canvas shows the readout itself.
    typedef void ( *ReadoutCB )( void* data, const string & text );
    void SetReadoutCallback( ReadoutCB cb, void* data )
    {
        m_ReadoutCB = cb;
        m_ReadoutData = data;
    }

    // Points of one more series, for the readout; NameNewSeries names those added since it last ran.
    void ClearSeriesPoints();
    void AddSeriesPoints( const vector < double > & x, const vector < double > & y );
    void NameNewSeries( const string & name );

    // Offered each mouse press first; returning true takes the press and its drag and release.
    typedef bool ( *PressCB )( void* data, int event, int mx, int my );
    void SetPressCallback( PressCB cb, void* data )
    {
        m_PressCB = cb;
        m_PressData = data;
    }

protected:

    void DrawLegend();
    void LegendSize( int & lw, int & lh );
    void LegendCorner( int corner, int lw, int lh, int & lx, int & ly );
    int CountPointsIn( int x0, int y0, int w, int h );
    void UpdateReadout( int mx, int my );
    bool AxisValue( Ca_Axis_* axis, int pix, double & val );
    void ZoomAbout( int mx, int my, double factor );
    void AnchorPoint( int anchor, int mx, int my, int & ax, int & ay );
    void SendLimits( double x0, double x1, double y0, double y1 );
    void ClearHighlight();
    void SetReadout( const string & text );
    void DrawReadout();
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
    bool m_DragZooming;
    int m_ZoomY;                    // Where a drag zoom last zoomed from
    int m_ZoomAnchorX;              // The point a drag zoom keeps fixed
    int m_ZoomAnchorY;
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
    string m_Readout;       // Shown on the canvas when there is no readout callback
    int m_NumNamed;         // Series named by NameNewSeries

    PressCB m_PressCB;
    void* m_PressData;
    bool m_PressTaken;      // The owner took the press under way
};

// Manual axis limit Parms that a canvas's zoom, pan and double click set.
class ScreenMgr;
class PlotLimitParms
{
public:
    PlotLimitParms();

    // Set the Parms a canvas's limit callback sets.
    void Link( ScreenMgr* mgr, BoolParm & xmin_manual, BoolParm & xmax_manual, BoolParm & ymin_manual, BoolParm & ymax_manual,
               Parm & xmin, Parm & xmax, Parm & ymin, Parm & ymax );

    ScreenMgr* m_ScreenMgr;     // Told to update the screens after a change

    BoolParm* m_XMinManual;
    BoolParm* m_XMaxManual;
    BoolParm* m_YMinManual;
    BoolParm* m_YMaxManual;
    Parm* m_XMin;
    Parm* m_XMax;
    Parm* m_YMin;
    Parm* m_YMax;
};

// A ResultsPlotCanvas::LimitsCB for data that is a PlotLimitParms: new limits are set by hand, reset makes them automatic.
void SetPlotLimitParms( void* data, double xmin, double xmax, double ymin, double ymax, bool reset );

// Add the legend placements, RESULTS_PLOT_LEGEND values, to a choice.
void AddLegendPlacementItems( Choice & choice );

// A pixel position as an int, held within a range that drawing and int casts can take.
int ResultsPixel( double p );

#endif //VSP_TOP_RESULTSPLOTCANVAS_H
