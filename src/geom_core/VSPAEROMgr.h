//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//

// VSPAEROMgr.h: interface for the VSPAERO Singleton.
//
//////////////////////////////////////////////////////////////////////

#if !defined(VSPAERO_INCLUDED_)
#define VSPAERO_INCLUDED_

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

class PropDriverGroup : public DriverGroup
{
public:
    PropDriverGroup();

    virtual void UpdateGroup( vector< string > parmIDs );
    virtual bool ValidDrivers( vector< int > choices );

    double Vinf;
    double rho;
    double D;
};

class RotorDisk : public ParmContainer
{
public:
    // Constructor, Destructor, Copy
    RotorDisk();
    ~RotorDisk();

    void Update( double V, double rho );

    // Setup File I/O
    void Write_STP_Data( FILE * InputFile ) const;

    vector< string > GetDriverParms() const;

    string GetParentID() const                  { return m_ParentGeomId; }
    unsigned int GetSurfNum() const             { return m_ParentGeomSurfNdx; }

    // vsp3 file xml I/O
    virtual xmlNodePtr EncodeXml( xmlNodePtr & node );
    virtual xmlNodePtr DecodeXml( xmlNodePtr & node );

    string m_GroupName;
    int m_GroupSuffix;

    vec3d m_XYZ;           // RotorXYZ_
    vec3d m_Normal;        // RotorNormal_

    Parm m_Diameter;       // RotorDiameter_
    Parm m_HubDiameter;    // RotorHubDiameter_
    Parm m_RPM;          // RotorRPM_
    BoolParm m_AutoHubDiaFlag; // Flag to pull hub diameter automatically from Prop Geom
    Parm m_CT;          // Rotor_CT_
    Parm m_CP;          // Rotor_CP_
    Parm m_CQ;          // Rotor_CQ_

    Parm m_T;
    Parm m_P;
    Parm m_Q;
    Parm m_J;
    Parm m_eta;

    PropDriverGroup m_DriverGroup;

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

    VspSurf CreateSurf() const;

    vector < DrawObj > m_CpSliceDOVec;

};

class ControlSurfaceGroup : public ParmContainer
{
public:
    // Constructor, Destructor, Copy

    ControlSurfaceGroup();
    ~ControlSurfaceGroup();

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
    void AddSubSurface( const VspAeroControlSurf &control_surf );
    void RemoveSubSurface( const string & ssid, int reflec_num );

    void SetGroupDisplaySuffix( int num );

};

class UnsteadyGroup : public ParmContainer
{
public:
    UnsteadyGroup();
    ~UnsteadyGroup();

    virtual xmlNodePtr EncodeXml( xmlNodePtr& node );
    virtual xmlNodePtr DecodeXml( xmlNodePtr& node );

    void ParmChanged( Parm* parm_ptr, int type );

    void Update();

    int WriteGroup( FILE *group_file );

    vector < pair < string, int > > GetCompSurfPairVec()                                    { return m_ComponentSurfPairVec; }
    void SetCompSurfPairVec( const vector < pair < string, int > > &comp_surf_pair_vec )    { m_ComponentSurfPairVec = comp_surf_pair_vec; }

    void SetVSPAEROIndexVec( const vector < int > &vepaero_index_vec )                      { m_ComponentVSPAEROIndexVec = vepaero_index_vec; }
    void SetGeomIDsInGroup( const vector < string > &gidvec )                               { m_GeomIDsInGroup = gidvec; }

    void AddComp( const string & comp_id, int surf_ind )                                    { m_ComponentSurfPairVec.push_back( std::make_pair( comp_id, surf_ind ) ); }

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

