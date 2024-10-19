//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//

// ScriptMgr.h: interface to AngelScript
// J.R Gloudemans
//
// Note: For consistency when adding Doxygen comments, follow the Qt style
// identified here: http://www.doxygen.nl/manual/docblocks.html#specialblock
//
//
//////////////////////////////////////////////////////////////////////

#include "ScriptMgr.h"

#include "Parm.h"
#include "Matrix4d.h"
#include "VSP_Geom_API.h"
#include "CustomGeom.h"
#include "AdvLinkMgr.h"
#include "AttributeManager.h"
#include "StringUtil.h"
#include "FileUtil.h"

// Make sure int32_t is defined.
#ifdef _MSC_VER
    #if _MSC_VER >= 1600
        #include <cstdint>
    #else
        typedef __int32 int32_t;
    #endif
#else
    #include <cstdint>
#endif

using namespace vsp;

//==== Implement a simple message callback function ====//
void MessageCallback( const asSMessageInfo *msg, void *param )
{
    char str[1024];
    const char *type = "ERR ";
    if( msg->type == asMSGTYPE_WARNING )
    {
        type = "WARN";
    }
    else if( msg->type == asMSGTYPE_INFORMATION )
    {
        type = "INFO";
    }
    snprintf( str, sizeof( str ),  "%s (%d, %d) : %s : %s\n", msg->section, msg->row, msg->col, type, msg->message );
    ScriptMgr.AddToMessages( str );
    printf( "%s", str );
}

//==================================================================================================//
//========================================= ScriptMgr      =========================================//
//==================================================================================================//


//==== Constructor ====//
ScriptMgrSingleton::ScriptMgrSingleton()
{
    m_SaveInt = 0;
    m_ScriptEngine = nullptr;
    m_ScriptMessages = "";

    m_IntArrayType = nullptr;
    m_DoubleArrayType = nullptr;
    m_DoubleMatArrayType = nullptr;
    m_Vec3dArrayType = nullptr;
    m_StringArrayType = nullptr;

}

//==== Set Up Script Engine, Script Error Callbacks ====//
void ScriptMgrSingleton::Init( )
{
    //==== Only Init Once ====//
    static bool init_flag = false;
    if ( init_flag )
        return;
    init_flag = true;

    //==== Create the Script Engine ====//
    m_ScriptEngine = asCreateScriptEngine( ANGELSCRIPT_VERSION );
    asIScriptEngine* se = m_ScriptEngine;

    //==== Set the message callback to receive information on errors in human readable form.  ====//
    int r = se->SetMessageCallback( asFUNCTION( MessageCallback ), 0, asCALL_CDECL );
    assert( r >= 0 );

    //==== Register Addons ====//
    RegisterStdString( m_ScriptEngine );
    RegisterScriptArray( m_ScriptEngine, true );
    RegisterScriptDateTime( m_ScriptEngine );
    RegisterScriptFile( m_ScriptEngine );
    RegisterScriptFileSystem( m_ScriptEngine );
    RegisterStdStringUtils( m_ScriptEngine );
    RegisterScriptMath( m_ScriptEngine );
    RegisterScriptAny( m_ScriptEngine );
    //==== Cache Some Common Types ====//
    m_IntArrayType    = se->GetTypeInfoById( se->GetTypeIdByDecl( "array<int>" ) );
    assert( m_IntArrayType );
    m_IntMatArrayType = se->GetTypeInfoById( se->GetTypeIdByDecl( "array<array<int>>" ) );
    assert( m_IntMatArrayType );
    m_DoubleArrayType = se->GetTypeInfoById( se->GetTypeIdByDecl( "array<double>" ) );
    assert( m_DoubleArrayType );
    m_DoubleMatArrayType = se->GetTypeInfoById( se->GetTypeIdByDecl( "array<array<double>>" ) );
    assert( m_DoubleMatArrayType );
    m_StringArrayType = se->GetTypeInfoById( se->GetTypeIdByDecl( "array<string>" ) );
    assert( m_StringArrayType );

    //==== Register VSP Enums ====//
    RegisterEnums( m_ScriptEngine );

    //==== Register VSP Objects ====//
    RegisterVec3d( m_ScriptEngine );
    m_Vec3dArrayType  = se->GetTypeInfoById( se->GetTypeIdByDecl( "array<vec3d>" ) );
    assert( m_Vec3dArrayType );

    RegisterMatrix4d( m_ScriptEngine );
    RegisterCustomGeomMgr( m_ScriptEngine );
    RegisterAdvLinkMgr( m_ScriptEngine );
    RegisterAPIErrorObj( m_ScriptEngine );
    RegisterAPI( m_ScriptEngine );
    RegisterUtility(  m_ScriptEngine );
}

void ScriptMgrSingleton::RunTestScripts()
{
    ////===== Run Test Scripts ====//
    //ScriptMgr.ReadScript( "TestScript", "../../TestScript.as"  );
    //ScriptMgr.ReadScriptFromFile( "TestScript", "../../../TestScript.as"  );
    //ScriptMgr.ExecuteScript( "TestScript", "void main()" );
    //ScriptMgr.ExecuteScript( "TestScript", "void TestAPIScript()" );
}

//==== Read And Execute Script File  ====//
int ScriptMgrSingleton::ReadExecuteScriptFile( const string &  file_name, const string &  function_name )
{
    string module_name = ReadScriptFromFile( "ReadExecute", file_name );

    return ExecuteScript( module_name.c_str(), function_name.c_str(), false, 0.0, false );
}

vector< string > ScriptMgrSingleton::ReadScriptsFromDir( const string & dir_name, const string & suffix )
{
    vector< string > mod_name_vec;

    vector< string > file_vec = ScanFolder( dir_name.c_str() );

    for ( int i = 0 ; i < ( int )file_vec.size() ; i++ )
    {
        int s_num = suffix.size();
        if ( file_vec[i].size() > s_num )
        {
            if ( file_vec[i].compare( file_vec[i].size() - s_num, s_num, suffix.c_str() ) == 0 )
            {
                string sub = file_vec[i].substr( 0, file_vec[i].size() - s_num );
                string file_name = dir_name;
                file_name.append( file_vec[i] );
                string module_name = ScriptMgr.ReadScriptFromFile( sub, file_name );

                if ( module_name.size() )
                    mod_name_vec.push_back( module_name );
            }
        }
    }

    return mod_name_vec;
}

//==== Start A New Module And Read Script ====//
string ScriptMgrSingleton::ReadScriptFromFile( const string & module_name, const string &  file_name )
{
    string content = ExtractContent( file_name );

    if ( content.size() < 2 )
    {
        return string();
    }

    return ReadScriptFromMemory( module_name, content );
}

//==== Start A New Module And Read Script ====//
string ScriptMgrSingleton::ReadScriptFromMemory( const string &  module_name, const string & script_content )
{
    int r;
    string updated_module_name = module_name;
    unordered_map< string, string >::iterator iter;

    //==== Check If Module Name Already Exists ====//
    iter = m_ModuleContentMap.find(updated_module_name);
    if ( iter != m_ModuleContentMap.end() )
    {
        //==== Check If Content is Same ====//
        if ( iter->second == script_content )
            return iter->first;

        //==== Need To Change Module Name ====//
        static int dup_cnt = 0;
        updated_module_name.append( StringUtil::int_to_string( dup_cnt, "%d" ) );
        dup_cnt++;
    }

    //==== Make Sure Not Duplicate Of Any Other Module ====//
    for ( iter = m_ModuleContentMap.begin() ; iter != m_ModuleContentMap.end() ; iter++ )
    {
        if ( iter->second == script_content )
            return iter->first;
    }

    //==== Start A New Module ====//
    r = m_ScriptBuilder.StartNewModule( m_ScriptEngine, updated_module_name.c_str() );
    if( r < 0 )        return string();

    r = m_ScriptBuilder.AddSectionFromMemory( updated_module_name.c_str(), script_content.c_str(), script_content.size()  );
    if ( r < 0 )    return string();

    r = m_ScriptBuilder.BuildModule();
    if ( r < 0 )    return string();

    //==== Add To Map ====//
    m_ModuleContentMap[ updated_module_name ] = script_content;

    return updated_module_name;
}

//==== Extract Content From File Into String ====//
string ScriptMgrSingleton::ExtractContent( const string & file_name )
{
    string file_content;
    FILE* fp = fopen( file_name.c_str(), "r" );
    if ( fp )
    {
        char buff[512];
        while ( fgets( buff, 512, fp ) )
        {
            file_content.append( buff );
        }
        file_content.append( "\0" );
        fclose( fp );
    }
    return file_content;
}

//==== Find Script And Remove ====//
bool ScriptMgrSingleton::RemoveScript( const string &  module_name )
{
    //==== Find Module ====//
    unordered_map< string, string >::iterator iter;
    iter = m_ModuleContentMap.find(module_name);
    if ( iter == m_ModuleContentMap.end() )
    {
        return false;                           // Could not find module name;
    }

    m_ModuleContentMap.erase( iter );

    int ret = m_ScriptEngine->DiscardModule( module_name.c_str() );

    if ( ret < 0 )
        return false;
    return true;
}


//==== Execute Function in Module ====//
int ScriptMgrSingleton::ExecuteScript( const char* module_name, const char* function_name, bool arg_flag, double arg, bool by_decl )
{
    // Find the function that is to be called.
    asIScriptModule *mod = m_ScriptEngine->GetModule( module_name );

    if ( !mod )
    {
        printf( "Error ExecuteScript GetModule %s\n", module_name );
        return 1;
    }

    asIScriptFunction *func = nullptr;
    if ( by_decl )
    {
        func = mod->GetFunctionByDecl( function_name );
    }
    else
    {
        func = mod->GetFunctionByName( function_name );
    }

    if( func == 0 )
    {
        return 1;
    }

    // Create our context, prepare it, and then execute
    asIScriptContext *ctx = m_ScriptEngine->CreateContext();
    ctx->Prepare( func );
    if ( arg_flag )
    {
        ctx->SetArgDouble( 0, arg );
    }
    int r = ctx->Execute();
    if( r != asEXECUTION_FINISHED )
    {
        // The execution didn't complete as expected. Determine what happened.
        if( r == asEXECUTION_EXCEPTION )
        {
            // An exception occurred, let the script writer know what happened so it can be corrected.
            printf( "An exception '%s' occurred \n", ctx->GetExceptionString() );
        }
        ctx->Release();
        return 1;
    }

    asDWORD ret = ctx->GetReturnDWord();
    int32_t rval = ret;

    ctx->Release();
    return rval;
}

//==== Return Script Content Given Module Name ====//
string ScriptMgrSingleton::FindModuleContent( const string &  module_name )
{
    unordered_map< string, string >::iterator iter;
    iter = m_ModuleContentMap.find( module_name );

    string file_string;
    if ( iter != m_ModuleContentMap.end() )
    {
        file_string = iter->second;
    }
    return file_string;
}

//==== Write Script Content To File ====//
int ScriptMgrSingleton::SaveScriptContentToFile( const string & module_name, const string & file_name )
{
    unordered_map< string, string >::iterator iter;
    iter = m_ModuleContentMap.find( module_name );

    if ( iter == m_ModuleContentMap.end() )
        return -1;

    FILE* fp = fopen( file_name.c_str(), "w" );
    if ( !fp )
        return -2;

    if ( iter->second.size() == 0 )
        return -3;


    fprintf( fp, "%s", iter->second.c_str() );
    fclose( fp );

    return 0;
}

//==== Find Includes And Replace With Included Code ====//
string ScriptMgrSingleton::ReplaceIncludes( const string & script_contents, const string & inc_file_path )
{
    vector< string::size_type > start_pos_vec;
    vector< string::size_type > end_pos_vec;
    vector< string > file_content_vec;

    string::size_type find_pos = 0;

    string ret_content;

    while ( 1 )
    {
        //==== Find Include ====//
        find_pos = script_contents.find( "#include", find_pos );
        if ( find_pos ==  std::string::npos )
            break;

        string::size_type first_quote = script_contents.find( '"', find_pos + 8 );
        if ( first_quote == std::string::npos )
            break;

        string::size_type second_quote = script_contents.find( '"', first_quote+1 );
        if ( second_quote == std::string::npos )
            break;

        start_pos_vec.push_back( find_pos );
        end_pos_vec.push_back( second_quote );

        string inc_file_name = script_contents.substr( first_quote+1, second_quote - first_quote - 1 );

        string full_path = inc_file_path + inc_file_name;
        string content = ExtractContent( full_path );
        file_content_vec.push_back( content );

        find_pos = second_quote+1;
    }

    //==== No Includes ====//
    if ( file_content_vec.size() == 0 )
        return script_contents;

    string::size_type curr_pos = 0;
    for ( int i = 0 ; i < (int)file_content_vec.size() ; i++ )
    {
        string::size_type s = start_pos_vec[i];
        string::size_type e = end_pos_vec[i];
        ret_content.append( script_contents.substr( curr_pos, s - curr_pos ) );

        ret_content.append( "// Begin Include Replacement\n" );
        ret_content.append( "//" );
        ret_content.append( script_contents, s, e-s+1 );
        ret_content.append( "\n" );
        if ( file_content_vec[i].size() > 0 )
        {
            ret_content.append( file_content_vec[i] );
        }
        ret_content.append( "// End Include Replacement\n" );

        curr_pos = end_pos_vec[i] + 1;
    }

    ret_content.append( script_contents.substr( curr_pos, script_contents.size() - curr_pos ) );

    //FILE * fp = fopen( "TestWrite.txt", "w" );
    //if ( fp )
    //{
    //    fprintf( fp, "%s", ret_content.c_str() );
    //    fclose( fp );
    //};

    return ret_content;
}









