//    
//   Track Ball
//  
//              C++ Class to handle 3D mouse motion.
//
//   J.R. Gloudemans - 10/5/93
//   Sterling Software
//

#ifdef WIN32
#include <windows.h>		
#endif

#ifdef __APPLE__
#  include <OpenGL/gl.h>
#else
#  include <GL/gl.h>
#endif

#include <math.h>

#include <stdio.h> 

extern "C" {
#include "trackball.h"
}

#include "track_ball.h"
#include "vec3d.h"

#ifndef M_SQRT1_2
#define M_SQRT1_2 (0.707106781)
#endif

#ifndef M_SQRT2
#define M_SQRT2  (1.414213462)
#endif

//===== Constructor  =====//
track_ball::track_ball()
{
  init_scale_val = 1.0;
  init_x_ang = init_y_ang = init_z_ang = 0.0;
  orig_mouse_x = orig_mouse_y = 0;
  quat_add_count = 0;
  init();
}

//===== Destuctor  =====//
track_ball::~track_ball()
{
}

//
track_ball& track_ball::operator=(const track_ball& a)  
{
  xorg = a.xorg;
  yorg = a.yorg;
  xsize = a.xsize;
  ysize = a.ysize;
  trans_x = a.trans_x;
  trans_y = a.trans_y;
  trans_z = a.trans_z;
  quat_add_count = a.quat_add_count;
  orig_mouse_x = a.orig_mouse_x;
  orig_mouse_y = a.orig_mouse_y;
  scale_val = a.scale_val;
  init_scale_val = a.init_scale_val;
  init_x_ang = a.init_x_ang;
  init_y_ang = a.init_y_ang;
  init_z_ang = a.init_z_ang;
  for (int i = 0 ; i < 4 ; i++)
    {
      rvec[i] = a.rvec[i]; 
      spinrot[i] = a.spinrot[i];
      for ( int j = 0 ; j < 4 ; j++ )
        {
          rot_mat[i][j] = a.rot_mat[i][j];
        }
    }
  return *this;
}

//===== Set Initial Viewing Angles  =====//
void track_ball::set_init_angle(double x_ang, double y_ang, double z_ang)
{
  init_x_ang = x_ang;
  init_y_ang = y_ang;
  init_z_ang = z_ang;
}

//===== Set Initial Scale  =====//
void track_ball::set_init_scale(double scale_in)
{
  init_scale_val = scale_in;
  scale_val = init_scale_val;
}


//===== Get Current Scale  =====//
double track_ball::get_scale()
{
  return(scale_val);
}

//===== Record Position Of Mouse At Click ====//    
void track_ball::click(int x, int y)
{
  orig_mouse_x = x;
  orig_mouse_y = y;
}

//===== Record Position Of Mouse At Click ====//    
void track_ball::poll(int mstate, int x, int y)
{
    long mx, my;
    long dx, dy, del;
    double x1, y1, x2, y2;
    double r[4];

 
    //==== Get Current Mouse Loc And Compute Delta ====//
    mx = x;
    my = y;

    dx = mx-orig_mouse_x;
    dy = my-orig_mouse_y;

     switch(mstate) {
	case STOP:      //=== Stop Spin ===//
            tball(spinrot,0.01f,-0.1f,0.01f,-0.1f);
	    break;
	case TRANSLATE:      //=== Translate ===//
	    trans_x += (double)dx/((double)xsize*scale_val);
	    trans_y += (double)dy/((double)xsize*scale_val);
            tball(spinrot,0.01f,-0.1f,0.01f,-0.1f);
	    break;
	case ROTATE:
	    x1 = (2.0*(double)(orig_mouse_x-xorg)/xsize)-1.0;
	    y1 = (2.0*(double)(orig_mouse_y-yorg)/ysize)-1.0;
	    x2 = (2.0*(double)( mx-xorg)/xsize)-1.0;
	    y2 = (2.0*(double)( my-yorg)/ysize)-1.0;
 	    tball(r,x1, y1,x2, y2);
   			spinrot[0] = r[0];
    	    spinrot[1] = r[1];
    	    spinrot[2] = r[2];
    	    spinrot[3] = r[3];
	    break;
	case SCALE:
	    del = dy;
	    scale_val -= 0.01*(double)del*init_scale_val; 
            if (scale_val <= 0.0) scale_val = 0.00001;
            tball(spinrot,0.01,-0.1,0.01,-0.1);
	    break;
	case SCALE_SLOW:
	    del = dx+dy;
	    scale_val += 0.001*(double)del*init_scale_val; 
            if (scale_val <= 0.0) scale_val = 0.00001;
            tball(spinrot,0.01,-0.1,0.01,-0.1);
	    break;
      }
    orig_mouse_x = mx;
    orig_mouse_y = my;
}


