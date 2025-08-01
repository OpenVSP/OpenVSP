//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
//// version 1.3 as detailed in the LICENSE file which accompanies this software.
//

// AdvLinkMgr.h: Parm Adv Link Mgr Singleton.
//
//////////////////////////////////////////////////////////////////////

#include "AdvLinkMgr.h"
#include "ParmMgr.h"
#include "StringUtil.h"
#include "StlHelper.h"
#include "AttributeManager.h"


//==== Constructor ====//
AdvLinkMgrSingleton::AdvLinkMgrSingleton()
{
    m_ActiveLink = nullptr;
    m_EditLinkIndex = 0;

}

void AdvLinkMgrSingleton::Init()
{

}

void AdvLinkMgrSingleton::Wype()
{
    //==== Delete All Links ====//
    for ( int i = 0 ; i < (int)m_LinkVec.size() ; i++ )
    {
        AttributeMgr.DeregisterCollID( m_LinkVec[i]->GetAttrCollection()->GetID() );
        delete m_LinkVec[i];
    }
    m_LinkVec.clear();
    m_ActiveLink = nullptr;
    m_EditLinkIndex = 0;
}

void AdvLinkMgrSingleton::Renew()
{
    Wype();
    Init();
}

//==== Check For Duplicate Link Name =====//
bool AdvLinkMgrSingleton::DuplicateLinkName( const string & name )
{
    for ( int i = 0 ; i < (int)m_LinkVec.size() ; i++ )
    {
        if ( m_LinkVec[i]->GetName() == name )
            return true;
    }
    return false;
}

//==== Add A Link =====//
AdvLink* AdvLinkMgrSingleton::AddLink( const string & name )
{
    //==== Create Unique Name ====//
    string base_name = name;
    if ( base_name.size() == 0 )
    {
        base_name = "Unnamed_Link";
    }
    string link_name = base_name;

    int cnt = 1;
    while ( DuplicateLinkName( link_name ) )
    {
        link_name = base_name + "_" + StringUtil::int_to_string( cnt, "%d" );
        cnt++;
    }

    AdvLink* alink = new AdvLink();
    alink->SetName( link_name );
    m_LinkVec.push_back( alink );
    m_EditLinkIndex = (int)m_LinkVec.size() - 1;

    AttributeMgr.RegisterCollID( m_LinkVec.back()->GetAttrCollection()->GetID(), m_LinkVec.back()->GetAttrCollection() );

    return alink;
}

void AdvLinkMgrSingleton::DelLink( AdvLink* link_ptr )
{
    if ( !link_ptr )
    {
        return;
    }

    if ( m_ActiveLink == link_ptr )
    {
        m_ActiveLink = nullptr;
    }
    m_EditLinkIndex = -1;

    vector_remove_val( m_LinkVec, link_ptr );
    AttributeMgr.DeregisterCollID( link_ptr->GetAttrCollection()->GetID() );

    delete link_ptr;
}

void AdvLinkMgrSingleton::DelLink( int index )
{
    AdvLink * link_ptr = GetLink( index );

    DelLink( link_ptr );
}

void AdvLinkMgrSingleton::DelAllLinks( )
{
    m_EditLinkIndex = -1;
    m_ActiveLink = nullptr;

    for ( int i = 0 ; i < (int)m_LinkVec.size() ; i++ )
    {
        AttributeMgr.DeregisterCollID( m_LinkVec[i]->GetAttrCollection()->GetID() );
        delete m_LinkVec[i];
    }
    m_LinkVec.clear();
}

void AdvLinkMgrSingleton::CheckLinks()
{
    //==== Check If Any Parms Have Added/Removed From Last Check ====//
    static int check_links_stamp = 0;
    if ( ParmMgr.GetNumParmChanges() == check_links_stamp )
    {
        return;
    }

    check_links_stamp = ParmMgr.GetNumParmChanges();

    for ( int i = 0 ; i < ( int )m_LinkVec.size() ; i++ )
    {
        m_LinkVec[i]->ValidateParms();
    }
}



AdvLink* AdvLinkMgrSingleton::GetLink( int index )
{
    if ( index >= 0 && index < (int)m_LinkVec.size() )
    {
        return m_LinkVec[index];
    }
    return nullptr;

}

int AdvLinkMgrSingleton::GetLinkIndex( const string & name )
{
    vector < string > link_names;
    link_names = GetLinkNames();
    return vector_find_val( link_names, name );
}

