//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//

// geom.h: interface for the geom class.
// J.R. Gloudemans
//////////////////////////////////////////////////////////////////////


#if !defined(AFX_GEOM_H__D3CD0543_7430_11D7_AC31_0003473A025A__INCLUDED_)
#define AFX_GEOM_H__D3CD0543_7430_11D7_AC31_0003473A025A__INCLUDED_

#ifdef WIN32
#pragma warning(disable : 4786)
#endif

#include "vec2d.h"
#include "vec3d.h"
#include "parm.h"
#include "bezier_curve.h"
#include "xsec_surf.h"
#include "herm_curve.h"
#include "bbox.h"
#include "xmlvsp.h"
#include "matrix.h"
#include "textureMgr.h"
#include "GridDensity.h"
#include "DragFactors.h"


class TMesh;
class TetraMassProp;
class Part;
class BaseSource;

#define NO_SYM 0
#define XY_SYM 1
#define XZ_SYM 2
#define YZ_SYM 3

#define POS_ATTACH_NONE		0
#define POS_ATTACH_FIXED	1
#define POS_ATTACH_UV		2 
#define POS_ATTACH_MATRIX	3

#define GEOM_WIRE_FLAG		0
#define GEOM_HIDDEN_FLAG	1
#define GEOM_SHADE_FLAG		2
#define GEOM_TEXTURE_FLAG	3

//==== Update Tags ====//
#define UPD_XFORM			101010			// General Update Tags	
#define UPD_NUM_PNT_XSEC	(UPD_XFORM+1)
#define UPD_POD				(UPD_XFORM+2)	// Pod Update Tags
#define UPD_MW_ALL          (UPD_XFORM+3)	// Wing Update Tags
#define UPD_MW_LOC_ROT      (UPD_XFORM+4)
#define UPD_MW_NUM_PNTS     (UPD_XFORM+5)
#define UPD_MW_AIRFOIL      (UPD_XFORM+6)
#define UPD_MW_CONT_SURF    (UPD_XFORM+7)
#define UPD_MW_SWEEP_LOC    (UPD_XFORM+8)
#define UPD_FUSE_XSEC		(UPD_XFORM+9)	// Fuse Update Tags
#define UPD_FUSE_BODY		(UPD_XFORM+10)
#define UPD_HAVOC			(UPD_XFORM+11)	// Havoc
#define UPD_EDIT_CRV		(UPD_XFORM+12)	// Edit Curve
			

class Aircraft;
class ON_BinaryFile;
class ON_3dmObjectAttributes;

static int geomCnt = 0;

class VertexID;

class VSPDLL GeomBase
{
public:
	GeomBase();
	virtual ~GeomBase()				{}

	virtual void setName( Stringc nameIn )		{ name_str = nameIn; }
	virtual Stringc getName()					{ return name_str; }
	virtual int getType()						{ return type; }
	virtual Stringc getTypeStr()				{ return type_str; }
	virtual void setColor( double r, double g, double b) { color.set_xyz( r, g, b ); }
	virtual vec3d getColor()					{ return color; }

	virtual void parm_changed(Parm*) = 0;


protected:
   int type;
   Stringc type_str;
   Stringc name_str;

   //==== Color ====//
   vec3d color;


};

class VSPDLL Geom : public GeomBase 
{
public:

	enum { NO_FLAG, RED_FLAG, YELLOW_FLAG };

	Geom( Aircraft* aptr );
	virtual ~Geom();

	virtual void write(xmlNodePtr node) = 0;
 	virtual void write_general_parms(xmlNodePtr node);
	virtual void read(xmlNodePtr node) = 0;
	virtual void read_general_parms(xmlNodePtr node);

	virtual void read(FILE* file_id)							{}
    virtual void read_general_parms(FILE* file_id);

	virtual void setColor( double r, double g, double b);

	virtual Stringc getName();


