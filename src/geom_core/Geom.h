//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//

// Gsom.h: interface for the GeomBase, GeomXForm, Geom Classes.
// J.R Gloudemans
//
//////////////////////////////////////////////////////////////////////

#if !defined(VSPGEOM__INCLUDED_)
#define VSPGEOM__INCLUDED_

#include "Parm.h"
#include "ParmContainer.h"
#include "Vec3d.h"
#include "Matrix.h"
#include "BndBox.h"
#include "DrawObj.h"
#include "VspSurf.h"
#include "TMesh.h"
#include "DragFactors.h"
#include "SubSurface.h"
#include "GridDensity.h"
#include "ResultsMgr.h"
#include "TextureMgr.h"
#include "ColorMgr.h"
#include "MaterialMgr.h"
#include "DegenGeom.h"
#include "XSec.h"
#include "XSecCurve.h"
#include "XSecSurf.h"
#include "FeaStructure.h"

#include <string>
#include <vector>
#include <map>

class XSecSurf;

// Type matching predominately performed by name matching -- use unique type names when
// adding new Geom types.  However, custom geoms each get a new type name and must be
// identified by matching ( type.m_Type == CUSTOM_GEOM_TYPE ).  Furthermore, this number
// must not change in the future -- i.e. CUSTOM_GEOM_TYPE must always == 9.  Otherwise,
// future users will not be able to open past files with custom geoms.
//
// In general, add any new XXX_GEOM_TYPE to this list before NUM_GEOM_TYPE and everything
// will be OK.  (Don't alphabetize or re-order this list).
//
enum { BASE_GEOM_TYPE, XFORM_GEOM_TYPE, GEOM_GEOM_TYPE, POD_GEOM_TYPE, FUSELAGE_GEOM_TYPE,
       MS_WING_GEOM_TYPE, BLANK_GEOM_TYPE, MESH_GEOM_TYPE, STACK_GEOM_TYPE, CUSTOM_GEOM_TYPE,
       PT_CLOUD_GEOM_TYPE, PROP_GEOM_TYPE, HINGE_GEOM_TYPE, CONFORMAL_GEOM_TYPE,
       ELLIPSOID_GEOM_TYPE, BOR_GEOM_TYPE, WIRE_FRAME_GEOM_TYPE, NUM_GEOM_TYPE
     };

class GeomType
{
public:

    GeomType();
    GeomType( int id, string name, bool fixed_flag = false, string module_name = string(), string display_type = string() );
    ~GeomType();

    void CopyFrom( const GeomType & t );

    int m_Type;
    string m_Name;
    bool m_FixedFlag;

    string m_GeomID;
    string m_ModuleName;
    string m_DisplayName;

};

class Vehicle;

class GeomGuiDraw
{
public:

    GeomGuiDraw();
    virtual ~GeomGuiDraw();

    void SetDisplayType( int t )
    {
        m_DisplayType = t;
    }
    int GetDisplayType()
    {
        return m_DisplayType;
    }

    void SetDrawType( int t )
    {
        m_DrawType = t;
    }
    int  GetDrawType()
    {
        return m_DrawType;
    }

    void SetNoShowFlag( bool f )
    {
        m_NoShowFlag = f;
    }
    bool GetNoShowFlag()
    {
        return m_NoShowFlag;
    }

    void SetWireColor( double r, double g, double b )
    {
        m_ColorMgr.SetWireColor( r, g, b, 255 );
    }

    vec3d GetWireColor()
    {
        Color * wColor = m_ColorMgr.GetWireColor();
        return vec3d( wColor->m_Red.Get(), wColor->m_Green.Get(), wColor->m_Blue.Get() );
    }

    void SetMaterialToDefault();
    void SetMaterial( std::string name, double ambi[], double diff[], double spec[], double emis[], double shin );
    void SetMaterial( const std::string &name );

    void SetDisplayChildrenFlag( bool f )
    {
        m_DisplayChildrenFlag = f;
    }
    void ToggleDisplayChildrenFlag()
    {
        m_DisplayChildrenFlag = !m_DisplayChildrenFlag;
    }
    bool GetDisplayChildrenFlag()
    {
        return m_DisplayChildrenFlag;
    }
    TextureMgr * getTextureMgr()
    {
        return &m_TextureMgr;
    }
    ColorMgr * getColorMgr()
    {
        return &m_ColorMgr;
    }
    Material * getMaterial()
    {
        return &m_Material;
    }
    void SetDispSubSurfFlag( bool f )
    {
        m_DispSubSurfFlag = f;
    }
    bool GetDispSubSurfFlag()
    {
        return m_DispSubSurfFlag;
    }
    void SetDispFeatureFlag( bool f )
    {
        m_DispFeatureFlag = f;
    }
    bool GetDispFeatureFlag()
    {
        return m_DispFeatureFlag;
    }
protected:

