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
    m_LastUndoPushed = false;
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

    AttributeMgr.DeregisterCollID( pc->GetAttrCollection()->GetID() );

    // Only erase the map entry if it refers to this container -- matches the
    // identity check in RemoveParm so a stale container with a duplicated ID can
    // not remove the map entry of the live one.
    if ( iter !=  m_ParmContainerMap.end() && iter->second == pc )
    {
        m_NumParmChanges++;
        m_ParmContainerMap.erase( iter );
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

//==== Every Live Container ====//
void ParmMgrSingleton::GetAllParmContainers( vector< ParmContainer* > & container_vec ) const
{
    container_vec.reserve( container_vec.size() + m_ParmContainerMap.size() );

    unordered_map< string, ParmContainer* >::const_iterator iter;
    for ( iter = m_ParmContainerMap.begin(); iter != m_ParmContainerMap.end(); ++iter )
    {
        container_vec.push_back( iter->second );
    }
}

//==== Move A Container From One Parent's Children To Another's ====//
void ParmMgrSingleton::ParentContainerChanged( ParmContainer* pc, const string & old_parent_id,
                                               const string & new_parent_id )
{
    if ( !pc || old_parent_id == new_parent_id )
    {
        return;
    }

    if ( !old_parent_id.empty() )
    {
        unordered_map< string, vector< ParmContainer* > >::iterator iter = m_ChildContainerMap.find( old_parent_id );
        if ( iter != m_ChildContainerMap.end() )
        {
            // From the back, where the newest is, keeping the order; once, as it is there once.
            vector< ParmContainer* > & children = iter->second;
            for ( int i = ( int )children.size() - 1; i >= 0; i-- )
            {
                if ( children[i] == pc )
                {
                    children.erase( children.begin() + i );
                    break;
                }
            }
            if ( children.empty() )
            {
                m_ChildContainerMap.erase( iter );
            }
        }
    }

    if ( !new_parent_id.empty() )
    {
        m_ChildContainerMap[ new_parent_id ].push_back( pc );
    }

    // What is listed under the containers above both parents has changed.
    m_NumParmChanges++;
}

const vector< ParmContainer* > & ParmMgrSingleton::GetChildContainers( const string & id ) const
{
    static const vector< ParmContainer* > none;

    unordered_map< string, vector< ParmContainer* > >::const_iterator iter = m_ChildContainerMap.find( id );
    if ( iter == m_ChildContainerMap.end() )
    {
        return none;
    }
    return iter->second;
}

bool ParmMgrSingleton::ValidateChildContainerMap()
{
    bool pass = true;

    // Every live container with a parent is among that parent's children, once.
    unordered_map< string, ParmContainer* >::iterator citer;
    for ( citer = m_ParmContainerMap.begin(); citer != m_ParmContainerMap.end(); ++citer )
    {
        ParmContainer* pc = citer->second;
        if ( !pc || pc->GetParentContainer().empty() )
        {
            continue;
        }

        const vector< ParmContainer* > & children = GetChildContainers( pc->GetParentContainer() );
        int count = 0;
        for ( int i = 0; i < ( int )children.size(); i++ )
        {
            if ( children[i] == pc )
            {
                count++;
            }
        }
        if ( count != 1 )
        {
            printf( "Container %s is %d times a child of its parent %s\n", pc->GetID().c_str(), count,
                    pc->GetParentContainer().c_str() );
            pass = false;
        }
    }

    // Every child kept is live and names that parent.
    unordered_map< string, vector< ParmContainer* > >::iterator iter;
    for ( iter = m_ChildContainerMap.begin(); iter != m_ChildContainerMap.end(); ++iter )
    {
        if ( iter->second.empty() )
        {
            printf( "Parent %s is kept with no children\n", iter->first.c_str() );
            pass = false;
        }
        for ( int i = 0; i < ( int )iter->second.size(); i++ )
        {
            ParmContainer* pc = iter->second[i];
            if ( FindParmContainer( pc->GetID() ) != pc )
            {
                printf( "A child of %s is not a live container\n", iter->first.c_str() );
                pass = false;
            }
            else if ( pc->GetParentContainer() != iter->first )
            {
                printf( "Container %s is a child of %s but names %s\n", pc->GetID().c_str(), iter->first.c_str(),
                        pc->GetParentContainer().c_str() );
                pass = false;
            }
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
        m_LastUndoPushed = m_LastUndoFlag;
        if ( m_LastUndoFlag )
            m_ParmUndoStack.push( m_LastUndo );
        m_LastUndo = undo;
        m_LastUndoFlag = true;
    }
}

//==== Take Back The Last Undo Entry ====//
void ParmMgrSingleton::DropLastUndo()
{
    if ( !m_LastUndoFlag )
    {
        return;
    }

    if ( m_LastUndoPushed && !m_ParmUndoStack.empty() )
    {
        m_LastUndo = m_ParmUndoStack.top();
        m_ParmUndoStack.pop();
    }
    else
    {
        m_LastUndoFlag = false;
    }
    m_LastUndoPushed = false;
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

        ParmContainer* pc = p->GetTopContainer();
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

        ParmContainer* pc = p->GetTopContainer();
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
