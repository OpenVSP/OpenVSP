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
#include "StringUtil.h"

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
    RotorDisk& operator=( const RotorDisk &RotorDisk );

    void ParmChanged( Parm* parm_ptr, int type );

    // Setup File I/O
    void Write_STP_Data( FILE * InputFile );

    string GetParentID()                  { return m_ParentGeomId; }
    unsigned int GetSurfNum()             { return m_ParentGeomSurfNdx; }

    // vsp3 file xml I/O
    virtual xmlNodePtr EncodeXml( xmlNodePtr & node );
    virtual xmlNodePtr DecodeXml( xmlNodePtr & node );

    string m_GroupName;
    int m_GroupSuffix;

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

    void SetGroupDisplaySuffix( int num );

};

class CpSlice : public ParmContainer
{
public:

    CpSlice();
    ~CpSlice();

    void ParmChanged( Parm* parm_ptr, int type );

    void LoadDrawObj( vector < DrawObj* > &draw_obj_vec, int id, bool highlight );

    IntParm m_CutType;
    Parm m_CutPosition;
    BoolParm m_DrawCutFlag;

private:

    VspSurf CreateSurf();

    vector < DrawObj > m_CpSliceDOVec;

};

class ControlSurfaceGroup : public ParmContainer
{
public:
    // Constructor, Destructor, Copy

    ControlSurfaceGroup( void );
    ~ControlSurfaceGroup( void );

    void ParmChanged( Parm* parm_ptr, int type );

    // Member variables

    string m_GroupName;
    string m_ParentGeomBaseID;

    int m_GroupSuffix;

    BoolParm m_IsUsed;

    vector < Parm* > m_DeflectionGainVec;     // ControlSurface_DeflectionDirection_
    vector < VspAeroControlSurf > m_ControlSurfVec;

    Parm m_DeflectionAngle;                   // ControlSurface_DeflectionAngle_

    // Setup File I/O
    void Write_STP_Data( FILE * InputFile );
    void Load_STP_Data( FILE * InputFile );

    // vsp3 file xml I/O
    virtual xmlNodePtr EncodeXml( xmlNodePtr & node );
    virtual xmlNodePtr DecodeXml( xmlNodePtr & node );

    // Subsurface Manipulation
    void AddSubSurface( VspAeroControlSurf control_surf );
    void RemoveSubSurface( const string & ssid, int reflec_num );

    void SetGroupDisplaySuffix( int num );

};

//==== VSPAERO Manager ====//
class VSPAEROMgrSingleton : public ParmContainer
{
public:
    // Selected type used for GUI visual aid of the selected rotor or control surface
    enum SELECTED_TYPE
    {
        ROTORDISK = 0,      // user has selected a rotor disk
        CONTROL_SURFACE,    // user has selected a control surface
    };
    // Aerodynamic reference area and length
    enum REF_WING_TYPE
    {
        MANUAL_REF = 0,     // manually specify the reference areas and lengths
        COMPONENT_REF,      // use a particular wing to calculate the reference area and lengths
        NUM_REF_TYPES,
    };

    static VSPAEROMgrSingleton& getInstance()
    {
        static VSPAEROMgrSingleton instance;
        return instance;
    }
    virtual void ParmChanged( Parm* parm_ptr, int type );

    void ClearAllPreviousResults();

    virtual xmlNodePtr EncodeXml( xmlNodePtr & node );
    virtual xmlNodePtr DecodeXml( xmlNodePtr & node );

    // Update Methods
    void Renew();
    void Update();
    void UpdateSref();
    void UpdateSetupParmLimits();
    void UpdateFilenames();
    void UpdateRotorDisks();
    void UpdateControlSurfaceGroups();
    void CleanCompleteControlSurfVec();
    void UpdateCompleteControlSurfVec();         // initializes one group per surface
    void UpdateActiveControlSurfVec();
    void UpdateBBox( vector < DrawObj* > & draw_obj_vec );
    void UpdateHighlighted( vector < DrawObj* > & draw_obj_vec );

    // Setter Methods
    void SetCurrentRotorDiskIndex( int index )              { m_CurrentRotorDiskIndex = index; }
    void SetCurrentCSGroupName( const string & name );
    void SetCurrentCSGroupIndex( int index )                { m_CurrentCSGroupIndex = index; }
    void SetCurrentType( int type )                         { m_LastSelectedType = type; }

