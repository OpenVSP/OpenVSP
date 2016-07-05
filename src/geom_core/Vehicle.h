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

#include "Parm.h"
#include "ParmContainer.h"
#include "ParmUndo.h"
#include "Geom.h"
#include "MessageMgr.h"
#include "DrawObj.h"
#include "LightMgr.h"
#include "DegenGeom.h"
#include "CfdMeshSettings.h"
#include "StructSettings.h"
#include "ClippingMgr.h"
#include "SnapTo.h"
#include "STEPutil.h"
#include "XferSurf.h"
#include "MaterialMgr.h"
#include "WaveDragMgr.h"
#include "GroupTransformations.h"

#include <assert.h>

#include <string>
#include <vector>
#include <deque>
#include <stack>
#include <memory>


#define MIN_FILE_VER 4 // Lowest file version number for 3.X vsp file
#define CURRENT_FILE_VER 4 // File version number for 3.X files that this executable writes

/*!
* Centralized place to access all GUI related Parm objects.
*/
class VehicleGuiDraw
{
public:
    /*!
    * Get Lights pointer.
    */
    static LightMgr * getLightMgr()
    {
        return LightMgr::getInstance();
    }
};

//==== Vehicle ====//
class Vehicle : public ParmContainer
{
public:

    Vehicle();
    virtual ~Vehicle();

    void Init();
    void RunTestScripts();
    void Renew();

    void ParmChanged( Parm* parm_ptr, int type );
    void UnDo();

    void Update( bool fullupdate = true );
    void UpdateGeom( const string &geom_id );
    void ForceUpdate();
    void UpdateGui();
    void RunScript( const string & file_name, const string & function_name = "void main()" );

    Geom* FindGeom( const string & geom_id );
    vector< Geom* > FindGeomVec( const vector< string > & geom_id_vec );

    string CreateGeom( const GeomType & type );
    string AddGeom( const GeomType & type );
    string AddGeom( Geom* add_geom );
    string AddMeshGeom( int set );

    virtual void AddLinkableContainers( vector< string > & linkable_container_vec );

    //==== Get All Geoms (Does NOT Return Collapsed Geoms if check_display_flag == true) ====//
    vector< string > GetGeomVec( bool check_display_flag = false );
    vector< Geom* > GetGeomStoreVec()                                { return m_GeomStoreVec; }
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
    void PasteClipboard();
    vector< string > CopyGeomVec( const vector<string> & geom_vec );

    vector< DrawObj* > GetDrawObjs();

    //==== Reset DrawObjs' m_GeomChanged flag to false. ====//
    void ResetDrawObjsGeomChangedFlags();

    //==== Geom Sets ====//
    void SetSetName( int index, const string& name );
    vector< string > GetSetNameVec()                        { return m_SetNameVec; }
    void ShowOnlySet( int index );
    void NoShowSet( int index );
    void ShowSet( int index );
    vector< string > GetGeomSet( int index );

    void HideAllExcept( string id );
    void HideAll();

    //==== Geom Type Data =====//
    vector< string > GetValidTypeGeoms();
    vector< GeomType > GetEditableGeomTypes();
    void AddType( const string & geom_id );
    void DeleteType( int index );

    int GetNumGeomTypes()                                    { return ( int )m_GeomTypeVec.size(); }
    int GetNumFixedGeomTypes();
    GeomType GetGeomType( int index );
    void SetGeomType( int index, GeomType & type );

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

    void SetupPaths();

    string GetExePath()                                     { return m_ExePath; }
    string GetHomePath()                                    { return m_HomePath; }
    string GetVSPAEROCmd()                                  { return m_VSPAEROCmd; }
    string GetVIEWERCmd()                                   { return m_VIEWERCmd; }
    string GetSLICERCmd()                                   { return m_SLICERCmd; }

    //==== Get Script Dir to Write Scripts ====//
    string GetWriteScriptDir()                              { return m_CustomScriptDirs[0]; }
    vector < string > GetCustomScriptDirs()                 { return m_CustomScriptDirs; }

    const VehicleGuiDraw * getVGuiDraw() const
    {
        return &m_VGuiDraw;
    }