	virtual void copy( Geom* fromGeom );
	virtual void drawAlpha();
	virtual void parm_changed(Parm*) = 0;
	virtual void update_bbox() = 0;
	virtual void update_xformed_bbox();
	virtual void draw_bbox();
	virtual int  valid_bnd_box()				{ return 1; }
	virtual bbox get_bnd_box()					{ return bnd_box_xform; }
	virtual void draw_highlight_boxes();
	virtual void deactivate();

	virtual void draw();
	virtual void triggerDraw();

	virtual void setRedFlag(int flag)			{ redFlag = flag; }
	virtual void setYellowFlag(int flag)		{ yellowFlag = flag; }
											
	virtual void setNoShowFlag(int flag);
	virtual int  getNoShowFlag()				{ return noshowFlag; }

	virtual void setSymCode( int in_sym );
	virtual int getSymCode()					{ return sym_code; }
	virtual vec3d getSymVec()					{ return sym_vec; }

	virtual int getIdNum()						{ return id_num; }
	virtual Stringc getIdStr()					{ return id_str; }

	virtual void setMaterialID( int m );
	virtual int getMaterialID()					{ return materialID; }

	virtual void setDisplayFlag( int flag )		{ displayFlag = flag; }
	virtual int  getDisplayFlag()				{ return displayFlag; }

	virtual void draw2D(vec2d & cursor)			{}
	virtual void mouseClick(vec2d & cursor)		{} 

	virtual void addChild( Geom* geomPtr );	
	virtual void removeChild( Geom* geomPtr );

	virtual void setParent( Geom* geomPtr );
	virtual vec3d getTotalTranVec();
	virtual vec3d getTotalRotVec();

	virtual vec3d getAttachFixedPos();
	virtual vec3d getAttachUVPos(double u, double v);				// Fixed or UV

	virtual void updateAttach(int absUpdateFlag);

	virtual void setPosAttachFlag( int flag );
	virtual int  getPosAttachFlag()				{ return posAttachFlag; }

	virtual float* getMat()						{ return matFVec; }
	virtual float* getReflMat()					{ return matReflFVec; }

	virtual Geom* getParent()					{ return parentGeom; }
	virtual vector< Geom* > getChildren()		{ return childGeomVec; }
	virtual void setChildren( vector< Geom* > & gVec )	{ childGeomVec = gVec; }

	virtual void setDisplayChildrenFlag( int f )	{ displayChildrenFlag = f; }
	virtual void toggleDisplayChildrenFlag()		{ displayChildrenFlag = !displayChildrenFlag; }
	virtual int getDisplayChildrenFlag()			{ return displayChildrenFlag; }

	virtual void loadDisplayChildren( vector< Geom* > & gVec );

	virtual Aircraft* get_aircraft_ptr()		{ return airPtr; }

	virtual void loadChildren( vector< Geom* >& gVec );

	virtual int countParents(int count);				

    virtual void compose_model_matrix();
    virtual void compose_reflect_matrix();

	virtual void computeCenter();

	virtual void acceptScaleFactor() = 0;
	virtual void resetScaleFactor() = 0;

	virtual void fastDrawFlagOn()				{ fastDrawFlag = 1; }
	virtual void fastDrawFlagOff()				{ fastDrawFlag = 0; }

	virtual void setRelXFormFlag( int f );
	virtual int  getRelXFormFlag()				{ return relXFormFlag; }

	virtual double get_tran_x();			// Total Offset Including Parents
	virtual double get_tran_y();
	virtual double get_tran_z();

	virtual double get_rot_x();				// Total Rotation Including Parents
	virtual double get_rot_y();
	virtual double get_rot_z();

	virtual void setPtrID( int id )				{ ptrID = id; }
	virtual int getPtrID()						{ return ptrID; }

	virtual void setMassPrior( int val )		{ massPrior = val; }
	virtual int  getMassPrior()					{ return massPrior; }
	virtual void setShellFlag( int f )			{ shellFlag = f; }
	virtual int  getShellFlag()					{ return shellFlag; }

	virtual int getParentPtrID()				{ return parentPtrID; }
	virtual vector<int> & getChildPtrIDVec()	{ return childPtrIDVec; }

