//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//

// ParmContainer.h: Parm Container classes.
// J.R Gloudemans
//
//////////////////////////////////////////////////////////////////////

#if !defined(VSP_PARMCONTRAINER__INCLUDED_)
#define VSP_PARMCONTRAINER__INCLUDED_

#include <string>
#include <map>

#include <limits.h>

#include "XmlUtil.h"

using std::string;
using std::map;

class Parm;

//==============================================================================//
//==============================================================================//
//==============================================================================//

//==== Parm Container ====//
class ParmContainer
{
public:
    ParmContainer();
    virtual ~ParmContainer();

    virtual void ParmChanged( Parm* parm_ptr, int type ) = 0;

    virtual void AddParm( const string & id );
    virtual void RemoveParm( const string & id );

    virtual string GetParentContainer()             { return m_ParentContainer; }
    virtual ParmContainer* GetParentContainerPtr();

    virtual bool GetLateUpdateFlag()                { return m_LateUpdateFlag; }
    virtual void SetLateUpdateFlag( bool flag )     { m_LateUpdateFlag = flag; }
    virtual int GetLatestChangeCnt();

    virtual string GetID()                          { return m_ID; }

    virtual void SetName( const string& name )      { m_Name = name; }
    virtual string GetName()                        { return m_Name; }

    virtual xmlNodePtr EncodeXml( xmlNodePtr & node );
    virtual xmlNodePtr DecodeXml( xmlNodePtr & node );

    virtual string FindParm( const string& group_name, int parm_ind );
    virtual string FindParm( const string& parm_name, const string& group_name );
    virtual string FindParm( int group_ind, int parm_ind );
    virtual string FindParm( const string& name );
    virtual int GetGroupNames( string parm_id, vector< string > & group_names );
    virtual int GetParmIDs( string parm_id, vector< string > & parm_ids );

    virtual void AddLinkableContainers( vector< string > & linkable_container_vec );
    virtual void AddLinkableParms( vector< string > & linkable_parm_vec, const string & link_container_id = string() );

    virtual void SetParentContainer( const string & id )    { m_ParentContainer = id; }

protected:

    string m_ID;                                        // Unique ID

    string m_Name;

    bool m_LockParmContainerID;
    bool m_LockParmID;
    bool m_LateUpdateFlag;
    bool m_LinkableFlag;

    vector< string > m_ParmVec;                         // Parms in container

    string m_ParentContainer;

    vector< string > m_GroupNames;                      // Linkable Group Names
    map< string, vector< string > > m_GroupParmMap;     // Group names mapped to vectors of parms

    //==== Methods ====//
    virtual string GenerateID();

    virtual void LoadGroupParmVec( vector< string > & parm_vec );
    virtual void LoadGroupParmVec( vector< string > & parm_vec, bool displaynames );

    virtual void ChangeID( string id );

};

//==============================================================================//
//==============================================================================//
//==============================================================================//

//==== User Parms ====//
class UserParmContainer : public ParmContainer
{
public:
    UserParmContainer();
    virtual ~UserParmContainer();

    virtual void SetNumPredefined( int n )                  { m_NumPredefined = n; }
    virtual void Renew( int num_initial_parms );
    virtual void ParmChanged( Parm* parm_ptr, int type );

    int GetNumUserParms()                                   { return m_UserParmVec.size(); }
    string GetUserParmId( int index );
    virtual string AddParm(int type, const string & name, const string & group );
    virtual void DeleteParm( int index );

    virtual xmlNodePtr EncodeXml( xmlNodePtr & node );
    virtual xmlNodePtr DecodeXml( xmlNodePtr & node );

protected:

    int m_NumPredefined;
    vector< Parm* > m_UserParmVec;

    virtual void Init( int num_initial_parms );
    virtual void Wype();

};


#endif // !defined(VSP_PARMCONTRAINER__INCLUDED_)