    // Getter Methods
    int GetCurrentRotorDiskIndex()                          { return m_CurrentRotorDiskIndex; }
    vector <RotorDisk*> GetRotorDiskVec()                      { return m_RotorDiskVec; };
    vector < VspAeroControlSurf > GetActiveCSVec()          { return m_ActiveControlSurfaceVec; }
    vector < VspAeroControlSurf > GetCompleteCSVec()        { return m_CompleteControlSurfaceVec; }
    int GetCurrentCSGroupIndex()                            { return m_CurrentCSGroupIndex; }
    string GetCurrentCSGGroupName();
    vector <ControlSurfaceGroup* > GetControlSurfaceGroupVec()   { return m_ControlSurfaceGroupVec; }
    vector < int > GetSelectedGroupedItems()                { return m_SelectedGroupedCS; }
    vector < int > GetSelectedUngroupedItems()              { return m_SelectedUngroupedCS; }

    // VSP Aero Functionality and Variables
    string ComputeGeometry();
    string CreateSetupFile();                          // natively creates a *.vspaero template setup file
    string ComputeSolver( FILE * logFile = NULL ); // returns a result with a vector of results id's under the name ResultVec
    string ComputeSolverBatch( FILE * logFile = NULL );
    string ComputeSolverSingle( FILE * logFile = NULL );
    ProcessUtil* GetSolverProcess();
    bool IsSolverRunning();
    void KillSolver();

    int ExportResultsToCSV( string fileName );

    IntParm m_AnalysisMethod;

    // CpSlicer Functionality and Variables
    string ComputeCpSlices( FILE * logFile = NULL );
    string ExecuteCpSlicer( FILE * logFile = NULL );
    void ClearCpSliceResults();
    void CreateCutsFile();
    void AddCpSlice( CpSlice* slice )                      { m_CpSliceVec.push_back( slice ); }
    CpSlice* AddCpSlice();
    bool ValidCpSliceInd( int ind );
    void DelCpSlice( int ind );
    CpSlice* GetCpSlice( int ind );
    int GetCpSliceIndex( const string & id );
    void AddCpSliceVec( int cut_type, vector < double > cut_vec );
    vector < double > GetCpSlicePosVec(int type );
    vector < CpSlice* > GetCpSliceVec()                    { return m_CpSliceVec; }
    void ClearCpSliceVec();
    int GetCurrentCpSliceIndex()                           { return m_CurrentCpSliceIndex; }
    void SetCurrentCpSliceIndex( int index )               { m_CurrentCpSliceIndex = index; }
    int GetCpSliceAnalysisType()                           { return m_CpSliceAnalysisType; }

    BoolParm m_CpSliceFlag;

    // Rotor Disk Functionality
    void AddRotorDisk();
    bool ValidRotorDiskIndex( int index );
    void UpdateRotorDiskSuffix();

    // Control Surface Group Functionality
    void InitControlSurfaceGroups();        // default initial grouping of control surfaces for VSPAERO
    void AddControlSurfaceGroup();
    void RemoveControlSurfaceGroup();
    void AddSelectedToCSGroup();
    void AddAllToCSGroup();
    void RemoveSelectedFromCSGroup();
    void RemoveAllFromCSGroup();
    void UpdateControlSurfaceGroupSuffix();

    virtual void AddLinkableParms( vector < string > & linkable_parm_vec, const string & link_container_id );

    // Highlighter Methods and Variables
    void HighlightSelected( int type );
    void LoadDrawObjs( vector < DrawObj* > & draw_obj_vec );

    vector < int > m_SelectedGroupedCS;
    vector < int > m_SelectedUngroupedCS;

    // File Names
    string m_ModelNameBase; // this is the name used in the execution string
    string m_DegenFileFull; //degengeom file name WITH .csv file extension
    string m_CompGeomFileFull; //geometry file used for panel method
    string m_SetupFile;
    string m_AdbFile;
    string m_HistoryFile;
    string m_LoadFile;
    string m_StabFile;
    string m_CutsFile;
    string m_SliceFile;

    IntParm m_GeomSet;

    // Reference Area Parms
    Parm m_Sref;
    Parm m_bref;
    Parm m_cref;
    string m_RefGeomID;
    IntParm m_RefFlag;

    BoolParm m_BatchModeFlag;

    // Mass Properties Parms
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

