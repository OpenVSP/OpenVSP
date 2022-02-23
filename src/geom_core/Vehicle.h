//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//

// Vehicle.h: interface for the Vehicle Class.
// J.R Gloudemans
//
//////////////////////////////////////////////////////////////////////

#if !defined(VEHICLE__INCLUDED_)
#define VEHICLE__INCLUDED_

#include "Defines.h"
#include "Parm.h"
#include "ParmContainer.h"
#include "ParmUndo.h"
#include "Geom.h"
#include "MessageMgr.h"
#include "DrawObj.h"
#include "LightMgr.h"
#include "DegenGeom.h"
#include "MeshCommonSettings.h"
#include "ClippingMgr.h"
#include "SnapTo.h"
#include "CADutil.h"
#include "XferSurf.h"
#include "MaterialMgr.h"
#include "WaveDragMgr.h"
#include "GroupTransformations.h"

#include <cassert>

#include <string>
#include <vector>
#include <deque>
#include <stack>
#include <memory>

// File versions must be integers.
#define MIN_FILE_VER 4 // Lowest file version number for 3.X vsp file
#define CURRENT_FILE_VER 5 // File version number for 3.X files that this executable writes

// We have not made substantial use of this flag to determine file compatibility issues.  However,
// its use will likely increase going forward.  Most parameters additions and file format changes
// can be made without incrementing the format number.  Increments should be used to ensure compatibility.
//
// 4 -- 3.0      Base 3.X file.
// 5 -- 3.17.1   Add support for scaling thickness of file-type airfoils.
//

#define NUM_SETS 20 // Number of sets
#define DEFAULT_SET vsp::SET_TYPE::SET_SHOWN // Default set index

//==== Vehicle ====//
class Vehicle : public ParmContainer
{
public:

    Vehicle();
    virtual ~Vehicle();

    void Init();
    static void RunTestScripts();
    void Renew();

    void GenAPIDocs( const string & file_name );

    void ParmChanged( Parm* parm_ptr, int type );
    static void UnDo();

    void Update( bool fullupdate = true );
    void UpdateManagers();
    void UpdateGeom( const string &geom_id );
    void ForceUpdate( int dirtyflag = GeomBase::NONE );
    static void UpdateGui();
    static int RunScript( const string & file_name, const string & function_name = "main" );

    Geom* FindGeom( const string & geom_id );
    vector< Geom* > FindGeomVec( const vector< string > & geom_id_vec );

    string CreateGeom( const GeomType & type );
    string AddGeom( const GeomType & type );
    string AddGeom( Geom* add_geom );
    string AddMeshGeom( int normal_set, int degen_set = vsp::SET_NONE, bool suppressdisks = false );

    virtual void AddLinkableContainers( vector< string > & linkable_container_vec );

    //==== Get All Geoms (Does NOT Return Collapsed Geoms if check_display_flag == true) ====//
    vector< string > GetGeomVec( bool check_display_flag = false );
    vector< Geom* > GetGeomStoreVec()                                { return m_GeomStoreVec; }
    vector < int > GetDegenGeomTypeVec( int set_index );
    void AddActiveGeom( const string & id );
    void SetActiveGeom( const string & id );
    void SetActiveGeomVec( vector< string > const & geom_id_vec )    { m_ActiveGeom = geom_id_vec; }
    void ClearActiveGeom()                                           { m_ActiveGeom.clear(); }
    vector< string > GetActiveGeomVec()                              { return m_ActiveGeom; }
    vector< Geom* > GetActiveGeomPtrVec()                            { return FindGeomVec( m_ActiveGeom ); }

    bool IsGeomActive( const string & geom_id );
    void ReorderActiveGeom( int direction );

    void CutActiveGeomVec();
    void DeleteActiveGeomVec();
    void CopyActiveGeomVec();
    void DeleteGeomVec( const vector<string> & del_vec );
    void DeleteGeom( const string & geom_id );
    void CutGeomVec( const vector<string> & cut_vec );
    void RemoveGeomVecFromHierarchy( const vector<string> & cut_vec );
    void DeleteClipBoard();
    vector< string > PasteClipboard();
    vector< string > CopyGeomVec( const vector<string> & geom_vec );

    vector< DrawObj* > GetDrawObjs();

    //==== Reset DrawObjs' m_GeomChanged flag to false. ====//
    void ResetDrawObjsGeomChangedFlags();

