//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
//// version 1.3 as detailed in the LICENSE file which accompanies this software.
//

// LinkMgr.h: Parm Link Mgr Singleton.
//
//////////////////////////////////////////////////////////////////////

#include "LinkMgr.h"
#include "AdvLinkMgr.h"
#include "ParmMgr.h"
#include "Vehicle.h"
#include "StlHelper.h"

bool LinkMgrSingleton::m_firsttime = true;

//==== Constructor ====//
LinkMgrSingleton::LinkMgrSingleton()
{
    m_CurrLinkIndex = -1;
    m_WorkingLink = NULL;
    m_NumPredefinedUserParms = 16;
    m_UserParms.SetNumPredefined( m_NumPredefinedUserParms );
    m_UserParms.Renew(m_NumPredefinedUserParms);
    m_FreezeUpdateFlag = false;
}

void LinkMgrSingleton::Init()
{
    m_firsttime = false;

    m_WorkingLink = new Link( );

    m_WorkingLink->SetParmA( m_UserParms.GetUserParmId( 0 ) );
    m_WorkingLink->SetParmB( m_UserParms.GetUserParmId( 1 ) );
    RegisterContainer( m_UserParms.GetID() );
}

void LinkMgrSingleton::Wype()
{
    // public members
    UnRegisterContainer( m_UserParms.GetID() );
    m_UserParms.Renew(m_NumPredefinedUserParms);

    // private members
    m_CurrLinkIndex = int();
    delete( m_WorkingLink );
    m_WorkingLink = NULL;

    DelAllLinks();
    m_LinkVec = deque< Link* >();

    m_UpdatedParmVec = vector< string >();

    m_BaseLinkableContainers = vector< string >();
    m_LinkableContainers = vector< string >();
}

void LinkMgrSingleton::Renew()
{
    Wype();
    Init();
}

void LinkMgrSingleton::RegisterContainer( const string & id )
{
    m_BaseLinkableContainers.push_back( id );
}

void LinkMgrSingleton::UnRegisterContainer( const string & id )
{
    vector_remove_val( m_BaseLinkableContainers, id );
}

bool LinkMgrSingleton::CheckContainerRegistered( const string & id )
{
    return vector_contains_val( m_BaseLinkableContainers, id );
}

//==== Get Current Parm Link ====//
Link* LinkMgrSingleton::GetCurrLink()
{
    Link* link;
    if (  m_CurrLinkIndex >= 0 && m_CurrLinkIndex < ( int )m_LinkVec.size() )
    {
        link = m_LinkVec[ m_CurrLinkIndex ];
    }
    else
    {
        link = m_WorkingLink;
    }

    Parm* pA = ParmMgr.FindParm( link->GetParmA() );
    Parm* pB = ParmMgr.FindParm( link->GetParmB() );

    if ( !pA || !pB )
    {
        ResetWorkingLink();
        link = m_WorkingLink;
    }

    return link;
}

//==== Get Parm Link Given Index ====//
Link* LinkMgrSingleton::GetLink( int index )
{
    if ( index >= 0 && index < ( int )m_LinkVec.size() )
    {
        return m_LinkVec[ index ];
    }
    return m_WorkingLink;
}

//==== Check All Links For Valid Parms ====//
void LinkMgrSingleton::CheckLinks()
{
    //==== Check If Any Parms Have Added/Removed From Last Check ====//
    static int check_links_stamp = 0;
    if ( ParmMgr.GetNumParmChanges() == check_links_stamp )
    {
        return;
    }

    check_links_stamp = ParmMgr.GetNumParmChanges();

    deque< int > del_indices;
    for ( int i = 0 ; i < ( int )m_LinkVec.size() ; i++ )
    {
        Parm* pA = ParmMgr.FindParm( m_LinkVec[i]->GetParmA() );
        Parm* pB = ParmMgr.FindParm( m_LinkVec[i]->GetParmB() );

        if ( !pA || !pB )
        {
            del_indices.push_front( i );
        }
    }

    if ( del_indices.size() )
    {
        m_CurrLinkIndex = -1;
    }

    for ( int i = 0 ; i < ( int )del_indices.size() ; i++ )
    {
        m_LinkVec.erase( m_LinkVec.begin() + del_indices[i] );
    }

}


