//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//
#include <algorithm>
#include <cassert>

#include "MessageMgr.h"

using std::unordered_map;
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
    // A listener that is destroyed while still registered leaves a dangling
    // pointer in the registry, and the next SendAll() makes a virtual call
    // through it.  Unregistering here means a listener cannot outlive its
    // registration no matter how it is destroyed.
    //
    // MessageMgr is safe to touch from here.  Every listener registers from its
    // own constructor, so the MessageMgr instance finishes construction inside
    // that of the first listener and is therefore destroyed after all of them.
    MessageMgr::getInstance().UnRegister( this );
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
 * Removes only the listener asked for.  Other listeners that happen to share
 * its name stay registered.  Every bucket is searched rather than only the one
 * matching the current name, so a listener that was renamed after it
 * registered is still removed.  Buckets left empty are dropped so that Send()
 * does not have to consider them.
 *
 * @param[in] msg_base Listener to unregister.
 */
void MessageMgr::UnRegister( MessageBase* msg_base )
{
    unordered_map< string, deque< MessageBase* > >::iterator iter = m_MessageRegMap.begin();

    while ( iter != m_MessageRegMap.end() )
    {
        deque< MessageBase* > & listeners = iter->second;

        listeners.erase( std::remove( listeners.begin(), listeners.end(), msg_base ), listeners.end() );

        if ( listeners.empty() )
        {
            iter = m_MessageRegMap.erase( iter );
        }
        else
        {
            ++iter;
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
    unordered_map< string, deque< MessageBase* > >::iterator iter;

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
    unordered_map< string, deque< MessageBase* > >::iterator iter;

    for ( iter = m_MessageRegMap.begin(); iter != m_MessageRegMap.end(); ++iter )
    {
        for ( int i = 0 ; i < ( int )iter->second.size() ; i++ )
        {
            iter->second[i]->MessageCallback( from_base, data );
        }
    }
}

void MessageMgr::SendError( const char* msg, const char* func, const char* file, int line, string stackdump )
{
    MessageData errMsgData;
    errMsgData.m_String = "Error";
    errMsgData.m_IntVec.push_back( -1 ); // vsp::VSP_UNKNOWN
    string message = string( msg ) + "\nIn: " + func + "\nOn Line: " + std::to_string( line ) + "\nIn File: " + file + "\n\n" + stackdump.c_str();
    errMsgData.m_StringVec.push_back( message );

    SendAll( errMsgData );
}
