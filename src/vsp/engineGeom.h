//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//

//******************************************************************************
//    
//   Engine Geometry Class
//  
// 
//   J.R. Gloudemans - 11/7/94
//   Sterling Software
//
//******************************************************************************

#ifndef ENG_GEOM_H
#define ENG_GEOM_H

#include "parm.h"
#include "geom.h"
#include "xsec_surf.h"
#include "herm_curve.h"
#include "xmlvsp.h"
#include "dyn_array.h"
#include "fuse_xsec.h"

#define UPD_ALL		0
#define UPD_LOC_ROT	1
#define UPD_ENGINE	2
#define UPD_INLET	3
#define UPD_NOZZLE	4
#define UPD_DIVERTOR	5

#define NUM_HUB_XSECS	6
#define NUM_ENG_XSECS 	4
#define HUB_LENGTH_RAD 	1.5

#define SUB_PITOT	1
#define SUPER_PITOT	2
#define TRIANG_2_D	3
#define INFINITE_2_D	4
#define FIXED_CONICAL	5
#define TRANS_CONICAL	6

#define CONVER_AXI		0
#define CONVER_RECT		1
#define CONVER_DIVER_AXI	2
#define CONVER_DIVER_RECT	3

#define NUM_PNTS_XSEC 	21
#define NXSEC_1		6
#define NXSEC_2		4
#define NXSEC_3		9

#define NUM_INL_XSECS	NXSEC_1+NXSEC_2+NXSEC_3

#define NUM_INL_DUCT_XSECS	20

#define NOZ_NX_1	7
#define NOZ_NX_2	6

#define NUM_NOZ_XSECS	NOZ_NX_1+NOZ_NX_2

class EngineGeom : public Geom
{
	enum { INLET_SURF, INL_DUCT_IN_SURF, INL_DUCT_OUT_SURF, NOZZLE_SURF, ENGINE_SURF };

   //======================//
   //==== Engine Parms ====//
   //======================//
   fuse_xsec eng_xsecs[NUM_HUB_XSECS + NUM_ENG_XSECS];
 
   Parm rad_tip;
   Parm max_tip;
   Parm hub_tip;
   Parm length;
   Xsec_surf engine_surf;

   //=====================//
   //==== Inlet Parms ====//
   //=====================//
   int inl_type;
   int inl_noz_color;
   int inl_xy_sym_flag;
   Xsec_surf inlet_surf;
 
   //==== Subsonic Pitot Inlet Parms ====//
   Bezier_curve inl_curve;
   fuse_xsec inl_xsec; 
   int inl_half_split_flag;
   Parm cowl_length;
   Parm area_ratio_eng_thrt;
   Parm area_ratio_hl_thrt;
   Parm lip_fine;
   Parm ht_wid_ratio;
   Parm upper_shape;
   Parm lower_shape;
   Parm inl_x_rot;
   Parm inl_scarf;

   //==== Duct Parms ====//
   int inl_duct_flag;
   Bezier_curve inl_duct_curve;
   Xsec_surf inl_duct_in;
   Xsec_surf inl_duct_out;
   Parm inl_duct_x_off;
   Parm inl_duct_y_off;
   Parm inl_duct_shape;

   //==== Divertor Parms ====//
   int divertor_flag;
   Parm divertor_height;
   Parm divertor_length;

   //=====================//
   //==== Nozzle Parms ===//
   //=====================//
   Xsec_surf nozzle_surf;
   int noz_type;

   //==== Convergent/Divergent Parms ====//
   Bezier_curve noz_curve;
   fuse_xsec noz_xsec; 
   Parm noz_length;
   Parm exit_area_ratio;
   Parm noz_ht_wid_ratio;
   Parm exit_throat_ratio;
   Parm dive_flap_ratio;

   //==== Duct Parms ====//
   int noz_duct_flag;
   Parm noz_duct_x_off;
   Parm noz_duct_y_off;
   Parm noz_duct_shape;

   //==== Simp Surf ====//
   Xsec_surf simp_surf;



public:

   EngineGeom(Aircraft* aptr);
   virtual ~EngineGeom();

   virtual void copy( Geom* fromGeom );

   void define_eng_parms(); 
   void define_inl_parms(); 
   void define_noz_parms(); 
   void set_ducted_default(); 

   //======================//
   //==== Engine Parms ====//
   //======================//
   Parm* get_rad_tip()			{ return(&rad_tip); }
   Parm* get_max_tip()			{ return(&max_tip); }
   Parm* get_hub_tip()			{ return(&hub_tip); }
   Parm* get_length()			{ return(&length); }
   void generate_engine_xsecs();
   
