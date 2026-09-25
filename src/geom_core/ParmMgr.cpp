//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
//// version 1.3 as detailed in the LICENSE file which accompanies this software.
//

// ParmMgr.h: interface for the Parm Mgr Singleton.
//
//////////////////////////////////////////////////////////////////////

#include "ParmMgr.h"
#include "VspUtil.h"
#include "IDMgr.h"
#include "ResultsMgr.h"

using std::map;
using std::string;
using std::unordered_map;


//==== Constructor ====//
ParmMgrSingleton::ParmMgrSingleton()
{
    m_NumParmChanges = 0;
    m_ChangeCnt = 0;
    m_LastUndoFlag = false;
    m_LastReset = "";
    m_DirtyFlag = true;
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
    AttributeMgr.RegisterCollID( m_ParmMap[id]->GetAttrCollection()->GetID(), m_ParmMap[id]->GetAttrCollection() );

    m_DirtyFlag = true;

    return true;
}

//==== Remove Parm From Map ====//
void ParmMgrSingleton::RemoveParm( Parm* p  )
{
    unordered_map< string, Parm* >::iterator iter;
    iter = m_ParmMap.find( p->GetID() );

    AttributeMgr.DeregisterCollID( p->GetAttrCollection()->GetID() );

    if ( iter !=  m_ParmMap.end() && iter->second == p )
    {
        m_NumParmChanges++;
        m_ParmMap.erase( iter );
    }

    m_DirtyFlag = true;
}

//==== Add Parm Container To Map ====//
void ParmMgrSingleton::AddParmContainer( ParmContainer* pc  )
{
    if ( pc )
    {
        m_NumParmChanges++;
        m_ParmContainerMap[pc->GetID()] = pc;
        AttributeMgr.RegisterCollID( pc->GetAttrCollection()->GetID(), pc->GetAttrCollection() );
    }

    m_DirtyFlag = true;
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
        AttributeMgr.DeregisterCollID( pc->GetAttrCollection()->GetID() );
    }

    m_DirtyFlag = true;
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
    return nullptr;
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
    return nullptr;
}

bool ParmMgrSingleton::ValidateParmMap()
{
    bool pass = true;
    unordered_map< string, Parm* >::iterator iter;
    for ( iter = m_ParmMap.begin(); iter != m_ParmMap.end(); ++iter )
    {
        if ( iter->second )
        {
            try
            {
                if ( iter->first == iter->second->GetID() )
                {
                    // All good
                }
                else
                {
                    printf( "ID compare failed %s\n", iter->first.c_str() );
                    pass = false;
                }
            }
            catch( int e )
            {
                printf( "Exception caught %s\n", iter->first.c_str() );
                pass = false;
            }
        }
        else
        {
            printf( "Pointer failed %s\n", iter->first.c_str() );
            pass = false;
        }
    }

    return pass;
}

