//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//
#include <cassert>

#include "MessageMgr.h"
using std::map;
using std::string;
using std::deque;

//==== Message Data ====//
MessageData::MessageData()
{
}

//==== Message Base ====//
MessageBase::MessageBase()
{
    m_Name = "DefaultName";
}

//==== Message Base ====//
MessageBase::~MessageBase()
{
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
    m_MessageRegMap.erase( msg_base->GetName() );
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
    Send( to_name, nullptr, data );
}

/** @brief Send MessageData message to designated receiver, from undesignated sender.
 *
 * @param[in] to_name Targeted message receiver.
 * @param[in] data    Message to send.
 */
void MessageMgr::Send( const string& to_name, const MessageData& data )
{
    Send( to_name, nullptr, data );
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
    SendAll( nullptr, data );
}

/** @brief Send MessageData message to all receivers, from undesignated sender.
 *
 * @param[in] data    Message to send.
 */
void MessageMgr::SendAll( const MessageData& data )
{
    SendAll(  nullptr, data );
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