//==== Register Enums ====//
void ScriptMgrSingleton::RegisterEnums( asIScriptEngine* se )
{
    // The format for enum comments:
        //! Brief description that appears in overview.
        /*! A more detailed description. */

    int r = se->RegisterEnum( "ABS_REL_FLAG" );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "ABS_REL_FLAG", "ABS", ABS );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "ABS_REL_FLAG", "REL", REL );
    assert( r >= 0 );


    r = se->RegisterEnum( "AIRFOIL_EXPORT_TYPE");
    assert( r >= 0 );
    r = se->RegisterEnumValue( "AIRFOIL_EXPORT_TYPE", "SELIG_AF_EXPORT", SELIG_AF_EXPORT );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "AIRFOIL_EXPORT_TYPE", "BEZIER_AF_EXPORT", BEZIER_AF_EXPORT );
    assert( r >= 0 );


    r = se->RegisterEnum( "ANG_UNITS" );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "ANG_UNITS", "ANG_RAD", ANG_RAD );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "ANG_UNITS", "ANG_DEG", ANG_DEG );
    assert( r >= 0 );


    r = se->RegisterEnum( "ATMOS_TYPE" );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "ATMOS_TYPE", "ATMOS_TYPE_US_STANDARD_1976", ATMOS_TYPE_US_STANDARD_1976 );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "ATMOS_TYPE", "ATMOS_TYPE_HERRINGTON_1966", ATMOS_TYPE_HERRINGTON_1966 );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "ATMOS_TYPE", "ATMOS_TYPE_MANUAL_P_R", ATMOS_TYPE_MANUAL_P_R );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "ATMOS_TYPE", "ATMOS_TYPE_MANUAL_P_T", ATMOS_TYPE_MANUAL_P_T );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "ATMOS_TYPE", "ATMOS_TYPE_MANUAL_R_T", ATMOS_TYPE_MANUAL_R_T );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "ATMOS_TYPE", "ATMOS_TYPE_MANUAL_RE_L", ATMOS_TYPE_MANUAL_RE_L );
    assert( r >= 0 );


    r = se->RegisterEnum( "ATTACH_TRANS_TYPE" );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "ATTACH_TRANS_TYPE", "ATTACH_TRANS_NONE", ATTACH_TRANS_NONE );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "ATTACH_TRANS_TYPE", "ATTACH_TRANS_COMP", ATTACH_TRANS_COMP );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "ATTACH_TRANS_TYPE", "ATTACH_TRANS_UV", ATTACH_TRANS_UV );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "ATTACH_TRANS_TYPE", "ATTACH_TRANS_RST", ATTACH_TRANS_RST );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "ATTACH_TRANS_TYPE", "ATTACH_TRANS_LMN", ATTACH_TRANS_LMN );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "ATTACH_TRANS_TYPE", "ATTACH_TRANS_EtaMN", ATTACH_TRANS_EtaMN );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "ATTACH_TRANS_TYPE", "ATTACH_TRANS_NUM_TYPES", ATTACH_TRANS_NUM_TYPES );
    assert( r >= 0 );


    r = se->RegisterEnum( "ATTACH_ROT_TYPE" );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "ATTACH_ROT_TYPE", "ATTACH_ROT_NONE", ATTACH_ROT_NONE );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "ATTACH_ROT_TYPE", "ATTACH_ROT_COMP", ATTACH_ROT_COMP );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "ATTACH_ROT_TYPE", "ATTACH_ROT_UV", ATTACH_ROT_UV );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "ATTACH_ROT_TYPE", "ATTACH_ROT_RST", ATTACH_ROT_RST );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "ATTACH_ROT_TYPE", "ATTACH_ROT_LMN", ATTACH_ROT_LMN );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "ATTACH_ROT_TYPE", "ATTACH_ROT_EtaMN", ATTACH_ROT_EtaMN );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "ATTACH_ROT_TYPE", "ATTACH_ROT_NUM_TYPES", ATTACH_ROT_NUM_TYPES );
    assert( r >= 0 );

    r = se->RegisterEnum( "ATTRIBUTABLE_TYPE" );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "ATTRIBUTABLE_TYPE", "ATTROBJ_PARM", ATTROBJ_PARM );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "ATTRIBUTABLE_TYPE", "ATTROBJ_GEOM", ATTROBJ_GEOM );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "ATTRIBUTABLE_TYPE", "ATTROBJ_VEH", ATTROBJ_VEH );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "ATTRIBUTABLE_TYPE", "ATTROBJ_SUBSURF", ATTROBJ_SUBSURF );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "ATTRIBUTABLE_TYPE", "ATTROBJ_MEASURE", ATTROBJ_MEASURE );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "ATTRIBUTABLE_TYPE", "ATTROBJ_LINK", ATTROBJ_LINK );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "ATTRIBUTABLE_TYPE", "ATTROBJ_ADVLINK", ATTROBJ_ADVLINK );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "ATTRIBUTABLE_TYPE", "ATTROBJ_ATTR", ATTROBJ_ATTR );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "ATTRIBUTABLE_TYPE", "ATTROBJ_COLLECTION", ATTROBJ_COLLECTION );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "ATTRIBUTABLE_TYPE", "ATTROBJ_FREE", ATTROBJ_FREE );
    assert( r >= 0 );

    r = se->RegisterEnum( "ATTRIBUTE_EVENT_GROUP" );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "ATTRIBUTE_EVENT_GROUP", "ATTR_GROUP_NONE", ATTR_GROUP_NONE );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "ATTRIBUTE_EVENT_GROUP", "ATTR_GROUP_WATERMARK", ATTR_GROUP_WATERMARK );
    assert( r >= 0 );


    r = se->RegisterEnum( "AUX_GEOM_MODE" );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "AUX_GEOM_MODE", "AUX_GEOM_ROTOR_TIP_PATH", AUX_GEOM_ROTOR_TIP_PATH );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "AUX_GEOM_MODE", "AUX_GEOM_ROTOR_BURST", AUX_GEOM_ROTOR_BURST );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "AUX_GEOM_MODE", "AUX_GEOM_THREE_PT_GROUND", AUX_GEOM_THREE_PT_GROUND );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "AUX_GEOM_MODE", "AUX_GEOM_TWO_PT_GROUND", AUX_GEOM_TWO_PT_GROUND );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "AUX_GEOM_MODE", "AUX_GEOM_ONE_PT_GROUND", AUX_GEOM_ONE_PT_GROUND );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "AUX_GEOM_MODE", "AUX_GEOM_THREE_PT_CCE", AUX_GEOM_THREE_PT_CCE );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "AUX_GEOM_MODE", "AUX_GEOM_SUPER_CONE", AUX_GEOM_SUPER_CONE );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "AUX_GEOM_MODE", "NUM_AUX_GEOM_MODES", NUM_AUX_GEOM_MODES );
    assert( r >= 0 );


    r = se->RegisterEnum( "BOGIE_SPACING_TYPE" );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "BOGIE_SPACING_TYPE", "BOGIE_CENTER_DIST", BOGIE_CENTER_DIST );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "BOGIE_SPACING_TYPE", "BOGIE_CENTER_DIST_FRAC", BOGIE_CENTER_DIST_FRAC );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "BOGIE_SPACING_TYPE", "BOGIE_GAP", BOGIE_GAP );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "BOGIE_SPACING_TYPE", "BOGIE_GAP_FRAC", BOGIE_GAP_FRAC );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "BOGIE_SPACING_TYPE", "NUM_BOGIE_SPACING_TYPE", NUM_BOGIE_SPACING_TYPE );
    assert( r >= 0 );


    r = se->RegisterEnum( "BOR_MODE" );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "BOR_MODE", "BOR_FLOWTHROUGH", BOR_FLOWTHROUGH );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "BOR_MODE", "BOR_UPPER", BOR_UPPER );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "BOR_MODE", "BOR_LOWER", BOR_LOWER );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "BOR_MODE", "BOR_NUM_MODES", BOR_NUM_MODES );
    assert( r >= 0 );


    r = se->RegisterEnum( "CAMBER_INPUT_FLAG" );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "CAMBER_INPUT_FLAG", "MAX_CAMB", MAX_CAMB );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "CAMBER_INPUT_FLAG", "DESIGN_CL", DESIGN_CL );
    assert( r >= 0 );


    r = se->RegisterEnum( "CAMERA_VIEW" );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "CAMERA_VIEW", "CAM_TOP", CAM_TOP );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "CAMERA_VIEW", "CAM_FRONT", CAM_FRONT );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "CAMERA_VIEW", "CAM_FRONT_YUP", CAM_FRONT_YUP );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "CAMERA_VIEW", "CAM_LEFT", CAM_LEFT );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "CAMERA_VIEW", "CAM_LEFT_ISO", CAM_LEFT_ISO );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "CAMERA_VIEW", "CAM_BOTTOM", CAM_BOTTOM );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "CAMERA_VIEW", "CAM_REAR", CAM_REAR );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "CAMERA_VIEW", "CAM_RIGHT", CAM_RIGHT );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "CAMERA_VIEW", "CAM_RIGHT_ISO", CAM_RIGHT_ISO );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "CAMERA_VIEW", "CAM_CENTER", CAM_CENTER );
    assert( r >= 0 );


    r = se->RegisterEnum( "CAP_TYPE" );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "CAP_TYPE", "NO_END_CAP", vsp::NO_END_CAP );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "CAP_TYPE", "FLAT_END_CAP", vsp::FLAT_END_CAP );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "CAP_TYPE", "ROUND_END_CAP", vsp::ROUND_END_CAP );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "CAP_TYPE", "EDGE_END_CAP", vsp::EDGE_END_CAP );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "CAP_TYPE", "SHARP_END_CAP", vsp::SHARP_END_CAP );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "CAP_TYPE", "ROUND_EXT_END_CAP_NONE", vsp::ROUND_EXT_END_CAP_NONE );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "CAP_TYPE", "ROUND_EXT_END_CAP_LE", vsp::ROUND_EXT_END_CAP_LE );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "CAP_TYPE", "ROUND_EXT_END_CAP_TE", vsp::ROUND_EXT_END_CAP_TE );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "CAP_TYPE", "ROUND_EXT_END_CAP_BOTH", vsp::ROUND_EXT_END_CAP_BOTH );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "CAP_TYPE", "NUM_END_CAP_OPTIONS", vsp::NUM_END_CAP_OPTIONS );
    assert( r >= 0 );


    r = se->RegisterEnum( "CFD_CONTROL_TYPE" );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "CFD_CONTROL_TYPE", "CFD_MIN_EDGE_LEN", CFD_MIN_EDGE_LEN );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "CFD_CONTROL_TYPE", "CFD_MAX_EDGE_LEN", CFD_MAX_EDGE_LEN );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "CFD_CONTROL_TYPE", "CFD_MAX_GAP", CFD_MAX_GAP );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "CFD_CONTROL_TYPE", "CFD_NUM_CIRCLE_SEGS", CFD_NUM_CIRCLE_SEGS );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "CFD_CONTROL_TYPE", "CFD_GROWTH_RATIO", CFD_GROWTH_RATIO );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "CFD_CONTROL_TYPE", "CFD_LIMIT_GROWTH_FLAG", CFD_LIMIT_GROWTH_FLAG );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "CFD_CONTROL_TYPE", "CFD_INTERSECT_SUBSURFACE_FLAG", CFD_INTERSECT_SUBSURFACE_FLAG );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "CFD_CONTROL_TYPE", "CFD_HALF_MESH_FLAG", CFD_HALF_MESH_FLAG );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "CFD_CONTROL_TYPE", "CFD_FAR_FIELD_FLAG", CFD_FAR_FIELD_FLAG );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "CFD_CONTROL_TYPE", "CFD_FAR_MAX_EDGE_LEN", CFD_FAR_MAX_EDGE_LEN );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "CFD_CONTROL_TYPE", "CFD_FAR_MAX_GAP", CFD_FAR_MAX_GAP );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "CFD_CONTROL_TYPE", "CFD_FAR_NUM_CIRCLE_SEGS", CFD_FAR_NUM_CIRCLE_SEGS );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "CFD_CONTROL_TYPE", "CFD_FAR_SIZE_ABS_FLAG", CFD_FAR_SIZE_ABS_FLAG );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "CFD_CONTROL_TYPE", "CFD_FAR_LENGTH", CFD_FAR_LENGTH );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "CFD_CONTROL_TYPE", "CFD_FAR_WIDTH", CFD_FAR_WIDTH );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "CFD_CONTROL_TYPE", "CFD_FAR_HEIGHT", CFD_FAR_HEIGHT );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "CFD_CONTROL_TYPE", "CFD_FAR_X_SCALE", CFD_FAR_X_SCALE );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "CFD_CONTROL_TYPE", "CFD_FAR_Y_SCALE", CFD_FAR_Y_SCALE );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "CFD_CONTROL_TYPE", "CFD_FAR_Z_SCALE", CFD_FAR_Z_SCALE );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "CFD_CONTROL_TYPE", "CFD_FAR_LOC_MAN_FLAG", CFD_FAR_LOC_MAN_FLAG );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "CFD_CONTROL_TYPE", "CFD_FAR_LOC_X", CFD_FAR_LOC_X );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "CFD_CONTROL_TYPE", "CFD_FAR_LOC_Y", CFD_FAR_LOC_Y );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "CFD_CONTROL_TYPE", "CFD_FAR_LOC_Z", CFD_FAR_LOC_Z );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "CFD_CONTROL_TYPE", "CFD_SRF_XYZ_FLAG", CFD_SRF_XYZ_FLAG );
    assert( r >= 0 );


    r = se->RegisterEnum( "CFD_MESH_EXPORT_TYPE" );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "CFD_MESH_EXPORT_TYPE", "CFD_STL_FILE_NAME", CFD_STL_FILE_NAME );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "CFD_MESH_EXPORT_TYPE", "CFD_POLY_FILE_NAME", CFD_POLY_FILE_NAME );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "CFD_MESH_EXPORT_TYPE", "CFD_TRI_FILE_NAME", CFD_TRI_FILE_NAME );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "CFD_MESH_EXPORT_TYPE", "CFD_OBJ_FILE_NAME", CFD_OBJ_FILE_NAME );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "CFD_MESH_EXPORT_TYPE", "CFD_DAT_FILE_NAME", CFD_DAT_FILE_NAME );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "CFD_MESH_EXPORT_TYPE", "CFD_KEY_FILE_NAME", CFD_KEY_FILE_NAME );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "CFD_MESH_EXPORT_TYPE", "CFD_GMSH_FILE_NAME", CFD_GMSH_FILE_NAME );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "CFD_MESH_EXPORT_TYPE", "CFD_TKEY_FILE_NAME", CFD_TKEY_FILE_NAME );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "CFD_MESH_EXPORT_TYPE", "CFD_FACET_FILE_NAME", CFD_FACET_FILE_NAME );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "CFD_MESH_EXPORT_TYPE", "CFD_VSPGEOM_FILE_NAME", CFD_VSPGEOM_FILE_NAME );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "CFD_MESH_EXPORT_TYPE", "CFD_NUM_FILE_NAMES", CFD_NUM_FILE_NAMES );
    assert( r >= 0 );


    r = se->RegisterEnum( "CFD_MESH_SOURCE_TYPE" );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "CFD_MESH_SOURCE_TYPE", "POINT_SOURCE", POINT_SOURCE );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "CFD_MESH_SOURCE_TYPE", "LINE_SOURCE", LINE_SOURCE );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "CFD_MESH_SOURCE_TYPE", "BOX_SOURCE", BOX_SOURCE );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "CFD_MESH_SOURCE_TYPE", "ULINE_SOURCE", ULINE_SOURCE );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "CFD_MESH_SOURCE_TYPE", "WLINE_SOURCE", WLINE_SOURCE );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "CFD_MESH_SOURCE_TYPE", "NUM_SOURCE_TYPES", NUM_SOURCE_TYPES );
    assert( r >= 0 );


    r = se->RegisterEnum( "CF_LAM_EQN" );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "CF_LAM_EQN", "CF_LAM_BLASIUS", CF_LAM_BLASIUS );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "CF_LAM_EQN", "CF_LAM_BLASIUS_W_HEAT", CF_LAM_BLASIUS_W_HEAT ); // TODO: Remove or implement
    assert( r >= 0 );


    r = se->RegisterEnum( "CF_TURB_EQN" );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "CF_TURB_EQN", "CF_TURB_EXPLICIT_FIT_SPALDING", CF_TURB_EXPLICIT_FIT_SPALDING );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "CF_TURB_EQN", "CF_TURB_EXPLICIT_FIT_SPALDING_CHI", CF_TURB_EXPLICIT_FIT_SPALDING_CHI );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "CF_TURB_EQN", "CF_TURB_EXPLICIT_FIT_SCHOENHERR", CF_TURB_EXPLICIT_FIT_SCHOENHERR );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "CF_TURB_EQN", "DO_NOT_USE_CF_TURB_IMPLICIT_KARMAN", DO_NOT_USE_CF_TURB_IMPLICIT_KARMAN );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "CF_TURB_EQN", "CF_TURB_IMPLICIT_SCHOENHERR", CF_TURB_IMPLICIT_SCHOENHERR );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "CF_TURB_EQN", "CF_TURB_IMPLICIT_KARMAN_SCHOENHERR", CF_TURB_IMPLICIT_KARMAN_SCHOENHERR );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "CF_TURB_EQN", "CF_TURB_POWER_LAW_BLASIUS", CF_TURB_POWER_LAW_BLASIUS );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "CF_TURB_EQN", "CF_TURB_POWER_LAW_PRANDTL_LOW_RE", CF_TURB_POWER_LAW_PRANDTL_LOW_RE );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "CF_TURB_EQN", "CF_TURB_POWER_LAW_PRANDTL_MEDIUM_RE", CF_TURB_POWER_LAW_PRANDTL_MEDIUM_RE );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "CF_TURB_EQN", "CF_TURB_POWER_LAW_PRANDTL_HIGH_RE", CF_TURB_POWER_LAW_PRANDTL_HIGH_RE );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "CF_TURB_EQN", "CF_TURB_SCHLICHTING_COMPRESSIBLE", CF_TURB_SCHLICHTING_COMPRESSIBLE );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "CF_TURB_EQN", "DO_NOT_USE_CF_TURB_SCHLICHTING_INCOMPRESSIBLE", DO_NOT_USE_CF_TURB_SCHLICHTING_INCOMPRESSIBLE );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "CF_TURB_EQN", "DO_NOT_USE_CF_TURB_SCHLICHTING_PRANDTL", DO_NOT_USE_CF_TURB_SCHLICHTING_PRANDTL );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "CF_TURB_EQN", "DO_NOT_USE_CF_TURB_SCHULTZ_GRUNOW_HIGH_RE", DO_NOT_USE_CF_TURB_SCHULTZ_GRUNOW_HIGH_RE );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "CF_TURB_EQN", "CF_TURB_SCHULTZ_GRUNOW_SCHOENHERR", CF_TURB_SCHULTZ_GRUNOW_SCHOENHERR );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "CF_TURB_EQN", "DO_NOT_USE_CF_TURB_WHITE_CHRISTOPH_COMPRESSIBLE", DO_NOT_USE_CF_TURB_WHITE_CHRISTOPH_COMPRESSIBLE );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "CF_TURB_EQN", "CF_TURB_ROUGHNESS_SCHLICHTING_AVG", CF_TURB_ROUGHNESS_SCHLICHTING_AVG );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "CF_TURB_EQN", "DO_NOT_USE_CF_TURB_ROUGHNESS_SCHLICHTING_LOCAL", DO_NOT_USE_CF_TURB_ROUGHNESS_SCHLICHTING_LOCAL );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "CF_TURB_EQN", "DO_NOT_USE_CF_TURB_ROUGHNESS_WHITE", DO_NOT_USE_CF_TURB_ROUGHNESS_WHITE );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "CF_TURB_EQN", "CF_TURB_ROUGHNESS_SCHLICHTING_AVG_FLOW_CORRECTION", CF_TURB_ROUGHNESS_SCHLICHTING_AVG_FLOW_CORRECTION );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "CF_TURB_EQN", "CF_TURB_HEATTRANSFER_WHITE_CHRISTOPH", CF_TURB_HEATTRANSFER_WHITE_CHRISTOPH );
    assert( r >= 0 );


    r = se->RegisterEnum( "CHEVRON_TYPE" );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "CHEVRON_TYPE", "CHEVRON_NONE", CHEVRON_NONE );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "CHEVRON_TYPE", "CHEVRON_PARTIAL", CHEVRON_PARTIAL );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "CHEVRON_TYPE", "CHEVRON_FULL", CHEVRON_FULL );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "CHEVRON_TYPE", "CHEVRON_NUM_TYPES", CHEVRON_NUM_TYPES );
    assert( r >= 0 );


    r = se->RegisterEnum( "CHEVRON_W01_MODES" );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "CHEVRON_W01_MODES", "CHEVRON_W01_SE", CHEVRON_W01_SE );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "CHEVRON_W01_MODES", "CHEVRON_W01_CW", CHEVRON_W01_CW );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "CHEVRON_W01_MODES", "CHEVRON_W01_NUM_MODES", CHEVRON_W01_NUM_MODES );
    assert( r >= 0 );


    r = se->RegisterEnum( "COLLISION_ERRORS" );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "COLLISION_ERRORS", "COLLISION_OK", COLLISION_OK );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "COLLISION_ERRORS", "COLLISION_INTERSECT_NO_SOLUTION", COLLISION_INTERSECT_NO_SOLUTION );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "COLLISION_ERRORS", "COLLISION_CLEAR_NO_SOLUTION", COLLISION_CLEAR_NO_SOLUTION );
    assert( r >= 0 );


    r = se->RegisterEnum( "COMPUTATION_FILE_TYPE" );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "COMPUTATION_FILE_TYPE", "NO_FILE_TYPE", NO_FILE_TYPE );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "COMPUTATION_FILE_TYPE", "COMP_GEOM_TXT_TYPE", COMP_GEOM_TXT_TYPE );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "COMPUTATION_FILE_TYPE", "COMP_GEOM_CSV_TYPE", COMP_GEOM_CSV_TYPE );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "COMPUTATION_FILE_TYPE", "SLICE_TXT_TYPE", SLICE_TXT_TYPE );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "COMPUTATION_FILE_TYPE", "MASS_PROP_TXT_TYPE", MASS_PROP_TXT_TYPE );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "COMPUTATION_FILE_TYPE", "DEGEN_GEOM_CSV_TYPE", DEGEN_GEOM_CSV_TYPE );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "COMPUTATION_FILE_TYPE", "DEGEN_GEOM_M_TYPE", DEGEN_GEOM_M_TYPE );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "COMPUTATION_FILE_TYPE", "CFD_STL_TYPE", CFD_STL_TYPE );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "COMPUTATION_FILE_TYPE", "CFD_POLY_TYPE", CFD_POLY_TYPE );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "COMPUTATION_FILE_TYPE", "CFD_TRI_TYPE", CFD_TRI_TYPE );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "COMPUTATION_FILE_TYPE", "CFD_OBJ_TYPE", CFD_OBJ_TYPE );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "COMPUTATION_FILE_TYPE", "CFD_DAT_TYPE", CFD_DAT_TYPE );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "COMPUTATION_FILE_TYPE", "CFD_KEY_TYPE", CFD_KEY_TYPE );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "COMPUTATION_FILE_TYPE", "CFD_GMSH_TYPE", CFD_GMSH_TYPE );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "COMPUTATION_FILE_TYPE", "CFD_TKEY_TYPE", CFD_TKEY_TYPE );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "COMPUTATION_FILE_TYPE", "PROJ_AREA_CSV_TYPE", PROJ_AREA_CSV_TYPE );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "COMPUTATION_FILE_TYPE", "WAVE_DRAG_TXT_TYPE", WAVE_DRAG_TXT_TYPE );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "COMPUTATION_FILE_TYPE", "VSPAERO_PANEL_TRI_TYPE", VSPAERO_PANEL_TRI_TYPE );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "COMPUTATION_FILE_TYPE", "DRAG_BUILD_CSV_TYPE", DRAG_BUILD_CSV_TYPE );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "COMPUTATION_FILE_TYPE", "CFD_FACET_TYPE", CFD_FACET_TYPE );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "COMPUTATION_FILE_TYPE", "CFD_VSPGEOM_TYPE", CFD_VSPGEOM_TYPE );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "COMPUTATION_FILE_TYPE", "VSPAERO_VSPGEOM_TYPE", VSPAERO_VSPGEOM_TYPE );
    assert( r >= 0 );


    r = se->RegisterEnum( "CONFORMAL_TRIM_TYPE" );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "CONFORMAL_TRIM_TYPE", "U_TRIM", U_TRIM );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "CONFORMAL_TRIM_TYPE", "L_TRIM", L_TRIM );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "CONFORMAL_TRIM_TYPE", "ETA_TRIM", ETA_TRIM );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "CONFORMAL_TRIM_TYPE", "NUM_TRIM_TYPES", NUM_TRIM_TYPES );
    assert( r >= 0 );


    r = se->RegisterEnum( "DELIM_TYPE" );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "DELIM_TYPE", "DELIM_COMMA", DELIM_COMMA );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "DELIM_TYPE", "DELIM_USCORE", DELIM_USCORE );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "DELIM_TYPE", "DELIM_SPACE", DELIM_SPACE );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "DELIM_TYPE", "DELIM_NONE", DELIM_NONE );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "DELIM_TYPE", "DELIM_NUM_TYPES", DELIM_NUM_TYPES );
    assert( r >= 0 );


    r = se->RegisterEnum( "DIMENSION_SET" );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "DIMENSION_SET", "SET_3D", SET_3D );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "DIMENSION_SET", "SET_2D", SET_2D );
    assert( r >= 0 );


    r = se->RegisterEnum( "DIR_INDEX" );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "DIR_INDEX", "X_DIR", X_DIR );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "DIR_INDEX", "Y_DIR", Y_DIR );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "DIR_INDEX", "Z_DIR", Z_DIR );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "DIR_INDEX", "ALL_DIR", ALL_DIR );
    assert( r >= 0 );


    r = se->RegisterEnum( "DISPLAY_TYPE" );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "DISPLAY_TYPE", "DISPLAY_BEZIER", DISPLAY_BEZIER );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "DISPLAY_TYPE", "DISPLAY_DEGEN_SURF", DISPLAY_DEGEN_SURF );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "DISPLAY_TYPE", "DISPLAY_DEGEN_PLATE", DISPLAY_DEGEN_PLATE );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "DISPLAY_TYPE", "DISPLAY_DEGEN_CAMBER", DISPLAY_DEGEN_CAMBER );
    assert( r >= 0 );


    r = se->RegisterEnum( "DRAW_TYPE" );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "DRAW_TYPE", "GEOM_DRAW_WIRE", GEOM_DRAW_WIRE );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "DRAW_TYPE", "GEOM_DRAW_HIDDEN", GEOM_DRAW_HIDDEN );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "DRAW_TYPE", "GEOM_DRAW_SHADE", GEOM_DRAW_SHADE );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "DRAW_TYPE", "GEOM_DRAW_TEXTURE", GEOM_DRAW_TEXTURE );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "DRAW_TYPE", "GEOM_DRAW_NONE", GEOM_DRAW_NONE );
    assert( r >= 0 );


    r = se->RegisterEnum( "ERROR_CODE" );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "ERROR_CODE", "VSP_OK", vsp::VSP_OK );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "ERROR_CODE", "VSP_INVALID_PTR", vsp::VSP_INVALID_PTR );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "ERROR_CODE", "VSP_INVALID_TYPE", vsp::VSP_INVALID_TYPE );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "ERROR_CODE", "VSP_CANT_FIND_TYPE", vsp::VSP_CANT_FIND_TYPE );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "ERROR_CODE", "VSP_CANT_FIND_PARM", vsp::VSP_CANT_FIND_PARM );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "ERROR_CODE", "VSP_CANT_FIND_NAME", vsp::VSP_CANT_FIND_NAME );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "ERROR_CODE", "VSP_INVALID_GEOM_ID", vsp::VSP_INVALID_GEOM_ID );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "ERROR_CODE", "VSP_FILE_DOES_NOT_EXIST", vsp::VSP_FILE_DOES_NOT_EXIST );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "ERROR_CODE", "VSP_FILE_WRITE_FAILURE", vsp::VSP_FILE_WRITE_FAILURE );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "ERROR_CODE", "VSP_FILE_READ_FAILURE", vsp::VSP_FILE_READ_FAILURE );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "ERROR_CODE", "VSP_WRONG_GEOM_TYPE", vsp::VSP_WRONG_GEOM_TYPE );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "ERROR_CODE", "VSP_WRONG_XSEC_TYPE", vsp::VSP_WRONG_XSEC_TYPE );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "ERROR_CODE", "VSP_WRONG_FILE_TYPE", vsp::VSP_WRONG_FILE_TYPE );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "ERROR_CODE", "VSP_INDEX_OUT_RANGE", vsp::VSP_INDEX_OUT_RANGE );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "ERROR_CODE", "VSP_INVALID_XSEC_ID", vsp::VSP_INVALID_XSEC_ID );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "ERROR_CODE", "VSP_INVALID_ID", vsp::VSP_INVALID_ID );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "ERROR_CODE", "VSP_CANT_SET_NOT_EQ_PARM", vsp::VSP_CANT_SET_NOT_EQ_PARM );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "ERROR_CODE", "VSP_AMBIGUOUS_SUBSURF", vsp::VSP_AMBIGUOUS_SUBSURF );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "ERROR_CODE", "VSP_INVALID_VARPRESET_SETNAME", vsp::VSP_INVALID_VARPRESET_SETNAME );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "ERROR_CODE", "VSP_INVALID_VARPRESET_GROUPNAME", vsp::VSP_INVALID_VARPRESET_GROUPNAME );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "ERROR_CODE", "VSP_CONFORMAL_PARENT_UNSUPPORTED", vsp::VSP_CONFORMAL_PARENT_UNSUPPORTED );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "ERROR_CODE", "VSP_UNEXPECTED_RESET_REMAP_ID", vsp::VSP_UNEXPECTED_RESET_REMAP_ID );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "ERROR_CODE", "VSP_INVALID_INPUT_VAL", vsp::VSP_INVALID_INPUT_VAL );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "ERROR_CODE", "VSP_INVALID_CF_EQN", vsp::VSP_INVALID_CF_EQN );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "ERROR_CODE", "VSP_INVALID_DRIVERS", vsp::VSP_INVALID_DRIVERS );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "ERROR_CODE", "VSP_ADV_LINK_BUILD_FAIL", vsp::VSP_ADV_LINK_BUILD_FAIL );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "ERROR_CODE", "VSP_DEPRECATED", vsp::VSP_DEPRECATED );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "ERROR_CODE", "VSP_LINK_LOOP_DETECTED", vsp::VSP_LINK_LOOP_DETECTED );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "ERROR_CODE", "VSP_LINK_OUTPUT_NOT_ASSIGNED", vsp::VSP_LINK_OUTPUT_NOT_ASSIGNED );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "ERROR_CODE", "VSP_DUPLICATE_NAME", vsp::VSP_DUPLICATE_NAME );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "ERROR_CODE", "VSP_GUI_DEVICE_DEACTIVATED", vsp::VSP_GUI_DEVICE_DEACTIVATED );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "ERROR_CODE", "VSP_NUM_ERROR_CODE", vsp::VSP_NUM_ERROR_CODE );
    assert( r >= 0 );


    r = se->RegisterEnum( "EXCRES_TYPE" );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "EXCRES_TYPE", "EXCRESCENCE_COUNT", EXCRESCENCE_COUNT );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "EXCRES_TYPE", "EXCRESCENCE_CD", EXCRESCENCE_CD );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "EXCRES_TYPE", "EXCRESCENCE_PERCENT_GEOM", EXCRESCENCE_PERCENT_GEOM );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "EXCRES_TYPE", "EXCRESCENCE_MARGIN", EXCRESCENCE_MARGIN );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "EXCRES_TYPE", "EXCRESCENCE_DRAGAREA", EXCRESCENCE_DRAGAREA );
    assert( r >= 0 );


    r = se->RegisterEnum( "EXPORT_TYPE" );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "EXPORT_TYPE", "EXPORT_FELISA", EXPORT_FELISA ); // TODO: Remove or implement
    assert( r >= 0 );
    r = se->RegisterEnumValue( "EXPORT_TYPE", "EXPORT_XSEC", EXPORT_XSEC );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "EXPORT_TYPE", "EXPORT_STL", EXPORT_STL );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "EXPORT_TYPE", "EXPORT_AWAVE", EXPORT_AWAVE ); // TODO: Remove or implement
    assert( r >= 0 );
    r = se->RegisterEnumValue( "EXPORT_TYPE", "EXPORT_NASCART", EXPORT_NASCART );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "EXPORT_TYPE", "EXPORT_POVRAY", EXPORT_POVRAY );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "EXPORT_TYPE", "EXPORT_CART3D", EXPORT_CART3D );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "EXPORT_TYPE", "EXPORT_VSPGEOM", EXPORT_VSPGEOM );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "EXPORT_TYPE", "EXPORT_VORXSEC", EXPORT_VORXSEC ); // TODO: Remove or implement
    assert( r >= 0 );
    r = se->RegisterEnumValue( "EXPORT_TYPE", "EXPORT_XSECGEOM", EXPORT_XSECGEOM ); // TODO: Remove or implement
    assert( r >= 0 );
    r = se->RegisterEnumValue( "EXPORT_TYPE", "EXPORT_GMSH", EXPORT_GMSH );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "EXPORT_TYPE", "EXPORT_X3D", EXPORT_X3D );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "EXPORT_TYPE", "EXPORT_STEP", EXPORT_STEP );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "EXPORT_TYPE", "EXPORT_PLOT3D", EXPORT_PLOT3D );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "EXPORT_TYPE", "EXPORT_IGES", EXPORT_IGES );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "EXPORT_TYPE", "EXPORT_BEM", EXPORT_BEM );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "EXPORT_TYPE", "EXPORT_DXF", EXPORT_DXF );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "EXPORT_TYPE", "EXPORT_FACET", EXPORT_FACET );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "EXPORT_TYPE", "EXPORT_SVG", EXPORT_SVG );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "EXPORT_TYPE", "EXPORT_PMARC", EXPORT_PMARC );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "EXPORT_TYPE", "EXPORT_OBJ", EXPORT_OBJ );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "EXPORT_TYPE", "EXPORT_SELIG_AIRFOIL", EXPORT_SELIG_AIRFOIL );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "EXPORT_TYPE", "EXPORT_BEZIER_AIRFOIL", EXPORT_BEZIER_AIRFOIL );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "EXPORT_TYPE", "EXPORT_IGES_STRUCTURE", EXPORT_IGES_STRUCTURE );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "EXPORT_TYPE", "EXPORT_STEP_STRUCTURE", EXPORT_STEP_STRUCTURE );
    assert( r >= 0 );

    r = se->RegisterEnum( "FEA_BC_MODE" );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "FEA_BC_MODE", "FEA_BCM_USER", FEA_BCM_USER );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "FEA_BC_MODE", "FEA_BCM_ALL", FEA_BCM_ALL );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "FEA_BC_MODE", "FEA_BCM_PIN", FEA_BCM_PIN );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "FEA_BC_MODE", "FEA_BCM_SYMM", FEA_BCM_SYMM );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "FEA_BC_MODE", "FEA_BCM_ASYMM", FEA_BCM_ASYMM );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "FEA_BC_MODE", "FEA_NUM_BCM_MODES", FEA_NUM_BCM_MODES );
    assert( r >= 0 );

    r = se->RegisterEnum( "FEA_BC_TYPE" );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "FEA_BC_TYPE", "FEA_BC_STRUCTURE", FEA_BC_STRUCTURE );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "FEA_BC_TYPE", "FEA_BC_PART", FEA_BC_PART );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "FEA_BC_TYPE", "FEA_BC_SUBSURF", FEA_BC_SUBSURF );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "FEA_BC_TYPE", "FEA_NUM_BC_TYPES", FEA_NUM_BC_TYPES );
    assert( r >= 0 );

    r = se->RegisterEnum( "FEA_CROSS_SECT_TYPE" );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "FEA_CROSS_SECT_TYPE", "FEA_XSEC_GENERAL", FEA_XSEC_GENERAL );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "FEA_CROSS_SECT_TYPE", "FEA_XSEC_CIRC", FEA_XSEC_CIRC );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "FEA_CROSS_SECT_TYPE", "FEA_XSEC_PIPE", FEA_XSEC_PIPE );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "FEA_CROSS_SECT_TYPE", "FEA_XSEC_I", FEA_XSEC_I );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "FEA_CROSS_SECT_TYPE", "FEA_XSEC_RECT", FEA_XSEC_RECT );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "FEA_CROSS_SECT_TYPE", "FEA_XSEC_BOX", FEA_XSEC_BOX );
    assert( r >= 0 );


    r = se->RegisterEnum( "FEA_EXPORT_TYPE" );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "FEA_EXPORT_TYPE", "FEA_MASS_FILE_NAME", FEA_MASS_FILE_NAME );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "FEA_EXPORT_TYPE", "FEA_NASTRAN_FILE_NAME", FEA_NASTRAN_FILE_NAME );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "FEA_EXPORT_TYPE", "FEA_NKEY_FILE_NAME", FEA_NKEY_FILE_NAME );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "FEA_EXPORT_TYPE", "FEA_CALCULIX_FILE_NAME", FEA_CALCULIX_FILE_NAME );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "FEA_EXPORT_TYPE", "FEA_STL_FILE_NAME", FEA_STL_FILE_NAME );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "FEA_EXPORT_TYPE", "FEA_GMSH_FILE_NAME", FEA_GMSH_FILE_NAME );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "FEA_EXPORT_TYPE", "FEA_SRF_FILE_NAME", FEA_SRF_FILE_NAME );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "FEA_EXPORT_TYPE", "FEA_CURV_FILE_NAME", FEA_CURV_FILE_NAME );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "FEA_EXPORT_TYPE", "FEA_PLOT3D_FILE_NAME", FEA_PLOT3D_FILE_NAME );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "FEA_EXPORT_TYPE", "FEA_IGES_FILE_NAME", FEA_IGES_FILE_NAME );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "FEA_EXPORT_TYPE", "FEA_STEP_FILE_NAME", FEA_STEP_FILE_NAME );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "FEA_EXPORT_TYPE", "FEA_NUM_FILE_NAMES", FEA_NUM_FILE_NAMES );
    assert( r >= 0 );


    r = se->RegisterEnum( "FEA_FIX_PT_TYPE" );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "FEA_FIX_PT_TYPE", "FEA_FIX_PT_ON_BODY", FEA_FIX_PT_ON_BODY );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "FEA_FIX_PT_TYPE", "FEA_FIX_PT_GLOBAL_XYZ", FEA_FIX_PT_GLOBAL_XYZ );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "FEA_FIX_PT_TYPE", "FEA_FIX_PT_DELTA_XYZ", FEA_FIX_PT_DELTA_XYZ );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "FEA_FIX_PT_TYPE", "FEA_FIX_PT_DELTA_UVN", FEA_FIX_PT_DELTA_UVN );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "FEA_FIX_PT_TYPE", "FEA_NUM_FIX_PT_TYPES", FEA_NUM_FIX_PT_TYPES );
    assert( r >= 0 );


    r = se->RegisterEnum( "FEA_MATERIAL_TYPE" );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "FEA_MATERIAL_TYPE", "FEA_ISOTROPIC", FEA_ISOTROPIC );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "FEA_MATERIAL_TYPE", "FEA_ENG_ORTHO", FEA_ENG_ORTHO );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "FEA_MATERIAL_TYPE", "FEA_ENG_ORTHO_TRANS_ISO", FEA_ENG_ORTHO_TRANS_ISO );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "FEA_MATERIAL_TYPE", "FEA_LAMINATE", FEA_LAMINATE );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "FEA_MATERIAL_TYPE", "FEA_NUM_MAT_TYPES", FEA_NUM_MAT_TYPES );
    assert( r >= 0 );


    r = se->RegisterEnum( "FEA_ORIENTATION_TYPE" );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "FEA_ORIENTATION_TYPE", "FEA_ORIENT_GLOBAL_X", FEA_ORIENT_GLOBAL_X );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "FEA_ORIENTATION_TYPE", "FEA_ORIENT_GLOBAL_Y", FEA_ORIENT_GLOBAL_Y );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "FEA_ORIENTATION_TYPE", "FEA_ORIENT_GLOBAL_Z", FEA_ORIENT_GLOBAL_Z );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "FEA_ORIENTATION_TYPE", "FEA_ORIENT_COMP_X", FEA_ORIENT_COMP_X );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "FEA_ORIENTATION_TYPE", "FEA_ORIENT_COMP_Y", FEA_ORIENT_COMP_Y );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "FEA_ORIENTATION_TYPE", "FEA_ORIENT_COMP_Z", FEA_ORIENT_COMP_Z );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "FEA_ORIENTATION_TYPE", "FEA_ORIENT_PART_U", FEA_ORIENT_PART_U );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "FEA_ORIENTATION_TYPE", "FEA_ORIENT_PART_V", FEA_ORIENT_PART_V );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "FEA_ORIENTATION_TYPE", "FEA_ORIENT_OML_U", FEA_ORIENT_OML_U );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "FEA_ORIENTATION_TYPE", "FEA_ORIENT_OML_V", FEA_ORIENT_OML_V );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "FEA_ORIENTATION_TYPE", "FEA_ORIENT_OML_R", FEA_ORIENT_OML_R );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "FEA_ORIENTATION_TYPE", "FEA_ORIENT_OML_S", FEA_ORIENT_OML_S );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "FEA_ORIENTATION_TYPE", "FEA_ORIENT_OML_T", FEA_ORIENT_OML_T );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "FEA_ORIENTATION_TYPE", "FEA_NUM_ORIENT_TYPES", FEA_NUM_ORIENT_TYPES );
    assert( r >= 0 );


    r = se->RegisterEnum( "FEA_PART_ELEMENT_TYPE" );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "FEA_PART_ELEMENT_TYPE", "FEA_DEPRECATED", FEA_DEPRECATED );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "FEA_PART_ELEMENT_TYPE", "FEA_SHELL", FEA_SHELL );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "FEA_PART_ELEMENT_TYPE", "FEA_BEAM", FEA_BEAM );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "FEA_PART_ELEMENT_TYPE", "FEA_SHELL_AND_BEAM", FEA_SHELL_AND_BEAM );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "FEA_PART_ELEMENT_TYPE", "FEA_NO_ELEMENTS", FEA_NO_ELEMENTS );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "FEA_PART_ELEMENT_TYPE", "FEA_NUM_ELEMENT_TYPES", FEA_NUM_ELEMENT_TYPES );
    assert( r >= 0 );


    r = se->RegisterEnum( "FEA_PART_TYPE" );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "FEA_PART_TYPE", "FEA_SLICE", FEA_SLICE );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "FEA_PART_TYPE", "FEA_RIB", FEA_RIB );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "FEA_PART_TYPE", "FEA_SPAR", FEA_SPAR );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "FEA_PART_TYPE", "FEA_FIX_POINT", FEA_FIX_POINT );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "FEA_PART_TYPE", "FEA_DOME", FEA_DOME );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "FEA_PART_TYPE", "FEA_RIB_ARRAY", FEA_RIB_ARRAY );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "FEA_PART_TYPE", "FEA_SLICE_ARRAY", FEA_SLICE_ARRAY );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "FEA_PART_TYPE", "FEA_TRIM", FEA_TRIM );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "FEA_PART_TYPE", "FEA_SKIN", FEA_SKIN );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "FEA_PART_TYPE", "FEA_POLY_SPAR", FEA_POLY_SPAR );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "FEA_PART_TYPE", "FEA_NUM_TYPES", FEA_NUM_TYPES );
    assert( r >= 0 );


    r = se->RegisterEnum( "FEA_RIB_NORMAL" );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "FEA_RIB_NORMAL", "NO_NORMAL", NO_NORMAL );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "FEA_RIB_NORMAL", "LE_NORMAL", LE_NORMAL );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "FEA_RIB_NORMAL", "TE_NORMAL", TE_NORMAL );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "FEA_RIB_NORMAL", "SPAR_NORMAL", SPAR_NORMAL );
    assert( r >= 0 );


    r = se->RegisterEnum( "FEA_SHELL_TREATMENT_TYPE" );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "FEA_SHELL_TREATMENT_TYPE", "FEA_KEEP", FEA_KEEP );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "FEA_SHELL_TREATMENT_TYPE", "FEA_DELETE", FEA_DELETE );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "FEA_SHELL_TREATMENT_TYPE", "FEA_NUM_SHELL_TREATMENT_TYPES", FEA_NUM_SHELL_TREATMENT_TYPES );
    assert( r >= 0 );


    r = se->RegisterEnum( "FEA_SLICE_TYPE" );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "FEA_SLICE_TYPE", "XY_BODY", XY_BODY );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "FEA_SLICE_TYPE", "YZ_BODY", YZ_BODY );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "FEA_SLICE_TYPE", "XZ_BODY", XZ_BODY );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "FEA_SLICE_TYPE", "XY_ABS", XY_ABS );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "FEA_SLICE_TYPE", "YZ_ABS", YZ_ABS );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "FEA_SLICE_TYPE", "XZ_ABS", XZ_ABS );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "FEA_SLICE_TYPE", "SPINE_NORMAL", SPINE_NORMAL );
    assert( r >= 0 );

    r = se->RegisterEnum( "FEA_POLY_SPAR_POINT" );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "FEA_POLY_SPAR_POINT", "POLY_SPAR_POINT_U01", POLY_SPAR_POINT_U01 );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "FEA_POLY_SPAR_POINT", "POLY_SPAR_POINT_U0N", POLY_SPAR_POINT_U0N );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "FEA_POLY_SPAR_POINT", "POLY_SPAR_POINT_ETA", POLY_SPAR_POINT_ETA );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "FEA_POLY_SPAR_POINT", "NUM_POLY_SPAR_POINT_TYPES", NUM_POLY_SPAR_POINT_TYPES );
    assert( r >= 0 );


    r = se->RegisterEnum( "FEA_UNIT_TYPE" );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "FEA_UNIT_TYPE", "SI_UNIT", SI_UNIT );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "FEA_UNIT_TYPE", "CGS_UNIT", CGS_UNIT );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "FEA_UNIT_TYPE", "MPA_UNIT", MPA_UNIT );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "FEA_UNIT_TYPE", "BFT_UNIT", BFT_UNIT );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "FEA_UNIT_TYPE", "BIN_UNIT", BIN_UNIT );
    assert( r >= 0 );


    r = se->RegisterEnum( "FF_B_EQN" );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "FF_B_EQN", "FF_B_MANUAL", FF_B_MANUAL );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "FF_B_EQN", "FF_B_SCHEMENSKY_BODY", FF_B_SCHEMENSKY_FUSE );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "FF_B_EQN", "FF_B_SCHEMENSKY_NACELLE", FF_B_SCHEMENSKY_NACELLE );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "FF_B_EQN", "FF_B_HOERNER_STREAMBODY", FF_B_HOERNER_STREAMBODY );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "FF_B_EQN", "FF_B_TORENBEEK", FF_B_TORENBEEK );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "FF_B_EQN", "FF_B_SHEVELL", FF_B_SHEVELL );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "FF_B_EQN", "FF_B_COVERT", FF_B_COVERT );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "FF_B_EQN", "FF_B_JENKINSON_FUSE", FF_B_JENKINSON_FUSE );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "FF_B_EQN", "FF_B_JENKINSON_WING_NACELLE", FF_B_JENKINSON_WING_NACELLE );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "FF_B_EQN", "FF_B_JENKINSON_AFT_FUSE_NACELLE", FF_B_JENKINSON_AFT_FUSE_NACELLE );
    assert( r >= 0 );


    r = se->RegisterEnum( "FF_W_EQN" );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "FF_W_EQN", "FF_W_MANUAL", FF_W_MANUAL );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "FF_W_EQN", "FF_W_EDET_CONV", FF_W_EDET_CONV );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "FF_W_EQN", "FF_W_EDET_ADV", FF_W_EDET_ADV );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "FF_W_EQN", "FF_W_HOERNER", FF_W_HOERNER );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "FF_W_EQN", "FF_W_COVERT", FF_W_COVERT );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "FF_W_EQN", "FF_W_WILLIAMS", FF_W_SHEVELL );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "FF_W_EQN", "FF_W_KROO", FF_W_KROO );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "FF_W_EQN", "FF_W_TORENBEEK", FF_W_TORENBEEK );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "FF_W_EQN", "FF_W_DATCOM", FF_W_DATCOM );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "FF_W_EQN", "FF_W_SCHEMENSKY_6_SERIES_AF", FF_W_SCHEMENSKY_6_SERIES_AF );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "FF_W_EQN", "FF_W_SCHEMENSKY_4_SERIES_AF", FF_W_SCHEMENSKY_4_SERIES_AF );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "FF_W_EQN", "FF_W_JENKINSON_WING", FF_W_JENKINSON_WING );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "FF_W_EQN", "FF_W_JENKINSON_TAIL", FF_W_JENKINSON_TAIL );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "FF_W_EQN", "FF_W_SCHEMENSKY_SUPERCRITICAL_AF", FF_W_SCHEMENSKY_SUPERCRITICAL_AF );
    assert( r >= 0 );


    r = se->RegisterEnum( "FREESTREAM_PD_UNITS" );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "FREESTREAM_PD_UNITS", "PD_UNITS_IMPERIAL", PD_UNITS_IMPERIAL );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "FREESTREAM_PD_UNITS", "PD_UNITS_METRIC", PD_UNITS_METRIC );
    assert( r >= 0 );


    r = se->RegisterEnum( "FILE_CHOOSER_MODE" );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "FILE_CHOOSER_MODE", "OPEN", OPEN );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "FILE_CHOOSER_MODE", "SAVE", SAVE );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "FILE_CHOOSER_MODE", "NUM_FILE_CHOOSER_MODES", NUM_FILE_CHOOSER_MODES );
    assert( r >= 0 );


    r = se->RegisterEnum( "FILE_CHOOSER_TYPE" );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "FILE_CHOOSER_TYPE", "FC_OPENVSP", FC_OPENVSP );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "FILE_CHOOSER_TYPE", "FC_NATIVE", FC_NATIVE );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "FILE_CHOOSER_TYPE", "NUM_FILE_CHOOSER_TYPES", NUM_FILE_CHOOSER_TYPES );
    assert( r >= 0 );


    r = se->RegisterEnum( "GDEV" );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "GDEV", "GDEV_TAB", GDEV_TAB );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "GDEV", "GDEV_SCROLL_TAB", GDEV_SCROLL_TAB );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "GDEV", "GDEV_GROUP", GDEV_GROUP );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "GDEV", "GDEV_PARM_BUTTON", GDEV_PARM_BUTTON );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "GDEV", "GDEV_INPUT", GDEV_INPUT );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "GDEV", "GDEV_OUTPUT", GDEV_OUTPUT );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "GDEV", "GDEV_SLIDER", GDEV_SLIDER );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "GDEV", "GDEV_SLIDER_ADJ_RANGE", GDEV_SLIDER_ADJ_RANGE );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "GDEV", "GDEV_CHECK_BUTTON", GDEV_CHECK_BUTTON );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "GDEV", "GDEV_RADIO_BUTTON", GDEV_RADIO_BUTTON );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "GDEV", "GDEV_TOGGLE_BUTTON", GDEV_TOGGLE_BUTTON );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "GDEV", "GDEV_TOGGLE_RADIO_GROUP", GDEV_TOGGLE_RADIO_GROUP ); // TODO: Implement or remove
    assert( r >= 0 );
    r = se->RegisterEnumValue( "GDEV", "GDEV_TRIGGER_BUTTON", GDEV_TRIGGER_BUTTON );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "GDEV", "GDEV_COUNTER", GDEV_COUNTER );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "GDEV", "GDEV_CHOICE", GDEV_CHOICE );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "GDEV", "GDEV_ADD_CHOICE_ITEM", GDEV_ADD_CHOICE_ITEM );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "GDEV", "GDEV_SLIDER_INPUT", GDEV_SLIDER_INPUT );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "GDEV", "GDEV_SLIDER_ADJ_RANGE_INPUT", GDEV_SLIDER_ADJ_RANGE_INPUT );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "GDEV", "GDEV_SLIDER_ADJ_RANGE_TWO_INPUT", GDEV_SLIDER_ADJ_RANGE_TWO_INPUT ); // TODO: Implement or remove
    assert( r >= 0 );
    r = se->RegisterEnumValue( "GDEV", "GDEV_FRACT_PARM_SLIDER", GDEV_FRACT_PARM_SLIDER );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "GDEV", "GDEV_STRING_INPUT", GDEV_STRING_INPUT );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "GDEV", "GDEV_INDEX_SELECTOR", GDEV_INDEX_SELECTOR );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "GDEV", "GDEV_COLOR_PICKER", GDEV_COLOR_PICKER );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "GDEV", "GDEV_YGAP", GDEV_YGAP );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "GDEV", "GDEV_DIVIDER_BOX", GDEV_DIVIDER_BOX );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "GDEV", "GDEV_BEGIN_SAME_LINE", GDEV_BEGIN_SAME_LINE );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "GDEV", "GDEV_END_SAME_LINE", GDEV_END_SAME_LINE );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "GDEV", "GDEV_FORCE_WIDTH", GDEV_FORCE_WIDTH );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "GDEV", "GDEV_SET_FORMAT", GDEV_SET_FORMAT );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "GDEV", "NUM_GDEV_TYPES", NUM_GDEV_TYPES );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "GDEV", "ALL_GDEV_TYPES", ALL_GDEV_TYPES );
    assert( r >= 0 );


    r = se->RegisterEnum( "GENDER" );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "GENDER", "MALE", MALE );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "GENDER", "FEMALE", FEMALE );
    assert( r >= 0 );


    r = se->RegisterEnum( "GEOMETRY_ANALYSIS_TYPE" );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "GEOMETRY_ANALYSIS_TYPE", "EXTERNAL_INTERFERENCE", EXTERNAL_INTERFERENCE );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "GEOMETRY_ANALYSIS_TYPE", "PACKAGING_INTERFERENCE", PACKAGING_INTERFERENCE );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "GEOMETRY_ANALYSIS_TYPE", "EXTERNAL_SELF_INTERFERENCE", EXTERNAL_SELF_INTERFERENCE );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "GEOMETRY_ANALYSIS_TYPE", "PLANE_STATIC_DISTANCE_INTERFERENCE", PLANE_STATIC_DISTANCE_INTERFERENCE );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "GEOMETRY_ANALYSIS_TYPE", "PLANE_2PT_ANGLE_INTERFERENCE", PLANE_2PT_ANGLE_INTERFERENCE );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "GEOMETRY_ANALYSIS_TYPE", "GEAR_CG_TIPBACK_ANALYSIS", GEAR_CG_TIPBACK_ANALYSIS );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "GEOMETRY_ANALYSIS_TYPE", "PLANE_1PT_ANGLE_INTERFERENCE", PLANE_1PT_ANGLE_INTERFERENCE );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "GEOMETRY_ANALYSIS_TYPE", "GEAR_WEIGHT_DISTRIBUTION_ANALYSIS", GEAR_WEIGHT_DISTRIBUTION_ANALYSIS );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "GEOMETRY_ANALYSIS_TYPE", "GEAR_TIPOVER_ANALYSIS", GEAR_TIPOVER_ANALYSIS );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "GEOMETRY_ANALYSIS_TYPE", "GEAR_TURN_ANALYSIS", GEAR_TURN_ANALYSIS );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "GEOMETRY_ANALYSIS_TYPE", "VISIBLE_FROM_POINT_ANALYSIS", VISIBLE_FROM_POINT_ANALYSIS );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "GEOMETRY_ANALYSIS_TYPE", "CCE_INTERFERENCE", CCE_INTERFERENCE );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "GEOMETRY_ANALYSIS_TYPE", "NUM_INTERFERENCE_TYPES", NUM_INTERFERENCE_TYPES );
    assert( r >= 0 );

    r = se->RegisterEnum( "GUI_GEOM_SCREEN" );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "GUI_GEOM_SCREEN", "POD_GEOM_SCREEN", POD_GEOM_SCREEN );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "GUI_GEOM_SCREEN", "FUSELAGE_GEOM_SCREEN", FUSELAGE_GEOM_SCREEN );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "GUI_GEOM_SCREEN", "MS_WING_GEOM_SCREEN", MS_WING_GEOM_SCREEN );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "GUI_GEOM_SCREEN", "BLANK_GEOM_SCREEN", BLANK_GEOM_SCREEN );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "GUI_GEOM_SCREEN", "MESH_GEOM_SCREEN", MESH_GEOM_SCREEN );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "GUI_GEOM_SCREEN", "STACK_GEOM_SCREEN", STACK_GEOM_SCREEN );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "GUI_GEOM_SCREEN", "CUSTOM_GEOM_SCREEN", CUSTOM_GEOM_SCREEN );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "GUI_GEOM_SCREEN", "PT_CLOUD_GEOM_SCREEN", PT_CLOUD_GEOM_SCREEN );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "GUI_GEOM_SCREEN", "PROP_GEOM_SCREEN", PROP_GEOM_SCREEN );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "GUI_GEOM_SCREEN", "HINGE_GEOM_SCREEN", HINGE_GEOM_SCREEN );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "GUI_GEOM_SCREEN", "MULT_GEOM_SCREEN", MULT_GEOM_SCREEN );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "GUI_GEOM_SCREEN", "CONFORMAL_SCREEN", CONFORMAL_SCREEN );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "GUI_GEOM_SCREEN", "ELLIPSOID_GEOM_SCREEN", ELLIPSOID_GEOM_SCREEN );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "GUI_GEOM_SCREEN", "BOR_GEOM_SCREEN", BOR_GEOM_SCREEN );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "GUI_GEOM_SCREEN", "WIRE_FRAME_GEOM_SCREEN", WIRE_FRAME_GEOM_SCREEN );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "GUI_GEOM_SCREEN", "HUMAN_GEOM_SCREEN", HUMAN_GEOM_SCREEN );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "GUI_GEOM_SCREEN", "ROUTING_GEOM_SCREEN", ROUTING_GEOM_SCREEN );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "GUI_GEOM_SCREEN", "AUXILIARY_GEOM_SCREEN", AUXILIARY_GEOM_SCREEN );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "GUI_GEOM_SCREEN", "NUM_GEOM_SCREENS", NUM_GEOM_SCREENS );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "GUI_GEOM_SCREEN", "ALL_GEOM_SCREENS", ALL_GEOM_SCREENS );
    assert( r >= 0 );


    r = se->RegisterEnum( "GUI_VSP_SCREEN" );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "GUI_VSP_SCREEN", "VSP_ADV_LINK_SCREEN", VSP_ADV_LINK_SCREEN );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "GUI_VSP_SCREEN", "VSP_ADV_LINK_VAR_RENAME_SCREEN", VSP_ADV_LINK_VAR_RENAME_SCREEN );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "GUI_VSP_SCREEN", "VSP_AERO_STRUCT_SCREEN", VSP_AERO_STRUCT_SCREEN );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "GUI_VSP_SCREEN", "VSP_AIRFOIL_CURVES_EXPORT_SCREEN", VSP_AIRFOIL_CURVES_EXPORT_SCREEN );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "GUI_VSP_SCREEN", "VSP_AIRFOIL_POINTS_EXPORT_SCREEN", VSP_AIRFOIL_POINTS_EXPORT_SCREEN );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "GUI_VSP_SCREEN", "VSP_BACKGROUND_SCREEN", VSP_BACKGROUND_SCREEN );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "GUI_VSP_SCREEN", "VSP_BACKGROUND3D_SCREEN", VSP_BACKGROUND3D_SCREEN );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "GUI_VSP_SCREEN", "VSP_BACKGROUND3D_PREVIEW_SCREEN", VSP_BACKGROUND3D_PREVIEW_SCREEN );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "GUI_VSP_SCREEN", "VSP_BEM_OPTIONS_SCREEN", VSP_BEM_OPTIONS_SCREEN );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "GUI_VSP_SCREEN", "VSP_CFD_MESH_SCREEN", VSP_CFD_MESH_SCREEN );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "GUI_VSP_SCREEN", "VSP_CLIPPING_SCREEN", VSP_CLIPPING_SCREEN );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "GUI_VSP_SCREEN", "VSP_COMP_GEOM_SCREEN", VSP_COMP_GEOM_SCREEN );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "GUI_VSP_SCREEN", "VSP_COR_SCREEN", VSP_COR_SCREEN );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "GUI_VSP_SCREEN", "VSP_CURVE_EDIT_SCREEN", VSP_CURVE_EDIT_SCREEN );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "GUI_VSP_SCREEN", "VSP_DEGEN_GEOM_SCREEN", VSP_DEGEN_GEOM_SCREEN );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "GUI_VSP_SCREEN", "VSP_DESIGN_VAR_SCREEN", VSP_DESIGN_VAR_SCREEN );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "GUI_VSP_SCREEN", "VSP_DXF_OPTIONS_SCREEN", VSP_DXF_OPTIONS_SCREEN );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "GUI_VSP_SCREEN", "VSP_EXPORT_SCREEN", VSP_EXPORT_SCREEN );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "GUI_VSP_SCREEN", "VSP_FEA_PART_EDIT_SCREEN", VSP_FEA_PART_EDIT_SCREEN );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "GUI_VSP_SCREEN", "VSP_FEA_XSEC_SCREEN", VSP_FEA_XSEC_SCREEN );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "GUI_VSP_SCREEN", "VSP_FIT_MODEL_SCREEN", VSP_FIT_MODEL_SCREEN );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "GUI_VSP_SCREEN", "VSP_IGES_OPTIONS_SCREEN", VSP_IGES_OPTIONS_SCREEN );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "GUI_VSP_SCREEN", "VSP_IGES_STRUCTURE_OPTIONS_SCREEN", VSP_IGES_STRUCTURE_OPTIONS_SCREEN );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "GUI_VSP_SCREEN", "VSP_EXPORT_CUSTOM_SCRIPT", VSP_EXPORT_CUSTOM_SCRIPT );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "GUI_VSP_SCREEN", "VSP_GEOMETRY_ANALYSIS_SCREEN", VSP_GEOMETRY_ANALYSIS_SCREEN );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "GUI_VSP_SCREEN", "VSP_IMPORT_SCREEN", VSP_IMPORT_SCREEN );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "GUI_VSP_SCREEN", "VSP_LIGHTING_SCREEN", VSP_LIGHTING_SCREEN );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "GUI_VSP_SCREEN", "VSP_MANAGE_GEOM_SCREEN", VSP_MANAGE_GEOM_SCREEN );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "GUI_VSP_SCREEN", "VSP_MANAGE_TEXTURE_SCREEN", VSP_MANAGE_TEXTURE_SCREEN );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "GUI_VSP_SCREEN", "VSP_MASS_PROP_SCREEN", VSP_MASS_PROP_SCREEN );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "GUI_VSP_SCREEN", "VSP_MATERIAL_EDIT_SCREEN", VSP_MATERIAL_EDIT_SCREEN );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "GUI_VSP_SCREEN", "VSP_MEASURE_SCREEN", VSP_MEASURE_SCREEN );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "GUI_VSP_SCREEN", "VSP_MODE_EDITOR_SCREEN", VSP_MODE_EDITOR_SCREEN );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "GUI_VSP_SCREEN", "VSP_SNAP_TO_SCREEN", VSP_SNAP_TO_SCREEN );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "GUI_VSP_SCREEN", "VSP_PARASITE_DRAG_SCREEN", VSP_PARASITE_DRAG_SCREEN );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "GUI_VSP_SCREEN", "VSP_PARM_DEBUG_SCREEN", VSP_PARM_DEBUG_SCREEN );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "GUI_VSP_SCREEN", "VSP_PARM_LINK_SCREEN", VSP_PARM_LINK_SCREEN );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "GUI_VSP_SCREEN", "VSP_PARM_SCREEN", VSP_PARM_SCREEN );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "GUI_VSP_SCREEN", "VSP_PICK_SET_SCREEN", VSP_PICK_SET_SCREEN );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "GUI_VSP_SCREEN", "VSP_PREFERENCES_SCREEN", VSP_PREFERENCES_SCREEN );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "GUI_VSP_SCREEN", "VSP_PROJECTION_SCREEN", VSP_PROJECTION_SCREEN );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "GUI_VSP_SCREEN", "VSP_PSLICE_SCREEN", VSP_PSLICE_SCREEN );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "GUI_VSP_SCREEN", "VSP_RESULTS_VIEWER_SCREEN", VSP_RESULTS_VIEWER_SCREEN );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "GUI_VSP_SCREEN", "VSP_SCREENSHOT_SCREEN", VSP_SCREENSHOT_SCREEN );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "GUI_VSP_SCREEN", "VSP_SELECT_FILE_SCREEN", VSP_SELECT_FILE_SCREEN );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "GUI_VSP_SCREEN", "VSP_SET_EDITOR_SCREEN", VSP_SET_EDITOR_SCREEN );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "GUI_VSP_SCREEN", "VSP_STEP_OPTIONS_SCREEN", VSP_STEP_OPTIONS_SCREEN );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "GUI_VSP_SCREEN", "VSP_STEP_STRUCTURE_OPTIONS_SCREEN", VSP_STEP_STRUCTURE_OPTIONS_SCREEN );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "GUI_VSP_SCREEN", "VSP_STL_OPTIONS_SCREEN", VSP_STL_OPTIONS_SCREEN );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "GUI_VSP_SCREEN", "VSP_STRUCT_SCREEN", VSP_STRUCT_SCREEN );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "GUI_VSP_SCREEN", "VSP_STRUCT_ASSEMBLY_SCREEN", VSP_STRUCT_ASSEMBLY_SCREEN );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "GUI_VSP_SCREEN", "VSP_SURFACE_INTERSECTION_SCREEN", VSP_SURFACE_INTERSECTION_SCREEN );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "GUI_VSP_SCREEN", "VSP_SVG_OPTIONS_SCREEN", VSP_SVG_OPTIONS_SCREEN );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "GUI_VSP_SCREEN", "VSP_USER_PARM_SCREEN", VSP_USER_PARM_SCREEN );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "GUI_VSP_SCREEN", "VSP_ATTRIBUTE_EXPLORER_SCREEN", VSP_ATTRIBUTE_EXPLORER_SCREEN );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "GUI_VSP_SCREEN", "VSP_VAR_PRESET_SCREEN", VSP_VAR_PRESET_SCREEN );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "GUI_VSP_SCREEN", "VSP_VEH_NOTES_SCREEN", VSP_VEH_NOTES_SCREEN );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "GUI_VSP_SCREEN", "VSP_VEH_SCREEN", VSP_VEH_SCREEN );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "GUI_VSP_SCREEN", "VSP_VIEW_SCREEN", VSP_VIEW_SCREEN );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "GUI_VSP_SCREEN", "VSP_VSPAERO_PLOT_SCREEN", VSP_VSPAERO_PLOT_SCREEN );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "GUI_VSP_SCREEN", "VSP_VSPAERO_SCREEN", VSP_VSPAERO_SCREEN );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "GUI_VSP_SCREEN", "VSP_XSEC_SCREEN", VSP_XSEC_SCREEN );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "GUI_VSP_SCREEN", "VSP_WAVEDRAG_SCREEN", VSP_WAVEDRAG_SCREEN );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "GUI_VSP_SCREEN", "VSP_MAIN_SCREEN", VSP_MAIN_SCREEN );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "GUI_VSP_SCREEN", "VSP_NUM_SCREENS", VSP_NUM_SCREENS );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "GUI_VSP_SCREEN", "VSP_ALL_SCREENS", VSP_ALL_SCREENS );
    assert( r >= 0 );


    r = se->RegisterEnum( "INIT_EDIT_XSEC_TYPE" );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "INIT_EDIT_XSEC_TYPE", "EDIT_XSEC_CIRCLE", EDIT_XSEC_CIRCLE );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "INIT_EDIT_XSEC_TYPE", "EDIT_XSEC_ELLIPSE", EDIT_XSEC_ELLIPSE );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "INIT_EDIT_XSEC_TYPE", "EDIT_XSEC_RECTANGLE", EDIT_XSEC_RECTANGLE );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "INIT_EDIT_XSEC_TYPE", "NUM_INIT_EDIT_XSEC_TYPES", NUM_INIT_EDIT_XSEC_TYPES );
    assert( r >= 0 );


    r = se->RegisterEnum( "IMPORT_TYPE" );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "IMPORT_TYPE", "IMPORT_STL", IMPORT_STL );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "IMPORT_TYPE", "IMPORT_NASCART", IMPORT_NASCART );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "IMPORT_TYPE", "IMPORT_CART3D_TRI", IMPORT_CART3D_TRI );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "IMPORT_TYPE", "IMPORT_XSEC_MESH", IMPORT_XSEC_MESH );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "IMPORT_TYPE", "IMPORT_PTS", IMPORT_PTS );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "IMPORT_TYPE", "IMPORT_V2", IMPORT_V2 );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "IMPORT_TYPE", "IMPORT_BEM", IMPORT_BEM );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "IMPORT_TYPE", "IMPORT_XSEC_WIRE", IMPORT_XSEC_WIRE );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "IMPORT_TYPE", "IMPORT_P3D_WIRE", IMPORT_P3D_WIRE );
    assert( r >= 0 );


    r = se->RegisterEnum( "INTERSECT_EXPORT_TYPE" );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "INTERSECT_EXPORT_TYPE", "INTERSECT_SRF_FILE_NAME", INTERSECT_SRF_FILE_NAME );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "INTERSECT_EXPORT_TYPE", "INTERSECT_CURV_FILE_NAME", INTERSECT_CURV_FILE_NAME );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "INTERSECT_EXPORT_TYPE", "INTERSECT_PLOT3D_FILE_NAME", INTERSECT_PLOT3D_FILE_NAME );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "INTERSECT_EXPORT_TYPE", "INTERSECT_IGES_FILE_NAME", INTERSECT_IGES_FILE_NAME );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "INTERSECT_EXPORT_TYPE", "INTERSECT_STEP_FILE_NAME", INTERSECT_STEP_FILE_NAME );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "INTERSECT_EXPORT_TYPE", "INTERSECT_NUM_FILE_NAMES", INTERSECT_NUM_FILE_NAMES );
    assert( r >= 0 );


    r = se->RegisterEnum( "LEN_UNITS" );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "LEN_UNITS", "LEN_MM", LEN_MM );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "LEN_UNITS", "LEN_CM", LEN_CM );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "LEN_UNITS", "LEN_M", LEN_M );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "LEN_UNITS", "LEN_IN", LEN_IN );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "LEN_UNITS", "LEN_FT", LEN_FT );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "LEN_UNITS", "LEN_YD", LEN_YD );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "LEN_UNITS", "LEN_UNITLESS", LEN_UNITLESS );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "LEN_UNITS", "NUM_LEN_UNIT", NUM_LEN_UNIT );
    assert( r >= 0 );


    r = se->RegisterEnum( "MASS_UNIT" );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "MASS_UNIT", "MASS_UNIT_G", MASS_UNIT_G );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "MASS_UNIT", "MASS_UNIT_KG", MASS_UNIT_KG );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "MASS_UNIT", "MASS_UNIT_TONNE", MASS_UNIT_TONNE );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "MASS_UNIT", "MASS_UNIT_LBM", MASS_UNIT_LBM );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "MASS_UNIT", "MASS_UNIT_SLUG", MASS_UNIT_SLUG );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "MASS_UNIT", "MASS_LBFSEC2IN", MASS_LBFSEC2IN );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "MASS_UNIT", "NUM_MASS_UNIT", NUM_MASS_UNIT );
    assert( r >= 0 );

    r = se->RegisterEnum( "OBJ_ID_LENGTH" );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "OBJ_ID_LENGTH", "ID_LENGTH_ATTR", ID_LENGTH_ATTR );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "OBJ_ID_LENGTH", "ID_LENGTH_ATTRCOLL", ID_LENGTH_ATTRCOLL );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "OBJ_ID_LENGTH", "ID_LENGTH_PARMCONTAINER", ID_LENGTH_PARMCONTAINER );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "OBJ_ID_LENGTH", "ID_LENGTH_PARM", ID_LENGTH_PARM );
    assert( r >= 0 );

    r = se->RegisterEnum( "MESH_TYPE" );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "MESH_TYPE", "TRI_MESH_TYPE", TRI_MESH_TYPE );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "MESH_TYPE", "QUAD_MESH_TYPE", QUAD_MESH_TYPE );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "MESH_TYPE", "NGON_MESH_TYPE", NGON_MESH_TYPE );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "MESH_TYPE", "NUM_MESH_TYPE", NUM_MESH_TYPE );
    assert( r >= 0 );


    r = se->RegisterEnum( "PARM_TYPE" );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "PARM_TYPE", "PARM_DOUBLE_TYPE", PARM_DOUBLE_TYPE );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "PARM_TYPE", "PARM_INT_TYPE",    PARM_INT_TYPE );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "PARM_TYPE", "PARM_BOOL_TYPE",   PARM_BOOL_TYPE );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "PARM_TYPE", "PARM_FRACTION_TYPE", PARM_FRACTION_TYPE );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "PARM_TYPE", "PARM_LIMITED_INT_TYPE", PARM_LIMITED_INT_TYPE );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "PARM_TYPE", "PARM_NOTEQ_TYPE", PARM_NOTEQ_TYPE );
    assert( r >= 0 );


    r = se->RegisterEnum( "PATCH_TYPE" );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "PATCH_TYPE", "PATCH_NONE", vsp::PATCH_NONE );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "PATCH_TYPE", "PATCH_POINT", vsp::PATCH_POINT );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "PATCH_TYPE", "PATCH_LINE", vsp::PATCH_LINE );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "PATCH_TYPE", "PATCH_COPY", vsp::PATCH_COPY );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "PATCH_TYPE", "PATCH_HALFWAY", vsp::PATCH_HALFWAY );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "PATCH_TYPE", "PATCH_NUM_TYPES", vsp::PATCH_NUM_TYPES );
    assert( r >= 0 );


    r = se->RegisterEnum( "PCURV_TYPE" );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "PCURV_TYPE", "LINEAR", LINEAR );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "PCURV_TYPE", "PCHIP", PCHIP );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "PCURV_TYPE", "CEDIT", CEDIT );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "PCURV_TYPE", "APPROX_CEDIT", APPROX_CEDIT );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "PCURV_TYPE", "NUM_PCURV_TYPE", NUM_PCURV_TYPE );
    assert( r >= 0 );


    r = se->RegisterEnum( "PRES_UNITS" );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "PRES_UNITS", "PRES_UNIT_PSF", PRES_UNIT_PSF );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "PRES_UNITS", "PRES_UNIT_PSI", PRES_UNIT_PSI );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "PRES_UNITS", "PRES_UNIT_BA", PRES_UNIT_BA );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "PRES_UNITS", "PRES_UNIT_PA", PRES_UNIT_PA );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "PRES_UNITS", "PRES_UNIT_KPA", PRES_UNIT_KPA );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "PRES_UNITS", "PRES_UNIT_MPA", PRES_UNIT_MPA );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "PRES_UNITS", "PRES_UNIT_INCHHG", PRES_UNIT_INCHHG );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "PRES_UNITS", "PRES_UNIT_MMHG", PRES_UNIT_MMHG );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "PRES_UNITS", "PRES_UNIT_MMH20", PRES_UNIT_MMH20 );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "PRES_UNITS", "PRES_UNIT_MB", PRES_UNIT_MB );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "PRES_UNITS", "PRES_UNIT_ATM", PRES_UNIT_ATM );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "PRES_UNITS", "NUM_PRES_UNIT", NUM_PRES_UNIT );
    assert( r >= 0 );


    r = se->RegisterEnum( "PROJ_BNDY_TYPE" );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "PROJ_BNDY_TYPE", "NO_BOUNDARY", NO_BOUNDARY );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "PROJ_BNDY_TYPE", "SET_BOUNDARY", SET_BOUNDARY );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "PROJ_BNDY_TYPE", "GEOM_BOUNDARY", GEOM_BOUNDARY );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "PROJ_BNDY_TYPE", "NUM_PROJ_BNDY_OPTIONS", NUM_PROJ_BNDY_OPTIONS );
    assert( r >= 0 );


    r = se->RegisterEnum( "PROJ_DIR_TYPE" );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "PROJ_DIR_TYPE", "X_PROJ", X_PROJ );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "PROJ_DIR_TYPE", "Y_PROJ", Y_PROJ );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "PROJ_DIR_TYPE", "Z_PROJ", Z_PROJ );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "PROJ_DIR_TYPE", "GEOM_PROJ", GEOM_PROJ );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "PROJ_DIR_TYPE", "VEC_PROJ", VEC_PROJ );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "PROJ_DIR_TYPE", "NUM_PROJ_DIR_OPTIONS", NUM_PROJ_DIR_OPTIONS );
    assert( r >= 0 );


    r = se->RegisterEnum( "PROJ_TGT_TYPE" );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "PROJ_TGT_TYPE", "SET_TARGET", SET_TARGET );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "PROJ_TGT_TYPE", "GEOM_TARGET", GEOM_TARGET );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "PROJ_TGT_TYPE", "MODE_TARGET", MODE_TARGET );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "PROJ_TGT_TYPE", "Z_TARGET", Z_TARGET );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "PROJ_TGT_TYPE", "XYZ_TARGET", XYZ_TARGET );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "PROJ_TGT_TYPE", "NUM_PROJ_TGT_OPTIONS", NUM_PROJ_TGT_OPTIONS );
    assert( r >= 0 );


    r = se->RegisterEnum( "PROP_AZIMUTH_MODE" );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "PROP_AZIMUTH_MODE", "PROP_AZI_UNIFORM", PROP_AZI_UNIFORM );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "PROP_AZIMUTH_MODE", "PROP_AZI_FREE", PROP_AZI_FREE );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "PROP_AZIMUTH_MODE", "PROP_AZI_BALANCED", PROP_AZI_BALANCED );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "PROP_AZIMUTH_MODE", "NUM_PROP_AZI", NUM_PROP_AZI );
    assert( r >= 0 );


    r = se->RegisterEnum( "PROP_DRIVERS" );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "PROP_DRIVERS", "RPM_PROP_DRIVER", RPM_PROP_DRIVER );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "PROP_DRIVERS", "CT_PROP_DRIVER", CT_PROP_DRIVER );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "PROP_DRIVERS", "CP_PROP_DRIVER", CP_PROP_DRIVER );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "PROP_DRIVERS", "T_PROP_DRIVER", T_PROP_DRIVER );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "PROP_DRIVERS", "ETA_PROP_DRIVER", ETA_PROP_DRIVER );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "PROP_DRIVERS", "J_PROP_DRIVER", J_PROP_DRIVER );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "PROP_DRIVERS", "P_PROP_DRIVER", P_PROP_DRIVER );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "PROP_DRIVERS", "CQ_PROP_DRIVER", CQ_PROP_DRIVER );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "PROP_DRIVERS", "Q_PROP_DRIVER", Q_PROP_DRIVER );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "PROP_DRIVERS", "NUM_PROP_DRIVER", NUM_PROP_DRIVER );
    assert( r >= 0 );


    r = se->RegisterEnum( "PROP_MODE" );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "PROP_MODE", "PROP_BLADES", PROP_BLADES );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "PROP_MODE", "PROP_BOTH", PROP_BOTH );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "PROP_MODE", "PROP_DISK", PROP_DISK );
    assert( r >= 0 );


    r = se->RegisterEnum( "PROP_PCURVE" );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "PROP_PCURVE", "PROP_CHORD", PROP_CHORD );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "PROP_PCURVE", "PROP_TWIST", PROP_TWIST );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "PROP_PCURVE", "PROP_RAKE", PROP_RAKE );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "PROP_PCURVE", "PROP_SKEW", PROP_SKEW );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "PROP_PCURVE", "PROP_SWEEP", PROP_SWEEP );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "PROP_PCURVE", "PROP_THICK", PROP_THICK );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "PROP_PCURVE", "PROP_CLI", PROP_CLI );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "PROP_PCURVE", "PROP_AXIAL", PROP_AXIAL );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "PROP_PCURVE", "PROP_TANGENTIAL", PROP_TANGENTIAL );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "PROP_PCURVE", "NUM_PROP_PCURVE", NUM_PROP_PCURVE );
    assert( r >= 0 );


    r = se->RegisterEnum( "REORDER_TYPE" );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "REORDER_TYPE", "REORDER_MOVE_UP", REORDER_MOVE_UP );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "REORDER_TYPE", "REORDER_MOVE_DOWN", REORDER_MOVE_DOWN );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "REORDER_TYPE", "REORDER_MOVE_TOP", REORDER_MOVE_TOP );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "REORDER_TYPE", "REORDER_MOVE_BOTTOM", REORDER_MOVE_BOTTOM );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "REORDER_TYPE", "NUM_REORDER_TYPES", NUM_REORDER_TYPES );
    assert( r >= 0 );


    r = se->RegisterEnum( "REF_WING_TYPE" );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "REF_WING_TYPE", "MANUAL_REF", MANUAL_REF );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "REF_WING_TYPE", "COMPONENT_REF", COMPONENT_REF );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "REF_WING_TYPE", "NUM_REF_TYPES", NUM_REF_TYPES );
    assert( r >= 0 );


    r = se->RegisterEnum( "RES_DATA_TYPE" );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "RES_DATA_TYPE", "INVALID_TYPE", INVALID_TYPE );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "RES_DATA_TYPE", "BOOL_DATA", BOOL_DATA );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "RES_DATA_TYPE", "INT_DATA", INT_DATA );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "RES_DATA_TYPE", "DOUBLE_DATA", DOUBLE_DATA );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "RES_DATA_TYPE", "STRING_DATA", STRING_DATA );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "RES_DATA_TYPE", "VEC3D_DATA", VEC3D_DATA );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "RES_DATA_TYPE", "INT_MATRIX_DATA", INT_MATRIX_DATA );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "RES_DATA_TYPE", "DOUBLE_MATRIX_DATA", DOUBLE_MATRIX_DATA );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "RES_DATA_TYPE", "ATTR_COLLECTION_DATA", ATTR_COLLECTION_DATA );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "RES_DATA_TYPE", "PARM_REFERENCE_DATA", PARM_REFERENCE_DATA );
    assert( r >= 0 );


    r = se->RegisterEnum( "RES_GEOM_TYPE" );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "RES_GEOM_TYPE", "MESH_INDEXED_TRI", MESH_INDEXED_TRI );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "RES_GEOM_TYPE", "MESH_SLICE_TRI", MESH_SLICE_TRI );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "RES_GEOM_TYPE", "GEOM_XSECS", GEOM_XSECS );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "RES_GEOM_TYPE", "MESH_INDEX_AND_SLICE_TRI", MESH_INDEX_AND_SLICE_TRI );
    assert( r >= 0 );


    r = se->RegisterEnum( "RHO_UNITS" );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "RHO_UNITS", "RHO_UNIT_SLUG_FT3", RHO_UNIT_SLUG_FT3 );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "RHO_UNITS", "RHO_UNIT_G_CM3", RHO_UNIT_G_CM3 );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "RHO_UNITS", "RHO_UNIT_KG_M3", RHO_UNIT_KG_M3 );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "RHO_UNITS", "RHO_UNIT_TONNE_MM3", RHO_UNIT_TONNE_MM3 );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "RHO_UNITS", "RHO_UNIT_LBM_FT3", RHO_UNIT_LBM_FT3 );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "RHO_UNITS", "RHO_UNIT_LBFSEC2_IN4", RHO_UNIT_LBFSEC2_IN4 );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "RHO_UNITS", "RHO_UNIT_LBM_IN3", RHO_UNIT_LBM_IN3 );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "RHO_UNITS", "NUM_RHO_UNIT", NUM_RHO_UNIT );
    assert( r >= 0 );


    r = se->RegisterEnum( "ROUTE_PT_TYPE" );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "ROUTE_PT_TYPE", "ROUTE_PT_COMP", ROUTE_PT_COMP );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "ROUTE_PT_TYPE", "ROUTE_PT_UV", ROUTE_PT_UV );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "ROUTE_PT_TYPE", "ROUTE_PT_RST", ROUTE_PT_RST );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "ROUTE_PT_TYPE", "ROUTE_PT_LMN", ROUTE_PT_LMN );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "ROUTE_PT_TYPE", "ROUTE_PT_EtaMN", ROUTE_PT_EtaMN );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "ROUTE_PT_TYPE", "ROUTE_PT_NUM_TYPES", ROUTE_PT_NUM_TYPES );
    assert( r >= 0 );


    r = se->RegisterEnum( "ROUTE_PT_DELTA_TYPE" );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "ROUTE_PT_DELTA_TYPE", "ROUTE_PT_DELTA_XYZ", ROUTE_PT_DELTA_XYZ );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "ROUTE_PT_DELTA_TYPE", "ROUTE_PT_DELTA_COMP", ROUTE_PT_DELTA_COMP );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "ROUTE_PT_DELTA_TYPE", "ROUTE_PT_DELTA_UVN", ROUTE_PT_DELTA_UVN );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "ROUTE_PT_DELTA_TYPE", "ROUTE_PT_DELTA_NUM_TYPES", ROUTE_PT_DELTA_NUM_TYPES );
    assert( r >= 0 );


    r = se->RegisterEnum( "SET_TYPE" );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "SET_TYPE", "SET_NONE", SET_NONE );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "SET_TYPE", "SET_ALL", SET_ALL );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "SET_TYPE", "SET_SHOWN", SET_SHOWN );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "SET_TYPE", "SET_NOT_SHOWN", SET_NOT_SHOWN );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "SET_TYPE", "SET_FIRST_USER", SET_FIRST_USER );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "SET_TYPE", "MIN_NUM_USER", MIN_NUM_USER );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "SET_TYPE", "MAX_NUM_SETS", MAX_NUM_SETS );
    assert( r >= 0 );


    r = se->RegisterEnum( "STEP_REPRESENTATION" );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "STEP_REPRESENTATION", "STEP_SHELL", STEP_SHELL );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "STEP_REPRESENTATION", "STEP_BREP", STEP_BREP );
    assert( r >= 0 );


    r = se->RegisterEnum( "SUBSURF_INCLUDE" );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "SUBSURF_INCLUDE", "SS_INC_TREAT_AS_PARENT", SS_INC_TREAT_AS_PARENT );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "SUBSURF_INCLUDE", "SS_INC_SEPARATE_TREATMENT", SS_INC_SEPARATE_TREATMENT );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "SUBSURF_INCLUDE", "SS_INC_ZERO_DRAG", SS_INC_ZERO_DRAG );
    assert( r >= 0 );


    r = se->RegisterEnum( "SUBSURF_INOUT" );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "SUBSURF_INOUT", "INSIDE", INSIDE );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "SUBSURF_INOUT", "OUTSIDE", OUTSIDE );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "SUBSURF_INOUT", "NONE", NONE );
    assert( r >= 0 );


    r = se->RegisterEnum( "SUBSURF_LINE_TYPE" );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "SUBSURF_LINE_TYPE", "CONST_U", CONST_U );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "SUBSURF_LINE_TYPE", "CONST_W", CONST_W );
    assert( r >= 0 );


    r = se->RegisterEnum( "SUBSURF_TYPE" );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "SUBSURF_TYPE", "SS_LINE", SS_LINE );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "SUBSURF_TYPE", "SS_RECTANGLE", SS_RECTANGLE );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "SUBSURF_TYPE", "SS_ELLIPSE", SS_ELLIPSE );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "SUBSURF_TYPE", "SS_CONTROL", SS_CONTROL );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "SUBSURF_TYPE", "SS_LINE_ARRAY", SS_LINE_ARRAY );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "SUBSURF_TYPE", "SS_FINITE_LINE", SS_FINITE_LINE );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "SUBSURF_TYPE", "SS_XSEC_CURVE", SS_XSEC_CURVE );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "SUBSURF_TYPE", "SS_NUM_TYPES", SS_NUM_TYPES );
    assert( r >= 0 );


    r = se->RegisterEnum( "SYM_FLAG" );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "SYM_FLAG", "SYM_XY", SYM_XY );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "SYM_FLAG", "SYM_XZ", SYM_XZ );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "SYM_FLAG", "SYM_YZ", SYM_YZ );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "SYM_FLAG", "SYM_ROT_X", SYM_ROT_X );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "SYM_FLAG", "SYM_ROT_Y", SYM_ROT_Y );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "SYM_FLAG", "SYM_ROT_Z", SYM_ROT_Z );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "SYM_FLAG", "SYM_PLANAR_TYPES", SYM_PLANAR_TYPES );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "SYM_FLAG", "SYM_NUM_TYPES", SYM_NUM_TYPES );
    assert( r >= 0 );


    r = se->RegisterEnum( "SYM_XSEC_TYPE" );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "SYM_XSEC_TYPE", "SYM_NONE", SYM_NONE );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "SYM_XSEC_TYPE", "SYM_RL", SYM_RL );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "SYM_XSEC_TYPE", "SYM_TB", SYM_TB );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "SYM_XSEC_TYPE", "SYM_ALL", SYM_ALL );


    r = se->RegisterEnum( "TEMP_UNITS" );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "TEMP_UNITS", "TEMP_UNIT_K", TEMP_UNIT_K );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "TEMP_UNITS", "TEMP_UNIT_C", TEMP_UNIT_C );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "TEMP_UNITS", "TEMP_UNIT_F", TEMP_UNIT_F );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "TEMP_UNITS", "TEMP_UNIT_R", TEMP_UNIT_R );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "TEMP_UNITS", "NUM_TEMP_UNIT", NUM_TEMP_UNIT );
    assert( r >= 0 );


    r = se->RegisterEnum( "TIRE_DIM_MODES" );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "TIRE_DIM_MODES", "TIRE_DIM_IN", TIRE_DIM_IN );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "TIRE_DIM_MODES", "TIRE_DIM_MODEL", TIRE_DIM_MODEL );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "TIRE_DIM_MODES", "TIRE_DIM_FRAC", TIRE_DIM_FRAC );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "TIRE_DIM_MODES", "NUM_TIRE_DIM_MODES", NUM_TIRE_DIM_MODES );
    assert( r >= 0 );


    r = se->RegisterEnum( "VEL_UNITS" );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "VEL_UNITS", "V_UNIT_FT_S", V_UNIT_FT_S );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "VEL_UNITS", "V_UNIT_M_S", V_UNIT_M_S );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "VEL_UNITS", "V_UNIT_MPH", V_UNIT_MPH );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "VEL_UNITS", "V_UNIT_KM_HR", V_UNIT_KM_HR );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "VEL_UNITS", "V_UNIT_KEAS", V_UNIT_KEAS );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "VEL_UNITS", "V_UNIT_KTAS", V_UNIT_KTAS );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "VEL_UNITS", "V_UNIT_MACH", V_UNIT_MACH );
    assert( r >= 0 );


    r = se->RegisterEnum( "VIEW_NUM" );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "VIEW_NUM", "VIEW_1", VIEW_1 );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "VIEW_NUM", "VIEW_2HOR", VIEW_2HOR );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "VIEW_NUM", "VIEW_2VER", VIEW_2VER );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "VIEW_NUM", "VIEW_4", VIEW_4 );
    assert( r >= 0 );


    r = se->RegisterEnum( "VIEW_ROT" );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "VIEW_ROT", "ROT_0", ROT_0 );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "VIEW_ROT", "ROT_90", ROT_90 );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "VIEW_ROT", "ROT_180", ROT_180 );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "VIEW_ROT", "ROT_270", ROT_270 );
    assert( r >= 0 );


    r = se->RegisterEnum( "VIEW_TYPE" );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "VIEW_TYPE", "VIEW_LEFT", VIEW_LEFT );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "VIEW_TYPE", "VIEW_RIGHT", VIEW_RIGHT );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "VIEW_TYPE", "VIEW_TOP", VIEW_TOP );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "VIEW_TYPE", "VIEW_BOTTOM", VIEW_BOTTOM );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "VIEW_TYPE", "VIEW_FRONT", VIEW_FRONT );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "VIEW_TYPE", "VIEW_REAR", VIEW_REAR );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "VIEW_TYPE", "VIEW_NONE", VIEW_NONE );
    assert( r >= 0 );


    r = se->RegisterEnum( "VSPAERO_NOISE_TYPE" );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "VSPAERO_NOISE_TYPE", "NOISE_FLYBY", NOISE_FLYBY );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "VSPAERO_NOISE_TYPE", "NOISE_FOOTPRINT", NOISE_FOOTPRINT );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "VSPAERO_NOISE_TYPE", "NOISE_STEADY", NOISE_STEADY );
    assert( r >= 0 );


    r = se->RegisterEnum( "VSPAERO_NOISE_UNIT" );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "VSPAERO_NOISE_UNIT", "NOISE_SI", NOISE_SI );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "VSPAERO_NOISE_UNIT", "NOISE_ENGLISH", NOISE_ENGLISH );
    assert( r >= 0 );


    r = se->RegisterEnum( "VSPAERO_PRECONDITION" );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "VSPAERO_PRECONDITION", "PRECON_MATRIX", PRECON_MATRIX );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "VSPAERO_PRECONDITION", "PRECON_JACOBI", PRECON_JACOBI );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "VSPAERO_PRECONDITION", "PRECON_SSOR", PRECON_SSOR );
    assert( r >= 0 );


    r = se->RegisterEnum( "VSPAERO_STABILITY_TYPE" );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "VSPAERO_STABILITY_TYPE", "STABILITY_OFF", STABILITY_OFF );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "VSPAERO_STABILITY_TYPE", "STABILITY_DEFAULT", STABILITY_DEFAULT );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "VSPAERO_STABILITY_TYPE", "STABILITY_P_ANALYSIS", STABILITY_P_ANALYSIS );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "VSPAERO_STABILITY_TYPE", "STABILITY_Q_ANALYSIS", STABILITY_Q_ANALYSIS );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "VSPAERO_STABILITY_TYPE", "STABILITY_R_ANALYSIS", STABILITY_R_ANALYSIS );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "VSPAERO_STABILITY_TYPE", "STABILITY_PITCH", STABILITY_PITCH );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "VSPAERO_STABILITY_TYPE", "STABILITY_NUM_TYPES", STABILITY_NUM_TYPES );
    assert( r >= 0 );


    r = se->RegisterEnum( "VSPAERO_CLMAX_TYPE" );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "VSPAERO_CLMAX_TYPE", "CLMAX_OFF", CLMAX_OFF );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "VSPAERO_CLMAX_TYPE", "CLMAX_2D", CLMAX_2D );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "VSPAERO_CLMAX_TYPE", "CLMAX_CARLSON", CLMAX_CARLSON );
    assert( r >= 0 );


    r = se->RegisterEnum( "VSP_SURF_CFD_TYPE" );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "VSP_SURF_CFD_TYPE", "CFD_NORMAL", CFD_NORMAL );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "VSP_SURF_CFD_TYPE", "CFD_NEGATIVE", CFD_NEGATIVE );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "VSP_SURF_CFD_TYPE", "CFD_TRANSPARENT", CFD_TRANSPARENT );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "VSP_SURF_CFD_TYPE", "CFD_STRUCTURE", CFD_STRUCTURE );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "VSP_SURF_CFD_TYPE", "CFD_STIFFENER", CFD_STIFFENER );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "VSP_SURF_CFD_TYPE", "CFD_NUM_TYPES", CFD_NUM_TYPES );
    assert( r >= 0 );


    r = se->RegisterEnum( "VSP_SURF_TYPE" );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "VSP_SURF_TYPE", "NORMAL_SURF", NORMAL_SURF );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "VSP_SURF_TYPE", "WING_SURF", WING_SURF );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "VSP_SURF_TYPE", "DISK_SURF", DISK_SURF );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "VSP_SURF_TYPE", "NUM_SURF_TYPES", NUM_SURF_TYPES );
    assert( r >= 0 );


    r = se->RegisterEnum( "WING_BLEND" );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "WING_BLEND", "BLEND_FREE", vsp::BLEND_FREE );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "WING_BLEND", "BLEND_ANGLES", vsp::BLEND_ANGLES );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "WING_BLEND", "BLEND_MATCH_IN_LE_TRAP", vsp::BLEND_MATCH_IN_LE_TRAP );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "WING_BLEND", "BLEND_MATCH_IN_TE_TRAP", vsp::BLEND_MATCH_IN_TE_TRAP );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "WING_BLEND", "BLEND_MATCH_OUT_LE_TRAP", vsp::BLEND_MATCH_OUT_LE_TRAP );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "WING_BLEND", "BLEND_MATCH_OUT_TE_TRAP", vsp::BLEND_MATCH_OUT_TE_TRAP );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "WING_BLEND", "BLEND_MATCH_IN_ANGLES", vsp::BLEND_MATCH_IN_ANGLES );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "WING_BLEND", "BLEND_MATCH_LE_ANGLES", vsp::BLEND_MATCH_LE_ANGLES );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "WING_BLEND", "BLEND_NUM_TYPES", vsp::BLEND_NUM_TYPES );
    assert( r >= 0 );


    r = se->RegisterEnum( "WING_DRIVERS" );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "WING_DRIVERS", "AR_WSECT_DRIVER", vsp::AR_WSECT_DRIVER );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "WING_DRIVERS", "SPAN_WSECT_DRIVER", vsp::SPAN_WSECT_DRIVER );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "WING_DRIVERS", "AREA_WSECT_DRIVER", vsp::AREA_WSECT_DRIVER );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "WING_DRIVERS", "TAPER_WSECT_DRIVER", vsp::TAPER_WSECT_DRIVER );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "WING_DRIVERS", "AVEC_WSECT_DRIVER", vsp::AVEC_WSECT_DRIVER );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "WING_DRIVERS", "ROOTC_WSECT_DRIVER", vsp::ROOTC_WSECT_DRIVER );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "WING_DRIVERS", "TIPC_WSECT_DRIVER", vsp::TIPC_WSECT_DRIVER );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "WING_DRIVERS", "SECSWEEP_WSECT_DRIVER", vsp::SECSWEEP_WSECT_DRIVER );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "WING_DRIVERS", "NUM_WSECT_DRIVER", vsp::NUM_WSECT_DRIVER );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "WING_DRIVERS", "SWEEP_WSECT_DRIVER", vsp::SWEEP_WSECT_DRIVER );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "WING_DRIVERS", "SWEEPLOC_WSECT_DRIVER", vsp::SWEEPLOC_WSECT_DRIVER );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "WING_DRIVERS", "SECSWEEPLOC_WSECT_DRIVER", vsp::SECSWEEPLOC_WSECT_DRIVER );
    assert( r >= 0 );


    r = se->RegisterEnum( "XDDM_QUANTITY_TYPE" );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "XDDM_QUANTITY_TYPE", "XDDM_VAR", XDDM_VAR );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "XDDM_QUANTITY_TYPE", "XDDM_CONST", XDDM_CONST );
    assert( r >= 0 );


    r = se->RegisterEnum( "XSEC_CLOSE_TYPE" );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "XSEC_CLOSE_TYPE", "CLOSE_NONE", CLOSE_NONE );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "XSEC_CLOSE_TYPE", "CLOSE_SKEWLOW", CLOSE_SKEWLOW );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "XSEC_CLOSE_TYPE", "CLOSE_SKEWUP", CLOSE_SKEWUP );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "XSEC_CLOSE_TYPE", "CLOSE_SKEWBOTH", CLOSE_SKEWBOTH );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "XSEC_CLOSE_TYPE", "CLOSE_EXTRAP", CLOSE_EXTRAP );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "XSEC_CLOSE_TYPE", "CLOSE_NUM_TYPES", CLOSE_NUM_TYPES );
    assert( r >= 0 );


    r = se->RegisterEnum( "XSEC_CRV_TYPE" );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "XSEC_CRV_TYPE", "XS_UNDEFINED", XS_UNDEFINED );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "XSEC_CRV_TYPE", "XS_POINT", XS_POINT );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "XSEC_CRV_TYPE", "XS_CIRCLE", XS_CIRCLE );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "XSEC_CRV_TYPE", "XS_ELLIPSE", XS_ELLIPSE );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "XSEC_CRV_TYPE", "XS_SUPER_ELLIPSE", XS_SUPER_ELLIPSE );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "XSEC_CRV_TYPE", "XS_ROUNDED_RECTANGLE", XS_ROUNDED_RECTANGLE );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "XSEC_CRV_TYPE", "XS_GENERAL_FUSE", XS_GENERAL_FUSE );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "XSEC_CRV_TYPE", "XS_FILE_FUSE", XS_FILE_FUSE );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "XSEC_CRV_TYPE", "XS_FOUR_SERIES", XS_FOUR_SERIES );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "XSEC_CRV_TYPE", "XS_SIX_SERIES", XS_SIX_SERIES );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "XSEC_CRV_TYPE", "XS_BICONVEX", XS_BICONVEX );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "XSEC_CRV_TYPE", "XS_WEDGE", XS_WEDGE );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "XSEC_CRV_TYPE", "XS_EDIT_CURVE", XS_EDIT_CURVE );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "XSEC_CRV_TYPE", "XS_FILE_AIRFOIL", XS_FILE_AIRFOIL );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "XSEC_CRV_TYPE", "XS_CST_AIRFOIL", XS_CST_AIRFOIL );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "XSEC_CRV_TYPE", "XS_VKT_AIRFOIL", XS_VKT_AIRFOIL );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "XSEC_CRV_TYPE", "XS_FOUR_DIGIT_MOD", XS_FOUR_DIGIT_MOD );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "XSEC_CRV_TYPE", "XS_FIVE_DIGIT", XS_FIVE_DIGIT );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "XSEC_CRV_TYPE", "XS_FIVE_DIGIT_MOD", XS_FIVE_DIGIT_MOD );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "XSEC_CRV_TYPE", "XS_ONE_SIX_SERIES", XS_ONE_SIX_SERIES );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "XSEC_CRV_TYPE", "XS_AC25_773", XS_AC25_773 );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "XSEC_CRV_TYPE", "XS_NUM_TYPES", XS_NUM_TYPES );
    assert( r >= 0 );


    r = se->RegisterEnum( "XSEC_DRIVERS" );  // TODO: improve these comments
    assert( r >= 0 );
    r = se->RegisterEnumValue( "XSEC_DRIVERS", "WIDTH_XSEC_DRIVER", WIDTH_XSEC_DRIVER );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "XSEC_DRIVERS", "AREA_XSEC_DRIVER", AREA_XSEC_DRIVER );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "XSEC_DRIVERS", "HEIGHT_XSEC_DRIVER", HEIGHT_XSEC_DRIVER );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "XSEC_DRIVERS", "HWRATIO_XSEC_DRIVER", HWRATIO_XSEC_DRIVER );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "XSEC_DRIVERS", "NUM_XSEC_DRIVER", NUM_XSEC_DRIVER );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "XSEC_DRIVERS", "CIRCLE_NUM_XSEC_DRIVER", CIRCLE_NUM_XSEC_DRIVER );
    assert( r >= 0 );


    r = se->RegisterEnum( "XSEC_FLAP_TYPE" );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "XSEC_FLAP_TYPE", "FLAP_NONE", FLAP_NONE );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "XSEC_FLAP_TYPE", "FLAP_PLAIN", FLAP_PLAIN );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "XSEC_FLAP_TYPE", "FLAP_NUM_TYPES", FLAP_NUM_TYPES );
    assert( r >= 0 );


    r = se->RegisterEnum( "XSEC_SIDES_TYPE" );  // TODO: improve these comments
    assert( r >= 0 );
    r = se->RegisterEnumValue( "XSEC_SIDES_TYPE", "XSEC_BOTH_SIDES", XSEC_BOTH_SIDES );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "XSEC_SIDES_TYPE", "XSEC_LEFT_SIDE", XSEC_LEFT_SIDE );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "XSEC_SIDES_TYPE", "XSEC_RIGHT_SIDE", XSEC_RIGHT_SIDE );
    assert( r >= 0 );


    r = se->RegisterEnum( "XSEC_TRIM_TYPE" );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "XSEC_TRIM_TYPE", "TRIM_NONE", TRIM_NONE );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "XSEC_TRIM_TYPE", "TRIM_X", TRIM_X );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "XSEC_TRIM_TYPE", "TRIM_THICK", TRIM_THICK );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "XSEC_TRIM_TYPE", "TRIM_NUM_TYPES", TRIM_NUM_TYPES );
    assert( r >= 0 );


    r = se->RegisterEnum( "XSEC_TYPE" );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "XSEC_TYPE", "XSEC_FUSE", XSEC_FUSE );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "XSEC_TYPE", "XSEC_STACK", XSEC_STACK );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "XSEC_TYPE", "XSEC_WING", XSEC_WING );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "XSEC_TYPE", "XSEC_CUSTOM", XSEC_CUSTOM );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "XSEC_TYPE", "XSEC_PROP", XSEC_PROP );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "XSEC_TYPE", "XSEC_NUM_TYPES", XSEC_NUM_TYPES );
    assert( r >= 0 );


    r = se->RegisterEnum( "XSEC_WIDTH_SHIFT" ); // TODO: improve these comments
    assert( r >= 0 );
    r = se->RegisterEnumValue( "XSEC_WIDTH_SHIFT", "XS_SHIFT_LE", XS_SHIFT_LE );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "XSEC_WIDTH_SHIFT", "XS_SHIFT_MID", XS_SHIFT_MID );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "XSEC_WIDTH_SHIFT", "XS_SHIFT_TE", XS_SHIFT_TE );
    assert( r >= 0 );
    //
}

