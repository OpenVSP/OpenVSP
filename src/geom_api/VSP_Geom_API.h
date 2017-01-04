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
extern void ReadVSPFile( const std::string & file_name );
extern void WriteVSPFile( const std::string & file_name, int set = SET_ALL );
extern void SetVSP3FileName( const std::string & file_name );
extern std::string GetVSPFileName();
extern void ClearVSPModel();
extern void InsertVSPFile( const std::string & file_name, const std::string & parent_geom_id );

extern void ExportFile( const std::string & file_name, int write_set_index, int file_type );
extern std::string ImportFile( const std::string & file_name, int file_type, const std::string & parent );

//======================== Design Files ================================//

extern void ReadApplyDESFile( const std::string & file_name );
extern void WriteDESFile( const std::string & file_name );
extern void ReadApplyXDDMFile( const std::string & file_name );
extern void WriteXDDMFile( const std::string & file_name );
extern int GetNumDesignVars();
extern void AddDesignVar( const std::string & parm_id, int type );
extern void DeleteAllDesignVars();
extern std::string GetDesignVar( int index );
extern int GetDesignVarType( int index );

//======================== Computations ================================//
extern void SetComputationFileName( int file_type, const std::string & file_name );
extern std::string ComputeMassProps( int set, int num_slices );
extern std::string ComputeCompGeom( int set, bool half_mesh, int file_export_types );
extern std::string ComputePlaneSlice( int set, int num_slices, const vec3d & norm, bool auto_bnd,
                                 double start_bnd = 0, double end_bnd = 0 );
extern void ComputeDegenGeom( int set, int file_export_types );
extern void ComputeCFDMesh( int set, int file_export_types );
extern void SetCFDMeshVal( int type, double val );
extern void SetCFDWakeFlag( const std::string & geom_id, bool flag );
extern void DeleteAllCFDSources();
extern void AddDefaultSources();
extern void AddCFDSource( int type, const std::string & geom_id, int surf_index,
                          double l1, double r1, double u1, double w1,
                          double l2 = 0, double r2 = 0, double u2 = 0, double w2 = 0 );

//======================== Analysis ================================//

extern int GetNumAnalysis();
extern std::vector<std::string> ListAnalysis();
extern std::vector<std::string> GetAnalysisInputNames( const std::string & analysis );
extern std::string ExecAnalysis( const std::string & analysis );

extern int GetNumAnalysisInputData( const std::string & analysis, const std::string & name );
extern int GetAnalysisInputType( const std::string & analysis, const std::string & name );
extern const std::vector< int > & GetIntAnalysisInput( const std::string & analysis, const std::string & name, int index = 0 );
extern const std::vector< double > & GetDoubleAnalysisInput( const std::string & analysis, const std::string & name, int index = 0 );
extern const std::vector<std::string> & GetStringAnalysisInput( const std::string & analysis, const std::string & name, int index = 0 );
extern const std::vector< vec3d > & GetVec3dAnalysisInput( const std::string & analysis, const std::string & name, int index = 0 );

extern void SetAnalysisInputDefaults( const std::string & analysis );
extern void SetIntAnalysisInput( const std::string & analysis, const std::string & name, const std::vector< int > & indata, int index = 0 );
extern void SetDoubleAnalysisInput( const std::string & analysis, const std::string & name, const std::vector< double > & indata, int index = 0 );
extern void SetStringAnalysisInput( const std::string & analysis, const std::string & name, const std::vector<std::string> & indata, int index = 0 );
extern void SetVec3dAnalysisInput( const std::string & analysis, const std::string & name, const std::vector< vec3d > & indata, int index = 0 );

extern void PrintAnalysisInputs( const std::string analysis_name );

//======================== Results ================================//
extern std::vector<std::string> GetAllResultsNames();
extern std::vector<std::string> GetAllDataNames( const std::string & results_id );
extern int GetNumResults( const std::string & name );
extern std::string FindResultsID( const std::string & name, int index = 0 );
extern std::string FindLatestResultsID( const std::string & name );
extern int GetNumData( const std::string & results_id, const std::string & data_name );
extern int GetResultsType( const std::string & results_id, const std::string & data_name );
extern const std::vector< int > & GetIntResults( const std::string & id, const std::string & name, int index = 0 );
extern const std::vector< double > & GetDoubleResults( const std::string & id, const std::string & name, int index = 0 );
extern const std::vector<std::string> & GetStringResults( const std::string & id, const std::string & name, int index = 0 );
extern const std::vector< vec3d > & GetVec3dResults( const std::string & id, const std::string & name, int index = 0 );
extern std::string CreateGeomResults( const std::string & geom_id, const std::string & name );
extern void DeleteAllResults();
extern void DeleteResult( const std::string & id );
extern void WriteResultsCSVFile( const std::string & id, const std::string & file_name );
extern void PrintResults( FILE * outputStream, const std::vector < std::string > &results_id_vec );
extern void PrintResults( FILE * outputStream, const std::string &results_id );

//======================== GUI Functions ================================//
extern void StartGui( );

