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
#include "ProjectionMgr.h"
#include "PropGeom.h"

#include "VSPAEROMgr.h"
#include "ParasiteDragMgr.h"

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

void AnalysisMgrSingleton::PrintAnalysisInputs( const string &fname, const string analysis_name )
{
    FILE *fp;
    fp = fopen( fname.c_str(), "w" );
    if ( fp )
    {
        PrintAnalysisInputs( fp, analysis_name );
        fclose( fp );
    }
}

void AnalysisMgrSingleton::PrintAnalysisInputs( const string analysis_name )
{
    PrintAnalysisInputs( stdout, analysis_name );
}

void AnalysisMgrSingleton::PrintAnalysisInputs( FILE * outputStream, const string analysis_name )
{
    fprintf( outputStream, "\t\t%-20s%s\t%s\t%s\n", "[input_name] ", "[type]", "[#]", "[current values-->]" );

    Analysis* analysis_ptr = FindAnalysis( analysis_name );
    if ( !analysis_ptr )
    {
        return;
    }
    vector < string > input_names = analysis_ptr->m_Inputs.GetAllDataNames();
    for ( unsigned int i_input_name = 0; i_input_name < input_names.size(); i_input_name++ )
    {
        // print out type and number of data entries
        int current_input_type = GetAnalysisInputType( analysis_name, input_names[i_input_name] );
        unsigned int current_input_num_data = ( unsigned int ) GetNumInputData( analysis_name, input_names[i_input_name] );
        fprintf( outputStream, "\t\t%-20s%d\t\t%d", input_names[i_input_name].c_str(), current_input_type, current_input_num_data );

        // print out the current value (this needs to handle different types and vector lengths
        fprintf( outputStream, "\t" );
        for ( unsigned int i_val = 0; i_val < current_input_num_data; i_val++ )
        {
            switch( current_input_type )
            {
            case vsp::INT_DATA :
            {
                vector<int> current_int_val = GetIntInputData( analysis_name, input_names[i_input_name], i_val );
                for ( unsigned int j_val = 0; j_val < current_int_val.size(); j_val++ )
                {
                    fprintf( outputStream, "%d ", current_int_val[j_val] );
                }
                break;
            }
            case vsp::DOUBLE_DATA :
            {
                vector<double> current_double_val = GetDoubleInputData( analysis_name, input_names[i_input_name], i_val );
                for ( unsigned int j_val = 0; j_val < current_double_val.size(); j_val++ )
                {
                    fprintf( outputStream, "%f ", current_double_val[j_val] );
                }
                break;
            }
            case vsp::STRING_DATA :
            {
                vector<string> current_string_val = GetStringInputData( analysis_name, input_names[i_input_name], i_val );
                for ( unsigned int j_val = 0; j_val < current_string_val.size(); j_val++ )
                {
                    fprintf( outputStream, "%s ", current_string_val[j_val].c_str() );
                }
                break;
            }
            case vsp::VEC3D_DATA :
            {
                vector<vec3d> current_vec3d_val = GetVec3dInputData( analysis_name, input_names[i_input_name], i_val );
                for ( unsigned int j_val = 0; j_val < current_vec3d_val.size(); j_val++ )
                {
                    fprintf( outputStream, "%f,%f,%f ", current_vec3d_val[j_val].x(), current_vec3d_val[j_val].y(), current_vec3d_val[j_val].z() );
                }
                break;
            }
            }    //end switch
        }    // end for

        fprintf( outputStream, "\n" );
    }
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


    VSPAEROComputeGeometryAnalysis *vsaga = new VSPAEROComputeGeometryAnalysis();

    RegisterAnalysis( "VSPAEROComputeGeometry", vsaga );


    VSPAEROSinglePointAnalysis *vsaspa = new VSPAEROSinglePointAnalysis();

    RegisterAnalysis( "VSPAEROSinglePoint", vsaspa );


    VSPAEROSweepAnalysis *vsasa = new VSPAEROSweepAnalysis();

    RegisterAnalysis( "VSPAEROSweep", vsasa );


    ParasiteDragFullAnalysis *vspdbu = new ParasiteDragFullAnalysis();

    RegisterAnalysis( "ParasiteDrag", vspdbu );
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
        int geomSet = 0;
        int halfMeshFlag = 0;
        int subSurfFlag = 1;

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
        int geomSet = 0;
        int numMassSlice = 20;

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
        int geomSet = 0;
        int numSlice = 10;
        vec3d axis( 1.0, 0.0, 0.0 );
        bool autobnd = true;
        double start = 0.0, end = 10.0;

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

    Results* res = NULL;

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

    if ( !res )
    {
        return string();
    }
    else
    {
        return res->GetID();
    }
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
        int set = WaveDragMgr.m_SelectedSetIndex.Get();
        int numSlices = WaveDragMgr.m_NumSlices.Get();
        int numRots = WaveDragMgr.m_NumRotSects.Get();
        double Mach = WaveDragMgr.m_MachNumber.Get();
        vector <string> Flow_vec = WaveDragMgr.m_SSFlow_vec;
        bool Symm = WaveDragMgr.m_SymmFlag.Get();

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
        int geomSetOrig = VSPAEROMgr.m_GeomSet.Get();
        if ( nvd )
        {
            VSPAEROMgr.m_GeomSet.Set( nvd->GetInt( 0 ) );
        }

        // Execute analysis
        res_id = VSPAEROMgr.ComputeGeometry();

        //Restore original values that were overwritten by analysis inputs
        VSPAEROMgr.m_GeomSet.Set( geomSetOrig );

    }
    
    return res_id;
}

