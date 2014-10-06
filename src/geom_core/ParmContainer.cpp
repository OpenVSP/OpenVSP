//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//

// ParmContainer.cpp: implementation of the Parm Container classes.
//
//////////////////////////////////////////////////////////////////////

#include "Parm.h"
#include "ParmMgr.h"
#include "LinkMgr.h"
#include "Vehicle.h"
#include "VehicleMgr.h"
#include <float.h>
#include <time.h>
#include <algorithm>

#include "XmlUtil.h"
#include "StringUtil.h"
#include "StlHelper.h"

using std::string;


//=========================================================================//
//=======================        Parm Container       =====================//
//=========================================================================//

//==== Constructor ====//
ParmContainer::ParmContainer()
{
    m_ID = GenerateID();
    m_Name = string( "Default" );
    m_LateUpdateFlag = true; // Force update first time through.
    m_LinkableFlag = false;
    ParmMgr.AddParmContainer( this );
}

//==== Destructor ====//
ParmContainer::~ParmContainer()
{
    ParmMgr.RemoveParmContainer( this );
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

//==== Return Pointer To Parent Container ====//
ParmContainer* ParmContainer::GetParentContainerPtr()
{
    return ParmMgr.FindParmContainer( m_ParentContainer );
}

//==== Create A Unique ID  =====//
string ParmContainer::GenerateID()
{
    return ParmMgr.GenerateID( 10 );
}

void ParmContainer::ChangeID( string id )
{
    ParmMgr.RemoveParmContainer( this );

    if( LinkMgr.CheckContainerRegistered( m_ID ) )
    {
        LinkMgr.UnRegisterContainer( m_ID );
        LinkMgr.RegisterContainer( id );
    }

    m_ID = id;

    for ( int i = 0 ; i < ( int )m_ParmVec.size() ; i++ )
    {
        string pid = m_ParmVec[i];
        Parm* p = ParmMgr.FindParm( pid );
        if ( p )
        {
            p->ReSetLinkContainerID();
        }
    }

    ParmMgr.AddParmContainer( this );
}

//==== Encode Data Into XML Data Struct ====//
xmlNodePtr ParmContainer::EncodeXml( xmlNodePtr & node )
{
    xmlNodePtr parmcontain_node = xmlNewChild( node, NULL, BAD_CAST "ParmContainer", NULL );
    if ( parmcontain_node )
    {
        XmlUtil::AddStringNode( parmcontain_node, "ID", m_ID );
        XmlUtil::AddStringNode( parmcontain_node, "Name", m_Name );
    }

    xmlNodePtr gnode;

    LoadGroupParmVec( m_ParmVec, false );

    map< string, vector< string > >::iterator groupIter;
    for ( groupIter = m_GroupParmMap.begin() ; groupIter != m_GroupParmMap.end() ; groupIter++ )
    {
        string name = groupIter->first;
        gnode = xmlNewChild( parmcontain_node, NULL, BAD_CAST name.c_str(), NULL );

        if ( gnode )
        {
            vector< string >::iterator parmIter;
            for ( parmIter = groupIter->second.begin(); parmIter != groupIter->second.end(); parmIter++ )
            {
                Parm* p = ParmMgr.FindParm( ( *parmIter ) );
                p->EncodeXml( gnode );
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
        string newID = ParmMgr.RemapID( oldID, m_ID );

        if( newID.compare( m_ID ) != 0 )        // they differ
        {
            ChangeID( newID );
        }

        SetName( XmlUtil::FindString(  child_node, "Name", m_Name ) );
    }

    xmlNodePtr gnode;

    LoadGroupParmVec( m_ParmVec, false );

    map< string, vector< string > >::iterator groupIter;
    for ( groupIter = m_GroupParmMap.begin() ; groupIter != m_GroupParmMap.end() ; groupIter++ )
    {
        string name = groupIter->first;
        gnode = XmlUtil::GetNode( child_node, name.c_str(), 0 );

        if ( gnode )
        {
            vector< string >::iterator parmIter;
            for ( parmIter = groupIter->second.begin(); parmIter != groupIter->second.end(); parmIter++ )
            {
                Parm* p = ParmMgr.FindParm( ( *parmIter ) );
                p->DecodeXml( gnode );
            }
        }
    }

    return child_node;

}

//==== Name Compare ====//
bool ParmNameCompare( const string a, const string b )
{
    Parm* pA = ParmMgr.FindParm( a );
    Parm* pB = ParmMgr.FindParm( b );

    if ( pA && pB )
    {
        return ( pA->GetName() < pB->GetName() );
    }
    return ( false );
}

//==== Load Parm Vec, Find Groups And Sort ====//
void ParmContainer::LoadGroupParmVec( vector< string > & parm_vec, bool displaynames )
{
    m_GroupNames.clear();
    m_GroupParmMap.clear();

    //==== Map Parms To Group Names ====//
    for ( int i = 0 ; i < ( int )parm_vec.size() ; i++ )
    {
        string pid = parm_vec[i];
        Parm* p = ParmMgr.FindParm( pid );
        if ( p )
        {
            string group_name;
            if( displaynames )
            {
                group_name = p->GetDisplayGroupName();
            }
            else
            {
                group_name = p->GetGroupName();
            }

            m_GroupParmMap[group_name].push_back( pid );
        }
    }

    map< string, vector< string > >::iterator iter;
    for ( iter = m_GroupParmMap.begin() ; iter != m_GroupParmMap.end() ; iter++ )
    {
        m_GroupNames.push_back( iter->first );
        sort( iter->second.begin(), iter->second.end(), ParmNameCompare );
    }
}

void ParmContainer::LoadGroupParmVec( vector< string > & parm_vec )
{
    LoadGroupParmVec( parm_vec, true );
}

//==== Find Parm ID Given GroupName and Parm Index ====//
string ParmContainer::FindParm( const string& group_name, int parm_ind )
{
    string id;

    map< string, vector< string > >::iterator iter = m_GroupParmMap.find( group_name );
    if ( iter == m_GroupParmMap.end() )
    {
        return id;
    }

    int num_parms = ( int )iter->second.size();
    if ( num_parms == 0 )
    {
        return id;
    }

    if ( parm_ind >= 0 && parm_ind < num_parms )
    {
        return iter->second[parm_ind];
    }

    return iter->second[0];
}

//==== Find Parm ID Given GroupName and Parm Index ====//
string ParmContainer::FindParm( int group_ind, int parm_ind  )
{
    string id;

    if ( ( int )m_GroupNames.size() == 0 )
    {
        return id;
    }

    if ( group_ind >= 0 && group_ind < ( int )m_GroupNames.size() )
    {
        return FindParm( m_GroupNames[group_ind], parm_ind );
    }

    return FindParm( m_GroupNames[0], parm_ind );
}

//==== Find Parm ID Given GroupName and Parm Name ====//
string ParmContainer::FindParm( const string& parm_name, const string& group_name  )
{
    string id;

    map< string, vector< string > >::iterator iter = m_GroupParmMap.find( group_name );
    if ( iter == m_GroupParmMap.end() )
    {
        return id;
    }

    int num_parms = ( int )iter->second.size();
    if ( num_parms == 0 )
    {
        return id;
    }

    for ( int i = 0 ; i < num_parms ; i++ )
    {
        string pid = iter->second[i];
        Parm* p = ParmMgr.FindParm( pid );
        if ( p && p->GetName() == parm_name )
        {
            return pid;
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

//==== Get Vector of Group Names And Index For Given Parm ID  ====//
int ParmContainer::GetGroupNames( string parm_id, vector< string > & group_names )
{
    int index = 0;

    string group_name;
    Parm* p = ParmMgr.FindParm( parm_id );
    if ( p )
    {
        group_name = p->GetDisplayGroupName();
    }

    map< string, vector< string > >::iterator iter;
    for ( iter = m_GroupParmMap.begin() ; iter != m_GroupParmMap.end() ; iter++ )
    {
        string name = iter->first;
        if ( name == group_name )
        {
            index = group_names.size();
        }
        group_names.push_back( name );
    }
    return index;
}

//==== Get Vector Parm IDs And Index For Given Parm ID  ====//
int ParmContainer::GetParmIDs( string parm_id, vector< string > & parm_ids )
{
    int index = 0;

    string group_name;
    Parm* p = ParmMgr.FindParm( parm_id );
    if ( p )
    {
        group_name = p->GetDisplayGroupName();
    }

    map< string, vector< string > >::iterator iter = m_GroupParmMap.find( group_name );

    if ( iter == m_GroupParmMap.end() )
    {
        return index;
    }

    for ( int i = 0 ; i < ( int )iter->second.size() ; i++ )
    {
        if ( parm_id == iter->second[i] )
        {
            index = parm_ids.size();
        }

        parm_ids.push_back( iter->second[i] );
    }

    return index;

}

void ParmContainer::AddLinkableContainers( vector< string > & linkable_container_vec )
{
    //==== Find Linkable Parms For This Container ====//
    vector< string > linkable_parm_vec;
    AddLinkableParms( linkable_parm_vec );

    //==== Sort Parms By Name And Find Groups ====//
    LoadGroupParmVec( linkable_parm_vec );

    //==== Add This Container To Vector Of Linkable Containers ====//
    linkable_container_vec.push_back( m_ID );
}

//==== Look Though All Parms and Load Linkable Ones ===//
void ParmContainer::AddLinkableParms( vector< string > & linkable_parm_vec, const string & link_container_id )
{
    for ( int i = 0 ; i < ( int )m_ParmVec.size() ; i++ )
    {
        Parm* p = ParmMgr.FindParm( m_ParmVec[i] );

        if ( p && p->IsLinkable() )
        {
            if ( link_container_id.size() )
            {
                p->SetLinkContainerID( link_container_id );
            }
            linkable_parm_vec.push_back( p->GetID() );
        }
    }
}


//=========================================================================//
//==================        User Parm Container       =====================//
//=========================================================================//

//==== Constructor ====//
UserParmContainer::UserParmContainer() : ParmContainer()
{
    m_Name = "UserParms";
    Init(0);
}

//==== Destructor ====//
UserParmContainer::~UserParmContainer()
{
}

void UserParmContainer::Init(int num_initial_parms)
{

    m_UserParmVec.resize( num_initial_parms );
    for( int i = 0 ; i < (int)m_UserParmVec.size() ; i++ )
    {
        string name( "User_" );
        string num = StringUtil::int_to_string( i, "%d" );
        name.append( num );
        m_UserParmVec[i] = new Parm();
        m_UserParmVec[i]->Init( name, "User_Group", this, 0.0, -1.0e12, 1.0e12 );
        m_UserParmVec[i]->ChangeID( name );
    }
}

void UserParmContainer::Wype()
{
    for( int i = 0 ; i < (int)m_UserParmVec.size() ; i++ )
    {
        delete m_UserParmVec[i];
    }
    m_UserParmVec.clear();
}

void UserParmContainer::Renew(int num_initial_parms)
{
    Wype();
    Init(num_initial_parms);
}

//==== Parm Changed ====//
void UserParmContainer::ParmChanged( Parm* parm_ptr, int type )
{
    Vehicle* veh = VehicleMgr.GetVehicle();
    if ( veh )
    {
        veh->ParmChanged( parm_ptr, Parm::SET );
    }
}

//==== Return User Parm ====//
string UserParmContainer::GetUserParmId( int index )
{
    string id;

    if ( index >= 0 && index < ( int )m_UserParmVec.size() )
    {
        return m_UserParmVec[index]->GetID();
    }

    return id;

}

//==== Encode Data Into XML Data Struct ====//
xmlNodePtr UserParmContainer::EncodeXml( xmlNodePtr & node )
{
    xmlNodePtr userparmcontain_node = xmlNewChild( node, NULL, BAD_CAST "UserParmContainer", NULL );
    if ( userparmcontain_node )
    {
       XmlUtil::AddIntNode( userparmcontain_node, "NumUserParms", (int)m_UserParmVec.size() );
       for ( int i = 0; i < static_cast<int>( m_UserParmVec.size() ); i++ )
        {
            Parm* p = m_UserParmVec[i];
            if ( p )
            {
                p->EncodeXml( userparmcontain_node, true );
            }
        }
    }

    return userparmcontain_node;
}

//==== Decode Data From XML Data Struct ====//
xmlNodePtr UserParmContainer::DecodeXml( xmlNodePtr & node )
{
    xmlNodePtr child_node = XmlUtil::GetNode( node, "UserParmContainer", 0 );
    if ( child_node )
    {
        int num_user = XmlUtil::FindInt( child_node, "NumUserParms", 0 );
        Renew( num_user );
        for ( int i = 0; i < static_cast<int>( m_UserParmVec.size() ); i++ )
        {
            xmlNodePtr pnode = XmlUtil::GetNode( child_node, "UserParm", i );
            Parm* p = m_UserParmVec[i];
            if ( pnode && p )
            {
                p->DecodeXml( pnode, true );
            }
        }
    }
    return child_node;
}

//==== Add User Defined Parm ====//
string UserParmContainer::AddParm(int type, const string & name, const string & group )
{
    Parm* p = ParmMgr.CreateParm( type );
    if ( p )
    {
        p->Init( name, group, this, 0.0, -1.0e6, 1.0e6, true );
        p->SetDescript( "User Parm Descript" );
        m_UserParmVec.push_back( p );
        return p->GetID();
    }
    return string();
}

//==== Delete User Defined Parm ====//
void UserParmContainer::DeleteParm(int index )
{
    if ( index >= 0 && index < m_UserParmVec.size() )
    {
        delete m_UserParmVec[index];
        m_UserParmVec.erase( m_UserParmVec.begin() + index );
    }
}


