//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//

// API.h: interface for the Vehicle Class and Vehicle Mgr Singleton.
// J.R Gloudemans
//
//////////////////////////////////////////////////////////////////////


#if !defined(VSPAPI__INCLUDED_)
#define VSPAPI__INCLUDED_


#include "APIErrorMgr.h"
#include "Vec3d.h"
#include "Matrix.h"

#include <string>
#include <stack>
#include <vector>


using std::string;
using std::stack;
using std::vector;

namespace vsp
{

//======================== API Functions ================================//
extern void VSPCheckSetup();
extern void VSPRenew();

extern void Update();
extern void VSPExit( int error_code );

//======================== File I/O ================================//
extern void ReadVSPFile( const string & file_name );
extern void WriteVSPFile( const string & file_name, int set = SET_ALL );
extern string GetVSPFileName();
extern void ClearVSPModel();
extern void InsertVSPFile( const string & file_name, const string & parent_geom_id );

extern void ExportFile( const string & file_name, int write_set_index, int file_type );
extern string ImportFile( const string & file_name, int file_type, const string & parent );

//======================== Design Files ================================//

extern void ReadApplyDESFile( const string & file_name );
extern void WriteDESFile( const string & file_name );
extern void ReadApplyXDDMFile( const string & file_name );
extern void WriteXDDMFile( const string & file_name );
extern int GetNumDesignVars();
extern void AddDesignVar( const string & parm_id, int type );
extern void DeleteAllDesignVars();
extern string GetDesignVar( int index );
extern int GetDesignVarType( int index );

//======================== Computations ================================//
extern void SetComputationFileName( int file_type, const string & file_name );
extern string ComputeMassProps( int set, int num_slices );
extern string ComputeCompGeom( int set, bool half_mesh, int file_export_types );
extern string ComputePlaneSlice( int set, int num_slices, const vec3d & norm, bool auto_bnd,
                                 double start_bnd = 0, double end_bnd = 0 );
extern string ComputeAwaveSlice( int set, int num_slices, int num_rots, double ang_control, bool comp_ang,
                                 const vec3d & norm, bool auto_bnd, double start_bnd = 0, double end_bnd = 0 );

extern void ComputeDegenGeom( int set, int file_export_types );
extern void ComputeCFDMesh( int set, int file_export_types );
extern void SetCFDMeshVal( int type, double val );
extern void SetCFDWakeFlag( const string & geom_id, bool flag );
extern void DeleteAllCFDSources();
extern void AddDefaultSources();
extern void AddCFDSource( int type, const string & geom_id, int surf_index,
                          double l1, double r1, double u1, double w1,
						  double l2 = 0, double r2 = 0, double u2 = 0, double w2 = 0 );

//======================== Results ================================//
extern std::vector< std::string > GetAllResultsNames();
extern std::vector< std::string > GetAllDataNames( const string & results_id );
extern int GetNumResults( const string & name );
extern string FindResultsID( const string & name, int index = 0 );
extern string FindLatestResultsID( const string & name );
extern int GetNumData( const string & results_id, const string & data_name );
extern const std::vector< int > & GetIntResults( const string & id, const string & name, int index = 0 );
extern const std::vector< double > & GetDoubleResults( const string & id, const string & name, int index = 0 );
extern const std::vector< std::string > & GetStringResults( const string & id, const string & name, int index = 0 );
extern const std::vector< vec3d > & GetVec3dResults( const string & id, const string & name, int index = 0 );
extern string CreateGeomResults( const string & geom_id, const string & name );
extern void DeleteAllResults();
extern void DeleteResult( const string & id );
extern void WriteResultsCSVFile( const string & id, const string & file_name );

//======================== GUI Functions ================================//
extern void StartGui( );

//======================== Geom Functions ================================//
extern std::vector< std::string > GetGeomTypes();
extern string AddGeom( const string & type, const string & parent = string() );
extern void CutGeomToClipboard( const string & geom_id );
extern void CopyGeomToClipboard( const string & geom_id );
extern void PasteGeomClipboard( const string & parent = string() );
extern std::vector< std::string > FindGeoms();
extern std::vector< std::string > FindGeomsWithName( const string & name );
extern string FindGeom( const string & name, int index );
extern void SetGeomName( const string & geom_id, const string & name );
extern string GetGeomName( const string & geom_id );
extern std::vector< std::string > GetGeomParmIDs( const string & geom_id );
extern string GetParm( const string & geom_id, const string & name, const string & group );
extern int GetNumXSecSurfs( const string & geom_id );
extern int GetNumMainSurfs( const string & geom_id );
extern string AddSubSurf( const string & geom_id, int type );
extern string GetSubSurf( const string & geom_id, int index );
extern void DeleteSubSurf( const string & geom_id, const string & sub_id );

extern void CutXSec( const string & geom_id, int index );
extern void CopyXSec( const string & geom_id, int index );
extern void PasteXSec( const string & geom_id, int index );
extern void InsertXSec( const string & geom_id, int index, int type );

//======================== Wing Section Functions ===================//
extern void SetDriverGroup( const string & geom_id, int section_index, int driver_0, int driver_1, int driver_2 );


//======================== XSecSurf ================================//
extern string GetXSecSurf( const string & geom_id, int index );
extern int GetNumXSec( const string & xsec_surf_id );
extern string GetXSec( const string & xsec_surf_id, int xsec_index );
extern void ChangeXSecShape( const string & xsec_surf_id, int xsec_index, int type );
extern void SetXSecSurfGlobalXForm( const string & xsec_surf_id, const Matrix4d & mat );
extern Matrix4d GetXSecSurfGlobalXForm( const string & xsec_surf_id );

//======================== XSec ================================//
extern int GetXSecShape( const string& xsec_id );
extern double GetXSecWidth( const string& xsec_id );
extern double GetXSecHeight( const string& xsec_id );
extern void SetXSecWidthHeight( const string& xsec_id, double w, double h );
extern std::vector< std::string > GetXSecParmIDs( const string& xsec_id );
extern string GetXSecParm( const string& xsec_id, const string& name );
extern std::vector<vec3d> ReadFileXSec( const string& xsec_id, const string& file_name );
extern void SetXSecPnts( const string& xsec_id, std::vector< vec3d > & pnt_vec );
extern vec3d ComputeXSecPnt( const string& xsec_id, double fract );
extern vec3d ComputeXSecTan( const string& xsec_id, double fract );
extern void ResetXSecSkinParms( const string& xsec_id );
extern void SetXSecContinuity( const string& xsec_id, int cx );
extern void SetXSecTanAngles( const string& xsec_id, int side, double top, double right, double bottom, double left );
extern void SetXSecTanSlews( const string& xsec_id, int side, double top, double right, double bottom, double left );
extern void SetXSecTanStrengths( const string& xsec_id, int side, double top, double right, double bottom, double left );
extern void SetXSecCurvatures( const string& xsec_id, int side, double top, double right, double bottom, double left );
extern void ReadFileAirfoil( const string& xsec_id, const string& file_name );
extern void SetAirfoilPnts( const string& xsec_id, std::vector< vec3d > & up_pnt_vec, std::vector< vec3d > & low_pnt_vec );
extern std::vector<vec3d> GetAirfoilUpperPnts( const string& xsec_id );
extern std::vector<vec3d> GetAirfoilLowerPnts( const string& xsec_id );
extern std::vector<double> GetUpperCSTCoefs( const string& xsec_id );
extern std::vector<double> GetLowerCSTCoefs( const string& xsec_id );
extern int GetUpperCSTDegree( const string& xsec_id );
extern int GetLowerCSTDegree( const string& xsec_id );
extern void SetUpperCST( const string& xsec_id, int deg, const std::vector<double> &coefs );
extern void SetLowerCST( const string& xsec_id, int deg, const std::vector<double> &coefs );
extern void PromoteCSTUpper( const string& xsec_id );
extern void PromoteCSTLower( const string& xsec_id );
extern void DemoteCSTUpper( const string& xsec_id );
extern void DemoteCSTLower( const string& xsec_id );
extern void FitAfCST( const string & xsec_surf_id, int xsec_index, int deg );

//======================== Sets ================================//
extern int GetNumSets();
extern void SetSetName( int index, const string& name );
extern string GetSetName( int index );
extern std::vector< std::string > GetGeomSetAtIndex( int index );
extern std::vector< std::string > GetGeomSet( const string & name );
extern int GetSetIndex( const string & name );
extern bool GetSetFlag( const string & geom_id, int set_index );
extern void SetSetFlag( const string & geom_id, int set_index, bool flag );

//======================== Parm Functions ================================//
extern bool ValidParm( const string & id );
extern double SetParmVal( const string & parm_id, double val );
extern double SetParmVal( const string & geom_id, const string & name, const string & group, double val );
extern double SetParmValLimits( const string & parm_id, double val, double lower_limit, double upper_limit );
extern double SetParmValUpdate( const string & parm_id, double val );
extern double SetParmValUpdate( const string & geom_id, const string & name, const string & group, double val );
extern double GetParmVal( const string & parm_id );
extern double GetParmVal( const string & geom_id, const string & name, const string & group );
extern int GetIntParmVal( const string & parm_id );
extern bool GetBoolParmVal( const string & parm_id );
extern void SetParmUpperLimit( const string & parm_id, double val );
extern double GetParmUpperLimit( const string & parm_id );
extern void SetParmLowerLimit( const string & parm_id, double val );
extern double GetParmLowerLimit( const string & parm_id );
extern int GetParmType( const string & parm_id );
extern string GetParmName( const string & parm_id );
extern string GetParmGroupName( const string & parm_id );
extern string GetParmDisplayGroupName( const string & parm_id );
extern string GetParmContainer( const string & parm_id );
extern void SetParmDescript( const string & parm_id, const string & desc );
extern string FindParm( const string & parm_container_id, const string & name, const string & group );

//======================== Parm Container Functions ======================//

extern std::vector< std::string > FindContainers();
extern std::vector< std::string > FindContainersWithName( const string & name );
extern string FindContainer( const string & name, int index );
extern string GetContainerName( const string & parm_container_id );
extern std::vector< std::string > FindContainerGroupNames( const string & parm_container_id );
extern std::vector< std::string > FindContainerParmIDs( const string & parm_container_id );


}           // End vsp namespace


