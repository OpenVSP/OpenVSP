//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//

// HeldenMgr.cpp
//
//////////////////////////////////////////////////////////////////////

#include "Vehicle.h"

#include "HeldenMgr.h"
#include "FileUtil.h"

/////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////

HeldenMgrSingleton::HeldenMgrSingleton() : ParmContainer()
{
    m_Name = "HeldenMgr";

    string groupName = "Helden";

    m_SelectedSetIndex.Init( "SelSetIndex", groupName, this, 0, 0, 1.0e12 );
    m_SelectedSetIndex.SetDescript( "Selected Set Index" );

    m_IGESLenUnit.Init( "LenUnit", groupName, this, vsp::LEN_FT, vsp::LEN_MM, vsp::LEN_FT );
    m_IGESSplitSurfs.Init( "SplitSurfs", groupName, this, true, 0, 1 );
    m_IGESSplitSubSurfs.Init( "SplitSubSurfs", groupName, this, false, 0, 1 );
    m_IGESToCubic.Init( "ToCubic", groupName, this, false, 0, 1 );
    m_IGESToCubicTol.Init( "ToCubicTol", groupName, this, 1e-6, 1e-12, 1e12 );
    m_IGESTrimTE.Init( "TrimTE", groupName, this, false, 0, 1 );

    m_HPatchProcessKill = false;
    m_HMeshProcessKill = false;

    Init();
}

void HeldenMgrSingleton::Init()
{
    m_LastMeshID = string("");
}

void HeldenMgrSingleton::Wype()
{
}

void HeldenMgrSingleton::Renew()
{
    Wype();
    Init();
}

xmlNodePtr HeldenMgrSingleton::EncodeXml( xmlNodePtr & node )
{
    xmlNodePtr heldennode = xmlNewChild( node, NULL, BAD_CAST"HeldenMgr", NULL );

    ParmContainer::EncodeXml( heldennode );

    return heldennode;
}

xmlNodePtr HeldenMgrSingleton::DecodeXml( xmlNodePtr & node )
{
    xmlNodePtr heldennode = XmlUtil::GetNode( node, "HeldenMgr", 0 );
    if ( heldennode )
    {
        ParmContainer::DecodeXml( heldennode );
    }

    return heldennode;
}

//==== Parm Changed ====//
void HeldenMgrSingleton::ParmChanged( Parm* parm_ptr, int type )
{
    Vehicle* veh = VehicleMgr.GetVehicle();

    if ( veh )
    {
        veh->ParmChanged( parm_ptr, type );
    }
}

void HeldenMgrSingleton::InitHSurf()
{
    Vehicle *veh = VehicleMgr.GetVehicle();

    if ( veh )
    {
        vector<string> args;

        // Run HeldenMesh with -x to create example input file.
        args.push_back( "-x" );

        // Execute Helden Patch
        m_HMeshProcess.ForkCmd( veh->GetExePath(), veh->GetHeldenSurfCmd(), args );

        // Capture output and dump to INPUT_EXAMPLE
        string exfname = FileInPathOf( "INPUT_HELDENSURF_EXAMPLE" );
        FILE *logFile = fopen( exfname.c_str(), "w" );
        MonitorHMesh( logFile, true );
        fclose( logFile );

        // Re-open INPUT_EXAMPLE for read.
        FILE *infile = fopen( exfname.c_str(), "r" );

        if ( infile )
        {
            string hsfname = FileInPathOf( "INPUT_HELDENSURFACE" );
            FILE *hsfile = fopen( hsfname.c_str(), "w" );

            if ( hsfile )
            {
                char buf[1024];

                int initskip = 2;
                int header = 2;

                // Skip initial lines.
                for ( int i = 0; i < initskip; i++ )
                {
                    fgets( buf, sizeof(buf), infile );
                }

                for ( int i = 0; i < header; i++ )
                {
                    fgets( buf, sizeof(buf), infile );
                    fprintf( hsfile, "%s", buf );
                }

                // Skip line with VSP.rst specified.
                fgets( buf, sizeof(buf), infile );

                // Print line with HELDENPATCH.rst instead.
                fprintf( hsfile, "HELDENPATCH.rst                    0.1         1           0\n" );

                // Dump remainder of file as example.
                while ( fgets( buf, sizeof( buf ), infile ) != NULL )
                {
                    fprintf( hsfile, "%s", buf );
                }

                fclose( hsfile );
            }
            fclose( infile );
        }
    }
}

