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

#include <string>
#include <limits.h>

#include "Parm.h"
#include "ParmContainer.h"

using std::string;

class ParmDef
{
public:

    string m_ParmID;
    string m_GeomID;
    string m_GeomName;
    int m_GeomIndex;
    string m_ParmName;
    string m_GroupName;
    string m_VarName;
};

//==== Advanced Link ====//
class AdvLink
{

public:
    AdvLink();
    virtual ~AdvLink();

    void SetModuleName( const string & module_name )                { m_ScriptModule = module_name; }

    void AddParm( const ParmDef & pd, bool input_flag );

    void SetVar( const string & var_name, double val );
    double GetVar( const string & var_name );

    bool UpdateLink( const string & pid );


protected:

    string m_ScriptModule;
    vector< ParmDef > m_InputParms;
    vector< ParmDef > m_OutputParms;
     
private:

    //==== Cannot Copy Link Because Of Parm Registration With ParmMgr ====//
    AdvLink( AdvLink const& copy );             // Not Implemented
    AdvLink& operator=( AdvLink const& copy );  // Not Implemented
};


#endif // !defined(VSP_LINK__INCLUDED_)