//======================== Geom Functions ================================//
extern std::vector<std::string> GetGeomTypes();
extern std::string AddGeom( const std::string & type, const std::string & parent = std::string() );
extern void DeleteGeom( const std::string & geom_id );
extern void DeleteGeomVec( const std::vector< std::string > & del_vec );
extern void CutGeomToClipboard( const std::string & geom_id );
extern void CopyGeomToClipboard( const std::string & geom_id );
extern void PasteGeomClipboard( const std::string & parent = std::string() );
extern std::vector<std::string> FindGeoms();
extern std::vector<std::string> FindGeomsWithName( const std::string & name );
extern std::string FindGeom( const std::string & name, int index );
extern void SetGeomName( const std::string & geom_id, const std::string & name );
extern std::string GetGeomName( const std::string & geom_id );
extern std::vector<std::string> GetGeomParmIDs( const std::string & geom_id );
extern std::string GetParm( const std::string & geom_id, const std::string & name, const std::string & group );
extern int GetNumXSecSurfs( const std::string & geom_id );
extern int GetNumMainSurfs( const std::string & geom_id );
extern std::string AddSubSurf( const std::string & geom_id, int type, int surfindex = 0 );
extern std::string GetSubSurf( const std::string & geom_id, int index );
extern void DeleteSubSurf( const std::string & geom_id, const std::string & sub_id );

extern void CutXSec( const std::string & geom_id, int index );
extern void CopyXSec( const std::string & geom_id, int index );
extern void PasteXSec( const std::string & geom_id, int index );
extern void InsertXSec( const std::string & geom_id, int index, int type );

//======================== Wing Section Functions ===================//
extern void SetDriverGroup( const std::string & geom_id, int section_index, int driver_0, int driver_1, int driver_2 );


//======================== XSecSurf ================================//
extern std::string GetXSecSurf( const std::string & geom_id, int index );
extern int GetNumXSec( const std::string & xsec_surf_id );
extern std::string GetXSec( const std::string & xsec_surf_id, int xsec_index );
extern void ChangeXSecShape( const std::string & xsec_surf_id, int xsec_index, int type );
extern void SetXSecSurfGlobalXForm( const std::string & xsec_surf_id, const Matrix4d & mat );
extern Matrix4d GetXSecSurfGlobalXForm( const std::string & xsec_surf_id );

//======================== XSec ================================//
extern int GetXSecShape( const std::string& xsec_id );
extern double GetXSecWidth( const std::string& xsec_id );
extern double GetXSecHeight( const std::string& xsec_id );
extern void SetXSecWidthHeight( const std::string& xsec_id, double w, double h );
extern std::vector<std::string> GetXSecParmIDs( const std::string& xsec_id );
extern std::string GetXSecParm( const std::string& xsec_id, const std::string& name );
extern std::vector<vec3d> ReadFileXSec( const std::string& xsec_id, const std::string& file_name );
extern void SetXSecPnts( const std::string& xsec_id, std::vector< vec3d > & pnt_vec );
extern vec3d ComputeXSecPnt( const std::string& xsec_id, double fract );
extern vec3d ComputeXSecTan( const std::string& xsec_id, double fract );
extern void ResetXSecSkinParms( const std::string& xsec_id );
extern void SetXSecContinuity( const std::string& xsec_id, int cx );
extern void SetXSecTanAngles( const std::string& xsec_id, int side, double top, double right, double bottom, double left );
extern void SetXSecTanSlews( const std::string& xsec_id, int side, double top, double right, double bottom, double left );
extern void SetXSecTanStrengths( const std::string& xsec_id, int side, double top, double right, double bottom, double left );
extern void SetXSecCurvatures( const std::string& xsec_id, int side, double top, double right, double bottom, double left );
extern void ReadFileAirfoil( const std::string& xsec_id, const std::string& file_name );
extern void SetAirfoilPnts( const std::string& xsec_id, std::vector< vec3d > & up_pnt_vec, std::vector< vec3d > & low_pnt_vec );
extern std::vector<vec3d> GetAirfoilUpperPnts( const std::string& xsec_id );
extern std::vector<vec3d> GetAirfoilLowerPnts( const std::string& xsec_id );
extern std::vector<double> GetUpperCSTCoefs( const std::string& xsec_id );
extern std::vector<double> GetLowerCSTCoefs( const std::string& xsec_id );
extern int GetUpperCSTDegree( const std::string& xsec_id );
extern int GetLowerCSTDegree( const std::string& xsec_id );
extern void SetUpperCST( const std::string& xsec_id, int deg, const std::vector<double> &coefs );
extern void SetLowerCST( const std::string& xsec_id, int deg, const std::vector<double> &coefs );
extern void PromoteCSTUpper( const std::string& xsec_id );
extern void PromoteCSTLower( const std::string& xsec_id );
extern void DemoteCSTUpper( const std::string& xsec_id );
extern void DemoteCSTLower( const std::string& xsec_id );
extern void FitAfCST( const std::string & xsec_surf_id, int xsec_index, int deg );

