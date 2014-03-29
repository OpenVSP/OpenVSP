//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//

// Leak Detection http://www.codeproject.com/KB/applications/visualleakdetector.aspx
//#include "vld.h"

#include <stdio.h>

#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>

#ifdef WIN32
#define NOMINMAX
#include <windows.h>
#endif

#include <libxml/tree.h>
#include <libxml/nanohttp.h>


#ifdef WIN32
#include <direct.h>
#define GetCurrentDir _getcwd
#else
#include <unistd.h>
#define GetCurrentDir getcwd
#endif


#include "main.h"
#include "VSP_Geom_API.h"

#include "EventMgr.h"


void vsp_exit()
{

    exit( 0 );
}


//========================================================//
//========================================================//
//========================= Main =========================//

int main( int argc, char** argv )
{
//==== Use Case 1 =====//

    //==== Create/Delete/Copy/Paste Geometry ====//
    vsp::VSPCheckSetup();
    vsp::ErrorMgr.PopErrorAndPrint( stdout );

    vector<string> types = vsp::GetGeomTypes( );
    vsp::ErrorMgr.PopErrorAndPrint( stdout );

    //==== Print Out All Available Geom Types ====//
    printf( "All available Geom types.\n" );
    for ( int i = 0 ; i < ( int )types.size() ; i++ )
    {
        printf( "Type %d = %s \n", i, types[i].c_str() );
    }

    //==== Add Fuselage Geom =====//
    string fuse_id = vsp::AddGeom( "FUSELAGE" );
    vsp::ErrorMgr.PopErrorAndPrint( stdout );

    //==== Add Pod Geom =====//
    string pod_id = vsp::AddGeom( "POD", fuse_id );
    vsp::ErrorMgr.PopErrorAndPrint( stdout );

    //==== Set Name ====//
    vsp::SetGeomName( pod_id, "Pod" );

    //==== Change Length ====//
    string len_id = vsp::GetParm( pod_id, "Length", "Design" );
    vsp::SetParmVal( len_id, 7.0 );

    //==== Change Finess Ration
    vsp::SetParmVal( pod_id, "FineRatio", "Design", 10.0 );

    //==== Change Y Location ====//
    string y_loc_id = vsp::GetParm( pod_id, "Y_Location", "XForm" );
    vsp::SetParmVal( y_loc_id, 1.0 );

    //==== Change X Location ====//
    vsp::SetParmVal( pod_id, "X_Location", "XForm", 3.0 );

    //==== Change Symmetry =====//
    string sym_flag_id = vsp::GetParm( pod_id, "Sym_Planar_Flag", "Sym" );
    vsp::SetParmValUpdate( sym_flag_id, vsp::SYM_XZ  );

    //==== Copy Pod Geom =====//
    vsp::CopyGeomToClipboard( pod_id );
    vsp::PasteGeomClipboard( fuse_id );         // Make fuse_id parent

    //==== Set Name ====//
    vsp::SetGeomName( pod_id, "Original_Pod" );

    string second_pod_id = vsp::FindGeom( "Pod", 0 );

    //==== Change Y Location ====//
    vsp::SetParmVal( second_pod_id, "Sym_Planar_Flag", "Sym", 0 );
    vsp::SetParmVal( second_pod_id, "Y_Location", "XForm", 0.0 );
    vsp::SetParmVal( second_pod_id, "Z_Location", "XForm", 1.0 );


    //==== Save Vehicle to File ====//
    string fname = "apitest.vsp3";

    vsp::WriteVSPFile( fname );


    //==== List out all geoms ====//
    printf( "All geoms in Vehicle.\n" );
    vector<string> geoms = vsp::FindGeoms();
    for ( int i = 0; i < ( int ) geoms.size(); i++ )
    {
        printf( "Geom id: %s name: %s \n", geoms[i].c_str(), vsp::GetGeomName( geoms[i] ).c_str() );
    }

//==== Use Case 2 ====
    printf( "\nStart of second use case.\n" );

    //==== Create/Delete/Copy/Paste Geometry ====//
    vsp::VSPRenew();
    vsp::ErrorMgr.PopErrorAndPrint( stdout );

    //==== List out all geoms ====//
    printf( "All geoms in Vehicle.\n" );
    geoms = vsp::FindGeoms();
    for ( int i = 0; i < ( int ) geoms.size(); i++ )
    {
        printf( "Geom id: %s name: %s \n", geoms[i].c_str(), vsp::GetGeomName( geoms[i] ).c_str() );
    }

    //==== Add Fuselage Geom =====//
    fuse_id = vsp::AddGeom( "FUSELAGE" );
    vsp::ErrorMgr.PopErrorAndPrint( stdout );

    //==== Get XSec Surf ID ====//
    string xsurf_id = vsp::GetXSecSurf( fuse_id, 0 );

    //==== Change Type of First XSec ====//
    vsp::ChangeXSecType( xsurf_id, 0, vsp::XS_SUPER_ELLIPSE );

    //==== Change Type of First XSec ====//
    string xsec_id = vsp::GetXSec( xsurf_id, 0 );
    string width_id  = vsp::GetXSecParm( xsec_id, "Super_Width" );
    string height_id = vsp::GetXSecParm( xsec_id, "Super_Height" );
    vsp::SetParmVal( width_id, 4.0 );
    vsp::SetParmVal( height_id, 2.0 );

    //==== Copy Cros-Section to Clipboard ====//
    vsp::CopyXSec( xsurf_id, 0 );

    //==== Paste Cross-Section ====///
    vsp::PasteXSec( xsurf_id, 1 );
    vsp::PasteXSec( xsurf_id, 2 );
    vsp::PasteXSec( xsurf_id, 3 );

    //===== Change Type To File XSec ====//
    vsp::ChangeXSecType( xsurf_id, 0, vsp::XS_FILE_FUSE );
    string file_xsec_id = vsp::GetXSec( xsurf_id, 0 );

    //===== Build Point Vec ====//
    vector< vec3d > pnt_vec;
    pnt_vec.push_back( vec3d( 0.0, 0.0, 2.0 ) );
    pnt_vec.push_back( vec3d( 0.0, 1.0, 0.0 ) );
    pnt_vec.push_back( vec3d( 0.0, 0.0, -2.0 ) );
    pnt_vec.push_back( vec3d( 0.0, -1.0, 0.0 ) );
    pnt_vec.push_back( vec3d( 0.0, 0.0, 2.0 ) );

    //===== Load Point Into XSec ====//
    vsp::SetXSecPnts( file_xsec_id, pnt_vec );

    printf( "End of second use case, all geoms in Vehicle.\n" );
    geoms = vsp::FindGeoms();

    for ( int i = 0; i < ( int ) geoms.size(); i++ )
    {
        printf( "Geom id: %s name: %s \n", geoms[i].c_str(), vsp::GetGeomName( geoms[i] ).c_str() );
    }

    //==== Use Case 3 ====//
    printf( "\nStart of third use case, read in first-case file.\n" );

    //==== Read Geometry From File ====//
    vsp::VSPRenew();
    vsp::ErrorMgr.PopErrorAndPrint( stdout );

    vsp::ReadVSPFile( fname );

    //==== List out all geoms ====//
    printf( "All geoms in Vehicle.\n" );
    geoms = vsp::FindGeoms();
    for ( int i = 0; i < ( int ) geoms.size(); i++ )
    {
        printf( "Geom id: %s name: %s \n", geoms[i].c_str(), vsp::GetGeomName( geoms[i] ).c_str() );
    }


    //==== Check And Print Any Errors ====//
    int num_err = vsp::ErrorMgr.GetNumTotalErrors();
    for ( int i = 0 ; i < num_err ; i++ )
    {
        vsp::ErrorObj err = vsp::ErrorMgr.PopLastError();
        printf( "err = %s\n", err.m_ErrorString.c_str() );
    }

    vsp::StartGui();
}
