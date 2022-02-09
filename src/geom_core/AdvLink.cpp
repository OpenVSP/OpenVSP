//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//
// AdvLink.cpp: implementation of the AdvLink classes.
//
//////////////////////////////////////////////////////////////////////

#include "AdvLink.h"
#include "AdvLinkMgr.h"
#include "ParmMgr.h"
#include "ScriptMgr.h"
#include "APIErrorMgr.h"

//===== Encode Variable Def =====//
xmlNodePtr VarDef::EncodeXml( xmlNodePtr & node )
{
    xmlNodePtr var_node =  xmlNewChild( node, NULL, BAD_CAST"VarDef", NULL );

    XmlUtil::SetStringProp( var_node, "Name", m_VarName );
    XmlUtil::SetStringProp( var_node, "ParmID", m_ParmID );

    return var_node;
}

//===== Dencode Variable Def =====//
xmlNodePtr VarDef::DecodeXml( xmlNodePtr & var_node )
{
    if ( var_node )
    {
        m_VarName = XmlUtil::FindStringProp( var_node, "Name", m_VarName );
        m_ParmID  = ParmMgr.RemapID( XmlUtil::FindStringProp( var_node, "ParmID", string() ) );

    }
    return var_node;
}

//=====================================================================================//
//=====================================================================================//
//=====================================================================================//

//==== Constructor ====//
AdvLink::AdvLink()
{
    m_ValidScript = false;
}

//==== Destructor ====//
AdvLink::~AdvLink()
{

}

bool AdvLink::ValidateParms()
{
    MessageData errMsgData;
    errMsgData.m_String = "Error";

    // Check if Parms still exist. If not, delete the variable and issue a warning to the user.
    bool all_valid_flag = true;

    vector < VarDef > valid_input_vars;
    for ( int i = 0 ; i < (int)m_InputVars.size() ; i++ )
    {
        if ( ParmMgr.FindParm( m_InputVars[i].m_ParmID ) )
        {
            valid_input_vars.push_back( m_InputVars[i] );
        }
        else
        {
            errMsgData.m_IntVec.push_back( vsp::VSP_CANT_FIND_PARM );
            char buf[255];
            sprintf( buf, "Error: Advanced Link Input Variable %s (ID: %s) No Longer Exists\n", m_OutputVars[i].m_VarName.c_str(), m_OutputVars[i].m_ParmID.c_str() );
            errMsgData.m_StringVec.emplace_back( string( buf ) );
            all_valid_flag = false;
        }
    }

    m_InputVars.clear();
    m_InputVars = valid_input_vars;

    vector < VarDef > valid_output_vars;

    for ( int i = 0 ; i < (int)m_OutputVars.size() ; i++ )
    {
        if ( ParmMgr.FindParm( m_OutputVars[i].m_ParmID ) )
        {
            valid_output_vars.push_back( m_OutputVars[i] );
        }
        else
        {
            errMsgData.m_IntVec.push_back( vsp::VSP_CANT_FIND_PARM );
            char buf[255];
            sprintf( buf, "Error: Advanced Link Output Variable %s (ID: %s) No Longer Exists\n", m_OutputVars[i].m_VarName.c_str(), m_OutputVars[i].m_ParmID.c_str() );
            errMsgData.m_StringVec.emplace_back( string( buf ) );
            all_valid_flag = false;
        }
    }

    m_OutputVars.clear();
    m_OutputVars = valid_output_vars;

    if ( !all_valid_flag )
    {
        MessageMgr::getInstance().SendAll( errMsgData );
        m_ValidScript = false;
    }

    return all_valid_flag;
}

bool AdvLink::DuplicateVarName( const string & name )
{
    for ( int i = 0 ; i < (int)m_InputVars.size() ; i++ )
    {
        if ( m_InputVars[i].m_VarName == name )
            return true;
    }
    for ( int i = 0 ; i < (int)m_OutputVars.size() ; i++ )
    {
        if ( m_OutputVars[i].m_VarName == name )
            return true;
    }
    return false;
}

void AdvLink::AddVar( const VarDef & pd, bool input_flag )
{
    if ( input_flag )
        m_InputVars.push_back( pd );
    else
        m_OutputVars.push_back( pd );

}

void AdvLink::DeleteVar( int index, bool input_flag )
{
    if ( input_flag && index >= 0 && index < (int)m_InputVars.size() )
    {
        m_InputVars.erase( m_InputVars.begin() + index );
    }
    else if ( !input_flag && index >= 0 && index < (int)m_OutputVars.size() )
    {
        m_OutputVars.erase( m_OutputVars.begin() + index );
    }
}