//==== Add Curr Link ====//
bool LinkMgrSingleton::AddCurrLink()
{
    //==== Check if Modifying Already Add Link ====//
    if (  m_CurrLinkIndex >= 0 && m_CurrLinkIndex < ( int )m_LinkVec.size() )
    {
        return false;
    }

    if ( m_WorkingLink->GetParmA() == m_WorkingLink->GetParmB() )
    {
        return false;
    }

    if ( CheckForDuplicateLink( m_WorkingLink->GetParmA(), m_WorkingLink->GetParmB() ) )
    {
        return false;
    }

    if ( AddLink( m_WorkingLink->GetParmA(), m_WorkingLink->GetParmB(), false ) )
    {
        // Initialize the link settings from the current working link
        m_LinkVec.back()->SetOffsetFlag( m_WorkingLink->GetOffsetFlag() );
        m_LinkVec.back()->m_Offset.Set( m_WorkingLink->m_Offset() );
        m_LinkVec.back()->SetScaleFlag( m_WorkingLink->GetScaleFlag() );
        m_LinkVec.back()->m_Scale.Set( m_WorkingLink->m_Scale() );
        m_LinkVec.back()->SetLowerLimitFlag( m_WorkingLink->GetLowerLimitFlag() );
        m_LinkVec.back()->m_LowerLimit.Set( m_WorkingLink->m_LowerLimit() );
        m_LinkVec.back()->SetUpperLimitFlag( m_WorkingLink->GetUpperLimitFlag() );
        m_LinkVec.back()->m_UpperLimit.Set( m_WorkingLink->m_UpperLimit() );
    }

    return true;
}

//==== Check For Duplicate Link  ====//
bool LinkMgrSingleton::CheckForDuplicateLink( const string & pA, const string &  pB )
{
    for ( int i = 0 ; i < ( int )m_LinkVec.size() ; i++ )
    {
        if ( m_LinkVec[i]->GetParmA() == pA &&
                m_LinkVec[i]->GetParmB() == pB )
        {
            return true;
        }
    }
    return false;
}

//==== Check If Parm is Used in Any Link ====//
bool LinkMgrSingleton::UsedInLink( const string & pid )
{
    for ( int i = 0 ; i < ( int )m_LinkVec.size() ; i++ )
    {
        if ( m_LinkVec[i]->GetParmA() == pid )
            return true;
        if ( m_LinkVec[i]->GetParmB() == pid )
            return true;
    }
    return false;
}


//==== Add New Link ====//
bool LinkMgrSingleton::AddLink( const string& pidA, const string& pidB, bool init_link_parms )
{
    //==== Make Sure Parm Are Not Already Linked ====//
    for ( int i = 0 ; i < ( int )m_LinkVec.size() ; i++ )
    {
        if ( m_LinkVec[i]->GetParmA() == pidA && m_LinkVec[i]->GetParmB() == pidB  )
        {
            return false;
        }
    }

    //==== Check If ParmIDs Are Valid ====//
    Parm* pA = ParmMgr.FindParm( pidA );
    Parm* pB = ParmMgr.FindParm( pidB );

    if ( pA == NULL || pB == NULL )
    {
        return false;
    }

    Link* pl = new Link();

    pl->SetParmA( pidA );
    pl->SetParmB( pidB );

    if ( init_link_parms )
    {
        pl->SetOffsetFlag( true );
        pl->m_Offset.Set( pB->Get() - pA->Get() );
        pl->SetScaleFlag( false );
        pl->m_Scale.Set( 1.0 );
    }

    m_LinkVec.push_back( pl );
    m_CurrLinkIndex = ( int )m_LinkVec.size() - 1;

    return true;
}

//==== Delete Curr Link ====//
void LinkMgrSingleton::DelCurrLink()
{
    if ( m_CurrLinkIndex < 0 || m_CurrLinkIndex >= ( int )m_LinkVec.size() )
    {
        return;
    }

    Link* pl = m_LinkVec[m_CurrLinkIndex];

    m_LinkVec.erase( m_LinkVec.begin() +  m_CurrLinkIndex );

    delete pl;

    m_CurrLinkIndex = -1;
}

