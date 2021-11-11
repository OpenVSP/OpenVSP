//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//

// AdvLink.h: Contain and manage advanced (scripted) links between two parms
// J.R Gloudemans
//
//////////////////////////////////////////////////////////////////////

#if !defined(VSP_ADVLINK__INCLUDED_)
#define VSP_ADVLINK__INCLUDED_

#include "Defines.h"

#include <string>
#include <limits.h>

#include "Parm.h"
#include "ParmContainer.h"

using std::string;

class VarDef
{
public:

    string m_VarName;
    string m_ParmID;

    xmlNodePtr EncodeXml( xmlNodePtr & node );
    xmlNodePtr DecodeXml( xmlNodePtr & node );

};

//=====================================================================================//
//=====================================================================================//
//=====================================================================================//

//==== Advanced Link ====//
class AdvLink
{

public:
    AdvLink();
    virtual ~AdvLink();

    bool DuplicateVarName( const string & name );

    void SetName( const string & name )                             { m_Name = name; }
    string GetName()                                                { return m_Name; }

    void SetScriptCode( const string & code )                       { m_ScriptCode = code; }
    string GetScriptCode()                                          { return m_ScriptCode; }
    bool BuildScript();
    void SetValidScriptFlag( bool flag )                            { m_ValidScript = flag; }
    bool ValidScript()                                              { return m_ValidScript; }
    string GetScriptErrors()                                        { return m_ScriptErrors; }
    bool ValidateParms();

    void AddVar( const VarDef & pd, bool input_flag );
    void DeleteVar( int index, bool input_flag );
    void DeleteAllVars( bool input_flag );

    void SetVar( const string & var_name, double val );
    double GetVar( const string & var_name );

    bool UpdateLink( const string & pid );
    void ForceUpdate();

    vector< VarDef > GetInputVars()                               { return m_InputVars; }
    vector< VarDef > GetOutputVars()                              { return m_OutputVars; }

    virtual void ParmChanged( Parm*, int )                        {}

    xmlNodePtr EncodeXml( xmlNodePtr & node );
    xmlNodePtr DecodeXml( xmlNodePtr & node );

    void SaveCode( const string & file_name );
    void ReadCode( const string & file_name );


protected:

    string m_Name;
    string m_ScriptModule;
    string m_ScriptCode;
    string m_CompleteScript;
    vector< VarDef > m_InputVars;
    vector< VarDef > m_OutputVars;

    bool m_ValidScript;
    string m_ScriptErrors;
     
private:

    //==== Cannot Copy Link Because Of Parm Registration With ParmMgr ====//
    AdvLink( AdvLink const& copy );             // Not Implemented
    AdvLink& operator=( AdvLink const& copy );  // Not Implemented
};


#endif // !defined(VSP_LINK__INCLUDED_)