void AdvLink::DeleteAllVars( bool input_flag )
{
    if ( input_flag )
    {
        m_InputVars.clear();
    }
    else
    {
        m_OutputVars.clear();
    }
}

void AdvLink::SetVar( const string & var_name, double val )
{
    //==== Look For Var ====//
    for ( int i = 0 ; i < (int)m_OutputVars.size() ; i++ )
    {
        if ( m_OutputVars[i].m_VarName == var_name )
        {
            Parm* parm_ptr = ParmMgr.FindParm(  m_OutputVars[i].m_ParmID );
            if ( parm_ptr && val > -1.0e15 && !parm_ptr->GetLinkUpdateFlag() )
            {
                parm_ptr->SetFromLink( val );
                break;
            }
        }
    }
}

double AdvLink::GetVar( const string & var_name )
{
    //==== Look For Var ====//
    for ( int i = 0 ; i < (int)m_InputVars.size() ; i++ )
    {
        if ( m_InputVars[i].m_VarName == var_name )
        {
            Parm* parm_ptr = ParmMgr.FindParm(  m_InputVars[i].m_ParmID );
            if ( parm_ptr )
            {
                return parm_ptr->Get();
            }
        }
    }
    return 0.0;
}

bool AdvLink::BuildScript()
{
    string script;

    m_ValidScript = false;

    //==== Find All Var Names ====//
    vector< string > var_vec;
    for ( int i = 0 ; i < (int)m_InputVars.size() ; i++ )
    {
        var_vec.push_back( m_InputVars[i].m_VarName );
    }
    for ( int i = 0 ; i < (int)m_OutputVars.size() ; i++ )
    {
        var_vec.push_back( m_OutputVars[i].m_VarName );
    }

    //==== Write Comment ====//
    string comment;
    comment = "//==== Adv Link: " + m_Name + " Generated Script ====//\n\n";
    script.append( comment );

    //==== Write Vars ====//
    for ( int i = 0 ; i < (int)m_InputVars.size() ; i++ )
    {
        string vdef = "double " + m_InputVars[i].m_VarName + " =  0;\n";
        script.append( vdef );
    }
    for ( int i = 0 ; i < (int)m_OutputVars.size() ; i++ )
    {
        string vdef = "double " + m_OutputVars[i].m_VarName + " =  -1.0e15;\n";
        script.append( vdef );
    }

    //==== Load Input ====//
    script.append( "\nvoid LoadInput()\n{\n" );
    for ( int i = 0 ; i < (int)m_InputVars.size() ; i++ )
    {
        string idef = "   " + m_InputVars[i].m_VarName + " = GetVar( \"" + m_InputVars[i].m_VarName + "\" );\n";
        script.append( idef );
    }
    script.append( "}\n\n" );

    //==== Load Output ====//
    script.append( "void LoadOutput()\n{\n" );
    for ( int i = 0 ; i < (int)m_OutputVars.size() ; i++ )
    {
        string idef = "   SetVar( \"" + m_OutputVars[i].m_VarName + "\", " + m_OutputVars[i].m_VarName + " );\n";
        script.append( idef );
    }
    script.append( "}\n\n" );

    script.append( "void UpdateLink()\n{\n" );
    script.append( "    LoadInput();\n\n" );
    script.append( m_ScriptCode );
    script.append( "\n    LoadOutput();\n" );
    script.append( "\n    Update();\n}\n" );

    m_CompleteScript = script;

    string module_name = "AdvLink_" + m_Name;

    ScriptMgr.RemoveScript( module_name );

    ScriptMgr.ClearMessages();
    m_ScriptModule = ScriptMgr.ReadScriptFromMemory( module_name, m_CompleteScript );

    if ( m_ScriptModule.size() == 0 )
    {
        m_ScriptErrors = ScriptMgr.GetMessages();

        MessageData errMsgData;
        errMsgData.m_String = "Error";

        errMsgData.m_IntVec.push_back( vsp::VSP_ADV_LINK_BUILD_FAIL );
        errMsgData.m_StringVec.emplace_back( m_ScriptErrors );

        // Temporarially suppress error manager printing because ScriptMgr.ReadScriptFromMemory
        // will have already dumped the same errors to the console.  We don't want to eliminate
        // that print site, so instead we silence this one.
        vsp::ErrorMgr.SilenceErrors();
        MessageMgr::getInstance().SendAll( errMsgData );
        vsp::ErrorMgr.PrintOnErrors();

        return false;
    }

    m_ValidScript = true;
    return true;
}

