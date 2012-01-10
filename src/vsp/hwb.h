//******************************************************************************
//
//   Hybrid Wing Body Geometry Class
//
//
//   Michael Link - 6/11/08
//   AVID LLC
//
//   jrg - 2/2/10
//   Added HwbSect and HwbBaseGeom (which are copies of wingsect/mw_wing) because when
//	 I changed the doubles to Parms in WingSect - it would crash hwb.  I think
//   there is some memory corruption in smooth_wing_joints.  Also, there is a 
//   huge memory leak in HWB (multiple airfoils are created and never deleted)
//   Watch the memory grow as you adjust the fillet sliders and not go down
//   after deleting the HWB.
//******************************************************************************

#ifndef HWB_GEOM_H
#define HWB_GEOM_H

#include "af.h"
#include "parm.h"
#include "geom.h"
#include "xsec_surf.h"
#include "herm_curve.h"
#include "xmlvsp.h"
#include "dyn_array.h"
#include "vec2d.h"
#include "editCurve.h"
#include <vector>

using namespace std;

//#include "ms_wing.h"

class HwbSect //: public DrawBase
{
public:

  HwbSect();

  virtual void fillDependData();
  virtual double tan_sweep_at(double loc, int sym_code);

	Af* rootAf;
	Af* tipAf;

  int driver;

  double ar;		// Aspect Ratio
  double tr;		// Taper Ratio
  double area;
  double span;
  double tc;		// Tip Chord
  double rc;		// Root Chord
  double sweep;
  double sweepLoc;	// Sweep Location

  double twist;
  double twistLoc;

  double dihedral;
  double dihed_crv1;
  double dihed_crv2;

  double dihed_crv1_str;
  double dihed_crv2_str;
	
  int dihedRotFlag;
  int smoothBlendFlag;

  int num_xsecs;
  int num_interp_xsecs;
  int num_actual_xsecs;

};

class HwbWingPnt
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

class HwbBaseGeom : public Geom
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

   Parm sect_aspect;
   Parm sect_taper;
   Parm sect_area;
   Parm sect_span;
   Parm sect_rc;
   Parm sect_tc;
   Parm sect_sweep;
   Parm sect_sweep_loc;
   Parm sect_twist;
   Parm sect_twist_loc;

   Parm sect_dihed1;
   Parm sect_dihed2;
   Parm sect_dihed_crv1;
   Parm sect_dihed_crv2;
   Parm sect_dihed_crv1_str;
   Parm sect_dihed_crv2_str;

   Parm deg_per_seg;
   Parm max_num_segs;

   bool rel_twist_flag;
   bool rel_dihedral_flag;
   bool round_end_cap_flag;


public:

   HwbBaseGeom(Aircraft* aptr);
   virtual ~HwbBaseGeom();

   virtual void copy( Geom* fromGeom );
   virtual void define_parms();

   //==== Sections and Airfoils ====//
   dyn_array< HwbSect > sects;
   int rootActiveFlag;
   dyn_array< Af* > foils;
   HwbSect copySect;

   Parm* get_total_area()               { return(&total_area); }
   Parm* get_total_span()               { return(&total_span); }
   Parm* get_total_proj_span()          { return(&total_proj_span); }
   Parm* get_avg_chord()                { return(&avg_chord); }

   Parm* get_sweep_off()                { return(&sweep_off); }

   Parm* get_total_aspect()             { return(&total_aspect); }
   Parm* get_sect_proj_span()			{ return(&sect_proj_span); }

   Parm* get_sect_aspect()				{ return(&sect_aspect); }
   Parm* get_sect_taper()				{ return(&sect_taper); }
   Parm* get_sect_area()				{ return(&sect_area); }
   Parm* get_sect_span()				{ return(&sect_span); }
   Parm* get_sect_rc()					{ return(&sect_rc); }
   Parm* get_sect_tc()					{ return(&sect_tc); }
   Parm* get_sect_sweep()				{ return(&sect_sweep); }
   Parm* get_sect_sweep_loc()			{ return(&sect_sweep_loc); }
   Parm* get_sect_twist()				{ return(&sect_twist); }
   Parm* get_sect_twist_loc()			{ return(&sect_twist_loc); }

   Parm* get_sect_dihed1()				{ return(&sect_dihed1); }
   Parm* get_sect_dihed2()				{ return(&sect_dihed2); }
   Parm* get_sect_dihed_crv1()			{ return(&sect_dihed_crv1); }
   Parm* get_sect_dihed_crv2()			{ return(&sect_dihed_crv2); }
   Parm* get_sect_dihed_crv1_str()		{ return(&sect_dihed_crv1_str); }
   Parm* get_sect_dihed_crv2_str()		{ return(&sect_dihed_crv2_str); }
  
   Parm* get_deg_per_seg()				{ return(&deg_per_seg); }
   Parm* get_max_num_segs()				{ return(&max_num_segs); }

   virtual int  get_root_active()								{ return rootActiveFlag; }
   virtual void set_root_active()								{ rootActiveFlag = 1; }
   virtual void set_tip_active()								{ rootActiveFlag = 0; }
   virtual Af* get_root_af_ptr()								{ return sects[currSect].rootAf; }
   virtual Af* get_tip_af_ptr()									{ return sects[currSect].tipAf; }
   virtual Af* get_af_ptr();

   virtual int get_driver()										{ return( driver ); }
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
   virtual void LoadDragFactors( DragFactors& drag_factors );

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

   virtual void AddDefaultSources(double base_len = 1.0);
   virtual void LoadLinkableParms( vector< Parm* > & parmVec );
   virtual void GetInteriorPnts( vector< vec3d > & pVec );


