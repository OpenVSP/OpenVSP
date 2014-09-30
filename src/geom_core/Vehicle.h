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
#include "LabelMgr.h"
#include "LightMgr.h"
#include "DegenGeom.h"
#include "CfdMeshSettings.h"
#include "ClippingMgr.h"
#include "STEPutil.h"
#include "XferSurf.h"

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
    * Get Labels pointer.
    */
    static LabelMgr * getLabelMgr()
    {
        return LabelMgr::getInstance();
    }

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

    void Update();
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
    void CopyActiveGeomVec();
    void CutGeomVec( const vector<string> & cut_vec );
    void DeleteClipBoard();
    void PasteClipboard();

    vector< DrawObj* > GetDrawObjs();

    //==== Reset DrawObjs' m_GeomChanged flag to false. ====//
    void ResetDrawObjsGeomChangedFlags();

    int GetFileOpenVersion()                                { return m_FileOpenVersion; }

    //==== Geom Sets ====//
    void SetSetName( int index, const string& name );
    vector< string > GetSetNameVec()                        { return m_SetNameVec; }
    void SetShowSet( int index );
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

    xmlNodePtr EncodeXml( xmlNodePtr & node, int set );
    xmlNodePtr DecodeXml( xmlNodePtr & node );

    enum { REORDER_MOVE_UP, REORDER_MOVE_DOWN, REORDER_MOVE_TOP, REORDER_MOVE_BOTTOM };

    bool ExistMesh( int set );

    int ReadXMLFile( const string & file_name );

    void SetVSP3FileName( const string & f_name )           { m_VSP3FileName = f_name; }
    string GetVSP3FileName()                                { return m_VSP3FileName; }

    const VehicleGuiDraw * getVGuiDraw() const
    {
        return &m_VGuiDraw;
    }

    //=== Export Files ===//
    void ExportFile( const string & file_name, int write_set, int file_type );
    bool WriteXMLFile( const string & file_name, int set );
    void WriteXSecFile( const string & file_name, int write_set );
    void WriteSTLFile( const string & file_name, int write_set );
    void WriteTRIFile( const string & file_name, int write_set );
    void WriteNascartFiles( const string & file_name, int write_set );
    void WriteGmshFile( const string & file_name, int write_set );
    void WriteX3DFile( const string & file_name, int write_set );
    void WriteX3DMaterial( xmlNodePtr node, int matid );
    void WriteX3DViewpoints( xmlNodePtr node );
    void WriteX3DViewpointProps( xmlNodePtr node, string orients, string cents, string posits, string sfov, string name );
    void WritePovRayFile( const string & file_name, int write_set );
    void WriteSTEPFile( const string & file_name, int write_set );

    void FetchXFerSurfs( int write_set, vector< XferSurf > &xfersurfs );
    //==== Computation File Names ====//
    string getExportFileName( int type );
    void setExportFileName( int type, string f_name );
    bool getExportCompGeomCsvFile()                    { return m_exportCompGeomCsvFile; }
    bool getExportDragBuildTsvFile()                { return m_exportDragBuildTsvFile; }
    void setExportCompGeomCsvFile( bool b )            { m_exportCompGeomCsvFile = b; }
    void setExportDragBuildTsvFile( bool b )        { m_exportDragBuildTsvFile = b; }

    bool getExportDegenGeomCsvFile( )
    {
        return m_exportDegenGeomCsvFile;
    }
    bool getExportDegenGeomMFile( )
    {
        return m_exportDegenGeomMFile;
    }
    void setExportDegenGeomCsvFile( bool b )
    {
        m_exportDegenGeomCsvFile = b;
    }
    void setExportDegenGeomMFile( bool b )
    {
        m_exportDegenGeomMFile = b;
    }

    //==== Import Files ====//
    string ImportFile( const string & file_name, int file_type );

    //Comp Geom
    string CompGeom( int set, int sliceFlag, int meshFlag, int halfFlag, int intSubsFlag = 1 );
    string CompGeomAndFlatten( int set, int sliceFlag, int meshFlag, int halfFlag, int intSubsFlag = 1 );
    string MassProps( int set, int numSlices );
    string MassPropsAndFlatten( int set, int numSlices );
    string AwaveSlice( int set, int numSlices, int numRots, double AngleControlVal, bool computeAngle,
                       vec3d norm, bool autoBoundsFlag, double start = 0, double end = 0 );
    string AwaveSliceAndFlatten( int set, int numSlices, int numRots, double AngleControlVal, bool computeAngle,
                                 vec3d norm, bool autoBoundsFlag, double start = 0, double end = 0 );
    string PSlice( int set, int numSlices, vec3d norm, bool autoBoundsFlag, double start = 0, double end = 0 );
    string PSliceAndFlatten( int set, int numSlices, vec3d norm, bool autoBoundsFlag, double start = 0, double end = 0 );

    //==== Degenerate Geometry ====//
    void CreateDegenGeom( int set );
    vector< DegenGeom > GetDegenGeomVec()	{ return m_DegenGeomVec; }
    string WriteDegenGeomFile();

    CfdMeshSettings* GetCfdSettingsPtr()
    {
        return &m_CfdSettings;
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

    BoolParm m_STEPSplitSurfs;
    BoolParm m_STEPMergePoints;
    BoolParm m_STEPToCubic;
    Parm m_STEPTolerance;

protected:

    vector< Geom* > m_GeomStoreVec;                 // All Geom Ptrs

    vector< DegenGeom > m_DegenGeomVec;         // Vector of components in degenerate representation
    vector< DegenPtMass > m_DegenPtMassVec;


    vector< string > m_ActiveGeom;              // Currently Active Geoms
    vector< string > m_TopGeom;                 // Top (no Parent) Geom IDs
    vector< string > m_ClipBoard;               // Clipboard IDs

    stack< ParmUndo > m_ParmUndoStack;

    vector< string > m_SetNameVec;

    vector< GeomType > m_GeomTypeVec;

    BndBox m_BBox;                              // Bounding Box Around All Geometries

    vector< string > CopyGeomVec( const vector<string> & geom_vec );
    void InsertIntoActiveDeque( const string & add_id, const string & parent_id );  // Insert Geom After Parent

    //==== Primary file name ====//
    string m_VSP3FileName;

    //==== Export Files ====//
    string m_compGeomTxtFileName;
    string m_compGeomCsvFileName;
    string m_compGeomTsvFileName;
    string m_MassPropFileName;
    string m_AwaveFileName;
    string m_degenGeomCsvFileName;
    string m_degenGeomMFileName;
    bool m_exportCompGeomCsvFile;
    bool m_exportDragBuildTsvFile;
    bool m_exportDegenGeomCsvFile;
    bool m_exportDegenGeomMFile;

    void DeleteGeom( const string & geom_id );

    // File Version Number
    int m_FileOpenVersion;

    CfdMeshSettings m_CfdSettings;
    CfdGridDensity m_CfdGridDensity;
    FeaGridDensity m_FeaGridDensity;

    ClippingMgr m_ClippingMgr;

    VehicleGuiDraw m_VGuiDraw;

private:

    void Wype();
};


#endif // !defined(VEHICLE__INCLUDED_)
