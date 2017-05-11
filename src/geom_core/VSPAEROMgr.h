//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//

// VSPAEROMgr.h: interface for the VSPAERO Singleton.
//
//////////////////////////////////////////////////////////////////////

#if !defined(VSPAERO__INCLUDED_)
#define VSPAERO__INCLUDED_

#include "VehicleMgr.h"
#include "Vehicle.h"
#include "Geom.h"
#include "Parm.h"
#include "ProcessUtil.h"

#include <vector>
#include <string>
using std::string;
using std::vector;

class VspAeroControlSurf
{
public:
    VspAeroControlSurf();

    string fullName;
    string parentGeomId;
    string SSID;
    bool isGrouped;
    unsigned int iReflect;      // mapping index to which reflected sub surface
};

class RotorDisk : public ParmContainer
{
public:
    // Constructor, Destructor, Copy
    RotorDisk( void );
    ~RotorDisk( void );
    RotorDisk( const RotorDisk &RotorDisk );
    RotorDisk& operator=( const RotorDisk &RotorDisk );

    RotorDisk( DegenGeom &degenGeom );  //TODO make this argument a const
    RotorDisk( const DegenDisk degenDisk, string parentGeomId, unsigned int parentGeomSurfNdx );
    void InitDisk();
    void ParmChanged( Parm* parm_ptr, int type );

    // Setup File I/O
    void Write_STP_Data( FILE * InputFile );
    void Load_STP_Data( FILE * InputFile );

    string GetParentID()                  { return m_ParentGeomId; }
    unsigned int GetSurfNum()             { return m_ParentGeomSurfNdx; }

    // vsp3 file xml I/O
    virtual xmlNodePtr EncodeXml( xmlNodePtr & node );
    virtual xmlNodePtr DecodeXml( xmlNodePtr & node );

    bool m_IsUsed;

    vec3d m_XYZ;           // RotorXYZ_
    vec3d m_Normal;        // RotorNormal_

    Parm m_Diameter;       // RotorDiameter_
    Parm m_HubDiameter;    // RotorHubDiameter_
    Parm m_RPM;          // RotorRPM_

    Parm m_CT;          // Rotor_CT_
    Parm m_CP;          // Rotor_CP_

    //identifing information for vsp model
    string m_ParentGeomId;
    unsigned int m_ParentGeomSurfNdx;

    void UpdateParmGroupName();
};

//==== VSPAERO Manager ====//
class VSPAEROMgrSingleton : public ParmContainer
{
public:
    static VSPAEROMgrSingleton& getInstance()
    {
        static VSPAEROMgrSingleton instance;
        return instance;
    }
    virtual void ParmChanged( Parm* parm_ptr, int type );

    void ClearAllPreviousResults();

    virtual xmlNodePtr EncodeXml( xmlNodePtr & node );
    virtual xmlNodePtr DecodeXml( xmlNodePtr & node );

    void Update();

    void UpdateFilenames();
    void UpdateRotorDisks();
    // Setter Methods
    void SetCurrentRotorDiskIndex( int index )              { m_CurrentRotorDiskIndex = index; }

    // Getter Methods
    int GetCurrentRotorDiskIndex()                          { return m_CurrentRotorDiskIndex; }
    vector <RotorDisk*> GetRotorDiskVec()                      { return m_RotorDiskVec; };
    string ComputeGeometry();
    void CreateSetupFile( FILE * logFile = NULL );
    string ComputeSolver( FILE * logFile = NULL ); // returns a result with a vector of results id's under the name ResultVec
    string ComputeSolverBatch( FILE * logFile = NULL );
    string ComputeSolverSingle( FILE * logFile = NULL );
    ProcessUtil* GetSolverProcess();
    bool IsSolverRunning();
    void KillSolver();

    int ExportResultsToCSV( string fileName );

    IntParm m_AnalysisMethod;

    BoolParm m_ForceNewSetupfile;
    // Rotor Disk Functionality
    void SetCurrentRotorDiskFromParms();
    void SetParmsFromCurrentRotorDisk();
    bool ValidRotorDiskIndex( int index );


    // file names
    string m_ModelNameBase; // this is the name used in the execution string
    string m_DegenFileFull; //degengeom file name WITH .csv file extension
    string m_CompGeomFileFull; //geometry file used for panel method
    string m_SetupFile;
    string m_AdbFile;
    string m_HistoryFile;
    string m_LoadFile;
    string m_StabFile;

