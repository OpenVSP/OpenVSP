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
    m_ScriptEngine = NULL;
    m_ScriptMessages = "";

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
    int r = se->SetMessageCallback( vspFUNCTION( MessageCallback ), 0, vspCALL_CDECL );
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
    m_DoubleArrayType = se->GetTypeInfoById( se->GetTypeIdByDecl( "array<double>" ) );
    assert( m_DoubleArrayType );
    m_DoubleMatArrayType = se->GetTypeInfoById( se->GetTypeIdByDecl( "array<array<double>@>" ) );
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
    map< string, string >::iterator iter;

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
    map< string, string >::iterator iter;
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

    asIScriptFunction *func = NULL;
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
    map< string, string >::iterator iter;
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
    map< string, string >::iterator iter;
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


    r = se->RegisterEnum( "FEA_SHELL_TREATMENT_TYPE" );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "FEA_SHELL_TREATMENT_TYPE", "FEA_KEEP", FEA_KEEP );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "FEA_SHELL_TREATMENT_TYPE", "FEA_DELETE", FEA_DELETE );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "FEA_SHELL_TREATMENT_TYPE", "FEA_NUM_SHELL_TREATMENT_TYPES", FEA_NUM_SHELL_TREATMENT_TYPES );
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
    r = se->RegisterEnumValue( "FEA_PART_TYPE", "FEA_NUM_TYPES", FEA_NUM_TYPES );
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
    r = se->RegisterEnumValue( "GUI_VSP_SCREEN", "VSP_VAR_PRESET_SCREEN", VSP_VAR_PRESET_SCREEN );
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
    r = se->RegisterEnumValue( "PROJ_TGT_TYPE", "NUM_PROJ_TGT_OPTIONS", NUM_PROJ_TGT_OPTIONS );
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
    r = se->RegisterEnumValue( "RES_DATA_TYPE", "INT_DATA", INT_DATA );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "RES_DATA_TYPE", "DOUBLE_DATA", DOUBLE_DATA );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "RES_DATA_TYPE", "STRING_DATA", STRING_DATA );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "RES_DATA_TYPE", "VEC3D_DATA", VEC3D_DATA );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "RES_DATA_TYPE", "DOUBLE_MATRIX_DATA", DOUBLE_MATRIX_DATA );
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


    r = se->RegisterEnum( "VSPAERO_ANALYSIS_METHOD" );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "VSPAERO_ANALYSIS_METHOD", "VORTEX_LATTICE", VORTEX_LATTICE );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "VSPAERO_ANALYSIS_METHOD", "PANEL", PANEL );
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
    r = se->RegisterObjectBehaviour( "vec3d", asBEHAVE_CONSTRUCT, "void f()", vspFUNCTION( Vec3dDefaultConstructor ), vspCALL_CDECL_OBJLAST );
    assert( r >= 0 );

    r = se->RegisterObjectBehaviour( "vec3d", asBEHAVE_CONSTRUCT, "void f(double, double, double)", vspFUNCTION( Vec3dInitConstructor ), vspCALL_CDECL_OBJLAST );
    assert( r >= 0 );

    r = se->RegisterObjectBehaviour( "vec3d", asBEHAVE_CONSTRUCT, "void f(const vec3d &in)", vspFUNCTION( Vec3dCopyConstructor ), vspCALL_CDECL_OBJLAST );
    assert( r >= 0 );

    //==== Register the vec3d Methods  ====//
    r = se->RegisterObjectMethod( "vec3d", "double& opIndex(int) const", vspMETHODPR( vec3d, operator[], ( int ), double& ), vspCALL_THISCALL );
    assert( r >= 0 );

    r = se->RegisterObjectMethod( "vec3d", "double x() const", vspMETHOD( vec3d, x ), vspCALL_THISCALL );
    assert( r >= 0 );

    r = se->RegisterObjectMethod( "vec3d", "double y() const", vspMETHOD( vec3d, y ), vspCALL_THISCALL );
    assert( r >= 0 );

    r = se->RegisterObjectMethod( "vec3d", "double z() const", vspMETHOD( vec3d, z ), vspCALL_THISCALL );
    assert( r >= 0 );

    r = se->RegisterObjectMethod( "vec3d", "vec3d& set_xyz(double x, double y, double z)", vspMETHOD( vec3d, set_xyz ), vspCALL_THISCALL );
    assert( r >= 0 );

    r = se->RegisterObjectMethod( "vec3d", "vec3d& set_x(double x)", vspMETHOD( vec3d, set_x ), vspCALL_THISCALL );
    assert( r >= 0 );

    r = se->RegisterObjectMethod( "vec3d", "vec3d& set_y(double y)", vspMETHOD( vec3d, set_y ), vspCALL_THISCALL );
    assert( r >= 0 );

    r = se->RegisterObjectMethod( "vec3d", "vec3d& set_z(double z)", vspMETHOD( vec3d, set_z ), vspCALL_THISCALL );
    assert( r >= 0 );

    r = se->RegisterObjectMethod( "vec3d", "void rotate_x(double cos_alpha, double sin_alpha)", vspMETHOD( vec3d, rotate_x ), vspCALL_THISCALL );
    assert( r >= 0 );

    r = se->RegisterObjectMethod( "vec3d", "void rotate_y(double cos_alpha, double sin_alpha)", vspMETHOD( vec3d, rotate_y ), vspCALL_THISCALL );
    assert( r >= 0 );

    r = se->RegisterObjectMethod( "vec3d", "void rotate_z(double cos_alpha, double sin_alpha)", vspMETHOD( vec3d, rotate_z ), vspCALL_THISCALL );
    assert( r >= 0 );

    r = se->RegisterObjectMethod( "vec3d", "void scale_x(double scale)", vspMETHOD( vec3d, scale_x ), vspCALL_THISCALL );
    assert( r >= 0 );

    r = se->RegisterObjectMethod( "vec3d", "void scale_y(double scale)", vspMETHOD( vec3d, scale_y ), vspCALL_THISCALL );
    assert( r >= 0 );

    r = se->RegisterObjectMethod( "vec3d", "void scale_z(double scale)", vspMETHOD( vec3d, scale_z ), vspCALL_THISCALL );
    assert( r >= 0 );

    r = se->RegisterObjectMethod( "vec3d", "void offset_x(double offset)", vspMETHOD( vec3d, offset_x ), vspCALL_THISCALL );
    assert( r >= 0 );

    r = se->RegisterObjectMethod( "vec3d", "void offset_y(double offset)", vspMETHOD( vec3d, offset_y ), vspCALL_THISCALL );
    assert( r >= 0 );

    r = se->RegisterObjectMethod( "vec3d", "void offset_z(double offset)", vspMETHOD( vec3d, offset_z ), vspCALL_THISCALL );
    assert( r >= 0 );

    r = se->RegisterObjectMethod( "vec3d", "void rotate_z_zero_x(double cos_alpha, double sin_alpha)", vspMETHOD( vec3d, rotate_z_zero_x ), vspCALL_THISCALL );
    assert( r >= 0 );

    r = se->RegisterObjectMethod( "vec3d", "void rotate_z_zero_y(double cos_alpha, double sin_alpha)", vspMETHOD( vec3d, rotate_z_zero_y ), vspCALL_THISCALL );
    assert( r >= 0 );

    r = se->RegisterObjectMethod( "vec3d", "vec3d reflect_xy()", vspMETHOD( vec3d, reflect_xy ), vspCALL_THISCALL );
    assert( r >= 0 );

    r = se->RegisterObjectMethod( "vec3d", "vec3d reflect_xz()", vspMETHOD( vec3d, reflect_xz ), vspCALL_THISCALL );
    assert( r >= 0 );

    r = se->RegisterObjectMethod( "vec3d", "vec3d reflect_yz()", vspMETHOD( vec3d, reflect_yz ), vspCALL_THISCALL );
    assert( r >= 0 );

    r = se->RegisterObjectMethod( "vec3d", "vec3d opAdd(const vec3d &in) const", vspFUNCTIONPR( operator+, ( const vec3d&, const vec3d& ), vec3d ), vspCALL_CDECL_OBJFIRST );
    assert( r >= 0 );

    r = se->RegisterObjectMethod( "vec3d", "vec3d opSub(const vec3d &in) const", vspFUNCTIONPR( operator-, ( const vec3d&, const vec3d& ), vec3d ), vspCALL_CDECL_OBJFIRST );
    assert( r >= 0 );

    r = se->RegisterObjectMethod( "vec3d", "vec3d opMul(double b) const", vspFUNCTIONPR( operator*, ( const vec3d & a, double b ), vec3d ), vspCALL_CDECL_OBJFIRST );
    assert( r >= 0 );


    r = se->RegisterObjectMethod( "vec3d", "vec3d opMul_r(const vec3d &in) const", vspFUNCTIONPR( operator*, ( const vec3d&, const vec3d& ), vec3d ), vspCALL_CDECL_OBJFIRST );
    assert( r >= 0 );


    r = se->RegisterObjectMethod( "vec3d", "vec3d opDiv(double b) const", vspFUNCTIONPR( operator/, ( const vec3d&, double b ), vec3d ), vspCALL_CDECL_OBJFIRST );
    assert( r >= 0 );


    r = se->RegisterObjectMethod( "vec3d", "double mag() const", vspMETHOD( vec3d, mag ), vspCALL_THISCALL );
    assert( r >= 0 );


    r = se->RegisterObjectMethod( "vec3d", "void normalize()", vspMETHOD( vec3d, normalize ), vspCALL_THISCALL );
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "double dist(const vec3d& in a, const vec3d& in b)", vspFUNCTIONPR( dist, ( const vec3d&, const vec3d& ), double  ), vspCALL_CDECL );
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "double dist_squared(const vec3d& in a, const vec3d& in b)", vspFUNCTIONPR( dist_squared, ( const vec3d&, const vec3d& ), double  ), vspCALL_CDECL );
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "double dot(const vec3d& in a, const vec3d& in b)", vspFUNCTIONPR( dot, ( const vec3d&, const vec3d& ), double  ), vspCALL_CDECL );
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "vec3d cross(const vec3d& in a, const vec3d& in b)", vspFUNCTIONPR( cross, ( const vec3d&, const vec3d& ), vec3d  ), vspCALL_CDECL );
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "double angle(const vec3d& in a, const vec3d& in b)", vspFUNCTIONPR( angle, ( const vec3d&, const vec3d& ), double  ), vspCALL_CDECL );
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "double signed_angle(const vec3d& in a, const vec3d& in b, const vec3d& in ref )", vspFUNCTIONPR( signed_angle, ( const vec3d & a, const vec3d & b, const vec3d & ref ), double  ), vspCALL_CDECL );
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "double cos_angle(const vec3d& in a, const vec3d& in b )", vspFUNCTIONPR( cos_angle, ( const vec3d & a, const vec3d & b ), double  ), vspCALL_CDECL );
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "vec3d RotateArbAxis(const vec3d& in p, double theta, const vec3d& in axis )", vspFUNCTIONPR( RotateArbAxis, ( const vec3d & p, double theta, const vec3d & axis ), vec3d ), vspCALL_CDECL );
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
    r = se->RegisterObjectBehaviour( "Matrix4d", asBEHAVE_CONSTRUCT, "void f()", vspFUNCTION( Matrix4dDefaultConstructor ), vspCALL_CDECL_OBJLAST );
    assert( r >= 0 ); // TODO?

    //===== Register the Matrix4d methods =====//


    r = se->RegisterObjectMethod( "Matrix4d", "void loadIdentity()", vspMETHOD( Matrix4d, loadIdentity ), vspCALL_THISCALL );
    assert( r >= 0 );


    r = se->RegisterObjectMethod( "Matrix4d", "void translatef( const double & in x, const double & in y, const double & in z)", vspMETHOD( Matrix4d, translatef ), vspCALL_THISCALL );
    assert( r >= 0 );


    r = se->RegisterObjectMethod( "Matrix4d", "void rotateX( const double & in ang )", vspMETHOD( Matrix4d, rotateX ), vspCALL_THISCALL );
    assert( r >= 0 );


    r = se->RegisterObjectMethod( "Matrix4d", "void rotateY( const double & in ang )", vspMETHOD( Matrix4d, rotateY ), vspCALL_THISCALL );
    assert( r >= 0 );


    r = se->RegisterObjectMethod( "Matrix4d", "void rotateZ( const double & in ang )", vspMETHOD( Matrix4d, rotateZ ), vspCALL_THISCALL );
    assert( r >= 0 );


    r = se->RegisterObjectMethod( "Matrix4d", "void rotate( const double & in ang, const vec3d & in axis )", vspMETHOD( Matrix4d, rotate ), vspCALL_THISCALL );
    assert( r >= 0 );


    r = se->RegisterObjectMethod( "Matrix4d", "void scale( const double & in scale )", vspMETHOD( Matrix4d, scale ), vspCALL_THISCALL );
    assert( r >= 0 );


    r = se->RegisterObjectMethod( "Matrix4d", "vec3d xform( const vec3d & in v )", vspMETHOD( Matrix4d, xform ), vspCALL_THISCALL );
    assert( r >= 0 );


    r = se->RegisterObjectMethod( "Matrix4d", "vec3d getAngles()", vspMETHOD( Matrix4d, getAngles ), vspCALL_THISCALL );
    assert( r >= 0 );


    r = se->RegisterObjectMethod( "Matrix4d", "void loadXZRef()", vspMETHOD( Matrix4d, loadXZRef ), vspCALL_THISCALL );
    assert( r >= 0 );


    r = se->RegisterObjectMethod( "Matrix4d", "void loadXYRef()", vspMETHOD( Matrix4d, loadXYRef ), vspCALL_THISCALL );
    assert( r >= 0 );


    r = se->RegisterObjectMethod( "Matrix4d", "void loadYZRef()", vspMETHOD( Matrix4d, loadYZRef ), vspCALL_THISCALL );
    assert( r >= 0 );


    r = se->RegisterObjectMethod( "Matrix4d", "void affineInverse()", vspMETHOD( Matrix4d, affineInverse ), vspCALL_THISCALL );
    assert( r >= 0 );


    r = se->RegisterObjectMethod("Matrix4d", "void buildXForm( const vec3d & in pos, const vec3d & in rot, const vec3d & in cent_rot )", vspMETHOD(Matrix4d, buildXForm), vspCALL_THISCALL );
    assert( r >= 0 ); // TODO: Example

    //TODO: Expose additional functions to the API (i.e. matMult)

}

