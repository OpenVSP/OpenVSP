//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//

// ParmContainer.cpp: implementation of the Parm Container classes.
//
//////////////////////////////////////////////////////////////////////

#include "Parm.h"
#include "ParmMgr.h"
#include "IDMgr.h"
#include "LinkMgr.h"
#include "StringUtil.h"
#include "StlHelper.h"
#include "Vehicle.h"
#include "VspUtil.h"

using std::string;

bool NameCompare( const string &parmID_A, const string &parmID_B )
{
    Parm* pA = ParmMgr.FindParm( parmID_A );
    Parm* pB = ParmMgr.FindParm( parmID_B );

    if ( pA && pB )
    {
        string c_name_A, g_name_A, p_name_A, c_name_B, g_name_B, p_name_B;
        ParmMgr.GetNames( parmID_A, c_name_A, g_name_A, p_name_A );
        ParmMgr.GetNames( parmID_B, c_name_B, g_name_B, p_name_B );

        // Check container names first
        int c = StringUtil::natural_compare( c_name_A, c_name_B );
        if ( c != 0 )
        {
            return c < 0;
        }

        string cAID = pA->GetContainerID();
        string cBID = pB->GetContainerID();

        // Matching container names, sort by container ID
        if ( cAID.compare( cBID ) != 0 )
        {
            return cAID < cBID;
        }

        // Matching ID, sort by group
        c = StringUtil::natural_compare( g_name_A, g_name_B );
        if ( c != 0 )
        {
            return c < 0;
        }

        // Matching group, sort by parameter
        c = StringUtil::natural_compare( p_name_A, p_name_B );
        if ( c != 0 )
        {
            return c < 0;
        }

    }
    return ( false );
}

//=========================================================================//
//=======================        Parm Container       =====================//
//=========================================================================//

//==== Constructor ====//
ParmContainer::ParmContainer()
{
    m_ID = GenerateID();
    m_ReadsIntoSelf = false;
    m_Name = string( "Default" );
    m_LateUpdateFlag = true; // Force update first time through.
    m_ParmContainer_Type = vsp::ATTROBJ_FREE;
    m_ListedFlag = true;
    AttachAttrCollection();
    ParmMgr.AddParmContainer( this );
}

//==== Destructor ====//
ParmContainer::~ParmContainer()
{
    // What still names this container as its parent is left with none.  Copied, as each one moves.
    vector< ParmContainer* > children = ParmMgr.GetChildContainers( m_ID );
    for ( int i = 0 ; i < ( int )children.size() ; i++ )
    {
        children[i]->SetParentContainer( string() );
    }

    ParmMgr.ParentContainerChanged( this, m_ParentContainer, string() );
    ParmMgr.RemoveParmContainer( this );
}

void ParmContainer::SetParentContainer( const string & id )
{
    // A container cannot be below itself: a parent that is this container, or sits below it, is refused.
    for ( ParmContainer* p = ParmMgr.FindParmContainer( id ); p; p = p->GetParentContainerPtr() )
    {
        if ( p == this )
        {
            return;
        }
    }

    ParmMgr.ParentContainerChanged( this, m_ParentContainer, id );
    m_ParentContainer = id;
}

void ParmContainer::ParmChanged( Parm* parm_ptr, int type )
{
    Vehicle* veh = VehicleMgr.GetVehicle();

    if ( veh )
    {
        veh->ParmChanged( parm_ptr, type );
    }
}

//==== Add Parm To Storage Vector ====//
void ParmContainer::AddParm( const string& id )
{
    m_ParmVec.push_back( id );
}

//==== Remove Parm From Storage Vector ====//
void ParmContainer::RemoveParm( const string& id )
{
    vector_remove_val( m_ParmVec, id );
}

//==== Put Every Parm Back The Way A New One Starts ====//
void ParmContainer::ResetToInitVals()
{
    for ( int i = 0; i < ( int )m_ParmVec.size(); i++ )
    {
        Parm* p = ParmMgr.FindParm( m_ParmVec[i] );

        if ( p )
        {
            p->ResetToInitVal();
        }
    }
}

//==== Return Pointer To Parent Container ====//
ParmContainer* ParmContainer::GetParentContainerPtr() const
{
    return ParmMgr.FindParmContainer( m_ParentContainer );
}

//==== Climb The Parents To The Top ====//
ParmContainer* ParmContainer::GetTopContainer()
{
    ParmContainer* top = this;
    for ( ParmContainer* p = GetParentContainerPtr(); p && top->GetListedFlag(); p = p->GetParentContainerPtr() )
    {
        top = p;
    }
    return top;
}