/* ExecuteHSurf(FILE * logFile)
Returns a result with a vector of results id's under the name ResultVec
Optional input of logFile allows outputting to a log file or the console
*/
void HeldenMgrSingleton::ExecuteHSurf( FILE *logFile )
{
    Vehicle *veh = VehicleMgr.GetVehicle();

    if ( veh )
    {
        vector<string> args;

        string igesfname = veh->getExportFileName( vsp::HELDEN_IGES_TYPE );

        string path = GetPath( igesfname );

        string hsfname = FileInPathOf( "INPUT_HELDENSURFACE" );

        args.push_back( hsfname );

        //Print out execute command
        string cmdStr = m_HSurfProcess.PrettyCmd( veh->GetExePath(), veh->GetHeldenSurfCmd(), args );
        if( logFile )
        {
            fprintf( logFile, "%s", cmdStr.c_str() );
        }
        else
        {
            MessageData data;
            data.m_String = "HeldenMessage";
            data.m_StringVec.push_back( cmdStr );
            MessageMgr::getInstance().Send( "ScreenMgr", NULL, data );
        }

        string pwd = GetCurrentWorkingDirectory();
        ChangeWorkingDirectory( path );
        // Execute Helden Patch
        m_HSurfProcess.ForkCmd( veh->GetExePath(), veh->GetHeldenSurfCmd(), args );

        // Return working directory
        ChangeWorkingDirectory( pwd );

        // ==== MonitorSolverProcess ==== //
        MonitorHSurf(logFile);

        // Check if the kill solver flag has been raised, if so clean up and return
        //  note: we could have exited the IsRunning loop if the process was killed
        if( m_HSurfProcessKill )
        {
            m_HSurfProcessKill = false;    //reset kill flag

            return;
        }

        // Send the message to update the screens
        MessageData data;
        data.m_String = "UpdateAllScreens";
        MessageMgr::getInstance().Send( "ScreenMgr", NULL, data );

    }
}

void HeldenMgrSingleton::MonitorHSurf(FILE *logFile)
{
    // ==== MonitorSolverProcess ==== //
    int bufsize = 1000;
    char *buf;
    buf = ( char* ) malloc( sizeof( char ) * ( bufsize + 1 ) );
    unsigned long nread = 1;
    bool runflag = m_HSurfProcess.IsRunning();
    while ( runflag || nread > 0 )
    {
        m_HSurfProcess.ReadStdoutPipe( buf, bufsize, &nread );
        if( nread > 0 && nread != ( unsigned long ) - 1 )
        {
            if ( buf )
            {
                buf[nread] = 0;
                StringUtil::change_from_to( buf, '\r', '\n' );
                if( logFile )
                {
                    fprintf( logFile, "%s", buf );
                }
                else
                {
                    MessageData data;
                    data.m_String = "HeldenMessage";
                    data.m_StringVec.push_back( string( buf ) );
                    MessageMgr::getInstance().Send( "ScreenMgr", NULL, data );
                }
            }
        }
        SleepForMilliseconds( 100 );
        runflag = m_HSurfProcess.IsRunning();
    }
}

// helper thread functions for VSPAERO GUI interface and multi-threaded impleentation
bool HeldenMgrSingleton::IsHSurfRunning()
{
    return m_HSurfProcess.IsRunning();
}

void HeldenMgrSingleton::KillHSurf()
{
    // Raise flag to break the compute solver thread
    m_HSurfProcessKill = true;
    return m_HSurfProcess.Kill();
}