//==== Delete All Links ====//
void LinkMgrSingleton::DelAllLinks()
{
    for ( int i = 0 ; i < ( int )m_LinkVec.size() ; i++ )
    {
        delete m_LinkVec[i];
    }

    m_LinkVec.clear();
    m_CurrLinkIndex = -1;
}
//==== Link All Parms In A Group ====//
bool LinkMgrSingleton::LinkAllGroup()
{
    Parm* pA =  ParmMgr.FindParm( m_WorkingLink->GetParmA() );
    Parm* pB =  ParmMgr.FindParm( m_WorkingLink->GetParmB() );

    if ( !pA || !pB )
    {
        return false;
    }

    string gnameA = pA->GetDisplayGroupName();
    string gnameB = pB->GetDisplayGroupName();

    if ( ( pA->GetLinkContainer() == pB->GetLinkContainer() ) && ( gnameA == gnameB ) )
    {
        return false;
    }

    vector< string > parmAVec, parmBVec;
    pA->GetLinkContainer()->GetParmIDs( pA->GetID(), parmAVec );
    pB->GetLinkContainer()->GetParmIDs( pB->GetID(), parmBVec );

    for ( int k = 0 ; k < ( int )parmAVec.size() ; k++ )
    {
        for ( int n = 0 ; n < ( int )parmBVec.size() ; n++ )
        {
            Parm* parmA = ParmMgr.FindParm( parmAVec[k] );
            Parm* parmB = ParmMgr.FindParm( parmBVec[n] );

            if ( parmA && parmB && ( parmA->GetName() == parmB->GetName() ) )
            {
                AddLink( parmAVec[k], parmBVec[n] );
            }
        }
    }

    return true;

}

//==== Link All Parms In A Container (Component) ====//
bool LinkMgrSingleton::LinkAllComp()
{
    Parm* pA =  ParmMgr.FindParm( m_WorkingLink->GetParmA() );
    Parm* pB =  ParmMgr.FindParm( m_WorkingLink->GetParmB() );

    ParmContainer* pcA = pA->GetLinkContainer();
    ParmContainer* pcB = pB->GetLinkContainer();

    if ( !pcA || !pcB )
    {
        return false;
    }

    if ( pcA == pcB )
    {
        return false;
    }

    vector< string > parmAVec, parmBVec;
    pcA->AddLinkableParms( parmAVec, pcA->GetID() );
    pcB->AddLinkableParms( parmBVec, pcB->GetID() );

    for ( int k = 0 ; k < ( int )parmAVec.size() ; k++ )
    {
        for ( int n = 0 ; n < ( int )parmBVec.size() ; n++ )
        {
            Parm* parmA = ParmMgr.FindParm( parmAVec[k] );
            Parm* parmB = ParmMgr.FindParm( parmBVec[n] );

            if ( parmA && parmB )
            {
                if ( parmA->GetName() == parmB->GetName() &&
                        parmA->GetDisplayGroupName() == parmB->GetDisplayGroupName() )
                {
                    AddLink( parmAVec[k], parmBVec[n] );
                }
            }
        }
    }

    return true;
}

//==== Reset Working Parm Link ====//
void LinkMgrSingleton::ResetWorkingLink()
{
    m_CurrLinkIndex = -1;

    m_WorkingLink->SetParmA( m_UserParms.GetUserParmId( 0 ) );
    m_WorkingLink->SetParmB( m_UserParms.GetUserParmId( 1 ) );

    m_WorkingLink->m_Scale = 1.0;
    m_WorkingLink->SetScaleFlag( false );

    m_WorkingLink->m_Offset = 0.0;
    m_WorkingLink->SetOffsetFlag( true );
}

//==== Parm Changed ====//
void LinkMgrSingleton::ParmChanged( const string& pid, bool start_flag  )
{
    //==== Find Parm Ptr ===//
    Parm* parm_ptr = ParmMgr.FindParm( pid );
    if ( !parm_ptr )
        return;

    if ( m_FreezeUpdateFlag )
        return;

    //==== Check For Advanced Links ====//
    bool adv_link_flag = AdvLinkMgr.IsInputParm( pid );

    //==== Look for Reg Links  ====//
    vector < Link* > parm_link_vec;
    for ( int i = 0 ; i < ( int )m_LinkVec.size() ; i++ )
    {
        if ( m_LinkVec[i]->GetParmA() == pid )
        {
            parm_link_vec.push_back( m_LinkVec[i] );
        }
    }

    //==== Check Links ====//
    bool reg_link_flag = false;
    if ( parm_link_vec.size() )
    {
        reg_link_flag = true;
    }

    //==== Abort if No Links ====//
    if ( !adv_link_flag && !reg_link_flag )
        return;

    //==== Set Link Update Flag ====//
    parm_ptr->SetLinkUpdateFlag( true );
    m_UpdatedParmVec.push_back( parm_ptr->GetID() );

    //==== Update Linked Parms ====//
    for ( int i = 0 ; i < ( int )parm_link_vec.size() ; i++ )
    {
        Link* pl = parm_link_vec[i];
        Parm* pB = ParmMgr.FindParm( pl->GetParmB() );

        if ( pB && ! pB->GetLinkUpdateFlag() )       // Prevent Circular
        {
            double offset = 0.0;
            if ( pl->GetOffsetFlag() )
            {
                offset = pl->m_Offset();
            }
            double scale = 1.0;
            if ( pl->GetScaleFlag() )
            {
                scale = pl->m_Scale();
            }

            double val = parm_ptr->Get() * scale + offset;

            if ( pl->GetLowerLimitFlag() && val < pl->m_LowerLimit() )      // Constraints
            {
                val = pl->m_LowerLimit();
            }

            if ( pl->GetUpperLimitFlag() && val > pl->m_UpperLimit() )      // Constraints
            {
                val = pl->m_UpperLimit();
            }

            pB->SetFromLink( val );
        }
    }

    //==== Update Adv Link ===//
    if ( adv_link_flag )
    {
        AdvLinkMgr.UpdateLinks( pid );
    }

    //==== Clean Up ====/
    if ( start_flag )      
    {
        for ( int i = 0 ; i < ( int )m_UpdatedParmVec.size() ; i++ )
        {
            Parm* p = ParmMgr.FindParm( m_UpdatedParmVec[i] );
            if ( p )
            {
                p->SetLinkUpdateFlag( false );
            }
        }
        m_UpdatedParmVec.clear();

        Vehicle* veh = VehicleMgr.GetVehicle();
        if ( veh )
        {
            veh->ParmChanged( parm_ptr, Parm::SET );
        }
    }
}



