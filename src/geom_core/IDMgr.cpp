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

        if( ( p == nullptr ) &&
            ( pc == nullptr ) &&
            ( a == nullptr ) &&
            ( ac == nullptr ) &&
            ( s == nullptr ) &&
            ( sg == nullptr ) &&
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

string IDMgrSingleton::RemapRefID( const string & oldID )
{
    if ( NonRandomID( oldID ) )
    {
        return oldID;
    }

    unordered_map< string, string >::const_iterator it = m_IDRemap.find( oldID );
    if ( it != m_IDRemap.end() )
    {
        return it->second;                  // the thing referred to came along; follow it
    }

    return oldID;                           // it did not, so this is already who is meant
}

string IDMgrSingleton::RemapCopiedID( const string & oldID )
{
    if ( NonRandomID( oldID ) )
    {
        return oldID;
    }

    unordered_map< string, string >::const_iterator it = m_IDRemap.find( oldID );
    if ( it != m_IDRemap.end() )
    {
        return it->second;
    }

    return string();
}

void IDMgrSingleton::PreRegisterIDs( xmlNodePtr node )
{
    if ( !node )
    {
        return;
    }

    RegisterIDs( node );
}

// An identity is written in one of two shapes: a container writes its ID as a child node, and
// an element that is itself the object carries an ID property, which is how a Parm and an
// attribute write theirs.  A reference is written under a name of its own, so neither shape
// picks one up.
void IDMgrSingleton::RegisterOneID( xmlNodePtr node )
{
    if ( !node || node->type != XML_ELEMENT_NODE )
    {
        return;
    }

    if ( xmlStrcmp( node->name, ( const xmlChar* )"ParmContainer" ) == 0 ||
         xmlStrcmp( node->name, ( const xmlChar* )"Setting" ) == 0 ||
         xmlStrcmp( node->name, ( const xmlChar* )"SettingGroup" ) == 0 )
    {
        string id = XmlUtil::FindString( node, "ID", string() );
        if ( !id.empty() && !NonRandomID( id ) )
        {
            RemapID( id );
        }
    }

    string prop = XmlUtil::FindStringProp( node, "ID", string() );
    if ( !prop.empty() && !NonRandomID( prop ) )
    {
        RemapID( prop );
    }
}

void IDMgrSingleton::RegisterIDs( xmlNodePtr node )
{
    // The node handed in is registered along with its children, so a caller may pass an
    // object's own element and not only a document root.
    RegisterOneID( node );

    for ( xmlNodePtr child = node->children; child; child = child->next )
    {
        if ( child->type == XML_ELEMENT_NODE )
        {
            RegisterIDs( child );
        }
    }
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