void VSPAEROComputeGeometryAnalysis::SetDefaults()
{
    // the default values use exactly what is setup in the VSPAEROMgr
    m_Inputs.Clear();
    Vehicle *veh = VehicleMgr.GetVehicle();
    if ( veh )
    {
        m_Inputs.Add( NameValData( "GeomSet", VSPAEROMgr.m_GeomSet.Get() ) );
        m_Inputs.Add( NameValData( "AnalysisMethod", VSPAEROMgr.m_AnalysisMethod.Get() ) );
    }
    else
    {
        // TODO Throw an error here
        printf("ERROR - trying to set defaults without a vehicle: void VSPAERODegenGeomAnalysis::SetDefaults()\n");
    }
}

string VSPAEROComputeGeometryAnalysis::Execute()
{
    string resId;
    Vehicle *veh = VehicleMgr.GetVehicle();

    if ( veh )
    {
        NameValData *nvd = NULL;

        //==== Apply current analysis input values ====//
        int geomSetOrig    = VSPAEROMgr.m_GeomSet.Get();
        nvd = m_Inputs.FindPtr( "GeomSet", 0 );
        VSPAEROMgr.m_GeomSet.Set( nvd->GetInt(0) );

        int analysisMethodOrig = VSPAEROMgr.m_AnalysisMethod.Get();
        nvd = m_Inputs.FindPtr( "AnalysisMethod", 0 );
        VSPAEROMgr.m_AnalysisMethod.Set( nvd->GetInt( 0 ) );

        //==== Execute Analysis ====//
        resId = VSPAEROMgr.ComputeGeometry();

        //==== Restore Original Values ====//
        VSPAEROMgr.m_GeomSet.Set( geomSetOrig );
        VSPAEROMgr.m_AnalysisMethod.Set( analysisMethodOrig );

    }
    
    return resId;
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
        m_Inputs.Add( NameValData( "AnalysisMethod",    VSPAEROMgr.m_AnalysisMethod.Get()    ) );
        m_Inputs.Add( NameValData( "NCPU",              VSPAEROMgr.m_NCPU.Get()              ) );
        m_Inputs.Add( NameValData( "WakeNumIter",       VSPAEROMgr.m_WakeNumIter.Get()       ) );
        m_Inputs.Add( NameValData( "WakeAvgStartIter",  VSPAEROMgr.m_WakeAvgStartIter.Get()  ) );
        m_Inputs.Add( NameValData( "WakeSkipUntilIter", VSPAEROMgr.m_WakeSkipUntilIter.Get() ) );
        m_Inputs.Add( NameValData( "StabilityCalcFlag", VSPAEROMgr.m_StabilityCalcFlag.Get() ) );

        //Reference area, lengths
        m_Inputs.Add( NameValData( "RefFlag",           VSPAEROMgr.m_RefFlag.Get()           ) );
        m_Inputs.Add( NameValData( "WingID",            " "                                  ) );
        m_Inputs.Add( NameValData( "Sref",              VSPAEROMgr.m_Sref.Get()              ) );
        m_Inputs.Add( NameValData( "bref",              VSPAEROMgr.m_bref.Get()              ) );
        m_Inputs.Add( NameValData( "cref",              VSPAEROMgr.m_cref.Get()              ) );

        //Moment center
        //TODO add flag to indentify if this is manual or computed
        m_Inputs.Add( NameValData( "CGGeomSet",         VSPAEROMgr.m_CGGeomSet.Get()         ) );
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
        if ( nvd )
        {
            VSPAEROMgr.m_GeomSet.Set( nvd->GetInt(0) );
        }

        int analysisMethodOrig = VSPAEROMgr.m_AnalysisMethod.Get();
        nvd = m_Inputs.FindPtr( "AnalysisMethod", 0 );
        VSPAEROMgr.m_AnalysisMethod.Set( nvd->GetInt( 0 ) );

        //    Regerence area, length parameters
        int refFlagOrig    = VSPAEROMgr.m_RefFlag.Get();
        string WingIDOrig    = VSPAEROMgr.m_RefGeomID;
        double srefOrig    = VSPAEROMgr.m_Sref.Get();
        double brefOrig    = VSPAEROMgr.m_bref.Get();
        double crefOrig    = VSPAEROMgr.m_cref.Get();
        nvd = m_Inputs.FindPtr( "RefFlag", 0 );
        if ( nvd )
        {
            VSPAEROMgr.m_RefFlag.Set( nvd->GetInt(0) );
        }
        nvd = m_Inputs.FindPtr( "WingID", 0 );
        if ( nvd )
        {
            VSPAEROMgr.m_RefGeomID = nvd->GetString(0);
        }

        if ( VSPAEROMgr.m_RefFlag.Get() == vsp::MANUAL_REF )
        {
            nvd = m_Inputs.FindPtr( "Sref", 0 );
            if ( nvd )
            {
                VSPAEROMgr.m_Sref.Set( nvd->GetDouble(0) );
            }
            nvd = m_Inputs.FindPtr( "bref", 0 );
            if ( nvd )
            {
                VSPAEROMgr.m_bref.Set( nvd->GetDouble(0) );
            }
            nvd = m_Inputs.FindPtr( "cref", 0 );
            if ( nvd )
            {
                VSPAEROMgr.m_cref.Set( nvd->GetDouble(0) );
            }
        }
        else if ( VSPAEROMgr.m_RefFlag.Get() == vsp::COMPONENT_REF )
        {
            VSPAEROMgr.Update();
            printf( "Wing Reference Parms: \n" );

            nvd = m_Inputs.FindPtr( "Sref", 0 );
            if ( nvd )
            {
                VSPAEROMgr.m_Sref.Set( VSPAEROMgr.m_Sref.Get() );
            }
            printf( " Sref: %7.3f \n", VSPAEROMgr.m_Sref.Get() );
            nvd = m_Inputs.FindPtr( "bref", 0 );
            if ( nvd )
            {
                VSPAEROMgr.m_bref.Set( VSPAEROMgr.m_bref.Get() );
            }
            printf( " bref: %7.3f \n", VSPAEROMgr.m_bref.Get() );
            nvd = m_Inputs.FindPtr( "cref", 0 );
            if ( nvd )
            {
                VSPAEROMgr.m_cref.Set( VSPAEROMgr.m_cref.Get() );
            }
            printf( " cref: %7.3f \n", VSPAEROMgr.m_cref.Get() );

            printf( "\n" );
        }

        //    Mass properties
        int cgGeomSetOrig    = VSPAEROMgr.m_CGGeomSet.Get();
        int numMassSliceOrig = VSPAEROMgr.m_NumMassSlice.Get();
        double xcgOrig        = VSPAEROMgr.m_Xcg.Get();
        double ycgOrig        = VSPAEROMgr.m_Ycg.Get();
        double zcgOrig        = VSPAEROMgr.m_Zcg.Get();
        nvd = m_Inputs.FindPtr( "CGGeomSet", 0 );
        if ( nvd )
        {
            VSPAEROMgr.m_CGGeomSet.Set( nvd->GetInt(0) );
        }
        nvd = m_Inputs.FindPtr( "NumMassSlice", 0 );
        if ( nvd )
        {
            VSPAEROMgr.m_NumMassSlice.Set( nvd->GetInt(0) );
        }
        nvd = m_Inputs.FindPtr( "Xcg", 0 );
        if ( nvd )
        {
            VSPAEROMgr.m_Xcg.Set( nvd->GetDouble(0) );
        }
        nvd = m_Inputs.FindPtr( "Ycg", 0 );
        if ( nvd )
        {
            VSPAEROMgr.m_Ycg.Set( nvd->GetDouble(0) );
        }
        nvd = m_Inputs.FindPtr( "Zcg", 0 );
        if ( nvd )
        {
            VSPAEROMgr.m_Zcg.Set( nvd->GetDouble(0) );
        }

        //    Freestream parameters (Alpha, Beta, Mach)
        double alphaOrig      = VSPAEROMgr.m_AlphaStart.Get();
        int    alphaNptsOrig  = VSPAEROMgr.m_AlphaNpts.Get();        // note this is NOT an input
        double betaOrig       = VSPAEROMgr.m_BetaStart.Get();
        int    betaNptsOrig   = VSPAEROMgr.m_BetaNpts.Get();        // note this is NOT an input
        double machOrig       = VSPAEROMgr.m_MachStart.Get();
        int    machNptsOrig   = VSPAEROMgr.m_MachNpts.Get();        // note this is NOT an input
        nvd = m_Inputs.FindPtr( "Alpha", 0 );
        if ( nvd )
        {
            VSPAEROMgr.m_AlphaStart.Set( nvd->GetDouble(0) );
        }
        VSPAEROMgr.m_AlphaNpts.Set( 1 );                    // note: this is NOT an input
        nvd = m_Inputs.FindPtr( "Beta", 0 );
        if ( nvd )
        {
            VSPAEROMgr.m_BetaStart.Set( nvd->GetDouble(0) );
        }
        VSPAEROMgr.m_BetaNpts.Set( 1 );                    // note: this is NOT an input
        nvd = m_Inputs.FindPtr( "Mach", 0 );
        if ( nvd )
        {
            VSPAEROMgr.m_MachStart.Set( nvd->GetDouble(0) );
        }
        VSPAEROMgr.m_MachNpts.Set( 1 );                    // note: this is NOT an input

        //Case Setup
        int ncpuOrig                 = VSPAEROMgr.m_NCPU.Get();
        int wakeNumIterOrig          = VSPAEROMgr.m_WakeNumIter.Get();
        int wakeAvgStartIterOrig     = VSPAEROMgr.m_WakeAvgStartIter.Get();
        int wakeSkipUntilIterOrig    = VSPAEROMgr.m_WakeSkipUntilIter.Get();
        bool stabilityCalcFlagOrig   = VSPAEROMgr.m_StabilityCalcFlag.Get();
        nvd = m_Inputs.FindPtr( "NCPU", 0 );
        if ( nvd )
        {
            VSPAEROMgr.m_NCPU.Set( nvd->GetInt(0) );
        }
        nvd = m_Inputs.FindPtr( "WakeNumIter" );
        if ( nvd )
        {
            VSPAEROMgr.m_WakeNumIter.Set( nvd->GetInt(0) );
        }
        nvd = m_Inputs.FindPtr( "WakeAvgStartIter" );
        if ( nvd )
        {
            VSPAEROMgr.m_WakeAvgStartIter.Set( nvd->GetInt(0) );
        }
        nvd = m_Inputs.FindPtr( "WakeSkipUntilIter" );
        if ( nvd )
        {
            VSPAEROMgr.m_WakeSkipUntilIter.Set( nvd->GetInt(0) );
        }
        nvd = m_Inputs.FindPtr( "StabilityCalcFlag", 0 );
        if ( nvd )
        {
            VSPAEROMgr.m_StabilityCalcFlag.Set( nvd->GetInt(0) );
        }


        //==== Execute Analysis ====//
        resId = VSPAEROMgr.ComputeSolver(stdout);


        //==== Restore Original Values ====//
        //    Geometry set
        VSPAEROMgr.m_GeomSet.Set( geomSetOrig );
        VSPAEROMgr.m_AnalysisMethod.Set( analysisMethodOrig );

        //    Regerence area, length parameters
        VSPAEROMgr.m_RefFlag.Set( refFlagOrig );
        VSPAEROMgr.m_RefGeomID = WingIDOrig;
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
        m_Inputs.Add( NameValData( "AnalysisMethod",    VSPAEROMgr.m_AnalysisMethod.Get()    ) );
        m_Inputs.Add( NameValData( "NCPU",              VSPAEROMgr.m_NCPU.Get()              ) );
        m_Inputs.Add( NameValData( "WakeNumIter",       VSPAEROMgr.m_WakeNumIter.Get()       ) );
        m_Inputs.Add( NameValData( "WakeAvgStartIter",  VSPAEROMgr.m_WakeAvgStartIter.Get()  ) );
        m_Inputs.Add( NameValData( "WakeSkipUntilIter", VSPAEROMgr.m_WakeSkipUntilIter.Get() ) );
        m_Inputs.Add( NameValData( "StabilityCalcFlag", VSPAEROMgr.m_StabilityCalcFlag.Get() ) );
        m_Inputs.Add( NameValData( "BatchModeFlag",     VSPAEROMgr.m_BatchModeFlag.Get()     ) );
        m_Inputs.Add( NameValData( "Symmetry",          VSPAEROMgr.m_Symmetry.Get()          ) );
        m_Inputs.Add( NameValData( "2DFEMFlag",         VSPAEROMgr.m_Write2DFEMFlag.Get()    ) );

        //Reference area, lengths
        m_Inputs.Add( NameValData( "RefFlag",           VSPAEROMgr.m_RefFlag.Get()           ) );
        m_Inputs.Add( NameValData( "WingID",            " "                                  ) );
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
        if ( nvd )
        {
            VSPAEROMgr.m_GeomSet.Set( nvd->GetInt(0) );
        }

        int analysisMethodOrig = VSPAEROMgr.m_AnalysisMethod.Get();
        nvd = m_Inputs.FindPtr( "AnalysisMethod", 0 );
        VSPAEROMgr.m_AnalysisMethod.Set( nvd->GetInt( 0 ) );

        //    Regerence area, length parameters
        int refFlagOrig    = VSPAEROMgr.m_RefFlag.Get();
        string WingIDOrig  = VSPAEROMgr.m_RefGeomID;
        double srefOrig    = VSPAEROMgr.m_Sref.Get();
        double brefOrig    = VSPAEROMgr.m_bref.Get();
        double crefOrig    = VSPAEROMgr.m_cref.Get();
        nvd = m_Inputs.FindPtr( "RefFlag", 0 );
        if ( nvd )
        {
            VSPAEROMgr.m_RefFlag.Set( nvd->GetInt(0) );
        }
        nvd = m_Inputs.FindPtr( "WingID", 0 );
        if ( nvd )
        {
            VSPAEROMgr.m_RefGeomID = nvd->GetString(0);
        }

        if ( VSPAEROMgr.m_RefFlag.Get() == vsp::MANUAL_REF )
        {
            nvd = m_Inputs.FindPtr( "Sref", 0 );
            if ( nvd )
            {
                VSPAEROMgr.m_Sref.Set( nvd->GetDouble(0) );
            }
            nvd = m_Inputs.FindPtr( "bref", 0 );
            if ( nvd )
            {
                VSPAEROMgr.m_bref.Set( nvd->GetDouble(0) );
            }
            nvd = m_Inputs.FindPtr( "cref", 0 );
            if ( nvd )
            {
                VSPAEROMgr.m_cref.Set( nvd->GetDouble(0) );
            }
        }
        else if ( VSPAEROMgr.m_RefFlag.Get() == vsp::COMPONENT_REF )
        {
            VSPAEROMgr.Update();
            printf( "Wing Reference Parms: \n" );

            nvd = m_Inputs.FindPtr( "Sref", 0 );
            if ( nvd )
            {
                VSPAEROMgr.m_Sref.Set( VSPAEROMgr.m_Sref.Get() );
            }
            printf( " Sref: %7.3f \n", VSPAEROMgr.m_Sref.Get() );
            nvd = m_Inputs.FindPtr( "bref", 0 );
            if ( nvd )
            {
                VSPAEROMgr.m_bref.Set( VSPAEROMgr.m_bref.Get() );
            }
            printf( " bref: %7.3f \n", VSPAEROMgr.m_bref.Get() );
            nvd = m_Inputs.FindPtr( "cref", 0 );
            if ( nvd )
            {
                VSPAEROMgr.m_cref.Set( VSPAEROMgr.m_cref.Get() );
            }
            printf( " cref: %7.3f \n", VSPAEROMgr.m_cref.Get() );

            printf( "\n" );
        }

        //    Mass properties
        int cgGeomSetOrig    = VSPAEROMgr.m_CGGeomSet.Get();
        int numMassSliceOrig = VSPAEROMgr.m_NumMassSlice.Get();
        double xcgOrig        = VSPAEROMgr.m_Xcg.Get();
        double ycgOrig        = VSPAEROMgr.m_Ycg.Get();
        double zcgOrig        = VSPAEROMgr.m_Zcg.Get();
        nvd = m_Inputs.FindPtr( "CGGeomSet", 0 );
        if ( nvd )
        {
            VSPAEROMgr.m_CGGeomSet.Set( nvd->GetInt(0) );
        }
        nvd = m_Inputs.FindPtr( "NumMassSlice", 0 );
        if ( nvd )
        {
            VSPAEROMgr.m_NumMassSlice.Set( nvd->GetInt(0) );
        }
        nvd = m_Inputs.FindPtr( "Xcg", 0 );
        if ( nvd )
        {
            VSPAEROMgr.m_Xcg.Set( nvd->GetDouble(0) );
        }
        nvd = m_Inputs.FindPtr( "Ycg", 0 );
        if ( nvd )
        {
            VSPAEROMgr.m_Ycg.Set( nvd->GetDouble(0) );
        }
        nvd = m_Inputs.FindPtr( "Zcg", 0 );
        if ( nvd )
        {
            VSPAEROMgr.m_Zcg.Set( nvd->GetDouble(0) );
        }

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
        if ( nvd )
        {
            VSPAEROMgr.m_AlphaStart.Set( nvd->GetDouble(0) );
        }
        nvd = m_Inputs.FindPtr( "AlphaEnd", 0 );
        if ( nvd )
        {
            VSPAEROMgr.m_AlphaEnd.Set( nvd->GetDouble(0) );
        }
        nvd = m_Inputs.FindPtr( "AlphaNpts", 0 );
        if ( nvd )
        {
            VSPAEROMgr.m_AlphaNpts.Set( nvd->GetInt(0) );
        }
        nvd = m_Inputs.FindPtr( "BetaStart", 0 );
        if ( nvd )
        {
            VSPAEROMgr.m_BetaStart.Set( nvd->GetDouble(0) );
        }
        nvd = m_Inputs.FindPtr( "BetaEnd", 0 );
        if ( nvd )
        {
            VSPAEROMgr.m_BetaEnd.Set( nvd->GetDouble(0) );
        }
        nvd = m_Inputs.FindPtr( "BetaNpts", 0 );
        if ( nvd )
        {
            VSPAEROMgr.m_BetaNpts.Set( nvd->GetInt(0) );
        }
        nvd = m_Inputs.FindPtr( "MachStart", 0 );
        if ( nvd )
        {
            VSPAEROMgr.m_MachStart.Set( nvd->GetDouble(0) );
        }
        nvd = m_Inputs.FindPtr( "MachEnd", 0 );
        if ( nvd )
        {
            VSPAEROMgr.m_MachEnd.Set( nvd->GetDouble(0) );
        }
        nvd = m_Inputs.FindPtr( "MachNpts", 0 );
        if ( nvd )
        {
            VSPAEROMgr.m_MachNpts.Set( nvd->GetInt(0) );
        }

        //Case Setup
        int ncpuOrig                 = VSPAEROMgr.m_NCPU.Get();
        int wakeNumIterOrig          = VSPAEROMgr.m_WakeNumIter.Get();
        int wakeAvgStartIterOrig     = VSPAEROMgr.m_WakeAvgStartIter.Get();
        int wakeSkipUntilIterOrig    = VSPAEROMgr.m_WakeSkipUntilIter.Get();
        bool stabilityCalcFlagOrig   = VSPAEROMgr.m_StabilityCalcFlag.Get();
        nvd = m_Inputs.FindPtr( "NCPU", 0 );
        if ( nvd )
        {
            VSPAEROMgr.m_NCPU.Set( nvd->GetInt(0) );
        }
        nvd = m_Inputs.FindPtr( "WakeNumIter" );
        if ( nvd )
        {
            VSPAEROMgr.m_WakeNumIter.Set( nvd->GetInt(0) );
        }
        nvd = m_Inputs.FindPtr( "WakeAvgStartIter" );
        if ( nvd )
        {
            VSPAEROMgr.m_WakeAvgStartIter.Set( nvd->GetInt(0) );
        }

        nvd = m_Inputs.FindPtr( "WakeSkipUntilIter" );
        if ( nvd )
        {
            VSPAEROMgr.m_WakeSkipUntilIter.Set( nvd->GetInt(0) );
        }
        nvd = m_Inputs.FindPtr( "StabilityCalcFlag", 0 );
        if ( nvd )
        {
            VSPAEROMgr.m_StabilityCalcFlag.Set( nvd->GetInt(0) );
        }

        bool BatchModeFlagOrig       = VSPAEROMgr.m_BatchModeFlag.Get();
        nvd = m_Inputs.FindPtr( "BatchModeFlag", 0 );
        if ( nvd )
        {
            VSPAEROMgr.m_BatchModeFlag.Set( nvd->GetInt(0) );
        }

        bool symmetryOrig   = VSPAEROMgr.m_Symmetry.Get();
        nvd = m_Inputs.FindPtr( "Symmetry", 0 );
        VSPAEROMgr.m_Symmetry.Set( nvd->GetInt(0) );

        bool write2DFEMOrig   = VSPAEROMgr.m_Write2DFEMFlag.Get();
        nvd = m_Inputs.FindPtr( "2DFEMFlag", 0 );
        VSPAEROMgr.m_Write2DFEMFlag.Set( nvd->GetInt(0) );

        //==== Execute Analysis ====//
        resId = VSPAEROMgr.ComputeSolver(stdout);


        //==== Restore Original Values ====//
        //    Geometry set
        VSPAEROMgr.m_GeomSet.Set( geomSetOrig );
        VSPAEROMgr.m_AnalysisMethod.Set( analysisMethodOrig );

        //    Regerence area, length parameters
        VSPAEROMgr.m_RefFlag.Set( refFlagOrig );
        VSPAEROMgr.m_RefGeomID = WingIDOrig;
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

        VSPAEROMgr.m_BatchModeFlag.Set( BatchModeFlagOrig );

        VSPAEROMgr.m_Symmetry.Set( symmetryOrig );
        VSPAEROMgr.m_Write2DFEMFlag.Set( write2DFEMOrig );
    }

    return resId;
}

