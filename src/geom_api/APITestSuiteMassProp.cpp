//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//
// APITestSuiteMassProp.cpp: Unit tests for geom_api
// Justin Gravett
//
//////////////////////////////////////////////////////////////////////


#include "VSP_Geom_API.h"
#include "APITestSuiteMassProp.h"
#include <float.h>

//Default tolerance to use for tests.  Most calculations are done as doubles and choosing single precision FLT_MIN gives some allowance for precision stackup in calculations
#define TEST_TOL FLT_MIN

double APITestSuiteMassProp::GetTol( double val )
{
    double TOL = 0.1;

    if ( val != 0 )
    {
        TOL = val * 0.005;
    }
    else if ( val == 0 )
    {
        TOL = 0.1;
    }

    return TOL;
}

double APITestSuiteMassProp::GetCGTol( double val, double mass )
{
    double TOL = 0.005;

    if ( val != 0 )
    {
        TOL = val * 0.001;
    }
    else if ( val == 0 )
    {
        TOL = 0.005 * mass;
    }

    return TOL;
}

double APITestSuiteMassProp::GetInertiaTol( double val, double mass )
{
    double TOL = 0.005;

    if ( val != 0 )
    {
        TOL = val * 0.01;
    }
    else if ( val == 0 )
    {
        TOL = 0.005 * mass;
    }

    return TOL;
}

void APITestSuiteMassProp::TestSolidCylinder()
{
    printf( "APITestSuiteMassProp::TestSolidCylinder()\n" );
    // make sure setup works
    vsp::VSPCheckSetup();
    vsp::VSPRenew();
    TEST_ASSERT( !vsp::ErrorMgr.PopErrorAndPrint( stdout ) );    //PopErrorAndPrint returns TRUE if there is an error we want ASSERT to check that this is FALSE

    printf( "--> Generating Solid Cylinder Geometry\n" );

    //==== Add Fuselage Geom and Set Parameters =====//
    string fus_id = vsp::AddGeom( "FUSELAGE" );
    TEST_ASSERT( fus_id.c_str() != NULL );

    //  Adjust Fuselage length to 12:
    double length = 12;
    TEST_ASSERT_DELTA( vsp::SetParmVal( fus_id, "Length", "Design", length ), length, TEST_TOL );

    //  Change Type of Middle XSec to Circle and Ends to Points
    vsp::CutXSec( fus_id, 2 );
    string fusxsurf_id = vsp::GetXSecSurf( fus_id, 0 );
    vsp::ChangeXSecShape( fusxsurf_id, 0, vsp::XS_POINT );
    vsp::ChangeXSecShape( fusxsurf_id, 1, vsp::XS_CIRCLE );
    vsp::ChangeXSecShape( fusxsurf_id, 2, vsp::XS_CIRCLE );
    vsp::ChangeXSecShape( fusxsurf_id, 3, vsp::XS_POINT );

    string xsec_id0 = vsp::GetXSec( fusxsurf_id, 0 );
    string xsec_id1 = vsp::GetXSec( fusxsurf_id, 1 );
    string xsec_id2 = vsp::GetXSec( fusxsurf_id, 2 );
    string xsec_id3 = vsp::GetXSec( fusxsurf_id, 3 );

    //  Set Beginning and End of Cylinder (% value)
    string xloc_id1 = vsp::GetXSecParm( xsec_id1, "XLocPercent" );
    TEST_ASSERT_DELTA( vsp::SetParmVal( xloc_id1, 0.0 ), 0.0, TEST_TOL );
    string xloc_id2 = vsp::GetXSecParm( xsec_id2, "XLocPercent" );
    TEST_ASSERT_DELTA( vsp::SetParmVal( xloc_id2, 1.0 ), 1.0, TEST_TOL );

    //  Set Radius to 2:
    double radius = 2;
    string dia_id1 = vsp::GetXSecParm( xsec_id1, "Circle_Diameter" );
    string dia_id2 = vsp::GetXSecParm( xsec_id2, "Circle_Diameter" );
    TEST_ASSERT_DELTA( vsp::SetParmVal( dia_id1, radius * 2 ), radius * 2, TEST_TOL );
    TEST_ASSERT_DELTA( vsp::SetParmVal( dia_id2, radius * 2 ), radius * 2, TEST_TOL );

    //  Straighten Cylinder Ends:
    string sym_id0 = vsp::GetXSecParm( xsec_id0, "AllSym" );
    TEST_ASSERT_DELTA( vsp::SetParmVal( sym_id0, 1 ), 1, TEST_TOL );
    string Lstrength_id0 = vsp::GetXSecParm( xsec_id0, "TopLStrength" );
    TEST_ASSERT_DELTA( vsp::SetParmVal( Lstrength_id0, 0 ), 0, TEST_TOL );
    string Rstrength_id0 = vsp::GetXSecParm( xsec_id0, "TopRStrength" );
    TEST_ASSERT_DELTA( vsp::SetParmVal( Rstrength_id0, 0 ), 0, TEST_TOL );
    string sym_id3 = vsp::GetXSecParm( xsec_id3, "AllSym" );
    TEST_ASSERT_DELTA( vsp::SetParmVal( sym_id3, 1 ), 1, TEST_TOL );
    string Lstrength_id3 = vsp::GetXSecParm( xsec_id3, "TopLStrength" );
    TEST_ASSERT_DELTA( vsp::SetParmVal( Lstrength_id3, 0 ), 0, TEST_TOL );
    string Rstrength_id3 = vsp::GetXSecParm( xsec_id3, "TopRStrength" );
    TEST_ASSERT_DELTA( vsp::SetParmVal( Rstrength_id3, 0 ), 0, TEST_TOL );

    //  Straighten Cylinder Middle:
    string sym_id1 = vsp::GetXSecParm( xsec_id1, "AllSym" );
    TEST_ASSERT_DELTA( vsp::SetParmVal( sym_id1, 1 ), 1, TEST_TOL );
    string Lstrength_id1 = vsp::GetXSecParm( xsec_id1, "TopLStrength" );
    TEST_ASSERT_DELTA( vsp::SetParmVal( Lstrength_id1, 0 ), 0, TEST_TOL );
    string Rstrength_id1 = vsp::GetXSecParm( xsec_id1, "TopRStrength" );
    TEST_ASSERT_DELTA( vsp::SetParmVal( Rstrength_id1, 0 ), 0, TEST_TOL );
    string sym_id2 = vsp::GetXSecParm( xsec_id2, "AllSym" );
    TEST_ASSERT_DELTA( vsp::SetParmVal( sym_id2, 1 ), 1, TEST_TOL );
    string Lstrength_id2 = vsp::GetXSecParm( xsec_id2, "TopLStrength" );
    TEST_ASSERT_DELTA( vsp::SetParmVal( Lstrength_id2, 0 ), 0, TEST_TOL );
    string Rstrength_id2 = vsp::GetXSecParm( xsec_id2, "TopRStrength" );
    TEST_ASSERT_DELTA( vsp::SetParmVal( Rstrength_id2, 0 ), 0, TEST_TOL );

    //  Set Density to 5:
    double rho = 5;
    TEST_ASSERT_DELTA( vsp::SetParmVal( fus_id, "Density", "Mass_Props", rho ), rho, TEST_TOL );

    //  Increase W Tessellation:
    TEST_ASSERT_DELTA( vsp::SetParmVal( fus_id, "Tess_W", "Shape", 97 ), 97, TEST_TOL );

    //==== Analytical Mass Properties ====//
    double pi = 3.14159265359;
    double I_xx = pi * pow( radius, 4 ) * rho * length / 2;
    double I_xy = 0;
    double I_yy = pi * pow( radius, 2 ) * rho * length * ( 3 * pow( radius, 2 ) + pow( length, 2 ) ) / 12;
    double I_yz = 0;
    double I_zz = I_yy;
    double I_xz = 0;
    double Cz = 0;
    double Cy = 0;
    double Cx = length / 2;
    double volume = pi * pow( radius, 2 ) * length;
    double mass = rho * volume;

    vsp::Update();
    TEST_ASSERT( !vsp::ErrorMgr.PopErrorAndPrint( stdout ) );    //PopErrorAndPrint returns TRUE if there is an error we want ASSERT to check that this is FALSE

    printf( "   Cylinder Radius: %d \n", 2 );
    printf( "   Cylinder length: %f \n", length );
    printf( "   Cylinder Density: %f \n", rho );

    //==== Save Vehicle to File ====//
    printf( "-->Saving VSP model\n" );
    string fname = "APITestSolidCylinder.vsp3";
    vsp::WriteVSPFile( fname, vsp::SET_ALL );
    TEST_ASSERT( !vsp::ErrorMgr.PopErrorAndPrint( stdout ) );    //PopErrorAndPrint returns TRUE if there is an error we want ASSERT to check that this is FALSE

    // Set defaults
    vsp::SetAnalysisInputDefaults( "MassProp" );

    // list inputs, type, and current values
    vsp::PrintAnalysisInputs( "MassProp" );

    //==== Execute Mass Properties Analysis ====//
    printf( "-->Executing Mass Properties Analysis\n" );
    string results_id = vsp::ExecAnalysis( "MassProp" );
    TEST_ASSERT( results_id.size() > 0 );
    printf( "COMPLETE\n" );
    TEST_ASSERT( !vsp::ErrorMgr.PopErrorAndPrint( stdout ) );    //PopErrorAndPrint returns TRUE if there is an error we want ASSERT to check that this is FALSE

    vsp::DeleteGeomVec( vsp::GetStringResults( results_id, "Mesh_GeomID" ) );
    TEST_ASSERT( !vsp::ErrorMgr.PopErrorAndPrint( stdout ) );    //PopErrorAndPrint returns TRUE if there is an error we want ASSERT to check that this is FALSE

    // Get & Display Results
    vsp::PrintResults( results_id );

    // Compare to Analytical Solution:
    vector<vec3d> resCG = vsp::GetVec3dResults( results_id, "Total_CG", 0 );
    vector<double> resMass = vsp::GetDoubleResults( results_id, "Total_Mass", 0 );
    vector<double> resVolume = vsp::GetDoubleResults( results_id, "Total_Volume", 0 );
    vector<double> resIxx = vsp::GetDoubleResults( results_id, "Total_Ixx", 0 );
    vector<double> resIxy = vsp::GetDoubleResults( results_id, "Total_Ixy", 0 );
    vector<double> resIxz = vsp::GetDoubleResults( results_id, "Total_Ixz", 0 );
    vector<double> resIyy = vsp::GetDoubleResults( results_id, "Total_Iyy", 0 );
    vector<double> resIyz = vsp::GetDoubleResults( results_id, "Total_Iyz", 0 );
    vector<double> resIzz = vsp::GetDoubleResults( results_id, "Total_Izz", 0 );

    double CGX_TOL = GetCGTol( Cx, mass );
    double CGY_TOL = GetCGTol( Cy, mass );
    double CGZ_TOL = GetCGTol( Cz, mass );
    double IXX_TOL = GetInertiaTol( I_xx, mass );
    double IXY_TOL = GetInertiaTol( I_xy, mass );
    double IXZ_TOL = GetInertiaTol( I_xz, mass );
    double IYY_TOL = GetInertiaTol( I_yy, mass );
    double IYZ_TOL = GetInertiaTol( I_yz, mass );
    double IZZ_TOL = GetInertiaTol( I_zz, mass );
    double MASS_TOL = GetTol( mass );
    double VOLUME_TOL = GetTol( volume );

    TEST_ASSERT_DELTA( resCG[0].x(), Cx, CGX_TOL );
    TEST_ASSERT_DELTA( resCG[0].y(), Cy, CGY_TOL );
    TEST_ASSERT_DELTA( resCG[0].z(), Cz, CGZ_TOL );
    TEST_ASSERT_DELTA( resIxx[0], I_xx, IXX_TOL );
    TEST_ASSERT_DELTA( resIxy[0], I_xy, IXY_TOL );
    TEST_ASSERT_DELTA( resIxz[0], I_xz, IXZ_TOL );
    TEST_ASSERT_DELTA( resIyy[0], I_yy, IYY_TOL );
    TEST_ASSERT_DELTA( resIyz[0], I_yz, IYZ_TOL );
    TEST_ASSERT_DELTA( resIzz[0], I_zz, IZZ_TOL );
    TEST_ASSERT_DELTA( resMass[0], mass, MASS_TOL );
    TEST_ASSERT_DELTA( resVolume[0], volume, VOLUME_TOL );

    printf( "-->Analytical Mass Properties:\n" );
    printf( "   Cylinder_CG: %7.3f %7.3f %7.3f \n", Cx, Cy, Cz );
    printf( "   Cylinder_Ixx: %7.3f \n", I_xx );
    printf( "   Cylinder_Ixy: %7.3f \n", I_xy );
    printf( "   Cylinder_Ixz: %7.3f \n", I_xz );
    printf( "   Cylinder_Iyy: %7.3f \n", I_yy );
    printf( "   Cylinder_Iyz: %7.3f \n", I_yz );
    printf( "   Cylinder_Izz: %7.3f \n", I_zz );
    printf( "   Cylinder Mass: %7.3f \n", mass );
    printf( "   Cylinder Volume: %7.3f \n", volume );

    // Final check for errors
    TEST_ASSERT( !vsp::ErrorMgr.PopErrorAndPrint( stdout ) );    //PopErrorAndPrint returns TRUE if there is an error we want ASSERT to check that this is FALSE
    printf( "\n" );
}