    int m_DisplayType; // Either Bezier Surface, Degen Surface, Degen Plate, or Degen Cambered Plate
    int  m_DrawType;

    bool m_NoShowFlag;
    bool m_DisplayChildrenFlag;
    bool m_DispSubSurfFlag;
    bool m_DispFeatureFlag;

    TextureMgr m_TextureMgr;
    ColorMgr m_ColorMgr;
    Material m_Material;
};

//==== Geom Base ====//
class GeomBase : public ParmContainer
{
public:
    GeomBase( Vehicle* vehicle_ptr );           // Default Constructor
    virtual ~GeomBase();                        // Destructor

    virtual GeomType GetType()
    {
        return m_Type;
    }
    virtual void SetType( const GeomType & type )
    {
        m_Type = type;
    }

    virtual void Update( bool fullupdate = true )           {}

    virtual void ParmChanged( Parm* parm_ptr, int type );
    virtual void ForceUpdate();

    virtual int CountParents( int count );
    virtual bool IsParentJoint();

    virtual void SetParentID( string id )
    {
        m_ParentID = id ;
    }
    virtual string GetParentID()
    {
        return m_ParentID;
    }
    virtual string GetAncestorID( int gen );
    virtual void BuildAncestorList( vector< string > &ancestors );
    virtual void AddChildID( string id )
    {
        m_ChildIDVec.push_back( id );
    }
    virtual void RemoveChildID( string id );
    virtual vector< string > GetChildIDVec()
    {
        return m_ChildIDVec;
    }
    virtual void SetChildIDVec( vector< string > & vec )
    {
        m_ChildIDVec = vec;
    }

    virtual bool UpdatedParm( const string & id );

    virtual void LoadIDAndChildren( vector< string > & id_vec, bool check_display_flag = false );

    virtual bool IsMatch( const string& id_string );

    virtual xmlNodePtr EncodeXml( xmlNodePtr & node );
    virtual xmlNodePtr DecodeXml( xmlNodePtr & node );

    GeomGuiDraw m_GuiDraw;

protected:

    Vehicle* m_Vehicle;

    GeomType m_Type;

    string m_ParentID;                                  // ID of Parent (NONE == No Parent)
    vector< string > m_ChildIDVec;                      // Children ID

    vector< string > m_UpdatedParmVec;
};

//==== Geom XForm ====//
class GeomXForm : public GeomBase
{
public:
    GeomXForm( Vehicle* vehicle_ptr );
    virtual ~GeomXForm();

    virtual void Update( bool fullupdate = true );
    virtual void ComposeModelMatrix();
    virtual Matrix4d ComposeAttachMatrix();
    virtual void SetCenter( double x, double y, double z )      { m_Center.set_xyz( x, y, z ); }
    virtual void ComputeCenter()
    {
        m_Center.set_xyz( 0.0, 0.0, 0.0 );
    }
    virtual void ResetScale();
    virtual void AcceptScale();
    virtual void Scale()
    {
        m_Scale = 1;
    }

    virtual void DeactivateXForms();
    virtual void SetIgnoreAbsFlag( bool val )
    {
        m_ignoreAbsFlag = val;
    }
    virtual void SetApplyAbsIgnoreFlag(bool val)
    {
        m_applyIgnoreAbsFlag = val;
    }


    Matrix4d getModelMatrix()
    {
        return m_ModelMatrix;
    }

    virtual Matrix4d GetAncestorAttachMatrix( int gen );
    virtual Matrix4d GetAncestorModelMatrix( int gen );

    Parm m_XLoc;                // XForm Translation
    Parm m_YLoc;
    Parm m_ZLoc;

    Parm m_XRelLoc;
    Parm m_YRelLoc;
    Parm m_ZRelLoc;

    Parm m_XRot;                // XForm Rotation
    Parm m_YRot;
    Parm m_ZRot;

