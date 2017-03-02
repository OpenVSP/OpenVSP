//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//

// VSPAEROMgr.cpp: VSPAERO Mgr Singleton.
//
//////////////////////////////////////////////////////////////////////

#include "MeshGeom.h"
#include "ParmMgr.h"
#include "StlHelper.h"
#include "VSPAEROMgr.h"
#include "WingGeom.h"

#include "StringUtil.h"
#include "FileUtil.h"

#include <regex>

//==== Constructor ====//
VSPAEROMgrSingleton::VSPAEROMgrSingleton() : ParmContainer()
{
    m_Name = "VSPAEROSettings";

    m_GeomSet.Init( "GeomSet", "VSPAERO", this, 0, 0, 12 );
    m_GeomSet.SetDescript( "Geometry set" );

    m_AnalysisMethod.Init( "AnalysisMethod", "VSPAERO", this, vsp::VSPAERO_ANALYSIS_METHOD::VORTEX_LATTICE, vsp::VSPAERO_ANALYSIS_METHOD::VORTEX_LATTICE, vsp::VSPAERO_ANALYSIS_METHOD::PANEL );
    m_AnalysisMethod.SetDescript( "Analysis method: 0=VLM, 1=Panel" );

    m_LastPanelMeshGeomId = string();

    m_Sref.Init( "Sref", "VSPAERO", this, 100.0, 0.0, 1e12 );
    m_Sref.SetDescript( "Reference area" );

    m_bref.Init( "bref", "VSPAERO", this, 1.0, 0.0, 1e6 );
    m_bref.SetDescript( "Reference span" );

    m_cref.Init( "cref", "VSPAERO", this, 1.0, 0.0, 1e6 );
    m_cref.SetDescript( "Reference chord" );

    m_RefFlag.Init( "RefFlag", "VSPAERO", this, MANUAL_REF, MANUAL_REF, COMPONENT_REF );
    m_RefFlag.SetDescript( "Reference quantity flag" );

    m_CGGeomSet.Init( "MassSet", "VSPAERO", this, 0, 0, 12 );
    m_CGGeomSet.SetDescript( "Mass property set" );

    m_NumMassSlice.Init( "NumMassSlice", "VSPAERO", this, 10, 10, 200 );
    m_NumMassSlice.SetDescript( "Number of mass property slices" );

    m_Xcg.Init( "Xcg", "VSPAERO", this, 0.0, -1.0e12, 1.0e12 );
    m_Xcg.SetDescript( "X Center of Gravity" );

    m_Ycg.Init( "Ycg", "VSPAERO", this, 0.0, -1.0e12, 1.0e12 );
    m_Ycg.SetDescript( "Y Center of Gravity" );

    m_Zcg.Init( "Zcg", "VSPAERO", this, 0.0, -1.0e12, 1.0e12 );
    m_Zcg.SetDescript( "Z Center of Gravity" );


    // Flow Condition
    m_AlphaStart.Init( "AlphaStart", "VSPAERO", this, 1.0, -180, 180 );
    m_AlphaStart.SetDescript( "Angle of attack (Start)" );
    m_AlphaEnd.Init( "AlphaEnd", "VSPAERO", this, 10.0, -180, 180 );
    m_AlphaEnd.SetDescript( "Angle of attack (End)" );
    m_AlphaNpts.Init( "AlphaNpts", "VSPAERO", this, 3, 1, 100 );
    m_AlphaNpts.SetDescript( "Angle of attack (Num Points)" );

    m_BetaStart.Init( "BetaStart", "VSPAERO", this, 0.0, -180, 180 );
    m_BetaStart.SetDescript( "Angle of sideslip (Start)" );
    m_BetaEnd.Init( "BetaEnd", "VSPAERO", this, 0.0, -180, 180 );
    m_BetaEnd.SetDescript( "Angle of sideslip (End)" );
    m_BetaNpts.Init( "BetaNpts", "VSPAERO", this, 1, 1, 100 );
    m_BetaNpts.SetDescript( "Angle of sideslip (Num Points)" );

    m_MachStart.Init( "MachStart", "VSPAERO", this, 0.0, 0.0, 5.0 );
    m_MachStart.SetDescript( "Freestream Mach number (Start)" );
    m_MachEnd.Init( "MachEnd", "VSPAERO", this, 0.0, 0.0, 5.0 );
    m_MachEnd.SetDescript( "Freestream Mach number (End)" );
    m_MachNpts.Init( "MachNpts", "VSPAERO", this, 1, 1, 100 );
    m_MachNpts.SetDescript( "Freestream Mach number (Num Points)" );


    // Case Setup
    m_NCPU.Init( "NCPU", "VSPAERO", this, 4, 1, 255 );
    m_NCPU.SetDescript( "Number of processors to use" );

    //    wake parameters
    m_WakeNumIter.Init( "WakeNumIter", "VSPAERO", this, 5, 1, 255 );
    m_WakeNumIter.SetDescript( "Number of wake iterations to execute, Default = 5" );
    m_WakeAvgStartIter.Init( "WakeAvgStartIter", "VSPAERO", this, 0, 0, 255 );
    m_WakeAvgStartIter.SetDescript( "Iteration at which to START averaging the wake. Default=0 --> No wake averaging" );
    m_WakeSkipUntilIter.Init( "WakeSkipUntilIter", "VSPAERO", this, 0, 0, 255 );
    m_WakeSkipUntilIter.SetDescript( "Iteration at which to START calculating the wake. Default=0 --> Wake calculated on each iteration" );

    m_StabilityCalcFlag.Init( "StabilityCalcFlag", "VSPAERO", this, 0.0, 0.0, 1.0 );
    m_StabilityCalcFlag.SetDescript( "Flag to calculate stability derivatives" );
    m_StabilityCalcFlag = false;

    m_BatchModeFlag.Init( "BatchModeFlag", "VSPAERO", this, true, false, true );
    m_BatchModeFlag.SetDescript( "Flag to calculate in batch mode" );
    m_BatchModeFlag = true;

    m_ForceNewSetupfile.Init( "ForceNewSetupfile", "VSPAERO", this, 0.0, 0.0, 1.0 );
    m_ForceNewSetupfile.SetDescript( "Flag to creation of new setup file in ComputeSolver() even if one exists" );
    m_ForceNewSetupfile = false;

    // This sets all the filename members to the appropriate value (for example: empty strings if there is no vehicle)
    UpdateFilenames();

    m_SolverProcessKill = false;

    // Plot limits
    m_ConvergenceXMinIsManual.Init( "m_ConvergenceXMinIsManual", "VSPAERO", this, 0, 0, 1 );
    m_ConvergenceXMaxIsManual.Init( "m_ConvergenceXMaxIsManual", "VSPAERO", this, 0, 0, 1 );
    m_ConvergenceYMinIsManual.Init( "m_ConvergenceYMinIsManual", "VSPAERO", this, 0, 0, 1 );
    m_ConvergenceYMaxIsManual.Init( "m_ConvergenceYMaxIsManual", "VSPAERO", this, 0, 0, 1 );
    m_ConvergenceXMin.Init( "m_ConvergenceXMin", "VSPAERO", this, -1, -1e12, 1e12 );
    m_ConvergenceXMax.Init( "m_ConvergenceXMax", "VSPAERO", this, 1, -1e12, 1e12 );
    m_ConvergenceYMin.Init( "m_ConvergenceYMin", "VSPAERO", this, -1, -1e12, 1e12 );
    m_ConvergenceYMax.Init( "m_ConvergenceYMax", "VSPAERO", this, 1, -1e12, 1e12 );

    m_LoadDistXMinIsManual.Init( "m_LoadDistXMinIsManual", "VSPAERO", this, 0, 0, 1 );
    m_LoadDistXMaxIsManual.Init( "m_LoadDistXMaxIsManual", "VSPAERO", this, 0, 0, 1 );
    m_LoadDistYMinIsManual.Init( "m_LoadDistYMinIsManual", "VSPAERO", this, 0, 0, 1 );
    m_LoadDistYMaxIsManual.Init( "m_LoadDistYMaxIsManual", "VSPAERO", this, 0, 0, 1 );
    m_LoadDistXMin.Init( "m_LoadDistXMin", "VSPAERO", this, -1, -1e12, 1e12 );
    m_LoadDistXMax.Init( "m_LoadDistXMax", "VSPAERO", this, 1, -1e12, 1e12 );
    m_LoadDistYMin.Init( "m_LoadDistYMin", "VSPAERO", this, -1, -1e12, 1e12 );
    m_LoadDistYMax.Init( "m_LoadDistYMax", "VSPAERO", this, 1, -1e12, 1e12 );

    m_SweepXMinIsManual.Init( "m_SweepXMinIsManual", "VSPAERO", this, 0, 0, 1 );
    m_SweepXMaxIsManual.Init( "m_SweepXMaxIsManual", "VSPAERO", this, 0, 0, 1 );
    m_SweepYMinIsManual.Init( "m_SweepYMinIsManual", "VSPAERO", this, 0, 0, 1 );
    m_SweepYMaxIsManual.Init( "m_SweepYMaxIsManual", "VSPAERO", this, 0, 0, 1 );
    m_SweepXMin.Init( "m_SweepXMin", "VSPAERO", this, -1, -1e12, 1e12 );
    m_SweepXMax.Init( "m_SweepXMax", "VSPAERO", this, 1, -1e12, 1e12 );
    m_SweepYMin.Init( "m_SweepYMin", "VSPAERO", this, -1, -1e12, 1e12 );
    m_SweepYMax.Init( "m_SweepYMax", "VSPAERO", this, 1, -1e12, 1e12 );

    // Run the update cycle to complete the setup
    Update();

}

