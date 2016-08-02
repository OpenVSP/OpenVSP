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
#include "PropGeom.h"

#include "VSPAEROMgr.h"

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
    BEMAnalysis *bem = new BEMAnalysis();

    RegisterAnalysis( "BladeElement", bem );

    CompGeomAnalysis *cga = new CompGeomAnalysis();

    RegisterAnalysis( "CompGeom", cga );


    EmintonLordAnalysis *ema = new EmintonLordAnalysis();

    RegisterAnalysis( "EmintonLord", ema );


    MassPropAnalysis *mpa = new MassPropAnalysis();

    RegisterAnalysis( "MassProp", mpa );


    PlanarSliceAnalysis *psa = new PlanarSliceAnalysis();

    RegisterAnalysis( "PlanarSlice", psa );


    ProjectionAnalysis *proj = new ProjectionAnalysis();

    RegisterAnalysis( "Projection", proj );


    WaveDragAnalysis *wave = new WaveDragAnalysis();

    RegisterAnalysis( "WaveDrag", wave );


    VSPAERODegenGeomAnalysis *vsadga = new VSPAERODegenGeomAnalysis();

    RegisterAnalysis( "VSPAERODegenGeom", vsadga );


    VSPAEROSinglePointAnalysis *vsaspa = new VSPAEROSinglePointAnalysis();

    RegisterAnalysis( "VSPAEROSinglePoint", vsaspa );


    VSPAEROSweepAnalysis *vsasa = new VSPAEROSweepAnalysis();

    RegisterAnalysis( "VSPAEROSweep", vsasa );
}

//======================================================================================//
//======================================= BEM ==========================================//
//======================================================================================//

void BEMAnalysis::SetDefaults()
{
    m_Inputs.Clear();

    Vehicle *veh = VehicleMgr.GetVehicle();

    if ( veh )
    {
        m_Inputs.Add( NameValData( "PropID", veh->m_BEMPropID ) );
    }
}

string BEMAnalysis::Execute()
{
    string res;

    Vehicle *veh = VehicleMgr.GetVehicle();

    if ( veh )
    {
        string propid;

        NameValData *nvd = NULL;

        nvd = m_Inputs.FindPtr( "PropID", 0 );
        if ( nvd )
        {
            propid = nvd->GetString( 0 );
        }

        Geom* geom = veh->FindGeom( propid );

        if ( geom )
        {
        	PropGeom* pgeom = dynamic_cast < PropGeom* > ( geom );
        	if ( pgeom )
        	{
        		res = pgeom->BuildBEMResults();
        	}
        }
    }

    return res;
}

//======================================================================================//
//================================= CompGeom ==========================================//
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
//=============================== Emington Lord Analysis ===============================//
//======================================================================================//

void EmintonLordAnalysis::SetDefaults()
{
    m_Inputs.Clear();
    vector < double > x_vec;
    vector < double > area_vec;
    m_Inputs.Add( NameValData( "X_vec", x_vec ) );
    m_Inputs.Add( NameValData( "Area_vec", area_vec ) );
}

string EmintonLordAnalysis::Execute()
{
    Vehicle *veh = VehicleMgr.GetVehicle();

    if ( veh )
    {
        vector < double > x_vec;
        vector < double > area_vec;

        NameValData *nvd = NULL;

        nvd = m_Inputs.FindPtr( "X_vec", 0 );
        if ( nvd )
        {
            x_vec = nvd->GetDoubleData();
        }

        nvd = m_Inputs.FindPtr( "Area_vec", 0 );
        if ( nvd )
        {
            area_vec = nvd->GetDoubleData();
        }

        int n = x_vec.size();
        vector < double > coeff(n-2);

        double Donq = WaveDragMgr.WaveDrag( x_vec, area_vec, coeff );

        Results* res = ResultsMgr.CreateResults( "EmintonLord" );
        if ( res )
        {
            res->Add( NameValData( "Donq", Donq ) );
            res->Add( NameValData( "Coeff", coeff ) );
            return res->GetID();
        }
    }

    return string();
}

