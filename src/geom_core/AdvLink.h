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
    string m_CName;
    string m_GName;
    string m_PName;

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
    string MakeVarNameUnique( const string & name );

    void SetName( const string & name );
    string GetName()                                                { return m_Name; }

    void SetScriptCode( const string & code )                       { m_ScriptCode = code; }
    string GetScriptCode()                                          { return m_ScriptCode; }
    bool BuildScript();
    void SetValidScriptFlag( bool flag )                            { m_ValidScript = flag; }
    bool ValidScript()                                              { return m_ValidScript; }
    string GetScriptErrors()                                        { return m_ScriptErrors; }
    bool ValidateParms();

    void AddVar( const string & parm_id, const string & var_name, bool input_flag );
    void AddVar( const VarDef & pd, bool input_flag );
    void DeleteVar( int index, bool input_flag );
    void DeleteAllVars( bool input_flag );
    void UpdateInputVarName( int index, const string & var_name, bool changeincode );
    void UpdateOutputVarName( int index, const string & var_name, bool changeincode );

    string GetInputVarName( int index );
    string GetOutputVarName( int index );

    int ReorderInputVar( int index, int action );
    int ReorderOutputVar( int index, int action );

    void UpdateInputsCGPNames();
    void UpdateOutputsCGPNames();
    int SortInputsVar( int index );
    int SortInputsCGP( int index );
    int SortOutputsVar( int index );
    int SortOutputsCGP( int index );

    void SetVar( const string & var_name, double val );
    double GetVar( const string & var_name );

    bool UpdateLink( const string & pid );
    void ForceUpdate();

    vector< VarDef > GetInputVars()                               { return m_InputVars; }
    vector< VarDef > GetOutputVars()                              { return m_OutputVars; }

    vector< string > GetInputNames();
    vector< string > GetInputParms();
    vector< string > GetOutputNames();
    vector< string > GetOutputParms();

    virtual void ParmChanged( Parm*, int )                        {}

    xmlNodePtr EncodeXml( xmlNodePtr & node );
    xmlNodePtr DecodeXml( xmlNodePtr & node );

    void SaveCode( const string & file_name );
    void ReadCode( const string & file_name );
    void SearchReplaceCode( const string & from, const string & to );

    void AttachAttrCollection()
    {
        m_AdvLinkAttrCollection.SetCollAttach( m_Name, vsp::ATTROBJ_ADVLINK );
    }

    AttributeCollection* GetAttrCollection()
    {
        return &m_AdvLinkAttrCollection;
    }
    AttributeCollection m_AdvLinkAttrCollection;

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
    AdvLink( AdvLink const& copy ) = delete;             // Not Implemented
    AdvLink& operator=( AdvLink const& copy ) = delete;  // Not Implemented
};


#endif // !defined(VSP_LINK__INCLUDED_)