//==== The Parms Of This Container And Of Those Below It ====//
void ParmContainer::GetParmIDsUnder( vector< string > & parm_ids )
{
    for ( int i = 0 ; i < ( int )m_ParmVec.size() ; i++ )
    {
        if ( ParmMgr.FindParm( m_ParmVec[i] ) )
        {
            parm_ids.push_back( m_ParmVec[i] );
        }
    }

    const vector< ParmContainer* > & children = ParmMgr.GetChildContainers( m_ID );
    for ( int i = 0 ; i < ( int )children.size() ; i++ )
    {
        if ( children[i]->GetListedFlag() )
        {
            children[i]->GetParmIDsUnder( parm_ids );
        }
    }
}

void ParmContainer::SetListedFlag( bool flag )
{
    if ( flag != m_ListedFlag )
    {
        m_ListedFlag = flag;
        ParmMgr.IncNumParmChanges();
    }
}

//==== Number The Groups Of This Container's Parms ====//
void ParmContainer::SetGroupDisplaySuffix( int num )
{
    for ( int i = 0 ; i < ( int )m_ParmVec.size() ; i++ )
    {
        Parm* p = ParmMgr.FindParm( m_ParmVec[i] );
        if ( p )
        {
            p->SetGroupDisplaySuffix( num );
        }
    }
}

void ParmContainer::SetGroupDisplayPrefix( const string & prefix )
{
    for ( int i = 0 ; i < ( int )m_ParmVec.size() ; i++ )
    {
        Parm* p = ParmMgr.FindParm( m_ParmVec[i] );
        if ( p )
        {
            p->SetGroupDisplayPrefix( prefix );
        }
    }
}

//==== Create A Unique ID  =====//
string ParmContainer::GenerateID()
{
    return GenerateRandomID( vsp::ID_LENGTH_PARMCONTAINER );
}

void ParmContainer::ChangeID( const string &id )
{
    // Whatever names this container as its parent goes on naming it.  Copied, as each one moves.
    vector< ParmContainer* > children = ParmMgr.GetChildContainers( m_ID );

    ParmMgr.RemoveParmContainer( this );

    if( LinkMgr.CheckContainerRegistered( m_ID ) )
    {
        LinkMgr.UnRegisterContainer( m_ID );
        LinkMgr.RegisterContainer( id );
    }

    m_ID = id;
    AttachAttrCollection();

    for ( int i = 0 ; i < ( int )children.size() ; i++ )
    {
        children[i]->SetParentContainer( m_ID );
    }

    ParmMgr.AddParmContainer( this );
}

//==== Get Latest Change Cnt ====//
int ParmContainer::GetLatestChangeCnt()
{
    int cnt = 0;
    for ( int i = 0 ; i < ( int )m_ParmVec.size() ; i++ )
    {
        string pid = m_ParmVec[i];
        Parm* p = ParmMgr.FindParm( pid );
        if ( p && p->GetChangeCnt() > cnt )
        {
            cnt = p->GetChangeCnt();
        }
    }
    return cnt;
}

void ParmContainer::SetName( const string& name, bool removeslashes )
{
    string temp_name = name;
    // Remove forward slashes, which FLTK interprets as new choice dropdowns
    if ( removeslashes )
    {
        StringUtil::remove_all( temp_name, '/' );
    }
    m_Name = temp_name;
}

//==== A Container's Own Parms By Their Group Names, As They Are Written To A File ====//
// Each group's ( Parm ID, Parm name ) pairs, sorted by name; the groups in name order.
typedef std::map< string, vector< std::pair< string, string > > > ParmGroups;

static void GroupParms( const vector< string > & parm_ids, ParmGroups & groups )
{
    for ( int i = 0 ; i < ( int )parm_ids.size() ; i++ )
    {
        Parm* p = ParmMgr.FindParm( parm_ids[i] );
        if ( p )
        {
            groups[ p->GetGroupName() ].push_back( std::pair< string, string >( parm_ids[i], p->GetName() ) );
        }
    }

    ParmGroups::iterator iter;
    for ( iter = groups.begin() ; iter != groups.end() ; ++iter )
    {
        sort( iter->second.begin(), iter->second.end(), ParmNameCompare );
    }
}

