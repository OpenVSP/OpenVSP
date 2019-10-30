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
//////////////////////////////////////////////////////////////////////

#include "ScriptMgr.h"

#include "Parm.h"
#include "Matrix.h"
#include "VSP_Geom_API.h"
#include "CustomGeom.h"
#include "AdvLinkMgr.h"
#include "Vehicle.h"
#include "StringUtil.h"
#include "FileUtil.h"

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

    //==== Cache Some Commom Types ====//
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
void ScriptMgrSingleton::ReadExecuteScriptFile( const string &  file_name, const string &  function_name )
{
    string module_name = ReadScriptFromFile( "ReadExecute", file_name );

    ExecuteScript( module_name.c_str(), function_name.c_str() );
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

    //==== Make Sure Not Dupicate Of Any Other Module ====//
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
bool ScriptMgrSingleton::ExecuteScript(  const char* module_name,  const char* function_name, bool arg_flag, double arg )
{
    int r;

    // Find the function that is to be called.
    asIScriptModule *mod = m_ScriptEngine->GetModule( module_name );

    if ( !mod )
    {
        printf( "Error ExecuteScript GetModule %s\n", module_name );
        return false;
    }

    asIScriptFunction *func = mod->GetFunctionByDecl( function_name );
    if( func == 0 )
    {
        return false;
    }

    // Create our context, prepare it, and then execute
    asIScriptContext *ctx = m_ScriptEngine->CreateContext();
    ctx->Prepare( func );
    if ( arg_flag )
    {
        ctx->SetArgDouble( 0, arg );
    }
    r = ctx->Execute();
    if( r != asEXECUTION_FINISHED )
    {
        // The execution didn't complete as expected. Determine what happened.
        if( r == asEXECUTION_EXCEPTION )
        {
            // An exception occurred, let the script writer know what happened so it can be corrected.
            printf( "An exception '%s' occurred \n", ctx->GetExceptionString() );
        }
        return false;
    }
    return true;
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
    r = se->RegisterEnumValue( "ATMOS_TYPE", "ATMOS_TYPE_mANUAL_P_T", ATMOS_TYPE_MANUAL_P_T, "/*!< Manual: pressure and temperature control */" );
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
    r = se->RegisterEnumValue( "CFD_CONTROL_TYPE", "CFD_FAR_LOC_MAN_FLAG", CFD_FAR_LOC_MAN_FLAG, "/*!< Far field locaion flag: centered or manual */" );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "CFD_CONTROL_TYPE", "CFD_FAR_LOC_X", CFD_FAR_LOC_X, "/*!< Far field X location */" );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "CFD_CONTROL_TYPE", "CFD_FAR_LOC_Y", CFD_FAR_LOC_Y, "/*!< Far field Y location */" );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "CFD_CONTROL_TYPE", "CFD_FAR_LOC_Z", CFD_FAR_LOC_Z, "/*!< Far field Z location */" );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "CFD_CONTROL_TYPE", "CFD_WAKE_SCALE", CFD_WAKE_SCALE, "/*!< Wake length scale */" );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "CFD_CONTROL_TYPE", "CFD_WAKE_ANGLE", CFD_WAKE_ANGLE, "/*!< Wake angle */" );
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
    r = se->RegisterEnumValue( "COMPUTATION_FILE_TYPE", "DRAG_BUILD_TSV_TYPE", DRAG_BUILD_TSV_TYPE, "/*!< Comp Geom TSV file type */" );
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
    r = se->RegisterEnumValue( "FEA_EXPORT_TYPE", "FEA_NUM_FILE_NAMES", FEA_NUM_FILE_NAMES, "/*!< Number of FEA Mesh export type. */" );
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

//    comment_str = R"(
///*!
//    EditCurveEXec initialization types
//*/)";
    r = se->RegisterEnum( "INIT_EDIT_XSEC_TYPE" );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "INIT_EDIT_XSEC_TYPE", "EDIT_XSEC_CIRCLE", EDIT_XSEC_CIRCLE );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "INIT_EDIT_XSEC_TYPE", "EDIT_XSEC_ELLIPSE", EDIT_XSEC_ELLIPSE );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "INIT_EDIT_XSEC_TYPE", "EDIT_XSEC_RECTANGLE", EDIT_XSEC_RECTANGLE );
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

    doc_struct.comment = "/*! Enum for Surface Intersection export file types. */";

    r = se->RegisterEnum( "INTERSECT_EXPORT_TYPE", doc_struct );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "INTERSECT_EXPORT_TYPE", "INTERSECT_SRF_FILE_NAME", INTERSECT_SRF_FILE_NAME, "/*!< SRF intersection file type */" );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "INTERSECT_EXPORT_TYPE", "INTERSECT_CURV_FILE_NAME", INTERSECT_CURV_FILE_NAME, "/*!< CURV intersection file type */" );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "INTERSECT_EXPORT_TYPE", "INTERSECT_PLOT3D_FILE_NAME", INTERSECT_PLOT3D_FILE_NAME, "/*!< PLOT3D intersection file type */" );
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
    r = se->RegisterEnumValue( "SET_TYPE", "SET_ALL", SET_ALL, "/*!< All set */" );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "SET_TYPE", "SET_SHOWN", SET_SHOWN, "/*!< Shown set */" );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "SET_TYPE", "SET_NOT_SHOWN", SET_NOT_SHOWN, "/*!< Not Shown set */" );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "SET_TYPE", "SET_FIRST_USER", SET_FIRST_USER, "/*!< 1st user named set */" );
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

    r = se->RegisterEnum( "SYM_XSEC_TYPE" );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "SYM_XSEC_TYPE", "SYM_NONE", SYM_NONE );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "SYM_XSEC_TYPE", "SYM_RL", SYM_RL );

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
    r = se->RegisterEnumValue( "VSPAERO_STABILITY_TYPE", "STABILITY_DEFAULT", STABILITY_DEFAULT, "/*!< Default stability analysis */" );
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

    r = se->RegisterObjectMethod( "vec3d", "void rotate_x(double cos_alpha, double sin_alpha)", asMETHOD( vec3d, rotate_x ), asCALL_THISCALL );
    assert( r >= 0 );
    r = se->RegisterObjectMethod( "vec3d", "void rotate_y(double cos_alpha, double sin_alpha)", asMETHOD( vec3d, rotate_y ), asCALL_THISCALL );
    assert( r >= 0 );
    r = se->RegisterObjectMethod( "vec3d", "void rotate_z(double cos_alpha, double sin_alpha)", asMETHOD( vec3d, rotate_z ), asCALL_THISCALL );
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
    r = se->RegisterObjectMethod( "vec3d", "void rotate_z_zero_x(double cos_alpha, double sin_alpha)", asMETHOD( vec3d, rotate_z_zero_x ), asCALL_THISCALL );
    assert( r >= 0 );
    r = se->RegisterObjectMethod( "vec3d", "void rotate_z_zero_y(double cos_alpha, double sin_alpha)", asMETHOD( vec3d, rotate_z_zero_y ), asCALL_THISCALL );
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
    r = se->RegisterObjectBehaviour( "Matrix4d", asBEHAVE_CONSTRUCT, "void f()", asFUNCTION( Matrix4dDefaultConstructor ), asCALL_CDECL_OBJLAST );
    assert( r >= 0 );

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

    r = se->RegisterObjectMethod( "Matrix4d", "void buildXForm( const vec3d & in pos, const vec3d & in rot, const vec3d & in cent_rot )", asMETHOD( Matrix4d, buildXForm ), asCALL_THISCALL );
    assert( r >= 0 );

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
    assert( r );
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
    assert( r );
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
    assert( r );

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
    r = se->RegisterGlobalFunction( "string AppendCustomXSec( const string & in xsec_surf_id, int type )",
                                    asMETHOD( CustomGeomMgrSingleton, AppendCustomXSec ), asCALL_THISCALL_ASGLOBAL, &CustomGeomMgr );
    assert( r );
    r = se->RegisterGlobalFunction( "void CutCustomXSec( const string & in xsec_surf_id, int index )",
                                    asMETHOD( CustomGeomMgrSingleton, CutCustomXSec ), asCALL_THISCALL_ASGLOBAL, &CustomGeomMgr );
    assert( r );
    r = se->RegisterGlobalFunction( "void CopyCustomXSec( const string & in xsec_surf_id, int index )",
                                    asMETHOD( CustomGeomMgrSingleton, CopyCustomXSec ), asCALL_THISCALL_ASGLOBAL, &CustomGeomMgr );
    assert( r );
    r = se->RegisterGlobalFunction( "void PasteCustomXSec( const string & in xsec_surf_id, int index )",
                                    asMETHOD( CustomGeomMgrSingleton, PasteCustomXSec ), asCALL_THISCALL_ASGLOBAL, &CustomGeomMgr );
    assert( r );
    r = se->RegisterGlobalFunction( "string InsertCustomXSec( const string & in xsec_surf_id, int type, int index )",
                                    asMETHOD( CustomGeomMgrSingleton, InsertCustomXSec ), asCALL_THISCALL_ASGLOBAL, &CustomGeomMgr );
    assert( r );


}



//==== Register Adv Link Mgr Object ====//
void ScriptMgrSingleton::RegisterAdvLinkMgr( asIScriptEngine* se )
{
    int r;
    r = se->RegisterGlobalFunction( "void AddInput( const string & in geom_name, int geom_index, const string & in parm_name, const string & in parm_group, const string & in var_name )",
                                    asMETHOD( AdvLinkMgrSingleton, AddInput ), asCALL_THISCALL_ASGLOBAL, &AdvLinkMgr );
    assert( r );
    r = se->RegisterGlobalFunction( "void AddOutput( const string & in geom_name, int geom_index, const string & in parm_name, const string & in parm_group, const string & in var_name )",
                                    asMETHOD( AdvLinkMgrSingleton, AddOutput ), asCALL_THISCALL_ASGLOBAL, &AdvLinkMgr );
    assert( r );
    r = se->RegisterGlobalFunction( "void SetVar( const string & in var_name, double val )", asMETHOD( AdvLinkMgrSingleton, SetVar ), asCALL_THISCALL_ASGLOBAL, &AdvLinkMgr );
    assert( r );
    r = se->RegisterGlobalFunction( "double GetVar( const string & in var_name )", asMETHOD( AdvLinkMgrSingleton, GetVar ), asCALL_THISCALL_ASGLOBAL, &AdvLinkMgr );
    assert( r );
}

