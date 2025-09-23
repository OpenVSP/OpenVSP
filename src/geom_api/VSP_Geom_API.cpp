//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//

// API.h: interface for the Vehicle Class and Vehicle Mgr Singleton.
// J.R Gloudemans
//
//////////////////////////////////////////////////////////////////////

#ifdef WIN32
#undef _HAS_STD_BYTE
#define _HAS_STD_BYTE 0
#endif

#define _USE_MATH_DEFINES
#include <cmath>

#include "AdvLinkMgr.h"
#include "AnalysisMgr.h"
#include "APIErrorMgr.h"
#include "AttributeManager.h"
#include "Background3DMgr.h"
#include "BORGeom.h"
#include "CfdMeshMgr.h"
#include "DesignVarMgr.h"
#include "FeaMeshMgr.h"
#include "LinkMgr.h"
#include "main.h"
#include "MeasureMgr.h"
#include "ParasiteDragMgr.h"
#include "ParmMgr.h"
#include "PropGeom.h"
#include "RoutingGeom.h"
#include "StructureMgr.h"
#include "SubSurfaceMgr.h"
#include "SurfaceIntersectionMgr.h"
#include "VarPresetMgr.h"
#include "Vehicle.h"
#include "VehicleMgr.h"
#include "VKTAirfoil.h"
#include "VSP_Geom_API.h"
#include "VSPAEROMgr.h"
#include "VspUtil.h"
#include "WingGeom.h"
#include "StlHelper.h"
#include "ModeMgr.h"

#include <cstdlib>
#include <csignal>

#include "APIUpdateCountMgr.h"
#include "eli/mutil/quad/simpson.hpp"

#ifdef __JETBRAINS_IDE__
#define VSP_USE_FLTK 1
#endif

#ifdef VSP_USE_FLTK
#include "GuiInterface.h"
#endif