//==== Encode Data Into XML Data Struct ====//
xmlNodePtr ParmContainer::EncodeXml( xmlNodePtr & node )
{
    xmlNodePtr parmcontain_node = xmlNewChild( node, nullptr, BAD_CAST "ParmContainer", nullptr );
    if ( parmcontain_node )
    {
        XmlUtil::AddStringNode( parmcontain_node, "ID", m_ID );
        XmlUtil::AddStringNode( parmcontain_node, "Name", m_Name );
        m_AttrCollection.EncodeXml( parmcontain_node );
    }

    xmlNodePtr gnode;

    ParmGroups groups;
    GroupParms( m_ParmVec, groups );

    ParmGroups::iterator groupIter;
    for ( groupIter = groups.begin() ; groupIter != groups.end() ; ++groupIter )
    {
        gnode = xmlNewChild( parmcontain_node, nullptr, BAD_CAST groupIter->first.c_str(), nullptr );

        if ( gnode )
        {
            vector< std::pair< string, string > >::iterator parmIter;
            for ( parmIter = groupIter->second.begin(); parmIter != groupIter->second.end(); ++parmIter )
            {
                string parmID = parmIter->first;
                Parm* p = ParmMgr.FindParm( parmID );
                if ( p )
                {
                    p->EncodeXml(gnode);
                }
            }
        }
    }

    return parmcontain_node;
}

//==== Decode Data From XML Data Struct ====//
xmlNodePtr ParmContainer::DecodeXml( xmlNodePtr & node )
{
    xmlNodePtr child_node = XmlUtil::GetNode( node, "ParmContainer", 0 );
    if ( child_node )
    {
        string oldID = XmlUtil::FindString( child_node, "ID", m_ID );
        string newID = IDMgr.RemapID( oldID, m_ID );

        if( newID.compare( m_ID ) != 0 )        // they differ
        {
            ChangeID( newID );
        }

        SetName( XmlUtil::FindString(  child_node, "Name", m_Name ) );
    }

    ParmGroups groups;
    GroupParms( m_ParmVec, groups );

    unordered_map< string, xmlNodePtr > group_nodes = XmlUtil::GetNodeMap( child_node );

    ParmGroups::iterator groupIter;
    for ( groupIter = groups.begin() ; groupIter != groups.end() ; ++groupIter )
    {
        xmlNodePtr gnode = XmlUtil::FindNode( group_nodes, groupIter->first );

        if ( gnode )
        {
            unordered_map< string, xmlNodePtr > parm_nodes = XmlUtil::GetNodeMap( gnode );

            vector< std::pair< string, string > >::iterator parmIter;
            for ( parmIter = groupIter->second.begin(); parmIter != groupIter->second.end(); ++parmIter )
            {
                string parmID = parmIter->first;
                Parm* p = ParmMgr.FindParm( parmID );
                if ( p )
                {
                    p->DecodeXmlNodeMap( parm_nodes );
                }
            }
        }
    }
    m_AttrCollection.DecodeXml( child_node );

    return child_node;

}

//==== Name Compare ====//
bool ParmNameCompare( const std::pair< string, string >& a, const std::pair< string, string >& b )
{
    return StringUtil::natural_compare( a.second, b.second ) < 0;
}

//==== Find Parm ID Given GroupName and Parm Name ====//
string ParmContainer::FindParm( const string& parm_name, const string& group_name  )
{
    string id;

    //==== Look Thru All Parms And Return First Name Match ====//
    for ( int i = 0 ; i < ( int )m_ParmVec.size() ; i++ )
    {
        Parm* p = ParmMgr.FindParm( m_ParmVec[i] );
        if ( p )
        {
            if ( p->GetName() == parm_name && p->GetGroupName() == group_name )
            {
                return p->GetID();
            }
        }
    }
    return id;
}

//==== Find Parm ID Given Parm Name ====//
string ParmContainer::FindParm( const string& parm_name  )
{
    //==== Look Thru All Parms And Return First Name Match ====//
    for ( int i = 0 ; i < ( int )m_ParmVec.size() ; i++ )
    {
        Parm* p = ParmMgr.FindParm( m_ParmVec[i] );
        if ( p )
        {
            if ( p->GetName() == parm_name )
            {
                return p->GetID();
            }
        }
    }

    return string();
}

void ParmContainer::AddTopContainerIDs( vector< string > & container_ids )
{
    container_ids.push_back( m_ID );
}

