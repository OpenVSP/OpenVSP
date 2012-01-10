//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//

//******************************************************************************
//
//   Fusealge Cross-Section
//
//
//   J.R. Gloudemans - 10/17/93
//   Sterling Software
//
//******************************************************************************



#ifndef FUSE_XSEC_H
#define FUSE_XSEC_H


#include "array.h"
#include "vec3d.h"
#include "herm_curve.h"
#include "stringc.h"
#include "util.h"
#include "drawBase.h"
#include "editCurve.h"
#include "geom.h"

#include <libxml/xmlmemory.h>
#include <libxml/parser.h>
#include <libxml/xpath.h>
#include "xmlvsp.h"


#define XSEC_POINT   0
#define CIRCLE  1
#define ELLIPSE 2
#define RND_BOX 3
#define GENERAL 4
#define FROM_FILE 5
#define EDIT_CRV 6

#define OML 0
#define IML 1

#define PNT_SPACE_PER_XSEC 0
#define PNT_SPACE_FIXED    1
#define PNT_SPACE_UNIFORM  2

class FuseGeom;

class fuse_xsec : public DrawBase , public GeomBase
{
  Geom* geom_ptr;

  vec3d pnt_on_spine;
  vec3d iml_pnt_on_spine;

  int imlFlag;					// Iml On/Off
  int interpThickFlag;
  Parm topThick;
  Parm botThick;
  Parm sideThick;

  double actTopThick;
  double actBotThick;
  double actSideThick;
  double imlXOff;
  double imlZOff;

  int num_pnts;
  int mlType;       			// OML, IML
  int xstype[2];					// Point, Circle....

  Parm height[2];
  Parm width[2];
  Parm max_width_loc[2];
  Parm corner_rad[2];
  Parm top_tan_angle[2];
  Parm bot_tan_angle[2];
  Parm top_str[2];
  Parm upp_str[2];
  Parm low_str[2];
  Parm bot_str[2];
  Parm z_offset;
  Parm location;

  Parm updateFuse;

  Parm profileTanStr1;			// Profile Stuff
  Parm profileTanStr2;
  Parm profileTanAng;

  int allTanStrFlag[2];

  herm_curve top_crv[2];
  herm_curve bot_crv[2];

  Stringc fileName;
  herm_curve file_crv[2];

  EditCurve edit_crv[2];

  struct_array<vec3d> pnts[2];
  int pntSpaceType;

  double rollerScaleFactor;
  double drawScaleFactor;

  //==== Num of Interpolated Sections Next to This One ====//
  int numSectInterp1;		
  int numSectInterp2;		

public:
  fuse_xsec(Geom* geom_ptr_in = 0);
  ~fuse_xsec();
  fuse_xsec& operator=(const fuse_xsec& in_fuse_xsec);

  void copy( fuse_xsec & ifx );

  void set_fuse_ptr( Geom* geom_ptr_in );

  void parm_changed(Parm* p);

  void draw();
  void set_ml_type( int ml_type_in );
  int get_ml_type();

  void set_iml_flag( int onoff );
  int  get_iml_flag()								{ return imlFlag; }
  void set_iml_z_offset( double zo)					{ imlZOff = zo; }
  double get_iml_z_offset()							{ return imlZOff; }
  void set_iml_x_offset( double xo)					{ imlXOff = xo; }

  void thickChange();
  Parm* get_top_thick()								{ return &(topThick); }
  Parm* get_bot_thick()								{ return &(botThick); }
  Parm* get_side_thick()							{ return &(sideThick); }

//  void set_top_thick( double t )					{ topThick = t; thickChange();  }
//  double get_top_thick()							{ return topThick; }
//  void set_bot_thick( double t )					{ botThick = t; thickChange(); }
//  double get_bot_thick()							{ return botThick; }
//  void set_side_thick( double t )					{ sideThick = t; thickChange(); }
//  void zero_thick()									{ topThick =  botThick = sideThick = 0.0; }

  void set_interp_thick_flag()						{ interpThickFlag = 1; }
//  double get_side_thick()							{ return sideThick; }
  double get_act_top_thick()						{ return actTopThick; }
  double get_act_bot_thick()						{ return actBotThick; }
  double get_act_side_thick()						{ return actSideThick; }

  double getRollerScaleFactor()						{ return rollerScaleFactor; }
  void   setDrawScaleFactor( double sf );

//  void set_fuse_ptr(FuseGeom* fuse_in)      { curr_fuse = fuse_in; }
  void regenerate();

