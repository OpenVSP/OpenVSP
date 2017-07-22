//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//
//////////////////////////////////////////////////////////////////////

#include "utils.H"

/*##############################################################################
#                                                                              #
#                               vector_cross                                   #
#                                                                              #
##############################################################################*/

void vector_cross(float *vec1, float *vec2, float *cross)
{

    cross[0] =  ( vec1[1]*vec2[2] - vec2[1]*vec1[2] );
    cross[1] = -( vec1[0]*vec2[2] - vec2[0]*vec1[2] );
    cross[2] =  ( vec1[0]*vec2[1] - vec2[0]*vec1[1] );

}

/*##############################################################################
#                                                                              #
#                               vector_cross                                   #
#                                                                              #
##############################################################################*/

void vector_cross(double *vec1, double *vec2, double *cross)
{

    cross[0] =  ( vec1[1]*vec2[2] - vec2[1]*vec1[2] );
    cross[1] = -( vec1[0]*vec2[2] - vec2[0]*vec1[2] );
    cross[2] =  ( vec1[0]*vec2[1] - vec2[0]*vec1[1] );

}

/*##############################################################################
#                                                                              #
#                               vector_cross                                   #
#                                                                              #
##############################################################################*/

void vector_cross(long double *vec1, long double *vec2, long double *cross)
{

    cross[0] =  ( vec1[1]*vec2[2] - vec2[1]*vec1[2] );
    cross[1] = -( vec1[0]*vec2[2] - vec2[0]*vec1[2] );
    cross[2] =  ( vec1[0]*vec2[1] - vec2[0]*vec1[1] );

}

/*##############################################################################
#                                                                              #
#                                 vector_dot                                   #
#                                                                              #
##############################################################################*/

float vector_dot(float *vec1, float *vec2)
{

    float dot;

    dot = vec1[0]*vec2[0] + vec1[1]*vec2[1] + vec1[2]*vec2[2];

    return(dot);

}

/*##############################################################################
#                                                                              #
#                                 vector_dot                                   #
#                                                                              #
##############################################################################*/

double vector_dot(double *vec1, double *vec2)
{

    double dot;

    dot = vec1[0]*vec2[0] + vec1[1]*vec2[1] + vec1[2]*vec2[2];

    return(dot);

}

/*##############################################################################
#                                                                              #
#                                 vector_dot                                   #
#                                                                              #
##############################################################################*/

long double vector_dot(long double *vec1, long double *vec2)
{

    long double dot;

    dot = vec1[0]*vec2[0] + vec1[1]*vec2[1] + vec1[2]*vec2[2];

    return(dot);

}

/*##############################################################################
#                                                                              #
#                               zero_int_array                                 #
#                                                                              #
##############################################################################*/

void zero_int_array(int *array, int size)
{

    int i;

    for ( i = 0 ; i <= size ; i++ ) {

       array[i] = 0;

    }

}

/*##############################################################################
#                                                                              #
#                             zero_float_array                                 #
#                                                                              #
##############################################################################*/

void zero_float_array(float *array, int size)
{

    int i;

    for ( i = 0 ; i <= size ; i++ ) {

       array[i] = 0.;

    }

}

/*##############################################################################
#                                                                              #
#                               zero_array                                     #
#                                                                              #
##############################################################################*/

void zero_array(int *array, int size)
{

    int i;

    for ( i = 0 ; i <= size ; i++ ) {

       array[i] = 0;

    }

}

/*##############################################################################
#                                                                              #
#                             zero_array                                       #
#                                                                              #
##############################################################################*/

void zero_array(float *array, int size)
{

    int i;

    for ( i = 0 ; i <= size ; i++ ) {

       array[i] = 0.;

    }

}

/*##############################################################################
#                                                                              #
#                             zero_array                                       #
#                                                                              #
##############################################################################*/

void zero_array(double *array, int size)
{

    int i;

    for ( i = 0 ; i <= size ; i++ ) {

       array[i] = 0.;

    }

}

/*##############################################################################
#                                                                              #
#                             zero_array                                       #
#                                                                              #
##############################################################################*/

void zero_array(long double *array, int size)
{

    int i;

    for ( i = 0 ; i <= size ; i++ ) {

       array[i] = 0.;

    }

}

/*##############################################################################
#                                                                              #
#                              resize_int_array                                #
#                                                                              #
##############################################################################*/

