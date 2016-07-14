//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//

// VSPAEROMgr.cpp: VSPAERO Mgr Singleton.
//
//////////////////////////////////////////////////////////////////////

#include "VSPAEROMgr.h"
#include "ParmMgr.h"
#include "Vehicle.h"
#include "VehicleMgr.h"
#include "StlHelper.h"
#include "APIDefines.h"
#include "WingGeom.h"

#include "StringUtil.h"
#include "FileUtil.h"

#include <regex>

//==== Constructor ====//
VSPAEROMgrSingleton::VSPAEROMgrSingleton() : ParmContainer()
{
    m_Name = "VSPAEROSettings";

    m_DegenGeomSet.Init( "GeomSet", "VSPAERO", this, 0, 0, 12 );
    m_DegenGeomSet.SetDescript( "Geometry set" );

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


    m_DegenFile     = string();
    m_DegenFileFull = string();
    m_SetupFile     = string();
    m_AdbFile       = string();
    m_HistoryFile   = string();
    m_LoadFile      = string();
    m_StabFile      = string();

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
    Vehicle *veh = VehicleMgr.GetVehicle();

    if( veh )
    {

        m_DegenFile = veh->getExportFileName( vsp::DEGEN_GEOM_CSV_TYPE );

        // test if we can open the file
        FILE *fp = NULL;
        fp = fopen( m_DegenFile.c_str(), "r" );
        if( fp )
        {
            fclose( fp );
        }
        else
        {
            m_DegenFile = string();
        }

        // remove the .csv file extension so it can be used as the base name for VSPAERO files
        if( !m_DegenFile.empty() )
        {
            int pos = m_DegenFile.find( ".csv" );
            if ( pos >= 0 )
            {
                m_DegenFile.erase( pos, m_DegenFile.length() - 1 );
            }
        }
    }
    else
    {
        m_DegenFile = string();
    }

    // Setup the remaining file names
    if( !m_DegenFile.empty() )
    {
        m_DegenFileFull = m_DegenFile + string( ".csv" );
        m_SetupFile     = m_DegenFile + string( ".vspaero" );
        m_AdbFile       = m_DegenFile + string( ".adb" );
        m_HistoryFile   = m_DegenFile + string( ".history" );
        m_LoadFile      = m_DegenFile + string( ".lod" );
        m_StabFile      = m_DegenFile + string( ".stab" );
    }
    else
    {
        m_DegenFileFull = string();
        m_SetupFile     = string();
        m_AdbFile       = string();
        m_HistoryFile   = string();
        m_LoadFile      = string();
        m_StabFile      = string();
    }
}

string VSPAEROMgrSingleton::ComputeGeometry()
{
    Vehicle *veh = VehicleMgr.GetVehicle();

    veh->CreateDegenGeom( VSPAEROMgr.m_DegenGeomSet() );

    // record original values
    bool exptMfile_orig = veh->getExportDegenGeomMFile();
    bool exptCSVfile_orig = veh->getExportDegenGeomCsvFile();
    veh->setExportDegenGeomMFile( false );
    veh->setExportDegenGeomCsvFile( true );

    UpdateFilenames();

    veh->WriteDegenGeomFile();

    // restore original values
    veh->setExportDegenGeomMFile( exptMfile_orig );
    veh->setExportDegenGeomCsvFile( exptCSVfile_orig );

    // add output filenames to results manager (this is important for the Analysis Manager)
    Results* res = ResultsMgr.CreateResults( "VSPAERO_DegenGeom" );
    // add to results manager
    res->Add( NameValData( "OutputFileName", m_DegenFile + string( ".csv" ) ) );
    res->Add( NameValData( "GeometrySet", VSPAEROMgr.m_DegenGeomSet() ) );

    return res->GetID();

}