void HeldenMgrSingleton::InitHPatch()
{
    Vehicle *veh = VehicleMgr.GetVehicle();

    BndBox new_box;
    int ngeom;
    vector< Geom* > geom_vec = veh->FindGeomVec( veh->GetGeomVec() );
    ngeom = (int) geom_vec.size();
    for ( int i = 0 ; i < ngeom ; i++ )
    {
        if( geom_vec[i]->GetSetFlag( m_SelectedSetIndex() ) )
        {
            new_box.Update( geom_vec[i]->GetBndBox() );
        }
    }

    string rstfname = FileInPathOf( "HELDENSURFACE.rst" );

    string hpfname = FileInPathOf( "INPUT_HELDENPATCH" );

    FILE *fp = fopen( hpfname.c_str(), "w" );

    if ( fp )
    {
        vec3d v( 10, 5, 20 );

        new_box.Scale( v );

        fprintf( fp, "%s\n", rstfname.c_str() );
        fprintf( fp, "cbox %f %f %f %f %f %f\n", new_box.GetMin( 0 ), new_box.GetMax( 0 ), new_box.GetMin( 1 ), new_box.GetMax( 1 ), new_box.GetMin( 2 ), new_box.GetMax( 2 ) );
        fprintf( fp, "clean\n" );
        fprintf( fp, "0\n" );

        fclose( fp );
    }

}

void HeldenMgrSingleton::ExportIGESConvertRST( FILE *logFile )
{

    Vehicle *veh = VehicleMgr.GetVehicle();

    if ( veh )
    {
        string igesfname = veh->getExportFileName( vsp::HELDEN_IGES_TYPE );

        string path = GetPath( igesfname );

        veh->WriteIGESFile( igesfname, m_SelectedSetIndex(), m_IGESLenUnit(), m_IGESSplitSubSurfs(),
                            m_IGESSplitSurfs(), m_IGESToCubic(), m_IGESToCubicTol(), m_IGESTrimTE(),
                            true, true, false, false, vsp::DELIM_USCORE );

        string fname = FileInPathOf( "INPUT_CONVERT" );
        FILE *fp = fopen( fname.c_str(), "w" );

        if ( fp )
        {
            fprintf( fp, "%s\n", igesfname.c_str() );
            fprintf( fp, "0\n" );

            fclose( fp );
        }

        vector<string> args;

        // Not sure why, but having an empty argument list doesn't work.
        // This redirection doesn't actually work either, as it should be handled by the OS.
        args.push_back( " < " + fname );

        //Print out execute command
        string cmdStr = m_HPatchProcess.PrettyCmd( veh->GetExePath(), veh->GetHeldenPatchCmd(), args );
        if( logFile )
        {
            fprintf( logFile, "%s", cmdStr.c_str() );
        }
        else
        {
            MessageData data;
            data.m_String = "HeldenMessage";
            data.m_StringVec.push_back( cmdStr );
            MessageMgr::getInstance().Send( "ScreenMgr", NULL, data );
        }

        string pwd = GetCurrentWorkingDirectory();
        ChangeWorkingDirectory( path );
        // Execute Helden Patch
        m_HPatchProcess.ForkCmd( veh->GetExePath(), veh->GetHeldenPatchCmd(), args );

        m_HPatchProcess.DumpFileStdinPipe( fname );

        // Return working directory
        ChangeWorkingDirectory( pwd );


        // ==== MonitorSolverProcess ==== //
        MonitorHPatch(logFile);

        // Check if the kill solver flag has been raised, if so clean up and return
        //  note: we could have exited the IsRunning loop if the process was killed
        if( m_HPatchProcessKill )
        {
            m_HPatchProcessKill = false;    //reset kill flag

            return;
        }

        // Send the message to update the screens
        MessageData data;
        data.m_String = "UpdateAllScreens";
        MessageMgr::getInstance().Send( "ScreenMgr", NULL, data );

    }
}