//==== Vec3d Constructors ====//
static void Vec3dDefaultConstructor( vec3d *self )
{
    new( self ) vec3d();
}
static void Vec3dCopyConstructor( const vec3d &other, vec3d *self )
{
    new( self ) vec3d( other );
}
static void Vec3dInitConstructor( double x, double y, double z, vec3d *self )
{
    new( self ) vec3d( x, y, z );
}

//==== Register Vec3d Object ====//
void ScriptMgrSingleton::RegisterVec3d( asIScriptEngine* se )
{

    //==== Register vec3d Object =====//
    int r = se->RegisterObjectType( "vec3d", sizeof( vec3d ), asOBJ_VALUE | asOBJ_POD | asOBJ_APP_CLASS_CA );
    assert( r >= 0 );

    //==== Register the vec3d Constructors  ====//
    r = se->RegisterObjectBehaviour( "vec3d", asBEHAVE_CONSTRUCT, "void f()", asFUNCTION( Vec3dDefaultConstructor ), asCALL_CDECL_OBJLAST );
    assert( r >= 0 );

    r = se->RegisterObjectBehaviour( "vec3d", asBEHAVE_CONSTRUCT, "void f(double, double, double)", asFUNCTION( Vec3dInitConstructor ), asCALL_CDECL_OBJLAST );
    assert( r >= 0 );

    r = se->RegisterObjectBehaviour( "vec3d", asBEHAVE_CONSTRUCT, "void f(const vec3d &in)", asFUNCTION( Vec3dCopyConstructor ), asCALL_CDECL_OBJLAST );
    assert( r >= 0 );

    //==== Register the vec3d Methods  ====//
    r = se->RegisterObjectMethod( "vec3d", "double& opIndex(int) const", asMETHODPR( vec3d, operator[], ( int ), double& ), asCALL_THISCALL );
    assert( r >= 0 );

    r = se->RegisterObjectMethod( "vec3d", "double x() const", asMETHOD( vec3d, x ), asCALL_THISCALL );
    assert( r >= 0 );

    r = se->RegisterObjectMethod( "vec3d", "double y() const", asMETHOD( vec3d, y ), asCALL_THISCALL );
    assert( r >= 0 );

    r = se->RegisterObjectMethod( "vec3d", "double z() const", asMETHOD( vec3d, z ), asCALL_THISCALL );
    assert( r >= 0 );

    r = se->RegisterObjectMethod( "vec3d", "vec3d& set_xyz(double x, double y, double z)", asMETHOD( vec3d, set_xyz ), asCALL_THISCALL );
    assert( r >= 0 );

    r = se->RegisterObjectMethod( "vec3d", "vec3d& set_x(double x)", asMETHOD( vec3d, set_x ), asCALL_THISCALL );
    assert( r >= 0 );

    r = se->RegisterObjectMethod( "vec3d", "vec3d& set_y(double y)", asMETHOD( vec3d, set_y ), asCALL_THISCALL );
    assert( r >= 0 );

    r = se->RegisterObjectMethod( "vec3d", "vec3d& set_z(double z)", asMETHOD( vec3d, set_z ), asCALL_THISCALL );
    assert( r >= 0 );

    r = se->RegisterObjectMethod( "vec3d", "void rotate_x(double theta)", asMETHOD( vec3d, rotate_x ), asCALL_THISCALL );
    assert( r >= 0 );

    r = se->RegisterObjectMethod( "vec3d", "void rotate_y(double theta)", asMETHOD( vec3d, rotate_y ), asCALL_THISCALL );
    assert( r >= 0 );

    r = se->RegisterObjectMethod( "vec3d", "void rotate_z(double theta)", asMETHOD( vec3d, rotate_z ), asCALL_THISCALL );
    assert( r >= 0 );

    r = se->RegisterObjectMethod( "vec3d", "void scale_x(double scale)", asMETHOD( vec3d, scale_x ), asCALL_THISCALL );
    assert( r >= 0 );

    r = se->RegisterObjectMethod( "vec3d", "void scale_y(double scale)", asMETHOD( vec3d, scale_y ), asCALL_THISCALL );
    assert( r >= 0 );

    r = se->RegisterObjectMethod( "vec3d", "void scale_z(double scale)", asMETHOD( vec3d, scale_z ), asCALL_THISCALL );
    assert( r >= 0 );

    r = se->RegisterObjectMethod( "vec3d", "void offset_x(double offset)", asMETHOD( vec3d, offset_x ), asCALL_THISCALL );
    assert( r >= 0 );

    r = se->RegisterObjectMethod( "vec3d", "void offset_y(double offset)", asMETHOD( vec3d, offset_y ), asCALL_THISCALL );
    assert( r >= 0 );

    r = se->RegisterObjectMethod( "vec3d", "void offset_z(double offset)", asMETHOD( vec3d, offset_z ), asCALL_THISCALL );
    assert( r >= 0 );

    r = se->RegisterObjectMethod( "vec3d", "vec3d reflect_xy()", asMETHOD( vec3d, reflect_xy ), asCALL_THISCALL );
    assert( r >= 0 );

    r = se->RegisterObjectMethod( "vec3d", "vec3d reflect_xz()", asMETHOD( vec3d, reflect_xz ), asCALL_THISCALL );
    assert( r >= 0 );

    r = se->RegisterObjectMethod( "vec3d", "vec3d reflect_yz()", asMETHOD( vec3d, reflect_yz ), asCALL_THISCALL );
    assert( r >= 0 );

    r = se->RegisterObjectMethod( "vec3d", "vec3d opAdd(const vec3d &in) const", asFUNCTIONPR( operator+, ( const vec3d&, const vec3d& ), vec3d ), asCALL_CDECL_OBJFIRST );
    assert( r >= 0 );

    r = se->RegisterObjectMethod( "vec3d", "vec3d opSub(const vec3d &in) const", asFUNCTIONPR( operator-, ( const vec3d&, const vec3d& ), vec3d ), asCALL_CDECL_OBJFIRST );
    assert( r >= 0 );

    r = se->RegisterObjectMethod( "vec3d", "vec3d opMul(double b) const", asFUNCTIONPR( operator*, ( const vec3d & a, double b ), vec3d ), asCALL_CDECL_OBJFIRST );
    assert( r >= 0 );


    r = se->RegisterObjectMethod( "vec3d", "vec3d opMul_r(const vec3d &in) const", asFUNCTIONPR( operator*, ( const vec3d&, const vec3d& ), vec3d ), asCALL_CDECL_OBJFIRST );
    assert( r >= 0 );


    r = se->RegisterObjectMethod( "vec3d", "vec3d opDiv(double b) const", asFUNCTIONPR( operator/, ( const vec3d&, double b ), vec3d ), asCALL_CDECL_OBJFIRST );
    assert( r >= 0 );


    r = se->RegisterObjectMethod( "vec3d", "double mag() const", asMETHOD( vec3d, mag ), asCALL_THISCALL );
    assert( r >= 0 );


    r = se->RegisterObjectMethod( "vec3d", "void normalize()", asMETHOD( vec3d, normalize ), asCALL_THISCALL );
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "double dist(const vec3d& in a, const vec3d& in b)", asFUNCTIONPR( dist, ( const vec3d&, const vec3d& ), double  ), asCALL_CDECL );
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "double dist_squared(const vec3d& in a, const vec3d& in b)", asFUNCTIONPR( dist_squared, ( const vec3d&, const vec3d& ), double  ), asCALL_CDECL );
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "double dot(const vec3d& in a, const vec3d& in b)", asFUNCTIONPR( dot, ( const vec3d&, const vec3d& ), double  ), asCALL_CDECL );
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "vec3d cross(const vec3d& in a, const vec3d& in b)", asFUNCTIONPR( cross, ( const vec3d&, const vec3d& ), vec3d  ), asCALL_CDECL );
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "double angle(const vec3d& in a, const vec3d& in b)", asFUNCTIONPR( angle, ( const vec3d&, const vec3d& ), double  ), asCALL_CDECL );
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "double signed_angle(const vec3d& in a, const vec3d& in b, const vec3d& in ref )", asFUNCTIONPR( signed_angle, ( const vec3d & a, const vec3d & b, const vec3d & ref ), double  ), asCALL_CDECL );
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "double cos_angle(const vec3d& in a, const vec3d& in b )", asFUNCTIONPR( cos_angle, ( const vec3d & a, const vec3d & b ), double  ), asCALL_CDECL );
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "vec3d RotateArbAxis(const vec3d& in p, double theta, const vec3d& in axis )", asFUNCTIONPR( RotateArbAxis, ( const vec3d & p, double theta, const vec3d & axis ), vec3d ), asCALL_CDECL );
    assert( r >= 0 );


}