    vector < string > m_GeomIDsInGroup; // Used with vspgeom files
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
    int GetCurrentRotorDiskIndex() const                          { return m_CurrentRotorDiskIndex; }
    vector <RotorDisk*> GetRotorDiskVec()                      { return m_RotorDiskVec; };
    vector < VspAeroControlSurf > GetActiveCSVec()          { return m_ActiveControlSurfaceVec; }
    vector < VspAeroControlSurf > GetCompleteCSVec()        { return m_CompleteControlSurfaceVec; }
    vector < VspAeroControlSurf > GetAvailableCSVec();
    int GetCurrentCSGroupIndex() const                            { return m_CurrentCSGroupIndex; }
    string GetCurrentCSGGroupName();
    vector <ControlSurfaceGroup* > GetControlSurfaceGroupVec()   { return m_ControlSurfaceGroupVec; }
    vector < int > GetSelectedGroupedItems() const                { return m_SelectedGroupedCS; }
    vector < int > GetSelectedUngroupedItems() const              { return m_SelectedUngroupedCS; }

    // VSP Aero Functionality and Variables
    string ComputeGeometry();
    string CreateSetupFile();                          // natively creates a *.vspaero template setup file
    string ComputeSolver( FILE * logFile = nullptr ); // returns a result with a vector of results id's under the name ResultVec
    string ComputeSolverBatch( FILE * logFile = nullptr );
    ProcessUtil* GetSolverProcess();
    bool IsSolverRunning();
    void KillSolver();

    static int ExportResultsToCSV( const string &fileName );

    string LoadExistingVSPAEROResults();

    // CpSlicer Functionality and Variables
    string ComputeCpSlices( FILE * logFile = nullptr );
    string ExecuteCpSlicer( FILE * logFile = nullptr );
    void ComputeQuadTreeSlices( FILE * logFile = nullptr );
    void ExecuteQuadTreeSlicer( FILE * logFile = nullptr );
    static void ClearCpSliceResults();
    void CreateCutsFile();
    void AddCpSlice( CpSlice* slice )                      { m_CpSliceVec.push_back( slice ); }
    CpSlice* AddCpSlice();
    bool ValidCpSliceInd( int ind );
    void DelCpSlice( int ind );
    CpSlice* GetCpSlice( int ind );
    int GetCpSliceIndex( const string & id );
    void AddCpSliceVec( int cut_type, const vector < double > &cut_vec );
    vector < double > GetCpSlicePosVec(int type );
    vector < CpSlice* > GetCpSliceVec()                    { return m_CpSliceVec; }
    void ClearCpSliceVec();
    int GetCurrentCpSliceIndex() const                           { return m_CurrentCpSliceIndex; }
    void SetCurrentCpSliceIndex( int index )               { m_CurrentCpSliceIndex = index; }

    // Rotor Disk Functionality
    void AddRotorDisk();
    bool ValidRotorDiskIndex( int index );
    void UpdateRotorDiskSuffix();
    RotorDisk* GetRotorDisk( int index );
    void PurgeDuplicateRotorDisks();

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
    int GetCurrentUnsteadyGroupIndex() const                          { return m_CurrentUnsteadyGroupIndex; }
    UnsteadyGroup* AddUnsteadyGroup();
    void DeleteUnsteadyGroup( const vector <int> &ind_vec );
    bool ValidUnsteadyGroupInd( int index );
    void AddUnsteadyGroup( UnsteadyGroup* group )               { m_UnsteadyGroupVec.push_back( group ); }
    UnsteadyGroup* GetUnsteadyGroup( int index );
    int GetUnsteadyGroupIndex( const string& id );
    string GetUnsteadyGroupID( int index );
    void ClearUnsteadyGroupVec();
    int NumUnsteadyGroups()                                  { return m_UnsteadyGroupVec.size(); }
    int NumUnsteadyRotorGroups();
    void HighlightUnsteadyGroup( vector < DrawObj* >& draw_obj_vec );
    static map < pair < string, int >, vector < int > > GetVSPAEROGeomIndexMap( int set_index, int thin_set );
    string ExecuteNoiseAnalysis( FILE* logFile, int noise_type, int noise_unit );

    virtual void AddLinkableParms( vector < string > & linkable_parm_vec, const string & link_container_id );

    // Highlighter Methods and Variables
    static void HighlightSelected( int type );
    void LoadDrawObjs( vector < DrawObj* > & draw_obj_vec );

