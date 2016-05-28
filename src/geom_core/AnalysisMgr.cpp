//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//

// AnalysisMgr.cpp: implementation of the Analysis class and AnalysisMgr Singleton.
// Rob McDonald
//
//////////////////////////////////////////////////////////////////////

#include "AnalysisMgr.h"
#include "Vehicle.h"
#include "VehicleMgr.h"
#include "ProjectionMgr.h"

void RWCollection::Clear()
{
    m_DataMap.clear();
}

//======================================================================================//
//======================================================================================//
//======================================================================================//

//==== Constructor ====//
AnalysisMgrSingleton::AnalysisMgrSingleton()
{
}
//==== Destructor ====//
AnalysisMgrSingleton::~AnalysisMgrSingleton()
{
    Wype();
}

void AnalysisMgrSingleton::Init()
{
    RegisterBuiltins();
}

void AnalysisMgrSingleton::Wype()
{
    map < string, Analysis* >::const_iterator it;

    for ( it = m_AnalysisMap.begin(); it != m_AnalysisMap.end(); it++ )
    {
        delete it->second;
    }

    m_AnalysisMap.clear();
}

void AnalysisMgrSingleton::Renew()
{
    Wype();
    Init();
}

int AnalysisMgrSingleton::GetNumAnalysis() const
{
    return m_AnalysisMap.size();
}

vector < string > AnalysisMgrSingleton::ListAnalysis() const
{
    vector < string > ret;

    map < string, Analysis* >::const_iterator it;

    for ( it = m_AnalysisMap.begin(); it != m_AnalysisMap.end(); it++ )
    {
        ret.push_back( it->first );
    }

    return ret;
}

Analysis* AnalysisMgrSingleton::FindAnalysis( const string & name ) const
{
    map < string, Analysis* >::const_iterator it;

    it = m_AnalysisMap.find( name );

    if ( it != m_AnalysisMap.end() )
    {
        return it->second;
    }

    return NULL;
}

bool AnalysisMgrSingleton::RegisterAnalysis( const string & name, Analysis* asys )
{
    Analysis *b = FindAnalysis( name );

    if ( !b )
    {
        asys->SetDefaults();
        m_AnalysisMap[name] = asys;
        return true;
    }

    return false; // name already in AnalysisMap
}

string AnalysisMgrSingleton::ExecAnalysis( const string & analysis )
{
    Analysis *analysis_ptr = FindAnalysis( analysis );

    if ( !analysis_ptr )
    {
        string ret;
        return ret;
    }

    return analysis_ptr->Execute();
}

bool AnalysisMgrSingleton::ValidAnalysisName( const string & analysis )
{
    Analysis* analysis_ptr = FindAnalysis( analysis );

    if ( !analysis_ptr )
    {
        return false;
    }
    return true;
}

bool AnalysisMgrSingleton::ValidAnalysisInputDataIndex( const string & analysis, const string & name, int index )
{
    Analysis* analysis_ptr = FindAnalysis( analysis );

    if ( !analysis_ptr )
    {
        return false;
    }

    NameValData* inpt_ptr = analysis_ptr->m_Inputs.FindPtr( name, index );
    if ( !inpt_ptr )
    {
        return false;
    }

    return true;
}

int AnalysisMgrSingleton::GetNumInputData( const string & analysis, const string & name )
{
    Analysis* analysis_ptr = FindAnalysis( analysis );
    if ( !analysis_ptr )
    {
        return 0;
    }

    return analysis_ptr->m_Inputs.GetNumData( name );
}

int AnalysisMgrSingleton::GetAnalysisInputType( const string & analysis, const string & name )
{
    Analysis* analysis_ptr = FindAnalysis( analysis );
    if ( !analysis_ptr )
    {
        return vsp::INVALID_TYPE;
    }

    NameValData* inpt_ptr = analysis_ptr->m_Inputs.FindPtr( name );
    if ( !inpt_ptr )
    {
        return vsp::INVALID_TYPE;
    }

    return inpt_ptr->GetType();
}

const vector<int> & AnalysisMgrSingleton::GetIntInputData( const string & analysis, const string & name, int index )
{
    Analysis* analysis_ptr = FindAnalysis( analysis );
    if ( !analysis_ptr )
    {
        return m_DefaultIntVec;
    }

    NameValData* inpt_ptr = analysis_ptr->m_Inputs.FindPtr( name, index );
    if ( !inpt_ptr )
    {
        return m_DefaultIntVec;
    }

    return inpt_ptr->GetIntData();
}