void VSPAEROMgrSingleton::ParmChanged( Parm* parm_ptr, int type )
{
    Vehicle* veh = VehicleMgr.GetVehicle();

    if ( veh )
    {
        veh->ParmChanged( parm_ptr, type );
    }
}


xmlNodePtr VSPAEROMgrSingleton::EncodeXml( xmlNodePtr & node )
{
    xmlNodePtr VSPAEROsetnode = xmlNewChild( node, NULL, BAD_CAST"VSPAEROSettings", NULL );

    ParmContainer::EncodeXml( VSPAEROsetnode );

    XmlUtil::AddStringNode( VSPAEROsetnode, "ReferenceGeomID", m_RefGeomID );

    return VSPAEROsetnode;
}

xmlNodePtr VSPAEROMgrSingleton::DecodeXml( xmlNodePtr & node )
{
    xmlNodePtr VSPAEROsetnode = XmlUtil::GetNode( node, "VSPAEROSettings", 0 );
    if ( VSPAEROsetnode )
    {
        ParmContainer::DecodeXml( VSPAEROsetnode );
        m_RefGeomID   = XmlUtil::FindString( VSPAEROsetnode, "ReferenceGeomID", m_RefGeomID );
    }

    return VSPAEROsetnode;
}


void VSPAEROMgrSingleton::Update()
{
    if( m_RefFlag() == MANUAL_REF )
    {
        m_Sref.Activate();
        m_bref.Activate();
        m_cref.Activate();
    }
    else
    {
        Geom* refgeom = VehicleMgr.GetVehicle()->FindGeom( m_RefGeomID );

        if( refgeom )
        {
            if( refgeom->GetType().m_Type == MS_WING_GEOM_TYPE )
            {
                WingGeom* refwing = ( WingGeom* ) refgeom;
                m_Sref.Set( refwing->m_TotalArea() );
                m_bref.Set( refwing->m_TotalSpan() );
                m_cref.Set( refwing->m_TotalChord() );

                m_Sref.Deactivate();
                m_bref.Deactivate();
                m_cref.Deactivate();
            }
        }
        else
        {
            m_RefGeomID = string();
        }
    }

    UpdateFilenames();
}

void VSPAEROMgrSingleton::UpdateFilenames()    //A.K.A. SetupDegenFile()
{
    // Initialize these to blanks.  if any of the checks fail the variables will at least contain an empty string
    m_ModelNameBase     = string();
    m_DegenFileFull     = string();
    m_CompGeomFileFull  = string();     // TODO this is set from the get export name
    m_SetupFile         = string();
    m_AdbFile           = string();
    m_HistoryFile       = string();
    m_LoadFile          = string();
    m_StabFile          = string();

    Vehicle *veh = VehicleMgr.GetVehicle();
    if( veh )
    {
        // Generate the base name based on the vsp3filename without the extension
        int pos = -1;
        switch ( m_AnalysisMethod.Get() )
        {
        case vsp::VSPAERO_ANALYSIS_METHOD::VORTEX_LATTICE:
            // The base_name is dependent on the DegenFileName
            // TODO extra "_DegenGeom" is added to the m_ModelBase
            m_DegenFileFull = veh->getExportFileName( vsp::DEGEN_GEOM_CSV_TYPE );

            m_ModelNameBase = m_DegenFileFull;
            pos = m_ModelNameBase.find( ".csv" );
            if ( pos >= 0 )
            {
                m_ModelNameBase.erase( pos, m_ModelNameBase.length() - 1 );
            }

            m_CompGeomFileFull  = string(); //This file is not used for vortex lattice analysis
            m_SetupFile         = m_ModelNameBase + string( ".vspaero" );
            m_AdbFile           = m_ModelNameBase + string( ".adb" );
            m_HistoryFile       = m_ModelNameBase + string( ".history" );
            m_LoadFile          = m_ModelNameBase + string( ".lod" );
            m_StabFile          = m_ModelNameBase + string( ".stab" );

            break;

        case vsp::VSPAERO_ANALYSIS_METHOD::PANEL:
            m_CompGeomFileFull = veh->getExportFileName( vsp::VSPAERO_PANEL_TRI_TYPE );

            m_ModelNameBase = m_CompGeomFileFull;
            pos = m_ModelNameBase.find( ".tri" );
            if ( pos >= 0 )
            {
                m_ModelNameBase.erase( pos, m_ModelNameBase.length() - 1 );
            }

            m_DegenFileFull     = m_ModelNameBase + string( "_DegenGeom.csv" );
            m_SetupFile         = m_ModelNameBase + string( ".vspaero" );
            m_AdbFile           = m_ModelNameBase + string( ".adb" );
            m_HistoryFile       = m_ModelNameBase + string( ".history" );
            m_LoadFile          = m_ModelNameBase + string( ".lod" );
            m_StabFile          = m_ModelNameBase + string( ".stab" );

            break;

        default:
            // TODO print out an error here
            break;
        }

    }

}

