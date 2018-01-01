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

extern string GetVSPAERORefWingID();
extern string SetVSPAERORefWingID( const std::string & geom_id );

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

extern void PrintAnalysisInputs( const std::string & analysis_name );

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
extern void PrintResults( const std::string &results_id );

//======================== GUI Functions ================================//
extern void StartGui( );

//======================== Geom Functions ================================//
extern std::vector<std::string> GetGeomTypes();
extern std::string AddGeom( const std::string & type, const std::string & parent = std::string() );
extern void UpdateGeom( const std::string & geom_id );
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

//======================== SubSurface Functions ================================//
extern std::string AddSubSurf( const std::string & geom_id, int type, int surfindex = 0 );
extern std::string GetSubSurf( const std::string & geom_id, int index );
extern std::vector<std::string> GetSubSurf( const std::string & geom_id, const std::string & name );
extern void DeleteSubSurf( const std::string & geom_id, const std::string & sub_id );
extern void DeleteSubSurf( const std::string & sub_id );
extern void SetSubSurfName(const std::string & geom_id, const std::string & sub_id, const std::string & name);
extern void SetSubSurfName( const std::string & sub_id, const std::string & name );
extern std::string GetSubSurfName( const std::string & geom_id, const std::string & sub_id );
extern std::string GetSubSurfName( const std::string & sub_id );
extern int GetSubSurfIndex( const std::string & sub_id );
extern std::vector<std::string> GetSubSurfIDVec( const std::string & geom_id );
extern std::vector<std::string> GetAllSubSurfIDs();
extern int GetNumSubSurf( const std::string & geom_id );
extern int GetSubSurfType( const std::string & sub_id );
extern std::vector<std::string> GetSubSurfParmIDs( const std::string & sub_id );

extern int AddFeaStruct( const std::string & geom_id, bool init_skin = true, int surfindex = 0 );
extern void DeleteFeaStruct( const std::string & geom_id, int fea_struct_id );
extern std::string GetFeaStructName( const std::string & geom_id, int fea_struct_id );
extern void SetFeaStructName( const std::string & geom_id, int fea_struct_id, const std::string & name );
extern std::string AddFeaPart( const std::string & geom_id, int fea_struct_id, int type );
extern void DeleteFeaPart( const std::string & geom_id, int fea_struct_id, const std::string & part_id );
extern std::string AddFeaSubSurf( const std::string & geom_id, int fea_struct_id, int type );
extern void DeleteFeaSubSurf( const std::string & geom_id, int fea_struct_id, const std::string & ss_id );
extern std::string AddFeaMaterial();
extern std::string AddFeaProperty( int property_type = 0 );
extern void SetFeaMeshVal( const std::string & geom_id, int fea_struct_id, int type, double val );
extern void SetFeaMeshFileName( const std::string & geom_id, int fea_struct_id, int file_type, const string & file_name );
extern void ComputeFeaMesh( const std::string & geom_id, int fea_struct_id, int file_type );

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
extern void WriteSeligAirfoilFile( const std::string & airfoil_name, std::vector<vec3d> & ordered_airfoil_pnts );
extern std::vector<vec3d> GetHersheyBarLiftDist( const int npts, const double alpha, const double Vinf, const double span, bool full_span_flag = false );
extern std::vector<vec3d> GetVKTAirfoilPnts( const int npts, const double alpha, const double epsilon, const double kappa, const double tau );
extern std::vector<double> GetVKTAirfoilCpDist( const double alpha, const double epsilon, const double kappa, const double tau, std::vector<vec3d> xyz_data );
extern std::vector<vec3d> GetEllipsoidSurfPnts( const vec3d center, const vec3d abc_rad, int u_npts = 20, int w_npts = 20 );
extern std::vector<vec3d> GetEllipsoidCpDist( const std::vector<vec3d> surf_pnt_vec, const vec3d abc_rad, const vec3d V_inf );
extern double IntegrateEllipsoidFlow( const vec3d abc_rad, const int abc_index );
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

