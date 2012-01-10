//******************************************************************************
//
//   Airfoil Class
//
//
//   J.R. Gloudemans - 8/2/93
//   Sterling Software
//
//******************************************************************************

#ifndef AF_H
#define AF_H


#include "sarray.h"
#include "vec3d.h"
#include "stringc.h"
#include "herm_curve.h"
#include "drawBase.h"

#include "xmlvsp.h"

#include "parm.h"

// Airfoil Types
#define NACA_4_SERIES 1
#define BICONVEX      2
#define WEDGE         3
#define AIRFOIL_FILE  4
#define NACA_6_SERIES 5


#define AF_UPDATE_GROUP 2323


//==== Fortran Common Block For 6-Series Airfoils =====//
extern "C"
{ void sixseries_( const int* ser, const float* t, const float* cli, const float* a ); }

extern "C"
{
  extern struct {
	  int nml;
		int nmu;
	  float xxl[1000];
	  float xxu[1000];
	  float yyl[1000];
	  float yyu[1000];
  } sixpnts_;
}


class Geom;

class Af : public DrawBase
{
  Geom* geom_ptr;

  int type;
  int curve_loaded_flag;
  int inverted_flag;

  Parm camber;
  Parm camber_loc;
  Parm thickness;
  Parm thickness_loc;

  Parm radius_le;
  Parm radius_te;
  Parm delta_y_le;

  Parm a;
  Parm ideal_cl;
  int sixser;

  int slat_flag;
  int slat_shear_flag;
  Parm slat_chord;
  Parm slat_angle;

  int flap_flag;
  int flap_shear_flag;
  Parm flap_chord;
  Parm flap_angle;

  Stringc name;

  int num_pnts;
  struct_array<vec3d> pnts;

  float orig_af_thickness;
  float radius_le_correction_factor;
  float radius_te_correction_factor;

  herm_curve upper_curve;
  herm_curve lower_curve;

  void generate_4_series();
  void generate_biconvex();
  void generate_6_series();
  void generate_wedge();
  void compute_zc_zt_theta( float x, float& zc, float& zt, float& theta);
  void load_name_4_series();
  void load_name_6_series();
  void load_name_biconvex();
  void load_name_wedge();
  void generate_from_file_curve();
  void load_curves();
  void load_4_series_curves();
  void load_6_series_curves();
  void load_biconvex_curves();
  void load_wedge_curves();
  float compute_thickness_af_curves();
  float compute_radius_le_af_curves();
  void  invert_airfoil();

	int readVspAirfoil( FILE* file_id, int* thickFlag, int* radiusLeFlag, int* radiusTeFlag );
	int readSeligAirfoil( FILE* file_id );

public:

  Af(Geom* geom_ptr_in);
  ~Af();
    
  Af& operator=(const Af& in_af);

  vector< Parm* > GetLinkableParms();

  void draw();

  void generate_airfoil();
  void load_name();
  void set_geom(Geom* gPtr);

  void  set_type(int type_in);
  int   get_type()              { return(type); }

  void  set_inverted_flag(int in_flag)  { inverted_flag = in_flag; }
  int   get_inverted_flag()             { return(inverted_flag); }

  Parm* get_camber()            { return(&camber); }
  Parm* get_camber_loc()        { return(&camber_loc); }
  // camber distribution 
  //  vertical location of the camber line compared to the nose of the airfoil

  Parm* get_thickness()         { return(&thickness); }
  Parm* get_thickness_loc()     { return(&thickness_loc); }
  Parm* get_ideal_cl()					{ return(&ideal_cl); }
  Parm* get_a()									{ return(&a); }

  void set_sixser( int ser );
  int get_sixser()							{ return (sixser); }

  // thickness distribution
  //  the vertical offset from the mean camber line
  Parm* get_leading_edge_radius()               { return(&radius_le); }
  Parm* get_trailing_edge_radius()              { return(&radius_te); }

  void  set_slat_flag(int in_flag)			{ slat_flag = in_flag; }
  int   get_slat_flag()						{ return(slat_flag); }
  void  set_slat_shear_flag(int in_flag)	{ slat_shear_flag = in_flag; }
  int   get_slat_shear_flag()				{ return(slat_shear_flag); }
  Parm* get_slat_chord()					{ return(&slat_chord); }
  Parm* get_slat_angle()					{ return(&slat_angle); }

  void  set_flap_flag(int in_flag)			{ flap_flag = in_flag; }
  int   get_flap_flag()						{ return(flap_flag); }
  void  set_flap_shear_flag(int in_flag)	{ flap_shear_flag = in_flag; }
  int   get_flap_shear_flag()				{ return(flap_shear_flag); }
  Parm* get_flap_chord()					{ return(&flap_chord); }
  Parm* get_flap_angle()					{ return(&flap_angle); }

  void add_slat_flap();

  int  read_af_file( Stringc file_name );
  Stringc& get_name()           { return(name); }

  float compute_leading_edge_radius();
  float get_delta_y_le();

  void set_num_pnts(int num_pnts_in);

  vec3d& get_pnt(int index)             { return(pnts[index]); }
  vec3d get_end_cap(int index);
  vec3d get_rounded_end_cap( int index );

  void write(FILE* file_id);
  void write(xmlNodePtr node);
  void read(FILE* file_id);
  void read(xmlNodePtr node);
  void write_aero_points(FILE* file_id);

  void init_script(Stringc s);

  void sterilize_parms();
};

#endif