// This copies Parm values from one ParmContainer to another.  It does not recurse into 'child' containers.  It does
// not create missing parms or groups.  All parm names must be unique -- and exact matches.
void ParmContainer::CopyVals( ParmContainer *from )
{
    for ( int i = 0 ; i < ( int )m_ParmVec.size() ; i++ )
    {
        Parm *p = ParmMgr.FindParm( m_ParmVec[i] );
        if ( p )
        {
            string n = p->GetName();

            string fid = from->FindParm( n );

            Parm *fp = ParmMgr.FindParm( fid );
            if ( fp )
            {
                p->Set( fp->Get() );
            }
        }
    }
}

void ParmContainer::CopyMatchingVals( ParmContainer *from )
{
    if ( !from || from == this )
    {
        return;
    }

    for ( int i = 0 ; i < ( int )m_ParmVec.size() ; i++ )
    {
        Parm *p = ParmMgr.FindParm( m_ParmVec[i] );
        if ( p )
        {
            Parm *fp = ParmMgr.FindParm( from->FindParm( p->GetName(), p->GetGroupName() ) );
            if ( fp )
            {
                p->Set( fp->Get() );
            }
        }
    }
}

// The IDs cannot simply be assigned across: for the moment between the two writes both objects
// would answer to the same ID, and whichever registry is asked first would hand back the wrong
// one.  A spare name in the middle keeps every ID unique at every step.
void ParmContainer::SwapIdentity( ParmContainer* other )
{
    if ( !other || other == this )
    {
        return;
    }

    SwapIDs( other );

    string mine = m_ID;
    string theirs = other->m_ID;

    other->ChangeID( "SWAPIDENTITY_TEMP_ID" );
    ChangeID( theirs );
    other->ChangeID( mine );
}

void ParmContainer::SwapContainerID( ParmContainer* other )
{
    if ( !other || other == this )
    {
        return;
    }

    string mine = m_ID;
    string theirs = other->m_ID;

    other->ChangeID( "SWAPIDENTITY_TEMP_ID" );
    ChangeID( theirs );
    other->ChangeID( mine );
}

void ParmContainer::HandAttributesTo( ParmContainer* to )
{
    if ( !to || to == this )
    {
        return;
    }

    m_AttrCollection.HandAttrsTo( to->GetAttrCollection() );
    to->GetAttrCollection()->SwapID( &m_AttrCollection );
}

void ParmContainer::HandUnpairedAttributesTo( ParmContainer* to )
{
    if ( !to || to == this )
    {
        return;
    }

    for ( int i = 0 ; i < ( int )m_ParmVec.size() ; i++ )
    {
        Parm* p = ParmMgr.FindParm( m_ParmVec[i] );
        if ( p && to->FindParm( p->GetName(), p->GetGroupName() ).empty() )
        {
            p->GetAttrCollection()->HandAttrsTo( to->GetAttrCollection() );
        }
    }
}

void ParmContainer::HandPairedAttributesTo( ParmContainer* to )
{
    if ( !to || to == this )
    {
        return;
    }

    for ( int i = 0 ; i < ( int )m_ParmVec.size() ; i++ )
    {
        Parm* p = ParmMgr.FindParm( m_ParmVec[i] );
        if ( p )
        {
            Parm* q = ParmMgr.FindParm( to->FindParm( p->GetName(), p->GetGroupName() ) );
            if ( q )
            {
                p->GetAttrCollection()->HandAttrsTo( q->GetAttrCollection() );
            }
        }
    }
}

void ParmContainer::DeleteAttributes()
{
    m_AttrCollection.DelAllAttrs();

    for ( int i = 0 ; i < ( int )m_ParmVec.size() ; i++ )
    {
        Parm* p = ParmMgr.FindParm( m_ParmVec[i] );
        if ( p )
        {
            p->GetAttrCollection()->DelAllAttrs();
        }
    }
}

void ParmContainer::SwapIDs( ParmContainer *from )
{
    // Make a copy to iterate over because SwapID's implicitly modifies m_ParmVec.
    vector< string > parmvec = m_ParmVec;

    vector< std::pair< string, string > > swap_matches;

    for ( int i = 0 ; i < ( int )parmvec.size() ; i++ )
    {
        Parm* pa = ParmMgr.FindParm( parmvec[i] );
        if ( pa )
        {
            Parm* pb = ParmMgr.FindParm( from->FindParm( pa->GetName(), pa->GetGroupName() ) );
            if ( pb )
            {
                swap_matches.push_back( std::pair < string, string > ( pa->GetID(), pb->GetID() ) );
            }
        }
    }

    for ( int i = 0 ; i < ( int )swap_matches.size() ; i++ )
    {
        ParmMgr.SwapIDs( swap_matches[i].first, swap_matches[i].second );
    }
}