//==== Matrix4d Constructors ====//
static void Matrix4dDefaultConstructor( Matrix4d *self )
{
    new( self ) Matrix4d();
}

//==== Register Matrix4d Object ====//
void ScriptMgrSingleton::RegisterMatrix4d( asIScriptEngine* se )
{
    //==== Register Matrix4d Object =====//


    int r = se->RegisterObjectType( "Matrix4d", sizeof( Matrix4d ), asOBJ_VALUE | asOBJ_POD | asOBJ_APP_CLASS_CA );
    assert( r >= 0 );

    //===== Register the Matrix4d constructor =====//
    r = se->RegisterObjectBehaviour( "Matrix4d", asBEHAVE_CONSTRUCT, "void f()", asFUNCTION( Matrix4dDefaultConstructor ), asCALL_CDECL_OBJLAST );
    assert( r >= 0 ); // TODO?

    //===== Register the Matrix4d methods =====//


    r = se->RegisterObjectMethod( "Matrix4d", "void loadIdentity()", asMETHOD( Matrix4d, loadIdentity ), asCALL_THISCALL );
    assert( r >= 0 );


    r = se->RegisterObjectMethod( "Matrix4d", "void translatef( const double & in x, const double & in y, const double & in z)", asMETHOD( Matrix4d, translatef ), asCALL_THISCALL );
    assert( r >= 0 );


    r = se->RegisterObjectMethod( "Matrix4d", "void rotateX( const double & in ang )", asMETHOD( Matrix4d, rotateX ), asCALL_THISCALL );
    assert( r >= 0 );


    r = se->RegisterObjectMethod( "Matrix4d", "void rotateY( const double & in ang )", asMETHOD( Matrix4d, rotateY ), asCALL_THISCALL );
    assert( r >= 0 );


    r = se->RegisterObjectMethod( "Matrix4d", "void rotateZ( const double & in ang )", asMETHOD( Matrix4d, rotateZ ), asCALL_THISCALL );
    assert( r >= 0 );


    r = se->RegisterObjectMethod( "Matrix4d", "void rotate( const double & in ang, const vec3d & in axis )", asMETHOD( Matrix4d, rotate ), asCALL_THISCALL );
    assert( r >= 0 );


    r = se->RegisterObjectMethod( "Matrix4d", "void scale( const double & in scale )", asMETHOD( Matrix4d, scale ), asCALL_THISCALL );
    assert( r >= 0 );


    r = se->RegisterObjectMethod( "Matrix4d", "vec3d xform( const vec3d & in v )", asMETHOD( Matrix4d, xform ), asCALL_THISCALL );
    assert( r >= 0 );


    r = se->RegisterObjectMethod( "Matrix4d", "vec3d getAngles()", asMETHOD( Matrix4d, getAngles ), asCALL_THISCALL );
    assert( r >= 0 );


    r = se->RegisterObjectMethod( "Matrix4d", "void loadXZRef()", asMETHOD( Matrix4d, loadXZRef ), asCALL_THISCALL );
    assert( r >= 0 );


    r = se->RegisterObjectMethod( "Matrix4d", "void loadXYRef()", asMETHOD( Matrix4d, loadXYRef ), asCALL_THISCALL );
    assert( r >= 0 );


    r = se->RegisterObjectMethod( "Matrix4d", "void loadYZRef()", asMETHOD( Matrix4d, loadYZRef ), asCALL_THISCALL );
    assert( r >= 0 );


    r = se->RegisterObjectMethod( "Matrix4d", "void affineInverse()", asMETHOD( Matrix4d, affineInverse ), asCALL_THISCALL );
    assert( r >= 0 );


    r = se->RegisterObjectMethod("Matrix4d", "void buildXForm( const vec3d & in pos, const vec3d & in rot, const vec3d & in cent_rot )", asMETHOD(Matrix4d, buildXForm), asCALL_THISCALL );
    assert( r >= 0 ); // TODO: Example

    //TODO: Expose additional functions to the API (i.e. matMult)

}

//==== Register Custom Geom Mgr Object ====//
void ScriptMgrSingleton::RegisterCustomGeomMgr( asIScriptEngine* se )
{

    int r;
    r = se->RegisterGlobalFunction( "string AddParm( int type, const string & in name, const string & in group )",
                                    asMETHOD( CustomGeomMgrSingleton, AddParm ), asCALL_THISCALL_ASGLOBAL, &CustomGeomMgr );
    assert( r );


    r = se->RegisterGlobalFunction( "string GetCurrCustomGeom()",
                                    asMETHOD( CustomGeomMgrSingleton, GetCurrCustomGeom ), asCALL_THISCALL_ASGLOBAL, &CustomGeomMgr );
    assert( r );


    r = se->RegisterGlobalFunction( "string GetCustomParm( int index )",
                                    asMETHOD( CustomGeomMgrSingleton, GetCustomParm ), asCALL_THISCALL_ASGLOBAL, &CustomGeomMgr );
    assert( r ); // TODO: Example


    r = se->RegisterGlobalFunction( "int AddGui( int type, const string & in label = string(), const string & in parm_name = string(), const string & in group_name = string(), double range = 10.0 )",
                                    asMETHOD( CustomGeomMgrSingleton, AddGui ), asCALL_THISCALL_ASGLOBAL, &CustomGeomMgr );
    assert( r );


    r = se->RegisterGlobalFunction( "void UpdateGui( int gui_id, const string & in parm_id )",
                                    asMETHOD( CustomGeomMgrSingleton, AddUpdateGui ), asCALL_THISCALL_ASGLOBAL, &CustomGeomMgr );
    assert( r );


    r = se->RegisterGlobalFunction( "string AddXSecSurf()",
                                    asMETHOD( CustomGeomMgrSingleton, AddXSecSurf ), asCALL_THISCALL_ASGLOBAL, &CustomGeomMgr );
    assert( r );


    r = se->RegisterGlobalFunction( "void RemoveXSecSurf(const string & in xsec_id)",
                                    asMETHOD( CustomGeomMgrSingleton, RemoveXSecSurf ), asCALL_THISCALL_ASGLOBAL, &CustomGeomMgr );
    assert( r );


    r = se->RegisterGlobalFunction( "void ClearXSecSurfs()",
                                    asMETHOD( CustomGeomMgrSingleton, ClearXSecSurfs ), asCALL_THISCALL_ASGLOBAL, &CustomGeomMgr );
    assert( r );


    r = se->RegisterGlobalFunction( "void SkinXSecSurf( bool closed_flag = false )",
                                    asMETHOD( CustomGeomMgrSingleton, SkinXSecSurf ), asCALL_THISCALL_ASGLOBAL, &CustomGeomMgr );
    assert( r );


    r = se->RegisterGlobalFunction( "void CloneSurf(int index, Matrix4d & in mat)",
                                    asMETHOD( CustomGeomMgrSingleton, CloneSurf ), asCALL_THISCALL_ASGLOBAL, &CustomGeomMgr );
    assert( r );


    r = se->RegisterGlobalFunction( "void TransformSurf(int index, Matrix4d & in mat)",
                                    asMETHOD( CustomGeomMgrSingleton, TransformSurf ), asCALL_THISCALL_ASGLOBAL, &CustomGeomMgr );
    assert( r ); // TODO: Example


    r = se->RegisterGlobalFunction( "void SetVspSurfType( int type, int surf_index = -1 )",
                                    asMETHOD( CustomGeomMgrSingleton, SetVspSurfType ), asCALL_THISCALL_ASGLOBAL, &CustomGeomMgr );
    assert( r );


    r = se->RegisterGlobalFunction( "void SetVspSurfCfdType( int type, int surf_index = -1 )",
                                    asMETHOD( CustomGeomMgrSingleton, SetVspSurfCfdType ), asCALL_THISCALL_ASGLOBAL, &CustomGeomMgr );
    assert( r );


    r = se->RegisterGlobalFunction( "void SetCustomXSecLoc( const string & in xsec_id, const vec3d & in loc )",
                                    asMETHOD( CustomGeomMgrSingleton, SetCustomXSecLoc ), asCALL_THISCALL_ASGLOBAL, &CustomGeomMgr );
    assert( r );


    r = se->RegisterGlobalFunction( "vec3d GetCustomXSecLoc( const string & in xsec_id )",
                                    asMETHOD( CustomGeomMgrSingleton, GetCustomXSecLoc ), asCALL_THISCALL_ASGLOBAL, &CustomGeomMgr );
    assert( r );


    r = se->RegisterGlobalFunction( "void SetCustomXSecRot( const string & in xsec_id, const vec3d & in rot )",
                                    asMETHOD( CustomGeomMgrSingleton, SetCustomXSecRot ), asCALL_THISCALL_ASGLOBAL, &CustomGeomMgr );
    assert( r );


    r = se->RegisterGlobalFunction( "vec3d GetCustomXSecRot( const string & in xsec_id )",
                                    asMETHOD( CustomGeomMgrSingleton, GetCustomXSecRot ), asCALL_THISCALL_ASGLOBAL, &CustomGeomMgr );
    assert( r );


    r = se->RegisterGlobalFunction( "bool CheckClearTriggerEvent( int gui_id )",
                                    asMETHOD( CustomGeomMgrSingleton, CheckClearTriggerEvent ), asCALL_THISCALL_ASGLOBAL, &CustomGeomMgr );
    assert( r ); // TODO: Example


    r = se->RegisterGlobalFunction(
        "void SetupCustomDefaultSource( int type, int surf_index, double l1, double r1, double u1, double w1, double l2 = 0, double r2 = 0, double u2 = 0, double w2 = 0 )",
        asMETHOD( CustomGeomMgrSingleton, SetupCustomDefaultSource ), asCALL_THISCALL_ASGLOBAL, &CustomGeomMgr );
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "void ClearAllCustomDefaultSources()",
                                    asMETHOD( CustomGeomMgrSingleton, ClearAllCustomDefaultSources ), asCALL_THISCALL_ASGLOBAL, &CustomGeomMgr );
    assert( r );


    r = se->RegisterGlobalFunction( "void SetCustomCenter( double x, double y, double z )",
                                    asMETHOD( CustomGeomMgrSingleton, SetCustomCenter ), asCALL_THISCALL_ASGLOBAL, &CustomGeomMgr );
    assert( r );


    r = se->RegisterGlobalFunction( "string AppendXSec( const string & in xsec_surf_id, int type )",
                                    asMETHOD( CustomGeomMgrSingleton, AppendCustomXSec ), asCALL_THISCALL_ASGLOBAL, &CustomGeomMgr );
    assert( r );

    // WARNING: Both versions of the AppendCustomXSec must be available to avoid breaking existing CustomGeom scripts


    r = se->RegisterGlobalFunction( "string AppendCustomXSec( const string & in xsec_surf_id, int type )",
                                    asMETHOD( CustomGeomMgrSingleton, AppendCustomXSec ), asCALL_THISCALL_ASGLOBAL, &CustomGeomMgr );
    assert( r );


    r = se->RegisterGlobalFunction( "void CutCustomXSec( const string & in xsec_surf_id, int index )",
                                    asMETHOD( CustomGeomMgrSingleton, CutCustomXSec ), asCALL_THISCALL_ASGLOBAL, &CustomGeomMgr );
    assert( r ); // TODO: Example


    r = se->RegisterGlobalFunction( "void CopyCustomXSec( const string & in xsec_surf_id, int index )",
                                    asMETHOD( CustomGeomMgrSingleton, CopyCustomXSec ), asCALL_THISCALL_ASGLOBAL, &CustomGeomMgr );
    assert( r ); // TODO: Example


    r = se->RegisterGlobalFunction( "void PasteCustomXSec( const string & in xsec_surf_id, int index )",
                                    asMETHOD( CustomGeomMgrSingleton, PasteCustomXSec ), asCALL_THISCALL_ASGLOBAL, &CustomGeomMgr );
    assert( r ); // TODO: Example


    r = se->RegisterGlobalFunction( "string InsertCustomXSec( const string & in xsec_surf_id, int type, int index )",
                                    asMETHOD( CustomGeomMgrSingleton, InsertCustomXSec ), asCALL_THISCALL_ASGLOBAL, &CustomGeomMgr );
    assert( r ); // TODO: Example


}



//==== Register Adv Link Mgr Object ====//
void ScriptMgrSingleton::RegisterAdvLinkMgr( asIScriptEngine* se )
{
    int r;

    r = se->RegisterGlobalFunction( "array<string>@+ GetAdvLinkNames()", asMETHOD( ScriptMgrSingleton, GetAdvLinkNames ), asCALL_THISCALL_ASGLOBAL, &ScriptMgr);
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "int GetLinkIndex( const string & in name )", asFUNCTION( vsp::GetLinkIndex ), asCALL_CDECL );
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "void DelAdvLink( int index )", asFUNCTION( vsp::DelAdvLink ), asCALL_CDECL );
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "void DelAllAdvLinks()", asFUNCTION( vsp::DelAllAdvLinks ), asCALL_CDECL );
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "void AddAdvLink( const string & in name )", asFUNCTION( vsp::AddAdvLink ), asCALL_CDECL );
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "void AddAdvLinkInput( int index, const string & in parm_id, const string & in var_name )", asFUNCTION( vsp::AddAdvLinkInput ), asCALL_CDECL );
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "void AddAdvLinkOutput( int index, const string & in parm_id, const string & in var_name )", asFUNCTION( vsp::AddAdvLinkOutput ), asCALL_CDECL );
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "void DelAdvLinkInput( int index, const string & in var_name )", asFUNCTION( vsp::DelAdvLinkInput ), asCALL_CDECL );
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "void DelAdvLinkOutput( int index, const string & in var_name )", asFUNCTION( vsp::DelAdvLinkOutput ), asCALL_CDECL );
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "array<string>@+ GetAdvLinkInputNames( int index )", asMETHOD( ScriptMgrSingleton, GetAdvLinkInputNames ), asCALL_THISCALL_ASGLOBAL, &ScriptMgr);
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "array<string>@+ GetAdvLinkInputParms( int index )", asMETHOD( ScriptMgrSingleton, GetAdvLinkInputParms ), asCALL_THISCALL_ASGLOBAL, &ScriptMgr);
    assert( r >= 0 );



    r = se->RegisterGlobalFunction( "array<string>@+ GetAdvLinkOutputNames( int index )", asMETHOD( ScriptMgrSingleton, GetAdvLinkOutputNames ), asCALL_THISCALL_ASGLOBAL, &ScriptMgr);
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "array<string>@+ GetAdvLinkOutputParms( int index )", asMETHOD( ScriptMgrSingleton, GetAdvLinkOutputParms ), asCALL_THISCALL_ASGLOBAL, &ScriptMgr);
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "bool ValidateAdvLinkParms( int index )", asFUNCTION( vsp::ValidateAdvLinkParms ), asCALL_CDECL );
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "void SetAdvLinkCode( int index, const string & in code )", asFUNCTION( vsp::SetAdvLinkCode ), asCALL_CDECL );
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "string GetAdvLinkCode( int index )", asFUNCTION( vsp::GetAdvLinkCode ), asCALL_CDECL );
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "void SearchReplaceAdvLinkCode( int index, const string & in from, const string & in to )", asFUNCTION( vsp::SearchReplaceAdvLinkCode ), asCALL_CDECL );
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "bool BuildAdvLinkScript( int index )", asFUNCTION( vsp::BuildAdvLinkScript ), asCALL_CDECL );
    assert( r >= 0 );

//
// These methods appear to have been mistakenly exposed to the API.
//
//
//    r = se->RegisterGlobalFunction( "void AddInput( const string & in parm_id, const string & in var_name )",
//                                    asMETHOD( AdvLinkMgrSingleton, AddInput ), asCALL_THISCALL_ASGLOBAL, &AdvLinkMgr );
//    assert( r );
//
//
//    r = se->RegisterGlobalFunction( "void AddOutput( const string & in parm_id, const string & in var_name )",
//                                    asMETHOD( AdvLinkMgrSingleton, AddOutput ), asCALL_THISCALL_ASGLOBAL, &AdvLinkMgr );
//    assert( r );


    r = se->RegisterGlobalFunction( "void SetVar( const string & in var_name, double val )", asMETHOD( AdvLinkMgrSingleton, SetVar ), asCALL_THISCALL_ASGLOBAL, &AdvLinkMgr );
    assert( r );


    r = se->RegisterGlobalFunction( "double GetVar( const string & in var_name )", asMETHOD( AdvLinkMgrSingleton, GetVar ), asCALL_THISCALL_ASGLOBAL, &AdvLinkMgr );
    assert( r );
}

//==== Register API E Functions ====//
void ScriptMgrSingleton::RegisterAPIErrorObj( asIScriptEngine* se )
{

    //==== Register ErrorObj Object =====//
    int r = se->RegisterObjectType( "ErrorObj", sizeof( vsp::ErrorObj ), asOBJ_VALUE | asOBJ_POD | asOBJ_APP_CLASS_CDA );
    assert( r >= 0 );


    r = se->RegisterObjectMethod( "ErrorObj", "ERROR_CODE GetErrorCode()", asMETHOD( vsp::ErrorObj, GetErrorCode ), asCALL_THISCALL );
    assert( r >= 0 );


    r = se->RegisterObjectMethod( "ErrorObj", "string GetErrorString()", asMETHOD( vsp::ErrorObj, GetErrorString ), asCALL_THISCALL );
    assert( r >= 0 );



}


