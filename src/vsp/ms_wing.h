//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//

//******************************************************************************
//
//   Multi Section Wing Geometry Class
//
//   J.R. Gloudemans - 10/2/01
//       Modified to ms_wing for arbitrary sections
//
//   J.R. Gloudemans - 2/1/95
//   Sterling Software
//       Original strake/aft multi-section wing
//
//******************************************************************************

#ifndef MS_WING_GEOM_H
#define MS_WING_GEOM_H

#include "parm.h"
#include "geom.h"
#include "xsec_surf.h"
#include "herm_curve.h"
#include "xmlvsp.h"
#include "dyn_array.h"
#include "vec2d.h"
#include "editCurve.h"


#define UPD_MSW_ALL              0
#define UPD_MSW_LOC_ROT          1
#define UPD_MSW_NUM_PNTS         2
#define UPD_MSW_AIRFOIL          3
#define UPD_MSW_TOTAL_SPAN       4
#define UPD_MSW_TOTAL_AREA       6
#define UPD_MSW_AVG_CHORD        7
#define UPD_MSW_SECTS            8

#define MSW_HIGHLIGHT_NONE		0
#define MSW_HIGHLIGHT_SECT		1
#define MSW_HIGHLIGHT_JOINT		2
#define MSW_HIGHLIGHT_FOIL		3

class Ms_wing_screen;
class Af;
class EditCurve;
class WingGeom;

#define MS_AR_TR_A  0
#define MS_AR_TR_S  1
#define MS_AR_TR_TC 2
#define MS_AR_TR_RC 3
#define MS_S_TC_RC  4
#define MS_A_TC_RC  5
#define MS_TR_S_A   6
#define MS_AR_A_RC  7

#define DEGREES_PER_AF (8.0)
#define MAX_AF_PER_BLEND (9)

class WingSect : public DrawBase
{
public:

  WingSect();
  virtual void draw();

  virtual void fillDependData();
  virtual double tan_sweep_at(double loc, int sym_code);
  virtual double tan_sweep_at(double loc, int sym_code, double swp);
  virtual double tan_sweep_at(double curr_loc, double new_loc, int sym_code);

  virtual void SetGeomPtr( Geom* geomPtr );
  virtual vector< Parm* > GetLinkableParms();

  Af* rootAf;
  Af* tipAf;

  int driver;
  int dihedRotFlag;
  int smoothBlendFlag;

  int num_xsecs;
  int num_interp_xsecs;
  int num_actual_xsecs;

  double refSweepLoc;

  Parm* get_ar()					{ return &ar; }
  Parm* get_tr()					{ return &tr; }
  Parm* get_area()					{ return &area; }
  Parm* get_span()					{ return &span; }
  Parm* get_tc()					{ return &tc; }
  Parm* get_rc()					{ return &rc; }
  Parm* get_sweep()					{ return &sweep; }
  Parm* get_sweepLoc()				{ return &sweepLoc; }

  Parm* get_twist()					{ return &twist; }
  Parm* get_twistLoc()				{ return &twistLoc; }

  Parm* get_dihedral()				{ return &dihedral; }
  Parm* get_dihed_crv1()			{ return &dihed_crv1; }
  Parm* get_dihed_crv2()			{ return &dihed_crv2; }
  Parm* get_dihed_crv1_str()		{ return &dihed_crv1_str; }
  Parm* get_dihed_crv2_str()		{ return &dihed_crv2_str; }

  double ar_val()					{ return ar(); }
  double tr_val()					{ return tr(); }
  double area_val()					{ return area(); }
  double span_val()					{ return span(); }
  double tc_val()					{ return tc(); }
  double rc_val()					{ return rc(); }
  double sweep_val()				{ return sweep(); }
  double sweepLoc_val()				{ return sweepLoc(); }

  double twist_val()				{ return twist(); }
  double twistLoc_val()				{ return twistLoc(); }

  double dihedral_val()					{ return dihedral(); }
  double dihed_crv1_val()				{ return dihed_crv1(); }
  double dihed_crv2_val()				{ return dihed_crv2(); }
  double dihed_crv1_str_val()			{ return dihed_crv1_str(); }
  double dihed_crv2_str_val()			{ return dihed_crv2_str(); }

  void ar_set( double v )			{ ar.set( v ); }
  void tr_set( double v )			{ tr.set( v ); }
  void area_set( double v )			{ area.set( v ); }
  void span_set( double v )			{ span.set( v ); }
  void tc_set( double v )			{ tc.set( v ); }
  void rc_set( double v )			{ rc.set( v ); }
  void sweep_set( double v )		{ sweep.set( v ); }
  void sweepLoc_set( double v )		{ sweepLoc.set( v ); }

  void twist_set( double v )		{ twist.set( v ); }
  void twistLoc_set( double v )		{ twistLoc.set( v ); }