const vector<double> & AnalysisMgrSingleton::GetDoubleInputData( const string & analysis, const string & name, int index )
{
    Analysis* analysis_ptr = FindAnalysis( analysis );
    if ( !analysis_ptr )
    {
        return m_DefaultDoubleVec;
    }

    NameValData* inpt_ptr = analysis_ptr->m_Inputs.FindPtr( name, index );
    if ( !inpt_ptr )
    {
        return m_DefaultDoubleVec;
    }

    return inpt_ptr->GetDoubleData();
}

const vector<string> & AnalysisMgrSingleton::GetStringInputData( const string & analysis, const string & name, int index )
{
    Analysis* analysis_ptr = FindAnalysis( analysis );
    if ( !analysis_ptr )
    {
        return m_DefaultStringVec;
    }

    NameValData* inpt_ptr = analysis_ptr->m_Inputs.FindPtr( name, index );
    if ( !inpt_ptr )
    {
        return m_DefaultStringVec;
    }

    return inpt_ptr->GetStringData();
}

const vector<vec3d> & AnalysisMgrSingleton::GetVec3dInputData( const string & analysis, const string & name, int index )
{
    Analysis* analysis_ptr = FindAnalysis( analysis );
    if ( !analysis_ptr )
    {
        return m_DefaultVec3dVec;
    }

    NameValData* inpt_ptr = analysis_ptr->m_Inputs.FindPtr( name, index );
    if ( !inpt_ptr )
    {
        return m_DefaultVec3dVec;
    }

    return inpt_ptr->GetVec3dData();
}

void AnalysisMgrSingleton::SetAnalysisInputDefaults( const string & analysis )
{
    Analysis* analysis_ptr = FindAnalysis( analysis );
    if ( !analysis_ptr )
    {
        return;
    }

    analysis_ptr->SetDefaults();
}

void AnalysisMgrSingleton::SetIntAnalysisInput( const string & analysis, const string & name, const vector< int > & d, int index )
{
    Analysis* analysis_ptr = FindAnalysis( analysis );
    if ( !analysis_ptr )
    {
        return;
    }

    NameValData* inpt_ptr = analysis_ptr->m_Inputs.FindPtr( name, index );
    if ( !inpt_ptr )
    {
        return;
    }

    inpt_ptr->SetIntData( d );
}

void AnalysisMgrSingleton::SetDoubleAnalysisInput( const string & analysis, const string & name, const vector< double > & d, int index )
{
    Analysis* analysis_ptr = FindAnalysis( analysis );
    if ( !analysis_ptr )
    {
        return;
    }

    NameValData* inpt_ptr = analysis_ptr->m_Inputs.FindPtr( name, index );
    if ( !inpt_ptr )
    {
        return;
    }

    inpt_ptr->SetDoubleData( d );
}

void AnalysisMgrSingleton::SetStringAnalysisInput( const string & analysis, const string & name, const vector< string > & d, int index )
{
    Analysis* analysis_ptr = FindAnalysis( analysis );
    if ( !analysis_ptr )
    {
        return;
    }

    NameValData* inpt_ptr = analysis_ptr->m_Inputs.FindPtr( name, index );
    if ( !inpt_ptr )
    {
        return;
    }

    inpt_ptr->SetStringData( d );
}

void AnalysisMgrSingleton::SetVec3dAnalysisInput( const string & analysis, const string & name, const vector< vec3d > & d, int index )
{
    Analysis* analysis_ptr = FindAnalysis( analysis );
    if ( !analysis_ptr )
    {
        return;
    }

    NameValData* inpt_ptr = analysis_ptr->m_Inputs.FindPtr( name, index );
    if ( !inpt_ptr )
    {
        return;
    }

    inpt_ptr->SetVec3dData( d );
}

void AnalysisMgrSingleton::RegisterBuiltins()
{
    CompGeomAnalysis *cga = new CompGeomAnalysis();

    RegisterAnalysis( "CompGeom", cga );


    MassPropAnalysis *mpa = new MassPropAnalysis();

    RegisterAnalysis( "MassProp", mpa );


    PlanarSliceAnalysis *psa = new PlanarSliceAnalysis();

    RegisterAnalysis( "PlanarSlice", psa );


    ProjectionAnalysis *proj = new ProjectionAnalysis();

    RegisterAnalysis( "Projection", proj );
}

//======================================================================================//
//======================================================================================//
//======================================================================================//

void CompGeomAnalysis::SetDefaults()
{
    m_Inputs.Clear();
    m_Inputs.Add( NameValData( "Set", 0 ) );
    m_Inputs.Add( NameValData( "HalfMeshFlag", 0 ) );
    m_Inputs.Add( NameValData( "SubSurfFlag", 1 ) );

    Vehicle *veh = VehicleMgr.GetVehicle();

    if ( veh )
    {
        m_Inputs.Add( NameValData( "WriteCSVFlag", veh->getExportCompGeomCsvFile() ) );
        m_Inputs.Add( NameValData( "WriteDragTSVFlag", veh->getExportDragBuildTsvFile() ) );
    }
}