//==== Register VSP API Functions ====//
void ScriptMgrSingleton::RegisterAPI( asIScriptEngine* se )
{
    int r;

    //==== API Error Functions ====//
    // TODO: Move to ErrorObj group

    r = se->RegisterGlobalFunction( "bool GetErrorLastCallFlag()", asMETHOD( vsp::ErrorMgrSingleton, GetErrorLastCallFlag ), asCALL_THISCALL_ASGLOBAL, &vsp::ErrorMgr );
    assert( r );


    r = se->RegisterGlobalFunction( "int GetNumTotalErrors()", asMETHOD( vsp::ErrorMgrSingleton, GetNumTotalErrors ), asCALL_THISCALL_ASGLOBAL, &vsp::ErrorMgr );
    assert( r );


    r = se->RegisterGlobalFunction( "ErrorObj PopLastError()", asMETHOD( vsp::ErrorMgrSingleton, PopLastError ), asCALL_THISCALL_ASGLOBAL, &vsp::ErrorMgr );
    assert( r );


    r = se->RegisterGlobalFunction( "ErrorObj GetLastError()", asMETHOD( vsp::ErrorMgrSingleton, GetLastError ), asCALL_THISCALL_ASGLOBAL, &vsp::ErrorMgr );
    assert( r );


    r = se->RegisterGlobalFunction( "void SilenceErrors()", asMETHOD( vsp::ErrorMgrSingleton, SilenceErrors ), asCALL_THISCALL_ASGLOBAL, &vsp::ErrorMgr );
    assert( r );


    r = se->RegisterGlobalFunction( "void PrintOnErrors()", asMETHOD( vsp::ErrorMgrSingleton, PrintOnErrors ), asCALL_THISCALL_ASGLOBAL, &vsp::ErrorMgr );
    assert( r );

    //==== Visualization Functions ====//

    r = se->RegisterGlobalFunction( "void InitGUI()", asFUNCTION( vsp::InitGUI ), asCALL_CDECL );
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "void StartGUI()", asFUNCTION( vsp::StartGUI ), asCALL_CDECL );
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "void EnableStopGUIMenuItem()", asFUNCTION( vsp::EnableStopGUIMenuItem ), asCALL_CDECL );
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "void DisableStopGUIMenuItem()", asFUNCTION( vsp::DisableStopGUIMenuItem ), asCALL_CDECL );
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "void StopGUI()", asFUNCTION( vsp::StopGUI ), asCALL_CDECL );
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "void PopupMsg( const string & in msg )", asFUNCTION( vsp::PopupMsg ), asCALL_CDECL );
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "void UpdateGUI()", asFUNCTION( vsp::UpdateGUI ), asCALL_CDECL );
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "bool IsGUIBuild()", asFUNCTION( vsp::IsGUIBuild ), asCALL_CDECL );
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "void Lock()", asFUNCTION( vsp::Lock ), asCALL_CDECL );
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "void Unlock()", asFUNCTION( vsp::Unlock ), asCALL_CDECL );
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "bool IsEventLoopRunning()", asFUNCTION( vsp::IsEventLoopRunning ), asCALL_CDECL );
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "void ScreenGrab( const string & in file_name, int w, int h, bool transparentBG, bool autocrop = false )", asFUNCTION( vsp::ScreenGrab ), asCALL_CDECL );
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "void SetViewAxis( bool vaxis )", asFUNCTION( vsp::SetViewAxis ), asCALL_CDECL );
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "void SetShowBorders( bool brdr )", asFUNCTION( vsp::SetShowBorders ), asCALL_CDECL );
    assert( r >= 0 );


    r = se->RegisterGlobalFunction("void SetGeomDrawType( const string & in geom_id, int type )", asFUNCTION(vsp::SetGeomDrawType), asCALL_CDECL);
    assert( r >= 0 );


    r = se->RegisterGlobalFunction("void SetGeomWireColor( const string & in geom_id, int r, int g, int b )", asFUNCTION(vsp::SetGeomWireColor), asCALL_CDECL);
    assert( r >= 0 );


    r = se->RegisterGlobalFunction("void SetGeomDisplayType( const string & in geom_id, int type )", asFUNCTION(vsp::SetGeomDisplayType), asCALL_CDECL);
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "void SetGeomMaterialName( const string & in geom_id, const string & in name )", asFUNCTION( vsp::SetGeomMaterialName ), asCALL_CDECL );
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "void AddMaterial( const string & in name, const vec3d & in ambient, const vec3d & in diffuse, const vec3d & in specular, const vec3d & in emissive, const double & in alpha, const double & in shininess )", asFUNCTION( vsp::AddMaterial ), asCALL_CDECL );
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "array<string>@+ GetMaterialNames()", asMETHOD( ScriptMgrSingleton, GetMaterialNames ), asCALL_THISCALL_ASGLOBAL, &ScriptMgr );
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "void SetBackground( double r, double g, double b )", asFUNCTION( vsp::SetBackground ), asCALL_CDECL );
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "void SetAllViews( int view )", asFUNCTION( vsp::SetAllViews ), asCALL_CDECL );
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "void SetView( int viewport, int view )", asFUNCTION( vsp::SetView ), asCALL_CDECL );
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "void FitAllViews()", asFUNCTION( vsp::FitAllViews ), asCALL_CDECL );
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "void ResetViews()", asFUNCTION( vsp::ResetViews ), asCALL_CDECL );
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "void SetWindowLayout( int r, int c )", asFUNCTION( vsp::SetWindowLayout ), asCALL_CDECL );
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "void SetGUIElementDisable( int e, bool state )", asFUNCTION( vsp::SetGUIElementDisable ), asCALL_CDECL );
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "void SetGUIScreenDisable( int s, bool state )", asFUNCTION( vsp::SetGUIScreenDisable ), asCALL_CDECL );
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "void SetGeomScreenDisable( int s, bool state )", asFUNCTION( vsp::SetGeomScreenDisable ), asCALL_CDECL );
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "void HideScreen( int s )", asFUNCTION( vsp::HideScreen ), asCALL_CDECL );
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "void ShowScreen( int s )", asFUNCTION( vsp::ShowScreen ), asCALL_CDECL );
    assert( r >= 0 );

    //==== Vehicle Functions ====//

    r = se->RegisterGlobalFunction( "void Update( bool update_managers = true)", asFUNCTION( vsp::Update ), asCALL_CDECL );
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "void VSPExit( int error_code )", asFUNCTION( vsp::VSPExit ), asCALL_CDECL );
    assert( r >= 0 ); // TODO: Example


    r = se->RegisterGlobalFunction( "void VSPCrash( int crash_type )", asFUNCTION( vsp::VSPCrash ), asCALL_CDECL );
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "void ClearVSPModel()", asFUNCTION( vsp::ClearVSPModel ), asCALL_CDECL );
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "string GetVSPFileName()", asFUNCTION( vsp::GetVSPFileName ), asCALL_CDECL );
    assert( r >= 0 );


    //==== File I/O Functions ====//

    r = se->RegisterGlobalFunction( "void ReadVSPFile( const string & in file_name )", asFUNCTION( vsp::ReadVSPFile ), asCALL_CDECL );
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "void WriteVSPFile( const string & in file_name, int set )", asFUNCTION( vsp::WriteVSPFile ), asCALL_CDECL );
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "void SetVSP3FileName( const string & in file_name )", asFUNCTION( vsp::SetVSP3FileName ), asCALL_CDECL );
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "void InsertVSPFile( const string & in file_name, const string & in parent )", asFUNCTION( vsp::InsertVSPFile ), asCALL_CDECL );
    assert( r >= 0 ); // TODO: Example


    r = se->RegisterGlobalFunction( "string ExportFile( const string & in file_name, int thick_set, int file_type, int subsFlag = 1, int thin_set = -1, bool useMode = false, const string & in modeID = \"\" )", asFUNCTION( vsp::ExportFile ), asCALL_CDECL );
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "string ImportFile( const string & in file_name, int file_type, const string & in parent )", asFUNCTION( vsp::ImportFile ), asCALL_CDECL );
    assert( r >= 0 ); // TODO: Example


    r = se->RegisterGlobalFunction( "void SetBEMPropID( const string & in prop_id )", asFUNCTION( vsp::SetBEMPropID ), asCALL_CDECL );
    assert( r >= 0 );

    //==== Design File Functions ====//

    r = se->RegisterGlobalFunction( "void ReadApplyDESFile( const string & in file_name )", asFUNCTION( vsp::ReadApplyDESFile ), asCALL_CDECL );
    assert( r >= 0 ); // TODO: Example


    r = se->RegisterGlobalFunction( "void WriteDESFile( const string & in file_name )", asFUNCTION( vsp::WriteDESFile ), asCALL_CDECL );
    assert( r >= 0 ); // TODO: Example


    r = se->RegisterGlobalFunction( "void ReadApplyXDDMFile( const string & in file_name )", asFUNCTION( vsp::ReadApplyXDDMFile ), asCALL_CDECL );
    assert( r >= 0 ); // TODO: Example


    r = se->RegisterGlobalFunction( "void WriteXDDMFile( const string & in file_name )", asFUNCTION( vsp::WriteXDDMFile ), asCALL_CDECL );
    assert( r >= 0 ); // TODO: Example


    r = se->RegisterGlobalFunction( "int GetNumDesignVars()", asFUNCTION( vsp::GetNumDesignVars ), asCALL_CDECL );
    assert( r >= 0 ); // TODO: Example


    r = se->RegisterGlobalFunction( "void AddDesignVar( const string & in parm_id, int type )", asFUNCTION( vsp::AddDesignVar ), asCALL_CDECL );
    assert( r >= 0 ); // TODO: Example


    r = se->RegisterGlobalFunction( "void DeleteAllDesignVars()", asFUNCTION( vsp::DeleteAllDesignVars ), asCALL_CDECL );
    assert( r >= 0 ); // TODO: Example


    r = se->RegisterGlobalFunction( "string GetDesignVar( int index )", asFUNCTION( vsp::GetDesignVar ), asCALL_CDECL );
    assert( r >= 0 ); // TODO: Example


    r = se->RegisterGlobalFunction( "int GetDesignVarType( int index )", asFUNCTION( vsp::GetDesignVarType ), asCALL_CDECL );
    assert( r >= 0 ); // TODO: Example

    //==== Computations ====//

    r = se->RegisterGlobalFunction( "string ComputeMassProps( int set, int num_slices, int idir = X_DIR )", asFUNCTION( vsp::ComputeMassProps ), asCALL_CDECL );
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "string ComputeCompGeom( int set, bool half_mesh, int file_export_types )", asFUNCTION( vsp::ComputeCompGeom ), asCALL_CDECL );
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "string ComputePlaneSlice( int set, int num_slices, const vec3d & in norm, bool auto_bnd, double start_bnd = 0, double end_bnd = 0, bool measureduct = false )", asFUNCTION( vsp::ComputePlaneSlice ), asCALL_CDECL );
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "void ComputeDegenGeom( int set, int file_type )", asFUNCTION( vsp::ComputeDegenGeom ), asCALL_CDECL );
    assert( r >= 0 );

    //==== CFD Mesh ====//

    r = se->RegisterGlobalFunction( "void SetComputationFileName( int file_type, const string & in file_name )", asFUNCTION( vsp::SetComputationFileName ), asCALL_CDECL );
    assert( r >= 0 ); // TODO: FIXME for FEA Mesh


    r = se->RegisterGlobalFunction( "void ComputeCFDMesh( int set, int degenset, int file_type )", asFUNCTION( vsp::ComputeCFDMesh ), asCALL_CDECL );
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "void SetCFDMeshVal( int type, double val )", asFUNCTION( vsp::SetCFDMeshVal ), asCALL_CDECL );
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "void SetCFDWakeFlag( const string & in geom_id, bool flag )", asFUNCTION( vsp::SetCFDWakeFlag ), asCALL_CDECL );
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "void DeleteAllCFDSources()", asFUNCTION( vsp::DeleteAllCFDSources ), asCALL_CDECL );
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "void AddDefaultSources()", asFUNCTION( vsp::AddDefaultSources ), asCALL_CDECL );
    assert( r >= 0 );


    r = se->RegisterGlobalFunction(
        "void AddCFDSource( int type, const string & in geom_id, int surf_index, double l1, double r1, double u1, double w1, double l2 = 0, double r2 = 0, double u2 = 0, double w2 = 0 )",
        asFUNCTION( vsp::AddCFDSource ), asCALL_CDECL );
    assert( r >= 0 );


    //==== Analysis Functions ====//

    r = se->RegisterGlobalFunction( "int GetNumAnalysis( )", asFUNCTION( vsp::GetNumAnalysis ), asCALL_CDECL );
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "array<string>@+ ListAnalysis()", asMETHOD( ScriptMgrSingleton, ListAnalysis ), asCALL_THISCALL_ASGLOBAL, &ScriptMgr );
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "array<string>@+ GetAnalysisInputNames(const string & in analysis )", asMETHOD( ScriptMgrSingleton, GetAnalysisInputNames ), asCALL_THISCALL_ASGLOBAL, &ScriptMgr );
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "string GetAnalysisDoc( const string & in analysis )", asFUNCTION( vsp::GetAnalysisDoc ), asCALL_CDECL );
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "string GetAnalysisInputDoc( const string & in analysis, const string & in name )", asFUNCTION( vsp::GetNumAnalysisInputData ), asCALL_CDECL );
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "string ExecAnalysis( const string & in analysis )", asFUNCTION( vsp::ExecAnalysis ), asCALL_CDECL );
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "int GetNumAnalysisInputData( const string & in analysis, const string & in name )", asFUNCTION( vsp::GetNumAnalysisInputData ), asCALL_CDECL );
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "int GetAnalysisInputType( const string & in analysis, const string & in name )", asFUNCTION( vsp::GetAnalysisInputType ), asCALL_CDECL );
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "array<int>@+ GetIntAnalysisInput( const string & in analysis, const string & in name, int index = 0 )", asMETHOD( ScriptMgrSingleton, GetIntAnalysisInput ), asCALL_THISCALL_ASGLOBAL, &ScriptMgr );
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "array<double>@+ GetDoubleAnalysisInput( const string & in analysis, const string & in name, int index = 0 )", asMETHOD( ScriptMgrSingleton, GetDoubleAnalysisInput ), asCALL_THISCALL_ASGLOBAL, &ScriptMgr );
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "array<string>@+ GetStringAnalysisInput( const string & in analysis, const string & in name, int index = 0 )", asMETHOD( ScriptMgrSingleton, GetStringAnalysisInput ), asCALL_THISCALL_ASGLOBAL, &ScriptMgr );
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "array<vec3d>@+ GetVec3dAnalysisInput( const string & in analysis, const string & in name, int index = 0 )", asMETHOD( ScriptMgrSingleton, GetVec3dAnalysisInput ), asCALL_THISCALL_ASGLOBAL, &ScriptMgr );
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "void PrintAnalysisInputs( const string & in analysis )", asFUNCTION( vsp::PrintAnalysisInputs ), asCALL_CDECL );
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "void PrintAnalysisDocs( const string & in analysis )", asFUNCTION( vsp::PrintAnalysisDocs ), asCALL_CDECL );
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "void SetAnalysisInputDefaults( const string & in analysis )", asFUNCTION( vsp::SetAnalysisInputDefaults ), asCALL_CDECL );
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "void SetIntAnalysisInput( const string & in analysis, const string & in name, array<int>@+ indata_arr, int index = 0 )", asMETHOD( ScriptMgrSingleton, SetIntAnalysisInput ), asCALL_THISCALL_ASGLOBAL, &ScriptMgr );
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "void SetDoubleAnalysisInput( const string & in analysis, const string & in name, array<double>@+ indata_arr, int index = 0 )", asMETHOD( ScriptMgrSingleton, SetDoubleAnalysisInput ), asCALL_THISCALL_ASGLOBAL, &ScriptMgr );
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "void SetStringAnalysisInput( const string & in analysis, const string & in name, array<string>@+ indata_arr, int index = 0 )", asMETHOD( ScriptMgrSingleton, SetStringAnalysisInput ), asCALL_THISCALL_ASGLOBAL, &ScriptMgr );
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "void SetVec3dAnalysisInput( const string & in analysis, const string & in name, array<vec3d>@+ indata_arr, int index = 0 )", asMETHOD( ScriptMgrSingleton, SetVec3dAnalysisInput ), asCALL_THISCALL_ASGLOBAL, &ScriptMgr );
    assert( r >= 0 );


    //==== Results Functions ====//

    r = se->RegisterGlobalFunction( "int GetNumResults( const string & in name )", asFUNCTION( vsp::GetNumResults ), asCALL_CDECL );
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "string GetResultsName( const string & in results_id )", asFUNCTION( vsp::GetResultsName ), asCALL_CDECL );
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "string GetResultsSetDoc( const string & in results_id )",
                                    asFUNCTION( vsp::GetResultsSetDoc ), asCALL_CDECL );
    assert( r >= 0 );


    r = se->RegisterGlobalFunction(
            "string GetResultsEntryDoc( const string & in results_id, const string & in data_name )",
            asFUNCTION( vsp::GetResultsSetDoc ), asCALL_CDECL );
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "string FindResultsID( const string & in name, int index = 0 )", asFUNCTION( vsp::FindResultsID ), asCALL_CDECL );
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "string FindLatestResultsID( const string & in name )", asFUNCTION( vsp::FindLatestResultsID ), asCALL_CDECL );
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "int GetNumData( const string & in results_id, const string & in data_name )", asFUNCTION( vsp::GetNumData ), asCALL_CDECL );
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "int GetResultsType( const string & in results_id, const string & in data_name )", asFUNCTION( vsp::GetResultsType ), asCALL_CDECL );
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "string SummarizeAttributes()", asFUNCTION( vsp::SummarizeAttributes ), asCALL_CDECL );
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "string SummarizeAttributesAsTree()", asFUNCTION( vsp::SummarizeAttributesAsTree ), asCALL_CDECL );
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "array<string>@+ FindAllAttributes()", asMETHOD( ScriptMgrSingleton, FindAllAttributes ), asCALL_THISCALL_ASGLOBAL, &ScriptMgr);
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "array<string>@+ FindAttributesByName( const string & in search_str )", asMETHOD( ScriptMgrSingleton, FindAttributesByName ), asCALL_THISCALL_ASGLOBAL, &ScriptMgr);
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "string FindAttributeByName( const string & in search_str, int index )", asFUNCTION( vsp::FindAttributeByName ), asCALL_CDECL );
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "string FindAttributeInCollection( const string & in coll_id, const string & in search_str, int index )", asFUNCTION( vsp::FindAttributeInCollection ), asCALL_CDECL );
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "array<string>@+ FindAttributeNamesInCollection( const string & in collID )", asMETHOD( ScriptMgrSingleton, FindAttributeNamesInCollection ), asCALL_THISCALL_ASGLOBAL, &ScriptMgr);
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "array<string>@+ FindAttributesInCollection( const string & in collID )", asMETHOD( ScriptMgrSingleton, FindAttributesInCollection ), asCALL_THISCALL_ASGLOBAL, &ScriptMgr);
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "array<string>@+ FindAttributedObjects()", asMETHOD( ScriptMgrSingleton, FindAttributedObjects ), asCALL_THISCALL_ASGLOBAL, &ScriptMgr);
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "int GetObjectType( const string & in id )", asFUNCTION( vsp::GetObjectType ), asCALL_CDECL );
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "string GetObjectTypeName( const string & in id )", asFUNCTION( vsp::GetObjectTypeName ), asCALL_CDECL );
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "string GetObjectName( const string & in attachID )", asFUNCTION( vsp::GetObjectName ), asCALL_CDECL );
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "string GetObjectParent( const string & in id )", asFUNCTION( vsp::GetObjectParent ), asCALL_CDECL );
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "string GetChildCollection( const string & in attachID )", asFUNCTION( vsp::GetChildCollection ), asCALL_CDECL );
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "string GetGeomSetCollection( const int & in index )", asFUNCTION( vsp::GetGeomSetCollection ), asCALL_CDECL );
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "string GetAttributeName( const string & in attrID )", asFUNCTION( vsp::GetAttributeName ), asCALL_CDECL );
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "string GetAttributeID( const string & in collID, const string & in attributeName, int index )", asFUNCTION( vsp::GetAttributeID ), asCALL_CDECL );
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "string GetAttributeDoc( const string & in attrID )", asFUNCTION( vsp::GetAttributeDoc ), asCALL_CDECL );
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "int GetAttributeType( const string & in attrID )", asFUNCTION( vsp::GetAttributeType ), asCALL_CDECL );
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "string GetAttributeTypeName( const string & in attrID )", asFUNCTION( vsp::GetAttributeTypeName ), asCALL_CDECL );
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "array<bool>@+ GetAttributeBoolVal( const string & in attrID )", asMETHOD( ScriptMgrSingleton, GetAttributeBoolVal ), asCALL_THISCALL_ASGLOBAL, &ScriptMgr);
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "array<int>@+ GetAttributeIntVal( const string & in attrID )", asMETHOD( ScriptMgrSingleton, GetAttributeIntVal ), asCALL_THISCALL_ASGLOBAL, &ScriptMgr);
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "array<double>@+ GetAttributeDoubleVal( const string & in attrID )", asMETHOD( ScriptMgrSingleton, GetAttributeDoubleVal ), asCALL_THISCALL_ASGLOBAL, &ScriptMgr);
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "array<string>@+ GetAttributeStringVal( const string & in attrID )", asMETHOD( ScriptMgrSingleton, GetAttributeStringVal ), asCALL_THISCALL_ASGLOBAL, &ScriptMgr);
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "array<string>@+ GetAttributeParmID( const string & in attrID )", asMETHOD( ScriptMgrSingleton, GetAttributeParmID ), asCALL_THISCALL_ASGLOBAL, &ScriptMgr);
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "array<double>@+ GetAttributeParmVal( const string & in attrID )", asMETHOD( ScriptMgrSingleton, GetAttributeParmVal ), asCALL_THISCALL_ASGLOBAL, &ScriptMgr);
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "array<string>@+ GetAttributeParmName( const string & in attrID )", asMETHOD( ScriptMgrSingleton, GetAttributeParmName ), asCALL_THISCALL_ASGLOBAL, &ScriptMgr);
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "array<vec3d>@+ GetAttributeVec3dVal( const string & in attrID )", asMETHOD( ScriptMgrSingleton, GetAttributeVec3dVal ), asCALL_THISCALL_ASGLOBAL, &ScriptMgr);
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "array<array<int>>@+ GetAttributeIntMatrixVal( const string & in attrID )", asMETHOD( ScriptMgrSingleton, GetAttributeIntMatrixVal ), asCALL_THISCALL_ASGLOBAL, &ScriptMgr);
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "array<array<double>>@+ GetAttributeDoubleMatrixVal( const string & in attrID )", asMETHOD( ScriptMgrSingleton, GetAttributeDoubleMatrixVal ), asCALL_THISCALL_ASGLOBAL, &ScriptMgr);
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "void SetAttributeName( const string & in attrID, const string & in name )", asFUNCTION( vsp::SetAttributeName ), asCALL_CDECL );
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "void SetAttributeDoc( const string & in attrID, const string & in doc )", asFUNCTION( vsp::SetAttributeDoc ), asCALL_CDECL );
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "void SetAttributeBool( const string & in attrID, bool value )", asFUNCTION( vsp::SetAttributeBool ), asCALL_CDECL );
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "void SetAttributeInt( const string & in attrID, int value )", asFUNCTION( vsp::SetAttributeInt ), asCALL_CDECL );
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "void SetAttributeDouble( const string & in attrID, double value )", asFUNCTION( vsp::SetAttributeDouble ), asCALL_CDECL );
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "void SetAttributeString( const string & in attrID, const string & in value )", asFUNCTION( vsp::SetAttributeString ), asCALL_CDECL );
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "void SetAttributeParmID( const string & in attrID, const string & in value )", asFUNCTION( vsp::SetAttributeParmID ), asCALL_CDECL );
    assert( r >= 0 );

    // check if we need @+ on the input- may be bad idea
    r = se->RegisterGlobalFunction( "void SetAttributeVec3d( const string & in attrID, array<vec3d>@+ value )", asMETHOD( ScriptMgrSingleton, SetAttributeVec3d ), asCALL_THISCALL_ASGLOBAL, &ScriptMgr);
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "void SetAttributeIntMatrix( const string & in attrID, array<array<int>>@+ value )", asMETHOD( ScriptMgrSingleton, SetAttributeIntMatrix ), asCALL_THISCALL_ASGLOBAL, &ScriptMgr);
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "void SetAttributeDoubleMatrix( const string & in attrID, array<array<double>>@+ value )", asMETHOD( ScriptMgrSingleton, SetAttributeDoubleMatrix ), asCALL_THISCALL_ASGLOBAL, &ScriptMgr);
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "void DeleteAttribute( const string & in attrID )", asFUNCTION( vsp::DeleteAttribute ), asCALL_CDECL );
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "string AddAttributeBool( const string & in collID, const string & in attributeName, bool value )", asFUNCTION( vsp::AddAttributeBool ), asCALL_CDECL );
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "string AddAttributeInt( const string & in collID, const string & in attributeName, int value )", asFUNCTION( vsp::AddAttributeInt ), asCALL_CDECL );
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "string AddAttributeDouble( const string & in collID, const string & in attributeName, double value )", asFUNCTION( vsp::AddAttributeDouble ), asCALL_CDECL );
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "string AddAttributeString( const string & in collID, const string & in attributeName, const string & in value )", asFUNCTION( vsp::AddAttributeString ), asCALL_CDECL );
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "string AddAttributeParm( const string & in collID, const string & in attributeName, const string & in value )", asFUNCTION( vsp::AddAttributeParm ), asCALL_CDECL );
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "string AddAttributeVec3d( const string & in attrID, const string & in attributeName, const array<vec3d>@+ value )", asMETHOD( ScriptMgrSingleton, AddAttributeVec3d ), asCALL_THISCALL_ASGLOBAL, &ScriptMgr);
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "string AddAttributeIntMatrix( const string & in collID, const string & in attributeName, const array<array<int>>@+ value)", asMETHOD( ScriptMgrSingleton, AddAttributeIntMatrix ), asCALL_THISCALL_ASGLOBAL, &ScriptMgr);
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "string AddAttributeDoubleMatrix( const string & in collID, const string & in attributeName, const array<array<double>>@+ value)", asMETHOD( ScriptMgrSingleton, AddAttributeDoubleMatrix ), asCALL_THISCALL_ASGLOBAL, &ScriptMgr);
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "string AddAttributeGroup( const string & in collID, const string & in attributeName )", asFUNCTION( vsp::AddAttributeGroup ), asCALL_CDECL );
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "int CopyAttribute( const string & in attr_id )", asFUNCTION( vsp::CopyAttribute ), asCALL_CDECL );
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "void CutAttribute( const string & in attr_id )", asFUNCTION( vsp::CutAttribute ), asCALL_CDECL );
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "array<string>@+ PasteAttribute( const string & in coll_id )", asMETHOD( ScriptMgrSingleton, PasteAttribute ), asCALL_THISCALL_ASGLOBAL, &ScriptMgr );
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "array<string>@+ GetAllResultsNames()", asMETHOD( ScriptMgrSingleton, GetAllResultsNames ), asCALL_THISCALL_ASGLOBAL, &ScriptMgr );
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "array<string>@+ GetAllDataNames(const string & in results_id )", asMETHOD( ScriptMgrSingleton, GetAllDataNames ), asCALL_THISCALL_ASGLOBAL, &ScriptMgr );
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "array<int>@+ GetIntResults( const string & in id, const string & in name, int index = 0 )", asMETHOD( ScriptMgrSingleton, GetIntResults ), asCALL_THISCALL_ASGLOBAL, &ScriptMgr );
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "array<double>@+ GetDoubleResults( const string & in id, const string & in name, int index = 0 )", asMETHOD( ScriptMgrSingleton, GetDoubleResults ), asCALL_THISCALL_ASGLOBAL, &ScriptMgr );
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "array<array<double>@>@+ GetDoubleMatResults( const string & in id, const string & in name, int index = 0 )", asMETHOD( ScriptMgrSingleton, GetDoubleMatResults ), asCALL_THISCALL_ASGLOBAL, &ScriptMgr );
    assert( r >= 0 ); // TODO: Example


    r = se->RegisterGlobalFunction( "array<string>@+ GetStringResults( const string & in id, const string & in name, int index = 0 )", asMETHOD( ScriptMgrSingleton, GetStringResults ), asCALL_THISCALL_ASGLOBAL, &ScriptMgr );
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "array<vec3d>@+ GetVec3dResults( const string & in id, const string & in name, int index = 0 )", asMETHOD( ScriptMgrSingleton, GetVec3dResults ), asCALL_THISCALL_ASGLOBAL, &ScriptMgr );
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "string CreateGeomResults( const string & in geom_id, const string & in name )", asFUNCTION( vsp::CreateGeomResults ), asCALL_CDECL );
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "void DeleteAllResults()", asFUNCTION( vsp::DeleteAllResults ), asCALL_CDECL );
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "void DeleteResult( const string & in id )", asFUNCTION( vsp::DeleteResult ), asCALL_CDECL );
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "void WriteResultsCSVFile( const string & in id, const string & in file_name )", asFUNCTION( vsp::WriteResultsCSVFile ), asCALL_CDECL );
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "void PrintResults( const string & in id )", asFUNCTION( vsp::PrintResults ), asCALL_CDECL );
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "void PrintResultsDoc( const string & in id )", asFUNCTION( vsp::PrintResultsDocs ), asCALL_CDECL );
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "void WriteTestResults()", asMETHOD( ResultsMgrSingleton, WriteTestResults ), asCALL_THISCALL_ASGLOBAL, &ResultsMgr );
    assert( r >= 0 );


    //==== Geom Functions ====//

    r = se->RegisterGlobalFunction( "array<string>@+ GetGeomTypes()", asMETHOD( ScriptMgrSingleton, GetGeomTypes ), asCALL_THISCALL_ASGLOBAL, &ScriptMgr );
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "string AddGeom( const string & in type, const string & in parent = string() )", asFUNCTION( vsp::AddGeom ), asCALL_CDECL );
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "void UpdateGeom(const string & in geom_id)", asFUNCTION( vsp::UpdateGeom ), asCALL_CDECL );
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "void DeleteGeom(const string & in geom_id)", asFUNCTION( vsp::DeleteGeom ), asCALL_CDECL );
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "void DeleteGeomVec( array<string>@+ del_arr )", asMETHOD( ScriptMgrSingleton, DeleteGeomVec ), asCALL_THISCALL_ASGLOBAL, &ScriptMgr );
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "void CutGeomToClipboard(const string & in geom_id)", asFUNCTION( vsp::CutGeomToClipboard ), asCALL_CDECL );
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "void CopyGeomToClipboard(const string & in geom_id)", asFUNCTION( vsp::CopyGeomToClipboard ), asCALL_CDECL );
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "array<string>@+ PasteGeomClipboard( const string & in parent_id = \"\" )", asMETHOD( ScriptMgrSingleton, PasteGeomClipboard ), asCALL_THISCALL_ASGLOBAL, &ScriptMgr );
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "array<string>@+ FindGeoms()", asMETHOD( ScriptMgrSingleton, FindGeoms ), asCALL_THISCALL_ASGLOBAL, &ScriptMgr );
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "array<string>@+ FindGeomsWithName(const string & in name)", asMETHOD( ScriptMgrSingleton, FindGeomsWithName ), asCALL_THISCALL_ASGLOBAL, &ScriptMgr );
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "string FindGeom(const string & in name, int index)", asFUNCTION( vsp::FindGeom ), asCALL_CDECL );
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "void SetGeomName( const string & in geom_id, const string & in name )", asFUNCTION( vsp::SetGeomName ), asCALL_CDECL );
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "string GetGeomName( const string & in geom_id )", asFUNCTION( vsp::GetGeomName ), asCALL_CDECL );
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "array<string>@+ GetGeomParmIDs(const string & in geom_id )", asMETHOD( ScriptMgrSingleton, GetGeomParmIDs ), asCALL_THISCALL_ASGLOBAL, &ScriptMgr );
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "int GetGeomVSPSurfCfdType( const string & in geom_id, int main_surf_ind = 0 )", asFUNCTION( vsp::GetGeomVSPSurfCfdType ), asCALL_CDECL );
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "int GetGeomVSPSurfType( const string & in geom_id, int main_surf_ind = 0 )", asFUNCTION( vsp::GetGeomVSPSurfType ), asCALL_CDECL );
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "string GetGeomTypeName(const string & in geom_id )", asFUNCTION( vsp::GetGeomTypeName ), asCALL_CDECL );
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "int GetNumMainSurfs( const string & in geom_id )", asFUNCTION( vsp::GetNumMainSurfs ), asCALL_CDECL );
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "int GetTotalNumSurfs( const string & in geom_id )", asFUNCTION( vsp::GetTotalNumSurfs ), asCALL_CDECL );
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "vec3d GetGeomBBoxMax( const string & in geom_id, int main_surf_ind = 0, bool ref_frame_is_absolute = true )", asFUNCTION( vsp::GetGeomBBoxMax ), asCALL_CDECL );
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "vec3d GetGeomBBoxMin( const string & in geom_id, int main_surf_ind = 0, bool ref_frame_is_absolute = true )", asFUNCTION( vsp::GetGeomBBoxMin ), asCALL_CDECL );
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "void SetGeomParent( const string & in geom_id, const string & in parent_id )", asFUNCTION( vsp::SetGeomParent ), asCALL_CDECL );
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "string GetGeomParent( const string & in geom_id )", asFUNCTION( vsp::GetGeomParent ), asCALL_CDECL );
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "array<string>@+ GetGeomChildren( const string & in geom_id )", asMETHOD( ScriptMgrSingleton, GetGeomChildren ), asCALL_THISCALL_ASGLOBAL, &ScriptMgr );
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "void SplitWingXSec( const string & in wing_id, int section_index )", asFUNCTION( vsp::SplitWingXSec ), asCALL_CDECL );
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "void SetDriverGroup( const string & in geom_id, int section_index, int driver_0, int driver_1 = -1, int driver_2 = -1)", asFUNCTION( vsp::SetDriverGroup ), asCALL_CDECL );
    assert( r >= 0 );

    //==== SubSurface Functions ====//

    r = se->RegisterGlobalFunction( "string AddSubSurf( const string & in geom_id, int type, int surfindex = 0 )", asFUNCTION( vsp::AddSubSurf ), asCALL_CDECL );
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "void DeleteSubSurf( const string & in geom_id, const string & in sub_id )", asFUNCTIONPR( vsp::DeleteSubSurf, ( const string &, const string & ), void ), asCALL_CDECL );
    assert( r >= 0 );
    // TODO: Why are there two DeleteSubSurf if Geom ID isn't needed?

    r = se->RegisterGlobalFunction( "void DeleteSubSurf( const string & in sub_id )", asFUNCTIONPR( vsp::DeleteSubSurf, ( const string & ), void ), asCALL_CDECL );
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "string GetSubSurf( const string & in geom_id, int index )", asFUNCTIONPR( vsp::GetSubSurf, ( const string &, int ), string ), asCALL_CDECL );
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "array<string>@+ GetSubSurf( const string & in geom_id, const string & in name )", asMETHOD( ScriptMgrSingleton, GetSubSurf ), asCALL_THISCALL_ASGLOBAL, &ScriptMgr );
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "void SetSubSurfName( const string & in geom_id, const string & in sub_id, const string & in name )", asFUNCTIONPR( vsp::SetSubSurfName, ( const string &, const string &, const string & ), void ), asCALL_CDECL );
    assert( r >= 0 );
    // TODO: Why are there two SetSubSurfName if Geom ID isn't needed?

    r = se->RegisterGlobalFunction( "void SetSubSurfName( const string & in sub_id, const string & in name )", asFUNCTIONPR( vsp::SetSubSurfName, ( const string &, const string & ), void ), asCALL_CDECL );
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "string GetSubSurfName( const string & in geom_id, const string & in sub_id )", asFUNCTIONPR( vsp::GetSubSurfName, ( const string &, const string & ), string ), asCALL_CDECL );
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "string GetSubSurfName( const string & in sub_id )", asFUNCTIONPR( vsp::GetSubSurfName, ( const string & ), string ), asCALL_CDECL );
    assert( r >= 0 );
    // TODO: Why are there two GetSubSurfName if Geom ID isn't needed?

    r = se->RegisterGlobalFunction( "int GetSubSurfIndex( const string & in sub_id )", asFUNCTION( vsp::GetSubSurfIndex ), asCALL_CDECL );
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "array<string>@+ GetSubSurfIDVec( const string & in geom_id )", asMETHOD( ScriptMgrSingleton, GetSubSurfIDVec ), asCALL_THISCALL_ASGLOBAL, &ScriptMgr );
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "array<string>@+ GetAllSubSurfIDs()", asMETHOD( ScriptMgrSingleton, GetAllSubSurfIDs ), asCALL_THISCALL_ASGLOBAL, &ScriptMgr );
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "int GetNumSubSurf( const string & in geom_id )", asFUNCTION( vsp::GetNumSubSurf ), asCALL_CDECL );
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "int GetSubSurfType( const string & in sub_id )", asFUNCTION( vsp::GetSubSurfType ), asCALL_CDECL );
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "array<string>@+ GetSubSurfParmIDs(const string & in sub_id )", asMETHOD( ScriptMgrSingleton, GetSubSurfParmIDs ), asCALL_THISCALL_ASGLOBAL, &ScriptMgr );
    assert( r >= 0 );

    //==== VSPAERO CS Group Functions ====//

    r = se->RegisterGlobalFunction("void AutoGroupVSPAEROControlSurfaces()", asFUNCTION(vsp::AutoGroupVSPAEROControlSurfaces), asCALL_CDECL );
    assert(r >= 0);


    r = se->RegisterGlobalFunction( "int GetNumControlSurfaceGroups()", asFUNCTION( vsp::GetNumControlSurfaceGroups ), asCALL_CDECL );
    assert( r >= 0 );


    r = se->RegisterGlobalFunction("int CreateVSPAEROControlSurfaceGroup()", asFUNCTION(vsp::CreateVSPAEROControlSurfaceGroup), asCALL_CDECL );
    assert(r >= 0);


    r = se->RegisterGlobalFunction("void AddAllToVSPAEROControlSurfaceGroup( int CSGroupIndex )", asFUNCTION(vsp::AddAllToVSPAEROControlSurfaceGroup ), asCALL_CDECL );
    assert(r >= 0);


    r = se->RegisterGlobalFunction("void RemoveAllFromVSPAEROControlSurfaceGroup( int CSGroupIndex )", asFUNCTION(vsp::RemoveAllFromVSPAEROControlSurfaceGroup ), asCALL_CDECL );
    assert(r >= 0);


    r = se->RegisterGlobalFunction( "array<string>@+ GetActiveCSNameVec( int CSGroupIndex )", asMETHOD( ScriptMgrSingleton, GetActiveCSNameVec ), asCALL_THISCALL_ASGLOBAL, &ScriptMgr );
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "array<string>@+ GetCompleteCSNameVec( )", asMETHOD( ScriptMgrSingleton, GetCompleteCSNameVec ), asCALL_THISCALL_ASGLOBAL, &ScriptMgr );
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "array<string>@+ GetAvailableCSNameVec( int CSGroupIndex )", asMETHOD( ScriptMgrSingleton, GetAvailableCSNameVec ), asCALL_THISCALL_ASGLOBAL, &ScriptMgr );
    assert( r >= 0 );


    r = se->RegisterGlobalFunction("void SetVSPAEROControlGroupName( const string & in name, int CSGroupIndex )", asFUNCTION(vsp::SetVSPAEROControlGroupName), asCALL_CDECL );
    assert(r >= 0);


    r = se->RegisterGlobalFunction( "string GetVSPAEROControlGroupName( int CSGroupIndex )", asFUNCTION( vsp::GetVSPAEROControlGroupName ), asCALL_CDECL );
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "void AddSelectedToCSGroup( array<int>@+ selected, int CSGroupIndex )", asMETHOD( ScriptMgrSingleton, AddSelectedToCSGroup ), asCALL_THISCALL_ASGLOBAL, &ScriptMgr );
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "void RemoveSelectedFromCSGroup( array<int>@+ selected, int CSGroupIndex )", asMETHOD( ScriptMgrSingleton, RemoveSelectedFromCSGroup ), asCALL_THISCALL_ASGLOBAL, &ScriptMgr );
    assert( r >= 0 ); // FIXME: RemoveSelectedFromCSGroup not working

    //==== VSPAERO Functions ====//

    r = se->RegisterGlobalFunction( "string GetVSPAERORefWingID()", asFUNCTION( vsp::GetVSPAERORefWingID ), asCALL_CDECL );
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "string SetVSPAERORefWingID( const string & in geom_id )", asFUNCTION( vsp::SetVSPAERORefWingID ), asCALL_CDECL );
    assert( r >= 0 );

    //==== VSPAERO Disk and Prop Functions ====//

    r = se->RegisterGlobalFunction( "string FindActuatorDisk( int disk_index )", asFUNCTION( vsp::FindActuatorDisk ), asCALL_CDECL );
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "int GetNumActuatorDisks()", asFUNCTION( vsp::GetNumActuatorDisks ), asCALL_CDECL );
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "string FindUnsteadyGroup( int group_index )", asFUNCTION( vsp::FindUnsteadyGroup ), asCALL_CDECL );
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "string GetUnsteadyGroupName( int group_index )", asFUNCTION( vsp::GetUnsteadyGroupName ), asCALL_CDECL );
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "array<string>@+ GetUnsteadyGroupCompIDs( int group_index )", asMETHOD( ScriptMgrSingleton, GetUnsteadyGroupCompIDs ), asCALL_THISCALL_ASGLOBAL, &ScriptMgr );
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "array<int>@+ GetUnsteadyGroupSurfIndexes( int group_index )", asMETHOD( ScriptMgrSingleton, GetUnsteadyGroupSurfIndexes ), asCALL_THISCALL_ASGLOBAL, &ScriptMgr );
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "int GetNumUnsteadyGroups()", asFUNCTION( vsp::GetNumUnsteadyGroups ), asCALL_CDECL );
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "int GetNumUnsteadyRotorGroups()", asFUNCTION( vsp::GetNumUnsteadyRotorGroups ), asCALL_CDECL );
    assert( r >= 0 );

    //==== XSecSurf Functions ====//

    r = se->RegisterGlobalFunction( "int GetNumXSecSurfs( const string & in geom_id )", asFUNCTION( vsp::GetNumXSecSurfs ), asCALL_CDECL );
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "string GetXSecSurf( const string & in geom_id, int index )", asFUNCTION( vsp::GetXSecSurf ), asCALL_CDECL );
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "int GetNumXSec( const string & in xsec_surf_id )", asFUNCTION( vsp::GetNumXSec ), asCALL_CDECL );
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "string GetXSec( const string & in xsec_surf_id, int xsec_index )", asFUNCTION( vsp::GetXSec ), asCALL_CDECL );
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "void ChangeXSecShape( const string & in xsec_surf_id, int xsec_index, int type )", asFUNCTION( vsp::ChangeXSecShape ), asCALL_CDECL );
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "void SetXSecSurfGlobalXForm( const string & in xsec_surf_id, const Matrix4d & in mat )", asFUNCTION( vsp::SetXSecSurfGlobalXForm ), asCALL_CDECL );
    assert( r >= 0 ); // TODO: Add Example


    r = se->RegisterGlobalFunction( "Matrix4d GetXSecSurfGlobalXForm( const string & in xsec_surf_id )", asFUNCTION( vsp::GetXSecSurfGlobalXForm ), asCALL_CDECL );
    assert( r >= 0 ); // TODO: Add Example

    //==== XSec Functions ====//

    r = se->RegisterGlobalFunction( "void SetXSecAlias( const string & in id, const string & in alias )", asFUNCTION( vsp::SetXSecAlias ), asCALL_CDECL );
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "string GetXSecAlias( const string & in id )", asFUNCTION( vsp::GetXSecAlias ), asCALL_CDECL );
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "void SetXSecCurveAlias( const string & in id, const string & in alias )", asFUNCTION( vsp::SetXSecCurveAlias ), asCALL_CDECL );
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "string GetXSecCurveAlias( const string & in id )", asFUNCTION( vsp::GetXSecCurveAlias ), asCALL_CDECL );
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "void CutXSec( const string & in geom_id, int index )", asFUNCTION( vsp::CutXSec ), asCALL_CDECL );
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "void CopyXSec( const string & in geom_id, int index )", asFUNCTION( vsp::CopyXSec ), asCALL_CDECL );
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "void PasteXSec( const string & in geom_id, int index )", asFUNCTION( vsp::PasteXSec ), asCALL_CDECL );
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "void InsertXSec( const string & in geom_id, int index, int type )", asFUNCTION( vsp::InsertXSec ), asCALL_CDECL );
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "int GetXSecShape( const string& in xsec_id )", asFUNCTION( vsp::GetXSecShape ), asCALL_CDECL );
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "double GetXSecWidth( const string& in xsec_id )", asFUNCTION( vsp::GetXSecWidth ), asCALL_CDECL );
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "double GetXSecHeight( const string& in xsec_id )", asFUNCTION( vsp::GetXSecHeight ), asCALL_CDECL );
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "void SetXSecWidth( const string& in xsec_id, double w )", asFUNCTION( vsp::SetXSecWidth ), asCALL_CDECL );
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "void SetXSecHeight( const string& in xsec_id, double h )", asFUNCTION( vsp::SetXSecHeight ), asCALL_CDECL );
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "void SetXSecWidthHeight( const string& in xsec_id, double w, double h )", asFUNCTION( vsp::SetXSecWidthHeight ), asCALL_CDECL );
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "array<string>@+ GetXSecParmIDs(const string & in xsec_id )", asMETHOD( ScriptMgrSingleton, GetXSecParmIDs ), asCALL_THISCALL_ASGLOBAL, &ScriptMgr );
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "string GetXSecParm( const string& in xsec_id, const string& in name )", asFUNCTION( vsp::GetXSecParm ), asCALL_CDECL );
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "array<vec3d>@+ ReadFileXSec(const string& in xsec_id, const string& in file_name )", asMETHOD( ScriptMgrSingleton, ReadFileXSec ), asCALL_THISCALL_ASGLOBAL, &ScriptMgr );
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "void SetXSecPnts( const string& in xsec_id, array<vec3d>@+ pnt_arr )", asMETHOD( ScriptMgrSingleton, SetXSecPnts ), asCALL_THISCALL_ASGLOBAL, &ScriptMgr );
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "vec3d ComputeXSecPnt( const string& in xsec_id, double fract )", asFUNCTION( vsp::ComputeXSecPnt ), asCALL_CDECL );
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "vec3d ComputeXSecTan( const string& in xsec_id, double fract )", asFUNCTION( vsp::ComputeXSecTan ), asCALL_CDECL );
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "void ResetXSecSkinParms( const string& in xsec_id )", asFUNCTION( vsp::ResetXSecSkinParms ), asCALL_CDECL );
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "void SetXSecContinuity( const string& in xsec_id, int cx )", asFUNCTION( vsp::SetXSecContinuity ), asCALL_CDECL );
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "void SetXSecTanAngles( const string& in xsec_id, int side, double top, double right = -1.0e12, double bottom = -1.0e12, double left = -1.0e12 )", asFUNCTION( vsp::SetXSecTanAngles ), asCALL_CDECL );
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "void SetXSecTanSlews( const string& in xsec_id, int side, double top, double right = -1.0e12, double bottom = -1.0e12, double left = -1.0e12 )", asFUNCTION( vsp::SetXSecTanSlews ), asCALL_CDECL );
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "void SetXSecTanStrengths( const string& in xsec_id, int side, double top, double right = -1.0e12, double bottom = -1.0e12, double left = -1.0e12 )", asFUNCTION( vsp::SetXSecTanStrengths ), asCALL_CDECL );
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "void SetXSecCurvatures( const string& in xsec_id, int side, double top, double right = -1.0e12, double bottom = -1.0e12, double left = -1.0e12 )", asFUNCTION( vsp::SetXSecCurvatures ), asCALL_CDECL );
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "void ReadFileAirfoil( const string& in xsec_id, const string& in file_name )", asFUNCTION( vsp::ReadFileAirfoil ), asCALL_CDECL );
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "void SetAirfoilUpperPnts( const string& in xsec_id, array<vec3d>@+ up_pnt_vec )", asMETHOD( ScriptMgrSingleton, SetAirfoilUpperPnts ), asCALL_THISCALL_ASGLOBAL, &ScriptMgr );
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "void SetAirfoilLowerPnts( const string& in xsec_id, array<vec3d>@+ low_pnt_vec )", asMETHOD( ScriptMgrSingleton, SetAirfoilLowerPnts ), asCALL_THISCALL_ASGLOBAL, &ScriptMgr );
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "void SetAirfoilPnts( const string& in xsec_id, array<vec3d>@+ up_pnt_vec, array<vec3d>@+ low_pnt_vec )", asMETHOD( ScriptMgrSingleton, SetAirfoilPnts ), asCALL_THISCALL_ASGLOBAL, &ScriptMgr );
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "array<vec3d>@+ GetHersheyBarLiftDist( const int& in npts, const double& in alpha, const double& in Vinf, const double& in span, bool full_span_flag = false )", asMETHOD( ScriptMgrSingleton, GetHersheyBarLiftDist ), asCALL_THISCALL_ASGLOBAL, &ScriptMgr );
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "array<vec3d>@+ GetHersheyBarDragDist( const int& in npts, const double& in alpha, const double& in Vinf, const double& in span, bool full_span_flag = false )", asMETHOD( ScriptMgrSingleton, GetHersheyBarDragDist ), asCALL_THISCALL_ASGLOBAL, &ScriptMgr );
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "array<vec3d>@+ GetVKTAirfoilPnts( const int& in npts, const double& in alpha, const double& in epsilon, const double& in kappa, const double& in tau )", asMETHOD( ScriptMgrSingleton, GetVKTAirfoilPnts ), asCALL_THISCALL_ASGLOBAL, &ScriptMgr );
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "array<double>@+ GetVKTAirfoilCpDist( const double& in alpha, const double& in epsilon, const double& in kappa, const double& in tau, array<vec3d>@+ xydata )", asMETHOD( ScriptMgrSingleton, GetVKTAirfoilCpDist ), asCALL_THISCALL_ASGLOBAL, &ScriptMgr );
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "array<vec3d>@+ GetEllipsoidSurfPnts( const vec3d& in center, const vec3d& in abc_rad, int u_npts = 20, int w_npts = 20 )", asMETHOD( ScriptMgrSingleton, GetEllipsoidSurfPnts ), asCALL_THISCALL_ASGLOBAL, &ScriptMgr );
    assert( r >= 0 );  // TODO: Example


    r = se->RegisterGlobalFunction( "array<vec3d>@+ GetFeatureLinePnts( const string& in geom_id )", asMETHOD( ScriptMgrSingleton, GetFeatureLinePnts ), asCALL_THISCALL_ASGLOBAL, &ScriptMgr );
    assert( r >= 0 );  // TODO: Example


    r = se->RegisterGlobalFunction( "array<double>@+ GetEllipsoidCpDist( array<vec3d>@+ surf_pnt_arr, const vec3d& in abc_rad, const vec3d& in V_inf )", asMETHOD( ScriptMgrSingleton, GetEllipsoidCpDist ), asCALL_THISCALL_ASGLOBAL, &ScriptMgr );
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "array<vec3d>@+ GetAirfoilUpperPnts(const string& in xsec_id )", asMETHOD( ScriptMgrSingleton, GetAirfoilUpperPnts ), asCALL_THISCALL_ASGLOBAL, &ScriptMgr );
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "array<vec3d>@+ GetAirfoilLowerPnts(const string& in xsec_id )", asMETHOD( ScriptMgrSingleton, GetAirfoilLowerPnts ), asCALL_THISCALL_ASGLOBAL, &ScriptMgr );
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "array<double>@+ GetUpperCSTCoefs( const string & in xsec_id )", asMETHOD( ScriptMgrSingleton, GetUpperCSTCoefs ), asCALL_THISCALL_ASGLOBAL, &ScriptMgr );
    assert( r >= 0 ); // TODO: Example


    r = se->RegisterGlobalFunction( "array<double>@+ GetLowerCSTCoefs( const string & in xsec_id )", asMETHOD( ScriptMgrSingleton, GetLowerCSTCoefs ), asCALL_THISCALL_ASGLOBAL, &ScriptMgr );
    assert( r >= 0 );  // TODO: Example


    r = se->RegisterGlobalFunction( "int GetUpperCSTDegree( const string& in xsec_id )", asFUNCTION( vsp::GetUpperCSTDegree ), asCALL_CDECL );
    assert( r >= 0 );  // TODO: Example


    r = se->RegisterGlobalFunction( "int GetLowerCSTDegree( const string& in xsec_id )", asFUNCTION( vsp::GetLowerCSTDegree ), asCALL_CDECL );
    assert( r >= 0 );  // TODO: Example


    r = se->RegisterGlobalFunction( "void SetUpperCST( const string& in xsec_id, int deg, array<double>@+ coeff_arr )", asMETHOD( ScriptMgrSingleton, SetUpperCST ), asCALL_THISCALL_ASGLOBAL, &ScriptMgr );
    assert( r >= 0 );  // TODO: Example


    r = se->RegisterGlobalFunction( "void SetLowerCST( const string& in xsec_id, int deg, array<double>@+ coeff_arr )", asMETHOD( ScriptMgrSingleton, SetLowerCST ), asCALL_THISCALL_ASGLOBAL, &ScriptMgr );
    assert( r >= 0 );  // TODO: Example


    r = se->RegisterGlobalFunction( "void PromoteCSTUpper( const string& in xsec_id )", asFUNCTION( vsp::PromoteCSTUpper ), asCALL_CDECL );
    assert( r >= 0 );  // TODO: Example


    r = se->RegisterGlobalFunction( "void PromoteCSTLower( const string& in xsec_id )", asFUNCTION( vsp::PromoteCSTLower ), asCALL_CDECL );
    assert( r >= 0 );  // TODO: Example


    r = se->RegisterGlobalFunction( "void DemoteCSTUpper( const string& in xsec_id )", asFUNCTION( vsp::DemoteCSTUpper ), asCALL_CDECL );
    assert( r >= 0 );  // TODO: Example


    r = se->RegisterGlobalFunction( "void DemoteCSTLower( const string& in xsec_id )", asFUNCTION( vsp::DemoteCSTLower ), asCALL_CDECL );
    assert( r >= 0 );  // TODO: Example


    r = se->RegisterGlobalFunction( "void FitAfCST( const string& in xsec_surf_id, int xsec_index, int deg )", asFUNCTION( vsp::FitAfCST ), asCALL_CDECL );
    assert( r >= 0 );  // TODO: Example


    r = se->RegisterGlobalFunction( "void WriteBezierAirfoil( const string& in file_name, const string& in geom_id, const double& in foilsurf_u )", asFUNCTION( vsp::WriteBezierAirfoil ), asCALL_CDECL );
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "void WriteSeligAirfoil( const string& in file_name, const string& in geom_id, const double& in foilsurf_u )", asFUNCTION( vsp::WriteSeligAirfoil ), asCALL_CDECL );
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "array<vec3d>@+ GetAirfoilCoordinates( const string& in geom_id, const double& in foilsurf_u )", asMETHOD( ScriptMgrSingleton, GetAirfoilCoordinates ), asCALL_THISCALL_ASGLOBAL, &ScriptMgr );
    assert( r >= 0 );

    //==== Edit Curve XSec Functions ====//

    r = se->RegisterGlobalFunction( "void EditXSecInitShape( const string& in xsec_id )", asFUNCTION( vsp::EditXSecInitShape ), asCALL_CDECL );
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "void EditXSecConvertTo( const string& in xsec_id, const int& in newtype )", asFUNCTION( vsp::EditXSecConvertTo ), asCALL_CDECL );
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "array<double>@+ GetEditXSecUVec( const string& in xsec_id )", asMETHOD( ScriptMgrSingleton, GetEditXSecUVec ), asCALL_THISCALL_ASGLOBAL, &ScriptMgr );
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "array<vec3d>@+ GetEditXSecCtrlVec( const string& in xsec_id, const bool non_dimensional = true )", asMETHOD( ScriptMgrSingleton, GetEditXSecCtrlVec ), asCALL_THISCALL_ASGLOBAL, &ScriptMgr );
    assert( r >= 0 );

    r = se->RegisterGlobalFunction( "void SetEditXSecPnts( const string& in xsec_id, array<double>@+ u_vec, array<vec3d>@+ control_pts, array<double>@+ r_vec )", asMETHOD( ScriptMgrSingleton, SetEditXSecPnts ), asCALL_THISCALL_ASGLOBAL, &ScriptMgr );
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "void EditXSecDelPnt( const string& in xsec_id, const int& in indx )", asFUNCTION( vsp::EditXSecDelPnt ), asCALL_CDECL );
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "int EditXSecSplit01( const string& in xsec_id, const double& in u )", asFUNCTION( vsp::EditXSecSplit01 ), asCALL_CDECL );
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "void MoveEditXSecPnt( const string& in xsec_id, const int& in indx, const vec3d& in new_pnt )", asFUNCTION( vsp::MoveEditXSecPnt ), asCALL_CDECL );
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "void ConvertXSecToEdit( const string& in geom_id, const int& in indx = 0 )", asFUNCTION( vsp::ConvertXSecToEdit ), asCALL_CDECL );
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "array<bool>@+ GetEditXSecFixedUVec( const string& in xsec_id )", asMETHOD( ScriptMgrSingleton, GetEditXSecFixedUVec ), asCALL_THISCALL_ASGLOBAL, &ScriptMgr );
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "void SetEditXSecFixedUVec( const string& in xsec_id, array<bool>@+ fixed_u_vec )", asMETHOD( ScriptMgrSingleton, SetEditXSecFixedUVec ), asCALL_THISCALL_ASGLOBAL, &ScriptMgr );
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "void ReparameterizeEditXSec( const string& in xsec_id )", asFUNCTION( vsp::ReparameterizeEditXSec ), asCALL_CDECL );
    assert( r >= 0 );

    //==== Background3D Functions ====//

    r = se->RegisterGlobalFunction( "string AddBackground3D()", asFUNCTION( vsp::AddBackground3D ), asCALL_CDECL );
    assert( r >= 0 );

    r = se->RegisterGlobalFunction( "int GetNumBackground3Ds()", asFUNCTION( vsp::GetNumBackground3Ds ), asCALL_CDECL );
    assert( r >= 0 );

    r = se->RegisterGlobalFunction( "array<string>@+ GetAllBackground3Ds()", asMETHOD( ScriptMgrSingleton, GetAllBackground3Ds ), asCALL_THISCALL_ASGLOBAL, &ScriptMgr );
    assert( r >= 0 );

    r = se->RegisterGlobalFunction( "void ShowAllBackground3Ds()", asFUNCTION( vsp::ShowAllBackground3Ds ), asCALL_CDECL );
    assert( r >= 0 );

    r = se->RegisterGlobalFunction( "void HideAllBackground3Ds()", asFUNCTION( vsp::HideAllBackground3Ds ), asCALL_CDECL );
    assert( r >= 0 );

    r = se->RegisterGlobalFunction( "void DelAllBackground3Ds()", asFUNCTION( vsp::DelAllBackground3Ds ), asCALL_CDECL );
    assert( r >= 0 );

    r = se->RegisterGlobalFunction( "void DelBackground3D( const string & in id )", asFUNCTION( vsp::DelBackground3D ), asCALL_CDECL );
    assert( r >= 0 );

    r = se->RegisterGlobalFunction( "array<string>@+ GetAllBackground3DRelativePaths()", asMETHOD( ScriptMgrSingleton, GetAllBackground3DRelativePaths ), asCALL_THISCALL_ASGLOBAL, &ScriptMgr );
    assert( r >= 0 );

    r = se->RegisterGlobalFunction( "array<string>@+ GetAllBackground3DAbsolutePaths()", asMETHOD( ScriptMgrSingleton, GetAllBackground3DAbsolutePaths ), asCALL_THISCALL_ASGLOBAL, &ScriptMgr );
    assert( r >= 0 );

    r = se->RegisterGlobalFunction( "string GetBackground3DRelativePath( const string & in id )", asFUNCTION( vsp::GetBackground3DRelativePath ), asCALL_CDECL );
    assert( r >= 0 );

    r = se->RegisterGlobalFunction( "string GetBackground3DAbsolutePath( const string & in id )", asFUNCTION( vsp::GetBackground3DAbsolutePath ), asCALL_CDECL );
    assert( r >= 0 );

    r = se->RegisterGlobalFunction( "void SetBackground3DRelativePath( const string & in id, const string & in fname )", asFUNCTION( vsp::SetBackground3DRelativePath ), asCALL_CDECL );
    assert( r >= 0 );

    r = se->RegisterGlobalFunction( "void SetBackground3DAbsolutePath( const string & in id, const string & in fname )", asFUNCTION( vsp::SetBackground3DAbsolutePath ), asCALL_CDECL );
    assert( r >= 0 );

    //==== RoutingGeom Functions ====//

    r = se->RegisterGlobalFunction( "int GetNumRoutingPts( const string & in routing_id )", asFUNCTION( vsp::GetNumRoutingPts ), asCALL_CDECL );
    assert( r >= 0 );

    r = se->RegisterGlobalFunction( "string AddRoutingPt( const string & in routing_id, const string & in geom_id, int surf_index )", asFUNCTION( vsp::AddRoutingPt ), asCALL_CDECL );
    assert( r >= 0 );

    r = se->RegisterGlobalFunction( "string InsertRoutingPt( const string & in routing_id, int index, const string & in geom_id, int surf_index )", asFUNCTION( vsp::InsertRoutingPt ), asCALL_CDECL );
    assert( r >= 0 );

    r = se->RegisterGlobalFunction( "void DelRoutingPt( const string & in routing_id, int index  )", asFUNCTION( vsp::DelRoutingPt ), asCALL_CDECL );
    assert( r >= 0 );

    r = se->RegisterGlobalFunction( "void DelAllRoutingPt( const string & in routing_id )", asFUNCTION( vsp::DelAllRoutingPt ), asCALL_CDECL );
    assert( r >= 0 );

    r = se->RegisterGlobalFunction( "int MoveRoutingPt( const string & in routing_id, int index, int reorder_type )", asFUNCTION( vsp::MoveRoutingPt ), asCALL_CDECL );
    assert( r >= 0 );

    r = se->RegisterGlobalFunction( "string GetRoutingPtID( const string & in routing_id, int index )", asFUNCTION( vsp::GetRoutingPtID ), asCALL_CDECL );
    assert( r >= 0 );

    r = se->RegisterGlobalFunction( "array<string>@+ GetAllRoutingPtIds( const string & in routing_id )", asMETHOD( ScriptMgrSingleton, GetAllRoutingPtIds ), asCALL_THISCALL_ASGLOBAL, &ScriptMgr );
    assert( r >= 0 );

    r = se->RegisterGlobalFunction( "string GetRoutingPtParentID( const string & in pt_id )", asFUNCTION( vsp::GetRoutingPtParentID ), asCALL_CDECL );
    assert( r >= 0 );

    r = se->RegisterGlobalFunction( "void SetRoutingPtParentID( const string & in pt_id, const string & in parent_id )", asFUNCTION( vsp::SetRoutingPtParentID ), asCALL_CDECL );
    assert( r >= 0 );

    r = se->RegisterGlobalFunction( "vec3d GetMainRoutingPtCoord( const string & in pt_id )", asFUNCTION( vsp::GetMainRoutingPtCoord ), asCALL_CDECL );
    assert( r >= 0 );

    r = se->RegisterGlobalFunction( "vec3d GetRoutingPtCoord( const string & in routing_id, int index, int symm_index )", asFUNCTION( vsp::GetRoutingPtCoord ), asCALL_CDECL );
    assert( r >= 0 );

    r = se->RegisterGlobalFunction( "array<vec3d>@+ GetAllRoutingPtCoords( const string & in routing_id, int symm_index )", asMETHOD( ScriptMgrSingleton, GetAllRoutingPtCoords ), asCALL_THISCALL_ASGLOBAL, &ScriptMgr );
    assert( r >= 0 );

    //==== BOR Functions ====//

    r = se->RegisterGlobalFunction( "void ChangeBORXSecShape( const string & in geom_id, int type )", asFUNCTION( vsp::ChangeBORXSecShape ), asCALL_CDECL );
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "int GetBORXSecShape( const string & in geom_id )", asFUNCTION( vsp::GetBORXSecShape ), asCALL_CDECL );
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "array<vec3d>@+ ReadBORFileXSec(const string& in bor_id, const string& in file_name )", asMETHOD( ScriptMgrSingleton, ReadBORFileXSec ), asCALL_THISCALL_ASGLOBAL, &ScriptMgr );
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "void SetBORXSecPnts( const string& in bor_id, array<vec3d>@+ pnt_arr )", asMETHOD( ScriptMgrSingleton, SetBORXSecPnts ), asCALL_THISCALL_ASGLOBAL, &ScriptMgr );
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "vec3d ComputeBORXSecPnt( const string& in bor_id, double fract )", asFUNCTION( vsp::ComputeBORXSecPnt ), asCALL_CDECL );
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "vec3d ComputeBORXSecTan( const string& in bor_id, double fract )", asFUNCTION( vsp::ComputeBORXSecTan ), asCALL_CDECL );
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "void ReadBORFileAirfoil( const string& in bor_id, const string& in file_name )", asFUNCTION( vsp::ReadBORFileAirfoil ), asCALL_CDECL );
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "void SetBORAirfoilUpperPnts( const string& in bor_id, array<vec3d>@+ up_pnt_vec )", asMETHOD( ScriptMgrSingleton, SetBORAirfoilUpperPnts ), asCALL_THISCALL_ASGLOBAL, &ScriptMgr );
    assert( r >= 0 );



    r = se->RegisterGlobalFunction( "void SetBORAirfoilLowerPnts( const string& in bor_id, array<vec3d>@+ low_pnt_vec )", asMETHOD( ScriptMgrSingleton, SetBORAirfoilLowerPnts ), asCALL_THISCALL_ASGLOBAL, &ScriptMgr );
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "void SetBORAirfoilPnts( const string& in bor_id, array<vec3d>@+ up_pnt_vec, array<vec3d>@+ low_pnt_vec )", asMETHOD( ScriptMgrSingleton, SetBORAirfoilPnts ), asCALL_THISCALL_ASGLOBAL, &ScriptMgr );
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "array<vec3d>@+ GetBORAirfoilUpperPnts(const string& in bor_id )", asMETHOD( ScriptMgrSingleton, GetBORAirfoilUpperPnts ), asCALL_THISCALL_ASGLOBAL, &ScriptMgr );
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "array<vec3d>@+ GetBORAirfoilLowerPnts(const string& in bor_id )", asMETHOD( ScriptMgrSingleton, GetBORAirfoilLowerPnts ), asCALL_THISCALL_ASGLOBAL, &ScriptMgr );
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "array<double>@+ GetBORUpperCSTCoefs( const string & in bor_id )", asMETHOD( ScriptMgrSingleton, GetBORUpperCSTCoefs ), asCALL_THISCALL_ASGLOBAL, &ScriptMgr );
    assert( r >= 0 ); // TODO: Example


    r = se->RegisterGlobalFunction( "array<double>@+ GetBORLowerCSTCoefs( const string & in bor_id )", asMETHOD( ScriptMgrSingleton, GetBORLowerCSTCoefs ), asCALL_THISCALL_ASGLOBAL, &ScriptMgr );
    assert( r >= 0 );  // TODO: Example


    r = se->RegisterGlobalFunction( "int GetBORUpperCSTDegree( const string& in bor_id )", asFUNCTION( vsp::GetBORUpperCSTDegree ), asCALL_CDECL );
    assert( r >= 0 );  // TODO: Example


    r = se->RegisterGlobalFunction( "int GetBORLowerCSTDegree( const string& in bor_id )", asFUNCTION( vsp::GetBORLowerCSTDegree ), asCALL_CDECL );
    assert( r >= 0 );  // TODO: Example


    r = se->RegisterGlobalFunction( "void SetBORUpperCST( const string& in bor_id, int deg, array<double>@+ coeff_arr )", asMETHOD( ScriptMgrSingleton, SetBORUpperCST ), asCALL_THISCALL_ASGLOBAL, &ScriptMgr );
    assert( r >= 0 );  // TODO: Example


    r = se->RegisterGlobalFunction( "void SetBORLowerCST( const string& in bor_id, int deg, array<double>@+ coeff_arr )", asMETHOD( ScriptMgrSingleton, SetBORLowerCST ), asCALL_THISCALL_ASGLOBAL, &ScriptMgr );
    assert( r >= 0 );  // TODO: Example


    r = se->RegisterGlobalFunction( "void PromoteBORCSTUpper( const string& in bor_id )", asFUNCTION( vsp::PromoteBORCSTUpper ), asCALL_CDECL );
    assert( r >= 0 );  // TODO: Example


    r = se->RegisterGlobalFunction( "void PromoteBORCSTLower( const string& in bor_id )", asFUNCTION( vsp::PromoteBORCSTLower ), asCALL_CDECL );
    assert( r >= 0 );  // TODO: Example


    r = se->RegisterGlobalFunction( "void DemoteBORCSTUpper( const string& in bor_id )", asFUNCTION( vsp::DemoteBORCSTUpper ), asCALL_CDECL );
    assert( r >= 0 );  // TODO: Example


    r = se->RegisterGlobalFunction( "void DemoteBORCSTLower( const string& in bor_id )", asFUNCTION( vsp::DemoteBORCSTLower ), asCALL_CDECL );
    assert( r >= 0 );  // TODO: Example


    r = se->RegisterGlobalFunction( "void FitBORAfCST( const string& in bor_id, int deg )", asFUNCTION( vsp::FitBORAfCST ), asCALL_CDECL );
    assert( r >= 0 );  // TODO: Example

    //==== Sets Functions ====//

    r = se->RegisterGlobalFunction( "int GetNumSets()", asFUNCTION( vsp::GetNumSets ), asCALL_CDECL );
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "void SetSetName( int index, const string& in name )", asFUNCTION( vsp::SetSetName ), asCALL_CDECL );
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "string GetSetName( int index )", asFUNCTION( vsp::GetSetName ), asCALL_CDECL );
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "array<string>@+ GetGeomSetAtIndex( int index )", asMETHOD( ScriptMgrSingleton, GetGeomSetAtIndex ), asCALL_THISCALL_ASGLOBAL, &ScriptMgr );
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "array<string>@+ GetGeomSet( const string & in name )", asMETHOD( ScriptMgrSingleton, GetGeomSet ), asCALL_THISCALL_ASGLOBAL, &ScriptMgr );
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "int GetSetIndex( const string & in name )", asFUNCTION( vsp::GetSetIndex ), asCALL_CDECL );
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "bool GetSetFlag( const string & in geom_id, int set_index )", asFUNCTION( vsp::GetSetFlag ), asCALL_CDECL );
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "void SetSetFlag( const string & in geom_id, int set_index, bool flag )", asFUNCTION( vsp::SetSetFlag ), asCALL_CDECL );
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "void CopyPasteSet(  int copyIndex, int pasteIndex  )", asFUNCTION( vsp::CopyPasteSet ), asCALL_CDECL );
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "bool GetBBoxSet( int set, double & out xmin_out, double & out ymin_out, double & out zmin_out, double & out xlen_out, double & out ylen_out, double & out zlen_out )", asFUNCTION( vsp::GetBBoxSet ), asCALL_CDECL );
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "bool GetScaleIndependentBBoxSet( int set, double & out xmin_out, double & out ymin_out, double & out zmin_out, double & out xlen_out, double & out ylen_out, double & out zlen_out )", asFUNCTION( vsp::GetScaleIndependentBBoxSet ), asCALL_CDECL );
    assert( r >= 0 );

    //=== Group Modifications ===//

    r = se->RegisterGlobalFunction( "void ScaleSet( int set_index, double scale )", asFUNCTION( vsp::ScaleSet ), asCALL_CDECL );
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "void RotateSet( int set_index, double x_rot_deg, double y_rot_deg, double z_rot_deg )", asFUNCTION( vsp::RotateSet ), asCALL_CDECL );
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "void TranslateSet( int set_index, const vec3d & in translation_vec )", asFUNCTION( vsp::TranslateSet ), asCALL_CDECL );
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "void TransformSet( int set_index, const vec3d & in translation_vec, double x_rot_deg, double y_rot_deg, double z_rot_deg, double scale, bool scale_translations_flag )", asFUNCTION( vsp::TransformSet ), asCALL_CDECL );
    assert( r >= 0 );

    //==== Parm Functions ====//

    r = se->RegisterGlobalFunction( "bool ValidParm( const string & in id )", asFUNCTION( vsp::ValidParm ), asCALL_CDECL );
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "double SetParmVal(const string & in parm_id, double val )",
                                    asFUNCTIONPR( vsp::SetParmVal, ( const string &, double val ), double ), asCALL_CDECL );
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "double SetParmValLimits(const string & in parm_id, double val, double lower_limit, double upper_limit )",
                                    asFUNCTION( vsp::SetParmValLimits ), asCALL_CDECL );
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "double SetParmValUpdate(const string & in parm_id, double val )",
                                    asFUNCTIONPR( vsp::SetParmValUpdate, ( const string &, double val ), double ), asCALL_CDECL );
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "double SetParmVal(const string & in container_id, const string & in name, const string & in group, double val )",
                                    asFUNCTIONPR( vsp::SetParmVal, ( const string &, const string &, const string &, double val ), double ), asCALL_CDECL );
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "double SetParmValUpdate(const string & in container_id, const string & in parm_name, const string & in parm_group_name, double val )",
                                    asFUNCTIONPR( vsp::SetParmValUpdate, ( const string &, const string &, const string &, double val ), double ), asCALL_CDECL );
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "double GetParmVal(const string & in parm_id )", asFUNCTIONPR( vsp::GetParmVal, ( const string & ), double ), asCALL_CDECL );
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "double GetParmVal(const string & in container_id, const string & in name, const string & in group )",
                                    asFUNCTIONPR( vsp::GetParmVal, ( const string &, const string &, const string & ), double ), asCALL_CDECL );
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "int GetIntParmVal(const string & in parm_id )", asFUNCTION( vsp::GetIntParmVal ), asCALL_CDECL );
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "bool GetBoolParmVal(const string & in parm_id )", asFUNCTION( vsp::GetBoolParmVal ), asCALL_CDECL );
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "void SetParmUpperLimit( const string & in parm_id, double val )", asFUNCTION( vsp::SetParmUpperLimit ), asCALL_CDECL );
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "double GetParmUpperLimit( const string & in parm_id )", asFUNCTION( vsp::GetParmUpperLimit ), asCALL_CDECL );
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "void SetParmLowerLimit( const string & in parm_id, double val )", asFUNCTION( vsp::SetParmLowerLimit ), asCALL_CDECL );
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "double GetParmLowerLimit( const string & in parm_id )", asFUNCTION( vsp::GetParmLowerLimit ), asCALL_CDECL );
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "int GetParmType( const string & in parm_id )", asFUNCTION( vsp::GetParmType ), asCALL_CDECL );
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "string GetParmName( const string & in parm_id )", asFUNCTION( vsp::GetParmName ), asCALL_CDECL );
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "string GetParmGroupName( const string & in parm_id )", asFUNCTION( vsp::GetParmGroupName ), asCALL_CDECL );
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "string GetParmDisplayGroupName( const string & in parm_id )", asFUNCTION( vsp::GetParmDisplayGroupName ), asCALL_CDECL );
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "string GetParmContainer( const string & in parm_id )", asFUNCTION( vsp::GetParmContainer ), asCALL_CDECL );
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "void SetParmDescript( const string & in parm_id, const string & in desc )", asFUNCTION( vsp::SetParmDescript ), asCALL_CDECL );
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "string GetParmDescript( const string & in parm_id )", asFUNCTION( vsp::GetParmDescript ), asCALL_CDECL );
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "string FindParm( const string & in parm_container_id, const string & in parm_name, const string & in group_name )", asFUNCTION( vsp::FindParm ), asCALL_CDECL );
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "string GetParm(const string & in container_id, const string & in name, const string & in group )", asFUNCTION( vsp::GetParm ), asCALL_CDECL );
    assert( r >= 0 );

    //=== Parm Container Functions ===//

    r = se->RegisterGlobalFunction( "array<string>@+ FindContainers()", asMETHOD( ScriptMgrSingleton, FindContainers ), asCALL_THISCALL_ASGLOBAL, &ScriptMgr );
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "array<string>@+ FindContainersWithName( const string & in name )", asMETHOD( ScriptMgrSingleton, FindContainersWithName ), asCALL_THISCALL_ASGLOBAL, &ScriptMgr);
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "string FindContainer( const string & in name, int index )", asFUNCTION( vsp::FindContainer ), asCALL_CDECL);
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "string GetContainerName( const string & in parm_container_id )", asFUNCTION( vsp::GetContainerName ), asCALL_CDECL);
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "array<string>@+ FindContainerGroupNames( const string & in parm_container_id )", asMETHOD( ScriptMgrSingleton, FindContainerGroupNames ), asCALL_THISCALL_ASGLOBAL, &ScriptMgr );
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "array<string>@+ FindContainerParmIDs( const string & in parm_container_id )", asMETHOD( ScriptMgrSingleton, FindContainerParmIDs ), asCALL_THISCALL_ASGLOBAL, &ScriptMgr );
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "string GetVehicleID()", asFUNCTION( vsp::GetVehicleID ), asCALL_CDECL);
    assert( r >= 0 );

    //=== Register User Parm Functions ====//

    r = se->RegisterGlobalFunction( "int GetNumUserParms()", asFUNCTION( vsp::GetNumUserParms ), asCALL_CDECL);
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "int GetNumPredefinedUserParms()", asFUNCTION( vsp::GetNumPredefinedUserParms ), asCALL_CDECL);
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "array<string>@+ GetAllUserParms()", asMETHOD( ScriptMgrSingleton, GetAllUserParms ), asCALL_THISCALL_ASGLOBAL, &ScriptMgr);
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "string GetUserParmContainer()", asFUNCTION( vsp::GetUserParmContainer ), asCALL_CDECL);
    assert( r >= 0 );



    r = se->RegisterGlobalFunction( "string AddUserParm( int type, const string & in name, const string & in group )",
                                    asFUNCTION( vsp::AddUserParm ), asCALL_CDECL );
    assert( r );


    r = se->RegisterGlobalFunction( "void DeleteUserParm( const string & in parm_id)", asFUNCTION( vsp::DeleteUserParm ), asCALL_CDECL);
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "void DeleteAllUserParm()", asFUNCTION( vsp::DeleteAllUserParm ), asCALL_CDECL);
    assert( r >= 0 );

    //=== Register Snap To Functions ====//

    r = se->RegisterGlobalFunction( "double ComputeMinClearanceDistance( const string & in geom_id, int set, bool useMode = false, const string & in modeID = string() )", asFUNCTION( vsp::ComputeMinClearanceDistance ), asCALL_CDECL );
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "double SnapParm( const string & in parm_id, double target_min_dist, bool inc_flag, int set, bool useMode = false, const string & in modeID = string() )", asFUNCTION( vsp::SnapParm ), asCALL_CDECL );
    assert( r >= 0 );

    //=== Register Var Preset Functions ====//

    r = se->RegisterGlobalFunction( "string AddVarPresetGroup( const string & in group_name )", asFUNCTION( vsp::AddVarPresetGroup ), asCALL_CDECL);
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "string AddVarPresetSetting( const string & in group_id, const string & in setting_name )", asFUNCTION( vsp::AddVarPresetSetting ), asCALL_CDECL);
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "void AddVarPresetParm( const string & in group_id, const string & in parm_ID )", asFUNCTION( vsp::AddVarPresetParm ), asCALL_CDECL);
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "void DeleteVarPresetGroup( const string & in group_id )", asFUNCTION( vsp::DeleteVarPresetGroup ), asCALL_CDECL);
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "void DeleteVarPresetSetting( const string & in group_id, const string & in setting_id )", asFUNCTION( vsp::DeleteVarPresetSetting ), asCALL_CDECL);
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "void DeleteVarPresetParm( const string & in group_id, const string & in parm_id )", asFUNCTION( vsp::DeleteVarPresetParm ), asCALL_CDECL);
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "void SetVarPresetParmVal( const string & in group_id, const string & in setting_id, const string & in parm_id, double parm_val )", asFUNCTION( vsp::SetVarPresetParmVal ), asCALL_CDECL);
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "double GetVarPresetParmVal( const string & in group_id, const string & in setting_id, const string & in parm_id )", asFUNCTION( vsp::GetVarPresetParmVal ), asCALL_CDECL);
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "string GetGroupName( const string & in group_id )", asFUNCTION( vsp::GetGroupName ), asCALL_CDECL);
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "string GetSettingName( const string & in setting_id )", asFUNCTION( vsp::GetSettingName ), asCALL_CDECL);
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "void SetGroupName( const string & in group_id, const string & in group_name )", asFUNCTION( vsp::SetGroupName ), asCALL_CDECL);
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "void SetSettingName( const string & in setting_id, const string & in setting_name )", asFUNCTION( vsp::SetSettingName ), asCALL_CDECL);
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "array<string>@+ GetVarPresetGroups()", asMETHOD( ScriptMgrSingleton, GetVarPresetGroups ), asCALL_THISCALL_ASGLOBAL, &ScriptMgr );
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "array<string>@+ GetVarPresetSettings( const string & in group_id )", asMETHOD( ScriptMgrSingleton, GetVarPresetSettings ), asCALL_THISCALL_ASGLOBAL, &ScriptMgr );
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "array<string>@+ GetVarPresetParmIDs( const string & in group_id )", asMETHOD( ScriptMgrSingleton, GetVarPresetParmIDs ), asCALL_THISCALL_ASGLOBAL, &ScriptMgr );
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "array<double>@+ GetVarPresetParmVals( const string & in setting_id )", asMETHOD( ScriptMgrSingleton, GetVarPresetParmVals ), asCALL_THISCALL_ASGLOBAL, &ScriptMgr );
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "void SetVarPresetParmVals( const string& in setting_id, array<double>@+ parm_vals )", asMETHOD( ScriptMgrSingleton, SetVarPresetParmVals ), asCALL_THISCALL_ASGLOBAL, &ScriptMgr );
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "void SaveVarPresetParmVals( const string & in group_id, const string & in setting_id )", asFUNCTION( vsp::SaveVarPresetParmVals ), asCALL_CDECL);
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "void ApplyVarPresetSetting( const string & in group_id, const string & in setting_id )", asFUNCTION( vsp::ApplyVarPresetSetting ), asCALL_CDECL);
    assert( r >= 0 );


    //=== Register Mode Functions ====//

    r = se->RegisterGlobalFunction( "string CreateAndAddMode( const string & in name, int normal_set, int degen_set )", asFUNCTION( vsp::CreateAndAddMode ), asCALL_CDECL);
    assert( r >= 0 );

    r = se->RegisterGlobalFunction( "int GetNumModes()", asFUNCTION( vsp::GetNumModes ), asCALL_CDECL);
    assert( r >= 0 );

    r = se->RegisterGlobalFunction( "array<string>@+ GetAllModes()", asMETHOD( ScriptMgrSingleton, GetAllModes ), asCALL_THISCALL_ASGLOBAL, &ScriptMgr );
    assert( r >= 0 );

    r = se->RegisterGlobalFunction( "void DelMode( const string & in mode_id )", asFUNCTION( vsp::DelMode ), asCALL_CDECL);
    assert( r >= 0 );

    r = se->RegisterGlobalFunction( "void DelAllModes()", asFUNCTION( vsp::DelAllModes ), asCALL_CDECL);
    assert( r >= 0 );

    r = se->RegisterGlobalFunction( "void ApplyModeSettings( const string & in mode_id )", asFUNCTION( vsp::ApplyModeSettings ), asCALL_CDECL);
    assert( r >= 0 );

    r = se->RegisterGlobalFunction( "void ShowOnlyMode( const string & in mode_id )", asFUNCTION( vsp::ShowOnlyMode ), asCALL_CDECL);
    assert( r >= 0 );

    r = se->RegisterGlobalFunction( "void ModeAddGroupSetting( const string & in mode_id, const string & in group_id, const string & in setting_id )", asFUNCTION( vsp::ModeAddGroupSetting ), asCALL_CDECL);
    assert( r >= 0 );

    r = se->RegisterGlobalFunction( "string ModeGetGroup( const string & in mode_id, int indx )", asFUNCTION( vsp::ModeGetGroup ), asCALL_CDECL);
    assert( r >= 0 );

    r = se->RegisterGlobalFunction( "string ModeGetSetting( const string & in mode_id, int indx )", asFUNCTION( vsp::ModeGetSetting ), asCALL_CDECL);
    assert( r >= 0 );

    r = se->RegisterGlobalFunction( "array<string>@+ ModeGetAllGroups( const string & in mode_id )", asMETHOD( ScriptMgrSingleton, ModeGetAllGroups ), asCALL_THISCALL_ASGLOBAL, &ScriptMgr );
    assert( r >= 0 );

    r = se->RegisterGlobalFunction( "array<string>@+ ModeGetAllSettings( const string & in mode_id )", asMETHOD( ScriptMgrSingleton, ModeGetAllSettings ), asCALL_THISCALL_ASGLOBAL, &ScriptMgr );
    assert( r >= 0 );

    r = se->RegisterGlobalFunction( "void RemoveGroupSetting( const string & in mode_id, int indx )", asFUNCTION( vsp::RemoveGroupSetting ), asCALL_CDECL);
    assert( r >= 0 );

    r = se->RegisterGlobalFunction( "void RemoveAllGroupSettings( const string & in mode_id )", asFUNCTION( vsp::RemoveAllGroupSettings ), asCALL_CDECL);
    assert( r >= 0 );

    //=== Register PCurve Functions ====//

    r = se->RegisterGlobalFunction( "void SetPCurve( const string& in geom_id, const int & in pcurveid, array<double>@+ tvec, array<double>@+ valvec, const int & in newtype )", asMETHOD( ScriptMgrSingleton, SetPCurve ), asCALL_THISCALL_ASGLOBAL, &ScriptMgr );
    assert( r >= 0 ); // TODO: Example


    r = se->RegisterGlobalFunction( "void PCurveConvertTo( const string & in geom_id, const int & in pcurveid, const int & in newtype )", asFUNCTION( vsp::PCurveConvertTo ), asCALL_CDECL );
    assert( r >= 0 ); // TODO: Example


    r = se->RegisterGlobalFunction( "int PCurveGetType( const string & in geom_id, const int & in pcurveid )", asFUNCTION( vsp::PCurveGetType ), asCALL_CDECL );
    assert( r >= 0 ); // TODO: Example


    r = se->RegisterGlobalFunction( "array<double>@+ PCurveGetTVec( const string & in geom_id, const int & in pcurveid )", asMETHOD( ScriptMgrSingleton, PCurveGetTVec ), asCALL_THISCALL_ASGLOBAL, &ScriptMgr );
    assert( r >= 0 ); // TODO: Example


    r = se->RegisterGlobalFunction( "array<double>@+ PCurveGetValVec( const string & in geom_id, const int & in pcurveid )", asMETHOD( ScriptMgrSingleton, PCurveGetValVec ), asCALL_THISCALL_ASGLOBAL, &ScriptMgr );
    assert( r >= 0 ); // TODO: Example


    r = se->RegisterGlobalFunction( "void PCurveDeletePt( const string & in geom_id, const int & in pcurveid, const int & in indx )", asFUNCTION( vsp::PCurveDeletePt ), asCALL_CDECL);
    assert( r >= 0 ); // TODO: Example


    r = se->RegisterGlobalFunction( "int PCurveSplit( const string & in geom_id, const int & in pcurveid, const double & in tsplit )", asFUNCTION( vsp::PCurveSplit ), asCALL_CDECL);
    assert( r >= 0 ); // TODO: Example


    r = se->RegisterGlobalFunction( "void ApproximateAllPropellerPCurves( const string & in geom_id )", asFUNCTION( vsp::ApproximateAllPropellerPCurves ), asCALL_CDECL);
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "void ResetPropellerThicknessCurve( const string & in geom_id )", asFUNCTION( vsp::ResetPropellerThicknessCurve ), asCALL_CDECL);
    assert( r >= 0 );

    //=== Register ParasiteDragTool Functions ====//

    r = se->RegisterGlobalFunction( "void AddExcrescence(const string & in excresName, const int & in excresType, const double & in excresVal)", asFUNCTION( vsp::AddExcrescence ), asCALL_CDECL );
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "void DeleteExcrescence(const int & in excresName)", asFUNCTION( vsp::DeleteExcrescence ), asCALL_CDECL );
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "void UpdateParasiteDrag()", asFUNCTION( vsp::UpdateParasiteDrag ), asCALL_CDECL );
    assert( r >= 0 ); // TODO: Example


    r = se->RegisterGlobalFunction( "void WriteAtmosphereCSVFile( const string & in file_name, const int & in atmos_type )", asFUNCTION( vsp::WriteAtmosphereCSVFile ), asCALL_CDECL );
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "void CalcAtmosphere( const double & in alt, const double & in delta_temp, const int & in atmos_type, double & out temp, double & out pres, double & out pres_ratio, double & out rho_ratio )", asFUNCTION( vsp::CalcAtmosphere ), asCALL_CDECL );
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "void WriteBodyFFCSVFile( const string & in file_name )", asFUNCTION( vsp::WriteBodyFFCSVFile ), asCALL_CDECL );
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "void WriteWingFFCSVFile( const string & in file_name )", asFUNCTION( vsp::WriteWingFFCSVFile ), asCALL_CDECL );
    assert( r >= 0 );

     // TODO: Improve description
    r = se->RegisterGlobalFunction( "void WriteCfEqnCSVFile( const string & in file_name )", asFUNCTION( vsp::WriteCfEqnCSVFile ), asCALL_CDECL );
    assert( r >= 0 );

     // TODO: Improve description
    r = se->RegisterGlobalFunction( "void WritePartialCfMethodCSVFile( const string & in file_name )", asFUNCTION( vsp::WritePartialCfMethodCSVFile ), asCALL_CDECL );
    assert( r >= 0 );

    //=== Register Surface Query Functions ===//

    r = se->RegisterGlobalFunction( "vec3d CompPnt01( const string & in geom_id, const int & in surf_indx, const double & in u, const double & in w )", asFUNCTION(vsp::CompPnt01), asCALL_CDECL );
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "vec3d CompNorm01( const string & in geom_id, const int & in surf_indx, const double & in u, const double & in w )", asFUNCTION(vsp::CompNorm01), asCALL_CDECL );
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "vec3d CompTanU01( const string & in geom_id, const int & in surf_indx, const double & in u, const double & in w )", asFUNCTION(vsp::CompTanU01), asCALL_CDECL );
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "vec3d CompTanW01( const string & in geom_id, const int & in surf_indx, const double & in u, const double & in w )", asFUNCTION(vsp::CompTanW01), asCALL_CDECL );
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "void CompCurvature01( const string & in geom_id, const int & in surf_indx, const double & in u, const double & in w, double & out k1, double & out k2, double & out ka, double & out kg )", asFUNCTION(vsp::CompCurvature01), asCALL_CDECL );
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "double ProjPnt01( const string & in geom_id, const int & in surf_indx, const vec3d & in pt, double & out u, double & out w )", asFUNCTION(vsp::ProjPnt01), asCALL_CDECL );
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "double ProjPnt01I( const string & in geom_id, const vec3d & in pt, int & out surf_indx, double & out u, double & out w )", asFUNCTION(vsp::ProjPnt01I), asCALL_CDECL );
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "double ProjPnt01Guess( const string & in geom_id, const int & in surf_indx, const vec3d & in pt, const double & in u0, const double & in w0, double & out u, double & out w )", asFUNCTION(vsp::ProjPnt01Guess), asCALL_CDECL );
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "double AxisProjPnt01( const string & in geom_id, const int & in surf_indx, const int & in iaxis, const vec3d & in pt, double & out u_out, double & out w_out )", asFUNCTION(vsp::AxisProjPnt01), asCALL_CDECL );
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "double AxisProjPnt01I( const string & in geom_id, const int & in iaxis, const vec3d & in pt, int & out surf_indx_out, double & out u_out, double & out w_out )", asFUNCTION(vsp::AxisProjPnt01I), asCALL_CDECL );
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "double AxisProjPnt01Guess( const string & in geom_id, const int & in surf_indx, const int & in iaxis, const vec3d & in pt, const double & in u0, const double & in w0, double & out u_out, double & out w_out )", asFUNCTION(vsp::AxisProjPnt01Guess), asCALL_CDECL );
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "bool InsideSurf( const string & in geom_id, const int & in surf_indx, const vec3d & in pt )", asFUNCTION(vsp::InsideSurf), asCALL_CDECL );
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "double FindRST( const string & in geom_id, const int & in surf_indx, const vec3d & in pt, double & out r, double & out s, double & out t )", asFUNCTION(vsp::FindRST), asCALL_CDECL );
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "double FindRSTGuess( const string & in geom_id, const int & in surf_indx, const vec3d & in pt, const double & in r0, const double & in s0, const double & in t0, double & out r, double & out s, double & out t )", asFUNCTION(vsp::FindRSTGuess), asCALL_CDECL );
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "vec3d CompPntRST( const string & in geom_id, const int & in surf_indx, const double & in r, const double & in s, const double & in t )", asFUNCTION(vsp::CompPntRST), asCALL_CDECL );
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "array<vec3d>@+ CompVecPntRST(const string & in geom_id, const int & in surf_indx, array<double>@+ rs, array<double>@+ ss, array<double>@+ ts )", asMETHOD( ScriptMgrSingleton, CompVecPntRST ), asCALL_THISCALL_ASGLOBAL, &ScriptMgr );
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "void ConvertRSTtoLMN( const string & in geom_id, const int & in surf_indx, const double & in r, const double & in s, const double & in t, double & out l_out, double & out m_out, double & out n_out )", asFUNCTION(vsp::ConvertRSTtoLMN), asCALL_CDECL );
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "void ConvertRtoL( const string & in geom_id, const int & in surf_indx, const double & in r, double & out l_out )", asFUNCTION(vsp::ConvertRtoL), asCALL_CDECL );
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "void ConvertLMNtoRST( const string & in geom_id, const int & in surf_indx, const double & in l, const double & in m, const double & in n, double & out r_out, double & out s_out, double & out t_out )", asFUNCTION(vsp::ConvertLMNtoRST), asCALL_CDECL );
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "void ConvertLtoR( const string & in geom_id, const int & in surf_indx, const double & in l, double & out r_out )", asFUNCTION(vsp::ConvertLtoR), asCALL_CDECL );
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "void ConvertUtoEta( const string & in geom_id, const double & in u, double & out eta )", asFUNCTION(vsp::ConvertUtoEta), asCALL_CDECL );
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "void ConvertEtatoU( const string & in geom_id, const double & in eta, double & out u_out )", asFUNCTION(vsp::ConvertEtatoU), asCALL_CDECL );
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "void ConvertRSTtoLMNVec(const string & in geom_id, const int & in surf_indx, array<double>@+ rs, array<double>@+ ss, array<double>@+ ts, array<double>@+ ls, array<double>@+ ms, array<double>@+ ns )", asMETHOD( ScriptMgrSingleton, ConvertRSTtoLMNVec ), asCALL_THISCALL_ASGLOBAL, &ScriptMgr );
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "void ConvertLMNtoRSTVec(const string & in geom_id, const int & in surf_indx, array<double>@+ ls, array<double>@+ ms, array<double>@+ ns, array<double>@+ rs, array<double>@+ ss, array<double>@+ ts )", asMETHOD( ScriptMgrSingleton, ConvertLMNtoRSTVec ), asCALL_THISCALL_ASGLOBAL, &ScriptMgr );
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "void GetUWTess01(const string & in geom_id, int & in surf_indx, array<double>@+ us, array<double>@+ ws )", asMETHOD( ScriptMgrSingleton, GetUWTess01 ), asCALL_THISCALL_ASGLOBAL, &ScriptMgr );
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "array<vec3d>@+ CompVecPnt01(const string & in geom_id, const int & in surf_indx, array<double>@+ us, array<double>@+ ws )", asMETHOD( ScriptMgrSingleton, CompVecPnt01 ), asCALL_THISCALL_ASGLOBAL, &ScriptMgr );
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "array<vec3d>@+ CompVecDegenPnt01(const string & in geom_id, const int & in surf_indx, const int & in degen_type, array<double>@+ us, array<double>@+ ws )", asMETHOD( ScriptMgrSingleton, CompVecDegenPnt01 ), asCALL_THISCALL_ASGLOBAL, &ScriptMgr );
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "array<vec3d>@+ CompVecNorm01(const string & in geom_id, const int & in surf_indx, array<double>@+ us, array<double>@+ ws )", asMETHOD( ScriptMgrSingleton, CompVecNorm01 ), asCALL_THISCALL_ASGLOBAL, &ScriptMgr );
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "void CompVecCurvature01(const string & in geom_id, const int & in surf_indx, array<double>@+ us, array<double>@+ ws, array<double>@+ k1s, array<double>@+ k2s, array<double>@+ kas, array<double>@+ kgs)", asMETHOD( ScriptMgrSingleton, CompVecCurvature01 ), asCALL_THISCALL_ASGLOBAL, &ScriptMgr );
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "void ProjVecPnt01(const string & in geom_id, const int & in surf_indx, array<vec3d>@+ pts, array<double>@+ us, array<double>@+ ws, array<double>@+ ds )", asMETHOD( ScriptMgrSingleton, ProjVecPnt01 ), asCALL_THISCALL_ASGLOBAL, &ScriptMgr );
    assert( r >= 0 );

    r = se->RegisterGlobalFunction( "void ProjVecPnt01Guess(const string & in geom_id, const int & in surf_indx, array<vec3d>@+ pts, array<double>@+ u0s, array<double>@+ w0s, array<double>@+ us, array<double>@+ ws, array<double>@+ ds )", asMETHOD( ScriptMgrSingleton, ProjVecPnt01Guess ), asCALL_THISCALL_ASGLOBAL, &ScriptMgr );
    assert( r >= 0 );

    r = se->RegisterGlobalFunction( "void AxisProjVecPnt01(const string & in geom_id, const int & in surf_indx, const int & in iaxis, array<vec3d>@+ pts, array<double>@+ us, array<double>@+ ws, array<double>@+ ds )", asMETHOD( ScriptMgrSingleton, AxisProjVecPnt01 ), asCALL_THISCALL_ASGLOBAL, &ScriptMgr );
    assert( r >= 0 );

    r = se->RegisterGlobalFunction( "void AxisProjVecPnt01Guess(const string & in geom_id, int & in surf_indx, const int & in iaxis, array<vec3d>@+ pts, array<double>@+ u0s, array<double>@+ w0s, array<double>@+ us, array<double>@+ ws, array<double>@+ ds )", asMETHOD( ScriptMgrSingleton, AxisProjVecPnt01Guess ), asCALL_THISCALL_ASGLOBAL, &ScriptMgr );
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "array<bool>@+ VecInsideSurf( const string & in geom_id, const int & in surf_indx, array<vec3d>@+ pts )", asMETHOD( ScriptMgrSingleton, VecInsideSurf ), asCALL_THISCALL_ASGLOBAL, &ScriptMgr );
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "void FindRSTVec(const string & in geom_id, const int & in surf_indx, array<vec3d>@+ pts, array<double>@+ rs, array<double>@+ ss, array<double>@+ ts, array<double>@+ ds )", asMETHOD( ScriptMgrSingleton, FindRSTVec ), asCALL_THISCALL_ASGLOBAL, &ScriptMgr );
    assert( r >= 0 );

    r = se->RegisterGlobalFunction( "void FindRSTVecGuess(const string & in geom_id, const int & in surf_indx, array<vec3d>@+ pts, array<double>@+ r0s, array<double>@+ s0s, array<double>@+ t0s, array<double>@+ rs, array<double>@+ ss, array<double>@+ ts, array<double>@+ ds )", asMETHOD( ScriptMgrSingleton, FindRSTVecGuess ), asCALL_THISCALL_ASGLOBAL, &ScriptMgr );
    assert( r >= 0 );

    //=== Register Measure Functions ===//

    r = se->RegisterGlobalFunction( "string AddRuler( const string & in startgeomid, int startsurfindx, double startu, double startw, const string & in endgeomid, int endsurfindx, double endu, double endw, const string & in name )", asFUNCTION( vsp::AddRuler ), asCALL_CDECL );
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "array<string>@+ GetAllRulers()", asMETHOD( ScriptMgrSingleton, GetAllRulers ), asCALL_THISCALL_ASGLOBAL, &ScriptMgr);
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "void DelRuler( const string & in id )", asFUNCTION( vsp::DelRuler ), asCALL_CDECL);
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "void DeleteAllRulers()", asFUNCTION( vsp::DeleteAllRulers ), asCALL_CDECL);
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "string AddProbe( const string & in geomid, int surfindx, double u, double w, const string & in name )", asFUNCTION( vsp::AddProbe ), asCALL_CDECL);
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "array<string>@+ GetAllProbes()", asMETHOD( ScriptMgrSingleton, GetAllProbes ), asCALL_THISCALL_ASGLOBAL, &ScriptMgr);
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "void DelProbe( const string & in id )", asFUNCTION( vsp::DelProbe ), asCALL_CDECL);
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "void DeleteAllProbes()", asFUNCTION( vsp::DeleteAllProbes ), asCALL_CDECL);
    assert( r >= 0 );

    //=== Register FeaStructure and FEA Mesh Functions ====//

    r = se->RegisterGlobalFunction( "int AddFeaStruct( const string & in geom_id, bool init_skin = true, int surfindex = 0 )", asFUNCTION( vsp::AddFeaStruct ), asCALL_CDECL );
    assert( r >= 0 ); // TODO: Force init_skin to true always


    r = se->RegisterGlobalFunction( "void DeleteFeaStruct( const string & in geom_id, int fea_struct_ind )", asFUNCTION( vsp::DeleteFeaStruct ), asCALL_CDECL );
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "void SetFeaMeshStructIndex( int struct_index )", asFUNCTION( vsp::SetFeaMeshStructIndex ), asCALL_CDECL);
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "string GetFeaStructID( const string & in geom_id, int fea_struct_ind )", asFUNCTION( vsp::GetFeaStructID ), asCALL_CDECL );
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "int GetFeaStructIndex( const string & in struct_id )", asFUNCTION( vsp::GetFeaStructIndex ), asCALL_CDECL );
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "string GetFeaStructParentGeomID( const string & in struct_id )", asFUNCTION( vsp::GetFeaStructParentGeomID ), asCALL_CDECL );
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "string GetFeaStructName( const string & in geom_id, int fea_struct_ind )", asFUNCTION( vsp::GetFeaStructName ), asCALL_CDECL );
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "void SetFeaStructName( const string & in geom_id, int fea_struct_ind, const string & in name )", asFUNCTION( vsp::SetFeaStructName ), asCALL_CDECL );
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "array<string>@+ GetFeaStructIDVec()", asMETHOD( ScriptMgrSingleton, GetFeaStructIDVec ), asCALL_THISCALL_ASGLOBAL, &ScriptMgr );
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "void SetFeaPartName( const string & in part_id, const string & in name )", asFUNCTION( vsp::SetFeaPartName ), asCALL_CDECL );
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "void SetFeaMeshVal( const string & in geom_id, int fea_struct_ind, int type, double val )", asFUNCTION( vsp::SetFeaMeshVal ), asCALL_CDECL );
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "void SetFeaMeshFileName( const string & in geom_id, int fea_struct_ind, int file_type, const string & in file_name )", asFUNCTION( vsp::SetFeaMeshFileName ), asCALL_CDECL );
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "void ComputeFeaMesh( const string & in geom_id, int fea_struct_ind, int file_type )", asFUNCTIONPR( vsp::ComputeFeaMesh, ( const string & in, int, int ), void ), asCALL_CDECL );
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "void ComputeFeaMesh( const string & in struct_id, int file_type )", asFUNCTIONPR( vsp::ComputeFeaMesh, ( const string & in, int ), void ), asCALL_CDECL );
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "string AddFeaPart( const string & in geom_id, int fea_struct_ind, int type )", asFUNCTION( vsp::AddFeaPart ), asCALL_CDECL );
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "void DeleteFeaPart( const string & in geom_id, int fea_struct_ind, const string & in part_id )", asFUNCTION( vsp::DeleteFeaPart ), asCALL_CDECL );
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "string GetFeaPartID( const string & in fea_struct_id, int fea_part_index )", asFUNCTION( vsp::GetFeaPartID ), asCALL_CDECL );
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "string GetFeaPartName( const string & in part_id )", asFUNCTION( vsp::GetFeaPartName ), asCALL_CDECL );
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "int GetFeaPartType( const string & in part_id )", asFUNCTION( vsp::GetFeaPartType ), asCALL_CDECL );
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "int GetFeaSubSurfIndex( const string & in ss_id )", asFUNCTION( vsp::GetFeaSubSurfIndex ), asCALL_CDECL );
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "int NumFeaStructures()", asFUNCTION( vsp::NumFeaStructures ), asCALL_CDECL );
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "int NumFeaParts( const string & in fea_struct_id )", asFUNCTION( vsp::NumFeaParts ), asCALL_CDECL );
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "int NumFeaSubSurfs( const string & in fea_struct_id )", asFUNCTION( vsp::NumFeaSubSurfs ), asCALL_CDECL );
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "array<string>@+ GetFeaPartIDVec(const string & in fea_struct_id)", asMETHOD( ScriptMgrSingleton, GetFeaPartIDVec ), asCALL_THISCALL_ASGLOBAL, &ScriptMgr );
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "array<string>@+ GetFeaSubSurfIDVec(const string & in fea_struct_id)", asMETHOD( ScriptMgrSingleton, GetFeaSubSurfIDVec ), asCALL_THISCALL_ASGLOBAL, &ScriptMgr );
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "void SetFeaPartPerpendicularSparID( const string & in part_id, const string & in perpendicular_spar_id )", asFUNCTION( vsp::SetFeaPartPerpendicularSparID ), asCALL_CDECL );
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "string GetFeaPartPerpendicularSparID( const string & in part_id )", asFUNCTION( vsp::GetFeaPartPerpendicularSparID ), asCALL_CDECL );
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "void SetFeaSubSurfName( const string & in subsurf_id, const string & in name )", asFUNCTION( vsp::SetFeaSubSurfName ), asCALL_CDECL );
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "string GetFeaSubSurfName( const string & in subsurf_id )", asFUNCTION( vsp::GetFeaSubSurfName ), asCALL_CDECL );
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "string AddFeaSubSurf( const string & in geom_id, int fea_struct_ind, int type )", asFUNCTION( vsp::AddFeaSubSurf ), asCALL_CDECL );
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "void DeleteFeaSubSurf( const string & in geom_id, int fea_struct_ind, const string & in ss_id )", asFUNCTION( vsp::DeleteFeaSubSurf ), asCALL_CDECL );
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "string AddFeaBC( const string & in fea_struct_id, int type )", asFUNCTION( vsp::AddFeaBC ), asCALL_CDECL );
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "void DelFeaBC( const string & in fea_struct_id, const string & in ss_id )", asFUNCTION( vsp::DelFeaBC ), asCALL_CDECL );
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "array<string>@+ GetFeaBCIDVec(const string & in fea_struct_id)", asMETHOD( ScriptMgrSingleton, GetFeaBCIDVec ), asCALL_THISCALL_ASGLOBAL, &ScriptMgr );
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "int NumFeaBCs( const string & in fea_struct_id )", asFUNCTION( vsp::NumFeaBCs ), asCALL_CDECL );
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "string AddFeaMaterial()", asFUNCTION( vsp::AddFeaMaterial ), asCALL_CDECL );
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "string AddFeaProperty( int property_type = 0 )", asFUNCTION( vsp::AddFeaProperty ), asCALL_CDECL );
    assert( r >= 0 );
}

