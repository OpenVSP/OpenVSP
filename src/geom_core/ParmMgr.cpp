//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
//// version 1.3 as detailed in the LICENSE file which accompanies this software.
//

// ParmMgr.h: interface for the Parm Mgr Singleton.
//
//////////////////////////////////////////////////////////////////////

#include "ParmMgr.h"
#include "VehicleMgr.h"
#include "UsingCpp11.h"
#include "APIDefines.h"
#include <assert.h>
#include <time.h>
#include <algorithm>

using std::map;
using std::string;
using std::unordered_map;


//==== Constructor ====//
ParmMgrSingleton::ParmMgrSingleton()
{
    m_NumParmChanges = 0;
    m_LastUndoFlag = false;
}

//==== Add Parm To Map ====//
bool ParmMgrSingleton::AddParm( Parm* p  )
{
    if ( !p )
    {
        return false;
    }

    string id = p->GetID();

    if ( id.size() == 0 )
    {
        return false;
    }

    //==== Check If Already Added ====//
    if ( m_ParmMap.find( p->GetID() ) != m_ParmMap.end() )
    {
        return false;
    }

    m_NumParmChanges++;
    m_ParmMap[id] = p;

    return true;
}

//==== Remove Parm From Map ====//
void ParmMgrSingleton::RemoveParm( Parm* p  )
{
    unordered_map< string, Parm* >::iterator iter;
    iter = m_ParmMap.find( p->GetID() );

    if ( iter !=  m_ParmMap.end() && iter->second == p )
    {
        m_NumParmChanges++;
        m_ParmMap.erase( iter );
    }
}

//==== Add Parm Container To Map ====//
void ParmMgrSingleton::AddParmContainer( ParmContainer* pc  )
{
    if ( pc )
    {
        m_NumParmChanges++;
        m_ParmContainerMap[pc->GetID()] = pc;
    }
}

//==== Remove Parm Container From Map ====//
void ParmMgrSingleton::RemoveParmContainer( ParmContainer* pc  )
{
    unordered_map< string, ParmContainer* >::iterator iter;
    iter = m_ParmContainerMap.find( pc->GetID() );

    if ( iter !=  m_ParmContainerMap.end() )
    {
        m_NumParmChanges++;
        m_ParmContainerMap.erase( iter );
    }
}

//==== Name Compare ====//
bool NameCompare( const string a, const string b )
{
    ParmContainer* pcA = ParmMgr.FindParmContainer( a );
    ParmContainer* pcB = ParmMgr.FindParmContainer( b );

    if ( pcA && pcB )
    {
        return ( pcA->GetName() < pcB->GetName() );
    }
    return ( false );
}


//==== Find Parm GivenID ====//
Parm* ParmMgrSingleton::FindParm( const string & id )
{
    unordered_map< string, Parm* >::iterator iter;

    iter = m_ParmMap.find( id );
    if ( iter != m_ParmMap.end() )
    {
        return iter->second;
    }
    return NULL;
}

//==== Find Parm Name Group Container ====//
string ParmMgrSingleton::FindParmID( const string & name, const string & group, const string & container )
{
    unordered_map< string, Parm* >::iterator iter;

    for ( iter = m_ParmMap.begin() ; iter != m_ParmMap.end() ; iter++ )
    {
        Parm* parm_ptr = iter->second;
        if ( parm_ptr )
        {
            if ( name == parm_ptr->GetName() && 
                 group == parm_ptr->GetGroupName() &&
                 container == parm_ptr->GetContainer()->GetName() )
            {
                return parm_ptr->GetID();
            }
        }
    }

    return string();
}


//==== Find Parm Container GivenID ====//
ParmContainer* ParmMgrSingleton::FindParmContainer( const string & id )
{
    unordered_map< string, ParmContainer* >::iterator iter;

    iter = m_ParmContainerMap.find( id );
    if ( iter != m_ParmContainerMap.end() )
    {
        return iter->second;
    }
    return NULL;
}


//==== Add Parm To Undo Stack ====//
void ParmMgrSingleton::AddToUndoStack( Parm* parm_ptr, bool drag_flag )
{
    ParmUndo undo( parm_ptr );

    if ( !drag_flag )
    {
        if ( m_LastUndoFlag )
            m_ParmUndoStack.push( m_LastUndo );
        m_LastUndo = undo;
        m_LastUndoFlag = true;
    }
}

//==== Add Parm To Undo Stack ====//
void ParmMgrSingleton::UnDo()
{
    if ( m_LastUndoFlag )
    {
        m_ParmUndoStack.push( m_LastUndo );
        m_LastUndoFlag = false;
    }

    if ( m_ParmUndoStack.size() == 0 )          // Nothing To Undo
    {
        return;
    }

    ParmUndo top = m_ParmUndoStack.top();       // Top Undo
    m_ParmUndoStack.pop();                      // Remove It

    Parm* parm_ptr = FindParm( top.GetID() );
    if ( parm_ptr )
    {
        parm_ptr->SetFromDevice( top.GetLastVal(), true );    // Set Last Val - Don't Add To Stack
    }
}

//==== Create A Unique ID  =====//
string ParmMgrSingleton::GenerateID( int length )
{
    static bool seed = false;
    if ( !seed )
    {
        seed = true;
        srand( ( unsigned int )time( NULL ) );
    }

    static char str[256];
    for ( int i = 0 ; i < length ; i++ )
    {
        str[i] = ( char )( ( rand() % 26 ) + 65 );
    }
    string id( str, length );

    return id;
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
string ParmMgrSingleton::RemapID( const string & oldID, const string & suggestID )
{
    string newID;

    // Check for special cases of non random ID's
    if( oldID.compare( "" ) == 0 || oldID.compare( "NONE" ) == 0 )
    {
        return oldID;
    }

    newID = m_IDRemap[oldID];

    if( newID.compare( "" ) == 0 )                          // oldID not yet in map
    {
        // Lookup ID as Parm and ParmConatiner
        Parm* p = FindParm( oldID );
        ParmContainer* pc = FindParmContainer( oldID );

        if( ( p == NULL ) && ( pc == NULL ) )               // No collision
        {
            newID = oldID;                                  //  reuse oldID.
        }
        else                                                // ID already used
        {
            if( suggestID.compare( "" ) != 0 )              //  suggestion provided
            {
                newID = suggestID;
            }
            else
            {
                newID = GenerateID( oldID.size() );    //  generate new.
            }
        }

        // Place newID in map.
        m_IDRemap[oldID] = newID;
    }

    return newID;
}

void ParmMgrSingleton::ResetRemapID()
{
    m_IDRemap.clear();
}

//==== Get Names For Parm Given ID ====//
void ParmMgrSingleton::GetNames( const string& parm_id, string& container_name,
                                 string& group_name, string& parm_name )
{
    Parm* p = FindParm( parm_id );
    if ( p )
    {
        parm_name = p->GetName();
        group_name = p->GetDisplayGroupName();

        ParmContainer* pc = p->GetLinkContainer();
        if ( pc )
        {
            container_name = pc->GetName();
        }
    }
}

//==== Create Parm and Add To Vector Of Parms ====//
Parm* ParmMgrSingleton::CreateParm( int type )
{
    Parm* p = NULL;
    if ( type == PARM_DOUBLE_TYPE )
    {
        p = new Parm();
    }
    else if ( type == PARM_INT_TYPE )
    {
        p = new IntParm();
    }
    else if ( type == PARM_BOOL_TYPE )
    {
        p = new BoolParm();
    }
    else if ( type == PARM_FRACTION_TYPE )
    {
        p = new FractionParm();
    }
    return p;
}