//======================== VSPAERO Functions ======================//
extern void AutoGroupVSPAEROControlSurfaces();
extern int GetNumControlSurfaceGroups();

//======================== Parasite Drag Tool Functions ======================//
extern void AddExcrescence(const std::string & excresName, const int & excresType, const double & excresVal);
extern void DeleteExcrescence(const int & index);
extern void WriteAtmosphereCSVFile( const std::string & file_name, const int atmos_type );
extern void CalcAtmosphere( const double & alt, const double & delta_temp, const int & atmos_type,
    double & temp, double & pres, double & pres_ratio, double & rho_ratio );
extern void WriteBodyFFCSVFile( const std::string & file_name );
extern void WriteWingFFCSVFile( const std::string & file_name );
extern void WriteCfEqnCSVFile( const std::string & file_name );
extern void WritePartialCfMethodCSVFile( const std::string & file_name );

//======================== Surface Query Functions ======================//
extern vec3d CompPnt01(const std::string &geom_id, const int &surf_indx, const double &u, const double &w);
extern vec3d CompNorm01(const std::string &geom_id, const int &surf_indx, const double &u, const double &w);
extern vec3d CompTanU01(const std::string &geom_id, const int &surf_indx, const double &u, const double &w);
extern vec3d CompTanW01(const std::string &geom_id, const int &surf_indx, const double &u, const double &w);
extern void CompCurvature01(const std::string &geom_id, const int &surf_indx, const double &u, const double &w,
                            double &k1_out, double &k2_out, double &ka_out, double &kg_out);

extern double ProjPnt01(const std::string &geom_id, const int &surf_indx, const vec3d &pt, double &u_out, double &w_out);
extern double ProjPnt01I(const std::string &geom_id, const vec3d &pt, int &surf_indx_out, double &u_out, double &w_out);
extern double ProjPnt01Guess(const std::string &geom_id, const int &surf_indx, const vec3d &pt, const double &u0, const double &w0, double &u_out, double &w_out);

extern std::vector < vec3d > CompVecPnt01(const std::string &geom_id, const int &surf_indx, const std::vector < double > &u_in_vec, const std::vector < double > &w_in_vec);
extern std::vector < vec3d > CompVecNorm01(const std::string &geom_id, const int &surf_indx, const std::vector < double > &us, const std::vector < double > &ws);
extern void CompVecCurvature01(const std::string &geom_id, const int &surf_indx, const std::vector < double > &us, const std::vector < double > &ws, std::vector < double > &k1_out_vec, std::vector < double > &k2_out_vec, std::vector < double > &ka_out_vec, std::vector < double > &kg_out_vec);
extern void ProjVecPnt01(const std::string &geom_id, const int &surf_indx, const std::vector < vec3d > &pts, std::vector < double > &u_out_vec, std::vector < double > &w_out_vec, std::vector < double > &d_out_vec );
extern void ProjVecPnt01Guess(const std::string &geom_id, const int &surf_indx, const std::vector < vec3d > &pts, const std::vector < double > &u0s, const std::vector < double > &w0s, std::vector < double > &u_out_vec, std::vector < double > &w_out_vec, std::vector < double > &d_out_vec );

extern void GetUWTess01(const std::string &geom_id, const int &surf_indx, std::vector < double > &u_out_vec, std::vector < double > &w_out_vec);

//======================= Measure Functions ============================//
extern string AddRuler( const string & startgeomid, int startsurfindx, double startu, double startw,
                        const string & endgeomid, int endsurfindx, double endu, double endw, const string & name );
extern std::vector < string > GetAllRulers();
extern void DelRuler( const string &id );
extern void DeleteAllRulers();

extern string AddProbe( const string & geomid, int surfindx, double u, double w, const string & name );
extern std::vector < string > GetAllProbes();
extern void DelProbe( const string &id );
extern void DeleteAllProbes();

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