/* ExecuteHPatch(FILE * logFile)
Returns a result with a vector of results id's under the name ResultVec
Optional input of logFile allows outputting to a log file or the console
*/
void HeldenMgrSingleton::ExecuteHPatch( FILE *logFile )
{
    Vehicle *veh = VehicleMgr.GetVehicle();

    if ( veh )
    {
        string igesfname = veh->getExportFileName( vsp::HELDEN_IGES_TYPE );

        string path = GetPath( igesfname );

        string hpfname = FileInPathOf( "INPUT_HELDENPATCH" );

        vector<string> args;

        // Not sure why, but having an empty argument list doesn't work.
        // This redirection doesn't actually work either, as it should be handled by the OS.
        args.push_back( " < " + hpfname );

        //Print out execute command
        string cmdStr = m_HPatchProcess.PrettyCmd( veh->GetExePath(), veh->GetHeldenPatchCmd(), args );
        if( logFile )
        {
            fprintf( logFile, "%s", cmdStr.c_str() );
        }
        else
        {
            MessageData data;
            data.m_String = "HeldenMessage";
            data.m_StringVec.push_back( cmdStr );
            MessageMgr::getInstance().Send( "ScreenMgr", NULL, data );
        }

        string pwd = GetCurrentWorkingDirectory();
        ChangeWorkingDirectory( path );
        // Execute Helden Patch
        m_HPatchProcess.ForkCmd( veh->GetExePath(), veh->GetHeldenPatchCmd(), args );

        m_HPatchProcess.DumpFileStdinPipe( hpfname );

        // Return working directory
        ChangeWorkingDirectory( pwd );


        // ==== MonitorSolverProcess ==== //
        MonitorHPatch(logFile);

        // Check if the kill solver flag has been raised, if so clean up and return
        //  note: we could have exited the IsRunning loop if the process was killed
        if( m_HPatchProcessKill )
        {
            m_HPatchProcessKill = false;    //reset kill flag

            return;
        }

        // Send the message to update the screens
        MessageData data;
        data.m_String = "UpdateAllScreens";
        MessageMgr::getInstance().Send( "ScreenMgr", NULL, data );

    }
}

void HeldenMgrSingleton::MonitorHPatch(FILE *logFile)
{
    // ==== MonitorSolverProcess ==== //
    int bufsize = 1000;
    char *buf;
    buf = ( char* ) malloc( sizeof( char ) * ( bufsize + 1 ) );
    unsigned long nread = 1;
    bool runflag = m_HPatchProcess.IsRunning();
    while ( runflag || nread > 0 )
    {
        m_HPatchProcess.ReadStdoutPipe( buf, bufsize, &nread );
        if( nread > 0 && nread != ( unsigned long ) - 1 )
        {
            if ( buf )
            {
                buf[nread] = 0;
                StringUtil::change_from_to( buf, '\r', '\n' );
                if( logFile )
                {
                    fprintf( logFile, "%s", buf );
                }
                else
                {
                    MessageData data;
                    data.m_String = "HeldenMessage";
                    data.m_StringVec.push_back( string( buf ) );
                    MessageMgr::getInstance().Send( "ScreenMgr", NULL, data );
                }
            }
        }
        SleepForMilliseconds( 100 );
        runflag = m_HPatchProcess.IsRunning();
    }
}

// helper thread functions for VSPAERO GUI interface and multi-threaded impleentation
bool HeldenMgrSingleton::IsHPatchRunning()
{
    return m_HPatchProcess.IsRunning();
}

void HeldenMgrSingleton::KillHPatch()
{
    // Raise flag to break the compute solver thread
    m_HPatchProcessKill = true;
    return m_HPatchProcess.Kill();
}