int* resize_int_array(int *array, int size, int new_size)
{

    int i;
    int *new_array;

    // Allocate and zero out new array space

    new_array = new int[new_size + 1];

    for ( i = 0 ; i <= new_size ; i++ ) {

       new_array[i] = 0;

    }

    // Copy over data

    for ( i = 0 ; i <= size ; i++ ) {

       new_array[i] = array[i];

    }

    // Delete old array and move pointer

    delete [] array;

    return new_array;

}

/*##############################################################################
#                                                                              #
#                            resize_float_array                                #
#                                                                              #
##############################################################################*/

float* resize_float_array(float *array, int size, int new_size)
{

    int i;
    float *new_array;

    // Allocate and zero out new array space

    new_array = new float[new_size + 1];

    for ( i = 0 ; i <= new_size ; i++ ) {

       new_array[i] = 0.;

    }

    // Copy over data

    for ( i = 0 ; i <= size ; i++ ) {

       new_array[i] = array[i];

    }

    // Delete old array and move pointer

    delete [] array;

    return new_array;

}

/*##############################################################################
#                                                                              #
#                              in_sphere                                       #
#                                                                              #
##############################################################################*/

void in_sphere(float xyz[4][3], float xyz_in[3], float &radius_in)
{

    float matrix_1[4][4], matrix_2[4][4], rhs[4], det_matrix_1, sub_det[4];
    float norm[4][3];
    int   i, j, l;

    // get outward facing normals for each face of this tet

    get_normals(xyz,norm);

    // pack the matrix equations

    for ( j = 0 ; j <= 2 ; j++ ) {

       matrix_1[0][j] = norm[0][j];
       matrix_1[1][j] = norm[1][j];
       matrix_1[2][j] = norm[2][j];
       matrix_1[3][j] = norm[3][j];

    }

    for ( i = 0 ; i <= 3 ; i++ ) {

       matrix_1[i][3] = 1.;

    }

    // pack the right-handside

    rhs[0] = norm[0][0]*xyz[0][0] + norm[0][1]*xyz[0][1] + norm[0][2]*xyz[0][2];
    rhs[1] = norm[1][0]*xyz[3][0] + norm[1][1]*xyz[3][1] + norm[1][2]*xyz[3][2];
    rhs[2] = norm[2][0]*xyz[3][0] + norm[2][1]*xyz[3][1] + norm[2][2]*xyz[3][2];
    rhs[3] = norm[3][0]*xyz[3][0] + norm[3][1]*xyz[3][1] + norm[3][2]*xyz[3][2];

    // get determinant of original 4x4 matrix

    determinant(matrix_1,&det_matrix_1);

    // solve system of equations using Kramer's rule

    if (  det_matrix_1 != 0. ) {

       for ( l = 0 ; l <= 3 ; l++ ) {

          // grab a copy of the original matrix

          for ( i = 0 ; i <= 3 ; i++ ) {

             for ( j = 0 ; j <= 3 ; j++ ) {

                matrix_2[i][j] = matrix_1[i][j];

             }

          }

          // fill the l'th column with right-hand-side vector

          for ( i = 0 ; i <= 3 ; i++ ) {

             matrix_2[i][l] = rhs[i];

          }

          // solve for determinant of this system

          determinant(matrix_2,&sub_det[l]);

       }

       // solve for the unknowns xyz_in and radius_in

       for ( j = 0 ; j <= 2 ; j++ ) {

          xyz_in[j] = sub_det[j]/det_matrix_1;

       }

       // solve for radius

       radius_in = sub_det[3]/det_matrix_1;

    }

    else {

       radius_in = -1.;

    }

}

/*##############################################################################
#                                                                              #
#                              circ_sphere                                     #
#                                                                              #
##############################################################################*/