namespace vsp
{
//===================================================================//
//===============       Helper Functions            =================//
//===================================================================//
//  Get the pointer to Vehicle - this is a helper function for the other API
//  functions
Vehicle* GetVehicle()
{
    VSPCheckSetup();

    //==== Check For Valid Vehicle Ptr ====//
    Vehicle* veh = VehicleMgr.GetVehicle();
    if ( !veh )
    {
        ErrorMgr.AddError( VSP_INVALID_PTR, "GetVehicle::Invalid Vehicle Ptr" );
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
    return nullptr;
}

// Find the pointer to a XSec given its id
XSec* FindXSec( const string & id )
{
    ParmContainer* pc = ParmMgr.FindParmContainer( id );

    if ( !pc )
    {
        return nullptr;
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

//    //==== Check For Valid Vehicle Ptr ====//
//    if ( !VehicleMgr.GetVehicle() )
//    {
//        ErrorMgr.AddError( VSP_INVALID_PTR, "VSPCheckSetup::Invalid Vehicle Ptr" );
//        exit( 0 );
//    }


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

void Update( bool update_managers )
{
    Vehicle* veh = GetVehicle();
    veh->Update();

    if ( update_managers )
    {
        // Update Managers that may respond to changes in geometry
        // This is not needed in the GUI since the screens will update
        // each manager
        veh->UpdateManagers();
    }

    ErrorMgr.NoError();
}


void VSPExit( int error_code )
{
    exit( error_code );
}

void VSPCrash( int crash_type )
{
    switch ( crash_type )
    {
        case 0:
            raise( SIGABRT );
        case 1:
            raise( SIGSEGV );
        case 2:
            int a[1];
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Warray-bounds"
            a[1] = 1;
#pragma clang diagnostic pop
        default:
            abort();
    }
}

int GetAndResetUpdateCount()
{
    return (int) UpdateCountMgr.GetAndResetUpdateCount();
}

void RegisterCFDMeshAnalyses()
{
    SurfaceIntersectionMgr.RegisterAnalysis();
    CfdMeshMgr.RegisterAnalysis();
    FeaMeshMgr.RegisterAnalysis();
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
        ErrorMgr.AddError( VSP_WRONG_FILE_TYPE, "ReadVSPFile::Error" );
        return;
    }
    veh->SetVSP3FileName( file_name );
    ErrorMgr.NoError();
}

void WriteVSPFile( const string & file_name, int set )
{
    Vehicle* veh = GetVehicle();
    veh->SetVSP3FileName( file_name );
    if( !veh->WriteXMLFile( file_name, set ) )
    {
        ErrorMgr.AddError( VSP_FILE_WRITE_FAILURE, "WriteVSPFile::Failure Writing File " + file_name );
        return;
    }
    ErrorMgr.NoError();
}

void SetVSP3FileName( const string & file_name )
{
    Vehicle* veh = GetVehicle();
    if ( !veh )
    {
        ErrorMgr.AddError( VSP_INVALID_PTR, "SetVSP3FileName::Failure Getting Vehicle Ptr" );
        return;
    }
    veh->SetVSP3FileName( file_name );
    ErrorMgr.NoError();
}

string GetVSPFileName()
{
    Vehicle* veh = GetVehicle();
    if ( !veh ) return string("nullptr");

    ErrorMgr.NoError();
    return veh->GetVSP3FileName();
}

void ClearVSPModel()
{
    GetVehicle()->Renew();
    ErrorMgr.NoError();
}

void InsertVSPFile( const string & file_name, const string & parent )
{
    Vehicle* veh = GetVehicle();

    Geom* parent_geom = nullptr;
    if ( parent.size() > 0 )
    {
        parent_geom = veh->FindGeom( parent );
        if ( !parent_geom )
        {
            ErrorMgr.AddError( VSP_INVALID_GEOM_ID, "InsertVSPFile::Can't Find Parent " + parent );
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
        ErrorMgr.AddError( VSP_WRONG_FILE_TYPE, "InsertVSPFile::Error" + file_name );
        return;
    }
    ErrorMgr.NoError();
}

string ImportFile( const string & file_name, int file_type, const string & parent  )
{
    Vehicle* veh = GetVehicle();
    Geom* parent_geom = nullptr;
    if ( parent.size() > 0 )
    {
        parent_geom = veh->FindGeom( parent );
        if ( !parent_geom )
        {
            ErrorMgr.AddError( VSP_INVALID_GEOM_ID, "ImportFile::Can't Find Parent " + parent );
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

string ExportFile( const string & file_name, int thick_set, int file_type, int subsFlag, int thin_set, bool useMode, const string &modeID )
{
    string mesh_id = GetVehicle()->ExportFile( file_name, thick_set, thin_set, subsFlag, file_type, useMode, modeID );

    ErrorMgr.NoError();
    return mesh_id;
}

void SetBEMPropID( const string & prop_id )
{
    Vehicle* veh = GetVehicle();
    Geom* geom_ptr = veh->FindGeom( prop_id );
    if ( !geom_ptr )
    {
        ErrorMgr.AddError( VSP_INVALID_PTR, "SetBEMPropID::Can't Find Geom " + prop_id );
        return;
    }
    else if ( geom_ptr->GetType().m_Type != PROP_GEOM_TYPE )
    {
        ErrorMgr.AddError( VSP_INVALID_PTR, "SetBEMPropID::Geom is not a propeller " + prop_id );
        return;
    }

    PropGeom* prop_ptr = dynamic_cast <PropGeom*> ( geom_ptr );
    if ( prop_ptr )
    {
        veh->m_BEMPropID = prop_id;
    }
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
        ErrorMgr.AddError( VSP_INVALID_PTR, "GetDesignVar::Design variable " + to_string( index ) + " out of range." );
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
        ErrorMgr.AddError( VSP_INVALID_PTR, "GetDesignVarType::Design variable index " + to_string( index ) + " out of range." );
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
        GetVehicle()->GetCfdSettingsPtr()->SetExportFileName( file_name, CFD_STL_FILE_NAME );
    if ( file_type == CFD_POLY_TYPE )
        GetVehicle()->GetCfdSettingsPtr()->SetExportFileName( file_name, CFD_POLY_FILE_NAME );
    if ( file_type == CFD_TRI_TYPE )
        GetVehicle()->GetCfdSettingsPtr()->SetExportFileName( file_name, CFD_TRI_FILE_NAME );
    if ( file_type == CFD_FACET_TYPE )
        GetVehicle()->GetCfdSettingsPtr()->SetExportFileName( file_name, CFD_FACET_FILE_NAME );
    if ( file_type == CFD_OBJ_TYPE )
        GetVehicle()->GetCfdSettingsPtr()->SetExportFileName( file_name, CFD_OBJ_FILE_NAME );
    if ( file_type == CFD_DAT_TYPE )
        GetVehicle()->GetCfdSettingsPtr()->SetExportFileName( file_name, CFD_DAT_FILE_NAME );
    if ( file_type == CFD_KEY_TYPE )
        GetVehicle()->GetCfdSettingsPtr()->SetExportFileName( file_name, CFD_KEY_FILE_NAME );
    if ( file_type == CFD_GMSH_TYPE )
        GetVehicle()->GetCfdSettingsPtr()->SetExportFileName( file_name, CFD_GMSH_FILE_NAME );
    if ( file_type == CFD_TKEY_TYPE )
        GetVehicle()->GetCfdSettingsPtr()->SetExportFileName( file_name, CFD_TKEY_FILE_NAME );
    if ( file_type == CFD_VSPGEOM_TYPE )
        GetVehicle()->GetCfdSettingsPtr()->SetExportFileName( file_name, CFD_VSPGEOM_FILE_NAME );

    ErrorMgr.NoError();
}

/// Compute Mass Properties on The Components in the Set
string ComputeMassProps( int set, int num_slices, int idir )
{
    Update();

    string id = GetVehicle()->MassPropsAndFlatten( set, vsp::SET_NONE, num_slices, idir);

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

    veh->setExportCompGeomTxtFile( false );
    if ( file_export_types & COMP_GEOM_TXT_TYPE )
    {
        veh->setExportCompGeomTxtFile( true );
    }

    string id = veh->CompGeomAndFlatten( set, half_mesh );

    if ( id.size() == 0 )
    {
        ErrorMgr.AddError( VSP_INVALID_ID, "ComputeCompGeom::Invalid ID " );
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

    bool useMode = false;
    string modeID;

    veh->CreateDegenGeom( set, useMode, modeID );
    veh->WriteDegenGeomFile();
    ErrorMgr.NoError();
}

//==== Compute Plane Slice =====//
string ComputePlaneSlice( int set, int num_slices, const vec3d & norm, bool auto_bnd, double start_bnd, double end_bnd, bool measureduct )
{
    Update();
    Vehicle* veh = GetVehicle();

    bool useMode = false;
    string modeID;

    string id = veh->PSliceAndFlatten( set, num_slices, norm, auto_bnd, start_bnd, end_bnd, measureduct, useMode, modeID );

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
        GetVehicle()->GetCfdGridDensityPtr()->m_MinLen = val;
    else if ( type == CFD_MAX_EDGE_LEN )
        GetVehicle()->GetCfdGridDensityPtr()->m_BaseLen = val;
    else if ( type == CFD_MAX_GAP )
        GetVehicle()->GetCfdGridDensityPtr()->m_MaxGap  = val;
    else if ( type == CFD_NUM_CIRCLE_SEGS )
        GetVehicle()->GetCfdGridDensityPtr()->m_NCircSeg = val;
    else if ( type == CFD_GROWTH_RATIO )
        GetVehicle()->GetCfdGridDensityPtr()->m_GrowRatio = val;
    else if ( type == CFD_LIMIT_GROWTH_FLAG )
        GetVehicle()->GetCfdGridDensityPtr()->SetRigorLimit( ToBool(val) );
    else if ( type == CFD_INTERSECT_SUBSURFACE_FLAG )
        GetVehicle()->GetCfdSettingsPtr()->m_IntersectSubSurfs = ToBool(val);
    else if ( type == CFD_HALF_MESH_FLAG )
        GetVehicle()->GetCfdSettingsPtr()->m_HalfMeshFlag = ToBool(val);
    else if ( type == CFD_FAR_FIELD_FLAG )
        GetVehicle()->GetCfdSettingsPtr()->m_FarMeshFlag = ToBool(val);
    else if ( type == CFD_FAR_MAX_EDGE_LEN )
        GetVehicle()->GetCfdGridDensityPtr()->m_FarMaxLen = val;
    else if ( type == CFD_FAR_MAX_GAP )
        GetVehicle()->GetCfdGridDensityPtr()->m_FarMaxGap = val;
    else if ( type == CFD_FAR_NUM_CIRCLE_SEGS )
        GetVehicle()->GetCfdGridDensityPtr()->m_FarNCircSeg = val;
    else if ( type == CFD_FAR_SIZE_ABS_FLAG )
        GetVehicle()->GetCfdSettingsPtr()->m_FarAbsSizeFlag = ToBool(val);
    else if ( type == CFD_FAR_LENGTH )
        GetVehicle()->GetCfdSettingsPtr()->m_FarLength = val;
    else if ( type == CFD_FAR_WIDTH )
        GetVehicle()->GetCfdSettingsPtr()->m_FarWidth = val;
    else if ( type == CFD_FAR_HEIGHT )
        GetVehicle()->GetCfdSettingsPtr()->m_FarHeight = val;
    else if ( type == CFD_FAR_X_SCALE )
        GetVehicle()->GetCfdSettingsPtr()->m_FarXScale = val;
    else if ( type == CFD_FAR_Y_SCALE )
        GetVehicle()->GetCfdSettingsPtr()->m_FarYScale = val;
    else if ( type == CFD_FAR_Z_SCALE )
        GetVehicle()->GetCfdSettingsPtr()->m_FarZScale = val;
    else if ( type == CFD_FAR_LOC_MAN_FLAG )
        GetVehicle()->GetCfdSettingsPtr()->m_FarManLocFlag = ToBool(val);
    else if ( type == CFD_FAR_LOC_X )
        GetVehicle()->GetCfdSettingsPtr()->m_FarXLocation = val;
    else if ( type == CFD_FAR_LOC_Y )
        GetVehicle()->GetCfdSettingsPtr()->m_FarYLocation = val;
    else if ( type == CFD_FAR_LOC_Z )
        GetVehicle()->GetCfdSettingsPtr()->m_FarZLocation = val;
    else if ( type == CFD_SRF_XYZ_FLAG )
        GetVehicle()->GetCfdSettingsPtr()->m_XYZIntCurveFlag = ToBool(val);
    else
    {
        ErrorMgr.AddError( VSP_CANT_FIND_TYPE, "SetCFDMeshVal::Can't Find Type " + to_string( ( long long )type ) );
        return;
    }

    ErrorMgr.NoError();
}
/// Turn On/Off Wake For Component
void SetCFDWakeFlag( const string & geom_id, bool flag )
{
    Vehicle* veh = GetVehicle();
    Geom* geom_ptr = veh->FindGeom( geom_id );
    if ( !geom_ptr )
    {
        ErrorMgr.AddError( VSP_INVALID_PTR, "SetCFDWakeFlag::Can't Find Geom " + geom_id );
        return;
    }

    geom_ptr->SetWakeActiveFlag( flag );

    if ( !geom_ptr->HasWingTypeSurfs() )
    {
        ErrorMgr.AddError( VSP_INVALID_TYPE, "SetCFDWakeFlag::Geom is not a wing-type surface" );
    }
    else
    {
        ErrorMgr.NoError();
    }
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
        ErrorMgr.AddError( VSP_INVALID_PTR, "AddCFDSource::Can't Find Geom " + geom_id );
        return;
    }

    CfdMeshMgr.SetCurrSourceGeomID( geom_id );
    CfdMeshMgr.SetCurrMainSurfIndx( surf_index );
    BaseSource* source =  CfdMeshMgr.AddSource( type );

    if ( !source )
    {
        ErrorMgr.AddError( VSP_INVALID_PTR, "AddCFDSource::Can't Find Type" );
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
void ComputeCFDMesh( int set, int degenset, int file_export_types )
{
    Update();
    Vehicle* veh = GetVehicle();

    veh->GetCfdSettingsPtr()->SetAllFileExportFlags( false );

    if ( file_export_types & CFD_STL_TYPE )
        veh->GetCfdSettingsPtr()->SetFileExportFlag( CFD_STL_FILE_NAME, true );
    if ( file_export_types & CFD_POLY_TYPE )
        veh->GetCfdSettingsPtr()->SetFileExportFlag( CFD_POLY_FILE_NAME, true );
    if ( file_export_types & CFD_TRI_TYPE )
        veh->GetCfdSettingsPtr()->SetFileExportFlag( CFD_TRI_FILE_NAME, true );
    if ( file_export_types & CFD_FACET_TYPE )
        veh->GetCfdSettingsPtr()->SetFileExportFlag( CFD_FACET_FILE_NAME, true );
    if ( file_export_types & CFD_OBJ_TYPE )
        veh->GetCfdSettingsPtr()->SetFileExportFlag( CFD_OBJ_FILE_NAME, true );
    if ( file_export_types & CFD_DAT_TYPE )
        veh->GetCfdSettingsPtr()->SetFileExportFlag( CFD_DAT_FILE_NAME, true );
    if ( file_export_types & CFD_KEY_TYPE )
        veh->GetCfdSettingsPtr()->SetFileExportFlag( CFD_KEY_FILE_NAME, true );
    if ( file_export_types & CFD_GMSH_TYPE )
        veh->GetCfdSettingsPtr()->SetFileExportFlag( CFD_GMSH_FILE_NAME, true );
    if ( file_export_types & CFD_TKEY_TYPE )
        veh->GetCfdSettingsPtr()->SetFileExportFlag( CFD_TKEY_FILE_NAME, true );
    if ( file_export_types & CFD_VSPGEOM_TYPE )
        veh->GetCfdSettingsPtr()->SetFileExportFlag( CFD_VSPGEOM_FILE_NAME, true );

    veh->GetCfdSettingsPtr()->m_SelectedSetIndex = set;
    veh->GetCfdSettingsPtr()->m_SelectedDegenSetIndex = degenset;
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

int CreateVSPAEROControlSurfaceGroup()
{
    VSPAEROMgr.Update();
    VSPAEROMgr.AddControlSurfaceGroup();
    return VSPAEROMgr.GetCurrentCSGroupIndex();
}

void AddAllToVSPAEROControlSurfaceGroup( int CSGroupIndex )
{
    if ( CSGroupIndex < 0 || CSGroupIndex > GetNumControlSurfaceGroups() )
    {
        ErrorMgr.AddError( VSP_INDEX_OUT_RANGE, "AddAllToVSPAEROControlSurfaceGroup::CSGroupIndex " + to_string( CSGroupIndex ) + " out of range" );
        return;
    }

    VSPAEROMgr.SetCurrentCSGroupIndex( CSGroupIndex );

    VSPAEROMgr.Update();

    VSPAEROMgr.AddAllToCSGroup();
}

void RemoveAllFromVSPAEROControlSurfaceGroup( int CSGroupIndex )
{
    if ( CSGroupIndex < 0 || CSGroupIndex > GetNumControlSurfaceGroups() )
    {
        ErrorMgr.AddError( VSP_INDEX_OUT_RANGE, "RemoveAllFromVSPAEROControlSurfaceGroup::CSGroupIndex " + to_string( CSGroupIndex ) + " out of range" );
        return;
    }

    VSPAEROMgr.SetCurrentCSGroupIndex( CSGroupIndex );

    VSPAEROMgr.Update();

    VSPAEROMgr.RemoveAllFromCSGroup();
}

std::vector < std::string > GetActiveCSNameVec( int CSGroupIndex )
{
    if ( CSGroupIndex < 0 || CSGroupIndex > GetNumControlSurfaceGroups() )
    {
        ErrorMgr.AddError( VSP_INDEX_OUT_RANGE, "GetActiveCSNameVec::CSGroupIndex " + to_string( CSGroupIndex ) + " out of range" );
        return {};
    }

    VSPAEROMgr.SetCurrentCSGroupIndex( CSGroupIndex );

    VSPAEROMgr.Update();

    vector < VspAeroControlSurf > active_cs_vec = VSPAEROMgr.GetActiveCSVec();
    vector < string > return_vec( active_cs_vec.size() );

    for ( size_t i = 0; i < return_vec.size(); i++ )
    {
        return_vec[i] = active_cs_vec[i].fullName;
    }

    return return_vec;
}

std::vector < std::string > GetCompleteCSNameVec( )
{
    VSPAEROMgr.Update();

    vector < VspAeroControlSurf > complete_cs_vec = VSPAEROMgr.GetCompleteCSVec();
    vector < string > return_vec( complete_cs_vec.size() );

    for ( size_t i = 0; i < return_vec.size(); i++ )
    {
        return_vec[i] = complete_cs_vec[i].fullName;
    }

    return return_vec;
}

std::vector < std::string > GetAvailableCSNameVec( int CSGroupIndex )
{
    if ( CSGroupIndex < 0 || CSGroupIndex > GetNumControlSurfaceGroups() )
    {
        ErrorMgr.AddError( VSP_INDEX_OUT_RANGE, "GetAvailableCSNameVec::CSGroupIndex " + to_string( CSGroupIndex ) + " out of range" );
        return {};
    }

    VSPAEROMgr.SetCurrentCSGroupIndex( CSGroupIndex );

    VSPAEROMgr.Update();

    vector < VspAeroControlSurf > avail_cs_vec = VSPAEROMgr.GetAvailableCSVec();
    vector < string > return_vec( avail_cs_vec.size() );

    for ( size_t i = 0; i < return_vec.size(); i++ )
    {
        return_vec[i] = avail_cs_vec[i].fullName;
    }

    return return_vec;
}

void SetVSPAEROControlGroupName(const string & name, int CSGroupIndex)
{
    if ( CSGroupIndex < 0 || CSGroupIndex > GetNumControlSurfaceGroups() )
    {
        ErrorMgr.AddError( VSP_INDEX_OUT_RANGE, "SetVSPAEROControlGroupName::CSGroupIndex " + to_string( CSGroupIndex ) + " out of range" );
        return;
    }

    VSPAEROMgr.SetCurrentCSGroupIndex( CSGroupIndex );

    VSPAEROMgr.Update();

    VSPAEROMgr.SetCurrentCSGroupName(name);
}

string GetVSPAEROControlGroupName( int CSGroupIndex )
{
    if ( CSGroupIndex < 0 || CSGroupIndex > GetNumControlSurfaceGroups() )
    {
        ErrorMgr.AddError( VSP_INDEX_OUT_RANGE, "GetVSPAEROControlGroupName::CSGroupIndex " + to_string( CSGroupIndex ) + " out of range" );
        return string();
    }

    VSPAEROMgr.SetCurrentCSGroupIndex( CSGroupIndex );

    VSPAEROMgr.Update();

    return VSPAEROMgr.GetCurrentCSGGroupName();
}

void AddSelectedToCSGroup( const vector <int> &selected, int CSGroupIndex)
{
    if ( CSGroupIndex < 0 || CSGroupIndex > GetNumControlSurfaceGroups() )
    {
        ErrorMgr.AddError( VSP_INDEX_OUT_RANGE, "AddSelectedToCSGroup::CSGroupIndex " + to_string( CSGroupIndex ) + " out of range" );
        return;
    }

    VSPAEROMgr.SetCurrentCSGroupIndex( CSGroupIndex );
    int max_cs_index = VSPAEROMgr.GetAvailableCSVec().size();

    if ( selected.size() == 0 || selected.size() > max_cs_index )
    {
        ErrorMgr.AddError( VSP_INVALID_INPUT_VAL, "AddSelectedToCSGroup::selected out of range" );
        return;
    }

    for ( size_t i = 0; i < selected.size(); i++ )
    {
        if ( selected[i] <= 0 || selected[i] > max_cs_index )
        {
            ErrorMgr.AddError( VSP_INVALID_INPUT_VAL, "AddSelectedToCSGroup::component of selected out of range (indexing must be one based)" );
            return;
        }
    }

    VSPAEROMgr.m_SelectedUngroupedCS = selected;

    VSPAEROMgr.Update();

    VSPAEROMgr.AddSelectedToCSGroup();
}

void RemoveSelectedFromCSGroup( const vector <int> &selected, int CSGroupIndex)
{
    if ( CSGroupIndex < 0 || CSGroupIndex > GetNumControlSurfaceGroups() )
    {
        ErrorMgr.AddError( VSP_INDEX_OUT_RANGE, "RemoveSelectedFromCSGroup::CSGroupIndex " + to_string( CSGroupIndex ) + " out of range" );
        return;
    }

    VSPAEROMgr.SetCurrentCSGroupIndex( CSGroupIndex );
    int max_cs_index = VSPAEROMgr.GetActiveCSVec().size();

    if ( selected.size() == 0 || selected.size() > max_cs_index )
    {
        ErrorMgr.AddError( VSP_INVALID_INPUT_VAL, "RemoveSelectedFromCSGroup::selected out of range" );
        return;
    }

    for ( size_t i = 0; i < selected.size(); i++ )
    {
        if ( selected[i] <= 0 || selected[i] > max_cs_index )
        {
            ErrorMgr.AddError( VSP_INVALID_INPUT_VAL, "RemoveSelectedFromCSGroup::component of selected out of range (indexing must be one based)" );
            return;
        }
    }

    VSPAEROMgr.m_SelectedGroupedCS = selected;

    VSPAEROMgr.Update();

    VSPAEROMgr.RemoveSelectedFromCSGroup();
}

int GetNumControlSurfaceGroups()
{
    return VSPAEROMgr.GetControlSurfaceGroupVec().size();
}


//===================================================================//
//=========       VSPAERO Unsteady Group Functions        ===========//
//===================================================================//
string FindUnsteadyGroup( int group_index )
{
    VSPAEROMgr.UpdateUnsteadyGroups();

    if ( !VSPAEROMgr.ValidUnsteadyGroupInd( group_index ) )
    {
        ErrorMgr.AddError( VSP_INDEX_OUT_RANGE, "FindUnsteadyGroup::group_index " + to_string( group_index ) + " out of range" );
        return string();
    }

    UnsteadyGroup* group = VSPAEROMgr.GetUnsteadyGroup( group_index );
    VSPAEROMgr.SetCurrentUnsteadyGroupIndex( group_index ); // Need if RPM is uniform
    return group->GetID();
}

string GetUnsteadyGroupName( int group_index )
{
    VSPAEROMgr.UpdateUnsteadyGroups();

    if ( !VSPAEROMgr.ValidUnsteadyGroupInd( group_index ) )
    {
        ErrorMgr.AddError( VSP_INDEX_OUT_RANGE, "GetUnsteadyGroupName::group_index " + to_string( group_index ) + " out of range" );
        return string();
    }

    UnsteadyGroup* group = VSPAEROMgr.GetUnsteadyGroup( group_index );
    VSPAEROMgr.SetCurrentUnsteadyGroupIndex( group_index ); // Need if RPM is uniform
    return group->GetName();
}

vector < string > GetUnsteadyGroupCompIDs( int group_index )
{
    vector < string > ret_vec;
    VSPAEROMgr.UpdateUnsteadyGroups();

    if ( !VSPAEROMgr.ValidUnsteadyGroupInd( group_index ) )
    {
        ErrorMgr.AddError( VSP_INDEX_OUT_RANGE, "GetUnsteadyGroupCompIDs::group_index " + to_string( group_index ) + " out of range" );
        return ret_vec;
    }

    UnsteadyGroup* group = VSPAEROMgr.GetUnsteadyGroup( group_index );

    vector < pair < string, int > > comp_vec = group->GetCompSurfPairVec();
    ret_vec.resize( comp_vec.size() );

    for ( size_t i = 0; i < comp_vec.size(); i++ )
    {
        ret_vec[i] = comp_vec[i].first;
    }

    VSPAEROMgr.SetCurrentUnsteadyGroupIndex( group_index ); // Need if RPM is uniform
    return ret_vec;
}

vector < int > GetUnsteadyGroupSurfIndexes( int group_index )
{
    vector < int > ret_vec;
    VSPAEROMgr.UpdateUnsteadyGroups();

    if ( !VSPAEROMgr.ValidUnsteadyGroupInd( group_index ) )
    {
        ErrorMgr.AddError( VSP_INDEX_OUT_RANGE, "GetUnsteadyGroupSurfIndexes::group_index " + to_string( group_index ) + " out of range" );
        return ret_vec;
    }

    UnsteadyGroup* group = VSPAEROMgr.GetUnsteadyGroup( group_index );

    vector < pair < string, int > > comp_vec = group->GetCompSurfPairVec();
    ret_vec.resize( comp_vec.size() );

    for ( size_t i = 0; i < comp_vec.size(); i++ )
    {
        ret_vec[i] = comp_vec[i].second;
    }

    VSPAEROMgr.SetCurrentUnsteadyGroupIndex( group_index ); // Need if RPM is uniform
    return ret_vec;
}

int GetNumUnsteadyGroups()
{
    VSPAEROMgr.UpdateUnsteadyGroups();

    return VSPAEROMgr.NumUnsteadyGroups();
}

int GetNumUnsteadyRotorGroups()
{
    VSPAEROMgr.UpdateUnsteadyGroups();

    return VSPAEROMgr.NumUnsteadyRotorGroups();
}

//===================================================================//
//=========       VSPAERO Actuator Disk Functions        ============//
//===================================================================//
string FindActuatorDisk( int disk_index )
{
    VSPAEROMgr.UpdateRotorDisks();

    if ( !VSPAEROMgr.ValidRotorDiskIndex( disk_index ) )
    {
        ErrorMgr.AddError( VSP_INDEX_OUT_RANGE, "FindActuatorDisk::disk_index " + to_string( disk_index ) + " out of range" );
        return string();
    }

    RotorDisk* disk = VSPAEROMgr.GetRotorDisk( disk_index );
    return disk->GetID();
}

int GetNumActuatorDisks()
{
    VSPAEROMgr.UpdateRotorDisks();

    return VSPAEROMgr.GetRotorDiskVec().size();
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

string GetAnalysisDoc( const std::string & analysis )
{
    if ( !AnalysisMgr.ValidAnalysisName( analysis ) )
    {
        ErrorMgr.AddError( VSP_INVALID_ID, "GetAnalysisDoc::Invalid Analysis ID " + analysis );
        return "";
    }

    Analysis *a = AnalysisMgr.FindAnalysis( analysis );

    return a->m_Inputs.GetDoc();
}

string GetAnalysisInputDoc( const std::string & analysis, const std::string & name )
{
    if ( !AnalysisMgr.ValidAnalysisName( analysis ) )
    {
        ErrorMgr.AddError( VSP_INVALID_ID, "GetAnalysisInputDoc::Invalid Analysis ID " + analysis );
        return "";
    }
    ErrorMgr.NoError();

    return AnalysisMgr.GetAnalysisInputDoc( analysis, name );
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
        ErrorMgr.AddError( VSP_INVALID_ID, "SetAnalysisInputDefaults::Invalid Analysis ID " + analysis );
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

void PrintAnalysisDocs( const string & analysis_name )
{
    if ( !AnalysisMgr.ValidAnalysisName( analysis_name ) )
    {
        ErrorMgr.AddError( VSP_INVALID_ID, "PrintAnalysisDocs::Invalid Analysis ID " + analysis_name );
    }

    AnalysisMgr.PrintAnalysisDocs( analysis_name );
}

//===================================================================//
//===============       Attributes Functions         ===================//
//===================================================================//

string SummarizeAttributes()
{
    return AttributeMgr.SummarizeAttributes();
}

string SummarizeAttributesAsTree()
{
    return AttributeMgr.SummarizeAttributesAsTree();
}

vector < string > FindAllAttributes()
{
    return AttributeMgr.FindAllAttributes();
}

vector < string > FindAttributesByName( const string & search_str )
{
    return AttributeMgr.FindAttributesByName( search_str );
}

string FindAttributeByName( const string & search_str, int index )
{
    return AttributeMgr.FindAttributeByName( search_str, index );
}

string FindAttributeInCollection( const string & obj_id, const string & search_str, int index )
{
    return AttributeMgr.FindAttributeInCollection( obj_id, search_str, index );
}

vector< string > FindAttributeNamesInCollection( const string & collID )
{
    return AttributeMgr.FindAttributeNamesInCollection( collID );
}

vector< string > FindAttributesInCollection( const string & collID )
{
    return AttributeMgr.FindAttributesInCollection( collID );
}

vector< string > FindAttributedObjects()
{
    return AttributeMgr.FindAttributedObjects();
}

int GetObjectType( const string & id )
{
    return AttributeMgr.GetObjectType( id );
}

string GetObjectTypeName( const string & id )
{
    return AttributeMgr.GetObjectTypeName( id );
}

string GetObjectName( const string & id )
{
    return AttributeMgr.GetObjectName( id );
}

string GetObjectParent( const string & id )
{
    return AttributeMgr.GetObjectParent( id );
}

string GetChildCollection( const string & id )
{
    return AttributeMgr.GetChildCollection( id );
}

string GetGeomSetCollection( const int & index )
{
    return AttributeMgr.GetGeomSetCollection( index );
}

string GetAttributeName( const string & attrID )
{
    return AttributeMgr.GetAttributeName( attrID );
}

string GetAttributeID( const string & collID, const string & attributeName, int index )
{
    return AttributeMgr.GetAttributeID( collID, attributeName, index );
}

string GetAttributeDoc( const string & attrID )
{
    return AttributeMgr.GetAttributeDoc( attrID );
}

int GetAttributeType( const string & attrID )
{
    return AttributeMgr.GetAttributeType( attrID );
}

string GetAttributeTypeName( const string & attrID )
{
    return AttributeMgr.GetAttributeTypeName( attrID );
}

vector< int > GetAttributeBoolVal( const string & attrID )
{
    return AttributeMgr.GetAttributeBoolVal( attrID );
}

vector< int > GetAttributeIntVal( const string & attrID )
{
    return AttributeMgr.GetAttributeIntVal( attrID );
}

vector< double > GetAttributeDoubleVal( const string & attrID )
{
    return AttributeMgr.GetAttributeDoubleVal( attrID );
}

vector< string > GetAttributeStringVal( const string & attrID )
{
    return AttributeMgr.GetAttributeStringVal( attrID );
}

vector< string >  GetAttributeParmID( const string & attrID )
{
    return AttributeMgr.GetAttributeParmID( attrID );
}

vector< double > GetAttributeParmVal( const string & attrID )
{
    return AttributeMgr.GetAttributeParmVal( attrID );
}

vector< string > GetAttributeParmName( const string & attrID )
{
    return AttributeMgr.GetAttributeParmName( attrID );
}

vector< vec3d > GetAttributeVec3dVal( const string & attrID )
{
    return AttributeMgr.GetAttributeVec3dVal( attrID );
}

vector< vector < int > > GetAttributeIntMatrixVal( const string & attrID )
{
    return AttributeMgr.GetAttributeIntMatrixVal( attrID );
}

vector< vector < double > > GetAttributeDoubleMatrixVal( const string & attrID )
{
    return AttributeMgr.GetAttributeDoubleMatrixVal( attrID );
}

void SetAttributeName( const string &attrID, const string &name )
{
    return AttributeMgr.SetAttributeName( attrID, name );
}

void SetAttributeDoc( const string & attrID, const string & doc )
{
    AttributeMgr.SetAttributeDoc( attrID, doc );
}

void SetAttributeBool( const string & attrID, bool value )
{
    AttributeMgr.SetAttributeBool( attrID, value );
}

void SetAttributeInt( const string & attrID, int value )
{
    AttributeMgr.SetAttributeInt( attrID, value );
}

void SetAttributeDouble( const string & attrID, double value )
{
    AttributeMgr.SetAttributeDouble( attrID, value );
}

void SetAttributeString( const string & attrID, const string & value )
{
    AttributeMgr.SetAttributeString( attrID, value );
}

void SetAttributeParmID( const string & attrID, const string & value )
{
    AttributeMgr.SetAttributeParmID( attrID, value );
}

void SetAttributeVec3d( const string & attrID, const vector < vec3d > & value )
{
    AttributeMgr.SetAttributeVec3d( attrID, value );
}

void SetAttributeIntMatrix( const string & attrID, const vector < vector < int > > &value )
{
    AttributeMgr.SetAttributeIntMatrix( attrID, value );
}

void SetAttributeDoubleMatrix( const string & attrID, const vector< vector< double > > & value )
{
    AttributeMgr.SetAttributeDoubleMatrix( attrID, value );
}

void DeleteAttribute( const string & attrID )
{
    AttributeMgr.DeleteAttribute( attrID );
}

string AddAttributeBool( const string & collID, const string & attributeName, bool value )
{
    return AttributeMgr.AddAttributeBool( collID, attributeName, value );
}

string AddAttributeInt( const string & collID, const string & attributeName, int value )
{
    return AttributeMgr.AddAttributeInt( collID, attributeName, value );
}

string AddAttributeDouble( const string & collID, const string & attributeName, double value)
{
    return AttributeMgr.AddAttributeDouble( collID, attributeName, value );
}

string AddAttributeString( const string & collID, const string & attributeName, const string & value )
{
    return AttributeMgr.AddAttributeString( collID, attributeName, value );
}

string AddAttributeParm( const string & collID, const string & attributeName, const string & parmID )
{
    return AttributeMgr.AddAttributeParm( collID, attributeName, parmID );
}

string AddAttributeVec3d( const string & collID, const string & attributeName, const vector < vec3d > & value )
{
    return AttributeMgr.AddAttributeVec3d( collID, attributeName, value );
}

string AddAttributeIntMatrix( const string & collID, const string & attributeName, const vector < vector < int > > & value )
{
    return AttributeMgr.AddAttributeIntMatrix( collID, attributeName, value );
}

string AddAttributeDoubleMatrix( const string & collID, const string & attributeName, const vector < vector < double > > & value )
{
    return AttributeMgr.AddAttributeDoubleMatrix( collID, attributeName, value );
}

string AddAttributeGroup( const string & collID, const string & attributeName )
{
    return AttributeMgr.AddAttributeGroup( collID, attributeName );
}

int CopyAttribute( const string & attrID )
{
    return AttributeMgr.CopyAttribute( attrID );
}

void CutAttribute( const string & attrID )
{
    AttributeMgr.CutAttribute( attrID );
}

vector < string > PasteAttribute( const string & objID )
{
    return AttributeMgr.PasteAttribute( objID );
}

//===================================================================//
//===============       Results Functions         ===================//
//===================================================================//
/// Get all results names available
vector<string> GetAllResultsNames()
{
    return ResultsMgr.GetAllResultsNames();
}

/// Get all data names available for this result
vector< string > GetAllDataNames( const string & results_id )
{
    if ( !ResultsMgr.ValidResultsID( results_id ) )
    {
        ErrorMgr.AddError( VSP_INVALID_ID, "GetAllDataNames::Invalid ID " + results_id );
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

/// Get the name of the results object with the given id
string GetResultsName( const string & results_id )
{
    if ( !ResultsMgr.ValidResultsID( results_id ) )
    {
        ErrorMgr.AddError( VSP_INVALID_ID, "GetResultName::Invalid ID " + results_id );
        string ret_str;
        return  ret_str;
    }

    return ResultsMgr.FindResultsPtr( results_id )->GetName();
}

string GetResultsSetDoc( const std::string & results_id )
{
    if ( !ResultsMgr.ValidResultsID( results_id ) )
    {
        ErrorMgr.AddError( VSP_INVALID_ID, "GetResultsSetDoc::Invalid ID " + results_id );
        return  "";
    }

    return ResultsMgr.FindResultsPtr( results_id )->GetDoc();
}

string GetResultsEntryDoc( const std::string & results_id, const std::string & data_name )
{
    if ( !ResultsMgr.ValidResultsID( results_id ) )
    {
        ErrorMgr.AddError( VSP_INVALID_ID, "GetResultsEntryDoc::Invalid ID " + results_id );
        return "";
    }
    ErrorMgr.NoError();

    return ResultsMgr.GetResultsEntryDoc( results_id, data_name );
}

/// Return the id of the results with the given results name and index
string FindResultsID( const string & name, int index )
{
    string id = ResultsMgr.FindResultsID( name, index );
    if ( id.size() == 0 )
    {
        ErrorMgr.AddError( VSP_CANT_FIND_NAME, "FindResultsID::Can't Find Name " + name + " at index " + to_string( index ) );
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
        ErrorMgr.AddError( VSP_CANT_FIND_NAME, "FindLatestResultsID::Can't Find Name " + name );
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
        ErrorMgr.AddError( VSP_INVALID_ID, "GetNumData::Invalid ID " + results_id );
        return 0;
    }
    ErrorMgr.NoError();
    return ResultsMgr.GetNumData( results_id, data_name );
}

extern int GetResultsType( const string & results_id, const string & data_name )
{
    if ( !ResultsMgr.ValidResultsID( results_id ) )
    {
        ErrorMgr.AddError( VSP_INVALID_ID, "GetResultsType::Invalid ID " + results_id );
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
        ErrorMgr.AddError( VSP_INVALID_ID, "GetIntResults::Invalid ID " + id );
    }
    else if ( !ResultsMgr.ValidDataNameIndex( id, name, index ) )
    {
        ErrorMgr.AddError( VSP_CANT_FIND_NAME, "GetIntResults::Can't Find Name " + name + " at index " + to_string( index ) );
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
        ErrorMgr.AddError( VSP_INVALID_ID, "GetDoubleResults::Invalid ID " + id );
    }
    else if ( !ResultsMgr.ValidDataNameIndex( id, name, index ) )
    {
        ErrorMgr.AddError( VSP_CANT_FIND_NAME, "GetDoubleResults::Can't Find Name " + name + " at index " + to_string( index ) );
    }
    else
    {
        ErrorMgr.NoError();
    }

    return ResultsMgr.GetDoubleResults( id, name, index );
}

/// Return the double matrix given the results id, data name and data index
const vector< vector<double> > & GetDoubleMatResults( const string & id, const string & name, int index )
{
    if ( !ResultsMgr.ValidResultsID( id ) )
    {
        ErrorMgr.AddError( VSP_INVALID_ID, "GetDoubleMatResults::Invalid ID " + id );
    }
    else if ( !ResultsMgr.ValidDataNameIndex( id, name, index ) )
    {
        ErrorMgr.AddError( VSP_CANT_FIND_NAME, "GetDoubleMatResults::Can't Find Name " + name + " at index " + to_string( index ) );
    }
    else
    {
        ErrorMgr.NoError();
    }

    return ResultsMgr.GetDoubleMatResults( id, name, index );
}

/// Return the string data given the results id, data name and data index
const vector<string> & GetStringResults( const string & id, const string & name, int index )
{
    if ( !ResultsMgr.ValidResultsID( id ) )
    {
        ErrorMgr.AddError( VSP_INVALID_ID, "GetStringResults::Invalid ID " + id );
    }
    else if ( !ResultsMgr.ValidDataNameIndex( id, name, index ) )
    {
        ErrorMgr.AddError( VSP_CANT_FIND_NAME, "GetStringResults::Can't Find Name " + name + " at index " + to_string( index ) );
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
        ErrorMgr.AddError( VSP_INVALID_ID, "GetVec3dResults::Invalid ID " + id );
    }
    else if ( !ResultsMgr.ValidDataNameIndex( id, name, index ) )
    {
        ErrorMgr.AddError( VSP_CANT_FIND_NAME, "GetVec3dResults::Can't Find Name " + name + " at index " + to_string( index ) );
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
        ErrorMgr.AddError( VSP_INVALID_GEOM_ID, "CreateGeomResults::Can't Find GeomID " + geom_id );
        return string();
    }

    string res_id = ResultsMgr.CreateGeomResults( geom_id, name );

    if ( !ResultsMgr.ValidResultsID( res_id ) )
    {
        ErrorMgr.AddError( VSP_INVALID_ID, "CreateGeomResults::Invalid Results " + res_id );
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
        ErrorMgr.AddError( VSP_INVALID_ID, "DeleteResult::Invalid ID " + id );
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
        ErrorMgr.AddError( VSP_INVALID_ID, "WriteResultsCSVFile::Invalid ID " + id );
        return;
    }
    resptr->WriteCSVFile( file_name );
    ErrorMgr.NoError();
 }

void PrintResults( const string &results_id )
{
    ResultsMgr.PrintResults( results_id );
}

void PrintResultsDocs( const std::string &results_id )
{
    ResultsMgr.PrintResultsDocs( results_id );
}

void WriteTestResults()
{
    ResultsMgr.WriteTestResults();
}

//===================================================================//
//===============        GUI Functions            ===================//
//===================================================================//

/// Check VehicleMgr for a valid vehicle pointer.  Create vehicle
/// pointer on first call.  There is a check to prevent multiple calls.

extern void InitGUI( )
{
#ifdef VSP_USE_FLTK
    if ( VehicleMgr.GetVehicle() )
    {
        GuiInterface::getInstance().InitGUI( VehicleMgr.GetVehicle() );
    }
    ErrorMgr.NoError();
#endif
}

extern void StartGUI( )
{
#ifdef VSP_USE_FLTK
    //==== Make Sure Init is Only Called Once ===//
    // This is probably not needed as InitGUI() has its own checks.
    static bool once = false;

    if ( !once )
    {
        InitGUI();
    }
    once = true;
    GuiInterface::getInstance().StartGUIAPI( );
#endif
}

extern void StopGUI( )
{
#ifdef VSP_USE_FLTK
    GuiInterface::getInstance().StopGUI( );
#endif
}

extern void EnableStopGUIMenuItem( )
{
#ifdef VSP_USE_FLTK
    GuiInterface::getInstance().EnableStopGUIMenuItem();
#endif
}

extern void DisableStopGUIMenuItem( )
{
#ifdef VSP_USE_FLTK
    GuiInterface::getInstance().DisableStopGUIMenuItem();
#endif
}

extern void PopupMsg( const std::string &msg )
{
#ifdef VSP_USE_FLTK
    GuiInterface::getInstance().PopupMsg( msg );
#endif
}

extern void UpdateGUI()
{
#ifdef VSP_USE_FLTK
    GuiInterface::getInstance().UpdateGUI();
#endif
}

extern bool IsGUIBuild()
{
#ifdef VSP_USE_FLTK
    return true;
#else
    return false;
#endif
}

extern void Lock( )
{
#ifdef VSP_USE_FLTK
    GuiInterface::getInstance().Lock( );
#endif
}

extern void Unlock( )
{
#ifdef VSP_USE_FLTK
    GuiInterface::getInstance().Unlock( );
#endif
}

extern bool IsEventLoopRunning( )
{
#ifdef VSP_USE_FLTK
    return GuiInterface::getInstance().IsEventLoopRunning( );
#endif
    return false;
}

void ScreenGrab( const string & fname, int w, int h, bool transparentBG, bool autocrop )
{
#ifdef VSP_USE_FLTK
    GuiInterface::getInstance().ScreenGrab( fname, w, h, transparentBG, autocrop );
#endif
}

void SetViewAxis( bool vaxis )
{
#ifdef VSP_USE_FLTK
    GuiInterface::getInstance().SetViewAxis( vaxis );
#endif
}

void SetShowBorders( bool brdr )
{
#ifdef VSP_USE_FLTK
    GuiInterface::getInstance().SetShowBorders( brdr );
#endif
}

void SetGeomDrawType(const string &geom_id, int type)
{
    Vehicle* veh = GetVehicle();
    Geom* geom_ptr = veh->FindGeom( geom_id );
    if ( !geom_ptr )
    {
        ErrorMgr.AddError( VSP_INVALID_PTR, "SetGeomDrawType::Can't Find Geom " + geom_id );
        return;
    }
    geom_ptr->m_GuiDraw.SetDrawType( type );

    ErrorMgr.NoError();
}

void SetGeomWireColor( const string &geom_id, int r, int g, int b )
{
    Vehicle* veh = GetVehicle();
    Geom* geom_ptr = veh->FindGeom( geom_id );
    if ( !geom_ptr )
    {
        ErrorMgr.AddError( VSP_INVALID_PTR, "SetGeomWireColor::Can't Find Geom " + geom_id );
        return;
    }
    geom_ptr->SetColor( r, g, b );

    ErrorMgr.NoError();
}

void SetGeomDisplayType(const string &geom_id, int type)
{
    Vehicle* veh = GetVehicle();
    Geom* geom_ptr = veh->FindGeom( geom_id );
    if ( !geom_ptr )
    {
        ErrorMgr.AddError( VSP_INVALID_PTR, "SetGeomDisplayType::Can't Find Geom " + geom_id );
        return;
    }
    geom_ptr->m_GuiDraw.SetDisplayType( type );
    geom_ptr->SetDirtyFlag( GeomBase::TESS );
    geom_ptr->SetLateUpdateFlag( true );

    ErrorMgr.NoError();
}

void SetGeomMaterialName( const string &geom_id, const string &name )
{
    Vehicle* veh = GetVehicle();
    Geom* geom_ptr = veh->FindGeom( geom_id );
    if ( !geom_ptr )
    {
        ErrorMgr.AddError( VSP_INVALID_ID, "SetGeomMaterialName::Can't Find Geom " + geom_id );
        return;
    }

    Material mat;
    if( MaterialMgr.FindMaterial( name, mat ) )
    {
        geom_ptr->SetMaterial( mat.m_Name, mat.m_Ambi, mat.m_Diff, mat.m_Spec, mat.m_Emis, mat.m_Shininess );
    }
    else
    {
        geom_ptr->SetMaterialToDefault();
        ErrorMgr.AddError( VSP_CANT_FIND_NAME, "SetGeomMaterialName::Can't Find Material " + name );
        return;
    }

    ErrorMgr.NoError();
}

void AddMaterial( const string &name, const vec3d & ambient, const vec3d & diffuse, const vec3d & specular, const vec3d & emissive, const double & alpha, const double & shininess )
{
    Material tmpmat;
    if ( MaterialMgr.FindMaterial( name, tmpmat ) )
    {
        ErrorMgr.AddError( VSP_DUPLICATE_NAME, "AddMaterial::Material " + name + " already exists." );
        return;
    }

    Material mat;
    mat.m_Name = name;
    mat.SetAmbient( ambient );
    mat.SetDiffuse( diffuse );
    mat.SetSpecular( specular );
    mat.SetEmissive( emissive );
    mat.SetAlpha( alpha );
    mat.SetShininess( shininess );

    MaterialMgr.AddMaterial( mat );
    ErrorMgr.NoError();
}

vector < string > GetMaterialNames()
{
    ErrorMgr.NoError();
    return MaterialMgr.GetNames();
}

void SetBackground( double r, double g, double b )
{
#ifdef VSP_USE_FLTK
    GuiInterface::getInstance().SetBackground( r, g, b );
#endif
}

void SetAllViews( int view )
{
#ifdef VSP_USE_FLTK
    GuiInterface::getInstance().SetAllViews( view );
#endif
}

void SetView( int viewport, int view )
{
#ifdef VSP_USE_FLTK
    GuiInterface::getInstance().SetView( viewport, view );
#endif
}

void FitAllViews()
{
#ifdef VSP_USE_FLTK
    GuiInterface::getInstance().FitAllViews();
#endif
}

void ResetViews()
{
#ifdef VSP_USE_FLTK
    GuiInterface::getInstance().ResetViews();
#endif
}

void SetWindowLayout( int r, int c )
{
#ifdef VSP_USE_FLTK
    GuiInterface::getInstance().SetWindowLayout( r, c );
#endif
}

void SetGUIElementDisable( int e, bool state )
{
#ifdef VSP_USE_FLTK
    GuiInterface::getInstance().SetGUIElementDisable( e, state );
#endif
}

void SetGUIScreenDisable( int s, bool state )
{
#ifdef VSP_USE_FLTK
    GuiInterface::getInstance().SetGUIScreenDisable( s, state );
#endif
}

void SetGeomScreenDisable( int s, bool state )
{
#ifdef VSP_USE_FLTK
    GuiInterface::getInstance().SetGeomScreenDisable( s, state );
#endif
}

void HideScreen( int s )
{
#ifdef VSP_USE_FLTK
    GuiInterface::getInstance().HideScreen( s );
#endif
}

void ShowScreen( int s )
{
#ifdef VSP_USE_FLTK
    GuiInterface::getInstance().ShowScreen( s );
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
    vector< string > ret_vec( veh->GetNumGeomTypes() );
    for ( int i = 0 ; i < veh->GetNumGeomTypes() ; i++ )
    {
        ret_vec[i] = veh->GetGeomType( i ).m_Name;
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
            break;
        }
    }

    if ( type_index == -1 )
    {
        ErrorMgr.AddError( VSP_CANT_FIND_TYPE, "AddGeom::Can't Find Type Name " + type );
        return ret_id;
    }

    Geom* parent_geom = nullptr;
    if ( parent.size() > 0 )
    {
        parent_geom = veh->FindGeom( parent );
        if ( !parent_geom )
        {
            ErrorMgr.AddError( VSP_INVALID_GEOM_ID, "AddGeom::Can't Find Parent " + parent );
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
        ErrorMgr.AddError( VSP_INVALID_GEOM_ID, "AddGeom::Failed To Add Geom" );
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
vector< string > PasteGeomClipboard( const string & parent )
{
    Vehicle* veh = GetVehicle();

    Geom* parent_geom = nullptr;
    if ( parent.size() > 0 )
    {
        parent_geom = veh->FindGeom( parent );
        if ( !parent_geom )
        {
            ErrorMgr.AddError( VSP_INVALID_GEOM_ID, "PasteGeomClipboard::Can't Find Parent " + parent );
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

    vector< string> pasted_ids = veh->PasteClipboard();
    ErrorMgr.NoError();

    return pasted_ids;
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
        ErrorMgr.AddError( VSP_INVALID_PTR, "SetGeomName::Can't Find Geom " + geom_id );
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
        ErrorMgr.AddError( VSP_INVALID_PTR, "GetGeomName::Can't Find Geom " + geom_id );
        return string();
    }
    ret_name = geom_ptr->GetName();
    ErrorMgr.NoError();
    return ret_name;
}

// Get the VSP Surface type for the specified Geom (i.e DISK_SURF)
int GetGeomVSPSurfType( const string& geom_id, int main_surf_ind )
{
    Vehicle* veh = GetVehicle();
    Geom* geom_ptr = veh->FindGeom( geom_id );
    if ( !geom_ptr )
    {
        ErrorMgr.AddError( VSP_INVALID_PTR, "GetGeomVSPSurfType::Can't Find Geom " + geom_id );
        return -1;
    }

    int nms = geom_ptr->GetNumMainSurfs();

    if ( main_surf_ind < 0 || main_surf_ind >= nms )
    {
        ErrorMgr.AddError( VSP_INDEX_OUT_RANGE, "GetGeomVSPSurfType::Main Surf Index " + to_string( main_surf_ind ) + " Out of Range" );
    }

    return geom_ptr->GetMainSurfType(main_surf_ind);
}

// Get the VSP Surface CFD type for the specified Geom (i.e TRANSPARENT_SURF)
int GetGeomVSPSurfCfdType( const string& geom_id, int main_surf_ind )
{
    Vehicle* veh = GetVehicle();
    Geom* geom_ptr = veh->FindGeom( geom_id );
    if ( !geom_ptr )
    {
        ErrorMgr.AddError( VSP_INVALID_PTR, "GetGeomVSPSurfCfdType::Can't Find Geom " + geom_id );
        return -1;
    }

    if ( main_surf_ind < 0 || main_surf_ind >= geom_ptr->GetNumMainSurfs() )
    {
        ErrorMgr.AddError( VSP_INDEX_OUT_RANGE, "GetGeomVSPSurfCfdType::Main Surf Index " + to_string( main_surf_ind ) + " Out of Range" );
    }

    return geom_ptr->GetMainCFDSurfType( main_surf_ind );
}

/// Get of the linkable parms ids for this geometry
vector< string > GetGeomParmIDs( const string & geom_id )
{
    vector< string > parm_vec;

    Vehicle* veh = GetVehicle();
    Geom* geom_ptr = veh->FindGeom( geom_id );
    if ( !geom_ptr )
    {
        ErrorMgr.AddError( VSP_INVALID_PTR, "GetGeomParmIDs::Can't Find Geom " + geom_id );
        return parm_vec;
    }

    geom_ptr->AddLinkableParms( parm_vec );

    ErrorMgr.NoError();
    return parm_vec;
}

/// Get the type of for this geometry
string GetGeomTypeName( const string & geom_id )
{
    Vehicle* veh = GetVehicle();
    Geom* geom_ptr = veh->FindGeom( geom_id );
    if ( !geom_ptr )
    {
        ErrorMgr.AddError( VSP_INVALID_PTR, "GeomGeomTypeName::Can't Find Geom " + geom_id );
        return string();
    }

    string typ = string( geom_ptr->GetType().m_Name );
    return typ;
}

/// Get the parm id given container id, parm name, and group name
string GetParm( const string & container_id, const string & name, const string & group )
{
    return FindParm( container_id, name, group );
}

// Set the parent of a Geom; new_parent_id can be either the Vehicle's ID or another Geom's ID
void SetGeomParent( const string& geom_id, const string& new_parent_id )
{
    Vehicle* veh = GetVehicle();
    Geom* geom_ptr = veh->FindGeom( geom_id );
    geom_ptr->ChangeParentID( new_parent_id );
}

// Get the parent of a Geom
string GetGeomParent( const string& geom_id )
{
    Vehicle* veh = GetVehicle();
    Geom* geom_ptr = veh->FindGeom( geom_id );
    if ( !geom_ptr )
    {
        ErrorMgr.AddError( VSP_INVALID_PTR, "GetGeomParent::Can't Find Geom " + geom_id );
        return string();
    }

    return geom_ptr->GetParentID();
}

// Get all children of a Geom
vector< string > GetGeomChildren( const string& geom_id )
{
    Vehicle* veh = GetVehicle();
    Geom* geom_ptr = veh->FindGeom( geom_id );
    if ( !geom_ptr )
    {
        ErrorMgr.AddError( VSP_INVALID_PTR, "GetGeomChildren::Can't Find Geom " + geom_id );
        return vector < string > {};
    }

    return geom_ptr->GetChildIDVec();
}

/// Get the number of xsec surfs used in the construction of this geom
int GetNumXSecSurfs( const string & geom_id )
{
    Vehicle* veh = GetVehicle();
    Geom* geom_ptr = veh->FindGeom( geom_id );
    if ( !geom_ptr )
    {
        ErrorMgr.AddError( VSP_INVALID_PTR, "GetNumXSecSurfs::Can't Find Geom " + geom_id );
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
        ErrorMgr.AddError( VSP_INVALID_PTR, "GetNumMainSurfs::Can't Find Geom " + geom_id );
        return 0;
    }

    ErrorMgr.NoError();
    return geom_ptr->GetNumMainSurfs();
}

/// Get the total number of surfs for this geom
int GetTotalNumSurfs( const string& geom_id )
{
    Vehicle* veh = GetVehicle();
    Geom* geom_ptr = veh->FindGeom( geom_id );
    if ( !geom_ptr )
    {
        ErrorMgr.AddError( VSP_INVALID_PTR, "GetTotalNumSurfs::Can't Find Geom " + geom_id );
        return 0;
    }

    ErrorMgr.NoError();
    return geom_ptr->GetNumTotalSurfs();
}

// Get the maximum bounding box point for the Geom in absolute or body axes
vec3d GetGeomBBoxMax( const string& geom_id, int main_surf_ind, bool ref_frame_is_absolute )
{
    Vehicle* veh = GetVehicle();
    Geom* geom_ptr = veh->FindGeom( geom_id );
    if ( !geom_ptr )
    {
        ErrorMgr.AddError( VSP_INVALID_PTR, "GetGeomBBoxMax::Can't Find Geom " + geom_id );
        return vec3d();
    }

    int num_surf = geom_ptr->GetNumTotalSurfs();

    if ( main_surf_ind < 0 || main_surf_ind >= num_surf )
    {
        ErrorMgr.AddError( VSP_INDEX_OUT_RANGE, "GetGeomBBoxMax::Main Surf Index " + to_string( main_surf_ind) + " Out of Range" );
    }

    // Determine BndBox dimensions prior to rotating and translating
    Matrix4d model_matrix = geom_ptr->getModelMatrix();
    model_matrix.affineInverse();

    BndBox bbox;

    if ( !ref_frame_is_absolute )
    {
        VspSurf orig_surf = *( geom_ptr->GetSurfPtr( main_surf_ind ) );
        orig_surf.Transform( model_matrix );
        orig_surf.GetBoundingBox( bbox );
    }
    else
    {
        geom_ptr->GetSurfPtr( main_surf_ind )->GetBoundingBox( bbox );
    }

    return bbox.GetMax();
}

// Get the minimum bounding box point for the Geom in absolute or body axes
vec3d GetGeomBBoxMin( const string& geom_id, int main_surf_ind, bool ref_frame_is_absolute )
{
    Vehicle* veh = GetVehicle();
    Geom* geom_ptr = veh->FindGeom( geom_id );
    if ( !geom_ptr )
    {
        ErrorMgr.AddError( VSP_INVALID_PTR, "GetGeomBBoxMin::Can't Find Geom " + geom_id );
        return vec3d();
    }

    int num_surf = geom_ptr->GetNumTotalSurfs();

    if ( main_surf_ind < 0 || main_surf_ind >= num_surf )
    {
        ErrorMgr.AddError( VSP_INDEX_OUT_RANGE, "GetGeomBBoxMin::Main Surf Index " + to_string( main_surf_ind ) + " Out of Range" );
    }

    // Determine BndBox dimensions prior to rotating and translating
    Matrix4d model_matrix = geom_ptr->getModelMatrix();
    model_matrix.affineInverse();

    BndBox bbox;

    if ( !ref_frame_is_absolute )
    {
        VspSurf orig_surf = *( geom_ptr->GetSurfPtr( main_surf_ind ) );
        orig_surf.Transform( model_matrix );
        orig_surf.GetBoundingBox( bbox );
    }
    else
    {
        geom_ptr->GetSurfPtr( main_surf_ind )->GetBoundingBox( bbox );
    }

    return bbox.GetMin();
}

/// Add a sub surface, return subsurface id
string AddSubSurf( const string & geom_id, int type, int surfindex )
{
    Vehicle* veh = GetVehicle();
    Geom* geom_ptr = veh->FindGeom( geom_id );
    if ( !geom_ptr )
    {
        ErrorMgr.AddError( VSP_INVALID_PTR, "AddSubSurf::Can't Find Geom " + geom_id );
        return string();
    }

    SubSurface* ssurf = nullptr;
    ssurf = geom_ptr->AddSubSurf( type, surfindex );
    if ( !ssurf )
    {
        ErrorMgr.AddError( VSP_INVALID_PTR, "AddSubSurf::Invalid Sub Surface Ptr " );
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
    SubSurface* ssurf = nullptr;
    ssurf = geom_ptr->GetSubSurf( index );
    if ( !ssurf )
    {
        ErrorMgr.AddError( VSP_INVALID_PTR, "GetSubSurf::Invalid Sub Surface Ptr " );
        return string();
    }
    ErrorMgr.NoError();
    return ssurf->GetID();
}

/// Get IDs for all sub surface with parent geom ID and name
std::vector<std::string> GetSubSurf( const string & geom_id, const string & name )
{
    vector<string> ID_vec;
    Vehicle* veh = GetVehicle();
    Geom* geom_ptr = veh->FindGeom( geom_id );
    if ( !geom_ptr )
    {
        ErrorMgr.AddError( VSP_INVALID_PTR, "GetSubSurf::Can't Find Geom " + geom_id );
        return ID_vec;
    }
    vector<SubSurface*> ss_vec = geom_ptr->GetSubSurfVec();
    for ( size_t i = 0; i < ss_vec.size(); i++ )
    {
        if ( strcmp( ss_vec[i]->GetName().c_str(), name.c_str() ) == 0 )
        {
            ID_vec.push_back( ss_vec[i]->GetID() );
        }
    }

    if ( ID_vec.size() == 0 )
    {
        ErrorMgr.AddError( VSP_CANT_FIND_NAME, "GetSubSurf::Can't Find Sub Surface with Name " + name );
        return ID_vec;
    }
    ErrorMgr.NoError();
    return ID_vec;
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
        ErrorMgr.AddError( VSP_INVALID_PTR, "DeleteSubSurf::Can't Find SubSurf " + sub_id );
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
    SubSurface* ssurf = nullptr;
    ssurf = geom_ptr->GetSubSurf( sub_id );
    if ( !ssurf )
    {
        ErrorMgr.AddError( VSP_INVALID_PTR, "SetSubSurfName::Invalid Sub Surface Ptr " + sub_id );
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
        ErrorMgr.AddError( VSP_INVALID_PTR, "SetSubSurfName::Invalid Sub Surface Ptr " + sub_id );
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
    SubSurface* ssurf = nullptr;
    ssurf = geom_ptr->GetSubSurf( sub_id );
    if ( !ssurf )
    {
        ErrorMgr.AddError( VSP_INVALID_PTR, "GetSubSurfName::Invalid Sub Surface Ptr " + sub_id );
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
        ErrorMgr.AddError( VSP_INVALID_PTR, "GetSubSurfName::Invalid Sub Surface Ptr " + sub_id );
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
        ErrorMgr.AddError( VSP_INVALID_PTR, "GetSubSurfIndex::Invalid Sub Surface Ptr " + sub_id );
        return -1;
    }
    Vehicle* veh = GetVehicle();
    Geom* geom_ptr = veh->FindGeom( ss_ptr->GetCompID() );
    if ( !geom_ptr )
    {
        ErrorMgr.AddError( VSP_INVALID_PTR, "GetSubSurfIndex::Can't Find Geom " + ss_ptr->GetCompID() );
        return -1;
    }

    int ss_ind = geom_ptr->GetSubSurfIndex( sub_id );

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

/// Get vector of all sub surface IDs
std::vector<std::string> GetAllSubSurfIDs()
{
    vector<SubSurface*> ss_vec = SubSurfaceMgr.GetSubSurfs();
    vector<string> ID_vec;
    ID_vec.resize( ss_vec.size() );

    for ( size_t i = 0; i < ss_vec.size(); i++ )
    {
        ID_vec[i] = ss_vec[i]->GetID();
    }

    ErrorMgr.NoError();
    return ID_vec;
}

/// Get # of sub surfaces for a geom
int GetNumSubSurf( const string & geom_id )
{
    Vehicle* veh = GetVehicle();
    Geom* geom_ptr = veh->FindGeom( geom_id );
    if ( !geom_ptr )
    {
        ErrorMgr.AddError( VSP_INVALID_PTR, "GetNumSubSurf::Can't Find Geom " + geom_id );
        return -1;
    }
    ErrorMgr.NoError();
    return geom_ptr->NumSubSurfs();
}

/// Get sub surface type
int GetSubSurfType( const string & sub_id )
{
    SubSurface* ssurf = SubSurfaceMgr.GetSubSurf( sub_id );
    if ( !ssurf )
    {
        ErrorMgr.AddError( VSP_INVALID_PTR, "GetSubSurfType::Invalid Sub Surface Ptr " + sub_id );
        return -1;
    }
    ErrorMgr.NoError();
    return ssurf->GetType();
}

/// Get of the linkable parms ids for this sub surface
std::vector<std::string> GetSubSurfParmIDs( const string & sub_id )
{
    vector< string > parm_vec;

    Vehicle* veh = GetVehicle();
    SubSurface* ss_ptr = SubSurfaceMgr.GetSubSurf( sub_id );
    if ( !ss_ptr )
    {
        ErrorMgr.AddError( VSP_INVALID_PTR, "GetSubSurfParmIDs::Can't Find SubSurface " + sub_id );
        return parm_vec;
    }

    ss_ptr->AddLinkableParms( parm_vec );

    ErrorMgr.NoError();
    return parm_vec;
}

//**********************************************************************//
//*****************     FEA Mesh API Functions     *********************//
//**********************************************************************//

/// Add an FeaStructure, return FeaStructure index
int AddFeaStruct( const string & geom_id, bool init_skin, int surfindex )
{
    StructureMgr.InitFeaProperties();

    Vehicle* veh = GetVehicle();
    if ( !veh )
    {
        return -1;
    }

    Geom* geom_ptr = veh->FindGeom( geom_id );
    if ( !geom_ptr )
    {
        ErrorMgr.AddError( VSP_INVALID_PTR, "AddFeaStruct::Can't Find Geom " + geom_id );
        return -1;
    }

    FeaStructure* feastruct = nullptr;
    feastruct = geom_ptr->AddFeaStruct( init_skin, surfindex );
    if ( !feastruct )
    {
        ErrorMgr.AddError( VSP_INVALID_PTR, "AddFeaStruct::Invalid FeaStructure Ptr" );
        return -1;
    }
    ErrorMgr.NoError();
    return ( geom_ptr->NumGeomFeaStructs() - 1 );
}

void SetFeaMeshStructIndex( int struct_index )
{
    FeaStructure* feastruct = StructureMgr.GetFeaStruct( struct_index );

    if ( feastruct )
    {
        FeaMeshMgr.SetFeaMeshStructID( feastruct->GetID() );
        ErrorMgr.NoError();
    }
    else
    {
        ErrorMgr.AddError( VSP_INDEX_OUT_RANGE, "SetFeaMeshStructIndex::Index Out of Range" );
    }
}

void DeleteFeaStruct( const string & geom_id, int fea_struct_ind )
{
    Vehicle* veh = GetVehicle();
    if ( !veh )
    {
        return;
    }

    Geom* geom_ptr = veh->FindGeom( geom_id );
    if ( !geom_ptr )
    {
        ErrorMgr.AddError( VSP_INVALID_PTR, "DeleteFeaStruct::Can't Find Geom " + geom_id );
        return;
    }
    if ( !geom_ptr->ValidGeomFeaStructInd( fea_struct_ind ) )
    {
        ErrorMgr.AddError( VSP_INVALID_PTR, "DeleteFeaStruct::Can't Find FeaStructure at index " + to_string( fea_struct_ind ) );
        return;
    }
    geom_ptr->DeleteFeaStruct( fea_struct_ind );
    ErrorMgr.NoError();
    return;
}

string GetFeaStructID( const string & geom_id, int fea_struct_ind )
{
    Vehicle* veh = GetVehicle();
    if ( !veh )
    {
        return string();
    }

    Geom* geom_ptr = veh->FindGeom( geom_id );
    if ( !geom_ptr )
    {
        ErrorMgr.AddError( VSP_INVALID_PTR, "GetFeaStructID::Can't Find Geom " + geom_id );
        return string();
    }

    FeaStructure* struct_ptr = geom_ptr->GetFeaStruct( fea_struct_ind );
    if ( !struct_ptr )
    {
        ErrorMgr.AddError( VSP_INVALID_PTR, "GetFeaStructID::Can't Find FeaStructure " + to_string( ( long long ) fea_struct_ind ) );
        return string();
    }
    ErrorMgr.NoError();
    return struct_ptr->GetID();
}

int GetFeaStructIndex( const string & struct_id )
{
    return StructureMgr.GetGeomFeaStructIndex( struct_id );
}

string GetFeaStructParentGeomID( const string & struct_id )
{
    return StructureMgr.GetFeaStructParentID( struct_id );
}

string GetFeaStructName( const string & geom_id, int fea_struct_ind )
{
    Vehicle* veh = GetVehicle();
    if ( !veh )
    {
        return string();
    }

    Geom* geom_ptr = veh->FindGeom( geom_id );
    if ( !geom_ptr )
    {
        ErrorMgr.AddError( VSP_INVALID_PTR, "GetFeaStructName::Can't Find Geom " + geom_id );
        return string();
    }

    FeaStructure* struct_ptr = geom_ptr->GetFeaStruct( fea_struct_ind );
    if ( !struct_ptr )
    {
        ErrorMgr.AddError( VSP_INVALID_PTR, "GetFeaStructName::Can't Find FeaStructure " + to_string( ( long long ) fea_struct_ind ) );
        return string();
    }
    ErrorMgr.NoError();
    return struct_ptr->GetName();
}

void SetFeaStructName( const string & geom_id, int fea_struct_ind, const string & name )
{
    Vehicle* veh = GetVehicle();
    if ( !veh )
    {
        return;
    }

    Geom* geom_ptr = veh->FindGeom( geom_id );
    if ( !geom_ptr )
    {
        ErrorMgr.AddError( VSP_INVALID_PTR, "SetFeaStructName::Can't Find Geom " + geom_id );
        return;
    }

    FeaStructure* struct_ptr = geom_ptr->GetFeaStruct( fea_struct_ind );
    if ( !struct_ptr )
    {
        ErrorMgr.AddError( VSP_INVALID_PTR, "SetFeaStructName::Can't Find FeaStructure " + to_string( ( long long ) fea_struct_ind ) );
        return;
    }
    struct_ptr->SetName( name );
    ErrorMgr.NoError();
    return;
}

void SetFeaPartName( const string & part_id, const string & name )
{
    FeaPart* part = StructureMgr.GetFeaPart( part_id );
    if ( !part )
    {
        ErrorMgr.AddError( VSP_INVALID_PTR, "SetFeaPartName::Can't Find FEA Part " + part_id + " to name " + name );
        return;
    }

    part->SetName( name );
    ErrorMgr.NoError();
    return;
}

vector< string > GetFeaStructIDVec()
{
    vector < string > ret_vec;
    Vehicle* veh = GetVehicle();
    if ( !veh )
    {
        return ret_vec;
    }

    vector < FeaStructure* > struct_vec = StructureMgr.GetAllFeaStructs();
    ret_vec.resize( struct_vec.size() );

    for ( size_t i = 0; i < struct_vec.size(); i++ )
    {
        ret_vec[i] = struct_vec[i]->GetID();
    }

    ErrorMgr.NoError();
    return ret_vec;
}

/// Add a FeaPart, return FeaPart ID
string AddFeaPart( const string & geom_id, int fea_struct_ind, int type )
{
    Vehicle* veh = GetVehicle();
    if ( !veh )
    {
        return string();
    }

    Geom* geom_ptr = veh->FindGeom( geom_id );
    if ( !geom_ptr )
    {
        ErrorMgr.AddError( VSP_INVALID_PTR, "AddFeaPart::Can't Find Geom " + geom_id );
        return string();
    }

    FeaStructure* feastruct = nullptr;
    feastruct = geom_ptr->GetFeaStruct( fea_struct_ind );
    if ( !feastruct )
    {
        ErrorMgr.AddError( VSP_INVALID_PTR, "AddFeaPart::Invalid FeaStructure Ptr at Index " + to_string( (long long)fea_struct_ind ) );
        return string();
    }

    FeaPart* feapart = nullptr;
    feapart = feastruct->AddFeaPart( type );
    if ( !feapart )
    {
        ErrorMgr.AddError( VSP_INVALID_PTR, "AddFeaPart::Invalid FeaPart Ptr" );
        return string();
    }
    feastruct->Update();
    ErrorMgr.NoError();
    return feapart->GetID();
}

void DeleteFeaPart( const string & geom_id, int fea_struct_ind, const string & part_id )
{
    Vehicle* veh = GetVehicle();
    if ( !veh )
    {
        return;
    }

    Geom* geom_ptr = veh->FindGeom( geom_id );
    if ( !geom_ptr )
    {
        ErrorMgr.AddError( VSP_INVALID_PTR, "DeleteFeaPart::Can't Find Geom " + geom_id );
        return;
    }

    FeaStructure* feastruct = nullptr;
    feastruct = geom_ptr->GetFeaStruct( fea_struct_ind );
    if ( !feastruct )
    {
        ErrorMgr.AddError( VSP_INVALID_PTR, "DeleteFeaPart::Invalid FeaStructure Ptr at index " + to_string( ( long long ) fea_struct_ind ) );
        return;
    }

    int index = feastruct->GetFeaPartIndex( part_id );
    if ( index == -1 )
    {
        ErrorMgr.AddError( VSP_INVALID_PTR, "DeleteFeaPart::Can't Find FeaPart " + part_id );
        return;
    }
    feastruct->DelFeaPart( index );
    ErrorMgr.NoError();
    return;
}

string GetFeaPartID( const string & fea_struct_id, int fea_part_index )
{
    FeaStructure* feastruct = nullptr;
    feastruct = StructureMgr.GetFeaStruct( fea_struct_id );
    if ( !feastruct )
    {
        ErrorMgr.AddError( VSP_INVALID_ID, "GetFeaPartID::Invalid FeaStructure ID" );
        return string();
    }

    FeaPart* part = feastruct->GetFeaPart( fea_part_index );

    if ( !part )
    {
        ErrorMgr.AddError( VSP_INDEX_OUT_RANGE, "GetFeaPartID::Invalid FeaPart Index" );
        return string();
    }

    ErrorMgr.NoError();
    return part->GetID();
}

string GetFeaPartName( const string & part_id )
{
    FeaPart* part = StructureMgr.GetFeaPart( part_id );
    if ( !part )
    {
        ErrorMgr.AddError( VSP_INVALID_PTR, "GetFeaPartName::Can't Find FEA Part " + part_id );
        return string();
    }

    ErrorMgr.NoError();
    return part->GetName();
}

int GetFeaPartType( const string & part_id )
{
    FeaPart* part = StructureMgr.GetFeaPart( part_id );
    if ( !part )
    {
        ErrorMgr.AddError( VSP_INVALID_PTR, "GetFeaPartType::Can't Find FEA Part " + part_id );
        return - 1;
    }

    ErrorMgr.NoError();
    return part->GetType();
}

void SetFeaPartPerpendicularSparID( const string& part_id, const string& perpendicular_spar_id )
{
    FeaPart* part = StructureMgr.GetFeaPart( part_id );
    if ( !part )
    {
        ErrorMgr.AddError( VSP_INVALID_PTR, "SetFeaPartPerpendicularSparID::Can't Find FEA Part " + part_id );
        return;
    }

    bool rib_type = part->GetType() == FEA_RIB;
    bool rib_array_type = part->GetType() == FEA_RIB_ARRAY;

    if ( !( rib_type || rib_array_type ) )
    {
        ErrorMgr.AddError( VSP_INVALID_TYPE, "SetFeaPartPerpendicularSparID::FEA Part is not Rib or Rib Array Type" );
        return;
    }

    // Check if Spar exists (not really necessary, but should be helpful)
    FeaPart* spar = StructureMgr.GetFeaPart( perpendicular_spar_id );
    if ( !spar || spar->GetType() != FEA_SPAR )
    {
        ErrorMgr.AddError( VSP_INVALID_PTR, "SetFeaPartPerpendicularSparID::Can't Find FEA Spar " + perpendicular_spar_id + ". ID will still be set." );
    }

    if ( rib_type )
    {
        FeaRib* rib = dynamic_cast<FeaRib*>( part );
        assert( rib );

        rib->SetPerpendicularEdgeID( perpendicular_spar_id );
    }
    else if ( rib_array_type )
    {
        FeaRibArray* rib_array = dynamic_cast<FeaRibArray*>( part );
        assert( rib_array );

        rib_array->SetPerpendicularEdgeID( perpendicular_spar_id );
    }
}

string GetFeaPartPerpendicularSparID( const string& part_id )
{
    FeaPart* part = StructureMgr.GetFeaPart( part_id );
    string ret_str = string();
    if ( !part )
    {
        ErrorMgr.AddError( VSP_INVALID_PTR, "GetFeaPartPerpendicularSparID::Can't Find FEA Part " + part_id );
        return ret_str;
    }

    bool rib_type = part->GetType() == FEA_RIB;
    bool rib_array_type = part->GetType() == FEA_RIB_ARRAY;

    if ( !( rib_type || rib_array_type ) )
    {
        ErrorMgr.AddError( VSP_INVALID_TYPE, "GetFeaPartPerpendicularSparID::FEA Part is not Rib or Rib Array Type" );
        return ret_str;
    }

    if ( rib_type )
    {
        FeaRib* rib = dynamic_cast<FeaRib*>( part );
        assert( rib );
        ret_str = rib->GetPerpendicularEdgeID();
    }
    else if ( rib_array_type )
    {
        FeaRibArray* rib_array = dynamic_cast<FeaRibArray*>( part );
        assert( rib_array );
        ret_str = rib_array->GetPerpendicularEdgeID();
    }

    return ret_str;
}

void SetFeaSubSurfName( const string & subsurf_id, const string & name )
{
    SubSurface* ssurf = StructureMgr.GetFeaSubSurf( subsurf_id );

    if ( !ssurf )
    {
        ErrorMgr.AddError( VSP_INVALID_PTR, "SetFeaSubSurfName::Can't Find FEA Sub Surface " + subsurf_id + " to name " + name );
        return;
    }

    ssurf->SetName( name );
    ErrorMgr.NoError();
    return;
}

string GetFeaSubSurfName( const string & subsurf_id )
{
    SubSurface* ssurf = StructureMgr.GetFeaSubSurf( subsurf_id );
    if ( !ssurf )
    {
        ErrorMgr.AddError( VSP_INVALID_PTR, "GetFeaSubSurfName::Can't Find FEA SubSurf " + subsurf_id );
        return string();
    }

    ErrorMgr.NoError();
    return ssurf->GetName();
}

/// Add a FeaSubSurface, return FeaSubSurface ID
string AddFeaSubSurf( const string & geom_id, int fea_struct_ind, int type )
{
    Vehicle* veh = GetVehicle();
    if ( !veh )
    {
        return string();

    }
    Geom* geom_ptr = veh->FindGeom( geom_id );
    if ( !geom_ptr )
    {
        ErrorMgr.AddError( VSP_INVALID_PTR, "AddFeaSubSurf::Can't Find Geom " + geom_id );
        return string();
    }

    FeaStructure* feastruct = nullptr;
    feastruct = geom_ptr->GetFeaStruct( fea_struct_ind );
    if ( !feastruct )
    {
        ErrorMgr.AddError( VSP_INVALID_PTR, "AddFeaSubSurf::Invalid FeaStructure Ptr at index " + to_string( (long long)fea_struct_ind ) );
        return string();
    }

    SubSurface* feasubsurf = nullptr;
    feasubsurf = feastruct->AddFeaSubSurf( type );
    if ( !feasubsurf )
    {
        ErrorMgr.AddError( VSP_INVALID_PTR, "AddFeaSubSurf::Invalid FeaSubSurface Ptr" );
        return string();
    }
    feastruct->Update();
    ErrorMgr.NoError();
    return feasubsurf->GetID();
}

void DeleteFeaSubSurf( const string & geom_id, int fea_struct_ind, const string & ss_id )
{
    Vehicle* veh = GetVehicle();
    if ( !veh )
    {
        return;
    }

    Geom* geom_ptr = veh->FindGeom( geom_id );
    if ( !geom_ptr )
    {
        ErrorMgr.AddError( VSP_INVALID_PTR, "DeleteFeaSubSurf::Can't Find Geom " + geom_id );
        return;
    }

    FeaStructure* feastruct = nullptr;
    feastruct = geom_ptr->GetFeaStruct( fea_struct_ind );
    if ( !feastruct )
    {
        ErrorMgr.AddError( VSP_INVALID_PTR, "DeleteFeaSubSurf::Invalid FeaStructure Ptr at index " + to_string( (long long)fea_struct_ind ) );
        return;
    }

    int index = StructureMgr.GetFeaSubSurfIndex( ss_id );
    if ( index == -1 )
    {
        ErrorMgr.AddError( VSP_INVALID_PTR, "DeleteFeaSubSurf::Can't Find FeaSubSurf " + ss_id );
        return;
    }
    feastruct->DelFeaSubSurf( index );
    ErrorMgr.NoError();
    return;
}

int GetFeaSubSurfIndex( const string & ss_id )
{
    int index = StructureMgr.GetFeaSubSurfIndex( ss_id );
    if ( index < 0 )
    {
        ErrorMgr.AddError( VSP_INVALID_PTR, "GetFeaSubSurfIndex::Can't Find FeaSubSurf " + ss_id );
        return index;
    }
    ErrorMgr.NoError();
    return index;
}

int NumFeaStructures()
{
    return StructureMgr.NumFeaStructures();
}

int NumFeaParts( const string & fea_struct_id )
{
    FeaStructure* feastruct = StructureMgr.GetFeaStruct( fea_struct_id );
    if ( !feastruct )
    {
        ErrorMgr.AddError( VSP_INVALID_ID, "NumFeaParts::Invalid FeaStructure ID " + fea_struct_id );
        return -1;
    }

    ErrorMgr.NoError();
    return feastruct->NumFeaParts();
}

int NumFeaSubSurfs( const string & fea_struct_id )
{
    FeaStructure* feastruct = StructureMgr.GetFeaStruct( fea_struct_id );
    if ( !feastruct )
    {
        ErrorMgr.AddError( VSP_INVALID_ID, "NumFeaSubSurfs::Invalid FeaStructure ID " + fea_struct_id );
        return -1;
    }

    ErrorMgr.NoError();
    return feastruct->NumFeaSubSurfs();
}

vector< string > GetFeaPartIDVec( const string & fea_struct_id )
{
    vector < string > ret_vec;
    FeaStructure* feastruct = StructureMgr.GetFeaStruct( fea_struct_id );
    if ( !feastruct )
    {
        ErrorMgr.AddError( VSP_INVALID_ID, "GetFeaPartIDVec::Invalid FeaStructure ID" );
        return ret_vec;
    }

    vector < FeaPart* > fea_part_vec = feastruct->GetFeaPartVec();
    ret_vec.resize( fea_part_vec.size() );

    for ( size_t i = 0; i < fea_part_vec.size(); i++ )
    {
        ret_vec[i] = fea_part_vec[i]->GetID();
    }

    ErrorMgr.NoError();
    return ret_vec;
}

vector< string > GetFeaSubSurfIDVec( const string & fea_struct_id )
{
    vector < string > ret_vec;
    FeaStructure* feastruct = StructureMgr.GetFeaStruct( fea_struct_id );
    if ( !feastruct )
    {
        ErrorMgr.AddError( VSP_INVALID_ID, "GetFeaSubSurfIDVec::Invalid FeaStructure ID " + fea_struct_id );
        return ret_vec;
    }

    vector < SubSurface* > fea_ss_vec = feastruct->GetFeaSubSurfVec();
    ret_vec.resize( fea_ss_vec.size() );

    for ( size_t i = 0; i < fea_ss_vec.size(); i++ )
    {
        ret_vec[i] = fea_ss_vec[i]->GetID();
    }

    ErrorMgr.NoError();
    return ret_vec;
}

std::string AddFeaBC( const string & fea_struct_id, int type )
{
    FeaStructure* feastruct = StructureMgr.GetFeaStruct( fea_struct_id );
    if ( !feastruct )
    {
        ErrorMgr.AddError( VSP_INVALID_ID, "AddFeaBC::Invalid FeaStructure ID " + fea_struct_id );
        return string();
    }

    FeaBC* bc = feastruct->AddFeaBC( type );
    if ( !bc )
    {
        ErrorMgr.AddError( VSP_INVALID_PTR, "AddFeaBC::Invalid FeaPart Ptr" );
        return string();
    }

    string bc_id = bc->GetID();

    ErrorMgr.NoError();
    return bc_id;
}

void DelFeaBC( const string & fea_struct_id, const std::string &bc_id )
{
    FeaStructure* feastruct = StructureMgr.GetFeaStruct( fea_struct_id );
    if ( !feastruct )
    {
        ErrorMgr.AddError( VSP_INVALID_ID, "DelFeaBC::Invalid FeaStructure ID " + fea_struct_id );
        return;
    }

    int indx = feastruct->GetFeaBCIndex( bc_id );

    if ( !feastruct->ValidFeaBCInd( indx ) )
    {
        ErrorMgr.AddError( VSP_INVALID_PTR, "DelFeaBC::Can't Find FeaBC " + bc_id );
        return;
    }

    feastruct->DelFeaBC( indx );

    ErrorMgr.NoError();
}

std::vector< std::string > GetFeaBCIDVec( const string & fea_struct_id )
{
    vector < string > ret_vec;
    FeaStructure* feastruct = StructureMgr.GetFeaStruct( fea_struct_id );
    if ( !feastruct )
    {
        ErrorMgr.AddError( VSP_INVALID_ID, "GetFeaBCVec::Invalid FeaStructure ID " + fea_struct_id );
        return ret_vec;
    }

    std::vector< FeaBC* > bcvec = feastruct->GetFeaBCVec();

    for ( int i = 0; i < bcvec.size(); i++ )
    {
        if ( bcvec[i] )
        {
            ret_vec.push_back( bcvec[i]->GetID() );
        }
    }

    ErrorMgr.NoError();
    return ret_vec;
}

int NumFeaBCs( const string & fea_struct_id )
{
    FeaStructure* feastruct = StructureMgr.GetFeaStruct( fea_struct_id );
    if ( !feastruct )
    {
        ErrorMgr.AddError( VSP_INVALID_ID, "NumFeaBCs::Invalid FeaStructure ID " + fea_struct_id );
        return -1;
    }

    int n = feastruct->NumFeaBCs();

    ErrorMgr.NoError();
    return n;
}

/// Add an FeaMaterial, return FeaMaterial ID
string AddFeaMaterial()
{
    FeaMaterial* feamat = nullptr;
    feamat = StructureMgr.AddFeaMaterial();
    if ( !feamat )
    {
        ErrorMgr.AddError( VSP_INVALID_PTR, "AddFeaMaterial::Invalid FeaMaterial Ptr" );
        return string();
    }
    ErrorMgr.NoError();
    return feamat->GetID();
}

/// Add an FeaProperty, return FeaProperty ID. The default is shell property type
string AddFeaProperty( int property_type )
{
    FeaProperty* feaprop = nullptr;
    feaprop = StructureMgr.AddFeaProperty( property_type );
    if ( !feaprop )
    {
        ErrorMgr.AddError( VSP_INVALID_PTR, "AddFeaProperty::Invalid FeaProperty Ptr" );
        return string();
    }
    ErrorMgr.NoError();
    return feaprop->GetID();
}

//==== Set a FEA Mesh Control Val =====//
void SetFeaMeshVal( const string & geom_id, int fea_struct_ind, int type, double val )
{
    Vehicle* veh = GetVehicle();
    if ( !veh )
    {
        return;
    }

    Geom* geom_ptr = veh->FindGeom( geom_id );
    if ( !geom_ptr )
    {
        ErrorMgr.AddError( VSP_INVALID_PTR, "SetFEAMeshVal::Can't Find Geom " + geom_id );
        return;
    }

    FeaStructure* feastruct = nullptr;
    feastruct = geom_ptr->GetFeaStruct( fea_struct_ind );
    if ( !feastruct )
    {
        ErrorMgr.AddError( VSP_INVALID_PTR, "SetFEAMeshVal::Invalid FeaStructure Ptr at index " + to_string( (long long)fea_struct_ind ) );
        return;
    }

    // Type as CFD or FEA Mesh Setting Enums?
    if ( type == CFD_MIN_EDGE_LEN )
        feastruct->GetFeaGridDensityPtr()->m_MinLen = val;
    else if ( type == CFD_MAX_EDGE_LEN )
        feastruct->GetFeaGridDensityPtr()->m_BaseLen = val;
    else if ( type == CFD_MAX_GAP )
        feastruct->GetFeaGridDensityPtr()->m_MaxGap = val;
    else if ( type == CFD_NUM_CIRCLE_SEGS )
        feastruct->GetFeaGridDensityPtr()->m_NCircSeg = val;
    else if ( type == CFD_GROWTH_RATIO )
        feastruct->GetFeaGridDensityPtr()->m_GrowRatio = val;
    else if ( type == CFD_LIMIT_GROWTH_FLAG )
        feastruct->GetFeaGridDensityPtr()->SetRigorLimit( ToBool( val ) );
    else if ( type == CFD_HALF_MESH_FLAG )
        feastruct->GetStructSettingsPtr()->m_HalfMeshFlag = ToBool( val );
    else
    {
        ErrorMgr.AddError( VSP_CANT_FIND_TYPE, "SetFEAMeshVal::Can't Find Type " + to_string( (long long)type ) );
        return;
    }

    ErrorMgr.NoError();
}

void SetFeaMeshFileName( const string & geom_id, int fea_struct_ind, int file_type, const string & file_name )
{
    Vehicle* veh = GetVehicle();
    if ( !veh )
    {
        return;
    }

    Geom* geom_ptr = veh->FindGeom( geom_id );
    if ( !geom_ptr )
    {
        ErrorMgr.AddError( VSP_INVALID_PTR, "SetFeaMeshFileNames::Can't Find Geom " + geom_id );
        return;
    }

    FeaStructure* feastruct = nullptr;
    feastruct = geom_ptr->GetFeaStruct( fea_struct_ind );
    if ( !feastruct )
    {
        ErrorMgr.AddError( VSP_INVALID_PTR, "SetFeaMeshFileNames::Invalid FeaStructure Ptr at index " + to_string( (long long)fea_struct_ind ) );
        return;
    }

    feastruct->GetStructSettingsPtr()->SetExportFileName( file_name, file_type );

    ErrorMgr.NoError();
}

/// Compute the FEA Mesh
void ComputeFeaMesh( const string & geom_id, int fea_struct_ind, int file_type )
{
    Update(); // Remove?
    Vehicle* veh = GetVehicle();
    if ( !veh )
    {
        return;
    }

    Geom* geom_ptr = veh->FindGeom( geom_id );
    if ( !geom_ptr )
    {
        ErrorMgr.AddError( VSP_INVALID_PTR, "ComputeFEAMesh::Can't Find Geom " + geom_id );
        return;
    }

    FeaStructure* feastruct = geom_ptr->GetFeaStruct( fea_struct_ind );
    if ( !feastruct )
    {
        ErrorMgr.AddError( VSP_INVALID_PTR, "ComputeFEAMesh::Invalid FeaStructure Ptr at index " + to_string( (long long)fea_struct_ind ) );
        return;
    }

    feastruct->GetStructSettingsPtr()->SetAllFileExportFlags( false );
    feastruct->GetStructSettingsPtr()->SetFileExportFlag( file_type, true );

    FeaMeshMgr.SetFeaMeshStructID( feastruct->GetID() );

    FeaMeshMgr.addOutputText( "CLEAR_TERMINAL" );
    FeaMeshMgr.GenerateFeaMesh();

    FeaMeshMgr.ExportFeaMesh( feastruct->GetID() );

    ErrorMgr.NoError();
}

void ComputeFeaMesh( const string & struct_id, int file_type )
{
    Update(); // Not sure if this is needed

    FeaStructure* feastruct = StructureMgr.GetFeaStruct( struct_id );
    if ( !feastruct )
    {
        ErrorMgr.AddError( VSP_INVALID_PTR, "ComputeFEAMesh::Can't Find Structure " + struct_id );
        return;
    }

    feastruct->GetStructSettingsPtr()->SetAllFileExportFlags( false );
    feastruct->GetStructSettingsPtr()->SetFileExportFlag( file_type, true );

    FeaMeshMgr.SetFeaMeshStructID( struct_id );

    FeaMeshMgr.addOutputText( "CLEAR_TERMINAL" );
    FeaMeshMgr.GenerateFeaMesh();

    FeaMeshMgr.ExportFeaMesh( struct_id );

    ErrorMgr.NoError();
}

void SetXSecAlias( const string & id, const string & alias )
{
    XSec* xs = FindXSec( id );
    if ( !xs )
    {
        ErrorMgr.AddError( VSP_INVALID_PTR, "SetXSecAlias::Can't Find XSec " + id );
    }

    ErrorMgr.NoError();
    xs->SetGroupAlias( alias );
}

string GetXSecAlias( const string & id )
{
    XSec* xs = FindXSec( id );
    if ( !xs )
    {
        ErrorMgr.AddError( VSP_INVALID_PTR, "GetXSecAlias::Can't Find XSec " + id );
        return string();
    }

    ErrorMgr.NoError();
    return xs->GetGroupAlias();
}

void SetXSecCurveAlias( const string & id, const string & alias )
{
    XSec* xs = FindXSec( id );
    if ( !xs )
    {
        ErrorMgr.AddError( VSP_INVALID_PTR, "SetXSecCurveAlias::Can't Find XSec " + id );
    }
    XSecCurve* xsc = xs->GetXSecCurve();

    ErrorMgr.NoError();
    xsc->SetGroupAlias( alias );
}

string GetXSecCurveAlias( const string & id )
{
    XSec* xs = FindXSec( id );
    if ( !xs )
    {
        ErrorMgr.AddError( VSP_INVALID_PTR, "GetXSecCurveAlias::Can't Find XSec " + id );
        return string();
    }
    XSecCurve* xsc = xs->GetXSecCurve();

    ErrorMgr.NoError();
    return xsc->GetGroupAlias();
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
        ErrorMgr.AddError( VSP_INVALID_PTR, "PasteXSec::Can't Find Geom " + geom_id );
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
        ErrorMgr.AddError( VSP_INVALID_PTR, "InsertXSec::Can't Find Geom " + geom_id );
        return;
    }

    geom_ptr->InsertXSec( index, type );
    ErrorMgr.NoError();
}


//===================================================================//
//===============       Wing Section Functions     ==================//
//===================================================================//

void SplitWingXSec( const string & wing_id, int section_index )
{
    Vehicle* veh = GetVehicle();
    Geom* geom_ptr = veh->FindGeom( wing_id );
    if ( !geom_ptr )
    {
        ErrorMgr.AddError( VSP_INVALID_PTR, "SplitWingXSec::Can't Find Geom " + wing_id );
        return;
    }

    WingGeom* wg = dynamic_cast<WingGeom*>( geom_ptr );
    if ( !wg )
    {
        ErrorMgr.AddError( VSP_INVALID_TYPE, "SplitWingXSec::Geom not a WingGeom" + wing_id );
        return;
    }

    wg->SplitWingSect( section_index );
    ErrorMgr.NoError();
}

void SetDriverGroup( const string & geom_id, int section_index, int driver_0, int driver_1, int driver_2 )
{
    Vehicle* veh = GetVehicle();
    Geom* geom_ptr = veh->FindGeom( geom_id );
    if ( !geom_ptr )
    {
        ErrorMgr.AddError( VSP_INVALID_PTR, "SetDriverGroup::Can't Find Geom " + geom_id );
        return;
    }

    if ( geom_ptr->GetType().m_Type == MS_WING_GEOM_TYPE )
    {
        WingGeom* wg = dynamic_cast<WingGeom*>( geom_ptr );
        WingSect* ws = wg->GetWingSect( section_index );
        if ( !ws )
        {
            ErrorMgr.AddError( VSP_INVALID_PTR, "SetDriverGroup::Invalid Wing Section Index " + to_string( ( long long )section_index ) );
            return;
        }

        vector < int > prevchoices = ws->m_DriverGroup.GetChoices();

        ws->m_DriverGroup.SetChoice( 0, driver_0 );
        ws->m_DriverGroup.SetChoice( 1, driver_1 );
        ws->m_DriverGroup.SetChoice( 2, driver_2 );

        bool valid = ws->m_DriverGroup.ValidDrivers( ws->m_DriverGroup.GetChoices() );
        if ( !valid )
        {
            ErrorMgr.AddError( VSP_INVALID_DRIVERS, "SetDriverGroup::Invalid wing drivers." );
            ws->m_DriverGroup.SetChoices( prevchoices );
            return;
        }

        ErrorMgr.NoError();
        return;
    }
    else
    {
        GeomXSec* gxs = dynamic_cast < GeomXSec* > ( geom_ptr );

        XSecCurve* xsc = nullptr;
        if ( gxs ) // Proceed as GeomXSec
        {
            xsc = gxs->GetXSec( section_index )->GetXSecCurve();
        }
        else
        {
            BORGeom* bor = dynamic_cast < BORGeom* > ( geom_ptr );
            if ( bor ) // Proceed as Body of Revolution
            {
                xsc = bor->GetXSecCurve();
            }
        }

        if ( xsc ) // Succeeded in getting an XSecCurve
        {
            vector < int > prevchoices = xsc->m_DriverGroup->GetChoices();

            // Only driver 0 used for Circles.
            xsc->m_DriverGroup->SetChoice( 0, driver_0 );

            // Driver 1 used for other XSecCurve types.
            if ( driver_1 > -1 )
                xsc->m_DriverGroup->SetChoice( 1, driver_1 );

            bool valid = xsc->m_DriverGroup->ValidDrivers( xsc->m_DriverGroup->GetChoices() );
            if ( !valid )
            {
                ErrorMgr.AddError( VSP_INVALID_DRIVERS, "SetDriverGroup::Invalid XSecCurve drivers." );
                xsc->m_DriverGroup->SetChoices( prevchoices );
                return;
            }
            ErrorMgr.NoError();
            return;
        }

    }
    ErrorMgr.AddError( VSP_INVALID_PTR, "SetDriverGroup::Invalid Geom Type " + geom_id );
    return;
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
        ErrorMgr.AddError( VSP_INVALID_PTR, "GetXSecSurf::Can't Find Geom " + geom_id );
        return string();
    }
    XSecSurf* xsec_surf = geom_ptr->GetXSecSurf( index );

    if ( !xsec_surf )
    {
        ErrorMgr.AddError( VSP_INVALID_PTR, "GetXSecSurf::Can't Find XSecSurf " + geom_id + ":" + to_string( ( long long )index ) );
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
        ErrorMgr.AddError( VSP_INVALID_PTR, "GetNumXSec::Can't Find XSecSurf " + xsec_surf_id );
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
        ErrorMgr.AddError( VSP_INVALID_PTR, "GetXSec::Can't Find XSecSurf " + xsec_surf_id );
        return string();
    }
    XSec* xsec = xsec_surf->FindXSec( xsec_index );
    if ( !xsec )
    {
        ErrorMgr.AddError( VSP_INVALID_PTR, "GetXSec::Can't Find XSec " + xsec_surf_id + ":" + to_string( ( long long )xsec_index ) );
        return string();
    }

    ErrorMgr.NoError();
    return xsec->GetID();
}

/// Change the shape of a particular XSec
void ChangeXSecShape( const string & xsec_surf_id, int xsec_index, int type )
{
    XSecSurf* xsec_surf = FindXSecSurf( xsec_surf_id );
    if ( !xsec_surf )
    {
        ErrorMgr.AddError( VSP_INVALID_PTR, "ChangeXSecShape::Can't Find XSecSurf " + xsec_surf_id );
        return;
    }
    if ( xsec_index < 0 || xsec_index >= xsec_surf->NumXSec() )
    {
        ErrorMgr.AddError( VSP_INDEX_OUT_RANGE, "ChangeXSecShape::XSec Index Out of Range " + xsec_surf_id + ":" + to_string( ( long long )xsec_index ) );
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
        ErrorMgr.AddError( VSP_INVALID_PTR, "SetXSecSurfGlobalXForm::Can't Find XSecSurf " + xsec_surf_id );
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
        ErrorMgr.AddError( VSP_INVALID_PTR, "GetXSecSurfGlobalXForm::Can't Find XSecSurf " + xsec_surf_id );
        return Matrix4d();
    }
    return xsec_surf->GetGlobalXForm();
}

//===================================================================//
//=================       XSec Functions         ====================//
//===================================================================//
/// Get XSec shape given xsec id
int GetXSecShape( const string& xsec_id )
{
    XSec* xs = FindXSec( xsec_id );
    if ( !xs )
    {
        ErrorMgr.AddError( VSP_INVALID_PTR, "GetXSecShape::Can't Find XSec " + xsec_id );
        return XS_UNDEFINED;
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
        ErrorMgr.AddError( VSP_INVALID_PTR, "GetXSecWidth::Can't Find XSec " + xsec_id );
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
        ErrorMgr.AddError( VSP_INVALID_PTR, "GetXSecHeight::Can't Find XSec " + xsec_id );
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
        ErrorMgr.AddError( VSP_INVALID_PTR, "SetXSecWidthHeight::Can't Find XSec " + xsec_id );
        return;
    }
    xs->GetXSecCurve()->SetWidthHeight( w, h );
    xs->ParmChanged( nullptr, Parm::SET_FROM_DEVICE ); // Force Update
    ErrorMgr.NoError();
}

void SetXSecWidth( const string& xsec_id, double w )
{
    XSec* xs = FindXSec( xsec_id );
    if ( !xs )
    {
        ErrorMgr.AddError( VSP_INVALID_PTR, "SetXSecWidth::Can't Find XSec " + xsec_id );
        return;
    }
    Parm* width_parm = ParmMgr.FindParm( xs->GetXSecCurve()->GetWidthParmID() );
    if ( !width_parm )
    {
        ErrorMgr.AddError( VSP_INVALID_PTR, "SetXSecWidth::Can't Find Width Parm " + xs->GetXSecCurve()->GetWidthParmID() );
        return;
    }
    width_parm->SetFromDevice( w );
    ErrorMgr.NoError();
}

void SetXSecHeight( const string& xsec_id, double h )
{
    XSec* xs = FindXSec( xsec_id );
    if ( !xs )
    {
        ErrorMgr.AddError( VSP_INVALID_PTR, "SetXSecHeight::Can't Find XSec " + xsec_id );
        return;
    }
    Parm* height_parm = ParmMgr.FindParm( xs->GetXSecCurve()->GetHeightParmID() );
    if ( !height_parm )
    {
        ErrorMgr.AddError( VSP_INVALID_PTR, "SetXSecHeight::Can't Find Width Parm " + xs->GetXSecCurve()->GetHeightParmID() );
        return;
    }
    height_parm->SetFromDevice( h );
    ErrorMgr.NoError();
}

/// Get of the linkable parms ids for this geometry
vector< string > GetXSecParmIDs( const string & xsec_id )
{
    vector< string > parm_vec;

    XSec* xs = FindXSec( xsec_id );
    if ( !xs )
    {
        ErrorMgr.AddError( VSP_INVALID_PTR, "GetXSecParmIDs::Can't Find XSec " + xsec_id );
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
        ErrorMgr.AddError( VSP_INVALID_PTR, "GetXSecParm::Can't Find XSec " + xsec_id );
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
        ErrorMgr.AddError( VSP_INVALID_PTR, "GetXSecParm::Can't Find XSecCurve " + xsec_id );
        return string();
    }

    string xscparm = xsc->FindParm( name );
    if ( ValidParm( xscparm ) )
    {
        ErrorMgr.NoError();
        return xscparm;
    }

    ErrorMgr.AddError( VSP_CANT_FIND_NAME, "GetXSecParm::Can't Find Parm " + name );
    return string();
}

//==== Read XSec From File ====//
vector<vec3d> ReadFileXSec( const string& xsec_id, const string& file_name )
{
    vector< vec3d > pnt_vec;
    XSec* xs = FindXSec( xsec_id );
    if ( !xs )
    {
        ErrorMgr.AddError( VSP_INVALID_PTR, "ReadFileXSec::Can't Find XSec " + xsec_id );
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
            ErrorMgr.AddError( VSP_FILE_DOES_NOT_EXIST, "ReadFileXSec::Error reading fuselage file " + file_name );
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
        ErrorMgr.AddError( VSP_INVALID_PTR, "SetXSecPnts::Can't Find XSec " + xsec_id );
        return;
    }
    if ( xs->GetXSecCurve()->GetType() != XS_FILE_FUSE )
    {
        ErrorMgr.AddError( VSP_WRONG_XSEC_TYPE, "SetXSecPnts::Wrong XSec Type" );
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
        ErrorMgr.AddError( VSP_INVALID_PTR, "ComputePnt::Can't Find XSec " + xsec_id );
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
        ErrorMgr.AddError( VSP_INVALID_PTR, "ComputeTan::Can't Find XSec " + xsec_id );
        return vec3d();
    }

    vec3d pnt = xs->GetCurve().CompTan01( fract );
    ErrorMgr.NoError();

    return pnt;
}

//==== Reset All XSec Skinning Parms ====//
void ResetXSecSkinParms( const string& xsec_id )
{
    XSec* xs = FindXSec( xsec_id );
    if ( !xs )
    {
        ErrorMgr.AddError( VSP_INVALID_PTR, "ResetXSecSkinParms::Can't Find XSec " + xsec_id );
        return;
    }
    SkinXSec* skinxs = dynamic_cast<SkinXSec*>(xs);
    if ( !skinxs )
    {
        ErrorMgr.AddError( VSP_INVALID_PTR, "ResetXSecSkinParms::Can't Convert To Skin XSec " + xsec_id );
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
        ErrorMgr.AddError( VSP_INVALID_PTR, "SetXSecContinuity::Can't Find XSec " + xsec_id );
        return;
    }
    SkinXSec* skinxs = dynamic_cast<SkinXSec*>(xs);
    if ( !skinxs )
    {
        ErrorMgr.AddError( VSP_INVALID_PTR, "SetXSecContinuity::Can't Convert To Skin XSec " + xsec_id );
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
        ErrorMgr.AddError( VSP_INVALID_PTR, "SetXSecTanAngles::Can't Find XSec " + xsec_id );
        return;
    }
    SkinXSec* skinxs = dynamic_cast<SkinXSec*>(xs);
    if ( !skinxs )
    {
        ErrorMgr.AddError( VSP_INVALID_PTR, "SetXSecTanAngles::Can't Convert To Skin XSec " + xsec_id );
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
        ErrorMgr.AddError( VSP_INVALID_PTR, "SetXSecTanSlews::Can't Find XSec " + xsec_id );
        return;
    }
    SkinXSec* skinxs = dynamic_cast<SkinXSec*>(xs);
    if ( !skinxs )
    {
        ErrorMgr.AddError( VSP_INVALID_PTR, "SetXSecTanSlews::Can't Convert To Skin XSec " + xsec_id );
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
        ErrorMgr.AddError( VSP_INVALID_PTR, "SetXSecTanStrengths::Can't Find XSec " + xsec_id );
        return;
    }
    SkinXSec* skinxs = dynamic_cast<SkinXSec*>(xs);
    if ( !skinxs )
    {
        ErrorMgr.AddError( VSP_INVALID_PTR, "SetXSecTanStrengths::Can't Convert To Skin XSec " + xsec_id );
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
        ErrorMgr.AddError( VSP_INVALID_PTR, "SetXSecCurvatures::Can't Find XSec " + xsec_id );
        return;
    }
    SkinXSec* skinxs = dynamic_cast<SkinXSec*>(xs);
    if ( !skinxs )
    {
        ErrorMgr.AddError( VSP_INVALID_PTR, "SetXSecCurvatures::Can't Convert To Skin XSec " + xsec_id );
        return;
    }

    skinxs->SetCurvatures( side, top, right, bottom, left );
    ErrorMgr.NoError();
}

void ReadFileAirfoil( const string& xsec_id, const string& file_name )
{
    XSec* xs = FindXSec( xsec_id );
    if ( !xs )
    {
        ErrorMgr.AddError( VSP_INVALID_PTR, "ReadFileAirfoil::Can't Find XSec " + xsec_id );
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
            ErrorMgr.AddError( VSP_FILE_DOES_NOT_EXIST, "ReadFileAirfoil::Error reading airfoil file " + file_name );
            return;
        }
    }

    ErrorMgr.AddError( VSP_WRONG_XSEC_TYPE, "ReadFileAirfoil::XSec Not XS_FILE_AIRFOIL Type " + xsec_id );
    return;
}

void SetAirfoilUpperPnts( const string& xsec_id, const std::vector< vec3d > & up_pnt_vec )
{
    XSec* xs = FindXSec( xsec_id );
    if ( !xs )
    {
        ErrorMgr.AddError( VSP_INVALID_PTR, "SetAirfoilUpperPnts::Can't Find XSec " + xsec_id );
        return;
    }

    if ( xs->GetXSecCurve()->GetType() != XS_FILE_AIRFOIL )
    {
        ErrorMgr.AddError( VSP_WRONG_XSEC_TYPE, "SetAirfoilUpperPnts::XSec Not XS_FILE_AIRFOIL Type" );
        return;
    }

    FileAirfoil* file_xs = dynamic_cast<FileAirfoil*>( xs->GetXSecCurve() );
    assert( file_xs );
    file_xs->SetAirfoilUpperPnts( up_pnt_vec );
    ErrorMgr.NoError();
}

void SetAirfoilLowerPnts( const string& xsec_id, const std::vector< vec3d > & low_pnt_vec )
{
    XSec* xs = FindXSec( xsec_id );
    if ( !xs )
    {
        ErrorMgr.AddError( VSP_INVALID_PTR, "SetAirfoilLowerPnts::Can't Find XSec " + xsec_id );
        return;
    }

    if ( xs->GetXSecCurve()->GetType() != XS_FILE_AIRFOIL )
    {
        ErrorMgr.AddError( VSP_WRONG_XSEC_TYPE, "SetAirfoilLowerPnts::XSec Not XS_FILE_AIRFOIL Type" );
        return;
    }

    FileAirfoil* file_xs = dynamic_cast<FileAirfoil*>( xs->GetXSecCurve() );
    assert( file_xs );
    file_xs->SetAirfoilLowerPnts( low_pnt_vec );
    ErrorMgr.NoError();
}

void SetAirfoilPnts( const string& xsec_id, const std::vector< vec3d > & up_pnt_vec, const std::vector< vec3d > & low_pnt_vec )
{
    XSec* xs = FindXSec( xsec_id );
    if ( !xs )
    {
        ErrorMgr.AddError( VSP_INVALID_PTR, "SetAirfoilPnts::Can't Find XSec " + xsec_id );
        return;
    }

    if ( xs->GetXSecCurve()->GetType() != XS_FILE_AIRFOIL )
    {
        ErrorMgr.AddError( VSP_WRONG_XSEC_TYPE, "SetAirfoilPnts::XSec Not XS_FILE_AIRFOIL Type" );
        return;
    }

    FileAirfoil* file_xs = dynamic_cast<FileAirfoil*>( xs->GetXSecCurve() );
    assert( file_xs );
    file_xs->SetAirfoilPnts( up_pnt_vec, low_pnt_vec );
    ErrorMgr.NoError();
}

void WriteSeligAirfoilFile( const std::string & airfoil_name, const std::vector<vec3d> & ordered_airfoil_pnts )
{
    // Note, the input airfoil coordinate points must be ordered in the correct Selig format: Start at X = 1, proceed
    //  along the top of the airfoil to x = 0.0 at the leading edge, and return to X = 1 along the bottom surface

    //==== Open file ====//
    string file_name = airfoil_name + ".dat";
    FILE* af = fopen( file_name.c_str(), "w" );
    if ( !af )
    {
        ErrorMgr.AddError( VSP_FILE_WRITE_FAILURE, "WriteSeligAirfoilFile::Error writing airfoil file " + airfoil_name );
        return;
    }

    string header = airfoil_name + " AIRFOIL\n";
    fprintf( af, "%s", header.c_str() );

    char buff[256];

    for ( size_t i = 0; i < ordered_airfoil_pnts.size(); i++ )
    {
        snprintf( buff, sizeof( buff ), " %7.6f     %7.6f\n", ordered_airfoil_pnts[i].x(), ordered_airfoil_pnts[i].y() );
        fprintf( af, "%s", buff );
    }

    fclose( af );
    ErrorMgr.NoError();
}

struct LLT_Data // Struct containing Lifting Line Theory data
{
    vector < long double > y_span_vec; // y position across half span
    vector < long double > gamma_vec; // circulation
    vector < long double > w_vec; // downwash velocity
    vector < long double > cl_vec; // lift coefficient
    vector < long double > cd_vec; // induced drag coefficient
};

LLT_Data GetHersheyLLTData( const unsigned int &npts, const long double &alpha, const long double &Vinf, const long double &span )
{
    LLT_Data llt_data;

    const long double alpha0 = 0.0; // zero lift angle of attack (rad)
    const long double c = 1.0; // root/tip chord

    vector < long double > theta_vec, r_vec, a_vec;
    vector < int > odd_vec;
    theta_vec.resize( npts );
    odd_vec.resize( npts );
    r_vec.resize( npts );
    a_vec.resize( npts );

    llt_data.y_span_vec.resize( npts );
    llt_data.gamma_vec.resize( npts );
    llt_data.w_vec.resize( npts );
    llt_data.cl_vec.resize( npts );
    llt_data.cd_vec.resize( npts );

    Eigen::Matrix<long double, Eigen::Dynamic, Eigen::Dynamic> c_mat;
    c_mat.resize( npts, npts );

    for ( size_t i = 0; i < npts; i++ )
    {
        theta_vec[i] = ( (double)i + 1.0l ) * ( ( M_PI / 2.0l ) / ( (double)npts ) ); // [0 to pi/2]
        llt_data.y_span_vec[i] = cos( theta_vec[i] ) * ( span / 2.0l ); // [tip to root]
        odd_vec[i] = 2 * i + 1;
        r_vec[i] = M_PI * c / 4.0l / ( span / 2.0l ) * ( alpha - alpha0 ) * sin( theta_vec[i] );
    }

    for ( size_t i = 0; i < npts; i++ )
    {
        for ( size_t j = 0; j < npts; j++ )
        {
            c_mat( i, j ) = sin( theta_vec[j] * (double)odd_vec[i] ) * ( M_PI * c * odd_vec[i] / 4.0l / ( span / 2.0l ) + sin( theta_vec[j] ) );
        }
    }

    // Invert the matrix
    c_mat = c_mat.inverse();

    // Matrix multiplication: [N,N]x[N,1]
    for ( size_t i = 0; i < npts; i++ )
    {
        for ( size_t j = 0; j < npts; j++ )
        {
            a_vec[i] += c_mat( j, i ) * r_vec[j];
        }
    }

    // Matrix multiplication: [N,N]x[N,1]
    for ( size_t i = 0; i < npts; i++ )
    {
        for ( size_t j = 0; j < npts; j++ )
        {
            llt_data.gamma_vec[i] += 4.0l * Vinf * ( span / 2.0l ) * sin( theta_vec[i] * (double)odd_vec[j] ) * a_vec[j];
            llt_data.w_vec[i] += Vinf * ( span / 2.0l ) * (double)odd_vec[j] * a_vec[j] * sin( theta_vec[i] * (double)odd_vec[j] ) / sin( theta_vec[i] );
        }

        llt_data.cl_vec[i] = 2.0l * llt_data.gamma_vec[i] / Vinf;
        llt_data.cd_vec[i] = 2.0l * llt_data.w_vec[i] * llt_data.gamma_vec[i] / ( c * ( span / 2.0l ) * pow( Vinf, 2.0 ) );
    }

    return llt_data;
}

std::vector<vec3d> GetHersheyBarLiftDist( const int &npts, const double &alpha, const double &Vinf, const double &span, bool full_span_flag )
{
    // Calculation of lift distribution for a Hershey Bar wing with unit chord length using Glauert's Method
    //  Input span is the entire wing span, which half is used in the following calculations. If full_span_flag == true,
    //  symmetry is applied to the results. Input alpha must be in radians.

    LLT_Data llt_data = GetHersheyLLTData( npts, alpha, Vinf, span );

    vector < vec3d > y_cl_vec;
    if ( full_span_flag )
    {
        y_cl_vec.resize( 2 * npts );

        for ( size_t i = 0; i < npts; i++ )
        {
            y_cl_vec[i] = vec3d( -1 * llt_data.y_span_vec[i], llt_data.cl_vec[i], 0.0 );
        }

        for ( size_t i = 0; i < npts; i++ )
        {
            y_cl_vec[( 2 * npts - 1 ) - i] = vec3d( llt_data.y_span_vec[i], llt_data.cl_vec[i], 0.0 ); // Apply symmetry
        }
    }
    else
    {
        y_cl_vec.resize( npts );

        for ( size_t i = 0; i < npts; i++ )
        {
            y_cl_vec[i] = vec3d( llt_data.y_span_vec[i], llt_data.cl_vec[i], 0.0 );
        }

        std::reverse( y_cl_vec.begin(), y_cl_vec.end() );
    }


    return y_cl_vec;
}

std::vector<vec3d> GetHersheyBarDragDist( const int &npts, const double &alpha, const double &Vinf, const double &span, bool full_span_flag )
{
    // Calculation of drag distribution for a Hershey Bar wing with unit chord length using Glauert's Method.
    //  Input span is the entire wing span, which half is used in the following calculations. If full_span_flag == true,
    //  symmetry is applied to the results. Input alpha must be in radians.

    LLT_Data llt_data = GetHersheyLLTData( npts, alpha, Vinf, span );

    vector < vec3d > y_cd_vec;
    if ( full_span_flag )
    {
        y_cd_vec.resize( 2 * npts );

        for ( size_t i = 0; i < npts; i++ )
        {
            y_cd_vec[i] = vec3d( -1 * llt_data.y_span_vec[i], llt_data.cd_vec[i], 0.0 );
        }

        for ( size_t i = 0; i < npts; i++ )
        {
            y_cd_vec[( 2 * npts - 1 ) - i] = vec3d( llt_data.y_span_vec[i], llt_data.cd_vec[i], 0.0 ); // Apply symmetry
        }
    }
    else
    {
        y_cd_vec.resize( npts );

        for ( size_t i = 0; i < npts; i++ )
        {
            y_cd_vec[i] = vec3d( llt_data.y_span_vec[i], llt_data.cd_vec[i], 0.0 );
        }

        std::reverse( y_cd_vec.begin(), y_cd_vec.end() );
    }

    return y_cd_vec;
}

std::vector<vec3d> GetVKTAirfoilPnts( const int &npts, const double &alpha, const double &epsilon, const double &kappa, const double &tau )
{
    // alpha = Angle of attack( radian )
    // epsilon = Thickness
    // kappa = Camber
    // tau = Trailing edge angle( radian )
    // npts = # of nodes in the circumferential direction

    const double ell = 0.25; // chord length = 4 * ell

    vector < vec3d > xyzdata;
    xyzdata.resize( npts );

    double a = ell * sqrt( ( 1.0 + epsilon ) * ( 1.0 + epsilon ) + kappa * kappa ); // Radius of circle
    double beta = asin( ell * kappa / a ); // Angle of TE location (rad)
    double n = 2.0 - tau / M_PI;
    doublec mu = doublec( -ell * epsilon, ell * kappa ); // Center of circle

    if ( ( ell * kappa / a ) > 1.0 )
    {
        ErrorMgr.AddError( VSP_INVALID_INPUT_VAL, "GetVKTAirfoilPnts: Camber parameter, kappa, is too large" );
        return xyzdata;
    }

    int ile = 0;
    double dmax = -1.0;
    // Evaluate points and track furthest from TE as surrogate for LE.
    // Would be better to identify LE as tightest curvature or similar.
    for ( size_t p = 0; p < npts; p++ )
    {
        // Clockwise from TE
        double theta = 2.0 * M_PI * ( 1.0 - p * 1.0 / ( npts - 1 ) ); // rad

        double xi = a * cos( theta - beta ) + mu.real();
        double eta = a * sin( theta - beta ) + mu.imag();
        doublec zeta = doublec( xi, eta );

        // Karman-Trefftz transformation
        doublec temp = pow( zeta - ell, n ) / pow( zeta + ell, n );
        doublec z = n * ell * ( 1.0 + temp ) / ( 1.0 - temp );
        xyzdata[p].set_xyz( z.real(), z.imag(), 0.0 );

        // Find point furthest from TE.  Declare that the LE.
        double d = dist( xyzdata[p], xyzdata[0] );
        if ( d > dmax )
        {
            dmax = d;
            ile = p;
        }
    }

    xyzdata[npts - 1] = xyzdata[0]; // Ensure closure

    // Shift and scale airfoil such that xle=0 and xte=1.
    double scale = xyzdata[0].x() - xyzdata[ile].x();
    double xshift = xyzdata[ile].x();

    for ( size_t j = 0; j < npts; j++ )
    {
        xyzdata[j].offset_x( -1 * xshift );
        xyzdata[j] = xyzdata[j] / scale;
    }

    return xyzdata;
}

std::vector<double> GetVKTAirfoilCpDist( const double &alpha, const double &epsilon, const double &kappa, const double &tau, const std::vector<vec3d> &xyzdata )
{
    // alpha = Angle of attack( radian )
    // epsilon = Thickness
    // kappa = Camber
    // tau = Trailing edge angle( radian )
    // xyzdata = output from vsp::GetVKTAirfoilPnts

    doublec i( 0, 1 );
    const double ell = 0.25; // chord length = 4 * ell

    const unsigned int npts = xyzdata.size();

    vector < double > cpdata;
    cpdata.resize( npts );

    double a = ell * sqrt( ( 1.0 + epsilon ) * ( 1.0 + epsilon ) + kappa * kappa ); // Radius of circle
    double beta = asin( ell * kappa / a ); // Angle of TE location (rad)
    double n = 2.0 - tau / M_PI;
    doublec mu = doublec( -ell * epsilon, ell * kappa ); // Center of circle

    if ( ( ell * kappa / a ) > 1.0 )
    {
        ErrorMgr.AddError( VSP_INVALID_INPUT_VAL, "GetVKTAirfoilCpDist: Camber parameter, kappa, is too large" );
        return cpdata;
    }

    int ile = 0;
    double dmax = -1.0;
    // Evaluate points and track furthest from TE as surrogate for LE.
    // Would be better to identify LE as tightest curvature or similar.
    for ( size_t p = 0; p < npts; p++ )
    {
        // Clockwise from TE
        double theta = 2.0 * M_PI * ( 1.0 - p * 1.0 / ( npts - 1 ) ); // rad

        double xi = a * cos( theta - beta ) + mu.real();
        double eta = a * sin( theta - beta ) + mu.imag();
        doublec zeta = doublec( xi, eta );

        // w(zeta): Complex velocity in the circle plane (a flow around a cylinder)
        doublec w = cmplx_velocity( zeta, alpha, beta, a, mu );

        // Compute the velocity in the airfoil plane : ( u, v ) = w / ( dZ / dzeta )
        // Derivative of the Karman - Trefftz transformation:
        doublec dzdzeta = derivative( zeta, ell, n );

        double u, v;

        if ( std::abs( theta ) <= FLT_EPSILON || std::abs( theta - 2.0 * M_PI ) <= FLT_EPSILON ) // Special treatment at the trailing edge (theta = 0.0 or 2*pi)
        {
            if ( std::abs( tau ) <= FLT_EPSILON ) // Joukowski airfoil (cusped trailing edge: tau = 0.0 )
            {
                doublec uv = ( ell / a ) * exp( 2.0 * i * beta ) * cos( alpha + beta );
                u = uv.real();
                v = -1 * uv.imag();
            }
            else // Karman-Trefftz airfoil (finite angle: tau > 0.0), TE must be a stagnation point.
            {
                u = 0.0;
                v = 0.0;
            }
        }
        else
        {
            doublec uv = w / dzdzeta;
            u = uv.real();
            v = -1 * uv.imag();
        }

        cpdata[p] = 1.0 - ( pow( u, 2.0 ) + ( pow( v, 2.0 ) ) );
    }

    return cpdata;
}

std::vector<vec3d> GetEllipsoidSurfPnts( const vec3d &center, const vec3d &abc_rad, int u_npts, int w_npts )
{
    // Generate the surface points for a ellipsoid of input abc radius vector at center. Based on the Matlab function ellipsoid.m
    if ( u_npts < 20 )
    {
        u_npts = 20;
    }
    if ( w_npts < 20 )
    {
        w_npts = 20;
    }

    vector < vec3d > surf_pnt_vec;

    vector < double > theta_vec, phi_vec;
    theta_vec.resize( u_npts );
    phi_vec.resize( w_npts );

    theta_vec[0] = 0.0; // theta: [0,2PI]
    phi_vec[0] = 0.0; // phi: [0,M_PI]

    const double theta_step = 2 * M_PI / ( u_npts - 1 );
    const double phi_step = M_PI / ( w_npts - 1 );

    for ( size_t i = 1; i < u_npts; i++ )
    {
        theta_vec[i] = theta_vec[i - 1] + theta_step;
    }

    for ( size_t i = 1; i < w_npts; i++ )
    {
        phi_vec[i] = phi_vec[i - 1] + phi_step;
    }

    for ( size_t u = 0; u < u_npts; u++ )
    {
        for ( size_t w = 0; w < w_npts; w++ )
        {
            surf_pnt_vec.push_back( vec3d( ( abc_rad.x() * cos( theta_vec[u] ) * sin( phi_vec[w] ) + center.x() ),
                ( abc_rad.y() * sin( theta_vec[u] ) * sin( phi_vec[w] ) + center.y() ),
                ( abc_rad.z() * cos( phi_vec[w] ) + center.z() ) ) );
        }
    }

    return surf_pnt_vec;
}

std::vector<vec3d> GetFeatureLinePnts( const string& geom_id )
{
    vector < vec3d > pnt_vec;

    Vehicle* veh = GetVehicle();
    Geom* geom_ptr = veh->FindGeom( geom_id );
    if ( !geom_ptr )
    {
        ErrorMgr.AddError( VSP_INVALID_PTR, "GetFeatureLinePnts::Can't Find Geom " + geom_id );
        return pnt_vec;
    }

    int num_surf = geom_ptr->GetNumTotalSurfs();

    double tol = 1e-2;

    for ( size_t i = 0; i < num_surf; i++ )
    {
        const VspSurf *surf = geom_ptr->GetSurfPtr( i );
        // U feature lines
        for ( int j = 0; j < surf->GetNumUFeature(); j++ )
        {
            vector < vec3d > ptline;
            surf->TessUFeatureLine( j, ptline, tol );

            for ( size_t k = 0; k < ptline.size(); k++ )
            {
                pnt_vec.push_back( ptline[k] );
            }
        }

        // V feature lines
        for ( int j = 0; j < surf->GetNumWFeature(); j++ )
        {
            vector < vec3d > ptline;
            surf->TessWFeatureLine( j, ptline, tol );

            for ( size_t k = 0; k < ptline.size(); k++ )
            {
                pnt_vec.push_back( ptline[k] );
            }
        }
    }

    ErrorMgr.NoError();
    return pnt_vec;
}

std::vector <double> GetEllipsoidCpDist( const std::vector<vec3d> &surf_pnt_vec, const vec3d &abc_rad, const vec3d &V_inf )
{
    // Generate Analytical Solution for Potential Flow at input ellipsoid surface points for input velocity vector (V).
    //  Based on Munk, M. M., 'Remarks on the Pressure Distribution over the Surface of an Ellipsoid, Moving Translationally
    //  Through a Perfect Fluid,' NACA TN-196, June 1924.

    double alpha = abc_rad.x() * abc_rad.y() * abc_rad.z() * IntegrateEllipsoidFlow( abc_rad, 0 );
    double beta = abc_rad.x() * abc_rad.y() * abc_rad.z() * IntegrateEllipsoidFlow( abc_rad, 1 );
    double gamma = abc_rad.x() * abc_rad.y() * abc_rad.z() * IntegrateEllipsoidFlow( abc_rad, 2 );

    double k1 = alpha / ( 2.0 - alpha );
    double k2 = beta / ( 2.0 - beta );
    double k3 = gamma / ( 2.0 - gamma );

    double A = k1 + 1;
    double B = k2 + 1;
    double C = k3 + 1;

    vector < vec3d > pot_vec, uvw_vec;
    vector < double > cp_vec;
    pot_vec.resize( surf_pnt_vec.size() );
    uvw_vec.resize( surf_pnt_vec.size() );
    cp_vec.resize( surf_pnt_vec.size() );

    double Vmax_x = A * V_inf.x();
    double Vmax_y = B * V_inf.y();
    double Vmax_z = C * V_inf.z();

    for ( size_t i = 0; i < surf_pnt_vec.size(); i++ )
    {
        // Velocity potential
        pot_vec[i] = vec3d( ( Vmax_x * surf_pnt_vec[i].x() ), ( Vmax_y * surf_pnt_vec[i].y() ), ( Vmax_z * surf_pnt_vec[i].z() ) );

        // Normal vector
        vec3d norm( ( 2.0 * surf_pnt_vec[i].x() / pow( abc_rad.x(), 2.0 ) ),
            ( 2.0 * surf_pnt_vec[i].y() / pow( abc_rad.y(), 2.0 ) ),
            ( 2.0 * surf_pnt_vec[i].z() / pow( abc_rad.z(), 2.0 ) ) );

        norm.normalize();

        // Vmax component in panel normal direction
        double Vnorm = Vmax_x * norm.x() + Vmax_y * norm.y() + Vmax_z * norm.z();

        // Surface velocity
        uvw_vec[i] = vec3d( ( Vmax_x - Vnorm * norm.x() ), ( Vmax_y - Vnorm * norm.y() ), ( Vmax_z - Vnorm * norm.z() ) );

        // Pressure Coefficient
        cp_vec[i] = 1.0 - pow( ( uvw_vec[i].mag() / V_inf.mag() ), 2.0 );
    }

    return cp_vec;
}

struct ellipsoid_flow_functor
{
    double operator()( const double &t )
    {
        return ( 1.0 / ( ( pow( abc_rad[abc_index], 2.0 ) + t ) * sqrt( ( pow( abc_rad.x(), 2.0 ) + t ) * ( pow( abc_rad.y(), 2.0 ) + t ) * ( pow( abc_rad.z(), 2.0 ) + t ) ) ) );
    }
    vec3d abc_rad;
    int abc_index; // a: 0, b: 1, c: 2
};

double IntegrateEllipsoidFlow( const vec3d &abc_rad, const int &abc_index )
{
    // Integration of Equations 6 and 7 for alpha, beta, and gamma in "Hydrodynamics" by Horace Lamb, Ch.5, Section 111, pg. 162.
    //  abc_index corresponds to a:0 for alpha, b:1 for beta, and c:2 for gamma
    ellipsoid_flow_functor fun;
    fun.abc_rad = abc_rad;
    fun.abc_index = abc_index;

    eli::mutil::quad::simpson< double > quad;

    return quad( fun, 0.0, 1.0e8 ); // Integrate from 0 to inf (Note: an upper limit greater than 1.0e8 will produce errors)
}

std::vector<vec3d> GetAirfoilUpperPnts( const string& xsec_id )
{
    vector< vec3d > pnt_vec;

    XSec* xs = FindXSec( xsec_id );
    if ( !xs )
    {
        ErrorMgr.AddError( VSP_INVALID_PTR, "GetAirfoilUpperPnts::Can't Find XSec " + xsec_id );
        return pnt_vec;
    }

    if ( xs->GetXSecCurve()->GetType() != XS_FILE_AIRFOIL )
    {
        ErrorMgr.AddError( VSP_WRONG_XSEC_TYPE, "GetAirfoilUpperPnts::XSec Not XS_FILE_AIRFOIL Type" );
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
        ErrorMgr.AddError( VSP_INVALID_PTR, "GetAirfoilLowerPnts::Can't Find XSec " + xsec_id );
        return pnt_vec;
    }

    if ( xs->GetXSecCurve()->GetType() != XS_FILE_AIRFOIL )
    {
        ErrorMgr.AddError( VSP_WRONG_XSEC_TYPE, "GetAirfoilLowerPnts::XSec Not XS_FILE_AIRFOIL Type" );
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
        ErrorMgr.AddError( VSP_INVALID_PTR, "GetUpperCSTCoefs::Can't Find XSec " + xsec_id );
        return ret_vec;
    }

    if ( xs->GetXSecCurve()->GetType() != XS_CST_AIRFOIL )
    {
        ErrorMgr.AddError( VSP_WRONG_XSEC_TYPE, "GetUpperCSTCoefs::XSec Not XS_CST_AIRFOIL Type" );
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
        ErrorMgr.AddError( VSP_INVALID_PTR, "GetLowerCSTCoefs::Can't Find XSec " + xsec_id );
        return ret_vec;
    }

    if ( xs->GetXSecCurve()->GetType() != XS_CST_AIRFOIL )
    {
        ErrorMgr.AddError( VSP_WRONG_XSEC_TYPE, "GetLowerCSTCoefs::XSec Not XS_CST_AIRFOIL Type" );
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
        ErrorMgr.AddError( VSP_INVALID_PTR, "GetUpperCSTDegree::Can't Find XSec " + xsec_id );
        return deg;
    }

    if ( xs->GetXSecCurve()->GetType() != XS_CST_AIRFOIL )
    {
        ErrorMgr.AddError( VSP_WRONG_XSEC_TYPE, "GetUpperCSTDegree::XSec Not XS_CST_AIRFOIL Type" );
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
        ErrorMgr.AddError( VSP_INVALID_PTR, "GetLowerCSTDegree::Can't Find XSec " + xsec_id );
        return deg;
    }

    if ( xs->GetXSecCurve()->GetType() != XS_CST_AIRFOIL )
    {
        ErrorMgr.AddError( VSP_WRONG_XSEC_TYPE, "GetLowerCSTDegree::XSec Not XS_CST_AIRFOIL Type" );
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
        ErrorMgr.AddError( VSP_INVALID_PTR, "SetUpperCST::Can't Find XSec " + xsec_id );
        return;
    }

    if ( xs->GetXSecCurve()->GetType() != XS_CST_AIRFOIL )
    {
        ErrorMgr.AddError( VSP_WRONG_XSEC_TYPE, "SetUpperCST::XSec Not XS_CST_AIRFOIL Type" );
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
        ErrorMgr.AddError( VSP_INVALID_PTR, "SetLowerCST::Can't Find XSec " + xsec_id );
        return;
    }

    if ( xs->GetXSecCurve()->GetType() != XS_CST_AIRFOIL )
    {
        ErrorMgr.AddError( VSP_WRONG_XSEC_TYPE, "SetLowerCST::XSec Not XS_CST_AIRFOIL Type" );
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
        ErrorMgr.AddError( VSP_INVALID_PTR, "PromoteCSTUpper::Can't Find XSec " + xsec_id );
        return;
    }

    if ( xs->GetXSecCurve()->GetType() != XS_CST_AIRFOIL )
    {
        ErrorMgr.AddError( VSP_WRONG_XSEC_TYPE, "PromoteCSTUpper::XSec Not XS_CST_AIRFOIL Type" );
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
        ErrorMgr.AddError( VSP_INVALID_PTR, "PromoteCSTLower::Can't Find XSec " + xsec_id );
        return;
    }

    if ( xs->GetXSecCurve()->GetType() != XS_CST_AIRFOIL )
    {
        ErrorMgr.AddError( VSP_WRONG_XSEC_TYPE, "PromoteCSTLower::XSec Not XS_CST_AIRFOIL Type" );
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
        ErrorMgr.AddError( VSP_INVALID_PTR, "DemoteCSTUpper::Can't Find XSec " + xsec_id );
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
        ErrorMgr.AddError( VSP_INVALID_PTR, "DemoteCSTLower::Can't Find XSec " + xsec_id );
        return;
    }

    if ( xs->GetXSecCurve()->GetType() != XS_CST_AIRFOIL )
    {
        ErrorMgr.AddError( VSP_WRONG_XSEC_TYPE, "DemoteCSTLower::XSec Not XS_CST_AIRFOIL Type" );
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
        ErrorMgr.AddError( VSP_INVALID_PTR, "FitAfCST::Can't Find XSecSurf " + xsec_surf_id );
        return;
    }
    XSec* xsec = xsec_surf->FindXSec( xsec_index );
    if ( !xsec )
    {
        ErrorMgr.AddError( VSP_INVALID_PTR, "FitAfCST::Can't Find XSec " + xsec_surf_id + ":" + to_string( ( long long )xsec_index ) );
        return;
    }

    if ( ( xsec->GetXSecCurve()->GetType() != XS_FOUR_SERIES ) &&
         ( xsec->GetXSecCurve()->GetType() != XS_SIX_SERIES ) &&
         ( xsec->GetXSecCurve()->GetType() != XS_FOUR_DIGIT_MOD ) &&
         ( xsec->GetXSecCurve()->GetType() != XS_FIVE_DIGIT ) &&
         ( xsec->GetXSecCurve()->GetType() != XS_FIVE_DIGIT_MOD ) &&
         ( xsec->GetXSecCurve()->GetType() != XS_ONE_SIX_SERIES ) &&
         ( xsec->GetXSecCurve()->GetType() != XS_FILE_AIRFOIL ) )
    {
        ErrorMgr.AddError( VSP_WRONG_XSEC_TYPE, "FitAfCST::XSec Not Fittable Airfoil Type" );
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
        ErrorMgr.AddError( VSP_INVALID_PTR, "FitAfCST::Can't Find New XSec " + xsec_surf_id + ":" + to_string( ( long long )xsec_index ) );
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
        ErrorMgr.AddError( VSP_WRONG_XSEC_TYPE, "FitAfCST::XSec Not XS_CST_AIRFOIL Type" );
        return;
    }

    CSTAirfoil* cst_xs = dynamic_cast< CSTAirfoil* >( newxsc );

    assert( cst_xs );
    cst_xs->FitCurve( c, deg );

    ErrorMgr.NoError();
}

void WriteBezierAirfoil( const std::string & file_name, const std::string & geom_id, const double &foilsurf_u )
{
    Vehicle* veh = GetVehicle();
    Geom* geom_ptr = veh->FindGeom( geom_id );
    if ( !geom_ptr )
    {
        ErrorMgr.AddError( VSP_INVALID_PTR, "WriteBezierAirfoil::Can't Find Geom " + geom_id );
        return;
    }

    if ( foilsurf_u < 0.0 || foilsurf_u > 1.0 )
    {
        ErrorMgr.AddError( VSP_INVALID_INPUT_VAL, "WriteBezierAirfoil::Invalid u Location " + to_string( foilsurf_u ) + " - Must be range [0,1]." );
        return;
    }

    geom_ptr->WriteBezierAirfoil( file_name, foilsurf_u );
    ErrorMgr.NoError();
}

void WriteSeligAirfoil( const std::string & file_name, const std::string & geom_id, const double &foilsurf_u )
{
    Vehicle* veh = GetVehicle();
    Geom* geom_ptr = veh->FindGeom( geom_id );
    if ( !geom_ptr )
    {
        ErrorMgr.AddError( VSP_INVALID_PTR, "WriteSeligAirfoil::Can't Find Geom " + geom_id );
        return;
    }

    if ( foilsurf_u < 0.0 || foilsurf_u > 1.0 )
    {
        ErrorMgr.AddError( VSP_INVALID_INPUT_VAL, "WriteSeligAirfoil::Invalid u Location " + to_string( foilsurf_u ) + " - Must be range [0,1]." );
        return;
    }

    geom_ptr->WriteSeligAirfoil( file_name, foilsurf_u );
    ErrorMgr.NoError();
}

vector < vec3d > GetAirfoilCoordinates( const std::string & geom_id, const double &foilsurf_u )
{
    vector < vec3d > ordered_vec;
    Vehicle* veh = GetVehicle();
    Geom* geom_ptr = veh->FindGeom( geom_id );
    if ( !geom_ptr )
    {
        ErrorMgr.AddError( VSP_INVALID_PTR, "GetAirfoilCoordinates::Can't Find Geom " + geom_id );
        return ordered_vec;
    }

    if ( foilsurf_u < 0.0 || foilsurf_u > 1.0 )
    {
        ErrorMgr.AddError( VSP_INVALID_INPUT_VAL, "GetAirfoilCoordinates::Invalid u Location " + to_string( foilsurf_u ) + " - Must be range [0,1]." );
        return ordered_vec;
    }

    ordered_vec = geom_ptr->GetAirfoilCoordinates( foilsurf_u );
    ErrorMgr.NoError();
    return ordered_vec;
}

//===================================================================//
//==============      Background3D Functions        =================//
//===================================================================//

string AddBackground3D()
{
    Background3D * bg = Background3DMgr.CreateAndAddBackground3D();

    if ( !bg )
    {
        ErrorMgr.AddError( VSP_COULD_NOT_CREATE_BACKGROUND3D, "AddBackground3D::Could not create Background3D" );
        return "NONE";
    }

    ErrorMgr.NoError();
    return bg->GetID();
}

int GetNumBackground3Ds()
{
    ErrorMgr.NoError();
    return Background3DMgr.GetNumBackground3Ds();
}

vector < string > GetAllBackground3Ds()
{
    ErrorMgr.NoError();
    return Background3DMgr.GetAllBackground3Ds();
}

void ShowAllBackground3Ds()
{
    ErrorMgr.NoError();
    Background3DMgr.ShowAllBackground3Ds();
}

void HideAllBackground3Ds()
{
    ErrorMgr.NoError();
    Background3DMgr.HideAllBackground3Ds();
}

void DelAllBackground3Ds()
{
    ErrorMgr.NoError();
    Background3DMgr.DelAllBackground3Ds();
}

void DelBackground3D( const string &id )
{
    Background3D * bg = Background3DMgr.GetBackground3D( id );

    if ( !bg )
    {
        ErrorMgr.AddError( VSP_INVALID_ID, "DelBackground3D::Could not find Background3D " + id );
        return;
    }

    ErrorMgr.NoError();
    Background3DMgr.DelBackground3D( id );
}

vector < string > GetAllBackground3DRelativePaths()
{
    ErrorMgr.NoError();
    return Background3DMgr.GetAllBackground3DRelativePaths();
}

vector < string > GetAllBackground3DAbsolutePaths()
{
    ErrorMgr.NoError();
    return Background3DMgr.GetAllBackground3DAbsolutePaths();
}

string GetBackground3DRelativePath( const string &id )
{
    Background3D * bg = Background3DMgr.GetBackground3D( id );

    if ( !bg )
    {
        ErrorMgr.AddError( VSP_INVALID_ID, "GetBackground3DRelativePath::Could not find Background3D " + id );
        return "";
    }

    ErrorMgr.NoError();
    return Background3DMgr.GetBackground3DRelativePath( id );
}

string GetBackground3DAbsolutePath( const string &id )
{
    Background3D * bg = Background3DMgr.GetBackground3D( id );

    if ( !bg )
    {
        ErrorMgr.AddError( VSP_INVALID_ID, "GetBackground3DAbsolutePath::Could not find Background3D " + id );
        return "";
    }

    ErrorMgr.NoError();
    return Background3DMgr.GetBackground3DAbsolutePath( id );
}

void SetBackground3DRelativePath( const string &id, const string &fname )
{
    Background3D * bg = Background3DMgr.GetBackground3D( id );

    if ( !bg )
    {
        ErrorMgr.AddError( VSP_INVALID_ID, "SetBackground3DRelativePath::Could not find Background3D " + id );
        return;
    }

    ErrorMgr.NoError();
    Background3DMgr.SetBackground3DRelativePath( id, fname );
}

void SetBackground3DAbsolutePath( const string &id, const string &fname )
{
    Background3D * bg = Background3DMgr.GetBackground3D( id );

    if ( !bg )
    {
        ErrorMgr.AddError( VSP_INVALID_ID, "SetBackground3DAbsolutePath::Could not find Background3D " + id );
        return;
    }

    ErrorMgr.NoError();
    Background3DMgr.SetBackground3DAbsolutePath( id, fname );
}

//===================================================================//
//==============      RoutingGeom Functions        ==================//
//===================================================================//

int GetNumRoutingPts( const string &routing_id )
{
    Vehicle* veh = GetVehicle();
    Geom* geom_ptr = veh->FindGeom( routing_id );
    if ( !geom_ptr )
    {
        ErrorMgr.AddError( VSP_INVALID_PTR, "GetNumRoutingPts::Can't Find Geom " + routing_id );
        return -1;
    }

    RoutingGeom* routing_ptr = dynamic_cast< RoutingGeom* > ( geom_ptr );

    if ( !routing_ptr || geom_ptr->GetType().m_Type != ROUTING_GEOM_TYPE )
    {
        ErrorMgr.AddError( VSP_INVALID_TYPE, "GetNumRoutingPts::Geom " + routing_id + " is not a RoutingGeom" );
        return -1;
    }

    ErrorMgr.NoError();
    return routing_ptr->GetNumPt();
}

string AddRoutingPt( const string &routing_id, const string &geom_id, int surf_index )
{
    string ret_id;

    Vehicle* veh = GetVehicle();
    Geom* rgeom_ptr = veh->FindGeom( routing_id );
    if ( !rgeom_ptr )
    {
        ErrorMgr.AddError( VSP_INVALID_PTR, "AddRoutingPt::Can't Find Geom " + routing_id );
        return ret_id;
    }

    RoutingGeom* routing_ptr = dynamic_cast< RoutingGeom* > ( rgeom_ptr );

    if ( !routing_ptr || rgeom_ptr->GetType().m_Type != ROUTING_GEOM_TYPE )
    {
        ErrorMgr.AddError( VSP_INVALID_TYPE, "AddRoutingPt::Geom " + routing_id + " is not a RoutingGeom" );
        return ret_id;
    }

    Geom* geom_ptr = veh->FindGeom( geom_id );
    if ( !geom_ptr )
    {
        ErrorMgr.AddError( VSP_INVALID_PTR, "AddRoutingPt::Can't Find Geom " + geom_id );
        return ret_id;
    }

    RoutingPoint *rpt = routing_ptr->AddPt();
    if ( !rpt )
    {
        ErrorMgr.AddError( VSP_INVALID_PTR, "AddRoutingPt::Could not add routing point." );
        return ret_id;
    }

    rpt->SetParentID( geom_id );
    rpt->m_SurfIndx.Set( surf_index );

    ret_id = rpt->GetID();

    ErrorMgr.NoError();
    return ret_id;
}

string InsertRoutingPt( const string &routing_id, int index, const string &geom_id, int surf_index )
{
    string ret_id;

    Vehicle* veh = GetVehicle();
    Geom* rgeom_ptr = veh->FindGeom( routing_id );
    if ( !rgeom_ptr )
    {
        ErrorMgr.AddError( VSP_INVALID_PTR, "InsertRoutingPt::Can't Find Geom " + routing_id );
        return ret_id;
    }

    RoutingGeom* routing_ptr = dynamic_cast< RoutingGeom* > ( rgeom_ptr );

    if ( !routing_ptr || rgeom_ptr->GetType().m_Type != ROUTING_GEOM_TYPE )
    {
        ErrorMgr.AddError( VSP_INVALID_TYPE, "InsertRoutingPt::Geom " + routing_id + " is not a RoutingGeom" );
        return ret_id;
    }

    Geom* geom_ptr = veh->FindGeom( geom_id );
    if ( !geom_ptr )
    {
        ErrorMgr.AddError( VSP_INVALID_PTR, "InsertRoutingPt::Can't Find Geom " + geom_id );
        return ret_id;
    }

    if ( index < 0 || index > routing_ptr->GetNumPt() ) // Note index can == size() for Insert
    {
        ErrorMgr.AddError( VSP_INDEX_OUT_RANGE, "InsertRoutingPt::index " + to_string( index ) + " is out of range" );
        return ret_id;
    }

    RoutingPoint *rpt = routing_ptr->InsertPt( index );
    if ( !rpt )
    {
        ErrorMgr.AddError( VSP_INVALID_PTR, "InsertRoutingPt::Could not insert routing point." );
        return ret_id;
    }

    rpt->SetParentID( geom_id );
    rpt->m_SurfIndx.Set( surf_index );

    ret_id = rpt->GetID();

    ErrorMgr.NoError();
    return ret_id;
}

void DelRoutingPt( const string &routing_id, int index )
{
    Vehicle* veh = GetVehicle();
    Geom* rgeom_ptr = veh->FindGeom( routing_id );
    if ( !rgeom_ptr )
    {
        ErrorMgr.AddError( VSP_INVALID_PTR, "DelRoutingPt::Can't Find Geom " + routing_id );
        return;
    }

    RoutingGeom* routing_ptr = dynamic_cast< RoutingGeom* > ( rgeom_ptr );

    if ( !routing_ptr || rgeom_ptr->GetType().m_Type != ROUTING_GEOM_TYPE )
    {
        ErrorMgr.AddError( VSP_INVALID_TYPE, "DelRoutingPt::Geom " + routing_id + " is not a RoutingGeom" );
        return;
    }

    if ( index < 0 || index >= routing_ptr->GetNumPt() )
    {
        ErrorMgr.AddError( VSP_INDEX_OUT_RANGE, "DelRoutingPt::index " + to_string( index ) + " is out of range" );
        return;
    }

    routing_ptr->DelPt( index );

    ErrorMgr.NoError();
    return;
}

void DelAllRoutingPt( const string &routing_id )
{
    Vehicle* veh = GetVehicle();
    Geom* rgeom_ptr = veh->FindGeom( routing_id );
    if ( !rgeom_ptr )
    {
        ErrorMgr.AddError( VSP_INVALID_PTR, "DelAllRoutingPt::Can't Find Geom " + routing_id );
        return;
    }

    RoutingGeom* routing_ptr = dynamic_cast< RoutingGeom* > ( rgeom_ptr );

    if ( !routing_ptr || rgeom_ptr->GetType().m_Type != ROUTING_GEOM_TYPE )
    {
        ErrorMgr.AddError( VSP_INVALID_TYPE, "DelAllRoutingPt::Geom " + routing_id + " is not a RoutingGeom" );
        return;
    }

    routing_ptr->DelAllPt();

    ErrorMgr.NoError();
    return;
}

int MoveRoutingPt( const string &routing_id, int index, int reorder_type )
{
    int newindex = index;
    Vehicle* veh = GetVehicle();
    Geom* rgeom_ptr = veh->FindGeom( routing_id );
    if ( !rgeom_ptr )
    {
        ErrorMgr.AddError( VSP_INVALID_PTR, "MoveRoutingPt::Can't Find Geom " + routing_id );
        return newindex;
    }

    RoutingGeom* routing_ptr = dynamic_cast< RoutingGeom* > ( rgeom_ptr );

    if ( !routing_ptr || rgeom_ptr->GetType().m_Type != ROUTING_GEOM_TYPE )
    {
        ErrorMgr.AddError( VSP_INVALID_TYPE, "MoveRoutingPt::Geom " + routing_id + " is not a RoutingGeom" );
        return newindex;
    }

    if ( index < 0 || index >= routing_ptr->GetNumPt() )
    {
        ErrorMgr.AddError( VSP_INDEX_OUT_RANGE, "MoveRoutingPt::index " + to_string( index ) + " is out of range" );
        return newindex;
    }

    newindex = routing_ptr->MovePt( index, reorder_type );

    ErrorMgr.NoError();
    return newindex;
}

string GetRoutingPtID( const string &routing_id, int index )
{
    string ret_id;

    Vehicle* veh = GetVehicle();
    Geom* rgeom_ptr = veh->FindGeom( routing_id );
    if ( !rgeom_ptr )
    {
        ErrorMgr.AddError( VSP_INVALID_PTR, "GetRoutingPtID::Can't Find Geom " + routing_id );
        return ret_id;
    }

    RoutingGeom* routing_ptr = dynamic_cast< RoutingGeom* > ( rgeom_ptr );

    if ( !routing_ptr || rgeom_ptr->GetType().m_Type != ROUTING_GEOM_TYPE )
    {
        ErrorMgr.AddError( VSP_INVALID_TYPE, "GetRoutingPtID::Geom " + routing_id + " is not a RoutingGeom" );
        return ret_id;
    }

    if ( index < 0 || index >= routing_ptr->GetNumPt() )
    {
        ErrorMgr.AddError( VSP_INDEX_OUT_RANGE, "GetRoutingPtID::index " + to_string( index ) + " is out of range" );
        return ret_id;
    }

    ret_id = routing_ptr->GetPtID( index );

    ErrorMgr.NoError();
    return ret_id;
}

vector < string > GetAllRoutingPtIds( const string &routing_id )
{
    vector < string > ret_vec;

    Vehicle* veh = GetVehicle();
    Geom* rgeom_ptr = veh->FindGeom( routing_id );
    if ( !rgeom_ptr )
    {
        ErrorMgr.AddError( VSP_INVALID_PTR, "GetAllRoutingPtIds::Can't Find Geom " + routing_id );
        return ret_vec;
    }

    RoutingGeom* routing_ptr = dynamic_cast< RoutingGeom* > ( rgeom_ptr );

    if ( !routing_ptr || rgeom_ptr->GetType().m_Type != ROUTING_GEOM_TYPE )
    {
        ErrorMgr.AddError( VSP_INVALID_TYPE, "GetAllRoutingPtIds::Geom " + routing_id + " is not a RoutingGeom" );
        return ret_vec;
    }

    ret_vec = routing_ptr->GetAllPtIds();

    ErrorMgr.NoError();
    return ret_vec;
}

string GetRoutingPtParentID( const string & pt_id )
{
    string ret_id;

    Vehicle* veh = GetVehicle();
    ParmContainer *pc = ParmMgr.FindParmContainer( pt_id );
    RoutingPoint *rpt = dynamic_cast< RoutingPoint* > ( pc );
    if ( !rpt )
    {
        ErrorMgr.AddError( VSP_INVALID_PTR, "GetRoutingPtParentID::Could not find routing point." );
        return ret_id;
    }

    ret_id = rpt->GetParentID();

    ErrorMgr.NoError();
    return ret_id;
}

void SetRoutingPtParentID( const string & pt_id, const string &parent_id )
{
    Vehicle* veh = GetVehicle();
    ParmContainer *pc = ParmMgr.FindParmContainer( pt_id );
    RoutingPoint *rpt = dynamic_cast< RoutingPoint* > ( pc );
    if ( !rpt )
    {
        ErrorMgr.AddError( VSP_INVALID_PTR, "SetRoutingPtParentID::Could not find routing point." );
        return;
    }

    rpt->SetParentID( parent_id );

    ErrorMgr.NoError();
    return;
}

vec3d GetMainRoutingPtCoord( const string &pt_id )
{
    vec3d ret;

    Vehicle* veh = GetVehicle();
    ParmContainer *pc = ParmMgr.FindParmContainer( pt_id );
    RoutingPoint *rpt = dynamic_cast< RoutingPoint* > ( pc );
    if ( !rpt )
    {
        ErrorMgr.AddError( VSP_INVALID_PTR, "GetRoutingPtCoord::Could not find routing point." );
        return ret;
    }

    ret = rpt->GetPt();

    ErrorMgr.NoError();
    return ret;
}

vec3d GetRoutingPtCoord( const string &routing_id, int index, int symm_index )
{
    vec3d ret;

    Vehicle* veh = GetVehicle();
    Geom* rgeom_ptr = veh->FindGeom( routing_id );
    if ( !rgeom_ptr )
    {
        ErrorMgr.AddError( VSP_INVALID_PTR, "GetRoutingPtCoord::Can't Find Geom " + routing_id );
        return ret;
    }

    RoutingGeom* routing_ptr = dynamic_cast< RoutingGeom* > ( rgeom_ptr );

    if ( !routing_ptr || rgeom_ptr->GetType().m_Type != ROUTING_GEOM_TYPE )
    {
        ErrorMgr.AddError( VSP_INVALID_TYPE, "GetRoutingPtCoord::Geom " + routing_id + " is not a RoutingGeom" );
        return ret;
    }

    if ( index < 0 || index >= routing_ptr->GetNumPt() )
    {
        ErrorMgr.AddError( VSP_INDEX_OUT_RANGE, "GetRoutingPtCoord::index " + to_string( index ) + " is out of range" );
        return ret;
    }

    if ( symm_index < 0 || symm_index >= routing_ptr->GetNumSymmCopies() )
    {
        ErrorMgr.AddError( VSP_INDEX_OUT_RANGE, "GetRoutingPtCoord::symm_index " + to_string( symm_index ) + " is out of range" );
        return ret;
    }

    ret = routing_ptr->GetPtCoord( index,symm_index );

    ErrorMgr.NoError();
    return ret;
}

vector < vec3d > GetAllRoutingPtCoords( const string &routing_id, int symm_index )
{
    vector < vec3d > ret_vec;

    Vehicle* veh = GetVehicle();
    Geom* rgeom_ptr = veh->FindGeom( routing_id );
    if ( !rgeom_ptr )
    {
        ErrorMgr.AddError( VSP_INVALID_PTR, "GetAllRoutingPtCoords::Can't Find Geom " + routing_id );
        return ret_vec;
    }

    RoutingGeom* routing_ptr = dynamic_cast< RoutingGeom* > ( rgeom_ptr );

    if ( !routing_ptr || rgeom_ptr->GetType().m_Type != ROUTING_GEOM_TYPE )
    {
        ErrorMgr.AddError( VSP_INVALID_TYPE, "GetAllRoutingPtCoords::Geom " + routing_id + " is not a RoutingGeom" );
        return ret_vec;
    }

    if ( symm_index < 0 || symm_index >= routing_ptr->GetNumSymmCopies() )
    {
        ErrorMgr.AddError( VSP_INDEX_OUT_RANGE, "GetAllRoutingPtCoords::symm_index " + to_string( symm_index ) + " is out of range" );
        return ret_vec;
    }

    ret_vec = routing_ptr->GetAllPtCoord( symm_index );

    ErrorMgr.NoError();
    return ret_vec;
}

//===================================================================//
//==================      BOR Functions        ======================//
//===================================================================//

//==== Specialized Geom Functions ====//
void ChangeBORXSecShape( const string & geom_id, int type )
{
    Vehicle* veh = GetVehicle();
    Geom* geom_ptr = veh->FindGeom( geom_id );
    if ( !geom_ptr )
    {
        ErrorMgr.AddError( VSP_INVALID_PTR, "ChangeBORXSecShape::Can't Find Geom " + geom_id );
        return;
    }
    else if ( geom_ptr->GetType().m_Type != BOR_GEOM_TYPE )
    {
        ErrorMgr.AddError( VSP_INVALID_TYPE, "ChangeBORXSecShape::Geom " + geom_id + " is not a body of revolution" );
        return;
    }

    BORGeom* bor_ptr = dynamic_cast< BORGeom* > ( geom_ptr );
    bor_ptr->SetXSecCurveType( type );
    ErrorMgr.NoError();
}

//==== Specialized Geom Functions ====//
int GetBORXSecShape( const string & geom_id )
{
    Vehicle* veh = GetVehicle();
    Geom* geom_ptr = veh->FindGeom( geom_id );
    if ( !geom_ptr )
    {
        ErrorMgr.AddError( VSP_INVALID_PTR, "GetBORXSecShape::Can't Find Geom " + geom_id );
        return XS_UNDEFINED;
    }
    else if ( geom_ptr->GetType().m_Type != BOR_GEOM_TYPE )
    {
        ErrorMgr.AddError( VSP_INVALID_TYPE, "GetBORXSecShape::Geom " + geom_id + " is not a body of revolution" );
        return XS_UNDEFINED;
    }

    BORGeom* bor_ptr = dynamic_cast< BORGeom* > ( geom_ptr );

    ErrorMgr.NoError();
    return bor_ptr->GetXSecCurveType();
}

//==== Read XSec From File ====//
vector<vec3d> ReadBORFileXSec( const string& bor_id, const string& file_name )
{
    vector< vec3d > pnt_vec;
    Vehicle* veh = GetVehicle();
    Geom* geom_ptr = veh->FindGeom( bor_id );
    if ( !geom_ptr )
    {
        ErrorMgr.AddError( VSP_INVALID_PTR, "ReadBORFileXSec::Can't Find Geom " + bor_id );
        return pnt_vec;
    }
    else if ( geom_ptr->GetType().m_Type != BOR_GEOM_TYPE )
    {
        ErrorMgr.AddError( VSP_INVALID_TYPE, "ReadBORFileXSec::Geom " + bor_id + " is not a body of revolution" );
        return pnt_vec;
    }

    BORGeom* bor_ptr = dynamic_cast< BORGeom* > ( geom_ptr );

    XSecCurve* xsc = bor_ptr->GetXSecCurve();

    if ( !xsc )
    {
        ErrorMgr.AddError( VSP_INVALID_PTR, "ReadBORFileXSec::Can't Get XSecCurve" );
        return pnt_vec;
    }

    if ( xsc->GetType() == XS_FILE_FUSE )
    {
        FileXSec* file_xs = dynamic_cast<FileXSec*>( xsc );
        assert( file_xs );
        if ( file_xs->ReadXsecFile( file_name ) )
        {
            ErrorMgr.NoError();
            return file_xs->GetUnityFilePnts();
        }
        else
        {
            ErrorMgr.AddError( VSP_FILE_DOES_NOT_EXIST, "ReadBORFileXSec::Error reading fuselage file " + file_name );
            return pnt_vec;
        }
    }

    ErrorMgr.AddError( VSP_WRONG_XSEC_TYPE, "ReadBORFileXSec::XSec Not XS_FILE_FUSE Type " );
    return pnt_vec;
}

//==== Set XSec Pnts ====//
void SetBORXSecPnts( const string& bor_id, vector< vec3d > & pnt_vec )
{
    Vehicle* veh = GetVehicle();
    Geom* geom_ptr = veh->FindGeom( bor_id );
    if ( !geom_ptr )
    {
        ErrorMgr.AddError( VSP_INVALID_PTR, "SetBORXSecPnts::Can't Find Geom " + bor_id );
        return;
    }
    else if ( geom_ptr->GetType().m_Type != BOR_GEOM_TYPE )
    {
        ErrorMgr.AddError( VSP_INVALID_TYPE, "SetBORXSecPnts::Geom " + bor_id + " is not a body of revolution" );
        return;
    }

    BORGeom* bor_ptr = dynamic_cast< BORGeom* > ( geom_ptr );

    XSecCurve* xsc = bor_ptr->GetXSecCurve();

    if ( !xsc )
    {
        ErrorMgr.AddError( VSP_INVALID_PTR, "SetBORXSecPnts::Can't Get XSecCurve" );
        return;
    }

    if ( xsc->GetType() != XS_FILE_FUSE )
    {
        ErrorMgr.AddError( VSP_WRONG_XSEC_TYPE, "SetBORXSecPnts::Wrong XSec Type" );
        return;
    }

    FileXSec* file_xs = dynamic_cast<FileXSec*>( xsc );
    assert( file_xs );
    file_xs->SetPnts( pnt_vec );
    ErrorMgr.NoError();
}

//==== Compute Point Along XSec ====//
vec3d ComputeBORXSecPnt( const string& bor_id, double fract )
{
    Vehicle* veh = GetVehicle();
    Geom* geom_ptr = veh->FindGeom( bor_id );
    if ( !geom_ptr )
    {
        ErrorMgr.AddError( VSP_INVALID_PTR, "ComputeBORXSecPnt::Can't Find Geom " + bor_id );
        return vec3d();
    }
    else if ( geom_ptr->GetType().m_Type != BOR_GEOM_TYPE )
    {
        ErrorMgr.AddError( VSP_INVALID_TYPE, "ComputeBORXSecPnt::Geom " + bor_id + " is not a body of revolution" );
        return vec3d();
    }

    BORGeom* bor_ptr = dynamic_cast< BORGeom* > ( geom_ptr );

    XSecCurve* xsc = bor_ptr->GetXSecCurve();

    if ( !xsc )
    {
        ErrorMgr.AddError( VSP_INVALID_PTR, "ComputeBORXSecPnt::Can't Get XSecCurve" );
        return vec3d();
    }

    vec3d pnt = xsc->GetCurve().CompPnt01( fract );
    ErrorMgr.NoError();

    return pnt;
}

//==== Compute Tan Along XSec ====//
vec3d ComputeBORXSecTan( const string& bor_id, double fract )
{
    Vehicle* veh = GetVehicle();
    Geom* geom_ptr = veh->FindGeom( bor_id );
    if ( !geom_ptr )
    {
        ErrorMgr.AddError( VSP_INVALID_PTR, "ComputeBORXSecTan::Can't Find Geom " + bor_id );
        return vec3d();
    }
    else if ( geom_ptr->GetType().m_Type != BOR_GEOM_TYPE )
    {
        ErrorMgr.AddError( VSP_INVALID_TYPE, "ComputeBORXSecTan::Geom " + bor_id + " is not a body of revolution" );
        return vec3d();
    }

    BORGeom* bor_ptr = dynamic_cast< BORGeom* > ( geom_ptr );

    XSecCurve* xsc = bor_ptr->GetXSecCurve();

    if ( !xsc )
    {
        ErrorMgr.AddError( VSP_INVALID_PTR, "ComputeBORXSecTan::Can't Get XSecCurve" );
        return vec3d();
    }

    vec3d pnt = xsc->GetCurve().CompTan01( fract );
    ErrorMgr.NoError();

    return pnt;
}

void ReadBORFileAirfoil( const string& bor_id, const string& file_name )
{
    Vehicle* veh = GetVehicle();
    Geom* geom_ptr = veh->FindGeom( bor_id );
    if ( !geom_ptr )
    {
        ErrorMgr.AddError( VSP_INVALID_PTR, "ReadBORFileAirfoil::Can't Find Geom " + bor_id );
        return;
    }
    else if ( geom_ptr->GetType().m_Type != BOR_GEOM_TYPE )
    {
        ErrorMgr.AddError( VSP_INVALID_TYPE, "ReadBORFileAirfoil::Geom " + bor_id + " is not a body of revolution" );
        return;
    }

    BORGeom* bor_ptr = dynamic_cast< BORGeom* > ( geom_ptr );

    XSecCurve* xsc = bor_ptr->GetXSecCurve();

    if ( !xsc )
    {
        ErrorMgr.AddError( VSP_INVALID_PTR, "ReadBORFileAirfoil::Can't Get XSecCurve" );
        return;
    }

    if ( xsc->GetType() == XS_FILE_AIRFOIL )
    {
        FileAirfoil* file_xs = dynamic_cast<FileAirfoil*>( xsc );
        assert( file_xs );
        if( file_xs->ReadFile( file_name ) )
        {
            ErrorMgr.NoError();
            return;
        }
        else
        {
            ErrorMgr.AddError( VSP_FILE_DOES_NOT_EXIST, "ReadBORFileAirfoil::Error reading airfoil file " + file_name );
            return;
        }
    }

    ErrorMgr.AddError( VSP_WRONG_XSEC_TYPE, "ReadBORFileAirfoil::XSec Not XS_FILE_AIRFOIL Type " + bor_id );
    return;
}

void SetBORAirfoilUpperPnts( const string& bor_id, const std::vector< vec3d > & up_pnt_vec )
{
    Vehicle* veh = GetVehicle();
    Geom* geom_ptr = veh->FindGeom( bor_id );
    if ( !geom_ptr )
    {
        ErrorMgr.AddError( VSP_INVALID_PTR, "SetBORAirfoilUpperPnts::Can't Find Geom " + bor_id );
        return;
    }
    else if ( geom_ptr->GetType().m_Type != BOR_GEOM_TYPE )
    {
        ErrorMgr.AddError( VSP_INVALID_TYPE, "SetBORAirfoilUpperPnts::Geom " + bor_id + " is not a body of revolution" );
        return;
    }

    BORGeom* bor_ptr = dynamic_cast< BORGeom* > ( geom_ptr );

    XSecCurve* xsc = bor_ptr->GetXSecCurve();

    if ( !xsc )
    {
        ErrorMgr.AddError( VSP_INVALID_PTR, "SetBORAirfoilUpperPnts::Can't Get XSecCurve" );
        return;
    }

    if ( xsc->GetType() != XS_FILE_AIRFOIL )
    {
        ErrorMgr.AddError( VSP_WRONG_XSEC_TYPE, "SetBORAirfoilUpperPnts::XSec Not XS_FILE_AIRFOIL Type" );
        return;
    }

    FileAirfoil* file_xs = dynamic_cast<FileAirfoil*>( xsc );
    assert( file_xs );
    file_xs->SetAirfoilUpperPnts( up_pnt_vec );
    ErrorMgr.NoError();
}

void SetBORAirfoilLowerPnts( const string& bor_id, const std::vector< vec3d > & low_pnt_vec )
{
    Vehicle* veh = GetVehicle();
    Geom* geom_ptr = veh->FindGeom( bor_id );
    if ( !geom_ptr )
    {
        ErrorMgr.AddError( VSP_INVALID_PTR, "SetBORAirfoilLowerPnts::Can't Find Geom " + bor_id );
        return;
    }
    else if ( geom_ptr->GetType().m_Type != BOR_GEOM_TYPE )
    {
        ErrorMgr.AddError( VSP_INVALID_TYPE, "SetBORAirfoilLowerPnts::Geom " + bor_id + " is not a body of revolution" );
        return;
    }

    BORGeom* bor_ptr = dynamic_cast< BORGeom* > ( geom_ptr );

    XSecCurve* xsc = bor_ptr->GetXSecCurve();

    if ( !xsc )
    {
        ErrorMgr.AddError( VSP_INVALID_PTR, "SetBORAirfoilLowerPnts::Can't Get XSecCurve" );
        return;
    }

    if ( xsc->GetType() != XS_FILE_AIRFOIL )
    {
        ErrorMgr.AddError( VSP_WRONG_XSEC_TYPE, "SetBORAirfoilLowerPnts::XSec Not XS_FILE_AIRFOIL Type" );
        return;
    }

    FileAirfoil* file_xs = dynamic_cast<FileAirfoil*>( xsc );
    assert( file_xs );
    file_xs->SetAirfoilLowerPnts( low_pnt_vec );
    ErrorMgr.NoError();
}

void SetBORAirfoilPnts( const string& bor_id, const std::vector< vec3d > & up_pnt_vec, const std::vector< vec3d > & low_pnt_vec )
{
    Vehicle* veh = GetVehicle();
    Geom* geom_ptr = veh->FindGeom( bor_id );
    if ( !geom_ptr )
    {
        ErrorMgr.AddError( VSP_INVALID_PTR, "SetBORAirfoilPnts::Can't Find Geom " + bor_id );
        return;
    }
    else if ( geom_ptr->GetType().m_Type != BOR_GEOM_TYPE )
    {
        ErrorMgr.AddError( VSP_INVALID_TYPE, "SetBORAirfoilPnts::Geom " + bor_id + " is not a body of revolution" );
        return;
    }

    BORGeom* bor_ptr = dynamic_cast< BORGeom* > ( geom_ptr );

    XSecCurve* xsc = bor_ptr->GetXSecCurve();

    if ( !xsc )
    {
        ErrorMgr.AddError( VSP_INVALID_PTR, "SetBORAirfoilPnts::Can't Get XSecCurve" );
        return;
    }

    if ( xsc->GetType() != XS_FILE_AIRFOIL )
    {
        ErrorMgr.AddError( VSP_WRONG_XSEC_TYPE, "SetBORAirfoilPnts::XSec Not XS_FILE_AIRFOIL Type" );
        return;
    }

    FileAirfoil* file_xs = dynamic_cast<FileAirfoil*>( xsc );
    assert( file_xs );
    file_xs->SetAirfoilPnts( up_pnt_vec, low_pnt_vec );
    ErrorMgr.NoError();
}

std::vector<vec3d> GetBORAirfoilUpperPnts( const string& bor_id )
{
    vector< vec3d > pnt_vec;
    Vehicle* veh = GetVehicle();
    Geom* geom_ptr = veh->FindGeom( bor_id );
    if ( !geom_ptr )
    {
        ErrorMgr.AddError( VSP_INVALID_PTR, "GetBORAirfoilUpperPnts::Can't Find Geom " + bor_id );
        return pnt_vec;
    }
    else if ( geom_ptr->GetType().m_Type != BOR_GEOM_TYPE )
    {
        ErrorMgr.AddError( VSP_INVALID_TYPE, "GetBORAirfoilUpperPnts::Geom " + bor_id + " is not a body of revolution" );
        return pnt_vec;
    }

    BORGeom* bor_ptr = dynamic_cast< BORGeom* > ( geom_ptr );

    XSecCurve* xsc = bor_ptr->GetXSecCurve();

    if ( !xsc )
    {
        ErrorMgr.AddError( VSP_INVALID_PTR, "GetBORAirfoilUpperPnts::Can't Get XSecCurve" );
        return pnt_vec;
    }

    if ( xsc->GetType() != XS_FILE_AIRFOIL )
    {
        ErrorMgr.AddError( VSP_WRONG_XSEC_TYPE, "GetBORAirfoilUpperPnts::XSec Not XS_FILE_AIRFOIL Type" );
        return pnt_vec;
    }

    FileAirfoil* file_xs = dynamic_cast<FileAirfoil*>( xsc );
    assert( file_xs );
    pnt_vec = file_xs->GetUpperPnts();
    ErrorMgr.NoError();
    return pnt_vec;
}

std::vector<vec3d> GetBORAirfoilLowerPnts( const string& bor_id )
{
    vector< vec3d > pnt_vec;

    Vehicle* veh = GetVehicle();
    Geom* geom_ptr = veh->FindGeom( bor_id );
    if ( !geom_ptr )
    {
        ErrorMgr.AddError( VSP_INVALID_PTR, "GetBORAirfoilLowerPnts::Can't Find Geom " + bor_id );
        return pnt_vec;
    }
    else if ( geom_ptr->GetType().m_Type != BOR_GEOM_TYPE )
    {
        ErrorMgr.AddError( VSP_INVALID_TYPE, "GetBORAirfoilLowerPnts::Geom " + bor_id + " is not a body of revolution" );
        return pnt_vec;
    }

    BORGeom* bor_ptr = dynamic_cast< BORGeom* > ( geom_ptr );

    XSecCurve* xsc = bor_ptr->GetXSecCurve();

    if ( !xsc )
    {
        ErrorMgr.AddError( VSP_INVALID_PTR, "GetBORAirfoilLowerPnts::Can't Get XSecCurve" );
        return pnt_vec;
    }

    if ( xsc->GetType() != XS_FILE_AIRFOIL )
    {
        ErrorMgr.AddError( VSP_WRONG_XSEC_TYPE, "GetBORAirfoilLowerPnts::XSec Not XS_FILE_AIRFOIL Type" );
        return pnt_vec;
    }

    FileAirfoil* file_xs = dynamic_cast<FileAirfoil*>( xsc );
    assert( file_xs );
    pnt_vec = file_xs->GetLowerPnts();
    ErrorMgr.NoError();
    return pnt_vec;
}

std::vector<double> GetBORUpperCSTCoefs( const string& bor_id )
{
    vector < double > ret_vec;

    Vehicle* veh = GetVehicle();
    Geom* geom_ptr = veh->FindGeom( bor_id );
    if ( !geom_ptr )
    {
        ErrorMgr.AddError( VSP_INVALID_PTR, "GetBORUpperCSTCoefs::Can't Find Geom " + bor_id );
        return ret_vec;
    }
    else if ( geom_ptr->GetType().m_Type != BOR_GEOM_TYPE )
    {
        ErrorMgr.AddError( VSP_INVALID_TYPE, "GetBORUpperCSTCoefs::Geom " + bor_id + " is not a body of revolution" );
        return ret_vec;
    }

    BORGeom* bor_ptr = dynamic_cast< BORGeom* > ( geom_ptr );

    XSecCurve* xsc = bor_ptr->GetXSecCurve();

    if ( !xsc )
    {
        ErrorMgr.AddError( VSP_INVALID_PTR, "GetBORUpperCSTCoefs::Can't Get XSecCurve" );
        return ret_vec;
    }


    if ( xsc->GetType() != XS_CST_AIRFOIL )
    {
        ErrorMgr.AddError( VSP_WRONG_XSEC_TYPE, "GetBORUpperCSTCoefs::XSec Not XS_CST_AIRFOIL Type" );
        return ret_vec;
    }

    CSTAirfoil* cst_xs = dynamic_cast<CSTAirfoil*>( xsc );
    assert( cst_xs );

    ret_vec = cst_xs->GetUpperCST();
    ErrorMgr.NoError();
    return ret_vec;
}

std::vector<double> GetBORLowerCSTCoefs( const string& bor_id )
{
    vector < double > ret_vec;

    Vehicle* veh = GetVehicle();
    Geom* geom_ptr = veh->FindGeom( bor_id );
    if ( !geom_ptr )
    {
        ErrorMgr.AddError( VSP_INVALID_PTR, "GetBORLowerCSTCoefs::Can't Find Geom " + bor_id );
        return ret_vec;
    }
    else if ( geom_ptr->GetType().m_Type != BOR_GEOM_TYPE )
    {
        ErrorMgr.AddError( VSP_INVALID_TYPE, "GetBORLowerCSTCoefs::Geom " + bor_id + " is not a body of revolution" );
        return ret_vec;
    }

    BORGeom* bor_ptr = dynamic_cast< BORGeom* > ( geom_ptr );

    XSecCurve* xsc = bor_ptr->GetXSecCurve();

    if ( !xsc )
    {
        ErrorMgr.AddError( VSP_INVALID_PTR, "GetBORLowerCSTCoefs::Can't Get XSecCurve" );
        return ret_vec;
    }

    if ( xsc->GetType() != XS_CST_AIRFOIL )
    {
        ErrorMgr.AddError( VSP_WRONG_XSEC_TYPE, "GetBORLowerCSTCoefs::XSec Not XS_CST_AIRFOIL Type" );
        return ret_vec;
    }

    CSTAirfoil* cst_xs = dynamic_cast<CSTAirfoil*>( xsc );
    assert( cst_xs );

    ret_vec = cst_xs->GetLowerCST();
    ErrorMgr.NoError();
    return ret_vec;
}

int GetBORUpperCSTDegree( const string& bor_id )
{
    int deg = -1;

    Vehicle* veh = GetVehicle();
    Geom* geom_ptr = veh->FindGeom( bor_id );
    if ( !geom_ptr )
    {
        ErrorMgr.AddError( VSP_INVALID_PTR, "GetBORUpperCSTDegree::Can't Find Geom " + bor_id );
        return deg;
    }
    else if ( geom_ptr->GetType().m_Type != BOR_GEOM_TYPE )
    {
        ErrorMgr.AddError( VSP_INVALID_TYPE, "GetBORUpperCSTDegree::Geom " + bor_id + " is not a body of revolution" );
        return deg;
    }

    BORGeom* bor_ptr = dynamic_cast< BORGeom* > ( geom_ptr );

    XSecCurve* xsc = bor_ptr->GetXSecCurve();

    if ( !xsc )
    {
        ErrorMgr.AddError( VSP_INVALID_PTR, "GetBORUpperCSTDegree::Can't Get XSecCurve" );
        return deg;
    }

    if ( xsc->GetType() != XS_CST_AIRFOIL )
    {
        ErrorMgr.AddError( VSP_WRONG_XSEC_TYPE, "GetBORUpperCSTDegree::XSec Not XS_CST_AIRFOIL Type" );
        return deg;
    }

    CSTAirfoil* cst_xs = dynamic_cast<CSTAirfoil*>( xsc );
    assert( cst_xs );

    deg = cst_xs->GetUpperDegree();
    ErrorMgr.NoError();
    return deg;
}

int GetBORLowerCSTDegree( const string& bor_id )
{
    int deg = -1;

    Vehicle* veh = GetVehicle();
    Geom* geom_ptr = veh->FindGeom( bor_id );
    if ( !geom_ptr )
    {
        ErrorMgr.AddError( VSP_INVALID_PTR, "GetBORLowerCSTDegree::Can't Find Geom " + bor_id );
        return deg;
    }
    else if ( geom_ptr->GetType().m_Type != BOR_GEOM_TYPE )
    {
        ErrorMgr.AddError( VSP_INVALID_TYPE, "GetBORLowerCSTDegree::Geom " + bor_id + " is not a body of revolution" );
        return deg;
    }

    BORGeom* bor_ptr = dynamic_cast< BORGeom* > ( geom_ptr );

    XSecCurve* xsc = bor_ptr->GetXSecCurve();

    if ( !xsc )
    {
        ErrorMgr.AddError( VSP_INVALID_PTR, "GetBORLowerCSTDegree::Can't Get XSecCurve" );
        return deg;
    }

    if ( xsc->GetType() != XS_CST_AIRFOIL )
    {
        ErrorMgr.AddError( VSP_WRONG_XSEC_TYPE, "GetBORLowerCSTDegree::XSec Not XS_CST_AIRFOIL Type" );
        return deg;
    }


    CSTAirfoil* cst_xs = dynamic_cast<CSTAirfoil*>( xsc );
    assert( cst_xs );

    deg = cst_xs->GetLowerDegree();
    ErrorMgr.NoError();
    return deg;
}

void SetBORUpperCST( const string& bor_id, int deg, const std::vector<double> &coefs )
{
    Vehicle* veh = GetVehicle();
    Geom* geom_ptr = veh->FindGeom( bor_id );
    if ( !geom_ptr )
    {
        ErrorMgr.AddError( VSP_INVALID_PTR, "SetBORUpperCST::Can't Find Geom " + bor_id );
        return;
    }
    else if ( geom_ptr->GetType().m_Type != BOR_GEOM_TYPE )
    {
        ErrorMgr.AddError( VSP_INVALID_TYPE, "SetBORUpperCST::Geom " + bor_id + " is not a body of revolution" );
        return;
    }

    BORGeom* bor_ptr = dynamic_cast< BORGeom* > ( geom_ptr );

    XSecCurve* xsc = bor_ptr->GetXSecCurve();

    if ( !xsc )
    {
        ErrorMgr.AddError( VSP_INVALID_PTR, "SetBORUpperCST::Can't Get XSecCurve" );
        return;
    }

    if ( xsc->GetType() != XS_CST_AIRFOIL )
    {
        ErrorMgr.AddError( VSP_WRONG_XSEC_TYPE, "SetBORUpperCST::XSec Not XS_CST_AIRFOIL Type" );
        return;
    }

    CSTAirfoil* cst_xs = dynamic_cast<CSTAirfoil*>( xsc );
    assert( cst_xs );

    ErrorMgr.NoError();
    cst_xs->SetUpperCST( deg, coefs );
}

void SetBORLowerCST( const string& bor_id, int deg, const std::vector<double> &coefs )
{
    Vehicle* veh = GetVehicle();
    Geom* geom_ptr = veh->FindGeom( bor_id );
    if ( !geom_ptr )
    {
        ErrorMgr.AddError( VSP_INVALID_PTR, "SetBORLowerCST::Can't Find Geom " + bor_id );
        return;
    }
    else if ( geom_ptr->GetType().m_Type != BOR_GEOM_TYPE )
    {
        ErrorMgr.AddError( VSP_INVALID_TYPE, "SetBORLowerCST::Geom " + bor_id + " is not a body of revolution" );
        return;
    }

    BORGeom* bor_ptr = dynamic_cast< BORGeom* > ( geom_ptr );

    XSecCurve* xsc = bor_ptr->GetXSecCurve();

    if ( !xsc )
    {
        ErrorMgr.AddError( VSP_INVALID_PTR, "SetBORLowerCST::Can't Get XSecCurve" );
        return;
    }

    if ( xsc->GetType() != XS_CST_AIRFOIL )
    {
        ErrorMgr.AddError( VSP_WRONG_XSEC_TYPE, "SetBORLowerCST::XSec Not XS_CST_AIRFOIL Type" );
        return;
    }

    CSTAirfoil* cst_xs = dynamic_cast<CSTAirfoil*>( xsc );
    assert( cst_xs );

    ErrorMgr.NoError();
    cst_xs->SetLowerCST( deg, coefs );
}

void PromoteBORCSTUpper( const string& bor_id )
{
    Vehicle* veh = GetVehicle();
    Geom* geom_ptr = veh->FindGeom( bor_id );
    if ( !geom_ptr )
    {
        ErrorMgr.AddError( VSP_INVALID_PTR, "PromoteBORCSTUpper::Can't Find Geom " + bor_id );
        return;
    }
    else if ( geom_ptr->GetType().m_Type != BOR_GEOM_TYPE )
    {
        ErrorMgr.AddError( VSP_INVALID_TYPE, "PromoteBORCSTUpper::Geom " + bor_id + " is not a body of revolution" );
        return;
    }

    BORGeom* bor_ptr = dynamic_cast< BORGeom* > ( geom_ptr );

    XSecCurve* xsc = bor_ptr->GetXSecCurve();

    if ( !xsc )
    {
        ErrorMgr.AddError( VSP_INVALID_PTR, "PromoteBORCSTUpper::Can't Get XSecCurve" );
        return;
    }

    if ( xsc->GetType() != XS_CST_AIRFOIL )
    {
        ErrorMgr.AddError( VSP_WRONG_XSEC_TYPE, "PromoteBORCSTUpper::XSec Not XS_CST_AIRFOIL Type" );
        return;
    }

    CSTAirfoil* cst_xs = dynamic_cast<CSTAirfoil*>( xsc );
    assert( cst_xs );

    ErrorMgr.NoError();
    cst_xs->PromoteUpper();
}

void PromoteBORCSTLower( const string& bor_id )
{
    Vehicle* veh = GetVehicle();
    Geom* geom_ptr = veh->FindGeom( bor_id );
    if ( !geom_ptr )
    {
        ErrorMgr.AddError( VSP_INVALID_PTR, "PromoteBORCSTLower::Can't Find Geom " + bor_id );
        return;
    }
    else if ( geom_ptr->GetType().m_Type != BOR_GEOM_TYPE )
    {
        ErrorMgr.AddError( VSP_INVALID_TYPE, "PromoteBORCSTLower::Geom " + bor_id + " is not a body of revolution" );
        return;
    }

    BORGeom* bor_ptr = dynamic_cast< BORGeom* > ( geom_ptr );

    XSecCurve* xsc = bor_ptr->GetXSecCurve();

    if ( !xsc )
    {
        ErrorMgr.AddError( VSP_INVALID_PTR, "PromoteBORCSTLower::Can't Get XSecCurve" );
        return;
    }

    if ( xsc->GetType() != XS_CST_AIRFOIL )
    {
        ErrorMgr.AddError( VSP_WRONG_XSEC_TYPE, "PromoteBORCSTLower::XSec Not XS_CST_AIRFOIL Type" );
        return;
    }

    CSTAirfoil* cst_xs = dynamic_cast<CSTAirfoil*>( xsc );
    assert( cst_xs );

    ErrorMgr.NoError();
    cst_xs->PromoteLower();
}

void DemoteBORCSTUpper( const string& bor_id )
{
    Vehicle* veh = GetVehicle();
    Geom* geom_ptr = veh->FindGeom( bor_id );
    if ( !geom_ptr )
    {
        ErrorMgr.AddError( VSP_INVALID_PTR, "DemoteBORCSTUpper::Can't Find Geom " + bor_id );
        return;
    }
    else if ( geom_ptr->GetType().m_Type != BOR_GEOM_TYPE )
    {
        ErrorMgr.AddError( VSP_INVALID_TYPE, "DemoteBORCSTUpper::Geom " + bor_id + " is not a body of revolution" );
        return;
    }

    BORGeom* bor_ptr = dynamic_cast< BORGeom* > ( geom_ptr );

    XSecCurve* xsc = bor_ptr->GetXSecCurve();

    if ( !xsc )
    {
        ErrorMgr.AddError( VSP_INVALID_PTR, "DemoteBORCSTUpper::Can't Get XSecCurve" );
        return;
    }

    if ( xsc->GetType() != XS_CST_AIRFOIL )
    {
        ErrorMgr.AddError( VSP_WRONG_XSEC_TYPE, "DemoteBORCSTUpper::XSec Not XS_CST_AIRFOIL Type" );
        return;
    }

    CSTAirfoil* cst_xs = dynamic_cast<CSTAirfoil*>( xsc );
    assert( cst_xs );

    ErrorMgr.NoError();
    cst_xs->DemoteUpper();
}

void DemoteBORCSTLower( const string& bor_id )
{
    Vehicle* veh = GetVehicle();
    Geom* geom_ptr = veh->FindGeom( bor_id );
    if ( !geom_ptr )
    {
        ErrorMgr.AddError( VSP_INVALID_PTR, "DemoteBORCSTLower::Can't Find Geom " + bor_id );
        return;
    }
    else if ( geom_ptr->GetType().m_Type != BOR_GEOM_TYPE )
    {
        ErrorMgr.AddError( VSP_INVALID_TYPE, "DemoteBORCSTLower::Geom " + bor_id + " is not a body of revolution" );
        return;
    }

    BORGeom* bor_ptr = dynamic_cast< BORGeom* > ( geom_ptr );

    XSecCurve* xsc = bor_ptr->GetXSecCurve();

    if ( !xsc )
    {
        ErrorMgr.AddError( VSP_INVALID_PTR, "DemoteBORCSTLower::Can't Get XSecCurve" );
        return;
    }

    if ( xsc->GetType() != XS_CST_AIRFOIL )
    {
        ErrorMgr.AddError( VSP_WRONG_XSEC_TYPE, "DemoteBORCSTLower::XSec Not XS_CST_AIRFOIL Type" );
        return;
    }

    CSTAirfoil* cst_xs = dynamic_cast<CSTAirfoil*>( xsc );
    assert( cst_xs );

    ErrorMgr.NoError();
    cst_xs->DemoteLower();
}

void FitBORAfCST( const string & bor_id, int deg )
{
    Vehicle* veh = GetVehicle();
    Geom* geom_ptr = veh->FindGeom( bor_id );
    if ( !geom_ptr )
    {
        ErrorMgr.AddError( VSP_INVALID_PTR, "FitBORAfCST::Can't Find Geom " + bor_id );
        return;
    }
    else if ( geom_ptr->GetType().m_Type != BOR_GEOM_TYPE )
    {
        ErrorMgr.AddError( VSP_INVALID_TYPE, "FitBORAfCST::Geom " + bor_id + " is not a body of revolution" );
        return;
    }

    BORGeom* bor_ptr = dynamic_cast< BORGeom* > ( geom_ptr );

    XSecCurve* xsc = bor_ptr->GetXSecCurve();

    if ( !xsc )
    {
        ErrorMgr.AddError( VSP_INVALID_PTR, "FitBORAfCST::Can't Get XSecCurve" );
        return;
    }

    if ( ( xsc->GetType() != XS_FOUR_SERIES ) &&
         ( xsc->GetType() != XS_SIX_SERIES ) &&
         ( xsc->GetType() != XS_FOUR_DIGIT_MOD ) &&
         ( xsc->GetType() != XS_FIVE_DIGIT ) &&
         ( xsc->GetType() != XS_FIVE_DIGIT_MOD ) &&
         ( xsc->GetType() != XS_ONE_SIX_SERIES ) &&
         ( xsc->GetType() != XS_FILE_AIRFOIL ) )
    {
        ErrorMgr.AddError( VSP_WRONG_XSEC_TYPE, "FitBORAfCST::XSec Not Fittable Airfoil Type" );
        return;
    }

    Airfoil* af_xs = dynamic_cast< Airfoil* >( xsc );
    if ( !af_xs )
    {
        ErrorMgr.AddError( VSP_INVALID_PTR, "FitBORAfCST::Can't Get Airfoil" );
        return;
    }

    VspCurve c = af_xs->GetOrigCurve();

    bor_ptr->SetXSecCurveType( XS_CST_AIRFOIL );

    XSecCurve* newxsc = bor_ptr->GetXSecCurve();
    if ( !newxsc )
    {
        ErrorMgr.AddError( VSP_INVALID_PTR, "FitBORAfCST::Can't Get New XSecCurve" );
        return;
    }

    if ( newxsc->GetType() != XS_CST_AIRFOIL )
    {
        ErrorMgr.AddError( VSP_WRONG_XSEC_TYPE, "FitBORAfCST::XSec Not XS_CST_AIRFOIL Type" );
        return;
    }

    CSTAirfoil* cst_xs = dynamic_cast< CSTAirfoil* >( newxsc );

    assert( cst_xs );
    cst_xs->FitCurve( c, deg );

    ErrorMgr.NoError();
}

//===================================================================//
//===============      Edit XSec Functions        ===================//
//===================================================================//

void EditXSecInitShape( const std::string & xsec_id )
{
    XSec* xs = FindXSec( xsec_id );
    if ( !xs )
    {
        ErrorMgr.AddError( VSP_INVALID_PTR, "EditXSecInitShape::Can't Find XSec " + xsec_id );
        return;
    }

    if ( xs->GetXSecCurve()->GetType() != XS_EDIT_CURVE )
    {
        ErrorMgr.AddError( VSP_WRONG_XSEC_TYPE, "EditXSecInitShape::XSec Not XS_EDIT_CURVE Type" );
        return;
    }

    EditCurveXSec* edit_xs = dynamic_cast<EditCurveXSec*>( xs->GetXSecCurve() );
    assert( edit_xs );

    ErrorMgr.NoError();

    edit_xs->InitShape();
}

void EditXSecConvertTo( const std::string & xsec_id, const int & newtype )
{
    XSec* xs = FindXSec( xsec_id );
    if ( !xs )
    {
        ErrorMgr.AddError( VSP_INVALID_PTR, "EditXSecConvertTo::Can't Find XSec " + xsec_id );
        return;
    }

    if ( xs->GetXSecCurve()->GetType() != XS_EDIT_CURVE )
    {
        ErrorMgr.AddError( VSP_WRONG_XSEC_TYPE, "EditXSecConvertTo::XSec Not XS_EDIT_CURVE Type" );
        return;
    }

    if ( newtype < 0 || newtype > vsp::CEDIT )
    {
        ErrorMgr.AddError( VSP_INVALID_TYPE, "EditXSecConvertTo::Invalid PCURV_TYPE" );
        return;
    }

    EditCurveXSec* edit_xs = dynamic_cast<EditCurveXSec*>( xs->GetXSecCurve() );
    assert( edit_xs );

    ErrorMgr.NoError();

    edit_xs->ConvertTo( newtype );
}

vector < double > GetEditXSecUVec( const std::string& xsec_id )
{
    vector < double > ret_vec;

    XSec* xs = FindXSec( xsec_id );
    if ( !xs )
    {
        ErrorMgr.AddError( VSP_INVALID_PTR, "GetEditXSecUVec::Can't Find XSec " + xsec_id );
        return ret_vec;
    }

    if ( xs->GetXSecCurve()->GetType() != XS_EDIT_CURVE )
    {
        ErrorMgr.AddError( VSP_WRONG_XSEC_TYPE, "GetEditXSecUVec::XSec Not XS_EDIT_CURVE Type" );
        return ret_vec;
    }

    EditCurveXSec* edit_xs = dynamic_cast<EditCurveXSec*>( xs->GetXSecCurve() );
    assert( edit_xs );

    ErrorMgr.NoError();

    return edit_xs->GetUVec();
}

vector < vec3d > GetEditXSecCtrlVec( const std::string& xsec_id, bool non_dimensional )
{
    vector < vec3d > ret_vec;

    XSec* xs = FindXSec( xsec_id );
    if ( !xs )
    {
        ErrorMgr.AddError( VSP_INVALID_PTR, "GetEditXSecCtrlVec::Can't Find XSec " + xsec_id );
        return ret_vec;
    }

    if ( xs->GetXSecCurve()->GetType() != XS_EDIT_CURVE )
    {
        ErrorMgr.AddError( VSP_WRONG_XSEC_TYPE, "GetEditXSecCtrlVec::XSec Not XS_EDIT_CURVE Type" );
        return ret_vec;
    }

    EditCurveXSec* edit_xs = dynamic_cast<EditCurveXSec*>( xs->GetXSecCurve() );
    assert( edit_xs );

    ErrorMgr.NoError();

    return edit_xs->GetCtrlPntVec( non_dimensional );
}

void SetEditXSecPnts( const std::string & xsec_id, const vector < double > &u_vec, const vector < vec3d > &control_pts, const vector < double > &r_vec )
{
    XSec* xs = FindXSec( xsec_id );
    if ( !xs )
    {
        ErrorMgr.AddError( VSP_INVALID_PTR, "SetEditXSecPnts::Can't Find XSec " + xsec_id );
        return;
    }

    if ( xs->GetXSecCurve()->GetType() != XS_EDIT_CURVE )
    {
        ErrorMgr.AddError( VSP_WRONG_XSEC_TYPE, "SetEditXSecPnts::XSec Not XS_EDIT_CURVE Type" );
        return;
    }

    EditCurveXSec* edit_xs = dynamic_cast<EditCurveXSec*>( xs->GetXSecCurve() );
    assert( edit_xs );

    ErrorMgr.NoError();

    edit_xs->SetPntVecs( u_vec, control_pts, r_vec );
}

void EditXSecDelPnt( const std::string & xsec_id, const int & indx )
{
    XSec* xs = FindXSec( xsec_id );
    if ( !xs )
    {
        ErrorMgr.AddError( VSP_INVALID_PTR, "EditXSecDelPnt::Can't Find XSec " + xsec_id );
        return;
    }

    if ( xs->GetXSecCurve()->GetType() != XS_EDIT_CURVE )
    {
        ErrorMgr.AddError( VSP_WRONG_XSEC_TYPE, "EditXSecDelPnt::XSec Not XS_EDIT_CURVE Type" );
        return;
    }

    EditCurveXSec* edit_xs = dynamic_cast<EditCurveXSec*>( xs->GetXSecCurve() );
    assert( edit_xs );

    ErrorMgr.NoError();

    return edit_xs->DeletePt( indx );
}

int EditXSecSplit01( const std::string & xsec_id, const double & u )
{
    XSec* xs = FindXSec( xsec_id );
    if ( !xs )
    {
        ErrorMgr.AddError( VSP_INVALID_PTR, "EditXSecSplit01::Can't Find XSec " + xsec_id );
        return -1;
    }

    if ( xs->GetXSecCurve()->GetType() != XS_EDIT_CURVE )
    {
        ErrorMgr.AddError( VSP_WRONG_XSEC_TYPE, "EditXSecSplit01::XSec Not XS_EDIT_CURVE Type" );
        return -1;
    }

    EditCurveXSec* edit_xs = dynamic_cast<EditCurveXSec*>( xs->GetXSecCurve() );
    assert( edit_xs );

    ErrorMgr.NoError();

    return edit_xs->Split01( u );
}

void MoveEditXSecPnt( const std::string & xsec_id, const int & indx, const vec3d & new_pnt )
{
    XSec* xs = FindXSec( xsec_id );
    if ( !xs )
    {
        ErrorMgr.AddError( VSP_INVALID_PTR, "MoveEditXSecPnt::Can't Find XSec " + xsec_id );
        return;
    }

    if ( xs->GetXSecCurve()->GetType() != XS_EDIT_CURVE )
    {
        ErrorMgr.AddError( VSP_WRONG_XSEC_TYPE, "MoveEditXSecPnt::XSec Not XS_EDIT_CURVE Type" );
        return;
    }

    EditCurveXSec* edit_xs = dynamic_cast<EditCurveXSec*>( xs->GetXSecCurve() );
    assert( edit_xs );

    if( indx < 0 || indx >= edit_xs->GetNumPts() )
    {
        ErrorMgr.AddError( VSP_INDEX_OUT_RANGE, "MoveEditXSecPnt::Invalid point index " + std::to_string( indx ) );
        return;
    }

    // edit_xs->MovePnt also moves adjacent CEDIT points, so just set parm values directly
    edit_xs->m_XParmVec[indx]->Set( new_pnt.x() );
    edit_xs->m_YParmVec[indx]->Set( new_pnt.y() );
    edit_xs->m_ZParmVec[indx]->Set( new_pnt.z() );

    edit_xs->ParmChanged( nullptr, Parm::SET_FROM_DEVICE ); // Force update

    ErrorMgr.NoError();
}

void ConvertXSecToEdit( const std::string & geom_id, const int & indx )
{
    Vehicle* veh = GetVehicle();
    Geom* geom_ptr = veh->FindGeom( geom_id );
    if ( !geom_ptr )
    {
        ErrorMgr.AddError( VSP_INVALID_PTR, "ConvertXSecToEdit::Can't Find Geom " + geom_id );
        return;
    }

    if ( geom_ptr->GetType().m_Type == BOR_GEOM_TYPE )
    {
        BORGeom* bor_ptr = dynamic_cast<BORGeom*> ( geom_ptr );
        if ( !bor_ptr )
        {
            ErrorMgr.AddError( VSP_INVALID_TYPE, "ConvertXSecToEdit::Geom " + geom_id + " is not a body of revolution" );
            return;
        }

        bor_ptr->ConvertToEdit();
        ErrorMgr.NoError();
        return;
    }
    else
    {
        GeomXSec* geom_xsec = dynamic_cast <GeomXSec*> ( geom_ptr );
        if ( !geom_xsec )
        {
            ErrorMgr.AddError( VSP_INVALID_TYPE, "ConvertXSecToEdit::Geom " + geom_id + " is not a GeomXSec" );
            return;
        }

        XSec* xs = geom_xsec->GetXSec( indx );

        if ( !xs )
        {
            ErrorMgr.AddError( VSP_INVALID_PTR, "ConvertXSecToEdit::Can't Find XSec " + to_string( ( long long )indx ) );
            return;
        }

        xs->ConvertToEdit( );
        ErrorMgr.NoError();
        return;
    }
}

vector < bool > GetEditXSecFixedUVec( const std::string& xsec_id )
{
    XSec* xs = FindXSec( xsec_id );
    if ( !xs )
    {
        ErrorMgr.AddError( VSP_INVALID_PTR, "GetEditXSecFixedUVec::Can't Find XSec " + xsec_id );
        return vector < bool > {};
    }

    if ( xs->GetXSecCurve()->GetType() != XS_EDIT_CURVE )
    {
        ErrorMgr.AddError( VSP_WRONG_XSEC_TYPE, "GetEditXSecFixedUVec::XSec Not XS_EDIT_CURVE Type" );
        return vector < bool > {};
    }

    EditCurveXSec* edit_xs = dynamic_cast<EditCurveXSec*>( xs->GetXSecCurve() );
    assert( edit_xs );

    ErrorMgr.NoError();
    return edit_xs->GetFixedUVec();
}

void SetEditXSecFixedUVec( const std::string& xsec_id, vector < bool > fixed_u_vec )
{
    XSec* xs = FindXSec( xsec_id );
    if ( !xs )
    {
        ErrorMgr.AddError( VSP_INVALID_PTR, "SetEditXSecFixedUVec::Can't Find XSec " + xsec_id );
        return;
    }

    if ( xs->GetXSecCurve()->GetType() != XS_EDIT_CURVE )
    {
        ErrorMgr.AddError( VSP_WRONG_XSEC_TYPE, "SetEditXSecFixedUVec::XSec Not XS_EDIT_CURVE Type" );
        return;
    }

    EditCurveXSec* edit_xs = dynamic_cast<EditCurveXSec*>( xs->GetXSecCurve() );
    assert( edit_xs );

    if ( edit_xs->GetNumPts() != fixed_u_vec.size() )
    {
        ErrorMgr.AddError( VSP_INDEX_OUT_RANGE, "SetEditXSecFixedUVec:Size of fixed_u_vec Not Equal to Number of Control Points" );
        return;
    }

    for ( size_t i = 0; i < fixed_u_vec.size(); i++ )
    {
        edit_xs->m_FixedUVec[i]->Set( fixed_u_vec[i] );
    }
    ErrorMgr.NoError();
}

void ReparameterizeEditXSec( const std::string & xsec_id )
{
    XSec* xs = FindXSec( xsec_id );
    if ( !xs )
    {
        ErrorMgr.AddError( VSP_INVALID_PTR, "ReparameterizeEditXSec::Can't Find XSec " + xsec_id );
        return;
    }

    if ( xs->GetXSecCurve()->GetType() != XS_EDIT_CURVE )
    {
        ErrorMgr.AddError( VSP_WRONG_XSEC_TYPE, "ReparameterizeEditXSec::XSec Not XS_EDIT_CURVE Type" );
        return;
    }

    EditCurveXSec* edit_xs = dynamic_cast<EditCurveXSec*>( xs->GetXSecCurve() );
    assert( edit_xs );

    edit_xs->ReparameterizeEqualArcLength();
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
        ErrorMgr.AddError( VSP_CANT_FIND_NAME, "GetGeomSet::Can't Find Name " + name );
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
        ErrorMgr.AddError( VSP_CANT_FIND_NAME, "GetSetIndex::Can't Find Name " + name );
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
    if ( set_index < 0 || set_index >= ( int )veh->GetSetNameVec().size() )
    {
        ErrorMgr.AddError( VSP_INDEX_OUT_RANGE, "SetSetFlag::Invalid Set Index " + to_string( ( long long ) set_index ) );
        return;
    }


    ErrorMgr.NoError();

    geom_ptr->SetSetFlag( set_index, flag );
}

void CopyPasteSet( int copy_index, int paste_index )
{

    Vehicle* veh = GetVehicle();
    if (copy_index < 0 || copy_index >=(int)veh->GetSetNameVec().size())
    {
        ErrorMgr.AddError(VSP_INDEX_OUT_RANGE, "CopyPasteSet::Invalid Copy Index " + to_string((long long)copy_index));
        return;
    }

    if (paste_index < 0 || paste_index >=(int)veh->GetSetNameVec().size())
    {
        ErrorMgr.AddError(VSP_INDEX_OUT_RANGE, "CopyPasteSet::Invalid Paste Index " + to_string((long long)paste_index));
        return;
    }

    if (copy_index > SET_NOT_SHOWN && paste_index > SET_NOT_SHOWN)
    {
        ErrorMgr.NoError();

        veh->CopyPasteSet(copy_index, paste_index);
    }

}

bool GetBBoxSet( int set, double & xmin_out, double & ymin_out, double & zmin_out, double & xlen_out, double & ylen_out, double & zlen_out  )
{
    Vehicle* veh = GetVehicle();
    if ( !veh )
    {
        ErrorMgr.AddError( VSP_INVALID_PTR, "GetBBoxSet::Can't Find Vehicle." );
        xmin_out = 0.0;
        ymin_out = 0.0;
        zmin_out = 0.0;
        xlen_out = 0.0;
        ylen_out = 0.0;
        zlen_out = 0.0;

        return false;
    }

    bool sethasmembers = veh->GetBndBoxSet( set, xmin_out, ymin_out, zmin_out, xlen_out, ylen_out, zlen_out );
    ErrorMgr.NoError();
    return sethasmembers;
}

bool GetScaleIndependentBBoxSet( int set, double & xmin_out, double & ymin_out, double & zmin_out, double & xlen_out, double & ylen_out, double & zlen_out  )
{
    Vehicle* veh = GetVehicle();
    if ( !veh )
    {
        ErrorMgr.AddError( VSP_INVALID_PTR, "GetScaleIndependentBBox::Can't Find Vehicle." );

        xmin_out = 0.0;
        ymin_out = 0.0;
        zmin_out = 0.0;
        xlen_out = 0.0;
        ylen_out = 0.0;
        zlen_out = 0.0;

        return false;
    }

    bool sethasmembers = veh->GetScaleIndependentBBoxSet( set, xmin_out, ymin_out, zmin_out, xlen_out, ylen_out, zlen_out );
    ErrorMgr.NoError();
    return sethasmembers;
}

//================================================================//
//=============== Group Modifications for Sets ===================//
//================================================================//

void ScaleSet( int set_index, double scale )
{
    Vehicle* veh = GetVehicle();
    GroupTransformations* group_trans = veh->GetGroupTransformationsPtr();
    if ( !group_trans )
    {
        ErrorMgr.AddError( VSP_INVALID_PTR, "ScaleSet::Can't Get Group Transformation Pointer" );
        return;
    }

    vector < string > geom_id_vec = veh->GetGeomSet( set_index );

    veh->ClearActiveGeom();
    veh->SetActiveGeomVec( geom_id_vec );
    group_trans->ReInitialize();

    group_trans->m_GroupScale.Set( scale );
    group_trans->ParmChanged( nullptr, Parm::SET_FROM_DEVICE );

    veh->ClearActiveGeom();
    group_trans->ReInitialize();
}

void RotateSet( int set_index, double x_rot_deg, double y_rot_deg, double z_rot_deg )
{
    Vehicle* veh = GetVehicle();
    GroupTransformations* group_trans = veh->GetGroupTransformationsPtr();
    if ( !group_trans )
    {
        ErrorMgr.AddError( VSP_INVALID_PTR, "RotateSet::Can't Get Group Transformation Pointer" );
        return;
    }

    vector < string > geom_id_vec = veh->GetGeomSet( set_index );

    veh->ClearActiveGeom();
    veh->SetActiveGeomVec( geom_id_vec );
    group_trans->ReInitialize();

    group_trans->m_GroupXRot.Set( x_rot_deg );
    group_trans->m_GroupYRot.Set( y_rot_deg );
    group_trans->m_GroupZRot.Set( z_rot_deg );
    group_trans->ParmChanged( nullptr, Parm::SET_FROM_DEVICE );

    veh->ClearActiveGeom();
    group_trans->ReInitialize();
}

void TranslateSet( int set_index, const vec3d &translation_vec )
{
    Vehicle* veh = GetVehicle();
    GroupTransformations* group_trans = veh->GetGroupTransformationsPtr();
    if ( !group_trans )
    {
        ErrorMgr.AddError( VSP_INVALID_PTR, "TranslateSet::Can't Get Group Transformation Pointer" );
        return;
    }

    vector < string > geom_id_vec = veh->GetGeomSet( set_index );

    veh->ClearActiveGeom();
    veh->SetActiveGeomVec( geom_id_vec );
    group_trans->ReInitialize();

    group_trans->m_GroupXLoc.Set( translation_vec.x() );
    group_trans->m_GroupYLoc.Set( translation_vec.y() );
    group_trans->m_GroupZLoc.Set( translation_vec.z() );
    group_trans->ParmChanged( nullptr, Parm::SET_FROM_DEVICE );

    veh->ClearActiveGeom();
    group_trans->ReInitialize();
}

void TransformSet( int set_index, const vec3d &translation_vec, double x_rot_deg, double y_rot_deg, double z_rot_deg, double scale, bool scale_translations_flag )
{
    Vehicle* veh = GetVehicle();
    GroupTransformations* group_trans = veh->GetGroupTransformationsPtr();
    if ( !group_trans )
    {
        ErrorMgr.AddError( VSP_INVALID_PTR, "TransformSet::Can't Get Group Transformation Pointer" );
        return;
    }

    vector < string > geom_id_vec = veh->GetGeomSet( set_index );

    veh->ClearActiveGeom();
    veh->SetActiveGeomVec( geom_id_vec );
    group_trans->ReInitialize();

    group_trans->m_GroupXLoc.Set( translation_vec.x() );
    group_trans->m_GroupYLoc.Set( translation_vec.y() );
    group_trans->m_GroupZLoc.Set( translation_vec.z() );
    group_trans->m_GroupXRot.Set( x_rot_deg );
    group_trans->m_GroupYRot.Set( y_rot_deg );
    group_trans->m_GroupZRot.Set( z_rot_deg );
    group_trans->m_GroupScale.Set( scale );
    group_trans->m_scaleGroupTranslations.Set( scale_translations_flag );
    group_trans->ParmChanged( nullptr, Parm::SET_FROM_DEVICE );

    veh->ClearActiveGeom();
    group_trans->ReInitialize();
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
        ErrorMgr.AddError( VSP_CANT_FIND_PARM, "SetParmVal::Can't Find Parm " + parm_id );
        return val;
    }
    ErrorMgr.NoError();
    return p->Set( val );
}

/// Set the parm value.  If update is true, the parm container is updated.
/// The final value of parm is returned.
double SetParmVal( const string & container_id, const string & name, const string & group, double val )
{
    string parm_id = GetParm( container_id, name, group );
    Parm* p = ParmMgr.FindParm( parm_id );
    if ( !p )
    {
        ErrorMgr.AddError( VSP_CANT_FIND_PARM, "SetParmVal::Can't Find Parm " + container_id + ":" + group + ":" + name  );
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
        ErrorMgr.AddError( VSP_CANT_FIND_PARM, "SetParmValLimits::Can't Find Parm " + parm_id );
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
        ErrorMgr.AddError( VSP_CANT_FIND_PARM, "SetParmValUpdate::Can't Find Parm " + parm_id );
        return val;
    }
    ErrorMgr.NoError();
    return p->SetFromDevice( val );         // Force Update
}


/// Set the parm value.  If update is true, the parm container is updated.
/// The final value of parm is returned.
double SetParmValUpdate( const string & container_id, const string & parm_name, const string & parm_group_name, double val )
{
    string parm_id = GetParm( container_id, parm_name, parm_group_name );
    Parm* p = ParmMgr.FindParm( parm_id );
    if ( !p )
    {
        ErrorMgr.AddError( VSP_CANT_FIND_PARM, "SetParmValUpdate::Can't Find Parm " + container_id + ":" + parm_group_name + ":" + parm_name );
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
        ErrorMgr.AddError( VSP_CANT_FIND_PARM, "GetParmVal::Can't Find Parm " + parm_id );
        return 0.0;
    }
    ErrorMgr.NoError();
    return p->Get();
}

/// Get the value of parm
double GetParmVal( const string & container_id, const string & name, const string & group )
{
    string parm_id = GetParm( container_id, name, group );
    Parm* p = ParmMgr.FindParm( parm_id );
    if ( !p )
    {
        ErrorMgr.AddError( VSP_CANT_FIND_PARM, "GetParmVal::Can't Find Parm " + container_id + ":" + group + ":" + name  );
        return 0.0;
    }
    ErrorMgr.NoError();
    return p->Get();
}

/// Get the value of an int parm
int GetIntParmVal( const string & parm_id )
{
    Parm* p = ParmMgr.FindParm( parm_id );
    if ( !p )
    {
        ErrorMgr.AddError( VSP_CANT_FIND_PARM, "GetIntParmVal::Can't Find Parm " + parm_id );
        return 0;
    }
    ErrorMgr.NoError();
    return (int)(p->Get()+0.5);
}

/// Get the value of a bool parm
bool GetBoolParmVal( const string & parm_id )
{
    Parm* p = ParmMgr.FindParm( parm_id );
    if ( !p )
    {
        ErrorMgr.AddError( VSP_CANT_FIND_PARM, "GetBoolParmVal::Can't Find Parm " + parm_id );
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
        ErrorMgr.AddError( VSP_CANT_FIND_PARM, "SetParmUpperLimit::Can't Find Parm " + parm_id );
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
        ErrorMgr.AddError( VSP_CANT_FIND_PARM, "GetParmUpperLimit::Can't Find Parm " + parm_id );
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
        ErrorMgr.AddError( VSP_CANT_FIND_PARM, "SetParmLowerLimit::Can't Find Parm " + parm_id );
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
        ErrorMgr.AddError( VSP_CANT_FIND_PARM, "GetParmLowerLimit::Can't Find Parm " + parm_id );
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
        ErrorMgr.AddError( VSP_CANT_FIND_PARM, "GetParmType::Can't Find Parm " + parm_id );
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
        ErrorMgr.AddError( VSP_CANT_FIND_PARM, "GetParmName::Can't Find Parm " + parm_id );
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
        ErrorMgr.AddError( VSP_CANT_FIND_PARM, "GetParmGroupName::Can't Find Parm " + parm_id );
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
        ErrorMgr.AddError( VSP_CANT_FIND_PARM, "GetParmDisplayGroupName::Can't Find Parm " + parm_id );
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
        ErrorMgr.AddError( VSP_CANT_FIND_PARM, "GetParmContainer::Can't Find Parm " + parm_id );
        return string();
    }
    ErrorMgr.NoError();
    return p->GetContainerID();
}

/// Set the parm description
void SetParmDescript( const string & parm_id, const string & desc )
{
    Parm* p = ParmMgr.FindParm( parm_id );
    if ( !p )
    {
        ErrorMgr.AddError( VSP_CANT_FIND_PARM, "SetParmDescript::Can't Find Parm " + parm_id );
        return;
    }
    ErrorMgr.NoError();
    return p->SetDescript( desc );
}

/// Get the parm description
string GetParmDescript( const string & parm_id )
{
    Parm* p = ParmMgr.FindParm( parm_id );
    if ( !p )
    {
        ErrorMgr.AddError( VSP_CANT_FIND_PARM, "GetParmDescript::Can't Find Parm " + parm_id );
        return string();
    }
    ErrorMgr.NoError();
    return p->GetDescript();
}

///  Find a parm id given parm container, name and group
string FindParm( const string & parm_container_id, const string& parm_name, const string& group_name )
{
    if ( ParmMgr.GetDirtyFlag() )
    {
        LinkMgr.BuildLinkableParmData();        // Make Sure Name/Group Get Mapped To Parms
    }

    ParmContainer* pc = ParmMgr.FindParmContainer( parm_container_id );

    if ( !pc )
    {
        ErrorMgr.AddError( VSP_INVALID_ID, "FindParm::Can't Find Parm Container " + parm_container_id );
        return string();
    }

   string parm_id = pc->FindParm( parm_name, group_name );
   Parm* p = ParmMgr.FindParm( parm_id );
   if ( !p )
   {
        ErrorMgr.AddError( VSP_CANT_FIND_PARM, "FindParm::Can't Find Parm " + parm_name + " " + group_name );
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
    if ( ParmMgr.GetDirtyFlag() )
    {
        LinkMgr.BuildLinkableParmData();
    }
    LinkMgr.GetAllContainerVec( containerVec );

    ErrorMgr.NoError();
    return containerVec;
}

vector< std::string > FindContainersWithName( const string & name )
{
    vector< string > containerVec;
    vector< string > ret_vec;
    if ( ParmMgr.GetDirtyFlag() )
    {
        LinkMgr.BuildLinkableParmData();
    }
    LinkMgr.GetAllContainerVec( containerVec );

    for ( int i = 0 ; i < ( int )containerVec.size() ; i++ )
    {
        ParmContainer* pc = ParmMgr.FindParmContainer( containerVec[i] );

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
    if ( ParmMgr.GetDirtyFlag() )
    {
        LinkMgr.BuildLinkableParmData();
    }
    LinkMgr.GetAllContainerVec( containerVec );

    for ( int i = 0 ; i < ( int )containerVec.size() ; i++ )
    {
        ParmContainer* pc = ParmMgr.FindParmContainer( containerVec[i] );

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
        ErrorMgr.AddError( VSP_INVALID_ID, "GetContainerName::Can't Find Parm Container " + parm_container_id );
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
        ErrorMgr.AddError( VSP_INVALID_ID, "FindContainerGroupNames::Can't Find Parm Container " + parm_container_id );
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
        ErrorMgr.AddError( VSP_INVALID_ID, "FindContainerParmIDs::Can't Find Parm Container " + parm_container_id );
        return parm_vec;
    }

    pc->AddLinkableParms( parm_vec );

    ErrorMgr.NoError();
    return parm_vec;
}

string GetVehicleID()
{
    Vehicle* veh = GetVehicle();

    ErrorMgr.NoError();
    return veh->GetID();
}

//===================================================================//
//===============         User Parm  Functions         ==============//
//===================================================================//

int GetNumUserParms()
{
    int nup = LinkMgr.GetNumUserParms();

    ErrorMgr.NoError();

    return nup;
}

int GetNumPredefinedUserParms()
{
    int npdup = LinkMgr.GetNumPredefinedUserParms();

    ErrorMgr.NoError();

    return npdup;
}

vector < std::string > GetAllUserParms()
{
    vector < string > ret = LinkMgr.GetAllUserParms();

    ErrorMgr.NoError();

    return ret;
}

string GetUserParmContainer()
{
    ParmContainer *pc = LinkMgr.GetUserParmContainer();

    if ( !pc )
    {
        ErrorMgr.AddError( VSP_INVALID_PTR, "GetUserParmContainer::Invalid UserParmContainer " );
        return string();
    }

    ErrorMgr.NoError();

    return pc->GetID();
}

string AddUserParm(int type, const string & name, const string & group )
{
    string ret = LinkMgr.AddUserParm( type, name, group );

    ErrorMgr.NoError();

    return ret;
}

void DeleteUserParm( const std::string & id )
{
    int indx = LinkMgr.GetUserParmIndex( id );

    if ( indx < 0 )
    {
        ErrorMgr.AddError( VSP_CANT_FIND_PARM, "DeleteUserParm::Invalid UserParm ID " );
        return;
    }

    LinkMgr.DeleteUserParm( indx );

    ErrorMgr.NoError();
}

void DeleteAllUserParm()
{
    LinkMgr.DeleteAllUserParm();

    ErrorMgr.NoError();
}

//===================================================================//
//===============           Snap To Functions          ==============//
//===================================================================//
double ComputeMinClearanceDistance( const string & geom_id, int set, bool useMode, const string &modeID )
{
    Vehicle* veh = GetVehicle();

    int old_set = veh->GetSnapToPtr()->m_CollisionSet;
    veh->GetSnapToPtr()->m_CollisionSet = set;

    bool old_useMode = veh->GetSnapToPtr()->m_UseMode();
    veh->GetSnapToPtr()->m_UseMode = useMode;

    string old_modeID = veh->GetSnapToPtr()->m_ModeID;
    veh->GetSnapToPtr()->m_ModeID = modeID;

    vector< string > old_active_geom = veh->GetActiveGeomVec();
    veh->SetActiveGeom( geom_id );

    veh->GetSnapToPtr()->CheckClearance();
    double min_clearance_dist = veh->GetSnapToPtr()->m_CollisionMinDist;

    //==== Restore State ====//
    veh->GetSnapToPtr()->m_CollisionSet = old_set;
    veh->GetSnapToPtr()->m_UseMode = old_useMode;
    veh->GetSnapToPtr()->m_ModeID = old_modeID;
    veh->SetActiveGeomVec( old_active_geom );

    return min_clearance_dist;
}

double SnapParm( const string & parm_id, double target_min_dist, bool inc_flag, int set, bool useMode, const string &modeID )
{
    Vehicle* veh = GetVehicle();

    int old_set = veh->GetSnapToPtr()->m_CollisionSet;
    veh->GetSnapToPtr()->m_CollisionSet = set;

    bool old_useMode = veh->GetSnapToPtr()->m_UseMode();
    veh->GetSnapToPtr()->m_UseMode = useMode;

    string old_modeID = veh->GetSnapToPtr()->m_ModeID;
    veh->GetSnapToPtr()->m_ModeID = modeID;

    double old_min_dist = veh->GetSnapToPtr()->m_CollisionTargetDist();
    veh->GetSnapToPtr()->m_CollisionTargetDist = target_min_dist;

    veh->GetSnapToPtr()->AdjParmToMinDist( parm_id, inc_flag );
    double min_clearance_dist = veh->GetSnapToPtr()->m_CollisionMinDist;

    //==== Restore State ====//
    veh->GetSnapToPtr()->m_CollisionSet = old_set;
    veh->GetSnapToPtr()->m_UseMode = old_useMode;
    veh->GetSnapToPtr()->m_ModeID = old_modeID;
    veh->GetSnapToPtr()->m_CollisionTargetDist = old_min_dist;

    return min_clearance_dist;
}

//===================================================================//
//===============     Variable Presets Functions       ==============//
//===================================================================//

string AddVarPresetGroup( const std::string &group_name )
{
    SettingGroup *sg = new SettingGroup();
    sg->SetName( group_name );

    if ( VarPresetMgr.AddSettingGroup( sg ) )
    {
        ErrorMgr.NoError();
        return sg->GetID();
    }

    delete sg;

    ErrorMgr.AddError( VSP_INVALID_ID, "AddVarPresetGroup::Unable to create var preset group." );
    return string();
}

string AddVarPresetSetting( const std::string &group_id, const std::string &setting_name )
{
    SettingGroup *sg = VarPresetMgr.FindSettingGroup( group_id );

    if ( sg )
    {
        Setting *s = new Setting();
        s->SetName( setting_name );

        if ( sg->AddSetting( s, true ) )
        {
            ErrorMgr.NoError();
            return s->GetID();
        }

        delete s;
    }

    ErrorMgr.AddError( VSP_INVALID_ID, "AddVarPresetSetting::Unable to create var preset setting." );
    return string();
}

void AddVarPresetParm( const std::string &group_id, const std::string &parm_id )
{
    SettingGroup *sg = VarPresetMgr.FindSettingGroup( group_id );

    if ( sg )
    {
        if ( sg->AddParm( parm_id ) )
        {
            ErrorMgr.NoError();
            return;
        }
        ErrorMgr.AddError( VSP_CANT_FIND_PARM, "AddVarPresetParm::Unable to add parm to var preset group." );
        return;
    }

    ErrorMgr.AddError( VSP_INVALID_ID, "AddVarPresetParm::Unable to find var preset group." );
}

void DeleteVarPresetGroup( const std::string &group_id )
{
    VarPresetMgr.DeleteSettingGroup( group_id );
    ErrorMgr.NoError();
}

void DeleteVarPresetSetting( const std::string &group_id, const std::string &setting_id )
{
    VarPresetMgr.DeleteSetting( group_id, setting_id );
    ErrorMgr.NoError();
}

void DeleteVarPresetParm( const std::string &group_id, const std::string &parm_id )
{
    SettingGroup *sg = VarPresetMgr.FindSettingGroup( group_id );

    if ( sg )
    {
        sg->RemoveParm( parm_id );
        ErrorMgr.NoError();
        return;
    }
    ErrorMgr.AddError( VSP_INVALID_ID, "DeleteVarPresetParm::Unable to find var preset group." );
}

void SetVarPresetParmVal( const std::string &group_id, const std::string &setting_id, const std::string &parm_id, double parm_val )
{
    SettingGroup *sg = VarPresetMgr.FindSettingGroup( group_id );

    if ( sg )
    {
        vector < string > parm_vec = sg->GetParmIDVec();

        int index = vector_find_val( parm_vec, parm_id );

        if ( index >= 0 )
        {
            Setting *s = VarPresetMgr.FindSetting( setting_id );
            if ( s )
            {
                s->SetParmVal( index, parm_val );
                ErrorMgr.NoError();
                return;
            }
        }

        ErrorMgr.AddError( VSP_CANT_FIND_PARM, "SetVarPresetParmVal::Unable to find Parm in var preset group." );
        return;
    }

    ErrorMgr.AddError( VSP_INVALID_ID, "SetVarPresetParmVal::Unable to find var preset group." );
}

double GetVarPresetParmVal( const std::string &group_id, const std::string &setting_id, const std::string &parm_id )
{
    SettingGroup *sg = VarPresetMgr.FindSettingGroup( group_id );

    if ( sg )
    {
        vector < string > parm_vec = sg->GetParmIDVec();

        int index = vector_find_val( parm_vec, parm_id );

        if ( index >= 0 )
        {
            Setting *s = VarPresetMgr.FindSetting( setting_id );
            if ( s )
            {
                ErrorMgr.NoError();
                return s->GetParmVal( index );
            }
            ErrorMgr.AddError( VSP_INVALID_ID, "GetVarPresetParmVal::Unable to find var preset setting." );
            return 0.0;
        }

        ErrorMgr.AddError( VSP_CANT_FIND_PARM, "GetVarPresetParmVal::Unable to find Parm in var preset group." );
        return 0.0;
    }

    ErrorMgr.AddError( VSP_INVALID_ID, "GetVarPresetParmVal::Unable to find var preset group." );
    return 0.0;
}

std::string GetGroupName( const std::string &group_id )
{
    SettingGroup *sg = VarPresetMgr.FindSettingGroup( group_id );

    if ( sg )
    {
        ErrorMgr.NoError();
        return sg->GetName();
    }
    ErrorMgr.AddError( VSP_INVALID_ID, "GetGroupName::Unable to find var preset group." );
    return string();
}

std::string GetSettingName( const std::string &setting_id )
{
    Setting *s = VarPresetMgr.FindSetting( setting_id );
    if ( s )
    {
        ErrorMgr.NoError();
        return s->GetName();
    }
    ErrorMgr.AddError( VSP_INVALID_ID, "GetSettingName::Unable to find var preset setting." );
    return string();
}

void SetGroupName( const std::string &group_id, const std::string &group_name )
{
    SettingGroup *sg = VarPresetMgr.FindSettingGroup( group_id );

    if ( sg )
    {
        sg->SetName( group_name );
        ErrorMgr.NoError();
        return;
    }
    ErrorMgr.AddError( VSP_INVALID_ID, "SetGroupName::Unable to find var preset group." );
}

void SetSettingName( const std::string &setting_id, const std::string &setting_name )
{
    Setting *s = VarPresetMgr.FindSetting( setting_id );
    if ( s )
    {
        s->SetName( setting_name );
        ErrorMgr.NoError();
        return;
    }
    ErrorMgr.AddError( VSP_INVALID_ID, "SetSettingName::Unable to find var preset setting." );
    return;
}

std::vector< std::string > GetVarPresetGroups()
{
    ErrorMgr.NoError();
    return VarPresetMgr.GetAllSettingGroups();
}

std::vector< std::string > GetVarPresetSettings( const std::string &group_id )
{
    SettingGroup *sg = VarPresetMgr.FindSettingGroup( group_id );

    if ( sg )
    {
        ErrorMgr.NoError();
        return sg->GetSettingIDVec();
    }

    ErrorMgr.AddError( VSP_INVALID_ID, "GetVarPresetSettings::Unable to find var preset group." );
    vector < string > ret;
    return ret;
}

std::vector< std::string > GetVarPresetParmIDs( const std::string &group_id )
{
    SettingGroup *sg = VarPresetMgr.FindSettingGroup( group_id );

    if ( sg )
    {
        ErrorMgr.NoError();
        return sg->GetParmIDVec();
    }

    ErrorMgr.AddError( VSP_INVALID_ID, "GetVarPresetParmIDs::Unable to find var preset group." );
    vector < string > ret;
    return ret;
}

std::vector< double > GetVarPresetParmVals( const std::string &setting_id )
{
    Setting *s = VarPresetMgr.FindSetting( setting_id );
    if ( s )
    {
        ErrorMgr.NoError();
        return s->GetParmValVec();
    }
    ErrorMgr.AddError( VSP_INVALID_ID, "GetVarPresetParmVals::Unable to find var preset setting." );
    vector < double > ret;
    return ret;
}

void SetVarPresetParmVals( const std::string &setting_id, const std::vector< double > &parm_vals )
{
    Setting *s = VarPresetMgr.FindSetting( setting_id );
    if ( s )
    {
        s->SetParmValVec( parm_vals );
        ErrorMgr.NoError();
        return;
    }
    ErrorMgr.AddError( VSP_INVALID_ID, "SetVarPresetParmVals::Unable to find var preset setting." );
}

void SaveVarPresetParmVals( const std::string &group_id, const std::string &setting_id )
{
    SettingGroup *sg = VarPresetMgr.FindSettingGroup( group_id );

    if ( sg )
    {
        sg->SaveSetting( setting_id );
        ErrorMgr.NoError();
        return;
    }
    ErrorMgr.AddError( VSP_INVALID_ID, "SaveVarPresetParmVals::Unable to find var preset group." );
}

void ApplyVarPresetSetting( const std::string &group_id, const std::string &setting_id )
{
    SettingGroup *sg = VarPresetMgr.FindSettingGroup( group_id );

    if ( sg )
    {
        sg->ApplySetting( setting_id );
        ErrorMgr.NoError();
        return;
    }
    ErrorMgr.AddError( VSP_INVALID_ID, "ApplyVarPresetSetting::Unable to find var preset group." );
}

//===================================================================//
//=======================     Mode Functions       ==================//
//===================================================================//

string CreateAndAddMode( const string & name, int normal_set, int degen_set )
{
    Mode *mod = ModeMgr.CreateAndAddMode();
    if ( mod )
    {
        mod->SetName( name );
        mod->m_NormalSet = normal_set;
        mod->m_DegenSet = degen_set;

        ErrorMgr.NoError();
        return mod->GetID();
    }

    ErrorMgr.AddError( VSP_INVALID_PTR, "CreateAndAddMode::Unable to create mode." );

    return string();
}

int GetNumModes()
{
    ErrorMgr.NoError();
    return ModeMgr.GetNumModes();
}

vector < string > GetAllModes()
{
    ErrorMgr.NoError();
    return ModeMgr.GetAllModes();
}

void DelMode( const string &mid )
{
    ErrorMgr.NoError();
    ModeMgr.DelMode( mid );
}

void DelAllModes()
{
    ErrorMgr.NoError();
    ModeMgr.DelAllModes();
}

void ApplyModeSettings( const string &mid )
{
    Mode *mod = ModeMgr.GetMode( mid );

    if ( !mod )
    {
        ErrorMgr.AddError( VSP_INVALID_ID, "ApplyModeSettings::Could not find mode." );
    }

    mod->ApplySettings();
    ErrorMgr.NoError();
}

void ShowOnlyMode( const string &mid )
{
    Mode *mod = ModeMgr.GetMode( mid );

    if ( !mod )
    {
        ErrorMgr.AddError( VSP_INVALID_ID, "ShowOnlyMode::Could not find mode." );
    }

    mod->ShowOnly();
    ErrorMgr.NoError();
}

void ModeAddGroupSetting( const string &mid, const string &gid, const string &sid )
{
    Mode *mod = ModeMgr.GetMode( mid );

    if ( !mod )
    {
        ErrorMgr.AddError( VSP_INVALID_ID, "ModeAddGroupSetting::Could not find mode." );
    }

    mod->AddSetting( pair < string, string > ( gid, sid ) );
    ErrorMgr.NoError();
}

string ModeGetGroup( const string &mid, int indx )
{
    Mode *mod = ModeMgr.GetMode( mid );

    if ( !mod )
    {
        ErrorMgr.AddError( VSP_INVALID_ID, "ModeGetGroup::Could not find mode." );
        return string();
    }

    pair < string, string > set = mod->GetSetting( indx );
    ErrorMgr.NoError();
    return set.first;
}

string ModeGetSetting( const string &mid, int indx )
{
    Mode *mod = ModeMgr.GetMode( mid );

    if ( !mod )
    {
        ErrorMgr.AddError( VSP_INVALID_ID, "ModeGetSetting::Could not find mode." );
        return string();
    }

    pair < string, string > set = mod->GetSetting( indx );
    ErrorMgr.NoError();
    return set.second;
}

vector < string > ModeGetAllGroups( const string &mid )
{
    vector < string > ret;
    Mode *mod = ModeMgr.GetMode( mid );

    if ( !mod )
    {
        ErrorMgr.AddError( VSP_INVALID_ID, "ModeGetAllGroups::Could not find mode." );
        return ret;
    }

    ret = mod->GetAllGroups();
    ErrorMgr.NoError();
    return ret;
}

vector < string > ModeGetAllSettings( const string &mid )
{
    vector < string > ret;
    Mode *mod = ModeMgr.GetMode( mid );

    if ( !mod )
    {
        ErrorMgr.AddError( VSP_INVALID_ID, "ModeGetAllSettings::Could not find mode." );
        return ret;
    }

    ret = mod->GetAllSettings();
    ErrorMgr.NoError();
    return ret;
}

void RemoveGroupSetting( const string &mid, int indx )
{
    Mode *mod = ModeMgr.GetMode( mid );

    if ( !mod )
    {
        ErrorMgr.AddError( VSP_INVALID_ID, "RemoveGroupSetting::Could not find mode." );
        return;
    }

    mod->RemoveSetting( indx );
    ErrorMgr.NoError();
}

void RemoveAllGroupSettings( const string &mid )
{
    Mode *mod = ModeMgr.GetMode( mid );

    if ( !mod )
    {
        ErrorMgr.AddError( VSP_INVALID_ID, "RemoveAllGroupSettings::Could not find mode." );
        return;
    }

    mod->RemoveAllSettings();
    ErrorMgr.NoError();
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
    else if ( geom_ptr->GetType().m_Type != PROP_GEOM_TYPE )
    {
        ErrorMgr.AddError( VSP_INVALID_PTR, "SetPCurve::Geom doesn't support PCurves " + geom_id );
        return;
    }

    PropGeom* prop_ptr = dynamic_cast < PropGeom* > (geom_ptr );
    PCurve *pc = nullptr;

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
    else if ( geom_ptr->GetType().m_Type != PROP_GEOM_TYPE )
    {
        ErrorMgr.AddError( VSP_INVALID_PTR, "PCurveConvertTo::Geom doesn't support PCurves " + geom_id );
        return;
    }

    PropGeom* prop_ptr = dynamic_cast < PropGeom* > (geom_ptr );
    PCurve *pc = nullptr;

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
    else if ( geom_ptr->GetType().m_Type != PROP_GEOM_TYPE )
    {
        ErrorMgr.AddError( VSP_INVALID_PTR, "PCurveGetType::Geom doesn't support PCurves " + geom_id );
        return -1;
    }

    PropGeom* prop_ptr = dynamic_cast < PropGeom* > (geom_ptr );
    PCurve *pc = nullptr;

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
    else if ( geom_ptr->GetType().m_Type != PROP_GEOM_TYPE )
    {
        ErrorMgr.AddError( VSP_INVALID_PTR, "PCurveGetTVec::Geom doesn't support PCurves " + geom_id );
        return retvec;
    }

    PropGeom* prop_ptr = dynamic_cast < PropGeom* > (geom_ptr );
    PCurve *pc = nullptr;

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
    else if ( geom_ptr->GetType().m_Type != PROP_GEOM_TYPE )
    {
        ErrorMgr.AddError( VSP_INVALID_PTR, "PCurveGetValVec::Geom doesn't support PCurves " + geom_id );
        return retvec;
    }

    PropGeom* prop_ptr = dynamic_cast < PropGeom* > (geom_ptr );
    PCurve *pc = nullptr;

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
    else if ( geom_ptr->GetType().m_Type != PROP_GEOM_TYPE )
    {
        ErrorMgr.AddError( VSP_INVALID_PTR, "PCurveDeletePt::Geom doesn't support PCurves " + geom_id );
        return;
    }

    PropGeom* prop_ptr = dynamic_cast < PropGeom* > (geom_ptr );
    PCurve *pc = nullptr;

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

int PCurveSplit( const string & geom_id, const int & pcurveid, const double & tsplit )
{
    Vehicle* veh = GetVehicle();
    Geom* geom_ptr = veh->FindGeom( geom_id );
    if ( !geom_ptr )
    {
        ErrorMgr.AddError( VSP_INVALID_PTR, "PCurveSplit::Can't Find Geom " + geom_id );
        return -1;
    }
    else if ( geom_ptr->GetType().m_Type != PROP_GEOM_TYPE )
    {
        ErrorMgr.AddError( VSP_INVALID_PTR, "PCurveSplit::Geom doesn't support PCurves " + geom_id );
        return -1;
    }

    PropGeom* prop_ptr = dynamic_cast < PropGeom* > (geom_ptr );
    PCurve *pc = nullptr;

    if ( prop_ptr )
    {
        pc = prop_ptr->GetPCurve( pcurveid );
    }

    if ( !pc )
    {
        ErrorMgr.AddError( VSP_INVALID_ID, "PCurveSplit::PCurve not found " + geom_id + " " + to_string( pcurveid ) );
        return -1;
    }

    return pc->Split( tsplit );

    ErrorMgr.NoError();
}

void ApproximateAllPropellerPCurves( const std::string & geom_id )
{
    Vehicle* veh = GetVehicle();
    Geom* geom_ptr = veh->FindGeom( geom_id );
    if ( !geom_ptr )
    {
        ErrorMgr.AddError( VSP_INVALID_PTR, "ApproximateAllPropellerPCurves::Can't Find Geom " + geom_id );
        return;
    }
    else if ( geom_ptr->GetType().m_Type != PROP_GEOM_TYPE )
    {
        ErrorMgr.AddError( VSP_INVALID_PTR, "ApproximateAllPropellerPCurves::Geom not a propeller " + geom_id );
        return;
    }

    PropGeom* prop_ptr = dynamic_cast < PropGeom* > (geom_ptr );

    if ( prop_ptr )
    {
        prop_ptr->ApproxCubicAllPCurves();
    }
    else
    {
        ErrorMgr.AddError( VSP_INVALID_PTR, "ApproximateAllPropellerPCurves::Geom not a propeller " + geom_id );
        return;
    }

    ErrorMgr.NoError();
}

void ResetPropellerThicknessCurve( const std::string & geom_id )
{
    Vehicle* veh = GetVehicle();
    Geom* geom_ptr = veh->FindGeom( geom_id );
    if ( !geom_ptr )
    {
        ErrorMgr.AddError( VSP_INVALID_PTR, "ResetPropellerThicknessCurve::Can't Find Geom " + geom_id );
        return;
    }
    else if ( geom_ptr->GetType().m_Type != PROP_GEOM_TYPE )
    {
        ErrorMgr.AddError( VSP_INVALID_PTR, "ResetPropellerThicknessCurve::Geom not a propeller " + geom_id );
        return;
    }

    PropGeom* prop_ptr = dynamic_cast < PropGeom* > (geom_ptr );

    if ( prop_ptr )
    {
        prop_ptr->ResetThickness();
    }
    else
    {
        ErrorMgr.AddError( VSP_INVALID_PTR, "ResetPropellerThicknessCurve::Geom not a propeller " + geom_id );
        return;
    }

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

void UpdateParasiteDrag()
{
    ParasiteDragMgr.Update();

    ErrorMgr.NoError();
}

void WriteAtmosphereCSVFile(const std::string & file_name, const int &atmos_type)
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
    Results* res = ResultsMgr.CreateResults( "Atmosphere", "Standard atmosphere verification results." );
    res->Add( new NameValData( "Alt", AltTestPoints, "Altitude." ) );
    res->Add( new NameValData( "Temp", temp_vec, "Temperature." ) );
    res->Add( new NameValData( "Pres", pres_vec, "Pressure." ) );
    res->Add( new NameValData( "Pres_Ratio", pres_ratio_vec, "Pressure ratio." ) );
    res->Add( new NameValData( "Rho_Ratio", rho_ratio_vec, "Density ratio." ) );
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
    Results* res = ResultsMgr.CreateResults("Body_Form_Factor", "Body form factor verification results.");
    char str[256];
    vector < double > body_ff_vec;
    vector < double > dol_array = linspace( 0.0, 0.3, 200 );
    res->Add( new NameValData( "D_L", dol_array, "D/L fineness ratio." ) );

    for (size_t body_ff_case = 0; body_ff_case <= vsp::FF_B_JENKINSON_AFT_FUSE_NACELLE; ++body_ff_case )
    {
        for (size_t j = 0; j < dol_array.size(); ++j )
        {
            body_ff_vec.push_back( ParasiteDragMgr.CalcFFBody( 1.0/dol_array[j], body_ff_case ) );
        }
        snprintf( str, sizeof( str ), "%s", ParasiteDragMgr.AssignFFBodyEqnName( body_ff_case ).c_str() );
        res->Add( new NameValData( str, body_ff_vec, "Form factor." ) );
        body_ff_vec.clear();
    }
    res->WriteCSVFile( file_name );
}

void WriteWingFFCSVFile(const std::string & file_name)
{
    Results* res = ResultsMgr.CreateResults("Wing_Form_Factor", "Wing form factor verification results.");
    char str[256];
    vector < double > wing_ff_vec;
    vector < double > toc_array = linspace( 0.0, 0.205, 200 );
    vector < double > perc_lam, sweep25, sweep50;
    perc_lam.push_back(0.0);
    sweep25.push_back(30.0 * M_PI / 180.0);
    sweep50.push_back(30.0 * M_PI / 180.0);
    ParasiteDragMgr.m_Atmos.SetMach(0.8);
    res->Add( new NameValData( "T_C", toc_array, "Thickness to chord ratios" ) );
    for (size_t wing_ff_case = 0; wing_ff_case < vsp::FF_W_SCHEMENSKY_SUPERCRITICAL_AF; ++wing_ff_case )
    {
        for (size_t j = 0; j < toc_array.size(); ++j )
        {
            wing_ff_vec.push_back( ParasiteDragMgr.CalcFFWing( toc_array[j], wing_ff_case, perc_lam[0], sweep25[0], sweep50[0]) );
        }
        snprintf( str, sizeof( str ), "%s", ParasiteDragMgr.AssignFFWingEqnName( wing_ff_case ).c_str() );
        res->Add( new NameValData( str, wing_ff_vec, "Form factor." ) );
        wing_ff_vec.clear();
    }
    res->WriteCSVFile( file_name );
}

void WriteCfEqnCSVFile(const std::string & file_name)
{
    Results* res = ResultsMgr.CreateResults("Friction_Coefficient", "Friction coefficient verification results.");
    char str[256];
    vector < double > lam_cf_vec, turb_cf_vec, ref_leng;
    vector < double > ReyIn_array = logspace( 3, 10, 500 );
    vector < double > roughness, gamma, taw_tw_ratio, te_tw_ratio;
    roughness.push_back(0.01);
    gamma.push_back(1.4);
    taw_tw_ratio.push_back(1.0);
    te_tw_ratio.push_back(1.0);
    ref_leng.push_back(1.0);

    for (size_t cf_case = 0; cf_case <= vsp::DO_NOT_USE_CF_TURB_WHITE_CHRISTOPH_COMPRESSIBLE; ++cf_case )
    {
        if ( !ParasiteDragMgr.IsTurbBlacklisted(cf_case ) ) {
            for ( size_t j = 0; j < ReyIn_array.size(); ++j )
            {
                turb_cf_vec.push_back( ParasiteDragMgr.CalcTurbCf( ReyIn_array[j], ref_leng[0], cf_case, roughness[0], gamma[0], taw_tw_ratio[0], te_tw_ratio[0]) );
            }
            snprintf( str, sizeof( str ), "%s", ParasiteDragMgr.AssignTurbCfEqnName( cf_case ).c_str() );
            res->Add( new NameValData( str, turb_cf_vec, "Turbulent skin friction coefficient." ) );
            turb_cf_vec.clear();
        }
    }

    for (size_t cf_case = 0; cf_case < vsp::CF_LAM_BLASIUS_W_HEAT; ++cf_case)
    {
        for (size_t i = 0; i < ReyIn_array.size(); ++i)
        {
            lam_cf_vec.push_back(ParasiteDragMgr.CalcLamCf(ReyIn_array[i], cf_case));
        }
        snprintf( str, sizeof( str ), "%s", ParasiteDragMgr.AssignLamCfEqnName( cf_case ).c_str() );
        res->Add( new NameValData( str, lam_cf_vec, "Laminar skin friction coefficient." ) );
        lam_cf_vec.clear();
    }

    res->Add( new NameValData( "ReyIn", ReyIn_array, "Reynolds number." ) );
    res->Add( new NameValData( "Ref_Leng", ref_leng, "Reference length." ) );
    res->WriteCSVFile( file_name );
}

void WritePartialCfMethodCSVFile(const std::string & file_name)
{
    Results* res = ResultsMgr.CreateResults("Friction_Coefficient", "Partial turbulence coefficient verification results.");
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

    res->Add( new NameValData( "LamPerc", lam_perc_array, "Percent laminar flow." ) );
    res->Add( new NameValData( "Cf", cf_vec, "Skin friction coefficient." ) );
    res->Add( new NameValData( "ReyIn", ReyIn_array, "Reynolds number." ) );
    res->Add( new NameValData( "Ref_Leng", ref_leng, "Reference length." ) );
    res->Add( new NameValData( "Re/L", reql_array, "Reynolds number per length." ) );
    res->Add( new NameValData( "Roughness", roughness, "Surface roughness." ) );
    res->Add( new NameValData( "Taw/Tw", taw_tw_ratio, "Taw/Tw temperature ratio." ) );
    res->Add( new NameValData( "Te/Tw", te_tw_ratio, "Te/Tw temperature ratio." ) );
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

    const VspSurf *surf = geom_ptr->GetSurfPtr( surf_indx );
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

    const VspSurf *surf = geom_ptr->GetSurfPtr( surf_indx );
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

    const VspSurf *surf = geom_ptr->GetSurfPtr( surf_indx );
    ret = surf->CompTanW01( clamp( u, 0.0, 1.0 ), clamp( w, 0.0, 1.0 ) );

    ErrorMgr.NoError();
    return ret;
}

void CompCurvature01(const std::string &geom_id, const int &surf_indx, const double &u, const double &w, double &k1_out,
                     double &k2_out, double &ka_out, double &kg_out )
{
    Vehicle* veh = GetVehicle();
    Geom* geom_ptr = veh->FindGeom( geom_id );

    k1_out = 0.0;
    k2_out = 0.0;
    ka_out = 0.0;
    kg_out = 0.0;

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

    const VspSurf *surf = geom_ptr->GetSurfPtr( surf_indx );
    surf->CompCurvature01( clamp( u, 0.0, 1.0 ), clamp( w, 0.0, 1.0 ), k1_out, k2_out, ka_out, kg_out );

    ErrorMgr.NoError();
}

double ProjPnt01(const std::string &geom_id, const int &surf_indx, const vec3d &pt, double &u_out, double &w_out)
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

    dmin = geom->GetSurfPtr( surf_indx )->FindNearest01( u_out, w_out, pt );

    ErrorMgr.NoError();

    return dmin;
}

double ProjPnt01I(const std::string &geom_id, const vec3d &pt, int &surf_indx,
                  double &u_out, double &w_out)
{
    Vehicle* vPtr = VehicleMgr.GetVehicle();
    Geom * geom = vPtr->FindGeom( geom_id );

    double dmin = std::numeric_limits<double>::max();

    if ( !geom )
    {
        ErrorMgr.AddError( VSP_INVALID_GEOM_ID, "ProjPnt01I::Can't Find Geom " + geom_id );
        return dmin;
    }

    dmin = geom->ProjPnt01I( pt, surf_indx, u_out, w_out );

    ErrorMgr.NoError();

    return dmin;
}

double ProjPnt01Guess(const std::string &geom_id, const int &surf_indx, const vec3d &pt, const double &u0, const double &w0, double &u_out, double &w_out)
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

    dmin = geom->GetSurfPtr( surf_indx )->FindNearest01( u_out, w_out, pt, clamp( u0, 0.0, 1.0 ), clamp( w0, 0.0, 1.0 ) );

    ErrorMgr.NoError();

    return dmin;
}

double AxisProjPnt01(const std::string &geom_id, const int &surf_indx, const int &iaxis, const vec3d &pt, double &u_out, double &w_out)
{
    Vehicle* vPtr = VehicleMgr.GetVehicle();
    Geom * geom = vPtr->FindGeom( geom_id );

    double idmin = -1;

    if ( !geom )
    {
        ErrorMgr.AddError( VSP_INVALID_GEOM_ID, "AxisProjPnt01::Can't Find Geom " + geom_id );
        return idmin;
    }

    if ( surf_indx < 0 || surf_indx >= geom->GetNumTotalSurfs() )
    {
        ErrorMgr.AddError( VSP_INDEX_OUT_RANGE, "AxisProjPnt01::Invalid Surface Index " + to_string( surf_indx ) );
        return idmin;
    }

    idmin = geom->GetSurfPtr( surf_indx )->ProjectPt01( pt, iaxis, u_out, w_out );

    ErrorMgr.NoError();

    return idmin;
}

double AxisProjPnt01I(const std::string &geom_id, const int &iaxis, const vec3d &pt, int &surf_indx_out, double &u_out, double &w_out)
{
    Vehicle* vPtr = VehicleMgr.GetVehicle();
    Geom * geom = vPtr->FindGeom( geom_id );

    double idmin = std::numeric_limits<double>::max();

    if ( !geom )
    {
        ErrorMgr.AddError( VSP_INVALID_GEOM_ID, "AxisProjPnt01I::Can't Find Geom " + geom_id );
        return idmin;
    }

    idmin = geom->AxisProjPnt01I( iaxis, pt, surf_indx_out, u_out, w_out );

    ErrorMgr.NoError();

    return idmin;
}

double AxisProjPnt01Guess(const std::string &geom_id, const int &surf_indx, const int &iaxis, const vec3d &pt, const double &u0, const double &w0, double &u_out, double &w_out)
{
    Vehicle* vPtr = VehicleMgr.GetVehicle();
    Geom * geom = vPtr->FindGeom( geom_id );

    double idmin = std::numeric_limits<double>::max();

    if ( !geom )
    {
        ErrorMgr.AddError( VSP_INVALID_GEOM_ID, "AxisProjPnt01Guess::Can't Find Geom " + geom_id );
        return idmin;
    }

    if ( surf_indx < 0 || surf_indx >= geom->GetNumTotalSurfs() )
    {
        ErrorMgr.AddError( VSP_INDEX_OUT_RANGE, "AxisProjPnt01Guess::Invalid Surface Index " + to_string( surf_indx ) );
        return idmin;
    }

    idmin = geom->GetSurfPtr( surf_indx )->ProjectPt01( pt, iaxis, clamp( u0, 0.0, 1.0 ), clamp( w0, 0.0, 1.0 ), u_out, w_out );

    ErrorMgr.NoError();

    return idmin;
}

bool InsideSurf( const std::string &geom_id, const int &surf_indx, const vec3d &pt )
{
    Vehicle* veh = GetVehicle();
    Geom* geom_ptr = veh->FindGeom( geom_id );
    if ( !geom_ptr )
    {
        ErrorMgr.AddError( VSP_INVALID_GEOM_ID, "InsideSurf::Can't Find Geom " + geom_id );
        return false;
    }

    if ( surf_indx < 0 || surf_indx >= geom_ptr->GetNumTotalSurfs() )
    {
        ErrorMgr.AddError( VSP_INDEX_OUT_RANGE, "InsideSurf::Invalid Surface Index " + to_string( surf_indx ) );
        return false;
    }

    bool ret = geom_ptr->GetSurfPtr( surf_indx )->IsInside( pt );

    ErrorMgr.NoError();
    return ret;
}

vec3d CompPntRST( const std::string &geom_id, const int &surf_indx, const double &r, const double &s, const double &t )
{
    Vehicle* veh = GetVehicle();
    Geom* geom_ptr = veh->FindGeom( geom_id );
    vec3d ret;
    if ( !geom_ptr )
    {
        ErrorMgr.AddError( VSP_INVALID_GEOM_ID, "CompPntRST::Can't Find Geom " + geom_id );
        return ret;
    }

    if ( surf_indx < 0 || surf_indx >= geom_ptr->GetNumTotalSurfs() )
    {
        ErrorMgr.AddError( VSP_INDEX_OUT_RANGE, "CompPntRST::Invalid Surface Index " + to_string( surf_indx ) );
        return ret;
    }

    ret = geom_ptr->CompPntRST( surf_indx, clamp( r, 0.0, 1.0 ), clamp( s, 0.0, 1.0 ), clamp( t, 0.0, 1.0 ) );

    ErrorMgr.NoError();
    return ret;
}

double FindRST( const std::string &geom_id, const int &surf_indx, const vec3d &pt, double &r_out, double &s_out, double &t_out )
{
    Vehicle* vPtr = VehicleMgr.GetVehicle();
    Geom * geom = vPtr->FindGeom( geom_id );

    double dist = std::numeric_limits<double>::max();

    if ( !geom )
    {
        ErrorMgr.AddError( VSP_INVALID_GEOM_ID, "FindRST::Can't Find Geom " + geom_id );
        return dist;
    }

    if ( surf_indx < 0 || surf_indx >= geom->GetNumTotalSurfs() )
    {
        ErrorMgr.AddError( VSP_INDEX_OUT_RANGE, "FindRST::Invalid Surface Index " + to_string( surf_indx ) );
        return dist;
    }

    dist = geom->GetSurfPtr( surf_indx )->FindRST( pt, r_out, s_out, t_out );

    ErrorMgr.NoError();

    return dist;
}

double FindRSTGuess( const std::string &geom_id, const int &surf_indx, const vec3d &pt, const double &r0, const double &s0, const double &t0, double &r_out, double &s_out, double &t_out )
{
    Vehicle* vPtr = VehicleMgr.GetVehicle();
    Geom * geom = vPtr->FindGeom( geom_id );

    double dist = std::numeric_limits<double>::max();

    if ( !geom )
    {
        ErrorMgr.AddError( VSP_INVALID_GEOM_ID, "FindRST::Can't Find Geom " + geom_id );
        return dist;
    }

    if ( surf_indx < 0 || surf_indx >= geom->GetNumTotalSurfs() )
    {
        ErrorMgr.AddError( VSP_INDEX_OUT_RANGE, "FindRST::Invalid Surface Index " + to_string( surf_indx ) );
        return dist;
    }

    dist = geom->GetSurfPtr( surf_indx )->FindRST( pt, clamp( r0, 0.0, 1.0 ), clamp( s0, 0.0, 1.0 ), clamp( t0, 0.0, 1.0 ), r_out, s_out, t_out );

    ErrorMgr.NoError();

    return dist;
}

void ConvertRSTtoLMN( const std::string &geom_id, const int &surf_indx, const double &r, const double &s, const double &t, double &l_out, double &m_out, double &n_out )
{
    Vehicle* vPtr = VehicleMgr.GetVehicle();
    Geom * geom = vPtr->FindGeom( geom_id );

    if ( !geom )
    {
        ErrorMgr.AddError( VSP_INVALID_GEOM_ID, "ConvertRSTtoLMN::Can't Find Geom " + geom_id );
        return;
    }

    if ( surf_indx < 0 || surf_indx >= geom->GetNumTotalSurfs() )
    {
        ErrorMgr.AddError( VSP_INDEX_OUT_RANGE, "ConvertRSTtoLMN::Invalid Surface Index " + to_string( surf_indx ) );
        return;
    }

    geom->GetSurfPtr( surf_indx )->ConvertRSTtoLMN( r, s, t, l_out, m_out, n_out );

    ErrorMgr.NoError();

    return;
}

void ConvertRtoL( const std::string &geom_id, const int &surf_indx, const double &r, double &l_out )
{
    Vehicle* vPtr = VehicleMgr.GetVehicle();
    Geom * geom = vPtr->FindGeom( geom_id );

    if ( !geom )
    {
        ErrorMgr.AddError( VSP_INVALID_GEOM_ID, "ConvertRtoL::Can't Find Geom " + geom_id );
        return;
    }

    if ( surf_indx < 0 || surf_indx >= geom->GetNumTotalSurfs() )
    {
        ErrorMgr.AddError( VSP_INDEX_OUT_RANGE, "ConvertRtoL::Invalid Surface Index " + to_string( surf_indx ) );
        return;
    }

    geom->GetSurfPtr( surf_indx )->ConvertRtoL( r, l_out );

    ErrorMgr.NoError();

    return;
}

void ConvertLMNtoRST( const std::string &geom_id, const int &surf_indx, const double &l, const double &m, const double &n, double &r_out, double &s_out, double &t_out )
{
    Vehicle* vPtr = VehicleMgr.GetVehicle();
    Geom * geom = vPtr->FindGeom( geom_id );

    if ( !geom )
    {
        ErrorMgr.AddError( VSP_INVALID_GEOM_ID, "ConvertLMNtoRST::Can't Find Geom " + geom_id );
        return;
    }

    if ( surf_indx < 0 || surf_indx >= geom->GetNumTotalSurfs() )
    {
        ErrorMgr.AddError( VSP_INDEX_OUT_RANGE, "ConvertLMNtoRST::Invalid Surface Index " + to_string( surf_indx ) );
        return;
    }

    geom->GetSurfPtr( surf_indx )->ConvertLMNtoRST( l, m, n, r_out, s_out, t_out );

    ErrorMgr.NoError();

    return;
}

void ConvertLtoR( const std::string &geom_id, const int &surf_indx, const double &l, double &r_out )
{
    Vehicle* vPtr = VehicleMgr.GetVehicle();
    Geom * geom = vPtr->FindGeom( geom_id );

    if ( !geom )
    {
        ErrorMgr.AddError( VSP_INVALID_GEOM_ID, "ConvertLtoR::Can't Find Geom " + geom_id );
        return;
    }

    if ( surf_indx < 0 || surf_indx >= geom->GetNumTotalSurfs() )
    {
        ErrorMgr.AddError( VSP_INDEX_OUT_RANGE, "ConvertLtoR::Invalid Surface Index " + to_string( surf_indx ) );
        return;
    }

    geom->GetSurfPtr( surf_indx )->ConvertLtoR( l, r_out );

    ErrorMgr.NoError();

    return;
}

void ConvertUtoEta( const std::string &geom_id, const double &u, double &eta_out )
{
    Vehicle* vPtr = VehicleMgr.GetVehicle();
    Geom * geom = vPtr->FindGeom( geom_id );

    eta_out = u;

    if ( !geom )
    {
        ErrorMgr.AddError( VSP_INVALID_GEOM_ID, "ConvertUtoEta::Can't Find Geom " + geom_id );
        return;
    }

    WingGeom *wg = dynamic_cast<WingGeom *>( geom );

    if ( geom->GetType().m_Type != MS_WING_GEOM_TYPE || !wg )
    {
        ErrorMgr.AddError( VSP_WRONG_GEOM_TYPE, "ConvertUtoEta::Geom is not a WingGeom" );
        return;
    }

    if ( wg )
    {
        eta_out = wg->UtoEta( u );
    }

    ErrorMgr.NoError();

    return;
}

void ConvertEtatoU( const std::string &geom_id, const double &eta, double &u_out )
{
    Vehicle* vPtr = VehicleMgr.GetVehicle();
    Geom * geom = vPtr->FindGeom( geom_id );

    u_out = eta;

    if ( !geom )
    {
        ErrorMgr.AddError( VSP_INVALID_GEOM_ID, "ConvertEtatoU::Can't Find Geom " + geom_id );
        return;
    }

    WingGeom *wg = dynamic_cast<WingGeom *>( geom );

    if ( geom->GetType().m_Type != MS_WING_GEOM_TYPE || !wg )
    {
        ErrorMgr.AddError( VSP_WRONG_GEOM_TYPE, "ConvertEtatoU::Geom is not a WingGeom" );
        return;
    }

    if ( wg )
    {
        u_out = wg->EtatoU( eta );
    }

    ErrorMgr.NoError();

    return;
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
            const VspSurf *surf = geom_ptr->GetSurfPtr( surf_indx );

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
                ErrorMgr.AddError( VSP_INDEX_OUT_RANGE, "CompVecPnt01::Invalid surf index " + to_string( surf_indx ) );
                return pts;
            }
        }
        else
        {
            ErrorMgr.AddError( VSP_INDEX_OUT_RANGE, "CompVecPnt01::Input size mismatch." );
            return pts;
        }
    }
    else
    {
        ErrorMgr.AddError( VSP_INVALID_GEOM_ID, "CompVecPnt01::Can't Find Geom " + geom_id );
        return pts;
    }
    ErrorMgr.NoError();
    return pts;
}

vector < vec3d > CompVecDegenPnt01( const std::string &geom_id, const int &surf_indx, const int &degen_type, const vector < double > &us, const vector < double > &ws )
{
    Vehicle* veh = GetVehicle();

    Geom* geom_ptr = veh->FindGeom( geom_id );

    vector < vec3d > pts;
    pts.resize( 0 );

    if ( geom_ptr )
    {
        if ( us.size() == ws.size() )
        {
            const VspSurf *surf = geom_ptr->GetSurfPtr( surf_indx );

            if ( surf )
            {
                pts.resize( us.size() );

                for ( int i = 0; i < us.size(); i++ )
                {
                    pts[i] = surf->CompDegenPnt01( degen_type, clamp( us[i], 0.0, 1.0 ), clamp( ws[i], 0.0, 1.0 ) );
                }
            }
            else
            {
                ErrorMgr.AddError( VSP_INDEX_OUT_RANGE, "CompVecDegenPnt01::Invalid surf index " + to_string( surf_indx ) );
                return pts;
            }
        }
        else
        {
            ErrorMgr.AddError( VSP_INDEX_OUT_RANGE, "CompVecDegenPnt01::Input size mismatch." );
            return pts;
        }
    }
    else
    {
        ErrorMgr.AddError( VSP_INVALID_GEOM_ID, "CompVecDegenPnt01::Can't Find Geom " + geom_id );
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
            const VspSurf *surf = geom_ptr->GetSurfPtr( surf_indx );

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
                ErrorMgr.AddError( VSP_INDEX_OUT_RANGE, "CompNorm01::Invalid surf index " + to_string( surf_indx ) );
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

void CompVecCurvature01( const std::string &geom_id, const int &surf_indx, const vector < double > &us, const vector < double > &ws, vector < double > &k1_out_vec, vector < double > &k2_out_vec, vector < double > &ka_out_vec, vector < double > &kg_out_vec )
{
    Vehicle* veh = GetVehicle();

    Geom* geom_ptr = veh->FindGeom( geom_id );

    k1_out_vec.resize( 0 );
    k2_out_vec.resize( 0 );
    ka_out_vec.resize( 0 );
    kg_out_vec.resize( 0 );

    if ( geom_ptr )
    {
        if ( us.size() == ws.size() )
        {
            const VspSurf *surf = geom_ptr->GetSurfPtr( surf_indx );

            if ( surf )
            {
                k1_out_vec.resize( us.size() );
                k2_out_vec.resize( us.size() );
                ka_out_vec.resize( us.size() );
                kg_out_vec.resize( us.size() );

                for ( int i = 0; i < us.size(); i++ )
                {
                    surf->CompCurvature01( clamp( us[i], 0.0, 1.0 ), clamp( ws[i], 0.0, 1.0 ), k1_out_vec[i], k2_out_vec[i], ka_out_vec[i], kg_out_vec[i] );
                }
            }
            else
            {
                ErrorMgr.AddError( VSP_INDEX_OUT_RANGE, "CompCurvature01::Invalid surf index " + to_string( surf_indx ) );
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

void ProjVecPnt01(const std::string &geom_id, const int &surf_indx, const vector < vec3d > &pts, vector < double > &u_out_vec, vector < double > &w_out_vec, vector < double > &d_out_vec )
{
    Vehicle* veh = GetVehicle();

    Geom* geom_ptr = veh->FindGeom( geom_id );

    u_out_vec.resize( 0 );
    w_out_vec.resize( 0 );
    d_out_vec.resize( 0 );

    if ( geom_ptr )
    {
        const VspSurf *surf = geom_ptr->GetSurfPtr( surf_indx );

        if ( surf )
        {
            u_out_vec.resize( pts.size() );
            w_out_vec.resize( pts.size() );
            d_out_vec.resize( pts.size() );

            for ( int i = 0; i < pts.size(); i++ )
            {
                d_out_vec[i] = surf->FindNearest01( u_out_vec[i], w_out_vec[i], pts[i] );
            }
        }
        else
        {
            ErrorMgr.AddError( VSP_INDEX_OUT_RANGE, "ProjVecPnt01::Invalid surf index " + to_string( surf_indx ) );
            return;
        }
    }
    else
    {
        ErrorMgr.AddError( VSP_INVALID_GEOM_ID, "ProjVecPnt01::Can't Find Geom " + geom_id );
        return;
    }
    ErrorMgr.NoError();
}

void ProjVecPnt01Guess( const std::string &geom_id, const int &surf_indx, const vector < vec3d > &pts, const vector < double > &u0s, const vector < double > &w0s, vector < double > &u_out_vec, vector < double > &w_out_vec, vector < double > &d_out_vec )
{
    Vehicle* veh = GetVehicle();

    Geom* geom_ptr = veh->FindGeom( geom_id );

    u_out_vec.resize( 0 );
    w_out_vec.resize( 0 );
    d_out_vec.resize( 0 );

    if ( geom_ptr )
    {
        if ( pts.size() == u0s.size() && pts.size() == w0s.size() )
        {
            const VspSurf *surf = geom_ptr->GetSurfPtr(surf_indx);

            if ( surf )
            {
                u_out_vec.resize( pts.size() );
                w_out_vec.resize( pts.size() );
                d_out_vec.resize( pts.size() );

                for ( int i = 0; i < pts.size(); i++ )
                {
                    d_out_vec[i] = surf->FindNearest01( u_out_vec[i], w_out_vec[i], pts[i], clamp( u0s[i], 0.0, 1.0 ), clamp( w0s[i], 0.0, 1.0 ) );
                }
            }
            else
            {
                ErrorMgr.AddError( VSP_INDEX_OUT_RANGE, "ProjVecPnt01Guess::Invalid surf index " + to_string( surf_indx ) );
                return;
            }
        }
        else
        {
            ErrorMgr.AddError( VSP_INDEX_OUT_RANGE, "ProjVecPnt01Guess::Input size mismatch." );
            return;
        }
    }
    else
    {
        ErrorMgr.AddError( VSP_INVALID_GEOM_ID, "ProjVecPnt01Guess::Can't Find Geom " + geom_id );
        return;
    }
    ErrorMgr.NoError();
}

void AxisProjVecPnt01(const std::string &geom_id, const int &surf_indx, const int &iaxis, const std::vector < vec3d > &pts, std::vector < double > &u_out_vec, std::vector < double > &w_out_vec, std::vector < double > &d_out_vec )
{
    Vehicle* veh = GetVehicle();

    Geom* geom_ptr = veh->FindGeom( geom_id );

    u_out_vec.resize( 0 );
    w_out_vec.resize( 0 );
    d_out_vec.resize( 0 );

    if ( geom_ptr )
    {
        const VspSurf *surf = geom_ptr->GetSurfPtr( surf_indx );

        if ( surf )
        {
            u_out_vec.resize( pts.size() );
            w_out_vec.resize( pts.size() );
            d_out_vec.resize( pts.size() );

            for ( int i = 0; i < pts.size(); i++ )
            {
                d_out_vec[i] = surf->ProjectPt01( pts[i], iaxis, u_out_vec[i], w_out_vec[i] );
            }
        }
        else
        {
            ErrorMgr.AddError( VSP_INDEX_OUT_RANGE, "AxisProjVecPnt01::Invalid surf index " + to_string( surf_indx ) );
            return;
        }
    }
    else
    {
        ErrorMgr.AddError( VSP_INVALID_GEOM_ID, "AxisProjVecPnt01::Can't Find Geom " + geom_id );
        return;
    }
    ErrorMgr.NoError();
}

void AxisProjVecPnt01Guess(const std::string &geom_id, const int &surf_indx, const int &iaxis, const std::vector < vec3d > &pts, const std::vector < double > &u0s, const std::vector < double > &w0s, std::vector < double > &u_out_vec, std::vector < double > &w_out_vec, std::vector < double > &d_out_vec )
{
    Vehicle* veh = GetVehicle();

    Geom* geom_ptr = veh->FindGeom( geom_id );

    u_out_vec.resize( 0 );
    w_out_vec.resize( 0 );
    d_out_vec.resize( 0 );

    if ( geom_ptr )
    {
        if ( pts.size() == u0s.size() && pts.size() == w0s.size() )
        {
            const VspSurf *surf = geom_ptr->GetSurfPtr(surf_indx);

            if ( surf )
            {
                u_out_vec.resize( pts.size() );
                w_out_vec.resize( pts.size() );
                d_out_vec.resize( pts.size() );

                for ( int i = 0; i < pts.size(); i++ )
                {
                    d_out_vec[i] = surf->ProjectPt01( pts[i], iaxis, clamp( u0s[i], 0.0, 1.0 ), clamp( w0s[i], 0.0, 1.0 ), u_out_vec[i], w_out_vec[i] );
                }
            }
            else
            {
                ErrorMgr.AddError( VSP_INDEX_OUT_RANGE, "AxisProjVecPnt01Guess::Invalid surf index " + to_string( surf_indx ) );
                return;
            }
        }
        else
        {
            ErrorMgr.AddError( VSP_INDEX_OUT_RANGE, "AxisProjVecPnt01Guess::Input size mismatch." );
            return;
        }
    }
    else
    {
        ErrorMgr.AddError( VSP_INVALID_GEOM_ID, "AxisProjVecPnt01Guess::Can't Find Geom " + geom_id );
        return;
    }
    ErrorMgr.NoError();
}

std::vector < bool > VecInsideSurf( const std::string &geom_id, const int &surf_indx, const std::vector < vec3d > &pts )
{
    Vehicle* veh = GetVehicle();

    Geom* geom_ptr = veh->FindGeom( geom_id );

    vector < bool > ret;
    ret.resize( 0 );

    if ( geom_ptr )
    {
        const VspSurf *surf = geom_ptr->GetSurfPtr( surf_indx );

        if ( surf )
        {
            ret.resize( pts.size(), false );

            for ( int i = 0; i < pts.size(); i++ )
            {
                ret[i] = surf->IsInside( pts[i] );
            }
        }
        else
        {
            ErrorMgr.AddError( VSP_INDEX_OUT_RANGE, "VecInsideSurf::Invalid surf index " + to_string( surf_indx ) );
            return ret;
        }
    }
    else
    {
        ErrorMgr.AddError( VSP_INVALID_GEOM_ID, "VecInsideSurf::Can't Find Geom " + geom_id );
        return ret;
    }
    ErrorMgr.NoError();
    return ret;
}

std::vector < vec3d > CompVecPntRST( const std::string &geom_id, const int &surf_indx, const std::vector < double > &rs, const std::vector < double > &ss, const std::vector < double > &ts )
{
    Vehicle* veh = GetVehicle();

    Geom* geom_ptr = veh->FindGeom( geom_id );

    vector < vec3d > pts;
    pts.resize( 0 );

    if ( geom_ptr )
    {
        if ( rs.size() == ss.size() && rs.size() == ts.size() )
        {
            const VspSurf *surf = geom_ptr->GetSurfPtr( surf_indx );

            if ( surf )
            {
                pts.resize( rs.size() );

                for ( int i = 0; i < rs.size(); i++ )
                {
                    pts[i] = surf->CompPntRST( clamp( rs[i], 0.0, 1.0 ), clamp( ss[i], 0.0, 1.0 ), clamp( ts[i], 0.0, 1.0 ) );
                }
            }
            else
            {
                ErrorMgr.AddError( VSP_INDEX_OUT_RANGE, "CompVecPntRST::Invalid surf index " + to_string( surf_indx ) );
                return pts;
            }
        }
        else
        {
            ErrorMgr.AddError( VSP_INDEX_OUT_RANGE, "CompVecPntRST::Input size mismatch." );
            return pts;
        }
    }
    else
    {
        ErrorMgr.AddError( VSP_INVALID_GEOM_ID, "CompVecPntRST::Can't Find Geom " + geom_id );
        return pts;
    }
    ErrorMgr.NoError();
    return pts;
}

void FindRSTVec( const std::string &geom_id, const int &surf_indx, const std::vector < vec3d > &pts, std::vector < double > &r_out_vec, std::vector < double > &s_out_vec, std::vector < double > &t_out_vec, std::vector < double > &d_out_vec )
{
    Vehicle* veh = GetVehicle();

    Geom* geom_ptr = veh->FindGeom( geom_id );

    r_out_vec.resize( 0 );
    s_out_vec.resize( 0 );
    t_out_vec.resize( 0 );
    d_out_vec.resize( 0 );

    if ( geom_ptr )
    {
        const VspSurf *surf = geom_ptr->GetSurfPtr( surf_indx );

        if ( surf )
        {
            r_out_vec.resize( pts.size() );
            s_out_vec.resize( pts.size() );
            t_out_vec.resize( pts.size() );
            d_out_vec.resize( pts.size() );

            surf->FindRST( pts, r_out_vec, s_out_vec, t_out_vec, d_out_vec );
        }
        else
        {
            ErrorMgr.AddError( VSP_INDEX_OUT_RANGE, "FindRSTVec::Invalid surf index " + to_string( surf_indx ) );
            return;
        }
    }
    else
    {
        ErrorMgr.AddError( VSP_INVALID_GEOM_ID, "FindRSTVec::Can't Find Geom " + geom_id );
        return;
    }
    ErrorMgr.NoError();
}

void FindRSTVecGuess( const std::string &geom_id, const int &surf_indx, const std::vector < vec3d > &pts, const std::vector < double > &r0s, const std::vector < double > &s0s, const std::vector < double > &t0s, std::vector < double > &r_out_vec, std::vector < double > &s_out_vec, std::vector < double > &t_out_vec, std::vector < double > &d_out_vec )
{
    Vehicle* veh = GetVehicle();

    Geom* geom_ptr = veh->FindGeom( geom_id );

    r_out_vec.resize( 0 );
    s_out_vec.resize( 0 );
    t_out_vec.resize( 0 );
    d_out_vec.resize( 0 );

    if ( geom_ptr )
    {
        if ( pts.size() == r0s.size() && pts.size() == s0s.size() && pts.size() == t0s.size() )
        {
            const VspSurf *surf = geom_ptr->GetSurfPtr(surf_indx);

            if ( surf )
            {
                r_out_vec.resize( pts.size() );
                s_out_vec.resize( pts.size() );
                t_out_vec.resize( pts.size() );
                d_out_vec.resize( pts.size() );

                for ( int i = 0; i < pts.size(); i++ )
                {
                    d_out_vec[i] = surf->FindRST( pts[i], clamp( r0s[i], 0.0, 1.0 ), clamp( s0s[i], 0.0, 1.0 ), clamp( t0s[i], 0.0, 1.0 ), r_out_vec[i], s_out_vec[i], t_out_vec[i] );
                }
            }
            else
            {
                ErrorMgr.AddError( VSP_INDEX_OUT_RANGE, "FindRSTVecGuess::Invalid surf index " + to_string( surf_indx ) );
                return;
            }
        }
        else
        {
            ErrorMgr.AddError( VSP_INDEX_OUT_RANGE, "FindRSTVecGuess::Input size mismatch." );
            return;
        }
    }
    else
    {
        ErrorMgr.AddError( VSP_INVALID_GEOM_ID, "FindRSTVecGuess::Can't Find Geom " + geom_id );
        return;
    }
    ErrorMgr.NoError();
}

void ConvertRSTtoLMNVec( const std::string &geom_id, const int &surf_indx,
                      const std::vector < double > &r_vec, const std::vector < double > &s_vec, const std::vector < double > &t_vec,
                      std::vector < double > &l_out_vec, std::vector < double > &m_out_vec, std::vector < double > &n_out_vec )
{
    Vehicle* veh = GetVehicle();

    Geom* geom_ptr = veh->FindGeom( geom_id );

    l_out_vec.resize( 0 );
    m_out_vec.resize( 0 );
    n_out_vec.resize( 0 );

    if ( geom_ptr )
    {
        if ( r_vec.size() == s_vec.size() && r_vec.size() == t_vec.size() )
        {
            const VspSurf *surf = geom_ptr->GetSurfPtr(surf_indx);

            if ( surf )
            {
                l_out_vec.resize( r_vec.size() );
                m_out_vec.resize( r_vec.size() );
                n_out_vec.resize( r_vec.size() );

                for ( int i = 0; i < r_vec.size(); i++ )
                {
                    surf->ConvertRSTtoLMN( r_vec[i], s_vec[i], t_vec[i], l_out_vec[i], m_out_vec[i], n_out_vec[i] );
                }
            }
            else
            {
                ErrorMgr.AddError( VSP_INDEX_OUT_RANGE, "ConvertRSTtoLMN::Invalid surf index " + to_string( surf_indx ) );
                return;
            }
        }
        else
        {
            ErrorMgr.AddError( VSP_INDEX_OUT_RANGE, "ConvertRSTtoLMN::Input size mismatch." );
            return;
        }
    }
    else
    {
        ErrorMgr.AddError( VSP_INVALID_GEOM_ID, "ConvertRSTtoLMN::Can't Find Geom " + geom_id );
        return;
    }
    ErrorMgr.NoError();
}

void ConvertLMNtoRSTVec( const std::string &geom_id, const int &surf_indx,
                      const std::vector < double > &l_vec, const std::vector < double > &m_vec, const std::vector < double > &n_vec,
                      std::vector < double > &r_out_vec, std::vector < double > &s_out_vec, std::vector < double > &t_out_vec )
{
    Vehicle* veh = GetVehicle();

    Geom* geom_ptr = veh->FindGeom( geom_id );

    r_out_vec.resize( 0 );
    s_out_vec.resize( 0 );
    t_out_vec.resize( 0 );

    if ( geom_ptr )
    {
        if ( l_vec.size() == m_vec.size() && l_vec.size() == n_vec.size() )
        {
            const VspSurf *surf = geom_ptr->GetSurfPtr(surf_indx);

            if ( surf )
            {
                r_out_vec.resize( l_vec.size() );
                s_out_vec.resize( l_vec.size() );
                t_out_vec.resize( l_vec.size() );

                for ( int i = 0; i < l_vec.size(); i++ )
                {
                    surf->ConvertRSTtoLMN( l_vec[i], m_vec[i], n_vec[i], r_out_vec[i], s_out_vec[i], t_out_vec[i] );
                }
            }
            else
            {
                ErrorMgr.AddError( VSP_INDEX_OUT_RANGE, "ConvertLMNtoRST::Invalid surf index " + to_string( surf_indx ) );
                return;
            }
        }
        else
        {
            ErrorMgr.AddError( VSP_INDEX_OUT_RANGE, "ConvertLMNtoRST::Input size mismatch." );
            return;
        }
    }
    else
    {
        ErrorMgr.AddError( VSP_INVALID_GEOM_ID, "ConvertLMNtoRST::Can't Find Geom " + geom_id );
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

string GetVSPVersion()
{
    return string( VSPVERSION4 );
}

int GetVSPVersionMajor()
{
    return VSP_VERSION_MAJOR;
}

int GetVSPVersionMinor()
{
    return VSP_VERSION_MINOR;
}

int GetVSPVersionChange()
{
    return VSP_VERSION_CHANGE;
}

string GetVSPExePath()
{
    Vehicle* veh = VehicleMgr.GetVehicle();
    if ( veh )
    {
        return veh->GetExePath();
    }
    return string();
}

bool SetVSPAEROPath( const std::string & path )
{
    Vehicle* veh = VehicleMgr.GetVehicle();
    if( veh )
    {
        return veh->SetVSPAEROPath( path );
    }
    return false;
}

std::string GetVSPAEROPath()
{
    Vehicle* veh = VehicleMgr.GetVehicle();
    if( veh )
    {
        return veh->GetVSPAEROPath();
    }
    return string();
}

bool CheckForVSPAERO( const std::string & path )
{
    Vehicle* veh = VehicleMgr.GetVehicle();
    if( veh )
    {
        return veh->CheckForVSPAERO( path );
    }
    return false;
}

bool SetVSPHelpPath( const std::string & path )
{
    Vehicle* veh = VehicleMgr.GetVehicle();
    if( veh )
    {
        return veh->SetHelpPath( path );
    }
    return false;
}

std::string GetVSPHelpPath()
{
    Vehicle* veh = VehicleMgr.GetVehicle();
    if( veh )
    {
        return veh->GetHelpPath();
    }
    return string();
}

bool CheckForVSPHelp( const std::string & path )
{
    Vehicle* veh = VehicleMgr.GetVehicle();
    if( veh )
    {
        return veh->CheckForHelp( path );
    }
    return false;
}

//======================= Advanced Link Functions ============================//

std::vector< std::string > GetAdvLinkNames()
{
    ErrorMgr.NoError();
    return AdvLinkMgr.GetLinkNames();
}

int GetLinkIndex( const string & name )
{
    int indx = AdvLinkMgr.GetLinkIndex( name );

    if ( indx < 0 )
    {
        ErrorMgr.AddError( VSP_CANT_FIND_NAME, "GetLinkIndex::Can't find advanced link with name: " + name );
        return indx;
    }

    ErrorMgr.NoError();
    return indx;
}

void DelAdvLink( int index )
{
    AdvLinkMgr.DelLink( index );

    ErrorMgr.NoError();
    return;
}

void DelAllAdvLinks()
{
    AdvLinkMgr.DelAllLinks();

    ErrorMgr.NoError();
    return;
}

void AddAdvLink( const string & name )
{
    AdvLinkMgr.AddLink( name );

    ErrorMgr.NoError();
    return;
}

void AddAdvLinkInput( int index, const string & parm_id, const string & var_name )
{
    AdvLink * adv_link = AdvLinkMgr.GetLink( index );

    if ( !adv_link )
    {
        ErrorMgr.AddError( VSP_INDEX_OUT_RANGE, "AddAdvLinkInput::Invalid Advanced Link Index " + to_string( index ) );
        return;
    }

    if ( adv_link->DuplicateVarName( var_name ) )
    {
        ErrorMgr.AddError( VSP_DUPLICATE_NAME, "AddAdvLinkInput::Duplicate advanced link input name " + var_name );
        return;
    }

    adv_link->AddVar( parm_id, var_name, true );

    ErrorMgr.NoError();
    return;
}

void AddAdvLinkOutput( int index, const string & parm_id, const string & var_name )
{
    AdvLink * adv_link = AdvLinkMgr.GetLink( index );

    if ( !adv_link )
    {
        ErrorMgr.AddError( VSP_INDEX_OUT_RANGE, "AddAdvLinkOutput::Invalid Advanced Link Index " + to_string( index ) );
        return;
    }

    if ( adv_link->DuplicateVarName( var_name ) )
    {
        ErrorMgr.AddError( VSP_DUPLICATE_NAME, "AddAdvLinkInput::Duplicate advanced link output name " + var_name );
        return;
    }

    adv_link->AddVar( parm_id, var_name, false );

    ErrorMgr.NoError();
    return;
}

void DelAdvLinkInput( int index, const string & var_name )
{
    AdvLink * adv_link = AdvLinkMgr.GetLink( index );

    if ( !adv_link )
    {
        ErrorMgr.AddError( VSP_INDEX_OUT_RANGE, "DelAdvLinkInput::Invalid Advanced Link Index " + to_string( index ) );
        return;
    }

    adv_link->DeleteVar( index, true );

    ErrorMgr.NoError();
    return;
}

void DelAdvLinkOutput( int index, const string & var_name )
{
    AdvLink * adv_link = AdvLinkMgr.GetLink( index );

    if ( !adv_link )
    {
        ErrorMgr.AddError( VSP_INDEX_OUT_RANGE, "DelAdvLinkOutput::Invalid Advanced Link Index " + to_string( index ) );
        return;
    }

    adv_link->DeleteVar( index, false );

    ErrorMgr.NoError();
    return;
}

std::vector< std::string > GetAdvLinkInputNames( int index )
{
    vector < string > ret;
    AdvLink * adv_link = AdvLinkMgr.GetLink( index );

    if ( !adv_link )
    {
        ErrorMgr.AddError( VSP_INDEX_OUT_RANGE, "GetAdvLinkInputNames::Invalid Advanced Link Index " + to_string( index ) );
        return ret;
    }

    ret = adv_link->GetInputNames();

    ErrorMgr.NoError();
    return ret;
}

std::vector< std::string > GetAdvLinkInputParms( int index )
{
    vector < string > ret;
    AdvLink * adv_link = AdvLinkMgr.GetLink( index );

    if ( !adv_link )
    {
        ErrorMgr.AddError( VSP_INDEX_OUT_RANGE, "GetAdvLinkInputParms::Invalid Advanced Link Index " + to_string( index ) );
        return ret;
    }

    ret = adv_link->GetInputParms();

    ErrorMgr.NoError();
    return ret;
}

std::vector< std::string > GetAdvLinkOutputNames( int index )
{
    vector < string > ret;
    AdvLink * adv_link = AdvLinkMgr.GetLink( index );

    if ( !adv_link )
    {
        ErrorMgr.AddError( VSP_INDEX_OUT_RANGE, "GetAdvLinkOutputNames::Invalid Advanced Link Index " + to_string( index ) );
        return ret;
    }

    ret = adv_link->GetOutputNames();

    ErrorMgr.NoError();
    return ret;
}

std::vector< std::string > GetAdvLinkOutputParms( int index )
{
    vector < string > ret;
    AdvLink * adv_link = AdvLinkMgr.GetLink( index );

    if ( !adv_link )
    {
        ErrorMgr.AddError( VSP_INDEX_OUT_RANGE, "GetAdvLinkOutputParms::Invalid Advanced Link Index " + to_string( index ) );
        return ret;
    }

    ret = adv_link->GetOutputParms();

    ErrorMgr.NoError();
    return ret;
}

bool ValidateAdvLinkParms( int index )
{
    AdvLink * adv_link = AdvLinkMgr.GetLink( index );

    if ( !adv_link )
    {
        ErrorMgr.AddError( VSP_INDEX_OUT_RANGE, "ValidateAdvLinkParms::Invalid Advanced Link Index " + to_string( index ) );
        return false;
    }

    // Errors thrown by ValidateParms if problems are encountered.
    bool ret = adv_link->ValidateParms();

    if ( ret )
    {
        ErrorMgr.NoError();
    }
    return ret;
}

void SetAdvLinkCode( int index, const string & code )
{
    AdvLink * adv_link = AdvLinkMgr.GetLink( index );

    if ( !adv_link )
    {
        ErrorMgr.AddError( VSP_INDEX_OUT_RANGE, "SetAdvLinkCode::Invalid Advanced Link Index " + to_string( index ) );
        return;
    }

    adv_link->SetScriptCode( code );

    ErrorMgr.NoError();
    return;
}

std::string GetAdvLinkCode( int index )
{
    string ret;
    AdvLink * adv_link = AdvLinkMgr.GetLink( index );

    if ( !adv_link )
    {
        ErrorMgr.AddError( VSP_INDEX_OUT_RANGE, "GetAdvLinkCode::Invalid Advanced Link Index " + to_string( index ) );
        return ret;
    }

    ret = adv_link->GetScriptCode();

    ErrorMgr.NoError();
    return ret;
}

void SearchReplaceAdvLinkCode( int index, const string & from, const string & to )
{
    AdvLink * adv_link = AdvLinkMgr.GetLink( index );

    if ( !adv_link )
    {
        ErrorMgr.AddError( VSP_INDEX_OUT_RANGE, "SearchReplaceAdvLinkCode::Invalid Advanced Link Index " + to_string( index ) );
        return;
    }

    adv_link->SearchReplaceCode( from, to );

    ErrorMgr.NoError();
    return;
}

bool BuildAdvLinkScript( int index )
{
    AdvLink * adv_link = AdvLinkMgr.GetLink( index );

    if ( !adv_link )
    {
        ErrorMgr.AddError( VSP_INDEX_OUT_RANGE, "BuildAdvLinkScript::Invalid Advanced Link Index " + to_string( index ) );
        return false;
    }

    // Errors thrown by BuildScript if problems are encountered.
    bool ret = adv_link->BuildScript();

    if ( ret )
    {
        ErrorMgr.NoError();
    }
    return ret;
}

}// vsp namespace