string VSPAEROMgrSingleton::ComputeGeometry()
{
    Vehicle *veh = VehicleMgr.GetVehicle();
    if ( !veh )
    {
        fprintf( stderr, "ERROR: Unable to get vehicle \n\tFile: %s \tLine:%d\n", __FILE__, __LINE__ );
        return string();
    }

    veh->CreateDegenGeom( VSPAEROMgr.m_GeomSet() );


    // record original values
    bool exptMfile_orig = veh->getExportDegenGeomMFile();
    bool exptCSVfile_orig = veh->getExportDegenGeomCsvFile();
    veh->setExportDegenGeomMFile( false );
    veh->setExportDegenGeomCsvFile( true );

    UpdateFilenames();

    // Note: while in panel mode the degen file required by vspaero is
    // dependent on the tri filename and not necessarily what the current
    // setting is for the vsp::DEGEN_GEOM_CSV_TYPE
    string degenGeomFile_orig = veh->getExportFileName( vsp::DEGEN_GEOM_CSV_TYPE );
    veh->setExportFileName( vsp::DEGEN_GEOM_CSV_TYPE, m_DegenFileFull );

    veh->WriteDegenGeomFile();

    // restore original values
    veh->setExportDegenGeomMFile( exptMfile_orig );
    veh->setExportDegenGeomCsvFile( exptCSVfile_orig );
    veh->setExportFileName( vsp::DEGEN_GEOM_CSV_TYPE, degenGeomFile_orig );

    WaitForFile( m_DegenFileFull );
    if ( !FileExist( m_DegenFileFull ) )
    {
        fprintf( stderr, "WARNING: DegenGeom file not found: %s\n\tFile: %s \tLine:%d\n", m_DegenFileFull.c_str(), __FILE__, __LINE__ );
    }

    // Generate *.tri geometry file for Panel method
    if ( m_AnalysisMethod.Get() == vsp::VSPAERO_ANALYSIS_METHOD::PANEL )
    {
        // Cleanup previously created meshGeom IDs created from VSPAEROMgr
        if ( veh->FindGeom( m_LastPanelMeshGeomId ) )
        {
            veh->DeleteGeom( m_LastPanelMeshGeomId );
        }

        // Compute intersected and trimmed geometry
        int halfFlag = 0;
        m_LastPanelMeshGeomId = veh->CompGeomAndFlatten( VSPAEROMgr.m_GeomSet(), halfFlag );

        // After CompGeomAndFlatten() is run all the geometry is hidden and the intersected & trimmed mesh is the only one shown
        veh->WriteTRIFile( m_CompGeomFileFull , vsp::SET_TYPE::SET_SHOWN );
        WaitForFile( m_CompGeomFileFull );
        if ( !FileExist( m_CompGeomFileFull ) )
        {
            fprintf( stderr, "WARNING: CompGeom file not found: %s\n\tFile: %s \tLine:%d\n", m_CompGeomFileFull.c_str(), __FILE__, __LINE__ );
        }

    }

    // Clear previous results
    while ( ResultsMgr.GetNumResults( "VSPAERO_Geom" ) > 0 )
    {
        ResultsMgr.DeleteResult( ResultsMgr.FindResultsID( "VSPAERO_Geom",  0 ) );
    }
    // Write out new results
    Results* res = ResultsMgr.CreateResults( "VSPAERO_Geom" );
    if ( !res )
    {
        fprintf( stderr, "ERROR: Unable to create result in result manager \n\tFile: %s \tLine:%d\n", __FILE__, __LINE__ );
        return string();
    }
    res->Add( NameValData( "GeometrySet", VSPAEROMgr.m_GeomSet() ) );
    res->Add( NameValData( "AnalysisMethod", m_AnalysisMethod.Get() ) );
    res->Add( NameValData( "DegenGeomFileName", m_DegenFileFull ) );
    if ( m_AnalysisMethod.Get() == vsp::VSPAERO_ANALYSIS_METHOD::PANEL )
    {
        res->Add( NameValData( "CompGeomFileName", m_CompGeomFileFull ) );
        res->Add( NameValData( "Mesh_GeomID", m_LastPanelMeshGeomId ) );
    }
    else
    {
        res->Add( NameValData( "CompGeomFileName", string() ) );
        res->Add( NameValData( "Mesh_GeomID", string() ) );
    }

    return res->GetID();

}

/* TODO - finish implementation of generating the setup file from the VSPAEROMgr*/
void VSPAEROMgrSingleton::CreateSetupFile( FILE * logFile )
{
    UpdateFilenames();

    Vehicle *veh = VehicleMgr.GetVehicle();
    if ( !veh )
    {
        fprintf( stderr, "ERROR %d: Unable to get vehicle \n\tFile: %s \tLine:%d\n", vsp::VSP_INVALID_PTR, __FILE__, __LINE__ );
        return;
    }

    // Clear existing serup file
    if ( FileExist( m_SetupFile ) )
    {
        remove( m_SetupFile.c_str() );
    }

    vector<string> args;
    args.push_back( "-setup" );

    args.push_back( "-sref" );
    args.push_back( StringUtil::double_to_string( m_Sref(), "%f" ) );

    args.push_back( "-bref" );
    args.push_back( StringUtil::double_to_string( m_bref(), "%f" ) );

    args.push_back( "-cref" );
    args.push_back( StringUtil::double_to_string( m_cref(), "%f" ) );

    args.push_back( "-cg" );
    args.push_back( StringUtil::double_to_string( m_Xcg(), "%f" ) );
    args.push_back( StringUtil::double_to_string( m_Ycg(), "%f" ) );
    args.push_back( StringUtil::double_to_string( m_Zcg(), "%f" ) );

    // If the GUI is selected for batch calculation write the setup file with
    // the entire vector of mach alpha and beta points otherwise just write the
    // starting freestream condition.  This will give the easiest to use setup
    // file representing the options in the GUI.
    if ( m_BatchModeFlag.Get() )
    {
        vector<double> alphaVec;
        vector<double> betaVec;
        vector<double> machVec;
        GetSweepVectors( alphaVec, betaVec, machVec );

        //====== Loop over flight conditions and solve ======//
        // Mach
        args.push_back( "-mach" );
        for ( int iMach = 0; iMach < machVec.size(); iMach++ )
        {
            args.push_back( StringUtil::double_to_string( machVec[iMach], "%f " ) );
        }
        args.push_back( "END" );

        // Alpha
        args.push_back( "-aoa" );
        for ( int iAlpha = 0; iAlpha < alphaVec.size(); iAlpha++ )
        {
            args.push_back( StringUtil::double_to_string( alphaVec[iAlpha], "%f " ) );
        }
        args.push_back( "END" );

        // Beta
        args.push_back( "-beta" );
        for ( int iBeta = 0; iBeta < betaVec.size(); iBeta++ )
        {
            args.push_back( StringUtil::double_to_string( betaVec[iBeta], "%f " ) );
        }
        args.push_back( "END" );
    }
    else
    {
        args.push_back( "-aoa" );
        args.push_back( StringUtil::double_to_string( m_AlphaStart(), "%f" ) );
        args.push_back( "END" );

        args.push_back( "-beta" );
        args.push_back( StringUtil::double_to_string( m_BetaStart(), "%f" ) );
        args.push_back( "END" );

        args.push_back( "-mach" );
        args.push_back( StringUtil::double_to_string( m_MachStart(), "%f" ) );
        args.push_back( "END" );
    }



    args.push_back( "-wakeiters" );
    args.push_back( StringUtil::int_to_string( m_WakeNumIter(), "%d" ) );

    args.push_back( m_ModelNameBase );

    //Print out execute command
    string cmdStr = m_SolverProcess.PrettyCmd( veh->GetExePath(), veh->GetVSPAEROCmd(), args );
    if( logFile )
    {
        fprintf( logFile, "%s", cmdStr.c_str() );
    }
    else
    {
        MessageData data;
        data.m_String = "VSPAEROSolverMessage";
        data.m_StringVec.push_back( cmdStr );
        MessageMgr::getInstance().Send( "ScreenMgr", NULL, data );
    }

    // Execute VSPAero
    m_SolverProcess.ForkCmd( veh->GetExePath(), veh->GetVSPAEROCmd(), args );

    // ==== MonitorSolverProcess ==== //
    MonitorSolver( logFile );

    // Check if the kill solver flag has been raised, if so clean up and return
    //  note: we could have exited the IsRunning loop if the process was killed
    if( m_SolverProcessKill )
    {
        m_SolverProcessKill = false;    //reset kill flag
    }

    // Wait until the setup file shows up on the file system
    WaitForFile( m_SetupFile );

    if ( !FileExist( m_SetupFile ) )
    {
        // shouldn't be able to get here but create a setup file with the correct settings
        fprintf( stderr, "ERROR %d: setup file not found, file %s\n\tFile: %s \tLine:%d\n", vsp::VSP_FILE_DOES_NOT_EXIST, m_SetupFile.c_str(), __FILE__, __LINE__ );
    }

    // Send the message to update the screens
    MessageData data;
    data.m_String = "UpdateAllScreens";
    MessageMgr::getInstance().Send( "ScreenMgr", NULL, data );

}

