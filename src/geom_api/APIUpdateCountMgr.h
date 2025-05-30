//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//

//
// Rob McDonald
//
//////////////////////////////////////////////////////////////////////

#if !defined(APIUPDATECOUNTMGR__INCLUDED_)
#define APIUPDATECOUNTMGR__INCLUDED_

#include "APIDefines.h"
#include "MessageMgr.h"

#include <string>

using std::string;

namespace vsp
{

//======================== Error Mgr ================================//
class UpdateCountMgrSingleton : MessageBase
{
public:

    unsigned long GetUpdateCount();
    void ResetUpdateCount();
    unsigned long GetAndResetUpdateCount();

    virtual void MessageCallback( const MessageBase* from, const MessageData& data );

    static UpdateCountMgrSingleton& getInstance()
    {
        static UpdateCountMgrSingleton instance;
        return instance;
    }

private:

    unsigned long m_UpdateCount;

    UpdateCountMgrSingleton();
    ~UpdateCountMgrSingleton();
    UpdateCountMgrSingleton( UpdateCountMgrSingleton const& copy ) = delete;          // Not Implemented
    UpdateCountMgrSingleton& operator=( UpdateCountMgrSingleton const& copy ) = delete; // Not Implemented
};

#define UpdateCountMgr UpdateCountMgrSingleton::getInstance()

}

#endif // !defined(APIUPDATECOUNTMGR__INCLUDED_)

