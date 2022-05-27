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
        if ( c_name_A.compare( c_name_B ) != 0 )
        {
            return c_name_A < c_name_B;
        }

        string cAID = pA->GetContainerID();
        string cBID = pB->GetContainerID();

        // Matching container names, sort by container ID
        if ( cAID.compare( cBID ) != 0 )
        {
            return cAID < cBID;
        }

        // Matching ID, sort by group
        if ( g_name_A.compare( g_name_B ) != 0 )
        {
            return g_name_A < g_name_B;
        }

        // Matching group, sort by parameter
        if ( p_name_A.compare( p_name_B ) != 0 )
        {
            return p_name_A < p_name_B;
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
    m_Name = string( "Default" );
    m_LateUpdateFlag = true; // Force update first time through.
    ParmMgr.AddParmContainer( this );
}

//==== Destructor ====//
ParmContainer::~ParmContainer()
{
    ParmMgr.RemoveParmContainer( this );
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

//==== Return Pointer To Parent Container ====//
ParmContainer* ParmContainer::GetParentContainerPtr() const
{
    return ParmMgr.FindParmContainer( m_ParentContainer );
}

//==== Create A Unique ID  =====//
string ParmContainer::GenerateID()
{
    return GenerateRandomID( 10 );
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
    ParmMgr.IncNumParmChanges();

    map< string, vector< string > >::iterator groupIter;
    for ( groupIter = m_GroupParmMap.begin() ; groupIter != m_GroupParmMap.end() ; ++groupIter )
    {
        string name = groupIter->first;
        gnode = xmlNewChild( parmcontain_node, NULL, BAD_CAST name.c_str(), NULL );

        if ( gnode )
        {
            vector< string >::iterator parmIter;
            for ( parmIter = groupIter->second.begin(); parmIter != groupIter->second.end(); ++parmIter )
            {
                Parm* p = ParmMgr.FindParm( ( *parmIter ) );
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
        string newID = ParmMgr.RemapID( oldID, m_ID );

        if( newID.compare( m_ID ) != 0 )        // they differ
        {
            ChangeID( newID );
        }

        SetName( XmlUtil::FindString(  child_node, "Name", m_Name ) );
    }

    xmlNodePtr gnode;

    LoadGroupParmVec( m_ParmVec, false );
    ParmMgr.IncNumParmChanges();

    map< string, vector< string > >::iterator groupIter;
    for ( groupIter = m_GroupParmMap.begin() ; groupIter != m_GroupParmMap.end() ; ++groupIter )
    {
        string name = groupIter->first;
        gnode = XmlUtil::GetNode( child_node, name.c_str(), 0 );

        if ( gnode )
        {
            vector< string >::iterator parmIter;
            for ( parmIter = groupIter->second.begin(); parmIter != groupIter->second.end(); ++parmIter )
            {
                Parm* p = ParmMgr.FindParm( ( *parmIter ) );
                if ( p )
                {
                    p->DecodeXml( gnode );
                }
            }
        }
    }

    return child_node;

}

//==== Name Compare ====//
bool ParmNameCompare( const string& a, const string& b )
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
    for ( iter = m_GroupParmMap.begin() ; iter != m_GroupParmMap.end() ; ++iter )
    {
        m_GroupNames.push_back( iter->first );
        sort( iter->second.begin(), iter->second.end(), ParmNameCompare );
    }

    // Rebuild Parm Links. Could also call LinkMgr.BuildLinkableParmData(), but this allows the links 
    // to just be rebuild once after this function is iteratively called when saving or loading a model.
    ParmMgr.SetDirtyFlag( true ); 
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
    if ( ParmMgr.GetDirtyFlag() )
    {
        LinkMgr.BuildLinkableParmData();
    }

    string id;
    map< string, vector< string > >::iterator iter;
    iter = m_GroupParmMap.find( group_name );

    if ( iter != m_GroupParmMap.end() )
    {
        //==== Look For Parm Name ====//
        vector< string > pid_vec = iter->second;
        for ( int i = 0 ; i < (int)pid_vec.size() ; i++ )
        {
            Parm* p = ParmMgr.FindParm( pid_vec[i] );
            if ( p )
            {
                if ( p->GetName() == parm_name )
                {
                    return pid_vec[i];
                }
            }
        }
    }

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

//==== Get Vector of Group Names ====//
void ParmContainer::GetGroupNames( vector< string > & group_names )
{
    map< string, vector< string > >::iterator iter;
    for ( iter = m_GroupParmMap.begin() ; iter != m_GroupParmMap.end() ; ++iter )
    {
        string name = iter->first;
        group_names.push_back( name );
    }
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
    for ( iter = m_GroupParmMap.begin() ; iter != m_GroupParmMap.end() ; ++iter )
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

        if ( p )
        {
            if ( link_container_id.size() )
            {
                p->SetLinkContainerID( link_container_id );
            }
            linkable_parm_vec.push_back( p->GetID() );
        }
    }
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
