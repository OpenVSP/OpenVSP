//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//
//
//////////////////////////////////////////////////////////////////////

#include "GeomCoreTestSuite.h"
#include "MeshGeom.h"
#include <cfloat>  //For DBL_EPSILON

//==== Test GeomXForm ====//
void GeomCoreTestSuite::GeomXFormTest()
{

}

//==== Test Parm ====//
void GeomCoreTestSuite::ParmTest()
{
    Parm p;
    p.Init( "Test", "Group1", NULL, 1.0, 0.0, 2.0 );
    TEST_ASSERT_DELTA( 1.0, p.Get(), 1.0e-12 );
    p.SetLowerLimit( 2.0 );
    TEST_ASSERT_DELTA( 2.0, p.Get(), 1.0e-12 );
    p.SetUpperLimit( 10.0 );
    p = 20.0;
    TEST_ASSERT_DELTA( 10.0, p.Get(), 1.0e-12 );
    p.Deactivate();
    TEST_ASSERT( ! p.GetActiveFlag() );
    p.SetLowerUpperLimits( -1.0e12, 1.0e12 );
    p *= 10;
    TEST_ASSERT_DELTA( 10.0, p.GetLastVal(), 1.0e-12 );
    p += 1;
    p -= 0.1;
    TEST_ASSERT_DELTA( 100.9, p(), 1.0e-12 );

    IntParm ip;
    ip.Init( "Test", "Group2", NULL, 1, 2, 10 );
    TEST_ASSERT( ip() == 2 );

    BoolParm bp;
    bp.Set( false );
    TEST_ASSERT( ! bp() );

    bp.Set( ( bool )23.23 );
    TEST_ASSERT( bp() );
}

//==== Test Vehicle ====//
void GeomCoreTestSuite::VehicleTest()
{

    Vehicle veh;
    GeomType type;
    type.m_Name = "POD";

    string id0 = veh.AddGeom( type );
    veh.AddActiveGeom( id0 );

    string id1 = veh.AddGeom( type );
    veh.ClearActiveGeom();

    string id2 = veh.AddGeom( type );

    veh.AddActiveGeom( id0 );
    string id3 = veh.AddGeom( type );

    veh.ClearActiveGeom();
    string id4 = veh.AddGeom( type );

    veh.AddActiveGeom( id3 );
    string id5 = veh.AddGeom( type );

    //vector< string > geom_vec = veh.GetGeomVec();
    //TEST_ASSERT( geom_vec[0] == id0 );
    //TEST_ASSERT( geom_vec[1] == id1 );
    //TEST_ASSERT( geom_vec[2] == id3 );
    //TEST_ASSERT( geom_vec[3] == id5 );
    //TEST_ASSERT( geom_vec[4] == id2 );
    //TEST_ASSERT( geom_vec[5] == id4 );

    //veh.SetActiveGeom( id3 );
    //veh.CutActiveGeomVec();

    //geom_vec = veh.GetGeomVec();
    //TEST_ASSERT( geom_vec[0] == id0 );
    //TEST_ASSERT( geom_vec[1] == id1 );
    //TEST_ASSERT( geom_vec[2] == id5 );
    //TEST_ASSERT( geom_vec[3] == id2 );
    //TEST_ASSERT( geom_vec[4] == id4 );

    //veh.AddActiveGeom( id0 );
    //veh.AddActiveGeom( id1 );
    //veh.CutActiveGeomVec();

    //geom_vec = veh.GetGeomVec();
    //TEST_ASSERT( geom_vec[0] == id2 );
    //TEST_ASSERT( geom_vec[1] == id4 );
    //TEST_ASSERT( geom_vec[2] == id5 );

    //veh.SetActiveGeom( id4 );
    //veh.PasteClipboard();

    //geom_vec = veh.GetGeomVec();
    //TEST_ASSERT( geom_vec[0] == id2 );
    //TEST_ASSERT( geom_vec[1] == id4 );
    //TEST_ASSERT( geom_vec[2] == id0 );
    //TEST_ASSERT( geom_vec[3] == id1 );
    //TEST_ASSERT( geom_vec[4] == id5 );

    //veh.SetActiveGeom( id2 );
    //veh.PasteClipboard();

    //geom_vec = veh.GetGeomVec();
    //TEST_ASSERT( geom_vec[0] == id2 );
    //TEST_ASSERT( geom_vec[3] == id4 );
    //TEST_ASSERT( geom_vec[4] == id0 );
    //TEST_ASSERT( geom_vec[5] == id1 );
    //TEST_ASSERT( geom_vec[6] == id5 );

}

