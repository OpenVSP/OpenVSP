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
extern void ComputeDegenGeom( int set, int file_export_types );
extern void ComputeCFDMesh( int set, int file_export_types );
extern void SetCFDMeshVal( int type, double val );
extern void SetCFDWakeFlag( const string & geom_id, bool flag );
extern void DeleteAllCFDSources();
extern void AddDefaultSources();
extern void AddCFDSource( int type, const string & geom_id, int surf_index,
                          double l1, double r1, double u1, double w1,
                          double l2 = 0, double r2 = 0, double u2 = 0, double w2 = 0 );

//======================== Analysis ================================//

extern int GetNumAnalysis();
extern vector<string> ListAnalysis();
extern vector<string> GetAnalysisInputNames( const string & analysis );
extern string ExecAnalysis( const string & analysis );

extern int GetNumAnalysisInputData( const string & analysis, const string & name );
extern int GetAnalysisInputType( const string & analysis, const string & name );
extern const vector< int > & GetIntAnalysisInput( const string & analysis, const string & name, int index = 0 );
extern const vector< double > & GetDoubleAnalysisInput( const string & analysis, const string & name, int index = 0 );
extern const vector<string> & GetStringAnalysisInput( const string & analysis, const string & name, int index = 0 );
extern const vector< vec3d > & GetVec3dAnalysisInput( const string & analysis, const string & name, int index = 0 );

extern void SetAnalysisInputDefaults( const string & analysis );
extern void SetIntAnalysisInput( const string & analysis, const string & name, const vector< int > & indata, int index = 0 );
extern void SetDoubleAnalysisInput( const string & analysis, const string & name, const vector< double > & indata, int index = 0 );
extern void SetStringAnalysisInput( const string & analysis, const string & name, const vector<string> & indata, int index = 0 );
extern void SetVec3dAnalysisInput( const string & analysis, const string & name, const vector< vec3d > & indata, int index = 0 );

//======================== Results ================================//
extern vector<string> GetAllResultsNames();
extern vector<string> GetAllDataNames( const string & results_id );
extern int GetNumResults( const string & name );
extern string FindResultsID( const string & name, int index = 0 );
extern string FindLatestResultsID( const string & name );
extern int GetNumData( const string & results_id, const string & data_name );
extern int GetResultsType( const string & results_id, const string & data_name );
extern const vector< int > & GetIntResults( const string & id, const string & name, int index = 0 );
extern const vector< double > & GetDoubleResults( const string & id, const string & name, int index = 0 );
extern const vector<string> & GetStringResults( const string & id, const string & name, int index = 0 );
extern const vector< vec3d > & GetVec3dResults( const string & id, const string & name, int index = 0 );
extern string CreateGeomResults( const string & geom_id, const string & name );
extern void DeleteAllResults();
extern void DeleteResult( const string & id );
extern void WriteResultsCSVFile( const string & id, const string & file_name );

//======================== GUI Functions ================================//
extern void StartGui( );

//======================== Geom Functions ================================//
extern vector<string> GetGeomTypes();
extern string AddGeom( const string & type, const string & parent = string() );
extern void DeleteGeom( const string & geom_id );
extern void DeleteGeomVec( const vector< string > & del_vec );
extern void CutGeomToClipboard( const string & geom_id );
extern void CopyGeomToClipboard( const string & geom_id );
extern void PasteGeomClipboard( const string & parent = string() );
extern vector<string> FindGeoms();
extern vector<string> FindGeomsWithName( const string & name );
extern string FindGeom( const string & name, int index );
extern void SetGeomName( const string & geom_id, const string & name );
extern string GetGeomName( const string & geom_id );
extern vector<string> GetGeomParmIDs( const string & geom_id );
extern string GetParm( const string & geom_id, const string & name, const string & group );
extern int GetNumXSecSurfs( const string & geom_id );
extern int GetNumMainSurfs( const string & geom_id );
extern string AddSubSurf( const string & geom_id, int type, int surfindex = 0 );
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
extern vector<string> GetXSecParmIDs( const string& xsec_id );
extern string GetXSecParm( const string& xsec_id, const string& name );
extern vector<vec3d> ReadFileXSec( const string& xsec_id, const string& file_name );
extern void SetXSecPnts( const string& xsec_id, vector< vec3d > & pnt_vec );
extern vec3d ComputeXSecPnt( const string& xsec_id, double fract );
extern vec3d ComputeXSecTan( const string& xsec_id, double fract );
extern void ResetXSecSkinParms( const string& xsec_id );
extern void SetXSecContinuity( const string& xsec_id, int cx );
extern void SetXSecTanAngles( const string& xsec_id, int side, double top, double right, double bottom, double left );
extern void SetXSecTanSlews( const string& xsec_id, int side, double top, double right, double bottom, double left );
extern void SetXSecTanStrengths( const string& xsec_id, int side, double top, double right, double bottom, double left );
extern void SetXSecCurvatures( const string& xsec_id, int side, double top, double right, double bottom, double left );
extern void ReadFileAirfoil( const string& xsec_id, const string& file_name );
extern void SetAirfoilPnts( const string& xsec_id, vector< vec3d > & up_pnt_vec, vector< vec3d > & low_pnt_vec );
extern vector<vec3d> GetAirfoilUpperPnts( const string& xsec_id );
extern vector<vec3d> GetAirfoilLowerPnts( const string& xsec_id );
extern vector<double> GetUpperCSTCoefs( const string& xsec_id );
extern vector<double> GetLowerCSTCoefs( const string& xsec_id );
extern int GetUpperCSTDegree( const string& xsec_id );
extern int GetLowerCSTDegree( const string& xsec_id );
extern void SetUpperCST( const string& xsec_id, int deg, const vector<double> &coefs );
extern void SetLowerCST( const string& xsec_id, int deg, const vector<double> &coefs );
extern void PromoteCSTUpper( const string& xsec_id );
extern void PromoteCSTLower( const string& xsec_id );
extern void DemoteCSTUpper( const string& xsec_id );
extern void DemoteCSTLower( const string& xsec_id );
extern void FitAfCST( const string & xsec_surf_id, int xsec_index, int deg );