bool AdvLink::UpdateLink( const string & pid )
{
    //==== Check Parm  ====//
    Parm* parm_ptr = ParmMgr.FindParm( pid );
    if ( !parm_ptr )
        return false;
 
    //==== Look For Var ====//
    bool run_link = false;
    for ( int i = 0 ; i < (int)m_InputVars.size() ; i++ )
    {
        if ( m_InputVars[i].m_ParmID == pid  )
            run_link = true;
    }

    if ( !run_link )
        return false;

    AdvLinkMgr.SetActiveLink( this );

    //==== Call Script ====//
    ScriptMgr.ExecuteScript( m_ScriptModule.c_str(), "void UpdateLink()" );

    return true;
}

void AdvLink::ForceUpdate()
{
    AdvLinkMgr.SetActiveLink( this );

    //==== Call Script ====//
    ScriptMgr.ExecuteScript( m_ScriptModule.c_str(), "void UpdateLink()" );
}

//==== Encode Contents of Adv Link Into XML Tree ====//
xmlNodePtr AdvLink::EncodeXml( xmlNodePtr & node )
{
    xmlNodePtr adv_link_node = xmlNewChild( node, NULL, BAD_CAST"AdvLink", NULL );

    XmlUtil::SetStringProp( adv_link_node, "Name", m_Name );
    string safe_script_code = XmlUtil::ConvertToXMLSafeChars( m_ScriptCode );
    XmlUtil::AddStringNode( adv_link_node, "ScriptCode", safe_script_code  );

    xmlNodePtr input_vars_node = xmlNewChild( adv_link_node, NULL, BAD_CAST"InputVars", NULL );
    for ( int i = 0 ; i < (int)m_InputVars.size() ; i++ )
    {
        m_InputVars[i].EncodeXml( input_vars_node );
    }
    xmlNodePtr output_vars_node = xmlNewChild( adv_link_node, NULL, BAD_CAST"OutputVars", NULL );
    for ( int i = 0 ; i < (int)m_OutputVars.size() ; i++ )
    {
        m_OutputVars[i].EncodeXml( output_vars_node );
    }
    return adv_link_node;
}

//==== Decode Contents of Adv Link From XML Tree ====//
xmlNodePtr AdvLink::DecodeXml( xmlNodePtr & adv_link_node )
{
    if ( adv_link_node )
    {
        m_Name = XmlUtil::FindStringProp( adv_link_node, "Name", m_Name );
        string safe_script_code = XmlUtil::FindString( adv_link_node, "ScriptCode", string() );
        m_ScriptCode = XmlUtil::ConvertFromXMLSafeChars( safe_script_code );

        //==== Input Vars ====//
        xmlNodePtr input_node = XmlUtil::GetNode( adv_link_node, "InputVars", 0 );
        unsigned int num_input = XmlUtil::GetNumNames( input_node, "VarDef" );
        m_InputVars.resize(num_input);
        for ( int i = 0 ; i < num_input ; i++ )
        {
            xmlNodePtr var_def_node = XmlUtil::GetNode( input_node, "VarDef", i );
            m_InputVars[i].DecodeXml( var_def_node );
        }

        //==== Output Vars ====//
        xmlNodePtr output_node = XmlUtil::GetNode( adv_link_node, "OutputVars", 0 );
        unsigned int num_output = XmlUtil::GetNumNames( output_node, "VarDef" );
        m_OutputVars.resize(num_output);
        for ( int i = 0 ; i < num_output ; i++ )
        {
            xmlNodePtr var_def_node = XmlUtil::GetNode( output_node, "VarDef", i );
            m_OutputVars[i].DecodeXml( var_def_node );
        }
    }

    return adv_link_node;
}

void AdvLink::SaveCode( const string & file_name )
{
    std::ofstream os( file_name );
    if ( os )
    {
        os << m_ScriptCode;
    }
}

void AdvLink::ReadCode( const string & file_name )
{
    std::ifstream t(file_name.c_str());
    std::stringstream buffer;
    buffer << t.rdbuf();

    string buffer_string = buffer.str();

    if ( buffer_string.size() > 0 )
    {
        m_ScriptCode = buffer_string;
        BuildScript();
    }
}

