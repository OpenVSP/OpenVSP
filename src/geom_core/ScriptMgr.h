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

    //==== Read Script From File - Return Module Name ====//
    string ReadScriptFromFile( const string & module_name, const string &  file_name );

    //==== Read Script From Memory - Return Module Name ====//
    string ReadScriptFromMemory( const string &  module_name, const string & script_content );


    void ExecuteScript(  const char* module_name,  const char* function_name );

    string FindModuleContent( const string & module_name );
    int SaveScriptContentToFile( const string & module_name, const string & file_name );

    void RunTestScripts();

    //==== Test Proxy Stuff ====//
    void SetSaveInt( int i )    { m_SaveInt = i; }
    int  GetSaveInt( )            { return m_SaveInt; }

    CScriptArray* GetProxyVec3dArray();
    CScriptArray* GetProxyStringArray();
    CScriptArray* GetProxyIntArray();
    CScriptArray* GetProxyDoubleArray();

    //==== Common Types =====//
    asIObjectType* m_IntArrayType;
    asIObjectType* m_DoubleArrayType;
    asIObjectType* m_Vec3dArrayType;
    asIObjectType* m_StringArrayType;

private:

    ScriptMgrSingleton();
    ScriptMgrSingleton( ScriptMgrSingleton const& copy );          // Not Implemented
    ScriptMgrSingleton& operator=( ScriptMgrSingleton const& copy ); // Not Implemented

    void RegisterEnums( asIScriptEngine* se );
    void RegisterVec3d( asIScriptEngine* se );
    void RegisterMatrix4d( asIScriptEngine* se );
    void RegisterCustomGeomMgr( asIScriptEngine* se );
    void RegisterAPIErrorObj( asIScriptEngine* se );
    void RegisterAPI( asIScriptEngine* se );

    //==== Member Variables ====//
    asIScriptEngine* m_ScriptEngine;
//    map< string, CScriptBuilder > m_BuilderMap;
    CScriptBuilder m_ScriptBuilder;
    map< string, string > m_ModuleContentMap;

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
    CScriptArray* GetXSecParmIDs( const string & xsec_id );
    CScriptArray* ReadFileXSec( const string& xsec_id, const string& file_name );
    CScriptArray* GetGeomSetAtIndex( int index );
    CScriptArray* GetGeomSet( const string & name );
    CScriptArray* GetAllResultsNames();
    CScriptArray* GetAllDataNames( const string & results_id );
    CScriptArray* GetIntResults( const string & id, const string & name, int index );
    CScriptArray* GetDoubleResults( const string & id, const string & name, int index );
    CScriptArray* GetStringResults( const string & id, const string & name, int index );
    CScriptArray* GetVec3dResults( const string & id, const string & name, int index );

    void SetXSecPnts( const string& xsec_id, CScriptArray* pnt_arr );
    void SetVec3dArray( CScriptArray* arr );

    string ExtractContent( const string & file_name );


};

#define ScriptMgr ScriptMgrSingleton::getInstance()

#endif // !defined(SCRIPTMGR__INCLUDED_)

