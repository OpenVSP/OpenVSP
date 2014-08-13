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
#include <windows.h>
#else
#include <pthread.h>
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
#include "VehicleMgr.h"
#include "GuiInterface.h"
#include "EventMgr.h"
#include "GeomCoreTestSuite.h"
#include "UtilTestSuite.h"
#include <string>
#include <time.h>
#include "APIDefines.h"

using namespace vsp;
using namespace std;

// Bitwise adds ecode to the current exit status code and returns to current exit status code
int vsp_add_and_get_estatus( int ecode )
{
    static int exit_status_code = ESTATUS_NO_ERRORS;
    exit_status_code |= ecode;

    return exit_status_code;
}

void vsp_exit()
{
    int exit_status = vsp_add_and_get_estatus( ESTATUS_NO_ERRORS );
    exit( exit_status );
}

bool RunUnitTests()
{
    Test::TextOutput output( Test::TextOutput::Verbose );

    //==== Util Test Suite ====//
    UtilTestSuite uts;
    uts.run( output );

    //==== Geom Core Test Suite ====//
    GeomCoreTestSuite gcts;
    gcts.run( output );


    return true;
}

//=====================================================//
//===== Batch Mode Check - Parse the Command Line =====//
//=====================================================//
int batchMode( int argc, char *argv[], Vehicle* vPtr )
{
    int i;
    int batchModeFlag = 0;
    int validateFileFlag = 0;
    int validFile = 1;
    int x3dFlag = 0;
    int scriptModeFlag = 0;

    string vsp_filename;
    string script_filename;

    i = 1;

    while ( i <= argc - 1 )
    {
        if ( strcmp( argv[i], "-batch" ) == 0 )
        {
            if ( i + 1 < argc )
            {
                vsp_filename = string( argv[++i] );
                batchModeFlag = 1;
            }
        }

        if ( strcmp( argv[i], "-validate" ) == 0 )
        {
            validateFileFlag = 1;
        }

        if ( strcmp( argv[i], "-x3d" ) == 0 )
        {
            x3dFlag = 1;
        }

        if ( strcmp( argv[i], "-script" ) == 0 )
        {
            if ( i + 1 < argc )
            {
                script_filename = string( argv[++i] );
                scriptModeFlag = 1;
            }
        }


        if ( strcmp( argv[i], "-help" ) == 0 || strcmp( argv[i], "-h" ) == 0 || strcmp( argv[i], "--help" ) == 0 )
        {
            printf( "\n" );
            printf( "          %s\n", VSPVERSION1 );
            printf( "--------------------------------------------\n" );
            printf( "Usage: vsp [inputfile.vsp] (run interactive version)\n" );
            printf( "     : vsp -batch  <filename>  (batch mode)\n" );
            printf( "     : vsp -script  <filename>  (run script)\n" );
            printf( "--------------------------------------------\n" );
            printf( "\n" );
            printf( "VSP batch options listed below:\n" );
            printf( "  -validate          Check if file is valid ( Returns 0 for valid,  %d for invalid)\n", ESTATUS_INVALID_FILE_ERROR );
            printf( "  -help              This message\n" );
            printf( "\n" );
            printf( "--------------------------------------------\n" );
            return 1;
        }

        i++;
    }

    if ( batchModeFlag == 0 && scriptModeFlag == 0 )
    {
        return 0;
    }

    if ( batchModeFlag )
    {
        // Read in File
        validFile = vPtr->ReadXMLFile( vsp_filename );
        string base_name;
        int ind = vsp_filename.find( ".vsp3" );

        if ( ind == vsp_filename.npos )
        {
            base_name = vsp_filename;
        }
        else
        {
            base_name = vsp_filename.substr( 0, ind );
        }

        if ( validateFileFlag )
        {
            if ( validFile != 0 || vPtr->GetFileOpenVersion() < MIN_FILE_VER )
            {
                vsp_add_and_get_estatus( ESTATUS_INVALID_FILE_ERROR );
            }
        }

        if ( x3dFlag )
        {
            string fname = base_name;
            fname.append( ".x3d" );
            vPtr->WriteX3DFile( fname, SET_ALL );
            printf( "X3D file name: %s \n", fname.c_str() );
        }
        return batchModeFlag;
    }
    if ( scriptModeFlag )
    {
        // Read Script File
        vPtr->RunScript( script_filename );
        return scriptModeFlag;
    }
    return 0;
}

bool ExtractVersionNumber( string & str, int* major, int* minor, int* change )
{
    *major = *minor = *change = -1;
    //==== Find Leading String ====//
    string target( "The latest release of OpenVSP is version " );
    size_t start = str.find( target ) + target.size();
    size_t end = str.find( ".\n", start );

    //==== Just The Number =====//
    string verstr = str.substr ( start, end - start );

    if ( verstr.size() < 5 || verstr.size() > 9 )
    {
        return false;
    }

    size_t period1 = verstr.find( "." );
    size_t period2 = verstr.find( ".", period1 + 1 );

    //==== Extract Major/Minor/Change Numbers ====//
    *major  = atoi( verstr.substr( 0, period1 ).c_str() );
    *minor  = atoi( verstr.substr( period1 + 1, period2 - ( period1 + 1 ) ).c_str() );
    *change = atoi( verstr.substr( period2 + 1, verstr.size() ).c_str() );

    if ( *major >= 0 && *minor >= 0 && *change >= 0 )
    {
        return true;
    }

    return false;
}

