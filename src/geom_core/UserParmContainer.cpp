//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//

// ParmContainer.cpp: implementation of the Parm Container classes.
//
//////////////////////////////////////////////////////////////////////

#include "Parm.h"
#include "ParmMgr.h"
#include "UserParmContainer.h"
#include "Vehicle.h"

#include "StringUtil.h"
#include "StlHelper.h"

using std::string;

//=========================================================================//
//==================        User Parm Container       =====================//
//=========================================================================//

//==== Constructor ====//
UserParmContainer::UserParmContainer() : ParmContainer()
{
    m_NumPredefined = 0;
    m_Name = "UserParms";
    Init(0);
}

//==== Destructor ====//
UserParmContainer::~UserParmContainer()
{
    Wype();
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
        veh->ParmChanged( parm_ptr, type );
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
    int num_curr_custom = m_UserParmVec.size() - m_NumPredefined;

    xmlNodePtr child_node = XmlUtil::GetNode( node, "UserParmContainer", 0 );
    if ( child_node )
    {
        int num_user = XmlUtil::FindInt( child_node, "NumUserParms", 0 );

        //==== Decode All User Parms ====//
        if ( num_curr_custom == 0 )
        {
            Renew( num_user );
            for ( int i = 0; i < static_cast<int>( m_UserParmVec.size() ); i++ )
            {
                xmlNodePtr pnode = XmlUtil::GetNode( child_node, "UserParm", i );
                if ( pnode && m_UserParmVec[i] )
                {
                    m_UserParmVec[i]->DecodeXml( pnode, true );
                }
            }
        }
        else
        {
            //==== Decode Predefined ====//
            for ( int i = 0; i < m_NumPredefined ; i++ )
            {
                xmlNodePtr pnode = XmlUtil::GetNode( child_node, "UserParm", i );
                if ( pnode && m_UserParmVec[i] )
                {
                    m_UserParmVec[i]->DecodeXml( pnode, true );
                }
            }
            //==== Append New Custom ====//
            int num_new_custom = num_user - m_NumPredefined;

            for ( int i = 0 ; i < num_new_custom ; i++ )
            {
                xmlNodePtr pnode = XmlUtil::GetNode( child_node, "UserParm", i + m_NumPredefined );
                Parm* p = new Parm();
                p->Init( "Temp", "User_Group", this, 0.0, -1.0e12, 1.0e12 );
                p->DecodeXml( pnode, true );
                m_UserParmVec.push_back( p );
            }
        }
    }
    SortVars();
    return child_node;
}

//==== Add User Defined Parm ====//
string UserParmContainer::AddParm(int type, const string & name, const string & group )
{
    Parm* p = ParmMgr.CreateParm( type );
    if ( p )
    {
        p->Init( name, group, this, 0.0, -1.0e6, 1.0e6 );
        p->SetDescript( "User Parm Descript" );
        m_UserParmVec.push_back( p );
        SortVars();
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

bool UserParmNameCompare( const Parm *dvA, const Parm *dvB )
{
    return NameCompare( dvA->GetID(), dvB->GetID() );
}

bool UserParmContainer::SortVars()
{
    if ( m_UserParmVec.size() < m_NumPredefined )
    {
        return false;
    }

    bool wassorted = std::is_sorted( m_UserParmVec.begin() + m_NumPredefined, m_UserParmVec.end(), UserParmNameCompare );

    if ( !wassorted )
    {
        std::sort( m_UserParmVec.begin() + m_NumPredefined, m_UserParmVec.end(), UserParmNameCompare );
    }

    return wassorted;
}
