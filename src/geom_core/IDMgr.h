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

#include "MessageMgr.h"
#include "XmlUtil.h"

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

    void RegisterIDs( xmlNodePtr node );
    void RegisterOneID( xmlNodePtr node );

public:
    static IDMgrSingleton& getInstance()
    {
        static IDMgrSingleton instance;
        return instance;
    }

    bool NonRandomID( const string &id );

    // Three rules, one for each thing an ID read out of a file can be.  All three answer in
    // the context of the last ResetRemapID.

    // The ID that names this object.  Keeps the ID read in, or issues a fresh one if that ID is
    // already taken, and remembers the pairing so every later mention of the old ID answers the
    // same.  Called from the DecodeXml of the six families that own an ID: ParmContainer, Parm,
    // NameValData, AttributeCollection, Setting and SettingGroup.
    string RemapID( const string & oldID, const string & suggestID, int size );
    string RemapID( const string & oldID, const string & suggestID = "" );

    // An ID that names some other object.  Follows that object if it came along with this read
    // or copy, and otherwise returns the ID unchanged, since it then already names what is
    // meant.  Never issues a new ID, so the identities it has to follow must reach the map
    // first -- see PreRegisterIDs.
    string RemapRefID( const string & oldID );

    // An ID that names some other object and only means anything within this operation: a
    // Geom's parent and children.  Follows that object if it came along, and otherwise returns
    // empty, since the link has nothing left to name.  Never issues a new ID.
    string RemapCopiedID( const string & oldID );

    // Registers every identity in a tree, so that a reference into that tree resolves whichever
    // order the file lists the two.
    //
    // For a tree about to be read into newly made objects, which is what the Geoms of a file
    // are.  Registering is a call to RemapID with no suggestion, so an identity that collides
    // takes a fresh ID.  An object that reads its own ID back instead, such as the Vehicle or a
    // settings container, keeps the ID it has and is not registered here.
    void PreRegisterIDs( xmlNodePtr node );

    // Always issues a new ID, of the length asked for, keeping nothing of the one read in.  A
    // V2 file wrote memory addresses where a v3 file writes IDs, so each is reissued on the way
    // in.
    string ForceRemapID( const string & oldID, int size );

    // Opens a remapping context and returns a token naming it.  Pass the token back when the
    // operation ends; a token that does not match reports a context reset inside another one.
    string ResetRemapID( const string & lastReset = "" );
};

#define IDMgr IDMgrSingleton::getInstance()

#endif // !defined(IDMGR__INCLUDED_)
