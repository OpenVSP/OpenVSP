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
    BoolParm m_AutoHubDiaFlag; // Flag to pull hub diameter automatically from Prop Geom
    Parm m_CT;          // Rotor_CT_
    Parm m_CP;          // Rotor_CP_

    bool m_FlipNormalFlag;

    //identifying information for vsp model
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
    static void Load_STP_Data( FILE * InputFile );

    // vsp3 file xml I/O
    virtual xmlNodePtr EncodeXml( xmlNodePtr & node );
    virtual xmlNodePtr DecodeXml( xmlNodePtr & node );

    // Subsurface Manipulation
    void AddSubSurface( VspAeroControlSurf control_surf );
    void RemoveSubSurface( const string & ssid, int reflec_num );

    void SetGroupDisplaySuffix( int num );

};

class UnsteadyGroup : public ParmContainer
{
public:
    UnsteadyGroup( void );
    ~UnsteadyGroup( void );

    virtual xmlNodePtr EncodeXml( xmlNodePtr& node );
    virtual xmlNodePtr DecodeXml( xmlNodePtr& node );

    void ParmChanged( Parm* parm_ptr, int type );

    void Update();

    int WriteGroup( FILE* logFile );

    int GetSelectedCompIndex()                                                              { return m_SelectedCompIndex; }
    void SetSelectedCompIndex( int index );

    vector < pair < string, int > > GetCompSurfPairVec()                                    { return m_ComponentSurfPairVec; }
    void SetCompSurfPairVec( const vector < pair < string, int > > comp_surf_pair_vec )     { m_ComponentSurfPairVec = comp_surf_pair_vec; }

    vector < int > GetVSPAEROIndexVec()                                                     { return m_ComponentVSPAEROIndexVec; }
    void SetVSPAEROIndexVec( vector < int > vepaero_index_vec )                             { m_ComponentVSPAEROIndexVec = vepaero_index_vec; }

    void AddComp( string comp_id, int surf_ind )                                            { m_ComponentSurfPairVec.push_back( std::make_pair( comp_id, surf_ind ) ); }
    void ClearCompIDVec()                                                                   { m_ComponentSurfPairVec.clear(); }
    void RemoveComp( int index );

    enum GEOM_PROPERTY_TYPE
    {
        GEOM_FIXED,
        GEOM_DYNAMIC,
        GEOM_ROTOR
    };

    string m_GroupName;
    bool m_ReverseFlag; // Flag to flip the RPM but leave the normal vector unchanged

    IntParm m_GeomPropertyType;
    Parm m_RotorDia;
    Parm m_Ox;
    Parm m_Oy;
    Parm m_Oz;
    Parm m_Rx;
    Parm m_Ry;
    Parm m_Rz;
    Parm m_Vx;
    Parm m_Vy;
    Parm m_Vz;
    Parm m_Ax;
    Parm m_Ay;
    Parm m_Az;
    Parm m_RPM;
    Parm m_Mass;
    Parm m_Ixx;
    Parm m_Iyy;
    Parm m_Izz;
    Parm m_Ixy;
    Parm m_Ixz;
    Parm m_Iyz;

private:

    vector < pair < string, int > > m_ComponentSurfPairVec; // Pairs of component IDs and symmetric surface index for the unsteady group
    vector < int > m_ComponentVSPAEROIndexVec;
    int m_SelectedCompIndex;
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
        UNSTEADY_GROUP      // user has selected an unsteady group
    };

    static VSPAEROMgrSingleton& getInstance()
    {
        static VSPAEROMgrSingleton instance;
        return instance;
    }
    virtual void ParmChanged( Parm* parm_ptr, int type );

    static void ClearAllPreviousResults();

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
    void UpdateUnsteadyGroups();
    void UpdateParmRestrictions();
    void UpdateAutoTimeStep();
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
    vector < VspAeroControlSurf > GetAvailableCSVec();
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

    string LoadExistingVSPAEROResults();

    // CpSlicer Functionality and Variables
    string ComputeCpSlices( FILE * logFile = NULL );
    string ExecuteCpSlicer( FILE * logFile = NULL );
    void ComputeQuadTreeSlices( FILE * logFile = NULL );
    void ExecuteQuadTreeSlicer( FILE * logFile = NULL );
    static void ClearCpSliceResults();
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

    // Rotor Disk Functionality
    void AddRotorDisk();
    bool ValidRotorDiskIndex( int index );
    void UpdateRotorDiskSuffix();
    RotorDisk* GetRotorDisk( int index );

    // Control Surface Group Functionality
    void InitControlSurfaceGroups();        // default initial grouping of control surfaces for VSPAERO
    void AddControlSurfaceGroup();
    void RemoveControlSurfaceGroup();
    void AddSelectedToCSGroup();
    void AddAllToCSGroup();
    void RemoveSelectedFromCSGroup();
    void RemoveAllFromCSGroup();
    void UpdateControlSurfaceGroupSuffix();

    // Unsteady Group Functions
    int CreateGroupsFile();
    void SetCurrentUnsteadyGroupIndex( int index );
    void SetCurrentUnsteadyGroupIndex( const string& id );
    int GetCurrentUnsteadyGroupIndex()                          { return m_CurrentUnsteadyGroupIndex; }
    UnsteadyGroup* AddUnsteadyGroup();
    void DeleteUnsteadyGroup( int index );
    bool ValidUnsteadyGroupInd( int index );
    void AddUnsteadyGroup( UnsteadyGroup* group )               { m_UnsteadyGroupVec.push_back( group ); }
    UnsteadyGroup* GetUnsteadyGroup( int index );
    int GetUnsteadyGroupIndex( const string& id );
    string GetUnsteadyGroupID( int index );
    void ClearUnsteadyGroupVec();
    int NumUnsteadyGroups()                                  { return m_UnsteadyGroupVec.size(); }
    int NumUnsteadyRotorGroups();
    void HighlightUnsteadyGroup( vector < DrawObj* >& draw_obj_vec );
    map < pair < string, int >, vector < int > > GetVSPAEROGeomIndexMap( int set_index );
    string ExecuteNoiseAnalysis( FILE* logFile, int noise_type, int noise_unit );

    virtual void AddLinkableParms( vector < string > & linkable_parm_vec, const string & link_container_id );

    // Highlighter Methods and Variables
    void HighlightSelected( int type );
    void LoadDrawObjs( vector < DrawObj* > & draw_obj_vec );

    IntParm m_AnalysisMethod;

    BoolParm m_CpSliceFlag;

    vector < int > m_SelectedGroupedCS;
    vector < int > m_SelectedUngroupedCS;

    // File Names
    string m_ModelNameBase; // this is the name used in the execution string
    string m_DegenFileFull; //degengeom file name WITH .csv file extension
    string m_CompGeomFileFull; //geometry file used for panel method
    string m_VSPGeomFileFull; // Experimental file format for mixed thick/thin surface representation
    string m_SetupFile;
    string m_AdbFile;
    string m_HistoryFile;
    string m_PolarFile;
    string m_LoadFile;
    string m_StabFile;
    string m_CutsFile;
    string m_SliceFile;
    string m_GroupsFile;
    vector < string > m_GroupResFiles;
    vector < string > m_RotorResFiles;
    vector < string > m_UnsteadyGroupResNames;

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
    Parm m_ReCrefStart, m_ReCrefEnd; IntParm m_ReCrefNpts;

    // Solver settings
    IntParm m_NCPU;
    BoolParm m_FixedWakeFlag;
    IntParm m_WakeNumIter;
    PowIntParm m_NumWakeNodes;

    // Other Setup Parameters
    Parm m_Vinf;
    Parm m_Rho;
    Parm m_Machref;
    Parm m_Vref;
    BoolParm m_ManualVrefFlag;
    IntParm m_Precondition;
    BoolParm m_KTCorrection;
    BoolParm m_Symmetry;
    BoolParm m_Write2DFEMFlag;
    BoolParm m_AlternateInputFormatFlag;
    IntParm m_ClMaxToggle;
    Parm m_ClMax;
    BoolParm m_MaxTurnToggle;
    Parm m_MaxTurnAngle;
    BoolParm m_FarDistToggle;
    Parm m_FarDist;
    BoolParm m_GroundEffectToggle;
    Parm m_GroundEffect;

    BoolParm m_RotateBladesFlag;
    BoolParm m_ActuatorDiskFlag;
    IntParm m_StabilityType;

    // Unsteady
    Parm m_TimeStepSize;
    IntParm m_NumTimeSteps;
    BoolParm m_AutoTimeStepFlag;
    IntParm m_AutoTimeNumRevs;
    BoolParm m_HoverRampFlag;
    Parm m_HoverRamp;
    BoolParm m_FromSteadyState;
    BoolParm m_NoiseCalcFlag;
    IntParm m_NoiseCalcType;
    IntParm m_NoiseUnits;
    BoolParm m_UniformPropRPMFlag;

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

    IntParm m_UnsteadyGroupSelectType;

    enum UNSTEADY_TYPE_SELECT
    {
        HISTORY_SELECT_TYPE,
        GROUP_SELECT_TYPE,
        ROTOR_SELECT_TYPE
    };

    IntParm m_LoadDistSelectType;

    enum LOAD_TYPE_SELECT
    {
        LOAD_SELECT_TYPE,
        BLADE_SELECT_TYPE
    };

    ProcessUtil m_SolverProcess; 
    ProcessUtil m_SlicerThread;

