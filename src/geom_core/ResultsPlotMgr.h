//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//
// Plots of Results data, saved with the model.
//
//////////////////////////////////////////////////////////////////////

#ifndef VSP_TOP_RESULTSPLOTMGR_H
#define VSP_TOP_RESULTSPLOTMGR_H

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

#endif //VSP_TOP_RESULTSPLOTMGR_H