#endif // !defined(VSPAPI__INCLUDED_)




/** \mainpage VSP API and Application
 *
 * VSP is a parametric aircraft geometry tool originally developed by NASA and
 * released as open-source software in January 2012.  VSP is currently being
 * refactored, redesigned, and rewritten to make the code more modular and
 * maintainable, and to provide an API such that other applications may access
 * the core geometry capability VSP provides.
 *
 * One of the primary goals of this refactoring is to separate the geometry
 * functions of VSP from the GUI and OpenGL 3D graphics.  Once complete, this
 * separation will enable batch-mode applications with the capabilities of VSP
 * to be compiled and run on computers where graphics libraries are not
 * welcome.  In addition, this will allow other dedicated applications to access
 * VSP's unique geometric capability without inheriting all of the GUI and
 * graphics.
 *
 * VSP's code is organized into three main groups.
 *  - util provides a set of core data structures which find common use.
 *  - geom_core provides the core geometry data structures and algorithms.
 *  - gui_and_draw provides all of the FLTK GUI code and OpenGL 3D drawing code.
 *
 * There must be no dependencies of geom_core and util on gui_and_draw.  However,
 * gui_and_draw can depend on util and geom_core.  In an interactive application,
 * there are times where geom_core needs to communicate with gui_and_draw -- for
 * example, to trigger an update.  VSP uses a subscription based message passing
 * capability in MessageMgr to support this.
 *
 * Some good places to start browsing the code:
 *  - MessageMgr is the message passing capability.
 *  - Vehicle is the main class for the geom_core.
 *  - Geom is the main base class for geometry entities.
 *  - Parm is the class for all parameters.
 *  - Parms are managed through ParmMgr, an alias for ParmMgrSingleton.
 *  - DrawObj s are created by Vehicle and drawn by VirtGlWindow.
 *
 */
