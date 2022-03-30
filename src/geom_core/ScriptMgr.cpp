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
// FIXME: asDocInfo group can't contain any underscores!
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
    sprintf( str, "%s (%d, %d) : %s : %s\n", msg->section, msg->row, msg->col, type, msg->message );
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
    int r = se->SetMessageCallback( asFUNCTION( MessageCallback ), 0, asCALL_CDECL );
    assert( r >= 0 );

    //==== Register Addons ====//
    RegisterStdString( m_ScriptEngine );

    string comment_str = R"(
  //!  AngelScript ScriptExtension for representing the C++ std::string
  /*! <a href="https://www.angelcode.com/angelscript/sdk/docs/manual/doc_script_stdlib_string.html">Angelscript string Documentation </a>
  */)";

    se->AddSkipComment( "string", comment_str.c_str() );

    RegisterScriptArray( m_ScriptEngine, true );

    comment_str = R"(
  //!  AngelScript ScriptExtension for representing the C++ std::vector
  /*! <a href="https://www.angelcode.com/angelscript/sdk/docs/manual/doc_datatypes_arrays.html">Angelscript array Documentation </a>
  */)";

    se->AddSkipComment( "array", comment_str.c_str() );

    RegisterScriptFile( m_ScriptEngine );

    comment_str = R"(
  //!  AngelScript ScriptExtension for representing the C++ std::FILE
  /*! <a href="https://www.angelcode.com/angelscript/sdk/docs/manual/doc_script_stdlib_file.html">Angelscript file Documentation </a>
  */)";

    se->AddSkipComment( "file", comment_str.c_str() );

    RegisterStdStringUtils( m_ScriptEngine );

    comment_str = R"(
  //!  AngelScript ScriptExtension for representing the C++ std::string
  /*! <a href="https://www.angelcode.com/angelscript/sdk/docs/manual/doc_datatypes_arrays.html">Angelscript string Documentation </a>
  */)";

    se->AddSkipComment( "string_util", comment_str.c_str() );  // FIXME

    RegisterScriptMath( m_ScriptEngine );

    comment_str = R"(
  //!  AngelScript ScriptExtension for representing the C++ std::math collection of functions
  /*! <a href="https://www.angelcode.com/angelscript/sdk/docs/manual/doc_addon_math.html">Angelscript array Documentation </a>
  */)";

    se->AddSkipComment( "math", comment_str.c_str() ); // FIXME

    RegisterScriptAny( m_ScriptEngine );

    comment_str = R"(
  //!  AngelScript ScriptExtension for representing generic container that can hold any value
  /*! <a href="https://www.angelcode.com/angelscript/sdk/docs/manual/doc_addon_any.html">Angelscript any Documentation </a>
  */)";

    se->AddSkipComment( "any", comment_str.c_str() );

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
        return 1;
    }

    asDWORD ret = ctx->GetReturnDWord();
    int32_t rval = ret;

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

  //  doc_struct.comment = R"(
  ////! ATTACH_TRANS_TYPE enum
  ///*! Enum for identifying the atmospheric model */)";

    asDocInfo doc_struct;
    doc_struct.comment = "/*! Enum that indicates if positions are relative or absolute. */";
    string group = "Enumerations";
    doc_struct.group = group.c_str();

    string group_description = R"(
    \brief All API enumerations are defined in this group. \n\n
    \ref index "Click here to return to the main page" )";
    se->AddGroup( group.c_str(), group.c_str(), group_description.c_str() );

    int r = se->RegisterEnum( "ABS_REL_FLAG", doc_struct );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "ABS_REL_FLAG", "ABS", ABS, "/*!< Absolute position */" );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "ABS_REL_FLAG", "REL", REL, "/*!< Relative position */" );
    assert( r >= 0 );

    doc_struct.comment = "/*! Enum used to identify the format of exported airfoil files. */";

    r = se->RegisterEnum( "AIRFOIL_EXPORT_TYPE", doc_struct );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "AIRFOIL_EXPORT_TYPE", "SELIG_AF_EXPORT", SELIG_AF_EXPORT, "/*!< Selig airfoil file format */" );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "AIRFOIL_EXPORT_TYPE", "BEZIER_AF_EXPORT", BEZIER_AF_EXPORT, "/*!< Bezier airfoil file format */" );
    assert( r >= 0 );

    doc_struct.comment = "/*! Enum for specifying angular units. */";

    r = se->RegisterEnum( "ANG_UNITS", doc_struct );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "ANG_UNITS", "ANG_RAD", ANG_RAD, "/*!< Radians */" );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "ANG_UNITS", "ANG_DEG", ANG_DEG, "/*!< Degrees */" );
    assert( r >= 0 );

    doc_struct.comment = "/*! Enum that identifies the atmospheric model used in the Parasite Drag tool. */";

    r = se->RegisterEnum( "ATMOS_TYPE", doc_struct );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "ATMOS_TYPE", "ATMOS_TYPE_US_STANDARD_1976", ATMOS_TYPE_US_STANDARD_1976, "/*!< US Standard Atmosphere 1976 (default) */" );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "ATMOS_TYPE", "ATMOS_TYPE_HERRINGTON_1966", ATMOS_TYPE_HERRINGTON_1966, "/*!< USAF 1966 */" );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "ATMOS_TYPE", "ATMOS_TYPE_MANUAL_P_R", ATMOS_TYPE_MANUAL_P_R, "/*!< Manual: pressure and density control */" );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "ATMOS_TYPE", "ATMOS_TYPE_MANUAL_P_T", ATMOS_TYPE_MANUAL_P_T, "/*!< Manual: pressure and temperature control */" );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "ATMOS_TYPE", "ATMOS_TYPE_MANUAL_R_T", ATMOS_TYPE_MANUAL_R_T, "/*!< Manual: density and temperature control */" );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "ATMOS_TYPE", "ATMOS_TYPE_MANUAL_RE_L", ATMOS_TYPE_MANUAL_RE_L, "/*!< Manual: Reynolds number and length control */" );
    assert( r >= 0 );

    doc_struct.comment = "/*! Enum that identifies the parent to child relative translation coordinate system. */";

    r = se->RegisterEnum( "ATTACH_TRANS_TYPE", doc_struct );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "ATTACH_TRANS_TYPE", "ATTACH_TRANS_NONE", ATTACH_TRANS_NONE, "/*!< No parent attachment for translations */" );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "ATTACH_TRANS_TYPE", "ATTACH_TRANS_COMP", ATTACH_TRANS_COMP, "/*!< Translation relative to parent body axes */" );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "ATTACH_TRANS_TYPE", "ATTACH_TRANS_UV", ATTACH_TRANS_UV, "/*!< Translation relative to parent surface coordinate frame */" );
    assert( r >= 0 );

    doc_struct.comment = "/*! Enum that determines parent to child relative rotation axes. */";

    r = se->RegisterEnum( "ATTACH_ROT_TYPE", doc_struct );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "ATTACH_ROT_TYPE", "ATTACH_ROT_NONE", ATTACH_ROT_NONE, "/*!< No parent attachment for rotations */" );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "ATTACH_ROT_TYPE", "ATTACH_ROT_COMP", ATTACH_ROT_COMP, "/*!< Rotation relative to parent body axes */" );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "ATTACH_ROT_TYPE", "ATTACH_ROT_UV", ATTACH_ROT_UV, "/*!< Rotation relative to parent surface coordinate frame */" );
    assert( r >= 0 );

    doc_struct.comment = "/*! Enum for Body of Revolution mode control. */";

    r = se->RegisterEnum( "BOR_MODE", doc_struct );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "BOR_MODE", "BOR_FLOWTHROUGH", BOR_FLOWTHROUGH, "/*!< Flowthrough mode (default) */" );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "BOR_MODE", "BOR_UPPER", BOR_UPPER, "/*!< Upper surface mode */" );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "BOR_MODE", "BOR_LOWER", BOR_LOWER, "/*!< Lower surface mode */" );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "BOR_MODE", "BOR_NUM_MODES", BOR_NUM_MODES, "/*!< Number of Body of Revolution modes */" );
    assert( r >= 0 );

    doc_struct.comment = "/*! Enum used to select between maximum camber or ideal lift coefficient inputs. */";

    r = se->RegisterEnum( "CAMBER_INPUT_FLAG", doc_struct );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "CAMBER_INPUT_FLAG", "MAX_CAMB", MAX_CAMB, "/*!< Input maximum camber, calculate ideal lift coefficient */" );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "CAMBER_INPUT_FLAG", "DESIGN_CL", DESIGN_CL, "/*!< Input ideal lift coefficient, calculate maximum camber */" );
    assert( r >= 0 );

    doc_struct.comment = "/*! Enum that identifies the end cap types for a geometry (i.e. wing root and tip). */";

    r = se->RegisterEnum( "CAP_TYPE", doc_struct );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "CAP_TYPE", "NO_END_CAP", vsp::NO_END_CAP, "/*!< No end cap */" );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "CAP_TYPE", "FLAT_END_CAP", vsp::FLAT_END_CAP, "/*!< Flat end cap */" );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "CAP_TYPE", "ROUND_END_CAP", vsp::ROUND_END_CAP, "/*!< Round end cap */" );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "CAP_TYPE", "EDGE_END_CAP", vsp::EDGE_END_CAP, "/*!< Edge end cap */" );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "CAP_TYPE", "SHARP_END_CAP", vsp::SHARP_END_CAP, "/*!< Sharp end cap */" );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "CAP_TYPE", "NUM_END_CAP_OPTIONS", vsp::NUM_END_CAP_OPTIONS, "/*!< Number of end cap options */" );
    assert( r >= 0 );

    doc_struct.comment = "/*! Enum used to identify each CFD mesh control option (also applicable to FEA Mesh). \\sa SetCFDMeshVal(), SetFEAMeshVal() */";

    r = se->RegisterEnum( "CFD_CONTROL_TYPE", doc_struct );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "CFD_CONTROL_TYPE", "CFD_MIN_EDGE_LEN", CFD_MIN_EDGE_LEN, "/*!< Minimum mesh edge length */" );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "CFD_CONTROL_TYPE", "CFD_MAX_EDGE_LEN", CFD_MAX_EDGE_LEN, "/*!< Maximum mesh edge length */" );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "CFD_CONTROL_TYPE", "CFD_MAX_GAP", CFD_MAX_GAP, "/*!< Maximum mesh edge gap */" );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "CFD_CONTROL_TYPE", "CFD_NUM_CIRCLE_SEGS", CFD_NUM_CIRCLE_SEGS, "/*!< Number of edge segments to resolve circle */" );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "CFD_CONTROL_TYPE", "CFD_GROWTH_RATIO", CFD_GROWTH_RATIO, "/*!< Maximum allowed edge growth ratio */" );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "CFD_CONTROL_TYPE", "CFD_LIMIT_GROWTH_FLAG", CFD_LIMIT_GROWTH_FLAG, "/*!< Rigorous 3D growth limiting flag */" );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "CFD_CONTROL_TYPE", "CFD_INTERSECT_SUBSURFACE_FLAG", CFD_INTERSECT_SUBSURFACE_FLAG, "/*!< Flag to intersect sub-surfaces */" );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "CFD_CONTROL_TYPE", "CFD_HALF_MESH_FLAG", CFD_HALF_MESH_FLAG, "/*!< Flag to generate a half mesh */" );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "CFD_CONTROL_TYPE", "CFD_FAR_FIELD_FLAG", CFD_FAR_FIELD_FLAG, "/*!< Flag to generate a far field mesh */" );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "CFD_CONTROL_TYPE", "CFD_FAR_MAX_EDGE_LEN", CFD_FAR_MAX_EDGE_LEN, "/*!< Maximum far field mesh edge length */" );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "CFD_CONTROL_TYPE", "CFD_FAR_MAX_GAP", CFD_FAR_MAX_GAP, "/*!< Maximum far field mesh edge gap */" );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "CFD_CONTROL_TYPE", "CFD_FAR_NUM_CIRCLE_SEGS", CFD_FAR_NUM_CIRCLE_SEGS, "/*!< Number of far field edge segments to resolve circle */" );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "CFD_CONTROL_TYPE", "CFD_FAR_SIZE_ABS_FLAG", CFD_FAR_SIZE_ABS_FLAG, "/*!< Relative or absolute size flag */" );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "CFD_CONTROL_TYPE", "CFD_FAR_LENGTH", CFD_FAR_LENGTH, "/*!< Far field length */" );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "CFD_CONTROL_TYPE", "CFD_FAR_WIDTH", CFD_FAR_WIDTH, "/*!< Far field width */" );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "CFD_CONTROL_TYPE", "CFD_FAR_HEIGHT", CFD_FAR_HEIGHT, "/*!< Far field height */" );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "CFD_CONTROL_TYPE", "CFD_FAR_X_SCALE", CFD_FAR_X_SCALE, "/*!<  Far field X scale */" );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "CFD_CONTROL_TYPE", "CFD_FAR_Y_SCALE", CFD_FAR_Y_SCALE, "/*!<  Far field Y scale */" );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "CFD_CONTROL_TYPE", "CFD_FAR_Z_SCALE", CFD_FAR_Z_SCALE, "/*!<  Far field Z scale */" );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "CFD_CONTROL_TYPE", "CFD_FAR_LOC_MAN_FLAG", CFD_FAR_LOC_MAN_FLAG, "/*!< Far field location flag: centered or manual */" );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "CFD_CONTROL_TYPE", "CFD_FAR_LOC_X", CFD_FAR_LOC_X, "/*!< Far field X location */" );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "CFD_CONTROL_TYPE", "CFD_FAR_LOC_Y", CFD_FAR_LOC_Y, "/*!< Far field Y location */" );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "CFD_CONTROL_TYPE", "CFD_FAR_LOC_Z", CFD_FAR_LOC_Z, "/*!< Far field Z location */" );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "CFD_CONTROL_TYPE", "CFD_SRF_XYZ_FLAG", CFD_SRF_XYZ_FLAG, "/*!< Flag to include X,Y,Z intersection curves in export files */" );
    assert( r >= 0 );

    doc_struct.comment = "/*! Enum used to describe various CFD Mesh export file options. \\sa SetComputationFileName(), ComputeCFDMesh() */";

    r = se->RegisterEnum( "CFD_MESH_EXPORT_TYPE", doc_struct );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "CFD_MESH_EXPORT_TYPE", "CFD_STL_FILE_NAME", CFD_STL_FILE_NAME,  "/*!< STL export type */" );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "CFD_MESH_EXPORT_TYPE", "CFD_POLY_FILE_NAME", CFD_POLY_FILE_NAME, "/*!< POLY export type */" );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "CFD_MESH_EXPORT_TYPE", "CFD_TRI_FILE_NAME", CFD_TRI_FILE_NAME, "/*!< TRI export type */" );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "CFD_MESH_EXPORT_TYPE", "CFD_OBJ_FILE_NAME", CFD_OBJ_FILE_NAME, "/*!< OBJ export type */" );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "CFD_MESH_EXPORT_TYPE", "CFD_DAT_FILE_NAME", CFD_DAT_FILE_NAME, "/*!< DAT export type */" );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "CFD_MESH_EXPORT_TYPE", "CFD_KEY_FILE_NAME", CFD_KEY_FILE_NAME, "/*!< KEY export type */" );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "CFD_MESH_EXPORT_TYPE", "CFD_GMSH_FILE_NAME", CFD_GMSH_FILE_NAME, "/*!< GMSH export type */" );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "CFD_MESH_EXPORT_TYPE", "CFD_SRF_FILE_NAME", CFD_SRF_FILE_NAME, "/*!< SRF export type */" );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "CFD_MESH_EXPORT_TYPE", "CFD_TKEY_FILE_NAME", CFD_TKEY_FILE_NAME, "/*!< TKEY export type */" );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "CFD_MESH_EXPORT_TYPE", "CFD_FACET_FILE_NAME", CFD_FACET_FILE_NAME, "/*!< FACET export type */" );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "CFD_MESH_EXPORT_TYPE", "CFD_CURV_FILE_NAME", CFD_CURV_FILE_NAME, "/*!< CURV export type */" );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "CFD_MESH_EXPORT_TYPE", "CFD_PLOT3D_FILE_NAME", CFD_PLOT3D_FILE_NAME, "/*!< PLOT3D export type */" );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "CFD_MESH_EXPORT_TYPE", "CFD_VSPGEOM_FILE_NAME", CFD_VSPGEOM_FILE_NAME, "/*!< VSPGEOM export type */" );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "CFD_MESH_EXPORT_TYPE", "CFD_NUM_FILE_NAMES", CFD_NUM_FILE_NAMES, "/*!< Number of CFD Mesh export file types */" );
    assert( r >= 0 );

    doc_struct.comment = "/*! Enum that indicates the CFD Mesh source type. \\sa AddCFDSource() */";

    r = se->RegisterEnum( "CFD_MESH_SOURCE_TYPE", doc_struct );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "CFD_MESH_SOURCE_TYPE", "POINT_SOURCE", POINT_SOURCE, "/*!< Point source */" );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "CFD_MESH_SOURCE_TYPE", "LINE_SOURCE", LINE_SOURCE, "/*!< Line source */" );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "CFD_MESH_SOURCE_TYPE", "BOX_SOURCE", BOX_SOURCE, "/*!< Box source */" );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "CFD_MESH_SOURCE_TYPE", "ULINE_SOURCE", ULINE_SOURCE, "/*!< Constant U Line source */" );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "CFD_MESH_SOURCE_TYPE", "WLINE_SOURCE", WLINE_SOURCE, "/*!< Constant W Line source */" );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "CFD_MESH_SOURCE_TYPE", "NUM_SOURCE_TYPES", NUM_SOURCE_TYPES, "/*!< Number of CFD Mesh source types */" );
    assert( r >= 0 );

    doc_struct.comment = "/*! Enum that identifies the Parasite Drag Tool laminar friction coefficient equation choice. */";

    r = se->RegisterEnum( "CF_LAM_EQN", doc_struct );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "CF_LAM_EQN", "CF_LAM_BLASIUS", CF_LAM_BLASIUS, "/*!< Blasius laminar Cf equation */" );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "CF_LAM_EQN", "CF_LAM_BLASIUS_W_HEAT", CF_LAM_BLASIUS_W_HEAT, "/*!< Blasius laminar Cf equation with heat (NOT IMPLEMENTED) */" ); // TODO: Remove or implement
    assert( r >= 0 );

    doc_struct.comment = "/*! Enum that identifies the Parasite Drag Tool turbulent friction coefficient equation choice. */";

    r = se->RegisterEnum( "CF_TURB_EQN", doc_struct );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "CF_TURB_EQN", "CF_TURB_EXPLICIT_FIT_SPALDING", CF_TURB_EXPLICIT_FIT_SPALDING, "/*!< Explicit Fit of Spalding turbulent Cf equation */" );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "CF_TURB_EQN", "CF_TURB_EXPLICIT_FIT_SPALDING_CHI", CF_TURB_EXPLICIT_FIT_SPALDING_CHI, "/*!< Explicit Fit of Spalding and Chi turbulent Cf equation */" );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "CF_TURB_EQN", "CF_TURB_EXPLICIT_FIT_SCHOENHERR", CF_TURB_EXPLICIT_FIT_SCHOENHERR, "/*!< Explicit Fit of Schoenherr turbulent Cf equation */" );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "CF_TURB_EQN", "DO_NOT_USE_CF_TURB_IMPLICIT_KARMAN", DO_NOT_USE_CF_TURB_IMPLICIT_KARMAN, "/*!< Implicit Karman turbulent Cf equation (DO NOT USE) */" );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "CF_TURB_EQN", "CF_TURB_IMPLICIT_SCHOENHERR", CF_TURB_IMPLICIT_SCHOENHERR, "/*!< Implicit Schoenherr turbulent Cf equation */" );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "CF_TURB_EQN", "CF_TURB_IMPLICIT_KARMAN_SCHOENHERR", CF_TURB_IMPLICIT_KARMAN_SCHOENHERR, "/*!< Implicit Karman-Schoenherr turbulent Cf equation */" );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "CF_TURB_EQN", "CF_TURB_POWER_LAW_BLASIUS", CF_TURB_POWER_LAW_BLASIUS, "/*!< Power Law Blasius turbulent Cf equation */" );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "CF_TURB_EQN", "CF_TURB_POWER_LAW_PRANDTL_LOW_RE", CF_TURB_POWER_LAW_PRANDTL_LOW_RE, "/*!<Power Law Prandtl Low Re turbulent Cf equation */" );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "CF_TURB_EQN", "CF_TURB_POWER_LAW_PRANDTL_MEDIUM_RE", CF_TURB_POWER_LAW_PRANDTL_MEDIUM_RE, "/*!< Power Law Prandtl Medium Re turbulent Cf equation */" );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "CF_TURB_EQN", "CF_TURB_POWER_LAW_PRANDTL_HIGH_RE", CF_TURB_POWER_LAW_PRANDTL_HIGH_RE, "/*!< Power Law Prandtl High Re turbulent Cf equation */" );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "CF_TURB_EQN", "CF_TURB_SCHLICHTING_COMPRESSIBLE", CF_TURB_SCHLICHTING_COMPRESSIBLE, "/*!< Schlichting Compressible turbulent Cf equation */" );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "CF_TURB_EQN", "DO_NOT_USE_CF_TURB_SCHLICHTING_INCOMPRESSIBLE", DO_NOT_USE_CF_TURB_SCHLICHTING_INCOMPRESSIBLE, "/*!< Schlichting Incompressible turbulent Cf equation (DO NOT USE) */" );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "CF_TURB_EQN", "DO_NOT_USE_CF_TURB_SCHLICHTING_PRANDTL", DO_NOT_USE_CF_TURB_SCHLICHTING_PRANDTL, "/*!< Schlichting-Prandtl turbulent Cf equation (DO NOT USE) */" );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "CF_TURB_EQN", "DO_NOT_USE_CF_TURB_SCHULTZ_GRUNOW_HIGH_RE", DO_NOT_USE_CF_TURB_SCHULTZ_GRUNOW_HIGH_RE, "/*!< Schultz-Grunow High Re turbulent Cf equation (DO NOT USE) */" );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "CF_TURB_EQN", "CF_TURB_SCHULTZ_GRUNOW_SCHOENHERR", CF_TURB_SCHULTZ_GRUNOW_SCHOENHERR, "/*!< Schultz-Grunow Estimate of Schoenherr turbulent Cf equation. */" );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "CF_TURB_EQN", "DO_NOT_USE_CF_TURB_WHITE_CHRISTOPH_COMPRESSIBLE", DO_NOT_USE_CF_TURB_WHITE_CHRISTOPH_COMPRESSIBLE, "/*!< White-Christoph Compressible turbulent Cf equation (DO NOT USE) */" );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "CF_TURB_EQN", "CF_TURB_ROUGHNESS_SCHLICHTING_AVG", CF_TURB_ROUGHNESS_SCHLICHTING_AVG, "/*!< Roughness Schlichting Avg turbulent Cf equation. */" );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "CF_TURB_EQN", "DO_NOT_USE_CF_TURB_ROUGHNESS_SCHLICHTING_LOCAL", DO_NOT_USE_CF_TURB_ROUGHNESS_SCHLICHTING_LOCAL, "/*!< Roughness Schlichting Local turbulent Cf equation (DO NOT USE) */" );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "CF_TURB_EQN", "DO_NOT_USE_CF_TURB_ROUGHNESS_WHITE", DO_NOT_USE_CF_TURB_ROUGHNESS_WHITE, "/*!< Roughness White turbulent Cf equation (DO NOT USE) */" );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "CF_TURB_EQN", "CF_TURB_ROUGHNESS_SCHLICHTING_AVG_FLOW_CORRECTION", CF_TURB_ROUGHNESS_SCHLICHTING_AVG_FLOW_CORRECTION, "/*!< Roughness Schlichting Avg Compressible turbulent Cf equation. */" );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "CF_TURB_EQN", "CF_TURB_HEATTRANSFER_WHITE_CHRISTOPH", CF_TURB_HEATTRANSFER_WHITE_CHRISTOPH, "/*!< Heat Transfer White-Christoph turbulent Cf equation. */" );
    assert( r >= 0 );

    doc_struct.comment = "/*! Enum for Chevron curve modification types. */";

    r = se->RegisterEnum( "CHEVRON_TYPE", doc_struct );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "CHEVRON_TYPE", "CHEVRON_NONE", CHEVRON_NONE, "/*!< No chevron. */" );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "CHEVRON_TYPE", "CHEVRON_PARTIAL", CHEVRON_PARTIAL, "/*!< One or more chevrons of limited extent. */" );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "CHEVRON_TYPE", "CHEVRON_FULL", CHEVRON_FULL, "/*!< Full period of chevrons. */" );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "CHEVRON_TYPE", "CHEVRON_NUM_TYPES", CHEVRON_NUM_TYPES, "/*!< Number of chevron types. */" );
    assert( r >= 0 );

    doc_struct.comment = "/*! Enum for Chevron W parameter modes. */";

    r = se->RegisterEnum( "CHEVRON_W01_MODES", doc_struct );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "CHEVRON_W01_MODES", "CHEVRON_W01_SE", CHEVRON_W01_SE, "/*!< Specify chevron W start and end. */" );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "CHEVRON_W01_MODES", "CHEVRON_W01_CW", CHEVRON_W01_CW, "/*!< Specify chevron W center and width. */" );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "CHEVRON_W01_MODES", "CHEVRON_W01_NUM_MODES", CHEVRON_W01_NUM_MODES, "/*!< Number of chevron W parameter mode types. */" );
    assert( r >= 0 );

    doc_struct.comment = "/*! Enum for Snap To collision error types. */";

    r = se->RegisterEnum( "COLLISION_ERRORS", doc_struct );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "COLLISION_ERRORS", "COLLISION_OK", COLLISION_OK, "/*!< No Error. */" );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "COLLISION_ERRORS", "COLLISION_INTERSECT_NO_SOLUTION", COLLISION_INTERSECT_NO_SOLUTION, "/*!< Touching, no solution */" );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "COLLISION_ERRORS", "COLLISION_CLEAR_NO_SOLUTION", COLLISION_CLEAR_NO_SOLUTION, "/*!< Not touching, no solution */" );
    assert( r >= 0 );

    doc_struct.comment = "/*! Enum used to identify various export file types. */";

    r = se->RegisterEnum( "COMPUTATION_FILE_TYPE", doc_struct );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "COMPUTATION_FILE_TYPE", "NO_FILE_TYPE", NO_FILE_TYPE, "/*!< No export file type */" );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "COMPUTATION_FILE_TYPE", "COMP_GEOM_TXT_TYPE", COMP_GEOM_TXT_TYPE, "/*!< Comp Geom TXT file type */" );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "COMPUTATION_FILE_TYPE", "COMP_GEOM_CSV_TYPE", COMP_GEOM_CSV_TYPE, "/*!< Comp Geom CSV file type */" );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "COMPUTATION_FILE_TYPE", "SLICE_TXT_TYPE", SLICE_TXT_TYPE, "/*!< Planar Slice TXT file type */" );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "COMPUTATION_FILE_TYPE", "MASS_PROP_TXT_TYPE", MASS_PROP_TXT_TYPE, "/*!< Mass Properties TXT file type */" );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "COMPUTATION_FILE_TYPE", "DEGEN_GEOM_CSV_TYPE", DEGEN_GEOM_CSV_TYPE, "/*!< Degen Geom CSV file type */" );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "COMPUTATION_FILE_TYPE", "DEGEN_GEOM_M_TYPE", DEGEN_GEOM_M_TYPE, "/*!< Degen Geom M file type */" );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "COMPUTATION_FILE_TYPE", "CFD_STL_TYPE", CFD_STL_TYPE, "/*!< CFD Mesh STL file type */" );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "COMPUTATION_FILE_TYPE", "CFD_POLY_TYPE", CFD_POLY_TYPE, "/*!<CFD Mesh POLY file type */" );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "COMPUTATION_FILE_TYPE", "CFD_TRI_TYPE", CFD_TRI_TYPE, "/*!< CFD Mesh TRI file type */" );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "COMPUTATION_FILE_TYPE", "CFD_OBJ_TYPE", CFD_OBJ_TYPE, "/*!< CFD Mesh OBJ file type */" );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "COMPUTATION_FILE_TYPE", "CFD_DAT_TYPE", CFD_DAT_TYPE, "/*!< CFD Mesh DAT file type */" );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "COMPUTATION_FILE_TYPE", "CFD_KEY_TYPE", CFD_KEY_TYPE, "/*!< CFD Mesh KAY file type */" );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "COMPUTATION_FILE_TYPE", "CFD_GMSH_TYPE", CFD_GMSH_TYPE, "/*!< CFD Mesh GMSH file type */" );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "COMPUTATION_FILE_TYPE", "CFD_SRF_TYPE", CFD_SRF_TYPE, "/*!< CFD Mesh SRF file type */" );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "COMPUTATION_FILE_TYPE", "CFD_TKEY_TYPE", CFD_TKEY_TYPE, "/*!< CFD Mesh TKEY file type */" );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "COMPUTATION_FILE_TYPE", "PROJ_AREA_CSV_TYPE", PROJ_AREA_CSV_TYPE, "/*!< Projected Area CSV file type */" );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "COMPUTATION_FILE_TYPE", "WAVE_DRAG_TXT_TYPE", WAVE_DRAG_TXT_TYPE, "/*!< Wave Drag TXT file type */" );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "COMPUTATION_FILE_TYPE", "VSPAERO_PANEL_TRI_TYPE", VSPAERO_PANEL_TRI_TYPE, "/*!< VSPAERO Panel Method TRI file type */" );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "COMPUTATION_FILE_TYPE", "DRAG_BUILD_CSV_TYPE", DRAG_BUILD_CSV_TYPE, "/*!< Parasite Drag CSV file type */" );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "COMPUTATION_FILE_TYPE", "CFD_FACET_TYPE", CFD_FACET_TYPE, "/*!< CFD Mesh FACET file type */" );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "COMPUTATION_FILE_TYPE", "CFD_CURV_TYPE", CFD_CURV_TYPE, "/*!< CFD Mesh CURV file type */" );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "COMPUTATION_FILE_TYPE", "CFD_PLOT3D_TYPE", CFD_PLOT3D_TYPE, "/*!< CFD Mesh PLOT3D file type */" );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "COMPUTATION_FILE_TYPE", "CFD_VSPGEOM_TYPE", CFD_VSPGEOM_TYPE, "/*!< CFD Mesh VSPGEOM file type */" );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "COMPUTATION_FILE_TYPE", "VSPAERO_VSPGEOM_TYPE", VSPAERO_VSPGEOM_TYPE, "/*!< VSPAERO VSPGEOM file type */" );
    assert( r >= 0 );

    doc_struct.comment = "/*! Enum used to identify delimiter type. */";

    r = se->RegisterEnum( "DELIM_TYPE", doc_struct );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "DELIM_TYPE", "DELIM_COMMA", DELIM_COMMA, "/*!< Comma delimiter */" );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "DELIM_TYPE", "DELIM_USCORE", DELIM_USCORE, "/*!< Underscore delimiter */" );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "DELIM_TYPE", "DELIM_SPACE", DELIM_SPACE, "/*!< Space delimiter */" );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "DELIM_TYPE", "DELIM_NONE", DELIM_NONE, "/*!< No delimiter */" );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "DELIM_TYPE", "DELIM_NUM_TYPES", DELIM_NUM_TYPES, "/*!< Number of delimiter types */" );
    assert( r >= 0 );

    doc_struct.comment = "/*! Enum for 2D or 3D DXF export options. */";

    r = se->RegisterEnum( "DIMENSION_SET", doc_struct );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "DIMENSION_SET", "SET_3D", SET_3D, "/*!< 3D DXF export (default) */" );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "DIMENSION_SET", "SET_2D", SET_2D, "/*!< 2D DXF export */" );
    assert( r >= 0 );

    doc_struct.comment = "/*! Enum used to identify axis of rotation or translation. */";

    r = se->RegisterEnum( "DIR_INDEX", doc_struct );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "DIR_INDEX", "X_DIR", X_DIR, "/*!< X direction */" );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "DIR_INDEX", "Y_DIR", Y_DIR, "/*!< Y direction */" );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "DIR_INDEX", "Z_DIR", Z_DIR, "/*!< Z direction */" );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "DIR_INDEX", "ALL_DIR", ALL_DIR, "/*!< All directions */" );
    assert( r >= 0 );

    doc_struct.comment = "/*! Enum for selecting the GUI display type for Geoms. */";

    r = se->RegisterEnum( "DISPLAY_TYPE", doc_struct );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "DISPLAY_TYPE", "DISPLAY_BEZIER", DISPLAY_BEZIER, "/*!< Display the normal Bezier surface (default) */" );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "DISPLAY_TYPE", "DISPLAY_DEGEN_SURF", DISPLAY_DEGEN_SURF, "/*!< Display as surface Degen Geom */" );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "DISPLAY_TYPE", "DISPLAY_DEGEN_PLATE", DISPLAY_DEGEN_PLATE, "/*!< Display as plate Degen Geom */" );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "DISPLAY_TYPE", "DISPLAY_DEGEN_CAMBER", DISPLAY_DEGEN_CAMBER, "/*!< Display as camber Degen Geom */" );
    assert( r >= 0 );

    doc_struct.comment = "/*! Enum for selecting the GUI draw type for Geoms. */";

    r = se->RegisterEnum( "DRAW_TYPE", doc_struct );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "DRAW_TYPE", "GEOM_DRAW_WIRE", GEOM_DRAW_WIRE, "/*!< Draw the wireframe mesh (see through) */" );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "DRAW_TYPE", "GEOM_DRAW_HIDDEN", GEOM_DRAW_HIDDEN, "/*!< Draw the hidden mesh */" );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "DRAW_TYPE", "GEOM_DRAW_SHADE", GEOM_DRAW_SHADE, "/*!< Draw the shaded mesh */" );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "DRAW_TYPE", "GEOM_DRAW_TEXTURE", GEOM_DRAW_TEXTURE, "/*!< Draw the textured mesh */" );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "DRAW_TYPE", "GEOM_DRAW_NONE", GEOM_DRAW_NONE, "/*!< Do not draw anything */" );
    assert( r >= 0 );

    doc_struct.comment = "/*! Enum for OpenVSP API error codes. */";

    r = se->RegisterEnum( "ERROR_CODE", doc_struct );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "ERROR_CODE", "VSP_OK", vsp::VSP_OK, "/*!< No error */" );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "ERROR_CODE", "VSP_INVALID_PTR", vsp::VSP_INVALID_PTR, "/*!< Invalid pointer error */" );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "ERROR_CODE", "VSP_INVALID_TYPE", vsp::VSP_INVALID_TYPE, "/*!< Invalid type error */" );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "ERROR_CODE", "VSP_CANT_FIND_TYPE", vsp::VSP_CANT_FIND_TYPE, "/*!< Can't find type error */" );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "ERROR_CODE", "VSP_CANT_FIND_PARM", vsp::VSP_CANT_FIND_PARM, "/*!< Can't find parm error */" );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "ERROR_CODE", "VSP_CANT_FIND_NAME", vsp::VSP_CANT_FIND_NAME, "/*!< Can't find name error */" );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "ERROR_CODE", "VSP_INVALID_GEOM_ID", vsp::VSP_INVALID_GEOM_ID, "/*!< Invalid Geom ID error */" );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "ERROR_CODE", "VSP_FILE_DOES_NOT_EXIST", vsp::VSP_FILE_DOES_NOT_EXIST, "/*!< File does not exist error */" );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "ERROR_CODE", "VSP_FILE_WRITE_FAILURE", vsp::VSP_FILE_WRITE_FAILURE, "/*!< File write failure error */" );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "ERROR_CODE", "VSP_FILE_READ_FAILURE", vsp::VSP_FILE_READ_FAILURE, "/*!< File read failure error */" );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "ERROR_CODE", "VSP_WRONG_XSEC_TYPE", vsp::VSP_WRONG_XSEC_TYPE, "/*!< Wrong XSec type error */" );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "ERROR_CODE", "VSP_WRONG_FILE_TYPE", vsp::VSP_WRONG_FILE_TYPE, "/*!< Wrong file type error */" );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "ERROR_CODE", "VSP_INDEX_OUT_RANGE", vsp::VSP_INDEX_OUT_RANGE, "/*!< Index out of range error */" );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "ERROR_CODE", "VSP_INVALID_XSEC_ID", vsp::VSP_INVALID_XSEC_ID, "/*!< Invalid XSec ID error */" );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "ERROR_CODE", "VSP_INVALID_ID", vsp::VSP_INVALID_ID, "/*!< Invalid ID error */" );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "ERROR_CODE", "VSP_CANT_SET_NOT_EQ_PARM", vsp::VSP_CANT_SET_NOT_EQ_PARM, "/*!< Can't set NotEqParm error */" );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "ERROR_CODE", "VSP_AMBIGUOUS_SUBSURF", vsp::VSP_AMBIGUOUS_SUBSURF, "/*!< Ambiguous flow-through sub-surface error */" );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "ERROR_CODE", "VSP_INVALID_VARPRESET_SETNAME", vsp::VSP_INVALID_VARPRESET_SETNAME, "/*!< Invalid Variable Preset set name error */" );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "ERROR_CODE", "VSP_INVALID_VARPRESET_GROUPNAME", vsp::VSP_INVALID_VARPRESET_GROUPNAME, "/*!< Invalid Variable Preset group name error */" );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "ERROR_CODE", "VSP_CONFORMAL_PARENT_UNSUPPORTED", vsp::VSP_CONFORMAL_PARENT_UNSUPPORTED, "/*!< Unsupported Conformal Geom parent error */" );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "ERROR_CODE", "VSP_UNEXPECTED_RESET_REMAP_ID", vsp::VSP_UNEXPECTED_RESET_REMAP_ID, "/*!< Unexpected reset remap ID error */" );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "ERROR_CODE", "VSP_INVALID_INPUT_VAL", vsp::VSP_INVALID_INPUT_VAL, "/*!< Invalid input value error */" );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "ERROR_CODE", "VSP_INVALID_CF_EQN", vsp::VSP_INVALID_CF_EQN, "/*!< Invalid friction coefficient equation error */" );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "ERROR_CODE", "VSP_INVALID_DRIVERS", vsp::VSP_INVALID_DRIVERS, "/*!< Invalid drivers for driver group */" );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "ERROR_CODE", "VSP_ADV_LINK_BUILD_FAIL", vsp::VSP_ADV_LINK_BUILD_FAIL, "/*!< Advanced link build failure */" );
    assert( r >= 0 );

    doc_struct.comment = "/*! Enum used to indicate Parasite Drag Tool excressence type. */";

    r = se->RegisterEnum( "EXCRES_TYPE", doc_struct );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "EXCRES_TYPE", "EXCRESCENCE_COUNT", EXCRESCENCE_COUNT, "/*!< Drag counts excressence type */" );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "EXCRES_TYPE", "EXCRESCENCE_CD", EXCRESCENCE_CD, "/*!< Drag coefficient excressence type */" );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "EXCRES_TYPE", "EXCRESCENCE_PERCENT_GEOM", EXCRESCENCE_PERCENT_GEOM, "/*!< Percent of parent Geom drag coefficient excressence type */" );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "EXCRES_TYPE", "EXCRESCENCE_MARGIN", EXCRESCENCE_MARGIN, "/*!< Percent margin excressence type */" );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "EXCRES_TYPE", "EXCRESCENCE_DRAGAREA", EXCRESCENCE_DRAGAREA, "/*!< Drag area (D/q) excressence type */" );
    assert( r >= 0 );

    doc_struct.comment = "/*! Enum for OpenVSP export types. */";

    r = se->RegisterEnum( "EXPORT_TYPE", doc_struct );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "EXPORT_TYPE", "EXPORT_FELISA", EXPORT_FELISA, "/*!< FELISA export type (NOT IMPLEMENTED) */" ); // TODO: Remove or implement
    assert( r >= 0 );
    r = se->RegisterEnumValue( "EXPORT_TYPE", "EXPORT_XSEC", EXPORT_XSEC, "/*!< XSec (*.hrm) export type */" );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "EXPORT_TYPE", "EXPORT_STL", EXPORT_STL, "/*!< Stereolith (*.stl) export type */" );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "EXPORT_TYPE", "EXPORT_AWAVE", EXPORT_AWAVE, "/*!< AWAVE export type (NOT IMPLEMENTED) */" ); // TODO: Remove or implement
    assert( r >= 0 );
    r = se->RegisterEnumValue( "EXPORT_TYPE", "EXPORT_NASCART", EXPORT_NASCART, "/*!< NASCART (*.dat) export type */" );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "EXPORT_TYPE", "EXPORT_POVRAY", EXPORT_POVRAY, "/*!< POVRAY (*.pov) export type */" );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "EXPORT_TYPE", "EXPORT_CART3D", EXPORT_CART3D, "/*!< Cart3D (*.tri) export type */" );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "EXPORT_TYPE", "EXPORT_VSPGEOM", EXPORT_VSPGEOM, "/*!< VSPGeom (*.vspgeom) export type */" );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "EXPORT_TYPE", "EXPORT_VORXSEC", EXPORT_VORXSEC, "/*!< VORXSEC  export type (NOT IMPLEMENTED) */" ); // TODO: Remove or implement
    assert( r >= 0 );
    r = se->RegisterEnumValue( "EXPORT_TYPE", "EXPORT_XSECGEOM", EXPORT_XSECGEOM, "/*!< XSECGEOM export type (NOT IMPLEMENTED) */" ); // TODO: Remove or implement
    assert( r >= 0 );
    r = se->RegisterEnumValue( "EXPORT_TYPE", "EXPORT_GMSH", EXPORT_GMSH, "/*!< Gmsh (*.msh) export type */" );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "EXPORT_TYPE", "EXPORT_X3D", EXPORT_X3D, "/*!< X3D (*.x3d) export type */" );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "EXPORT_TYPE", "EXPORT_STEP", EXPORT_STEP, "/*!< STEP (*.stp) export type */" );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "EXPORT_TYPE", "EXPORT_PLOT3D", EXPORT_PLOT3D, "/*!< PLOT3D (*.p3d) export type */" );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "EXPORT_TYPE", "EXPORT_IGES", EXPORT_IGES, "/*!< IGES (*.igs) export type */" );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "EXPORT_TYPE", "EXPORT_BEM", EXPORT_BEM, "/*!< Blade Element (*.bem) export type */" );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "EXPORT_TYPE", "EXPORT_DXF", EXPORT_DXF, "/*!< AutoCAD (*.dxf) export type */" );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "EXPORT_TYPE", "EXPORT_FACET", EXPORT_FACET, "/*!< Xpatch (*.facet) export type */" );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "EXPORT_TYPE", "EXPORT_SVG", EXPORT_SVG, "/*!< SVG (*.svg) export type */" );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "EXPORT_TYPE", "EXPORT_PMARC", EXPORT_PMARC, "/*!< PMARC 12 (*.pmin) export type */" );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "EXPORT_TYPE", "EXPORT_OBJ", EXPORT_OBJ, "/*!< OBJ (*.obj) export type */" );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "EXPORT_TYPE", "EXPORT_SELIG_AIRFOIL", EXPORT_SELIG_AIRFOIL, "/*!< Airfoil points (*.dat) export type */" );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "EXPORT_TYPE", "EXPORT_BEZIER_AIRFOIL", EXPORT_BEZIER_AIRFOIL, "/*!< Airfoil curves (*.bz) export type */" );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "EXPORT_TYPE", "EXPORT_IGES_STRUCTURE", EXPORT_IGES_STRUCTURE, "/*!< IGES structure (*.igs) export type */" );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "EXPORT_TYPE", "EXPORT_STEP_STRUCTURE", EXPORT_STEP_STRUCTURE, "/*!< STEP structure (*.stp) export type */" );
    assert( r >= 0 );

    doc_struct.comment = "/*! Enum used to indicate FEA beam element cross-section type. */";

    r = se->RegisterEnum( "FEA_CROSS_SECT_TYPE", doc_struct );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "FEA_CROSS_SECT_TYPE", "FEA_XSEC_GENERAL", FEA_XSEC_GENERAL, "/*!< General XSec type */" );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "FEA_CROSS_SECT_TYPE", "FEA_XSEC_CIRC", FEA_XSEC_CIRC, "/*!< Circle XSec type */" );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "FEA_CROSS_SECT_TYPE", "FEA_XSEC_PIPE", FEA_XSEC_PIPE, "/*!< Pipe XSec type */" );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "FEA_CROSS_SECT_TYPE", "FEA_XSEC_I", FEA_XSEC_I, "/*!< I XSec type */" );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "FEA_CROSS_SECT_TYPE", "FEA_XSEC_RECT", FEA_XSEC_RECT, "/*!< Rectangle XSec type */" );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "FEA_CROSS_SECT_TYPE", "FEA_XSEC_BOX", FEA_XSEC_BOX, "/*!< Box XSec type */" );
    assert( r >= 0 );

    doc_struct.comment = "/*! Enum for the various FEA Mesh export types. */";

    r = se->RegisterEnum( "FEA_EXPORT_TYPE", doc_struct );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "FEA_EXPORT_TYPE", "FEA_MASS_FILE_NAME", FEA_MASS_FILE_NAME, "/*!< FEA Mesh mass export type */" );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "FEA_EXPORT_TYPE", "FEA_NASTRAN_FILE_NAME", FEA_NASTRAN_FILE_NAME, "/*!< FEA Mesh NASTRAN export type */" );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "FEA_EXPORT_TYPE", "FEA_NKEY_FILE_NAME", FEA_NKEY_FILE_NAME, "/*!< FEA Mesh NKey export type */" );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "FEA_EXPORT_TYPE", "FEA_CALCULIX_FILE_NAME", FEA_CALCULIX_FILE_NAME, "/*!< FEA Mesh Calculix export type */" );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "FEA_EXPORT_TYPE", "FEA_STL_FILE_NAME", FEA_STL_FILE_NAME, "/*!< FEA Mesh STL export type */" );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "FEA_EXPORT_TYPE", "FEA_GMSH_FILE_NAME", FEA_GMSH_FILE_NAME, "/*!< FEA Mesh GMSH export type */" );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "FEA_EXPORT_TYPE", "FEA_SRF_FILE_NAME", FEA_SRF_FILE_NAME, "/*!< FEA Mesh SRF export type */" );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "FEA_EXPORT_TYPE", "FEA_CURV_FILE_NAME", FEA_CURV_FILE_NAME, "/*!< FEA Mesh CURV export type */" );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "FEA_EXPORT_TYPE", "FEA_PLOT3D_FILE_NAME", FEA_PLOT3D_FILE_NAME, "/*!< FEA Mesh PLOT3D export type */" );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "FEA_EXPORT_TYPE", "FEA_IGES_FILE_NAME", FEA_IGES_FILE_NAME, "/*!< FEA Mesh trimmed IGES export type */" );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "FEA_EXPORT_TYPE", "FEA_STEP_FILE_NAME", FEA_STEP_FILE_NAME, "/*!< FEA Mesh trimmed STEP export type */" );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "FEA_EXPORT_TYPE", "FEA_NUM_FILE_NAMES", FEA_NUM_FILE_NAMES, "/*!< Number of FEA Mesh export type. */" );
    assert( r >= 0 );

    doc_struct.comment = "/*! Enum for FEA material types. */";

    r = se->RegisterEnum( "FEA_MATERIAL_TYPE", doc_struct );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "FEA_MATERIAL_TYPE", "FEA_ISOTROPIC", FEA_ISOTROPIC, "/*!< Isotropic material */" );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "FEA_MATERIAL_TYPE", "FEA_ENG_ORTHO", FEA_ENG_ORTHO, "/*!< Orthotropic material in engineering parameters */" );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "FEA_MATERIAL_TYPE", "FEA_NUM_MAT_TYPES", FEA_NUM_MAT_TYPES, "/*!< Number of FEA material types */" );
    assert( r >= 0 );

    doc_struct.comment = "/*! Enum for FEA material orientation types. */";

    r = se->RegisterEnum( "FEA_ORIENTATION_TYPE", doc_struct );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "FEA_ORIENTATION_TYPE", "FEA_ORIENT_GLOBAL_X", FEA_ORIENT_GLOBAL_X, "/*!< FEA Global X material orientation */" );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "FEA_ORIENTATION_TYPE", "FEA_ORIENT_GLOBAL_Y", FEA_ORIENT_GLOBAL_Y, "/*!< FEA Global Y material orientation */" );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "FEA_ORIENTATION_TYPE", "FEA_ORIENT_GLOBAL_Z", FEA_ORIENT_GLOBAL_Z, "/*!< FEA Global Z material orientation */" );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "FEA_ORIENTATION_TYPE", "FEA_ORIENT_COMP_X", FEA_ORIENT_COMP_X, "/*!< FEA Comp X material orientation */" );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "FEA_ORIENTATION_TYPE", "FEA_ORIENT_COMP_Y", FEA_ORIENT_COMP_Y, "/*!< FEA Comp Y material orientation */" );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "FEA_ORIENTATION_TYPE", "FEA_ORIENT_COMP_Z", FEA_ORIENT_COMP_Z, "/*!< FEA Comp Z material orientation */" );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "FEA_ORIENTATION_TYPE", "FEA_ORIENT_PART_U", FEA_ORIENT_PART_U, "/*!< FEA Part U material orientation */" );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "FEA_ORIENTATION_TYPE", "FEA_ORIENT_PART_V", FEA_ORIENT_PART_V, "/*!< FEA Part V material orientation */" );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "FEA_ORIENTATION_TYPE", "FEA_ORIENT_OML_U", FEA_ORIENT_OML_U, "/*!< FEA OML U material orientation */" );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "FEA_ORIENTATION_TYPE", "FEA_ORIENT_OML_V", FEA_ORIENT_OML_V, "/*!< FEA OML V material orientation */" );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "FEA_ORIENTATION_TYPE", "FEA_ORIENT_OML_R", FEA_ORIENT_OML_R, "/*!< FEA OML R material orientation */" );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "FEA_ORIENTATION_TYPE", "FEA_ORIENT_OML_S", FEA_ORIENT_OML_S, "/*!< FEA OML S material orientation */" );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "FEA_ORIENTATION_TYPE", "FEA_ORIENT_OML_T", FEA_ORIENT_OML_T, "/*!< FEA OML T material orientation */" );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "FEA_ORIENTATION_TYPE", "FEA_NUM_ORIENT_TYPES", FEA_NUM_ORIENT_TYPES, "/*!< Number of FEA material orientation types */" );
    assert( r >= 0 );

    doc_struct.comment = "/*! Enum for FEA Part element types. */";

    r = se->RegisterEnum( "FEA_PART_ELEMENT_TYPE", doc_struct );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "FEA_PART_ELEMENT_TYPE", "FEA_SHELL", FEA_SHELL, "/*!< Shell (tris) FEA element type */" );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "FEA_PART_ELEMENT_TYPE", "FEA_BEAM", FEA_BEAM, "/*!< Beam FEA element type */" );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "FEA_PART_ELEMENT_TYPE", "FEA_SHELL_AND_BEAM", FEA_SHELL_AND_BEAM, "/*!< Both Shell and Beam FEA element types */" );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "FEA_PART_ELEMENT_TYPE", "FEA_NO_ELEMENTS", FEA_NO_ELEMENTS, "/*!< FEA part with no elements */" );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "FEA_PART_ELEMENT_TYPE", "FEA_NUM_ELEMENT_TYPES", FEA_NUM_ELEMENT_TYPES, "/*!< Number of FEA element type choices */" );
    assert( r >= 0 );

    doc_struct.comment = "/*! Enum used to identify the available FEA Part types. */";

    r = se->RegisterEnum( "FEA_PART_TYPE", doc_struct );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "FEA_PART_TYPE", "FEA_SLICE", FEA_SLICE, "/*!< Slice FEA Part type */" );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "FEA_PART_TYPE", "FEA_RIB", FEA_RIB, "/*!< Rib FEA Part type */" );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "FEA_PART_TYPE", "FEA_SPAR", FEA_SPAR, "/*!< Spar FEA Part type */" );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "FEA_PART_TYPE", "FEA_FIX_POINT", FEA_FIX_POINT, "/*!< Fixed Point FEA Part type */" );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "FEA_PART_TYPE", "FEA_DOME", FEA_DOME, "/*!< Dome FEA Part type */" );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "FEA_PART_TYPE", "FEA_RIB_ARRAY", FEA_RIB_ARRAY, "/*!< Rib array FEA Part type */" );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "FEA_PART_TYPE", "FEA_SLICE_ARRAY", FEA_SLICE_ARRAY, "/*!< Slice array FEA Part type */" );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "FEA_PART_TYPE", "FEA_TRIM", FEA_TRIM, "/*!< Trim FEA Part type */" );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "FEA_PART_TYPE", "FEA_SKIN", FEA_SKIN, "/*!< Skin FEA Part type */" );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "FEA_PART_TYPE", "FEA_NUM_TYPES", FEA_NUM_TYPES, "/*!< Number of FEA Part types */" );
    assert( r >= 0 );

    doc_struct.comment = "/*! Enum for FEA Slice orientation. */";

    r = se->RegisterEnum( "FEA_SLICE_TYPE", doc_struct );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "FEA_SLICE_TYPE", "XY_BODY", XY_BODY, "/*!< Slice is parallel to parent Geom body XY plane */" );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "FEA_SLICE_TYPE", "YZ_BODY", YZ_BODY, "/*!< Slice is parallel to parent Geom body YZ plane */" );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "FEA_SLICE_TYPE", "XZ_BODY", XZ_BODY, "/*!< Slice is parallel to parent Geom body XZ plane */" );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "FEA_SLICE_TYPE", "XY_ABS", XY_ABS, "/*!< Slice is parallel to absolute XY plane */" );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "FEA_SLICE_TYPE", "YZ_ABS", YZ_ABS, "/*!< Slice is parallel to absolute YZ plane */" );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "FEA_SLICE_TYPE", "XZ_ABS", XZ_ABS, "/*!< Slice is parallel to absolute XZ plane */" );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "FEA_SLICE_TYPE", "SPINE_NORMAL", SPINE_NORMAL, "/*!< Slice is perpendicular to thespine of the parent Geom */" );
    assert( r >= 0 );

    doc_struct.comment = "/*! Enum used to identify the FEA Mesh unit system. */";

    r = se->RegisterEnum( "FEA_UNIT_TYPE", doc_struct );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "FEA_UNIT_TYPE", "SI_UNIT", SI_UNIT, "/*!< SI unit system */" );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "FEA_UNIT_TYPE", "CGS_UNIT", CGS_UNIT, "/*!< CGS unit system */" );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "FEA_UNIT_TYPE", "MPA_UNIT", MPA_UNIT, "/*!< MPA unit system */" );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "FEA_UNIT_TYPE", "BFT_UNIT", BFT_UNIT, "/*!< BFT unit system */" );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "FEA_UNIT_TYPE", "BIN_UNIT", BIN_UNIT, "/*!< BIN unit system */" );
    assert( r >= 0 );

    doc_struct.comment = "/*! Enum that defines the type of edge to set the initial position of FEA Ribs and FEA Rib Arrays to. */";

    r = se->RegisterEnum( "FEA_RIB_NORMAL", doc_struct );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "FEA_RIB_NORMAL", "NO_NORMAL", NO_NORMAL, "/*!< FEA Rib or Rib Array has no set perpendicular edge */" );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "FEA_RIB_NORMAL", "LE_NORMAL", LE_NORMAL, "/*!< FEA Rib or Rib Array is set perpendicular to the leading edge */" );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "FEA_RIB_NORMAL", "TE_NORMAL", TE_NORMAL, "/*!< FEA Rib or Rib Array is set perpendicular to the trailing edge */" );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "FEA_RIB_NORMAL", "SPAR_NORMAL", SPAR_NORMAL, "/*!< FEA Rib or Rib Array is set perpendicular to an FEA Spar */" );
    assert( r >= 0 );

    doc_struct.comment = "/*! Enum for Parasite Drag Tool form factor equations for body-type components. */";

    r = se->RegisterEnum( "FF_B_EQN", doc_struct );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "FF_B_EQN", "FF_B_MANUAL", FF_B_MANUAL, "/*!< Manual FF equation */" );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "FF_B_EQN", "FF_B_SCHEMENSKY_BODY", FF_B_SCHEMENSKY_FUSE, "/*!< Schemensky Fuselage FF equation */" );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "FF_B_EQN", "FF_B_SCHEMENSKY_NACELLE", FF_B_SCHEMENSKY_NACELLE, "/*!< Schemensky Nacelle FF equation */" );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "FF_B_EQN", "FF_B_HOERNER_STREAMBODY", FF_B_HOERNER_STREAMBODY, "/*!< Hoerner Streamlined Body FF equation */" );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "FF_B_EQN", "FF_B_TORENBEEK", FF_B_TORENBEEK, "/*!< Torenbeek FF equation */" );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "FF_B_EQN", "FF_B_SHEVELL", FF_B_SHEVELL, "/*!< Shevell FF equation */" );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "FF_B_EQN", "FF_B_COVERT", FF_B_COVERT, "/*!< Covert FF equation */" );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "FF_B_EQN", "FF_B_JENKINSON_FUSE", FF_B_JENKINSON_FUSE, "/*!< Jenkinson Fuselage FF equation */" );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "FF_B_EQN", "FF_B_JENKINSON_WING_NACELLE", FF_B_JENKINSON_WING_NACELLE, "/*!< Jenkinson Wing Nacelle FF equation */" );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "FF_B_EQN", "FF_B_JENKINSON_AFT_FUSE_NACELLE", FF_B_JENKINSON_AFT_FUSE_NACELLE, "/*!< Jenkinson Aft Fuselage Nacelle FF equation */" );
    assert( r >= 0 );

    doc_struct.comment = "/*! Enum for Parasite Drag Tool form factor equations for wing-type components. */";

    r = se->RegisterEnum( "FF_W_EQN", doc_struct );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "FF_W_EQN", "FF_W_MANUAL", FF_W_MANUAL, "/*!< Manual FF equation */" );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "FF_W_EQN", "FF_W_EDET_CONV", FF_W_EDET_CONV, "/*!< EDET Conventional Airfoil FF equation */" );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "FF_W_EQN", "FF_W_EDET_ADV", FF_W_EDET_ADV, "/*!< EDET Advanced Airfoil FF equation */" );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "FF_W_EQN", "FF_W_HOERNER", FF_W_HOERNER, "/*!< Hoerner FF equation */" );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "FF_W_EQN", "FF_W_COVERT", FF_W_COVERT, "/*!< Covert FF equation */" );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "FF_W_EQN", "FF_W_WILLIAMS", FF_W_SHEVELL, "/*!< Shevell FF equation */" );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "FF_W_EQN", "FF_W_KROO", FF_W_KROO, "/*!< Kroo FF equation */" );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "FF_W_EQN", "FF_W_TORENBEEK", FF_W_TORENBEEK, "/*!< Torenbeek FF equation */" );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "FF_W_EQN", "FF_W_DATCOM", FF_W_DATCOM, "/*!< DATCOM FF equation */" );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "FF_W_EQN", "FF_W_SCHEMENSKY_6_SERIES_AF", FF_W_SCHEMENSKY_6_SERIES_AF, "/*!< Schemensky 6 Series Airfoil FF equation */" );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "FF_W_EQN", "FF_W_SCHEMENSKY_4_SERIES_AF", FF_W_SCHEMENSKY_4_SERIES_AF, "/*!< Schemensky 4 Series Airfoil FF equation */" );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "FF_W_EQN", "FF_W_JENKINSON_WING", FF_W_JENKINSON_WING, "/*!< Jenkinson Wing FF equation */" );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "FF_W_EQN", "FF_W_JENKINSON_TAIL", FF_W_JENKINSON_TAIL, "/*!< Jenkinson Tail FF equation */" );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "FF_W_EQN", "FF_W_SCHEMENSKY_SUPERCRITICAL_AF", FF_W_SCHEMENSKY_SUPERCRITICAL_AF, "/*!< Schemensky Supercritical Airfoil FF equation */" );
    assert( r >= 0 );

    doc_struct.comment = "/*! Enum for Parasite Drag Tool freestream unit system. */";

    r = se->RegisterEnum( "FREESTREAM_PD_UNITS", doc_struct );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "FREESTREAM_PD_UNITS", "PD_UNITS_IMPERIAL", PD_UNITS_IMPERIAL, "/*!< Imperial unit system */"  );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "FREESTREAM_PD_UNITS", "PD_UNITS_METRIC", PD_UNITS_METRIC, "/*!< Metric unit system */"  );
    assert( r >= 0 );

    doc_struct.comment = "/*! Enum used for custom GUI development. */";

    r = se->RegisterEnum( "GDEV", doc_struct );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "GDEV", "GDEV_TAB", GDEV_TAB, "/*!< Custom GUI Tab */" );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "GDEV", "GDEV_SCROLL_TAB", GDEV_SCROLL_TAB, "/*!< Custom GUI Fl_Scroll and Tab */" );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "GDEV", "GDEV_GROUP", GDEV_GROUP, "/*!< Custom GUI Group */" );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "GDEV", "GDEV_PARM_BUTTON", GDEV_PARM_BUTTON, "/*!< Custom GUI ParmButton */" );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "GDEV", "GDEV_INPUT", GDEV_INPUT, "/*!< Custom GUI Input */" );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "GDEV", "GDEV_OUTPUT", GDEV_OUTPUT, "/*!< Custom GUI Output */" );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "GDEV", "GDEV_SLIDER", GDEV_SLIDER, "/*!< Custom GUI Slider */" );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "GDEV", "GDEV_SLIDER_ADJ_RANGE", GDEV_SLIDER_ADJ_RANGE, "/*!< Custom GUI SliderAdjRangeInput */" );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "GDEV", "GDEV_CHECK_BUTTON", GDEV_CHECK_BUTTON, "/*!< Custom GUI CheckButton */" );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "GDEV", "GDEV_RADIO_BUTTON", GDEV_RADIO_BUTTON, "/*!< Custom GUI RadioButton */" );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "GDEV", "GDEV_TOGGLE_BUTTON", GDEV_TOGGLE_BUTTON, "/*!< Custom GUI ToggleButton */" );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "GDEV", "GDEV_TOGGLE_RADIO_GROUP", GDEV_TOGGLE_RADIO_GROUP, "/*!< Custom GUI ToggleRadioGroup (NOT IMPLEMENTED) */" ); // TODO: Implement or remove
    assert( r >= 0 );
    r = se->RegisterEnumValue( "GDEV", "GDEV_TRIGGER_BUTTON", GDEV_TRIGGER_BUTTON, "/*!< Custom GUI TriggerButton */" );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "GDEV", "GDEV_COUNTER", GDEV_COUNTER, "/*!< Custom GUI Counter */" );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "GDEV", "GDEV_CHOICE", GDEV_CHOICE, "/*!< Custom GUI Choice */" );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "GDEV", "GDEV_ADD_CHOICE_ITEM", GDEV_ADD_CHOICE_ITEM, "/*!< Add item to custom GUI Choice */" );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "GDEV", "GDEV_SLIDER_INPUT", GDEV_SLIDER_INPUT, "/*!< Custom GUI SliderInput */" );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "GDEV", "GDEV_SLIDER_ADJ_RANGE_INPUT", GDEV_SLIDER_ADJ_RANGE_INPUT, "/*!< Custom GUI SliderAdjRangeInput */" );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "GDEV", "GDEV_SLIDER_ADJ_RANGE_TWO_INPUT", GDEV_SLIDER_ADJ_RANGE_TWO_INPUT, "/*!< Custom GUI SliderAdjRangeInput with two inputs (NOT IMPLEMENTED) */" ); // TODO: Implement or remove
    assert( r >= 0 );
    r = se->RegisterEnumValue( "GDEV", "GDEV_FRACT_PARM_SLIDER", GDEV_FRACT_PARM_SLIDER, "/*!< Custom GUI FractParmSlider */" );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "GDEV", "GDEV_STRING_INPUT", GDEV_STRING_INPUT, "/*!< Custom GUI StringInput */" );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "GDEV", "GDEV_INDEX_SELECTOR", GDEV_INDEX_SELECTOR, "/*!< Custom GUI IndexSelector */" );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "GDEV", "GDEV_COLOR_PICKER", GDEV_COLOR_PICKER, "/*!< Custom GUI ColorPicker */" );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "GDEV", "GDEV_YGAP", GDEV_YGAP, "/*!< Custom GUI Y gap */" );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "GDEV", "GDEV_DIVIDER_BOX", GDEV_DIVIDER_BOX, "/*!< Custom GUI divider box */" );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "GDEV", "GDEV_BEGIN_SAME_LINE", GDEV_BEGIN_SAME_LINE, "/*!< Set begin same line flag for custom GUI */" );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "GDEV", "GDEV_END_SAME_LINE", GDEV_END_SAME_LINE, "/*!< Set end same line flag for custom GUI */" );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "GDEV", "GDEV_FORCE_WIDTH", GDEV_FORCE_WIDTH, "/*!< Set forced width for custom GUI */" );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "GDEV", "GDEV_SET_FORMAT", GDEV_SET_FORMAT, "/*!< Set format label for custom GUI */" );
    assert( r >= 0 );

    doc_struct.comment = "/*! Enum for OpenVSP Human component gender types. */";

    r = se->RegisterEnum( "GENDER", doc_struct );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "GENDER", "MALE", MALE, "/*!< Male Human component */" );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "GENDER", "FEMALE", FEMALE, "/*!< Female Human component */" );
    assert( r >= 0 );

    doc_struct.comment = "/*! Initial shape enums for XS_EDIT_CURVE type XSecs. */";

    r = se->RegisterEnum( "INIT_EDIT_XSEC_TYPE", doc_struct );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "INIT_EDIT_XSEC_TYPE", "EDIT_XSEC_CIRCLE", EDIT_XSEC_CIRCLE, "/*!< Circle initialized as cubic Bezier type */" );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "INIT_EDIT_XSEC_TYPE", "EDIT_XSEC_ELLIPSE", EDIT_XSEC_ELLIPSE, "/*!< Ellipse initialized as PCHIP type */" );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "INIT_EDIT_XSEC_TYPE", "EDIT_XSEC_RECTANGLE", EDIT_XSEC_RECTANGLE, "/*!< Rectangle initialized as linear type */" );
    assert( r >= 0 );

    doc_struct.comment = "/*! Enum for OpenVSP import types. */";

    r = se->RegisterEnum( "IMPORT_TYPE", doc_struct );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "IMPORT_TYPE", "IMPORT_STL", IMPORT_STL, "/*!< Stereolith (*.stl) import */" );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "IMPORT_TYPE", "IMPORT_NASCART", IMPORT_NASCART, "/*!< NASCART (*.dat) import */" );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "IMPORT_TYPE", "IMPORT_CART3D_TRI", IMPORT_CART3D_TRI, "/*!< Cart3D (*.try) import */" );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "IMPORT_TYPE", "IMPORT_XSEC_MESH", IMPORT_XSEC_MESH, "/*!< XSec as Tri Mesh (*.hrm) import */" );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "IMPORT_TYPE", "IMPORT_PTS", IMPORT_PTS, "/*!< Point Cloud (*.pts) import */" );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "IMPORT_TYPE", "IMPORT_V2", IMPORT_V2, "/*!< OpenVSP v2 (*.vsp) import */" );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "IMPORT_TYPE", "IMPORT_BEM", IMPORT_BEM, "/*!< Blade Element (*.bem) import */" );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "IMPORT_TYPE", "IMPORT_XSEC_WIRE", IMPORT_XSEC_WIRE, "/*!< XSec as Wireframe (*.hrm) import */" );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "IMPORT_TYPE", "IMPORT_P3D_WIRE", IMPORT_P3D_WIRE, "/*!< Plot3D as Wireframe (*.p3d) import */" );
    assert( r >= 0 );

    doc_struct.comment = "/*! Enum for Surface Intersection export file types. */";

    r = se->RegisterEnum( "INTERSECT_EXPORT_TYPE", doc_struct );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "INTERSECT_EXPORT_TYPE", "INTERSECT_SRF_FILE_NAME", INTERSECT_SRF_FILE_NAME, "/*!< SRF intersection file type */" );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "INTERSECT_EXPORT_TYPE", "INTERSECT_CURV_FILE_NAME", INTERSECT_CURV_FILE_NAME, "/*!< CURV intersection file type */" );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "INTERSECT_EXPORT_TYPE", "INTERSECT_PLOT3D_FILE_NAME", INTERSECT_PLOT3D_FILE_NAME, "/*!< PLOT3D intersection file type */" );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "INTERSECT_EXPORT_TYPE", "INTERSECT_IGES_FILE_NAME", INTERSECT_IGES_FILE_NAME, "/*!< IGES intersection file type */" );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "INTERSECT_EXPORT_TYPE", "INTERSECT_STEP_FILE_NAME", INTERSECT_STEP_FILE_NAME, "/*!< STEP intersection file type */" );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "INTERSECT_EXPORT_TYPE", "INTERSECT_NUM_FILE_NAMES", INTERSECT_NUM_FILE_NAMES, "/*!< Number of surface intersection file types */" );
    assert( r >= 0 );

    doc_struct.comment = "/*! Enum that describes units for length. */";

    r = se->RegisterEnum( "LEN_UNITS", doc_struct );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "LEN_UNITS", "LEN_MM", LEN_MM, "/*!< Millimeter  */" );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "LEN_UNITS", "LEN_CM", LEN_CM, "/*!< Centimeter */" );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "LEN_UNITS", "LEN_M", LEN_M, "/*!< Meter */" );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "LEN_UNITS", "LEN_IN", LEN_IN, "/*!< Inch */" );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "LEN_UNITS", "LEN_FT", LEN_FT, "/*!< Feet */" );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "LEN_UNITS", "LEN_YD", LEN_YD, "/*!< Yard */" );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "LEN_UNITS", "LEN_UNITLESS", LEN_UNITLESS, "/*!< Unitless */" );
    assert( r >= 0 );

    doc_struct.comment = "/*! Enum that describes units for mass. */";

    r = se->RegisterEnum( "MASS_UNIT", doc_struct );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "MASS_UNIT", "MASS_UNIT_G", MASS_UNIT_G, "/*!< Gram */" );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "MASS_UNIT", "MASS_UNIT_KG", MASS_UNIT_KG, "/*!< Kilogram */" );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "MASS_UNIT", "MASS_UNIT_TONNE", MASS_UNIT_TONNE, "/*!< Tonne */" );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "MASS_UNIT", "MASS_UNIT_LBM", MASS_UNIT_LBM, "/*!< Pound-mass */" );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "MASS_UNIT", "MASS_UNIT_SLUG", MASS_UNIT_SLUG, "/*!< Slug */" );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "MASS_UNIT", "MASS_LBFSEC2IN", MASS_LBFSEC2IN, "/*!< Pound-force-second squared per inch  */" );
    assert( r >= 0 );

    doc_struct.comment = "/*! Enum for OpenVSP's various Parm class types. */";

    r = se->RegisterEnum( "PARM_TYPE", doc_struct );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "PARM_TYPE", "PARM_DOUBLE_TYPE", PARM_DOUBLE_TYPE, "/*!< Double Parm type (Parm) */" );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "PARM_TYPE", "PARM_INT_TYPE",    PARM_INT_TYPE, "/*!< Integer Parm type (IntParm) */" );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "PARM_TYPE", "PARM_BOOL_TYPE",   PARM_BOOL_TYPE, "/*!< Bool Parm type (BoolParm) */" );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "PARM_TYPE", "PARM_FRACTION_TYPE", PARM_FRACTION_TYPE, "/*!< Fraction Parm type (FractionParm) */" );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "PARM_TYPE", "PARM_LIMITED_INT_TYPE", PARM_LIMITED_INT_TYPE, "/*!< Limited integer Parm type (LimIntParm) */" );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "PARM_TYPE", "PARM_NOTEQ_TYPE", PARM_NOTEQ_TYPE, "/*!< Not equal Parm type (NotEqParm) */" );
    assert( r >= 0 );

    doc_struct.comment = "/*! Enum used to identify patch types for WireGeoms. */";

    r = se->RegisterEnum( "PATCH_TYPE", doc_struct );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "PATCH_TYPE", "PATCH_NONE", vsp::PATCH_NONE, "/*!< No patch */" );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "PATCH_TYPE", "PATCH_POINT", vsp::PATCH_POINT, "/*!< Point patch type */" );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "PATCH_TYPE", "PATCH_LINE", vsp::PATCH_LINE, "/*!< Line patch type */" );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "PATCH_TYPE", "PATCH_COPY", vsp::PATCH_COPY, "/*!< Copy patch type */" );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "PATCH_TYPE", "PATCH_HALFWAY", vsp::PATCH_HALFWAY, "/*!< Halfway patch type */" );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "PATCH_TYPE", "PATCH_NUM_TYPES", vsp::PATCH_NUM_TYPES, "/*!< Number of patch types */" );
    assert( r >= 0 );

    doc_struct.comment = "/*! Enum for parametric curve types. */";

    r = se->RegisterEnum( "PCURV_TYPE", doc_struct );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "PCURV_TYPE", "LINEAR", LINEAR, "/*!< Linear curve type */" );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "PCURV_TYPE", "PCHIP", PCHIP, "/*!< Piecewise Cubic Hermite Interpolating Polynomial curve type */" );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "PCURV_TYPE", "CEDIT", CEDIT, "/*!< Cubic Bezier curve type */" );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "PCURV_TYPE", "APPROX_CEDIT", APPROX_CEDIT, "/*!< Approximate curve as Cubic Bezier */" );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "PCURV_TYPE", "NUM_PCURV_TYPE", NUM_PCURV_TYPE, "/*!< Number of curve types */" );
    assert( r >= 0 );

    doc_struct.comment = "/*! Enum that describes units for pressure. */";

    r = se->RegisterEnum( "PRES_UNITS", doc_struct );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "PRES_UNITS", "PRES_UNIT_PSF", PRES_UNIT_PSF, "/*!< Pounds per square foot */" );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "PRES_UNITS", "PRES_UNIT_PSI", PRES_UNIT_PSI, "/*!< Pounds per square inch */" );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "PRES_UNITS", "PRES_UNIT_BA", PRES_UNIT_BA, "/*!< Barye */" );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "PRES_UNITS", "PRES_UNIT_PA", PRES_UNIT_PA, "/*!< Pascal */" );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "PRES_UNITS", "PRES_UNIT_KPA", PRES_UNIT_KPA, "/*!< Kilopascal */" );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "PRES_UNITS", "PRES_UNIT_MPA", PRES_UNIT_MPA, "/*!< Megapascal */" );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "PRES_UNITS", "PRES_UNIT_INCHHG", PRES_UNIT_INCHHG, "/*!< Inch of mercury */" );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "PRES_UNITS", "PRES_UNIT_MMHG", PRES_UNIT_MMHG, "/*!< Millimeter of mercury */" );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "PRES_UNITS", "PRES_UNIT_MMH20", PRES_UNIT_MMH20, "/*!< Millimeter of water */" );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "PRES_UNITS", "PRES_UNIT_MB", PRES_UNIT_MB, "/*!< Millibar */" );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "PRES_UNITS", "PRES_UNIT_ATM", PRES_UNIT_ATM, "/*!< Atmosphere */" );
    assert( r >= 0 );

    doc_struct.comment = "/*! Enum for Projected Area boundary option type. */";

    r = se->RegisterEnum( "PROJ_BNDY_TYPE", doc_struct );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "PROJ_BNDY_TYPE", "NO_BOUNDARY", NO_BOUNDARY, "/*!< No boundary */" );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "PROJ_BNDY_TYPE", "SET_BOUNDARY", SET_BOUNDARY, "/*!< Set boundary */" );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "PROJ_BNDY_TYPE", "GEOM_BOUNDARY", GEOM_BOUNDARY, "/*!< Geom boundary */" );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "PROJ_BNDY_TYPE", "NUM_PROJ_BNDY_OPTIONS", NUM_PROJ_BNDY_OPTIONS, "/*!< Number of projected area boundary options */" );
    assert( r >= 0 );

    doc_struct.comment = "/*! Enum for Projected Area direction type. */";

    r = se->RegisterEnum( "PROJ_DIR_TYPE", doc_struct );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "PROJ_DIR_TYPE", "X_PROJ", X_PROJ, "/*!< Project in X axis direction */" );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "PROJ_DIR_TYPE", "Y_PROJ", Y_PROJ, "/*!< Project in Y axis direction */" );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "PROJ_DIR_TYPE", "Z_PROJ", Z_PROJ, "/*!< Project in Z axis direction */" );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "PROJ_DIR_TYPE", "GEOM_PROJ", GEOM_PROJ, "/*!< Project toward a Geom */" );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "PROJ_DIR_TYPE", "VEC_PROJ", VEC_PROJ, "/*!< Project along a 3D vector */" );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "PROJ_DIR_TYPE", "NUM_PROJ_DIR_OPTIONS", NUM_PROJ_DIR_OPTIONS, "/*!< Number of Projected Area direction types */" );
    assert( r >= 0 );

    doc_struct.comment = "/*! Enum for Projected Area target type. */";

    r = se->RegisterEnum( "PROJ_TGT_TYPE", doc_struct );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "PROJ_TGT_TYPE", "SET_TARGET", SET_TARGET, "/*!< Set target type */" );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "PROJ_TGT_TYPE", "GEOM_TARGET", GEOM_TARGET, "/*!< Geom target type */" );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "PROJ_TGT_TYPE", "NUM_PROJ_TGT_OPTIONS", NUM_PROJ_TGT_OPTIONS, "/*!< Number of Projected Area target types */" );
    assert( r >= 0 );

    doc_struct.comment = "/*! Enum used to specify the mode of a propeller Geom. */";

    r = se->RegisterEnum( "PROP_MODE", doc_struct );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "PROP_MODE", "PROP_BLADES", PROP_BLADES, "/*!< Propeller Geom is defined by individual propeller blades */" );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "PROP_MODE", "PROP_BOTH", PROP_BOTH, "/*!< Propeller Geom is defined by blades and a disk together */" );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "PROP_MODE", "PROP_DISK", PROP_DISK, "/*!< Propeller Geom is defined by a flat circular disk */" );
    assert( r >= 0 );

    doc_struct.comment = "/*! Enum for the various propeller blade curve parameterization options. */";

    r = se->RegisterEnum( "PROP_PCURVE", doc_struct );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "PROP_PCURVE", "PROP_CHORD", PROP_CHORD, "/*!< Chord parameterization */" );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "PROP_PCURVE", "PROP_TWIST", PROP_TWIST, "/*!< Twist parameterization */" );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "PROP_PCURVE", "PROP_RAKE", PROP_RAKE, "/*!< Rake parameterization */" );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "PROP_PCURVE", "PROP_SKEW", PROP_SKEW, "/*!< Skew parameterization */" );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "PROP_PCURVE", "PROP_SWEEP", PROP_SWEEP, "/*!< Sweep parameterization */" );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "PROP_PCURVE", "PROP_THICK", PROP_THICK, "/*!< Thickness parameterization */" );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "PROP_PCURVE", "PROP_CLI", PROP_CLI, "/*!< Induced lift coefficient parameterization */" );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "PROP_PCURVE", "PROP_AXIAL", PROP_AXIAL, "/*!< Axial parameterization */" );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "PROP_PCURVE", "PROP_TANGENTIAL", PROP_TANGENTIAL, "/*!< Tangential parameterization */" );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "PROP_PCURVE", "NUM_PROP_PCURVE", NUM_PROP_PCURVE, "/*!< Number of propeller blade curve parameterization options */" );
    assert( r >= 0 );

    doc_struct.comment = "/*! Enum used to indicate manual or component reference type. */";

    r = se->RegisterEnum( "REF_WING_TYPE", doc_struct );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "REF_WING_TYPE", "MANUAL_REF", MANUAL_REF, "/*!< Manual reference */" );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "REF_WING_TYPE", "COMPONENT_REF", COMPONENT_REF, "/*!< Component reference */" );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "REF_WING_TYPE", "NUM_REF_TYPES", NUM_REF_TYPES, "/*!< Number of wing reference types */" );
    assert( r >= 0 );

    doc_struct.comment = "/*! Enum representing the possible data types returned from the ResultsMgr. */";

    r = se->RegisterEnum( "RES_DATA_TYPE", doc_struct );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "RES_DATA_TYPE", "INVALID_TYPE", INVALID_TYPE, "/*!< Invalid results data type */" );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "RES_DATA_TYPE", "INT_DATA", INT_DATA, "/*!< Integer result data type */" );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "RES_DATA_TYPE", "DOUBLE_DATA", DOUBLE_DATA, "/*!< Double result data type */" );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "RES_DATA_TYPE", "STRING_DATA", STRING_DATA, "/*!< String result data type */" );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "RES_DATA_TYPE", "VEC3D_DATA", VEC3D_DATA, "/*!< Vec3d result data type */" );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "RES_DATA_TYPE", "DOUBLE_MATRIX_DATA", DOUBLE_MATRIX_DATA, "/*!< Double matrix result data type */" );
    assert( r >= 0 );

    doc_struct.comment = "/*! Enum representing the possible Geom types returned from the ResultsMgr. */";

    r = se->RegisterEnum( "RES_GEOM_TYPE", doc_struct );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "RES_GEOM_TYPE", "MESH_INDEXED_TRI", MESH_INDEXED_TRI, "/*!< Indexed triangulated mesh Geom type */" );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "RES_GEOM_TYPE", "MESH_SLICE_TRI", MESH_SLICE_TRI, "/*!< Sliced Triangulated mesh Geom type */" );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "RES_GEOM_TYPE", "GEOM_XSECS", GEOM_XSECS, "/*!< GeomXSec Geom type */" );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "RES_GEOM_TYPE", "MESH_INDEX_AND_SLICE_TRI", MESH_INDEX_AND_SLICE_TRI, "/*!< Both indexed and sliced triangulated mesh Geom type */" );
    assert( r >= 0 );

    doc_struct.comment = "/*! Enum that describes units for density. */";

    r = se->RegisterEnum( "RHO_UNITS", doc_struct );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "RHO_UNITS", "RHO_UNIT_SLUG_FT3", RHO_UNIT_SLUG_FT3, "/*!< Slug per cubic foot */" );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "RHO_UNITS", "RHO_UNIT_G_CM3", RHO_UNIT_G_CM3, "/*!< Gram per cubic centimeter */" );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "RHO_UNITS", "RHO_UNIT_KG_M3", RHO_UNIT_KG_M3, "/*!< Kilogram per cubic meter */" );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "RHO_UNITS", "RHO_UNIT_TONNE_MM3", RHO_UNIT_TONNE_MM3, "/*!< Tonne per cubic millimeter */" );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "RHO_UNITS", "RHO_UNIT_LBF_FT3", RHO_UNIT_LBF_FT3, "/*!< Pound-force per cubic foot */" );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "RHO_UNITS", "RHO_UNIT_LBFSEC2_IN4", RHO_UNIT_LBFSEC2_IN4, "/*!< Pound-force-second squared per inch to the fourth */" );
    assert( r >= 0 );

    doc_struct.comment = "/*! Enum for specifying named set types. */";

    r = se->RegisterEnum( "SET_TYPE", doc_struct );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "SET_TYPE", "SET_NONE", SET_NONE, "/*!< None set */" );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "SET_TYPE", "SET_ALL", SET_ALL, "/*!< All set */" );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "SET_TYPE", "SET_SHOWN", SET_SHOWN, "/*!< Shown set */" );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "SET_TYPE", "SET_NOT_SHOWN", SET_NOT_SHOWN, "/*!< Not Shown set */" );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "SET_TYPE", "SET_FIRST_USER", SET_FIRST_USER, "/*!< 1st user named set */" );
    assert( r >= 0 );

    doc_struct.comment = "/*! Enum that identifies the trimmed STEP export representation type. */";

    r = se->RegisterEnum( "STEP_REPRESENTATION", doc_struct );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "STEP_REPRESENTATION", "STEP_SHELL", STEP_SHELL, "/*!< Manifold shell surface STEP file representation */" );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "STEP_REPRESENTATION", "STEP_BREP", STEP_BREP, "/*!< Manifold solid BREP STEP file representation */" );
    assert( r >= 0 );

    doc_struct.comment = "/*! Enum used to identify Parasite Drag Tool sub-surface treatment. */";

    r = se->RegisterEnum( "SUBSURF_INCLUDE", doc_struct );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "SUBSURF_INCLUDE", "SS_INC_TREAT_AS_PARENT", SS_INC_TREAT_AS_PARENT, "/*!< Treat the sub-surface the same as the parent */" );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "SUBSURF_INCLUDE", "SS_INC_SEPARATE_TREATMENT", SS_INC_SEPARATE_TREATMENT, "/*!< Treat the sub-surface separately from the parent */" );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "SUBSURF_INCLUDE", "SS_INC_ZERO_DRAG", SS_INC_ZERO_DRAG, "/*!< No drag contribution for the sub-surface */" );
    assert( r >= 0 );

    doc_struct.comment = "/*! Enum for indicating which part of the parent surface a sub-surfacce is dedfine. */";

    r = se->RegisterEnum( "SUBSURF_INOUT", doc_struct );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "SUBSURF_INOUT", "INSIDE", INSIDE, "/*!< The interior of the sub-surface is its surface */" );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "SUBSURF_INOUT", "OUTSIDE", OUTSIDE, "/*!< The exterior of the sub-surface is its surface */" );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "SUBSURF_INOUT", "NONE", NONE, "/*!< No part of the parent surface belongs to the sub-surface */" );
    assert( r >= 0 );

    doc_struct.comment = "/*! Enum that identifies which surface coordinate is constant for a line sub-surface. */";

    r = se->RegisterEnum( "SUBSURF_LINE_TYPE", doc_struct );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "SUBSURF_LINE_TYPE", "CONST_U", CONST_U, "/*!< Constant U sub-surface */" );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "SUBSURF_LINE_TYPE", "CONST_W", CONST_W, "/*!< Constant W sub-surface */" );
    assert( r >= 0 );

    doc_struct.comment = "/*! Enum for the various sub-surface types. */";

    r = se->RegisterEnum( "SUBSURF_TYPE", doc_struct );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "SUBSURF_TYPE", "SS_LINE", SS_LINE, "/*!< Line sub-surface type */" );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "SUBSURF_TYPE", "SS_RECTANGLE", SS_RECTANGLE, "/*!< Rectangle sub-surface type */" );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "SUBSURF_TYPE", "SS_ELLIPSE", SS_ELLIPSE, "/*!< Ellipse sub-surface type */" );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "SUBSURF_TYPE", "SS_CONTROL", SS_CONTROL, "/*!< Control sub-surface type */" );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "SUBSURF_TYPE", "SS_LINE_ARRAY", SS_LINE_ARRAY, "/*!< Line array sub-surface type */" );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "SUBSURF_TYPE", "SS_FINITE_LINE", SS_FINITE_LINE, "/*!< Finite line sub-surface type */" );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "SUBSURF_TYPE", "SS_NUM_TYPES", SS_NUM_TYPES, "/*!< Number of sub-surface types */" );
    assert( r >= 0 );

    doc_struct.comment = "/*! Enum that represents various symmetry types. */";

    r = se->RegisterEnum( "SYM_FLAG", doc_struct );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "SYM_FLAG", "SYM_XY", SYM_XY, "/*!< Symmetry about the XY plane */" );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "SYM_FLAG", "SYM_XZ", SYM_XZ, "/*!< Symmetry about the XZ plane */" );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "SYM_FLAG", "SYM_YZ", SYM_YZ, "/*!< Symmetry about the YZ plane */" );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "SYM_FLAG", "SYM_ROT_X", SYM_ROT_X, "/*!< Rotational symmetry about the X axis */" );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "SYM_FLAG", "SYM_ROT_Y", SYM_ROT_Y, "/*!< Rotational symmetry about the Y axis */" );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "SYM_FLAG", "SYM_ROT_Z", SYM_ROT_Z, "/*!< Rotational symmetry about the Z axis */" );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "SYM_FLAG", "SYM_PLANAR_TYPES", SYM_PLANAR_TYPES, "/*!< Number of planar symmetry types */" );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "SYM_FLAG", "SYM_NUM_TYPES", SYM_NUM_TYPES, "/*!< Number of symmetry types */" );
    assert( r >= 0 );

    doc_struct.comment = "/*! Symmetry enum for Rounded Rectangle and Edit Curve type XSecs. */";

    r = se->RegisterEnum( "SYM_XSEC_TYPE", doc_struct );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "SYM_XSEC_TYPE", "SYM_NONE", SYM_NONE, "/*!< No symmetry */" );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "SYM_XSEC_TYPE", "SYM_RL", SYM_RL, "/*!< Right-left symmetry: right is mirrored to the left */" );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "SYM_XSEC_TYPE", "SYM_TB", SYM_TB, "/*!< Top-bottom symmetry: top is mirrored to the bottom */" );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "SYM_XSEC_TYPE", "SYM_ALL", SYM_ALL, "/*!< All symmetry with top right as primary */" );

    doc_struct.comment = "/*! Enum that describes units for temperature. */";

    r = se->RegisterEnum( "TEMP_UNITS", doc_struct );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "TEMP_UNITS", "TEMP_UNIT_K", TEMP_UNIT_K, "/*!< Kelvin */" );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "TEMP_UNITS", "TEMP_UNIT_C", TEMP_UNIT_C, "/*!< Celsius */" );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "TEMP_UNITS", "TEMP_UNIT_F", TEMP_UNIT_F, "/*!< Fahrenheit  */" );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "TEMP_UNITS", "TEMP_UNIT_R", TEMP_UNIT_R, "/*!< Rankine  */" );
    assert( r >= 0 );

    doc_struct.comment = "/*! Enum that describes units for velocity. */";

    r = se->RegisterEnum( "VEL_UNITS", doc_struct );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "VEL_UNITS", "V_UNIT_FT_S", V_UNIT_FT_S, "/*!< Feet per second */" );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "VEL_UNITS", "V_UNIT_M_S", V_UNIT_M_S, "/*!< Meter per second */" );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "VEL_UNITS", "V_UNIT_MPH", V_UNIT_MPH, "/*!< Mile per hour */" );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "VEL_UNITS", "V_UNIT_KM_HR", V_UNIT_KM_HR, "/*!< Kilometer per hour */" );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "VEL_UNITS", "V_UNIT_KEAS", V_UNIT_KEAS, "/*!< Knots equivalent airspeed */" );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "VEL_UNITS", "V_UNIT_KTAS", V_UNIT_KTAS, "/*!< Knots true airspeed */" );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "VEL_UNITS", "V_UNIT_MACH", V_UNIT_MACH, "/*!< Mach */" );
    assert( r >= 0 );

    doc_struct.comment = "/*! Enum for 2D drawing types (DXF & SVG). */";

    r = se->RegisterEnum( "VIEW_NUM", doc_struct );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "VIEW_NUM", "VIEW_1", VIEW_1, "/*!< One 2D view */" );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "VIEW_NUM", "VIEW_2HOR", VIEW_2HOR, "/*!< Two horizontal 2D views */" );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "VIEW_NUM", "VIEW_2VER", VIEW_2VER, "/*!< Two vertical 2D views */" );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "VIEW_NUM", "VIEW_4", VIEW_4, "/*!< Four 2D views */" );
    assert( r >= 0 );

    doc_struct.comment = "/*! Enum for describing 2D view rotations (DXF & SVG). */";

    r = se->RegisterEnum( "VIEW_ROT", doc_struct );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "VIEW_ROT", "ROT_0", ROT_0, "/*!< No rotation */" );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "VIEW_ROT", "ROT_90", ROT_90, "/*!< 90 degree rotation */" );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "VIEW_ROT", "ROT_180", ROT_180, "/*!< 180 degree rotation */" );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "VIEW_ROT", "ROT_270", ROT_270, "/*!< 270 degree rotation */" );
    assert( r >= 0 );

    doc_struct.comment = "/*! Enum for describing 2D view types (DXF & SVG). */";

    r = se->RegisterEnum( "VIEW_TYPE", doc_struct );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "VIEW_TYPE", "VIEW_LEFT", VIEW_LEFT, "/*!< Left 2D view type */" );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "VIEW_TYPE", "VIEW_RIGHT", VIEW_RIGHT, "/*!< Right 2D view type */" );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "VIEW_TYPE", "VIEW_TOP", VIEW_TOP, "/*!< Top 2D view type */" );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "VIEW_TYPE", "VIEW_BOTTOM", VIEW_BOTTOM, "/*!< Bottom 2D view type */" );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "VIEW_TYPE", "VIEW_FRONT", VIEW_FRONT, "/*!< Front 2D view type */" );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "VIEW_TYPE", "VIEW_REAR", VIEW_REAR, "/*!< Rear 2D view type */" );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "VIEW_TYPE", "VIEW_NONE", VIEW_NONE, "/*!< No 2D view type */" );
    assert( r >= 0 );

    doc_struct.comment = "/*! Enum that definies the VSPAERO analysis method. */";

    r = se->RegisterEnum( "VSPAERO_ANALYSIS_METHOD", doc_struct );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "VSPAERO_ANALYSIS_METHOD", "VORTEX_LATTICE", VORTEX_LATTICE, "/*!< VSPAERO vortex lattice method */" );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "VSPAERO_ANALYSIS_METHOD", "PANEL", PANEL, "/*!< VSPAERO panel method */" );
    assert( r >= 0 );

    doc_struct.comment = "/*! Enums for VSPAERO unsteady noise calculation types. */";

    r = se->RegisterEnum( "VSPAERO_NOISE_TYPE", doc_struct );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "VSPAERO_NOISE_TYPE", "NOISE_FLYBY", NOISE_FLYBY, "/*!< Set up fly by noise analysis in VSPAERO for PSU-WOPWOP */" );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "VSPAERO_NOISE_TYPE", "NOISE_FOOTPRINT", NOISE_FOOTPRINT, "/*!< Set up footprint noise analysis in VSPAERO for PSU-WOPWOP */" );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "VSPAERO_NOISE_TYPE", "NOISE_STEADY", NOISE_STEADY, "/*!< Set up steady state noise analysis in VSPAERO for PSU-WOPWOP */" );
    assert( r >= 0 );

    doc_struct.comment = "/*! Enums for VSPAERO unsteady noise units. */";

    r = se->RegisterEnum( "VSPAERO_NOISE_UNIT", doc_struct );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "VSPAERO_NOISE_UNIT", "NOISE_SI", NOISE_SI, "/*!< Assume geometry and VSPAERO inputs in SI (m N kg s) for PSU-WOPWOP  */" );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "VSPAERO_NOISE_UNIT", "NOISE_ENGLISH", NOISE_ENGLISH, "/*!< Assume geometry and VSPAERO inputs in english (ft lbf slug s) units, will convert to SI (m N kg s) for PSU-WOPWOP */" );
    assert( r >= 0 );

    doc_struct.comment = "/*! Enum for the types of preconditioner used in VSPAERO. */";

    r = se->RegisterEnum( "VSPAERO_PRECONDITION", doc_struct );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "VSPAERO_PRECONDITION", "PRECON_MATRIX", PRECON_MATRIX, "/*!< Matrix preconditioner */" );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "VSPAERO_PRECONDITION", "PRECON_JACOBI", PRECON_JACOBI, "/*!< Jacobi preconditioner */" );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "VSPAERO_PRECONDITION", "PRECON_SSOR", PRECON_SSOR, "/*!< Symmetric successive over-relaxation preconditioner */" );
    assert( r >= 0 );

    doc_struct.comment = "/*! Enum for the types of VSPAERO stability analyses. */";

    r = se->RegisterEnum( "VSPAERO_STABILITY_TYPE", doc_struct );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "VSPAERO_STABILITY_TYPE", "STABILITY_OFF", STABILITY_OFF, "/*!< No stability analysis (off) */" );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "VSPAERO_STABILITY_TYPE", "STABILITY_DEFAULT", STABILITY_DEFAULT, "/*!< Normal steady stability analysis */" );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "VSPAERO_STABILITY_TYPE", "STABILITY_P_ANALYSIS", STABILITY_P_ANALYSIS, "/*!< P stability analysis */" );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "VSPAERO_STABILITY_TYPE", "STABILITY_Q_ANALYSIS", STABILITY_Q_ANALYSIS, "/*!< Q stability analysis */" );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "VSPAERO_STABILITY_TYPE", "STABILITY_R_ANALYSIS", STABILITY_R_ANALYSIS, "/*!< R stability analysis */" );
    assert( r >= 0 );
    //r = se->RegisterEnumValue( "VSPAERO_STABILITY_TYPE", "STABILITY_HEAVE", STABILITY_HEAVE );
    //assert( r >= 0 );
    //r = se->RegisterEnumValue( "VSPAERO_STABILITY_TYPE", "STABILITY_IMPULSE", STABILITY_IMPULSE );
    //assert( r >= 0 );

    doc_struct.comment = "/*! Enum for the VSPAERO stall modeling options (Cl Max VSPAERO input). */";

    r = se->RegisterEnum( "VSPAERO_CLMAX_TYPE", doc_struct );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "VSPAERO_CLMAX_TYPE", "CLMAX_OFF", CLMAX_OFF, "/*!< Stall modeling off (Cl Max = 0) */" );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "VSPAERO_CLMAX_TYPE", "CLMAX_2D", CLMAX_2D, "/*!< 2D Cl Max stall modeling with user defined value */" );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "VSPAERO_CLMAX_TYPE", "CLMAX_CARLSON", CLMAX_CARLSON, "/*!< Carlson's Pressure Correlation stal model (Cl Max = 999) */" );
    assert( r >= 0 );

    doc_struct.comment = "/*! Enum that is used to describe surfaces in CFD Mesh. */";

    r = se->RegisterEnum( "VSP_SURF_CFD_TYPE", doc_struct );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "VSP_SURF_CFD_TYPE", "CFD_NORMAL", CFD_NORMAL, "/*!< Normal CFD Mesh surface */" );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "VSP_SURF_CFD_TYPE", "CFD_NEGATIVE", CFD_NEGATIVE, "/*!< Negative volume CFD Mesh surface */" );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "VSP_SURF_CFD_TYPE", "CFD_TRANSPARENT", CFD_TRANSPARENT, "/*!< Transparent CFD Mesh surface */" );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "VSP_SURF_CFD_TYPE", "CFD_STRUCTURE", CFD_STRUCTURE, "/*!< FEA structure CFD Mesh surface */" );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "VSP_SURF_CFD_TYPE", "CFD_STIFFENER", CFD_STIFFENER, "/*!< FEA stiffener CFD Mesh surface */" );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "VSP_SURF_CFD_TYPE", "CFD_NUM_TYPES", CFD_NUM_TYPES, "/*!< Number of CFD Mesh surface types */" );
    assert( r >= 0 );

    doc_struct.comment = "/*! Enum for the different surface types in OpenVSP. */";

    r = se->RegisterEnum( "VSP_SURF_TYPE", doc_struct );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "VSP_SURF_TYPE", "NORMAL_SURF", NORMAL_SURF, "/*!< Normal VSP surface */" );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "VSP_SURF_TYPE", "WING_SURF", WING_SURF, "/*!< Wing VSP surface */" );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "VSP_SURF_TYPE", "DISK_SURF", DISK_SURF, "/*!< Disk VSP surface */" );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "VSP_SURF_TYPE", "PROP_SURF", PROP_SURF, "/*!< Propeller VSP surface */" );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "VSP_SURF_TYPE", "NUM_SURF_TYPES", NUM_SURF_TYPES, "/*!< Number of VSP surface types */" );
    assert( r >= 0 );

    doc_struct.comment = "/*! Enum used to identify the type of wing blending between XSecs. */";

    r = se->RegisterEnum( "WING_BLEND", doc_struct );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "WING_BLEND", "BLEND_FREE", vsp::BLEND_FREE, "/*!< Free blending */" );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "WING_BLEND", "BLEND_ANGLES", vsp::BLEND_ANGLES, "/*!< Blend based on angles (sweep & dihedral) */" );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "WING_BLEND", "BLEND_MATCH_IN_LE_TRAP", vsp::BLEND_MATCH_IN_LE_TRAP, "/*!< Match inboard leading edge trapezoid */" );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "WING_BLEND", "BLEND_MATCH_IN_TE_TRAP", vsp::BLEND_MATCH_IN_TE_TRAP, "/*!< Match inboard trailing edge trapezoid */" );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "WING_BLEND", "BLEND_MATCH_OUT_LE_TRAP", vsp::BLEND_MATCH_OUT_LE_TRAP, "/*!< Match outboard leading edge trapezoid */" );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "WING_BLEND", "BLEND_MATCH_OUT_TE_TRAP", vsp::BLEND_MATCH_OUT_TE_TRAP, "/*!< Match outboard trailing edge trapezoid */" );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "WING_BLEND", "BLEND_MATCH_IN_ANGLES", vsp::BLEND_MATCH_IN_ANGLES, "/*!< Match inboard angles */" );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "WING_BLEND", "BLEND_MATCH_LE_ANGLES", vsp::BLEND_MATCH_LE_ANGLES, "/*!< Match leading edge angles */" );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "WING_BLEND", "BLEND_NUM_TYPES", vsp::BLEND_NUM_TYPES, "/*!< Number of blending types */" );
    assert( r >= 0 );

    doc_struct.comment = "/*! Enum for controlling wing section planform parameter control and linking. */";

    r = se->RegisterEnum( "WING_DRIVERS", doc_struct );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "WING_DRIVERS", "AR_WSECT_DRIVER", vsp::AR_WSECT_DRIVER, "/*!< Aspect ratio driver */" );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "WING_DRIVERS", "SPAN_WSECT_DRIVER", vsp::SPAN_WSECT_DRIVER, "/*!< Span driver */" );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "WING_DRIVERS", "AREA_WSECT_DRIVER", vsp::AREA_WSECT_DRIVER, "/*!< Area driver */" );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "WING_DRIVERS", "TAPER_WSECT_DRIVER", vsp::TAPER_WSECT_DRIVER, "/*!< Taper driver */" );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "WING_DRIVERS", "AVEC_WSECT_DRIVER", vsp::AVEC_WSECT_DRIVER, "/*!< Average chord driver */" );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "WING_DRIVERS", "ROOTC_WSECT_DRIVER", vsp::ROOTC_WSECT_DRIVER, "/*!< Root chord driver */" );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "WING_DRIVERS", "TIPC_WSECT_DRIVER", vsp::TIPC_WSECT_DRIVER, "/*!< Tip chord driver */" );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "WING_DRIVERS", "SECSWEEP_WSECT_DRIVER", vsp::SECSWEEP_WSECT_DRIVER, "/*!< Section sweep driver */" );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "WING_DRIVERS", "NUM_WSECT_DRIVER", vsp::NUM_WSECT_DRIVER, "/*!< Number of wing section drivers */" );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "WING_DRIVERS", "SWEEP_WSECT_DRIVER", vsp::SWEEP_WSECT_DRIVER, "/*!< Sweepo driver */" );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "WING_DRIVERS", "SWEEPLOC_WSECT_DRIVER", vsp::SWEEPLOC_WSECT_DRIVER, "/*!< Sweep location driver */" );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "WING_DRIVERS", "SECSWEEPLOC_WSECT_DRIVER", vsp::SECSWEEPLOC_WSECT_DRIVER, "/*!< Secondary sweep location driver */" );
    assert( r >= 0 );

    doc_struct.comment = "/*! Enum that identifies the working XDDM type. */";

    r = se->RegisterEnum( "XDDM_QUANTITY_TYPE", doc_struct );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "XDDM_QUANTITY_TYPE", "XDDM_VAR", XDDM_VAR, "/*!< Variable XDDM type */" );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "XDDM_QUANTITY_TYPE", "XDDM_CONST", XDDM_CONST, "/*!< Constant XDDM type */" );
    assert( r >= 0 );

    doc_struct.comment = "/*! Enum for modifying XSec through closure types. */";

    r = se->RegisterEnum( "XSEC_CLOSE_TYPE", doc_struct );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "XSEC_CLOSE_TYPE", "CLOSE_NONE", CLOSE_NONE, "/*!< No closure */" );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "XSEC_CLOSE_TYPE", "CLOSE_SKEWLOW", CLOSE_SKEWLOW, "/*!< Skew lower closure */" );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "XSEC_CLOSE_TYPE", "CLOSE_SKEWUP", CLOSE_SKEWUP, "/*!< Skew upper closure */" );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "XSEC_CLOSE_TYPE", "CLOSE_SKEWBOTH", CLOSE_SKEWBOTH, "/*!< Skew both closure */" );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "XSEC_CLOSE_TYPE", "CLOSE_EXTRAP", CLOSE_EXTRAP, "/*!< Extrapolate closure */" );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "XSEC_CLOSE_TYPE", "CLOSE_NUM_TYPES", CLOSE_NUM_TYPES, "/*!< Number of XSec closure types */" );
    assert( r >= 0 );

    doc_struct.comment = "/*! Enum that identifies the various OpenVSP XSecCurve types. */";

    r = se->RegisterEnum( "XSEC_CRV_TYPE", doc_struct );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "XSEC_CRV_TYPE", "XS_UNDEFINED", XS_UNDEFINED, "/*!< Undefined XSec */" );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "XSEC_CRV_TYPE", "XS_POINT", XS_POINT, "/*!< Point XSec */" );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "XSEC_CRV_TYPE", "XS_CIRCLE", XS_CIRCLE, "/*!< Circle XSec */" );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "XSEC_CRV_TYPE", "XS_ELLIPSE", XS_ELLIPSE, "/*!< Ellipse XSec */" );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "XSEC_CRV_TYPE", "XS_SUPER_ELLIPSE", XS_SUPER_ELLIPSE, "/*!< Super ellipse XSec */" );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "XSEC_CRV_TYPE", "XS_ROUNDED_RECTANGLE", XS_ROUNDED_RECTANGLE, "/*!< Rounded rectangle XSec */" );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "XSEC_CRV_TYPE", "XS_GENERAL_FUSE", XS_GENERAL_FUSE, "/*!< General fuselage XSec */" );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "XSEC_CRV_TYPE", "XS_FILE_FUSE", XS_FILE_FUSE, "/*!< Fuselage file XSec */" );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "XSEC_CRV_TYPE", "XS_FOUR_SERIES", XS_FOUR_SERIES, "/*!< Four series XSec */" );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "XSEC_CRV_TYPE", "XS_SIX_SERIES", XS_SIX_SERIES, "/*!< Six series XSec */" );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "XSEC_CRV_TYPE", "XS_BICONVEX", XS_BICONVEX, "/*!< Biconvex XSec */" );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "XSEC_CRV_TYPE", "XS_WEDGE", XS_WEDGE, "/*!< Wedge XSec */" );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "XSEC_CRV_TYPE", "XS_EDIT_CURVE", XS_EDIT_CURVE, "/*!< Generic Edit Curve XSec */" );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "XSEC_CRV_TYPE", "XS_FILE_AIRFOIL", XS_FILE_AIRFOIL, "/*!< Airfoil file XSec */" );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "XSEC_CRV_TYPE", "XS_CST_AIRFOIL", XS_CST_AIRFOIL, "/*!< CST airfoil XSec */" );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "XSEC_CRV_TYPE", "XS_VKT_AIRFOIL", XS_VKT_AIRFOIL, "/*!< VKT airfoil XSec */" );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "XSEC_CRV_TYPE", "XS_FOUR_DIGIT_MOD", XS_FOUR_DIGIT_MOD, "/*!< Four digit modified XSec */" );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "XSEC_CRV_TYPE", "XS_FIVE_DIGIT", XS_FIVE_DIGIT, "/*!< Five digit XSec */" );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "XSEC_CRV_TYPE", "XS_FIVE_DIGIT_MOD", XS_FIVE_DIGIT_MOD, "/*!< Five digit modified XSec */" );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "XSEC_CRV_TYPE", "XS_ONE_SIX_SERIES", XS_ONE_SIX_SERIES, "/*!< One six series XSec */" );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "XSEC_CRV_TYPE", "XS_NUM_TYPES", XS_NUM_TYPES, "/*!< Number of XSec types */" );
    assert( r >= 0 );

    doc_struct.comment = "/*! Enum for XSec drivers. */";

    r = se->RegisterEnum( "XSEC_DRIVERS", doc_struct );  // TODO: improve these comments
    assert( r >= 0 );
    r = se->RegisterEnumValue( "XSEC_DRIVERS", "WIDTH_XSEC_DRIVER", WIDTH_XSEC_DRIVER, "/*!< Width driver */" );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "XSEC_DRIVERS", "AREA_XSEC_DRIVER", AREA_XSEC_DRIVER, "/*!< Area driver */" );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "XSEC_DRIVERS", "HEIGHT_XSEC_DRIVER", HEIGHT_XSEC_DRIVER, "/*!< Height driver */" );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "XSEC_DRIVERS", "HWRATIO_XSEC_DRIVER", HWRATIO_XSEC_DRIVER, "/*!< Height/width ratio driver */" );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "XSEC_DRIVERS", "NUM_XSEC_DRIVER", NUM_XSEC_DRIVER, "/*!< Number of XSec drivers */" );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "XSEC_DRIVERS", "CIRCLE_NUM_XSEC_DRIVER", CIRCLE_NUM_XSEC_DRIVER, "/*!< Number of Circle XSec drivers */" );
    assert( r >= 0 );

    doc_struct.comment = "/*! Enum for XSec side types. */";

    r = se->RegisterEnum( "XSEC_SIDES_TYPE", doc_struct );  // TODO: improve these comments
    assert( r >= 0 );
    r = se->RegisterEnumValue( "XSEC_SIDES_TYPE", "XSEC_BOTH_SIDES", XSEC_BOTH_SIDES, "/*!< Both sides */" );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "XSEC_SIDES_TYPE", "XSEC_LEFT_SIDE", XSEC_LEFT_SIDE, "/*!< Left side */" );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "XSEC_SIDES_TYPE", "XSEC_RIGHT_SIDE", XSEC_RIGHT_SIDE, "/*!< Right side */" );
    assert( r >= 0 );

    doc_struct.comment = "/*! Enum used to identify XSec trim type. */";

    r = se->RegisterEnum( "XSEC_TRIM_TYPE", doc_struct );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "XSEC_TRIM_TYPE", "TRIM_NONE", TRIM_NONE, "/*!< No trimming */" );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "XSEC_TRIM_TYPE", "TRIM_X", TRIM_X, "/*!< Trim XSec by X */" );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "XSEC_TRIM_TYPE", "TRIM_THICK", TRIM_THICK, "/*!< Trim XSec by thickness */" );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "XSEC_TRIM_TYPE", "TRIM_NUM_TYPES", TRIM_NUM_TYPES, "/*!< Number of trimming types */" );
    assert( r >= 0 );

    doc_struct.comment = "/*! Enum for the various XSec types in OpenVSP. */";

    r = se->RegisterEnum( "XSEC_TYPE", doc_struct );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "XSEC_TYPE", "XSEC_FUSE", XSEC_FUSE, "/*!< Fuselage XSec Geom */" );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "XSEC_TYPE", "XSEC_STACK", XSEC_STACK, "/*!< Stack XSec Geom */" );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "XSEC_TYPE", "XSEC_WING", XSEC_WING, "/*!< Wing XSec Geom */" );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "XSEC_TYPE", "XSEC_CUSTOM", XSEC_CUSTOM, "/*!< Custom XSec Geom */" );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "XSEC_TYPE", "XSEC_PROP", XSEC_PROP, "/*!< Propeller XSec Geom */" );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "XSEC_TYPE", "XSEC_NUM_TYPES", XSEC_NUM_TYPES, "/*!< Number of XSec types */" );
    assert( r >= 0 );

    doc_struct.comment = "/*! Enum for XSec width shift. */";

    r = se->RegisterEnum( "XSEC_WIDTH_SHIFT", doc_struct ); // TODO: improve these comments
    assert( r >= 0 );
    r = se->RegisterEnumValue( "XSEC_WIDTH_SHIFT", "XS_SHIFT_LE", XS_SHIFT_LE, "/*!< Shift leading edge */" );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "XSEC_WIDTH_SHIFT", "XS_SHIFT_MID", XS_SHIFT_MID, "/*!< Shift midpoint */" );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "XSEC_WIDTH_SHIFT", "XS_SHIFT_TE", XS_SHIFT_TE, "/*!< Shift trailing edge */" );
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
    asDocInfo doc_struct;

    string group = "vec3d";
    doc_struct.group = group.c_str();

    string group_description = R"(
    \brief API functions that utilize the vec3d class are grouped here. For details of the class, including member functions, see vec3d. \n\n
    \ref index "Click here to return to the main page" )";
    se->AddGroup( group.c_str(), "Vec3D Functions", group_description.c_str() );

    doc_struct.comment = R"(
  //!  A class for representing 3D vectors. 
  /*!
    vec3d is typically used to describe coordinate points and vectors in 3D space.
    All 3 elements in the vector are of type double.
  */)";
    //==== Register vec3d Object =====//
    int r = se->RegisterObjectType( "vec3d", sizeof( vec3d ), asOBJ_VALUE | asOBJ_POD | asOBJ_APP_CLASS_CA, doc_struct );
    assert( r >= 0 );

    //==== Register the vec3d Constructors  ====//
    doc_struct.comment = R"(
  /*!
    Initialize a vec3d object with X, Y, and Z set to 0.
  */)"; // Not shown in header
    r = se->RegisterObjectBehaviour( "vec3d", asBEHAVE_CONSTRUCT, "void f()", asFUNCTION( Vec3dDefaultConstructor ), asCALL_CDECL_OBJLAST, doc_struct );
    assert( r >= 0 );

    doc_struct.comment = R"(
  /*!
    Initialize a vec3d object with input X, Y, and Z values. 
  */)"; // Not shown in header
    r = se->RegisterObjectBehaviour( "vec3d", asBEHAVE_CONSTRUCT, "void f(double, double, double)", asFUNCTION( Vec3dInitConstructor ), asCALL_CDECL_OBJLAST, doc_struct );
    assert( r >= 0 );

    doc_struct.comment = R"(
  /*!
    Initialize a vec3d with another vec3d. Equivalent to the "=" operator.
  */)"; // Not shown in header
    r = se->RegisterObjectBehaviour( "vec3d", asBEHAVE_CONSTRUCT, "void f(const vec3d &in)", asFUNCTION( Vec3dCopyConstructor ), asCALL_CDECL_OBJLAST, doc_struct );
    assert( r >= 0 );

    //==== Register the vec3d Methods  ====//
    doc_struct.comment = R"(
  /*!
    Indexing operator for vec3d. Supported indexes are 0 (X), 1 (Y), and 2 (Z).
  */)";
    r = se->RegisterObjectMethod( "vec3d", "double& opIndex(int) const", asMETHODPR( vec3d, operator[], ( int ), double& ), asCALL_THISCALL, doc_struct );
    assert( r >= 0 );

    doc_struct.comment = R"(
  /*!
    Get the X coordinate (index 0) of the vec3d
    \code{cpp}
    vec3d a();                                // Default Constructor

    a.set_xyz( 2.0, 4.0, 6.0 );

    Print( "a.x() = ", false );
    Print( a.x() );

    Print( "a[0]= ", false );
    Print( a[0] );
    \endcode
    \return X value
  */)";
    r = se->RegisterObjectMethod( "vec3d", "double x() const", asMETHOD( vec3d, x ), asCALL_THISCALL, doc_struct );
    assert( r >= 0 );

    doc_struct.comment = R"(
  /*!
    Get the Y coordinate (index 1) of the vec3d
    \code{cpp}
    vec3d a();                                // Default Constructor

    a.set_xyz( 2.0, 4.0, 6.0 );

    Print( "a.y() = ", false );
    Print( a.y() );

    Print( "a[1]= ", false );
    Print( a[1] );
    \endcode
    \return Y value
  */)";
    r = se->RegisterObjectMethod( "vec3d", "double y() const", asMETHOD( vec3d, y ), asCALL_THISCALL, doc_struct );
    assert( r >= 0 );

    doc_struct.comment = R"(
  /*!
    Get the Z coordinate (index 2) of the vec3d
    \code{cpp}
    vec3d a();                                // Default Constructor

    a.set_xyz( 2.0, 4.0, 6.0 );

    Print( "a.z() = ", false );
    Print( a.z() );

    Print( "a[2]= ", false );
    Print( a[2] );
    \endcode
    \return Z value
  */)";
    r = se->RegisterObjectMethod( "vec3d", "double z() const", asMETHOD( vec3d, z ), asCALL_THISCALL, doc_struct );
    assert( r >= 0 );

    doc_struct.comment = R"(
  /*!
    Set all three elements of the vec3d vector
    \code{.cpp}
    //==== Test Vec3d ====//
    vec3d a();                                // Default Constructor

    a.set_xyz( 2.0, 4.0, 6.0 );
    \endcode
    \param [in] x New X value
    \param [in] y New Y value
    \param [in] z New Z value
    \return Updated vec3d
  */)";
    r = se->RegisterObjectMethod( "vec3d", "vec3d& set_xyz(double x, double y, double z)", asMETHOD( vec3d, set_xyz ), asCALL_THISCALL, doc_struct );
    assert( r >= 0 );

    doc_struct.comment = R"(
  /*!
    Set the X coordinate (index 0) of the vec3d
    \code{.cpp}
    //==== Test Vec3d ====//
    vec3d a();                                // Default Constructor

    a.set_x( 2.0 );
    \endcode
    \param [in] x New X value
    \return Updated vec3d
  */)";

    r = se->RegisterObjectMethod( "vec3d", "vec3d& set_x(double x)", asMETHOD( vec3d, set_x ), asCALL_THISCALL, doc_struct );
    assert( r >= 0 );

    doc_struct.comment = R"(
  /*!
    Set the Y coordinate (index 1) of the vec3d
    \code{.cpp}
    //==== Test Vec3d ====//
    vec3d a();                                // Default Constructor

    a.set_y( 4.0 );
    \endcode
    \param [in] y New Y value
    \return Updated vec3d
  */)";

    r = se->RegisterObjectMethod( "vec3d", "vec3d& set_y(double y)", asMETHOD( vec3d, set_y ), asCALL_THISCALL, doc_struct );
    assert( r >= 0 );

    doc_struct.comment = R"(
  /*!
    Set the z coordinate (index 2) of the vec3d
    \code{.cpp}
    //==== Test Vec3d ====//
    vec3d a();                                // Default Constructor

    a.set_z( 6.0 );
    \endcode
    \param [in] z in double new z value
    \return vec3d result
  */)";

    r = se->RegisterObjectMethod( "vec3d", "vec3d& set_z(double z)", asMETHOD( vec3d, set_z ), asCALL_THISCALL, doc_struct );
    assert( r >= 0 );

    doc_struct.comment = R"(
  /*!
    Rotate the vec3d about the X axis. 
    \f{equation}{x = cos \textunderscore alpha * x + sin \textunderscore alpha * z\f}
    \f{equation}{y = -sin \textunderscore alpha * old_y + cos \textunderscore alpha * z\f}
    \code{.cpp}
    //==== Test Vec3d ====//
    vec3d a();                                // Default Constructor
    float PI = 3.14;

    a.set_xyz( 1.0, 0.0, 0.0 );

    a.rotate_x( cos( 0.5 * PI ), sin( 0.5 * PI ) );
    \endcode
    \param [in] cos_alpha Cosine of rotation angle
    \param [in] sin_alpha Sine of rotation angle
  */)";

    r = se->RegisterObjectMethod( "vec3d", "void rotate_x(double cos_alpha, double sin_alpha)", asMETHOD( vec3d, rotate_x ), asCALL_THISCALL, doc_struct );
    assert( r >= 0 );

    doc_struct.comment = R"(
  /*!
    Rotate the vec3d about the Y axis. 
    \f{equation}{x = cos \textunderscore alpha * x - sin \textunderscore alpha * z\f}
    \f{equation}{z =  sin \textunderscore alpha * old_x + cos \textunderscore alpha * z\f}
    \code{.cpp}
    //==== Test Vec3d ====//
    vec3d a();                                // Default Constructor
    float PI = 3.14;

    a.set_xyz( 1.0, 0.0, 0.0 );

    a.rotate_y( cos( 0.5 * PI ), sin( 0.5 * PI ) );
    \endcode
    \param [in] cos_alpha Cosine of rotation angle
    \param [in] sin_alpha Sine of rotation angle
  */)";

    r = se->RegisterObjectMethod( "vec3d", "void rotate_y(double cos_alpha, double sin_alpha)", asMETHOD( vec3d, rotate_y ), asCALL_THISCALL, doc_struct );
    assert( r >= 0 );

    doc_struct.comment = R"(
  /*!
    Rotate the vec3d about the Z axis.
    \f{equation}{x = cos \textunderscore alpha * x + sin \textunderscore alpha * y\f}
    \f{equation}{y = -sin \textunderscore alpha * old_x + cos \textunderscore alpha * y\f}
    \code{.cpp}
    //==== Test Vec3d ====//
    vec3d a();                                // Default Constructor
    float PI = 3.14;

    a.set_xyz( 1.0, 0.0, 0.0 );

    a.rotate_z( cos( 0.5 * PI ), sin( 0.5 * PI ) );
    \endcode
    \param [in] cos_alpha Cosine of rotation angle
    \param [in] sin_alpha Sine of rotation angle
  */)";

    r = se->RegisterObjectMethod( "vec3d", "void rotate_z(double cos_alpha, double sin_alpha)", asMETHOD( vec3d, rotate_z ), asCALL_THISCALL, doc_struct );
    assert( r >= 0 );

    doc_struct.comment = R"(
/*!
    Scale the X coordinate of the vec3d
    \code{.cpp}
    //==== Test Vec3d ====//
    vec3d a();                                // Default Constructor

    //===== Test Scale ====//
    a.set_xyz( 2.0, 2.0, 2.0 );

    a.scale_x( 2.0 );
    \endcode
    \param [in] scale Scaling factor for the X value
*/)";
    r = se->RegisterObjectMethod( "vec3d", "void scale_x(double scale)", asMETHOD( vec3d, scale_x ), asCALL_THISCALL, doc_struct );
    assert( r >= 0 );

    doc_struct.comment = R"(
/*!
    Scale the Y coordinate of the vec3d
    \code{.cpp}
    //==== Test Vec3d ====//
    vec3d a();                                // Default Constructor

    //===== Test Scale ====//
    a.set_xyz( 2.0, 2.0, 2.0 );

    a.scale_y( 2.0 );
    \endcode
    \param [in] scale Scaling factor for the Y value
*/)";
    r = se->RegisterObjectMethod( "vec3d", "void scale_y(double scale)", asMETHOD( vec3d, scale_y ), asCALL_THISCALL, doc_struct );
    assert( r >= 0 );
    
    doc_struct.comment = R"(
/*!
    Scale the Z coordinate of the vec3d
    \code{.cpp}
    //==== Test Vec3d ====//
    vec3d a();                                // Default Constructor

    //===== Test Scale ====//
    a.set_xyz( 2.0, 2.0, 2.0 );

    a.scale_z( 2.0 );
    \endcode
    \param [in] scale Scaling factor for the Z value
*/)";
    r = se->RegisterObjectMethod( "vec3d", "void scale_z(double scale)", asMETHOD( vec3d, scale_z ), asCALL_THISCALL, doc_struct );
    assert( r >= 0 );

    doc_struct.comment = R"(
/*!
    Offset the X coordinate of the vec3d
    \code{.cpp}
    //==== Test Vec3d ====//
    vec3d a();                                // Default Constructor

    //===== Test Offset ====//
    a.set_xyz( 2.0, 2.0, 2.0 );

    a.offset_x( 10.0 );
    \endcode
    \param [in] offset Offset for the X value
*/)";
    r = se->RegisterObjectMethod( "vec3d", "void offset_x(double offset)", asMETHOD( vec3d, offset_x ), asCALL_THISCALL, doc_struct );
    assert( r >= 0 );
    
    doc_struct.comment = R"(
/*!
    Offset the Y coordinate of the vec3d
    \code{.cpp}
    //==== Test Vec3d ====//
    vec3d a();                                // Default Constructor

    //===== Test Offset ====//
    a.set_xyz( 2.0, 2.0, 2.0 );

    a.offset_y( 10.0 );
    \endcode
    \param [in] offset Offset for the Y value
*/)";
    r = se->RegisterObjectMethod( "vec3d", "void offset_y(double offset)", asMETHOD( vec3d, offset_y ), asCALL_THISCALL, doc_struct );
    assert( r >= 0 );
    
    doc_struct.comment = R"(
/*!
    Offset the Z coordinate of the vec3d
    \code{.cpp}
    //==== Test Vec3d ====//
    vec3d a();                                // Default Constructor

    //===== Test Offset ====//
    a.set_xyz( 2.0, 2.0, 2.0 );

    a.offset_z( 10.0 );
    \endcode
    \param [in] offset Offset for the Z value
*/)";
    r = se->RegisterObjectMethod( "vec3d", "void offset_z(double offset)", asMETHOD( vec3d, offset_z ), asCALL_THISCALL, doc_struct );
    assert( r >= 0 );
    
    doc_struct.comment = R"(
/*!
    Rotate the vec3d about the Z axis assuming zero X coordinate value 
    \param [in] cos_alpha Cosine of rotation angle
    \param [in] sin_alpha Sine of rotation angle
*/)"; // TODO: Example
    r = se->RegisterObjectMethod( "vec3d", "void rotate_z_zero_x(double cos_alpha, double sin_alpha)", asMETHOD( vec3d, rotate_z_zero_x ), asCALL_THISCALL, doc_struct );
    assert( r >= 0 );
    
    doc_struct.comment = R"(
/*!
    Rotate the vec3d about the Z axis assuming zero Y coordinate value 
    \param [in] cos_alpha Cosine of rotation angle
    \param [in] sin_alpha Sine of rotation angle
*/)"; // TODO: Example
    r = se->RegisterObjectMethod( "vec3d", "void rotate_z_zero_y(double cos_alpha, double sin_alpha)", asMETHOD( vec3d, rotate_z_zero_y ), asCALL_THISCALL, doc_struct );
    assert( r >= 0 );
    
    doc_struct.comment = R"(
/*!
    Reflect the vec3d across the XY plane
    \code{.cpp}
    //==== Test Vec3d ====//
    vec3d a(), b();                                // Default Constructor

    //===== Test Reflect ====//
    a.set_xyz( 1.0, 2.0, 3.0 );

    b = a.reflect_xy();
    \endcode
    \return Reflected vec3d
*/)";
    r = se->RegisterObjectMethod( "vec3d", "vec3d reflect_xy()", asMETHOD( vec3d, reflect_xy ), asCALL_THISCALL, doc_struct );
    assert( r >= 0 );
    
    doc_struct.comment = R"(
/*!
    Reflect the vec3d across the XZ plane
    \code{.cpp}
    //==== Test Vec3d ====//
    vec3d a(), b();                                // Default Constructor

    //===== Test Reflect ====//
    a.set_xyz( 1.0, 2.0, 3.0 );

    b = a.reflect_xz();
    \endcode
    \return Reflected vec3d
*/)";
    r = se->RegisterObjectMethod( "vec3d", "vec3d reflect_xz()", asMETHOD( vec3d, reflect_xz ), asCALL_THISCALL, doc_struct );
    assert( r >= 0 );
    
    doc_struct.comment = R"(
/*!
    Reflect the vec3d across the YZ plane
    \code{.cpp}
    //==== Test Vec3d ====//
    vec3d a(), b();                                // Default Constructor

    //===== Test Reflect ====//
    a.set_xyz( 1.0, 2.0, 3.0 );

    b = a.reflect_yz();
    \endcode
    \return Reflected vec3d
*/)";
    r = se->RegisterObjectMethod( "vec3d", "vec3d reflect_yz()", asMETHOD( vec3d, reflect_yz ), asCALL_THISCALL, doc_struct );
    assert( r >= 0 );

    doc_struct.comment = R"(
  /*!
    Addition operator for two vec3d objects, performed by the addition of each corresponding component
    \code{.cpp}
    vec3d a(), b();                                // Default Constructor

    a.set_xyz( 1.0, 2.0, 3.0 );
    b.set_xyz( 4.0, 5.0, 6.0 );

    vec3d c = a + b;

    Print( "a + b = ", false );

    Print( c );
    \endcode
  */)";
    r = se->RegisterObjectMethod( "vec3d", "vec3d opAdd(const vec3d &in) const", asFUNCTIONPR( operator+, ( const vec3d&, const vec3d& ), vec3d ), asCALL_CDECL_OBJFIRST, doc_struct );
    assert( r >= 0 );

    doc_struct.comment = R"(
  /*!
    Subtraction operator for two vec3d objects, performed by the subtraction of each corresponding component    \code{.cpp}
    \code{.cpp}
    vec3d a(), b();                                // Default Constructor

    a.set_xyz( 1.0, 2.0, 3.0 );
    b.set_xyz( 4.0, 5.0, 6.0 );

    vec3d c = a - b;

    Print( "a - b = ", false );

    Print( c );
    \endcode
  */)";
    r = se->RegisterObjectMethod( "vec3d", "vec3d opSub(const vec3d &in) const", asFUNCTIONPR( operator-, ( const vec3d&, const vec3d& ), vec3d ), asCALL_CDECL_OBJFIRST, doc_struct );
    assert( r >= 0 );

    doc_struct.comment = R"(
  /*!
    Scalar multiplication operator for a vec3d, performed by the multiplication of each vec3d component and the scalar
    \code{.cpp}
    vec3d a();                                // Default Constructor

    a.set_xyz( 1.0, 2.0, 3.0 );

    double b = 1.5;

    vec3d c = a * b;

    Print( "a * b = ", false );

    Print( c );
    \endcode
  */)";
    r = se->RegisterObjectMethod( "vec3d", "vec3d opMul(double b) const", asFUNCTIONPR( operator*, ( const vec3d & a, double b ), vec3d ), asCALL_CDECL_OBJFIRST, doc_struct );
    assert( r >= 0 );

    doc_struct.comment = R"(
  /*!
    Multiplication operator for two vec3d objects, performed by the multiplication of each corresponding component
    \code{.cpp}
    vec3d a(), b();                                // Default Constructor

    a.set_xyz( 1.0, 2.0, 3.0 );
    b.set_xyz( 4.0, 5.0, 6.0 );

    vec3d c = a * b;

    Print( "a * b = ", false );

    Print( c );
    \endcode
  */)";
    r = se->RegisterObjectMethod( "vec3d", "vec3d opMul_r(const vec3d &in) const", asFUNCTIONPR( operator*, ( const vec3d&, const vec3d& ), vec3d ), asCALL_CDECL_OBJFIRST, doc_struct );
    assert( r >= 0 );

    doc_struct.comment = R"(
  /*!
    Scalar division operator for a vec3d, performed by the division of of each vec3d component by the scalar
    \code{.cpp}
    vec3d a();                                // Default Constructor

    a.set_xyz( 1.0, 2.0, 3.0 );

    double b = 1.5;

    vec3d c = a / b;

    Print( "a / b = ", false );

    Print( c );
    \endcode
  */)";
    r = se->RegisterObjectMethod( "vec3d", "vec3d opDiv(double b) const", asFUNCTIONPR( operator/, ( const vec3d&, double b ), vec3d ), asCALL_CDECL_OBJFIRST, doc_struct );
    assert( r >= 0 );
    
    doc_struct.comment = R"(
/*!
    Get the magnitude of a vec3d
    \code{.cpp}
    //==== Test Vec3d ====//
    vec3d a();                                // Default Constructor

    //==== Test Mag ====//
    a.set_xyz( 1.0, 2.0, 3.0 );

    if ( abs( a.mag() - sqrt( 14 ) ) > 1e-6 )                        { Print( "---> Error: Vec3d Mag " ); }
    \endcode
    \return Magnitude
*/)";
    r = se->RegisterObjectMethod( "vec3d", "double mag() const", asMETHOD( vec3d, mag ), asCALL_THISCALL, doc_struct );
    assert( r >= 0 );
    
    doc_struct.comment = R"(
/*!
    Normalize the vec3d
    \code{.cpp}
    //==== Test Vec3d ====//
    vec3d a(), b(), c();                                // Default Constructor

    //==== Test Cross ====//
    a.set_xyz( 4.0, 0.0, 0.0 );
    b.set_xyz( 0.0, 3.0, 0.0 );

    c = cross( a, b );

    c.normalize();
    \endcode
*/)";
    r = se->RegisterObjectMethod( "vec3d", "void normalize()", asMETHOD( vec3d, normalize ), asCALL_THISCALL, doc_struct );
    assert( r >= 0 );
    
    doc_struct.comment = R"(
/*!
    Calculate the distance between two vec3d inputs
    \code{.cpp}
    //==== Test Vec3d ====//
    vec3d a(), b();                                // Default Constructor

    //==== Test Dist ====//
    a.set_xyz( 2.0, 2.0, 2.0 );
    b.set_xyz( 3.0, 4.0, 5.0 );

    double d = dist( a, b );

    if ( abs( d - sqrt( 14 ) ) > 1e-6 )    { Print( "---> Error: Vec3d Dist " ); }
    \endcode
    \sa dist_squared
    \param [in] a First vec3d
    \param [in] b Second vec3d
    \return Distance
*/)";
    r = se->RegisterGlobalFunction( "double dist(const vec3d& in a, const vec3d& in b)", asFUNCTIONPR( dist, ( const vec3d&, const vec3d& ), double  ), asCALL_CDECL, doc_struct );
    assert( r >= 0 );
    
    doc_struct.comment = R"(
/*!
    Calculate distance squared between two vec3d inputs
    \code{.cpp}
    //==== Test Vec3d ====//
    vec3d a(), b();                                // Default Constructor

    //==== Test Dist ====//
    a.set_xyz( 2.0, 2.0, 2.0 );
    b.set_xyz( 3.0, 4.0, 5.0 );

    double d2 = dist_squared( a, b );

    if ( abs( d2 - 14 ) > 1e-6 )    { Print( "---> Error: Vec3d Dist " ); }
    \endcode
    \sa dist
    \param [in] a First vec3d
    \param [in] b Second vec3d
    \return Distance squared
*/)";
    r = se->RegisterGlobalFunction( "double dist_squared(const vec3d& in a, const vec3d& in b)", asFUNCTIONPR( dist_squared, ( const vec3d&, const vec3d& ), double  ), asCALL_CDECL, doc_struct );
    assert( r >= 0 );
    
    doc_struct.comment = R"(
/*!
    Calculate the dot product between two vec3d inputs
    \code{.cpp}
    //==== Test Vec3d ====//
    vec3d a(), b();                                // Default Constructor

    //==== Test Dot ====//
    a.set_xyz( 1.0, 2.0, 3.0 );
    b.set_xyz( 2.0, 3.0, 4.0 );

    if ( abs( dot( a, b ) - 20 ) > 1e-6 )                            { Print( "---> Error: Vec3d Dot " ); }
    \endcode
    \param [in] a First vec3d
    \param [in] b Second vec3d
    \return Dot product
*/)";
    r = se->RegisterGlobalFunction( "double dot(const vec3d& in a, const vec3d& in b)", asFUNCTIONPR( dot, ( const vec3d&, const vec3d& ), double  ), asCALL_CDECL, doc_struct );
    assert( r >= 0 );

    doc_struct.comment = R"(
/*!
    Calculate the cross product between two vec3d inputs
    \code{.cpp}
    //==== Test Vec3d ====//
    vec3d a(), b(), c();                                // Default Constructor

    //==== Test Cross ====//
    a.set_xyz( 4.0, 0.0, 0.0 );
    b.set_xyz( 0.0, 3.0, 0.0 );

    c = cross( a, b );

    c.normalize();
    \endcode
    \param [in] a First vec3d
    \param [in] b Second vec3d
    \return Cross product
*/)";
    r = se->RegisterGlobalFunction( "vec3d cross(const vec3d& in a, const vec3d& in b)", asFUNCTIONPR( cross, ( const vec3d&, const vec3d& ), vec3d  ), asCALL_CDECL, doc_struct );
    assert( r >= 0 );

    doc_struct.comment = R"(
/*!
    Calculate the angle between two vec3d inputs (dot product divided by their magnitudes multiplied)
    \code{.cpp}
    //==== Test Vec3d ====//
    vec3d a(), b();                                // Default Constructor
    float PI = 3.14159265359;

    //==== Test Angle ====//
    a.set_xyz( 1.0, 1.0, 0.0 );
    b.set_xyz( 1.0, 0.0, 0.0 );

    if ( abs( angle( a, b ) - PI / 4 ) > 1e-6 )                    { Print( "---> Error: Vec3d Angle " ); }
    \endcode
    \param [in] a First vec3d
    \param [in] b Second vec3d
    \return Angle in Radians
*/)";
    r = se->RegisterGlobalFunction( "double angle(const vec3d& in a, const vec3d& in b)", asFUNCTIONPR( angle, ( const vec3d&, const vec3d& ), double  ), asCALL_CDECL, doc_struct );
    assert( r >= 0 );

    doc_struct.comment = R"(
/*!
    Calculate the signed angle between two vec3d inputs (dot product divided by their magnitudes multiplied) and an input reference axis
    \code{.cpp}
    //==== Test Vec3d ====//
    vec3d a(), b(), c();                                // Default Constructor
    float PI = 3.14159265359;

    //==== Test Angle ====//
    a.set_xyz( 1.0, 1.0, 0.0 );
    b.set_xyz( 1.0, 0.0, 0.0 );
    c.set_xyz( 0.0, 0.0, 1.0 );

    if ( abs( signed_angle( a, b, c ) - -PI / 4 ) > 1e-6 )            { Print( "---> Error: Vec3d SignedAngle " ); }
    \endcode
    \param [in] a First vec3d
    \param [in] b Second vec3d
    \param [in] ref Reference axis
    \return Angle in Radians
*/)";
    r = se->RegisterGlobalFunction( "double signed_angle(const vec3d& in a, const vec3d& in b, const vec3d& in ref )", asFUNCTIONPR( signed_angle, ( const vec3d & a, const vec3d & b, const vec3d & ref ), double  ), asCALL_CDECL, doc_struct );
    assert( r >= 0 );

    doc_struct.comment = R"(
/*!
    Calculate the cosine of angle between two vec3d inputs
    \code{.cpp}
    vec3d pnt = vec3d( 2, 4, 6);

    vec3d line_pt1(), line_pt2();

    line_pt1.set_z( 4 );
    line_pt2.set_y( 3 );

    vec3d p_ln1 = pnt - line_pt1;

    vec3d ln2_ln1 = line_pt2 - line_pt1;

    double numer =  cos_angle( p_ln1, ln2_ln1 ) * p_ln1.mag();
    \endcode
    \sa angle
    \param [in] a First vec3d
    \param [in] b Second vec3d
    \return Angle in Radians
*/)"; // TODO: verify description
    r = se->RegisterGlobalFunction( "double cos_angle(const vec3d& in a, const vec3d& in b )", asFUNCTIONPR( cos_angle, ( const vec3d & a, const vec3d & b ), double  ), asCALL_CDECL, doc_struct );
    assert( r >= 0 );

    doc_struct.comment = R"(
/*!
    Rotate a input point by specified angle around an arbitrary axis. Assume right hand coordinate system
    \code{.cpp}
    //==== Test Vec3d ====//
    vec3d a(), b(), c();                                // Default Constructor
    float PI = 3.14;

    //==== Test Rotate ====//
    a.set_xyz( 1.0, 1.0, 0.0 );
    b.set_xyz( 1.0, 0.0, 0.0 );
    c.set_xyz( 0.0, 0.0, 1.0 );

    c = RotateArbAxis( b, PI, a );
    \endcode
    \param [in] p Coordinate point to rotate
    \param [in] theta Angle of rotation in Radians
    \param [in] axis Reference axis for rotation
    \return Coordinates of rotated point
*/)";
    r = se->RegisterGlobalFunction( "vec3d RotateArbAxis(const vec3d& in p, double theta, const vec3d& in axis )", asFUNCTIONPR( RotateArbAxis, ( const vec3d & p, double theta, const vec3d & axis ), vec3d ), asCALL_CDECL, doc_struct );
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
    asDocInfo doc_struct;

    string group = "Matrix4d";
    doc_struct.group = group.c_str();

    string group_description = R"(
    \brief API functions that utilize the Matrix4d class are grouped here. For details of the class, including member functions, see Matrix4d. \n\n
    \ref index "Click here to return to the main page" )";
    se->AddGroup( group.c_str(), "Matrix4d Functions", group_description.c_str() );

    //==== Register Matrix4d Object =====//
    doc_struct.comment = R"(
//!  A class for representing 4x4 matricies. 
/*!
    Matrix4d is typically used to perform rotations, translations, scaling, projections, and other transformations in 3D space.
*/)";

    int r = se->RegisterObjectType( "Matrix4d", sizeof( Matrix4d ), asOBJ_VALUE | asOBJ_POD | asOBJ_APP_CLASS_CA, doc_struct );
    assert( r >= 0 );

    //===== Register the Matrix4d constructor =====//
    r = se->RegisterObjectBehaviour( "Matrix4d", asBEHAVE_CONSTRUCT, "void f()", asFUNCTION( Matrix4dDefaultConstructor ), asCALL_CDECL_OBJLAST );
    assert( r >= 0 ); // TODO?

    //===== Register the Matrix4d methods =====//

    doc_struct.comment = R"(
/*!
    Create a 4x4 identity matrix
    \code{.cpp}
    //==== Test Matrix4d ====//
    Matrix4d m();                            // Default Constructor
    m.loadIdentity();

    \endcode
    \return Identity Matrix4d
*/)";
    r = se->RegisterObjectMethod( "Matrix4d", "void loadIdentity()", asMETHOD( Matrix4d, loadIdentity ), asCALL_THISCALL, doc_struct );
    assert( r >= 0 );

    doc_struct.comment = R"(
/*!
    Translate the Matrix4d along the given axes values
    \code{.cpp}
    //==== Test Matrix4d ====//
    Matrix4d m();                            // Default Constructor

    m.loadIdentity();

    m.translatef( 1.0, 0.0, 0.0 );
    \endcode
    \param [in] x Translation along the X axis
    \param [in] y Translation along the Y axis
    \param [in] z Translation along the Z axis
    \return Translated Matrix4d
*/)";
    r = se->RegisterObjectMethod( "Matrix4d", "void translatef( const double & in x, const double & in y, const double & in z)", asMETHOD( Matrix4d, translatef ), asCALL_THISCALL, doc_struct );
    assert( r >= 0 );

    doc_struct.comment = R"(
/*!
    Rotate the Matrix4d about the X axis 
    \code{.cpp}
    //==== Test Matrix4d ====//
    Matrix4d m();                            // Default Constructor

    m.loadIdentity();

    m.rotateX( 90.0 );
    \endcode
    \param [in] ang Angle of rotation (degrees)
*/)";
    r = se->RegisterObjectMethod( "Matrix4d", "void rotateX( const double & in ang )", asMETHOD( Matrix4d, rotateX ), asCALL_THISCALL, doc_struct );
    assert( r >= 0 );

    doc_struct.comment = R"(
/*!
    Rotate the Matrix4d about the Y axis
    \code{.cpp}
    //==== Test Matrix4d ====//
    Matrix4d m();                            // Default Constructor

    m.loadIdentity();

    m.rotateY( 90.0 );
    \endcode
    \param [in] ang Angle of rotation (degrees)
*/)";
    r = se->RegisterObjectMethod( "Matrix4d", "void rotateY( const double & in ang )", asMETHOD( Matrix4d, rotateY ), asCALL_THISCALL, doc_struct );
    assert( r >= 0 );

    doc_struct.comment = R"(
/*!
    Rotate the Matrix4d about the Z axis
    \code{.cpp}
    //==== Test Matrix4d ====//
    Matrix4d m();                            // Default Constructor

    m.loadIdentity();

    m.rotateZ( 90.0 );
    \endcode
    \param [in] ang Angle of rotation (degrees)
*/)";
    r = se->RegisterObjectMethod( "Matrix4d", "void rotateZ( const double & in ang )", asMETHOD( Matrix4d, rotateZ ), asCALL_THISCALL, doc_struct );
    assert( r >= 0 );

    doc_struct.comment = R"(
/*!
    Rotate the Matrix4d about an arbitrary axis
    \code{.cpp}
    //==== Test Matrix4d ====//
    Matrix4d m();                            // Default Constructor
    float PI = 3.14;

    m.loadIdentity();

    m.rotate( PI / 4, vec3d( 0.0, 0.0, 1.0 ) );      // Radians
    \endcode
    \param [in] ang Angle of rotation (rad)
    \param [in] axis Vector to rotate about
*/)";
    r = se->RegisterObjectMethod( "Matrix4d", "void rotate( const double & in ang, const vec3d & in axis )", asMETHOD( Matrix4d, rotate ), asCALL_THISCALL, doc_struct );
    assert( r >= 0 );

    doc_struct.comment = R"(
/*!
    Multiply the Matrix4d by a scalar value
    \code{.cpp}
    //==== Test Matrix4d ====//
    Matrix4d m();                            // Default Constructor

    m.loadXZRef();

    m.scale( 10.0 );
    \endcode
    \param [in] scale Value to scale by
*/)";
    r = se->RegisterObjectMethod( "Matrix4d", "void scale( const double & in scale )", asMETHOD( Matrix4d, scale ), asCALL_THISCALL, doc_struct );
    assert( r >= 0 );

    doc_struct.comment = R"(
/*!
    Transform the Matrix4d by the given vector 
    \code{.cpp}
    //==== Test Matrix4d ====//
    Matrix4d m();                            // Default Constructor

    m.loadIdentity();

    vec3d a = m.xform( vec3d( 1.0, 2.0, 3.0 ) );
    \endcode
    \param [in] v Transformation vector
*/)";
    r = se->RegisterObjectMethod( "Matrix4d", "vec3d xform( const vec3d & in v )", asMETHOD( Matrix4d, xform ), asCALL_THISCALL, doc_struct );
    assert( r >= 0 );

    doc_struct.comment = R"(
/*!
    Calculate the Matrix4d's angles between the X, Y and Z axes
    \code{.cpp}
    Matrix4d mat;
    float PI = 3.14;

    mat.loadIdentity();

    m.rotate( PI / 4, vec3d( 0.0, 0.0, 1.0 ) );      // Radians

    vec3d angles = mat.getAngles();
    \endcode
    \return Angle measurement between each axis (degrees)
*/)";
    r = se->RegisterObjectMethod( "Matrix4d", "vec3d getAngles()", asMETHOD( Matrix4d, getAngles ), asCALL_THISCALL, doc_struct );
    assert( r >= 0 );
    
    doc_struct.comment = R"(
/*!
    Load an identy Matrix4d and set the Y coordinate of the diagonal (index 5) to -1
    \code{.cpp}
    //==== Test Matrix4d ====//
    Matrix4d m();                            // Default Constructor

    m.loadXZRef();

    vec3d b = m.xform( vec3d( 1, 2, 3 ) );
    \endcode
*/)";
    r = se->RegisterObjectMethod( "Matrix4d", "void loadXZRef()", asMETHOD( Matrix4d, loadXZRef ), asCALL_THISCALL, doc_struct );
    assert( r >= 0 );

    doc_struct.comment = R"(
/*!
    Load an identy Matrix4d and set the Z coordinate of the diagonal (index 10) to -1
    \code{.cpp}
    //==== Test Matrix4d ====//
    Matrix4d m();                            // Default Constructor

    m.loadXYRef();

    vec3d b = m.xform( vec3d( 1, 2, 3 ) );
    \endcode
*/)";
    r = se->RegisterObjectMethod( "Matrix4d", "void loadXYRef()", asMETHOD( Matrix4d, loadXYRef ), asCALL_THISCALL, doc_struct );
    assert( r >= 0 );

    doc_struct.comment = R"(
/*!
    Load an identy Matrix4d and set the X coordinate of the diagonal (index 0) to -1
    \code{.cpp}
    //==== Test Matrix4d ====//
    Matrix4d m();                            // Default Constructor

    m.loadYZRef();

    vec3d b = m.xform( vec3d( 1, 2, 3 ) );
    \endcode
*/)";
    r = se->RegisterObjectMethod( "Matrix4d", "void loadYZRef()", asMETHOD( Matrix4d, loadYZRef ), asCALL_THISCALL, doc_struct );
    assert( r >= 0 );

    doc_struct.comment = R"(
/*!
    Perform an affine transform on the Matrix4d
    \code{.cpp}
    //==== Test Matrix4d ====//
    Matrix4d m();                            // Default Constructor

    m.loadIdentity();

    m.rotateY( 10.0 );
    m.rotateX( 20.0 );
    m.rotateZ( 30.0 );

    vec3d c = m.xform( vec3d( 1.0, 1.0, 1.0 ) );

    m.affineInverse();
    \endcode
*/)";
    r = se->RegisterObjectMethod( "Matrix4d", "void affineInverse()", asMETHOD( Matrix4d, affineInverse ), asCALL_THISCALL, doc_struct );
    assert( r >= 0 );

    doc_struct.comment = R"(
/*!
    Translate the Matrix4d to a given position and rotate it a about a given center of rotation 
    \param [in] pos Position to translate to
    \param [in] rot Angle of rotation (degrees)
    \param [in] cent_rot Center of rotation
*/)";
    r = se->RegisterObjectMethod("Matrix4d", "void buildXForm( const vec3d & in pos, const vec3d & in rot, const vec3d & in cent_rot )", asMETHOD(Matrix4d, buildXForm), asCALL_THISCALL, doc_struct );
    assert( r >= 0 ); // TODO: Example

    //TODO: Expose additional functions to the API (i.e. matMult)

}

//==== Register Custom Geom Mgr Object ====//
void ScriptMgrSingleton::RegisterCustomGeomMgr( asIScriptEngine* se )
{
    asDocInfo doc_struct;

    string group = "CustomGeom";
    doc_struct.group = group.c_str();
    doc_struct.export_api_test = false;

    string group_description = R"(
    \brief This functions grouped here are used to create and manipulate Custom Components. Custom components
    are defined in *.vsppart files included in the /"Custom Scripts/" directory. Examples of Custom Components
    are available in the directory for reference. OpenVSP looks in 3 locations for the /"Custom Scripts/" folder, 
    where Custom Components are loaded: the root directory, the VSP executable directory, and the home directory. 
    Note, these functions are specific to defining Custom Components and can't be called from standard API scripts
    (i.e. *.vspscript). However, a Custom Component can be created as a *.vsppart file and then accessed through
    secondary API scripts. \n\n
    \ref index "Click here to return to the main page" )";
    se->AddGroup( group.c_str(), "Custom Geometry Functions", group_description.c_str() );

    doc_struct.comment = R"(
  /*!
    Function to add a new Parm of input type, name, and group for a custom Geom component
    \code{.cpp}
    string length = AddParm( PARM_DOUBLE_TYPE, "Length", "Design" );

    SetParmValLimits( length, 10.0, 0.001, 1.0e12 );

    SetParmDescript( length, "Total Length of Custom Geom" );
    \endcode
    \sa PARM_TYPE
    \param [in] type Parm type enum (i.e. PARM_DOUBLE_TYPE)
    \param [in] name Parm name
    \param [in] group Parm group
    \return Parm ID
  */)";

    int r;
    r = se->RegisterGlobalFunction( "string AddParm( int type, const string & in name, const string & in group )",
                                    asMETHOD( CustomGeomMgrSingleton, AddParm ), asCALL_THISCALL_ASGLOBAL, &CustomGeomMgr, doc_struct );
    assert( r );

    doc_struct.comment = R"(
/*!
    Get the Geom ID of the current custom Geom
    \code{.cpp}
    //==== Add Cross Sections  =====//
    string xsec_surf = AddXSecSurf();
    for ( int i = 0 ; i < 4 ; i++ )
    {
        AppendCustomXSec( xsec_surf, XS_ROUNDED_RECTANGLE);
    }

    //==== Set Some Decent Tess Vals ====//
    string geom_id = GetCurrCustomGeom();

    SetParmVal( GetParm( geom_id, "Tess_U",  "Shape" ), 33 );

    SetParmVal( GetParm( geom_id, "Tess_W",  "Shape" ), 13 );
    \endcode
    \return Geom ID
*/)";
    r = se->RegisterGlobalFunction( "string GetCurrCustomGeom()",
                                    asMETHOD( CustomGeomMgrSingleton, GetCurrCustomGeom ), asCALL_THISCALL_ASGLOBAL, &CustomGeomMgr, doc_struct );
    assert( r );

    doc_struct.comment = R"(
/*!
    Get the Parm ID for the input Parm index of the current custom Geom
    \param [in] index Parm index
    \return Parm ID
*/)";
    r = se->RegisterGlobalFunction( "string GetCustomParm( int index )",
                                    asMETHOD( CustomGeomMgrSingleton, GetCustomParm ), asCALL_THISCALL_ASGLOBAL, &CustomGeomMgr, doc_struct );
    assert( r ); // TODO: Example

    doc_struct.comment = R"(
/*!
    Add a GUI element for the current custom Geom. Some inputs may not be used depending on the input type. The Parm name and group name are used to 
    identify the Parm ID associated with the GUI element. 
    \code{.cpp}
    //==== InitGui Is Called Once During Each Custom Geom Construction ====//
    
    AddGui( GDEV_TAB, "Design"  );

    AddGui( GDEV_YGAP );

    AddGui( GDEV_DIVIDER_BOX, "Design" );

    AddGui( GDEV_SLIDER_ADJ_RANGE_INPUT, "Length", "Length", "Design"  );

    AddGui( GDEV_YGAP );

    AddGui( GDEV_SLIDER_ADJ_RANGE_INPUT, "Diameter", "Diameter", "Design"  );
    
    \endcode
    \sa GDEV
    \param [in] type GUI element type enum (i.e. GDEV_SLIDER_ADJ_RANGE)
    \param [in] label Optional GUI device label
    \param [in] parm_name Optional Parm name
    \param [in] group_name Optional Parm group name
    \param [in] range Optional GUI element range
    \return GUI element index
*/)";
    r = se->RegisterGlobalFunction( "int AddGui( int type, const string & in label = string(), const string & in parm_name = string(), const string & in group_name = string(), double range = 10.0 )",
                                    asMETHOD( CustomGeomMgrSingleton, AddGui ), asCALL_THISCALL_ASGLOBAL, &CustomGeomMgr, doc_struct );
    assert( r );

    doc_struct.comment = R"(
/*!
    Update the GUI element with an associated Parm
    /code{.cpp}
    //==== UpdateGui Is Called Every Time The Gui is Updated ====//
    
    string geom_id = GetCurrCustomGeom();

    UpdateGui( WidthSlider, GetParm( geom_id, "Width", "Design" ) );

    UpdateGui( SeatHeightSlider, GetParm( geom_id, "SeatHeight", "Design" ) );

    UpdateGui( SeatLengthSlider, GetParm( geom_id, "SeatLength", "Design" ) );

    UpdateGui( BackHeightSlider, GetParm( geom_id, "BackHeight", "Design" ) );
    /endcode
    \param [in] gui_id Index of the GUI element
    \param [in] parm_id Parm ID 
*/)";
    r = se->RegisterGlobalFunction( "void UpdateGui( int gui_id, const string & in parm_id )",
                                    asMETHOD( CustomGeomMgrSingleton, AddUpdateGui ), asCALL_THISCALL_ASGLOBAL, &CustomGeomMgr, doc_struct );
    assert( r );

    doc_struct.comment = R"(
/*!
    Add an XSecSurf to the current custom Geom
    /code{.cpp}
    //==== Add Cross Sections  =====//
    string seat_surf = AddXSecSurf();

    AppendCustomXSec( seat_surf, XS_POINT);

    AppendCustomXSec( seat_surf, XS_ROUNDED_RECTANGLE);

    AppendCustomXSec( seat_surf, XS_ROUNDED_RECTANGLE);

    AppendCustomXSec( seat_surf, XS_POINT);
    /endcode
    \return XSecSurf ID
*/)";
    r = se->RegisterGlobalFunction( "string AddXSecSurf()",
                                    asMETHOD( CustomGeomMgrSingleton, AddXSecSurf ), asCALL_THISCALL_ASGLOBAL, &CustomGeomMgr, doc_struct );
    assert( r );

    doc_struct.comment = R"(
/*!
    Remove an XSecSurf from the current custom Geom
    \code{.cpp}
    //==== Add Temp Spine Surf ====//
    string spine_surf = AddXSecSurf();

    string spine_xsec = AppendXSec( spine_surf, XS_GENERAL_FUSE );

    SetCustomXSecRot( spine_xsec, vec3d( 0, 0, 90 ) );

    string geom_id = GetCurrCustomGeom();
 
    //==== Get The XSec Surf ====//
    string xsec_surf = GetXSecSurf( geom_id, 0 );

    //=== Remove Spine Surf ====//
    RemoveXSecSurf( spine_surf );
    \endcode
    \param [in] xsec_id XSecSurf ID
*/)";
    r = se->RegisterGlobalFunction( "void RemoveXSecSurf(const string & in xsec_id)",
                                    asMETHOD( CustomGeomMgrSingleton, RemoveXSecSurf ), asCALL_THISCALL_ASGLOBAL, &CustomGeomMgr, doc_struct );
    assert( r );

    doc_struct.comment = R"(
/*!
    Clear all XSecSurf from the current custom Geom
    /code{.cpp}
    //==== Add Cross Sections  =====//
    string surf1 = AddXSecSurf();

    string surf2 = AddXSecSurf();

    ClearXSecSurfs();
    /endcode
*/)";
    r = se->RegisterGlobalFunction( "void ClearXSecSurfs()",
                                    asMETHOD( CustomGeomMgrSingleton, ClearXSecSurfs ), asCALL_THISCALL_ASGLOBAL, &CustomGeomMgr, doc_struct );
    assert( r );

    doc_struct.comment = R"(
/*!
    Perform a skinning update for the current custom Geom. This is typically the last call in the UpdateSurf 
    function that gets called every time the Geom is updated. 
    \code{.cpp}
    //==== UpdateSurf Is Called Every Time The Geom is Updated ====//
    
    string geom_id = GetCurrCustomGeom();

    //==== Get The XSec Surf ====//
    string xsec_surf = GetXSecSurf( geom_id, 0 );

    //==== Define The First/Last XSec Placement ====//
    string xsec3 = GetXSec( xsec_surf, 3 );

    SetCustomXSecLoc( xsec3, vec3d( 10.0, 0, 5.0 ) );

    SkinXSecSurf();
    
    \endcode
    \param [in] closed_flag Flag to set the last XSec equal to the first
*/)";
    r = se->RegisterGlobalFunction( "void SkinXSecSurf( bool closed_flag = false )",
                                    asMETHOD( CustomGeomMgrSingleton, SkinXSecSurf ), asCALL_THISCALL_ASGLOBAL, &CustomGeomMgr, doc_struct );
    assert( r );

    doc_struct.comment = R"(
/*!
    Make a copy of the current custom Geom's main surface at given index and apply a transformation
    \code{.cpp}
    string geom_id = GetCurrCustomGeom();

    Matrix4d mat;

    double x = 2.0;
    double y = 5.0;
    double z = 0.0;

    mat.translatef( x, y, z );

    CloneSurf( 0, mat );
    \endcode
    \param [in] index Main surface index
    \param [in] mat Transformation matrix
*/)"; 
    r = se->RegisterGlobalFunction( "void CloneSurf(int index, Matrix4d & in mat)",
                                    asMETHOD( CustomGeomMgrSingleton, CloneSurf ), asCALL_THISCALL_ASGLOBAL, &CustomGeomMgr, doc_struct );
    assert( r );

    doc_struct.comment = R"(
/*!
    Perform a transformation for the main surface at input index for the current custom Geom
    \param [in] index Main surface index
    \param [in] mat Transformation matrix
*/)";
    r = se->RegisterGlobalFunction( "void TransformSurf(int index, Matrix4d & in mat)",
                                    asMETHOD( CustomGeomMgrSingleton, TransformSurf ), asCALL_THISCALL_ASGLOBAL, &CustomGeomMgr, doc_struct );
    assert( r ); // TODO: Example

    doc_struct.comment = R"(
/*!
    Set the surface type for the current custom Geom at given surface index
    \code{.cpp}
    //==== UpdateSurf Is Called Every Time The Geom is Updated ====//
    
    SetVspSurfType( DISK_SURF, -1 );

    SetVspSurfCfdType( CFD_TRANSPARENT, -1 );

    SkinXSecSurf();
    
    \endcode
    \sa VSP_SURF_TYPE
    \param [in] type Surface type enum (i.e DISK_SURF)
    \param [in] surf_index Main surface index. The default value of -1 is used to indicate all main surfaces are the same type.
*/)";
    r = se->RegisterGlobalFunction( "void SetVspSurfType( int type, int surf_index = -1 )",
                                    asMETHOD( CustomGeomMgrSingleton, SetVspSurfType ), asCALL_THISCALL_ASGLOBAL, &CustomGeomMgr, doc_struct );
    assert( r );

    doc_struct.comment = R"(
/*!
    Set the surface CFD type for the current custom Geom at given surface index
    \code{.cpp}
    //==== UpdateSurf Is Called Every Time The Geom is Updated ====//
    
    SetVspSurfType( DISK_SURF, -1 );

    SetVspSurfCfdType( CFD_TRANSPARENT, -1 );

    SkinXSecSurf();
    
    \endcode
    \sa VSP_SURF_CFD_TYPE
    \param [in] type CFD surface type enum (i.e. CFD_TRANSPARENT)
    \param [in] surf_index Main surface index. The default value of -1 is used to indicate all main surfaces are the same type.
*/)";
    r = se->RegisterGlobalFunction( "void SetVspSurfCfdType( int type, int surf_index = -1 )",
                                    asMETHOD( CustomGeomMgrSingleton, SetVspSurfCfdType ), asCALL_THISCALL_ASGLOBAL, &CustomGeomMgr, doc_struct );
    assert( r );

    doc_struct.comment = R"(
/*!
    Set the location of an XSec for the current custom Geom
    \code{.cpp}
    string geom_id = GetCurrCustomGeom();

    //==== Get The XSec Surf ====//
    string xsec_surf = GetXSecSurf( geom_id, 0 );

    string xsec0 = GetXSec( xsec_surf, 0 );

    SetCustomXSecLoc( xsec0, vec3d( 0.0, 0.0, 5.0 ) );
    \endcode
    \param [in] xsec_id XSec ID
    \param [in] loc 3D location
*/)";
    r = se->RegisterGlobalFunction( "void SetCustomXSecLoc( const string & in xsec_id, const vec3d & in loc )",
                                    asMETHOD( CustomGeomMgrSingleton, SetCustomXSecLoc ), asCALL_THISCALL_ASGLOBAL, &CustomGeomMgr, doc_struct );
    assert( r );

    doc_struct.comment = R"(
/*!
    Get the location of an XSec for the current custom Geom
    \code{.cpp}
    string geom_id = GetCurrCustomGeom();

    //==== Get The XSec Surf ====//
    string xsec_surf = GetXSecSurf( geom_id, 0 );

    string xsec0 = GetXSec( xsec_surf, 0 );

    SetCustomXSecLoc( xsec0, vec3d( 1.0, 2.0, 3.0 ) );

    vec3d loc =  GetCustomXSecLoc( xsec0 );

    Print( "Custom XSec Location: ", false );

    Print( loc );
    \endcode
    \param [in] xsec_id XSec ID
    \return 3D location
*/)";
    r = se->RegisterGlobalFunction( "vec3d GetCustomXSecLoc( const string & in xsec_id )",
                                    asMETHOD( CustomGeomMgrSingleton, GetCustomXSecLoc ), asCALL_THISCALL_ASGLOBAL, &CustomGeomMgr, doc_struct );
    assert( r );

    doc_struct.comment = R"(
/*!
    Set the rotation of an XSec for the current custom Geom
    \code{.cpp}
    //==== Add Spine Surf ====//
    string spine_surf = AddXSecSurf();

    string spine_xsec = AppendXSec( spine_surf, XS_GENERAL_FUSE );

    SetCustomXSecRot( spine_xsec, vec3d( 0, 0, 90 ) );
    \endcode
    \param [in] xsec_id XSec ID
    \param [in] rot Angle of rotation about the X, Y and Z axes (degrees)
*/)";
    r = se->RegisterGlobalFunction( "void SetCustomXSecRot( const string & in xsec_id, const vec3d & in rot )",
                                    asMETHOD( CustomGeomMgrSingleton, SetCustomXSecRot ), asCALL_THISCALL_ASGLOBAL, &CustomGeomMgr, doc_struct );
    assert( r );

    doc_struct.comment = R"(
/*!
    Get the rotation of an XSec for the current custom Geom
    \code{.cpp}
    //==== Add Spine Surf ====//
    string spine_surf = AddXSecSurf();

    string spine_xsec = AppendXSec( spine_surf, XS_GENERAL_FUSE );

    SetCustomXSecRot( spine_xsec, vec3d( 0, 0, 90 ) );

    string geom_id = GetCurrCustomGeom();

    //==== Get The XSec Surf ====//
    string xsec_surf = GetXSecSurf( geom_id, 0 );

    string xsec0 = GetXSec( xsec_surf, 0 );

    vec3d rot =  GetCustomXSecRot( xsec0 );

    Print( "Custom XSec Rotation: ", false );

    Print( rot );
    \endcode
    \param [in] xsec_id XSec ID
    \return 3D rotation (degrees)
*/)";
    r = se->RegisterGlobalFunction( "vec3d GetCustomXSecRot( const string & in xsec_id )",
                                    asMETHOD( CustomGeomMgrSingleton, GetCustomXSecRot ), asCALL_THISCALL_ASGLOBAL, &CustomGeomMgr, doc_struct );
    assert( r );

    doc_struct.comment = R"(
/*!
    Check and clear a trigger event for a custom Geom GUI element
    \param [in] gui_id GUI element index
    \return True if successful, false otherwise
*/)";
    r = se->RegisterGlobalFunction( "bool CheckClearTriggerEvent( int gui_id )",
                                    asMETHOD( CustomGeomMgrSingleton, CheckClearTriggerEvent ), asCALL_THISCALL_ASGLOBAL, &CustomGeomMgr, doc_struct );
    assert( r ); // TODO: Example

    doc_struct.comment = R"(
/*!
    Add a CFD Mesh default source for the current custom Geom. Note, certain input params may not be used depending on the source type.
    \code{.cpp}
    
    //==== Add Cross Sections  =====//
    string xsec_surf = AddXSecSurf();

    AppendCustomXSec( xsec_surf, XS_POINT);

    AppendCustomXSec( xsec_surf, XS_CIRCLE);

    //==== Add A Default Point Source At Nose ====//
    SetupCustomDefaultSource( POINT_SOURCE, 0, 0.1, 1.0, 1.0, 1.0 );
    
    \endcode
    \sa CFD_MESH_SOURCE_TYPE
    \param [in] type CFD Mesh source type enum (i.e. BOX_SOURCE)
    \param [in] surf_index Main surface index
    \param [in] l1 Source first edge length
    \param [in] r1 Source first radius
    \param [in] u1 Source first U location
    \param [in] w1 Source first W location
    \param [in] l2 Source second edge length
    \param [in] r2 Source second radius
    \param [in] u2 Source second U location
    \param [in] w2 Source second W location
*/)";
    r = se->RegisterGlobalFunction(
        "void SetupCustomDefaultSource( int type, int surf_index, double l1, double r1, double u1, double w1, double l2 = 0, double r2 = 0, double u2 = 0, double w2 = 0 )",
        asMETHOD( CustomGeomMgrSingleton, SetupCustomDefaultSource ), asCALL_THISCALL_ASGLOBAL, &CustomGeomMgr, doc_struct );
    assert( r >= 0 );

    doc_struct.comment = R"(
/*!
    Clear all default sources for the current custom Geom
    \code{.cpp}
    SetupCustomDefaultSource( POINT_SOURCE, 0, 0.1, 1.0, 1.0, 1.0 );

    ClearAllCustomDefaultSources();
    \endcode
*/)";
    r = se->RegisterGlobalFunction( "void ClearAllCustomDefaultSources()",
                                    asMETHOD( CustomGeomMgrSingleton, ClearAllCustomDefaultSources ), asCALL_THISCALL_ASGLOBAL, &CustomGeomMgr, doc_struct );
    assert( r );

    doc_struct.comment = R"(
/*!
    Set the center point of the current custom Geom
    \code{.cpp}
    string geom_id = GetCurrCustomGeom();

    double ht= GetParmVal( GetParm( geom_id, "Height", "Design" ) );

    double origin = GetParmVal( GetParm( geom_id, "Origin", "XForm" ) );

    SetCustomCenter( ht*origin, 0, 0 );
    \endcode
    \param [in] x X coordinate
    \param [in] y Y coordinate
    \param [in] z Z coordinate
*/)";
    r = se->RegisterGlobalFunction( "void SetCustomCenter( double x, double y, double z )",
                                    asMETHOD( CustomGeomMgrSingleton, SetCustomCenter ), asCALL_THISCALL_ASGLOBAL, &CustomGeomMgr, doc_struct );
    assert( r );

    doc_struct.comment = R"(
/*!
    Append an XSec to the current custom Geom. This function is identical to AppendCustomXSec.
    \code{.cpp}
    //==== Add Cross Sections  =====//
    string xsec_surf = AddXSecSurf();

    AppendXSec( xsec_surf, XS_POINT);

    AppendXSec( xsec_surf, XS_CIRCLE );

    AppendXSec( xsec_surf, XS_CIRCLE );

    AppendXSec( xsec_surf, XS_ELLIPSE );

    AppendXSec( xsec_surf, XS_POINT);
    \endcode
    \sa XSEC_CRV_TYPE, AppendCustomXSec
    \param [in] xsec_surf_id XSecSurf ID
    \param [in] type XSec type enum (i.e. XSEC_WEDGE)
    \return XSec ID
*/)";
    r = se->RegisterGlobalFunction( "string AppendXSec( const string & in xsec_surf_id, int type )",
                                    asMETHOD( CustomGeomMgrSingleton, AppendCustomXSec ), asCALL_THISCALL_ASGLOBAL, &CustomGeomMgr, doc_struct );
    assert( r );

    // WARNING: Both versions of the AppendCustomXSec must be available to avoid breaking existing CustomGeom scripts

    doc_struct.comment = R"(
/*!
    Append an XSec to the current custom Geom. This function is identical to AppendXSec.
    \code{.cpp}
    //==== Add Cross Sections  =====//
    string xsec_surf = AddXSecSurf();

    AppendCustomXSec( xsec_surf, XS_POINT);

    AppendCustomXSec( xsec_surf, XS_CIRCLE );

    AppendCustomXSec( xsec_surf, XS_CIRCLE );

    AppendCustomXSec( xsec_surf, XS_ELLIPSE );

    AppendCustomXSec( xsec_surf, XS_POINT);
    \endcode
    \sa XSEC_CRV_TYPE, AppendXSec
    \param [in] xsec_surf_id XSecSurf ID
    \param [in] type XSec type enum (i.e. XSEC_WEDGE)
    \return XSec ID
*/)";
    r = se->RegisterGlobalFunction( "string AppendCustomXSec( const string & in xsec_surf_id, int type )",
                                    asMETHOD( CustomGeomMgrSingleton, AppendCustomXSec ), asCALL_THISCALL_ASGLOBAL, &CustomGeomMgr, doc_struct );
    assert( r );

    doc_struct.comment = R"(
/*!
    Remove an XSec from the current custom Geom and keep it in memory
    \sa PasteCustomXSec
    \param [in] xsec_surf_id XSecSurf ID
    \param [in] index XSec index
*/)";
    r = se->RegisterGlobalFunction( "void CutCustomXSec( const string & in xsec_surf_id, int index )",
                                    asMETHOD( CustomGeomMgrSingleton, CutCustomXSec ), asCALL_THISCALL_ASGLOBAL, &CustomGeomMgr, doc_struct );
    assert( r ); // TODO: Example

    doc_struct.comment = R"(
/*!
    Copy an XSec from the current custom Geom and keep it in memory
    \sa PasteCustomXSec
    \param [in] xsec_surf_id XSecSurf ID
    \param [in] index XSec index
*/)";
    r = se->RegisterGlobalFunction( "void CopyCustomXSec( const string & in xsec_surf_id, int index )",
                                    asMETHOD( CustomGeomMgrSingleton, CopyCustomXSec ), asCALL_THISCALL_ASGLOBAL, &CustomGeomMgr, doc_struct );
    assert( r ); // TODO: Example

    doc_struct.comment = R"(
/*!
    Paste the XSec currently held in memory for the current custom Geom at given index
    \sa CutCustomXSec, CopyCustomXSec
    \param [in] xsec_surf_id XSecSurf ID
    \param [in] index XSec index
*/)";
    r = se->RegisterGlobalFunction( "void PasteCustomXSec( const string & in xsec_surf_id, int index )",
                                    asMETHOD( CustomGeomMgrSingleton, PasteCustomXSec ), asCALL_THISCALL_ASGLOBAL, &CustomGeomMgr, doc_struct );
    assert( r ); // TODO: Example

    doc_struct.comment = R"(
/*!
    Insert a new XSec at the given index for the currently selected custom Geom
    \sa XSEC_CRV_TYPE
    \param [in] xsec_surf_id XSecSurf ID
    \param [in] type XSec type enum (i.e. XSEC_WEDGE)
    \param [in] index XSec index
*/)";
    r = se->RegisterGlobalFunction( "string InsertCustomXSec( const string & in xsec_surf_id, int type, int index )",
                                    asMETHOD( CustomGeomMgrSingleton, InsertCustomXSec ), asCALL_THISCALL_ASGLOBAL, &CustomGeomMgr, doc_struct );
    assert( r ); // TODO: Example


}



//==== Register Adv Link Mgr Object ====//
void ScriptMgrSingleton::RegisterAdvLinkMgr( asIScriptEngine* se )
{
    int r;
    asDocInfo doc_struct;

    string group = "AdvancedLink";
    doc_struct.group = group.c_str();

    string group_description = R"(
    \brief The following functions are available for the Advanced Link tool. \n\n
    \ref index "Click here to return to the main page" )";
    se->AddGroup( group.c_str(), "Advanced Link Functions", group_description.c_str() );

    doc_struct.comment = R"(
/*!
    Add an Advanced Link input Parm
    \code{.cpp}
    // Add Pod Geom
    string pid = AddGeom( "POD" );

    string tess_u_id = FindParm( pid, "Tess_U", "Shape" );

    AddInput( tess_u_id, "ExampleVariable" );
    \endcode
    \param [in] parm_id Parm ID
    \param [in] var_name Advanced Link variable name
*/)";
    r = se->RegisterGlobalFunction( "void AddInput( const string & in parm_id, const string & in var_name )",
                                    asMETHOD( AdvLinkMgrSingleton, AddInput ), asCALL_THISCALL_ASGLOBAL, &AdvLinkMgr, doc_struct );
    assert( r );

    doc_struct.comment = R"(
/*!
    Add an Advanced Link output Parm
    \code{.cpp}
    // Add Pod Geom
    string pid = AddGeom( "POD" );

    string tess_u_id = FindParm( pid, "Tess_U", "Shape" );

    AddOutput( tess_u_id, "ExampleVariable" );
    \endcode
    \param [in] parm_id Parm ID
    \param [in] var_name Advanced Link variable name
*/)";
    r = se->RegisterGlobalFunction( "void AddOutput( const string & in parm_id, const string & in var_name )",
                                    asMETHOD( AdvLinkMgrSingleton, AddOutput ), asCALL_THISCALL_ASGLOBAL, &AdvLinkMgr, doc_struct );
    assert( r );

    doc_struct.comment = R"(
/*!
    Set an Advanced Link variable to the specified value
    \code{.cpp}
    // Add Pod Geom
    string pid = AddGeom( "POD" );

    string tess_u_id = FindParm( pid, "Tess_U", "Shape" );

    AddInput( tess_u_id, "ExampleVariable" );

    SetVar( "ExampleVariable", 20 );
    \endcode
    \param [in] var_name Advanced Link variable name
    \param [in] val Value for the variable
*/)";
    r = se->RegisterGlobalFunction( "void SetVar( const string & in var_name, double val )", asMETHOD( AdvLinkMgrSingleton, SetVar ), asCALL_THISCALL_ASGLOBAL, &AdvLinkMgr, doc_struct );
    assert( r );

    doc_struct.comment = R"(
/*!
    Get the value of the specified Advanced Link variable
    \code{.cpp}
    // Add Pod Geom
    string pid = AddGeom( "POD" );

    string tess_u_id = FindParm( pid, "Tess_U", "Shape" );

    AddInput( tess_u_id, "ExampleVariable" );

    SetVar( "ExampleVariable", 20 );

    Print( "ExampleVariable: ", false );

    Print( GetVar( "ExampleVariable" ) );
    \endcode
    \param [in] var_name Advanced Link variable name
    \return Value for the variable
*/)";
    r = se->RegisterGlobalFunction( "double GetVar( const string & in var_name )", asMETHOD( AdvLinkMgrSingleton, GetVar ), asCALL_THISCALL_ASGLOBAL, &AdvLinkMgr, doc_struct );
    assert( r );
}

//==== Register API E Functions ====//
void ScriptMgrSingleton::RegisterAPIErrorObj( asIScriptEngine* se )
{
    asDocInfo doc_struct;

    string group = "ErrorObj";
    doc_struct.group = group.c_str();

    // Note: No group needed for object methods only

    doc_struct.comment = R"(
//!  A class for representing API Errors
/*!
    ErrorObj is defined by an error code enum and associated error string. 
*/)";

    //==== Register ErrorObj Object =====//
    int r = se->RegisterObjectType( "ErrorObj", sizeof( vsp::ErrorObj ), asOBJ_VALUE | asOBJ_POD | asOBJ_APP_CLASS_CDA, doc_struct );
    assert( r >= 0 );

    doc_struct.comment = R"(
/*!
    Get the ERROR_CODE enum of the last raised error
    \code{.cpp}
    ErrorObj err = PopLastError();

    if ( err.GetErrorCode() != VSP_CANT_FIND_PARM )            { Print( "---> Error: API PopLast" ); }
    \endcode
    \sa ERROR_CODE
    \return ERROR_CODE error code enum
*/)";
    r = se->RegisterObjectMethod( "ErrorObj", "ERROR_CODE GetErrorCode()", asMETHOD( vsp::ErrorObj, GetErrorCode ), asCALL_THISCALL, doc_struct );
    assert( r >= 0 );

    doc_struct.comment = R"(
/*!
    Get the error string of the last raised error
    \code{.cpp}
    //==== Check For API Errors ====//
    while ( GetNumTotalErrors() > 0 )
    {
        ErrorObj err = PopLastError();
        Print( err.GetErrorString() );
    }
    \endcode
    \return Error string
*/)";
    r = se->RegisterObjectMethod( "ErrorObj", "string GetErrorString()", asMETHOD( vsp::ErrorObj, GetErrorString ), asCALL_THISCALL, doc_struct );
    assert( r >= 0 );



}


//==== Register VSP API Functions ====//
void ScriptMgrSingleton::RegisterAPI( asIScriptEngine* se )
{
    int r;
    asDocInfo doc_struct;

    //==== API Error Functions ====//
    // TODO: Move to ErrorObj group
    string group = "APIError";
    doc_struct.group = group.c_str();

    string group_description = R"(
    \brief Handling of OpenVSP ErrorObj information is accomplished through this group of API functions. \n\n
    \ref index "Click here to return to the main page")";
    se->AddGroup( group.c_str(), "API Error Functions", group_description.c_str() );

    doc_struct.comment = R"(
/*!
    Check if there was an error on the last call to the API
    \code{.cpp}
    //==== Force API to silence error messages ====//
    SilenceErrors();

    //==== Bogus Call To Create API Error ====//
    Print( string( "---> Test Error Handling" ) );

    SetParmVal( "BogusParmID", 23.0 );

    if ( !GetErrorLastCallFlag() )                        { Print( "---> Error: API GetErrorLastCallFlag " ); }

    //==== Tell API to print error messages ====//
    PrintOnErrors();
    \endcode
    \return False if no error, true otherwise
*/)";
    r = se->RegisterGlobalFunction( "bool GetErrorLastCallFlag()", asMETHOD( vsp::ErrorMgrSingleton, GetErrorLastCallFlag ), asCALL_THISCALL_ASGLOBAL, &vsp::ErrorMgr, doc_struct );
    assert( r );

    doc_struct.comment = R"(
/*!
    Count the total number of errors on the stack
    \code{.cpp}
    //==== Force API to silence error messages ====//
    SilenceErrors();

    Print( "Creating an API error" );
    SetParmVal( "ABCDEFG", "Test_Name", "Test_Group", 123.4 );

    //==== Check For API Errors ====//
    while ( GetNumTotalErrors() > 0 )
    {
        ErrorObj err = PopLastError();
        Print( err.GetErrorString() );
    }

    //==== Tell API to print error messages ====//
    PrintOnErrors();
    \endcode
    \return Number of errors
*/)";
    r = se->RegisterGlobalFunction( "int GetNumTotalErrors()", asMETHOD( vsp::ErrorMgrSingleton, GetNumTotalErrors ), asCALL_THISCALL_ASGLOBAL, &vsp::ErrorMgr, doc_struct );
    assert( r );

    doc_struct.comment = R"(
/*!
    Pop (remove) and return the most recent error from the stack. Note, errors are printed on occurrence by default. 
    \code{.cpp}
    //==== Force API to silence error messages ====//
    SilenceErrors();

    Print( "Creating an API error" );
    SetParmVal( "ABCDEFG", "Test_Name", "Test_Group", 123.4 );

    //==== Check For API Errors ====//
    while ( GetNumTotalErrors() > 0 )
    {
        ErrorObj err = PopLastError();
        Print( err.GetErrorString() );
    }

    //==== Tell API to print error messages ====//
    PrintOnErrors();
    \endcode
    \return Error object
*/)";
    r = se->RegisterGlobalFunction( "ErrorObj PopLastError()", asMETHOD( vsp::ErrorMgrSingleton, PopLastError ), asCALL_THISCALL_ASGLOBAL, &vsp::ErrorMgr, doc_struct );
    assert( r );

    doc_struct.comment = R"(
/*!
    Return the most recent error from the stack (does NOT pop error off the stack)
    \code{.cpp}
    //==== Force API to silence error messages ====//
    SilenceErrors();

    Print( "Creating an API error" );
    SetParmVal( "ABCDEFG", "Test_Name", "Test_Group", 123.4 );

    //==== Check For API Errors ====//
    ErrorObj err = GetLastError();

    Print( err.GetErrorString() );

    //==== Tell API to print error messages ====//
    PrintOnErrors();
    \endcode
    \sa SilenceErrors, PrintOnErrors;
    \return Error object
*/)";
    r = se->RegisterGlobalFunction( "ErrorObj GetLastError()", asMETHOD( vsp::ErrorMgrSingleton, GetLastError ), asCALL_THISCALL_ASGLOBAL, &vsp::ErrorMgr, doc_struct );
    assert( r );

    doc_struct.comment = R"(
/*!
    Prevent errors from printing to stdout as they occur.
    \code{.cpp}
    //==== Force API to silence error messages ====//
    SilenceErrors();

    Print( "Creating an API error" );
    SetParmVal( "ABCDEFG", "Test_Name", "Test_Group", 123.4 );

    //==== Tell API to print error messages ====//
    PrintOnErrors();
    \endcode
    \sa PrintOnErrors
*/)";
    r = se->RegisterGlobalFunction( "void SilenceErrors()", asMETHOD( vsp::ErrorMgrSingleton, SilenceErrors ), asCALL_THISCALL_ASGLOBAL, &vsp::ErrorMgr, doc_struct );
    assert( r );

    doc_struct.comment = R"(
/*!
    Cause errors to be printed to stdout as they occur.
    \code{.cpp}
    //==== Force API to silence error messages ====//
    SilenceErrors();

    Print( "Creating an API error" );
    SetParmVal( "ABCDEFG", "Test_Name", "Test_Group", 123.4 );

    //==== Tell API to print error messages ====//
    PrintOnErrors();
    \endcode
    \sa SilenceErrors
*/)";
    r = se->RegisterGlobalFunction( "void PrintOnErrors()", asMETHOD( vsp::ErrorMgrSingleton, PrintOnErrors ), asCALL_THISCALL_ASGLOBAL, &vsp::ErrorMgr, doc_struct );
    assert( r );

    //==== Visualization Functions ====//
    group = "Visualization";
    doc_struct.group = group.c_str();

    group_description = R"(
    \brief The following group of functions allow for the OpenVSP GUI to be manipulated through the API. \n\n
    \ref index "Click here to return to the main page" )";
    se->AddGroup( group.c_str(), "Visualization Functions", group_description.c_str() );

    doc_struct.comment = R"(
/*!
    Capture the specified screen and save to file. Note, VSP_USE_FLTK must be defined
    \code{.cpp}
    int screenw = 2000;                                             // Set screenshot width and height
    int screenh = 2000;

    string fname = "test_screen_grab.png";

    ScreenGrab( fname, screenw, screenh, true );                // Take PNG screenshot
    \endcode
    \param [in] file_name Output file name
    \param [in] w Width of screen grab
    \param [in] h Height of screen grab
    \param [in] transparentBG Transparent background flag
*/)";
    r = se->RegisterGlobalFunction( "void ScreenGrab( const string & in file_name, int w, int h, bool transparentBG )", asFUNCTION( vsp::ScreenGrab ), asCALL_CDECL, doc_struct );
    assert( r >= 0 );

    doc_struct.comment = R"(
/*!
    Toggle viewing the axis
    \code{.cpp}
    SetViewAxis( false );                                           // Turn off axis marker in corner of viewscreen
    \endcode
    \param [in] vaxis True to show the axis, false to hide the axis
*/)";
    r = se->RegisterGlobalFunction( "void SetViewAxis( bool vaxis )", asFUNCTION( vsp::SetViewAxis ), asCALL_CDECL, doc_struct );
    assert( r >= 0 );

    doc_struct.comment = R"(
/*!
    Toggle viewing the border frame
    \code{.cpp}
    SetShowBorders( false );                                        // Turn off red/black border on active window
    \endcode
    \param [in] brdr True to show the border frame, false to hide the border frame
*/)";
    r = se->RegisterGlobalFunction( "void SetShowBorders( bool brdr )", asFUNCTION( vsp::SetShowBorders ), asCALL_CDECL, doc_struct );
    assert( r >= 0 );

    doc_struct.comment = R"(
/*!
    Set the draw type of the specified goemetry
    \code{.cpp}
    string pid = AddGeom( "POD", "" );                             // Add Pod for testing

    SetGeomDrawType( pid, GEOM_DRAW_SHADE );                       // Make pod appear as shaded
    \endcode
    \sa DRAW_TYPE
    \param [in] geom_id Geom ID
    \param [in] type Draw type enum (i.e. GEOM_DRAW_SHADE)
*/)";
    r = se->RegisterGlobalFunction("void SetGeomDrawType( const string & in geom_id, int type )", asFUNCTION(vsp::SetGeomDrawType), asCALL_CDECL, doc_struct);
    assert( r >= 0 );

    doc_struct.comment = R"(
/*!
    Set the display type of the specified goemetry
    \code{.cpp}
    string pid = AddGeom( "POD" );                             // Add Pod for testing

    SetGeomDisplayType( pid, DISPLAY_DEGEN_PLATE );                       // Make pod appear as Bezier plate (Degen Geom) 
    \endcode
    \sa DISPLAY_TYPE
    \param [in] geom_id Geom ID
    \param [in] type Display type enum (i.e. DISPLAY_BEZIER)
*/)";
    r = se->RegisterGlobalFunction("void SetGeomDisplayType( const string & in geom_id, int type )", asFUNCTION(vsp::SetGeomDisplayType), asCALL_CDECL, doc_struct);
    assert( r >= 0 );

    doc_struct.comment = R"(
/*!
    Set the background color
    \code{.cpp}
    SetBackground( 1.0, 1.0, 1.0 );                                 // Set background to bright white
    \endcode
    \param [in] r Red 8-bit unsigned integer (range: 0-255)
    \param [in] g Green 8-bit unsigned integer (range: 0-255)
    \param [in] b Blue 8-bit unsigned integer (range: 0-255)
*/)";
    r = se->RegisterGlobalFunction( "void SetBackground( double r, double g, double b )", asFUNCTION( vsp::SetBackground ), asCALL_CDECL, doc_struct );
    assert( r >= 0 );

    //==== Vehicle Functions ====//
    group = "Vehicle";
    doc_struct.group = group.c_str();

    group_description = R"(
    \brief The Vehicle group of functions are high-level commands that pertain to the entire OpenVSP model. \n\n
    \ref index "Click here to return to the main page" )";
    se->AddGroup( group.c_str(), "Vehicle Functions", group_description.c_str() );

    doc_struct.comment = R"(
/*!
    Update the entire vehicle and all lower level children. An input, which is true by default, is available to specify
    if managers should be updated as well. The managers are typically updated by their respective GUI, so must be 
    updated through the API as well to avoid unexpected behavior. 
    \code{.cpp}
    string fid = AddGeom( "FUSELAGE", "" );             // Add Fuselage

    string xsec_surf = GetXSecSurf( fid, 0 );           // Get First (and Only) XSec Surf

    int num_xsecs = GetNumXSec( xsec_surf );

    //==== Set Tan Angles At Nose/Tail
    SetXSecTanAngles( GetXSec( xsec_surf, 0 ), XSEC_BOTH_SIDES, 90 );
    SetXSecTanAngles( GetXSec( xsec_surf, num_xsecs - 1 ), XSEC_BOTH_SIDES, -90 );

    Update();       // Force Surface Update
    \endcode
    \param update_managers Flag to indicate if managers should be updated
*/)";
    r = se->RegisterGlobalFunction( "void Update( bool update_managers = true)", asFUNCTION( vsp::Update ), asCALL_CDECL, doc_struct );
    assert( r >= 0 );

    doc_struct.comment = R"(
/*!
    Exit the program with a specific error code
    \param [in] error_code Error code
*/)";
    r = se->RegisterGlobalFunction( "void VSPExit( int error_code )", asFUNCTION( vsp::VSPExit ), asCALL_CDECL, doc_struct );
    assert( r >= 0 ); // TODO: Example

    doc_struct.comment = R"(
/*!
    Clear the current OpenVSP model
    \code{.cpp}
    string fid = AddGeom( "FUSELAGE", "" );             // Add Fuselage

    //==== Reset Geometry ====//
    Print( string( "--->Resetting VSP model to blank slate\n" ) );
    ClearVSPModel();
    \endcode
*/)";
    r = se->RegisterGlobalFunction( "void ClearVSPModel()", asFUNCTION( vsp::ClearVSPModel ), asCALL_CDECL, doc_struct );
    assert( r >= 0 );

    doc_struct.comment = R"(
/*!
    Get the file name of the current OpenVSP project
    \code{.cpp}
    string fid = AddGeom( "FUSELAGE", "" );             // Add Fuselage

    string fname = "example_fuse.vsp3";

    SetVSP3FileName( fname );

    Update();

    //==== Save Vehicle to File ====//
    Print( "\tSaving vehicle file to: ", false );

    Print( fname );

    WriteVSPFile( GetVSPFileName(), SET_ALL );
    \endcode
    \return File name for the current OpenVSP project
*/)";
    r = se->RegisterGlobalFunction( "string GetVSPFileName()", asFUNCTION( vsp::GetVSPFileName ), asCALL_CDECL, doc_struct );
    assert( r >= 0 );


    //==== File I/O Functions ====//
    group = "FileIO";
    doc_struct.group = group.c_str();

    group_description = R"(
    \brief This group of functions provides file input and output interfacing through the API. \n\n
    \ref index "Click here to return to the main page" )";
    se->AddGroup( group.c_str(), "File Input and Output Functions", group_description.c_str() );

    doc_struct.comment = R"(
/*!
    Load an OpenVSP project from a VSP3 file
    \code{.cpp}
    string fid = AddGeom( "FUSELAGE", "" );             // Add Fuselage

    string fname = "example_fuse.vsp3";

    SetVSP3FileName( fname );

    Update();

    //==== Save Vehicle to File ====//
    Print( "\tSaving vehicle file to: ", false );

    Print( fname );

    WriteVSPFile( GetVSPFileName(), SET_ALL );

    //==== Reset Geometry ====//
    Print( string( "--->Resetting VSP model to blank slate\n" ) );

    ClearVSPModel();

    ReadVSPFile( fname );
    \endcode
    \param [in] file_name *.vsp3 file name
*/)";
    r = se->RegisterGlobalFunction( "void ReadVSPFile( const string & in file_name )", asFUNCTION( vsp::ReadVSPFile ), asCALL_CDECL, doc_struct );
    assert( r >= 0 );

    doc_struct.comment = R"(
/*!
    Save the current OpenVSP project to a VSP3 file
    \code{.cpp}
    string fid = AddGeom( "FUSELAGE", "" );             // Add Fuselage

    string fname = "example_fuse.vsp3";

    SetVSP3FileName( fname );

    Update();

    //==== Save Vehicle to File ====//
    Print( "\tSaving vehicle file to: ", false );

    Print( fname );

    WriteVSPFile( GetVSPFileName(), SET_ALL );

    //==== Reset Geometry ====//
    Print( string( "--->Resetting VSP model to blank slate\n" ) );

    ClearVSPModel();

    ReadVSPFile( fname );
    \endcode
    \param [in] file_name *.vsp3 file name
    \param [in] set Set index to write (i.e. SET_ALL)
*/)";
    r = se->RegisterGlobalFunction( "void WriteVSPFile( const string & in file_name, int set )", asFUNCTION( vsp::WriteVSPFile ), asCALL_CDECL, doc_struct );
    assert( r >= 0 );

    doc_struct.comment = R"(
/*!
    Set the file name of a OpenVSP project
    \code{.cpp}
    string fid = AddGeom( "FUSELAGE", "" );             // Add Fuselage

    string fname = "example_fuse.vsp3";

    SetVSP3FileName( fname );

    Update();

    //==== Save Vehicle to File ====//
    Print( "\tSaving vehicle file to: ", false );

    Print( fname );

    WriteVSPFile( GetVSPFileName(), SET_ALL );

    //==== Reset Geometry ====//
    Print( string( "--->Resetting VSP model to blank slate\n" ) );

    ClearVSPModel();

    ReadVSPFile( fname );
    \endcode
    \param [in] file_name File name
*/)";
    r = se->RegisterGlobalFunction( "void SetVSP3FileName( const string & in file_name )", asFUNCTION( vsp::SetVSP3FileName ), asCALL_CDECL, doc_struct );
    assert( r >= 0 );

    doc_struct.comment = R"(
/*!
    Insert an external OpenVSP project into the current project. All Geoms in the external project are placed as children of the specified parent.
    If no parent or an invalid parent is given, the Geoms are inserted at the top level.
    \param [in] file_name *.vsp3 filename
    \param [in] parent Parent geom ID (ignored with empty string)
*/)";
    r = se->RegisterGlobalFunction( "void InsertVSPFile( const string & in file_name, const string & in parent )", asFUNCTION( vsp::InsertVSPFile ), asCALL_CDECL, doc_struct );
    assert( r >= 0 ); // TODO: Example

    doc_struct.comment = R"(
/*!
    Export a file from OpenVSP. Many formats are available, such as STL, IGES, and SVG. If a mesh is generated for a particular export, 
    the ID of the MeshGeom will be returned. If no mesh is generated an empty string will be returned. 
    \code{.cpp}
    string wid = AddGeom( "WING" );             // Add Wing

    ExportFile( "Airfoil_Metadata.csv", SET_ALL, EXPORT_SELIG_AIRFOIL );

    string mesh_id = ExportFile( "Example_Mesh.msh", SET_ALL, EXPORT_GMSH );
    DeleteGeom( mesh_id ); // Delete the mesh generated by the GMSH export
    \endcode
    \sa EXPORT_TYPE
    \param [in] file_name Export file name
    \param [in] thick_set Set index to export (i.e. SET_ALL)
    \param [in] file_type File type enum (i.e. EXPORT_IGES)
    \param [in] file_type File type enum (i.e. EXPORT_VSPGEOM)
    \return Mesh Geom ID if the export generates a mesh
*/)";
    r = se->RegisterGlobalFunction( "string ExportFile( const string & in file_name, int thick_set, int file_type, int thin_set = -1 )", asFUNCTION( vsp::ExportFile ), asCALL_CDECL, doc_struct );
    assert( r >= 0 );

    doc_struct.comment = R"(
/*!
    Import a file into OpenVSP. Many formats are available, such as NASCART, V2, and BEM). The imported Geom, mesh, or other object is inserted
    as a child of the specified parent. If no parent or an invalid parent is given, the import will be done at the top level.
    \sa IMPORT_TYPE
    \param [in] file_name Import file name
    \param [in] file_type File type enum (i.e. IMPORT_PTS)
    \param [in] parent Parent Geom ID (ignored with empty string)
*/)";
    r = se->RegisterGlobalFunction( "string ImportFile( const string & in file_name, int file_type, const string & in parent )", asFUNCTION( vsp::ImportFile ), asCALL_CDECL, doc_struct );
    assert( r >= 0 ); // TODO: Example

    doc_struct.comment = R"(
/*!
    Set the ID of the propeller to be exported to a BEM file. Call this function before ExportFile.
    \code{.cpp}
    //==== Add Prop Geometry ====//
    string prop_id = AddGeom( "PROP" );

    SetBEMPropID( prop_id );

    ExportFile( "ExampleBEM.bem", SET_ALL, EXPORT_BEM );
    \endcode
    \sa EXPORT_TYPE, ExportFile
    \param [in] prop_id Propeller Geom ID
*/)";
    r = se->RegisterGlobalFunction( "void SetBEMPropID( const string & in prop_id )", asFUNCTION( vsp::SetBEMPropID ), asCALL_CDECL, doc_struct );
    assert( r >= 0 );

    //==== Design File Functions ====//
    group = "DesignFile";
    doc_struct.group = group.c_str();

    group_description = R"(
    \brief This group of functions is available for managing Design Variables through the API. \n\n
    \ref index "Click here to return to the main page" )";
    se->AddGroup( group.c_str(), "Design File Functions", group_description.c_str() );

    doc_struct.comment = R"(
/*!
    Read in and apply a design file (*.des) to the current OpenVSP project
    \param [in] file_name *.des input file
*/)";
    r = se->RegisterGlobalFunction( "void ReadApplyDESFile( const string & in file_name )", asFUNCTION( vsp::ReadApplyDESFile ), asCALL_CDECL, doc_struct );
    assert( r >= 0 ); // TODO: Example

    doc_struct.comment = R"(
/*!
    Write all design variables to a design file (*.des)
    \param [in] file_name *.des output file
*/)";
    r = se->RegisterGlobalFunction( "void WriteDESFile( const string & in file_name )", asFUNCTION( vsp::WriteDESFile ), asCALL_CDECL, doc_struct );
    assert( r >= 0 ); // TODO: Example

    doc_struct.comment = R"(
/*!
    Read in and apply a Cart3D XDDM file (*.xddm) to the current OpenVSP project
    \param [in] file_name *.xddm input file
*/)";
    r = se->RegisterGlobalFunction( "void ReadApplyXDDMFile( const string & in file_name )", asFUNCTION( vsp::ReadApplyXDDMFile ), asCALL_CDECL, doc_struct );
    assert( r >= 0 ); // TODO: Example

    doc_struct.comment = R"(
/*!
    Write all design variables to a Cart3D XDDM file (*.xddm)
    \param [in] file_name *.xddm output file
*/)";
    r = se->RegisterGlobalFunction( "void WriteXDDMFile( const string & in file_name )", asFUNCTION( vsp::WriteXDDMFile ), asCALL_CDECL, doc_struct );
    assert( r >= 0 ); // TODO: Example

    doc_struct.comment = R"(
/*!
    Get the number of design variables
    \return int Number of design variables
*/)";
    r = se->RegisterGlobalFunction( "int GetNumDesignVars()", asFUNCTION( vsp::GetNumDesignVars ), asCALL_CDECL, doc_struct );
    assert( r >= 0 ); // TODO: Example

    doc_struct.comment = R"(
/*!
    Add a design variable
    \sa XDDM_QUANTITY_TYPE
    \param [in] parm_id Parm ID
    \param [in] type XDDM type enum (XDDM_VAR or XDDM_CONST)
*/)";
    r = se->RegisterGlobalFunction( "void AddDesignVar( const string & in parm_id, int type )", asFUNCTION( vsp::AddDesignVar ), asCALL_CDECL, doc_struct );
    assert( r >= 0 ); // TODO: Example

    doc_struct.comment = R"(
/*!
    Delete all design variables
*/)";
    r = se->RegisterGlobalFunction( "void DeleteAllDesignVars()", asFUNCTION( vsp::DeleteAllDesignVars ), asCALL_CDECL, doc_struct );
    assert( r >= 0 ); // TODO: Example

    doc_struct.comment = R"(
/*!
    Get the Parm ID of the specified design variable
    \param [in] index Index of design variable
    \return Parm ID
*/)";
    r = se->RegisterGlobalFunction( "string GetDesignVar( int index )", asFUNCTION( vsp::GetDesignVar ), asCALL_CDECL, doc_struct );
    assert( r >= 0 ); // TODO: Example

    doc_struct.comment = R"(
/*!
    Get the XDDM type of the specified design variable
    \sa XDDM_QUANTITY_TYPE
    \param [in] index Index of design variable
    \return XDDM type enum (XDDM_VAR or XDDM_CONST)
*/)";
    r = se->RegisterGlobalFunction( "int GetDesignVarType( int index )", asFUNCTION( vsp::GetDesignVarType ), asCALL_CDECL, doc_struct );
    assert( r >= 0 ); // TODO: Example

    //==== Computations ====//
    group = "Computations";
    doc_struct.group = group.c_str();

    group_description = R"(
    \brief The following group of API functions are available for general computations. In general, it is best practice to 
    perform computations through the the Analysis group instead of calling these functions directly. \n\n
    \ref index "Click here to return to the main page" )";
    se->AddGroup( group.c_str(), "General Computation Functions", group_description.c_str() );

    doc_struct.comment = R"(
/*!
    Compute mass properties for the components in the set. Alternatively can be run through the Analysis Manager with 'MassProp'.
    \code{.cpp}
    //==== Test Mass Props ====//
    string pid = AddGeom( "POD", "" );

    string mesh_id = ComputeMassProps( SET_ALL, 20 );

    string mass_res_id = FindLatestResultsID( "Mass_Properties" );

    array<double> @double_arr = GetDoubleResults( mass_res_id, "Total_Mass" );

    if ( double_arr.size() != 1 )                                    { Print( "---> Error: API ComputeMassProps" ); }
    \endcode
    \sa SetAnalysisInputDefaults, PrintAnalysisInputs, ExecAnalysis
    \param [in] set Set index (i.e. SET_ALL)
    \param [in] num_slices Number of slices
    \return MeshGeom ID
*/)";
    r = se->RegisterGlobalFunction( "string ComputeMassProps( int set, int num_slices )", asFUNCTION( vsp::ComputeMassProps ), asCALL_CDECL, doc_struct );
    assert( r >= 0 );

    doc_struct.comment = R"(
/*!
    Mesh, intersect, and trim components in the set. Alternatively can be run through the Analysis Manager with 'CompGeom'.
    \code{.cpp}
    //==== Add Pod Geom ====//
    string pid = AddGeom( "POD", "" );

    //==== Run CompGeom And Get Results ====//
    string mesh_id = ComputeCompGeom( SET_ALL, false, 0 );                      // Half Mesh false and no file export

    string comp_res_id = FindLatestResultsID( "Comp_Geom" );                    // Find Results ID

    array<double> @double_arr = GetDoubleResults( comp_res_id, "Wet_Area" );    // Extract Results
    \endcode
    \sa SetAnalysisInputDefaults, PrintAnalysisInputs, ExecAnalysis, COMPUTATION_FILE_TYPE
    \param [in] set Set index (i.e. SET_ALL)
    \param [in] half_mesh Flag to ignore surfaces on the negative side of the XZ plane (e.g. symmetry)
    \param [in] file_export_types CompGeom file type to export (supports XOR i.e. COMP_GEOM_CSV_TYPE & COMP_GEOM_TXT_TYPE )
    \return MeshGeom ID
*/)";
    r = se->RegisterGlobalFunction( "string ComputeCompGeom( int set, bool half_mesh, int file_export_types )", asFUNCTION( vsp::ComputeCompGeom ), asCALL_CDECL, doc_struct );
    assert( r >= 0 );

    doc_struct.comment = R"(
/*!
    Slice and mesh the components in the set. Alternatively can be run through the Analysis Manager with 'PlanarSlice'.
    \code{.cpp}
    //==== Add Pod Geom ====//
    string pid = AddGeom( "POD", "" );

    //==== Test Plane Slice ====//
    string slice_mesh_id = ComputePlaneSlice( 0, 6, vec3d( 0.0, 0.0, 1.0 ), true );

    string pslice_results = FindLatestResultsID( "Slice" );

    array<double> @double_arr = GetDoubleResults( pslice_results, "Slice_Area" );

    if ( double_arr.size() != 6 )                                    { Print( "---> Error: API ComputePlaneSlice" ); }
    \endcode
    \sa SetAnalysisInputDefaults, PrintAnalysisInputs, ExecAnalysis
    \param [in] set Set index (i.e. SET_ALL)
    \param [in] num_slices Number of slices
    \param [in] norm Normal axis for all slices
    \param [in] auto_bnd Flag to automatically set the start and end bound locations
    \param [in] start_bnd Location of the first slice along the normal axis (default: 0.0)
    \param [in] end_bnd Location of the last slice along the normal axis (default: 0.0)
    \return MeshGeom ID
*/)";
    r = se->RegisterGlobalFunction( "string ComputePlaneSlice( int set, int num_slices, const vec3d & in norm, bool auto_bnd, double start_bnd = 0, double end_bnd = 0 )", asFUNCTION( vsp::ComputePlaneSlice ), asCALL_CDECL, doc_struct );
    assert( r >= 0 );

    doc_struct.comment = R"(
/*!
    Compute the degenerate geometry representation for the components in the set. Alternatively can be run through the Analysis Manager with 'DegenGeom' or 'VSPAERODegenGeom'.
    \code{.cpp}
    //==== Set File Name ====//
    SetComputationFileName( DEGEN_GEOM_CSV_TYPE, "TestDegenScript.csv" );

    //==== Run Degen Geom ====//
    ComputeDegenGeom( SET_ALL, DEGEN_GEOM_CSV_TYPE );
    \endcode
    \sa SetAnalysisInputDefaults, PrintAnalysisInputs, ExecAnalysis, COMPUTATION_FILE_TYPE
    \param [in] set Set index (i.e. SET_ALL)
    \param [in] file_type DegenGeom file type to export (supports XOR i.e DEGEN_GEOM_M_TYPE & DEGEN_GEOM_CSV_TYPE)
*/)";
    r = se->RegisterGlobalFunction( "void ComputeDegenGeom( int set, int file_type )", asFUNCTION( vsp::ComputeDegenGeom ), asCALL_CDECL, doc_struct );
    assert( r >= 0 );

    //==== CFD Mesh ====//
    group = "CFDMesh";
    doc_struct.group = group.c_str();

    group_description = R"(
    \brief This group of functions is used to setup and run the CFD Mesh tool through the API. \n\n
    \ref index "Click here to return to the main page" )";
    se->AddGroup( group.c_str(), "CFD Mesh Functions", group_description.c_str() );

    doc_struct.comment = R"(
/*!
    Get the file name of a specified file type. Note, this function cannot be used to set FEA Mesh file names.
    \code{.cpp}
    //==== Set File Name ====//
    SetComputationFileName( DEGEN_GEOM_CSV_TYPE, "TestDegenScript.csv" );

    //==== Run Degen Geom ====//
    ComputeDegenGeom( SET_ALL, DEGEN_GEOM_CSV_TYPE );
    \endcode
    \sa COMPUTATION_FILE_TYPE, SetFeaMeshFileName
    \param [in] file_type File type enum (i.e. CFD_TRI_TYPE, COMP_GEOM_TXT_TYPE)
    \param [in] file_name File name
*/)";
    r = se->RegisterGlobalFunction( "void SetComputationFileName( int file_type, const string & in file_name )", asFUNCTION( vsp::SetComputationFileName ), asCALL_CDECL, doc_struct );
    assert( r >= 0 ); // TODO: FIXME for FEA Mesh

    doc_struct.comment = R"(
/*!
    Create a CFD Mesh for the components in the set. This analysis cannot be run through the Analysis Manager.
    \code{.cpp}
    //==== CFDMesh Method Facet Export =====//
    SetComputationFileName( CFD_FACET_TYPE, "TestCFDMeshFacet_API.facet" );

   Print( "\tComputing CFDMesh..." );

    ComputeCFDMesh( SET_ALL, CFD_FACET_TYPE );
    \endcode
    \sa COMPUTATION_FILE_TYPE
    \param [in] set Set index (i.e. SET_ALL)
    \param [in] file_type CFD Mesh file type to export (supports XOR i.e CFD_SRF_TYPE & CFD_STL_TYPE)
*/)";
    r = se->RegisterGlobalFunction( "void ComputeCFDMesh( int set, int file_type )", asFUNCTION( vsp::ComputeCFDMesh ), asCALL_CDECL, doc_struct );
    assert( r >= 0 );

    doc_struct.comment = R"(
/*!
    Set the value of a specific CFD Mesh option
    \code{.cpp}
    SetCFDMeshVal( CFD_MIN_EDGE_LEN, 1.0 );
    \endcode
    \sa CFD_CONTROL_TYPE
    \param [in] type CFD Mesh control type enum (i.e. CFD_GROWTH_RATIO)
    \param [in] val Value to set
*/)";
    r = se->RegisterGlobalFunction( "void SetCFDMeshVal( int type, double val )", asFUNCTION( vsp::SetCFDMeshVal ), asCALL_CDECL, doc_struct );
    assert( r >= 0 );

    doc_struct.comment = R"(
/*!
    Activate or deactivate the CFD Mesh wake for a particular Geom. Note, the wake flag is only applicable for wing-type surfaces. 
    Also, this function is simply an alternative to setting the value of the Parm with the available Parm setting API functions.
    \code{.cpp}
    //==== Add Wing Geom ====//
    string wid = AddGeom( "WING", "" );

    SetCFDWakeFlag( wid, true );
    // This is equivalent to SetParmValUpdate( wid, "Wake", "Shape", 1.0 );
    // To change the scale: SetParmValUpdate( wid, "WakeScale", "WakeSettings", 10.0 );
    // To change the angle: SetParmValUpdate( wid, "WakeAngle", "WakeSettings", -5.0 );
    \endcode
    \sa SetParmVal, SetParmValUpdate
    \param [in] geom_id Geom ID
    \param [in] flag True to activate, false to deactivate
*/)";
    r = se->RegisterGlobalFunction( "void SetCFDWakeFlag( const string & in geom_id, bool flag )", asFUNCTION( vsp::SetCFDWakeFlag ), asCALL_CDECL, doc_struct );
    assert( r >= 0 );

    doc_struct.comment = R"(
/*!
    Delete all CFD Mesh sources for all Geoms
    \code{.cpp}
    //==== Add Pod Geom ====//
    string pid = AddGeom( "POD", "" );

    AddCFDSource( POINT_SOURCE, pid, 0, 0.25, 2.0, 0.5, 0.5 );      // Add A Point Source

    DeleteAllCFDSources();
    \endcode
*/)";
    r = se->RegisterGlobalFunction( "void DeleteAllCFDSources()", asFUNCTION( vsp::DeleteAllCFDSources ), asCALL_CDECL, doc_struct );
    assert( r >= 0 );

    doc_struct.comment = R"(
/*!
    Add default CFD Mesh sources for all Geoms
    \code{.cpp}
    //==== Add Pod Geom ====//
    string pid = AddGeom( "POD", "" );

    AddDefaultSources(); // 3 Sources: Def_Fwd_PS, Def_Aft_PS, Def_Fwd_Aft_LS
    \endcode
*/)";
    r = se->RegisterGlobalFunction( "void AddDefaultSources()", asFUNCTION( vsp::AddDefaultSources ), asCALL_CDECL, doc_struct );
    assert( r >= 0 );
    
    doc_struct.comment = R"(
/*!
    Add a CFD Mesh default source for the indicated Geom. Note, certain input params may not be used depending on the source type
    \code{.cpp}
    //==== Add Pod Geom ====//
    string pid = AddGeom( "POD", "" );

    AddCFDSource( POINT_SOURCE, pid, 0, 0.25, 2.0, 0.5, 0.5 );      // Add A Point Source
    \endcode
    \sa CFD_MESH_SOURCE_TYPE
    \param [in] type CFD Mesh source type( i.e.BOX_SOURCE )
    \param [in] geom_id Geom ID
    \param [in] surf_index Main surface index
    \param [in] l1 Source first edge length
    \param [in] r1 Source first radius
    \param [in] u1 Source first U location
    \param [in] w1 Source first W location
    \param [in] l2 Source second edge length
    \param [in] r2 Source second radius
    \param [in] u2 Source second U location
    \param [in] w2 Source second W location
*/)";
    r = se->RegisterGlobalFunction(
        "void AddCFDSource( int type, const string & in geom_id, int surf_index, double l1, double r1, double u1, double w1, double l2 = 0, double r2 = 0, double u2 = 0, double w2 = 0 )",
        asFUNCTION( vsp::AddCFDSource ), asCALL_CDECL, doc_struct );
    assert( r >= 0 );


    //==== Analysis Functions ====//
    group = "Analysis";
    doc_struct.group = group.c_str();

    group_description = R"(
    \brief This group is for functions included in the Analysis Manager. The Analysis Manager allows for 
    OpenVSP analyses to be setup and run through the API without having to modify Parms directly. Examples
    are available for every available analysis type. The results of running an analysis can be accessed
    through the functions defined in the Results group. \n\n
    \ref index "Click here to return to the main page" )";
    se->AddGroup( group.c_str(), "Analysis Manager Functions", group_description.c_str() );

    doc_struct.comment = R"(
/*!
    Get the number of analysis types available in the Analysis Manager
    \code{.cpp}
    int nanalysis = GetNumAnalysis();

    Print( "Number of registered analyses: " + nanalysis );
    \endcode
    \return Number of analyses
*/)";
    r = se->RegisterGlobalFunction( "int GetNumAnalysis( )", asFUNCTION( vsp::GetNumAnalysis ), asCALL_CDECL, doc_struct );
    assert( r >= 0 );

    doc_struct.comment = R"(
/*!
    Get the name of every available analysis in the Analysis Manager
    \code{.cpp}
    array< string > @analysis_array = ListAnalysis();

    Print( "List of Available Analyses: " );

    for ( int i = 0; i < int( analysis_array.size() ); i++ )
    {
        Print( "    " + analysis_array[i] );
    }
    \endcode
    \return Array of analysis names
*/)";
    r = se->RegisterGlobalFunction( "array<string>@ ListAnalysis()", asMETHOD( ScriptMgrSingleton, ListAnalysis ), asCALL_THISCALL_ASGLOBAL, &ScriptMgr, doc_struct );
    assert( r >= 0 );

    doc_struct.comment = R"(
/*!
    Get the name of every available input for a particular analysis
    \code{.cpp}
    string analysis_name = "VSPAEROComputeGeometry";

    array<string>@ in_names =  GetAnalysisInputNames( analysis_name );

    Print("Analysis Inputs: ");

    for ( int i = 0; i < int( in_names.size() ); i++)
    {
        Print( ( "\t" + in_names[i] + "\n" ) );
    }
    \endcode
    \param [in] analysis Snalysis name
    \return Array of input names
*/)";
    r = se->RegisterGlobalFunction( "array<string>@ GetAnalysisInputNames(const string & in analysis )", asMETHOD( ScriptMgrSingleton, GetAnalysisInputNames ), asCALL_THISCALL_ASGLOBAL, &ScriptMgr, doc_struct );
    assert( r >= 0 );

    doc_struct.comment = R"(
/*!
    Execute an analysis through the Analysis Manager
    \code{.cpp}
    string analysis_name = "VSPAEROComputeGeometry";

    string res_id = ExecAnalysis( analysis_name );
    \endcode
    \param [in] analysis Snalysis name
    \return Result ID
*/)";
    r = se->RegisterGlobalFunction( "string ExecAnalysis( const string & in analysis )", asFUNCTION( vsp::ExecAnalysis ), asCALL_CDECL, doc_struct );
    assert( r >= 0 );

    doc_struct.comment = R"(
/*!
    Get the number of input data for the particular analysis type and input
    \param [in] analysis Analysis name
    \param [in] name Input name
    \return Number of input data
*/)";
    r = se->RegisterGlobalFunction( "int GetNumAnalysisInputData( const string & in analysis, const string & in name )", asFUNCTION( vsp::GetNumAnalysisInputData ), asCALL_CDECL, doc_struct );
    assert( r >= 0 );

    doc_struct.comment = R"(
/*!
    Get the data type for a particulat analysis type and input
    \code{.cpp}
    string analysis = "VSPAEROComputeGeometry";

    array < string > @ inp_array = GetAnalysisInputNames( analysis );

    for ( int j = 0; j < int( inp_array.size() ); j++ )
    {
        int typ = GetAnalysisInputType( analysis, inp_array[j] );
    }
    \endcode
    \sa RES_DATA_TYPE
    \param [in] analysis Analysis name
    \param [in] name Input name
    \return int Data type enum (i.e. DOUBLE_DATA)
*/)";
    r = se->RegisterGlobalFunction( "int GetAnalysisInputType( const string & in analysis, const string & in name )", asFUNCTION( vsp::GetAnalysisInputType ), asCALL_CDECL, doc_struct );
    assert( r >= 0 );

    doc_struct.comment = R"(
/*!
    Get the current integer values for the particular analysis, input, and data index
    \code{.cpp}
    //==== Analysis: VSPAero Compute Geometry ====//
    string analysis_name = "VSPAEROComputeGeometry";

    // Set to panel method
    array< int > analysis_method = GetIntAnalysisInput( analysis_name, "AnalysisMethod" );

    analysis_method[0] = ( VSPAERO_ANALYSIS_METHOD::VORTEX_LATTICE );

    SetIntAnalysisInput( analysis_name, "AnalysisMethod", analysis_method );
    \endcode
    \sa RES_DATA_TYPE, SetIntAnalysisInput
    \param [in] analysis Analysis name
    \param [in] name Input name
    \param [in] index Data index
    \return Array of analysis input values
*/)";
    r = se->RegisterGlobalFunction( "array<int>@ GetIntAnalysisInput( const string & in analysis, const string & in name, int index = 0 )", asMETHOD( ScriptMgrSingleton, GetIntAnalysisInput ), asCALL_THISCALL_ASGLOBAL, &ScriptMgr, doc_struct );
    assert( r >= 0 );

    doc_struct.comment = R"(
/*!
    Get the current double values for the particular analysis, input, and data index
    \code{.cpp}
    array<double> vinfFCinput = GetDoubleAnalysisInput( "ParasiteDrag", "Vinf" );

    vinfFCinput[0] = 629;

    SetDoubleAnalysisInput( "ParasiteDrag", "Vinf", vinfFCinput );
    \endcode
    \sa RES_DATA_TYPE, SetDoubleAnalysisInput
    \param [in] analysis Analysis name
    \param [in] name Input name
    \param [in] index Data index
    \return Array of analysis input values
*/)";
    r = se->RegisterGlobalFunction( "array<double>@ GetDoubleAnalysisInput( const string & in analysis, const string & in name, int index = 0 )", asMETHOD( ScriptMgrSingleton, GetDoubleAnalysisInput ), asCALL_THISCALL_ASGLOBAL, &ScriptMgr, doc_struct );
    assert( r >= 0 );

    doc_struct.comment = R"(
/*!
    Get the current string values for the particular analysis, input, and data index
    \code{.cpp}
    array<string> fileNameInput = GetStringAnalysisInput( "ParasiteDrag", "FileName" );

    fileNameInput[0] = "ParasiteDragExample";

    SetStringAnalysisInput( "ParasiteDrag", "FileName", fileNameInput );
    \endcode
    \sa RES_DATA_TYPE, SetStringAnalysisInput
    \param [in] analysis Analysis name
    \param [in] name Input name
    \param [in] index Data index
    \return Array of analysis input values
*/)";
    r = se->RegisterGlobalFunction( "array<string>@ GetStringAnalysisInput( const string & in analysis, const string & in name, int index = 0 )", asMETHOD( ScriptMgrSingleton, GetStringAnalysisInput ), asCALL_THISCALL_ASGLOBAL, &ScriptMgr, doc_struct );
    assert( r >= 0 );

    doc_struct.comment = R"(
/*!
    Get the current vec3d values for the particular analysis, input, and data index
    \code{.cpp}
    // PlanarSlice
    array<vec3d> norm = GetVec3dAnalysisInput( "PlanarSlice", "Norm" );

    norm[0].set_xyz( 0.23, 0.6, 0.15 );

    SetVec3dAnalysisInput( "PlanarSlice", "Norm", norm );
    \endcode
    \sa RES_DATA_TYPE, SetVec3dAnalysisInput
    \param [in] analysis Analysis name
    \param [in] name Input name
    \param [in] index Data index
    \return Array of analysis input values
*/)";
    r = se->RegisterGlobalFunction( "array<vec3d>@ GetVec3dAnalysisInput( const string & in analysis, const string & in name, int index = 0 )", asMETHOD( ScriptMgrSingleton, GetVec3dAnalysisInput ), asCALL_THISCALL_ASGLOBAL, &ScriptMgr, doc_struct );
    assert( r >= 0 );

    doc_struct.comment = R"(
/*!
    Print to stdout all current input values for a specific analysis
    \code{.cpp}
    //==== Analysis: VSPAero Compute Geometry ====//
    string analysis_name = "VSPAEROComputeGeometry";

    // list inputs, type, and current values
    PrintAnalysisInputs( analysis_name );
    \endcode
    \param [in] analysis Analysis name
*/)";
    r = se->RegisterGlobalFunction( "void PrintAnalysisInputs( const string & in analysis )", asFUNCTION( vsp::PrintAnalysisInputs ), asCALL_CDECL, doc_struct );
    assert( r >= 0 );

    doc_struct.comment = R"(
/*!
   Set all input values to their defaults for a specific analysis
    \code{.cpp}
    //==== Analysis: VSPAero Compute Geometry ====//
    string analysis_name = "VSPAEROComputeGeometry";

    // Set defaults
    SetAnalysisInputDefaults( analysis_name );
    \endcode
    \param [in] analysis Analysis name
*/)";
    r = se->RegisterGlobalFunction( "void SetAnalysisInputDefaults( const string & in analysis )", asFUNCTION( vsp::SetAnalysisInputDefaults ), asCALL_CDECL, doc_struct );
    assert( r >= 0 );

    doc_struct.comment = R"(
/*!
    Set the value of a particular analysis input of integer type
    \code{.cpp}
    //==== Analysis: VSPAero Compute Geometry ====//
    string analysis_name = "VSPAEROComputeGeometry";

    // Set to panel method
    array< int > analysis_method = GetIntAnalysisInput( analysis_name, "AnalysisMethod" );

    analysis_method[0] = ( VSPAERO_ANALYSIS_METHOD::VORTEX_LATTICE );

    SetIntAnalysisInput( analysis_name, "AnalysisMethod", analysis_method );
    \endcode
    \sa GetIntAnalysisInput
    \param [in] analysis Analysis name
    \param [in] name Input name
    \param [in] indata_arr Array of integer values to set the input to
    \param [in] index Data index
*/)";
    r = se->RegisterGlobalFunction( "void SetIntAnalysisInput( const string & in analysis, const string & in name, array<int>@ indata_arr, int index = 0 )", asMETHOD( ScriptMgrSingleton, SetIntAnalysisInput ), asCALL_THISCALL_ASGLOBAL, &ScriptMgr, doc_struct );
    assert( r >= 0 );

    doc_struct.comment = R"(
/*!
    Set the value of a particular analysis input of double type
    \code{.cpp}
    //==== Analysis: CpSlicer ====//
    string analysis_name = "CpSlicer";

    // Setup cuts
    array < double > ycuts;
    ycuts.push_back( 2.0 );
    ycuts.push_back( 4.5 );
    ycuts.push_back( 8.0 );
    
    SetDoubleAnalysisInput( analysis_name, "YSlicePosVec", ycuts, 0 );
    \endcode
    \sa GetDoubleAnalysisInput
    \param [in] analysis Analysis name
    \param [in] name Input name
    \param [in] indata_arr Array of double values to set the input to
    \param [in] index Data index
*/)";
    r = se->RegisterGlobalFunction( "void SetDoubleAnalysisInput( const string & in analysis, const string & in name, array<double>@ indata_arr, int index = 0 )", asMETHOD( ScriptMgrSingleton, SetDoubleAnalysisInput ), asCALL_THISCALL_ASGLOBAL, &ScriptMgr, doc_struct );
    assert( r >= 0 );

    doc_struct.comment = R"(
/*!
    Set the value of a particular analysis input of string type
    \code{.cpp}
    array<string> fileNameInput = GetStringAnalysisInput( "ParasiteDrag", "FileName" );

    fileNameInput[0] = "ParasiteDragExample";

    SetStringAnalysisInput( "ParasiteDrag", "FileName", fileNameInput );
    \endcode
    \sa GetStringAnalysisInput
    \param [in] analysis Analysis name
    \param [in] name Input name
    \param [in] indata_arr Array of string values to set the input to
    \param [in] index Data index
*/)";
    r = se->RegisterGlobalFunction( "void SetStringAnalysisInput( const string & in analysis, const string & in name, array<string>@ indata_arr, int index = 0 )", asMETHOD( ScriptMgrSingleton, SetStringAnalysisInput ), asCALL_THISCALL_ASGLOBAL, &ScriptMgr, doc_struct );
    assert( r >= 0 );

    doc_struct.comment = R"(
/*!
    Set the value of a particular analysis input of vec3d type
    \code{.cpp}
    // PlanarSlice
    array<vec3d> norm = GetVec3dAnalysisInput( "PlanarSlice", "Norm" );

    norm[0].set_xyz( 0.23, 0.6, 0.15 );

    SetVec3dAnalysisInput( "PlanarSlice", "Norm", norm );
    \endcode
    \sa GetVec3dAnalysisInput
    \param [in] analysis Analysis name
    \param [in] name Input name
    \param [in] indata_arr Array of vec3d values to set the input to
    \param [in] index int Data index
*/)";
    r = se->RegisterGlobalFunction( "void SetVec3dAnalysisInput( const string & in analysis, const string & in name, array<vec3d>@ indata_arr, int index = 0 )", asMETHOD( ScriptMgrSingleton, SetVec3dAnalysisInput ), asCALL_THISCALL_ASGLOBAL, &ScriptMgr, doc_struct );
    assert( r >= 0 );


    //==== Results Functions ====//
    group = "Results";
    doc_struct.group = group.c_str();

    group_description = R"(
    \brief This group is for functions included in the Results Manager. The Results Manager stores
    analysis results and provides methods to get, print, and export them. \n\n
    \ref index "Click here to return to the main page" )";
    se->AddGroup( group.c_str(), "Results Manager Functions", group_description.c_str() );

    doc_struct.comment = R"(
/*!
    Get the number of results for a particular result name
    \code{.cpp}
    //==== Write Some Fake Test Results =====//
    WriteTestResults();

    if ( GetNumResults( "Test_Results" ) != 2 )                { Print( "---> Error: API GetNumResults" ); }
    \endcode
    \param [in] name Input name
    \return Number of results
*/)";
    r = se->RegisterGlobalFunction( "int GetNumResults( const string & in name )", asFUNCTION( vsp::GetNumResults ), asCALL_CDECL, doc_struct );
    assert( r >= 0 );

    doc_struct.comment = R"(
/*!
    Get the name of a result given its ID
    \code{.cpp}
    //==== Analysis: VSPAero Compute Geometry ====//
    string analysis_name = "VSPAEROComputeGeometry";

    // Set defaults
    SetAnalysisInputDefaults( analysis_name );

    string res_id = ( ExecAnalysis( analysis_name ) );

    Print( "Results Name: ", false );

    Print( GetResultsName( res_id ) );
    \endcode
    \param [in] results_id Result ID
    \return Result name
*/)";
    r = se->RegisterGlobalFunction( "string GetResultsName( const string & in results_id )", asFUNCTION( vsp::GetResultsName ), asCALL_CDECL, doc_struct );
    assert( r >= 0 );

    doc_struct.comment = R"(
/*!
    Find a results ID given its name and index
    \code{.cpp}
    //==== Write Some Fake Test Results =====//
    WriteTestResults();

    string res_id = FindResultsID( "Test_Results" );

    if ( res_id.size() == 0 )                                { Print( "---> Error: API FindResultsID" ); }
    \endcode
    \param [in] name Result name
    \param [in] index Result index
    \return Result ID
*/)";
    r = se->RegisterGlobalFunction( "string FindResultsID( const string & in name, int index = 0 )", asFUNCTION( vsp::FindResultsID ), asCALL_CDECL, doc_struct );
    assert( r >= 0 );

    doc_struct.comment = R"(
/*!
    Find the latest results ID for particular result name
    \code{.cpp}
    //==== Write Some Fake Test Results =====//
    WriteTestResults();

    array< string > @results_array = GetAllResultsNames();

    for ( int i = 0; i < int( results_array.size() ); i++ )
    {
        string resid = FindLatestResultsID( results_array[i] );
        PrintResults( resid );
    }
    \endcode
    \param [in] name Result name
    \return Result ID
*/)";
    r = se->RegisterGlobalFunction( "string FindLatestResultsID( const string & in name )", asFUNCTION( vsp::FindLatestResultsID ), asCALL_CDECL, doc_struct );
    assert( r >= 0 );

    doc_struct.comment = R"(
/*!
    Get the number of data values for a given result ID and data name
    \code{.cpp}
    //==== Write Some Fake Test Results =====//
    WriteTestResults();

    string res_id = FindResultsID( "Test_Results" );

    if ( GetNumData( res_id, "Test_Int" ) != 2 )            { Print( "---> Error: API GetNumData " ); }
    
    array<int> @int_arr = GetIntResults( res_id, "Test_Int", 0 );

    if ( int_arr[0] != 1 )                                    { Print( "---> Error: API GetIntResults" ); }
    
    int_arr = GetIntResults( res_id, "Test_Int", 1 );

    if ( int_arr[0] != 2 )                                    { Print( "---> Error: API GetIntResults" ); }
    \endcode
    \param [in] results_id Result ID
    \param [in] data_name Data name
    \return Number of data values
*/)";
    r = se->RegisterGlobalFunction( "int GetNumData( const string & in results_id, const string & in data_name )", asFUNCTION( vsp::GetNumData ), asCALL_CDECL, doc_struct );
    assert( r >= 0 );

    doc_struct.comment = R"(
/*!
    Get the data type for a given result ID and data name
    \code{.cpp}
    //==== Write Some Fake Test Results =====//
    WriteTestResults();

    string res_id = FindResultsID( "Test_Results" );

    array < string > @ res_array = GetAllDataNames( res_id );

    for ( int j = 0; j < int( res_array.size() ); j++ )
    {
        int typ = GetResultsType( res_id, res_array[j] );
    }
    \endcode
    \sa RES_DATA_TYPE
    \param [in] results_id Result ID
    \param [in] data_name Data name
    \return Data type enum (i.e. DOUBLE_DATA)
*/)";
    r = se->RegisterGlobalFunction( "int GetResultsType( const string & in results_id, const string & in data_name )", asFUNCTION( vsp::GetResultsType ), asCALL_CDECL, doc_struct );
    assert( r >= 0 );

    doc_struct.comment = R"(
/*!
    Get the name of all results in the Results Manager
    \code{.cpp}
    //==== Write Some Fake Test Results =====//
    WriteTestResults();

    array< string > @results_array = GetAllResultsNames();

    for ( int i = 0; i < int( results_array.size() ); i++ )
    {
        string resid = FindLatestResultsID( results_array[i] );
        PrintResults( resid );
    }
    \endcode
    \return Array of result names
*/)";
    r = se->RegisterGlobalFunction( "array<string>@ GetAllResultsNames()", asMETHOD( ScriptMgrSingleton, GetAllResultsNames ), asCALL_THISCALL_ASGLOBAL, &ScriptMgr, doc_struct );
    assert( r >= 0 );

    doc_struct.comment = R"(
/*!
    Get all data names for a particular result
    \code{.cpp}
    //==== Write Some Fake Test Results =====//
    WriteTestResults();

    string res_id = FindResultsID( "Test_Results" );

    array< string > @data_names = GetAllDataNames( res_id );

    if ( data_names.size() != 5 )                            { Print( "---> Error: API GetAllDataNames" ); }
    \endcode
    \param [in] results_id Result ID
    \return Array of result names
*/)";
    r = se->RegisterGlobalFunction( "array<string>@ GetAllDataNames(const string & in results_id )", asMETHOD( ScriptMgrSingleton, GetAllDataNames ), asCALL_THISCALL_ASGLOBAL, &ScriptMgr, doc_struct );
    assert( r >= 0 );

    doc_struct.comment = R"(
/*!
    Get all integer values for a particular result, name, and index
    \code{.cpp}
    //==== Write Some Fake Test Results =====//
    WriteTestResults();

    string res_id = FindResultsID( "Test_Results" );

    if ( GetNumData( res_id, "Test_Int" ) != 2 )            { Print( "---> Error: API GetNumData " ); }
    
    array<int> @int_arr = GetIntResults( res_id, "Test_Int", 0 );

    if ( int_arr[0] != 1 )                                    { Print( "---> Error: API GetIntResults" ); }
    
    int_arr = GetIntResults( res_id, "Test_Int", 1 );

    if ( int_arr[0] != 2 )                                    { Print( "---> Error: API GetIntResults" ); }
    \endcode
    \param [in] id Result ID
    \param [in] name Data name
    \param [in] index Data index
    \return Array of data values
*/)";
    r = se->RegisterGlobalFunction( "array<int>@ GetIntResults( const string & in id, const string & in name, int index = 0 )", asMETHOD( ScriptMgrSingleton, GetIntResults ), asCALL_THISCALL_ASGLOBAL, &ScriptMgr, doc_struct );
    assert( r >= 0 );

    doc_struct.comment = R"(
/*!
    Get all double values for a particular result, name, and index
    \code{.cpp}
    //==== Add Pod Geom ====//
    string pid = AddGeom( "POD", "" );

    //==== Run CompGeom And View Results ====//
    string mesh_id = ComputeCompGeom( SET_ALL, false, 0 );                      // Half Mesh false and no file export

    string comp_res_id = FindLatestResultsID( "Comp_Geom" );                    // Find Results ID

    array<double> @double_arr = GetDoubleResults( comp_res_id, "Wet_Area" );    // Extract Results
    \endcode
    \param [in] id Result ID
    \param [in] name Data name
    \param [in] index Data index
    \return Array of data values
*/)";
    r = se->RegisterGlobalFunction( "array<double>@ GetDoubleResults( const string & in id, const string & in name, int index = 0 )", asMETHOD( ScriptMgrSingleton, GetDoubleResults ), asCALL_THISCALL_ASGLOBAL, &ScriptMgr, doc_struct );
    assert( r >= 0 );

    doc_struct.comment = R"(
/*!
    Get all matrix (vector<vector<double>>) values for a particular result, name, and index
    \param [in] id Result ID
    \param [in] name Data name
    \param [in] index Data index
    \return 2D array of data values
*/)";
    r = se->RegisterGlobalFunction( "array<array<double>@>@ GetDoubleMatResults( const string & in id, const string & in name, int index = 0 )", asMETHOD( ScriptMgrSingleton, GetDoubleMatResults ), asCALL_THISCALL_ASGLOBAL, &ScriptMgr, doc_struct );
    assert( r >= 0 ); // TODO: Example

    doc_struct.comment = R"(
/*!
    Get all string values for a particular result, name, and index
    \code{.cpp}
    //==== Write Some Fake Test Results =====//
    WriteTestResults();

    string res_id = FindResultsID( "Test_Results" );

    array<string> @str_arr = GetStringResults( res_id, "Test_String" );

    if ( str_arr[0] != "This Is A Test" )                    { Print( "---> Error: API GetStringResults" ); }
    \endcode
    \param [in] id Result ID
    \param [in] name Data name
    \param [in] index Data index
    \return Array of data values
*/)";
    r = se->RegisterGlobalFunction( "array<string>@ GetStringResults( const string & in id, const string & in name, int index = 0 )", asMETHOD( ScriptMgrSingleton, GetStringResults ), asCALL_THISCALL_ASGLOBAL, &ScriptMgr, doc_struct );
    assert( r >= 0 );

    doc_struct.comment = R"(
/*!
    Get all vec3d values for a particular result, name, and index
    \code{.cpp}
    //==== Write Some Fake Test Results =====//

    double tol = 0.00001;

    WriteTestResults();

    string res_id = FindLatestResultsID( "Test_Results" );

    array<vec3d> @vec3d_vec = GetVec3dResults( res_id, "Test_Vec3d" );

    Print( "X: ", false );
    Print( vec3d_vec[0].x(), false );

    Print( "\tY: ", false );
    Print( vec3d_vec[0].y(), false );

    Print( "\tZ: ", false );
    Print( vec3d_vec[0].z() );
    \endcode
    \param [in] id Result ID
    \param [in] name Data name
    \param [in] index Data index
    \return Array of data values
*/)";
    r = se->RegisterGlobalFunction( "array<vec3d>@ GetVec3dResults( const string & in id, const string & in name, int index = 0 )", asMETHOD( ScriptMgrSingleton, GetVec3dResults ), asCALL_THISCALL_ASGLOBAL, &ScriptMgr, doc_struct );
    assert( r >= 0 );
    
    doc_struct.comment = R"(
/*!
    Create a new result for a Geom
    \code{.cpp}
    //==== Test Comp Geom ====//
    string gid1 = AddGeom( "POD", "" );

    string mesh_id = ComputeCompGeom( 0, false, 0 );

    //==== Test Comp Geom Mesh Results ====//
    string mesh_geom_res_id = CreateGeomResults( mesh_id, "Comp_Mesh" );

    array<int> @int_arr = GetIntResults( mesh_geom_res_id, "Num_Tris" );

    if ( int_arr[0] < 4 )                                            { Print( "---> Error: API CreateGeomResults" ); }
    \endcode
    \param [in] geom_id Geom ID
    \param [in] name Result name
    \return Result ID
*/)";
    r = se->RegisterGlobalFunction( "string CreateGeomResults( const string & in geom_id, const string & in name )", asFUNCTION( vsp::CreateGeomResults ), asCALL_CDECL, doc_struct );
    assert( r >= 0 );

    doc_struct.comment = R"(
/*!
    Delete all results
    \code{.cpp}
    //==== Test Comp Geom ====//
    string gid1 = AddGeom( "POD", "" );

    string mesh_id = ComputeCompGeom( 0, false, 0 );

    //==== Test Comp Geom Mesh Results ====//
    string mesh_geom_res_id = CreateGeomResults( mesh_id, "Comp_Mesh" );

    DeleteAllResults();

    if ( GetNumResults( "Comp_Mesh" ) != 0 )                { Print( "---> Error: API DeleteAllResults" ); }
    \endcode
*/)";
    r = se->RegisterGlobalFunction( "void DeleteAllResults()", asFUNCTION( vsp::DeleteAllResults ), asCALL_CDECL, doc_struct );
    assert( r >= 0 );

    doc_struct.comment = R"(
/*!
    Delete a particular result
    \code{.cpp}
    //==== Test Comp Geom ====//
    string gid1 = AddGeom( "POD", "" );

    string mesh_id = ComputeCompGeom( 0, false, 0 );

    //==== Test Comp Geom Mesh Results ====//
    string mesh_geom_res_id = CreateGeomResults( mesh_id, "Comp_Mesh" );

    DeleteResult( mesh_geom_res_id );

    if ( GetNumResults( "Comp_Mesh" ) != 0 )                { Print( "---> Error: API DeleteResult" ); }
    \endcode
    \param [in] id Result ID
*/)";
    r = se->RegisterGlobalFunction( "void DeleteResult( const string & in id )", asFUNCTION( vsp::DeleteResult ), asCALL_CDECL, doc_struct );
    assert( r >= 0 );

    doc_struct.comment = R"(
/*!
    Export a result to CSV
    \code{.cpp}
    // Add Pod Geom
    string pid = AddGeom( "POD" );

    string analysis_name = "VSPAEROComputeGeometry";

    string rid = ExecAnalysis( analysis_name );

    WriteResultsCSVFile( rid, "CompGeomRes.csv" );
    \endcode
    \param [in] id Rsult ID
    \param [in] file_name CSV output file name
*/)";
    r = se->RegisterGlobalFunction( "void WriteResultsCSVFile( const string & in id, const string & in file_name )", asFUNCTION( vsp::WriteResultsCSVFile ), asCALL_CDECL, doc_struct );
    assert( r >= 0 );

    doc_struct.comment = R"(
/*!
    Print a result's name value pairs to stdout
    \code{.cpp}
    // Add Pod Geom
    string pid = AddGeom( "POD" );

    string analysis_name = "VSPAEROComputeGeometry";

    string rid = ExecAnalysis( analysis_name );

    // Get & Display Results
    PrintResults( rid );
    \endcode
    \param [in] id Result ID
*/)";
    r = se->RegisterGlobalFunction( "void PrintResults( const string & in id )", asFUNCTION( vsp::PrintResults ), asCALL_CDECL, doc_struct );
    assert( r >= 0 );

    doc_struct.comment = R"(
/*!
    Create two sets of test results, each containing int, string, vec3d, double, and vector< double > data types. 
    \code{.cpp}
    //==== Write Some Fake Test Results =====//
    WriteTestResults();

    if ( GetNumResults( "Test_Results" ) != 2 )                { Print( "---> Error: API GetNumResults" ); }
    \endcode
*/)";
    r = se->RegisterGlobalFunction( "void WriteTestResults()", asMETHOD( ResultsMgrSingleton, WriteTestResults ), asCALL_THISCALL_ASGLOBAL, &ResultsMgr, doc_struct );
    assert( r >= 0 );


    //==== Geom Functions ====//
    group = "Geom";
    doc_struct.group = group.c_str();

    group_description = R"(
    \brief This group of functions is available for adding, deleting, and modifying OpenVSP Geoms through the API. \n\n
    \ref index "Click here to return to the main page" )";
    se->AddGroup( group.c_str(), "Geom Functions", group_description.c_str() );

    doc_struct.comment = R"(
/*!
    Get an array of all Geom types (i.e FUSELAGE, POD, etc.)
    \code{.cpp}
        //==== Add Pod Geometries ====//
    string pod1 = AddGeom( "POD", "" );
    string pod2 = AddGeom( "POD", "" );

    array< string > @type_array = GetGeomTypes();

    if ( type_array[0] != "POD" )                { Print( "---> Error: API GetGeomTypes  " ); }
    \endcode
    \return Array of Geom type names
*/)";
    r = se->RegisterGlobalFunction( "array<string>@ GetGeomTypes()", asMETHOD( ScriptMgrSingleton, GetGeomTypes ), asCALL_THISCALL_ASGLOBAL, &ScriptMgr, doc_struct );
    assert( r >= 0 );

    doc_struct.comment = R"(
/*!
    Add a new Geom of given type as a child of the specified parent. If no parent or an invalid parent is given, the Geom is placed at the top level 
    \code{.cpp}
    //==== Add Wing Geometry ====//
    string wing_id = AddGeom( "WING" );
    \endcode
    \param [in] type Geom type (i.e FUSELAGE, POD, etc.)
    \param [in] parent Parent Geom ID
    \return Geom ID
*/)";
    r = se->RegisterGlobalFunction( "string AddGeom( const string & in type, const string & in parent = string() )", asFUNCTION( vsp::AddGeom ), asCALL_CDECL, doc_struct );
    assert( r >= 0 );

    doc_struct.comment = R"(
/*!
    Perform an update for the specified Geom
    \code{.cpp}
    //==== Add Wing Geometry ====//
    string wing_id = AddGeom( "WING" );

    //==== Add Pod Geometry ====//
    string pod_id = AddGeom( "POD" );

    SetParmVal( pod_id, "X_Rel_Location", "XForm", 5.0 );

    UpdateGeom( pod_id ); // Faster than updating the whole vehicle
    \endcode
    \sa Update()
    \param [in] geom_id Geom ID
*/)";
    r = se->RegisterGlobalFunction( "void UpdateGeom(const string & in geom_id)", asFUNCTION( vsp::UpdateGeom ), asCALL_CDECL, doc_struct );
    assert( r >= 0 );

    doc_struct.comment = R"(
/*!
    Delete a particular Geom
    \code{.cpp}
    //==== Add Wing Geometry ====//
    string wing_id = AddGeom( "WING" );

    //==== Add Pod Geometry ====//
    string pod_id = AddGeom( "POD" );

    DeleteGeom( wing_id );
    \endcode
    \param [in] geom_id Geom ID
*/)";
    r = se->RegisterGlobalFunction( "void DeleteGeom(const string & in geom_id)", asFUNCTION( vsp::DeleteGeom ), asCALL_CDECL, doc_struct );
    assert( r >= 0 );

    doc_struct.comment = R"(
/*!
    Delete multiple Geoms
    \code{.cpp}
    //==== Add Pod Geometry ====//
    string pid = AddGeom( "POD", "" );

    string rid = ExecAnalysis( "CompGeom" );

    array<string>@ mesh_id_vec = GetStringResults( rid, "Mesh_GeomID" );

    DeleteGeomVec( mesh_id_vec );
    \endcode
    \param [in] del_arr Array of Geom IDs
*/)";
    r = se->RegisterGlobalFunction( "void DeleteGeomVec( array<string>@ del_arr )", asMETHOD( ScriptMgrSingleton, DeleteGeomVec ), asCALL_THISCALL_ASGLOBAL, &ScriptMgr, doc_struct );
    assert( r >= 0 );

    doc_struct.comment = R"(
/*!
    Cut Geom from current location and store on clipboard
    \code{.cpp}
    //==== Add Pod Geometries ====//
    string pid1 = AddGeom( "POD", "" );
    string pid2 = AddGeom( "POD", "" );

    CutGeomToClipboard( pid1 );

    PasteGeomClipboard( pid2 ); // Paste Pod 1 as child of Pod 2

    array< string > @geom_ids = FindGeoms();

    if ( geom_ids.size() != 2 )                { Print( "---> Error: API Cut/Paste Geom  " ); }
    \endcode
    \sa PasteGeomClipboard
    \param [in] geom_id Geom ID
*/)";
    r = se->RegisterGlobalFunction( "void CutGeomToClipboard(const string & in geom_id)", asFUNCTION( vsp::CutGeomToClipboard ), asCALL_CDECL, doc_struct );
    assert( r >= 0 );

    doc_struct.comment = R"(
/*!
    Copy Geom from current location and store on clipboard
    \code{.cpp}
    //==== Add Pod Geometries ====//
    string pid1 = AddGeom( "POD", "" );
    string pid2 = AddGeom( "POD", "" );

    CopyGeomToClipboard( pid1 );

    PasteGeomClipboard( pid2 ); // Paste Pod 1 as child of Pod 2

    array< string > @geom_ids = FindGeoms();

    if ( geom_ids.size() != 3 )                { Print( "---> Error: API Copy/Paste Geom  " ); }
    \endcode
    \sa PasteGeomClipboard
    \param [in] geom_id Geom ID
*/)";
    r = se->RegisterGlobalFunction( "void CopyGeomToClipboard(const string & in geom_id)", asFUNCTION( vsp::CopyGeomToClipboard ), asCALL_CDECL, doc_struct );
    assert( r >= 0 );

    doc_struct.comment = R"(
/*!
    Paste Geom from clipboard into the model. The Geom is pasted as a child of the specified parent, but will be placed at top level if no parent or an invalid one is provided.
    \code{.cpp}
    //==== Add Pod Geometries ====//
    string pid1 = AddGeom( "POD", "" );
    string pid2 = AddGeom( "POD", "" );

    CutGeomToClipboard( pid1 );

    PasteGeomClipboard( pid2 ); // Paste Pod 1 as child of Pod 2

    array< string > @geom_ids = FindGeoms();

    if ( geom_ids.size() != 2 )                { Print( "---> Error: API Cut/Paste Geom  " ); }
    \endcode
    \param [in] parent_id Parent Geom ID
    \return Array of pasted Geom IDs
*/)";
    r = se->RegisterGlobalFunction( "array<string>@ PasteGeomClipboard( const string & in parent_id = \"\" )", asMETHOD( ScriptMgrSingleton, PasteGeomClipboard ), asCALL_THISCALL_ASGLOBAL, &ScriptMgr, doc_struct );
    assert( r >= 0 );

    doc_struct.comment = R"(
/*!
    Find and return all Geom IDs in the model
    \code{.cpp}
    //==== Add Pod Geometries ====//
    string pod1 = AddGeom( "POD", "" );
    string pod2 = AddGeom( "POD", "" );

    //==== There Should Be Two Geoms =====//
    array< string > @geom_ids = FindGeoms();

    if ( geom_ids.size() != 2 )                        { Print( "---> Error: API FindGeoms " ); }
    \endcode
    \return Array of all Geom IDs
*/)";
    r = se->RegisterGlobalFunction( "array<string>@ FindGeoms()", asMETHOD( ScriptMgrSingleton, FindGeoms ), asCALL_THISCALL_ASGLOBAL, &ScriptMgr, doc_struct );
    assert( r >= 0 );

    doc_struct.comment = R"(
/*!
    Find and return all Geom IDs with the specified name
    \code{.cpp}
    //==== Add Pod Geometry ====//
    string pid = AddGeom( "POD", "" );

    SetGeomName( pid, "ExamplePodName" );

    array< string > @geom_ids = FindGeomsWithName( "ExamplePodName" );

    if ( geom_ids.size() != 1 )
    {
        Print( "---> Error: API FindGeomsWithName " );
    }
    \endcode
    \sa FindGeom
    \param [in] name Geom name
    \return Array of Geom IDs
*/)";
    r = se->RegisterGlobalFunction( "array<string>@ FindGeomsWithName(const string & in name)", asMETHOD( ScriptMgrSingleton, FindGeomsWithName ), asCALL_THISCALL_ASGLOBAL, &ScriptMgr, doc_struct );
    assert( r >= 0 );

    doc_struct.comment = R"(
/*!
    Find and return the Geom ID with the specified name at given index. Equivalent to FindGeomsWithName( name )[index].
    \code{.cpp}
    //==== Add Pod Geometry ====//
    string pid = AddGeom( "POD", "" );

    SetGeomName( pid, "ExamplePodName" );

    string geom_id = FindGeom( "ExamplePodName", 0 );

    array< string > @geom_ids = FindGeomsWithName( "ExamplePodName" );

    if ( geom_ids[0] != geom_id )
    {
        Print( "---> Error: API FindGeom & FindGeomsWithName" );
    }
    \endcode
    \sa FindGeomsWithName
    \param [in] name Geom name
    \param [in] index 
    \return Geom ID with name at specified index
*/)";
    r = se->RegisterGlobalFunction( "string FindGeom(const string & in name, int index)", asFUNCTION( vsp::FindGeom ), asCALL_CDECL, doc_struct );
    assert( r >= 0 );

    doc_struct.comment = R"(
/*!
    Set the name of the specified Geom
    \code{.cpp}
    //==== Add Pod Geometry ====//
    string pid = AddGeom( "POD", "" );

    SetGeomName( pid, "ExamplePodName" );

    array< string > @geom_ids = FindGeomsWithName( "ExamplePodName" );

    if ( geom_ids.size() != 1 )
    {
        Print( "---> Error: API FindGeomsWithName " );
    }
    \endcode
    \param [in] geom_id Geom ID
    \param [in] name Geom name
*/)";
    r = se->RegisterGlobalFunction( "void SetGeomName( const string & in geom_id, const string & in name )", asFUNCTION( vsp::SetGeomName ), asCALL_CDECL, doc_struct );
    assert( r >= 0 );

    doc_struct.comment = R"(
/*!
    Get the name of a specific Geom
    \code{.cpp}
    //==== Add Pod Geometry ====//
    string pid = AddGeom( "POD", "" );

    SetGeomName( pid, "ExamplePodName" );

    string name_str = "Geom Name: " + GetGeomName( pid );

    Print( name_str );
    \endcode
    \param [in] geom_id Geom ID
    \return Geom name
*/)";
    r = se->RegisterGlobalFunction( "string GetGeomName( const string & in geom_id )", asFUNCTION( vsp::GetGeomName ), asCALL_CDECL, doc_struct );
    assert( r >= 0 );

    doc_struct.comment = R"(
/*!
    Get all Parm IDs associated with this Geom Parm container
    \code{.cpp}
    //==== Add Pod Geometry ====//
    string pid = AddGeom( "POD", "" );

    Print( string( "---> Test Get Parm Arrays" ) );

    array< string > @parm_array = GetGeomParmIDs( pid );

    if ( parm_array.size() < 1 )            { Print( "---> Error: API GetGeomParmIDs " ); }
    \endcode
    \param [in] geom_id Geom ID
    \return Array of Parm IDs
*/)";
    r = se->RegisterGlobalFunction( "array<string>@ GetGeomParmIDs(const string & in geom_id )", asMETHOD( ScriptMgrSingleton, GetGeomParmIDs ), asCALL_THISCALL_ASGLOBAL, &ScriptMgr, doc_struct );
    assert( r >= 0 );

    doc_struct.comment = R"(
/*!
    Get the VSP surface CFD type of the specified Geom
    \code{.cpp}
    //==== Add Wing Geometry ====//
    string wing_id = AddGeom( "WING" );

    if ( GetGeomVSPSurfCfdType( wing_id ) != CFD_NORMAL )
    {
        Print( "---> Error: API GetGeomVSPSurfCfdType " );
    }
    \endcode
    \sa VSP_SURF_CFD_TYPE
    \param [in] geom_id Geom ID
    \param [in] main_surf_ind Main surface index
    \return VSP surface CFD type enum (i.e. CFD_TRANSPARENT)
*/)";
    r = se->RegisterGlobalFunction( "int GetGeomVSPSurfCfdType( const string & in geom_id, int main_surf_ind = 0 )", asFUNCTION( vsp::GetGeomVSPSurfCfdType ), asCALL_CDECL, doc_struct );
    assert( r >= 0 );

    doc_struct.comment = R"(
/*!
    Get the VSP surface type of the specified Geom
    \code{.cpp}
    //==== Add Wing Geometry ====//
    string wing_id = AddGeom( "WING" );

    if ( GetGeomVSPSurfType( wing_id ) != WING_SURF )
    {
        Print( "---> Error: API GetGeomVSPSurfType " );
    }
    \endcode
    \sa VSP_SURF_TYPE
    \param [in] geom_id Geom ID
    \param [in] main_surf_ind Main surface index
    \return VSP surface type enum (i.e. DISK_SURF)
*/)";
    r = se->RegisterGlobalFunction( "int GetGeomVSPSurfType( const string & in geom_id, int main_surf_ind = 0 )", asFUNCTION( vsp::GetGeomVSPSurfType ), asCALL_CDECL, doc_struct );
    assert( r >= 0 );

    doc_struct.comment = R"(
/*!
    Get the type name of specified Geom (i.e. FUSELAGE)
    \code{.cpp}
    //==== Add Wing Geometry ====//
    string wing_id = AddGeom( "WING" );

    Print( "Geom Type Name: ", false );

    Print( GetGeomTypeName( wing_id ) );
    \endcode
    \param [in] geom_id Geom ID
    \return Geom type name
*/)";
    r = se->RegisterGlobalFunction( "string GetGeomTypeName(const string & in geom_id )", asFUNCTION( vsp::GetGeomTypeName ), asCALL_CDECL, doc_struct );
    assert( r >= 0 );

    doc_struct.comment = R"(
/*!
    Get the number of main surfaces for the specified Geom. Multiple main surfaces may exist for CustoGeoms, propellors, etc., but 
    does not include surfaces created due to symmetry.
    \code{.cpp}
    //==== Add Prop Geometry ====//
    string prop_id = AddGeom( "PROP" );

    int num_surf = 0;

    num_surf = GetNumMainSurfs( prop_id ); // Should be the same as the number of blades

    Print( "Number of Propeller Surfaces: ", false );

    Print( num_surf );
    \endcode
    \param [in] geom_id Geom ID
    \return Number of main surfaces
*/)";
    r = se->RegisterGlobalFunction( "int GetNumMainSurfs( const string & in geom_id )", asFUNCTION( vsp::GetNumMainSurfs ), asCALL_CDECL, doc_struct );
    assert( r >= 0 );

    doc_struct.comment = R"(
/*!
    Get the total number of surfaces for the specified Geom. This is equivalent to the number of main surface multiplied 
    by the number of symmetric copies. 
    \code{.cpp}
    //==== Add Wing Geometry ====//
    string wing_id = AddGeom( "WING" );

    int num_surf = 0;

    num_surf = GetTotalNumSurfs( wing_id ); // Wings default with XZ symmetry on -> 2 surfaces

    Print( "Total Number of Wing Surfaces: ", false );

    Print( num_surf );
    \endcode
    \param [in] geom_id Geom ID
    \return Number of main surfaces
*/)";
    r = se->RegisterGlobalFunction( "int GetTotalNumSurfs( const string & in geom_id )", asFUNCTION( vsp::GetTotalNumSurfs ), asCALL_CDECL, doc_struct );
    assert( r >= 0 );

    doc_struct.comment = R"(
/*!
    Get the the maximum coordinate of the bounding box of a Geom with given main surface index. The Geom bounding
    box may be specified in absolute or body reference frame. 
    \code{.cpp}
    //==== Add Pod Geometry ====//
    string pid = AddGeom( "POD" );

    SetParmVal( FindParm( pid, "Y_Rotation", "XForm" ), 45 );
    SetParmVal( FindParm( pid, "Z_Rotation", "XForm" ), 25 );

    Update();
    
    vec3d max_pnt = GetGeomBBoxMax( pid, 0, false );
    \endcode
    \sa GetGeomBBoxMin
    \param [in] geom_id Geom ID
    \param [in] main_surf_ind Main surface index
    \param [in] ref_frame_is_absolute Flag to specify absolute or body reference frame
    \return Maximum coordinate of the bounding box
*/)";
    r = se->RegisterGlobalFunction( "vec3d GetGeomBBoxMax( const string & in geom_id, int main_surf_ind = 0, bool ref_frame_is_absolute = true )", asFUNCTION( vsp::GetGeomBBoxMax ), asCALL_CDECL, doc_struct );
    assert( r >= 0 );

    doc_struct.comment = R"(
/*!
    Get the the minimum coordinate of the bounding box of a Geom with given main surface index. The Geom bounding
    box may be specified in absolute or body reference frame. 
    \code{.cpp}
    //==== Add Pod Geometry ====//
    string pid = AddGeom( "POD" );

    SetParmVal( FindParm( pid, "Y_Rotation", "XForm" ), 45 );
    SetParmVal( FindParm( pid, "Z_Rotation", "XForm" ), 25 );

    Update();
    
    vec3d min_pnt = GetGeomBBoxMin( pid, 0, false );
    \endcode
    \sa GetGeomBBoxMax
    \param [in] geom_id Geom ID
    \param [in] main_surf_ind Main surface index
    \param [in] ref_frame_is_absolute Flag to specify absolute or body reference frame
    \return Minimum coordinate of the bounding box
*/)";
    r = se->RegisterGlobalFunction( "vec3d GetGeomBBoxMin( const string & in geom_id, int main_surf_ind = 0, bool ref_frame_is_absolute = true )", asFUNCTION( vsp::GetGeomBBoxMin ), asCALL_CDECL, doc_struct );
    assert( r >= 0 );

    doc_struct.comment = R"(
/*!
    Get the parent Geom ID for the input child Geom. "NONE" is returned if the Geom has no parent.
    \code{.cpp}
    //==== Add Parent and Child Geometry ====//
    string pod1 = AddGeom( "POD" );

    string pod2 = AddGeom( "POD", pod1 );

    Print( "Parent ID of Pod #2: ", false );

    Print( GetGeomParent( pod2 ) );
    \endcode
    \param [in] geom_id Geom ID
    \return Parent Geom ID
*/)";
    r = se->RegisterGlobalFunction( "string GetGeomParent( const string & in geom_id )", asFUNCTION( vsp::GetGeomParent ), asCALL_CDECL, doc_struct );
    assert( r >= 0 );

    doc_struct.comment = R"(
/*!
    Get the IDs for each child of the input parent Geom.
    \code{.cpp}
    //==== Add Parent and Child Geometry ====//
    string pod1 = AddGeom( "POD" );

    string pod2 = AddGeom( "POD", pod1 );

    string pod3 = AddGeom( "POD", pod2 );

    Print( "Children of Pod #1: " );

    array<string> children = GetGeomChildren( pod1 );

    for ( int i = 0; i < int( children.size() ); i++ )
    {
        Print( "\t", false );
        Print( children[i] );
    }
    \endcode
    \param [in] geom_id Geom ID
    \return Array of child Geom IDs
*/)";
    r = se->RegisterGlobalFunction( "array<string>@ GetGeomChildren( const string & in geom_id )", asMETHOD( ScriptMgrSingleton, GetGeomChildren ), asCALL_THISCALL_ASGLOBAL, &ScriptMgr, doc_struct );
    assert( r >= 0 );

    doc_struct.comment = R"(
/*!
    Set the driver group for a wing section or a XSecCurve. Care has to be taken when setting these driver groups to ensure a valid combination.
    \code{.cpp}
    //==== Add Wing Geometry and Set Parms ====//
    string wing_id = AddGeom( "WING", "" );

    //==== Set Wing Section Controls ====//
    SetDriverGroup( wing_id, 1, AR_WSECT_DRIVER, ROOTC_WSECT_DRIVER, TIPC_WSECT_DRIVER );

    Update();

    //==== Set Parms ====//
    SetParmVal( wing_id, "Root_Chord", "XSec_1", 2 );
    SetParmVal( wing_id, "Tip_Chord", "XSec_1", 1 );

    Update();
    \endcode
    \sa WING_DRIVERS, XSEC_DRIVERS
    \param [in] geom_id Geom ID
    \param [in] section_index Wing section index
    \param [in] driver_0 First driver enum (i.e. SPAN_WSECT_DRIVER)
    \param [in] driver_1 Second driver enum (i.e. ROOTC_WSECT_DRIVER)
    \param [in] driver_2 Third driver enum (i.e. TIPC_WSECT_DRIVER)
    */)";
    r = se->RegisterGlobalFunction( "void SetDriverGroup( const string & in geom_id, int section_index, int driver_0, int driver_1 = -1, int driver_2 = -1)", asFUNCTION( vsp::SetDriverGroup ), asCALL_CDECL, doc_struct );
    assert( r >= 0 );

    //==== SubSurface Functions ====//
    group = "SubSurface";
    doc_struct.group = group.c_str();

    group_description = R"(
    \brief Functions related to Sub-Surfaces are defined in this group. \n\n
    \ref index "Click here to return to the main page" )";
    se->AddGroup( group.c_str(), "Sub-Surface Functions", group_description.c_str() );

    doc_struct.comment = R"(
/*!
    Add a sub-surface to the specified Geom
    \code{.cpp}
    string wid = AddGeom( "WING", "" );                             // Add Wing

    // Note: Parm Group for SubSurfaces in the form: "SS_" + type + "_" + count (initialized at 1)
    string ss_line_id = AddSubSurf( wid, SS_LINE );                      // Add Sub Surface Line

    SetParmVal( wid, "Const_Line_Value", "SubSurface_1", 0.4 );     // Change Location
    \endcode
    \sa SUBSURF_TYPE
    \param [in] geom_id Geom ID
    \param [in] type Sub-surface type enum (i.e. SS_RECTANGLE)
    \param [in] surfindex Main surface index (default: 0)
    \return Sub-surface ID
*/)";
    r = se->RegisterGlobalFunction( "string AddSubSurf( const string & in geom_id, int type, int surfindex = 0 )", asFUNCTION( vsp::AddSubSurf ), asCALL_CDECL, doc_struct );
    assert( r >= 0 );

    doc_struct.comment = R"(
/*!
    Delete the specified sub-surface
    \code{.cpp}
    string wid = AddGeom( "WING", "" );                             // Add Wing

    string ss_line_id = AddSubSurf( wid, SS_LINE );                      // Add Sub Surface Line
    string ss_rec_id = AddSubSurf( wid, SS_RECTANGLE );                        // Add Sub Surface Rectangle

    Print("Delete SS_Line\n");

    DeleteSubSurf( wid, ss_line_id );
    
    int num_ss = GetNumSubSurf( wid );

    string num_str = string("Number of SubSurfaces: ") + formatInt( num_ss, '' ) + string("\n");

    Print( num_str );
    \endcode
    \param [in] geom_id Geom ID
    \param [in] sub_id Sub-surface ID
*/)";
    r = se->RegisterGlobalFunction( "void DeleteSubSurf( const string & in geom_id, const string & in sub_id )", asFUNCTIONPR( vsp::DeleteSubSurf, ( const string &, const string & ), void ), asCALL_CDECL, doc_struct );
    assert( r >= 0 );
    // TODO: Why are there two DeleteSubSurf if Geom ID isn't needed?
    doc_struct.comment = R"(
/*!
    Delete the specified sub-surface
    \code{.cpp}
    string wid = AddGeom( "WING", "" );                             // Add Wing

    string ss_line_id = AddSubSurf( wid, SS_LINE );                      // Add Sub Surface Line
    string ss_rec_id = AddSubSurf( wid, SS_RECTANGLE );                        // Add Sub Surface Rectangle

    Print("Delete SS_Line\n");

    DeleteSubSurf( ss_line_id );
    
    int num_ss = GetNumSubSurf( wid );

    string num_str = string("Number of SubSurfaces: ") + formatInt( num_ss, '' ) + string("\n");

    Print( num_str );
    \endcode
    \param [in] sub_id Sub-surface ID
*/)";
    r = se->RegisterGlobalFunction( "void DeleteSubSurf( const string & in sub_id )", asFUNCTIONPR( vsp::DeleteSubSurf, ( const string & ), void ), asCALL_CDECL, doc_struct );
    assert( r >= 0 );

    doc_struct.comment = R"(
/*!
    Get the ID of the specified sub-surface
    \code{.cpp}
    string wid = AddGeom( "WING", "" ); // Add Wing

    string ss_rec_1 = AddSubSurf( wid, SS_RECTANGLE ); // Add Sub Surface Rectangle #1

    string ss_rec_2 = AddSubSurf( wid, SS_RECTANGLE ); // Add Sub Surface Rectangle #2

    Print( ss_rec_2, false );

    Print( " = ", false );

    Print( GetSubSurf( wid, 1 ) );
    \endcode
    \param [in] geom_id Geom ID
    \param [in] index Sub-surface index
    \return Sub-surface ID
*/)";
    r = se->RegisterGlobalFunction( "string GetSubSurf( const string & in geom_id, int index )", asFUNCTIONPR( vsp::GetSubSurf, ( const string &, int ), string ), asCALL_CDECL, doc_struct );
    assert( r >= 0 );

    doc_struct.comment = R"(
/*!
    Get all sub-surface IDs with specified parent Geom and name
    \code{.cpp}
    string wid = AddGeom( "WING", "" );                             // Add Wing

    string ss_rec_id = AddSubSurf( wid, SS_RECTANGLE );                        // Add Sub Surface Rectangle

    rec_name = GetSubSurfName( ss_rec_id );

    id_vec = GetSubSurf( wid, rec_name );

    string ID_str = string("IDs of subsurfaces named \"") + rec_name + string("\": ") + id_vec[0];

    Print( ID_str );
    \endcode
    \param [in] geom_id Geom ID
    \param [in] name Sub-surface name
    \return Array of sub-surface IDs
*/)";
    r = se->RegisterGlobalFunction( "array<string>@ GetSubSurf( const string & in geom_id, const string & in name )", asMETHOD( ScriptMgrSingleton, GetSubSurf ), asCALL_THISCALL_ASGLOBAL, &ScriptMgr, doc_struct );
    assert( r >= 0 );

    doc_struct.comment = R"(
/*!
    Set the name of the specified sub-surface
    \code{.cpp}
    string wid = AddGeom( "WING", "" );                             // Add Wing

    string ss_rec_id = AddSubSurf( wid, SS_RECTANGLE );                        // Add Sub Surface Rectangle

    string new_name = string("New_SS_Rec_Name");

    SetSubSurfName( wid, ss_rec_id, new_name );
    \endcode
    \param [in] geom_id Geom ID
    \param [in] sub_id Sub-surface ID
    \param [in] name Sub-surface name
*/)";
    r = se->RegisterGlobalFunction( "void SetSubSurfName( const string & in geom_id, const string & in sub_id, const string & in name )", asFUNCTIONPR( vsp::SetSubSurfName, ( const string &, const string &, const string & ), void ), asCALL_CDECL, doc_struct );
    assert( r >= 0 );
    // TODO: Why are there two SetSubSurfName if Geom ID isn't needed?
    doc_struct.comment = R"(
/*!
    Set the name of the specified sub-surface
    \code{.cpp}
    string wid = AddGeom( "WING", "" );                             // Add Wing

    string ss_rec_id = AddSubSurf( wid, SS_RECTANGLE );                        // Add Sub Surface Rectangle

    string new_name = string("New_SS_Rec_Name");

    SetSubSurfName( ss_rec_id, new_name );
    \endcode
    \param [in] sub_id Sub-surface ID
    \param [in] name Sub-surface name
*/)";
    r = se->RegisterGlobalFunction( "void SetSubSurfName( const string & in sub_id, const string & in name )", asFUNCTIONPR( vsp::SetSubSurfName, ( const string &, const string & ), void ), asCALL_CDECL, doc_struct );
    assert( r >= 0 );

    doc_struct.comment = R"(
/*!
    Get the name of the specified sub-surface
    \code{.cpp}
    string wid = AddGeom( "WING", "" );                             // Add Wing

    string ss_rec_id = AddSubSurf( wid, SS_RECTANGLE );                        // Add Sub Surface Rectangle

    string rec_name = GetSubSurfName( wid, ss_rec_id );

    string name_str = string("Current Name of SS_Rectangle: ") + rec_name + string("\n");

    Print( name_str );
    \endcode
    \param [in] geom_id Geom ID
    \param [in] sub_id Sub-surface ID
    \return Sub-surface name
*/)";
    r = se->RegisterGlobalFunction( "string GetSubSurfName( const string & in geom_id, const string & in sub_id )", asFUNCTIONPR( vsp::GetSubSurfName, ( const string &, const string & ), string ), asCALL_CDECL, doc_struct );
    assert( r >= 0 );

    doc_struct.comment = R"(
/*!
    Get the name of the specified sub-surface
    \code{.cpp}
    string wid = AddGeom( "WING", "" );                             // Add Wing

    string ss_rec_id = AddSubSurf( wid, SS_RECTANGLE );                        // Add Sub Surface Rectangle

    string rec_name = GetSubSurfName( ss_rec_id );

    string name_str = string("Current Name of SS_Rectangle: ") + rec_name + string("\n");

    Print( name_str );
    \endcode
    \param [in] sub_id Sub-surface ID
    \return Sub-surface name
*/)";
    r = se->RegisterGlobalFunction( "string GetSubSurfName( const string & in sub_id )", asFUNCTIONPR( vsp::GetSubSurfName, ( const string & ), string ), asCALL_CDECL, doc_struct );
    assert( r >= 0 );
    // TODO: Why are there two GetSubSurfName if Geom ID isn't needed?
    doc_struct.comment = R"(
/*!
    Get the index of the specified sub-surface in its parent Geom's sub-surface vector
    \code{.cpp}
    string wid = AddGeom( "WING", "" );                             // Add Wing

    string ss_line_id = AddSubSurf( wid, SS_LINE );                      // Add Sub Surface Line
    string ss_rec_id = AddSubSurf( wid, SS_RECTANGLE );                        // Add Sub Surface Rectangle

    int ind = GetSubSurfIndex( ss_rec_id );

    string ind_str = string("Index of SS_Rectangle: ") + ind + string("\n");

    Print( ind_str );
    \endcode
    \param [in] sub_id Sub-surface ID
    \return Sub-surface index
*/)";
    r = se->RegisterGlobalFunction( "int GetSubSurfIndex( const string & in sub_id )", asFUNCTION( vsp::GetSubSurfIndex ), asCALL_CDECL, doc_struct );
    assert( r >= 0 );

    doc_struct.comment = R"(
/*!
    Get a vector of all sub-surface IDs for the specified geometry
    \code{.cpp}
    string wid = AddGeom( "WING", "" );                             // Add Wing

    string ss_line_id = AddSubSurf( wid, SS_LINE );                      // Add Sub Surface Line
    string ss_rec_id = AddSubSurf( wid, SS_RECTANGLE );                        // Add Sub Surface Rectangle

    array<string> id_vec = GetSubSurfIDVec( wid );

    string id_type_str = string( "SubSurface IDs and Type Indexes -> ");
    
    for ( uint i = 0; i < uint(id_vec.length()); i++ )
    {
        id_type_str += id_vec[i];

        id_type_str += string(": ");

        id_type_str += GetSubSurfType(id_vec[i]);

        id_type_str += string("\t");
    }
    
    id_type_str += string("\n");

    Print( id_type_str );
    \endcode
    \param [in] geom_id Geom ID
    \return Array of sub-surface IDs
*/)";
    r = se->RegisterGlobalFunction( "array<string>@ GetSubSurfIDVec( const string & in geom_id )", asMETHOD( ScriptMgrSingleton, GetSubSurfIDVec ), asCALL_THISCALL_ASGLOBAL, &ScriptMgr, doc_struct );
    assert( r >= 0 );

    doc_struct.comment = R"(
/*!
    Get a vector of all sub-surface IDs for the entire model
    \return Array of sub-surface IDs
*/)";
    r = se->RegisterGlobalFunction( "array<string>@ GetAllSubSurfIDs()", asMETHOD( ScriptMgrSingleton, GetAllSubSurfIDs ), asCALL_THISCALL_ASGLOBAL, &ScriptMgr, doc_struct );
    assert( r >= 0 );

    doc_struct.comment = R"(
/*!
    Get the number of sub-surfaces for the specified Geom
    \code{.cpp}
    string wid = AddGeom( "WING", "" );                             // Add Wing

    string ss_line_id = AddSubSurf( wid, SS_LINE );                      // Add Sub Surface Line
    string ss_rec_id = AddSubSurf( wid, SS_RECTANGLE );                        // Add Sub Surface Rectangle

    int num_ss = GetNumSubSurf( wid );

    string num_str = string("Number of SubSurfaces: ") + num_ss + string("\n");

    Print( num_str );
    \endcode
    \param [in] geom_id Geom ID
    \return Number of Sub-surfaces
*/)";
    r = se->RegisterGlobalFunction( "int GetNumSubSurf( const string & in geom_id )", asFUNCTION( vsp::GetNumSubSurf ), asCALL_CDECL, doc_struct );
    assert( r >= 0 );

    doc_struct.comment = R"(
/*!
    Get the type for the specified sub-surface (i.e. SS_RECTANGLE)
    \code{.cpp}
    string wid = AddGeom( "WING", "" );                             // Add Wing

    string ss_line_id = AddSubSurf( wid, SS_LINE );                      // Add Sub Surface Line
    string ss_rec_id = AddSubSurf( wid, SS_RECTANGLE );                        // Add Sub Surface Rectangle

    array<string> id_vec = GetSubSurfIDVec( wid );

    string id_type_str = string( "SubSurface IDs and Type Indexes -> ");
    
    for ( uint i = 0; i < uint(id_vec.length()); i++ )
    {
        id_type_str += id_vec[i];

        id_type_str += string(": ");

        id_type_str += GetSubSurfType(id_vec[i]);

        id_type_str += string("\t");
    }
    
    id_type_str += string("\n");

    Print( id_type_str );
    \endcode
    \sa SUBSURF_TYPE
    \param [in] sub_id Sub-surface ID
    \return Sub-surface type enum (i.e. SS_RECTANGLE)
*/)";
    r = se->RegisterGlobalFunction( "int GetSubSurfType( const string & in sub_id )", asFUNCTION( vsp::GetSubSurfType ), asCALL_CDECL, doc_struct );
    assert( r >= 0 );

    doc_struct.comment = R"(
/*!
    Get the vector of Parm IDs for specified sub-surface
    \code{.cpp}
    string wid = AddGeom( "WING", "" );                             // Add Wing

    string ss_line_id = AddSubSurf( wid, SS_LINE );                      // Add Sub Surface Line

    // Get and list all Parm info for SS_Line
    array<string> parm_id_vec = GetSubSurfParmIDs( ss_line_id );
    
    for ( uint i = 0; i < uint(parm_id_vec.length()); i++ )
    {
        string id_name_str = string("\tName: ") + GetParmName( parm_id_vec[i] ) + string(", Group: ") + GetParmDisplayGroupName( parm_id_vec[i] ) +
            string(", ID: ") + parm_id_vec[i] + string("\n");

        Print( id_name_str );
    }
    \endcode
    \param [in] sub_id Sub-surface ID
    \return Array of Parm IDs
*/)";
    r = se->RegisterGlobalFunction( "array<string>@ GetSubSurfParmIDs(const string & in sub_id )", asMETHOD( ScriptMgrSingleton, GetSubSurfParmIDs ), asCALL_THISCALL_ASGLOBAL, &ScriptMgr, doc_struct );
    assert( r >= 0 );

    //==== VSPAERO CS Group Functions ====//
    group = "CSGroup";
    doc_struct.group = group.c_str();

    group_description = R"(
    \brief This group of functions is available for manipulating VSPAERO control surface groups through the API.
    Note, VSPAERO also includes rectangle type sub-surfaces as possible control surfaces. \n\n
    \ref index "Click here to return to the main page" )";
    se->AddGroup( group.c_str(), "VSPAERO Control Surface Group Functions", group_description.c_str() );

    doc_struct.comment = R"(
/*!
    Creates the initial default grouping for the control surfaces.
    The initial grouping collects all surface copies of the sub-surface into a single group.
    For example if a wing is defined with an aileron and that wing is symmetrical about the
    xz plane there will be a surface copy of the master wing surface as well as a copy of
    the sub-surface. The two sub-surfaces may get deflected differently during analysis
    routines and can be identified uniquely by their full name.
    \code{.cpp}
    string wid = AddGeom( "WING", "" );                             // Add Wing

    string aileron_id = AddSubSurf( wid, SS_CONTROL );                      // Add Control Surface Sub-Surface

    //==== Add Vertical tail and set some parameters =====//
    string vert_id = AddGeom( "WING" );

    SetGeomName( vert_id, "Vert" );

    SetParmValUpdate( vert_id, "TotalArea", "WingGeom", 10.0 );
    SetParmValUpdate( vert_id, "X_Rel_Location", "XForm", 8.5 );
    SetParmValUpdate( vert_id, "X_Rel_Rotation", "XForm", 90 );

    string rudder_id = AddSubSurf( vert_id, SS_CONTROL );                      // Add Control Surface Sub-Surface

    AutoGroupVSPAEROControlSurfaces();

    Update();

    Print( "COMPLETE\n" );
    string control_group_settings_container_id = FindContainer( "VSPAEROSettings", 0 );   // auto grouping produces parm containers within VSPAEROSettings

    //==== Set Control Surface Group Deflection Angle ====//
    Print( "\tSetting control surface group deflection angles..." );

    //  setup asymmetric deflection for aileron
    string deflection_gain_id;

    // subsurfaces get added to groups with "CSGQualities_[geom_name]_[control_surf_name]"
    // subsurfaces gain parm name is "Surf[surfndx]_Gain" starting from 0 to NumSymmetricCopies-1

    deflection_gain_id = FindParm( control_group_settings_container_id, "Surf_" + aileron_id + "_0_Gain", "ControlSurfaceGroup_0" );
    deflection_gain_id = FindParm( control_group_settings_container_id, "Surf_" + aileron_id + "_1_Gain", "ControlSurfaceGroup_0" );

    //  deflect aileron
    string deflection_angle_id = FindParm( control_group_settings_container_id, "DeflectionAngle", "ControlSurfaceGroup_0" );
    \endcode
    \sa CreateVSPAEROControlSurfaceGroup
*/)";
    r = se->RegisterGlobalFunction("void AutoGroupVSPAEROControlSurfaces()", asFUNCTION(vsp::AutoGroupVSPAEROControlSurfaces), asCALL_CDECL, doc_struct );
    assert(r >= 0);

    doc_struct.comment = R"(
/*!
    Get the total number of control surface groups
    \code{.cpp}
    string wid = AddGeom( "WING", "" );                             // Add Wing

    string aileron_id = AddSubSurf( wid, SS_CONTROL );                      // Add Control Surface Sub-Surface

    //==== Add Horizontal tail and set some parameters =====//
    string horiz_id = AddGeom( "WING", "" );

    SetGeomName( horiz_id, "Vert" );

    SetParmValUpdate( horiz_id, "TotalArea", "WingGeom", 10.0 );
    SetParmValUpdate( horiz_id, "X_Rel_Location", "XForm", 8.5 );

    string elevator_id = AddSubSurf( horiz_id, SS_CONTROL );                      // Add Control Surface Sub-Surface

    AutoGroupVSPAEROControlSurfaces();

    int num_group = GetNumControlSurfaceGroups();

    if ( num_group != 2 ) { Print( "Error: GetNumControlSurfaceGroups" ); }
    \endcode
    \return Number of control surface groups
*/)";
    r = se->RegisterGlobalFunction( "int GetNumControlSurfaceGroups()", asFUNCTION( vsp::GetNumControlSurfaceGroups ), asCALL_CDECL, doc_struct );
    assert( r >= 0 );

    doc_struct.comment = R"(
/*!
    Add a new VSPAERO control surface group using the default naming convention. The control surface group will not contain any
    control surfaces until they are added. 
    \code{.cpp}
    string wid = AddGeom( "WING", "" );                             // Add Wing

    string aileron_id = AddSubSurf( wid, SS_CONTROL );                      // Add Control Surface Sub-Surface

    int group_index = CreateVSPAEROControlSurfaceGroup(); // Empty control surface group

    int num_group = GetNumControlSurfaceGroups();

    if ( num_group != 1 ) { Print( "Error: CreateVSPAEROControlSurfaceGroup" ); }
    \endcode
    \sa AddSelectedToCSGroup
    \return Index of the new VSPAERO control surface group
*/)";
    r = se->RegisterGlobalFunction("int CreateVSPAEROControlSurfaceGroup()", asFUNCTION(vsp::CreateVSPAEROControlSurfaceGroup), asCALL_CDECL, doc_struct );
    assert(r >= 0);

    doc_struct.comment = R"(
/*!
    Add all available control surfaces to the control surface group at the specified index
    \code{.cpp}
    string wid = AddGeom( "WING", "" );                             // Add Wing

    string aileron_id = AddSubSurf( wid, SS_CONTROL );                      // Add Control Surface Sub-Surface

    int group_index = CreateVSPAEROControlSurfaceGroup(); // Empty control surface group

    AddAllToVSPAEROControlSurfaceGroup( group_index );
    \endcode
    \param [in] CSGroupIndex Index of the control surface group
*/)";
    r = se->RegisterGlobalFunction("void AddAllToVSPAEROControlSurfaceGroup( int CSGroupIndex )", asFUNCTION(vsp::AddAllToVSPAEROControlSurfaceGroup ), asCALL_CDECL, doc_struct );
    assert(r >= 0);

    doc_struct.comment = R"(
/*!
    Remove all used control surfaces from the control surface group at the specified index
    \code{.cpp}
    string wid = AddGeom( "WING", "" );                             // Add Wing

    string aileron_id = AddSubSurf( wid, SS_CONTROL );                      // Add Control Surface Sub-Surface

    int group_index = CreateVSPAEROControlSurfaceGroup(); // Empty control surface group

    AddAllToVSPAEROControlSurfaceGroup( group_index );

    RemoveAllFromVSPAEROControlSurfaceGroup( group_index ); // Empty control surface group
    \endcode
    \param [in] CSGroupIndex Index of the control surface group
*/)";
    r = se->RegisterGlobalFunction("void RemoveAllFromVSPAEROControlSurfaceGroup( int CSGroupIndex )", asFUNCTION(vsp::RemoveAllFromVSPAEROControlSurfaceGroup ), asCALL_CDECL, doc_struct );
    assert(r >= 0);

    doc_struct.comment = R"(
/*!
    Get the names of each active (used) control surface in the control surface group at the specified index
    \code{.cpp}
    string wid = AddGeom( "WING", "" );                             // Add Wing

    string aileron_id = AddSubSurf( wid, SS_CONTROL );                      // Add Control Surface Sub-Surface

    int group_index = CreateVSPAEROControlSurfaceGroup(); // Empty control surface group

    AddAllToVSPAEROControlSurfaceGroup( group_index );

    array<string> @cs_name_vec = GetActiveCSNameVec( group_index );

    Print( "Active CS in Group Index #", false );
    Print( group_index );

    for ( int i = 0; i < int( cs_name_vec.size() ); i++ )
    {
        Print( cs_name_vec[i] );
    }
    \endcode
    \param [in] CSGroupIndex Index of the control surface group
    \return Array of active control surface names
*/)";
    r = se->RegisterGlobalFunction( "array<string>@ GetActiveCSNameVec( int CSGroupIndex )", asMETHOD( ScriptMgrSingleton, GetActiveCSNameVec ), asCALL_THISCALL_ASGLOBAL, &ScriptMgr, doc_struct );
    assert( r >= 0 );

    doc_struct.comment = R"(
/*!
    Get the names of all control surfaces. Some may be active (used) while others may be available.
    \code{.cpp}
    string wid = AddGeom( "WING", "" );                             // Add Wing

    string aileron_id = AddSubSurf( wid, SS_CONTROL );                      // Add Control Surface Sub-Surface

    int group_index = CreateVSPAEROControlSurfaceGroup(); // Empty control surface group

    array<string> @cs_name_vec = GetCompleteCSNameVec();

    Print( "All Control Surfaces: ", false );

    for ( int i = 0; i < int( cs_name_vec.size() ); i++ )
    {
        Print( cs_name_vec[i] );
    }
    \endcode
    \return Array of all control surface names
*/)";
    r = se->RegisterGlobalFunction( "array<string>@ GetCompleteCSNameVec( )", asMETHOD( ScriptMgrSingleton, GetCompleteCSNameVec ), asCALL_THISCALL_ASGLOBAL, &ScriptMgr, doc_struct );
    assert( r >= 0 );

    doc_struct.comment = R"(
/*!
    Get the names of each available (not used) control surface in the control surface group at the specified index
    \code{.cpp}
    string wid = AddGeom( "WING", "" ); // Add Wing

    string aileron_id = AddSubSurf( wid, SS_CONTROL ); // Add Control Surface Sub-Surface

    int group_index = CreateVSPAEROControlSurfaceGroup(); // Empty control surface group

    array<string> @cs_name_vec = GetAvailableCSNameVec( group_index );

    array < int > cs_ind_vec(1);
    cs_ind_vec[0] = 1;

    AddSelectedToCSGroup( cs_ind_vec, group_index ); // Add the first available control surface to the group
    \endcode
    \param [in] CSGroupIndex Index of the control surface group
    \return Array of active control surface names
*/)";
    r = se->RegisterGlobalFunction( "array<string>@ GetAvailableCSNameVec( int CSGroupIndex )", asMETHOD( ScriptMgrSingleton, GetAvailableCSNameVec ), asCALL_THISCALL_ASGLOBAL, &ScriptMgr, doc_struct );
    assert( r >= 0 );

    doc_struct.comment = R"(
/*!
    Set the name for the control surface group at the specified index
    \code{.cpp}
    string wid = AddGeom( "WING", "" ); // Add Wing

    string aileron_id = AddSubSurf( wid, SS_CONTROL ); // Add Control Surface Sub-Surface

    int group_index = CreateVSPAEROControlSurfaceGroup(); // Empty control surface group

    SetVSPAEROControlGroupName( "Example_CS_Group", group_index );

    Print( "CS Group name: ", false );

    Print( GetVSPAEROControlGroupName( group_index ) );
    \endcode
    \param [in] name Name to set for the control surface group
    \param [in] CSGroupIndex Index of the control surface group
*/)";
    r = se->RegisterGlobalFunction("void SetVSPAEROControlGroupName( const string & in name, int CSGroupIndex )", asFUNCTION(vsp::SetVSPAEROControlGroupName), asCALL_CDECL, doc_struct );
    assert(r >= 0);

    doc_struct.comment = R"(
/*!
    Get the name of the control surface group at the specified index
    \code{.cpp}
    string wid = AddGeom( "WING", "" ); // Add Wing

    string aileron_id = AddSubSurf( wid, SS_CONTROL ); // Add Control Surface Sub-Surface

    int group_index = CreateVSPAEROControlSurfaceGroup(); // Empty control surface group

    SetVSPAEROControlGroupName( "Example_CS_Group", group_index );

    Print( "CS Group name: ", false );

    Print( GetVSPAEROControlGroupName( group_index ) );
    \endcode
    \param [in] CSGroupIndex Index of the control surface group
*/)";
    r = se->RegisterGlobalFunction( "string GetVSPAEROControlGroupName( int CSGroupIndex )", asFUNCTION( vsp::GetVSPAEROControlGroupName ), asCALL_CDECL, doc_struct );
    assert( r >= 0 );

    doc_struct.comment = R"(
/*!
    Add each control surfaces in the array of control surface indexes to the control surface group at the specified index. 

    \warning The indexes in input "selected" must be matched with available control surfaces identified by GetAvailableCSNameVec. 
    The "selected" input uses one- based indexing to associate available control surfaces.

    \code{.cpp}
    string wid = AddGeom( "WING", "" ); // Add Wing

    string aileron_id = AddSubSurf( wid, SS_CONTROL ); // Add Control Surface Sub-Surface

    int group_index = CreateVSPAEROControlSurfaceGroup(); // Empty control surface group

    array < string > cs_name_vec = GetAvailableCSNameVec( group_index );

    array < int > cs_ind_vec( cs_name_vec.size() );

    for ( int i = 0; i < int( cs_name_vec.size() ); i++ )
    {
        cs_ind_vec[i] = i + 1;
    }

    AddSelectedToCSGroup( cs_ind_vec, group_index ); // Add all available control surfaces to the group
    \endcode
    \sa GetAvailableCSNameVec
    \param [in] selected Array of control surface indexes to add to the group. Note, the integer values are one based.
    \param [in] CSGroupIndex Index of the control surface group
*/)";
    r = se->RegisterGlobalFunction( "void AddSelectedToCSGroup( array<int>@ selected, int CSGroupIndex )", asMETHOD( ScriptMgrSingleton, AddSelectedToCSGroup ), asCALL_THISCALL_ASGLOBAL, &ScriptMgr, doc_struct );
    assert( r >= 0 );

    doc_struct.comment = R"(
/*!
    Remove each control surfaces in the array of control surface indexes from the control surface group at the specified index. 

    \warning The indexes in input "selected" must be matched with active control surfaces identified by GetActiveCSNameVec. The 
    "selected" input uses one-based indexing to associate available control surfaces. 

    \code{.cpp}
    string wid = AddGeom( "WING", "" ); // Add Wing

    string aileron_id = AddSubSurf( wid, SS_CONTROL ); // Add Control Surface Sub-Surface

    int group_index = CreateVSPAEROControlSurfaceGroup(); // Empty control surface group

    array < string > cs_name_vec = GetAvailableCSNameVec( group_index );

    array < int > cs_ind_vec( cs_name_vec.size() );

    for ( int i = 0; i < int( cs_name_vec.size() ); i++ )
    {
        cs_ind_vec[i] = i + 1;
    }

    AddSelectedToCSGroup( cs_ind_vec, group_index ); // Add the available control surfaces to the group

    array < int > remove_cs_ind_vec( 1 );
    remove_cs_ind_vec[0] = 1;

    RemoveSelectedFromCSGroup( remove_cs_ind_vec, group_index ); // Remove the first control surface
    \endcode
    \sa GetActiveCSNameVec
    \param [in] selected Array of control surface indexes to remove from the group. Note, the integer values are one based.
    \param [in] CSGroupIndex Index of the control surface group
*/)";
    r = se->RegisterGlobalFunction( "void RemoveSelectedFromCSGroup( array<int>@ selected, int CSGroupIndex )", asMETHOD( ScriptMgrSingleton, RemoveSelectedFromCSGroup ), asCALL_THISCALL_ASGLOBAL, &ScriptMgr, doc_struct );
    assert( r >= 0 ); // FIXME: RemoveSelectedFromCSGroup not working

    //==== VSPAERO Functions ====//
    group = "VSPAERO";
    doc_struct.group = group.c_str();

    group_description = R"(
    \brief The following group of functions are specific to VSPAERO. However, their relevance has been 
    mostly replaced by Analysis Manager capabilities. \n\n
    \ref index "Click here to return to the main page" )";
    se->AddGroup( group.c_str(), "VSPAERO Functions", group_description.c_str() );

    doc_struct.comment = R"(
/*!
    Get ID of the current VSPAERO reference Geom
    \return Reference Geom ID
*/)";
    r = se->RegisterGlobalFunction( "string GetVSPAERORefWingID()", asFUNCTION( vsp::GetVSPAERORefWingID ), asCALL_CDECL, doc_struct );
    assert( r >= 0 );

    doc_struct.comment = R"(
/*!
    Set the current VSPAERO reference Geom ID
    \code{.cpp}
    //==== Add Wing Geom and set some parameters =====//
    string wing_id = AddGeom( "WING" );

    SetGeomName( wing_id, "MainWing" );

    //==== Add Vertical tail and set some parameters =====//
    string vert_id = AddGeom( "WING" );

    SetGeomName( vert_id, "Vert" );

    SetParmValUpdate( vert_id, "TotalArea", "WingGeom", 10.0 );
    SetParmValUpdate( vert_id, "X_Rel_Location", "XForm", 8.5 );
    SetParmValUpdate( vert_id, "X_Rel_Rotation", "XForm", 90 );

    //==== Set VSPAERO Reference lengths & areas ====//
    SetVSPAERORefWingID( wing_id ); // Set as reference wing for VSPAERO

    Print( "VSPAERO Reference Wing ID: ", false );

    Print( GetVSPAERORefWingID() );
    \endcode
    \param [in] geom_id Reference Geom ID
*/)";
    r = se->RegisterGlobalFunction( "string SetVSPAERORefWingID( const string & in geom_id )", asFUNCTION( vsp::SetVSPAERORefWingID ), asCALL_CDECL, doc_struct );
    assert( r >= 0 );

    //==== VSPAERO Disk and Prop Functions ====//
    group = "VSPAERODiskAndProp";
    doc_struct.group = group.c_str();

    group_description = R"(
    \brief The following group of functions provide API capability for setting up actuator disks (Disk tab
    of VSPAERO GUI) and propellers (Propeller tab of VSPAERO GUI) for VSPAERO analysis. If a propeller 
    geometry is used to model the actuator disk, the "PropMode" must be set to PROP_DISK or PROP_BOTH. 
    Alternatively, the "PropMode" but be set to PROP_BLADE or PROP_BOTH for unsteady analysis.
    must be set to PROP_DISK or PROP_BOTH.\n\n
    \ref index "Click here to return to the main page" )";
    se->AddGroup( group.c_str(), "VSPAERO Actuator Disk and Propeller Functions", group_description.c_str() );

    doc_struct.comment = R"(
/*!
    Get the ID of a VSPAERO actuator disk at the specified index. An empty string is returned if
    the index is out of range.
    \code{.cpp}
    // Add a propeller
    string prop_id = AddGeom( "PROP", "" );
    SetParmVal( prop_id, "PropMode", "Design", PROP_DISK );
    SetParmVal( prop_id, "Diameter", "Design", 6.0 );

    Update();

    // Setup the actuator disk VSPAERO parms
    string disk_id = FindActuatorDisk( 0 );

    SetParmVal( FindParm( disk_id, "RotorRPM", "Rotor" ), 1234.0 );
    SetParmVal( FindParm( disk_id, "RotorCT", "Rotor" ), 0.35 );
    SetParmVal( FindParm( disk_id, "RotorCP", "Rotor" ), 0.55 );
    SetParmVal( FindParm( disk_id, "RotorHubDiameter", "Rotor" ), 1.0 );
    \endcode
    \sa PROP_MODE
    \param [in] disk_index Actuator disk index for the current VSPAERO set
    \return Actuator disk ID
*/)";
    r = se->RegisterGlobalFunction( "string FindActuatorDisk( int disk_index )", asFUNCTION( vsp::FindActuatorDisk ), asCALL_CDECL, doc_struct );
    assert( r >= 0 );

    doc_struct.comment = R"(
/*!
    Get the number of actuator disks in the current VSPAERO set. This is equivalent to the number of disk surfaces in the VSPAERO set.
    \code{.cpp}
    // Set VSPAERO set index to SET_ALL
    SetParmVal( FindParm( FindContainer( "VSPAEROSettings", 0 ), "GeomSet", "VSPAERO" ), SET_ALL );

    // Add a propeller
    string prop_id = AddGeom( "PROP", "" );
    SetParmValUpdate( prop_id, "PropMode", "Design", PROP_BLADES );

    int num_disk = GetNumActuatorDisks(); // Should be 0

    SetParmValUpdate( prop_id, "PropMode", "Design", PROP_DISK );

    num_disk = GetNumActuatorDisks(); // Should be 1
    \endcode
    \sa PROP_MODE
    \return Number of actuator disks in the current VSPAERO set
*/)";
    r = se->RegisterGlobalFunction( "int GetNumActuatorDisks()", asFUNCTION( vsp::GetNumActuatorDisks ), asCALL_CDECL, doc_struct );
    assert( r >= 0 );

    doc_struct.comment = R"(
/*!
    Get the ID of the VSPAERO unsteady group at the specified index. An empty string is returned if
    the index is out of range.
    \code{.cpp}
    string wing_id = AddGeom( "WING" );
    string pod_id = AddGeom( "POD" );

    // Create an actuator disk
    string prop_id = AddGeom( "PROP", "" );
    SetParmVal( prop_id, "PropMode", "Design", PROP_BLADES );

    Update();

    // Setup the unsteady group VSPAERO parms
    string disk_id = FindUnsteadyGroup( 1 ); // fixed components are in group 0 (wing & pod)

    SetParmVal( FindParm( disk_id, "RPM", "UnsteadyGroup" ), 1234.0 );
    \endcode
    \sa PROP_MODE
    \param [in] group_index Unsteady group index for the current VSPAERO set
    \return Unsteady group ID
*/)";
    r = se->RegisterGlobalFunction( "string FindUnsteadyGroup( int group_index )", asFUNCTION( vsp::FindUnsteadyGroup ), asCALL_CDECL, doc_struct );
    assert( r >= 0 );

    doc_struct.comment = R"(
/*!
    Get the name of the unsteady group at the specified index.
    \code{.cpp}
    // Add a pod and wing
    string pod_id = AddGeom( "POD", "" );
    string wing_id = AddGeom( "WING", pod_id );

    SetParmVal( wing_id, "X_Rel_Location", "XForm", 2.5 );
    Update();

    Print( GetUnsteadyGroupName( 0 ) );
    \endcode
    \sa SetUnsteadyGroupName
    \param [in] group_index Unsteady group index for the current VSPAERO set
    \return Unsteady group name
*/)";
    r = se->RegisterGlobalFunction( "string GetUnsteadyGroupName( int group_index )", asFUNCTION( vsp::GetUnsteadyGroupName ), asCALL_CDECL, doc_struct );
    assert( r >= 0 );

    doc_struct.comment = R"(
/*!
    Get an array of IDs for all components in the unsteady group at the specified index.
    \code{.cpp}
    // Add a pod and wing
    string pod_id = AddGeom( "POD", "" );
    string wing_id = AddGeom( "WING", pod_id ); // Default with symmetry on -> 2 surfaces

    SetParmVal( wing_id, "X_Rel_Location", "XForm", 2.5 );
    Update();

    array < string > comp_ids = GetUnsteadyGroupCompIDs( 0 );

    if ( comp_ids.size() != 3 )
    {
        Print( "ERROR: GetUnsteadyGroupCompIDs" );
    }
    \endcode
    \sa GetUnsteadyGroupSurfIndexes
    \param [in] group_index Unsteady group index for the current VSPAERO set
    \return Array of component IDs
*/)";
    r = se->RegisterGlobalFunction( "array<string>@ GetUnsteadyGroupCompIDs( int group_index )", asMETHOD( ScriptMgrSingleton, GetUnsteadyGroupCompIDs ), asCALL_THISCALL_ASGLOBAL, &ScriptMgr, doc_struct );
    assert( r >= 0 );

    doc_struct.comment = R"(
/*!
    Get an array of surface indexes for all components in the unsteady group at the specified index.
    \code{.cpp}
    // Add a pod and wing
    string pod_id = AddGeom( "POD", "" );
    string wing_id = AddGeom( "WING", pod_id ); // Default with symmetry on -> 2 surfaces

    SetParmVal( wing_id, "X_Rel_Location", "XForm", 2.5 );
    Update();

    array < int > surf_indexes = GetUnsteadyGroupSurfIndexes( 0 );

    if ( surf_indexes.size() != 3 )
    {
        Print( "ERROR: GetUnsteadyGroupSurfIndexes" );
    }
    \endcode
    \sa GetUnsteadyGroupCompIDs
    \param [in] group_index Unsteady group index for the current VSPAERO set
    \return Array of surface indexes
*/)";
    r = se->RegisterGlobalFunction( "array<int>@ GetUnsteadyGroupSurfIndexes( int group_index )", asMETHOD( ScriptMgrSingleton, GetUnsteadyGroupSurfIndexes ), asCALL_THISCALL_ASGLOBAL, &ScriptMgr, doc_struct );
    assert( r >= 0 );

    doc_struct.comment = R"(
/*!
    Get the number of unsteady groups in the current VSPAERO set. Each propeller is placed in its own unsteady group. All symmetric copies
    of propellers are also placed in an unsteady group. All other component types are placed in a single fixed component unsteady group.
    \code{.cpp}
    // Set VSPAERO set index to SET_ALL
    SetParmVal( FindParm( FindContainer( "VSPAEROSettings", 0 ), "GeomSet", "VSPAERO" ), SET_ALL );

    // Add a propeller
    string prop_id = AddGeom( "PROP" );
    SetParmValUpdate( prop_id, "PropMode", "Design", PROP_DISK );

    int num_group = GetNumUnsteadyGroups(); // Should be 0

    SetParmValUpdate( prop_id, "PropMode", "Design", PROP_BLADES );

    num_group = GetNumUnsteadyGroups(); // Should be 1

    string wing_id = AddGeom( "WING" );

    num_group = GetNumUnsteadyGroups(); // Should be 2 (includes fixed component group)
    \endcode
    \sa PROP_MODE, GetNumUnsteadyRotorGroups
    \return Number of unsteady groups in the current VSPAERO set
*/)";
    r = se->RegisterGlobalFunction( "int GetNumUnsteadyGroups()", asFUNCTION( vsp::GetNumUnsteadyGroups ), asCALL_CDECL, doc_struct );
    assert( r >= 0 );

    doc_struct.comment = R"(
/*!
    Get the number of unsteady rotor groups in the current VSPAERO set. This is equivalent to the total number of propeller Geoms, 
    including each symmetric copy, in the current VSPAERO set. While all fixed components (wings, fuseleage, etc.) are placed in 
    their own unsteady group, this function does not consider them. 
    \code{.cpp}
    // Set VSPAERO set index to SET_ALL
    SetParmVal( FindParm( FindContainer( "VSPAEROSettings", 0 ), "GeomSet", "VSPAERO" ), SET_ALL );

    // Add a propeller
    string prop_id = AddGeom( "PROP" );
    SetParmValUpdate( prop_id, "PropMode", "Design", PROP_DISK );

    int num_group = GetNumUnsteadyRotorGroups(); // Should be 0

    SetParmValUpdate( prop_id, "PropMode", "Design", PROP_BLADES );

    num_group = GetNumUnsteadyRotorGroups(); // Should be 1

    string wing_id = AddGeom( "WING" );

    num_group = GetNumUnsteadyRotorGroups(); // Should be 1 still (fixed group not included)
    \endcode
    \sa PROP_MODE, GetNumUnsteadyGroups
    \return Number of unsteady rotor groups in the current VSPAERO set
*/)";
    r = se->RegisterGlobalFunction( "int GetNumUnsteadyRotorGroups()", asFUNCTION( vsp::GetNumUnsteadyRotorGroups ), asCALL_CDECL, doc_struct );
    assert( r >= 0 );

    //==== XSecSurf Functions ====//
    group = "XSecSurf";
    doc_struct.group = group.c_str();

    group_description = R"(
    \brief This group of API functions provides capabilities related to the XSecSurf class in OpenVSP. \n\n
    \ref index "Click here to return to the main page" )";
    se->AddGroup( group.c_str(), "XSecSurf Functions", group_description.c_str() );

    doc_struct.comment = R"(
/*!
    Get the number of XSecSurfs for the specified Geom
    \code{.cpp}
    //==== Add Fuselage Geometry ====//
    string fuseid = AddGeom( "FUSELAGE", "" );

    int num_xsec_surfs = GetNumXSecSurfs( fuseid );

    if ( num_xsec_surfs != 1 )                { Print( "---> Error: API GetNumXSecSurfs  " ); }
    \endcode
    \param [in] geom_id Geom ID
    \return Number of XSecSurfs
*/)";
    r = se->RegisterGlobalFunction( "int GetNumXSecSurfs( const string & in geom_id )", asFUNCTION( vsp::GetNumXSecSurfs ), asCALL_CDECL, doc_struct );
    assert( r >= 0 );

    doc_struct.comment = R"(
/*!
    Get the XSecSurf ID for a particular Geom and XSecSurf index
    \code{.cpp}
    // Add Stack
    string sid = AddGeom( "STACK", "" );

    // Get First (and Only) XSec Surf
    string xsec_surf = GetXSecSurf( sid, 0 );
    \endcode
    \param [in] geom_id Geom ID
    \param [in] index XSecSurf index
    \return XSecSurf ID
*/)";
    r = se->RegisterGlobalFunction( "string GetXSecSurf( const string & in geom_id, int index )", asFUNCTION( vsp::GetXSecSurf ), asCALL_CDECL, doc_struct );
    assert( r >= 0 );

    doc_struct.comment = R"(
/*!
    Get number of XSecs in an XSecSurf
    \code{.cpp}
    // Add Stack
    string sid = AddGeom( "STACK", "" );

    // Get First (and Only) XSec Surf
    string xsec_surf = GetXSecSurf( sid, 0 );

    // Flatten ends
    int num_xsecs = GetNumXSec( xsec_surf );

    for ( int i = 0 ; i < num_xsecs ; i++ )
    {
        string xsec = GetXSec( xsec_surf, i );

        SetXSecTanAngles( xsec, XSEC_BOTH_SIDES, 0 );       // Set Tangent Angles At Cross Section

        SetXSecTanStrengths( xsec, XSEC_BOTH_SIDES, 0.0 );  // Set Tangent Strengths At Cross Section
    }
    \endcode
    \param [in] xsec_surf_id XSecSurf ID
    \return Number of XSecs
*/)";
    r = se->RegisterGlobalFunction( "int GetNumXSec( const string & in xsec_surf_id )", asFUNCTION( vsp::GetNumXSec ), asCALL_CDECL, doc_struct );
    assert( r >= 0 );

    doc_struct.comment = R"(
/*!
    Get Xsec ID for a particular XSecSurf at given index
    \code{.cpp}
    // Add Stack
    string sid = AddGeom( "STACK", "" );

    // Get First (and Only) XSec Surf
    string xsec_surf = GetXSecSurf( sid, 0 );

    // Identify XSec 1
    string xsec_1 = GetXSec( xsec_surf, 1 );
    \endcode
    \param [in] xsec_surf_id XSecSurf ID
    \param [in] xsec_index Xsec index
    \return Xsec ID
*/)";
    r = se->RegisterGlobalFunction( "string GetXSec( const string & in xsec_surf_id, int xsec_index )", asFUNCTION( vsp::GetXSec ), asCALL_CDECL, doc_struct );
    assert( r >= 0 );

    doc_struct.comment = R"(
/*!
    Change the shape of a particular XSec, identified by an XSecSurf ID and XSec index
    \code{.cpp}
    // Add Stack
    string sid = AddGeom( "STACK", "" );

    // Get First (and Only) XSec Surf
    string xsec_surf = GetXSecSurf( sid, 0 );

    // Set XSec 1 & 2 to Edit Curve type
    ChangeXSecShape( xsec_surf, 1, XS_EDIT_CURVE );
    ChangeXSecShape( xsec_surf, 2, XS_EDIT_CURVE );

    string xsec_2 = GetXSec( xsec_surf, 2 );
    
    if ( GetXSecShape( xsec_2 ) != XS_EDIT_CURVE )
    {
        Print( "Error: ChangeXSecShape" );
    }
    \endcode
    \sa XSEC_CRV_TYPE
    \param [in] xsec_surf_id XSecSurf ID
    \param [in] xsec_index Xsec index
    \param [in] type Xsec type enum (i.e. XS_ELLIPSE)
*/)";
    r = se->RegisterGlobalFunction( "void ChangeXSecShape( const string & in xsec_surf_id, int xsec_index, int type )", asFUNCTION( vsp::ChangeXSecShape ), asCALL_CDECL, doc_struct );
    assert( r >= 0 );

    doc_struct.comment = R"(
/*!
    Set the global surface transform matrix for given XSecSurf
    \param [in] xsec_surf_id XSecSurf ID
    \param [in] mat Transformation matrix
*/)";
    r = se->RegisterGlobalFunction( "void SetXSecSurfGlobalXForm( const string & in xsec_surf_id, const Matrix4d & in mat )", asFUNCTION( vsp::SetXSecSurfGlobalXForm ), asCALL_CDECL, doc_struct );
    assert( r >= 0 ); // TODO: Add Example

    doc_struct.comment = R"(
/*!
    Get the global surface transform matrix for given XSecSurf
    \param [in] xsec_surf_id XSecSurf ID
    \return Transformation matrix
*/)";
    r = se->RegisterGlobalFunction( "Matrix4d GetXSecSurfGlobalXForm( const string & in xsec_surf_id )", asFUNCTION( vsp::GetXSecSurfGlobalXForm ), asCALL_CDECL, doc_struct );
    assert( r >= 0 ); // TODO: Add Example

    //==== XSec Functions ====//
    group = "XSec";
    doc_struct.group = group.c_str();

    group_description = R"(
    \brief This group of functions provides API control of cross-sections (XSecs). Airfoils are a type of 
    XSec included in this group as well. API functions for Body of Revolution XSecs are included in the 
    Specialized Geometry group. \n\n
    \ref index "Click here to return to the main page" )";
    se->AddGroup( group.c_str(), "XSec and Airfoil Functions", group_description.c_str() );

    doc_struct.comment = R"(
/*!
    Cut a cross-section from the specified geometry and maintain it in memory
    \code{.cpp}
    string fid = AddGeom( "FUSELAGE", "" );             // Add Fuselage

    //==== Insert, Cut, Paste Example ====//
    InsertXSec( fid, 1, XS_ROUNDED_RECTANGLE );         // Insert A Cross-Section

    CopyXSec( fid, 2 );                                 // Copy Just Created XSec To Clipboard

    PasteXSec( fid, 1 );                                // Paste Clipboard

    CutXSec( fid, 2 );                                  // Cut Created XSec
    \endcode
    \sa PasteXSec
    \param [in] geom_id Geom ID
    \param [in] index XSec index
*/)";
    r = se->RegisterGlobalFunction( "void CutXSec( const string & in geom_id, int index )", asFUNCTION( vsp::CutXSec ), asCALL_CDECL, doc_struct );
    assert( r >= 0 );

    doc_struct.comment = R"(
/*!
    Copy a cross-section from the specified geometry and maintain it in memory
    \code{.cpp}
    // Add Stack
    string sid = AddGeom( "STACK", "" );

    // Copy XSec To Clipboard
    CopyXSec( sid, 1 );

    // Paste To XSec 3
    PasteXSec( sid, 3 );
    \endcode
    \sa PasteXSec
    \param [in] geom_id Geom ID
    \param [in] index XSec index
*/)";
    r = se->RegisterGlobalFunction( "void CopyXSec( const string & in geom_id, int index )", asFUNCTION( vsp::CopyXSec ), asCALL_CDECL, doc_struct );
    assert( r >= 0 );

    doc_struct.comment = R"(
/*!
    Paste the cross-section currently held in memory to the specified geometry
    \code{.cpp}
    // Add Stack
    string sid = AddGeom( "STACK", "" );

    // Copy XSec To Clipboard
    CopyXSec( sid, 1 );

    // Paste To XSec 3
    PasteXSec( sid, 3 );
    \endcode
    \sa CutXSec, CopyXSec
    \param [in] geom_id Geom ID
    \param [in] index XSec index
*/)";
    r = se->RegisterGlobalFunction( "void PasteXSec( const string & in geom_id, int index )", asFUNCTION( vsp::PasteXSec ), asCALL_CDECL, doc_struct );
    assert( r >= 0 );

    doc_struct.comment = R"(
/*!
    Insert a cross-section of particular type to the specified geometry after the given index
    \code{.cpp}
    string wing_id = AddGeom( "WING" );

    //===== Add XSec ====//
    InsertXSec( wing_id, 1, XS_SIX_SERIES );
    \endcode
    \sa XSEC_CRV_TYPE
    \param [in] geom_id Geom ID
    \param [in] index XSec index
    \param [in] type XSec type enum (i.e. XS_GENERAL_FUSE)
*/)";
    r = se->RegisterGlobalFunction( "void InsertXSec( const string & in geom_id, int index, int type )", asFUNCTION( vsp::InsertXSec ), asCALL_CDECL, doc_struct );
    assert( r >= 0 );

    doc_struct.comment = R"(
/*!
    Get the shape of an XSec
    \code{.cpp}
    // Add Stack
    string sid = AddGeom( "STACK", "" );

    // Get First (and Only) XSec Surf
    string xsec_surf = GetXSecSurf( sid, 0 );

    ChangeXSecShape( xsec_surf, 1, XS_EDIT_CURVE );

    string xsec = GetXSec( xsec_surf, 1 );

    if ( GetXSecShape( xsec ) != XS_EDIT_CURVE ) { Print( "ERROR: GetXSecShape" ); }
    \endcode
    \sa XSEC_CRV_TYPE
    \param [in] xsec_id XSec ID
    \return XSec type enum (i.e. XS_ELLIPSE)
*/)";
    r = se->RegisterGlobalFunction( "int GetXSecShape( const string& in xsec_id )", asFUNCTION( vsp::GetXSecShape ), asCALL_CDECL, doc_struct );
    assert( r >= 0 );

    doc_struct.comment = R"(
/*!
    Get the width of an XSec. Note that POINT type XSecs have a width and height of 0, regardless of what width and height it is set to.
    \code{.cpp}
    // Add Fuselage Geom
    string fuseid = AddGeom( "FUSELAGE", "" );

    string xsec_surf = GetXSecSurf( fuseid, 0 );

    string xsec = GetXSec( xsec_surf, GetNumXSec( xsec_surf ) - 2 ); // Get 2nd to last XSec

    SetXSecWidthHeight( xsec, 3.0, 6.0 );

    if ( abs( GetXSecWidth( xsec ) - 3.0 ) > 1e-6 )        { Print( "---> Error: API Get/Set Width " ); }
    \endcode
    \sa SetXSecWidth
    \param [in] xsec_id XSec ID
    \return Xsec width
*/)";
    r = se->RegisterGlobalFunction( "double GetXSecWidth( const string& in xsec_id )", asFUNCTION( vsp::GetXSecWidth ), asCALL_CDECL, doc_struct );
    assert( r >= 0 );

    doc_struct.comment = R"(
/*!
    Get the height of an XSec. Note that POINT type XSecs have a width and height of 0, regardless of what width and height it is set to.
    \code{.cpp}
    // Add Fuselage Geom
    string fuseid = AddGeom( "FUSELAGE", "" );

    string xsec_surf = GetXSecSurf( fuseid, 0 );

    string xsec = GetXSec( xsec_surf, GetNumXSec( xsec_surf ) - 2 ); // Get 2nd to last XSec

    SetXSecWidthHeight( xsec, 3.0, 6.0 );

    if ( abs( GetXSecHeight( xsec ) - 6.0 ) > 1e-6 )        { Print( "---> Error: API Get/Set Width " ); }
    \endcode
    \sa SetXSecHeight
    \param [in] xsec_id XSec ID
    \return Xsec height
*/)";
    r = se->RegisterGlobalFunction( "double GetXSecHeight( const string& in xsec_id )", asFUNCTION( vsp::GetXSecHeight ), asCALL_CDECL, doc_struct );
    assert( r >= 0 );

    doc_struct.comment = R"(
/*!
    Set the width of an XSec. Note that POINT type XSecs have a width and height of 0, regardless of what is input to SetXSecWidth.
    \code{.cpp}
    // Add Stack
    string sid = AddGeom( "STACK", "" );

    // Get First (and Only) XSec Surf
    string xsec_surf = GetXSecSurf( sid, 0 );

    // Identify XSec 2
    string xsec_2 = GetXSec( xsec_surf, 2 );

    SetXSecWidth( xsec_2, 1.5 );
    \endcode
    \sa GetXSecWidth
    \param [in] xsec_id XSec ID
    \param [in] w Xsec width
*/)";
    r = se->RegisterGlobalFunction( "void SetXSecWidth( const string& in xsec_id, double w )", asFUNCTION( vsp::SetXSecWidth ), asCALL_CDECL, doc_struct );
    assert( r >= 0 );

    doc_struct.comment = R"(
/*!
    Set the height of an XSec. Note that POINT type XSecs have a width and height of 0, regardless of what is input to SetXSecHeight.
    \code{.cpp}
    // Add Stack
    string sid = AddGeom( "STACK", "" );

    // Get First (and Only) XSec Surf
    string xsec_surf = GetXSecSurf( sid, 0 );

    // Identify XSec 2
    string xsec_2 = GetXSec( xsec_surf, 2 );

    SetXSecHeight( xsec_2, 1.5 );
    \endcode
    \sa GetXSecHeight
    \param [in] xsec_id XSec ID
    \param [in] h Xsec height
*/)";
    r = se->RegisterGlobalFunction( "void SetXSecHeight( const string& in xsec_id, double h )", asFUNCTION( vsp::SetXSecHeight ), asCALL_CDECL, doc_struct );
    assert( r >= 0 );

    doc_struct.comment = R"(
/*!
    Set the width and height of an XSec. Note, if the XSec is an EDIT_CURVE type and PreserveARFlag is true, the input width value will be 
    ignored and instead set from on the input height and aspect ratio. Use SetXSecWidth and SetXSecHeight directly to avoid this. 
    \code{.cpp}
    // Add Stack
    string sid = AddGeom( "STACK", "" );

    // Get First (and Only) XSec Surf
    string xsec_surf = GetXSecSurf( sid, 0 );

    // Identify XSec 2
    string xsec_2 = GetXSec( xsec_surf, 2 );

    SetXSecWidthHeight( xsec_2, 1.5, 1.5 );
    \endcode
    \sa SetXSecWidth, SetXSecHeight
    \param [in] xsec_id XSec ID
    \param [in] w Xsec width
    \param [in] h Xsec height
*/)";
    r = se->RegisterGlobalFunction( "void SetXSecWidthHeight( const string& in xsec_id, double w, double h )", asFUNCTION( vsp::SetXSecWidthHeight ), asCALL_CDECL, doc_struct );
    assert( r >= 0 );

    doc_struct.comment = R"(
/*!
    Get all Parm IDs for specified XSec Parm Container
    \code{.cpp}
    // Add Fuselage Geom
    string fuseid = AddGeom( "FUSELAGE", "" );

    string xsec_surf = GetXSecSurf( fuseid, 0 );

    string xsec = GetXSec( xsec_surf, GetNumXSec( xsec_surf ) - 1 );

    array< string > @parm_array = GetXSecParmIDs( xsec );

    if ( parm_array.size() < 1 )                        { Print( "---> Error: API GetXSecParmIDs " ); }
    \endcode
    \param [in] xsec_id XSec ID
    \return Array of Parm IDs
*/)";
    r = se->RegisterGlobalFunction( "array<string>@ GetXSecParmIDs(const string & in xsec_id )", asMETHOD( ScriptMgrSingleton, GetXSecParmIDs ), asCALL_THISCALL_ASGLOBAL, &ScriptMgr, doc_struct );
    assert( r >= 0 );

    doc_struct.comment = R"(
/*!
    Get a specific Parm ID from an Xsec
    \code{.cpp}
    // Add Fuselage Geom
    string fuseid = AddGeom( "FUSELAGE", "" );

    string xsec_surf = GetXSecSurf( fuseid, 0 );

    ChangeXSecShape( xsec_surf, GetNumXSec( xsec_surf ) - 1, XS_ROUNDED_RECTANGLE );

    string xsec = GetXSec( xsec_surf, GetNumXSec( xsec_surf ) - 1 );

    string wid = GetXSecParm( xsec, "RoundedRect_Width" );

    if ( !ValidParm( wid ) )                            { Print( "---> Error: API GetXSecParm " ); }
    \endcode
    \param [in] xsec_id XSec ID
    \param [in] name Parm name
    \return Parm ID
*/)";
    r = se->RegisterGlobalFunction( "string GetXSecParm( const string& in xsec_id, const string& in name )", asFUNCTION( vsp::GetXSecParm ), asCALL_CDECL, doc_struct );
    assert( r >= 0 );

    doc_struct.comment = R"(
/*!
    Read in XSec shape from fuselage (*.fsx) file and set to the specified XSec. The XSec must be of type XS_FILE_FUSE.
    \code{.cpp}
    // Add Fuselage Geom
    string fuseid = AddGeom( "FUSELAGE", "" );

    string xsec_surf = GetXSecSurf( fuseid, 0 );

    ChangeXSecShape( xsec_surf, 2, XS_FILE_FUSE );

    string xsec = GetXSec( xsec_surf, 2 );

    array< vec3d > @vec_array = ReadFileXSec( xsec, "TestXSec.fxs" );
    \endcode
    \param [in] xsec_id XSec ID
    \param [in] file_name Fuselage XSec file name
    \return Array of coordinate points read from the file and set to the XSec
*/)";
    r = se->RegisterGlobalFunction( "array<vec3d>@ ReadFileXSec(const string& in xsec_id, const string& in file_name )", asMETHOD( ScriptMgrSingleton, ReadFileXSec ), asCALL_THISCALL_ASGLOBAL, &ScriptMgr, doc_struct );
    assert( r >= 0 );

    doc_struct.comment = R"(
/*!
    Set the coordinate points for a specific XSec. The XSec must be of type XS_FILE_FUSE.
    \code{.cpp}
    // Add Fuselage Geom
    string fuseid = AddGeom( "FUSELAGE", "" );

    string xsec_surf = GetXSecSurf( fuseid, 0 );

    ChangeXSecShape( xsec_surf, 2, XS_FILE_FUSE );

    string xsec = GetXSec( xsec_surf, 2 );

    array< vec3d > @vec_array = ReadFileXSec( xsec, "TestXSec.fxs" );

    if ( vec_array.size() > 0 )
    {
        vec_array[1] = vec_array[1] * 2.0;
        vec_array[3] = vec_array[3] * 2.0;

        SetXSecPnts( xsec, vec_array );
    }
    \endcode
    \param [in] xsec_id XSec ID
    \param [in] pnt_arr Array of XSec coordinate points
*/)";
    r = se->RegisterGlobalFunction( "void SetXSecPnts( const string& in xsec_id, array<vec3d>@ pnt_arr )", asMETHOD( ScriptMgrSingleton, SetXSecPnts ), asCALL_THISCALL_ASGLOBAL, &ScriptMgr, doc_struct );
    assert( r >= 0 );

    doc_struct.comment = R"(
/*!
    Compute 3D coordinate for a point on an XSec curve given the parameter value (U) along the curve 
    \code{.cpp}
    //==== Add Geom ====//
    string stack_id = AddGeom( "STACK" );

    //==== Get The XSec Surf ====//
    string xsec_surf = GetXSecSurf( stack_id, 0 );

    string xsec = GetXSec( xsec_surf, 2 );

    double u_fract = 0.25;

    vec3d pnt = ComputeXSecPnt( xsec, u_fract );
    \endcode
    \param [in] xsec_id XSec ID
    \param [in] fract Curve parameter value (range: 0 - 1)
    \return 3D coordinate point
*/)";
    r = se->RegisterGlobalFunction( "vec3d ComputeXSecPnt( const string& in xsec_id, double fract )", asFUNCTION( vsp::ComputeXSecPnt ), asCALL_CDECL, doc_struct );
    assert( r >= 0 );

    doc_struct.comment = R"(
/*!
    Compute the tangent vector of a point on an XSec curve given the parameter value (U) along the curve 
    \code{.cpp}
    //==== Add Geom ====//
    string stack_id = AddGeom( "STACK" );

    //==== Get The XSec Surf ====//
    string xsec_surf = GetXSecSurf( stack_id, 0 );

    string xsec = GetXSec( xsec_surf, 2 );

    double u_fract = 0.25;

    vec3d tan = ComputeXSecTan( xsec, u_fract );
    \endcode
    \param [in] xsec_id XSec ID
    \param [in] fract Curve parameter value (range: 0 - 1)
    \return Tangent vector
*/)";
    r = se->RegisterGlobalFunction( "vec3d ComputeXSecTan( const string& in xsec_id, double fract )", asFUNCTION( vsp::ComputeXSecTan ), asCALL_CDECL, doc_struct );
    assert( r >= 0 );

    doc_struct.comment = R"(
/*!
    Reset all skinning Parms for a specified XSec. Set top, bottom, left, and right strengths, slew, angle, and curvature to 0. Set all symmetry and equality conditions to false.
    \code{.cpp}
    string fid = AddGeom( "FUSELAGE", "" );             // Add Fuselage

    string xsec_surf = GetXSecSurf( fid, 0 );           // Get First (and Only) XSec Surf

    int num_xsecs = GetNumXSec( xsec_surf );

    string xsec = GetXSec( xsec_surf, 1 );

    SetXSecTanAngles( xsec, XSEC_BOTH_SIDES, 0.0 );       // Set Tangent Angles At Cross Section
    SetXSecContinuity( xsec, 1 );                       // Set Continuity At Cross Section

    ResetXSecSkinParms( xsec );
    \endcode
    \param [in] xsec_id XSec ID
*/)";
    r = se->RegisterGlobalFunction( "void ResetXSecSkinParms( const string& in xsec_id )", asFUNCTION( vsp::ResetXSecSkinParms ), asCALL_CDECL, doc_struct );
    assert( r >= 0 );

    doc_struct.comment = R"(
/*!
    Set C-type continuity enforcement for a particular XSec
    \code{.cpp}
    string fid = AddGeom( "FUSELAGE", "" );             // Add Fuselage

    string xsec_surf = GetXSecSurf( fid, 0 );           // Get First (and Only) XSec Surf

    int num_xsecs = GetNumXSec( xsec_surf );

    for ( int i = 0 ; i < num_xsecs ; i++ )
    {
        string xsec = GetXSec( xsec_surf, i );

        SetXSecContinuity( xsec, 1 );                       // Set Continuity At Cross Section
    }
    \endcode
    \param [in] xsec_id XSec ID
    \param [in] cx Continuity level (0, 1, or 2)
*/)";
    r = se->RegisterGlobalFunction( "void SetXSecContinuity( const string& in xsec_id, int cx )", asFUNCTION( vsp::SetXSecContinuity ), asCALL_CDECL, doc_struct );
    assert( r >= 0 );

    doc_struct.comment = R"(
/*!
    Set the tangent angles for the specified XSec
    \code{.cpp}
    // Add Stack
    string sid = AddGeom( "STACK", "" );

    // Get First (and Only) XSec Surf
    string xsec_surf = GetXSecSurf( sid, 0 );

    int num_xsecs = GetNumXSec( xsec_surf );

    for ( int i = 0 ; i < num_xsecs ; i++ )
    {
        string xsec = GetXSec( xsec_surf, i );

        SetXSecTanAngles( xsec, XSEC_BOTH_SIDES, 10.0 );       // Set Tangent Angles At Cross Section
    }
    \endcode
    \sa XSEC_SIDES_TYPE
    \param [in] xsec_id XSec ID
    \param [in] side Side type enum (i.e. XSEC_BOTH_SIDES)
    \param [in] top Top angle (degrees)
    \param [in] right Right angle (degrees)
    \param [in] bottom Bottom angle (degrees)
    \param [in] left Left angle (degrees)
*/)";
    r = se->RegisterGlobalFunction( "void SetXSecTanAngles( const string& in xsec_id, int side, double top, double right = -1.0e12, double bottom = -1.0e12, double left = -1.0e12 )", asFUNCTION( vsp::SetXSecTanAngles ), asCALL_CDECL, doc_struct );
    assert( r >= 0 );

    doc_struct.comment = R"(
/*!
    Set the tangent slew angles for the specified XSec
    \code{.cpp}
    // Add Stack
    string sid = AddGeom( "STACK", "" );

    // Get First (and Only) XSec Surf
    string xsec_surf = GetXSecSurf( sid, 0 );

    int num_xsecs = GetNumXSec( xsec_surf );

    for ( int i = 0 ; i < num_xsecs ; i++ )
    {
        string xsec = GetXSec( xsec_surf, i );

        SetXSecTanSlews( xsec, XSEC_BOTH_SIDES, 5.0 );       // Set Tangent Slews At Cross Section
    }
    \endcode
    \sa XSEC_SIDES_TYPE
    \param [in] xsec_id XSec ID
    \param [in] side Side type enum (i.e. XSEC_BOTH_SIDES)
    \param [in] top Top angle (degrees)
    \param [in] right Right angle (degrees)
    \param [in] bottom Bottom angle (degrees)
    \param [in] left Left angle (degrees)
*/)";
    r = se->RegisterGlobalFunction( "void SetXSecTanSlews( const string& in xsec_id, int side, double top, double right = -1.0e12, double bottom = -1.0e12, double left = -1.0e12 )", asFUNCTION( vsp::SetXSecTanSlews ), asCALL_CDECL, doc_struct );
    assert( r >= 0 );

    doc_struct.comment = R"(
/*!
    Set the tangent strengths for the specified XSec
    \code{.cpp}
    // Add Stack
    string sid = AddGeom( "STACK", "" );

    // Get First (and Only) XSec Surf
    string xsec_surf = GetXSecSurf( sid, 0 );

    // Flatten ends
    int num_xsecs = GetNumXSec( xsec_surf );

    for ( int i = 0 ; i < num_xsecs ; i++ )
    {
        string xsec = GetXSec( xsec_surf, i );

        SetXSecTanStrengths( xsec, XSEC_BOTH_SIDES, 0.8 );  // Set Tangent Strengths At Cross Section
    }
    \endcode
    \sa XSEC_SIDES_TYPE
    \param [in] xsec_id XSec ID
    \param [in] side Side type enum (i.e. XSEC_BOTH_SIDES)
    \param [in] top Top strength
    \param [in] right Right strength
    \param [in] bottom Bottom strength
    \param [in] left Left strength
*/)";
    r = se->RegisterGlobalFunction( "void SetXSecTanStrengths( const string& in xsec_id, int side, double top, double right = -1.0e12, double bottom = -1.0e12, double left = -1.0e12 )", asFUNCTION( vsp::SetXSecTanStrengths ), asCALL_CDECL, doc_struct );
    assert( r >= 0 );

    doc_struct.comment = R"(
/*!
    Set curvatures for the specified XSec
    \code{.cpp}
    // Add Stack
    string sid = AddGeom( "STACK", "" );

    // Get First (and Only) XSec Surf
    string xsec_surf = GetXSecSurf( sid, 0 );

    // Flatten ends
    int num_xsecs = GetNumXSec( xsec_surf );

    for ( int i = 0 ; i < num_xsecs ; i++ )
    {
        string xsec = GetXSec( xsec_surf, i );

        SetXSecCurvatures( xsec, XSEC_BOTH_SIDES, 0.2 );  // Set Tangent Strengths At Cross Section
    }
    \endcode
    \sa XSEC_SIDES_TYPE
    \param [in] xsec_id XSec ID
    \param [in] side Side type enum (i.e. XSEC_BOTH_SIDES)
    \param [in] top Top curvature
    \param [in] right Right curvature
    \param [in] bottom Bottom curvature
    \param [in] left Left curvature
*/)";
    r = se->RegisterGlobalFunction( "void SetXSecCurvatures( const string& in xsec_id, int side, double top, double right = -1.0e12, double bottom = -1.0e12, double left = -1.0e12 )", asFUNCTION( vsp::SetXSecCurvatures ), asCALL_CDECL, doc_struct );
    assert( r >= 0 );

    doc_struct.comment = R"(
/*!
    Read in XSec shape from airfoil file and set to the specified XSec. The XSec must be of type XS_FILE_AIRFOIL. Airfoil files may be in Lednicer or Selig format with *.af or *.dat extensions. 
    \code{.cpp}
    // Add Fuselage Geom
    string fuseid = AddGeom( "FUSELAGE", "" );

    string xsec_surf = GetXSecSurf( fuseid, 0 );

    ChangeXSecShape( xsec_surf, 1, XS_FILE_AIRFOIL );

    string xsec = GetXSec( xsec_surf, 1 );

    ReadFileAirfoil( xsec, "airfoil/N0012_VSP.af" );
    \endcode
    \param [in] xsec_id XSec ID
    \param [in] file_name Airfoil XSec file name
*/)";
    r = se->RegisterGlobalFunction( "void ReadFileAirfoil( const string& in xsec_id, const string& in file_name )", asFUNCTION( vsp::ReadFileAirfoil ), asCALL_CDECL, doc_struct );
    assert( r >= 0 );

    doc_struct.comment = R"(
/*!
    Set the upper points for an airfoil. The XSec must be of type XS_FILE_AIRFOIL.
    \code{.cpp}
    // Add Fuselage Geom
    string fuseid = AddGeom( "FUSELAGE", "" );

    string xsec_surf = GetXSecSurf( fuseid, 0 );

    ChangeXSecShape( xsec_surf, 1, XS_FILE_AIRFOIL );

    string xsec = GetXSec( xsec_surf, 1 );

    ReadFileAirfoil( xsec, "airfoil/N0012_VSP.af" );

    array< vec3d > @up_array = GetAirfoilUpperPnts( xsec );

    for ( int i = 0 ; i < int( up_array.size() ) ; i++ )
    {
        up_array[i].scale_y( 2.0 );
    }

    SetAirfoilUpperPnts( xsec, up_array );
    \endcode
    \param [in] xsec_id XSec ID
    \param [in] up_pnt_vec Array of points defining the upper surface of the airfoil
*/)";
    r = se->RegisterGlobalFunction( "void SetAirfoilUpperPnts( const string& in xsec_id, array<vec3d>@ up_pnt_vec )", asMETHOD( ScriptMgrSingleton, SetAirfoilUpperPnts ), asCALL_THISCALL_ASGLOBAL, &ScriptMgr, doc_struct );
    assert( r >= 0 );

    doc_struct.comment = R"(
/*!
    Set the ower points for an airfoil. The XSec must be of type XS_FILE_AIRFOIL.
    \code{.cpp}
    // Add Fuselage Geom
    string fuseid = AddGeom( "FUSELAGE", "" );

    string xsec_surf = GetXSecSurf( fuseid, 0 );

    ChangeXSecShape( xsec_surf, 1, XS_FILE_AIRFOIL );

    string xsec = GetXSec( xsec_surf, 1 );

    ReadFileAirfoil( xsec, "airfoil/N0012_VSP.af" );

    array< vec3d > @low_array = GetAirfoilLowerPnts( xsec );

    for ( int i = 0 ; i < int( low_array.size() ) ; i++ )
    {
        low_array[i].scale_y( 0.5 );
    }

    SetAirfoilUpperPnts( xsec, up_array );
    \endcode
    \param [in] xsec_id XSec ID
    \param [in] low_pnt_vec Array of points defining the lower surface of the airfoil
*/)";
    r = se->RegisterGlobalFunction( "void SetAirfoilLowerPnts( const string& in xsec_id, array<vec3d>@ low_pnt_vec )", asMETHOD( ScriptMgrSingleton, SetAirfoilLowerPnts ), asCALL_THISCALL_ASGLOBAL, &ScriptMgr, doc_struct );
    assert( r >= 0 );

    doc_struct.comment = R"(
/*!
    Set the upper and lower points for an airfoil. The XSec must be of type XS_FILE_AIRFOIL.
    \code{.cpp}
    // Add Fuselage Geom
    string fuseid = AddGeom( "FUSELAGE", "" );

    string xsec_surf = GetXSecSurf( fuseid, 0 );

    ChangeXSecShape( xsec_surf, 1, XS_FILE_AIRFOIL );

    string xsec = GetXSec( xsec_surf, 1 );

    ReadFileAirfoil( xsec, "airfoil/N0012_VSP.af" );

    array< vec3d > @up_array = GetAirfoilUpperPnts( xsec );

    array< vec3d > @low_array = GetAirfoilLowerPnts( xsec );

    for ( int i = 0 ; i < int( up_array.size() ) ; i++ )
    {
        up_array[i].scale_y( 2.0 );

        low_array[i].scale_y( 0.5 );
    }

    SetAirfoilPnts( xsec, up_array, low_array );
    \endcode
    \param [in] xsec_id XSec ID
    \param [in] up_pnt_vec Array of points defining the upper surface of the airfoil
    \param [in] low_pnt_vec Array of points defining the lower surface of the airfoil
*/)";
    r = se->RegisterGlobalFunction( "void SetAirfoilPnts( const string& in xsec_id, array<vec3d>@ up_pnt_vec, array<vec3d>@ low_pnt_vec )", asMETHOD( ScriptMgrSingleton, SetAirfoilPnts ), asCALL_THISCALL_ASGLOBAL, &ScriptMgr, doc_struct );
    assert( r >= 0 );

    doc_struct.comment = R"(
/*!
    Get the theoretical lift (Cl) distribution for a Hershey Bar wing with unit chord length using Glauert's Method. This function was initially created to compare VSPAERO results to Lifting Line Theory. 
    If full_span_flag is set to true symmetry is applied to the results.
    \code{.cpp}
    // Compute theoretical lift and drag distributions using 100 points
    double Vinf = 100;

    double halfAR = 20;

    double alpha_deg = 10;

    int n_pts = 100;

    array<vec3d> cl_dist_theo = GetHersheyBarLiftDist( int( n_pts ), Deg2Rad( alpha_deg ), Vinf, ( 2 * halfAR ), false );

    array<vec3d> cd_dist_theo = GetHersheyBarDragDist( int( n_pts ), Deg2Rad( alpha_deg ), Vinf, ( 2 * halfAR ), false );
    \endcode
    \param [in] npts Number of points along the span to assess
    \param [in] alpha Wing angle of attack (Radians)
    \param [in] Vinf Freestream velocity
    \param [in] span Hershey Bar full-span. Note, only half is used in the calculation
    \param [in] full_span_flag Flag to apply symmetry to results
    \return Theoretical coefficient of lift distribution array (size = 2*npts if full_span_flag = true)
*/)";
    r = se->RegisterGlobalFunction( "array<vec3d>@ GetHersheyBarLiftDist( const int& in npts, const double& in alpha, const double& in Vinf, const double& in span, bool full_span_flag = false )", asMETHOD( ScriptMgrSingleton, GetHersheyBarLiftDist ), asCALL_THISCALL_ASGLOBAL, &ScriptMgr, doc_struct );
    assert( r >= 0 );

    doc_struct.comment = R"(
/*!
    Get the theoretical drag (Cd) distribution for a Hershey Bar wing with unit chord length using Glauert's Method. This function was initially created to compare VSPAERO results to Lifting Line Theory. 
    If full_span_flag is set to true symmetry is applied to the results.
    \code{.cpp}
    // Compute theoretical lift and drag distributions using 100 points
    double Vinf = 100;

    double halfAR = 20;

    double alpha_deg = 10;

    int n_pts = 100;

    array<vec3d> cl_dist_theo = GetHersheyBarLiftDist( int( n_pts ), Deg2Rad( alpha_deg ), Vinf, ( 2 * halfAR ), false );

    array<vec3d> cd_dist_theo = GetHersheyBarDragDist( int( n_pts ), Deg2Rad( alpha_deg ), Vinf, ( 2 * halfAR ), false );
    \endcode
    \param [in] npts Number of points along the span to assess
    \param [in] alpha Wing angle of attack (Radians)
    \param [in] Vinf Freestream velocity
    \param [in] span Hershey Bar full-span. Note, only half is used in the calculation
    \param [in] full_span_flag Flag to apply symmetry to results (default: false)
    \return Theoretical coefficient of drag distribution array (size = 2*npts if full_span_flag = true)
*/)";
    r = se->RegisterGlobalFunction( "array<vec3d>@ GetHersheyBarDragDist( const int& in npts, const double& in alpha, const double& in Vinf, const double& in span, bool full_span_flag = false )", asMETHOD( ScriptMgrSingleton, GetHersheyBarDragDist ), asCALL_THISCALL_ASGLOBAL, &ScriptMgr, doc_struct );
    assert( r >= 0 );

    doc_struct.comment = R"(
/*!
    Get the 2D coordinates an input number of points along a Von K�rm�n-Trefftz airfoil of specified shape
    \code{.cpp}
    const double pi = 3.14159265358979323846;

    const int npts = 122;

    const double alpha = 0.0;

    const double epsilon = 0.1;

    const double kappa = 0.1;

    const double tau = 10;

    array<vec3d> xyz_airfoil = GetVKTAirfoilPnts(npts, alpha, epsilon, kappa, tau*(pi/180) );

    array<double> cp_dist = GetVKTAirfoilCpDist( alpha, epsilon, kappa, tau*(pi/180), xyz_airfoil );
    \endcode
    \param [in] npts Number of points along the airfoil to return
    \param [in] alpha Airfoil angle of attack (Radians)
    \param [in] epsilon Airfoil thickness
    \param [in] kappa Airfoil camber
    \param [in] tau Airfoil trailing edge angle (Radians)
    \return Array of points on the VKT airfoil (size = npts)
*/)";
    r = se->RegisterGlobalFunction( "array<vec3d>@ GetVKTAirfoilPnts( const int& in npts, const double& in alpha, const double& in epsilon, const double& in kappa, const double& in tau )", asMETHOD( ScriptMgrSingleton, GetVKTAirfoilPnts ), asCALL_THISCALL_ASGLOBAL, &ScriptMgr, doc_struct );
    assert( r >= 0 );

    doc_struct.comment = R"(
/*!
    Get the pressure coefficient (Cp) along a Von Kármán-Trefftz airfoil of specified shape at specified points along the airfoil
    \code{.cpp}
    const double pi = 3.14159265358979323846;

    const int npts = 122;

    const double alpha = 0.0;

    const double epsilon = 0.1;

    const double kappa = 0.1;

    const double tau = 10;

    array<vec3d> xyz_airfoil = GetVKTAirfoilPnts(npts, alpha, epsilon, kappa, tau*(pi/180) );

    array<double> cp_dist = GetVKTAirfoilCpDist( alpha, epsilon, kappa, tau*(pi/180), xyz_airfoil );
    \endcode
    \sa GetVKTAirfoilPnts
    \param [in] alpha Airfoil angle of attack (Radians)
    \param [in] epsilon Airfoil thickness
    \param [in] kappa Airfoil camber
    \param [in] tau Airfoil trailing edge angle (Radians)
    \param [in] xydata Array of points on the airfoil to evaluate
    \return Array of Cp values for each point in xydata
*/)";
    r = se->RegisterGlobalFunction( "array<double>@ GetVKTAirfoilCpDist( const double& in alpha, const double& in epsilon, const double& in kappa, const double& in tau, array<vec3d>@ xydata )", asMETHOD( ScriptMgrSingleton, GetVKTAirfoilCpDist ), asCALL_THISCALL_ASGLOBAL, &ScriptMgr, doc_struct );
    assert( r >= 0 );

    doc_struct.comment = R"(
/*!
    Generate the surface coordinate points for a ellipsoid at specified center of input radius along each axis. 
    Based on the MATLAB function ellipsoid (https://in.mathworks.com/help/matlab/ref/ellipsoid.html).
    \sa GetVKTAirfoilPnts
    \param [in] center 3D location of the ellipsoid center
    \param [in] abc_rad Radius along the A (X), B (Y), and C (Z) axes
    \param [in] u_npts Number of points in the U direction
    \param [in] w_npts Number of points in the W direction
    \return Array of coordinates describing the ellipsoid surface
*/)";
    r = se->RegisterGlobalFunction( "array<vec3d>@ GetEllipsoidSurfPnts( const vec3d& in center, const vec3d& in abc_rad, int u_npts = 20, int w_npts = 20 )", asMETHOD( ScriptMgrSingleton, GetEllipsoidSurfPnts ), asCALL_THISCALL_ASGLOBAL, &ScriptMgr, doc_struct );
    assert( r >= 0 );  // TODO: Example

    doc_struct.comment = R"(
/*!
    Get the points along the feature lines of a particular Geom
    \param [in] geom_id Geom ID
    \return Array of points along the Geom's feature lines
*/)";
    r = se->RegisterGlobalFunction( "array<vec3d>@ GetFeatureLinePnts( const string& in geom_id )", asMETHOD( ScriptMgrSingleton, GetFeatureLinePnts ), asCALL_THISCALL_ASGLOBAL, &ScriptMgr, doc_struct );
    assert( r >= 0 );  // TODO: Example

    doc_struct.comment = R"(
/*!
    Generate Analytical Solution for Potential Flow for specified ellipsoid shape at input surface points for input velocity vector.
    Based on Munk, M. M., 'Remarks on the Pressure Distribution over the Surface of an Ellipsoid, Moving Translationally Through a Perfect 
    Fluid,' NACA TN-196, June 1924. Function initially created to compare VSPAERO results to theory.
    \code{.cpp}
    const double pi = 3.14159265358979323846;

    const int npts = 101;

    const vec3d abc_rad = vec3d(1.0, 2.0, 3.0);

    const double alpha = 5; // deg

    const double beta = 5; // deg

    const double V_inf = 100.0;

    array < vec3d > x_slice_pnt_vec(npts);
    array<double> theta_vec(npts);

    theta_vec[0] = 0;

    for ( int i = 1; i < npts; i++ )
    {
        theta_vec[i] = theta_vec[i-1] + (2 * pi / ( npts - 1) );
    }

    for ( int i = 0; i < npts; i++ )
    {
        x_slice_pnt_vec[i] = vec3d( 0, abc_rad[1] * cos( theta_vec[i] ), abc_rad[2] *sin( theta_vec[i] ) );
    }

    vec3d V_vec = vec3d( ( V_inf * cos( Deg2Rad( alpha ) ) * cos( Deg2Rad( beta ) ) ), ( V_inf * sin( Deg2Rad( beta ) ) ), ( V_inf * sin( Deg2Rad( alpha ) ) * cos( Deg2Rad( beta ) ) ) );

    array < double > cp_dist = GetEllipsoidCpDist( x_slice_pnt_vec, abc_rad, V_vec );
    \endcode
    \sa GetEllipsoidSurfPnts
    \param [in] surf_pnt_arr Array of points on the ellipsoid surface to assess
    \param [in] abc_rad Radius along the A (X), B (Y), and C (Z) axes
    \param [in] V_inf 3D components of freestream velocity
    \return Array of Cp results corresponding to each point in surf_pnt_arr
*/)";
    r = se->RegisterGlobalFunction( "array<double>@ GetEllipsoidCpDist( array<vec3d>@ surf_pnt_arr, const vec3d& in abc_rad, const vec3d& in V_inf )", asMETHOD( ScriptMgrSingleton, GetEllipsoidCpDist ), asCALL_THISCALL_ASGLOBAL, &ScriptMgr, doc_struct );
    assert( r >= 0 );

    doc_struct.comment = R"(
/*!
    Get the coordinate points for the upper surface of an airfoil. The XSec must be of type XS_FILE_AIRFOIL
    \code{.cpp}
    // Add Fuselage Geom
    string fuseid = AddGeom( "FUSELAGE", "" );

    string xsec_surf = GetXSecSurf( fuseid, 0 );

    ChangeXSecShape( xsec_surf, 1, XS_FILE_AIRFOIL );

    string xsec = GetXSec( xsec_surf, 1 );

    ReadFileAirfoil( xsec, "airfoil/N0012_VSP.af" );

    array< vec3d > @up_array = GetAirfoilUpperPnts( xsec );
    \endcode
    \sa SetAirfoilPnts
    \param [in] xsec_id XSec ID
    \return Array of coordinate points for the upper airfoil surface
*/)";
    r = se->RegisterGlobalFunction( "array<vec3d>@ GetAirfoilUpperPnts(const string& in xsec_id )", asMETHOD( ScriptMgrSingleton, GetAirfoilUpperPnts ), asCALL_THISCALL_ASGLOBAL, &ScriptMgr, doc_struct );
    assert( r >= 0 );

    doc_struct.comment = R"(
/*!
    Get the coordinate points for the lower surface of an airfoil. The XSec must be of type XS_FILE_AIRFOIL
    \code{.cpp}
    // Add Fuselage Geom
    string fuseid = AddGeom( "FUSELAGE", "" );

    string xsec_surf = GetXSecSurf( fuseid, 0 );

    ChangeXSecShape( xsec_surf, 1, XS_FILE_AIRFOIL );

    string xsec = GetXSec( xsec_surf, 1 );

    ReadFileAirfoil( xsec, "airfoil/N0012_VSP.af" );

    array< vec3d > @low_array = GetAirfoilLowerPnts( xsec );
    \endcode
    \sa SetAirfoilPnts
    \param [in] xsec_id XSec ID
    \return Array of coordinate points for the lower airfoil surface
*/)";
    r = se->RegisterGlobalFunction( "array<vec3d>@ GetAirfoilLowerPnts(const string& in xsec_id )", asMETHOD( ScriptMgrSingleton, GetAirfoilLowerPnts ), asCALL_THISCALL_ASGLOBAL, &ScriptMgr, doc_struct );
    assert( r >= 0 );

    doc_struct.comment = R"(
/*!
    Get the CST coefficients for the upper surface of an airfoil. The XSec must be of type XS_CST_AIRFOIL
    \sa SetUpperCST
    \param [in] xsec_id XSec ID
    \return Array of CST coefficients for the upper airfoil surface
*/)";
    r = se->RegisterGlobalFunction( "array<double>@ GetUpperCSTCoefs( const string & in xsec_id )", asMETHOD( ScriptMgrSingleton, GetUpperCSTCoefs ), asCALL_THISCALL_ASGLOBAL, &ScriptMgr, doc_struct );
    assert( r >= 0 ); // TODO: Example

    doc_struct.comment = R"(
/*!
    Get the CST coefficients for the lower surface of an airfoil. The XSec must be of type XS_CST_AIRFOIL
    \sa SetLowerCST
    \param [in] xsec_id XSec ID
    \return Array of CST coefficients for the lower airfoil surface
*/)";
    r = se->RegisterGlobalFunction( "array<double>@ GetLowerCSTCoefs( const string & in xsec_id )", asMETHOD( ScriptMgrSingleton, GetLowerCSTCoefs ), asCALL_THISCALL_ASGLOBAL, &ScriptMgr, doc_struct );
    assert( r >= 0 );  // TODO: Example

    doc_struct.comment = R"(
/*!
    Get the CST degree for the upper surface of an airfoil. The XSec must be of type XS_CST_AIRFOIL
    \sa SetUpperCST
    \param [in] xsec_id XSec ID
    \return CST Degree for upper airfoil surface
*/)";
    r = se->RegisterGlobalFunction( "int GetUpperCSTDegree( const string& in xsec_id )", asFUNCTION( vsp::GetUpperCSTDegree ), asCALL_CDECL, doc_struct );
    assert( r >= 0 );  // TODO: Example

    doc_struct.comment = R"(
/*!
    Get the CST degree for the lower surface of an airfoil. The XSec must be of type XS_CST_AIRFOIL
    \sa SetLowerCST
    \param [in] xsec_id XSec ID
    \return CST Degree for lower airfoil surface
*/)";
    r = se->RegisterGlobalFunction( "int GetLowerCSTDegree( const string& in xsec_id )", asFUNCTION( vsp::GetLowerCSTDegree ), asCALL_CDECL, doc_struct );
    assert( r >= 0 );  // TODO: Example

    doc_struct.comment = R"(
/*!
    Set the CST degree and coefficients for the upper surface of an airfoil. The number of coefficients should be one more than the CST degree. The XSec must be of type XS_CST_AIRFOIL
    \sa GetUpperCSTDegree, GetUpperCSTCoefs
    \param [in] xsec_id XSec ID
    \param [in] deg CST degree of upper airfoil surface
    \param [in] coeff_arr Array of CST coefficients for the upper airfoil surface
*/)";
    r = se->RegisterGlobalFunction( "void SetUpperCST( const string& in xsec_id, int deg, array<double>@ coeff_arr )", asMETHOD( ScriptMgrSingleton, SetUpperCST ), asCALL_THISCALL_ASGLOBAL, &ScriptMgr, doc_struct );
    assert( r >= 0 );  // TODO: Example

    doc_struct.comment = R"(
/*!
    Set the CST degree and coefficients for the lower surface of an airfoil. The number of coefficients should be one more than the CST degree. The XSec must be of type XS_CST_AIRFOIL
    \sa GetLowerCSTDegree, GetLowerCSTCoefs
    \param [in] xsec_id XSec ID
    \param [in] deg CST degree of lower airfoil surface
    \param [in] coeff_arr Array of CST coefficients for the lower airfoil surface
*/)";
    r = se->RegisterGlobalFunction( "void SetLowerCST( const string& in xsec_id, int deg, array<double>@ coeff_arr )", asMETHOD( ScriptMgrSingleton, SetLowerCST ), asCALL_THISCALL_ASGLOBAL, &ScriptMgr, doc_struct );
    assert( r >= 0 );  // TODO: Example

    doc_struct.comment = R"(
/*!
    Promote the CST for the upper airfoil surface. The XSec must be of type XS_CST_AIRFOIL
    \sa GetUpperCSTDegree
    \param [in] xsec_id XSec ID
*/)";
    r = se->RegisterGlobalFunction( "void PromoteCSTUpper( const string& in xsec_id )", asFUNCTION( vsp::PromoteCSTUpper ), asCALL_CDECL, doc_struct );
    assert( r >= 0 );  // TODO: Example

    doc_struct.comment = R"(
/*!
    Promote the CST for the lower airfoil surface. The XSec must be of type XS_CST_AIRFOIL
    \sa GetLowerCSTDegree
    \param [in] xsec_id XSec ID
*/)";
    r = se->RegisterGlobalFunction( "void PromoteCSTLower( const string& in xsec_id )", asFUNCTION( vsp::PromoteCSTLower ), asCALL_CDECL, doc_struct );
    assert( r >= 0 );  // TODO: Example

    doc_struct.comment = R"(
/*!
    Demote the CST for the upper airfoil surface. The XSec must be of type XS_CST_AIRFOIL
    \sa GetUpperCSTDegree
    \param [in] xsec_id XSec ID
*/)";
    r = se->RegisterGlobalFunction( "void DemoteCSTUpper( const string& in xsec_id )", asFUNCTION( vsp::DemoteCSTUpper ), asCALL_CDECL, doc_struct );
    assert( r >= 0 );  // TODO: Example

    doc_struct.comment = R"(
/*!
    Demote the CST for the lower airfoil surface. The XSec must be of type XS_CST_AIRFOIL
    \sa GetLowerCSTDegree
    \param [in] xsec_id XSec ID
*/)";
    r = se->RegisterGlobalFunction( "void DemoteCSTLower( const string& in xsec_id )", asFUNCTION( vsp::DemoteCSTLower ), asCALL_CDECL, doc_struct );
    assert( r >= 0 );  // TODO: Example

    doc_struct.comment = R"(
/*!
    Fit a CST airfoil for an existing airfoil of type XS_FOUR_SERIES, XS_SIX_SERIES, XS_FOUR_DIGIT_MOD, XS_FIVE_DIGIT, XS_FIVE_DIGIT_MOD, XS_ONE_SIX_SERIES, or XS_FILE_AIRFOIL.
    \param [in] xsec_surf_id XsecSurf ID
    \param [in] xsec_index XSec index
    \param [in] deg CST degree
*/)";
    r = se->RegisterGlobalFunction( "void FitAfCST( const string& in xsec_surf_id, int xsec_index, int deg )", asFUNCTION( vsp::FitAfCST ), asCALL_CDECL, doc_struct );
    assert( r >= 0 );  // TODO: Example

    doc_struct.comment = R"(
/*!
    Write out the untwisted unit-length 2D Bezier curve for the specified airfoil in custom *.bz format. The output will describe the analytical shape of the airfoil. See BezierAirfoilExample.m and BezierCtrlToCoordPnts.m for examples of
    discretizing the Bezier curve and generating a Selig airfoil file. 
    \code{.cpp}
    //==== Add Wing Geometry and Set Parms ====//
    string wing_id = AddGeom( "WING", "" );

    const double u = 0.5; // export airfoil at mid span location

    //==== Write Bezier Airfoil File ====//
    WriteBezierAirfoil( "Example_Bezier.bz", wing_id, u );
    \endcode
    \param [in] file_name Airfoil (*.bz) output file name
    \param [in] geom_id Geom ID
    \param [in] foilsurf_u U location (range: 0 - 1) along the surface. The foil surface does not include root and tip caps (i.e. 2 section wing -> XSec0 @ u=0, XSec1 @ u=0.5, XSec2 @ u=1.0)
*/)";
    r = se->RegisterGlobalFunction( "void WriteBezierAirfoil( const string& in file_name, const string& in geom_id, const double& in foilsurf_u )", asFUNCTION( vsp::WriteBezierAirfoil ), asCALL_CDECL, doc_struct );
    assert( r >= 0 );

    doc_struct.comment = R"(
/*!
    Write out the untwisted unit-length 2D coordinate points for the specified airfoil in Selig format. Coordinate points follow the on-screen wire frame W tessellation. 
    \code{.cpp}
    //==== Add Wing Geometry and Set Parms ====//
    string wing_id = AddGeom( "WING", "" );

    const double u = 0.5; // export airfoil at mid span location

    //==== Write Selig Airfoil File ====//
    WriteSeligAirfoil( "Example_Selig.dat", wing_id, u );
    \endcode
    \sa GetAirfoilCoordinates
    \param [in] file_name Airfoil (*.dat) output file name
    \param [in] geom_id Geom ID
    \param [in] foilsurf_u U location (range: 0 - 1) along the surface. The foil surface does not include root and tip caps (i.e. 2 section wing -> XSec0 @ u=0, XSec1 @ u=0.5, XSec2 @ u=1.0)
*/)";
    r = se->RegisterGlobalFunction( "void WriteSeligAirfoil( const string& in file_name, const string& in geom_id, const double& in foilsurf_u )", asFUNCTION( vsp::WriteSeligAirfoil ), asCALL_CDECL, doc_struct );
    assert( r >= 0 );

    doc_struct.comment = R"(
/*!
    Get the untwisted unit-length 2D coordinate points for the specified airfoil
    \sa WriteSeligAirfoil
    \param [in] geom_id Geom ID
    \param [in] foilsurf_u U location (range: 0 - 1) along the surface. The foil surface does not include root and tip caps (i.e. 2 section wing -> XSec0 @ u=0, XSec1 @ u=0.5, XSec2 @ u=1.0)
*/)";
    r = se->RegisterGlobalFunction( "array<vec3d>@ GetAirfoilCoordinates( const string& in geom_id, const double& in foilsurf_u )", asMETHOD( ScriptMgrSingleton, GetAirfoilCoordinates ), asCALL_THISCALL_ASGLOBAL, &ScriptMgr, doc_struct );
    assert( r >= 0 );

    //==== Edit Curve XSec Functions ====//
    group = "EditCurveXSec";
    doc_struct.group = group.c_str();

    group_description = R"(
    \brief Functions for modifying XSecs of type XS_EDIT_CURVE are defined here. \n\n
    \ref index "Click here to return to the main page" )";
    se->AddGroup( group.c_str(), "Edit Curve XSec Functions", group_description.c_str() );

    doc_struct.comment = R"(
/*!
    Initialize the EditCurveXSec to the current value of m_ShapeType (i.e. EDIT_XSEC_ELLIPSE)
    \code{.cpp}
    // Add Stack
    string sid = AddGeom( "STACK", "" );

    // Get First (and Only) XSec Surf
    string xsec_surf = GetXSecSurf( sid, 0 );

    ChangeXSecShape( xsec_surf, 2, XS_EDIT_CURVE );

    // Identify XSec 2
    string xsec_2 = GetXSec( xsec_surf, 2 );

    // Set XSec 2 to linear
    EditXSecConvertTo( xsec_2, LINEAR );

    EditXSecInitShape( xsec_2 ); // Change back to default ellipse
    \endcode
    \sa INIT_EDIT_XSEC_TYPE
    \param [in] xsec_id XSec ID
*/)";
    r = se->RegisterGlobalFunction( "void EditXSecInitShape( const string& in xsec_id )", asFUNCTION( vsp::EditXSecInitShape ), asCALL_CDECL, doc_struct );
    assert( r >= 0 );

    doc_struct.comment = R"(
/*!
    Convert the EditCurveXSec curve type to the specified new type. Note, EditCurveXSec uses the same enumerations for PCurve to identify curve type, 
    but APPROX_CEDIT is not supported at this time. 
    \code{.cpp}
    // Add Stack
    string sid = AddGeom( "STACK", "" );

    // Get First (and Only) XSec Surf
    string xsec_surf = GetXSecSurf( sid, 0 );

    ChangeXSecShape( xsec_surf, 1, XS_EDIT_CURVE );

    // Identify XSec 1
    string xsec_1 = GetXSec( xsec_surf, 1 );

    // Set XSec 1 to Linear
    EditXSecConvertTo( xsec_1, LINEAR );
    \endcode
    \sa PCURV_TYPE
    \param [in] xsec_id XSec ID
    \param [in] newtype New curve type enum (i.e. CEDIT)
*/)";
    r = se->RegisterGlobalFunction( "void EditXSecConvertTo( const string& in xsec_id, const int& in newtype )", asFUNCTION( vsp::EditXSecConvertTo ), asCALL_CDECL, doc_struct );
    assert( r >= 0 );

    doc_struct.comment = R"(
/*!
    Get the U parameter vector for an EditCurveXSec. The vector will be in increasing order with a range of 0 - 1. 
    \code{.cpp}
    // Add Stack
    string sid = AddGeom( "STACK", "" );

    // Get First (and Only) XSec Surf
    string xsec_surf = GetXSecSurf( sid, 0 );

    ChangeXSecShape( xsec_surf, 2, XS_EDIT_CURVE );

    // Identify XSec 2
    string xsec_2 = GetXSec( xsec_surf, 2 );

    // Set XSec 2 to linear
    EditXSecConvertTo( xsec_2, LINEAR );

    array < double > u_vec = GetEditXSecUVec( xsec_2 );

    if ( u_vec[1] - 0.25 > 1e-6 )
    {
        Print( "Error: GetEditXSecUVec" );
    }
    \endcode
    \param [in] xsec_id XSec ID
    \return Array of U parameter values
*/)";
    r = se->RegisterGlobalFunction( "array<double>@ GetEditXSecUVec( const string& in xsec_id )", asMETHOD( ScriptMgrSingleton, GetEditXSecUVec ), asCALL_THISCALL_ASGLOBAL, &ScriptMgr, doc_struct );
    assert( r >= 0 );

    doc_struct.comment = R"(
/*!
    Get the control point vector for an EditCurveXSec. Note, the returned array of vec3d values will be represented in 2D with Z set to 0.
    \code{.cpp}
    // Add Stack
    string sid = AddGeom( "STACK", "" );

    // Get First (and Only) XSec Surf
    string xsec_surf = GetXSecSurf( sid, 0 );

    ChangeXSecShape( xsec_surf, 1, XS_EDIT_CURVE );

    // Identify XSec 1
    string xsec_1 = GetXSec( xsec_surf, 1 );

    // Get the control points for the default shape
    array < vec3d > xsec1_pts = GetEditXSecCtrlVec( xsec_1, true ); // The returned control points will not be scaled by width and height

    Print( "Normalized Bottom Point of XSecCurve: " + xsec1_pts[3].x() + ", " + xsec1_pts[3].y() + ", " + xsec1_pts[3].z() );
    \endcode
    \param [in] xsec_id XSec ID
    \param [in] non_dimensional True to get the points non-dimensionalized, False to get them scaled by m_Width and m_Height
    \return Array of control points
*/)";
    r = se->RegisterGlobalFunction( "array<vec3d>@ GetEditXSecCtrlVec( const string& in xsec_id, const bool non_dimensional = true )", asMETHOD( ScriptMgrSingleton, GetEditXSecCtrlVec ), asCALL_THISCALL_ASGLOBAL, &ScriptMgr, doc_struct );
    assert( r >= 0 );

    doc_struct.comment = R"(
/*!
    Set the U parameter vector and the control point vector for an EditCurveXSec. The arrays must be of equal length, with the values for U defined in 
    increasing order and range 0 - 1. The input control points to SetEditXSecPnts must be nondimensionalized in the approximate range of [-0.5, 0.5]. 
    \code{.cpp}
    // Add Stack
    string sid = AddGeom( "STACK", "" );

    // Get First (and Only) XSec Surf
    string xsec_surf = GetXSecSurf( sid, 0 );

    ChangeXSecShape( xsec_surf, 2, XS_EDIT_CURVE );

    // Identify XSec 2
    string xsec_2 = GetXSec( xsec_surf, 2 );

    // Set XSec 2 to linear
    EditXSecConvertTo( xsec_2, LINEAR );

    // Turn off R/L symmetry
    SetParmVal( GetXSecParm( xsec_2, "SymType"), SYM_NONE );

    // Define a square 
    array < vec3d > xsec2_pts(5);

    xsec2_pts[0] = vec3d( 0.5, 0.5, 0.0 );
    xsec2_pts[1] = vec3d( 0.5, -0.5, 0.0 );
    xsec2_pts[2] = vec3d( -0.5, -0.5, 0.0 );
    xsec2_pts[3] = vec3d( -0.5, 0.5, 0.0 );
    xsec2_pts[4] = vec3d( 0.5, 0.5, 0.0 );

    // u vec must start at 0.0 and end at 1.0
    array < double > u_vec(5);

    u_vec[0] = 0.0;
    u_vec[1] = 0.25;
    u_vec[2] = 0.5;
    u_vec[3] = 0.75;
    u_vec[4] = 1.0;

    array < double > r_vec(5);

    r_vec[0] = 0.0;
    r_vec[1] = 0.0;
    r_vec[2] = 0.0;
    r_vec[3] = 0.0;
    r_vec[4] = 0.0;

    SetEditXSecPnts( xsec_2, u_vec, xsec2_pts, r_vec ); // Note: points are unscaled by the width and height parms

    array < vec3d > new_pnts = GetEditXSecCtrlVec( xsec_2, true ); // The returned control points will not be scaled by width and height
    
    if ( dist( new_pnts[3], xsec2_pts[3] ) > 1e-6 ) 
    {
        Print( "Error: SetEditXSecPnts");
    }
    \endcode
    \param [in] xsec_id XSec ID
    \param [in] u_vec Array of U parameter values
    \param [in] r_vec Array of R parameter values
    \param [in] control_pts Nondimensionalized array of control points
*/)";
    r = se->RegisterGlobalFunction( "void SetEditXSecPnts( const string& in xsec_id, array<double>@ u_vec, array<vec3d>@ control_pts, array<double>@ r_vec )", asMETHOD( ScriptMgrSingleton, SetEditXSecPnts ), asCALL_THISCALL_ASGLOBAL, &ScriptMgr, doc_struct );
    assert( r >= 0 );

    doc_struct.comment = R"(
/*!
    Delete an EditCurveXSec control point. Note, cubic Bezier intermediate control points (those not on the curve) cannot be deleted.
    The previous and next Bezier control point will be deleted along with the point on the curve. Regardless of curve type, the first
    and last points may not be deleted. 
    \code{.cpp}
    // Add Stack
    string sid = AddGeom( "STACK", "" );

    // Get First (and Only) XSec Surf
    string xsec_surf = GetXSecSurf( sid, 0 );

    ChangeXSecShape( xsec_surf, 2, XS_EDIT_CURVE );

    // Identify XSec 2
    string xsec_2 = GetXSec( xsec_surf, 2 );

    // Turn off R/L symmetry
    SetParmVal( GetXSecParm( xsec_2, "SymType"), SYM_NONE );

    array < vec3d > old_pnts = GetEditXSecCtrlVec( xsec_2, true ); // The returned control points will not be scaled by width and height
    
    EditXSecDelPnt( xsec_2, 3 ); // Remove control point at bottom of circle

    array < vec3d > new_pnts = GetEditXSecCtrlVec( xsec_2, true ); // The returned control points will not be scaled by width and height
    
    if ( old_pnts.size() - new_pnts.size() != 3  ) 
    {
        Print( "Error: EditXSecDelPnt");
    }
    \endcode
    \param [in] xsec_id XSec ID
    \param [in] indx Control point index
*/)";
    r = se->RegisterGlobalFunction( "void EditXSecDelPnt( const string& in xsec_id, const int& in indx )", asFUNCTION( vsp::EditXSecDelPnt ), asCALL_CDECL, doc_struct );
    assert( r >= 0 );

    doc_struct.comment = R"(
/*!
    Split the EditCurveXSec at the specified U value
    \code{.cpp}
    // Add Stack
    string sid = AddGeom( "STACK", "" );

    // Get First (and Only) XSec Surf
    string xsec_surf = GetXSecSurf( sid, 0 );

    ChangeXSecShape( xsec_surf, 2, XS_EDIT_CURVE );

    // Identify XSec 2
    string xsec_2 = GetXSec( xsec_surf, 2 );

    // Turn off R/L symmetry
    SetParmVal( GetXSecParm( xsec_2, "SymType"), SYM_NONE );

    array < vec3d > old_pnts = GetEditXSecCtrlVec( xsec_2, true ); // The returned control points will not be scaled by width and height

    int new_pnt_ind = EditXSecSplit01( xsec_2, 0.375 );

    array < vec3d > new_pnts = GetEditXSecCtrlVec( xsec_2, true ); // The returned control points will not be scaled by width and height
    
    if ( new_pnts.size() - old_pnts.size() != 3  ) 
    {
        Print( "Error: EditXSecSplit01");
    }
    \endcode
    \param [in] xsec_id XSec ID
    \param [in] u U value to split the curve at (0 - 1)
    \return Index of the point added from the split
*/)";
    r = se->RegisterGlobalFunction( "int EditXSecSplit01( const string& in xsec_id, const double& in u )", asFUNCTION( vsp::EditXSecSplit01 ), asCALL_CDECL, doc_struct );
    assert( r >= 0 );

    doc_struct.comment = R"(
/*!
    Move an EditCurveXSec control point. The XSec points are nondimensionalized by m_Width and m_Height and 
    defined in 2D, so the Z value of the new coordinate point will be ignored.
    \code{.cpp}
    // Add Stack
    string sid = AddGeom( "STACK", "" );

    // Get First (and Only) XSec Surf
    string xsec_surf = GetXSecSurf( sid, 0 );

    ChangeXSecShape( xsec_surf, 1, XS_EDIT_CURVE );

    // Identify XSec 1
    string xsec_1 = GetXSec( xsec_surf, 1 );

    // Turn off R/L symmetry
    SetParmVal( GetXSecParm( xsec_1, "SymType"), SYM_NONE );

    // Get the control points for the default shape
    array < vec3d > xsec1_pts = GetEditXSecCtrlVec( xsec_1, true ); // The returned control points will not be scaled by width and height

    // Identify a control point that lies on the curve and shift it in Y
    int move_pnt_ind = 3;

    vec3d new_pnt = vec3d( xsec1_pts[move_pnt_ind].x(), 2 * xsec1_pts[move_pnt_ind].y(), 0.0 );

    // Move the control point
    MoveEditXSecPnt( xsec_1, move_pnt_ind, new_pnt );

    array < vec3d > new_pnts = GetEditXSecCtrlVec( xsec_1, true ); // The returned control points will not be scaled by width and height

    if ( dist( new_pnt, new_pnts[move_pnt_ind] ) > 1e-6 )
    {
        Print( "Error: MoveEditXSecPnt" );
    }
    \endcode
    \param [in] xsec_id XSec ID
    \param [in] indx Control point index
    \param [in] new_pnt Coordinate of the new point
*/)";
    r = se->RegisterGlobalFunction( "void MoveEditXSecPnt( const string& in xsec_id, const int& in indx, const vec3d& in new_pnt )", asFUNCTION( vsp::MoveEditXSecPnt ), asCALL_CDECL, doc_struct );
    assert( r >= 0 );

    doc_struct.comment = R"(
/*!
    Convert any XSec type into an EditCurveXSec. This function will work for BOR Geoms, in which case the input XSec index is ignored.
    \code{.cpp}
    // Add Stack
    string sid = AddGeom( "STACK", "" );

    // Get First (and Only) XSec Surf
    string xsec_surf = GetXSecSurf( sid, 0 );

    ChangeXSecShape( xsec_surf, 1, XS_ROUNDED_RECTANGLE );

    // Convert Rounded Rectangle to Edit Curve type XSec
    ConvertXSecToEdit( sid, 1 );

    // Identify XSec 1
    string xsec_1 = GetXSec( xsec_surf, 1 );

    // Get the control points for the default shape
    array < vec3d > xsec1_pts = GetEditXSecCtrlVec( xsec_1, true ); // The returned control points will not be scaled by width and height
    \endcode
    \param [in] geom_id Geom ID
    \param [in] indx XSec index
*/)";
    r = se->RegisterGlobalFunction( "void ConvertXSecToEdit( const string& in geom_id, const int& in indx = 0 )", asFUNCTION( vsp::ConvertXSecToEdit ), asCALL_CDECL, doc_struct );
    assert( r >= 0 );

    doc_struct.comment = R"(
/*!
    Get the vector of fixed U flags for each control point in an EditCurveXSec. The fixed U flag is used to hold the 
    U parameter of the control point constant when performing an equal arc length reparameterization of the curve. 
    \code{.cpp}
    // Add Wing
    string wid = AddGeom( "WING" );

    // Get First (and Only) XSec Surf
    string xsec_surf = GetXSecSurf( wid, 0 );

    ChangeXSecShape( xsec_surf, 1, XS_EDIT_CURVE );

    // Identify XSec 1
    string xsec_1 = GetXSec( xsec_surf, 1 );

    array < bool > @ fixed_u_vec = GetEditXSecFixedUVec( xsec_1 );

    fixed_u_vec[3] = true; // change a flag
    
    SetEditXSecFixedUVec( xsec_1, fixed_u_vec );

    ReparameterizeEditXSec( xsec_1 );
    \endcode
    \sa SetEditXSecFixedUVec, ReparameterizeEditXSec
    \param [in] xsec_id XSec ID
    \return Array of bool values for each control point
*/)";
    r = se->RegisterGlobalFunction( "array<bool>@ GetEditXSecFixedUVec( const string& in xsec_id )", asMETHOD( ScriptMgrSingleton, GetEditXSecFixedUVec ), asCALL_THISCALL_ASGLOBAL, &ScriptMgr, doc_struct );
    assert( r >= 0 );

    doc_struct.comment = R"(
/*!
    Set the vector of fixed U flags for each control point in an EditCurveXSec. The fixed U flag is used to hold the 
    U parameter of the control point constant when performing an equal arc length reparameterization of the curve. 
    \code{.cpp}
    // Add Wing
    string wid = AddGeom( "WING" );

    // Get First (and Only) XSec Surf
    string xsec_surf = GetXSecSurf( wid, 0 );

    ChangeXSecShape( xsec_surf, 1, XS_EDIT_CURVE );

    // Identify XSec 1
    string xsec_1 = GetXSec( xsec_surf, 1 );

    array < bool > @ fixed_u_vec = GetEditXSecFixedUVec( xsec_1 );

    fixed_u_vec[3] = true; // change a flag
    
    SetEditXSecFixedUVec( xsec_1, fixed_u_vec );

    ReparameterizeEditXSec( xsec_1 );
    \endcode
    \sa GetEditXSecFixedUVec, ReparameterizeEditXSec
    \param [in] xsec_id XSec ID
    \param [in] fixed_u_vec Array of fixed U flags
*/)";
    r = se->RegisterGlobalFunction( "void SetEditXSecFixedUVec( const string& in xsec_id, array<bool>@ fixed_u_vec )", asMETHOD( ScriptMgrSingleton, SetEditXSecFixedUVec ), asCALL_THISCALL_ASGLOBAL, &ScriptMgr, doc_struct );
    assert( r >= 0 );

    doc_struct.comment = R"(
/*!
    Perform an equal arc length repareterization on an EditCurveXSec. The reparameterization is performed between 
    specific U values if the Fixed U flag is true. This allows corners, such as at 0.25, 0.5, and 0.75 U, to be held
    constant while everything between them is reparameterized. 
    \code{.cpp}
    // Add Wing
    string wid = AddGeom( "WING" );

    // Get First (and Only) XSec Surf
    string xsec_surf = GetXSecSurf( wid, 0 );

    ChangeXSecShape( xsec_surf, 1, XS_EDIT_CURVE );

    // Identify XSec 1
    string xsec_1 = GetXSec( xsec_surf, 1 );

    array < bool > @ fixed_u_vec = GetEditXSecFixedUVec( xsec_1 );

    fixed_u_vec[3] = true; // change a flag
    
    SetEditXSecFixedUVec( xsec_1, fixed_u_vec );

    ReparameterizeEditXSec( xsec_1 );
    \endcode
    \sa SetEditXSecFixedUVec, GetEditXSecFixedUVec
    \param [in] xsec_id XSec ID
*/)";
    r = se->RegisterGlobalFunction( "void ReparameterizeEditXSec( const string& in xsec_id )", asFUNCTION( vsp::ReparameterizeEditXSec ), asCALL_CDECL, doc_struct );
    assert( r >= 0 );

    //==== BOR Functions ====//
    group = "BOR";
    doc_struct.group = group.c_str();

    group_description = R"(
    \brief This group of API functions provides capabilities related to the body of revolution (BOR) geometry type in OpenVSP. \n\n
    \ref index "Click here to return to the main page" )";
    se->AddGroup( group.c_str(), "BOR Functions", group_description.c_str() );

    doc_struct.comment = R"(
/*!
    Set the XSec type for a BOR component
    \code{.cpp}
    // Add Body of Recolution
    string bor_id = AddGeom( "BODYOFREVOLUTION", "" );

    ChangeBORXSecShape( bor_id, XS_ROUNDED_RECTANGLE );

    if ( GetBORXSecShape( bor_id ) != XS_ROUNDED_RECTANGLE ) { Print( "ERROR: ChangeBORXSecShape" ); }
    \endcode
    \sa XSEC_CRV_TYPE
    \param [in] geom_id Geom ID
    \param [in] type XSec type enum (i.e. XS_ROUNDED_RECTANGLE)
*/)";
    r = se->RegisterGlobalFunction( "void ChangeBORXSecShape( const string & in geom_id, int type )", asFUNCTION( vsp::ChangeBORXSecShape ), asCALL_CDECL, doc_struct );
    assert( r >= 0 );

    doc_struct.comment = R"(
/*!
    Get the XSec type for a BOR component
    \code{.cpp}
    // Add Body of Recolution
    string bor_id = AddGeom( "BODYOFREVOLUTION", "" );

    ChangeBORXSecShape( bor_id, XS_ROUNDED_RECTANGLE );

    if ( GetBORXSecShape( bor_id ) != XS_ROUNDED_RECTANGLE ) { Print( "ERROR: GetBORXSecShape" ); }
    \endcode
    \param [in] geom_id Geom ID
    \return XSec type enum (i.e. XS_ROUNDED_RECTANGLE)
*/)";
    r = se->RegisterGlobalFunction( "int GetBORXSecShape( const string & in geom_id )", asFUNCTION( vsp::GetBORXSecShape ), asCALL_CDECL, doc_struct );
    assert( r >= 0 );

    doc_struct.comment = R"(
/*!
    Set the coordinate points for a specific BOR. The BOR XSecCurve must be of type XS_FILE_FUSE.
    \code{.cpp}
    // Add Body of Recolution
    string bor_id = AddGeom( "BODYOFREVOLUTION", "" );

    ChangeBORXSecShape( bor_id, XS_FILE_FUSE );

    array< vec3d > @vec_array = ReadBORFileXSec( bor_id, "TestXSec.fxs" );
    \endcode
    \param [in] bor_id Geom ID of BOR
    \param [in] file_name Fuselage XSec file name
    \return Array of coordinate points read from the file and set to the XSec
*/)";
    r = se->RegisterGlobalFunction( "array<vec3d>@ ReadBORFileXSec(const string& in bor_id, const string& in file_name )", asMETHOD( ScriptMgrSingleton, ReadBORFileXSec ), asCALL_THISCALL_ASGLOBAL, &ScriptMgr, doc_struct );
    assert( r >= 0 );

    doc_struct.comment = R"(
/*!
    Set the coordinate points for a specific BOR. The BOR XSecCurve must be of type XS_FILE_FUSE.
    \code{.cpp}
    // Add Body of Recolution
    string bor_id = AddGeom( "BODYOFREVOLUTION", "" );

    ChangeBORXSecShape( bor_id, XS_FILE_FUSE );

    array< vec3d > @vec_array = ReadBORFileXSec( bor_id, "TestXSec.fxs" );

    if ( vec_array.size() > 0 )
    {
        vec_array[1] = vec_array[1] * 2.0;
        vec_array[3] = vec_array[3] * 2.0;

        SetBORXSecPnts( bor_id, vec_array );
    }
    \endcode
    \param [in] bor_id Geom ID of BOR
    \param [in] pnt_arr Array of XSec coordinate points
*/)";
    r = se->RegisterGlobalFunction( "void SetBORXSecPnts( const string& in bor_id, array<vec3d>@ pnt_arr )", asMETHOD( ScriptMgrSingleton, SetBORXSecPnts ), asCALL_THISCALL_ASGLOBAL, &ScriptMgr, doc_struct );
    assert( r >= 0 );

    doc_struct.comment = R"(
/*!
    Compute 3D coordinate for a point on a BOR XSecCurve given the parameter value (U) along the curve
    \code{.cpp}
    //==== Add Geom ====//
    // Add Body of Recolution
    string bor_id = AddGeom( "BODYOFREVOLUTION", "" );

    double u_fract = 0.25;

    vec3d pnt = ComputeBORXSecPnt( bor_id, u_fract );
    \endcode
    \param [in] bor_id Geom ID of BOR
    \param [in] fract Curve parameter value (range: 0 - 1)
    \return 3D coordinate point
*/)";
    r = se->RegisterGlobalFunction( "vec3d ComputeBORXSecPnt( const string& in bor_id, double fract )", asFUNCTION( vsp::ComputeBORXSecPnt ), asCALL_CDECL, doc_struct );
    assert( r >= 0 );

    doc_struct.comment = R"(
/*!
    Compute the tangent vector of a point on a BOR XSecCurve given the parameter value (U) along the curve
    \code{.cpp}
    // Add Body of Recolution
    string bor_id = AddGeom( "BODYOFREVOLUTION", "" );

    double u_fract = 0.25;

    vec3d tan = ComputeBORXSecTan( bor_id, u_fract );
    \endcode
    \param [in] bor_id Geom ID of BOR
    \param [in] fract Curve parameter value (range: 0 - 1)
    \return Tangent vector
*/)";
    r = se->RegisterGlobalFunction( "vec3d ComputeBORXSecTan( const string& in bor_id, double fract )", asFUNCTION( vsp::ComputeBORXSecTan ), asCALL_CDECL, doc_struct );
    assert( r >= 0 );

    doc_struct.comment = R"(
/*!
    Read in shape from airfoil file and set to the specified BOR XSecCurve. The XSecCurve must be of type XS_FILE_AIRFOIL. Airfoil files may be in Lednicer or Selig format with *.af or *.dat extensions.
    \code{.cpp}
    // Add Body of Recolution
    string bor_id = AddGeom( "BODYOFREVOLUTION", "" );

    ChangeBORXSecShape( bor_id, XS_FILE_AIRFOIL );

    ReadBORFileAirfoil( bor_id, "airfoil/N0012_VSP.af" );
    \endcode
    \param [in] bor_id Geom ID of BOR
    \param [in] file_name Airfoil XSec file name
*/)";
    r = se->RegisterGlobalFunction( "void ReadBORFileAirfoil( const string& in bor_id, const string& in file_name )", asFUNCTION( vsp::ReadBORFileAirfoil ), asCALL_CDECL, doc_struct );
    assert( r >= 0 );

    doc_struct.comment = R"(
/*!
    Set the upper points for an airfoil on a BOR. The BOR XSecCurve must be of type XS_FILE_AIRFOIL.
    \code{.cpp}
    // Add Body of Recolution
    string bor_id = AddGeom( "BODYOFREVOLUTION", "" );

    ChangeBORXSecShape( bor_id, XS_FILE_AIRFOIL );

    ReadBORFileAirfoil( bor_id, "airfoil/N0012_VSP.af" );

    array< vec3d > @up_array = GetBORAirfoilUpperPnts( bor_id );

    for ( int i = 0 ; i < int( up_array.size() ) ; i++ )
    {
        up_array[i].scale_y( 2.0 );
    }

    SetBORAirfoilUpperPnts( bor_id, up_array );
    \endcode
    \param [in] bor_id Geom ID of BOR
    \param [in] up_pnt_vec Array of points defining the upper surface of the airfoil
*/)";
    r = se->RegisterGlobalFunction( "void SetBORAirfoilUpperPnts( const string& in bor_id, array<vec3d>@ up_pnt_vec )", asMETHOD( ScriptMgrSingleton, SetBORAirfoilUpperPnts ), asCALL_THISCALL_ASGLOBAL, &ScriptMgr, doc_struct );
    assert( r >= 0 );


    doc_struct.comment = R"(
/*!
    Set the lower points for an airfoil on a BOR. The BOR XSecCurve must be of type XS_FILE_AIRFOIL.
    \code{.cpp}
    // Add Body of Recolution
    string bor_id = AddGeom( "BODYOFREVOLUTION", "" );

    ChangeBORXSecShape( bor_id, XS_FILE_AIRFOIL );

    ReadBORFileAirfoil( bor_id, "airfoil/N0012_VSP.af" );

    array< vec3d > @low_array = GetBORAirfoilLowerPnts( bor_id );

    for ( int i = 0 ; i < int( low_array.size() ) ; i++ )
    {
        low_array[i].scale_y( 0.5 );
    }

    SetBORAirfoilLowerPnts( bor_id, low_array );
    \endcode
    \param [in] bor_id Geom ID of BOR
    \param [in] low_pnt_vec Array of points defining the lower surface of the airfoil
*/)";
    r = se->RegisterGlobalFunction( "void SetBORAirfoilLowerPnts( const string& in bor_id, array<vec3d>@ low_pnt_vec )", asMETHOD( ScriptMgrSingleton, SetBORAirfoilLowerPnts ), asCALL_THISCALL_ASGLOBAL, &ScriptMgr, doc_struct );
    assert( r >= 0 );

    doc_struct.comment = R"(
/*!
    Set the upper and lower points for an airfoil on a BOR. The BOR XSecCurve must be of type XS_FILE_AIRFOIL.
    \code{.cpp}
    // Add Body of Recolution
    string bor_id = AddGeom( "BODYOFREVOLUTION", "" );

    ChangeBORXSecShape( bor_id, XS_FILE_AIRFOIL );

    ReadBORFileAirfoil( bor_id, "airfoil/N0012_VSP.af" );

    array< vec3d > @up_array = GetBORAirfoilUpperPnts( bor_id );

    array< vec3d > @low_array = GetBORAirfoilLowerPnts( bor_id );

    for ( int i = 0 ; i < int( up_array.size() ) ; i++ )
    {
        up_array[i].scale_y( 2.0 );

        low_array[i].scale_y( 0.5 );
    }

    SetBORAirfoilPnts( bor_id, up_array, low_array );
    \endcode
    \param [in] bor_id Geom ID of BOR
    \param [in] up_pnt_vec Array of points defining the upper surface of the airfoil
    \param [in] low_pnt_vec Array of points defining the lower surface of the airfoil
*/)";
    r = se->RegisterGlobalFunction( "void SetBORAirfoilPnts( const string& in bor_id, array<vec3d>@ up_pnt_vec, array<vec3d>@ low_pnt_vec )", asMETHOD( ScriptMgrSingleton, SetBORAirfoilPnts ), asCALL_THISCALL_ASGLOBAL, &ScriptMgr, doc_struct );
    assert( r >= 0 );

    doc_struct.comment = R"(
/*!
    Get the coordinate points for the upper surface of an airfoil on a BOR. The BOR XSecCurve must be of type XS_FILE_AIRFOIL
    \code{.cpp}
    // Add Body of Recolution
    string bor_id = AddGeom( "BODYOFREVOLUTION", "" );

    ChangeBORXSecShape( bor_id, XS_FILE_AIRFOIL );

    ReadBORFileAirfoil( bor_id, "airfoil/N0012_VSP.af" );

    array< vec3d > @up_array = GetBORAirfoilUpperPnts( bor_id );
    \endcode
    \sa SetAirfoilPnts
    \param [in] bor_id Geom ID of BOR
    \return Array of coordinate points for the upper airfoil surface
*/)";
    r = se->RegisterGlobalFunction( "array<vec3d>@ GetBORAirfoilUpperPnts(const string& in bor_id )", asMETHOD( ScriptMgrSingleton, GetBORAirfoilUpperPnts ), asCALL_THISCALL_ASGLOBAL, &ScriptMgr, doc_struct );
    assert( r >= 0 );

    doc_struct.comment = R"(
/*!
    Get the coordinate points for the lower surface of an airfoil of a BOR. The XSecCurve must be of type XS_FILE_AIRFOIL
    \code{.cpp}
    // Add Body of Recolution
    string bor_id = AddGeom( "BODYOFREVOLUTION", "" );

    ChangeBORXSecShape( bor_id, XS_FILE_AIRFOIL );

    ReadBORFileAirfoil( bor_id, "airfoil/N0012_VSP.af" );

    array< vec3d > @low_array = GetBORAirfoilLowerPnts( bor_id );
    \endcode
    \sa SetAirfoilPnts
    \param [in] bor_id Geom ID of BOR
    \return Array of coordinate points for the lower airfoil surface
*/)";
    r = se->RegisterGlobalFunction( "array<vec3d>@ GetBORAirfoilLowerPnts(const string& in bor_id )", asMETHOD( ScriptMgrSingleton, GetBORAirfoilLowerPnts ), asCALL_THISCALL_ASGLOBAL, &ScriptMgr, doc_struct );
    assert( r >= 0 );

    doc_struct.comment = R"(
/*!
    Get the CST coefficients for the upper surface of an airfoil of a BOR. The XSecCurve must be of type XS_CST_AIRFOIL
    \sa SetUpperCST
    \param [in] bor_id Geom ID of BOR
    \return Array of CST coefficients for the upper airfoil surface
*/)";
    r = se->RegisterGlobalFunction( "array<double>@ GetBORUpperCSTCoefs( const string & in bor_id )", asMETHOD( ScriptMgrSingleton, GetBORUpperCSTCoefs ), asCALL_THISCALL_ASGLOBAL, &ScriptMgr, doc_struct );
    assert( r >= 0 ); // TODO: Example

    doc_struct.comment = R"(
/*!
    Get the CST coefficients for the lower surface of an airfoil of a BOR. The XSecCurve must be of type XS_CST_AIRFOIL
    \sa SetLowerCST
    \param [in] bor_id Geom ID of BOR
    \return Array of CST coefficients for the lower airfoil surface
*/)";
    r = se->RegisterGlobalFunction( "array<double>@ GetBORLowerCSTCoefs( const string & in bor_id )", asMETHOD( ScriptMgrSingleton, GetBORLowerCSTCoefs ), asCALL_THISCALL_ASGLOBAL, &ScriptMgr, doc_struct );
    assert( r >= 0 );  // TODO: Example

    doc_struct.comment = R"(
/*!
    Get the CST degree for the upper surface of an airfoil of a BOR. The XSecCurve must be of type XS_CST_AIRFOIL
    \sa SetUpperCST
    \param [in] bor_id Geom ID of BOR
    \return CST Degree for upper airfoil surface
*/)";
    r = se->RegisterGlobalFunction( "int GetBORUpperCSTDegree( const string& in bor_id )", asFUNCTION( vsp::GetBORUpperCSTDegree ), asCALL_CDECL, doc_struct );
    assert( r >= 0 );  // TODO: Example

    doc_struct.comment = R"(
/*!
    Get the CST degree for the lower surface of an airfoil of a BOR. The XSecCurve must be of type XS_CST_AIRFOIL
    \sa SetLowerCST
    \param [in] bor_id Geom ID of BOR
    \return CST Degree for lower airfoil surface
*/)";
    r = se->RegisterGlobalFunction( "int GetBORLowerCSTDegree( const string& in bor_id )", asFUNCTION( vsp::GetBORLowerCSTDegree ), asCALL_CDECL, doc_struct );
    assert( r >= 0 );  // TODO: Example

    doc_struct.comment = R"(
/*!
    Set the CST degree and coefficients for the upper surface of an airfoil of a BOR. The number of coefficients should be one more than the CST degree. The XSecCurve must be of type XS_CST_AIRFOIL
    \sa GetUpperCSTDegree, GetUpperCSTCoefs
    \param [in] bor_id Geom ID of BOR
    \param [in] deg CST degree of upper airfoil surface
    \param [in] coeff_arr Array of CST coefficients for the upper airfoil surface
*/)";
    r = se->RegisterGlobalFunction( "void SetBORUpperCST( const string& in bor_id, int deg, array<double>@ coeff_arr )", asMETHOD( ScriptMgrSingleton, SetBORUpperCST ), asCALL_THISCALL_ASGLOBAL, &ScriptMgr, doc_struct );
    assert( r >= 0 );  // TODO: Example

    doc_struct.comment = R"(
/*!
    Set the CST degree and coefficients for the lower surface of an airfoil of a BOR. The number of coefficients should be one more than the CST degree. The XSecCurve must be of type XS_CST_AIRFOIL
    \sa GetLowerCSTDegree, GetLowerCSTCoefs
    \param [in] bor_id Geom ID of BOR
    \param [in] deg CST degree of lower airfoil surface
    \param [in] coeff_arr Array of CST coefficients for the lower airfoil surface
*/)";
    r = se->RegisterGlobalFunction( "void SetBORLowerCST( const string& in bor_id, int deg, array<double>@ coeff_arr )", asMETHOD( ScriptMgrSingleton, SetBORLowerCST ), asCALL_THISCALL_ASGLOBAL, &ScriptMgr, doc_struct );
    assert( r >= 0 );  // TODO: Example

    doc_struct.comment = R"(
/*!
    Promote the CST for the upper airfoil surface of a BOR. The XSecCurve must be of type XS_CST_AIRFOIL
    \sa GetUpperCSTDegree
    \param [in] bor_id Geom ID of BOR
*/)";
    r = se->RegisterGlobalFunction( "void PromoteBORCSTUpper( const string& in bor_id )", asFUNCTION( vsp::PromoteBORCSTUpper ), asCALL_CDECL, doc_struct );
    assert( r >= 0 );  // TODO: Example

    doc_struct.comment = R"(
/*!
    Promote the CST for the lower airfoil surface of a BOR. The XSecCurve must be of type XS_CST_AIRFOIL
    \sa GetLowerCSTDegree
    \param [in] bor_id Geom ID of BOR
*/)";
    r = se->RegisterGlobalFunction( "void PromoteBORCSTLower( const string& in bor_id )", asFUNCTION( vsp::PromoteBORCSTLower ), asCALL_CDECL, doc_struct );
    assert( r >= 0 );  // TODO: Example

    doc_struct.comment = R"(
/*!
    Demote the CST for the upper airfoil surface of a BOR. The XSecCurve must be of type XS_CST_AIRFOIL
    \sa GetUpperCSTDegree
    \param [in] bor_id Geom ID of BOR
*/)";
    r = se->RegisterGlobalFunction( "void DemoteBORCSTUpper( const string& in bor_id )", asFUNCTION( vsp::DemoteBORCSTUpper ), asCALL_CDECL, doc_struct );
    assert( r >= 0 );  // TODO: Example

    doc_struct.comment = R"(
/*!
    Demote the CST for the lower airfoil surface of a BOR. The XSecCurve must be of type XS_CST_AIRFOIL
    \sa GetLowerCSTDegree
    \param [in] bor_id Geom ID of BOR
*/)";
    r = se->RegisterGlobalFunction( "void DemoteBORCSTLower( const string& in bor_id )", asFUNCTION( vsp::DemoteBORCSTLower ), asCALL_CDECL, doc_struct );
    assert( r >= 0 );  // TODO: Example

    doc_struct.comment = R"(
/*!
    Fit a CST airfoil for an existing airfoil of a BOR of type XS_FOUR_SERIES, XS_SIX_SERIES, XS_FOUR_DIGIT_MOD, XS_FIVE_DIGIT, XS_FIVE_DIGIT_MOD, XS_ONE_SIX_SERIES, or XS_FILE_AIRFOIL.
    \param [in] bor_id Geom ID of BOR
    \param [in] deg CST degree
*/)";
    r = se->RegisterGlobalFunction( "void FitBORAfCST( const string& in bor_id, int deg )", asFUNCTION( vsp::FitBORAfCST ), asCALL_CDECL, doc_struct );
    assert( r >= 0 );  // TODO: Example

    //==== Sets Functions ====//
    group = "Sets";
    doc_struct.group = group.c_str();

    group_description = R"(
    \brief The following group of API functions deals with set manipulation. \n\n
    \ref index "Click here to return to the main page" )";
    se->AddGroup( group.c_str(), "Functions for Sets", group_description.c_str() );

    doc_struct.comment = R"(
/*!
    Get the total number of defined sets. Named sets are used to group components and read/write on them. The number of named
    sets will be 10 for OpenVSP versions up to 3.17.1 and 20 for later versions. 
    \code{.cpp}
    if ( GetNumSets() <= 0 )                            { Print( "---> Error: API GetNumSets " ); }
    \endcode
    \return Number of sets
*/)";
    r = se->RegisterGlobalFunction( "int GetNumSets()", asFUNCTION( vsp::GetNumSets ), asCALL_CDECL, doc_struct );
    assert( r >= 0 );

    doc_struct.comment = R"(
/*!
    Set the name of a set at specified index
    \code{.cpp}
    SetSetName( 3, "SetFromScript" );

    if ( GetSetName( 3 ) != "SetFromScript" )            { Print( "---> Error: API Get/Set Set Name " ); }
    \endcode
    \sa SET_TYPE
    \param [in] index Set index
    \param [in] name Set name
*/)";
    r = se->RegisterGlobalFunction( "void SetSetName( int index, const string& in name )", asFUNCTION( vsp::SetSetName ), asCALL_CDECL, doc_struct );
    assert( r >= 0 );

    doc_struct.comment = R"(
/*!
    Get the name of a set at specified index
    \code{.cpp}
    SetSetName( 3, "SetFromScript" );

    if ( GetSetName( 3 ) != "SetFromScript" )            { Print( "---> Error: API Get/Set Set Name " ); }
    \endcode
    \sa SET_TYPE
    \param [in] index Set index
    \return Set name
*/)";
    r = se->RegisterGlobalFunction( "string GetSetName( int index )", asFUNCTION( vsp::GetSetName ), asCALL_CDECL, doc_struct );
    assert( r >= 0 );

    doc_struct.comment = R"(
/*!
    Get an array of Geom IDs for the specified set index
    \code{.cpp}
    SetSetName( 3, "SetFromScript" );

    array<string> @geom_arr1 = GetGeomSetAtIndex( 3 );

    array<string> @geom_arr2 = GetGeomSet( "SetFromScript" );

    if ( geom_arr1.size() != geom_arr2.size() )            { Print( "---> Error: API GetGeomSet " ); }
    \endcode
    \sa SET_TYPE
    \param [in] index Set index
    \return Array of Geom IDs
*/)";
    r = se->RegisterGlobalFunction( "array<string>@ GetGeomSetAtIndex( int index )", asMETHOD( ScriptMgrSingleton, GetGeomSetAtIndex ), asCALL_THISCALL_ASGLOBAL, &ScriptMgr, doc_struct );
    assert( r >= 0 );

    doc_struct.comment = R"(
/*!
    Get an array of Geom IDs for the specified set name
    \code{.cpp}
    SetSetName( 3, "SetFromScript" );

    array<string> @geom_arr1 = GetGeomSetAtIndex( 3 );

    array<string> @geom_arr2 = GetGeomSet( "SetFromScript" );

    if ( geom_arr1.size() != geom_arr2.size() )            { Print( "---> Error: API GetGeomSet " ); }
    \endcode
    \param [in] name const string set name
    \return array<string> array of Geom IDs
*/)";
    r = se->RegisterGlobalFunction( "array<string>@ GetGeomSet( const string & in name )", asMETHOD( ScriptMgrSingleton, GetGeomSet ), asCALL_THISCALL_ASGLOBAL, &ScriptMgr, doc_struct );
    assert( r >= 0 );

    doc_struct.comment = R"(
/*!
    Get the set index for the specified set name
    \code{.cpp}
    SetSetName( 3, "SetFromScript" );

    if ( GetSetIndex( "SetFromScript" ) != 3 ) { Print( "ERROR: GetSetIndex" ); }
    \endcode
    \param [in] name Set name
    \return Set index
*/)";
    r = se->RegisterGlobalFunction( "int GetSetIndex( const string & in name )", asFUNCTION( vsp::GetSetIndex ), asCALL_CDECL, doc_struct );
    assert( r >= 0 );

    doc_struct.comment = R"(
/*!
    Check if a Geom is in the set at the specified set index
    \code{.cpp}
    // Add Fuselage Geom
    string fuseid = AddGeom( "FUSELAGE", "" );

    SetSetFlag( fuseid, 3, true );

    if ( !GetSetFlag( fuseid, 3 ) )                        { Print( "---> Error: API Set/Get Set Flag " ); }
    \endcode
    \param [in] geom_id Geom ID
    \param [in] set_index Set index
    \return True if geom is in the set, false otherwise
*/)";
    r = se->RegisterGlobalFunction( "bool GetSetFlag( const string & in geom_id, int set_index )", asFUNCTION( vsp::GetSetFlag ), asCALL_CDECL, doc_struct );
    assert( r >= 0 );

    doc_struct.comment = R"(
/*!
    Set whether or not a Geom is a member of the set at specified set index
    \code{.cpp}
    // Add Fuselage Geom
    string fuseid = AddGeom( "FUSELAGE", "" );

    SetSetFlag( fuseid, 3, true );

    if ( !GetSetFlag( fuseid, 3 ) )                        { Print( "---> Error: API Set/Get Set Flag " ); }
    \endcode
    \param [in] geom_id Geom ID
    \param [in] set_index Set index
    \param [in] flag Flag that indicates set membership
*/)";
    r = se->RegisterGlobalFunction( "void SetSetFlag( const string & in geom_id, int set_index, bool flag )", asFUNCTION( vsp::SetSetFlag ), asCALL_CDECL, doc_struct );
    assert( r >= 0 );

    doc_struct.comment = R"(
/*!
    Copies all the states of a geom set and pastes them into a specific set based on passed in indexs 
    \code{.cpp}
    // Add Fuselage Geom
    string fuseid = AddGeom( "FUSELAGE", "" );

    //set fuseid's state for set 3 to true
    SetSetFlag( fuseid, 3, true );

    //Copy set 3 and Paste into set 4
    CopyPasteSet( 3, 4 );

    //get fuseid's state for set 4
    bool flag_value = GetSetFlag( fuseid, 4 );

    if ( flag_value != true)                      { Print( "---> Error: API CopyPasteSet " ); }
    \endcode
    \param [in] copyIndex Copy Index
    \param [in] pasteIndex Paste Index
*/)";
    r = se->RegisterGlobalFunction( "void CopyPasteSet(  int copyIndex, int pasteIndex  )", asFUNCTION( vsp::CopyPasteSet ), asCALL_CDECL, doc_struct );
    assert( r >= 0 );

    //=== Group Modifications ===//
    group = "GroupMod";
    doc_struct.group = group.c_str();

    group_description = R"(
    \brief The functions in this group allow for sets to be scaled, rotated, and translated. \n\n
    \ref index "Click here to return to the main page" )";
    se->AddGroup( group.c_str(), "Group Modification Functions", group_description.c_str() );

    doc_struct.comment = R"(
/*!
    Apply a scale factor to a set
    \code{.cpp}
    // Add Fuselage Geom
    string fuseid = AddGeom( "FUSELAGE" );

    SetSetFlag( fuseid, 3, true );

    // Scale by a factor of 2
    ScaleSet( 3, 2.0 );
    \endcode
    \param [in] set_index Set index
    \param [in] scale Scale factor
*/)";
    r = se->RegisterGlobalFunction( "void ScaleSet( int set_index, double scale )", asFUNCTION( vsp::ScaleSet ), asCALL_CDECL, doc_struct );
    assert( r >= 0 );

    doc_struct.comment = R"(
/*!
    Rotate a set about the global X, Y, and Z axes
    \code{.cpp}
    // Add Fuselage Geom
    string fuseid = AddGeom( "FUSELAGE" );

    SetSetFlag( fuseid, 3, true );

    // Rotate 90 degrees about Y
    RotateSet( 3, 0, 90, 0 );
    \endcode
    \param [in] set_index Set index
    \param [in] x_rot_deg Rotation about the X axis (degrees)
    \param [in] y_rot_deg Rotation about the Y axis (degrees)
    \param [in] z_rot_deg Rotation about the Z axis (degrees)
*/)";
    r = se->RegisterGlobalFunction( "void RotateSet( int set_index, double x_rot_deg, double y_rot_deg, double z_rot_deg )", asFUNCTION( vsp::RotateSet ), asCALL_CDECL, doc_struct );
    assert( r >= 0 );

    doc_struct.comment = R"(
/*!
    Translate a set along a given vector
    \code{.cpp}
    // Add Fuselage Geom
    string fuseid = AddGeom( "FUSELAGE" );

    SetSetFlag( fuseid, 3, true );

    // Translate 2 units in X and 3 units in Y
    TranslateSet( 3, vec3d( 2, 3, 0 ) );
    \endcode
    \param [in] set_index Set index
    \param [in] translation_vec Translation vector
*/)";
    r = se->RegisterGlobalFunction( "void TranslateSet( int set_index, const vec3d & in translation_vec )", asFUNCTION( vsp::TranslateSet ), asCALL_CDECL, doc_struct );
    assert( r >= 0 );

    doc_struct.comment = R"(
/*!
    Apply translation, rotation, and scale transformations to a set
    \code{.cpp}
    // Add Fuselage Geom
    string fuseid = AddGeom( "FUSELAGE" );

    SetSetFlag( fuseid, 3, true );

    // Translate 2 units in X and 3 units in Y, rotate 90 degrees about Y, and scale by a factor of 2
    TransformSet( 3, vec3d( 2, 3, 0 ), 0, 90, 0, 2.0, true );
    \endcode
    \sa TranslateSet, RotateSet, ScaleSet
    \param [in] set_index Set index
    \param [in] translation_vec Translation vector
    \param [in] x_rot_deg Rotation about the X axis (degrees)
    \param [in] y_rot_deg Rotation about the Y axis (degrees)
    \param [in] z_rot_deg Rotation about the Z axis (degrees)
    \param [in] scale Scale factor
    \param [in] scale_translations_flag Flag to apply the scale factor to translations
*/)";
    r = se->RegisterGlobalFunction( "void TransformSet( int set_index, const vec3d & in translation_vec, double x_rot_deg, double y_rot_deg, double z_rot_deg, double scale, bool scale_translations_flag )", asFUNCTION( vsp::TransformSet ), asCALL_CDECL, doc_struct );
    assert( r >= 0 );

    //==== Parm Functions ====//
    group = "Parm";
    doc_struct.group = group.c_str();

    group_description = R"(
    \brief Every Parm in OpenVSP can be accessed and modified through the functions defined in this API group.
    Every Parm has an associated ParmContainer. \n\n
    \ref index "Click here to return to the main page" )";
    se->AddGroup( group.c_str(), "Parm Functions", group_description.c_str() );

    doc_struct.comment = R"(
/*!
    Check if given Parm is valid
    \code{.cpp}
    //==== Add Pod Geometry ====//
    string pid = AddGeom( "POD" );

    string lenid = GetParm( pid, "Length", "Design" );

    if ( !ValidParm( lenid ) )                { Print( "---> Error: API GetParm  " ); }
    \endcode
    \param [in] id Parm ID
    \return True if Parm ID is valid, false otherwise
*/)";
    r = se->RegisterGlobalFunction( "bool ValidParm( const string & in id )", asFUNCTION( vsp::ValidParm ), asCALL_CDECL, doc_struct );
    assert( r >= 0 );

    doc_struct.comment = R"(
/*!
    Set the value of the specified Parm.
    \code{.cpp}
    // Add Fuselage Geom
    string fuseid = AddGeom( "FUSELAGE", "" );

    string xsec_surf = GetXSecSurf( fuseid, 0 );

    ChangeXSecShape( xsec_surf, GetNumXSec( xsec_surf ) - 1, XS_ROUNDED_RECTANGLE );

    string xsec = GetXSec( xsec_surf, GetNumXSec( xsec_surf ) - 1 );

    string wid = GetXSecParm( xsec, "RoundedRect_Width" );

    SetParmVal( wid, 23.0 );

    if ( abs( GetParmVal( wid ) - 23 ) > 1e-6 )                { Print( "---> Error: API Parm Val Set/Get " ); }
    \endcode
    \sa SetParmValUpdate
    \param [in] parm_id Parm ID
    \param [in] val Parm value to set
    \return Value that the Parm was set to
*/)";
    r = se->RegisterGlobalFunction( "double SetParmVal(const string & in parm_id, double val )",
                                    asFUNCTIONPR( vsp::SetParmVal, ( const string &, double val ), double ), asCALL_CDECL, doc_struct );
    assert( r >= 0 );

    doc_struct.comment = R"(
/*!
    Set the value along with the upper and lower limits of the specified Parm
    \code{.cpp}
    string pod_id = AddGeom( "POD" );

    string length = FindParm( pod_id, "Length", "Design" );

    SetParmValLimits( length, 10.0, 0.001, 1.0e12 );

    SetParmDescript( length, "Total Length of Geom" );
    \endcode
    \sa SetParmLowerLimit, SetParmUpperLimit
    \param [in] parm_id Parm ID
    \param [in] val Parm value to set
    \param [in] lower_limit Parm lower limit
    \param [in] upper_limit Parm upper limit
    \return Value that the Parm was set to
*/)";
    r = se->RegisterGlobalFunction( "double SetParmValLimits(const string & in parm_id, double val, double lower_limit, double upper_limit )",
                                    asFUNCTION( vsp::SetParmValLimits ), asCALL_CDECL, doc_struct );
    assert( r >= 0 );

    doc_struct.comment = R"(
/*!
    Set the value of the specified Parm and force an Update.
    \code{.cpp}
    //==== Add Pod Geometry ====//
    string pod_id = AddGeom( "POD" );

    string parm_id = GetParm( pod_id, "X_Rel_Location", "XForm" );

    SetParmValUpdate( parm_id, 5.0 );
    \endcode
    \sa SetParmVal
    \param [in] parm_id Parm ID
    \param [in] val Parm value to set
    \return Value that the Parm was set to
*/)";
    r = se->RegisterGlobalFunction( "double SetParmValUpdate(const string & in parm_id, double val )",
                                    asFUNCTIONPR( vsp::SetParmValUpdate, ( const string &, double val ), double ), asCALL_CDECL, doc_struct );
    assert( r >= 0 );

    doc_struct.comment = R"(
/*!
    Set the value of the specified Parm. This function includes a call to GetParm to identify the Parm ID given the Geom ID, Parm name, and Parm group.
    \code{.cpp}
    string wing_id = AddGeom( "WING" );

    SetParmVal( wing_id, "ThickChord", "XSecCurve_0", 0.2 );
    \endcode
    \sa SetParmValUpdate
    \param [in] geom_id Geom ID
    \param [in] name Parm name
    \param [in] group Parm group name
    \param [in] val Parm value to set
    \return Value that the Parm was set to
*/)";
    r = se->RegisterGlobalFunction( "double SetParmVal(const string & in geom_id, const string & in name, const string & in group, double val )",
                                    asFUNCTIONPR( vsp::SetParmVal, ( const string &, const string &, const string &, double val ), double ), asCALL_CDECL, doc_struct );
    assert( r >= 0 );

    doc_struct.comment = R"(
/*!
    Set the value of the specified Parm and force an Update. This function includes a call to GetParm to identify the Parm ID given the Geom ID, Parm name, and Parm group.
    \code{.cpp}
    //==== Add Pod Geometry ====//
    string pod_id = AddGeom( "POD" );

    SetParmValUpdate( pod_id, "X_Rel_Location", "XForm", 5.0 );
    \endcode
    \sa SetParmVal
    \param [in] geom_id Geom ID
    \param [in] parm_name Parm name
    \param [in] parm_group_name Parm group name
    \param [in] val Parm value to set
    \return Value that the Parm was set to
*/)";
    r = se->RegisterGlobalFunction( "double SetParmValUpdate(const string & in geom_id, const string & in parm_name, const string & in parm_group_name, double val )",
                                    asFUNCTIONPR( vsp::SetParmValUpdate, ( const string &, const string &, const string &, double val ), double ), asCALL_CDECL, doc_struct );
    assert( r >= 0 );

    doc_struct.comment = R"(
/*!
    Get the value of the specified Parm. The data type of the Parm value will be cast to a double
    \code{.cpp}
    // Add Fuselage Geom
    string fuseid = AddGeom( "FUSELAGE", "" );

    string xsec_surf = GetXSecSurf( fuseid, 0 );

    ChangeXSecShape( xsec_surf, GetNumXSec( xsec_surf ) - 1, XS_ROUNDED_RECTANGLE );

    string xsec = GetXSec( xsec_surf, GetNumXSec( xsec_surf ) - 1 );

    string wid = GetXSecParm( xsec, "RoundedRect_Width" );

    SetParmVal( wid, 23.0 );

    if ( abs( GetParmVal( wid ) - 23 ) > 1e-6 )                { Print( "---> Error: API Parm Val Set/Get " ); }
    \endcode
    \param [in] parm_id Parm ID
    \return Parm value
*/)";
    r = se->RegisterGlobalFunction( "double GetParmVal(const string & in parm_id )", asFUNCTIONPR( vsp::GetParmVal, ( const string & ), double ), asCALL_CDECL, doc_struct );
    assert( r >= 0 );

    doc_struct.comment = R"(
/*!
    Get the value of the specified double type Parm. This function includes a call to GetParm to identify the Parm ID given the Geom ID, Parm name, and Parm group.
    The data type of the Parm value will be cast to a double.
    \code{.cpp}
    //==== Add Pod Geometry ====//
    string pod_id = AddGeom( "POD" );

    double length = GetParmVal( pod_id, "Length", "Design" );
    \endcode
    \param [in] geom_id Geom ID
    \param [in] name Parm name
    \param [in] group Parm group name
    \return Parm value
*/)";
    r = se->RegisterGlobalFunction( "double GetParmVal(const string & in geom_id, const string & in name, const string & in group )",
                                    asFUNCTIONPR( vsp::GetParmVal, ( const string &, const string &, const string & ), double ), asCALL_CDECL, doc_struct );
    assert( r >= 0 );

    doc_struct.comment = R"(
/*!
    Get the value of the specified int type Parm
    \code{.cpp}
    //==== Add Prop Geometry ====//
    string prop_id = AddGeom( "PROP" );

    string num_blade_id = GetParm( prop_id, "NumBlade", "Design" );

    int num_blade = GetIntParmVal( num_blade_id );
    \endcode
    \param [in] parm_id Parm ID
    \return Parm value
*/)";
    r = se->RegisterGlobalFunction( "int GetIntParmVal(const string & in parm_id )", asFUNCTION( vsp::GetIntParmVal ), asCALL_CDECL, doc_struct );
    assert( r >= 0 );

    doc_struct.comment = R"(
/*!
    Get the value of the specified bool type Parm
    \code{.cpp}
    //==== Add Prop Geometry ====//
    string prop_id = AddGeom( "PROP" );

    string rev_flag_id = GetParm( prop_id, "ReverseFlag", "Design" );

    bool reverse_flag = GetBoolParmVal( rev_flag_id );
    \endcode
    \param [in] parm_id Parm ID
    \return Parm value
*/)";
    r = se->RegisterGlobalFunction( "bool GetBoolParmVal(const string & in parm_id )", asFUNCTION( vsp::GetBoolParmVal ), asCALL_CDECL, doc_struct );
    assert( r >= 0 );

    doc_struct.comment = R"(
/*!
    Set the upper limit value for the specified Parm
    \code{.cpp}
    // Add Fuselage Geom
    string fuseid = AddGeom( "FUSELAGE", "" );

    string xsec_surf = GetXSecSurf( fuseid, 0 );

    ChangeXSecShape( xsec_surf, GetNumXSec( xsec_surf ) - 1, XS_ROUNDED_RECTANGLE );

    string xsec = GetXSec( xsec_surf, GetNumXSec( xsec_surf ) - 1 );

    string wid = GetXSecParm( xsec, "RoundedRect_Width" );

    SetParmVal( wid, 23.0 );

    SetParmUpperLimit( wid, 13.0 );

    if ( abs( GetParmVal( wid ) - 13 ) > 1e-6 )                { Print( "---> Error: API SetParmUpperLimit " ); }
    \endcode
    \sa SetParmValLimits
    \param [in] parm_id Parm ID
    \param [in] val Parm upper limit
*/)";
    r = se->RegisterGlobalFunction( "void SetParmUpperLimit( const string & in parm_id, double val )", asFUNCTION( vsp::SetParmUpperLimit ), asCALL_CDECL, doc_struct );
    assert( r >= 0 );

    doc_struct.comment = R"(
/*!
    Get the upper limit value for the specified Parm
    \code{.cpp}
    //==== Add Prop Geometry ====//
    string prop_id = AddGeom( "PROP" );

    string num_blade_id = GetParm( prop_id, "NumBlade", "Design" );

    double max_blade = GetParmUpperLimit( num_blade_id );
    \endcode
    \param [in] parm_id Parm ID
    \return Parm upper limit
*/)";
    r = se->RegisterGlobalFunction( "double GetParmUpperLimit( const string & in parm_id )", asFUNCTION( vsp::GetParmUpperLimit ), asCALL_CDECL, doc_struct );
    assert( r >= 0 );

    doc_struct.comment = R"(
/*!
    Set the lower limit value for the specified Parm
    \code{.cpp}
    // Add Fuselage Geom
    string fuseid = AddGeom( "FUSELAGE", "" );

    string xsec_surf = GetXSecSurf( fuseid, 0 );

    ChangeXSecShape( xsec_surf, GetNumXSec( xsec_surf ) - 1, XS_ROUNDED_RECTANGLE );

    string xsec = GetXSec( xsec_surf, GetNumXSec( xsec_surf ) - 1 );

    string wid = GetXSecParm( xsec, "RoundedRect_Width" );

    SetParmVal( wid, 13.0 );

    SetParmLowerLimit( wid, 15.0 );

    if ( abs( GetParmVal( wid ) - 15 ) > 1e-6 )                { Print( "---> Error: API SetParmLowerLimit " ); }
    \endcode
    \sa SetParmValLimits
    \param [in] parm_id Parm ID
    \param [in] val Parm lower limit
*/)";
    r = se->RegisterGlobalFunction( "void SetParmLowerLimit( const string & in parm_id, double val )", asFUNCTION( vsp::SetParmLowerLimit ), asCALL_CDECL, doc_struct );
    assert( r >= 0 );

    doc_struct.comment = R"(
/*!
    Get the lower limit value for the specified Parm
    \code{.cpp}
    //==== Add Prop Geometry ====//
    string prop_id = AddGeom( "PROP" );

    string num_blade_id = GetParm( prop_id, "NumBlade", "Design" );

    double min_blade = GetParmLowerLimit( num_blade_id );
    \endcode
    \param [in] parm_id Parm ID
    \return Parm lower limit
*/)";
    r = se->RegisterGlobalFunction( "double GetParmLowerLimit( const string & in parm_id )", asFUNCTION( vsp::GetParmLowerLimit ), asCALL_CDECL, doc_struct );
    assert( r >= 0 );

    doc_struct.comment = R"(
/*!
    Get the data type for the specified Parm
    \code{.cpp}
    // Add Fuselage Geom
    string fuseid = AddGeom( "FUSELAGE", "" );

    string xsec_surf = GetXSecSurf( fuseid, 0 );

    ChangeXSecShape( xsec_surf, GetNumXSec( xsec_surf ) - 1, XS_ROUNDED_RECTANGLE );

    string xsec = GetXSec( xsec_surf, GetNumXSec( xsec_surf ) - 1 );

    string wid = GetXSecParm( xsec, "RoundedRect_Width" );

    if ( GetParmType( wid ) != PARM_DOUBLE_TYPE )        { Print( "---> Error: API GetParmType " ); }
    \endcode
    \sa PARM_TYPE
    \param [in] parm_id Parm ID
    \return Parm data type enum (i.e. PARM_BOOL_TYPE)
*/)";
    r = se->RegisterGlobalFunction( "int GetParmType( const string & in parm_id )", asFUNCTION( vsp::GetParmType ), asCALL_CDECL, doc_struct );
    assert( r >= 0 );

    doc_struct.comment = R"(
/*!
    Get the name for the specified Parm
    \code{.cpp}
    //==== Add Pod Geometry ====//
    string pod_id = AddGeom( "POD" );

    //==== Add FeaStructure to Pod ====//
    int struct_ind = AddFeaStruct( pod_id );

    //==== Get Structure Name and Parm Container ID ====// 
    string parm_container_name = GetFeaStructName( pod_id, struct_ind );

    string parm_container_id = FindContainer( parm_container_name, struct_ind );

    //==== Get and List All Parms in the Container ====// 
    array<string> parm_ids = FindContainerParmIDs( parm_container_id );
    
    for ( uint i = 0; i < uint(parm_ids.length()); i++ )
    {
        string name_id = GetParmName( parm_ids[i] ) + string(": ") + parm_ids[i] + string("\n");

        Print( name_id );
    }
    \endcode
    \param [in] parm_id Parm ID
    \return Parm name
*/)";
    r = se->RegisterGlobalFunction( "string GetParmName( const string & in parm_id )", asFUNCTION( vsp::GetParmName ), asCALL_CDECL, doc_struct );
    assert( r >= 0 );

    doc_struct.comment = R"(
/*!
    Get the group name for the specified Parm
    \code{.cpp}
    string veh_id = FindContainer( "Vehicle", 0 );

    //==== Get and List All Parms in the Container ====// 
    array<string> parm_ids = FindContainerParmIDs( veh_id );

    Print( "Parm Groups and IDs in Vehicle Parm Container: " );

    for ( uint i = 0; i < uint(parm_ids.length()); i++ )
    {
        string group_str = GetParmGroupName( parm_ids[i] ) + string(": ") + parm_ids[i] + string("\n");

        Print( group_str );
    }
    \endcode
    \param [in] parm_id Parm ID
    \return Parm group name
*/)";
    r = se->RegisterGlobalFunction( "string GetParmGroupName( const string & in parm_id )", asFUNCTION( vsp::GetParmGroupName ), asCALL_CDECL, doc_struct );
    assert( r >= 0 );

    doc_struct.comment = R"(
/*!
    Get the display group name for the specified Parm
    \code{.cpp}
    string veh_id = FindContainer( "Vehicle", 0 );

    //==== Get and List All Parms in the Container ====// 
    array<string> parm_ids = FindContainerParmIDs( veh_id );

    Print( "Parm Group Display Names and IDs in Vehicle Parm Container: " );

    for ( uint i = 0; i < uint(parm_ids.length()); i++ )
    {
        string group_str = GetParmDisplayGroupName( parm_ids[i] ) + string(": ") + parm_ids[i] + string("\n");

        Print( group_str );
    }
    \endcode
    \param [in] parm_id Parm ID
    \return Parm display group name
*/)";
    r = se->RegisterGlobalFunction( "string GetParmDisplayGroupName( const string & in parm_id )", asFUNCTION( vsp::GetParmDisplayGroupName ), asCALL_CDECL, doc_struct );
    assert( r >= 0 );

    doc_struct.comment = R"(
/*!
    Get Parm Container ID for the specified Parm
    \code{.cpp}
    // Add Fuselage Geom
    string fuseid = AddGeom( "FUSELAGE", "" );

    string xsec_surf = GetXSecSurf( fuseid, 0 );

    ChangeXSecShape( xsec_surf, GetNumXSec( xsec_surf ) - 1, XS_ROUNDED_RECTANGLE );

    string xsec = GetXSec( xsec_surf, GetNumXSec( xsec_surf ) - 1 );

    string wid = GetXSecParm( xsec, "RoundedRect_Width" );

    string cid = GetParmContainer( wid );

    if ( cid.size() == 0 )                                { Print( "---> Error: API GetParmContainer " ); }
    \endcode
    \param [in] parm_id Parm ID
    \return Parm Container ID
*/)";
    r = se->RegisterGlobalFunction( "string GetParmContainer( const string & in parm_id )", asFUNCTION( vsp::GetParmContainer ), asCALL_CDECL, doc_struct );
    assert( r >= 0 );

    doc_struct.comment = R"(
/*!
    Set the description of the specified Parm
    \code{.cpp}
    string pod_id = AddGeom( "POD" );

    string length = FindParm( pod_id, "Length", "Design" );

    SetParmValLimits( length, 10.0, 0.001, 1.0e12 );

    SetParmDescript( length, "Total Length of Geom" );
    \endcode
    \param [in] parm_id Parm ID
    \param [in] desc Parm description
*/)";
    r = se->RegisterGlobalFunction( "void SetParmDescript( const string & in parm_id, const string & in desc )", asFUNCTION( vsp::SetParmDescript ), asCALL_CDECL, doc_struct );
    assert( r >= 0 );

    doc_struct.comment = R"(
/*!
    Find a Parm ID given the Parm Container ID, Parm name, and Parm group
    \code{.cpp}
    //==== Add Wing Geometry ====//
    string wing_id = AddGeom( "WING" );

    //==== Turn Symmetry OFF ====//
    string sym_id = FindParm( wing_id, "Sym_Planar_Flag", "Sym");

    SetParmVal( sym_id, 0.0 ); // Note: bool input not supported in SetParmVal
    \endcode
    \param [in] parm_container_id Parm Container ID
    \param [in] parm_name Parm name
    \param [in] group_name Parm group name
    \return Parm ID
*/)";
    r = se->RegisterGlobalFunction( "string FindParm( const string & in parm_container_id, const string & in parm_name, const string & in group_name )", asFUNCTION( vsp::FindParm ), asCALL_CDECL, doc_struct );
    assert( r >= 0 );

    doc_struct.comment = R"(
/*!
    Get Parm ID
    \code{.cpp}
    //==== Add Pod Geometry ====//
    string pid = AddGeom( "POD" );

    string lenid = GetParm( pid, "Length", "Design" );

    if ( !ValidParm( lenid ) )                { Print( "---> Error: API GetParm  " ); }
    \endcode
    \param [in] geom_id Geom ID
    \param [in] name Parm name
    \param [in] group Parm group name
    \return Array of Parm ID
*/)";
    r = se->RegisterGlobalFunction( "string GetParm(const string & in geom_id, const string & in name, const string & in group )", asFUNCTION( vsp::GetParm ), asCALL_CDECL, doc_struct );
    assert( r >= 0 );

    //=== Parm Container Functions ===//
    group = "ParmContainer";
    doc_struct.group = group.c_str();

    group_description = R"(
    \brief All Parms in OpenVSP are stored in Parm Containers. The functions in this group can be used to 
    work with Parm Containers through the API. \n\n
    \ref index "Click here to return to the main page" )";
    se->AddGroup( group.c_str(), "Parm Container Functions", group_description.c_str() );

    doc_struct.comment = R"(
/*!
    Get an array of all Parm Container IDs
    \code{.cpp}
    array<string> @ctr_arr = FindContainers();

    Print( "---> API Parm Container IDs: " );

    for ( int i = 0; i < int( ctr_arr.size() ); i++ )
    {
        string message = "\t" + ctr_arr[i] + "\n";

        Print( message );
    }
    \endcode
    \return Array of Parm Container IDs
*/)";
    r = se->RegisterGlobalFunction( "array<string>@ FindContainers()", asMETHOD( ScriptMgrSingleton, FindContainers ), asCALL_THISCALL_ASGLOBAL, &ScriptMgr, doc_struct );
    assert( r >= 0 );

    doc_struct.comment = R"(
/*!
    Get an array of Parm Container IDs for Containers with the specified name
    \code{.cpp}
    array<string> @ctr_arr = FindContainersWithName( "UserParms" );

    if ( ctr_arr.size() > 0 )            { Print( ( "UserParms Parm Container ID: " + ctr_arr[0] ) ); }
    \endcode
    \param [in] name Parm Container name
    \return Array of Parm Container IDs
*/)";
    r = se->RegisterGlobalFunction( "array<string>@ FindContainersWithName( const string & in name )", asMETHOD( ScriptMgrSingleton, FindContainersWithName ), asCALL_THISCALL_ASGLOBAL, &ScriptMgr, doc_struct);
    assert( r >= 0 );

    doc_struct.comment = R"(
/*!
    Get the ID of a Parm Container with specified name at input index
    \code{.cpp}
    //===== Get Vehicle Parm Container ID ====//
    string veh_id = FindContainer( "Vehicle", 0 );
    \endcode
    \sa FindContainersWithName
    \param [in] name Parm Container name
    \param [in] index Parm Container index
    \return Parm Container ID
*/)";
    r = se->RegisterGlobalFunction( "string FindContainer( const string & in name, int index )", asFUNCTION( vsp::FindContainer ), asCALL_CDECL, doc_struct);
    assert( r >= 0 );

    doc_struct.comment = R"(
/*!
    Get the name of the specified Parm Container
    \code{.cpp}
    string veh_id = FindContainer( "Vehicle", 0 );

    if ( GetContainerName( veh_id ) != "Vehicle" )         { Print( "---> Error: API GetContainerName" ); }
    \endcode
    \param [in] parm_container_id Parm Container ID
    \return Parm Container name
*/)";
    r = se->RegisterGlobalFunction( "string GetContainerName( const string & in parm_container_id )", asFUNCTION( vsp::GetContainerName ), asCALL_CDECL, doc_struct);
    assert( r >= 0 );

    doc_struct.comment = R"(
/*!
    Get an array of Parm group names included in the specified Container
    \code{.cpp}
    string user_ctr = FindContainer( "UserParms", 0 );

    array<string> @grp_arr = FindContainerGroupNames( user_ctr );

    Print( "---> UserParms Container Group IDs: " );
    for ( int i = 0; i < int( grp_arr.size() ); i++ )
    {
        string message = "\t" + grp_arr[i] + "\n";

        Print( message );
    }
    \endcode
    \param [in] parm_container_id Parm Container ID
    \return Array of Parm group names
*/)";
    r = se->RegisterGlobalFunction( "array<string>@ FindContainerGroupNames( const string & in parm_container_id )", asMETHOD( ScriptMgrSingleton, FindContainerGroupNames ), asCALL_THISCALL_ASGLOBAL, &ScriptMgr, doc_struct );
    assert( r >= 0 );

    doc_struct.comment = R"(
/*!
    Get an array of Parm IDs included in the specified Container
    \code{.cpp}
    //==== Add Pod Geometry ====//
    string pod_id = AddGeom( "POD" );

    //==== Add FeaStructure to Pod ====//
    int struct_ind = AddFeaStruct( pod_id );

    //==== Get Structure Name and Parm Container ID ====// 
    string parm_container_name = GetFeaStructName( pod_id, struct_ind );

    string parm_container_id = FindContainer( parm_container_name, struct_ind );

    //==== Get and List All Parms in the Container ====// 
    array<string> parm_ids = FindContainerParmIDs( parm_container_id );
    
    for ( uint i = 0; i < uint(parm_ids.length()); i++ )
    {
        string name_id = GetParmName( parm_ids[i] ) + string(": ") + parm_ids[i] + string("\n");

        Print( name_id );
    }
    \endcode
    \param [in] parm_container_id Parm Container ID
    \return Array of Parm IDs
*/)";
    r = se->RegisterGlobalFunction( "array<string>@ FindContainerParmIDs( const string & in parm_container_id )", asMETHOD( ScriptMgrSingleton, FindContainerParmIDs ), asCALL_THISCALL_ASGLOBAL, &ScriptMgr, doc_struct );
    assert( r >= 0 );

    doc_struct.comment = R"(
/*!
    Get the ID of the Vehicle Parm Container
    \code{.cpp}
    //===== Get Vehicle Parm Container ID ====//
    string veh_id = GetVehicleID();
    \endcode
    \return Vehicle ID
*/)";
    r = se->RegisterGlobalFunction( "string GetVehicleID()", asFUNCTION( vsp::GetVehicleID ), asCALL_CDECL, doc_struct);
    assert( r >= 0 );

    //=== Register Snap To Functions ====//
    group = "SnapTo";
    doc_struct.group = group.c_str();

    group_description = R"(
    \brief This group of API functions provide the capabilities available in the Snap-To tool. \n\n
    \ref index "Click here to return to the main page" )";
    se->AddGroup( group.c_str(), "Snap-To Functions", group_description.c_str() );

    doc_struct.comment = R"(
/*!
    Compute the minimum clearance distance for the specified geometry
    \code{.cpp}
    string fid = AddGeom( "FUSELAGE", "" );             // Add Fuselage

    string pid = AddGeom( "POD", "" );                     // Add Pod

    string x = GetParm( pid, "X_Rel_Location", "XForm" );

    SetParmVal( x, 3.0 );

    Update();

    double min_dist = ComputeMinClearanceDistance( pid, SET_ALL );
    \endcode
    \param [in] geom_id Geom ID
    \param [in] set Collision set enum (i.e. SET_ALL)
    \return Minimum clearance distance
*/)";
    r = se->RegisterGlobalFunction( "double ComputeMinClearanceDistance( const string & in geom_id, int set )", asFUNCTION( vsp::ComputeMinClearanceDistance ), asCALL_CDECL, doc_struct );
    assert( r >= 0 );

    doc_struct.comment = R"(
/*!
    Snap the specified Parm to input target minimum clearance distance
    \code{.cpp}
    //Add Geoms
    string fid = AddGeom( "FUSELAGE", "" );             // Add Fuselage

    string pid = AddGeom( "POD", "" );                     // Add Pod

    string x = GetParm( pid, "X_Rel_Location", "XForm" );

    SetParmVal( x, 3.0 );

    Update();

    double min_dist = SnapParm( x, 0.1, true, SET_ALL );
    \endcode
    \param [in] parm_id Parm ID
    \param [in] target_min_dist Target minimum clearance distance
    \param [in] inc_flag Direction indication flag. If true, upper parm limit is used and direction is set to positive
    \param [in] set Collision set enum (i.e. SET_ALL)
    \return Minimum clearance distance
*/)"; // TODO: Validate inc_flag description
    r = se->RegisterGlobalFunction( "double SnapParm( const string & in parm_id, double target_min_dist, bool inc_flag, int set )", asFUNCTION( vsp::SnapParm ), asCALL_CDECL, doc_struct );
    assert( r >= 0 );

    //=== Register Var Preset Functions ====//
    group = "VariablePreset";
    doc_struct.group = group.c_str();

    group_description = R"(
    \brief This group of functions can be used to add, remove, and modify Variable Presets through the API. \n\n
    \ref index "Click here to return to the main page" )";
    se->AddGroup( group.c_str(), "Variable Preset Functions", group_description.c_str() );

    doc_struct.comment = R"(
/*!
    Add a Variable Presets group
    \code{.cpp}
    // Add Pod Geom
    string pod1 = AddGeom( "POD", "" );

    AddVarPresetGroup( "Tess" );

    if ( GetVarPresetGroupNames().size() != 1 )                    { Print( "---> Error: API AddVarPresetGroup" ); }
    \endcode
    \param [in] group_name Variable Presets group name
*/)";
    r = se->RegisterGlobalFunction( "void AddVarPresetGroup( const string & in group_name )", asFUNCTION( vsp::AddVarPresetGroup ), asCALL_CDECL, doc_struct);
    assert( r >= 0 );

    doc_struct.comment = R"(
/*!
    Add a setting to the currently active Variable Preset
    \code{.cpp}
    // Add Pod Geom
    string pod1 = AddGeom( "POD", "" );

    AddVarPresetGroup( "Tess" );

    AddVarPresetSetting( "Coarse" );

    if ( GetVarPresetSettingNamesWName( "Tess" ).size() != 1 )            { Print( "---> Error: API AddVarPresetSetting" ); }
    \endcode
    \param [in] setting_name Variable Presets setting name
*/)";
    r = se->RegisterGlobalFunction( "void AddVarPresetSetting( const string & in setting_name )", asFUNCTION( vsp::AddVarPresetSetting ), asCALL_CDECL, doc_struct);
    assert( r >= 0 );

    doc_struct.comment = R"(
/*!
    Add a Parm to the currently active Variable Preset
    \code{.cpp}
    // Add Pod Geom
    string pod1 = AddGeom( "POD", "" );

    AddVarPresetGroup( "Tess" );

    AddVarPresetSetting( "Coarse" );

    string p1 = FindParm( pod1, "Tess_U", "Shape" );

    AddVarPresetParm( p1 );
    \endcode
    \param [in] parm_ID Parm ID
*/)";
    r = se->RegisterGlobalFunction( "void AddVarPresetParm( const string & in parm_ID )", asFUNCTIONPR( vsp::AddVarPresetParm, ( const string & ), void ), asCALL_CDECL, doc_struct);
    assert( r >= 0 );

    doc_struct.comment = R"(
/*!
    Add a Parm to the specified Variable Preset group
    \code{.cpp}
    // Add Pod Geom
    string pod1 = AddGeom( "POD", "" );

    AddVarPresetGroup( "Tess" );

    string p1 = FindParm( pod1, "Tess_U", "Shape" );

    AddVarPresetParm( p1, "Tess" );
    \endcode
    \param [in] parm_ID Parm ID
    \param [in] group_name Variable Presets group name
*/)";
    r = se->RegisterGlobalFunction( "void AddVarPresetParm( const string & in parm_ID, const string & in group_name )", asFUNCTIONPR( vsp::AddVarPresetParm, ( const string &, const string & ), void ), asCALL_CDECL, doc_struct);
    assert( r >= 0 );

    doc_struct.comment = R"(
/*!
    Edit the value of a Parm in the currently active Variable Preset
    \code{.cpp}
    // Add Pod Geom
    string pod1 = AddGeom( "POD", "" );

    AddVarPresetGroup( "Tess" );

    AddVarPresetSetting( "Coarse" );

    string p1 = FindParm( pod1, "Tess_U", "Shape" );

    AddVarPresetParm( p1 );

    EditVarPresetParm( p1, 5 );
    \endcode
    \param [in] parm_ID Parm ID
    \param [in] parm_val Parm value
*/)";
    r = se->RegisterGlobalFunction( "void EditVarPresetParm( const string & in parm_ID, double parm_val )", asFUNCTIONPR( vsp::EditVarPresetParm, ( const string &, double ), void ), asCALL_CDECL, doc_struct);
    assert( r >= 0 );

    doc_struct.comment = R"(
/*!
    Edit the value of a Parm in the specified Variable Preset group and setting
    \code{.cpp}
    // Add Pod Geom
    string pod1 = AddGeom( "POD", "" );

    AddVarPresetGroup( "Tess" );

    AddVarPresetSetting( "Coarse" );

    string p1 = FindParm( pod1, "Tess_U", "Shape" );

    AddVarPresetParm( p1 );

    AddVarPresetGroup( "New_Group" );

    EditVarPresetParm( p1, 5, "Tess", "Coarse" );
    \endcode
    \param [in] parm_ID Parm ID
    \param [in] parm_val Parm value
    \param [in] group_name Variable Presets group name
    \param [in] setting_name Variable Presets setting name
*/)";
    r = se->RegisterGlobalFunction( "void EditVarPresetParm( const string & in parm_ID, double parm_val, const string & in group_name, const string & in setting_name )", asFUNCTIONPR( vsp::EditVarPresetParm,
        ( const string &, double, const string &, const string & ), void ), asCALL_CDECL, doc_struct);
    assert( r >= 0 );

    doc_struct.comment = R"(
/*!
    Remove a Parm from the currently active Variable Preset group
    \code{.cpp}
    // Add Pod Geom
    string pod1 = AddGeom( "POD", "" );

    AddVarPresetGroup( "Tess" );

    AddVarPresetSetting( "Coarse" );

    string p1 = FindParm( pod1, "Tess_U", "Shape" );

    AddVarPresetParm( p1 );

    DeleteVarPresetParm( p1 );
    \endcode
    \param [in] parm_ID Parm ID
*/)";
    r = se->RegisterGlobalFunction( "void DeleteVarPresetParm( const string & in parm_ID )", asFUNCTIONPR( vsp::DeleteVarPresetParm, ( const string & ), void ), asCALL_CDECL, doc_struct);
    assert( r >= 0 );

    doc_struct.comment = R"(
/*!
    Remove a Parm from a Variable Preset group
    \code{.cpp}
    // Add Pod Geom
    string pod1 = AddGeom( "POD", "" );

    AddVarPresetGroup( "Tess" );

    AddVarPresetSetting( "Coarse" );

    string p1 = FindParm( pod1, "Tess_U", "Shape" );

    AddVarPresetParm( p1 );

    AddVarPresetGroup( "New_Group" );

    DeleteVarPresetParm( p1, "Tess" );
    \endcode
    \param [in] parm_ID Parm ID
    \param [in] group_name Variable Presets group name
*/)";
    r = se->RegisterGlobalFunction( "void DeleteVarPresetParm( const string & in parm_ID, const string & in group_name )", asFUNCTIONPR( vsp::DeleteVarPresetParm, ( const string &, const string & ), void ), asCALL_CDECL, doc_struct);
    assert( r >= 0 );

    doc_struct.comment = R"(
/*!
    Change the currently active Variable Preset
    \code{.cpp}
    // Add Pod Geom
    string pod1 = AddGeom( "POD", "" );

    AddVarPresetGroup( "Config" );

    AddVarPresetSetting( "Default" );

    string p1 = FindParm( pod1, "Y_Rel_Rotation", "XForm" );
    string p2 = FindParm( pod1, "Z_Rel_Rotation", "XForm" );

    AddVarPresetParm( p1 );
    AddVarPresetParm( p2 );

    SwitchVarPreset( "Config", "Default" );
    \endcode
    \param [in] group_name Variable Presets group name
    \param [in] setting_name Variable Presets setting name
*/)";
    r = se->RegisterGlobalFunction( "void SwitchVarPreset( const string & in group_name, const string & in setting_name )", asFUNCTION( vsp::SwitchVarPreset ), asCALL_CDECL, doc_struct);
    assert( r >= 0 );

    doc_struct.comment = R"(
/*!
    Delete a Variable Preset
    \code{.cpp}
    AddVarPresetGroup( "Tess" );

    AddVarPresetSetting( "Fine" );
    
    DeleteVarPresetSet( "Tess", "Fine" );

    if ( GetVarPresetSettingNamesWName( "Tess" ).size() != 0 )    { Print ( "---> Error: DeleteVarPresetSet" ); }
    \endcode
    \param [in] group_name Variable Presets group
    \param [in] setting_name Variable Presets setting name
    \return true is successful, false otherwise
*/)";
    r = se->RegisterGlobalFunction( "bool DeleteVarPresetSet( const string & in group_name, const string & in setting_name )", asFUNCTION( vsp::DeleteVarPresetSet ), asCALL_CDECL, doc_struct);
    assert( r >= 0 );

    doc_struct.comment = R"(
/*!
    Get the currently active Variable Presets group name
    \code{.cpp}
    // Add Pod Geom
    string pod1 = AddGeom( "POD", "" );

    AddVarPresetGroup( "Tess" );

    AddVarPresetSetting( "Coarse" );

    string p1 = FindParm( pod1, "Tess_U", "Shape" );

    AddVarPresetParm( p1 );

    AddVarPresetGroup( "New_Group" );

    AddVarPresetSetting( "New_Setting" );

    Print( "Current Group: " );

    Print( GetCurrentGroupName() );
    \endcode
    \return Variable Presets group name
*/)";
    r = se->RegisterGlobalFunction( "string GetCurrentGroupName()", asFUNCTION( vsp::GetCurrentGroupName ), asCALL_CDECL, doc_struct);
    assert( r >= 0 );

    doc_struct.comment = R"(
/*!
    Get the currently active Variable Presets setting name
    \code{.cpp}
    // Add Pod Geom
    string pod1 = AddGeom( "POD", "" );

    AddVarPresetGroup( "Tess" );

    AddVarPresetSetting( "Coarse" );

    string p1 = FindParm( pod1, "Tess_U", "Shape" );

    AddVarPresetParm( p1 );

    AddVarPresetGroup( "New_Group" );

    AddVarPresetSetting( "New_Setting" );

    Print( "Current Setting: " );

    Print( GetCurrentSettingName() );
    \endcode
    \return Variable Presets setting name
*/)";
    r = se->RegisterGlobalFunction( "string GetCurrentSettingName()", asFUNCTION( vsp::GetCurrentSettingName ), asCALL_CDECL, doc_struct);
    assert( r >= 0 );

    doc_struct.comment = R"(
/*!
    Get all Variable Preset group names
    \code{.cpp}
    // Add Pod Geom
    string pod1 = AddGeom( "POD", "" );

    AddVarPresetGroup( "Tess" );

    if ( GetVarPresetGroupNames().size() != 1 )                    { Print( "---> Error: API AddVarPresetGroup" ); }
    \endcode
    \return Array of Variable Presets group names
*/)";
    r = se->RegisterGlobalFunction( "array<string>@ GetVarPresetGroupNames()", asMETHOD( ScriptMgrSingleton, GetVarPresetGroupNames ), asCALL_THISCALL_ASGLOBAL, &ScriptMgr, doc_struct);
    assert( r >= 0 );

    doc_struct.comment = R"(
/*!
    Get the name of each settings in the specified Variable Presets group name
    \code{.cpp}
    // Add Pod Geom
    string pod1 = AddGeom( "POD", "" );

    AddVarPresetGroup( "Tess" );

    AddVarPresetSetting( "Coarse" );

    if ( GetVarPresetSettingNamesWName( "Tess" ).size() != 1 )            { Print( "---> Error: API AddVarPresetSetting" ); }
    \endcode
    \param [in] group_name Variable Presets group name
    \return Array of Variable Presets setting names
*/)";
    r = se->RegisterGlobalFunction( "array<string>@ GetVarPresetSettingNamesWName( const string & in group_name )", asMETHOD( ScriptMgrSingleton, GetVarPresetSettingNamesWName ), asCALL_THISCALL_ASGLOBAL, &ScriptMgr, doc_struct);
    assert( r >= 0 );

    doc_struct.comment = R"(
/*!
    Get the name of each settings in the specified Variable Presets group index
    \code{.cpp}
    // Add Pod Geom
    string pod1 = AddGeom( "POD", "" );

    AddVarPresetGroup( "Tess" );

    AddVarPresetSetting( "Coarse" );

    string p1 = FindParm( pod1, "Tess_U", "Shape" );

    AddVarPresetParm( p1 );

    AddVarPresetGroup( "New_Group" );

    AddVarPresetSetting( "New_Setting_1" );
    AddVarPresetSetting( "New_Setting_2" );

    array < string > group_1_settings = GetVarPresetSettingNamesWIndex( 1 );

    if ( group_1_settings.size() != 2 )            { Print( "---> Error: API GetVarPresetSettingNamesWIndex" ); }
    \endcode
    \param [in] group_index Variable Presets group index
    \return Array of Variable Presets setting names
*/)";
    r = se->RegisterGlobalFunction( "array<string>@ GetVarPresetSettingNamesWIndex( int group_index )", asMETHOD( ScriptMgrSingleton, GetVarPresetSettingNamesWIndex ), asCALL_THISCALL_ASGLOBAL, &ScriptMgr, doc_struct);
    assert( r >= 0 );

    doc_struct.comment = R"(
/*!
    Get the value of each Parm in the currently active Variable Preset group and setting
    \code{.cpp}
    // Add Pod Geom
    string pod1 = AddGeom( "POD", "" );

    AddVarPresetGroup( "Tess" );

    AddVarPresetSetting( "Coarse" );

    string p1 = FindParm( pod1, "Tess_U", "Shape" );

    AddVarPresetParm( p1 );

    EditVarPresetParm( p1, 5 );

    array <double> p_vals = GetVarPresetParmVals();

    if ( p_vals[0] != 5 )                                { Print ( "---> Error: API EditVarPresetParm" ); }
    \endcode
    \return Array of Variable Presets Parm values
*/)";
    r = se->RegisterGlobalFunction( "array<double>@ GetVarPresetParmVals()", asMETHOD( ScriptMgrSingleton, GetVarPresetParmVals ), asCALL_THISCALL_ASGLOBAL, &ScriptMgr, doc_struct);
    assert( r >= 0 );

    doc_struct.comment = R"(
/*!
    Get the value of each Parm in the specified Variable Preset group and setting
    param [in] group_name Variable Presets group name
    param [in] setting_name Variable Presets setting name
    \code{.cpp}
    // Add Pod Geom
    string pod1 = AddGeom( "POD", "" );

    AddVarPresetGroup( "New_Group" );

    AddVarPresetSetting( "New_Setting_1" );
    AddVarPresetSetting( "New_Setting_2" );

    string p1 = FindParm( pod1, "Y_Rel_Rotation", "XForm" );
    string p2 = FindParm( pod1, "Z_Rel_Rotation", "XForm" );

    AddVarPresetParm( p1 );
    AddVarPresetParm( p2 );

    EditVarPresetParm( p2, 2, "New_Group", "New_Setting_2" );

    array < double > parm_vals = GetVarPresetParmValsWNames( "New_Group", "New_Setting_2" );

    if ( parm_vals.size() != 2 )            { Print( "---> Error: API GetVarPresetParmValsWNames" ); }
    \endcode
    \return Array of Variable Presets Parm values
*/)";
    r = se->RegisterGlobalFunction( "array<double>@ GetVarPresetParmValsWNames( const string & in group_name, const string & in setting_name )", asMETHOD( ScriptMgrSingleton, GetVarPresetParmValsWNames ), asCALL_THISCALL_ASGLOBAL, &ScriptMgr, doc_struct);
    assert( r >= 0 );

    doc_struct.comment = R"(
/*!
    Get the Parm IDs contained in the currently active Variable Presets group
    \code{.cpp}
    // Add Pod Geom
    string pod1 = AddGeom( "POD", "" );

    AddVarPresetGroup( "Tess" );

    AddVarPresetSetting( "Coarse" );

    string p1 = FindParm( pod1, "Tess_U", "Shape" );

    AddVarPresetParm( p1 );

    array <string> p_IDs = GetVarPresetParmIDs();

    if ( p_IDs.size() != 1 )                                { Print( "---> Error: API AddVarPresetParm" ); }
    \endcode
    \return Array of Variable Presets Parm IDs
*/)";
    r = se->RegisterGlobalFunction( "array<string>@ GetVarPresetParmIDs()", asMETHOD( ScriptMgrSingleton, GetVarPresetParmIDs ), asCALL_THISCALL_ASGLOBAL, &ScriptMgr, doc_struct);
    assert( r >= 0 );

    doc_struct.comment = R"(
/*!
    Get the Parm IDs contained in the specitied Variable Presets group
    \code{.cpp}
    // Add Pod Geom
    string pod1 = AddGeom( "POD", "" );

    AddVarPresetGroup( "New_Group" );

    AddVarPresetSetting( "New_Setting_1" );
    AddVarPresetSetting( "New_Setting_2" );

    string p1 = FindParm( pod1, "Y_Rel_Rotation", "XForm" );
    string p2 = FindParm( pod1, "Z_Rel_Rotation", "XForm" );

    AddVarPresetParm( p1 );
    AddVarPresetParm( p2 );

    array < string > parm_ids = GetVarPresetParmIDsWName( "New_Group" );

    if ( parm_ids.size() != 2 )            { Print( "---> Error: API GetVarPresetParmIDsWName" ); }
    \endcode
    \param [in] group_name Variable Presets group name
    \return Array of Parm IDs
*/)";
    r = se->RegisterGlobalFunction( "array<string>@ GetVarPresetParmIDsWName( const string & in group_name )", asMETHOD( ScriptMgrSingleton, GetVarPresetParmIDsWName ), asCALL_THISCALL_ASGLOBAL, &ScriptMgr, doc_struct);
    assert( r >= 0 );

    //=== Register PCurve Functions ====//
    group = "PCurve";
    doc_struct.group = group.c_str();

    group_description = R"(
    \brief The following group of API functions may be used to control parametric propeller blade curves (PCurves). \n\n
    \ref index "Click here to return to the main page" )";
    se->AddGroup( group.c_str(), "Propeller Blade Curve Functions", group_description.c_str() );

    doc_struct.comment = R"(
/*!
    Set the parameters, values, and curve type of a propeller blade curve (P Curve)
    \sa PCURV_TYPE
    \param [in] geom_id Parent Geom ID
    \param [in] pcurveid P Curve index
    \param [in] tvec Array of parameter values
    \param [in] valvec Array of values
    \param [in] newtype Curve type enum (i.e. CEDIT)
*/)";
    r = se->RegisterGlobalFunction( "void SetPCurve( const string& in geom_id, const int & in pcurveid, array<double>@ tvec, array<double>@ valvec, const int & in newtype )", asMETHOD( ScriptMgrSingleton, SetPCurve ), asCALL_THISCALL_ASGLOBAL, &ScriptMgr, doc_struct );
    assert( r >= 0 ); // TODO: Example

    doc_struct.comment = R"(
/*!
    Change the type of a propeller blade curve (P Curve)
    \sa PCURV_TYPE
    \param [in] geom_id Parent Geom ID
    \param [in] pcurveid P Curve index
    \param [in] newtype Curve type enum (i.e. CEDIT)
*/)";
    r = se->RegisterGlobalFunction( "void PCurveConvertTo( const string & in geom_id, const int & in pcurveid, const int & in newtype )", asFUNCTION( vsp::PCurveConvertTo ), asCALL_CDECL, doc_struct );
    assert( r >= 0 ); // TODO: Example

    doc_struct.comment = R"(
/*!
    Get the type of a propeller blade curve (P Curve)
    \sa PCURV_TYPE
    \param [in] geom_id Parent Geom ID
    \param [in] pcurveid P Curve index
    \return Curve type enum (i.e. CEDIT)
*/)";
    r = se->RegisterGlobalFunction( "int PCurveGetType( const string & in geom_id, const int & in pcurveid )", asFUNCTION( vsp::PCurveGetType ), asCALL_CDECL, doc_struct );
    assert( r >= 0 ); // TODO: Example

    doc_struct.comment = R"(
/*!
    Get the parameters of a propeller blade curve (P Curve). Each parameter is a fraction of propeller radius.
    \param [in] geom_id Parent Geom ID
    \param [in] pcurveid P Curve index
    \return Array of parameters
*/)";
    r = se->RegisterGlobalFunction( "array<double>@ PCurveGetTVec( const string & in geom_id, const int & in pcurveid )", asMETHOD( ScriptMgrSingleton, PCurveGetTVec ), asCALL_THISCALL_ASGLOBAL, &ScriptMgr, doc_struct );
    assert( r >= 0 ); // TODO: Example

    doc_struct.comment = R"(
/*!
    Get the values of a propeller blade curve (P Curve). What the values represent id dependent on the curve type (i.e. twist, chord, etc.).
    \param [in] geom_id Parent Geom ID
    \param [in] pcurveid P Curve index
    \return Array of values
*/)";
    r = se->RegisterGlobalFunction( "array<double>@ PCurveGetValVec( const string & in geom_id, const int & in pcurveid )", asMETHOD( ScriptMgrSingleton, PCurveGetValVec ), asCALL_THISCALL_ASGLOBAL, &ScriptMgr, doc_struct );
    assert( r >= 0 ); // TODO: Example

    doc_struct.comment = R"(
/*!
    Delete a propeller blade curve (P Curve) point
    \param [in] geom_id Parent Geom ID
    \param [in] pcurveid P Curve index
    \param [in] indx Point index
*/)";
    r = se->RegisterGlobalFunction( "void PCurveDeletePt( const string & in geom_id, const int & in pcurveid, const int & in indx )", asFUNCTION( vsp::PCurveDeletePt ), asCALL_CDECL, doc_struct);
    assert( r >= 0 ); // TODO: Example

    doc_struct.comment = R"(
/*!
    Split a propeller blade curve (P Curve) at the specified 1D parameter
    \param [in] geom_id Parent Geom ID
    \param [in] pcurveid P Curve index
    \param [in] tsplit 1D parameter split location
    \return Index of new control point
*/)";
    r = se->RegisterGlobalFunction( "int PCurveSplit( const string & in geom_id, const int & in pcurveid, const double & in tsplit )", asFUNCTION( vsp::PCurveSplit ), asCALL_CDECL, doc_struct);
    assert( r >= 0 ); // TODO: Example

    doc_struct.comment = R"(
/*!
    Approximate all propeller blade curves with cubic Bezier curves.
    \code{.cpp}
    // Add Propeller
    string prop = AddGeom( "PROP", "" );

    ApproximateAllPropellerPCurves( prop );

    \endcode
    \param [in] geom_id Geom ID
    */)";
    r = se->RegisterGlobalFunction( "void ApproximateAllPropellerPCurves( const string & in geom_id )", asFUNCTION( vsp::ApproximateAllPropellerPCurves ), asCALL_CDECL, doc_struct);
    assert( r >= 0 );

    doc_struct.comment = R"(
/*!
    Reset propeller T/C curve to match basic thickness of file-type airfoils.  Typically only used for a propeller that
    has been constructed with file-type airfoils across the blade.  The new thickness curve will be a PCHIP curve
    with t/c matching the propeller's XSecs -- unless it is a file XSec, then the Base thickness is used.
    \code{.cpp}
    // Add Propeller
    string prop = AddGeom( "PROP", "" );

    ResetPropellerThicknessCurve( prop );

    \endcode
    \param [in] geom_id Geom ID
    */)";
    r = se->RegisterGlobalFunction( "void ResetPropellerThicknessCurve( const string & in geom_id )", asFUNCTION( vsp::ResetPropellerThicknessCurve ), asCALL_CDECL, doc_struct);
    assert( r >= 0 );

    //=== Register ParasiteDragTool Functions ====//
    group = "ParasiteDrag";
    doc_struct.group = group.c_str();

    group_description = R"(
    \brief This group of API functions is supplemental to performing a Paraste Drag analysis through the Analysis Manager. They include
    functions to write out Parasite Drag Tool equations, calculate atmospheric properties, and control excrescences. \n\n
    \ref index "Click here to return to the main page")";
    se->AddGroup( group.c_str(), "Parasite Drag Functions", group_description.c_str() );

    doc_struct.comment = R"(
/*!
    Add an Excresence to the Parasite Drag Tool
    \code{.cpp}
    AddExcrescence( "Miscellaneous", EXCRESCENCE_COUNT, 8.5 );

    AddExcrescence( "Cowl Boattail", EXCRESCENCE_CD, 0.0003 );
    \endcode
    \sa EXCRES_TYPE
    \param [in] excresName Name of the Excressence
    \param [in] excresType Excressence type enum (i.e. EXCRESCENCE_PERCENT_GEOM)
    \param [in] excresVal Excressence value
*/)";
    r = se->RegisterGlobalFunction( "void AddExcrescence(const string & in excresName, const int & in excresType, const double & in excresVal)", asFUNCTION( vsp::AddExcrescence ), asCALL_CDECL, doc_struct );
    assert( r >= 0 );

    doc_struct.comment = R"(
/*!
    Delete an Excresence from the Parasite Drag Tool
    \code{.cpp}
    AddExcrescence( "Miscellaneous", EXCRESCENCE_COUNT, 8.5 );

    AddExcrescence( "Cowl Boattail", EXCRESCENCE_CD, 0.0003 );

    AddExcrescence( "Percentage Example", EXCRESCENCE_PERCENT_GEOM, 5 );

    DeleteExcrescence( 2 ); // Last Index
    \endcode
    \param [in] excresName Name of the Excressence
*/)";
    r = se->RegisterGlobalFunction( "void DeleteExcrescence(const int & in excresName)", asFUNCTION( vsp::DeleteExcrescence ), asCALL_CDECL, doc_struct );
    assert( r >= 0 );

    doc_struct.comment = R"(
/*!
    Update any reference geometry, atmospheric properties, excressences, etc. in the Parasite Drag Tool
*/)";
    r = se->RegisterGlobalFunction( "void UpdateParasiteDrag()", asFUNCTION( vsp::UpdateParasiteDrag ), asCALL_CDECL, doc_struct );
    assert( r >= 0 ); // TODO: Example

    doc_struct.comment = R"(
/*!
    Calculate the atmospheric properties determined by a specified model for a preset array of altitudes ranging from 0 to 90000 m and 
    write the results to a CSV output file
    \code{.cpp}
    Print( "Starting USAF Atmosphere 1966 Table Creation. \n" );

    WriteAtmosphereCSVFile( "USAFAtmosphere1966Data.csv", ATMOS_TYPE_HERRINGTON_1966 );
    \endcode
    \sa ATMOS_TYPE
    \param [in] file_name Output CSV file
    \param [in] atmos_type Atmospheric model enum (i.e. ATMOS_TYPE_HERRINGTON_1966)
*/)";
    r = se->RegisterGlobalFunction( "void WriteAtmosphereCSVFile( const string & in file_name, const int & in atmos_type )", asFUNCTION( vsp::WriteAtmosphereCSVFile ), asCALL_CDECL, doc_struct );
    assert( r >= 0 );

    doc_struct.comment = R"(
/*!
    Calculate the atmospheric properties determined by a specified model at input altitude and temperature deviation. This function may 
    not be used for any manual atmospheric model types (i.e. ATMOS_TYPE_MANUAL_P_T). This function assumes freestream units are metric,
    temperature units are Kelvin, and pressure units are kPA.
    \code{.cpp}
    double temp, pres, pres_ratio, rho_ratio;

    double alt = 4000;

    double delta_temp = 0;

    CalcAtmosphere( alt, delta_temp, ATMOS_TYPE_US_STANDARD_1976, temp, pres, pres_ratio, rho_ratio );
    \endcode
    \sa ATMOS_TYPE
    \param [in] alt Altitude
    \param [in] delta_temp Deviation in temperature from the value specified in the atmospheric model
    \param [in] atmos_type Atmospheric model enum (i.e. ATMOS_TYPE_HERRINGTON_1966)
    \param [out] temp output Temperature
    \param [out] pres output Pressure
    \param [out] pres_ratio Output pressure ratio
    \param [out] rho_ratio Output density ratio
*/)";
    r = se->RegisterGlobalFunction( "void CalcAtmosphere( const double & in alt, const double & in delta_temp, const int & in atmos_type, double & out temp, double & out pres, double & out pres_ratio, double & out rho_ratio )", asFUNCTION( vsp::CalcAtmosphere ), asCALL_CDECL, doc_struct );
    assert( r >= 0 );

    doc_struct.comment = R"(
/*!
    Calculate the form factor from each body FF equation (i.e. Hoerner Streamlined Body) and write the results to a CSV output file
    \code{.cpp}
    Print( "Starting Body Form Factor Data Creation. \n" );
    WriteBodyFFCSVFile( "BodyFormFactorData.csv" );
    \endcode
    \param [in] file_name Output CSV file
*/)";
    r = se->RegisterGlobalFunction( "void WriteBodyFFCSVFile( const string & in file_name )", asFUNCTION( vsp::WriteBodyFFCSVFile ), asCALL_CDECL, doc_struct );
    assert( r >= 0 );

    doc_struct.comment = R"(
/*!
    Calculate the form factor from each wing FF equation (i.e. Schemensky 4 Series Airfoil) and write the results to a CSV output file
    \code{.cpp}
    Print( "Starting Wing Form Factor Data Creation. \n" );
    WriteWingFFCSVFile( "WingFormFactorData.csv" );
    \endcode
    \param [in] file_name Output CSV file
*/)";
    r = se->RegisterGlobalFunction( "void WriteWingFFCSVFile( const string & in file_name )", asFUNCTION( vsp::WriteWingFFCSVFile ), asCALL_CDECL, doc_struct );
    assert( r >= 0 );

    doc_struct.comment = R"(
/*!
    Calculate the coefficient of friction from each Cf equation (i.e. Power Law Blasius) and write the results to a CSV output file
    \code{.cpp}
    Print( "Starting Turbulent Friciton Coefficient Data Creation. \n" );
    WriteCfEqnCSVFile( "FrictionCoefficientData.csv" );
    \endcode
    \param [in] file_name Output CSV file
*/)"; // TODO: Improve description
    r = se->RegisterGlobalFunction( "void WriteCfEqnCSVFile( const string & in file_name )", asFUNCTION( vsp::WriteCfEqnCSVFile ), asCALL_CDECL, doc_struct );
    assert( r >= 0 );

    doc_struct.comment = R"(
/*!
    Calculate the partial coefficient of friction and write the results to a CSV output file
    \code{.cpp}
    Print( "Starting Partial Friction Method Data Creation. \n" );
    WritePartialCfMethodCSVFile( "PartialFrictionMethodData.csv" );
    \endcode
    \param [in] file_name Output CSV file
*/)"; // TODO: Improve description
    r = se->RegisterGlobalFunction( "void WritePartialCfMethodCSVFile( const string & in file_name )", asFUNCTION( vsp::WritePartialCfMethodCSVFile ), asCALL_CDECL, doc_struct );
    assert( r >= 0 );

    //=== Register Surface Query Functions ===//
    group = "SurfaceQuery";
    doc_struct.group = group.c_str();

    group_description = R"(
    \brief This group of API functions pertains to general surface queries for Geom surfaces, such as computing
    3D location from surface coordinates, identifying curvature, and performing point projections. \n\n
    \ref index "Click here to return to the main page" )";
    se->AddGroup( group.c_str(), "Geom Surface Query Functions", group_description.c_str() );

    doc_struct.comment = R"(
/*!
    Calculate the 3D coordinate equivalent for the input surface coordinate point
    \code{.cpp}
    // Add Pod Geom
    string geom_id = AddGeom( "POD", "" );

    int surf_indx = 0;

    double u = 0.12345;
    double w = 0.67890;

    vec3d pnt = CompPnt01( geom_id, surf_indx, u, w );

    Print( "Point: ( " + pnt.x() + ', ' + pnt.y() + ', ' + pnt.z() + ' )' );
    \endcode
    \param [in] geom_id Parent Geom ID
    \param [in] surf_indx Main surface index from the parent Geom
    \param [in] u U (0 - 1) surface coordinate
    \param [in] w W (0 - 1) surface coordinate
    \return Normal vector3D coordinate point
*/)";
    r = se->RegisterGlobalFunction( "vec3d CompPnt01( const string & in geom_id, const int & in surf_indx, const double & in u, const double & in w )", asFUNCTION(vsp::CompPnt01), asCALL_CDECL, doc_struct );
    assert( r >= 0 );

    doc_struct.comment = R"(
/*!
    Calculate the normal vector on the specified surface at input surface coordinate
    \code{.cpp}
    // Add Pod Geom
    string geom_id = AddGeom( "POD", "" );

    int surf_indx = 0;

    double u = 0.12345;
    double w = 0.67890;

    vec3d norm = CompNorm01( geom_id, surf_indx, u, w );

    Print( "Point: ( " + norm.x() + ', ' + norm.y() + ', ' + norm.z() + ' )' );
    \endcode
    \param [in] geom_id Parent Geom ID
    \param [in] surf_indx Main surface index from the parent Geom
    \param [in] u U (0 - 1) surface coordinate
    \param [in] w W (0 - 1) surface coordinate
    \return Normal vector
*/)";
    r = se->RegisterGlobalFunction( "vec3d CompNorm01( const string & in geom_id, const int & in surf_indx, const double & in u, const double & in w )", asFUNCTION(vsp::CompNorm01), asCALL_CDECL, doc_struct );
    assert( r >= 0 );

    doc_struct.comment = R"(
/*!
    Calculate the vector tangent to the specified surface at input surface coordinate in the U direction
    \code{.cpp}
    // Add Pod Geom
    string geom_id = AddGeom( "POD", "" );

    int surf_indx = 0;

    double u = 0.12345;
    double w = 0.67890;

    vec3d tanu = CompTanU01( geom_id, surf_indx, u, w );

    Print( "Point: ( " + tanu.x() + ', ' + tanu.y() + ', ' + tanu.z() + ' )' );
    \endcode
    \param [in] geom_id Parent Geom ID
    \param [in] surf_indx Main surface index from the parent Geom
    \param [in] u U (0 - 1) surface coordinate
    \param [in] w W (0 - 1) surface coordinate
    \return Tangent vector in U direction
*/)";
    r = se->RegisterGlobalFunction( "vec3d CompTanU01( const string & in geom_id, const int & in surf_indx, const double & in u, const double & in w )", asFUNCTION(vsp::CompTanU01), asCALL_CDECL, doc_struct );
    assert( r >= 0 );

    doc_struct.comment = R"(
/*!
    Calculate the vector tangent to the specified surface at input surface coordinate in the W direction
    \code{.cpp}
    // Add Pod Geom
    string geom_id = AddGeom( "POD", "" );

    int surf_indx = 0;

    double u = 0.12345;
    double w = 0.67890;

    vec3d tanw = CompTanW01( geom_id, surf_indx, u, w );

    Print( "Point: ( " + tanw.x() + ', ' + tanw.y() + ', ' + tanw.z() + ' )' );
    \endcode
    \param [in] geom_id Parent Geom ID
    \param [in] surf_indx Main surface index from the parent Geom
    \param [in] u U (0 - 1) surface coordinate
    \param [in] w W (0 - 1) surface coordinate
    \return Tangent vector in W direction
*/)";
    r = se->RegisterGlobalFunction( "vec3d CompTanW01( const string & in geom_id, const int & in surf_indx, const double & in u, const double & in w )", asFUNCTION(vsp::CompTanW01), asCALL_CDECL, doc_struct );
    assert( r >= 0 );

    doc_struct.comment = R"(
/*!
    Determine the curvature of a specified surface at the input surface coordinate point
    \code{.cpp}
    // Add Pod Geom
    string geom_id = AddGeom( "POD", "" );

    int surf_indx = 0;

    double k1, k2, ka, kg;

    double u, w;
    u = 0.25;
    w = 0.75;

    CompCurvature01( geom_id, surf_indx, u, w, k1, k2, ka, kg );

    Print( "Curvature : k1 " + k1 + " k2 " + k2 + " ka " + ka + " kg " + kg );
    \endcode
    \param [in] geom_id Parent Geom ID
    \param [in] surf_indx Main surface index from the parent Geom
    \param [in] u U (0 - 1) surface coordinate
    \param [in] w W (0 - 1) surface coordinate
    \param [out] k1 Output value of maximum principal curvature
    \param [out] k2 Output value of minimum principal curvature
    \param [out] ka Output value of mean curvature
    \param [out] kg Output value of Gaussian curvature
*/)";
    r = se->RegisterGlobalFunction( "void CompCurvature01( const string & in geom_id, const int & in surf_indx, const double & in u, const double & in w, double & out k1, double & out k2, double & out ka, double & out kg )", asFUNCTION(vsp::CompCurvature01), asCALL_CDECL, doc_struct );
    assert( r >= 0 );

    doc_struct.comment = R"(
/*!
    Determine the nearest surface coordinate for an input 3D coordinate point and calculate the distance between the 
    3D point and the closest point of the surface.
    \code{.cpp}
    // Add Pod Geom
    string geom_id = AddGeom( "POD", "" );

    int surf_indx = 0;

    double u = 0.12345;
    double w = 0.67890;

    vec3d pnt = CompPnt01( geom_id, surf_indx, u, w );

    vec3d norm = CompNorm01( geom_id, surf_indx, u, w );

    double uout, wout;

    // Offset point from surface
    pnt = pnt + norm;

    double d = ProjPnt01( geom_id, surf_indx, pnt, uout, wout );

    Print( "Dist " + d + " u " + uout + " w " + wout );
    \endcode
    \sa ProjPnt01Guess, ProjPnt01I
    \param [in] geom_id Parent Geom ID
    \param [in] surf_indx Main surface index from the parent Geom
    \param [in] pt Input 3D coordinate point
    \param [out] u Output closest U (0 - 1) surface coordinate
    \param [out] w Output closest W (0 - 1) surface coordinate
    \return Distance between the 3D point and the closest point of the surface
*/)";
    r = se->RegisterGlobalFunction( "double ProjPnt01( const string & in geom_id, const int & in surf_indx, const vec3d & in pt, double & out u, double & out w )", asFUNCTION(vsp::ProjPnt01), asCALL_CDECL, doc_struct );
    assert( r >= 0 );

    doc_struct.comment = R"(
/*!
    Determine the nearest surface coordinate and corresponding parent Geom main surface index for an input 3D coordinate point. Return the distance between
    the closest point and the input.
    \code{.cpp}
    // Add Pod Geom
    string geom_id = AddGeom( "POD", "" );

    int surf_indx = 0;

    double u = 0.12345;
    double w = 0.67890;

    double d = 0;

    vec3d pnt = CompPnt01( geom_id, surf_indx, u, w );

    vec3d norm = CompNorm01( geom_id, surf_indx, u, w );

    double uout, wout;

    int surf_indx_out;

    // Offset point from surface
    pnt = pnt + norm;

    d = ProjPnt01I( geom_id, pnt, surf_indx_out, uout, wout );

    Print( "Dist " + d + " u " + uout + " w " + wout + " surf_index " + surf_indx_out );
    \endcode
    \sa ProjPnt01, ProjPnt01Guess
    \param [in] geom_id Parent Geom ID
    \param [in] pt Input 3D coordinate point
    \param [out] surf_indx Output main surface index from the parent Geom
    \param [out] u Output closest U (0 - 1) surface coordinat
    \param [out] w Output closest W (0 - 1) surface coordinat
    \return Distance between the 3D point and the closest point of the surface
*/)";
    r = se->RegisterGlobalFunction( "double ProjPnt01I( const string & in geom_id, const vec3d & in pt, int & out surf_indx, double & out u, double & out w )", asFUNCTION(vsp::ProjPnt01I), asCALL_CDECL, doc_struct );
    assert( r >= 0 );

    doc_struct.comment = R"(
/*!
    Determine the nearest surface coordinate for an input 3D coordinate point and calculate the distance between the 
    3D point and the closest point of the surface. This function takes an input surface coordinate guess for, offering 
    a potential decrease in computation time compared to ProjPnt01.
    \code{.cpp}
    // Add Pod Geom
    string geom_id = AddGeom( "POD", "" );

    int surf_indx = 0;

    double u = 0.12345;
    double w = 0.67890;

    double d = 0;

    vec3d pnt = CompPnt01( geom_id, surf_indx, u, w );

    vec3d norm = CompNorm01( geom_id, surf_indx, u, w );

    double uout, wout;

    // Offset point from surface
    pnt = pnt + norm;

    d = ProjPnt01Guess( geom_id, surf_indx, pnt, u + 0.1, w + 0.1, uout, wout );

    Print( "Dist " + d + " u " + uout + " w " + wout );
    \endcode
    \sa ProjPnt01, ProjPnt01I
    \param [in] geom_id Parent Geom ID
    \param [in] surf_indx Main surface index from the parent Geom
    \param [in] pt Input 3D coordinate point
    \param [in] u0 Input U (0 - 1) surface coordinate guess
    \param [in] w0 Input W (0 - 1) surface coordinate guess
    \param [out] u Output closest U (0 - 1) surface coordinate
    \param [out] w Output closest W (0 - 1) surface coordinate
    \return Distance between the 3D point and the closest point of the surface
*/)";
    r = se->RegisterGlobalFunction( "double ProjPnt01Guess( const string & in geom_id, const int & in surf_indx, const vec3d & in pt, const double & in u0, const double & in w0, double & out u, double & out w )", asFUNCTION(vsp::ProjPnt01Guess), asCALL_CDECL, doc_struct );
    assert( r >= 0 );

    doc_struct.comment = R"(
/*!
    Project an input 3D coordinate point onto a surface along a specified axis.  If the axis-aligned ray from the point intersects the surface multiple times, the nearest intersection is returned.  If the axis-aligned ray from the point does not intersect the surface, the original point is returned and -1 is returned in the other output parameters.

    \code{.cpp}
    // Add Pod Geom
    string geom_id = AddGeom( "POD", "" );

    int surf_indx = 0;

    double u = 0.12345;
    double w = 0.67890;

    vec3d surf_pt = CompPnt01( geom_id, surf_indx, u, w );
    vec3d pt = surf_pt;

    pt.offset_y( -5.0 );

    double u_out, w_out;
    vec3d p_out;

    double idist = AxisProjPnt01( geom_id, surf_indx, Y_DIR, pt, u_out, w_out, p_out);

    Print( "iDist " + idist + " u_out " + u_out + " w_out " + w_out );
    Print( "3D Offset ", false);
    Print( surf_pt - p_out );
    \endcode
    \sa AxisProjPnt01Guess, AxisProjPnt01I, AxisProjVecPnt01, AxisProjVecPnt01Guess
    \param [in] geom_id Parent Geom ID
    \param [in] surf_indx Main surface index from the parent Geom
    \param [in] iaxis Axis direction to project point along (X_DIR, Y_DIR, or Z_DIR)
    \param [in] pt Input 3D coordinate point
    \param [out] u_out Output closest U (0 - 1) surface coordinate
    \param [out] w_out Output closest W (0 - 1) surface coordinate
    \param [out] p_out Output 3D coordinate point
    \return Axis aligned distance between the 3D point and the projected point on the surface
*/)";
    r = se->RegisterGlobalFunction( "double AxisProjPnt01( const string & in geom_id, const int & in surf_indx, const int & in iaxis, const vec3d & in pt, double & out u_out, double & out w_out, vec3d & out p_out )", asFUNCTION(vsp::AxisProjPnt01), asCALL_CDECL, doc_struct );
    assert( r >= 0 );

    doc_struct.comment = R"(
/*!
    Project an input 3D coordinate point onto a Geom along a specified axis.  The intersecting surface index is also returned.  If the axis-aligned ray from the point intersects the Geom multiple times, the nearest intersection is returned.  If the axis-aligned ray from the point does not intersect the Geom, the original point is returned and -1 is returned in the other output parameters.

    \code{.cpp}
    // Add Pod Geom
    string geom_id = AddGeom( "POD", "" );

    int surf_indx = 0;

    double u = 0.12345;
    double w = 0.67890;

    vec3d surf_pt = CompPnt01( geom_id, surf_indx, u, w );
    vec3d pt = surf_pt;

    pt.offset_y( -5.0 );

    double u_out, w_out;
    vec3d p_out;
    int surf_indx_out;

    double idist = AxisProjPnt01I( geom_id, Y_DIR, pt, surf_indx_out, u_out, w_out, p_out);

    Print( "iDist " + idist + " u_out " + u_out + " w_out " + w_out + " surf_index " + surf_indx_out );
    Print( "3D Offset ", false);
    Print( surf_pt - p_out );
    \endcode
    \sa AxisProjPnt01, AxisProjPnt01Guess, AxisProjVecPnt01, AxisProjVecPnt01Guess
    \param [in] geom_id Parent Geom ID
    \param [in] iaxis Axis direction to project point along (X_DIR, Y_DIR, or Z_DIR)
    \param [in] pt Input 3D coordinate point
    \param [out] surf_indx_out Output main surface index from the parent Geom
    \param [out] u_out Output closest U (0 - 1) surface coordinate
    \param [out] w_out Output closest W (0 - 1) surface coordinate
    \param [out] p_out Output 3D coordinate point
    \return Axis aligned distance between the 3D point and the projected point on the surface
*/)";
    r = se->RegisterGlobalFunction( "double AxisProjPnt01I( const string & in geom_id, const int & in iaxis, const vec3d & in pt, int & out surf_indx_out, double & out u_out, double & out w_out, vec3d & out p_out )", asFUNCTION(vsp::AxisProjPnt01I), asCALL_CDECL, doc_struct );
    assert( r >= 0 );

    doc_struct.comment = R"(
/*!
    Project an input 3D coordinate point onto a surface along a specified axis given an initial guess of surface parameter.  If the axis-aligned ray from the point intersects the surface multiple times, the nearest intersection is returned.  If the axis-aligned ray from the point does not intersect the surface, the original point is returned and -1 is returned in the other output parameters.  The surface parameter guess should allow this call to be faster than calling AxisProjPnt01 without a guess.

    \code{.cpp}
    // Add Pod Geom
    string geom_id = AddGeom( "POD", "" );

    int surf_indx = 0;

    double u = 0.12345;
    double w = 0.67890;

    

    vec3d surf_pt = CompPnt01( geom_id, surf_indx, u, w );
    vec3d pt = surf_pt;

    pt.offset_y( -5.0 );

    // Construct initial guesses near actual parameters
    double u0 = u + 0.01234;
    double w0 = w - 0.05678;

    double uout, wout;
    vec3d p_out;

    double d = AxisProjPnt01Guess( geom_id, surf_indx, Y_DIR, pt, u0, w0, uout, wout, p_out);

    Print( "Dist " + d + " u " + uout + " w " + wout );
    \endcode
    \sa AxisProjPnt01, AxisProjPnt01I, AxisProjVecPnt01, AxisProjVecPnt01Guess
    \param [in] geom_id Parent Geom ID
    \param [in] surf_indx Main surface index from the parent Geom
    \param [in] iaxis Axis direction to project point along (X_DIR, Y_DIR, or Z_DIR)
    \param [in] pt Input 3D coordinate point
    \param [in] u0 Input U (0 - 1) surface coordinate guess
    \param [in] w0 Input W (0 - 1) surface coordinate guess
    \param [out] u_out Output closest U (0 - 1) surface coordinate
    \param [out] w_out Output closest W (0 - 1) surface coordinate
    \param [out] p_out Output 3D coordinate point
    \return Distance between the 3D point and the closest point of the surface
*/)";
    r = se->RegisterGlobalFunction( "double AxisProjPnt01Guess( const string & in geom_id, const int & in surf_indx, const int & in iaxis, const vec3d & in pt, const double & in u0, const double & in w0, double & out u_out, double & out w_out, vec3d & out p_out )", asFUNCTION(vsp::AxisProjPnt01Guess), asCALL_CDECL, doc_struct );
    assert( r >= 0 );

    doc_struct.comment = R"(
/*!
    Test whether a given point is inside a specified surface.

    \code{.cpp}
    // Add Pod Geom
    string geom_id = AddGeom( "POD", "" );

    int surf_indx = 0;

    double r = 0.12;
    double s = 0.34;
    double t = 0.56;

    vec3d pnt = CompPntRST( geom_id, surf_indx, r, s, t );

    bool res = InsideSurf( geom_id, surf_indx, pt );

    if ( res )
    {
        print( "Inside" );
    }
    else
    {
        print( "Outside" );
    }

    \endcode
    \sa VecInsideSurf
    \param [in] geom_id Parent Geom ID
    \param [in] surf_indx Main surface index from the parent Geom
    \param [in] pt Input 3D coordinate point
    \return Boolean true if the point is inside the surface, false otherwise.
*/)";
    r = se->RegisterGlobalFunction( "bool InsideSurf( const string & in geom_id, const int & in surf_indx, const vec3d & in pt )", asFUNCTION(vsp::InsideSurf), asCALL_CDECL, doc_struct );
    assert( r >= 0 );

    doc_struct.comment = R"(
/*!
    Determine the nearest (R, S, T) volume coordinate for an input (X, Y, Z) 3D coordinate point and calculate the distance between the
    3D point and the found volume point.
    \code{.cpp}
    // Add Pod Geom
    string geom_id = AddGeom( "POD", "" );

    int surf_indx = 0;

    double r = 0.12;
    double s = 0.34;
    double t = 0.56;

    vec3d pnt = CompPntRST( geom_id, surf_indx, r, s, t );

    double rout, sout, tout;

    double d = FindRST( geom_id, surf_indx, pnt, rout, sout, tout );

    Print( "Dist " + d + " r " + rout + " s " + sout + " t " + tout );
    \endcode
    \sa FindRSTGuess
    \param [in] geom_id Parent Geom ID
    \param [in] surf_indx Main surface index from the parent Geom
    \param [in] pt Input 3D coordinate point
    \param [out] r Output closest R (0 - 1.0) volume coordinate
    \param [out] s Output closest S (0 - 0.5) volume coordinate
    \param [out] t Output closest T (0 - 1.0) volume coordinate
    \return Distance between the 3D point and the closest point of the volume
*/)";
    r = se->RegisterGlobalFunction( "double FindRST( const string & in geom_id, const int & in surf_indx, const vec3d & in pt, double & out r, double & out s, double & out t )", asFUNCTION(vsp::FindRST), asCALL_CDECL, doc_struct );
    assert( r >= 0 );

    doc_struct.comment = R"(
/*!
    Determine the nearest (R, S, T) volume coordinate for an input (X, Y, Z) 3D coordinate point given an initial guess of volume coordinates.  Also calculate the distance between the
    3D point and the found volume point.

    \code{.cpp}
    // Add Pod Geom
    string geom_id = AddGeom( "POD", "" );

    int surf_indx = 0;

    double r = 0.12;
    double s = 0.34;
    double t = 0.56;

    vec3d pnt = CompPntRST( geom_id, surf_indx, r, s, t );

    double rout, sout, tout;

    double r0 = 0.1;
    double s0 = 0.3;
    double t0 = 0.5;

    double d = FindRSTGuess( geom_id, surf_indx, pnt, r0, s0, t0, rout, sout, tout );

    Print( "Dist " + d + " r " + rout + " s " + sout + " t " + tout );
    \endcode
    \sa FindRST
    \param [in] geom_id Parent Geom ID
    \param [in] surf_indx Main surface index from the parent Geom
    \param [in] pt Input 3D coordinate point
    \param [in] r0 Input R (0 - 1.0) volume coordinate guess
    \param [in] s0 Input S (0 - 0.5) volume coordinate guess
    \param [in] t0 Input T (0 - 1.0) volume coordinate guess
    \param [out] r Output closest R (0 - 1.0) volume coordinate
    \param [out] s Output closest S (0 - 0.5) volume coordinate
    \param [out] t Output closest T (0 - 1.0) volume coordinate
    \return Distance between the 3D point and the closest point of the volume
*/)";
    r = se->RegisterGlobalFunction( "double FindRSTGuess( const string & in geom_id, const int & in surf_indx, const vec3d & in pt, const double & in r0, const double & in s0, const double & in t0, double & out r, double & out s, double & out t )", asFUNCTION(vsp::FindRSTGuess), asCALL_CDECL, doc_struct );
    assert( r >= 0 );

    doc_struct.comment = R"(
/*!
    Calculate the (X, Y, Z) coordinate for the input volume (R, S, T) coordinate point
    \code{.cpp}
    // Add Pod Geom
    string geom_id = AddGeom( "POD", "" );

    int surf_indx = 0;

    double r = 0.12;
    double s = 0.34;
    double t = 0.56;

    vec3d pnt = CompPntRST( geom_id, surf_indx, r, s, t );

    Print( "Point: ( " + pnt.x() + ', ' + pnt.y() + ', ' + pnt.z() + ' )' );
    \endcode
    \param [in] geom_id Parent Geom ID
    \param [in] surf_indx Main surface index from the parent Geom
    \param [in] r R (0 - 1) volume coordinate
    \param [in] s S (0 - 0.5) volume coordinate
    \param [in] t T (0 - 1) volume coordinate
    \return vec3d coordinate point
*/)";
    r = se->RegisterGlobalFunction( "vec3d CompPntRST( const string & in geom_id, const int & in surf_indx, const double & in r, const double & in s, const double & in t )", asFUNCTION(vsp::CompPntRST), asCALL_CDECL, doc_struct );
    assert( r >= 0 );

    doc_struct.comment = R"(
/*!
    Determine 3D coordinate for each volume coordinate point in the input arrays
    \code{.cpp}
    // Add Pod Geom
    string geom_id = AddGeom( "POD", "" );

    int n = 5;

    array<double> rvec, svec, tvec;

    rvec.resize( n );
    svec.resize( n );
    tvec.resize( n );

    for( int i = 0 ; i < n ; i++ )
    {
        rvec[i] = (i+1)*1.0/(n+1);

        svec[i] = (n-i)*0.5/(n+1);

        tvec[i] = (i+1)*1.0/(n+1);
    }

    array< vec3d > ptvec = CompVecPntRST( geom_id, 0, rvec, svec, tvec );
    \endcode
    \param [in] geom_id Parent Geom ID
    \param [in] surf_indx Main surface index from the parent Geom
    \param [in] rs Input array of R (0 - 1.0) volume coordinates
    \param [in] ss Input array of S (0 - 0.5) volume coordinates
    \param [in] ts Input array of T (0 - 1.0) volume coordinates
    \return Array of 3D coordinate points
*/)";
    r = se->RegisterGlobalFunction( "array<vec3d>@ CompVecPntRST(const string & in geom_id, const int & in surf_indx, array<double>@ rs, array<double>@ ss, array<double>@ ts )", asMETHOD( ScriptMgrSingleton, CompVecPntRST ), asCALL_THISCALL_ASGLOBAL, &ScriptMgr, doc_struct );
    assert( r >= 0 );

    doc_struct.comment = R"(
/*!
    Convert RST volumetric coordinates to LMN coordinates.
    \code{.cpp}
    // Add Pod Geom
    string geom_id = AddGeom( "POD", "" );

    int surf_indx = 0;

    double r = 0.12;
    double s = 0.34;
    double t = 0.56;
    double l, m, n;

    ConvertRSTtoLMN( geom_id, surf_indx, r, s, t, l, m, n );

    \endcode
    \param [in] geom_id Parent Geom ID
    \param [in] surf_indx Main surface index from the parent Geom
    \param [in] r R (0 - 1) volume coordinate
    \param [in] s S (0 - 0.5) volume coordinate
    \param [in] t T (0 - 1) volume coordinate
    \param [out] l L (0 - 1) linear volume coordinate
    \param [out] m M (0 - 1) linear volume coordinate
    \param [out] n N (0 - 1) linear volume coordinate
    \return void
*/)";
    r = se->RegisterGlobalFunction( "vec3d ConvertRSTtoLMN( const string & in geom_id, const int & in surf_indx, const double & in r, const double & in s, const double & in t, double & out l, double & out m, double & out n )", asFUNCTION(vsp::ConvertRSTtoLMN), asCALL_CDECL, doc_struct );
    assert( r >= 0 );

    doc_struct.comment = R"(
/*!
    Convert LMN volumetric coordinates to RST coordinates.
    \code{.cpp}
    // Add Pod Geom
    string geom_id = AddGeom( "POD", "" );

    int surf_indx = 0;

    double l = 0.12;
    double m = 0.34;
    double n = 0.56;
    double r, s, t;

    ConvertLMNtoRST( geom_id, surf_indx, l, m, n, r, s, t );

    \endcode
    \param [in] geom_id Parent Geom ID
    \param [in] surf_indx Main surface index from the parent Geom
    \param [in] l L (0 - 1) linear volume coordinate
    \param [in] m M (0 - 1) linear volume coordinate
    \param [in] n N (0 - 1) linear volume coordinate
    \param [out] r R (0 - 1) volume coordinate
    \param [out] s S (0 - 0.5) volume coordinate
    \param [out] t T (0 - 1) volume coordinate
    \return void
*/)";
    r = se->RegisterGlobalFunction( "vec3d ConvertLMNtoRST( const string & in geom_id, const int & in surf_indx, const double & in l, const double & in m, const double & in n, double & out r, double & out s, double & out t )", asFUNCTION(vsp::ConvertLMNtoRST), asCALL_CDECL, doc_struct );
    assert( r >= 0 );


    doc_struct.comment = R"(
/*!
    Convert vector of RST volumetric coordinates to LMN coordinates.

    \code{.cpp}
    // Add Pod Geom
    string geom_id = AddGeom( "POD", "" );

    int n = 5;

    array<double> rvec, svec, tvec;

    rvec.resize( n );
    svec.resize( n );
    tvec.resize( n );

    for( int i = 0 ; i < n ; i++ )
    {
        rvec[i] = (i+1)*1.0/(n+1);
        svec[i] = 0.5 * (n-i)*1.0/(n+1);
        tvec[i] = (i+1)*1.0/(n+1);
    }

    array<double> lvec, mvec, nvec;

    ConvertRSTtoLMNVec( geom_id, 0, rvec, svec, tvec, lvec, mvec, nvec );

    \endcode
    \sa ConvertLMNtoRSTVec, ConvertRSTtoLMN, ConvertLMNtoRST
    \param [in] geom_id Parent Geom ID
    \param [in] surf_indx Main surface index from the parent Geom
    \param [in] rs Input array of R (0 - 1) volumetric coordinate
    \param [in] ss Input array of S (0 - 0.5) volumetric coordinate
    \param [in] ts Input array of T (0 - 1) volumetric coordinate
    \param [out] ls Output array of L (0 - 1) linear volumetric coordinate
    \param [out] ms Output array of M (0 - 1) linear volumetric coordinate
    \param [out] ns Output array of N (0 - 1) linear volumetric coordinate
*/)";
    r = se->RegisterGlobalFunction( "void ConvertRSTtoLMNVec(const string & in geom_id, const int & in surf_indx, array<double>@ rs, array<double>@ ss, array<double>@ ts, array<double>@ ls, array<double>@ ms, array<double>@ ns )", asMETHOD( ScriptMgrSingleton, ConvertRSTtoLMNVec ), asCALL_THISCALL_ASGLOBAL, &ScriptMgr, doc_struct );
    assert( r >= 0 );

    doc_struct.comment = R"(
/*!
    Convert vector of LMN volumetric coordinates to RST coordinates.

    \code{.cpp}
    // Add Pod Geom
    string geom_id = AddGeom( "POD", "" );

    int n = 5;

    array<double> lvec, mvec, nvec;

    lvec.resize( n );
    mvec.resize( n );
    nvec.resize( n );

    for( int i = 0 ; i < n ; i++ )
    {
        lvec[i] = (i+1)*1.0/(n+1);
        mvec[i] = (n-i)*1.0/(n+1);
        nvec[i] = (i+1)*1.0/(n+1);
    }

    array<double> rvec, svec, tvec;

    ConvertLMNtoRSTVec( geom_id, 0, lvec, mvec, nvec, rvec, svec, tvec );

    \endcode
    \sa ConvertRSTtoLMNVec, ConvertRSTtoLMN, ConvertLMNtoRST
    \param [in] geom_id Parent Geom ID
    \param [in] surf_indx Main surface index from the parent Geom
    \param [in] rs Input array of R (0 - 1) volumetric coordinate
    \param [in] ss Input array of S (0 - 0.5) volumetric coordinate
    \param [in] ts Input array of T (0 - 1) volumetric coordinate
    \param [out] ls Output array of L (0 - 1) linear volumetric coordinate
    \param [out] ms Output array of M (0 - 1) linear volumetric coordinate
    \param [out] ns Output array of N (0 - 1) linear volumetric coordinate
*/)";
    r = se->RegisterGlobalFunction( "void ConvertLMNtoRSTVec(const string & in geom_id, const int & in surf_indx, array<double>@ ls, array<double>@ ms, array<double>@ ns, array<double>@ rs, array<double>@ ss, array<double>@ ts )", asMETHOD( ScriptMgrSingleton, ConvertLMNtoRSTVec ), asCALL_THISCALL_ASGLOBAL, &ScriptMgr, doc_struct );
    assert( r >= 0 );

    doc_struct.comment = R"(
/*!
    Get the surface coordinate point of each intersection of the tessellated wireframe for a particular surface
    \code{.cpp}
    // Add Pod Geom
    string geom_id = AddGeom( "POD", "" );

    int surf_indx = 0;

    array<double> utess, wtess;

    GetUWTess01( geom_id, surf_indx, utess, wtess );
    \endcode
    \param [in] geom_id Parent Geom ID
    \param [in] surf_indx Main surface index from the parent Geom
    \param [out] us Output array of U (0 - 1) surface coordinates
    \param [out] ws Output array of W (0 - 1) surface coordinates
*/)";
    r = se->RegisterGlobalFunction( "void GetUWTess01(const string & in geom_id, int & in surf_indx, array<double>@ us, array<double>@ ws )", asMETHOD( ScriptMgrSingleton, GetUWTess01 ), asCALL_THISCALL_ASGLOBAL, &ScriptMgr, doc_struct );
    assert( r >= 0 );

    doc_struct.comment = R"(
/*!
    Determine 3D coordinate for each surface coordinate point in the input arrays
    \code{.cpp}
    // Add Pod Geom
    string geom_id = AddGeom( "POD", "" );

    int n = 5;

    array<double> uvec, wvec;

    uvec.resize( n );
    wvec.resize( n );

    for( int i = 0 ; i < n ; i++ )
    {
        uvec[i] = (i+1)*1.0/(n+1);

        wvec[i] = (n-i)*1.0/(n+1);
    }

    array< vec3d > ptvec = CompVecPnt01( geom_id, 0, uvec, wvec );
    \endcode
    \param [in] geom_id Parent Geom ID
    \param [in] surf_indx Main surface index from the parent Geom
    \param [in] us Input array of U (0 - 1) surface coordinates
    \param [in] ws Input array of W (0 - 1) surface coordinates
    \return Array of 3D coordinate points
*/)";
    r = se->RegisterGlobalFunction( "array<vec3d>@ CompVecPnt01(const string & in geom_id, const int & in surf_indx, array<double>@ us, array<double>@ ws )", asMETHOD( ScriptMgrSingleton, CompVecPnt01 ), asCALL_THISCALL_ASGLOBAL, &ScriptMgr, doc_struct );
    assert( r >= 0 );

    doc_struct.comment = R"(
/*!
    Determine the normal vector on a surface for each surface coordinate point in the input arrays
    \code{.cpp}
    // Add Pod Geom
    string geom_id = AddGeom( "POD", "" );

    int n = 5;

    array<double> uvec, wvec;

    uvec.resize( n );
    wvec.resize( n );

    for( int i = 0 ; i < n ; i++ )
    {
        uvec[i] = (i+1)*1.0/(n+1);

        wvec[i] = (n-i)*1.0/(n+1);
    }

    array< vec3d > normvec = CompVecNorm01( geom_id, 0, uvec, wvec );
    \endcode
    \param [in] geom_id Parent Geom ID
    \param [in] surf_indx Main surface index from the parent Geom
    \param [in] us Input array of U (0 - 1) surface coordinates
    \param [in] ws Input array of W (0 - 1) surface coordinates
    \return Array of 3D normal vectors
*/)";
    r = se->RegisterGlobalFunction( "array<vec3d>@ CompVecNorm01(const string & in geom_id, const int & in surf_indx, array<double>@ us, array<double>@ws )", asMETHOD( ScriptMgrSingleton, CompVecNorm01 ), asCALL_THISCALL_ASGLOBAL, &ScriptMgr, doc_struct );
    assert( r >= 0 );

    doc_struct.comment = R"(
/*!
    Determine the curvature of a specified surface at each surface coordinate point in the input arrays
    \code{.cpp}
    // Add Pod Geom
    string geom_id = AddGeom( "POD", "" );

    int n = 5;

    array<double> uvec, wvec;

    uvec.resize( n );
    wvec.resize( n );

    for( int i = 0 ; i < n ; i++ )
    {
        uvec[i] = (i+1)*1.0/(n+1);

        wvec[i] = (n-i)*1.0/(n+1);
    }

    array<double> k1vec, k2vec, kavec, kgvec;

    CompVecCurvature01( geom_id, 0, uvec, wvec, k1vec, k2vec, kavec, kgvec );
    \endcode
    \param [in] geom_id Parent Geom ID
    \param [in] surf_indx Main surface index from the parent Geom
    \param [in] us Input array of U (0 - 1) surface coordinates
    \param [in] ws Input array of W (0 - 1) surface coordinates
    \param [out] k1s Output array of maximum principal curvatures
    \param [out] k2s Output array of minimum principal curvatures
    \param [out] kas Output array of mean curvatures
    \param [out] kgs Output array of Gaussian curvatures
*/)";
    r = se->RegisterGlobalFunction( "void CompVecCurvature01(const string & in geom_id, const int & in surf_indx, array<double>@ us, array<double>@ ws, array<double>@ k1s, array<double>@ k2s, array<double>@ kas, array<double>@ kgs)", asMETHOD( ScriptMgrSingleton, CompVecCurvature01 ), asCALL_THISCALL_ASGLOBAL, &ScriptMgr, doc_struct );
    assert( r >= 0 );

    doc_struct.comment = R"(
/*!
    Determine the nearest surface coordinates for an input array of 3D coordinate points and calculate the distance between each 
    3D point and the closest point of the surface.
    \code{.cpp}
    // Add Pod Geom
    string geom_id = AddGeom( "POD", "" );

    int n = 5;

    array<double> uvec, wvec;

    uvec.resize( n );
    wvec.resize( n );

    for( int i = 0 ; i < n ; i++ )
    {
        uvec[i] = (i+1)*1.0/(n+1);

        wvec[i] = (n-i)*1.0/(n+1);
    }

    array< vec3d > ptvec = CompVecPnt01( geom_id, 0, uvec, wvec );

    array< vec3d > normvec = CompVecNorm01( geom_id, 0, uvec, wvec );

    for( int i = 0 ; i < n ; i++ )
    {
        ptvec[i] = ptvec[i] + normvec[i];
    }

    array<double> uoutv, woutv, doutv;

    ProjVecPnt01( geom_id, 0, ptvec, uoutv, woutv, doutv );
    \endcode
    \sa ProjVecPnt01Guess
    \param [in] geom_id Parent Geom ID
    \param [in] surf_indx Main surface index from the parent Geom
    \param [in] pts Input array of 3D coordinate points
    \param [out] us Output array of the closest U (0 - 1) surface coordinate for each 3D input point
    \param [out] ws Output array of the closest W (0 - 1) surface coordinate for each 3D input point
    \param [out] ds Output array of distances for each 3D point and the closest point of the surface
*/)";
    r = se->RegisterGlobalFunction( "void ProjVecPnt01(const string & in geom_id, const int & in surf_indx, array<vec3d>@ pts, array<double>@ us, array<double>@ ws, array<double>@ ds )", asMETHOD( ScriptMgrSingleton, ProjVecPnt01 ), asCALL_THISCALL_ASGLOBAL, &ScriptMgr, doc_struct );
    assert( r >= 0 );

    doc_struct.comment = R"(
/*!
    Determine the nearest surface coordinates for an input array of 3D coordinate points and calculate the distance between each 
    3D point and the closest point of the surface. This function takes an input array of surface coordinate guesses for each 3D
    coordinate, offering a potential decrease in computation time compared to ProjVecPnt01.
    \code{.cpp}
    // Add Pod Geom
    string geom_id = AddGeom( "POD", "" );

    int n = 5;

    array<double> uvec, wvec;

    uvec.resize( n );
    wvec.resize( n );

    for( int i = 0 ; i < n ; i++ )
    {
        uvec[i] = (i+1)*1.0/(n+1);

        wvec[i] = (n-i)*1.0/(n+1);
    }

    array< vec3d > ptvec = CompVecPnt01( geom_id, 0, uvec, wvec );

    array< vec3d > normvec = CompVecNorm01( geom_id, 0, uvec, wvec );

    for( int i = 0 ; i < n ; i++ )
    {
        ptvec[i] = ptvec[i] + normvec[i];
    }

    array<double> uoutv, woutv, doutv, u0v, w0v;

    u0v.resize( n );
    w0v.resize( n );

    for( int i = 0 ; i < n ; i++ )
    {
        u0v[i] = uvec[i] + 0.01234;

        w0v[i] = wvec[i] - 0.05678;
    }

    ProjVecPnt01Guess( geom_id, 0, ptvec, u0v,  w0v,  uoutv, woutv, doutv );
    \endcode
    \sa ProjVecPnt01, 
    \param [in] geom_id Parent Geom ID
    \param [in] surf_indx Main surface index from the parent Geom
    \param [in] pts Input array of 3D coordinate points
    \param [in] u0s Input array of U (0 - 1) surface coordinate guesses
    \param [in] w0s Input array of W (0 - 1) surface coordinate guesses
    \param [out] us Output array of the closest U (0 - 1) surface coordinate for each 3D input point
    \param [out] ws Output array of the closest W (0 - 1) surface coordinate for each 3D input point
    \param [out] ds Output array of distances for each 3D point and the closest point of the surface
*/)";
    r = se->RegisterGlobalFunction( "void ProjVecPnt01Guess(const string & in geom_id, const int & in surf_indx, array<vec3d>@ pts, array<double>@ u0s, array<double>@ w0s, array<double>@ us, array<double>@ ws, array<double>@ ds )", asMETHOD( ScriptMgrSingleton, ProjVecPnt01Guess ), asCALL_THISCALL_ASGLOBAL, &ScriptMgr, doc_struct );
    assert( r >= 0 );

    doc_struct.comment = R"(
/*!
    Project an input array of 3D coordinate points onto a surface along a specified axis.  If the axis-aligned ray from the point intersects the surface multiple times, the nearest intersection is returned.  If the axis-aligned ray from the point does not intersect the surface, the original point is returned and -1 is returned in the other output parameters.


    \code{.cpp}
       // Add Pod Geom
    string geom_id = AddGeom( "POD", "" );
    int surf_indx = 0;

    int n = 5;

    array<double> uvec, wvec;

    uvec.resize( n );
    wvec.resize( n );

    for( int i = 0 ; i < n ; i++ )
    {
        uvec[i] = (i+1)*1.0/(n+1);

        wvec[i] = (n-i)*1.0/(n+1);
    }

    array< vec3d > ptvec = CompVecPnt01( geom_id, surf_indx, uvec, wvec );

    for( int i = 0 ; i < n ; i++ )
    {
        ptvec[i].offset_y( -5.0 );
    }

    array<double> uoutv, woutv, doutv;
    array< vec3d > poutv;

    AxisProjVecPnt01( geom_id, surf_indx, Y_DIR, ptvec, uoutv, woutv, poutv, doutv );

    // Some of these outputs are expected to be non-zero because the projected point is on the opposite side of
    // the pod from the originally computed point.  I.e. there were multiple solutions and the original point
    // is not the closest intersection point.  We could offset those points in the +Y direction instead of -Y.
    for( int i = 0 ; i < n ; i++ )
    {
        Print( i, false );
        Print( "U delta ", false );
        Print( uvec[i] - uoutv[i], false );
        Print( "W delta ", false );
        Print( wvec[i] - woutv[i] );
    }

    \endcode
    \sa AxisProjPnt01, AxisProjPnt01Guess, AxisProjPnt01I, AxisProjVecPnt01Guess
    \param [in] geom_id Parent Geom ID
    \param [in] surf_indx Main surface index from the parent Geom
    \param [in] iaxis Axis direction to project point along (X_DIR, Y_DIR, or Z_DIR)
    \param [in] pts Input array of 3D coordinate points
    \param [out] us Output array of the closest U (0 - 1) surface coordinate for each 3D input point
    \param [out] ws Output array of the closest W (0 - 1) surface coordinate for each 3D input point
    \param [out] ps_out Output array of 3D coordinate point
    \param [out] ds Output array of axis distances for each 3D point and the projected point of the surface
*/)";
    r = se->RegisterGlobalFunction( "void AxisProjVecPnt01(const string & in geom_id, const int & in surf_indx, const int & in iaxis, array<vec3d>@ pts, array<double>@ us, array<double>@ ws, array<vec3d>@ ps_out, array<double>@ ds )", asMETHOD( ScriptMgrSingleton, AxisProjVecPnt01 ), asCALL_THISCALL_ASGLOBAL, &ScriptMgr, doc_struct );
    assert( r >= 0 );

    doc_struct.comment = R"(
/*!
    Project an input array of 3D coordinate points onto a surface along a specified axis given initial guess arrays of surface parameter.  If the axis-aligned ray from the point intersects the surface multiple times, the nearest intersection is returned.  If the axis-aligned ray from the point does not intersect the surface, the original point is returned and -1 is returned in the other output parameters.  The surface parameter guess should allow this call to be faster than calling AxisProjVecPnt01 without a guess.
    \code{.cpp}
    // Add Pod Geom
    string geom_id = AddGeom( "POD", "" );
    int surf_indx = 0;

    int n = 5;

    array<double> uvec, wvec;

    uvec.resize( n );
    wvec.resize( n );

    for( int i = 0 ; i < n ; i++ )
    {
        uvec[i] = (i+1)*1.0/(n+1);

        wvec[i] = (n-i)*1.0/(n+1);
    }

    array< vec3d > ptvec = CompVecPnt01( geom_id, surf_indx, uvec, wvec );

    for( int i = 0 ; i < n ; i++ )
    {
        ptvec[i].offset_y( -5.0 );
    }

    array<double> uoutv, woutv, doutv, u0v, w0v;
    array< vec3d > poutv;

    u0v.resize( n );
    w0v.resize( n );

    for( int i = 0 ; i < n ; i++ )
    {
        u0v[i] = uvec[i] + 0.01234;
        w0v[i] = wvec[i] - 0.05678;
    }

    AxisProjVecPnt01Guess( geom_id, surf_indx, Y_DIR, ptvec, u0v,  w0v,  uoutv, woutv, poutv, doutv );

    for( int i = 0 ; i < n ; i++ )
    {
        Print( i, false );
        Print( "U delta ", false );
        Print( uvec[i] - uoutv[i], false );
        Print( "W delta ", false );
        Print( wvec[i] - woutv[i] );
    }

    \endcode
    \sa AxisProjPnt01, AxisProjPnt01Guess, AxisProjPnt01I, AxisProjVecPnt01
    \param [in] geom_id Parent Geom ID
    \param [in] surf_indx Main surface index from the parent Geom
    \param [in] iaxis Axis direction to project point along (X_DIR, Y_DIR, or Z_DIR)
    \param [in] pts Input array of 3D coordinate points
    \param [in] u0s Input array of U (0 - 1) surface coordinate guesses
    \param [in] w0s Input array of W (0 - 1) surface coordinate guesses
    \param [out] us Output array of the closest U (0 - 1) surface coordinate for each 3D input point
    \param [out] ws Output array of the closest W (0 - 1) surface coordinate for each 3D input point
    \param [out] ps_out Output array of 3D coordinate point
    \param [out] ds Output array of axis distances for each 3D point and the projected point of the surface
*/)";
    r = se->RegisterGlobalFunction( "void AxisProjVecPnt01Guess(const string & in geom_id, int & in surf_indx, const int & in iaxis, array<vec3d>@ pts, array<double>@ u0s, array<double>@ w0s, array<double>@ us, array<double>@ ws, array<vec3d>@ ps_out, array<double>@ ds )", asMETHOD( ScriptMgrSingleton, AxisProjVecPnt01Guess ), asCALL_THISCALL_ASGLOBAL, &ScriptMgr, doc_struct );
    assert( r >= 0 );

    doc_struct.comment = R"(
/*!
    Test whether a vector of points are inside a specified surface.

    \code{.cpp}
    // Add Pod Geom
    string geom_id = AddGeom( "POD", "" );

    int surf_indx = 0;

    int n = 5;

    array<double> rvec, svec, tvec;

    rvec.resize( n );
    svec.resize( n );
    tvec.resize( n );

    for( int i = 0 ; i < n ; i++ )
    {
        rvec[i] = (i+1)*1.0/(n+1);

        svec[i] = (n-i)*0.5/(n+1);

        tvec[i] = (i+1)*1.0/(n+1);
    }

    array< vec3d > ptvec = CompVecPntRST( geom_id, 0, rvec, svec, tvec );

    array<bool> res;
    res = VecInsideSurf( geom_id, surf_indx, ptvec );

    \endcode
    \sa VecInsideSurf
    \param [in] geom_id Parent Geom ID
    \param [in] surf_indx Main surface index from the parent Geom
    \param [in] pts Input array of 3D coordinate points
    \return Boolean vector for each point.  True if it is inside the surface, false otherwise.
*/)";
    r = se->RegisterGlobalFunction( "array<bool>@  VecInsideSurf( const string & in geom_id, const int & in surf_indx, array<vec3d>@ pts )", asFUNCTION(vsp::VecInsideSurf), asCALL_CDECL, doc_struct );
    assert( r >= 0 );

   doc_struct.comment = R"(
/*!
    Determine the nearest volume coordinates for an input array of 3D coordinate points and calculate the distance between each
    3D point and the found point in the volume.
    \code{.cpp}
    // Add Pod Geom
    string geom_id = AddGeom( "POD", "" );

    int n = 5;

    array<double> rvec, svec, tvec;

    rvec.resize( n );
    svec.resize( n );
    tvec.resize( n );

    for( int i = 0 ; i < n ; i++ )
    {
        rvec[i] = (i+1)*1.0/(n+1);

        svec[i] = (n-i)*0.5/(n+1);

        tvec[i] = (i+1)*1.0/(n+1);
    }

    array< vec3d > ptvec = CompVecPntRST( geom_id, 0, rvec, svec, tvec );

    array<double> routv, soutv, toutv, doutv;

    FindRSTVec( geom_id, 0, ptvec, routv, soutv, toutv, doutv );
    \endcode
    \sa FindRSTVecGuess
    \param [in] geom_id Parent Geom ID
    \param [in] surf_indx Main surface index from the parent Geom
    \param [in] pts Input array of 3D coordinate points
    \param [out] rs Output array of the closest R (0 - 1.0) volume coordinate for each 3D input point
    \param [out] ss Output array of the closest S (0 - 0.5) volume coordinate for each 3D input point
    \param [out] ts Output array of the closest T (0 - 1.0) volume coordinate for each 3D input point
    \param [out] ds Output array of distances for each 3D point and the closest point of the volume
*/)";
    r = se->RegisterGlobalFunction( "void FindRSTVec(const string & in geom_id, const int & in surf_indx, array<vec3d>@ pts, array<double>@ rs, array<double>@ ss, array<double>@ ts, array<double>@ ds )", asMETHOD( ScriptMgrSingleton, FindRSTVec ), asCALL_THISCALL_ASGLOBAL, &ScriptMgr, doc_struct );
    assert( r >= 0 );

    doc_struct.comment = R"(
/*!
    Determine the nearest volume coordinates for an input array of 3D coordinate points and calculate the distance between each
    3D point and the closest point of the volume. This function takes an input array of volume coordinate guesses for each 3D
    coordinate, offering a potential decrease in computation time compared to FindRSTVec.
    \code{.cpp}
    // Add Pod Geom
    string geom_id = AddGeom( "POD", "" );

    int n = 5;

    array<double> rvec, svec, tvec;

    rvec.resize( n );
    svec.resize( n );
    tvec.resize( n );

    for( int i = 0 ; i < n ; i++ )
    {
        rvec[i] = (i+1)*1.0/(n+1);

        svec[i] = (n-i)*0.5/(n+1);

        tvec[i] = (i+1)*1.0/(n+1);
    }

    array< vec3d > ptvec = CompVecPntRST( geom_id, 0, rvec, svec, tvec );

    array<double> routv, soutv, toutv, doutv;

    for( int i = 0 ; i < n ; i++ )
    {
        ptvec[i] = ptvec[i] * 0.9;
    }

    FindRSTVecGuess( geom_id, 0, ptvec, rvec, svec, tvec, routv, soutv, toutv, doutv );
    \endcode
    \sa FindRSTVec,
    \param [in] geom_id Parent Geom ID
    \param [in] surf_indx Main surface index from the parent Geom
    \param [in] pts Input array of 3D coordinate points
    \param [in] r0s Input array of U (0 - 1.0) volume coordinate guesses
    \param [in] s0s Input array of S (0 - 0.5) volume coordinate guesses
    \param [in] t0s Input array of T (0 - 1.0) volume coordinate guesses
    \param [out] rs Output array of the closest R (0 - 1.0) volume coordinate for each 3D input point
    \param [out] ss Output array of the closest S (0 - 0.5) volume coordinate for each 3D input point
    \param [out] ts Output array of the closest T (0 - 1.0) volume coordinate for each 3D input point
    \param [out] ds Output array of distances for each 3D point and the closest point of the volume
*/)";
    r = se->RegisterGlobalFunction( "void FindRSTVecGuess(const string & in geom_id, const int & in surf_indx, array<vec3d>@ pts, array<double>@ r0s, array<double>@ s0s, array<double>@ t0s, array<double>@ rs, array<double>@ ss, array<double>@ ts, array<double>@ ds )", asMETHOD( ScriptMgrSingleton, FindRSTVecGuess ), asCALL_THISCALL_ASGLOBAL, &ScriptMgr, doc_struct );
    assert( r >= 0 );

    //=== Register Measure Functions ===//
    group = "Measure";
    doc_struct.group = group.c_str();

    group_description = R"(
    \brief This group of API functions can be used to control the Ruler Tool through the API. \n\n
    \ref index "Click here to return to the main page" )";
    se->AddGroup( group.c_str(), "Measure Tool Functions", group_description.c_str() );

    doc_struct.comment = R"(
/*!
    Create a new Ruler and add it to the Measure Tool
    \code{.cpp}
    string pid1 = AddGeom( "POD", "" );

    SetParmVal( pid1, "Y_Rel_Location", "XForm", 2.0 );

    string pid2 = AddGeom( "POD", "" );

    SetParmVal( pid2, "Z_Rel_Location", "XForm", 4.0 );

    string rid = AddRuler( pid1, 1, 0.2, 0.3, pid2, 0, 0.2, 0.3, "Ruler 1" );

    SetParmVal( FindParm( rid, "X_Offset", "Measure" ), 6.0 );
    \endcode
    \param [in] startgeomid Start parent Geom ID
    \param [in] startsurfindx Main surface index from the staring parent Geom
    \param [in] startu Surface u (0 - 1) start coordinate
    \param [in] startw Surface w (0 - 1) start coordinate
    \param [in] endgeomid End parent Geom ID
    \param [in] endsurfindx Main surface index on the end parent Geom
    \param [in] endu Surface u (0 - 1) end coordinate
    \param [in] endw Surface w (0 - 1) end coordinate
    \param [in] name Ruler name
    \return Ruler ID
*/)";
    r = se->RegisterGlobalFunction( "string AddRuler( const string & in startgeomid, int startsurfindx, double startu, double startw, const string & in endgeomid, int endsurfindx, double endu, double endw, const string & in name )", asFUNCTION( vsp::AddRuler ), asCALL_CDECL, doc_struct );
    assert( r >= 0 );

    doc_struct.comment = R"(
/*!
    Get an array of all Ruler IDs from the Measure Tool
    \code{.cpp}
    string pid1 = AddGeom( "POD", "" );

    SetParmVal( pid1, "Y_Rel_Location", "XForm", 2.0 );

    string pid2 = AddGeom( "POD", "" );

    SetParmVal( pid2, "Z_Rel_Location", "XForm", 4.0 );

    string rid1 = AddRuler( pid1, 1, 0.2, 0.3, pid2, 0, 0.2, 0.3, "Ruler 1" );

    string rid2 = AddRuler( pid1, 0, 0.4, 0.6, pid1, 1, 0.8, 0.9, "Ruler 2" );

    array< string > @ruler_array = GetAllRulers();

    Print("Two Rulers");

    for( int n = 0 ; n < int( ruler_array.length() ) ; n++ )
    {
        Print( ruler_array[n] );
    }
    \endcode
    \return Array of Ruler IDs
*/)";
    r = se->RegisterGlobalFunction( "array<string>@ GetAllRulers()", asMETHOD( ScriptMgrSingleton, GetAllRulers ), asCALL_THISCALL_ASGLOBAL, &ScriptMgr, doc_struct);
    assert( r >= 0 );

    doc_struct.comment = R"(
/*!
    Delete a particular Ruler from the Meaure Tool
    \code{.cpp}
    string pid1 = AddGeom( "POD", "" );

    SetParmVal( pid1, "Y_Rel_Location", "XForm", 2.0 );

    string pid2 = AddGeom( "POD", "" );

    SetParmVal( pid2, "Z_Rel_Location", "XForm", 4.0 );

    string rid1 = AddRuler( pid1, 1, 0.2, 0.3, pid2, 0, 0.2, 0.3, "Ruler 1" );

    string rid2 = AddRuler( pid1, 0, 0.4, 0.6, pid1, 1, 0.8, 0.9, "Ruler 2" );

    array< string > @ruler_array = GetAllRulers();

    DelRuler( ruler_array[0] );
    \endcode
    \param [in] id Ruler ID
*/)";
    r = se->RegisterGlobalFunction( "void DelRuler( const string & in id )", asFUNCTION( vsp::DelRuler ), asCALL_CDECL, doc_struct);
    assert( r >= 0 );

    doc_struct.comment = R"(
/*!
    Delete all Rulers from the Meaure Tool
    \code{.cpp}
    string pid1 = AddGeom( "POD", "" );

    SetParmVal( pid1, "Y_Rel_Location", "XForm", 2.0 );

    string pid2 = AddGeom( "POD", "" );

    SetParmVal( pid2, "Z_Rel_Location", "XForm", 4.0 );

    string rid1 = AddRuler( pid1, 1, 0.2, 0.3, pid2, 0, 0.2, 0.3, "Ruler 1" );

    string rid2 = AddRuler( pid1, 0, 0.4, 0.6, pid1, 1, 0.8, 0.9, "Ruler 2" );

    DeleteAllRulers();
    \endcode
*/)";
    r = se->RegisterGlobalFunction( "void DeleteAllRulers()", asFUNCTION( vsp::DeleteAllRulers ), asCALL_CDECL, doc_struct);
    assert( r >= 0 );

    doc_struct.comment = R"(
/*!
    Create a new Probe and add it to the Measure Tool
    \code{.cpp}
    string pid1 = AddGeom( "POD", "" );

    SetParmVal( pid1, "Y_Rel_Location", "XForm", 2.0 );

    string probe_id = AddProbe( pid1, 0, 0.5, 0.8, "Probe 1" );

    SetParmVal( FindParm( probe_id, "Len", "Measure" ), 3.0 );
    \endcode
    \param [in] geomid Parent Geom ID
    \param [in] surfindx Main surface index from the parent Geom
    \param [in] u Surface u (0 - 1) coordinate
    \param [in] w Surface w (0 - 1) coordinate
    \param [in] name Probe name
    \return Probe ID
*/)";
    r = se->RegisterGlobalFunction( "string AddProbe( const string & in geomid, int surfindx, double u, double w, const string & in name )", asFUNCTION( vsp::AddProbe ), asCALL_CDECL, doc_struct);
    assert( r >= 0 );

    doc_struct.comment = R"(
/*!
    Get an array of all Probe IDs from the Measure Tool
    \code{.cpp}
    string pid1 = AddGeom( "POD", "" );

    SetParmVal( pid1, "Y_Rel_Location", "XForm", 2.0 );

    string probe_id = AddProbe( pid1, 0, 0.5, 0.8, "Probe 1" );

    array< string > @probe_array = GetAllProbes();

    Print( "One Probe: ", false );

    Print( probe_array[0] );
    \endcode
    \return [in] Array of Probe IDs
*/)";
    r = se->RegisterGlobalFunction( "array<string>@ GetAllProbes()", asMETHOD( ScriptMgrSingleton, GetAllProbes ), asCALL_THISCALL_ASGLOBAL, &ScriptMgr, doc_struct);
    assert( r >= 0 );

    doc_struct.comment = R"(
/*!
    Delete a specific Probe from the Measure Tool
    \code{.cpp}
    string pid1 = AddGeom( "POD", "" );

    SetParmVal( pid1, "Y_Rel_Location", "XForm", 2.0 );

    string probe_id_1 = AddProbe( pid1, 0, 0.5, 0.8, "Probe 1" );
    string probe_id_2 = AddProbe( pid1, 0, 0.2, 0.3, "Probe 2" );

    DelProbe( probe_id_1 );

    array< string > @probe_array = GetAllProbes();

    if ( probe_array.size() != 1 ) { Print( "Error: DelProbe" ); }
    \endcode
    \param [in] id Probe ID
*/)";
    r = se->RegisterGlobalFunction( "void DelProbe( const string & in id )", asFUNCTION( vsp::DelProbe ), asCALL_CDECL, doc_struct);
    assert( r >= 0 );

    doc_struct.comment = R"(
/*!
    Delete all Probes from the Measure Tool
    \code{.cpp}
    string pid1 = AddGeom( "POD", "" );

    SetParmVal( pid1, "Y_Rel_Location", "XForm", 2.0 );

    string probe_id_1 = AddProbe( pid1, 0, 0.5, 0.8, "Probe 1" );
    string probe_id_2 = AddProbe( pid1, 0, 0.2, 0.3, "Probe 2" );

    DeleteAllProbes();

    array< string > @probe_array = GetAllProbes();

    if ( probe_array.size() != 0 ) { Print( "Error: DeleteAllProbes" ); }
    \endcode
*/)";
    r = se->RegisterGlobalFunction( "void DeleteAllProbes()", asFUNCTION( vsp::DeleteAllProbes ), asCALL_CDECL, doc_struct);
    assert( r >= 0 );

    //=== Register FeaStructure and FEA Mesh Functions ====//
    group = "FEAMesh";
    doc_struct.group = group.c_str();

    group_description = R"(
    \brief The following group of API functions supports all functionality of the FEA Mesh Tool. Structures, FEA Parts, 
    materials, and properties can be defined and manipulated. Mesh and output file settings can be adjusted, and an FEA
    mesh can be generated. \n\n
    \ref index "Click here to return to the main page" )";
    se->AddGroup( group.c_str(), "FEA Mesh Functions", group_description.c_str() );

    doc_struct.comment = R"(
/*!
    Add an FEA Structure to a specified Geom
    \warning init_skin should ALWAYS be set to true. 
    \code{.cpp}
    //==== Add Pod Geometry ====//
    string pod_id = AddGeom( "POD" );

    //==== Add FeaStructure to Pod ====//
    int struct_ind = AddFeaStruct( pod_id );
    \endcode
    \param [in] geom_id Parent Geom ID
    \param [in] init_skin Flag to initialize the FEA Structure by creating an FEA Skin from the parent Geom's OML at surfindex
    \param [in] surfindex Main surface index for the FEA Structure
    \return FEA Structure index
*/)";
    r = se->RegisterGlobalFunction( "int AddFeaStruct( const string & in geom_id, bool init_skin = true, int surfindex = 0 )", asFUNCTION( vsp::AddFeaStruct ), asCALL_CDECL, doc_struct );
    assert( r >= 0 ); // TODO: Force init_skin to true always

    doc_struct.comment = R"(
/*!
    Delete an FEA Structure and all FEA Parts and FEA SubSurfaces associated with it
    \code{.cpp}
    //==== Add Pod Geometry ====//
    string pod_id = AddGeom( "POD" );

    //==== Add FeaStructure to Pod ====//
    int struct_ind_1 = AddFeaStruct( pod_id );

    int struct_ind_2 = AddFeaStruct( pod_id );

    DeleteFeaStruct( pod_id, struct_ind_1 );
    \endcode
    \param [in] geom_id Parent Geom ID
    \param [in] fea_struct_ind FEA Structure index
*/)";
    r = se->RegisterGlobalFunction( "void DeleteFeaStruct( const string & in geom_id, int fea_struct_ind )", asFUNCTION( vsp::DeleteFeaStruct ), asCALL_CDECL, doc_struct );
    assert( r >= 0 );

    doc_struct.comment = R"(
/*!
    Sets FeaMeshMgr m_FeaMeshStructIndex member using passed in index of a FeaStructure
    \code{.cpp}

    //==== Add Pod Geometry ====//
    string pod_id = AddGeom( "POD" );

    //==== Add FeaStructure to Pod ====//
    int struct_ind = AddFeaStruct( pod_id );

    SetFeaMeshStructIndex( struct_ind );

    if ( FindGeoms().size() != 0 ) { Print( "ERROR: VSPRenew" ); }
    \endcode
*/)";
    r = se->RegisterGlobalFunction( "void SetFeaMeshStructIndex( int struct_index )", asFUNCTION( vsp::SetFeaMeshStructIndex ), asCALL_CDECL, doc_struct);
    assert( r >= 0 );

    doc_struct.comment = R"(
/*!
    Get the ID of an FEA Structure
    \code{.cpp}
    //==== Add Pod Geometry ====//
    string pod_id = AddGeom( "POD" );

    //==== Add FeaStructure to Pod ====//
    int struct_ind = AddFeaStruct( pod_id );

    string struct_id = GetFeaStructID( pod_id, struct_ind );
    \endcode
    \param [in] geom_id Parent Geom ID
    \param [in] fea_struct_ind FEA Structure index
    \return FEA Structure ID
*/)";
    r = se->RegisterGlobalFunction( "string GetFeaStructID( const string & in geom_id, int fea_struct_ind )", asFUNCTION( vsp::GetFeaStructID ), asCALL_CDECL, doc_struct );
    assert( r >= 0 );

    doc_struct.comment = R"(
/*!
    Get the index of an FEA Structure in its Parent Geom's vector of Structures
    \code{.cpp}
    //==== Add Pod Geometry ====//
    string pod_id = AddGeom( "POD" );

    //==== Add FeaStructure to Pod ====//
    int struct_ind_1 = AddFeaStruct( pod_id );

    int struct_ind_2 = AddFeaStruct( pod_id );

    string struct_id_2 = GetFeaStructID( pod_id, struct_ind_2 );

    DeleteFeaStruct( pod_id, struct_ind_1 );

    int struct_ind_2_new = GetFeaStructIndex( struct_id_2 );
    \endcode
    \param [in] struct_id FEA Structure ID
    \return FEA Structure index
*/)";
    r = se->RegisterGlobalFunction( "int GetFeaStructIndex( const string & in struct_id )", asFUNCTION( vsp::GetFeaStructIndex ), asCALL_CDECL, doc_struct );
    assert( r >= 0 );

    doc_struct.comment = R"(
/*!
    Get the Parent Geom ID for an FEA Structure
    \code{.cpp}
    //==== Add Pod Geometry ====//
    string pod_id = AddGeom( "POD" );

    //==== Add FeaStructure to Pod ====//
    int struct_ind = AddFeaStruct( pod_id );

    string struct_id = GetFeaStructID( pod_id, struct_ind );

    //==== Get Parent Geom ID and Index ====//
    string parent_id = GetFeaStructParentGeomID( struct_id );
    \endcode
    \param [in] struct_id FEA Structure ID
    \return Parent Geom ID
*/)";
    r = se->RegisterGlobalFunction( "string GetFeaStructParentGeomID( const string & in struct_id )", asFUNCTION( vsp::GetFeaStructParentGeomID ), asCALL_CDECL, doc_struct );
    assert( r >= 0 );

    doc_struct.comment = R"(
/*!
    Get the name of an FEA Structure. The FEA Structure name functions as the the Parm Container name
    \code{.cpp}
    //==== Add Pod Geometry ====//
    string pod_id = AddGeom( "POD" );
    
    //==== Add FeaStructure to Pod ====//
    int struct_ind = AddFeaStruct( pod_id );

    //==== Get Structure Name ====// 
    string parm_container_name = GetFeaStructName( pod_id, struct_ind );

    string display_name = string("Current Structure Parm Container Name: ") + parm_container_name + string("\n");

    Print( display_name );
    \endcode
    \sa FindContainer, SetFeaStructName
    \param [in] geom_id Parent Geom ID
    \param [in] fea_struct_ind FEA Structure index
    \return Name for the FEA Structure
*/)";
    r = se->RegisterGlobalFunction( "string GetFeaStructName( const string & in geom_id, int fea_struct_ind )", asFUNCTION( vsp::GetFeaStructName ), asCALL_CDECL, doc_struct );
    assert( r >= 0 );

    doc_struct.comment = R"(
/*!
    Set the name of an FEA Structure
    \code{.cpp}
    //==== Add Pod Geometry ====//
    string pod_id = AddGeom( "POD" );
    
    //==== Add FeaStructure to Pod ====//
    int struct_ind = AddFeaStruct( pod_id );

    //==== Change the Structure Name ====// 
    SetFeaStructName( pod_id, struct_ind, "Example_Struct" );

    string parm_container_id = FindContainer( "Example_Struct", struct_ind );

    string display_id = string("New Structure Parm Container ID: ") + parm_container_id + string("\n");

    Print( display_id );
    \endcode
    \sa GetFeaStructName
    \param [in] geom_id Parent Geom ID
    \param [in] fea_struct_ind FEA Structure index
    \param [in] name New name for the FEA Structure
*/)";
    r = se->RegisterGlobalFunction( "void SetFeaStructName( const string & in geom_id, int fea_struct_ind, const string & in name )", asFUNCTION( vsp::SetFeaStructName ), asCALL_CDECL, doc_struct );
    assert( r >= 0 );

    doc_struct.comment = R"(
/*!
    Get the IDs of all FEA Structures in the vehicle
    \code{.cpp}
    //==== Add Geometries ====//
    string pod_id = AddGeom( "POD" );
    string wing_id = AddGeom( "WING" );
    
    //==== Add FeaStructures ====//
    int pod_struct_ind = AddFeaStruct( pod_id );
    int wing_struct_ind = AddFeaStruct( wing_id );

    array < string > struct_id_vec = GetFeaStructIDVec();
    \endcode
    \sa NumFeaStructures
    \return Array of FEA Structure IDs
*/)";
    r = se->RegisterGlobalFunction( "array<string>@ GetFeaStructIDVec()", asMETHOD( ScriptMgrSingleton, GetFeaStructIDVec ), asCALL_THISCALL_ASGLOBAL, &ScriptMgr, doc_struct );
    assert( r >= 0 );

    doc_struct.comment = R"(
/*!
    Set the name of an FEA Part
    \code{.cpp}
    //==== Add Pod Geometry ====//
    string pod_id = AddGeom( "POD" );

    //==== Add FeaStructure to Pod ====//
    int struct_ind = AddFeaStruct( pod_id );

    //==== Add Bulkead ====//
    string bulkhead_id = AddFeaPart( pod_id, struct_ind, FEA_SLICE );

    SetFeaPartName( bulkhead_id, "Bulkhead" );
    \endcode
    \sa GetFeaPartName
    \param [in] part_id FEA Part ID
    \param [in] name New name for the FEA Part
*/)";
    r = se->RegisterGlobalFunction( "void SetFeaPartName( const string & in part_id, const string & in name )", asFUNCTION( vsp::SetFeaPartName ), asCALL_CDECL, doc_struct );
    assert( r >= 0 );

    doc_struct.comment = R"(
/*!
    Set the value of a particular FEA Mesh option for the specified Structure. Note, FEA Mesh makes use of enums initially created for CFD Mesh 
    but not all CFD Mesh options are available for FEA Mesh.
    \code{.cpp}
    //==== Add Pod Geometry ====//
    string pod_id = AddGeom( "POD" );

    //==== Add FeaStructure to Pod ====//
    int struct_ind = AddFeaStruct( pod_id );

    //==== Adjust FeaMeshSettings ====//
    SetFeaMeshVal( pod_id, struct_ind, CFD_MAX_EDGE_LEN, 0.75 );

    SetFeaMeshVal( pod_id, struct_ind, CFD_MIN_EDGE_LEN, 0.2 );
    \endcode
    \sa CFD_CONTROL_TYPE
    \param [in] geom_id Parent Geom ID
    \param [in] fea_struct_ind FEA Structure index
    \param [in] type FEA Mesh option type enum (i.e. CFD_MAX_EDGE_LEN)
    \param [in] val Value the option is set to
*/)";
    r = se->RegisterGlobalFunction( "void SetFeaMeshVal( const string & in geom_id, int fea_struct_ind, int type, double val )", asFUNCTION( vsp::SetFeaMeshVal ), asCALL_CDECL, doc_struct );
    assert( r >= 0 );

    doc_struct.comment = R"(
/*!
    Set the name of a particular FEA Mesh output file for a specified Structure
    \code{.cpp}
    //==== Add Pod Geometry ====//
    string pod_id = AddGeom( "POD" );

    //==== Add FeaStructure to Pod ====//
    int struct_ind = AddFeaStruct( pod_id );

    string struct_id = GetFeaStructID( pod_id, struct_ind );

    //=== Set Export File Name ===//
    string export_name = "FEAMeshTest_calculix.dat";

    //==== Get Parent Geom ID and Index ====//
    string parent_id = GetFeaStructParentGeomID( struct_id ); // same as pod_id

    SetFeaMeshFileName( parent_id, struct_ind, FEA_CALCULIX_FILE_NAME, export_name );
    \endcode
    \param [in] geom_id Parent Geom ID
    \param [in] fea_struct_ind FEA Structure index
    \param [in] file_type FEA output file type enum (i.e. FEA_EXPORT_TYPE)
    \param [in] file_name Name for the output file
*/)";
    r = se->RegisterGlobalFunction( "void SetFeaMeshFileName( const string & in geom_id, int fea_struct_ind, int file_type, const string & in file_name )", asFUNCTION( vsp::SetFeaMeshFileName ), asCALL_CDECL, doc_struct );
    assert( r >= 0 );

    doc_struct.comment = R"(
/*!
    Compute an FEA Mesh for a Structure. Only a single output file can be generated with this function.
    \code{.cpp}
    //==== Add Pod Geometry ====//
    string pod_id = AddGeom( "POD" );

    //==== Add FeaStructure to Pod ====//
    int struct_ind = AddFeaStruct( pod_id );

    string struct_id = GetFeaStructID( pod_id, struct_ind );

    //==== Generate FEA Mesh and Export ====//
    Print( string( "--> Generating FeaMesh " ) );

    //==== Get Parent Geom ID and Index ====//
    string parent_id = GetFeaStructParentGeomID( struct_id ); // same as pod_id

    ComputeFeaMesh( parent_id, struct_ind, FEA_CALCULIX_FILE_NAME );
    // Could also call ComputeFeaMesh ( struct_id, FEA_CALCULIX_FILE_NAME );
    \endcode
    \sa SetFeaMeshFileName, FEA_EXPORT_TYPE
    \param [in] geom_id Parent Geom ID
    \param [in] fea_struct_ind FEA Structure index
    \param [in] file_type FEA output file type enum (i.e. FEA_EXPORT_TYPE)
*/)";
    r = se->RegisterGlobalFunction( "void ComputeFeaMesh( const string & in geom_id, int fea_struct_ind, int file_type )", asFUNCTIONPR( vsp::ComputeFeaMesh, ( const string & in, int, int ), void ), asCALL_CDECL, doc_struct );
    assert( r >= 0 );

    doc_struct.comment = R"(
/*!
    Compute an FEA Mesh for a Structure. Only a single output file can be generated with this function.
    \code{.cpp}
    //==== Add Pod Geometry ====//
    string pod_id = AddGeom( "POD" );

    //==== Add FeaStructure to Pod ====//
    int struct_ind = AddFeaStruct( pod_id );

    string struct_id = GetFeaStructID( pod_id, struct_ind );

    //==== Generate FEA Mesh and Export ====//
    Print( string( "--> Generating FeaMesh " ) );

    ComputeFeaMesh ( struct_id, FEA_CALCULIX_FILE_NAME ); 
    // Could also call ComputeFeaMesh( parent_id, struct_ind, FEA_CALCULIX_FILE_NAME );
    \endcode
    \sa SetFeaMeshFileName, FEA_EXPORT_TYPE
    \param [in] struct_id FEA Structure ID
    \param [in] file_type FEA output file type enum (i.e. FEA_EXPORT_TYPE)
*/)";
    r = se->RegisterGlobalFunction( "void ComputeFeaMesh( const string & in struct_id, int file_type )", asFUNCTIONPR( vsp::ComputeFeaMesh, ( const string & in, int ), void ), asCALL_CDECL, doc_struct );
    assert( r >= 0 );

    doc_struct.comment = R"(
/*!
    Add an FEA Part to a Structure
    \code{.cpp}
    //==== Add Pod Geometry ====//
    string pod_id = AddGeom( "POD" );

    //==== Add FeaStructure to Pod ====//
    int struct_ind = AddFeaStruct( pod_id );

    //==== Add Bulkead ====//
    string bulkhead_id = AddFeaPart( pod_id, struct_ind, FEA_SLICE );

    SetParmVal( FindParm( bulkhead_id, "IncludedElements", "FeaPart" ), FEA_SHELL_AND_BEAM );

    SetParmVal( FindParm( bulkhead_id, "RelCenterLocation", "FeaPart" ), 0.15 );
    \endcode
    \sa FEA_PART_TYPE
    \param [in] geom_id Parent Geom ID
    \param [in] fea_struct_ind FEA Structure index
    \param [in] type FEA Part type enum (i.e. FEA_RIB)
    \return FEA Part ID
*/)";
    r = se->RegisterGlobalFunction( "string AddFeaPart( const string & in geom_id, int fea_struct_ind, int type )", asFUNCTION( vsp::AddFeaPart ), asCALL_CDECL, doc_struct );
    assert( r >= 0 );

    doc_struct.comment = R"(
/*!
    Delete an FEA Part from a Structure
    \code{.cpp}
    //==== Add Pod Geometry ====//
    string pod_id = AddGeom( "POD" );

    //==== Add FeaStructure to Pod ====//
    int struct_ind = AddFeaStruct( pod_id );

    //==== Add Bulkead ====//
    string bulkhead_id = AddFeaPart( pod_id, struct_ind, FEA_SLICE );

    //==== Add Fixed Point ====//
    string fixed_id = AddFeaPart( pod_id, struct_ind, FEA_FIX_POINT );

    //==== Delete Bulkead ====//
    DeleteFeaPart( pod_id, struct_ind, bulkhead_id );
    \endcode
    \param [in] geom_id Parent Geom ID
    \param [in] fea_struct_ind FEA Structure index
    \param [in] part_id FEA Part ID
*/)";
    r = se->RegisterGlobalFunction( "void DeleteFeaPart( const string & in geom_id, int fea_struct_ind, const string & in part_id )", asFUNCTION( vsp::DeleteFeaPart ), asCALL_CDECL, doc_struct );
    assert( r >= 0 );

    doc_struct.comment = R"(
/*!
    Get the Parm ID of an FEA Part, identified from a FEA Structure Parm ID and FEA Part index.
    \code{.cpp}
    //==== Add Pod Geometry ====//
    string pod_id = AddGeom( "POD" );

    //==== Add FeaStructure to Pod ====//
    int struct_ind = AddFeaStruct( pod_id );

    string struct_id = GetFeaStructID( pod_id, struct_ind );

    //==== Add Bulkead ====//
    string bulkhead_id = AddFeaPart( pod_id, struct_ind, FEA_SLICE );

    Update();

    if ( bulkhead_id != GetFeaPartID( struct_id, 1 ) ) // These should be equivalent (index 0 is skin)
    {
        Print( "Error: GetFeaPartID" );
    }
    \endcode
    \param [in] fea_struct_id FEA Structure ID
    \param [in] fea_part_index FEA Part index
    \return FEA Part ID
*/)";
    r = se->RegisterGlobalFunction( "string GetFeaPartID( const string & in fea_struct_id, int fea_part_index )", asFUNCTION( vsp::GetFeaPartID ), asCALL_CDECL, doc_struct );
    assert( r >= 0 );

    doc_struct.comment = R"(
/*!
    Get the name of an FEA Part
    \code{.cpp}
    //==== Add Fuselage Geometry ====//
    string fuse_id = AddGeom( "FUSELAGE" );

    //==== Add FeaStructure to Pod ====//
    int struct_ind = AddFeaStruct( fuse_id );

    //==== Add Bulkead ====//
    string bulkhead_id = AddFeaPart( fuse_id, struct_ind, FEA_SLICE );

    string name = "example_name";
    SetFeaPartName( bulkhead_id, name );

    if ( name != GetFeaPartName( bulkhead_id ) ) // These should be equivalent
    {
        Print( "Error: GetFeaPartName" );
    }
    \endcode
    \sa SetFeaPartName
    \param [in] part_id FEA Part ID
    \return FEA Part name
*/)";
    r = se->RegisterGlobalFunction( "string GetFeaPartName( const string & in part_id )", asFUNCTION( vsp::GetFeaPartName ), asCALL_CDECL, doc_struct );
    assert( r >= 0 );

    doc_struct.comment = R"(
/*!
    Get the type of an FEA Part
    \code{.cpp}
    //==== Add Pod Geometry ====//
    string pod_id = AddGeom( "POD" );

    //==== Add FeaStructure to Pod ====//
    int struct_ind = AddFeaStruct( pod_id );

    //==== Add Slice ====//
    string slice_id = AddFeaPart( pod_id, struct_ind, FEA_SLICE );

    if ( FEA_SLICE != GetFeaPartType( slice_id ) ) // These should be equivalent
    {
        Print( "Error: GetFeaPartType" );
    }
    \endcode
    \sa FEA_PART_TYPE
    \param [in] part_id FEA Part ID
    \return FEA Part type enum
*/)";
    r = se->RegisterGlobalFunction( "int GetFeaPartType( const string & in part_id )", asFUNCTION( vsp::GetFeaPartType ), asCALL_CDECL, doc_struct );
    assert( r >= 0 );

    doc_struct.comment = R"(
/*!
    Get the index of an FEA SubSurface give the SubSurface ID
    \code{.cpp}
    //==== Add Pod Geometry ====//
    string pod_id = AddGeom( "POD" );

    //==== Add FeaStructure to Pod ====//
    int struct_ind = AddFeaStruct( pod_id );

    //==== Add Slice ====//
    string slice_id = AddFeaPart( pod_id, struct_ind, FEA_SLICE );

    //==== Add LineArray ====//
    string line_array_id = AddFeaSubSurf( pod_id, struct_ind, SS_LINE_ARRAY );
    
    //==== Add Rectangle ====//
    string rect_id = AddFeaSubSurf( pod_id, struct_ind, SS_RECTANGLE );

    if ( 1 != GetFeaSubSurfIndex( rect_id ) ) // These should be equivalent
    {
        Print( "Error: GetFeaSubSurfIndex" );
    }
    \endcode
    \param [in] ss_id FEA SubSurface ID
    \return FEA SubSurface Index
*/)";
    r = se->RegisterGlobalFunction( "int GetFeaSubSurfIndex( const string & in ss_id )", asFUNCTION( vsp::GetFeaSubSurfIndex ), asCALL_CDECL, doc_struct );
    assert( r >= 0 );

    doc_struct.comment = R"(
/*!
    Get the total number of FEA Subsurfaces in the vehicle
    \code{.cpp}
    //==== Add Pod Geometry ====//
    string wing_id = AddGeom( "WING" );

    //==== Add FeaStructure to Pod ====//
    int struct_1 = AddFeaStruct( wing_id );
    int struct_2 = AddFeaStruct( wing_id );

    if ( NumFeaStructures() != 2 )
    {
        Print( "Error: NumFeaStructures" );
    }
    \endcode
    \sa GetFeaStructIDVec
    \return Total Number of FEA Structures
*/)";
    r = se->RegisterGlobalFunction( "int NumFeaStructures()", asFUNCTION( vsp::NumFeaStructures ), asCALL_CDECL, doc_struct );
    assert( r >= 0 );

    doc_struct.comment = R"(
/*!
    Get the number of FEA Parts for a particular FEA Structure
    \code{.cpp}
    //==== Add Pod Geometry ====//
    string pod_id = AddGeom( "POD" );

    //==== Add FeaStructure to Pod ====//
    int struct_ind = AddFeaStruct( pod_id );

    string struct_id = GetFeaStructID( pod_id, struct_ind );

    //==== Add FEA Parts ====//
    string slice_id = AddFeaPart( pod_id, struct_ind, FEA_SLICE );
    string dome_id = AddFeaPart( pod_id, struct_ind, FEA_DOME );

    if ( NumFeaParts( struct_id ) != 3 ) // Includes FeaSkin
    {
        Print( "Error: NumFeaParts" );
    }
    \endcode
    \sa GetFeaPartIDVec
    \param [in] fea_struct_id FEA Structure ID
    \return Number of FEA Parts
*/)";
    r = se->RegisterGlobalFunction( "int NumFeaParts( const string & in fea_struct_id )", asFUNCTION( vsp::NumFeaParts ), asCALL_CDECL, doc_struct );
    assert( r >= 0 );

    doc_struct.comment = R"(
/*!
    Get the number of FEA Subsurfaces for a particular FEA Structure
    \code{.cpp}
    //==== Add Pod Geometry ====//
    string wing_id = AddGeom( "WING" );

    //==== Add FeaStructure to Pod ====//
    int struct_ind = AddFeaStruct( wing_id );

    string struct_id = GetFeaStructID( wing_id, struct_ind );

    //==== Add SubSurfaces ====//
    string line_array_id = AddFeaSubSurf( wing_id, struct_ind, SS_LINE_ARRAY );
    string rectangle_id = AddFeaSubSurf( wing_id, struct_ind, SS_RECTANGLE );

    if ( NumFeaSubSurfs( struct_id ) != 2 )
    {
        Print( "Error: NumFeaSubSurfs" );
    }
    \endcode
    \sa GetFeaSubSurfIDVec
    \param [in] fea_struct_id FEA Structure ID
    \return Number of FEA SubSurfaces
*/)";
    r = se->RegisterGlobalFunction( "int NumFeaSubSurfs( const string & in fea_struct_id )", asFUNCTION( vsp::NumFeaSubSurfs ), asCALL_CDECL, doc_struct );
    assert( r >= 0 );

    doc_struct.comment = R"(
/*!
    Get the IDs of all FEA Parts in the given FEA Structure
    \code{.cpp}
    //==== Add Geometries ====//
    string pod_id = AddGeom( "POD" );

    //==== Add FeaStructure to Pod ====//
    int struct_ind = AddFeaStruct( pod_id );
    
    string struct_id = GetFeaStructID( pod_id, struct_ind );

    //==== Add FEA Parts ====//
    string slice_id = AddFeaPart( pod_id, struct_ind, FEA_SLICE );
    string dome_id = AddFeaPart( pod_id, struct_ind, FEA_DOME );

    array < string > part_id_vec = GetFeaPartIDVec( struct_id ); // Should include slice_id & dome_id
    \endcode
    \sa NumFeaParts
    \param [in] fea_struct_id FEA Structure ID
    \return Array of FEA Part IDs
*/)";
    r = se->RegisterGlobalFunction( "array<string>@ GetFeaPartIDVec(const string & in fea_struct_id)", asMETHOD( ScriptMgrSingleton, GetFeaPartIDVec ), asCALL_THISCALL_ASGLOBAL, &ScriptMgr, doc_struct );
    assert( r >= 0 );

    doc_struct.comment = R"(
/*!
    Get the IDs of all FEA SubSurfaces in the given FEA Structure
    \code{.cpp}
    //==== Add Geometries ====//
    string pod_id = AddGeom( "POD" );

    //==== Add FeaStructure to Pod ====//
    int struct_ind = AddFeaStruct( pod_id );

    string struct_id = GetFeaStructID( pod_id, struct_ind );

    //==== Add SubSurfaces ====//
    string line_array_id = AddFeaSubSurf( pod_id, struct_ind, SS_LINE_ARRAY );
    string rectangle_id = AddFeaSubSurf( pod_id, struct_ind, SS_RECTANGLE );

    array < string > part_id_vec = GetFeaSubSurfIDVec( struct_id ); // Should include line_array_id & rectangle_id
    \endcode
    \sa NumFeaSubSurfs
    \param [in] fea_struct_id FEA Structure ID
    \return Array of FEA Part IDs
*/)";
    r = se->RegisterGlobalFunction( "array<string>@ GetFeaSubSurfIDVec(const string & in fea_struct_id)", asMETHOD( ScriptMgrSingleton, GetFeaSubSurfIDVec ), asCALL_THISCALL_ASGLOBAL, &ScriptMgr, doc_struct );
    assert( r >= 0 );

    doc_struct.comment = R"(
/*!
    Set the ID of the perpendicular spar for an FEA Rib or Rib Array. Note, the FEA Rib or Rib Array should have "SPAR_NORMAL"
    set for the "PerpendicularEdgeType" Parm. If it is not, the ID will still be set, but the orientation of the Rib or Rib
    Array will not change.
    \code{.cpp}
    //==== Add Wing Geometry ====//
    string wing_id = AddGeom( "WING" );

    //==== Add FeaStructure to Wing ====//
    int struct_ind = AddFeaStruct( wing_id );

    //==== Add Rib ====//
    string rib_id = AddFeaPart( wing_id, struct_ind, FEA_RIB );

    //==== Add Spars ====//
    string spar_id_1 = AddFeaPart( wing_id, struct_ind, FEA_SPAR );
    string spar_id_2 = AddFeaPart( wing_id, struct_ind, FEA_SPAR );

    SetParmVal( FindParm( spar_id_1, "RelCenterLocation", "FeaPart" ), 0.25 );
    SetParmVal( FindParm( spar_id_2, "RelCenterLocation", "FeaPart" ), 0.75 );

    //==== Set Perpendicular Edge type to SPAR ====//
    SetParmVal( FindParm( rib_id, "PerpendicularEdgeType", "FeaRib" ), SPAR_NORMAL );

    SetFeaPartPerpendicularSparID( rib_id, spar_id_2 );

    if ( spar_id_2 != GetFeaPartPerpendicularSparID( rib_id ) )
    {
        Print( "Error: SetFeaPartPerpendicularSparID" );
    }
    \endcode
    \sa FEA_RIB_NORMAL, GetFeaPartPerpendicularSparID
    \param [in] part_id FEA Part ID (Rib or Rib Array Type)
    \param [in] perpendicular_spar_id FEA Spar ID
*/)";
    r = se->RegisterGlobalFunction( "void SetFeaPartPerpendicularSparID( const string & in part_id, const string & in perpendicular_spar_id )", asFUNCTION( vsp::SetFeaPartPerpendicularSparID ), asCALL_CDECL, doc_struct );
    assert( r >= 0 );

    doc_struct.comment = R"(
/*!
    Get the ID of the perpendicular spar for an FEA Rib or Rib Array. Note, the FEA Rib or Rib Array doesn't have to have "SPAR_NORMAL"
    set for the "PerpendicularEdgeType" Parm for this function to still return a value. 
    \code{.cpp}
    //==== Add Wing Geometry ====//
    string wing_id = AddGeom( "WING" );

    //==== Add FeaStructure to Wing ====//
    int struct_ind = AddFeaStruct( wing_id );

    //==== Add Rib ====//
    string rib_id = AddFeaPart( wing_id, struct_ind, FEA_RIB );

    //==== Add Spars ====//
    string spar_id_1 = AddFeaPart( wing_id, struct_ind, FEA_SPAR );
    string spar_id_2 = AddFeaPart( wing_id, struct_ind, FEA_SPAR );

    SetParmVal( FindParm( spar_id_1, "RelCenterLocation", "FeaPart" ), 0.25 );
    SetParmVal( FindParm( spar_id_2, "RelCenterLocation", "FeaPart" ), 0.75 );

    //==== Set Perpendicular Edge type to SPAR ====//
    SetParmVal( FindParm( rib_id, "PerpendicularEdgeType", "FeaRib" ), SPAR_NORMAL );

    SetFeaPartPerpendicularSparID( rib_id, spar_id_2 );

    if ( spar_id_2 != GetFeaPartPerpendicularSparID( rib_id ) )
    {
        Print( "Error: GetFeaPartPerpendicularSparID" );
    }
    \endcode
    \sa FEA_RIB_NORMAL, SetFeaPartPerpendicularSparID
    \param [in] part_id FEA Part ID (Rib or Rib Array Type)
    \return Perpendicular FEA Spar ID
*/)";
    r = se->RegisterGlobalFunction( "string GetFeaPartPerpendicularSparID( const string & in part_id )", asFUNCTION( vsp::GetFeaPartPerpendicularSparID ), asCALL_CDECL, doc_struct );
    assert( r >= 0 );

    doc_struct.comment = R"(
/*!
    Add an FEA SubSurface to a Structure
    \code{.cpp}
    //==== Add Pod Geometry ====//
    string pod_id = AddGeom( "POD" );

    //==== Add FeaStructure to Pod ====//
    int struct_ind = AddFeaStruct( pod_id );

    //==== Add LineArray ====//
    string line_array_id = AddFeaSubSurf( pod_id, struct_ind, SS_LINE_ARRAY );

    SetParmVal( FindParm( line_array_id, "ConstLineType", "SS_LineArray" ), 1 ); // Constant W

    SetParmVal( FindParm( line_array_id, "Spacing", "SS_LineArray" ), 0.25 );
    \endcode
    \sa SUBSURF_TYPE
    \param [in] geom_id Parent Geom ID
    \param [in] fea_struct_ind FEA Structure index
    \param [in] type FEA SubSurface type enum (i.e. SS_ELLIPSE)
    \return FEA SubSurface ID
*/)";
    r = se->RegisterGlobalFunction( "string AddFeaSubSurf( const string & in geom_id, int fea_struct_ind, int type )", asFUNCTION( vsp::AddFeaSubSurf ), asCALL_CDECL, doc_struct );
    assert( r >= 0 );

    doc_struct.comment = R"(
/*!
    Delete an FEA SubSurface from a Structure
    \code{.cpp}
    //==== Add Pod Geometry ====//
    string pod_id = AddGeom( "POD" );

    //==== Add FeaStructure to Pod ====//
    int struct_ind = AddFeaStruct( pod_id );

    //==== Add LineArray ====//
    string line_array_id = AddFeaSubSurf( pod_id, struct_ind, SS_LINE_ARRAY );

    //==== Add Rectangle ====//
    string rect_id = AddFeaSubSurf( pod_id, struct_ind, SS_RECTANGLE );

    //==== Delete LineArray ====//
    DeleteFeaSubSurf( pod_id, struct_ind, line_array_id );
    \endcode
    \param [in] geom_id Parent Geom ID
    \param [in] fea_struct_ind FEA Structure index
    \param [in] ss_id FEA SubSurface ID
*/)";
    r = se->RegisterGlobalFunction( "void DeleteFeaSubSurf( const string & in geom_id, int fea_struct_ind, const string & in ss_id )", asFUNCTION( vsp::DeleteFeaSubSurf ), asCALL_CDECL, doc_struct );
    assert( r >= 0 );

    doc_struct.comment = R"(
/*!
    Add an FEA Material the FEA Mesh material library. Materials are available across all Geoms and Structures. 
    \code{.cpp}
    //==== Create FeaMaterial ====//
    string mat_id = AddFeaMaterial();

    SetParmVal( FindParm( mat_id, "MassDensity", "FeaMaterial" ), 0.016 );
    \endcode
    \return FEA Material ID
*/)";
    r = se->RegisterGlobalFunction( "string AddFeaMaterial()", asFUNCTION( vsp::AddFeaMaterial ), asCALL_CDECL, doc_struct );
    assert( r >= 0 );

    doc_struct.comment = R"(
/*!
    Add aa FEA Property the FEA Mesh property library. Properties are available across all Geoms and Structures. Currently only beam and 
    shell properties are available. Note FEA_SHELL_AND_BEAM is not a valid property type.
    \code{.cpp}
    //==== Create FeaProperty ====//
    string prop_id = AddFeaProperty();

    SetParmVal( FindParm( prop_id, "Thickness", "FeaProperty" ), 0.01 );
    \endcode
    \sa FEA_PART_ELEMENT_TYPE
    \param [in] property_type FEA Property type enum (i.e. FEA_SHELL). 
    \return FEA Property ID
*/)";
    r = se->RegisterGlobalFunction( "string AddFeaProperty( int property_type = 0 )", asFUNCTION( vsp::AddFeaProperty ), asCALL_CDECL, doc_struct );
    assert( r >= 0 );
}

void ScriptMgrSingleton::RegisterUtility( asIScriptEngine* se )
{
    //==== Register Utility Functions ====//
    int r;
    asDocInfo doc_struct;
    string group = "APIUtilities";
    doc_struct.group = group.c_str();

    string group_description = R"(
    \brief This group of functions is provided for general API utilities, such as printing to stdout, performing
    basic math functions, and identifying basic OpenVSP information. \n\n
    \ref index "Click here to return to the main page" )";
    se->AddGroup( group.c_str(), "General API Utility Functions", group_description.c_str() );

    doc_struct.comment = R"(
/*!
    Print a string to stdout
    \code{.cpp}
    Print( string( "This is an example of printing a string to stdout." ) );
    \endcode
    \param [in] data Value of string type
    \param [in] new_line Flag to append a new line escape sequence to data
*/)";
    r = se->RegisterGlobalFunction( "void Print(const string & in data, bool new_line = true )", asMETHODPR( ScriptMgrSingleton, Print, (const string &, bool), void ), asCALL_THISCALL_ASGLOBAL, &ScriptMgr, doc_struct );
    assert( r >= 0 );

    doc_struct.comment = R"(
/*!
    Print a vec3d to stdout
    \code{.cpp}
    Print( vec3d( 1, 2, 3 ) );
    \endcode
    \param [in] data Value of vec3d type
    \param [in] new_line Flag to append a new line escape sequence to data
*/)";
    r = se->RegisterGlobalFunction( "void Print(const vec3d & in data, bool new_line = true )", asMETHODPR( ScriptMgrSingleton, Print, (const vec3d &, bool), void ), asCALL_THISCALL_ASGLOBAL, &ScriptMgr, doc_struct );
    assert( r >= 0 );

    doc_struct.comment = R"(
/*!
    Print a double to stdout
    \code{.cpp}
    Print( 12.34 );
    \endcode
    \param [in] data Value of double type
    \param [in] new_line Flag to append a new line escape sequence to data
*/)";
    r = se->RegisterGlobalFunction( "void Print(double data, bool new_line = true )", asMETHODPR( ScriptMgrSingleton, Print, (double, bool), void ), asCALL_THISCALL_ASGLOBAL, &ScriptMgr, doc_struct );
    assert( r >= 0 );

    doc_struct.comment = R"(
/*!
    Print an integer to stdout
    \code{.cpp}
    Print( 12 );
    \endcode
    \param [in] data integer value
    \param [in] new_line flag to append a new line escape sequence to data
*/)";
    r = se->RegisterGlobalFunction( "void Print(int data, bool new_line = true )", asMETHODPR( ScriptMgrSingleton, Print, (int, bool), void ), asCALL_THISCALL_ASGLOBAL, &ScriptMgr, doc_struct );
    assert( r >= 0 );

    doc_struct.comment = R"(
/*!
    Get the minimum of two values -> (x < y ) ? x : y
    \code{.cpp}
    double min_val = Min( 1.0, 2.0 );

    if ( abs( 1.0 - min_val ) > 1e-6 ) { Print( "Error: Min" ); }
    \endcode
    \param [in] x First value
    \param [in] y Second value
    \return Smallest of x and y
*/)";
    r = se->RegisterGlobalFunction( "double Min( double x, double y)", asMETHOD( ScriptMgrSingleton, Min ), asCALL_THISCALL_ASGLOBAL, &ScriptMgr, doc_struct);
    assert( r >= 0 );

    doc_struct.comment = R"(
/*!
    Get the maximum of two values -> (x > y ) ? x : y
    \code{.cpp}
    double max_val = Max( 1.0, 2.0 );

    if ( abs( 2.0 - max_val ) > 1e-6 ) { Print( "Error: Max" ); }
    \endcode
    \param [in] x First value
    \param [in] y Second value
    \return Largest of x and y
*/)";
    r = se->RegisterGlobalFunction( "double Max( double x, double y)", asMETHOD( ScriptMgrSingleton, Max ), asCALL_THISCALL_ASGLOBAL, &ScriptMgr, doc_struct);
    assert( r >= 0 );

    doc_struct.comment = R"(
/*!
    Convert radians to degrees
    \code{.cpp}
    double rad = 0.785; // radians

    double deg = Rad2Deg( rad ); // degrees
    \endcode
    \param [in] r Value in radians
    \return Value in degrees
*/)";
    r = se->RegisterGlobalFunction( "double Rad2Deg( double r )", asMETHOD( ScriptMgrSingleton, Rad2Deg ), asCALL_THISCALL_ASGLOBAL, &ScriptMgr, doc_struct);
    assert( r >= 0 );

    doc_struct.comment = R"(
/*!
    Convert degrees to radians
    \code{.cpp}
    double deg = 45; // degrees

    double rad = Deg2Rad( deg ); // radians
    \endcode
    \param [in] d Value in degrees
    \return Value in radians
*/)";
    r = se->RegisterGlobalFunction( "double Deg2Rad( double d )", asMETHOD( ScriptMgrSingleton, Deg2Rad ), asCALL_THISCALL_ASGLOBAL, &ScriptMgr, doc_struct);
    assert( r >= 0 );

    doc_struct.comment = R"(
/*!
    Get the version of the OpenVSP instance currently running
    \code{.cpp}
    Print( "The current OpenVSP version is: ", false );

    Print( GetVSPVersion() );
    \endcode
    \return OpenVSP version string (i.e. "OpenVSP 3.17.1")
*/)";
    r = se->RegisterGlobalFunction( "string GetVSPVersion( )", asFUNCTION( vsp::GetVSPVersion ), asCALL_CDECL, doc_struct);
    assert( r >= 0 );

    doc_struct.comment = R"(
/*!
    Get the path to the OpenVSP executable. OpenVSP will assume that the VSPAERO, VSPSLICER, and VSPVIEWER are in the same directory unless 
    instructed otherwise.
    \code{.cpp}
    Print( "The current VSP executable path is: ", false );

    Print( GetVSPExePath() );
    \endcode
    \sa SetVSPAEROPath, CheckForVSPAERO, GetVSPAEROPath
    \return Path to the OpenVSP executable
*/)";
    r = se->RegisterGlobalFunction( "string GetVSPExePath()", asFUNCTION( vsp::GetVSPExePath ), asCALL_CDECL, doc_struct);
    assert( r >= 0 );

    doc_struct.comment = R"(
/*!
    Set the path to the VSPAERO executables (Solver, Viewer, and Slicer). By default, OpenVSP will assume that the VSPAERO executables are in the
    same directory as the VSP executable. However, this may need to be changed when using certain API languages like MATLAB and Python. For example,
    Python may treat the location of the Python executable as the VSP executable path, so either the VSPAERO executable needs to be moved to the same
    directory or this function can be called to tell Python where to look for VSPAERO. 
    \code{.cpp}
    if ( !CheckForVSPAERO( GetVSPExePath() ) )
    {
        string vspaero_path = "C:/Users/example_user/Documents/OpenVSP_3.4.5";
        SetVSPAEROPath( vspaero_path );
    }
    \endcode
    \sa GetVSPExePath, CheckForVSPAERO, GetVSPAEROPath
    \param [in] path Absolute path to directory containing VSPAERO executable
    \return Flag that indicates whether or not the path was set correctly
*/)";
    r = se->RegisterGlobalFunction( "bool SetVSPAEROPath( const string & in path )", asFUNCTION( vsp::SetVSPAEROPath ), asCALL_CDECL, doc_struct );
    assert( r >= 0 );

    doc_struct.comment = R"(
/*!
    Get the path that OpenVSP will use to look for all VSPAERO executables (Solver, Slicer, and Viewer) when attempting to execute
    VSPAERO. If the VSPAERO executables are not in this location, they must either be copied there or the VSPAERO path must be set
    using SetVSPAEROPath.
    \code{.cpp}
    if ( !CheckForVSPAERO( GetVSPAEROPath() ) )
    {
        Print( "VSPAERO is not where OpenVSP thinks it is. I should move the VSPAERO executable or call SetVSPAEROPath." );
    }
    \endcode
    \sa GetVSPExePath, CheckForVSPAERO, SetVSPAEROPath
    \return Path OpenVSP will look for VSPAERO
*/)";
    r = se->RegisterGlobalFunction( "string GetVSPAEROPath()", asFUNCTION( vsp::GetVSPAEROPath ), asCALL_CDECL, doc_struct );
    assert( r >= 0 );

    doc_struct.comment = R"(
/*!
    Check if all VSPAERO executables (Solver, Viewer, and Slicer) are in a given directory. Note that this function will return false 
    if only one or two VSPAERO executables are found. An error message will indicate the executables that are missing. This may be 
    acceptable, as only the Solver is needed in all cases. The Viewer and Slicer may not be needed. 
    \code{.cpp}
    string vspaero_path = "C:/Users/example_user/Documents/OpenVSP_3.4.5";

    if ( CheckForVSPAERO( vspaero_path ) )
    {
        SetVSPAEROPath( vspaero_path );
    }
    \endcode
    \sa GetVSPExePath, GetVSPAEROPath, SetVSPAEROPath
    \param [in] path Absolute path to check for VSPAERO executables
    \return Flag that indicates if all VSPAERO executables are found or not
*/)";
    r = se->RegisterGlobalFunction( "bool CheckForVSPAERO( const string & in path )", asFUNCTION( vsp::CheckForVSPAERO ), asCALL_CDECL, doc_struct );
    assert( r >= 0 );

    doc_struct.comment = R"(
/*!
    Check if OpenVSP has been initialized successfully. If not, the OpenVSP instance will be exited. This call should be placed at the 
    beginning of all API scripts.
    \code{.cpp}
    
    VSPCheckSetup();

    // Continue to do things...
    
    \endcode
*/)";
    r = se->RegisterGlobalFunction( "void VSPCheckSetup()", asFUNCTION( vsp::VSPCheckSetup ), asCALL_CDECL, doc_struct);
    assert( r >= 0 );

    doc_struct.comment = R"(
/*!
    Clear and reinitialize OpenVSP to all default settings
    \code{.cpp}
    //==== Add Pod Geometry ====//
    string pod_id = AddGeom( "POD" );

    SetParmVal( pod_id, "Y_Rel_Location", "XForm", 2.0 );

    VSPRenew();

    if ( FindGeoms().size() != 0 ) { Print( "ERROR: VSPRenew" ); }
    \endcode
*/)";
    r = se->RegisterGlobalFunction( "void VSPRenew()", asFUNCTION( vsp::VSPRenew ), asCALL_CDECL, doc_struct);
    assert( r >= 0 );

    //====  Register Proxy Utility Functions ====//
    group = "ProxyUtitity";
    doc_struct.group = group.c_str();

    group_description = R"(
    \brief The API functions defined in this group enable conversion between AngelScript and OpenVSP C++ data types, 
    such as array and vector. \n\n
    \ref index "Click here to return to the main page" )";
    se->AddGroup( group.c_str(), "API Proxy Utility Functions", group_description.c_str() );

    doc_struct.comment = R"(
/*!
    Get a vector < vec3d > from a CScriptArray. Functions such as this are required to convert between AngelScript array and C++ std::vector.
    \code{.cpp}
    //==== Get Vector of Vec3d From App =====//
    array< vec3d > @vec3d_array = GetProxyVec3dArray();
    \endcode
    \return C++ type vector of vec3d objects
*/)";
    r = se->RegisterGlobalFunction( "array<vec3d>@ GetProxyVec3dArray()", asMETHOD( ScriptMgrSingleton, GetProxyVec3dArray ), asCALL_THISCALL_ASGLOBAL, &ScriptMgr, doc_struct );
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
