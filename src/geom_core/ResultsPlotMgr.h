//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//
// Plots of Results data, saved with the model.
//
//////////////////////////////////////////////////////////////////////

#ifndef VSP_TOP_RESULTSPLOTMGR_H
#define VSP_TOP_RESULTSPLOTMGR_H

#include "APIDefines.h"
#include "ParmContainer.h"
#include "Parm.h"

#include <string>
#include <vector>

using std::string;
using std::vector;

class Results;

//==== One plottable vector in a Result: a whole vector, a vec3d component, or a matrix row or column ====//
class ResultsPlotVar
{
public:
    ResultsPlotVar();

    string m_Label;         // Unique in its Result; series match variables across Results by label
    string m_DataName;
    int m_DataIndex;        // Among entries that share m_DataName
    int m_Comp;             // -1 whole vector; vec3d 0..2 = x, y, z, 3 = magnitude; matrix row or column
    int m_Len;
};

// Every numeric vector in a Result that can be plotted.
void ListResultsPlotVars( Results* res, bool matrix_rows, vector < ResultsPlotVar > & vars );

// The values of the variable with this label, false when the Result has no such variable.
bool GetResultsPlotVarData( Results* res, const string & label, bool matrix_rows, vector < double > & data );

//==== One plotted series, tied to the Results that exist now ====//
class ResultsSeriesDef
{
public:
    ResultsSeriesDef();

    bool operator==( const ResultsSeriesDef & other ) const;

    int m_Mode;
    int m_Element;
    bool m_MatrixRows;
    vector < string > m_ResultIDs;  // One for a vector series, the points' Results across Results
    string m_XLabel;                // Empty for the point index
    string m_YLabel;
};

//==== A kind of Result: its name and its case name ====//
class ResultsSource
{
public:
    ResultsSource();
    ResultsSource( const string & name, const string & case_name );

    bool operator==( const ResultsSource & other ) const;

    bool Matches( Results* res ) const;
    string GetText() const;

    string m_Name;
    string m_Case;
};

// Each kind of Result that exists, in the order first made.
void ListResultsSources( vector < ResultsSource > & sources, vector < int > & counts );

// Every Result of these kinds, oldest first.
void ListResultsOfSources( const vector < ResultsSource > & sources, vector < string > & ids );

//==== What a curve plots: kinds of Result, which runs of them, and their data ====//
class ResultsCurve : public ParmContainer
{
public:
    ResultsCurve();

    virtual xmlNodePtr EncodeXml( xmlNodePtr & node );
    virtual xmlNodePtr DecodeXml( xmlNodePtr & node );

    void CopyFrom( ResultsCurve* from );

    // Number the curve's Parm group, as shown in Parm lists, by its place in its plot.
    void SetDisplaySuffix( int num );

    // The runs of the curve's kinds of Result, oldest first.
    void GetResultIDs( vector < string > & ids ) const;

    // The runs of the curve's kinds a RESULTS_RUN_CHOSEN curve plots, and the choice of them.
    void GetChosenIDs( vector < string > & ids ) const;
    void SetChosenIDs( const vector < string > & ids );

    // The series the curve makes from the Results that exist now.
    void GetSeriesDefs( vector < ResultsSeriesDef > & defs ) const;

    string GetText() const;

    IntParm m_Mode;
    IntParm m_Element;
    IntParm m_MatrixRows;
    IntParm m_Run;          // RESULTS_RUN_CHOSEN, 0 for every run, 1 for the newest, 2 for the one before, ...

    vector < ResultsSource > m_Sources;
    vector < string > m_UncheckedIDs;   // Results left out when m_Run is RESULTS_RUN_CHOSEN, so new ones are plotted;
                                        // not saved, as Results are not
    string m_XLabel;                    // Empty for the point index or Result order
    vector < string > m_YLabels;
};

//==== One plot: its curves, axes and style ====//
class ResultsPlot : public ParmContainer
{
public:
    ResultsPlot();
    virtual ~ResultsPlot();

    virtual void ParmChanged( Parm* parm_ptr, int type );
    void Update();

    virtual xmlNodePtr EncodeXml( xmlNodePtr & node );
    virtual xmlNodePtr DecodeXml( xmlNodePtr & node );

