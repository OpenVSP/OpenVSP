//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//
//
// Rob McDonald
//////////////////////////////////////////////////////////////////////

#include "AeroStructMgr.h"
#include "Vehicle.h"
#include "MeshGeom.h"
#include "WingGeom.h"
#include "FileUtil.h"
#include "VSPAEROMgr.h"
#include "StructureMgr.h"

AeroStructSingleton::AeroStructSingleton() : ParmContainer()
{
    m_Name = "AeroStructSettings";


}

xmlNodePtr AeroStructSingleton::EncodeXml( xmlNodePtr & node )
{
    xmlNodePtr AeroStructnode = xmlNewChild( node, NULL, BAD_CAST"AeroStructMgr", NULL );

    ParmContainer::EncodeXml( AeroStructnode );


    return AeroStructnode;
}

xmlNodePtr AeroStructSingleton::DecodeXml( xmlNodePtr & node )
{
    xmlNodePtr AeroStructnode = XmlUtil::GetNode( node, "AeroStructMgr", 0 );

    if ( AeroStructnode )
    {
        ParmContainer::DecodeXml( AeroStructnode );
    }

    return AeroStructnode;
}

void AeroStructSingleton::Update()
{
    VSPAEROMgr.UpdateFilenames();

    m_ADBFile = VSPAEROMgr.m_AdbFile;
    m_ADBFileFound = FileExist( m_ADBFile );


    m_FEAMeshFile = string();
    FeaStructure* fea_struct = StructureMgr.GetFeaStruct( StructureMgr.m_CurrStructIndex() );
    if ( fea_struct )
    {
        vector < string > fnames = fea_struct->GetStructSettingsPtr()->GetExportFileNames();
        m_FEAMeshFile = fnames[ vsp::FEA_CALCULIX_FILE_NAME ];
    }
    m_FEAMeshFileFound = FileExist( m_FEAMeshFile );

    m_FEAInputFile = string();
    if ( m_FEAMeshFileFound )
    {
        m_FEAInputFile = GetBasename( m_FEAMeshFile ) + ".static.inp";
    }
    m_FEAInputFileFound = FileExist( m_FEAInputFile );


    m_FEASolutionFile = string();
    if ( m_FEAInputFileFound )
    {
        m_FEASolutionFile = GetBasename( m_FEAInputFile ) + ".frd";
    }
    m_FEASolutionFileFound = FileExist( m_FEASolutionFile );


}

void AeroStructSingleton::LoadDrawObjs( vector< DrawObj* > &draw_obj_vec )
{
    Vehicle *veh = VehicleMgr.GetVehicle();
    if ( !veh )
    {
        assert( false );
        return;
    }


}

void AeroStructSingleton::FindCCX( const string & path )
{
    m_CalculiXFound = false;
    m_CalculiXCmd = "ccx";
    m_CalculiXPath = string();

    // Check specific location
#ifdef WIN32
    if ( CheckForFile( path, "ccx.exe" ) )
#else
    if ( CheckForFile( path, "ccx" ) )
#endif
    {
        m_CalculiXFound = true;
        m_CalculiXPath = path;
    }
    else // Check for ccx in path
    {
#ifdef WIN32
        system( "ccx > temp.txt" );

        // Get size of temp file
        FILE* fp = fopen( "temp.txt", "r" );
        fseek(fp, 0L, SEEK_END);
        size_t sz = ftell( fp );
        fclose( fp );

        system( "del temp.txt" );

        if ( sz != 0 )
#else
        if ( !system( "which ccx > /dev/null 2>&1" ))
#endif
        {
            m_CalculiXFound = true;
        }
    }
}

void AeroStructSingleton::TransferLoads( FILE * logFile )
{
    Vehicle* veh = VehicleMgr.GetVehicle();
    if ( !veh )
    {
        return;
    }

    //====== Send command to be executed by the system at the command prompt ======//
    vector<string> args;

    args.push_back( "-interp" );

    args.push_back( GetBasename( m_ADBFile ) ); // adb base name.

    args.push_back( GetBasename( m_FEAMeshFile ) ); // FEA mesh base name.

    args.push_back( "-dynp" );
    args.push_back( "150.0"); // Dynamic pressure value

    args.push_back( "-label" );
    args.push_back( "foo"); // Label FEA stuff.


    string cmdStr = m_LoadsProcess.PrettyCmd( veh->GetVSPAEROPath(), veh->GetLOADSCmd(), args );
    if( logFile )
    {
        fprintf( logFile, "%s", cmdStr.c_str() );
    }
    else
    {
        MessageData data;
        data.m_String = "AeroStructMessage";
        data.m_StringVec.push_back( cmdStr );
        MessageMgr::getInstance().Send( "ScreenMgr", NULL, data );
    }

    printf( "%s\n", cmdStr.c_str() );

    m_LoadsProcess.ForkCmd( veh->GetVSPAEROPath(), veh->GetLOADSCmd(), args );

    // ==== MonitorSolverProcess ==== //
    MonitorProcess( logFile, &m_LoadsProcess, "AeroStructMessage" );
}

void AeroStructSingleton::ComputeStructure( FILE * logFile )
{
    Vehicle* veh = VehicleMgr.GetVehicle();
    if ( !veh )
    {
        return;
    }

    //====== Send command to be executed by the system at the command prompt ======//
    vector<string> args;

    args.push_back( GetBasename( m_FEAInputFile ) );


    string cmdStr = m_CalculiXProcess.PrettyCmd( m_CalculiXPath, m_CalculiXCmd, args );
    if( logFile )
    {
        fprintf( logFile, "%s", cmdStr.c_str() );
    }
    else
    {
        MessageData data;
        data.m_String = "AeroStructMessage";
        data.m_StringVec.push_back( cmdStr );
        MessageMgr::getInstance().Send( "ScreenMgr", NULL, data );
    }

    printf( "%s\n", cmdStr.c_str() );

    m_CalculiXProcess.ForkCmd( m_CalculiXPath, m_CalculiXCmd, args );

    printf( "Forked\n" );


    // ==== MonitorSolverProcess ==== //
    MonitorProcess( logFile, &m_CalculiXProcess, "AeroStructMessage" );

}
