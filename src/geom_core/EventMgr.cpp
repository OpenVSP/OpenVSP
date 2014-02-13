//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//
#include <assert.h>

#include "EventMgr.h"
#include "Vehicle.h"


//==== Constructor ====//
EventMgr::EventMgr()
{
    m_Vehicle = NULL;
}

void EventMgr::Init( Vehicle* vPtr )
{
    m_Vehicle = vPtr;
}

void EventMgr::GuiParmEvent( int geom_id, const string& parm_name, const string& group_name )
{
    assert( m_Vehicle );

    Parm* pPtr = m_Vehicle->GetParm( geom_id, parm_name, group_name );


}


