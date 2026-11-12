//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//
// Plots of Results data, saved with the model.
//
//////////////////////////////////////////////////////////////////////

#include "ResultsPlotMgr.h"

#include "ResultsMgr.h"

#include <algorithm>
#include <cmath>

//==== Variables ====//
ResultsPlotVar::ResultsPlotVar()
{
    m_DataIndex = 0;
    m_Comp = -1;
    m_Len = 0;
}

static string DataLabel( const string & name, int index )
{
    if ( index == 0 )
    {
        return name;
    }
    char buf[32];
    snprintf( buf, sizeof( buf ), "#%d", index );
    return name + buf;
}

static void AddVar( vector < ResultsPlotVar > & vars, const string & label, const string & name, int index, int comp, int len )
{
    ResultsPlotVar v;
    v.m_Label = label;
    v.m_DataName = name;
    v.m_DataIndex = index;
    v.m_Comp = comp;
    v.m_Len = len;
    vars.push_back( v );
}

static int MatrixCols( const vector < vector < double > > & m )
{
    int ncol = 0;
    for ( int i = 0; i < ( int ) m.size(); i++ )
    {
        ncol = std::max( ncol, ( int ) m[i].size() );
    }
    return ncol;
}

static int IntMatrixCols( const vector < vector < int > > & m )
{
    int ncol = 0;
    for ( int i = 0; i < ( int ) m.size(); i++ )
    {
        ncol = std::max( ncol, ( int ) m[i].size() );
    }
    return ncol;
}

static void AddMatrixVars( vector < ResultsPlotVar > & vars, const string & base, const string & name, int index, int nrow, int ncol, bool matrix_rows )
{
    char buf[64];
    if ( matrix_rows )
    {
        for ( int i = 0; i < nrow; i++ )
        {
            snprintf( buf, sizeof( buf ), "[%d,:]", i );
            AddVar( vars, base + buf, name, index, i, ncol );
        }
    }
    else
    {
        for ( int j = 0; j < ncol; j++ )
        {
            snprintf( buf, sizeof( buf ), "[:,%d]", j );
            AddVar( vars, base + buf, name, index, j, nrow );
        }
    }
}

void ListResultsPlotVars( Results* res, bool matrix_rows, vector < ResultsPlotVar > & vars )
{
    vars.clear();
    if ( !res )
    {
        return;
    }

    vector < string > names = res->GetAllDataNames();
    for ( int i = 0; i < ( int ) names.size(); i++ )
    {
        int ndata = res->GetNumData( names[i] );
        for ( int j = 0; j < ndata; j++ )
        {
            NameValData* nvd = res->FindPtr( names[i], j );
            if ( !nvd )
            {
                continue;
            }

            string base = DataLabel( names[i], j );

            switch ( nvd->GetType() )
            {
                case vsp::BOOL_DATA:
                    AddVar( vars, base, names[i], j, -1, ( int ) nvd->GetBoolData().size() );
                    break;
                case vsp::INT_DATA:
                    AddVar( vars, base, names[i], j, -1, ( int ) nvd->GetIntData().size() );
                    break;
                case vsp::DOUBLE_DATA:
                    AddVar( vars, base, names[i], j, -1, ( int ) nvd->GetDoubleData().size() );
                    break;
                case vsp::VEC3D_DATA:
                {
                    int n = ( int ) nvd->GetVec3dData().size();
                    AddVar( vars, base + ".x", names[i], j, 0, n );
                    AddVar( vars, base + ".y", names[i], j, 1, n );
                    AddVar( vars, base + ".z", names[i], j, 2, n );
                    AddVar( vars, "|" + base + "|", names[i], j, 3, n );
                    break;
                }
                case vsp::INT_MATRIX_DATA:
                {
                    const vector < vector < int > > & m = nvd->GetIntMatData();
                    AddMatrixVars( vars, base, names[i], j, ( int ) m.size(), IntMatrixCols( m ), matrix_rows );
                    break;
                }
                case vsp::DOUBLE_MATRIX_DATA:
                {
                    const vector < vector < double > > & m = nvd->GetDoubleMatData();
                    AddMatrixVars( vars, base, names[i], j, ( int ) m.size(), MatrixCols( m ), matrix_rows );
                    break;
                }
                default:
                    break;
            }
        }
    }
}