protected:
	Xsec_surf mwing_surf;
	Xsec_surf base_surf;

	int currSect;
	int nextSect;
	int driver;

	//===== Aerodynamic Data =====
	double trimmed_span;

   virtual void computeTotals();
   virtual void loadWingPnts( dyn_array< HwbWingPnt > & wingPnts );
};


//========================================================================//
//========================================================================//
//========================================================================//
//========================================================================//

class Hwb_geom : public HwbBaseGeom
{

	public:
		Hwb_geom(Aircraft* aptr);
		virtual ~Hwb_geom();

   		virtual void copy( Geom* fromGeom );
	
		virtual void write(xmlNodePtr);
		virtual void read(xmlNodePtr);

		virtual void draw_sect_box();
		virtual void draw();
		virtual void add_sect();
		virtual void define_parms();
		virtual void draw_refl_sect_box();
		virtual void parm_changed( Parm* );
		virtual void ins_sect();
		virtual void del_sect();
		virtual void paste_sect();
		virtual void scale();
		virtual void set_dihed_rot_flag( int );
		virtual void set_num_interp( int );
		virtual void loadWingPnts( dyn_array< HwbWingPnt >& );
		virtual void generate_surf();
		//virtual void loadWingStrakeGeom( Hwb_geom* );
		virtual void write_rhino_file(ON_BinaryFile* archive, ON_3dmObjectAttributes* attributes );
		virtual int get_curr_joint()				{return currSect;}
		virtual void set_curr_joint( int );
		virtual void set_curr_sect( int );
		virtual void smooth_wing_joints();
		virtual void generate();
		virtual void set_highlight_type( int type )		{ highlightType = type; }
		virtual void set_depend_parms();
		virtual void dump_xsec_file(int, FILE*);

		Parm* get_sweep_percent_edge_length() 			{return sweep_percent_edge_length;}
		Parm* get_tesweep_percent_edge_length() 		{return tesweep_percent_edge_length;}
		Parm* get_dihedral_percent_edge_length() 		{return dihedral_percent_edge_length;}
		Parm* get_sweep_degperseg() 				{return sweep_degperseg;}
		Parm* get_tesweep_degperseg() 				{return tesweep_degperseg;}
		Parm* get_dihedral_degperseg() 				{return dihedral_degperseg;}

		Parm* get_filleted_tip_chord()				{return filleted_tip_chord;}
		Parm* get_filleted_root_chord()				{return filleted_root_chord;}

		void DebugOutput();

	protected:
		dyn_array< int > section_sizes;
		dyn_array< HwbSect > sub_sects;
		dyn_array< Parm* > sweep_percent_edge_lengths;
		dyn_array< Parm* > tesweep_percent_edge_lengths;
		dyn_array< Parm* > dihedral_percent_edge_lengths;
		dyn_array< Parm* > sweep_degrees_perseg;
		dyn_array< Parm* > tesweep_degrees_perseg;
		dyn_array< Parm* > dihedral_degrees_perseg;
		dyn_array< Af* > sub_sect_foils;
		dyn_array< double > chord_lengths;
		dyn_array< int > dihedral_rot_starting_indices;

		Parm* sweep_percent_edge_length;
		Parm* tesweep_percent_edge_length;
		Parm* dihedral_percent_edge_length;
		Parm* sweep_degperseg;
		Parm* tesweep_degperseg;
		Parm* dihedral_degperseg;

		Parm* filleted_tip_chord;
		Parm* filleted_root_chord;
		
		int get_starting_index(int index);

	private:
		int highlightType;
		bool skip_generation;
};

#endif