//==== Register Custom Geom Mgr Object ====//
void ScriptMgrSingleton::RegisterCustomGeomMgr( asIScriptEngine* se )
{

    int r;
    r = se->RegisterGlobalFunction( "string AddParm( int type, const string & in name, const string & in group )",
                                    vspMETHOD( CustomGeomMgrSingleton, AddParm ), vspCALL_THISCALL_ASGLOBAL, &CustomGeomMgr );
    assert( r );


    r = se->RegisterGlobalFunction( "string GetCurrCustomGeom()",
                                    vspMETHOD( CustomGeomMgrSingleton, GetCurrCustomGeom ), vspCALL_THISCALL_ASGLOBAL, &CustomGeomMgr );
    assert( r );


    r = se->RegisterGlobalFunction( "string GetCustomParm( int index )",
                                    vspMETHOD( CustomGeomMgrSingleton, GetCustomParm ), vspCALL_THISCALL_ASGLOBAL, &CustomGeomMgr );
    assert( r ); // TODO: Example


    r = se->RegisterGlobalFunction( "int AddGui( int type, const string & in label = string(), const string & in parm_name = string(), const string & in group_name = string(), double range = 10.0 )",
                                    vspMETHOD( CustomGeomMgrSingleton, AddGui ), vspCALL_THISCALL_ASGLOBAL, &CustomGeomMgr );
    assert( r );


    r = se->RegisterGlobalFunction( "void UpdateGui( int gui_id, const string & in parm_id )",
                                    vspMETHOD( CustomGeomMgrSingleton, AddUpdateGui ), vspCALL_THISCALL_ASGLOBAL, &CustomGeomMgr );
    assert( r );


    r = se->RegisterGlobalFunction( "string AddXSecSurf()",
                                    vspMETHOD( CustomGeomMgrSingleton, AddXSecSurf ), vspCALL_THISCALL_ASGLOBAL, &CustomGeomMgr );
    assert( r );


    r = se->RegisterGlobalFunction( "void RemoveXSecSurf(const string & in xsec_id)",
                                    vspMETHOD( CustomGeomMgrSingleton, RemoveXSecSurf ), vspCALL_THISCALL_ASGLOBAL, &CustomGeomMgr );
    assert( r );


    r = se->RegisterGlobalFunction( "void ClearXSecSurfs()",
                                    vspMETHOD( CustomGeomMgrSingleton, ClearXSecSurfs ), vspCALL_THISCALL_ASGLOBAL, &CustomGeomMgr );
    assert( r );


    r = se->RegisterGlobalFunction( "void SkinXSecSurf( bool closed_flag = false )",
                                    vspMETHOD( CustomGeomMgrSingleton, SkinXSecSurf ), vspCALL_THISCALL_ASGLOBAL, &CustomGeomMgr );
    assert( r );


    r = se->RegisterGlobalFunction( "void CloneSurf(int index, Matrix4d & in mat)",
                                    vspMETHOD( CustomGeomMgrSingleton, CloneSurf ), vspCALL_THISCALL_ASGLOBAL, &CustomGeomMgr );
    assert( r );


    r = se->RegisterGlobalFunction( "void TransformSurf(int index, Matrix4d & in mat)",
                                    vspMETHOD( CustomGeomMgrSingleton, TransformSurf ), vspCALL_THISCALL_ASGLOBAL, &CustomGeomMgr );
    assert( r ); // TODO: Example


    r = se->RegisterGlobalFunction( "void SetVspSurfType( int type, int surf_index = -1 )",
                                    vspMETHOD( CustomGeomMgrSingleton, SetVspSurfType ), vspCALL_THISCALL_ASGLOBAL, &CustomGeomMgr );
    assert( r );


    r = se->RegisterGlobalFunction( "void SetVspSurfCfdType( int type, int surf_index = -1 )",
                                    vspMETHOD( CustomGeomMgrSingleton, SetVspSurfCfdType ), vspCALL_THISCALL_ASGLOBAL, &CustomGeomMgr );
    assert( r );


    r = se->RegisterGlobalFunction( "void SetCustomXSecLoc( const string & in xsec_id, const vec3d & in loc )",
                                    vspMETHOD( CustomGeomMgrSingleton, SetCustomXSecLoc ), vspCALL_THISCALL_ASGLOBAL, &CustomGeomMgr );
    assert( r );


    r = se->RegisterGlobalFunction( "vec3d GetCustomXSecLoc( const string & in xsec_id )",
                                    vspMETHOD( CustomGeomMgrSingleton, GetCustomXSecLoc ), vspCALL_THISCALL_ASGLOBAL, &CustomGeomMgr );
    assert( r );


    r = se->RegisterGlobalFunction( "void SetCustomXSecRot( const string & in xsec_id, const vec3d & in rot )",
                                    vspMETHOD( CustomGeomMgrSingleton, SetCustomXSecRot ), vspCALL_THISCALL_ASGLOBAL, &CustomGeomMgr );
    assert( r );


    r = se->RegisterGlobalFunction( "vec3d GetCustomXSecRot( const string & in xsec_id )",
                                    vspMETHOD( CustomGeomMgrSingleton, GetCustomXSecRot ), vspCALL_THISCALL_ASGLOBAL, &CustomGeomMgr );
    assert( r );


    r = se->RegisterGlobalFunction( "bool CheckClearTriggerEvent( int gui_id )",
                                    vspMETHOD( CustomGeomMgrSingleton, CheckClearTriggerEvent ), vspCALL_THISCALL_ASGLOBAL, &CustomGeomMgr );
    assert( r ); // TODO: Example


    r = se->RegisterGlobalFunction(
        "void SetupCustomDefaultSource( int type, int surf_index, double l1, double r1, double u1, double w1, double l2 = 0, double r2 = 0, double u2 = 0, double w2 = 0 )",
        vspMETHOD( CustomGeomMgrSingleton, SetupCustomDefaultSource ), vspCALL_THISCALL_ASGLOBAL, &CustomGeomMgr );
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "void ClearAllCustomDefaultSources()",
                                    vspMETHOD( CustomGeomMgrSingleton, ClearAllCustomDefaultSources ), vspCALL_THISCALL_ASGLOBAL, &CustomGeomMgr );
    assert( r );


    r = se->RegisterGlobalFunction( "void SetCustomCenter( double x, double y, double z )",
                                    vspMETHOD( CustomGeomMgrSingleton, SetCustomCenter ), vspCALL_THISCALL_ASGLOBAL, &CustomGeomMgr );
    assert( r );


    r = se->RegisterGlobalFunction( "string AppendXSec( const string & in xsec_surf_id, int type )",
                                    vspMETHOD( CustomGeomMgrSingleton, AppendCustomXSec ), vspCALL_THISCALL_ASGLOBAL, &CustomGeomMgr );
    assert( r );

    // WARNING: Both versions of the AppendCustomXSec must be available to avoid breaking existing CustomGeom scripts


    r = se->RegisterGlobalFunction( "string AppendCustomXSec( const string & in xsec_surf_id, int type )",
                                    vspMETHOD( CustomGeomMgrSingleton, AppendCustomXSec ), vspCALL_THISCALL_ASGLOBAL, &CustomGeomMgr );
    assert( r );


    r = se->RegisterGlobalFunction( "void CutCustomXSec( const string & in xsec_surf_id, int index )",
                                    vspMETHOD( CustomGeomMgrSingleton, CutCustomXSec ), vspCALL_THISCALL_ASGLOBAL, &CustomGeomMgr );
    assert( r ); // TODO: Example


    r = se->RegisterGlobalFunction( "void CopyCustomXSec( const string & in xsec_surf_id, int index )",
                                    vspMETHOD( CustomGeomMgrSingleton, CopyCustomXSec ), vspCALL_THISCALL_ASGLOBAL, &CustomGeomMgr );
    assert( r ); // TODO: Example


    r = se->RegisterGlobalFunction( "void PasteCustomXSec( const string & in xsec_surf_id, int index )",
                                    vspMETHOD( CustomGeomMgrSingleton, PasteCustomXSec ), vspCALL_THISCALL_ASGLOBAL, &CustomGeomMgr );
    assert( r ); // TODO: Example


    r = se->RegisterGlobalFunction( "string InsertCustomXSec( const string & in xsec_surf_id, int type, int index )",
                                    vspMETHOD( CustomGeomMgrSingleton, InsertCustomXSec ), vspCALL_THISCALL_ASGLOBAL, &CustomGeomMgr );
    assert( r ); // TODO: Example


}



//==== Register Adv Link Mgr Object ====//
void ScriptMgrSingleton::RegisterAdvLinkMgr( asIScriptEngine* se )
{
    int r;

    r = se->RegisterGlobalFunction( "array<string>@ GetAdvLinkNames()", vspMETHOD( ScriptMgrSingleton, GetAdvLinkNames ), vspCALL_THISCALL_ASGLOBAL, &ScriptMgr);
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "int GetLinkIndex( const string & in name )", vspFUNCTION( vsp::GetLinkIndex ), vspCALL_CDECL );
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "void DelAdvLink( int index )", vspFUNCTION( vsp::DelAdvLink ), vspCALL_CDECL );
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "void DelAllAdvLinks()", vspFUNCTION( vsp::DelAllAdvLinks ), vspCALL_CDECL );
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "void AddAdvLink( const string & in name )", vspFUNCTION( vsp::AddAdvLink ), vspCALL_CDECL );
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "void AddAdvLinkInput( int index, const string & in parm_id, const string & in var_name )", vspFUNCTION( vsp::AddAdvLinkInput ), vspCALL_CDECL );
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "void AddAdvLinkOutput( int index, const string & in parm_id, const string & in var_name )", vspFUNCTION( vsp::AddAdvLinkOutput ), vspCALL_CDECL );
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "void DelAdvLinkInput( int index, const string & in var_name )", vspFUNCTION( vsp::DelAdvLinkInput ), vspCALL_CDECL );
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "void DelAdvLinkOutput( int index, const string & in var_name )", vspFUNCTION( vsp::DelAdvLinkOutput ), vspCALL_CDECL );
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "array<string>@ GetAdvLinkInputNames( int index )", vspMETHOD( ScriptMgrSingleton, GetAdvLinkInputNames ), vspCALL_THISCALL_ASGLOBAL, &ScriptMgr);
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "array<string>@ GetAdvLinkInputParms( int index )", vspMETHOD( ScriptMgrSingleton, GetAdvLinkInputParms ), vspCALL_THISCALL_ASGLOBAL, &ScriptMgr);
    assert( r >= 0 );



    r = se->RegisterGlobalFunction( "array<string>@ GetAdvLinkOutputNames( int index )", vspMETHOD( ScriptMgrSingleton, GetAdvLinkOutputNames ), vspCALL_THISCALL_ASGLOBAL, &ScriptMgr);
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "array<string>@ GetAdvLinkOutputParms( int index )", vspMETHOD( ScriptMgrSingleton, GetAdvLinkOutputParms ), vspCALL_THISCALL_ASGLOBAL, &ScriptMgr);
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "bool ValidateAdvLinkParms( int index )", vspFUNCTION( vsp::ValidateAdvLinkParms ), vspCALL_CDECL );
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "void SetAdvLinkCode( int index, const string & in code )", vspFUNCTION( vsp::SetAdvLinkCode ), vspCALL_CDECL );
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "string GetAdvLinkCode( int index )", vspFUNCTION( vsp::GetAdvLinkCode ), vspCALL_CDECL );
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "void SearchReplaceAdvLinkCode( int index, const string & in from, const string & in to )", vspFUNCTION( vsp::SearchReplaceAdvLinkCode ), vspCALL_CDECL );
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "bool BuildAdvLinkScript( int index )", vspFUNCTION( vsp::BuildAdvLinkScript ), vspCALL_CDECL );
    assert( r >= 0 );

//
// These methods appear to have been mistakenly exposed to the API.
//
//
//    r = se->RegisterGlobalFunction( "void AddInput( const string & in parm_id, const string & in var_name )",
//                                    vspMETHOD( AdvLinkMgrSingleton, AddInput ), vspCALL_THISCALL_ASGLOBAL, &AdvLinkMgr );
//    assert( r );
//
//
//    r = se->RegisterGlobalFunction( "void AddOutput( const string & in parm_id, const string & in var_name )",
//                                    vspMETHOD( AdvLinkMgrSingleton, AddOutput ), vspCALL_THISCALL_ASGLOBAL, &AdvLinkMgr );
//    assert( r );


    r = se->RegisterGlobalFunction( "void SetVar( const string & in var_name, double val )", vspMETHOD( AdvLinkMgrSingleton, SetVar ), vspCALL_THISCALL_ASGLOBAL, &AdvLinkMgr );
    assert( r );


    r = se->RegisterGlobalFunction( "double GetVar( const string & in var_name )", vspMETHOD( AdvLinkMgrSingleton, GetVar ), vspCALL_THISCALL_ASGLOBAL, &AdvLinkMgr );
    assert( r );
}

//==== Register API E Functions ====//
void ScriptMgrSingleton::RegisterAPIErrorObj( asIScriptEngine* se )
{

    //==== Register ErrorObj Object =====//
    int r = se->RegisterObjectType( "ErrorObj", sizeof( vsp::ErrorObj ), asOBJ_VALUE | asOBJ_POD | asOBJ_APP_CLASS_CDA );
    assert( r >= 0 );


    r = se->RegisterObjectMethod( "ErrorObj", "ERROR_CODE GetErrorCode()", vspMETHOD( vsp::ErrorObj, GetErrorCode ), vspCALL_THISCALL );
    assert( r >= 0 );


    r = se->RegisterObjectMethod( "ErrorObj", "string GetErrorString()", vspMETHOD( vsp::ErrorObj, GetErrorString ), vspCALL_THISCALL );
    assert( r >= 0 );



}


