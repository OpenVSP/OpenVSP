//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//

// ParmMgr.h: interface for the Parm Mgr Singleton.
// J.R Gloudemans
//
//////////////////////////////////////////////////////////////////////

#if !defined(PARMMGR__INCLUDED_)
#define PARMMGR__INCLUDED_

#include "Parm.h"
#include "ParmUndo.h"
#include "MessageMgr.h"

#include <map>
#include <unordered_map>
#include <stack>

using std::string;
using std::unordered_map;
using std::unordered_multimap;

//==== Parm Manager ====//
class ParmMgrSingleton
{
private:
    ParmMgrSingleton();
    ParmMgrSingleton( ParmMgrSingleton const& copy ) = delete;          // Not Implemented
    ParmMgrSingleton& operator=( ParmMgrSingleton const& copy ) = delete; // Not Implemented

    bool m_LastUndoFlag;
    bool m_LastUndoPushed;      // Making m_LastUndo pushed the one before it onto the stack
    ParmUndo m_LastUndo;
    std::stack< ParmUndo > m_ParmUndoStack;

    string m_ActiveParmID;

    unordered_map< string, Parm* > m_ParmMap;                       // ID->Parm Map
    unordered_map< string, ParmContainer* > m_ParmContainerMap;     // ID->Parm Container Map

    // The containers that name each ID as their parent, in the order they took it: the inverse of
    // every live container's parent container.
    unordered_map< string, vector< ParmContainer* > > m_ChildContainerMap;

    unordered_map< string, string > m_IDRemap;                      // oldID->newID Map
    string m_LastReset;

    int m_NumParmChanges;
    int m_ChangeCnt;

    bool m_DirtyFlag;

public:
    static ParmMgrSingleton& getInstance()
    {
        static ParmMgrSingleton instance;
        return instance;
    }

    bool AddParm( Parm* parm_ptr );
    void RemoveParm( Parm* parm_ptr );
    void AddParmContainer( ParmContainer* parm_container_ptr );
    void RemoveParmContainer( ParmContainer* parm_container_ptr );

    Parm* FindParm( const string & id );
    ParmContainer* FindParmContainer( const string & id );

    bool ValidateParmMap();
    bool ValidateParmContainerMap();

    // Kept by ParmContainer: a container moves from the children of one parent ID to another's.
    // An empty ID is no parent.
    void ParentContainerChanged( ParmContainer* parm_container_ptr, const string & old_parent_id,
                                 const string & new_parent_id );

    // The containers that name this ID as their parent, whether or not a container holds the ID.
    const vector< ParmContainer* > & GetChildContainers( const string & id ) const;

    // Whether the children kept for each ID are exactly the live containers naming it as parent.
    bool ValidateChildContainerMap();

    void AddToUndoStack( Parm* parm_ptr, bool drag_flag );
    // Takes back the entry AddToUndoStack made last, as if that change had not been made.
    void DropLastUndo();
    void UnDo();
    void ClearUndo();

    void SwapIDs( const string &aID, const string &bID );

    void SetActiveParm( const string &id )         { m_ActiveParmID = id; }
    Parm* GetActiveParm()                   { return FindParm( m_ActiveParmID ); }
    int GetNumParmChanges()                 { return m_NumParmChanges; }
    void IncNumParmChanges()                { m_NumParmChanges++; }
    int GetChangeCnt()                      { m_ChangeCnt++; return m_ChangeCnt; }

    static Parm* CreateParm( int type );

    //=== Get Container, Group and Parm Name Given Parm ID ====//
    void GetNames( const string& parm_id, string& container_name, string& group_name, string& parm_name );
    void GetNames( const string& parm_id, string& container_name, string& group_name, string& group_alias, string& parm_name );

    bool GetDirtyFlag()                     { return m_DirtyFlag; }
    void SetDirtyFlag( bool flag )          { m_DirtyFlag = flag; }

};

#define ParmMgr ParmMgrSingleton::getInstance()

#endif // !defined(PARMMGR__INCLUDED_)