void circ_sphere(float xyz[4][3], float xyz_out[3], float &radius_out)
{

    float matrix_1[4][4], matrix_2[4][4], rhs[4], det_matrix_1, sub_det[4];
    int   i, j, l;

    // pack the matrix equations

    for ( j = 0 ; j <= 2 ; j++ ) {

       matrix_1[0][j] = -2.*xyz[0][j];
       matrix_1[1][j] = -2.*xyz[1][j];
       matrix_1[2][j] = -2.*xyz[2][j];
       matrix_1[3][j] = -2.*xyz[3][j];

    }

    for ( i = 0 ; i <= 3 ; i++ ) {

       matrix_1[i][3] = 1.;

    }

    // pack the right-handside

    rhs[0] = -( SQR(xyz[0][0]) + SQR(xyz[0][1]) + SQR(xyz[0][2]) );
    rhs[1] = -( SQR(xyz[1][0]) + SQR(xyz[1][1]) + SQR(xyz[1][2]) );
    rhs[2] = -( SQR(xyz[2][0]) + SQR(xyz[2][1]) + SQR(xyz[2][2]) );
    rhs[3] = -( SQR(xyz[3][0]) + SQR(xyz[3][1]) + SQR(xyz[3][2]) );

    // get determinant of original 4x4 matrix

    determinant(matrix_1,&det_matrix_1);

    // solve system of equations using Kramer's rule

    if ( det_matrix_1 != 0. )  {

       for ( l = 0 ; l <= 2 ; l++ ) {

          // grab a copy of the original matrix

          for ( i = 0 ; i <= 3 ; i++ ) {

             for (j = 0 ; j <= 3 ; j++ ) {

                matrix_2[i][j] = matrix_1[i][j];

             }

          }

          // fill the l'th column with righthandside vector

          for ( i = 0 ; i <= 3 ; i++ ) {

             matrix_2[i][l] = rhs[i];

          }

          // find the determinant of this matrix

          determinant(matrix_2,&sub_det[l]);

       }

       // solve for the unknowns xyz_out[] and the radius squared

       radius_out = 0.;

       for ( j = 0 ; j <= 2 ; j++ ) {

          xyz_out[j] = sub_det[j]/det_matrix_1;

          radius_out = radius_out + SQR((xyz[0][j] - xyz_out[j]));

       }

       // the above radius is radius squared

       radius_out = sqrt(radius_out);

    }

    else {

       radius_out = -1.;

    }

}

/*##############################################################################
#                                                                              #
#                              get_normals                                     #
#                                                                              #
##############################################################################*/

void get_normals(float xyz[4][3], float normals[4][3])
{

    float x1norm, x2norm, x3norm, x4norm, dx[4][4], dy[4][4], dz[4][4];

    // normal vector to face 1-2-3 is normals[0][]

    dx[0][1] = xyz[1][0] - xyz[0][0];
    dy[0][1] = xyz[1][1] - xyz[0][1];
    dz[0][1] = xyz[1][2] - xyz[0][2];

    dx[0][2] = xyz[2][0] - xyz[0][0];
    dy[0][2] = xyz[2][1] - xyz[0][1];
    dz[0][2] = xyz[2][2] - xyz[0][2];

    // form the vector 1-3 cross 1-2, and normalize it1

    normals[0][0] =  ( dy[0][2]*dz[0][1] - dy[0][1]*dz[0][2] );
    normals[0][1] = -( dx[0][2]*dz[0][1] - dx[0][1]*dz[0][2] );
    normals[0][2] =  ( dx[0][2]*dy[0][1] - dx[0][1]*dy[0][2] );

    x1norm = sqrt( SQR(normals[0][0])
                 + SQR(normals[0][1])
                 + SQR(normals[0][2]) );

    normals[0][0] = normals[0][0]/x1norm;
    normals[0][1] = normals[0][1]/x1norm;
    normals[0][2] = normals[0][2]/x1norm;

    // normal vector to face 1-2-4 is normals[1][]

    dx[0][1] = xyz[1][0] - xyz[0][0];
    dy[0][1] = xyz[1][1] - xyz[0][1];
    dz[0][1] = xyz[1][2] - xyz[0][2];

    dx[0][3] = xyz[3][0] - xyz[0][0];
    dy[0][3] = xyz[3][1] - xyz[0][1];
    dz[0][3] = xyz[3][2] - xyz[0][2];

    // form the vector 1-2 cross 1-4, and normalize it1

    normals[1][0] =  ( dy[0][1]*dz[0][3] - dy[0][3]*dz[0][1] );
    normals[1][1] = -( dx[0][1]*dz[0][3] - dx[0][3]*dz[0][1] );
    normals[1][2] =  ( dx[0][1]*dy[0][3] - dx[0][3]*dy[0][1] );

    x2norm = sqrt( SQR(normals[1][0])
                 + SQR(normals[1][1])
                 + SQR(normals[1][2]) );

    normals[1][0] = normals[1][0]/x2norm;
    normals[1][1] = normals[1][1]/x2norm;
    normals[1][2] = normals[1][2]/x2norm;

    // normal vector to face 2-3-4 is normals[2][]

    dx[1][2] = xyz[2][0] - xyz[1][0];
    dy[1][2] = xyz[2][1] - xyz[1][1];
    dz[1][2] = xyz[2][2] - xyz[1][2];

    dx[1][3] = xyz[3][0] - xyz[1][0];
    dy[1][3] = xyz[3][1] - xyz[1][1];
    dz[1][3] = xyz[3][2] - xyz[1][2];

    // form the vector 2-3 cross 2-4, and normalize it1

    normals[2][0] =  ( dy[1][2]*dz[1][3] - dy[1][3]*dz[1][2] );
    normals[2][1] = -( dx[1][2]*dz[1][3] - dx[1][3]*dz[1][2] );
    normals[2][2] =  ( dx[1][2]*dy[1][3] - dx[1][3]*dy[1][2] );

    x3norm = sqrt( SQR(normals[2][0])
                 + SQR(normals[2][1])
                 + SQR(normals[2][2]) );

    normals[2][0] = normals[2][0]/x3norm;
    normals[2][1] = normals[2][1]/x3norm;
    normals[2][2] = normals[2][2]/x3norm;

    // normal vector to face 3-1-4 is normals[3]1

    dx[2][0] = xyz[0][0] - xyz[2][0];
    dy[2][0] = xyz[0][1] - xyz[2][1];
    dz[2][0] = xyz[0][2] - xyz[2][2];

    dx[2][3] = xyz[3][0] - xyz[2][0];
    dy[2][3] = xyz[3][1] - xyz[2][1];
    dz[2][3] = xyz[3][2] - xyz[2][2];

    // form the vector 3-1 cross 3-4, and normalize it1

    normals[3][0] =  ( dy[2][0]*dz[2][3] - dy[2][3]*dz[2][0] );
    normals[3][1] = -( dx[2][0]*dz[2][3] - dx[2][3]*dz[2][0] );
    normals[3][2] =  ( dx[2][0]*dy[2][3] - dx[2][3]*dy[2][0] );

    x4norm = sqrt( SQR(normals[3][0])
                 + SQR(normals[3][1])
                 + SQR(normals[3][2]) );

    normals[3][0] = normals[3][0]/x4norm;
    normals[3][1] = normals[3][1]/x4norm;
    normals[3][2] = normals[3][2]/x4norm;

}