bool GetResultsPlotVarData( Results* res, const string & label, bool matrix_rows, vector < double > & data )
{
    data.clear();

    vector < ResultsPlotVar > vars;
    ListResultsPlotVars( res, matrix_rows, vars );

    for ( int i = 0; i < ( int ) vars.size(); i++ )
    {
        if ( vars[i].m_Label != label )
        {
            continue;
        }

        const ResultsPlotVar & v = vars[i];
        NameValData* nvd = res->FindPtr( v.m_DataName, v.m_DataIndex );
        if ( !nvd )
        {
            return false;
        }

        switch ( nvd->GetType() )
        {
            case vsp::BOOL_DATA:
            {
                const vector < int > & d = nvd->GetBoolData();
                data.assign( d.begin(), d.end() );
                break;
            }
            case vsp::INT_DATA:
            {
                const vector < int > & d = nvd->GetIntData();
                data.assign( d.begin(), d.end() );
                break;
            }
            case vsp::DOUBLE_DATA:
            {
                data = nvd->GetDoubleData();
                break;
            }
            case vsp::VEC3D_DATA:
            {
                const vector < vec3d > & d = nvd->GetVec3dData();
                data.resize( d.size() );
                for ( int k = 0; k < ( int ) d.size(); k++ )
                {
                    if ( v.m_Comp == 3 )
                    {
                        data[k] = d[k].mag();
                    }
                    else
                    {
                        data[k] = d[k][ v.m_Comp ];
                    }
                }
                break;
            }
            case vsp::INT_MATRIX_DATA:
            {
                const vector < vector < int > > & m = nvd->GetIntMatData();
                if ( matrix_rows )
                {
                    data.assign( m[ v.m_Comp ].begin(), m[ v.m_Comp ].end() );
                }
                else
                {
                    data.resize( m.size(), NAN );
                    for ( int k = 0; k < ( int ) m.size(); k++ )
                    {
                        if ( v.m_Comp < ( int ) m[k].size() )
                        {
                            data[k] = m[k][ v.m_Comp ];
                        }
                    }
                }
                break;
            }
            case vsp::DOUBLE_MATRIX_DATA:
            {
                const vector < vector < double > > & m = nvd->GetDoubleMatData();
                if ( matrix_rows )
                {
                    data = m[ v.m_Comp ];
                }
                else
                {
                    data.resize( m.size(), NAN );
                    for ( int k = 0; k < ( int ) m.size(); k++ )
                    {
                        if ( v.m_Comp < ( int ) m[k].size() )
                        {
                            data[k] = m[k][ v.m_Comp ];
                        }
                    }
                }
                break;
            }
            default:
                return false;
        }
        return true;
    }
    return false;
}

//==== Kinds of Result ====//
ResultsSource::ResultsSource()
{
}

ResultsSource::ResultsSource( const string & name, const string & case_name )
{
    m_Name = name;
    m_Case = case_name;
}

bool ResultsSource::operator==( const ResultsSource & other ) const
{
    return m_Name == other.m_Name && m_Case == other.m_Case;
}

bool ResultsSource::Matches( Results* res ) const
{
    return res && res->GetName() == m_Name && GetResultsCaseName( res ) == m_Case;
}

string ResultsSource::GetText() const
{
    if ( m_Case.empty() )
    {
        return m_Name;
    }
    return m_Case + " - " + m_Name;
}

// Every Result, oldest first.
static void SortedResultsIDs( vector < string > & ids )
{
    vector < string > all = ResultsMgr.GetAllResultsIDs();

    vector < std::pair < int, string > > count_ids;
    for ( int i = 0; i < ( int ) all.size(); i++ )
    {
        Results* res = ResultsMgr.FindResultsPtr( all[i] );
        if ( res )
        {
            count_ids.push_back( std::make_pair( res->GetCreationCount(), all[i] ) );
        }
    }
    std::sort( count_ids.begin(), count_ids.end() );

    ids.clear();
    for ( int i = 0; i < ( int ) count_ids.size(); i++ )
    {
        ids.push_back( count_ids[i].second );
    }
}

void ListResultsSources( vector < ResultsSource > & sources, vector < int > & counts )
{
    sources.clear();
    counts.clear();

    vector < string > ids;
    SortedResultsIDs( ids );

    for ( int i = 0; i < ( int ) ids.size(); i++ )
    {
        Results* res = ResultsMgr.FindResultsPtr( ids[i] );
        ResultsSource s( res->GetName(), GetResultsCaseName( res ) );

        vector < ResultsSource >::iterator it = std::find( sources.begin(), sources.end(), s );
        if ( it == sources.end() )
        {
            sources.push_back( s );
            counts.push_back( 1 );
        }
        else
        {
            counts[ it - sources.begin() ]++;
        }
    }
}