	//==== File IO ====//
	virtual void setOutputFlag( int f )			{ outputFlag = f; }
	virtual int  getOutputFlag()				{ return outputFlag; }
	virtual void setOutputNameID( int m )		{ outputNameID = m; }
	virtual int  getOutputNameID()				{ return outputNameID; }

	virtual void write_pov_tris(FILE* pov_file);
	virtual void write_stl_file(FILE* pov_file);
	virtual void write_nascart_file(FILE* pov_file)			{}
	virtual void write_rhino_file(ON_BinaryFile* archive, ON_3dmObjectAttributes* attributes);
	virtual int  get_num_bezier_comps()								{ return 0; }
	virtual void write_bezier_file( int id, FILE* file_id );

	virtual int getNumXSecSurfs();
	virtual void dump_xsec_file(int geom_no, FILE* dump_file);


	//===== Felisa Functions =====
	virtual int write_felisa_file(int geom_no, FILE*)		{ return(geom_no); }
	virtual void write_bco_info(FILE*, int&, int&)			{}
	virtual int get_num_felisa_comps()						{ return(0); }
	virtual int get_num_felisa_line_sources()				{ return(0); }
	virtual int get_num_felisa_tri_sources()				{ return(0); }
	virtual int get_num_felisa_wings()						{ return(0); }
	virtual void write_felisa_line_sources(FILE*)			{ }
	virtual void write_felisa_tri_sources(FILE*)			{ }

	//==== Load Vector of Tri Meshes ====//
	virtual	vector< TMesh* > createTMeshVec()	{ vector< TMesh*> tMeshVec; return tMeshVec; }

	void  storeModelMatrix();

    virtual vec3d getVertex3d(int surf, double x, double p, int r) = 0;//	{ return vec3d(); }
	virtual void  getVertexVec(vector< VertexID > *vertVec) = 0;//		{ }
	vec2d projectPoint(vec3d point, int refl);
	vec3d xformPoint(vec3d point, int refl);
	vec2d viewProjectPoint(vec3d point); 

	//==== Reference Parms ====//
	virtual int getRefFlag()						{ return refFlag; }
	virtual void setRefFlag( int f )				{ refFlag = f; }

	virtual double getRefArea()						{ return refArea; }
	virtual double getRefSpan()						{ return refSpan; }
	virtual double getRefCbar()						{ return refCbar; }

	virtual void setRefArea(double v)				{ if ( !autoRefAreaFlag ) refArea = v; }
	virtual void setRefSpan(double v)				{ if ( !autoRefSpanFlag ) refSpan = v; }
	virtual void setRefCbar(double v)				{ if ( !autoRefCbarFlag ) refCbar = v; }

	virtual void setRefAreaAutoFlag( int f )		{ autoRefAreaFlag = f; }
	virtual void setRefSpanAutoFlag( int f )		{ autoRefSpanFlag = f; }
	virtual void setRefCbarAutoFlag( int f )		{ autoRefCbarFlag = f; }

	virtual int getRefAreaAutoFlag()				{ return autoRefAreaFlag; }
	virtual int getRefSpanAutoFlag()				{ return autoRefSpanFlag; }
	virtual int getRefCbarAutoFlag()				{ return autoRefCbarFlag; }

	virtual void setAeroCenterAutoFlag( int f )		{ autoAeroCenterFlag = f; }
	virtual int  getAeroCenterAutoFlag()			{ return autoAeroCenterFlag; }
	virtual void  setAeroCenter( vec3d p )			{ if ( !autoAeroCenterFlag ) aeroCenter = p; }
	virtual vec3d getAeroCenter()					{ return aeroCenter; }

	virtual vector< AppliedTex > getAppliedTexVec()	{ return appTexVec; }
	virtual int getNumTex();
	virtual AppliedTex* getTex(int id);
	virtual int  getCurrentTexID();
	virtual void setCurrentTexID( int id );
	virtual void addTex( const char* filename );
	virtual void delCurrentTex();

	virtual int getNumSurf()						{ return surfVec.size(); }


	//==== General Parms ====//
	Parm xLoc;					// XForm Translation
	Parm yLoc;
	Parm zLoc;

