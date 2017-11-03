//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//

// ScriptMgr.h: interface to AngelScript
// J.R Gloudemans
//
//////////////////////////////////////////////////////////////////////

#if !defined(SCRIPTMGR__INCLUDED_)
#define SCRIPTMGR__INCLUDED_


#include <angelscript.h>
#include <scriptstdstring/scriptstdstring.h>
#include <scriptbuilder/scriptbuilder.h>
#include <scriptarray/scriptarray.h>
#include <scriptany/scriptany.h>
#include <scriptfile/scriptfile.h>
#include <scriptmath/scriptmath.h>

#include "Vec3d.h"
#include "XmlUtil.h"
#include "main.h"

#include <assert.h>
#include <string>
#include <vector>
#include <map>
using std::string;
using std::map;
using std::vector;

class ScriptMgrSingleton
{
public:
    static ScriptMgrSingleton& getInstance()
    {
        static ScriptMgrSingleton instance;
        return instance;
    }

    void Init();

    //==== Read And Execute Script File  ====//
    void ReadExecuteScriptFile( const string &  file_name, const string &  function_name = "void main()" );

    //==== Read Script From File - Return Module Name ====//
    string ReadScriptFromFile( const string & module_name, const string &  file_name );

    //==== Read All Scripts In Dir and Return Module Names ====//
    vector< string > ReadScriptsFromDir( const string & dir_name, const string & suffix );

    //==== Read Script From Memory - Return Module Name ====//
    string ReadScriptFromMemory( const string &  module_name, const string & script_content );

    //==== Find Script And Remove ====//
    bool RemoveScript( const string &  module_name );

    bool ExecuteScript(  const char* module_name,  const char* function_name, bool arg_flag = false, double arg = 0.0 );

    void AddToMessages( const string & msg )                { m_ScriptMessages += msg; }
    void ClearMessages()                                    { m_ScriptMessages.clear(); }
    string GetMessages()                                    { return m_ScriptMessages; }   

    string FindModuleContent( const string & module_name );
    string ExtractContent( const string & file_name );
    int SaveScriptContentToFile( const string & module_name, const string & file_name );
    string ReplaceIncludes( const string & script_contents, const string & inc_file_path );

    void RunTestScripts();

    //==== Test Proxy Stuff ====//
    void SetSaveInt( int i )      { m_SaveInt = i; }
    int  GetSaveInt( )            { return m_SaveInt; }

    CScriptArray* GetProxyVec3dArray();
    CScriptArray* GetProxyStringArray();
    CScriptArray* GetProxyIntArray();
    CScriptArray* GetProxyDoubleArray();

    void FillDoubleArray( vector < double > & in, CScriptArray* out );

    //==== Common Types =====//
    asITypeInfo* m_IntArrayType;
    asITypeInfo* m_DoubleArrayType;
    asITypeInfo* m_Vec3dArrayType;
    asITypeInfo* m_StringArrayType;

    //==== Utility ====//
    void Print( const string & data, bool new_line );
    void Print( const vec3d & data, bool new_line );
    void Print( double data, bool new_line );
    void Print( int data, bool new_line );

    double Rad2Deg( double r )                      { return r*RAD_2_DEG; }
    double Deg2Rad( double d )                      { return d*DEG_2_RAD; }
    double Min( double x, double y )                { return  (x < y ) ? x : y; }
    double Max( double x, double y )                { return  (x > y ) ? x : y; }
    string GetVSPVersion()                          { return VSPVERSION4; }

private:

    ScriptMgrSingleton();
    ScriptMgrSingleton( ScriptMgrSingleton const& copy );          // Not Implemented
    ScriptMgrSingleton& operator=( ScriptMgrSingleton const& copy ); // Not Implemented

    void RegisterEnums( asIScriptEngine* se );
    void RegisterVec3d( asIScriptEngine* se );
    void RegisterMatrix4d( asIScriptEngine* se );
    void RegisterCustomGeomMgr( asIScriptEngine* se );
    void RegisterAdvLinkMgr( asIScriptEngine* se );
    void RegisterAPIErrorObj( asIScriptEngine* se );
    void RegisterAPI( asIScriptEngine* se );
    void RegisterUtility( asIScriptEngine* se );

    //==== Member Variables ====//
    asIScriptEngine* m_ScriptEngine;
//    map< string, CScriptBuilder > m_BuilderMap;
    CScriptBuilder m_ScriptBuilder;
    map< string, string > m_ModuleContentMap;
    string m_ScriptMessages;

    //==== Test Proxy Stuff ====//
    int m_SaveInt;
    vector< vec3d > m_ProxyVec3dArray;
    vector< string > m_ProxyStringArray;
    vector< int > m_ProxyIntArray;
    vector< double > m_ProxyDoubleArray;

    CScriptArray* GetGeomTypes();
    CScriptArray* FindGeoms();
    CScriptArray* FindGeomsWithName( const string & name );
    CScriptArray* GetGeomParmIDs( const string & geom_id );
    CScriptArray* GetSubSurfIDVec( const string & geom_id );
    CScriptArray* GetAllSubSurfIDs();
    CScriptArray* GetSubSurf( const string & geom_id, const string & name );
    CScriptArray* GetSubSurfParmIDs( const string & sub_id );
    CScriptArray* GetXSecParmIDs( const string & xsec_id );
    CScriptArray* ReadFileXSec( const string& xsec_id, const string& file_name );
    CScriptArray* GetAirfoilUpperPnts( const string& xsec_id );
    CScriptArray* GetAirfoilLowerPnts( const string& xsec_id );
    CScriptArray* GetGeomSetAtIndex( int index );
    CScriptArray* GetGeomSet( const string & name );

