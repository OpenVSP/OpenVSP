//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//

// DesignVarMgr.h: interface for the Design Variable Singleton.
//
//////////////////////////////////////////////////////////////////////

#if !defined(DESIGNVAR__INCLUDED_)
#define DESIGNVAR__INCLUDED_

#include "Parm.h"
#include "ParmContainer.h"
#include "APIDefines.h"
#include <vector>
#include <string>
using std::string;
using std::vector;

class DesignVar
{
public:
    DesignVar();

    string m_ParmID;
    int m_XDDM_Type;
};


//==== Design Variable Manager ====//
class DesignVarMgrSingleton
{
public:
    static DesignVarMgrSingleton& getInstance()
    {
        static DesignVarMgrSingleton instance;
        return instance;
    }

    virtual void Renew();

    virtual bool AddCurrVar();
    virtual void DelCurrVar();
    virtual void DelAllVars();

    bool CheckForDuplicateVar( const string & p );

    virtual bool AddVar( const string& parm_id, int xddmtype );

    virtual void SetCurrVarIndex( int i )
    {
        m_CurrVarIndex = i;
    }
    virtual int  GetCurrVarIndex()
    {
        return m_CurrVarIndex;
    }
    virtual DesignVar* GetCurrVar();
    virtual void CheckVars();

    virtual bool SortVars();

    virtual unsigned int GetNumVars()
    {
        return ( unsigned int )m_VarVec.size();
    }
    virtual DesignVar* GetVar( int index );

    virtual void SetWorkingParmID( string parm_id );

    virtual void WriteDesVarsDES( const string &newfile );
    virtual void ReadDesVarsDES( const string &newfile );

    virtual void WriteDesVarsXDDM( const string &newfile );
    virtual void ReadDesVarsXDDM( const string &newfile );

    virtual void ResetWorkingVar();

private:

    DesignVarMgrSingleton();
    DesignVarMgrSingleton( DesignVarMgrSingleton const& copy );          // Not Implemented
    DesignVarMgrSingleton& operator=( DesignVarMgrSingleton const& copy ); // Not Implemented

    void Init();
    void Wype();

    int m_CurrVarIndex;

    string m_WorkingParmID;

    vector < DesignVar* > m_VarVec;

};

#define DesignVarMgr DesignVarMgrSingleton::getInstance()

#endif
