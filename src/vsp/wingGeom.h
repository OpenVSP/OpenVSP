//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//

//******************************************************************************
//
//   Multi Section Wing (Strake/Aft Ext) Geometry Class
//
//
//   J.R. Gloudemans - 2/1/95
//   Sterling Software
//
//******************************************************************************

#ifndef MWING_GEOM_H
#define MWING_GEOM_H


#include "parm.h"
#include "geom.h"
#include "xsec_surf.h"
#include "herm_curve.h"
#include "xmlvsp.h"
#include "dyn_array.h"
#include "ms_wing.h"

#define AR_TR_A  0
#define AR_TR_S  1
#define AR_TR_TC 2
#define AR_TR_RC 3
#define S_TC_RC  4
#define A_TC_RC  5
#define TR_S_A   6


#define MW_ROOT_AF      0
#define MW_STRAKE_AF    1
#define MW_AFT_AF       2
#define MW_TIP_AF       3

//===== Define Flap Types =====
#define MW_NO_FLAP      1
#define MW_PLAIN_FLAP   2
#define MW_SPLIT_FLAP   3
#define MW_SLOT_FLAP    4
#define MW_FOWLER_FLAP  5

//===== Define Slat Types =====
#define MW_NO_SLAT      1
#define MW_SLAT         2
#define MW_KRUEGER      3
#define MW_LE_FLAP      4

//===== Define Trimming Options =====
#define MW_NO_TRIM         0
#define MW_FLAP_TRIM       1
#define MW_ALL_MOVE_TRIM   2

class Mwing_screen;
class Af;

class WingGeom : public Geom
{
	enum { WING_SURF, IN_FLAP_SURF, OUT_FLAP_SURF, IN_SLAT_SURF, OUT_SLAT_SURF };

   Xsec_surf mwing_surf;

   int curr_af_id;
   Af* root_foil;
   Af* strake_foil;
   Af* aft_foil;
   Af* tip_foil;

   int driver;
   Parm aspect;
   Parm taper;
   Parm area;
   Parm span;
   Parm tip_chord;
   Parm root_chord;

   Parm sweep;
   Parm sweep_loc;
   double old_sweep_loc;

   Parm twist_loc;
   Parm in_twist;
   Parm in_dihed;
   Parm mid_twist;
   Parm mid_dihed;
   Parm out_twist;
   Parm out_dihed;

   int strake_aft_flag;
   Parm strake_sweep;
   Parm strake_span_per;
   Parm aft_ext_sweep;
   Parm aft_ext_span_per;

   Parm total_area;

   //==== Control Surface Parms ====//
   Xsec_surf in_flap_surf;
   Xsec_surf out_flap_surf;
   Xsec_surf in_slat_surf;
   Xsec_surf out_slat_surf;

   int all_move_flag;
   int in_flap_type;
   int out_flap_type;
   int in_slat_type;
   int out_slat_type;

   Parm in_flap_span_in;
   Parm in_flap_span_out;
   Parm in_flap_chord;
   Parm out_flap_span_in;
   Parm out_flap_span_out;
   Parm out_flap_chord;

   Parm in_slat_span_in;
   Parm in_slat_span_out;
   Parm in_slat_chord;
   Parm out_slat_span_in;
   Parm out_slat_span_out;
   Parm out_slat_chord;

   Parm defl_scale;
   Parm twist_scale;

   Stringc deflect_file_name;
   Stringc deflect_name;
   int deflect_flag;

   //===== Aerodynamic Data =====
   double        trimmed_span;

public:

   WingGeom(Aircraft* aptr);
   virtual ~WingGeom();

   virtual void copy( Geom* fromGeom );

   virtual void define_parms();

   virtual void  set_curr_af_id(int val_in)     { curr_af_id = val_in; };
   virtual int   get_curr_af_id()               { return(curr_af_id); };

   virtual void set_num_pnts_all_afs();

   Af* get_af_ptr();
   Af* get_root_af_ptr()                { return(root_foil); }
   Af* get_strake_af_ptr()              { return(strake_foil); }
   Af* get_aft_af_ptr()                 { return(aft_foil); }
   Af* get_tip_af_ptr()                 { return(tip_foil); }

   virtual void  set_driver(int val_in);
   virtual int   get_driver()                   { return(driver); }
   Parm* get_aspect()                   { return(&aspect); }
   Parm* get_taper()                    { return(&taper); }
   Parm* get_mw_area()                  { return(&area); }
   Parm* get_mw_span()                  { return(&span); }
   Parm* get_root_chord()               { return(&root_chord); }
   Parm* get_tip_chord()                { return(&tip_chord); }

   Parm* get_sweep()                    { return(&sweep); }
   Parm* get_sweep_loc()                { return(&sweep_loc); }

   Parm* get_twist_loc()                { return(&twist_loc); }
   Parm* get_in_twist()                 { return(&in_twist); }
   Parm* get_in_dihed()                 { return(&in_dihed); }
   Parm* get_mid_twist()                { return(&mid_twist); }
   Parm* get_mid_dihed()                { return(&mid_dihed); }
   Parm* get_out_twist()                { return(&out_twist); }
   Parm* get_out_dihed()                { return(&out_dihed); }

   Parm* get_strake_sweep()             { return(&strake_sweep); }
   Parm* get_strake_span_per()          { return(&strake_span_per); }
   Parm* get_aft_ext_sweep()            { return(&aft_ext_sweep); }
   Parm* get_aft_ext_span_per()         { return(&aft_ext_span_per); }

   Parm* get_total_area()               { return(&total_area); }

   virtual void  set_strake_aft_flag(int flag_in);
   virtual int   get_strake_aft_flag()          { return( strake_aft_flag ); }