    CScriptArray* ListAnalysis();
    CScriptArray* GetAnalysisInputNames( const string & analysis );
    CScriptArray* GetIntAnalysisInput( const string & analysis, const string & name, int index );
    CScriptArray* GetDoubleAnalysisInput( const string & analysis, const string & name, int index );
    CScriptArray* GetStringAnalysisInput( const string & analysis, const string & name, int index );
    CScriptArray* GetVec3dAnalysisInput( const string & analysis, const string & name, int index );

    CScriptArray* GetAllResultsNames();
    CScriptArray* GetAllDataNames( const string & results_id );
    CScriptArray* GetIntResults( const string & id, const string & name, int index );
    CScriptArray* GetDoubleResults( const string & id, const string & name, int index );
    CScriptArray* GetStringResults( const string & id, const string & name, int index );
    CScriptArray* GetVec3dResults( const string & id, const string & name, int index );
    CScriptArray* FindContainers();
    CScriptArray* FindContainersWithName( const string & name );
    CScriptArray* FindContainerGroupNames( const string & parm_container_id );
    CScriptArray* FindContainerParmIDs( const string & parm_container_id );
    CScriptArray* GetUpperCSTCoefs( const string & xsec_id );
    CScriptArray* GetLowerCSTCoefs( const string & xsec_id );

    void DeleteGeomVec( CScriptArray* del_arr );

    void SetXSecPnts( const string& xsec_id, CScriptArray* pnt_arr );
    void SetAirfoilPnts( const string& xsec_id, CScriptArray* up_pnt_arr, CScriptArray* low_pnt_arr );
    void SetVec3dArray( CScriptArray* arr );

    CScriptArray* GetVKTAirfoilPnts( const int npts, const double alpha, const double epsilon, const double kappa, const double tau );
    CScriptArray* GetVKTAirfoilCpDist( const double alpha, const double epsilon, const double kappa, const double tau, CScriptArray* xyz_data );
    CScriptArray* GetEllipsoidSurfPnts( const vec3d& center, const vec3d& abc_rad, int u_npts = 20, int w_npts = 20 );

    void SetUpperCST( const string& xsec_id, int deg, CScriptArray* coefs );
    void SetLowerCST( const string& xsec_id, int deg, CScriptArray* coefs );

    void SetIntAnalysisInput( const string& analysis, const string & name, CScriptArray* indata, int index );
    void SetDoubleAnalysisInput( const string& analysis, const string & name, CScriptArray* indata, int index );
    void SetStringAnalysisInput( const string& analysis, const string & name, CScriptArray* indata, int index );
    void SetVec3dAnalysisInput( const string& analysis, const string & name, CScriptArray* indata, int index );

    // ==== Variable Preset Functions ====//
    CScriptArray* GetVarPresetGroupNames();
    CScriptArray* GetVarPresetSettingNamesWName( string group_name );
    CScriptArray* GetVarPresetSettingNamesWIndex( int group_index );
    CScriptArray* GetVarPresetParmVals();
    CScriptArray* GetVarPresetParmValsWNames( string group_name, string setting_name );
    CScriptArray* GetVarPresetParmIDs();
    CScriptArray* GetVarPresetParmIDsWName( string group_name );
    void AddVarPresetGroup( const string &group_name );
    void AddVarPresetSetting( const string &setting_name );
    void AddVarPresetParm( const string &parm_ID );
    void AddVarPresetParm( const string &parm_ID, string group_name );
    void EditVarPresetParm( const string &parm_ID, double parm_val );
    void EditVarPresetParm( const string &parm_ID, double parm_val, string group_name, string setting_name );
    void DeleteVarPresetParm( const string &parm_ID );
    void DeleteVarPresetParm( const string &parm_ID, string group_name );
    void SwitchVarPreset( string group_name, string setting_name );
    void DeleteVarPresetSet( string group_name, string setting_name );

    // ==== PCurve Functions ====//
    void SetPCurve( const string& geom_id, const int & pcurveid, CScriptArray* tvec, CScriptArray* valvec, const int & newtype );
    CScriptArray* PCurveGetTVec( const std::string & geom_id, const int & pcurveid );
    CScriptArray* PCurveGetValVec( const std::string & geom_id, const int & pcurveid );

    // ==== Parasite Drag Tool Functions ====//
    void AddExcrescence(const std::string & excresName, int excresType, double excresVal);
    void DeleteExcrescence(int index);

    //=== Register Measure Functions ===//
    CScriptArray* GetAllRulers();
    CScriptArray* GetAllProbes();

    CScriptArray* CompVecPnt01(const string &geom_id, const int &surf_indx, CScriptArray* us, CScriptArray* ws);
    CScriptArray* CompVecNorm01(const string &geom_id, const int &surf_indx, CScriptArray* us, CScriptArray* ws);
    void CompVecCurvature01(const string &geom_id, const int &surf_indx, CScriptArray* us, CScriptArray* ws, CScriptArray* k1s, CScriptArray* k2s, CScriptArray* kas, CScriptArray* kgs);
    void ProjVecPnt01(const string &geom_id, int &surf_indx, CScriptArray* pts, CScriptArray* us, CScriptArray* ws, CScriptArray* ds );
    void ProjVecPnt01Guess(const string &geom_id, int &surf_indx, CScriptArray* pts, CScriptArray* u0s, CScriptArray* w0s, CScriptArray* us, CScriptArray* ws, CScriptArray* ds );
    void GetUWTess01(const string &geom_id, int &surf_indx, CScriptArray* us, CScriptArray* ws );

};

#define ScriptMgr ScriptMgrSingleton::getInstance()

#endif // !defined(SCRIPTMGR__INCLUDED_)

