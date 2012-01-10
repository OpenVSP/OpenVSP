//******************************************************************************
//    
//   Fuselage Geometry Class
//  
// 
//   J.R. Gloudemans - 10/2/93
//   Sterling Software
//
//******************************************************************************

#ifndef FUSE_OLDGEOM_H
#define FUSE_OLDGEOM_H


#include "vec2d.h"
#include "vec3d.h"
#include "parm.h"
#include "bezier_curve.h"
#include "xsec_surf.h"
#include "herm_curve.h"
#include "bbox.h"
#include "xmlvsp.h"
#include "geom.h"
#include "fuse_xsec.h"

#include "xsec_surf.h"
#include "dl_list.h"

#define NOSE 0
#define AFT  1

//===== Define Computed Parameters =====
#define NUM_COMP_PARMS	6
#define TOTAL_LENGTH	0
#define NOSE_ASPECT	1
#define AFT_ASPECT	2
#define SEP_XSEC_AREA	3
#define MAX_XSEC_AREA	4
#define MAX_XSEC_WIDTH  5

//===== Define Computed Slice Parameters =====
#define NUM_SLICE_PARMS 6
#define X_LOC		0
#define BURY_FLAG	1
#define AREA		2
#define ASPECT		3
#define CAMBER		4
#define CORNER_RAD	5


class ControlPnt
{
public:

	enum {TOP, BOTTOM, SIDE, };

	int xSecID;
	int pntID;

	vec3d pnt3d;
	vec2d pnt2d;

};

class Fuse_geom : public Geom
{
	enum { OML_SURF, IML_SURF };


   int imlFlag;
   vec3d imlColor;
   int imlMaterialID;
   Xsec_surf oml_surf;
   Xsec_surf iml_surf;
   array_2d<vec3d> pnt_xsec;

   //==== Cross Section Data ====//
   int curr_xsec_num;
   fuse_xsec saved_xsec;

   herm_curve spine;
   
   Parm length;
   Parm camber;
   Parm camber_loc;
   Parm aft_offset;
   Parm nose_angle;
   Parm nose_str;
   Parm nose_rho;
   Parm aft_rho;

   int nose_super_flag;
   int aft_super_flag;

   //===== Control Points =====//
   int activeControlPntID;
   vector< ControlPnt > cPntVec;

   //===== Computed Parameters =====
   float comp_parms[NUM_COMP_PARMS];
   array_2d<float>slice_parms; 

   //==== Private Functions ====//
   void gen_parms();

public:

   Fuse_geom(Aircraft* aptr);
   virtual ~Fuse_geom();

   virtual void copy( Geom* fromGeom );

   //==== Cross Section Data ====//
   vector< fuse_xsec* > xsecVec;

   virtual void drawAlpha(void);
   virtual void parm_changed(Parm* p);
   virtual void update_bbox();
   virtual vec3d getAttachUVPos(double u, double v);			
   virtual void acceptScaleFactor();
   virtual void resetScaleFactor();

   virtual void computeCenter();
 
   virtual void set_num_pnts( int pnt_in );
   virtual void set_pnt_space_type( int type );
   
   Parm* get_length()					{ return &(length); }
   Parm* get_camber()					{ return &(camber); }
   Parm* get_camber_loc()				{ return &(camber_loc); }
   Parm* get_aft_offset()				{ return &(aft_offset); }
   Parm* get_nose_angle()				{ return &(nose_angle); }
   Parm* get_nose_str()					{ return &(nose_str); }
   Parm* get_nose_rho()					{ return &(nose_rho); }
   Parm* get_aft_rho()					{ return &(aft_rho); }
      
   fuse_xsec* get_curr_xsec()			{ return xsecVec[curr_xsec_num]; }
   fuse_xsec* get_xsec(int number);
   fuse_xsec* add_xsec(fuse_xsec* curr_xsec);
   fuse_xsec* delete_xsec(fuse_xsec* curr_xsec);
   virtual void copy_xsec(fuse_xsec* curr_xsec);
   virtual void paste_xsec(fuse_xsec* curr_xsec);
   virtual int  get_num_xsecs()					{ return(xsecVec.size()); }  
   virtual void set_curr_xsec_num(int number)	{ curr_xsec_num = number; }  
   virtual int  get_curr_xsec_num()				{ return(curr_xsec_num); }  
   virtual void draw();
   virtual void generate();
   virtual void regenerate();
   virtual void write(FILE* dump_file);
   virtual void write(xmlNodePtr node);
   virtual void read(FILE* dump_file);
   virtual void read(xmlNodePtr node);
 
