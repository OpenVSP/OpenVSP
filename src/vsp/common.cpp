//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//

#include "common.h"
#include "main.h"
#include "VSP_Geom_API.h"
#include "DesignVarMgr.h"

// Bitwise adds ecode to the current exit status code and returns to current exit status code
int vsp_add_and_get_estatus( unsigned int ecode )
{
    static int exit_status_code = ESTATUS_NO_ERRORS;
    exit_status_code |= ecode;

    return exit_status_code;
}

void vsp_exit( int ret )
{
    unsigned int uret = ret;
    int exit_status = vsp_add_and_get_estatus( uret );
    exit( exit_status );
}

//=====================================================//
//===== Batch Mode Check - Parse the Command Line =====//
//=====================================================//
int batchMode( int argc, char *argv[], Vehicle* vPtr, int &ret )
{
    ret = 0; // Success
    int i;
    int validFile = 1;
    int scriptModeFlag = 0;
    int desModeFlag = 0;
    int xddmModeFlag = 0;
    int genDocFlag = 0;
    int vspFileFlag = 0;

    string vsp_filename;
    string script_filename;
    string des_filename;
    string xddm_filename;

    i = 1;

    while ( i <= argc - 1 )
    {
        if ( strcmp( argv[i], "-script" ) == 0 )
        {
            if ( i + 1 < argc )
            {
                script_filename = string( argv[++i] );
                scriptModeFlag = 1;
            }
        }
        else if ( strcmp( argv[i], "-des" ) == 0 )
        {
            if ( i + 1 < argc )
            {
                des_filename = string( argv[++i] );
                desModeFlag = 1;
            }
        }
        else if ( strcmp( argv[i], "-xddm" ) == 0 )
        {
            if ( i + 1 < argc )
            {
                xddm_filename = string( argv[++i] );
                xddmModeFlag = 1;
            }
        }
        else if ( strcmp( argv[i], "-doc" ) == 0 )
        {
            genDocFlag = 1;
        }
        else
        {
            vsp_filename = string( argv[i] );
            vspFileFlag = 1;
        }

        if ( strcmp( argv[i], "-help" ) == 0 || strcmp( argv[i], "-h" ) == 0 || strcmp( argv[i], "--help" ) == 0 )
        {
            printf( "\n" );
            printf( "          %s\n", VSPVERSION1 );
            printf( "-----------------------------------------------------------\n" );
            printf( "Usage: vsp [inputfile.vsp3]               Run interactively\n" );
            printf( "     : vsp -script <vspscriptfile>        Run script\n" );
            printf( "-----------------------------------------------------------\n" );
            printf( "\n" );
            printf( "VSP command line options listed below:\n" );
            printf( "  -help              This message\n" );
            printf( "  -des <desfile>     Set variables according to *.des file\n" );
            printf( "  -xddm <xddmfile>   Set variables according to *.xddm file\n" );
            printf( "  -doc               Generate an API header file for Doxygen (openvsp_as.h)\n" );
            printf( "\n" );
            printf( "-----------------------------------------------------------\n" );
            return 1;
        }

        i++;
    }

    if ( vspFileFlag )
    {
        vPtr->SetVSP3FileName( vsp_filename );
        validFile = vPtr->ReadXMLFile( vsp_filename );
    }

    if ( desModeFlag && vspFileFlag )
    {
        DesignVarMgr.ReadDesVarsDES( des_filename );
    }

    if ( xddmModeFlag && vspFileFlag )
    {
        DesignVarMgr.ReadDesVarsXDDM( xddm_filename );
    }

    if ( genDocFlag )
    {
        vPtr->GenAPIDocs( ( "openvsp_as.h" ) ); // vPtr->GetExePath() + "/" + 
        return 1; // Exit VSP
    }

    if ( scriptModeFlag )
    {
        // Read Script File
        ret = vPtr->RunScript( script_filename );
        return scriptModeFlag;
    }
    return 0;
}
