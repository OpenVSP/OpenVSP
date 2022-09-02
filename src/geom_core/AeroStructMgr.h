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

private:

    AeroStructSingleton();
    AeroStructSingleton( AeroStructSingleton const& copy );            // Not Implemented
    AeroStructSingleton& operator=( AeroStructSingleton const& copy ); // Not Implemented

    bool m_CalculiXFound;
    string m_CalculiXPath;
    string m_CalculiXCmd;

};

#define AeroStructMgr AeroStructSingleton::getInstance()

#endif