    //==== Geom Sets ====//
    void SetSetName( int index, const string& name );
    vector< string > GetSetNameVec( bool includeNone = false );
    void ShowOnlySet( int index );
    void NoShowSet( int index );
    void ShowSet( int index );
    vector< string > GetGeomSet( int index );

    void HideAllExcept( const string& id );
    void HideAll();

    void CopyPasteSet(int copyIndex, int pasteIndex);

    //==== Geom Type Data =====//
    vector< string > GetValidTypeGeoms();
    vector< GeomType > GetEditableGeomTypes();
    void AddType( const string & geom_id );
    void DeleteType( int index );

    int GetNumGeomTypes()                                    { return ( int )m_GeomTypeVec.size(); }
    int GetNumFixedGeomTypes();
    GeomType GetGeomType( int index );
    void SetGeomType( int index, const GeomType & type );

    BndBox GetBndBox()                                        { return m_BBox; }
    void UpdateBBox();
    bool GetVisibleBndBox( BndBox &b );

    xmlNodePtr EncodeXml( xmlNodePtr & node, int set );
    xmlNodePtr DecodeXml( xmlNodePtr & node );

    xmlNodePtr DecodeXmlGeomsOnly( xmlNodePtr & node );

    enum { REORDER_MOVE_UP, REORDER_MOVE_DOWN, REORDER_MOVE_TOP, REORDER_MOVE_BOTTOM };

    bool ExistMesh( int set );

    vector < string > GetPtCloudGeoms();

    int ReadXMLFile( const string & file_name );
    int ReadXMLFileGeomsOnly( const string & file_name );

    void SetVSP3FileName( const string & f_name );
    string GetVSP3FileName()                                { return m_VSP3FileName; }
    int GetFileVersion()                                    { return m_FileOpenVersion; }

    void SetupPaths();

    string GetExePath()                                     { return m_ExePath; }
    string GetHomePath()                                    { return m_HomePath; }
    string GetVSPAEROPath()                                 { return m_VSPAEROPath; }
    string GetVSPAEROCmd()                                  { return m_VSPAEROCmd; }
    string GetVIEWERCmd()                                   { return m_VIEWERCmd; }
    string GetSLICERCmd()                                   { return m_SLICERCmd; }

    bool CheckForVSPAERO( const string & path );
    bool SetVSPAEROPath( const string & path );

    //==== Get Script Dir to Write Scripts ====//
    string GetWriteScriptDir()                              { return m_CustomScriptDirs[0]; }
    vector < string > GetCustomScriptDirs()                 { return m_CustomScriptDirs; }

    //=== Export Files ===//
    // Return Mesh Geom ID if the export generates a mesh, otherwise return an 
    // empty string. This facilitates deleting the generated mesh from the API.
    string ExportFile( const string & file_name, int write_set, int degen_set, int file_type );
    bool WriteXMLFile( const string & file_name, int set );
    void WriteXSecFile( const string & file_name, int write_set );
    void WritePLOT3DFile( const string & file_name, int write_set );
    string WriteSTLFile( const string & file_name, int write_set );
    string WriteTaggedMSSTLFile( const string & file_name, int write_set );
    string WriteFacetFile( const string & file_name, int write_set );
    string WriteTRIFile( const string & file_name, int write_set );
    string WriteOBJFile( const string & file_name, int write_set );
    string WriteVSPGeomFile( const string & file_name, int write_set, int degen_set, bool half_flag = false, bool hideset = true, bool suppressdisks = false );
    string WriteNascartFiles( const string & file_name, int write_set );
    string WriteGmshFile( const string & file_name, int write_set );
    void WriteX3DFile( const string & file_name, int write_set );
    static void WriteX3DMaterial( xmlNodePtr node, Material * material );
    void WriteX3DViewpoints( xmlNodePtr node );
    static void WriteX3DViewpointProps( xmlNodePtr node, string orients, string cents, string posits, string sfov, string name );
    void WritePovRayFile( const string & file_name, int write_set );
    void WriteSTEPFile( const string & file_name, int write_set );
    void WriteSTEPFile( const string & file_name, int write_set, bool labelID,
                        bool labelName, bool labelSurfNo, int delimType );
    void WriteStructureSTEPFile( const string & file_name );
    void WriteIGESFile( const string & file_name, int write_set );
    void WriteIGESFile( const string & file_name, int write_set, int lenUnit, bool splitSubSurfs, bool splitSurfs,
                        bool toCubic, double toCubicTol, bool trimTE, bool labelID, bool labelName,
                        bool labelSurfNo, bool labelSplitNo, int delimType );
    void WriteStructureIGESFile( const string & file_name );
    void WriteStructureIGESFile( const string & file_name, int feaMeshStructIndex, bool splitSurfs,
                        bool toCubic, double toCubicTol, bool labelID, bool labelName,
                        bool labelSurfNo, bool labelSplitNo, int delimType );

