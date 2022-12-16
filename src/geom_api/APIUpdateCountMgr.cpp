//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//

//
// Rob McDonald
//
//////////////////////////////////////////////////////////////////////

#include "APIUpdateCountMgr.h"

using namespace vsp;

//===================================================================//
//==================== Update Count Mgr =============================//
//===================================================================//

UpdateCountMgrSingleton::UpdateCountMgrSingleton()
{
    m_UpdateCount = 0;
    MessageBase::Register( string( "UpdateCountMgr" ) );
}

UpdateCountMgrSingleton::~UpdateCountMgrSingleton()
{
    MessageMgr::getInstance().UnRegister( this );
}

unsigned long UpdateCountMgrSingleton::GetUpdateCount()
{
    return m_UpdateCount;
}

void UpdateCountMgrSingleton::ResetUpdateCount()
{
    m_UpdateCount = 0;
}

unsigned long UpdateCountMgrSingleton::GetAndResetUpdateCount()
{
    unsigned long tmp = m_UpdateCount;
    m_UpdateCount = 0;
    return tmp;
}

//==== Message Callbacks ====//
void UpdateCountMgrSingleton::MessageCallback( const MessageBase* from, const MessageData& data )
{
    if ( data.m_String == string( "UpdateAllScreens" ) )
    {
        m_UpdateCount++;
    }
}