  void dihedral_set( double v )			{ dihedral.set( v ); }
  void dihed_crv1_set( double v )		{ dihed_crv1.set( v ); }
  void dihed_crv2_set( double v )		{ dihed_crv2.set( v ); }
  void dihed_crv1_str_set( double v )	{ dihed_crv1_str.set( v ); }
  void dihed_crv2_str_set( double v )	{ dihed_crv2_str.set( v ); }



  void Build( double span_val, double tc_val, double rc_val, double sweep_val, 
	  double dihedral_val, double dihedral_crv1, double dihedral_crv2 );

  void SetDriver( int d );

private:


  Parm ar;
  Parm tr;		// Taper Ratio
  Parm area;
  Parm span;
  Parm tc;		// Tip Chord
  Parm rc;		// Root Chord
  Parm sweep;
  Parm sweepLoc;	// Sweep Location

  Parm twist;
  Parm twistLoc;

  Parm dihedral;
  Parm dihed_crv1;
  Parm dihed_crv2;

  Parm dihed_crv1_str;
  Parm dihed_crv2_str;
	

};

class WingPnt
{
public:

	double x;
	double y;
	double z;

	int sectID;
	double sectFract;

	double chord;
	double twist;
	double twistLoc;
	double dihedRot;

	int blendFlag;
};


class Ms_wing_geom : public Geom
{
protected:

   int highlightType;

   Parm total_area;
   Parm total_span;
   Parm total_proj_span;
   Parm avg_chord;
   Parm sect_proj_span;

   Parm sweep_off;

   Parm total_aspect;

   Parm deg_per_seg;
   Parm max_num_segs;

   bool rel_twist_flag;
   bool rel_dihedral_flag;
   bool round_end_cap_flag;


public:

   Ms_wing_geom(Aircraft* aptr);
   virtual ~Ms_wing_geom();

   virtual void copy( Geom* fromGeom );
   virtual void define_parms();
   virtual WingSect* getCurrWingSect(); 

   //==== Sections and Airfoils ====//
   dyn_array< WingSect > sects;
   int rootActiveFlag;
   dyn_array< Af* > foils;
   WingSect copySect;

   Parm* get_total_area()               { return(&total_area); }
   Parm* get_total_span()               { return(&total_span); }
   Parm* get_total_proj_span()          { return(&total_proj_span); }
   Parm* get_avg_chord()                { return(&avg_chord); }

   Parm* get_sweep_off()                { return(&sweep_off); }

   Parm* get_total_aspect()             { return(&total_aspect); }
   Parm* get_sect_proj_span()			{ return(&sect_proj_span); }

   Parm* get_sect_aspect()				{ return( sects[currSect].get_ar() ); }
   Parm* get_sect_taper()				{ return( sects[currSect].get_tr() ); }
   Parm* get_sect_area()				{ return( sects[currSect].get_area() ); }
   Parm* get_sect_span()				{ return( sects[currSect].get_span() ); }
   Parm* get_sect_rc()					{ return( sects[currSect].get_rc() ); }
   Parm* get_sect_tc()					{ return( sects[currSect].get_tc() ); }
   Parm* get_sect_sweep()				{ return( sects[currSect].get_sweep() ); }
   Parm* get_sect_sweep_loc()			{ return( sects[currSect].get_sweepLoc() ); }
   double get_sect_ref_sweep_loc()		{ return( sects[currSect].refSweepLoc ); }
   Parm* get_sect_twist()				{ return( sects[currSect].get_twist() ); }
   Parm* get_sect_twist_loc()			{ return( sects[currSect].get_twistLoc() ); }

   Parm* get_sect_dihed1()				{ return( sects[currSect].get_dihedral() ); }
   Parm* get_sect_dihed2()				{ return( sects[nextSect].get_dihedral() ); }
   Parm* get_sect_dihed_crv1()			{ return( sects[currSect].get_dihed_crv1() ); }
   Parm* get_sect_dihed_crv2()			{ return( sects[currSect].get_dihed_crv2() ); }
   Parm* get_sect_dihed_crv1_str()		{ return( sects[currSect].get_dihed_crv1_str() ); }
   Parm* get_sect_dihed_crv2_str()		{ return( sects[currSect].get_dihed_crv2_str() ); }
  
   Parm* get_deg_per_seg()				{ return(&deg_per_seg); }
   Parm* get_max_num_segs()				{ return(&max_num_segs); }

   virtual int  get_root_active()								{ return rootActiveFlag; }
   virtual void set_root_active()								{ rootActiveFlag = 1; }
   virtual void set_tip_active()								{ rootActiveFlag = 0; }
   virtual Af* get_root_af_ptr()								{ return sects[currSect].rootAf; }
   virtual Af* get_tip_af_ptr()									{ return sects[currSect].tipAf; }
   virtual Af* get_af_ptr();

   virtual int get_driver()										{ return( sects[currSect].driver ); }
   virtual void set_driver(int driver_in);

   virtual int get_curr_sect()									{ return( currSect ); }
   virtual void set_curr_sect( int cs );

   virtual void set_curr_joint( int cj );
   virtual int get_curr_joint();
   virtual void set_curr_foil( int cf );
   virtual int get_curr_foil();

