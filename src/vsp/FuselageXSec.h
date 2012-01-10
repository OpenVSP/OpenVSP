//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//

//******************************************************************************
//
//   Fusealge Cross-Section 2
//
//   J.R. Gloudemans - 8/18/2009
//
//******************************************************************************
#ifndef FUSELAGEXSEC_H
#define FUSELAGEXSEC_H

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

#define FXS_POINT   0
#define FXS_CIRCLE  1
#define FXS_ELLIPSE 2
#define FXS_BOX 3
#define FXS_RND_BOX 4
#define FXS_GENERAL 5
#define FXS_FROM_FILE 6
#define FXS_EDIT_CRV 7

#define PNT_SPACE_PER_XSEC 0
#define PNT_SPACE_FIXED    1
#define PNT_SPACE_UNIFORM  2

class FuselageGeom;

class FuselageXSec : public DrawBase , public GeomBase
{
public:
  FuselageXSec(Geom* geom_ptr_in = 0);
  ~FuselageXSec();
//  FuselageXSec& operator=(const FuselageXSec& in_fuse_xsec);

  void copy( FuselageXSec & ifx );

  void set_fuse_ptr( Geom* geom_ptr_in );

  void parm_changed(Parm* p);

  void draw();

  double getRollerScaleFactor()						{ return rollerScaleFactor; }
  void   setDrawScaleFactor( double sf );

  void regenerate();

  int get_num_pnts()                    { return(num_pnts); }
  void set_num_pnts(int num_pnts_in);

  void set_type(int type_in);       
  int  get_type()                       { return(xstype); }

  void set_height( float h )			{ height = h; }
  void set_width(  float w )			{ width = w; }
  
  Parm* get_height()					{ return &(height); }
  Parm* get_width()						{ return &(width); }
  Parm* get_max_width_loc()				{ return &(max_width_loc); }
  Parm* get_corner_rad()				{ return &(corner_rad); }
  Parm* get_top_tan_angle()				{ return &(top_tan_angle); }
  Parm* get_bot_tan_angle()				{ return &(bot_tan_angle); }
  Parm* get_top_str()					{ return &(top_str); }
  Parm* get_upp_str()					{ return &(upp_str); }
  Parm* get_bot_str()					{ return &(bot_str); }
  Parm* get_low_str()					{ return &(low_str); }	
  Parm* get_y_offset()					{ return &(y_offset); }
  Parm* get_z_offset()					{ return &(z_offset); }
  Parm* get_location()					{ return &(location); }

  int  getAllTanStrFlag()				{ return allTanStrFlag; }
  void setAllTanStrFlag( int f );

  //==== Profile Tangent Str ====//
  Parm* getTopTanAng()					{ return &topTanAng; }
  Parm* getTopTanStr1()					{ return &topTanStr1; }
  Parm* getTopTanStr2()					{ return &topTanStr2; }
  Parm* getBotTanAng()					{ return &botTanAng; }
  Parm* getBotTanStr1()					{ return &botTanStr1; }
  Parm* getBotTanStr2()					{ return &botTanStr2; }
  Parm* getLeftTanAng()					{ return &leftTanAng; }
  Parm* getLeftTanStr1()				{ return &leftTanStr1; }
  Parm* getLeftTanStr2()				{ return &leftTanStr2; }
  Parm* getRightTanAng()				{ return &rightTanAng; }
  Parm* getRightTanStr1()				{ return &rightTanStr1; }
  Parm* getRightTanStr2()				{ return &rightTanStr2; }

  void set_all_str(float val)			{ top_str = upp_str = low_str = bot_str = val; }
  double get_all_str()					{ return(top_str()); }

  void set_top_sym( int f )				{ topSymFlag = f; }
  int  get_top_sym( )					{ return topSymFlag; }
  void set_side_sym( int f )			{ sideSymFlag = f; }
  int  get_side_sym( )					{ return sideSymFlag; }
  void setShapeSymmetry();

  void write(xmlNodePtr node);
  void write_parms(xmlNodePtr node );
  void read(xmlNodePtr node);
  void read_parms(xmlNodePtr node );

  void generate();
  void gen_parms();
  void set_location_limits( double loc_min, double loc_max )
											{ location.set_lower_upper(loc_min, loc_max); }

  void set_loc_on_spine(double loc_in)		{ location.set( loc_in ); }
  double get_loc_on_spine()					{ return(location());  }
  void set_pnt_on_spine(const vec3d& offset);
  vec3d& get_pnt_on_spine()					{ return(pnt_on_spine); }
 
  vec3d get_pnt(int index);
  struct_array<vec3d> get_pnts()			{ return pnts; }

  double linear_interp( int imin, int ival, int imax, double vmin, double vmax );
  double cubic_interp( int imin, int ival, int imax, double vmin, double vmax );
  double cosine_interp( int imin, int ival, int imax, double vmin, double vmax );
  double get_tan_ang( int index );
  double get_tan_str1( int index );
  double get_tan_str2( int index );

  void compute_pnt_distribution(int& num_top, int& num_circle, int& num_bot);
  void set_pnt_space_type( int type )		{ pntSpaceType = type;  }
  int  get_pnt_space_type()					{ return pntSpaceType; }
  void load_pnts( );
  void load_uniform_pnts( );

  void load_file_pnts( );
  void load_edit_crv_pnts( );
  void load_box_pnts( );

  int  read_xsec_file( Stringc file_name );
  herm_curve* get_file_crv()							{ return &file_crv; }
  void set_file_crv( herm_curve* crv );
  void interp_file_crv( double fract, herm_curve* crv0, herm_curve* crv1 );
 
  EditCurve* getEditCrv()									{ return &edit_crv; }
  void setEditCrv( EditCurve* crv );

  double computeArea();

  //==== Number of Interpolated Sections Next to This One ====//
  int getNumSectInterp1()									{ return numSectInterp1; }
  int getNumSectInterp2()									{ return numSectInterp2; }
  void setNumSectInterp1(int val)							{ numSectInterp1 = val; }
  void setNumSectInterp2(int val)							{ numSectInterp2 = val; }

protected:
  Geom* geom_ptr;

  vec3d pnt_on_spine;
 
  int num_pnts;
  int xstype;					// Point, Circle....

  Parm height;
  Parm width;
  Parm max_width_loc;
  Parm corner_rad;
  Parm top_tan_angle;
  Parm bot_tan_angle;
  Parm top_str;
  Parm upp_str;
  Parm low_str;
  Parm bot_str;
  Parm y_offset;
  Parm z_offset;
  Parm location;

  Parm updateFuse;

  Parm topTanAng;				// Profile Shape
  Parm topTanStr1;
  Parm topTanStr2;
  Parm botTanAng;				
  Parm botTanStr1;
  Parm botTanStr2;
  Parm leftTanAng;				
  Parm leftTanStr1;
  Parm leftTanStr2;
  Parm rightTanAng;				
  Parm rightTanStr1;
  Parm rightTanStr2;

  int topSymFlag;
  int sideSymFlag;

  int allTanStrFlag;

  herm_curve top_crv;
  herm_curve bot_crv;

  Stringc fileName;
  herm_curve file_crv;

  EditCurve edit_crv;

  struct_array<vec3d> pnts;
  int pntSpaceType;

  double rollerScaleFactor;
  double drawScaleFactor;

  //==== Num of Interpolated Sections Next to This One ====//
  int numSectInterp1;		
  int numSectInterp2;		
 
};

#endif