//==== Register API E Functions ====//
void ScriptMgrSingleton::RegisterAPIErrorObj( asIScriptEngine* se )
{
    //==== Register vec3d Object =====//
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

    //==== API Error Fucntions ====//
    r = se->RegisterGlobalFunction( "bool GetErrorLastCallFlag()", asMETHOD( vsp::ErrorMgrSingleton, GetErrorLastCallFlag ), asCALL_THISCALL_ASGLOBAL, &vsp::ErrorMgr );
    assert( r );
    r = se->RegisterGlobalFunction( "int GetNumTotalErrors()", asMETHOD( vsp::ErrorMgrSingleton, GetNumTotalErrors ), asCALL_THISCALL_ASGLOBAL, &vsp::ErrorMgr );
    assert( r );
    r = se->RegisterGlobalFunction( "ErrorObj PopLastError()", asMETHOD( vsp::ErrorMgrSingleton, PopLastError ), asCALL_THISCALL_ASGLOBAL, &vsp::ErrorMgr );
    assert( r );
    r = se->RegisterGlobalFunction( "ErrorObj GetLastError()", asMETHOD( vsp::ErrorMgrSingleton, GetLastError ), asCALL_THISCALL_ASGLOBAL, &vsp::ErrorMgr );
    assert( r );

    //==== Visualization Functions ====//
    r = se->RegisterGlobalFunction( "void ScreenGrab( const string & in file_name, int w, int h, bool transparentBG )", asFUNCTION( vsp::ScreenGrab ), asCALL_CDECL );
    assert( r >= 0 );
    r = se->RegisterGlobalFunction( "void SetViewAxis( bool vaxis )", asFUNCTION( vsp::SetViewAxis ), asCALL_CDECL );
    assert( r >= 0 );
    r = se->RegisterGlobalFunction( "void SetShowBorders( bool brdr )", asFUNCTION( vsp::SetShowBorders ), asCALL_CDECL );
    assert( r >= 0 );
    r = se->RegisterGlobalFunction("void SetGeomDrawType( const string & in geom_id, int type )", asFUNCTION(vsp::SetGeomDrawType), asCALL_CDECL);
    assert( r >= 0 );
    r = se->RegisterGlobalFunction("void SetGeomDisplayType( const string & in geom_id, int type )", asFUNCTION(vsp::SetGeomDisplayType), asCALL_CDECL);
    assert( r >= 0 );
    r = se->RegisterGlobalFunction( "void SetBackground( double r, double g, double b )", asFUNCTION( vsp::SetBackground ), asCALL_CDECL );
    assert( r >= 0 );

    //==== Vehicle Functions ====//
    r = se->RegisterGlobalFunction( "void Update()", asFUNCTION( vsp::Update ), asCALL_CDECL );
    assert( r >= 0 );
    r = se->RegisterGlobalFunction( "void VSPExit( int error_code )", asFUNCTION( vsp::VSPExit ), asCALL_CDECL );
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
    assert( r >= 0 );
    r = se->RegisterGlobalFunction( "void ExportFile( const string & in file_name, int write_set_index, int file_type )", asFUNCTION( vsp::ExportFile ), asCALL_CDECL );
    assert( r >= 0 );
    r = se->RegisterGlobalFunction( "string ImportFile( const string & in file_name, int file_type, const string & in parent )", asFUNCTION( vsp::ImportFile ), asCALL_CDECL );
    assert( r >= 0 );

    //==== Design File Functions ====//
    r = se->RegisterGlobalFunction( "void ReadApplyDESFile( const string & in file_name )", asFUNCTION( vsp::ReadApplyDESFile ), asCALL_CDECL );
    assert( r >= 0 );
    r = se->RegisterGlobalFunction( "void WriteDESFile( const string & in file_name )", asFUNCTION( vsp::WriteDESFile ), asCALL_CDECL );
    assert( r >= 0 );
    r = se->RegisterGlobalFunction( "void ReadApplyXDDMFile( const string & in file_name )", asFUNCTION( vsp::ReadApplyXDDMFile ), asCALL_CDECL );
    assert( r >= 0 );
    r = se->RegisterGlobalFunction( "void WriteXDDMFile( const string & in file_name )", asFUNCTION( vsp::WriteXDDMFile ), asCALL_CDECL );
    assert( r >= 0 );
    r = se->RegisterGlobalFunction( "int GetNumDesignVars()", asFUNCTION( vsp::GetNumDesignVars ), asCALL_CDECL );
    assert( r >= 0 );
    r = se->RegisterGlobalFunction( "void AddDesignVar( const string & in parm_id, int type )", asFUNCTION( vsp::AddDesignVar ), asCALL_CDECL );
    assert( r >= 0 );
    r = se->RegisterGlobalFunction( "void DeleteAllDesignVars()", asFUNCTION( vsp::DeleteAllDesignVars ), asCALL_CDECL );
    assert( r >= 0 );
    r = se->RegisterGlobalFunction( "string GetDesignVar( int index )", asFUNCTION( vsp::GetDesignVar ), asCALL_CDECL );
    assert( r >= 0 );
    r = se->RegisterGlobalFunction( "int GetDesignVarType( int index )", asFUNCTION( vsp::GetDesignVarType ), asCALL_CDECL );
    assert( r >= 0 );

    //==== Computations ====//
    r = se->RegisterGlobalFunction( "void SetComputationFileName( int file_type, const string & in file_name )", asFUNCTION( vsp::SetComputationFileName ), asCALL_CDECL );
    assert( r >= 0 );
    r = se->RegisterGlobalFunction( "string ComputeMassProps( int set, int num_slices )", asFUNCTION( vsp::ComputeMassProps ), asCALL_CDECL );
    assert( r >= 0 );
    r = se->RegisterGlobalFunction( "string ComputeCompGeom( int set, bool half_mesh, int file_export_types )", asFUNCTION( vsp::ComputeCompGeom ), asCALL_CDECL );
    assert( r >= 0 );
    r = se->RegisterGlobalFunction( "string ComputePlaneSlice( int set, int num_slices, const vec3d & in norm, bool auto_bnd, double start_bnd = 0, double end_bnd = 0 )", asFUNCTION( vsp::ComputePlaneSlice ), asCALL_CDECL );
    assert( r >= 0 );
    r = se->RegisterGlobalFunction( "void ComputeDegenGeom( int set, int file_type )", asFUNCTION( vsp::ComputeDegenGeom ), asCALL_CDECL );
    assert( r >= 0 );
    r = se->RegisterGlobalFunction( "void ComputeCFDMesh( int set, int file_type )", asFUNCTION( vsp::ComputeCFDMesh ), asCALL_CDECL );
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
    r = se->RegisterGlobalFunction( "array<string>@  ListAnalysis()", asMETHOD( ScriptMgrSingleton, ListAnalysis ), asCALL_THISCALL_ASGLOBAL, &ScriptMgr );
    assert( r >= 0 );
    r = se->RegisterGlobalFunction( "array<string>@  GetAnalysisInputNames(const string & in analysis )", asMETHOD( ScriptMgrSingleton, GetAnalysisInputNames ), asCALL_THISCALL_ASGLOBAL, &ScriptMgr );
    assert( r >= 0 );
    r = se->RegisterGlobalFunction( "string ExecAnalysis( const string & in analysis )", asFUNCTION( vsp::ExecAnalysis ), asCALL_CDECL );
    assert( r >= 0 );
    r = se->RegisterGlobalFunction( "int GetNumAnalysisInputData( const string & in analysis, const string & in name )", asFUNCTION( vsp::GetNumAnalysisInputData ), asCALL_CDECL );
    assert( r >= 0 );
    r = se->RegisterGlobalFunction( "int GetAnalysisInputType( const string & in analysis, const string & in name )", asFUNCTION( vsp::GetAnalysisInputType ), asCALL_CDECL );
    assert( r >= 0 );
    r = se->RegisterGlobalFunction( "array<int>@  GetIntAnalysisInput( const string & in analysis, const string & in name, int index = 0 )", asMETHOD( ScriptMgrSingleton, GetIntAnalysisInput ), asCALL_THISCALL_ASGLOBAL, &ScriptMgr );
    assert( r >= 0 );
    r = se->RegisterGlobalFunction( "array<double>@  GetDoubleAnalysisInput( const string & in analysis, const string & in name, int index = 0 )", asMETHOD( ScriptMgrSingleton, GetDoubleAnalysisInput ), asCALL_THISCALL_ASGLOBAL, &ScriptMgr );
    assert( r >= 0 );
    r = se->RegisterGlobalFunction( "array<string>@  GetStringAnalysisInput( const string & in analysis, const string & in name, int index = 0 )", asMETHOD( ScriptMgrSingleton, GetStringAnalysisInput ), asCALL_THISCALL_ASGLOBAL, &ScriptMgr );
    assert( r >= 0 );
    r = se->RegisterGlobalFunction( "array<vec3d>@  GetVec3dAnalysisInput( const string & in analysis, const string & in name, int index = 0 )", asMETHOD( ScriptMgrSingleton, GetVec3dAnalysisInput ), asCALL_THISCALL_ASGLOBAL, &ScriptMgr );
    assert( r >= 0 );
    r = se->RegisterGlobalFunction( "void PrintAnalysisInputs( const string & in analysis )", asFUNCTION( vsp::PrintAnalysisInputs ), asCALL_CDECL );
    assert( r >= 0 );

    r = se->RegisterGlobalFunction( "void SetAnalysisInputDefaults( const string & in analysis )", asFUNCTION( vsp::SetAnalysisInputDefaults ), asCALL_CDECL );
    assert( r >= 0 );
    r = se->RegisterGlobalFunction( "void SetIntAnalysisInput( const string & in analysis, const string & in name, array<int>@ indata_arr, int index = 0 )", asMETHOD( ScriptMgrSingleton, SetIntAnalysisInput ), asCALL_THISCALL_ASGLOBAL, &ScriptMgr );
    assert( r >= 0 );
    r = se->RegisterGlobalFunction( "void SetDoubleAnalysisInput( const string & in analysis, const string & in name, array<double>@ indata_arr, int index = 0 )", asMETHOD( ScriptMgrSingleton, SetDoubleAnalysisInput ), asCALL_THISCALL_ASGLOBAL, &ScriptMgr );
    assert( r >= 0 );
    r = se->RegisterGlobalFunction( "void SetStringAnalysisInput( const string & in analysis, const string & in name, array<string>@ indata_arr, int index = 0 )", asMETHOD( ScriptMgrSingleton, SetStringAnalysisInput ), asCALL_THISCALL_ASGLOBAL, &ScriptMgr );
    assert( r >= 0 );
    r = se->RegisterGlobalFunction( "void SetVec3dAnalysisInput( const string & in analysis, const string & in name, array<vec3d>@ indata_arr, int index = 0 )", asMETHOD( ScriptMgrSingleton, SetVec3dAnalysisInput ), asCALL_THISCALL_ASGLOBAL, &ScriptMgr );
    assert( r >= 0 );


    //==== Results Functions ====//
    r = se->RegisterGlobalFunction( "int GetNumResults( const string & in name )", asFUNCTION( vsp::GetNumResults ), asCALL_CDECL );
    assert( r >= 0 );
    r = se->RegisterGlobalFunction( "string GetResultsName( const string & in results_id )", asFUNCTION( vsp::GetResultsName ), asCALL_CDECL );
    assert( r >= 0 );
    r = se->RegisterGlobalFunction( "string FindResultsID( const string & in name, int index = 0 )", asFUNCTION( vsp::FindResultsID ), asCALL_CDECL );
    assert( r >= 0 );
    r = se->RegisterGlobalFunction( "string FindLatestResultsID( const string & in name )", asFUNCTION( vsp::FindLatestResultsID ), asCALL_CDECL );
    assert( r >= 0 );
    r = se->RegisterGlobalFunction( "int GetNumData( const string & in results_id, const string & in data_name )", asFUNCTION( vsp::GetNumData ), asCALL_CDECL );
    assert( r >= 0 );
    r = se->RegisterGlobalFunction( "int GetResultsType( const string & in results_id, const string & in data_name )", asFUNCTION( vsp::GetResultsType ), asCALL_CDECL );
    assert( r >= 0 );
    r = se->RegisterGlobalFunction( "array<string>@  GetAllResultsNames()", asMETHOD( ScriptMgrSingleton, GetAllResultsNames ), asCALL_THISCALL_ASGLOBAL, &ScriptMgr );
    assert( r >= 0 );
    r = se->RegisterGlobalFunction( "array<string>@  GetAllDataNames(const string & in results_id )", asMETHOD( ScriptMgrSingleton, GetAllDataNames ), asCALL_THISCALL_ASGLOBAL, &ScriptMgr );
    assert( r >= 0 );
    r = se->RegisterGlobalFunction( "array<int>@  GetIntResults( const string & in id, const string & in name, int index = 0 )", asMETHOD( ScriptMgrSingleton, GetIntResults ), asCALL_THISCALL_ASGLOBAL, &ScriptMgr );
    assert( r >= 0 );
    r = se->RegisterGlobalFunction( "array<double>@  GetDoubleResults( const string & in id, const string & in name, int index = 0 )", asMETHOD( ScriptMgrSingleton, GetDoubleResults ), asCALL_THISCALL_ASGLOBAL, &ScriptMgr );
    assert( r >= 0 );
    r = se->RegisterGlobalFunction( "array<array<double>@>@ GetDoubleMatResults( const string & in id, const string & in name, int index = 0 )", asMETHOD( ScriptMgrSingleton, GetDoubleMatResults ), asCALL_THISCALL_ASGLOBAL, &ScriptMgr );
    assert( r >= 0 );
    r = se->RegisterGlobalFunction( "array<string>@  GetStringResults( const string & in id, const string & in name, int index = 0 )", asMETHOD( ScriptMgrSingleton, GetStringResults ), asCALL_THISCALL_ASGLOBAL, &ScriptMgr );
    assert( r >= 0 );
    r = se->RegisterGlobalFunction( "array<vec3d>@  GetVec3dResults( const string & in id, const string & in name, int index = 0 )", asMETHOD( ScriptMgrSingleton, GetVec3dResults ), asCALL_THISCALL_ASGLOBAL, &ScriptMgr );
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

    r = se->RegisterGlobalFunction( "void WriteTestResults()", asMETHOD( ResultsMgrSingleton, WriteTestResults ), asCALL_THISCALL_ASGLOBAL, &ResultsMgr );
    assert( r >= 0 );


    //==== Geom Functions ====//
    r = se->RegisterGlobalFunction( "array<string>@  GetGeomTypes()", asMETHOD( ScriptMgrSingleton, GetGeomTypes ), asCALL_THISCALL_ASGLOBAL, &ScriptMgr );
    assert( r >= 0 );
    r = se->RegisterGlobalFunction( "string AddGeom( const string & in type, const string & in parent = string() )", asFUNCTION( vsp::AddGeom ), asCALL_CDECL );
    assert( r >= 0 );
    r = se->RegisterGlobalFunction( "void UpdateGeom(const string & in geom_id)", asFUNCTION( vsp::UpdateGeom ), asCALL_CDECL );
    assert( r >= 0 );
    r = se->RegisterGlobalFunction( "void DeleteGeom(const string & in geom_id)", asFUNCTION( vsp::DeleteGeom ), asCALL_CDECL );
    assert( r >= 0 );
    r = se->RegisterGlobalFunction( "void DeleteGeomVec( array<string>@ del_arr )", asMETHOD( ScriptMgrSingleton, DeleteGeomVec ), asCALL_THISCALL_ASGLOBAL, &ScriptMgr );
    assert( r >= 0 );
    r = se->RegisterGlobalFunction( "void CutGeomToClipboard(const string & in geom_id)", asFUNCTION( vsp::CutGeomToClipboard ), asCALL_CDECL );
    assert( r >= 0 );
    r = se->RegisterGlobalFunction( "void CopyGeomToClipboard(const string & in geom_id)", asFUNCTION( vsp::CopyGeomToClipboard ), asCALL_CDECL );
    assert( r >= 0 );
    r = se->RegisterGlobalFunction( "array<string>@  PasteGeomClipboard( const string & in parent_id = string() )", asMETHOD( ScriptMgrSingleton, PasteGeomClipboard ), asCALL_THISCALL_ASGLOBAL, &ScriptMgr );
    assert( r >= 0 );

    r = se->RegisterGlobalFunction( "array<string>@  FindGeoms()", asMETHOD( ScriptMgrSingleton, FindGeoms ), asCALL_THISCALL_ASGLOBAL, &ScriptMgr );
    assert( r >= 0 );
    r = se->RegisterGlobalFunction( "array<string>@  FindGeomsWithName(const string & in name)", asMETHOD( ScriptMgrSingleton, FindGeomsWithName ), asCALL_THISCALL_ASGLOBAL, &ScriptMgr );
    assert( r >= 0 );
    r = se->RegisterGlobalFunction( "void SetGeomName( const string & in geom_id, const string & in name )", asFUNCTION( vsp::SetGeomName ), asCALL_CDECL );
    assert( r >= 0 );
    r = se->RegisterGlobalFunction( "string GetGeomName( const string & in geom_id )", asFUNCTION( vsp::GetGeomName ), asCALL_CDECL );
    assert( r >= 0 );
    r = se->RegisterGlobalFunction( "array<string>@  GetGeomParmIDs(const string & in geom_id )", asMETHOD( ScriptMgrSingleton, GetGeomParmIDs ), asCALL_THISCALL_ASGLOBAL, &ScriptMgr );
    assert( r >= 0 );
    r = se->RegisterGlobalFunction( "string GetGeomTypeName(const string & in geom_id )", asFUNCTION( vsp::GetGeomTypeName ), asCALL_CDECL );
    assert( r >= 0 );
    r = se->RegisterGlobalFunction( "string GetParm(const string & in geom_id, const string & in name, const string & in group )", asFUNCTION( vsp::GetParm ), asCALL_CDECL );
    assert( r >= 0 );
    r = se->RegisterGlobalFunction( "int GetNumXSecSurfs( const string & in geom_id )", asFUNCTION( vsp::GetNumXSecSurfs ), asCALL_CDECL );
    assert( r >= 0 );
    r = se->RegisterGlobalFunction( "int GetNumMainSurfs( const string & in geom_id )", asFUNCTION( vsp::GetNumMainSurfs ), asCALL_CDECL );
    assert( r >= 0 );
    r = se->RegisterGlobalFunction( "string AddSubSurf( const string & in geom_id, int type, int surfindex = 0 )", asFUNCTION( vsp::AddSubSurf ), asCALL_CDECL );
    assert( r >= 0 );
    r = se->RegisterGlobalFunction( "void DeleteSubSurf( const string & in geom_id, const string & in sub_id )", asFUNCTIONPR( vsp::DeleteSubSurf, ( const string &, const string & ), void ), asCALL_CDECL );
    assert( r >= 0 );
    r = se->RegisterGlobalFunction( "void DeleteSubSurf( const string & in sub_id )", asFUNCTIONPR( vsp::DeleteSubSurf, ( const string & ), void ), asCALL_CDECL );
    assert( r >= 0 );
    r = se->RegisterGlobalFunction( "string GetSubSurf( const string & in geom_id, int index )", asFUNCTIONPR( vsp::GetSubSurf, ( const string &, int ), string ), asCALL_CDECL );
    assert( r >= 0 );
    r = se->RegisterGlobalFunction( "array<string>@ GetSubSurf( const string & in geom_id, const string & in name )", asMETHOD( ScriptMgrSingleton, GetSubSurf ), asCALL_THISCALL_ASGLOBAL, &ScriptMgr );
    assert( r >= 0 );
    r = se->RegisterGlobalFunction( "void SetSubSurfName( const string & in geom_id, const string & in sub_id, const string & in name )", asFUNCTIONPR( vsp::SetSubSurfName, ( const string &, const string &, const string & ), void ), asCALL_CDECL );
    assert( r >= 0 );
    r = se->RegisterGlobalFunction( "void SetSubSurfName( const string & in sub_id, const string & in name )", asFUNCTIONPR( vsp::SetSubSurfName, ( const string &, const string & ), void ), asCALL_CDECL );
    assert( r >= 0 );
    r = se->RegisterGlobalFunction( "string GetSubSurfName( const string & in geom_id, const string & in sub_id )", asFUNCTIONPR( vsp::GetSubSurfName, ( const string &, const string & ), string ), asCALL_CDECL );
    assert( r >= 0 );
    r = se->RegisterGlobalFunction( "string GetSubSurfName( const string & in sub_id )", asFUNCTIONPR( vsp::GetSubSurfName, ( const string & ), string ), asCALL_CDECL );
    assert( r >= 0 );
    r = se->RegisterGlobalFunction( "int GetSubSurfIndex( const string & in sub_id )", asFUNCTION( vsp::GetSubSurfIndex ), asCALL_CDECL );
    assert( r >= 0 );
    r = se->RegisterGlobalFunction( "array<string>@ GetSubSurfIDVec( const string & in geom_id )", asMETHOD( ScriptMgrSingleton, GetSubSurfIDVec ), asCALL_THISCALL_ASGLOBAL, &ScriptMgr );
    assert( r >= 0 );
    r = se->RegisterGlobalFunction( "array<string>@ GetAllSubSurfIDs()", asMETHOD( ScriptMgrSingleton, GetAllSubSurfIDs ), asCALL_THISCALL_ASGLOBAL, &ScriptMgr );
    assert( r >= 0 );
    r = se->RegisterGlobalFunction( "int GetNumSubSurf( const string & in geom_id )", asFUNCTION( vsp::GetNumSubSurf ), asCALL_CDECL );
    assert( r >= 0 );
    r = se->RegisterGlobalFunction( "int GetSubSurfType( const string & in sub_id )", asFUNCTION( vsp::GetSubSurfType ), asCALL_CDECL );
    assert( r >= 0 );
    r = se->RegisterGlobalFunction( "array<string>@ GetSubSurfParmIDs(const string & in sub_id )", asMETHOD( ScriptMgrSingleton, GetSubSurfParmIDs ), asCALL_THISCALL_ASGLOBAL, &ScriptMgr );
    assert( r >= 0 );
    r = se->RegisterGlobalFunction("void AutoGroupVSPAEROControlSurfaces()", asFUNCTION(vsp::AutoGroupVSPAEROControlSurfaces), asCALL_CDECL);
    assert(r >= 0);
    r = se->RegisterGlobalFunction( "int GetNumControlSurfaceGroups()", asFUNCTION( vsp::GetNumControlSurfaceGroups ), asCALL_CDECL );
    assert( r >= 0 );
    r = se->RegisterGlobalFunction("int CreateVSPAEROControlSurfaceGroup()", asFUNCTION(vsp::CreateVSPAEROControlSurfaceGroup), asCALL_CDECL);
    assert(r >= 0);
    r = se->RegisterGlobalFunction("void AddAllToVSPAEROControlSurfaceGroup( int CSGroupIndex )", asFUNCTION(vsp::AddAllToVSPAEROControlSurfaceGroup ), asCALL_CDECL);
    assert(r >= 0);
    r = se->RegisterGlobalFunction("void RemoveAllFromVSPAEROControlSurfaceGroup( int CSGroupIndex )", asFUNCTION(vsp::RemoveAllFromVSPAEROControlSurfaceGroup ), asCALL_CDECL);
    assert(r >= 0);
    r = se->RegisterGlobalFunction( "array<string>@ GetActiveCSNameVec( int CSGroupIndex )", asMETHOD( ScriptMgrSingleton, GetActiveCSNameVec ), asCALL_THISCALL_ASGLOBAL, &ScriptMgr );
    assert( r >= 0 );
    r = se->RegisterGlobalFunction( "array<string>@ GetCompleteCSNameVec( )", asMETHOD( ScriptMgrSingleton, GetCompleteCSNameVec ), asCALL_THISCALL_ASGLOBAL, &ScriptMgr );
    assert( r >= 0 );
    r = se->RegisterGlobalFunction( "array<string>@ GetAvailableCSNameVec( int CSGroupIndex )", asMETHOD( ScriptMgrSingleton, GetAvailableCSNameVec ), asCALL_THISCALL_ASGLOBAL, &ScriptMgr );
    assert( r >= 0 );
    r = se->RegisterGlobalFunction("void SetVSPAEROControlGroupName( const string & in name, int CSGroupIndex )", asFUNCTION(vsp::SetVSPAEROControlGroupName), asCALL_CDECL);
    assert(r >= 0);
    r = se->RegisterGlobalFunction( "string GetVSPAEROControlGroupName( int CSGroupIndex )", asFUNCTION( vsp::GetVSPAEROControlGroupName ), asCALL_CDECL );
    assert( r >= 0 );
    r = se->RegisterGlobalFunction( "void AddSelectedToCSGroup( array<int>@ selected, int CSGroupIndex )", asMETHOD( ScriptMgrSingleton, AddSelectedToCSGroup ), asCALL_THISCALL_ASGLOBAL, &ScriptMgr );
    assert( r >= 0 );
    r = se->RegisterGlobalFunction( "void RemoveSelectedFromCSGroup( array<int>@ selected, int CSGroupIndex )", asMETHOD( ScriptMgrSingleton, RemoveSelectedFromCSGroup ), asCALL_THISCALL_ASGLOBAL, &ScriptMgr );
    assert( r >= 0 );
    r = se->RegisterGlobalFunction( "void CutXSec( const string & in geom_id, int index )", asFUNCTION( vsp::CutXSec ), asCALL_CDECL );
    assert( r >= 0 );
    r = se->RegisterGlobalFunction( "void CopyXSec( const string & in geom_id, int index )", asFUNCTION( vsp::CopyXSec ), asCALL_CDECL );
    assert( r >= 0 );
    r = se->RegisterGlobalFunction( "void PasteXSec( const string & in geom_id, int index )", asFUNCTION( vsp::PasteXSec ), asCALL_CDECL );
    assert( r >= 0 );
    r = se->RegisterGlobalFunction( "void InsertXSec( const string & in geom_id, int index, int type )", asFUNCTION( vsp::InsertXSec ), asCALL_CDECL );
    assert( r >= 0 );

    //==== VSPAERO Functions ====//
    r = se->RegisterGlobalFunction( "string GetVSPAERORefWingID()", asFUNCTION( vsp::GetVSPAERORefWingID ), asCALL_CDECL );
    assert( r >= 0 );
    r = se->RegisterGlobalFunction( "string SetVSPAERORefWingID( const string & in geom_id )", asFUNCTION( vsp::SetVSPAERORefWingID ), asCALL_CDECL );
    assert( r >= 0 );

    //==== Wing Sect Functions ====//
    r = se->RegisterGlobalFunction( "void SetDriverGroup( const string & in geom_id, int section_index, int driver_0, int driver_1, int driver_2)", asFUNCTION( vsp::SetDriverGroup ), asCALL_CDECL );
    assert( r >= 0 );

    //==== XSecSurf Functions ====//
    r = se->RegisterGlobalFunction( "string GetXSecSurf( const string & in geom_id, int index )", asFUNCTION( vsp::GetXSecSurf ), asCALL_CDECL );
    assert( r >= 0 );
    r = se->RegisterGlobalFunction( "int GetNumXSec( const string & in xsec_surf_id )", asFUNCTION( vsp::GetNumXSec ), asCALL_CDECL );
    assert( r >= 0 );
    r = se->RegisterGlobalFunction( "string GetXSec( const string & in xsec_surf_id, int xsec_index )", asFUNCTION( vsp::GetXSec ), asCALL_CDECL );
    assert( r >= 0 );
    r = se->RegisterGlobalFunction( "void ChangeXSecShape( const string & in xsec_surf_id, int xsec_index, int type )", asFUNCTION( vsp::ChangeXSecShape ), asCALL_CDECL );
    assert( r >= 0 );
    r = se->RegisterGlobalFunction( "void SetXSecSurfGlobalXForm( const string & in xsec_surf_id, const Matrix4d & in mat )", asFUNCTION( vsp::SetXSecSurfGlobalXForm ), asCALL_CDECL );
    assert( r >= 0 );
    r = se->RegisterGlobalFunction( "Matrix4d GetXSecSurfGlobalXForm( const string & in xsec_surf_id )", asFUNCTION( vsp::GetXSecSurfGlobalXForm ), asCALL_CDECL );
    assert( r >= 0 );

    //==== XSec Functions ====//
    r = se->RegisterGlobalFunction( "int GetXSecShape( const string& in xsec_id )", asFUNCTION( vsp::GetXSecShape ), asCALL_CDECL );
    assert( r >= 0 );
    r = se->RegisterGlobalFunction( "double GetXSecWidth( const string& in xsec_id )", asFUNCTION( vsp::GetXSecWidth ), asCALL_CDECL );
    assert( r >= 0 );
    r = se->RegisterGlobalFunction( "double GetXSecHeight( const string& in xsec_id )", asFUNCTION( vsp::GetXSecHeight ), asCALL_CDECL );
    assert( r >= 0 );
    r = se->RegisterGlobalFunction( "void SetXSecWidthHeight( const string& in xsec_id, double w, double h )", asFUNCTION( vsp::SetXSecWidthHeight ), asCALL_CDECL );
    assert( r >= 0 );
    r = se->RegisterGlobalFunction( "array<string>@  GetXSecParmIDs(const string & in xsec_id )", asMETHOD( ScriptMgrSingleton, GetXSecParmIDs ), asCALL_THISCALL_ASGLOBAL, &ScriptMgr );
    assert( r >= 0 );
    r = se->RegisterGlobalFunction( "string GetXSecParm( const string& in xsec_id, const string& in name )", asFUNCTION( vsp::GetXSecParm ), asCALL_CDECL );
    assert( r >= 0 );
    r = se->RegisterGlobalFunction( "array<vec3d>@  ReadFileXSec(const string& in xsec_id, const string& in file_name  )", asMETHOD( ScriptMgrSingleton, ReadFileXSec ), asCALL_THISCALL_ASGLOBAL, &ScriptMgr );
    assert( r >= 0 );
    r = se->RegisterGlobalFunction( "void SetXSecPnts( const string& in xsec_id, array<vec3d>@ pnt_arr )", asMETHOD( ScriptMgrSingleton, SetXSecPnts ), asCALL_THISCALL_ASGLOBAL, &ScriptMgr );
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
    r = se->RegisterGlobalFunction( "void SetAirfoilPnts( const string& in xsec_id, array<vec3d>@ up_pnt_vec, array<vec3d>@ low_pnt_vec )", asMETHOD( ScriptMgrSingleton, SetAirfoilPnts ), asCALL_THISCALL_ASGLOBAL, &ScriptMgr );
    assert( r >= 0 );
    r = se->RegisterGlobalFunction( "array<vec3d>@ GetHersheyBarLiftDist( const int npts, const double alpha, const double Vinf, const double span, bool full_span_flag = false )", asMETHOD( ScriptMgrSingleton, GetHersheyBarLiftDist ), asCALL_THISCALL_ASGLOBAL, &ScriptMgr );
    assert( r >= 0 );
    r = se->RegisterGlobalFunction( "array<vec3d>@ GetHersheyBarDragDist( const int npts, const double alpha, const double Vinf, const double span, bool full_span_flag = false )", asMETHOD( ScriptMgrSingleton, GetHersheyBarDragDist ), asCALL_THISCALL_ASGLOBAL, &ScriptMgr );
    assert( r >= 0 );
    r = se->RegisterGlobalFunction( "array<vec3d>@ GetVKTAirfoilPnts( const int npts, const double alpha, const double epsilon, const double kappa, const double tau )", asMETHOD( ScriptMgrSingleton, GetVKTAirfoilPnts ), asCALL_THISCALL_ASGLOBAL, &ScriptMgr );
    assert( r >= 0 );
    r = se->RegisterGlobalFunction( "array<double>@ GetVKTAirfoilCpDist( const double alpha, const double epsilon, const double kappa, const double tau, array<vec3d>@ xydata )", asMETHOD( ScriptMgrSingleton, GetVKTAirfoilCpDist ), asCALL_THISCALL_ASGLOBAL, &ScriptMgr );
    assert( r >= 0 );
    r = se->RegisterGlobalFunction( "array<vec3d>@ GetEllipsoidSurfPnts( const vec3d& in center, const vec3d& in abc_rad, int u_npts = 20, int w_npts = 20 )", asMETHOD( ScriptMgrSingleton, GetEllipsoidSurfPnts ), asCALL_THISCALL_ASGLOBAL, &ScriptMgr );
    assert( r >= 0 );
    r = se->RegisterGlobalFunction( "array<vec3d>@ GetFeatureLinePnts( const string& in geom_id )", asMETHOD( ScriptMgrSingleton, GetFeatureLinePnts ), asCALL_THISCALL_ASGLOBAL, &ScriptMgr );
    assert( r >= 0 );
    r = se->RegisterGlobalFunction( "array<double>@ GetEllipsoidCpDist( array<vec3d>@ surf_pnt_arr, const vec3d& in abc_rad, const vec3d& in V_inf )", asMETHOD( ScriptMgrSingleton, GetEllipsoidCpDist ), asCALL_THISCALL_ASGLOBAL, &ScriptMgr );
    assert( r >= 0 );
    r = se->RegisterGlobalFunction( "array<vec3d>@  GetAirfoilUpperPnts(const string& in xsec_id )", asMETHOD( ScriptMgrSingleton, GetAirfoilUpperPnts ), asCALL_THISCALL_ASGLOBAL, &ScriptMgr );
    assert( r >= 0 );
    r = se->RegisterGlobalFunction( "array<vec3d>@  GetAirfoilLowerPnts(const string& in xsec_id )", asMETHOD( ScriptMgrSingleton, GetAirfoilLowerPnts ), asCALL_THISCALL_ASGLOBAL, &ScriptMgr );
    assert( r >= 0 );
    r = se->RegisterGlobalFunction( "array<double>@  GetUpperCSTCoefs( const string & in xsec_id )", asMETHOD( ScriptMgrSingleton, GetUpperCSTCoefs ), asCALL_THISCALL_ASGLOBAL, &ScriptMgr );
    assert( r >= 0 );
    r = se->RegisterGlobalFunction( "array<double>@  GetLowerCSTCoefs( const string & in xsec_id )", asMETHOD( ScriptMgrSingleton, GetLowerCSTCoefs ), asCALL_THISCALL_ASGLOBAL, &ScriptMgr );
    assert( r >= 0 );
    r = se->RegisterGlobalFunction( "int GetUpperCSTDegree( const string& in xsec_id )", asFUNCTION( vsp::GetUpperCSTDegree ), asCALL_CDECL );
    assert( r >= 0 );
    r = se->RegisterGlobalFunction( "int GetLowerCSTDegree( const string& in xsec_id )", asFUNCTION( vsp::GetLowerCSTDegree ), asCALL_CDECL );
    assert( r >= 0 );
    r = se->RegisterGlobalFunction( "void SetUpperCST( const string& in xsec_id, int deg, array<double>@ coeff_arr )", asMETHOD( ScriptMgrSingleton, SetUpperCST ), asCALL_THISCALL_ASGLOBAL, &ScriptMgr );
    assert( r >= 0 );
    r = se->RegisterGlobalFunction( "void SetLowerCST( const string& in xsec_id, int deg, array<double>@ coeff_arr )", asMETHOD( ScriptMgrSingleton, SetLowerCST ), asCALL_THISCALL_ASGLOBAL, &ScriptMgr );
    assert( r >= 0 );
    r = se->RegisterGlobalFunction( "void PromoteCSTUpper( const string& in xsec_id )", asFUNCTION( vsp::PromoteCSTUpper ), asCALL_CDECL );
    assert( r >= 0 );
    r = se->RegisterGlobalFunction( "void PromoteCSTLower( const string& in xsec_id )", asFUNCTION( vsp::PromoteCSTLower ), asCALL_CDECL );
    assert( r >= 0 );
    r = se->RegisterGlobalFunction( "void DemoteCSTUpper( const string& in xsec_id )", asFUNCTION( vsp::DemoteCSTUpper ), asCALL_CDECL );
    assert( r >= 0 );
    r = se->RegisterGlobalFunction( "void DemoteCSTLower( const string& in xsec_id )", asFUNCTION( vsp::DemoteCSTLower ), asCALL_CDECL );
    assert( r >= 0 );
    r = se->RegisterGlobalFunction( "void FitAfCST( const string& in xsec_id, int xsec_index, int deg )", asFUNCTION( vsp::FitAfCST ), asCALL_CDECL );
    assert( r >= 0 );

    r = se->RegisterGlobalFunction( "void WriteBezierAirfoil( const string& in file_name, const string& in geom_id, const double foilsurf_u )", asFUNCTION( vsp::WriteBezierAirfoil ), asCALL_CDECL );
    assert( r >= 0 );
    r = se->RegisterGlobalFunction( "void WriteSeligAirfoil( const string& in file_name, const string& in geom_id, const double foilsurf_u )", asFUNCTION( vsp::WriteSeligAirfoil ), asCALL_CDECL );
    assert( r >= 0 );
    r = se->RegisterGlobalFunction( "array<vec3d>@ GetAirfoilCoordinates( const string& in geom_id, const double foilsurf_u )", asMETHOD( ScriptMgrSingleton, GetAirfoilCoordinates ), asCALL_THISCALL_ASGLOBAL, &ScriptMgr );
    assert( r >= 0 );

    //==== Edit Curve XSec Functions ====//
//    comment_str = R"(
///*!
//    Initialize the EditCurveXSec to the current value of m_ShapeType (i.e. EDIT_XSEC_ELLIPSE)
//    \sa INIT_EDIT_XSEC_TYPE
//    \param xsec_id XSec ID
//*/)";
    r = se->RegisterGlobalFunction( "void EditXSecInitShape( const string& in xsec_id )", asFUNCTION( vsp::EditXSecInitShape ), asCALL_CDECL );
    assert( r >= 0 );

//    comment_str = R"(
///*!
//    Convert the EditCurveXSec curve type to the specified new type. Note, EditCurveXSec uses the same enumerations for PCurve to identify curve type.
//    \sa PCURV_TYPE
//    \param newtype New curve type enum (i.e. CEDIT)
//    \param xsec_id XSec ID
//*/)";
    r = se->RegisterGlobalFunction( "void EditXSecConvertTo( const string& in xsec_id, const int& in newtype )", asFUNCTION( vsp::EditXSecConvertTo ), asCALL_CDECL );
    assert( r >= 0 );

//    comment_str = R"(
///*!
//    Get the U parameter vector for an EditCurveXSec. The vector will be in increasing order with a range of 0 - 1. 
//    \param xsec_id XSec ID
//    \return Array of U parameter values
//*/)";
    r = se->RegisterGlobalFunction( "array<double>@ GetEditXSecUVec( const string& in xsec_id )", asMETHOD( ScriptMgrSingleton, GetEditXSecUVec ), asCALL_THISCALL_ASGLOBAL, &ScriptMgr );
    assert( r >= 0 );

//    comment_str = R"(
///*!
//    Get the control point vector for an EditCurveXSec. Note, the returned array of vec3d values will be represented in 2D with Z set to 0.
//    \param xsec_id XSec ID
//    \param non_dimensional True to get the points non-dimensionalized, False to get them scaled by m_Width and m_Height
//    \return Array of control points
//*/)";
    r = se->RegisterGlobalFunction( "array<vec3d>@ GetEditXSecCtrlVec( const string& in xsec_id, const bool non_dimensional = true )", asMETHOD( ScriptMgrSingleton, GetEditXSecCtrlVec ), asCALL_THISCALL_ASGLOBAL, &ScriptMgr );
    assert( r >= 0 );

//    comment_str = R"(
///*!
//    Set the U parameter vector and the control point vector for an EditCurveXSec. The arrays must be of equal length, with the values for U defined in 
//    increasing order and range 0 - 1. The XSec is nondimentionalized and 2D, so the input control points wil be scaled by m_Width and m_Height and the 
//    Z values will be ignred. 
//    \param xsec_id XSec ID
//    \param u_vec Array of U parameter values
//    \param control_pts Array of control points
//*/)";
    r = se->RegisterGlobalFunction( "void SetEditXSecPnts( const string& in xsec_id, array<double>@ u_vec, array<vec3d>@ control_pts )", asMETHOD( ScriptMgrSingleton, SetEditXSecPnts ), asCALL_THISCALL_ASGLOBAL, &ScriptMgr );
    assert( r >= 0 );

//    comment_str = R"(
///*!
//    Delete an EditCurveXSec control point. Note, cubic Bezier intermediate control points (those not on the curve) cannot be deleted.
//    The previous and next Bezier control point will be deleted along with the point on the curve. Regardless of curve type, the first
//    and last points may not be deleted. 
//    \param xsec_id XSec ID
//    \param indx Control point index
//*/)";
    r = se->RegisterGlobalFunction( "void EditXSecDelPnt( const string& in xsec_id, const int& in indx )", asFUNCTION( vsp::EditXSecDelPnt ), asCALL_CDECL );
    assert( r >= 0 );

//    comment_str = R"(
///*!
//    Split the EditCurveXSec at the specified U value
//    \param xsec_id XSec ID
//    \param u U value to split the curve at (0 - 1)
//    \return Index of the point added from the split
//*/)";
    r = se->RegisterGlobalFunction( "int EditXSecSplit01( const string& in xsec_id, const double& in u )", asFUNCTION( vsp::EditXSecSplit01 ), asCALL_CDECL );
    assert( r >= 0 );

//    comment_str = R"(
///*!
//    Move an EditCurveXSec control point. The XSec points are nondimensionalized by m_Width and m_Height and 
//    defined in 2D, so the Z value of the new coordinate point will be ignored.
//    \param xsec_id XSec ID
//    \param xndx Control point index
//    \param new_pnt Coordinate of the new point
//*/)";
    r = se->RegisterGlobalFunction( "void MoveEditXSecPnt( const string& in xsec_id, const int& in indx, const vec3d& in new_pnt )", asFUNCTION( vsp::MoveEditXSecPnt ), asCALL_CDECL );
    assert( r >= 0 );

//    comment_str = R"(
///*!
//    Convert any XSec type into an EditCurveXSec. This function will work for BOR Geoms, in which case the input XSec index is ignored.
//    \param geom_id Geom ID
//    \param indx XSec index
//*/)";
    r = se->RegisterGlobalFunction( "void ConvertXSecToEdit( const string& in geom_id, const int& in indx = 0 )", asFUNCTION( vsp::ConvertXSecToEdit ), asCALL_CDECL );
    assert( r >= 0 );

    //==== Specialized Geom Functions ====//
    r = se->RegisterGlobalFunction( "void ChangeBORXSecShape( const string & in geom_id, int type )", asFUNCTION( vsp::ChangeBORXSecShape ), asCALL_CDECL );
    assert( r >= 0 );
    r = se->RegisterGlobalFunction( "int GetBORXSecShape( const string & in geom_id )", asFUNCTION( vsp::GetBORXSecShape ), asCALL_CDECL );
    assert( r >= 0 );

    //==== Sets Functions ====//
    r = se->RegisterGlobalFunction( "int GetNumSets()", asFUNCTION( vsp::GetNumSets ), asCALL_CDECL );
    assert( r >= 0 );
    r = se->RegisterGlobalFunction( "void SetSetName( int index, const string& in name )", asFUNCTION( vsp::SetSetName ), asCALL_CDECL );
    assert( r >= 0 );
    r = se->RegisterGlobalFunction( "string GetSetName( int index )", asFUNCTION( vsp::GetSetName ), asCALL_CDECL );
    assert( r >= 0 );
    r = se->RegisterGlobalFunction( "array<string>@  GetGeomSetAtIndex( int index )", asMETHOD( ScriptMgrSingleton, GetGeomSetAtIndex ), asCALL_THISCALL_ASGLOBAL, &ScriptMgr );
    assert( r >= 0 );
    r = se->RegisterGlobalFunction( "array<string>@  GetGeomSet( const string & in name )", asMETHOD( ScriptMgrSingleton, GetGeomSet ), asCALL_THISCALL_ASGLOBAL, &ScriptMgr );
    assert( r >= 0 );
    r = se->RegisterGlobalFunction( "int GetSetIndex( const string & in name )", asFUNCTION( vsp::GetSetIndex ), asCALL_CDECL );
    assert( r >= 0 );
    r = se->RegisterGlobalFunction( "bool GetSetFlag( const string & in geom_id, int set_index )", asFUNCTION( vsp::GetSetFlag ), asCALL_CDECL );
    assert( r >= 0 );
    r = se->RegisterGlobalFunction( "void SetSetFlag( const string & in geom_id, int set_index, bool flag )", asFUNCTION( vsp::SetSetFlag ), asCALL_CDECL );
    assert( r >= 0 );

    //=== Group Modifications ===//
    r = se->RegisterGlobalFunction( "void ScaleSet( int set_index, double scale )", asFUNCTION( vsp::ScaleSet ), asCALL_CDECL );
    assert( r >= 0 );
    r = se->RegisterGlobalFunction( "void RotateSet( int set_index, double x_rot_deg, double y_rot_deg, double z_rot_deg )", asFUNCTION( vsp::RotateSet ), asCALL_CDECL );
    assert( r >= 0 );
    r = se->RegisterGlobalFunction( "void TranslateSet( int set_index, vec3d translation_vec )", asFUNCTION( vsp::TranslateSet ), asCALL_CDECL );
    assert( r >= 0 );
    r = se->RegisterGlobalFunction( "void TransformSet( int set_index, vec3d translation_vec, double x_rot_deg, double y_rot_deg, double z_rot_deg, double scale, bool scale_translations_flag )", asFUNCTION( vsp::TransformSet ), asCALL_CDECL );
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
    r = se->RegisterGlobalFunction( "double SetParmVal(const string & in geom_id, const string & in name, const string & in group, double val )",
                                    asFUNCTIONPR( vsp::SetParmVal, ( const string &, const string &, const string &, double val ), double ), asCALL_CDECL );
    assert( r >= 0 );
    r = se->RegisterGlobalFunction( "double SetParmValUpdate(const string & in geom_id, const string & in parm_name, const string & in parm_group_name, double val )",
                                    asFUNCTIONPR( vsp::SetParmValUpdate, ( const string &, const string &, const string &, double val ), double ), asCALL_CDECL );
    assert( r >= 0 );
    r = se->RegisterGlobalFunction( "double GetParmVal(const string & in parm_id )", asFUNCTIONPR( vsp::GetParmVal, ( const string & ), double ), asCALL_CDECL );
    assert( r >= 0 );
    r = se->RegisterGlobalFunction( "double GetParmVal(const string & in geom_id, const string & in name, const string & in group )",
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
    r = se->RegisterGlobalFunction( "string FindParm( const string & in parm_container_id, const string & in parm_name, const string & in group_name )", asFUNCTION( vsp::FindParm ), asCALL_CDECL );
    assert( r >= 0 );

    //=== Parm Container Functions ===//
    r = se->RegisterGlobalFunction( "array<string>@  FindContainers()", asMETHOD( ScriptMgrSingleton, FindContainers ), asCALL_THISCALL_ASGLOBAL, &ScriptMgr );
    assert( r >= 0 );
    r = se->RegisterGlobalFunction( "array<string>@  FindContainersWithName( const string & in name )", asMETHOD( ScriptMgrSingleton, FindContainersWithName ), asCALL_THISCALL_ASGLOBAL, &ScriptMgr );
    assert( r >= 0 );
    r = se->RegisterGlobalFunction( "string FindContainer( const string & in name, int index )", asFUNCTION( vsp::FindContainer ), asCALL_CDECL );
    assert( r >= 0 );
    r = se->RegisterGlobalFunction( "string GetContainerName( const string & in parm_container_id )", asFUNCTION( vsp::GetContainerName ), asCALL_CDECL );
    assert( r >= 0 );
    r = se->RegisterGlobalFunction( "array<string>@  FindContainerGroupNames( const string & in parm_container_id )", asMETHOD( ScriptMgrSingleton, FindContainerGroupNames ), asCALL_THISCALL_ASGLOBAL, &ScriptMgr );
    assert( r >= 0 );
    r = se->RegisterGlobalFunction( "array<string>@  FindContainerParmIDs( const string & in parm_container_id )", asMETHOD( ScriptMgrSingleton, FindContainerParmIDs ), asCALL_THISCALL_ASGLOBAL, &ScriptMgr );
    assert( r >= 0 );

    //=== Register Snap To Functions ====//
    r = se->RegisterGlobalFunction( "double ComputeMinClearanceDistance( const string & in geom_id, int set )", asFUNCTION( vsp::ComputeMinClearanceDistance ), asCALL_CDECL );
    assert( r >= 0 );
    r = se->RegisterGlobalFunction( "double SnapParm( const string & in parm_id, double target_min_dist, bool inc_flag, int set  )", asFUNCTION( vsp::SnapParm ), asCALL_CDECL );
    assert( r >= 0 );

    //=== Register Var Preset Functions ====//
    r = se->RegisterGlobalFunction( "void AddVarPresetGroup( const string & in group_name )", asFUNCTION( vsp::AddVarPresetGroup ), asCALL_CDECL );
    assert( r >= 0 );
    r = se->RegisterGlobalFunction( "void AddVarPresetSetting( const string & in setting_name )", asFUNCTION( vsp::AddVarPresetSetting ), asCALL_CDECL );
    assert( r >= 0 );
    r = se->RegisterGlobalFunction( "void AddVarPresetParm( const string & in parm_ID )", asFUNCTIONPR( vsp::AddVarPresetParm, ( const string & ), void ), asCALL_CDECL );
    assert( r >= 0 );
    r = se->RegisterGlobalFunction( "void AddVarPresetParm( const string & in parm_ID, const string & in group_name )", asFUNCTIONPR( vsp::AddVarPresetParm, ( const string &, const string & ), void ), asCALL_CDECL );
    assert( r >= 0 );
    r = se->RegisterGlobalFunction( "void EditVarPresetParm( const string & in parm_ID, double parm_val )", asFUNCTIONPR( vsp::EditVarPresetParm, ( const string &, double ), void ), asCALL_CDECL );
    assert( r >= 0 );
    r = se->RegisterGlobalFunction( "void EditVarPresetParm( const string & in parm_ID, double parm_val, const string & in group_name, const string & in setting_name )", asFUNCTIONPR( vsp::EditVarPresetParm,
        ( const string &, double, const string &, const string & ), void ), asCALL_CDECL );
    assert( r >= 0 );
    r = se->RegisterGlobalFunction( "void DeleteVarPresetParm( const string & in parm_ID )", asFUNCTIONPR( vsp::DeleteVarPresetParm, ( const string & ), void ), asCALL_CDECL );
    assert( r >= 0 );
    r = se->RegisterGlobalFunction( "void DeleteVarPresetParm( const string & in parm_ID, const string & in group_name )", asFUNCTIONPR( vsp::DeleteVarPresetParm, ( const string &, const string & ), void ), asCALL_CDECL );
    assert( r >= 0 );
    r = se->RegisterGlobalFunction( "void SwitchVarPreset( const string & in group_name, const string & in setting_name )", asFUNCTION( vsp::SwitchVarPreset ), asCALL_CDECL );
    assert( r >= 0 );
    r = se->RegisterGlobalFunction( "bool DeleteVarPresetSet( const string & in group_name, const string & in setting_name )", asFUNCTION( vsp::DeleteVarPresetSet ), asCALL_CDECL );
    assert( r >= 0 );
    r = se->RegisterGlobalFunction( "string GetCurrentGroupName()", asFUNCTION( vsp::GetCurrentGroupName ), asCALL_CDECL );
    assert( r >= 0 );
    r = se->RegisterGlobalFunction( "string GetCurrentSettingName()", asFUNCTION( vsp::GetCurrentSettingName ), asCALL_CDECL );
    assert( r >= 0 );
    r = se->RegisterGlobalFunction( "array<string>@ GetVarPresetGroupNames()", asMETHOD( ScriptMgrSingleton, GetVarPresetGroupNames ), asCALL_THISCALL_ASGLOBAL, &ScriptMgr );
    assert( r >= 0 );
    r = se->RegisterGlobalFunction( "array<string>@ GetVarPresetSettingNamesWName( const string & in group_name )", asMETHOD( ScriptMgrSingleton, GetVarPresetSettingNamesWName ), asCALL_THISCALL_ASGLOBAL, &ScriptMgr );
    assert( r >= 0 );
    r = se->RegisterGlobalFunction( "array<string>@ GetVarPresetSettingNamesWIndex( int group_index )", asMETHOD( ScriptMgrSingleton, GetVarPresetSettingNamesWIndex ), asCALL_THISCALL_ASGLOBAL, &ScriptMgr );
    assert( r >= 0 );
    r = se->RegisterGlobalFunction( "array<double>@ GetVarPresetParmVals()", asMETHOD( ScriptMgrSingleton, GetVarPresetParmVals ), asCALL_THISCALL_ASGLOBAL, &ScriptMgr );
    assert( r >= 0 );
    r = se->RegisterGlobalFunction( "array<double>@ GetVarPresetParmValsWNames( const string & in group_name, const string & in setting_name )", asMETHOD( ScriptMgrSingleton, GetVarPresetParmValsWNames ), asCALL_THISCALL_ASGLOBAL, &ScriptMgr );
    assert( r >= 0 );
    r = se->RegisterGlobalFunction( "array<string>@ GetVarPresetParmIDs()", asMETHOD( ScriptMgrSingleton, GetVarPresetParmIDs ), asCALL_THISCALL_ASGLOBAL, &ScriptMgr );
    assert( r >= 0 );
    r = se->RegisterGlobalFunction( "array<string>@ GetVarPresetParmIDsWName( const string & in group_name )", asMETHOD( ScriptMgrSingleton, GetVarPresetParmIDsWName ), asCALL_THISCALL_ASGLOBAL, &ScriptMgr );
    assert( r >= 0 );

    //=== Register PCurve Functions ====//
    r = se->RegisterGlobalFunction( "void SetPCurve( const string& in geom_id, const int & in pcurveid, array<double>@ tvec, array<double>@ valvec, const int & in newtype )", asMETHOD( ScriptMgrSingleton, SetPCurve ), asCALL_THISCALL_ASGLOBAL, &ScriptMgr );
    assert( r >= 0 );
    r = se->RegisterGlobalFunction( "void PCurveConvertTo( const string & in geom_id, const int & in pcurveid, const int & in newtype )", asFUNCTION( vsp::PCurveConvertTo ), asCALL_CDECL );
    assert( r >= 0 );
    r = se->RegisterGlobalFunction( "int PCurveGetType( const string & in geom_id, const int & in pcurveid )", asFUNCTION( vsp::PCurveGetType ), asCALL_CDECL );
    assert( r >= 0 );
    r = se->RegisterGlobalFunction( "array<double>@ PCurveGetTVec( const string & in geom_id, const int & in pcurveid )", asMETHOD( ScriptMgrSingleton, PCurveGetTVec ), asCALL_THISCALL_ASGLOBAL, &ScriptMgr );
    assert( r >= 0 );
    r = se->RegisterGlobalFunction( "array<double>@ PCurveGetValVec( const string & in geom_id, const int & in pcurveid )", asMETHOD( ScriptMgrSingleton, PCurveGetValVec ), asCALL_THISCALL_ASGLOBAL, &ScriptMgr );
    assert( r >= 0 );
    r = se->RegisterGlobalFunction( "void PCurveDeletePt( const string & in geom_id, const int & in pcurveid, const int & in indx )", asFUNCTION( vsp::PCurveDeletePt ), asCALL_CDECL );
    assert( r >= 0 );
    r = se->RegisterGlobalFunction( "int PCurveSplit( const string & in geom_id, const int & in pcurveid, const double & in tsplit )", asFUNCTION( vsp::PCurveSplit ), asCALL_CDECL );
    assert( r >= 0 );

    //=== Register ParasiteDragTool Functions ====//
    r = se->RegisterGlobalFunction( "void AddExcrescence(const string & in excresName, const int & in excresType, const double & in excresVal)", asFUNCTION( vsp::AddExcrescence ), asCALL_CDECL );
    assert( r >= 0 );
    r = se->RegisterGlobalFunction( "void DeleteExcrescence(const int & in excresName)", asFUNCTION( vsp::DeleteExcrescence ), asCALL_CDECL );
    assert( r >= 0 );
    r = se->RegisterGlobalFunction( "void UpdateParasiteDrag()", asFUNCTION( vsp::UpdateParasiteDrag ), asCALL_CDECL );
    assert( r >= 0 );
    r = se->RegisterGlobalFunction( "void WriteAtmosphereCSVFile( const string & in file_name, const int & in atmos_type )", asFUNCTION( vsp::WriteAtmosphereCSVFile ), asCALL_CDECL );
    assert( r >= 0 );
    r = se->RegisterGlobalFunction( "void CalcAtmosphere( const double & in alt, const double & in delta_temp, const int & in atmos_type, double & out temp, double & out pres, double & out pres_ratio, double & out rho_ratio )", asFUNCTION( vsp::CalcAtmosphere ), asCALL_CDECL );
    assert( r >= 0 );
    r = se->RegisterGlobalFunction( "void WriteBodyFFCSVFile( const string & in file_name )", asFUNCTION( vsp::WriteBodyFFCSVFile ), asCALL_CDECL );
    assert( r >= 0 );
    r = se->RegisterGlobalFunction( "void WriteWingFFCSVFile( const string & in file_name )", asFUNCTION( vsp::WriteWingFFCSVFile ), asCALL_CDECL );
    assert( r >= 0 );
    r = se->RegisterGlobalFunction( "void WriteCfEqnCSVFile( const string & in file_name )", asFUNCTION( vsp::WriteCfEqnCSVFile ), asCALL_CDECL );
    assert( r >= 0 );
    r = se->RegisterGlobalFunction( "void WritePartialCfMethodCSVFile( const string & in file_name )", asFUNCTION( vsp::WritePartialCfMethodCSVFile ), asCALL_CDECL );
    assert( r >= 0 );

    //=== Register Surface Query Functions ===//
    r = se->RegisterGlobalFunction( "vec3d CompPnt01( const string & in geom_id, const int & in surf_indx, const double & in u, const double & in w )", asFUNCTION(vsp::CompPnt01), asCALL_CDECL);
    assert( r >= 0 );
    r = se->RegisterGlobalFunction( "vec3d CompNorm01( const string & in geom_id, const int & in surf_indx, const double & in u, const double & in w )", asFUNCTION(vsp::CompNorm01), asCALL_CDECL);
    assert( r >= 0 );
    r = se->RegisterGlobalFunction( "vec3d CompTanU01( const string & in geom_id, const int & in surf_indx, const double & in u, const double & in w )", asFUNCTION(vsp::CompTanU01), asCALL_CDECL);
    assert( r >= 0 );
    r = se->RegisterGlobalFunction( "vec3d CompTanW01( const string & in geom_id, const int & in surf_indx, const double & in u, const double & in w )", asFUNCTION(vsp::CompTanW01), asCALL_CDECL);
    assert( r >= 0 );
    r = se->RegisterGlobalFunction( "void CompCurvature01( const string & in geom_id, const int & in surf_indx, const double & in u, const double & in w, double & out k1, double & out k2, double & out ka, double & out kg )", asFUNCTION(vsp::CompCurvature01), asCALL_CDECL);
    assert( r >= 0 );
    r = se->RegisterGlobalFunction( "double ProjPnt01( const string & in geom_id, const int & in surf_indx, const vec3d & in pt, double & out u, double & out w )", asFUNCTION(vsp::ProjPnt01), asCALL_CDECL);
    assert( r >= 0 );
    r = se->RegisterGlobalFunction( "double ProjPnt01I( const string & in geom_id, const vec3d & in pt, int & out surf_indx, double & out u, double & out w )", asFUNCTION(vsp::ProjPnt01I), asCALL_CDECL);
    assert( r >= 0 );
    r = se->RegisterGlobalFunction( "double ProjPnt01Guess( const string & in geom_id, const int & in surf_indx, const vec3d & in pt, const double & in u0, const double & in w0, double & out u, double & out w )", asFUNCTION(vsp::ProjPnt01Guess), asCALL_CDECL);
    assert( r >= 0 );
    r = se->RegisterGlobalFunction( "void GetUWTess01(const string & in geom_id, int & in surf_indx, array<double>@ us, array<double>@ ws )", asMETHOD( ScriptMgrSingleton, GetUWTess01 ), asCALL_THISCALL_ASGLOBAL, &ScriptMgr );
    assert( r >= 0 );
    r = se->RegisterGlobalFunction( "array<vec3d>@ CompVecPnt01(const string & in geom_id, const int & in surf_indx, array<double>@ us, array<double>@ ws )", asMETHOD( ScriptMgrSingleton, CompVecPnt01 ), asCALL_THISCALL_ASGLOBAL, &ScriptMgr );
    assert( r >= 0 );
    r = se->RegisterGlobalFunction( "array<vec3d>@ CompVecNorm01(const string & in geom_id, const int & in surf_indx, array<double>@ us, array<double>@ws )", asMETHOD( ScriptMgrSingleton, CompVecNorm01 ), asCALL_THISCALL_ASGLOBAL, &ScriptMgr );
    assert( r >= 0 );
    r = se->RegisterGlobalFunction( "void CompVecCurvature01(const string & in geom_id, const int & in surf_indx, array<double>@ us, array<double>@ ws, array<double>@ k1s, array<double>@ k2s, array<double>@ kas, array<double>@ kgs)", asMETHOD( ScriptMgrSingleton, CompVecCurvature01 ), asCALL_THISCALL_ASGLOBAL, &ScriptMgr );
    assert( r >= 0 );
    r = se->RegisterGlobalFunction( "void ProjVecPnt01(const string & in geom_id, int & in surf_indx, array<vec3d>@ pts, array<double>@ us, array<double>@ ws, array<double>@ ds )", asMETHOD( ScriptMgrSingleton, ProjVecPnt01 ), asCALL_THISCALL_ASGLOBAL, &ScriptMgr );
    assert( r >= 0 );
    r = se->RegisterGlobalFunction( "void ProjVecPnt01Guess(const string & in geom_id, int & in surf_indx, array<vec3d>@ pts, array<double>@ u0s, array<double>@ w0s, array<double>@ us, array<double>@ ws, array<double>@ ds )", asMETHOD( ScriptMgrSingleton, ProjVecPnt01Guess ), asCALL_THISCALL_ASGLOBAL, &ScriptMgr );
    assert( r >= 0 );

    //=== Register Measure Functions ===//
    r = se->RegisterGlobalFunction( "string AddRuler( const string & in startgeomid, int startsurfindx, double startu, double startw, const string & in endgeomid, int endsurfindx, double endu, double endw, const string & in name )", asFUNCTION( vsp::AddRuler ), asCALL_CDECL );
    assert( r >= 0 );
    r = se->RegisterGlobalFunction( "array<string>@ GetAllRulers()", asMETHOD( ScriptMgrSingleton, GetAllRulers ), asCALL_THISCALL_ASGLOBAL, &ScriptMgr );
    assert( r >= 0 );
    r = se->RegisterGlobalFunction( "void DelRuler( const string & in id )", asFUNCTION( vsp::DelRuler ), asCALL_CDECL );
    assert( r >= 0 );
    r = se->RegisterGlobalFunction( "void DeleteAllRulers()", asFUNCTION( vsp::DeleteAllRulers ), asCALL_CDECL );
    assert( r >= 0 );
    r = se->RegisterGlobalFunction( "string AddProbe( const string & in geomid, int surfindx, double u, double w, const string & in name )", asFUNCTION( vsp::AddProbe ), asCALL_CDECL );
    assert( r >= 0 );
    r = se->RegisterGlobalFunction( "array<string>@ GetAllProbes()", asMETHOD( ScriptMgrSingleton, GetAllProbes ), asCALL_THISCALL_ASGLOBAL, &ScriptMgr );
    assert( r >= 0 );
    r = se->RegisterGlobalFunction( "void DelProbe( const string & in id )", asFUNCTION( vsp::DelProbe ), asCALL_CDECL );
    assert( r >= 0 );
    r = se->RegisterGlobalFunction( "void DeleteAllProbes()", asFUNCTION( vsp::DeleteAllProbes ), asCALL_CDECL );
    assert( r >= 0 );

    //=== Register FeaStructure and FEA Mesh Functions ====//
    r = se->RegisterGlobalFunction( "int AddFeaStruct( const string & in geom_id, bool init_skin = true, int surfindex = 0 )", asFUNCTION( vsp::AddFeaStruct ), asCALL_CDECL );
    assert( r >= 0 );
    r = se->RegisterGlobalFunction( "void DeleteFeaStruct( const string & in geom_id, int fea_struct_ind )", asFUNCTION( vsp::DeleteFeaStruct ), asCALL_CDECL );
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
    r = se->RegisterGlobalFunction( "void SetFeaPartName( const string & in part_id, const string & in name )", asFUNCTION( vsp::SetFeaPartName ), asCALL_CDECL );
    assert( r >= 0 );
    r = se->RegisterGlobalFunction( "void SetFeaMeshVal( const string & in geom_id, int fea_struct_ind, int type, double val )", asFUNCTION( vsp::SetFeaMeshVal ), asCALL_CDECL );
    assert( r >= 0 );
    r = se->RegisterGlobalFunction( "void SetFeaMeshFileName( const string & in geom_id, int fea_struct_id, int file_type, const string & in file_name )", asFUNCTION( vsp::SetFeaMeshFileName ), asCALL_CDECL );
    assert( r >= 0 );
    r = se->RegisterGlobalFunction( "void ComputeFeaMesh( const string & in geom_id, int fea_struct_ind, int file_type )", asFUNCTIONPR( vsp::ComputeFeaMesh, ( const string & in, int, int ), void ), asCALL_CDECL );
    assert( r >= 0 );
    r = se->RegisterGlobalFunction( "void ComputeFeaMesh( const string & in struct_id, int file_type )", asFUNCTIONPR( vsp::ComputeFeaMesh, ( const string & in, int ), void ), asCALL_CDECL );
    assert( r >= 0 );
    r = se->RegisterGlobalFunction( "string AddFeaPart( const string & in geom_id, int fea_struct_id, int type )", asFUNCTION( vsp::AddFeaPart ), asCALL_CDECL );
    assert( r >= 0 );
    r = se->RegisterGlobalFunction( "string DeleteFeaPart( const string & in geom_id, int fea_struct_id, const string & in part_id )", asFUNCTION( vsp::DeleteFeaPart ), asCALL_CDECL );
    assert( r >= 0 );
    r = se->RegisterGlobalFunction( "string AddFeaSubSurf( const string & in geom_id, int fea_struct_id, int type )", asFUNCTION( vsp::AddFeaSubSurf ), asCALL_CDECL );
    assert( r >= 0 );
    r = se->RegisterGlobalFunction( "string DeleteFeaSubSurf( const string & in geom_id, int fea_struct_id, const string & in ss_id )", asFUNCTION( vsp::DeleteFeaSubSurf ), asCALL_CDECL );
    assert( r >= 0 );
    r = se->RegisterGlobalFunction( "string AddFeaMaterial()", asFUNCTION( vsp::AddFeaMaterial ), asCALL_CDECL );
    assert( r >= 0 );
    r = se->RegisterGlobalFunction( "string AddFeaProperty( int property_type = 0 )", asFUNCTION( vsp::AddFeaProperty ), asCALL_CDECL );
    assert( r >= 0 );
}

void ScriptMgrSingleton::RegisterUtility( asIScriptEngine* se )
{
    //==== Register Utility Functions ====//
    int r;
    r = se->RegisterGlobalFunction( "void Print(const string & in data, bool new_line = true )", asMETHODPR( ScriptMgrSingleton, Print, (const string &, bool), void ), asCALL_THISCALL_ASGLOBAL, &ScriptMgr );
    assert( r >= 0 );
    r = se->RegisterGlobalFunction( "void Print(const vec3d & in data, bool new_line = true )", asMETHODPR( ScriptMgrSingleton, Print, (const vec3d &, bool), void ), asCALL_THISCALL_ASGLOBAL, &ScriptMgr );
    assert( r >= 0 );
    r = se->RegisterGlobalFunction( "void Print(double data, bool new_line = true )", asMETHODPR( ScriptMgrSingleton, Print, (double, bool), void ), asCALL_THISCALL_ASGLOBAL, &ScriptMgr );
    assert( r >= 0 );
    r = se->RegisterGlobalFunction( "void Print(int data, bool new_line = true )", asMETHODPR( ScriptMgrSingleton, Print, (int, bool), void ), asCALL_THISCALL_ASGLOBAL, &ScriptMgr );
    assert( r >= 0 );
    r = se->RegisterGlobalFunction( "double Min( double x, double y)", asMETHOD( ScriptMgrSingleton, Min ), asCALL_THISCALL_ASGLOBAL, &ScriptMgr );
    assert( r >= 0 );
    r = se->RegisterGlobalFunction( "double Max( double x, double y)", asMETHOD( ScriptMgrSingleton, Max ), asCALL_THISCALL_ASGLOBAL, &ScriptMgr );
    assert( r >= 0 );
    r = se->RegisterGlobalFunction( "double Rad2Deg( double r )", asMETHOD( ScriptMgrSingleton, Rad2Deg ), asCALL_THISCALL_ASGLOBAL, &ScriptMgr );
    assert( r >= 0 );
    r = se->RegisterGlobalFunction( "double Deg2Rad( double d )", asMETHOD( ScriptMgrSingleton, Deg2Rad ), asCALL_THISCALL_ASGLOBAL, &ScriptMgr );
    assert( r >= 0 );
    r = se->RegisterGlobalFunction( "string GetVSPVersion( )", asMETHOD( ScriptMgrSingleton, GetVSPVersion ), asCALL_THISCALL_ASGLOBAL, &ScriptMgr );
    assert( r >= 0 );
    r = se->RegisterGlobalFunction( "string GetVSPExePath()", asFUNCTION( vsp::GetVSPExePath ), asCALL_CDECL );
    assert( r >= 0 );
    r = se->RegisterGlobalFunction( "void VSPCheckSetup()", asFUNCTION( vsp::VSPCheckSetup ), asCALL_CDECL );
    assert( r >= 0 );
    r = se->RegisterGlobalFunction( "void VSPRenew()", asFUNCTION( vsp::VSPRenew ), asCALL_CDECL );
    assert( r >= 0 );

    //====  Register Proxy Utility Functions ====//
    r = se->RegisterGlobalFunction( "array<vec3d>@ GetProxyVec3dArray()", asMETHOD( ScriptMgrSingleton, GetProxyVec3dArray ), asCALL_THISCALL_ASGLOBAL, &ScriptMgr );
    assert( r >= 0 );
    r = se->RegisterGlobalFunction( "void SetVec3dArray( array<vec3d>@ arr )", asMETHOD( ScriptMgrSingleton, SetVec3dArray ), asCALL_THISCALL_ASGLOBAL, &ScriptMgr );
    assert( r >= 0 );
}


void ScriptMgrSingleton::SetVec3dArray( CScriptArray* varr )
{
    vector< vec3d > vec;
    vec.resize( varr->GetSize() );
    for ( int i = 0 ; i < ( int )varr->GetSize() ; i++ )
    {
        vec[i] = * ( vec3d* )( varr->At( i ) );
    }


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

void ScriptMgrSingleton::FillDoubleArray( vector < double > & in, CScriptArray* out )
{
    out->Resize( in.size() );
    for ( int i = 0 ; i < ( int )in.size() ; i++ )
    {
        out->SetValue( i, &in[i] );
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

    int_vec.resize( selected->GetSize() );
    for ( int i = 0; i < (int)selected->GetSize(); i++ )
    {
        int_vec[i] = *(int*)( selected->At( i ) );
    }

    vsp::AddSelectedToCSGroup( int_vec, CSGroupIndex );
}

void ScriptMgrSingleton::RemoveSelectedFromCSGroup( CScriptArray* selected, int CSGroupIndex )
{
    vector < int > int_vec;

    int_vec.resize( selected->GetSize() );
    for ( int i = 0; i < (int)selected->GetSize(); i++ )
    {
        int_vec[i] = *(int*)( selected->At( i ) );
    }

    vsp::RemoveSelectedFromCSGroup( int_vec, CSGroupIndex );
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

void ScriptMgrSingleton::DeleteGeomVec( CScriptArray* del_arr )
{
    vector < string > del_vec;

    del_vec.resize( del_arr->GetSize() );
    for ( int i = 0 ; i < ( int )del_arr->GetSize() ; i++ )
    {
        del_vec[i] = * ( string* )( del_arr->At( i ) );
    }

    vsp::DeleteGeomVec( del_vec );
}

void ScriptMgrSingleton::SetXSecPnts( const string& xsec_id, CScriptArray* pnt_arr )
{
    vector< vec3d > pnt_vec;
    pnt_vec.resize( pnt_arr->GetSize() );
    for ( int i = 0 ; i < ( int )pnt_arr->GetSize() ; i++ )
    {
        pnt_vec[i] = * ( vec3d* )( pnt_arr->At( i ) );
    }
    vsp::SetXSecPnts( xsec_id, pnt_vec );
}

void ScriptMgrSingleton::SetAirfoilPnts( const string& xsec_id, CScriptArray* up_pnt_arr, CScriptArray* low_pnt_arr )
{
    vector< vec3d > up_pnt_vec;
    up_pnt_vec.resize( up_pnt_arr->GetSize() );
    for ( int i = 0 ; i < ( int )up_pnt_arr->GetSize() ; i++ )
    {
        up_pnt_vec[i] = * ( vec3d* )( up_pnt_arr->At( i ) );
    }

    vector< vec3d > low_pnt_vec;
    low_pnt_vec.resize( low_pnt_arr->GetSize() );
    for ( int i = 0 ; i < ( int )low_pnt_arr->GetSize() ; i++ )
    {
        low_pnt_vec[i] = * ( vec3d* )( low_pnt_arr->At( i ) );
    }

    vsp::SetAirfoilPnts( xsec_id, up_pnt_vec, low_pnt_vec );
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
    xyz_vec.resize( xyzdata->GetSize() );
    for ( int i = 0; i < (int)xyzdata->GetSize(); i++ )
    {
        xyz_vec[i] = *(vec3d*)( xyzdata->At( i ) );
    }

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
    surf_pnt_vec.resize( surf_pnt_arr->GetSize() );
    for ( int i = 0; i < (int)surf_pnt_arr->GetSize(); i++ )
    {
        surf_pnt_vec[i] = *(vec3d*)( surf_pnt_arr->At( i ) );
    }

    m_ProxyDoubleArray = vsp::GetEllipsoidCpDist( surf_pnt_vec, abc_rad, V_inf );

    return GetProxyDoubleArray();
}

void ScriptMgrSingleton::SetUpperCST( const string& xsec_id, int deg, CScriptArray* coefs_arr )
{
    vector < double > coefs_vec;

    coefs_vec.resize( coefs_arr->GetSize() );
    for ( int i = 0 ; i < ( int )coefs_arr->GetSize() ; i++ )
    {
        coefs_vec[i] = * ( double* )( coefs_arr->At( i ) );
    }

    vsp::SetUpperCST( xsec_id, deg, coefs_vec );
}

void ScriptMgrSingleton::SetLowerCST( const string& xsec_id, int deg, CScriptArray* coefs_arr )
{
    vector < double > coefs_vec;

    coefs_vec.resize( coefs_arr->GetSize() );
    for ( int i = 0 ; i < ( int )coefs_arr->GetSize() ; i++ )
    {
        coefs_vec[i] = * ( double* )( coefs_arr->At( i ) );
    }

    vsp::SetLowerCST( xsec_id, deg, coefs_vec );
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

void ScriptMgrSingleton::SetEditXSecPnts( const string & xsec_id, CScriptArray* u_vec, CScriptArray* control_pts )
{
    vector < vec3d > control_pnt_vec( control_pts->GetSize() );

    for ( int i = 0; i < (int)control_pts->GetSize(); i++ )
    {
        control_pnt_vec[i] = *(vec3d*)( control_pts->At( i ) );
    }

    vector < double > new_u_vec( u_vec->GetSize() );

    for ( int i = 0; i < (int)u_vec->GetSize(); i++ )
    {
        new_u_vec[i] = *(double*)( u_vec->At( i ) );
    }

    vsp::SetEditXSecPnts( xsec_id, new_u_vec, control_pnt_vec );
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

    t_vec.resize( t_arr->GetSize() );
    for ( int i = 0 ; i < ( int )t_arr->GetSize() ; i++ )
    {
        t_vec[i] = * ( double* )( t_arr->At( i ) );
    }

    vector < double > val_vec;

    val_vec.resize( t_arr->GetSize() );
    for ( int i = 0 ; i < ( int )val_arr->GetSize() ; i++ )
    {
        val_vec[i] = * ( double* )( val_arr->At( i ) );
    }

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

void ScriptMgrSingleton::SetIntAnalysisInput( const string& analysis, const string & name, CScriptArray* indata, int index )
{
    vector < int > indata_vec;

    indata_vec.resize( indata->GetSize() );
    for ( int i = 0 ; i < ( int )indata->GetSize() ; i++ )
    {
        indata_vec[i] = * ( int* )( indata->At( i ) );
    }

    vsp::SetIntAnalysisInput( analysis, name, indata_vec, index );
}

void ScriptMgrSingleton::SetDoubleAnalysisInput( const string& analysis, const string & name, CScriptArray* indata, int index )
{
    vector < double > indata_vec;

    indata_vec.resize( indata->GetSize() );
    for ( int i = 0 ; i < ( int )indata->GetSize() ; i++ )
    {
        indata_vec[i] = * ( double* )( indata->At( i ) );
    }

    vsp::SetDoubleAnalysisInput( analysis, name, indata_vec, index );
}

void ScriptMgrSingleton::SetStringAnalysisInput( const string& analysis, const string & name, CScriptArray* indata, int index )
{
    vector < string > indata_vec;

    indata_vec.resize( indata->GetSize() );
    for ( int i = 0 ; i < ( int )indata->GetSize() ; i++ )
    {
        indata_vec[i] = * ( string* )( indata->At( i ) );
    }

    vsp::SetStringAnalysisInput( analysis, name, indata_vec, index );
}

void ScriptMgrSingleton::SetVec3dAnalysisInput( const string& analysis, const string & name, CScriptArray* indata, int index )
{
    vector < vec3d > indata_vec;

    indata_vec.resize( indata->GetSize() );
    for ( int i = 0 ; i < ( int )indata->GetSize() ; i++ )
    {
        indata_vec[i] = * ( vec3d* )( indata->At( i ) );
    }

    vsp::SetVec3dAnalysisInput( analysis, name, indata_vec, index );
}

CScriptArray* ScriptMgrSingleton::CompVecPnt01(const string &geom_id, const int &surf_indx, CScriptArray* us, CScriptArray* ws)
{
    vector < double > in_us;
    in_us.resize( us->GetSize() );
    for ( int i = 0 ; i < ( int )us->GetSize() ; i++ )
    {
        in_us[i] = * ( double* )( us->At( i ) );
    }

    vector < double > in_ws;
    in_ws.resize( ws->GetSize() );
    for ( int i = 0 ; i < ( int )ws->GetSize() ; i++ )
    {
        in_ws[i] = * ( double* )( ws->At( i ) );
    }

    m_ProxyVec3dArray = vsp::CompVecPnt01( geom_id, surf_indx, in_us, in_ws );
    return GetProxyVec3dArray();
}

CScriptArray* ScriptMgrSingleton::CompVecNorm01(const string &geom_id, const int &surf_indx, CScriptArray* us, CScriptArray* ws)
{
    vector < double > in_us;
    in_us.resize( us->GetSize() );
    for ( int i = 0 ; i < ( int )us->GetSize() ; i++ )
    {
        in_us[i] = * ( double* )( us->At( i ) );
    }

    vector < double > in_ws;
    in_ws.resize( ws->GetSize() );
    for ( int i = 0 ; i < ( int )ws->GetSize() ; i++ )
    {
        in_ws[i] = * ( double* )( ws->At( i ) );
    }

    m_ProxyVec3dArray = vsp::CompVecNorm01( geom_id, surf_indx, in_us, in_ws );
    return GetProxyVec3dArray();
}

void ScriptMgrSingleton::CompVecCurvature01(const string &geom_id, const int &surf_indx, CScriptArray* us, CScriptArray* ws, CScriptArray* k1s, CScriptArray* k2s, CScriptArray* kas, CScriptArray* kgs)
{
    vector < double > in_us;
    in_us.resize( us->GetSize() );
    for ( int i = 0 ; i < ( int )us->GetSize() ; i++ )
    {
        in_us[i] = * ( double* )( us->At( i ) );
    }

    vector < double > in_ws;
    in_ws.resize( ws->GetSize() );
    for ( int i = 0 ; i < ( int )ws->GetSize() ; i++ )
    {
        in_ws[i] = * ( double* )( ws->At( i ) );
    }

    vector < double > out_k1s;
    vector < double > out_k2s;
    vector < double > out_kas;
    vector < double > out_kgs;

    vsp::CompVecCurvature01( geom_id, surf_indx, in_us, in_ws, out_k1s, out_k2s, out_kas, out_kgs );

    FillDoubleArray( out_k1s, k1s );
    FillDoubleArray( out_k2s, k2s );
    FillDoubleArray( out_kas, kas );
    FillDoubleArray( out_kgs, kgs );
}

void ScriptMgrSingleton::ProjVecPnt01(const string &geom_id, int &surf_indx, CScriptArray* pts, CScriptArray* us, CScriptArray* ws, CScriptArray* ds )
{
    vector < vec3d > in_pts;

    in_pts.resize( pts->GetSize() );
    for ( int i = 0 ; i < ( int )pts->GetSize() ; i++ )
    {
        in_pts[i] = * ( vec3d* )( pts->At( i ) );
    }

    vector < double > out_us;
    vector < double > out_ws;
    vector < double > out_ds;

    vsp::ProjVecPnt01( geom_id, surf_indx, in_pts, out_us, out_ws, out_ds );

    FillDoubleArray( out_us, us );
    FillDoubleArray( out_ws, ws );
    FillDoubleArray( out_ds, ds );
}

void ScriptMgrSingleton::ProjVecPnt01Guess(const string &geom_id, int &surf_indx, CScriptArray* pts, CScriptArray* u0s, CScriptArray* w0s, CScriptArray* us, CScriptArray* ws, CScriptArray* ds )
{
    vector < vec3d > in_pts;

    in_pts.resize( pts->GetSize() );
    for ( int i = 0 ; i < ( int )pts->GetSize() ; i++ )
    {
        in_pts[i] = * ( vec3d* )( pts->At( i ) );
    }

    vector < double > in_u0s;
    in_u0s.resize( u0s->GetSize() );
    for ( int i = 0 ; i < ( int )u0s->GetSize() ; i++ )
    {
        in_u0s[i] = * ( double* )( u0s->At( i ) );
    }

    vector < double > in_w0s;
    in_w0s.resize( w0s->GetSize() );
    for ( int i = 0 ; i < ( int )w0s->GetSize() ; i++ )
    {
        in_w0s[i] = * ( double* )( w0s->At( i ) );
    }

    vector < double > out_us;
    vector < double > out_ws;
    vector < double > out_ds;

    vsp::ProjVecPnt01Guess( geom_id, surf_indx, in_pts, in_u0s, in_w0s, out_us, out_ws, out_ds );

    FillDoubleArray( out_us, us );
    FillDoubleArray( out_ws, ws );
    FillDoubleArray( out_ds, ds );
}

void ScriptMgrSingleton::GetUWTess01(const string &geom_id, int &surf_indx, CScriptArray* us, CScriptArray* ws )
{
    vector < double > out_us;
    vector < double > out_ws;

    vsp::GetUWTess01( geom_id, surf_indx, out_us, out_ws );

    FillDoubleArray( out_us, us );
    FillDoubleArray( out_ws, ws );
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