	Parm xRelLoc;				// XForm Translation Relative to Parent 
	Parm yRelLoc;
	Parm zRelLoc;

	Parm xRot;					// XForm Rotation
	Parm yRot;
	Parm zRot;

	Parm xRelRot;				// XForm Rotation Relative to Parent 
	Parm yRelRot;
	Parm zRelRot;

	Parm origin;

	double lastScaleFactor;
	Parm scaleFactor;			// XForm Scale

	Parm uAttach;				// UV Attach Points
	Parm vAttach;

	Parm numPnts;
	Parm numXsecs;

	int relXFormFlag;
	Parm xLocOff;				// Offset for XLoc
	Parm yLocOff;				// Offset for YLoc
	Parm zLocOff;				// Offset for ZLoc

	Parm xRotOff;				// Offset for XRot
	Parm yRotOff;				// Offset for YRot
	Parm zRotOff;				// Offset for ZRot

	Parm density;
	Parm shellMassArea;

	//==== Structures ====//
	int currPartID;
	void SetCurrPartID( int pid )			{ currPartID = pid; }
	int  GetCurrPartID()					{ return currPartID; }
	void DelCurrPart();
	void AddStructurePart( Part* part )		{ partVec.push_back( part ); }
	vector< Part* > getStructurePartVec()	{ return partVec; }

	//==== Sources for CFD Mesh ====//
	int currSourceID;
	virtual void AddDefaultSources(double base_len = 1.0)						{}
	virtual void SetCurrSourceID( int sid )					{ currSourceID = sid; }
	virtual int  GetCurrSourceID()							{ return currSourceID; }
	virtual void DelCurrSource();
	virtual void DelAllSources();
	virtual void UpdateSources();
	virtual void AddCfdMeshSource( BaseSource* source )		{ sourceVec.push_back( source ); }
	virtual vector< BaseSource* > getCfdMeshSourceVec()		{ return sourceVec; }
	virtual void GetInteriorPnts( vector< vec3d > & pVec )	{}

	//==== Parm Linking ====//
	virtual void LoadLinkableParms( vector< Parm* > & parmVec );
	virtual void AddLinkableParm( Parm* p, vector< Parm* >& pVec, GeomBase* gPtr, const Stringc grpName );

	//==== Set Drag Factors ====//
	virtual void LoadDragFactors( DragFactors& drag_factors )	{};

protected:
	void  buildVertexVec(Xsec_surf * xsurf, int surface, vector< VertexID > * vertVec);
	void  buildVertexVec(vector<TMesh*> * mesh, int surface, vector< VertexID > * vertVec);

	Aircraft* airPtr;

	Geom* parentGeom;
	vector< Geom* > childGeomVec;

	int displayChildrenFlag;

   int ptrID;
   int parentPtrID;
   vector< int > childPtrIDVec;

   int massPrior;
   int shellFlag;

   int id_num;
   Stringc id_str;

   int saveFlag;
   int noshowFlag;
   int fastDrawFlag;

   int yellowFlag;
   int redFlag;

   int selectedFlag;
   int displayFlag;

   int materialID;

   int outputFlag;				// Whether to export this geom
   int outputNameID;

   virtual void drawTextures(bool reflFlag);
   int currTexID;
   vector< AppliedTex > appTexVec;
   vector< Xsec_surf* > surfVec;

   //==== Structure Parts ====//
   vector< Part* > partVec;

   //==== CFD Mesh Sources ====//
   vector< BaseSource* > sourceVec;
 
   //==== Transformations ====//	
   vec3d  center;

   //==== Attach Flags ====//
   int posAttachFlag;
   int matAttachFlag;

   int sym_code;
   vec3d sym_vec;
   float model_mat[4][4];
   float reflect_mat[4][4];
   float matFVec[16];
   float matReflFVec[16];
   Matrix viewMatrix;
   Matrix modelMatrix;
   Matrix reflMatrix;


   //==== Bounding Boxes ====//
   bbox bnd_box;				// Box around geom
   bbox bnd_box_xform;			// Box around xformed geom			

   int drawFlag;