   virtual int get_dihed_rot_flag()								{ return( sects[currSect].dihedRotFlag ); }
   virtual void set_dihed_rot_flag( int rf );

   virtual int get_smooth_blend_flag()							{ return( sects[currSect].smoothBlendFlag ); }
   virtual void set_smooth_blend_flag( int sbf );

   virtual int get_num_interp()									{ return( sects[currSect].num_interp_xsecs ); }
   virtual void set_num_interp( int sbf );

   virtual void inc_all_interp();
   virtual void dec_all_interp();

   virtual int get_num_sect()									{ return( sects.dimension() ); }

   virtual bool get_rel_twist_flag()							{ return rel_twist_flag; }
   virtual void set_rel_twist_flag( bool flag );

   virtual bool get_rel_dihedral_flag()							{ return rel_dihedral_flag; }
   virtual void set_rel_dihedral_flag( bool flag );
   virtual double get_sum_dihedral( int sect_id );

   virtual bool get_round_end_cap_flag()						{ return round_end_cap_flag; }
   virtual void set_round_end_cap_flag( bool flag );

   virtual void add_sect();
   virtual void del_sect();
   virtual void ins_sect();
   virtual void copy_sect();
   virtual void paste_sect();

   virtual void set_depend_parms();
   virtual void set_num_pnts_all_afs();
   virtual void matchWingSects();

   virtual void load_hidden_surf();
   virtual void load_normals();
   virtual void update_bbox();

   virtual int  get_highlight_type()							{ return highlightType; }
   virtual void set_highlight_type( int type )					{ highlightType = type; }

   virtual void draw_sect_box();
   virtual void draw_refl_sect_box();
   virtual void draw_bbox( bbox & box );

   virtual void draw();
   virtual void drawAlpha();
   virtual void generate();
   virtual void regenerate();
   virtual void generate_surf();
   virtual void generate_flap_slat_surfs();
   virtual void write(FILE* dump_file);
   virtual void write(xmlNodePtr node);
   virtual void read(FILE* dump_file);
   virtual void read(xmlNodePtr node);
   virtual void loadWingStrakeGeom( WingGeom* wg );
   virtual void write_rhino_file(ON_BinaryFile* archive, ON_3dmObjectAttributes* attributes );
   virtual bool merged_center_foil();
   virtual int get_num_bezier_comps();
   virtual void write_bezier_file( int id, FILE* file_id );

   virtual vector< TMesh* > createTMeshVec();

   virtual vec3d getAttachUVPos(double u, double v);			
   virtual void acceptScaleFactor();
   virtual void resetScaleFactor();

   virtual void dump_xsec_file(int, FILE*);

   //===== Aero Functions =====
   virtual void write_aero_file(FILE* aero_file, int aero_id_num);
   virtual void         set_trimmed_span(double val)           { trimmed_span = val; }
   virtual double        get_trimmed_span()                    { return(trimmed_span); }
   
   //===== Felisa Functions =====   
   virtual int write_felisa_file(int geom_no, FILE* dump_file);
   virtual void write_bco_info(FILE*, int&, int&);
   virtual int get_num_felisa_comps();
   virtual int get_num_felisa_wings();
   virtual int get_num_felisa_line_sources();
   virtual int get_num_felisa_tri_sources();
   virtual void write_felisa_line_sources(FILE*);
   virtual void write_felisa_tri_sources(FILE*);

   virtual double get_area()									  { return total_area(); }
   virtual double get_span()									  { return total_span(); }
   virtual double get_cbar();

   virtual void computeCenter();
   virtual void parm_changed(Parm* chg_parm);

   //==== Scaling Methods ====
   virtual void scale();

    virtual vec3d getVertex3d(int surfid, double x, double p, int r)			{ return mwing_surf.get_vertex(x, p, r); }
	virtual void  getVertexVec(vector< VertexID > *vertVec)		{ buildVertexVec(&mwing_surf, 0, vertVec); }

   virtual double getRefArea();
   virtual double getRefSpan();
   virtual double getRefCbar();
   virtual vec3d  getAeroCenter();
   virtual void   computeCbarAC();
   virtual void LoadDragFactors( DragFactors& drag_factors );
 
   virtual void AddDefaultSources(double base_len = 1.0);
   virtual void LoadLinkableParms( vector< Parm* > & parmVec );
   virtual void RemoveWingSectParmReferences( int sect_id );
   virtual void RemoveAirfoilParmReferences( Af* afPtr );
   virtual void DeleteAllFoilsAndSects();
   virtual void GetInteriorPnts( vector< vec3d > & pVec );
   

protected:
	Xsec_surf mwing_surf;
	Xsec_surf base_surf;

	int currSect;
	int nextSect;
	//int driver;

	//===== Aerodynamic Data =====
	double trimmed_span;

   virtual void computeTotals();
   virtual void loadWingPnts( dyn_array< WingPnt > & wingPnts );
};

#endif