void VSPAEROMgrSingleton::ClearAllPreviousResults()
{
    while ( ResultsMgr.GetNumResults( "VSPAERO_History" ) > 0 )
    {
        ResultsMgr.DeleteResult( ResultsMgr.FindResultsID( "VSPAERO_History",  0 ) );
    }
    while ( ResultsMgr.GetNumResults( "VSPAERO_Load" ) > 0 )
    {
        ResultsMgr.DeleteResult( ResultsMgr.FindResultsID( "VSPAERO_Load",  0 ) );
    }
    while ( ResultsMgr.GetNumResults( "VSPAERO_Stab" ) > 0 )
    {
        ResultsMgr.DeleteResult( ResultsMgr.FindResultsID( "VSPAERO_Stab",  0 ) );
    }
    while ( ResultsMgr.GetNumResults( "VSPAERO_Wrapper" ) > 0 )
    {
        ResultsMgr.DeleteResult( ResultsMgr.FindResultsID( "VSPAERO_Wrapper",  0 ) );
    }
}

void VSPAEROMgrSingleton::GetSweepVectors( vector<double> &alphaVec, vector<double> &betaVec, vector<double> &machVec )
{
    // grab current parm values
    double alphaStart = m_AlphaStart.Get();
    double alphaEnd = m_AlphaEnd.Get();
    int alphaNpts = m_AlphaNpts.Get();

    double betaStart = m_BetaStart.Get();
    double betaEnd = m_BetaEnd.Get();
    int betaNpts = m_BetaNpts.Get();

    double machStart = m_MachStart.Get();
    double machEnd = m_MachEnd.Get();
    int machNpts = m_MachNpts.Get();

    // Calculate spacing
    double alphaDelta = 0.0;
    if ( alphaNpts > 1 )
    {
        alphaDelta = ( alphaEnd - alphaStart ) / ( alphaNpts - 1.0 );
    }
    for ( int iAlpha = 0; iAlpha < alphaNpts; iAlpha++ )
    {
        //Set current alpha value
        alphaVec.push_back( alphaStart + double( iAlpha ) * alphaDelta );
    }

    double betaDelta = 0.0;
    if ( betaNpts > 1 )
    {
        betaDelta = ( betaEnd - betaStart ) / ( betaNpts - 1.0 );
    }
    for ( int iBeta = 0; iBeta < betaNpts; iBeta++ )
    {
        //Set current alpha value
        betaVec.push_back( betaStart + double( iBeta ) * betaDelta );
    }

    double machDelta = 0.0;
    if ( machNpts > 1 )
    {
        machDelta = ( machEnd - machStart ) / ( machNpts - 1.0 );
    }
    for ( int iMach = 0; iMach < machNpts; iMach++ )
    {
        //Set current alpha value
        machVec.push_back( machStart + double( iMach ) * machDelta );
    }
}

/* ComputeSolver(FILE * logFile)
Returns a result with a vector of results id's under the name ResultVec
Optional input of logFile allows outputting to a log file or the console
*/
string VSPAEROMgrSingleton::ComputeSolver( FILE * logFile )
{
    UpdateFilenames();
    if ( m_BatchModeFlag.Get() )
    {
        return ComputeSolverBatch( logFile );
    }
    else
    {
        return ComputeSolverSingle( logFile );
    }
}

/* ComputeSolverSingle(FILE * logFile)
*/
string VSPAEROMgrSingleton::ComputeSolverSingle( FILE * logFile )
{
    std::vector <string> res_id_vector;

    Vehicle *veh = VehicleMgr.GetVehicle();

    if ( veh )
    {

        string adbFileName = m_AdbFile;
        string historyFileName = m_HistoryFile;
        string loadFileName = m_LoadFile;
        string stabFileName = m_StabFile;
        string modelNameBase = m_ModelNameBase;

        bool stabilityFlag = m_StabilityCalcFlag.Get();
        vsp::VSPAERO_ANALYSIS_METHOD analysisMethod = ( vsp::VSPAERO_ANALYSIS_METHOD )m_AnalysisMethod.Get();

        int ncpu = m_NCPU.Get();

        int wakeAvgStartIter = m_WakeAvgStartIter.Get();
        int wakeSkipUntilIter = m_WakeSkipUntilIter.Get();


        //====== Modify/Update the setup file ======//
        if ( !FileExist( m_SetupFile ) || m_ForceNewSetupfile.Get() )
        {
            // if the setup file doesn't exist, create one with the current settings
            // TODO output a warning to the user that we are creating a default file
            CreateSetupFile();
        }

        vector<double> alphaVec;
        vector<double> betaVec;
        vector<double> machVec;
        GetSweepVectors( alphaVec, betaVec, machVec );

        //====== Loop over flight conditions and solve ======//
        // TODO make this into a case list with a single loop
        for ( int iAlpha = 0; iAlpha < alphaVec.size(); iAlpha++ )
        {
            //Set current alpha value
            double current_alpha = alphaVec[iAlpha];

            for ( int iBeta = 0; iBeta < betaVec.size(); iBeta++ )
            {
                //Set current beta value
                double current_beta = betaVec[iBeta];

                for ( int iMach = 0; iMach < machVec.size(); iMach++ )
                {
                    //Set current mach value
                    double current_mach = machVec[iMach];

                    //====== Clear VSPAERO output files ======//
                    if ( FileExist( adbFileName ) )
                    {
                        remove( adbFileName.c_str() );
                    }
                    if ( FileExist( historyFileName ) )
                    {
                        remove( historyFileName.c_str() );
                    }
                    if ( FileExist( loadFileName ) )
                    {
                        remove( loadFileName.c_str() );
                    }
                    if ( FileExist( stabFileName ) )
                    {
                        remove( stabFileName.c_str() );
                    }

                    //====== Send command to be executed by the system at the command prompt ======//
                    vector<string> args;
                    // Set mach, alpha, beta (save to local "current*" variables to use as header information in the results manager)
                    args.push_back( "-fs" );       // "freestream" override flag
                    args.push_back( StringUtil::double_to_string( current_mach, "%f" ) );
                    args.push_back( "END" );
                    args.push_back( StringUtil::double_to_string( current_alpha, "%f" ) );
                    args.push_back( "END" );
                    args.push_back( StringUtil::double_to_string( current_beta, "%f" ) );
                    args.push_back( "END" );
                    // Set number of openmp threads
                    args.push_back( "-omp" );
                    args.push_back( StringUtil::int_to_string( m_NCPU.Get(), "%d" ) );
                    // Set stability run arguments
                    if ( stabilityFlag )
                    {
                        args.push_back( "-stab" );
                    }
                    // Force averaging startign at wake iteration N
                    if( wakeAvgStartIter >= 1 )
                    {
                        args.push_back( "-avg" );
                        args.push_back( StringUtil::int_to_string( wakeAvgStartIter, "%d" ) );
                    }
                    if( wakeSkipUntilIter >= 1 )
                    {
                        // No wake for first N iterations
                        args.push_back( "-nowake" );
                        args.push_back( StringUtil::int_to_string( wakeSkipUntilIter, "%d" ) );
                    }

                    // Add model file name
                    args.push_back( modelNameBase );

                    //Print out execute command
                    string cmdStr = m_SolverProcess.PrettyCmd( veh->GetExePath(), veh->GetVSPAEROCmd(), args );
                    if( logFile )
                    {
                        fprintf( logFile, "%s", cmdStr.c_str() );
                    }
                    else
                    {
                        MessageData data;
                        data.m_String = "VSPAEROSolverMessage";
                        data.m_StringVec.push_back( cmdStr );
                        MessageMgr::getInstance().Send( "ScreenMgr", NULL, data );
                    }

                    // Execute VSPAero
                    m_SolverProcess.ForkCmd( veh->GetExePath(), veh->GetVSPAEROCmd(), args );

                    // ==== MonitorSolverProcess ==== //
                    MonitorSolver( logFile );


                    // Check if the kill solver flag has been raised, if so clean up and return
                    //  note: we could have exited the IsRunning loop if the process was killed
                    if( m_SolverProcessKill )
                    {
                        m_SolverProcessKill = false;    //reset kill flag

                        return string();    //return empty result ID vector
                    }

                    //====== Read in all of the results ======//
                    // read the files if there is new data that has not successfully been read in yet
                    ReadHistoryFile( historyFileName, res_id_vector, analysisMethod );
                    ReadLoadFile( loadFileName, res_id_vector, analysisMethod );
                    if ( stabilityFlag )
                    {
                        ReadStabFile( stabFileName, res_id_vector, analysisMethod );      //*.STAB stability coeff file
                    }

                    // Send the message to update the screens
                    MessageData data;
                    data.m_String = "UpdateAllScreens";
                    MessageMgr::getInstance().Send( "ScreenMgr", NULL, data );

                }    //Mach sweep loop

            }    //beta sweep loop

        }    //alpha sweep loop

    }

    // Create "wrapper" result to contain a vector of result IDs (this maintains compatibility to return a single result after computation)
    Results *res = ResultsMgr.CreateResults( "VSPAERO_Wrapper" );
    if( !res )
    {
        return string();
    }
    else
    {
        res->Add( NameValData( "ResultsVec", res_id_vector ) );
        return res->GetID();
    }
}

