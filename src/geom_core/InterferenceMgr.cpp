//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//

// InterferenceMgr.cpp
//
// Rob McDonald
//////////////////////////////////////////////////////////////////////

#include "InterferenceMgr.h"

#include "StlHelper.h"
#include "Vehicle.h"

InterferenceCase::InterferenceCase()
{

}

void InterferenceCase::Update()
{
}

xmlNodePtr InterferenceCase::EncodeXml( xmlNodePtr & node )
{
    xmlNodePtr icase_node = xmlNewChild( node, NULL, BAD_CAST"InterferenceCase", NULL );

    ParmContainer::EncodeXml( icase_node );

    return icase_node;
}

xmlNodePtr InterferenceCase::DecodeXml( xmlNodePtr & node )
{
    ParmContainer::DecodeXml( node );

    return node;
}


//===============================================================================//
//===============================================================================//
//===============================================================================//


InterferenceMgrSingleton::InterferenceMgrSingleton()
{

}

InterferenceMgrSingleton::~InterferenceMgrSingleton()
{
    Wype();
}

xmlNodePtr InterferenceMgrSingleton::EncodeXml( xmlNodePtr & node )
{
    xmlNodePtr interferencemgr_node = xmlNewChild( node, NULL, BAD_CAST"InterferenceMgr", NULL );

    if ( interferencemgr_node )
    {
        for ( int i = 0; i < m_ICaseVec.size(); i++ )
        {
            m_ICaseVec[i]->EncodeXml( interferencemgr_node );
        }
    }

    return interferencemgr_node;
}

xmlNodePtr InterferenceMgrSingleton::DecodeXml( xmlNodePtr & node )
{
    xmlNodePtr interferencemgr_node = XmlUtil::GetNode( node, "InterferenceMgr", 0 );

    if ( interferencemgr_node )
    {


        int num_icase = XmlUtil::GetNumNames( interferencemgr_node, "InterferenceCase" );
        for ( int i = 0; i < num_icase; i++ )
        {
            xmlNodePtr icasenode = XmlUtil::GetNode( interferencemgr_node, "InterferenceCase", i );

            if ( icasenode )
            {
                string id = AddInterferenceCase();
                InterferenceCase* icase = GetInterferenceCase( id );

                if ( icase )
                {
                    icase->DecodeXml( icasenode );
                }
            }
        }
    }

    return node;
}

void InterferenceMgrSingleton::Renew()
{
    Wype();
}

void InterferenceMgrSingleton::Wype()
{
    for ( int i = 0; i < (int)m_ICaseVec.size(); i++ )
    {
        delete m_ICaseVec[i];
    }
    m_ICaseVec.clear();
}

void InterferenceMgrSingleton::Update()
{
    for ( int i = 0; i < (int)m_ICaseVec.size(); i++ )
    {
        m_ICaseVec[i]->Update();
    }

}

void InterferenceMgrSingleton::AddLinkableContainers( vector< string > & linkable_container_vec )
{
    for ( int i = 0; i < (int)m_ICaseVec.size(); i++ )
    {
        m_ICaseVec[i]->AddLinkableContainers( linkable_container_vec );
    }
}

string InterferenceMgrSingleton::AddInterferenceCase()
{
    InterferenceCase* ic = new InterferenceCase();

    m_ICaseVec.push_back( ic );
    return ic->GetID();
}

void InterferenceMgrSingleton::DeleteInterferenceCase( const string &id )
{
    int indx = GetInterferenceCaseIndex( id );
    DeleteInterferenceCase( indx );
}

void InterferenceMgrSingleton::DeleteInterferenceCase( int indx )
{
    if ( indx >= 0 && indx < m_ICaseVec.size() )
    {
        delete m_ICaseVec[ indx ];

        m_ICaseVec.erase( m_ICaseVec.begin() + indx );
    }
}

void InterferenceMgrSingleton::DeleteAllInterferenceCases()
{
    for ( int i = 0; i < (int)m_ICaseVec.size(); i++ )
    {
        delete m_ICaseVec[i];
    }
    m_ICaseVec.clear();
}

InterferenceCase * InterferenceMgrSingleton::GetInterferenceCase( int indx ) const
{
    if ( indx >= 0 && indx < m_ICaseVec.size() )
    {
        return m_ICaseVec[ indx ];
    }
    return nullptr;
}

InterferenceCase * InterferenceMgrSingleton::GetInterferenceCase( const string &id ) const
{
    for ( int i = 0; i < (int)m_ICaseVec.size(); i++ )
    {
        if ( m_ICaseVec[i]->GetID() == id )
        {
            return m_ICaseVec[i];
        }
    }

    return nullptr;
}

int InterferenceMgrSingleton::GetInterferenceCaseIndex( const string &id ) const
{
    for ( int i = 0; i < (int)m_ICaseVec.size(); i++ )
    {
        if ( m_ICaseVec[i]->GetID() == id )
        {
            return i;
        }
    }

    return -1;
}