void APITestSuiteMassProp::TestThickWallCylinder()
{
    printf( "APITestSuiteMassProp::TestThickWallCylinder()\n" );
    // make sure setup works
    vsp::VSPCheckSetup();
    vsp::VSPRenew();
    TEST_ASSERT( !vsp::ErrorMgr.PopErrorAndPrint( stdout ) );    //PopErrorAndPrint returns TRUE if there is an error we want ASSERT to check that this is FALSE

    printf( "--> Generating Thick Wall Cylinder Geometry\n" );

    //==== Add Stack Geom and Set Parameters =====//
    string stack_id = vsp::AddGeom( "STACK" );

    //  Change Type of All XSec to Circle
    string stackxsurf_id = vsp::GetXSecSurf( stack_id, 0 );
    vsp::ChangeXSecShape( stackxsurf_id, 0, vsp::XS_CIRCLE );
    vsp::ChangeXSecShape( stackxsurf_id, 1, vsp::XS_CIRCLE );
    vsp::ChangeXSecShape( stackxsurf_id, 2, vsp::XS_CIRCLE );
    vsp::ChangeXSecShape( stackxsurf_id, 3, vsp::XS_CIRCLE );
    vsp::ChangeXSecShape( stackxsurf_id, 4, vsp::XS_CIRCLE );

    //  Set Design Policy to STACK_LOOP
    TEST_ASSERT_DELTA( vsp::SetParmVal( stack_id, "OrderPolicy", "Design", 1 ), 1, TEST_TOL );

    //  Get XSec IDs:
    string xsec_id0 = vsp::GetXSec( stackxsurf_id, 0 );
    string xsec_id1 = vsp::GetXSec( stackxsurf_id, 1 );
    string xsec_id2 = vsp::GetXSec( stackxsurf_id, 2 );
    string xsec_id3 = vsp::GetXSec( stackxsurf_id, 3 );
    string xsec_id4 = vsp::GetXSec( stackxsurf_id, 4 );

    //  Set Inner Radius to 1:
    double r_in = 1;
    string dia_id0 = vsp::GetXSecParm( xsec_id0, "Circle_Diameter" );
    string dia_id3 = vsp::GetXSecParm( xsec_id3, "Circle_Diameter" );
    string dia_id4 = vsp::GetXSecParm( xsec_id4, "Circle_Diameter" );

    TEST_ASSERT_DELTA( vsp::SetParmVal( dia_id0, r_in * 2 ), r_in * 2, TEST_TOL );
    TEST_ASSERT_DELTA( vsp::SetParmVal( dia_id3, r_in * 2 ), r_in * 2, TEST_TOL );
    TEST_ASSERT_DELTA( vsp::SetParmVal( dia_id4, r_in * 2 ), r_in * 2, TEST_TOL );

    //  Set Outer Radius to 2:
    double R_out = 2;
    string dia_id1 = vsp::GetXSecParm( xsec_id1, "Circle_Diameter" );
    string dia_id2 = vsp::GetXSecParm( xsec_id2, "Circle_Diameter" );

    TEST_ASSERT_DELTA( vsp::SetParmVal( dia_id1, R_out * 2 ), R_out * 2, TEST_TOL );
    TEST_ASSERT_DELTA( vsp::SetParmVal( dia_id2, R_out * 2 ), R_out * 2, TEST_TOL );

    //  Set Beginning and End of Cylinder
    double length = 12;
    string xloc_id1 = vsp::GetXSecParm( xsec_id1, "XDelta" );
    TEST_ASSERT_DELTA( vsp::SetParmVal( xloc_id1, 0.0 ), 0.0, TEST_TOL );
    string xloc_id2 = vsp::GetXSecParm( xsec_id2, "XDelta" );
    TEST_ASSERT_DELTA( vsp::SetParmVal( xloc_id2, length ), length, TEST_TOL );
    string xloc_id3 = vsp::GetXSecParm( xsec_id3, "XDelta" );
    TEST_ASSERT_DELTA( vsp::SetParmVal( xloc_id3, 0.0 ), 0.0, TEST_TOL );

    //  Straighten Cylinder:
    string sym_id0 = vsp::GetXSecParm( xsec_id0, "AllSym" );
    TEST_ASSERT_DELTA( vsp::SetParmVal( sym_id0, 1 ), 1, TEST_TOL );
    string Lstrength_id0 = vsp::GetXSecParm( xsec_id0, "TopLStrength" );
    TEST_ASSERT_DELTA( vsp::SetParmVal( Lstrength_id0, 0 ), 0, TEST_TOL );
    string Rstrength_id0 = vsp::GetXSecParm( xsec_id0, "TopRStrength" );
    TEST_ASSERT_DELTA( vsp::SetParmVal( Rstrength_id0, 0 ), 0, TEST_TOL );
    string sym_id1 = vsp::GetXSecParm( xsec_id1, "AllSym" );
    TEST_ASSERT_DELTA( vsp::SetParmVal( sym_id1, 1 ), 1, TEST_TOL );
    string Lstrength_id1 = vsp::GetXSecParm( xsec_id1, "TopLStrength" );
    TEST_ASSERT_DELTA( vsp::SetParmVal( Lstrength_id1, 0 ), 0, TEST_TOL );
    string Rstrength_id1 = vsp::GetXSecParm( xsec_id1, "TopRStrength" );
    TEST_ASSERT_DELTA( vsp::SetParmVal( Rstrength_id1, 0 ), 0, TEST_TOL );
    string sym_id2 = vsp::GetXSecParm( xsec_id2, "AllSym" );
    TEST_ASSERT_DELTA( vsp::SetParmVal( sym_id2, 1 ), 1, TEST_TOL );
    string Lstrength_id2 = vsp::GetXSecParm( xsec_id2, "TopLStrength" );
    TEST_ASSERT_DELTA( vsp::SetParmVal( Lstrength_id2, 0 ), 0, TEST_TOL );
    string Rstrength_id2 = vsp::GetXSecParm( xsec_id2, "TopRStrength" );
    TEST_ASSERT_DELTA( vsp::SetParmVal( Rstrength_id2, 0 ), 0, TEST_TOL );
    string sym_id3 = vsp::GetXSecParm( xsec_id3, "AllSym" );
    TEST_ASSERT_DELTA( vsp::SetParmVal( sym_id3, 1 ), 1, TEST_TOL );
    string Lstrength_id3 = vsp::GetXSecParm( xsec_id3, "TopLStrength" );
    TEST_ASSERT_DELTA( vsp::SetParmVal( Lstrength_id3, 0 ), 0, TEST_TOL );
    string Rstrength_id3 = vsp::GetXSecParm( xsec_id3, "TopRStrength" );
    TEST_ASSERT_DELTA( vsp::SetParmVal( Rstrength_id3, 0 ), 0, TEST_TOL );
    string sym_id4 = vsp::GetXSecParm( xsec_id4, "AllSym" );
    TEST_ASSERT_DELTA( vsp::SetParmVal( sym_id4, 1 ), 1, TEST_TOL );
    string Lstrength_id4 = vsp::GetXSecParm( xsec_id4, "TopLStrength" );
    TEST_ASSERT_DELTA( vsp::SetParmVal( Lstrength_id4, 0 ), 0, TEST_TOL );
    string Rstrength_id4 = vsp::GetXSecParm( xsec_id4, "TopRStrength" );
    TEST_ASSERT_DELTA( vsp::SetParmVal( Rstrength_id4, 0 ), 0, TEST_TOL );

    //  Set Density to 5:
    double rho = 5;
    TEST_ASSERT_DELTA( vsp::SetParmVal( stack_id, "Density", "Mass_Props", rho ), rho, TEST_TOL );

    //  Increase W Tessellation:
    TEST_ASSERT_DELTA( vsp::SetParmVal( stack_id, "Tess_W", "Shape", 73 ), 73, TEST_TOL );

    //==== Analytical Mass Properties ====//
    double pi = 3.14159265359;
    double I_xx = ( pi * rho * length / 2 ) * ( pow( R_out, 4 ) - pow( r_in, 4 ) );
    double I_xy = 0;
    double I_yy = ( pi * rho * length / 12 ) * ( 3 * ( pow( R_out, 4 ) - pow( r_in, 4 ) ) + pow( length, 2 ) * ( pow( R_out, 2 ) - pow( r_in, 2 ) ) );
    double I_yz = 0;
    double I_zz = I_yy;
    double I_xz = 0;
    double Cz = 0;
    double Cy = 0;
    double Cx = length / 2;
    double volume = pi * length * ( pow( R_out, 2 ) - pow( r_in, 2 ) );
    double mass = rho * volume;

    vsp::Update();
    TEST_ASSERT( !vsp::ErrorMgr.PopErrorAndPrint( stdout ) );    //PopErrorAndPrint returns TRUE if there is an error we want ASSERT to check that this is FALSE

    printf( "   Cylinder Outer Radius: %f \n", R_out );
    printf( "   Cylinder Inner Radius: %f \n", r_in );
    printf( "   Cylinder Length: %f \n", length );
    printf( "   Cylinder Density: %f \n", rho );

    //==== Save Vehicle to File ====//
    printf( "-->Saving VSP model\n" );
    string fname = "APITestThickWallCylinder.vsp3";
    vsp::WriteVSPFile( fname, vsp::SET_ALL );
    TEST_ASSERT( !vsp::ErrorMgr.PopErrorAndPrint( stdout ) );    //PopErrorAndPrint returns TRUE if there is an error we want ASSERT to check that this is FALSE

    // Set defaults
    vsp::SetAnalysisInputDefaults( "MassProp" );

    // list inputs, type, and current values
    vsp::PrintAnalysisInputs( "MassProp" );

    //==== Execute Mass Properties Analysis ====//
    printf( "-->Executing Mass Properties Analysis\n" );
    string results_id = vsp::ExecAnalysis( "MassProp" );
    TEST_ASSERT( results_id.size() > 0 );
    printf( "COMPLETE\n" );
    TEST_ASSERT( !vsp::ErrorMgr.PopErrorAndPrint( stdout ) );    //PopErrorAndPrint returns TRUE if there is an error we want ASSERT to check that this is FALSE

    vsp::DeleteGeomVec( vsp::GetStringResults( results_id, "Mesh_GeomID" ) );
    TEST_ASSERT( !vsp::ErrorMgr.PopErrorAndPrint( stdout ) );    //PopErrorAndPrint returns TRUE if there is an error we want ASSERT to check that this is FALSE

    // Get & Display Results
    vsp::PrintResults( results_id );

    // Compare to Analytical Solution:
    vector<vec3d> resCG = vsp::GetVec3dResults( results_id, "Total_CG", 0 );
    vector<double> resMass = vsp::GetDoubleResults( results_id, "Total_Mass", 0 );
    vector<double> resVolume = vsp::GetDoubleResults( results_id, "Total_Volume", 0 );
    vector<double> resIxx = vsp::GetDoubleResults( results_id, "Total_Ixx", 0 );
    vector<double> resIxy = vsp::GetDoubleResults( results_id, "Total_Ixy", 0 );
    vector<double> resIxz = vsp::GetDoubleResults( results_id, "Total_Ixz", 0 );
    vector<double> resIyy = vsp::GetDoubleResults( results_id, "Total_Iyy", 0 );
    vector<double> resIyz = vsp::GetDoubleResults( results_id, "Total_Iyz", 0 );
    vector<double> resIzz = vsp::GetDoubleResults( results_id, "Total_Izz", 0 );

    double CGX_TOL = GetCGTol( Cx, mass );
    double CGY_TOL = GetCGTol( Cy, mass );
    double CGZ_TOL = GetCGTol( Cz, mass );
    double IXX_TOL = GetInertiaTol( I_xx, mass );
    double IXY_TOL = GetInertiaTol( I_xy, mass );
    double IXZ_TOL = GetInertiaTol( I_xz, mass );
    double IYY_TOL = GetInertiaTol( I_yy, mass );
    double IYZ_TOL = GetInertiaTol( I_yz, mass );
    double IZZ_TOL = GetInertiaTol( I_zz, mass );
    double MASS_TOL = GetTol( mass );
    double VOLUME_TOL = GetTol( volume );

    TEST_ASSERT_DELTA( resCG[0].x(), Cx, CGX_TOL );
    TEST_ASSERT_DELTA( resCG[0].y(), Cy, CGY_TOL );
    TEST_ASSERT_DELTA( resCG[0].z(), Cz, CGZ_TOL );
    TEST_ASSERT_DELTA( resIxx[0], I_xx, IXX_TOL );
    TEST_ASSERT_DELTA( resIxy[0], I_xy, IXY_TOL );
    TEST_ASSERT_DELTA( resIxz[0], I_xz, IXZ_TOL );
    TEST_ASSERT_DELTA( resIyy[0], I_yy, IYY_TOL );
    TEST_ASSERT_DELTA( resIyz[0], I_yz, IYZ_TOL );
    TEST_ASSERT_DELTA( resIzz[0], I_zz, IZZ_TOL );
    TEST_ASSERT_DELTA( resMass[0], mass, MASS_TOL );
    TEST_ASSERT_DELTA( resVolume[0], volume, VOLUME_TOL );

    printf( "-->Analytical Mass Properties:\n" );
    printf( "   Cylinder_CG: %7.3f %7.3f %7.3f \n", Cx, Cy, Cz );
    printf( "   Cylinder_Ixx: %7.3f \n", I_xx );
    printf( "   Cylinder_Ixy: %7.3f \n", I_xy );
    printf( "   Cylinder_Ixz: %7.3f \n", I_xz );
    printf( "   Cylinder_Iyy: %7.3f \n", I_yy );
    printf( "   Cylinder_Iyz: %7.3f \n", I_yz );
    printf( "   Cylinder_Izz: %7.3f \n", I_zz );
    printf( "   Cylinder Mass: %7.3f \n", mass );
    printf( "   Cylinder Volume: %7.3f \n", volume );

    // Final check for errors
    TEST_ASSERT( !vsp::ErrorMgr.PopErrorAndPrint( stdout ) );    //PopErrorAndPrint returns TRUE if there is an error we want ASSERT to check that this is FALSE
    printf( "\n" );
}