void LinkMgrSingleton::SetParm( bool flagA, string parm_id )
{
    if ( !ParmMgr.FindParm( parm_id ) )
    {
        if ( flagA )
        {
            parm_id = m_UserParms.GetUserParmId( 0 );
        }
        else
        {
            parm_id = m_UserParms.GetUserParmId( 1 );
        }
    }

    if ( flagA )
    {
        m_WorkingLink->SetParmA( parm_id );
    }
    else
    {
        m_WorkingLink->SetParmB( parm_id );
    }

    m_WorkingLink->InitOffsetScale();
}

ParmContainer* LinkMgrSingleton::FindParmContainer( int index )
{
    if ( index >= 0 && index < ( int )m_LinkableContainers.size() )
    {
        string id = m_LinkableContainers[index];
        return ParmMgr.FindParmContainer( id );
    }

    return NULL;
}

void LinkMgrSingleton::SetParm( bool flagA, int container_ind, int group_ind, int parm_ind )
{
    ParmContainer* pc = FindParmContainer( container_ind );

    if ( pc )
    {
        SetParm( flagA, PickParm( container_ind, group_ind, parm_ind ) );
    }
    else
    {
        ResetWorkingLink();
    }
}

string LinkMgrSingleton::PickParm( int container_ind, int group_ind, int parm_ind )
{
    ParmContainer* pc = FindParmContainer( container_ind );

    string pid;

    if ( pc )
    {
        pid = pc->FindParm( group_ind, parm_ind );
    }

    return pid;
}

//==== Find All Linkable Containers and Parms ====//
void LinkMgrSingleton::BuildLinkableParmData()
{
    //==== Check If Any Parms Have Added/Removed From Last Build ====//
    static int build_linkable_stamp = 0;
    if ( ParmMgr.GetNumParmChanges() == build_linkable_stamp )
    {
        return;
    }

    build_linkable_stamp = ParmMgr.GetNumParmChanges();

    m_LinkableContainers.clear();

    for ( int i = 0 ; i < ( int )m_BaseLinkableContainers.size() ; i++ )
    {
        ParmContainer* pc = ParmMgr.FindParmContainer( m_BaseLinkableContainers[i] );
        if ( pc )
        {
            //==== Find All Linkable Containers (Children of Base Container) And Parms ===//
            pc->AddLinkableContainers( m_LinkableContainers );
        }
    }

    ParmMgr.SetDirtyFlag( false );
}

//==== Get Container Name Vec And Find Match Index For Parm_ID ====//
int LinkMgrSingleton::GetCurrContainerVec( const string& parm_id, vector< string > & idVec )
{
    int index = 0;

    if ( ParmMgr.GetDirtyFlag() )
    {
        BuildLinkableParmData();
    }

    //==== Find Container ID ====//
    string curr_container_id;
    Parm* p = ParmMgr.FindParm( parm_id );
    if ( p )
    {
        curr_container_id = p->GetLinkContainerID();
    }

    //==== Find Index And Load Container Names ===//
    for ( int i = 0 ; i < ( int )m_LinkableContainers.size() ; i++ )
    {
        if ( m_LinkableContainers[i] == curr_container_id )
        {
            index = i;
        }

        ParmContainer* pc = ParmMgr.FindParmContainer( m_LinkableContainers[i] );

        string name;
        if ( pc )
        {
            name = pc->GetName();
        }

        idVec.push_back( name );
    }

    return index;
}