    void WriteBEMFile( const string & file_name, int write_set );
    void WriteAirfoilFile( const string & file_name, int write_set );
    void WriteDXFFile( const string & file_name, int write_set );
    void WriteSVGFile( const string & file_name, int write_set );
    void WritePMARCFile( const string & file_name, int write_set );

    void WriteVehProjectionLinesDXF( FILE * file_name, const BndBox &dxfbox );
    void WriteVehProjectionLinesSVG( xmlNodePtr root, const BndBox &svgbox );

    vector< vector < vec3d > > GetVehProjectionLines( int view, vec3d offset );

    virtual void SetVehProjectVec3d( const vector < vector < vec3d > > &polyvec, int dir_index )
    {
        m_VehProjectVec3d[dir_index] = polyvec;
    }
    virtual vector < vector < vec3d > >  GetVehProjectVec3d( int dir_index )
    {
        return m_VehProjectVec3d[dir_index];
    }

    void FetchXFerSurfs( int write_set, vector< XferSurf > &xfersurfs );
    //==== Computation File Names ====//
    string getExportFileName( int type );
    void setExportFileName( int type, string f_name );
    void resetExportFileNames();

    bool getExportCompGeomCsvFile()                    { return m_exportCompGeomCsvFile(); }
    void setExportCompGeomCsvFile( bool b )            { m_exportCompGeomCsvFile.Set( b ); }

    bool getExportDegenGeomCsvFile( )                  { return m_exportDegenGeomCsvFile(); }
    bool getExportDegenGeomMFile( )                    { return m_exportDegenGeomMFile(); }
    void setExportDegenGeomCsvFile( bool b )           { m_exportDegenGeomCsvFile.Set( b ); }
    void setExportDegenGeomMFile( bool b )             { m_exportDegenGeomMFile.Set( b ); }

    //==== Import Files ====//
    string ImportFile( const string & file_name, int file_type );

    string ImportV2File( const string & file_name );

    //Comp Geom
    string CompGeom( int set, int degenset, int halfFlag, int intSubsFlag = 1, bool hideset = true, bool suppressdisks = false );
    string CompGeomAndFlatten( int set, int halfFlag, int intSubsFlag = 1, int degenset = vsp::SET_NONE, bool hideset = true, bool suppressdisks = false );
    string MassProps( int set, int numSlices, bool hidegeom = true, bool writefile = true );
    string MassPropsAndFlatten( int set, int numSlices, bool hidegeom = true, bool writefile = true );
    string PSlice( int set, int numSlices, vec3d norm, bool autoBoundsFlag, double start = 0, double end = 0 );
    string PSliceAndFlatten( int set, int numSlices, vec3d norm, bool autoBoundsFlag, double start = 0, double end = 0 );

    //==== Degenerate Geometry ====//
    void CreateDegenGeom( int set );
    vector< DegenGeom > GetDegenGeomVec()    { return m_DegenGeomVec; }
    string WriteDegenGeomFile();
    void ClearDegenGeom()   { m_DegenGeomVec.clear(); }

    //==== Surface Query ====//
    vec3d CompPnt01(const std::string &geom_id, const int &surf_indx, const double &u, const double &w);
    vec3d CompNorm01(const std::string &geom_id, const int &surf_indx, const double &u, const double &w);
    void CompCurvature01(const std::string &geom_id, const int &surf_indx, const double &u, const double &w, double &k1, double &k2, double &ka, double &kg);
    double ProjPnt01I(const std::string &geom_id, const vec3d & pt, int &surf_indx, double &u, double &w);
    double AxisProjPnt01I(const std::string &geom_id, const int &iaxis, const vec3d &pt, int &surf_indx_out, double &u_out, double &w_out, vec3d &p_out );

    //=== Surface API ===//
    string ExportSurfacePatches( int set );

    CfdMeshSettings* GetCfdSettingsPtr()
    {
        return &m_CfdSettings;
    }

    IntersectSettings* GetISectSettingsPtr()
    {
        return &m_ISectSettings;
    }

