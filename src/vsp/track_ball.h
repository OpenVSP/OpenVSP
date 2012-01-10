//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//

//******************************************************************************
//    
//   Track Ball
//  
//              C++ Class to handle 3D mouse motion.
//
//   J.R. Gloudemans - 10/5/93
//   Sterling Software
//       Converted to C++
//
//******************************************************************************


#ifndef TRACK_BALL_H
#define TRACK_BALL_H

#define STOP		0
#define TRANSLATE 	1
#define ROTATE	 	2
#define SCALE	 	3
#define SCALE_SLOW	4

class vec3d;


class track_ball
{
  long xorg, yorg;
  long xsize, ysize;
  
  double trans_x, trans_y, trans_z;
  double rvec[4];
  double spinrot[4];
  int quat_add_count;
  long orig_mouse_x, orig_mouse_y;
  double scale_val;
  double init_scale_val;
  double rot_mat[4][4];
  
  double init_x_ang, init_y_ang, init_z_ang; // radians 
  
  void  tball(double e[4], double p1x, double p1y, double p2x, double p2y);
  void  gettracktransform(double mat[4][4]);
  void add_two_quats(double e1[4], double e2[4], double dest[4]);
  void build_Rmatrix(double m[4][4], double e[4]);
  void ax_to_quat(vec3d& a, double phi, double e[4]);
  
  void reinit(double scale, const vec3d& trans, const vec3d& rot_rad);
  
public:

  enum Axis {
    XAXIS,
    YAXIS,
    ZAXIS
  };

  
  track_ball();
  ~track_ball();
  track_ball& operator=(const track_ball& a); 
  void set_init_angle(double x_ang, double y_ang, double z_ang);
  void set_init_scale(double scale_in);
  void zero_trans()		{ trans_x = trans_y = trans_z = 0.0; }
  double get_scale();
  void set_scale( double s )		{ scale_val = s; }
  
  void click(int x, int y);
  void poll(int mstate, int x, int y);
  void transform();
  
  void init(); // initializes scale, translation, and rotation
  void reinit_rotation(); // reinitializes rotation only
  
  void size_window(int xo, int yo, int xs, int ys );
  
  void translate(Axis axis, double value);
  void rotate(Axis axis, double value);

  double get_trans_x() const    { return(trans_x); }
  double get_trans_y() const    { return(trans_y); }
  double get_trans_z() const    { return(trans_z); }

  double get_init_ang_x()		{ return(init_x_ang); }
  double get_init_ang_y()		{ return(init_y_ang); }
  double get_init_ang_z()		{ return(init_z_ang); }

  double get_rvec_x()		{ return( rvec[0] ); }
  double get_rvec_y()		{ return( rvec[1] ); }
  double get_rvec_z()		{ return( rvec[2] ); }
  double get_rvec_w()		{ return( rvec[3] ); }

  void set_trans( double x, double y, double z );
  void set_init_ang( double xang, double yang, double zang );
  void set_rvec( double rx, double ry, double rz, double rw );

  void adj_x( double val )		{ trans_x += val/scale_val;   tball(spinrot,0.01f,-0.1f,0.01f,-0.1f); }
  void adj_y( double val )		{ trans_y += val/scale_val;   tball(spinrot,0.01f,-0.1f,0.01f,-0.1f); }
  void adj_z( double val )		{ trans_z += val/scale_val;   tball(spinrot,0.01f,-0.1f,0.01f,-0.1f); }
  void adj_scale( double val )	{ scale_val += val;			  tball(spinrot,0.01f,-0.1f,0.01f,-0.1f); }

};

#endif