void HeldenMgrSingleton::InitHMesh()
{
    Vehicle *veh = VehicleMgr.GetVehicle();

    if ( veh )
    {
        vector<string> args;

        // Run HeldenMesh with -x to create example input file.
        args.push_back( "-x" );

        // Execute Helden Patch
        m_HMeshProcess.ForkCmd( veh->GetExePath(), veh->GetHeldenMeshCmd(), args );

        // Capture output and dump to INPUT_EXAMPLE
        string exfname = FileInPathOf( "INPUT_EXAMPLE" );
        FILE *logFile = fopen( exfname.c_str(), "w" );
        MonitorHMesh( logFile, true );
        fclose( logFile );

        // Re-open INPUT_EXAMPLE for read.
        FILE *infile = fopen( exfname.c_str(), "r" );

        if ( infile )
        {
            string hmfname = FileInPathOf( "INPUT_HELDENMESH" );
            FILE *hmfile = fopen( hmfname.c_str(), "w" );

            if ( hmfile )
            {
                char buf[1024];

                int initskip = 2;
                int headerfirst = 3;
                int headernext = 7;

                // Skip initial lines.
                for ( int i = 0; i < initskip; i++ )
                {
                    fgets( buf, sizeof(buf), infile );
                }

                // Copy header lines.
                for ( int i = 0; i < headerfirst; i++ )
                {
                    fgets( buf, sizeof(buf), infile );
                    fprintf( hmfile, "%s", buf );
                }

                fprintf( hmfile, "OUTPUT: cart3d\n" );

                // Continue copying the header lines.
                for ( int i = 0; i < headernext; i++ )
                {
                    fgets( buf, sizeof(buf), infile );
                    fprintf( hmfile, "%s", buf );
                }

                fprintf( hmfile, "SRC_PATCH: All\n" );
                fprintf( hmfile, "STRENGTH    WIDTH_RATIO STRETCH_MAX EDGE_ANGLE\n" );
                fprintf( hmfile, "1           2           1000000     30\n" );
                fprintf( hmfile, "S_MIN       S_BASE      CURV_ANGLE  S_EDGE\n" );
                fprintf( hmfile, "0.3         10          3           0.3\n" );
                fprintf( hmfile, "----------------------------------------------------------\n" );

                // Build geometry dependent lines.
                int ngeom;
                vector< Geom* > geom_vec = veh->FindGeomVec( veh->GetGeomVec() );
                ngeom = (int) geom_vec.size();
                for ( int i = 0 ; i < ngeom ; i++ )
                {
                    if( geom_vec[i]->GetSetFlag( m_SelectedSetIndex() ) )
                    {
                        fprintf( hmfile, "SRC_PATCH: %s_%s\n", geom_vec[i]->GetID().c_str(), geom_vec[i]->GetName().c_str() );
                        fprintf( hmfile, "STRENGTH    WIDTH_RATIO STRETCH_MAX EDGE_ANGLE\n" );
                        fprintf( hmfile, "1           2           1000000     30\n" );
                        fprintf( hmfile, "S_MIN       S_BASE      CURV_ANGLE  S_EDGE\n" );
                        fprintf( hmfile, "0.3         10          3           0.3\n" );
                        fprintf( hmfile, "----------------------------------------------------------\n" );
                    }
                }

                // Reset INPUT_EXAMPLE file to beginning
                rewind( infile );

                // Skip initial lines.
                for ( int i = 0; i < initskip; i++ )
                {
                    fgets( buf, sizeof(buf), infile );
                }

                fprintf( hmfile, "----------------------------------------------------------\n" );
                fprintf( hmfile, "SEE BELOW FOR FURTHER DOCUMENTATION FOR REFERENCE\n" );

                // Dump remainder of file as example.
                while (fgets(buf, sizeof(buf), infile) != NULL)
                {
                    fprintf( hmfile, "%s", buf );
                }


                fclose( hmfile );
            }
            fclose( infile );
        }
    }
}

