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
#include <unordered_map>

#include <climits>

#include "XmlUtil.h"
#include "ResultsMgr.h"

using std::string;
using std::unordered_map;

class Parm;

bool NameCompare( const string &parmID_A, const string &parmID_B );

//==============================================================================//
//==============================================================================//
//==============================================================================//

//==== Parm Container ====//
class ParmContainer
{
public:
    ParmContainer();
    virtual ~ParmContainer();

    // Do not allow assignment operator for ParmContainers.  Copying ParmContainers causes hard to find bugs.
    ParmContainer& operator= ( const ParmContainer& pc ) = delete;

    virtual void ParmChanged( Parm* parm_ptr, int type );

    virtual void AddParm( const string & id );
    virtual void RemoveParm( const string & id );

    virtual string GetParentContainer() const       { return m_ParentContainer; }
    virtual ParmContainer* GetParentContainerPtr() const;

    virtual void SetLateUpdateFlag( bool flag )     { m_LateUpdateFlag = flag; }
    virtual int GetLatestChangeCnt();

    virtual string GetID() const                    { return m_ID; }

    virtual void SetName( const string& name, bool removeslashes = true );
    virtual string GetName() const                  { return m_Name; }

    void SetParmContainerType( int type )
    {
        m_ParmContainer_Type = type;
    }

    int GetParmContainerType()
    {
        return m_ParmContainer_Type;
    }

    virtual xmlNodePtr EncodeXml( xmlNodePtr & node );
    virtual xmlNodePtr DecodeXml( xmlNodePtr & node );

    virtual string FindParm( const string& group_name, int parm_ind );
    virtual string FindParm( const string& parm_name, const string& group_name );
    virtual string FindParm( int group_ind, int parm_ind );
    virtual string FindParm( const string& name );
    virtual void GetGroupNames( vector< string > & group_names );
    virtual int GetGroupNames( const string &parm_id, vector< string > & group_names );
    virtual int GetParmIDs( const string &parm_id, vector< string > & parm_ids );

    virtual void AddLinkableContainers( vector< string > & linkable_container_vec );
    virtual void AddLinkableParms( vector< string > & linkable_parm_vec, const string & link_container_id = string() );

    virtual void SetParentContainer( const string & id )    { m_ParentContainer = id; }

    virtual void CopyVals( ParmContainer *from );

    virtual void SwapIDs( ParmContainer* from );

    AttributeCollection* GetAttrCollection()
    {
        return &m_AttrCollection;
    }

    void AttachAttrCollection()
    {
        m_AttrCollection.SetCollAttach( m_ID, m_ParmContainer_Type );
    }

    AttributeCollection m_AttrCollection;

protected:

    string m_ID;                                        // Unique ID

    string m_Name;

    bool m_LateUpdateFlag;

    vector< string > m_ParmVec;                         // Parms in container

    string m_ParentContainer;

    vector< string > m_GroupNames;                      // Linkable Group Names
    unordered_map< string, vector< string > > m_GroupParmMap;     // Group names mapped to vectors of parms

    int m_ParmContainer_Type;                           // Enumerated type for setting collection attachments (geom, veh, link, etc.)

    //==== Methods ====//
    virtual string GenerateID();

    virtual void LoadGroupParmVec( vector< string > & parm_vec );
    virtual void LoadGroupParmVec( vector< string > & parm_vec, bool displaynames );

    virtual void ChangeID( const string &id );

};


#endif // !defined(VSP_PARMCONTRAINER__INCLUDED_)