    // Other Setup Parameters
    Parm m_Vinf;
    Parm m_Rho;
    Parm m_ReCref;
    IntParm m_Precondition;
    BoolParm m_VortexLift;
    BoolParm m_LeadingEdgeSuction;
    BoolParm m_KTCorrection;
    BoolParm m_Symmetry;
    BoolParm m_Write2DFEMFlag;
    BoolParm m_ClMaxToggle;
    Parm m_ClMax;
    BoolParm m_MaxTurnToggle;
    Parm m_MaxTurnAngle;
    BoolParm m_FarDistToggle;
    Parm m_FarDist;

    // Unsteady
    BoolParm m_StabilityCalcFlag;
    IntParm m_StabilityType;

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

    BoolParm m_CpSliceXMinIsManual;
    BoolParm m_CpSliceXMaxIsManual;
    BoolParm m_CpSliceYMinIsManual;
    BoolParm m_CpSliceYMaxIsManual;
    Parm m_CpSliceXMin;
    Parm m_CpSliceXMax;
    Parm m_CpSliceYMin;
    Parm m_CpSliceYMax;

    BoolParm m_CpSliceYAxisFlipFlag;
    BoolParm m_CpSlicePlotLinesFlag;

    BoolParm m_UnsteadyXMinIsManual;
    BoolParm m_UnsteadyXMaxIsManual;
    BoolParm m_UnsteadyYMinIsManual;
    BoolParm m_UnsteadyYMaxIsManual;
    Parm m_UnsteadyXMin;
    Parm m_UnsteadyXMax;
    Parm m_UnsteadyYMin;
    Parm m_UnsteadyYMax;

    ProcessUtil m_SolverProcess;
    ProcessUtil m_SlicerThread;

protected:
    DrawObj m_HighlightDrawObj;

    BndBox m_BBox;

    int m_LastSelectedType;

    string m_LastPanelMeshGeomId;

    int WaitForFile( string filename );  // function is used to wait for the result to show up on the file system
    void GetSweepVectors( vector<double> &alphaVec, vector<double> &betaVec, vector<double> &machVec );

    void MonitorSolver( FILE * logFile );
    bool m_SolverProcessKill;

    // helper functions for VSPAERO files
    void ReadHistoryFile( string filename, vector <string> &res_id_vector, vsp::VSPAERO_ANALYSIS_METHOD analysisMethod );
    void ReadLoadFile( string filename, vector <string> &res_id_vector, vsp::VSPAERO_ANALYSIS_METHOD analysisMethod );
    void ReadStabFile( string filename, vector <string> &res_id_vector, vsp::VSPAERO_ANALYSIS_METHOD analysisMethod, vsp::VSPAERO_STABILITY_TYPE stabilityType );
    vector <string> ReadDelimLine( FILE * fp, char * delimeters );
    bool CheckForCaseHeader( std::vector<string> headerStr );
    bool CheckForResultHeader( std::vector < string > headerstr );
    int ReadVSPAEROCaseHeader( Results * res, FILE * fp, vsp::VSPAERO_ANALYSIS_METHOD analysisMethod );
    void ReadSliceFile( string filename, vector <string> &res_id_vector );

    void AddResultHeader( string res_id, double mach, double alpha, double beta, vsp::VSPAERO_ANALYSIS_METHOD analysisMethod );


private:
    VSPAEROMgrSingleton();
    VSPAEROMgrSingleton( VSPAEROMgrSingleton const& copy );            // Not Implemented
    VSPAEROMgrSingleton& operator=( VSPAEROMgrSingleton const& copy ); // Not Implemented

    vector< RotorDisk* > m_RotorDiskVec;
    vector< VspAeroControlSurf > m_CompleteControlSurfaceVec;   // list of all control and rectangle sub-surfaces in the model selected as control surfaces
    vector < VspAeroControlSurf > m_ActiveControlSurfaceVec;
    vector< ControlSurfaceGroup* > m_ControlSurfaceGroupVec;
    vector < CpSlice* > m_CpSliceVec;

    vector < DegenGeom > m_DegenGeomVec;

    int m_CurrentRotorDiskIndex;
    int m_CurrentCSGroupIndex;
    int m_CurrentCpSliceIndex;

    int m_CpSliceAnalysisType;

    bool m_Verbose;
};

#define VSPAEROMgr VSPAEROMgrSingleton::getInstance()

#endif
