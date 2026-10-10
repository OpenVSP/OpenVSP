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
#include "FitModelMgr.h"
#include "GeometryAnalysisMgr.h"
#include "HingeGeom.h"
#include "LinkMgr.h"
#include "Link.h"
#include "main.h"
#include "MeasureMgr.h"
#include "ParasiteDragMgr.h"
#include "WaveDragMgr.h"
#include "AeroStructMgr.h"
#include "ParmMgr.h"
#include "PntNodeMerge.h"
#include "PropGeom.h"
#include "RoutingGeom.h"
#include "AuxiliaryGeom.h"
#include "GearGeom.h"
#include "GeomEngine.h"
#include "StackGeom.h"
#include "StructureMgr.h"
#include "SubSurfaceMgr.h"
#include "SurfaceIntersectionMgr.h"
#include "VarPresetMgr.h"
#include "Vehicle.h"
#include "VehicleMgr.h"
#include "VKTAirfoil.h"
#include "VSP_Geom_API.h"
#include "CloneGeom.h"
#include "VSPAEROMgr.h"
#include "LightMgr.h"
#include "VspUtil.h"
#include "WingGeom.h"
#include "MeshGeom.h"
#include "PtCloudGeom.h"
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
XSecSurf* FindXSecSurf( const std::string & id )
{
    Vehicle* veh = GetVehicle();
 //   vector< Geom* > geom_vec = veh->FindGeomVec( veh->GetGeomVec() );
    std::vector< Geom* > geom_vec = veh->GetGeomStoreVec();

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
XSec* FindXSec( const std::string & id )
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
#if defined(__clang__)
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Warray-bounds"
#endif
            a[1] = 1;
#if defined(__clang__)
#pragma clang diagnostic pop
#endif
        default:
            abort();
    }
}

int GetAndResetUpdateCount()
{
    ErrorMgr.NoError();
    return (int) UpdateCountMgr.GetAndResetUpdateCount();
}

//===================================================================//
//===============       File I/O Functions        ===================//
//===================================================================//

void ReadVSPFile( const std::string & file_name )
{
    Vehicle* veh = GetVehicle();
    int err = veh->ReadXMLFile( file_name );
    if( err != 0 )
    {
        ErrorMgr.AddError( VSP_WRONG_FILE_TYPE, "ReadVSPFile::Error Reading File " + file_name );
        return;
    }
    veh->SetVSP3FileName( file_name );
    ErrorMgr.NoError();
}

void WriteVSPFile( const std::string & file_name, int set )
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

void SetVSP3FileName( const std::string & file_name )
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

std::string GetVSPFileName()
{
    Vehicle* veh = GetVehicle();
    if ( !veh ) return std::string("nullptr");

    ErrorMgr.NoError();
    return veh->GetVSP3FileName();
}

void ClearVSPModel()
{
    GetVehicle()->Renew();
    ErrorMgr.NoError();
}

// The IDs a script uses for a parent that is the Vehicle itself.
static bool TopLevelParentID( const std::string & id )
{
    return id.empty() || id == "NONE" || id == GetVehicle()->GetID();
}

void InsertVSPFile( const std::string & file_name, const std::string & parent )
{
    Vehicle* veh = GetVehicle();

    Geom* parent_geom = nullptr;
    if ( !TopLevelParentID( parent ) )
    {
        parent_geom = veh->FindGeom( parent );
        if ( !parent_geom )
        {
            ErrorMgr.AddError( VSP_INVALID_GEOM_ID, "InsertVSPFile::Can't Find Parent " + parent );
            return;
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
        ErrorMgr.AddError( VSP_WRONG_FILE_TYPE, "InsertVSPFile::Error Reading File " + file_name );
        return;
    }
    ErrorMgr.NoError();
}

std::string ImportFile( const std::string & file_name, int file_type, const std::string & parent  )
{
    Vehicle* veh = GetVehicle();
    Geom* parent_geom = nullptr;
    if ( !TopLevelParentID( parent ) )
    {
        parent_geom = veh->FindGeom( parent );
        if ( !parent_geom )
        {
            ErrorMgr.AddError( VSP_INVALID_GEOM_ID, "ImportFile::Can't Find Parent " + parent );
            return std::string();
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

std::string ExportFile( const std::string & file_name, int thick_set, int file_type, int subsFlag, int thin_set, bool useMode, const std::string &modeID )
{
    if ( file_type == EXPORT_STEP_STITCH || file_type == EXPORT_IGES_STITCH )
    {
        std::string err = SurfaceIntersectionMgr.SplitStitchSurfaces( file_name, file_type == EXPORT_STEP_STITCH, thick_set, thin_set,
                                                                      useMode, modeID );
        if ( !err.empty() )
        {
            ErrorMgr.AddError( VSP_FILE_WRITE_FAILURE, "ExportFile::" + err + ", " + file_name + " not written" );
            return std::string();
        }

        ErrorMgr.NoError();
        return std::string();
    }

    std::string mesh_id = GetVehicle()->ExportFile( file_name, thick_set, thin_set, subsFlag, file_type, useMode, modeID );

    ErrorMgr.NoError();
    return mesh_id;
}

std::string GetBEMPropID()
{
    Vehicle* veh = GetVehicle();
    if ( !veh )
    {
        ErrorMgr.AddError( VSP_INVALID_PTR, "GetBEMPropID::Failure Getting Vehicle Ptr" );
        return std::string();
    }

    ErrorMgr.NoError();
    return veh->m_BEMPropID;
}

void SetBEMPropID( const std::string & prop_id )
{
    Vehicle* veh = GetVehicle();
    Geom* geom_ptr = veh->FindGeom( prop_id );
    if ( !geom_ptr )
    {
        ErrorMgr.AddError( VSP_INVALID_GEOM_ID, "SetBEMPropID::Can't Find Geom " + prop_id );
        return;
    }
    else if ( geom_ptr->GetBehaviorType() != PROP_GEOM_TYPE )
    {
        ErrorMgr.AddError( VSP_WRONG_GEOM_TYPE, "SetBEMPropID::Geom " + prop_id + " is not a propeller" );
        return;
    }

    PropGeom* behavior_prop = dynamic_cast <PropGeom*> ( geom_ptr->GetBehaviorGeom() );
    if ( !behavior_prop )
    {
        ErrorMgr.AddError( VSP_INVALID_PTR, "SetBEMPropID::Geom is not a propeller " + prop_id );
        return;
    }

    veh->m_BEMPropID = prop_id;
    ErrorMgr.NoError();
}

//===================================================================//
//======================== Design Files =============================//
//===================================================================//
void ReadApplyDESFile( const std::string & file_name )
{
    DesignVarMgr.ReadDesVarsDES( file_name );
    ErrorMgr.NoError();
}

void WriteDESFile( const std::string & file_name )
{
    DesignVarMgr.WriteDesVarsDES( file_name );
    ErrorMgr.NoError();
}

void ReadApplyXDDMFile( const std::string & file_name )
{
    DesignVarMgr.ReadDesVarsXDDM( file_name );
    ErrorMgr.NoError();
}

void WriteXDDMFile( const std::string & file_name )
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

void AddDesignVar( const std::string & parm_id, int type )
{
    if ( !ParmMgr.FindParm( parm_id ) )
    {
        ErrorMgr.AddError( VSP_CANT_FIND_PARM, "AddDesignVar::Can't Find Parm " + parm_id );
        return;
    }

    DesignVarMgr.AddVar( parm_id, type );
    ErrorMgr.NoError();
}

void AddDesignVarLimits( const string & parm_id, int type, double lowerlimit, double upperlimit )
{
    if ( !ParmMgr.FindParm( parm_id ) )
    {
        ErrorMgr.AddError( VSP_CANT_FIND_PARM, "AddDesignVarLimits::Can't Find Parm " + parm_id );
        return;
    }

    DesignVarMgr.AddVar( parm_id, type, lowerlimit, upperlimit );
    ErrorMgr.NoError();
}

void DeleteAllDesignVars()
{
    DesignVarMgr.DelAllVars();
    ErrorMgr.NoError();
}

std::string GetDesignVar( int index )
{
    std::string parm_id;

    DesignVar* dv = DesignVarMgr.GetVar( index );

    if ( !dv )
    {
        ErrorMgr.AddError( VSP_INDEX_OUT_RANGE, "GetDesignVar::Design variable " + to_string( index ) + " out of range." );
        return parm_id;
    }

    parm_id = dv->m_ParmID;

    ErrorMgr.NoError();
    return parm_id;
}

static int GetDesignVarTypeImpl( int index, const std::string & caller )
{
    int dvtype = -1;

    DesignVar* dv = DesignVarMgr.GetVar( index );

    if ( !dv )
    {
        ErrorMgr.AddError( VSP_INDEX_OUT_RANGE, caller + "::Design variable index " + to_string( index ) + " out of range." );
        return dvtype;
    }

    dvtype = dv->m_XDDM_Type;

    ErrorMgr.NoError();
    return dvtype;
}

int GetDesignVarType( int index )
{
    return GetDesignVarTypeImpl( index, "GetDesignVarType" );
}

//==== The index of a design variable by Parm ID, reporting through the named caller ====//
int FindDesignVarIndex( const std::string & parm_id, const std::string & caller )
{
    for ( int i = 0; i < ( int )DesignVarMgr.GetNumVars(); i++ )
    {
        if ( DesignVarMgr.GetVar( i )->m_ParmID == parm_id )
        {
            return i;
        }
    }

    ErrorMgr.AddError( VSP_INVALID_ID, caller + "::Parm " + parm_id + " is not a design variable" );
    return -1;
}

int GetDesignVarTypeByID( const std::string & parm_id )
{
    int index = FindDesignVarIndex( parm_id, "GetDesignVarTypeByID" );
    if ( index < 0 )
    {
        return -1;
    }

    return GetDesignVarTypeImpl( index, "GetDesignVarTypeByID" );
}

void DeleteDesignVar( const std::string & parm_id )
{
    int index = FindDesignVarIndex( parm_id, "DeleteDesignVar" );
    if ( index < 0 )
    {
        return;
    }

    DesignVarMgr.SetCurrVarIndex( index );
    DesignVarMgr.DelCurrVar();

    ErrorMgr.NoError();
}

std::vector < std::string > GetDesignVarVec()
{
    std::vector < std::string > ret_vec;

    for ( int i = 0; i < ( int )DesignVarMgr.GetNumVars(); i++ )
    {
        ret_vec.push_back( DesignVarMgr.GetVar( i )->m_ParmID );
    }

    ErrorMgr.NoError();
    return ret_vec;
}

//===================================================================//
//===============      Computations               ===================//
//===================================================================//
/// Set Computation File Names
std::string GetComputationFileName( int file_type )
{
    // The CFD Mesh outputs are kept by the CFD settings under their own
    // enumeration; everything else lives in the Vehicle's own table.  Mirror
    // the routing SetComputationFileName does.
    std::string ret;
    if ( file_type == CFD_STL_TYPE )
        ret = GetVehicle()->GetCfdSettingsPtr()->GetExportFileName( CFD_STL_FILE_NAME );
    else if ( file_type == CFD_POLY_TYPE )
        ret = GetVehicle()->GetCfdSettingsPtr()->GetExportFileName( CFD_POLY_FILE_NAME );
    else if ( file_type == CFD_TRI_TYPE )
        ret = GetVehicle()->GetCfdSettingsPtr()->GetExportFileName( CFD_TRI_FILE_NAME );
    else if ( file_type == CFD_FACET_TYPE )
        ret = GetVehicle()->GetCfdSettingsPtr()->GetExportFileName( CFD_FACET_FILE_NAME );
    else if ( file_type == CFD_OBJ_TYPE )
        ret = GetVehicle()->GetCfdSettingsPtr()->GetExportFileName( CFD_OBJ_FILE_NAME );
    else if ( file_type == CFD_DAT_TYPE )
        ret = GetVehicle()->GetCfdSettingsPtr()->GetExportFileName( CFD_DAT_FILE_NAME );
    else if ( file_type == CFD_KEY_TYPE )
        ret = GetVehicle()->GetCfdSettingsPtr()->GetExportFileName( CFD_KEY_FILE_NAME );
    else if ( file_type == CFD_GMSH_TYPE )
        ret = GetVehicle()->GetCfdSettingsPtr()->GetExportFileName( CFD_GMSH_FILE_NAME );
    else if ( file_type == CFD_TKEY_TYPE )
        ret = GetVehicle()->GetCfdSettingsPtr()->GetExportFileName( CFD_TKEY_FILE_NAME );
    else if ( file_type == CFD_VSPGEOM_TYPE )
        ret = GetVehicle()->GetCfdSettingsPtr()->GetExportFileName( CFD_VSPGEOM_FILE_NAME );
    else if ( file_type == CFD_POGS_TYPE )
        ret = GetVehicle()->GetCfdSettingsPtr()->GetExportFileName( CFD_POGS_FILE_NAME );
    else
        ret = GetVehicle()->getExportFileName( file_type );

    ErrorMgr.NoError();
    return ret;
}

void SetComputationFileName( int file_type, const std::string & file_name )
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
    if ( file_type == CFD_POGS_TYPE )
        GetVehicle()->GetCfdSettingsPtr()->SetExportFileName( file_name, CFD_POGS_FILE_NAME );

    ErrorMgr.NoError();
}

/// Compute Mass Properties on The Components in the Set
std::string ComputeMassProps( int set, int num_slices, int idir )
{
    Update();

    std::string id = GetVehicle()->MassPropsAndFlatten( set, vsp::SET_NONE, num_slices, idir);

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
std::string ComputeCompGeom( int set, bool half_mesh, int file_export_types )
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

    std::string id = veh->CompGeomAndFlatten( set, half_mesh );

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
    std::string modeID;

    veh->CreateDegenGeom( set, useMode, modeID );
    veh->WriteDegenGeomFile();
    ErrorMgr.NoError();
}

//==== Compute Plane Slice =====//
std::string ComputePlaneSlice( int set, int num_slices, const vec3d & norm, bool auto_bnd, double start_bnd, double end_bnd, bool measureduct )
{
    Update();
    Vehicle* veh = GetVehicle();

    bool useMode = false;
    std::string modeID;

    std::string id = veh->PSliceAndFlatten( set, num_slices, norm, auto_bnd, start_bnd, end_bnd, measureduct, useMode, modeID );

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
double GetCFDMeshVal( int type )
{
    double ret = 0.0;

    if ( type == CFD_MIN_EDGE_LEN )
        ret = GetVehicle()->GetCfdGridDensityPtr()->m_MinLen();
    else if ( type == CFD_MAX_EDGE_LEN )
        ret = GetVehicle()->GetCfdGridDensityPtr()->m_BaseLen();
    else if ( type == CFD_MAX_GAP )
        ret = GetVehicle()->GetCfdGridDensityPtr()->m_MaxGap();
    else if ( type == CFD_NUM_CIRCLE_SEGS )
        ret = GetVehicle()->GetCfdGridDensityPtr()->m_NCircSeg();
    else if ( type == CFD_GROWTH_RATIO )
        ret = GetVehicle()->GetCfdGridDensityPtr()->m_GrowRatio();
    else if ( type == CFD_LIMIT_GROWTH_FLAG )
        ret = GetVehicle()->GetCfdGridDensityPtr()->GetRigorLimit();
    else if ( type == CFD_INTERSECT_SUBSURFACE_FLAG )
        ret = GetVehicle()->GetCfdSettingsPtr()->m_IntersectSubSurfs();
    else if ( type == CFD_HALF_MESH_FLAG )
        ret = GetVehicle()->GetCfdSettingsPtr()->m_HalfMeshFlag();
    else if ( type == CFD_FAR_FIELD_FLAG )
        ret = GetVehicle()->GetCfdSettingsPtr()->m_FarMeshFlag();
    else if ( type == CFD_FAR_MAX_EDGE_LEN )
        ret = GetVehicle()->GetCfdGridDensityPtr()->m_FarMaxLen();
    else if ( type == CFD_FAR_MAX_GAP )
        ret = GetVehicle()->GetCfdGridDensityPtr()->m_FarMaxGap();
    else if ( type == CFD_FAR_NUM_CIRCLE_SEGS )
        ret = GetVehicle()->GetCfdGridDensityPtr()->m_FarNCircSeg();
    else if ( type == CFD_FAR_SIZE_ABS_FLAG )
        ret = GetVehicle()->GetCfdSettingsPtr()->m_FarAbsSizeFlag();
    else if ( type == CFD_FAR_LENGTH )
        ret = GetVehicle()->GetCfdSettingsPtr()->m_FarLength();
    else if ( type == CFD_FAR_WIDTH )
        ret = GetVehicle()->GetCfdSettingsPtr()->m_FarWidth();
    else if ( type == CFD_FAR_HEIGHT )
        ret = GetVehicle()->GetCfdSettingsPtr()->m_FarHeight();
    else if ( type == CFD_FAR_X_SCALE )
        ret = GetVehicle()->GetCfdSettingsPtr()->m_FarXScale();
    else if ( type == CFD_FAR_Y_SCALE )
        ret = GetVehicle()->GetCfdSettingsPtr()->m_FarYScale();
    else if ( type == CFD_FAR_Z_SCALE )
        ret = GetVehicle()->GetCfdSettingsPtr()->m_FarZScale();
    else if ( type == CFD_FAR_LOC_MAN_FLAG )
        ret = GetVehicle()->GetCfdSettingsPtr()->m_FarManLocFlag();
    else if ( type == CFD_FAR_LOC_X )
        ret = GetVehicle()->GetCfdSettingsPtr()->m_FarXLocation();
    else if ( type == CFD_FAR_LOC_Y )
        ret = GetVehicle()->GetCfdSettingsPtr()->m_FarYLocation();
    else if ( type == CFD_FAR_LOC_Z )
        ret = GetVehicle()->GetCfdSettingsPtr()->m_FarZLocation();
    else if ( type == CFD_SPLIT_JOIN_SURFS_FLAG )
        ret = GetVehicle()->GetCfdSettingsPtr()->m_SplitJoinSurfsFlag();
    else if ( type == CFD_PARALLEL_MESH_FLAG )
        ret = GetVehicle()->GetCfdSettingsPtr()->m_ParallelMeshFlag();
    else if ( type == CFD_POGS_NUM_REFINE )
        ret = GetVehicle()->GetCfdSettingsPtr()->m_POGSNRef();
    else
    {
        ErrorMgr.AddError( VSP_CANT_FIND_TYPE, "GetCFDMeshVal::Can't Find Type " + to_string( ( long long )type ) );
        return ret;
    }

    ErrorMgr.NoError();
    return ret;
}

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
    else if ( type == CFD_SPLIT_JOIN_SURFS_FLAG )
        GetVehicle()->GetCfdSettingsPtr()->m_SplitJoinSurfsFlag = ToBool(val);
    else if ( type == CFD_PARALLEL_MESH_FLAG )
        GetVehicle()->GetCfdSettingsPtr()->m_ParallelMeshFlag = ToBool(val);
    else if ( type == CFD_POGS_NUM_REFINE )
        GetVehicle()->GetCfdSettingsPtr()->m_POGSNRef = val;
    else
    {
        ErrorMgr.AddError( VSP_CANT_FIND_TYPE, "SetCFDMeshVal::Can't Find Type " + to_string( ( long long )type ) );
        return;
    }

    ErrorMgr.NoError();
}
/// Turn On/Off Wake For Component
bool GetCFDWakeFlag( const std::string & geom_id )
{
    Vehicle* veh = GetVehicle();
    Geom* geom_ptr = veh->FindGeom( geom_id );
    if ( !geom_ptr )
    {
        ErrorMgr.AddError( VSP_INVALID_GEOM_ID, "GetCFDWakeFlag::Can't Find Geom " + geom_id );
        return false;
    }

    ErrorMgr.NoError();
    return geom_ptr->GetWakeActiveFlag();
}

void SetCFDWakeFlag( const std::string & geom_id, bool flag )
{
    Vehicle* veh = GetVehicle();
    Geom* geom_ptr = veh->FindGeom( geom_id );
    if ( !geom_ptr )
    {
        ErrorMgr.AddError( VSP_INVALID_GEOM_ID, "SetCFDWakeFlag::Can't Find Geom " + geom_id );
        return;
    }

    geom_ptr->SetWakeActiveFlag( flag );

    if ( !geom_ptr->HasWingTypeSurfs() )
    {
        ErrorMgr.AddError( VSP_WRONG_GEOM_TYPE, "SetCFDWakeFlag::Geom " + geom_id + " is not a wing-type surface" );
    }
    else
    {
        ErrorMgr.NoError();
    }
}

/// Name The Geom Standing In For The Far Field Box
void SetCFDFarFieldGeomID( const string & geom_id )
{
    Vehicle* veh = GetVehicle();

    // An empty ID is how the setting reads before anything has been chosen, so allow it
    // back in to clear the choice.
    if ( !geom_id.empty() && !veh->FindGeom( geom_id ) )
    {
        ErrorMgr.AddError( VSP_INVALID_GEOM_ID, "SetCFDFarFieldGeomID::Can't Find Geom " + geom_id );
        return;
    }

    veh->GetCfdSettingsPtr()->SetFarGeomID( geom_id );

    ErrorMgr.NoError();
}

/// The Geom Standing In For The Far Field Box
string GetCFDFarFieldGeomID()
{
    ErrorMgr.NoError();
    return GetVehicle()->GetCfdSettingsPtr()->GetFarGeomID();
}

/// Add A CFD Source
std::string AddCFDSource( int type, const std::string & geom_id, int surf_index,
                          double l1, double r1, double u1, double w1,
                          double l2, double r2, double u2, double w2 )
{
    Vehicle* veh = GetVehicle();
    Geom* geom_ptr = veh->FindGeom( geom_id );
    if ( !geom_ptr )
    {
        ErrorMgr.AddError( VSP_INVALID_GEOM_ID, "AddCFDSource::Can't Find Geom " + geom_id );
        return std::string();
    }

    if ( surf_index < 0 || surf_index >= geom_ptr->GetNumMainSurfs() )
    {
        ErrorMgr.AddError( VSP_INDEX_OUT_RANGE, "AddCFDSource::Geom " + geom_id + " has no main surface " + to_string( surf_index ) );
        return std::string();
    }

    CfdMeshMgr.SetCurrSourceGeomID( geom_id );
    CfdMeshMgr.SetCurrMainSurfIndx( surf_index );
    BaseSource* source =  CfdMeshMgr.AddSource( type );

    if ( !source )
    {
        ErrorMgr.AddError( VSP_INVALID_TYPE, "AddCFDSource::Invalid Source Type " + to_string( type ) );
        return std::string();
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
    return source->GetID();
}

/// Delete All CFD Sources
//==== Look up a Geom by ID, reporting through the named caller. ====//
Geom* FindGeomForSources( const std::string & geom_id, const std::string & caller )
{
    Vehicle* veh = GetVehicle();
    Geom* geom_ptr = veh->FindGeom( geom_id );
    if ( !geom_ptr )
    {
        ErrorMgr.AddError( VSP_INVALID_GEOM_ID, caller + "::Can't Find Geom " + geom_id );
        return nullptr;
    }
    return geom_ptr;
}

//==== The Geom a CFD source is on and the source's index there, reporting through
//==== the named caller when it cannot be found. ====//
Geom* FindCFDSourceGeom( const std::string & source_id, int & index, const std::string & caller )
{
    std::vector< Geom* > geom_vec = GetVehicle()->GetGeomStoreVec();
    for ( size_t i = 0; i < geom_vec.size(); i++ )
    {
        std::vector< BaseSource* > source_vec = geom_vec[i]->GetCfdMeshMainSourceVec();
        for ( int j = 0; j < ( int )source_vec.size(); j++ )
        {
            if ( source_vec[j]->GetID() == source_id )
            {
                index = j;
                return geom_vec[i];
            }
        }
    }

    ErrorMgr.AddError( VSP_INVALID_ID, caller + "::Can't Find CFD Source " + source_id );
    return nullptr;
}

std::vector < std::string > GetCFDSourceIDVec( const std::string & geom_id )
{
    std::vector < std::string > ret_vec;

    Geom* geom_ptr = FindGeomForSources( geom_id, "GetCFDSourceIDVec" );
    if ( !geom_ptr )
    {
        return ret_vec;
    }

    std::vector< BaseSource* > source_vec = geom_ptr->GetCfdMeshMainSourceVec();
    for ( size_t i = 0; i < source_vec.size(); i++ )
    {
        ret_vec.push_back( source_vec[i]->GetID() );
    }

    ErrorMgr.NoError();
    return ret_vec;
}

int GetNumCFDSources( const std::string & geom_id )
{
    Geom* geom_ptr = FindGeomForSources( geom_id, "GetNumCFDSources" );
    if ( !geom_ptr )
    {
        return -1;
    }

    ErrorMgr.NoError();
    return ( int )geom_ptr->GetCfdMeshMainSourceVec().size();
}

std::string GetCFDSourceID( const std::string & geom_id, int source_index )
{
    Geom* geom_ptr = FindGeomForSources( geom_id, "GetCFDSourceID" );
    if ( !geom_ptr )
    {
        return std::string();
    }

    std::vector< BaseSource* > source_vec = geom_ptr->GetCfdMeshMainSourceVec();

    if ( source_index < 0 || source_index >= ( int )source_vec.size() )
    {
        ErrorMgr.AddError( VSP_INDEX_OUT_RANGE, "GetCFDSourceID::Source Index " + to_string( source_index ) + " Out of Range" );
        return std::string();
    }

    ErrorMgr.NoError();
    return source_vec[ source_index ]->GetID();
}

static std::string GetCFDSourceNameImpl( const std::string & geom_id, int source_index, const std::string & caller )
{
    Geom* geom_ptr = FindGeomForSources( geom_id, caller );
    if ( !geom_ptr )
    {
        return std::string();
    }

    std::vector< BaseSource* > source_vec = geom_ptr->GetCfdMeshMainSourceVec();

    if ( source_index < 0 || source_index >= ( int )source_vec.size() )
    {
        ErrorMgr.AddError( VSP_INDEX_OUT_RANGE, caller + "::Source Index " + to_string( source_index ) + " Out of Range" );
        return std::string();
    }

    ErrorMgr.NoError();
    return source_vec[ source_index ]->GetName();
}

std::string GetCFDSourceName( const std::string & geom_id, int source_index )
{
    return GetCFDSourceNameImpl( geom_id, source_index, "GetCFDSourceName" );
}

std::string GetCFDSourceNameByID( const std::string & source_id )
{
    int index = -1;
    Geom* geom_ptr = FindCFDSourceGeom( source_id, index, "GetCFDSourceNameByID" );
    if ( !geom_ptr )
    {
        return std::string();
    }

    return GetCFDSourceNameImpl( geom_ptr->GetID(), index, "GetCFDSourceNameByID" );
}

static void SetCFDSourceNameImpl( const std::string & geom_id, int source_index, const std::string & name, const std::string & caller )
{
    Geom* geom_ptr = FindGeomForSources( geom_id, caller );
    if ( !geom_ptr )
    {
        return;
    }

    std::vector< BaseSource* > source_vec = geom_ptr->GetCfdMeshMainSourceVec();

    if ( source_index < 0 || source_index >= ( int )source_vec.size() )
    {
        ErrorMgr.AddError( VSP_INDEX_OUT_RANGE, caller + "::Source Index " + to_string( source_index ) + " Out of Range" );
        return;
    }

    source_vec[ source_index ]->SetName( name );

    ErrorMgr.NoError();
}

void SetCFDSourceName( const std::string & geom_id, int source_index, const std::string & name )
{
    SetCFDSourceNameImpl( geom_id, source_index, name, "SetCFDSourceName" );
}

void SetCFDSourceNameByID( const std::string & source_id, const std::string & name )
{
    int index = -1;
    Geom* geom_ptr = FindCFDSourceGeom( source_id, index, "SetCFDSourceNameByID" );
    if ( !geom_ptr )
    {
        return;
    }

    SetCFDSourceNameImpl( geom_ptr->GetID(), index, name, "SetCFDSourceNameByID" );
}

static int GetCFDSourceTypeImpl( const std::string & geom_id, int source_index, const std::string & caller )
{
    Geom* geom_ptr = FindGeomForSources( geom_id, caller );
    if ( !geom_ptr )
    {
        return -1;
    }

    std::vector< BaseSource* > source_vec = geom_ptr->GetCfdMeshMainSourceVec();

    if ( source_index < 0 || source_index >= ( int )source_vec.size() )
    {
        ErrorMgr.AddError( VSP_INDEX_OUT_RANGE, caller + "::Source Index " + to_string( source_index ) + " Out of Range" );
        return -1;
    }

    ErrorMgr.NoError();
    return source_vec[ source_index ]->GetType();
}

int GetCFDSourceType( const std::string & geom_id, int source_index )
{
    return GetCFDSourceTypeImpl( geom_id, source_index, "GetCFDSourceType" );
}

int GetCFDSourceTypeByID( const std::string & source_id )
{
    int index = -1;
    Geom* geom_ptr = FindCFDSourceGeom( source_id, index, "GetCFDSourceTypeByID" );
    if ( !geom_ptr )
    {
        return -1;
    }

    return GetCFDSourceTypeImpl( geom_ptr->GetID(), index, "GetCFDSourceTypeByID" );
}

static void DeleteCFDSourceImpl( const std::string & geom_id, int source_index, const std::string & caller )
{
    Geom* geom_ptr = FindGeomForSources( geom_id, caller );
    if ( !geom_ptr )
    {
        return;
    }

    if ( source_index < 0 || source_index >= ( int )geom_ptr->GetCfdMeshMainSourceVec().size() )
    {
        ErrorMgr.AddError( VSP_INDEX_OUT_RANGE, caller + "::Source Index " + to_string( source_index ) + " Out of Range" );
        return;
    }

    int prev_curr = geom_ptr->GetCurrSourceID();

    geom_ptr->SetCurrSourceID( source_index );
    geom_ptr->DelCurrSource();
    geom_ptr->SetCurrSourceID( prev_curr );

    ErrorMgr.NoError();
}

void DeleteCFDSource( const std::string & geom_id, int source_index )
{
    DeleteCFDSourceImpl( geom_id, source_index, "DeleteCFDSource" );
}

void DeleteCFDSourceByID( const std::string & source_id )
{
    int index = -1;
    Geom* geom_ptr = FindCFDSourceGeom( source_id, index, "DeleteCFDSourceByID" );
    if ( !geom_ptr )
    {
        return;
    }

    DeleteCFDSourceImpl( geom_ptr->GetID(), index, "DeleteCFDSourceByID" );
}

void AdjustAllCFDSourceLen( double mult )
{
    if ( mult <= 0.0 )
    {
        ErrorMgr.AddError( VSP_INVALID_INPUT_VAL, "AdjustAllCFDSourceLen::Multiplier " + to_string( mult ) + " Must Be Positive" );
        return;
    }

    CfdMeshMgr.AdjustAllSourceLen( mult );

    ErrorMgr.NoError();
}

void AdjustAllCFDSourceRad( double mult )
{
    if ( mult <= 0.0 )
    {
        ErrorMgr.AddError( VSP_INVALID_INPUT_VAL, "AdjustAllCFDSourceRad::Multiplier " + to_string( mult ) + " Must Be Positive" );
        return;
    }

    CfdMeshMgr.AdjustAllSourceRad( mult );

    ErrorMgr.NoError();
}

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
    if ( file_export_types & CFD_POGS_TYPE )
        veh->GetCfdSettingsPtr()->SetFileExportFlag( CFD_POGS_FILE_NAME, true );

    veh->GetCfdSettingsPtr()->m_SelectedSetIndex = set;
    veh->GetCfdSettingsPtr()->m_SelectedDegenSetIndex = degenset;
    CfdMeshMgr.GenerateMesh();
    ErrorMgr.NoError();
}

void SetCFDMeshModeID( const std::string & mode_id )
{
    if ( !mode_id.empty() && !ModeMgr.GetMode( mode_id ) )
    {
        ErrorMgr.AddError( VSP_INVALID_ID, "SetCFDMeshModeID::Can't Find Mode " + mode_id );
        return;
    }

    GetVehicle()->GetCfdSettingsPtr()->m_ModeID = mode_id;
    ErrorMgr.NoError();
}

std::string GetCFDMeshModeID()
{
    ErrorMgr.NoError();
    return GetVehicle()->GetCfdSettingsPtr()->m_ModeID;
}

void SetIntersectModeID( const std::string & mode_id )
{
    if ( !mode_id.empty() && !ModeMgr.GetMode( mode_id ) )
    {
        ErrorMgr.AddError( VSP_INVALID_ID, "SetIntersectModeID::Can't Find Mode " + mode_id );
        return;
    }

    GetVehicle()->GetISectSettingsPtr()->m_ModeID = mode_id;
    ErrorMgr.NoError();
}

std::string GetIntersectModeID()
{
    ErrorMgr.NoError();
    return GetVehicle()->GetISectSettingsPtr()->m_ModeID;
}

//==== The total index of a Structure by ID, reporting through the named caller ====//
int FindTotalFeaStructIndex( const std::string & struct_id, const std::string & caller )
{
    FeaStructure* feastruct = StructureMgr.GetFeaStruct( struct_id );
    if ( !feastruct )
    {
        ErrorMgr.AddError( VSP_INVALID_ID, caller + "::Can't Find FeaStructure " + struct_id );
        return -1;
    }
    return StructureMgr.GetTotFeaStructIndex( feastruct );
}

//==== The ID of the Structure at a total index, or an empty string ====//
std::string TotalFeaStructID( int index )
{
    FeaStructure* feastruct = StructureMgr.GetFeaStruct( index );
    if ( !feastruct )
    {
        return std::string();
    }
    return feastruct->GetID();
}

void SetSTEPStructureExportID( const std::string & struct_id )
{
    int index = FindTotalFeaStructIndex( struct_id, "SetSTEPStructureExportID" );
    if ( index < 0 )
    {
        return;
    }

    GetVehicle()->m_STEPStructureExportIndex = index;
    ErrorMgr.NoError();
}

std::string GetSTEPStructureExportID()
{
    ErrorMgr.NoError();
    return TotalFeaStructID( GetVehicle()->m_STEPStructureExportIndex() );
}

void SetIGESStructureExportID( const std::string & struct_id )
{
    int index = FindTotalFeaStructIndex( struct_id, "SetIGESStructureExportID" );
    if ( index < 0 )
    {
        return;
    }

    GetVehicle()->m_IGESStructureExportIndex = index;
    ErrorMgr.NoError();
}

std::string GetIGESStructureExportID()
{
    ErrorMgr.NoError();
    return TotalFeaStructID( GetVehicle()->m_IGESStructureExportIndex() );
}

void SetAeroStructTargetID( const std::string & id )
{
    AeroStructMgr.Update();

    int index = vector_find_val( AeroStructMgr.m_StructAssyIDVec, id );
    if ( index < 0 )
    {
        ErrorMgr.AddError( VSP_INVALID_ID, "SetAeroStructTargetID::Can't Find FEA Structure or Assembly " + id );
        return;
    }

    AeroStructMgr.m_CurrStructAssyIndex = index;
    AeroStructMgr.Update();
    ErrorMgr.NoError();
}

std::string GetAeroStructTargetID()
{
    AeroStructMgr.Update();

    int index = AeroStructMgr.m_CurrStructAssyIndex();
    ErrorMgr.NoError();
    if ( index < 0 || index >= ( int )AeroStructMgr.m_StructAssyIDVec.size() )
    {
        return std::string();
    }
    return AeroStructMgr.m_StructAssyIDVec[ index ];
}

/// Get/Set reference wing
std::string GetVSPAERORefWingID()
{
    Vehicle* veh = GetVehicle();
    if ( !veh )
    {
        ErrorMgr.AddError( VSP_INVALID_PTR, "GetVSPAERORefWingID::Can't Find Vehicle" );
        return std::string();
    }

    if ( VSPAEROMgr.m_RefFlag.Get() != vsp::COMPONENT_REF )
    {
        ErrorMgr.NoError();
        return std::string();
    }

    Geom* geom_ptr = veh->FindGeom( VSPAEROMgr.m_RefGeomID );
    if ( !geom_ptr )
    {
        ErrorMgr.AddError( VSP_INVALID_PTR, "GetVSPAERORefWingID::Can't Find Geom " + VSPAEROMgr.m_RefGeomID );
        return std::string();
    }

    ErrorMgr.NoError();
    return VSPAEROMgr.m_RefGeomID;
}

std::string SetVSPAERORefWingID( const std::string & geom_id )
{
    Vehicle* veh = GetVehicle();
    if (!veh)
    {
        ErrorMgr.AddError( VSP_INVALID_PTR, "SetVSPAERORefWingID::Can't Find Vehicle" );
        return std::string();
    }

    Geom* geom_ptr = veh->FindGeom( geom_id );
    if ( !geom_ptr )
    {
        ErrorMgr.AddError( VSP_INVALID_GEOM_ID, "SetVSPAERORefWingID::Can't Find Geom " + geom_id );
        return  std::string();
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
    ErrorMgr.NoError();
}

//==== The index of a control surface group by ID, reporting through the named caller ====//
int FindControlSurfaceGroupIndex( const std::string & group_id, const std::string & caller )
{
    std::vector < ControlSurfaceGroup* > cs_vec = VSPAEROMgr.GetControlSurfaceGroupVec();
    for ( int i = 0; i < ( int )cs_vec.size(); i++ )
    {
        if ( cs_vec[i] && cs_vec[i]->GetID() == group_id )
        {
            return i;
        }
    }

    ErrorMgr.AddError( VSP_INVALID_ID, caller + "::Can't Find Control Surface Group " + group_id );
    return -1;
}

//==== The position of a control surface in a list, or -1 ====//
int FindControlSurface( const std::vector < VspAeroControlSurf > & cs_vec, const std::string & ss_id, int reflect_index )
{
    for ( int i = 0; i < ( int )cs_vec.size(); i++ )
    {
        if ( cs_vec[i].SSID == ss_id && ( int )cs_vec[i].iReflect == reflect_index )
        {
            return i;
        }
    }
    return -1;
}

//==== The subsurface IDs and reflection indexes of a list of control surfaces ====//
std::vector < std::string > ControlSurfaceSubSurfIDs( const std::vector < VspAeroControlSurf > & cs_vec )
{
    std::vector < std::string > ret_vec( cs_vec.size() );
    for ( size_t i = 0; i < cs_vec.size(); i++ )
    {
        ret_vec[i] = cs_vec[i].SSID;
    }
    return ret_vec;
}

std::vector < int > ControlSurfaceReflects( const std::vector < VspAeroControlSurf > & cs_vec )
{
    std::vector < int > ret_vec( cs_vec.size() );
    for ( size_t i = 0; i < cs_vec.size(); i++ )
    {
        ret_vec[i] = cs_vec[i].iReflect;
    }
    return ret_vec;
}

static void DeleteVSPAEROControlSurfaceGroupImpl( int CSGroupIndex, const std::string & caller )
{
    if ( CSGroupIndex < 0 || CSGroupIndex >= GetNumControlSurfaceGroups() )
    {
        ErrorMgr.AddError( VSP_INDEX_OUT_RANGE, caller + "::CSGroupIndex " + to_string( CSGroupIndex ) + " out of range" );
        return;
    }

    VSPAEROMgr.SetCurrentCSGroupIndex( CSGroupIndex );
    VSPAEROMgr.RemoveControlSurfaceGroup();

    ErrorMgr.NoError();
}

void DeleteVSPAEROControlSurfaceGroup( int CSGroupIndex )
{
    DeleteVSPAEROControlSurfaceGroupImpl( CSGroupIndex, "DeleteVSPAEROControlSurfaceGroup" );
}

void DeleteVSPAEROControlSurfaceGroupByID( const std::string & group_id )
{
    int index = FindControlSurfaceGroupIndex( group_id, "DeleteVSPAEROControlSurfaceGroupByID" );
    if ( index < 0 )
    {
        return;
    }

    DeleteVSPAEROControlSurfaceGroupImpl( index, "DeleteVSPAEROControlSurfaceGroupByID" );
}

std::string AddCpSlice( int cut_type, double location )
{
    CpSlice* slice = VSPAEROMgr.AddCpSlice();

    if ( !slice )
    {
        ErrorMgr.AddError( VSP_INVALID_PTR, "AddCpSlice::Failed To Add Cp Slice" );
        return std::string();
    }

    slice->m_CutType.Set( cut_type );
    slice->m_CutPosition.Set( location );

    ErrorMgr.NoError();
    return slice->GetID();
}

int GetNumCpSlices()
{
    int ret = ( int )VSPAEROMgr.GetCpSliceVec().size();

    ErrorMgr.NoError();
    return ret;
}

std::string GetCpSliceID( int slice_index )
{
    if ( !VSPAEROMgr.ValidCpSliceInd( slice_index ) )
    {
        ErrorMgr.AddError( VSP_INDEX_OUT_RANGE, "GetCpSliceID::Slice Index " + to_string( slice_index ) + " Out of Range" );
        return std::string();
    }

    ErrorMgr.NoError();
    return VSPAEROMgr.GetCpSlice( slice_index )->GetID();
}

std::vector < std::string > GetCpSliceIDVec()
{
    std::vector < std::string > ret_vec;

    std::vector < CpSlice* > slice_vec = VSPAEROMgr.GetCpSliceVec();
    for ( size_t i = 0; i < slice_vec.size(); i++ )
    {
        ret_vec.push_back( slice_vec[i]->GetID() );
    }

    ErrorMgr.NoError();
    return ret_vec;
}

static void DelCpSliceImpl( int slice_index, const std::string & caller )
{
    if ( !VSPAEROMgr.ValidCpSliceInd( slice_index ) )
    {
        ErrorMgr.AddError( VSP_INDEX_OUT_RANGE, caller + "::Slice Index " + to_string( slice_index ) + " Out of Range" );
        return;
    }

    VSPAEROMgr.DelCpSlice( slice_index );

    ErrorMgr.NoError();
}

void DelCpSlice( int slice_index )
{
    DelCpSliceImpl( slice_index, "DelCpSlice" );
}

void DelCpSliceByID( const std::string & slice_id )
{
    int index = VSPAEROMgr.GetCpSliceIndex( slice_id );
    if ( !VSPAEROMgr.ValidCpSliceInd( index ) )
    {
        ErrorMgr.AddError( VSP_INVALID_ID, "DelCpSliceByID::Can't Find Cp Slice " + slice_id );
        return;
    }

    DelCpSliceImpl( index, "DelCpSliceByID" );
}

void DeleteAllCpSlices()
{
    VSPAEROMgr.ClearCpSliceVec();

    ErrorMgr.NoError();
}

void DeleteAllVarPresetGroups()
{
    VarPresetMgr.DeleteAllSettingGroups();

    ErrorMgr.NoError();
}

void DeleteAllVarPresetSettings( const std::string &group_id )
{
    if ( !VarPresetMgr.FindSettingGroup( group_id ) )
    {
        ErrorMgr.AddError( VSP_INVALID_ID, "DeleteAllVarPresetSettings::Invalid Var Preset Group ID " + group_id );
        return;
    }

    VarPresetMgr.DeleteAllSettingsInGroup( group_id );

    ErrorMgr.NoError();
}

int CreateVSPAEROControlSurfaceGroup()
{
    VSPAEROMgr.Update();
    VSPAEROMgr.AddControlSurfaceGroup();
    ErrorMgr.NoError();
    return VSPAEROMgr.GetCurrentCSGroupIndex();
}

std::string AddVSPAEROControlSurfaceGroup()
{
    return FindControlSurfaceGroup( CreateVSPAEROControlSurfaceGroup() );
}

static void AddAllToVSPAEROControlSurfaceGroupImpl( int CSGroupIndex, const std::string & caller )
{
    if ( CSGroupIndex < 0 || CSGroupIndex >= GetNumControlSurfaceGroups() )
    {
        ErrorMgr.AddError( VSP_INDEX_OUT_RANGE, caller + "::CSGroupIndex " + to_string( CSGroupIndex ) + " out of range" );
        return;
    }

    VSPAEROMgr.SetCurrentCSGroupIndex( CSGroupIndex );

    VSPAEROMgr.Update();

    VSPAEROMgr.AddAllToCSGroup();

    ErrorMgr.NoError();
}

void AddAllToVSPAEROControlSurfaceGroup( int CSGroupIndex )
{
    AddAllToVSPAEROControlSurfaceGroupImpl( CSGroupIndex, "AddAllToVSPAEROControlSurfaceGroup" );
}

void AddAllToVSPAEROControlSurfaceGroupByID( const std::string & group_id )
{
    int index = FindControlSurfaceGroupIndex( group_id, "AddAllToVSPAEROControlSurfaceGroupByID" );
    if ( index < 0 )
    {
        return;
    }

    AddAllToVSPAEROControlSurfaceGroupImpl( index, "AddAllToVSPAEROControlSurfaceGroupByID" );
}

static void RemoveAllFromVSPAEROControlSurfaceGroupImpl( int CSGroupIndex, const std::string & caller )
{
    if ( CSGroupIndex < 0 || CSGroupIndex >= GetNumControlSurfaceGroups() )
    {
        ErrorMgr.AddError( VSP_INDEX_OUT_RANGE, caller + "::CSGroupIndex " + to_string( CSGroupIndex ) + " out of range" );
        return;
    }

    VSPAEROMgr.SetCurrentCSGroupIndex( CSGroupIndex );

    VSPAEROMgr.Update();

    VSPAEROMgr.RemoveAllFromCSGroup();

    ErrorMgr.NoError();
}

void RemoveAllFromVSPAEROControlSurfaceGroup( int CSGroupIndex )
{
    RemoveAllFromVSPAEROControlSurfaceGroupImpl( CSGroupIndex, "RemoveAllFromVSPAEROControlSurfaceGroup" );
}

void RemoveAllFromVSPAEROControlSurfaceGroupByID( const std::string & group_id )
{
    int index = FindControlSurfaceGroupIndex( group_id, "RemoveAllFromVSPAEROControlSurfaceGroupByID" );
    if ( index < 0 )
    {
        return;
    }

    RemoveAllFromVSPAEROControlSurfaceGroupImpl( index, "RemoveAllFromVSPAEROControlSurfaceGroupByID" );
}

static std::vector < std::string > GetActiveCSNameVecImpl( int CSGroupIndex, const std::string & caller )
{
    if ( CSGroupIndex < 0 || CSGroupIndex >= GetNumControlSurfaceGroups() )
    {
        ErrorMgr.AddError( VSP_INDEX_OUT_RANGE, caller + "::CSGroupIndex " + to_string( CSGroupIndex ) + " out of range" );
        return {};
    }

    VSPAEROMgr.SetCurrentCSGroupIndex( CSGroupIndex );

    VSPAEROMgr.Update();

    if ( !VSPAEROMgr.GetActiveCSVecPtr() )
    {
        ErrorMgr.AddError( VSP_INVALID_PTR, caller + "::Can't Find Active Control Surfaces of Group " + to_string( CSGroupIndex ) );
        return std::vector < std::string >();
    }

    std::vector < VspAeroControlSurf > active_cs_vec = *VSPAEROMgr.GetActiveCSVecPtr();
    std::vector < std::string > return_vec( active_cs_vec.size() );

    for ( size_t i = 0; i < return_vec.size(); i++ )
    {
        return_vec[i] = active_cs_vec[i].fullName;
    }

    ErrorMgr.NoError();
    return return_vec;
}

std::vector < std::string > GetActiveCSNameVec( int CSGroupIndex )
{
    return GetActiveCSNameVecImpl( CSGroupIndex, "GetActiveCSNameVec" );
}

std::vector < std::string > GetActiveCSNameVecByID( const std::string & group_id )
{
    int index = FindControlSurfaceGroupIndex( group_id, "GetActiveCSNameVecByID" );
    if ( index < 0 )
    {
        return std::vector < std::string > ();
    }

    return GetActiveCSNameVecImpl( index, "GetActiveCSNameVecByID" );
}

std::vector < std::string > GetActiveCSSubSurfIDVec( const std::string & group_id )
{
    int index = FindControlSurfaceGroupIndex( group_id, "GetActiveCSSubSurfIDVec" );
    if ( index < 0 )
    {
        return std::vector < std::string > ();
    }

    VSPAEROMgr.Update();

    ErrorMgr.NoError();
    return ControlSurfaceSubSurfIDs( VSPAEROMgr.GetControlSurfaceGroupVec()[ index ]->m_ControlSurfVec );
}

std::vector < int > GetActiveCSReflectVec( const std::string & group_id )
{
    int index = FindControlSurfaceGroupIndex( group_id, "GetActiveCSReflectVec" );
    if ( index < 0 )
    {
        return std::vector < int > ();
    }

    VSPAEROMgr.Update();

    ErrorMgr.NoError();
    return ControlSurfaceReflects( VSPAEROMgr.GetControlSurfaceGroupVec()[ index ]->m_ControlSurfVec );
}

std::vector < std::string > GetCompleteCSNameVec( )
{
    VSPAEROMgr.Update();

    std::vector < VspAeroControlSurf > complete_cs_vec = VSPAEROMgr.GetCompleteCSVec();
    std::vector < std::string > return_vec( complete_cs_vec.size() );

    for ( size_t i = 0; i < return_vec.size(); i++ )
    {
        return_vec[i] = complete_cs_vec[i].fullName;
    }

    ErrorMgr.NoError();
    return return_vec;
}

std::vector < std::string > GetCompleteCSSubSurfIDVec()
{
    VSPAEROMgr.Update();

    ErrorMgr.NoError();
    return ControlSurfaceSubSurfIDs( VSPAEROMgr.GetCompleteCSVec() );
}

std::vector < int > GetCompleteCSReflectVec()
{
    VSPAEROMgr.Update();

    ErrorMgr.NoError();
    return ControlSurfaceReflects( VSPAEROMgr.GetCompleteCSVec() );
}

static std::vector < std::string > GetAvailableCSNameVecImpl( int CSGroupIndex, const std::string & caller )
{
    if ( CSGroupIndex < 0 || CSGroupIndex >= GetNumControlSurfaceGroups() )
    {
        ErrorMgr.AddError( VSP_INDEX_OUT_RANGE, caller + "::CSGroupIndex " + to_string( CSGroupIndex ) + " out of range" );
        return {};
    }

    VSPAEROMgr.SetCurrentCSGroupIndex( CSGroupIndex );

    VSPAEROMgr.Update();

    std::vector < VspAeroControlSurf > avail_cs_vec = VSPAEROMgr.GetAvailableCSVec();
    std::vector < std::string > return_vec( avail_cs_vec.size() );

    for ( size_t i = 0; i < return_vec.size(); i++ )
    {
        return_vec[i] = avail_cs_vec[i].fullName;
    }

    ErrorMgr.NoError();
    return return_vec;
}

std::vector < std::string > GetAvailableCSNameVec( int CSGroupIndex )
{
    return GetAvailableCSNameVecImpl( CSGroupIndex, "GetAvailableCSNameVec" );
}

std::vector < std::string > GetAvailableCSNameVecByID( const std::string & group_id )
{
    int index = FindControlSurfaceGroupIndex( group_id, "GetAvailableCSNameVecByID" );
    if ( index < 0 )
    {
        return std::vector < std::string > ();
    }

    return GetAvailableCSNameVecImpl( index, "GetAvailableCSNameVecByID" );
}

std::vector < std::string > GetAvailableCSSubSurfIDVec( const std::string & group_id )
{
    int index = FindControlSurfaceGroupIndex( group_id, "GetAvailableCSSubSurfIDVec" );
    if ( index < 0 )
    {
        return std::vector < std::string > ();
    }

    VSPAEROMgr.SetCurrentCSGroupIndex( index );
    VSPAEROMgr.Update();

    ErrorMgr.NoError();
    return ControlSurfaceSubSurfIDs( VSPAEROMgr.GetAvailableCSVec() );
}

std::vector < int > GetAvailableCSReflectVec( const std::string & group_id )
{
    int index = FindControlSurfaceGroupIndex( group_id, "GetAvailableCSReflectVec" );
    if ( index < 0 )
    {
        return std::vector < int > ();
    }

    VSPAEROMgr.SetCurrentCSGroupIndex( index );
    VSPAEROMgr.Update();

    ErrorMgr.NoError();
    return ControlSurfaceReflects( VSPAEROMgr.GetAvailableCSVec() );
}

static void SetVSPAEROControlGroupNameImpl( const std::string & name, int CSGroupIndex, const std::string & caller )
{
    if ( CSGroupIndex < 0 || CSGroupIndex >= GetNumControlSurfaceGroups() )
    {
        ErrorMgr.AddError( VSP_INDEX_OUT_RANGE, caller + "::CSGroupIndex " + to_string( CSGroupIndex ) + " out of range" );
        return;
    }

    VSPAEROMgr.SetCurrentCSGroupIndex( CSGroupIndex );

    VSPAEROMgr.Update();

    VSPAEROMgr.SetCurrentCSGroupName(name);

    ErrorMgr.NoError();
}

void SetVSPAEROControlGroupName( const std::string & name, int CSGroupIndex )
{
    SetVSPAEROControlGroupNameImpl( name, CSGroupIndex, "SetVSPAEROControlGroupName" );
}

void SetVSPAEROControlGroupNameByID( const std::string & name, const std::string & group_id )
{
    int index = FindControlSurfaceGroupIndex( group_id, "SetVSPAEROControlGroupNameByID" );
    if ( index < 0 )
    {
        return;
    }

    SetVSPAEROControlGroupNameImpl( name, index, "SetVSPAEROControlGroupNameByID" );
}

static std::string GetVSPAEROControlGroupNameImpl( int CSGroupIndex, const std::string & caller )
{
    if ( CSGroupIndex < 0 || CSGroupIndex >= GetNumControlSurfaceGroups() )
    {
        ErrorMgr.AddError( VSP_INDEX_OUT_RANGE, caller + "::CSGroupIndex " + to_string( CSGroupIndex ) + " out of range" );
        return std::string();
    }

    VSPAEROMgr.SetCurrentCSGroupIndex( CSGroupIndex );

    VSPAEROMgr.Update();

    ErrorMgr.NoError();
    return VSPAEROMgr.GetCurrentCSGGroupName();
}

std::string GetVSPAEROControlGroupName( int CSGroupIndex )
{
    return GetVSPAEROControlGroupNameImpl( CSGroupIndex, "GetVSPAEROControlGroupName" );
}

std::string GetVSPAEROControlGroupNameByID( const std::string & group_id )
{
    int index = FindControlSurfaceGroupIndex( group_id, "GetVSPAEROControlGroupNameByID" );
    if ( index < 0 )
    {
        return std::string();
    }

    return GetVSPAEROControlGroupNameImpl( index, "GetVSPAEROControlGroupNameByID" );
}

void AddSelectedToCSGroup( const std::vector <int> &selected, int CSGroupIndex)
{
    if ( CSGroupIndex < 0 || CSGroupIndex >= GetNumControlSurfaceGroups() )
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
            ErrorMgr.AddError( VSP_INDEX_OUT_RANGE, "AddSelectedToCSGroup::Selected Index " + to_string( selected[i] ) + " Out of Range (indexing must be one based)" );
            return;
        }
    }

    VSPAEROMgr.m_SelectedUngroupedCS = selected;

    VSPAEROMgr.Update();

    VSPAEROMgr.AddSelectedToCSGroup();

    ErrorMgr.NoError();
}

void RemoveSelectedFromCSGroup( const std::vector <int> &selected, int CSGroupIndex)
{
    if ( CSGroupIndex < 0 || CSGroupIndex >= GetNumControlSurfaceGroups() )
    {
        ErrorMgr.AddError( VSP_INDEX_OUT_RANGE, "RemoveSelectedFromCSGroup::CSGroupIndex " + to_string( CSGroupIndex ) + " out of range" );
        return;
    }

    VSPAEROMgr.SetCurrentCSGroupIndex( CSGroupIndex );

    if ( !VSPAEROMgr.GetActiveCSVecPtr() )
    {
        ErrorMgr.AddError( VSP_INVALID_PTR, "RemoveSelectedFromCSGroup::Can't Find Active Control Surfaces of Group " + to_string( CSGroupIndex ) );
        return;
    }

    int max_cs_index = VSPAEROMgr.GetActiveCSVecPtr()->size();

    if ( selected.size() == 0 || selected.size() > max_cs_index )
    {
        ErrorMgr.AddError( VSP_INVALID_INPUT_VAL, "RemoveSelectedFromCSGroup::selected out of range" );
        return;
    }

    for ( size_t i = 0; i < selected.size(); i++ )
    {
        if ( selected[i] <= 0 || selected[i] > max_cs_index )
        {
            ErrorMgr.AddError( VSP_INDEX_OUT_RANGE, "RemoveSelectedFromCSGroup::Selected Index " + to_string( selected[i] ) + " Out of Range (indexing must be one based)" );
            return;
        }
    }

    VSPAEROMgr.m_SelectedGroupedCS = selected;

    VSPAEROMgr.Update();

    VSPAEROMgr.RemoveSelectedFromCSGroup();

    ErrorMgr.NoError();
}

void AddSubSurfToCSGroup( const std::string & group_id, const std::string & ss_id, int reflect_index )
{
    int index = FindControlSurfaceGroupIndex( group_id, "AddSubSurfToCSGroup" );
    if ( index < 0 )
    {
        return;
    }

    VSPAEROMgr.SetCurrentCSGroupIndex( index );
    VSPAEROMgr.Update();

    int pos = FindControlSurface( VSPAEROMgr.GetAvailableCSVec(), ss_id, reflect_index );
    if ( pos < 0 )
    {
        ErrorMgr.AddError( VSP_INVALID_ID, "AddSubSurfToCSGroup::SubSurface " + ss_id + " reflection " + to_string( reflect_index ) + " is not a control surface outside group " + group_id );
        return;
    }

    std::vector < int > selected( 1, pos + 1 );
    AddSelectedToCSGroup( selected, index );
}

void RemoveSubSurfFromCSGroup( const std::string & group_id, const std::string & ss_id, int reflect_index )
{
    int index = FindControlSurfaceGroupIndex( group_id, "RemoveSubSurfFromCSGroup" );
    if ( index < 0 )
    {
        return;
    }

    VSPAEROMgr.Update();

    int pos = FindControlSurface( VSPAEROMgr.GetControlSurfaceGroupVec()[ index ]->m_ControlSurfVec, ss_id, reflect_index );
    if ( pos < 0 )
    {
        ErrorMgr.AddError( VSP_INVALID_ID, "RemoveSubSurfFromCSGroup::SubSurface " + ss_id + " reflection " + to_string( reflect_index ) + " is not in group " + group_id );
        return;
    }

    std::vector < int > selected( 1, pos + 1 );
    RemoveSelectedFromCSGroup( selected, index );
}

int GetNumControlSurfaceGroups()
{
    ErrorMgr.NoError();
    return VSPAEROMgr.GetControlSurfaceGroupVec().size();
}

std::string FindControlSurfaceGroup( int group_index )
{
    vector < ControlSurfaceGroup* > cs_vec = VSPAEROMgr.GetControlSurfaceGroupVec();

    if ( group_index < 0 || group_index >= ( int )cs_vec.size() )
    {
        ErrorMgr.AddError( VSP_INDEX_OUT_RANGE, "FindControlSurfaceGroup::group_index " +
                           to_string( group_index ) + " out of range" );
        return std::string();
    }

    if ( !cs_vec[ group_index ] )
    {
        ErrorMgr.AddError( VSP_INVALID_PTR, "FindControlSurfaceGroup::Group " +
                           to_string( group_index ) + " is empty" );
        return std::string();
    }

    ErrorMgr.NoError();
    return cs_vec[ group_index ]->GetID();
}

std::vector < std::string > GetControlSurfaceGroupIDVec()
{
    std::vector < std::string > ret_vec;

    std::vector < ControlSurfaceGroup* > cs_vec = VSPAEROMgr.GetControlSurfaceGroupVec();
    for ( size_t i = 0; i < cs_vec.size(); i++ )
    {
        if ( cs_vec[i] )
        {
            ret_vec.push_back( cs_vec[i]->GetID() );
        }
    }

    ErrorMgr.NoError();
    return ret_vec;
}


//===================================================================//
//=========       VSPAERO Unsteady Group Functions        ===========//
//===================================================================//
//==== The index of an unsteady group by ID, reporting through the named caller ====//
int FindUnsteadyGroupIndex( const std::string & group_id, const std::string & caller )
{
    VSPAEROMgr.UpdateUnsteadyGroups();

    int index = VSPAEROMgr.GetUnsteadyGroupIndex( group_id );
    if ( !VSPAEROMgr.ValidUnsteadyGroupInd( index ) )
    {
        ErrorMgr.AddError( VSP_INVALID_ID, caller + "::Can't Find Unsteady Group " + group_id );
        return -1;
    }
    return index;
}

std::string FindUnsteadyGroup( int group_index )
{
    VSPAEROMgr.UpdateUnsteadyGroups();

    if ( !VSPAEROMgr.ValidUnsteadyGroupInd( group_index ) )
    {
        ErrorMgr.AddError( VSP_INDEX_OUT_RANGE, "FindUnsteadyGroup::group_index " + to_string( group_index ) + " out of range" );
        return std::string();
    }

    UnsteadyGroup* group = VSPAEROMgr.GetUnsteadyGroup( group_index );
    VSPAEROMgr.SetCurrentUnsteadyGroupIndex( group_index ); // Need if RPM is uniform
    ErrorMgr.NoError();
    return group->GetID();
}

static std::string GetUnsteadyGroupNameImpl( int group_index, const std::string & caller )
{
    VSPAEROMgr.UpdateUnsteadyGroups();

    if ( !VSPAEROMgr.ValidUnsteadyGroupInd( group_index ) )
    {
        ErrorMgr.AddError( VSP_INDEX_OUT_RANGE, caller + "::group_index " + to_string( group_index ) + " out of range" );
        return std::string();
    }

    UnsteadyGroup* group = VSPAEROMgr.GetUnsteadyGroup( group_index );
    VSPAEROMgr.SetCurrentUnsteadyGroupIndex( group_index ); // Need if RPM is uniform
    ErrorMgr.NoError();
    return group->GetName();
}

std::string GetUnsteadyGroupName( int group_index )
{
    return GetUnsteadyGroupNameImpl( group_index, "GetUnsteadyGroupName" );
}

std::string GetUnsteadyGroupNameByID( const std::string & group_id )
{
    int index = FindUnsteadyGroupIndex( group_id, "GetUnsteadyGroupNameByID" );
    if ( index < 0 )
    {
        return std::string();
    }

    return GetUnsteadyGroupNameImpl( index, "GetUnsteadyGroupNameByID" );
}

static std::vector < std::string > GetUnsteadyGroupCompIDsImpl( int group_index, const std::string & caller )
{
    std::vector < std::string > ret_vec;
    VSPAEROMgr.UpdateUnsteadyGroups();

    if ( !VSPAEROMgr.ValidUnsteadyGroupInd( group_index ) )
    {
        ErrorMgr.AddError( VSP_INDEX_OUT_RANGE, caller + "::group_index " + to_string( group_index ) + " out of range" );
        return ret_vec;
    }

    UnsteadyGroup* group = VSPAEROMgr.GetUnsteadyGroup( group_index );

    std::vector < pair < std::string, int > > comp_vec = group->GetCompSurfPairVec();
    ret_vec.resize( comp_vec.size() );

    for ( size_t i = 0; i < comp_vec.size(); i++ )
    {
        ret_vec[i] = comp_vec[i].first;
    }

    VSPAEROMgr.SetCurrentUnsteadyGroupIndex( group_index ); // Need if RPM is uniform
    ErrorMgr.NoError();
    return ret_vec;
}

std::vector < std::string > GetUnsteadyGroupCompIDs( int group_index )
{
    return GetUnsteadyGroupCompIDsImpl( group_index, "GetUnsteadyGroupCompIDs" );
}

std::vector < std::string > GetUnsteadyGroupCompIDsByID( const std::string & group_id )
{
    int index = FindUnsteadyGroupIndex( group_id, "GetUnsteadyGroupCompIDsByID" );
    if ( index < 0 )
    {
        return std::vector < std::string > ();
    }

    return GetUnsteadyGroupCompIDsImpl( index, "GetUnsteadyGroupCompIDsByID" );
}

static std::vector < int > GetUnsteadyGroupSurfIndexesImpl( int group_index, const std::string & caller )
{
    std::vector < int > ret_vec;
    VSPAEROMgr.UpdateUnsteadyGroups();

    if ( !VSPAEROMgr.ValidUnsteadyGroupInd( group_index ) )
    {
        ErrorMgr.AddError( VSP_INDEX_OUT_RANGE, caller + "::group_index " + to_string( group_index ) + " out of range" );
        return ret_vec;
    }

    UnsteadyGroup* group = VSPAEROMgr.GetUnsteadyGroup( group_index );

    std::vector < pair < std::string, int > > comp_vec = group->GetCompSurfPairVec();
    ret_vec.resize( comp_vec.size() );

    for ( size_t i = 0; i < comp_vec.size(); i++ )
    {
        ret_vec[i] = comp_vec[i].second;
    }

    VSPAEROMgr.SetCurrentUnsteadyGroupIndex( group_index ); // Need if RPM is uniform
    ErrorMgr.NoError();
    return ret_vec;
}

std::vector < int > GetUnsteadyGroupSurfIndexes( int group_index )
{
    return GetUnsteadyGroupSurfIndexesImpl( group_index, "GetUnsteadyGroupSurfIndexes" );
}

std::vector < int > GetUnsteadyGroupSurfIndexesByID( const std::string & group_id )
{
    int index = FindUnsteadyGroupIndex( group_id, "GetUnsteadyGroupSurfIndexesByID" );
    if ( index < 0 )
    {
        return std::vector < int > ();
    }

    return GetUnsteadyGroupSurfIndexesImpl( index, "GetUnsteadyGroupSurfIndexesByID" );
}

int GetNumUnsteadyGroups()
{
    VSPAEROMgr.UpdateUnsteadyGroups();

    ErrorMgr.NoError();
    return VSPAEROMgr.NumUnsteadyGroups();
}

std::vector < std::string > GetUnsteadyGroupIDVec()
{
    std::vector < std::string > ret_vec;

    VSPAEROMgr.UpdateUnsteadyGroups();

    for ( int i = 0; i < VSPAEROMgr.NumUnsteadyGroups(); i++ )
    {
        ret_vec.push_back( VSPAEROMgr.GetUnsteadyGroupID( i ) );
    }

    ErrorMgr.NoError();
    return ret_vec;
}

int GetNumUnsteadyRotorGroups()
{
    VSPAEROMgr.UpdateUnsteadyGroups();

    ErrorMgr.NoError();
    return VSPAEROMgr.NumUnsteadyRotorGroups();
}

//===================================================================//
//=========       VSPAERO Actuator Disk Functions        ============//
//===================================================================//
std::string FindActuatorDisk( int disk_index )
{
    VSPAEROMgr.UpdateRotorDisks();

    if ( !VSPAEROMgr.ValidRotorDiskIndex( disk_index ) )
    {
        ErrorMgr.AddError( VSP_INDEX_OUT_RANGE, "FindActuatorDisk::disk_index " + to_string( disk_index ) + " out of range" );
        return std::string();
    }

    RotorDisk* disk = VSPAEROMgr.GetRotorDisk( disk_index );
    ErrorMgr.NoError();
    return disk->GetID();
}

std::string FindActuatorDiskByGeom( const std::string & geom_id, int surf_index )
{
    if ( !GetVehicle()->FindGeom( geom_id ) )
    {
        ErrorMgr.AddError( VSP_INVALID_GEOM_ID, "FindActuatorDiskByGeom::Can't Find Geom " + geom_id );
        return std::string();
    }

    VSPAEROMgr.UpdateRotorDisks();

    std::vector < RotorDisk* > disk_vec = VSPAEROMgr.GetRotorDiskVec();
    for ( size_t i = 0; i < disk_vec.size(); i++ )
    {
        if ( disk_vec[i]->GetParentID() == geom_id && ( int )disk_vec[i]->GetSurfNum() == surf_index )
        {
            ErrorMgr.NoError();
            return disk_vec[i]->GetID();
        }
    }

    ErrorMgr.AddError( VSP_INVALID_ID, "FindActuatorDiskByGeom::No Actuator Disk For Geom " + geom_id + " Surface " + to_string( surf_index ) );
    return std::string();
}

std::string GetActuatorDiskGeomID( const std::string & disk_id )
{
    VSPAEROMgr.UpdateRotorDisks();

    RotorDisk* disk = dynamic_cast < RotorDisk* > ( ParmMgr.FindParmContainer( disk_id ) );
    if ( !disk )
    {
        ErrorMgr.AddError( VSP_INVALID_ID, "GetActuatorDiskGeomID::Can't Find Actuator Disk " + disk_id );
        return std::string();
    }

    ErrorMgr.NoError();
    return disk->GetParentID();
}

int GetNumActuatorDisks()
{
    VSPAEROMgr.UpdateRotorDisks();

    ErrorMgr.NoError();
    return VSPAEROMgr.GetRotorDiskVec().size();
}

std::vector < std::string > GetActuatorDiskIDVec()
{
    std::vector < std::string > ret_vec;

    VSPAEROMgr.UpdateRotorDisks();

    std::vector < RotorDisk* > disk_vec = VSPAEROMgr.GetRotorDiskVec();
    for ( size_t i = 0; i < disk_vec.size(); i++ )
    {
        ret_vec.push_back( disk_vec[i]->GetID() );
    }

    ErrorMgr.NoError();
    return ret_vec;
}

//===================================================================//
//===============       Analysis Functions        ===================//
//===================================================================//

int GetNumAnalysis()
{
  ErrorMgr.NoError();
  return AnalysisMgr.GetNumAnalysis();
}

std::vector < std::string > ListAnalysis()
{
    ErrorMgr.NoError();
    return AnalysisMgr.ListAnalysis();
}

std::vector < std::string > GetAnalysisInputNames( const std::string & analysis )
{
    if ( !AnalysisMgr.ValidAnalysisName( analysis ) )
    {
        ErrorMgr.AddError( VSP_INVALID_ID, "GetAnalysisInputNames::Invalid Analysis ID " + analysis );
        std::vector < std::string > ret;
        return ret;
    }

    Analysis *a = AnalysisMgr.FindAnalysis( analysis );

    ErrorMgr.NoError();
    return a->m_Inputs.GetAllDataNames();
}

std::string GetAnalysisDoc( const std::string & analysis )
{
    if ( !AnalysisMgr.ValidAnalysisName( analysis ) )
    {
        ErrorMgr.AddError( VSP_INVALID_ID, "GetAnalysisDoc::Invalid Analysis ID " + analysis );
        return "";
    }

    Analysis *a = AnalysisMgr.FindAnalysis( analysis );

    ErrorMgr.NoError();
    return a->m_Inputs.GetDoc();
}

std::string GetAnalysisInputDoc( const std::string & analysis, const std::string & name )
{
    if ( !AnalysisMgr.ValidAnalysisName( analysis ) )
    {
        ErrorMgr.AddError( VSP_INVALID_ID, "GetAnalysisInputDoc::Invalid Analysis ID " + analysis );
        return "";
    }
    ErrorMgr.NoError();

    return AnalysisMgr.GetAnalysisInputDoc( analysis, name );
}

std::string ExecAnalysis( const std::string & analysis )
{
    if ( !AnalysisMgr.ValidAnalysisName( analysis ) )
    {
        ErrorMgr.AddError( VSP_INVALID_ID, "ExecAnalysis::Invalid Analysis ID " + analysis );
        std::string ret;
        return ret;
    }

    ErrorMgr.NoError();
    return AnalysisMgr.ExecAnalysis( analysis );
}

int GetNumAnalysisInputData( const std::string & analysis, const std::string & name )
{
    if ( !AnalysisMgr.ValidAnalysisName( analysis ) )
    {
        ErrorMgr.AddError( VSP_INVALID_ID, "GetNumAnalysisInputData::Invalid Analysis ID " + analysis );
        return 0;
    }
    ErrorMgr.NoError();

    return AnalysisMgr.GetNumInputData( analysis, name );
}

int GetAnalysisInputType( const std::string & analysis, const std::string & name )
{
    if ( !AnalysisMgr.ValidAnalysisName( analysis ) )
    {
        ErrorMgr.AddError( VSP_INVALID_ID, "GetAnalysisInputType::Invalid Analysis ID " + analysis );
        return vsp::INVALID_TYPE;
    }
    ErrorMgr.NoError();

    return AnalysisMgr.GetAnalysisInputType( analysis, name );
}

const std::vector< int > & GetIntAnalysisInput( const std::string & analysis, const std::string & name, int index )
{
    if ( !AnalysisMgr.ValidAnalysisName( analysis ) )
    {
        ErrorMgr.AddError( VSP_INVALID_ID, "GetIntAnalysisInput::Invalid Analysis ID " + analysis );
    }
    else if ( !AnalysisMgr.ValidAnalysisInputDataIndex( analysis, name, index ) )
    {
        ErrorMgr.AddError( VSP_CANT_FIND_NAME, "GetIntAnalysisInput::Can't Find Name " + name + " Index " + to_string( index ) );
    }
    else
    {
        ErrorMgr.NoError();
    }

    return AnalysisMgr.GetIntInputData( analysis, name, index );
}

const std::vector< double > & GetDoubleAnalysisInput( const std::string & analysis, const std::string & name, int index )
{
    if ( !AnalysisMgr.ValidAnalysisName( analysis ) )
    {
        ErrorMgr.AddError( VSP_INVALID_ID, "GetDoubleAnalysisInput::Invalid Analysis ID " + analysis );
    }
    else if ( !AnalysisMgr.ValidAnalysisInputDataIndex( analysis, name, index ) )
    {
        ErrorMgr.AddError( VSP_CANT_FIND_NAME, "GetDoubleAnalysisInput::Can't Find Name " + name + " Index " + to_string( index ) );
    }
    else
    {
        ErrorMgr.NoError();
    }

    return AnalysisMgr.GetDoubleInputData( analysis, name, index );
}

const std::vector< std::string > & GetStringAnalysisInput( const std::string & analysis, const std::string & name, int index )
{
    if ( !AnalysisMgr.ValidAnalysisName( analysis ) )
    {
        ErrorMgr.AddError( VSP_INVALID_ID, "GetStringAnalysisInput::Invalid Analysis ID " + analysis );
    }
    else if ( !AnalysisMgr.ValidAnalysisInputDataIndex( analysis, name, index ) )
    {
        ErrorMgr.AddError( VSP_CANT_FIND_NAME, "GetStringAnalysisInput::Can't Find Name " + name + " Index " + to_string( index ) );
    }
    else
    {
        ErrorMgr.NoError();
    }

    return AnalysisMgr.GetStringInputData( analysis, name, index );
}

const std::vector< vec3d > & GetVec3dAnalysisInput( const std::string & analysis, const std::string & name, int index )
{
    if ( !AnalysisMgr.ValidAnalysisName( analysis ) )
    {
        ErrorMgr.AddError( VSP_INVALID_ID, "GetVec3dAnalysisInput::Invalid Analysis ID " + analysis );
    }
    else if ( !AnalysisMgr.ValidAnalysisInputDataIndex( analysis, name, index ) )
    {
        ErrorMgr.AddError( VSP_CANT_FIND_NAME, "GetVec3dAnalysisInput::Can't Find Name " + name + " Index " + to_string( index ) );
    }
    else
    {
        ErrorMgr.NoError();
    }

    return AnalysisMgr.GetVec3dInputData( analysis, name, index );
}

void SetAnalysisInputDefaults( const std::string & analysis )
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

void SetIntAnalysisInput( const std::string & analysis, const std::string & name, const std::vector< int > & indata, int index )
{
    if ( !AnalysisMgr.ValidAnalysisName( analysis ) )
    {
        ErrorMgr.AddError( VSP_INVALID_ID, "SetIntAnalysisInput::Invalid Analysis ID " + analysis );
    }
    else if ( !AnalysisMgr.ValidAnalysisInputDataIndex( analysis, name, index ) )
    {
        ErrorMgr.AddError( VSP_CANT_FIND_NAME, "SetIntAnalysisInput::Can't Find Name " + name + " Index " + to_string( index ) );
    }
    else
    {
        ErrorMgr.NoError();
    }

    AnalysisMgr.SetIntAnalysisInput( analysis, name, indata, index );
}

void SetDoubleAnalysisInput( const std::string & analysis, const std::string & name, const std::vector< double > & indata, int index )
{
    if ( !AnalysisMgr.ValidAnalysisName( analysis ) )
    {
        ErrorMgr.AddError( VSP_INVALID_ID, "SetDoubleAnalysisInput::Invalid Analysis ID " + analysis );
    }
    else if ( !AnalysisMgr.ValidAnalysisInputDataIndex( analysis, name, index ) )
    {
        ErrorMgr.AddError( VSP_CANT_FIND_NAME, "SetDoubleAnalysisInput::Can't Find Name " + name + " Index " + to_string( index ) );
    }
    else
    {
        ErrorMgr.NoError();
    }

    AnalysisMgr.SetDoubleAnalysisInput( analysis, name, indata, index );
}

void SetStringAnalysisInput( const std::string & analysis, const std::string & name, const std::vector< std::string > & indata, int index )
{
    if ( !AnalysisMgr.ValidAnalysisName( analysis ) )
    {
        ErrorMgr.AddError( VSP_INVALID_ID, "SetStringAnalysisInput::Invalid Analysis ID " + analysis );
    }
    else if ( !AnalysisMgr.ValidAnalysisInputDataIndex( analysis, name, index ) )
    {
        ErrorMgr.AddError( VSP_CANT_FIND_NAME, "SetStringAnalysisInput::Can't Find Name " + name + " Index " + to_string( index ) );
    }
    else
    {
        ErrorMgr.NoError();
    }

    AnalysisMgr.SetStringAnalysisInput( analysis, name, indata, index );
}

void SetVec3dAnalysisInput( const std::string & analysis, const std::string & name, const std::vector< vec3d > & indata, int index )
{
    if ( !AnalysisMgr.ValidAnalysisName( analysis ) )
    {
        ErrorMgr.AddError( VSP_INVALID_ID, "SetVec3dAnalysisInput::Invalid Analysis ID " + analysis );
    }
    else if ( !AnalysisMgr.ValidAnalysisInputDataIndex( analysis, name, index ) )
    {
        ErrorMgr.AddError( VSP_CANT_FIND_NAME, "SetVec3dAnalysisInput::Can't Find Name " + name + " Index " + to_string( index ) );
    }
    else
    {
        ErrorMgr.NoError();
    }

    AnalysisMgr.SetVec3dAnalysisInput( analysis, name, indata, index );
}

void PrintAnalysisInputs( const std::string & analysis_name )
{
    if ( !AnalysisMgr.ValidAnalysisName( analysis_name ) )
    {
        ErrorMgr.AddError( VSP_INVALID_ID, "PrintAnalysisInputs::Invalid Analysis ID " + analysis_name );
        return;
    }

    AnalysisMgr.PrintAnalysisInputs( analysis_name );
    ErrorMgr.NoError();
}

void PrintAnalysisDocs( const std::string & analysis_name )
{
    if ( !AnalysisMgr.ValidAnalysisName( analysis_name ) )
    {
        ErrorMgr.AddError( VSP_INVALID_ID, "PrintAnalysisDocs::Invalid Analysis ID " + analysis_name );
        return;
    }

    AnalysisMgr.PrintAnalysisDocs( analysis_name );
    ErrorMgr.NoError();
}

std::string AddGeometryAnalysis()
{
    std::string ga_id = GeometryAnalysisMgr.AddGeometryAnalysis();

    ErrorMgr.NoError();
    return ga_id;
}

void DeleteGeometryAnalysis( const std::string &id )
{
    if ( !GeometryAnalysisMgr.GetGeometryAnalysis( id ) )
    {
        ErrorMgr.AddError( VSP_INVALID_ID, "DeleteGeometryAnalysis::Can't Find Geometry Analysis " + id );
        return;
    }

    GeometryAnalysisMgr.DeleteGeometryAnalysis( id );
    ErrorMgr.NoError();
}

void DeleteAllGeometryAnalyses()
{
    GeometryAnalysisMgr.DeleteAllGeometryAnalyses();
    ErrorMgr.NoError();
}

std::vector < std::string > GetAllGeometryAnalysesIDVec()
{
    std::vector < std::string > ret = GeometryAnalysisMgr.GetAllGeometryAnalysesIDVec();

    ErrorMgr.NoError();
    return ret;
}

void SetActiveGeometryAnalysis( const std::string &id )
{
    // Reject an unknown ID without disturbing the current selection.  An empty string
    // is allowed and clears the active case.
    if ( id != "" && !GeometryAnalysisMgr.GetGeometryAnalysis( id ) )
    {
        ErrorMgr.AddError( VSP_INVALID_ID, "SetActiveGeometryAnalysis::Can't Find Geometry Analysis " + id );
        return;
    }

    GeometryAnalysisMgr.SetActiveGeometryAnalysisID( id );
    ErrorMgr.NoError();
}

std::string GetActiveGeometryAnalysis()
{
    std::string ret = GeometryAnalysisMgr.GetActiveGeometryAnalysisID();

    ErrorMgr.NoError();
    return ret;
}

std::string MakeMeshGeom( const std::string &ga_id )
{
    GeometryAnalysisCase *gcase = GeometryAnalysisMgr.GetGeometryAnalysis( ga_id );
    if ( !gcase )
    {
        ErrorMgr.AddError( VSP_INVALID_ID, "MakeMeshGeom::Can't Find Geometry Analysis " + ga_id );
        return std::string();
    }

    std::string mesh_id = gcase->MakeMeshGeom();

    ErrorMgr.NoError();
    return mesh_id;
}

//===================================================================//
//===============       Attributes Functions         ===================//
//===================================================================//

// Report an ID that names no attribute.
static bool AttributeFound( const std::string & attr_id, const std::string & caller )
{
    if ( !AttributeMgr.GetAttributePtr( attr_id ) )
    {
        ErrorMgr.AddError( VSP_INVALID_ID, caller + "::Can't Find Attribute " + attr_id );
        return false;
    }
    return true;
}

// Report an ID that names no attribute collection.
static bool AttributeCollectionFound( const std::string & coll_id, const std::string & caller )
{
    if ( !AttributeMgr.GetCollectionPtr( coll_id ) )
    {
        ErrorMgr.AddError( VSP_INVALID_ID, caller + "::Can't Find Attribute Collection " + coll_id );
        return false;
    }
    return true;
}

// Report an ID that names neither an attribute collection nor an object that has one.
static bool AttributeParentFound( const std::string & id, const std::string & caller )
{
    if ( !AttributeMgr.GetCollectionPtr( id ) && !AttributeMgr.GetCollectionFromParentID( id ) )
    {
        ErrorMgr.AddError( VSP_INVALID_ID, caller + "::Can't Find Attribute Collection For " + id );
        return false;
    }
    return true;
}

// Report an ID that names nothing the attribute calls know.
static bool AttributeObjectFound( const std::string & id, const std::string & caller )
{
    if ( AttributeMgr.GetObjectType( id ) == vsp::ATTROBJ_FREE && !ParmMgr.FindParmContainer( id ) )
    {
        ErrorMgr.AddError( VSP_INVALID_ID, caller + "::Can't Find Object " + id );
        return false;
    }
    return true;
}

std::string SummarizeAttributes()
{
    ErrorMgr.NoError();
    return AttributeMgr.SummarizeAttributes();
}

std::string SummarizeAttributesAsTree()
{
    ErrorMgr.NoError();
    return AttributeMgr.SummarizeAttributesAsTree();
}

std::vector < std::string > FindAllAttributes()
{
    ErrorMgr.NoError();
    return AttributeMgr.FindAllAttributes();
}

std::vector < std::string > FindAttributesByName( const std::string & search_str )
{
    ErrorMgr.NoError();
    return AttributeMgr.FindAttributesByName( search_str );
}

std::string FindAttributeByName( const std::string & search_str, int index )
{
    ErrorMgr.NoError();
    return AttributeMgr.FindAttributeByName( search_str, index );
}

std::string FindAttributeInCollection( const std::string & obj_id, const std::string & search_str, int index )
{
    if ( AttributeParentFound( obj_id, "FindAttributeInCollection" ) )
    {
        ErrorMgr.NoError();
    }
    return AttributeMgr.FindAttributeInCollection( obj_id, search_str, index );
}

std::vector< std::string > FindAttributeNamesInCollection( const std::string & collID )
{
    if ( !AttributeMgr.GetCollectionPtr( collID ) )
    {
        ErrorMgr.AddError( VSP_INVALID_ID, "FindAttributeNamesInCollection::Can't Find Attribute Collection " + collID );
        return std::vector< std::string >();
    }

    ErrorMgr.NoError();
    return AttributeMgr.FindAttributeNamesInCollection( collID );
}

std::vector< std::string > FindAttributesInCollection( const std::string & collID )
{
    if ( !AttributeMgr.GetCollectionPtr( collID ) )
    {
        ErrorMgr.AddError( VSP_INVALID_ID, "FindAttributesInCollection::Can't Find Attribute Collection " + collID );
        return std::vector< std::string >();
    }

    ErrorMgr.NoError();
    return AttributeMgr.FindAttributesInCollection( collID );
}

std::vector< std::string > FindAttributesInCollectionByName( const std::string & obj_id, const std::string & search_str )
{
    if ( !AttributeMgr.GetCollectionPtr( obj_id ) && !AttributeMgr.GetCollectionFromParentID( obj_id ) )
    {
        ErrorMgr.AddError( VSP_INVALID_ID, "FindAttributesInCollectionByName::Can't Find Attribute Collection " + obj_id );
        return std::vector< std::string >();
    }

    ErrorMgr.NoError();
    return AttributeMgr.FindAttributesInCollection( obj_id, search_str );
}

std::vector< std::string > FindAttributedObjects()
{
    ErrorMgr.NoError();
    return AttributeMgr.FindAttributedObjects();
}

int GetObjectType( const std::string & id )
{
    if ( AttributeObjectFound( id, "GetObjectType" ) )
    {
        ErrorMgr.NoError();
    }
    return AttributeMgr.GetObjectType( id );
}

std::string GetObjectTypeName( const std::string & id )
{
    if ( AttributeObjectFound( id, "GetObjectTypeName" ) )
    {
        ErrorMgr.NoError();
    }
    return AttributeMgr.GetObjectTypeName( id );
}

std::string GetObjectName( const std::string & id )
{
    if ( AttributeObjectFound( id, "GetObjectName" ) )
    {
        ErrorMgr.NoError();
    }
    return AttributeMgr.GetObjectName( id );
}

std::string GetObjectParent( const std::string & id )
{
    if ( AttributeObjectFound( id, "GetObjectParent" ) )
    {
        ErrorMgr.NoError();
    }
    return AttributeMgr.GetObjectParent( id );
}

std::string GetChildCollection( const std::string & id )
{
    if ( AttributeParentFound( id, "GetChildCollection" ) )
    {
        ErrorMgr.NoError();
    }
    return AttributeMgr.GetChildCollection( id );
}

std::string GetGeomSetCollection( const int & index )
{
    std::string coll_id = AttributeMgr.GetGeomSetCollection( index );
    if ( coll_id.empty() )
    {
        ErrorMgr.AddError( VSP_INDEX_OUT_RANGE, "GetGeomSetCollection::User Set Index " + to_string( index ) + " Out of Range" );
        return coll_id;
    }

    ErrorMgr.NoError();
    return coll_id;
}

std::string GetAttributeName( const std::string & attrID )
{
    if ( AttributeFound( attrID, "GetAttributeName" ) )
    {
        ErrorMgr.NoError();
    }
    return AttributeMgr.GetAttributeName( attrID );
}

std::string GetAttributeID( const std::string & collID, const std::string & attributeName, int index )
{
    if ( AttributeCollectionFound( collID, "GetAttributeID" ) )
    {
        ErrorMgr.NoError();
    }
    return AttributeMgr.GetAttributeID( collID, attributeName, index );
}

std::vector< std::string > GetAttributeIDVec( const std::string & collID, const std::string & attributeName )
{
    std::vector< std::string > ret_vec;

    AttributeCollection* coll = AttributeMgr.GetCollectionPtr( collID );
    if ( !coll )
    {
        ErrorMgr.AddError( VSP_INVALID_ID, "GetAttributeIDVec::Can't Find Attribute Collection " + collID );
        return ret_vec;
    }

    for ( int i = 0; coll->FindPtr( attributeName, i ); i++ )
    {
        ret_vec.push_back( coll->FindPtr( attributeName, i )->GetID() );
    }

    ErrorMgr.NoError();
    return ret_vec;
}

std::string GetAttributeDoc( const std::string & attrID )
{
    if ( AttributeFound( attrID, "GetAttributeDoc" ) )
    {
        ErrorMgr.NoError();
    }
    return AttributeMgr.GetAttributeDoc( attrID );
}

int GetAttributeType( const std::string & attrID )
{
    if ( AttributeFound( attrID, "GetAttributeType" ) )
    {
        ErrorMgr.NoError();
    }
    return AttributeMgr.GetAttributeType( attrID );
}

std::string GetAttributeTypeName( const std::string & attrID )
{
    if ( AttributeFound( attrID, "GetAttributeTypeName" ) )
    {
        ErrorMgr.NoError();
    }
    return AttributeMgr.GetAttributeTypeName( attrID );
}

std::vector< int > GetAttributeBoolVal( const std::string & attrID )
{
    if ( AttributeFound( attrID, "GetAttributeBoolVal" ) )
    {
        ErrorMgr.NoError();
    }
    return AttributeMgr.GetAttributeBoolVal( attrID );
}

std::vector< int > GetAttributeIntVal( const std::string & attrID )
{
    if ( AttributeFound( attrID, "GetAttributeIntVal" ) )
    {
        ErrorMgr.NoError();
    }
    return AttributeMgr.GetAttributeIntVal( attrID );
}

std::vector< double > GetAttributeDoubleVal( const std::string & attrID )
{
    if ( AttributeFound( attrID, "GetAttributeDoubleVal" ) )
    {
        ErrorMgr.NoError();
    }
    return AttributeMgr.GetAttributeDoubleVal( attrID );
}

std::vector< std::string > GetAttributeStringVal( const std::string & attrID )
{
    if ( AttributeFound( attrID, "GetAttributeStringVal" ) )
    {
        ErrorMgr.NoError();
    }
    return AttributeMgr.GetAttributeStringVal( attrID );
}

std::vector< std::string >  GetAttributeParmID( const std::string & attrID )
{
    if ( AttributeFound( attrID, "GetAttributeParmID" ) )
    {
        ErrorMgr.NoError();
    }
    return AttributeMgr.GetAttributeParmID( attrID );
}

std::vector< double > GetAttributeParmVal( const std::string & attrID )
{
    if ( AttributeFound( attrID, "GetAttributeParmVal" ) )
    {
        ErrorMgr.NoError();
    }
    return AttributeMgr.GetAttributeParmVal( attrID );
}

std::vector< std::string > GetAttributeParmName( const std::string & attrID )
{
    if ( AttributeFound( attrID, "GetAttributeParmName" ) )
    {
        ErrorMgr.NoError();
    }
    return AttributeMgr.GetAttributeParmName( attrID );
}

std::vector< vec3d > GetAttributeVec3dVal( const std::string & attrID )
{
    if ( AttributeFound( attrID, "GetAttributeVec3dVal" ) )
    {
        ErrorMgr.NoError();
    }
    return AttributeMgr.GetAttributeVec3dVal( attrID );
}

std::vector< std::vector < int > > GetAttributeIntMatrixVal( const std::string & attrID )
{
    if ( AttributeFound( attrID, "GetAttributeIntMatrixVal" ) )
    {
        ErrorMgr.NoError();
    }
    return AttributeMgr.GetAttributeIntMatrixVal( attrID );
}

std::vector< std::vector < double > > GetAttributeDoubleMatrixVal( const std::string & attrID )
{
    if ( AttributeFound( attrID, "GetAttributeDoubleMatrixVal" ) )
    {
        ErrorMgr.NoError();
    }
    return AttributeMgr.GetAttributeDoubleMatrixVal( attrID );
}

void SetAttributeName( const std::string &attrID, const std::string &name )
{
    if ( AttributeFound( attrID, "SetAttributeName" ) )
    {
        ErrorMgr.NoError();
    }
    AttributeMgr.SetAttributeName( attrID, name );
}

void SetAttributeDoc( const std::string & attrID, const std::string & doc )
{
    if ( AttributeFound( attrID, "SetAttributeDoc" ) )
    {
        ErrorMgr.NoError();
    }
    AttributeMgr.SetAttributeDoc( attrID, doc );
}

void SetAttributeBool( const std::string & attrID, bool value )
{
    if ( AttributeFound( attrID, "SetAttributeBool" ) )
    {
        ErrorMgr.NoError();
    }
    AttributeMgr.SetAttributeBool( attrID, value );
}

void SetAttributeInt( const std::string & attrID, int value )
{
    if ( AttributeFound( attrID, "SetAttributeInt" ) )
    {
        ErrorMgr.NoError();
    }
    AttributeMgr.SetAttributeInt( attrID, value );
}

void SetAttributeDouble( const std::string & attrID, double value )
{
    if ( AttributeFound( attrID, "SetAttributeDouble" ) )
    {
        ErrorMgr.NoError();
    }
    AttributeMgr.SetAttributeDouble( attrID, value );
}

void SetAttributeString( const std::string & attrID, const std::string & value )
{
    if ( AttributeFound( attrID, "SetAttributeString" ) )
    {
        ErrorMgr.NoError();
    }
    AttributeMgr.SetAttributeString( attrID, value );
}

void SetAttributeParmID( const std::string & attrID, const std::string & value )
{
    if ( AttributeFound( attrID, "SetAttributeParmID" ) )
    {
        ErrorMgr.NoError();
    }
    AttributeMgr.SetAttributeParmID( attrID, value );
}

void SetAttributeVec3d( const std::string & attrID, const std::vector < vec3d > & value )
{
    if ( AttributeFound( attrID, "SetAttributeVec3d" ) )
    {
        ErrorMgr.NoError();
    }
    AttributeMgr.SetAttributeVec3d( attrID, value );
}

void SetAttributeIntMatrix( const std::string & attrID, const std::vector < std::vector < int > > &value )
{
    if ( AttributeFound( attrID, "SetAttributeIntMatrix" ) )
    {
        ErrorMgr.NoError();
    }
    AttributeMgr.SetAttributeIntMatrix( attrID, value );
}

void SetAttributeDoubleMatrix( const std::string & attrID, const std::vector< std::vector< double > > & value )
{
    if ( AttributeFound( attrID, "SetAttributeDoubleMatrix" ) )
    {
        ErrorMgr.NoError();
    }
    AttributeMgr.SetAttributeDoubleMatrix( attrID, value );
}

void DeleteAttribute( const std::string & attrID )
{
    if ( AttributeFound( attrID, "DeleteAttribute" ) )
    {
        ErrorMgr.NoError();
    }
    AttributeMgr.DeleteAttribute( attrID );
}

std::string AddAttributeBool( const std::string & collID, const std::string & attributeName, bool value )
{
    if ( !AttributeCollectionFound( collID, "AddAttributeBool" ) )
    {
        return std::string();
    }

    ErrorMgr.NoError();
    return AttributeMgr.AddAttributeBool( collID, attributeName, value );
}

std::string AddAttributeInt( const std::string & collID, const std::string & attributeName, int value )
{
    if ( !AttributeCollectionFound( collID, "AddAttributeInt" ) )
    {
        return std::string();
    }

    ErrorMgr.NoError();
    return AttributeMgr.AddAttributeInt( collID, attributeName, value );
}

std::string AddAttributeDouble( const std::string & collID, const std::string & attributeName, double value)
{
    if ( !AttributeCollectionFound( collID, "AddAttributeDouble" ) )
    {
        return std::string();
    }

    ErrorMgr.NoError();
    return AttributeMgr.AddAttributeDouble( collID, attributeName, value );
}

std::string AddAttributeString( const std::string & collID, const std::string & attributeName, const std::string & value )
{
    if ( !AttributeCollectionFound( collID, "AddAttributeString" ) )
    {
        return std::string();
    }

    ErrorMgr.NoError();
    return AttributeMgr.AddAttributeString( collID, attributeName, value );
}

std::string AddAttributeParm( const std::string & collID, const std::string & attributeName, const std::string & parmID )
{
    if ( !AttributeCollectionFound( collID, "AddAttributeParm" ) )
    {
        return std::string();
    }

    ErrorMgr.NoError();
    return AttributeMgr.AddAttributeParm( collID, attributeName, parmID );
}

std::string AddAttributeVec3d( const std::string & collID, const std::string & attributeName, const std::vector < vec3d > & value )
{
    if ( !AttributeCollectionFound( collID, "AddAttributeVec3d" ) )
    {
        return std::string();
    }

    ErrorMgr.NoError();
    return AttributeMgr.AddAttributeVec3d( collID, attributeName, value );
}

std::string AddAttributeIntMatrix( const std::string & collID, const std::string & attributeName, const std::vector < std::vector < int > > & value )
{
    if ( !AttributeCollectionFound( collID, "AddAttributeIntMatrix" ) )
    {
        return std::string();
    }

    ErrorMgr.NoError();
    return AttributeMgr.AddAttributeIntMatrix( collID, attributeName, value );
}

std::string AddAttributeDoubleMatrix( const std::string & collID, const std::string & attributeName, const std::vector < std::vector < double > > & value )
{
    if ( !AttributeCollectionFound( collID, "AddAttributeDoubleMatrix" ) )
    {
        return std::string();
    }

    ErrorMgr.NoError();
    return AttributeMgr.AddAttributeDoubleMatrix( collID, attributeName, value );
}

std::string AddAttributeGroup( const std::string & collID, const std::string & attributeName )
{
    if ( !AttributeCollectionFound( collID, "AddAttributeGroup" ) )
    {
        return std::string();
    }

    ErrorMgr.NoError();
    return AttributeMgr.AddAttributeGroup( collID, attributeName );
}

int CopyAttribute( const std::string & attrID )
{
    if ( AttributeFound( attrID, "CopyAttribute" ) )
    {
        ErrorMgr.NoError();
    }
    return AttributeMgr.CopyAttribute( attrID );
}

void CutAttribute( const std::string & attrID )
{
    if ( AttributeFound( attrID, "CutAttribute" ) )
    {
        ErrorMgr.NoError();
    }
    AttributeMgr.CutAttribute( attrID );
}

std::vector < std::string > PasteAttribute( const std::string & objID )
{
    if ( !AttributeParentFound( objID, "PasteAttribute" ) )
    {
        return std::vector < std::string >();
    }

    ErrorMgr.NoError();
    return AttributeMgr.PasteAttribute( objID );
}

//===================================================================//
//===============       Results Functions         ===================//
//===================================================================//
/// Get all results names available
std::vector<std::string> GetAllResultsNames()
{
    ErrorMgr.NoError();
    return ResultsMgr.GetAllResultsNames();
}

/// Get all data names available for this result
std::vector< std::string > GetAllDataNames( const std::string & results_id )
{
    if ( !ResultsMgr.ValidResultsID( results_id ) )
    {
        ErrorMgr.AddError( VSP_INVALID_ID, "GetAllDataNames::Invalid ID " + results_id );
        std::vector<std::string> ret_vec;
        return ret_vec;
    }
    ErrorMgr.NoError();
    return ResultsMgr.GetAllDataNames( results_id );
}

/// Get the number of results with the given name.
int GetNumResults( const std::string & name )
{
    ErrorMgr.NoError();
    return ResultsMgr.GetNumResults( name );
}

/// Get the name of the results object with the given id
std::string GetResultsName( const std::string & results_id )
{
    if ( !ResultsMgr.ValidResultsID( results_id ) )
    {
        ErrorMgr.AddError( VSP_INVALID_ID, "GetResultsName::Invalid ID " + results_id );
        std::string ret_str;
        return  ret_str;
    }

    ErrorMgr.NoError();
    return ResultsMgr.FindResultsPtr( results_id )->GetName();
}

std::string GetResultsSetDoc( const std::string & results_id )
{
    if ( !ResultsMgr.ValidResultsID( results_id ) )
    {
        ErrorMgr.AddError( VSP_INVALID_ID, "GetResultsSetDoc::Invalid ID " + results_id );
        return  "";
    }

    ErrorMgr.NoError();
    return ResultsMgr.FindResultsPtr( results_id )->GetDoc();
}

std::string GetResultsEntryDoc( const std::string & results_id, const std::string & data_name )
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
std::string FindResultsID( const std::string & name, int index )
{
    std::string id = ResultsMgr.FindResultsID( name, index );
    if ( id.size() == 0 )
    {
        ErrorMgr.AddError( VSP_CANT_FIND_NAME, "FindResultsID::Can't Find Name " + name + " at index " + to_string( index ) );
        return id;
    }
    ErrorMgr.NoError();
    return id;
}

std::vector < std::string > FindResultsIDs( const std::string & name )
{
    std::vector < std::string > ret_vec;

    for ( int i = 0; i < ResultsMgr.GetNumResults( name ); i++ )
    {
        ret_vec.push_back( ResultsMgr.FindResultsID( name, i ) );
    }

    ErrorMgr.NoError();
    return ret_vec;
}

std::vector < std::string > GetAllResultsIDs()
{
    std::vector < std::string > ret_vec;

    // The names come from an unordered map; sort them so the IDs come in a stable order
    std::vector < std::string > names = ResultsMgr.GetAllResultsNames();
    std::sort( names.begin(), names.end() );

    for ( size_t i = 0; i < names.size(); i++ )
    {
        for ( int j = 0; j < ResultsMgr.GetNumResults( names[i] ); j++ )
        {
            ret_vec.push_back( ResultsMgr.FindResultsID( names[i], j ) );
        }
    }

    ErrorMgr.NoError();
    return ret_vec;
}

/// Return the id of the latest results with the given results name
std::string FindLatestResultsID( const std::string & name )
{
    std::string id = ResultsMgr.FindLatestResultsID( name );
    if ( id.size() == 0 )
    {
        ErrorMgr.AddError( VSP_CANT_FIND_NAME, "FindLatestResultsID::Can't Find Name " + name );
        return id;
    }
    ErrorMgr.NoError();
    return id;
}

/// Return the number of data entries given results_id and data name
extern int GetNumData( const std::string & results_id, const std::string & data_name )
{
    if ( !ResultsMgr.ValidResultsID( results_id ) )
    {
        ErrorMgr.AddError( VSP_INVALID_ID, "GetNumData::Invalid ID " + results_id );
        return 0;
    }
    ErrorMgr.NoError();
    return ResultsMgr.GetNumData( results_id, data_name );
}

extern int GetResultsType( const std::string & results_id, const std::string & data_name )
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
const std::vector<int> & GetIntResults( const std::string & id, const std::string & name, int index )
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
const std::vector<double> & GetDoubleResults( const std::string & id, const std::string & name, int index )
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
const std::vector< std::vector<double> > & GetDoubleMatResults( const std::string & id, const std::string & name, int index )
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
const std::vector<std::string> & GetStringResults( const std::string & id, const std::string & name, int index )
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
const std::vector<vec3d> & GetVec3dResults( const std::string & id, const std::string & name, int index )
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
extern std::string CreateGeomResults( const std::string & geom_id, const std::string & name )
{
    Vehicle* veh = GetVehicle();

    Geom* geom_ptr =  veh->FindGeom( geom_id );

    if ( !geom_ptr )
    {
        ErrorMgr.AddError( VSP_INVALID_GEOM_ID, "CreateGeomResults::Can't Find GeomID " + geom_id );
        return std::string();
    }

    std::string res_id = ResultsMgr.CreateGeomResults( geom_id, name );

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
void DeleteResult( const std::string & id )
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
void WriteResultsCSVFile( const std::string & id, const std::string & file_name )
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

void PrintResults( const std::string &results_id )
{
    if ( !ResultsMgr.FindResultsPtr( results_id ) )
    {
        ErrorMgr.AddError( VSP_INVALID_ID, "PrintResults::Invalid ID " + results_id );
        return;
    }

    ResultsMgr.PrintResults( results_id );
    ErrorMgr.NoError();
}

void PrintResultsDocs( const std::string &results_id )
{
    if ( !ResultsMgr.FindResultsPtr( results_id ) )
    {
        ErrorMgr.AddError( VSP_INVALID_ID, "PrintResultsDocs::Invalid ID " + results_id );
        return;
    }

    ResultsMgr.PrintResultsDocs( results_id );
    ErrorMgr.NoError();
}

void WriteTestResults()
{
    ResultsMgr.WriteTestResults();
    ErrorMgr.NoError();
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

void ScreenGrab( const std::string & fname, int w, int h, bool transparentBG, bool autocrop )
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

void SetGeomDrawType(const std::string &geom_id, int type)
{
    Vehicle* veh = GetVehicle();
    Geom* geom_ptr = veh->FindGeom( geom_id );
    if ( !geom_ptr )
    {
        ErrorMgr.AddError( VSP_INVALID_GEOM_ID, "SetGeomDrawType::Can't Find Geom " + geom_id );
        return;
    }
    geom_ptr->m_GuiDraw.SetDrawType( type );

    ErrorMgr.NoError();
}

void SetGeomWireColor( const std::string &geom_id, int r, int g, int b )
{
    Vehicle* veh = GetVehicle();
    Geom* geom_ptr = veh->FindGeom( geom_id );
    if ( !geom_ptr )
    {
        ErrorMgr.AddError( VSP_INVALID_GEOM_ID, "SetGeomWireColor::Can't Find Geom " + geom_id );
        return;
    }
    geom_ptr->SetColor( r, g, b );

    ErrorMgr.NoError();
}

void SetGeomDisplayType(const std::string &geom_id, int type)
{
    Vehicle* veh = GetVehicle();
    Geom* geom_ptr = veh->FindGeom( geom_id );
    if ( !geom_ptr )
    {
        ErrorMgr.AddError( VSP_INVALID_GEOM_ID, "SetGeomDisplayType::Can't Find Geom " + geom_id );
        return;
    }

    CloneGeom* clone_ptr = dynamic_cast< CloneGeom* >( geom_ptr );
    if ( clone_ptr && clone_ptr->GetOriginalGeom() )
    {
        ErrorMgr.AddError( VSP_WRONG_GEOM_TYPE, "SetGeomDisplayType::Geom " + geom_id +
                           " is a Clone -- it is shown the way the Geom it copies is" );
        return;
    }

    geom_ptr->m_GuiDraw.SetDisplayType( type );
    geom_ptr->SetDirtyFlag( GeomBase::TESS );
    geom_ptr->SetLateUpdateFlag( true );

    ErrorMgr.NoError();
}

int GetGeomDrawType( const std::string &geom_id )
{
    Vehicle* veh = GetVehicle();
    Geom* geom_ptr = veh->FindGeom( geom_id );
    if ( !geom_ptr )
    {
        ErrorMgr.AddError( VSP_INVALID_GEOM_ID, "GetGeomDrawType::Can't Find Geom " + geom_id );
        return GEOM_DRAW_WIRE;
    }

    ErrorMgr.NoError();
    return geom_ptr->m_GuiDraw.GetDrawType();
}

int GetGeomDisplayType( const std::string &geom_id )
{
    Vehicle* veh = GetVehicle();
    Geom* geom_ptr = veh->FindGeom( geom_id );
    if ( !geom_ptr )
    {
        ErrorMgr.AddError( VSP_INVALID_GEOM_ID, "GetGeomDisplayType::Can't Find Geom " + geom_id );
        return DISPLAY_BEZIER;
    }

    ErrorMgr.NoError();
    return geom_ptr->m_GuiDraw.GetDisplayType();
}

vec3d GetGeomWireColor( const std::string &geom_id )
{
    Vehicle* veh = GetVehicle();
    Geom* geom_ptr = veh->FindGeom( geom_id );
    if ( !geom_ptr )
    {
        ErrorMgr.AddError( VSP_INVALID_GEOM_ID, "GetGeomWireColor::Can't Find Geom " + geom_id );
        return vec3d();
    }

    ErrorMgr.NoError();
    return geom_ptr->m_GuiDraw.GetWireColor();
}

std::string GetGeomMaterialName( const std::string &geom_id )
{
    Vehicle* veh = GetVehicle();
    Geom* geom_ptr = veh->FindGeom( geom_id );
    if ( !geom_ptr )
    {
        ErrorMgr.AddError( VSP_INVALID_GEOM_ID, "GetGeomMaterialName::Can't Find Geom " + geom_id );
        return std::string();
    }

    const Material *mat = geom_ptr->GetMaterial();
    if ( !mat )
    {
        ErrorMgr.AddError( VSP_INVALID_PTR, "GetGeomMaterialName::Geom " + geom_id + " has no material" );
        return std::string();
    }

    ErrorMgr.NoError();
    return mat->m_Name;
}

void SetGeomMaterialName( const std::string &geom_id, const std::string &name )
{
    Vehicle* veh = GetVehicle();
    Geom* geom_ptr = veh->FindGeom( geom_id );
    if ( !geom_ptr )
    {
        ErrorMgr.AddError( VSP_INVALID_GEOM_ID, "SetGeomMaterialName::Can't Find Geom " + geom_id );
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

void AddMaterial( const std::string &name, const vec3d & ambient, const vec3d & diffuse, const vec3d & specular, const vec3d & emissive, const double & alpha, const double & shininess )
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
    mat.m_UserMaterial = true;

    MaterialMgr.AddMaterial( mat );
    ErrorMgr.NoError();
}

std::vector < std::string > GetMaterialNames()
{
    ErrorMgr.NoError();
    return MaterialMgr.GetNames();
}

int GetNumLights()
{
    ErrorMgr.NoError();
    return ( int )LightMgr.GetVec().size();
}

std::string FindLight( int index )
{
    vector < Light* > lights = LightMgr.GetVec();

    if ( index < 0 || index >= ( int )lights.size() )
    {
        ErrorMgr.AddError( VSP_INDEX_OUT_RANGE, "FindLight::index " + to_string( index ) +
                           " out of range" );
        return std::string();
    }

    if ( !lights[ index ] )
    {
        ErrorMgr.AddError( VSP_INVALID_PTR, "FindLight::Light " + to_string( index ) + " is empty" );
        return std::string();
    }

    ErrorMgr.NoError();
    return lights[ index ]->GetID();
}

std::vector < std::string > GetLightIDVec()
{
    std::vector < std::string > ret_vec;

    vector < Light* > lights = LightMgr.GetVec();
    for ( size_t i = 0; i < lights.size(); i++ )
    {
        if ( lights[i] )
        {
            ret_vec.push_back( lights[i]->GetID() );
        }
    }

    ErrorMgr.NoError();
    return ret_vec;
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
std::vector< std::string > GetGeomTypes()
{
    Vehicle* veh = GetVehicle();

    //==== Load All Type Names ====//
    std::vector< std::string > ret_vec( veh->GetNumGeomTypes() );
    for ( int i = 0 ; i < veh->GetNumGeomTypes() ; i++ )
    {
        ret_vec[i] = veh->GetGeomType( i ).m_Name;
    }

    ErrorMgr.NoError();
    return ret_vec;
}

/// Add a geometry of given type to the vehicle.  The geometry will be a child of
/// optional parent geometry.  The ID string of the geometry will be returned.
std::string AddGeom( const std::string & type, const std::string & parent  )
{
    Vehicle* veh = GetVehicle();

    std::string ret_id;

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
    if ( !TopLevelParentID( parent ) )
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

void UpdateGeom( const std::string & geom_id )
{
    Vehicle* veh = GetVehicle();

    if ( !veh->FindGeom( geom_id ) )
    {
        ErrorMgr.AddError( VSP_INVALID_GEOM_ID, "UpdateGeom::Can't Find Geom " + geom_id );
        return;
    }

    veh->UpdateGeom( geom_id );

    ErrorMgr.NoError();
}


// Validate clone_delete before anything is removed.
static bool ValidCloneDelete( int clone_delete, const std::string & caller )
{
    if ( clone_delete < 0 || clone_delete >= CLONE_DELETE_NUM_TYPES )
    {
        ErrorMgr.AddError( VSP_INVALID_TYPE, caller + "::Clone delete type " + to_string( clone_delete ) + " is not a CLONE_DELETE_TYPE" );
        return false;
    }
    return true;
}

// Reports VSP_CLONE_ORIGINAL_LOST for each Clone left empty, including failed replacements.
// Leaves the call flagged if anything else was reported.
static void ReportClonesOf( Vehicle* veh, const std::vector< std::string > & clone_vec, int n_errors_before, const std::string & caller )
{
    for ( int i = 0; i < ( int )clone_vec.size(); i++ )
    {
        CloneGeom* clone_ptr = dynamic_cast< CloneGeom* >( veh->FindGeom( clone_vec[i] ) );
        if ( clone_ptr && clone_ptr->GetOriginalID().empty() )
        {
            ErrorMgr.AddError( VSP_CLONE_ORIGINAL_LOST, caller + "::" + clone_ptr->GetOriginalLostMessage() );
        }
    }

    if ( ErrorMgr.GetNumTotalErrors() == n_errors_before )
    {
        ErrorMgr.NoError();
    }
}

static void DeleteGeoms( const std::vector< std::string > & del_vec, int clone_delete, const std::string & caller )
{
    Vehicle* veh = GetVehicle();

    if ( !ValidCloneDelete( clone_delete, caller ) )
    {
        return;
    }

    std::vector< std::string > found_vec;
    std::vector< std::string > missing_vec;
    for ( int i = 0; i < ( int )del_vec.size(); i++ )
    {
        if ( veh->FindGeom( del_vec[i] ) )
        {
            found_vec.push_back( del_vec[i] );
        }
        else
        {
            missing_vec.push_back( del_vec[i] );
        }
    }

    int n_errors_before = ErrorMgr.GetNumTotalErrors();
    std::vector< std::string > clone_vec = veh->FindClonesOf( found_vec );

    veh->DeleteGeomVec( found_vec, clone_delete );

    ReportClonesOf( veh, clone_vec, n_errors_before, caller );

    for ( int i = 0; i < ( int )missing_vec.size(); i++ )
    {
        ErrorMgr.AddError( VSP_INVALID_GEOM_ID, caller + "::Can't Find Geom " + missing_vec[i] );
    }
}

void DeleteGeom( const std::string & geom_id, int clone_delete )
{
    DeleteGeoms( { geom_id }, clone_delete, "DeleteGeom" );
}

void DeleteGeomVec( const std::vector< std::string > & del_vec, int clone_delete )
{
    DeleteGeoms( del_vec, clone_delete, "DeleteGeomVec" );
}

/// Cut geometry and place it in the clipboard.  The clipboard is cleared before
/// the cut geom is placed there.
void CutGeomToClipboard( const std::string & geom_id, int clone_delete )
{
    Vehicle* veh = GetVehicle();

    if ( !ValidCloneDelete( clone_delete, "CutGeomToClipboard" ) )
    {
        return;
    }

    if ( !veh->FindGeom( geom_id ) )
    {
        ErrorMgr.AddError( VSP_INVALID_GEOM_ID, "CutGeomToClipboard::Can't Find Geom " + geom_id );
        return;
    }

    int n_errors_before = ErrorMgr.GetNumTotalErrors();
    std::vector< std::string > clone_vec = veh->FindClonesOf( { geom_id } );

    veh->SetActiveGeom( geom_id );
    veh->CutActiveGeomVec( clone_delete );

    ReportClonesOf( veh, clone_vec, n_errors_before, "CutGeomToClipboard" );
}

std::vector< std::string > FindGeomClones( const std::vector< std::string > & geom_id_vec )
{
    Vehicle* veh = GetVehicle();

    std::vector< std::string > clone_vec = veh->FindClonesOf( geom_id_vec );

    ErrorMgr.NoError();
    return clone_vec;
}

/// Copy geometry and place it in the clipboard.  The clipboard is cleared before
/// the geometry is placed there.
void CopyGeomToClipboard( const std::string & geom_id )
{
    Vehicle* veh = GetVehicle();

    if ( !veh->FindGeom( geom_id ) )
    {
        ErrorMgr.AddError( VSP_INVALID_GEOM_ID, "CopyGeomToClipboard::Can't Find Geom " + geom_id );
        return;
    }

    veh->SetActiveGeom( geom_id );
    veh->CopyActiveGeomVec();

    ErrorMgr.NoError();
}

/// Paste the geometry in the clipboard to the vehicle.  The geometry is inserted
/// as a child of the optional parent id
std::vector< std::string > PasteGeomClipboard( const std::string & parent )
{
    Vehicle* veh = GetVehicle();

    Geom* parent_geom = nullptr;
    if ( !TopLevelParentID( parent ) )
    {
        parent_geom = veh->FindGeom( parent );
        if ( !parent_geom )
        {
            ErrorMgr.AddError( VSP_INVALID_GEOM_ID, "PasteGeomClipboard::Can't Find Parent " + parent );
            return std::vector< std::string >();
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

    std::vector< std::string> pasted_ids = veh->PasteClipboard();
    ErrorMgr.NoError();

    return pasted_ids;
}

std::string ConvertFuselageToStack( const std::string & geom_id )
{
    Vehicle* veh = GetVehicle();
    Geom* geom_ptr = veh->FindGeom( geom_id );
    if ( !geom_ptr )
    {
        ErrorMgr.AddError( VSP_INVALID_GEOM_ID, "ConvertFuselageToStack::Can't Find Geom " + geom_id );
        return std::string();
    }

    if ( geom_ptr->GetType().m_Type != FUSELAGE_GEOM_TYPE )
    {
        ErrorMgr.AddError( VSP_WRONG_GEOM_TYPE, "ConvertFuselageToStack::Geom " + geom_id + " is not a Fuselage" );
        return std::string();
    }

    std::string stack_id = veh->ConvertFuselageToStack( geom_id );
    if ( stack_id.empty() )
    {
        ErrorMgr.AddError( VSP_INVALID_ID, "ConvertFuselageToStack::Could not convert Fuselage " + geom_id );
        return std::string();
    }

    ErrorMgr.NoError();
    return stack_id;
}

/// Find and return all geoms
std::vector< std::string > FindGeoms()
{
    Vehicle* veh = GetVehicle();

    std::vector< std::string > ret_vec = veh->GetGeomVec();

    ErrorMgr.NoError();
    return ret_vec;
}

/// Find and return all geoms with given name
std::vector< std::string > FindGeomsWithName( const std::string & name )
{
    std::vector< std::string > ret_vec;
    Vehicle* veh = GetVehicle();

    std::vector< std::string > geom_id_vec = veh->GetGeomVec();
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
std::string FindGeom( const std::string & name, int index )
{
    std::vector< std::string > id_vec;
    Vehicle* veh = GetVehicle();

    std::vector< std::string > geom_id_vec = veh->GetGeomVec();
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
        ErrorMgr.AddError( VSP_CANT_FIND_NAME, "FindGeom::Can't Find Name " + name + " at Index " + to_string( index ) );
        return std::string();
    }
    ErrorMgr.NoError();
    return id_vec[index];
}

/// Set name of geom
void SetGeomName( const std::string & geom_id, const std::string & name )
{
    Vehicle* veh = GetVehicle();
    Geom* geom_ptr = veh->FindGeom( geom_id );
    if ( !geom_ptr )
    {
        ErrorMgr.AddError( VSP_INVALID_GEOM_ID, "SetGeomName::Can't Find Geom " + geom_id );
        return;
    }

    // A self-naming Geom would overwrite this on its next update, so refuse, as the GUI does.
    if ( geom_ptr->NameIsAutomatic() )
    {
        ErrorMgr.AddError( VSP_WRONG_GEOM_TYPE, "SetGeomName::Geom " + geom_id +
                           " names itself and would write over this on its next update" );
        return;
    }

    geom_ptr->SetName( name );

    // A name is not a Parm, so update here for Clones named after this Geom.
    Update();

    ErrorMgr.NoError();
}

/// Get name of geom
std::string GetGeomName( const std::string & geom_id )
{
    std::string ret_name;
    Vehicle* veh = GetVehicle();
    Geom* geom_ptr = veh->FindGeom( geom_id );
    if ( !geom_ptr )
    {
        ErrorMgr.AddError( VSP_INVALID_GEOM_ID, "GetGeomName::Can't Find Geom " + geom_id );
        return std::string();
    }
    ret_name = geom_ptr->GetName();
    ErrorMgr.NoError();
    return ret_name;
}

void SetGeomCloneOriginal( const std::string & clone_id, const std::string & original_id )
{
    Vehicle* veh = GetVehicle();
    Geom* geom_ptr = veh->FindGeom( clone_id );
    if ( !geom_ptr )
    {
        ErrorMgr.AddError( VSP_INVALID_GEOM_ID, "SetGeomCloneOriginal::Can't Find Geom " + clone_id );
        return;
    }

    CloneGeom* clone_ptr = dynamic_cast< CloneGeom* >( geom_ptr );
    if ( !clone_ptr )
    {
        ErrorMgr.AddError( VSP_WRONG_GEOM_TYPE, "SetGeomCloneOriginal::Geom " + clone_id + " is not a Clone" );
        return;
    }

    // An empty ID clears the original; the Clone shows nothing but keeps the Parms it copied.
    if ( original_id.empty() )
    {
        clone_ptr->SetOriginalID( original_id );
        ErrorMgr.NoError();
        return;
    }

    Geom* original_ptr = veh->FindGeom( original_id );
    if ( !original_ptr )
    {
        ErrorMgr.AddError( VSP_INVALID_GEOM_ID, "SetGeomCloneOriginal::Can't Find Geom " + original_id );
        return;
    }

    if ( original_id == clone_id )
    {
        ErrorMgr.AddError( VSP_INVALID_INPUT_VAL, "SetGeomCloneOriginal::A Clone cannot be a Clone of itself" );
        return;
    }

    if ( clone_ptr->IsCloneAncestor( original_id ) )
    {
        ErrorMgr.AddError( VSP_INVALID_INPUT_VAL, "SetGeomCloneOriginal::Geom " + original_id +
                           " already copies Clone " + clone_id + ", so this would close a ring of Clones" );
        return;
    }

    if ( clone_ptr->IsDescendant( original_id ) )
    {
        ErrorMgr.AddError( VSP_INVALID_INPUT_VAL, "SetGeomCloneOriginal::Geom " + original_id +
                           " hangs off Clone " + clone_id + ", so it updates after it and would always be a pass behind" );
        return;
    }

    if ( !clone_ptr->SetOriginalID( original_id ) )
    {
        ErrorMgr.AddError( VSP_INVALID_ID, "SetGeomCloneOriginal::Clone " + clone_id +
                           " cannot copy Geom " + original_id );
        return;
    }

    ErrorMgr.NoError();
}

std::string GetGeomCloneOriginal( const std::string & clone_id )
{
    Vehicle* veh = GetVehicle();
    Geom* geom_ptr = veh->FindGeom( clone_id );
    if ( !geom_ptr )
    {
        ErrorMgr.AddError( VSP_INVALID_GEOM_ID, "GetGeomCloneOriginal::Can't Find Geom " + clone_id );
        return std::string();
    }

    CloneGeom* clone_ptr = dynamic_cast< CloneGeom* >( geom_ptr );
    if ( !clone_ptr )
    {
        ErrorMgr.AddError( VSP_WRONG_GEOM_TYPE, "GetGeomCloneOriginal::Geom " + clone_id + " is not a Clone" );
        return std::string();
    }

    string origid = clone_ptr->GetOriginalID();
    if ( origid == "NONE" )
    {
        origid = std::string();
    }

    ErrorMgr.NoError();
    return origid;
}

void SetGeomCloneNameSuffix( const std::string & clone_id, const std::string & name_suffix )
{
    Vehicle* veh = GetVehicle();
    Geom* geom_ptr = veh->FindGeom( clone_id );
    if ( !geom_ptr )
    {
        ErrorMgr.AddError( VSP_INVALID_GEOM_ID, "SetGeomCloneNameSuffix::Can't Find Geom " + clone_id );
        return;
    }

    CloneGeom* clone_ptr = dynamic_cast< CloneGeom* >( geom_ptr );
    if ( !clone_ptr )
    {
        ErrorMgr.AddError( VSP_WRONG_GEOM_TYPE, "SetGeomCloneNameSuffix::Geom " + clone_id + " is not a Clone" );
        return;
    }

    clone_ptr->SetNameSuffix( name_suffix );
    clone_ptr->Update();

    ErrorMgr.NoError();
}

std::string GetGeomCloneNameSuffix( const std::string & clone_id )
{
    Vehicle* veh = GetVehicle();
    Geom* geom_ptr = veh->FindGeom( clone_id );
    if ( !geom_ptr )
    {
        ErrorMgr.AddError( VSP_INVALID_GEOM_ID, "GetGeomCloneNameSuffix::Can't Find Geom " + clone_id );
        return std::string();
    }

    CloneGeom* clone_ptr = dynamic_cast< CloneGeom* >( geom_ptr );
    if ( !clone_ptr )
    {
        ErrorMgr.AddError( VSP_WRONG_GEOM_TYPE, "GetGeomCloneNameSuffix::Geom " + clone_id + " is not a Clone" );
        return std::string();
    }

    ErrorMgr.NoError();
    return clone_ptr->GetNameSuffix();
}

std::string ReplaceCloneGeom( const std::string & clone_id )
{
    Vehicle* veh = GetVehicle();
    Geom* geom_ptr = veh->FindGeom( clone_id );
    if ( !geom_ptr )
    {
        ErrorMgr.AddError( VSP_INVALID_GEOM_ID, "ReplaceCloneGeom::Can't Find Geom " + clone_id );
        return std::string();
    }

    CloneGeom* clone_ptr = dynamic_cast< CloneGeom* >( geom_ptr );
    if ( !clone_ptr )
    {
        ErrorMgr.AddError( VSP_WRONG_GEOM_TYPE, "ReplaceCloneGeom::Geom " + clone_id + " is not a Clone" );
        return std::string();
    }

    if ( !clone_ptr->GetOriginalGeom() )
    {
        ErrorMgr.AddError( VSP_INVALID_ID, "ReplaceCloneGeom::Clone " + clone_id + " has no original to copy" );
        return std::string();
    }

    int n_errors_before = ErrorMgr.GetNumTotalErrors();

    std::string new_id = veh->ReplaceCloneGeom( clone_id );
    if ( new_id.empty() )
    {
        ErrorMgr.AddError( VSP_INVALID_ID, "ReplaceCloneGeom::Could not replace Clone " + clone_id );
        return std::string();
    }

    // Leave the call flagged if anything was reported, e.g. a replacement that moved.
    if ( ErrorMgr.GetNumTotalErrors() == n_errors_before )
    {
        ErrorMgr.NoError();
    }
    return new_id;
}

std::vector< std::string > CloneGeomVec( const std::vector< std::string > & geom_id_vec, const std::string & name_suffix )
{
    Vehicle* veh = GetVehicle();

    for ( int i = 0 ; i < ( int )geom_id_vec.size() ; i++ )
    {
        if ( !veh->FindGeom( geom_id_vec[i] ) )
        {
            ErrorMgr.AddError( VSP_INVALID_GEOM_ID, "CloneGeomVec::Can't Find Geom " + geom_id_vec[i] );
            return std::vector< std::string >();
        }
    }

    // Vehicle::CloneGeomVec selects the new Clones; keep the script's selection.
    std::vector< std::string > active_store = veh->GetActiveGeomVec();

    std::vector< std::string > clone_vec = veh->CloneGeomVec( geom_id_vec, name_suffix );

    veh->SetActiveGeomVec( active_store );

    ErrorMgr.NoError();
    return clone_vec;
}

// Get the VSP Surface type for the specified Geom (i.e DISK_SURF)
int GetGeomVSPSurfType( const std::string& geom_id, int main_surf_ind )
{
    Vehicle* veh = GetVehicle();
    Geom* geom_ptr = veh->FindGeom( geom_id );
    if ( !geom_ptr )
    {
        ErrorMgr.AddError( VSP_INVALID_GEOM_ID, "GetGeomVSPSurfType::Can't Find Geom " + geom_id );
        return -1;
    }

    int nms = geom_ptr->GetNumMainSurfs();

    if ( main_surf_ind < 0 || main_surf_ind >= nms )
    {
        ErrorMgr.AddError( VSP_INDEX_OUT_RANGE, "GetGeomVSPSurfType::Main Surf Index " + to_string( main_surf_ind ) + " Out of Range" );
        return -1;
    }

    ErrorMgr.NoError();
    return geom_ptr->GetMainSurfType(main_surf_ind);
}

// Get the VSP Surface CFD type for the specified Geom (i.e TRANSPARENT_SURF)
int GetGeomVSPSurfCfdType( const std::string& geom_id, int main_surf_ind )
{
    Vehicle* veh = GetVehicle();
    Geom* geom_ptr = veh->FindGeom( geom_id );
    if ( !geom_ptr )
    {
        ErrorMgr.AddError( VSP_INVALID_GEOM_ID, "GetGeomVSPSurfCfdType::Can't Find Geom " + geom_id );
        return -1;
    }

    if ( main_surf_ind < 0 || main_surf_ind >= geom_ptr->GetNumMainSurfs() )
    {
        ErrorMgr.AddError( VSP_INDEX_OUT_RANGE, "GetGeomVSPSurfCfdType::Main Surf Index " + to_string( main_surf_ind ) + " Out of Range" );
        return -1;
    }

    ErrorMgr.NoError();
    return geom_ptr->GetMainCFDSurfType( main_surf_ind );
}

/// Get of the linkable parms ids for this geometry
std::vector< std::string > GetGeomParmIDs( const std::string & geom_id )
{
    std::vector< std::string > parm_vec;

    Vehicle* veh = GetVehicle();
    Geom* geom_ptr = veh->FindGeom( geom_id );
    if ( !geom_ptr )
    {
        ErrorMgr.AddError( VSP_INVALID_GEOM_ID, "GetGeomParmIDs::Can't Find Geom " + geom_id );
        return parm_vec;
    }

    geom_ptr->AddLinkableParms( parm_vec );

    ErrorMgr.NoError();
    return parm_vec;
}

/// Get the type of for this geometry
std::string GetGeomTypeName( const std::string & geom_id )
{
    Vehicle* veh = GetVehicle();
    Geom* geom_ptr = veh->FindGeom( geom_id );
    if ( !geom_ptr )
    {
        ErrorMgr.AddError( VSP_INVALID_GEOM_ID, "GetGeomTypeName::Can't Find Geom " + geom_id );
        return std::string();
    }

    std::string typ = std::string( geom_ptr->GetType().m_Name );
    ErrorMgr.NoError();
    return typ;
}

static std::string FindParmImpl( const std::string & parm_container_id, const std::string& parm_name, const std::string& group_name, const std::string & caller );

/// Get the parm id given container id, parm name, and group name
std::string GetParm( const std::string & container_id, const std::string & name, const std::string & group )
{
    return FindParmImpl( container_id, name, group, "GetParm" );
}

// Set the parent of a Geom; new_parent_id can be either the Vehicle's ID or another Geom's ID
//===================================================================//
//==================       Texture Functions      ===================//
//===================================================================//

std::string AttachGeomTexture( const std::string & geom_id, const std::string & file_name )
{
    Vehicle* veh = GetVehicle();
    Geom* geom_ptr = veh->FindGeom( geom_id );
    if ( !geom_ptr )
    {
        ErrorMgr.AddError( VSP_INVALID_GEOM_ID, "AttachGeomTexture::Can't Find Geom " + geom_id );
        return std::string();
    }

    std::string ret_id = geom_ptr->m_GuiDraw.getTextureMgr()->AttachTexture( file_name );

    if ( ret_id.empty() )
    {
        ErrorMgr.AddError( VSP_INVALID_ID, "AttachGeomTexture::No Texture Was Attached" );
        return ret_id;
    }

    ErrorMgr.NoError();
    return ret_id;
}

void RemoveGeomTexture( const std::string & geom_id, const std::string & texture_id )
{
    Vehicle* veh = GetVehicle();
    Geom* geom_ptr = veh->FindGeom( geom_id );
    if ( !geom_ptr )
    {
        ErrorMgr.AddError( VSP_INVALID_GEOM_ID, "RemoveGeomTexture::Can't Find Geom " + geom_id );
        return;
    }

    TextureMgr* tex_mgr = geom_ptr->m_GuiDraw.getTextureMgr();

    if ( !tex_mgr->FindTexture( texture_id ) )
    {
        ErrorMgr.AddError( VSP_INVALID_ID, "RemoveGeomTexture::Can't Find Texture " + texture_id );
        return;
    }

    tex_mgr->RemoveTexture( texture_id );

    ErrorMgr.NoError();
}

std::vector < std::string > GetGeomTextureIDVec( const std::string & geom_id )
{
    std::vector < std::string > ret;

    Vehicle* veh = GetVehicle();
    Geom* geom_ptr = veh->FindGeom( geom_id );
    if ( !geom_ptr )
    {
        ErrorMgr.AddError( VSP_INVALID_GEOM_ID, "GetGeomTextureIDVec::Can't Find Geom " + geom_id );
        return ret;
    }

    ret = geom_ptr->m_GuiDraw.getTextureMgr()->GetTextureVec();

    ErrorMgr.NoError();
    return ret;
}

std::string GetGeomTextureFileName( const std::string & geom_id, const std::string & texture_id )
{
    Vehicle* veh = GetVehicle();
    Geom* geom_ptr = veh->FindGeom( geom_id );
    if ( !geom_ptr )
    {
        ErrorMgr.AddError( VSP_INVALID_GEOM_ID, "GetGeomTextureFileName::Can't Find Geom " + geom_id );
        return std::string();
    }

    Texture* tex = geom_ptr->m_GuiDraw.getTextureMgr()->FindTexture( texture_id );

    if ( !tex )
    {
        ErrorMgr.AddError( VSP_INVALID_ID, "GetGeomTextureFileName::Can't Find Texture " + texture_id );
        return std::string();
    }

    ErrorMgr.NoError();
    return tex->m_FileName;
}

void ReorderGeom( const std::string & geom_id, int reorder_type )
{
    Vehicle* veh = GetVehicle();
    if ( !veh->FindGeom( geom_id ) )
    {
        ErrorMgr.AddError( VSP_INVALID_GEOM_ID, "ReorderGeom::Can't Find Geom " + geom_id );
        return;
    }

    if ( reorder_type < REORDER_MOVE_UP || reorder_type > REORDER_MOVE_BOTTOM )
    {
        ErrorMgr.AddError( VSP_INVALID_TYPE, "ReorderGeom::Invalid Reorder Type " + to_string( reorder_type ) );
        return;
    }

    veh->ReorderGeom( geom_id, reorder_type );
    veh->Update();

    ErrorMgr.NoError();
}

void SetGeomParent( const std::string& geom_id, const std::string& new_parent_id )
{
    Vehicle* veh = GetVehicle();
    Geom* geom_ptr = veh->FindGeom( geom_id );
    if ( !geom_ptr )
    {
        ErrorMgr.AddError( VSP_INVALID_GEOM_ID, "SetGeomParent::Can't Find Geom " + geom_id );
        return;
    }

    Geom* parent_ptr = veh->FindGeom( new_parent_id );
    if ( !parent_ptr && !TopLevelParentID( new_parent_id ) )
    {
        ErrorMgr.AddError( VSP_INVALID_GEOM_ID, "SetGeomParent::Can't Find Parent " + new_parent_id );
        return;
    }

    if ( parent_ptr && parent_ptr->DependsOn( geom_id ) )
    {
        ErrorMgr.AddError( VSP_INVALID_INPUT_VAL, "SetGeomParent::Geom " + geom_id + " can't be a child of " + new_parent_id + ", which depends on it" );
        return;
    }

    if ( parent_ptr )
    {
        geom_ptr->ChangeParentID( new_parent_id );
    }
    else
    {
        geom_ptr->ChangeParentID( veh->GetID() );
    }
    ErrorMgr.NoError();
}

// Get the parent of a Geom
std::string GetGeomParent( const std::string& geom_id )
{
    Vehicle* veh = GetVehicle();
    Geom* geom_ptr = veh->FindGeom( geom_id );
    if ( !geom_ptr )
    {
        ErrorMgr.AddError( VSP_INVALID_GEOM_ID, "GetGeomParent::Can't Find Geom " + geom_id );
        return std::string();
    }

    ErrorMgr.NoError();
    return geom_ptr->GetParentID();
}

//==== An ancestor's ID, or an empty string past the top of the tree ====//
std::string AncestorIDOrNone( Geom* geom_ptr, int gen )
{
    std::string id = geom_ptr->GetAncestorID( gen );
    if ( id == "NONE" )
    {
        return std::string();
    }
    return id;
}

std::string GetGeomSymAncestorID( const std::string & geom_id )
{
    Geom* geom_ptr = GetVehicle()->FindGeom( geom_id );
    if ( !geom_ptr )
    {
        ErrorMgr.AddError( VSP_INVALID_GEOM_ID, "GetGeomSymAncestorID::Can't Find Geom " + geom_id );
        return std::string();
    }

    ErrorMgr.NoError();
    return AncestorIDOrNone( geom_ptr, geom_ptr->m_SymAncestor() - 1 );
}

std::string GetGeomIncorporatedAncestorID( const std::string & geom_id )
{
    Geom* geom_ptr = GetVehicle()->FindGeom( geom_id );
    if ( !geom_ptr )
    {
        ErrorMgr.AddError( VSP_INVALID_GEOM_ID, "GetGeomIncorporatedAncestorID::Can't Find Geom " + geom_id );
        return std::string();
    }

    ErrorMgr.NoError();
    return AncestorIDOrNone( geom_ptr, geom_ptr->m_GroupedAncestorGen() );
}

//==== The generation of a Geom's ancestor by ID, reporting through the named caller
//==== when it is not one ====//
int FindAncestorGen( Geom* geom_ptr, const std::string & ancestor_id, const std::string & caller )
{
    for ( int gen = 0; ; gen++ )
    {
        std::string id = geom_ptr->GetAncestorID( gen );
        if ( id.empty() || id == "NONE" )
        {
            break;
        }
        if ( id == ancestor_id )
        {
            return gen;
        }
    }

    ErrorMgr.AddError( VSP_INVALID_INPUT_VAL, caller + "::Geom " + ancestor_id + " is not an ancestor of " + geom_ptr->GetID() );
    return -1;
}

void SetGeomSymAncestorID( const std::string & geom_id, const std::string & ancestor_id )
{
    Geom* geom_ptr = GetVehicle()->FindGeom( geom_id );
    if ( !geom_ptr )
    {
        ErrorMgr.AddError( VSP_INVALID_GEOM_ID, "SetGeomSymAncestorID::Can't Find Geom " + geom_id );
        return;
    }

    // Generation 0 is the global origin, 1 the Geom itself
    if ( ancestor_id.empty() )
    {
        geom_ptr->m_SymAncestor = 0;
        ErrorMgr.NoError();
        return;
    }

    int gen = FindAncestorGen( geom_ptr, ancestor_id, "SetGeomSymAncestorID" );
    if ( gen < 0 )
    {
        return;
    }

    geom_ptr->m_SymAncestor = gen + 1;
    ErrorMgr.NoError();
}

void SetGeomIncorporatedAncestorID( const std::string & geom_id, const std::string & ancestor_id )
{
    Geom* geom_ptr = GetVehicle()->FindGeom( geom_id );
    if ( !geom_ptr )
    {
        ErrorMgr.AddError( VSP_INVALID_GEOM_ID, "SetGeomIncorporatedAncestorID::Can't Find Geom " + geom_id );
        return;
    }

    int gen = FindAncestorGen( geom_ptr, ancestor_id, "SetGeomIncorporatedAncestorID" );
    if ( gen < 0 )
    {
        return;
    }

    geom_ptr->m_GroupedAncestorGen = gen;
    ErrorMgr.NoError();
}

// Get all children of a Geom
std::vector< std::string > GetGeomChildren( const std::string& geom_id )
{
    Vehicle* veh = GetVehicle();
    Geom* geom_ptr = veh->FindGeom( geom_id );
    if ( !geom_ptr )
    {
        ErrorMgr.AddError( VSP_INVALID_GEOM_ID, "GetGeomChildren::Can't Find Geom " + geom_id );
        return std::vector < std::string > {};
    }

    ErrorMgr.NoError();
    return geom_ptr->GetChildIDVec();
}

/// Get the number of xsec surfs used in the construction of this geom
int GetNumXSecSurfs( const std::string & geom_id )
{
    Vehicle* veh = GetVehicle();
    Geom* geom_ptr = veh->FindGeom( geom_id );
    if ( !geom_ptr )
    {
        ErrorMgr.AddError( VSP_INVALID_GEOM_ID, "GetNumXSecSurfs::Can't Find Geom " + geom_id );
        return 0;
    }

    ErrorMgr.NoError();
    return geom_ptr->GetNumXSecSurfs();
}

/// Get the number of main surfs for this geom
int GetNumMainSurfs( const std::string & geom_id )
{
    Vehicle* veh = GetVehicle();
    Geom* geom_ptr = veh->FindGeom( geom_id );
    if ( !geom_ptr )
    {
        ErrorMgr.AddError( VSP_INVALID_GEOM_ID, "GetNumMainSurfs::Can't Find Geom " + geom_id );
        return 0;
    }

    ErrorMgr.NoError();
    return geom_ptr->GetNumMainSurfs();
}

/// Get the total number of surfs for this geom
int GetTotalNumSurfs( const std::string& geom_id )
{
    Vehicle* veh = GetVehicle();
    Geom* geom_ptr = veh->FindGeom( geom_id );
    if ( !geom_ptr )
    {
        ErrorMgr.AddError( VSP_INVALID_GEOM_ID, "GetTotalNumSurfs::Can't Find Geom " + geom_id );
        return 0;
    }

    ErrorMgr.NoError();
    return geom_ptr->GetNumTotalSurfs();
}

// Get the maximum bounding box point for the Geom in absolute or body axes
vec3d GetGeomBBoxMax( const std::string& geom_id, int main_surf_ind, bool ref_frame_is_absolute )
{
    Vehicle* veh = GetVehicle();
    Geom* geom_ptr = veh->FindGeom( geom_id );
    if ( !geom_ptr )
    {
        ErrorMgr.AddError( VSP_INVALID_GEOM_ID, "GetGeomBBoxMax::Can't Find Geom " + geom_id );
        return vec3d();
    }

    // Determine BndBox dimensions prior to rotating and translating
    Matrix4d model_matrix = geom_ptr->getModelMatrix();
    model_matrix.affineInverse();

    BndBox bbox;

    // A Geom with no surfaces -- a mesh, a point cloud, a wireframe -- has none to ask for a
    // bounding box, but it keeps one of its own.  That box is in absolute coordinates, so a
    // box in body axes is the most a box can give: the absolute one turned back through the
    // model matrix, which holds the geometry but is not as tight as what the surface path
    // returns.  There is no surface to index either, so main_surf_ind does not apply.
    if ( geom_ptr->isNonSurfaceType() )
    {
        bbox = geom_ptr->GetBndBox();

        if ( !ref_frame_is_absolute )
        {
            bbox.Transform( model_matrix );
        }

        ErrorMgr.NoError();
        return bbox.GetMax();
    }

    int num_surf = geom_ptr->GetNumTotalSurfs();

    if ( main_surf_ind < 0 || main_surf_ind >= num_surf )
    {
        ErrorMgr.AddError( VSP_INDEX_OUT_RANGE, "GetGeomBBoxMax::Surf Index " + to_string( main_surf_ind ) + " Out of Range" );
        return vec3d();
    }

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

    ErrorMgr.NoError();
    return bbox.GetMax();
}

// Get the minimum bounding box point for the Geom in absolute or body axes
vec3d GetGeomBBoxMin( const std::string& geom_id, int main_surf_ind, bool ref_frame_is_absolute )
{
    Vehicle* veh = GetVehicle();
    Geom* geom_ptr = veh->FindGeom( geom_id );
    if ( !geom_ptr )
    {
        ErrorMgr.AddError( VSP_INVALID_GEOM_ID, "GetGeomBBoxMin::Can't Find Geom " + geom_id );
        return vec3d();
    }

    // Determine BndBox dimensions prior to rotating and translating
    Matrix4d model_matrix = geom_ptr->getModelMatrix();
    model_matrix.affineInverse();

    BndBox bbox;

    // See GetGeomBBoxMax:  a Geom with no surfaces answers out of its own bounding box.
    if ( geom_ptr->isNonSurfaceType() )
    {
        bbox = geom_ptr->GetBndBox();

        if ( !ref_frame_is_absolute )
        {
            bbox.Transform( model_matrix );
        }

        ErrorMgr.NoError();
        return bbox.GetMin();
    }

    int num_surf = geom_ptr->GetNumTotalSurfs();

    if ( main_surf_ind < 0 || main_surf_ind >= num_surf )
    {
        ErrorMgr.AddError( VSP_INDEX_OUT_RANGE, "GetGeomBBoxMin::Surf Index " + to_string( main_surf_ind ) + " Out of Range" );
        return vec3d();
    }

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

    ErrorMgr.NoError();
    return bbox.GetMin();
}

/// Add a sub surface, return subsurface id
std::string AddSubSurf( const std::string & geom_id, int type, int surfindex )
{
    Vehicle* veh = GetVehicle();
    Geom* geom_ptr = veh->FindGeom( geom_id );
    if ( !geom_ptr )
    {
        ErrorMgr.AddError( VSP_INVALID_GEOM_ID, "AddSubSurf::Can't Find Geom " + geom_id );
        return std::string();
    }

    if ( type == SS_CONTROL && !SSControlSurf::IsValidParent( geom_ptr ) )
    {
        ErrorMgr.AddError( VSP_INVALID_TYPE, "AddSubSurf::Control surfaces need a Wing or Prop, not " + geom_ptr->GetType().m_Name );
        return std::string();
    }

    if ( surfindex < -1 || surfindex >= geom_ptr->GetNumMainSurfs() )
    {
        ErrorMgr.AddError( VSP_INDEX_OUT_RANGE, "AddSubSurf::Main surface index " + to_string( surfindex ) + " out of range" );
        return std::string();
    }

    SubSurface* ssurf = nullptr;
    ssurf = geom_ptr->AddSubSurf( type, surfindex );
    if ( !ssurf )
    {
        ErrorMgr.AddError( VSP_INVALID_PTR, "AddSubSurf::Invalid Sub Surface Ptr " );
        return std::string();
    }
    ssurf->Update();
    ErrorMgr.NoError();
    return ssurf->GetID();
}

/// Get ID for sub surface at index
std::string GetSubSurf( const std::string & geom_id, int index )
{
    Vehicle* veh = GetVehicle();
    Geom* geom_ptr = veh->FindGeom( geom_id );
    if ( !geom_ptr )
    {
        ErrorMgr.AddError( VSP_INVALID_GEOM_ID, "GetSubSurf::Can't Find Geom " + geom_id  );
        return std::string();
    }
    SubSurface* ssurf = nullptr;
    ssurf = geom_ptr->GetSubSurf( index );
    if ( !ssurf )
    {
        ErrorMgr.AddError( VSP_INDEX_OUT_RANGE, "GetSubSurf::Sub Surface Index " + to_string( index ) + " Out of Range" );
        return std::string();
    }
    ErrorMgr.NoError();
    return ssurf->GetID();
}

/// Get IDs for all sub surface with parent geom ID and name
std::vector<std::string> GetSubSurf( const std::string & geom_id, const std::string & name )
{
    std::vector<std::string> ID_vec;
    Vehicle* veh = GetVehicle();
    Geom* geom_ptr = veh->FindGeom( geom_id );
    if ( !geom_ptr )
    {
        ErrorMgr.AddError( VSP_INVALID_GEOM_ID, "GetSubSurf::Can't Find Geom " + geom_id );
        return ID_vec;
    }
    std::vector<SubSurface*> ss_vec = geom_ptr->GetSubSurfVec();
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

void ReorderSubSurf( const std::string & geom_id, const std::string & sub_id, int reorder_type )
{
    Vehicle* veh = GetVehicle();
    Geom* geom_ptr = veh->FindGeom( geom_id );
    if ( !geom_ptr )
    {
        ErrorMgr.AddError( VSP_INVALID_GEOM_ID, "ReorderSubSurf::Can't Find Geom " + geom_id );
        return;
    }

    if ( !geom_ptr->GetSubSurf( sub_id ) )
    {
        ErrorMgr.AddError( VSP_INVALID_ID, "ReorderSubSurf::Can't Find Sub Surface " + sub_id );
        return;
    }

    if ( reorder_type < REORDER_MOVE_UP || reorder_type > REORDER_MOVE_BOTTOM )
    {
        ErrorMgr.AddError( VSP_INVALID_TYPE, "ReorderSubSurf::Invalid Reorder Type " + to_string( reorder_type ) );
        return;
    }

    geom_ptr->ReorderSubSurf( sub_id, reorder_type );

    ErrorMgr.NoError();
}

void DeleteSubSurf( const std::string & geom_id, const std::string & sub_id )
{
    Vehicle* veh = GetVehicle();
    Geom* geom_ptr = veh->FindGeom( geom_id );
    if ( !geom_ptr )
    {
        ErrorMgr.AddError( VSP_INVALID_GEOM_ID, "DeleteSubSurf::Can't Find Geom " + geom_id );
        return;
    }
    int index = geom_ptr->GetSubSurfIndex( sub_id );
    if ( index == -1 )
    {
        ErrorMgr.AddError( VSP_INVALID_ID, "DeleteSubSurf::Can't Find SubSurf " + sub_id );
        return;
    }

    // A Clone rebuilds its copied subsurfaces on every update, so they cannot be deleted here.
    CloneGeom* clone_ptr = dynamic_cast< CloneGeom* >( geom_ptr );
    if ( clone_ptr && clone_ptr->IsCopiedSubSurf( sub_id ) )
    {
        ErrorMgr.AddError( VSP_WRONG_GEOM_TYPE, "DeleteSubSurf::SubSurf " + sub_id +
                           " is copied from Geom " + clone_ptr->GetOriginalID() +
                           " -- delete it there, or turn the Clone's subsurface copying off" );
        return;
    }

    geom_ptr->DelSubSurf( index );
    ErrorMgr.NoError();
    return;
}

void DeleteSubSurf( const std::string & sub_id )
{
    SubSurface* ss_ptr = SubSurfaceMgr.GetSubSurf( sub_id );
    if ( !ss_ptr )
    {
        ErrorMgr.AddError( VSP_INVALID_ID, "DeleteSubSurf::Can't Find SubSurf " + sub_id );
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
        ErrorMgr.AddError( VSP_INVALID_ID, "DeleteSubSurf::Can't Find SubSurf " + sub_id );
        return;
    }

    // A Clone rebuilds its copied subsurfaces on every update, so they cannot be deleted here.
    CloneGeom* clone_ptr = dynamic_cast< CloneGeom* >( geom_ptr );
    if ( clone_ptr && clone_ptr->IsCopiedSubSurf( sub_id ) )
    {
        ErrorMgr.AddError( VSP_WRONG_GEOM_TYPE, "DeleteSubSurf::SubSurf " + sub_id +
                           " is copied from Geom " + clone_ptr->GetOriginalID() +
                           " -- delete it there, or turn the Clone's subsurface copying off" );
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
        ErrorMgr.AddError( VSP_INVALID_GEOM_ID, "SetSubSurfName::Can't Find Geom " + geom_id );
        return;
    }
    SubSurface* ssurf = nullptr;
    ssurf = geom_ptr->GetSubSurf( sub_id );
    if ( !ssurf )
    {
        ErrorMgr.AddError( VSP_INVALID_ID, "SetSubSurfName::Can't Find Sub Surface " + sub_id );
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
        ErrorMgr.AddError( VSP_INVALID_ID, "SetSubSurfName::Can't Find Sub Surface " + sub_id );
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
        ErrorMgr.AddError( VSP_INVALID_GEOM_ID, "GetSubSurfName::Can't Find Geom " + geom_id );
        return std::string();
    }
    SubSurface* ssurf = nullptr;
    ssurf = geom_ptr->GetSubSurf( sub_id );
    if ( !ssurf )
    {
        ErrorMgr.AddError( VSP_INVALID_ID, "GetSubSurfName::Can't Find Sub Surface " + sub_id );
        return std::string();
    }
    ErrorMgr.NoError();
    return ssurf->GetName();
}

std::string GetSubSurfName( const std::string & sub_id )
{
    SubSurface* ssurf = SubSurfaceMgr.GetSubSurf( sub_id );
    if ( !ssurf )
    {
        ErrorMgr.AddError( VSP_INVALID_ID, "GetSubSurfName::Can't Find Sub Surface " + sub_id );
        return std::string();
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
        ErrorMgr.AddError( VSP_INVALID_ID, "GetSubSurfIndex::Can't Find Sub Surface " + sub_id );
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
        ErrorMgr.AddError( VSP_INVALID_GEOM_ID, "GetSubSurfIDVec::Can't Find Geom " + geom_id );
        return std::vector<std::string>();
    }

    std::vector<SubSurface*> ss_vec = geom_ptr->GetSubSurfVec();
    std::vector<std::string> ID_vec;
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
    std::vector<SubSurface*> ss_vec = SubSurfaceMgr.GetSubSurfs();
    std::vector<std::string> ID_vec;
    ID_vec.resize( ss_vec.size() );

    for ( size_t i = 0; i < ss_vec.size(); i++ )
    {
        ID_vec[i] = ss_vec[i]->GetID();
    }

    ErrorMgr.NoError();
    return ID_vec;
}

/// Get # of sub surfaces for a geom
int GetNumSubSurf( const std::string & geom_id )
{
    Vehicle* veh = GetVehicle();
    Geom* geom_ptr = veh->FindGeom( geom_id );
    if ( !geom_ptr )
    {
        ErrorMgr.AddError( VSP_INVALID_GEOM_ID, "GetNumSubSurf::Can't Find Geom " + geom_id );
        return -1;
    }
    ErrorMgr.NoError();
    return geom_ptr->NumSubSurfs();
}

/// Get sub surface type
int GetSubSurfType( const std::string & sub_id )
{
    SubSurface* ssurf = SubSurfaceMgr.GetSubSurf( sub_id );
    if ( !ssurf )
    {
        ErrorMgr.AddError( VSP_INVALID_ID, "GetSubSurfType::Can't Find Sub Surface " + sub_id );
        return -1;
    }
    ErrorMgr.NoError();
    return ssurf->GetType();
}

/// Get of the linkable parms ids for this sub surface
std::vector<std::string> GetSubSurfParmIDs( const std::string & sub_id )
{
    std::vector< std::string > parm_vec;

    Vehicle* veh = GetVehicle();
    SubSurface* ss_ptr = SubSurfaceMgr.GetSubSurf( sub_id );
    if ( !ss_ptr )
    {
        ErrorMgr.AddError( VSP_INVALID_ID, "GetSubSurfParmIDs::Can't Find SubSurface " + sub_id );
        return parm_vec;
    }

    ss_ptr->AddLinkableParms( parm_vec );

    ErrorMgr.NoError();
    return parm_vec;
}

void IntersectSubSurf( const std::string & sub_id )
{
    SubSurface* ss_ptr = SubSurfaceMgr.GetSubSurf( sub_id );
    if ( !ss_ptr )
    {
        ErrorMgr.AddError( VSP_INVALID_ID, "IntersectSubSurf::Can't Find SubSurface " + sub_id );
        return;
    }

    SSIntersect* ss_intersect_ptr = dynamic_cast< SSIntersect* >( ss_ptr );
    if ( !ss_intersect_ptr )
    {
        ErrorMgr.AddError( VSP_INVALID_TYPE, "IntersectSubSurf::SubSurface " + sub_id + " is not SS_INTERSECT type" );
        return;
    }

    ss_intersect_ptr->Intersect();
    ErrorMgr.NoError();
}

std::string GetIntersectSubSurfGeomID( const std::string & sub_id )
{
    SubSurface* ss_ptr = SubSurfaceMgr.GetSubSurf( sub_id );
    if ( !ss_ptr )
    {
        ErrorMgr.AddError( VSP_INVALID_ID, "GetIntersectSubSurfGeomID::Can't Find SubSurface " + sub_id );
        return std::string();
    }

    SSIntersect* ss_intersect_ptr = dynamic_cast< SSIntersect* >( ss_ptr );
    if ( !ss_intersect_ptr )
    {
        ErrorMgr.AddError( VSP_INVALID_TYPE, "GetIntersectSubSurfGeomID::SubSurface " + sub_id + " is not SS_INTERSECT type" );
        return std::string();
    }

    ErrorMgr.NoError();
    return ss_intersect_ptr->m_IntersectID;
}

void SetIntersectSubSurfGeomID( const std::string & sub_id, const std::string & geom_id )
{
    SubSurface* ss_ptr = SubSurfaceMgr.GetSubSurf( sub_id );
    if ( !ss_ptr )
    {
        ErrorMgr.AddError( VSP_INVALID_ID, "SetIntersectSubSurfGeomID::Can't Find SubSurface " + sub_id );
        return;
    }

    SSIntersect* ss_intersect_ptr = dynamic_cast< SSIntersect* >( ss_ptr );
    if ( !ss_intersect_ptr )
    {
        ErrorMgr.AddError( VSP_INVALID_TYPE, "SetIntersectSubSurfGeomID::SubSurface " + sub_id + " is not SS_INTERSECT type" );
        return;
    }

    Vehicle* veh = GetVehicle();
    Geom* geom_ptr = veh->FindGeom( geom_id );
    if ( !geom_ptr )
    {
        ErrorMgr.AddError( VSP_INVALID_GEOM_ID, "SetIntersectSubSurfGeomID::Can't Find Geom " + geom_id );
        return;
    }

    ss_intersect_ptr->m_IntersectID = geom_id;

    ErrorMgr.NoError();
}

//**********************************************************************//
//*****************     FEA Mesh API Functions     *********************//
//**********************************************************************//

//==== Look up the Structure a Geom ID and index name, reporting through the
//==== named caller when it cannot be found. ====//
FeaStructure* FindFeaStruct( const std::string & geom_id, int fea_struct_ind, const std::string & caller )
{
    Vehicle* veh = GetVehicle();
    if ( !veh )
    {
        return nullptr;
    }

    Geom* geom_ptr = veh->FindGeom( geom_id );
    if ( !geom_ptr )
    {
        ErrorMgr.AddError( VSP_INVALID_GEOM_ID, caller + "::Can't Find Geom " + geom_id );
        return nullptr;
    }

    FeaStructure* feastruct = geom_ptr->GetFeaStruct( fea_struct_ind );
    if ( !feastruct )
    {
        ErrorMgr.AddError( VSP_INDEX_OUT_RANGE, caller + "::FeaStructure Index " + to_string( fea_struct_ind ) + " Out of Range" );
        return nullptr;
    }

    return feastruct;
}

//==== Look up a Structure by ID, reporting through the named caller when it
//==== cannot be found. ====//
FeaStructure* FindFeaStruct( const std::string & struct_id, const std::string & caller )
{
    FeaStructure* feastruct = StructureMgr.GetFeaStruct( struct_id );
    if ( !feastruct )
    {
        ErrorMgr.AddError( VSP_INVALID_ID, caller + "::Can't Find FeaStructure " + struct_id );
    }
    return feastruct;
}

//==== The Structure that holds a FeaPart or FEA SubSurface ====//
FeaStructure* FindFeaPartStruct( const std::string & part_id )
{
    std::vector < FeaStructure* > struct_vec = StructureMgr.GetAllFeaStructs();
    for ( size_t i = 0; i < struct_vec.size(); i++ )
    {
        if ( struct_vec[i]->GetFeaPartIndex( part_id ) >= 0 )
        {
            return struct_vec[i];
        }

        std::vector < SubSurface* > ss_vec = struct_vec[i]->GetFeaSubSurfVec();
        for ( size_t j = 0; j < ss_vec.size(); j++ )
        {
            if ( ss_vec[j]->GetID() == part_id )
            {
                return struct_vec[i];
            }
        }
    }
    return nullptr;
}

//==== Whether the Structure a Geom ID and index name holds a FeaPart or FEA
//==== SubSurface, reporting through the named caller when it does not. ====//
bool FeaStructHolds( const std::string & geom_id, int fea_struct_ind, const std::string & part_id, const std::string & caller )
{
    FeaStructure* feastruct = FindFeaStruct( geom_id, fea_struct_ind, caller );
    if ( !feastruct )
    {
        return false;
    }

    if ( FindFeaPartStruct( part_id ) != feastruct )
    {
        ErrorMgr.AddError( VSP_INVALID_ID, caller + "::Can't Find " + part_id + " in FeaStructure " + feastruct->GetID() );
        return false;
    }
    return true;
}

/// Add an FeaStructure, return FeaStructure index
static int AddFeaStructImpl( const std::string & geom_id, bool init_skin, int surfindex, const std::string & caller )
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
        ErrorMgr.AddError( VSP_INVALID_GEOM_ID, caller + "::Can't Find Geom " + geom_id );
        return -1;
    }

    FeaStructure* feastruct = nullptr;
    feastruct = geom_ptr->AddFeaStruct( init_skin, surfindex );
    if ( !feastruct )
    {
        ErrorMgr.AddError( VSP_INVALID_PTR, caller + "::Invalid FeaStructure Ptr" );
        return -1;
    }
    ErrorMgr.NoError();
    return ( geom_ptr->NumGeomFeaStructs() - 1 );
}

int AddFeaStruct( const std::string & geom_id, bool init_skin, int surfindex )
{
    return AddFeaStructImpl( geom_id, init_skin, surfindex, "AddFeaStruct" );
}

/// Add an FeaStructure, return FeaStructure ID
std::string AddFeaStructure( const std::string & geom_id, bool init_skin, int surfindex )
{
    int index = AddFeaStructImpl( geom_id, init_skin, surfindex, "AddFeaStructure" );
    if ( index < 0 )
    {
        return std::string();
    }

    return GetFeaStructID( geom_id, index );
}

int GetFeaMeshStructIndex()
{
    FeaStructure* feastruct = StructureMgr.GetFeaStruct( FeaMeshMgr.GetFeaMeshStructID() );

    if ( !feastruct )
    {
        ErrorMgr.AddError( VSP_INVALID_ID, "GetFeaMeshStructIndex::No Structure Is Current" );
        return -1;
    }

    ErrorMgr.NoError();
    return StructureMgr.GetTotFeaStructIndex( feastruct );
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
        ErrorMgr.AddError( VSP_INDEX_OUT_RANGE, "SetFeaMeshStructIndex::FeaStructure Index " + to_string( struct_index ) + " Out of Range" );
    }
}

std::string GetFeaMeshStructID()
{
    FeaStructure* feastruct = StructureMgr.GetFeaStruct( FeaMeshMgr.GetFeaMeshStructID() );

    if ( !feastruct )
    {
        ErrorMgr.AddError( VSP_INVALID_ID, "GetFeaMeshStructID::No Structure Is Current" );
        return std::string();
    }

    ErrorMgr.NoError();
    return feastruct->GetID();
}

void SetFeaMeshStructID( const std::string & struct_id )
{
    FeaStructure* feastruct = FindFeaStruct( struct_id, "SetFeaMeshStructID" );
    if ( !feastruct )
    {
        return;
    }

    FeaMeshMgr.SetFeaMeshStructID( struct_id );
    ErrorMgr.NoError();
}

void DeleteFeaStruct( const std::string & geom_id, int fea_struct_ind )
{
    Vehicle* veh = GetVehicle();
    if ( !veh )
    {
        return;
    }

    Geom* geom_ptr = veh->FindGeom( geom_id );
    if ( !geom_ptr )
    {
        ErrorMgr.AddError( VSP_INVALID_GEOM_ID, "DeleteFeaStruct::Can't Find Geom " + geom_id );
        return;
    }
    if ( !geom_ptr->ValidGeomFeaStructInd( fea_struct_ind ) )
    {
        ErrorMgr.AddError( VSP_INDEX_OUT_RANGE, "DeleteFeaStruct::FeaStructure Index " + to_string( fea_struct_ind ) + " Out of Range" );
        return;
    }
    geom_ptr->DeleteFeaStruct( fea_struct_ind );
    ErrorMgr.NoError();
    return;
}

void DeleteFeaStructByID( const std::string & struct_id )
{
    FeaStructure* feastruct = FindFeaStruct( struct_id, "DeleteFeaStructByID" );
    if ( !feastruct )
    {
        return;
    }

    Geom* geom_ptr = GetVehicle()->FindGeom( feastruct->GetParentGeomID() );
    if ( !geom_ptr )
    {
        ErrorMgr.AddError( VSP_INVALID_PTR, "DeleteFeaStructByID::Can't Find Geom " + feastruct->GetParentGeomID() );
        return;
    }

    geom_ptr->DeleteFeaStruct( geom_ptr->GetFeaStructIndex( struct_id ) );
    ErrorMgr.NoError();
}

std::string GetFeaStructID( const std::string & geom_id, int fea_struct_ind )
{
    Vehicle* veh = GetVehicle();
    if ( !veh )
    {
        return std::string();
    }

    Geom* geom_ptr = veh->FindGeom( geom_id );
    if ( !geom_ptr )
    {
        ErrorMgr.AddError( VSP_INVALID_GEOM_ID, "GetFeaStructID::Can't Find Geom " + geom_id );
        return std::string();
    }

    FeaStructure* struct_ptr = geom_ptr->GetFeaStruct( fea_struct_ind );
    if ( !struct_ptr )
    {
        ErrorMgr.AddError( VSP_INDEX_OUT_RANGE, "GetFeaStructID::FeaStructure Index " + to_string( fea_struct_ind ) + " Out of Range" );
        return std::string();
    }
    ErrorMgr.NoError();
    return struct_ptr->GetID();
}

int GetFeaStructIndex( const std::string & struct_id )
{
    if ( !FindFeaStruct( struct_id, "GetFeaStructIndex" ) )
    {
        return -1;
    }

    ErrorMgr.NoError();
    return StructureMgr.GetGeomFeaStructIndex( struct_id );
}

std::string GetFeaStructParentGeomID( const std::string & struct_id )
{
    if ( !FindFeaStruct( struct_id, "GetFeaStructParentGeomID" ) )
    {
        return std::string();
    }

    ErrorMgr.NoError();
    return StructureMgr.GetFeaStructParentID( struct_id );
}

static std::string GetFeaStructNameImpl( FeaStructure* feastruct, const std::string & caller )
{
    ErrorMgr.NoError();
    return feastruct->GetName();
}

std::string GetFeaStructName( const std::string & geom_id, int fea_struct_ind )
{
    FeaStructure* feastruct = FindFeaStruct( geom_id, fea_struct_ind, "GetFeaStructName" );
    if ( !feastruct )
    {
        return std::string();
    }

    return GetFeaStructNameImpl( feastruct, "GetFeaStructName" );
}

std::string GetFeaStructNameByID( const std::string & struct_id )
{
    FeaStructure* feastruct = FindFeaStruct( struct_id, "GetFeaStructNameByID" );
    if ( !feastruct )
    {
        return std::string();
    }

    return GetFeaStructNameImpl( feastruct, "GetFeaStructNameByID" );
}

static void SetFeaStructNameImpl( FeaStructure* feastruct, const std::string & name, const std::string & caller )
{
    feastruct->SetName( name );
    ErrorMgr.NoError();
}

void SetFeaStructName( const std::string & geom_id, int fea_struct_ind, const std::string & name )
{
    FeaStructure* feastruct = FindFeaStruct( geom_id, fea_struct_ind, "SetFeaStructName" );
    if ( !feastruct )
    {
        return;
    }

    SetFeaStructNameImpl( feastruct, name, "SetFeaStructName" );
}

void SetFeaStructNameByID( const std::string & struct_id, const std::string & name )
{
    FeaStructure* feastruct = FindFeaStruct( struct_id, "SetFeaStructNameByID" );
    if ( !feastruct )
    {
        return;
    }

    SetFeaStructNameImpl( feastruct, name, "SetFeaStructNameByID" );
}

void SetFeaPartName( const std::string & part_id, const std::string & name )
{
    FeaPart* part = StructureMgr.GetFeaPart( part_id );
    if ( !part )
    {
        ErrorMgr.AddError( VSP_INVALID_ID, "SetFeaPartName::Can't Find FEA Part " + part_id + " to name " + name );
        return;
    }

    part->SetName( name );
    ErrorMgr.NoError();
    return;
}

std::vector< std::string > GetFeaStructIDVec()
{
    std::vector < std::string > ret_vec;
    Vehicle* veh = GetVehicle();
    if ( !veh )
    {
        return ret_vec;
    }

    std::vector < FeaStructure* > struct_vec = StructureMgr.GetAllFeaStructs();
    ret_vec.resize( struct_vec.size() );

    for ( size_t i = 0; i < struct_vec.size(); i++ )
    {
        ret_vec[i] = struct_vec[i]->GetID();
    }

    ErrorMgr.NoError();
    return ret_vec;
}

std::vector< std::string > GetFeaStructIDVecByGeom( const std::string & geom_id )
{
    std::vector < std::string > ret_vec;

    Geom* geom_ptr = GetVehicle()->FindGeom( geom_id );
    if ( !geom_ptr )
    {
        ErrorMgr.AddError( VSP_INVALID_GEOM_ID, "GetFeaStructIDVecByGeom::Can't Find Geom " + geom_id );
        return ret_vec;
    }

    std::vector < FeaStructure* > struct_vec = geom_ptr->GetFeaStructVec();
    ret_vec.resize( struct_vec.size() );

    for ( size_t i = 0; i < struct_vec.size(); i++ )
    {
        ret_vec[i] = struct_vec[i]->GetID();
    }

    ErrorMgr.NoError();
    return ret_vec;
}

/// Add a FeaPart to a Structure, return FeaPart ID
static std::string AddFeaPartImpl( FeaStructure* feastruct, int type, const std::string & caller )
{
    FeaPart* feapart = nullptr;
    feapart = feastruct->AddFeaPart( type );
    if ( !feapart )
    {
        ErrorMgr.AddError( VSP_INVALID_PTR, caller + "::Invalid FeaPart Ptr" );
        return std::string();
    }
    feastruct->Update();
    ErrorMgr.NoError();
    return feapart->GetID();
}

std::string AddFeaPart( const std::string & geom_id, int fea_struct_ind, int type )
{
    FeaStructure* feastruct = FindFeaStruct( geom_id, fea_struct_ind, "AddFeaPart" );
    if ( !feastruct )
    {
        return std::string();
    }

    return AddFeaPartImpl( feastruct, type, "AddFeaPart" );
}

/// Add a FeaPart, return FeaPart ID
std::string AddFeaPartByID( const std::string & struct_id, int type )
{
    FeaStructure* feastruct = FindFeaStruct( struct_id, "AddFeaPartByID" );
    if ( !feastruct )
    {
        return std::string();
    }

    return AddFeaPartImpl( feastruct, type, "AddFeaPartByID" );
}

static void IndividualizeFeaPartImpl( const std::string & part_id, const std::string & caller )
{
    FeaStructure* feastruct = FindFeaPartStruct( part_id );
    if ( !feastruct )
    {
        ErrorMgr.AddError( VSP_INVALID_ID, caller + "::Can't Find FeaPart " + part_id );
        return;
    }

    // Rib and slice arrays are FeaParts; SSLine arrays are FeaSubSurfaces.  The two
    // live in separate lists, so look in both.
    int index = feastruct->GetFeaPartIndex( part_id );
    if ( index >= 0 )
    {
        FeaPart* part = feastruct->GetFeaPart( index );

        if ( part && part->GetType() == FEA_RIB_ARRAY )
        {
            feastruct->IndividualizeRibArray( index );
            ErrorMgr.NoError();
            return;
        }

        if ( part && part->GetType() == FEA_SLICE_ARRAY )
        {
            feastruct->IndividualizeSliceArray( index );
            ErrorMgr.NoError();
            return;
        }

        ErrorMgr.AddError( VSP_INVALID_TYPE, caller + "::FEA Part " + part_id + " is not an array" );
        return;
    }

    std::vector < SubSurface* > ss_vec = feastruct->GetFeaSubSurfVec();
    for ( int i = 0; i < ( int )ss_vec.size(); i++ )
    {
        if ( ss_vec[i] && ss_vec[i]->GetID() == part_id )
        {
            if ( ss_vec[i]->GetType() != SS_LINE_ARRAY )
            {
                ErrorMgr.AddError( VSP_INVALID_TYPE, caller + "::FEA Part " + part_id + " is not an array" );
                return;
            }

            feastruct->IndividualizeSSLineArray( i );
            ErrorMgr.NoError();
            return;
        }
    }

    ErrorMgr.AddError( VSP_INVALID_ID, caller + "::Can't Find FeaPart " + part_id );
}

void IndividualizeFeaPart( const std::string & geom_id, int fea_struct_ind, const std::string & part_id )
{
    if ( FeaStructHolds( geom_id, fea_struct_ind, part_id, "IndividualizeFeaPart" ) )
    {
        IndividualizeFeaPartImpl( part_id, "IndividualizeFeaPart" );
    }
}

void IndividualizeFeaPartByID( const std::string & part_id )
{
    IndividualizeFeaPartImpl( part_id, "IndividualizeFeaPartByID" );
}

static void ReorderFeaPartImpl( const std::string & part_id, int reorder_type, const std::string & caller )
{
    if ( reorder_type < REORDER_MOVE_UP || reorder_type > REORDER_MOVE_BOTTOM )
    {
        ErrorMgr.AddError( VSP_INVALID_TYPE, caller + "::Invalid Reorder Type " + to_string( reorder_type ) );
        return;
    }

    FeaStructure* feastruct = FindFeaPartStruct( part_id );
    int index = -1;
    if ( feastruct )
    {
        index = feastruct->GetFeaPartIndex( part_id );
    }
    if ( index == -1 )
    {
        ErrorMgr.AddError( VSP_INVALID_ID, caller + "::Can't Find FeaPart " + part_id );
        return;
    }

    feastruct->ReorderFeaPart( index, reorder_type );

    ErrorMgr.NoError();
}

void ReorderFeaPart( const std::string & geom_id, int fea_struct_ind, const std::string & part_id, int reorder_type )
{
    if ( FeaStructHolds( geom_id, fea_struct_ind, part_id, "ReorderFeaPart" ) )
    {
        ReorderFeaPartImpl( part_id, reorder_type, "ReorderFeaPart" );
    }
}

void ReorderFeaPartByID( const std::string & part_id, int reorder_type )
{
    ReorderFeaPartImpl( part_id, reorder_type, "ReorderFeaPartByID" );
}

static void DeleteFeaPartImpl( const std::string & part_id, const std::string & caller )
{
    FeaStructure* feastruct = FindFeaPartStruct( part_id );
    int index = -1;
    if ( feastruct )
    {
        index = feastruct->GetFeaPartIndex( part_id );
    }
    if ( index == -1 )
    {
        ErrorMgr.AddError( VSP_INVALID_ID, caller + "::Can't Find FeaPart " + part_id );
        return;
    }
    feastruct->DelFeaPart( index );
    ErrorMgr.NoError();
}

void DeleteFeaPart( const std::string & geom_id, int fea_struct_ind, const std::string & part_id )
{
    if ( FeaStructHolds( geom_id, fea_struct_ind, part_id, "DeleteFeaPart" ) )
    {
        DeleteFeaPartImpl( part_id, "DeleteFeaPart" );
    }
}

void DeleteFeaPartByID( const std::string & part_id )
{
    DeleteFeaPartImpl( part_id, "DeleteFeaPartByID" );
}

std::string GetFeaPartID( const std::string & fea_struct_id, int fea_part_index )
{
    FeaStructure* feastruct = nullptr;
    feastruct = StructureMgr.GetFeaStruct( fea_struct_id );
    if ( !feastruct )
    {
        ErrorMgr.AddError( VSP_INVALID_ID, "GetFeaPartID::Can't Find FeaStructure " + fea_struct_id );
        return std::string();
    }

    FeaPart* part = feastruct->GetFeaPart( fea_part_index );

    if ( !part )
    {
        ErrorMgr.AddError( VSP_INDEX_OUT_RANGE, "GetFeaPartID::FeaPart Index " + to_string( fea_part_index ) + " Out of Range" );
        return std::string();
    }

    ErrorMgr.NoError();
    return part->GetID();
}

std::string GetFeaPartName( const std::string & part_id )
{
    FeaPart* part = StructureMgr.GetFeaPart( part_id );
    if ( !part )
    {
        ErrorMgr.AddError( VSP_INVALID_ID, "GetFeaPartName::Can't Find FEA Part " + part_id );
        return std::string();
    }

    ErrorMgr.NoError();
    return part->GetName();
}

int GetFeaPartType( const std::string & part_id )
{
    FeaPart* part = StructureMgr.GetFeaPart( part_id );
    if ( !part )
    {
        ErrorMgr.AddError( VSP_INVALID_ID, "GetFeaPartType::Can't Find FEA Part " + part_id );
        return - 1;
    }

    ErrorMgr.NoError();
    return part->GetType();
}

void SetFeaPartPerpendicularSparID( const std::string& part_id, const std::string& perpendicular_spar_id )
{
    FeaPart* part = StructureMgr.GetFeaPart( part_id );
    if ( !part )
    {
        ErrorMgr.AddError( VSP_INVALID_ID, "SetFeaPartPerpendicularSparID::Can't Find FEA Part " + part_id );
        return;
    }

    bool rib_type = part->GetType() == FEA_RIB;
    bool rib_array_type = part->GetType() == FEA_RIB_ARRAY;

    if ( !( rib_type || rib_array_type ) )
    {
        ErrorMgr.AddError( VSP_INVALID_TYPE, "SetFeaPartPerpendicularSparID::FEA Part " + part_id + " is not a Rib or Rib Array" );
        return;
    }

    // Check if Spar exists (not really necessary, but should be helpful)
    FeaPart* spar = StructureMgr.GetFeaPart( perpendicular_spar_id );
    if ( !spar || spar->GetType() != FEA_SPAR )
    {
        ErrorMgr.AddError( VSP_INVALID_ID, "SetFeaPartPerpendicularSparID::Can't Find FEA Spar " + perpendicular_spar_id + ". ID will still be set." );
    }
    else
    {
        ErrorMgr.NoError();
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

std::string GetFeaPartPerpendicularSparID( const std::string& part_id )
{
    FeaPart* part = StructureMgr.GetFeaPart( part_id );
    std::string ret_str = std::string();
    if ( !part )
    {
        ErrorMgr.AddError( VSP_INVALID_ID, "GetFeaPartPerpendicularSparID::Can't Find FEA Part " + part_id );
        return ret_str;
    }

    bool rib_type = part->GetType() == FEA_RIB;
    bool rib_array_type = part->GetType() == FEA_RIB_ARRAY;

    if ( !( rib_type || rib_array_type ) )
    {
        ErrorMgr.AddError( VSP_INVALID_TYPE, "GetFeaPartPerpendicularSparID::FEA Part " + part_id + " is not a Rib or Rib Array" );
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

    ErrorMgr.NoError();
    return ret_str;
}

//==== The shell or cap property ID a FeaPart or FEA SubSurface holds, and the legacy
//==== index Parm that overrides it while set ====//
bool FindFeaPartProperty( const std::string & part_id, bool cap, std::string* & id, IntParm* & legacy, const std::string & caller )
{
    FeaPart* part = StructureMgr.GetFeaPart( part_id );
    SubSurface* ssurf = StructureMgr.GetFeaSubSurf( part_id );

    if ( part && cap )
    {
        id = &part->m_CapFeaPropertyID;
        legacy = &part->m_CapFeaPropertyIndex;
    }
    else if ( part )
    {
        id = &part->m_FeaPropertyID;
        legacy = &part->m_FeaPropertyIndex;
    }
    else if ( ssurf && cap )
    {
        id = &ssurf->m_CapFeaPropertyID;
        legacy = &ssurf->m_CapFeaPropertyIndex;
    }
    else if ( ssurf )
    {
        id = &ssurf->m_FeaPropertyID;
        legacy = &ssurf->m_FeaPropertyIndex;
    }
    else
    {
        ErrorMgr.AddError( VSP_INVALID_ID, caller + "::Can't Find FEA Part or SubSurface " + part_id );
        return false;
    }
    return true;
}

//==== Set the shell or cap property of a FeaPart or FEA SubSurface ====//
void SetFeaPartPropertyIDOf( const std::string & part_id, const std::string & property_id, bool cap, const std::string & caller )
{
    FeaProperty* prop = StructureMgr.GetFeaProperty( property_id );
    if ( !prop )
    {
        ErrorMgr.AddError( VSP_INVALID_ID, caller + "::Can't Find FEA Property " + property_id );
        return;
    }

    int type = vsp::FEA_SHELL;
    std::string type_name = "shell";
    if ( cap )
    {
        type = vsp::FEA_BEAM;
        type_name = "beam";
    }
    if ( prop->m_FeaPropertyType() != type )
    {
        ErrorMgr.AddError( VSP_INVALID_TYPE, caller + "::FEA Property " + property_id + " is not a " + type_name + " property" );
        return;
    }

    std::string* id = nullptr;
    IntParm* legacy = nullptr;
    if ( !FindFeaPartProperty( part_id, cap, id, legacy, caller ) )
    {
        return;
    }

    *id = property_id;
    legacy->Set( -1 );
    ErrorMgr.NoError();
}

//==== The shell or cap property of a FeaPart or FEA SubSurface, or the one a legacy
//==== index names while it is set ====//
std::string GetFeaPartPropertyIDOf( const std::string & part_id, bool cap, const std::string & caller )
{
    std::string* id = nullptr;
    IntParm* legacy = nullptr;
    if ( !FindFeaPartProperty( part_id, cap, id, legacy, caller ) )
    {
        return std::string();
    }

    ErrorMgr.NoError();

    std::vector < FeaProperty* > prop_vec = StructureMgr.GetFeaPropertyVec();
    if ( legacy->Get() >= 0 && legacy->Get() < ( int )prop_vec.size() )
    {
        return prop_vec[ legacy->Get() ]->GetID();
    }
    return *id;
}

void SetFeaPartPropertyID( const std::string & part_id, const std::string & property_id )
{
    SetFeaPartPropertyIDOf( part_id, property_id, false, "SetFeaPartPropertyID" );
}

std::string GetFeaPartPropertyID( const std::string & part_id )
{
    return GetFeaPartPropertyIDOf( part_id, false, "GetFeaPartPropertyID" );
}

void SetFeaPartCapPropertyID( const std::string & part_id, const std::string & property_id )
{
    SetFeaPartPropertyIDOf( part_id, property_id, true, "SetFeaPartCapPropertyID" );
}

std::string GetFeaPartCapPropertyID( const std::string & part_id )
{
    return GetFeaPartPropertyIDOf( part_id, true, "GetFeaPartCapPropertyID" );
}

//==== Look up a FEA fixed point by ID, reporting through the named caller ====//
FeaFixPoint* FindFeaFixPoint( const std::string & fixpt_id, const std::string & caller )
{
    FeaFixPoint* fixpt = dynamic_cast < FeaFixPoint* > ( StructureMgr.GetFeaPart( fixpt_id ) );
    if ( !fixpt )
    {
        ErrorMgr.AddError( VSP_INVALID_ID, caller + "::Can't Find FEA Fixed Point " + fixpt_id );
    }
    return fixpt;
}

void SetFeaFixPointParentID( const std::string & fixpt_id, const std::string & part_id )
{
    FeaFixPoint* fixpt = FindFeaFixPoint( fixpt_id, "SetFeaFixPointParentID" );
    if ( !fixpt )
    {
        return;
    }

    FeaPart* parent = StructureMgr.GetFeaPart( part_id );
    if ( !parent || parent->GetType() == FEA_FIX_POINT || FindFeaPartStruct( part_id ) != FindFeaPartStruct( fixpt_id ) )
    {
        ErrorMgr.AddError( VSP_INVALID_ID, "SetFeaFixPointParentID::FEA Part " + part_id + " is not a surface in the fixed point's Structure" );
        return;
    }

    fixpt->m_ParentFeaPartID = part_id;
    fixpt->m_SurfDirty = true;
    ErrorMgr.NoError();
}

std::string GetFeaFixPointParentID( const std::string & fixpt_id )
{
    FeaFixPoint* fixpt = FindFeaFixPoint( fixpt_id, "GetFeaFixPointParentID" );
    if ( !fixpt )
    {
        return std::string();
    }

    ErrorMgr.NoError();
    return fixpt->m_ParentFeaPartID;
}

void SetFeaFixPointOtherGeomID( const std::string & fixpt_id, const std::string & geom_id )
{
    FeaFixPoint* fixpt = FindFeaFixPoint( fixpt_id, "SetFeaFixPointOtherGeomID" );
    if ( !fixpt )
    {
        return;
    }

    Geom* geom_ptr = GetVehicle()->FindGeom( geom_id );
    if ( !geom_id.empty() && !geom_ptr )
    {
        ErrorMgr.AddError( VSP_INVALID_GEOM_ID, "SetFeaFixPointOtherGeomID::Can't Find Geom " + geom_id );
        return;
    }

    // The Geoms the GUI does not offer have no surface to place a point on
    if ( geom_ptr )
    {
        int type = geom_ptr->GetType().m_Type;
        if ( type == MESH_GEOM_TYPE || type == HUMAN_GEOM_TYPE || type == PT_CLOUD_GEOM_TYPE ||
             type == WIRE_FRAME_GEOM_TYPE || type == BLANK_GEOM_TYPE )
        {
            ErrorMgr.AddError( VSP_WRONG_GEOM_TYPE, "SetFeaFixPointOtherGeomID::Geom " + geom_id + " has no surface for a fixed point" );
            return;
        }
    }

    fixpt->m_OtherGeomID = geom_id;
    fixpt->m_SurfDirty = true;
    ErrorMgr.NoError();
}

std::string GetFeaFixPointOtherGeomID( const std::string & fixpt_id )
{
    FeaFixPoint* fixpt = FindFeaFixPoint( fixpt_id, "GetFeaFixPointOtherGeomID" );
    if ( !fixpt )
    {
        return std::string();
    }

    ErrorMgr.NoError();
    return fixpt->m_OtherGeomID;
}

void SetFeaSubSurfName( const std::string & subsurf_id, const std::string & name )
{
    SubSurface* ssurf = StructureMgr.GetFeaSubSurf( subsurf_id );

    if ( !ssurf )
    {
        ErrorMgr.AddError( VSP_INVALID_ID, "SetFeaSubSurfName::Can't Find FEA Sub Surface " + subsurf_id + " to name " + name );
        return;
    }

    ssurf->SetName( name );
    ErrorMgr.NoError();
    return;
}

std::string GetFeaSubSurfName( const std::string & subsurf_id )
{
    SubSurface* ssurf = StructureMgr.GetFeaSubSurf( subsurf_id );
    if ( !ssurf )
    {
        ErrorMgr.AddError( VSP_INVALID_ID, "GetFeaSubSurfName::Can't Find FEA SubSurf " + subsurf_id );
        return std::string();
    }

    ErrorMgr.NoError();
    return ssurf->GetName();
}

/// Add a FeaSubSurface to a Structure, return FeaSubSurface ID
static std::string AddFeaSubSurfImpl( FeaStructure* feastruct, int type, const std::string & caller )
{
    // A control surface is laid out on a wing-like section.
    if ( type == SS_CONTROL )
    {
        Vehicle* veh = GetVehicle();
        Geom* geom_ptr = nullptr;
        if ( veh )
        {
            geom_ptr = veh->FindGeom( feastruct->GetParentGeomID() );
        }
        if ( !SSControlSurf::IsValidParent( geom_ptr ) )
        {
            string parent = feastruct->GetParentGeomID();
            if ( geom_ptr )
            {
                parent = geom_ptr->GetType().m_Name;
            }
            ErrorMgr.AddError( VSP_INVALID_TYPE, caller + "::Control surfaces need a Wing or Prop, not " + parent );
            return std::string();
        }
    }

    SubSurface* feasubsurf = nullptr;
    feasubsurf = feastruct->AddFeaSubSurf( type );
    if ( !feasubsurf )
    {
        ErrorMgr.AddError( VSP_INVALID_TYPE, caller + "::Invalid FeaSubSurface Type " + to_string( type ) );
        return std::string();
    }
    feastruct->Update();
    ErrorMgr.NoError();
    return feasubsurf->GetID();
}

std::string AddFeaSubSurf( const std::string & geom_id, int fea_struct_ind, int type )
{
    FeaStructure* feastruct = FindFeaStruct( geom_id, fea_struct_ind, "AddFeaSubSurf" );
    if ( !feastruct )
    {
        return std::string();
    }

    return AddFeaSubSurfImpl( feastruct, type, "AddFeaSubSurf" );
}

/// Add a FeaSubSurface, return FeaSubSurface ID
std::string AddFeaSubSurfByID( const std::string & struct_id, int type )
{
    FeaStructure* feastruct = FindFeaStruct( struct_id, "AddFeaSubSurfByID" );
    if ( !feastruct )
    {
        return std::string();
    }

    return AddFeaSubSurfImpl( feastruct, type, "AddFeaSubSurfByID" );
}

//==== The position of a subsurface among a Structure's own subsurfaces, or -1 ====//
int FindFeaSubSurfIndex( FeaStructure* feastruct, const std::string & ss_id )
{
    vector < SubSurface* > ss_vec = feastruct->GetFeaSubSurfVec();
    for ( int i = 0; i < ( int )ss_vec.size(); i++ )
    {
        if ( ss_vec[i]->GetID() == ss_id )
        {
            return i;
        }
    }
    return -1;
}

void DeleteFeaSubSurf( const std::string & geom_id, int fea_struct_ind, const std::string & ss_id )
{
    FeaStructure* feastruct = FindFeaStruct( geom_id, fea_struct_ind, "DeleteFeaSubSurf" );
    if ( !feastruct )
    {
        return;
    }

    int index = FindFeaSubSurfIndex( feastruct, ss_id );
    if ( index == -1 )
    {
        ErrorMgr.AddError( VSP_INVALID_ID, "DeleteFeaSubSurf::Can't Find FeaSubSurf " + ss_id + " in FeaStructure " + feastruct->GetID() );
        return;
    }
    feastruct->DelFeaSubSurf( index );
    ErrorMgr.NoError();
    return;
}

void DeleteFeaSubSurfByID( const std::string & ss_id )
{
    FeaStructure* feastruct = FindFeaPartStruct( ss_id );
    int index = -1;
    if ( feastruct )
    {
        index = FindFeaSubSurfIndex( feastruct, ss_id );
    }
    if ( index == -1 )
    {
        ErrorMgr.AddError( VSP_INVALID_ID, "DeleteFeaSubSurfByID::Can't Find FeaSubSurf " + ss_id );
        return;
    }
    feastruct->DelFeaSubSurf( index );
    ErrorMgr.NoError();
}

int GetFeaSubSurfIndex( const std::string & ss_id )
{
    int index = StructureMgr.GetFeaSubSurfIndex( ss_id );
    if ( index < 0 )
    {
        ErrorMgr.AddError( VSP_INVALID_ID, "GetFeaSubSurfIndex::Can't Find FeaSubSurf " + ss_id );
        return index;
    }
    ErrorMgr.NoError();
    return index;
}

int GetFeaPolySparNumPt( const std::string & pspar_id )
{
    FeaPart* part = StructureMgr.GetFeaPart( pspar_id );
    if ( !part )
    {
        ErrorMgr.AddError( VSP_INVALID_ID, "GetFeaPolySparNumPt::Can't Find FEA Part " + pspar_id );
        return -1;
    }

    FeaPolySpar* pspar = dynamic_cast< FeaPolySpar* >( part );
    if( !pspar )
    {
        ErrorMgr.AddError( VSP_INVALID_TYPE, "GetFeaPolySparNumPt::FEA Part " + pspar_id + " is not a poly spar" );
        return -1;
    }

    ErrorMgr.NoError();
    return pspar->GetNumPt();
}

std::string AddFeaPolySparPt( const std::string & pspar_id )
{
    FeaPart* part = StructureMgr.GetFeaPart( pspar_id );
    if ( !part )
    {
        ErrorMgr.AddError( VSP_INVALID_ID, "AddFeaPolySparPt::Can't Find FEA Part " + pspar_id );
        return std::string();
    }

    FeaPolySpar* pspar = dynamic_cast< FeaPolySpar* >( part );
    if( !pspar )
    {
        ErrorMgr.AddError( VSP_INVALID_TYPE, "AddFeaPolySparPt::FEA Part " + pspar_id + " is not a poly spar" );
        return std::string();
    }

    PolySparPoint *pt = pspar->AddPt();

    ErrorMgr.NoError();
    return pt->GetID();
}

//==== Look up a FEA Poly Spar by ID, reporting through the named caller ====//
FeaPolySpar* FindFeaPolySpar( const std::string & pspar_id, const std::string & caller )
{
    FeaPolySpar* pspar = dynamic_cast< FeaPolySpar* >( StructureMgr.GetFeaPart( pspar_id ) );
    if ( !pspar )
    {
        ErrorMgr.AddError( VSP_INVALID_ID, caller + "::Can't Find FEA Poly Spar " + pspar_id );
    }
    return pspar;
}

//==== The index of a Poly Spar point by ID, reporting through the named caller ====//
int FindFeaPolySparPtIndex( FeaPolySpar* pspar, const std::string & pt_id, const std::string & caller )
{
    for ( int i = 0; i < pspar->GetNumPt(); i++ )
    {
        if ( pspar->GetPt( i )->GetID() == pt_id )
        {
            return i;
        }
    }

    ErrorMgr.AddError( VSP_INVALID_ID, caller + "::Can't Find Point " + pt_id + " in FEA Poly Spar " + pspar->GetID() );
    return -1;
}

static std::string InsertFeaPolySparPtImpl( const std::string & pspar_id, int index, const std::string & caller )
{
    FeaPart* part = StructureMgr.GetFeaPart( pspar_id );
    if ( !part )
    {
        ErrorMgr.AddError( VSP_INVALID_ID, caller + "::Can't Find FEA Part " + pspar_id );
        return std::string();
    }

    FeaPolySpar* pspar = dynamic_cast< FeaPolySpar* >( part );
    if( !pspar )
    {
        ErrorMgr.AddError( VSP_INVALID_TYPE, caller + "::FEA Part " + pspar_id + " is not a poly spar" );
        return std::string();
    }

    if ( !pspar->ValidPtIndex( index ) )
    {
        ErrorMgr.AddError( VSP_INDEX_OUT_RANGE, caller + "::Point index out of range " + to_string( index )  );
        return std::string();
    }

    PolySparPoint *pt = pspar->InsertPt( index );

    ErrorMgr.NoError();
    return pt->GetID();
}

std::string InsertFeaPolySparPt( const std::string & pspar_id, int index )
{
    return InsertFeaPolySparPtImpl( pspar_id, index, "InsertFeaPolySparPt" );
}

std::string InsertFeaPolySparPtByID( const std::string & pspar_id, const std::string & before_pt_id )
{
    FeaPolySpar* pspar = FindFeaPolySpar( pspar_id, "InsertFeaPolySparPtByID" );
    if ( !pspar )
    {
        return std::string();
    }

    int index = FindFeaPolySparPtIndex( pspar, before_pt_id, "InsertFeaPolySparPtByID" );
    if ( index < 0 )
    {
        return std::string();
    }

    return InsertFeaPolySparPtImpl( pspar_id, index, "InsertFeaPolySparPtByID" );
}

static void DelFeaPolySparPtImpl( const std::string & pspar_id, int index, const std::string & caller )
{
    FeaPart* part = StructureMgr.GetFeaPart( pspar_id );
    if ( !part )
    {
        ErrorMgr.AddError( VSP_INVALID_ID, caller + "::Can't Find FEA Part " + pspar_id );
        return;
    }

    FeaPolySpar* pspar = dynamic_cast< FeaPolySpar* >( part );
    if( !pspar )
    {
        ErrorMgr.AddError( VSP_INVALID_TYPE, caller + "::FEA Part " + pspar_id + " is not a poly spar" );
        return;
    }

    if ( !pspar->ValidPtIndex( index ) )
    {
        ErrorMgr.AddError( VSP_INDEX_OUT_RANGE, caller + "::Point index out of range " + to_string( index )  );
        return;
    }

    pspar->DelPt( index );

    ErrorMgr.NoError();
    return;
}

void DelFeaPolySparPt( const std::string & pspar_id, int index )
{
    DelFeaPolySparPtImpl( pspar_id, index, "DelFeaPolySparPt" );
}

void DelFeaPolySparPtByID( const std::string & pspar_id, const std::string & pt_id )
{
    FeaPolySpar* pspar = FindFeaPolySpar( pspar_id, "DelFeaPolySparPtByID" );
    if ( !pspar )
    {
        return;
    }

    int index = FindFeaPolySparPtIndex( pspar, pt_id, "DelFeaPolySparPtByID" );
    if ( index < 0 )
    {
        return;
    }

    DelFeaPolySparPtImpl( pspar_id, index, "DelFeaPolySparPtByID" );
}

void DelAllFeaPolySparPt( const std::string & pspar_id )
{
    FeaPart* part = StructureMgr.GetFeaPart( pspar_id );
    if ( !part )
    {
        ErrorMgr.AddError( VSP_INVALID_ID, "DelAllFeaPolySparPt::Can't Find FEA Part " + pspar_id );
        return;
    }

    FeaPolySpar* pspar = dynamic_cast< FeaPolySpar* >( part );
    if( !pspar )
    {
        ErrorMgr.AddError( VSP_INVALID_TYPE, "DelAllFeaPolySparPt::FEA Part " + pspar_id + " is not a poly spar" );
        return;
    }

    pspar->DelAllPt();

    ErrorMgr.NoError();
    return;
}

static int MoveFeaPolySparPtImpl( const std::string & pspar_id, int index, int reorder_type, const std::string & caller )
{
    FeaPart* part = StructureMgr.GetFeaPart( pspar_id );
    if ( !part )
    {
        ErrorMgr.AddError( VSP_INVALID_ID, caller + "::Can't Find FEA Part " + pspar_id );
        return index;
    }

    FeaPolySpar* pspar = dynamic_cast< FeaPolySpar* >( part );
    if( !pspar )
    {
        ErrorMgr.AddError( VSP_INVALID_TYPE, caller + "::FEA Part " + pspar_id + " is not a poly spar" );
        return index;
    }

    if ( !pspar->ValidPtIndex( index ) )
    {
        ErrorMgr.AddError( VSP_INDEX_OUT_RANGE, caller + "::Point index out of range " + to_string( index )  );
        return index;
    }

    ErrorMgr.NoError();
    return pspar->MovePt( index, reorder_type );
}

int MoveFeaPolySparPt( const std::string & pspar_id, int index, int reorder_type )
{
    return MoveFeaPolySparPtImpl( pspar_id, index, reorder_type, "MoveFeaPolySparPt" );
}

int MoveFeaPolySparPtByID( const std::string & pspar_id, const std::string & pt_id, int reorder_type )
{
    FeaPolySpar* pspar = FindFeaPolySpar( pspar_id, "MoveFeaPolySparPtByID" );
    if ( !pspar )
    {
        return -1;
    }

    int index = FindFeaPolySparPtIndex( pspar, pt_id, "MoveFeaPolySparPtByID" );
    if ( index < 0 )
    {
        return -1;
    }

    return MoveFeaPolySparPtImpl( pspar_id, index, reorder_type, "MoveFeaPolySparPtByID" );
}

static void SetFeaPolySparPtNameImpl( const std::string & pspar_id, int index, const std::string & name, const std::string & caller )
{
    FeaPart* part = StructureMgr.GetFeaPart( pspar_id );
    if ( !part )
    {
        ErrorMgr.AddError( VSP_INVALID_ID, caller + "::Can't Find FEA Part " + pspar_id );
        return;
    }

    FeaPolySpar* pspar = dynamic_cast< FeaPolySpar* >( part );
    if( !pspar )
    {
        ErrorMgr.AddError( VSP_INVALID_TYPE, caller + "::FEA Part " + pspar_id + " is not a poly spar" );
        return;
    }

    if ( !pspar->ValidPtIndex( index ) )
    {
        ErrorMgr.AddError( VSP_INDEX_OUT_RANGE, caller + "::Point index out of range " + to_string( index )  );
        return;
    }

    PolySparPoint * pt = pspar->GetPt( index );

    if ( pt )
    {
        pt->SetName( name );
    }

    ErrorMgr.NoError();
    return;
}

void SetFeaPolySparPtName( const std::string & pspar_id, int index, const std::string & name )
{
    SetFeaPolySparPtNameImpl( pspar_id, index, name, "SetFeaPolySparPtName" );
}

void SetFeaPolySparPtNameByID( const std::string & pspar_id, const std::string & pt_id, const std::string & name )
{
    FeaPolySpar* pspar = FindFeaPolySpar( pspar_id, "SetFeaPolySparPtNameByID" );
    if ( !pspar )
    {
        return;
    }

    int index = FindFeaPolySparPtIndex( pspar, pt_id, "SetFeaPolySparPtNameByID" );
    if ( index < 0 )
    {
        return;
    }

    SetFeaPolySparPtNameImpl( pspar_id, index, name, "SetFeaPolySparPtNameByID" );
}

static std::string GetFeaPolySparPtNameImpl( const std::string & pspar_id, int index, const std::string & caller )
{
    FeaPart* part = StructureMgr.GetFeaPart( pspar_id );
    if ( !part )
    {
        ErrorMgr.AddError( VSP_INVALID_ID, caller + "::Can't Find FEA Part " + pspar_id );
        return std::string();
    }

    FeaPolySpar* pspar = dynamic_cast< FeaPolySpar* >( part );
    if( !pspar )
    {
        ErrorMgr.AddError( VSP_INVALID_TYPE, caller + "::FEA Part " + pspar_id + " is not a poly spar" );
        return std::string();
    }

    if ( !pspar->ValidPtIndex( index ) )
    {
        ErrorMgr.AddError( VSP_INDEX_OUT_RANGE, caller + "::Point index out of range " + to_string( index )  );
        return std::string();
    }

    PolySparPoint * pt = pspar->GetPt( index );

    if ( pt )
    {
        ErrorMgr.NoError();
        return pt->GetName();
    }


    return std::string();
}

std::string GetFeaPolySparPtName( const std::string & pspar_id, int index )
{
    return GetFeaPolySparPtNameImpl( pspar_id, index, "GetFeaPolySparPtName" );
}

std::string GetFeaPolySparPtNameByID( const std::string & pspar_id, const std::string & pt_id )
{
    FeaPolySpar* pspar = FindFeaPolySpar( pspar_id, "GetFeaPolySparPtNameByID" );
    if ( !pspar )
    {
        return std::string();
    }

    int index = FindFeaPolySparPtIndex( pspar, pt_id, "GetFeaPolySparPtNameByID" );
    if ( index < 0 )
    {
        return std::string();
    }

    return GetFeaPolySparPtNameImpl( pspar_id, index, "GetFeaPolySparPtNameByID" );
}

std::string GetFeaPolySparPtID( const std::string & pspar_id, int index )
{
    FeaPart* part = StructureMgr.GetFeaPart( pspar_id );
    if ( !part )
    {
        ErrorMgr.AddError( VSP_INVALID_ID, "GetFeaPolySparPtID::Can't Find FEA Part " + pspar_id );
        return std::string();
    }

    FeaPolySpar* pspar = dynamic_cast< FeaPolySpar* >( part );
    if( !pspar )
    {
        ErrorMgr.AddError( VSP_INVALID_TYPE, "GetFeaPolySparPtID::Part is not a poly spar " + pspar_id );
        return std::string();
    }

    if ( !pspar->ValidPtIndex( index ) )
    {
        ErrorMgr.AddError( VSP_INDEX_OUT_RANGE, "GetFeaPolySparPtID::Point index out of range " + to_string( index )  );
        return std::string();
    }

    PolySparPoint * pt = pspar->GetPt( index );

    if ( pt )
    {
        ErrorMgr.NoError();
        return pt->GetID();
    }

    return std::string();
}

std::vector < std::string > GetAllFeaPolySparPtIDVec( const std::string & pspar_id )
{
    std::vector < std::string > ret;
    FeaPart* part = StructureMgr.GetFeaPart( pspar_id );
    if ( !part )
    {
        ErrorMgr.AddError( VSP_INVALID_ID, "GetAllFeaPolySparPtIDVec::Can't Find FEA Part " + pspar_id );
        return ret;
    }

    FeaPolySpar* pspar = dynamic_cast< FeaPolySpar* >( part );
    if( !pspar )
    {
        ErrorMgr.AddError( VSP_INVALID_TYPE, "GetAllFeaPolySparPtIDVec::Part is not a poly spar " + pspar_id );
        return ret;
    }

    ret = pspar->GetAllPtIDVec();

    ErrorMgr.NoError();
    return ret;
}

int NumFeaStructures()
{
    ErrorMgr.NoError();
    return StructureMgr.NumFeaStructures();
}

int NumFeaParts( const std::string & fea_struct_id )
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

int NumFeaSubSurfs( const std::string & fea_struct_id )
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

std::vector< std::string > GetFeaPartIDVec( const std::string & fea_struct_id )
{
    std::vector < std::string > ret_vec;
    FeaStructure* feastruct = StructureMgr.GetFeaStruct( fea_struct_id );
    if ( !feastruct )
    {
        ErrorMgr.AddError( VSP_INVALID_ID, "GetFeaPartIDVec::Invalid FeaStructure ID " + fea_struct_id );
        return ret_vec;
    }

    std::vector < FeaPart* > fea_part_vec = feastruct->GetFeaPartVec();
    ret_vec.resize( fea_part_vec.size() );

    for ( size_t i = 0; i < fea_part_vec.size(); i++ )
    {
        ret_vec[i] = fea_part_vec[i]->GetID();
    }

    ErrorMgr.NoError();
    return ret_vec;
}

std::vector< std::string > GetFeaSubSurfIDVec( const std::string & fea_struct_id )
{
    std::vector < std::string > ret_vec;
    FeaStructure* feastruct = StructureMgr.GetFeaStruct( fea_struct_id );
    if ( !feastruct )
    {
        ErrorMgr.AddError( VSP_INVALID_ID, "GetFeaSubSurfIDVec::Invalid FeaStructure ID " + fea_struct_id );
        return ret_vec;
    }

    std::vector < SubSurface* > fea_ss_vec = feastruct->GetFeaSubSurfVec();
    ret_vec.resize( fea_ss_vec.size() );

    for ( size_t i = 0; i < fea_ss_vec.size(); i++ )
    {
        ret_vec[i] = fea_ss_vec[i]->GetID();
    }

    ErrorMgr.NoError();
    return ret_vec;
}

std::string AddFeaBC( const std::string & fea_struct_id, int type )
{
    FeaStructure* feastruct = StructureMgr.GetFeaStruct( fea_struct_id );
    if ( !feastruct )
    {
        ErrorMgr.AddError( VSP_INVALID_ID, "AddFeaBC::Invalid FeaStructure ID " + fea_struct_id );
        return std::string();
    }

    FeaBC* bc = feastruct->AddFeaBC( type );
    if ( !bc )
    {
        ErrorMgr.AddError( VSP_INVALID_PTR, "AddFeaBC::Invalid FeaBC Ptr" );
        return std::string();
    }

    std::string bc_id = bc->GetID();

    ErrorMgr.NoError();
    return bc_id;
}

void DelFeaBC( const std::string & fea_struct_id, const std::string &bc_id )
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
        ErrorMgr.AddError( VSP_INVALID_ID, "DelFeaBC::Can't Find FeaBC " + bc_id );
        return;
    }

    feastruct->DelFeaBC( indx );

    ErrorMgr.NoError();
}

std::vector< std::string > GetFeaBCIDVec( const std::string & fea_struct_id )
{
    std::vector < std::string > ret_vec;
    FeaStructure* feastruct = StructureMgr.GetFeaStruct( fea_struct_id );
    if ( !feastruct )
    {
        ErrorMgr.AddError( VSP_INVALID_ID, "GetFeaBCIDVec::Invalid FeaStructure ID " + fea_struct_id );
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

int NumFeaBCs( const std::string & fea_struct_id )
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

//==== Look up a FEA boundary condition and its Structure by ID ====//
FeaBC* FindFeaBC( const std::string & bc_id, FeaStructure* & feastruct, const std::string & caller )
{
    std::vector < FeaStructure* > struct_vec = StructureMgr.GetAllFeaStructs();
    for ( size_t i = 0; i < struct_vec.size(); i++ )
    {
        int indx = struct_vec[i]->GetFeaBCIndex( bc_id );
        if ( struct_vec[i]->ValidFeaBCInd( indx ) )
        {
            feastruct = struct_vec[i];
            return struct_vec[i]->GetFeaBC( indx );
        }
    }

    ErrorMgr.AddError( VSP_INVALID_ID, caller + "::Can't Find FeaBC " + bc_id );
    feastruct = nullptr;
    return nullptr;
}

void SetFeaBCPartID( const std::string & bc_id, const std::string & part_id )
{
    FeaStructure* feastruct = nullptr;
    FeaBC* bc = FindFeaBC( bc_id, feastruct, "SetFeaBCPartID" );
    if ( !bc )
    {
        return;
    }

    if ( bc->m_FeaBCType() != FEA_BC_PART )
    {
        ErrorMgr.AddError( VSP_INVALID_TYPE, "SetFeaBCPartID::FeaBC " + bc_id + " is not a part boundary condition" );
        return;
    }

    if ( feastruct->GetFeaPartIndex( part_id ) < 0 )
    {
        ErrorMgr.AddError( VSP_INVALID_ID, "SetFeaBCPartID::Can't Find FEA Part " + part_id + " in FeaStructure " + feastruct->GetID() );
        return;
    }

    bc->SetPartID( part_id );
    ErrorMgr.NoError();
}

std::string GetFeaBCPartID( const std::string & bc_id )
{
    FeaStructure* feastruct = nullptr;
    FeaBC* bc = FindFeaBC( bc_id, feastruct, "GetFeaBCPartID" );
    if ( !bc )
    {
        return std::string();
    }

    ErrorMgr.NoError();
    return bc->GetPartID();
}

void SetFeaBCSubSurfID( const std::string & bc_id, const std::string & ss_id )
{
    FeaStructure* feastruct = nullptr;
    FeaBC* bc = FindFeaBC( bc_id, feastruct, "SetFeaBCSubSurfID" );
    if ( !bc )
    {
        return;
    }

    if ( bc->m_FeaBCType() != FEA_BC_SUBSURF )
    {
        ErrorMgr.AddError( VSP_INVALID_TYPE, "SetFeaBCSubSurfID::FeaBC " + bc_id + " is not a subsurface boundary condition" );
        return;
    }

    if ( FindFeaPartStruct( ss_id ) != feastruct || feastruct->GetFeaPartIndex( ss_id ) >= 0 )
    {
        ErrorMgr.AddError( VSP_INVALID_ID, "SetFeaBCSubSurfID::Can't Find FEA SubSurface " + ss_id + " in FeaStructure " + feastruct->GetID() );
        return;
    }

    bc->SetSubSurfID( ss_id );
    ErrorMgr.NoError();
}

std::string GetFeaBCSubSurfID( const std::string & bc_id )
{
    FeaStructure* feastruct = nullptr;
    FeaBC* bc = FindFeaBC( bc_id, feastruct, "GetFeaBCSubSurfID" );
    if ( !bc )
    {
        return std::string();
    }

    ErrorMgr.NoError();
    return bc->GetSubSurfID();
}

/// Add an FeaMaterial, return FeaMaterial ID
//==== Look up an FEA Assembly by ID, reporting through the named caller. ====//
FeaAssembly* FindFeaAssembly( const std::string & assembly_id, const std::string & caller )
{
    FeaAssembly* assy = StructureMgr.GetFeaAssembly( assembly_id );

    if ( !assy )
    {
        ErrorMgr.AddError( VSP_INVALID_ID, caller + "::Invalid FEA Assembly ID " + assembly_id );
    }

    return assy;
}

std::string AddFeaAssembly()
{
    FeaAssembly* assy = StructureMgr.AddFeaAssembly();

    if ( !assy )
    {
        ErrorMgr.AddError( VSP_INVALID_PTR, "AddFeaAssembly::Invalid FEA Assembly Ptr" );
        return std::string();
    }

    ErrorMgr.NoError();
    return assy->GetID();
}

void DeleteFeaAssembly( const std::string & assembly_id )
{
    std::vector < FeaAssembly* > assy_vec = StructureMgr.GetFeaAssemblyVec();

    for ( int i = 0; i < ( int )assy_vec.size(); i++ )
    {
        if ( assy_vec[i]->GetID() == assembly_id )
        {
            StructureMgr.DeleteFeaAssembly( i );

            ErrorMgr.NoError();
            return;
        }
    }

    ErrorMgr.AddError( VSP_INVALID_ID, "DeleteFeaAssembly::Invalid FEA Assembly ID " + assembly_id );
}

int NumFeaAssemblies()
{
    ErrorMgr.NoError();
    return StructureMgr.NumFeaAssembly();
}

std::vector < std::string > GetFeaAssemblyIDVec()
{
    std::vector < std::string > ret;

    std::vector < FeaAssembly* > assy_vec = StructureMgr.GetFeaAssemblyVec();

    ret.reserve( assy_vec.size() );
    for ( int i = 0; i < ( int )assy_vec.size(); i++ )
    {
        ret.push_back( assy_vec[i]->GetID() );
    }

    ErrorMgr.NoError();
    return ret;
}

std::string GetFeaAssemblyName( const std::string & assembly_id )
{
    FeaAssembly* assy = FindFeaAssembly( assembly_id, "GetFeaAssemblyName" );
    if ( !assy )
    {
        return std::string();
    }

    ErrorMgr.NoError();
    return assy->GetName();
}

void SetFeaAssemblyName( const std::string & assembly_id, const std::string & name )
{
    FeaAssembly* assy = FindFeaAssembly( assembly_id, "SetFeaAssemblyName" );
    if ( !assy )
    {
        return;
    }

    assy->SetName( name );
    assy->ResetExportFileNames();

    ErrorMgr.NoError();
}

void AddFeaStructureToAssembly( const std::string & assembly_id, const std::string & struct_id )
{
    FeaAssembly* assy = FindFeaAssembly( assembly_id, "AddFeaStructureToAssembly" );
    if ( !assy )
    {
        return;
    }

    if ( !StructureMgr.GetFeaStruct( struct_id ) )
    {
        ErrorMgr.AddError( VSP_INVALID_ID, "AddFeaStructureToAssembly::Invalid FeaStructure ID " + struct_id );
        return;
    }

    assy->AddStructure( struct_id );

    ErrorMgr.NoError();
}

void DeleteFeaStructureFromAssembly( const std::string & assembly_id, const std::string & struct_id )
{
    FeaAssembly* assy = FindFeaAssembly( assembly_id, "DeleteFeaStructureFromAssembly" );
    if ( !assy )
    {
        return;
    }

    if ( !vector_contains_val( assy->m_StructIDVec, struct_id ) )
    {
        ErrorMgr.AddError( VSP_INVALID_ID, "DeleteFeaStructureFromAssembly::Assembly Does Not Hold Structure " + struct_id );
        return;
    }

    assy->DelStructure( struct_id );

    ErrorMgr.NoError();
}

void AddFeaAssemblyStructure( const std::string & assembly_id, const std::string & struct_id )
{
    FeaAssembly* assy = FindFeaAssembly( assembly_id, "AddFeaAssemblyStructure" );
    if ( !assy )
    {
        return;
    }

    // The assembly holds structures by ID, and nothing downstream checks that the ID names one, so
    // it is checked here.
    if ( StructureMgr.GetFeaStruct( struct_id ) == nullptr )
    {
        ErrorMgr.AddError( VSP_INVALID_ID, "AddFeaAssemblyStructure::Can't Find Structure " + struct_id );
        return;
    }

    if ( vector_contains_val( assy->m_StructIDVec, struct_id ) )
    {
        ErrorMgr.AddError( VSP_INVALID_INPUT_VAL, "AddFeaAssemblyStructure::Structure Already In Assembly " + struct_id );
        return;
    }

    assy->AddStructure( struct_id );

    ErrorMgr.NoError();
}

void DeleteFeaAssemblyStructure( const std::string & assembly_id, const std::string & struct_id )
{
    FeaAssembly* assy = FindFeaAssembly( assembly_id, "DeleteFeaAssemblyStructure" );
    if ( !assy )
    {
        return;
    }

    if ( !vector_contains_val( assy->m_StructIDVec, struct_id ) )
    {
        ErrorMgr.AddError( VSP_INVALID_ID, "DeleteFeaAssemblyStructure::Structure Not In Assembly " + struct_id );
        return;
    }

    assy->DelStructure( struct_id );

    ErrorMgr.NoError();
}

std::vector < std::string > GetFeaAssemblyStructureIDVec( const std::string & assembly_id )
{
    std::vector < std::string > ret;

    FeaAssembly* assy = FindFeaAssembly( assembly_id, "GetFeaAssemblyStructureIDVec" );
    if ( !assy )
    {
        return ret;
    }

    ErrorMgr.NoError();
    return assy->m_StructIDVec;
}

//==== Whether a fixed point lies in a Structure, reporting through the
//==== named caller when it does not ====//
bool FeaStructHoldsFixPoint( const std::string & fix_pt_id, const std::string & struct_id, const std::string & caller )
{
    FeaStructure* feastruct = StructureMgr.GetFeaStruct( struct_id );
    FeaPart* part = StructureMgr.GetFeaPart( fix_pt_id );
    if ( !feastruct || !part || part->GetType() != FEA_FIX_POINT || feastruct->GetFeaPartIndex( fix_pt_id ) < 0 )
    {
        ErrorMgr.AddError( VSP_INVALID_ID, caller + "::" + fix_pt_id + " is not a fixed point of FeaStructure " + struct_id );
        return false;
    }
    return true;
}

std::string AddFeaAssemblyConnection( const std::string & assembly_id,
                                      const std::string & start_fix_pt_id, const std::string & start_struct_id, int start_index,
                                      const std::string & end_fix_pt_id, const std::string & end_struct_id, int end_index )
{
    FeaAssembly* assy = FindFeaAssembly( assembly_id, "AddFeaAssemblyConnection" );
    if ( !assy )
    {
        return std::string();
    }

    if ( !FeaStructHoldsFixPoint( start_fix_pt_id, start_struct_id, "AddFeaAssemblyConnection" ) ||
         !FeaStructHoldsFixPoint( end_fix_pt_id, end_struct_id, "AddFeaAssemblyConnection" ) )
    {
        return std::string();
    }

    size_t nconn = assy->m_ConnectionVec.size();

    assy->AddConnection( start_fix_pt_id, start_struct_id, start_index,
                         end_fix_pt_id, end_struct_id, end_index );

    if ( assy->m_ConnectionVec.size() == nconn )
    {
        ErrorMgr.AddError( VSP_INVALID_INPUT_VAL, "AddFeaAssemblyConnection::A connection needs two different fixed points" );
        return std::string();
    }

    ErrorMgr.NoError();
    return assy->m_ConnectionVec.back()->GetID();
}

void DeleteFeaAssemblyConnection( const std::string & assembly_id, int connection_index )
{
    FeaAssembly* assy = FindFeaAssembly( assembly_id, "DeleteFeaAssemblyConnection" );
    if ( !assy )
    {
        return;
    }

    if ( connection_index < 0 || connection_index >= ( int )assy->m_ConnectionVec.size() )
    {
        ErrorMgr.AddError( VSP_INDEX_OUT_RANGE, "DeleteFeaAssemblyConnection::Connection Index " + to_string( connection_index ) + " Out of Range" );
        return;
    }

    assy->DelConnection( connection_index );

    ErrorMgr.NoError();
}

void DeleteFeaAssemblyConnectionByID( const std::string & assembly_id, const std::string & connection_id )
{
    FeaAssembly* assy = FindFeaAssembly( assembly_id, "DeleteFeaAssemblyConnectionByID" );
    if ( !assy )
    {
        return;
    }

    for ( int i = 0; i < ( int )assy->m_ConnectionVec.size(); i++ )
    {
        if ( assy->m_ConnectionVec[i] && assy->m_ConnectionVec[i]->GetID() == connection_id )
        {
            assy->DelConnection( i );
            ErrorMgr.NoError();
            return;
        }
    }

    ErrorMgr.AddError( VSP_INVALID_ID, "DeleteFeaAssemblyConnectionByID::Can't Find Connection " + connection_id + " in FEA Assembly " + assembly_id );
}

int NumFeaAssemblyConnections( const std::string & assembly_id )
{
    FeaAssembly* assy = FindFeaAssembly( assembly_id, "NumFeaAssemblyConnections" );
    if ( !assy )
    {
        return -1;
    }

    ErrorMgr.NoError();
    return ( int )assy->m_ConnectionVec.size();
}

std::string GetFeaAssemblyConnectionID( const std::string & assembly_id, int connection_index )
{
    FeaAssembly* assy = FindFeaAssembly( assembly_id, "GetFeaAssemblyConnectionID" );
    if ( !assy )
    {
        return std::string();
    }

    if ( connection_index < 0 || connection_index >= ( int )assy->m_ConnectionVec.size() )
    {
        ErrorMgr.AddError( VSP_INDEX_OUT_RANGE, "GetFeaAssemblyConnectionID::connection_index " +
                           to_string( connection_index ) + " out of range" );
        return std::string();
    }

    if ( !assy->m_ConnectionVec[ connection_index ] )
    {
        ErrorMgr.AddError( VSP_INVALID_PTR, "GetFeaAssemblyConnectionID::Connection " +
                           to_string( connection_index ) + " is empty" );
        return std::string();
    }

    ErrorMgr.NoError();
    return assy->m_ConnectionVec[ connection_index ]->GetID();
}

std::vector < std::string > GetFeaAssemblyConnectionIDVec( const std::string & assembly_id )
{
    std::vector < std::string > ret_vec;

    FeaAssembly* assy = FindFeaAssembly( assembly_id, "GetFeaAssemblyConnectionIDVec" );
    if ( !assy )
    {
        return ret_vec;
    }

    for ( size_t i = 0; i < assy->m_ConnectionVec.size(); i++ )
    {
        if ( assy->m_ConnectionVec[i] )
        {
            ret_vec.push_back( assy->m_ConnectionVec[i]->GetID() );
        }
    }

    ErrorMgr.NoError();
    return ret_vec;
}

//==== Look up a FEA connection by ID, reporting through the named caller ====//
FeaConnection* FindFeaConnection( const std::string & connection_id, const std::string & caller )
{
    FeaConnection* conn = dynamic_cast < FeaConnection* > ( ParmMgr.FindParmContainer( connection_id ) );
    if ( !conn )
    {
        ErrorMgr.AddError( VSP_INVALID_ID, caller + "::Can't Find FEA Connection " + connection_id );
    }
    return conn;
}

std::string GetFeaConnectionStartFixPtID( const std::string & connection_id )
{
    FeaConnection* conn = FindFeaConnection( connection_id, "GetFeaConnectionStartFixPtID" );
    if ( !conn )
    {
        return std::string();
    }

    ErrorMgr.NoError();
    return conn->m_StartFixPtID;
}

std::string GetFeaConnectionStartStructID( const std::string & connection_id )
{
    FeaConnection* conn = FindFeaConnection( connection_id, "GetFeaConnectionStartStructID" );
    if ( !conn )
    {
        return std::string();
    }

    ErrorMgr.NoError();
    return conn->m_StartStructID;
}

std::string GetFeaConnectionEndFixPtID( const std::string & connection_id )
{
    FeaConnection* conn = FindFeaConnection( connection_id, "GetFeaConnectionEndFixPtID" );
    if ( !conn )
    {
        return std::string();
    }

    ErrorMgr.NoError();
    return conn->m_EndFixPtID;
}

std::string GetFeaConnectionEndStructID( const std::string & connection_id )
{
    FeaConnection* conn = FindFeaConnection( connection_id, "GetFeaConnectionEndStructID" );
    if ( !conn )
    {
        return std::string();
    }

    ErrorMgr.NoError();
    return conn->m_EndStructID;
}

std::string GetFeaAssemblyFileName( const std::string & assembly_id, int file_type )
{
    FeaAssembly* assy = FindFeaAssembly( assembly_id, "GetFeaAssemblyFileName" );
    if ( !assy )
    {
        return std::string();
    }

    ErrorMgr.NoError();
    return assy->m_AssemblySettings.GetExportFileName( file_type );
}

void SetFeaAssemblyFileName( const std::string & assembly_id, int file_type, const std::string & file_name )
{
    FeaAssembly* assy = FindFeaAssembly( assembly_id, "SetFeaAssemblyFileName" );
    if ( !assy )
    {
        return;
    }

    assy->m_AssemblySettings.SetExportFileName( file_name, file_type );

    ErrorMgr.NoError();
}

void ComputeFeaAssemblyMesh( const std::string & assembly_id )
{
    FeaAssembly* assy = FindFeaAssembly( assembly_id, "ComputeFeaAssemblyMesh" );
    if ( !assy )
    {
        return;
    }

    // Mesh whatever structures in the assembly have not been meshed yet, then
    // write the assembly out.
    for ( int i = 0; i < ( int )assy->m_StructIDVec.size(); i++ )
    {
        FeaMeshMgr.SetFeaMeshStructID( assy->m_StructIDVec[i] );
        FeaMeshMgr.UpdateStructure();
    }

    FeaMeshMgr.MeshUnMeshed( assy->m_StructIDVec );
    FeaMeshMgr.ExportAssemblyMesh( assembly_id );

    ErrorMgr.NoError();
}

std::vector < std::string > GetFeaMaterialIDVec()
{
    std::vector < std::string > ret;

    std::vector < FeaMaterial* > mat_vec = StructureMgr.GetFeaMaterialVec();

    ret.reserve( mat_vec.size() );
    for ( int i = 0; i < ( int )mat_vec.size(); i++ )
    {
        ret.push_back( mat_vec[i]->GetID() );
    }

    ErrorMgr.NoError();
    return ret;
}

void DeleteFeaMaterial( const std::string &id )
{
    if ( !StructureMgr.GetFeaMaterial( id ) )
    {
        ErrorMgr.AddError( VSP_INVALID_ID, "DeleteFeaMaterial::Invalid FeaMaterial ID " + id );
        return;
    }

    // The built in materials belong to the library and are refused.
    if ( !StructureMgr.DeleteFeaMaterial( id ) )
    {
        ErrorMgr.AddError( VSP_INVALID_INPUT_VAL, "DeleteFeaMaterial::Can't Delete FeaMaterial " + id );
        return;
    }

    ErrorMgr.NoError();
}

std::vector < std::string > GetFeaPropertyIDVec()
{
    std::vector < std::string > ret;

    std::vector < FeaProperty* > prop_vec = StructureMgr.GetFeaPropertyVec();

    ret.reserve( prop_vec.size() );
    for ( int i = 0; i < ( int )prop_vec.size(); i++ )
    {
        ret.push_back( prop_vec[i]->GetID() );
    }

    ErrorMgr.NoError();
    return ret;
}

void DeleteFeaProperty( const std::string &id )
{
    if ( !StructureMgr.GetFeaProperty( id ) )
    {
        ErrorMgr.AddError( VSP_INVALID_ID, "DeleteFeaProperty::Invalid FeaProperty ID " + id );
        return;
    }

    StructureMgr.DeleteFeaProperty( id );

    ErrorMgr.NoError();
}

std::string AddFeaMaterial()
{
    FeaMaterial* feamat = nullptr;
    feamat = StructureMgr.AddFeaMaterial();
    if ( !feamat )
    {
        ErrorMgr.AddError( VSP_INVALID_PTR, "AddFeaMaterial::Invalid FeaMaterial Ptr" );
        return std::string();
    }
    ErrorMgr.NoError();
    return feamat->GetID();
}

/// Add an FeaProperty, return FeaProperty ID. The default is shell property type
std::string AddFeaProperty( int property_type )
{
    FeaProperty* feaprop = nullptr;
    feaprop = StructureMgr.AddFeaProperty( property_type );
    if ( !feaprop )
    {
        ErrorMgr.AddError( VSP_INVALID_PTR, "AddFeaProperty::Invalid FeaProperty Ptr" );
        return std::string();
    }
    ErrorMgr.NoError();
    return feaprop->GetID();
}

//==== Look up a FEA property by ID, reporting through the named caller ====//
FeaProperty* FindFeaProperty( const std::string & property_id, const std::string & caller )
{
    FeaProperty* prop = StructureMgr.GetFeaProperty( property_id );
    if ( !prop )
    {
        ErrorMgr.AddError( VSP_INVALID_ID, caller + "::Can't Find FEA Property " + property_id );
    }
    return prop;
}

void SetFeaPropertyMaterialID( const std::string & property_id, const std::string & material_id )
{
    FeaProperty* prop = FindFeaProperty( property_id, "SetFeaPropertyMaterialID" );
    if ( !prop )
    {
        return;
    }

    FeaMaterial* mat = StructureMgr.GetFeaMaterial( material_id );
    if ( !mat )
    {
        ErrorMgr.AddError( VSP_INVALID_ID, "SetFeaPropertyMaterialID::Can't Find FEA Material " + material_id );
        return;
    }

    if ( prop->m_FeaPropertyType() == FEA_BEAM && mat->m_FeaMaterialType() != FEA_ISOTROPIC )
    {
        ErrorMgr.AddError( VSP_INVALID_TYPE, "SetFeaPropertyMaterialID::FEA Material " + material_id + " is not isotropic, which a beam property needs" );
        return;
    }

    prop->m_FeaMaterialIndex = -1;
    prop->m_FeaMaterialID = material_id;
    ErrorMgr.NoError();
}

std::string GetFeaPropertyMaterialID( const std::string & property_id )
{
    FeaProperty* prop = FindFeaProperty( property_id, "GetFeaPropertyMaterialID" );
    if ( !prop )
    {
        return std::string();
    }

    // A legacy material index, while set, decides the material on the next Update
    if ( prop->m_FeaMaterialIndex() != -1 )
    {
        prop->Update();
    }

    ErrorMgr.NoError();
    return prop->m_FeaMaterialID;
}

static double GetFeaMeshValImpl( FeaStructure* feastruct, int type, const std::string & caller )
{
    double ret = 0.0;

    if ( type == CFD_MIN_EDGE_LEN )
        ret = feastruct->GetFeaGridDensityPtr()->m_MinLen();
    else if ( type == CFD_MAX_EDGE_LEN )
        ret = feastruct->GetFeaGridDensityPtr()->m_BaseLen();
    else if ( type == CFD_MAX_GAP )
        ret = feastruct->GetFeaGridDensityPtr()->m_MaxGap();
    else if ( type == CFD_NUM_CIRCLE_SEGS )
        ret = feastruct->GetFeaGridDensityPtr()->m_NCircSeg();
    else if ( type == CFD_GROWTH_RATIO )
        ret = feastruct->GetFeaGridDensityPtr()->m_GrowRatio();
    else if ( type == CFD_LIMIT_GROWTH_FLAG )
        ret = feastruct->GetFeaGridDensityPtr()->GetRigorLimit();
    else if ( type == CFD_HALF_MESH_FLAG )
        ret = feastruct->GetStructSettingsPtr()->m_HalfMeshFlag();
    else if ( type == CFD_SPLIT_JOIN_SURFS_FLAG )
        ret = feastruct->GetStructSettingsPtr()->m_SplitJoinSurfsFlag();
    else if ( type == CFD_PARALLEL_MESH_FLAG )
        ret = feastruct->GetStructSettingsPtr()->m_ParallelMeshFlag();
    else
    {
        ErrorMgr.AddError( VSP_CANT_FIND_TYPE, caller + "::Can't Find Type " + to_string( ( long long )type ) );
        return ret;
    }

    ErrorMgr.NoError();
    return ret;
}

double GetFeaMeshVal( const std::string & geom_id, int fea_struct_ind, int type )
{
    FeaStructure* feastruct = FindFeaStruct( geom_id, fea_struct_ind, "GetFeaMeshVal" );
    if ( !feastruct )
    {
        return 0.0;
    }

    return GetFeaMeshValImpl( feastruct, type, "GetFeaMeshVal" );
}

double GetFeaMeshValByID( const std::string & struct_id, int type )
{
    FeaStructure* feastruct = FindFeaStruct( struct_id, "GetFeaMeshValByID" );
    if ( !feastruct )
    {
        return 0.0;
    }

    return GetFeaMeshValImpl( feastruct, type, "GetFeaMeshValByID" );
}

static std::string GetFeaMeshFileNameImpl( FeaStructure* feastruct, int file_type, const std::string & caller )
{
    ErrorMgr.NoError();
    return feastruct->GetStructSettingsPtr()->GetExportFileName( file_type );
}

std::string GetFeaMeshFileName( const std::string & geom_id, int fea_struct_ind, int file_type )
{
    FeaStructure* feastruct = FindFeaStruct( geom_id, fea_struct_ind, "GetFeaMeshFileName" );
    if ( !feastruct )
    {
        return std::string();
    }

    return GetFeaMeshFileNameImpl( feastruct, file_type, "GetFeaMeshFileName" );
}

std::string GetFeaMeshFileNameByID( const std::string & struct_id, int file_type )
{
    FeaStructure* feastruct = FindFeaStruct( struct_id, "GetFeaMeshFileNameByID" );
    if ( !feastruct )
    {
        return std::string();
    }

    return GetFeaMeshFileNameImpl( feastruct, file_type, "GetFeaMeshFileNameByID" );
}

static void SetFeaMeshValImpl( FeaStructure* feastruct, int type, double val, const std::string & caller )
{
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
    else if ( type == CFD_SPLIT_JOIN_SURFS_FLAG )
        feastruct->GetStructSettingsPtr()->m_SplitJoinSurfsFlag = ToBool( val );
    else if ( type == CFD_PARALLEL_MESH_FLAG )
        feastruct->GetStructSettingsPtr()->m_ParallelMeshFlag = ToBool( val );
    else
    {
        ErrorMgr.AddError( VSP_CANT_FIND_TYPE, caller + "::Can't Find Type " + to_string( (long long)type ) );
        return;
    }

    ErrorMgr.NoError();
}

void SetFeaMeshVal( const std::string & geom_id, int fea_struct_ind, int type, double val )
{
    FeaStructure* feastruct = FindFeaStruct( geom_id, fea_struct_ind, "SetFeaMeshVal" );
    if ( !feastruct )
    {
        return;
    }

    SetFeaMeshValImpl( feastruct, type, val, "SetFeaMeshVal" );
}

void SetFeaMeshValByID( const std::string & struct_id, int type, double val )
{
    FeaStructure* feastruct = FindFeaStruct( struct_id, "SetFeaMeshValByID" );
    if ( !feastruct )
    {
        return;
    }

    SetFeaMeshValImpl( feastruct, type, val, "SetFeaMeshValByID" );
}

static void SetFeaMeshFileNameImpl( FeaStructure* feastruct, int file_type, const std::string & file_name, const std::string & caller )
{
    feastruct->GetStructSettingsPtr()->SetExportFileName( file_name, file_type );

    ErrorMgr.NoError();
}

void SetFeaMeshFileName( const std::string & geom_id, int fea_struct_ind, int file_type, const std::string & file_name )
{
    FeaStructure* feastruct = FindFeaStruct( geom_id, fea_struct_ind, "SetFeaMeshFileName" );
    if ( !feastruct )
    {
        return;
    }

    SetFeaMeshFileNameImpl( feastruct, file_type, file_name, "SetFeaMeshFileName" );
}

void SetFeaMeshFileNameByID( const std::string & struct_id, int file_type, const std::string & file_name )
{
    FeaStructure* feastruct = FindFeaStruct( struct_id, "SetFeaMeshFileNameByID" );
    if ( !feastruct )
    {
        return;
    }

    SetFeaMeshFileNameImpl( feastruct, file_type, file_name, "SetFeaMeshFileNameByID" );
}

/// Compute the FEA Mesh
void ComputeFeaMesh( const std::string & geom_id, int fea_struct_ind, int file_type )
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
        ErrorMgr.AddError( VSP_INVALID_GEOM_ID, "ComputeFeaMesh::Can't Find Geom " + geom_id );
        return;
    }

    FeaStructure* feastruct = geom_ptr->GetFeaStruct( fea_struct_ind );
    if ( !feastruct )
    {
        ErrorMgr.AddError( VSP_INDEX_OUT_RANGE, "ComputeFeaMesh::FeaStructure Index " + to_string( (long long)fea_struct_ind ) + " Out of Range" );
        return;
    }

    feastruct->GetStructSettingsPtr()->SetAllFileExportFlags( false );
    feastruct->GetStructSettingsPtr()->SetFileExportFlag( file_type, true );

    FeaMeshMgr.SetFeaMeshStructID( feastruct->GetID() );
    FeaMeshMgr.UpdateStructure();

    FeaMeshMgr.addOutputText( "CLEAR_TERMINAL" );
    FeaMeshMgr.GenerateFeaMesh();

    FeaMeshMgr.ExportFeaMesh( feastruct->GetID() );
    FeaMeshMgr.ExportCADFiles();

    ErrorMgr.NoError();
}

void ComputeFeaMesh( const std::string & struct_id, int file_type )
{
    Update(); // Not sure if this is needed

    FeaStructure* feastruct = StructureMgr.GetFeaStruct( struct_id );
    if ( !feastruct )
    {
        ErrorMgr.AddError( VSP_INVALID_ID, "ComputeFeaMesh::Can't Find Structure " + struct_id );
        return;
    }

    feastruct->GetStructSettingsPtr()->SetAllFileExportFlags( false );
    feastruct->GetStructSettingsPtr()->SetFileExportFlag( file_type, true );

    FeaMeshMgr.SetFeaMeshStructID( struct_id );
    FeaMeshMgr.UpdateStructure();

    FeaMeshMgr.addOutputText( "CLEAR_TERMINAL" );
    FeaMeshMgr.GenerateFeaMesh();

    FeaMeshMgr.ExportFeaMesh( struct_id );
    FeaMeshMgr.ExportCADFiles();

    ErrorMgr.NoError();
}

void SetXSecAlias( const std::string & id, const std::string & alias )
{
    XSec* xs = FindXSec( id );
    if ( !xs )
    {
        ErrorMgr.AddError( VSP_INVALID_XSEC_ID, "SetXSecAlias::Can't Find XSec " + id );
        return;
    }

    ErrorMgr.NoError();
    xs->SetGroupAlias( alias );
}

std::string GetXSecAlias( const std::string & id )
{
    XSec* xs = FindXSec( id );
    if ( !xs )
    {
        ErrorMgr.AddError( VSP_INVALID_XSEC_ID, "GetXSecAlias::Can't Find XSec " + id );
        return std::string();
    }

    ErrorMgr.NoError();
    return xs->GetGroupAlias();
}

void SetXSecCurveAlias( const std::string & id, const std::string & alias )
{
    XSec* xs = FindXSec( id );
    if ( !xs )
    {
        ErrorMgr.AddError( VSP_INVALID_XSEC_ID, "SetXSecCurveAlias::Can't Find XSec " + id );
        return;
    }
    XSecCurve* xsc = xs->GetXSecCurve();

    ErrorMgr.NoError();
    xsc->SetGroupAlias( alias );
}

std::string GetXSecCurveAlias( const std::string & id )
{
    XSec* xs = FindXSec( id );
    if ( !xs )
    {
        ErrorMgr.AddError( VSP_INVALID_XSEC_ID, "GetXSecCurveAlias::Can't Find XSec " + id );
        return std::string();
    }
    XSecCurve* xsc = xs->GetXSecCurve();

    ErrorMgr.NoError();
    return xsc->GetGroupAlias();
}

// A Clone's cross sections belong to its original, so XSec edits on a Clone are refused.
static bool RefuseOnClone( Geom* geom_ptr, const std::string &fname )
{
    if ( !dynamic_cast< CloneGeom* >( geom_ptr ) )
    {
        return false;
    }

    ErrorMgr.AddError( VSP_WRONG_GEOM_TYPE, fname + "::Geom " + geom_ptr->GetID() +
                       " is a Clone -- change the Geom it copies" );
    return true;
}

//==== The XSecSurf an XSec is on and the XSec's index there, reporting through the
//==== named caller when it cannot be found. ====//
XSecSurf* FindXSecSurfOf( const std::string & xsec_id, int & index, const std::string & caller )
{
    XSecSurf* xsec_surf = nullptr;

    XSec* xs = FindXSec( xsec_id );
    if ( xs )
    {
        xsec_surf = dynamic_cast < XSecSurf* > ( xs->GetParentContainerPtr() );
    }

    if ( !xsec_surf )
    {
        ErrorMgr.AddError( VSP_INVALID_XSEC_ID, caller + "::Can't Find XSec " + xsec_id );
        return nullptr;
    }

    index = xsec_surf->FindXSecIndex( xsec_id );
    return xsec_surf;
}

//==== The Geom an XSec is on and the XSec's index among the Geom's own XSecs,
//==== reporting through the named caller when it cannot be found. ====//
Geom* FindXSecGeom( const std::string & xsec_id, int & index, const std::string & caller )
{
    XSecSurf* xsec_surf = FindXSecSurfOf( xsec_id, index, caller );
    if ( !xsec_surf )
    {
        return nullptr;
    }

    Geom* geom_ptr = dynamic_cast < Geom* > ( xsec_surf->GetParentContainerPtr() );
    if ( !geom_ptr || geom_ptr->GetXSecSurf( 0 ) != xsec_surf )
    {
        ErrorMgr.AddError( VSP_INVALID_PTR, caller + "::XSec " + xsec_id + " is not one of its Geom's own XSecs" );
        return nullptr;
    }

    return geom_ptr;
}

//==== Whether an index names one of a Geom's XSecs that the cut, copy, paste, insert and
//==== split calls may change, reporting through the named caller ====//
bool GeomXSecIndexInRange( Geom* geom_ptr, int index, const std::string & caller )
{
    // A wing's root XSec has no section to cut, copy, paste, insert after or split
    int first = 0;
    if ( geom_ptr->GetType().m_Type == MS_WING_GEOM_TYPE )
    {
        first = 1;
    }

    XSecSurf* xsec_surf = geom_ptr->GetXSecSurf( 0 );
    if ( !xsec_surf || index < first || index >= xsec_surf->NumXSec() )
    {
        ErrorMgr.AddError( VSP_INDEX_OUT_RANGE, caller + "::XSec Index " + to_string( index ) + " Out of Range" );
        return false;
    }
    return true;
}

static void CutXSecImpl( const std::string & geom_id, int index, const std::string & caller )
{
    Vehicle* veh = GetVehicle();
    Geom* geom_ptr = veh->FindGeom( geom_id );
    if ( !geom_ptr )
    {
        ErrorMgr.AddError( VSP_INVALID_GEOM_ID, caller + "::Can't Find Geom " + geom_id );
        return;
    }

    if ( RefuseOnClone( geom_ptr, caller ) || !GeomXSecIndexInRange( geom_ptr, index, caller ) )
    {
        return;
    }

    int nxsec = geom_ptr->GetXSecSurf( 0 )->NumXSec();

    geom_ptr->CutXSec( index );
    Update();

    if ( geom_ptr->GetXSecSurf( 0 )->NumXSec() == nxsec )
    {
        ErrorMgr.AddError( VSP_INVALID_INPUT_VAL, caller + "::Geom " + geom_id + " has no XSec to spare" );
        return;
    }

    ErrorMgr.NoError();
}

void CutXSec( const std::string & geom_id, int index )
{
    CutXSecImpl( geom_id, index, "CutXSec" );
}

void CutXSecByID( const std::string & xsec_id )
{
    int index = -1;
    Geom* geom_ptr = FindXSecGeom( xsec_id, index, "CutXSecByID" );
    if ( !geom_ptr )
    {
        return;
    }

    CutXSecImpl( geom_ptr->GetID(), index, "CutXSecByID" );
}

static void CopyXSecImpl( const std::string & geom_id, int index, const std::string & caller )
{
    Vehicle* veh = GetVehicle();
    Geom* geom_ptr = veh->FindGeom( geom_id );
    if ( !geom_ptr )
    {
        ErrorMgr.AddError( VSP_INVALID_GEOM_ID, caller + "::Can't Find Geom " + geom_id );
        return;
    }

    if ( !GeomXSecIndexInRange( geom_ptr, index, caller ) )
    {
        return;
    }

    geom_ptr->CopyXSec( index );
    ErrorMgr.NoError();
}

void CopyXSec( const std::string & geom_id, int index )
{
    CopyXSecImpl( geom_id, index, "CopyXSec" );
}

void CopyXSecByID( const std::string & xsec_id )
{
    int index = -1;
    Geom* geom_ptr = FindXSecGeom( xsec_id, index, "CopyXSecByID" );
    if ( !geom_ptr )
    {
        return;
    }

    CopyXSecImpl( geom_ptr->GetID(), index, "CopyXSecByID" );
}

static void PasteXSecImpl( const std::string & geom_id, int index, const std::string & caller )
{
    Vehicle* veh = GetVehicle();
    Geom* geom_ptr = veh->FindGeom( geom_id );
    if ( !geom_ptr )
    {
        ErrorMgr.AddError( VSP_INVALID_GEOM_ID, caller + "::Can't Find Geom " + geom_id );
        return;
    }

    if ( RefuseOnClone( geom_ptr, caller ) || !GeomXSecIndexInRange( geom_ptr, index, caller ) )
    {
        return;
    }

    geom_ptr->PasteXSec( index );
    ErrorMgr.NoError();
}

void PasteXSec( const std::string & geom_id, int index )
{
    PasteXSecImpl( geom_id, index, "PasteXSec" );
}

void PasteXSecByID( const std::string & xsec_id )
{
    int index = -1;
    Geom* geom_ptr = FindXSecGeom( xsec_id, index, "PasteXSecByID" );
    if ( !geom_ptr )
    {
        return;
    }

    PasteXSecImpl( geom_ptr->GetID(), index, "PasteXSecByID" );
}

static void InsertXSecImpl( const std::string & geom_id, int index, int type, const std::string & caller )
{
    Vehicle* veh = GetVehicle();
    Geom* geom_ptr = veh->FindGeom( geom_id );
    if ( !geom_ptr )
    {
        ErrorMgr.AddError( VSP_INVALID_GEOM_ID, caller + "::Can't Find Geom " + geom_id );
        return;
    }

    if ( RefuseOnClone( geom_ptr, caller ) || !GeomXSecIndexInRange( geom_ptr, index, caller ) )
    {
        return;
    }

    int nxsec = geom_ptr->GetXSecSurf( 0 )->NumXSec();

    geom_ptr->InsertXSec( index, type );

    if ( geom_ptr->GetXSecSurf( 0 )->NumXSec() == nxsec )
    {
        ErrorMgr.AddError( VSP_INVALID_INPUT_VAL, caller + "::No XSec Inserted After Index " + to_string( index ) );
        return;
    }

    ErrorMgr.NoError();
}

void InsertXSec( const std::string & geom_id, int index, int type )
{
    InsertXSecImpl( geom_id, index, type, "InsertXSec" );
}

std::string InsertXSecByID( const std::string & xsec_id, int type )
{
    int index = -1;
    Geom* geom_ptr = FindXSecGeom( xsec_id, index, "InsertXSecByID" );
    if ( !geom_ptr )
    {
        return std::string();
    }

    XSecSurf* xsec_surf = geom_ptr->GetXSecSurf( 0 );
    int nxsec = xsec_surf->NumXSec();

    InsertXSecImpl( geom_ptr->GetID(), index, type, "InsertXSecByID" );
    if ( ErrorMgr.GetErrorLastCallFlag() )
    {
        return std::string();
    }

    XSec* xs = xsec_surf->FindXSec( index + 1 );
    if ( xsec_surf->NumXSec() == nxsec || !xs )
    {
        ErrorMgr.AddError( VSP_INVALID_INPUT_VAL, "InsertXSecByID::No XSec Inserted After " + xsec_id );
        return std::string();
    }

    return xs->GetID();
}


//===================================================================//
//===============       Wing Section Functions     ==================//
//===================================================================//

static void SplitWingXSecImpl( const std::string & wing_id, int section_index, const std::string & caller )
{
    Vehicle* veh = GetVehicle();
    Geom* geom_ptr = veh->FindGeom( wing_id );
    if ( !geom_ptr )
    {
        ErrorMgr.AddError( VSP_INVALID_GEOM_ID, caller + "::Can't Find Geom " + wing_id );
        return;
    }

    WingGeom* wg = dynamic_cast<WingGeom*>( geom_ptr );
    if ( !wg )
    {
        ErrorMgr.AddError( VSP_WRONG_GEOM_TYPE, caller + "::Geom " + wing_id + " is not a Wing" );
        return;
    }

    if ( !GeomXSecIndexInRange( geom_ptr, section_index, caller ) )
    {
        return;
    }

    wg->SplitWingSect( section_index );
    ErrorMgr.NoError();
}

void SplitWingXSec( const std::string & wing_id, int section_index )
{
    SplitWingXSecImpl( wing_id, section_index, "SplitWingXSec" );
}

std::string SplitWingXSecByID( const std::string & xsec_id )
{
    int index = -1;
    Geom* geom_ptr = FindXSecGeom( xsec_id, index, "SplitWingXSecByID" );
    if ( !geom_ptr )
    {
        return std::string();
    }

    XSecSurf* xsec_surf = geom_ptr->GetXSecSurf( 0 );
    int nxsec = xsec_surf->NumXSec();

    SplitWingXSecImpl( geom_ptr->GetID(), index, "SplitWingXSecByID" );
    if ( ErrorMgr.GetErrorLastCallFlag() )
    {
        return std::string();
    }

    XSec* xs = xsec_surf->FindXSec( index + 1 );
    if ( xsec_surf->NumXSec() == nxsec || !xs )
    {
        ErrorMgr.AddError( VSP_INVALID_INPUT_VAL, "SplitWingXSecByID::Section " + xsec_id + " Was Not Split" );
        return std::string();
    }

    return xs->GetID();
}

static std::vector < int > GetDriverGroupImpl( const std::string & geom_id, int section_index, const std::string & caller )
{
    std::vector < int > choices;

    Vehicle* veh = GetVehicle();
    Geom* geom_ptr = veh->FindGeom( geom_id );
    if ( !geom_ptr )
    {
        ErrorMgr.AddError( VSP_INVALID_GEOM_ID, caller + "::Can't Find Geom " + geom_id );
        return choices;
    }

    // Read-only, so a Clone of a wing answers from its original.  SetDriverGroup does not.
    if ( geom_ptr->GetBehaviorType() == MS_WING_GEOM_TYPE )
    {
        WingGeom* behavior_wing = dynamic_cast<WingGeom*>( geom_ptr->GetBehaviorGeom() );
        WingSect* ws = behavior_wing->GetWingSect( section_index );
        if ( !ws )
        {
            ErrorMgr.AddError( VSP_INDEX_OUT_RANGE, caller + "::Invalid Wing Section Index " + to_string( ( long long )section_index ) );
            return choices;
        }

        ErrorMgr.NoError();
        return ws->m_DriverGroup.GetChoices();
    }

    GeomXSec* gxs = dynamic_cast < GeomXSec* > ( geom_ptr );

    XSecCurve* xsc = nullptr;
    if ( gxs ) // Proceed as GeomXSec
    {
        XSec* xs = gxs->GetXSec( section_index );
        if ( !xs )
        {
            ErrorMgr.AddError( VSP_INDEX_OUT_RANGE, caller + "::Invalid XSec Index " + to_string( ( long long )section_index ) );
            return choices;
        }
        xsc = xs->GetXSecCurve();
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
        ErrorMgr.NoError();
        return xsc->m_DriverGroup->GetChoices();
    }

    ErrorMgr.AddError( VSP_WRONG_GEOM_TYPE, caller + "::Invalid Geom Type " + geom_id );
    return choices;
}

std::vector < int > GetDriverGroup( const std::string & geom_id, int section_index )
{
    return GetDriverGroupImpl( geom_id, section_index, "GetDriverGroup" );
}

std::vector < int > GetDriverGroupByID( const std::string & xsec_id )
{
    int index = -1;
    Geom* geom_ptr = FindXSecGeom( xsec_id, index, "GetDriverGroupByID" );
    if ( !geom_ptr )
    {
        return std::vector < int > ();
    }

    return GetDriverGroupImpl( geom_ptr->GetID(), index, "GetDriverGroupByID" );
}

static void SetDriverGroupImpl( const std::string & geom_id, int section_index, int driver_0, int driver_1, int driver_2, const std::string & caller )
{
    Vehicle* veh = GetVehicle();
    Geom* geom_ptr = veh->FindGeom( geom_id );
    if ( !geom_ptr )
    {
        ErrorMgr.AddError( VSP_INVALID_GEOM_ID, caller + "::Can't Find Geom " + geom_id );
        return;
    }

    if ( geom_ptr->GetType().m_Type == MS_WING_GEOM_TYPE )
    {
        WingGeom* wg = dynamic_cast<WingGeom*>( geom_ptr );
        WingSect* ws = wg->GetWingSect( section_index );
        if ( !ws )
        {
            ErrorMgr.AddError( VSP_INDEX_OUT_RANGE, caller + "::Invalid Wing Section Index " + to_string( ( long long )section_index ) );
            return;
        }

        std::vector < int > prevchoices = ws->m_DriverGroup.GetChoices();

        ws->m_DriverGroup.SetChoice( 0, driver_0 );
        ws->m_DriverGroup.SetChoice( 1, driver_1 );
        ws->m_DriverGroup.SetChoice( 2, driver_2 );

        bool valid = ws->m_DriverGroup.ValidDrivers( ws->m_DriverGroup.GetChoices() );
        if ( !valid )
        {
            ErrorMgr.AddError( VSP_INVALID_DRIVERS, caller + "::Invalid wing drivers." );
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
            XSec* xs = gxs->GetXSec( section_index );
            if ( !xs )
            {
                ErrorMgr.AddError( VSP_INDEX_OUT_RANGE, caller + "::Invalid XSec Index " + to_string( ( long long )section_index ) );
                return;
            }
            xsc = xs->GetXSecCurve();
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
            std::vector < int > prevchoices = xsc->m_DriverGroup->GetChoices();

            // Only driver 0 used for Circles.
            xsc->m_DriverGroup->SetChoice( 0, driver_0 );

            // Driver 1 used for other XSecCurve types.
            if ( driver_1 > -1 )
                xsc->m_DriverGroup->SetChoice( 1, driver_1 );

            bool valid = xsc->m_DriverGroup->ValidDrivers( xsc->m_DriverGroup->GetChoices() );
            if ( !valid )
            {
                ErrorMgr.AddError( VSP_INVALID_DRIVERS, caller + "::Invalid XSecCurve drivers." );
                xsc->m_DriverGroup->SetChoices( prevchoices );
                return;
            }
            ErrorMgr.NoError();
            return;
        }

    }
    ErrorMgr.AddError( VSP_WRONG_GEOM_TYPE, caller + "::Invalid Geom Type " + geom_id );
    return;
}

void SetDriverGroup( const std::string & geom_id, int section_index, int driver_0, int driver_1, int driver_2 )
{
    SetDriverGroupImpl( geom_id, section_index, driver_0, driver_1, driver_2, "SetDriverGroup" );
}

void SetDriverGroupByID( const std::string & xsec_id, int driver_0, int driver_1, int driver_2 )
{
    int index = -1;
    Geom* geom_ptr = FindXSecGeom( xsec_id, index, "SetDriverGroupByID" );
    if ( !geom_ptr )
    {
        return;
    }

    SetDriverGroupImpl( geom_ptr->GetID(), index, driver_0, driver_1, driver_2, "SetDriverGroupByID" );
}

//===================================================================//
//===============       XSecSurf Functions         ==================//
//===================================================================//
/// Get the XSecSurf ID given Geom ID and index
std::string GetXSecSurf( const std::string & geom_id, int index )
{
    Vehicle* veh = GetVehicle();
    Geom* geom_ptr = veh->FindGeom( geom_id );
    if ( !geom_ptr )
    {
        ErrorMgr.AddError( VSP_INVALID_GEOM_ID, "GetXSecSurf::Can't Find Geom " + geom_id );
        return std::string();
    }
    XSecSurf* xsec_surf = geom_ptr->GetXSecSurf( index );

    if ( !xsec_surf )
    {
        ErrorMgr.AddError( VSP_INDEX_OUT_RANGE, "GetXSecSurf::XSecSurf Index " + to_string( ( long long )index ) + " Out of Range for Geom " + geom_id );
        return std::string();
    }

    ErrorMgr.NoError();
    return xsec_surf->GetID();
}

std::vector < std::string > GetXSecSurfIDVec( const std::string & geom_id )
{
    std::vector < std::string > ret_vec;

    Geom* geom_ptr = GetVehicle()->FindGeom( geom_id );
    if ( !geom_ptr )
    {
        ErrorMgr.AddError( VSP_INVALID_GEOM_ID, "GetXSecSurfIDVec::Can't Find Geom " + geom_id );
        return ret_vec;
    }

    for ( int i = 0; i < geom_ptr->GetNumXSecSurfs(); i++ )
    {
        ret_vec.push_back( geom_ptr->GetXSecSurf( i )->GetID() );
    }

    ErrorMgr.NoError();
    return ret_vec;
}

/// Get the number of XSecs in the XSecSurf
int GetNumXSec( const std::string & xsec_surf_id )
{
    XSecSurf* xsec_surf = FindXSecSurf( xsec_surf_id );
    if ( !xsec_surf )
    {
        ErrorMgr.AddError( VSP_INVALID_ID, "GetNumXSec::Can't Find XSecSurf " + xsec_surf_id );
        return 0;
    }

    ErrorMgr.NoError();
    return xsec_surf->NumXSec();
}

/// Get xsec id from xsec_surf at given index.
std::string GetXSec( const std::string & xsec_surf_id, int xsec_index )
{
    XSecSurf* xsec_surf = FindXSecSurf( xsec_surf_id );
    if ( !xsec_surf )
    {
        ErrorMgr.AddError( VSP_INVALID_ID, "GetXSec::Can't Find XSecSurf " + xsec_surf_id );
        return std::string();
    }
    XSec* xsec = xsec_surf->FindXSec( xsec_index );
    if ( !xsec )
    {
        ErrorMgr.AddError( VSP_INDEX_OUT_RANGE, "GetXSec::XSec Index " + to_string( ( long long )xsec_index ) + " Out of Range in XSecSurf " + xsec_surf_id );
        return std::string();
    }

    ErrorMgr.NoError();
    return xsec->GetID();
}

std::vector < std::string > GetXSecIDVec( const std::string & xsec_surf_id )
{
    std::vector < std::string > ret_vec;

    XSecSurf* xsec_surf = FindXSecSurf( xsec_surf_id );
    if ( !xsec_surf )
    {
        ErrorMgr.AddError( VSP_INVALID_ID, "GetXSecIDVec::Can't Find XSecSurf " + xsec_surf_id );
        return ret_vec;
    }

    for ( int i = 0; i < xsec_surf->NumXSec(); i++ )
    {
        ret_vec.push_back( xsec_surf->FindXSec( i )->GetID() );
    }

    ErrorMgr.NoError();
    return ret_vec;
}

//==== One of an engine Geom's face or lip index Parms, reporting through the named caller ====//
IntParm* FindEngineXSecIndex( const std::string & geom_id, int which, const std::string & caller )
{
    GeomEngine* eng = dynamic_cast < GeomEngine* > ( GetVehicle()->FindGeom( geom_id ) );
    if ( !eng )
    {
        ErrorMgr.AddError( VSP_INVALID_GEOM_ID, caller + "::Can't Find Engine Geom " + geom_id );
        return nullptr;
    }

    if ( which == 0 )
    {
        return &eng->m_EngineInFaceIndex;
    }
    if ( which == 1 )
    {
        return &eng->m_EngineInLipIndex;
    }
    if ( which == 2 )
    {
        return &eng->m_EngineOutFaceIndex;
    }
    return &eng->m_EngineOutLipIndex;
}

//==== Set an engine face or lip index Parm from an XSec ID ====//
void SetEngineXSecID( const std::string & geom_id, int which, const std::string & xsec_id, const std::string & caller )
{
    IntParm* indx = FindEngineXSecIndex( geom_id, which, caller );
    if ( !indx )
    {
        return;
    }

    int index = -1;
    Geom* geom_ptr = FindXSecGeom( xsec_id, index, caller );
    if ( !geom_ptr )
    {
        return;
    }

    if ( geom_ptr->GetID() != geom_id )
    {
        ErrorMgr.AddError( VSP_INVALID_ID, caller + "::XSec " + xsec_id + " is not on Geom " + geom_id );
        return;
    }

    indx->Set( index );
    ErrorMgr.NoError();
}

//==== The XSec ID an engine face or lip index Parm names ====//
std::string GetEngineXSecID( const std::string & geom_id, int which, const std::string & caller )
{
    IntParm* indx = FindEngineXSecIndex( geom_id, which, caller );
    if ( !indx )
    {
        return std::string();
    }

    XSec* xs = GetVehicle()->FindGeom( geom_id )->GetXSecSurf( 0 )->FindXSec( indx->Get() );
    if ( !xs )
    {
        ErrorMgr.AddError( VSP_INVALID_PTR, caller + "::Can't Find XSec " + to_string( indx->Get() ) );
        return std::string();
    }

    ErrorMgr.NoError();
    return xs->GetID();
}

void SetEngineInletFaceXSecID( const std::string & geom_id, const std::string & xsec_id )
{
    SetEngineXSecID( geom_id, 0, xsec_id, "SetEngineInletFaceXSecID" );
}

std::string GetEngineInletFaceXSecID( const std::string & geom_id )
{
    return GetEngineXSecID( geom_id, 0, "GetEngineInletFaceXSecID" );
}

void SetEngineInletLipXSecID( const std::string & geom_id, const std::string & xsec_id )
{
    SetEngineXSecID( geom_id, 1, xsec_id, "SetEngineInletLipXSecID" );
}

std::string GetEngineInletLipXSecID( const std::string & geom_id )
{
    return GetEngineXSecID( geom_id, 1, "GetEngineInletLipXSecID" );
}

void SetEngineOutletFaceXSecID( const std::string & geom_id, const std::string & xsec_id )
{
    SetEngineXSecID( geom_id, 2, xsec_id, "SetEngineOutletFaceXSecID" );
}

std::string GetEngineOutletFaceXSecID( const std::string & geom_id )
{
    return GetEngineXSecID( geom_id, 2, "GetEngineOutletFaceXSecID" );
}

void SetEngineOutletLipXSecID( const std::string & geom_id, const std::string & xsec_id )
{
    SetEngineXSecID( geom_id, 3, xsec_id, "SetEngineOutletLipXSecID" );
}

std::string GetEngineOutletLipXSecID( const std::string & geom_id )
{
    return GetEngineXSecID( geom_id, 3, "GetEngineOutletLipXSecID" );
}

//==== A spar's, rib's or rib array's start or end wing section Parm, reporting through
//==== the named caller when the part has none ====//
IntParm* FindFeaPartWingSection( const std::string & part_id, bool end, const std::string & caller )
{
    FeaPart* part = StructureMgr.GetFeaPart( part_id );
    if ( !part )
    {
        ErrorMgr.AddError( VSP_INVALID_ID, caller + "::Can't Find FEA Part " + part_id );
        return nullptr;
    }

    FeaSpar* spar = dynamic_cast < FeaSpar* > ( part );
    FeaRib* rib = dynamic_cast < FeaRib* > ( part );
    FeaRibArray* rib_array = dynamic_cast < FeaRibArray* > ( part );

    if ( spar )
    {
        if ( end )
        {
            return &spar->m_EndWingSection;
        }
        return &spar->m_StartWingSection;
    }
    if ( rib )
    {
        if ( end )
        {
            return &rib->m_EndWingSection;
        }
        return &rib->m_StartWingSection;
    }
    if ( rib_array )
    {
        if ( end )
        {
            return &rib_array->m_EndWingSection;
        }
        return &rib_array->m_StartWingSection;
    }

    ErrorMgr.AddError( VSP_INVALID_TYPE, caller + "::FEA Part " + part_id + " is not a spar, rib or rib array" );
    return nullptr;
}

//==== Set a wing section Parm of a FEA part from the section's XSec ID ====//
void SetFeaPartWingSectionID( const std::string & part_id, const std::string & xsec_id, bool end, const std::string & caller )
{
    IntParm* sect = FindFeaPartWingSection( part_id, end, caller );
    if ( !sect )
    {
        return;
    }

    int index = -1;
    Geom* geom_ptr = FindXSecGeom( xsec_id, index, caller );
    if ( !geom_ptr )
    {
        return;
    }

    if ( geom_ptr->GetID() != StructureMgr.GetFeaPart( part_id )->GetParentGeomID() || index < 1 )
    {
        ErrorMgr.AddError( VSP_INVALID_ID, caller + "::XSec " + xsec_id + " is not a section of the part's wing" );
        return;
    }

    // The part's own limits tie start and end together; open them to the whole wing so one
    // can move past the other, and the part's next Update ties them again
    sect->SetLowerUpperLimits( 1, geom_ptr->GetXSecSurf( 0 )->NumXSec() - 1 );
    sect->Set( index );

    ErrorMgr.NoError();
}

//==== The XSec ID of a wing section Parm of a FEA part ====//
std::string GetFeaPartWingSectionID( const std::string & part_id, bool end, const std::string & caller )
{
    IntParm* sect = FindFeaPartWingSection( part_id, end, caller );
    if ( !sect )
    {
        return std::string();
    }

    Geom* geom_ptr = GetVehicle()->FindGeom( StructureMgr.GetFeaPart( part_id )->GetParentGeomID() );
    XSec* xs = nullptr;
    if ( geom_ptr && geom_ptr->GetXSecSurf( 0 ) )
    {
        xs = geom_ptr->GetXSecSurf( 0 )->FindXSec( sect->Get() );
    }

    if ( !xs )
    {
        ErrorMgr.AddError( VSP_INVALID_PTR, caller + "::Can't Find Wing Section " + to_string( sect->Get() ) );
        return std::string();
    }

    ErrorMgr.NoError();
    return xs->GetID();
}

void SetFeaPartStartWingSectionID( const std::string & part_id, const std::string & xsec_id )
{
    SetFeaPartWingSectionID( part_id, xsec_id, false, "SetFeaPartStartWingSectionID" );
}

std::string GetFeaPartStartWingSectionID( const std::string & part_id )
{
    return GetFeaPartWingSectionID( part_id, false, "GetFeaPartStartWingSectionID" );
}

void SetFeaPartEndWingSectionID( const std::string & part_id, const std::string & xsec_id )
{
    SetFeaPartWingSectionID( part_id, xsec_id, true, "SetFeaPartEndWingSectionID" );
}

std::string GetFeaPartEndWingSectionID( const std::string & part_id )
{
    return GetFeaPartWingSectionID( part_id, true, "GetFeaPartEndWingSectionID" );
}

/// Change the shape of a particular XSec
static void FitCSTAirfoilImpl( const std::string & xsec_surf_id, int xsec_index, int deg, const std::string & caller )
{
    XSecSurf* xsec_surf = FindXSecSurf( xsec_surf_id );
    if ( !xsec_surf )
    {
        ErrorMgr.AddError( VSP_INVALID_ID, caller + "::Can't Find XSecSurf " + xsec_surf_id );
        return;
    }
    if ( xsec_index < 0 || xsec_index >= xsec_surf->NumXSec() )
    {
        ErrorMgr.AddError( VSP_INDEX_OUT_RANGE, caller + "::XSec Index Out of Range " + xsec_surf_id + ":" + to_string( ( long long )xsec_index ) );
        return;
    }

    if ( deg < 1 )
    {
        ErrorMgr.AddError( VSP_INVALID_INPUT_VAL, caller + "::Fit Degree " + to_string( deg ) + " Must Be Positive" );
        return;
    }

    XSec* xs = xsec_surf->FindXSec( xsec_index );
    if ( !xs )
    {
        ErrorMgr.AddError( VSP_INVALID_PTR, caller + "::Can't Find XSec" );
        return;
    }

    Airfoil* af = dynamic_cast< Airfoil* >( xs->GetXSecCurve() );
    if ( !af )
    {
        ErrorMgr.AddError( VSP_WRONG_XSEC_TYPE, caller + "::XSec Is Not An Airfoil " + xsec_surf_id + ":" + to_string( ( long long )xsec_index ) );
        return;
    }

    // Grab the curve before the conversion; changing the shape replaces the XSecCurve.
    VspCurve c = af->GetOrigCurve();

    xsec_surf->ChangeXSecShape( xsec_index, XS_CST_AIRFOIL );

    XSec* newxs = xsec_surf->FindXSec( xsec_index );
    if ( !newxs )
    {
        ErrorMgr.AddError( VSP_INVALID_PTR, caller + "::Can't Find XSec" );
        return;
    }

    CSTAirfoil* cst_xs = dynamic_cast< CSTAirfoil* >( newxs->GetXSecCurve() );
    if ( !cst_xs )
    {
        ErrorMgr.AddError( VSP_WRONG_XSEC_TYPE, caller + "::XSec Did Not Convert To XS_CST_AIRFOIL" );
        return;
    }

    cst_xs->FitCurve( c, deg );
    cst_xs->Update();
    newxs->Update();

    ErrorMgr.NoError();
}

void FitCSTAirfoil( const std::string & xsec_surf_id, int xsec_index, int deg )
{
    FitCSTAirfoilImpl( xsec_surf_id, xsec_index, deg, "FitCSTAirfoil" );
}

void FitCSTAirfoilByID( const std::string & xsec_id, int deg )
{
    int index = -1;
    XSecSurf* xsec_surf = FindXSecSurfOf( xsec_id, index, "FitCSTAirfoilByID" );
    if ( !xsec_surf )
    {
        return;
    }

    FitCSTAirfoilImpl( xsec_surf->GetID(), index, deg, "FitCSTAirfoilByID" );
}

static void ChangeXSecShapeImpl( const std::string & xsec_surf_id, int xsec_index, int type, const std::string & caller )
{
    XSecSurf* xsec_surf = FindXSecSurf( xsec_surf_id );
    if ( !xsec_surf )
    {
        ErrorMgr.AddError( VSP_INVALID_ID, caller + "::Can't Find XSecSurf " + xsec_surf_id );
        return;
    }
    if ( xsec_index < 0 || xsec_index >= xsec_surf->NumXSec() )
    {
        ErrorMgr.AddError( VSP_INDEX_OUT_RANGE, caller + "::XSec Index Out of Range " + xsec_surf_id + ":" + to_string( ( long long )xsec_index ) );
        return;
    }
    if ( type < 0 || type >= vsp::XS_NUM_TYPES )
    {
        ErrorMgr.AddError( VSP_INVALID_TYPE, caller + "::Invalid XSec Type " + to_string( type ) );
        return;
    }

    ErrorMgr.NoError();
    xsec_surf->ChangeXSecShape( xsec_index, type );
}

void ChangeXSecShape( const std::string & xsec_surf_id, int xsec_index, int type )
{
    ChangeXSecShapeImpl( xsec_surf_id, xsec_index, type, "ChangeXSecShape" );
}

void ChangeXSecShapeByID( const std::string & xsec_id, int type )
{
    int index = -1;
    XSecSurf* xsec_surf = FindXSecSurfOf( xsec_id, index, "ChangeXSecShapeByID" );
    if ( !xsec_surf )
    {
        return;
    }

    ChangeXSecShapeImpl( xsec_surf->GetID(), index, type, "ChangeXSecShapeByID" );
}

/// Set The Global XForm Matrix For This XSec Surf
void SetXSecSurfGlobalXForm( const std::string & xsec_surf_id, const Matrix4d & mat )
{
    XSecSurf* xsec_surf = FindXSecSurf( xsec_surf_id );
    if ( !xsec_surf )
    {
        ErrorMgr.AddError( VSP_INVALID_ID, "SetXSecSurfGlobalXForm::Can't Find XSecSurf " + xsec_surf_id );
        return;
    }
    xsec_surf->SetGlobalXForm( mat );
    ErrorMgr.NoError();
}

/// Get The Global XForm Matrix For This XSec Surf
Matrix4d GetXSecSurfGlobalXForm( const std::string & xsec_surf_id )
{
    XSecSurf* xsec_surf = FindXSecSurf( xsec_surf_id );
    if ( !xsec_surf )
    {
        ErrorMgr.AddError( VSP_INVALID_ID, "GetXSecSurfGlobalXForm::Can't Find XSecSurf " + xsec_surf_id );
        return Matrix4d();
    }
    ErrorMgr.NoError();
    return xsec_surf->GetGlobalXForm();
}

//===================================================================//
//=================       XSec Functions         ====================//
//===================================================================//
/// Get XSec shape given xsec id
int GetXSecShape( const std::string& xsec_id )
{
    XSec* xs = FindXSec( xsec_id );
    if ( !xs )
    {
        ErrorMgr.AddError( VSP_INVALID_XSEC_ID, "GetXSecShape::Can't Find XSec " + xsec_id );
        return XS_UNDEFINED;
    }

    ErrorMgr.NoError();
    return xs->GetXSecCurve()->GetType();
}

/// Get XSec Width
double GetXSecWidth( const std::string& xsec_id )
{
    XSec* xs = FindXSec( xsec_id );
    if ( !xs )
    {
        ErrorMgr.AddError( VSP_INVALID_XSEC_ID, "GetXSecWidth::Can't Find XSec " + xsec_id );
        return 0;
    }
    ErrorMgr.NoError();
    return xs->GetXSecCurve()->GetWidth();
}

/// Get XSec Height
double GetXSecHeight( const std::string& xsec_id )
{
    XSec* xs = FindXSec( xsec_id );
    if ( !xs )
    {
        ErrorMgr.AddError( VSP_INVALID_XSEC_ID, "GetXSecHeight::Can't Find XSec " + xsec_id );
        return 0;
    }
    ErrorMgr.NoError();
    return xs->GetXSecCurve()->GetHeight();
}

/// Set XSec Width & Height
void SetXSecWidthHeight( const std::string& xsec_id, double w, double h )
{
    XSec* xs = FindXSec( xsec_id );
    if ( !xs )
    {
        ErrorMgr.AddError( VSP_INVALID_XSEC_ID, "SetXSecWidthHeight::Can't Find XSec " + xsec_id );
        return;
    }
    xs->GetXSecCurve()->SetWidthHeight( w, h );
    xs->ParmChanged( nullptr, Parm::SET_FROM_DEVICE ); // Force Update
    ErrorMgr.NoError();
}

void SetXSecWidth( const std::string& xsec_id, double w )
{
    XSec* xs = FindXSec( xsec_id );
    if ( !xs )
    {
        ErrorMgr.AddError( VSP_INVALID_XSEC_ID, "SetXSecWidth::Can't Find XSec " + xsec_id );
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

void SetXSecHeight( const std::string& xsec_id, double h )
{
    XSec* xs = FindXSec( xsec_id );
    if ( !xs )
    {
        ErrorMgr.AddError( VSP_INVALID_XSEC_ID, "SetXSecHeight::Can't Find XSec " + xsec_id );
        return;
    }
    Parm* height_parm = ParmMgr.FindParm( xs->GetXSecCurve()->GetHeightParmID() );
    if ( !height_parm )
    {
        ErrorMgr.AddError( VSP_INVALID_PTR, "SetXSecHeight::Can't Find Height Parm " + xs->GetXSecCurve()->GetHeightParmID() );
        return;
    }
    height_parm->SetFromDevice( h );
    ErrorMgr.NoError();
}

/// Get of the linkable parms ids for this geometry
std::vector< std::string > GetXSecParmIDs( const std::string & xsec_id )
{
    std::vector< std::string > parm_vec;

    XSec* xs = FindXSec( xsec_id );
    if ( !xs )
    {
        ErrorMgr.AddError( VSP_INVALID_XSEC_ID, "GetXSecParmIDs::Can't Find XSec " + xsec_id );
        return parm_vec;
    }

    xs->AddLinkableParms( parm_vec );

    ErrorMgr.NoError();
    return parm_vec;
}


/// Get named ParmID from XSec
std::string GetXSecParm( const std::string& xsec_id, const std::string& name )
{
    XSec* xs = FindXSec( xsec_id );
    if ( !xs )
    {
        ErrorMgr.AddError( VSP_INVALID_XSEC_ID, "GetXSecParm::Can't Find XSec " + xsec_id );
        return std::string();
    }

    //==== Valid XSec Parm - Return Parm ID ====//
    std::string xsparm = xs->FindParm( name );
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
        return std::string();
    }

    std::string xscparm = xsc->FindParm( name );
    if ( ValidParm( xscparm ) )
    {
        ErrorMgr.NoError();
        return xscparm;
    }

    ErrorMgr.AddError( VSP_CANT_FIND_PARM, "GetXSecParm::Can't Find Parm " + name );
    return std::string();
}

//==== Read XSec From File ====//
std::vector<vec3d> ReadFileXSec( const std::string& xsec_id, const std::string& file_name )
{
    std::vector< vec3d > pnt_vec;
    XSec* xs = FindXSec( xsec_id );
    if ( !xs )
    {
        ErrorMgr.AddError( VSP_INVALID_XSEC_ID, "ReadFileXSec::Can't Find XSec " + xsec_id );
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
            ErrorMgr.AddError( VSP_FILE_READ_FAILURE, "ReadFileXSec::Error reading fuselage file " + file_name );
            return pnt_vec;
        }
    }

    ErrorMgr.AddError( VSP_WRONG_XSEC_TYPE, "ReadFileXSec::XSec Not XS_FILE_FUSE Type " + xsec_id );
    return pnt_vec;
}

//==== Set XSec Pnts ====//
std::vector< vec3d > GetXSecPnts( const std::string& xsec_id )
{
    std::vector< vec3d > pnt_vec;

    XSec* xs = FindXSec( xsec_id );
    if ( !xs )
    {
        ErrorMgr.AddError( VSP_INVALID_XSEC_ID, "GetXSecPnts::Can't Find XSec " + xsec_id );
        return pnt_vec;
    }
    if ( xs->GetXSecCurve()->GetType() != XS_FILE_FUSE )
    {
        ErrorMgr.AddError( VSP_WRONG_XSEC_TYPE, "GetXSecPnts::Wrong XSec Type " + xsec_id );
        return pnt_vec;
    }

    FileXSec* file_xs = dynamic_cast<FileXSec*>( xs->GetXSecCurve() );
    assert( file_xs );
    pnt_vec = file_xs->GetUnityFilePnts();
    ErrorMgr.NoError();
    return pnt_vec;
}

void SetXSecPnts( const std::string& xsec_id, std::vector< vec3d > & pnt_vec )
{
    XSec* xs = FindXSec( xsec_id );
    if ( !xs )
    {
        ErrorMgr.AddError( VSP_INVALID_XSEC_ID, "SetXSecPnts::Can't Find XSec " + xsec_id );
        return;
    }
    if ( xs->GetXSecCurve()->GetType() != XS_FILE_FUSE )
    {
        ErrorMgr.AddError( VSP_WRONG_XSEC_TYPE, "SetXSecPnts::Wrong XSec Type " + xsec_id );
        return;
    }

    FileXSec* file_xs = dynamic_cast<FileXSec*>( xs->GetXSecCurve() );
    assert( file_xs );
    if ( !file_xs->SetPnts( pnt_vec ) )
    {
        ErrorMgr.AddError( VSP_INVALID_INPUT_VAL, "SetXSecPnts::The points must form a closed section in the XY plane, running from the right-hand point down through the bottom, round the side and over the top" );
        return;
    }
    ErrorMgr.NoError();
}

//==== Compute Point Along XSec ====//
vec3d ComputeXSecPnt( const std::string& xsec_id, double fract )
{
    XSec* xs = FindXSec( xsec_id );
    if ( !xs )
    {
        ErrorMgr.AddError( VSP_INVALID_XSEC_ID, "ComputeXSecPnt::Can't Find XSec " + xsec_id );
        return vec3d();
    }

    vec3d pnt = xs->GetCurve().CompPnt01( fract );
    ErrorMgr.NoError();

    return pnt;
}

//==== Compute Tan Along XSec ====//
vec3d ComputeXSecTan( const std::string& xsec_id, double fract )
{
    XSec* xs = FindXSec( xsec_id );
    if ( !xs )
    {
        ErrorMgr.AddError( VSP_INVALID_XSEC_ID, "ComputeXSecTan::Can't Find XSec " + xsec_id );
        return vec3d();
    }

    vec3d pnt = xs->GetCurve().CompTan01( fract );
    ErrorMgr.NoError();

    return pnt;
}

//==== Reset All XSec Skinning Parms ====//
void ResetXSecSkinParms( const std::string& xsec_id )
{
    XSec* xs = FindXSec( xsec_id );
    if ( !xs )
    {
        ErrorMgr.AddError( VSP_INVALID_XSEC_ID, "ResetXSecSkinParms::Can't Find XSec " + xsec_id );
        return;
    }
    SkinXSec* skinxs = dynamic_cast<SkinXSec*>(xs);
    if ( !skinxs )
    {
        ErrorMgr.AddError( VSP_WRONG_GEOM_TYPE, "ResetXSecSkinParms::Can't Convert To Skin XSec " + xsec_id );
        return;
    }

    skinxs->Reset();
    ErrorMgr.NoError();
}

//==== Set Continuity At XSec ====//
int GetXSecContinuity( const std::string& xsec_id )
{
    XSec* xs = FindXSec( xsec_id );
    if ( !xs )
    {
        ErrorMgr.AddError( VSP_INVALID_XSEC_ID, "GetXSecContinuity::Can't Find XSec " + xsec_id );
        return -1;
    }
    SkinXSec* skinxs = dynamic_cast<SkinXSec*>(xs);
    if ( !skinxs )
    {
        ErrorMgr.AddError( VSP_WRONG_GEOM_TYPE, "GetXSecContinuity::Can't Convert To Skin XSec " + xsec_id );
        return -1;
    }

    ErrorMgr.NoError();
    return skinxs->GetContinuity();
}

// Whether this Geom's cross sections are the kind that carry skinning.
//
// GeomXSec alone does not say so: Wing and Prop are GeomXSec, but their cross sections are
// not SkinXSec, so every spine call on them found nothing to do and said nothing about it.
// The spine functions returned -1, which is also what they return for a Geom ID that does
// not exist, so a caller could not tell the two apart by the return value or by the error
// queue.
static bool SkinnableGeom( GeomXSec* gxs )
{
    if ( !gxs )
    {
        return false;
    }

    return dynamic_cast < SkinXSec* > ( gxs->GetXSec( 0 ) ) != nullptr;
}

int AddSkinSpine( const string& geom_id, double w )
{
    Vehicle* veh = GetVehicle();
    Geom* geom_ptr = veh->FindGeom( geom_id );
    if ( !geom_ptr )
    {
        ErrorMgr.AddError( VSP_INVALID_GEOM_ID, "AddSkinSpine::Can't Find Geom " + geom_id );
        return -1;
    }

    GeomXSec* gxs = dynamic_cast < GeomXSec* > ( geom_ptr );
    if ( !SkinnableGeom( gxs ) )
    {
        ErrorMgr.AddError( VSP_WRONG_GEOM_TYPE, "AddSkinSpine::Geom does not support skinning " + geom_id );
        return -1;
    }

    // Written as a positive test so a NaN is refused rather than waved through: every
    // comparison against a NaN is false, so "w < 0 || w > 1" lets one past, and it then
    // reaches the station sort, whose comparator a NaN makes inconsistent.
    if ( !( w >= 0.0 && w <= 1.0 ) )
    {
        ErrorMgr.AddError( VSP_INVALID_INPUT_VAL, "AddSkinSpine::Position " + to_string( w ) + " is not on [0, 1]" );
        return -1;
    }

    // And not on a station that is already there.  Two stations at the same parameter are
    // one station, so the newcomer is merged away by GetStations and does nothing -- while
    // GetNumSkinSpines counts it, the Skinning tab lists it, and its sliders move nothing.
    // 0 and 1 are both the fixed Right station on a periodic section, which is what a
    // mistyped position most often lands on.
    if ( !gxs->SkinSpineW01IsClear( w ) )
    {
        ErrorMgr.AddError( VSP_INVALID_INPUT_VAL,
                           "AddSkinSpine::Position " + to_string( w ) + " is already taken by a station" );
        return -1;
    }

    int index = gxs->AddSkinSpine( w );
    geom_ptr->Update();

    ErrorMgr.NoError();
    return index;
}

//==== The Geom a skin spine is on and the spine's index there, reporting through the
//==== named caller when it cannot be found. ====//
Geom* FindSkinSpineGeom( const std::string & spine_id, int & index, const std::string & caller )
{
    SkinSpine* sp = dynamic_cast < SkinSpine* > ( ParmMgr.FindParmContainer( spine_id ) );
    SkinXSec* sxs = nullptr;
    if ( sp )
    {
        sxs = dynamic_cast < SkinXSec* > ( sp->GetParentContainerPtr() );
    }

    if ( !sxs )
    {
        ErrorMgr.AddError( VSP_INVALID_ID, caller + "::Can't Find Spine " + spine_id );
        return nullptr;
    }

    index = -1;
    for ( int i = 0; i < sxs->NumSpines(); i++ )
    {
        if ( sxs->GetSpine( i ) == sp )
        {
            index = i;
        }
    }

    int xsec_index = -1;
    return FindXSecGeom( sxs->GetID(), xsec_index, caller );
}

static void DelSkinSpineImpl( const string& geom_id, int index, const std::string & caller )
{
    Vehicle* veh = GetVehicle();
    Geom* geom_ptr = veh->FindGeom( geom_id );
    if ( !geom_ptr )
    {
        ErrorMgr.AddError( VSP_INVALID_GEOM_ID, caller + "::Can't Find Geom " + geom_id );
        return;
    }

    GeomXSec* gxs = dynamic_cast < GeomXSec* > ( geom_ptr );
    if ( !SkinnableGeom( gxs ) )
    {
        ErrorMgr.AddError( VSP_WRONG_GEOM_TYPE, caller + "::Geom does not support skinning " + geom_id );
        return;
    }

    // GetSkinSpineID raises for exactly this; deleting should fail the same way rather than
    // return quietly having done nothing.
    if ( index < 0 || index >= gxs->NumSkinSpines() )
    {
        ErrorMgr.AddError( VSP_INDEX_OUT_RANGE, caller + "::Index " + to_string( index ) + " is out of range" );
        return;
    }

    gxs->DelSkinSpine( index );
    geom_ptr->Update();

    ErrorMgr.NoError();
}

void DelSkinSpine( const string& geom_id, int index )
{
    DelSkinSpineImpl( geom_id, index, "DelSkinSpine" );
}

void DelSkinSpineByID( const std::string & spine_id )
{
    int index = -1;
    Geom* geom_ptr = FindSkinSpineGeom( spine_id, index, "DelSkinSpineByID" );
    if ( !geom_ptr )
    {
        return;
    }

    DelSkinSpineImpl( geom_ptr->GetID(), index, "DelSkinSpineByID" );
}

void DelAllSkinSpines( const string& geom_id )
{
    Vehicle* veh = GetVehicle();
    Geom* geom_ptr = veh->FindGeom( geom_id );
    if ( !geom_ptr )
    {
        ErrorMgr.AddError( VSP_INVALID_GEOM_ID, "DelAllSkinSpines::Can't Find Geom " + geom_id );
        return;
    }

    GeomXSec* gxs = dynamic_cast < GeomXSec* > ( geom_ptr );
    if ( !SkinnableGeom( gxs ) )
    {
        ErrorMgr.AddError( VSP_WRONG_GEOM_TYPE, "DelAllSkinSpines::Geom does not support skinning " + geom_id );
        return;
    }

    gxs->DelAllSkinSpines();
    geom_ptr->Update();

    ErrorMgr.NoError();
}

static string GetSkinSpineNameImpl( const string& geom_id, int index, const std::string & caller )
{
    Vehicle* veh = GetVehicle();
    Geom* geom_ptr = veh->FindGeom( geom_id );
    if ( !geom_ptr )
    {
        ErrorMgr.AddError( VSP_INVALID_GEOM_ID, caller + "::Can't Find Geom " + geom_id );
        return string();
    }

    GeomXSec* gxs = dynamic_cast < GeomXSec* > ( geom_ptr );
    if ( !SkinnableGeom( gxs ) )
    {
        ErrorMgr.AddError( VSP_WRONG_GEOM_TYPE, caller + "::Geom does not support skinning " + geom_id );
        return string();
    }

    if ( index < 0 || index >= gxs->NumSkinSpines() )
    {
        ErrorMgr.AddError( VSP_INDEX_OUT_RANGE, caller + "::Index " + to_string( index ) + " is out of range" );
        return string();
    }

    ErrorMgr.NoError();
    return gxs->GetSkinSpineName( index );
}

string GetSkinSpineName( const string& geom_id, int index )
{
    return GetSkinSpineNameImpl( geom_id, index, "GetSkinSpineName" );
}

std::string GetSkinSpineNameByID( const std::string & spine_id )
{
    int index = -1;
    Geom* geom_ptr = FindSkinSpineGeom( spine_id, index, "GetSkinSpineNameByID" );
    if ( !geom_ptr )
    {
        return std::string();
    }

    return GetSkinSpineNameImpl( geom_ptr->GetID(), index, "GetSkinSpineNameByID" );
}

static void SetSkinSpineNameImpl( const string& geom_id, int index, const string& name, const std::string & caller )
{
    Vehicle* veh = GetVehicle();
    Geom* geom_ptr = veh->FindGeom( geom_id );
    if ( !geom_ptr )
    {
        ErrorMgr.AddError( VSP_INVALID_GEOM_ID, caller + "::Can't Find Geom " + geom_id );
        return;
    }

    GeomXSec* gxs = dynamic_cast < GeomXSec* > ( geom_ptr );
    if ( !SkinnableGeom( gxs ) )
    {
        ErrorMgr.AddError( VSP_WRONG_GEOM_TYPE, caller + "::Geom does not support skinning " + geom_id );
        return;
    }

    if ( index < 0 || index >= gxs->NumSkinSpines() )
    {
        ErrorMgr.AddError( VSP_INDEX_OUT_RANGE, caller + "::Index " + to_string( index ) + " is out of range" );
        return;
    }

    gxs->SetSkinSpineName( index, name );
    geom_ptr->Update();

    ErrorMgr.NoError();
}

void SetSkinSpineName( const string& geom_id, int index, const string& name )
{
    SetSkinSpineNameImpl( geom_id, index, name, "SetSkinSpineName" );
}

void SetSkinSpineNameByID( const std::string & spine_id, const std::string & name )
{
    int index = -1;
    Geom* geom_ptr = FindSkinSpineGeom( spine_id, index, "SetSkinSpineNameByID" );
    if ( !geom_ptr )
    {
        return;
    }

    SetSkinSpineNameImpl( geom_ptr->GetID(), index, name, "SetSkinSpineNameByID" );
}

int GetNumSkinSpines( const string& geom_id )
{
    Vehicle* veh = GetVehicle();
    Geom* geom_ptr = veh->FindGeom( geom_id );
    if ( !geom_ptr )
    {
        ErrorMgr.AddError( VSP_INVALID_GEOM_ID, "GetNumSkinSpines::Can't Find Geom " + geom_id );
        return 0;
    }

    GeomXSec* gxs = dynamic_cast < GeomXSec* > ( geom_ptr );
    if ( !SkinnableGeom( gxs ) )
    {
        ErrorMgr.AddError( VSP_WRONG_GEOM_TYPE, "GetNumSkinSpines::Geom does not support skinning " + geom_id );
        return 0;
    }

    ErrorMgr.NoError();
    return gxs->NumSkinSpines();
}

string GetSkinSpineParm( const string& spine_id, const string& name )
{
    ParmContainer* pc = ParmMgr.FindParmContainer( spine_id );

    SkinSpine* sp = dynamic_cast < SkinSpine* > ( pc );
    if ( !sp )
    {
        ErrorMgr.AddError( VSP_INVALID_ID, "GetSkinSpineParm::Can't Find Spine " + spine_id );
        return string();
    }

    string pid = sp->FindParm( name );
    if ( ValidParm( pid ) )
    {
        ErrorMgr.NoError();
        return pid;
    }

    ErrorMgr.AddError( VSP_CANT_FIND_PARM, "GetSkinSpineParm::Can't Find Parm " + name );
    return string();
}

string GetSkinSpineID( const string& xsec_id, int index )
{
    XSec* xs = FindXSec( xsec_id );
    if ( !xs )
    {
        ErrorMgr.AddError( VSP_INVALID_XSEC_ID, "GetSkinSpineID::Can't Find XSec " + xsec_id );
        return string();
    }

    SkinXSec* sxs = dynamic_cast < SkinXSec* > ( xs );
    if ( !sxs )
    {
        ErrorMgr.AddError( VSP_WRONG_GEOM_TYPE, "GetSkinSpineID::Can't Convert To Skin XSec " + xsec_id );
        return string();
    }

    SkinSpine* sp = sxs->GetSpine( index );
    if ( !sp )
    {
        ErrorMgr.AddError( VSP_INDEX_OUT_RANGE, "GetSkinSpineID::Spine Index " + to_string( index ) + " Out of Range" );
        return string();
    }

    ErrorMgr.NoError();
    return sp->GetID();
}

std::vector < std::string > GetSkinSpineIDVec( const std::string & xsec_id )
{
    std::vector < std::string > ret_vec;

    XSec* xs = FindXSec( xsec_id );
    if ( !xs )
    {
        ErrorMgr.AddError( VSP_INVALID_XSEC_ID, "GetSkinSpineIDVec::Can't Find XSec " + xsec_id );
        return ret_vec;
    }

    SkinXSec* sxs = dynamic_cast < SkinXSec* > ( xs );
    if ( !sxs )
    {
        ErrorMgr.AddError( VSP_WRONG_GEOM_TYPE, "GetSkinSpineIDVec::XSec " + xsec_id + " is on a Geom without skinning" );
        return ret_vec;
    }

    for ( int i = 0; i < sxs->NumSpines(); i++ )
    {
        ret_vec.push_back( sxs->GetSpine( i )->GetID() );
    }

    ErrorMgr.NoError();
    return ret_vec;
}

void SetXSecContinuity( const std::string& xsec_id, int cx )
{
    XSec* xs = FindXSec( xsec_id );
    if ( !xs )
    {
        ErrorMgr.AddError( VSP_INVALID_XSEC_ID, "SetXSecContinuity::Can't Find XSec " + xsec_id );
        return;
    }
    SkinXSec* skinxs = dynamic_cast<SkinXSec*>(xs);
    if ( !skinxs )
    {
        ErrorMgr.AddError( VSP_WRONG_GEOM_TYPE, "SetXSecContinuity::Can't Convert To Skin XSec " + xsec_id );
        return;
    }

    skinxs->SetContinuity( cx );
    ErrorMgr.NoError();
}

//==== Set Tan Angles At XSec ====//
std::vector < double > GetXSecTanAngles( const std::string& xsec_id, int side )
{
    std::vector < double > ret;

    XSec* xs = FindXSec( xsec_id );
    if ( !xs )
    {
        ErrorMgr.AddError( VSP_INVALID_XSEC_ID, "GetXSecTanAngles::Can't Find XSec " + xsec_id );
        return ret;
    }
    SkinXSec* skinxs = dynamic_cast<SkinXSec*>(xs);
    if ( !skinxs )
    {
        ErrorMgr.AddError( VSP_WRONG_GEOM_TYPE, "GetXSecTanAngles::Can't Convert To Skin XSec " + xsec_id );
        return ret;
    }

    // The two sides can hold different values, so XSEC_BOTH_SIDES names no
    // single answer.
    if ( side != XSEC_LEFT_SIDE && side != XSEC_RIGHT_SIDE )
    {
        ErrorMgr.AddError( VSP_INVALID_TYPE, "GetXSecTanAngles::Side Must Be XSEC_LEFT_SIDE Or XSEC_RIGHT_SIDE, Not " + to_string( side ) );
        return ret;
    }

    ret = skinxs->GetTanAngles( side );
    ErrorMgr.NoError();
    return ret;
}

std::vector < double > GetXSecTanSlews( const std::string& xsec_id, int side )
{
    std::vector < double > ret;

    XSec* xs = FindXSec( xsec_id );
    if ( !xs )
    {
        ErrorMgr.AddError( VSP_INVALID_XSEC_ID, "GetXSecTanSlews::Can't Find XSec " + xsec_id );
        return ret;
    }
    SkinXSec* skinxs = dynamic_cast<SkinXSec*>(xs);
    if ( !skinxs )
    {
        ErrorMgr.AddError( VSP_WRONG_GEOM_TYPE, "GetXSecTanSlews::Can't Convert To Skin XSec " + xsec_id );
        return ret;
    }

    // The two sides can hold different values, so XSEC_BOTH_SIDES names no
    // single answer.
    if ( side != XSEC_LEFT_SIDE && side != XSEC_RIGHT_SIDE )
    {
        ErrorMgr.AddError( VSP_INVALID_TYPE, "GetXSecTanSlews::Side Must Be XSEC_LEFT_SIDE Or XSEC_RIGHT_SIDE, Not " + to_string( side ) );
        return ret;
    }

    ret = skinxs->GetTanSlews( side );
    ErrorMgr.NoError();
    return ret;
}

std::vector < double > GetXSecTanStrengths( const std::string& xsec_id, int side )
{
    std::vector < double > ret;

    XSec* xs = FindXSec( xsec_id );
    if ( !xs )
    {
        ErrorMgr.AddError( VSP_INVALID_XSEC_ID, "GetXSecTanStrengths::Can't Find XSec " + xsec_id );
        return ret;
    }
    SkinXSec* skinxs = dynamic_cast<SkinXSec*>(xs);
    if ( !skinxs )
    {
        ErrorMgr.AddError( VSP_WRONG_GEOM_TYPE, "GetXSecTanStrengths::Can't Convert To Skin XSec " + xsec_id );
        return ret;
    }

    // The two sides can hold different values, so XSEC_BOTH_SIDES names no
    // single answer.
    if ( side != XSEC_LEFT_SIDE && side != XSEC_RIGHT_SIDE )
    {
        ErrorMgr.AddError( VSP_INVALID_TYPE, "GetXSecTanStrengths::Side Must Be XSEC_LEFT_SIDE Or XSEC_RIGHT_SIDE, Not " + to_string( side ) );
        return ret;
    }

    ret = skinxs->GetTanStrengths( side );
    ErrorMgr.NoError();
    return ret;
}

std::vector < double > GetXSecCurvatures( const std::string& xsec_id, int side )
{
    std::vector < double > ret;

    XSec* xs = FindXSec( xsec_id );
    if ( !xs )
    {
        ErrorMgr.AddError( VSP_INVALID_XSEC_ID, "GetXSecCurvatures::Can't Find XSec " + xsec_id );
        return ret;
    }
    SkinXSec* skinxs = dynamic_cast<SkinXSec*>(xs);
    if ( !skinxs )
    {
        ErrorMgr.AddError( VSP_WRONG_GEOM_TYPE, "GetXSecCurvatures::Can't Convert To Skin XSec " + xsec_id );
        return ret;
    }

    // The two sides can hold different values, so XSEC_BOTH_SIDES names no
    // single answer.
    if ( side != XSEC_LEFT_SIDE && side != XSEC_RIGHT_SIDE )
    {
        ErrorMgr.AddError( VSP_INVALID_TYPE, "GetXSecCurvatures::Side Must Be XSEC_LEFT_SIDE Or XSEC_RIGHT_SIDE, Not " + to_string( side ) );
        return ret;
    }

    ret = skinxs->GetCurvatures( side );
    ErrorMgr.NoError();
    return ret;
}

void SetXSecTanAngles( const std::string& xsec_id, int side, double top, double right, double bottom, double left )
{
    XSec* xs = FindXSec( xsec_id );
    if ( !xs )
    {
        ErrorMgr.AddError( VSP_INVALID_XSEC_ID, "SetXSecTanAngles::Can't Find XSec " + xsec_id );
        return;
    }
    if ( side != XSEC_BOTH_SIDES && side != XSEC_LEFT_SIDE && side != XSEC_RIGHT_SIDE )
    {
        ErrorMgr.AddError( VSP_INVALID_TYPE, "SetXSecTanAngles::Side Must Be XSEC_BOTH_SIDES, XSEC_LEFT_SIDE Or XSEC_RIGHT_SIDE, Not " + to_string( side ) );
        return;
    }
    SkinXSec* skinxs = dynamic_cast<SkinXSec*>(xs);
    if ( !skinxs )
    {
        ErrorMgr.AddError( VSP_WRONG_GEOM_TYPE, "SetXSecTanAngles::Can't Convert To Skin XSec " + xsec_id );
        return;
    }

    skinxs->SetTanAngles( side, top, right, bottom, left );
    ErrorMgr.NoError();
}

//==== Set Tan Slews At XSec ====//
void SetXSecTanSlews( const std::string& xsec_id, int side, double top, double right, double bottom, double left )
{
    XSec* xs = FindXSec( xsec_id );
    if ( !xs )
    {
        ErrorMgr.AddError( VSP_INVALID_XSEC_ID, "SetXSecTanSlews::Can't Find XSec " + xsec_id );
        return;
    }
    if ( side != XSEC_BOTH_SIDES && side != XSEC_LEFT_SIDE && side != XSEC_RIGHT_SIDE )
    {
        ErrorMgr.AddError( VSP_INVALID_TYPE, "SetXSecTanSlews::Side Must Be XSEC_BOTH_SIDES, XSEC_LEFT_SIDE Or XSEC_RIGHT_SIDE, Not " + to_string( side ) );
        return;
    }
    SkinXSec* skinxs = dynamic_cast<SkinXSec*>(xs);
    if ( !skinxs )
    {
        ErrorMgr.AddError( VSP_WRONG_GEOM_TYPE, "SetXSecTanSlews::Can't Convert To Skin XSec " + xsec_id );
        return;
    }

    skinxs->SetTanSlews( side, top, right, bottom, left );
    ErrorMgr.NoError();
}

//==== Set Tan Strengths At XSec ====//
void SetXSecTanStrengths( const std::string& xsec_id, int side, double top, double right, double bottom, double left )
{
    XSec* xs = FindXSec( xsec_id );
    if ( !xs )
    {
        ErrorMgr.AddError( VSP_INVALID_XSEC_ID, "SetXSecTanStrengths::Can't Find XSec " + xsec_id );
        return;
    }
    if ( side != XSEC_BOTH_SIDES && side != XSEC_LEFT_SIDE && side != XSEC_RIGHT_SIDE )
    {
        ErrorMgr.AddError( VSP_INVALID_TYPE, "SetXSecTanStrengths::Side Must Be XSEC_BOTH_SIDES, XSEC_LEFT_SIDE Or XSEC_RIGHT_SIDE, Not " + to_string( side ) );
        return;
    }
    SkinXSec* skinxs = dynamic_cast<SkinXSec*>(xs);
    if ( !skinxs )
    {
        ErrorMgr.AddError( VSP_WRONG_GEOM_TYPE, "SetXSecTanStrengths::Can't Convert To Skin XSec " + xsec_id );
        return;
    }

    skinxs->SetTanStrengths( side, top, right, bottom, left );
    ErrorMgr.NoError();
}

//==== Set Curvature Angles At XSec ====//
void SetXSecCurvatures( const std::string& xsec_id, int side, double top, double right, double bottom, double left )
{
    XSec* xs = FindXSec( xsec_id );
    if ( !xs )
    {
        ErrorMgr.AddError( VSP_INVALID_XSEC_ID, "SetXSecCurvatures::Can't Find XSec " + xsec_id );
        return;
    }
    if ( side != XSEC_BOTH_SIDES && side != XSEC_LEFT_SIDE && side != XSEC_RIGHT_SIDE )
    {
        ErrorMgr.AddError( VSP_INVALID_TYPE, "SetXSecCurvatures::Side Must Be XSEC_BOTH_SIDES, XSEC_LEFT_SIDE Or XSEC_RIGHT_SIDE, Not " + to_string( side ) );
        return;
    }
    SkinXSec* skinxs = dynamic_cast<SkinXSec*>(xs);
    if ( !skinxs )
    {
        ErrorMgr.AddError( VSP_WRONG_GEOM_TYPE, "SetXSecCurvatures::Can't Convert To Skin XSec " + xsec_id );
        return;
    }

    skinxs->SetCurvatures( side, top, right, bottom, left );
    ErrorMgr.NoError();
}

void ReadFileAirfoil( const std::string& xsec_id, const std::string& file_name )
{
    XSec* xs = FindXSec( xsec_id );
    if ( !xs )
    {
        ErrorMgr.AddError( VSP_INVALID_XSEC_ID, "ReadFileAirfoil::Can't Find XSec " + xsec_id );
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
            ErrorMgr.AddError( VSP_FILE_READ_FAILURE, "ReadFileAirfoil::Error reading airfoil file " + file_name );
            return;
        }
    }

    ErrorMgr.AddError( VSP_WRONG_XSEC_TYPE, "ReadFileAirfoil::XSec Not XS_FILE_AIRFOIL Type " + xsec_id );
    return;
}

void SetAirfoilUpperPnts( const std::string& xsec_id, const std::vector< vec3d > & up_pnt_vec )
{
    XSec* xs = FindXSec( xsec_id );
    if ( !xs )
    {
        ErrorMgr.AddError( VSP_INVALID_XSEC_ID, "SetAirfoilUpperPnts::Can't Find XSec " + xsec_id );
        return;
    }

    if ( xs->GetXSecCurve()->GetType() != XS_FILE_AIRFOIL )
    {
        ErrorMgr.AddError( VSP_WRONG_XSEC_TYPE, "SetAirfoilUpperPnts::XSec Not XS_FILE_AIRFOIL Type " + xsec_id );
        return;
    }

    FileAirfoil* file_xs = dynamic_cast<FileAirfoil*>( xs->GetXSecCurve() );
    assert( file_xs );
    file_xs->SetAirfoilUpperPnts( up_pnt_vec );
    ErrorMgr.NoError();
}

void SetAirfoilLowerPnts( const std::string& xsec_id, const std::vector< vec3d > & low_pnt_vec )
{
    XSec* xs = FindXSec( xsec_id );
    if ( !xs )
    {
        ErrorMgr.AddError( VSP_INVALID_XSEC_ID, "SetAirfoilLowerPnts::Can't Find XSec " + xsec_id );
        return;
    }

    if ( xs->GetXSecCurve()->GetType() != XS_FILE_AIRFOIL )
    {
        ErrorMgr.AddError( VSP_WRONG_XSEC_TYPE, "SetAirfoilLowerPnts::XSec Not XS_FILE_AIRFOIL Type " + xsec_id );
        return;
    }

    FileAirfoil* file_xs = dynamic_cast<FileAirfoil*>( xs->GetXSecCurve() );
    assert( file_xs );
    file_xs->SetAirfoilLowerPnts( low_pnt_vec );
    ErrorMgr.NoError();
}

void SetAirfoilPnts( const std::string& xsec_id, const std::vector< vec3d > & up_pnt_vec, const std::vector< vec3d > & low_pnt_vec )
{
    XSec* xs = FindXSec( xsec_id );
    if ( !xs )
    {
        ErrorMgr.AddError( VSP_INVALID_XSEC_ID, "SetAirfoilPnts::Can't Find XSec " + xsec_id );
        return;
    }

    if ( xs->GetXSecCurve()->GetType() != XS_FILE_AIRFOIL )
    {
        ErrorMgr.AddError( VSP_WRONG_XSEC_TYPE, "SetAirfoilPnts::XSec Not XS_FILE_AIRFOIL Type " + xsec_id );
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
    std::string file_name = airfoil_name + ".dat";
    FILE* af = fopen( file_name.c_str(), "w" );
    if ( !af )
    {
        ErrorMgr.AddError( VSP_FILE_WRITE_FAILURE, "WriteSeligAirfoilFile::Error writing airfoil file " + file_name );
        return;
    }

    std::string header = airfoil_name + " AIRFOIL\n";
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
    std::vector < long double > y_span_vec; // y position across half span
    std::vector < long double > gamma_vec; // circulation
    std::vector < long double > w_vec; // downwash velocity
    std::vector < long double > cl_vec; // lift coefficient
    std::vector < long double > cd_vec; // induced drag coefficient
};

LLT_Data GetHersheyLLTData( const unsigned int &npts, const long double &alpha, const long double &Vinf, const long double &span )
{
    LLT_Data llt_data;

    const long double alpha0 = 0.0; // zero lift angle of attack (rad)
    const long double c = 1.0; // root/tip chord

    std::vector < long double > theta_vec, r_vec, a_vec;
    std::vector < int > odd_vec;
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

    std::vector < vec3d > y_cl_vec;
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


    ErrorMgr.NoError();
    return y_cl_vec;
}

std::vector<vec3d> GetHersheyBarDragDist( const int &npts, const double &alpha, const double &Vinf, const double &span, bool full_span_flag )
{
    // Calculation of drag distribution for a Hershey Bar wing with unit chord length using Glauert's Method.
    //  Input span is the entire wing span, which half is used in the following calculations. If full_span_flag == true,
    //  symmetry is applied to the results. Input alpha must be in radians.

    LLT_Data llt_data = GetHersheyLLTData( npts, alpha, Vinf, span );

    std::vector < vec3d > y_cd_vec;
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

    ErrorMgr.NoError();
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

    std::vector < vec3d > xyzdata;
    xyzdata.resize( npts );

    double a = ell * sqrt( ( 1.0 + epsilon ) * ( 1.0 + epsilon ) + kappa * kappa ); // Radius of circle
    double beta = asin( ell * kappa / a ); // Angle of TE location (rad)
    double n = 2.0 - tau / M_PI;
    doublec mu = doublec( -ell * epsilon, ell * kappa ); // Center of circle

    if ( ( ell * kappa / a ) > 1.0 )
    {
        ErrorMgr.AddError( VSP_INVALID_INPUT_VAL, "GetVKTAirfoilPnts::Camber parameter, kappa, " + to_string( kappa ) + " is too large" );
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

    ErrorMgr.NoError();
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

    std::vector < double > cpdata;
    cpdata.resize( npts );

    double a = ell * sqrt( ( 1.0 + epsilon ) * ( 1.0 + epsilon ) + kappa * kappa ); // Radius of circle
    double beta = asin( ell * kappa / a ); // Angle of TE location (rad)
    double n = 2.0 - tau / M_PI;
    doublec mu = doublec( -ell * epsilon, ell * kappa ); // Center of circle

    if ( ( ell * kappa / a ) > 1.0 )
    {
        ErrorMgr.AddError( VSP_INVALID_INPUT_VAL, "GetVKTAirfoilCpDist::Camber parameter, kappa, " + to_string( kappa ) + " is too large" );
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

    ErrorMgr.NoError();
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

    std::vector < vec3d > surf_pnt_vec;

    std::vector < double > theta_vec, phi_vec;
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

    ErrorMgr.NoError();
    return surf_pnt_vec;
}

std::vector<vec3d> GetFeatureLinePnts( const std::string& geom_id )
{
    std::vector < vec3d > pnt_vec;

    Vehicle* veh = GetVehicle();
    Geom* geom_ptr = veh->FindGeom( geom_id );
    if ( !geom_ptr )
    {
        ErrorMgr.AddError( VSP_INVALID_GEOM_ID, "GetFeatureLinePnts::Can't Find Geom " + geom_id );
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
            std::vector < vec3d > ptline;
            surf->TessUFeatureLine( j, ptline, tol );

            for ( size_t k = 0; k < ptline.size(); k++ )
            {
                pnt_vec.push_back( ptline[k] );
            }
        }

        // V feature lines
        for ( int j = 0; j < surf->GetNumWFeature(); j++ )
        {
            std::vector < vec3d > ptline;
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

    std::vector < vec3d > pot_vec, uvw_vec;
    std::vector < double > cp_vec;
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

    ErrorMgr.NoError();
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

    ErrorMgr.NoError();
    return quad( fun, 0.0, 1.0e8 ); // Integrate from 0 to inf (Note: an upper limit greater than 1.0e8 will produce errors)
}

std::vector<vec3d> GetAirfoilUpperPnts( const std::string& xsec_id )
{
    std::vector< vec3d > pnt_vec;

    XSec* xs = FindXSec( xsec_id );
    if ( !xs )
    {
        ErrorMgr.AddError( VSP_INVALID_XSEC_ID, "GetAirfoilUpperPnts::Can't Find XSec " + xsec_id );
        return pnt_vec;
    }

    if ( xs->GetXSecCurve()->GetType() != XS_FILE_AIRFOIL )
    {
        ErrorMgr.AddError( VSP_WRONG_XSEC_TYPE, "GetAirfoilUpperPnts::XSec Not XS_FILE_AIRFOIL Type " + xsec_id );
        return pnt_vec;
    }

    FileAirfoil* file_xs = dynamic_cast<FileAirfoil*>( xs->GetXSecCurve() );
    assert( file_xs );
    pnt_vec = file_xs->GetUpperPnts();
    ErrorMgr.NoError();
    return pnt_vec;
}

std::vector<vec3d> GetAirfoilLowerPnts( const std::string& xsec_id )
{
    std::vector< vec3d > pnt_vec;

    XSec* xs = FindXSec( xsec_id );
    if ( !xs )
    {
        ErrorMgr.AddError( VSP_INVALID_XSEC_ID, "GetAirfoilLowerPnts::Can't Find XSec " + xsec_id );
        return pnt_vec;
    }

    if ( xs->GetXSecCurve()->GetType() != XS_FILE_AIRFOIL )
    {
        ErrorMgr.AddError( VSP_WRONG_XSEC_TYPE, "GetAirfoilLowerPnts::XSec Not XS_FILE_AIRFOIL Type " + xsec_id );
        return pnt_vec;
    }

    FileAirfoil* file_xs = dynamic_cast<FileAirfoil*>( xs->GetXSecCurve() );
    assert( file_xs );
    pnt_vec = file_xs->GetLowerPnts();
    ErrorMgr.NoError();
    return pnt_vec;
}

std::vector<double> GetUpperCSTCoefs( const std::string& xsec_id )
{
    std::vector < double > ret_vec;

    XSec* xs = FindXSec( xsec_id );
    if ( !xs )
    {
        ErrorMgr.AddError( VSP_INVALID_XSEC_ID, "GetUpperCSTCoefs::Can't Find XSec " + xsec_id );
        return ret_vec;
    }

    if ( xs->GetXSecCurve()->GetType() != XS_CST_AIRFOIL )
    {
        ErrorMgr.AddError( VSP_WRONG_XSEC_TYPE, "GetUpperCSTCoefs::XSec Not XS_CST_AIRFOIL Type " + xsec_id );
        return ret_vec;
    }

    CSTAirfoil* cst_xs = dynamic_cast<CSTAirfoil*>( xs->GetXSecCurve() );
    assert( cst_xs );

    ret_vec = cst_xs->GetUpperCST();
    ErrorMgr.NoError();
    return ret_vec;
}

std::vector<double> GetLowerCSTCoefs( const std::string& xsec_id )
{
    std::vector < double > ret_vec;

    XSec* xs = FindXSec( xsec_id );
    if ( !xs )
    {
        ErrorMgr.AddError( VSP_INVALID_XSEC_ID, "GetLowerCSTCoefs::Can't Find XSec " + xsec_id );
        return ret_vec;
    }

    if ( xs->GetXSecCurve()->GetType() != XS_CST_AIRFOIL )
    {
        ErrorMgr.AddError( VSP_WRONG_XSEC_TYPE, "GetLowerCSTCoefs::XSec Not XS_CST_AIRFOIL Type " + xsec_id );
        return ret_vec;
    }

    CSTAirfoil* cst_xs = dynamic_cast<CSTAirfoil*>( xs->GetXSecCurve() );
    assert( cst_xs );

    ret_vec = cst_xs->GetLowerCST();
    ErrorMgr.NoError();
    return ret_vec;
}

int GetUpperCSTDegree( const std::string& xsec_id )
{
    int deg = -1;

    XSec* xs = FindXSec( xsec_id );
    if ( !xs )
    {
        ErrorMgr.AddError( VSP_INVALID_XSEC_ID, "GetUpperCSTDegree::Can't Find XSec " + xsec_id );
        return deg;
    }

    if ( xs->GetXSecCurve()->GetType() != XS_CST_AIRFOIL )
    {
        ErrorMgr.AddError( VSP_WRONG_XSEC_TYPE, "GetUpperCSTDegree::XSec Not XS_CST_AIRFOIL Type " + xsec_id );
        return deg;
    }

    CSTAirfoil* cst_xs = dynamic_cast<CSTAirfoil*>( xs->GetXSecCurve() );
    assert( cst_xs );

    deg = cst_xs->GetUpperDegree();
    ErrorMgr.NoError();
    return deg;
}

int GetLowerCSTDegree( const std::string& xsec_id )
{
    int deg = -1;

    XSec* xs = FindXSec( xsec_id );
    if ( !xs )
    {
        ErrorMgr.AddError( VSP_INVALID_XSEC_ID, "GetLowerCSTDegree::Can't Find XSec " + xsec_id );
        return deg;
    }

    if ( xs->GetXSecCurve()->GetType() != XS_CST_AIRFOIL )
    {
        ErrorMgr.AddError( VSP_WRONG_XSEC_TYPE, "GetLowerCSTDegree::XSec Not XS_CST_AIRFOIL Type " + xsec_id );
        return deg;
    }

    CSTAirfoil* cst_xs = dynamic_cast<CSTAirfoil*>( xs->GetXSecCurve() );
    assert( cst_xs );

    deg = cst_xs->GetLowerDegree();
    ErrorMgr.NoError();
    return deg;
}

void SetUpperCST( const std::string& xsec_id, int deg, const std::vector<double> &coefs )
{
    XSec* xs = FindXSec( xsec_id );
    if ( !xs )
    {
        ErrorMgr.AddError( VSP_INVALID_XSEC_ID, "SetUpperCST::Can't Find XSec " + xsec_id );
        return;
    }

    if ( xs->GetXSecCurve()->GetType() != XS_CST_AIRFOIL )
    {
        ErrorMgr.AddError( VSP_WRONG_XSEC_TYPE, "SetUpperCST::XSec Not XS_CST_AIRFOIL Type " + xsec_id );
        return;
    }

    CSTAirfoil* cst_xs = dynamic_cast<CSTAirfoil*>( xs->GetXSecCurve() );
    assert( cst_xs );

    ErrorMgr.NoError();
    cst_xs->SetUpperCST( deg, coefs );
}

void SetLowerCST( const std::string& xsec_id, int deg, const std::vector<double> &coefs )
{
    XSec* xs = FindXSec( xsec_id );
    if ( !xs )
    {
        ErrorMgr.AddError( VSP_INVALID_XSEC_ID, "SetLowerCST::Can't Find XSec " + xsec_id );
        return;
    }

    if ( xs->GetXSecCurve()->GetType() != XS_CST_AIRFOIL )
    {
        ErrorMgr.AddError( VSP_WRONG_XSEC_TYPE, "SetLowerCST::XSec Not XS_CST_AIRFOIL Type " + xsec_id );
        return;
    }

    CSTAirfoil* cst_xs = dynamic_cast<CSTAirfoil*>( xs->GetXSecCurve() );
    assert( cst_xs );

    ErrorMgr.NoError();
    cst_xs->SetLowerCST( deg, coefs );
}

void PromoteCSTUpper( const std::string& xsec_id )
{
    XSec* xs = FindXSec( xsec_id );
    if ( !xs )
    {
        ErrorMgr.AddError( VSP_INVALID_XSEC_ID, "PromoteCSTUpper::Can't Find XSec " + xsec_id );
        return;
    }

    if ( xs->GetXSecCurve()->GetType() != XS_CST_AIRFOIL )
    {
        ErrorMgr.AddError( VSP_WRONG_XSEC_TYPE, "PromoteCSTUpper::XSec Not XS_CST_AIRFOIL Type " + xsec_id );
        return;
    }

    CSTAirfoil* cst_xs = dynamic_cast<CSTAirfoil*>( xs->GetXSecCurve() );
    assert( cst_xs );

    ErrorMgr.NoError();
    cst_xs->PromoteUpper();
}

void PromoteCSTLower( const std::string& xsec_id )
{
    XSec* xs = FindXSec( xsec_id );
    if ( !xs )
    {
        ErrorMgr.AddError( VSP_INVALID_XSEC_ID, "PromoteCSTLower::Can't Find XSec " + xsec_id );
        return;
    }

    if ( xs->GetXSecCurve()->GetType() != XS_CST_AIRFOIL )
    {
        ErrorMgr.AddError( VSP_WRONG_XSEC_TYPE, "PromoteCSTLower::XSec Not XS_CST_AIRFOIL Type " + xsec_id );
        return;
    }

    CSTAirfoil* cst_xs = dynamic_cast<CSTAirfoil*>( xs->GetXSecCurve() );
    assert( cst_xs );

    ErrorMgr.NoError();
    cst_xs->PromoteLower();
}

void DemoteCSTUpper( const std::string& xsec_id )
{
    XSec* xs = FindXSec( xsec_id );
    if ( !xs )
    {
        ErrorMgr.AddError( VSP_INVALID_XSEC_ID, "DemoteCSTUpper::Can't Find XSec " + xsec_id );
        return;
    }

    if ( xs->GetXSecCurve()->GetType() != XS_CST_AIRFOIL )
    {
        ErrorMgr.AddError( VSP_WRONG_XSEC_TYPE, "DemoteCSTUpper::XSec Not XS_CST_AIRFOIL Type " + xsec_id  );
        return;
    }

    CSTAirfoil* cst_xs = dynamic_cast<CSTAirfoil*>( xs->GetXSecCurve() );
    assert( cst_xs );

    ErrorMgr.NoError();
    cst_xs->DemoteUpper();
}

void DemoteCSTLower( const std::string& xsec_id )
{
    XSec* xs = FindXSec( xsec_id );
    if ( !xs )
    {
        ErrorMgr.AddError( VSP_INVALID_XSEC_ID, "DemoteCSTLower::Can't Find XSec " + xsec_id );
        return;
    }

    if ( xs->GetXSecCurve()->GetType() != XS_CST_AIRFOIL )
    {
        ErrorMgr.AddError( VSP_WRONG_XSEC_TYPE, "DemoteCSTLower::XSec Not XS_CST_AIRFOIL Type " + xsec_id );
        return;
    }

    CSTAirfoil* cst_xs = dynamic_cast<CSTAirfoil*>( xs->GetXSecCurve() );
    assert( cst_xs );

    ErrorMgr.NoError();
    cst_xs->DemoteLower();
}

static void FitAfCSTImpl( const std::string & xsec_surf_id, int xsec_index, int deg, const std::string & caller )
{
    XSecSurf* xsec_surf = FindXSecSurf( xsec_surf_id );
    if ( !xsec_surf )
    {
        ErrorMgr.AddError( VSP_INVALID_ID, caller + "::Can't Find XSecSurf " + xsec_surf_id );
        return;
    }
    XSec* xsec = xsec_surf->FindXSec( xsec_index );
    if ( !xsec )
    {
        ErrorMgr.AddError( VSP_INDEX_OUT_RANGE, caller + "::XSec Index Out of Range " + xsec_surf_id + ":" + to_string( ( long long )xsec_index ) );
        return;
    }

    if ( deg < 1 )
    {
        ErrorMgr.AddError( VSP_INVALID_INPUT_VAL, caller + "::Fit Degree " + to_string( deg ) + " Must Be Positive" );
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
        ErrorMgr.AddError( VSP_WRONG_XSEC_TYPE, caller + "::XSec Not Fittable Airfoil Type " + xsec_surf_id + ":" + to_string( ( long long )xsec_index ) );
        return;
    }

    XSecCurve* xsc = xsec->GetXSecCurve();
    if ( !xsc )
    {
        ErrorMgr.AddError( VSP_INVALID_PTR, caller + "::Can't Get XSecCurve" );
        return;
    }

    Airfoil* af_xs = dynamic_cast< Airfoil* >( xsc );
    if ( !af_xs )
    {
        ErrorMgr.AddError( VSP_INVALID_PTR, caller + "::Can't Get Airfoil" );
        return;
    }

    VspCurve c = af_xs->GetOrigCurve();

    xsec_surf->ChangeXSecShape( xsec_index, XS_CST_AIRFOIL );

    XSec* newxsec = xsec_surf->FindXSec( xsec_index );
    if ( !newxsec )
    {
        ErrorMgr.AddError( VSP_INVALID_PTR, caller + "::Can't Find New XSec " + xsec_surf_id + ":" + to_string( ( long long )xsec_index ) );
        return;
    }

    XSecCurve* newxsc = newxsec->GetXSecCurve();
    if ( !newxsc )
    {
        ErrorMgr.AddError( VSP_INVALID_PTR, caller + "::Can't Get New XSecCurve" );
        return;
    }

    if ( newxsc->GetType() != XS_CST_AIRFOIL )
    {
        ErrorMgr.AddError( VSP_WRONG_XSEC_TYPE, caller + "::XSec Not XS_CST_AIRFOIL Type" );
        return;
    }

    CSTAirfoil* cst_xs = dynamic_cast< CSTAirfoil* >( newxsc );

    assert( cst_xs );
    cst_xs->FitCurve( c, deg );

    ErrorMgr.NoError();
}

void FitAfCST( const std::string & xsec_surf_id, int xsec_index, int deg )
{
    FitAfCSTImpl( xsec_surf_id, xsec_index, deg, "FitAfCST" );
}

void FitAfCSTByID( const std::string & xsec_id, int deg )
{
    int index = -1;
    XSecSurf* xsec_surf = FindXSecSurfOf( xsec_id, index, "FitAfCSTByID" );
    if ( !xsec_surf )
    {
        return;
    }

    FitAfCSTImpl( xsec_surf->GetID(), index, deg, "FitAfCSTByID" );
}

void WriteBezierAirfoil( const std::string & file_name, const std::string & geom_id, const double &foilsurf_u )
{
    Vehicle* veh = GetVehicle();
    Geom* geom_ptr = veh->FindGeom( geom_id );
    if ( !geom_ptr )
    {
        ErrorMgr.AddError( VSP_INVALID_GEOM_ID, "WriteBezierAirfoil::Can't Find Geom " + geom_id );
        return;
    }

    if ( foilsurf_u < 0.0 || foilsurf_u > 1.0 )
    {
        ErrorMgr.AddError( VSP_INVALID_INPUT_VAL, "WriteBezierAirfoil::Invalid u Location " + to_string( foilsurf_u ) + " - Must be range [0,1]." );
        return;
    }

    // Airfoils carry no placement, so a Clone's are its original's.
    geom_ptr->GetBehaviorGeom()->WriteBezierAirfoil( file_name, foilsurf_u );
    ErrorMgr.NoError();
}

void WriteSeligAirfoil( const std::string & file_name, const std::string & geom_id, const double &foilsurf_u )
{
    Vehicle* veh = GetVehicle();
    Geom* geom_ptr = veh->FindGeom( geom_id );
    if ( !geom_ptr )
    {
        ErrorMgr.AddError( VSP_INVALID_GEOM_ID, "WriteSeligAirfoil::Can't Find Geom " + geom_id );
        return;
    }

    if ( foilsurf_u < 0.0 || foilsurf_u > 1.0 )
    {
        ErrorMgr.AddError( VSP_INVALID_INPUT_VAL, "WriteSeligAirfoil::Invalid u Location " + to_string( foilsurf_u ) + " - Must be range [0,1]." );
        return;
    }

    // Airfoils carry no placement, so a Clone's are its original's.
    geom_ptr->GetBehaviorGeom()->WriteSeligAirfoil( file_name, foilsurf_u );
    ErrorMgr.NoError();
}

std::vector < vec3d > GetAirfoilCoordinates( const std::string & geom_id, const double &foilsurf_u )
{
    std::vector < vec3d > ordered_vec;
    Vehicle* veh = GetVehicle();
    Geom* geom_ptr = veh->FindGeom( geom_id );
    if ( !geom_ptr )
    {
        ErrorMgr.AddError( VSP_INVALID_GEOM_ID, "GetAirfoilCoordinates::Can't Find Geom " + geom_id );
        return ordered_vec;
    }

    if ( foilsurf_u < 0.0 || foilsurf_u > 1.0 )
    {
        ErrorMgr.AddError( VSP_INVALID_INPUT_VAL, "GetAirfoilCoordinates::Invalid u Location " + to_string( foilsurf_u ) + " - Must be range [0,1]." );
        return ordered_vec;
    }

    // Airfoils carry no placement, so a Clone's are its original's.
    ordered_vec = geom_ptr->GetBehaviorGeom()->GetAirfoilCoordinates( foilsurf_u );
    ErrorMgr.NoError();
    return ordered_vec;
}

//===================================================================//
//==============      Background3D Functions        =================//
//===================================================================//

std::string AddBackground3D()
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

std::vector < std::string > GetAllBackground3Ds()
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

void DelBackground3D( const std::string &id )
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

std::vector < std::string > GetAllBackground3DRelativePaths()
{
    ErrorMgr.NoError();
    return Background3DMgr.GetAllBackground3DRelativePaths();
}

std::vector < std::string > GetAllBackground3DAbsolutePaths()
{
    ErrorMgr.NoError();
    return Background3DMgr.GetAllBackground3DAbsolutePaths();
}

std::string GetBackground3DRelativePath( const std::string &id )
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

std::string GetBackground3DAbsolutePath( const std::string &id )
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

void SetBackground3DRelativePath( const std::string &id, const std::string &fname )
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

void SetBackground3DAbsolutePath( const std::string &id, const std::string &fname )
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

int GetNumRoutingPts( const std::string &routing_id )
{
    Vehicle* veh = GetVehicle();
    Geom* geom_ptr = veh->FindGeom( routing_id );
    if ( !geom_ptr )
    {
        ErrorMgr.AddError( VSP_INVALID_GEOM_ID, "GetNumRoutingPts::Can't Find Geom " + routing_id );
        return -1;
    }

    // Read-only, so a Clone of a route answers too.
    RouteRole* route_ptr = Geom::CastTo< RouteRole > ( geom_ptr );

    if ( !route_ptr )
    {
        ErrorMgr.AddError( VSP_WRONG_GEOM_TYPE, "GetNumRoutingPts::Geom " + routing_id + " is not a RoutingGeom" );
        return -1;
    }

    ErrorMgr.NoError();
    return route_ptr->GetNumRoutePts();
}

std::string AddRoutingPt( const std::string &routing_id, const std::string &geom_id, int surf_index )
{
    std::string ret_id;

    Vehicle* veh = GetVehicle();
    Geom* rgeom_ptr = veh->FindGeom( routing_id );
    if ( !rgeom_ptr )
    {
        ErrorMgr.AddError( VSP_INVALID_GEOM_ID, "AddRoutingPt::Can't Find Geom " + routing_id );
        return ret_id;
    }

    RoutingGeom* routing_ptr = dynamic_cast< RoutingGeom* > ( rgeom_ptr );

    if ( !routing_ptr || rgeom_ptr->GetType().m_Type != ROUTING_GEOM_TYPE )
    {
        ErrorMgr.AddError( VSP_WRONG_GEOM_TYPE, "AddRoutingPt::Geom " + routing_id + " is not a RoutingGeom" );
        return ret_id;
    }

    Geom* geom_ptr = veh->FindGeom( geom_id );
    if ( !geom_ptr )
    {
        ErrorMgr.AddError( VSP_INVALID_GEOM_ID, "AddRoutingPt::Can't Find Geom " + geom_id );
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

//==== The index of a routing point by ID, reporting through the named caller ====//
int FindRoutingPtIndex( const std::string & routing_id, const std::string & pt_id, const std::string & caller )
{
    Geom* rgeom_ptr = GetVehicle()->FindGeom( routing_id );
    if ( !rgeom_ptr )
    {
        ErrorMgr.AddError( VSP_INVALID_GEOM_ID, caller + "::Can't Find Geom " + routing_id );
        return -1;
    }

    // A Clone's points are its original's.
    RoutingGeom* behavior_route = dynamic_cast< RoutingGeom* > ( rgeom_ptr->GetBehaviorGeom() );
    if ( !behavior_route || rgeom_ptr->GetBehaviorType() != ROUTING_GEOM_TYPE )
    {
        ErrorMgr.AddError( VSP_WRONG_GEOM_TYPE, caller + "::Geom " + routing_id + " is not a RoutingGeom" );
        return -1;
    }

    int index = vector_find_val( behavior_route->GetAllPtIds(), pt_id );
    if ( index < 0 )
    {
        ErrorMgr.AddError( VSP_INVALID_ID, caller + "::Can't Find Routing Point " + pt_id + " in " + routing_id );
    }
    return index;
}

static std::string InsertRoutingPtImpl( const std::string &routing_id, int index, const std::string &geom_id, int surf_index, const std::string & caller )
{
    std::string ret_id;

    Vehicle* veh = GetVehicle();
    Geom* rgeom_ptr = veh->FindGeom( routing_id );
    if ( !rgeom_ptr )
    {
        ErrorMgr.AddError( VSP_INVALID_GEOM_ID, caller + "::Can't Find Geom " + routing_id );
        return ret_id;
    }

    RoutingGeom* routing_ptr = dynamic_cast< RoutingGeom* > ( rgeom_ptr );

    if ( !routing_ptr || rgeom_ptr->GetType().m_Type != ROUTING_GEOM_TYPE )
    {
        ErrorMgr.AddError( VSP_WRONG_GEOM_TYPE, caller + "::Geom " + routing_id + " is not a RoutingGeom" );
        return ret_id;
    }

    Geom* geom_ptr = veh->FindGeom( geom_id );
    if ( !geom_ptr )
    {
        ErrorMgr.AddError( VSP_INVALID_GEOM_ID, caller + "::Can't Find Geom " + geom_id );
        return ret_id;
    }

    if ( index < 0 || index > routing_ptr->GetNumPt() ) // Note index can == size() for Insert
    {
        ErrorMgr.AddError( VSP_INDEX_OUT_RANGE, caller + "::index " + to_string( index ) + " is out of range" );
        return ret_id;
    }

    RoutingPoint *rpt = routing_ptr->InsertPt( index );
    if ( !rpt )
    {
        ErrorMgr.AddError( VSP_INVALID_PTR, caller + "::Could not insert routing point." );
        return ret_id;
    }

    rpt->SetParentID( geom_id );
    rpt->m_SurfIndx.Set( surf_index );

    ret_id = rpt->GetID();

    ErrorMgr.NoError();
    return ret_id;
}

std::string InsertRoutingPt( const std::string &routing_id, int index, const std::string &geom_id, int surf_index )
{
    return InsertRoutingPtImpl( routing_id, index, geom_id, surf_index, "InsertRoutingPt" );
}

std::string InsertRoutingPtByID( const std::string &routing_id, const std::string &before_pt_id, const std::string &geom_id, int surf_index )
{
    int index = FindRoutingPtIndex( routing_id, before_pt_id, "InsertRoutingPtByID" );
    if ( index < 0 )
    {
        return std::string();
    }

    return InsertRoutingPtImpl( routing_id, index, geom_id, surf_index, "InsertRoutingPtByID" );
}

static void DelRoutingPtImpl( const std::string &routing_id, int index, const std::string & caller )
{
    Vehicle* veh = GetVehicle();
    Geom* rgeom_ptr = veh->FindGeom( routing_id );
    if ( !rgeom_ptr )
    {
        ErrorMgr.AddError( VSP_INVALID_GEOM_ID, caller + "::Can't Find Geom " + routing_id );
        return;
    }

    RoutingGeom* routing_ptr = dynamic_cast< RoutingGeom* > ( rgeom_ptr );

    if ( !routing_ptr || rgeom_ptr->GetType().m_Type != ROUTING_GEOM_TYPE )
    {
        ErrorMgr.AddError( VSP_WRONG_GEOM_TYPE, caller + "::Geom " + routing_id + " is not a RoutingGeom" );
        return;
    }

    if ( index < 0 || index >= routing_ptr->GetNumPt() )
    {
        ErrorMgr.AddError( VSP_INDEX_OUT_RANGE, caller + "::index " + to_string( index ) + " is out of range" );
        return;
    }

    routing_ptr->DelPt( index );

    ErrorMgr.NoError();
    return;
}

void DelRoutingPt( const std::string &routing_id, int index )
{
    DelRoutingPtImpl( routing_id, index, "DelRoutingPt" );
}

void DelRoutingPtByID( const std::string &routing_id, const std::string &pt_id )
{
    int index = FindRoutingPtIndex( routing_id, pt_id, "DelRoutingPtByID" );
    if ( index < 0 )
    {
        return;
    }

    DelRoutingPtImpl( routing_id, index, "DelRoutingPtByID" );
}

void DelAllRoutingPt( const std::string &routing_id )
{
    Vehicle* veh = GetVehicle();
    Geom* rgeom_ptr = veh->FindGeom( routing_id );
    if ( !rgeom_ptr )
    {
        ErrorMgr.AddError( VSP_INVALID_GEOM_ID, "DelAllRoutingPt::Can't Find Geom " + routing_id );
        return;
    }

    RoutingGeom* routing_ptr = dynamic_cast< RoutingGeom* > ( rgeom_ptr );

    if ( !routing_ptr || rgeom_ptr->GetType().m_Type != ROUTING_GEOM_TYPE )
    {
        ErrorMgr.AddError( VSP_WRONG_GEOM_TYPE, "DelAllRoutingPt::Geom " + routing_id + " is not a RoutingGeom" );
        return;
    }

    routing_ptr->DelAllPt();

    ErrorMgr.NoError();
    return;
}

static int MoveRoutingPtImpl( const std::string &routing_id, int index, int reorder_type, const std::string & caller )
{
    int newindex = index;
    Vehicle* veh = GetVehicle();
    Geom* rgeom_ptr = veh->FindGeom( routing_id );
    if ( !rgeom_ptr )
    {
        ErrorMgr.AddError( VSP_INVALID_GEOM_ID, caller + "::Can't Find Geom " + routing_id );
        return newindex;
    }

    RoutingGeom* routing_ptr = dynamic_cast< RoutingGeom* > ( rgeom_ptr );

    if ( !routing_ptr || rgeom_ptr->GetType().m_Type != ROUTING_GEOM_TYPE )
    {
        ErrorMgr.AddError( VSP_WRONG_GEOM_TYPE, caller + "::Geom " + routing_id + " is not a RoutingGeom" );
        return newindex;
    }

    if ( index < 0 || index >= routing_ptr->GetNumPt() )
    {
        ErrorMgr.AddError( VSP_INDEX_OUT_RANGE, caller + "::index " + to_string( index ) + " is out of range" );
        return newindex;
    }

    newindex = routing_ptr->MovePt( index, reorder_type );

    ErrorMgr.NoError();
    return newindex;
}

int MoveRoutingPt( const std::string &routing_id, int index, int reorder_type )
{
    return MoveRoutingPtImpl( routing_id, index, reorder_type, "MoveRoutingPt" );
}

int MoveRoutingPtByID( const std::string &routing_id, const std::string &pt_id, int reorder_type )
{
    int index = FindRoutingPtIndex( routing_id, pt_id, "MoveRoutingPtByID" );
    if ( index < 0 )
    {
        return -1;
    }

    return MoveRoutingPtImpl( routing_id, index, reorder_type, "MoveRoutingPtByID" );
}

std::string GetRoutingPtID( const std::string &routing_id, int index )
{
    std::string ret_id;

    Vehicle* veh = GetVehicle();
    Geom* rgeom_ptr = veh->FindGeom( routing_id );
    if ( !rgeom_ptr )
    {
        ErrorMgr.AddError( VSP_INVALID_GEOM_ID, "GetRoutingPtID::Can't Find Geom " + routing_id );
        return ret_id;
    }

    // For a Clone these IDs belong to its original; writing through them changes the original.
    RoutingGeom* behavior_route = dynamic_cast< RoutingGeom* > ( rgeom_ptr->GetBehaviorGeom() );

    if ( !behavior_route || rgeom_ptr->GetBehaviorType() != ROUTING_GEOM_TYPE )
    {
        ErrorMgr.AddError( VSP_WRONG_GEOM_TYPE, "GetRoutingPtID::Geom " + routing_id + " is not a RoutingGeom" );
        return ret_id;
    }

    if ( index < 0 || index >= behavior_route->GetNumPt() )
    {
        ErrorMgr.AddError( VSP_INDEX_OUT_RANGE, "GetRoutingPtID::index " + to_string( index ) + " is out of range" );
        return ret_id;
    }

    ret_id = behavior_route->GetPtID( index );

    ErrorMgr.NoError();
    return ret_id;
}

std::vector < std::string > GetAllRoutingPtIds( const std::string &routing_id )
{
    std::vector < std::string > ret_vec;

    Vehicle* veh = GetVehicle();
    Geom* rgeom_ptr = veh->FindGeom( routing_id );
    if ( !rgeom_ptr )
    {
        ErrorMgr.AddError( VSP_INVALID_GEOM_ID, "GetAllRoutingPtIds::Can't Find Geom " + routing_id );
        return ret_vec;
    }

    // For a Clone these IDs belong to its original; writing through them changes the original.
    RoutingGeom* behavior_route = dynamic_cast< RoutingGeom* > ( rgeom_ptr->GetBehaviorGeom() );

    if ( !behavior_route || rgeom_ptr->GetBehaviorType() != ROUTING_GEOM_TYPE )
    {
        ErrorMgr.AddError( VSP_WRONG_GEOM_TYPE, "GetAllRoutingPtIds::Geom " + routing_id + " is not a RoutingGeom" );
        return ret_vec;
    }

    ret_vec = behavior_route->GetAllPtIds();

    ErrorMgr.NoError();
    return ret_vec;
}

std::string GetRoutingPtParentID( const std::string & pt_id )
{
    std::string ret_id;

    Vehicle* veh = GetVehicle();
    ParmContainer *pc = ParmMgr.FindParmContainer( pt_id );
    RoutingPoint *rpt = dynamic_cast< RoutingPoint* > ( pc );
    if ( !rpt )
    {
        ErrorMgr.AddError( VSP_INVALID_ID, "GetRoutingPtParentID::Can't Find Routing Point " + pt_id );
        return ret_id;
    }

    ret_id = rpt->GetParentID();

    ErrorMgr.NoError();
    return ret_id;
}

void SetRoutingPtParentID( const std::string & pt_id, const std::string &parent_id )
{
    Vehicle* veh = GetVehicle();
    ParmContainer *pc = ParmMgr.FindParmContainer( pt_id );
    RoutingPoint *rpt = dynamic_cast< RoutingPoint* > ( pc );
    if ( !rpt )
    {
        ErrorMgr.AddError( VSP_INVALID_ID, "SetRoutingPtParentID::Can't Find Routing Point " + pt_id );
        return;
    }

    if ( !TopLevelParentID( parent_id ) && !veh->FindGeom( parent_id ) )
    {
        ErrorMgr.AddError( VSP_INVALID_GEOM_ID, "SetRoutingPtParentID::Can't Find Geom " + parent_id );
        return;
    }

    rpt->SetParentID( parent_id );
    if ( rpt->GetParentID() != parent_id )
    {
        ErrorMgr.AddError( VSP_INVALID_INPUT_VAL, "SetRoutingPtParentID::Geom " + parent_id + " depends on the routing Geom" );
        return;
    }

    ErrorMgr.NoError();
    return;
}

vec3d GetMainRoutingPtCoord( const std::string &pt_id )
{
    vec3d ret;

    Vehicle* veh = GetVehicle();
    ParmContainer *pc = ParmMgr.FindParmContainer( pt_id );
    RoutingPoint *rpt = dynamic_cast< RoutingPoint* > ( pc );
    if ( !rpt )
    {
        ErrorMgr.AddError( VSP_INVALID_ID, "GetMainRoutingPtCoord::Can't Find Routing Point " + pt_id );
        return ret;
    }

    ret = rpt->GetPt();

    ErrorMgr.NoError();
    return ret;
}

static vec3d GetRoutingPtCoordImpl( const std::string &routing_id, int index, int symm_index, const std::string & caller )
{
    vec3d ret;

    Vehicle* veh = GetVehicle();
    Geom* rgeom_ptr = veh->FindGeom( routing_id );
    if ( !rgeom_ptr )
    {
        ErrorMgr.AddError( VSP_INVALID_GEOM_ID, caller + "::Can't Find Geom " + routing_id );
        return ret;
    }

    // Placed coordinates, so asked of this Geom, not its original; a Clone holds its own.
    RouteRole* route_ptr = Geom::CastTo< RouteRole > ( rgeom_ptr );

    if ( !route_ptr )
    {
        ErrorMgr.AddError( VSP_WRONG_GEOM_TYPE, caller + "::Geom " + routing_id + " is not a RoutingGeom" );
        return ret;
    }

    if ( index < 0 || index >= route_ptr->GetNumRoutePts() )
    {
        ErrorMgr.AddError( VSP_INDEX_OUT_RANGE, caller + "::index " + to_string( index ) + " is out of range" );
        return ret;
    }

    if ( symm_index < 0 || symm_index >= rgeom_ptr->GetNumSymmCopies() )
    {
        ErrorMgr.AddError( VSP_INDEX_OUT_RANGE, caller + "::symm_index " + to_string( symm_index ) + " is out of range" );
        return ret;
    }

    ret = route_ptr->GetRoutePtCoord( index, symm_index );

    ErrorMgr.NoError();
    return ret;
}

vec3d GetRoutingPtCoord( const std::string &routing_id, int index, int symm_index )
{
    return GetRoutingPtCoordImpl( routing_id, index, symm_index, "GetRoutingPtCoord" );
}

vec3d GetRoutingPtCoordByID( const std::string &routing_id, const std::string &pt_id, int symm_index )
{
    int index = FindRoutingPtIndex( routing_id, pt_id, "GetRoutingPtCoordByID" );
    if ( index < 0 )
    {
        return vec3d();
    }

    return GetRoutingPtCoordImpl( routing_id, index, symm_index, "GetRoutingPtCoordByID" );
}

std::vector < vec3d > GetAllRoutingPtCoords( const std::string &routing_id, int symm_index )
{
    std::vector < vec3d > ret_vec;

    Vehicle* veh = GetVehicle();
    Geom* rgeom_ptr = veh->FindGeom( routing_id );
    if ( !rgeom_ptr )
    {
        ErrorMgr.AddError( VSP_INVALID_GEOM_ID, "GetAllRoutingPtCoords::Can't Find Geom " + routing_id );
        return ret_vec;
    }

    // Placed coordinates, so they come from the Geom that was asked -- see GetRoutingPtCoord.
    RouteRole* route_ptr = Geom::CastTo< RouteRole > ( rgeom_ptr );

    if ( !route_ptr )
    {
        ErrorMgr.AddError( VSP_WRONG_GEOM_TYPE, "GetAllRoutingPtCoords::Geom " + routing_id + " is not a RoutingGeom" );
        return ret_vec;
    }

    if ( symm_index < 0 || symm_index >= rgeom_ptr->GetNumSymmCopies() )
    {
        ErrorMgr.AddError( VSP_INDEX_OUT_RANGE, "GetAllRoutingPtCoords::symm_index " + to_string( symm_index ) + " is out of range" );
        return ret_vec;
    }

    ret_vec = route_ptr->GetAllRoutePtCoord( symm_index );

    ErrorMgr.NoError();
    return ret_vec;
}

std::vector < vec3d > GetRoutingCurve( const std::string &routing_id, int symm_index )
{
    std::vector < vec3d > ret_vec;

    Vehicle* veh = GetVehicle();
    Geom* rgeom_ptr = veh->FindGeom( routing_id );
    if ( !rgeom_ptr )
    {
        ErrorMgr.AddError( VSP_INVALID_GEOM_ID, "GetRoutingCurve::Can't Find Geom " + routing_id );
        return ret_vec;
    }

    // The placed curve, so it comes from the Geom that was asked -- see GetRoutingPtCoord.
    RouteRole* route_ptr = Geom::CastTo< RouteRole > ( rgeom_ptr );

    if ( !route_ptr )
    {
        ErrorMgr.AddError( VSP_WRONG_GEOM_TYPE, "GetRoutingCurve::Geom " + routing_id + " is not a RoutingGeom" );
        return ret_vec;
    }

    if ( symm_index < 0 || symm_index >= rgeom_ptr->GetNumSymmCopies() )
    {
        ErrorMgr.AddError( VSP_INDEX_OUT_RANGE, "GetRoutingCurve::symm_index " + to_string( symm_index ) + " is out of range" );
        return ret_vec;
    }

    ret_vec = route_ptr->GetRouteCurve( symm_index );

    ErrorMgr.NoError();
    return ret_vec;
}

//===================================================================//
//==================      GearGeom Functions       ==================//
//===================================================================//

//===================================================================//
//===============       Auxiliary Geom Functions      ===============//
//===================================================================//

// Look up an AuxiliaryGeom, reporting through the named caller on failure.
static AuxiliaryGeom* FindAuxiliaryGeom( const std::string &geom_id, const std::string &caller )
{
    Vehicle* veh = GetVehicle();
    Geom* geom_ptr = veh->FindGeom( geom_id );
    if ( !geom_ptr )
    {
        ErrorMgr.AddError( VSP_INVALID_GEOM_ID, caller + "::Can't Find Geom " + geom_id );
        return nullptr;
    }

    AuxiliaryGeom* aux_ptr = dynamic_cast< AuxiliaryGeom* > ( geom_ptr );

    if ( !aux_ptr || geom_ptr->GetType().m_Type != AUXILIARY_GEOM_TYPE )
    {
        ErrorMgr.AddError( VSP_WRONG_GEOM_TYPE, caller + "::Geom " + geom_id + " is not an AuxiliaryGeom" );
        return nullptr;
    }

    return aux_ptr;
}

void SetAuxiliaryGeomContactPtID( const std::string &geom_id, int index, const std::string &bogie_id )
{
    AuxiliaryGeom* aux_ptr = FindAuxiliaryGeom( geom_id, "SetAuxiliaryGeomContactPtID" );
    if ( !aux_ptr )
    {
        return;
    }

    if ( index < 0 || index > 2 )
    {
        ErrorMgr.AddError( VSP_INDEX_OUT_RANGE, "SetAuxiliaryGeomContactPtID::Index Out Of Range " + to_string( index ) );
        return;
    }

    if ( index == 0 )
    {
        aux_ptr->SetContactPt1ID( bogie_id );
    }
    else if ( index == 1 )
    {
        aux_ptr->SetContactPt2ID( bogie_id );
    }
    else
    {
        aux_ptr->SetContactPt3ID( bogie_id );
    }

    ErrorMgr.NoError();
}

std::string GetAuxiliaryGeomContactPtID( const std::string &geom_id, int index )
{
    AuxiliaryGeom* aux_ptr = FindAuxiliaryGeom( geom_id, "GetAuxiliaryGeomContactPtID" );
    if ( !aux_ptr )
    {
        return std::string();
    }

    if ( index < 0 || index > 2 )
    {
        ErrorMgr.AddError( VSP_INDEX_OUT_RANGE, "GetAuxiliaryGeomContactPtID::Index Out Of Range " + to_string( index ) );
        return std::string();
    }

    std::string ret_id;
    if ( index == 0 )
    {
        ret_id = aux_ptr->GetContactPt1ID();
    }
    else if ( index == 1 )
    {
        ret_id = aux_ptr->GetContactPt2ID();
    }
    else
    {
        ret_id = aux_ptr->GetContactPt3ID();
    }

    ErrorMgr.NoError();
    return ret_id;
}

void ReadAuxiliaryGeomCCEFile( const std::string &geom_id, const std::string &file_name )
{
    AuxiliaryGeom* aux_ptr = FindAuxiliaryGeom( geom_id, "ReadAuxiliaryGeomCCEFile" );
    if ( !aux_ptr )
    {
        return;
    }

    if ( !aux_ptr->ReadCCEFile( file_name ) )
    {
        ErrorMgr.AddError( VSP_FILE_DOES_NOT_EXIST, "ReadAuxiliaryGeomCCEFile::Can't Read File " + file_name );
        return;
    }

    aux_ptr->Update();

    ErrorMgr.NoError();
}

//===================================================================//
//=================       Stack Geom Functions      =================//
//===================================================================//

void InitStackPreset( const std::string &geom_id, int preset )
{
    Vehicle* veh = GetVehicle();
    Geom* geom_ptr = veh->FindGeom( geom_id );
    if ( !geom_ptr )
    {
        ErrorMgr.AddError( VSP_INVALID_GEOM_ID, "InitStackPreset::Can't Find Geom " + geom_id );
        return;
    }

    StackGeom* stack_ptr = dynamic_cast< StackGeom* > ( geom_ptr );

    if ( !stack_ptr )
    {
        ErrorMgr.AddError( VSP_WRONG_GEOM_TYPE, "InitStackPreset::Geom " + geom_id + " is not a StackGeom" );
        return;
    }

    if ( preset < STACK_PRESET_DEFAULT || preset > STACK_PRESET_FLOWPATH_OUT )
    {
        ErrorMgr.AddError( VSP_INVALID_TYPE, "InitStackPreset::Invalid Preset " + to_string( preset ) );
        return;
    }

    stack_ptr->InitParms( preset );
    stack_ptr->Update();

    ErrorMgr.NoError();
}

std::string CreateAndAddBogie( const std::string &gear_id )
{
    std::string ret_id;

    Vehicle* veh = GetVehicle();
    Geom* geom_ptr = veh->FindGeom( gear_id );
    if ( !geom_ptr )
    {
        ErrorMgr.AddError( VSP_INVALID_GEOM_ID, "CreateAndAddBogie::Can't Find Geom " + gear_id );
        return ret_id;
    }

    GearGeom* gear_ptr = dynamic_cast< GearGeom* > ( geom_ptr );

    if ( !gear_ptr || geom_ptr->GetType().m_Type != GEAR_GEOM_TYPE )
    {
        ErrorMgr.AddError( VSP_WRONG_GEOM_TYPE, "CreateAndAddBogie::Geom " + gear_id + " is not a GearGeom" );
        return ret_id;
    }

    ret_id = gear_ptr->CreateAndAddBogie( 0 );

    ErrorMgr.NoError();
    return ret_id;
}

int GetNumBogies( const std::string &gear_id )
{
    Vehicle* veh = GetVehicle();
    Geom* geom_ptr = veh->FindGeom( gear_id );
    if ( !geom_ptr )
    {
        ErrorMgr.AddError( VSP_INVALID_GEOM_ID, "GetNumBogies::Can't Find Geom " + gear_id );
        return -1;
    }

    // Read-only, so a Clone of a gear answers from its original.
    GearGeom* behavior_gear = dynamic_cast< GearGeom* > ( geom_ptr->GetBehaviorGeom() );

    if ( !behavior_gear || geom_ptr->GetBehaviorType() != GEAR_GEOM_TYPE )
    {
        ErrorMgr.AddError( VSP_WRONG_GEOM_TYPE, "GetNumBogies::Geom " + gear_id + " is not a GearGeom" );
        return -1;
    }

    ErrorMgr.NoError();
    return ( int )behavior_gear->GetBogieVec().size();
}

std::vector < std::string > GetAllBogies( const std::string &gear_id )
{
    std::vector < std::string > ret_vec;

    Vehicle* veh = GetVehicle();
    Geom* geom_ptr = veh->FindGeom( gear_id );
    if ( !geom_ptr )
    {
        ErrorMgr.AddError( VSP_INVALID_GEOM_ID, "GetAllBogies::Can't Find Geom " + gear_id );
        return ret_vec;
    }

    // For a Clone these are its original's bogies; writing through them changes the original.
    GearGeom* behavior_gear = dynamic_cast< GearGeom* > ( geom_ptr->GetBehaviorGeom() );

    if ( !behavior_gear || geom_ptr->GetBehaviorType() != GEAR_GEOM_TYPE )
    {
        ErrorMgr.AddError( VSP_WRONG_GEOM_TYPE, "GetAllBogies::Geom " + gear_id + " is not a GearGeom" );
        return ret_vec;
    }

    ret_vec = behavior_gear->GetAllBogies();

    ErrorMgr.NoError();
    return ret_vec;
}

void DelBogie( const std::string &gear_id, const std::string &bogie_id )
{
    Vehicle* veh = GetVehicle();
    Geom* geom_ptr = veh->FindGeom( gear_id );
    if ( !geom_ptr )
    {
        ErrorMgr.AddError( VSP_INVALID_GEOM_ID, "DelBogie::Can't Find Geom " + gear_id );
        return;
    }

    GearGeom* gear_ptr = dynamic_cast< GearGeom* > ( geom_ptr );

    if ( !gear_ptr || geom_ptr->GetType().m_Type != GEAR_GEOM_TYPE )
    {
        ErrorMgr.AddError( VSP_WRONG_GEOM_TYPE, "DelBogie::Geom " + gear_id + " is not a GearGeom" );
        return;
    }

    if ( !gear_ptr->GetBogie( bogie_id ) )
    {
        ErrorMgr.AddError( VSP_INVALID_ID, "DelBogie::Can't Find Bogie " + bogie_id );
        return;
    }

    gear_ptr->DelBogie( bogie_id );

    ErrorMgr.NoError();
    return;
}

void DelAllBogies( const std::string &gear_id )
{
    Vehicle* veh = GetVehicle();
    Geom* geom_ptr = veh->FindGeom( gear_id );
    if ( !geom_ptr )
    {
        ErrorMgr.AddError( VSP_INVALID_GEOM_ID, "DelAllBogies::Can't Find Geom " + gear_id );
        return;
    }

    GearGeom* gear_ptr = dynamic_cast< GearGeom* > ( geom_ptr );

    if ( !gear_ptr || geom_ptr->GetType().m_Type != GEAR_GEOM_TYPE )
    {
        ErrorMgr.AddError( VSP_WRONG_GEOM_TYPE, "DelAllBogies::Geom " + gear_id + " is not a GearGeom" );
        return;
    }

    gear_ptr->DelAllBogies();

    ErrorMgr.NoError();
    return;
}

//==== Whether a Geom ID names a Geom or is empty, reporting through the named caller ====//
bool GeomIDOrNone( const std::string & geom_id, const std::string & caller )
{
    if ( !geom_id.empty() && !GetVehicle()->FindGeom( geom_id ) )
    {
        ErrorMgr.AddError( VSP_INVALID_GEOM_ID, caller + "::Can't Find Geom " + geom_id );
        return false;
    }
    return true;
}

//==== Look up a bogie by ID, reporting through the named caller ====//
Bogie* FindBogie( const std::string & bogie_id, const std::string & caller )
{
    Bogie* bogie = dynamic_cast < Bogie* > ( ParmMgr.FindParmContainer( bogie_id ) );
    if ( !bogie )
    {
        ErrorMgr.AddError( VSP_INVALID_ID, caller + "::Can't Find Bogie " + bogie_id );
    }
    return bogie;
}

void SetBogieStowParentID( const std::string & bogie_id, const std::string & geom_id )
{
    Bogie* bogie = FindBogie( bogie_id, "SetBogieStowParentID" );
    if ( !bogie || !GeomIDOrNone( geom_id, "SetBogieStowParentID" ) )
    {
        return;
    }

    Geom* geom_ptr = GetVehicle()->FindGeom( geom_id );
    if ( geom_ptr && geom_ptr->GetType().m_Type == PT_CLOUD_GEOM_TYPE )
    {
        ErrorMgr.AddError( VSP_WRONG_GEOM_TYPE, "SetBogieStowParentID::Geom " + geom_id + " is a point cloud" );
        return;
    }

    bogie->SetStowParentID( geom_id );
    if ( bogie->GetStowParentID() != geom_id )
    {
        ErrorMgr.AddError( VSP_INVALID_INPUT_VAL, "SetBogieStowParentID::Geom " + geom_id + " is built from the bogie's gear" );
        return;
    }
    ErrorMgr.NoError();
}

std::string GetBogieStowParentID( const std::string & bogie_id )
{
    Bogie* bogie = FindBogie( bogie_id, "GetBogieStowParentID" );
    if ( !bogie )
    {
        return std::string();
    }

    ErrorMgr.NoError();
    return bogie->GetStowParentID();
}

void SetBogieMechParentID( const std::string & bogie_id, const std::string & geom_id )
{
    Bogie* bogie = FindBogie( bogie_id, "SetBogieMechParentID" );
    if ( !bogie || !GeomIDOrNone( geom_id, "SetBogieMechParentID" ) )
    {
        return;
    }

    Geom* geom_ptr = GetVehicle()->FindGeom( geom_id );
    if ( geom_ptr && geom_ptr->GetType().m_Type == PT_CLOUD_GEOM_TYPE )
    {
        ErrorMgr.AddError( VSP_WRONG_GEOM_TYPE, "SetBogieMechParentID::Geom " + geom_id + " is a point cloud" );
        return;
    }

    bogie->SetMechParentID( geom_id );
    if ( bogie->GetMechParentID() != geom_id )
    {
        ErrorMgr.AddError( VSP_INVALID_INPUT_VAL, "SetBogieMechParentID::Geom " + geom_id + " is built from the bogie's gear" );
        return;
    }
    ErrorMgr.NoError();
}

std::string GetBogieMechParentID( const std::string & bogie_id )
{
    Bogie* bogie = FindBogie( bogie_id, "GetBogieMechParentID" );
    if ( !bogie )
    {
        return std::string();
    }

    ErrorMgr.NoError();
    return bogie->GetMechParentID();
}

//===================================================================//
//==================      BOR Functions        ======================//
//===================================================================//

//==== Specialized Geom Functions ====//
void ChangeBORXSecShape( const std::string & geom_id, int type )
{
    Vehicle* veh = GetVehicle();
    Geom* geom_ptr = veh->FindGeom( geom_id );
    if ( !geom_ptr )
    {
        ErrorMgr.AddError( VSP_INVALID_GEOM_ID, "ChangeBORXSecShape::Can't Find Geom " + geom_id );
        return;
    }
    else if ( geom_ptr->GetType().m_Type != BOR_GEOM_TYPE )
    {
        ErrorMgr.AddError( VSP_WRONG_GEOM_TYPE, "ChangeBORXSecShape::Geom " + geom_id + " is not a body of revolution" );
        return;
    }

    BORGeom* bor_ptr = dynamic_cast< BORGeom* > ( geom_ptr );
    bor_ptr->SetXSecCurveType( type );
    ErrorMgr.NoError();
}

//==== Specialized Geom Functions ====//
int GetBORXSecShape( const std::string & geom_id )
{
    Vehicle* veh = GetVehicle();
    Geom* geom_ptr = veh->FindGeom( geom_id );
    if ( !geom_ptr )
    {
        ErrorMgr.AddError( VSP_INVALID_GEOM_ID, "GetBORXSecShape::Can't Find Geom " + geom_id );
        return XS_UNDEFINED;
    }
    else if ( geom_ptr->GetBehaviorType() != BOR_GEOM_TYPE )
    {
        ErrorMgr.AddError( VSP_WRONG_GEOM_TYPE, "GetBORXSecShape::Geom " + geom_id + " is not a body of revolution" );
        return XS_UNDEFINED;
    }

    // Read-only, so a Clone of a body of revolution answers from its original.
    BORGeom* behavior_bor = dynamic_cast< BORGeom* > ( geom_ptr->GetBehaviorGeom() );

    ErrorMgr.NoError();
    return behavior_bor->GetXSecCurveType();
}

//==== Read XSec From File ====//
std::vector<vec3d> ReadBORFileXSec( const std::string& bor_id, const std::string& file_name )
{
    std::vector< vec3d > pnt_vec;
    Vehicle* veh = GetVehicle();
    Geom* geom_ptr = veh->FindGeom( bor_id );
    if ( !geom_ptr )
    {
        ErrorMgr.AddError( VSP_INVALID_GEOM_ID, "ReadBORFileXSec::Can't Find Geom " + bor_id );
        return pnt_vec;
    }
    else if ( geom_ptr->GetType().m_Type != BOR_GEOM_TYPE )
    {
        ErrorMgr.AddError( VSP_WRONG_GEOM_TYPE, "ReadBORFileXSec::Geom " + bor_id + " is not a body of revolution" );
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
            ErrorMgr.AddError( VSP_FILE_READ_FAILURE, "ReadBORFileXSec::Error reading fuselage file " + file_name );
            return pnt_vec;
        }
    }

    ErrorMgr.AddError( VSP_WRONG_XSEC_TYPE, "ReadBORFileXSec::XSec Not XS_FILE_FUSE Type " + bor_id );
    return pnt_vec;
}

//==== Set XSec Pnts ====//
std::vector< vec3d > GetBORXSecPnts( const std::string& bor_id )
{
    std::vector< vec3d > pnt_vec;

    Vehicle* veh = GetVehicle();
    Geom* geom_ptr = veh->FindGeom( bor_id );
    if ( !geom_ptr )
    {
        ErrorMgr.AddError( VSP_INVALID_GEOM_ID, "GetBORXSecPnts::Can't Find Geom " + bor_id );
        return pnt_vec;
    }
    else if ( geom_ptr->GetBehaviorType() != BOR_GEOM_TYPE )
    {
        ErrorMgr.AddError( VSP_WRONG_GEOM_TYPE, "GetBORXSecPnts::Geom " + bor_id + " is not a body of revolution" );
        return pnt_vec;
    }

    // Read-only, so a Clone of a body of revolution answers from its original.
    BORGeom* behavior_bor = dynamic_cast< BORGeom* > ( geom_ptr->GetBehaviorGeom() );

    XSecCurve* xsc = behavior_bor->GetXSecCurve();

    if ( !xsc )
    {
        ErrorMgr.AddError( VSP_INVALID_PTR, "GetBORXSecPnts::Can't Get XSecCurve" );
        return pnt_vec;
    }

    if ( xsc->GetType() != XS_FILE_FUSE )
    {
        ErrorMgr.AddError( VSP_WRONG_XSEC_TYPE, "GetBORXSecPnts::XSec Not XS_FILE_FUSE Type " + bor_id );
        return pnt_vec;
    }

    FileXSec* file_xs = dynamic_cast<FileXSec*>( xsc );
    assert( file_xs );
    pnt_vec = file_xs->GetUnityFilePnts();
    ErrorMgr.NoError();
    return pnt_vec;
}

void SetBORXSecPnts( const std::string& bor_id, std::vector< vec3d > & pnt_vec )
{
    Vehicle* veh = GetVehicle();
    Geom* geom_ptr = veh->FindGeom( bor_id );
    if ( !geom_ptr )
    {
        ErrorMgr.AddError( VSP_INVALID_GEOM_ID, "SetBORXSecPnts::Can't Find Geom " + bor_id );
        return;
    }
    else if ( geom_ptr->GetType().m_Type != BOR_GEOM_TYPE )
    {
        ErrorMgr.AddError( VSP_WRONG_GEOM_TYPE, "SetBORXSecPnts::Geom " + bor_id + " is not a body of revolution" );
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
        ErrorMgr.AddError( VSP_WRONG_XSEC_TYPE, "SetBORXSecPnts::XSec Not XS_FILE_FUSE Type " + bor_id );
        return;
    }

    FileXSec* file_xs = dynamic_cast<FileXSec*>( xsc );
    assert( file_xs );
    if ( !file_xs->SetPnts( pnt_vec ) )
    {
        ErrorMgr.AddError( VSP_INVALID_INPUT_VAL, "SetBORXSecPnts::The points must form a closed section in the XY plane, running from the right-hand point down through the bottom, round the side and over the top" );
        return;
    }
    ErrorMgr.NoError();
}

//==== Compute Point Along XSec ====//
vec3d ComputeBORXSecPnt( const std::string& bor_id, double fract )
{
    Vehicle* veh = GetVehicle();
    Geom* geom_ptr = veh->FindGeom( bor_id );
    if ( !geom_ptr )
    {
        ErrorMgr.AddError( VSP_INVALID_GEOM_ID, "ComputeBORXSecPnt::Can't Find Geom " + bor_id );
        return vec3d();
    }
    else if ( geom_ptr->GetBehaviorType() != BOR_GEOM_TYPE )
    {
        ErrorMgr.AddError( VSP_WRONG_GEOM_TYPE, "ComputeBORXSecPnt::Geom " + bor_id + " is not a body of revolution" );
        return vec3d();
    }

    // Read-only, so a Clone of a body of revolution answers from its original.
    BORGeom* behavior_bor = dynamic_cast< BORGeom* > ( geom_ptr->GetBehaviorGeom() );

    XSecCurve* xsc = behavior_bor->GetXSecCurve();

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
vec3d ComputeBORXSecTan( const std::string& bor_id, double fract )
{
    Vehicle* veh = GetVehicle();
    Geom* geom_ptr = veh->FindGeom( bor_id );
    if ( !geom_ptr )
    {
        ErrorMgr.AddError( VSP_INVALID_GEOM_ID, "ComputeBORXSecTan::Can't Find Geom " + bor_id );
        return vec3d();
    }
    else if ( geom_ptr->GetBehaviorType() != BOR_GEOM_TYPE )
    {
        ErrorMgr.AddError( VSP_WRONG_GEOM_TYPE, "ComputeBORXSecTan::Geom " + bor_id + " is not a body of revolution" );
        return vec3d();
    }

    // Read-only, so a Clone of a body of revolution answers from its original.
    BORGeom* behavior_bor = dynamic_cast< BORGeom* > ( geom_ptr->GetBehaviorGeom() );

    XSecCurve* xsc = behavior_bor->GetXSecCurve();

    if ( !xsc )
    {
        ErrorMgr.AddError( VSP_INVALID_PTR, "ComputeBORXSecTan::Can't Get XSecCurve" );
        return vec3d();
    }

    vec3d pnt = xsc->GetCurve().CompTan01( fract );
    ErrorMgr.NoError();

    return pnt;
}

void ReadBORFileAirfoil( const std::string& bor_id, const std::string& file_name )
{
    Vehicle* veh = GetVehicle();
    Geom* geom_ptr = veh->FindGeom( bor_id );
    if ( !geom_ptr )
    {
        ErrorMgr.AddError( VSP_INVALID_GEOM_ID, "ReadBORFileAirfoil::Can't Find Geom " + bor_id );
        return;
    }
    else if ( geom_ptr->GetType().m_Type != BOR_GEOM_TYPE )
    {
        ErrorMgr.AddError( VSP_WRONG_GEOM_TYPE, "ReadBORFileAirfoil::Geom " + bor_id + " is not a body of revolution" );
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
            ErrorMgr.AddError( VSP_FILE_READ_FAILURE, "ReadBORFileAirfoil::Error reading airfoil file " + file_name );
            return;
        }
    }

    ErrorMgr.AddError( VSP_WRONG_XSEC_TYPE, "ReadBORFileAirfoil::XSec Not XS_FILE_AIRFOIL Type " + bor_id );
    return;
}

void SetBORAirfoilUpperPnts( const std::string& bor_id, const std::vector< vec3d > & up_pnt_vec )
{
    Vehicle* veh = GetVehicle();
    Geom* geom_ptr = veh->FindGeom( bor_id );
    if ( !geom_ptr )
    {
        ErrorMgr.AddError( VSP_INVALID_GEOM_ID, "SetBORAirfoilUpperPnts::Can't Find Geom " + bor_id );
        return;
    }
    else if ( geom_ptr->GetType().m_Type != BOR_GEOM_TYPE )
    {
        ErrorMgr.AddError( VSP_WRONG_GEOM_TYPE, "SetBORAirfoilUpperPnts::Geom " + bor_id + " is not a body of revolution" );
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
        ErrorMgr.AddError( VSP_WRONG_XSEC_TYPE, "SetBORAirfoilUpperPnts::XSec Not XS_FILE_AIRFOIL Type " + bor_id );
        return;
    }

    FileAirfoil* file_xs = dynamic_cast<FileAirfoil*>( xsc );
    assert( file_xs );
    file_xs->SetAirfoilUpperPnts( up_pnt_vec );
    ErrorMgr.NoError();
}

void SetBORAirfoilLowerPnts( const std::string& bor_id, const std::vector< vec3d > & low_pnt_vec )
{
    Vehicle* veh = GetVehicle();
    Geom* geom_ptr = veh->FindGeom( bor_id );
    if ( !geom_ptr )
    {
        ErrorMgr.AddError( VSP_INVALID_GEOM_ID, "SetBORAirfoilLowerPnts::Can't Find Geom " + bor_id );
        return;
    }
    else if ( geom_ptr->GetType().m_Type != BOR_GEOM_TYPE )
    {
        ErrorMgr.AddError( VSP_WRONG_GEOM_TYPE, "SetBORAirfoilLowerPnts::Geom " + bor_id + " is not a body of revolution" );
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
        ErrorMgr.AddError( VSP_WRONG_XSEC_TYPE, "SetBORAirfoilLowerPnts::XSec Not XS_FILE_AIRFOIL Type " + bor_id );
        return;
    }

    FileAirfoil* file_xs = dynamic_cast<FileAirfoil*>( xsc );
    assert( file_xs );
    file_xs->SetAirfoilLowerPnts( low_pnt_vec );
    ErrorMgr.NoError();
}

void SetBORAirfoilPnts( const std::string& bor_id, const std::vector< vec3d > & up_pnt_vec, const std::vector< vec3d > & low_pnt_vec )
{
    Vehicle* veh = GetVehicle();
    Geom* geom_ptr = veh->FindGeom( bor_id );
    if ( !geom_ptr )
    {
        ErrorMgr.AddError( VSP_INVALID_GEOM_ID, "SetBORAirfoilPnts::Can't Find Geom " + bor_id );
        return;
    }
    else if ( geom_ptr->GetType().m_Type != BOR_GEOM_TYPE )
    {
        ErrorMgr.AddError( VSP_WRONG_GEOM_TYPE, "SetBORAirfoilPnts::Geom " + bor_id + " is not a body of revolution" );
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
        ErrorMgr.AddError( VSP_WRONG_XSEC_TYPE, "SetBORAirfoilPnts::XSec Not XS_FILE_AIRFOIL Type " + bor_id );
        return;
    }

    FileAirfoil* file_xs = dynamic_cast<FileAirfoil*>( xsc );
    assert( file_xs );
    file_xs->SetAirfoilPnts( up_pnt_vec, low_pnt_vec );
    ErrorMgr.NoError();
}

std::vector<vec3d> GetBORAirfoilUpperPnts( const std::string& bor_id )
{
    std::vector< vec3d > pnt_vec;
    Vehicle* veh = GetVehicle();
    Geom* geom_ptr = veh->FindGeom( bor_id );
    if ( !geom_ptr )
    {
        ErrorMgr.AddError( VSP_INVALID_GEOM_ID, "GetBORAirfoilUpperPnts::Can't Find Geom " + bor_id );
        return pnt_vec;
    }
    else if ( geom_ptr->GetBehaviorType() != BOR_GEOM_TYPE )
    {
        ErrorMgr.AddError( VSP_WRONG_GEOM_TYPE, "GetBORAirfoilUpperPnts::Geom " + bor_id + " is not a body of revolution" );
        return pnt_vec;
    }

    // Read-only, so a Clone of a body of revolution answers from its original.
    BORGeom* behavior_bor = dynamic_cast< BORGeom* > ( geom_ptr->GetBehaviorGeom() );

    XSecCurve* xsc = behavior_bor->GetXSecCurve();

    if ( !xsc )
    {
        ErrorMgr.AddError( VSP_INVALID_PTR, "GetBORAirfoilUpperPnts::Can't Get XSecCurve" );
        return pnt_vec;
    }

    if ( xsc->GetType() != XS_FILE_AIRFOIL )
    {
        ErrorMgr.AddError( VSP_WRONG_XSEC_TYPE, "GetBORAirfoilUpperPnts::XSec Not XS_FILE_AIRFOIL Type " + bor_id );
        return pnt_vec;
    }

    FileAirfoil* file_xs = dynamic_cast<FileAirfoil*>( xsc );
    assert( file_xs );
    pnt_vec = file_xs->GetUpperPnts();
    ErrorMgr.NoError();
    return pnt_vec;
}

std::vector<vec3d> GetBORAirfoilLowerPnts( const std::string& bor_id )
{
    std::vector< vec3d > pnt_vec;

    Vehicle* veh = GetVehicle();
    Geom* geom_ptr = veh->FindGeom( bor_id );
    if ( !geom_ptr )
    {
        ErrorMgr.AddError( VSP_INVALID_GEOM_ID, "GetBORAirfoilLowerPnts::Can't Find Geom " + bor_id );
        return pnt_vec;
    }
    else if ( geom_ptr->GetBehaviorType() != BOR_GEOM_TYPE )
    {
        ErrorMgr.AddError( VSP_WRONG_GEOM_TYPE, "GetBORAirfoilLowerPnts::Geom " + bor_id + " is not a body of revolution" );
        return pnt_vec;
    }

    // Read-only, so a Clone of a body of revolution answers from its original.
    BORGeom* behavior_bor = dynamic_cast< BORGeom* > ( geom_ptr->GetBehaviorGeom() );

    XSecCurve* xsc = behavior_bor->GetXSecCurve();

    if ( !xsc )
    {
        ErrorMgr.AddError( VSP_INVALID_PTR, "GetBORAirfoilLowerPnts::Can't Get XSecCurve" );
        return pnt_vec;
    }

    if ( xsc->GetType() != XS_FILE_AIRFOIL )
    {
        ErrorMgr.AddError( VSP_WRONG_XSEC_TYPE, "GetBORAirfoilLowerPnts::XSec Not XS_FILE_AIRFOIL Type " + bor_id );
        return pnt_vec;
    }

    FileAirfoil* file_xs = dynamic_cast<FileAirfoil*>( xsc );
    assert( file_xs );
    pnt_vec = file_xs->GetLowerPnts();
    ErrorMgr.NoError();
    return pnt_vec;
}

std::vector<double> GetBORUpperCSTCoefs( const std::string& bor_id )
{
    std::vector < double > ret_vec;

    Vehicle* veh = GetVehicle();
    Geom* geom_ptr = veh->FindGeom( bor_id );
    if ( !geom_ptr )
    {
        ErrorMgr.AddError( VSP_INVALID_GEOM_ID, "GetBORUpperCSTCoefs::Can't Find Geom " + bor_id );
        return ret_vec;
    }
    else if ( geom_ptr->GetBehaviorType() != BOR_GEOM_TYPE )
    {
        ErrorMgr.AddError( VSP_WRONG_GEOM_TYPE, "GetBORUpperCSTCoefs::Geom " + bor_id + " is not a body of revolution" );
        return ret_vec;
    }

    // Read-only, so a Clone of a body of revolution answers from its original.
    BORGeom* behavior_bor = dynamic_cast< BORGeom* > ( geom_ptr->GetBehaviorGeom() );

    XSecCurve* xsc = behavior_bor->GetXSecCurve();

    if ( !xsc )
    {
        ErrorMgr.AddError( VSP_INVALID_PTR, "GetBORUpperCSTCoefs::Can't Get XSecCurve" );
        return ret_vec;
    }


    if ( xsc->GetType() != XS_CST_AIRFOIL )
    {
        ErrorMgr.AddError( VSP_WRONG_XSEC_TYPE, "GetBORUpperCSTCoefs::XSec Not XS_CST_AIRFOIL Type " + bor_id );
        return ret_vec;
    }

    CSTAirfoil* cst_xs = dynamic_cast<CSTAirfoil*>( xsc );
    assert( cst_xs );

    ret_vec = cst_xs->GetUpperCST();
    ErrorMgr.NoError();
    return ret_vec;
}

std::vector<double> GetBORLowerCSTCoefs( const std::string& bor_id )
{
    std::vector < double > ret_vec;

    Vehicle* veh = GetVehicle();
    Geom* geom_ptr = veh->FindGeom( bor_id );
    if ( !geom_ptr )
    {
        ErrorMgr.AddError( VSP_INVALID_GEOM_ID, "GetBORLowerCSTCoefs::Can't Find Geom " + bor_id );
        return ret_vec;
    }
    else if ( geom_ptr->GetBehaviorType() != BOR_GEOM_TYPE )
    {
        ErrorMgr.AddError( VSP_WRONG_GEOM_TYPE, "GetBORLowerCSTCoefs::Geom " + bor_id + " is not a body of revolution" );
        return ret_vec;
    }

    // Read-only, so a Clone of a body of revolution answers from its original.
    BORGeom* behavior_bor = dynamic_cast< BORGeom* > ( geom_ptr->GetBehaviorGeom() );

    XSecCurve* xsc = behavior_bor->GetXSecCurve();

    if ( !xsc )
    {
        ErrorMgr.AddError( VSP_INVALID_PTR, "GetBORLowerCSTCoefs::Can't Get XSecCurve" );
        return ret_vec;
    }

    if ( xsc->GetType() != XS_CST_AIRFOIL )
    {
        ErrorMgr.AddError( VSP_WRONG_XSEC_TYPE, "GetBORLowerCSTCoefs::XSec Not XS_CST_AIRFOIL Type " + bor_id );
        return ret_vec;
    }

    CSTAirfoil* cst_xs = dynamic_cast<CSTAirfoil*>( xsc );
    assert( cst_xs );

    ret_vec = cst_xs->GetLowerCST();
    ErrorMgr.NoError();
    return ret_vec;
}

int GetBORUpperCSTDegree( const std::string& bor_id )
{
    int deg = -1;

    Vehicle* veh = GetVehicle();
    Geom* geom_ptr = veh->FindGeom( bor_id );
    if ( !geom_ptr )
    {
        ErrorMgr.AddError( VSP_INVALID_GEOM_ID, "GetBORUpperCSTDegree::Can't Find Geom " + bor_id );
        return deg;
    }
    else if ( geom_ptr->GetBehaviorType() != BOR_GEOM_TYPE )
    {
        ErrorMgr.AddError( VSP_WRONG_GEOM_TYPE, "GetBORUpperCSTDegree::Geom " + bor_id + " is not a body of revolution" );
        return deg;
    }

    // Read-only, so a Clone of a body of revolution answers from its original.
    BORGeom* behavior_bor = dynamic_cast< BORGeom* > ( geom_ptr->GetBehaviorGeom() );

    XSecCurve* xsc = behavior_bor->GetXSecCurve();

    if ( !xsc )
    {
        ErrorMgr.AddError( VSP_INVALID_PTR, "GetBORUpperCSTDegree::Can't Get XSecCurve" );
        return deg;
    }

    if ( xsc->GetType() != XS_CST_AIRFOIL )
    {
        ErrorMgr.AddError( VSP_WRONG_XSEC_TYPE, "GetBORUpperCSTDegree::XSec Not XS_CST_AIRFOIL Type " + bor_id );
        return deg;
    }

    CSTAirfoil* cst_xs = dynamic_cast<CSTAirfoil*>( xsc );
    assert( cst_xs );

    deg = cst_xs->GetUpperDegree();
    ErrorMgr.NoError();
    return deg;
}

int GetBORLowerCSTDegree( const std::string& bor_id )
{
    int deg = -1;

    Vehicle* veh = GetVehicle();
    Geom* geom_ptr = veh->FindGeom( bor_id );
    if ( !geom_ptr )
    {
        ErrorMgr.AddError( VSP_INVALID_GEOM_ID, "GetBORLowerCSTDegree::Can't Find Geom " + bor_id );
        return deg;
    }
    else if ( geom_ptr->GetBehaviorType() != BOR_GEOM_TYPE )
    {
        ErrorMgr.AddError( VSP_WRONG_GEOM_TYPE, "GetBORLowerCSTDegree::Geom " + bor_id + " is not a body of revolution" );
        return deg;
    }

    // Read-only, so a Clone of a body of revolution answers from its original.
    BORGeom* behavior_bor = dynamic_cast< BORGeom* > ( geom_ptr->GetBehaviorGeom() );

    XSecCurve* xsc = behavior_bor->GetXSecCurve();

    if ( !xsc )
    {
        ErrorMgr.AddError( VSP_INVALID_PTR, "GetBORLowerCSTDegree::Can't Get XSecCurve" );
        return deg;
    }

    if ( xsc->GetType() != XS_CST_AIRFOIL )
    {
        ErrorMgr.AddError( VSP_WRONG_XSEC_TYPE, "GetBORLowerCSTDegree::XSec Not XS_CST_AIRFOIL Type " + bor_id );
        return deg;
    }


    CSTAirfoil* cst_xs = dynamic_cast<CSTAirfoil*>( xsc );
    assert( cst_xs );

    deg = cst_xs->GetLowerDegree();
    ErrorMgr.NoError();
    return deg;
}

void SetBORUpperCST( const std::string& bor_id, int deg, const std::vector<double> &coefs )
{
    Vehicle* veh = GetVehicle();
    Geom* geom_ptr = veh->FindGeom( bor_id );
    if ( !geom_ptr )
    {
        ErrorMgr.AddError( VSP_INVALID_GEOM_ID, "SetBORUpperCST::Can't Find Geom " + bor_id );
        return;
    }
    else if ( geom_ptr->GetType().m_Type != BOR_GEOM_TYPE )
    {
        ErrorMgr.AddError( VSP_WRONG_GEOM_TYPE, "SetBORUpperCST::Geom " + bor_id + " is not a body of revolution" );
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
        ErrorMgr.AddError( VSP_WRONG_XSEC_TYPE, "SetBORUpperCST::XSec Not XS_CST_AIRFOIL Type " + bor_id );
        return;
    }

    CSTAirfoil* cst_xs = dynamic_cast<CSTAirfoil*>( xsc );
    assert( cst_xs );

    ErrorMgr.NoError();
    cst_xs->SetUpperCST( deg, coefs );
}

void SetBORLowerCST( const std::string& bor_id, int deg, const std::vector<double> &coefs )
{
    Vehicle* veh = GetVehicle();
    Geom* geom_ptr = veh->FindGeom( bor_id );
    if ( !geom_ptr )
    {
        ErrorMgr.AddError( VSP_INVALID_GEOM_ID, "SetBORLowerCST::Can't Find Geom " + bor_id );
        return;
    }
    else if ( geom_ptr->GetType().m_Type != BOR_GEOM_TYPE )
    {
        ErrorMgr.AddError( VSP_WRONG_GEOM_TYPE, "SetBORLowerCST::Geom " + bor_id + " is not a body of revolution" );
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
        ErrorMgr.AddError( VSP_WRONG_XSEC_TYPE, "SetBORLowerCST::XSec Not XS_CST_AIRFOIL Type " + bor_id );
        return;
    }

    CSTAirfoil* cst_xs = dynamic_cast<CSTAirfoil*>( xsc );
    assert( cst_xs );

    ErrorMgr.NoError();
    cst_xs->SetLowerCST( deg, coefs );
}

void PromoteBORCSTUpper( const std::string& bor_id )
{
    Vehicle* veh = GetVehicle();
    Geom* geom_ptr = veh->FindGeom( bor_id );
    if ( !geom_ptr )
    {
        ErrorMgr.AddError( VSP_INVALID_GEOM_ID, "PromoteBORCSTUpper::Can't Find Geom " + bor_id );
        return;
    }
    else if ( geom_ptr->GetType().m_Type != BOR_GEOM_TYPE )
    {
        ErrorMgr.AddError( VSP_WRONG_GEOM_TYPE, "PromoteBORCSTUpper::Geom " + bor_id + " is not a body of revolution" );
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
        ErrorMgr.AddError( VSP_WRONG_XSEC_TYPE, "PromoteBORCSTUpper::XSec Not XS_CST_AIRFOIL Type " + bor_id );
        return;
    }

    CSTAirfoil* cst_xs = dynamic_cast<CSTAirfoil*>( xsc );
    assert( cst_xs );

    ErrorMgr.NoError();
    cst_xs->PromoteUpper();
}

void PromoteBORCSTLower( const std::string& bor_id )
{
    Vehicle* veh = GetVehicle();
    Geom* geom_ptr = veh->FindGeom( bor_id );
    if ( !geom_ptr )
    {
        ErrorMgr.AddError( VSP_INVALID_GEOM_ID, "PromoteBORCSTLower::Can't Find Geom " + bor_id );
        return;
    }
    else if ( geom_ptr->GetType().m_Type != BOR_GEOM_TYPE )
    {
        ErrorMgr.AddError( VSP_WRONG_GEOM_TYPE, "PromoteBORCSTLower::Geom " + bor_id + " is not a body of revolution" );
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
        ErrorMgr.AddError( VSP_WRONG_XSEC_TYPE, "PromoteBORCSTLower::XSec Not XS_CST_AIRFOIL Type " + bor_id );
        return;
    }

    CSTAirfoil* cst_xs = dynamic_cast<CSTAirfoil*>( xsc );
    assert( cst_xs );

    ErrorMgr.NoError();
    cst_xs->PromoteLower();
}

void DemoteBORCSTUpper( const std::string& bor_id )
{
    Vehicle* veh = GetVehicle();
    Geom* geom_ptr = veh->FindGeom( bor_id );
    if ( !geom_ptr )
    {
        ErrorMgr.AddError( VSP_INVALID_GEOM_ID, "DemoteBORCSTUpper::Can't Find Geom " + bor_id );
        return;
    }
    else if ( geom_ptr->GetType().m_Type != BOR_GEOM_TYPE )
    {
        ErrorMgr.AddError( VSP_WRONG_GEOM_TYPE, "DemoteBORCSTUpper::Geom " + bor_id + " is not a body of revolution" );
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
        ErrorMgr.AddError( VSP_WRONG_XSEC_TYPE, "DemoteBORCSTUpper::XSec Not XS_CST_AIRFOIL Type " + bor_id );
        return;
    }

    CSTAirfoil* cst_xs = dynamic_cast<CSTAirfoil*>( xsc );
    assert( cst_xs );

    ErrorMgr.NoError();
    cst_xs->DemoteUpper();
}

void DemoteBORCSTLower( const std::string& bor_id )
{
    Vehicle* veh = GetVehicle();
    Geom* geom_ptr = veh->FindGeom( bor_id );
    if ( !geom_ptr )
    {
        ErrorMgr.AddError( VSP_INVALID_GEOM_ID, "DemoteBORCSTLower::Can't Find Geom " + bor_id );
        return;
    }
    else if ( geom_ptr->GetType().m_Type != BOR_GEOM_TYPE )
    {
        ErrorMgr.AddError( VSP_WRONG_GEOM_TYPE, "DemoteBORCSTLower::Geom " + bor_id + " is not a body of revolution" );
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
        ErrorMgr.AddError( VSP_WRONG_XSEC_TYPE, "DemoteBORCSTLower::XSec Not XS_CST_AIRFOIL Type " + bor_id );
        return;
    }

    CSTAirfoil* cst_xs = dynamic_cast<CSTAirfoil*>( xsc );
    assert( cst_xs );

    ErrorMgr.NoError();
    cst_xs->DemoteLower();
}

void FitBORAfCST( const std::string & bor_id, int deg )
{
    Vehicle* veh = GetVehicle();
    Geom* geom_ptr = veh->FindGeom( bor_id );
    if ( !geom_ptr )
    {
        ErrorMgr.AddError( VSP_INVALID_GEOM_ID, "FitBORAfCST::Can't Find Geom " + bor_id );
        return;
    }
    else if ( geom_ptr->GetType().m_Type != BOR_GEOM_TYPE )
    {
        ErrorMgr.AddError( VSP_WRONG_GEOM_TYPE, "FitBORAfCST::Geom " + bor_id + " is not a body of revolution" );
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
        ErrorMgr.AddError( VSP_WRONG_XSEC_TYPE, "FitBORAfCST::XSec Not Fittable Airfoil Type " + bor_id );
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
        ErrorMgr.AddError( VSP_WRONG_XSEC_TYPE, "FitBORAfCST::XSec Not XS_CST_AIRFOIL Type " + bor_id );
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
        ErrorMgr.AddError( VSP_INVALID_XSEC_ID, "EditXSecInitShape::Can't Find XSec " + xsec_id );
        return;
    }

    if ( xs->GetXSecCurve()->GetType() != XS_EDIT_CURVE )
    {
        ErrorMgr.AddError( VSP_WRONG_XSEC_TYPE, "EditXSecInitShape::XSec Not XS_EDIT_CURVE Type " + xsec_id );
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
        ErrorMgr.AddError( VSP_INVALID_XSEC_ID, "EditXSecConvertTo::Can't Find XSec " + xsec_id );
        return;
    }

    if ( xs->GetXSecCurve()->GetType() != XS_EDIT_CURVE )
    {
        ErrorMgr.AddError( VSP_WRONG_XSEC_TYPE, "EditXSecConvertTo::XSec Not XS_EDIT_CURVE Type " + xsec_id );
        return;
    }

    if ( newtype < 0 || newtype > vsp::CEDIT )
    {
        ErrorMgr.AddError( VSP_INVALID_TYPE, "EditXSecConvertTo::Invalid PCURV_TYPE " + to_string( newtype ) );
        return;
    }

    EditCurveXSec* edit_xs = dynamic_cast<EditCurveXSec*>( xs->GetXSecCurve() );
    assert( edit_xs );

    ErrorMgr.NoError();

    edit_xs->ConvertTo( newtype );
}

std::vector < double > GetEditXSecUVec( const std::string& xsec_id )
{
    std::vector < double > ret_vec;

    XSec* xs = FindXSec( xsec_id );
    if ( !xs )
    {
        ErrorMgr.AddError( VSP_INVALID_XSEC_ID, "GetEditXSecUVec::Can't Find XSec " + xsec_id );
        return ret_vec;
    }

    if ( xs->GetXSecCurve()->GetType() != XS_EDIT_CURVE )
    {
        ErrorMgr.AddError( VSP_WRONG_XSEC_TYPE, "GetEditXSecUVec::XSec Not XS_EDIT_CURVE Type " + xsec_id );
        return ret_vec;
    }

    EditCurveXSec* edit_xs = dynamic_cast<EditCurveXSec*>( xs->GetXSecCurve() );
    assert( edit_xs );

    ErrorMgr.NoError();

    return edit_xs->GetUVec();
}

std::vector < vec3d > GetEditXSecCtrlVec( const std::string& xsec_id, bool non_dimensional )
{
    std::vector < vec3d > ret_vec;

    XSec* xs = FindXSec( xsec_id );
    if ( !xs )
    {
        ErrorMgr.AddError( VSP_INVALID_XSEC_ID, "GetEditXSecCtrlVec::Can't Find XSec " + xsec_id );
        return ret_vec;
    }

    if ( xs->GetXSecCurve()->GetType() != XS_EDIT_CURVE )
    {
        ErrorMgr.AddError( VSP_WRONG_XSEC_TYPE, "GetEditXSecCtrlVec::XSec Not XS_EDIT_CURVE Type " + xsec_id );
        return ret_vec;
    }

    EditCurveXSec* edit_xs = dynamic_cast<EditCurveXSec*>( xs->GetXSecCurve() );
    assert( edit_xs );

    ErrorMgr.NoError();

    return edit_xs->GetCtrlPntVec( non_dimensional );
}

void SetEditXSecPnts( const std::string & xsec_id, const std::vector < double > &u_vec, const std::vector < vec3d > &control_pts, const std::vector < double > &r_vec )
{
    XSec* xs = FindXSec( xsec_id );
    if ( !xs )
    {
        ErrorMgr.AddError( VSP_INVALID_XSEC_ID, "SetEditXSecPnts::Can't Find XSec " + xsec_id );
        return;
    }

    if ( xs->GetXSecCurve()->GetType() != XS_EDIT_CURVE )
    {
        ErrorMgr.AddError( VSP_WRONG_XSEC_TYPE, "SetEditXSecPnts::XSec Not XS_EDIT_CURVE Type " + xsec_id );
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
        ErrorMgr.AddError( VSP_INVALID_XSEC_ID, "EditXSecDelPnt::Can't Find XSec " + xsec_id );
        return;
    }

    if ( xs->GetXSecCurve()->GetType() != XS_EDIT_CURVE )
    {
        ErrorMgr.AddError( VSP_WRONG_XSEC_TYPE, "EditXSecDelPnt::XSec Not XS_EDIT_CURVE Type " + xsec_id );
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
        ErrorMgr.AddError( VSP_INVALID_XSEC_ID, "EditXSecSplit01::Can't Find XSec " + xsec_id );
        return -1;
    }

    if ( xs->GetXSecCurve()->GetType() != XS_EDIT_CURVE )
    {
        ErrorMgr.AddError( VSP_WRONG_XSEC_TYPE, "EditXSecSplit01::XSec Not XS_EDIT_CURVE Type " + xsec_id );
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
        ErrorMgr.AddError( VSP_INVALID_XSEC_ID, "MoveEditXSecPnt::Can't Find XSec " + xsec_id );
        return;
    }

    if ( xs->GetXSecCurve()->GetType() != XS_EDIT_CURVE )
    {
        ErrorMgr.AddError( VSP_WRONG_XSEC_TYPE, "MoveEditXSecPnt::XSec Not XS_EDIT_CURVE Type " + xsec_id );
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

static void ConvertXSecToEditImpl( const std::string & geom_id, const int & indx, const std::string & caller )
{
    Vehicle* veh = GetVehicle();
    Geom* geom_ptr = veh->FindGeom( geom_id );
    if ( !geom_ptr )
    {
        ErrorMgr.AddError( VSP_INVALID_GEOM_ID, caller + "::Can't Find Geom " + geom_id );
        return;
    }

    if ( geom_ptr->GetType().m_Type == BOR_GEOM_TYPE )
    {
        BORGeom* bor_ptr = dynamic_cast<BORGeom*> ( geom_ptr );
        if ( !bor_ptr )
        {
            ErrorMgr.AddError( VSP_WRONG_GEOM_TYPE, caller + "::Geom " + geom_id + " is not a body of revolution" );
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
            ErrorMgr.AddError( VSP_WRONG_GEOM_TYPE, caller + "::Geom " + geom_id + " is not a GeomXSec" );
            return;
        }

        XSec* xs = geom_xsec->GetXSec( indx );

        if ( !xs )
        {
            ErrorMgr.AddError( VSP_INDEX_OUT_RANGE, caller + "::XSec Index " + to_string( ( long long )indx ) + " Out of Range" );
            return;
        }

        xs->ConvertToEdit( );
        ErrorMgr.NoError();
        return;
    }
}

void ConvertXSecToEdit( const std::string & geom_id, const int & indx )
{
    ConvertXSecToEditImpl( geom_id, indx, "ConvertXSecToEdit" );
}

void ConvertXSecToEditByID( const std::string & xsec_id )
{
    int index = -1;
    Geom* geom_ptr = FindXSecGeom( xsec_id, index, "ConvertXSecToEditByID" );
    if ( !geom_ptr )
    {
        return;
    }

    ConvertXSecToEditImpl( geom_ptr->GetID(), index, "ConvertXSecToEditByID" );
}

std::vector < bool > GetEditXSecFixedUVec( const std::string& xsec_id )
{
    XSec* xs = FindXSec( xsec_id );
    if ( !xs )
    {
        ErrorMgr.AddError( VSP_INVALID_XSEC_ID, "GetEditXSecFixedUVec::Can't Find XSec " + xsec_id );
        return std::vector < bool > {};
    }

    if ( xs->GetXSecCurve()->GetType() != XS_EDIT_CURVE )
    {
        ErrorMgr.AddError( VSP_WRONG_XSEC_TYPE, "GetEditXSecFixedUVec::XSec Not XS_EDIT_CURVE Type " + xsec_id );
        return std::vector < bool > {};
    }

    EditCurveXSec* edit_xs = dynamic_cast<EditCurveXSec*>( xs->GetXSecCurve() );
    assert( edit_xs );

    ErrorMgr.NoError();
    return edit_xs->GetFixedUVec();
}

void SetEditXSecFixedUVec( const std::string& xsec_id, std::vector < bool > fixed_u_vec )
{
    XSec* xs = FindXSec( xsec_id );
    if ( !xs )
    {
        ErrorMgr.AddError( VSP_INVALID_XSEC_ID, "SetEditXSecFixedUVec::Can't Find XSec " + xsec_id );
        return;
    }

    if ( xs->GetXSecCurve()->GetType() != XS_EDIT_CURVE )
    {
        ErrorMgr.AddError( VSP_WRONG_XSEC_TYPE, "SetEditXSecFixedUVec::XSec Not XS_EDIT_CURVE Type " + xsec_id );
        return;
    }

    EditCurveXSec* edit_xs = dynamic_cast<EditCurveXSec*>( xs->GetXSecCurve() );
    assert( edit_xs );

    if ( edit_xs->GetNumPts() != fixed_u_vec.size() )
    {
        ErrorMgr.AddError( VSP_INVALID_INPUT_VAL, "SetEditXSecFixedUVec::Size of fixed_u_vec " + to_string( ( int )fixed_u_vec.size() ) + " Not Equal to Number of Control Points " + to_string( edit_xs->GetNumPts() ) );
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
        ErrorMgr.AddError( VSP_INVALID_XSEC_ID, "ReparameterizeEditXSec::Can't Find XSec " + xsec_id );
        return;
    }

    if ( xs->GetXSecCurve()->GetType() != XS_EDIT_CURVE )
    {
        ErrorMgr.AddError( VSP_WRONG_XSEC_TYPE, "ReparameterizeEditXSec::XSec Not XS_EDIT_CURVE Type " + xsec_id );
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
    ErrorMgr.NoError();
    return veh->GetSetNameVec().size();
}

// Report a set index past the last set.
static bool SetIndexInRange( int set_index, const std::string & caller )
{
    if ( set_index < 0 || set_index >= ( int )GetVehicle()->GetSetNameVec().size() )
    {
        ErrorMgr.AddError( VSP_INDEX_OUT_RANGE, caller + "::Set Index " + to_string( set_index ) + " Out of Range" );
        return false;
    }
    return true;
}

/// Set the set name at the provided index. Index between 0 and NumSets.
void SetSetName( int index, const std::string& name )
{
    if ( !SetIndexInRange( index, "SetSetName" ) )
    {
        return;
    }

    Vehicle* veh = GetVehicle();
    veh->SetSetName( index, name );
    ErrorMgr.NoError();
}

/// Get the set name at the provided index
std::string GetSetName( int index )
{
    Vehicle* veh = GetVehicle();
    std::vector< std::string > name_vec =  veh->GetSetNameVec();
    if ( index < 0 || index >= ( int )name_vec.size() )
    {
        ErrorMgr.AddError( VSP_INDEX_OUT_RANGE, "GetSetName::Index Out Of Range " + to_string( ( long long )index )  );
        return std::string();
    }
    ErrorMgr.NoError();
    return name_vec[index];
}

/// Get the set of geom ids at the given set index
std::vector< std::string > GetGeomSetAtIndex( int index )
{
    if ( !SetIndexInRange( index, "GetGeomSetAtIndex" ) )
    {
        return std::vector< std::string >();
    }

    Vehicle* veh = GetVehicle();
    ErrorMgr.NoError();
    return veh->GetGeomSet( index );
}

/// Get the set of geom ids at the given set name
std::vector< std::string > GetGeomSet( const std::string & name )
{
    Vehicle* veh = GetVehicle();
    std::vector< std::string > name_vec =  veh->GetSetNameVec();
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
        std::vector< std::string > ret_vec;
        ErrorMgr.AddError( VSP_CANT_FIND_NAME, "GetGeomSet::Can't Find Name " + name );
        return ret_vec;
    }
    ErrorMgr.NoError();
    return veh->GetGeomSet( index );
}

/// Get the set index given the set name
int GetSetIndex( const std::string & name )
{
    Vehicle* veh = GetVehicle();
    std::vector< std::string > name_vec =  veh->GetSetNameVec();
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
bool GetSetFlag( const std::string & geom_id, int set_index )
{
    Vehicle* veh = GetVehicle();
    Geom* geom_ptr = veh->FindGeom( geom_id );
    if ( !geom_ptr )
    {
        ErrorMgr.AddError( VSP_INVALID_GEOM_ID, "GetSetFlag::Can't Find Geom " + geom_id );
        return false;
    }
    if ( !SetIndexInRange( set_index, "GetSetFlag" ) )
    {
        return false;
    }
    ErrorMgr.NoError();
    return geom_ptr->GetSetFlag( set_index );
}

/// Set set flag for geom given set index
void SetSetFlag( const std::string & geom_id, int set_index, bool flag )
{
    Vehicle* veh = GetVehicle();
    Geom* geom_ptr = veh->FindGeom( geom_id );
    if ( !geom_ptr )
    {
        ErrorMgr.AddError( VSP_INVALID_GEOM_ID, "SetSetFlag::Can't Find Geom " + geom_id );
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
    else
    {
        ErrorMgr.AddError( VSP_INVALID_INPUT_VAL, "CopyPasteSet::Sets up to SET_NOT_SHOWN can be neither copied nor pasted over" );
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
        ErrorMgr.AddError( VSP_INVALID_PTR, "GetScaleIndependentBBoxSet::Can't Find Vehicle." );

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

    if ( !SetIndexInRange( set_index, "ScaleSet" ) )
    {
        return;
    }

    std::vector < std::string > geom_id_vec = veh->GetGeomSet( set_index );

    veh->ClearActiveGeom();
    veh->SetActiveGeomVec( geom_id_vec );
    group_trans->ReInitialize();

    group_trans->m_GroupScale.Set( scale );
    group_trans->ParmChanged( nullptr, Parm::SET_FROM_DEVICE );

    veh->ClearActiveGeom();
    group_trans->ReInitialize();
    ErrorMgr.NoError();
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

    if ( !SetIndexInRange( set_index, "RotateSet" ) )
    {
        return;
    }

    std::vector < std::string > geom_id_vec = veh->GetGeomSet( set_index );

    veh->ClearActiveGeom();
    veh->SetActiveGeomVec( geom_id_vec );
    group_trans->ReInitialize();

    group_trans->m_GroupXRot.Set( x_rot_deg );
    group_trans->m_GroupYRot.Set( y_rot_deg );
    group_trans->m_GroupZRot.Set( z_rot_deg );
    group_trans->ParmChanged( nullptr, Parm::SET_FROM_DEVICE );

    veh->ClearActiveGeom();
    group_trans->ReInitialize();
    ErrorMgr.NoError();
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

    if ( !SetIndexInRange( set_index, "TranslateSet" ) )
    {
        return;
    }

    std::vector < std::string > geom_id_vec = veh->GetGeomSet( set_index );

    veh->ClearActiveGeom();
    veh->SetActiveGeomVec( geom_id_vec );
    group_trans->ReInitialize();

    group_trans->m_GroupXLoc.Set( translation_vec.x() );
    group_trans->m_GroupYLoc.Set( translation_vec.y() );
    group_trans->m_GroupZLoc.Set( translation_vec.z() );
    group_trans->ParmChanged( nullptr, Parm::SET_FROM_DEVICE );

    veh->ClearActiveGeom();
    group_trans->ReInitialize();
    ErrorMgr.NoError();
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

    if ( !SetIndexInRange( set_index, "TransformSet" ) )
    {
        return;
    }

    std::vector < std::string > geom_id_vec = veh->GetGeomSet( set_index );

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
    ErrorMgr.NoError();
}

//===================================================================//
//===============       Parm Functions            ===================//
//===================================================================//

/// Check if the given parm id is valid
bool ValidParm( const std::string & parm_id )
{
    ErrorMgr.NoError();

    Parm* p = ParmMgr.FindParm( parm_id );
    if ( !p )
    {
        return false;
    }

    return true;
}

/// Set the parm value.
/// The final value of parm is returned.
double SetParmVal( const std::string & parm_id, double val )
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
double SetParmVal( const std::string & container_id, const std::string & name, const std::string & group, double val )
{
    std::string parm_id = FindParmImpl( container_id, name, group, "SetParmVal" );
    Parm* p = ParmMgr.FindParm( parm_id );
    if ( !p )
    {
        ErrorMgr.AddError( VSP_CANT_FIND_PARM, "SetParmVal::Can't Find Parm " + container_id + ":" + group + ":" + name  );
        return val;
    }
    ErrorMgr.NoError();
    return p->Set( val );
}

double SetParmValLimits( const std::string & parm_id, double val, double lower_limit, double upper_limit )
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
double SetParmValUpdate( const std::string & parm_id, double val )
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
double SetParmValUpdate( const std::string & container_id, const std::string & parm_name, const std::string & parm_group_name, double val )
{
    std::string parm_id = FindParmImpl( container_id, parm_name, parm_group_name, "SetParmValUpdate" );
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
double GetParmVal( const std::string & parm_id )
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
double GetParmVal( const std::string & container_id, const std::string & name, const std::string & group )
{
    std::string parm_id = FindParmImpl( container_id, name, group, "GetParmVal" );
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
int GetIntParmVal( const std::string & parm_id )
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
bool GetBoolParmVal( const std::string & parm_id )
{
    Parm* p = ParmMgr.FindParm( parm_id );
    if ( !p )
    {
        ErrorMgr.AddError( VSP_CANT_FIND_PARM, "GetBoolParmVal::Can't Find Parm " + parm_id );
        return false;
    }
    if ( p->GetType() != PARM_BOOL_TYPE )
    {
        ErrorMgr.AddError( VSP_INVALID_TYPE, "GetBoolParmVal::Parm " + parm_id + " is not a BoolParm" );
        return false;
    }
    ErrorMgr.NoError();

    BoolParm* bp = dynamic_cast<BoolParm*>( p );
    return bp->Get();
}

/// Set the upper limit of parm
void SetParmUpperLimit( const std::string & parm_id, double val )
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
double GetParmUpperLimit( const std::string & parm_id )
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
void SetParmLowerLimit( const std::string & parm_id, double val )
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
double GetParmLowerLimit( const std::string & parm_id )
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
int GetParmType( const std::string & parm_id )
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
std::string GetParmName( const std::string & parm_id )
{
    Parm* p = ParmMgr.FindParm( parm_id );
    if ( !p )
    {
        ErrorMgr.AddError( VSP_CANT_FIND_PARM, "GetParmName::Can't Find Parm " + parm_id );
        return std::string();
    }
    ErrorMgr.NoError();
    return p->GetName();
}

/// Get the parm group name
std::string GetParmGroupName( const std::string & parm_id )
{
    Parm* p = ParmMgr.FindParm( parm_id );
    if ( !p )
    {
        ErrorMgr.AddError( VSP_CANT_FIND_PARM, "GetParmGroupName::Can't Find Parm " + parm_id );
        return std::string();
    }
    ErrorMgr.NoError();
    return p->GetGroupName();
}

/// Get the parm display group name
std::string GetParmDisplayGroupName( const std::string & parm_id )
{
    Parm* p = ParmMgr.FindParm( parm_id );
    if ( !p )
    {
        ErrorMgr.AddError( VSP_CANT_FIND_PARM, "GetParmDisplayGroupName::Can't Find Parm " + parm_id );
        return std::string();
    }
    ErrorMgr.NoError();
    return p->GetDisplayGroupName();
}

/// The the parm container id
std::string GetParmContainer( const std::string & parm_id )
{
    Parm* p = ParmMgr.FindParm( parm_id );
    if ( !p )
    {
        ErrorMgr.AddError( VSP_CANT_FIND_PARM, "GetParmContainer::Can't Find Parm " + parm_id );
        return std::string();
    }
    ErrorMgr.NoError();
    return p->GetContainerID();
}

/// Set the parm description
void SetParmDescript( const std::string & parm_id, const std::string & desc )
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
std::string GetParmDescript( const std::string & parm_id )
{
    Parm* p = ParmMgr.FindParm( parm_id );
    if ( !p )
    {
        ErrorMgr.AddError( VSP_CANT_FIND_PARM, "GetParmDescript::Can't Find Parm " + parm_id );
        return std::string();
    }
    ErrorMgr.NoError();
    return p->GetDescript();
}

///  Find a parm id given parm container, name and group
static std::string FindParmImpl( const std::string & parm_container_id, const std::string& parm_name, const std::string& group_name, const std::string & caller )
{
    if ( ParmMgr.GetDirtyFlag() )
    {
        LinkMgr.BuildLinkableParmData();        // Make Sure Name/Group Get Mapped To Parms
    }

    ParmContainer* pc = ParmMgr.FindParmContainer( parm_container_id );

    if ( !pc )
    {
        ErrorMgr.AddError( VSP_INVALID_ID, caller + "::Can't Find Parm Container " + parm_container_id );
        return std::string();
    }

   std::string parm_id = pc->FindParm( parm_name, group_name );
   Parm* p = ParmMgr.FindParm( parm_id );
   if ( !p )
   {
        ErrorMgr.AddError( VSP_CANT_FIND_PARM, caller + "::Can't Find Parm " + parm_name + " " + group_name );
        return std::string();
   }
   ErrorMgr.NoError();

   return parm_id;
}

std::string FindParm( const std::string & parm_container_id, const std::string& parm_name, const std::string& group_name )
{
    return FindParmImpl( parm_container_id, parm_name, group_name, "FindParm" );
}


//===================================================================//
//===============       Parm Container Functions       ==============//
//===================================================================//

std::vector< std::string > FindContainers()
{
    std::vector< std::string > containerVec;
    if ( ParmMgr.GetDirtyFlag() )
    {
        LinkMgr.BuildLinkableParmData();
    }
    LinkMgr.GetAllContainerVec( containerVec );

    ErrorMgr.NoError();
    return containerVec;
}

std::vector< std::string > FindContainersWithName( const std::string & name )
{
    std::vector< std::string > containerVec;
    std::vector< std::string > ret_vec;
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

std::string FindContainer( const std::string & name, int index )
{
    std::vector< std::string > containerVec;
    std::vector< std::string > id_vec;
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
        ErrorMgr.AddError( VSP_INDEX_OUT_RANGE, "FindContainer::Index " + to_string( ( long long )index ) + " Out of Range for " + to_string( ( int )id_vec.size() ) + " Containers Named " + name );
        return std::string();
    }
    ErrorMgr.NoError();
    return id_vec[index];
}

void SetContainerName( const std::string & parm_container_id, const std::string & name )
{
    ParmContainer* pc = ParmMgr.FindParmContainer( parm_container_id );

    if ( !pc )
    {
        ErrorMgr.AddError( VSP_INVALID_ID, "SetContainerName::Can't Find Parm Container " + parm_container_id );
        return;
    }

    // A self-naming Geom refuses here too, as in SetGeomName.
    Geom* geom_ptr = dynamic_cast< Geom* >( pc );
    if ( geom_ptr && geom_ptr->NameIsAutomatic() )
    {
        ErrorMgr.AddError( VSP_WRONG_GEOM_TYPE, "SetContainerName::Geom " + parm_container_id +
                           " names itself and would write over this on its next update" );
        return;
    }

    // SetName is virtual, so a container that does more than store the string
    // -- a Geom updating its attributes, for instance -- still does it.
    pc->SetName( name );

    ErrorMgr.NoError();
}

std::string GetContainerName( const std::string & parm_container_id )
{
    std::string ret_name;

    ParmContainer* pc = ParmMgr.FindParmContainer( parm_container_id );

    if ( !pc )
    {
        ErrorMgr.AddError( VSP_INVALID_ID, "GetContainerName::Can't Find Parm Container " + parm_container_id );
        return std::string();
    }

    ret_name = pc->GetName();
    ErrorMgr.NoError();
    return ret_name;
}

std::vector< std::string > FindContainerGroupNames( const std::string & parm_container_id )
{
    std::vector< std::string > ret_names;

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

std::vector< std::string > FindContainerParmIDs( const std::string & parm_container_id )
{
    std::vector< std::string > parm_vec;

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

std::string GetVehicleID()
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

std::vector < std::string > GetAllUserParms()
{
    std::vector < std::string > ret = LinkMgr.GetAllUserParms();

    ErrorMgr.NoError();

    return ret;
}

std::string GetUserParmContainer()
{
    ParmContainer *pc = LinkMgr.GetUserParmContainer();

    if ( !pc )
    {
        ErrorMgr.AddError( VSP_INVALID_PTR, "GetUserParmContainer::Invalid UserParmContainer " );
        return std::string();
    }

    ErrorMgr.NoError();

    return pc->GetID();
}

std::string AddUserParm(int type, const std::string & name, const std::string & group )
{
    std::string ret = LinkMgr.AddUserParm( type, name, group );

    ErrorMgr.NoError();

    return ret;
}

void DeleteUserParm( const std::string & id )
{
    int indx = LinkMgr.GetUserParmIndex( id );

    if ( indx < 0 )
    {
        ErrorMgr.AddError( VSP_CANT_FIND_PARM, "DeleteUserParm::Can't Find User Parm " + id );
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
double ComputeMinClearanceDistance( const std::string & geom_id, int set, bool useMode, const std::string &modeID )
{
    Vehicle* veh = GetVehicle();

    if ( !veh->FindGeom( geom_id ) )
    {
        ErrorMgr.AddError( VSP_INVALID_GEOM_ID, "ComputeMinClearanceDistance::Can't Find Geom " + geom_id );
        return 0.0;
    }

    int old_set = veh->GetSnapToPtr()->m_CollisionSet;
    veh->GetSnapToPtr()->m_CollisionSet = set;

    bool old_useMode = veh->GetSnapToPtr()->m_UseMode();
    veh->GetSnapToPtr()->m_UseMode = useMode;

    std::string old_modeID = veh->GetSnapToPtr()->m_ModeID;
    veh->GetSnapToPtr()->m_ModeID = modeID;

    std::vector< std::string > old_active_geom = veh->GetActiveGeomVec();
    veh->SetActiveGeom( geom_id );

    veh->GetSnapToPtr()->CheckClearance();
    double min_clearance_dist = veh->GetSnapToPtr()->m_CollisionMinDist;

    //==== Restore State ====//
    veh->GetSnapToPtr()->m_CollisionSet = old_set;
    veh->GetSnapToPtr()->m_UseMode = old_useMode;
    veh->GetSnapToPtr()->m_ModeID = old_modeID;
    veh->SetActiveGeomVec( old_active_geom );

    ErrorMgr.NoError();
    return min_clearance_dist;
}

double SnapParm( const std::string & parm_id, double target_min_dist, bool inc_flag, int set, bool useMode, const std::string &modeID )
{
    Vehicle* veh = GetVehicle();

    if ( !ParmMgr.FindParm( parm_id ) )
    {
        ErrorMgr.AddError( VSP_CANT_FIND_PARM, "SnapParm::Can't Find Parm " + parm_id );
        return 0.0;
    }

    int old_set = veh->GetSnapToPtr()->m_CollisionSet;
    veh->GetSnapToPtr()->m_CollisionSet = set;

    bool old_useMode = veh->GetSnapToPtr()->m_UseMode();
    veh->GetSnapToPtr()->m_UseMode = useMode;

    std::string old_modeID = veh->GetSnapToPtr()->m_ModeID;
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

    ErrorMgr.NoError();
    return min_clearance_dist;
}

//===================================================================//
//=======================  Fit Model Functions  =====================//
//===================================================================//

// Look up a target point by index, reporting the error the API reports for a bad index.  Every
// function here that takes an index goes through this so they all fail the same way.
static TargetPt * FindFitModelTargetPt( int index, const std::string & routine )
{
    if ( index < 0 || index >= FitModelMgr.GetNumTargetPt() )
    {
        ErrorMgr.AddError( VSP_INDEX_OUT_RANGE, routine + "::Index Out Of Range " + to_string( index ) );
        return nullptr;
    }

    TargetPt *tpt = FitModelMgr.GetTargetPt( index );

    if ( !tpt )
    {
        ErrorMgr.AddError( VSP_INVALID_PTR, routine + "::Can't Find Target Point " + to_string( index ) );
        return nullptr;
    }

    return tpt;
}

// A target point is matched against the first surface of a Geom, so a Geom that has no surfaces
// -- a Blank, a point cloud -- cannot carry one.  Nothing downstream checks, and the optimizer
// would read through the null surface pointer.
static Geom * FindFitModelMatchGeom( const std::string & geom_id, int surf_indx, const std::string & routine )
{
    Vehicle* veh = GetVehicle();
    Geom* geom_ptr = veh->FindGeom( geom_id );

    if ( !geom_ptr )
    {
        ErrorMgr.AddError( VSP_INVALID_GEOM_ID, routine + "::Can't Find Geom " + geom_id );
        return nullptr;
    }

    if ( geom_ptr->GetNumTotalSurfs() < 1 )
    {
        ErrorMgr.AddError( VSP_WRONG_GEOM_TYPE, routine + "::Geom Has No Surface " + geom_id );
        return nullptr;
    }

    if ( surf_indx < 0 || surf_indx >= geom_ptr->GetNumTotalSurfs() )
    {
        ErrorMgr.AddError( VSP_INDEX_OUT_RANGE, routine + "::Surface Index Out Of Range " + to_string( surf_indx ) +
                           ", " + geom_id + " has " + to_string( geom_ptr->GetNumTotalSurfs() ) + " surfaces" );
        return nullptr;
    }

    return geom_ptr;
}

static bool CheckFitModelTargetType( int type, const std::string & routine )
{
    if ( type != FIT_MODEL_FIXED && type != FIT_MODEL_FREE )
    {
        ErrorMgr.AddError( VSP_INVALID_TYPE, routine + "::Invalid Fit Model Target Type " + to_string( type ) );
        return false;
    }
    return true;
}

void ResetFitModel()
{
    FitModelMgr.Renew();
    ErrorMgr.NoError();
}

// One place where a single target point is built, so each of the four ways in reports under its
// own name.  Mirrors AddFitModelTargetPtGroup for the vector forms.
static int AddFitModelTargetPtOne( const std::string & geom_id, int surf_indx, const vec3d & pt, int u_type, int w_type,
                                   double u, double w, const std::string & routine )
{
    Geom* geom_ptr = FindFitModelMatchGeom( geom_id, surf_indx, routine );
    if ( !geom_ptr )
    {
        return -1;
    }

    if ( !CheckFitModelTargetType( u_type, routine ) ||
         !CheckFitModelTargetType( w_type, routine ) )
    {
        return -1;
    }

    TargetPt *tpt = new TargetPt();
    tpt->SetPt( pt );
    tpt->SetMatchGeom( geom_id );
    tpt->SetSurfIndx( surf_indx );
    tpt->SetUW( vec2d( u, w ) );
    tpt->SetUType( u_type );
    tpt->SetWType( w_type );

    // Only moves the free directions; with both pinned this does nothing.
    tpt->SearchUW( geom_ptr );

    FitModelMgr.AddTargetPt( tpt );

    ErrorMgr.NoError();
    return FitModelMgr.GetNumTargetPt() - 1;
}

int AddFitModelTargetPt( const std::string & geom_id, int surf_indx, const vec3d & pt, int u_type, int w_type, double u, double w )
{
    return AddFitModelTargetPtOne( geom_id, surf_indx, pt, u_type, w_type, u, w, "AddFitModelTargetPt" );
}

int AddFitModelTargetPtFixedU( const std::string & geom_id, int surf_indx, const vec3d & pt, double u )
{
    return AddFitModelTargetPtOne( geom_id, surf_indx, pt, FIT_MODEL_FIXED, FIT_MODEL_FREE, u, 0.0, "AddFitModelTargetPtFixedU" );
}

int AddFitModelTargetPtFixedW( const std::string & geom_id, int surf_indx, const vec3d & pt, double w )
{
    return AddFitModelTargetPtOne( geom_id, surf_indx, pt, FIT_MODEL_FREE, FIT_MODEL_FIXED, 0.0, w, "AddFitModelTargetPtFixedW" );
}

int AddFitModelTargetPtFixedUW( const std::string & geom_id, int surf_indx, const vec3d & pt, double u, double w )
{
    return AddFitModelTargetPtOne( geom_id, surf_indx, pt, FIT_MODEL_FIXED, FIT_MODEL_FIXED, u, w, "AddFitModelTargetPtFixedUW" );
}

// One place where a group of target points is built.  u_vec and w_vec, when given, carry a
// coordinate per point; otherwise u and w are used for every point.  A pinned direction is left
// where it is put.  A free one is searched onto the nearest point of the surface, because the
// topology can tell a caller where a point sits in a pinned direction but seldom in a free one,
// and a coordinate left at zero would otherwise pin the search to a corner of the surface.
static void AddFitModelTargetPtGroup( const std::string & geom_id, int surf_indx, const std::vector < vec3d > & pt_vec,
                                      int u_type, int w_type, double u, double w,
                                      const std::vector < double > & u_vec, const std::vector < double > & w_vec,
                                      const std::string & routine )
{
    Geom* geom_ptr = FindFitModelMatchGeom( geom_id, surf_indx, routine );
    if ( !geom_ptr )
    {
        return;
    }

    if ( !u_vec.empty() && u_vec.size() != pt_vec.size() )
    {
        ErrorMgr.AddError( VSP_INVALID_INPUT_VAL, routine + "::U Vector Length " + to_string( ( int )u_vec.size() ) +
                           " Does Not Match Point Vector Length " + to_string( ( int )pt_vec.size() ) );
        return;
    }

    if ( !w_vec.empty() && w_vec.size() != pt_vec.size() )
    {
        ErrorMgr.AddError( VSP_INVALID_INPUT_VAL, routine + "::W Vector Length " + to_string( ( int )w_vec.size() ) +
                           " Does Not Match Point Vector Length " + to_string( ( int )pt_vec.size() ) );
        return;
    }

    for ( int i = 0; i < ( int )pt_vec.size(); i++ )
    {
        double ui = u;
        if ( !u_vec.empty() )
        {
            ui = u_vec[i];
        }

        double wi = w;
        if ( !w_vec.empty() )
        {
            wi = w_vec[i];
        }

        TargetPt *tpt = new TargetPt();
        tpt->SetPt( pt_vec[i] );
        tpt->SetMatchGeom( geom_id );
        tpt->SetSurfIndx( surf_indx );
        tpt->SetUW( vec2d( ui, wi ) );
        tpt->SetUType( u_type );
        tpt->SetWType( w_type );

        // Only moves the free directions; with both pinned this does nothing.
        tpt->SearchUW( geom_ptr );

        FitModelMgr.AddTargetPt( tpt );
    }

    ErrorMgr.NoError();
}

void AddFitModelTargetPts( const std::string & geom_id, int surf_indx, const std::vector < vec3d > & pt_vec )
{
    AddFitModelTargetPtGroup( geom_id, surf_indx, pt_vec, FIT_MODEL_FREE, FIT_MODEL_FREE, 0.0, 0.0,
                              std::vector < double >(), std::vector < double >(), "AddFitModelTargetPts" );
}

void AddFitModelTargetPtsFixedU( const std::string & geom_id, int surf_indx, const std::vector < vec3d > & pt_vec, double u )
{
    AddFitModelTargetPtGroup( geom_id, surf_indx, pt_vec, FIT_MODEL_FIXED, FIT_MODEL_FREE, u, 0.0,
                              std::vector < double >(), std::vector < double >(), "AddFitModelTargetPtsFixedU" );
}

void AddFitModelTargetPtsFixedUs( const std::string & geom_id, int surf_indx, const std::vector < vec3d > & pt_vec, const std::vector < double > & u_vec )
{
    AddFitModelTargetPtGroup( geom_id, surf_indx, pt_vec, FIT_MODEL_FIXED, FIT_MODEL_FREE, 0.0, 0.0,
                              u_vec, std::vector < double >(), "AddFitModelTargetPtsFixedUs" );
}

void AddFitModelTargetPtsFixedW( const std::string & geom_id, int surf_indx, const std::vector < vec3d > & pt_vec, double w )
{
    AddFitModelTargetPtGroup( geom_id, surf_indx, pt_vec, FIT_MODEL_FREE, FIT_MODEL_FIXED, 0.0, w,
                              std::vector < double >(), std::vector < double >(), "AddFitModelTargetPtsFixedW" );
}

void AddFitModelTargetPtsFixedWs( const std::string & geom_id, int surf_indx, const std::vector < vec3d > & pt_vec, const std::vector < double > & w_vec )
{
    AddFitModelTargetPtGroup( geom_id, surf_indx, pt_vec, FIT_MODEL_FREE, FIT_MODEL_FIXED, 0.0, 0.0,
                              std::vector < double >(), w_vec, "AddFitModelTargetPtsFixedWs" );
}

void AddFitModelTargetPtsFixedUW( const std::string & geom_id, int surf_indx, const std::vector < vec3d > & pt_vec, double u, double w )
{
    AddFitModelTargetPtGroup( geom_id, surf_indx, pt_vec, FIT_MODEL_FIXED, FIT_MODEL_FIXED, u, w,
                              std::vector < double >(), std::vector < double >(), "AddFitModelTargetPtsFixedUW" );
}

void AddFitModelTargetPtsFixedUWs( const std::string & geom_id, int surf_indx, const std::vector < vec3d > & pt_vec, const std::vector < double > & u_vec, const std::vector < double > & w_vec )
{
    AddFitModelTargetPtGroup( geom_id, surf_indx, pt_vec, FIT_MODEL_FIXED, FIT_MODEL_FIXED, 0.0, 0.0,
                              u_vec, w_vec, "AddFitModelTargetPtsFixedUWs" );
}

void DelFitModelTargetPt( int index )
{
    if ( index < 0 || index >= FitModelMgr.GetNumTargetPt() )
    {
        ErrorMgr.AddError( VSP_INDEX_OUT_RANGE, "DelFitModelTargetPt::Index Out Of Range " + to_string( index ) );
        return;
    }

    // The manager deletes whichever point it currently calls current, so say which that is.  It
    // clears the selection afterwards, which is left alone here: the points above the deleted one
    // have moved down, so any index held from before now names a different point.
    FitModelMgr.SetCurrTargetPtIndex( index );
    FitModelMgr.DelCurrTargetPt();

    ErrorMgr.NoError();
}

void DelAllFitModelTargetPts()
{
    FitModelMgr.DelAllTargetPts();
    ErrorMgr.NoError();
}

void SortFitModelTargetPtsByDist()
{
    FitModelMgr.SortTargetPtsByDist();
    ErrorMgr.NoError();
}

int MoveFitModelTargetPt( int index, int reorder_type )
{
    if ( index < 0 || index >= FitModelMgr.GetNumTargetPt() )
    {
        ErrorMgr.AddError( VSP_INDEX_OUT_RANGE, "MoveFitModelTargetPt::index " + to_string( index ) + " is out of range" );
        return index;
    }

    if ( reorder_type < REORDER_MOVE_UP || reorder_type > REORDER_MOVE_BOTTOM )
    {
        ErrorMgr.AddError( VSP_INVALID_TYPE, "MoveFitModelTargetPt::Invalid Reorder Type " + to_string( reorder_type ) );
        return index;
    }

    int newindex = FitModelMgr.MoveTargetPt( index, reorder_type );

    ErrorMgr.NoError();
    return newindex;
}

int GetNumFitModelTargetPts()
{
    ErrorMgr.NoError();
    return FitModelMgr.GetNumTargetPt();
}

vec3d GetFitModelTargetPt( int index )
{
    TargetPt *tpt = FindFitModelTargetPt( index, "GetFitModelTargetPt" );
    if ( !tpt )
    {
        return vec3d();
    }

    ErrorMgr.NoError();
    return tpt->GetPt();
}

void SetFitModelTargetPt( int index, const vec3d & pt )
{
    TargetPt *tpt = FindFitModelTargetPt( index, "SetFitModelTargetPt" );
    if ( !tpt )
    {
        return;
    }

    tpt->SetPt( pt );
    ErrorMgr.NoError();
}

std::string GetFitModelTargetPtGeom( int index )
{
    TargetPt *tpt = FindFitModelTargetPt( index, "GetFitModelTargetPtGeom" );
    if ( !tpt )
    {
        return std::string();
    }

    ErrorMgr.NoError();
    return tpt->GetMatchGeom();
}

void SetFitModelTargetPtGeom( int index, const std::string & geom_id )
{
    TargetPt *tpt = FindFitModelTargetPt( index, "SetFitModelTargetPtGeom" );
    if ( !tpt )
    {
        return;
    }

    // The index has to be in range on the Geom being moved to, not the one being left.
    if ( !FindFitModelMatchGeom( geom_id, tpt->GetSurfIndx(), "SetFitModelTargetPtGeom" ) )
    {
        return;
    }

    tpt->SetMatchGeom( geom_id );
    ErrorMgr.NoError();
}

int GetFitModelTargetPtSurfIndx( int index )
{
    TargetPt *tpt = FindFitModelTargetPt( index, "GetFitModelTargetPtSurfIndx" );
    if ( !tpt )
    {
        return -1;
    }

    ErrorMgr.NoError();
    return tpt->GetSurfIndx();
}

void SetFitModelTargetPtSurfIndx( int index, int surf_indx )
{
    TargetPt *tpt = FindFitModelTargetPt( index, "SetFitModelTargetPtSurfIndx" );
    if ( !tpt )
    {
        return;
    }

    if ( !FindFitModelMatchGeom( tpt->GetMatchGeom(), surf_indx, "SetFitModelTargetPtSurfIndx" ) )
    {
        return;
    }

    tpt->SetSurfIndx( surf_indx );
    ErrorMgr.NoError();
}

double GetFitModelTargetPtU( int index )
{
    TargetPt *tpt = FindFitModelTargetPt( index, "GetFitModelTargetPtU" );
    if ( !tpt )
    {
        return 0.0;
    }

    ErrorMgr.NoError();
    return tpt->GetUW().x();
}

double GetFitModelTargetPtDist( int index )
{
    TargetPt *tpt = FindFitModelTargetPt( index, "GetFitModelTargetPtDist" );
    if ( !tpt )
    {
        return 0.0;
    }

    ErrorMgr.NoError();
    return tpt->GetDist();
}

double GetFitModelTargetPtW( int index )
{
    TargetPt *tpt = FindFitModelTargetPt( index, "GetFitModelTargetPtW" );
    if ( !tpt )
    {
        return 0.0;
    }

    ErrorMgr.NoError();
    return tpt->GetUW().y();
}

void SetFitModelTargetPtUW( int index, double u, double w )
{
    TargetPt *tpt = FindFitModelTargetPt( index, "SetFitModelTargetPtUW" );
    if ( !tpt )
    {
        return;
    }

    tpt->SetUW( vec2d( u, w ) );
    ErrorMgr.NoError();
}

int GetFitModelTargetPtUType( int index )
{
    TargetPt *tpt = FindFitModelTargetPt( index, "GetFitModelTargetPtUType" );
    if ( !tpt )
    {
        return FIT_MODEL_FIXED;
    }

    ErrorMgr.NoError();
    return tpt->GetUType();
}

void SetFitModelTargetPtUType( int index, int u_type )
{
    TargetPt *tpt = FindFitModelTargetPt( index, "SetFitModelTargetPtUType" );
    if ( !tpt )
    {
        return;
    }

    if ( !CheckFitModelTargetType( u_type, "SetFitModelTargetPtUType" ) )
    {
        return;
    }

    tpt->SetUType( u_type );
    ErrorMgr.NoError();
}

int GetFitModelTargetPtWType( int index )
{
    TargetPt *tpt = FindFitModelTargetPt( index, "GetFitModelTargetPtWType" );
    if ( !tpt )
    {
        return FIT_MODEL_FIXED;
    }

    ErrorMgr.NoError();
    return tpt->GetWType();
}

void SetFitModelTargetPtWType( int index, int w_type )
{
    TargetPt *tpt = FindFitModelTargetPt( index, "SetFitModelTargetPtWType" );
    if ( !tpt )
    {
        return;
    }

    if ( !CheckFitModelTargetType( w_type, "SetFitModelTargetPtWType" ) )
    {
        return;
    }

    tpt->SetWType( w_type );
    ErrorMgr.NoError();
}

vec3d GetFitModelTargetPtSurfPt( int index )
{
    TargetPt *tpt = FindFitModelTargetPt( index, "GetFitModelTargetPtSurfPt" );
    if ( !tpt )
    {
        return vec3d();
    }

    if ( !tpt->IsValid() )
    {
        ErrorMgr.AddError( VSP_INVALID_GEOM_ID, "GetFitModelTargetPtSurfPt::Can't Find Surface For Geom " + tpt->GetMatchGeom() );
        return vec3d();
    }

    ErrorMgr.NoError();
    return tpt->GetMatchPt();
}

void AddFitModelVar( const std::string & parm_id )
{
    Parm* p = ParmMgr.FindParm( parm_id );
    if ( !p )
    {
        ErrorMgr.AddError( VSP_CANT_FIND_PARM, "AddFitModelVar::Can't Find Parm " + parm_id );
        return;
    }

    if ( !FitModelMgr.AddVar( parm_id ) )
    {
        ErrorMgr.AddError( VSP_INVALID_INPUT_VAL, "AddFitModelVar::Duplicate Variable " + parm_id );
        return;
    }

    ErrorMgr.NoError();
}

void DelFitModelVar( const std::string & parm_id )
{
    if ( !FitModelMgr.CheckForDuplicateVar( parm_id ) )
    {
        ErrorMgr.AddError( VSP_CANT_FIND_PARM, "DelFitModelVar::Can't Find Variable " + parm_id );
        return;
    }

    FitModelMgr.DelVar( parm_id );
    ErrorMgr.NoError();
}

void DelAllFitModelVars()
{
    FitModelMgr.DelAllVars();
    ErrorMgr.NoError();
}

int GetNumFitModelVars()
{
    ErrorMgr.NoError();
    return FitModelMgr.GetNumVars();
}

std::string GetFitModelVar( int index )
{
    if ( index < 0 || index >= FitModelMgr.GetNumVars() )
    {
        ErrorMgr.AddError( VSP_INDEX_OUT_RANGE, "GetFitModelVar::Index Out Of Range " + to_string( index ) );
        return std::string();
    }

    ErrorMgr.NoError();
    return FitModelMgr.GetVar( index );
}

std::vector < std::string > GetFitModelVarVec()
{
    ErrorMgr.NoError();
    return FitModelMgr.GetVarVec();
}

void SearchFitModelTargetUW()
{
    FitModelMgr.SearchTargetUW();
    ErrorMgr.NoError();
}

void RefineFitModelTargetUW()
{
    FitModelMgr.RefineTargetUW();
    ErrorMgr.NoError();
}

double UpdateFitModelDist()
{
    // Zero is a real answer here, and the wanted one, so it cannot also stand for having nothing
    // to measure.  The manager reports -1 for that.
    if ( FitModelMgr.GetNumTargetPt() == 0 )
    {
        FitModelMgr.UpdateDist();
        ErrorMgr.AddError( VSP_INVALID_INPUT_VAL, "UpdateFitModelDist::No Target Points" );
        return -1.0;
    }

    FitModelMgr.UpdateDist();
    ErrorMgr.NoError();
    return FitModelMgr.m_DistMetric;
}

double GetFitModelDist()
{
    ErrorMgr.NoError();
    return FitModelMgr.m_DistMetric;
}

int GetNumFitModelOptVars()
{
    FitModelMgr.UpdateNumOptVars();
    ErrorMgr.NoError();
    return FitModelMgr.GetNumOptVars();
}

int OptimizeFitModel()
{
    if ( FitModelMgr.GetNumTargetPt() == 0 )
    {
        ErrorMgr.AddError( VSP_INVALID_INPUT_VAL, "OptimizeFitModel::No Target Points" );
        return 0;
    }

    FitModelMgr.UpdateNumOptVars();
    int n = FitModelMgr.GetNumOptVars();
    if ( n == 0 )
    {
        ErrorMgr.AddError( VSP_INVALID_INPUT_VAL, "OptimizeFitModel::No Variables Or Free Target Points" );
        return 0;
    }

    // Least squares needs at least as many conditions as unknowns.  Each target point supplies
    // three.  Given fewer, the solver returns its invalid-input code and changes nothing, which on
    // its own does not say which way the problem was underdetermined.
    int m = 3 * FitModelMgr.GetNumTargetPt();
    if ( m < n )
    {
        ErrorMgr.AddError( VSP_INVALID_INPUT_VAL, "OptimizeFitModel::Too Few Target Points, " +
                           to_string( m ) + " conditions for " + to_string( n ) + " degrees of freedom" );
        return 0;
    }

    int info = FitModelMgr.Optimize();

    FitModelMgr.UpdateDist();

    ErrorMgr.NoError();
    return info;
}

bool CanUndoFitModel()
{
    ErrorMgr.NoError();
    return FitModelMgr.CanUndo();
}

bool UndoFitModel()
{
    ErrorMgr.NoError();
    return FitModelMgr.Undo();
}

void SaveFitModelFile( const std::string & file_name )
{
    FitModelMgr.SetSaveFitFileName( file_name );

    if ( !FitModelMgr.Save() )
    {
        ErrorMgr.AddError( VSP_FILE_WRITE_FAILURE, "SaveFitModelFile::Failure Writing File " + file_name );
        return;
    }

    ErrorMgr.NoError();
}

int LoadFitModelFile( const std::string & file_name )
{
    FitModelMgr.SetLoadFitFileName( file_name );

    int err = FitModelMgr.Load();

    if ( err != 0 )
    {
        ErrorMgr.AddError( VSP_FILE_READ_FAILURE, "LoadFitModelFile::Failure Reading File " + file_name );
        return err;
    }

    ErrorMgr.NoError();
    return err;
}

//===================================================================//
//====================  Point Cloud Functions  ======================//
//===================================================================//

static bool CheckPtsTol( double tol, const std::string & routine )
{
    if ( tol < 0.0 )
    {
        ErrorMgr.AddError( VSP_INVALID_INPUT_VAL, routine + "::Negative Tolerance " + to_string( tol ) );
        return false;
    }
    return true;
}

static bool CheckDirIndex( int dir_index, const std::string & routine )
{
    if ( dir_index < vsp::X_DIR || dir_index > vsp::Z_DIR )
    {
        ErrorMgr.AddError( VSP_INVALID_TYPE, routine + "::Invalid Direction Index " + to_string( dir_index ) );
        return false;
    }
    return true;
}

std::string CreatePtCloudGeomFromPts( const std::vector < vec3d > & pt_vec, const std::string & name )
{
    Vehicle* veh = GetVehicle();

    // A point cloud is not one of the types offered for creation -- it only ever arrives by import
    // or by conversion from a mesh -- so the type is built here the way Vehicle::ImportFile builds
    // it, rather than looked up by name.
    GeomType type = GeomType( PT_CLOUD_GEOM_TYPE, "PTS", true );
    std::string id = veh->AddGeom( type );

    if ( id.empty() || id.compare( "NONE" ) == 0 )
    {
        ErrorMgr.AddError( VSP_INVALID_PTR, "CreatePtCloudGeomFromPts::Could Not Add Point Cloud Geom" );
        return std::string();
    }

    Geom* geom_ptr = veh->FindGeom( id );
    if ( !geom_ptr )
    {
        ErrorMgr.AddError( VSP_INVALID_PTR, "CreatePtCloudGeomFromPts::Could Not Add Point Cloud Geom" );
        return std::string();
    }

    PtCloudGeom* pt_cloud = dynamic_cast< PtCloudGeom* >( geom_ptr );
    if ( !pt_cloud )
    {
        ErrorMgr.AddError( VSP_INVALID_PTR, "CreatePtCloudGeomFromPts::Geom Is Not A Point Cloud" );
        return std::string();
    }

    pt_cloud->m_Pts = pt_vec;
    pt_cloud->InitPts();

    if ( !name.empty() )
    {
        pt_cloud->SetName( name );
    }

    pt_cloud->SetDirtyFlag( GeomBase::SURF );
    pt_cloud->Update();

    Update();

    ErrorMgr.NoError();
    return id;
}

std::vector < vec3d > KeepPtsInBBox( const std::vector < vec3d > & pt_vec, const vec3d & min_pt, const vec3d & max_pt )
{
    ErrorMgr.NoError();
    return FilterPntsInBBox( pt_vec, min_pt, max_pt, true );
}

std::vector < vec3d > RemovePtsInBBox( const std::vector < vec3d > & pt_vec, const vec3d & min_pt, const vec3d & max_pt )
{
    ErrorMgr.NoError();
    return FilterPntsInBBox( pt_vec, min_pt, max_pt, false );
}

std::vector < vec3d > KeepPtsInRange( const std::vector < vec3d > & pt_vec, int dir_index, double low, double high )
{
    if ( !CheckDirIndex( dir_index, "KeepPtsInRange" ) )
    {
        return std::vector < vec3d >();
    }

    ErrorMgr.NoError();
    return FilterPntsInRange( pt_vec, dir_index, low, high, true );
}

std::vector < vec3d > RemovePtsInRange( const std::vector < vec3d > & pt_vec, int dir_index, double low, double high )
{
    if ( !CheckDirIndex( dir_index, "RemovePtsInRange" ) )
    {
        return std::vector < vec3d >();
    }

    ErrorMgr.NoError();
    return FilterPntsInRange( pt_vec, dir_index, low, high, false );
}

std::vector < vec3d > KeepPtsAbove( const std::vector < vec3d > & pt_vec, int dir_index, double val )
{
    if ( !CheckDirIndex( dir_index, "KeepPtsAbove" ) )
    {
        return std::vector < vec3d >();
    }

    ErrorMgr.NoError();
    return FilterPntsByValue( pt_vec, dir_index, val, true );
}

std::vector < vec3d > KeepPtsBelow( const std::vector < vec3d > & pt_vec, int dir_index, double val )
{
    if ( !CheckDirIndex( dir_index, "KeepPtsBelow" ) )
    {
        return std::vector < vec3d >();
    }

    ErrorMgr.NoError();
    return FilterPntsByValue( pt_vec, dir_index, val, false );
}

std::vector < vec3d > KeepPtsNearPt( const std::vector < vec3d > & pt_vec, const vec3d & center, double radius )
{
    if ( !CheckPtsTol( radius, "KeepPtsNearPt" ) )
    {
        return std::vector < vec3d >();
    }

    ErrorMgr.NoError();
    return FilterPntsNearPnt( pt_vec, center, radius, true );
}

std::vector < vec3d > RemovePtsNearPt( const std::vector < vec3d > & pt_vec, const vec3d & center, double radius )
{
    if ( !CheckPtsTol( radius, "RemovePtsNearPt" ) )
    {
        return std::vector < vec3d >();
    }

    ErrorMgr.NoError();
    return FilterPntsNearPnt( pt_vec, center, radius, false );
}

// Look up the surface a point filter is measured against, reporting whichever way it is not there.
static const VspSurf * FindPtsSurf( const std::string & geom_id, int surf_indx, double tol, const std::string & routine )
{
    if ( !CheckPtsTol( tol, routine ) )
    {
        return nullptr;
    }

    Vehicle* veh = GetVehicle();
    Geom* geom_ptr = veh->FindGeom( geom_id );
    if ( !geom_ptr )
    {
        ErrorMgr.AddError( VSP_INVALID_GEOM_ID, routine + "::Can't Find Geom " + geom_id );
        return nullptr;
    }

    if ( surf_indx < 0 || surf_indx >= geom_ptr->GetNumTotalSurfs() )
    {
        ErrorMgr.AddError( VSP_INDEX_OUT_RANGE, routine + "::Surface Index Out Of Range " + to_string( surf_indx ) );
        return nullptr;
    }

    const VspSurf* surf = geom_ptr->GetSurfPtr( surf_indx );
    if ( !surf )
    {
        ErrorMgr.AddError( VSP_INVALID_PTR, routine + "::Can't Find Surface " + to_string( surf_indx ) );
        return nullptr;
    }

    return surf;
}

std::vector < vec3d > KeepPtsNearGeom( const std::vector < vec3d > & pt_vec, const std::string & geom_id, int surf_indx, double tol )
{
    const VspSurf* surf = FindPtsSurf( geom_id, surf_indx, tol, "KeepPtsNearGeom" );
    if ( !surf )
    {
        return std::vector < vec3d >();
    }

    ErrorMgr.NoError();
    return FilterPntsNearSurf( pt_vec, surf, tol, true );
}

std::vector < vec3d > RemovePtsNearGeom( const std::vector < vec3d > & pt_vec, const std::string & geom_id, int surf_indx, double tol )
{
    const VspSurf* surf = FindPtsSurf( geom_id, surf_indx, tol, "RemovePtsNearGeom" );
    if ( !surf )
    {
        return std::vector < vec3d >();
    }

    ErrorMgr.NoError();
    return FilterPntsNearSurf( pt_vec, surf, tol, false );
}

std::vector < vec3d > UniquePts( const std::vector < vec3d > & pt_vec, double tol )
{
    std::vector < vec3d > out;

    if ( !CheckPtsTol( tol, "UniquePts" ) )
    {
        return out;
    }

    out = UniquePnts( pt_vec, tol );

    ErrorMgr.NoError();
    return out;
}

std::vector < vec3d > UnionPts( const std::vector < vec3d > & pt_vec_a, const std::vector < vec3d > & pt_vec_b, double tol )
{
    std::vector < vec3d > both;

    if ( !CheckPtsTol( tol, "UnionPts" ) )
    {
        return both;
    }

    both = UnionPnts( pt_vec_a, pt_vec_b, tol );

    ErrorMgr.NoError();
    return both;
}

std::vector < vec3d > IntersectPts( const std::vector < vec3d > & pt_vec_a, const std::vector < vec3d > & pt_vec_b, double tol )
{
    std::vector < vec3d > out;

    if ( !CheckPtsTol( tol, "IntersectPts" ) )
    {
        return out;
    }

    out = FilterPntsByMembership( pt_vec_a, pt_vec_b, tol, true );

    ErrorMgr.NoError();
    return out;
}

std::vector < vec3d > SubtractPts( const std::vector < vec3d > & pt_vec_a, const std::vector < vec3d > & pt_vec_b, double tol )
{
    std::vector < vec3d > out;

    if ( !CheckPtsTol( tol, "SubtractPts" ) )
    {
        return out;
    }

    out = FilterPntsByMembership( pt_vec_a, pt_vec_b, tol, false );

    ErrorMgr.NoError();
    return out;
}

//===================================================================//
//===============     Variable Presets Functions       ==============//
//===================================================================//

std::string AddVarPresetGroup( const std::string &group_name )
{
    SettingGroup *sg = new SettingGroup();
    sg->SetName( group_name );

    if ( VarPresetMgr.AddSettingGroup( sg ) )
    {
        ErrorMgr.NoError();
        return sg->GetID();
    }

    delete sg;

    ErrorMgr.AddError( VSP_INVALID_PTR, "AddVarPresetGroup::Unable to create var preset group." );
    return std::string();
}

std::string AddVarPresetSetting( const std::string &group_id, const std::string &setting_name )
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

    ErrorMgr.AddError( VSP_INVALID_ID, "AddVarPresetSetting::Unable to create var preset setting in group " + group_id );
    return std::string();
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
        ErrorMgr.AddError( VSP_CANT_FIND_PARM, "AddVarPresetParm::Unable to add Parm " + parm_id + " to var preset group " + group_id );
        return;
    }

    ErrorMgr.AddError( VSP_INVALID_ID, "AddVarPresetParm::Unable to find var preset group " + group_id );
}

void DeleteVarPresetGroup( const std::string &group_id )
{
    if ( !VarPresetMgr.FindSettingGroup( group_id ) )
    {
        ErrorMgr.AddError( VSP_INVALID_ID, "DeleteVarPresetGroup::Unable to find var preset group " + group_id );
        return;
    }

    VarPresetMgr.DeleteSettingGroup( group_id );
    ErrorMgr.NoError();
}

void DeleteVarPresetSetting( const std::string &group_id, const std::string &setting_id )
{
    if ( !VarPresetMgr.FindSettingGroup( group_id ) )
    {
        ErrorMgr.AddError( VSP_INVALID_ID, "DeleteVarPresetSetting::Unable to find var preset group " + group_id );
        return;
    }

    if ( !VarPresetMgr.FindSetting( setting_id ) )
    {
        ErrorMgr.AddError( VSP_INVALID_ID, "DeleteVarPresetSetting::Unable to find var preset setting " + setting_id );
        return;
    }

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
    ErrorMgr.AddError( VSP_INVALID_ID, "DeleteVarPresetParm::Unable to find var preset group " + group_id );
}

void SetVarPresetParmVal( const std::string &group_id, const std::string &setting_id, const std::string &parm_id, double parm_val )
{
    SettingGroup *sg = VarPresetMgr.FindSettingGroup( group_id );

    if ( sg )
    {
        std::vector < std::string > parm_vec = sg->GetParmIDVec();

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

        ErrorMgr.AddError( VSP_CANT_FIND_PARM, "SetVarPresetParmVal::Unable to find Parm " + parm_id + " in var preset group, or Setting " + setting_id );
        return;
    }

    ErrorMgr.AddError( VSP_INVALID_ID, "SetVarPresetParmVal::Unable to find var preset group " + group_id );
}

double GetVarPresetParmVal( const std::string &group_id, const std::string &setting_id, const std::string &parm_id )
{
    SettingGroup *sg = VarPresetMgr.FindSettingGroup( group_id );

    if ( sg )
    {
        std::vector < std::string > parm_vec = sg->GetParmIDVec();

        int index = vector_find_val( parm_vec, parm_id );

        if ( index >= 0 )
        {
            Setting *s = VarPresetMgr.FindSetting( setting_id );
            if ( s )
            {
                ErrorMgr.NoError();
                return s->GetParmVal( index );
            }
            ErrorMgr.AddError( VSP_INVALID_ID, "GetVarPresetParmVal::Unable to find var preset setting " + setting_id );
            return 0.0;
        }

        ErrorMgr.AddError( VSP_CANT_FIND_PARM, "GetVarPresetParmVal::Unable to find Parm " + parm_id + " in var preset group " + group_id );
        return 0.0;
    }

    ErrorMgr.AddError( VSP_INVALID_ID, "GetVarPresetParmVal::Unable to find var preset group " + group_id );
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
    ErrorMgr.AddError( VSP_INVALID_ID, "GetGroupName::Unable to find var preset group " + group_id );
    return std::string();
}

std::string GetSettingName( const std::string &setting_id )
{
    Setting *s = VarPresetMgr.FindSetting( setting_id );
    if ( s )
    {
        ErrorMgr.NoError();
        return s->GetName();
    }
    ErrorMgr.AddError( VSP_INVALID_ID, "GetSettingName::Unable to find var preset setting " + setting_id );
    return std::string();
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
    ErrorMgr.AddError( VSP_INVALID_ID, "SetGroupName::Unable to find var preset group " + group_id );
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
    ErrorMgr.AddError( VSP_INVALID_ID, "SetSettingName::Unable to find var preset setting " + setting_id );
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

    ErrorMgr.AddError( VSP_INVALID_ID, "GetVarPresetSettings::Unable to find var preset group " + group_id );
    std::vector < std::string > ret;
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

    ErrorMgr.AddError( VSP_INVALID_ID, "GetVarPresetParmIDs::Unable to find var preset group " + group_id );
    std::vector < std::string > ret;
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
    ErrorMgr.AddError( VSP_INVALID_ID, "GetVarPresetParmVals::Unable to find var preset setting " + setting_id );
    std::vector < double > ret;
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
    ErrorMgr.AddError( VSP_INVALID_ID, "SetVarPresetParmVals::Unable to find var preset setting " + setting_id );
}

void SaveVarPresetParmVals( const std::string &group_id, const std::string &setting_id )
{
    SettingGroup *sg = VarPresetMgr.FindSettingGroup( group_id );

    if ( sg )
    {
        if ( !VarPresetMgr.FindSetting( setting_id ) )
        {
            ErrorMgr.AddError( VSP_INVALID_ID, "SaveVarPresetParmVals::Unable to find var preset setting " + setting_id );
            return;
        }

        sg->SaveSetting( setting_id );
        ErrorMgr.NoError();
        return;
    }
    ErrorMgr.AddError( VSP_INVALID_ID, "SaveVarPresetParmVals::Unable to find var preset group " + group_id );
}

void ApplyVarPresetSetting( const std::string &group_id, const std::string &setting_id )
{
    SettingGroup *sg = VarPresetMgr.FindSettingGroup( group_id );

    if ( sg )
    {
        if ( !VarPresetMgr.FindSetting( setting_id ) )
        {
            ErrorMgr.AddError( VSP_INVALID_ID, "ApplyVarPresetSetting::Unable to find var preset setting " + setting_id );
            return;
        }

        sg->ApplySetting( setting_id );
        ErrorMgr.NoError();
        return;
    }
    ErrorMgr.AddError( VSP_INVALID_ID, "ApplyVarPresetSetting::Unable to find var preset group " + group_id );
}

//===================================================================//
//=======================     Mode Functions       ==================//
//===================================================================//

std::string CreateAndAddMode( const std::string & name, int normal_set, int degen_set )
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

    return std::string();
}

int GetNumModes()
{
    ErrorMgr.NoError();
    return ModeMgr.GetNumModes();
}

std::vector < std::string > GetAllModes()
{
    ErrorMgr.NoError();
    return ModeMgr.GetAllModes();
}

void DelMode( const std::string &mid )
{
    if ( !ModeMgr.GetMode( mid ) )
    {
        ErrorMgr.AddError( VSP_INVALID_ID, "DelMode::Could not find mode " + mid );
        return;
    }

    ModeMgr.DelMode( mid );
    ErrorMgr.NoError();
}

void DelAllModes()
{
    ErrorMgr.NoError();
    ModeMgr.DelAllModes();
}

std::string GetModeName( const std::string &mid )
{
    Mode *mod = ModeMgr.GetMode( mid );

    if ( !mod )
    {
        ErrorMgr.AddError( VSP_INVALID_ID, "GetModeName::Could not find mode " + mid );
        return std::string();
    }

    ErrorMgr.NoError();
    return mod->GetName();
}

void SetModeName( const std::string &mid, const std::string &name )
{
    Mode *mod = ModeMgr.GetMode( mid );

    if ( !mod )
    {
        ErrorMgr.AddError( VSP_INVALID_ID, "SetModeName::Could not find mode " + mid );
        return;
    }

    mod->SetName( name );

    ErrorMgr.NoError();
}

void ApplyModeSettings( const std::string &mid )
{
    Mode *mod = ModeMgr.GetMode( mid );

    if ( !mod )
    {
        ErrorMgr.AddError( VSP_INVALID_ID, "ApplyModeSettings::Could not find mode " + mid );
        return;
    }

    mod->ApplySettings();
    ErrorMgr.NoError();
}

void ShowOnlyMode( const std::string &mid )
{
    Mode *mod = ModeMgr.GetMode( mid );

    if ( !mod )
    {
        ErrorMgr.AddError( VSP_INVALID_ID, "ShowOnlyMode::Could not find mode " + mid );
        return;
    }

    mod->ShowOnly();
    ErrorMgr.NoError();
}

void ModeAddGroupSetting( const std::string &mid, const std::string &gid, const std::string &sid )
{
    Mode *mod = ModeMgr.GetMode( mid );

    if ( !mod )
    {
        ErrorMgr.AddError( VSP_INVALID_ID, "ModeAddGroupSetting::Could not find mode " + mid );
        return;
    }

    if ( !VarPresetMgr.FindSettingGroup( gid ) )
    {
        ErrorMgr.AddError( VSP_INVALID_ID, "ModeAddGroupSetting::Unable to find var preset group " + gid );
        return;
    }

    if ( !VarPresetMgr.FindSetting( sid ) )
    {
        ErrorMgr.AddError( VSP_INVALID_ID, "ModeAddGroupSetting::Unable to find var preset setting " + sid );
        return;
    }

    mod->AddSetting( pair < std::string, std::string > ( gid, sid ) );
    ErrorMgr.NoError();
}

std::string ModeGetGroup( const std::string &mid, int indx )
{
    Mode *mod = ModeMgr.GetMode( mid );

    if ( !mod )
    {
        ErrorMgr.AddError( VSP_INVALID_ID, "ModeGetGroup::Could not find mode " + mid );
        return std::string();
    }

    if ( indx < 0 || indx >= ( int )mod->GetAllGroupSettings().size() )
    {
        ErrorMgr.AddError( VSP_INDEX_OUT_RANGE, "ModeGetGroup::Setting index " + to_string( indx ) + " out of range" );
        return std::string();
    }

    pair < std::string, std::string > set = mod->GetSetting( indx );
    ErrorMgr.NoError();
    return set.first;
}

static std::string ModeGetSettingImpl( const std::string &mid, int indx, const std::string & caller )
{
    Mode *mod = ModeMgr.GetMode( mid );

    if ( !mod )
    {
        ErrorMgr.AddError( VSP_INVALID_ID, caller + "::Could not find mode " + mid );
        return std::string();
    }

    if ( indx < 0 || indx >= ( int )mod->GetAllGroupSettings().size() )
    {
        ErrorMgr.AddError( VSP_INDEX_OUT_RANGE, caller + "::Setting index " + to_string( indx ) + " out of range" );
        return std::string();
    }

    pair < std::string, std::string > set = mod->GetSetting( indx );
    ErrorMgr.NoError();
    return set.second;
}

std::string ModeGetSetting( const std::string &mid, int indx )
{
    return ModeGetSettingImpl( mid, indx, "ModeGetSetting" );
}

std::vector < std::string > ModeGetAllGroups( const std::string &mid )
{
    std::vector < std::string > ret;
    Mode *mod = ModeMgr.GetMode( mid );

    if ( !mod )
    {
        ErrorMgr.AddError( VSP_INVALID_ID, "ModeGetAllGroups::Could not find mode " + mid );
        return ret;
    }

    ret = mod->GetAllGroups();
    ErrorMgr.NoError();
    return ret;
}

std::vector < std::string > ModeGetAllSettings( const std::string &mid )
{
    std::vector < std::string > ret;
    Mode *mod = ModeMgr.GetMode( mid );

    if ( !mod )
    {
        ErrorMgr.AddError( VSP_INVALID_ID, "ModeGetAllSettings::Could not find mode " + mid );
        return ret;
    }

    ret = mod->GetAllSettings();
    ErrorMgr.NoError();
    return ret;
}

static void RemoveGroupSettingImpl( const std::string &mid, int indx, const std::string & caller )
{
    Mode *mod = ModeMgr.GetMode( mid );

    if ( !mod )
    {
        ErrorMgr.AddError( VSP_INVALID_ID, caller + "::Could not find mode " + mid );
        return;
    }

    if ( indx < 0 || indx >= ( int )mod->GetAllGroupSettings().size() )
    {
        ErrorMgr.AddError( VSP_INDEX_OUT_RANGE, caller + "::Setting index " + to_string( indx ) + " out of range" );
        return;
    }

    mod->RemoveSetting( indx );
    ErrorMgr.NoError();
}

void RemoveGroupSetting( const std::string &mid, int indx )
{
    RemoveGroupSettingImpl( mid, indx, "RemoveGroupSetting" );
}

//==== The index of a variable preset group's setting in a mode, reporting through the
//==== named caller when the mode or the group's setting cannot be found ====//
int FindModeGroupIndex( const std::string & mid, const std::string & gid, const std::string & caller )
{
    Mode *mod = ModeMgr.GetMode( mid );
    if ( !mod )
    {
        ErrorMgr.AddError( VSP_INVALID_ID, caller + "::Could not find mode " + mid );
        return -1;
    }

    std::vector < std::string > gids = mod->GetAllGroups();
    int index = vector_find_val( gids, gid );
    if ( index < 0 )
    {
        ErrorMgr.AddError( VSP_INVALID_ID, caller + "::Mode " + mid + " has no setting for group " + gid );
    }
    return index;
}

void RemoveGroupSettingByID( const std::string &mid, const std::string &gid )
{
    int index = FindModeGroupIndex( mid, gid, "RemoveGroupSettingByID" );
    if ( index < 0 )
    {
        return;
    }

    RemoveGroupSettingImpl( mid, index, "RemoveGroupSettingByID" );
}

std::string ModeGetGroupSetting( const std::string &mid, const std::string &gid )
{
    int index = FindModeGroupIndex( mid, gid, "ModeGetGroupSetting" );
    if ( index < 0 )
    {
        return std::string();
    }

    return ModeGetSettingImpl( mid, index, "ModeGetGroupSetting" );
}

void RemoveAllGroupSettings( const std::string &mid )
{
    Mode *mod = ModeMgr.GetMode( mid );

    if ( !mod )
    {
        ErrorMgr.AddError( VSP_INVALID_ID, "RemoveAllGroupSettings::Could not find mode " + mid );
        return;
    }

    mod->RemoveAllSettings();
    ErrorMgr.NoError();
}

//===================================================================//
//===============     Parametric Curve Functions       ==============//
//===================================================================//

void SetPCurve( const std::string & geom_id, const int & pcurveid, const std::vector < double > & tvec,
    const std::vector < double > & valvec, const int & newtype )
{
    Vehicle* veh = GetVehicle();
    Geom* geom_ptr = veh->FindGeom( geom_id );
    if ( !geom_ptr )
    {
        ErrorMgr.AddError( VSP_INVALID_GEOM_ID, "SetPCurve::Can't Find Geom " + geom_id );
        return;
    }
    else if ( geom_ptr->GetType().m_Type != PROP_GEOM_TYPE )
    {
        ErrorMgr.AddError( VSP_WRONG_GEOM_TYPE, "SetPCurve::Geom doesn't support PCurves " + geom_id );
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
        ErrorMgr.AddError( VSP_INVALID_TYPE, "SetPCurve::PCurve not found " + geom_id + " " + to_string( pcurveid ) );
        return;
    }

    pc->SetCurve( tvec, valvec, newtype );

    ErrorMgr.NoError();
}

void PCurveConvertTo( const std::string & geom_id, const int & pcurveid, const int & newtype )
{
    Vehicle* veh = GetVehicle();
    Geom* geom_ptr = veh->FindGeom( geom_id );
    if ( !geom_ptr )
    {
        ErrorMgr.AddError( VSP_INVALID_GEOM_ID, "PCurveConvertTo::Can't Find Geom " + geom_id );
        return;
    }
    else if ( geom_ptr->GetType().m_Type != PROP_GEOM_TYPE )
    {
        ErrorMgr.AddError( VSP_WRONG_GEOM_TYPE, "PCurveConvertTo::Geom doesn't support PCurves " + geom_id );
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
        ErrorMgr.AddError( VSP_INVALID_TYPE, "PCurveConvertTo::PCurve not found " + geom_id + " " + to_string( pcurveid ) );
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
        ErrorMgr.AddError( VSP_INVALID_GEOM_ID, "PCurveGetType::Can't Find Geom " + geom_id );
        return -1;
    }
    else if ( geom_ptr->GetBehaviorType() != PROP_GEOM_TYPE )
    {
        ErrorMgr.AddError( VSP_WRONG_GEOM_TYPE, "PCurveGetType::Geom doesn't support PCurves " + geom_id );
        return -1;
    }

    // Read-only, so a Clone of a propeller answers from its original.
    PropGeom* behavior_prop = dynamic_cast < PropGeom* > ( geom_ptr->GetBehaviorGeom() );
    PCurve *pc = nullptr;

    if ( behavior_prop )
    {
        pc = behavior_prop->GetPCurve( pcurveid );
    }

    if ( !pc )
    {
        ErrorMgr.AddError( VSP_INVALID_TYPE, "PCurveGetType::PCurve not found " + geom_id + " " + to_string( pcurveid ) );
        return -1;
    }

    ErrorMgr.NoError();

    return pc->m_CurveType();
}

std::vector < double > PCurveGetTVec( const std::string & geom_id, const int & pcurveid )
{
    std::vector < double > retvec;

    Vehicle* veh = GetVehicle();
    Geom* geom_ptr = veh->FindGeom( geom_id );
    if ( !geom_ptr )
    {
        ErrorMgr.AddError( VSP_INVALID_GEOM_ID, "PCurveGetTVec::Can't Find Geom " + geom_id );
        return retvec;
    }
    else if ( geom_ptr->GetBehaviorType() != PROP_GEOM_TYPE )
    {
        ErrorMgr.AddError( VSP_WRONG_GEOM_TYPE, "PCurveGetTVec::Geom doesn't support PCurves " + geom_id );
        return retvec;
    }

    // Read-only, so a Clone of a propeller answers from its original.
    PropGeom* behavior_prop = dynamic_cast < PropGeom* > ( geom_ptr->GetBehaviorGeom() );
    PCurve *pc = nullptr;

    if ( behavior_prop )
    {
        pc = behavior_prop->GetPCurve( pcurveid );
    }

    if ( !pc )
    {
        ErrorMgr.AddError( VSP_INVALID_TYPE, "PCurveGetTVec::PCurve not found " + geom_id + " " + to_string( pcurveid ) );
        return retvec;
    }

    retvec = pc->GetTVec();

    ErrorMgr.NoError();

    return retvec;
}

std::vector < double > PCurveGetValVec( const std::string & geom_id, const int & pcurveid )
{
    std::vector < double > retvec;

    Vehicle* veh = GetVehicle();
    Geom* geom_ptr = veh->FindGeom( geom_id );
    if ( !geom_ptr )
    {
        ErrorMgr.AddError( VSP_INVALID_GEOM_ID, "PCurveGetValVec::Can't Find Geom " + geom_id );
        return retvec;
    }
    else if ( geom_ptr->GetBehaviorType() != PROP_GEOM_TYPE )
    {
        ErrorMgr.AddError( VSP_WRONG_GEOM_TYPE, "PCurveGetValVec::Geom doesn't support PCurves " + geom_id );
        return retvec;
    }

    // Read-only, so a Clone of a propeller answers from its original.
    PropGeom* behavior_prop = dynamic_cast < PropGeom* > ( geom_ptr->GetBehaviorGeom() );
    PCurve *pc = nullptr;

    if ( behavior_prop )
    {
        pc = behavior_prop->GetPCurve( pcurveid );
    }

    if ( !pc )
    {
        ErrorMgr.AddError( VSP_INVALID_TYPE, "PCurveGetValVec::PCurve not found " + geom_id + " " + to_string( pcurveid ) );
        return retvec;
    }

    retvec = pc->GetValVec();

    ErrorMgr.NoError();

    return retvec;
}

void PCurveDeletePt( const std::string & geom_id, const int & pcurveid, const int & indx )
{
    Vehicle* veh = GetVehicle();
    Geom* geom_ptr = veh->FindGeom( geom_id );
    if ( !geom_ptr )
    {
        ErrorMgr.AddError( VSP_INVALID_GEOM_ID, "PCurveDeletePt::Can't Find Geom " + geom_id );
        return;
    }
    else if ( geom_ptr->GetType().m_Type != PROP_GEOM_TYPE )
    {
        ErrorMgr.AddError( VSP_WRONG_GEOM_TYPE, "PCurveDeletePt::Geom doesn't support PCurves " + geom_id );
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
        ErrorMgr.AddError( VSP_INVALID_TYPE, "PCurveDeletePt::PCurve not found " + geom_id + " " + to_string( pcurveid ) );
        return;
    }

    if ( indx <= 0 || indx >= pc->GetNumPts() - 1 )
    {
        ErrorMgr.AddError( VSP_INDEX_OUT_RANGE, "PCurveDeletePt::Point index " + to_string( indx ) + " is an end point or out of range" );
        return;
    }

    if ( pc->m_CurveType() == vsp::CEDIT && ( indx % 3 ) != 0 )
    {
        ErrorMgr.AddError( VSP_INVALID_INPUT_VAL, "PCurveDeletePt::Point index " + to_string( indx ) + " is a CEDIT control point" );
        return;
    }

    pc->DeletePt( indx );

    ErrorMgr.NoError();
}

int PCurveSplit( const std::string & geom_id, const int & pcurveid, const double & tsplit )
{
    Vehicle* veh = GetVehicle();
    Geom* geom_ptr = veh->FindGeom( geom_id );
    if ( !geom_ptr )
    {
        ErrorMgr.AddError( VSP_INVALID_GEOM_ID, "PCurveSplit::Can't Find Geom " + geom_id );
        return -1;
    }
    else if ( geom_ptr->GetType().m_Type != PROP_GEOM_TYPE )
    {
        ErrorMgr.AddError( VSP_WRONG_GEOM_TYPE, "PCurveSplit::Geom doesn't support PCurves " + geom_id );
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
        ErrorMgr.AddError( VSP_INVALID_TYPE, "PCurveSplit::PCurve not found " + geom_id + " " + to_string( pcurveid ) );
        return -1;
    }

    return pc->Split( tsplit );

    ErrorMgr.NoError();
}

void ResetPropellerThickness( const std::string & geom_id )
{
    Vehicle* veh = GetVehicle();
    Geom* geom_ptr = veh->FindGeom( geom_id );
    if ( !geom_ptr )
    {
        ErrorMgr.AddError( VSP_INVALID_GEOM_ID, "ResetPropellerThickness::Can't Find Geom " + geom_id );
        return;
    }

    PropGeom* prop_ptr = dynamic_cast< PropGeom* >( geom_ptr );
    if ( !prop_ptr )
    {
        ErrorMgr.AddError( VSP_WRONG_GEOM_TYPE, "ResetPropellerThickness::Geom " + geom_id + " is not a propeller" );
        return;
    }

    prop_ptr->ResetThickness();
    prop_ptr->Update();

    ErrorMgr.NoError();
}

void ApproximateAllPropellerPCurves( const std::string & geom_id )
{
    Vehicle* veh = GetVehicle();
    Geom* geom_ptr = veh->FindGeom( geom_id );
    if ( !geom_ptr )
    {
        ErrorMgr.AddError( VSP_INVALID_GEOM_ID, "ApproximateAllPropellerPCurves::Can't Find Geom " + geom_id );
        return;
    }
    else if ( geom_ptr->GetType().m_Type != PROP_GEOM_TYPE )
    {
        ErrorMgr.AddError( VSP_WRONG_GEOM_TYPE, "ApproximateAllPropellerPCurves::Geom not a propeller " + geom_id );
        return;
    }

    PropGeom* prop_ptr = dynamic_cast < PropGeom* > (geom_ptr );

    if ( prop_ptr )
    {
        prop_ptr->ApproxCubicAllPCurves();
    }
    else
    {
        ErrorMgr.AddError( VSP_WRONG_GEOM_TYPE, "ApproximateAllPropellerPCurves::Geom not a propeller " + geom_id );
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
        ErrorMgr.AddError( VSP_INVALID_GEOM_ID, "ResetPropellerThicknessCurve::Can't Find Geom " + geom_id );
        return;
    }
    else if ( geom_ptr->GetType().m_Type != PROP_GEOM_TYPE )
    {
        ErrorMgr.AddError( VSP_WRONG_GEOM_TYPE, "ResetPropellerThicknessCurve::Geom not a propeller " + geom_id );
        return;
    }

    PropGeom* prop_ptr = dynamic_cast < PropGeom* > (geom_ptr );

    if ( prop_ptr )
    {
        prop_ptr->ResetThickness();
    }
    else
    {
        ErrorMgr.AddError( VSP_WRONG_GEOM_TYPE, "ResetPropellerThicknessCurve::Geom not a propeller " + geom_id );
        return;
    }

    ErrorMgr.NoError();
}

//===================================================================//
//===============    Parasite Drag Tool Functions      ==============//
//===================================================================//

void AddExcrescence(const std::string &excresName, const int & excresType, const double & excresVal)
{
    if ( excresType < vsp::EXCRESCENCE_COUNT || excresType > vsp::EXCRESCENCE_DRAGAREA )
    {
        ErrorMgr.AddError( VSP_INVALID_TYPE, "AddExcrescence::Invalid Excrescence Type " + to_string( excresType ) );
        return;
    }

    ParasiteDragMgr.AddExcrescence(excresName,excresType,excresVal);

    ErrorMgr.NoError();
}

int GetNumExcrescences()
{
    int ret = ( int )ParasiteDragMgr.GetExcresVec().size();

    ErrorMgr.NoError();
    return ret;
}

void DeleteAllExcrescences()
{
    while ( ParasiteDragMgr.GetExcresVec().size() > 0 )
    {
        ParasiteDragMgr.DeleteExcrescence( 0 );
    }

    ErrorMgr.NoError();
}

void DeleteExcrescence(const int & index)
{
    // An index outside the table would be erased anyway, which is undefined
    // behaviour rather than a diagnosable mistake.
    if ( index < 0 || index >= ( int )ParasiteDragMgr.GetExcresVec().size() )
    {
        ErrorMgr.AddError( VSP_INDEX_OUT_RANGE, "DeleteExcrescence::Index " + to_string( index ) + " Out of Range" );
        return;
    }

    ParasiteDragMgr.DeleteExcrescence(index);

    ErrorMgr.NoError();
}

void UpdateParasiteDrag()
{
    ParasiteDragMgr.Update();

    ErrorMgr.NoError();
}

std::string ExportParasiteDragToCSV( const std::string & file_name )
{
    std::string res_id = ParasiteDragMgr.ExportToCSV( file_name );

    if ( res_id.empty() )
    {
        ErrorMgr.AddError( VSP_FILE_WRITE_FAILURE, "ExportParasiteDragToCSV::Can't Write File " + file_name );
        return res_id;
    }

    ErrorMgr.NoError();
    return res_id;
}

void SetParasiteDragRefWingID( const std::string & geom_id )
{
    if ( geom_id.empty() )
    {
        ParasiteDragMgr.m_RefGeomID = geom_id;
        ParasiteDragMgr.m_RefFlag = vsp::MANUAL_REF;
        ErrorMgr.NoError();
        return;
    }

    Geom* geom_ptr = GetVehicle()->FindGeom( geom_id );
    if ( !geom_ptr )
    {
        ErrorMgr.AddError( VSP_INVALID_GEOM_ID, "SetParasiteDragRefWingID::Can't Find Geom " + geom_id );
        return;
    }

    if ( geom_ptr->GetBehaviorType() != MS_WING_GEOM_TYPE )
    {
        ErrorMgr.AddError( VSP_WRONG_GEOM_TYPE, "SetParasiteDragRefWingID::Geom " + geom_id + " is not a wing" );
        return;
    }

    ParasiteDragMgr.m_RefGeomID = geom_id;
    ParasiteDragMgr.m_RefFlag = vsp::COMPONENT_REF;
    ErrorMgr.NoError();
}

std::string GetParasiteDragRefWingID()
{
    ErrorMgr.NoError();
    return ParasiteDragMgr.m_RefGeomID;
}

void SetParasiteDragModeID( const std::string & mode_id )
{
    if ( !mode_id.empty() && !ModeMgr.GetMode( mode_id ) )
    {
        ErrorMgr.AddError( VSP_INVALID_ID, "SetParasiteDragModeID::Can't Find Mode " + mode_id );
        return;
    }

    ParasiteDragMgr.m_ModeID = mode_id;
    ErrorMgr.NoError();
}

std::string GetParasiteDragModeID()
{
    ErrorMgr.NoError();
    return ParasiteDragMgr.m_ModeID;
}

void SetWaveDragRefWingID( const std::string & geom_id )
{
    if ( geom_id.empty() )
    {
        WaveDragMgr.m_RefGeomID = geom_id;
        WaveDragMgr.m_RefFlag = vsp::MANUAL_REF;
        ErrorMgr.NoError();
        return;
    }

    Geom* geom_ptr = GetVehicle()->FindGeom( geom_id );
    if ( !geom_ptr )
    {
        ErrorMgr.AddError( VSP_INVALID_GEOM_ID, "SetWaveDragRefWingID::Can't Find Geom " + geom_id );
        return;
    }

    if ( geom_ptr->GetBehaviorType() != MS_WING_GEOM_TYPE )
    {
        ErrorMgr.AddError( VSP_WRONG_GEOM_TYPE, "SetWaveDragRefWingID::Geom " + geom_id + " is not a wing" );
        return;
    }

    WaveDragMgr.m_RefGeomID = geom_id;
    WaveDragMgr.m_RefFlag = vsp::COMPONENT_REF;
    ErrorMgr.NoError();
}

std::string GetWaveDragRefWingID()
{
    ErrorMgr.NoError();
    return WaveDragMgr.m_RefGeomID;
}

void SetWaveDragModeID( const std::string & mode_id )
{
    if ( !mode_id.empty() && !ModeMgr.GetMode( mode_id ) )
    {
        ErrorMgr.AddError( VSP_INVALID_ID, "SetWaveDragModeID::Can't Find Mode " + mode_id );
        return;
    }

    WaveDragMgr.m_ModeID = mode_id;
    ErrorMgr.NoError();
}

std::string GetWaveDragModeID()
{
    ErrorMgr.NoError();
    return WaveDragMgr.m_ModeID;
}

void SetVSPAEROModeID( const std::string & mode_id )
{
    if ( !mode_id.empty() && !ModeMgr.GetMode( mode_id ) )
    {
        ErrorMgr.AddError( VSP_INVALID_ID, "SetVSPAEROModeID::Can't Find Mode " + mode_id );
        return;
    }

    VSPAEROMgr.m_ModeID = mode_id;
    ErrorMgr.NoError();
}

std::string GetVSPAEROModeID()
{
    ErrorMgr.NoError();
    return VSPAEROMgr.m_ModeID;
}

void SetVSPAEROCGModeID( const std::string & mode_id )
{
    if ( !mode_id.empty() && !ModeMgr.GetMode( mode_id ) )
    {
        ErrorMgr.AddError( VSP_INVALID_ID, "SetVSPAEROCGModeID::Can't Find Mode " + mode_id );
        return;
    }

    VSPAEROMgr.m_CGModeID = mode_id;
    ErrorMgr.NoError();
}

std::string GetVSPAEROCGModeID()
{
    ErrorMgr.NoError();
    return VSPAEROMgr.m_CGModeID;
}

void AddWaveDragFlowSubSurf( const std::string & ss_id )
{
    if ( !SubSurfaceMgr.GetSubSurf( ss_id ) )
    {
        ErrorMgr.AddError( VSP_INVALID_ID, "AddWaveDragFlowSubSurf::Can't Find SubSurface " + ss_id );
        return;
    }

    if ( !vector_contains_val( WaveDragMgr.m_SSFlow_vec, ss_id ) )
    {
        WaveDragMgr.m_SSFlow_vec.push_back( ss_id );
    }
    ErrorMgr.NoError();
}

void DeleteWaveDragFlowSubSurf( const std::string & ss_id )
{
    if ( !vector_contains_val( WaveDragMgr.m_SSFlow_vec, ss_id ) )
    {
        ErrorMgr.AddError( VSP_INVALID_ID, "DeleteWaveDragFlowSubSurf::SubSurface " + ss_id + " is not a Wave Drag flow face" );
        return;
    }

    vector_remove_val( WaveDragMgr.m_SSFlow_vec, ss_id );
    ErrorMgr.NoError();
}

std::vector < std::string > GetWaveDragFlowSubSurfVec()
{
    ErrorMgr.NoError();
    return WaveDragMgr.m_SSFlow_vec;
}

void WriteAtmosphereCSVFile(const std::string & file_name, const int &atmos_type)
{
    const static double arr[] = {0.0, 5000.0, 10000.0, 10999.0, 11001.0, 15000.0, 19999.0, 20000.0,
    20001.0, 25000.0, 30000.0, 31999.0, 32001.0, 35000.0, 40000.0, 45000.0, 46999.0, 47001.0, 50000.0,
    50999.0, 51001.0, 55000.0, 60000.0, 65000.0, 70000.0, 70999.0, 71001.0, 75000.0, 80000.0, 84851.0,
    84853.0, 85000.0, 90000.0}; //meters
    std::vector < double > AltTestPoints (arr, arr + sizeof(arr) / sizeof(arr[0]) );

    double temp, pres, pres_ratio, rho_ratio;
    std::vector < double > temp_vec, pres_vec, pres_ratio_vec, rho_ratio_vec;

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
    ErrorMgr.NoError();
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
        temp = 0.0;
        pres = 0.0;
        pres_ratio = 0.0;
        rho_ratio = 0.0;
        ErrorMgr.AddError( VSP_INVALID_TYPE, "CalcAtmosphere::Invalid Atmosphere Type " + to_string( atmos_type ) );
        return;
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
    std::vector < double > body_ff_vec;
    std::vector < double > dol_array = linspace( 0.0, 0.3, 200 );
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
    ErrorMgr.NoError();
}

void WriteWingFFCSVFile(const std::string & file_name)
{
    Results* res = ResultsMgr.CreateResults("Wing_Form_Factor", "Wing form factor verification results.");
    char str[256];
    std::vector < double > wing_ff_vec;
    std::vector < double > toc_array = linspace( 0.0, 0.205, 200 );
    std::vector < double > toc_loc, sweep25, sweep50;
    toc_loc.push_back(0.25);
    sweep25.push_back(30.0 * M_PI / 180.0);
    sweep50.push_back(30.0 * M_PI / 180.0);
    ParasiteDragMgr.m_Atmos.SetMach(0.8);
    res->Add( new NameValData( "T_C", toc_array, "Thickness to chord ratios" ) );
    for (size_t wing_ff_case = 0; wing_ff_case < vsp::FF_W_SCHEMENSKY_SUPERCRITICAL_AF; ++wing_ff_case )
    {
        for (size_t j = 0; j < toc_array.size(); ++j )
        {
            wing_ff_vec.push_back( ParasiteDragMgr.CalcFFWing( toc_array[j], wing_ff_case, toc_loc[0], sweep25[0], sweep50[0]) );
        }
        snprintf( str, sizeof( str ), "%s", ParasiteDragMgr.AssignFFWingEqnName( wing_ff_case ).c_str() );
        res->Add( new NameValData( str, wing_ff_vec, "Form factor." ) );
        wing_ff_vec.clear();
    }
    res->WriteCSVFile( file_name );
    ErrorMgr.NoError();
}

void WriteCfEqnCSVFile(const std::string & file_name)
{
    Results* res = ResultsMgr.CreateResults("Friction_Coefficient", "Friction coefficient verification results.");
    char str[256];
    std::vector < double > lam_cf_vec, turb_cf_vec, ref_leng;
    std::vector < double > ReyIn_array = logspace( 3, 10, 500 );
    std::vector < double > roughness, gamma, taw_tw_ratio, te_tw_ratio;
    roughness.push_back(0.01);
    gamma.push_back(1.4);
    taw_tw_ratio.push_back(1.0);
    te_tw_ratio.push_back(1.0);
    ref_leng.push_back(1.0);

    for (size_t cf_case = 0; cf_case <= vsp::DO_NOT_USE_CF_TURB_HEATTRANSFER_WHITE_CHRISTOPH; ++cf_case )
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
    ErrorMgr.NoError();
}

void WritePartialCfMethodCSVFile(const std::string & file_name)
{
    Results* res = ResultsMgr.CreateResults("Friction_Coefficient", "Partial turbulence coefficient verification results.");
    std::vector < double > cf_vec, ref_leng;
    std::vector < double > lam_perc_array = linspace( 0, 100, 1000 );
    std::vector < double > ReyIn_array, reql_array;
    ReyIn_array.push_back( 1.0e7 );
    reql_array.push_back( 1.0e7 );
    std::vector < double > roughness, taw_tw_ratio, te_tw_ratio;
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
    ErrorMgr.NoError();
}

//============================================================================//

double CompareGeomSurfaces( const std::string & geom_a, const std::string & geom_b )
{
    Vehicle* veh = GetVehicle();
    Geom* a = veh->FindGeom( geom_a );
    if ( !a )
    {
        ErrorMgr.AddError( VSP_INVALID_GEOM_ID, "CompareGeomSurfaces::Can't Find Geom " + geom_a );
        return -1.0;
    }
    Geom* b = veh->FindGeom( geom_b );
    if ( !b )
    {
        ErrorMgr.AddError( VSP_INVALID_GEOM_ID, "CompareGeomSurfaces::Can't Find Geom " + geom_b );
        return -1.0;
    }

    if ( a->GetNumTotalSurfs() != b->GetNumTotalSurfs() )
    {
        ErrorMgr.AddError( VSP_INVALID_INPUT_VAL, "CompareGeomSurfaces::Geoms have " + to_string( a->GetNumTotalSurfs() ) +
                           " and " + to_string( b->GetNumTotalSurfs() ) + " surfaces" );
        return -1.0;
    }

    double worst = 0.0;
    for ( int i = 0; i < a->GetNumTotalSurfs(); i++ )
    {
        const VspSurf* sa = a->GetSurfPtr( i );
        const VspSurf* sb = b->GetSurfPtr( i );
        double bound = -1.0;
        if ( sa && sb )
        {
            sa->Compare( *sb, bound );
        }
        if ( bound < 0.0 )
        {
            ErrorMgr.AddError( VSP_INVALID_INPUT_VAL, "CompareGeomSurfaces::Surface " + to_string( i ) + " cannot be compared" );
            return -1.0;
        }
        worst = std::max( worst, bound );
    }

    ErrorMgr.NoError();
    return worst;
}

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
    u_out = 0.0;
    w_out = 0.0;

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

    if ( !CheckDirIndex( iaxis, "AxisProjPnt01" ) )
    {
        return idmin;
    }

    idmin = geom->GetSurfPtr( surf_indx )->ProjectPt01( pt, iaxis, u_out, w_out );

    ErrorMgr.NoError();

    return idmin;
}

double AxisProjPnt01I(const std::string &geom_id, const int &iaxis, const vec3d &pt, int &surf_indx_out, double &u_out, double &w_out)
{
    surf_indx_out = -1;
    u_out = 0.0;
    w_out = 0.0;

    Vehicle* vPtr = VehicleMgr.GetVehicle();
    Geom * geom = vPtr->FindGeom( geom_id );

    double idmin = std::numeric_limits<double>::max();

    if ( !geom )
    {
        ErrorMgr.AddError( VSP_INVALID_GEOM_ID, "AxisProjPnt01I::Can't Find Geom " + geom_id );
        return idmin;
    }

    if ( !CheckDirIndex( iaxis, "AxisProjPnt01I" ) )
    {
        return idmin;
    }

    idmin = geom->AxisProjPnt01I( iaxis, pt, surf_indx_out, u_out, w_out );

    ErrorMgr.NoError();

    return idmin;
}

double AxisProjPnt01Guess(const std::string &geom_id, const int &surf_indx, const int &iaxis, const vec3d &pt, const double &u0, const double &w0, double &u_out, double &w_out)
{
    u_out = 0.0;
    w_out = 0.0;

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

    if ( !CheckDirIndex( iaxis, "AxisProjPnt01Guess" ) )
    {
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
        ErrorMgr.AddError( VSP_INVALID_GEOM_ID, "FindRSTGuess::Can't Find Geom " + geom_id );
        return dist;
    }

    if ( surf_indx < 0 || surf_indx >= geom->GetNumTotalSurfs() )
    {
        ErrorMgr.AddError( VSP_INDEX_OUT_RANGE, "FindRSTGuess::Invalid Surface Index " + to_string( surf_indx ) );
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

    // A Clone of a wing answers from its original.
    WingGeom *behavior_wing = dynamic_cast<WingGeom *>( geom->GetBehaviorGeom() );

    if ( geom->GetBehaviorType() != MS_WING_GEOM_TYPE || !behavior_wing )
    {
        ErrorMgr.AddError( VSP_WRONG_GEOM_TYPE, "ConvertUtoEta::Geom " + geom_id + " is not a WingGeom" );
        return;
    }

    eta_out = behavior_wing->UtoEta( u );

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

    // A Clone of a wing answers from its original.
    WingGeom *behavior_wing = dynamic_cast<WingGeom *>( geom->GetBehaviorGeom() );

    if ( geom->GetBehaviorType() != MS_WING_GEOM_TYPE || !behavior_wing )
    {
        ErrorMgr.AddError( VSP_WRONG_GEOM_TYPE, "ConvertEtatoU::Geom " + geom_id + " is not a WingGeom" );
        return;
    }

    u_out = behavior_wing->EtatoU( eta );

    ErrorMgr.NoError();

    return;
}

std::vector < vec3d > CompVecPnt01( const std::string &geom_id, const int &surf_indx, const std::vector < double > &us, const std::vector < double > &ws )
{
    Vehicle* veh = GetVehicle();

    Geom* geom_ptr = veh->FindGeom( geom_id );

    std::vector < vec3d > pts;
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
            ErrorMgr.AddError( VSP_INVALID_INPUT_VAL, "CompVecPnt01::Input size mismatch." );
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

std::vector < vec3d > CompVecDegenPnt01( const std::string &geom_id, const int &surf_indx, const int &degen_type, const std::vector < double > &us, const std::vector < double > &ws )
{
    Vehicle* veh = GetVehicle();

    Geom* geom_ptr = veh->FindGeom( geom_id );

    std::vector < vec3d > pts;
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
            ErrorMgr.AddError( VSP_INVALID_INPUT_VAL, "CompVecDegenPnt01::Input size mismatch." );
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

std::vector < vec3d > CompVecNorm01( const std::string &geom_id, const int &surf_indx, const std::vector < double > &us, const std::vector < double > &ws )
{
    Vehicle* veh = GetVehicle();

    Geom* geom_ptr = veh->FindGeom( geom_id );

    std::vector < vec3d > norms;
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
                ErrorMgr.AddError( VSP_INDEX_OUT_RANGE, "CompVecNorm01::Invalid surf index " + to_string( surf_indx ) );
                return norms;
            }
        }
        else
        {
            ErrorMgr.AddError( VSP_INVALID_INPUT_VAL, "CompVecNorm01::Input size mismatch." );
            return norms;
        }
    }
    else
    {
        ErrorMgr.AddError( VSP_INVALID_GEOM_ID, "CompVecNorm01::Can't Find Geom " + geom_id );
        return norms;
    }
    ErrorMgr.NoError();
    return norms;
}

void CompVecCurvature01( const std::string &geom_id, const int &surf_indx, const std::vector < double > &us, const std::vector < double > &ws, std::vector < double > &k1_out_vec, std::vector < double > &k2_out_vec, std::vector < double > &ka_out_vec, std::vector < double > &kg_out_vec )
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
                ErrorMgr.AddError( VSP_INDEX_OUT_RANGE, "CompVecCurvature01::Invalid surf index " + to_string( surf_indx ) );
                return;
            }
        }
        else
        {
            ErrorMgr.AddError( VSP_INVALID_INPUT_VAL, "CompVecCurvature01::Input size mismatch." );
            return;
        }
    }
    else
    {
        ErrorMgr.AddError( VSP_INVALID_GEOM_ID, "CompVecCurvature01::Can't Find Geom " + geom_id );
        return;
    }
    ErrorMgr.NoError();
}

void ProjVecPnt01(const std::string &geom_id, const int &surf_indx, const std::vector < vec3d > &pts, std::vector < double > &u_out_vec, std::vector < double > &w_out_vec, std::vector < double > &d_out_vec )
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

void ProjVecPnt01Guess( const std::string &geom_id, const int &surf_indx, const std::vector < vec3d > &pts, const std::vector < double > &u0s, const std::vector < double > &w0s, std::vector < double > &u_out_vec, std::vector < double > &w_out_vec, std::vector < double > &d_out_vec )
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
            ErrorMgr.AddError( VSP_INVALID_INPUT_VAL, "ProjVecPnt01Guess::Input size mismatch." );
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

        if ( !CheckDirIndex( iaxis, "AxisProjVecPnt01" ) )
        {
            return;
        }

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
        if ( !CheckDirIndex( iaxis, "AxisProjVecPnt01Guess" ) )
        {
            return;
        }

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
            ErrorMgr.AddError( VSP_INVALID_INPUT_VAL, "AxisProjVecPnt01Guess::Input size mismatch." );
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

    std::vector < bool > ret;
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

    std::vector < vec3d > pts;
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
            ErrorMgr.AddError( VSP_INVALID_INPUT_VAL, "CompVecPntRST::Input size mismatch." );
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
            ErrorMgr.AddError( VSP_INVALID_INPUT_VAL, "FindRSTVecGuess::Input size mismatch." );
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
                ErrorMgr.AddError( VSP_INDEX_OUT_RANGE, "ConvertRSTtoLMNVec::Invalid surf index " + to_string( surf_indx ) );
                return;
            }
        }
        else
        {
            ErrorMgr.AddError( VSP_INVALID_INPUT_VAL, "ConvertRSTtoLMNVec::Input size mismatch." );
            return;
        }
    }
    else
    {
        ErrorMgr.AddError( VSP_INVALID_GEOM_ID, "ConvertRSTtoLMNVec::Can't Find Geom " + geom_id );
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
                    surf->ConvertLMNtoRST( l_vec[i], m_vec[i], n_vec[i], r_out_vec[i], s_out_vec[i], t_out_vec[i] );
                }
            }
            else
            {
                ErrorMgr.AddError( VSP_INDEX_OUT_RANGE, "ConvertLMNtoRSTVec::Invalid surf index " + to_string( surf_indx ) );
                return;
            }
        }
        else
        {
            ErrorMgr.AddError( VSP_INVALID_INPUT_VAL, "ConvertLMNtoRSTVec::Input size mismatch." );
            return;
        }
    }
    else
    {
        ErrorMgr.AddError( VSP_INVALID_GEOM_ID, "ConvertLMNtoRSTVec::Can't Find Geom " + geom_id );
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

// Report a Geom or surface a measure can't be placed on.
static bool MeasureSurfFound( const std::string & geom_id, int surf_indx, const std::string & caller )
{
    Geom* geom_ptr = GetVehicle()->FindGeom( geom_id );
    if ( !geom_ptr )
    {
        ErrorMgr.AddError( VSP_INVALID_GEOM_ID, caller + "::Can't Find Geom " + geom_id );
        return false;
    }

    // A Geom with no surface places the measure at its origin
    if ( surf_indx < 0 || ( geom_ptr->GetNumTotalSurfs() > 0 && surf_indx >= geom_ptr->GetNumTotalSurfs() ) )
    {
        ErrorMgr.AddError( VSP_INDEX_OUT_RANGE, caller + "::Surface Index " + to_string( surf_indx ) + " Out of Range for Geom " + geom_id );
        return false;
    }

    return true;
}

std::string AddProtractor( const std::string & startgeomid, int startsurfindx, double startu, double startw,
                     const std::string & midgeomid, int midsurfindx, double midu, double midw,
                     const std::string & endgeomid, int endsurfindx, double endu, double endw, const std::string & name )
{
    if ( !MeasureSurfFound( startgeomid, startsurfindx, "AddProtractor" ) ||
         !MeasureSurfFound( midgeomid, midsurfindx, "AddProtractor" ) ||
         !MeasureSurfFound( endgeomid, endsurfindx, "AddProtractor" ) )
    {
        return std::string();
    }

    ErrorMgr.NoError();
    return MeasureMgr.CreateAndAddProtractor( startgeomid, startsurfindx, startu, startw,
                                              midgeomid, midsurfindx, midu, midw,
                                              endgeomid, endsurfindx, endu, endw, name );
}

std::vector < std::string > GetAllProtractors()
{
    ErrorMgr.NoError();
    return MeasureMgr.GetAllProtractors();
}

void DelProtractor( const std::string &id )
{
    if ( !MeasureMgr.GetProtractor( id ) )
    {
        ErrorMgr.AddError( VSP_INVALID_ID, "DelProtractor::Can't Find Protractor " + id );
        return;
    }

    MeasureMgr.DelProtractor( id );
    ErrorMgr.NoError();
}

void DeleteAllProtractors()
{
    MeasureMgr.DelAllProtractors();
    ErrorMgr.NoError();
}

//==== The Geom ID a measure keeps for one of its points, reporting through the named
//==== caller when the measure has no such point.  The point is 0 for the origin, 1
//==== for the middle and 2 for the end. ====//
std::string* FindMeasureGeomID( const std::string & measure_id, int which, const std::string & caller )
{
    ParmContainer* pc = ParmMgr.FindParmContainer( measure_id );

    Ruler* ruler = dynamic_cast < Ruler* > ( pc );
    Probe* probe = dynamic_cast < Probe* > ( pc );
    RSTProbe* rst_probe = dynamic_cast < RSTProbe* > ( pc );
    Protractor* protractor = dynamic_cast < Protractor* > ( pc );

    if ( !ruler && !probe && !rst_probe && !protractor )
    {
        ErrorMgr.AddError( VSP_INVALID_ID, caller + "::Can't Find Measure " + measure_id );
        return nullptr;
    }

    if ( which == 0 )
    {
        if ( ruler )
        {
            return &ruler->m_OriginGeomID;
        }
        if ( probe )
        {
            return &probe->m_OriginGeomID;
        }
        if ( rst_probe )
        {
            return &rst_probe->m_OriginGeomID;
        }
        return &protractor->m_OriginGeomID;
    }

    if ( which == 1 && protractor )
    {
        return &protractor->m_MidGeomID;
    }

    if ( which == 2 && ruler )
    {
        return &ruler->m_EndGeomID;
    }
    if ( which == 2 && protractor )
    {
        return &protractor->m_EndGeomID;
    }

    ErrorMgr.AddError( VSP_INVALID_TYPE, caller + "::Measure " + measure_id + " has no such point" );
    return nullptr;
}

void SetMeasureOriginGeomID( const std::string & measure_id, const std::string & geom_id )
{
    std::string* id = FindMeasureGeomID( measure_id, 0, "SetMeasureOriginGeomID" );
    if ( !id || !GeomIDOrNone( geom_id, "SetMeasureOriginGeomID" ) )
    {
        return;
    }

    *id = geom_id;
    ErrorMgr.NoError();
}

std::string GetMeasureOriginGeomID( const std::string & measure_id )
{
    std::string* id = FindMeasureGeomID( measure_id, 0, "GetMeasureOriginGeomID" );
    if ( !id )
    {
        return std::string();
    }

    ErrorMgr.NoError();
    return *id;
}

void SetMeasureMidGeomID( const std::string & measure_id, const std::string & geom_id )
{
    std::string* id = FindMeasureGeomID( measure_id, 1, "SetMeasureMidGeomID" );
    if ( !id || !GeomIDOrNone( geom_id, "SetMeasureMidGeomID" ) )
    {
        return;
    }

    *id = geom_id;
    ErrorMgr.NoError();
}

std::string GetMeasureMidGeomID( const std::string & measure_id )
{
    std::string* id = FindMeasureGeomID( measure_id, 1, "GetMeasureMidGeomID" );
    if ( !id )
    {
        return std::string();
    }

    ErrorMgr.NoError();
    return *id;
}

void SetMeasureEndGeomID( const std::string & measure_id, const std::string & geom_id )
{
    std::string* id = FindMeasureGeomID( measure_id, 2, "SetMeasureEndGeomID" );
    if ( !id || !GeomIDOrNone( geom_id, "SetMeasureEndGeomID" ) )
    {
        return;
    }

    *id = geom_id;
    ErrorMgr.NoError();
}

std::string GetMeasureEndGeomID( const std::string & measure_id )
{
    std::string* id = FindMeasureGeomID( measure_id, 2, "GetMeasureEndGeomID" );
    if ( !id )
    {
        return std::string();
    }

    ErrorMgr.NoError();
    return *id;
}

std::string AddRSTProbe( const std::string & geomid, int surfindx, double r, double s, double t, const std::string & name )
{
    if ( !MeasureSurfFound( geomid, surfindx, "AddRSTProbe" ) )
    {
        return std::string();
    }

    ErrorMgr.NoError();
    return MeasureMgr.CreateAndAddRSTProbe( geomid, surfindx, r, s, t, name );
}

std::vector < std::string > GetAllRSTProbes()
{
    ErrorMgr.NoError();
    return MeasureMgr.GetAllRSTProbes();
}

void DelRSTProbe( const std::string &id )
{
    if ( !MeasureMgr.GetRSTProbe( id ) )
    {
        ErrorMgr.AddError( VSP_INVALID_ID, "DelRSTProbe::Can't Find RST Probe " + id );
        return;
    }

    MeasureMgr.DelRSTProbe( id );
    ErrorMgr.NoError();
}

void DeleteAllRSTProbes()
{
    MeasureMgr.DelAllRSTProbes();
    ErrorMgr.NoError();
}

std::vector < vec3d > ControlSurfaceHingeLine( const std::string & id, int surf_indx )
{
    Vehicle* veh = GetVehicle();
    vector < vec3d > pts;

    SSControlSurf* cs = dynamic_cast< SSControlSurf* > ( SubSurfaceMgr.GetSubSurf( id ) );
    Geom *h = veh->FindGeom( id );
    if ( h && !Geom::CastTo< JointRole >( h ) )
    {
        h = nullptr;
    }

    if ( !h && !cs )
    {
        ErrorMgr.AddError( VSP_INVALID_GEOM_ID, "ControlSurfaceHingeLine::ID is not a control surface or hinge " + id );
        return pts;
    }

    if ( cs ) // If control surface is a valid subsurface
    {
        string gid = cs->GetParentContainer();
        Geom *geom_ptr = veh->FindGeom( gid );
        if ( geom_ptr )
        {
            if ( surf_indx < 0 || surf_indx >= geom_ptr->GetNumTotalSurfs() ) // Then make sure surf_index is valid for geom_ptr
            {
                ErrorMgr.AddError( VSP_INDEX_OUT_RANGE, "ControlSurfaceHingeLine::Invalid Surface Index " + to_string( surf_indx ) );
                return pts;
            }
        }
        else
        {
            ErrorMgr.AddError( VSP_INVALID_PTR, "ControlSurfaceHingeLine::Control surface parent geom ID invalid " + gid );
            return pts;
        }
    }

    pts = veh->ControlSurfaceHingeLine( id, surf_indx );

    ErrorMgr.NoError();
    return pts;
}

std::string AddRuler( const std::string & startgeomid, int startsurfindx, double startu, double startw,
                 const std::string & endgeomid, int endsurfindx, double endu, double endw, const std::string & name )
{
    if ( !MeasureSurfFound( startgeomid, startsurfindx, "AddRuler" ) ||
         !MeasureSurfFound( endgeomid, endsurfindx, "AddRuler" ) )
    {
        return std::string();
    }

    ErrorMgr.NoError();
    return MeasureMgr.CreateAndAddRuler( startgeomid, startsurfindx, startu, startw,
                                         endgeomid, endsurfindx, endu, endw, name );
}

std::vector < std::string > GetAllRulers()
{
    ErrorMgr.NoError();
    return MeasureMgr.GetAllRulers();
}

void DelRuler( const std::string &id )
{
    if ( !MeasureMgr.GetRuler( id ) )
    {
        ErrorMgr.AddError( VSP_INVALID_ID, "DelRuler::Can't Find Ruler " + id );
        return;
    }

    MeasureMgr.DelRuler( id );
    ErrorMgr.NoError();
}

void DeleteAllRulers()
{
    MeasureMgr.DelAllRulers();
    ErrorMgr.NoError();
}

std::string AddProbe( const std::string & geomid, int surfindx, double u, double w, const std::string & name )
{
    if ( !MeasureSurfFound( geomid, surfindx, "AddProbe" ) )
    {
        return std::string();
    }

    ErrorMgr.NoError();
    return MeasureMgr.CreateAndAddProbe( geomid, surfindx, u, w, name );
}

std::vector < std::string > GetAllProbes()
{
    ErrorMgr.NoError();
    return MeasureMgr.GetAllProbes();
}

void DelProbe( const std::string &id )
{
    if ( !MeasureMgr.GetProbe( id ) )
    {
        ErrorMgr.AddError( VSP_INVALID_ID, "DelProbe::Can't Find Probe " + id );
        return;
    }

    MeasureMgr.DelProbe( id );
    ErrorMgr.NoError();
}

void DeleteAllProbes()
{
    MeasureMgr.DelAllProbes();
    ErrorMgr.NoError();
}

//===================================================================//
//========================  General Utilities  ======================//
//===================================================================//

// The bodies live in VspUtil.h and VspUtil.cpp, next to the MAX, MIN and DEG2RAD macros they are
// the function forms of.  These are the wrappers that put them in the API.

void Print( const std::string & data, bool new_line )
{
    ::Print( data, new_line );
    ErrorMgr.NoError();
}

void Print( const vec3d & data, bool new_line )
{
    ::Print( data, new_line );
    ErrorMgr.NoError();
}

void Print( double data, bool new_line )
{
    ::Print( data, new_line );
    ErrorMgr.NoError();
}

void Print( int data, bool new_line )
{
    ::Print( data, new_line );
    ErrorMgr.NoError();
}

double Min( double x, double y )
{
    ErrorMgr.NoError();
    return ::Min( x, y );
}

double Max( double x, double y )
{
    ErrorMgr.NoError();
    return ::Max( x, y );
}

double Rad2Deg( double r )
{
    ErrorMgr.NoError();
    return ::Rad2Deg( r );
}

double Deg2Rad( double d )
{
    ErrorMgr.NoError();
    return ::Deg2Rad( d );
}

std::string GetVSPVersion()
{
    ErrorMgr.NoError();
    return std::string( VSPVERSION4 );
}

int GetVSPVersionMajor()
{
    ErrorMgr.NoError();
    return VSP_VERSION_MAJOR;
}

int GetVSPVersionMinor()
{
    ErrorMgr.NoError();
    return VSP_VERSION_MINOR;
}

int GetVSPVersionChange()
{
    ErrorMgr.NoError();
    return VSP_VERSION_CHANGE;
}

std::string GetVSPExePath()
{
    Vehicle* veh = VehicleMgr.GetVehicle();
    if ( veh )
    {
        ErrorMgr.NoError();
        return veh->GetExePath();
    }
    ErrorMgr.AddError( VSP_INVALID_PTR, "GetVSPExePath::Can't Find Vehicle" );
    return std::string();
}

bool SetVSPAEROPath( const std::string & path )
{
    Vehicle* veh = VehicleMgr.GetVehicle();
    if( veh )
    {
        ErrorMgr.NoError();
        return veh->SetVSPAEROPath( path );
    }
    ErrorMgr.AddError( VSP_INVALID_PTR, "SetVSPAEROPath::Can't Find Vehicle" );
    return false;
}

std::string GetVSPAEROPath()
{
    Vehicle* veh = VehicleMgr.GetVehicle();
    if( veh )
    {
        ErrorMgr.NoError();
        return veh->GetVSPAEROPath();
    }
    ErrorMgr.AddError( VSP_INVALID_PTR, "GetVSPAEROPath::Can't Find Vehicle" );
    return std::string();
}

bool CheckForVSPAERO( const std::string & path )
{
    Vehicle* veh = VehicleMgr.GetVehicle();
    if( veh )
    {
        ErrorMgr.NoError();
        return veh->CheckForVSPAERO( path );
    }
    ErrorMgr.AddError( VSP_INVALID_PTR, "CheckForVSPAERO::Can't Find Vehicle" );
    return false;
}

bool SetVSPHelpPath( const std::string & path )
{
    Vehicle* veh = VehicleMgr.GetVehicle();
    if( veh )
    {
        ErrorMgr.NoError();
        return veh->SetHelpPath( path );
    }
    ErrorMgr.AddError( VSP_INVALID_PTR, "SetVSPHelpPath::Can't Find Vehicle" );
    return false;
}

std::string GetVSPHelpPath()
{
    Vehicle* veh = VehicleMgr.GetVehicle();
    if( veh )
    {
        ErrorMgr.NoError();
        return veh->GetHelpPath();
    }
    ErrorMgr.AddError( VSP_INVALID_PTR, "GetVSPHelpPath::Can't Find Vehicle" );
    return std::string();
}

bool CheckForVSPHelp( const std::string & path )
{
    Vehicle* veh = VehicleMgr.GetVehicle();
    if( veh )
    {
        ErrorMgr.NoError();
        return veh->CheckForHelp( path );
    }
    ErrorMgr.AddError( VSP_INVALID_PTR, "CheckForVSPHelp::Can't Find Vehicle" );
    return false;
}

//============ Geometry Analysis Az/El Functions =====================//

//==== Look up a Geometry Analysis case by ID, reporting through the named
//==== caller. ====//
GeometryAnalysisCase* FindGeometryAnalysis( const std::string & geom_analysis_id, const std::string & caller )
{
    GeometryAnalysisCase* gcase = GeometryAnalysisMgr.GetGeometryAnalysis( geom_analysis_id );

    if ( !gcase )
    {
        ErrorMgr.AddError( VSP_INVALID_ID, caller + "::Invalid Geometry Analysis ID " + geom_analysis_id );
    }

    return gcase;
}

std::string AddGeometryAnalysisAzEl( const std::string & geom_analysis_id, double azimuth, double elevation )
{
    GeometryAnalysisCase* gcase = FindGeometryAnalysis( geom_analysis_id, "AddGeometryAnalysisAzEl" );
    if ( !gcase )
    {
        return std::string();
    }

    gcase->AddAzEl( azimuth, elevation );

    ErrorMgr.NoError();
    return gcase->m_VizAzimuthVec.back()->GetID();
}

void DeleteGeometryAnalysisAzEl( const std::string & geom_analysis_id, int index )
{
    GeometryAnalysisCase* gcase = FindGeometryAnalysis( geom_analysis_id, "DeleteGeometryAnalysisAzEl" );
    if ( !gcase )
    {
        return;
    }

    if ( index < 0 || index >= ( int )gcase->m_VizAzimuthVec.size() )
    {
        ErrorMgr.AddError( VSP_INDEX_OUT_RANGE, "DeleteGeometryAnalysisAzEl::Index " + to_string( index ) + " Out of Range" );
        return;
    }

    gcase->RemoveAzEl( index );

    ErrorMgr.NoError();
}

void DeleteGeometryAnalysisAzElByID( const std::string & geom_analysis_id, const std::string & azimuth_parm_id )
{
    GeometryAnalysisCase* gcase = FindGeometryAnalysis( geom_analysis_id, "DeleteGeometryAnalysisAzElByID" );
    if ( !gcase )
    {
        return;
    }

    for ( int i = 0; i < ( int )gcase->m_VizAzimuthVec.size(); i++ )
    {
        if ( gcase->m_VizAzimuthVec[i]->GetID() == azimuth_parm_id )
        {
            gcase->RemoveAzEl( i );
            ErrorMgr.NoError();
            return;
        }
    }

    ErrorMgr.AddError( VSP_INVALID_ID, "DeleteGeometryAnalysisAzElByID::Parm " + azimuth_parm_id + " is not an azimuth of " + geom_analysis_id );
}

void DeleteAllGeometryAnalysisAzEl( const std::string & geom_analysis_id )
{
    GeometryAnalysisCase* gcase = FindGeometryAnalysis( geom_analysis_id, "DeleteAllGeometryAnalysisAzEl" );
    if ( !gcase )
    {
        return;
    }

    gcase->RemoveAllAzEl();

    ErrorMgr.NoError();
}

int GetNumGeometryAnalysisAzEl( const std::string & geom_analysis_id )
{
    GeometryAnalysisCase* gcase = FindGeometryAnalysis( geom_analysis_id, "GetNumGeometryAnalysisAzEl" );
    if ( !gcase )
    {
        return -1;
    }

    ErrorMgr.NoError();
    return ( int )gcase->m_VizAzimuthVec.size();
}

std::string GetGeometryAnalysisAzimuthParm( const std::string & geom_analysis_id, int index )
{
    GeometryAnalysisCase* gcase = FindGeometryAnalysis( geom_analysis_id, "GetGeometryAnalysisAzimuthParm" );
    if ( !gcase )
    {
        return std::string();
    }

    if ( index < 0 || index >= ( int )gcase->m_VizAzimuthVec.size() )
    {
        ErrorMgr.AddError( VSP_INDEX_OUT_RANGE, "GetGeometryAnalysisAzimuthParm::Index " + to_string( index ) + " Out of Range" );
        return std::string();
    }

    ErrorMgr.NoError();
    return gcase->m_VizAzimuthVec[ index ]->GetID();
}

std::string GetGeometryAnalysisElevationParm( const std::string & geom_analysis_id, int index )
{
    GeometryAnalysisCase* gcase = FindGeometryAnalysis( geom_analysis_id, "GetGeometryAnalysisElevationParm" );
    if ( !gcase )
    {
        return std::string();
    }

    if ( index < 0 || index >= ( int )gcase->m_VizElevationVec.size() )
    {
        ErrorMgr.AddError( VSP_INDEX_OUT_RANGE, "GetGeometryAnalysisElevationParm::Index " + to_string( index ) + " Out of Range" );
        return std::string();
    }

    ErrorMgr.NoError();
    return gcase->m_VizElevationVec[ index ]->GetID();
}

std::vector < std::string > GetGeometryAnalysisAzimuthParmVec( const std::string & geom_analysis_id )
{
    std::vector < std::string > ret_vec;

    GeometryAnalysisCase* gcase = FindGeometryAnalysis( geom_analysis_id, "GetGeometryAnalysisAzimuthParmVec" );
    if ( !gcase )
    {
        return ret_vec;
    }

    for ( size_t i = 0; i < gcase->m_VizAzimuthVec.size(); i++ )
    {
        ret_vec.push_back( gcase->m_VizAzimuthVec[i]->GetID() );
    }

    ErrorMgr.NoError();
    return ret_vec;
}

std::vector < std::string > GetGeometryAnalysisElevationParmVec( const std::string & geom_analysis_id )
{
    std::vector < std::string > ret_vec;

    GeometryAnalysisCase* gcase = FindGeometryAnalysis( geom_analysis_id, "GetGeometryAnalysisElevationParmVec" );
    if ( !gcase )
    {
        return ret_vec;
    }

    for ( size_t i = 0; i < gcase->m_VizElevationVec.size(); i++ )
    {
        ret_vec.push_back( gcase->m_VizElevationVec[i]->GetID() );
    }

    ErrorMgr.NoError();
    return ret_vec;
}

void SetGeometryAnalysisPrimaryModeID( const std::string & geom_analysis_id, const std::string & mode_id )
{
    GeometryAnalysisCase* gcase = FindGeometryAnalysis( geom_analysis_id, "SetGeometryAnalysisPrimaryModeID" );
    if ( !gcase )
    {
        return;
    }

    if ( !mode_id.empty() && !ModeMgr.GetMode( mode_id ) )
    {
        ErrorMgr.AddError( VSP_INVALID_ID, "SetGeometryAnalysisPrimaryModeID::Can't Find Mode " + mode_id );
        return;
    }

    gcase->m_PrimaryModeID = mode_id;
    ErrorMgr.NoError();
}

std::string GetGeometryAnalysisPrimaryModeID( const std::string & geom_analysis_id )
{
    GeometryAnalysisCase* gcase = FindGeometryAnalysis( geom_analysis_id, "GetGeometryAnalysisPrimaryModeID" );
    if ( !gcase )
    {
        return std::string();
    }

    ErrorMgr.NoError();
    return gcase->m_PrimaryModeID;
}

void SetGeometryAnalysisPrimaryGeomID( const std::string & geom_analysis_id, const std::string & geom_id )
{
    GeometryAnalysisCase* gcase = FindGeometryAnalysis( geom_analysis_id, "SetGeometryAnalysisPrimaryGeomID" );
    if ( !gcase || !GeomIDOrNone( geom_id, "SetGeometryAnalysisPrimaryGeomID" ) )
    {
        return;
    }

    gcase->m_PrimaryGeomID = geom_id;
    ErrorMgr.NoError();
}

std::string GetGeometryAnalysisPrimaryGeomID( const std::string & geom_analysis_id )
{
    GeometryAnalysisCase* gcase = FindGeometryAnalysis( geom_analysis_id, "GetGeometryAnalysisPrimaryGeomID" );
    if ( !gcase )
    {
        return std::string();
    }

    ErrorMgr.NoError();
    return gcase->m_PrimaryGeomID;
}

void SetGeometryAnalysisSecondaryGeomID( const std::string & geom_analysis_id, const std::string & geom_id )
{
    GeometryAnalysisCase* gcase = FindGeometryAnalysis( geom_analysis_id, "SetGeometryAnalysisSecondaryGeomID" );
    if ( !gcase || !GeomIDOrNone( geom_id, "SetGeometryAnalysisSecondaryGeomID" ) )
    {
        return;
    }

    gcase->m_SecondaryGeomID = geom_id;
    ErrorMgr.NoError();
}

std::string GetGeometryAnalysisSecondaryGeomID( const std::string & geom_analysis_id )
{
    GeometryAnalysisCase* gcase = FindGeometryAnalysis( geom_analysis_id, "GetGeometryAnalysisSecondaryGeomID" );
    if ( !gcase )
    {
        return std::string();
    }

    ErrorMgr.NoError();
    return gcase->m_SecondaryGeomID;
}

void SetGeometryAnalysisDirectionGeomID( const std::string & geom_analysis_id, const std::string & geom_id )
{
    GeometryAnalysisCase* gcase = FindGeometryAnalysis( geom_analysis_id, "SetGeometryAnalysisDirectionGeomID" );
    if ( !gcase || !GeomIDOrNone( geom_id, "SetGeometryAnalysisDirectionGeomID" ) )
    {
        return;
    }

    gcase->m_DirectionGeomID = geom_id;
    ErrorMgr.NoError();
}

std::string GetGeometryAnalysisDirectionGeomID( const std::string & geom_analysis_id )
{
    GeometryAnalysisCase* gcase = FindGeometryAnalysis( geom_analysis_id, "GetGeometryAnalysisDirectionGeomID" );
    if ( !gcase )
    {
        return std::string();
    }

    ErrorMgr.NoError();
    return gcase->m_DirectionGeomID;
}

void AddGeometryAnalysisCutout( const std::string & geom_analysis_id, const std::string & ss_id )
{
    GeometryAnalysisCase* gcase = FindGeometryAnalysis( geom_analysis_id, "AddGeometryAnalysisCutout" );
    if ( !gcase )
    {
        return;
    }

    if ( !SubSurfaceMgr.GetSubSurf( ss_id ) )
    {
        ErrorMgr.AddError( VSP_INVALID_ID, "AddGeometryAnalysisCutout::Can't Find SubSurface " + ss_id );
        return;
    }

    if ( !vector_contains_val( gcase->m_CutoutVec, ss_id ) )
    {
        gcase->m_CutoutVec.push_back( ss_id );
    }
    ErrorMgr.NoError();
}

void DeleteGeometryAnalysisCutout( const std::string & geom_analysis_id, const std::string & ss_id )
{
    GeometryAnalysisCase* gcase = FindGeometryAnalysis( geom_analysis_id, "DeleteGeometryAnalysisCutout" );
    if ( !gcase )
    {
        return;
    }

    if ( !vector_contains_val( gcase->m_CutoutVec, ss_id ) )
    {
        ErrorMgr.AddError( VSP_INVALID_ID, "DeleteGeometryAnalysisCutout::SubSurface " + ss_id + " is not a cutout of " + geom_analysis_id );
        return;
    }

    vector_remove_val( gcase->m_CutoutVec, ss_id );
    ErrorMgr.NoError();
}

std::vector < std::string > GetGeometryAnalysisCutoutVec( const std::string & geom_analysis_id )
{
    GeometryAnalysisCase* gcase = FindGeometryAnalysis( geom_analysis_id, "GetGeometryAnalysisCutoutVec" );
    if ( !gcase )
    {
        return std::vector < std::string > ();
    }

    ErrorMgr.NoError();
    return gcase->m_CutoutVec;
}

//================ Measure Tool Display Functions ====================//

void ShowAllRulers()
{
    MeasureMgr.ShowAllRulers();
    ErrorMgr.NoError();
}

void HideAllRulers()
{
    MeasureMgr.HideAllRulers();
    ErrorMgr.NoError();
}

void ShowAllProbes()
{
    MeasureMgr.ShowAllProbes();
    ErrorMgr.NoError();
}

void HideAllProbes()
{
    MeasureMgr.HideAllProbes();
    ErrorMgr.NoError();
}

void ShowAllProtractors()
{
    MeasureMgr.ShowAllProtractors();
    ErrorMgr.NoError();
}

void HideAllProtractors()
{
    MeasureMgr.HideAllProtractors();
    ErrorMgr.NoError();
}

void ShowAllRSTProbes()
{
    MeasureMgr.ShowAllRSTProbes();
    ErrorMgr.NoError();
}

void HideAllRSTProbes()
{
    MeasureMgr.HideAllRSTProbes();
    ErrorMgr.NoError();
}

//================ FEA Trim Part Functions ====================//

//==== Look up an FEA Trim part by ID, reporting through the named caller. ====//
FeaPartTrim* FindFeaPartTrim( const std::string & part_id, const std::string & caller )
{
    FeaPart* part = StructureMgr.GetFeaPart( part_id );

    if ( !part )
    {
        ErrorMgr.AddError( VSP_INVALID_ID, caller + "::Invalid FEA Part ID " + part_id );
        return nullptr;
    }

    FeaPartTrim* trim = dynamic_cast< FeaPartTrim* >( part );

    if ( !trim )
    {
        ErrorMgr.AddError( VSP_INVALID_TYPE, caller + "::FEA Part " + part_id + " is not a Trim part" );
        return nullptr;
    }

    return trim;
}

void AddFeaTrimPart( const std::string & trim_id, const std::string & part_id )
{
    FeaPartTrim* trim = FindFeaPartTrim( trim_id, "AddFeaTrimPart" );
    if ( !trim )
    {
        return;
    }

    if ( !StructureMgr.GetFeaPart( part_id ) )
    {
        ErrorMgr.AddError( VSP_INVALID_ID, "AddFeaTrimPart::Invalid FEA Part ID " + part_id );
        return;
    }

    trim->AddTrimPart( part_id );

    ErrorMgr.NoError();
}

void DeleteFeaTrimPart( const std::string & trim_id, int index )
{
    FeaPartTrim* trim = FindFeaPartTrim( trim_id, "DeleteFeaTrimPart" );
    if ( !trim )
    {
        return;
    }

    if ( index < 0 || index >= ( int )trim->m_TrimFeaPartIDVec.size() )
    {
        ErrorMgr.AddError( VSP_INDEX_OUT_RANGE, "DeleteFeaTrimPart::Trim Part Index " + to_string( index ) + " Out of Range" );
        return;
    }

    trim->DeleteTrimPart( index );

    ErrorMgr.NoError();
}

void DeleteFeaTrimPartByID( const std::string & trim_id, const std::string & part_id )
{
    FeaPartTrim* trim = FindFeaPartTrim( trim_id, "DeleteFeaTrimPartByID" );
    if ( !trim )
    {
        return;
    }

    int index = vector_find_val( trim->m_TrimFeaPartIDVec, part_id );
    if ( index < 0 )
    {
        ErrorMgr.AddError( VSP_INVALID_ID, "DeleteFeaTrimPartByID::FEA Part " + part_id + " does not trim " + trim_id );
        return;
    }

    trim->DeleteTrimPart( index );

    ErrorMgr.NoError();
}

std::vector < std::string > GetFeaTrimPartIDVec( const std::string & trim_id )
{
    std::vector < std::string > ret;

    FeaPartTrim* trim = FindFeaPartTrim( trim_id, "GetFeaTrimPartIDVec" );
    if ( !trim )
    {
        return ret;
    }

    ErrorMgr.NoError();
    return trim->m_TrimFeaPartIDVec;
}

//==================== Geom Operation Functions ======================//

//==== Look up a Geom by ID, reporting through the named caller. ====//
Geom* FindGeomForOp( const std::string & geom_id, const std::string & caller )
{
    Vehicle* veh = GetVehicle();
    Geom* geom_ptr = veh->FindGeom( geom_id );
    if ( !geom_ptr )
    {
        ErrorMgr.AddError( VSP_INVALID_GEOM_ID, caller + "::Can't Find Geom " + geom_id );
        return nullptr;
    }
    return geom_ptr;
}

static void CopyAirfoilImpl( const std::string & geom_id, int index, const std::string & caller )
{
    Geom* geom_ptr = FindGeomForOp( geom_id, caller );
    if ( !geom_ptr )
    {
        return;
    }

    WingGeom* wg = dynamic_cast< WingGeom* >( geom_ptr );
    if ( !wg )
    {
        ErrorMgr.AddError( VSP_WRONG_GEOM_TYPE, caller + "::Geom " + geom_id + " is not a wing" );
        return;
    }

    if ( index < 0 || index >= wg->NumXSec() )
    {
        ErrorMgr.AddError( VSP_INDEX_OUT_RANGE, caller + "::Section Index " + to_string( index ) + " Out of Range" );
        return;
    }

    wg->CopyAirfoil( index );

    ErrorMgr.NoError();
}

void CopyAirfoil( const std::string & geom_id, int index )
{
    CopyAirfoilImpl( geom_id, index, "CopyAirfoil" );
}

void CopyAirfoilByID( const std::string & xsec_id )
{
    int index = -1;
    Geom* geom_ptr = FindXSecGeom( xsec_id, index, "CopyAirfoilByID" );
    if ( !geom_ptr )
    {
        return;
    }

    CopyAirfoilImpl( geom_ptr->GetID(), index, "CopyAirfoilByID" );
}

static void PasteAirfoilImpl( const std::string & geom_id, int index, const std::string & caller )
{
    Geom* geom_ptr = FindGeomForOp( geom_id, caller );
    if ( !geom_ptr )
    {
        return;
    }

    WingGeom* wg = dynamic_cast< WingGeom* >( geom_ptr );
    if ( !wg )
    {
        ErrorMgr.AddError( VSP_WRONG_GEOM_TYPE, caller + "::Geom " + geom_id + " is not a wing" );
        return;
    }

    if ( index < 0 || index >= wg->NumXSec() )
    {
        ErrorMgr.AddError( VSP_INDEX_OUT_RANGE, caller + "::Section Index " + to_string( index ) + " Out of Range" );
        return;
    }

    wg->PasteAirfoil( index );
    Update();

    ErrorMgr.NoError();
}

void PasteAirfoil( const std::string & geom_id, int index )
{
    PasteAirfoilImpl( geom_id, index, "PasteAirfoil" );
}

void PasteAirfoilByID( const std::string & xsec_id )
{
    int index = -1;
    Geom* geom_ptr = FindXSecGeom( xsec_id, index, "PasteAirfoilByID" );
    if ( !geom_ptr )
    {
        return;
    }

    PasteAirfoilImpl( geom_ptr->GetID(), index, "PasteAirfoilByID" );
}

static void ClearSkinningImpl( const std::string & geom_id, int index, const std::string & caller )
{
    Geom* geom_ptr = FindGeomForOp( geom_id, caller );
    if ( !geom_ptr )
    {
        return;
    }

    GeomXSec* gxs = dynamic_cast< GeomXSec* >( geom_ptr );
    if ( !gxs )
    {
        ErrorMgr.AddError( VSP_WRONG_GEOM_TYPE, caller + "::Geom " + geom_id + " does not carry skinning" );
        return;
    }

    if ( index != -1 && !gxs->GetXSec( index ) )
    {
        ErrorMgr.AddError( VSP_INDEX_OUT_RANGE, caller + "::XSec Index " + to_string( index ) + " Out of Range" );
        return;
    }

    if ( gxs->ClearSkinning( index ) == 0 )
    {
        ErrorMgr.AddError( VSP_WRONG_GEOM_TYPE, caller + "::Geom " + geom_id + " has no skinned section at index " + to_string( index ) );
        return;
    }
    Update();

    ErrorMgr.NoError();
}

void ClearSkinning( const std::string & geom_id, int index )
{
    ClearSkinningImpl( geom_id, index, "ClearSkinning" );
}

void ClearSkinningByID( const std::string & xsec_id )
{
    int index = -1;
    Geom* geom_ptr = FindXSecGeom( xsec_id, index, "ClearSkinningByID" );
    if ( !geom_ptr )
    {
        return;
    }

    ClearSkinningImpl( geom_ptr->GetID(), index, "ClearSkinningByID" );
}

void ResetGeomScale( const std::string & geom_id )
{
    Geom* geom_ptr = FindGeomForOp( geom_id, "ResetGeomScale" );
    if ( !geom_ptr )
    {
        return;
    }

    geom_ptr->ResetScale();
    Update();

    ErrorMgr.NoError();
}

void AcceptGeomScale( const std::string & geom_id )
{
    Geom* geom_ptr = FindGeomForOp( geom_id, "AcceptGeomScale" );
    if ( !geom_ptr )
    {
        return;
    }

    geom_ptr->AcceptScale();
    Update();

    ErrorMgr.NoError();
}

std::vector < vec3d > GetPtCloudPnts( const std::string & geom_id )
{
    std::vector < vec3d > ret;

    Geom* geom_ptr = FindGeomForOp( geom_id, "GetPtCloudPnts" );
    if ( !geom_ptr )
    {
        return ret;
    }

    PtCloudGeom* pg = dynamic_cast< PtCloudGeom* >( geom_ptr );
    if ( !pg )
    {
        ErrorMgr.AddError( VSP_WRONG_GEOM_TYPE, "GetPtCloudPnts::Geom " + geom_id + " is not a point cloud" );
        return ret;
    }

    ErrorMgr.NoError();
    return pg->m_Pts;
}

std::string CreatePtCloudGeom( const std::string & geom_id )
{
    Geom* geom_ptr = FindGeomForOp( geom_id, "CreatePtCloudGeom" );
    if ( !geom_ptr )
    {
        return std::string();
    }

    MeshGeom* mg = dynamic_cast< MeshGeom* >( geom_ptr );
    if ( !mg )
    {
        ErrorMgr.AddError( VSP_WRONG_GEOM_TYPE, "CreatePtCloudGeom::Geom " + geom_id + " is not a mesh" );
        return std::string();
    }

    // MeshGeom adds the point cloud to the model without handing back its ID,
    // so look for whatever arrived.
    Vehicle* veh = GetVehicle();
    std::vector < std::string > before = veh->GetGeomVec();

    mg->CreatePtCloudGeom();
    Update();

    std::vector < std::string > after = veh->GetGeomVec();

    for ( int i = 0; i < ( int )after.size(); i++ )
    {
        if ( !vector_contains_val( before, after[i] ) )
        {
            ErrorMgr.NoError();
            return after[i];
        }
    }

    ErrorMgr.AddError( VSP_INVALID_PTR, "CreatePtCloudGeom::No Point Cloud Was Created" );
    return std::string();
}

std::string CreateNGonMeshGeom( const std::string & geom_id )
{
    Geom* geom_ptr = FindGeomForOp( geom_id, "CreateNGonMeshGeom" );
    if ( !geom_ptr )
    {
        return std::string();
    }

    MeshGeom* mg = dynamic_cast< MeshGeom* >( geom_ptr );
    if ( !mg )
    {
        ErrorMgr.AddError( VSP_WRONG_GEOM_TYPE, "CreateNGonMeshGeom::Geom " + geom_id + " is not a mesh" );
        return std::string();
    }

    std::string ret = mg->CreateNGonMeshGeom();
    Update();

    if ( ret.length() == 0 )
    {
        ErrorMgr.AddError( VSP_INVALID_PTR, "CreateNGonMeshGeom::No NGon Mesh Was Created" );
        return ret;
    }

    ErrorMgr.NoError();
    return ret;
}

void ProjectPtCloudPts( const std::string & geom_id, const std::string & target_geom_id, int surf_index, int dir_index )
{
    Geom* geom_ptr = FindGeomForOp( geom_id, "ProjectPtCloudPts" );
    if ( !geom_ptr )
    {
        return;
    }

    PtCloudGeom* pt_cloud = dynamic_cast< PtCloudGeom* >( geom_ptr );
    if ( !pt_cloud )
    {
        ErrorMgr.AddError( VSP_WRONG_GEOM_TYPE, "ProjectPtCloudPts::Geom " + geom_id + " is not a point cloud" );
        return;
    }

    Vehicle* veh = GetVehicle();
    Geom* target_ptr = veh->FindGeom( target_geom_id );
    if ( !target_ptr )
    {
        ErrorMgr.AddError( VSP_INVALID_GEOM_ID, "ProjectPtCloudPts::Can't Find Geom " + target_geom_id );
        return;
    }

    if ( surf_index < 0 || surf_index >= target_ptr->GetNumTotalSurfs() )
    {
        ErrorMgr.AddError( VSP_INDEX_OUT_RANGE, "ProjectPtCloudPts::Surface Index Out Of Range " + to_string( surf_index ) );
        return;
    }

    if ( dir_index < X_DIR || dir_index > Z_DIR )
    {
        ErrorMgr.AddError( VSP_INVALID_TYPE, "ProjectPtCloudPts::Invalid Direction " + to_string( dir_index ) );
        return;
    }

    pt_cloud->ProjectPts( target_geom_id, surf_index, dir_index );
    Update();

    ErrorMgr.NoError();
}

std::string CreateConvexHull( const std::string & geom_id )
{
    Geom* geom_ptr = FindGeomForOp( geom_id, "CreateConvexHull" );
    if ( !geom_ptr )
    {
        return std::string();
    }

    PtCloudGeom* pg = dynamic_cast< PtCloudGeom* >( geom_ptr );
    if ( !pg )
    {
        ErrorMgr.AddError( VSP_WRONG_GEOM_TYPE, "CreateConvexHull::Geom " + geom_id + " is not a point cloud" );
        return std::string();
    }

    // The hull arrives as a new MeshGeom rather than replacing the cloud, and
    // it is added without handing back its ID, so look for whatever arrived.
    Vehicle* veh = GetVehicle();
    std::vector < std::string > before = veh->GetGeomVec();

    pg->CreateConvexHull();
    Update();

    std::vector < std::string > after = veh->GetGeomVec();

    for ( int i = 0; i < ( int )after.size(); i++ )
    {
        if ( !vector_contains_val( before, after[i] ) )
        {
            ErrorMgr.NoError();
            return after[i];
        }
    }

    ErrorMgr.AddError( VSP_INVALID_PTR, "CreateConvexHull::No Hull Was Created" );
    return std::string();
}

void ShowSet( int set_index )
{
    Vehicle* veh = GetVehicle();

    if ( set_index < 0 || set_index >= ( int )veh->GetSetNameVec().size() )
    {
        ErrorMgr.AddError( VSP_INDEX_OUT_RANGE, "ShowSet::Set Index " + to_string( set_index ) + " Out of Range" );
        return;
    }

    veh->ShowSet( set_index );

    ErrorMgr.NoError();
}

void NoShowSet( int set_index )
{
    Vehicle* veh = GetVehicle();

    if ( set_index < 0 || set_index >= ( int )veh->GetSetNameVec().size() )
    {
        ErrorMgr.AddError( VSP_INDEX_OUT_RANGE, "NoShowSet::Set Index " + to_string( set_index ) + " Out of Range" );
        return;
    }

    veh->NoShowSet( set_index );

    ErrorMgr.NoError();
}

void ShowOnlySet( int set_index )
{
    Vehicle* veh = GetVehicle();

    if ( set_index < 0 || set_index >= ( int )veh->GetSetNameVec().size() )
    {
        ErrorMgr.AddError( VSP_INDEX_OUT_RANGE, "ShowOnlySet::Set Index " + to_string( set_index ) + " Out of Range" );
        return;
    }

    veh->ShowOnlySet( set_index );

    ErrorMgr.NoError();
}

//============ XSecCurve Clipboard Functions =========================//

static void CopyXSecCurveImpl( const std::string & geom_id, int index, const std::string & caller )
{
    Geom* geom_ptr = FindGeomForOp( geom_id, caller );
    if ( !geom_ptr )
    {
        return;
    }

    BORGeom* bor = dynamic_cast< BORGeom* >( geom_ptr );
    if ( bor )
    {
        // A body of revolution carries exactly one XSecCurve, so the index is
        // not used.
        bor->CopyXSecCurve();

        ErrorMgr.NoError();
        return;
    }

    GeomXSec* gxs = dynamic_cast< GeomXSec* >( geom_ptr );
    if ( !gxs )
    {
        ErrorMgr.AddError( VSP_WRONG_GEOM_TYPE, caller + "::Geom " + geom_id + " carries no XSecCurve" );
        return;
    }

    if ( index < 0 || index >= gxs->GetXSecSurf( 0 )->NumXSec() )
    {
        ErrorMgr.AddError( VSP_INDEX_OUT_RANGE, caller + "::XSec Index " + to_string( index ) + " Out of Range" );
        return;
    }

    gxs->GetXSecSurf( 0 )->CopyXSecCurve( index );

    ErrorMgr.NoError();
}

void CopyXSecCurve( const std::string & geom_id, int index )
{
    CopyXSecCurveImpl( geom_id, index, "CopyXSecCurve" );
}

void CopyXSecCurveByID( const std::string & xsec_id )
{
    int index = -1;
    Geom* geom_ptr = FindXSecGeom( xsec_id, index, "CopyXSecCurveByID" );
    if ( !geom_ptr )
    {
        return;
    }

    CopyXSecCurveImpl( geom_ptr->GetID(), index, "CopyXSecCurveByID" );
}

static void PasteXSecCurveImpl( const std::string & geom_id, int index, const std::string & caller )
{
    Geom* geom_ptr = FindGeomForOp( geom_id, caller );
    if ( !geom_ptr )
    {
        return;
    }

    BORGeom* bor = dynamic_cast< BORGeom* >( geom_ptr );
    if ( bor )
    {
        bor->PasteXSecCurve();
        Update();

        ErrorMgr.NoError();
        return;
    }

    GeomXSec* gxs = dynamic_cast< GeomXSec* >( geom_ptr );
    if ( !gxs )
    {
        ErrorMgr.AddError( VSP_WRONG_GEOM_TYPE, caller + "::Geom " + geom_id + " carries no XSecCurve" );
        return;
    }

    if ( index < 0 || index >= gxs->GetXSecSurf( 0 )->NumXSec() )
    {
        ErrorMgr.AddError( VSP_INDEX_OUT_RANGE, caller + "::XSec Index " + to_string( index ) + " Out of Range" );
        return;
    }

    gxs->GetXSecSurf( 0 )->PasteXSecCurve( index );
    Update();

    ErrorMgr.NoError();
}

void PasteXSecCurve( const std::string & geom_id, int index )
{
    PasteXSecCurveImpl( geom_id, index, "PasteXSecCurve" );
}

void PasteXSecCurveByID( const std::string & xsec_id )
{
    int index = -1;
    Geom* geom_ptr = FindXSecGeom( xsec_id, index, "PasteXSecCurveByID" );
    if ( !geom_ptr )
    {
        return;
    }

    PasteXSecCurveImpl( geom_ptr->GetID(), index, "PasteXSecCurveByID" );
}

//============ FEA Laminate Layer Functions =========================//

//==== Look up an FEA laminate material by ID, reporting through the named
//==== caller. ====//
FeaMaterial* FindFeaLaminate( const std::string & material_id, const std::string & caller )
{
    FeaMaterial* mat = StructureMgr.GetFeaMaterial( material_id );

    if ( !mat )
    {
        ErrorMgr.AddError( VSP_INVALID_ID, caller + "::Invalid FEA Material ID " + material_id );
        return nullptr;
    }

    if ( mat->m_FeaMaterialType() != FEA_LAMINATE )
    {
        ErrorMgr.AddError( VSP_INVALID_TYPE, caller + "::FEA Material " + material_id + " is not a laminate" );
        return nullptr;
    }

    return mat;
}

void ReorderFeaLayer( const std::string & material_id, const std::string & layer_id, int reorder_type )
{
    FeaMaterial* mat = FindFeaLaminate( material_id, "ReorderFeaLayer" );
    if ( !mat )
    {
        return;
    }

    if ( reorder_type < REORDER_MOVE_UP || reorder_type > REORDER_MOVE_BOTTOM )
    {
        ErrorMgr.AddError( VSP_INVALID_TYPE, "ReorderFeaLayer::Invalid Reorder Type " + to_string( reorder_type ) );
        return;
    }

    std::vector < FeaLayer* > lay_vec = mat->GetLayerVec();

    int index = -1;
    for ( int i = 0; i < ( int )lay_vec.size(); i++ )
    {
        if ( lay_vec[i]->GetID() == layer_id )
        {
            index = i;
        }
    }

    if ( index < 0 )
    {
        ErrorMgr.AddError( VSP_INVALID_ID, "ReorderFeaLayer::Invalid FEA Layer ID " + layer_id );
        return;
    }

    // ReorderCurrentLayer works off the material's current layer, so point it at
    // the requested one first.
    mat->SetCurrLayerIndex( index );
    mat->ReorderCurrentLayer( reorder_type );

    ErrorMgr.NoError();
}

std::string AddFeaLayer( const std::string & material_id )
{
    FeaMaterial* mat = FindFeaLaminate( material_id, "AddFeaLayer" );
    if ( !mat )
    {
        return std::string();
    }

    FeaLayer* lay = mat->AddLayer();

    if ( !lay )
    {
        ErrorMgr.AddError( VSP_INVALID_PTR, "AddFeaLayer::Invalid FEA Layer Ptr" );
        return std::string();
    }

    ErrorMgr.NoError();
    return lay->GetID();
}

void DeleteFeaLayer( const std::string & material_id, const std::string & layer_id )
{
    FeaMaterial* mat = FindFeaLaminate( material_id, "DeleteFeaLayer" );
    if ( !mat )
    {
        return;
    }

    if ( !mat->DeleteLayer( layer_id ) )
    {
        ErrorMgr.AddError( VSP_INVALID_ID, "DeleteFeaLayer::Invalid FEA Layer ID " + layer_id );
        return;
    }

    ErrorMgr.NoError();
}

int NumFeaLayers( const std::string & material_id )
{
    FeaMaterial* mat = FindFeaLaminate( material_id, "NumFeaLayers" );
    if ( !mat )
    {
        return -1;
    }

    ErrorMgr.NoError();
    return mat->NumLayers();
}

std::vector < std::string > GetFeaLayerIDVec( const std::string & material_id )
{
    std::vector < std::string > ret;

    FeaMaterial* mat = FindFeaLaminate( material_id, "GetFeaLayerIDVec" );
    if ( !mat )
    {
        return ret;
    }

    std::vector < FeaLayer* > lay_vec = mat->GetLayerVec();

    ret.reserve( lay_vec.size() );
    for ( int i = 0; i < ( int )lay_vec.size(); i++ )
    {
        ret.push_back( lay_vec[i]->GetID() );
    }

    ErrorMgr.NoError();
    return ret;
}

//==== Look up a FEA layer by ID, reporting through the named caller ====//
FeaLayer* FindFeaLayer( const std::string & layer_id, const std::string & caller )
{
    FeaLayer* layer = dynamic_cast < FeaLayer* > ( ParmMgr.FindParmContainer( layer_id ) );
    if ( !layer )
    {
        ErrorMgr.AddError( VSP_INVALID_ID, caller + "::Can't Find FEA Layer " + layer_id );
    }
    return layer;
}

void SetFeaLayerMaterialID( const std::string & layer_id, const std::string & material_id )
{
    FeaLayer* layer = FindFeaLayer( layer_id, "SetFeaLayerMaterialID" );
    if ( !layer )
    {
        return;
    }

    if ( !StructureMgr.GetFeaMaterial( material_id ) )
    {
        ErrorMgr.AddError( VSP_INVALID_ID, "SetFeaLayerMaterialID::Can't Find FEA Material " + material_id );
        return;
    }

    layer->m_FeaMaterialID = material_id;
    ErrorMgr.NoError();
}

std::string GetFeaLayerMaterialID( const std::string & layer_id )
{
    FeaLayer* layer = FindFeaLayer( layer_id, "GetFeaLayerMaterialID" );
    if ( !layer )
    {
        return std::string();
    }

    ErrorMgr.NoError();
    return layer->m_FeaMaterialID;
}

//======================= Parm Link Functions ============================//

//==== Look up a Parm Link by ID, reporting through the named caller. ====//
Link* FindParmLink( const std::string & link_id, const std::string & caller )
{
    for ( int i = 0; i < LinkMgr.GetNumLinks(); i++ )
    {
        Link* lnk = LinkMgr.GetLink( i );
        if ( lnk && lnk->GetID() == link_id )
        {
            return lnk;
        }
    }

    ErrorMgr.AddError( VSP_INVALID_ID, caller + "::Can't Find Parm Link " + link_id );
    return nullptr;
}

std::string AddParmLink( const std::string & parm_a_id, const std::string & parm_b_id )
{
    if ( !ParmMgr.FindParm( parm_a_id ) )
    {
        ErrorMgr.AddError( VSP_CANT_FIND_PARM, "AddParmLink::Can't Find Parm " + parm_a_id );
        return std::string();
    }

    if ( !ParmMgr.FindParm( parm_b_id ) )
    {
        ErrorMgr.AddError( VSP_CANT_FIND_PARM, "AddParmLink::Can't Find Parm " + parm_b_id );
        return std::string();
    }

    // AddLink refuses a pairing it already holds.
    if ( !LinkMgr.AddLink( parm_a_id, parm_b_id ) )
    {
        ErrorMgr.AddError( VSP_INVALID_INPUT_VAL, "AddParmLink::Can't Link " + parm_a_id + " To " + parm_b_id );
        return std::string();
    }

    Link* lnk = LinkMgr.GetLink( LinkMgr.GetNumLinks() - 1 );

    if ( !lnk )
    {
        ErrorMgr.AddError( VSP_INVALID_PTR, "AddParmLink::Invalid Link Ptr" );
        return std::string();
    }

    ErrorMgr.NoError();
    return lnk->GetID();
}

int GetNumParmLinks()
{
    ErrorMgr.NoError();
    return LinkMgr.GetNumLinks();
}

std::string GetParmLinkID( int index )
{
    if ( index < 0 || index >= LinkMgr.GetNumLinks() )
    {
        ErrorMgr.AddError( VSP_INDEX_OUT_RANGE, "GetParmLinkID::Link Index " + to_string( index ) + " Out of Range" );
        return std::string();
    }

    Link* lnk = LinkMgr.GetLink( index );

    if ( !lnk )
    {
        ErrorMgr.AddError( VSP_INVALID_PTR, "GetParmLinkID::Invalid Link Ptr" );
        return std::string();
    }

    ErrorMgr.NoError();
    return lnk->GetID();
}

std::vector < std::string > GetAllParmLinks()
{
    std::vector < std::string > ret_vec;

    for ( int i = 0; i < LinkMgr.GetNumLinks(); i++ )
    {
        Link* lnk = LinkMgr.GetLink( i );
        if ( lnk )
        {
            ret_vec.push_back( lnk->GetID() );
        }
    }

    ErrorMgr.NoError();
    return ret_vec;
}

std::string GetParmLinkAParm( const std::string & link_id )
{
    Link* lnk = FindParmLink( link_id, "GetParmLinkAParm" );
    if ( !lnk )
    {
        return std::string();
    }

    ErrorMgr.NoError();
    return lnk->GetParmA();
}

std::string GetParmLinkBParm( const std::string & link_id )
{
    Link* lnk = FindParmLink( link_id, "GetParmLinkBParm" );
    if ( !lnk )
    {
        return std::string();
    }

    ErrorMgr.NoError();
    return lnk->GetParmB();
}

void SetParmLinkAParm( const std::string & link_id, const std::string & parm_id )
{
    Link* lnk = FindParmLink( link_id, "SetParmLinkAParm" );
    if ( !lnk )
    {
        return;
    }

    if ( !ParmMgr.FindParm( parm_id ) )
    {
        ErrorMgr.AddError( VSP_CANT_FIND_PARM, "SetParmLinkAParm::Can't Find Parm " + parm_id );
        return;
    }

    lnk->SetParmA( parm_id );

    ErrorMgr.NoError();
}

void SetParmLinkBParm( const std::string & link_id, const std::string & parm_id )
{
    Link* lnk = FindParmLink( link_id, "SetParmLinkBParm" );
    if ( !lnk )
    {
        return;
    }

    if ( !ParmMgr.FindParm( parm_id ) )
    {
        ErrorMgr.AddError( VSP_CANT_FIND_PARM, "SetParmLinkBParm::Can't Find Parm " + parm_id );
        return;
    }

    lnk->SetParmB( parm_id );

    ErrorMgr.NoError();
}

void DelParmLink( const std::string & link_id )
{
    for ( int i = 0; i < LinkMgr.GetNumLinks(); i++ )
    {
        Link* lnk = LinkMgr.GetLink( i );
        if ( lnk && lnk->GetID() == link_id )
        {
            set < int > todel;
            todel.insert( i );
            LinkMgr.DelLinks( todel );

            ErrorMgr.NoError();
            return;
        }
    }

    ErrorMgr.AddError( VSP_INVALID_ID, "DelParmLink::Can't Find Parm Link " + link_id );
}

void SortParmLinksByA()
{
    LinkMgr.SortLinksByA();

    ErrorMgr.NoError();
}

void SortParmLinksByB()
{
    LinkMgr.SortLinksByB();

    ErrorMgr.NoError();
}

void DelAllParmLinks()
{
    LinkMgr.DelAllLinks();

    ErrorMgr.NoError();
}

bool LinkAllComp( const std::string & parm_a_id, const std::string & parm_b_id )
{
    if ( !ParmMgr.FindParm( parm_a_id ) || !ParmMgr.FindParm( parm_b_id ) )
    {
        ErrorMgr.AddError( VSP_CANT_FIND_PARM, "LinkAllComp::Can't Find Parm " + parm_a_id + " or " + parm_b_id );
        return false;
    }

    LinkMgr.SetParm( true, parm_a_id );
    LinkMgr.SetParm( false, parm_b_id );

    bool ret = LinkMgr.LinkAllComp();

    ErrorMgr.NoError();
    return ret;
}

bool LinkAllGroup( const std::string & parm_a_id, const std::string & parm_b_id )
{
    if ( !ParmMgr.FindParm( parm_a_id ) || !ParmMgr.FindParm( parm_b_id ) )
    {
        ErrorMgr.AddError( VSP_CANT_FIND_PARM, "LinkAllGroup::Can't Find Parm " + parm_a_id + " or " + parm_b_id );
        return false;
    }

    LinkMgr.SetParm( true, parm_a_id );
    LinkMgr.SetParm( false, parm_b_id );

    bool ret = LinkMgr.LinkAllGroup();

    ErrorMgr.NoError();
    return ret;
}

bool GetParmLinkOffsetFlag( const std::string & link_id )
{
    Link* lnk = FindParmLink( link_id, "GetParmLinkOffsetFlag" );
    if ( !lnk )
    {
        return false;
    }

    ErrorMgr.NoError();
    return lnk->GetOffsetFlag();
}

void SetParmLinkOffsetFlag( const std::string & link_id, bool flag )
{
    Link* lnk = FindParmLink( link_id, "SetParmLinkOffsetFlag" );
    if ( !lnk )
    {
        return;
    }

    lnk->SetOffsetFlag( flag );

    ErrorMgr.NoError();
}

bool GetParmLinkScaleFlag( const std::string & link_id )
{
    Link* lnk = FindParmLink( link_id, "GetParmLinkScaleFlag" );
    if ( !lnk )
    {
        return false;
    }

    ErrorMgr.NoError();
    return lnk->GetScaleFlag();
}

void SetParmLinkScaleFlag( const std::string & link_id, bool flag )
{
    Link* lnk = FindParmLink( link_id, "SetParmLinkScaleFlag" );
    if ( !lnk )
    {
        return;
    }

    lnk->SetScaleFlag( flag );

    ErrorMgr.NoError();
}

bool GetParmLinkLowerLimitFlag( const std::string & link_id )
{
    Link* lnk = FindParmLink( link_id, "GetParmLinkLowerLimitFlag" );
    if ( !lnk )
    {
        return false;
    }

    ErrorMgr.NoError();
    return lnk->GetLowerLimitFlag();
}

void SetParmLinkLowerLimitFlag( const std::string & link_id, bool flag )
{
    Link* lnk = FindParmLink( link_id, "SetParmLinkLowerLimitFlag" );
    if ( !lnk )
    {
        return;
    }

    lnk->SetLowerLimitFlag( flag );

    ErrorMgr.NoError();
}

bool GetParmLinkUpperLimitFlag( const std::string & link_id )
{
    Link* lnk = FindParmLink( link_id, "GetParmLinkUpperLimitFlag" );
    if ( !lnk )
    {
        return false;
    }

    ErrorMgr.NoError();
    return lnk->GetUpperLimitFlag();
}

void SetParmLinkUpperLimitFlag( const std::string & link_id, bool flag )
{
    Link* lnk = FindParmLink( link_id, "SetParmLinkUpperLimitFlag" );
    if ( !lnk )
    {
        return;
    }

    lnk->SetUpperLimitFlag( flag );

    ErrorMgr.NoError();
}

//======================= Advanced Link Functions ============================//

std::vector< std::string > GetAdvLinkNames()
{
    ErrorMgr.NoError();
    return AdvLinkMgr.GetLinkNames();
}

int GetLinkIndex( const std::string & name )
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

std::string GetAdvLinkName( int index )
{
    AdvLink * adv_link = AdvLinkMgr.GetLink( index );

    if ( !adv_link )
    {
        ErrorMgr.AddError( VSP_INDEX_OUT_RANGE, "GetAdvLinkName::Invalid Advanced Link Index " + to_string( index ) );
        return std::string();
    }

    ErrorMgr.NoError();
    return adv_link->GetName();
}

void SetAdvLinkName( int index, const std::string & name )
{
    AdvLink * adv_link = AdvLinkMgr.GetLink( index );

    if ( !adv_link )
    {
        ErrorMgr.AddError( VSP_INDEX_OUT_RANGE, "SetAdvLinkName::Invalid Advanced Link Index " + to_string( index ) );
        return;
    }

    // Links are found by name, so a name another link has is refused
    if ( !AdvLinkMgr.SetLinkName( adv_link, name ) )
    {
        ErrorMgr.AddError( VSP_DUPLICATE_NAME, "SetAdvLinkName::Another Advanced Link is named " + name );
        return;
    }

    ErrorMgr.NoError();
}

void DelAdvLink( int index )
{
    if ( !AdvLinkMgr.GetLink( index ) )
    {
        ErrorMgr.AddError( VSP_INDEX_OUT_RANGE, "DelAdvLink::Invalid Advanced Link Index " + to_string( index ) );
        return;
    }

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

void AddAdvLink( const std::string & name )
{
    AdvLinkMgr.AddLink( name );

    ErrorMgr.NoError();
    return;
}

void AddAdvLinkInput( int index, const std::string & parm_id, const std::string & var_name )
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

    if ( !ParmMgr.FindParm( parm_id ) )
    {
        ErrorMgr.AddError( VSP_CANT_FIND_PARM, "AddAdvLinkInput::Can't Find Parm " + parm_id );
        return;
    }

    adv_link->AddVar( parm_id, var_name, true );

    ErrorMgr.NoError();
    return;
}

void AddAdvLinkOutput( int index, const std::string & parm_id, const std::string & var_name )
{
    AdvLink * adv_link = AdvLinkMgr.GetLink( index );

    if ( !adv_link )
    {
        ErrorMgr.AddError( VSP_INDEX_OUT_RANGE, "AddAdvLinkOutput::Invalid Advanced Link Index " + to_string( index ) );
        return;
    }

    if ( adv_link->DuplicateVarName( var_name ) )
    {
        ErrorMgr.AddError( VSP_DUPLICATE_NAME, "AddAdvLinkOutput::Duplicate advanced link output name " + var_name );
        return;
    }

    if ( !ParmMgr.FindParm( parm_id ) )
    {
        ErrorMgr.AddError( VSP_CANT_FIND_PARM, "AddAdvLinkOutput::Can't Find Parm " + parm_id );
        return;
    }

    adv_link->AddVar( parm_id, var_name, false );

    ErrorMgr.NoError();
    return;
}

void DelAllAdvLinkInputs( int index )
{
    AdvLink * adv_link = AdvLinkMgr.GetLink( index );

    if ( !adv_link )
    {
        ErrorMgr.AddError( VSP_INDEX_OUT_RANGE, "DelAllAdvLinkInputs::Invalid Advanced Link Index " + to_string( index ) );
        return;
    }

    adv_link->DeleteAllVars( true );

    ErrorMgr.NoError();
    return;
}

void DelAllAdvLinkOutputs( int index )
{
    AdvLink * adv_link = AdvLinkMgr.GetLink( index );

    if ( !adv_link )
    {
        ErrorMgr.AddError( VSP_INDEX_OUT_RANGE, "DelAllAdvLinkOutputs::Invalid Advanced Link Index " + to_string( index ) );
        return;
    }

    adv_link->DeleteAllVars( false );

    ErrorMgr.NoError();
    return;
}

void DelAdvLinkInput( int index, const std::string & var_name )
{
    AdvLink * adv_link = AdvLinkMgr.GetLink( index );

    if ( !adv_link )
    {
        ErrorMgr.AddError( VSP_INDEX_OUT_RANGE, "DelAdvLinkInput::Invalid Advanced Link Index " + to_string( index ) );
        return;
    }

    // This used to hand the advanced link's own index to the index based
    // DeleteVar and ignore var_name entirely, so it removed whichever variable
    // happened to sit at that index.
    if ( !adv_link->DeleteVar( var_name, true ) )
    {
        ErrorMgr.AddError( VSP_CANT_FIND_NAME, "DelAdvLinkInput::Can't Find Input Variable " + var_name );
        return;
    }

    ErrorMgr.NoError();
    return;
}

void DelAdvLinkOutput( int index, const std::string & var_name )
{
    AdvLink * adv_link = AdvLinkMgr.GetLink( index );

    if ( !adv_link )
    {
        ErrorMgr.AddError( VSP_INDEX_OUT_RANGE, "DelAdvLinkOutput::Invalid Advanced Link Index " + to_string( index ) );
        return;
    }

    // Same fix as DelAdvLinkInput above.
    if ( !adv_link->DeleteVar( var_name, false ) )
    {
        ErrorMgr.AddError( VSP_CANT_FIND_NAME, "DelAdvLinkOutput::Can't Find Output Variable " + var_name );
        return;
    }

    ErrorMgr.NoError();
    return;
}

std::vector< std::string > GetAdvLinkInputNames( int index )
{
    std::vector < std::string > ret;
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
    std::vector < std::string > ret;
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
    std::vector < std::string > ret;
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
    std::vector < std::string > ret;
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

// Look up an advanced link input or output by variable name, reporting through the
// named caller on failure.
static int FindAdvLinkVarIndex( AdvLink* adv_link, bool input_flag, const std::string & var_name, const std::string & caller )
{
    std::vector < std::string > names;
    if ( input_flag )
    {
        names = adv_link->GetInputNames();
    }
    else
    {
        names = adv_link->GetOutputNames();
    }

    for ( int i = 0; i < ( int )names.size(); i++ )
    {
        if ( names[i] == var_name )
        {
            return i;
        }
    }

    ErrorMgr.AddError( VSP_CANT_FIND_NAME, caller + "::Can't Find Variable " + var_name );
    return -1;
}

void ReorderAdvLinkInput( int index, const std::string & var_name, int reorder_type )
{
    AdvLink * adv_link = AdvLinkMgr.GetLink( index );

    if ( !adv_link )
    {
        ErrorMgr.AddError( VSP_INDEX_OUT_RANGE, "ReorderAdvLinkInput::Invalid Advanced Link Index " + to_string( index ) );
        return;
    }

    if ( reorder_type < REORDER_MOVE_UP || reorder_type > REORDER_MOVE_BOTTOM )
    {
        ErrorMgr.AddError( VSP_INVALID_TYPE, "ReorderAdvLinkInput::Invalid Reorder Type " + to_string( reorder_type ) );
        return;
    }

    int var_index = FindAdvLinkVarIndex( adv_link, true, var_name, "ReorderAdvLinkInput" );
    if ( var_index < 0 )
    {
        return;
    }

    adv_link->ReorderInputVar( var_index, reorder_type );

    ErrorMgr.NoError();
}

void ReorderAdvLinkOutput( int index, const std::string & var_name, int reorder_type )
{
    AdvLink * adv_link = AdvLinkMgr.GetLink( index );

    if ( !adv_link )
    {
        ErrorMgr.AddError( VSP_INDEX_OUT_RANGE, "ReorderAdvLinkOutput::Invalid Advanced Link Index " + to_string( index ) );
        return;
    }

    if ( reorder_type < REORDER_MOVE_UP || reorder_type > REORDER_MOVE_BOTTOM )
    {
        ErrorMgr.AddError( VSP_INVALID_TYPE, "ReorderAdvLinkOutput::Invalid Reorder Type " + to_string( reorder_type ) );
        return;
    }

    int var_index = FindAdvLinkVarIndex( adv_link, false, var_name, "ReorderAdvLinkOutput" );
    if ( var_index < 0 )
    {
        return;
    }

    adv_link->ReorderOutputVar( var_index, reorder_type );

    ErrorMgr.NoError();
}

void WriteAdvLinkCodeFile( int index, const std::string & file_name )
{
    AdvLink * adv_link = AdvLinkMgr.GetLink( index );

    if ( !adv_link )
    {
        ErrorMgr.AddError( VSP_INDEX_OUT_RANGE, "WriteAdvLinkCodeFile::Invalid Advanced Link Index " + to_string( index ) );
        return;
    }

    if ( !adv_link->SaveCode( file_name ) )
    {
        ErrorMgr.AddError( VSP_FILE_WRITE_FAILURE, "WriteAdvLinkCodeFile::Can't Write File " + file_name );
        return;
    }

    ErrorMgr.NoError();
}

void ReadAdvLinkCodeFile( int index, const std::string & file_name )
{
    AdvLink * adv_link = AdvLinkMgr.GetLink( index );

    if ( !adv_link )
    {
        ErrorMgr.AddError( VSP_INDEX_OUT_RANGE, "ReadAdvLinkCodeFile::Invalid Advanced Link Index " + to_string( index ) );
        return;
    }

    if ( !adv_link->ReadCode( file_name ) )
    {
        ErrorMgr.AddError( VSP_FILE_DOES_NOT_EXIST, "ReadAdvLinkCodeFile::Can't Read File " + file_name );
        return;
    }

    ErrorMgr.NoError();
}

void SortAdvLinkInputsVar( int index )
{
    AdvLink * adv_link = AdvLinkMgr.GetLink( index );

    if ( !adv_link )
    {
        ErrorMgr.AddError( VSP_INDEX_OUT_RANGE, "SortAdvLinkInputsVar::Invalid Advanced Link Index " + to_string( index ) );
        return;
    }

    adv_link->SortInputsVar( -1 );

    ErrorMgr.NoError();
}

void SortAdvLinkInputsCGP( int index )
{
    AdvLink * adv_link = AdvLinkMgr.GetLink( index );

    if ( !adv_link )
    {
        ErrorMgr.AddError( VSP_INDEX_OUT_RANGE, "SortAdvLinkInputsCGP::Invalid Advanced Link Index " + to_string( index ) );
        return;
    }

    adv_link->SortInputsCGP( -1 );

    ErrorMgr.NoError();
}

void SortAdvLinkOutputsVar( int index )
{
    AdvLink * adv_link = AdvLinkMgr.GetLink( index );

    if ( !adv_link )
    {
        ErrorMgr.AddError( VSP_INDEX_OUT_RANGE, "SortAdvLinkOutputsVar::Invalid Advanced Link Index " + to_string( index ) );
        return;
    }

    adv_link->SortOutputsVar( -1 );

    ErrorMgr.NoError();
}

void SortAdvLinkOutputsCGP( int index )
{
    AdvLink * adv_link = AdvLinkMgr.GetLink( index );

    if ( !adv_link )
    {
        ErrorMgr.AddError( VSP_INDEX_OUT_RANGE, "SortAdvLinkOutputsCGP::Invalid Advanced Link Index " + to_string( index ) );
        return;
    }

    adv_link->SortOutputsCGP( -1 );

    ErrorMgr.NoError();
}

void SetAdvLinkCode( int index, const std::string & code )
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
    std::string ret;
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

void SearchReplaceAdvLinkCode( int index, const std::string & from, const std::string & to )
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