//======================================================================================//
//============================== Parasite Drag =========================================//
//======================================================================================//

void ParasiteDragFullAnalysis::SetDefaults()
{
    // the default values use exactly what is setup in the VSPAEROMgr
    m_Inputs.Clear();
    Vehicle *veh = VehicleMgr.GetVehicle();
    if ( veh )
    {
        // File Name
        m_Inputs.Add( NameValData( "FileName",          ParasiteDragMgr.m_FileName ) );

        // Geometry to Degen
        m_Inputs.Add( NameValData( "GeomSet",           ParasiteDragMgr.m_SetChoice.Get() ) );

        // Friction Coefficient Eqn Choice
        m_Inputs.Add( NameValData( "LamCfEqnChoice",    ParasiteDragMgr.m_LamCfEqnType.Get() ) );
        m_Inputs.Add( NameValData( "TurbCfEqnChoice",   ParasiteDragMgr.m_TurbCfEqnType.Get() ) );

        // Unit Choice
        m_Inputs.Add( NameValData( "LengthUnit",        ParasiteDragMgr.m_LengthUnit.Get() ) );
        m_Inputs.Add( NameValData( "VelocityUnit",      ParasiteDragMgr.m_VinfUnitType.Get() ) );
        m_Inputs.Add( NameValData( "TempUnit",          ParasiteDragMgr.m_TempUnit.Get() ) );

        // Freestream Props
        m_Inputs.Add( NameValData( "FreestreamPropChoice", ParasiteDragMgr.m_FreestreamType.Get() ) );
        m_Inputs.Add( NameValData( "Vinf",                 ParasiteDragMgr.m_Vinf.Get() ) );
        m_Inputs.Add( NameValData( "Altitude",             ParasiteDragMgr.m_Hinf.Get() ) );
        m_Inputs.Add( NameValData( "DeltaTemp",            ParasiteDragMgr.m_DeltaT.Get() ) );
        m_Inputs.Add( NameValData( "Temperature",          ParasiteDragMgr.m_Temp.Get() ) );
        m_Inputs.Add( NameValData( "Pressure",             ParasiteDragMgr.m_Pres.Get() ) );
        m_Inputs.Add( NameValData( "Density",              ParasiteDragMgr.m_Rho.Get() ) );
        //m_Inputs.Add( NameValData( "Medium",               ParasiteDragMgr.m_MediumType.Get() ) );

        // Reference Area
        m_Inputs.Add( NameValData( "RefFlag", ParasiteDragMgr.m_RefFlag.Get() ) );
        m_Inputs.Add( NameValData( "Sref",    ParasiteDragMgr.m_Sref.Get() ) );
    }
    else
    {
        // TODO Throw an error here
        printf("ERROR - trying to set defaults without a vehicle: void ParasiteDragFullAnalysis::SetDefaults()\n");
    }
}