/* ComputeSolverBatch(FILE * logFile)
*/
string VSPAEROMgrSingleton::ComputeSolverBatch( FILE * logFile )
{
    std::vector <string> res_id_vector;

    Vehicle *veh = VehicleMgr.GetVehicle();

    if ( veh )
    {

        string adbFileName = m_AdbFile;
        string historyFileName = m_HistoryFile;
        string loadFileName = m_LoadFile;
        string stabFileName = m_StabFile;
        string modelNameBase = m_ModelNameBase;

        bool stabilityFlag = m_StabilityCalcFlag.Get();
        vsp::VSPAERO_ANALYSIS_METHOD analysisMethod = ( vsp::VSPAERO_ANALYSIS_METHOD )m_AnalysisMethod.Get();

        int ncpu = m_NCPU.Get();

        int wakeAvgStartIter = m_WakeAvgStartIter.Get();
        int wakeSkipUntilIter = m_WakeSkipUntilIter.Get();


        //====== Modify/Update the setup file ======//
        if ( !FileExist( m_SetupFile ) || m_ForceNewSetupfile.Get() )
        {
            // if the setup file doesn't exist, create one with the current settings
            // TODO output a warning to the user that we are creating a default file
            CreateSetupFile();
        }

        vector<double> alphaVec;
        vector<double> betaVec;
        vector<double> machVec;
        GetSweepVectors( alphaVec, betaVec, machVec );

        //====== Clear VSPAERO output files ======//
        if ( FileExist( adbFileName ) )
        {
            remove( adbFileName.c_str() );
        }
        if ( FileExist( historyFileName ) )
        {
            remove( historyFileName.c_str() );
        }
        if ( FileExist( loadFileName ) )
        {
            remove( loadFileName.c_str() );
        }
        if ( FileExist( stabFileName ) )
        {
            remove( stabFileName.c_str() );
        }

        //====== generate batch mode command to be executed by the system at the command prompt ======//
        vector<string> args;
        // Set mach, alpha, beta (save to local "current*" variables to use as header information in the results manager)
        args.push_back( "-fs" );       // "freestream" override flag

        //====== Loop over flight conditions and solve ======//
        // Mach
        for ( int iMach = 0; iMach < machVec.size(); iMach++ )
        {
            args.push_back( StringUtil::double_to_string( machVec[iMach], "%f " ) );
        }
        args.push_back( "END" );
        // Alpha
        for ( int iAlpha = 0; iAlpha < alphaVec.size(); iAlpha++ )
        {
            args.push_back( StringUtil::double_to_string( alphaVec[iAlpha], "%f " ) );
        }
        args.push_back( "END" );
        // Beta
        for ( int iBeta = 0; iBeta < betaVec.size(); iBeta++ )
        {
            args.push_back( StringUtil::double_to_string( betaVec[iBeta], "%f " ) );
        }
        args.push_back( "END" );

        // Set number of openmp threads
        args.push_back( "-omp" );
        args.push_back( StringUtil::int_to_string( m_NCPU.Get(), "%d" ) );
        // Set stability run arguments
        if ( stabilityFlag )
        {
            args.push_back( "-stab" );
        }
        // Force averaging startign at wake iteration N
        if( wakeAvgStartIter >= 1 )
        {
            args.push_back( "-avg" );
            args.push_back( StringUtil::int_to_string( wakeAvgStartIter, "%d" ) );
        }
        if( wakeSkipUntilIter >= 1 )
        {
            // No wake for first N iterations
            args.push_back( "-nowake" );
            args.push_back( StringUtil::int_to_string( wakeSkipUntilIter, "%d" ) );
        }

        // Add model file name
        args.push_back( modelNameBase );

        //Print out execute command
        string cmdStr = m_SolverProcess.PrettyCmd( veh->GetExePath(), veh->GetVSPAEROCmd(), args );
        if( logFile )
        {
            fprintf( logFile, "%s", cmdStr.c_str() );
        }
        else
        {
            MessageData data;
            data.m_String = "VSPAEROSolverMessage";
            data.m_StringVec.push_back( cmdStr );
            MessageMgr::getInstance().Send( "ScreenMgr", NULL, data );
        }

        // Execute VSPAero
        m_SolverProcess.ForkCmd( veh->GetExePath(), veh->GetVSPAEROCmd(), args );

        // ==== MonitorSolverProcess ==== //
        MonitorSolver( logFile );


        // Check if the kill solver flag has been raised, if so clean up and return
        //  note: we could have exited the IsRunning loop if the process was killed
        if( m_SolverProcessKill )
        {
            m_SolverProcessKill = false;    //reset kill flag

            return string();    //return empty result ID vector
        }

        //====== Read in all of the results ======//
        ReadHistoryFile( historyFileName, res_id_vector, analysisMethod );
        ReadLoadFile( loadFileName, res_id_vector, analysisMethod );
        if ( stabilityFlag )
        {
            ReadStabFile( stabFileName, res_id_vector, analysisMethod );      //*.STAB stability coeff file
        }

        // Send the message to update the screens
        MessageData data;
        data.m_String = "UpdateAllScreens";
        MessageMgr::getInstance().Send( "ScreenMgr", NULL, data );

    }

    // Create "wrapper" result to contain a vector of result IDs (this maintains compatibility to return a single result after computation)
    Results *res = ResultsMgr.CreateResults( "VSPAERO_Wrapper" );
    if( !res )
    {
        return string();
    }
    else
    {
        res->Add( NameValData( "ResultsVec", res_id_vector ) );
        return res->GetID();
    }
}

