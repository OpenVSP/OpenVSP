//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
//// version 1.3 as detailed in the LICENSE file which accompanies this software.
//

// ParmMgr.h: interface for the Parm Mgr Singleton.
//
//////////////////////////////////////////////////////////////////////

#include "AttributeManager.h"
#include "IDMgr.h"
#include "ParmMgr.h"
#include "VspUtil.h"
#include "VarPresetMgr.h"
#include "VspUtil.h"

using std::map;
using std::string;
using std::unordered_map;


//==== Constructor ====//
IDMgrSingleton::IDMgrSingleton()
{

}

//==== Remap oldID into newID avoiding collisions ====//

// RemapID will map an old set of ID's to a new set of ID's.
//
// When an oldID is first passed to RemapID, a newID is chosen which will be returned
// every time oldID is passed (until a reset).
//
// If oldID is not used by VSP (i.e. there are no collisions), then newID will match
// oldID.  This allows seamless persistence of ID's.
//
// If oldID is already used by VSP (i.e. there are collisions), then newID will either
// be set an optional suggestedID or randomly selected.
//
// Remapping is done in context of the last time ResetRemapID was called.  I.e. collisions
// are when an ID already exists when Reset is called.
//

//==== Bool Checker for Non-random IDs ====//
bool IDMgrSingleton::NonRandomID( const string &id )
{
    // Check for special cases of non random ID's
    if( id.compare( "" ) == 0 ||
        id.compare( "NONE" ) == 0 ||
        id.substr( 0, 5 ) == "User_" ||         // User parameters.
        id.substr( 0, 1 ) == "_" ||             // Built-in FEA materials.
        id.substr( 0, 5 ) == "_Attr" )           // Fixed Attr IDs
    {
        return true;
    }
    return false;
}

string IDMgrSingleton::RemapID( const string & oldID, const string & suggestID )
{
    return RemapID( oldID, suggestID, -1 );
}

// ForceRemapID works as above, but makes no attempt to
string IDMgrSingleton::ForceRemapID( const string & oldID, int size )
{
    string dummy;
    return RemapID( oldID, dummy, size );
}

string IDMgrSingleton::RemapID( const string & oldID, const string & suggestID, int size )
{
    string newID;

    if ( NonRandomID( oldID ) )
    {
        return oldID;
    }

    newID = m_IDRemap[oldID];

    if( newID.compare( "" ) == 0 )                          // oldID not yet in map
    {
        // Lookup ID as Parm and ParmConatiner
        Parm* p = ParmMgr.FindParm( oldID );
        ParmContainer* pc = ParmMgr.FindParmContainer( oldID );
        NameValData* a = AttributeMgr.GetAttributePtr( oldID );
        AttributeCollection* ac = AttributeMgr.GetCollectionPtr( oldID );
        Setting *s = VarPresetMgr.FindSetting( oldID );
        SettingGroup *sg = VarPresetMgr.FindSettingGroup( oldID );

        if( ( p == NULL ) &&
            ( pc == NULL ) &&
            ( a == NULL ) &&
            ( ac == NULL ) &&
            ( s == NULL ) &&
            ( sg == NULL ) &&
            size == -1 )
        {
            newID = oldID;                                  //  reuse oldID.
        }
        else                                                // ID already used
        {
            if ( size != -1 )
            {
                newID = GenerateRandomID( size );                 //  generate new.
            }
            else if ( suggestID.compare( "" ) != 0 )        //  suggestion provided
            {
                newID = suggestID;
            }
            else
            {
                newID = GenerateRandomID( oldID.size() );    //  generate new.
            }
        }

        // Place newID in map.
        m_IDRemap[oldID] = newID;
    }

    return newID;
}

string IDMgrSingleton::ResetRemapID( const string & lastReset )
{
    if ( lastReset != "" && lastReset != m_LastReset )
    {
        MessageData errMsgData;
        errMsgData.m_String = "Error";
        errMsgData.m_IntVec.push_back( vsp::VSP_UNEXPECTED_RESET_REMAP_ID );
        errMsgData.m_StringVec.push_back( "Error:  Unexpected intermediate ResetRemapID.  This should be harmless, but please contact rob.a.mcdonald@gmail.com to help debug this issue." );

        MessageMgr::getInstance().SendAll( errMsgData );
    }

    m_IDRemap.clear();

    m_LastReset = GenerateRandomID( 3 );
    return m_LastReset;
}
