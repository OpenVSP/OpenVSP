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

    // The series the curve makes from the Results that exist now.
    void GetSeriesDefs( vector < ResultsSeriesDef > & defs ) const;

    string GetText() const;

    IntParm m_Mode;
    IntParm m_Element;
    IntParm m_MatrixRows;
    IntParm m_Run;          // 0 for every run, 1 for the newest, 2 for the one before, ...

    vector < ResultsSource > m_Sources;
    string m_XLabel;                    // Empty for the point index or Result order
    vector < string > m_YLabels;
};

#endif //VSP_TOP_RESULTSPLOTMGR_H