    virtual GridDensity* GetCfdGridDensityPtr()
    {
        return &m_CfdGridDensity;
    }

    ClippingMgr* GetClippinMgrPtr()
    {
        return &m_ClippingMgr;
    }

    SnapTo* GetSnapToPtr()
    {
        return &m_SnapTo;
    }

    // ==== Getter for GroupTransformations ==== //
    GroupTransformations* GetGroupTransformationsPtr()
    {
        return &m_GroupTransformations;
    }

    virtual vec3d GetXSecLineColor()                    { return m_XSecLineColor; }
    virtual void SetXSecLineColor( vec3d color )        { m_XSecLineColor = color; }

    //==== Mass Properties ====//
    vec3d m_IxxIyyIzz;
    vec3d m_IxyIxzIyz;
    vec3d m_CG;
    double m_TotalMass;

    IntParm m_NumMassSlices;
    BoolParm m_DrawCgFlag;

    IntParm m_NumPlanerSlices;
    BoolParm m_AutoBoundsFlag;
    Parm m_PlanarStartLocation;
    Parm m_PlanarEndLocation;
    IntParm m_PlanarAxisType;

    Parm m_BbXLen;
    Parm m_BbYLen;
    Parm m_BbZLen;
    Parm m_BbXMin;
    Parm m_BbYMin;
    Parm m_BbZMin;

    IntParm m_STEPLenUnit;
    Parm m_STEPTol;
    BoolParm m_STEPSplitSurfs;
    BoolParm m_STEPSplitSubSurfs;
    BoolParm m_STEPMergePoints;
    BoolParm m_STEPToCubic;
    Parm m_STEPToCubicTol;
    BoolParm m_STEPTrimTE;
    BoolParm m_STEPExportPropMainSurf;

    BoolParm m_STEPLabelID;
    BoolParm m_STEPLabelName;
    BoolParm m_STEPLabelSurfNo;
    IntParm m_STEPLabelDelim;

    IntParm m_STEPStructureExportIndex;
    Parm m_STEPStructureTol;
    BoolParm m_STEPStructureSplitSurfs;
    BoolParm m_STEPStructureMergePoints;
    BoolParm m_STEPStructureToCubic;
    Parm m_STEPStructureToCubicTol;

    BoolParm m_STEPStructureLabelID;
    BoolParm m_STEPStructureLabelName;
    BoolParm m_STEPStructureLabelSurfNo;
    IntParm m_STEPStructureLabelDelim;

    IntParm m_IGESLenUnit;
    BoolParm m_IGESSplitSurfs;
    BoolParm m_IGESSplitSubSurfs;
    BoolParm m_IGESToCubic;
    Parm m_IGESToCubicTol;
    BoolParm m_IGESTrimTE;
    BoolParm m_IGESExportPropMainSurf;

    BoolParm m_IGESLabelID;
    BoolParm m_IGESLabelName;
    BoolParm m_IGESLabelSurfNo;
    BoolParm m_IGESLabelSplitNo;
    IntParm m_IGESLabelDelim;

    IntParm m_IGESStructureExportIndex;
    BoolParm m_IGESStructureSplitSurfs;
    BoolParm m_IGESStructureToCubic;
    Parm m_IGESStructureToCubicTol;

    BoolParm m_IGESStructureLabelID;
    BoolParm m_IGESStructureLabelName;
    BoolParm m_IGESStructureLabelSurfNo;
    BoolParm m_IGESStructureLabelSplitNo;
    IntParm m_IGESStructureLabelDelim;

    //==== DXF Export ====//
    IntParm m_DXFLenUnit;
    BoolParm m_DXFProjectionFlag;
    Parm m_DXFTessFactor;
    BoolParm m_DXFAllXSecFlag;
    BoolParm m_DXFColorFlag;
    int m_ColorCount;
    IntParm m_DXF2DView;
    IntParm m_DXF2D3DFlag;
    IntParm m_DXF4View1;
    IntParm m_DXF4View2;
    IntParm m_DXF4View3;
    IntParm m_DXF4View4;
    IntParm m_DXF4View1_rot;
    IntParm m_DXF4View2_rot;
    IntParm m_DXF4View3_rot;
    IntParm m_DXF4View4_rot;

