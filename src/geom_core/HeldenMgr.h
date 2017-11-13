//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//

//////////////////////////////////////////////////////////////////////
// HeldenMgr.h
//////////////////////////////////////////////////////////////////////


#if !defined(HELDENMGR__INCLUDED_)
#define HELDENMGR__INCLUDED_

#include "Vehicle.h"

#include <assert.h>
#include "ProcessUtil.h"
#include "StringUtil.h"

#include <string>

using namespace std;

//////////////////////////////////////////////////////////////////////
class HeldenMgrSingleton : public ParmContainer
{
public:
    static HeldenMgrSingleton& getInstance()
    {
        static HeldenMgrSingleton instance;
        return instance;
    }

    virtual void Renew();

    virtual xmlNodePtr EncodeXml( xmlNodePtr & node );
    virtual xmlNodePtr DecodeXml( xmlNodePtr & node );

    virtual void ParmChanged( Parm* parm_ptr, int type );


    IntParm m_SelectedSetIndex;

    IntParm m_IGESLenUnit;
    BoolParm m_IGESSplitSurfs;
    BoolParm m_IGESSplitSubSurfs;
    BoolParm m_IGESToCubic;
    Parm m_IGESToCubicTol;
    BoolParm m_IGESTrimTE;

    bool IsHSurfRunning();
    void KillHSurf();
    void InitHSurf();

    // returns a result with a vector of results id's under the name ResultVec
    void ExecuteHSurf(FILE *logFile = NULL);

    ProcessUtil m_HSurfProcess;

    bool IsHPatchRunning();
    void KillHPatch();
    void InitHPatch();

    // returns a result with a vector of results id's under the name ResultVec
    void ExecuteHPatch(FILE *logFile = NULL);

    ProcessUtil m_HPatchProcess;

    bool IsHMeshRunning();
    void KillHMesh();
    void InitHMesh();

    // returns a result with a vector of results id's under the name ResultVec
    void ExecuteHMesh(FILE *logFile = NULL);

    ProcessUtil m_HMeshProcess;

    string FileInPathOf( const string & fin );

protected:

    void MonitorHSurf(FILE *logFile);
    bool m_HSurfProcessKill;

    void MonitorHPatch(FILE *logFile);
    bool m_HPatchProcessKill;

    void MonitorHMesh(FILE *logFile);
    bool m_HMeshProcessKill;

private:

    HeldenMgrSingleton();
    HeldenMgrSingleton( HeldenMgrSingleton const& copy );            // Not Implemented
    HeldenMgrSingleton& operator=( HeldenMgrSingleton const& copy ); // Not Implemented

    void Init();
    void Wype();

};

#define HeldenMgr HeldenMgrSingleton::getInstance()

#endif