/*##############################################################################
#                                                                              #
#                              determinant                                     #
#                                                                              #
##############################################################################*/

void determinant(float mat[4][4], float *det_matrix)
{

    float det_1[6], det_2[4];

    // find determinants of 2x2 cofactors

    det_1[0] = mat[2][0]*mat[3][1] - mat[2][1]*mat[3][0];
    det_1[1] = mat[2][0]*mat[3][2] - mat[2][2]*mat[3][0];
    det_1[2] = mat[2][0]*mat[3][3] - mat[2][3]*mat[3][0];
    det_1[3] = mat[2][1]*mat[3][2] - mat[2][2]*mat[3][1];
    det_1[4] = mat[2][1]*mat[3][3] - mat[2][3]*mat[3][1];
    det_1[5] = mat[2][2]*mat[3][3] - mat[2][3]*mat[3][2];

    // find determinants of 3x3 cofactors

    det_2[0] = mat[1][0]*det_1[3] - mat[1][1]*det_1[1] + mat[1][2]*det_1[0];
    det_2[1] = mat[1][0]*det_1[4] - mat[1][1]*det_1[2] + mat[1][3]*det_1[0];
    det_2[2] = mat[1][0]*det_1[5] - mat[1][2]*det_1[2] + mat[1][3]*det_1[1];
    det_2[3] = mat[1][1]*det_1[5] - mat[1][2]*det_1[4] + mat[1][3]*det_1[3];

    // find determinant of mat

    *det_matrix = mat[0][0]*det_2[3] - mat[0][1]*det_2[2]
                + mat[0][2]*det_2[1] - mat[0][3]*det_2[0];

}

/*##############################################################################
#                                                                              #
#                                 tri_seg_int                                  #
#                                                                              #
# Find the intersection of a triangle and a line segment                       #
#                                                                              #                                                                              #
##############################################################################*/