    //=== Export Files ===//
    void ExportFile( const string & file_name, int write_set, int file_type );
    bool WriteXMLFile( const string & file_name, int set );
    void WriteXSecFile( const string & file_name, int write_set );
    void WritePLOT3DFile( const string & file_name, int write_set );
    void WriteSTLFile( const string & file_name, int write_set );
    void WriteTaggedMSSTLFile( const string & file_name, int write_set );
    void WriteFacetFile( const string & file_name, int write_set );
    void WriteTRIFile( const string & file_name, int write_set );
    void WriteNascartFiles( const string & file_name, int write_set );
    void WriteGmshFile( const string & file_name, int write_set );
    void WriteX3DFile( const string & file_name, int write_set );
    void WriteX3DMaterial( xmlNodePtr node, Material * material );
    void WriteX3DViewpoints( xmlNodePtr node );
    void WriteX3DViewpointProps( xmlNodePtr node, string orients, string cents, string posits, string sfov, string name );
    void WritePovRayFile( const string & file_name, int write_set );
    void WriteSTEPFile( const string & file_name, int write_set );
    void WriteIGESFile( const string & file_name, int write_set );
    void WriteBEMFile( const string & file_name, int write_set );
    void WriteDXFFile( const string & file_name, int write_set );
    void WriteSVGFile( const string & file_name, int write_set );

    void WriteVehProjectionLinesDXF( FILE * file_name, const BndBox &dxfbox );
    void WriteVehProjectionLinesSVG( xmlNodePtr root, const BndBox &svgbox );

    vector< vector < vec3d > > GetVehProjectionLines( int view, vec3d offset );

    virtual void SetVehProjectVec3d( vector < vector < vec3d > > polyvec, int dir_index )
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
    bool getExportDragBuildTsvFile()                   { return m_exportDragBuildTsvFile(); }
    void setExportCompGeomCsvFile( bool b )            { m_exportCompGeomCsvFile.Set( b ); }
    void setExportDragBuildTsvFile( bool b )           { m_exportDragBuildTsvFile.Set( b ); }

    bool getExportDegenGeomCsvFile( )                  { return m_exportDegenGeomCsvFile(); }
    bool getExportDegenGeomMFile( )                    { return m_exportDegenGeomMFile(); }
    void setExportDegenGeomCsvFile( bool b )           { m_exportDegenGeomCsvFile.Set( b ); }
    void setExportDegenGeomMFile( bool b )             { m_exportDegenGeomMFile.Set( b ); }

    //==== Import Files ====//
    string ImportFile( const string & file_name, int file_type );

    string ImportV2File( const string & file_name );

    //Comp Geom
    string CompGeom( int set, int halfFlag, int intSubsFlag = 1 );
    string CompGeomAndFlatten( int set, int halfFlag, int intSubsFlag = 1 );
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

    CfdMeshSettings* GetCfdSettingsPtr()
    {
        return &m_CfdSettings;
    }

    StructSettings* GetStructSettingsPtr()
    {
        return &m_StructSettings;
    }

    virtual GridDensity* GetCfdGridDensityPtr()
    {
        return &m_CfdGridDensity;
    }

    virtual GridDensity* GetFeaGridDensityPtr()
    {
        return &m_FeaGridDensity;
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

    //==== Mass Properties ====//
    vec3d m_IxxIyyIzz;
    vec3d m_IxyIxzIyz;
    vec3d m_CG;
    int m_NumMassSlices;
    double m_TotalMass;

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

    IntParm m_IGESLenUnit;
    BoolParm m_IGESSplitSurfs;
    BoolParm m_IGESSplitSubSurfs;
    BoolParm m_IGESToCubic;
    Parm m_IGESToCubicTol;
    BoolParm m_IGESTrimTE;

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

    string m_BEMPropID;

    BoolParm m_STLMultiSolid;

    BoolParm m_exportCompGeomCsvFile;
    BoolParm m_exportDragBuildTsvFile;
    BoolParm m_exportDegenGeomCsvFile;
    BoolParm m_exportDegenGeomMFile;

    Parm m_AxisLength;
    Parm m_TextSize;
    IntParm m_MeasureLenUnit;


protected:

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
    string m_VSPAEROCmd;
    string m_VIEWERCmd;
    string m_SLICERCmd;
    vector< string > m_CustomScriptDirs;


    CfdMeshSettings m_CfdSettings;
    StructSettings m_StructSettings;
    CfdGridDensity m_CfdGridDensity;
    FeaGridDensity m_FeaGridDensity;

    ClippingMgr m_ClippingMgr;
    SnapTo m_SnapTo;

    VehicleGuiDraw m_VGuiDraw;

    // Class to handle group transformations
    GroupTransformations m_GroupTransformations;

private:

    void Wype();
};


#endif // !defined(VEHICLE__INCLUDED_)
