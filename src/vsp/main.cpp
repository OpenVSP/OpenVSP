//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//

// Leak Detection http://www.codeproject.com/KB/applications/visualleakdetector.aspx
//#include "vld.h"

#include <cstdio>

#ifdef WIN32
#include <windows.h>
#else
#include <pthread.h>
#endif

#include <libxml/tree.h>
#include <libxml/nanohttp.h>

#include <functional>

#ifdef WIN32
#include <direct.h>
#define GetCurrentDir _getcwd
#else
#include <unistd.h>
#define GetCurrentDir getcwd
#endif


#include "main.h"
#include "VehicleMgr.h"
#include "GuiInterface.h"
#include "common.h"

using namespace vsp;
using namespace std;

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

    return *major >= 0 && *minor >= 0 && *change >= 0;
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

    unsigned long user_id = ( unsigned long )std::hash<std::string>{}( string( cCurrentPath ) );

    //==== Post User Info To Server ====//
    char poststr[256];
    sprintf( poststr, "postvar1=%lu&postvar2=%d\r\n", user_id, ver_no );
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
        FILE* vsptime_fpw = fopen( ".vsptime", "w" );
        if ( vsptime_fpw )
        {
            fprintf( vsptime_fpw, "%d", ( int )time( NULL ) );
            fclose( vsptime_fpw );
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
    //==== Get Vehicle Ptr ====//
    Vehicle* vPtr = VehicleMgr.GetVehicle();

    int ret;
    if ( batchMode( argc, argv, vPtr, ret ) )
    {
        vsp_exit( ret );
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