//======================================================================================//
//================================= Mass Properties ====================================//
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
//================================= Planar Slice =======================================//
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

//======================================================================================//
//================================= Wave Drag ==========================================//
//======================================================================================//

void WaveDragAnalysis::SetDefaults()
{
    m_Inputs.Clear();
    m_Inputs.Add( NameValData( "Set", WaveDragMgr.m_SelectedSetIndex.Get() ) );
    m_Inputs.Add( NameValData( "NumSlices", WaveDragMgr.m_NumSlices.Get() ) );
    m_Inputs.Add( NameValData( "NumRotSects", WaveDragMgr.m_NumRotSects.Get() ) );
    m_Inputs.Add( NameValData( "Mach", WaveDragMgr.m_MachNumber.Get() ) );
    m_Inputs.Add( NameValData( "SSFlow_vec", WaveDragMgr.m_SSFlow_vec ) );
    m_Inputs.Add( NameValData( "SymmFlag", WaveDragMgr.m_SymmFlag.Get() ) );
}

string WaveDragAnalysis::Execute()
{
    string res;

    Vehicle *veh = VehicleMgr.GetVehicle();

    if ( veh )
    {
        int set;
        int numSlices;
        int numRots;
        double Mach;
        vector <string> Flow_vec;
        bool Symm;

        NameValData *nvd = NULL;

        nvd = m_Inputs.FindPtr( "Set", 0 );
        if ( nvd )
        {
            set = nvd->GetInt( 0 );
        }

        nvd = m_Inputs.FindPtr( "NumSlices", 0 );
        if ( nvd )
        {
            numSlices = nvd->GetInt( 0 );
        }

        nvd = m_Inputs.FindPtr( "NumRotSects", 0 );
        if ( nvd )
        {
            numRots = nvd->GetInt( 0 );
        }

        nvd = m_Inputs.FindPtr( "Mach", 0 );
        if ( nvd )
        {
            Mach = nvd->GetDouble( 0 );
        }

        nvd = m_Inputs.FindPtr( "SSFlow_vec", 0 );
        if ( nvd )
        {
            Flow_vec = nvd->GetStringData();
        }

        nvd = m_Inputs.FindPtr( "SymmFlag", 0 );
        if ( nvd )
        {
            Symm = nvd->GetInt( 0 );
        }

        res = WaveDragMgr.SliceAndAnalyze( set, numSlices, numRots, Mach, Flow_vec, Symm );
    }

    return res;
}


//======================================================================================//
//================================= VSPAERO ============================================//
//======================================================================================//

void VSPAERODegenGeomAnalysis::SetDefaults()
{
    // the default values use exactly what is setup in the VSPAEROMgr
    m_Inputs.Clear();
    Vehicle *veh = VehicleMgr.GetVehicle();
    if ( veh )
    {
        m_Inputs.Add( NameValData( "GeomSet", VSPAEROMgr.m_GeomSet.Get() ) );
    }
    else
    {
        // TODO Throw an error here
        printf("ERROR: trying to set defaults without a vehicle \n\tFile: %s \tLine:%d\n",__FILE__,__LINE__);
    }
}

string VSPAERODegenGeomAnalysis::Execute()
{
    string res_id;
    Vehicle *veh = VehicleMgr.GetVehicle();

    if ( veh )
    {
        NameValData *nvd = NULL;

        // Apply current analysis input values
        nvd = m_Inputs.FindPtr( "GeomSet", 0 );
        int geomSetOrig;
        if ( nvd )
        {
            geomSetOrig = VSPAEROMgr.m_GeomSet.Get();
            VSPAEROMgr.m_GeomSet.Set( nvd->GetInt( 0 ) );
        }

        // Execute analysis
        res_id = VSPAEROMgr.ComputeGeometry();

        //Restore original values that were overwritten by analysis inputs
        VSPAEROMgr.m_GeomSet.Set( geomSetOrig );

    }
    
    return res_id;
}