void VSPAEROMgrSingleton::MonitorSolver( FILE * logFile )
{
    // ==== MonitorSolverProcess ==== //
    int bufsize = 1000;
    char *buf;
    buf = ( char* ) malloc( sizeof( char ) * ( bufsize + 1 ) );
    unsigned long nread = 1;
    bool runflag = m_SolverProcess.IsRunning();
    while ( runflag || nread > 0 )
    {
        m_SolverProcess.ReadStdoutPipe( buf, bufsize, &nread );
        if( nread > 0 )
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
                    data.m_String = "VSPAEROSolverMessage";
                    data.m_StringVec.push_back( string( buf ) );
                    MessageMgr::getInstance().Send( "ScreenMgr", NULL, data );
                }
            }
        }

        SleepForMilliseconds( 100 );
        runflag = m_SolverProcess.IsRunning();
    }
}

void VSPAEROMgrSingleton::AddResultHeader( string res_id, double mach, double alpha, double beta, vsp::VSPAERO_ANALYSIS_METHOD analysisMethod )
{
    // Add Flow Condition header to each result
    Results * res = ResultsMgr.FindResultsPtr( res_id );
    if ( res )
    {
        res->Add( NameValData( "AnalysisMethod", analysisMethod ) );
    }
}

// helper thread functions for VSPAERO GUI interface and multi-threaded impleentation
bool VSPAEROMgrSingleton::IsSolverRunning()
{
    return m_SolverProcess.IsRunning();
}

void VSPAEROMgrSingleton::KillSolver()
{
    // Raise flag to break the compute solver thread
    m_SolverProcessKill = true;
    return m_SolverProcess.Kill();
}

ProcessUtil* VSPAEROMgrSingleton::GetSolverProcess()
{
    return &m_SolverProcess;
}

// function is used to wait for the result to show up on the file system
void VSPAEROMgrSingleton::WaitForFile( string filename )
{
    // Wait until the results show up on the file system
    int n_wait = 0;
    // wait no more than 5 seconds = (50*100)/1000
    while ( ( !FileExist( filename ) ) & ( n_wait < 50 ) )
    {
        n_wait++;
        SleepForMilliseconds( 100 );
    }
    SleepForMilliseconds( 100 );  //additional wait for file
}

/*******************************************************
Read .HISTORY file output from VSPAERO
analysisMethod is passed in because the parm it is set by might change by the time we are done calculating the solution
See: VSP_Solver.C in vspaero project
line 4351 - void VSP_SOLVER::OutputStatusFile(int Type)
line 4407 - void VSP_SOLVER::OutputZeroLiftDragToStatusFile(void)
TODO:
- Update this function to use the generic table read as used in: string VSPAEROMgrSingleton::ReadStabFile()
*******************************************************/
void VSPAEROMgrSingleton::ReadHistoryFile( string filename, vector <string> &res_id_vector, vsp::VSPAERO_ANALYSIS_METHOD analysisMethod )
{
    //TODO return success or failure
    FILE *fp = NULL;
    //size_t result;
    bool read_success = false;

    //HISTORY file
    WaitForFile( filename );
    fp = fopen( filename.c_str(), "r" );
    if ( fp == NULL )
    {
        fprintf( stderr, "ERROR %d: Could not open History file: %s\n\tFile: %s \tLine:%d\n", vsp::VSP_FILE_DOES_NOT_EXIST, m_HistoryFile.c_str(), __FILE__, __LINE__ );
        return;
    }

    Results* res = NULL;
    std::vector<string> data_string_array;

    char seps[]   = " :,\t\n";
    while ( !feof( fp ) )
    {
        data_string_array = ReadDelimLine( fp, seps ); //this is also done in some of the embedded loops below

        if ( CheckForCaseHeader( data_string_array ) )
        {
            res = ResultsMgr.CreateResults( "VSPAERO_History" );
            res_id_vector.push_back( res->GetID() );

            if ( ReadVSPAEROCaseHeader( res, fp, analysisMethod ) != 0 )
            {
                // Failed to read the case header
                fprintf( stderr, "ERROR %d: Could not read case header in VSPAERO file: %s\n\tFile: %s \tLine:%d\n", vsp::VSP_FILE_READ_FAILURE, m_StabFile.c_str(), __FILE__, __LINE__ );
                return;
            }

        }

        //READ wake iteration table
        /* Example wake iteration table
        Iter      Mach       AoA      Beta       CL         CDo       CDi      CDtot      CS        L/D        E        CFx       CFy       CFz       CMx       CMy       CMz       T/QS
        1   0.00000   1.00000   0.00000   0.03329   0.00364   0.00009   0.00373  -0.00000   8.93773 395.42033  -0.00049  -0.00000   0.03329  -0.00000  -0.09836  -0.00000   0.00000
        2   0.00000   1.00000   0.00000   0.03329   0.00364   0.00009   0.00373  -0.00000   8.93494 394.87228  -0.00049  -0.00000   0.03328  -0.00000  -0.09834  -0.00000   0.00000
        ...
        */
        int wake_iter_table_columns = 18;
        if( data_string_array.size() == wake_iter_table_columns )
        {
            //discard the header row and read the next line assuming that it is numeric
            data_string_array = ReadDelimLine( fp, seps );

            // create new vectors for this set of results information
            std::vector<int> i;
            std::vector<double> Mach;
            std::vector<double> Alpha;
            std::vector<double> Beta;
            std::vector<double> CL;
            std::vector<double> CDo;
            std::vector<double> CDi;
            std::vector<double> CDtot;
            std::vector<double> CS;
            std::vector<double> LoD;
            std::vector<double> E;
            std::vector<double> CFx;
            std::vector<double> CFy;
            std::vector<double> CFz;
            std::vector<double> CMx;
            std::vector<double> CMy;
            std::vector<double> CMz;
            std::vector<double> ToQS;

            while ( data_string_array.size() == wake_iter_table_columns )
            {
                i.push_back(        std::stoi( data_string_array[0] ) );

                Mach.push_back(     std::stod( data_string_array[1] ) );
                Alpha.push_back(    std::stod( data_string_array[2] ) );
                Beta.push_back(     std::stod( data_string_array[3] ) );

                CL.push_back(       std::stod( data_string_array[4] ) );
                CDo.push_back(      std::stod( data_string_array[5] ) );
                CDi.push_back(      std::stod( data_string_array[6] ) );
                CDtot.push_back(    std::stod( data_string_array[7] ) );
                CS.push_back(       std::stod( data_string_array[8] ) );

                LoD.push_back(      std::stod( data_string_array[9] ) );
                E.push_back(        std::stod( data_string_array[10] ) );

                CFx.push_back(      std::stod( data_string_array[11] ) );
                CFy.push_back(      std::stod( data_string_array[12] ) );
                CFz.push_back(      std::stod( data_string_array[13] ) );

                CMx.push_back(      std::stod( data_string_array[14] ) );
                CMy.push_back(      std::stod( data_string_array[15] ) );
                CMz.push_back(      std::stod( data_string_array[16] ) );

                ToQS.push_back(     std::stod( data_string_array[17] ) );

                data_string_array = ReadDelimLine( fp, seps );
            }

            //add to the results manager
            if ( res )
            {
                res->Add( NameValData( "WakeIter", i ) );
                res->Add( NameValData( "Mach", Mach ) );
                res->Add( NameValData( "Alpha", Alpha ) );
                res->Add( NameValData( "Beta", Beta ) );
                res->Add( NameValData( "CL", CL ) );
                res->Add( NameValData( "CDo", CDo ) );
                res->Add( NameValData( "CDi", CDi ) );
                res->Add( NameValData( "CDtot", CDtot ) );
                res->Add( NameValData( "CS", CS ) );
                res->Add( NameValData( "L/D", LoD ) );
                res->Add( NameValData( "E", E ) );
                res->Add( NameValData( "CFx", CFx ) );
                res->Add( NameValData( "CFy", CFy ) );
                res->Add( NameValData( "CFz", CFz ) );
                res->Add( NameValData( "CMx", CMx ) );
                res->Add( NameValData( "CMy", CMy ) );
                res->Add( NameValData( "CMz", CMz ) );
                res->Add( NameValData( "T/QS", ToQS ) );
            }

        } // end of wake iteration

    } //end feof loop to read entire history file

    fclose ( fp );

    return;
}