int tri_seg_int(float *p1, float *p2, float *p3, float *p4, float *p5, float *tt,
                float *uu, float *ww)
{

    float udir[3], wdir[3], tdir[3], cross[3];
    float u_len, w_len, t_len, min_len, max_len, zero;
    float t_one, u_one, w_one, t_zero, u_zero, w_zero;
    float det1, det2, det3, u, w, t, xn[3], dot;
    int idir;

    // form the direction vectors for triangle and segment edges

    for ( idir = 0 ; idir <= 2 ; idir++ ) {

       udir[idir] = p2[idir] - p1[idir];  // triangle edge vector 1
       wdir[idir] = p3[idir] - p1[idir];  // triangle edge vector 2
       tdir[idir] = p5[idir] - p4[idir];  // edge segment vector

    }

    // get max and min edge lengths

    u_len = sqrt( SQR(udir[0]) + SQR(udir[1]) + SQR(udir[2]) );

    w_len = sqrt( SQR(wdir[0]) + SQR(wdir[1]) + SQR(wdir[2]) );

    t_len = sqrt( SQR(tdir[0]) + SQR(tdir[1]) + SQR(tdir[2]) );

    min_len = MIN3(u_len,w_len,t_len);

    max_len = MAX3(u_len,w_len,t_len);

    // calculate separate tolerances for each parameter

    zero = -0.001;

    t_zero = zero * (max_len / t_len) * (min_len / max_len);
    t_one  = 1. - t_zero;

    u_zero = zero * (max_len / u_len) * (min_len / max_len);
    u_one  = 1. - u_zero;

    w_zero = zero * (max_len / w_len) * (min_len / max_len);
    w_one  = 1. - w_zero;

    // calculate the parameter t for equation of line

    vector_cross(udir,wdir,cross);

    det1 = vector_dot(cross,tdir);

    if ( det1 != 0. ) {

       t = ( vector_dot(cross,p1) - vector_dot(cross,p4) )/det1;

       *tt = t;

       if ( t >= t_zero && t <= t_one ) {

          // calculate first parameter for equation of plane

          vector_cross(wdir,tdir,cross);

          det2 = vector_dot(cross,udir);

          if ( det2 != 0. ) {

             u = ( vector_dot(cross,p4) - vector_dot(cross,p1) )/det2;

             *uu = u;

             if ( u >= u_zero && u <= u_one ) {

                // calculate second parameter for equation of plane

                vector_cross(udir,tdir,cross);

                det3 = vector_dot(cross,wdir);

                if ( det3 != 0. ) {

                   w = ( vector_dot(cross,p4) - vector_dot(cross,p1) )/det3;

                   *ww = w;

                   if ( w >= w_zero && w <= w_one ) {

                      // if u + w <= 1 then point is valid intersection!

                      if ( ( u + w ) <= MAX(u_one,w_one) ) {

                         // now determine which node lies below the plane

                         xn[0] =  ( udir[1]*wdir[2] - wdir[1]*udir[2] );
                         xn[1] = -( udir[0]*wdir[2] - wdir[0]*udir[2] );
                         xn[2] =  ( udir[0]*wdir[1] - wdir[0]*udir[1] );

                         dot = vector_dot(xn,tdir);

                         // node 1 is below plane

                         if ( dot >= 0. ) {

                            return(1);

                         }

                         // node 2 is below plane

                         else {

                            return(2);

                         }

                      }

                   }

                }

             }

          }

       }

    }

    return(0);

}

/*##############################################################################
#                                                                              #
#                                 prl_seg_int                                  #
#                                                                              #
# Find the intersection of a parrallelogram and a line segment                 #
#                                                                              #                                                                              #
##############################################################################*/