protected:
    static int WaitForFile( string filename );  // function is used to wait for the result to show up on the file system
    void GetSweepVectors( vector<double> &alphaVec, vector<double> &betaVec, vector<double> &machVec, vector<double> &recrefVec );

    void MonitorSolver( FILE * logFile );
    bool m_SolverProcessKill;

    // helper functions for VSPAERO files
    void ReadHistoryFile( string filename, vector <string> &res_id_vector, vsp::VSPAERO_ANALYSIS_METHOD analysisMethod, double recref );
    void ReadPolarFile( string filename, vector <string> &res_id_vector, double recref );
    void ReadLoadFile( string filename, vector <string> &res_id_vector, vsp::VSPAERO_ANALYSIS_METHOD analysisMethod );
    void ReadStabFile( string filename, vector <string> &res_id_vector, vsp::VSPAERO_ANALYSIS_METHOD analysisMethod, vsp::VSPAERO_STABILITY_TYPE stabilityType );
    static vector <string> ReadDelimLine( FILE * fp, char * delimiters );
    static bool CheckForCaseHeader( std::vector<string> headerStr );
    static bool CheckForResultHeader( std::vector < string > headerstr );
    static int ReadVSPAEROCaseHeader( Results * res, FILE * fp, vsp::VSPAERO_ANALYSIS_METHOD analysisMethod );
    void ReadSetupFile(); // Read the VSPAERO setup file to identify VSPAERO inputs needed to generate existing VSPAERO results
    void ReadSliceFile( string filename, vector <string> &res_id_vector );
    void ReadGroupResFile( string filename, vector <string> &res_id_vector, string group_name = "" );
    void ReadRotorResFile( string filename, vector <string> &res_id_vector, string group_name = "" );
    static void AddResultHeader( string res_id, double mach, double alpha, double beta, vsp::VSPAERO_ANALYSIS_METHOD analysisMethod );

    DrawObj m_HighlightDrawObj;

    BndBox m_BBox;

    int m_LastSelectedType;

    string m_LastPanelMeshGeomId;

private:
    VSPAEROMgrSingleton();
    VSPAEROMgrSingleton( VSPAEROMgrSingleton const& copy );            // Not Implemented
    VSPAEROMgrSingleton& operator=( VSPAEROMgrSingleton const& copy ); // Not Implemented

    vector< RotorDisk* > m_RotorDiskVec;
    vector< VspAeroControlSurf > m_CompleteControlSurfaceVec;   // list of all control and rectangle sub-surfaces in the model selected as control surfaces
    vector < VspAeroControlSurf > m_ActiveControlSurfaceVec;
    vector< ControlSurfaceGroup* > m_ControlSurfaceGroupVec;
    vector < CpSlice* > m_CpSliceVec;
    vector < UnsteadyGroup* > m_UnsteadyGroupVec;

    vector < DegenGeom > m_DegenGeomVec;

    int m_CurrentRotorDiskIndex;
    int m_CurrentCSGroupIndex;
    int m_CurrentCpSliceIndex;
    int m_CurrentUnsteadyGroupIndex;

    int m_CpSliceAnalysisType;

    bool m_Verbose;

    int m_iCase; // Index corresponding to the current VSPAERO solution case in non-batch mode runs (see ComputeSolverSingle)

    // VSPAERO Inputs from Existing Results
    int m_PreviousStabilityType;
    int m_PreviousNumUnsteadyGroups;
    int m_PreviousNumUnsteadyProps;
};

#define VSPAEROMgr VSPAEROMgrSingleton::getInstance()

#endif