void VSPAEROSinglePointAnalysis::SetDefaults()
{
    // the default values use exactly what is setup in the VSPAEROMgr
    m_Inputs.Clear();
    Vehicle *veh = VehicleMgr.GetVehicle();
    if ( veh )
    {

        //Case Setup
        m_Inputs.Add( NameValData( "GeomSet",           VSPAEROMgr.m_GeomSet.Get()           ) );
        m_Inputs.Add( NameValData( "NCPU",              VSPAEROMgr.m_NCPU.Get()              ) );
        m_Inputs.Add( NameValData( "WakeNumIter",       VSPAEROMgr.m_WakeNumIter.Get()       ) );
        m_Inputs.Add( NameValData( "WakeAvgStartIter",  VSPAEROMgr.m_WakeAvgStartIter.Get()  ) );
        m_Inputs.Add( NameValData( "WakeSkipUntilIter", VSPAEROMgr.m_WakeSkipUntilIter.Get() ) );
        m_Inputs.Add( NameValData( "StabilityCalcFlag", VSPAEROMgr.m_StabilityCalcFlag.Get() ) );

        //Reference area, lengths
        m_Inputs.Add( NameValData( "RefFlag",           VSPAEROMgr.m_RefFlag.Get()           ) );
        m_Inputs.Add( NameValData( "Sref",              VSPAEROMgr.m_Sref.Get()              ) );
        m_Inputs.Add( NameValData( "bref",              VSPAEROMgr.m_bref.Get()              ) );
        m_Inputs.Add( NameValData( "cref",              VSPAEROMgr.m_cref.Get()              ) );

        //Moment center
        //TODO add flag to indentify if this is manual or computed
        m_Inputs.Add( NameValData( "CGGeomSet",           VSPAEROMgr.m_CGGeomSet.Get()         ) );
        m_Inputs.Add( NameValData( "NumMassSlice",      VSPAEROMgr.m_NumMassSlice.Get()      ) );
        m_Inputs.Add( NameValData( "Xcg",               VSPAEROMgr.m_Xcg.Get()               ) );
        m_Inputs.Add( NameValData( "Ycg",               VSPAEROMgr.m_Ycg.Get()               ) );
        m_Inputs.Add( NameValData( "Zcg",               VSPAEROMgr.m_Zcg.Get()               ) );

        //Input parameters (only uses *Start value.  Execute() sets *Npts to 1 to run a single point calculation)
        m_Inputs.Add( NameValData( "Alpha",             VSPAEROMgr.m_AlphaStart.Get()        ) );
        m_Inputs.Add( NameValData( "Beta",              VSPAEROMgr.m_BetaStart.Get()         ) );
        m_Inputs.Add( NameValData( "Mach",              VSPAEROMgr.m_MachStart.Get()         ) );

    }
    else
    {
        // TODO Throw an error here
        printf("ERROR: trying to set defaults without a vehicle \n\tFile: %s \tLine:%d\n",__FILE__,__LINE__);
    }
}

