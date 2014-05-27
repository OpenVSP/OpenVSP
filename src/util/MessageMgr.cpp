//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//
#include <assert.h>

#include "MessageMgr.h"
using std::map;
using std::string;
using std::deque;

//==== Message Data ====//
MessageData::MessageData()
{
    m_Ptr = NULL;
}

//==== Message Base ====//
MessageBase::MessageBase()
{
    m_Name = "DefaultName";
}

//==== Message Base ====//
MessageBase::~MessageBase()
{
    UnRegister();
}

/** @brief Register this MessageBase listener.
 */
void MessageBase::Register()
{
    MessageMgr::getInstance().Register( this );
}

/** @brief Set name and register this MessageBase listener.
 *
 * @param[in] name   Message listener name.
 *
 */
void MessageBase::Register( const string & name )
{
    SetName( name );
    Register();
}

/** @brief UnRegister this MessageBase listener.
 */
void MessageBase::UnRegister()
{
    MessageMgr::getInstance().UnRegister( this );
}

//==== Constructor ====//
MessageMgr::MessageMgr()
{
}

/** @brief Register MessageBase listener.
 *
 * @param[in] msg_base Listener to register.
 */
void MessageMgr::Register( MessageBase* msg_base )
{
    m_MessageRegMap[msg_base->GetName()].push_back( msg_base );
}

/** @brief UnRegister MessageBase listener.
 *
 * @param[in] msg_base Listener to unregister.
 */
void MessageMgr::UnRegister( MessageBase* msg_base )
{
    map< string, deque< MessageBase* > >::iterator iter;

    if ( m_MessageRegMap.size() == 0 )
    {
        return;
    }

    iter = m_MessageRegMap.begin();

    // In a case where std::map has only one element in its list, map::end() returns
    // bad pointer.  Handle one element std::map as special case.
    if ( m_MessageRegMap.size() == 1 )
    {
        deque< int > erase_index_vec;
        for ( int i = 0 ; i < ( int )( iter->second ).size() ; i++ )
        {
            if ( ( iter->second )[i] == msg_base )
            {
                erase_index_vec.push_front( i );
            }
        }

        for ( int e = 0 ; e < ( int )erase_index_vec.size() ; e++ )
        {
            int erase_index = erase_index_vec[e];
            ( iter->second ).erase( ( iter->second ).begin() + erase_index );
        }
    }
    else
    {
        while ( iter !=  m_MessageRegMap.end() )
        {
            deque< int > erase_index_vec;
            for ( int i = 0 ; i < ( int )( iter->second ).size() ; i++ )
            {
                if ( ( iter->second )[i] == msg_base )
                {
                    erase_index_vec.push_front( i );
                }
            }

            for ( int e = 0 ; e < ( int )erase_index_vec.size() ; e++ )
            {
                int erase_index = erase_index_vec[e];
                ( iter->second ).erase( ( iter->second ).begin() + erase_index );
            }
            iter++;
        }
    }

}

/** @brief Send string message to designated receiver, from undesignated sender.
 *
 * @param[in] to_name Targeted message receiver.
 * @param[in] msg     Message to send.
 */
void MessageMgr::Send( const string& to_name, const string& msg )
{
    MessageData data;
    data.m_String = msg;
    Send( to_name, NULL, data );
}

/** @brief Send MessageData message to designated receiver, from undesignated sender.
 *
 * @param[in] to_name Targeted message receiver.
 * @param[in] data    Message to send.
 */
void MessageMgr::Send( const string& to_name, const MessageData& data )
{
    Send( to_name, NULL, data );
}

/** @brief Send MessageData message to designated receiver, from designated sender.
 *
 * @param[in] to_name   Targeted message receiver.
 * @param[in] from_base Message sender.
 * @param[in] data      Message to send.
 */
void MessageMgr::Send( const string& to_name, const MessageBase* from_base, const MessageData& data  )
{
    map< string, deque< MessageBase* > >::iterator iter;

    iter = m_MessageRegMap.find( to_name );
    if ( iter != m_MessageRegMap.end() )
    {
        for ( int i = 0 ; i < ( int )iter->second.size() ; i++ )
        {
            iter->second[i]->MessageCallback( from_base, data );
        }
    }
}

/** @brief Send string message to all receivers, from undesignated sender.
 *
 * @param[in] msg     Message to send.
 */
void MessageMgr::SendAll( const string& msg )
{
    MessageData data;
    data.m_String = msg;
    SendAll( NULL, data );
}

/** @brief Send MessageData message to all receivers, from undesignated sender.
 *
 * @param[in] data    Message to send.
 */
void MessageMgr::SendAll( const MessageData& data )
{
    SendAll(  NULL, data );
}

/** @brief Send MessageData message to all receivers, from designated sender.
 *
 * @param[in] from_base Message sender.
 * @param[in] data      Message to send.
 */
void MessageMgr::SendAll( const MessageBase* from_base, const MessageData& data  )
{
    map< string, deque< MessageBase* > >::iterator iter;

    for ( iter = m_MessageRegMap.begin(); iter != m_MessageRegMap.end(); ++iter )
    {
        for ( int i = 0 ; i < ( int )iter->second.size() ; i++ )
        {
            iter->second[i]->MessageCallback( from_base, data );
        }
    }
}
