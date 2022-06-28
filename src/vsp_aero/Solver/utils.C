//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//
//////////////////////////////////////////////////////////////////////

#include "utils.H"

#include "START_NAME_SPACE.H"

/*##############################################################################
#                                                                              #
#                               vector_cross                                   #
#                                                                              #
##############################################################################*/

void vector_cross(VSPAERO_DOUBLE *vec1, VSPAERO_DOUBLE *vec2, VSPAERO_DOUBLE *cross)
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

VSPAERO_DOUBLE vector_dot(VSPAERO_DOUBLE *vec1, VSPAERO_DOUBLE *vec2)
{

    VSPAERO_DOUBLE dot;

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
#                             zero_double_arra                                 #
#                                                                              #
##############################################################################*/

void zero_double_array(VSPAERO_DOUBLE *array, int size)
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

    if ( size > 0 ) {

       for ( i = 0 ; i <= size ; i++ ) {

          new_array[i] = array[i];

       }

       // Delete old array and move pointer

       delete [] array;

    }

    return new_array;

}

/*##############################################################################
#                                                                              #
#                            resize_double_array                               #
#                                                                              #
##############################################################################*/

VSPAERO_DOUBLE* resize_double_array(VSPAERO_DOUBLE *array, int size, int new_size)
{

    int i;
    VSPAERO_DOUBLE *new_array;

    // Allocate and zero out new array space

    new_array = new VSPAERO_DOUBLE[new_size + 1];

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

void in_sphere(VSPAERO_DOUBLE xyz[4][3], VSPAERO_DOUBLE xyz_in[3], VSPAERO_DOUBLE &radius_in)
{

    VSPAERO_DOUBLE matrix_1[4][4], matrix_2[4][4], rhs[4], det_matrix_1, sub_det[4];
    VSPAERO_DOUBLE norm[4][3];
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

void circ_sphere(VSPAERO_DOUBLE xyz[4][3], VSPAERO_DOUBLE xyz_out[3], VSPAERO_DOUBLE &radius_out)
{

    VSPAERO_DOUBLE matrix_1[4][4], matrix_2[4][4], rhs[4], det_matrix_1, sub_det[4];
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

void get_normals(VSPAERO_DOUBLE xyz[4][3], VSPAERO_DOUBLE normals[4][3])
{

    VSPAERO_DOUBLE x1norm, x2norm, x3norm, x4norm, dx[4][4], dy[4][4], dz[4][4];

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

void determinant(VSPAERO_DOUBLE mat[4][4], VSPAERO_DOUBLE *det_matrix)
{

    VSPAERO_DOUBLE det_1[6], det_2[4];

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
#                                                                              # 
##############################################################################*/

int tri_seg_int(VSPAERO_DOUBLE *p1, VSPAERO_DOUBLE *p2, VSPAERO_DOUBLE *p3, VSPAERO_DOUBLE *p4, VSPAERO_DOUBLE *p5,
                VSPAERO_DOUBLE *tt, VSPAERO_DOUBLE *uu, VSPAERO_DOUBLE *ww)
{

    VSPAERO_DOUBLE udir[3], wdir[3], tdir[3], cross[3];
    VSPAERO_DOUBLE u_len, w_len, t_len, min_len, max_len, zero;
    VSPAERO_DOUBLE t_one, u_one, w_one, t_zero, u_zero, w_zero;
    VSPAERO_DOUBLE det1, det2, det3, u, w, t, xn[3], dot;
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

    zero = -1.e-8;

    // t_zero = zero * (max_len / t_len) * (min_len / max_len);
    // t_one  = 1. - t_zero;
    // 
    // u_zero = zero * (max_len / u_len) * (min_len / max_len);
    // u_one  = 1. - u_zero;
    // 
    // w_zero = zero * (max_len / w_len) * (min_len / max_len);
    // w_one  = 1. - w_zero;

    t_zero = u_zero = w_zero = 0.;
    t_one = u_one = w_one = 1.;

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
#                                                                              #
##############################################################################*/

int prl_seg_int(VSPAERO_DOUBLE *p1, VSPAERO_DOUBLE *p2, VSPAERO_DOUBLE *p3, VSPAERO_DOUBLE *p4, VSPAERO_DOUBLE *p5,
                VSPAERO_DOUBLE *tt, VSPAERO_DOUBLE *uu, VSPAERO_DOUBLE *ww)
{

    VSPAERO_DOUBLE udir[3], wdir[3], tdir[3], cross[3];
    VSPAERO_DOUBLE u_len, w_len, t_len, min_len, max_len, zero;
    VSPAERO_DOUBLE t_one, u_one, w_one, t_zero, u_zero, w_zero;
    VSPAERO_DOUBLE det1, det2, det3, u, w, t, xn[3], dot;
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

    zero = -1.e-8;

    // t_zero = zero * (max_len / t_len) * (min_len / max_len);
    // t_one  = 1. - t_zero;
    // 
    // u_zero = zero * (max_len / u_len) * (min_len / max_len);
    // u_one  = 1. - u_zero;
    // 
    // w_zero = zero * (max_len / w_len) * (min_len / max_len);
    // w_one  = 1. - w_zero;

    t_zero = u_zero = w_zero = 0.;
    t_one = u_one = w_one = 1.;

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

    return(0);

}

/*##############################################################################
#                                                                              #
#                                 compare_boxes                                #
#                                                                              #
# Compares the bounding boxes of two objects for overlap                       #
#                                                                              # 
##############################################################################*/

int compare_boxes(BBOX &box1, BBOX &box2)
{

    VSPAERO_DOUBLE tol_x, tol_y, tol_z;

    tol_x = 0.01*MAX3( ABS(box1.x_max-box1.x_min), ABS(box2.x_max-box2.x_min), 1. );
    tol_y = 0.01*MAX3( ABS(box1.y_max-box1.y_min), ABS(box2.y_max-box2.y_min), 1. );
    tol_z = 0.01*MAX3( ABS(box1.z_max-box1.z_min), ABS(box2.z_max-box2.z_min), 1. );

//    tol_x = tol_y = tol_z = 0.;

    if ( (box1.x_min - box2.x_max) > tol_x ) return(0); // Return 0 if no overlap

    if ( (box2.x_min - box1.x_max) > tol_x ) return(0);

    if ( (box1.y_min - box2.y_max) > tol_y ) return(0);

    if ( (box2.y_min - box1.y_max) > tol_y ) return(0);

    if ( (box1.z_min - box2.z_max) > tol_z ) return(0);

    if ( (box2.z_min - box1.z_max) > tol_z ) return(0);

    return(1); // Boxes overlap

}

/*##############################################################################
#                                                                              #
#                                 inside_box                                   #
#                                                                              #
# Checks if point xyz is inside bounding box                                   #
#                                                                              #  
##############################################################################*/

int inside_box(BBOX &box, VSPAERO_DOUBLE xyz[3])
{

    if ( xyz[0] >= box.x_min && xyz[0] <= box.x_max ) {
     
       if ( xyz[1] >= box.y_min && xyz[1] <= box.y_max  ) {
        
          if ( xyz[2] >= box.z_min && xyz[2] <= box.z_max ) return 1;
          
       }
       
    }
    
    return 0;

}

/*##############################################################################
#                                                                              #
#                          calculate_box_overlap                               #
#                                                                              #
# Compares the bounding boxes of two objects for overlap                       #
#                                                                              # 
##############################################################################*/

VSPAERO_DOUBLE calculate_box_overlap(BBOX &box1, BBOX &box2)
{

    VSPAERO_DOUBLE tol_x, tol_y, tol_z, ds;

    if ( compare_boxes(box1, box2) == 0 ) return 0.;
    
    tol_x = 0.01*MAX3( ABS(box1.x_max-box1.x_min), ABS(box2.x_max-box2.x_min), 1. );
    tol_y = 0.01*MAX3( ABS(box1.y_max-box1.y_min), ABS(box2.y_max-box2.y_min), 1. );
    tol_z = 0.01*MAX3( ABS(box1.z_max-box1.z_min), ABS(box2.z_max-box2.z_min), 1. );

    ds = 1.e9;

    // X
    
    if ( (box1.x_min - box2.x_max) > tol_x ) {
       
       return 0.;
       
    }
    
    else {
       
       ds = MIN(ds, ABS(box1.x_min - box2.x_max));
       
    }
    
    if ( (box2.x_min - box1.x_max) > tol_x ) {
       
       return 0.;
       
    }
    
    else {
       
       ds = MIN(ds, ABS(box2.x_min - box1.x_max));
       
    }
    
    // Y
        
    if ( (box1.y_min - box2.y_max) > tol_y ) {
       
       return 0.;
       
    }
    
    else {
       
       ds = MIN(ds, ABS(box1.y_min - box2.y_max));
       
    }

    if ( (box2.y_min - box1.y_max) > tol_y ) {
       
       return 0.;
       
    }
    
    else {
       
       ds = MIN(ds, ABS(box2.y_min - box1.y_max));
       
    }
    
    // Z

    if ( (box1.z_min - box2.z_max) > tol_z ) {
       
       return 0.;
       
    }
    
    else {
       
       ds = MIN(ds, ABS(box1.z_min - box2.z_max));
       
    }

    if ( (box2.z_min - box1.z_max) > tol_z ) {
       
       return 0.;
       
    }
    
    else {
       
       ds = MIN(ds, ABS(box2.z_min - box1.z_max));
       
    }

    return ds; // Boxes overlap

}

/*##############################################################################
#                                                                              #
#                                 box_calculate_size                           #
#                                                                              #
# Calculate size of the bounding box                                           #
#                                                                              #
##############################################################################*/

void box_calculate_size(BBOX &box)
{

    box.Length_Squared = pow(box.x_max - box.x_min, 2.) 
                       + pow(box.y_max - box.y_min, 2.) 
                       + pow(box.z_max - box.z_min, 2.) ;

}

/*##############################################################################
#                                                                              #
#                                 box_distance_ratio                           #
#                                                                              #
# Calculate distance to bbox as a fraction of max box size                     #
#                                                                              #
##############################################################################*/

VSPAERO_DOUBLE box_distance_ratio(BBOX &box, VSPAERO_DOUBLE xyz[3])
{

    VSPAERO_DOUBLE x, y, z, Distance;
    
    x = xyz[0];
    if ( x > box.x_max ) x = box.x_max;
    if ( x < box.x_min ) x = box.x_min;
  
    y = xyz[1];
    if ( y > box.y_max ) y = box.y_max;
    if ( y < box.y_min ) y = box.y_min;
    
    z = xyz[2];
    if ( z > box.z_max ) z = box.z_max;
    if ( z < box.z_min ) z = box.z_min;
    
    Distance = pow(x-xyz[0], 2.) + pow(y-xyz[1], 2.) + pow(z-xyz[2], 2.);
      
    Distance /= box.Length_Squared;
    
    return Distance;

}

/*##############################################################################
#                                                                              #
#                                 lines_intersect                              #
#                                                                              #
# Check if two lines intersect                                                 #
#                                                                              #                                                                              #
##############################################################################*/

int lines_intersect(VSPAERO_DOUBLE *u, VSPAERO_DOUBLE *v, VSPAERO_DOUBLE *p, VSPAERO_DOUBLE *q, VSPAERO_DOUBLE &t1, VSPAERO_DOUBLE &t2,
                    VSPAERO_DOUBLE &ds1, VSPAERO_DOUBLE &ds2 )
{

    MATRIX A(3,2), b(3), At(2,3), AA(2,2), bb(2), xx(2);
    VSPAERO_DOUBLE det;

    // Find intersection of two lines given by
    // (x,y)_1 = u + t1 * v
    // (x,y)_2 = p + t2 * q
    //
    // Where u, v, p, q are vectors in 3 space and t1, t2 are scalars
    //
    // We return u, v the intersect location on each line (if it exists)
    // We return ds1, ds2 the distance along each line... ie
    // ds1 = | alpha * v |
    // ds2 = | beta  * q |
    //
    // The function returns 1 if there is an itersection, otherwise 0
    //
    // Some notes: We assume the vectors all lie in some plane, to some
    // tolerance... we are doing the intersection of 2 vectors in 3 space...
    // So.. we only have 2 unknowns (u,v) and 3 equations... we do a least
    // squares solution, which is exact if they are truly in a plane. If the
    // vectors are not in a plane, then we are projecting a solution... and
    // may not be what you wanted! -DJK

    // Set up Ax = b, where A is a 3 x 2

    A(1,1) = v[0];    A(1,2) = -q[0];    b(1) = p[0] - u[0];
    A(2,1) = v[1];    A(2,2) = -q[1];    b(2) = p[1] - u[1];
    A(3,1) = v[2];    A(3,2) = -q[2];    b(3) = p[2] - u[2];

    // Form tranpose of matrix, multiply through equation to form 2x2 system

    At = A.transpose();

    AA = At * A;

    bb = At * b;

    // Check the determinant

    det = AA(1,1)*AA(2,2) - AA(1,2)*AA(2,1);

    // If determinant looks ok, the solve system

    if ( ABS(det) != 0. ) {

      xx = bb / AA;

    }

    else {

       return 0;

    }

    // Extract out u, v

    t1 = xx(1);
    t2 = xx(2);

    // Solve for magnitude of distance along each vector to intersection point

    ds1 = ABS(t1) * sqrt( v[0]*v[0] + v[1]*v[1] + v[2]*v[2] );
    ds2 = ABS(t2) * sqrt( q[0]*q[0] + q[1]*q[1] + q[2]*q[2] );

    return 1;

}

/*##############################################################################
#                                                                              #
#                               asearch                                        #
#                                                                              #
# searches an array then returns the index i such that:                        #
# the input value is inside the interval a(i) and a(i+1)                       #                                                                             *
#                                                                              #
##############################################################################*/

int asearch(VSPAERO_DOUBLE value, const VSPAERO_DOUBLE * array, int dim, VSPAERO_DOUBLE &sa, int &bound)
{
    int i;
    int sign = (array[1] < array[2]) ? 1 : -1;
    i = 0;
    VSPAERO_DOUBLE x = sign*value;
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

/*##############################################################################
#                                                                              #
#                                 Intersect2DLines                             #
#                                                                              #
# Check if 2 lines, in 2 dimensions, intersect                                 #
#                                                                              #
# Find intersection of two lines given by                                      #
# (x,y)_1 = u + t1 * v                                                         #
# (x,y)_2 = p + t2 * q                                                         #
#                                                                              #
# Where u, v, p, q are vectors in 2 space and t1, t2 are scalars               #
#                                                                              #
##############################################################################*/

int Intersect2DLines(VSPAERO_DOUBLE *u, VSPAERO_DOUBLE *v, VSPAERO_DOUBLE *p, VSPAERO_DOUBLE *q, VSPAERO_DOUBLE &t1, VSPAERO_DOUBLE &t2)
{

    VSPAERO_DOUBLE A11, A12, A21, A22, b1, b2, Det;

    // Set up Ax = b

    A11 = v[0]; A12 = -q[0]; b1 = p[0] - u[0];
    A21 = v[1]; A22 = -q[1]; b2 = p[1] - u[1];

    // Check the determinant

    Det = A11 * A22 - A12 * A21;

    // If determinant looks ok, the solve system

    if ( ABS(Det) > 0. ) {

       t1 = ( b1 * A22 - b2 * A12 ) / Det;

       t2 = ( b2 * A11 - b1 * A21 ) / Det;
       
       return 1;
       
    }

    return 0;

}

/*##############################################################################
#                                                                              #
#                                 CheckIfInsideTri                             #
#                                                                              #
# Check if 2 lines, in 2 dimensions, intersect                                 #
#                                                                              # 
# Find intersection of two lines given by                                      #
# (x,y)_1 = u + t1 * v                                                         #
# (x,y)_2 = p + t2 * q                                                         #
#                                                                              #
# Where u, v, p, q are vectors in 2 space and t1, t2 are scalars               #
#                                                                              #
##############################################################################*/

int CheckIfInsideTri(VSPAERO_DOUBLE *x, VSPAERO_DOUBLE *y, VSPAERO_DOUBLE *p, VSPAERO_DOUBLE Eps)
{

    VSPAERO_DOUBLE A11, A12, A21, A22, b1, b2, Det, t1, t2;

    // Set up Ax = b

    A11 = x[1] - x[0]; A12 = x[2] - x[0]; b1 = p[0] - x[0];
    A21 = y[1] - y[0]; A22 = y[2] - y[0]; b2 = p[1] - y[0];

    // Check the determinant

    Det = A11 * A22 - A12 * A21;

    // If determinant looks ok, the solve system

    if ( ABS(Det) > 0. ) {

       t1 = ( b1 * A22 - b2 * A12 ) / Det;

       if ( t1 > -Eps && t1 < 1. + Eps ) {

          t2 = ( b2 * A11 - b1 * A21 ) / Det;

          if ( t2 > -Eps && t2 < 1. + Eps ) {

             if ( t1 + t2 < 1. + Eps ) {

                return 1;

             }

          }

       }

    }

    return 0;

}

/*##############################################################################
#                                                                              #
#                        PointIsBetweenPlanes                                  #
#                                                                              #
# Determine if point lies between two planes                                   #
# Plane 1 normal points into volume, Plane 2 normal points out of volume       #
#                                                                              #
##############################################################################*/

int PointIsBetweenPlanes(VSPAERO_DOUBLE *Plane_1_Normal, VSPAERO_DOUBLE *Plane_1_Point,
                         VSPAERO_DOUBLE *Plane_2_Normal, VSPAERO_DOUBLE *Plane_2_Point,
                         VSPAERO_DOUBLE *Point) 
{
   
   if ( PointIsOnRightSideOfPlane(Plane_1_Normal, Plane_1_Point, Point) >= 0. ) {
      
      if ( PointIsOnRightSideOfPlane(Plane_2_Normal, Plane_2_Point, Point ) <= 0. ) {
         
         return 1;
         
      }
      
      return 0;
      
   }
   
   return 0;
   
}

/*##############################################################################
#                                                                              #
#                        PointIsOnRightSideOfPlane                             #
#                                                                              #
# Determine which side of a plan a point lies                                  #
#                                                                              #
# return postivie vale if point is on + (defined by normal) side, negative     #
# value if on negative side, and 0 if it happens to lie on the plane...        #
#                                                                              #
##############################################################################*/

VSPAERO_DOUBLE PointIsOnRightSideOfPlane(VSPAERO_DOUBLE *PlaneNormal, VSPAERO_DOUBLE *PlanePoint, VSPAERO_DOUBLE *Point)
{

    VSPAERO_DOUBLE Vec[3];
    
    Vec[0] = Point[0] - PlanePoint[0];
    Vec[1] = Point[1] - PlanePoint[1];
    Vec[2] = Point[2] - PlanePoint[2];

    return vector_dot(PlaneNormal,Vec);
    
}

/*##############################################################################
#                                                                              #
#                        PlaneSegmentIntersection                              #
#                                                                              #
# Calculate intersection of plane and a line                                   #
#                                                                              #
##############################################################################*/

int PlaneSegmentIntersection(VSPAERO_DOUBLE *PlaneNormal, VSPAERO_DOUBLE *PlanePoint, VSPAERO_DOUBLE *LinePoint_1, VSPAERO_DOUBLE *LinePoint_2, VSPAERO_DOUBLE &s)
{

    VSPAERO_DOUBLE Vec[3], Vec2[3], Dot, Dot2, Eps;
    
    Eps = 1.e-7;
    
    // Line direction vector
    
    Vec[0] = LinePoint_2[0] - LinePoint_1[0];
    Vec[1] = LinePoint_2[1] - LinePoint_1[1];
    Vec[2] = LinePoint_2[2] - LinePoint_1[2];

    Dot = sqrt(vector_dot(Vec,Vec));
    
    Vec[0] /= Dot;
    Vec[1] /= Dot;
    Vec[2] /= Dot;
    
    // Now calculate the intersection point
    
    Dot = vector_dot(PlaneNormal, Vec);
    
    Vec2[0] = PlanePoint[0] - LinePoint_1[0];
    Vec2[1] = PlanePoint[1] - LinePoint_1[1];
    Vec2[2] = PlanePoint[2] - LinePoint_1[2];
    
    Dot2 = vector_dot(Vec2,PlaneNormal);
    
    // Line and plane are parallel
    
    if ( ABS(Dot) <= Eps ) {
       
       // Line lays in the plane of the plane...
       
       if ( ABS(Dot2) <= Eps ) {

          s = 0;
          
          return 0;
          
       }
       
       // Line is just parallel with plane
       
       else {
          
          s = 0.;
          
          return -1;
          
       }
       
    }
    
    // Else there is an intersection
    
    else {
       
      s = Dot2/Dot;
      
      return 1;
      
    }
 
}
/*##############################################################################
#                                                                              #
#                                     ABS                                      #
#                                                                              #
#                                                                              #
##############################################################################*/

int ABS(int a)
{
 
    if ( a > 0 ) {
       
       return a;
       
    }
    
    else {
       
       return -a;
       
    }
    
}

/*##############################################################################
#                                                                              #
#                                     ABS                                      #
#                                                                              #
#                                                                              #
##############################################################################*/

VSPAERO_DOUBLE ABS(VSPAERO_DOUBLE a)
{

    if ( a > 0. ) {
       
       return a;
       
    }
    
    else {
       
       return -a;
       
    }
    
}

/*##############################################################################
#                                                                              #
#                                     SQR                                      #
#                                                                              #
#                                                                              #
##############################################################################*/

VSPAERO_DOUBLE SQR(VSPAERO_DOUBLE a)
{
   
    return a*a;
    
}

/*##############################################################################
#                                                                              #
#                                     SGN                                      #
#                                                                              #
#                                                                              #
##############################################################################*/

int SGN(int a)
{
   
    if ( a > 0 ) {
       
       return 1;
       
    }
    
    else {
       
       return -1;
       
    }
        
}

/*##############################################################################
#                                                                              #
#                                     SGN                                      #
#                                                                              #
#                                                                              #
##############################################################################*/

VSPAERO_DOUBLE SGN(VSPAERO_DOUBLE a)
{
   
    if ( a > 0. ) {
       
       return 1.;
       
    }
    
    else {
       
       return -1.;
       
    }
    
}

/*##############################################################################
#                                                                              #
#                                     MIN                                      #
#                                                                              #
#                                                                              #
##############################################################################*/

int MIN(int a, int b)
{
   
    if ( a < b ) {
       
       return a;
       
    }
    
    else {
       
       return b;
       
    }
    
}

/*##############################################################################
#                                                                              #
#                                     MAX                                      #
#                                                                              #
#                                                                              #
##############################################################################*/

int MAX(int a, int b)
{
   
    if ( a > b ) {
       
       return a;
       
    }
    
    else {
       
       return b;
       
    }
    
}

/*##############################################################################
#                                                                              #
#                                     MIN                                      #
#                                                                              #
#                                                                              #
##############################################################################*/

VSPAERO_DOUBLE MIN(VSPAERO_DOUBLE a, VSPAERO_DOUBLE b)
{
   
    if ( a < b ) {
       
       return a;
       
    }
    
    else {
       
       return b;
       
    }
    
}

/*##############################################################################
#                                                                              #
#                                     MAX                                      #
#                                                                              #
#                                                                              #
##############################################################################*/

VSPAERO_DOUBLE MAX(VSPAERO_DOUBLE a, VSPAERO_DOUBLE b)
{
   
    if ( a > b ) {
       
       return a;
       
    }
    
    else {
       
       return b;
       
    }
    
}

/*##############################################################################
#                                                                              #
#                                     MIN                                      #
#                                                                              #
#                                                                              #
##############################################################################*/

int MIN3(int a, int b, int c)
{
   
    if ( a < b && a < c ) {
       
       return a;
       
    }
    
    else if ( b < a && b < c ) {
       
       return b;
       
    }
    
    else {
       
       return c;
       
    }
    
}

/*##############################################################################
#                                                                              #
#                                     MAX                                      #
#                                                                              #
#                                                                              #
##############################################################################*/

int MAX3(int a, int b, int c)
{
   
    if ( a > b && a > c ) {
       
       return a;
       
    }
    
    else if ( b > a && b > c ) {
       
       return b;
       
    }
    
    else {
       
       return c;
       
    }
    
}

/*##############################################################################
#                                                                              #
#                                     MIN                                      #
#                                                                              #
#                                                                              #
##############################################################################*/

VSPAERO_DOUBLE MIN3(VSPAERO_DOUBLE a, VSPAERO_DOUBLE b, VSPAERO_DOUBLE c)
{
   
    if ( a < b && a < c ) {
       
       return a;
       
    }
    
    else if ( b < a && b < c ) {
       
       return b;
       
    }
    
    else {
       
       return c;
       
    }
    
}

/*##############################################################################
#                                                                              #
#                                     MAX                                      #
#                                                                              #
#                                                                              #
##############################################################################*/

VSPAERO_DOUBLE MAX3(VSPAERO_DOUBLE a, VSPAERO_DOUBLE b, VSPAERO_DOUBLE c)
{
   
    if ( a > b && a > c ) {
       
       return a;
       
    }
    
    else if ( DOUBLE(b) > DOUBLE(a) && DOUBLE(b) > DOUBLE(c) ) {
       
       return b;
       
    }
    
    else {
       
       return c;
       
    }
    
}

#include "END_NAME_SPACE.H"