//======================== Sets ================================//
extern int GetNumSets();
extern void SetSetName( int index, const string& name );
extern string GetSetName( int index );
extern vector<string> GetGeomSetAtIndex( int index );
extern vector<string> GetGeomSet( const string & name );
extern int GetSetIndex( const string & name );
extern bool GetSetFlag( const string & geom_id, int set_index );
extern void SetSetFlag( const string & geom_id, int set_index, bool flag );

//======================== Parm Functions ================================//
extern bool ValidParm( const string & id );
extern double SetParmVal( const string & parm_id, double val );
extern double SetParmVal( const string & geom_id, const string & name, const string & group, double val );
extern double SetParmValLimits( const string & parm_id, double val, double lower_limit, double upper_limit );
extern double SetParmValUpdate( const string & parm_id, double val );
extern double SetParmValUpdate( const string & geom_id, const string & parm_name, const string & parm_group_name, double val );
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
extern string FindParm( const string & parm_container_id, const string& parm_name, const string& group_name );

//======================== Parm Container Functions ======================//

extern vector<string> FindContainers();
extern vector<string> FindContainersWithName( const string & name );
extern string FindContainer( const string & name, int index );
extern string GetContainerName( const string & parm_container_id );
extern vector<string> FindContainerGroupNames( const string & parm_container_id );
extern vector<string> FindContainerParmIDs( const string & parm_container_id );

//======================== Snap To Functions ======================//
extern double ComputeMinClearanceDistance( const string & geom_id, int set  = SET_ALL );
extern double SnapParm( const string & parm_id, double target_min_dist, bool inc_flag, int set = SET_ALL );

//======================== Variable Preset Functions ======================//
extern void AddVarPresetGroup( const string &group_name );
extern void AddVarPresetSetting( const string &setting_name );
extern void AddVarPresetParm( const string &parm_ID );
extern void AddVarPresetParm( const string &parm_ID, const string &group_name );
extern void EditVarPresetParm( const string &parm_ID, double parm_val );
extern void EditVarPresetParm( const string &parm_ID, double parm_val, const string &group_name,
    const string &setting_name );
extern void DeleteVarPresetParm( const string &parm_ID );
extern void DeleteVarPresetParm( const string &parm_ID, const string &group_name );
extern void SwitchVarPreset( const string &group_name, const string &setting_name );
extern bool DeleteVarPresetSet( const string &group_name, const string &setting_name );
extern std::string GetCurrentGroupName();
extern std::string GetCurrentSettingName();
extern std::vector< std::string > GetVarPresetGroupNames();
extern std::vector< std::string > GetVarPresetSettingNames( const string &group_name );
extern std::vector< std::string > GetVarPresetSettingNames( int group_index );
extern std::vector< double > GetVarPresetParmVals();
extern std::vector< double > GetVarPresetParmVals( const string &group_name, const string &setting_name );
extern std::vector< std::string > GetVarPresetParmIDs();
extern std::vector< std::string > GetVarPresetParmIDs( const string &group_name );

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