int prl_seg_int(float *p1, float *p2, float *p3, float *p4, float *p5, float *tt,
                float *uu, float *ww)
{

    float udir[3], wdir[3], tdir[3], cross[3];
    float u_len, w_len, t_len, min_len, max_len, zero;
    float t_one, u_one, w_one, t_zero, u_zero, w_zero;
    float det1, det2, det3, u, w, t, xn[3], dot;
    int idir;

    // form the direction vectors for triangle and segment edges

    for ( idir = 0 ; idir <= 2 ; idir++ ) {

       udir[idir] = p2[idir] - p1[idir];  // triangle edge vector 1
       wdir[idir] = p3[idir] - p1[idir];  // triangle edge vector 2
       tdir[idir] = p5[idir] - p4[idir];  // edge segment vector

    }

    // get max and min edge lengths

    u_len = sqrt( SQR(udir[0]) + SQR(udir[1]) + SQR(udir[2]) );

    w_len = sqrt( SQR(wdir[0]) + SQR(wdir[1]) + SQR(wdir[2]) );

    t_len = sqrt( SQR(tdir[0]) + SQR(tdir[1]) + SQR(tdir[2]) );

    min_len = MIN3(u_len,w_len,t_len);

    max_len = MAX3(u_len,w_len,t_len);

    // calculate separate tolerances for each parameter

    zero = -0.001;

    t_zero = zero * (max_len / t_len) * (min_len / max_len);
    t_one  = 1. - t_zero;

    u_zero = zero * (max_len / u_len) * (min_len / max_len);
    u_one  = 1. - u_zero;

    w_zero = zero * (max_len / w_len) * (min_len / max_len);
    w_one  = 1. - w_zero;

    // calculate the parameter t for equation of line

    vector_cross(udir,wdir,cross);

    det1 = vector_dot(cross,tdir);

    if ( det1 != 0. ) {

       t = ( vector_dot(cross,p1) - vector_dot(cross,p4) )/det1;

       *tt = t;

       if ( t >= t_zero && t <= t_one ) {

          // calculate first parameter for equation of plane

          vector_cross(wdir,tdir,cross);

          det2 = vector_dot(cross,udir);

          if ( det2 != 0. ) {

             u = ( vector_dot(cross,p4) - vector_dot(cross,p1) )/det2;

             *uu = u;

             if ( u >= u_zero && u <= u_one ) {

                // calculate second parameter for equation of plane

                vector_cross(udir,tdir,cross);

                det3 = vector_dot(cross,wdir);

                if ( det3 != 0. ) {

                   w = ( vector_dot(cross,p4) - vector_dot(cross,p1) )/det3;

                   *ww = w;

                   if ( w >= w_zero && w <= w_one ) {

                      // if made it here, valid intersection!

                      // now determine which node lies below the plane

                      xn[0] =  ( udir[1]*wdir[2] - wdir[1]*udir[2] );
                      xn[1] = -( udir[0]*wdir[2] - wdir[0]*udir[2] );
                      xn[2] =  ( udir[0]*wdir[1] - wdir[0]*udir[1] );

                      dot = vector_dot(xn,tdir);

                      // node 1 is below plane

                      if ( dot >= 0. ) {

                         return(1);

                      }

                      // node 2 is below plane

                      else {

                         return(2);

                      }

                   }

                }

             }

          }

       }

    }

    return(0);

}

/*##############################################################################
#                                                                              #
#                                 compare_boxes                                #
#                                                                              #
# Compares the bounding boxes of two objects for overlap                       #
#                                                                              #                                                                              #
##############################################################################*/

int compare_boxes(BBOX box1, BBOX box2)
{

    float tol_x, tol_y, tol_z;

    tol_x = 0.01*MAX3( ABS(box1.x_max-box1.x_min), ABS(box2.x_max-box2.x_min), 1. );
    tol_y = 0.01*MAX3( ABS(box1.y_max-box1.y_min), ABS(box2.y_max-box2.y_min), 1. );
    tol_z = 0.01*MAX3( ABS(box1.z_max-box1.z_min), ABS(box2.z_max-box2.z_min), 1. );

    if ( (box1.x_min - box2.x_max) > tol_x ) return(0);

    if ( (box2.x_min - box1.x_max) > tol_x ) return(0);

    if ( (box1.y_min - box2.y_max) > tol_y ) return(0);

    if ( (box2.y_min - box1.y_max) > tol_y ) return(0);

    if ( (box1.z_min - box2.z_max) > tol_z ) return(0);

    if ( (box2.z_min - box1.z_max) > tol_z ) return(0);

    return(1);

}

/*##############################################################################
#                                                                              #
#                               asearch                                        #
#                                                                              #
# searches an array then returns the index i such that:                        #
# the input value is inside the interval a(i) and a(i+1)                       #                                                                             *
#                                                                              #
##############################################################################*/

int asearch(float value, const float * array, int dim, float &sa, int &bound)
{
    int i;
    int sign = (array[1] < array[2]) ? 1 : -1;
    i = 0;
    float x = sign*value;
    if (x < sign*array[0])
    {
        sa = 0.0;
        bound = 1;
        return 0;
    }
    while ( i < dim )
    {
       if ( x < sign*array[i] )
       {
           sa = (value-array[i-1]) / (array[i]-array[i-1]);
           bound = 0;
           return (i-1);
       }
       i++;
    }
    bound = 1;
    sa = 1.;
    return (dim-2);
}

