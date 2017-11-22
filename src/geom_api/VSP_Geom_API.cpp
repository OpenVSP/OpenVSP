//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//

// API.h: interface for the Vehicle Class and Vehicle Mgr Singleton.
// J.R Gloudemans
//
//////////////////////////////////////////////////////////////////////

#include "VSP_Geom_API.h"
#include "VehicleMgr.h"
#include "Vehicle.h"
#include "ParmMgr.h"
#include "LinkMgr.h"
#include "AnalysisMgr.h"
#include "CfdMeshMgr.h"
#include "Util.h"
#include "DesignVarMgr.h"
#include "VarPresetMgr.h"
#include "ParasiteDragMgr.h"
#include "WingGeom.h"
#include "PropGeom.h"
#include "VSPAEROMgr.h"
#include "MeasureMgr.h"
#include "SubSurfaceMgr.h"

#ifdef VSP_USE_FLTK
#include "GuiInterface.h"
#endif

namespace vsp
{
//===================================================================//
//===============       Helper Functions            =================//
//===================================================================//
//  Get the pointer to Vehicle - this is a helper funtion for the other API
//  functions
Vehicle* GetVehicle()
{
    VSPCheckSetup();

    //==== Check For Valid Vehicle Ptr ====//
    Vehicle* veh = VehicleMgr.GetVehicle();
    if ( !veh )
    {
        ErrorMgr.AddError( VSP_INVALID_PTR, "GetVehicle::Invalid Vehicle Ptr"  );
        return veh;
    }
    return veh;
}

// Find the pointer to a XSecSurf given its id
XSecSurf* FindXSecSurf( const string & id )
{
    Vehicle* veh = GetVehicle();
 //   vector< Geom* > geom_vec = veh->FindGeomVec( veh->GetGeomVec() );
    vector< Geom* > geom_vec = veh->GetGeomStoreVec();

    for ( int i = 0 ; i < ( int )geom_vec.size() ; i++ )
    {
        Geom* gptr = geom_vec[i];
        for ( int j = 0 ; j < gptr->GetNumXSecSurfs() ; j++ )
        {
            XSecSurf* xsec_surf = gptr->GetXSecSurf( j );
            if ( xsec_surf && ( id == xsec_surf->GetID() ) )
            {
                return xsec_surf;
            }
        }
    }
    return NULL;
}

// Find the pointer to a XSec given its id
XSec* FindXSec( const string & id )
{
    ParmContainer* pc = ParmMgr.FindParmContainer( id );

    if ( !pc )
    {
        return NULL;
    }

    XSec* xs = dynamic_cast<XSec*>( pc );
    return xs;
}


//===================================================================//
//===============       API Functions               =================//
//===================================================================//



/// Check VehicleMgr for a valid vehicle pointer.  Create vehicle
/// pointer on first call.  There is a check to prevent multiple calls.
void VSPCheckSetup()
{
    //==== Make Sure Init is Only Called Once ===//
    static bool once = false;
    if ( once )
    {
        return;
    }
    once = true;

    //==== Check For Valid Vehicle Ptr ====//
    if ( !VehicleMgr.GetVehicle() )
    {
        ErrorMgr.AddError( VSP_INVALID_PTR, "VSPInit::Invalid Vehicle Ptr"  );
        exit( 0 );
    }

// Please dont do this - messes up the batch script mode
//#ifdef VSP_USE_FLTK
//    GuiInterface::getInstance().InitGui( GetVehicle() );
//#endif

    ErrorMgr.NoError();
}


void VSPRenew()
{
    Vehicle* veh = GetVehicle();
    veh->Renew();
    ErrorMgr.NoError();
}

void Update()
{
    Vehicle* veh = GetVehicle();
    veh->Update();
    ErrorMgr.NoError();
}


void VSPExit( int error_code )
{
    exit( error_code );
}



//===================================================================//
//===============       File I/O Functions        ===================//
//===================================================================//

void ReadVSPFile( const string & file_name )
{
    Vehicle* veh = GetVehicle();
    int err = veh->ReadXMLFile( file_name );
    if( err != 0 )
    {
        ErrorMgr.AddError( VSP_WRONG_FILE_TYPE, "ReadVSPFile::Error"  );
        return;
    }
    veh->SetVSP3FileName( file_name );
    ErrorMgr.NoError();
}

void WriteVSPFile( const string & file_name, int set )
{
    Vehicle* veh = GetVehicle();
    if( !veh->WriteXMLFile( file_name, set ) )
    {
        ErrorMgr.AddError( VSP_FILE_WRITE_FAILURE, "WriteVSPFile::Failure Writing File"  );
        return;
    }
    ErrorMgr.NoError();
}

void SetVSP3FileName( const string & file_name )
{
    Vehicle* veh = GetVehicle();
    if ( !veh )
    {
        ErrorMgr.AddError( VSP_INVALID_PTR, "SetVSP3FileName::Failure Getting Vehicle Ptr"  );
        return;
    }
    veh->SetVSP3FileName( file_name );
    ErrorMgr.NoError();
}

string GetVSPFileName()
{
    Vehicle* veh = GetVehicle();
    if ( !veh ) return string("NULL");

    ErrorMgr.NoError();
    return veh->GetVSP3FileName();
}

void ClearVSPModel()
{
    GetVehicle()->Renew();
    ErrorMgr.NoError();
}

void InsertVSPFile( const string & file_name, const string & parent  )
{
    Vehicle* veh = GetVehicle();

    Geom* parent_geom = NULL;
    if ( parent.size() > 0 )
    {
        parent_geom = veh->FindGeom( parent );
        if ( !parent_geom )
        {
            ErrorMgr.AddError( VSP_INVALID_GEOM_ID, "AddGeom::Can't Find Parent " + parent  );
        }
    }

    if ( parent_geom )
    {
        veh->SetActiveGeom( parent );
    }
    else
    {
        veh->ClearActiveGeom();
    }

    int err = veh->ReadXMLFileGeomsOnly( file_name );
    if( err != 0 )
    {
        ErrorMgr.AddError( VSP_WRONG_FILE_TYPE, "InsertVSPFile::Error"  );
        return;
    }
    ErrorMgr.NoError();
}

string ImportFile( const string & file_name, int file_type, const string & parent  )
{
    Vehicle* veh = GetVehicle();
    Geom* parent_geom = NULL;
    if ( parent.size() > 0 )
    {
        parent_geom = veh->FindGeom( parent );
        if ( !parent_geom )
        {
            ErrorMgr.AddError( VSP_INVALID_GEOM_ID, "AddGeom::Can't Find Parent " + parent  );
        }
    }

    if ( parent_geom )
    {
        veh->SetActiveGeom( parent );
    }
    else
    {
        veh->ClearActiveGeom();
    }

    ErrorMgr.NoError();
    return veh->ImportFile( file_name, file_type );
}

void ExportFile( const string & file_name, int write_set_index, int file_type )
{
    GetVehicle()->ExportFile( file_name, write_set_index, file_type );

    ErrorMgr.NoError();
}

//===================================================================//
//======================== Design Files =============================//
//===================================================================//
void ReadApplyDESFile( const string & file_name )
{
    DesignVarMgr.ReadDesVarsDES( file_name );
    ErrorMgr.NoError();
}

void WriteDESFile( const string & file_name )
{
    DesignVarMgr.WriteDesVarsDES( file_name );
    ErrorMgr.NoError();
}

void ReadApplyXDDMFile( const string & file_name )
{
    DesignVarMgr.ReadDesVarsXDDM( file_name );
    ErrorMgr.NoError();
}

void WriteXDDMFile( const string & file_name )
{
    DesignVarMgr.WriteDesVarsXDDM( file_name );
    ErrorMgr.NoError();
}

int GetNumDesignVars()
{
    int num = DesignVarMgr.GetNumVars();

    ErrorMgr.NoError();
    return num;
}

void AddDesignVar( const string & parm_id, int type )
{
    DesignVarMgr.AddVar( parm_id, type );
    ErrorMgr.NoError();
}

void DeleteAllDesignVars()
{
    DesignVarMgr.DelAllVars();
    ErrorMgr.NoError();
}

string GetDesignVar( int index )
{
    string parm_id;

    DesignVar* dv = DesignVarMgr.GetVar( index );

    if ( !dv )
    {
        ErrorMgr.AddError( VSP_INVALID_PTR, "GetDesignVar::Design variable out of range." );
        return parm_id;
    }

    parm_id = dv->m_ParmID;

    ErrorMgr.NoError();
    return parm_id;
}

int GetDesignVarType( int index )
{
    int dvtype = -1;

    DesignVar* dv = DesignVarMgr.GetVar( index );

    if ( !dv )
    {
        ErrorMgr.AddError( VSP_INVALID_PTR, "GetDesignVarType::Design variable out of range." );
        return dvtype;
    }

    dvtype = dv->m_XDDM_Type;

    ErrorMgr.NoError();
    return dvtype;
}

//===================================================================//
//===============      Computations               ===================//
//===================================================================//
/// Set Computation File Names
void SetComputationFileName( int file_type, const string & file_name )
{
    GetVehicle()->setExportFileName( file_type, file_name );

    if ( file_type == CFD_STL_TYPE )
        CfdMeshMgr.GetCfdSettingsPtr()->SetExportFileName( file_name, CFD_STL_FILE_NAME );
    if ( file_type == CFD_POLY_TYPE )
        CfdMeshMgr.GetCfdSettingsPtr()->SetExportFileName( file_name, CFD_POLY_FILE_NAME );
    if ( file_type == CFD_TRI_TYPE )
        CfdMeshMgr.GetCfdSettingsPtr()->SetExportFileName( file_name, CFD_TRI_FILE_NAME );
    if ( file_type == CFD_FACET_TYPE )
        CfdMeshMgr.GetCfdSettingsPtr()->SetExportFileName( file_name, CFD_FACET_FILE_NAME );
    if ( file_type == CFD_OBJ_TYPE )
        CfdMeshMgr.GetCfdSettingsPtr()->SetExportFileName( file_name, CFD_OBJ_FILE_NAME );
    if ( file_type == CFD_DAT_TYPE )
        CfdMeshMgr.GetCfdSettingsPtr()->SetExportFileName( file_name, CFD_DAT_FILE_NAME );
    if ( file_type == CFD_KEY_TYPE )
        CfdMeshMgr.GetCfdSettingsPtr()->SetExportFileName( file_name, CFD_KEY_FILE_NAME );
    if ( file_type == CFD_GMSH_TYPE )
        CfdMeshMgr.GetCfdSettingsPtr()->SetExportFileName( file_name, CFD_GMSH_FILE_NAME );
    if ( file_type == CFD_SRF_TYPE )
        CfdMeshMgr.GetCfdSettingsPtr()->SetExportFileName( file_name, CFD_SRF_FILE_NAME );
    if ( file_type == CFD_TKEY_TYPE )
        CfdMeshMgr.GetCfdSettingsPtr()->SetExportFileName( file_name, CFD_TKEY_FILE_NAME );

    ErrorMgr.NoError();
}

/// Compute Mass Properties on The Components in the Set
string ComputeMassProps( int set, int num_slices )
{
    Update();

    string id = GetVehicle()->MassPropsAndFlatten( set, num_slices );

    if ( id.size() == 0 )
    {
        ErrorMgr.AddError( VSP_INVALID_ID, "ComputeMassProps::Invalid ID " );
    }
    else
    {
        ErrorMgr.NoError();
    }

    return id;
}

/// Compute the Union and Wetted Surface Area and Volumes
string ComputeCompGeom( int set, bool half_mesh, int file_export_types )
{
    Update();
    Vehicle* veh = GetVehicle();

    veh->setExportCompGeomCsvFile( false );
    if ( file_export_types & COMP_GEOM_CSV_TYPE )
    {
        veh->setExportCompGeomCsvFile( true );
    }

    veh->setExportDragBuildTsvFile( false );
    if ( file_export_types & DRAG_BUILD_TSV_TYPE )
    {
        veh->setExportDragBuildTsvFile( true );
    }

    string id = veh->CompGeomAndFlatten( set, half_mesh );

    if ( id.size() == 0 )
    {
        ErrorMgr.AddError( VSP_INVALID_ID, "ComputeMassProps::Invalid ID " );
    }
    else
    {
        ErrorMgr.NoError();
    }

    return id;
}

/// Compute the Degenerate Geometry
void ComputeDegenGeom( int set, int file_export_types )
{
    Update();
    Vehicle* veh = GetVehicle();

    veh->setExportDegenGeomMFile( false );
    if ( file_export_types & DEGEN_GEOM_M_TYPE )
    {
        veh->setExportDegenGeomMFile( true );
    }

    veh->setExportDegenGeomCsvFile( false );
    if ( file_export_types & DEGEN_GEOM_CSV_TYPE )
    {
        veh->setExportDegenGeomCsvFile( true );
    }

    veh->CreateDegenGeom( set );
    veh->WriteDegenGeomFile();
    ErrorMgr.NoError();
}

//==== Compute Plane Slice =====//
string ComputePlaneSlice( int set, int num_slices, const vec3d & norm, bool auto_bnd, double start_bnd, double end_bnd )
{
    Update();
    Vehicle* veh = GetVehicle();

    string id = veh->PSliceAndFlatten( set, num_slices, norm, auto_bnd, start_bnd, end_bnd );

    if ( id.size() == 0 )
    {
        ErrorMgr.AddError( VSP_INVALID_ID, "ComputePlaneSlice::Invalid ID " );
    }
    else
    {
        ErrorMgr.NoError();
    }

    return id;
}

//==== Set a CFD Mesh Control Val =====//
void SetCFDMeshVal( int type, double val )
{
    if ( type == CFD_MIN_EDGE_LEN )
        CfdMeshMgr.GetGridDensityPtr()->m_MinLen = val;
    else if ( type == CFD_MAX_EDGE_LEN )
        CfdMeshMgr.GetGridDensityPtr()->m_BaseLen = val;
    else if ( type == CFD_MAX_GAP )
        CfdMeshMgr.GetGridDensityPtr()->m_MaxGap  = val;
    else if ( type == CFD_NUM_CIRCLE_SEGS )
        CfdMeshMgr.GetGridDensityPtr()->m_NCircSeg = val;
    else if ( type == CFD_GROWTH_RATIO )
        CfdMeshMgr.GetGridDensityPtr()->m_GrowRatio = val;
    else if ( type == CFD_LIMIT_GROWTH_FLAG )
        CfdMeshMgr.GetGridDensityPtr()->SetRigorLimit( ToBool(val) );
    else if ( type == CFD_INTERSECT_SUBSURFACE_FLAG )
        CfdMeshMgr.GetCfdSettingsPtr()->m_IntersectSubSurfs = ToBool(val);
    else if ( type == CFD_HALF_MESH_FLAG )
        CfdMeshMgr.GetCfdSettingsPtr()->SetHalfMeshFlag( ToBool(val) );
    else if ( type == CFD_FAR_FIELD_FLAG )
        CfdMeshMgr.GetCfdSettingsPtr()->SetFarMeshFlag( ToBool(val) );
    else if ( type == CFD_FAR_MAX_EDGE_LEN )
        CfdMeshMgr.GetGridDensityPtr()->m_FarMaxLen = val;
    else if ( type == CFD_FAR_MAX_GAP )
        CfdMeshMgr.GetGridDensityPtr()->m_FarMaxGap = val;
    else if ( type == CFD_FAR_NUM_CIRCLE_SEGS )
        CfdMeshMgr.GetGridDensityPtr()->m_FarNCircSeg = val;
    else if ( type == CFD_FAR_SIZE_ABS_FLAG )
        CfdMeshMgr.GetCfdSettingsPtr()->SetFarAbsSizeFlag( ToBool(val) );
    else if ( type == CFD_FAR_LENGTH )
        CfdMeshMgr.GetCfdSettingsPtr()->m_FarLength = val;
    else if ( type == CFD_FAR_WIDTH )
        CfdMeshMgr.GetCfdSettingsPtr()->m_FarWidth = val;
    else if ( type == CFD_FAR_HEIGHT )
        CfdMeshMgr.GetCfdSettingsPtr()->m_FarHeight = val;
    else if ( type == CFD_FAR_X_SCALE )
        CfdMeshMgr.GetCfdSettingsPtr()->m_FarXScale = val;
    else if ( type == CFD_FAR_Y_SCALE )
        CfdMeshMgr.GetCfdSettingsPtr()->m_FarYScale = val;
    else if ( type == CFD_FAR_Z_SCALE )
        CfdMeshMgr.GetCfdSettingsPtr()->m_FarZScale = val;
    else if ( type == CFD_FAR_LOC_MAN_FLAG )
        CfdMeshMgr.GetCfdSettingsPtr()->SetFarManLocFlag( ToBool(val) );
    else if ( type == CFD_FAR_LOC_X )
        CfdMeshMgr.GetCfdSettingsPtr()->m_FarXLocation = val;
    else if ( type == CFD_FAR_LOC_Y )
        CfdMeshMgr.GetCfdSettingsPtr()->m_FarYLocation = val;
    else if ( type == CFD_FAR_LOC_Z )
        CfdMeshMgr.GetCfdSettingsPtr()->m_FarZLocation = val;
    else if ( type == CFD_WAKE_SCALE )
        CfdMeshMgr.GetCfdSettingsPtr()->m_WakeScale = val;
    else if ( type == CFD_WAKE_ANGLE )
        CfdMeshMgr.GetCfdSettingsPtr()->m_WakeAngle = val;
    else if ( type == CFD_SRF_XYZ_FLAG )
        CfdMeshMgr.GetCfdSettingsPtr()->m_XYZIntCurveFlag = ToBool(val);
    else
    {
        ErrorMgr.AddError( VSP_CANT_FIND_TYPE, "SetCFDMeshVal::Can't Find Type " + to_string( ( long long )type ) );
        return;
    }

    ErrorMgr.NoError();
}
/// Turn On/Off Wakg For Component
void SetCFDWakeFlag( const string & geom_id, bool flag )
{
    Vehicle* veh = GetVehicle();
    Geom* geom_ptr = veh->FindGeom( geom_id );
    if ( !geom_ptr )
    {
        ErrorMgr.AddError( VSP_INVALID_PTR, "SetCFDWakeFlag::Can't Find Geom"  );
        return;
    }

    geom_ptr->SetWakeActiveFlag( flag );
    ErrorMgr.NoError();
}

/// Add A CFD Source
void AddCFDSource( int type, const string & geom_id, int surf_index,
                   double l1, double r1, double u1, double w1,
                   double l2, double r2, double u2, double w2 )
{
    Vehicle* veh = GetVehicle();
    Geom* geom_ptr = veh->FindGeom( geom_id );
    if ( !geom_ptr )
    {
        ErrorMgr.AddError( VSP_INVALID_PTR, "AddCFDSource::Can't Find Geom"  );
        return;
    }

    CfdMeshMgr.SetCurrSourceGeomID( geom_id );
    CfdMeshMgr.SetCurrMainSurfIndx( surf_index );
    BaseSource* source =  CfdMeshMgr.AddSource( type );

    if ( !source )
    {
        ErrorMgr.AddError( VSP_INVALID_PTR, "AddCFDSource::Can't Find Type"  );
        return;
    }

    source->m_Len = l1;
    source->m_Rad = r1;
    if ( source->GetType() == POINT_SOURCE )
    {
        PointSource* ps = dynamic_cast< PointSource* > (source);
        ps->m_ULoc = u1;
        ps->m_WLoc = w1;
    }
    else if ( source->GetType() == LINE_SOURCE )
    {
        LineSource* ls = dynamic_cast< LineSource* > (source);
        ls->m_Len2 = l2;
        ls->m_Rad2 = r2;
        ls->m_ULoc1 = u1;
        ls->m_WLoc1 = w1;
        ls->m_ULoc2 = u2;
        ls->m_WLoc2 = w2;
    }
    else if ( source->GetType() == BOX_SOURCE )
    {
        BoxSource* bs = dynamic_cast< BoxSource* > (source);
        bs->m_ULoc1 = u1;
        bs->m_WLoc1 = w1;
        bs->m_ULoc2 = u2;
        bs->m_WLoc2 = w2;
    }
    else if ( source->GetType() == ULINE_SOURCE )
    {
        ULineSource* bs = dynamic_cast< ULineSource* > (source);
        bs->m_Val = u1;
    }
    else if ( source->GetType() == WLINE_SOURCE )
    {
        WLineSource* bs = dynamic_cast< WLineSource* > (source);
        bs->m_Val = w1;
    }
    ErrorMgr.NoError();

}

/// Delete All CFD Sources
void DeleteAllCFDSources()
{
    CfdMeshMgr.DeleteAllSources();
    ErrorMgr.NoError();
}

/// Add Default Source To All Geometry
void AddDefaultSources()
{
    CfdMeshMgr.AddDefaultSources();
    ErrorMgr.NoError();
}

/// Compute the CFD Mesh
void ComputeCFDMesh( int set, int file_export_types )
{
    Update();
    Vehicle* veh = GetVehicle();

    CfdMeshMgr.GetCfdSettingsPtr()->SetAllFileExportFlags( false );

    if ( file_export_types & CFD_STL_TYPE )
        CfdMeshMgr.GetCfdSettingsPtr()->SetFileExportFlag( CFD_STL_FILE_NAME, true );
    if ( file_export_types & CFD_POLY_TYPE )
        CfdMeshMgr.GetCfdSettingsPtr()->SetFileExportFlag( CFD_POLY_FILE_NAME, true );
    if ( file_export_types & CFD_TRI_TYPE )
        CfdMeshMgr.GetCfdSettingsPtr()->SetFileExportFlag( CFD_TRI_FILE_NAME, true );
    if ( file_export_types & CFD_FACET_TYPE )
        CfdMeshMgr.GetCfdSettingsPtr()->SetFileExportFlag( CFD_FACET_FILE_NAME, true );
    if ( file_export_types & CFD_OBJ_TYPE )
        CfdMeshMgr.GetCfdSettingsPtr()->SetFileExportFlag( CFD_OBJ_FILE_NAME, true );
    if ( file_export_types & CFD_DAT_TYPE )
        CfdMeshMgr.GetCfdSettingsPtr()->SetFileExportFlag( CFD_DAT_FILE_NAME, true );
    if ( file_export_types & CFD_KEY_TYPE )
        CfdMeshMgr.GetCfdSettingsPtr()->SetFileExportFlag( CFD_KEY_FILE_NAME, true );
    if ( file_export_types & CFD_GMSH_TYPE )
        CfdMeshMgr.GetCfdSettingsPtr()->SetFileExportFlag( CFD_GMSH_FILE_NAME, true );
    if ( file_export_types & CFD_SRF_TYPE )
        CfdMeshMgr.GetCfdSettingsPtr()->SetFileExportFlag( CFD_SRF_FILE_NAME, true );
    if ( file_export_types & CFD_TKEY_TYPE )
        CfdMeshMgr.GetCfdSettingsPtr()->SetFileExportFlag( CFD_TKEY_FILE_NAME, true );

    CfdMeshMgr.GetCfdSettingsPtr()->m_SelectedSetIndex = set;
    CfdMeshMgr.GenerateMesh();
    ErrorMgr.NoError();
}

/// Get/Set reference wing
string GetVSPAERORefWingID()
{
    Vehicle* veh = GetVehicle();
    if ( !veh )
    {
        ErrorMgr.AddError( VSP_INVALID_PTR, "GetVSPAERORefWingID::Can't Find Vehicle" );
        return string();
    }

    if ( VSPAEROMgr.m_RefFlag.Get() != vsp::COMPONENT_REF )
    {
        return string();
    }

    Geom* geom_ptr = veh->FindGeom( VSPAEROMgr.m_RefGeomID );
    if ( !geom_ptr )
    {
        ErrorMgr.AddError( VSP_INVALID_PTR, "GetVSPAERORefWingID::Can't Find Geom" );
        return string();
    }

    return VSPAEROMgr.m_RefGeomID;
}

string SetVSPAERORefWingID( const string & geom_id )
{
    Vehicle* veh = GetVehicle();
    if (!veh)
    {
        ErrorMgr.AddError( VSP_INVALID_PTR, "SetVSPAERORefWingID::Can't Find Vehicle" );
        return string();
    }

    Geom* geom_ptr = veh->FindGeom( geom_id );
    if ( !geom_ptr )
    {
        ErrorMgr.AddError( VSP_INVALID_PTR, "SetVSPAERORefWingID::Can't Find Geom" );
        return  string();
    }

    VSPAEROMgr.m_RefGeomID = geom_id;
    VSPAEROMgr.m_RefFlag = vsp::COMPONENT_REF;

    ErrorMgr.NoError();

    return VSPAEROMgr.m_RefGeomID;
}

void AutoGroupVSPAEROControlSurfaces()
{
    VSPAEROMgr.Update();
    VSPAEROMgr.InitControlSurfaceGroups();
}

int GetNumControlSurfaceGroups()
{
    return VSPAEROMgr.GetControlSurfaceGroupVec().size();
}

//===================================================================//
//===============       Analysis Functions        ===================//
//===================================================================//

int GetNumAnalysis()
{
  return AnalysisMgr.GetNumAnalysis();
}

vector < string > ListAnalysis()
{
    return AnalysisMgr.ListAnalysis();
}

vector < string > GetAnalysisInputNames( const string & analysis )
{
    if ( !AnalysisMgr.ValidAnalysisName( analysis ) )
    {
        ErrorMgr.AddError( VSP_INVALID_ID, "GetAnalysisInputNames::Invalid Analysis ID " + analysis );
        vector < string > ret;
        return ret;
    }

    Analysis *a = AnalysisMgr.FindAnalysis( analysis );

    return a->m_Inputs.GetAllDataNames();
}

string ExecAnalysis( const string & analysis )
{
    if ( !AnalysisMgr.ValidAnalysisName( analysis ) )
    {
        ErrorMgr.AddError( VSP_INVALID_ID, "ExecAnalysis::Invalid Analysis ID " + analysis );
        string ret;
        return ret;
    }

    return AnalysisMgr.ExecAnalysis( analysis );
}

int GetNumAnalysisInputData( const string & analysis, const string & name )
{
    if ( !AnalysisMgr.ValidAnalysisName( analysis ) )
    {
        ErrorMgr.AddError( VSP_INVALID_ID, "GetNumAnalysisInputData::Invalid Analysis ID " + analysis );
        return 0;
    }
    ErrorMgr.NoError();

    return AnalysisMgr.GetNumInputData( analysis, name );
}

int GetAnalysisInputType( const string & analysis, const string & name )
{
    if ( !AnalysisMgr.ValidAnalysisName( analysis ) )
    {
        ErrorMgr.AddError( VSP_INVALID_ID, "GetAnalysisInputType::Invalid Analysis ID " + analysis );
        return vsp::INVALID_TYPE;
    }
    ErrorMgr.NoError();

    return AnalysisMgr.GetAnalysisInputType( analysis, name );
}

const vector< int > & GetIntAnalysisInput( const string & analysis, const string & name, int index )
{
    if ( !AnalysisMgr.ValidAnalysisName( analysis ) )
    {
        ErrorMgr.AddError( VSP_INVALID_ID, "GetIntAnalysisInput::Invalid Analysis ID " + analysis );
    }
    else if ( !AnalysisMgr.ValidAnalysisInputDataIndex( analysis, name, index ) )
    {
        ErrorMgr.AddError( VSP_CANT_FIND_NAME, "GetIntAnalysisInput::Can't Find Name " + name );
    }
    else
    {
        ErrorMgr.NoError();
    }

    return AnalysisMgr.GetIntInputData( analysis, name, index );
}

const std::vector< double > & GetDoubleAnalysisInput( const string & analysis, const string & name, int index )
{
    if ( !AnalysisMgr.ValidAnalysisName( analysis ) )
    {
        ErrorMgr.AddError( VSP_INVALID_ID, "GetDoubleAnalysisInput::Invalid Analysis ID " + analysis );
    }
    else if ( !AnalysisMgr.ValidAnalysisInputDataIndex( analysis, name, index ) )
    {
        ErrorMgr.AddError( VSP_CANT_FIND_NAME, "GetDoubleAnalysisInput::Can't Find Name " + name );
    }
    else
    {
        ErrorMgr.NoError();
    }

    return AnalysisMgr.GetDoubleInputData( analysis, name, index );
}

const std::vector< std::string > & GetStringAnalysisInput( const string & analysis, const string & name, int index )
{
    if ( !AnalysisMgr.ValidAnalysisName( analysis ) )
    {
        ErrorMgr.AddError( VSP_INVALID_ID, "GetStringAnalysisInput::Invalid Analysis ID " + analysis );
    }
    else if ( !AnalysisMgr.ValidAnalysisInputDataIndex( analysis, name, index ) )
    {
        ErrorMgr.AddError( VSP_CANT_FIND_NAME, "GetStringAnalysisInput::Can't Find Name " + name );
    }
    else
    {
        ErrorMgr.NoError();
    }

    return AnalysisMgr.GetStringInputData( analysis, name, index );
}

const std::vector< vec3d > & GetVec3dAnalysisInput( const string & analysis, const string & name, int index )
{
    if ( !AnalysisMgr.ValidAnalysisName( analysis ) )
    {
        ErrorMgr.AddError( VSP_INVALID_ID, "GetVec3dAnalysisInput::Invalid Analysis ID " + analysis );
    }
    else if ( !AnalysisMgr.ValidAnalysisInputDataIndex( analysis, name, index ) )
    {
        ErrorMgr.AddError( VSP_CANT_FIND_NAME, "GetVec3dAnalysisInput::Can't Find Name " + name );
    }
    else
    {
        ErrorMgr.NoError();
    }

    return AnalysisMgr.GetVec3dInputData( analysis, name, index );
}

void SetAnalysisInputDefaults( const string & analysis )
{
    if ( !AnalysisMgr.ValidAnalysisName( analysis ) )
    {
        ErrorMgr.AddError( VSP_INVALID_ID, "SetIntAnalysisInput::Invalid Analysis ID " + analysis );
    }
    else
    {
        ErrorMgr.NoError();
    }

    AnalysisMgr.SetAnalysisInputDefaults( analysis );
}

void SetIntAnalysisInput( const string & analysis, const string & name, const std::vector< int > & indata, int index )
{
    if ( !AnalysisMgr.ValidAnalysisName( analysis ) )
    {
        ErrorMgr.AddError( VSP_INVALID_ID, "SetIntAnalysisInput::Invalid Analysis ID " + analysis );
    }
    else if ( !AnalysisMgr.ValidAnalysisInputDataIndex( analysis, name, index ) )
    {
        ErrorMgr.AddError( VSP_CANT_FIND_NAME, "SetIntAnalysisInput::Can't Find Name " + name );
    }
    else
    {
        ErrorMgr.NoError();
    }

    AnalysisMgr.SetIntAnalysisInput( analysis, name, indata, index );
}

void SetDoubleAnalysisInput( const string & analysis, const string & name, const std::vector< double > & indata, int index )
{
    if ( !AnalysisMgr.ValidAnalysisName( analysis ) )
    {
        ErrorMgr.AddError( VSP_INVALID_ID, "SetDoubleAnalysisInput::Invalid Analysis ID " + analysis );
    }
    else if ( !AnalysisMgr.ValidAnalysisInputDataIndex( analysis, name, index ) )
    {
        ErrorMgr.AddError( VSP_CANT_FIND_NAME, "SetDoubleAnalysisInput::Can't Find Name " + name );
    }
    else
    {
        ErrorMgr.NoError();
    }

    AnalysisMgr.SetDoubleAnalysisInput( analysis, name, indata, index );
}

void SetStringAnalysisInput( const string & analysis, const string & name, const std::vector< std::string > & indata, int index )
{
    if ( !AnalysisMgr.ValidAnalysisName( analysis ) )
    {
        ErrorMgr.AddError( VSP_INVALID_ID, "SetStringAnalysisInput::Invalid Analysis ID " + analysis );
    }
    else if ( !AnalysisMgr.ValidAnalysisInputDataIndex( analysis, name, index ) )
    {
        ErrorMgr.AddError( VSP_CANT_FIND_NAME, "SetStringAnalysisInput::Can't Find Name " + name );
    }
    else
    {
        ErrorMgr.NoError();
    }

    AnalysisMgr.SetStringAnalysisInput( analysis, name, indata, index );
}

void SetVec3dAnalysisInput( const string & analysis, const string & name, const std::vector< vec3d > & indata, int index )
{
    if ( !AnalysisMgr.ValidAnalysisName( analysis ) )
    {
        ErrorMgr.AddError( VSP_INVALID_ID, "SetVec3dAnalysisInput::Invalid Analysis ID " + analysis );
    }
    else if ( !AnalysisMgr.ValidAnalysisInputDataIndex( analysis, name, index ) )
    {
        ErrorMgr.AddError( VSP_CANT_FIND_NAME, "SetVec3dAnalysisInput::Can't Find Name " + name );
    }
    else
    {
        ErrorMgr.NoError();
    }

    AnalysisMgr.SetVec3dAnalysisInput( analysis, name, indata, index );
}

void PrintAnalysisInputs( const string & analysis_name )
{
    if ( !AnalysisMgr.ValidAnalysisName( analysis_name ) )
    {
        ErrorMgr.AddError( VSP_INVALID_ID, "PrintAnalysisInputs::Invalid Analysis ID " + analysis_name );
    }

    AnalysisMgr.PrintAnalysisInputs( analysis_name );
}

//===================================================================//
//===============       Results Functions         ===================//
//===================================================================//
/// Get all results names avaiable
vector<string> GetAllResultsNames()
{
    return ResultsMgr.GetAllResultsNames();
}

/// Get all data names avaiable for this result
vector< string > GetAllDataNames( const string & results_id )
{
    if ( !ResultsMgr.ValidResultsID( results_id ) )
    {
        ErrorMgr.AddError( VSP_INVALID_ID, "GetAllDataNames::Invalid ID " + results_id  );
        vector<string> ret_vec;
        return ret_vec;
    }
    return ResultsMgr.GetAllDataNames( results_id );
}

/// Get the number of results with the given name.
int GetNumResults( const string & name )
{
    return ResultsMgr.GetNumResults( name );
}

/// Return the id of the results with the given results name and index
string FindResultsID( const string & name, int index )
{
    string id = ResultsMgr.FindResultsID( name, index );
    if ( id.size() == 0 )
    {
        ErrorMgr.AddError( VSP_CANT_FIND_NAME, "FindResultsID::Can't Find Name " + name  );
        return id;
    }
    ErrorMgr.NoError();
    return id;
}

/// Return the id of the latest results with the given results name
string FindLatestResultsID( const string & name )
{
    string id = ResultsMgr.FindLatestResultsID( name );
    if ( id.size() == 0 )
    {
        ErrorMgr.AddError( VSP_CANT_FIND_NAME, "FindLatestResultsID::Can't Find Name " + name  );
        return id;
    }
    ErrorMgr.NoError();
    return id;
}

/// Return the number of data entries given results_id and data name
extern int GetNumData( const string & results_id, const string & data_name )
{
    if ( !ResultsMgr.ValidResultsID( results_id ) )
    {
        ErrorMgr.AddError( VSP_INVALID_ID, "GetIntResults::Invalid ID " + results_id  );
        return 0;
    }
    ErrorMgr.NoError();
    return ResultsMgr.GetNumData( results_id, data_name );
}

extern int GetResultsType( const string & results_id, const string & data_name )
{
    if ( !ResultsMgr.ValidResultsID( results_id ) )
    {
        ErrorMgr.AddError( VSP_INVALID_ID, "GetResultsType::Invalid ID " + results_id  );
        return vsp::INVALID_TYPE;
    }
    ErrorMgr.NoError();
    return ResultsMgr.GetResultsType( results_id, data_name );
}

/// Return the int data given the results id, data name and data index
const vector<int> & GetIntResults( const string & id, const string & name, int index )
{
    if ( !ResultsMgr.ValidResultsID( id ) )
    {
        ErrorMgr.AddError( VSP_INVALID_ID, "GetIntResults::Invalid ID " + id  );
    }
    else if ( !ResultsMgr.ValidDataNameIndex( id, name, index ) )
    {
        ErrorMgr.AddError( VSP_CANT_FIND_NAME, "GetIntResults::Can't Find Name " + name  );
    }
    else
    {
        ErrorMgr.NoError();
    }

    return ResultsMgr.GetIntResults( id, name, index );
}

/// Return the double data given the results id, data name and data index
const vector<double> & GetDoubleResults( const string & id, const string & name, int index )
{
    if ( !ResultsMgr.ValidResultsID( id ) )
    {
        ErrorMgr.AddError( VSP_INVALID_ID, "GetDoubleResults::Invalid ID " + id  );
    }
    else if ( !ResultsMgr.ValidDataNameIndex( id, name, index ) )
    {
        ErrorMgr.AddError( VSP_CANT_FIND_NAME, "GetDoubleResults::Can't Find Name " + name  );
    }
    else
    {
        ErrorMgr.NoError();
    }

    return ResultsMgr.GetDoubleResults( id, name, index );
}

/// Return the string data given the results id, data name and data index
const vector<string> & GetStringResults( const string & id, const string & name, int index )
{
    if ( !ResultsMgr.ValidResultsID( id ) )
    {
        ErrorMgr.AddError( VSP_INVALID_ID, "GetStringResults::Invalid ID " + id  );
    }
    else if ( !ResultsMgr.ValidDataNameIndex( id, name, index ) )
    {
        ErrorMgr.AddError( VSP_CANT_FIND_NAME, "GetStringResults::Can't Find Name " + name  );
    }
    else
    {
        ErrorMgr.NoError();
    }

    return ResultsMgr.GetStringResults( id, name, index );
}

/// Return the vec3d data given the results id, data name and data index
const vector<vec3d> & GetVec3dResults( const string & id, const string & name, int index )
{
    if ( !ResultsMgr.ValidResultsID( id ) )
    {
        ErrorMgr.AddError( VSP_INVALID_ID, "GetVec3dResults::Invalid ID " + id  );
    }
    else if ( !ResultsMgr.ValidDataNameIndex( id, name, index ) )
    {
        ErrorMgr.AddError( VSP_CANT_FIND_NAME, "GetVec3dResults::Can't Find Name " + name  );
    }
    else
    {
        ErrorMgr.NoError();
    }

    return ResultsMgr.GetVec3dResults( id, name, index );
}

/// Create Geometry Results (Only Mesh Geom For Now) - Return Result ID
extern string CreateGeomResults( const string & geom_id, const string & name )
{
    Vehicle* veh = GetVehicle();

    Geom* geom_ptr =  veh->FindGeom( geom_id );

    if ( !geom_ptr )
    {
        ErrorMgr.AddError( VSP_INVALID_GEOM_ID, "CreateGeomResults::Can't Find GeomID " + geom_id  );
        return string();
    }

    string res_id = ResultsMgr.CreateGeomResults( geom_id, name );

    if ( !ResultsMgr.ValidResultsID( res_id ) )
    {
        ErrorMgr.AddError( VSP_INVALID_ID, "CreateGeomResults::Invalid Results " + res_id  );
    }
    else
    {
        ErrorMgr.NoError();
    }

    return res_id;
}


/// Delete all results
void DeleteAllResults()
{
    ResultsMgr.DeleteAllResults();
    ErrorMgr.NoError();
}

/// Delete result given id
void DeleteResult( const string & id )
{
    if ( !ResultsMgr.ValidResultsID( id ) )
    {
        ErrorMgr.AddError( VSP_INVALID_ID, "DeleteResult::Invalid ID " + id  );
    }
    else
    {
        ErrorMgr.NoError();
    }

    ResultsMgr.DeleteResult( id );
}

// Write Results To CSV File ====//
void WriteResultsCSVFile( const string & id, const string & file_name )
{
    Results* resptr = ResultsMgr.FindResultsPtr( id );

    if ( !resptr )
    {
        ErrorMgr.AddError( VSP_INVALID_ID, "WriteResultsCSVFile::Invalid ID " + id  );
        return;
    }
    resptr->WriteCSVFile( file_name );
    ErrorMgr.NoError();
 }

void PrintResults( const string &results_id )
{
    ResultsMgr.PrintResults( results_id );
}

//===================================================================//
//===============        GUI Functions            ===================//
//===================================================================//

extern void StartGui( )
{
#ifdef VSP_USE_FLTK
    GuiInterface::getInstance().StartGuiAPI( );
#endif
}


//===================================================================//
//===============       Geom Functions            ===================//
//===================================================================//

/// Get a vector of geometry types. The types will include user defined components
/// if available.  Fixed geom types are: "BLANK", "POD", "FUSELAGE"
vector< string > GetGeomTypes()
{
    Vehicle* veh = GetVehicle();

    //==== Load All Type Names ====//
    vector< string > ret_vec;
    for ( int i = 0 ; i < veh->GetNumGeomTypes() ; i++ )
    {
        ret_vec.push_back( veh->GetGeomType( i ).m_Name );
    }

    ErrorMgr.NoError();
    return ret_vec;
}

/// Add a geometry of given type to the vehicle.  The geometry will be a child of
/// optional parent geometry.  The ID string of the geometry will be returned.
string AddGeom( const string & type, const string & parent  )
{
    Vehicle* veh = GetVehicle();

    string ret_id;

    //==== Find Type Index ===//
    int type_index = -1;
    for ( int i = 0 ; i < veh->GetNumGeomTypes() ; i++ )
    {
        if ( veh->GetGeomType( i ).m_Name == type )
        {
            type_index = i;
        }
    }

    if ( type_index == -1 )
    {
        ErrorMgr.AddError( VSP_CANT_FIND_TYPE, "AddGeom::Can't Find Type Name " + type  );
        return ret_id;
    }

    Geom* parent_geom = NULL;
    if ( parent.size() > 0 )
    {
        parent_geom = veh->FindGeom( parent );
        if ( !parent_geom )
        {
            ErrorMgr.AddError( VSP_INVALID_GEOM_ID, "AddGeom::Can't Find Parent " + parent  );
            return ret_id;
        }
    }

    if ( parent_geom )
    {
        veh->SetActiveGeom( parent );
    }
    else
    {
        veh->ClearActiveGeom();
    }

    ret_id = veh->AddGeom( veh->GetGeomType( type_index )  );

    Geom* added_geom = veh->FindGeom( ret_id );

    if ( !added_geom )
    {
        ErrorMgr.AddError( VSP_INVALID_GEOM_ID, "AddGeom::Failed To Add Geom"  );
        return ret_id;
    }

    ErrorMgr.NoError();
    return ret_id;
}

void UpdateGeom( const string & geom_id )
{
    Vehicle* veh = GetVehicle();

    veh->UpdateGeom( geom_id );

    ErrorMgr.NoError();
}


void DeleteGeom( const string & geom_id )
{
    Vehicle* veh = GetVehicle();

    veh->DeleteGeom( geom_id );

    ErrorMgr.NoError();
}

void DeleteGeomVec( const vector< string > & del_vec )
{
    Vehicle* veh = GetVehicle();

    veh->DeleteGeomVec( del_vec );

    ErrorMgr.NoError();
}

/// Cut geometry and place it in the clipboard.  The clipboard is cleared before
/// the cut geom is placed there.
void CutGeomToClipboard( const string & geom_id )
{
    Vehicle* veh = GetVehicle();

    veh->SetActiveGeom( geom_id );
    veh->CutActiveGeomVec();

    ErrorMgr.NoError();
}

/// Copy geometry and place it in the clipboard.  The clipboard is cleared before
/// the geometry is placed there.
void CopyGeomToClipboard( const string & geom_id )
{
    Vehicle* veh = GetVehicle();

    veh->SetActiveGeom( geom_id );
    veh->CopyActiveGeomVec();

    ErrorMgr.NoError();
}

/// Paste the geometry in the clipboard to the vehicle.  The geometry is inserted
/// as a child of the optional parent id
void PasteGeomClipboard( const string & parent )
{
    Vehicle* veh = GetVehicle();

    Geom* parent_geom = NULL;
    if ( parent.size() > 0 )
    {
        parent_geom = veh->FindGeom( parent );
        if ( !parent_geom )
        {
            ErrorMgr.AddError( VSP_INVALID_GEOM_ID, "PasteGeomClipboard::Can't Find Parent " + parent  );
        }
    }

    if ( parent_geom )
    {
        veh->SetActiveGeom( parent );
    }
    else
    {
        veh->ClearActiveGeom();
    }

    veh->PasteClipboard();
    ErrorMgr.NoError();
}

/// Find and return all geoms
vector< string > FindGeoms()
{
    Vehicle* veh = GetVehicle();

    vector< string > ret_vec = veh->GetGeomVec();

    ErrorMgr.NoError();
    return ret_vec;
}

/// Find and return all geoms with given name
vector< string > FindGeomsWithName( const string & name )
{
    vector< string > ret_vec;
    Vehicle* veh = GetVehicle();

    vector< string > geom_id_vec = veh->GetGeomVec();
    for ( int i = 0 ; i < ( int )geom_id_vec.size() ; i++ )
    {
        Geom* gptr = veh->FindGeom( geom_id_vec[i] );
        if ( gptr && gptr->GetName() == name )
        {
            ret_vec.push_back( geom_id_vec[i] );
        }
    }
    ErrorMgr.NoError();
    return ret_vec;
}

/// Find and return all geom with given name and index
string FindGeom( const string & name, int index )
{
    vector< string > id_vec;
    Vehicle* veh = GetVehicle();

    vector< string > geom_id_vec = veh->GetGeomVec();
    for ( int i = 0 ; i < ( int )geom_id_vec.size() ; i++ )
    {
        Geom* gptr = veh->FindGeom( geom_id_vec[i] );
        if ( gptr && gptr->GetName() == name )
        {
            id_vec.push_back( geom_id_vec[i] );
        }
    }

    if ( index < 0 || index >= ( int )id_vec.size() )
    {
        ErrorMgr.AddError( VSP_INVALID_GEOM_ID, "FindGeom::Can't Find Name " + name + " or Index" + to_string( ( long long )index ) );
        return string();
    }
    ErrorMgr.NoError();
    return id_vec[index];
}

/// Set name of geom
void SetGeomName( const string & geom_id, const string & name )
{
    Vehicle* veh = GetVehicle();
    Geom* geom_ptr = veh->FindGeom( geom_id );
    if ( !geom_ptr )
    {
        ErrorMgr.AddError( VSP_INVALID_PTR, "SetGeomName::Can't Find Geom " + geom_id  );
        return;
    }
    geom_ptr->SetName( name );
    ErrorMgr.NoError();
}

/// Get name of geom
string GetGeomName( const string & geom_id )
{
    string ret_name;
    Vehicle* veh = GetVehicle();
    Geom* geom_ptr = veh->FindGeom( geom_id );
    if ( !geom_ptr )
    {
        ErrorMgr.AddError( VSP_INVALID_PTR, "GetGeomName::Can't Find Geom"  );
        return string();
    }
    ret_name = geom_ptr->GetName();
    ErrorMgr.NoError();
    return ret_name;
}

/// Get of the linkable parms ids for this geometry
vector< string > GetGeomParmIDs( const string & geom_id  )
{
    vector< string > parm_vec;

    Vehicle* veh = GetVehicle();
    Geom* geom_ptr = veh->FindGeom( geom_id );
    if ( !geom_ptr )
    {
        ErrorMgr.AddError( VSP_INVALID_PTR, "GetGeomParmIDs::Can't Find Geom " + geom_id  );
        return parm_vec;
    }

    geom_ptr->AddLinkableParms( parm_vec );

    ErrorMgr.NoError();
    return parm_vec;
}

/// Get the parm id given geom id, parm name, and group name
string GetParm( const string & geom_id, const string & name, const string & group )
{
    Vehicle* veh = GetVehicle();
    LinkMgr.BuildLinkableParmData();        // Make Sure Name/Group Get Mapped To Parms

    string parm_id;

    Geom* geom_ptr = veh->FindGeom( geom_id );
    if ( !geom_ptr )
    {
        ErrorMgr.AddError( VSP_INVALID_PTR, "GetParm::Can't Find Geom " + geom_id );
        return parm_id;
    }

    parm_id = geom_ptr->FindParm( name, group );

    Parm* p = ParmMgr.FindParm( parm_id );
    if ( !p )
    {
        ErrorMgr.AddError( VSP_CANT_FIND_PARM, "GetParm::Can't Find Parm " + geom_id + ":" + group + ":" + name  );
        return parm_id;
    }
    ErrorMgr.NoError();
    return parm_id;
}

/// Get the number of xsec surfs used in the construction of this geom
int GetNumXSecSurfs( const string & geom_id )
{
    Vehicle* veh = GetVehicle();
    Geom* geom_ptr = veh->FindGeom( geom_id );
    if ( !geom_ptr )
    {
        ErrorMgr.AddError( VSP_INVALID_PTR, "GetNumXSecSurfs::Can't Find Geom " + geom_id  );
        return 0;
    }

    ErrorMgr.NoError();
    return geom_ptr->GetNumXSecSurfs();
}

/// Get the number of main surfs for this geom
int GetNumMainSurfs( const string & geom_id )
{
    Vehicle* veh = GetVehicle();
    Geom* geom_ptr = veh->FindGeom( geom_id );
    if ( !geom_ptr )
    {
        ErrorMgr.AddError( VSP_INVALID_PTR, "GetNumMainSurfs::Can't Find Geom " + geom_id  );
        return 0;
    }

    ErrorMgr.NoError();
    return geom_ptr->GetNumMainSurfs();
}

/// Add a sub surface, return subsurface id
string AddSubSurf( const string & geom_id, int type, int surfindex )
{
    Vehicle* veh = GetVehicle();
    Geom* geom_ptr = veh->FindGeom( geom_id );
    if ( !geom_ptr )
    {
        ErrorMgr.AddError( VSP_INVALID_PTR, "AddSubSurf::Can't Find Geom " + geom_id  );
        return string();
    }

    SubSurface* ssurf = NULL;
    ssurf = geom_ptr->AddSubSurf( type, surfindex );
    if ( !ssurf )
    {
        ErrorMgr.AddError( VSP_INVALID_PTR, "AddSubSurf::Invalid Sub Surface Ptr "  );
        return string();
    }
    ssurf->Update();
    ErrorMgr.NoError();
    return ssurf->GetID();
}

/// Get ID for sub surface at index
string GetSubSurf( const string & geom_id, int index )
{
    Vehicle* veh = GetVehicle();
    Geom* geom_ptr = veh->FindGeom( geom_id );
    if ( !geom_ptr )
    {
        ErrorMgr.AddError( VSP_INVALID_PTR, "GetSubSurf::Can't Find Geom " + geom_id  );
        return string();
    }
    SubSurface* ssurf = NULL;
    ssurf = geom_ptr->GetSubSurf( index );
    if ( !ssurf )
    {
        ErrorMgr.AddError( VSP_INVALID_PTR, "GetSubSurf::Invalid Sub Surface Ptr "  );
        return string();
    }
    ErrorMgr.NoError();
    return ssurf->GetID();
}

void DeleteSubSurf( const string & geom_id, const string & sub_id )
{
    Vehicle* veh = GetVehicle();
    Geom* geom_ptr = veh->FindGeom( geom_id );
    if ( !geom_ptr )
    {
        ErrorMgr.AddError( VSP_INVALID_PTR, "DeleteSubSurf::Can't Find Geom " + geom_id );
        return;
    }
    int index = geom_ptr->GetSubSurfIndex( sub_id );
    if ( index == -1 )
    {
        ErrorMgr.AddError( VSP_INVALID_PTR, "DeleteSubSurf::Can't Find SubSurf " + geom_id );
        return;
    }
    geom_ptr->DelSubSurf( index );
    ErrorMgr.NoError();
    return;
}

void DeleteSubSurf( const string & sub_id )
{
    SubSurface* ss_ptr = SubSurfaceMgr.GetSubSurf( sub_id );
    if ( !ss_ptr )
    {
        ErrorMgr.AddError( VSP_INVALID_PTR, "DeleteSubSurf::Can't Find SubSurf " + sub_id );
        return;
    }
    Vehicle* veh = GetVehicle();
    Geom* geom_ptr = veh->FindGeom( ss_ptr->GetCompID() );
    if ( !geom_ptr )
    {
        ErrorMgr.AddError( VSP_INVALID_PTR, "DeleteSubSurf::Can't Find Geom " + ss_ptr->GetCompID() );
        return;
    }
    int index = geom_ptr->GetSubSurfIndex( sub_id );
    if ( index == -1 )
    {
        ErrorMgr.AddError( VSP_INVALID_PTR, "DeleteSubSurf::Can't Find SubSurf " + sub_id );
        return;
    }
    geom_ptr->DelSubSurf( index );
    ErrorMgr.NoError();
    return;
}

void SetSubSurfName( const std::string & geom_id, const std::string & sub_id, const std::string & name )
{
    Vehicle* veh = GetVehicle();
    Geom* geom_ptr = veh->FindGeom( geom_id );
    if ( !geom_ptr )
    {
        ErrorMgr.AddError( VSP_INVALID_PTR, "SetSubSurfName::Can't Find Geom " + geom_id );
        return;
    }
    SubSurface* ssurf = NULL;
    ssurf = geom_ptr->GetSubSurf( sub_id );
    if ( !ssurf )
    {
        ErrorMgr.AddError( VSP_INVALID_PTR, "SetSubSurfName::Invalid Sub Surface Ptr " );
        return;
    }
    ssurf->SetName( name );
    ErrorMgr.NoError();
    return;
}

void SetSubSurfName( const std::string & sub_id, const std::string & name )
{
    SubSurface* ss_ptr = SubSurfaceMgr.GetSubSurf( sub_id );
    if ( !ss_ptr )
    {
        ErrorMgr.AddError( VSP_INVALID_PTR, "SetSubSurfName::Invalid Sub Surface Ptr " );
        return;
    }
    ss_ptr->SetName( name );
    ErrorMgr.NoError();
    return;
}

std::string GetSubSurfName( const std::string & geom_id, const std::string & sub_id )
{
    Vehicle* veh = GetVehicle();
    Geom* geom_ptr = veh->FindGeom( geom_id );
    if ( !geom_ptr )
    {
        ErrorMgr.AddError( VSP_INVALID_PTR, "GetSubSurfName::Can't Find Geom " + geom_id );
        return string();
    }
    SubSurface* ssurf = NULL;
    ssurf = geom_ptr->GetSubSurf( sub_id );
    if ( !ssurf )
    {
        ErrorMgr.AddError( VSP_INVALID_PTR, "GetSubSurfName::Invalid Sub Surface Ptr " );
        return string();
    }
    ErrorMgr.NoError();
    return ssurf->GetName();
}

std::string GetSubSurfName( const std::string & sub_id )
{
    SubSurface* ssurf = SubSurfaceMgr.GetSubSurf( sub_id );
    if ( !ssurf )
    {
        ErrorMgr.AddError( VSP_INVALID_PTR, "GetSubSurfName::Invalid Sub Surface Ptr " );
        return string();
    }
    ErrorMgr.NoError();
    return ssurf->GetName();
}

/// Get index for sub surface given ID
int GetSubSurfIndex( const std::string & sub_id )
{
    SubSurface* ss_ptr = SubSurfaceMgr.GetSubSurf( sub_id );
    if ( !ss_ptr )
    {
        ErrorMgr.AddError( VSP_INVALID_PTR, "GetSubSurfIndex::Invalid Sub Surface Ptr " );
        return -1;
    }
    Vehicle* veh = GetVehicle();
    Geom* geom_ptr = veh->FindGeom( ss_ptr->GetCompID() );
    if ( !geom_ptr )
    {
        ErrorMgr.AddError( VSP_INVALID_PTR, "GetSubSurfIndex::Can't Find Geom " + ss_ptr->GetCompID() );
        return -1;
    }

    int ss_ind = geom_ptr->GetSubSurfIndex( sub_id );;

    ErrorMgr.NoError();
    return ss_ind;
}

/// Get vector of sub surface IDs for a Geom
std::vector<std::string> GetSubSurfIDVec( const std::string & geom_id )
{
    Vehicle* veh = GetVehicle();
    Geom* geom_ptr = veh->FindGeom( geom_id );
    if ( !geom_ptr )
    {
        ErrorMgr.AddError( VSP_INVALID_PTR, "GetSubSurfIDVec::Can't Find Geom " + geom_id );
        return vector<string>();
    }

    vector<SubSurface*> ss_vec = geom_ptr->GetSubSurfVec();
    vector<string> ID_vec;
    ID_vec.resize( ss_vec.size() );

    for ( size_t i = 0; i < ss_vec.size(); i++ )
    {
        ID_vec[i] = ss_vec[i]->GetID();
    }

    ErrorMgr.NoError();
    return ID_vec;
}

void CutXSec( const string & geom_id, int index )
{
    Vehicle* veh = GetVehicle();
    Geom* geom_ptr = veh->FindGeom( geom_id );
    if ( !geom_ptr )
    {
        ErrorMgr.AddError( VSP_INVALID_PTR, "CutXSec::Can't Find Geom " + geom_id );
        return;
    }

    geom_ptr->CutXSec( index );
    Update();

    ErrorMgr.NoError();
}

void CopyXSec( const string & geom_id, int index )
{
    Vehicle* veh = GetVehicle();
    Geom* geom_ptr = veh->FindGeom( geom_id );
    if ( !geom_ptr )
    {
        ErrorMgr.AddError( VSP_INVALID_PTR, "CopyXSec::Can't Find Geom " + geom_id );
        return;
    }

    geom_ptr->CopyXSec( index );
    ErrorMgr.NoError();
}

void PasteXSec( const string & geom_id, int index )
{
    Vehicle* veh = GetVehicle();
    Geom* geom_ptr = veh->FindGeom( geom_id );
    if ( !geom_ptr )
    {
        ErrorMgr.AddError( VSP_INVALID_PTR, "PasteXSec::Can't Find Geom " + geom_id  );
        return;
    }

    geom_ptr->PasteXSec( index );
    ErrorMgr.NoError();
}

void InsertXSec( const string & geom_id, int index, int type )
{
    Vehicle* veh = GetVehicle();
    Geom* geom_ptr = veh->FindGeom( geom_id );
    if ( !geom_ptr )
    {
        ErrorMgr.AddError( VSP_INVALID_PTR, "InsertXSec::Can't Find Geom " + geom_id  );
        return;
    }

    geom_ptr->InsertXSec( index, type );
    ErrorMgr.NoError();
}


//===================================================================//
//===============       Wing Section Functions     ==================//
//===================================================================//
void SetDriverGroup( const string & geom_id, int section_index, int driver_0, int driver_1, int driver_2 )
{
    Vehicle* veh = GetVehicle();
    Geom* geom_ptr = veh->FindGeom( geom_id );
    if ( !geom_ptr )
    {
        ErrorMgr.AddError( VSP_INVALID_PTR, "SetDriverGroup::Can't Find Geom " + geom_id  );
        return;
    }

    WingGeom* wg = dynamic_cast< WingGeom* >(geom_ptr);
    if ( !wg )
    {
        ErrorMgr.AddError( VSP_INVALID_PTR, "SetDriverGroup::Invalid Geom Type " + geom_id  );
        return;
    }

    WingSect* ws = wg->GetWingSect( section_index );
    if ( !ws )
    {
        ErrorMgr.AddError( VSP_INVALID_PTR, "SetDriverGroup::Invalid Wing Section Index " + to_string( ( long long )section_index ) );
        return;
    }

    ws->m_DriverGroup.SetChoice( 0, driver_0 );
    ws->m_DriverGroup.SetChoice( 1, driver_1 );
    ws->m_DriverGroup.SetChoice( 2, driver_2 );
    ErrorMgr.NoError();
}

//===================================================================//
//===============       XSecSurf Functions         ==================//
//===================================================================//
/// Get the XSecSurf ID given Geom ID and index
string GetXSecSurf( const string & geom_id, int index )
{
    Vehicle* veh = GetVehicle();
    Geom* geom_ptr = veh->FindGeom( geom_id );
    if ( !geom_ptr )
    {
        ErrorMgr.AddError( VSP_INVALID_PTR, "GetXSecSurf::Can't Find Geom " + geom_id  );
        return string();
    }
    XSecSurf* xsec_surf = geom_ptr->GetXSecSurf( index );

    if ( !xsec_surf )
    {
        ErrorMgr.AddError( VSP_INVALID_PTR, "GetXSecSurf::Can't Find XSecSurf " + geom_id + ":" + to_string( ( long long )index )  );
        return string();
    }

    ErrorMgr.NoError();
    return xsec_surf->GetID();
}

/// Get the number of XSecs in the XSecSurf
int GetNumXSec( const string & xsec_surf_id )
{
    XSecSurf* xsec_surf = FindXSecSurf( xsec_surf_id );
    if ( !xsec_surf )
    {
        ErrorMgr.AddError( VSP_INVALID_PTR, "GetNumXSec::Can't Find XSecSurf " + xsec_surf_id  );
        return 0;
    }

    ErrorMgr.NoError();
    return xsec_surf->NumXSec();
}

/// Get xsec id from xsec_surf at given index.
string GetXSec( const string & xsec_surf_id, int xsec_index )
{
    XSecSurf* xsec_surf = FindXSecSurf( xsec_surf_id );
    if ( !xsec_surf )
    {
        ErrorMgr.AddError( VSP_INVALID_PTR, "GetXSec::Can't Find XSecSurf " + xsec_surf_id  );
        return string();
    }
    XSec* xsec = xsec_surf->FindXSec( xsec_index );
    if ( !xsec )
    {
        ErrorMgr.AddError( VSP_INVALID_PTR, "GetXSec::Can't Find XSec " + xsec_surf_id + ":" + to_string( ( long long )xsec_index )  );
        return string();
    }

    ErrorMgr.NoError();
    return xsec->GetID();
}

///// Cut xsec from xsec_surf.  The xsec is stored in a clipboard and can be pasted.
//void CutXSec( const string & xsec_surf_id, int xsec_index )
//{
//    XSecSurf* xsec_surf = FindXSecSurf( xsec_surf_id );
//    if ( !xsec_surf )
//    {
//        ErrorMgr.AddError( VSP_INVALID_PTR, "CutXSec::Can't Find XSecSurf " + xsec_surf_id  );
//        return;
//    }
//    if ( xsec_index < 0 || xsec_index >= xsec_surf->NumXSec() )
//    {
//        ErrorMgr.AddError( VSP_INDEX_OUT_RANGE, "CutXSec::XSec Index Out of Range " + xsec_surf_id + ":" + to_string( ( long long )xsec_index )  );
//        return;
//    }
//
//    ErrorMgr.NoError();
//    xsec_surf->CutXSec( xsec_index );
//}

///// Copy xsec from xsec_surf.  The xsec is stored in a clipboard and can be pasted.
//void CopyXSec( const string & xsec_surf_id, int xsec_index )
//{
//    XSecSurf* xsec_surf = FindXSecSurf( xsec_surf_id );
//    if ( !xsec_surf )
//    {
//        ErrorMgr.AddError( VSP_INVALID_PTR, "CopyXSec::Can't Find XSecSurf " + xsec_surf_id  );
//        return;
//    }
//    if ( xsec_index < 0 || xsec_index >= xsec_surf->NumXSec() )
//    {
//        ErrorMgr.AddError( VSP_INDEX_OUT_RANGE, "CopyXSec::XSec Index Out of Range " + xsec_surf_id + ":" + to_string( ( long long )xsec_index )  );
//        return;
//    }
//
//    ErrorMgr.NoError();
//    xsec_surf->CopyXSec( xsec_index );
//}

///// Paste xsec from clipboard to xsec_surf.  The pasted xsec replaces the xsec at xsec index.
//void PasteXSec( const string & xsec_surf_id, int xsec_index )
//{
//    XSecSurf* xsec_surf = FindXSecSurf( xsec_surf_id );
//    if ( !xsec_surf )
//    {
//        ErrorMgr.AddError( VSP_INVALID_PTR, "PasteXSec::Can't Find XSecSurf " + xsec_surf_id );
//        return;
//    }
//    if ( xsec_index < 0 || xsec_index >= xsec_surf->NumXSec() )
//    {
//        ErrorMgr.AddError( VSP_INDEX_OUT_RANGE, "PasteXSec::XSec Index Out of Range " + xsec_surf_id + ":" + to_string( ( long long )xsec_index )  );
//        return;
//    }
//
//    ErrorMgr.NoError();
//    xsec_surf->PasteXSec( xsec_index );
//}

///// Create an xsec of type type and add it to the end of xsecsurf
//string AppendXSec( const string & xsec_surf_id, int type  )
//{
//    XSecSurf* xsec_surf = FindXSecSurf( xsec_surf_id );
//    if ( !xsec_surf )
//    {
//        ErrorMgr.AddError( VSP_INVALID_PTR, "AddXSec::Can't Find XSecSurf " + xsec_surf_id  );
//        return string();
//    }
//
//    string id  = xsec_surf->AddXSec( type );
//    if ( id.size() == 0 )
//    {
//        ErrorMgr.AddError( VSP_INVALID_XSEC_ID, "AddXSec::Invalid XSec Type " + to_string( ( long long ) type ) );
//        return id;
//    }
//
//    ErrorMgr.NoError();
//    return id;
//}

///// Create an xsec of type type and insert it after xsec_index
//string InsertXSec( const string & xsec_surf_id, int type, int xsec_index  )
//{
//    XSecSurf* xsec_surf = FindXSecSurf( xsec_surf_id );
//    if ( !xsec_surf )
//    {
//        ErrorMgr.AddError( VSP_INVALID_PTR, "InsertXSec::Can't Find XSecSurf " + xsec_surf_id  );
//        return string();
//    }
//    if ( xsec_index < 0 || xsec_index >= xsec_surf->NumXSec() )
//    {
//        ErrorMgr.AddError( VSP_INDEX_OUT_RANGE, "InsertXSec::XSec Index Out of Range " + xsec_surf_id + ":" + to_string( ( long long )xsec_index )  );
//        return string();
//    }
//
//    string id  = xsec_surf->InsertXSec( type, xsec_index );
//    if ( id.size() == 0 )
//    {
//        ErrorMgr.AddError( VSP_INVALID_XSEC_ID, "InsertXSec::Invalid XSec Type " + to_string( ( long long ) type ) );
//        return id;
//    }
//
//    ErrorMgr.NoError();
//    return id;
//}

/// Create an xsec of type and insert it after xsec_index
void ChangeXSecShape( const string & xsec_surf_id, int xsec_index, int type )
{
    XSecSurf* xsec_surf = FindXSecSurf( xsec_surf_id );
    if ( !xsec_surf )
    {
        ErrorMgr.AddError( VSP_INVALID_PTR, "ChangeXSecShape::Can't Find XSecSurf " + xsec_surf_id  );
        return;
    }
    if ( xsec_index < 0 || xsec_index >= xsec_surf->NumXSec() )
    {
        ErrorMgr.AddError( VSP_INDEX_OUT_RANGE, "ChangeXSecShape::XSec Index Out of Range " + xsec_surf_id + ":" + to_string( ( long long )xsec_index )  );
        return;
    }

    ErrorMgr.NoError();
    xsec_surf->ChangeXSecShape( xsec_index, type );
}

/// Set The Global XForm Matrix For This XSec Surf
void SetXSecSurfGlobalXForm( const string & xsec_surf_id, const Matrix4d & mat )
{
    XSecSurf* xsec_surf = FindXSecSurf( xsec_surf_id );
    if ( !xsec_surf )
    {
        ErrorMgr.AddError( VSP_INVALID_PTR, "SetXSecSurfGlobalXForm::Can't Find XSecSurf " + xsec_surf_id  );
        return;
    }
    xsec_surf->SetGlobalXForm( mat );
}

/// Get The Global XForm Matrix For This XSec Surf
Matrix4d GetXSecSurfGlobalXForm( const string & xsec_surf_id )
{
    XSecSurf* xsec_surf = FindXSecSurf( xsec_surf_id );
    if ( !xsec_surf )
    {
        ErrorMgr.AddError( VSP_INVALID_PTR, "ChangeXSecShape::Can't Find XSecSurf " + xsec_surf_id  );
        return Matrix4d();
    }
    return xsec_surf->GetGlobalXForm();
}
//===================================================================//
//===============       XSec Functions         ==================//
//===================================================================//
/// Get XSec shape given xsec id
int GetXSecShape( const string& xsec_id )
{
    XSec* xs = FindXSec( xsec_id );
    if ( !xs )
    {
        ErrorMgr.AddError( VSP_INVALID_PTR, "GetXSecShape::Can't Find XSec " + xsec_id  );
        return 0;
    }

    ErrorMgr.NoError();
    return xs->GetXSecCurve()->GetType();
}

/// Get XSec Width
double GetXSecWidth( const string& xsec_id )
{
    XSec* xs = FindXSec( xsec_id );
    if ( !xs )
    {
        ErrorMgr.AddError( VSP_INVALID_PTR, "GetXSecWidth::Can't Find XSec " + xsec_id  );
        return 0;
    }
    ErrorMgr.NoError();
    return xs->GetXSecCurve()->GetWidth();
}

/// Get XSec Height
double GetXSecHeight( const string& xsec_id )
{
    XSec* xs = FindXSec( xsec_id );
    if ( !xs )
    {
        ErrorMgr.AddError( VSP_INVALID_PTR, "GetXSecHeight::Can't Find XSec " + xsec_id  );
        return 0;
    }
    ErrorMgr.NoError();
    return xs->GetXSecCurve()->GetHeight();
}

/// Set XSec Width & Height
void SetXSecWidthHeight( const string& xsec_id, double w, double h )
{
    XSec* xs = FindXSec( xsec_id );
    if ( !xs )
    {
        ErrorMgr.AddError( VSP_INVALID_PTR, "SetXSecWidthHeight::Can't Find XSec " + xsec_id  );
        return;
    }
    xs->GetXSecCurve()->SetWidthHeight( w, h );
    xs->ParmChanged( NULL, Parm::SET_FROM_DEVICE ); // Force Update
    ErrorMgr.NoError();
}

/// Get of the linkable parms ids for this geometry
vector< string > GetXSecParmIDs( const string & xsec_id  )
{
    vector< string > parm_vec;

    XSec* xs = FindXSec( xsec_id );
    if ( !xs )
    {
        ErrorMgr.AddError( VSP_INVALID_PTR, "GetXSecParmIDs::Can't Find XSec " + xsec_id  );
        return parm_vec;
    }

    xs->AddLinkableParms( parm_vec );

    ErrorMgr.NoError();
    return parm_vec;
}


/// Get named ParmID from XSec
string GetXSecParm( const string& xsec_id, const string& name )
{
    XSec* xs = FindXSec( xsec_id );
    if ( !xs )
    {
        ErrorMgr.AddError( VSP_INVALID_PTR, "GetXSecParm::Can't Find XSec " + xsec_id  );
        return string();
    }

    //==== Valid XSec Parm - Return Parm ID ====//
    string xsparm = xs->FindParm( name );
    if ( ValidParm( xsparm ) )
    {
        ErrorMgr.NoError();
        return xsparm;
    }

    //==== Check Curve For Name ====//
    XSecCurve* xsc = xs->GetXSecCurve();
    if ( !xsc )
    {
        ErrorMgr.AddError( VSP_INVALID_PTR, "GetXSecParm::Can't Find XSecCurve " + xsec_id  );
        return string();
    }

    string xscparm = xsc->FindParm( name );
    if ( ValidParm( xscparm ) )
    {
        ErrorMgr.NoError();
        return xscparm;
    }

    ErrorMgr.AddError( VSP_CANT_FIND_NAME, "GetXSecParm::Can't Find Parm " + name  );
    return string();
}

//==== Read XSec From File ====//
vector<vec3d> ReadFileXSec( const string& xsec_id, const string& file_name )
{
    vector< vec3d > pnt_vec;
    XSec* xs = FindXSec( xsec_id );
    if ( !xs )
    {
        ErrorMgr.AddError( VSP_INVALID_PTR, "ReadFileXSec::Can't Find XSec " + xsec_id  );
        return pnt_vec;
    }

    if ( xs->GetXSecCurve()->GetType() == XS_FILE_FUSE )
    {
        FileXSec* file_xs = dynamic_cast<FileXSec*>( xs->GetXSecCurve() );
        assert( file_xs );
        if ( file_xs->ReadXsecFile( file_name ) )
        {
            ErrorMgr.NoError();
            return file_xs->GetUnityFilePnts();
        }
        else
        {
            ErrorMgr.AddError( VSP_FILE_DOES_NOT_EXIST, "ReadFileXSec::Error reading fuselage file" );
            return pnt_vec;
        }
    }

    ErrorMgr.AddError( VSP_WRONG_XSEC_TYPE, "ReadFileXSec::XSec Not XS_FILE_FUSE Type " + xsec_id );
    return pnt_vec;
}

//==== Set XSec Pnts ====//
void SetXSecPnts( const string& xsec_id, vector< vec3d > & pnt_vec )
{
    XSec* xs = FindXSec( xsec_id );
    if ( !xs )
    {
        ErrorMgr.AddError( VSP_INVALID_PTR, "SetXSecPnts::Can't Find XSec " + xsec_id  );
        return;
    }
    if ( xs->GetXSecCurve()->GetType() != XS_FILE_FUSE )
    {
        ErrorMgr.AddError( VSP_WRONG_XSEC_TYPE, "SetXSecPnts::Wrong XSec Type"  );
        return;
    }

    FileXSec* file_xs = dynamic_cast<FileXSec*>( xs->GetXSecCurve() );
    assert( file_xs );
    file_xs->SetPnts( pnt_vec );
    ErrorMgr.NoError();
}

//==== Compute Point Along XSec ====//
vec3d ComputeXSecPnt( const string& xsec_id, double fract )
{
    XSec* xs = FindXSec( xsec_id );
    if ( !xs )
    {
        ErrorMgr.AddError( VSP_INVALID_PTR, "ComputePnt::Can't Find XSec " + xsec_id  );
        return vec3d();
    }

    vec3d pnt = xs->GetCurve().CompPnt01( fract );
    ErrorMgr.NoError();

    return pnt;
}

//==== Compute Tan Along XSec ====//
vec3d ComputeXSecTan( const string& xsec_id, double fract )
{
    XSec* xs = FindXSec( xsec_id );
    if ( !xs )
    {
        ErrorMgr.AddError( VSP_INVALID_PTR, "ComputeTan::Can't Find XSec " + xsec_id  );
        return vec3d();
    }

    vec3d pnt = xs->GetCurve().CompTan01( fract );
    ErrorMgr.NoError();

    return pnt;
}

//==== Reset All XSec Skining Parms ====//
void ResetXSecSkinParms( const string& xsec_id )
{
    XSec* xs = FindXSec( xsec_id );
    if ( !xs )
    {
        ErrorMgr.AddError( VSP_INVALID_PTR, "ResetXSecSkinParms::Can't Find XSec " + xsec_id  );
        return;
    }
    SkinXSec* skinxs = dynamic_cast<SkinXSec*>(xs);
    if ( !skinxs )
    {
        ErrorMgr.AddError( VSP_INVALID_PTR, "ResetXSecSkinParms::Can't Convert To Skin XSec " + xsec_id  );
        return;
    }

    skinxs->Reset();
    ErrorMgr.NoError();
}

//==== Set Continuity At XSec ====//
void SetXSecContinuity( const string& xsec_id, int cx )
{
    XSec* xs = FindXSec( xsec_id );
    if ( !xs )
    {
        ErrorMgr.AddError( VSP_INVALID_PTR, "SetXSecContinuity::Can't Find XSec " + xsec_id  );
        return;
    }
    SkinXSec* skinxs = dynamic_cast<SkinXSec*>(xs);
    if ( !skinxs )
    {
        ErrorMgr.AddError( VSP_INVALID_PTR, "SetXSecContinuity::Can't Convert To Skin XSec " + xsec_id  );
        return;
    }

    skinxs->SetContinuity( cx );
    ErrorMgr.NoError();
}

//==== Set Tan Angles At XSec ====//
void SetXSecTanAngles( const string& xsec_id, int side, double top, double right, double bottom, double left )
{
    XSec* xs = FindXSec( xsec_id );
    if ( !xs )
    {
        ErrorMgr.AddError( VSP_INVALID_PTR, "SetXSecTanAngles::Can't Find XSec " + xsec_id  );
        return;
    }
    SkinXSec* skinxs = dynamic_cast<SkinXSec*>(xs);
    if ( !skinxs )
    {
        ErrorMgr.AddError( VSP_INVALID_PTR, "SetXSecTanAngles::Can't Convert To Skin XSec " + xsec_id  );
        return;
    }

    skinxs->SetTanAngles( side, top, right, bottom, left );
    ErrorMgr.NoError();
}

//==== Set Tan Slews At XSec ====//
void SetXSecTanSlews( const string& xsec_id, int side, double top, double right, double bottom, double left )
{
    XSec* xs = FindXSec( xsec_id );
    if ( !xs )
    {
        ErrorMgr.AddError( VSP_INVALID_PTR, "SetXSecTanSlews::Can't Find XSec " + xsec_id  );
        return;
    }
    SkinXSec* skinxs = dynamic_cast<SkinXSec*>(xs);
    if ( !skinxs )
    {
        ErrorMgr.AddError( VSP_INVALID_PTR, "SetXSecTanSlews::Can't Convert To Skin XSec " + xsec_id  );
        return;
    }

    skinxs->SetTanSlews( side, top, right, bottom, left );
    ErrorMgr.NoError();
}

//==== Set Tan Strengths At XSec ====//
void SetXSecTanStrengths( const string& xsec_id, int side, double top, double right, double bottom, double left )
{
    XSec* xs = FindXSec( xsec_id );
    if ( !xs )
    {
        ErrorMgr.AddError( VSP_INVALID_PTR, "SetXSecTanStrengths::Can't Find XSec " + xsec_id  );
        return;
    }
    SkinXSec* skinxs = dynamic_cast<SkinXSec*>(xs);
    if ( !skinxs )
    {
        ErrorMgr.AddError( VSP_INVALID_PTR, "SetXSecTanStrengths::Can't Convert To Skin XSec " + xsec_id  );
        return;
    }

    skinxs->SetTanStrengths( side, top, right, bottom, left );
    ErrorMgr.NoError();
}

//==== Set Curvature Angles At XSec ====//
void SetXSecCurvatures( const string& xsec_id, int side, double top, double right, double bottom, double left )
{
    XSec* xs = FindXSec( xsec_id );
    if ( !xs )
    {
        ErrorMgr.AddError( VSP_INVALID_PTR, "SetXSecCurvatures::Can't Find XSec " + xsec_id  );
        return;
    }
    SkinXSec* skinxs = dynamic_cast<SkinXSec*>(xs);
    if ( !skinxs )
    {
        ErrorMgr.AddError( VSP_INVALID_PTR, "SetXSecCurvatures::Can't Convert To Skin XSec " + xsec_id  );
        return;
    }

    skinxs->SetCurvatures( side, top, right, bottom, left );
    ErrorMgr.NoError();
}

void ReadFileAirfoil( const string& xsec_id, const string& file_name )
{
    vector< vec3d > pnt_vec;
    XSec* xs = FindXSec( xsec_id );
    if ( !xs )
    {
        ErrorMgr.AddError( VSP_INVALID_PTR, "ReadFileAirfoil::Can't Find XSec " + xsec_id  );
        return;
    }

    if ( xs->GetXSecCurve()->GetType() == XS_FILE_AIRFOIL )
    {
        FileAirfoil* file_xs = dynamic_cast<FileAirfoil*>( xs->GetXSecCurve() );
        assert( file_xs );
        if( file_xs->ReadFile( file_name ) )
        {
            ErrorMgr.NoError();
            return;
        }
        else
        {
            ErrorMgr.AddError( VSP_FILE_DOES_NOT_EXIST, "ReadFileAirfoil::Error reading airfoil file" );
            return;
        }
    }

    ErrorMgr.AddError( VSP_WRONG_XSEC_TYPE, "ReadFileAirfoil::XSec Not XS_FILE_AIRFOIL Type " + xsec_id  );
    return;
}

void SetAirfoilPnts( const string& xsec_id, std::vector< vec3d > & up_pnt_vec, std::vector< vec3d > & low_pnt_vec )
{
    XSec* xs = FindXSec( xsec_id );
    if ( !xs )
    {
        ErrorMgr.AddError( VSP_INVALID_PTR, "SetAirfoilPnts::Can't Find XSec " + xsec_id  );
        return;
    }

    if ( xs->GetXSecCurve()->GetType() != XS_FILE_AIRFOIL )
    {
        ErrorMgr.AddError( VSP_WRONG_XSEC_TYPE, "SetAirfoilPnts::XSec Not XS_FILE_AIRFOIL Type"  );
        return;
    }

    FileAirfoil* file_xs = dynamic_cast<FileAirfoil*>( xs->GetXSecCurve() );
    assert( file_xs );
    file_xs->SetAirfoilPnts( up_pnt_vec, low_pnt_vec );
    ErrorMgr.NoError();
}

std::vector<vec3d> GetAirfoilUpperPnts( const string& xsec_id )
{
    vector< vec3d > pnt_vec;

    XSec* xs = FindXSec( xsec_id );
    if ( !xs )
    {
        ErrorMgr.AddError( VSP_INVALID_PTR, "GetAirfoilUpperPnts::Can't Find XSec " + xsec_id  );
        return pnt_vec;
    }

    if ( xs->GetXSecCurve()->GetType() != XS_FILE_AIRFOIL )
    {
        ErrorMgr.AddError( VSP_WRONG_XSEC_TYPE, "GetAirfoilUpperPnts::XSec Not XS_FILE_AIRFOIL Type"  );
        return pnt_vec;
    }

    FileAirfoil* file_xs = dynamic_cast<FileAirfoil*>( xs->GetXSecCurve() );
    assert( file_xs );
    pnt_vec = file_xs->GetUpperPnts();
    ErrorMgr.NoError();
    return pnt_vec;
}

std::vector<vec3d> GetAirfoilLowerPnts( const string& xsec_id )
{
    vector< vec3d > pnt_vec;

    XSec* xs = FindXSec( xsec_id );
    if ( !xs )
    {
        ErrorMgr.AddError( VSP_INVALID_PTR, "GetAirfoilLowerPnts::Can't Find XSec " + xsec_id  );
        return pnt_vec;
    }

    if ( xs->GetXSecCurve()->GetType() != XS_FILE_AIRFOIL )
    {
        ErrorMgr.AddError( VSP_WRONG_XSEC_TYPE, "GetAirfoilLowerPnts::XSec Not XS_FILE_AIRFOIL Type"  );
        return pnt_vec;
    }

    FileAirfoil* file_xs = dynamic_cast<FileAirfoil*>( xs->GetXSecCurve() );
    assert( file_xs );
    pnt_vec = file_xs->GetLowerPnts();
    ErrorMgr.NoError();
    return pnt_vec;
}

std::vector<double> GetUpperCSTCoefs( const string& xsec_id )
{
    vector < double > ret_vec;

    XSec* xs = FindXSec( xsec_id );
    if ( !xs )
    {
        ErrorMgr.AddError( VSP_INVALID_PTR, "GetUpperCSTCoefs::Can't Find XSec " + xsec_id  );
        return ret_vec;
    }

    if ( xs->GetXSecCurve()->GetType() != XS_CST_AIRFOIL )
    {
        ErrorMgr.AddError( VSP_WRONG_XSEC_TYPE, "GetUpperCSTCoefs::XSec Not XS_CST_AIRFOIL Type"  );
        return ret_vec;
    }

    CSTAirfoil* cst_xs = dynamic_cast<CSTAirfoil*>( xs->GetXSecCurve() );
    assert( cst_xs );

    ret_vec = cst_xs->GetUpperCST();
    ErrorMgr.NoError();
    return ret_vec;
}

std::vector<double> GetLowerCSTCoefs( const string& xsec_id )
{
    vector < double > ret_vec;

    XSec* xs = FindXSec( xsec_id );
    if ( !xs )
    {
        ErrorMgr.AddError( VSP_INVALID_PTR, "GetLowerCSTCoefs::Can't Find XSec " + xsec_id  );
        return ret_vec;
    }

    if ( xs->GetXSecCurve()->GetType() != XS_CST_AIRFOIL )
    {
        ErrorMgr.AddError( VSP_WRONG_XSEC_TYPE, "GetLowerCSTCoefs::XSec Not XS_CST_AIRFOIL Type"  );
        return ret_vec;
    }

    CSTAirfoil* cst_xs = dynamic_cast<CSTAirfoil*>( xs->GetXSecCurve() );
    assert( cst_xs );

    ret_vec = cst_xs->GetLowerCST();
    ErrorMgr.NoError();
    return ret_vec;
}

int GetUpperCSTDegree( const string& xsec_id )
{
    int deg = -1;

    XSec* xs = FindXSec( xsec_id );
    if ( !xs )
    {
        ErrorMgr.AddError( VSP_INVALID_PTR, "GetUpperCSTDegree::Can't Find XSec " + xsec_id  );
        return deg;
    }

    if ( xs->GetXSecCurve()->GetType() != XS_CST_AIRFOIL )
    {
        ErrorMgr.AddError( VSP_WRONG_XSEC_TYPE, "GetUpperCSTDegree::XSec Not XS_CST_AIRFOIL Type"  );
        return deg;
    }

    CSTAirfoil* cst_xs = dynamic_cast<CSTAirfoil*>( xs->GetXSecCurve() );
    assert( cst_xs );

    deg = cst_xs->GetUpperDegree();
    ErrorMgr.NoError();
    return deg;
}

int GetLowerCSTDegree( const string& xsec_id )
{
    int deg = -1;

    XSec* xs = FindXSec( xsec_id );
    if ( !xs )
    {
        ErrorMgr.AddError( VSP_INVALID_PTR, "GetLowerCSTDegree::Can't Find XSec " + xsec_id  );
        return deg;
    }

    if ( xs->GetXSecCurve()->GetType() != XS_CST_AIRFOIL )
    {
        ErrorMgr.AddError( VSP_WRONG_XSEC_TYPE, "GetLowerCSTDegree::XSec Not XS_CST_AIRFOIL Type"  );
        return deg;
    }

    CSTAirfoil* cst_xs = dynamic_cast<CSTAirfoil*>( xs->GetXSecCurve() );
    assert( cst_xs );

    deg = cst_xs->GetLowerDegree();
    ErrorMgr.NoError();
    return deg;
}

void SetUpperCST( const string& xsec_id, int deg, const std::vector<double> &coefs )
{
    XSec* xs = FindXSec( xsec_id );
    if ( !xs )
    {
        ErrorMgr.AddError( VSP_INVALID_PTR, "SetUpperCST::Can't Find XSec " + xsec_id  );
        return;
    }

    if ( xs->GetXSecCurve()->GetType() != XS_CST_AIRFOIL )
    {
        ErrorMgr.AddError( VSP_WRONG_XSEC_TYPE, "SetUpperCST::XSec Not XS_CST_AIRFOIL Type"  );
        return;
    }

    CSTAirfoil* cst_xs = dynamic_cast<CSTAirfoil*>( xs->GetXSecCurve() );
    assert( cst_xs );

    ErrorMgr.NoError();
    cst_xs->SetUpperCST( deg, coefs );
}

void SetLowerCST( const string& xsec_id, int deg, const std::vector<double> &coefs )
{
    XSec* xs = FindXSec( xsec_id );
    if ( !xs )
    {
        ErrorMgr.AddError( VSP_INVALID_PTR, "SetLowerCST::Can't Find XSec " + xsec_id  );
        return;
    }

    if ( xs->GetXSecCurve()->GetType() != XS_CST_AIRFOIL )
    {
        ErrorMgr.AddError( VSP_WRONG_XSEC_TYPE, "SetLowerCST::XSec Not XS_CST_AIRFOIL Type"  );
        return;
    }

    CSTAirfoil* cst_xs = dynamic_cast<CSTAirfoil*>( xs->GetXSecCurve() );
    assert( cst_xs );

    ErrorMgr.NoError();
    cst_xs->SetLowerCST( deg, coefs );
}

void PromoteCSTUpper( const string& xsec_id )
{
    XSec* xs = FindXSec( xsec_id );
    if ( !xs )
    {
        ErrorMgr.AddError( VSP_INVALID_PTR, "PromoteCSTUpper::Can't Find XSec " + xsec_id  );
        return;
    }

    if ( xs->GetXSecCurve()->GetType() != XS_CST_AIRFOIL )
    {
        ErrorMgr.AddError( VSP_WRONG_XSEC_TYPE, "PromoteCSTUpper::XSec Not XS_CST_AIRFOIL Type"  );
        return;
    }

    CSTAirfoil* cst_xs = dynamic_cast<CSTAirfoil*>( xs->GetXSecCurve() );
    assert( cst_xs );

    ErrorMgr.NoError();
    cst_xs->PromoteUpper();
}

void PromoteCSTLower( const string& xsec_id )
{
    XSec* xs = FindXSec( xsec_id );
    if ( !xs )
    {
        ErrorMgr.AddError( VSP_INVALID_PTR, "PromoteCSTLower::Can't Find XSec " + xsec_id  );
        return;
    }

    if ( xs->GetXSecCurve()->GetType() != XS_CST_AIRFOIL )
    {
        ErrorMgr.AddError( VSP_WRONG_XSEC_TYPE, "PromoteCSTLower::XSec Not XS_CST_AIRFOIL Type"  );
        return;
    }

    CSTAirfoil* cst_xs = dynamic_cast<CSTAirfoil*>( xs->GetXSecCurve() );
    assert( cst_xs );

    ErrorMgr.NoError();
    cst_xs->PromoteLower();
}

void DemoteCSTUpper( const string& xsec_id )
{
    XSec* xs = FindXSec( xsec_id );
    if ( !xs )
    {
        ErrorMgr.AddError( VSP_INVALID_PTR, "DemoteCSTUpper::Can't Find XSec " + xsec_id  );
        return;
    }

    if ( xs->GetXSecCurve()->GetType() != XS_CST_AIRFOIL )
    {
        ErrorMgr.AddError( VSP_WRONG_XSEC_TYPE, "DemoteCSTUpper::XSec Not XS_CST_AIRFOIL Type"  );
        return;
    }

    CSTAirfoil* cst_xs = dynamic_cast<CSTAirfoil*>( xs->GetXSecCurve() );
    assert( cst_xs );

    ErrorMgr.NoError();
    cst_xs->DemoteUpper();
}

void DemoteCSTLower( const string& xsec_id )
{
    XSec* xs = FindXSec( xsec_id );
    if ( !xs )
    {
        ErrorMgr.AddError( VSP_INVALID_PTR, "DemoteCSTLower::Can't Find XSec " + xsec_id  );
        return;
    }

    if ( xs->GetXSecCurve()->GetType() != XS_CST_AIRFOIL )
    {
        ErrorMgr.AddError( VSP_WRONG_XSEC_TYPE, "DemoteCSTLower::XSec Not XS_CST_AIRFOIL Type"  );
        return;
    }

    CSTAirfoil* cst_xs = dynamic_cast<CSTAirfoil*>( xs->GetXSecCurve() );
    assert( cst_xs );

    ErrorMgr.NoError();
    cst_xs->DemoteLower();
}

void FitAfCST( const string & xsec_surf_id, int xsec_index, int deg )
{
    XSecSurf* xsec_surf = FindXSecSurf( xsec_surf_id );
    if ( !xsec_surf )
    {
        ErrorMgr.AddError( VSP_INVALID_PTR, "FitAfCST::Can't Find XSecSurf " + xsec_surf_id  );
        return;
    }
    XSec* xsec = xsec_surf->FindXSec( xsec_index );
    if ( !xsec )
    {
        ErrorMgr.AddError( VSP_INVALID_PTR, "FitAfCST::Can't Find XSec " + xsec_surf_id + ":" + to_string( ( long long )xsec_index )  );
        return;
    }

    if ( ( xsec->GetXSecCurve()->GetType() != XS_FOUR_SERIES ) ||
         ( xsec->GetXSecCurve()->GetType() != XS_SIX_SERIES ) ||
         ( xsec->GetXSecCurve()->GetType() != XS_FILE_AIRFOIL ) )
    {
        ErrorMgr.AddError( VSP_WRONG_XSEC_TYPE, "FitAfCST::XSec Not Fittable Airfoil Type"  );
        return;
    }

    XSecCurve* xsc = xsec->GetXSecCurve();
    if ( !xsc )
    {
        ErrorMgr.AddError( VSP_INVALID_PTR, "FitAfCST::Can't Get XSecCurve" );
        return;
    }

    Airfoil* af_xs = dynamic_cast< Airfoil* >( xsc );
    if ( !af_xs )
    {
        ErrorMgr.AddError( VSP_INVALID_PTR, "FitAfCST::Can't Get Airfoil" );
        return;
    }

    VspCurve c = af_xs->GetOrigCurve();

    xsec_surf->ChangeXSecShape( xsec_index, XS_CST_AIRFOIL );

    XSec* newxsec = xsec_surf->FindXSec( xsec_index );
    if ( !newxsec )
    {
        ErrorMgr.AddError( VSP_INVALID_PTR, "FitAfCST::Can't Find New XSec " + xsec_surf_id + ":" + to_string( ( long long )xsec_index )  );
        return;
    }

    XSecCurve* newxsc = newxsec->GetXSecCurve();
    if ( !newxsc )
    {
        ErrorMgr.AddError( VSP_INVALID_PTR, "FitAfCST::Can't Get New XSecCurve" );
        return;
    }

    if ( newxsc->GetType() != XS_CST_AIRFOIL )
    {
        ErrorMgr.AddError( VSP_WRONG_XSEC_TYPE, "FitAfCST::XSec Not XS_CST_AIRFOIL Type"  );
        return;
    }

    CSTAirfoil* cst_xs = dynamic_cast< CSTAirfoil* >( newxsc );

    assert( cst_xs );
    cst_xs->FitCurve( c, deg );

    ErrorMgr.NoError();
}

//===================================================================//
//===============       Set Functions            ===================//
//===================================================================//

/// Get the total number of defined sets.  Named sets are used to group components
/// and perform read/write or operations on them
int GetNumSets()
{
    Vehicle* veh = GetVehicle();
    return veh->GetSetNameVec().size();
}

/// Set the set name at the provided index. Index between 0 and NumSets.
void SetSetName( int index, const string& name )
{
    Vehicle* veh = GetVehicle();
    veh->SetSetName( index, name );
}

/// Get the set name at the provided index
string GetSetName( int index )
{
    Vehicle* veh = GetVehicle();
    vector< string > name_vec =  veh->GetSetNameVec();
    if ( index < 0 || index >= ( int )name_vec.size() )
    {
        ErrorMgr.AddError( VSP_INDEX_OUT_RANGE, "GetSetName::Index Out Of Range " + to_string( ( long long )index )  );
        return string();
    }
    ErrorMgr.NoError();
    return name_vec[index];
}

/// Get the set of geom ids at the given set index
vector< string > GetGeomSetAtIndex( int index )
{
    Vehicle* veh = GetVehicle();
    return veh->GetGeomSet( index );
}

/// Get the set of geom ids at the given set name
vector< string > GetGeomSet( const string & name )
{
    Vehicle* veh = GetVehicle();
    vector< string > name_vec =  veh->GetSetNameVec();
    int index = -1;
    for ( int i = 0 ; i < ( int )name_vec.size() ; i++ )
    {
        if ( name == name_vec[i] )
        {
            index = i;
        }
    }
    if ( index == -1 )
    {
        vector< string > ret_vec;
        ErrorMgr.AddError( VSP_CANT_FIND_NAME, "GetGeomSet::Can't Find Name " + name  );
        return ret_vec;
    }
    ErrorMgr.NoError();
    return veh->GetGeomSet( index );
}

/// Get the set index given the set name
int GetSetIndex( const string & name )
{
    Vehicle* veh = GetVehicle();
    vector< string > name_vec =  veh->GetSetNameVec();
    int index = -1;
    for ( int i = 0 ; i < ( int )name_vec.size() ; i++ )
    {
        if ( name == name_vec[i] )
        {
            index = i;
        }
    }
    if ( index == -1 )
    {
        ErrorMgr.AddError( VSP_CANT_FIND_NAME, "GetSetIndex::Can't Find Name " + name  );
        return index;
    }

    ErrorMgr.NoError();
    return index;
}

/// Check if geom in set (given index)
bool GetSetFlag( const string & geom_id, int set_index )
{
    Vehicle* veh = GetVehicle();
    Geom* geom_ptr = veh->FindGeom( geom_id );
    if ( !geom_ptr )
    {
        ErrorMgr.AddError( VSP_INVALID_PTR, "GetSetFlag::Can't Find Geom " + geom_id );
        return false;
    }
    ErrorMgr.NoError();
    return geom_ptr->GetSetFlag( set_index );
}

/// Set set flag for geom given set index
void SetSetFlag( const string & geom_id, int set_index, bool flag )
{
    Vehicle* veh = GetVehicle();
    Geom* geom_ptr = veh->FindGeom( geom_id );
    if ( !geom_ptr )
    {
        ErrorMgr.AddError( VSP_INVALID_PTR, "SetSetFlag::Can't Find Geom " + geom_id );
        return;
    }
    if ( set_index < 0 || set_index > ( int )veh->GetSetNameVec().size() )
    {
        ErrorMgr.AddError( VSP_INDEX_OUT_RANGE, "SetSetFlag::Invalid Set Index " + to_string( ( long long ) set_index ) );
        return;
    }


    ErrorMgr.NoError();

    geom_ptr->SetSetFlag( set_index, flag );
}

//===================================================================//
//===============       Parm Functions            ===================//
//===================================================================//

/// Check if the given parm id is valid
bool ValidParm( const string & parm_id )
{
    Parm* p = ParmMgr.FindParm( parm_id );
    if ( !p )
    {
        return false;
    }

    return true;
}

/// Set the parm value.
/// The final value of parm is returned.
double SetParmVal( const string & parm_id, double val )
{
    Parm* p = ParmMgr.FindParm( parm_id );
    if ( !p )
    {
        ErrorMgr.AddError( VSP_CANT_FIND_PARM, "SetParmVal::Can't Find Parm " + parm_id  );
        return val;
    }
    ErrorMgr.NoError();
    return p->Set( val );
}

/// Set the parm value.  If update is true, the parm container is updated.
/// The final value of parm is returned.
double SetParmVal( const string & geom_id, const string & name, const string & group, double val )
{
    string parm_id = GetParm( geom_id, name, group );
    Parm* p = ParmMgr.FindParm( parm_id );
    if ( !p )
    {
        ErrorMgr.AddError( VSP_CANT_FIND_PARM, "SetParmVal::Can't Find Parm " + parm_id  );
        return val;
    }
    ErrorMgr.NoError();
    return p->Set( val );
}

double SetParmValLimits( const string & parm_id, double val, double lower_limit, double upper_limit )
{
    Parm* p = ParmMgr.FindParm( parm_id );
    if ( !p )
    {
        ErrorMgr.AddError( VSP_CANT_FIND_PARM, "SetParmValLimits::Can't Find Parm " + parm_id  );
        return val;
    }
    ErrorMgr.NoError();

    p->SetLowerUpperLimits( lower_limit, upper_limit );
    return p->Set( val );
}


/// Set the parm value.
/// The final value of parm is returned.
double SetParmValUpdate( const string & parm_id, double val )
{
    Parm* p = ParmMgr.FindParm( parm_id );
    if ( !p )
    {
        ErrorMgr.AddError( VSP_CANT_FIND_PARM, "SetParmVal::Can't Find Parm " + parm_id  );
        return val;
    }
    ErrorMgr.NoError();
    return p->SetFromDevice( val );         // Force Update
}


/// Set the parm value.  If update is true, the parm container is updated.
/// The final value of parm is returned.
double SetParmValUpdate( const string & geom_id, const string & parm_name, const string & parm_group_name, double val )
{
    string parm_id = GetParm( geom_id, parm_name, parm_group_name );
    Parm* p = ParmMgr.FindParm( parm_id );
    if ( !p )
    {
        ErrorMgr.AddError( VSP_CANT_FIND_PARM, "SetParmVal::Can't Find Parm " + parm_id  );
        return val;
    }
    ErrorMgr.NoError();
    return p->SetFromDevice( val );         // Force Update
}

/// Get the value of parm
double GetParmVal( const string & parm_id )
{
    Parm* p = ParmMgr.FindParm( parm_id );
    if ( !p )
    {
        ErrorMgr.AddError( VSP_CANT_FIND_PARM, "GetParmVal::Can't Find Parm " + parm_id  );
        return 0.0;
    }
    ErrorMgr.NoError();
    return p->Get();
}

/// Get the value of parm
double GetParmVal( const string & geom_id, const string & name, const string & group )
{
    string parm_id = GetParm( geom_id, name, group );
    Parm* p = ParmMgr.FindParm( parm_id );
    if ( !p )
    {
        ErrorMgr.AddError( VSP_CANT_FIND_PARM, "GetParmVal::Can't Find Parm " + name  );
        return 0.0;
    }
    ErrorMgr.NoError();
    return p->Get();
}

/// Get the value of parm
int GetIntParmVal( const string & parm_id )
{
    Parm* p = ParmMgr.FindParm( parm_id );
    if ( !p )
    {
        ErrorMgr.AddError( VSP_CANT_FIND_PARM, "GetParmVal::Can't Find Parm " + parm_id  );
        return 0;
    }
    ErrorMgr.NoError();
    return (int)(p->Get()+0.5);
}

/// Get the value of parm
bool GetBoolParmVal( const string & parm_id )
{
    Parm* p = ParmMgr.FindParm( parm_id );
    if ( !p )
    {
        ErrorMgr.AddError( VSP_CANT_FIND_PARM, "GetParmVal::Can't Find Parm " + parm_id  );
        return false;
    }
    if ( p->GetType() != PARM_BOOL_TYPE )
    {
        return false;
    }
    ErrorMgr.NoError();

    BoolParm* bp = dynamic_cast<BoolParm*>( p );
    return bp->Get();
}

/// Set the upper limit of parm
void SetParmUpperLimit( const string & parm_id, double val )
{
    Parm* p = ParmMgr.FindParm( parm_id );
    if ( !p )
    {
        ErrorMgr.AddError( VSP_CANT_FIND_PARM, "SetParmUpperLimit::Can't Find Parm " + parm_id  );
        return;
    }
    ErrorMgr.NoError();
    p->SetUpperLimit( val );
}

/// Get upper limit of parm
double GetParmUpperLimit( const string & parm_id )
{
    Parm* p = ParmMgr.FindParm( parm_id );
    if ( !p )
    {
        ErrorMgr.AddError( VSP_CANT_FIND_PARM, "GetParmUpperLimit::Can't Find Parm " + parm_id  );
        return 0.0;
    }
    ErrorMgr.NoError();
    return p->GetUpperLimit();
}

/// Set the lower limit of parm
void SetParmLowerLimit( const string & parm_id, double val )
{
    Parm* p = ParmMgr.FindParm( parm_id );
    if ( !p )
    {
        ErrorMgr.AddError( VSP_CANT_FIND_PARM, "SetParmLowerLimit::Can't Find Parm " + parm_id  );
        return;
    }
    ErrorMgr.NoError();
    p->SetLowerLimit( val );
}

/// Get the lower limit of parm
double GetParmLowerLimit( const string & parm_id )
{
    Parm* p = ParmMgr.FindParm( parm_id );
    if ( !p )
    {
        ErrorMgr.AddError( VSP_CANT_FIND_PARM, "GetParmLowerLimit::Can't Find Parm " + parm_id  );
        return 0.0;
    }
    ErrorMgr.NoError();
    return p->GetLowerLimit();
}

/// Get the parm type.
/// 0 = Double
/// 1 = Int
/// 2 = Bool
/// 3 = Fraction
int GetParmType( const string & parm_id )
{
    Parm* p = ParmMgr.FindParm( parm_id );
    if ( !p )
    {
        ErrorMgr.AddError( VSP_CANT_FIND_PARM, "GetParmType::Can't Find Parm " + parm_id  );
        return PARM_DOUBLE_TYPE;
    }
    ErrorMgr.NoError();
    return p->GetType();
}

/// Get the parm name
string GetParmName( const string & parm_id )
{
    Parm* p = ParmMgr.FindParm( parm_id );
    if ( !p )
    {
        ErrorMgr.AddError( VSP_CANT_FIND_PARM, "GetParmName::Can't Find Parm " + parm_id  );
        return string();
    }
    ErrorMgr.NoError();
    return p->GetName();
}

/// Get the parm group name
string GetParmGroupName( const string & parm_id )
{
    Parm* p = ParmMgr.FindParm( parm_id );
    if ( !p )
    {
        ErrorMgr.AddError( VSP_CANT_FIND_PARM, "GetParmGroup::Can't Find Parm " + parm_id  );
        return string();
    }
    ErrorMgr.NoError();
    return p->GetGroupName();
}

/// Get the parm display group name
string GetParmDisplayGroupName( const string & parm_id )
{
    Parm* p = ParmMgr.FindParm( parm_id );
    if ( !p )
    {
        ErrorMgr.AddError( VSP_CANT_FIND_PARM, "GetParmDisplayGroup::Can't Find Parm " + parm_id  );
        return string();
    }
    ErrorMgr.NoError();
    return p->GetDisplayGroupName();
}

/// The the parm container id
string GetParmContainer( const string & parm_id )
{
    Parm* p = ParmMgr.FindParm( parm_id );
    if ( !p )
    {
        ErrorMgr.AddError( VSP_CANT_FIND_PARM, "GetParmContainer::Can't Find Parm " + parm_id  );
        return string();
    }
    ErrorMgr.NoError();
    return p->GetContainerID();
}

/// Set the parm desciption
void SetParmDescript( const string & parm_id, const string & desc )
{
    Parm* p = ParmMgr.FindParm( parm_id );
    if ( !p )
    {
        ErrorMgr.AddError( VSP_CANT_FIND_PARM, "SetParmDescript::Can't Find Parm " + parm_id  );
        return;
    }
    ErrorMgr.NoError();
    return p->SetDescript( desc );
}

///  Find a parm id given parm container, name and group
string FindParm( const string & parm_container_id, const string& parm_name, const string& group_name )
{
    ParmContainer* pc = ParmMgr.FindParmContainer( parm_container_id );

    if ( !pc )
    {
        ErrorMgr.AddError( VSP_INVALID_ID, "FindParm::Can't Find Parm Container " + parm_container_id  );
        return string();
    }

   string parm_id = pc->FindParm( parm_name, group_name );
   Parm* p = ParmMgr.FindParm( parm_id );
   if ( !p )
   {
        ErrorMgr.AddError( VSP_CANT_FIND_PARM, "FindParm::Can't Find Parm " + parm_id  );
        return string();
   }
   ErrorMgr.NoError();

   return parm_id;
}


//===================================================================//
//===============       Parm Container Functions       ==============//
//===================================================================//

vector< std::string > FindContainers()
{
    vector< string > containerVec;
    LinkMgr.BuildLinkableParmData();
    LinkMgr.GetAllContainerVec( containerVec );

    ErrorMgr.NoError();
    return containerVec;
}

vector< std::string > FindContainersWithName( const string & name )
{
    vector< string > containerVec;
    vector< string > ret_vec;
    LinkMgr.BuildLinkableParmData();
    LinkMgr.GetAllContainerVec( containerVec );

    for ( int i = 0 ; i < ( int )containerVec.size() ; i++ )
    {
        ParmContainer* pc = ParmMgr.FindParmContainer( containerVec[i]  );

        if ( pc && pc->GetName() == name )
        {
            ret_vec.push_back( containerVec[i] );
        }
    }
    ErrorMgr.NoError();
    return ret_vec;
}

string FindContainer( const string & name, int index )
{
    vector< string > containerVec;
    vector< string > id_vec;
    LinkMgr.BuildLinkableParmData();
    LinkMgr.GetAllContainerVec( containerVec );

    for ( int i = 0 ; i < ( int )containerVec.size() ; i++ )
    {
        ParmContainer* pc = ParmMgr.FindParmContainer( containerVec[i]  );

        if ( pc && pc->GetName() == name )
        {
            id_vec.push_back( containerVec[i] );
        }
    }

    if ( index < 0 || index >= ( int )id_vec.size() )
    {
        ErrorMgr.AddError( VSP_INVALID_GEOM_ID, "FindContainer::Can't Find Name " + name + " or Index" + to_string( ( long long )index ) );
        return string();
    }
    ErrorMgr.NoError();
    return id_vec[index];
}

string GetContainerName( const string & parm_container_id )
{
    string ret_name;

    ParmContainer* pc = ParmMgr.FindParmContainer( parm_container_id );

    if ( !pc )
    {
        ErrorMgr.AddError( VSP_INVALID_ID, "GetContainerName::Can't Find Parm Container " + parm_container_id  );
        return string();
    }

    ret_name = pc->GetName();
    ErrorMgr.NoError();
    return ret_name;
}

vector< string > FindContainerGroupNames( const string & parm_container_id )
{
    vector< string > ret_names;

    ParmContainer* pc = ParmMgr.FindParmContainer( parm_container_id );

    if ( !pc )
    {
        ErrorMgr.AddError( VSP_INVALID_ID, "FindContainerGroupNames::Can't Find Parm Container " + parm_container_id  );
        return ret_names;
    }

    pc->GetGroupNames( ret_names );

    ErrorMgr.NoError();
    return ret_names;
}

vector< string > FindContainerParmIDs( const string & parm_container_id )
{
    vector< string > parm_vec;

    ParmContainer* pc = ParmMgr.FindParmContainer( parm_container_id );

    if ( !pc )
    {
        ErrorMgr.AddError( VSP_INVALID_ID, "FindContainerParmIDs::Can't Find Parm Container " + parm_container_id  );
        return parm_vec;
    }

    pc->AddLinkableParms( parm_vec );

    ErrorMgr.NoError();
    return parm_vec;
}


//===================================================================//
//===============           Snap To Functions          ==============//
//===================================================================//
double ComputeMinClearanceDistance( const string & geom_id, int set )
{
    Vehicle* veh = GetVehicle();

    int old_set = veh->GetSnapToPtr()->m_CollisionSet;
    veh->GetSnapToPtr()->m_CollisionSet = set;

    vector< string > old_active_geom = veh->GetActiveGeomVec();
    veh->SetActiveGeom( geom_id );

    veh->GetSnapToPtr()->CheckClearance();
    double min_clearance_dist = veh->GetSnapToPtr()->m_CollisionMinDist;

    //==== Restore State ====//
    veh->GetSnapToPtr()->m_CollisionSet = old_set;
    veh->SetActiveGeomVec( old_active_geom );

    return min_clearance_dist;
}

double SnapParm( const string & parm_id, double target_min_dist, bool inc_flag, int set  )
{
    Vehicle* veh = GetVehicle();

    int old_set = veh->GetSnapToPtr()->m_CollisionSet;
    veh->GetSnapToPtr()->m_CollisionSet = set;

    double old_min_dist = veh->GetSnapToPtr()->m_CollisionTargetDist();
    veh->GetSnapToPtr()->m_CollisionTargetDist = target_min_dist;

    veh->GetSnapToPtr()->AdjParmToMinDist( parm_id, inc_flag );
    double min_clearance_dist = veh->GetSnapToPtr()->m_CollisionMinDist;

    //==== Restore State ====//
    veh->GetSnapToPtr()->m_CollisionSet = old_set;
    veh->GetSnapToPtr()->m_CollisionTargetDist = old_min_dist;

    return min_clearance_dist;
}

//===================================================================//
//===============     Variable Presets Functions       ==============//
//===================================================================//

void AddVarPresetGroup( const string &group_name )
{
    VarPresetMgr.AddGroup( group_name );
    VarPresetMgr.SavePreset();

    ErrorMgr.NoError();
}

void AddVarPresetSetting( const string &setting_name )
{
    VarPresetMgr.AddSetting( setting_name );
    VarPresetMgr.SavePreset();

    ErrorMgr.NoError();
}

void AddVarPresetParm( const string &parm_ID )
{
    VarPresetMgr.AddVar( parm_ID );
    VarPresetMgr.SavePreset();

    ErrorMgr.NoError();
}

void AddVarPresetParm( const string &parm_ID, const string &group_name )
{
    VarPresetMgr.GroupChange( group_name );
    VarPresetMgr.AddVar( parm_ID );
    VarPresetMgr.SavePreset();

    ErrorMgr.NoError();
}

void EditVarPresetParm( const string &parm_ID, double parm_val )
{
    Parm *p = ParmMgr.FindParm( parm_ID );
    if ( p )
    {
        p->Set( parm_val );
    }
    else
    {
        ErrorMgr.AddError( VSP_INVALID_ID, "EditVarPresetParm::Can't Find Parm " + parm_ID  );
    }
    VarPresetMgr.SavePreset();
}

void EditVarPresetParm( const string &parm_ID, double parm_val, const string &group_name,
    const string &setting_name )
{
    SwitchVarPreset( group_name, setting_name );
    EditVarPresetParm( parm_ID, parm_val );
}

void DeleteVarPresetParm( const string &parm_ID )
{
    VarPresetMgr.SetWorkingParmID( parm_ID );
    VarPresetMgr.DelCurrVar();
    VarPresetMgr.SavePreset();

    ErrorMgr.NoError();
}

void DeleteVarPresetParm( const string &parm_ID, const string &group_name )
{
    VarPresetMgr.GroupChange( group_name );
    if (VarPresetMgr.GetActiveGroupText().compare( group_name ) == 0 )
    {
        ErrorMgr.NoError();
    }
    else
    {
        ErrorMgr.AddError( VSP_INVALID_VARPRESET_GROUPNAME, "DeleteVarPresetParm::Can't Find Group " + group_name );
    }
    DeleteVarPresetParm( parm_ID );
}

void SwitchVarPreset( const string &group_name, const string &setting_name )
{
    VarPresetMgr.GroupChange( group_name );
    if (VarPresetMgr.GetActiveGroupText().compare( group_name ) == 0 )
    {
        ErrorMgr.NoError();
    }
    else
    {
        ErrorMgr.AddError( VSP_INVALID_VARPRESET_GROUPNAME, "SwitchVarPreset::Can't Find Group " + group_name );
    }
    VarPresetMgr.SettingChange( setting_name );
    if (VarPresetMgr.GetActiveSettingText().compare( setting_name ) == 0 )
    {
        ErrorMgr.NoError();
    }
    else
    {
        ErrorMgr.AddError( VSP_INVALID_VARPRESET_SETNAME, "SwitchSaveParmGroup::Can't Find Setting " + setting_name  );
    }
}

bool DeleteVarPresetSet( const string &group_name, const string &setting_name )
{
    if ( VarPresetMgr.DeletePreset( group_name, setting_name ) )
    {
        ErrorMgr.NoError();
        return true;
    }
    else
    {
        ErrorMgr.AddError( VSP_INVALID_VARPRESET_GROUPNAME, "SwitchSaveParmGroup::Can't Find Group " + group_name  );
        ErrorMgr.AddError( VSP_INVALID_VARPRESET_SETNAME, "SwitchSaveParmGroup::Can't Find Setting " + setting_name  );
        return false;
    }
}

string GetCurrentGroupName()
{
    return VarPresetMgr.GetActiveGroupText();
}

string GetCurrentSettingName()
{
    return VarPresetMgr.GetActiveSettingText();
}

vector <string> GetVarPresetGroupNames()
{
    ErrorMgr.NoError();
    return VarPresetMgr.GetGroupNames();
}

vector <string> GetVarPresetSettingNamesWName( const string &group_name )
{
    vector <string> vec;
    vec = VarPresetMgr.GetSettingNames( group_name );

    if ( vec.empty() )
    {
        ErrorMgr.AddError( VSP_INVALID_VARPRESET_GROUPNAME, "SwitchSaveParmGroup::Can't Find Group " + group_name  );
        return vec;
    }
    else
    {
        ErrorMgr.NoError();
        return vec;
    }
}

vector <string> GetVarPresetSettingNamesWIndex( int group_index )
{
    vector <string> vec;
    vec = VarPresetMgr.GetSettingNames( group_index );

    if ( vec.empty() )
    {
        ErrorMgr.AddError( VSP_INVALID_VARPRESET_GROUPNAME, "SwitchSaveParmGroup::Can't Find Group @ Index " + to_string( group_index ) );
        return vec;
    }
    else
    {
        ErrorMgr.NoError();
        return vec;
    }
}

vector <double> GetVarPresetParmVals()
{
    ErrorMgr.NoError();
    return VarPresetMgr.GetCurrentParmVals();
}

vector <double> GetVarPresetParmValsWNames( const string &group_name, const string &setting_name )
{
    ErrorMgr.NoError();
    return VarPresetMgr.GetParmVals( group_name, setting_name );
}

vector <string> GetVarPresetParmIDs()
{
    ErrorMgr.NoError();
    return VarPresetMgr.GetCurrentParmIDs();
}

vector <string> GetVarPresetParmIDsWName( const string &group_name )
{
    ErrorMgr.NoError();
    return VarPresetMgr.GetParmIDs( group_name );
}

//===================================================================//
//===============     Parametric Curve Functions       ==============//
//===================================================================//

void SetPCurve( const string & geom_id, const int & pcurveid, const vector < double > & tvec,
    const vector < double > & valvec, const int & newtype )
{
    Vehicle* veh = GetVehicle();
    Geom* geom_ptr = veh->FindGeom( geom_id );
    if ( !geom_ptr )
    {
        ErrorMgr.AddError( VSP_INVALID_PTR, "SetPCurve::Can't Find Geom " + geom_id );
        return;
    }

    PropGeom* prop_ptr = dynamic_cast < PropGeom* > (geom_ptr );
    if ( !prop_ptr )
    {
        ErrorMgr.AddError( VSP_INVALID_PTR, "SetPCurve::Geom doesn't support PCurves " + geom_id );
        return;
    }

    PCurve *pc = NULL;

    if ( prop_ptr )
    {
        pc = prop_ptr->GetPCurve( pcurveid );
    }

    if ( !pc )
    {
        ErrorMgr.AddError( VSP_INVALID_ID, "SetPCurve::PCurve not found " + geom_id + " " + to_string( pcurveid ) );
        return;
    }

    pc->SetCurve( tvec, valvec, newtype );

    ErrorMgr.NoError();
}

void PCurveConvertTo( const string & geom_id, const int & pcurveid, const int & newtype )
{
    Vehicle* veh = GetVehicle();
    Geom* geom_ptr = veh->FindGeom( geom_id );
    if ( !geom_ptr )
    {
        ErrorMgr.AddError( VSP_INVALID_PTR, "PCurveConvertTo::Can't Find Geom " + geom_id );
        return;
    }

    PropGeom* prop_ptr = dynamic_cast < PropGeom* > (geom_ptr );
    if ( !prop_ptr )
    {
        ErrorMgr.AddError( VSP_INVALID_PTR, "PCurveConvertTo::Geom doesn't support PCurves " + geom_id );
        return;
    }

    PCurve *pc = NULL;

    if ( prop_ptr )
    {
        pc = prop_ptr->GetPCurve( pcurveid );
    }

    if ( !pc )
    {
        ErrorMgr.AddError( VSP_INVALID_ID, "PCurveConvertTo::PCurve not found " + geom_id + " " + to_string( pcurveid ) );
        return;
    }

    pc->ConvertTo( newtype );

    ErrorMgr.NoError();
}

int PCurveGetType( const std::string & geom_id, const int & pcurveid )
{
    Vehicle* veh = GetVehicle();
    Geom* geom_ptr = veh->FindGeom( geom_id );
    if ( !geom_ptr )
    {
        ErrorMgr.AddError( VSP_INVALID_PTR, "PCurveGetType::Can't Find Geom " + geom_id );
        return -1;
    }

    PropGeom* prop_ptr = dynamic_cast < PropGeom* > (geom_ptr );
    if ( !prop_ptr )
    {
        ErrorMgr.AddError( VSP_INVALID_PTR, "PCurveGetType::Geom doesn't support PCurves " + geom_id );
        return -1;
    }

    PCurve *pc = NULL;

    if ( prop_ptr )
    {
        pc = prop_ptr->GetPCurve( pcurveid );
    }

    if ( !pc )
    {
        ErrorMgr.AddError( VSP_INVALID_ID, "PCurveGetType::PCurve not found " + geom_id + " " + to_string( pcurveid ) );
        return -1;
    }

    ErrorMgr.NoError();

    return pc->m_CurveType();
}

vector < double > PCurveGetTVec( const string & geom_id, const int & pcurveid )
{
    vector < double > retvec;

    Vehicle* veh = GetVehicle();
    Geom* geom_ptr = veh->FindGeom( geom_id );
    if ( !geom_ptr )
    {
        ErrorMgr.AddError( VSP_INVALID_PTR, "PCurveGetTVec::Can't Find Geom " + geom_id );
        return retvec;
    }

    PropGeom* prop_ptr = dynamic_cast < PropGeom* > (geom_ptr );
    if ( !prop_ptr )
    {
        ErrorMgr.AddError( VSP_INVALID_PTR, "PCurveGetTVec::Geom doesn't support PCurves " + geom_id );
        return retvec;
    }

    PCurve *pc = NULL;

    if ( prop_ptr )
    {
        pc = prop_ptr->GetPCurve( pcurveid );
    }

    if ( !pc )
    {
        ErrorMgr.AddError( VSP_INVALID_ID, "PCurveGetTVec::PCurve not found " + geom_id + " " + to_string( pcurveid ) );
        return retvec;
    }

    retvec = pc->GetTVec();

    ErrorMgr.NoError();

    return retvec;
}

vector < double > PCurveGetValVec( const string & geom_id, const int & pcurveid )
{
    vector < double > retvec;

    Vehicle* veh = GetVehicle();
    Geom* geom_ptr = veh->FindGeom( geom_id );
    if ( !geom_ptr )
    {
        ErrorMgr.AddError( VSP_INVALID_PTR, "PCurveGetValVec::Can't Find Geom " + geom_id );
        return retvec;
    }

    PropGeom* prop_ptr = dynamic_cast < PropGeom* > (geom_ptr );
    if ( !prop_ptr )
    {
        ErrorMgr.AddError( VSP_INVALID_PTR, "PCurveGetValVec::Geom doesn't support PCurves " + geom_id );
        return retvec;
    }

    PCurve *pc = NULL;

    if ( prop_ptr )
    {
        pc = prop_ptr->GetPCurve( pcurveid );
    }

    if ( !pc )
    {
        ErrorMgr.AddError( VSP_INVALID_ID, "PCurveGetValVec::PCurve not found " + geom_id + " " + to_string( pcurveid ) );
        return retvec;
    }

    retvec = pc->GetValVec();

    ErrorMgr.NoError();

    return retvec;
}

void PCurveDeletePt( const string & geom_id, const int & pcurveid, const int & indx )
{
    Vehicle* veh = GetVehicle();
    Geom* geom_ptr = veh->FindGeom( geom_id );
    if ( !geom_ptr )
    {
        ErrorMgr.AddError( VSP_INVALID_PTR, "PCurveDeletePt::Can't Find Geom " + geom_id );
        return;
    }

    PropGeom* prop_ptr = dynamic_cast < PropGeom* > (geom_ptr );
    if ( !prop_ptr )
    {
        ErrorMgr.AddError( VSP_INVALID_PTR, "PCurveDeletePt::Geom doesn't support PCurves " + geom_id );
        return;
    }

    PCurve *pc = NULL;

    if ( prop_ptr )
    {
        pc = prop_ptr->GetPCurve( pcurveid );
    }

    if ( !pc )
    {
        ErrorMgr.AddError( VSP_INVALID_ID, "PCurveDeletePt::PCurve not found " + geom_id + " " + to_string( pcurveid ) );
        return;
    }

    return pc->DeletePt( indx );

    ErrorMgr.NoError();
}

void PCurveSplit( const string & geom_id, const int & pcurveid, const double & tsplit )
{
    Vehicle* veh = GetVehicle();
    Geom* geom_ptr = veh->FindGeom( geom_id );
    if ( !geom_ptr )
    {
        ErrorMgr.AddError( VSP_INVALID_PTR, "PCurveSplit::Can't Find Geom " + geom_id );
        return;
    }

    PropGeom* prop_ptr = dynamic_cast < PropGeom* > (geom_ptr );
    if ( !prop_ptr )
    {
        ErrorMgr.AddError( VSP_INVALID_PTR, "PCurveSplit::Geom doesn't support PCurves " + geom_id );
        return;
    }

    PCurve *pc = NULL;

    if ( prop_ptr )
    {
        pc = prop_ptr->GetPCurve( pcurveid );
    }

    if ( !pc )
    {
        ErrorMgr.AddError( VSP_INVALID_ID, "PCurveSplit::PCurve not found " + geom_id + " " + to_string( pcurveid ) );
        return;
    }

    return pc->Split( tsplit );

    ErrorMgr.NoError();
}

//===================================================================//
//===============    Parasite Drag Tool Functions      ==============//
//===================================================================//

void AddExcrescence(const std::string &excresName, const int & excresType, const double & excresVal)
{
    ParasiteDragMgr.AddExcrescence(excresName,excresType,excresVal);

    ErrorMgr.NoError();
}

void DeleteExcrescence(const int & index)
{
    ParasiteDragMgr.DeleteExcrescence(index);

    ErrorMgr.NoError();
}

void WriteAtmosphereCSVFile(const std::string & file_name, const int atmos_type)
{
    const static double arr[] = {0.0, 5000.0, 10000.0, 10999.0, 11001.0, 15000.0, 19999.0, 20000.0,
    20001.0, 25000.0, 30000.0, 31999.0, 32001.0, 35000.0, 40000.0, 45000.0, 46999.0, 47001.0, 50000.0,
    50999.0, 51001.0, 55000.0, 60000.0, 65000.0, 70000.0, 70999.0, 71001.0, 75000.0, 80000.0, 84851.0,
    84853.0, 85000.0, 90000.0}; //meters
    vector < double > AltTestPoints (arr, arr + sizeof(arr) / sizeof(arr[0]) );

    double temp, pres, pres_ratio, rho_ratio;
    vector < double > temp_vec, pres_vec, pres_ratio_vec, rho_ratio_vec;

    for (size_t i = 0; i < AltTestPoints.size(); ++i )
    {
        vsp::CalcAtmosphere( AltTestPoints[i], 0.0, atmos_type,
            temp, pres, pres_ratio, rho_ratio );
        temp_vec.push_back( temp );
        pres_vec.push_back( pres );
        pres_ratio_vec.push_back( pres_ratio );
        rho_ratio_vec.push_back( rho_ratio );
    }
    Results* res = ResultsMgr.CreateResults("Atmosphere");
    res->Add(NameValData("Alt", AltTestPoints));
    res->Add(NameValData("Temp", temp_vec));
    res->Add(NameValData("Pres", pres_vec));
    res->Add(NameValData("Pres_Ratio", pres_ratio_vec));
    res->Add(NameValData("Rho_Ratio", rho_ratio_vec));
    res->WriteCSVFile( file_name );
}

void CalcAtmosphere(const double & alt, const double & delta_temp, const int & atmos_type,
    double & temp, double & pres, double & pres_ratio, double & rho_ratio )
{
    Atmosphere atmos;

    switch(atmos_type)
    {
    case vsp::ATMOS_TYPE_US_STANDARD_1976:
        atmos.USStandardAtmosphere1976( alt, delta_temp, vsp::PD_UNITS_METRIC, vsp::TEMP_UNIT_K, vsp::PRES_UNIT_KPA);
        break;

    case vsp::ATMOS_TYPE_HERRINGTON_1966:
        atmos.USAF1966( alt, delta_temp, vsp::PD_UNITS_METRIC, vsp::TEMP_UNIT_K, vsp::PRES_UNIT_KPA);
        break;

    default:
        break;
    }

    temp = atmos.GetTemp();
    pres = atmos.GetPres();
    pres_ratio = atmos.GetPressureRatio();
    rho_ratio = atmos.GetDensityRatio();

    ErrorMgr.NoError();
}

void WriteBodyFFCSVFile(const std::string & file_name)
{
    Results* res = ResultsMgr.CreateResults("Body_Form_Factor");
    char str[256];
    vector < double > body_ff_vec, FR, ref_leng, max_x_area;
    vector < double > dol_array = linspace( 0.0, 0.3, 200 );
    res->Add(NameValData("D_L", dol_array));
    ref_leng.push_back(10.0);
    max_x_area.push_back( PI * 1.0 * 1.0 );
    FR.push_back(ref_leng.back() / sqrt(max_x_area.back()) );
    for (size_t body_ff_case = 0; body_ff_case <= vsp::FF_B_JENKINSON_AFT_FUSE_NACELLE; ++body_ff_case )
    {
        for (size_t j = 0; j < dol_array.size(); ++j )
        {
            body_ff_vec.push_back( ParasiteDragMgr.CalcFFBody( 1.0/dol_array[j], FR[0], body_ff_case, ref_leng[0], max_x_area[0] ) );
        }
        sprintf( str, "%s", ParasiteDragMgr.AssignFFBodyEqnName( body_ff_case ).c_str());
        res->Add(NameValData(str, body_ff_vec));
        body_ff_vec.clear();
    }
    res->Add(NameValData("Ref_Leng", ref_leng));
    res->Add(NameValData("Max_X_Area", max_x_area));
    res->WriteCSVFile( file_name );
}

void WriteWingFFCSVFile(const std::string & file_name)
{
    Results* res = ResultsMgr.CreateResults("Wing_Form_Factor");
    char str[256];
    vector < double > wing_ff_vec;
    vector < double > toc_array = linspace( 0.0, 0.205, 200 );
    vector < double > perc_lam, sweep25, sweep50;
    perc_lam.push_back(0.0);
    sweep25.push_back(30.0 * PI / 180.0);
    sweep50.push_back(30.0 * PI / 180.0);
    ParasiteDragMgr.m_Atmos.SetMach(0.8);
    res->Add(NameValData("T_C", toc_array));
    for (size_t wing_ff_case = 0; wing_ff_case < vsp::FF_W_SCHEMENSKY_SUPERCRITICAL_AF; ++wing_ff_case )
    {
        for (size_t j = 0; j < toc_array.size(); ++j )
        {
            wing_ff_vec.push_back( ParasiteDragMgr.CalcFFWing( toc_array[j], wing_ff_case, perc_lam[0], sweep25[0], sweep50[0]) );
        }
        sprintf( str, "%s", ParasiteDragMgr.AssignFFWingEqnName( wing_ff_case ).c_str());
        res->Add(NameValData(str, wing_ff_vec));
        wing_ff_vec.clear();
    }
    res->WriteCSVFile( file_name );
}

void WriteCfEqnCSVFile(const std::string & file_name)
{
    Results* res = ResultsMgr.CreateResults("Friction_Coefficient");
    char str[256];
    vector < double > lam_cf_vec, turb_cf_vec, ref_leng;
    vector < double > ReyIn_array = logspace( 3, 10, 500 );
    vector < double > roughness, gamma, taw_tw_ratio, te_tw_ratio;
    roughness.push_back(0.01);
    gamma.push_back(1.4);
    taw_tw_ratio.push_back(1.0);
    te_tw_ratio.push_back(1.0);
    ref_leng.push_back(1.0);

    for (size_t cf_case = 0; cf_case <= vsp::CF_TURB_WHITE_CHRISTOPH_COMPRESSIBLE; ++cf_case )
    {
        for (size_t j = 0; j < ReyIn_array.size(); ++j )
        {
            turb_cf_vec.push_back( ParasiteDragMgr.CalcTurbCf( ReyIn_array[j], ref_leng[0], cf_case, roughness[0], gamma[0], taw_tw_ratio[0], te_tw_ratio[0]) );
        }
        sprintf( str, "%s", ParasiteDragMgr.AssignTurbCfEqnName( cf_case ).c_str());
        res->Add(NameValData(str, turb_cf_vec));
        turb_cf_vec.clear();
    }

    for (size_t cf_case = 0; cf_case < vsp::CF_LAM_BLASIUS_W_HEAT; ++cf_case)
    {
        for (size_t i = 0; i < ReyIn_array.size(); ++i)
        {
            lam_cf_vec.push_back(ParasiteDragMgr.CalcLamCf(ReyIn_array[i], cf_case));
        }
        sprintf( str, "%s", ParasiteDragMgr.AssignLamCfEqnName( cf_case ).c_str());
        res->Add(NameValData(str, lam_cf_vec));
        lam_cf_vec.clear();
    }

    res->Add(NameValData("ReyIn", ReyIn_array));
    res->Add(NameValData("Ref_Leng", ref_leng));
    res->WriteCSVFile( file_name );
}

void WritePartialCfMethodCSVFile(const std::string & file_name)
{
    Results* res = ResultsMgr.CreateResults("Friction_Coefficient");
    char str[256];
    vector < double > cf_vec, ref_leng;
    vector < double > lam_perc_array = linspace( 0, 100, 1000 );
    vector < double > ReyIn_array, reql_array;
    ReyIn_array.push_back( 1.0e7 );
    reql_array.push_back( 1.0e7 );
    vector < double > roughness, taw_tw_ratio, te_tw_ratio;
    roughness.push_back(0.0);
    taw_tw_ratio.push_back(1.0);
    te_tw_ratio.push_back(1.0);
    ref_leng.push_back(1.0);

    for (size_t i = 0; i < lam_perc_array.size(); ++i )
    {
        cf_vec.push_back( ParasiteDragMgr.CalcPartialTurbulence( lam_perc_array[i], ReyIn_array[0], ref_leng[0], reql_array[0],
            roughness[0], taw_tw_ratio[0], te_tw_ratio[0]) );
    }

    res->Add(NameValData("LamPerc", lam_perc_array));
    res->Add(NameValData("Cf", cf_vec));
    res->Add(NameValData("ReyIn", ReyIn_array));
    res->Add(NameValData("Ref_Leng", ref_leng));
    res->Add(NameValData("Re\/L", reql_array));
    res->Add(NameValData("Roughness", roughness));
    res->Add(NameValData("Taw\/Tw", taw_tw_ratio));
    res->Add(NameValData("Te\/Tw", te_tw_ratio));
    res->WriteCSVFile( file_name );
}

//============================================================================//

vec3d CompPnt01(const std::string &geom_id, const int &surf_indx, const double &u, const double &w)
{
    Vehicle* veh = GetVehicle();
    Geom* geom_ptr = veh->FindGeom( geom_id );
    vec3d ret;
    if ( !geom_ptr )
    {
        ErrorMgr.AddError( VSP_INVALID_GEOM_ID, "CompPnt01::Can't Find Geom " + geom_id );
        return ret;
    }

    if ( surf_indx < 0 || surf_indx >= geom_ptr->GetNumTotalSurfs() )
    {
        ErrorMgr.AddError( VSP_INDEX_OUT_RANGE, "CompPnt01::Invalid Surface Index " + to_string( surf_indx ) );
        return ret;
    }

    ret = geom_ptr->CompPnt01(surf_indx, clamp( u, 0.0, 1.0 ), clamp( w, 0.0, 1.0 ) );

    ErrorMgr.NoError();
    return ret;
}

vec3d CompNorm01(const std::string &geom_id, const int &surf_indx, const double &u, const double &w)
{
    Vehicle* veh = GetVehicle();
    Geom* geom_ptr = veh->FindGeom( geom_id );
    vec3d ret;
    if ( !geom_ptr )
    {
        ErrorMgr.AddError( VSP_INVALID_GEOM_ID, "CompNorm01::Can't Find Geom " + geom_id );
        return ret;
    }

    if ( surf_indx < 0 || surf_indx >= geom_ptr->GetNumTotalSurfs() )
    {
        ErrorMgr.AddError( VSP_INDEX_OUT_RANGE, "CompNorm01::Invalid Surface Index " + to_string( surf_indx ) );
        return ret;
    }

    VspSurf *surf = geom_ptr->GetSurfPtr( surf_indx );
    ret = surf->CompNorm01( clamp( u, 0.0, 1.0 ), clamp( w, 0.0, 1.0 ) );

    ErrorMgr.NoError();
    return ret;
}

vec3d CompTanU01(const std::string &geom_id, const int &surf_indx, const double &u, const double &w)
{
    Vehicle* veh = GetVehicle();
    Geom* geom_ptr = veh->FindGeom( geom_id );
    vec3d ret;
    if ( !geom_ptr )
    {
        ErrorMgr.AddError( VSP_INVALID_GEOM_ID, "CompTanU01::Can't Find Geom " + geom_id );
        return ret;
    }

    if ( surf_indx < 0 || surf_indx >= geom_ptr->GetNumTotalSurfs() )
    {
        ErrorMgr.AddError( VSP_INDEX_OUT_RANGE, "CompTanU01::Invalid Surface Index " + to_string( surf_indx ) );
        return ret;
    }

    VspSurf *surf = geom_ptr->GetSurfPtr( surf_indx );
    ret = surf->CompTanU01( clamp( u, 0.0, 1.0 ), clamp( w, 0.0, 1.0 ) );

    ErrorMgr.NoError();
    return ret;
}

vec3d CompTanW01(const std::string &geom_id, const int &surf_indx, const double &u, const double &w)
{
    Vehicle* veh = GetVehicle();
    Geom* geom_ptr = veh->FindGeom( geom_id );
    vec3d ret;
    if ( !geom_ptr )
    {
        ErrorMgr.AddError( VSP_INVALID_GEOM_ID, "CompTanW01::Can't Find Geom " + geom_id );
        return ret;
    }

    if ( surf_indx < 0 || surf_indx >= geom_ptr->GetNumTotalSurfs() )
    {
        ErrorMgr.AddError( VSP_INDEX_OUT_RANGE, "CompTanW01::Invalid Surface Index " + to_string( surf_indx ) );
        return ret;
    }

    VspSurf *surf = geom_ptr->GetSurfPtr( surf_indx );
    ret = surf->CompTanW01( clamp( u, 0.0, 1.0 ), clamp( w, 0.0, 1.0 ) );

    ErrorMgr.NoError();
    return ret;
}

void CompCurvature01(const std::string &geom_id, const int &surf_indx, const double &u, const double &w, double &k1,
                     double &k2, double &ka, double &kg)
{
    Vehicle* veh = GetVehicle();
    Geom* geom_ptr = veh->FindGeom( geom_id );

    k1 = 0.0;
    k2 = 0.0;
    ka = 0.0;
    kg = 0.0;

    if ( !geom_ptr )
    {
        ErrorMgr.AddError( VSP_INVALID_GEOM_ID, "CompCurvature01::Can't Find Geom " + geom_id );
        return;
    }

    if ( surf_indx < 0 || surf_indx >= geom_ptr->GetNumTotalSurfs() )
    {
        ErrorMgr.AddError( VSP_INDEX_OUT_RANGE, "CompCurvature01::Invalid Surface Index " + to_string( surf_indx ) );
        return;
    }

    VspSurf *surf = geom_ptr->GetSurfPtr( surf_indx );
    surf->CompCurvature01( clamp( u, 0.0, 1.0 ), clamp( w, 0.0, 1.0 ), k1, k2, ka, kg );

    ErrorMgr.NoError();
}

double ProjPnt01(const std::string &geom_id, const int &surf_indx, const vec3d &pt, double &u, double &w)
{
    Vehicle* vPtr = VehicleMgr.GetVehicle();
    Geom * geom = vPtr->FindGeom( geom_id );

    double dmin = std::numeric_limits<double>::max();

    if ( !geom )
    {
        ErrorMgr.AddError( VSP_INVALID_GEOM_ID, "ProjPnt01::Can't Find Geom " + geom_id );
        return dmin;
    }

    if ( surf_indx < 0 || surf_indx >= geom->GetNumTotalSurfs() )
    {
        ErrorMgr.AddError( VSP_INDEX_OUT_RANGE, "ProjPnt01::Invalid Surface Index " + to_string( surf_indx ) );
        return dmin;
    }

    dmin = geom->GetSurfPtr( surf_indx )->FindNearest01( u, w, pt );

    ErrorMgr.NoError();

    return dmin;
}

double ProjPnt01I(const std::string &geom_id, const vec3d &pt, int &surf_indx,
                  double &u, double &w)
{
    Vehicle* vPtr = VehicleMgr.GetVehicle();
    Geom * geom = vPtr->FindGeom( geom_id );

    double dmin = std::numeric_limits<double>::max();

    if ( !geom )
    {
        ErrorMgr.AddError( VSP_INVALID_GEOM_ID, "ProjPnt01I::Can't Find Geom " + geom_id );
        return dmin;
    }

    dmin = vPtr->ProjPnt01I( geom_id, pt, surf_indx, u, w );

    ErrorMgr.NoError();

    return dmin;
}

double ProjPnt01Guess(const std::string &geom_id, const int &surf_indx, const vec3d &pt, const double &u0, const double &w0, double &u, double &w)
{
    Vehicle* vPtr = VehicleMgr.GetVehicle();
    Geom * geom = vPtr->FindGeom( geom_id );

    double dmin = std::numeric_limits<double>::max();

    if ( !geom )
    {
        ErrorMgr.AddError( VSP_INVALID_GEOM_ID, "ProjPnt01Guess::Can't Find Geom " + geom_id );
        return dmin;
    }

    if ( surf_indx < 0 || surf_indx >= geom->GetNumTotalSurfs() )
    {
        ErrorMgr.AddError( VSP_INDEX_OUT_RANGE, "ProjPnt01Guess::Invalid Surface Index " + to_string( surf_indx ) );
        return dmin;
    }

    dmin = geom->GetSurfPtr( surf_indx )->FindNearest01( u, w, pt, clamp( u0, 0.0, 1.0 ), clamp( w0, 0.0, 1.0 ) );

    ErrorMgr.NoError();

    return dmin;
}

vector < vec3d > CompVecPnt01( const std::string &geom_id, const int &surf_indx, const vector < double > &us, const vector < double > &ws )
{
    Vehicle* veh = GetVehicle();

    Geom* geom_ptr = veh->FindGeom( geom_id );

    vector < vec3d > pts;
    pts.resize( 0 );

    if ( geom_ptr )
    {
        if ( us.size() == ws.size() )
        {
            VspSurf *surf = geom_ptr->GetSurfPtr( surf_indx );

            if ( surf )
            {
                pts.resize( us.size() );

                for ( int i = 0; i < us.size(); i++ )
                {
                    pts[i] = surf->CompPnt01( clamp( us[i], 0.0, 1.0 ), clamp( ws[i], 0.0, 1.0 ) );
                }
            }
            else
            {
                ErrorMgr.AddError( VSP_INDEX_OUT_RANGE, "CompPnt01::Invalid surf index." );
                return pts;
            }
        }
        else
        {
            ErrorMgr.AddError( VSP_INDEX_OUT_RANGE, "CompPnt01::Input size mismatch." );
            return pts;
        }
    }
    else
    {
        ErrorMgr.AddError( VSP_INVALID_GEOM_ID, "CompPnt01::Can't Find Geom " + geom_id );
        return pts;
    }
    ErrorMgr.NoError();
    return pts;
}

vector < vec3d > CompVecNorm01( const std::string &geom_id, const int &surf_indx, const vector < double > &us, const vector < double > &ws )
{
    Vehicle* veh = GetVehicle();

    Geom* geom_ptr = veh->FindGeom( geom_id );

    vector < vec3d > norms;
    norms.resize( 0 );

    if ( geom_ptr )
    {
        if ( us.size() == ws.size() )
        {
            VspSurf *surf = geom_ptr->GetSurfPtr( surf_indx );

            if ( surf )
            {
                norms.resize( us.size() );

                for ( int i = 0; i < us.size(); i++ )
                {
                    norms[i] = surf->CompNorm01( clamp( us[i], 0.0, 1.0 ), clamp( ws[i], 0.0, 1.0 ) );
                }
            }
            else
            {
                ErrorMgr.AddError( VSP_INDEX_OUT_RANGE, "CompNorm01::Invalid surf index." );
                return norms;
            }
        }
        else
        {
            ErrorMgr.AddError( VSP_INDEX_OUT_RANGE, "CompNorm01::Input size mismatch." );
            return norms;
        }
    }
    else
    {
        ErrorMgr.AddError( VSP_INVALID_GEOM_ID, "CompNorm01::Can't Find Geom " + geom_id );
        return norms;
    }
    ErrorMgr.NoError();
    return norms;
}

void CompVecCurvature01( const std::string &geom_id, const int &surf_indx, const vector < double > &us, const vector < double > &ws, vector < double > &k1s, vector < double > &k2s, vector < double > &kas, vector < double > &kgs )
{
    Vehicle* veh = GetVehicle();

    Geom* geom_ptr = veh->FindGeom( geom_id );

    k1s.resize( 0 );
    k2s.resize( 0 );
    kas.resize( 0 );
    kgs.resize( 0 );

    if ( geom_ptr )
    {
        if ( us.size() == ws.size() )
        {
            VspSurf *surf = geom_ptr->GetSurfPtr( surf_indx );

            if ( surf )
            {
                k1s.resize( us.size() );
                k2s.resize( us.size() );
                kas.resize( us.size() );
                kgs.resize( us.size() );

                for ( int i = 0; i < us.size(); i++ )
                {
                    surf->CompCurvature01( clamp( us[i], 0.0, 1.0 ), clamp( ws[i], 0.0, 1.0 ), k1s[i], k2s[i], kas[i], kgs[i] );
                }
            }
            else
            {
                ErrorMgr.AddError( VSP_INDEX_OUT_RANGE, "CompCurvature01::Invalid surf index." );
                return;
            }
        }
        else
        {
            ErrorMgr.AddError( VSP_INDEX_OUT_RANGE, "CompCurvature01::Input size mismatch." );
            return;
        }
    }
    else
    {
        ErrorMgr.AddError( VSP_INVALID_GEOM_ID, "CompCurvature01::Can't Find Geom " + geom_id );
        return;
    }
    ErrorMgr.NoError();
}

void ProjVecPnt01(const std::string &geom_id, const int &surf_indx, const vector < vec3d > &pts, vector < double > &us, vector < double > &ws, vector < double > &ds )
{
    Vehicle* veh = GetVehicle();

    Geom* geom_ptr = veh->FindGeom( geom_id );

    us.resize( 0 );
    ws.resize( 0 );
    ds.resize( 0 );

    if ( geom_ptr )
    {
        VspSurf *surf = geom_ptr->GetSurfPtr( surf_indx );

        if ( surf )
        {
            us.resize( pts.size() );
            ws.resize( pts.size() );
            ds.resize( pts.size() );

            for ( int i = 0; i < pts.size(); i++ )
            {
                ds[i] = surf->FindNearest01( us[i], ws[i], pts[i] );
            }
        }
        else
        {
            ErrorMgr.AddError( VSP_INDEX_OUT_RANGE, "ProjPnt01::Invalid surf index." );
            return;
        }
    }
    else
    {
        ErrorMgr.AddError( VSP_INVALID_GEOM_ID, "ProjPnt01::Can't Find Geom " + geom_id );
        return;
    }
    ErrorMgr.NoError();
}

void ProjVecPnt01Guess( const std::string &geom_id, const int &surf_indx, const vector < vec3d > &pts, const vector < double > &u0s, const vector < double > &w0s, vector < double > &us, vector < double > &ws, vector < double > &ds )
{
    Vehicle* veh = GetVehicle();

    Geom* geom_ptr = veh->FindGeom( geom_id );

    us.resize( 0 );
    ws.resize( 0 );
    ds.resize( 0 );

    if ( geom_ptr )
    {
        if ( pts.size() == u0s.size() && pts.size() == w0s.size() )
        {
            VspSurf *surf = geom_ptr->GetSurfPtr(surf_indx);

            if ( surf )
            {
                us.resize( pts.size() );
                ws.resize( pts.size() );
                ds.resize( pts.size() );

                for ( int i = 0; i < pts.size(); i++ )
                {
                    ds[i] = surf->FindNearest01( us[i], ws[i], pts[i], clamp( u0s[i], 0.0, 1.0 ), clamp( w0s[i], 0.0, 1.0 ) );
                }
            }
            else
            {
                ErrorMgr.AddError( VSP_INDEX_OUT_RANGE, "ProjPnt01Guess::Invalid surf index." );
                return;
            }
        }
        else
        {
            ErrorMgr.AddError( VSP_INDEX_OUT_RANGE, "ProjPnt01Guess::Input size mismatch." );
            return;
        }
    }
    else
    {
        ErrorMgr.AddError( VSP_INVALID_GEOM_ID, "ProjPnt01Guess::Can't Find Geom " + geom_id );
        return;
    }
    ErrorMgr.NoError();
}

void GetUWTess01(const std::string &geom_id, const int &surf_indx, std::vector < double > &u_out_vec, std::vector < double > &w_out_vec)
{
    Vehicle* veh = GetVehicle();
    Geom* geom_ptr = veh->FindGeom( geom_id );

    if ( !geom_ptr )
    {
        ErrorMgr.AddError( VSP_INVALID_GEOM_ID, "GetUWTess01::Can't Find Geom " + geom_id );
        return;
    }

    if ( surf_indx < 0 || surf_indx >= geom_ptr->GetNumTotalSurfs() )
    {
        ErrorMgr.AddError( VSP_INDEX_OUT_RANGE, "GetUWTess01::Invalid Surface Index " + to_string( surf_indx ) );
        return;
    }

    geom_ptr->GetUWTess01( surf_indx, u_out_vec, w_out_vec );

    ErrorMgr.NoError();
    return;
}

string AddRuler( const string & startgeomid, int startsurfindx, double startu, double startw,
                 const string & endgeomid, int endsurfindx, double endu, double endw, const string & name )
{
    return MeasureMgr.CreateAndAddRuler( startgeomid, startsurfindx, startu, startw,
                                         endgeomid, endsurfindx, endu, endw, name );
}

vector < string > GetAllRulers()
{
    return MeasureMgr.GetAllRulers();
}

void DelRuler( const string &id )
{
    MeasureMgr.DelRuler( id );
}

void DeleteAllRulers()
{
    MeasureMgr.DelAllRulers();
}

string AddProbe( const string & geomid, int surfindx, double u, double w, const string & name )
{
    return MeasureMgr.CreateAndAddProbe( geomid, surfindx, u, w, name );
}

vector < string > GetAllProbes()
{
    return MeasureMgr.GetAllProbes();
}

void DelProbe( const string &id )
{
    MeasureMgr.DelProbe( id );
}

void DeleteAllProbes()
{
    MeasureMgr.DelAllProbes();
}

}   // vsp namespace