    Parm m_XRelRot;
    Parm m_YRelRot;
    Parm m_ZRelRot;

    Parm m_Origin;                      // X-rotation origin
    vec3d m_Center;                     // Rotation center

    IntParm m_AbsRelFlag;
    IntParm m_TransAttachFlag;          // Flag to set translations attachment
    IntParm m_RotAttachFlag;            // Flag to set rotation attachment

    Parm m_ULoc;                        // UV Attachment Parameters
    Parm m_WLoc;

    Parm m_Scale;                       // Scaling Parameter
    Parm m_LastScale;

    vec3d m_AttachOrigin;
    vector < vec3d > m_AttachAxis;

protected:

    bool m_ignoreAbsFlag;
    bool m_applyIgnoreAbsFlag;          // Controls whether the ignoreAbsFlag is obeyed
    Matrix4d m_ModelMatrix;

};

//==== Geom  ====//
class Geom : public GeomXForm
{
public:

    Geom( Vehicle* vehicle_ptr );
    virtual ~Geom();

    virtual void Update( bool fullupdate = true );
    virtual void LoadMainDrawObjs( vector< DrawObj* > & draw_obj_vec );
    virtual void LoadDrawObjs( vector< DrawObj* > & draw_obj_vec );

    virtual void SetColor( int r, int g, int b );
    virtual vec3d GetColor();

    virtual void SetMaterialToDefault();
    virtual void SetMaterial( std::string name, double ambi[], double diff[], double spec[], double emis[], double shin );
    virtual Material * GetMaterial();

    virtual bool GetSetFlag( int index );
    virtual vector< bool > GetSetFlags()
    {
        return m_SetFlags;
    }
    virtual void SetSetFlag( int index, bool f );
    virtual void UpdateSets();

    virtual VspSurf* GetSurfPtr();
    virtual VspSurf* GetSurfPtr( int indx );
    virtual void GetSurfVec( vector<VspSurf> &surf_vec )
    {
        surf_vec = m_SurfVec;
    }
    virtual int GetNumMainSurfs()
    {
        return m_MainSurfVec.size();
    }
    virtual void GetMainSurfVec( vector<VspSurf> &surf_vec )    { surf_vec = m_MainSurfVec; }
    virtual int GetNumSymFlags();
    virtual int GetNumTotalSurfs();
    virtual int GetNumTotalHrmSurfs();
    virtual int GetNumSymmCopies();

    virtual vector < int > & GetSymmIndexs( int imain )
    {
        return m_SurfSymmMap[ imain ];
    }

    virtual int GetMainSurfID( int surfnum )
    {
        if ( surfnum >= 0 && surfnum < m_SurfIndxVec.size() )
        {
            return m_SurfIndxVec[ surfnum ];
        }
        else
        {
            return -1;
        }
    }

    /*
    * Reset m_GeomChanged flag in DrawObj to false.
    */
    virtual void ResetGeomChangedFlag();

    virtual vec3d CompPnt01(const double &u, const double &w);
    virtual vec3d CompPnt01(const int &indx, const double &u, const double &w);
    virtual void GetUWTess01( int indx, vector < double > &u, vector < double > &w );

    virtual bool CompRotCoordSys( const double &u, const double &w, Matrix4d &rotmat );
    virtual bool CompTransCoordSys( const double &u, const double &w, Matrix4d &transmat );

    virtual vector < Matrix4d > GetFeaTransMatVec()
    {
        return m_FeaTransMatVec;
    }

    //==== XSec Surfs ====//
    virtual int GetNumXSecSurfs()
    {
        return 0;
    }
    virtual XSecSurf* GetXSecSurf( int index )
    {
        return NULL;
    }

    virtual void CopyFrom( Geom* geom );

    virtual xmlNodePtr EncodeXml( xmlNodePtr & node );
    virtual xmlNodePtr DecodeXml( xmlNodePtr & node );

    virtual xmlNodePtr EncodeGeom( xmlNodePtr & node );
    virtual xmlNodePtr DecodeGeom( xmlNodePtr & node );

    virtual void ReadV2File( xmlNodePtr &root );

    virtual int GetSymFlag();

    virtual vector< TMesh* > CreateTMeshVec();

    virtual BndBox GetBndBox()
    {
        return m_BBox;
    }