void ScriptMgrSingleton::RegisterUtility( asIScriptEngine* se )
{
    int r;
    //==== Register Utility Functions ====//

    r = se->RegisterGlobalFunction( "void Print(const string & in data, bool new_line = true )", asMETHODPR( ScriptMgrSingleton, Print, (const string &, bool), void ), asCALL_THISCALL_ASGLOBAL, &ScriptMgr );
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "void Print(const vec3d & in data, bool new_line = true )", asMETHODPR( ScriptMgrSingleton, Print, (const vec3d &, bool), void ), asCALL_THISCALL_ASGLOBAL, &ScriptMgr );
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "void Print(double data, bool new_line = true )", asMETHODPR( ScriptMgrSingleton, Print, (double, bool), void ), asCALL_THISCALL_ASGLOBAL, &ScriptMgr );
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "void Print(int data, bool new_line = true )", asMETHODPR( ScriptMgrSingleton, Print, (int, bool), void ), asCALL_THISCALL_ASGLOBAL, &ScriptMgr );
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "double Min( double x, double y)", asMETHOD( ScriptMgrSingleton, Min ), asCALL_THISCALL_ASGLOBAL, &ScriptMgr);
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "double Max( double x, double y)", asMETHOD( ScriptMgrSingleton, Max ), asCALL_THISCALL_ASGLOBAL, &ScriptMgr);
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "double Rad2Deg( double r )", asMETHOD( ScriptMgrSingleton, Rad2Deg ), asCALL_THISCALL_ASGLOBAL, &ScriptMgr);
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "double Deg2Rad( double d )", asMETHOD( ScriptMgrSingleton, Deg2Rad ), asCALL_THISCALL_ASGLOBAL, &ScriptMgr);
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "string GetVSPVersion( )", asFUNCTION( vsp::GetVSPVersion ), asCALL_CDECL);
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "int GetVSPVersionMajor( )", asFUNCTION( vsp::GetVSPVersionMajor ), asCALL_CDECL);
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "int GetVSPVersionMinor( )", asFUNCTION( vsp::GetVSPVersionMinor ), asCALL_CDECL);
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "int GetVSPVersionChange( )", asFUNCTION( vsp::GetVSPVersionChange ), asCALL_CDECL);
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "string GetVSPExePath()", asFUNCTION( vsp::GetVSPExePath ), asCALL_CDECL);
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "bool SetVSPAEROPath( const string & in path )", asFUNCTION( vsp::SetVSPAEROPath ), asCALL_CDECL );
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "string GetVSPAEROPath()", asFUNCTION( vsp::GetVSPAEROPath ), asCALL_CDECL );
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "bool CheckForVSPAERO( const string & in path )", asFUNCTION( vsp::CheckForVSPAERO ), asCALL_CDECL );
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "bool SetVSPHelpPath( const string & in path )", asFUNCTION( vsp::SetVSPHelpPath ), asCALL_CDECL );
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "string GetVSPHelpPath()", asFUNCTION( vsp::GetVSPHelpPath ), asCALL_CDECL );
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "bool CheckForVSPHelp( const string & in path )", asFUNCTION( vsp::CheckForVSPHelp ), asCALL_CDECL );
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "void VSPCheckSetup()", asFUNCTION( vsp::VSPCheckSetup ), asCALL_CDECL);
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "void VSPRenew()", asFUNCTION( vsp::VSPRenew ), asCALL_CDECL);
    assert( r >= 0 );

    //====  Register Proxy Utility Functions ====//

    r = se->RegisterGlobalFunction( "array<vec3d>@+ GetProxyVec3dArray()", asMETHOD( ScriptMgrSingleton, GetProxyVec3dArray ), asCALL_THISCALL_ASGLOBAL, &ScriptMgr );
    assert( r >= 0 );

}