/* ExecuteHMesh(FILE * logFile)
Returns a result with a vector of results id's under the name ResultVec
Optional input of logFile allows outputting to a log file or the console
*/
void HeldenMgrSingleton::ExecuteHMesh( FILE *logFile )
{
    Vehicle *veh = VehicleMgr.GetVehicle();

    if ( veh )
    {
        if ( !m_LastMeshID.empty() )
        {
            veh->DeleteGeom( m_LastMeshID );
            m_LastMeshID = string( "" );
        }

        vector<string> args;

        string igesfname = veh->getExportFileName( vsp::HELDEN_IGES_TYPE );

        string path = GetPath( igesfname );

        string hmfname = "INPUT_HELDENMESH";

        // Not sure why, but having an empty argument list doesn't work.
        // This redirection doesn't actually work either, as it should be handled by the OS.
        args.push_back( hmfname );

        //Print out execute command
        string cmdStr = m_HMeshProcess.PrettyCmd( veh->GetExePath(), veh->GetHeldenMeshCmd(), args );
        if( logFile )
        {
            fprintf( logFile, "%s", cmdStr.c_str() );
        }
        else
        {
            MessageData data;
            data.m_String = "HeldenMessage";
            data.m_StringVec.push_back( cmdStr );
            MessageMgr::getInstance().Send( "ScreenMgr", NULL, data );
        }

        string pwd = GetCurrentWorkingDirectory();
        ChangeWorkingDirectory( path );
        // Execute Helden Patch
        m_HMeshProcess.ForkCmd( veh->GetExePath(), veh->GetHeldenMeshCmd(), args );

        // Return working directory
        ChangeWorkingDirectory( pwd );

        // ==== MonitorSolverProcess ==== //
        MonitorHMesh( logFile, false );

        // Check if the kill solver flag has been raised, if so clean up and return
        //  note: we could have exited the IsRunning loop if the process was killed
        if( m_HMeshProcessKill )
        {
            m_HMeshProcessKill = false;    //reset kill flag

            return;
        }

        // Send the message to update the screens
        MessageData data;
        data.m_String = "UpdateAllScreens";
        MessageMgr::getInstance().Send( "ScreenMgr", NULL, data );

    }
}

void HeldenMgrSingleton::MonitorHMesh(FILE *logFile, bool initflag )
{
    // ==== MonitorSolverProcess ==== //
    int bufsize = 1000;
    char *buf;
    buf = ( char* ) malloc( sizeof( char ) * ( bufsize + 1 ) );
    unsigned long nread = 1;
    bool runflag = m_HMeshProcess.IsRunning();
    while ( runflag || nread > 0 )
    {
        m_HMeshProcess.ReadStdoutPipe( buf, bufsize, &nread );
        if( nread > 0 && nread != ( unsigned long ) - 1 )
        {
            if ( buf )
            {
                buf[nread] = 0;

                // Create sbuf containing buf, but without '\r'.
                string sbuf;
                char * pch = strtok( buf, "\r" );
                while( pch != NULL )
                {
                    sbuf.append(pch);
                    pch = strtok( NULL, "\r" );
                }

                if( logFile )
                {
                    fprintf( logFile, "%s", sbuf.c_str() );
                }
                else
                {
                    MessageData data;
                    data.m_String = "HeldenMessage";
                    data.m_StringVec.push_back( sbuf );
                    MessageMgr::getInstance().Send( "ScreenMgr", NULL, data );
                }
            }
        }
        SleepForMilliseconds( 100 );
        runflag = m_HMeshProcess.IsRunning();
    }

    if ( !initflag )
    {
        Vehicle *veh = VehicleMgr.GetVehicle();

        if( veh )
        {
            veh->HideAll();

            string fname = FileInPathOf( "Addams.i.tri" );

            m_LastMeshID = veh->ImportFile( fname, vsp::IMPORT_CART3D_TRI );
        }
    }
}

// helper thread functions for VSPAERO GUI interface and multi-threaded impleentation
bool HeldenMgrSingleton::IsHMeshRunning()
{
    return m_HMeshProcess.IsRunning();
}

void HeldenMgrSingleton::KillHMesh()
{
    // Raise flag to break the compute solver thread
    m_HMeshProcessKill = true;
    return m_HMeshProcess.Kill();
}


string HeldenMgrSingleton::FileInPathOf( const string & fin )
{
    Vehicle *veh = VehicleMgr.GetVehicle();

    string igesfname = veh->getExportFileName( vsp::HELDEN_IGES_TYPE );

    string fout = GetPath( igesfname );

    if ( fout.size() > 0 )
    {
        fout.append( "/" );
    }
    fout.append( fin );

    return fout;
}
