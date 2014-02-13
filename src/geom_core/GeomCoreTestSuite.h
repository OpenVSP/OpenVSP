//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//

// GeomCodeTestSuite.h: Unit tests for geom_core
// J.R Gloudemans
//
//////////////////////////////////////////////////////////////////////

#if !defined(VSPGEOMCORETESTSUITE__INCLUDED_)
#define VSPGEOMCORETESTSUITE__INCLUDED_

#include "cpptest.h"
#include "Vehicle.h"
#include "Vec3d.h"
#include "XSec.h"

class GeomCoreTestSuite : public Test::Suite
{
public:
    GeomCoreTestSuite()
    {
        TEST_ADD( GeomCoreTestSuite::GeomXFormTest )
        TEST_ADD( GeomCoreTestSuite::ParmTest )
        TEST_ADD( GeomCoreTestSuite::VehicleTest )
        TEST_ADD( GeomCoreTestSuite::PodTest )
        TEST_ADD( GeomCoreTestSuite::XmlTest )
        TEST_ADD( GeomCoreTestSuite::MeshIOTest )
    }

private:
    void GeomXFormTest();
    void ParmTest();
    void VehicleTest();
    void PodTest();
    void XmlTest();
    void MeshIOTest();
    void CompareMeshes( Vehicle & veh, string mesh_a, string mesh_b );
    void CompareVec3ds( const vec3d & v1, const vec3d & v2, const char * msg = NULL );

    void WritePnts( std::vector< vec3d > & pnt_vec, std::string file_name );

};

#endif // !defined(VSPGEOMCORETESTSUITE__INCLUDED_)