   virtual void load_normals();
   virtual void load_hidden_surf();

   //===== Aero Functions =====
//jrg   virtual void	write_aero_file(FILE* aero_file, int aero_id_num);

   //===== Computed Paramter Functions =====
   virtual void	 set_comp_parm(int index, float val)		{ comp_parms[index] = val; }
   virtual float get_comp_parm(int index)			{ return(comp_parms[index]); }
   virtual void	 set_num_slices(int num_slc)				{ slice_parms.init(NUM_SLICE_PARMS, num_slc); }
   virtual void	 set_slice_parm(int index, int slc_no, float val)	{ slice_parms( index, slc_no) =  val; }
   virtual float get_slice_parm(int index, int slc_no)			{ return(slice_parms( index, slc_no)); }

   virtual void dump_xsec_file(int geom_no, FILE* dump_file);
	
   virtual int write_felisa_file(int geom_no, FILE* dump_file);
   virtual void write_bco_info(FILE*, int&, int&);
   virtual int get_num_felisa_comps();

   virtual int get_num_felisa_line_sources();
   virtual int get_num_felisa_tri_sources()		{ return(0); }
   virtual void write_felisa_line_sources(FILE*);
   virtual void write_felisa_tri_sources(FILE*)		{  }

   virtual int  get_num_felisa_comps_single_surf();
   virtual int  write_felisa_file_single_surf( int geom_no, FILE* dump_file );
   virtual void write_bco_info_single_surf(FILE*, int&, int&);
   virtual void write_felisa_line_sources_single_surf(FILE*);

   virtual void xsec_trigger();
   virtual void gen_surf();
   virtual void interpolate_from_file( int mlt, float sup_fract, fuse_xsec * xsec0, fuse_xsec * xsec1, fuse_xsec * out );
   virtual void interpolate_from_edit_crv( int mlt, float sup_fract, fuse_xsec * xsec0, fuse_xsec * xsec1, fuse_xsec * out );
 
   virtual void super_ellipse_interp_xsec( fuse_xsec* xsec0, fuse_xsec* xsec1, double fract,   
										   int ml_type, double rho, fuse_xsec* result );
   
   virtual void comp_spine();

   virtual float get_area();
   virtual float get_span();
   virtual float get_cbar();

   virtual void LoadDragFactors( DragFactors& drag_factors );


   //===== Scaling methods =====
   virtual void scale();

   virtual void set_iml_flag( int flag );
   virtual int  get_iml_flag(  )								{ return imlFlag;  }
	
   virtual void setImlColor( double r, double g, double b);
   virtual vec3d getImlColor()									{ return imlColor; }

   virtual void setImlMaterialID( int m );
   virtual int getImlMaterialID()								{ return imlMaterialID; }

   virtual void write_rhino_file(ON_BinaryFile* archive, ON_3dmObjectAttributes* attributes );
   virtual vector< TMesh* > createTMeshVec();

   virtual int  get_num_bezier_comps();
   virtual void write_bezier_file( int id, FILE* file_id );

   virtual void debug_print();

   virtual vec3d getVertex3d(int surfid, double x, double p, int r);
   virtual void  getVertexVec(vector< VertexID > *vertVec);

   virtual void incNumInterpAll(int off);	
   virtual void updateNumInter();

   virtual void setNoseSuperFlag(int f)							{ nose_super_flag = f; }
   virtual int  getNoseSuperFlag()								{ return nose_super_flag; }
   virtual void setAftSuperFlag(int f)							{ aft_super_flag = f; }
   virtual int  getAftSuperFlag()								{ return aft_super_flag; }

   //===== Control Point Draw/Edit =====//
   virtual void draw2D(vec2d & cursor);
   virtual void drawControlPoints();

   virtual double getRefArea();
   virtual double getRefSpan();
   virtual double getRefCbar();
   virtual vec3d  getAeroCenter();

   virtual void DeleteAllXSecs();

   virtual void AddDefaultSources(double base_len = 1.0);
   virtual void RemoveXSecParmReferences( fuse_xsec* xsec );
   virtual void LoadLinkableParms( vector< Parm* > & parmVec );
   virtual void GetInteriorPnts( vector< vec3d > & pVec );

};

#endif