// Simple version message function to be executed by main thread
// when commanded by version check thread.
//void newver_msg( void *data ) {
//  if ( screenMgrPtr )
//      screenMgrPtr->MessageBox("A new version of OpenVSP is available at http://www.openvsp.org/");
//}

#ifdef WIN32
DWORD WINAPI CheckVersionNumber( LPVOID lpParameter )
#else
void* CheckVersionNumber( void *threadid )
#endif

{
    //==== Init Nano HTTP ====//
    xmlNanoHTTPInit();
    xmlNanoHTTPScanProxy( NULL );

    //==== Compute Version Number ====//
    int ver_no = 10000 * VSP_VERSION_MAJOR + 100 * VSP_VERSION_MINOR + VSP_VERSION_CHANGE;

    char cCurrentPath[FILENAME_MAX];
    GetCurrentDir( cCurrentPath, sizeof( cCurrentPath ) );

    int user_id = 0;
    int path_len = strlen( cCurrentPath );

    for ( int i = 0 ; i < path_len ; i++ )
    {
        srand ( ( unsigned int )cCurrentPath[i] );
        user_id += rand() % 100000 + 1;
    }

    // Reset random seed after generating user_id.
    // +1 is to be sure this isn't called less than zero seconds from before.
    srand( ( unsigned int )time( NULL ) + 1 );

    //==== Post User Info To Server ====//
    char poststr[256];
    sprintf( poststr, "postvar1=%d&postvar2=%d\r\n", user_id, ver_no );
    int poststrlen = strlen( poststr );
    const char*  headers = "Content-Type: application/x-www-form-urlencoded \n";

    void * ctx = 0;
    ctx = xmlNanoHTTPMethod( "http://www.openvsp.org/vspuse_post.php", "POST", poststr, NULL, headers, poststrlen );

    if ( ctx )
    {
        xmlNanoHTTPClose( ctx );
    }

    ctx = 0;

    //==== Open Settings File ====//
    bool check_version_flag = true;
    FILE* vsptime_fp = fopen( ".vsptime", "r" );
    if ( vsptime_fp )
    {
        char str[256];
        fgets( str, 256, vsptime_fp );
        int vsp_time = atoi( str );
        int del_time =  ( int )time( NULL ) - vsp_time;
        if ( del_time < 60 * 60 * 24 * 7 )          // Check Every Week
        {
            check_version_flag = false;
        }

        fclose( vsptime_fp );
    }

    //==== Enough Time Has Passed - Check For New Version ====//
    if ( check_version_flag )
    {
        //==== Webpage with Version Info ====//
        char * pContentType = 0;
        ctx = xmlNanoHTTPOpen( "http://www.openvsp.org/latest_version3.html", &pContentType );

        int retCode = xmlNanoHTTPReturnCode( ctx );

        //==== Http Return Code 200 -> OK  ====//
        string contentStr;
        if ( retCode == 200 )
        {
            char buf[2048];
            int len = 1;
            while ( len > 0 && contentStr.size() < 10000 )
            {
                len = xmlNanoHTTPRead( ctx, buf, sizeof( buf ) );
                contentStr.append( buf, len );
            }
        }

        //==== Pulled A String From Server =====//
        if ( contentStr.size() > 0 )
        {
            int major_ver, minor_ver, change_ver;
            bool valid = ExtractVersionNumber( contentStr, &major_ver, &minor_ver, &change_ver );

            if ( valid )
            {
                if ( major_ver != VSP_VERSION_MAJOR || minor_ver != VSP_VERSION_MINOR || change_ver != VSP_VERSION_CHANGE )
                {
                    // Send message to main thread to display new version message.
                    printf( "A new version is available\n" );
                    GuiInterface::getInstance().PopupMsg( "A new version of OpenVSP is available at http://www.openvsp.org/", false );
                }
            }
        }
        //===== Write Time =====//
        FILE* vsptime_fp = fopen( ".vsptime", "w" );
        if ( vsptime_fp )
        {
            fprintf( vsptime_fp, "%d", ( int )time( NULL ) );
            fclose( vsptime_fp );
        }
    }

    if ( ctx )
    {
        xmlNanoHTTPClose( ctx );
    }

    xmlNanoHTTPCleanup();

    return 0;
}

void ThreadCheckVersionNumber()
{
#ifdef WIN32

    DWORD myThreadID;
    HANDLE myHandle = CreateThread( 0, 0, CheckVersionNumber, 0, 0, &myThreadID );
#else

    long t = 0;
    pthread_t thread;
    pthread_create( &thread, NULL, CheckVersionNumber, ( void * )t );
#endif
}

//========================================================//
//========================================================//
//========================= Main =========================//

int main( int argc, char** argv )
{

//#ifndef NDEBUG
//  RunUnitTests();
//#endif

    //==== Get Vehicle Ptr ====//
    Vehicle* vPtr = VehicleMgr.GetVehicle();

    if ( batchMode( argc, argv, vPtr ) )
    {
        vsp_exit();
    }

    //==== Init Gui ====//
    GuiInterface::getInstance().InitGui( vPtr );

    //==== Check Server For Version Number ====//
    ThreadCheckVersionNumber();

    //==== Run Test Scripts =====//
#ifndef NDEBUG
    vPtr->RunTestScripts();
#endif

    //==== Start Gui - FLTK Now Control Process ====//
    GuiInterface::getInstance().StartGui();
}
