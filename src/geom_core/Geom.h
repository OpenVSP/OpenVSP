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

//#include "xmlvsp.h"

#include <string>
#include <vector>
#include <map>

class XSecSurf;

//#define stringify( name ) # name

enum { BASE_GEOM_TYPE, XFORM_GEOM_TYPE, GEOM_GEOM_TYPE, POD_GEOM_TYPE, FUSELAGE_GEOM_TYPE,
       MS_WING_GEOM_TYPE, BLANK_GEOM_TYPE, MESH_GEOM_TYPE, STACK_GEOM_TYPE, CUSTOM_GEOM_TYPE,
       NUM_GEOM_TYPE,
     };

class GeomType
{
public:

    GeomType();
    GeomType( int id, string name, bool fixed_flag = false, string module_name = string()  );
    ~GeomType();

    void CopyFrom( const GeomType & t );

    int m_Type;
    string m_Name;
    bool m_FixedFlag;

    string m_GeomID;
    string m_ModuleName;

};

class Vehicle;

class GeomGuiDraw
{
public:

    GeomGuiDraw();
    virtual ~GeomGuiDraw();

    enum { GEOM_DRAW_WIRE, GEOM_DRAW_HIDDEN, GEOM_DRAW_SHADE, GEOM_DRAW_TEXTURE };

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
    void SetMaterialID( int m )
    {
        m_MaterialID = m;
    }
    virtual int getMaterialID()
    {
        return m_MaterialID;
    }

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
    MaterialMgr * getMaterialMgr()
    {
        return &m_MaterialMgr;
    }
    void SetDispSubSurfFlag( bool f )
    {
        m_DispSubSurfFlag = f;
    }
    bool GetDispSubSurfFlag()
    {
        return m_DispSubSurfFlag;
    }
protected:

    int  m_DrawType;

    bool m_NoShowFlag;
    bool m_DisplayChildrenFlag;
    bool m_DispSubSurfFlag;

    int m_MaterialID;

    TextureMgr m_TextureMgr;
    ColorMgr m_ColorMgr;
    MaterialMgr m_MaterialMgr;
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

    virtual void Update()                                   {}
    virtual void UpdateXForm()                              {}

    virtual void ParmChanged( Parm* parm_ptr, int type );
    virtual void ForceUpdate();

    virtual int CountParents( int count );

    virtual void SetParentID( string id )
    {
        m_ParentID = id ;
    }
    virtual string GetParentID()
    {
        return m_ParentID;
    }
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

    virtual void LoadDrawObjs( vector< DrawObj* > & draw_obj_vec )      {}

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

    virtual void Update();
    virtual void UpdateXForm();
    virtual void ComposeModelMatrix();
    virtual Matrix4d ComposeAttachMatrix();
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

    enum { ATTACH_TRANS_NONE = 0, ATTACH_TRANS_COMP, ATTACH_TRANS_UV, };
    enum { ATTACH_ROT_NONE = 0, ATTACH_ROT_COMP, ATTACH_ROT_UV, }; // Attachment Flags
    enum { ABSOLUTE_XFORM = 0, RELATIVE_XFORM, };

    virtual void DeactivateXForms();
    virtual void SetIgnoreAbsFlag( bool val )
    {
        m_ignoreAbsFlag = val;
    }


    Matrix4d getModelMatrix()
    {
        return m_ModelMatrix;
    }

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
    BoolParm m_relFlag;                 // Bool Parm to determine relative or absolute transformations

    Parm m_Scale;                       // Scaling Parameter

protected:

    bool m_ignoreAbsFlag;
    Matrix4d m_ModelMatrix;
    double m_LastScale;

    //virtual void LoadParmMap();

//  Location
//  Rotation
//  Origin
//  Scale
//  Matrices
//  Attach Data

};

//==== Geom  ====//
class Geom : public GeomXForm
{
public:

    Geom( Vehicle* vehicle_ptr );
    virtual ~Geom();

    virtual void Update();
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
    virtual int GetNumSymFlags();
    virtual int GetNumTotalSurfs();
    virtual int GetNumSymmCopies();

    /*
    * Reset m_GeomChanged flag in DrawObj to false.
    */
    virtual void ResetGeomChangedFlag();

    virtual vec3d GetUWPt( const double &u, const double &w );
    virtual vec3d GetUWPt( const int &indx, const double &u, const double &w );

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

    virtual int GetSymFlag();

    virtual vector< TMesh* > CreateTMeshVec();

    virtual BndBox GetBndBox()
    {
        return m_BBox;
    }

    virtual void WriteXSecFile( int geom_no, FILE* dump_file );
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
    virtual SubSurface* AddSubSurf( int type );
    virtual bool ValidSubSurfInd( int ind );
    virtual void DelSubSurf( int ind );
    virtual SubSurface* GetSubSurf( int ind );
    virtual vector< SubSurface* > GetSubSurfVec()
    {
        return m_SubSurfVec;
    }

    virtual int NumSubSurfs()
    {
        return m_SubSurfVec.size();
    }
    virtual void RecolorSubSurfs( int active_ind );

    //==== Set Drag Factors ====//
    virtual void LoadDragFactors( DragFactors& drag_factors )   {};

    //===== Degenerate Geometry =====//
    virtual void CreateDegenGeom( vector<DegenGeom> &dgs);

    IntParm m_TessU;
    LimIntParm m_TessW;

    IntParm m_SymPlanFlag;
    IntParm m_SymAxFlag;
    IntParm m_SymRotN; // Number Axial Symmetric Objects

    //==== Mass Properties ====//
    IntParm  m_MassPrior;
    Parm     m_Density;
    Parm     m_MassArea;
    BoolParm m_ShellFlag;

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
    virtual void AddCfdMeshSource( BaseSource* source )
    {
        sourceVec.push_back( source );
    }
    virtual vector< BaseSource* > getCfdMeshSourceVec()
    {
        return sourceVec;
    }
    virtual void GetInteriorPnts( vector< vec3d > & pVec )  {}

    //==== Wake for CFD Mesh ====//
    virtual void SetWakeActiveFlag( bool flag )
    {
        m_WakeActiveFlag = flag;
    }
    virtual bool GetWakeActiveFlag()
    {
        return m_WakeActiveFlag;
    }
    virtual void AppendWakeEdges( vector< vector< vec3d > > & edges );
    virtual bool HasWingTypeSurfs();

protected:

    virtual void UpdateSurf() = 0;
    virtual void UpdateSymmAttach();
    virtual void UpdateChildren();
    virtual void UpdateBBox();
    virtual void UpdateDrawObj();

    virtual void UpdateTesselate( int indx, vector< vector< vec3d > > &pnts, vector< vector< vec3d > > &norms );
    virtual void UpdateTesselate( int indx, vector< vector< vec3d > > &pnts, vector< vector< vec3d > > &norms, vector< vector< vec3d > > &uw_pnts );

    vector<VspSurf> m_MainSurfVec;
    vector<VspSurf> m_SurfVec;
    vector<int> m_SurfIndxVec;
    vector<DrawObj> m_WireShadeDrawObj_vec;
    DrawObj m_HighlightDrawObj;

    BndBox m_BBox;

    vector< bool > m_SetFlags;

    vector<SubSurface*> m_SubSurfVec;

//  //==== Structure Parts ====//
//  int currPartID;
//  vector< Part* > partVec;

    //==== CFD Mesh Sources ====//
    int currSourceID;
    vector< BaseSource* > sourceVec;

    //==== Wake for CFD Mesh ====//
    bool m_WakeActiveFlag;
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
