//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//

//******************************************************************************
//    
//   Cross Section Surface Geometry Class
//  
// 
//   J.R. Gloudemans - 10/16/94
//   Sterling Software
//
//******************************************************************************

#ifndef XSEC_SURF_H
#define XSEC_SURF_H

#ifdef WIN32
#include <windows.h>		
#endif

#ifdef __APPLE__
#  include <OpenGL/gl.h>
#else
#  include <GL/gl.h>
#endif

#pragma warning(disable:4786)

#include <stdio.h>
#include <math.h>

#include "array.h"
#include "array_2d.h"
#include "vec3d.h"
#include "vec2d.h"
#include "util.h"
#include "textureMgr.h"

#include <vector>
//using namespace std;


class Material;
class ON_BinaryFile;
class ON_3dmObjectAttributes;
class bezier_surf;
class TMesh;


class Xsec_surf
{

private:
 
protected:
 
   int num_xsecs;
   int num_pnts;

   array_2d<vec3d> pnts_xsecs;
   array_2d<vec3d> hidden_surf;
   array_2d<vec3d> normals;
   array< double > uArray;
   array< double > wArray;

   int refl_pnts_xsecs_code;
   int refl_hidden_code;
   int refl_normals_code;
   array_2d<vec3d> refl_pnts_xsecs;
   array_2d<vec3d> refl_hidden_surf;
   array_2d<vec3d> refl_normals;
   
   int fast_draw_flag;
   int highlight_xsec_flag;
   int highlight_xsec_id;
   vec3d highlight_xsec_color;

   //==== Flags to Indicate Tangent Type ====//
   array< int > pnt_tan_flags;
   array< int > xsec_tan_flags;

   bool draw_flag;


public:

   Xsec_surf();
   virtual ~Xsec_surf();

   void set_num_xsecs(int num_in);
   void set_num_pnts(int  num_in);
   int get_num_xsecs()			{ return(num_xsecs); }
   int get_num_pnts()			{ return(num_pnts); }

   void rotate_xsec_x(int index, double deg);  
   void rotate_xsec_y(int index, double deg);  
   void rotate_xsec_z(int index, double deg); 

   void rotate_xsec_z_zero_x(int index, double deg); 
 
   void scale_xsec_x(int index, double scale);  
   void scale_xsec_y(int index, double scale);  
   void scale_xsec_z(int index, double scale); 
   void offset_xsec_x(int index, double offset);  
   void offset_xsec_y(int index, double offset);  
   void offset_xsec_z(int index, double offset); 

   void set_pnt(int ixs, int ipnt, const vec3d& v_in)	
      { pnts_xsecs(ixs,ipnt) = v_in; }
   vec3d get_pnt(int ixs, int ipnt)	{ return pnts_xsecs(ixs,ipnt); }

   void get_xsec( int ixs, vector< vec3d > & pvec );
   void get_stringer( int ipnt, vector < vec3d > & pvec );

   vec3d comp_uv_pnt( double u, double v );

   vec3d get_xsec_center( int ixs );
   double get_max_dist_from_xsec( int ixs, const vec3d& pnt_in); 

   vec3d linear_interpolate_xsec(int ixs, double fract);

   void copy_xsec( Xsec_surf* fromSurf, int fromXs, int toXs );

   void load_hidden_surf();
   void load_normals();
   void load_sharp_normals();
   void load_uw();

   void set_sym_code( int c )				{ refl_pnts_xsecs_code = c; }
   void load_refl_pnts_xsecs();
   void load_refl_hidden_surf();
   void load_refl_normals();

   void set_highlight_xsec_id( int id )		{ highlight_xsec_id  = id; }
   void set_highlight_xsec_flag( int flag )	{ highlight_xsec_flag  = flag; }
   void set_highlight_xsec_color( vec3d c )	{ highlight_xsec_color = c; }

   void draw_wire();
   void draw_hidden(); 
   void draw_shaded(); 
   void draw_texture( AppliedTex& atex );
   void draw_refl_texture( AppliedTex& atex, int sym_code_in );

   void remap_texture( double u, double width, bool wrapFlag, vector< double > & valVec, 
						vector< int > & indexVec, vector< double > & valRemap );

   void draw_refl_wire( int sym_code_in ); 
   void draw_refl_hidden( int sym_code_in ); 
   void draw_refl_shaded( int sym_code_in ); 

   void write(xmlNodePtr node);
   void read(xmlNodePtr node);

   void write_xsec( int index, float mat[4][4], FILE* file_id, double scale_factor = 1. );
   void write_refl_xsec( int sym_code_in, int index, float mat[4][4], FILE* file_id, double scale_factor = 1.);

   void write_pov_smooth_tris( int sym_code_in, float mat[4][4], float refl_mat[4][4], FILE* file_id);
   void write_stl_tris(int sym_code_in, float mat[4][4], float refl_mat[4][4], FILE* file_id); 
   void write_rhino_file(int sym_code_in, float mat[4][4], float refl_mat[4][4], 
						ON_BinaryFile* archive, ON_3dmObjectAttributes* attributes );
   void write_split_rhino_file( int sym_code_in, float mat[4][4], float refl_mat[4][4], 
						   vector <int> & split_u, vector <int> & split_w, bool singleCompFlag, 
						   ON_BinaryFile* archive, ON_3dmObjectAttributes* attributes );
   void write_bezier_file( FILE* file_id, int sym_code_in, float mat[4][4], float refl_mat[4][4], 
						   vector <int> & split_u, vector <int> & split_w, bool singleCompFlag = false );


   void write_rhino_nurbs_surface( vector< vector< vec3d > > & control_vec,
	   ON_BinaryFile* archive, ON_3dmObjectAttributes* attributes ); 

   void fast_draw_on()                          { fast_draw_flag = 1; };
   void fast_draw_off()                         { fast_draw_flag = 0; };

   void load_bezier_surface( bezier_surf* surf );

   TMesh* createTMesh(float mat[4][4]);
   TMesh* createReflTMesh(int sym_code_in, float refl_mat[4][4]);

   //==== Flags to Indicate Tangent Type ====//
   void clear_pnt_tan_flags();
   void clear_xsec_tan_flags();
   void set_pnt_tan_flag( int ind, int flag );
   int  get_pnt_tan_flag( int ind );
   void set_xsec_tan_flag( int ind, int flag );
   int  get_xsec_tan_flag( int ind );

//   void get_vertex_vec(std::vector<vec3d> * vec);
//   vec3d get_vertex(int i);
   vec3d get_vertex(double x, double p, int r);
//   vec3d get_refl_vertex(int x, int p);

   void set_draw_flag( bool flag )				{ draw_flag = flag; }
   bool get_draw_flag( )						{ return draw_flag; }
 
};   


#endif  



