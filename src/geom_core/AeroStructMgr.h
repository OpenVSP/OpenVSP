//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//
//
// Rob McDonald
//////////////////////////////////////////////////////////////////////

#if !defined(AEROSTRUCT__INCLUDED_)
#define AEROSTRUCT__INCLUDED_

#include "VehicleMgr.h"
#include "Vehicle.h"
#include "Geom.h"
#include "Parm.h"
#include "ProcessUtil.h"

#include <vector>
#include <string>
using std::string;
using std::vector;

class AeroStructSingleton : public ParmContainer
{
public:

    static AeroStructSingleton& getInstance()
    {
        static AeroStructSingleton instance;
        return instance;
    }

    virtual xmlNodePtr EncodeXml( xmlNodePtr & node );
    virtual xmlNodePtr DecodeXml( xmlNodePtr & node );

    void Update();

    void LoadDrawObjs( vector< DrawObj* > &draw_obj_vec );

    void FindCCX( const string & path );
    bool GetCalculiXFound()                                  { return m_CalculiXFound; }

    void TransferLoads( FILE * logFile = NULL );
    void ComputeStructure( FILE * logFile = NULL );

    Parm m_DynPress;

    bool m_ADBFileFound;
    bool m_FEAMeshFileFound;
    bool m_FEAInputFileFound;
    bool m_FEASolutionFileFound;

    string m_ADBFile;
    string m_FEAMeshFile;
    string m_FEAInputFile;
    string m_FEASolutionFile;

private:

    AeroStructSingleton();
    AeroStructSingleton( AeroStructSingleton const& copy );            // Not Implemented
    AeroStructSingleton& operator=( AeroStructSingleton const& copy ); // Not Implemented

    bool m_CalculiXFound;
    string m_CalculiXPath;
    string m_CalculiXCmd;

    ProcessUtil m_LoadsProcess;
    ProcessUtil m_CalculiXProcess;

};

#define AeroStructMgr AeroStructSingleton::getInstance()

#endif
