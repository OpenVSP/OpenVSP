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

    // Put every Parm in the container back the way a new one starts.  A container with
    // state that is not a Parm has more to do and should extend this.
    virtual void ResetToInitVals();

    virtual string GetParentContainer() const       { return m_ParentContainer; }
    virtual ParmContainer* GetParentContainerPtr() const;

    // Whether this container is listed under the containers above it.  One that is not -- a Custom Geom's cross
    // section surface, which its script builds again on each update and which is never saved, so its Parms do
    // not last to be linked to -- is the top of its own Parms.
    void SetListedFlag( bool flag );
    bool GetListedFlag() const                      { return m_ListedFlag; }

    virtual void SetLateUpdateFlag( bool flag )     { m_LateUpdateFlag = flag; }
    virtual int GetLatestChangeCnt();

    virtual string GetID() const                    { return m_ID; }

    // Whether reading a file or a copy decodes into this very object, or into one that replaces
    // it, instead of making a new one, so an ID the read brings that this object already holds
    // stays with it.  Set by the objects that outlive a model -- the Vehicle, its settings, the
    // manager singletons -- and by VSPAERO's Cp slices.
    void SetReadsIntoSelf( bool flag )              { m_ReadsIntoSelf = flag; }
    bool GetReadsIntoSelf() const                   { return m_ReadsIntoSelf; }

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

    // Rebuild the map that finds a Parm by group and name from the Parms held now.
    virtual void UpdateGroupParmMap();

    virtual void GetGroupNames( vector< string > & group_names );
    virtual int GetGroupNames( const string &parm_id, vector< string > & group_names );
    virtual int GetParmIDs( const string &parm_id, vector< string > & parm_ids );

    virtual void AddLinkableContainers( vector< string > & linkable_container_vec );
    virtual void AddLinkableParms( vector< string > & linkable_parm_vec, const string & link_container_id = string() );

    // Name the container this one belongs to; ParmMgr keeps the inverse.
    // One that would put this container below itself is refused, so a chain of parents always ends.
    virtual void SetParentContainer( const string & id );

    virtual void CopyVals( ParmContainer *from );

    // Like CopyVals, but pairs the Parms by group as well as by name -- the pairing SwapIDs
    // uses -- so a name that appears in two groups is not confused.
    virtual void CopyMatchingVals( ParmContainer *from );

    virtual void SwapIDs( ParmContainer* from );

    // Trade identities with another container: the Parms the two have in common by group and
    // name, and then the container IDs themselves.  Everything holding either identity goes on
    // naming the same thing afterwards -- a link, a design variable, an analysis that named a
    // Geom, a control surface group that named a subsurface.  For when one object takes
    // another's place rather than being created beside it.
    virtual void SwapIdentity( ParmContainer* other );

    // Trade container IDs only, leaving the Parms' IDs where they are.  For a container nested in
    // one whose SwapIDs has already traded its Parms.
    virtual void SwapContainerID( ParmContainer* other );

    // Moves this container's attributes to another, and the receiving collection takes the
    // giving collection's ID as well.  A Parm's attributes belong with its ID, not here.
    virtual void HandAttributesTo( ParmContainer* to );

    // Moves the attributes of each Parm with no counterpart of the same group and name in
    // another container onto that container, where they are kept rather than lost.
    virtual void HandUnpairedAttributesTo( ParmContainer* to );

    // Moves the attributes of each Parm to the Parm of the same group and name in another
    // container, beside whatever that one holds.
    virtual void HandPairedAttributesTo( ParmContainer* to );

    // Deletes the attributes on this container and on each of its Parms.
    virtual void DeleteAttributes();

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
    bool m_ReadsIntoSelf;

    string m_Name;

    bool m_LateUpdateFlag;

    vector< string > m_ParmVec;                         // Parms in container

    string m_ParentContainer;

    vector< string > m_GroupNames;                      // Linkable Group Names
    unordered_map< string, vector< std::pair< string, string > > > m_GroupParmMap;     // Group names mapped to vectors of parms

    int m_ParmContainer_Type;                           // Enumerated type for setting collection attachments (geom, veh, link, etc.)

    bool m_ListedFlag;

    //==== Methods ====//
    virtual string GenerateID();

    virtual void LoadGroupParmVec( vector< string > & parm_vec );
    virtual void LoadGroupParmVec( vector< string > & parm_vec, bool displaynames );

    // Take a new ID.  This moves the object in the registries that find it by ID, and tells
    // whatever this object contains that its container has a new name.  It does not tell
    // anything holding this ID as a reference -- a link, an attribute, a preset, a Geom's
    // parent, child or step child list.
    //
    // Those hold together because an ID only changes before anything in the model refers to the
    // object: a file read or a paste settles the ID first and then reads the references,
    // translating each through the remap map, and IDs assigned at construction, such as user
    // parms and the built-in FEA materials, are set before anything can name them.
    //
    // ParmMgr::SwapIDs is the deliberate exception, and compensates by swapping: two live Parms
    // trade IDs so that everything holding either one keeps naming the same cross section.
    virtual void ChangeID( const string &id );

};


#endif // !defined(VSP_PARMCONTRAINER__INCLUDED_)