string CompGeomAnalysis::Execute()
{
    string res;

    Vehicle *veh = VehicleMgr.GetVehicle();

    if ( veh )
    {
        int geomSet;
        int halfMeshFlag;
        int subSurfFlag;

        NameValData *nvd = NULL;

        nvd = m_Inputs.FindPtr( "Set", 0 );
        if ( nvd )
        {
            geomSet = nvd->GetInt( 0 );
        }

        nvd = m_Inputs.FindPtr( "HalfMeshFlag", 0 );
        if ( nvd )
        {
            halfMeshFlag = nvd->GetInt( 0 );
        }

        nvd = m_Inputs.FindPtr( "SubSurfFlag", 0 );
        if ( nvd )
        {
            subSurfFlag = nvd->GetInt( 0 );
        }

        nvd = m_Inputs.FindPtr( "WriteCSVFlag", 0 );
        veh->setExportCompGeomCsvFile( !!nvd->GetInt( 0 ) );

        nvd = m_Inputs.FindPtr( "WriteDragTSVFlag", 0 );
        veh->setExportDragBuildTsvFile( !!nvd->GetInt( 0 ) );

        string geom = veh->CompGeomAndFlatten( geomSet, halfMeshFlag, subSurfFlag );

        res = ResultsMgr.FindLatestResultsID( "Comp_Geom" );
    }

    return res;
}


//======================================================================================//
//======================================================================================//
//======================================================================================//

void MassPropAnalysis::SetDefaults()
{
    m_Inputs.Clear();
    m_Inputs.Add( NameValData( "Set", 0 ) );

    Vehicle *veh = VehicleMgr.GetVehicle();
    if ( veh )
    {
        m_Inputs.Add( NameValData( "NumMassSlices", veh->m_NumMassSlices ) );
    }
    else
    {
        m_Inputs.Add( NameValData( "NumMassSlices", 20 ) );
    }
}

string MassPropAnalysis::Execute()
{
    string res;

    Vehicle *veh = VehicleMgr.GetVehicle();

    if ( veh )
    {
        int geomSet;
        int numMassSlice;

        NameValData *nvd = NULL;

        nvd = m_Inputs.FindPtr( "Set", 0 );
        if ( nvd )
        {
            geomSet = nvd->GetInt( 0 );
        }

        nvd = m_Inputs.FindPtr( "NumMassSlices", 0 );
        if ( nvd )
        {
            numMassSlice = nvd->GetInt( 0 );
        }

        string geom = veh->MassPropsAndFlatten( geomSet, numMassSlice );

        res = ResultsMgr.FindLatestResultsID( "Mass_Properties" );
    }

    return res;
}

//======================================================================================//
//======================================================================================//
//======================================================================================//

void PlanarSliceAnalysis::SetDefaults()
{
    m_Inputs.Clear();
    m_Inputs.Add( NameValData( "Set", 0 ) );
    m_Inputs.Add( NameValData( "NumSlices", 10 ) );
    m_Inputs.Add( NameValData( "Norm", vec3d( 1.0, 0.0, 0.0 ) ) );
    m_Inputs.Add( NameValData( "AutoBoundFlag", 1 ) );
    m_Inputs.Add( NameValData( "StartVal", 0.0 ) );
    m_Inputs.Add( NameValData( "EndVal", 10.0 ) );
}

string PlanarSliceAnalysis::Execute()
{
    string res;

    Vehicle *veh = VehicleMgr.GetVehicle();

    if ( veh )
    {
        int geomSet;
        int numSlice;
        vec3d axis;
        bool autobnd = true;
        double start, end;

        NameValData *nvd = NULL;

        nvd = m_Inputs.FindPtr( "Set", 0 );
        if ( nvd )
        {
            geomSet = nvd->GetInt( 0 );
        }

        nvd = m_Inputs.FindPtr( "NumSlices", 0 );
        if ( nvd )
        {
            numSlice = nvd->GetInt( 0 );
        }

        nvd = m_Inputs.FindPtr( "Norm", 0 );
        if ( nvd )
        {
            axis = nvd->GetVec3d( 0 );
        }

        nvd = m_Inputs.FindPtr( "AutoBoundFlag", 0 );
        if ( nvd )
        {
            int ab = nvd->GetInt( 0 );
            if ( ab == 1 )
            {
                autobnd = true;
            }
            else
            {
                autobnd = false;
            }
        }

        nvd = m_Inputs.FindPtr( "StartVal", 0 );
        if ( nvd )
        {
            start = nvd->GetDouble( 0 );
        }

        nvd = m_Inputs.FindPtr( "EndVal", 0 );
        if ( nvd )
        {
            end = nvd->GetDouble( 0 );
        }

        string geom = veh->PSliceAndFlatten( geomSet, numSlice,  axis,  autobnd,  start,  end  );

        res = ResultsMgr.FindLatestResultsID( "Slice" );
    }

    return res;
}