void APITestSuiteMassProp::TestCylindricalShell()
{
    printf( "APITestSuiteMassProp::TestCylindricalShell()\n" );
    // make sure setup works
    vsp::VSPCheckSetup();
    vsp::VSPRenew();
    TEST_ASSERT( !vsp::ErrorMgr.PopErrorAndPrint( stdout ) );    //PopErrorAndPrint returns TRUE if there is an error we want ASSERT to check that this is FALSE

    printf( "--> Generating Cylindrical Shell Geometry\n" );

    //==== Add Stack Geom and Set Parameters =====//
    string stack_id = vsp::AddGeom( "STACK" );

    //  Change Type of All XSec to Circle
    vsp::CutXSec( stack_id, 3 );
    string stackxsurf_id = vsp::GetXSecSurf( stack_id, 0 );
    vsp::ChangeXSecShape( stackxsurf_id, 0, vsp::XS_CIRCLE );
    vsp::ChangeXSecShape( stackxsurf_id, 1, vsp::XS_CIRCLE );
    vsp::ChangeXSecShape( stackxsurf_id, 2, vsp::XS_CIRCLE );
    vsp::ChangeXSecShape( stackxsurf_id, 3, vsp::XS_CIRCLE );

    //  Get XSec IDs:
    string xsec_id0 = vsp::GetXSec( stackxsurf_id, 0 );
    string xsec_id1 = vsp::GetXSec( stackxsurf_id, 1 );
    string xsec_id2 = vsp::GetXSec( stackxsurf_id, 2 );
    string xsec_id3 = vsp::GetXSec( stackxsurf_id, 3 );

    //  Set Radius to 2:
    double radius = 2;
    string dia_id0 = vsp::GetXSecParm( xsec_id0, "Circle_Diameter" );
    string dia_id1 = vsp::GetXSecParm( xsec_id1, "Circle_Diameter" );
    string dia_id2 = vsp::GetXSecParm( xsec_id2, "Circle_Diameter" );
    string dia_id3 = vsp::GetXSecParm( xsec_id3, "Circle_Diameter" );

    TEST_ASSERT_DELTA( vsp::SetParmVal( dia_id0, 0 ), 0, TEST_TOL );
    TEST_ASSERT_DELTA( vsp::SetParmVal( dia_id1, radius * 2 ), radius * 2, TEST_TOL );
    TEST_ASSERT_DELTA( vsp::SetParmVal( dia_id2, radius * 2 ), radius * 2, TEST_TOL );
    TEST_ASSERT_DELTA( vsp::SetParmVal( dia_id3, 0 ), 0, TEST_TOL );

    //  Set Beginning and End of Cylinder
    double length = 12;
    string xloc_id0 = vsp::GetXSecParm( xsec_id0, "XDelta" );
    TEST_ASSERT_DELTA( vsp::SetParmVal( xloc_id0, 0.0 ), 0.0, TEST_TOL );
    string xloc_id1 = vsp::GetXSecParm( xsec_id1, "XDelta" );
    TEST_ASSERT_DELTA( vsp::SetParmVal( xloc_id1, 0.0 ), 0.0, TEST_TOL );
    string xloc_id2 = vsp::GetXSecParm( xsec_id2, "XDelta" );
    TEST_ASSERT_DELTA( vsp::SetParmVal( xloc_id2, length ), length, TEST_TOL );
    string xloc_id3 = vsp::GetXSecParm( xsec_id3, "XDelta" );
    TEST_ASSERT_DELTA( vsp::SetParmVal( xloc_id3, 0.0 ), 0.0, TEST_TOL );

    //  Straighten Cylinder:
    string sym_id0 = vsp::GetXSecParm( xsec_id0, "AllSym" );
    TEST_ASSERT_DELTA( vsp::SetParmVal( sym_id0, 1 ), 1, TEST_TOL );
    string Lstrength_id0 = vsp::GetXSecParm( xsec_id0, "TopLStrength" );
    TEST_ASSERT_DELTA( vsp::SetParmVal( Lstrength_id0, 0 ), 0, TEST_TOL );
    string Rstrength_id0 = vsp::GetXSecParm( xsec_id0, "TopRStrength" );
    TEST_ASSERT_DELTA( vsp::SetParmVal( Rstrength_id0, 0 ), 0, TEST_TOL );
    string sym_id1 = vsp::GetXSecParm( xsec_id1, "AllSym" );
    TEST_ASSERT_DELTA( vsp::SetParmVal( sym_id1, 1 ), 1, TEST_TOL );
    string Lstrength_id1 = vsp::GetXSecParm( xsec_id1, "TopLStrength" );
    TEST_ASSERT_DELTA( vsp::SetParmVal( Lstrength_id1, 0 ), 0, TEST_TOL );
    string Rstrength_id1 = vsp::GetXSecParm( xsec_id1, "TopRStrength" );
    TEST_ASSERT_DELTA( vsp::SetParmVal( Rstrength_id1, 0 ), 0, TEST_TOL );
    string sym_id2 = vsp::GetXSecParm( xsec_id2, "AllSym" );
    TEST_ASSERT_DELTA( vsp::SetParmVal( sym_id2, 1 ), 1, TEST_TOL );
    string Lstrength_id2 = vsp::GetXSecParm( xsec_id2, "TopLStrength" );
    TEST_ASSERT_DELTA( vsp::SetParmVal( Lstrength_id2, 0 ), 0, TEST_TOL );
    string Rstrength_id2 = vsp::GetXSecParm( xsec_id2, "TopRStrength" );
    TEST_ASSERT_DELTA( vsp::SetParmVal( Rstrength_id2, 0 ), 0, TEST_TOL );
    string sym_id3 = vsp::GetXSecParm( xsec_id3, "AllSym" );
    TEST_ASSERT_DELTA( vsp::SetParmVal( sym_id3, 1 ), 1, TEST_TOL );
    string Lstrength_id3 = vsp::GetXSecParm( xsec_id3, "TopLStrength" );
    TEST_ASSERT_DELTA( vsp::SetParmVal( Lstrength_id3, 0 ), 0, TEST_TOL );
    string Rstrength_id3 = vsp::GetXSecParm( xsec_id3, "TopRStrength" );
    TEST_ASSERT_DELTA( vsp::SetParmVal( Rstrength_id3, 0 ), 0, TEST_TOL );

    //  Set Thin Shell Flag:
    TEST_ASSERT_DELTA( vsp::SetParmVal( stack_id, "Shell_Flag", "Mass_Props", 1 ), 1, TEST_TOL );

    //  Set Mass/Area:
    double rho_A = 0.5;
    TEST_ASSERT_DELTA( vsp::SetParmVal( stack_id, "Mass_Area", "Mass_Props", rho_A ), rho_A, TEST_TOL );

    //  Set Density:
    double rho = 0;
    TEST_ASSERT_DELTA( vsp::SetParmVal( stack_id, "Density", "Mass_Props", rho ), rho, TEST_TOL );

    //  Increase W Tessellation:
    TEST_ASSERT_DELTA( vsp::SetParmVal( stack_id, "Tess_W", "Shape", 73 ), 73, TEST_TOL );

    //==== Analytical Mass Properties ====//
    double diskSA = PI * pow( radius, 2.0 );
    double diskMass = rho_A * diskSA;
    double shellSA = 2.0 * PI * radius * length;
    double shellMass = rho_A * shellSA;
    double diskIxx = diskMass * pow( radius, 2.0 ) / 2.0;
    double diskIyy = diskMass * pow( radius, 2.0 ) / 4.0 + diskMass * pow( ( length / 2.0 ), 2.0 );
    double shellIxx = shellMass * pow( radius, 2.0 );
    double shellIyy = shellMass * pow( radius, 2.0 ) / 2.0 + shellMass * pow( length, 2.0 ) / 12.0;
    double shellIzz = shellIyy;
    double mass = shellMass + ( 2.0 * diskMass );
    double I_xx = shellIxx + ( 2.0 * diskIxx );
    double I_xy = 0;
    double I_yy = shellIyy + ( 2.0 * diskIyy );
    double I_yz = 0;
    double I_zz = I_yy;
    double I_xz = 0;
    double Cz = 0;
    double Cy = 0;
    double Cx = length / 2;

    vsp::Update();
    TEST_ASSERT( !vsp::ErrorMgr.PopErrorAndPrint( stdout ) );    //PopErrorAndPrint returns TRUE if there is an error we want ASSERT to check that this is FALSE

    printf( "   Cylinder Radius: %d \n", 2 );
    printf( "   Cylinder Length: %f \n", length );
    printf( "   Cylinder Density: %7.3f \n", rho );
    printf( "   Cylinder Area Density: %7.3f \n", rho_A );

    //==== Save Vehicle to File ====//
    printf( "-->Saving VSP model\n" );
    string fname = "APITestCylindricalShell.vsp3";
    vsp::WriteVSPFile( fname, vsp::SET_ALL );
    TEST_ASSERT( !vsp::ErrorMgr.PopErrorAndPrint( stdout ) );    //PopErrorAndPrint returns TRUE if there is an error we want ASSERT to check that this is FALSE

    // Set defaults
    vsp::SetAnalysisInputDefaults( "MassProp" );

    // list inputs, type, and current values
    vsp::PrintAnalysisInputs( "MassProp" );

    //==== Execute Mass Properties Analysis ====//
    printf( "-->Executing Mass Properties Analysis\n" );
    string results_id = vsp::ExecAnalysis( "MassProp" );
    TEST_ASSERT( results_id.size() > 0 );
    printf( "COMPLETE\n" );
    TEST_ASSERT( !vsp::ErrorMgr.PopErrorAndPrint( stdout ) );    //PopErrorAndPrint returns TRUE if there is an error we want ASSERT to check that this is FALSE

    vsp::DeleteGeomVec( vsp::GetStringResults( results_id, "Mesh_GeomID" ) );
    TEST_ASSERT( !vsp::ErrorMgr.PopErrorAndPrint( stdout ) );    //PopErrorAndPrint returns TRUE if there is an error we want ASSERT to check that this is FALSE

    // Get & Display Results
    vsp::PrintResults( results_id );

    // Compare to Analytical Solution:
    vector<vec3d> resCG = vsp::GetVec3dResults( results_id, "Total_CG", 0 );
    vector<double> resMass = vsp::GetDoubleResults( results_id, "Total_Mass", 0 );
    vector<double> resVolume = vsp::GetDoubleResults( results_id, "Total_Volume", 0 );
    vector<double> resIxx = vsp::GetDoubleResults( results_id, "Total_Ixx", 0 );
    vector<double> resIxy = vsp::GetDoubleResults( results_id, "Total_Ixy", 0 );
    vector<double> resIxz = vsp::GetDoubleResults( results_id, "Total_Ixz", 0 );
    vector<double> resIyy = vsp::GetDoubleResults( results_id, "Total_Iyy", 0 );
    vector<double> resIyz = vsp::GetDoubleResults( results_id, "Total_Iyz", 0 );
    vector<double> resIzz = vsp::GetDoubleResults( results_id, "Total_Izz", 0 );

    double CGX_TOL = GetCGTol( Cx, mass );
    double CGY_TOL = GetCGTol( Cy, mass );
    double CGZ_TOL = GetCGTol( Cz, mass );
    double IXX_TOL = GetInertiaTol( I_xx, mass );
    double IXY_TOL = GetInertiaTol( I_xy, mass );
    double IXZ_TOL = GetInertiaTol( I_xz, mass );
    double IYY_TOL = GetInertiaTol( I_yy, mass );
    double IYZ_TOL = GetInertiaTol( I_yz, mass );
    double IZZ_TOL = GetInertiaTol( I_zz, mass );
    double MASS_TOL = GetTol( mass );

    TEST_ASSERT_DELTA( resCG[0].x(), Cx, CGX_TOL );
    TEST_ASSERT_DELTA( resCG[0].y(), Cy, CGY_TOL );
    TEST_ASSERT_DELTA( resCG[0].z(), Cz, CGZ_TOL );
    TEST_ASSERT_DELTA( resIxx[0], I_xx, IXX_TOL );
    TEST_ASSERT_DELTA( resIxy[0], I_xy, IXY_TOL );
    TEST_ASSERT_DELTA( resIxz[0], I_xz, IXZ_TOL );
    TEST_ASSERT_DELTA( resIyy[0], I_yy, IYY_TOL );
    TEST_ASSERT_DELTA( resIyz[0], I_yz, IYZ_TOL );
    TEST_ASSERT_DELTA( resIzz[0], I_zz, IZZ_TOL );
    TEST_ASSERT_DELTA( resMass[0], mass, MASS_TOL );

    printf( "-->Analytical Mass Properties:\n" );
    printf( "   Cylinder_CG: %7.3f %7.3f %7.3f \n", Cx, Cy, Cz );
    printf( "   Cylinder_Ixx: %7.3f \n", I_xx );
    printf( "   Cylinder_Ixy: %7.3f \n", I_xy );
    printf( "   Cylinder_Ixz: %7.3f \n", I_xz );
    printf( "   Cylinder_Iyy: %7.3f \n", I_yy );
    printf( "   Cylinder_Iyz: %7.3f \n", I_yz );
    printf( "   Cylinder_Izz: %7.3f \n", I_zz );
    printf( "   Cylinder Mass: %7.3f \n", mass );
    printf( "   Cylinder Surface Area: %7.3f \n", shellSA );

    // Final check for errors
    TEST_ASSERT( !vsp::ErrorMgr.PopErrorAndPrint( stdout ) );    //PopErrorAndPrint returns TRUE if there is an error we want ASSERT to check that this is FALSE
    printf( "\n" );
}