    virtual void WriteAirfoilFiles( FILE* meta_fid );
    virtual void WriteBezierAirfoil( const string & file_name, double foilsurf_u_location );
    virtual void WriteSeligAirfoil( const string & file_name, double foilsurf_u_location );
    virtual vector < vec3d > GetAirfoilCoordinates( double foilsurf_u_location );

    virtual void WriteXSecFile( int geom_no, FILE* dump_file );
    virtual void WritePLOT3DFileExtents( FILE* dump_file );
    virtual void WritePLOT3DFileXYZ( FILE* dump_file );

    virtual void SetupPMARCFile( int &ipatch, vector < int > &idpat );
    virtual void WritePMARCGeomFile(FILE *dump_file, int &ipatch, vector<int> &idpat);
    virtual void WritePMARCWakeFile(FILE *dump_file, int &ipatch, vector<int> &idpat);
    virtual void WriteStl( FILE* fid ) {};
    virtual void WriteX3D( xmlNodePtr node );
    virtual void WritePovRay( FILE* fid, int comp_num );
    virtual void WritePovRayTri( FILE* fid, const vec3d& v, const vec3d& n, bool comma = true );
    virtual void CreateGeomResults( Results* res );

    virtual void AddLinkableParms( vector< string > & linkable_parm_vec, const string & link_container_id = string() );
    virtual void ChangeID( string id );

    //==== Sub Surface Managment Methods ====//
    virtual void AddSubSurf( SubSurface* sub_surf )
    {
        m_SubSurfVec.push_back( sub_surf );
    }
    virtual SubSurface* AddSubSurf( int type, int surfindex );
    virtual bool ValidSubSurfInd( int ind );
    virtual void DelSubSurf( int ind );
    virtual SubSurface* GetSubSurf( int ind );
    virtual SubSurface* GetSubSurf( const string & id );
    virtual int GetSubSurfIndex( const string & id );
    virtual vector< SubSurface* > GetSubSurfVec()
    {
        return m_SubSurfVec;
    }

    virtual int NumSubSurfs()
    {
        return m_SubSurfVec.size();
    }
    virtual void RecolorSubSurfs( int active_ind );

    //==== FeaStructure Data =====//
    vector < FeaStructure* > GetFeaStructVec()
    {
        return m_FeaStructVec;
    }
    virtual FeaStructure* AddFeaStruct( bool initskin, int surf_index );
    virtual FeaStructure* GetFeaStruct( int fea_struct_ind );
    virtual int GetFeaStructIndex( const string & structure_id );
    virtual void DeleteFeaStruct( int index );
    virtual bool ValidGeomFeaStructInd( int index );
    virtual int NumGeomFeaStructs()
    {
        return m_FeaStructVec.size();
    }

    //==== Set Drag Factors ====//
    virtual void LoadDragFactors( DragFactors& drag_factors )   {};

    //===== Degenerate Geometry =====//
    virtual void CreateDegenGeom( vector<DegenGeom> &dgs, bool preview = false );
    virtual void CreateDegenGeom( vector<DegenGeom> &dgs, const vector< vector< vec3d > > &pnts, const vector< vector< vec3d > > &nrms, const vector< vector< vec3d > > &uwpnts,
                                  bool urootcap, int isurf, bool preview, bool flipnormal, int surftype, VspSurf *fs );

    IntParm m_TessU;
    LimIntParm m_TessW;

    IntParm m_SymAncestor;
    BoolParm m_SymAncestOriginFlag;
    IntParm m_SymPlanFlag;
    IntParm m_SymAxFlag;
    IntParm m_SymRotN; // Number Axial Symmetric Objects

    //==== Mass Properties ====//
    IntParm  m_MassPrior;
    Parm     m_Density;
    Parm     m_MassArea;
    BoolParm m_ShellFlag;

    //==== Negative Volume Properties (GUI Purposes) ====//
    BoolParm m_NegativeVolumeFlag;

    Parm m_BbXLen;
    Parm m_BbYLen;
    Parm m_BbZLen;
    Parm m_BbXMin;
    Parm m_BbYMin;
    Parm m_BbZMin;

//XSec Surfs
//Bounding Boxes
//Color
//Material
//Display Flags
//OtherData

