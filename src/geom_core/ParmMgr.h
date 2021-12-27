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

#include "UsingCpp11.h"

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
    ParmMgrSingleton( ParmMgrSingleton const& copy );          // Not Implemented
    ParmMgrSingleton& operator=( ParmMgrSingleton const& copy ); // Not Implemented

    bool m_LastUndoFlag;
    ParmUndo m_LastUndo;
    std::stack< ParmUndo > m_ParmUndoStack;

    string m_ActiveParmID;

    unordered_map< string, Parm* > m_ParmMap;                       // ID->Parm Map
    unordered_map< string, ParmContainer* > m_ParmContainerMap;     // ID->Parm Container Map

    unordered_map< string, string > m_IDRemap;                      // oldID->newID Map
    string m_LastReset;

    int m_NumParmChanges;
    int m_ChangeCnt;

    bool m_DirtyFlag;

    string RemapID( const string & oldID, const string & suggestID, int size );

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
    string FindParmID( const string & name, const string & group, const string & container );
    ParmContainer* FindParmContainer( const string & id );

    void AddToUndoStack( Parm* parm_ptr, bool drag_flag );
    void UnDo();

    string ForceRemapID( const string & oldID, int size );
    string RemapID( const string & oldID, const string & suggestID = "" );
    string ResetRemapID( const string & lastReset = "" );

    void SetActiveParm( const string &id )         { m_ActiveParmID = id; }
    Parm* GetActiveParm()                   { return FindParm( m_ActiveParmID ); }
    int GetNumParmChanges()                 { return m_NumParmChanges; }
    void IncNumParmChanges()                { m_NumParmChanges++; }
    int GetChangeCnt()                      { m_ChangeCnt++; return m_ChangeCnt; }

    static Parm* CreateParm( int type );

    //=== Get Container, Group and Parm Name Given Parm ID ====//
    void GetNames( const string& parm_id, string& container_name, string& group_name, string& parm_name );

    bool GetDirtyFlag()                     { return m_DirtyFlag; }
    void SetDirtyFlag( bool flag )          { m_DirtyFlag = flag; }

};

#define ParmMgr ParmMgrSingleton::getInstance()

#endif // !defined(PARMMGR__INCLUDED_)