vector< string > AdvLinkMgrSingleton::GetLinkNames()
{
    vector < string > link_names( m_LinkVec.size() );

    for ( int i = 0 ; i < ( int )m_LinkVec.size() ; i++ )
    {
        link_names[i] = m_LinkVec[i]->GetName();
    }
    return link_names;
}

void AdvLinkMgrSingleton::AddInput( const string & parm_id, const string & var_name )
{
    AddInputOutput( parm_id, var_name, true );
}

void AdvLinkMgrSingleton::AddOutput( const string & parm_id, const string & var_name )
{
    AddInputOutput( parm_id, var_name, false );
}

void AdvLinkMgrSingleton:: AddInputOutput( const string & parm_id, const string & var_name, bool input_flag )
{
    AdvLink* edit_link = GetLink( GetEditLinkIndex() );
    if ( !edit_link )
        return;

    //==== Find Parm Ptr ===//
    Parm* parm_ptr = ParmMgr.FindParm( parm_id );
    if ( !parm_ptr )
        return;

    VarDef pd;
    pd.m_ParmID = parm_id;
    pd.m_VarName = var_name;

    edit_link->AddVar( pd, input_flag );
}


void AdvLinkMgrSingleton::SetVar( const string & var_name, double val )
{
    if ( !m_ActiveLink )
        return;

    m_ActiveLink->SetVar( var_name, val );

}

double AdvLinkMgrSingleton::GetVar( const string & var_name )
{
    if ( !m_ActiveLink )
        return 0.0;

    return m_ActiveLink->GetVar( var_name );
}

bool AdvLinkMgrSingleton::IsInputParm( const string& pid )
{
    for ( int i = 0 ; i < (int)m_LinkVec.size() ; i++ )
    {
         vector< VarDef > def_vec = m_LinkVec[i]->GetInputVars();
         for ( int j = 0 ; j < (int)def_vec.size() ; j++ )
         {
             if ( pid == def_vec[j].m_ParmID )
             {
                 if ( ParmMgr.FindParm( pid ) )
                 {
                     return true;
                 }
             }
         }
    }
    return false;
}

bool AdvLinkMgrSingleton::IsOutputParm( const string& pid )
{
    for ( int i = 0 ; i < (int)m_LinkVec.size() ; i++ )
    {
         vector< VarDef > def_vec = m_LinkVec[i]->GetOutputVars();
         for ( int j = 0 ; j < (int)def_vec.size() ; j++ )
         {
             if ( pid == def_vec[j].m_ParmID )
             {
                 if ( ParmMgr.FindParm( pid ) )
                 {
                     return true;
                 }
             }
         }
    }
    return false;
}

//==== Parm Changed ====//
void AdvLinkMgrSingleton::UpdateLinks( const string& pid  )
{
    //==== Find Parm Ptr ===//
    Parm* parm_ptr = ParmMgr.FindParm( pid );
    if ( !parm_ptr )
    {
        return;
    }

    //==== Check All Links And Update If Needed ====//
    for ( int i = 0 ; i < (int)m_LinkVec.size() ; i++ )
    {
        m_LinkVec[i]->UpdateLink( pid );
    }
}

//==== Force Update of All Links ====//
void AdvLinkMgrSingleton::ForceUpdate()
{
    for ( int i = 0 ; i < ( int )m_LinkVec.size() ; i++ )
    {
        m_LinkVec[i]->ForceUpdate();
    }
}

xmlNodePtr AdvLinkMgrSingleton::EncodeXml( xmlNodePtr & node )
{
    xmlNodePtr linkmgr_node = xmlNewChild( node, nullptr, BAD_CAST"AdvLinkMgr", nullptr );

    for ( int i = 0 ; i < ( int )m_LinkVec.size() ; i++ )
    {
        if( m_LinkVec[i] )
        {
            m_LinkVec[i]->EncodeXml( linkmgr_node );
        }
    }


    return linkmgr_node;
}

xmlNodePtr AdvLinkMgrSingleton::DecodeXml( xmlNodePtr & node )
{
    xmlNodePtr linkmgr_node = XmlUtil::GetNode( node, "AdvLinkMgr", 0 );
    if ( linkmgr_node )
    {
        int num = XmlUtil::GetNumNames( linkmgr_node, "AdvLink" );
        for ( int i = 0 ; i < num ; i++ )
        {
            xmlNodePtr link_node = XmlUtil::GetNode( linkmgr_node, "AdvLink", i );
            if ( link_node )
            {
                AdvLink* link = AddLink( "" );
                link->DecodeXml( link_node );
                link->BuildScript();
            }
        }
    }

    return linkmgr_node;
}