bool ParmMgrSingleton::ValidateParmContainerMap()
{
    bool pass = true;
    unordered_map< string, ParmContainer* >::iterator iter;
    for ( iter = m_ParmContainerMap.begin(); iter != m_ParmContainerMap.end(); ++iter )
    {
        if ( iter->second )
        {
            try
            {
                if ( iter->first == iter->second->GetID() )
                {
                    // All good
                }
                else
                {
                    printf( "ID compare failed %s\n", iter->first.c_str() );
                    pass = false;
                }
            }
            catch( int e )
            {
                printf( "Exception caught %s\n", iter->first.c_str() );
                pass = false;
            }
        }
        else
        {
            printf( "Pointer failed %s\n", iter->first.c_str() );
            pass = false;
        }
    }

    return pass;
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
void ParmMgrSingleton::ClearUndo()
{
    while ( !m_ParmUndoStack.empty() )
    {
        m_ParmUndoStack.pop();
    }
    m_LastUndoFlag = false;
}

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

// Whatever the user attached to a Parm belongs to the ID, not to the object holding it.  An
// attribute names its Parm by ID, so a note on "the leading edge radius" has to end up on
// whichever Parm answers to that ID once the swap is done.  Each Parm owns its collection as a
// member and ChangeID only re-points the collection's attach, so without this the attributes
// stay with the object that a caller of SwapIDs is about to destroy -- a cross section whose
// shape was changed, say -- and go with it, silently.
static void SwapParmAttributes( Parm* parm_a, Parm* parm_b )
{
    AttributeCollection* coll_a = parm_a->GetAttrCollection();
    AttributeCollection* coll_b = parm_b->GetAttrCollection();

    if ( !coll_a || !coll_b )
    {
        return;
    }

    vector< NameValData* > was_on_a = coll_a->GetAllPtrs();
    vector< NameValData* > was_on_b = coll_b->GetAllPtrs();

    // Taken out of both before either is added to, so a name held on both sides cannot collide
    // part way through.  Remove unlinks and deregisters; the attribute object itself survives.
    for ( int i = 0 ; i < ( int )was_on_a.size() ; i++ )
    {
        coll_a->Remove( was_on_a[i] );
    }
    for ( int i = 0 ; i < ( int )was_on_b.size() ; i++ )
    {
        coll_b->Remove( was_on_b[i] );
    }

    // Crossed over, because the IDs have been: Add re-attaches each attribute to the collection
    // taking it and registers it again, so it now names the Parm that holds its original ID.
    for ( int i = 0 ; i < ( int )was_on_a.size() ; i++ )
    {
        coll_b->Add( was_on_a[i] );
    }
    for ( int i = 0 ; i < ( int )was_on_b.size() ; i++ )
    {
        coll_a->Add( was_on_b[i] );
    }

    // And the collections' own IDs, so a collection ID names the same attributes afterwards.
    coll_a->SwapID( coll_b );
}

void ParmMgrSingleton::SwapIDs( const string &aID, const string &bID )
{
    if ( aID == bID )
    {
        return;
    }

    Parm* parm_a = FindParm( aID );
    Parm* parm_b = FindParm( bID );

    if ( parm_a && parm_b )
    {
        parm_b->ChangeID( "TEMP" );
        parm_a->ChangeID( bID );
        parm_b->ChangeID( aID );

        SwapParmAttributes( parm_a, parm_b );

        // Each container finds its Parms by group and name through a map of IDs.
        if ( parm_a->GetContainer() )
        {
            parm_a->GetContainer()->UpdateGroupParmMap();
        }
        if ( parm_b->GetContainer() && parm_b->GetContainer() != parm_a->GetContainer() )
        {
            parm_b->GetContainer()->UpdateGroupParmMap();
        }
        IncNumParmChanges();
    }
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

//Overload for getting group alias
void ParmMgrSingleton::GetNames( const string& parm_id, string& container_name,
    string& group_name, string& group_alias, string& parm_name )
{
    Parm* p = FindParm( parm_id );
    if ( p )
    {
        // As this overload accesses the group alias too, we ensure it's not part of the group_name variable
        bool include_alias = false;
        parm_name = p->GetName();
        group_name = p->GetDisplayGroupName( include_alias );
        group_alias = p->GetGroupAlias();

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
    Parm* p = nullptr;
    if ( type == vsp::PARM_DOUBLE_TYPE )
    {
        p = new Parm();
    }
    else if ( type == vsp::PARM_INT_TYPE )
    {
        p = new IntParm();
    }
    else if ( type == vsp::PARM_BOOL_TYPE )
    {
        p = new BoolParm();
    }
    else if ( type == vsp::PARM_FRACTION_TYPE )
    {
        p = new FractionParm();
    }
    else if ( type == vsp::PARM_LIMITED_INT_TYPE )
    {
        p = new LimIntParm();
    }
    else if ( type == vsp::PARM_NOTEQ_TYPE )
    {
        p = new NotEqParm();
    }
    else if ( type == vsp::PARM_POWER_INT_TYPE )
    {
        p = new PowIntParm();
    }
    return p;
}