   //==== Protected Functions ====//
   int changedBaseParm( Parm* p );

   //===== Aero Data =====

   int refFlag;					// Is This the Reference Component?
   double refArea;
   double refSpan;
   double refCbar;
   int autoRefAreaFlag;
   int autoRefSpanFlag;
   int autoRefCbarFlag;
   vec3d aeroCenter;
   int autoAeroCenterFlag;

 

};


class VertexID
{
public:
	VertexID()					{ reset(); }
    VertexID(const VertexID& v);
    VertexID& operator=(const VertexID& v);

	Geom * geomPtr;
	int surface;
	double section;
	double point;
	int reflect;

	int axisProjType;
	enum { AXIS_PROJ_NONE, AXIS_PROJ_LINE, AXIS_PROJ_PLANE };
	vec3d axisProjPnt0;
	vec3d axisProjPnt1;
	vec3d axisProjPnt2;

	// base vertex position
	virtual vec3d pos3d()					{ return geomPtr->getVertex3d(surface, section, point, reflect); }
	
	// vertex projected to screen
	virtual vec2d viewProject( vec3d point)	{ return geomPtr->viewProjectPoint(point); }
	virtual vec2d project(vec3d point)		{ return geomPtr->projectPoint(point, reflect); }
	virtual vec2d pos2d()					{ return project(pos3d()); }
	
	// vertex position after transformation
	virtual vec3d xform(vec3d point)		{ return geomPtr->xformPoint(point, reflect); }
	virtual vec3d posXform()				{ return xform(pos3d()); }

	virtual int isSet()						{ return geomPtr != NULL; }
	virtual void reset();

	virtual void setAxisProject( int x, int y, int z, vec3d orig );
	virtual vec3d posXformProjAxis();


	virtual void write(Aircraft * airPtr, xmlNodePtr root, Stringc name);
	virtual void read(Aircraft * airPtr, xmlNodePtr root, Stringc name);
};

class PodGeom : public Geom
{
public:
	PodGeom(Aircraft* aptr);
	virtual ~PodGeom();

	virtual void write(xmlNodePtr node);
	virtual void read(xmlNodePtr node);

	virtual void copy( Geom* fromGeom );
	virtual void parm_changed(Parm*);
	virtual void draw();
	virtual void drawAlpha();
	virtual void update_bbox();

	virtual void generate();
	virtual void generate_body_curve();
	virtual void gen_body_surf();

	virtual vec3d getAttachUVPos(double u, double v);			

	virtual void acceptScaleFactor();
	virtual void resetScaleFactor();

	virtual void computeCenter();

	virtual void write_rhino_file(ON_BinaryFile* archive, ON_3dmObjectAttributes* attributes );
	virtual int  get_num_bezier_comps();
	virtual void write_bezier_file( int id, FILE* file_id );
	virtual void dump_xsec_file(int geom_no, FILE* dump_file);

    virtual vec3d getVertex3d(int surf, double x, double p, int r)	{ return body_surf.get_vertex(x, p, r); }
	virtual void  getVertexVec(vector< VertexID > *vertVec)		{ buildVertexVec(&body_surf, 0, vertVec); }

	vector< TMesh* > createTMeshVec();

	virtual void AddDefaultSources(double base_len = 1.0);
	virtual void LoadLinkableParms( vector< Parm* > & parmVec );
	virtual void GetInteriorPnts( vector< vec3d > & pVec );

//	virtual void RegisterLinkableParms();



	virtual double getRefArea();
	virtual double getRefSpan();
	virtual double getRefCbar();
	virtual vec3d  getAeroCenter();

	Parm length;
	Parm fine_ratio;
 
	herm_curve hcircle;

	Bezier_curve body_curve;
	Xsec_surf body_surf;

};

class GeomGroup : public Geom
{
public:
	GeomGroup(Aircraft* aptr);
	virtual ~GeomGroup();

	virtual void write(xmlNodePtr node)			{}	
	virtual void read(xmlNodePtr node)			{} 

	virtual int getNumXSecSurfs()				{ return 0; }