/* TODO - finish implementation of generating the setup file from the VSPAEROMgr*/
void VSPAEROMgrSingleton::CreateSetupFile()
{
    Vehicle *veh = VehicleMgr.GetVehicle();

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

    args.push_back( "-aoa" );
    args.push_back( StringUtil::double_to_string( m_AlphaStart(), "%f" ) );

    args.push_back( "-beta" );
    args.push_back( StringUtil::double_to_string( m_BetaStart(), "%f" ) );

    args.push_back( "-mach" );
    args.push_back( StringUtil::double_to_string( m_MachStart(), "%f" ) );

    args.push_back( "-wakeiters" );
    args.push_back( StringUtil::int_to_string( m_WakeNumIter(), "%d" ) );

    args.push_back( m_DegenFile );

    // Execute VSPAero
    m_SolverProcess.ForkCmd( veh->GetExePath(), veh->GetVSPAEROCmd(), args );

    // wait for the solver to finish
    bool runflag = m_SolverProcess.IsRunning();
    while ( runflag )
    {
        SleepForMilliseconds( 100 );
        runflag = m_SolverProcess.IsRunning();
    }

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
        printf( "ERROR - setup file not found\n" );
    }
}

void VSPAEROMgrSingleton::ClearAllPreviousResults()
{
    while ( ResultsMgr.GetNumResults( "VSPAERO_DegenGeom" ) > 0 )
    {
        ResultsMgr.DeleteResult( ResultsMgr.FindResultsID( "VSPAERO_DegenGeom",  0 ) );
    }
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

string VSPAEROMgrSingleton::ComputeSolver()
{
    std::vector <string> res_id_vector;

    Vehicle *veh = VehicleMgr.GetVehicle();

    if ( veh )
    {
        // Calculate spacing
        double alphaDelta = 0.0;
        if ( m_AlphaNpts.Get() > 1 )
        {
            alphaDelta = ( m_AlphaEnd.Get() - m_AlphaStart.Get() ) / ( m_AlphaNpts.Get() - 1.0 );
        }
        double betaDelta = 0.0;
        if ( m_BetaNpts.Get() > 1 )
        {
            betaDelta = ( m_BetaEnd.Get() - m_BetaStart.Get() ) / ( m_BetaNpts.Get() - 1.0 );
        }
        double machDelta = 0.0;
        if ( m_MachNpts.Get() > 1 )
        {
            machDelta = ( m_MachEnd.Get() - m_MachStart.Get() ) / ( m_MachNpts.Get() - 1.0 );
        }

        //====== Modify/Update the setup file ======//
        if ( !FileExist( m_SetupFile ) )
        {
            // if the setup file doesn't exist, create one with the current settings
            // TODO output a warning to the user that we are creating a default file
            CreateSetupFile();
        }

        //====== Loop over flight conditions and solve ======//
        // TODO make this into a case list with a single loop
        for ( int iAlpha = 0; iAlpha < m_AlphaNpts.Get(); iAlpha++ )
        {
            //Set current alpha value
            double current_alpha = m_AlphaStart.Get() + double( iAlpha ) * alphaDelta;

            for ( int iBeta = 0; iBeta < m_BetaNpts.Get(); iBeta++ )
            {
                //Set current beta value
                double current_beta = m_BetaStart.Get() + double( iBeta ) * betaDelta;

                for ( int iMach = 0; iMach < m_MachNpts.Get(); iMach++ )
                {
                    //Set current mach value
                    double current_mach = m_MachStart.Get() + double( iMach ) * machDelta;

                    //====== Clear VSPAERO output files ======//
                    if ( FileExist( m_AdbFile ) )
                    {
                        remove( m_AdbFile.c_str() );
                    }
                    if ( FileExist( m_HistoryFile ) )
                    {
                        remove( m_HistoryFile.c_str() );
                    }
                    if ( FileExist( m_LoadFile ) )
                    {
                        remove( m_LoadFile.c_str() );
                    }
                    if ( FileExist( m_StabFile ) )
                    {
                        remove( m_StabFile.c_str() );
                    }



                    //====== Send command to be executed by the system at the command prompt ======//
                    vector<string> args;
                    // Set mach, alpha, beta (save to local "current*" variables to use as header information in the results manager)
                    args.push_back( "-fs" );       // "freestream" override flag
                    args.push_back( StringUtil::double_to_string( current_mach, "%f" ) );
                    args.push_back( StringUtil::double_to_string( current_alpha, "%f" ) );
                    args.push_back( StringUtil::double_to_string( current_beta, "%f" ) );
                    // Set number of openmp threads
                    args.push_back( "-omp" );
                    args.push_back( StringUtil::int_to_string( m_NCPU.Get(), "%d" ) );
                    // Set stability run arguments
                    if ( m_StabilityCalcFlag.Get() )
                    {
                        args.push_back( "-stab" );
                    }
                    // Force averaging startign at wake iteration N
                    args.push_back( "-avg" );
                    args.push_back( StringUtil::int_to_string( m_WakeAvgStartIter.Get(), "%d" ) );
                    // No wake for first N iterations
                    args.push_back( "-nowake" );
                    args.push_back( StringUtil::int_to_string( m_WakeSkipUntilIter.Get(), "%d" ) );

                    // Add model file name
                    args.push_back( m_DegenFile );

                    // Execute VSPAero
                    m_SolverProcess.ForkCmd( veh->GetExePath(), veh->GetVSPAEROCmd(), args );

                    // wait for the solver to finish
                    bool runflag = m_SolverProcess.IsRunning();
                    while ( runflag )
                    {
                        SleepForMilliseconds( 100 );
                        runflag = m_SolverProcess.IsRunning();
                    }

                    // Check if the kill solver flag has been raised, if so clean up and return
                    //  note: we could have exited the IsRunning loop if the process was killed
                    if( m_SolverProcessKill )
                    {
                        m_SolverProcessKill = false;    //reset kill flag

                        return string();    //return empty result ID vector
                    }

                    //====== Read in all of the results ======//
                    // read the files if there is new data that has not successfully been read in yet
                    res_id_vector.push_back( ReadHistoryFile() );
                    AddResultHeader( res_id_vector[res_id_vector.size() - 1], current_mach, current_alpha, current_beta );
                    res_id_vector.push_back( ReadLoadFile() );
                    AddResultHeader( res_id_vector[res_id_vector.size() - 1], current_mach, current_alpha, current_beta );
                    if ( m_StabilityCalcFlag.Get() )
                    {
                        res_id_vector.push_back( ReadStabFile() );        //*.STAB stability coeff file
                        AddResultHeader( res_id_vector[res_id_vector.size() - 1], current_mach, current_alpha, current_beta );
                    }

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

void VSPAEROMgrSingleton::AddResultHeader( string res_id, double mach, double alpha, double beta )
{
    // Add Flow Condition header to each result
    Results * res = ResultsMgr.FindResultsPtr( res_id );
    res->Add( NameValData( "FC_Mach", mach ) );
    res->Add( NameValData( "FC_Alpha", alpha ) );
    res->Add( NameValData( "FC_Beta", beta ) );
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
See: VSP_Solver.C in vspaero project
line 4351 - void VSP_SOLVER::OutputStatusFile(int Type)
line 4407 - void VSP_SOLVER::OutputZeroLiftDragToStatusFile(void)
TODO:
- Update this function to use the generic table read as used in: string VSPAEROMgrSingleton::ReadStabFile()
*******************************************************/
string VSPAEROMgrSingleton::ReadHistoryFile()
{
    string res_id;
    //TODO return success or failure
    FILE *fp = NULL;
    size_t result;
    bool read_success = false;

    //Read setup file to get number of wakeiterations
    fp = fopen( m_SetupFile.c_str(), "r" );
    if ( fp == NULL )
    {
        fputs ( "VSPAEROMgrSingleton::ReadHistoryFile() - File open error\n", stderr );
    }
    else
    {
        // find the 'WakeIters' token
        char key[] = "WakeIters";
        char param[30];
        float param_val = 0;
        int n_wakeiters = -1;
        bool found = false;
        while ( !feof( fp ) & !found )
        {
            if( fscanf( fp, "%s = %f\n", param, &param_val ) == 2 )
            {
                if( strcmp( key, param ) == 0 )
                {
                    n_wakeiters = ( int )param_val;
                    found = true;
                }
            }
        }
        fclose( fp );

        //HISTORY file
        WaitForFile( m_HistoryFile );
        fp = fopen( m_HistoryFile.c_str(), "r" );
        if ( fp == NULL )
        {
            fputs ( "VSPAEROMgrSingleton::ReadHistoryFile() - File open error\n", stderr );
        }
        else
        {
            // Read header line - we don't ever use this it's just a way to move the file pointer
            // TODO - use the fields in the header string as the parameter names in the results manager
            char headerstr [256];
            fgets( headerstr, 255, fp );
            // split headerstr into fieldnames
            std::vector<string>fieldnames;
            int n_fields = 0;
            char * pch;
            pch = strtok ( headerstr, " " );
            while ( pch != NULL )
            {
                n_fields++;
                fieldnames.push_back( pch );
                pch = strtok ( NULL, " " );
            }


            // Read wake iter data
            std::vector<int> i;                 i.assign( n_wakeiters, 0 );
            std::vector<double> Mach;           Mach.assign( n_wakeiters, 0 );
            std::vector<double> Alpha;          Alpha.assign( n_wakeiters, 0 );
            std::vector<double> Beta;           Beta.assign( n_wakeiters, 0 );
            std::vector<double> CL;             CL.assign( n_wakeiters, 0 );
            std::vector<double> CDo;            CDo.assign( n_wakeiters, 0 );
            std::vector<double> CDi;            CDi.assign( n_wakeiters, 0 );
            std::vector<double> CDtot;          CDtot.assign( n_wakeiters, 0 );
            std::vector<double> CS;             CS.assign( n_wakeiters, 0 );
            std::vector<double> CFx;            CFx.assign( n_wakeiters, 0 );
            std::vector<double> CFy;            CFy.assign( n_wakeiters, 0 );
            std::vector<double> CFz;            CFz.assign( n_wakeiters, 0 );
            std::vector<double> CMx;            CMx.assign( n_wakeiters, 0 );
            std::vector<double> CMy;            CMy.assign( n_wakeiters, 0 );
            std::vector<double> CMz;            CMz.assign( n_wakeiters, 0 );
            std::vector<double> LoD;            LoD.assign( n_wakeiters, 0 );
            std::vector<double> E;              E.assign( n_wakeiters, 0 );
            std::vector<double> ToQS;           ToQS.assign( n_wakeiters, 0 );

            // Read in all of the wake data first before adding to the results manager
            for( int i_wake = 0; i_wake < n_wakeiters; i_wake++ )
            {
                //TODO - add results to results manager based on header string
                result = fscanf( fp, "%d", &i[i_wake] );
                result = fscanf( fp, "%lf", &Mach[i_wake] );
                result = fscanf( fp, "%lf", &Alpha[i_wake] );
                result = fscanf( fp, "%lf", &Beta[i_wake] );
                result = fscanf( fp, "%lf", &CL[i_wake] );
                result = fscanf( fp, "%lf", &CDo[i_wake] );
                result = fscanf( fp, "%lf", &CDi[i_wake] );
                result = fscanf( fp, "%lf", &CDtot[i_wake] );
                result = fscanf( fp, "%lf", &CS[i_wake] );
                result = fscanf( fp, "%lf", &LoD[i_wake] );
                result = fscanf( fp, "%lf", &E[i_wake] );
                result = fscanf( fp, "%lf", &CFx[i_wake] );
                result = fscanf( fp, "%lf", &CFy[i_wake] );
                result = fscanf( fp, "%lf", &CFz[i_wake] );
                result = fscanf( fp, "%lf", &CMx[i_wake] );
                result = fscanf( fp, "%lf", &CMy[i_wake] );
                result = fscanf( fp, "%lf", &CMz[i_wake] );
                result = fscanf( fp, "%lf", &ToQS[i_wake] );
            }
            fclose ( fp );

            //add to the results manager
            Results* res = ResultsMgr.CreateResults( "VSPAERO_History" );
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

            res_id = res->GetID();
        }
    }

    return res_id;
}

/*******************************************************
Read .LOD file output from VSPAERO
See: VSP_Solver.C in vspaero project
line 2851 - void VSP_SOLVER::CalculateSpanWiseLoading(void)
TODO:
- Update this function to use the generic table read as used in: string VSPAEROMgrSingleton::ReadStabFile()
- Read in Component table information, this is the 2nd table at the bottom of the .lod file
*******************************************************/
string VSPAEROMgrSingleton::ReadLoadFile()
{
    string res_id;

    FILE *fp = NULL;
    size_t result;
    bool read_success = false;

    //LOAD file
    WaitForFile( m_LoadFile );
    fp = fopen( m_LoadFile.c_str(), "r" );
    if ( fp == NULL )
    {
        fputs ( "VSPAEROMgrSingleton::ReadLoadFile() - File open error\n", stderr );
    }
    else
    {
        // Read header line - we don't ever use this it's just a way to move the file pointer
        // TODO - use the fields in the header string as the parameter names in the results manager
        std::vector<string>fieldnames;
        char strbuff[1024];                // buffer for entire line in file
        char * pch;
        fgets( strbuff, 1024, fp );
        pch = strtok ( strbuff, " " );
        while ( pch != NULL )
        {
            fieldnames.push_back( pch );
            pch = strtok ( NULL, " " );
        }

        std::vector<int> WingId;
        std::vector<double> Yavg;
        std::vector<double> Chord;
        std::vector<double> CL;
        std::vector<double> CD;
        std::vector<double> CS;

        std::vector<double> CLc;
        std::vector<double> CDc;
        std::vector<double> CSc;
        //std::vector<double> CLc_ideal;  // TODO represents elliptical load distribution

        int t_WingId;
        double t_Yavg;
        double t_Chord;
        double t_CL;
        double t_CD;
        double t_CS;

        //READ and ADD to the results manager
        Results* res = ResultsMgr.CreateResults( "VSPAERO_Load" );
        result = fscanf( fp, "%d %lf %lf %lf %lf %lf", &t_WingId, &t_Yavg, &t_Chord, &t_CL, &t_CD, &t_CS );
        while ( result == fieldnames.size() - 1 )
        {

            WingId.push_back( t_WingId );
            Yavg.push_back( t_Yavg );
            Chord.push_back( t_Chord );
            CL.push_back( t_CL );
            CD.push_back( t_CD );
            CS.push_back( t_CS );

            CLc.push_back( t_CL * t_Chord );
            CDc.push_back( t_CD * t_Chord );
            CSc.push_back( t_CS * t_Chord );

            // read the next line
            result = fscanf( fp, "%d %lf %lf %lf %lf %lf", &t_WingId, &t_Yavg, &t_Chord, &t_CL, &t_CD, &t_CS );
        }
        fclose ( fp );

        // add to results manager
        res->Add( NameValData( "WingId", WingId ) );
        res->Add( NameValData( "Yavg", Yavg ) );
        res->Add( NameValData( "Chord", Chord ) );
        res->Add( NameValData( "cl", CL ) );
        res->Add( NameValData( "cd", CD ) );
        res->Add( NameValData( "cs", CS ) );

        res->Add( NameValData( "cl*c", CLc ) );
        res->Add( NameValData( "cd*c", CDc ) );
        res->Add( NameValData( "cs*c", CSc ) );

        res_id = res->GetID();
    }

    return res_id;
}

/*******************************************************
Read .STAB file output from VSPAERO
See: VSP_Solver.C in vspaero project
*******************************************************/
string VSPAEROMgrSingleton::ReadStabFile()
{
    string res_id;

    FILE *fp = NULL;
    size_t result;
    bool read_success = false;
    fp = fopen( m_StabFile.c_str() , "r" );
    if ( fp == NULL )
    {
        fputs ( "VSPAEROMgrSingleton::ReadStabFile() - File open error\n", stderr );
    }
    else
    {
        Results* res = ResultsMgr.CreateResults( "VSPAERO_Stab" );

        /* Read top header sectio.  Example:
            Sref_:      45.0000
            Cref_:       2.5000
            Bref_:      18.0000
            Xcg_:        0.0000
            Ycg_:        0.0000
            Zcg_:        0.0000
            AoA:         4.0000
            Beta_:      -3.0000
            Mach_:       0.4000
            Rho_:        0.0024
            Vinf_:     100.0000
        #
        */
        float value;
        char strbuff[1024];
        result = fscanf( fp, "%s%f\n", strbuff, &value );
        while ( result == 2 )
        {
            // format and add name/value pair to the results manager
            string name = strbuff;
            size_t pos = name.find( "_" ); // find and erase underscore
            if ( pos != std::string::npos )
            {
                name.erase( pos );
            }
            pos = name.find( ":" ); //find and erase :
            if ( pos != std::string::npos )
            {
                name.erase( pos );
            }
            res->Add( NameValData( name, value ) );

            // read the next value
            result = fscanf( fp, "%s%f\n", strbuff, &value );
        }

        /* Read the available data tables
        Example:

        */
        std::vector<string> table_column_names;
        std::vector<string> data_string_array;
        char * pch;
        // Read in all of the data into the results manager
        while ( !feof( fp ) )
        {
            // Read entire line
            char strbuff2[1024];
            fgets( strbuff2, 1023, fp );
            strcpy( strbuff, " " );
            strcat( strbuff, strbuff2 );

            // Parse if this is not a comment line
            if ( strncmp( strbuff2, "#", 1 ) != 0 )
            {
                // Split space delimited string
                data_string_array.clear();
                pch = strtok ( strbuff, " " );
                while ( pch != NULL )
                {
                    data_string_array.push_back( pch );
                    pch = strtok ( NULL, " " );
                }

                // Checks for header format
                if ( ( data_string_array.size() != table_column_names.size() ) | ( table_column_names.size() == 0 ) )
                {
                    //Indicator that the data table has changed or has not been initialized.
                    table_column_names.clear();
                    table_column_names = data_string_array;
                }
                else
                {
                    //This is a continuation of the current table and add this row to the results manager
                    for ( unsigned int i_field = 1; i_field < data_string_array.size() - 1; i_field++ )
                    {
                        //convert to double
                        double temp_val = 0;
                        int result = 0;
                        result = sscanf( data_string_array[i_field].c_str(), "%lf", &temp_val );
                        if ( result == 1 )
                        {
                            res->Add( NameValData( data_string_array[0] + "_" + table_column_names[i_field], temp_val ) );
                        }
                    }
                } //end new table check
            } // end comment line check
        } //end for while !feof(fp)


        fclose ( fp );
        res_id = res->GetID();
    }

    return res_id;
}

//Export Results to CSV
//  Return Values:
//  -1 = INVALID Result ID
//  0 = Success
// TODO make return values into enum
int VSPAEROMgrSingleton::ExportResultsToCSV( string fileName )
{
    // Get the results
    string resId = ResultsMgr.FindResultsID( "VSPAERO_Wrapper",  0 );
    Results* resptr = ResultsMgr.FindResultsPtr( resId );
    if ( !resptr )
    {
        printf( "WriteResultsCSVFile::Invalid ID %s", resId.c_str() );
        return -1;
    }

    //Write the output file
    vector <string> resIdVector = ResultsMgr.GetStringResults( resId, "ResultsVec" );
    ResultsMgr.WriteCSVFile( fileName, resIdVector );

    return 0; //success
}