string VSPAEROSinglePointAnalysis::Execute()
{
    string resId;
    Vehicle *veh = VehicleMgr.GetVehicle();

    if ( veh )
    {

        NameValData *nvd = NULL;

        //==== Apply current analysis input values ====//
        //    Geometry set
        int geomSetOrig    = VSPAEROMgr.m_GeomSet.Get();
        nvd = m_Inputs.FindPtr( "GeomSet", 0 );
        VSPAEROMgr.m_GeomSet.Set( nvd->GetInt(0) );

        //    Regerence area, length parameters
        int refFlagOrig    = VSPAEROMgr.m_RefFlag.Get();
        double srefOrig    = VSPAEROMgr.m_Sref.Get();
        double brefOrig    = VSPAEROMgr.m_bref.Get();
        double crefOrig    = VSPAEROMgr.m_cref.Get();
        nvd = m_Inputs.FindPtr( "RefFlag", 0 );
        VSPAEROMgr.m_RefFlag.Set( nvd->GetInt(0) );
        nvd = m_Inputs.FindPtr( "Sref", 0 );
        VSPAEROMgr.m_Sref.Set( nvd->GetDouble(0) );
        nvd = m_Inputs.FindPtr( "bref", 0 );
        VSPAEROMgr.m_bref.Set( nvd->GetDouble(0) );
        nvd = m_Inputs.FindPtr( "cref", 0 );
        VSPAEROMgr.m_cref.Set( nvd->GetDouble(0) );

        //    Mass properties
        int cgGeomSetOrig    = VSPAEROMgr.m_CGGeomSet.Get();
        int numMassSliceOrig = VSPAEROMgr.m_NumMassSlice.Get();
        double xcgOrig        = VSPAEROMgr.m_Xcg.Get();
        double ycgOrig        = VSPAEROMgr.m_Ycg.Get();
        double zcgOrig        = VSPAEROMgr.m_Zcg.Get();
        nvd = m_Inputs.FindPtr( "CGGeomSet", 0 );
        VSPAEROMgr.m_CGGeomSet.Set( nvd->GetInt(0) );
        nvd = m_Inputs.FindPtr( "NumMassSlice", 0 );
        VSPAEROMgr.m_NumMassSlice.Set( nvd->GetInt(0) );
        nvd = m_Inputs.FindPtr( "Xcg", 0 );
        VSPAEROMgr.m_Xcg.Set( nvd->GetDouble(0) );
        nvd = m_Inputs.FindPtr( "Ycg", 0 );
        VSPAEROMgr.m_Ycg.Set( nvd->GetDouble(0) );
        nvd = m_Inputs.FindPtr( "Zcg", 0 );
        VSPAEROMgr.m_Zcg.Set( nvd->GetDouble(0) );

        //    Freestream parameters (Alpha, Beta, Mach)
        double alphaOrig      = VSPAEROMgr.m_AlphaStart.Get();
        int    alphaNptsOrig = VSPAEROMgr.m_AlphaNpts.Get();        // note this is NOT an input
        double betaOrig       = VSPAEROMgr.m_BetaStart.Get();
        int    betaNptsOrig  = VSPAEROMgr.m_BetaNpts.Get();        // note this is NOT an input
        double machOrig       = VSPAEROMgr.m_MachStart.Get();
        int    machNptsOrig  = VSPAEROMgr.m_MachNpts.Get();        // note this is NOT an input
        nvd = m_Inputs.FindPtr( "Alpha", 0 );
        VSPAEROMgr.m_AlphaStart.Set( nvd->GetDouble(0) );
        VSPAEROMgr.m_AlphaNpts.Set( 1 );                    // note: this is NOT an input
        nvd = m_Inputs.FindPtr( "Beta", 0 );
        VSPAEROMgr.m_BetaStart.Set( nvd->GetDouble(0) );
        VSPAEROMgr.m_BetaNpts.Set( 1 );                    // note: this is NOT an input
        nvd = m_Inputs.FindPtr( "Mach", 0 );
        VSPAEROMgr.m_MachStart.Set( nvd->GetDouble(0) );
        VSPAEROMgr.m_MachNpts.Set( 1 );                    // note: this is NOT an input

        //Case Setup
        int ncpuOrig                 = VSPAEROMgr.m_NCPU.Get();
        int wakeNumIterOrig          = VSPAEROMgr.m_WakeNumIter.Get();
        int wakeAvgStartIterOrig     = VSPAEROMgr.m_WakeAvgStartIter.Get();
        int wakeSkipUntilIterOrig    = VSPAEROMgr.m_WakeSkipUntilIter.Get();
        bool stabilityCalcFlagOrig   = VSPAEROMgr.m_StabilityCalcFlag.Get();
        nvd = m_Inputs.FindPtr( "NCPU", 0 );
        VSPAEROMgr.m_NCPU.Set( nvd->GetInt(0) );
        nvd = m_Inputs.FindPtr( "WakeNumIter" );
        VSPAEROMgr.m_WakeNumIter.Set( nvd->GetInt(0) );
        nvd = m_Inputs.FindPtr( "WakeAvgStartIter" );
        VSPAEROMgr.m_WakeAvgStartIter.Set( nvd->GetInt(0) );
        nvd = m_Inputs.FindPtr( "WakeSkipUntilIter" );
        VSPAEROMgr.m_WakeSkipUntilIter.Set( nvd->GetInt(0) );
        nvd = m_Inputs.FindPtr( "StabilityCalcFlag", 0 );
        VSPAEROMgr.m_StabilityCalcFlag.Set( nvd->GetInt(0) );

        //==== Execute Analysis ====//
        resId = VSPAEROMgr.ComputeSolver(stdout);


        //==== Restore Original Values ====//
        //    Geometry set
        VSPAEROMgr.m_GeomSet.Set( geomSetOrig );

        //    Regerence area, length parameters
        VSPAEROMgr.m_RefFlag.Set( refFlagOrig );
        VSPAEROMgr.m_Sref.Set( srefOrig );
        VSPAEROMgr.m_bref.Set( brefOrig );
        VSPAEROMgr.m_cref.Set( crefOrig );

        //    Mass properties
        VSPAEROMgr.m_CGGeomSet.Set(cgGeomSetOrig);
        VSPAEROMgr.m_NumMassSlice.Set(numMassSliceOrig);
        VSPAEROMgr.m_Xcg.Set(xcgOrig);
        VSPAEROMgr.m_Ycg.Set(ycgOrig);
        VSPAEROMgr.m_Zcg.Set(zcgOrig);

        //    Freestream parameters (Alpha, Beta, Mach)
        VSPAEROMgr.m_AlphaStart.Set(alphaOrig);
        VSPAEROMgr.m_AlphaNpts.Set(alphaNptsOrig);        // note this is NOT an input
        VSPAEROMgr.m_BetaStart.Set(betaOrig);
        VSPAEROMgr.m_BetaNpts.Set(betaNptsOrig);        // note this is NOT an input
        VSPAEROMgr.m_MachStart.Set(machOrig);
        VSPAEROMgr.m_MachNpts.Set(machNptsOrig);        // note this is NOT an input

        //    Case Setup
        VSPAEROMgr.m_NCPU.Set( ncpuOrig );
        VSPAEROMgr.m_WakeNumIter.Set( wakeNumIterOrig );
        VSPAEROMgr.m_WakeAvgStartIter.Set( wakeAvgStartIterOrig );
        VSPAEROMgr.m_WakeSkipUntilIter.Set( wakeSkipUntilIterOrig );
        VSPAEROMgr.m_StabilityCalcFlag.Set( stabilityCalcFlagOrig );

    }

    return resId;
}