	virtual void parm_changed(Parm*);
	virtual void draw()									{}
	virtual void drawAlpha()							{}
	virtual int  valid_bnd_box()						{ return 0; }
	virtual void update_bbox()							{}
	virtual void acceptScaleFactor();					
	virtual void resetScaleFactor();
	virtual void setColor( double r, double g, double b);
	virtual void setMaterialID( int m );

	virtual void setGeomVec( vector< Geom* >& gVec )	{ geomVec = gVec; }

	virtual void resetXform();
	virtual void acceptXform();

	virtual void clear();

    virtual vec3d getVertex3d(int surf, double x, double p, int r)	{ return vec3d(); }
	virtual void  getVertexVec(vector< VertexID > *vertVec)		{ }

	virtual int  getScaleLocFlag()						{ return scaleLocFlag; }
	virtual void setScaleLocFlag(int f)					{ scaleLocFlag = f; }

protected:

	int scaleLocFlag;
	vector< Geom* > geomVec;

};


class BlankGeom : public Geom
{
public:
	BlankGeom(Aircraft* aptr);
	virtual ~BlankGeom();

	virtual void copy( Geom* fromGeom );
	virtual void write(xmlNodePtr node);
	virtual void read(xmlNodePtr node);

	virtual void parm_changed(Parm*);
	virtual void draw();
	virtual void drawAlpha();
	virtual void update_bbox()				{}

	virtual int  valid_bnd_box()						{ return 0; }

	virtual void acceptScaleFactor();
	virtual void resetScaleFactor();

	virtual int getNumXSecSurfs()				{ return 0; }

    virtual vec3d getVertex3d(int surf, double x, double p, int r)	{ return vec3d(); }
	virtual void  getVertexVec(vector< VertexID > *vertVec)		{ }

	virtual void setPointMassFlag( int f )			{ pointMassFlag = f; }
	virtual int  getPointMassFlag()					{ return pointMassFlag; }
	virtual void loadPointMass( TetraMassProp* mp );

	Parm pointMass;

protected:

	int pointMassFlag;

};


class XSecGeom : public Geom
{
public:
	XSecGeom(Aircraft* aptr);
	virtual ~XSecGeom();

	virtual int read_xsec( const char* file_name );

	virtual void write(xmlNodePtr node);
	virtual void read(xmlNodePtr node);

	virtual void copy( Geom* fromGeom );
	virtual void parm_changed(Parm*);
	virtual void draw();
	virtual void drawAlpha();
	virtual void update_bbox();

	virtual void generate();

	virtual vec3d getAttachUVPos(double u, double v);			

	virtual void scale();
	virtual void acceptScaleFactor();
	virtual void resetScaleFactor();

	virtual void computeCenter();

	virtual void write_rhino_file(ON_BinaryFile* archive, ON_3dmObjectAttributes* attributes );
	virtual int  get_num_bezier_comps();
	virtual void write_bezier_file( int id, FILE* file_id );
	virtual void dump_xsec_file(int geom_no, FILE* dump_file);

	double three_pnt_angle( vec3d& p0, vec3d & p1, vec3d & p2 );
	vector<int> find_tan_flags( vector< vec3d > pntVec, double angle_limit );

    virtual vec3d getVertex3d(int surf, double x, double p, int r);
	virtual void  getVertexVec(vector< VertexID > *vertVec);

	vector< TMesh* > createTMeshVec();

	virtual void AddDefaultSources(double base_len = 1.0);
	virtual void LoadLinkableParms( vector< Parm* > & parmVec );
	virtual void GetInteriorPnts( vector< vec3d > & pVec );

	virtual double getRefArea();
	virtual double getRefSpan();
	virtual double getRefCbar();
	virtual vec3d  getAeroCenter();

	vector< Xsec_surf > xsecCompVec;	

	Parm xScale;
	Parm yScale;
	Parm zScale;

	double lastXScale;
	double lastYScale;
	double lastZScale;

	Parm uAngle;
	Parm wAngle;



};
#endif // !defined(AFX_GEOM_H__D3CD0543_7430_11D7_AC31_0003473A025A__INCLUDED_)