string ParasiteDragFullAnalysis::Execute()
{
    string res_id;
    Vehicle *veh = VehicleMgr.GetVehicle();

    if ( veh )
    {
        NameValData *nvd = NULL;

        // File Name
        string fileNameOrig = ParasiteDragMgr.m_FileName;
        nvd = m_Inputs.FindPtr( "FileName", 0 );
        ParasiteDragMgr.m_FileName = nvd->GetString(0);

        // Geometry Set Choice
        int geomSetOrig = ParasiteDragMgr.m_SetChoice.Get();
        nvd = m_Inputs.FindPtr( "GeomSet", 0 );
        ParasiteDragMgr.m_SetChoice.Set( nvd->GetInt(0) );

        // Friction Coefficient Eqn Choice
        int lamCfEqnChoiceOrig = ParasiteDragMgr.m_LamCfEqnType.Get();
        int turbCfEqnChoiceOrig = ParasiteDragMgr.m_TurbCfEqnType.Get();
        nvd = m_Inputs.FindPtr( "LamCfEqnChoice", 0 );
        ParasiteDragMgr.m_LamCfEqnType.Set( nvd->GetInt(0) );
        nvd = m_Inputs.FindPtr( "TurbCfEqnChoice", 0 );
        ParasiteDragMgr.m_TurbCfEqnType.Set( nvd->GetInt(0) );

        // Unit Choice
        int lengthUnitChoiceOrig = ParasiteDragMgr.m_LengthUnit.Get();
        int velocityUnitChoiceOrig = ParasiteDragMgr.m_VinfUnitType.Get();
        int tempUnitChoiceOrig = ParasiteDragMgr.m_TempUnit.Get();
        nvd = m_Inputs.FindPtr( "LengthUnit", 0 );
        ParasiteDragMgr.m_LengthUnit.Set( nvd->GetInt(0) );
        nvd = m_Inputs.FindPtr( "VelocityUnit", 0 );
        ParasiteDragMgr.m_VinfUnitType.Set( nvd->GetInt(0) );
        nvd = m_Inputs.FindPtr( "TempUnit", 0 );
        ParasiteDragMgr.m_TempUnit.Set( nvd->GetInt(0) );

        // Freestream Props
        int freestreamPropChoiceOrig = ParasiteDragMgr.m_FreestreamType.Get();
        double vinfOrig = ParasiteDragMgr.m_Vinf.Get();
        double altOrig = ParasiteDragMgr.m_Hinf.Get();
        double deltaTempOrig = ParasiteDragMgr.m_DeltaT.Get();
        double tempOrig = ParasiteDragMgr.m_Temp.Get();
        double presOrig = ParasiteDragMgr.m_Pres.Get();
        double densOrig = ParasiteDragMgr.m_Rho.Get();
        //int mediumChoiceOrig = ParasiteDragMgr.m_MediumType.Get();
        nvd = m_Inputs.FindPtr( "FreestreamPropChoice", 0 );
        ParasiteDragMgr.m_FreestreamType.Set( nvd->GetInt(0) );
        nvd = m_Inputs.FindPtr( "Vinf", 0 );
        ParasiteDragMgr.m_Vinf.Set( nvd->GetDouble(0) );
        nvd = m_Inputs.FindPtr( "Altitude", 0 );
        ParasiteDragMgr.m_Hinf.Set( nvd->GetDouble(0) );
        nvd = m_Inputs.FindPtr( "DeltaTemp", 0 );
        ParasiteDragMgr.m_DeltaT.Set( nvd->GetDouble(0) );
        nvd = m_Inputs.FindPtr( "Temperature", 0 );
        ParasiteDragMgr.m_Temp.Set( nvd->GetDouble(0) );
        nvd = m_Inputs.FindPtr( "Pressure", 0 );
        ParasiteDragMgr.m_Pres.Set( nvd->GetDouble(0) );
        nvd = m_Inputs.FindPtr( "Density", 0 );
        ParasiteDragMgr.m_Rho.Set( nvd->GetDouble(0) );
        nvd = m_Inputs.FindPtr( "Medium", 0 );
        //ParasiteDragMgr.m_MediumType.Set( nvd->GetInt(0) );

        // Reference Area
        int refFlagOrig = ParasiteDragMgr.m_RefFlag.Get();
        double srefOrig = ParasiteDragMgr.m_Sref.Get();
        nvd = m_Inputs.FindPtr( "RefFlag", 0 );
        ParasiteDragMgr.m_RefFlag.Set( nvd->GetInt(0) );
        nvd = m_Inputs.FindPtr( "Sref", 0 );
        ParasiteDragMgr.m_Sref.Set( nvd->GetDouble(0) );

        // Execute analysis
        res_id = ParasiteDragMgr.ComputeBuildUp();

        // ==== Restore original values that were overwritten by analysis inputs ==== //
        // Geometry Set
        ParasiteDragMgr.m_SetChoice.Set( geomSetOrig );

        // Friction Coefficient Equations
        ParasiteDragMgr.m_LamCfEqnType.Set( lamCfEqnChoiceOrig );
        ParasiteDragMgr.m_TurbCfEqnType.Set( turbCfEqnChoiceOrig );

        // Unit Choices
        ParasiteDragMgr.m_LengthUnit.Set( lengthUnitChoiceOrig );
        ParasiteDragMgr.m_VinfUnitType.Set( velocityUnitChoiceOrig );
        ParasiteDragMgr.m_TempUnit.Set( tempUnitChoiceOrig );

        // Freestream Props
        ParasiteDragMgr.m_FreestreamType.Set( freestreamPropChoiceOrig );
        ParasiteDragMgr.m_Vinf.Set( vinfOrig );
        ParasiteDragMgr.m_Hinf.Set( altOrig );
        ParasiteDragMgr.m_DeltaT.Set( deltaTempOrig );
        ParasiteDragMgr.m_Temp.Set( tempOrig );
        ParasiteDragMgr.m_Pres.Set( presOrig );
        ParasiteDragMgr.m_Rho.Set( densOrig );
        //ParasiteDragMgr.m_MediumType.Set( mediumChoiceOrig );

        // Reference Area
        ParasiteDragMgr.m_RefFlag.Set( refFlagOrig );
        ParasiteDragMgr.m_Sref.Set( srefOrig );
    }

    return res_id;
}