void APITestSuiteMassProp::TestSphere()
{
    printf( "APITestSuiteMassProp::TestSphere()\n" );
    // make sure setup works
    vsp::VSPCheckSetup();
    vsp::VSPRenew();
    TEST_ASSERT( !vsp::ErrorMgr.PopErrorAndPrint( stdout ) );    //PopErrorAndPrint returns TRUE if there is an error we want ASSERT to check that this is FALSE

    printf( "--> Generating Sphere Geometry\n" );

    //==== Add Stack Geom and Set Parameters =====//
    string stack_id = vsp::AddGeom( "STACK" );

    //  Change Type of Middle XSec to Circle and Ends to Points
    vsp::CutXSec( stack_id, 2 );
    vsp::CutXSec( stack_id, 3 );
    string stackxsurf_id = vsp::GetXSecSurf( stack_id, 0 );
    vsp::ChangeXSecShape( stackxsurf_id, 0, vsp::XS_POINT );
    vsp::ChangeXSecShape( stackxsurf_id, 1, vsp::XS_CIRCLE );
    vsp::ChangeXSecShape( stackxsurf_id, 2, vsp::XS_POINT );

    //  Get XSec IDs:
    string xsec_id0 = vsp::GetXSec( stackxsurf_id, 0 );
    string xsec_id1 = vsp::GetXSec( stackxsurf_id, 1 );
    string xsec_id2 = vsp::GetXSec( stackxsurf_id, 2 );

    //  Set Radius to 2:
    double radius = 2;
    string dia_id1 = vsp::GetXSecParm( xsec_id1, "Circle_Diameter" );

    TEST_ASSERT_DELTA( vsp::SetParmVal( dia_id1, radius * 2 ), radius * 2, TEST_TOL );

    //  Set Beginning and End of Sphere
    string xloc_id0 = vsp::GetXSecParm( xsec_id0, "XDelta" );
    TEST_ASSERT_DELTA( vsp::SetParmVal( xloc_id0, 0.0 ), 0.0, TEST_TOL );
    string xloc_id1 = vsp::GetXSecParm( xsec_id1, "XDelta" );
    TEST_ASSERT_DELTA( vsp::SetParmVal( xloc_id1, radius ), radius, TEST_TOL );
    string xloc_id2 = vsp::GetXSecParm( xsec_id2, "XDelta" );
    TEST_ASSERT_DELTA( vsp::SetParmVal( xloc_id2, radius ), radius, TEST_TOL );

    //  Increase U Tessellation
    string xutess_id0 = vsp::GetXSecParm( xsec_id0, "SectTess_U" );
    TEST_ASSERT_DELTA( vsp::SetParmVal( xutess_id0, 20 ), 20, TEST_TOL );
    string xutess_id1 = vsp::GetXSecParm( xsec_id1, "SectTess_U" );
    TEST_ASSERT_DELTA( vsp::SetParmVal( xutess_id1, 20 ), 20, TEST_TOL );
    string xutess_id2 = vsp::GetXSecParm( xsec_id2, "SectTess_U" );
    TEST_ASSERT_DELTA( vsp::SetParmVal( xutess_id2, 20 ), 20, TEST_TOL );

    //  Round Sphere:
    string sym_id0 = vsp::GetXSecParm( xsec_id0, "AllSym" );
    TEST_ASSERT_DELTA( vsp::SetParmVal( sym_id0, 1 ), 1, TEST_TOL );
    string Lstrength_id0 = vsp::GetXSecParm( xsec_id0, "TopLStrength" );
    TEST_ASSERT_DELTA( vsp::SetParmVal( Lstrength_id0, 1.172 ), 1.172, TEST_TOL );
    string Langle_id0 = vsp::GetXSecParm( xsec_id0, "TopLAngle" );
    TEST_ASSERT_DELTA( vsp::SetParmVal( Langle_id0, 90 ), 90, TEST_TOL );
    string Rstrength_id0 = vsp::GetXSecParm( xsec_id0, "TopRStrength" );
    TEST_ASSERT_DELTA( vsp::SetParmVal( Rstrength_id0, 1.172 ), 1.172, TEST_TOL );
    string sym_id1 = vsp::GetXSecParm( xsec_id1, "AllSym" );
    TEST_ASSERT_DELTA( vsp::SetParmVal( sym_id1, 1 ), 1, TEST_TOL );
    string Lstrength_id1 = vsp::GetXSecParm( xsec_id1, "TopLStrength" );
    TEST_ASSERT_DELTA( vsp::SetParmVal( Lstrength_id1, 1.172 ), 1.172, TEST_TOL );
    string Langle_id1 = vsp::GetXSecParm( xsec_id1, "TopLAngle" );
    TEST_ASSERT_DELTA( vsp::SetParmVal( Langle_id1, 0 ), 0, TEST_TOL );
    string Rstrength_id1 = vsp::GetXSecParm( xsec_id1, "TopRStrength" );
    TEST_ASSERT_DELTA( vsp::SetParmVal( Rstrength_id1, 1.172 ), 1.172, TEST_TOL );
    string sym_id2 = vsp::GetXSecParm( xsec_id2, "AllSym" );
    TEST_ASSERT_DELTA( vsp::SetParmVal( sym_id2, 1 ), 1, TEST_TOL );
    string Lstrength_id2 = vsp::GetXSecParm( xsec_id2, "TopLStrength" );
    TEST_ASSERT_DELTA( vsp::SetParmVal( Lstrength_id2, 1.172 ), 1.172, TEST_TOL );
    string Langle_id2 = vsp::GetXSecParm( xsec_id2, "TopLAngle" );
    TEST_ASSERT_DELTA( vsp::SetParmVal( Langle_id2, -90 ), -90, TEST_TOL );
    string Rstrength_id2 = vsp::GetXSecParm( xsec_id2, "TopRStrength" );
    TEST_ASSERT_DELTA( vsp::SetParmVal( Rstrength_id2, 1.172 ), 1.172, TEST_TOL );

    //  Center at orgin:
    TEST_ASSERT_DELTA( vsp::SetParmVal( stack_id, "X_Rel_Location", "XForm", ( -1 * radius ) ), ( -1 * radius ), TEST_TOL );

    //  Set Density to 5:
    int rho = 5;
    TEST_ASSERT_DELTA( vsp::SetParmVal( stack_id, "Density", "Mass_Props", rho ), rho, TEST_TOL );

    //  Increase W Tessellation:
    TEST_ASSERT_DELTA( vsp::SetParmVal( stack_id, "Tess_W", "Shape", 113 ), 113, TEST_TOL );

    //==== Analytical Mass Properties ====//
    double pi = 3.14159265359;
    double I_xx = 8 * pi * pow( radius, 5 ) * rho / 15;
    double I_xy = 0;
    double I_yy = I_xx;
    double I_yz = 0;
    double I_zz = I_yy;
    double I_xz = 0;
    double Cz = 0;
    double Cy = 0;
    double Cx = 0;
    double volume = 4 * pi * pow( radius , 3 ) / 3;
    double mass = rho * volume;

    vsp::Update();
    TEST_ASSERT( !vsp::ErrorMgr.PopErrorAndPrint( stdout ) );    //PopErrorAndPrint returns TRUE if there is an error we want ASSERT to check that this is FALSE

    printf( "   Sphere Radius: %d \n", 2 );
    printf( "   Sphere Density: %d \n", 5 );

    //==== Save Vehicle to File ====//
    printf( "-->Saving VSP model\n" );
    string fname = "APITestSphere.vsp3";
    vsp::WriteVSPFile( fname, vsp::SET_ALL );
    TEST_ASSERT( !vsp::ErrorMgr.PopErrorAndPrint( stdout ) );    //PopErrorAndPrint returns TRUE if there is an error we want ASSERT to check that this is FALSE

    // Set defaults
    vsp::SetAnalysisInputDefaults( "MassProp" );

    // list inputs, type, and current values
    vsp::PrintAnalysisInputs( "MassProp" );

    //==== Execute Mass Properties Analysis ====//
    printf( "-->Executing Mass Properties Analysis\n" );
    string results_id = vsp::ExecAnalysis( "MassProp" );
    TEST_ASSERT( results_id.size() > 0 );
    printf( "COMPLETE\n" );
    TEST_ASSERT( !vsp::ErrorMgr.PopErrorAndPrint( stdout ) );    //PopErrorAndPrint returns TRUE if there is an error we want ASSERT to check that this is FALSE

    vsp::DeleteGeomVec( vsp::GetStringResults( results_id, "Mesh_GeomID" ) );
    TEST_ASSERT( !vsp::ErrorMgr.PopErrorAndPrint( stdout ) );    //PopErrorAndPrint returns TRUE if there is an error we want ASSERT to check that this is FALSE

    // Get & Display Results
    vsp::PrintResults( results_id );

    // Compare to Analytical Solution:
    vector<vec3d> resCG = vsp::GetVec3dResults( results_id, "Total_CG", 0 );
    vector<double> resMass = vsp::GetDoubleResults( results_id, "Total_Mass", 0 );
    vector<double> resVolume = vsp::GetDoubleResults( results_id, "Total_Volume", 0 );
    vector<double> resIxx = vsp::GetDoubleResults( results_id, "Total_Ixx", 0 );
    vector<double> resIxy = vsp::GetDoubleResults( results_id, "Total_Ixy", 0 );
    vector<double> resIxz = vsp::GetDoubleResults( results_id, "Total_Ixz", 0 );
    vector<double> resIyy = vsp::GetDoubleResults( results_id, "Total_Iyy", 0 );
    vector<double> resIyz = vsp::GetDoubleResults( results_id, "Total_Iyz", 0 );
    vector<double> resIzz = vsp::GetDoubleResults( results_id, "Total_Izz", 0 );

    double CGX_TOL = GetCGTol( Cx, mass );
    double CGY_TOL = GetCGTol( Cy, mass );
    double CGZ_TOL = GetCGTol( Cz, mass );
    double IXX_TOL = GetInertiaTol( I_xx, mass );
    double IXY_TOL = GetInertiaTol( I_xy, mass );
    double IXZ_TOL = GetInertiaTol( I_xz, mass );
    double IYY_TOL = GetInertiaTol( I_yy, mass );
    double IYZ_TOL = GetInertiaTol( I_yz, mass );
    double IZZ_TOL = GetInertiaTol( I_zz, mass );
    double MASS_TOL = GetTol( mass );
    double VOLUME_TOL = GetTol( volume );

    TEST_ASSERT_DELTA( resCG[0].x(), Cx, CGX_TOL );
    TEST_ASSERT_DELTA( resCG[0].y(), Cy, CGY_TOL );
    TEST_ASSERT_DELTA( resCG[0].z(), Cz, CGZ_TOL );
    TEST_ASSERT_DELTA( resIxx[0], I_xx, IXX_TOL );
    TEST_ASSERT_DELTA( resIxy[0], I_xy, IXY_TOL );
    TEST_ASSERT_DELTA( resIxz[0], I_xz, IXZ_TOL );
    TEST_ASSERT_DELTA( resIyy[0], I_yy, IYY_TOL );
    TEST_ASSERT_DELTA( resIyz[0], I_yz, IYZ_TOL );
    TEST_ASSERT_DELTA( resIzz[0], I_zz, IZZ_TOL );
    TEST_ASSERT_DELTA( resMass[0], mass, MASS_TOL );
    TEST_ASSERT_DELTA( resVolume[0], volume, VOLUME_TOL );

    printf( "-->Analytical Mass Properties:\n" );
    printf( "   Sphere_CG: %7.3f %7.3f %7.3f \n", Cx, Cy, Cz );
    printf( "   Sphere_Ixx: %7.3f \n", I_xx );
    printf( "   Sphere_Ixy: %7.3f \n", I_xy );
    printf( "   Sphere_Ixz: %7.3f \n", I_xz );
    printf( "   Sphere_Iyy: %7.3f \n", I_yy );
    printf( "   Sphere_Iyz: %7.3f \n", I_yz );
    printf( "   Sphere_Izz: %7.3f \n", I_zz );
    printf( "   Sphere Mass: %7.3f \n", mass );
    printf( "   Sphere Volume: %7.3f \n", volume );

    // Final check for errors
    TEST_ASSERT( !vsp::ErrorMgr.PopErrorAndPrint( stdout ) );    //PopErrorAndPrint returns TRUE if there is an error we want ASSERT to check that this is FALSE
    printf( "\n" );
}