    void CopyFrom( ResultsPlot* from );

    // Set all four limits by hand, or make all four automatic, with one notice of the change.
    void SetManualLimits( double xmin, double xmax, double ymin, double ymax );
    void SetAutoLimits();

    // Show automatic limits in the limit Parms not set by hand, with no notice: they only display.
    void ShowAutoLimits( double xmin, double xmax, double ymin, double ymax );

    // The plot's Parms, then each curve's under the plot.
    virtual void AddLinkableParms( vector < string > & linkable_parm_vec, const string & link_container_id = string() );

    // The curve being edited, null when the plot has none.
    ResultsCurve* GetEditCurve() const;
    int GetEditIndex() const                    { return m_EditIndex; }
    void SetEditIndex( int index );

    // Add a curve like from, or an empty one when from is null, and edit it.
    ResultsCurve* AddCurve( ResultsCurve* from );
    void DeleteCurve( int index );
    void DeleteAllCurves();

    int GetNumCurves() const                    { return ( int ) m_Curves.size(); }
    ResultsCurve* GetCurve( int index ) const;

    // Index of the curve with this ID, -1 when the plot has none.
    int FindCurveIndex( const string & curve_id ) const;

    // Series of every curve, each once.
    void GetSeriesDefs( vector < ResultsSeriesDef > & defs ) const;

    BoolParm m_XMinManual;
    BoolParm m_XMaxManual;
    BoolParm m_YMinManual;
    BoolParm m_YMaxManual;
    Parm m_XMin;
    Parm m_XMax;
    Parm m_YMin;
    Parm m_YMax;
    BoolParm m_XLog;
    BoolParm m_YLog;

    BoolParm m_ShowLines;
    BoolParm m_ShowPoints;
    IntParm m_Legend;

protected:

    void NumberCurves();
    bool SetHeld( Parm & parm, double val );

    vector < ResultsCurve* > m_Curves;
    int m_EditIndex;    // Index of the curve being edited, -1 when there are none
    bool m_HoldNotice;  // Parm changes are not passed on while true
};

//==== Every plot of the model ====//
class ResultsPlotMgrSingleton
{
public:
    static ResultsPlotMgrSingleton& getInstance()
    {
        static ResultsPlotMgrSingleton instance;
        return instance;
    }

    xmlNodePtr EncodeXml( xmlNodePtr & node );
    xmlNodePtr DecodeXml( xmlNodePtr & node );

    void Renew();
    void Wype();

    void AddLinkableContainers( vector < string > & linkable_container_vec );

    // Write a plot, its curves and style but no Results, to a file of its own; false when it cannot be written.
    bool WritePlotFile( int index, const string & file_name );

    // Add the plot a plot file holds, as the plot after the others, and make it current; null when the file
    // holds none.
    ResultsPlot* ReadPlotFile( const string & file_name );

    ResultsPlot* AddPlot();
    ResultsPlot* CopyPlot( int index );
    void DeletePlot( int index );

    int GetNumPlots() const                     { return ( int ) m_Plots.size(); }
    ResultsPlot* GetPlot( int index ) const;

    // Index of the plot with this ID, -1 when there is none.
    int FindPlotIndex( const string & plot_id ) const;
    ResultsPlot* FindPlot( const string & plot_id ) const;

    // The plot holding the curve with this ID and the curve's index in it; null when no plot holds it.
    ResultsPlot* FindCurvePlot( const string & curve_id, int & curve_index ) const;

    int GetCurrIndex() const                    { return m_CurrIndex; }
    void SetCurrIndex( int index );
    ResultsPlot* GetCurrPlot() const;

private:
    ResultsPlotMgrSingleton();
    ~ResultsPlotMgrSingleton();
    ResultsPlotMgrSingleton( ResultsPlotMgrSingleton const& copy ) = delete;
    ResultsPlotMgrSingleton& operator=( ResultsPlotMgrSingleton const& copy ) = delete;

    bool NameUsed( const string & name ) const;
    string UniqueName( const string & base ) const;

    vector < ResultsPlot* > m_Plots;
    int m_CurrIndex;
};

#define ResultsPlotMgr ResultsPlotMgrSingleton::getInstance()

#endif //VSP_TOP_RESULTSPLOTMGR_H