//===== Utility Functions Vec3d Proxy Array =====//
CScriptArray* ScriptMgrSingleton::GetProxyVec3dArray()
{
    //==== This Will Get Deleted By The Script Engine ====//
    CScriptArray* sarr = CScriptArray::Create( m_Vec3dArrayType, m_ProxyVec3dArray.size() );
    for ( int i = 0 ; i < ( int )sarr->GetSize() ; i++ )
    {
        sarr->SetValue( i, &m_ProxyVec3dArray[i] );
    }
    return sarr;
}

//==== Utility Functions String Proxy Array =====//
CScriptArray* ScriptMgrSingleton::GetProxyStringArray()
{
    //==== This Will Get Deleted By The Script Engine ====//
    CScriptArray* sarr = CScriptArray::Create( m_StringArrayType, m_ProxyStringArray.size() );
    for ( int i = 0 ; i < ( int )sarr->GetSize() ; i++ )
    {
        sarr->SetValue( i, &m_ProxyStringArray[i] );
    }
    return sarr;
}

//==== Utility Functions String Proxy Array =====//
CScriptArray* ScriptMgrSingleton::GetProxyIntArray()
{
    //==== This Will Get Deleted By The Script Engine ====//
    CScriptArray* sarr = CScriptArray::Create( m_IntArrayType, m_ProxyIntArray.size() );
    for ( int i = 0 ; i < ( int )sarr->GetSize() ; i++ )
    {
        sarr->SetValue( i, &m_ProxyIntArray[i] );
    }
    return sarr;
}

//==== Utility Functions String Proxy Array =====//
CScriptArray* ScriptMgrSingleton::GetProxyDoubleArray()
{
    //==== This Will Get Deleted By The Script Engine ====//
    CScriptArray* sarr = CScriptArray::Create( m_DoubleArrayType, m_ProxyDoubleArray.size() );
    for ( int i = 0 ; i < ( int )sarr->GetSize() ; i++ )
    {
        sarr->SetValue( i, &m_ProxyDoubleArray[i] );
    }
    return sarr;
}

CScriptArray* ScriptMgrSingleton::GetProxyIntMatArray()
{
    CScriptArray* sarr = CScriptArray::Create( m_IntMatArrayType, m_ProxyIntMatArray.size() );
    for ( int i = 0; i < ( int )sarr->GetSize(); i++ )
    {
        CScriptArray* darr = CScriptArray::Create( m_IntArrayType, m_ProxyIntMatArray[i].size() );
        for ( int j = 0; j < ( int )darr->GetSize(); j++ )
        {
            darr->SetValue( j, &m_ProxyIntMatArray[i][j]);
        }
        sarr->SetValue( i, darr );
    }
    return sarr;
}

CScriptArray* ScriptMgrSingleton::GetProxyDoubleMatArray()
{
    CScriptArray* sarr = CScriptArray::Create( m_DoubleMatArrayType, m_ProxyDoubleMatArray.size() );
    for ( int i = 0; i < ( int )sarr->GetSize(); i++ )
    {
        CScriptArray* darr = CScriptArray::Create( m_DoubleArrayType, m_ProxyDoubleMatArray[i].size() );
        for ( int j = 0; j < ( int )darr->GetSize(); j++ )
        {
            darr->SetValue( j, &m_ProxyDoubleMatArray[i][j]);
        }
        sarr->SetValue( i, darr );
    }
    return sarr;
}

template < class T >
void ScriptMgrSingleton::FillASArray( vector < T > & in, CScriptArray* out )
{
    out->Resize( in.size() );
    for ( int i = 0 ; i < ( int )in.size() ; i++ )
    {
        out->SetValue( i, &in[i] );
    }
}

template < class T >
void ScriptMgrSingleton::FillSTLVector( CScriptArray* in, vector < T > & out )
{
    out.resize( in->GetSize() );
    for ( int i = 0 ; i < ( int )in->GetSize() ; i++ )
    {
        out[i] = * ( T* ) ( in->At( i ) );
    }
}

// maybe don't template this fill matrix- do a FillDoubleMatrix and FillIntMatrix?
template < class T >
void ScriptMgrSingleton::FillSTLMatrix( CScriptArray* in, vector < vector < T > > & out )
{
    out.resize( in->GetSize() );
    for ( int i = 0 ; i < ( int )in->GetSize() ; i++ )
    {
        CScriptArray* row = ( CScriptArray* ) ( in->At( i ) );
        if ( row )
        {
            FillSTLVector( row, out[i] );
        }
    }
}

//==== Wrappers For API Functions That Return Vectors ====//
CScriptArray* ScriptMgrSingleton::GetGeomTypes()
{
    m_ProxyStringArray = vsp::GetGeomTypes();
    return GetProxyStringArray();
}

CScriptArray* ScriptMgrSingleton::PasteGeomClipboard( const string & parent )
{
    m_ProxyStringArray = vsp::PasteGeomClipboard( parent );
    return GetProxyStringArray();
}

CScriptArray* ScriptMgrSingleton::FindGeoms()
{
    m_ProxyStringArray = vsp::FindGeoms();
    return GetProxyStringArray();
}

CScriptArray* ScriptMgrSingleton::FindGeomsWithName( const string & name )
{
    m_ProxyStringArray = vsp::FindGeomsWithName( name );
    return GetProxyStringArray();
}

CScriptArray* ScriptMgrSingleton::GetGeomParmIDs( const string & geom_id )
{
    m_ProxyStringArray = vsp::GetGeomParmIDs( geom_id );
    return GetProxyStringArray();
}

CScriptArray* ScriptMgrSingleton::GetGeomChildren( const string & geom_id )
{
    m_ProxyStringArray = vsp::GetGeomChildren( geom_id );
    return GetProxyStringArray();
}

CScriptArray* ScriptMgrSingleton::GetSubSurfIDVec( const string & geom_id )
{
    m_ProxyStringArray = vsp::GetSubSurfIDVec( geom_id );
    return GetProxyStringArray();
}

CScriptArray* ScriptMgrSingleton::GetAllSubSurfIDs()
{
    m_ProxyStringArray = vsp::GetAllSubSurfIDs();
    return GetProxyStringArray();
}

CScriptArray* ScriptMgrSingleton::GetSubSurf( const string & geom_id, const string & name )
{
    m_ProxyStringArray = vsp::GetSubSurf( geom_id, name );
    return GetProxyStringArray();
}

CScriptArray* ScriptMgrSingleton::GetSubSurfParmIDs( const string & sub_id )
{
    m_ProxyStringArray = vsp::GetSubSurfParmIDs( sub_id );
    return GetProxyStringArray();
}

CScriptArray* ScriptMgrSingleton::GetActiveCSNameVec( int CSGroupIndex )
{
    m_ProxyStringArray = vsp::GetActiveCSNameVec( CSGroupIndex );
    return GetProxyStringArray();
}

CScriptArray* ScriptMgrSingleton::GetCompleteCSNameVec( )
{
    m_ProxyStringArray = vsp::GetCompleteCSNameVec( );
    return GetProxyStringArray();
}

CScriptArray* ScriptMgrSingleton::GetAvailableCSNameVec( int CSGroupIndex )
{
    m_ProxyStringArray = vsp::GetAvailableCSNameVec( CSGroupIndex );
    return GetProxyStringArray();
}

void ScriptMgrSingleton::AddSelectedToCSGroup( CScriptArray* selected, int CSGroupIndex )
{
    vector < int > int_vec;
    FillSTLVector( selected, int_vec );

    vsp::AddSelectedToCSGroup( int_vec, CSGroupIndex );
}

void ScriptMgrSingleton::RemoveSelectedFromCSGroup( CScriptArray* selected, int CSGroupIndex )
{
    vector < int > int_vec;
    FillSTLVector( selected, int_vec );

    vsp::RemoveSelectedFromCSGroup( int_vec, CSGroupIndex );
}

CScriptArray* ScriptMgrSingleton::GetUnsteadyGroupCompIDs( int group_index )
{
    m_ProxyStringArray = vsp::GetUnsteadyGroupCompIDs( group_index );
    return GetProxyStringArray();
}

CScriptArray* ScriptMgrSingleton::GetUnsteadyGroupSurfIndexes( int group_index )
{
    m_ProxyIntArray = vsp::GetUnsteadyGroupSurfIndexes( group_index );
    return GetProxyIntArray();
}

CScriptArray* ScriptMgrSingleton::GetXSecParmIDs( const string & xsec_id )
{
    m_ProxyStringArray = vsp::GetXSecParmIDs( xsec_id );
    return GetProxyStringArray();
}

CScriptArray* ScriptMgrSingleton::ReadFileXSec( const string& xsec_id, const string& file_name )
{
    m_ProxyVec3dArray = vsp::ReadFileXSec( xsec_id, file_name );
    return GetProxyVec3dArray();
}

CScriptArray* ScriptMgrSingleton::GetAirfoilUpperPnts( const string& xsec_id )
{
    m_ProxyVec3dArray = vsp::GetAirfoilUpperPnts( xsec_id );
    return GetProxyVec3dArray();
}

CScriptArray* ScriptMgrSingleton::GetAirfoilLowerPnts( const string& xsec_id )
{
    m_ProxyVec3dArray = vsp::GetAirfoilLowerPnts( xsec_id );
    return GetProxyVec3dArray();
}

CScriptArray* ScriptMgrSingleton::ReadBORFileXSec( const string& bor_id, const string& file_name )
{
    m_ProxyVec3dArray = vsp::ReadBORFileXSec( bor_id, file_name );
    return GetProxyVec3dArray();
}

CScriptArray* ScriptMgrSingleton::GetBORAirfoilUpperPnts( const string& bor_id )
{
    m_ProxyVec3dArray = vsp::GetBORAirfoilUpperPnts( bor_id );
    return GetProxyVec3dArray();
}

CScriptArray* ScriptMgrSingleton::GetBORAirfoilLowerPnts( const string& bor_id )
{
    m_ProxyVec3dArray = vsp::GetBORAirfoilLowerPnts( bor_id );
    return GetProxyVec3dArray();
}

CScriptArray* ScriptMgrSingleton::GetGeomSetAtIndex( int index )
{
    m_ProxyStringArray = vsp::GetGeomSetAtIndex( index );
    return GetProxyStringArray();
}

CScriptArray* ScriptMgrSingleton::GetGeomSet( const string & name )
{
    m_ProxyStringArray = vsp::GetGeomSet( name );
    return GetProxyStringArray();
}

CScriptArray* ScriptMgrSingleton::GetAllBackground3Ds()
{
    m_ProxyStringArray = vsp::GetAllBackground3Ds();
    return GetProxyStringArray();
}

CScriptArray* ScriptMgrSingleton::GetAllBackground3DRelativePaths()
{
    m_ProxyStringArray = vsp::GetAllBackground3DRelativePaths();
    return GetProxyStringArray();
}

CScriptArray* ScriptMgrSingleton::GetAllBackground3DAbsolutePaths()
{
    m_ProxyStringArray = vsp::GetAllBackground3DAbsolutePaths();
    return GetProxyStringArray();
}

CScriptArray* ScriptMgrSingleton::GetMaterialNames()
{
    m_ProxyStringArray = vsp::GetMaterialNames();
    return GetProxyStringArray();
}