    BoolParm m_CpSliceFlag;

    vector < int > m_SelectedGroupedCS;
    vector < int > m_SelectedUngroupedCS;

    // File Names
    string m_ModelNameBase; // this is the name used in the execution string
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
    IntParm m_ThinGeomSet;

    BoolParm m_UseMode;
    string m_ModeID;

    BoolParm m_CGUseMode;
    string m_CGModeID;

    Parm m_CullFrac;
    BoolParm m_CullFracFlag;
    BoolParm m_ContinueCoPlanarWakesFlag;

    // Reference Area Parms
    Parm m_Sref;
    Parm m_bref;
    Parm m_cref;
    string m_RefGeomID;
    IntParm m_RefFlag;
    BoolParm m_MACFlag;
    BoolParm m_SCurveFlag;

    // Mass Properties Parms
    IntParm m_CGGeomSet;
    IntParm m_CGDegenSet;
    IntParm m_NumMassSlice;
    IntParm m_MassSliceDir;
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

    bool m_StopBeforeRun;

    ProcessUtil m_SolverProcess; 
    ProcessUtil m_SlicerThread;

protected:
    void GetSweepVectors( vector<double> &alphaVec, vector<double> &betaVec, vector<double> &machVec, vector<double> &recrefVec ) const;

    bool m_SolverProcessKill;

    // helper functions for VSPAERO files
    void ReadHistoryFile( const string &filename, vector <string> &res_id_vector, double recref ) const;
    void ReadPolarFile( const string &filename, vector <string> &res_id_vector, double recref ) const;
    void ReadLoadFile( const string &filename, vector <string> &res_id_vector ) const;
    void ReadStabFile( const string &filename, vector <string> &res_id_vector, vsp::VSPAERO_STABILITY_TYPE stabilityType );
    static vector <string> ReadDelimLine( FILE * fp, char * delimiters );
    static bool CheckForCaseHeader( const std::vector<string> &headerStr );
    static bool CheckForResultHeader( const std::vector < string > &headerstr );
    static int ReadVSPAEROCaseHeader( Results * res, FILE * fp );
    void ReadSetupFile(); // Read the VSPAERO setup file to identify VSPAERO inputs needed to generate existing VSPAERO results
    void ReadSliceFile( const string &filename, vector <string> &res_id_vector ) const;
    static void ReadGroupResFile( const string &filename, vector <string> &res_id_vector, const string &group_name = "" );
    static void ReadRotorResFile( const string &filename, vector <string> &res_id_vector, const string &group_name = "" );

    DrawObj m_HighlightDrawObj;

    BndBox m_BBox;

    int m_LastSelectedType;

    string m_LastPanelMeshGeomId;

private:
    VSPAEROMgrSingleton();
    VSPAEROMgrSingleton( VSPAEROMgrSingleton const& copy ) = delete;            // Not Implemented
    VSPAEROMgrSingleton& operator=( VSPAEROMgrSingleton const& copy ) = delete; // Not Implemented
    ~VSPAEROMgrSingleton();

    vector< RotorDisk* > m_RotorDiskVec;
    vector< VspAeroControlSurf > m_CompleteControlSurfaceVec;   // list of all control and rectangle sub-surfaces in the model selected as control surfaces
    vector < VspAeroControlSurf > m_ActiveControlSurfaceVec;
    vector< ControlSurfaceGroup* > m_ControlSurfaceGroupVec;
    vector < CpSlice* > m_CpSliceVec;
    vector < UnsteadyGroup* > m_UnsteadyGroupVec;

    int m_CurrentRotorDiskIndex;
    int m_CurrentCSGroupIndex;
    int m_CurrentCpSliceIndex;
    int m_CurrentUnsteadyGroupIndex;

    bool m_Verbose;

    // VSPAERO Inputs from Existing Results
    int m_PreviousStabilityType;
    int m_PreviousNumUnsteadyGroups;
    int m_PreviousNumUnsteadyProps;
};

#define VSPAEROMgr VSPAEROMgrSingleton::getInstance()

#endif