    IntParm m_GeomSet;

    Parm m_Sref;
    Parm m_bref;
    Parm m_cref;
    string m_RefGeomID;
    IntParm m_RefFlag;
    BoolParm m_StabilityCalcFlag;
    BoolParm m_BatchModeFlag;

    IntParm m_CGGeomSet;
    IntParm m_NumMassSlice;
    Parm m_Xcg;
    Parm m_Ycg;
    Parm m_Zcg;

    // Flow Condition Settings
    Parm m_AlphaStart, m_AlphaEnd; IntParm m_AlphaNpts;
    Parm m_BetaStart, m_BetaEnd; IntParm m_BetaNpts;
    Parm m_MachStart, m_MachEnd; IntParm m_MachNpts;

    // Solver settings
    IntParm m_NCPU;
    IntParm m_WakeNumIter;
    IntParm m_WakeAvgStartIter;
    IntParm m_WakeSkipUntilIter;

    // Disk Element Parms
    Parm m_Diameter;
    Parm m_HubDiameter;
    Parm m_RPM;
    Parm m_CT;
    Parm m_CP;


    // Plotwindow settings
    BoolParm m_ConvergenceXMinIsManual;
    BoolParm m_ConvergenceXMaxIsManual;
    BoolParm m_ConvergenceYMinIsManual;
    BoolParm m_ConvergenceYMaxIsManual;
    Parm m_ConvergenceXMin;
    Parm m_ConvergenceXMax;
    Parm m_ConvergenceYMin;
    Parm m_ConvergenceYMax;

    BoolParm m_LoadDistXMinIsManual;
    BoolParm m_LoadDistXMaxIsManual;
    BoolParm m_LoadDistYMinIsManual;
    BoolParm m_LoadDistYMaxIsManual;
    Parm m_LoadDistXMin;
    Parm m_LoadDistXMax;
    Parm m_LoadDistYMin;
    Parm m_LoadDistYMax;

    BoolParm m_SweepXMinIsManual;
    BoolParm m_SweepXMaxIsManual;
    BoolParm m_SweepYMinIsManual;
    BoolParm m_SweepYMaxIsManual;
    Parm m_SweepXMin;
    Parm m_SweepXMax;
    Parm m_SweepYMin;
    Parm m_SweepYMax;

    ProcessUtil m_SolverProcess;

protected:
    string m_LastPanelMeshGeomId;

    int WaitForFile( string filename );  // function is used to wait for the result to show up on the file system
    void GetSweepVectors( vector<double> &alphaVec, vector<double> &betaVec, vector<double> &machVec );

    void MonitorSolver( FILE * logFile );
    bool m_SolverProcessKill;

    // helper functions for VSPAERO files
    void ReadHistoryFile( string filename, vector <string> &res_id_vector, vsp::VSPAERO_ANALYSIS_METHOD analysisMethod );
    void ReadLoadFile( string filename, vector <string> &res_id_vector, vsp::VSPAERO_ANALYSIS_METHOD analysisMethod );
    void ReadStabFile( string filename, vector <string> &res_id_vector, vsp::VSPAERO_ANALYSIS_METHOD analysisMethod );
    vector <string> ReadDelimLine( FILE * fp, char * delimeters );
    bool CheckForCaseHeader( std::vector<string> headerStr );
    int ReadVSPAEROCaseHeader( Results * res, FILE * fp, vsp::VSPAERO_ANALYSIS_METHOD analysisMethod );

    void AddResultHeader( string res_id, double mach, double alpha, double beta, vsp::VSPAERO_ANALYSIS_METHOD analysisMethod );


private:
    VSPAEROMgrSingleton();
    VSPAEROMgrSingleton( VSPAEROMgrSingleton const& copy );            // Not Implemented
    VSPAEROMgrSingleton& operator=( VSPAEROMgrSingleton const& copy ); // Not Implemented

    vector< RotorDisk* > m_RotorDiskVec;
    vector< VspAeroControlSurf > m_CompleteControlSurfaceVec;   // list of all control and rectangle sub-surfaces in the model selected as control surfaces
    vector < VspAeroControlSurf > m_ActiveControlSurfaceVec;
    vector < VspAeroControlSurf > m_UngroupedCS;
    int m_CurrentRotorDiskIndex;
};

#define VSPAEROMgr VSPAEROMgrSingleton::getInstance()

#endif
