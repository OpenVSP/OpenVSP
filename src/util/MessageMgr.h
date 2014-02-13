//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//
//
//////////////////////////////////////////////////////////////////////

/** @file MessageMgr.h
 * @brief Subscription based message passing capability.
 *
 * VSP uses a subscription based message passing capability to enable communication
 * between modules while not requiring any compile-time dependencies.  Specifically,
 * we require a full capability geometry engine with no dependency on GUI or OpenGL
 * libraries.  This will allow VSP to be used as an API by other programs and also
 * allow batch mode operation on computer systems where graphics libraries are not
 * welcome.
 *
 * The message passing capability consists of three classes.
 *  - MessageData The messages themselves.
 *  - MessageBase Base class for any message listener.
 *  - MessageMgr  Message manager for listener registration and message passing.
 *
 * The compile-time dependency prohibition actually only disallows the geometry core
 * from depending on any GUI or OpenGL code -- the GUI and OpenGL code can depend on
 * the geometry core.  We take advantage of this asymmetry to greatly simplify the
 * message passing capability.  MessageData are expected to be relatively simple
 * messages used to trigger events on the listener side of things.  On the other hand,
 * the listener side can have full access to the geometry core API to send or receive
 * complex data or commands.
 *
 */

#if !defined(MESSAGE_MGR__INCLUDED_)
#define MESSAGE_MGR__INCLUDED_

#include <string>
#include <vector>
#include <deque>
#include <map>

using std::string;
using std::vector;

/** @class MessageData
 * @brief Message class.
 *
 * MessageData objects are passed via MessageMgr to classes derived from MessageBase
 * and registered with MessageMgr.
 */
class MessageData
{
public:
    MessageData();
    string m_String;
    vector< int > m_IntVec;
    vector< double > m_DoubleVec;
    vector< string > m_StringVec;
    void* m_Ptr;
};


/** @class MessageBase
 * @brief Message listener base class.
 *
 * Classes derived from MessageBase can be registered with MessageMgr and will
 * receive MessageData messages.
 */
class MessageBase
{
public:
    MessageBase();
    virtual ~MessageBase();

    /** @brief Set message listener name.
     *
     * @param[in] name   Message listener name.
     */
    virtual void SetName( const string & name )
    {
        m_Name = name;
    }

    /** @brief Get message listener name.
     */
    virtual string GetName()
    {
        return m_Name;
    }

    virtual void Register();
    virtual void Register( const string & name );
    virtual void UnRegister();

    /** @brief Callback function executed when message received.
     *
     * @param[in] from   MessageBase that sent message.
     * @param[in] data   Message data.
     */
    virtual void MessageCallback( const MessageBase* from, const MessageData& data ) = 0;

protected:
    string m_Name;
};

/** @class MessageMgr
 * @brief Message manager handles listener registry and message broadcasting.
 *
 * MessageMgr maintains the registration of MessageBase listeners and broadcasts
 * MessageData messages to registered listeners.
 */
class MessageMgr
{
private:
    MessageMgr();
    MessageMgr( MessageMgr const& copy );          // Not Implemented
    MessageMgr& operator=( MessageMgr const& copy ); // Not Implemented

    std::map< string, std::deque< MessageBase* > > m_MessageRegMap;

public:
    /** @brief Get common instance of MessageMgr.
     */
    static MessageMgr& getInstance()
    {
        static MessageMgr instance;
        return instance;
    }

    void Register( MessageBase* msg_base );
    void UnRegister( MessageBase* msg_base );

    void Send( const string& to_name, const string& msg );
    void Send( const string& to_name, const MessageData& data  );
    void Send( const string& to_name, const MessageBase* from_base, const MessageData& data  );

    void SendAll( const string& msg );
    void SendAll( const MessageData& data  );
    void SendAll( const MessageBase* from_base, const MessageData& data );

};



#endif // !defined(MESSAGE_MGR__INCLUDED_)