  int get_num_pnts()                    { return(num_pnts); }
  void set_num_pnts(int num_pnts_in);

  void set_type(int type_in);       
  int  get_type()                       { return(xstype[mlType]); }

  void set_height( float h )			{ height[mlType] = h; }
  void set_width(  float w )			{ width[mlType] = w; }
  
  Parm* get_height()					{ return &(height[mlType]); }
  Parm* get_width()						{ return &(width[mlType]); }
  Parm* get_max_width_loc()				{ return &(max_width_loc[mlType]); }
  Parm* get_corner_rad()				{ return &(corner_rad[mlType]); }
  Parm* get_top_tan_angle()				{ return &(top_tan_angle[mlType]); }
  Parm* get_bot_tan_angle()				{ return &(bot_tan_angle[mlType]); }
  Parm* get_top_str()					{ return &(top_str[mlType]); }
  Parm* get_upp_str()					{ return &(upp_str[mlType]); }
  Parm* get_bot_str()					{ return &(bot_str[mlType]); }
  Parm* get_low_str()					{ return &(low_str[mlType]); }	
  Parm* get_z_offset()					{ return &(z_offset); }
  Parm* get_location()					{ return &(location); }

  int  getAllTanStrFlag()				{ return allTanStrFlag[mlType]; }
  void setAllTanStrFlag( int f );

  //==== Profile Tangent Str ====//
  Parm* getProfileTanStr1()				{ return &profileTanStr1; }
  Parm* getProfileTanStr2()				{ return &profileTanStr2; }
  Parm* getProfileTanAng()				{ return &profileTanAng; }

  void set_all_str(float val)
		{ top_str[mlType] = upp_str[mlType] = low_str[mlType] = bot_str[mlType] = val; }
  double get_all_str()					{ return(top_str[mlType]()); }

  void write(FILE* dump_file);
  void write_parms(FILE* file_id, int mltype );
  void read(FILE* dump_file);
  void read_parms(FILE* file_id, int mltype );

  void write(xmlNodePtr node);
  void write_parms(xmlNodePtr node, int mltype );
  void read(xmlNodePtr node);
  void read_parms(xmlNodePtr node, int mltype );

  void generate();
  void gen_parms();
  void set_location_limits( double loc_min, double loc_max )
											{ location.set_lower_upper(loc_min, loc_max); }

//  void set_location(double loc_in)			{ location.set( loc_in ); }
  void set_loc_on_spine(double loc_in)		{ location.set( loc_in ); }
//  void set_loc_on_spine(double loc_in)		{ loc_on_spine = loc_in; }
//  double get_loc_on_spine()					{ return(loc_on_spine);  }
  double get_loc_on_spine()					{ return(location());  }
  void set_pnt_on_spine(const vec3d& offset);

  vec3d& get_pnt_on_spine()					{ return(pnt_on_spine); }
  vec3d& get_iml_pnt_on_spine();

  vec3d get_pnt(int index);
  vec3d get_pnt(int ml_type, int index);

  struct_array<vec3d> get_pnts()			{ return pnts[mlType]; }

  void compute_pnt_distribution(int mlt, int& num_top, int& num_circle, int& num_bot);
  void set_pnt_space_type( int type )		{ pntSpaceType = type;  }
  int  get_pnt_space_type()					{ return pntSpaceType; }
  void load_pnts( int mlt );
  void load_uniform_pnts( int mlt );
  void load_file_pnts(int mlt );

  void load_edit_crv_pnts( int mlt );

  int  read_xsec_file( Stringc file_name );
  herm_curve* get_file_crv(int mlt)							{ return &file_crv[mlt]; }
  void set_file_crv( herm_curve* crv );
  void interp_file_crv( double fract, herm_curve* crv0, herm_curve* crv1 );
 
  EditCurve* getEditCrv()									{ return &edit_crv[mlType]; }
  EditCurve* getEditCrv(int mlt)							{ return &edit_crv[mlt]; }
  void setEditCrv( EditCurve* crv );						

  void debug_print();

  //==== Number of Interpolated Sections Next to This One ====//
  int getNumSectInterp1()									{ return numSectInterp1; }
  int getNumSectInterp2()									{ return numSectInterp2; }
  void setNumSectInterp1(int val)							{ numSectInterp1 = val; }
  void setNumSectInterp2(int val)							{ numSectInterp2 = val; }

  double computeArea();
 
};

#endif