/*******************************************************
Read .LOD file output from VSPAERO
See: VSP_Solver.C in vspaero project
line 2851 - void VSP_SOLVER::CalculateSpanWiseLoading(void)
TODO:
- Update this function to use the generic table read as used in: string VSPAEROMgrSingleton::ReadStabFile()
- Read in Component table information, this is the 2nd table at the bottom of the .lod file
*******************************************************/
void VSPAEROMgrSingleton::ReadLoadFile( string filename, vector <string> &res_id_vector, vsp::VSPAERO_ANALYSIS_METHOD analysisMethod )
{
    FILE *fp = NULL;
    bool read_success = false;

    //LOAD file
    WaitForFile( filename );
    fp = fopen( filename.c_str(), "r" );
    if ( fp == NULL )
    {
        fprintf( stderr, "ERROR %d: Could not open Load file: %s\n\tFile: %s \tLine:%d\n", vsp::VSP_FILE_DOES_NOT_EXIST, m_LoadFile.c_str(), __FILE__, __LINE__ );
        return;
    }

    Results* res = NULL;
    std::vector< std::string > data_string_array;
    std::vector< std::vector< double > > data_array;

    double cref = 1.0;

    char seps[]   = " :,\t\n";
    while ( !feof( fp ) )
    {
        data_string_array = ReadDelimLine( fp, seps ); //this is also done in some of the embedded loops below

        if ( CheckForCaseHeader( data_string_array ) )
        {
            res = ResultsMgr.CreateResults( "VSPAERO_Load" );
            res_id_vector.push_back( res->GetID() );

            if ( ReadVSPAEROCaseHeader( res, fp, analysisMethod ) != 0 )
            {
                // Failed to read the case header
                fprintf( stderr, "ERROR %d: Could not read case header in VSPAERO file: %s\n\tFile: %s \tLine:%d\n", vsp::VSP_FILE_READ_FAILURE, m_StabFile.c_str(), __FILE__, __LINE__ );
                return;
            }

            cref = res->FindPtr( "FC_Cref_" )->GetDouble( 0 );

        }

        // Sectional distribution table
        int nSectionalDataTableCols = 13;
        if ( data_string_array.size() == nSectionalDataTableCols )
        {
            //discard the header row and read the next line assuming that it is numeric
            data_string_array = ReadDelimLine( fp, seps );

            // Raw data vectors
            std::vector<int> WingId;
            std::vector<double> Yavg;
            std::vector<double> Chord;
            std::vector<double> VoVinf;
            std::vector<double> Cl;
            std::vector<double> Cd;
            std::vector<double> Cs;
            std::vector<double> Cx;
            std::vector<double> Cy;
            std::vector<double> Cz;
            std::vector<double> Cmx;
            std::vector<double> Cmy;
            std::vector<double> Cmz;

            //normalized by local chord
            std::vector<double> Clc_cref;
            std::vector<double> Cdc_cref;
            std::vector<double> Csc_cref;
            std::vector<double> Cxc_cref;
            std::vector<double> Cyc_cref;
            std::vector<double> Czc_cref;
            std::vector<double> Cmxc_cref;
            std::vector<double> Cmyc_cref;
            std::vector<double> Cmzc_cref;

            double chordRatio;

            // read the data rows
            while ( data_string_array.size() == nSectionalDataTableCols )
            {
                // Store the raw data
                WingId.push_back( std::stoi( data_string_array[0] ) );
                Yavg.push_back(   std::stod( data_string_array[1] ) );
                Chord.push_back(  std::stod( data_string_array[2] ) );
                VoVinf.push_back( std::stod( data_string_array[3] ) );
                Cl.push_back(     std::stod( data_string_array[4] ) );
                Cd.push_back(     std::stod( data_string_array[5] ) );
                Cs.push_back(     std::stod( data_string_array[6] ) );
                Cx.push_back(     std::stod( data_string_array[7] ) );
                Cy.push_back(     std::stod( data_string_array[8] ) );
                Cz.push_back(     std::stod( data_string_array[9] ) );
                Cmx.push_back(    std::stod( data_string_array[10] ) );
                Cmy.push_back(    std::stod( data_string_array[11] ) );
                Cmz.push_back(    std::stod( data_string_array[12] ) );

                chordRatio = Chord.back() / cref;

                // Normalized by local chord
                Clc_cref.push_back( Cl.back() * chordRatio );
                Cdc_cref.push_back( Cd.back() * chordRatio );
                Csc_cref.push_back( Cs.back() * chordRatio );
                Cxc_cref.push_back( Cx.back() * chordRatio );
                Cyc_cref.push_back( Cy.back() * chordRatio );
                Czc_cref.push_back( Cz.back() * chordRatio );
                Cmxc_cref.push_back( Cmx.back() * chordRatio );
                Cmyc_cref.push_back( Cmy.back() * chordRatio );
                Cmzc_cref.push_back( Cmz.back() * chordRatio );

                // Read the next line and loop
                data_string_array = ReadDelimLine( fp, seps );
            }

            // Finish up by adding the data to the result res
            res->Add( NameValData( "WingId", WingId ) );
            res->Add( NameValData( "Yavg", Yavg ) );
            res->Add( NameValData( "Chord", Chord ) );
            res->Add( NameValData( "V/Vinf", Chord ) );
            res->Add( NameValData( "cl", Cl ) );
            res->Add( NameValData( "cd", Cd ) );
            res->Add( NameValData( "cs", Cs ) );
            res->Add( NameValData( "cx", Cx ) );
            res->Add( NameValData( "cy", Cy ) );
            res->Add( NameValData( "cz", Cz ) );
            res->Add( NameValData( "cmx", Cmx ) );
            res->Add( NameValData( "cmy", Cmy ) );
            res->Add( NameValData( "cmz", Cmz ) );

            res->Add( NameValData( "cl*c/cref", Clc_cref ) );
            res->Add( NameValData( "cd*c/cref", Cdc_cref ) );
            res->Add( NameValData( "cs*c/cref", Csc_cref ) );
            res->Add( NameValData( "cx*c/cref", Cxc_cref ) );
            res->Add( NameValData( "cy*c/cref", Cyc_cref ) );
            res->Add( NameValData( "cz*c/cref", Czc_cref ) );
            res->Add( NameValData( "cmx*c/cref", Cmxc_cref ) );
            res->Add( NameValData( "cmy*c/cref", Cmyc_cref ) );
            res->Add( NameValData( "cmz*c/cref", Cmzc_cref ) );

        } // end sectional table read

    } // end file loop

    std::fclose ( fp );

    return;
}