//==== Register VSP API Functions ====//
void ScriptMgrSingleton::RegisterAPI( asIScriptEngine* se )
{
    int r;

    //==== API Error Functions ====//
    // TODO: Move to ErrorObj group

    r = se->RegisterGlobalFunction( "bool GetErrorLastCallFlag()", vspMETHOD( vsp::ErrorMgrSingleton, GetErrorLastCallFlag ), vspCALL_THISCALL_ASGLOBAL, &vsp::ErrorMgr );
    assert( r );


    r = se->RegisterGlobalFunction( "int GetNumTotalErrors()", vspMETHOD( vsp::ErrorMgrSingleton, GetNumTotalErrors ), vspCALL_THISCALL_ASGLOBAL, &vsp::ErrorMgr );
    assert( r );


    r = se->RegisterGlobalFunction( "ErrorObj PopLastError()", vspMETHOD( vsp::ErrorMgrSingleton, PopLastError ), vspCALL_THISCALL_ASGLOBAL, &vsp::ErrorMgr );
    assert( r );


    r = se->RegisterGlobalFunction( "ErrorObj GetLastError()", vspMETHOD( vsp::ErrorMgrSingleton, GetLastError ), vspCALL_THISCALL_ASGLOBAL, &vsp::ErrorMgr );
    assert( r );


    r = se->RegisterGlobalFunction( "void SilenceErrors()", vspMETHOD( vsp::ErrorMgrSingleton, SilenceErrors ), vspCALL_THISCALL_ASGLOBAL, &vsp::ErrorMgr );
    assert( r );


    r = se->RegisterGlobalFunction( "void PrintOnErrors()", vspMETHOD( vsp::ErrorMgrSingleton, PrintOnErrors ), vspCALL_THISCALL_ASGLOBAL, &vsp::ErrorMgr );
    assert( r );

    //==== Visualization Functions ====//

    r = se->RegisterGlobalFunction( "void InitGUI()", vspFUNCTION( vsp::InitGUI ), vspCALL_CDECL );
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "void StartGUI()", vspFUNCTION( vsp::StartGUI ), vspCALL_CDECL );
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "void EnableReturnToAPI()", vspFUNCTION( vsp::EnableReturnToAPI ), vspCALL_CDECL );
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "void DisableReturnToAPI()", vspFUNCTION( vsp::DisableReturnToAPI ), vspCALL_CDECL );
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "void StopGUI()", vspFUNCTION( vsp::StopGUI ), vspCALL_CDECL );
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "void HideScreens()", vspFUNCTION( vsp::HideScreens ), vspCALL_CDECL );
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "void ShowScreens()", vspFUNCTION( vsp::ShowScreens ), vspCALL_CDECL );
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "void PopupMsg( const string & in msg )", vspFUNCTION( vsp::PopupMsg ), vspCALL_CDECL );
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "void UpdateGUI()", vspFUNCTION( vsp::UpdateGUI ), vspCALL_CDECL );
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "bool IsGUIBuild()", vspFUNCTION( vsp::IsGUIBuild ), vspCALL_CDECL );
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "void Lock()", vspFUNCTION( vsp::Lock ), vspCALL_CDECL );
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "void Unlock()", vspFUNCTION( vsp::Unlock ), vspCALL_CDECL );
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "void ScreenGrab( const string & in file_name, int w, int h, bool transparentBG, bool autocrop = false )", vspFUNCTION( vsp::ScreenGrab ), vspCALL_CDECL );
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "void SetViewAxis( bool vaxis )", vspFUNCTION( vsp::SetViewAxis ), vspCALL_CDECL );
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "void SetShowBorders( bool brdr )", vspFUNCTION( vsp::SetShowBorders ), vspCALL_CDECL );
    assert( r >= 0 );


    r = se->RegisterGlobalFunction("void SetGeomDrawType( const string & in geom_id, int type )", vspFUNCTION(vsp::SetGeomDrawType), vspCALL_CDECL);
    assert( r >= 0 );


    r = se->RegisterGlobalFunction("void SetGeomDisplayType( const string & in geom_id, int type )", vspFUNCTION(vsp::SetGeomDisplayType), vspCALL_CDECL);
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "void SetBackground( double r, double g, double b )", vspFUNCTION( vsp::SetBackground ), vspCALL_CDECL );
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "void SetGUIElementDisable( int e, bool state )", vspFUNCTION( vsp::SetGUIElementDisable ), vspCALL_CDECL );
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "void SetGUIScreenDisable( int s, bool state )", vspFUNCTION( vsp::SetGUIScreenDisable ), vspCALL_CDECL );
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "void SetGeomScreenDisable( int s, bool state )", vspFUNCTION( vsp::SetGeomScreenDisable ), vspCALL_CDECL );
    assert( r >= 0 );

    //==== Vehicle Functions ====//

    r = se->RegisterGlobalFunction( "void Update( bool update_managers = true)", vspFUNCTION( vsp::Update ), vspCALL_CDECL );
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "void VSPExit( int error_code )", vspFUNCTION( vsp::VSPExit ), vspCALL_CDECL );
    assert( r >= 0 ); // TODO: Example


    r = se->RegisterGlobalFunction( "void ClearVSPModel()", vspFUNCTION( vsp::ClearVSPModel ), vspCALL_CDECL );
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "string GetVSPFileName()", vspFUNCTION( vsp::GetVSPFileName ), vspCALL_CDECL );
    assert( r >= 0 );


    //==== File I/O Functions ====//

    r = se->RegisterGlobalFunction( "void ReadVSPFile( const string & in file_name )", vspFUNCTION( vsp::ReadVSPFile ), vspCALL_CDECL );
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "void WriteVSPFile( const string & in file_name, int set )", vspFUNCTION( vsp::WriteVSPFile ), vspCALL_CDECL );
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "void SetVSP3FileName( const string & in file_name )", vspFUNCTION( vsp::SetVSP3FileName ), vspCALL_CDECL );
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "void InsertVSPFile( const string & in file_name, const string & in parent )", vspFUNCTION( vsp::InsertVSPFile ), vspCALL_CDECL );
    assert( r >= 0 ); // TODO: Example


    r = se->RegisterGlobalFunction( "string ExportFile( const string & in file_name, int thick_set, int file_type, int subsFlag = 1, int thin_set = -1 )", vspFUNCTION( vsp::ExportFile ), vspCALL_CDECL );
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "string ImportFile( const string & in file_name, int file_type, const string & in parent )", vspFUNCTION( vsp::ImportFile ), vspCALL_CDECL );
    assert( r >= 0 ); // TODO: Example


    r = se->RegisterGlobalFunction( "void SetBEMPropID( const string & in prop_id )", vspFUNCTION( vsp::SetBEMPropID ), vspCALL_CDECL );
    assert( r >= 0 );

    //==== Design File Functions ====//

    r = se->RegisterGlobalFunction( "void ReadApplyDESFile( const string & in file_name )", vspFUNCTION( vsp::ReadApplyDESFile ), vspCALL_CDECL );
    assert( r >= 0 ); // TODO: Example


    r = se->RegisterGlobalFunction( "void WriteDESFile( const string & in file_name )", vspFUNCTION( vsp::WriteDESFile ), vspCALL_CDECL );
    assert( r >= 0 ); // TODO: Example


    r = se->RegisterGlobalFunction( "void ReadApplyXDDMFile( const string & in file_name )", vspFUNCTION( vsp::ReadApplyXDDMFile ), vspCALL_CDECL );
    assert( r >= 0 ); // TODO: Example


    r = se->RegisterGlobalFunction( "void WriteXDDMFile( const string & in file_name )", vspFUNCTION( vsp::WriteXDDMFile ), vspCALL_CDECL );
    assert( r >= 0 ); // TODO: Example


    r = se->RegisterGlobalFunction( "int GetNumDesignVars()", vspFUNCTION( vsp::GetNumDesignVars ), vspCALL_CDECL );
    assert( r >= 0 ); // TODO: Example


    r = se->RegisterGlobalFunction( "void AddDesignVar( const string & in parm_id, int type )", vspFUNCTION( vsp::AddDesignVar ), vspCALL_CDECL );
    assert( r >= 0 ); // TODO: Example


    r = se->RegisterGlobalFunction( "void DeleteAllDesignVars()", vspFUNCTION( vsp::DeleteAllDesignVars ), vspCALL_CDECL );
    assert( r >= 0 ); // TODO: Example


    r = se->RegisterGlobalFunction( "string GetDesignVar( int index )", vspFUNCTION( vsp::GetDesignVar ), vspCALL_CDECL );
    assert( r >= 0 ); // TODO: Example


    r = se->RegisterGlobalFunction( "int GetDesignVarType( int index )", vspFUNCTION( vsp::GetDesignVarType ), vspCALL_CDECL );
    assert( r >= 0 ); // TODO: Example

    //==== Computations ====//

    r = se->RegisterGlobalFunction( "string ComputeMassProps( int set, int num_slices, int idir = X_DIR )", vspFUNCTION( vsp::ComputeMassProps ), vspCALL_CDECL );
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "string ComputeCompGeom( int set, bool half_mesh, int file_export_types )", vspFUNCTION( vsp::ComputeCompGeom ), vspCALL_CDECL );
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "string ComputePlaneSlice( int set, int num_slices, const vec3d & in norm, bool auto_bnd, double start_bnd = 0, double end_bnd = 0, bool measureduct = false )", vspFUNCTION( vsp::ComputePlaneSlice ), vspCALL_CDECL );
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "void ComputeDegenGeom( int set, int file_type )", vspFUNCTION( vsp::ComputeDegenGeom ), vspCALL_CDECL );
    assert( r >= 0 );

    //==== CFD Mesh ====//

    r = se->RegisterGlobalFunction( "void SetComputationFileName( int file_type, const string & in file_name )", vspFUNCTION( vsp::SetComputationFileName ), vspCALL_CDECL );
    assert( r >= 0 ); // TODO: FIXME for FEA Mesh


    r = se->RegisterGlobalFunction( "void ComputeCFDMesh( int set, int degenset, int file_type )", vspFUNCTION( vsp::ComputeCFDMesh ), vspCALL_CDECL );
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "void SetCFDMeshVal( int type, double val )", vspFUNCTION( vsp::SetCFDMeshVal ), vspCALL_CDECL );
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "void SetCFDWakeFlag( const string & in geom_id, bool flag )", vspFUNCTION( vsp::SetCFDWakeFlag ), vspCALL_CDECL );
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "void DeleteAllCFDSources()", vspFUNCTION( vsp::DeleteAllCFDSources ), vspCALL_CDECL );
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "void AddDefaultSources()", vspFUNCTION( vsp::AddDefaultSources ), vspCALL_CDECL );
    assert( r >= 0 );


    r = se->RegisterGlobalFunction(
        "void AddCFDSource( int type, const string & in geom_id, int surf_index, double l1, double r1, double u1, double w1, double l2 = 0, double r2 = 0, double u2 = 0, double w2 = 0 )",
        vspFUNCTION( vsp::AddCFDSource ), vspCALL_CDECL );
    assert( r >= 0 );


    //==== Analysis Functions ====//

    r = se->RegisterGlobalFunction( "int GetNumAnalysis( )", vspFUNCTION( vsp::GetNumAnalysis ), vspCALL_CDECL );
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "array<string>@ ListAnalysis()", vspMETHOD( ScriptMgrSingleton, ListAnalysis ), vspCALL_THISCALL_ASGLOBAL, &ScriptMgr );
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "array<string>@ GetAnalysisInputNames(const string & in analysis )", vspMETHOD( ScriptMgrSingleton, GetAnalysisInputNames ), vspCALL_THISCALL_ASGLOBAL, &ScriptMgr );
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "string GetAnalysisDoc( const string & in analysis )", vspFUNCTION( vsp::GetAnalysisDoc ), vspCALL_CDECL );
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "string GetAnalysisInputDoc( const string & in analysis, const string & in name )", vspFUNCTION( vsp::GetNumAnalysisInputData ), vspCALL_CDECL );
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "string ExecAnalysis( const string & in analysis )", vspFUNCTION( vsp::ExecAnalysis ), vspCALL_CDECL );
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "int GetNumAnalysisInputData( const string & in analysis, const string & in name )", vspFUNCTION( vsp::GetNumAnalysisInputData ), vspCALL_CDECL );
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "int GetAnalysisInputType( const string & in analysis, const string & in name )", vspFUNCTION( vsp::GetAnalysisInputType ), vspCALL_CDECL );
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "array<int>@ GetIntAnalysisInput( const string & in analysis, const string & in name, int index = 0 )", vspMETHOD( ScriptMgrSingleton, GetIntAnalysisInput ), vspCALL_THISCALL_ASGLOBAL, &ScriptMgr );
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "array<double>@ GetDoubleAnalysisInput( const string & in analysis, const string & in name, int index = 0 )", vspMETHOD( ScriptMgrSingleton, GetDoubleAnalysisInput ), vspCALL_THISCALL_ASGLOBAL, &ScriptMgr );
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "array<string>@ GetStringAnalysisInput( const string & in analysis, const string & in name, int index = 0 )", vspMETHOD( ScriptMgrSingleton, GetStringAnalysisInput ), vspCALL_THISCALL_ASGLOBAL, &ScriptMgr );
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "array<vec3d>@ GetVec3dAnalysisInput( const string & in analysis, const string & in name, int index = 0 )", vspMETHOD( ScriptMgrSingleton, GetVec3dAnalysisInput ), vspCALL_THISCALL_ASGLOBAL, &ScriptMgr );
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "void PrintAnalysisInputs( const string & in analysis )", vspFUNCTION( vsp::PrintAnalysisInputs ), vspCALL_CDECL );
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "void PrintAnalysisDocs( const string & in analysis )", vspFUNCTION( vsp::PrintAnalysisDocs ), vspCALL_CDECL );
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "void SetAnalysisInputDefaults( const string & in analysis )", vspFUNCTION( vsp::SetAnalysisInputDefaults ), vspCALL_CDECL );
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "void SetIntAnalysisInput( const string & in analysis, const string & in name, array<int>@ indata_arr, int index = 0 )", vspMETHOD( ScriptMgrSingleton, SetIntAnalysisInput ), vspCALL_THISCALL_ASGLOBAL, &ScriptMgr );
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "void SetDoubleAnalysisInput( const string & in analysis, const string & in name, array<double>@ indata_arr, int index = 0 )", vspMETHOD( ScriptMgrSingleton, SetDoubleAnalysisInput ), vspCALL_THISCALL_ASGLOBAL, &ScriptMgr );
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "void SetStringAnalysisInput( const string & in analysis, const string & in name, array<string>@ indata_arr, int index = 0 )", vspMETHOD( ScriptMgrSingleton, SetStringAnalysisInput ), vspCALL_THISCALL_ASGLOBAL, &ScriptMgr );
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "void SetVec3dAnalysisInput( const string & in analysis, const string & in name, array<vec3d>@ indata_arr, int index = 0 )", vspMETHOD( ScriptMgrSingleton, SetVec3dAnalysisInput ), vspCALL_THISCALL_ASGLOBAL, &ScriptMgr );
    assert( r >= 0 );


    //==== Results Functions ====//

    r = se->RegisterGlobalFunction( "int GetNumResults( const string & in name )", vspFUNCTION( vsp::GetNumResults ), vspCALL_CDECL );
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "string GetResultsName( const string & in results_id )", vspFUNCTION( vsp::GetResultsName ), vspCALL_CDECL );
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "string GetResultsSetDoc( const string & in results_id )",
                                    vspFUNCTION( vsp::GetResultsSetDoc ), vspCALL_CDECL );
    assert( r >= 0 );


    r = se->RegisterGlobalFunction(
            "string GetResultsEntryDoc( const string & in results_id, const string & in data_name )",
            vspFUNCTION( vsp::GetResultsSetDoc ), vspCALL_CDECL );
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "string FindResultsID( const string & in name, int index = 0 )", vspFUNCTION( vsp::FindResultsID ), vspCALL_CDECL );
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "string FindLatestResultsID( const string & in name )", vspFUNCTION( vsp::FindLatestResultsID ), vspCALL_CDECL );
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "int GetNumData( const string & in results_id, const string & in data_name )", vspFUNCTION( vsp::GetNumData ), vspCALL_CDECL );
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "int GetResultsType( const string & in results_id, const string & in data_name )", vspFUNCTION( vsp::GetResultsType ), vspCALL_CDECL );
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "array<string>@ GetAllResultsNames()", vspMETHOD( ScriptMgrSingleton, GetAllResultsNames ), vspCALL_THISCALL_ASGLOBAL, &ScriptMgr );
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "array<string>@ GetAllDataNames(const string & in results_id )", vspMETHOD( ScriptMgrSingleton, GetAllDataNames ), vspCALL_THISCALL_ASGLOBAL, &ScriptMgr );
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "array<int>@ GetIntResults( const string & in id, const string & in name, int index = 0 )", vspMETHOD( ScriptMgrSingleton, GetIntResults ), vspCALL_THISCALL_ASGLOBAL, &ScriptMgr );
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "array<double>@ GetDoubleResults( const string & in id, const string & in name, int index = 0 )", vspMETHOD( ScriptMgrSingleton, GetDoubleResults ), vspCALL_THISCALL_ASGLOBAL, &ScriptMgr );
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "array<array<double>@>@ GetDoubleMatResults( const string & in id, const string & in name, int index = 0 )", vspMETHOD( ScriptMgrSingleton, GetDoubleMatResults ), vspCALL_THISCALL_ASGLOBAL, &ScriptMgr );
    assert( r >= 0 ); // TODO: Example


    r = se->RegisterGlobalFunction( "array<string>@ GetStringResults( const string & in id, const string & in name, int index = 0 )", vspMETHOD( ScriptMgrSingleton, GetStringResults ), vspCALL_THISCALL_ASGLOBAL, &ScriptMgr );
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "array<vec3d>@ GetVec3dResults( const string & in id, const string & in name, int index = 0 )", vspMETHOD( ScriptMgrSingleton, GetVec3dResults ), vspCALL_THISCALL_ASGLOBAL, &ScriptMgr );
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "string CreateGeomResults( const string & in geom_id, const string & in name )", vspFUNCTION( vsp::CreateGeomResults ), vspCALL_CDECL );
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "void DeleteAllResults()", vspFUNCTION( vsp::DeleteAllResults ), vspCALL_CDECL );
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "void DeleteResult( const string & in id )", vspFUNCTION( vsp::DeleteResult ), vspCALL_CDECL );
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "void WriteResultsCSVFile( const string & in id, const string & in file_name )", vspFUNCTION( vsp::WriteResultsCSVFile ), vspCALL_CDECL );
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "void PrintResults( const string & in id )", vspFUNCTION( vsp::PrintResults ), vspCALL_CDECL );
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "void PrintResultsDoc( const string & in id )", vspFUNCTION( vsp::PrintResultsDocs ), vspCALL_CDECL );
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "void WriteTestResults()", vspMETHOD( ResultsMgrSingleton, WriteTestResults ), vspCALL_THISCALL_ASGLOBAL, &ResultsMgr );
    assert( r >= 0 );


    //==== Geom Functions ====//

    r = se->RegisterGlobalFunction( "array<string>@ GetGeomTypes()", vspMETHOD( ScriptMgrSingleton, GetGeomTypes ), vspCALL_THISCALL_ASGLOBAL, &ScriptMgr );
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "string AddGeom( const string & in type, const string & in parent = string() )", vspFUNCTION( vsp::AddGeom ), vspCALL_CDECL );
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "void UpdateGeom(const string & in geom_id)", vspFUNCTION( vsp::UpdateGeom ), vspCALL_CDECL );
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "void DeleteGeom(const string & in geom_id)", vspFUNCTION( vsp::DeleteGeom ), vspCALL_CDECL );
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "void DeleteGeomVec( array<string>@ del_arr )", vspMETHOD( ScriptMgrSingleton, DeleteGeomVec ), vspCALL_THISCALL_ASGLOBAL, &ScriptMgr );
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "void CutGeomToClipboard(const string & in geom_id)", vspFUNCTION( vsp::CutGeomToClipboard ), vspCALL_CDECL );
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "void CopyGeomToClipboard(const string & in geom_id)", vspFUNCTION( vsp::CopyGeomToClipboard ), vspCALL_CDECL );
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "array<string>@ PasteGeomClipboard( const string & in parent_id = \"\" )", vspMETHOD( ScriptMgrSingleton, PasteGeomClipboard ), vspCALL_THISCALL_ASGLOBAL, &ScriptMgr );
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "array<string>@ FindGeoms()", vspMETHOD( ScriptMgrSingleton, FindGeoms ), vspCALL_THISCALL_ASGLOBAL, &ScriptMgr );
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "array<string>@ FindGeomsWithName(const string & in name)", vspMETHOD( ScriptMgrSingleton, FindGeomsWithName ), vspCALL_THISCALL_ASGLOBAL, &ScriptMgr );
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "string FindGeom(const string & in name, int index)", vspFUNCTION( vsp::FindGeom ), vspCALL_CDECL );
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "void SetGeomName( const string & in geom_id, const string & in name )", vspFUNCTION( vsp::SetGeomName ), vspCALL_CDECL );
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "string GetGeomName( const string & in geom_id )", vspFUNCTION( vsp::GetGeomName ), vspCALL_CDECL );
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "array<string>@ GetGeomParmIDs(const string & in geom_id )", vspMETHOD( ScriptMgrSingleton, GetGeomParmIDs ), vspCALL_THISCALL_ASGLOBAL, &ScriptMgr );
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "int GetGeomVSPSurfCfdType( const string & in geom_id, int main_surf_ind = 0 )", vspFUNCTION( vsp::GetGeomVSPSurfCfdType ), vspCALL_CDECL );
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "int GetGeomVSPSurfType( const string & in geom_id, int main_surf_ind = 0 )", vspFUNCTION( vsp::GetGeomVSPSurfType ), vspCALL_CDECL );
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "string GetGeomTypeName(const string & in geom_id )", vspFUNCTION( vsp::GetGeomTypeName ), vspCALL_CDECL );
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "int GetNumMainSurfs( const string & in geom_id )", vspFUNCTION( vsp::GetNumMainSurfs ), vspCALL_CDECL );
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "int GetTotalNumSurfs( const string & in geom_id )", vspFUNCTION( vsp::GetTotalNumSurfs ), vspCALL_CDECL );
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "vec3d GetGeomBBoxMax( const string & in geom_id, int main_surf_ind = 0, bool ref_frame_is_absolute = true )", vspFUNCTION( vsp::GetGeomBBoxMax ), vspCALL_CDECL );
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "vec3d GetGeomBBoxMin( const string & in geom_id, int main_surf_ind = 0, bool ref_frame_is_absolute = true )", vspFUNCTION( vsp::GetGeomBBoxMin ), vspCALL_CDECL );
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "string GetGeomParent( const string & in geom_id )", vspFUNCTION( vsp::GetGeomParent ), vspCALL_CDECL );
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "array<string>@ GetGeomChildren( const string & in geom_id )", vspMETHOD( ScriptMgrSingleton, GetGeomChildren ), vspCALL_THISCALL_ASGLOBAL, &ScriptMgr );
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "void SetDriverGroup( const string & in geom_id, int section_index, int driver_0, int driver_1 = -1, int driver_2 = -1)", vspFUNCTION( vsp::SetDriverGroup ), vspCALL_CDECL );
    assert( r >= 0 );

    //==== SubSurface Functions ====//

    r = se->RegisterGlobalFunction( "string AddSubSurf( const string & in geom_id, int type, int surfindex = 0 )", vspFUNCTION( vsp::AddSubSurf ), vspCALL_CDECL );
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "void DeleteSubSurf( const string & in geom_id, const string & in sub_id )", vspFUNCTIONPR( vsp::DeleteSubSurf, ( const string &, const string & ), void ), vspCALL_CDECL );
    assert( r >= 0 );
    // TODO: Why are there two DeleteSubSurf if Geom ID isn't needed?

    r = se->RegisterGlobalFunction( "void DeleteSubSurf( const string & in sub_id )", vspFUNCTIONPR( vsp::DeleteSubSurf, ( const string & ), void ), vspCALL_CDECL );
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "string GetSubSurf( const string & in geom_id, int index )", vspFUNCTIONPR( vsp::GetSubSurf, ( const string &, int ), string ), vspCALL_CDECL );
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "array<string>@ GetSubSurf( const string & in geom_id, const string & in name )", vspMETHOD( ScriptMgrSingleton, GetSubSurf ), vspCALL_THISCALL_ASGLOBAL, &ScriptMgr );
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "void SetSubSurfName( const string & in geom_id, const string & in sub_id, const string & in name )", vspFUNCTIONPR( vsp::SetSubSurfName, ( const string &, const string &, const string & ), void ), vspCALL_CDECL );
    assert( r >= 0 );
    // TODO: Why are there two SetSubSurfName if Geom ID isn't needed?

    r = se->RegisterGlobalFunction( "void SetSubSurfName( const string & in sub_id, const string & in name )", vspFUNCTIONPR( vsp::SetSubSurfName, ( const string &, const string & ), void ), vspCALL_CDECL );
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "string GetSubSurfName( const string & in geom_id, const string & in sub_id )", vspFUNCTIONPR( vsp::GetSubSurfName, ( const string &, const string & ), string ), vspCALL_CDECL );
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "string GetSubSurfName( const string & in sub_id )", vspFUNCTIONPR( vsp::GetSubSurfName, ( const string & ), string ), vspCALL_CDECL );
    assert( r >= 0 );
    // TODO: Why are there two GetSubSurfName if Geom ID isn't needed?

    r = se->RegisterGlobalFunction( "int GetSubSurfIndex( const string & in sub_id )", vspFUNCTION( vsp::GetSubSurfIndex ), vspCALL_CDECL );
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "array<string>@ GetSubSurfIDVec( const string & in geom_id )", vspMETHOD( ScriptMgrSingleton, GetSubSurfIDVec ), vspCALL_THISCALL_ASGLOBAL, &ScriptMgr );
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "array<string>@ GetAllSubSurfIDs()", vspMETHOD( ScriptMgrSingleton, GetAllSubSurfIDs ), vspCALL_THISCALL_ASGLOBAL, &ScriptMgr );
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "int GetNumSubSurf( const string & in geom_id )", vspFUNCTION( vsp::GetNumSubSurf ), vspCALL_CDECL );
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "int GetSubSurfType( const string & in sub_id )", vspFUNCTION( vsp::GetSubSurfType ), vspCALL_CDECL );
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "array<string>@ GetSubSurfParmIDs(const string & in sub_id )", vspMETHOD( ScriptMgrSingleton, GetSubSurfParmIDs ), vspCALL_THISCALL_ASGLOBAL, &ScriptMgr );
    assert( r >= 0 );

    //==== VSPAERO CS Group Functions ====//

    r = se->RegisterGlobalFunction("void AutoGroupVSPAEROControlSurfaces()", vspFUNCTION(vsp::AutoGroupVSPAEROControlSurfaces), vspCALL_CDECL );
    assert(r >= 0);


    r = se->RegisterGlobalFunction( "int GetNumControlSurfaceGroups()", vspFUNCTION( vsp::GetNumControlSurfaceGroups ), vspCALL_CDECL );
    assert( r >= 0 );


    r = se->RegisterGlobalFunction("int CreateVSPAEROControlSurfaceGroup()", vspFUNCTION(vsp::CreateVSPAEROControlSurfaceGroup), vspCALL_CDECL );
    assert(r >= 0);


    r = se->RegisterGlobalFunction("void AddAllToVSPAEROControlSurfaceGroup( int CSGroupIndex )", vspFUNCTION(vsp::AddAllToVSPAEROControlSurfaceGroup ), vspCALL_CDECL );
    assert(r >= 0);


    r = se->RegisterGlobalFunction("void RemoveAllFromVSPAEROControlSurfaceGroup( int CSGroupIndex )", vspFUNCTION(vsp::RemoveAllFromVSPAEROControlSurfaceGroup ), vspCALL_CDECL );
    assert(r >= 0);


    r = se->RegisterGlobalFunction( "array<string>@ GetActiveCSNameVec( int CSGroupIndex )", vspMETHOD( ScriptMgrSingleton, GetActiveCSNameVec ), vspCALL_THISCALL_ASGLOBAL, &ScriptMgr );
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "array<string>@ GetCompleteCSNameVec( )", vspMETHOD( ScriptMgrSingleton, GetCompleteCSNameVec ), vspCALL_THISCALL_ASGLOBAL, &ScriptMgr );
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "array<string>@ GetAvailableCSNameVec( int CSGroupIndex )", vspMETHOD( ScriptMgrSingleton, GetAvailableCSNameVec ), vspCALL_THISCALL_ASGLOBAL, &ScriptMgr );
    assert( r >= 0 );


    r = se->RegisterGlobalFunction("void SetVSPAEROControlGroupName( const string & in name, int CSGroupIndex )", vspFUNCTION(vsp::SetVSPAEROControlGroupName), vspCALL_CDECL );
    assert(r >= 0);


    r = se->RegisterGlobalFunction( "string GetVSPAEROControlGroupName( int CSGroupIndex )", vspFUNCTION( vsp::GetVSPAEROControlGroupName ), vspCALL_CDECL );
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "void AddSelectedToCSGroup( array<int>@ selected, int CSGroupIndex )", vspMETHOD( ScriptMgrSingleton, AddSelectedToCSGroup ), vspCALL_THISCALL_ASGLOBAL, &ScriptMgr );
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "void RemoveSelectedFromCSGroup( array<int>@ selected, int CSGroupIndex )", vspMETHOD( ScriptMgrSingleton, RemoveSelectedFromCSGroup ), vspCALL_THISCALL_ASGLOBAL, &ScriptMgr );
    assert( r >= 0 ); // FIXME: RemoveSelectedFromCSGroup not working

    //==== VSPAERO Functions ====//

    r = se->RegisterGlobalFunction( "string GetVSPAERORefWingID()", vspFUNCTION( vsp::GetVSPAERORefWingID ), vspCALL_CDECL );
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "string SetVSPAERORefWingID( const string & in geom_id )", vspFUNCTION( vsp::SetVSPAERORefWingID ), vspCALL_CDECL );
    assert( r >= 0 );

    //==== VSPAERO Disk and Prop Functions ====//

    r = se->RegisterGlobalFunction( "string FindActuatorDisk( int disk_index )", vspFUNCTION( vsp::FindActuatorDisk ), vspCALL_CDECL );
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "int GetNumActuatorDisks()", vspFUNCTION( vsp::GetNumActuatorDisks ), vspCALL_CDECL );
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "string FindUnsteadyGroup( int group_index )", vspFUNCTION( vsp::FindUnsteadyGroup ), vspCALL_CDECL );
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "string GetUnsteadyGroupName( int group_index )", vspFUNCTION( vsp::GetUnsteadyGroupName ), vspCALL_CDECL );
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "array<string>@ GetUnsteadyGroupCompIDs( int group_index )", vspMETHOD( ScriptMgrSingleton, GetUnsteadyGroupCompIDs ), vspCALL_THISCALL_ASGLOBAL, &ScriptMgr );
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "array<int>@ GetUnsteadyGroupSurfIndexes( int group_index )", vspMETHOD( ScriptMgrSingleton, GetUnsteadyGroupSurfIndexes ), vspCALL_THISCALL_ASGLOBAL, &ScriptMgr );
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "int GetNumUnsteadyGroups()", vspFUNCTION( vsp::GetNumUnsteadyGroups ), vspCALL_CDECL );
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "int GetNumUnsteadyRotorGroups()", vspFUNCTION( vsp::GetNumUnsteadyRotorGroups ), vspCALL_CDECL );
    assert( r >= 0 );

    //==== XSecSurf Functions ====//

    r = se->RegisterGlobalFunction( "int GetNumXSecSurfs( const string & in geom_id )", vspFUNCTION( vsp::GetNumXSecSurfs ), vspCALL_CDECL );
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "string GetXSecSurf( const string & in geom_id, int index )", vspFUNCTION( vsp::GetXSecSurf ), vspCALL_CDECL );
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "int GetNumXSec( const string & in xsec_surf_id )", vspFUNCTION( vsp::GetNumXSec ), vspCALL_CDECL );
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "string GetXSec( const string & in xsec_surf_id, int xsec_index )", vspFUNCTION( vsp::GetXSec ), vspCALL_CDECL );
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "void ChangeXSecShape( const string & in xsec_surf_id, int xsec_index, int type )", vspFUNCTION( vsp::ChangeXSecShape ), vspCALL_CDECL );
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "void SetXSecSurfGlobalXForm( const string & in xsec_surf_id, const Matrix4d & in mat )", vspFUNCTION( vsp::SetXSecSurfGlobalXForm ), vspCALL_CDECL );
    assert( r >= 0 ); // TODO: Add Example


    r = se->RegisterGlobalFunction( "Matrix4d GetXSecSurfGlobalXForm( const string & in xsec_surf_id )", vspFUNCTION( vsp::GetXSecSurfGlobalXForm ), vspCALL_CDECL );
    assert( r >= 0 ); // TODO: Add Example

    //==== XSec Functions ====//

    r = se->RegisterGlobalFunction( "void CutXSec( const string & in geom_id, int index )", vspFUNCTION( vsp::CutXSec ), vspCALL_CDECL );
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "void CopyXSec( const string & in geom_id, int index )", vspFUNCTION( vsp::CopyXSec ), vspCALL_CDECL );
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "void PasteXSec( const string & in geom_id, int index )", vspFUNCTION( vsp::PasteXSec ), vspCALL_CDECL );
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "void InsertXSec( const string & in geom_id, int index, int type )", vspFUNCTION( vsp::InsertXSec ), vspCALL_CDECL );
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "int GetXSecShape( const string& in xsec_id )", vspFUNCTION( vsp::GetXSecShape ), vspCALL_CDECL );
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "double GetXSecWidth( const string& in xsec_id )", vspFUNCTION( vsp::GetXSecWidth ), vspCALL_CDECL );
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "double GetXSecHeight( const string& in xsec_id )", vspFUNCTION( vsp::GetXSecHeight ), vspCALL_CDECL );
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "void SetXSecWidth( const string& in xsec_id, double w )", vspFUNCTION( vsp::SetXSecWidth ), vspCALL_CDECL );
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "void SetXSecHeight( const string& in xsec_id, double h )", vspFUNCTION( vsp::SetXSecHeight ), vspCALL_CDECL );
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "void SetXSecWidthHeight( const string& in xsec_id, double w, double h )", vspFUNCTION( vsp::SetXSecWidthHeight ), vspCALL_CDECL );
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "array<string>@ GetXSecParmIDs(const string & in xsec_id )", vspMETHOD( ScriptMgrSingleton, GetXSecParmIDs ), vspCALL_THISCALL_ASGLOBAL, &ScriptMgr );
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "string GetXSecParm( const string& in xsec_id, const string& in name )", vspFUNCTION( vsp::GetXSecParm ), vspCALL_CDECL );
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "array<vec3d>@ ReadFileXSec(const string& in xsec_id, const string& in file_name )", vspMETHOD( ScriptMgrSingleton, ReadFileXSec ), vspCALL_THISCALL_ASGLOBAL, &ScriptMgr );
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "void SetXSecPnts( const string& in xsec_id, array<vec3d>@ pnt_arr )", vspMETHOD( ScriptMgrSingleton, SetXSecPnts ), vspCALL_THISCALL_ASGLOBAL, &ScriptMgr );
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "vec3d ComputeXSecPnt( const string& in xsec_id, double fract )", vspFUNCTION( vsp::ComputeXSecPnt ), vspCALL_CDECL );
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "vec3d ComputeXSecTan( const string& in xsec_id, double fract )", vspFUNCTION( vsp::ComputeXSecTan ), vspCALL_CDECL );
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "void ResetXSecSkinParms( const string& in xsec_id )", vspFUNCTION( vsp::ResetXSecSkinParms ), vspCALL_CDECL );
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "void SetXSecContinuity( const string& in xsec_id, int cx )", vspFUNCTION( vsp::SetXSecContinuity ), vspCALL_CDECL );
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "void SetXSecTanAngles( const string& in xsec_id, int side, double top, double right = -1.0e12, double bottom = -1.0e12, double left = -1.0e12 )", vspFUNCTION( vsp::SetXSecTanAngles ), vspCALL_CDECL );
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "void SetXSecTanSlews( const string& in xsec_id, int side, double top, double right = -1.0e12, double bottom = -1.0e12, double left = -1.0e12 )", vspFUNCTION( vsp::SetXSecTanSlews ), vspCALL_CDECL );
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "void SetXSecTanStrengths( const string& in xsec_id, int side, double top, double right = -1.0e12, double bottom = -1.0e12, double left = -1.0e12 )", vspFUNCTION( vsp::SetXSecTanStrengths ), vspCALL_CDECL );
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "void SetXSecCurvatures( const string& in xsec_id, int side, double top, double right = -1.0e12, double bottom = -1.0e12, double left = -1.0e12 )", vspFUNCTION( vsp::SetXSecCurvatures ), vspCALL_CDECL );
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "void ReadFileAirfoil( const string& in xsec_id, const string& in file_name )", vspFUNCTION( vsp::ReadFileAirfoil ), vspCALL_CDECL );
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "void SetAirfoilUpperPnts( const string& in xsec_id, array<vec3d>@ up_pnt_vec )", vspMETHOD( ScriptMgrSingleton, SetAirfoilUpperPnts ), vspCALL_THISCALL_ASGLOBAL, &ScriptMgr );
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "void SetAirfoilLowerPnts( const string& in xsec_id, array<vec3d>@ low_pnt_vec )", vspMETHOD( ScriptMgrSingleton, SetAirfoilLowerPnts ), vspCALL_THISCALL_ASGLOBAL, &ScriptMgr );
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "void SetAirfoilPnts( const string& in xsec_id, array<vec3d>@ up_pnt_vec, array<vec3d>@ low_pnt_vec )", vspMETHOD( ScriptMgrSingleton, SetAirfoilPnts ), vspCALL_THISCALL_ASGLOBAL, &ScriptMgr );
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "array<vec3d>@ GetHersheyBarLiftDist( const int& in npts, const double& in alpha, const double& in Vinf, const double& in span, bool full_span_flag = false )", vspMETHOD( ScriptMgrSingleton, GetHersheyBarLiftDist ), vspCALL_THISCALL_ASGLOBAL, &ScriptMgr );
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "array<vec3d>@ GetHersheyBarDragDist( const int& in npts, const double& in alpha, const double& in Vinf, const double& in span, bool full_span_flag = false )", vspMETHOD( ScriptMgrSingleton, GetHersheyBarDragDist ), vspCALL_THISCALL_ASGLOBAL, &ScriptMgr );
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "array<vec3d>@ GetVKTAirfoilPnts( const int& in npts, const double& in alpha, const double& in epsilon, const double& in kappa, const double& in tau )", vspMETHOD( ScriptMgrSingleton, GetVKTAirfoilPnts ), vspCALL_THISCALL_ASGLOBAL, &ScriptMgr );
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "array<double>@ GetVKTAirfoilCpDist( const double& in alpha, const double& in epsilon, const double& in kappa, const double& in tau, array<vec3d>@ xydata )", vspMETHOD( ScriptMgrSingleton, GetVKTAirfoilCpDist ), vspCALL_THISCALL_ASGLOBAL, &ScriptMgr );
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "array<vec3d>@ GetEllipsoidSurfPnts( const vec3d& in center, const vec3d& in abc_rad, int u_npts = 20, int w_npts = 20 )", vspMETHOD( ScriptMgrSingleton, GetEllipsoidSurfPnts ), vspCALL_THISCALL_ASGLOBAL, &ScriptMgr );
    assert( r >= 0 );  // TODO: Example


    r = se->RegisterGlobalFunction( "array<vec3d>@ GetFeatureLinePnts( const string& in geom_id )", vspMETHOD( ScriptMgrSingleton, GetFeatureLinePnts ), vspCALL_THISCALL_ASGLOBAL, &ScriptMgr );
    assert( r >= 0 );  // TODO: Example


    r = se->RegisterGlobalFunction( "array<double>@ GetEllipsoidCpDist( array<vec3d>@ surf_pnt_arr, const vec3d& in abc_rad, const vec3d& in V_inf )", vspMETHOD( ScriptMgrSingleton, GetEllipsoidCpDist ), vspCALL_THISCALL_ASGLOBAL, &ScriptMgr );
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "array<vec3d>@ GetAirfoilUpperPnts(const string& in xsec_id )", vspMETHOD( ScriptMgrSingleton, GetAirfoilUpperPnts ), vspCALL_THISCALL_ASGLOBAL, &ScriptMgr );
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "array<vec3d>@ GetAirfoilLowerPnts(const string& in xsec_id )", vspMETHOD( ScriptMgrSingleton, GetAirfoilLowerPnts ), vspCALL_THISCALL_ASGLOBAL, &ScriptMgr );
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "array<double>@ GetUpperCSTCoefs( const string & in xsec_id )", vspMETHOD( ScriptMgrSingleton, GetUpperCSTCoefs ), vspCALL_THISCALL_ASGLOBAL, &ScriptMgr );
    assert( r >= 0 ); // TODO: Example


    r = se->RegisterGlobalFunction( "array<double>@ GetLowerCSTCoefs( const string & in xsec_id )", vspMETHOD( ScriptMgrSingleton, GetLowerCSTCoefs ), vspCALL_THISCALL_ASGLOBAL, &ScriptMgr );
    assert( r >= 0 );  // TODO: Example


    r = se->RegisterGlobalFunction( "int GetUpperCSTDegree( const string& in xsec_id )", vspFUNCTION( vsp::GetUpperCSTDegree ), vspCALL_CDECL );
    assert( r >= 0 );  // TODO: Example


    r = se->RegisterGlobalFunction( "int GetLowerCSTDegree( const string& in xsec_id )", vspFUNCTION( vsp::GetLowerCSTDegree ), vspCALL_CDECL );
    assert( r >= 0 );  // TODO: Example


    r = se->RegisterGlobalFunction( "void SetUpperCST( const string& in xsec_id, int deg, array<double>@ coeff_arr )", vspMETHOD( ScriptMgrSingleton, SetUpperCST ), vspCALL_THISCALL_ASGLOBAL, &ScriptMgr );
    assert( r >= 0 );  // TODO: Example


    r = se->RegisterGlobalFunction( "void SetLowerCST( const string& in xsec_id, int deg, array<double>@ coeff_arr )", vspMETHOD( ScriptMgrSingleton, SetLowerCST ), vspCALL_THISCALL_ASGLOBAL, &ScriptMgr );
    assert( r >= 0 );  // TODO: Example


    r = se->RegisterGlobalFunction( "void PromoteCSTUpper( const string& in xsec_id )", vspFUNCTION( vsp::PromoteCSTUpper ), vspCALL_CDECL );
    assert( r >= 0 );  // TODO: Example


    r = se->RegisterGlobalFunction( "void PromoteCSTLower( const string& in xsec_id )", vspFUNCTION( vsp::PromoteCSTLower ), vspCALL_CDECL );
    assert( r >= 0 );  // TODO: Example


    r = se->RegisterGlobalFunction( "void DemoteCSTUpper( const string& in xsec_id )", vspFUNCTION( vsp::DemoteCSTUpper ), vspCALL_CDECL );
    assert( r >= 0 );  // TODO: Example


    r = se->RegisterGlobalFunction( "void DemoteCSTLower( const string& in xsec_id )", vspFUNCTION( vsp::DemoteCSTLower ), vspCALL_CDECL );
    assert( r >= 0 );  // TODO: Example


    r = se->RegisterGlobalFunction( "void FitAfCST( const string& in xsec_surf_id, int xsec_index, int deg )", vspFUNCTION( vsp::FitAfCST ), vspCALL_CDECL );
    assert( r >= 0 );  // TODO: Example


    r = se->RegisterGlobalFunction( "void WriteBezierAirfoil( const string& in file_name, const string& in geom_id, const double& in foilsurf_u )", vspFUNCTION( vsp::WriteBezierAirfoil ), vspCALL_CDECL );
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "void WriteSeligAirfoil( const string& in file_name, const string& in geom_id, const double& in foilsurf_u )", vspFUNCTION( vsp::WriteSeligAirfoil ), vspCALL_CDECL );
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "array<vec3d>@ GetAirfoilCoordinates( const string& in geom_id, const double& in foilsurf_u )", vspMETHOD( ScriptMgrSingleton, GetAirfoilCoordinates ), vspCALL_THISCALL_ASGLOBAL, &ScriptMgr );
    assert( r >= 0 );

    //==== Edit Curve XSec Functions ====//

    r = se->RegisterGlobalFunction( "void EditXSecInitShape( const string& in xsec_id )", vspFUNCTION( vsp::EditXSecInitShape ), vspCALL_CDECL );
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "void EditXSecConvertTo( const string& in xsec_id, const int& in newtype )", vspFUNCTION( vsp::EditXSecConvertTo ), vspCALL_CDECL );
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "array<double>@ GetEditXSecUVec( const string& in xsec_id )", vspMETHOD( ScriptMgrSingleton, GetEditXSecUVec ), vspCALL_THISCALL_ASGLOBAL, &ScriptMgr );
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "array<vec3d>@ GetEditXSecCtrlVec( const string& in xsec_id, const bool non_dimensional = true )", vspMETHOD( ScriptMgrSingleton, GetEditXSecCtrlVec ), vspCALL_THISCALL_ASGLOBAL, &ScriptMgr );
    assert( r >= 0 );

    r = se->RegisterGlobalFunction( "void SetEditXSecPnts( const string& in xsec_id, array<double>@ u_vec, array<vec3d>@ control_pts, array<double>@ r_vec )", vspMETHOD( ScriptMgrSingleton, SetEditXSecPnts ), vspCALL_THISCALL_ASGLOBAL, &ScriptMgr );
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "void EditXSecDelPnt( const string& in xsec_id, const int& in indx )", vspFUNCTION( vsp::EditXSecDelPnt ), vspCALL_CDECL );
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "int EditXSecSplit01( const string& in xsec_id, const double& in u )", vspFUNCTION( vsp::EditXSecSplit01 ), vspCALL_CDECL );
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "void MoveEditXSecPnt( const string& in xsec_id, const int& in indx, const vec3d& in new_pnt )", vspFUNCTION( vsp::MoveEditXSecPnt ), vspCALL_CDECL );
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "void ConvertXSecToEdit( const string& in geom_id, const int& in indx = 0 )", vspFUNCTION( vsp::ConvertXSecToEdit ), vspCALL_CDECL );
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "array<bool>@ GetEditXSecFixedUVec( const string& in xsec_id )", vspMETHOD( ScriptMgrSingleton, GetEditXSecFixedUVec ), vspCALL_THISCALL_ASGLOBAL, &ScriptMgr );
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "void SetEditXSecFixedUVec( const string& in xsec_id, array<bool>@ fixed_u_vec )", vspMETHOD( ScriptMgrSingleton, SetEditXSecFixedUVec ), vspCALL_THISCALL_ASGLOBAL, &ScriptMgr );
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "void ReparameterizeEditXSec( const string& in xsec_id )", vspFUNCTION( vsp::ReparameterizeEditXSec ), vspCALL_CDECL );
    assert( r >= 0 );

    //==== BOR Functions ====//

    r = se->RegisterGlobalFunction( "void ChangeBORXSecShape( const string & in geom_id, int type )", vspFUNCTION( vsp::ChangeBORXSecShape ), vspCALL_CDECL );
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "int GetBORXSecShape( const string & in geom_id )", vspFUNCTION( vsp::GetBORXSecShape ), vspCALL_CDECL );
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "array<vec3d>@ ReadBORFileXSec(const string& in bor_id, const string& in file_name )", vspMETHOD( ScriptMgrSingleton, ReadBORFileXSec ), vspCALL_THISCALL_ASGLOBAL, &ScriptMgr );
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "void SetBORXSecPnts( const string& in bor_id, array<vec3d>@ pnt_arr )", vspMETHOD( ScriptMgrSingleton, SetBORXSecPnts ), vspCALL_THISCALL_ASGLOBAL, &ScriptMgr );
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "vec3d ComputeBORXSecPnt( const string& in bor_id, double fract )", vspFUNCTION( vsp::ComputeBORXSecPnt ), vspCALL_CDECL );
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "vec3d ComputeBORXSecTan( const string& in bor_id, double fract )", vspFUNCTION( vsp::ComputeBORXSecTan ), vspCALL_CDECL );
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "void ReadBORFileAirfoil( const string& in bor_id, const string& in file_name )", vspFUNCTION( vsp::ReadBORFileAirfoil ), vspCALL_CDECL );
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "void SetBORAirfoilUpperPnts( const string& in bor_id, array<vec3d>@ up_pnt_vec )", vspMETHOD( ScriptMgrSingleton, SetBORAirfoilUpperPnts ), vspCALL_THISCALL_ASGLOBAL, &ScriptMgr );
    assert( r >= 0 );



    r = se->RegisterGlobalFunction( "void SetBORAirfoilLowerPnts( const string& in bor_id, array<vec3d>@ low_pnt_vec )", vspMETHOD( ScriptMgrSingleton, SetBORAirfoilLowerPnts ), vspCALL_THISCALL_ASGLOBAL, &ScriptMgr );
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "void SetBORAirfoilPnts( const string& in bor_id, array<vec3d>@ up_pnt_vec, array<vec3d>@ low_pnt_vec )", vspMETHOD( ScriptMgrSingleton, SetBORAirfoilPnts ), vspCALL_THISCALL_ASGLOBAL, &ScriptMgr );
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "array<vec3d>@ GetBORAirfoilUpperPnts(const string& in bor_id )", vspMETHOD( ScriptMgrSingleton, GetBORAirfoilUpperPnts ), vspCALL_THISCALL_ASGLOBAL, &ScriptMgr );
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "array<vec3d>@ GetBORAirfoilLowerPnts(const string& in bor_id )", vspMETHOD( ScriptMgrSingleton, GetBORAirfoilLowerPnts ), vspCALL_THISCALL_ASGLOBAL, &ScriptMgr );
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "array<double>@ GetBORUpperCSTCoefs( const string & in bor_id )", vspMETHOD( ScriptMgrSingleton, GetBORUpperCSTCoefs ), vspCALL_THISCALL_ASGLOBAL, &ScriptMgr );
    assert( r >= 0 ); // TODO: Example


    r = se->RegisterGlobalFunction( "array<double>@ GetBORLowerCSTCoefs( const string & in bor_id )", vspMETHOD( ScriptMgrSingleton, GetBORLowerCSTCoefs ), vspCALL_THISCALL_ASGLOBAL, &ScriptMgr );
    assert( r >= 0 );  // TODO: Example


    r = se->RegisterGlobalFunction( "int GetBORUpperCSTDegree( const string& in bor_id )", vspFUNCTION( vsp::GetBORUpperCSTDegree ), vspCALL_CDECL );
    assert( r >= 0 );  // TODO: Example


    r = se->RegisterGlobalFunction( "int GetBORLowerCSTDegree( const string& in bor_id )", vspFUNCTION( vsp::GetBORLowerCSTDegree ), vspCALL_CDECL );
    assert( r >= 0 );  // TODO: Example


    r = se->RegisterGlobalFunction( "void SetBORUpperCST( const string& in bor_id, int deg, array<double>@ coeff_arr )", vspMETHOD( ScriptMgrSingleton, SetBORUpperCST ), vspCALL_THISCALL_ASGLOBAL, &ScriptMgr );
    assert( r >= 0 );  // TODO: Example


    r = se->RegisterGlobalFunction( "void SetBORLowerCST( const string& in bor_id, int deg, array<double>@ coeff_arr )", vspMETHOD( ScriptMgrSingleton, SetBORLowerCST ), vspCALL_THISCALL_ASGLOBAL, &ScriptMgr );
    assert( r >= 0 );  // TODO: Example


    r = se->RegisterGlobalFunction( "void PromoteBORCSTUpper( const string& in bor_id )", vspFUNCTION( vsp::PromoteBORCSTUpper ), vspCALL_CDECL );
    assert( r >= 0 );  // TODO: Example


    r = se->RegisterGlobalFunction( "void PromoteBORCSTLower( const string& in bor_id )", vspFUNCTION( vsp::PromoteBORCSTLower ), vspCALL_CDECL );
    assert( r >= 0 );  // TODO: Example


    r = se->RegisterGlobalFunction( "void DemoteBORCSTUpper( const string& in bor_id )", vspFUNCTION( vsp::DemoteBORCSTUpper ), vspCALL_CDECL );
    assert( r >= 0 );  // TODO: Example


    r = se->RegisterGlobalFunction( "void DemoteBORCSTLower( const string& in bor_id )", vspFUNCTION( vsp::DemoteBORCSTLower ), vspCALL_CDECL );
    assert( r >= 0 );  // TODO: Example


    r = se->RegisterGlobalFunction( "void FitBORAfCST( const string& in bor_id, int deg )", vspFUNCTION( vsp::FitBORAfCST ), vspCALL_CDECL );
    assert( r >= 0 );  // TODO: Example

    //==== Sets Functions ====//

    r = se->RegisterGlobalFunction( "int GetNumSets()", vspFUNCTION( vsp::GetNumSets ), vspCALL_CDECL );
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "void SetSetName( int index, const string& in name )", vspFUNCTION( vsp::SetSetName ), vspCALL_CDECL );
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "string GetSetName( int index )", vspFUNCTION( vsp::GetSetName ), vspCALL_CDECL );
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "array<string>@ GetGeomSetAtIndex( int index )", vspMETHOD( ScriptMgrSingleton, GetGeomSetAtIndex ), vspCALL_THISCALL_ASGLOBAL, &ScriptMgr );
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "array<string>@ GetGeomSet( const string & in name )", vspMETHOD( ScriptMgrSingleton, GetGeomSet ), vspCALL_THISCALL_ASGLOBAL, &ScriptMgr );
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "int GetSetIndex( const string & in name )", vspFUNCTION( vsp::GetSetIndex ), vspCALL_CDECL );
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "bool GetSetFlag( const string & in geom_id, int set_index )", vspFUNCTION( vsp::GetSetFlag ), vspCALL_CDECL );
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "void SetSetFlag( const string & in geom_id, int set_index, bool flag )", vspFUNCTION( vsp::SetSetFlag ), vspCALL_CDECL );
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "void CopyPasteSet(  int copyIndex, int pasteIndex  )", vspFUNCTION( vsp::CopyPasteSet ), vspCALL_CDECL );
    assert( r >= 0 );

    //=== Group Modifications ===//

    r = se->RegisterGlobalFunction( "void ScaleSet( int set_index, double scale )", vspFUNCTION( vsp::ScaleSet ), vspCALL_CDECL );
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "void RotateSet( int set_index, double x_rot_deg, double y_rot_deg, double z_rot_deg )", vspFUNCTION( vsp::RotateSet ), vspCALL_CDECL );
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "void TranslateSet( int set_index, const vec3d & in translation_vec )", vspFUNCTION( vsp::TranslateSet ), vspCALL_CDECL );
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "void TransformSet( int set_index, const vec3d & in translation_vec, double x_rot_deg, double y_rot_deg, double z_rot_deg, double scale, bool scale_translations_flag )", vspFUNCTION( vsp::TransformSet ), vspCALL_CDECL );
    assert( r >= 0 );

    //==== Parm Functions ====//

    r = se->RegisterGlobalFunction( "bool ValidParm( const string & in id )", vspFUNCTION( vsp::ValidParm ), vspCALL_CDECL );
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "double SetParmVal(const string & in parm_id, double val )",
                                    vspFUNCTIONPR( vsp::SetParmVal, ( const string &, double val ), double ), vspCALL_CDECL );
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "double SetParmValLimits(const string & in parm_id, double val, double lower_limit, double upper_limit )",
                                    vspFUNCTION( vsp::SetParmValLimits ), vspCALL_CDECL );
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "double SetParmValUpdate(const string & in parm_id, double val )",
                                    vspFUNCTIONPR( vsp::SetParmValUpdate, ( const string &, double val ), double ), vspCALL_CDECL );
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "double SetParmVal(const string & in container_id, const string & in name, const string & in group, double val )",
                                    vspFUNCTIONPR( vsp::SetParmVal, ( const string &, const string &, const string &, double val ), double ), vspCALL_CDECL );
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "double SetParmValUpdate(const string & in container_id, const string & in parm_name, const string & in parm_group_name, double val )",
                                    vspFUNCTIONPR( vsp::SetParmValUpdate, ( const string &, const string &, const string &, double val ), double ), vspCALL_CDECL );
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "double GetParmVal(const string & in parm_id )", vspFUNCTIONPR( vsp::GetParmVal, ( const string & ), double ), vspCALL_CDECL );
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "double GetParmVal(const string & in container_id, const string & in name, const string & in group )",
                                    vspFUNCTIONPR( vsp::GetParmVal, ( const string &, const string &, const string & ), double ), vspCALL_CDECL );
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "int GetIntParmVal(const string & in parm_id )", vspFUNCTION( vsp::GetIntParmVal ), vspCALL_CDECL );
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "bool GetBoolParmVal(const string & in parm_id )", vspFUNCTION( vsp::GetBoolParmVal ), vspCALL_CDECL );
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "void SetParmUpperLimit( const string & in parm_id, double val )", vspFUNCTION( vsp::SetParmUpperLimit ), vspCALL_CDECL );
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "double GetParmUpperLimit( const string & in parm_id )", vspFUNCTION( vsp::GetParmUpperLimit ), vspCALL_CDECL );
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "void SetParmLowerLimit( const string & in parm_id, double val )", vspFUNCTION( vsp::SetParmLowerLimit ), vspCALL_CDECL );
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "double GetParmLowerLimit( const string & in parm_id )", vspFUNCTION( vsp::GetParmLowerLimit ), vspCALL_CDECL );
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "int GetParmType( const string & in parm_id )", vspFUNCTION( vsp::GetParmType ), vspCALL_CDECL );
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "string GetParmName( const string & in parm_id )", vspFUNCTION( vsp::GetParmName ), vspCALL_CDECL );
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "string GetParmGroupName( const string & in parm_id )", vspFUNCTION( vsp::GetParmGroupName ), vspCALL_CDECL );
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "string GetParmDisplayGroupName( const string & in parm_id )", vspFUNCTION( vsp::GetParmDisplayGroupName ), vspCALL_CDECL );
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "string GetParmContainer( const string & in parm_id )", vspFUNCTION( vsp::GetParmContainer ), vspCALL_CDECL );
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "void SetParmDescript( const string & in parm_id, const string & in desc )", vspFUNCTION( vsp::SetParmDescript ), vspCALL_CDECL );
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "string GetParmDescript( const string & in parm_id )", vspFUNCTION( vsp::GetParmDescript ), vspCALL_CDECL );
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "string FindParm( const string & in parm_container_id, const string & in parm_name, const string & in group_name )", vspFUNCTION( vsp::FindParm ), vspCALL_CDECL );
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "string GetParm(const string & in container_id, const string & in name, const string & in group )", vspFUNCTION( vsp::GetParm ), vspCALL_CDECL );
    assert( r >= 0 );

    //=== Parm Container Functions ===//

    r = se->RegisterGlobalFunction( "array<string>@ FindContainers()", vspMETHOD( ScriptMgrSingleton, FindContainers ), vspCALL_THISCALL_ASGLOBAL, &ScriptMgr );
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "array<string>@ FindContainersWithName( const string & in name )", vspMETHOD( ScriptMgrSingleton, FindContainersWithName ), vspCALL_THISCALL_ASGLOBAL, &ScriptMgr);
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "string FindContainer( const string & in name, int index )", vspFUNCTION( vsp::FindContainer ), vspCALL_CDECL);
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "string GetContainerName( const string & in parm_container_id )", vspFUNCTION( vsp::GetContainerName ), vspCALL_CDECL);
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "array<string>@ FindContainerGroupNames( const string & in parm_container_id )", vspMETHOD( ScriptMgrSingleton, FindContainerGroupNames ), vspCALL_THISCALL_ASGLOBAL, &ScriptMgr );
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "array<string>@ FindContainerParmIDs( const string & in parm_container_id )", vspMETHOD( ScriptMgrSingleton, FindContainerParmIDs ), vspCALL_THISCALL_ASGLOBAL, &ScriptMgr );
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "string GetVehicleID()", vspFUNCTION( vsp::GetVehicleID ), vspCALL_CDECL);
    assert( r >= 0 );

    //=== Register User Parm Functions ====//

    r = se->RegisterGlobalFunction( "int GetNumUserParms()", vspFUNCTION( vsp::GetNumUserParms ), vspCALL_CDECL);
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "int GetNumPredefinedUserParms()", vspFUNCTION( vsp::GetNumPredefinedUserParms ), vspCALL_CDECL);
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "array<string>@ GetAllUserParms()", vspMETHOD( ScriptMgrSingleton, GetAllUserParms ), vspCALL_THISCALL_ASGLOBAL, &ScriptMgr);
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "string GetUserParmContainer()", vspFUNCTION( vsp::GetUserParmContainer ), vspCALL_CDECL);
    assert( r >= 0 );



    r = se->RegisterGlobalFunction( "string AddUserParm( int type, const string & in name, const string & in group )",
                                    vspFUNCTION( vsp::AddUserParm ), vspCALL_CDECL );
    assert( r );


    r = se->RegisterGlobalFunction( "void DeleteUserParm( const string & in parm_id)", vspFUNCTION( vsp::DeleteUserParm ), vspCALL_CDECL);
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "void DeleteAllUserParm()", vspFUNCTION( vsp::DeleteAllUserParm ), vspCALL_CDECL);
    assert( r >= 0 );

    //=== Register Snap To Functions ====//

    r = se->RegisterGlobalFunction( "double ComputeMinClearanceDistance( const string & in geom_id, int set )", vspFUNCTION( vsp::ComputeMinClearanceDistance ), vspCALL_CDECL );
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "double SnapParm( const string & in parm_id, double target_min_dist, bool inc_flag, int set )", vspFUNCTION( vsp::SnapParm ), vspCALL_CDECL );
    assert( r >= 0 );

    //=== Register Var Preset Functions ====//

    r = se->RegisterGlobalFunction( "void AddVarPresetGroup( const string & in group_name )", vspFUNCTION( vsp::AddVarPresetGroup ), vspCALL_CDECL);
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "void AddVarPresetSetting( const string & in setting_name )", vspFUNCTION( vsp::AddVarPresetSetting ), vspCALL_CDECL);
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "void AddVarPresetParm( const string & in parm_ID )", vspFUNCTIONPR( vsp::AddVarPresetParm, ( const string & ), void ), vspCALL_CDECL);
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "void AddVarPresetParm( const string & in parm_ID, const string & in group_name )", vspFUNCTIONPR( vsp::AddVarPresetParm, ( const string &, const string & ), void ), vspCALL_CDECL);
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "void EditVarPresetParm( const string & in parm_ID, double parm_val )", vspFUNCTIONPR( vsp::EditVarPresetParm, ( const string &, double ), void ), vspCALL_CDECL);
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "void EditVarPresetParm( const string & in parm_ID, double parm_val, const string & in group_name, const string & in setting_name )", vspFUNCTIONPR( vsp::EditVarPresetParm,
        ( const string &, double, const string &, const string & ), void ), vspCALL_CDECL );
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "void DeleteVarPresetParm( const string & in parm_ID )", vspFUNCTIONPR( vsp::DeleteVarPresetParm, ( const string & ), void ), vspCALL_CDECL);
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "void DeleteVarPresetParm( const string & in parm_ID, const string & in group_name )", vspFUNCTIONPR( vsp::DeleteVarPresetParm, ( const string &, const string & ), void ), vspCALL_CDECL);
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "void SwitchVarPreset( const string & in group_name, const string & in setting_name )", vspFUNCTION( vsp::SwitchVarPreset ), vspCALL_CDECL);
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "bool DeleteVarPresetSet( const string & in group_name, const string & in setting_name )", vspFUNCTION( vsp::DeleteVarPresetSet ), vspCALL_CDECL);
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "string GetCurrentGroupName()", vspFUNCTION( vsp::GetCurrentGroupName ), vspCALL_CDECL);
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "string GetCurrentSettingName()", vspFUNCTION( vsp::GetCurrentSettingName ), vspCALL_CDECL);
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "array<string>@ GetVarPresetGroupNames()", vspMETHOD( ScriptMgrSingleton, GetVarPresetGroupNames ), vspCALL_THISCALL_ASGLOBAL, &ScriptMgr);
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "array<string>@ GetVarPresetSettingNamesWName( const string & in group_name )", vspMETHOD( ScriptMgrSingleton, GetVarPresetSettingNamesWName ), vspCALL_THISCALL_ASGLOBAL, &ScriptMgr);
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "array<string>@ GetVarPresetSettingNamesWIndex( int group_index )", vspMETHOD( ScriptMgrSingleton, GetVarPresetSettingNamesWIndex ), vspCALL_THISCALL_ASGLOBAL, &ScriptMgr);
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "array<double>@ GetVarPresetParmVals()", vspMETHOD( ScriptMgrSingleton, GetVarPresetParmVals ), vspCALL_THISCALL_ASGLOBAL, &ScriptMgr);
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "array<double>@ GetVarPresetParmValsWNames( const string & in group_name, const string & in setting_name )", vspMETHOD( ScriptMgrSingleton, GetVarPresetParmValsWNames ), vspCALL_THISCALL_ASGLOBAL, &ScriptMgr);
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "array<string>@ GetVarPresetParmIDs()", vspMETHOD( ScriptMgrSingleton, GetVarPresetParmIDs ), vspCALL_THISCALL_ASGLOBAL, &ScriptMgr);
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "array<string>@ GetVarPresetParmIDsWName( const string & in group_name )", vspMETHOD( ScriptMgrSingleton, GetVarPresetParmIDsWName ), vspCALL_THISCALL_ASGLOBAL, &ScriptMgr);
    assert( r >= 0 );

    //=== Register PCurve Functions ====//

    r = se->RegisterGlobalFunction( "void SetPCurve( const string& in geom_id, const int & in pcurveid, array<double>@ tvec, array<double>@ valvec, const int & in newtype )", vspMETHOD( ScriptMgrSingleton, SetPCurve ), vspCALL_THISCALL_ASGLOBAL, &ScriptMgr );
    assert( r >= 0 ); // TODO: Example


    r = se->RegisterGlobalFunction( "void PCurveConvertTo( const string & in geom_id, const int & in pcurveid, const int & in newtype )", vspFUNCTION( vsp::PCurveConvertTo ), vspCALL_CDECL );
    assert( r >= 0 ); // TODO: Example


    r = se->RegisterGlobalFunction( "int PCurveGetType( const string & in geom_id, const int & in pcurveid )", vspFUNCTION( vsp::PCurveGetType ), vspCALL_CDECL );
    assert( r >= 0 ); // TODO: Example


    r = se->RegisterGlobalFunction( "array<double>@ PCurveGetTVec( const string & in geom_id, const int & in pcurveid )", vspMETHOD( ScriptMgrSingleton, PCurveGetTVec ), vspCALL_THISCALL_ASGLOBAL, &ScriptMgr );
    assert( r >= 0 ); // TODO: Example


    r = se->RegisterGlobalFunction( "array<double>@ PCurveGetValVec( const string & in geom_id, const int & in pcurveid )", vspMETHOD( ScriptMgrSingleton, PCurveGetValVec ), vspCALL_THISCALL_ASGLOBAL, &ScriptMgr );
    assert( r >= 0 ); // TODO: Example


    r = se->RegisterGlobalFunction( "void PCurveDeletePt( const string & in geom_id, const int & in pcurveid, const int & in indx )", vspFUNCTION( vsp::PCurveDeletePt ), vspCALL_CDECL);
    assert( r >= 0 ); // TODO: Example


    r = se->RegisterGlobalFunction( "int PCurveSplit( const string & in geom_id, const int & in pcurveid, const double & in tsplit )", vspFUNCTION( vsp::PCurveSplit ), vspCALL_CDECL);
    assert( r >= 0 ); // TODO: Example


    r = se->RegisterGlobalFunction( "void ApproximateAllPropellerPCurves( const string & in geom_id )", vspFUNCTION( vsp::ApproximateAllPropellerPCurves ), vspCALL_CDECL);
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "void ResetPropellerThicknessCurve( const string & in geom_id )", vspFUNCTION( vsp::ResetPropellerThicknessCurve ), vspCALL_CDECL);
    assert( r >= 0 );

    //=== Register ParasiteDragTool Functions ====//

    r = se->RegisterGlobalFunction( "void AddExcrescence(const string & in excresName, const int & in excresType, const double & in excresVal)", vspFUNCTION( vsp::AddExcrescence ), vspCALL_CDECL );
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "void DeleteExcrescence(const int & in excresName)", vspFUNCTION( vsp::DeleteExcrescence ), vspCALL_CDECL );
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "void UpdateParasiteDrag()", vspFUNCTION( vsp::UpdateParasiteDrag ), vspCALL_CDECL );
    assert( r >= 0 ); // TODO: Example


    r = se->RegisterGlobalFunction( "void WriteAtmosphereCSVFile( const string & in file_name, const int & in atmos_type )", vspFUNCTION( vsp::WriteAtmosphereCSVFile ), vspCALL_CDECL );
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "void CalcAtmosphere( const double & in alt, const double & in delta_temp, const int & in atmos_type, double & out temp, double & out pres, double & out pres_ratio, double & out rho_ratio )", vspFUNCTION( vsp::CalcAtmosphere ), vspCALL_CDECL );
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "void WriteBodyFFCSVFile( const string & in file_name )", vspFUNCTION( vsp::WriteBodyFFCSVFile ), vspCALL_CDECL );
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "void WriteWingFFCSVFile( const string & in file_name )", vspFUNCTION( vsp::WriteWingFFCSVFile ), vspCALL_CDECL );
    assert( r >= 0 );

     // TODO: Improve description
    r = se->RegisterGlobalFunction( "void WriteCfEqnCSVFile( const string & in file_name )", vspFUNCTION( vsp::WriteCfEqnCSVFile ), vspCALL_CDECL );
    assert( r >= 0 );

     // TODO: Improve description
    r = se->RegisterGlobalFunction( "void WritePartialCfMethodCSVFile( const string & in file_name )", vspFUNCTION( vsp::WritePartialCfMethodCSVFile ), vspCALL_CDECL );
    assert( r >= 0 );

    //=== Register Surface Query Functions ===//

    r = se->RegisterGlobalFunction( "vec3d CompPnt01( const string & in geom_id, const int & in surf_indx, const double & in u, const double & in w )", vspFUNCTION(vsp::CompPnt01), vspCALL_CDECL );
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "vec3d CompNorm01( const string & in geom_id, const int & in surf_indx, const double & in u, const double & in w )", vspFUNCTION(vsp::CompNorm01), vspCALL_CDECL );
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "vec3d CompTanU01( const string & in geom_id, const int & in surf_indx, const double & in u, const double & in w )", vspFUNCTION(vsp::CompTanU01), vspCALL_CDECL );
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "vec3d CompTanW01( const string & in geom_id, const int & in surf_indx, const double & in u, const double & in w )", vspFUNCTION(vsp::CompTanW01), vspCALL_CDECL );
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "void CompCurvature01( const string & in geom_id, const int & in surf_indx, const double & in u, const double & in w, double & out k1, double & out k2, double & out ka, double & out kg )", vspFUNCTION(vsp::CompCurvature01), vspCALL_CDECL );
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "double ProjPnt01( const string & in geom_id, const int & in surf_indx, const vec3d & in pt, double & out u, double & out w )", vspFUNCTION(vsp::ProjPnt01), vspCALL_CDECL );
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "double ProjPnt01I( const string & in geom_id, const vec3d & in pt, int & out surf_indx, double & out u, double & out w )", vspFUNCTION(vsp::ProjPnt01I), vspCALL_CDECL );
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "double ProjPnt01Guess( const string & in geom_id, const int & in surf_indx, const vec3d & in pt, const double & in u0, const double & in w0, double & out u, double & out w )", vspFUNCTION(vsp::ProjPnt01Guess), vspCALL_CDECL );
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "double AxisProjPnt01( const string & in geom_id, const int & in surf_indx, const int & in iaxis, const vec3d & in pt, double & out u_out, double & out w_out, vec3d & out p_out )", vspFUNCTION(vsp::AxisProjPnt01), vspCALL_CDECL );
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "double AxisProjPnt01I( const string & in geom_id, const int & in iaxis, const vec3d & in pt, int & out surf_indx_out, double & out u_out, double & out w_out, vec3d & out p_out )", vspFUNCTION(vsp::AxisProjPnt01I), vspCALL_CDECL );
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "double AxisProjPnt01Guess( const string & in geom_id, const int & in surf_indx, const int & in iaxis, const vec3d & in pt, const double & in u0, const double & in w0, double & out u_out, double & out w_out, vec3d & out p_out )", vspFUNCTION(vsp::AxisProjPnt01Guess), vspCALL_CDECL );
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "bool InsideSurf( const string & in geom_id, const int & in surf_indx, const vec3d & in pt )", vspFUNCTION(vsp::InsideSurf), vspCALL_CDECL );
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "double FindRST( const string & in geom_id, const int & in surf_indx, const vec3d & in pt, double & out r, double & out s, double & out t )", vspFUNCTION(vsp::FindRST), vspCALL_CDECL );
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "double FindRSTGuess( const string & in geom_id, const int & in surf_indx, const vec3d & in pt, const double & in r0, const double & in s0, const double & in t0, double & out r, double & out s, double & out t )", vspFUNCTION(vsp::FindRSTGuess), vspCALL_CDECL );
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "vec3d CompPntRST( const string & in geom_id, const int & in surf_indx, const double & in r, const double & in s, const double & in t )", vspFUNCTION(vsp::CompPntRST), vspCALL_CDECL );
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "array<vec3d>@ CompVecPntRST(const string & in geom_id, const int & in surf_indx, array<double>@ rs, array<double>@ ss, array<double>@ ts )", vspMETHOD( ScriptMgrSingleton, CompVecPntRST ), vspCALL_THISCALL_ASGLOBAL, &ScriptMgr );
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "vec3d ConvertRSTtoLMN( const string & in geom_id, const int & in surf_indx, const double & in r, const double & in s, const double & in t, double & out l, double & out m, double & out n )", vspFUNCTION(vsp::ConvertRSTtoLMN), vspCALL_CDECL );
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "vec3d ConvertRtoL( const string & in geom_id, const int & in surf_indx, const double & in r, double & out l )", vspFUNCTION(vsp::ConvertRtoL), vspCALL_CDECL );
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "vec3d ConvertLMNtoRST( const string & in geom_id, const int & in surf_indx, const double & in l, const double & in m, const double & in n, double & out r, double & out s, double & out t )", vspFUNCTION(vsp::ConvertLMNtoRST), vspCALL_CDECL );
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "vec3d ConvertLtoR( const string & in geom_id, const int & in surf_indx, const double & in l, double & out r )", vspFUNCTION(vsp::ConvertLtoR), vspCALL_CDECL );
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "vec3d ConvertUtoEta( const string & in geom_id, const double & in u, double & out eta )", vspFUNCTION(vsp::ConvertUtoEta), vspCALL_CDECL );
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "vec3d ConvertEtatoU( const string & in geom_id, const double & in eta, double & out u )", vspFUNCTION(vsp::ConvertEtatoU), vspCALL_CDECL );
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "void ConvertRSTtoLMNVec(const string & in geom_id, const int & in surf_indx, array<double>@ rs, array<double>@ ss, array<double>@ ts, array<double>@ ls, array<double>@ ms, array<double>@ ns )", vspMETHOD( ScriptMgrSingleton, ConvertRSTtoLMNVec ), vspCALL_THISCALL_ASGLOBAL, &ScriptMgr );
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "void ConvertLMNtoRSTVec(const string & in geom_id, const int & in surf_indx, array<double>@ ls, array<double>@ ms, array<double>@ ns, array<double>@ rs, array<double>@ ss, array<double>@ ts )", vspMETHOD( ScriptMgrSingleton, ConvertLMNtoRSTVec ), vspCALL_THISCALL_ASGLOBAL, &ScriptMgr );
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "void GetUWTess01(const string & in geom_id, int & in surf_indx, array<double>@ us, array<double>@ ws )", vspMETHOD( ScriptMgrSingleton, GetUWTess01 ), vspCALL_THISCALL_ASGLOBAL, &ScriptMgr );
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "array<vec3d>@ CompVecPnt01(const string & in geom_id, const int & in surf_indx, array<double>@ us, array<double>@ ws )", vspMETHOD( ScriptMgrSingleton, CompVecPnt01 ), vspCALL_THISCALL_ASGLOBAL, &ScriptMgr );
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "array<vec3d>@ CompVecNorm01(const string & in geom_id, const int & in surf_indx, array<double>@ us, array<double>@ws )", vspMETHOD( ScriptMgrSingleton, CompVecNorm01 ), vspCALL_THISCALL_ASGLOBAL, &ScriptMgr );
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "void CompVecCurvature01(const string & in geom_id, const int & in surf_indx, array<double>@ us, array<double>@ ws, array<double>@ k1s, array<double>@ k2s, array<double>@ kas, array<double>@ kgs)", vspMETHOD( ScriptMgrSingleton, CompVecCurvature01 ), vspCALL_THISCALL_ASGLOBAL, &ScriptMgr );
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "void ProjVecPnt01(const string & in geom_id, const int & in surf_indx, array<vec3d>@ pts, array<double>@ us, array<double>@ ws, array<double>@ ds )", vspMETHOD( ScriptMgrSingleton, ProjVecPnt01 ), vspCALL_THISCALL_ASGLOBAL, &ScriptMgr );
    assert( r >= 0 );

    r = se->RegisterGlobalFunction( "void ProjVecPnt01Guess(const string & in geom_id, const int & in surf_indx, array<vec3d>@ pts, array<double>@ u0s, array<double>@ w0s, array<double>@ us, array<double>@ ws, array<double>@ ds )", vspMETHOD( ScriptMgrSingleton, ProjVecPnt01Guess ), vspCALL_THISCALL_ASGLOBAL, &ScriptMgr );
    assert( r >= 0 );

    r = se->RegisterGlobalFunction( "void AxisProjVecPnt01(const string & in geom_id, const int & in surf_indx, const int & in iaxis, array<vec3d>@ pts, array<double>@ us, array<double>@ ws, array<vec3d>@ ps_out, array<double>@ ds )", vspMETHOD( ScriptMgrSingleton, AxisProjVecPnt01 ), vspCALL_THISCALL_ASGLOBAL, &ScriptMgr );
    assert( r >= 0 );

    r = se->RegisterGlobalFunction( "void AxisProjVecPnt01Guess(const string & in geom_id, int & in surf_indx, const int & in iaxis, array<vec3d>@ pts, array<double>@ u0s, array<double>@ w0s, array<double>@ us, array<double>@ ws, array<vec3d>@ ps_out, array<double>@ ds )", vspMETHOD( ScriptMgrSingleton, AxisProjVecPnt01Guess ), vspCALL_THISCALL_ASGLOBAL, &ScriptMgr );
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "array<bool>@  VecInsideSurf( const string & in geom_id, const int & in surf_indx, array<vec3d>@ pts )", vspFUNCTION(vsp::VecInsideSurf), vspCALL_CDECL );
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "void FindRSTVec(const string & in geom_id, const int & in surf_indx, array<vec3d>@ pts, array<double>@ rs, array<double>@ ss, array<double>@ ts, array<double>@ ds )", vspMETHOD( ScriptMgrSingleton, FindRSTVec ), vspCALL_THISCALL_ASGLOBAL, &ScriptMgr );
    assert( r >= 0 );

    r = se->RegisterGlobalFunction( "void FindRSTVecGuess(const string & in geom_id, const int & in surf_indx, array<vec3d>@ pts, array<double>@ r0s, array<double>@ s0s, array<double>@ t0s, array<double>@ rs, array<double>@ ss, array<double>@ ts, array<double>@ ds )", vspMETHOD( ScriptMgrSingleton, FindRSTVecGuess ), vspCALL_THISCALL_ASGLOBAL, &ScriptMgr );
    assert( r >= 0 );

    //=== Register Measure Functions ===//

    r = se->RegisterGlobalFunction( "string AddRuler( const string & in startgeomid, int startsurfindx, double startu, double startw, const string & in endgeomid, int endsurfindx, double endu, double endw, const string & in name )", vspFUNCTION( vsp::AddRuler ), vspCALL_CDECL );
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "array<string>@ GetAllRulers()", vspMETHOD( ScriptMgrSingleton, GetAllRulers ), vspCALL_THISCALL_ASGLOBAL, &ScriptMgr);
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "void DelRuler( const string & in id )", vspFUNCTION( vsp::DelRuler ), vspCALL_CDECL);
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "void DeleteAllRulers()", vspFUNCTION( vsp::DeleteAllRulers ), vspCALL_CDECL);
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "string AddProbe( const string & in geomid, int surfindx, double u, double w, const string & in name )", vspFUNCTION( vsp::AddProbe ), vspCALL_CDECL);
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "array<string>@ GetAllProbes()", vspMETHOD( ScriptMgrSingleton, GetAllProbes ), vspCALL_THISCALL_ASGLOBAL, &ScriptMgr);
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "void DelProbe( const string & in id )", vspFUNCTION( vsp::DelProbe ), vspCALL_CDECL);
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "void DeleteAllProbes()", vspFUNCTION( vsp::DeleteAllProbes ), vspCALL_CDECL);
    assert( r >= 0 );

    //=== Register FeaStructure and FEA Mesh Functions ====//

    r = se->RegisterGlobalFunction( "int AddFeaStruct( const string & in geom_id, bool init_skin = true, int surfindex = 0 )", vspFUNCTION( vsp::AddFeaStruct ), vspCALL_CDECL );
    assert( r >= 0 ); // TODO: Force init_skin to true always


    r = se->RegisterGlobalFunction( "void DeleteFeaStruct( const string & in geom_id, int fea_struct_ind )", vspFUNCTION( vsp::DeleteFeaStruct ), vspCALL_CDECL );
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "void SetFeaMeshStructIndex( int struct_index )", vspFUNCTION( vsp::SetFeaMeshStructIndex ), vspCALL_CDECL);
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "string GetFeaStructID( const string & in geom_id, int fea_struct_ind )", vspFUNCTION( vsp::GetFeaStructID ), vspCALL_CDECL );
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "int GetFeaStructIndex( const string & in struct_id )", vspFUNCTION( vsp::GetFeaStructIndex ), vspCALL_CDECL );
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "string GetFeaStructParentGeomID( const string & in struct_id )", vspFUNCTION( vsp::GetFeaStructParentGeomID ), vspCALL_CDECL );
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "string GetFeaStructName( const string & in geom_id, int fea_struct_ind )", vspFUNCTION( vsp::GetFeaStructName ), vspCALL_CDECL );
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "void SetFeaStructName( const string & in geom_id, int fea_struct_ind, const string & in name )", vspFUNCTION( vsp::SetFeaStructName ), vspCALL_CDECL );
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "array<string>@ GetFeaStructIDVec()", vspMETHOD( ScriptMgrSingleton, GetFeaStructIDVec ), vspCALL_THISCALL_ASGLOBAL, &ScriptMgr );
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "void SetFeaPartName( const string & in part_id, const string & in name )", vspFUNCTION( vsp::SetFeaPartName ), vspCALL_CDECL );
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "void SetFeaMeshVal( const string & in geom_id, int fea_struct_ind, int type, double val )", vspFUNCTION( vsp::SetFeaMeshVal ), vspCALL_CDECL );
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "void SetFeaMeshFileName( const string & in geom_id, int fea_struct_ind, int file_type, const string & in file_name )", vspFUNCTION( vsp::SetFeaMeshFileName ), vspCALL_CDECL );
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "void ComputeFeaMesh( const string & in geom_id, int fea_struct_ind, int file_type )", vspFUNCTIONPR( vsp::ComputeFeaMesh, ( const string & in, int, int ), void ), vspCALL_CDECL );
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "void ComputeFeaMesh( const string & in struct_id, int file_type )", vspFUNCTIONPR( vsp::ComputeFeaMesh, ( const string & in, int ), void ), vspCALL_CDECL );
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "string AddFeaPart( const string & in geom_id, int fea_struct_ind, int type )", vspFUNCTION( vsp::AddFeaPart ), vspCALL_CDECL );
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "void DeleteFeaPart( const string & in geom_id, int fea_struct_ind, const string & in part_id )", vspFUNCTION( vsp::DeleteFeaPart ), vspCALL_CDECL );
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "string GetFeaPartID( const string & in fea_struct_id, int fea_part_index )", vspFUNCTION( vsp::GetFeaPartID ), vspCALL_CDECL );
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "string GetFeaPartName( const string & in part_id )", vspFUNCTION( vsp::GetFeaPartName ), vspCALL_CDECL );
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "int GetFeaPartType( const string & in part_id )", vspFUNCTION( vsp::GetFeaPartType ), vspCALL_CDECL );
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "int GetFeaSubSurfIndex( const string & in ss_id )", vspFUNCTION( vsp::GetFeaSubSurfIndex ), vspCALL_CDECL );
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "int NumFeaStructures()", vspFUNCTION( vsp::NumFeaStructures ), vspCALL_CDECL );
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "int NumFeaParts( const string & in fea_struct_id )", vspFUNCTION( vsp::NumFeaParts ), vspCALL_CDECL );
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "int NumFeaSubSurfs( const string & in fea_struct_id )", vspFUNCTION( vsp::NumFeaSubSurfs ), vspCALL_CDECL );
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "array<string>@ GetFeaPartIDVec(const string & in fea_struct_id)", vspMETHOD( ScriptMgrSingleton, GetFeaPartIDVec ), vspCALL_THISCALL_ASGLOBAL, &ScriptMgr );
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "array<string>@ GetFeaSubSurfIDVec(const string & in fea_struct_id)", vspMETHOD( ScriptMgrSingleton, GetFeaSubSurfIDVec ), vspCALL_THISCALL_ASGLOBAL, &ScriptMgr );
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "void SetFeaPartPerpendicularSparID( const string & in part_id, const string & in perpendicular_spar_id )", vspFUNCTION( vsp::SetFeaPartPerpendicularSparID ), vspCALL_CDECL );
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "string GetFeaPartPerpendicularSparID( const string & in part_id )", vspFUNCTION( vsp::GetFeaPartPerpendicularSparID ), vspCALL_CDECL );
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "string AddFeaSubSurf( const string & in geom_id, int fea_struct_ind, int type )", vspFUNCTION( vsp::AddFeaSubSurf ), vspCALL_CDECL );
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "void DeleteFeaSubSurf( const string & in geom_id, int fea_struct_ind, const string & in ss_id )", vspFUNCTION( vsp::DeleteFeaSubSurf ), vspCALL_CDECL );
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "string AddFeaMaterial()", vspFUNCTION( vsp::AddFeaMaterial ), vspCALL_CDECL );
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "string AddFeaProperty( int property_type = 0 )", vspFUNCTION( vsp::AddFeaProperty ), vspCALL_CDECL );
    assert( r >= 0 );
}