//======================== Sets ================================//
extern int GetNumSets();
extern void SetSetName( int index, const std::string& name );
extern std::string GetSetName( int index );
extern std::vector<std::string> GetGeomSetAtIndex( int index );
extern std::vector<std::string> GetGeomSet( const std::string & name );
extern int GetSetIndex( const std::string & name );
extern bool GetSetFlag( const std::string & geom_id, int set_index );
extern void SetSetFlag( const std::string & geom_id, int set_index, bool flag );

//======================== Parm Functions ================================//
extern bool ValidParm( const std::string & id );
extern double SetParmVal( const std::string & parm_id, double val );
extern double SetParmVal( const std::string & geom_id, const std::string & name, const std::string & group, double val );
extern double SetParmValLimits( const std::string & parm_id, double val, double lower_limit, double upper_limit );
extern double SetParmValUpdate( const std::string & parm_id, double val );
extern double SetParmValUpdate( const std::string & geom_id, const std::string & parm_name, const std::string & parm_group_name, double val );
extern double GetParmVal( const std::string & parm_id );
extern double GetParmVal( const std::string & geom_id, const std::string & name, const std::string & group );
extern int GetIntParmVal( const std::string & parm_id );
extern bool GetBoolParmVal( const std::string & parm_id );
extern void SetParmUpperLimit( const std::string & parm_id, double val );
extern double GetParmUpperLimit( const std::string & parm_id );
extern void SetParmLowerLimit( const std::string & parm_id, double val );
extern double GetParmLowerLimit( const std::string & parm_id );
extern int GetParmType( const std::string & parm_id );
extern std::string GetParmName( const std::string & parm_id );
extern std::string GetParmGroupName( const std::string & parm_id );
extern std::string GetParmDisplayGroupName( const std::string & parm_id );
extern std::string GetParmContainer( const std::string & parm_id );
extern void SetParmDescript( const std::string & parm_id, const std::string & desc );
extern std::string FindParm( const std::string & parm_container_id, const std::string& parm_name, const std::string& group_name );

//======================== Parm Container Functions ======================//

extern std::vector<std::string> FindContainers();
extern std::vector<std::string> FindContainersWithName( const std::string & name );
extern std::string FindContainer( const std::string & name, int index );
extern std::string GetContainerName( const std::string & parm_container_id );
extern std::vector<std::string> FindContainerGroupNames( const std::string & parm_container_id );
extern std::vector<std::string> FindContainerParmIDs( const std::string & parm_container_id );

//======================== Snap To Functions ======================//
extern double ComputeMinClearanceDistance( const std::string & geom_id, int set  = SET_ALL );
extern double SnapParm( const std::string & parm_id, double target_min_dist, bool inc_flag, int set = SET_ALL );

//======================== Variable Preset Functions ======================//
extern void AddVarPresetGroup( const std::string &group_name );
extern void AddVarPresetSetting( const std::string &setting_name );
extern void AddVarPresetParm( const std::string &parm_ID );
extern void AddVarPresetParm( const std::string &parm_ID, const std::string &group_name );
extern void EditVarPresetParm( const std::string &parm_ID, double parm_val );
extern void EditVarPresetParm( const std::string &parm_ID, double parm_val, const std::string &group_name,
    const std::string &setting_name );
extern void DeleteVarPresetParm( const std::string &parm_ID );
extern void DeleteVarPresetParm( const std::string &parm_ID, const std::string &group_name );
extern void SwitchVarPreset( const std::string &group_name, const std::string &setting_name );
extern bool DeleteVarPresetSet( const std::string &group_name, const std::string &setting_name );
extern std::string GetCurrentGroupName();
extern std::string GetCurrentSettingName();
extern std::vector< std::string > GetVarPresetGroupNames();
extern std::vector< std::string > GetVarPresetSettingNamesWName( const std::string &group_name );
extern std::vector< std::string > GetVarPresetSettingNamesWIndex( int group_index );
extern std::vector< double > GetVarPresetParmVals();
extern std::vector< double > GetVarPresetParmValsWNames( const std::string &group_name, const std::string &setting_name );
extern std::vector< std::string > GetVarPresetParmIDs();
extern std::vector< std::string > GetVarPresetParmIDsWName( const std::string &group_name );

//======================== Parametric Curve Functions ======================//
extern void SetPCurve( const std::string & geom_id, const int & pcurveid, const std::vector < double > & tvec,
    const std::vector < double > & valvec, const int & newtype );
extern void PCurveConvertTo( const std::string & geom_id, const int & pcurveid, const int & newtype );
extern int PCurveGetType( const std::string & geom_id, const int & pcurveid );
extern std::vector < double > PCurveGetTVec( const std::string & geom_id, const int & pcurveid );
extern std::vector < double > PCurveGetValVec( const std::string & geom_id, const int & pcurveid );
extern void PCurveDeletePt( const std::string & geom_id, const int & pcurveid, const int & indx );
extern void PCurveSplit( const std::string & geom_id, const int & pcurveid, const double & tsplit );

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
