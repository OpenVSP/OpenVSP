//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//

// LinkMgr.h: interface for the Parm Mgr Singleton.
// J.R Gloudemans
//
//////////////////////////////////////////////////////////////////////

#if !defined(LINKMGR__INCLUDED_)
#define LINKMGR__INCLUDED_

#include "Link.h"
#include "UserParmContainer.h"
#include <deque>
using std::string;
using std::vector;
using std::deque;


//==== Parm Link Manager ====//
class LinkMgrSingleton
{
public:
    static LinkMgrSingleton& getInstance()
    {
        static LinkMgrSingleton instance;
        if( m_firsttime )
        {
            instance.Init();
        }
        return instance;
    }

    virtual void Renew();

    virtual bool AddCurrLink();                 // Add Link (A Copy of Working Link
    virtual void DelCurrLink();                 // Delete Currently Selected Link
    virtual void DelAllLinks();
    virtual bool LinkAllComp();                 // Link All Parms in Selected Parm Containers
    virtual bool LinkAllGroup();                // Link All Parms in Selected Group
    virtual void CheckLinks();                  // Check If All Links Are Still Valid
    virtual bool CheckForDuplicateLink( const string & pA, const string &  pB );
    virtual bool UsedInLink( const string & pid );

    virtual bool AddLink( const string& pA, const string& pB, bool init_link_parms = true );         // Link Two Parms
    virtual void AddLink( Link* link )                      {  m_LinkVec.push_back( link ); }
    virtual void ParmChanged( const string& pid, bool start_flag );     // A Parm Has Changed Check Links

    virtual void SetCurrLinkIndex( int i )                  { m_CurrLinkIndex = i; }
    virtual int  GetCurrLinkIndex()                         { return m_CurrLinkIndex; }
    virtual Link* GetCurrLink();

    virtual int GetNumLinks()                               { return ( int )m_LinkVec.size(); }
    virtual Link* GetLink( int index );

    virtual void SetParm( bool flagA, string parm_id );
    virtual void SetParm( bool flagA, int container_ind, int group_ind, int parm_ind );
    virtual string PickParm( int container_ind, int group_ind, int parm_ind );

    virtual void ResetWorkingLink();                            // Reset Working Link To Valid Parms

    virtual void RegisterContainer( const string & id );        // Register Base Containers
    virtual void UnRegisterContainer( const string & id );
    virtual bool CheckContainerRegistered( const string & id );
    virtual void BuildLinkableParmData();                       // Build A List of Linkable Containers And Parms

    ParmContainer* FindParmContainer( int index );              // Given Index Return Linkable Parm Container

    //==== User Parms ====//
    int GetNumUserParms()                                   { return m_UserParms.GetNumUserParms(); }
    int GetNumPredefinedUserParms()                         { return m_NumPredefinedUserParms; }
    string GetUserParmId( int index )                       { return m_UserParms.GetUserParmId( index ); }
    string AddUserParm(int type, const string & name, const string & group );
    void DeleteUserParm( int index );
    void DeleteAllUserParm( );
    UserParmContainer * GetUserParmContainer()              { return &m_UserParms; }

    //==== Build Container, Group And Parm Vecs Given Parm ID ====//
    int GetCurrContainerVec( const string& parm_id, vector< string > & idVec );
    int GetCurrGroupNameVec( const string& parm_id, vector< string > & nameVec );
    int GetCurrParmIDVec( const string& parm_id, vector< string > & idVec );

    void GetAllContainerVec( vector< string > & idVec )
    {
        idVec = m_LinkableContainers;
    }

    xmlNodePtr EncodeXml( xmlNodePtr & node );
    xmlNodePtr DecodeXml( xmlNodePtr & node );

    void SortLinksByA();
    void SortLinksByB();

    void SetFreezeUpdateFlag( bool flag )
    {
        m_FreezeUpdateFlag = flag;
    }
    bool GetFreezeUpdateFlag() const
    {
        return m_FreezeUpdateFlag;
    }

private:

    LinkMgrSingleton();
    LinkMgrSingleton( LinkMgrSingleton const& copy );          // Not Implemented
    LinkMgrSingleton& operator=( LinkMgrSingleton const& copy ); // Not Implemented

    void Init();
    void Wype();

    int m_CurrLinkIndex;
    Link *m_WorkingLink;

    static bool m_firsttime;

    bool m_FreezeUpdateFlag; // Flag to disable updating links when ParmChanged is called. This prevents geoms not being updated properly on XML import

    deque< Link* > m_LinkVec;

    vector< string > m_UpdatedParmVec;      // Keep Track Of Linked Parm To Prevent Circular Links

    vector< string > m_BaseLinkableContainers;              // Base Registered Parm Containers
    vector< string > m_LinkableContainers;                  // All valid Linkable Container

    int m_NumPredefinedUserParms;
    UserParmContainer m_UserParms;                              // User Defined Parms

};

#define LinkMgr LinkMgrSingleton::getInstance()

#endif // !defined(LINKMGR__INCLUDED_)