void APITestSuiteMassProp::TestSolidCone()
{
    printf( "APITestSuiteMassProp::TestSolidCone()\n" );
    // make sure setup works
    vsp::VSPCheckSetup();
    vsp::VSPRenew();
    TEST_ASSERT( !vsp::ErrorMgr.PopErrorAndPrint( stdout ) );    //PopErrorAndPrint returns TRUE if there is an error we want ASSERT to check that this is FALSE

    printf( "--> Generating Cone Geometry\n" );

    //==== Add Stack Geom and Set Parameters =====//
    string stack_id = vsp::AddGeom( "STACK" );

    //  Change Type of Middle XSec to Circle and Ends to Points
    vsp::CutXSec( stack_id, 2 );
    vsp::CutXSec( stack_id, 3 );
    string stackxsurf_id = vsp::GetXSecSurf( stack_id, 0 );
    vsp::ChangeXSecShape( stackxsurf_id, 0, vsp::XS_POINT );
    vsp::ChangeXSecShape( stackxsurf_id, 1, vsp::XS_CIRCLE );
    vsp::ChangeXSecShape( stackxsurf_id, 2, vsp::XS_POINT );

    //  Get XSec IDs:
    string xsec_id0 = vsp::GetXSec( stackxsurf_id, 0 );
    string xsec_id1 = vsp::GetXSec( stackxsurf_id, 1 );
    string xsec_id2 = vsp::GetXSec( stackxsurf_id, 2 );

    //  Set Radius to 2:
    double radius = 2;
    string dia_id1 = vsp::GetXSecParm( xsec_id1, "Circle_Diameter" );

    TEST_ASSERT_DELTA( vsp::SetParmVal( dia_id1, radius * 2 ), radius * 2, TEST_TOL );

    //  Set Beginning and End of Cone
    double length = 12;
    string xloc_id1 = vsp::GetXSecParm( xsec_id1, "XDelta" );
    TEST_ASSERT_DELTA( vsp::SetParmVal( xloc_id1, length ), length, TEST_TOL );
    string xloc_id2 = vsp::GetXSecParm( xsec_id2, "XDelta" );
    TEST_ASSERT_DELTA( vsp::SetParmVal( xloc_id2, 0.0 ), 0.0, TEST_TOL );

    //  Straighten Cone:
    string sym_id0 = vsp::GetXSecParm( xsec_id0, "AllSym" );
    TEST_ASSERT_DELTA( vsp::SetParmVal( sym_id0, 1 ), 1, TEST_TOL );
    string Lstrength_id0 = vsp::GetXSecParm( xsec_id0, "TopLStrength" );
    TEST_ASSERT_DELTA( vsp::SetParmVal( Lstrength_id0, 0 ), 0, TEST_TOL )
    string Rstrength_id0 = vsp::GetXSecParm( xsec_id0, "TopRStrength" );
    TEST_ASSERT_DELTA( vsp::SetParmVal( Rstrength_id0, 0 ), 0, TEST_TOL )
    string sym_id1 = vsp::GetXSecParm( xsec_id1, "AllSym" );
    TEST_ASSERT_DELTA( vsp::SetParmVal( sym_id1, 1 ), 1, TEST_TOL )
    string Lstrength_id1 = vsp::GetXSecParm( xsec_id1, "TopLStrength" );
    TEST_ASSERT_DELTA( vsp::SetParmVal( Lstrength_id1, 0 ), 0, TEST_TOL )
    string Rstrength_id1 = vsp::GetXSecParm( xsec_id1, "TopRStrength" );
    TEST_ASSERT_DELTA( vsp::SetParmVal( Rstrength_id1, 0 ), 0, TEST_TOL )
    string sym_id2 = vsp::GetXSecParm( xsec_id2, "AllSym" );
    TEST_ASSERT_DELTA( vsp::SetParmVal( sym_id2, 1 ), 1, TEST_TOL )
    string Lstrength_id2 = vsp::GetXSecParm( xsec_id2, "TopLStrength" );
    TEST_ASSERT_DELTA( vsp::SetParmVal( Lstrength_id2, 0 ), 0, TEST_TOL )
    string Rstrength_id2 = vsp::GetXSecParm( xsec_id2, "TopRStrength" );
    TEST_ASSERT_DELTA( vsp::SetParmVal( Rstrength_id2, 0 ), 0, TEST_TOL )

    //  Set Density:
    double rho = 5;
    TEST_ASSERT_DELTA( vsp::SetParmVal( stack_id, "Density", "Mass_Props", rho ), rho, TEST_TOL )

    //  Increase W Tessellation:
    TEST_ASSERT_DELTA( vsp::SetParmVal( stack_id, "Tess_W", "Shape", 73 ), 73, TEST_TOL )

    //  Increase U Tessellation
    string xutess_id0 = vsp::GetXSecParm( xsec_id0, "SectTess_U" );
    TEST_ASSERT_DELTA( vsp::SetParmVal( xutess_id0, 20 ), 20, TEST_TOL );
    string xutess_id1 = vsp::GetXSecParm( xsec_id1, "SectTess_U" );
    TEST_ASSERT_DELTA( vsp::SetParmVal( xutess_id1, 20 ), 20, TEST_TOL );
    string xutess_id2 = vsp::GetXSecParm( xsec_id2, "SectTess_U" );
    TEST_ASSERT_DELTA( vsp::SetParmVal( xutess_id2, 20 ), 20, TEST_TOL );

    //==== Analytical Mass Properties ====//
    double pi = 3.14159265359;
    double I_xx = pi * pow( radius, 4 ) * length * rho / 10;
    double I_xy = 0;
    double I_yy = pi * pow( radius, 2 ) * length * rho * ( 4 * pow( radius, 2 ) + pow( length, 2 ) ) / 80;
    double I_yz = 0;
    double I_zz = I_yy;
    double I_xz = 0;
    double Cz = 0;
    double Cy = 0;
    double Cx = 3 * length / 4;
    double volume = pi * pow( radius, 2 ) * length / 3;
    double mass = rho * volume;

    vsp::Update();
    TEST_ASSERT( !vsp::ErrorMgr.PopErrorAndPrint( stdout ) );    //PopErrorAndPrint returns TRUE if there is an error we want ASSERT to check that this is FALSE

    printf( "   Cone Radius: %d \n", 2 );
    printf( "   Cone Length: %f \n", length );
    printf( "   Cone Density: %d \n", 5 );

    //==== Save Vehicle to File ====//
    printf( "-->Saving VSP model\n" );
    string fname = "APITestSolidCone.vsp3";
    vsp::WriteVSPFile( fname, vsp::SET_ALL );
    TEST_ASSERT( !vsp::ErrorMgr.PopErrorAndPrint( stdout ) );    //PopErrorAndPrint returns TRUE if there is an error we want ASSERT to check that this is FALSE

    // Set defaults
    vsp::SetAnalysisInputDefaults( "MassProp" );

    // list inputs, type, and current values
    vsp::PrintAnalysisInputs( "MassProp" );

    //==== Execute Mass Properties Analysis ====//
    printf( "-->Executing Mass Properties Analysis\n" );
    string results_id = vsp::ExecAnalysis( "MassProp" );
    TEST_ASSERT( results_id.size() > 0 );
    printf( "COMPLETE\n" );
    TEST_ASSERT( !vsp::ErrorMgr.PopErrorAndPrint( stdout ) );    //PopErrorAndPrint returns TRUE if there is an error we want ASSERT to check that this is FALSE

    vsp::DeleteGeomVec( vsp::GetStringResults( results_id, "Mesh_GeomID" ) );
    TEST_ASSERT( !vsp::ErrorMgr.PopErrorAndPrint( stdout ) );    //PopErrorAndPrint returns TRUE if there is an error we want ASSERT to check that this is FALSE

    // Get & Display Results
    vsp::PrintResults( results_id );

    // Compare to Analytical Solution:
    vector<vec3d> resCG = vsp::GetVec3dResults( results_id, "Total_CG", 0 );
    vector<double> resMass = vsp::GetDoubleResults( results_id, "Total_Mass", 0 );
    vector<double> resVolume = vsp::GetDoubleResults( results_id, "Total_Volume", 0 );
    vector<double> resIxx = vsp::GetDoubleResults( results_id, "Total_Ixx", 0 );
    vector<double> resIxy = vsp::GetDoubleResults( results_id, "Total_Ixy", 0 );
    vector<double> resIxz = vsp::GetDoubleResults( results_id, "Total_Ixz", 0 );
    vector<double> resIyy = vsp::GetDoubleResults( results_id, "Total_Iyy", 0 );
    vector<double> resIyz = vsp::GetDoubleResults( results_id, "Total_Iyz", 0 );
    vector<double> resIzz = vsp::GetDoubleResults( results_id, "Total_Izz", 0 );

    double CGX_TOL = GetCGTol( Cx, mass );
    double CGY_TOL = GetCGTol( Cy, mass );
    double CGZ_TOL = GetCGTol( Cz, mass );
    double IXX_TOL = GetInertiaTol( I_xx, mass );
    double IXY_TOL = GetInertiaTol( I_xy, mass );
    double IXZ_TOL = GetInertiaTol( I_xz, mass );
    double IYY_TOL = GetInertiaTol( I_yy, mass );
    double IYZ_TOL = GetInertiaTol( I_yz, mass );
    double IZZ_TOL = GetInertiaTol( I_zz, mass );
    double MASS_TOL = GetTol( mass );
    double VOLUME_TOL = GetTol( volume );

    TEST_ASSERT_DELTA( resCG[0].x(), Cx, CGX_TOL );
    TEST_ASSERT_DELTA( resCG[0].y(), Cy, CGY_TOL );
    TEST_ASSERT_DELTA( resCG[0].z(), Cz, CGZ_TOL );
    TEST_ASSERT_DELTA( resIxx[0], I_xx, IXX_TOL );
    TEST_ASSERT_DELTA( resIxy[0], I_xy, IXY_TOL );
    TEST_ASSERT_DELTA( resIxz[0], I_xz, IXZ_TOL );
    TEST_ASSERT_DELTA( resIyy[0], I_yy, IYY_TOL );
    TEST_ASSERT_DELTA( resIyz[0], I_yz, IYZ_TOL );
    TEST_ASSERT_DELTA( resIzz[0], I_zz, IZZ_TOL );
    TEST_ASSERT_DELTA( resMass[0], mass, MASS_TOL );
    TEST_ASSERT_DELTA( resVolume[0], volume, VOLUME_TOL );

    printf( "-->Analytical Mass Properties:\n" );
    printf( "   Cone_CG: %7.3f %7.3f %7.3f \n", Cx, Cy, Cz );
    printf( "   Cone_Ixx: %7.3f \n", I_xx );
    printf( "   Cone_Ixy: %7.3f \n", I_xy );
    printf( "   Cone_Ixz: %7.3f \n", I_xz );
    printf( "   Cone_Iyy: %7.3f \n", I_yy );
    printf( "   Cone_Iyz: %7.3f \n", I_yz );
    printf( "   Cone_Izz: %7.3f \n", I_zz );
    printf( "   Cone Mass: %7.3f \n", mass );
    printf( "   Cone Volume: %7.3f \n", volume );

    // Final check for errors
    TEST_ASSERT( !vsp::ErrorMgr.PopErrorAndPrint( stdout ) );    //PopErrorAndPrint returns TRUE if there is an error we want ASSERT to check that this is FALSE
    printf( "\n" );
}