   virtual void  set_all_move_flag(int val_in)  { all_move_flag = val_in; }
   virtual int   get_all_move_flag()            { return(all_move_flag); }
   virtual void  set_in_flap_type(int val_in)   { in_flap_type = val_in; }
   virtual int   get_in_flap_type()             { return(in_flap_type); }
   virtual void  set_out_flap_type(int val_in)  { out_flap_type = val_in; }
   virtual int   get_out_flap_type()            { return(out_flap_type); }
   virtual void  set_in_slat_type(int val_in)   { in_slat_type = val_in; }
   virtual int   get_in_slat_type()             { return(in_slat_type); }
   virtual void  set_out_slat_type(int val_in)  { out_slat_type = val_in; }
   virtual int   get_out_slat_type()            { return(out_slat_type); }

   Parm* get_in_flap_span_in()          { return(&in_flap_span_in); }
   Parm* get_in_flap_span_out()         { return(&in_flap_span_out); }
   Parm* get_in_flap_chord()            { return(&in_flap_chord); }
   Parm* get_in_slat_span_in()          { return(&in_slat_span_in); }
   Parm* get_in_slat_span_out()         { return(&in_slat_span_out); }
   Parm* get_in_slat_chord()            { return(&in_slat_chord); }

   Parm* get_out_flap_span_in()         { return(&out_flap_span_in); }
   Parm* get_out_flap_span_out()        { return(&out_flap_span_out); }
   Parm* get_out_flap_chord()           { return(&out_flap_chord); }
   Parm* get_out_slat_span_in()         { return(&out_slat_span_in); }
   Parm* get_out_slat_span_out()        { return(&out_slat_span_out); }
   Parm* get_out_slat_chord()           { return(&out_slat_chord); }

   Parm* get_defl_scale()               { return(&defl_scale); }
   Parm* get_twist_scale()              { return(&twist_scale); }

   virtual double get_x_quarter_cbar();	       // feet
   virtual double get_inboard_chord_length();   // Root Chord + Strake and Aft Extension, feet
   virtual double get_x_le_root();			   // feet
   virtual double get_strake_chord_length();    // strake chord
   virtual double get_aft_ext_chord_length();   // aft ext chord

   bool strake_chord_exists();
   bool aft_ext_chord_exists();

   // root chord's origin location from aircraft's origin
   virtual double get_x_loc_root_chord(); 
   virtual double get_y_loc_root_chord(); 
   virtual double get_z_loc_root_chord();

   // strake chord's origin location from aircraft's origin
   virtual double get_x_loc_strake_chord();
   virtual double get_y_loc_strake_chord(); 
   virtual double get_z_loc_strake_chord(); 

   // aft extension chord's origin location from aircraft's origin
   virtual double get_x_loc_aft_chord(); 
   virtual double get_y_loc_aft_chord();
   virtual double get_z_loc_aft_chord(); 

   // tip chord's origin location from aircraft's origin
   virtual double get_x_loc_tip_chord();  
   virtual double get_y_loc_tip_chord(); 
   virtual double get_z_loc_tip_chord();  

   virtual void set_depend_parms();

   virtual void  set_deflect_flag(int flag_in);
   virtual int   get_deflect_flag()             { return( deflect_flag ); }
   virtual int read_deflect_file( Stringc file_name );
   herm_curve deflect_curve;
   herm_curve twist_curve;

   virtual Stringc get_deflect_file_name()      { return deflect_name; }

   virtual void load_hidden_surf();
   virtual void load_normals();
   virtual void update_bbox();

   virtual void draw();
   virtual void drawAlpha();
   virtual void generate();
   virtual void regenerate();
   virtual void generate_surf();
   virtual void generate_flap_slat_surfs();
   virtual void write(FILE* dump_file);
   virtual void write(xmlNodePtr node);
   virtual void read(FILE* dump_file);
   virtual void readSimple(FILE* dump_file);
   virtual void read(xmlNodePtr node);

   virtual void write_rhino_file(ON_BinaryFile* archive, ON_3dmObjectAttributes* attributes );
   virtual vector< TMesh* > createTMeshVec();


   void dump_xsec_file(int, FILE*);


   virtual vec3d getAttachUVPos(double u, double v);			
   void acceptScaleFactor();
   void resetScaleFactor();


   //===== Aero Functions =====
   virtual void write_aero_file(FILE* aero_file, int aero_id_num);
   void         set_trimmed_span(double val)             { trimmed_span = val; }
   double        get_trimmed_span()                      { return(trimmed_span); }
   
   //===== Felisa Functions =====   
   virtual int write_felisa_file(int geom_no, FILE* dump_file);
   virtual void write_bco_info(FILE*, int&, int&);
   virtual int get_num_felisa_comps();
   virtual int get_num_felisa_wings();
   virtual int get_num_felisa_line_sources()            { return(10); }
   virtual int get_num_felisa_tri_sources()             { return(6); }
   virtual void write_felisa_line_sources(FILE*);
   virtual void write_felisa_tri_sources(FILE*);
   virtual void write_aero_xsec(FILE* dump_file);

   virtual double get_area();
   virtual double get_span();
   virtual double get_cbar();

   virtual double get_tan_sweep_at(double per_chord_loc);

   virtual void setSymCode(int in_sym);

   virtual void computeCenter();
   virtual void parm_changed(Parm* chg_parm);

   //==== Scaling Methods ====
   virtual void scale();

   virtual vec3d getVertex3d(int surfid, double x, double p, int r);
   virtual void  getVertexVec(vector< VertexID > *vertVec);

   virtual double loadMSWingSectVec( vector< WingSect >& sectVec );

private:

   virtual double get_sweep_at(double per_chord_loc); // in radians
};

#endif