   //=====================//
   //==== Inlet Parms ====//
   //=====================//
   int get_inl_type()			{ return(inl_type); }

   void put_inl_noz_color( int col_in )	{ inl_noz_color = col_in; }
   int get_inl_noz_color( )		{ return(inl_noz_color); }

   void set_inl_duct_flag(int flag)	{ inl_duct_flag = flag; }
   int get_inl_duct_flag()		{ return(inl_duct_flag); }
   Parm* get_inl_duct_x_off()		{ return(&inl_duct_x_off); }
   Parm* get_inl_duct_y_off()		{ return(&inl_duct_y_off); }
   Parm* get_inl_duct_shape()		{ return(&inl_duct_shape); }

   Parm* get_divertor_height()		{ return(&divertor_height); }
   Parm* get_divertor_length()		{ return(&divertor_length); }

   Parm* get_cowl_length()		{ return(&cowl_length); }
   Parm* get_area_ratio_eng_thrt()	{ return(&area_ratio_eng_thrt); }
   Parm* get_area_ratio_hl_thrt()	{ return(&area_ratio_hl_thrt); }
   Parm* get_lip_fine()			{ return(&lip_fine); }
   Parm* get_ht_wid_ratio()		{ return(&ht_wid_ratio); }
   Parm* get_upper_shape()		{ return(&upper_shape); }
   Parm* get_lower_shape()		{ return(&lower_shape); }

   Parm* get_inl_x_rot()		{ return(&inl_x_rot); }
   Parm* get_inl_scarf()		{ return(&inl_scarf); }

   //=====================//
   //==== Nozzle Parms ====//
   //=====================//
   Parm* get_noz_duct_x_off()		{ return(&noz_duct_x_off); }
   Parm* get_noz_duct_y_off()		{ return(&noz_duct_y_off); }
   Parm* get_noz_duct_shape()		{ return(&noz_duct_shape); }

   Parm* get_exit_area_ratio()		{ return(&exit_area_ratio); }
   Parm* get_noz_length()			{ return(&noz_length); }
   Parm* get_noz_ht_wid_ratio()		{ return(&noz_ht_wid_ratio); }
   Parm* get_exit_throat_ratio()	{ return(&exit_throat_ratio); }
   Parm* get_dive_flap_ratio()		{ return(&dive_flap_ratio); }

   void generate_inlet_curves();
   void gen_inl_surf();
   void generate_nozzle_curves();
   void gen_noz_surf();
   void generate_inl_duct_xsecs();
   void load_hidden_surf();
   void load_normals();
   void update_bbox();

   void load_simp_surf();
  
   void draw();
   void generate();
   void regenerate();
   void gen_surf();
   void write(FILE* dump_file);
   void write(xmlNodePtr node);
   void read(FILE* dump_file);
   void read(xmlNodePtr node);

   void dump_xsec_file(int geom_no, FILE* dump_file);

   virtual int write_felisa_file(int geom_no, FILE* dump_file);
   virtual void write_bco_info(FILE*, int&, int&);
   virtual int get_num_felisa_comps();

   virtual int get_num_felisa_line_sources();
   virtual int get_num_felisa_tri_sources();
   virtual void write_felisa_line_sources(FILE*);
   virtual void write_felisa_tri_sources(FILE*);

   virtual int  get_num_bezier_comps();
   virtual void write_bezier_file( int id, FILE* file_id );

   virtual void drawAlpha(void);
   virtual void scale();		
   virtual void acceptScaleFactor();
   virtual void resetScaleFactor();

   virtual void write_rhino_file(ON_BinaryFile* archive, ON_3dmObjectAttributes* attributes );
   virtual vector< TMesh* > createTMeshVec();

   float get_area();
   float get_span();
   float get_cbar();

   virtual void parm_changed(Parm* chg_parm);

   //===== Aero Functions =====
//   virtual void	write_aero_file(FILE* aero_file, int aero_id_num);

    virtual vec3d getVertex3d(int surfid, double x, double p, int r);
	virtual void  getVertexVec(vector< VertexID > *vertVec);

	virtual vec3d getAttachUVPos(double u, double v);
	virtual void  AddDefaultSources(double base_len = 1.0);

   virtual double getRefArea();
   virtual double getRefSpan();
   virtual double getRefCbar();
   virtual vec3d  getAeroCenter();

   virtual void LoadLinkableParms( vector< Parm* > & parmVec );
   virtual void GetInteriorPnts( vector< vec3d > & pVec );

};

#endif