//======================================================================================//
//======================================================================================//
//======================================================================================//

void ProjectionAnalysis::SetDefaults()
{
    m_Inputs.Clear();

    m_Inputs.Add( NameValData( "TargetType", vsp::SET_TARGET ) );
    m_Inputs.Add( NameValData( "BoundaryType", vsp::NO_BOUNDARY ) );
    m_Inputs.Add( NameValData( "DirectionType", vsp::X_PROJ ) );

    m_Inputs.Add( NameValData( "TargetSet", 0 ) );
    m_Inputs.Add( NameValData( "BoundarySet", 0 ) );

    m_Inputs.Add( NameValData( "TargetGeomID", "" ) );
    m_Inputs.Add( NameValData( "BoundaryGeomID", "" ) );
    m_Inputs.Add( NameValData( "DirectionGeomID", "" ) );

    m_Inputs.Add( NameValData( "Direction", vec3d( 1.0, 0.0, 0.0 ) ) );
}


string ProjectionAnalysis::Execute()
{
    NameValData *nvd = NULL;

    int targetType = vsp::SET_TARGET;
    nvd = m_Inputs.FindPtr( "TargetType", 0 );
    if ( nvd )
    {
        targetType = nvd->GetInt( 0 );
    }

    int boundaryType = vsp::NO_BOUNDARY;
    nvd = m_Inputs.FindPtr( "BoundaryType", 0 );
    if ( nvd )
    {
        boundaryType = nvd->GetInt( 0 );
    }

    int directionType = vsp::X_PROJ;
    nvd = m_Inputs.FindPtr( "DirectionType", 0 );
    if ( nvd )
    {
        directionType = nvd->GetInt( 0 );
    }

    int targetSet = 0;
    nvd = m_Inputs.FindPtr( "TargetSet", 0 );
    if ( nvd )
    {
        targetSet = nvd->GetInt( 0 );
    }

    int boundarySet = 0;
    nvd = m_Inputs.FindPtr( "BoundarySet", 0 );
    if ( nvd )
    {
        boundarySet = nvd->GetInt( 0 );
    }

    string targetGeomID = "";
    nvd = m_Inputs.FindPtr( "TargetGeomID", 0 );
    if ( nvd )
    {
        targetGeomID = nvd->GetString( 0 );
    }

    string boundaryGeomID = "";
    nvd = m_Inputs.FindPtr( "BoundaryGeomID", 0 );
    if ( nvd )
    {
        boundaryGeomID = nvd->GetString( 0 );
    }

    string directionGeomID = "";
    nvd = m_Inputs.FindPtr( "DirectionGeomID", 0 );
    if ( nvd )
    {
        directionGeomID = nvd->GetString( 0 );
    }

    vec3d dir = vec3d( 1.0, 0.0, 0.0 );
    nvd = m_Inputs.FindPtr( "Direction", 0 );
    if ( nvd )
    {
        dir = nvd->GetVec3d( 0 );
    }

    if ( directionType != vsp::VEC_PROJ)
    {
        dir = ProjectionMgr.GetDirection( directionType, directionGeomID );
    }

    Results* res;

    switch ( boundaryType )
    {
        case vsp::NO_BOUNDARY:
            if ( targetType == vsp::SET_TARGET )
            {
                res = ProjectionMgr.Project( targetSet, dir );
            }
            else
            {
                res = ProjectionMgr.Project( targetGeomID, dir );
            }
            break;
        case vsp::SET_BOUNDARY:
            if ( targetType == vsp::SET_TARGET )
            {
                res = ProjectionMgr.Project( targetSet, boundarySet, dir );
            }
            else
            {
                res = ProjectionMgr.Project( targetGeomID, boundarySet, dir );
            }
            break;
        case vsp::GEOM_BOUNDARY:
            if ( targetType == vsp::SET_TARGET )
            {
                res = ProjectionMgr.Project( targetSet, boundaryGeomID, dir );
            }
            else
            {
                res = ProjectionMgr.Project( targetGeomID, boundaryGeomID, dir );
            }
            break;
    }

    return res->GetID();
}