void VSPAEROSweepAnalysis::SetDefaults()
{
    // the default values use exactly what is setup in the VSPAEROMgr
    m_Inputs.Clear();
    Vehicle *veh = VehicleMgr.GetVehicle();
    if ( veh )
    {

        //Case Setup
        m_Inputs.Add( NameValData( "GeomSet",           VSPAEROMgr.m_GeomSet.Get()           ) );
        m_Inputs.Add( NameValData( "NCPU",              VSPAEROMgr.m_NCPU.Get()              ) );
        m_Inputs.Add( NameValData( "WakeNumIter",       VSPAEROMgr.m_WakeNumIter.Get()       ) );
        m_Inputs.Add( NameValData( "WakeAvgStartIter",  VSPAEROMgr.m_WakeAvgStartIter.Get()  ) );
        m_Inputs.Add( NameValData( "WakeSkipUntilIter", VSPAEROMgr.m_WakeSkipUntilIter.Get() ) );
        m_Inputs.Add( NameValData( "StabilityCalcFlag", VSPAEROMgr.m_StabilityCalcFlag.Get() ) );

        //Reference area, lengths
        m_Inputs.Add( NameValData( "RefFlag",           VSPAEROMgr.m_RefFlag.Get()           ) );
        m_Inputs.Add( NameValData( "Sref",              VSPAEROMgr.m_Sref.Get()              ) );
        m_Inputs.Add( NameValData( "bref",              VSPAEROMgr.m_bref.Get()              ) );
        m_Inputs.Add( NameValData( "cref",              VSPAEROMgr.m_cref.Get()              ) );

        //Moment center
        //TODO add flag to indentify if this is manual or computed
        m_Inputs.Add( NameValData( "CGGeomSet",           VSPAEROMgr.m_CGGeomSet.Get()         ) );
        m_Inputs.Add( NameValData( "NumMassSlice",      VSPAEROMgr.m_NumMassSlice.Get()      ) );
        m_Inputs.Add( NameValData( "Xcg",               VSPAEROMgr.m_Xcg.Get()               ) );
        m_Inputs.Add( NameValData( "Ycg",               VSPAEROMgr.m_Ycg.Get()               ) );
        m_Inputs.Add( NameValData( "Zcg",               VSPAEROMgr.m_Zcg.Get()               ) );

        //Flow Condition
        m_Inputs.Add( NameValData( "AlphaStart",        VSPAEROMgr.m_AlphaStart.Get()        ) );
        m_Inputs.Add( NameValData( "AlphaEnd",          VSPAEROMgr.m_AlphaEnd.Get()          ) );
        m_Inputs.Add( NameValData( "AlphaNpts",         VSPAEROMgr.m_AlphaNpts.Get()         ) );
        m_Inputs.Add( NameValData( "BetaStart",         VSPAEROMgr.m_BetaStart.Get()         ) );
        m_Inputs.Add( NameValData( "BetaEnd",           VSPAEROMgr.m_BetaEnd.Get()           ) );
        m_Inputs.Add( NameValData( "BetaNpts",          VSPAEROMgr.m_BetaNpts.Get()          ) );
        m_Inputs.Add( NameValData( "MachStart",         VSPAEROMgr.m_MachStart.Get()         ) );
        m_Inputs.Add( NameValData( "MachEnd",           VSPAEROMgr.m_MachEnd.Get()           ) );
        m_Inputs.Add( NameValData( "MachNpts",          VSPAEROMgr.m_MachNpts.Get()          ) );

    }
    else
    {
        // TODO Throw an error here
        printf("ERROR: trying to set defaults without a vehicle \n\tFile: %s \tLine:%d\n",__FILE__,__LINE__);
    }
}