//==== Test Pod ====//
void GeomCoreTestSuite::PodTest()
{
    //FuseXSec fxsec;

    //fxsec.m_Height.Set(1.0);
    //fxsec.Update();
    //vector< vec3d > pnts = fxsec.GetBasePnts();
    //WritePnts( pnts, "SE_Test.dat" );


}


//==== WriteCurve =====//
void GeomCoreTestSuite::WritePnts( std::vector< vec3d > & pnt_vec, std::string file_name )
{
    FILE* fp = fopen( file_name.c_str(), "w" );
    if ( fp )
    {
        for ( int i = 0 ; i < ( int )pnt_vec.size() ; i++ )
        {
            fprintf( fp, "%f %f\n", pnt_vec[i].y(), pnt_vec[i].z() );
        }
        fprintf( fp, "C GREEN\n" );
        fclose( fp );
    }


}

void GeomCoreTestSuite::XmlTest()
{
    xmlNodePtr root = xmlNewNode( NULL, ( const xmlChar * )"Vsp_Geometry" );

    //==== Encode XML ====//
    vector< int > int_vec;
    int_vec.push_back( 2 );
    int_vec.push_back( 23 );
    int_vec.push_back( 3 );
    XmlUtil::AddVectorIntNode( root, "Int_Vec_Test", int_vec );

    vector< double > dbl_vec;
    dbl_vec.push_back( 1.234 );
    dbl_vec.push_back( 23.2323 );
    dbl_vec.push_back( 3.3323 );
    dbl_vec.push_back( 3.4323 );
    XmlUtil::AddVectorDoubleNode( root, "Dbl_Vec_Test", dbl_vec );


    //==== Decode XML ====//
    vector< int > ret_vec = XmlUtil::ExtractVectorIntNode( root, "Int_Vec_Test" );
    TEST_ASSERT( int_vec.size() == ret_vec.size() );
    for ( int i = 0 ; i < ( int )int_vec.size() ; i++ )
    {
        TEST_ASSERT( int_vec[i] == ret_vec[i] );
    }

    vector< double > dbl_ret_vec = XmlUtil::ExtractVectorDoubleNode( root, "Dbl_Vec_Test" );
    TEST_ASSERT( dbl_vec.size() == dbl_ret_vec.size() );
    for ( int i = 0 ; i < ( int )dbl_vec.size() ; i++ )
    {
        TEST_ASSERT( std::abs( dbl_vec[i] - dbl_ret_vec[i] ) < DBL_EPSILON  );
    }

    xmlFreeNode( root );
}