    //==== SVG Export ====//
    IntParm m_SVGLenUnit;
    IntParm m_SVGSet;
    Parm m_Scale;
    BoolParm m_SVGProjectionFlag;
    Parm m_SVGTessFactor;
    BoolParm m_SVGAllXSecFlag;
    IntParm m_SVGView;
    IntParm m_SVGView1;
    IntParm m_SVGView2;
    IntParm m_SVGView3;
    IntParm m_SVGView4;
    IntParm m_SVGView1_rot;
    IntParm m_SVGView2_rot;
    IntParm m_SVGView3_rot;
    IntParm m_SVGView4_rot;

    // DesignVarMgr
    IntParm m_WorkingXDDMType;

    // FitModelMgr
    BoolParm m_SelectOneFlag;
    BoolParm m_SelectBoxFlag;
    IntParm m_UType;
    IntParm m_WType;
    Parm m_UTargetPt;
    Parm m_WTargetPt;

    // ProjectionMgr
    IntParm m_TargetType;
    IntParm m_BoundaryType;
    IntParm m_DirectionType;
    Parm m_XComp;
    Parm m_YComp;
    Parm m_ZComp;

    // ManageViewScreen
    IntParm m_ViewportSizeXValue;
    IntParm m_ViewportSizeYValue;
    Parm m_CORXValue;
    Parm m_CORYValue;
    Parm m_CORZValue;
    Parm m_PanXPosValue;
    Parm m_PanYPosValue;
    Parm m_ZoomValue;
    Parm m_XRotationValue;
    Parm m_YRotationValue;
    Parm m_ZRotationValue;

    // ScreenshotScreen
    FractionParm m_NewRatioValue;
    IntParm m_NewWidthValue;
    IntParm m_NewHeightValue;
    BoolParm m_TransparentBGFlag;

    // UserParmScreen
    Parm m_UserParmVal;
    Parm m_UserParmMin;
    Parm m_UserParmMax;

    // ManageBackgroundScreen
    FractionParm m_BGWidthScaleValue;
    FractionParm m_BGHeightScaleValue;
    FractionParm m_BGXOffsetValue;
    FractionParm m_BGYOffsetValue;

    string m_BEMPropID;

    IntParm m_AFExportType;
    Parm m_AFWTessFactor;
    BoolParm m_AFAppendGeomIDFlag;
    string m_AFFileDir;

    BoolParm m_STLMultiSolid;
    BoolParm m_STLExportPropMainSurf;

    BoolParm m_exportCompGeomCsvFile;
    BoolParm m_exportDegenGeomCsvFile;
    BoolParm m_exportDegenGeomMFile;

    Parm m_AxisLength;
    Parm m_TextSize;
    IntParm m_MeasureLenUnit;


    IntParm m_StructUnit;

    BoolParm m_CopySetsWithGeomsFlag;

protected:

    virtual void SetExportPropMainSurf( bool b );

    vector< Geom* > m_GeomStoreVec;                 // All Geom Ptrs

    vector< DegenGeom > m_DegenGeomVec;         // Vector of components in degenerate representation
    vector< DegenPtMass > m_DegenPtMassVec;

    vector < vector < vector < vec3d > > > m_VehProjectVec3d; // Vector of projection lines for each view direction (x, y, or z)

    vector< string > m_ActiveGeom;              // Currently Active Geoms
    vector< string > m_TopGeom;                 // Top (no Parent) Geom IDs
    vector< string > m_ClipBoard;               // Clipboard IDs

    vector< string > m_SetNameVec;

    vector< GeomType > m_GeomTypeVec;

    bool m_UpdatingBBox;
    BndBox m_BBox;                              // Bounding Box Around All Geometries

    void SetApplyAbsIgnoreFlag( const vector< string > &g_vec, bool val );

    //==== Primary file name ====//
    string m_VSP3FileName;

    //==== Export Files ====//
    map< int, string > m_ExportFileNames;

    // File Version Number
    int m_FileOpenVersion;

    string m_ExePath;
    string m_HomePath;
    string m_VSPAEROPath;
    string m_VSPAEROCmd;
    string m_VIEWERCmd;
    string m_SLICERCmd;
    vector< string > m_CustomScriptDirs;

    IntersectSettings m_ISectSettings;

    CfdMeshSettings m_CfdSettings;
    CfdGridDensity m_CfdGridDensity;

    ClippingMgr m_ClippingMgr;
    SnapTo m_SnapTo;

    // Class to handle group transformations
    GroupTransformations m_GroupTransformations;

    // Color of lines in XSecViewScreen
    vec3d m_XSecLineColor;

private:

    void Wype();
};


#endif // !defined(VEHICLE__INCLUDED_)