//===== Record Position Of Mouse At Click ====//    
void track_ball::transform()
{
    double m[4][4];

    //==== Get Current Transformation ====//
    gettracktransform(m);
    glMultMatrixd( (GLdouble*)m);
}

//===== Initialize Trackball ====//    
void track_ball::size_window(int xo, int yo, int xs, int ys )
{
  xorg = xo;
  yorg = yo;
  
  
  xsize = xs;
  ysize = ys;
}


//===== Initialize Trackball ====//    
void track_ball::init()
{
    reinit( init_scale_val, vec3d(0.,0.,0.), vec3d(init_x_ang, init_y_ang, init_z_ang) );
}

//===== Reinitialize Trackball ====//
void track_ball::reinit(double scale, const vec3d& trans, const vec3d& rot_rad)
{
    double r[4];

    scale_val = scale;
    trans_x = trans.x();
    trans_y = trans.y();
    trans_z = trans.z();
    rvec[0] = rvec[1] = rvec[2] = 0.0;  rvec[3] = 1.0;

    vec3d a = vec3d(1.0, 0.0, 0.0);
    ax_to_quat(a, rot_rad.x(), r);
    add_two_quats(r, rvec, rvec);

    a = vec3d(0.0, 1.0, 0.0);
    ax_to_quat(a, rot_rad.y(), r);
    add_two_quats(r, rvec, rvec);

    a = vec3d(0.0, 0.0, 1.0);
    ax_to_quat(a, rot_rad.z(), r);
    add_two_quats(r, rvec, rvec);
    
    spinrot[0] = spinrot[1] = spinrot[2] = 0.0; spinrot[3] = 1.0;
}

//===== Reinitialize Trackball Rotation ====//
void track_ball::reinit_rotation()
{
    reinit( scale_val, vec3d(trans_x, trans_y, trans_z), vec3d(init_x_ang, init_y_ang, init_z_ang) );
}


//===== Trackball ====//    
void track_ball::tball(double e[4], double p1x, double p1y, double p2x, double p2y)
{
// TODO
}

//===== Given an axis and angle, compute quaternion. ====//
void track_ball::ax_to_quat(vec3d& a, double phi, double e[4])
{
// TODO
}


//===== Compute and Load Current Tranformation Matrix ====//
void track_ball::gettracktransform(double mat[4][4])
{
    add_two_quats(spinrot,rvec,rvec);
    build_Rmatrix(mat,rvec);
}
 
//===== Compute and Load Current Tranformation Matrix ====//
void track_ball::add_two_quats(double e1[4], double e2[4], double dest[4])
{
// TODO
}

//===== Compute and Load Current Tranformation Matrix ====//
void track_ball::build_Rmatrix(double m[4][4], double e[4])
{
// TODO
}

void track_ball::translate(Axis axis, double value) 
{
  switch(axis) {
    case XAXIS:
      trans_x = value*10;
     // trans_y = 0;  //commented out to allow x and y translation to occur at the same time
      tball(spinrot,0.01,-0.1,0.01,-0.1);
      break;
    case YAXIS:
     // trans_x = 0;  //commented out to allow x and y translation to occur at the same time
      trans_y = value*10;
      tball(spinrot,0.01,-0.1,0.01,-0.1);
      break;
    case ZAXIS:
      scale_val = value;
      tball(spinrot,0.01,-0.1,0.01,-0.1);
      break;
  }
}


void track_ball::rotate(Axis axis, double value) 
{
  double phi;
  vec3d ax;
  double e[4];

  if ( !value ) 
    return;

  switch(axis) {
    case XAXIS:
      ax.set_xyz(1.0, 0.0, 0.0);
      break;
    case YAXIS:
      ax.set_xyz(0.0, 1.0, 0.0);
      break;
    case ZAXIS:
      ax.set_xyz(0.0, 0.0, 1.0);
      break;
  }
  phi = (value + 1.0)/2.1;

  ax_to_quat(ax, phi, e);
  spinrot[0] = e[0];
  spinrot[1] = e[1];
  spinrot[2] = e[2];
  spinrot[3] = e[3];
}

void track_ball::set_trans( double x, double y, double z )
{
	trans_x = x;
	trans_y = y;
	trans_z = z;
}

void track_ball::set_init_ang( double xang, double yang, double zang )
{
	init_x_ang = xang;
	init_y_ang = yang;
	init_z_ang = zang;
}

void track_ball::set_rvec( double rx, double ry, double rz, double rw )
{
	rvec[0] = rx;
	rvec[1] = ry;
	rvec[2] = rz;
	rvec[3] = rw;
}