CScriptArray* ScriptMgrSingleton::ListAnalysis()
{
    m_ProxyStringArray = vsp::ListAnalysis();
    return GetProxyStringArray();
}

CScriptArray* ScriptMgrSingleton::GetAnalysisInputNames( const string & analysis )
{
    m_ProxyStringArray = vsp::GetAnalysisInputNames( analysis );
    return GetProxyStringArray();
}

CScriptArray* ScriptMgrSingleton::GetIntAnalysisInput( const string & analysis, const string & name, int index )
{
    m_ProxyIntArray = vsp::GetIntAnalysisInput( analysis, name, index );
    return GetProxyIntArray();
}

CScriptArray* ScriptMgrSingleton::GetDoubleAnalysisInput( const string & analysis, const string & name, int index )
{
    m_ProxyDoubleArray = vsp::GetDoubleAnalysisInput( analysis, name, index );
    return GetProxyDoubleArray();
}

CScriptArray* ScriptMgrSingleton::GetStringAnalysisInput( const string & analysis, const string & name, int index )
{
    m_ProxyStringArray = vsp::GetStringAnalysisInput( analysis, name, index );
    return GetProxyStringArray();
}

CScriptArray* ScriptMgrSingleton::GetVec3dAnalysisInput( const string & analysis, const string & name, int index )
{
    m_ProxyVec3dArray = vsp::GetVec3dAnalysisInput( analysis, name, index );
    return GetProxyVec3dArray();
}

CScriptArray* ScriptMgrSingleton::FindAllAttributes()
{
    m_ProxyStringArray = vsp::FindAllAttributes();
    return GetProxyStringArray();
}

CScriptArray* ScriptMgrSingleton::FindAttributesByName( const string & search_str )
{
    m_ProxyStringArray = vsp::FindAttributesByName( search_str );
    return GetProxyStringArray();
}

CScriptArray* ScriptMgrSingleton::FindAttributedObjects()
{
    m_ProxyStringArray = vsp::FindAttributedObjects();
    return GetProxyStringArray();
}

CScriptArray* ScriptMgrSingleton::FindAttributesInCollection( const string &collID )
{
    m_ProxyStringArray = vsp::FindAttributesInCollection( collID );
    return GetProxyStringArray();
}

CScriptArray* ScriptMgrSingleton::FindAttributeNamesInCollection( const string & collID )
{
    m_ProxyStringArray = vsp::FindAttributeNamesInCollection( collID );
    return GetProxyStringArray();
}

CScriptArray* ScriptMgrSingleton::GetAttributeBoolVal( const string & attrID )
{
    m_ProxyIntArray = vsp::GetAttributeBoolVal( attrID );
    return GetProxyIntArray();
}

CScriptArray* ScriptMgrSingleton::GetAttributeIntVal( const string & attrID )
{
    m_ProxyIntArray = vsp::GetAttributeIntVal( attrID );
    return GetProxyIntArray();
}

CScriptArray* ScriptMgrSingleton::GetAttributeDoubleVal( const string & attrID )
{
    m_ProxyDoubleArray = vsp::GetAttributeDoubleVal( attrID );
    return GetProxyDoubleArray();
}

CScriptArray* ScriptMgrSingleton::GetAttributeStringVal( const string & attrID )
{
    m_ProxyStringArray = vsp::GetAttributeStringVal( attrID );
    return GetProxyStringArray();
}

CScriptArray* ScriptMgrSingleton::GetAttributeParmID( const string & attrID )
{
    m_ProxyStringArray = vsp::GetAttributeParmID( attrID );
    return GetProxyStringArray();
}

CScriptArray* ScriptMgrSingleton::GetAttributeParmVal( const string & attrID )
{
    m_ProxyDoubleArray = vsp::GetAttributeParmVal( attrID );
    return GetProxyDoubleArray();
}

CScriptArray* ScriptMgrSingleton::GetAttributeParmName( const string & attrID )
{
    m_ProxyStringArray = vsp::GetAttributeParmName( attrID );
    return GetProxyStringArray();
}

CScriptArray* ScriptMgrSingleton::GetAttributeVec3dVal( const string & attrID )
{
    m_ProxyVec3dArray = vsp::GetAttributeVec3dVal( attrID );
    return GetProxyVec3dArray();
}

CScriptArray* ScriptMgrSingleton::GetAttributeIntMatrixVal( const string & attrID )
{
    m_ProxyIntMatArray = vsp::GetAttributeIntMatrixVal( attrID );
    return GetProxyIntMatArray();
}

CScriptArray* ScriptMgrSingleton::GetAttributeDoubleMatrixVal( const string & attrID )
{
    m_ProxyDoubleMatArray = vsp::GetAttributeDoubleMatrixVal( attrID );
    return GetProxyDoubleMatArray();
}

void ScriptMgrSingleton::SetAttributeVec3d( const string & attrID, CScriptArray* vec3dVector )
{
    vector < vec3d > vec3d_vec;
    FillSTLVector( vec3dVector, vec3d_vec );
    vsp::SetAttributeVec3d( attrID, vec3d_vec );
}

void ScriptMgrSingleton::SetAttributeIntMatrix( const string & attrID, CScriptArray* intMatrix )
{
    vector < vector < int > > int_mat;
    FillSTLMatrix( intMatrix, int_mat );
    vsp::SetAttributeIntMatrix( attrID, int_mat );
}

void ScriptMgrSingleton::SetAttributeDoubleMatrix( const string & attrID, CScriptArray* dblMatrix )
{
    vector < vector < double > > dbl_mat;
    FillSTLMatrix( dblMatrix, dbl_mat );
    vsp::SetAttributeDoubleMatrix( attrID, dbl_mat );
}

string ScriptMgrSingleton::AddAttributeVec3d( const string & attrID, const string & attributeName, CScriptArray* vec3dVector )
{
    vector < vec3d > vec3d_vec;
    FillSTLVector( vec3dVector, vec3d_vec );
    string id = vsp::AddAttributeVec3d( attrID, attributeName, vec3d_vec );
    return id;
}

string ScriptMgrSingleton::AddAttributeIntMatrix( const string & collID, const string & attributeName, CScriptArray* intMatrix )
{
    vector < vector < int > > int_mat;
    FillSTLMatrix( intMatrix, int_mat );
    string id = vsp::AddAttributeIntMatrix( collID, attributeName, int_mat );
    return id;
}

string ScriptMgrSingleton::AddAttributeDoubleMatrix( const string & collID, const string & attributeName, CScriptArray* dblMatrix )
{
    vector < vector < double > > dbl_mat;
    FillSTLMatrix( dblMatrix, dbl_mat );
    string id = vsp::AddAttributeDoubleMatrix( collID, attributeName, dbl_mat );
    return id;
}

CScriptArray* ScriptMgrSingleton::PasteAttribute( const string & collID )
{
    m_ProxyStringArray = vsp::PasteAttribute( collID );
    return GetProxyStringArray();
}

CScriptArray* ScriptMgrSingleton::GetAllResultsNames()
{
    m_ProxyStringArray = vsp::GetAllResultsNames();
    return GetProxyStringArray();
}

CScriptArray* ScriptMgrSingleton::GetAllDataNames( const string & results_id )
{
    m_ProxyStringArray = vsp::GetAllDataNames( results_id );
    return GetProxyStringArray();
}

CScriptArray* ScriptMgrSingleton::GetIntResults( const string & id, const string & name, int index )
{
    m_ProxyIntArray = vsp::GetIntResults( id, name, index );
    return GetProxyIntArray();
}

CScriptArray* ScriptMgrSingleton::GetDoubleResults( const string & id, const string & name, int index )
{
    m_ProxyDoubleArray = vsp::GetDoubleResults( id, name, index );
    return GetProxyDoubleArray();
}

CScriptArray* ScriptMgrSingleton::GetDoubleMatResults(const string &id, const string &name, int index )
{
    m_ProxyDoubleMatArray = vsp::GetDoubleMatResults( id, name, index);
    return GetProxyDoubleMatArray();
}

CScriptArray* ScriptMgrSingleton::GetStringResults( const string & id, const string & name, int index )
{
    m_ProxyStringArray = vsp::GetStringResults( id, name, index );
    return GetProxyStringArray();
}

CScriptArray* ScriptMgrSingleton::GetVec3dResults( const string & id, const string & name, int index )
{
    m_ProxyVec3dArray = vsp::GetVec3dResults( id, name, index );
    return GetProxyVec3dArray();
}

CScriptArray* ScriptMgrSingleton::FindContainers()
{
    m_ProxyStringArray = vsp::FindContainers();
    return GetProxyStringArray();
}

CScriptArray* ScriptMgrSingleton::FindContainersWithName( const string & name )
{
    m_ProxyStringArray = vsp::FindContainersWithName( name );
    return GetProxyStringArray();
}

CScriptArray* ScriptMgrSingleton::FindContainerGroupNames( const string & parm_container_id )
{
    m_ProxyStringArray = vsp::FindContainerGroupNames( parm_container_id );
    return GetProxyStringArray();
}

CScriptArray* ScriptMgrSingleton::FindContainerParmIDs( const string & parm_container_id )
{
    m_ProxyStringArray = vsp::FindContainerParmIDs( parm_container_id );
    return GetProxyStringArray();
}

CScriptArray* ScriptMgrSingleton::GetAllUserParms()
{
    m_ProxyStringArray = vsp::GetAllUserParms();
    return GetProxyStringArray();
}

CScriptArray* ScriptMgrSingleton::GetUpperCSTCoefs( const string & xsec_id )
{
    m_ProxyDoubleArray = vsp::GetUpperCSTCoefs( xsec_id );
    return GetProxyDoubleArray();
}

CScriptArray* ScriptMgrSingleton::GetLowerCSTCoefs( const string & xsec_id )
{
    m_ProxyDoubleArray = vsp::GetLowerCSTCoefs( xsec_id );
    return GetProxyDoubleArray();
}

CScriptArray* ScriptMgrSingleton::GetAllRoutingPtIds( const string &routing_id )
{
    m_ProxyStringArray = vsp::GetAllRoutingPtIds( routing_id );
    return GetProxyStringArray();
}

CScriptArray* ScriptMgrSingleton::GetAllRoutingPtCoords( const string &routing_id, int symm_index )
{
    m_ProxyVec3dArray = vsp::GetAllRoutingPtCoords( routing_id, symm_index );
    return GetProxyVec3dArray();
}

CScriptArray* ScriptMgrSingleton::GetBORUpperCSTCoefs( const string & bor_id )
{
    m_ProxyDoubleArray = vsp::GetBORUpperCSTCoefs( bor_id );
    return GetProxyDoubleArray();
}

CScriptArray* ScriptMgrSingleton::GetBORLowerCSTCoefs( const string & bor_id )
{
    m_ProxyDoubleArray = vsp::GetBORLowerCSTCoefs( bor_id );
    return GetProxyDoubleArray();
}

void ScriptMgrSingleton::DeleteGeomVec( CScriptArray* del_arr )
{
    vector < string > del_vec;
    FillSTLVector( del_arr, del_vec );

    vsp::DeleteGeomVec( del_vec );
}

void ScriptMgrSingleton::SetXSecPnts( const string& xsec_id, CScriptArray* pnt_arr )
{
    vector< vec3d > pnt_vec;
    FillSTLVector( pnt_arr, pnt_vec );

    vsp::SetXSecPnts( xsec_id, pnt_vec );
}

void ScriptMgrSingleton::SetAirfoilUpperPnts( const string& xsec_id, CScriptArray* up_pnt_arr )
{
    vector< vec3d > up_pnt_vec;
    up_pnt_vec.resize( up_pnt_arr->GetSize() );
    for ( int i = 0 ; i < ( int )up_pnt_arr->GetSize() ; i++ )
    {
        up_pnt_vec[i] = * ( vec3d* )( up_pnt_arr->At( i ) );
    }

    vsp::SetAirfoilUpperPnts( xsec_id, up_pnt_vec );
}

void ScriptMgrSingleton::SetAirfoilLowerPnts( const string& xsec_id, CScriptArray* low_pnt_arr )
{
    vector< vec3d > low_pnt_vec;
    low_pnt_vec.resize( low_pnt_arr->GetSize() );
    for ( int i = 0 ; i < ( int )low_pnt_arr->GetSize() ; i++ )
    {
        low_pnt_vec[i] = * ( vec3d* )( low_pnt_arr->At( i ) );
    }

    vsp::SetAirfoilLowerPnts( xsec_id, low_pnt_vec );
}

void ScriptMgrSingleton::SetAirfoilPnts( const string& xsec_id, CScriptArray* up_pnt_arr, CScriptArray* low_pnt_arr )
{
    vector< vec3d > up_pnt_vec;
    FillSTLVector( up_pnt_arr, up_pnt_vec );

    vector< vec3d > low_pnt_vec;
    FillSTLVector( low_pnt_arr, low_pnt_vec );

    vsp::SetAirfoilPnts( xsec_id, up_pnt_vec, low_pnt_vec );
}

void ScriptMgrSingleton::SetBORXSecPnts( const string& bor_id, CScriptArray* pnt_arr )
{
    vector< vec3d > pnt_vec;
    FillSTLVector( pnt_arr, pnt_vec );

    vsp::SetBORXSecPnts( bor_id, pnt_vec );
}

void ScriptMgrSingleton::SetBORAirfoilUpperPnts( const string& bor_id, CScriptArray* up_pnt_arr )
{
    vector< vec3d > up_pnt_vec;
    up_pnt_vec.resize( up_pnt_arr->GetSize() );
    for ( int i = 0 ; i < ( int )up_pnt_arr->GetSize() ; i++ )
    {
        up_pnt_vec[i] = * ( vec3d* )( up_pnt_arr->At( i ) );
    }

    vsp::SetBORAirfoilUpperPnts( bor_id, up_pnt_vec );
}

void ScriptMgrSingleton::SetBORAirfoilLowerPnts( const string& bor_id, CScriptArray* low_pnt_arr )
{
    vector< vec3d > low_pnt_vec;
    low_pnt_vec.resize( low_pnt_arr->GetSize() );
    for ( int i = 0 ; i < ( int )low_pnt_arr->GetSize() ; i++ )
    {
        low_pnt_vec[i] = * ( vec3d* )( low_pnt_arr->At( i ) );
    }

    vsp::SetBORAirfoilLowerPnts( bor_id, low_pnt_vec );
}

void ScriptMgrSingleton::SetBORAirfoilPnts( const string& bor_id, CScriptArray* up_pnt_arr, CScriptArray* low_pnt_arr )
{
    vector< vec3d > up_pnt_vec;
    FillSTLVector( up_pnt_arr, up_pnt_vec );

    vector< vec3d > low_pnt_vec;
    FillSTLVector( low_pnt_arr, low_pnt_vec );

    vsp::SetBORAirfoilPnts( bor_id, up_pnt_vec, low_pnt_vec );
}

CScriptArray* ScriptMgrSingleton::GetAirfoilCoordinates( const std::string & geom_id, const double &foilsurf_u )
{
    m_ProxyVec3dArray = vsp::GetAirfoilCoordinates( geom_id, foilsurf_u );

    return GetProxyVec3dArray();
}

CScriptArray* ScriptMgrSingleton::GetHersheyBarLiftDist( const int &npts, const double &alpha, const double &Vinf, const double &span, bool full_span_flag )
{
    m_ProxyVec3dArray = vsp::GetHersheyBarLiftDist( npts, alpha, Vinf, span, full_span_flag );

    return GetProxyVec3dArray();
}

CScriptArray* ScriptMgrSingleton::GetHersheyBarDragDist( const int &npts, const double &alpha, const double &Vinf, const double &span, bool full_span_flag )
{
    m_ProxyVec3dArray = vsp::GetHersheyBarDragDist( npts, alpha, Vinf, span, full_span_flag );

    return GetProxyVec3dArray();
}

CScriptArray* ScriptMgrSingleton::GetVKTAirfoilPnts( const int &npts, const double &alpha, const double &epsilon, const double &kappa, const double &tau )
{
    m_ProxyVec3dArray = vsp::GetVKTAirfoilPnts( npts, alpha, epsilon, kappa, tau );

    return GetProxyVec3dArray();
}

CScriptArray* ScriptMgrSingleton::GetVKTAirfoilCpDist( const double &alpha, const double &epsilon, const double &kappa, const double &tau, CScriptArray* xyzdata )
{
    vector< vec3d > xyz_vec;
    FillSTLVector( xyzdata, xyz_vec );

    m_ProxyDoubleArray = vsp::GetVKTAirfoilCpDist( alpha, epsilon, kappa, tau, xyz_vec );

    return GetProxyDoubleArray();
}

CScriptArray* ScriptMgrSingleton::GetEllipsoidSurfPnts( const vec3d& center, const vec3d& abc_rad, int u_npts, int w_npts )
{
    m_ProxyVec3dArray = vsp::GetEllipsoidSurfPnts( center, abc_rad, u_npts, w_npts );

    return GetProxyVec3dArray();
}

CScriptArray* ScriptMgrSingleton::GetFeatureLinePnts( const string & geom_id )
{
    m_ProxyVec3dArray = vsp::GetFeatureLinePnts( geom_id );

    return GetProxyVec3dArray();
}

CScriptArray* ScriptMgrSingleton::GetEllipsoidCpDist( CScriptArray* surf_pnt_arr, const vec3d& abc_rad, const vec3d& V_inf )
{
    vector< vec3d > surf_pnt_vec;
    FillSTLVector( surf_pnt_arr, surf_pnt_vec );

    m_ProxyDoubleArray = vsp::GetEllipsoidCpDist( surf_pnt_vec, abc_rad, V_inf );

    return GetProxyDoubleArray();
}

void ScriptMgrSingleton::SetUpperCST( const string& xsec_id, int deg, CScriptArray* coefs_arr )
{
    vector < double > coefs_vec;
    FillSTLVector( coefs_arr, coefs_vec );

    vsp::SetUpperCST( xsec_id, deg, coefs_vec );
}

void ScriptMgrSingleton::SetLowerCST( const string& xsec_id, int deg, CScriptArray* coefs_arr )
{
    vector < double > coefs_vec;
    FillSTLVector( coefs_arr, coefs_vec );

    vsp::SetLowerCST( xsec_id, deg, coefs_vec );
}

void ScriptMgrSingleton::SetBORUpperCST( const string& bor_id, int deg, CScriptArray* coefs_arr )
{
    vector < double > coefs_vec;
    FillSTLVector( coefs_arr, coefs_vec );

    vsp::SetBORUpperCST( bor_id, deg, coefs_vec );
}

void ScriptMgrSingleton::SetBORLowerCST( const string& bor_id, int deg, CScriptArray* coefs_arr )
{
    vector < double > coefs_vec;
    FillSTLVector( coefs_arr, coefs_vec );

    vsp::SetBORLowerCST( bor_id, deg, coefs_vec );
}

//==== Edit Curve XSec Functions ====//
CScriptArray* ScriptMgrSingleton::GetEditXSecUVec( const std::string& xsec_id )
{
    m_ProxyDoubleArray = vsp::GetEditXSecUVec( xsec_id );

    return GetProxyDoubleArray();
}

CScriptArray* ScriptMgrSingleton::GetEditXSecCtrlVec( const std::string & xsec_id, bool non_dimensional )
{
    m_ProxyVec3dArray = vsp::GetEditXSecCtrlVec( xsec_id, non_dimensional );

    return GetProxyVec3dArray();
}

void ScriptMgrSingleton::SetEditXSecPnts( const string & xsec_id, CScriptArray* u_vec, CScriptArray* control_pts, CScriptArray* r_vec )
{
    vector < vec3d > control_pnt_vec;
    FillSTLVector( control_pts, control_pnt_vec );

    vector < double > new_u_vec;
    FillSTLVector( u_vec, new_u_vec );

    vector < double > new_r_vec;
    FillSTLVector( r_vec, new_r_vec );

    vsp::SetEditXSecPnts( xsec_id, new_u_vec, control_pnt_vec, new_r_vec );
}

CScriptArray* ScriptMgrSingleton::GetEditXSecFixedUVec( const std::string& xsec_id )
{
    vector < bool > temp_vec = vsp::GetEditXSecFixedUVec( xsec_id );

    m_ProxyIntArray.clear();
    m_ProxyIntArray.resize( temp_vec.size() );
    for ( size_t i = 0; i < temp_vec.size(); i++ )
    {
        // Cast bool to int
        m_ProxyIntArray[i] = (int)temp_vec[i];
    }

    return GetProxyIntArray();
}

void ScriptMgrSingleton::SetEditXSecFixedUVec( const string & xsec_id, CScriptArray* fixed_u_vec )
{
    vector < bool > new_fixed_u_vec;
    FillSTLVector( fixed_u_vec, new_fixed_u_vec );

    vsp::SetEditXSecFixedUVec( xsec_id, new_fixed_u_vec );
}

//==== Variable Preset Functions ====//

CScriptArray* ScriptMgrSingleton::GetVarPresetGroups()
{
    m_ProxyStringArray = vsp::GetVarPresetGroups();
    return GetProxyStringArray();
}

CScriptArray* ScriptMgrSingleton::GetVarPresetSettings( const string &group_id )
{
    m_ProxyStringArray = vsp::GetVarPresetSettings( group_id );
    return GetProxyStringArray();
}

CScriptArray* ScriptMgrSingleton::GetVarPresetParmIDs( const string &group_id )
{
    m_ProxyStringArray = vsp::GetVarPresetParmIDs( group_id );
    return GetProxyStringArray();
}

CScriptArray* ScriptMgrSingleton::GetVarPresetParmVals( const string &setting_id )
{
    m_ProxyDoubleArray = vsp::GetVarPresetParmVals( setting_id );
    return GetProxyDoubleArray();
}

void ScriptMgrSingleton::SetVarPresetParmVals( const string& setting_id, CScriptArray* coefs_arr )
{
    vector < double > coefs_vec;
    FillSTLVector( coefs_arr, coefs_vec );
    vsp::SetVarPresetParmVals( setting_id, coefs_vec );
}

//==== Mode Functions ====//
CScriptArray* ScriptMgrSingleton::GetAllModes()
{
    m_ProxyStringArray = vsp::GetAllModes();
    return GetProxyStringArray();
}

CScriptArray* ScriptMgrSingleton::ModeGetAllGroups( const string &mode_id )
{
    m_ProxyStringArray = vsp::ModeGetAllGroups( mode_id );
    return GetProxyStringArray();
}

CScriptArray* ScriptMgrSingleton::ModeGetAllSettings( const string &mode_id )
{
    m_ProxyStringArray = vsp::ModeGetAllSettings( mode_id );
    return GetProxyStringArray();
}

//==== PCurve Functions ====//
void ScriptMgrSingleton::SetPCurve( const string& geom_id, const int & pcurveid, CScriptArray* t_arr, CScriptArray* val_arr, const int & newtype )
{
    vector < double > t_vec;
    FillSTLVector( t_arr, t_vec );

    vector < double > val_vec;
    FillSTLVector( val_arr, val_vec );

    vsp::SetPCurve( geom_id, pcurveid, t_vec, val_vec, newtype );
}

CScriptArray* ScriptMgrSingleton::PCurveGetTVec( const std::string & geom_id, const int & pcurveid )
{
    m_ProxyDoubleArray = vsp::PCurveGetTVec( geom_id, pcurveid );
    return GetProxyDoubleArray();
}

CScriptArray* ScriptMgrSingleton::PCurveGetValVec( const std::string & geom_id, const int & pcurveid )
{
    m_ProxyDoubleArray = vsp::PCurveGetValVec( geom_id, pcurveid );
    return GetProxyDoubleArray();
}

//==== Parasite Drag Tool Functions ====//
void ScriptMgrSingleton::AddExcrescence( const std::string & excresName, int excresType, double excresVal)
{
    vsp::AddExcrescence(excresName,excresType,excresVal);
}

void ScriptMgrSingleton::DeleteExcrescence(int index)
{
    vsp::DeleteExcrescence(index);
}

CScriptArray* ScriptMgrSingleton::GetFeaStructIDVec()
{
    m_ProxyStringArray = vsp::GetFeaStructIDVec( );
    return GetProxyStringArray();
}

CScriptArray* ScriptMgrSingleton::GetFeaSubSurfIDVec( const string & fea_struct_id )
{
    m_ProxyStringArray = vsp::GetFeaSubSurfIDVec( fea_struct_id );
    return GetProxyStringArray();
}

CScriptArray* ScriptMgrSingleton::GetFeaPartIDVec( const string & fea_struct_id )
{
    m_ProxyStringArray = vsp::GetFeaPartIDVec( fea_struct_id );
    return GetProxyStringArray();
}

CScriptArray* ScriptMgrSingleton::GetFeaBCIDVec( const string & fea_struct_id )
{
    m_ProxyStringArray = vsp::GetFeaBCIDVec( fea_struct_id );
    return GetProxyStringArray();
}


void ScriptMgrSingleton::SetIntAnalysisInput( const string& analysis, const string & name, CScriptArray* indata, int index )
{
    vector < int > indata_vec;
    FillSTLVector( indata, indata_vec );

    vsp::SetIntAnalysisInput( analysis, name, indata_vec, index );
}

void ScriptMgrSingleton::SetDoubleAnalysisInput( const string& analysis, const string & name, CScriptArray* indata, int index )
{
    vector < double > indata_vec;
    FillSTLVector( indata, indata_vec );

    vsp::SetDoubleAnalysisInput( analysis, name, indata_vec, index );
}

void ScriptMgrSingleton::SetStringAnalysisInput( const string& analysis, const string & name, CScriptArray* indata, int index )
{
    vector < string > indata_vec;
    FillSTLVector( indata, indata_vec );

    vsp::SetStringAnalysisInput( analysis, name, indata_vec, index );
}

void ScriptMgrSingleton::SetVec3dAnalysisInput( const string& analysis, const string & name, CScriptArray* indata, int index )
{
    vector < vec3d > indata_vec;
    FillSTLVector( indata, indata_vec );

    vsp::SetVec3dAnalysisInput( analysis, name, indata_vec, index );
}

CScriptArray* ScriptMgrSingleton::CompVecPnt01(const string &geom_id, const int &surf_indx, CScriptArray* us, CScriptArray* ws)
{
    vector < double > in_us;
    FillSTLVector( us, in_us );

    vector < double > in_ws;
    FillSTLVector( ws, in_ws );

    m_ProxyVec3dArray = vsp::CompVecPnt01( geom_id, surf_indx, in_us, in_ws );
    return GetProxyVec3dArray();
}

CScriptArray* ScriptMgrSingleton::CompVecDegenPnt01(const string &geom_id, const int &surf_indx, const int &degen_type, CScriptArray* us, CScriptArray* ws)
{
    vector < double > in_us;
    FillSTLVector( us, in_us );

    vector < double > in_ws;
    FillSTLVector( ws, in_ws );

    m_ProxyVec3dArray = vsp::CompVecDegenPnt01( geom_id, surf_indx, degen_type, in_us, in_ws );
    return GetProxyVec3dArray();
}

CScriptArray* ScriptMgrSingleton::CompVecPntRST(const string &geom_id, const int &surf_indx, CScriptArray* rs, CScriptArray* ss, CScriptArray* ts)
{
    vector < double > in_rs;
    FillSTLVector( rs, in_rs );

    vector < double > in_ss;
    FillSTLVector( ss, in_ss );

    vector < double > in_ts;
    FillSTLVector( ts, in_ts );

    m_ProxyVec3dArray = vsp::CompVecPntRST( geom_id, surf_indx, in_rs, in_ss, in_ts );
    return GetProxyVec3dArray();
}

CScriptArray* ScriptMgrSingleton::CompVecNorm01(const string &geom_id, const int &surf_indx, CScriptArray* us, CScriptArray* ws)
{
    vector < double > in_us;
    FillSTLVector( us, in_us );

    vector < double > in_ws;
    FillSTLVector( ws, in_ws );

    m_ProxyVec3dArray = vsp::CompVecNorm01( geom_id, surf_indx, in_us, in_ws );
    return GetProxyVec3dArray();
}

void ScriptMgrSingleton::CompVecCurvature01(const string &geom_id, const int &surf_indx, CScriptArray* us, CScriptArray* ws, CScriptArray* k1s, CScriptArray* k2s, CScriptArray* kas, CScriptArray* kgs)
{
    vector < double > in_us;
    FillSTLVector( us, in_us );

    vector < double > in_ws;
    FillSTLVector( ws, in_ws );

    vector < double > out_k1s;
    vector < double > out_k2s;
    vector < double > out_kas;
    vector < double > out_kgs;

    vsp::CompVecCurvature01( geom_id, surf_indx, in_us, in_ws, out_k1s, out_k2s, out_kas, out_kgs );

    FillASArray( out_k1s, k1s );
    FillASArray( out_k2s, k2s );
    FillASArray( out_kas, kas );
    FillASArray( out_kgs, kgs );
}

void ScriptMgrSingleton::ProjVecPnt01(const string &geom_id, const int &surf_indx, CScriptArray* pts, CScriptArray* us, CScriptArray* ws, CScriptArray* ds )
{
    vector < vec3d > in_pts;
    FillSTLVector( pts, in_pts );

    vector < double > out_us;
    vector < double > out_ws;
    vector < double > out_ds;

    vsp::ProjVecPnt01( geom_id, surf_indx, in_pts, out_us, out_ws, out_ds );

    FillASArray( out_us, us );
    FillASArray( out_ws, ws );
    FillASArray( out_ds, ds );
}

void ScriptMgrSingleton::ProjVecPnt01Guess(const string &geom_id, const int &surf_indx, CScriptArray* pts, CScriptArray* u0s, CScriptArray* w0s, CScriptArray* us, CScriptArray* ws, CScriptArray* ds )
{
    vector < vec3d > in_pts;
    FillSTLVector( pts, in_pts );

    vector < double > in_u0s;
    FillSTLVector( u0s, in_u0s );

    vector < double > in_w0s;
    FillSTLVector( w0s, in_w0s );

    vector < double > out_us;
    vector < double > out_ws;
    vector < double > out_ds;

    vsp::ProjVecPnt01Guess( geom_id, surf_indx, in_pts, in_u0s, in_w0s, out_us, out_ws, out_ds );

    FillASArray( out_us, us );
    FillASArray( out_ws, ws );
    FillASArray( out_ds, ds );
}

void ScriptMgrSingleton::AxisProjVecPnt01(const string &geom_id, const int &surf_indx, const int &iaxis, CScriptArray* pts, CScriptArray* us, CScriptArray* ws, CScriptArray* ds )
{
    vector < vec3d > in_pts;
    FillSTLVector( pts, in_pts );

    vector < double > out_us;
    vector < double > out_ws;
    vector < double > out_ds;

    vsp::AxisProjVecPnt01( geom_id, surf_indx, iaxis, in_pts, out_us, out_ws, out_ds );

    FillASArray( out_us, us );
    FillASArray( out_ws, ws );
    FillASArray( out_ds, ds );
}

void ScriptMgrSingleton::AxisProjVecPnt01Guess(const string &geom_id, const int &surf_indx, const int &iaxis, CScriptArray* pts, CScriptArray* u0s, CScriptArray* w0s, CScriptArray* us, CScriptArray* ws, CScriptArray* ds )
{
    vector < vec3d > in_pts;
    FillSTLVector( pts, in_pts );

    vector < double > in_u0s;
    FillSTLVector( u0s, in_u0s );

    vector < double > in_w0s;
    FillSTLVector( w0s, in_w0s );

    vector < double > out_us;
    vector < double > out_ws;
    vector < double > out_ds;

    vsp::AxisProjVecPnt01Guess( geom_id, surf_indx, iaxis, in_pts, in_u0s, in_w0s, out_us, out_ws, out_ds );

    FillASArray( out_us, us );
    FillASArray( out_ws, ws );
    FillASArray( out_ds, ds );
}

CScriptArray* ScriptMgrSingleton::VecInsideSurf( const string &geom_id, const int &surf_indx, CScriptArray* pts )
{
    vector < vec3d > in_pts;
    FillSTLVector( pts, in_pts );

    vector < bool > temp_vec = vsp::VecInsideSurf( geom_id, surf_indx, in_pts );

    m_ProxyIntArray.clear();
    m_ProxyIntArray.resize( temp_vec.size() );
    for ( size_t i = 0; i < temp_vec.size(); i++ )
    {
        // Cast bool to int
        m_ProxyIntArray[i] = (int)temp_vec[i];
    }

    return GetProxyIntArray();
}

void ScriptMgrSingleton::FindRSTVec(const string &geom_id, const int &surf_indx, CScriptArray* pts, CScriptArray* rs, CScriptArray* ss, CScriptArray* ts, CScriptArray* ds )
{
    vector < vec3d > in_pts;
    FillSTLVector( pts, in_pts );

    vector < double > out_rs;
    vector < double > out_ss;
    vector < double > out_ts;
    vector < double > out_ds;

    vsp::FindRSTVec( geom_id, surf_indx, in_pts, out_rs, out_ss, out_ts, out_ds );

    FillASArray( out_rs, rs );
    FillASArray( out_ss, ss );
    FillASArray( out_ts, ts );
    FillASArray( out_ds, ds );
}

void ScriptMgrSingleton::FindRSTVecGuess(const string &geom_id, const int &surf_indx, CScriptArray* pts, CScriptArray* r0s, CScriptArray* s0s, CScriptArray* t0s, CScriptArray* rs, CScriptArray* ss, CScriptArray* ts, CScriptArray* ds )
{
    vector < vec3d > in_pts;
    FillSTLVector( pts, in_pts );

    vector < double > in_r0s;
    vector < double > in_s0s;
    vector < double > in_t0s;

    FillSTLVector( r0s, in_r0s );
    FillSTLVector( s0s, in_s0s );
    FillSTLVector( t0s, in_t0s );

    vector < double > out_rs;
    vector < double > out_ss;
    vector < double > out_ts;
    vector < double > out_ds;

    vsp::FindRSTVecGuess(geom_id, surf_indx, in_pts, in_r0s, in_s0s, in_t0s, out_rs, out_ss, out_ts, out_ds );

    FillASArray( out_rs, rs );
    FillASArray( out_ss, ss );
    FillASArray( out_ts, ts );
    FillASArray( out_ds, ds );
}

void ScriptMgrSingleton::ConvertRSTtoLMNVec(const string &geom_id, const int &surf_indx, CScriptArray* rs, CScriptArray* ss, CScriptArray* ts, CScriptArray* ls, CScriptArray* ms, CScriptArray* ns )
{
    vector < double > in_rs;
    vector < double > in_ss;
    vector < double > in_ts;

    FillSTLVector( rs, in_rs );
    FillSTLVector( ss, in_ss );
    FillSTLVector( ts, in_ts );

    vector < double > out_ls;
    vector < double > out_ms;
    vector < double > out_ns;

    vsp::ConvertRSTtoLMNVec( geom_id, surf_indx, in_rs, in_ss, in_ts, out_ls, out_ms, out_ns );

    FillASArray( out_ls, ls );
    FillASArray( out_ms, ms );
    FillASArray( out_ns, ns );
}

void ScriptMgrSingleton::ConvertLMNtoRSTVec(const string &geom_id, const int &surf_indx, CScriptArray* ls, CScriptArray* ms, CScriptArray* ns, CScriptArray* rs, CScriptArray* ss, CScriptArray* ts )
{
    vector < double > in_ls;
    vector < double > in_ms;
    vector < double > in_ns;

    FillSTLVector( ls, in_ls );
    FillSTLVector( ms, in_ms );
    FillSTLVector( ns, in_ns );

    vector < double > out_rs;
    vector < double > out_ss;
    vector < double > out_ts;

    vsp::ConvertLMNtoRSTVec( geom_id, surf_indx, in_ls, in_ms, in_ns, out_rs, out_ss, out_ts );

    FillASArray( out_rs, rs );
    FillASArray( out_ss, ss );
    FillASArray( out_ts, ts );
}

void ScriptMgrSingleton::GetUWTess01(const string &geom_id, const int &surf_indx, CScriptArray* us, CScriptArray* ws )
{
    vector < double > out_us;
    vector < double > out_ws;

    vsp::GetUWTess01( geom_id, surf_indx, out_us, out_ws );

    FillASArray( out_us, us );
    FillASArray( out_ws, ws );
}

//=== Register Measure Functions ===//
CScriptArray* ScriptMgrSingleton::GetAllRulers()
{
    m_ProxyStringArray = vsp::GetAllRulers();
    return GetProxyStringArray();}

CScriptArray* ScriptMgrSingleton::GetAllProbes()
{
    m_ProxyStringArray = vsp::GetAllProbes();
    return GetProxyStringArray();
}

//==== Console Print String Data ====//
void ScriptMgrSingleton::Print( const string & data, bool new_line )
{
    printf( " %s ", data.c_str() );
    if ( new_line ) printf( "\n" );
}

//==== Console Print Vec3d Data ====//
void ScriptMgrSingleton::Print( const vec3d & data, bool new_line )
{
    printf( " %f, %f, %f ", data.x(), data.y(), data.z() );
    if ( new_line ) printf( "\n" );
}

//==== Console Print Double Data ====//
void ScriptMgrSingleton::Print( double data, bool new_line )
{
    printf( " %f ", data );
    if ( new_line ) printf( "\n" );
}

//==== Console Print Int Data ====//
void ScriptMgrSingleton::Print( int data, bool new_line )
{
    printf( " %d ", data );
    if ( new_line ) printf( "\n" );
}

//=== Register Advanced Link Functions ===//
CScriptArray* ScriptMgrSingleton::GetAdvLinkNames()
{
    m_ProxyStringArray = vsp::GetAdvLinkNames();
    return GetProxyStringArray();
}

CScriptArray* ScriptMgrSingleton::GetAdvLinkInputNames( int index )
{
    m_ProxyStringArray = vsp::GetAdvLinkInputNames( index );
    return GetProxyStringArray();
}

CScriptArray* ScriptMgrSingleton::GetAdvLinkInputParms( int index )
{
    m_ProxyStringArray = vsp::GetAdvLinkInputParms( index );
    return GetProxyStringArray();
}

CScriptArray* ScriptMgrSingleton::GetAdvLinkOutputNames( int index )
{
    m_ProxyStringArray = vsp::GetAdvLinkOutputNames( index );
    return GetProxyStringArray();
}

CScriptArray* ScriptMgrSingleton::GetAdvLinkOutputParms( int index )
{
    m_ProxyStringArray = vsp::GetAdvLinkOutputParms( index );
    return GetProxyStringArray();
}