    virtual void SetGeomProjectVec3d( vector < vector < vec3d > > polyvec, int dir_index )
    {
        m_GeomProjectVec3d[dir_index] = polyvec;
    }
    virtual vector < vector < vec3d > > GetGeomProjectVec3d( int dir_index )
    {
        return m_GeomProjectVec3d[dir_index];
    }
    virtual void ClearGeomProjectVec3d()
    {
        m_GeomProjectVec3d.clear();
        m_GeomProjectVec3d.resize( 3 );
    }

    virtual void SetForceXSecFlag( bool flag )
    {
        m_ForceXSecFlag = flag;
    }
    virtual bool GetForceXSecFlag( )
    {
        return m_ForceXSecFlag;
    }

//  //==== Structures ====//
//  void SetCurrPartID( int pid )           { currPartID = pid; }
//  int  GetCurrPartID()                    { return currPartID; }
//  void DelCurrPart();
//  void AddStructurePart( Part* part )     { partVec.push_back( part ); }
//  vector< Part* > getStructurePartVec()   { return partVec; }

    //==== Sources for CFD Mesh ====//
    virtual void AddDefaultSources( double base_len = 1.0 )                       {}
    virtual void SetCurrSourceID( int sid )
    {
        currSourceID = sid;
    }
    virtual int  GetCurrSourceID()
    {
        return currSourceID;
    }
    virtual void DelCurrSource();
    virtual void DelAllSources();
    virtual void UpdateSources();
    virtual BaseSource* CreateSource( int type );
    virtual BaseSimpleSource* CreateSimpleSource( int type );
    virtual void AddCfdMeshSource( BaseSource* source )
    {
        m_MainSourceVec.push_back( source );
    }
    virtual vector< BaseSource* > GetCfdMeshMainSourceVec()
    {
        return m_MainSourceVec;
    }
    virtual vector< BaseSimpleSource* > GetCfdMeshSimpSourceVec()
    {
        return m_SimpSourceVec;
    }
    virtual void GetInteriorPnts( vector< vec3d > & pVec )  {}

    //==== Wake for CFD Mesh ====//
    virtual void SetWakeActiveFlag( bool flag )
    {
        m_WakeActiveFlag.Set( flag );
    }
    virtual void AppendWakeEdges( vector< vector< vec3d > > & edges );
    virtual bool HasWingTypeSurfs();

    //=== End Cap Parms ===//
    IntParm m_CapUMinOption;
    LimIntParm m_CapUMinTess;
    IntParm m_CapUMaxOption;
    IntParm m_CapWMinOption;
    IntParm m_CapWMaxOption;

    Parm m_CapUMinLength;
    Parm m_CapUMinOffset;
    Parm m_CapUMinStrength;
    BoolParm m_CapUMinSweepFlag;

    Parm m_CapUMaxLength;
    Parm m_CapUMaxOffset;
    Parm m_CapUMaxStrength;
    BoolParm m_CapUMaxSweepFlag;
    bool m_CappingDone;

    //==== Wake for CFD Mesh ====//
    BoolParm m_WakeActiveFlag;

    //==== Parasite Drag Parms ====//
    IntParm m_FFBodyEqnType;
    IntParm m_FFWingEqnType;
    Parm m_PercLam;
    Parm m_FFUser;
    Parm m_Q;
    Parm m_Roughness;
    Parm m_TeTwRatio;
    Parm m_TawTwRatio;
    IntParm m_GroupedAncestorGen;
    BoolParm m_ExpandedListFlag;

    //==== Basic Geom XSec Functions - Override ====//
    virtual void CutXSec( int index )               {}
    virtual void CopyXSec( int index )              {}
    virtual void PasteXSec( int index )             {}
    virtual void InsertXSec( int index, int type )  {}

    void WriteFeatureLinesDXF( FILE * file_name, const BndBox &dxfbox );
    void WriteProjectionLinesDXF( FILE * file_name, const BndBox &dxfbox );
    vector < vector< vec3d > > GetGeomProjectionLines( int view, vec3d offset );
    void WriteFeatureLinesSVG( xmlNodePtr root, const BndBox &dxfbox );
    void WriteProjectionLinesSVG( xmlNodePtr root, const BndBox &dxfbox );

    virtual void OffsetXSecs( double off )          {}

    virtual void UpdateDegenDrawObj();

    virtual void ExportSurfacePatches( vector< string > &surf_res_ids );

protected:

    bool m_UpdateBlock;