void ScriptMgrSingleton::RegisterUtility( asIScriptEngine* se )
{
    int r;
    //==== Register Utility Functions ====//

    r = se->RegisterGlobalFunction( "void Print(const string & in data, bool new_line = true )", vspMETHODPR( ScriptMgrSingleton, Print, (const string &, bool), void ), vspCALL_THISCALL_ASGLOBAL, &ScriptMgr );
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "void Print(const vec3d & in data, bool new_line = true )", vspMETHODPR( ScriptMgrSingleton, Print, (const vec3d &, bool), void ), vspCALL_THISCALL_ASGLOBAL, &ScriptMgr );
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "void Print(double data, bool new_line = true )", vspMETHODPR( ScriptMgrSingleton, Print, (double, bool), void ), vspCALL_THISCALL_ASGLOBAL, &ScriptMgr );
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "void Print(int data, bool new_line = true )", vspMETHODPR( ScriptMgrSingleton, Print, (int, bool), void ), vspCALL_THISCALL_ASGLOBAL, &ScriptMgr );
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "double Min( double x, double y)", vspMETHOD( ScriptMgrSingleton, Min ), vspCALL_THISCALL_ASGLOBAL, &ScriptMgr);
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "double Max( double x, double y)", vspMETHOD( ScriptMgrSingleton, Max ), vspCALL_THISCALL_ASGLOBAL, &ScriptMgr);
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "double Rad2Deg( double r )", vspMETHOD( ScriptMgrSingleton, Rad2Deg ), vspCALL_THISCALL_ASGLOBAL, &ScriptMgr);
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "double Deg2Rad( double d )", vspMETHOD( ScriptMgrSingleton, Deg2Rad ), vspCALL_THISCALL_ASGLOBAL, &ScriptMgr);
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "string GetVSPVersion( )", vspFUNCTION( vsp::GetVSPVersion ), vspCALL_CDECL);
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "int GetVSPVersionMajor( )", vspFUNCTION( vsp::GetVSPVersionMajor ), vspCALL_CDECL);
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "int GetVSPVersionMinor( )", vspFUNCTION( vsp::GetVSPVersionMinor ), vspCALL_CDECL);
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "int GetVSPVersionChange( )", vspFUNCTION( vsp::GetVSPVersionChange ), vspCALL_CDECL);
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "string GetVSPExePath()", vspFUNCTION( vsp::GetVSPExePath ), vspCALL_CDECL);
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "bool SetVSPAEROPath( const string & in path )", vspFUNCTION( vsp::SetVSPAEROPath ), vspCALL_CDECL );
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "string GetVSPAEROPath()", vspFUNCTION( vsp::GetVSPAEROPath ), vspCALL_CDECL );
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "bool CheckForVSPAERO( const string & in path )", vspFUNCTION( vsp::CheckForVSPAERO ), vspCALL_CDECL );
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "void VSPCheckSetup()", vspFUNCTION( vsp::VSPCheckSetup ), vspCALL_CDECL);
    assert( r >= 0 );


    r = se->RegisterGlobalFunction( "void VSPRenew()", vspFUNCTION( vsp::VSPRenew ), vspCALL_CDECL);
    assert( r >= 0 );

    //====  Register Proxy Utility Functions ====//

    r = se->RegisterGlobalFunction( "array<vec3d>@ GetProxyVec3dArray()", vspMETHOD( ScriptMgrSingleton, GetProxyVec3dArray ), vspCALL_THISCALL_ASGLOBAL, &ScriptMgr );
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
        sarr->SetValue( i, &darr );
    }
    return sarr;
}

