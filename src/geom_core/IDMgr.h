//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//

// IDMgr.h: interface for the ID Mgr Singleton.
// J.R Gloudemans
//
//////////////////////////////////////////////////////////////////////

#if !defined(IDMGR__INCLUDED_)
#define IDMGR__INCLUDED_

#include "UsingCpp11.h"

#include "MessageMgr.h"

#include <map>
#include <unordered_map>
#include <stack>

using std::string;
using std::unordered_map;
using std::unordered_multimap;

//==== ID Manager ====//
class IDMgrSingleton
{
private:
    IDMgrSingleton();

    unordered_map< string, string > m_IDRemap;                      // oldID->newID Map
    string m_LastReset;

public:
    static IDMgrSingleton& getInstance()
    {
        static IDMgrSingleton instance;
        return instance;
    }

    bool NonRandomID( const string &id );

    string RemapID( const string & oldID, const string & suggestID, int size );
    string RemapID( const string & oldID, const string & suggestID = "" );
    string ForceRemapID( const string & oldID, int size );
    string ResetRemapID( const string & lastReset = "" );
};

#define IDMgr IDMgrSingleton::getInstance()

#endif // !defined(IDMGR__INCLUDED_)
