//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//

//******************************************************************************
//    
//		Fuselage Geometry Class
//  
//		J.R. Gloudemans - 8/18/2009
//
//******************************************************************************

#ifndef FUSELAGEGEOM_H
#define FUSELAGEGEOM_H


#include "vec2d.h"
#include "vec3d.h"
#include "parm.h"
#include "bezier_curve.h"
#include "xsec_surf.h"
#include "herm_curve.h"
#include "bbox.h"
#include "xmlvsp.h"
#include "geom.h"
#include "FuselageXSec.h"

#include "fuse_geom.h"

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


class FuselageGeom : public Geom
{
   Xsec_surf surf;
   array_2d<vec3d> pnt_xsec;

   //==== Cross Section Data ====//
   int curr_xsec_num;
   FuselageXSec saved_xsec;

   herm_curve spine;
   
   Parm length;

   //===== Control Points =====//
   int activeControlPntID;
   vector< ControlPnt > cPntVec;

   //===== Computed Parameters =====
   float comp_parms[NUM_COMP_PARMS];
   array_2d<float>slice_parms; 

   //==== Private Functions ====//
   void gen_parms();

public:

   FuselageGeom(Aircraft* aptr);
   virtual ~FuselageGeom();

   virtual void copy( Geom* fromGeom );

   //==== Cross Section Data ====//
   vector< FuselageXSec* > xsecVec;

   virtual void drawAlpha(void);
   virtual void parm_changed(Parm* p);
   virtual void update_bbox();
   virtual vec3d getAttachUVPos(double u, double v);			
   virtual void acceptScaleFactor();
   virtual void resetScaleFactor();

   virtual void computeCenter();
 
   virtual void set_num_pnts( int pnt_in );
   virtual void set_pnt_space_type( int type );
   
   Parm* get_length()							{ return &(length); }
      
   FuselageXSec* get_curr_xsec()				{ return xsecVec[curr_xsec_num]; }
   FuselageXSec* get_xsec(int number);
   FuselageXSec* add_xsec(FuselageXSec* curr_xsec);
   FuselageXSec* delete_xsec(FuselageXSec* curr_xsec);
   virtual void copy_xsec(FuselageXSec* curr_xsec);
   virtual void paste_xsec(FuselageXSec* curr_xsec);
   virtual int  get_num_xsecs()					{ return(xsecVec.size()); }  
   virtual void set_curr_xsec_num(int number)	{ curr_xsec_num = number; }  
   virtual int  get_curr_xsec_num()				{ return(curr_xsec_num); }  
   virtual void draw();
   virtual void draw_wire_fuselage();
   virtual void generate();
   virtual void regenerate();
   virtual void write(xmlNodePtr node);
   virtual void read(xmlNodePtr node);
   virtual void read(FILE* file_id) {Geom::read(file_id);}
 
   virtual void load_normals();
   virtual void load_hidden_surf();

   //===== Computed Paramter Functions =====
   virtual void	 set_comp_parm(int index, float val)		{ comp_parms[index] = val; }
   virtual float get_comp_parm(int index)					{ return(comp_parms[index]); }
   virtual void	 set_num_slices(int num_slc)				{ slice_parms.init(NUM_SLICE_PARMS, num_slc); }
   virtual void	 set_slice_parm(int index, int slc_no, float val)	{ slice_parms( index, slc_no) =  val; }
   virtual float get_slice_parm(int index, int slc_no)				{ return(slice_parms( index, slc_no)); }

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
   
   virtual void comp_spine();

   virtual float get_area();
   virtual float get_span();
   virtual float get_cbar();

   virtual void LoadDragFactors( DragFactors& drag_factors );

   //===== Scaling methods =====
   virtual void scale();

   virtual void write_rhino_file(ON_BinaryFile* archive, ON_3dmObjectAttributes* attributes );
   virtual vector< TMesh* > createTMeshVec();

   virtual int  get_num_bezier_comps();
   virtual void write_bezier_file( int id, FILE* file_id );

   virtual void debug_print();

   virtual vec3d getVertex3d(int surf, double x, double p, int r);
   virtual void  getVertexVec(vector< VertexID > *vertVec);

   virtual void incNumInterpAll(int off);	
   virtual void updateNumInter();

   //===== Control Point Draw/Edit =====//
   virtual void draw2D(vec2d & cursor);
   virtual void drawControlPoints();

   virtual double getRefArea();
   virtual double getRefSpan();
   virtual double getRefCbar();
   virtual vec3d  getAeroCenter();

   virtual void AddDefaultSources(double base_len = 1.0);
   virtual void DeleteAllXSecs();
   virtual void LoadLinkableParms( vector< Parm* > & parmVec );
   virtual void RemoveXSecParmReferences( FuselageXSec* xsec );
   virtual void GetInteriorPnts( vector< vec3d > & pVec );
};

#endif