//==== Test Import/Export Files ====//
void GeomCoreTestSuite::MeshIOTest()
{
    Vehicle veh;
    GeomType type;
    string out_file;
    type.m_Type = POD_GEOM_TYPE;
    type.m_Name = "POD";

    string id0 = veh.AddGeom( type );

    string mesh_orig = veh.AddMeshGeom( 0 );
    TEST_ASSERT( mesh_orig.compare( "NONE" ) != 0 );

    out_file = "nascart_test.dat";
    veh.WriteNascartFiles( out_file, 0 );
    string mesh_ncart = veh.ImportFile( out_file, vsp::IMPORT_NASCART );
    TEST_ASSERT( mesh_ncart.compare( "NONE" ) != 0 );
    CompareMeshes( veh, mesh_orig, mesh_ncart );
    veh.ClearActiveGeom();
    veh.AddActiveGeom( mesh_ncart );
    veh.CutActiveGeomVec();

    out_file = "stl_test.stl";
    veh.WriteSTLFile( out_file, 0 );
    string mesh_stl = veh.ImportFile( out_file, vsp::IMPORT_STL );
    TEST_ASSERT( mesh_stl.compare( "NONE" ) != 0 );
    CompareMeshes( veh, mesh_orig, mesh_stl );
    veh.ClearActiveGeom();
    veh.AddActiveGeom( mesh_stl );
    veh.CutActiveGeomVec();

    out_file = "tri_test.tri";
    veh.WriteTRIFile( out_file, 0 );
    string mesh_tri = veh.ImportFile( out_file, vsp::IMPORT_CART3D_TRI );
    TEST_ASSERT( mesh_tri.compare( "NONE" ) != 0 );
    CompareMeshes( veh, mesh_orig, mesh_tri );
    veh.ClearActiveGeom();
    veh.AddActiveGeom( mesh_tri );
    veh.CutActiveGeomVec();

    out_file = "test_xsec.hrm";
    veh.WriteXSecFile( out_file, 0 );
    string mesh_hrm = veh.ImportFile( out_file, vsp::IMPORT_XSEC_MESH );
    TEST_ASSERT( mesh_hrm.compare( "NONE" ) != 0 );
    CompareMeshes( veh, mesh_orig, mesh_hrm );
    veh.ClearActiveGeom();
    veh.AddActiveGeom( mesh_hrm );
    veh.CutActiveGeomVec();
}

void GeomCoreTestSuite::CompareMeshes( Vehicle & veh, string mesh_a, string mesh_b )
{
    MeshGeom* mesh_1 = ( MeshGeom* )veh.FindGeom( mesh_a );
    MeshGeom* mesh_2 = ( MeshGeom* )veh.FindGeom( mesh_b );

    if ( mesh_1 && mesh_2 )
    {

        mesh_1->FlattenTMeshVec();
        mesh_2->FlattenTMeshVec();

        TEST_ASSERT( mesh_1->m_TMeshVec.size() == mesh_2->m_TMeshVec.size() );

        for ( int i = 0 ; i < ( int )mesh_1->m_TMeshVec.size() ; i++ )
        {
            TEST_ASSERT( mesh_1->m_TMeshVec[i]->m_TVec.size() == mesh_2->m_TMeshVec[i]->m_TVec.size() )
            for ( int t = 0 ; t < ( int )mesh_1->m_TMeshVec[i]->m_TVec.size() ; t++ )
            {
                TTri* tri1 = mesh_1->m_TMeshVec[i]->m_TVec[t];
                TTri* tri2 = mesh_2->m_TMeshVec[i]->m_TVec[t];
                CompareVec3ds( tri1->m_N0->m_Pnt, tri2->m_N0->m_Pnt );
                CompareVec3ds( tri1->m_N1->m_Pnt, tri2->m_N1->m_Pnt );
                CompareVec3ds( tri1->m_N2->m_Pnt, tri2->m_N2->m_Pnt );
                CompareVec3ds( tri1->m_Norm, tri2->m_Norm, "Norm" );
            }
        }
    }
}

void GeomCoreTestSuite::CompareVec3ds( const vec3d & v1, const vec3d & v2, const char* msg )
{
    char str[255];
    sprintf( str, "v1[0]: %10.16g v2[0]: %10.16g %s", v1[0], v2[0], msg );
    TEST_ASSERT_MSG( std::abs( v1[0] - v2[0] ) < 1e-5, str );
    sprintf( str, "v1[1]: %10.16g v2[1]: %10.16g %s", v1[1], v2[1], msg );
    TEST_ASSERT_MSG( std::abs( v1[1] - v2[1] ) < 1e-5, str );
    sprintf( str, "v1[2]: %10.16g v%10.16g %s", v1[2], v2[2], msg );
    TEST_ASSERT_MSG( std::abs( v1[2] - v2[2] ) < 1e-5, str );
}