//==== Get Group Name Vec And Find Match Index For Parm_ID ====//
int LinkMgrSingleton::GetCurrGroupNameVec( const string& parm_id, vector< string > & nameVec )
{
    int index = 0;
    if ( ParmMgr.GetDirtyFlag() )
    {
        LinkMgr.BuildLinkableParmData();
    }

    if ( m_LinkableContainers.size() == 0 )
    {
        return index;
    }

    Parm* p = ParmMgr.FindParm( parm_id );
    if ( p )
    {
        ParmContainer* pc = p->GetLinkContainer();
        if ( pc )
        {
            return pc->GetGroupNames( parm_id, nameVec );
        }
    }

    return index;
}

//==== Get Parm ID Vec And Find Match Index For Parm_ID ====//
int LinkMgrSingleton::GetCurrParmIDVec( const string& parm_id, vector< string > & idVec )
{
    int index = 0;
    if ( ParmMgr.GetDirtyFlag() )
    {
        LinkMgr.BuildLinkableParmData();
    }

    if ( m_LinkableContainers.size() == 0 )
    {
        return index;
    }

    Parm* p = ParmMgr.FindParm( parm_id );
    if ( p )
    {
        ParmContainer* pc = p->GetLinkContainer();
        if ( pc )
        {
            return pc->GetParmIDs( parm_id, idVec );
        }
    }

    return index;
}

xmlNodePtr LinkMgrSingleton::EncodeXml( xmlNodePtr & node )
{
    m_UserParms.EncodeXml( node );

    xmlNodePtr linkmgr_node = xmlNewChild( node, NULL, BAD_CAST"LinkMgr", NULL );

    for ( int i = 0 ; i < ( int )m_LinkVec.size() ; i++ )
    {
        if( m_LinkVec[i] )
        {
            m_LinkVec[i]->EncodeXml( linkmgr_node );
        }
    }


    return linkmgr_node;
}

xmlNodePtr LinkMgrSingleton::DecodeXml( xmlNodePtr & node )
{
    m_UserParms.DecodeXml( node );

    if ( m_UserParms.GetNumUserParms() < m_NumPredefinedUserParms )
    {
        m_UserParms.Renew(m_NumPredefinedUserParms);
    }

    xmlNodePtr linkmgr_node = XmlUtil::GetNode( node, "LinkMgr", 0 );
    if ( linkmgr_node )
    {
        int num = XmlUtil::GetNumNames( linkmgr_node, "Link" );
        for ( int i = 0 ; i < num ; i++ )
        {
            xmlNodePtr link_node = XmlUtil::GetNode( linkmgr_node, "Link", i );
            if ( link_node )
            {
                Link* link = new Link();
                link->DecodeXml( link_node );

                AddLink( link );
            }
        }
    }

    return linkmgr_node;
}

string LinkMgrSingleton::AddUserParm(int type, const string & name, const string & group )
{
    //==== Check For Duplicate ====//
    int num_parms =  GetNumUserParms() - GetNumPredefinedUserParms();
    for ( int i = 0 ; i < num_parms ; i++ )
    {
        string pid = LinkMgr.GetUserParmId( i +  GetNumPredefinedUserParms() );
        Parm* pptr = ParmMgr.FindParm( pid );
        if ( pptr && pptr->GetName() == name && pptr->GetGroupName() == group )
        {
            return string();
        }
    }
    return m_UserParms.AddParm( type, name, group );
 }

void LinkMgrSingleton::DeleteUserParm( int index )
{
    if ( index >= m_NumPredefinedUserParms && index < m_UserParms.GetNumUserParms() )
    {
        m_UserParms.DeleteParm( index );
    }
}

void LinkMgrSingleton::DeleteAllUserParm( )
{
    while ( m_UserParms.GetNumUserParms() > m_NumPredefinedUserParms )
    {
        m_UserParms.DeleteParm( m_UserParms.GetNumUserParms() - 1 );
    }
}

bool LinkNameCompareA( const Link *lnkA, const Link *lnkB )
{
    return NameCompare( lnkA->GetParmA(), lnkB->GetParmA() );
}

bool LinkNameCompareB( const Link *lnkA, const Link *lnkB )
{
    return NameCompare( lnkA->GetParmB(), lnkB->GetParmB() );
}

void LinkMgrSingleton::SortLinksByA()
{
    std::sort( m_LinkVec.begin(), m_LinkVec.end(), LinkNameCompareA );
}

void LinkMgrSingleton::SortLinksByB()
{
    std::sort( m_LinkVec.begin(), m_LinkVec.end(), LinkNameCompareB );
}
