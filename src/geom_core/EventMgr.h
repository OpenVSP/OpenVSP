//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//
//
//////////////////////////////////////////////////////////////////////

#if !defined(EVENT_MGR__INCLUDED_)
#define EVENT_MGR__INCLUDED_

#include <string>
using namespace std;

class Vehicle;

//==== Event Mgr ====//
class EventMgr
{
private:
    EventMgr();
    EventMgr( EventMgr const& copy );          // Not Implemented
    EventMgr& operator=( EventMgr const& copy ); // Not Implemented

    Vehicle* m_Vehicle;


public:
    static EventMgr& getInstance()
    {
        static EventMgr instance;
        return instance;
    }
    void Init( Vehicle* vPtr );

    void GuiParmEvent( int geom_id, const string& parm_name, const string& group_name );
};

#endif // !defined(EVENT_MGR__INCLUDED_)
