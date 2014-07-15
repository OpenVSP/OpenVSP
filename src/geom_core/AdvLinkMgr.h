//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//

// AdvLinkMgr.h: interface for the Adv Link Mgr Singleton.
// J.R Gloudemans
//
//////////////////////////////////////////////////////////////////////

#if !defined(ADVLINKMGR__INCLUDED_)
#define ADVLINKMGR__INCLUDED_

#include "AdvLink.h"
#include <deque>
using std::string;
using std::vector;
using std::deque;


//==== Adv Link Manager ====//
class AdvLinkMgrSingleton
{
public:
    static AdvLinkMgrSingleton& getInstance()
    {
        static AdvLinkMgrSingleton instance;
        return instance;
    }

    void Init();
    void ReadAdvLinkScripts();
    void AddAdvLink( const string & script_module_name );

    void AddInput( const string & geom_name, int geom_index, const string & parm_name, 
                   const string & parm_group, const string & var_name );
    void AddOutput( const string & geom_name, int geom_index, const string & parm_name, 
                   const string & parm_group, const string & var_name );

    void SetVar( const string & var_name, double val );
    double GetVar( const string & var_name );

    void ParmChanged( const string& pid, bool start_flag );
    void SetCurrLink( AdvLink* adv_link )                              { m_CurrLink = adv_link; }

private:

    AdvLinkMgrSingleton();
    AdvLinkMgrSingleton( AdvLinkMgrSingleton const& copy );             // Not Implemented
    AdvLinkMgrSingleton& operator=( AdvLinkMgrSingleton const& copy );  // Not Implemented

    void AddInputOutput( const string & geom_name, int geom_index, const string & parm_name, 
                   const string & parm_group, const string & var_name, bool input_flag );

    AdvLink* m_CurrLink;
    vector< AdvLink* > m_LinkVec;

};

#define AdvLinkMgr AdvLinkMgrSingleton::getInstance()

#endif // !defined(LINKMGR__INCLUDED_)