string VSPAEROSweepAnalysis::Execute()
{
    string resId;
    Vehicle *veh = VehicleMgr.GetVehicle();

    if ( veh )
    {

        NameValData *nvd = NULL;

        //==== Apply current analysis input values ====//
        //    Geometry set
        int geomSetOrig    = VSPAEROMgr.m_GeomSet.Get();
        nvd = m_Inputs.FindPtr( "GeomSet", 0 );
        VSPAEROMgr.m_GeomSet.Set( nvd->GetInt(0) );

        //    Regerence area, length parameters
        int refFlagOrig    = VSPAEROMgr.m_RefFlag.Get();
        double srefOrig    = VSPAEROMgr.m_Sref.Get();
        double brefOrig    = VSPAEROMgr.m_bref.Get();
        double crefOrig    = VSPAEROMgr.m_cref.Get();
        nvd = m_Inputs.FindPtr( "RefFlag", 0 );
        VSPAEROMgr.m_RefFlag.Set( nvd->GetInt(0) );
        nvd = m_Inputs.FindPtr( "Sref", 0 );
        VSPAEROMgr.m_Sref.Set( nvd->GetDouble(0) );
        nvd = m_Inputs.FindPtr( "bref", 0 );
        VSPAEROMgr.m_bref.Set( nvd->GetDouble(0) );
        nvd = m_Inputs.FindPtr( "cref", 0 );
        VSPAEROMgr.m_cref.Set( nvd->GetDouble(0) );

        //    Mass properties
        int cgGeomSetOrig    = VSPAEROMgr.m_CGGeomSet.Get();
        int numMassSliceOrig = VSPAEROMgr.m_NumMassSlice.Get();
        double xcgOrig        = VSPAEROMgr.m_Xcg.Get();
        double ycgOrig        = VSPAEROMgr.m_Ycg.Get();
        double zcgOrig        = VSPAEROMgr.m_Zcg.Get();
        nvd = m_Inputs.FindPtr( "CGGeomSet", 0 );
        VSPAEROMgr.m_CGGeomSet.Set( nvd->GetInt(0) );
        nvd = m_Inputs.FindPtr( "NumMassSlice", 0 );
        VSPAEROMgr.m_NumMassSlice.Set( nvd->GetInt(0) );
        nvd = m_Inputs.FindPtr( "Xcg", 0 );
        VSPAEROMgr.m_Xcg.Set( nvd->GetDouble(0) );
        nvd = m_Inputs.FindPtr( "Ycg", 0 );
        VSPAEROMgr.m_Ycg.Set( nvd->GetDouble(0) );
        nvd = m_Inputs.FindPtr( "Zcg", 0 );
        VSPAEROMgr.m_Zcg.Set( nvd->GetDouble(0) );

        //    Freestream parameters (Alpha, Beta, Mach)
        double alphaStartOrig    = VSPAEROMgr.m_AlphaStart.Get();
        double alphaEndOrig    = VSPAEROMgr.m_AlphaEnd.Get();
        int alphaNptsOrig        = VSPAEROMgr.m_AlphaNpts.Get();
        double betaStartOrig    = VSPAEROMgr.m_BetaStart.Get();
        double betaEndOrig    = VSPAEROMgr.m_BetaEnd.Get();
        int betaNptsOrig        = VSPAEROMgr.m_BetaNpts.Get();
        double machStartOrig    = VSPAEROMgr.m_MachStart.Get();
        double machEndOrig    = VSPAEROMgr.m_MachEnd.Get();
        int machNptsOrig        = VSPAEROMgr.m_MachNpts.Get();
        nvd = m_Inputs.FindPtr( "AlphaStart", 0 );
        VSPAEROMgr.m_AlphaStart.Set( nvd->GetDouble(0) );
        nvd = m_Inputs.FindPtr( "AlphaEnd", 0 );
        VSPAEROMgr.m_AlphaEnd.Set( nvd->GetDouble(0) );
        nvd = m_Inputs.FindPtr( "AlphaNpts", 0 );
        VSPAEROMgr.m_AlphaNpts.Set( nvd->GetInt(0) );
        nvd = m_Inputs.FindPtr( "BetaStart", 0 );
        VSPAEROMgr.m_BetaStart.Set( nvd->GetDouble(0) );
        nvd = m_Inputs.FindPtr( "BetaEnd", 0 );
        VSPAEROMgr.m_BetaEnd.Set( nvd->GetDouble(0) );
        nvd = m_Inputs.FindPtr( "BetaNpts", 0 );
        VSPAEROMgr.m_BetaNpts.Set( nvd->GetInt(0) );
        nvd = m_Inputs.FindPtr( "MachStart", 0 );
        VSPAEROMgr.m_MachStart.Set( nvd->GetDouble(0) );
        nvd = m_Inputs.FindPtr( "MachEnd", 0 );
        VSPAEROMgr.m_MachEnd.Set( nvd->GetDouble(0) );
        nvd = m_Inputs.FindPtr( "MachNpts", 0 );
        VSPAEROMgr.m_MachNpts.Set( nvd->GetInt(0) );

        //Case Setup
        int ncpuOrig                 = VSPAEROMgr.m_NCPU.Get();
        int wakeNumIterOrig          = VSPAEROMgr.m_WakeNumIter.Get();
        int wakeAvgStartIterOrig     = VSPAEROMgr.m_WakeAvgStartIter.Get();
        int wakeSkipUntilIterOrig    = VSPAEROMgr.m_WakeSkipUntilIter.Get();
        bool stabilityCalcFlagOrig   = VSPAEROMgr.m_StabilityCalcFlag.Get(); // note: this is NOT an input
        nvd = m_Inputs.FindPtr( "NCPU", 0 );
        VSPAEROMgr.m_NCPU.Set( nvd->GetInt(0) );
        nvd = m_Inputs.FindPtr( "WakeNumIter" );
        VSPAEROMgr.m_WakeNumIter.Set( nvd->GetInt(0) );
        nvd = m_Inputs.FindPtr( "WakeAvgStartIter" );
        VSPAEROMgr.m_WakeAvgStartIter.Set( nvd->GetInt(0) );
        nvd = m_Inputs.FindPtr( "WakeSkipUntilIter" );
        VSPAEROMgr.m_WakeSkipUntilIter.Set( nvd->GetInt(0) );
        nvd = m_Inputs.FindPtr( "StabilityCalcFlag", 0 );
        VSPAEROMgr.m_StabilityCalcFlag.Set( nvd->GetInt(0) );


        //==== Execute Analysis ====//
        resId = VSPAEROMgr.ComputeSolver(stdout);


        //==== Restore Original Values ====//
        //    Geometry set
        VSPAEROMgr.m_GeomSet.Set( geomSetOrig );

        //    Regerence area, length parameters
        VSPAEROMgr.m_RefFlag.Set( refFlagOrig );
        VSPAEROMgr.m_Sref.Set( srefOrig );
        VSPAEROMgr.m_bref.Set( brefOrig );
        VSPAEROMgr.m_cref.Set( crefOrig );

        //    Mass properties
        VSPAEROMgr.m_CGGeomSet.Set(cgGeomSetOrig);
        VSPAEROMgr.m_NumMassSlice.Set(numMassSliceOrig);
        VSPAEROMgr.m_Xcg.Set(xcgOrig);
        VSPAEROMgr.m_Ycg.Set(ycgOrig);
        VSPAEROMgr.m_Zcg.Set(zcgOrig);

        //    Freestream parameters (Alpha, Beta, Mach)
        VSPAEROMgr.m_AlphaStart.Set(alphaStartOrig);
        VSPAEROMgr.m_AlphaStart.Set(alphaEndOrig);
        VSPAEROMgr.m_AlphaNpts.Set(alphaNptsOrig);
        VSPAEROMgr.m_BetaStart.Set(betaStartOrig);
        VSPAEROMgr.m_BetaStart.Set(betaEndOrig);
        VSPAEROMgr.m_BetaNpts.Set(betaNptsOrig);
        VSPAEROMgr.m_MachStart.Set(machStartOrig);
        VSPAEROMgr.m_MachStart.Set(machEndOrig);
        VSPAEROMgr.m_MachNpts.Set(machNptsOrig);

        //    Case Setup
        VSPAEROMgr.m_NCPU.Set( ncpuOrig );
        VSPAEROMgr.m_WakeNumIter.Set( wakeNumIterOrig );
        VSPAEROMgr.m_WakeAvgStartIter.Set( wakeAvgStartIterOrig );
        VSPAEROMgr.m_WakeSkipUntilIter.Set( wakeSkipUntilIterOrig );
        VSPAEROMgr.m_StabilityCalcFlag.Set( stabilityCalcFlagOrig );

    }

    return resId;
}