template < class T >
void ScriptMgrSingleton::FillArray( vector < T > & in, CScriptArray* out )
{
    out->Resize( in.size() );
    for ( int i = 0 ; i < ( int )in.size() ; i++ )
    {
        out->SetValue( i, &in[i] );
    }
}

template < class T >
void ScriptMgrSingleton::FillArray( CScriptArray* in, vector < T > & out )
{
    out.resize( in->GetSize() );
    for ( int i = 0 ; i < ( int )in->GetSize() ; i++ )
    {
        out[i] = * ( T* ) ( in->At( i ) );
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
    FillArray( selected, int_vec );

    vsp::AddSelectedToCSGroup( int_vec, CSGroupIndex );
}

void ScriptMgrSingleton::RemoveSelectedFromCSGroup( CScriptArray* selected, int CSGroupIndex )
{
    vector < int > int_vec;
    FillArray( selected, int_vec );

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
    FillArray( del_arr, del_vec );

    vsp::DeleteGeomVec( del_vec );
}

void ScriptMgrSingleton::SetXSecPnts( const string& xsec_id, CScriptArray* pnt_arr )
{
    vector< vec3d > pnt_vec;
    FillArray( pnt_arr, pnt_vec );

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
    FillArray( up_pnt_arr, up_pnt_vec );

    vector< vec3d > low_pnt_vec;
    FillArray( low_pnt_arr, low_pnt_vec );

    vsp::SetAirfoilPnts( xsec_id, up_pnt_vec, low_pnt_vec );
}

void ScriptMgrSingleton::SetBORXSecPnts( const string& bor_id, CScriptArray* pnt_arr )
{
    vector< vec3d > pnt_vec;
    FillArray( pnt_arr, pnt_vec );

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
    FillArray( up_pnt_arr, up_pnt_vec );

    vector< vec3d > low_pnt_vec;
    FillArray( low_pnt_arr, low_pnt_vec );

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
    FillArray( xyzdata, xyz_vec );

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
    FillArray( surf_pnt_arr, surf_pnt_vec );

    m_ProxyDoubleArray = vsp::GetEllipsoidCpDist( surf_pnt_vec, abc_rad, V_inf );

    return GetProxyDoubleArray();
}

void ScriptMgrSingleton::SetUpperCST( const string& xsec_id, int deg, CScriptArray* coefs_arr )
{
    vector < double > coefs_vec;
    FillArray( coefs_arr, coefs_vec );

    vsp::SetUpperCST( xsec_id, deg, coefs_vec );
}

void ScriptMgrSingleton::SetLowerCST( const string& xsec_id, int deg, CScriptArray* coefs_arr )
{
    vector < double > coefs_vec;
    FillArray( coefs_arr, coefs_vec );

    vsp::SetLowerCST( xsec_id, deg, coefs_vec );
}

void ScriptMgrSingleton::SetBORUpperCST( const string& bor_id, int deg, CScriptArray* coefs_arr )
{
    vector < double > coefs_vec;
    FillArray( coefs_arr, coefs_vec );

    vsp::SetBORUpperCST( bor_id, deg, coefs_vec );
}

void ScriptMgrSingleton::SetBORLowerCST( const string& bor_id, int deg, CScriptArray* coefs_arr )
{
    vector < double > coefs_vec;
    FillArray( coefs_arr, coefs_vec );

    vsp::SetBORLowerCST( bor_id, deg, coefs_vec );
}

//==== Edit Curve XSec Functions ====//
CScriptArray* ScriptMgrSingleton::GetEditXSecUVec( const std::string& xsec_id )
{
    m_ProxyDoubleArray = vsp::GetEditXSecUVec( xsec_id );

    return GetProxyDoubleArray();
}

CScriptArray* ScriptMgrSingleton::GetEditXSecCtrlVec( const std::string & xsec_id, const bool non_dimensional )
{
    m_ProxyVec3dArray = vsp::GetEditXSecCtrlVec( xsec_id, non_dimensional );

    return GetProxyVec3dArray();
}

void ScriptMgrSingleton::SetEditXSecPnts( const string & xsec_id, CScriptArray* u_vec, CScriptArray* control_pts, CScriptArray* r_vec )
{
    vector < vec3d > control_pnt_vec;
    FillArray( control_pts, control_pnt_vec );

    vector < double > new_u_vec;
    FillArray( u_vec, new_u_vec );

    vector < double > new_r_vec;
    FillArray( r_vec, new_r_vec );

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
    FillArray( fixed_u_vec, new_fixed_u_vec );

    vsp::SetEditXSecFixedUVec( xsec_id, new_fixed_u_vec );
}

//==== Variable Preset Functions ====//
CScriptArray* ScriptMgrSingleton::GetVarPresetGroupNames()
{
    m_ProxyStringArray = vsp::GetVarPresetGroupNames();
    return GetProxyStringArray();
}

CScriptArray* ScriptMgrSingleton::GetVarPresetSettingNamesWName( string group_name )
{
    m_ProxyStringArray = vsp::GetVarPresetSettingNamesWName( group_name );
    return GetProxyStringArray();
}

CScriptArray* ScriptMgrSingleton::GetVarPresetSettingNamesWIndex( int group_index )
{
    m_ProxyStringArray = vsp::GetVarPresetSettingNamesWIndex( group_index );
    return GetProxyStringArray();
}

CScriptArray* ScriptMgrSingleton::GetVarPresetParmVals()
{
    m_ProxyDoubleArray = vsp::GetVarPresetParmVals();
    return GetProxyDoubleArray();
}

CScriptArray* ScriptMgrSingleton::GetVarPresetParmValsWNames( string group_name, string setting_name )
{
    m_ProxyDoubleArray = vsp::GetVarPresetParmValsWNames( group_name, setting_name );
    return GetProxyDoubleArray();
}

CScriptArray* ScriptMgrSingleton::GetVarPresetParmIDs()
{
    m_ProxyStringArray = vsp::GetVarPresetParmIDs();
    return GetProxyStringArray();
}

CScriptArray* ScriptMgrSingleton::GetVarPresetParmIDsWName( string group_name )
{
    m_ProxyStringArray = vsp::GetVarPresetParmIDsWName( group_name );
    return GetProxyStringArray();
}

void ScriptMgrSingleton::AddVarPresetGroup( const string &group_name )
{
    vsp::AddVarPresetGroup( group_name );
}

void ScriptMgrSingleton::AddVarPresetSetting( const string &setting_name )
{
    vsp::AddVarPresetSetting( setting_name );
}

void ScriptMgrSingleton::AddVarPresetParm( const string &parm_ID )
{
    vsp::AddVarPresetParm( parm_ID );
}

void ScriptMgrSingleton::AddVarPresetParm( const string &parm_ID, string group_name )
{
    vsp::AddVarPresetParm( parm_ID, group_name );
}

void ScriptMgrSingleton::EditVarPresetParm( const string &parm_ID, double parm_val )
{
    vsp::EditVarPresetParm( parm_ID, parm_val );
}

void ScriptMgrSingleton::EditVarPresetParm( const string &parm_ID, double parm_val, string group_name, string setting_name )
{
    vsp::EditVarPresetParm( parm_ID, parm_val, group_name, setting_name );
}

void ScriptMgrSingleton::DeleteVarPresetParm( const string &parm_ID )
{
    vsp::DeleteVarPresetParm( parm_ID );
}

void ScriptMgrSingleton::DeleteVarPresetParm( const string &parm_ID, string group_name )
{
    vsp::DeleteVarPresetParm( parm_ID, group_name );
}

void ScriptMgrSingleton::SwitchVarPreset( string group_name, string setting_name )
{
    vsp::SwitchVarPreset( group_name, setting_name );
}

void ScriptMgrSingleton::DeleteVarPresetSet( string group_name, string setting_name )
{
    vsp::DeleteVarPresetSet( group_name, setting_name );
}

//==== PCurve Functions ====//
void ScriptMgrSingleton::SetPCurve( const string& geom_id, const int & pcurveid, CScriptArray* t_arr, CScriptArray* val_arr, const int & newtype )
{
    vector < double > t_vec;
    FillArray( t_arr, t_vec );

    vector < double > val_vec;
    FillArray( val_arr, val_vec );

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


void ScriptMgrSingleton::SetIntAnalysisInput( const string& analysis, const string & name, CScriptArray* indata, int index )
{
    vector < int > indata_vec;
    FillArray( indata, indata_vec );

    vsp::SetIntAnalysisInput( analysis, name, indata_vec, index );
}

void ScriptMgrSingleton::SetDoubleAnalysisInput( const string& analysis, const string & name, CScriptArray* indata, int index )
{
    vector < double > indata_vec;
    FillArray( indata, indata_vec );

    vsp::SetDoubleAnalysisInput( analysis, name, indata_vec, index );
}

void ScriptMgrSingleton::SetStringAnalysisInput( const string& analysis, const string & name, CScriptArray* indata, int index )
{
    vector < string > indata_vec;
    FillArray( indata, indata_vec );

    vsp::SetStringAnalysisInput( analysis, name, indata_vec, index );
}

void ScriptMgrSingleton::SetVec3dAnalysisInput( const string& analysis, const string & name, CScriptArray* indata, int index )
{
    vector < vec3d > indata_vec;
    FillArray( indata, indata_vec );

    vsp::SetVec3dAnalysisInput( analysis, name, indata_vec, index );
}

CScriptArray* ScriptMgrSingleton::CompVecPnt01(const string &geom_id, const int &surf_indx, CScriptArray* us, CScriptArray* ws)
{
    vector < double > in_us;
    FillArray( us, in_us );

    vector < double > in_ws;
    FillArray( ws, in_ws );

    m_ProxyVec3dArray = vsp::CompVecPnt01( geom_id, surf_indx, in_us, in_ws );
    return GetProxyVec3dArray();
}

CScriptArray* ScriptMgrSingleton::CompVecPntRST(const string &geom_id, const int &surf_indx, CScriptArray* rs, CScriptArray* ss, CScriptArray* ts)
{
    vector < double > in_rs;
    FillArray( rs, in_rs );

    vector < double > in_ss;
    FillArray( ss, in_ss );

    vector < double > in_ts;
    FillArray( ts, in_ts );

    m_ProxyVec3dArray = vsp::CompVecPntRST( geom_id, surf_indx, in_rs, in_ss, in_ts );
    return GetProxyVec3dArray();
}

CScriptArray* ScriptMgrSingleton::CompVecNorm01(const string &geom_id, const int &surf_indx, CScriptArray* us, CScriptArray* ws)
{
    vector < double > in_us;
    FillArray( us, in_us );

    vector < double > in_ws;
    FillArray( ws, in_ws );

    m_ProxyVec3dArray = vsp::CompVecNorm01( geom_id, surf_indx, in_us, in_ws );
    return GetProxyVec3dArray();
}

void ScriptMgrSingleton::CompVecCurvature01(const string &geom_id, const int &surf_indx, CScriptArray* us, CScriptArray* ws, CScriptArray* k1s, CScriptArray* k2s, CScriptArray* kas, CScriptArray* kgs)
{
    vector < double > in_us;
    FillArray( us, in_us );

    vector < double > in_ws;
    FillArray( ws, in_ws );

    vector < double > out_k1s;
    vector < double > out_k2s;
    vector < double > out_kas;
    vector < double > out_kgs;

    vsp::CompVecCurvature01( geom_id, surf_indx, in_us, in_ws, out_k1s, out_k2s, out_kas, out_kgs );

    FillArray( out_k1s, k1s );
    FillArray( out_k2s, k2s );
    FillArray( out_kas, kas );
    FillArray( out_kgs, kgs );
}

void ScriptMgrSingleton::ProjVecPnt01(const string &geom_id, const int &surf_indx, CScriptArray* pts, CScriptArray* us, CScriptArray* ws, CScriptArray* ds )
{
    vector < vec3d > in_pts;
    FillArray( pts, in_pts );

    vector < double > out_us;
    vector < double > out_ws;
    vector < double > out_ds;

    vsp::ProjVecPnt01( geom_id, surf_indx, in_pts, out_us, out_ws, out_ds );

    FillArray( out_us, us );
    FillArray( out_ws, ws );
    FillArray( out_ds, ds );
}

void ScriptMgrSingleton::ProjVecPnt01Guess(const string &geom_id, const int &surf_indx, CScriptArray* pts, CScriptArray* u0s, CScriptArray* w0s, CScriptArray* us, CScriptArray* ws, CScriptArray* ds )
{
    vector < vec3d > in_pts;
    FillArray( pts, in_pts );

    vector < double > in_u0s;
    FillArray( u0s, in_u0s );

    vector < double > in_w0s;
    FillArray( w0s, in_w0s );

    vector < double > out_us;
    vector < double > out_ws;
    vector < double > out_ds;

    vsp::ProjVecPnt01Guess( geom_id, surf_indx, in_pts, in_u0s, in_w0s, out_us, out_ws, out_ds );

    FillArray( out_us, us );
    FillArray( out_ws, ws );
    FillArray( out_ds, ds );
}

void ScriptMgrSingleton::AxisProjVecPnt01(const string &geom_id, const int &surf_indx, const int &iaxis, CScriptArray* pts, CScriptArray* us, CScriptArray* ws, CScriptArray* ps_out, CScriptArray* ds )
{
    vector < vec3d > in_pts;
    FillArray( pts, in_pts );

    vector < double > out_us;
    vector < double > out_ws;
    vector < double > out_ds;
    vector < vec3d > out_pts;

    vsp::AxisProjVecPnt01( geom_id, surf_indx, iaxis, in_pts, out_us, out_ws, out_pts, out_ds );

    FillArray( out_us, us );
    FillArray( out_ws, ws );
    FillArray( out_ds, ds );
    FillArray( out_pts, ps_out );
}

void ScriptMgrSingleton::AxisProjVecPnt01Guess(const string &geom_id, const int &surf_indx, const int &iaxis, CScriptArray* pts, CScriptArray* u0s, CScriptArray* w0s, CScriptArray* us, CScriptArray* ws, CScriptArray* ps_out, CScriptArray* ds )
{
    vector < vec3d > in_pts;
    FillArray( pts, in_pts );

    vector < double > in_u0s;
    FillArray( u0s, in_u0s );

    vector < double > in_w0s;
    FillArray( w0s, in_w0s );

    vector < double > out_us;
    vector < double > out_ws;
    vector < double > out_ds;
    vector < vec3d > out_pts;

    vsp::AxisProjVecPnt01Guess( geom_id, surf_indx, iaxis, in_pts, in_u0s, in_w0s, out_us, out_ws, out_pts,out_ds );

    FillArray( out_us, us );
    FillArray( out_ws, ws );
    FillArray( out_ds, ds );
    FillArray( out_pts, ps_out );
}

void ScriptMgrSingleton::FindRSTVec(const string &geom_id, const int &surf_indx, CScriptArray* pts, CScriptArray* rs, CScriptArray* ss, CScriptArray* ts, CScriptArray* ds )
{
    vector < vec3d > in_pts;
    FillArray( pts, in_pts );

    vector < double > out_rs;
    vector < double > out_ss;
    vector < double > out_ts;
    vector < double > out_ds;

    vsp::FindRSTVec( geom_id, surf_indx, in_pts, out_rs, out_ss, out_ts, out_ds );

    FillArray( out_rs, rs );
    FillArray( out_ss, ss );
    FillArray( out_ts, ts );
    FillArray( out_ds, ds );
}

void ScriptMgrSingleton::FindRSTVecGuess(const string &geom_id, const int &surf_indx, CScriptArray* pts, CScriptArray* r0s, CScriptArray* s0s, CScriptArray* t0s, CScriptArray* rs, CScriptArray* ss, CScriptArray* ts, CScriptArray* ds )
{
    vector < vec3d > in_pts;
    FillArray( pts, in_pts );

    vector < double > in_r0s;
    vector < double > in_s0s;
    vector < double > in_t0s;

    FillArray( r0s, in_r0s );
    FillArray( s0s, in_s0s );
    FillArray( t0s, in_t0s );

    vector < double > out_rs;
    vector < double > out_ss;
    vector < double > out_ts;
    vector < double > out_ds;

    vsp::FindRSTVecGuess(geom_id, surf_indx, in_pts, in_r0s, in_s0s, in_t0s, out_rs, out_ss, out_ts, out_ds );

    FillArray( out_rs, rs );
    FillArray( out_ss, ss );
    FillArray( out_ts, ts );
    FillArray( out_ds, ds );
}

void ScriptMgrSingleton::ConvertRSTtoLMNVec(const string &geom_id, const int &surf_indx, CScriptArray* rs, CScriptArray* ss, CScriptArray* ts, CScriptArray* ls, CScriptArray* ms, CScriptArray* ns )
{
    vector < double > in_rs;
    vector < double > in_ss;
    vector < double > in_ts;

    FillArray( rs, in_rs );
    FillArray( ss, in_ss );
    FillArray( ts, in_ts );

    vector < double > out_ls;
    vector < double > out_ms;
    vector < double > out_ns;

    vsp::ConvertRSTtoLMNVec( geom_id, surf_indx, in_rs, in_ss, in_ts, out_ls, out_ms, out_ns );

    FillArray( out_ls, ls );
    FillArray( out_ms, ms );
    FillArray( out_ns, ns );
}

void ScriptMgrSingleton::ConvertLMNtoRSTVec(const string &geom_id, const int &surf_indx, CScriptArray* ls, CScriptArray* ms, CScriptArray* ns, CScriptArray* rs, CScriptArray* ss, CScriptArray* ts )
{
    vector < double > in_ls;
    vector < double > in_ms;
    vector < double > in_ns;

    FillArray( ls, in_ls );
    FillArray( ms, in_ms );
    FillArray( ns, in_ns );

    vector < double > out_rs;
    vector < double > out_ss;
    vector < double > out_ts;

    vsp::ConvertLMNtoRSTVec( geom_id, surf_indx, in_ls, in_ms, in_ns, out_rs, out_ss, out_ts );

    FillArray( out_rs, rs );
    FillArray( out_ss, ss );
    FillArray( out_ts, ts );
}

void ScriptMgrSingleton::GetUWTess01(const string &geom_id, int &surf_indx, CScriptArray* us, CScriptArray* ws )
{
    vector < double > out_us;
    vector < double > out_ws;

    vsp::GetUWTess01( geom_id, surf_indx, out_us, out_ws );

    FillArray( out_us, us );
    FillArray( out_ws, ws );
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