void APITestSuiteMassProp::TestShellCone()
{
    printf( "APITestSuiteMassProp::TestShellCone()\n" );
    // make sure setup works
    vsp::VSPCheckSetup();
    vsp::VSPRenew();
    TEST_ASSERT( !vsp::ErrorMgr.PopErrorAndPrint( stdout ) );    //PopErrorAndPrint returns TRUE if there is an error we want ASSERT to check that this is FALSE

    printf( "--> Generating Shell Cone Geometry\n" );

    //==== Add Stack Geom and Set Parameters =====//
    string stack_id = vsp::AddGeom( "STACK" );

    //  Change Type of Middle XSec to Circle and Ends to Points
    vsp::CutXSec( stack_id, 2 );
    string stackxsurf_id = vsp::GetXSecSurf( stack_id, 0 );
    vsp::ChangeXSecShape( stackxsurf_id, 0, vsp::XS_POINT );
    vsp::ChangeXSecShape( stackxsurf_id, 1, vsp::XS_CIRCLE );
    vsp::ChangeXSecShape( stackxsurf_id, 3, vsp::XS_CIRCLE );

    //  Get XSec IDs:
    string xsec_id0 = vsp::GetXSec( stackxsurf_id, 0 );
    string xsec_id1 = vsp::GetXSec( stackxsurf_id, 1 );
    string xsec_id2 = vsp::GetXSec( stackxsurf_id, 3 );

    //  Set Radius to 2:
    double radius = 2;
    string dia_id1 = vsp::GetXSecParm( xsec_id1, "Circle_Diameter" );
    string dia_id2 = vsp::GetXSecParm( xsec_id2, "Circle_Diameter" );

    vsp::SetParmVal( dia_id1, radius * 2 );
    vsp::SetParmVal( dia_id2, 0 );

    //  Set Beginning and End of Cone
    double length = 12;
    string xloc_id1 = vsp::GetXSecParm( xsec_id1, "XDelta" );
    vsp::SetParmVal( xloc_id1, length );
    string xloc_id2 = vsp::GetXSecParm( xsec_id2, "XDelta" );
    vsp::SetParmVal( xloc_id2, 0 );

    //  Straighten Cone:
    string sym_id0 = vsp::GetXSecParm( xsec_id0, "AllSym" );
    vsp::SetParmVal( sym_id0, 1 );
    string Lstrength_id0 = vsp::GetXSecParm( xsec_id0, "TopLStrength" );
    vsp::SetParmVal( Lstrength_id0, 0 );
    string Rstrength_id0 = vsp::GetXSecParm( xsec_id0, "TopRStrength" );
    vsp::SetParmVal( Rstrength_id0, 0 );
    string sym_id1 = vsp::GetXSecParm( xsec_id1, "AllSym" );
    vsp::SetParmVal( sym_id1, 1 );
    string Lstrength_id1 = vsp::GetXSecParm( xsec_id1, "TopLStrength" );
    vsp::SetParmVal( Lstrength_id1, 0 );
    string Rstrength_id1 = vsp::GetXSecParm( xsec_id1, "TopRStrength" );
    vsp::SetParmVal( Rstrength_id1, 0 );
    string sym_id2 = vsp::GetXSecParm( xsec_id2, "AllSym" );
    vsp::SetParmVal( sym_id2, 1 );
    string Lstrength_id2 = vsp::GetXSecParm( xsec_id2, "TopLStrength" );
    vsp::SetParmVal( Lstrength_id2, 0 );
    string Rstrength_id2 = vsp::GetXSecParm( xsec_id2, "TopRStrength" );
    vsp::SetParmVal( Rstrength_id2, 0 );

    //   Open End of Cone
    vsp::CutXSec( stack_id, 2 );

    //  Set Thin Shell Flag:
    vsp::SetParmVal( stack_id, "Shell_Flag", "Mass_Props", 1 );

    //  Set Mass/Area:
    double rho_A = 0.5;
    vsp::SetParmVal( stack_id, "Mass_Area", "Mass_Props", rho_A );

    //  Set Density:
    double rho = 0;
    vsp::SetParmVal( stack_id, "Density", "Mass_Props", rho );

    //  Increase W Tessellation:
    vsp::SetParmVal( stack_id, "Tess_W", "Shape", 73 );

    //  Increase U Tessellation
    string xutess_id0 = vsp::GetXSecParm( xsec_id0, "SectTess_U" );
    vsp::SetParmVal( xutess_id0, 20 );
    string xutess_id1 = vsp::GetXSecParm( xsec_id1, "SectTess_U" );
    vsp::SetParmVal( xutess_id1, 20 );

    //==== Analytical Mass Properties ====//
    double diskSA = PI * pow( radius, 2.0 );
    double shellSA = PI * radius * sqrt( pow( length, 2.0 ) + pow( radius, 2.0 ) );
    double SA = diskSA + shellSA;
    double diskMass = rho_A * diskSA;
    double shellMass = rho_A * shellSA;
    double shellIxx = shellMass * pow( radius, 2 ) / 2;
    double shellIyy = ( shellMass * pow( radius, 2 ) / 4 ) + ( shellMass * pow( length, 2 ) / 18 ) + shellMass * pow( ( 3 * length / 4 - 8.564 ), 2 );
    double diskIxx = diskMass * pow( radius, 2 ) / 2;
    double diskIyy = diskMass * pow( radius, 2 ) / 4 + diskMass * pow( 3.436, 2 );
    double mass = diskMass + shellMass;
    double I_xx = shellIxx + diskIxx;
    double I_xy = 0;
    double I_yy = shellIyy + diskIyy;
    double I_yz = 0;
    double I_zz = I_yy;
    double I_xz = 0;
    double Cz = 0;
    double Cy = 0;
    double Cx = ( 3 * length / 4 ) - 0.436;

    vsp::Update();
    TEST_ASSERT( !vsp::ErrorMgr.PopErrorAndPrint( stdout ) );    //PopErrorAndPrint returns TRUE if there is an error we want ASSERT to check that this is FALSE

    printf( "   Cone Radius: %d \n", 2 );
    printf( "   Cone Length: %f \n", length );
    printf( "   Cone Density: %7.3f \n", rho );
    printf( "   Cone Area Density: %7.3f \n", rho_A );

    //==== Save Vehicle to File ====//
    printf( "-->Saving VSP model\n" );
    string fname = "APITestShellCone.vsp3";
    vsp::WriteVSPFile( fname, vsp::SET_ALL );
    TEST_ASSERT( !vsp::ErrorMgr.PopErrorAndPrint( stdout ) );    //PopErrorAndPrint returns TRUE if there is an error we want ASSERT to check that this is FALSE

    // Set defaults
    vsp::SetAnalysisInputDefaults( "MassProp" );

    // list inputs, type, and current values
    vsp::PrintAnalysisInputs( "MassProp" );

    //==== Execute Mass Properties Analysis ====//
    printf( "-->Executing Mass Properties Analysis\n" );
    string results_id = vsp::ExecAnalysis( "MassProp" );
    TEST_ASSERT( results_id.size() > 0 );
    printf( "COMPLETE\n" );
    TEST_ASSERT( !vsp::ErrorMgr.PopErrorAndPrint( stdout ) );    //PopErrorAndPrint returns TRUE if there is an error we want ASSERT to check that this is FALSE

    vsp::DeleteGeomVec( vsp::GetStringResults( results_id, "Mesh_GeomID" ) );
    TEST_ASSERT( !vsp::ErrorMgr.PopErrorAndPrint( stdout ) );    //PopErrorAndPrint returns TRUE if there is an error we want ASSERT to check that this is FALSE

    // Get & Display Results
    vsp::PrintResults( results_id );

    // Compare to Analytical Solution:
    vector<vec3d> resCG = vsp::GetVec3dResults( results_id, "Total_CG", 0 );
    vector<double> resMass = vsp::GetDoubleResults( results_id, "Total_Mass", 0 );
    vector<double> resVolume = vsp::GetDoubleResults( results_id, "Total_Volume", 0 );
    vector<double> resIxx = vsp::GetDoubleResults( results_id, "Total_Ixx", 0 );
    vector<double> resIxy = vsp::GetDoubleResults( results_id, "Total_Ixy", 0 );
    vector<double> resIxz = vsp::GetDoubleResults( results_id, "Total_Ixz", 0 );
    vector<double> resIyy = vsp::GetDoubleResults( results_id, "Total_Iyy", 0 );
    vector<double> resIyz = vsp::GetDoubleResults( results_id, "Total_Iyz", 0 );
    vector<double> resIzz = vsp::GetDoubleResults( results_id, "Total_Izz", 0 );

    double CGX_TOL = GetCGTol( Cx, mass );
    double CGY_TOL = GetCGTol( Cy, mass );
    double CGZ_TOL = GetCGTol( Cz, mass );
    double IXX_TOL = GetInertiaTol( I_xx, mass );
    double IXY_TOL = GetInertiaTol( I_xy, mass );
    double IXZ_TOL = GetInertiaTol( I_xz, mass );
    double IYY_TOL = GetInertiaTol( I_yy, mass );
    double IYZ_TOL = GetInertiaTol( I_yz, mass );
    double IZZ_TOL = GetInertiaTol( I_zz, mass );
    double MASS_TOL = GetTol( mass );

    TEST_ASSERT_DELTA( resCG[0].x(), Cx, CGX_TOL );
    TEST_ASSERT_DELTA( resCG[0].y(), Cy, CGY_TOL );
    TEST_ASSERT_DELTA( resCG[0].z(), Cz, CGZ_TOL );
    TEST_ASSERT_DELTA( resIxx[0], I_xx, IXX_TOL );
    TEST_ASSERT_DELTA( resIxy[0], I_xy, IXY_TOL );
    TEST_ASSERT_DELTA( resIxz[0], I_xz, IXZ_TOL );
    TEST_ASSERT_DELTA( resIyy[0], I_yy, IYY_TOL );
    TEST_ASSERT_DELTA( resIyz[0], I_yz, IYZ_TOL );
    TEST_ASSERT_DELTA( resIzz[0], I_zz, IZZ_TOL );
    TEST_ASSERT_DELTA( resMass[0], mass, MASS_TOL );

    printf( "-->Analytical Mass Properties:\n" );
    printf( "   Cone_CG: %7.3f %7.3f %7.3f \n", Cx, Cy, Cz );
    printf( "   Cone_Ixx: %7.3f \n", I_xx );
    printf( "   Cone_Ixy: %7.3f \n", I_xy );
    printf( "   Cone_Ixz: %7.3f \n", I_xz );
    printf( "   Cone_Iyy: %7.3f \n", I_yy );
    printf( "   Cone_Iyz: %7.3f \n", I_yz );
    printf( "   Cone_Izz: %7.3f \n", I_zz );
    printf( "   Cone Mass: %7.3f \n", mass );
    printf( "   Cone Surface Area: %7.3f \n", SA );

    // Final check for errors
    TEST_ASSERT( !vsp::ErrorMgr.PopErrorAndPrint( stdout ) );    //PopErrorAndPrint returns TRUE if there is an error we want ASSERT to check that this is FALSE
    printf( "\n" );
}

