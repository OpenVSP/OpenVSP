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
    void Wype();
    void Renew();

    AdvLink* AddLink( const string & name );
    void DelLink( AdvLink* link_ptr );
    void DelAllLinks();
    void CheckLinks();                  // Check If All Links Are Still Valid

    void AddAdvLink( const string & script_module_name );

    void AddInput( const string & parm_id, const string & var_name );
    void AddOutput( const string & parm_id, const string & var_name );

    vector< AdvLink* > GetLinks()                                       { return m_LinkVec; }

    void SetVar( const string & var_name, double val );
    double GetVar( const string & var_name );

    bool IsInputParm( const string& pid );
    void UpdateLinks( const string& pid );
    void ForceUpdate( );
    void SetActiveLink( AdvLink* adv_link )                             { m_ActiveLink = adv_link; }
    AdvLink* GetActiveLink()                                            { return m_ActiveLink; }

    AdvLink* GetLink( int index );
    void SetEditLinkIndex( int index )                                  { m_EditLinkIndex = index; }
    int GetEditLinkIndex()                                              { return m_EditLinkIndex; }

    bool DuplicateLinkName( const string & name );

    xmlNodePtr EncodeXml( xmlNodePtr & node );
    xmlNodePtr DecodeXml( xmlNodePtr & node );


private:

    AdvLinkMgrSingleton();
    AdvLinkMgrSingleton( AdvLinkMgrSingleton const& copy );             // Not Implemented
    AdvLinkMgrSingleton& operator=( AdvLinkMgrSingleton const& copy );  // Not Implemented

    void AddInputOutput( const string & parm_id, const string & var_name, bool input_flag );
    int m_EditLinkIndex;
    AdvLink* m_ActiveLink;
    vector< AdvLink* > m_LinkVec;

};

#define AdvLinkMgr AdvLinkMgrSingleton::getInstance()

#endif // !defined(LINKMGR__INCLUDED_)
