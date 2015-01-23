//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//

// API.h: interface for the Vehicle Class and Vehicle Mgr Singleton.
// J.R Gloudemans
//
//////////////////////////////////////////////////////////////////////

#include "VSP_Geom_API.h"
#include "APIErrorMgr.h"
#include "VehicleMgr.h"
#include "Vehicle.h"
#include "ParmMgr.h"
#include "LinkMgr.h"
#include "ResultsMgr.h"
#include "XSecSurf.h"
#include "CfdMeshMgr.h"
#include "Util.h"
#include "DesignVarMgr.h"
#include "SubSurfaceMgr.h"
#include "WingGeom.h"

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

    int err = veh->ReadXMLFile( file_name );
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

    string id = veh->CompGeomAndFlatten( set, 0, 0, half_mesh );

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

//==== Compute AWave Slice =====//
extern string ComputeAwaveSlice( int set, int num_slices, int num_rots, double ang_control, bool comp_ang,
                                 const vec3d & norm, bool auto_bnd, double start_bnd, double end_bnd )
{
    Update();
    Vehicle* veh = GetVehicle();

    string id = veh->AwaveSliceAndFlatten( set, num_slices, num_rots, ang_control, comp_ang, norm, auto_bnd, start_bnd, end_bnd );

    if ( id.size() == 0 )
    {
        ErrorMgr.AddError( VSP_INVALID_ID, "ComputeAwaveSlice::Invalid ID " );
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
    else
    {
        ErrorMgr.AddError( VSP_CANT_FIND_TYPE, "SetCFDMeshVal::Can't Find Type " + type  );
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
string AddSubSurf( const string & geom_id, int type )
{
    Vehicle* veh = GetVehicle();
    Geom* geom_ptr = veh->FindGeom( geom_id );
    if ( !geom_ptr )
    {
        ErrorMgr.AddError( VSP_INVALID_PTR, "AddSubSurf::Can't Find Geom " + geom_id  );
        return string();
    }

    SubSurface* ssurf = NULL;
    ssurf = geom_ptr->AddSubSurf( type );
    if ( !ssurf )
    {
        ErrorMgr.AddError( VSP_INVALID_PTR, "AddSubSurface::Invalid Sub Surface Ptr "  );
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
        ErrorMgr.AddError( VSP_INVALID_PTR, "DeleteSubSurf::Can't Find Geom " + geom_id  );
        return;
    }

    int index = -1;
    vector< SubSurface* > svec = geom_ptr->GetSubSurfVec();
    for ( int i = 0 ; i < (int)svec.size() ; i++ )
    {
        if ( svec[i]->GetID() == sub_id )
            index = i;
    }
    if ( index == -1 )
    {
        ErrorMgr.AddError( VSP_INVALID_PTR, "DeleteSubSurf::Can't Find SubSurf " + geom_id  );
        return;
    }
    geom_ptr->DelSubSurf( index );
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
        ErrorMgr.AddError( VSP_INVALID_PTR, "SetDriverGroup::Invalid Wing Section Index " + section_index  );
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

///// Set the XSec type for the XSecSurf
//void SetXSecType( const string & xsec_surf_id, int type )
//{
//    XSecSurf* xsec_surf = FindXSecSurf( xsec_surf_id );
//    if ( !xsec_surf )
//    {
//        ErrorMgr.AddError( VSP_INVALID_PTR, "SetXSecType::Can't Find XSecSurf " + xsec_surf_id  );
//    }
//
//    ErrorMgr.NoError();
//    return xsec_surf->SetXSecType( type );
//}
//
///// Get the XSec type for the XSecSurf
//int GetXSecType( const string & xsec_surf_id )
//{
//    XSecSurf* xsec_surf = FindXSecSurf( xsec_surf_id );
//    if ( !xsec_surf )
//    {
//        ErrorMgr.AddError( VSP_INVALID_PTR, "GetXSecType::Can't Find XSecSurf " + xsec_surf_id  );
//        return 0;
//    }
//
//    ErrorMgr.NoError();
//    return xsec_surf->GetXSecType();
//}

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

/// Cut xsec from xsec_surf.  The xsec is stored in a clipboard and can be pasted.
void CutXSec( const string & xsec_surf_id, int xsec_index )
{
    XSecSurf* xsec_surf = FindXSecSurf( xsec_surf_id );
    if ( !xsec_surf )
    {
        ErrorMgr.AddError( VSP_INVALID_PTR, "CutXSec::Can't Find XSecSurf " + xsec_surf_id  );
        return;
    }
    if ( xsec_index < 0 || xsec_index >= xsec_surf->NumXSec() )
    {
        ErrorMgr.AddError( VSP_INDEX_OUT_RANGE, "CutXSec::XSec Index Out of Range " + xsec_surf_id + ":" + to_string( ( long long )xsec_index )  );
        return;
    }

    ErrorMgr.NoError();
    xsec_surf->CutXSec( xsec_index );
}

/// Copy xsec from xsec_surf.  The xsec is stored in a clipboard and can be pasted.
void CopyXSec( const string & xsec_surf_id, int xsec_index )
{
    XSecSurf* xsec_surf = FindXSecSurf( xsec_surf_id );
    if ( !xsec_surf )
    {
        ErrorMgr.AddError( VSP_INVALID_PTR, "CopyXSec::Can't Find XSecSurf " + xsec_surf_id  );
        return;
    }
    if ( xsec_index < 0 || xsec_index >= xsec_surf->NumXSec() )
    {
        ErrorMgr.AddError( VSP_INDEX_OUT_RANGE, "CopyXSec::XSec Index Out of Range " + xsec_surf_id + ":" + to_string( ( long long )xsec_index )  );
        return;
    }

    ErrorMgr.NoError();
    xsec_surf->CopyXSec( xsec_index );
}

/// Paste xsec from clipboard to xsec_surf.  The pasted xsec replaces the xsec at xsec index.
void PasteXSec( const string & xsec_surf_id, int xsec_index )
{
    XSecSurf* xsec_surf = FindXSecSurf( xsec_surf_id );
    if ( !xsec_surf )
    {
        ErrorMgr.AddError( VSP_INVALID_PTR, "PasteXSec::Can't Find XSecSurf " + xsec_surf_id );
        return;
    }
    if ( xsec_index < 0 || xsec_index >= xsec_surf->NumXSec() )
    {
        ErrorMgr.AddError( VSP_INDEX_OUT_RANGE, "PasteXSec::XSec Index Out of Range " + xsec_surf_id + ":" + to_string( ( long long )xsec_index )  );
        return;
    }

    ErrorMgr.NoError();
    xsec_surf->PasteXSec( xsec_index );
}

/// Create an xsec of type type and add it to the end of xsecsurf
string AppendXSec( const string & xsec_surf_id, int type  )
{
    XSecSurf* xsec_surf = FindXSecSurf( xsec_surf_id );
    if ( !xsec_surf )
    {
        ErrorMgr.AddError( VSP_INVALID_PTR, "AddXSec::Can't Find XSecSurf " + xsec_surf_id  );
        return string();
    }

    string id  = xsec_surf->AddXSec( type );
    if ( id.size() == 0 )
    {
        ErrorMgr.AddError( VSP_INVALID_XSEC_ID, "AddXSec::Invalid XSec Type " + to_string( ( long long ) type ) );
        return id;
    }

    ErrorMgr.NoError();
    return id;
}

/// Create an xsec of type type and insert it after xsec_index
string InsertXSec( const string & xsec_surf_id, int type, int xsec_index  )
{
    XSecSurf* xsec_surf = FindXSecSurf( xsec_surf_id );
    if ( !xsec_surf )
    {
        ErrorMgr.AddError( VSP_INVALID_PTR, "InsertXSec::Can't Find XSecSurf " + xsec_surf_id  );
        return string();
    }
    if ( xsec_index < 0 || xsec_index >= xsec_surf->NumXSec() )
    {
        ErrorMgr.AddError( VSP_INDEX_OUT_RANGE, "InsertXSec::XSec Index Out of Range " + xsec_surf_id + ":" + to_string( ( long long )xsec_index )  );
        return string();
    }

    string id  = xsec_surf->InsertXSec( type, xsec_index );
    if ( id.size() == 0 )
    {
        ErrorMgr.AddError( VSP_INVALID_XSEC_ID, "InsertXSec::Invalid XSec Type " + to_string( ( long long ) type ) );
        return id;
    }

    ErrorMgr.NoError();
    return id;
}

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

    ErrorMgr.AddError( VSP_CANT_FIND_NAME, "GetXSecParm::Can't Find XSecCurve " + name  );
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
        file_xs->ReadXsecFile( file_name );
        return file_xs->GetUnityFilePnts();
    }

    ErrorMgr.NoError();
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
double SetParmValUpdate( const string & geom_id, const string & name, const string & group, double val )
{
    string parm_id = GetParm( geom_id, name, group );
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
string FindParm( const string & parm_container_id, const string & name, const string & group )
{
    ParmContainer* pc = ParmMgr.FindParmContainer( parm_container_id );

    if ( !pc )
    {
        ErrorMgr.AddError( VSP_INVALID_ID, "FindParm::Can't Find Parm Container " + parm_container_id  );
        return string();
    }

   string parm_id = pc->FindParm( name, group );
   Parm* p = ParmMgr.FindParm( parm_id );
   if ( !p )
   {
        ErrorMgr.AddError( VSP_CANT_FIND_PARM, "FindParm::Can't Find Parm " + parm_id  );
        return string();
   }
   ErrorMgr.NoError();

   return parm_id;
}



//============================================================================//
}   // vsp namespace