    virtual void UpdateSurf() = 0;
    void UpdateEndCaps();
    virtual void UpdateFeatureLines();
    virtual void UpdateFlags();
    virtual void UpdateSymmAttach();
    virtual void UpdateChildren( bool fullupdate );
    virtual void UpdateBBox();
    virtual void UpdateDrawObj();

    virtual void UpdateTesselate( int indx, vector< vector< vec3d > > &pnts, vector< vector< vec3d > > &norms, bool degen );
    virtual void UpdateTesselate( int indx, vector< vector< vec3d > > &pnts, vector< vector< vec3d > > &norms, vector< vector< vec3d > > &uw_pnts, bool degen );

    virtual void UpdateSplitTesselate( int indx, vector< vector< vector< vec3d > > > &pnts, vector< vector< vector< vec3d > > > &norms );

    virtual void CalcTexCoords( int indx, vector< vector< vector< double > > > &utex, vector< vector< vector< double > > > &vtex, const vector< vector< vector< vec3d > > > & pnts );

    vector<VspSurf> m_MainSurfVec;
    vector<VspSurf> m_SurfVec;
    vector<int> m_SurfIndxVec;
    vector< vector< int > > m_SurfSymmMap;
    vector<int> m_SurfCopyIndx;
    vector< Matrix4d > m_TransMatVec; // Vector of transformation matrixes
    vector< Matrix4d > m_FeaTransMatVec; // Vector of transformation matrixes
    vector<DrawObj> m_WireShadeDrawObj_vec;
    vector<DrawObj> m_FeatureDrawObj_vec;
    DrawObj m_HighlightDrawObj;
    vector<DrawObj> m_AxisDrawObj_vec;
    vector<DrawObj> m_DegenPlateDrawObj_vec;
    vector<DrawObj> m_DegenSurfDrawObj_vec;
    vector<DrawObj> m_DegenCamberPlateDrawObj_vec;
    vector<DrawObj> m_DegenSubSurfDrawObj_vec;

    BndBox m_BBox;

    vector< bool > m_SetFlags;

    vector<SubSurface*> m_SubSurfVec;

    vector < vector < vector < vec3d > > > m_GeomProjectVec3d; // Vector of projection lines for each view direction (x, y, or z)
    bool m_ForceXSecFlag; // Flag to force feature lines at xsecs

//  //==== Structure Parts ====//

    vector< FeaStructure* > m_FeaStructVec;     // Vector of FeaStructures for this Geom
    int m_FeaStructCount; // Counter used for creating unique name for structures

    //==== CFD Mesh Sources ====//
    int currSourceID;
    vector< BaseSource* > m_MainSourceVec;
    vector< BaseSimpleSource* > m_SimpSourceVec;

    vector< bool > m_CapUMinSuccess;
    vector< bool > m_CapUMaxSuccess;
    vector< bool > m_CapWMinSuccess;
    vector< bool > m_CapWMaxSuccess;

    enum { V2_NO_SYM = 0,
           V2_XY_SYM = 1,
           V2_XZ_SYM = 2,
           V2_YZ_SYM = 3
    };

    enum { V2_POS_ATTACH_NONE = 0,
           V2_POS_ATTACH_FIXED = 1,
           V2_POS_ATTACH_UV = 2,
           V2_POS_ATTACH_MATRIX = 3,
    };
};

//==== GeomXSec  ====//
class GeomXSec : public Geom
{
public:
    GeomXSec( Vehicle* vehicle_ptr );
    virtual ~GeomXSec();

    virtual XSecSurf* GetXSecSurf( int index )
    {
        return &m_XSecSurf;
    }

    virtual void LoadDrawObjs( vector< DrawObj* > & draw_obj_vec );

    virtual int GetActiveXSecIndex()
    {
        return m_ActiveXSec;
    }
    virtual void SetActiveXSecIndex( int index );

    virtual XSec* GetXSec( int index );
    virtual void AddDefaultSourcesXSec( double base_len, double len_ref, int ixsec );

    virtual void OffsetXSecs( double off );

protected:
    virtual void UpdateDrawObj();

    XSecSurf m_XSecSurf;
    vector<DrawObj> m_XSecDrawObj_vec;
    DrawObj m_HighlightXSecDrawObj;

    DrawObj m_CurrentXSecDrawObj;

    int m_ActiveXSec;
    int m_MinActiveXSec;

};
#endif // !defined(VSPGEOM__INCLUDED_)
