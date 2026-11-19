//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//
// Plots of Results data, saved with the model.
//
//////////////////////////////////////////////////////////////////////

#include "ResultsPlotMgr.h"

#include "IDMgr.h"
#include "ParmMgr.h"
#include "ResultsMgr.h"
#include "Vehicle.h"
#include "XmlUtil.h"

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

//==== Series ====//
ResultsSeriesDef::ResultsSeriesDef()
{
    m_Mode = vsp::RESULTS_PLOT_VECTORS;
    m_Element = vsp::RESULTS_PLOT_FIRST;
    m_MatrixRows = false;
}

bool ResultsSeriesDef::operator==( const ResultsSeriesDef & other ) const
{
    return m_Mode == other.m_Mode && m_Element == other.m_Element && m_MatrixRows == other.m_MatrixRows &&
           m_ResultIDs == other.m_ResultIDs && m_XLabel == other.m_XLabel && m_YLabel == other.m_YLabel;
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

//==== Curve ====//
ResultsCurve::ResultsCurve() : ParmContainer()
{
    m_Name = "ResultsCurve";

    string groupname = "ResultsCurve";

    m_Mode.Init( "Mode", groupname, this, vsp::RESULTS_PLOT_VECTORS, vsp::RESULTS_PLOT_VECTORS, vsp::RESULTS_PLOT_ACROSS_RESULTS );
    m_Mode.SetDescript( "Plot each Result's vectors, or one point per Result" );
    m_Element.Init( "Element", groupname, this, vsp::RESULTS_PLOT_FIRST, vsp::RESULTS_PLOT_FIRST, vsp::RESULTS_PLOT_LAST );
    m_Element.SetDescript( "Element of each Result's vector plotted across Results" );
    m_MatrixRows.Init( "MatrixRows", groupname, this, 0, 0, 1 );
    m_MatrixRows.SetDescript( "Plot matrix rows rather than columns" );
    m_Run.Init( "Run", groupname, this, 0, 0, 1000000 );
    m_Run.SetDescript( "Runs of each kind of Result plotted: 0 every run, 1 the newest, 2 the one before, ..." );
}

xmlNodePtr ResultsCurve::EncodeXml( xmlNodePtr & node )
{
    xmlNodePtr curve_node = xmlNewChild( node, NULL, BAD_CAST"ResultsCurve", NULL );

    ParmContainer::EncodeXml( curve_node );

    for ( int i = 0; i < ( int ) m_Sources.size(); i++ )
    {
        xmlNodePtr src_node = xmlNewChild( curve_node, NULL, BAD_CAST"Source", NULL );
        XmlUtil::AddStringNode( src_node, "Name", m_Sources[i].m_Name );
        XmlUtil::AddStringNode( src_node, "Case", m_Sources[i].m_Case );
    }

    XmlUtil::AddStringNode( curve_node, "XLabel", m_XLabel );

    for ( int i = 0; i < ( int ) m_YLabels.size(); i++ )
    {
        XmlUtil::AddStringNode( curve_node, "YLabel", m_YLabels[i] );
    }

    return curve_node;
}

xmlNodePtr ResultsCurve::DecodeXml( xmlNodePtr & node )
{
    ParmContainer::DecodeXml( node );

    m_Sources.clear();
    vector < xmlNodePtr > src_nodes = XmlUtil::GetNodes( node, "Source" );
    for ( int i = 0; i < ( int ) src_nodes.size(); i++ )
    {
        m_Sources.push_back( ResultsSource( XmlUtil::FindString( src_nodes[i], "Name", string() ),
                                            XmlUtil::FindString( src_nodes[i], "Case", string() ) ) );
    }

    m_XLabel = XmlUtil::FindString( node, "XLabel", string() );

    m_YLabels.clear();
    vector < xmlNodePtr > y_nodes = XmlUtil::GetNodes( node, "YLabel" );
    for ( int i = 0; i < ( int ) y_nodes.size(); i++ )
    {
        m_YLabels.push_back( XmlUtil::ExtractString( y_nodes[i] ) );
    }

    return node;
}

void ResultsCurve::CopyFrom( ResultsCurve* from )
{
    if ( !from || from == this )
    {
        return;
    }
    CopyVals( from );
    m_Sources = from->m_Sources;
    m_XLabel = from->m_XLabel;
    m_YLabels = from->m_YLabels;
}

void ResultsCurve::SetDisplaySuffix( int num )
{
    for ( int i = 0; i < ( int ) m_ParmVec.size(); i++ )
    {
        Parm* p = ParmMgr.FindParm( m_ParmVec[i] );
        if ( p )
        {
            p->SetGroupDisplaySuffix( num );
        }
    }
}

void ResultsCurve::GetResultIDs( vector < string > & ids ) const
{
    vector < string > all;
    SortedResultsIDs( all );

    vector < bool > picked( all.size(), false );
    for ( int s = 0; s < ( int ) m_Sources.size(); s++ )
    {
        vector < int > runs;
        for ( int i = 0; i < ( int ) all.size(); i++ )
        {
            if ( m_Sources[s].Matches( ResultsMgr.FindResultsPtr( all[i] ) ) )
            {
                runs.push_back( i );
            }
        }

        if ( m_Run() == 0 )
        {
            for ( int i = 0; i < ( int ) runs.size(); i++ )
            {
                picked[ runs[i] ] = true;
            }
        }
        else if ( m_Run() <= ( int ) runs.size() )
        {
            picked[ runs[ runs.size() - m_Run() ] ] = true;
        }
    }

    ids.clear();
    for ( int i = 0; i < ( int ) all.size(); i++ )
    {
        if ( picked[i] )
        {
            ids.push_back( all[i] );
        }
    }
}

void ResultsCurve::GetSeriesDefs( vector < ResultsSeriesDef > & defs ) const
{
    defs.clear();

    vector < string > ids;
    GetResultIDs( ids );
    if ( ids.empty() )
    {
        return;
    }

    ResultsSeriesDef base;
    base.m_Mode = m_Mode();
    base.m_Element = m_Element();
    base.m_MatrixRows = m_MatrixRows();
    base.m_XLabel = m_XLabel;

    // Each Result's variable labels, sorted, listed once rather than per Y.
    vector < vector < string > > labels( ids.size() );
    if ( base.m_Mode != vsp::RESULTS_PLOT_ACROSS_RESULTS )
    {
        for ( int i = 0; i < ( int ) ids.size(); i++ )
        {
            vector < ResultsPlotVar > vars;
            ListResultsPlotVars( ResultsMgr.FindResultsPtr( ids[i] ), base.m_MatrixRows, vars );
            for ( int j = 0; j < ( int ) vars.size(); j++ )
            {
                labels[i].push_back( vars[j].m_Label );
            }
            std::sort( labels[i].begin(), labels[i].end() );
        }
    }

    for ( int k = 0; k < ( int ) m_YLabels.size(); k++ )
    {
        ResultsSeriesDef def = base;
        def.m_YLabel = m_YLabels[k];

        if ( base.m_Mode == vsp::RESULTS_PLOT_ACROSS_RESULTS )
        {
            def.m_ResultIDs = ids;
            defs.push_back( def );
        }
        else
        {
            for ( int i = 0; i < ( int ) ids.size(); i++ )
            {
                // Only Results that have the variable.
                if ( std::binary_search( labels[i].begin(), labels[i].end(), def.m_YLabel ) )
                {
                    def.m_ResultIDs.assign( 1, ids[i] );
                    defs.push_back( def );
                }
            }
        }
    }
}

string ResultsCurve::GetText() const
{
    string y;
    for ( int i = 0; i < ( int ) m_YLabels.size(); i++ )
    {
        if ( i > 0 )
        {
            y += ", ";
        }
        y += m_YLabels[i];
    }
    if ( y.empty() )
    {
        y = "(no Y)";
    }

    string x = m_XLabel;
    if ( x.empty() )
    {
        if ( m_Mode() == vsp::RESULTS_PLOT_ACROSS_RESULTS )
        {
            x = "Result Order";
        }
        else
        {
            x = "Index";
        }
    }

    string src;
    for ( int i = 0; i < ( int ) m_Sources.size(); i++ )
    {
        if ( i > 0 )
        {
            src += ", ";
        }
        src += m_Sources[i].GetText();
    }
    if ( src.empty() )
    {
        src = "(no Results)";
    }

    string run;
    if ( m_Run() == 0 )
    {
        run = "all";
    }
    else if ( m_Run() == 1 )
    {
        run = "newest";
    }
    else
    {
        char buf[64];
        snprintf( buf, sizeof( buf ), "newest - %d", m_Run() - 1 );
        run = buf;
    }
    if ( m_Mode() == vsp::RESULTS_PLOT_ACROSS_RESULTS )
    {
        run = "across " + run;
        if ( m_Element() == vsp::RESULTS_PLOT_LAST )
        {
            run += ", last";
        }
    }
    if ( m_MatrixRows() )
    {
        run += ", rows";
    }

    return y + " vs " + x + " - " + src + " (" + run + ")";
}

//==== Plot ====//
ResultsPlot::ResultsPlot() : ParmContainer()
{
    m_Name = "Plot";

    string groupname = "ResultsPlot";

    m_XMinManual.Init( "XMinManual", groupname, this, false, 0, 1 );
    m_XMinManual.SetDescript( "Set the X axis minimum by hand" );
    m_XMaxManual.Init( "XMaxManual", groupname, this, false, 0, 1 );
    m_XMaxManual.SetDescript( "Set the X axis maximum by hand" );
    m_YMinManual.Init( "YMinManual", groupname, this, false, 0, 1 );
    m_YMinManual.SetDescript( "Set the Y axis minimum by hand" );
    m_YMaxManual.Init( "YMaxManual", groupname, this, false, 0, 1 );
    m_YMaxManual.SetDescript( "Set the Y axis maximum by hand" );
    m_XMin.Init( "XMin", groupname, this, 0.0, -1e300, 1e300 );
    m_XMin.SetDescript( "X axis minimum" );
    m_XMax.Init( "XMax", groupname, this, 1.0, -1e300, 1e300 );
    m_XMax.SetDescript( "X axis maximum" );
    m_YMin.Init( "YMin", groupname, this, 0.0, -1e300, 1e300 );
    m_YMin.SetDescript( "Y axis minimum" );
    m_YMax.Init( "YMax", groupname, this, 1.0, -1e300, 1e300 );
    m_YMax.SetDescript( "Y axis maximum" );
    m_XLog.Init( "XLog", groupname, this, false, 0, 1 );
    m_XLog.SetDescript( "Logarithmic X axis" );
    m_YLog.Init( "YLog", groupname, this, false, 0, 1 );
    m_YLog.SetDescript( "Logarithmic Y axis" );

    m_ShowLines.Init( "ShowLines", groupname, this, true, 0, 1 );
    m_ShowLines.SetDescript( "Draw lines through each series' points" );
    m_ShowPoints.Init( "ShowPoints", groupname, this, true, 0, 1 );
    m_ShowPoints.SetDescript( "Draw a marker at each point" );
    m_Legend.Init( "Legend", groupname, this, vsp::RESULTS_LEGEND_BEST, vsp::RESULTS_LEGEND_BEST, vsp::RESULTS_LEGEND_OFF );
    m_Legend.SetDescript( "Legend placement" );

    m_EditIndex = -1;
    m_HoldNotice = false;

    Update();
}

ResultsPlot::~ResultsPlot()
{
    DeleteAllCurves();
}

void ResultsPlot::ParmChanged( Parm* parm_ptr, int type )
{
    Update();
    if ( !m_HoldNotice )
    {
        ParmContainer::ParmChanged( parm_ptr, type );
    }
}

void ResultsPlot::Update()
{
    // Limits set automatically are shown but not edited.
    m_XMin.SetActiveFlag( m_XMinManual() );
    m_XMax.SetActiveFlag( m_XMaxManual() );
    m_YMin.SetActiveFlag( m_YMinManual() );
    m_YMax.SetActiveFlag( m_YMaxManual() );
}

xmlNodePtr ResultsPlot::EncodeXml( xmlNodePtr & node )
{
    xmlNodePtr plot_node = xmlNewChild( node, NULL, BAD_CAST"ResultsPlot", NULL );

    ParmContainer::EncodeXml( plot_node );

    XmlUtil::AddIntNode( plot_node, "EditIndex", m_EditIndex );

    for ( int i = 0; i < ( int ) m_Curves.size(); i++ )
    {
        m_Curves[i]->EncodeXml( plot_node );
    }

    return plot_node;
}

xmlNodePtr ResultsPlot::DecodeXml( xmlNodePtr & node )
{
    ParmContainer::DecodeXml( node );

    DeleteAllCurves();
    vector < xmlNodePtr > curve_nodes = XmlUtil::GetNodes( node, "ResultsCurve" );
    for ( int i = 0; i < ( int ) curve_nodes.size(); i++ )
    {
        ResultsCurve* curve = new ResultsCurve();
        curve->DecodeXml( curve_nodes[i] );
        m_Curves.push_back( curve );
    }
    NumberCurves();

    SetEditIndex( XmlUtil::FindInt( node, "EditIndex", 0 ) );

    Update();

    return node;
}

void ResultsPlot::CopyFrom( ResultsPlot* from )
{
    if ( !from || from == this )
    {
        return;
    }

    CopyVals( from );

    DeleteAllCurves();
    for ( int i = 0; i < ( int ) from->m_Curves.size(); i++ )
    {
        ResultsCurve* curve = new ResultsCurve();
        curve->CopyFrom( from->m_Curves[i] );
        m_Curves.push_back( curve );
    }
    NumberCurves();
    SetEditIndex( from->m_EditIndex );

    Update();
}

// Set a Parm, which updates links from it, with the notice to the Vehicle held; true if it changed.
bool ResultsPlot::SetHeld( Parm & parm, double val )
{
    double old = parm.Get();
    m_HoldNotice = true;
    parm.Set( val );
    m_HoldNotice = false;
    return parm.Get() != old;
}

void ResultsPlot::SetManualLimits( double xmin, double xmax, double ymin, double ymax )
{
    bool changed = false;
    changed = SetHeld( m_XMinManual, 1.0 ) || changed;
    changed = SetHeld( m_XMaxManual, 1.0 ) || changed;
    changed = SetHeld( m_YMinManual, 1.0 ) || changed;
    changed = SetHeld( m_YMaxManual, 1.0 ) || changed;
    changed = SetHeld( m_XMin, xmin ) || changed;
    changed = SetHeld( m_XMax, xmax ) || changed;
    changed = SetHeld( m_YMin, ymin ) || changed;
    changed = SetHeld( m_YMax, ymax ) || changed;
    if ( changed )
    {
        ParmChanged( &m_XMin, Parm::SET );
    }
}

void ResultsPlot::SetAutoLimits()
{
    bool changed = false;
    changed = SetHeld( m_XMinManual, 0.0 ) || changed;
    changed = SetHeld( m_XMaxManual, 0.0 ) || changed;
    changed = SetHeld( m_YMinManual, 0.0 ) || changed;
    changed = SetHeld( m_YMaxManual, 0.0 ) || changed;
    if ( changed )
    {
        ParmChanged( &m_XMinManual, Parm::SET );
    }
}

void ResultsPlot::ShowAutoLimits( double xmin, double xmax, double ymin, double ymax )
{
    if ( !m_XMinManual() )
    {
        SetHeld( m_XMin, xmin );
    }
    if ( !m_XMaxManual() )
    {
        SetHeld( m_XMax, xmax );
    }
    if ( !m_YMinManual() )
    {
        SetHeld( m_YMin, ymin );
    }
    if ( !m_YMaxManual() )
    {
        SetHeld( m_YMax, ymax );
    }
}

void ResultsPlot::AddLinkableParms( vector < string > & linkable_parm_vec, const string & link_container_id )
{
    ParmContainer::AddLinkableParms( linkable_parm_vec, link_container_id );

    for ( int i = 0; i < ( int ) m_Curves.size(); i++ )
    {
        m_Curves[i]->AddLinkableParms( linkable_parm_vec, m_ID );
    }
}

void ResultsPlot::NumberCurves()
{
    for ( int i = 0; i < ( int ) m_Curves.size(); i++ )
    {
        m_Curves[i]->SetDisplaySuffix( i );
    }
}

ResultsCurve* ResultsPlot::GetEditCurve() const
{
    return GetCurve( m_EditIndex );
}

void ResultsPlot::SetEditIndex( int index )
{
    if ( m_Curves.empty() )
    {
        m_EditIndex = -1;
    }
    else
    {
        m_EditIndex = std::max( 0, std::min( index, ( int ) m_Curves.size() - 1 ) );
    }
}

ResultsCurve* ResultsPlot::AddCurve( ResultsCurve* from )
{
    ResultsCurve* curve = new ResultsCurve();
    curve->CopyFrom( from );
    m_Curves.push_back( curve );
    m_EditIndex = ( int ) m_Curves.size() - 1;
    NumberCurves();
    return curve;
}

void ResultsPlot::DeleteCurve( int index )
{
    if ( index < 0 || index >= ( int ) m_Curves.size() )
    {
        return;
    }
    delete m_Curves[ index ];
    m_Curves.erase( m_Curves.begin() + index );
    NumberCurves();
    SetEditIndex( index );
}

void ResultsPlot::DeleteAllCurves()
{
    for ( int i = 0; i < ( int ) m_Curves.size(); i++ )
    {
        delete m_Curves[i];
    }
    m_Curves.clear();
    m_EditIndex = -1;
}

ResultsCurve* ResultsPlot::GetCurve( int index ) const
{
    if ( index >= 0 && index < ( int ) m_Curves.size() )
    {
        return m_Curves[ index ];
    }
    return nullptr;
}

void ResultsPlot::GetSeriesDefs( vector < ResultsSeriesDef > & defs ) const
{
    defs.clear();

    for ( int i = 0; i < ( int ) m_Curves.size(); i++ )
    {
        vector < ResultsSeriesDef > cdefs;
        m_Curves[i]->GetSeriesDefs( cdefs );
        for ( int j = 0; j < ( int ) cdefs.size(); j++ )
        {
            if ( std::find( defs.begin(), defs.end(), cdefs[j] ) == defs.end() )
            {
                defs.push_back( cdefs[j] );
            }
        }
    }
}

//==== Manager ====//
ResultsPlotMgrSingleton::ResultsPlotMgrSingleton()
{
    m_CurrIndex = -1;
}

ResultsPlotMgrSingleton::~ResultsPlotMgrSingleton()
{
    Wype();
}

xmlNodePtr ResultsPlotMgrSingleton::EncodeXml( xmlNodePtr & node )
{
    xmlNodePtr mgr_node = xmlNewChild( node, NULL, BAD_CAST"ResultsPlotMgr", NULL );

    XmlUtil::AddIntNode( mgr_node, "CurrIndex", m_CurrIndex );

    for ( int i = 0; i < ( int ) m_Plots.size(); i++ )
    {
        m_Plots[i]->EncodeXml( mgr_node );
    }

    return mgr_node;
}

xmlNodePtr ResultsPlotMgrSingleton::DecodeXml( xmlNodePtr & node )
{
    xmlNodePtr mgr_node = XmlUtil::GetNode( node, "ResultsPlotMgr", 0 );
    if ( !mgr_node )
    {
        return node;
    }

    // Plots of an inserted file go after the ones already here.
    int nbefore = ( int ) m_Plots.size();

    vector < xmlNodePtr > plot_nodes = XmlUtil::GetNodes( mgr_node, "ResultsPlot" );
    for ( int i = 0; i < ( int ) plot_nodes.size(); i++ )
    {
        ResultsPlot* plot = new ResultsPlot();
        plot->DecodeXml( plot_nodes[i] );
        if ( nbefore > 0 && NameUsed( plot->GetName() ) )
        {
            plot->SetName( UniqueName( plot->GetName() ) );
        }
        m_Plots.push_back( plot );
    }

    if ( nbefore == 0 )
    {
        SetCurrIndex( XmlUtil::FindInt( mgr_node, "CurrIndex", 0 ) );
    }

    return node;
}

void ResultsPlotMgrSingleton::Renew()
{
    Wype();
}

void ResultsPlotMgrSingleton::Wype()
{
    for ( int i = 0; i < ( int ) m_Plots.size(); i++ )
    {
        delete m_Plots[i];
    }
    m_Plots.clear();
    m_CurrIndex = -1;
}

void ResultsPlotMgrSingleton::AddLinkableContainers( vector < string > & linkable_container_vec )
{
    for ( int i = 0; i < ( int ) m_Plots.size(); i++ )
    {
        m_Plots[i]->AddLinkableContainers( linkable_container_vec );
    }
}

bool ResultsPlotMgrSingleton::WritePlotFile( int index, const string & file_name )
{
    ResultsPlot* plot = GetPlot( index );
    if ( !plot )
    {
        return false;
    }

    xmlDocPtr doc = xmlNewDoc( ( const xmlChar * )"1.0" );

    xmlNodePtr root = xmlNewNode( nullptr, ( const xmlChar * )"Vsp_ResultsPlot" );
    xmlDocSetRootElement( doc, root );
    XmlUtil::AddIntNode( root, "Version", CURRENT_FILE_VER );

    plot->EncodeXml( root );

    int err = xmlSaveFormatFile( file_name.c_str(), doc, 1 );
    xmlFreeDoc( doc );

    return err != -1;
}

ResultsPlot* ResultsPlotMgrSingleton::ReadPlotFile( const string & file_name )
{
    LIBXML_TEST_VERSION
    xmlKeepBlanksDefault( 0 );

    xmlDocPtr doc = xmlReadFile( file_name.c_str(), nullptr, 0 );
    if ( !doc )
    {
        return nullptr;
    }

    xmlNodePtr root = xmlDocGetRootElement( doc );
    xmlNodePtr plot_node = nullptr;
    if ( root && !xmlStrcmp( root->name, ( const xmlChar * )"Vsp_ResultsPlot" ) )
    {
        plot_node = XmlUtil::GetNode( root, "ResultsPlot", 0 );
    }
    if ( !plot_node )
    {
        xmlFreeDoc( doc );
        return nullptr;
    }

    // IDs the model already uses are issued afresh, so one file can be read into a model again and again.
    string lastreset = IDMgr.ResetRemapID();
    IDMgr.PreRegisterIDs( root );

    ResultsPlot* plot = new ResultsPlot();
    plot->DecodeXml( plot_node );

    IDMgr.ResetRemapID( lastreset );
    xmlFreeDoc( doc );

    if ( NameUsed( plot->GetName() ) )
    {
        plot->SetName( UniqueName( plot->GetName() ) );
    }
    m_Plots.push_back( plot );
    m_CurrIndex = ( int ) m_Plots.size() - 1;
    return plot;
}

bool ResultsPlotMgrSingleton::NameUsed( const string & name ) const
{
    for ( int i = 0; i < ( int ) m_Plots.size(); i++ )
    {
        if ( m_Plots[i]->GetName() == name )
        {
            return true;
        }
    }
    return false;
}

string ResultsPlotMgrSingleton::UniqueName( const string & base ) const
{
    for ( int n = 1; ; n++ )
    {
        char buf[32];
        snprintf( buf, sizeof( buf ), " %d", n );
        string name = base + buf;
        if ( !NameUsed( name ) )
        {
            return name;
        }
    }
}

ResultsPlot* ResultsPlotMgrSingleton::AddPlot()
{
    ResultsPlot* plot = new ResultsPlot();
    plot->SetName( UniqueName( "Plot" ) );
    plot->AddCurve( nullptr );
    m_Plots.push_back( plot );
    m_CurrIndex = ( int ) m_Plots.size() - 1;
    return plot;
}

ResultsPlot* ResultsPlotMgrSingleton::CopyPlot( int index )
{
    ResultsPlot* from = GetPlot( index );
    if ( !from )
    {
        return nullptr;
    }

    ResultsPlot* plot = new ResultsPlot();
    plot->CopyFrom( from );
    string name = from->GetName() + " Copy";
    if ( NameUsed( name ) )
    {
        name = UniqueName( name );
    }
    plot->SetName( name );
    m_Plots.push_back( plot );
    m_CurrIndex = ( int ) m_Plots.size() - 1;
    return plot;
}

void ResultsPlotMgrSingleton::DeletePlot( int index )
{
    if ( index < 0 || index >= ( int ) m_Plots.size() )
    {
        return;
    }
    delete m_Plots[ index ];
    m_Plots.erase( m_Plots.begin() + index );
    // The plot after the deleted one, or the new last plot, becomes current.
    if ( m_CurrIndex > index || m_CurrIndex >= ( int ) m_Plots.size() )
    {
        m_CurrIndex--;
    }
    SetCurrIndex( m_CurrIndex );
}

ResultsPlot* ResultsPlotMgrSingleton::GetPlot( int index ) const
{
    if ( index >= 0 && index < ( int ) m_Plots.size() )
    {
        return m_Plots[ index ];
    }
    return nullptr;
}

void ResultsPlotMgrSingleton::SetCurrIndex( int index )
{
    if ( index >= 0 && index < ( int ) m_Plots.size() )
    {
        m_CurrIndex = index;
    }
    else if ( !m_Plots.empty() )
    {
        m_CurrIndex = 0;
    }
    else
    {
        m_CurrIndex = -1;
    }
}

ResultsPlot* ResultsPlotMgrSingleton::GetCurrPlot() const
{
    return GetPlot( m_CurrIndex );
}