void APITestSuiteMassProp::TestRectangularPrism()
{
    printf( "APITestSuiteMassProp::TestRectangularPrism()\n" );
    // make sure setup works
    vsp::VSPCheckSetup();
    vsp::VSPRenew();
    TEST_ASSERT( !vsp::ErrorMgr.PopErrorAndPrint( stdout ) );    //PopErrorAndPrint returns TRUE if there is an error we want ASSERT to check that this is FALSE

    printf( "--> Generating Rectangular Prism Geometry\n" );

    //==== Add Stack Geom and Set Parameters =====//
    string stack_id = vsp::AddGeom( "STACK" );

    //  Change Type of Middle XSec to Rounded Rectangles and Ends to Points
    vsp::CutXSec( stack_id, 2 );
    string stackxsurf_id = vsp::GetXSecSurf( stack_id, 0 );
    vsp::ChangeXSecShape( stackxsurf_id, 0, vsp::XS_POINT );
    vsp::ChangeXSecShape( stackxsurf_id, 1, vsp::XS_ROUNDED_RECTANGLE );
    vsp::ChangeXSecShape( stackxsurf_id, 2, vsp::XS_ROUNDED_RECTANGLE );
    vsp::ChangeXSecShape( stackxsurf_id, 3, vsp::XS_POINT );

    //  Get XSec IDs:
    string xsec_id0 = vsp::GetXSec( stackxsurf_id, 0 );
    string xsec_id1 = vsp::GetXSec( stackxsurf_id, 1 );
    string xsec_id2 = vsp::GetXSec( stackxsurf_id, 2 );
    string xsec_id3 = vsp::GetXSec( stackxsurf_id, 3 );

    //  Set Height to 2:
    double height = 2;
    string height_id1 = vsp::GetXSecParm( xsec_id1, "RoundedRect_Height" );
    string height_id2 = vsp::GetXSecParm( xsec_id2, "RoundedRect_Height" );

    TEST_ASSERT_DELTA( vsp::SetParmVal( height_id1, height ), height, TEST_TOL );
    TEST_ASSERT_DELTA( vsp::SetParmVal( height_id2, height ), height, TEST_TOL );

    //  Set Width to 4:
    double width = 4;
    string width_id1 = vsp::GetXSecParm( xsec_id1, "RoundedRect_Width" );
    string width_id2 = vsp::GetXSecParm( xsec_id2, "RoundedRect_Width" );

    TEST_ASSERT_DELTA( vsp::SetParmVal( width_id1, width ), width, TEST_TOL );
    TEST_ASSERT_DELTA( vsp::SetParmVal( width_id2, width ), width, TEST_TOL );

    // Sharpen Corners
    string corner_id1 = vsp::GetXSecParm( xsec_id1, "RoundRectXSec_Radius" );
    string corner_id2 = vsp::GetXSecParm( xsec_id2, "RoundRectXSec_Radius" );

    TEST_ASSERT_DELTA( vsp::SetParmVal( corner_id1, 0 ), 0, TEST_TOL );
    TEST_ASSERT_DELTA( vsp::SetParmVal( corner_id2, 0 ), 0, TEST_TOL );

    //  Set Beginning and End of Rectangular Prism
    double length = 12;
    string xloc_id1 = vsp::GetXSecParm( xsec_id1, "XDelta" );
    TEST_ASSERT_DELTA( vsp::SetParmVal( xloc_id1, 0.0 ), 0.0, TEST_TOL );
    string xloc_id2 = vsp::GetXSecParm( xsec_id2, "XDelta" );
    TEST_ASSERT_DELTA( vsp::SetParmVal( xloc_id2, length ), length, TEST_TOL );
    string xloc_id3 = vsp::GetXSecParm( xsec_id3, "XDelta" );
    TEST_ASSERT_DELTA( vsp::SetParmVal( xloc_id3, 0.0 ), 0.0, TEST_TOL );

    //  Straighten Rectangular Prism:
    string sym_id0 = vsp::GetXSecParm( xsec_id0, "AllSym" );
    TEST_ASSERT_DELTA( vsp::SetParmVal( sym_id0, 1 ), 1, TEST_TOL );
    string Lstrength_id0 = vsp::GetXSecParm( xsec_id0, "TopLStrength" );
    TEST_ASSERT_DELTA( vsp::SetParmVal( Lstrength_id0, 0 ), 0, TEST_TOL );
    string Rstrength_id0 = vsp::GetXSecParm( xsec_id0, "TopRStrength" );
    TEST_ASSERT_DELTA( vsp::SetParmVal( Rstrength_id0, 0 ), 0, TEST_TOL );
    string sym_id1 = vsp::GetXSecParm( xsec_id1, "AllSym" );
    TEST_ASSERT_DELTA( vsp::SetParmVal( sym_id1, 1 ), 1, TEST_TOL );
    string Lstrength_id1 = vsp::GetXSecParm( xsec_id1, "TopLStrength" );
    TEST_ASSERT_DELTA( vsp::SetParmVal( Lstrength_id1, 0 ), 0, TEST_TOL );
    string Rstrength_id1 = vsp::GetXSecParm( xsec_id1, "TopRStrength" );
    TEST_ASSERT_DELTA( vsp::SetParmVal( Rstrength_id1, 0 ), 0, TEST_TOL );
    string sym_id2 = vsp::GetXSecParm( xsec_id2, "AllSym" );
    TEST_ASSERT_DELTA( vsp::SetParmVal( sym_id2, 1 ), 1, TEST_TOL );
    string Lstrength_id2 = vsp::GetXSecParm( xsec_id2, "TopLStrength" );
    TEST_ASSERT_DELTA( vsp::SetParmVal( Lstrength_id2, 0 ), 0, TEST_TOL );
    string Rstrength_id2 = vsp::GetXSecParm( xsec_id2, "TopRStrength" );
    TEST_ASSERT_DELTA( vsp::SetParmVal( Rstrength_id2, 0 ), 0, TEST_TOL );
    string sym_id3 = vsp::GetXSecParm( xsec_id3, "AllSym" );
    TEST_ASSERT_DELTA( vsp::SetParmVal( sym_id3, 1 ), 1, TEST_TOL );
    string Lstrength_id3 = vsp::GetXSecParm( xsec_id3, "TopLStrength" );
    TEST_ASSERT_DELTA( vsp::SetParmVal( Lstrength_id3, 0 ), 0, TEST_TOL );
    string Rstrength_id3 = vsp::GetXSecParm( xsec_id3, "TopRStrength" );
    TEST_ASSERT_DELTA( vsp::SetParmVal( Rstrength_id3, 0 ), 0, TEST_TOL );

    //  Set Density to 5:
    double rho = 5;
    TEST_ASSERT_DELTA( vsp::SetParmVal( stack_id, "Density", "Mass_Props", rho ), rho, TEST_TOL );

    //  Increase W Tessellation:
    TEST_ASSERT_DELTA( vsp::SetParmVal( stack_id, "Tess_W", "Shape", 81 ), 81, TEST_TOL );

    //  Increase U Tessellation
    string xutess_id0 = vsp::GetXSecParm( xsec_id0, "SectTess_U" );
    TEST_ASSERT_DELTA( vsp::SetParmVal( xutess_id0, 20 ), 20, TEST_TOL );
    string xutess_id1 = vsp::GetXSecParm( xsec_id1, "SectTess_U" );
    TEST_ASSERT_DELTA( vsp::SetParmVal( xutess_id1, 20 ), 20, TEST_TOL );
    string xutess_id2 = vsp::GetXSecParm( xsec_id2, "SectTess_U" );
    TEST_ASSERT_DELTA( vsp::SetParmVal( xutess_id2, 20 ), 20, TEST_TOL );
    string xutess_id3 = vsp::GetXSecParm( xsec_id3, "SectTess_U" );
    TEST_ASSERT_DELTA( vsp::SetParmVal( xutess_id3, 20 ), 20, TEST_TOL );

    //==== Analytical Mass Properties ====//
    double pi = 3.14159265359;
    double I_xx = rho * length * width * height * ( pow( width, 2 ) + pow( height, 2 ) ) / 12;
    double I_xy = 0;
    double I_yy = rho * length * width * height * ( pow( length, 2 ) + pow( height, 2 ) ) / 12;
    double I_yz = 0;
    double I_zz = rho * length * width * height * ( pow( length, 2 ) + pow( width, 2 ) ) / 12;
    double I_xz = 0;
    double Cz = 0;
    double Cy = 0;
    double Cx = length / 2;
    double volume = length * width * height;
    double mass = rho * volume;

    vsp::Update();
    TEST_ASSERT( !vsp::ErrorMgr.PopErrorAndPrint( stdout ) );    //PopErrorAndPrint returns TRUE if there is an error we want ASSERT to check that this is FALSE

    printf( "   Rectangular Prism Width: %f \n", width );
    printf( "   Rectangular Prism Height: %f \n", height );
    printf( "   Rectangular Prism Length: %f \n", length );
    printf( "   Rectangular Prism Density: %f \n", rho );

    //==== Save Vehicle to File ====//
    printf( "-->Saving VSP model\n" );
    string fname = "APITestRectangularPrism.vsp3";
    vsp::WriteVSPFile( fname, vsp::SET_ALL );
    TEST_ASSERT( !vsp::ErrorMgr.PopErrorAndPrint( stdout ) );    //PopErrorAndPrint returns TRUE if there is an error we want ASSERT to check that this is FALSE

    // Set defaults
    vsp::SetAnalysisInputDefaults( "MassProp" );

    // list inputs, type, and current values
    vsp::PrintAnalysisInputs( "MassProp" );

    //==== Execute Mass Properties Analysis ====//
    printf( "-->Executing Mass Properties Analysis\n" );
    string results_id = vsp::ExecAnalysis( "MassProp" );
    TEST_ASSERT( results_id.size() > 0 );
    printf( "COMPLETE\n" );
    TEST_ASSERT( !vsp::ErrorMgr.PopErrorAndPrint( stdout ) );    //PopErrorAndPrint returns TRUE if there is an error we want ASSERT to check that this is FALSE

    vsp::DeleteGeomVec( vsp::GetStringResults( results_id, "Mesh_GeomID" ) );
    TEST_ASSERT( !vsp::ErrorMgr.PopErrorAndPrint( stdout ) );    //PopErrorAndPrint returns TRUE if there is an error we want ASSERT to check that this is FALSE

    // Get & Display Results
    vsp::PrintResults( results_id );

    // Compare to Analytical Solution:
    vector<vec3d> resCG = vsp::GetVec3dResults( results_id, "Total_CG", 0 );
    vector<double> resMass = vsp::GetDoubleResults( results_id, "Total_Mass", 0 );
    vector<double> resVolume = vsp::GetDoubleResults( results_id, "Total_Volume", 0 );
    vector<double> resIxx = vsp::GetDoubleResults( results_id, "Total_Ixx", 0 );
    vector<double> resIxy = vsp::GetDoubleResults( results_id, "Total_Ixy", 0 );
    vector<double> resIxz = vsp::GetDoubleResults( results_id, "Total_Ixz", 0 );
    vector<double> resIyy = vsp::GetDoubleResults( results_id, "Total_Iyy", 0 );
    vector<double> resIyz = vsp::GetDoubleResults( results_id, "Total_Iyz", 0 );
    vector<double> resIzz = vsp::GetDoubleResults( results_id, "Total_Izz", 0 );

    double CGX_TOL = GetCGTol( Cx, mass );
    double CGY_TOL = GetCGTol( Cy, mass );
    double CGZ_TOL = GetCGTol( Cz, mass );
    double IXX_TOL = GetInertiaTol( I_xx, mass );
    double IXY_TOL = GetInertiaTol( I_xy, mass );
    double IXZ_TOL = GetInertiaTol( I_xz, mass );
    double IYY_TOL = GetInertiaTol( I_yy, mass );
    double IYZ_TOL = GetInertiaTol( I_yz, mass );
    double IZZ_TOL = GetInertiaTol( I_zz, mass );
    double MASS_TOL = GetTol( mass );
    double VOLUME_TOL = GetTol( volume );

    TEST_ASSERT_DELTA( resCG[0].x(), Cx, CGX_TOL );
    TEST_ASSERT_DELTA( resCG[0].y(), Cy, CGY_TOL );
    TEST_ASSERT_DELTA( resCG[0].z(), Cz, CGZ_TOL );
    TEST_ASSERT_DELTA( resIxx[0], I_xx, IXX_TOL );
    TEST_ASSERT_DELTA( resIxy[0], I_xy, IXY_TOL );
    TEST_ASSERT_DELTA( resIxz[0], I_xz, IXZ_TOL );
    TEST_ASSERT_DELTA( resIyy[0], I_yy, IYY_TOL );
    TEST_ASSERT_DELTA( resIyz[0], I_yz, IYZ_TOL );
    TEST_ASSERT_DELTA( resIzz[0], I_zz, IZZ_TOL );
    TEST_ASSERT_DELTA( resMass[0], mass, MASS_TOL );
    TEST_ASSERT_DELTA( resVolume[0], volume, VOLUME_TOL );

    printf( "-->Analytical Mass Properties:\n" );
    printf( "   RectangularPrism_CG: %7.3f %7.3f %7.3f \n", Cx, Cy, Cz );
    printf( "   RectangularPrism_Ixx: %7.3f \n", I_xx );
    printf( "   RectangularPrism_Ixy: %7.3f \n", I_xy );
    printf( "   RectangularPrism_Ixz: %7.3f \n", I_xz );
    printf( "   RectangularPrism_Iyy: %7.3f \n", I_yy );
    printf( "   RectangularPrism_Iyz: %7.3f \n", I_yz );
    printf( "   RectangularPrism_Izz: %7.3f \n", I_zz );
    printf( "   Rectangular Prism Mass: %7.3f \n", mass );
    printf( "   Rectangular Prism Volume: %7.3f \n", volume );

    // Final check for errors
    TEST_ASSERT( !vsp::ErrorMgr.PopErrorAndPrint( stdout ) );    //PopErrorAndPrint returns TRUE if there is an error we want ASSERT to check that this is FALSE
    printf( "\n" );
}