/*******************************************************
Read .STAB file output from VSPAERO
See: VSP_Solver.C in vspaero project
*******************************************************/
void VSPAEROMgrSingleton::ReadStabFile( string filename, vector <string> &res_id_vector, vsp::VSPAERO_ANALYSIS_METHOD analysisMethod )
{
    FILE *fp = NULL;
    bool read_success = false;
    WaitForFile( filename );
    fp = fopen( filename.c_str() , "r" );
    if ( fp == NULL )
    {
        fprintf( stderr, "ERROR %d: Could not open Stab file: %s\n\tFile: %s \tLine:%d\n", vsp::VSP_FILE_DOES_NOT_EXIST, m_StabFile.c_str(), __FILE__, __LINE__ );
        return;
    }

    Results* res = NULL;

    std::vector<string> table_column_names;
    std::vector<string> data_string_array;

    // Read in all of the data into the results manager
    char seps[]   = " :,\t\n";
    while ( !feof( fp ) )
    {
        data_string_array = ReadDelimLine( fp, seps ); //this is also done in some of the embedded loops below

        if ( CheckForCaseHeader( data_string_array ) )
        {
            res = ResultsMgr.CreateResults( "VSPAERO_Stab" );
            res_id_vector.push_back( res->GetID() );

            if ( ReadVSPAEROCaseHeader( res, fp, analysisMethod ) != 0 )
            {
                // Failed to read the case header
                fprintf( stderr, "ERROR %d: Could not read case header in VSPAERO file: %s\n\tFile: %s \tLine:%d\n", vsp::VSP_FILE_READ_FAILURE, m_StabFile.c_str(), __FILE__, __LINE__ );
                return;
            }
        }
        else if ( data_string_array.size() > 0 )
        {
            // Parse if this is not a comment line
            if ( res && strncmp( data_string_array[0].c_str(), "#", 1 ) != 0 )
            {

                //================ Table Data ================//
                // Checks for table header format
                if ( ( data_string_array.size() != table_column_names.size() ) || ( table_column_names.size() == 0 ) )
                {
                    //Indicator that the data table has changed or has not been initialized.
                    table_column_names.clear();
                    table_column_names = data_string_array;
                }
                else
                {
                    //This is a continuation of the current table and add this row to the results manager
                    for ( unsigned int i_field = 1; i_field < data_string_array.size(); i_field++ )
                    {
                        //attempt to read a double if that fails then treat it as a string result
                        double temp_val = 0;
                        int result = 0;
                        result = sscanf( data_string_array[i_field].c_str(), "%lf", &temp_val );
                        if ( result == 1 )
                        {
                            res->Add( NameValData( data_string_array[0] + "_" + table_column_names[i_field], temp_val ) );
                        }
                        else
                        {
                            res->Add( NameValData( data_string_array[0] + "_" + table_column_names[i_field], data_string_array[i_field] ) );
                        }
                    }
                } //end new table check

            } // end comment line check

        } // end data_string_array.size()>0 check

    } //end for while !feof(fp)

    std::fclose ( fp );

    return;
}

vector <string> VSPAEROMgrSingleton::ReadDelimLine( FILE * fp, char * delimeters )
{

    vector <string> dataStringVector;
    dataStringVector.clear();

    char strbuff[1024];                // buffer for entire line in file
    if ( fgets( strbuff, 1024, fp ) != NULL )
    {
        char * pch = strtok ( strbuff, delimeters );
        while ( pch != NULL )
        {
            dataStringVector.push_back( pch );
            pch = strtok ( NULL, delimeters );
        }
    }

    return dataStringVector;
}

bool VSPAEROMgrSingleton::CheckForCaseHeader( std::vector<string> headerStr )
{
    if ( headerStr.size() == 1 )
    {
        if ( strcmp( headerStr[0].c_str(), "*****************************************************************************************************************************************************************************************" ) == 0 )
        {
            return true;
        }
    }

    return false;
}

int VSPAEROMgrSingleton::ReadVSPAEROCaseHeader( Results * res, FILE * fp, vsp::VSPAERO_ANALYSIS_METHOD analysisMethod )
{
    // check input arguments
    if ( res == NULL )
    {
        // Bad pointer
        fprintf( stderr, "ERROR %d: Invalid results pointer\n\tFile: %s \tLine:%d\n", vsp::VSP_INVALID_PTR, __FILE__, __LINE__ );
        return -1;
    }
    if ( fp == NULL )
    {
        // Bad pointer
        fprintf( stderr, "ERROR %d: Invalid file pointer\n\tFile: %s \tLine:%d\n", vsp::VSP_INVALID_PTR, __FILE__, __LINE__ );
        return -2;
    }

    char seps[]   = " :,\t\n";
    std::vector<string> data_string_array;

    //skip any blank lines before the header
    while ( !feof( fp ) && data_string_array.size() == 0 )
    {
        data_string_array = ReadDelimLine( fp, seps ); //this is also done in some of the embedded loops below
    }

    // Read header table
    bool needs_header = true;
    bool mach_found = false;
    bool alpha_found = false;
    bool beta_found = false;
    double current_mach = -FLT_MAX;
    double current_alpha = -FLT_MAX;
    double current_beta = -FLT_MAX;
    double value;
    while ( !feof( fp ) && data_string_array.size() > 0 )
    {
        // Parse if this is not a comment line
        if ( ( strncmp( data_string_array[0].c_str(), "#", 1 ) != 0 ) && ( data_string_array.size() == 3 ) )
        {
            // assumption that the 2nd entry is a number
            if ( sscanf( data_string_array[1].c_str(), "%lf", &value ) == 1 )
            {
                res->Add( NameValData( "FC_" + data_string_array[0], value ) );

                // save flow condition information to be added to the header later
                if ( strcmp( data_string_array[0].c_str(), "Mach_" ) == 0 )
                {
                    current_mach = value;
                    mach_found = true;
                }
                if ( strcmp( data_string_array[0].c_str(), "AoA_" ) == 0 )
                {
                    current_alpha = value;
                    alpha_found = true;
                }
                if ( strcmp( data_string_array[0].c_str(), "Beta_" ) == 0 )
                {
                    current_beta = value;
                    beta_found = true;
                }

                // check if the information needed for the result header has been read in
                if ( mach_found && alpha_found && beta_found && needs_header )
                {
                    AddResultHeader( res->GetID(), current_mach, current_alpha, current_beta, analysisMethod );
                    needs_header = false;
                }
            }
        }

        // read the next line
        data_string_array = ReadDelimLine( fp, seps );

    } // end while

    if ( needs_header )
    {
        fprintf( stderr, "WARNING: Case header incomplete \n\tFile: %s \tLine:%d\n", __FILE__, __LINE__ );
        return -3;
    }

    return 0; // no errors
}

//Export Results to CSV
//  Return Values:
//  -1 = INVALID Result ID
//  0 = Success
// TODO make return values into enum
int VSPAEROMgrSingleton::ExportResultsToCSV( string fileName )
{
    int retVal;

    // Get the results
    string resId = ResultsMgr.FindLatestResultsID( "VSPAERO_Wrapper" );
    if ( resId != string() )
    {
        // Get all the child results
        vector <string> resIdVector = ResultsMgr.GetStringResults( resId, "ResultsVec" );
        retVal = ResultsMgr.WriteCSVFile( fileName, resIdVector );
    }
    else
    {
        fprintf( stderr, "ERROR %d: Unable to get VSPAERO_Wrapper result \n\tFile: %s \tLine:%d\n", vsp::VSP_CANT_FIND_NAME, __FILE__, __LINE__ );
        retVal = vsp::VSP_CANT_FIND_NAME; //failure
    }

    return retVal;
}

