//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//
//////////////////////////////////////////////////////////////////////

#ifndef VSP_NODE_H
#define VSP_NODE_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <assert.h>
#include "utils.H"
#include "quat.H"

#include "START_NAME_SPACE.H"

// Definition of the VSP_NODE class

class VSP_NODE {

private:

    // Node is on trailing edge
    
    int IsTrailingEdgeNode_;
    
    // Node is on boundary edge
    
    int IsBoundaryEdgeNode_;
    
    // Node is on intersection edge
    
    int IsIntersectionNode_;
    
    // Node is on corner of two boundaries
    
    int IsBoundaryCornerNode_;
    
    // Node is on the wake, and attached to trailing edge of wing
    
    int IsWakeTrailingEdgeNode_;
    
    // Node is on symmetry plane
    
    int IsSymmetryPlaneNode_;
    
    // VSPAERO Component ID
    
    int ComponentID_;
    
    // OpenVSP ComponentID_;
    
    int OpenVSP_ComponentID_;
    
    // Surface ID
    
    int SurfaceID_;

    // XYZ location of this node
    
    double x_;
    double y_;
    double z_;

    // Delta xyz used for GMRES solve on wakes
    
    double dxyz_[3];
    
    // Adjoint perturbation velocities
    
    double dq0_dxyz_[3];
    double dq1_dxyz_[3];
    double dq2_dxyz_[3];
    
    // Adjoint solution, Psi
    
    double Psi_[3];
            
    // Partial derivative of invisicid, viscous, waked induced forces wrt this node xyz, ie Fx,Fy,Fz wrt x,y,z --> 18 components

    double pForces_pMesh_[9][3];

    // Total derivative of invisicid, viscous, waked induced forces wrt this node xyz, ie Fx,Fy,Fz wrt x,y,z --> 18 components

    double DForces_DMesh_[9][3];

    // Partial derivative of invisicid, viscous, waked induced moments wrt this node xyz, ie Fx,Fy,Fz wrt x,y,z --> 18 components

    double pMoments_pMesh_[6][3];

    // Total derivative of invisicid and viscous moments wrt this node xyz, ie Fx,Fy,Fz wrt x,y,z --> 18 components

    double DMoments_DMesh_[6][3];
               
    // Partial derivative of invisicid and viscous aero forces wrt this node xyz, ie CL,CD,CS wrt x,y,z --> 18 components

    double pAeroForces_pMesh_[9][3];

    // Total derivative of invisicid, viscous, waked induced aero forces wrt this node xyz, ie CL,CD,CS wrt x,y,z --> 18 components

    double DAeroForces_DMesh_[9][3];

    // Partial derivative of invisicid, viscous, waked induced aero moments wrt this node xyz, ie Cl,Cm,Cn wrt x,y,z --> 18 components

    double pAeroMoments_pMesh_[6][3];

    // Total derivative of invisicid and viscous aero moments wrt this node xyz, ie Cl,Cm,Cn wrt x,y,z --> 18 components

    double DAeroMoments_DMesh_[6][3];
                     
    // Corresponding coarse grid node at Level + 1
    
    int CoarseGridNode_;
    
    // Corresponding fine grid node at Level - 1
    
    int FineGridNode_;

public:

    // Constructor, Destructor, Copy

    VSP_NODE(void);
   ~VSP_NODE(void);
    VSP_NODE(const VSP_NODE &VSPNode);
    VSP_NODE& operator=(const VSP_NODE &VSPNode);

    /** X coordinate of node **/

    double &x(void) { return x_; };
    
    /** Y coordinate of node **/
    
    double &y(void) { return y_; };

    /** Z coordinate of node **/

    double &z(void) { return z_; };

    /** Perturbation of X coordinate of node **/

    double &dX(void) { return dxyz_[0]; };
    
    /** Perturbation of Y coordinate of node **/
    
    double &dY(void) { return dxyz_[1]; };

    /** Perturbation of Z coordinate of node **/

    double &dZ(void) { return dxyz_[2]; };
    
    /** Perturbation of XYZ coordinates of node **/

    double *dXYZ(void) { return dxyz_; };    
    
    /** Adjoint perturbation velocities **/
    
    double &dq0_dx(void) { return dq0_dxyz_[0]; };
    double &dq0_dy(void) { return dq0_dxyz_[1]; };
    double &dq0_dz(void) { return dq0_dxyz_[2]; };

    double &dq1_dx(void) { return dq1_dxyz_[0]; };
    double &dq1_dy(void) { return dq1_dxyz_[1]; };
    double &dq1_dz(void) { return dq1_dxyz_[2]; };
    
    double &dq2_dx(void) { return dq2_dxyz_[0]; };
    double &dq2_dy(void) { return dq2_dxyz_[1]; };
    double &dq2_dz(void) { return dq2_dxyz_[2]; };
        
    /** Adjoint solution, Psi **/
        
    double &Psi(int i) { return Psi_[i]; };
       
// Inviscid forces partial Derivatives
        
    /** Partial derivative of inviscid Fx wrt x,y, and z **/

    double &pFix_pX(void) { return pForces_pMesh_[0][0]; };
    double &pFix_pY(void) { return pForces_pMesh_[0][1]; };
    double &pFix_pZ(void) { return pForces_pMesh_[0][2]; };
                    
    double &pFix_pJ(int j) { return pForces_pMesh_[0][j]; };

    /** Partial derivative of inviscid Fy wrt x,y, and z **/

    double &pFiy_pX(void) { return pForces_pMesh_[1][0]; };
    double &pFiy_pY(void) { return pForces_pMesh_[1][1]; };
    double &pFiy_pZ(void) { return pForces_pMesh_[1][2]; };
                      
    double &pFiy_pJ(int j) { return pForces_pMesh_[1][j]; };

    /** Partial derivative of inviscid Fz wrt x,y, and z **/
    
    double &pFiz_pX(void) { return pForces_pMesh_[2][0]; };
    double &pFiz_pY(void) { return pForces_pMesh_[2][1]; };
    double &pFiz_pZ(void) { return pForces_pMesh_[2][2]; };
                                     
    double &pFiz_pJ(int j) { return pForces_pMesh_[2][j]; };

    /** Partial derivative of inviscid Mx wrt x,y, and z **/

    double &pMix_pX(void) { return pMoments_pMesh_[0][0]; };
    double &pMix_pY(void) { return pMoments_pMesh_[0][1]; };
    double &pMix_pZ(void) { return pMoments_pMesh_[0][2]; };

    double &pMix_pJ(int j) { return pMoments_pMesh_[0][j]; };

    /** Partial derivative of inviscid My wrt x,y, and z **/

    double &pMiy_pX(void) { return pMoments_pMesh_[1][0]; };
    double &pMiy_pY(void) { return pMoments_pMesh_[1][1]; };
    double &pMiy_pZ(void) { return pMoments_pMesh_[1][2]; };
                      
    double &pMiy_pJ(int j) { return pMoments_pMesh_[1][j]; };

    /** Partial derivative of inviscid Mz wrt x,y, and z **/
   
    double &pMiz_pX(void) { return pMoments_pMesh_[2][0]; };
    double &pMiz_pY(void) { return pMoments_pMesh_[2][1]; };
    double &pMiz_pZ(void) { return pMoments_pMesh_[2][2]; };
                                       
    double &pMiz_pJ(int j) { return pMoments_pMesh_[2][j]; };

// Viscous forces partial Derivatives
        
    /** Partial derivative of viscous Fx wrt x,y, and z **/

    double &pFox_pX(void) { return pForces_pMesh_[3][0]; };
    double &pFox_pY(void) { return pForces_pMesh_[3][1]; };
    double &pFox_pZ(void) { return pForces_pMesh_[3][2]; };
                                                           
    double &pFxo_pJ(int j) { return pForces_pMesh_[3][j]; };
                       
    /** Partial derivative of viscous Fy wrt x,y, and z **/
                       
    double &pFoy_pX(void) { return pForces_pMesh_[4][0]; };
    double &pFoy_pY(void) { return pForces_pMesh_[4][1]; };
    double &pFoy_pZ(void) { return pForces_pMesh_[4][2]; };
                                                          
    double &pFoy_pJ(int j) { return pForces_pMesh_[4][j]; };
                       
    /** Partial derivative of viscous Fz wrt x,y, and z **/
                       
    double &pFoz_pX(void) { return pForces_pMesh_[5][0]; };
    double &pFoz_pY(void) { return pForces_pMesh_[5][1]; };
    double &pFoz_pZ(void) { return pForces_pMesh_[5][2]; };
                                                          
    double &pFoz_pJ(int j) { return pForces_pMesh_[5][j]; };
                       
    /** Partial derivative of viscous Mx wrt x,y, and z **/
                       
    double &pMox_pX(void) { return pMoments_pMesh_[3][0]; };
    double &pMox_pY(void) { return pMoments_pMesh_[3][1]; };
    double &pMox_pZ(void) { return pMoments_pMesh_[3][2]; };
                                                         
    double &pMox_pJ(int j) { return pMoments_pMesh_[3][j]; };
                       
    /** Partial derivative of viscous My wrt x,y, and z **/
                       
    double &pMoy_pX(void) { return pMoments_pMesh_[4][0]; };
    double &pMoy_pY(void) { return pMoments_pMesh_[4][1]; };
    double &pMoy_pZ(void) { return pMoments_pMesh_[4][2]; };
                                                    
    double &pMoy_pJ(int j) { return pMoments_pMesh_[4][j]; };
                       
    /** Partial derivative of viscous Mz wrt x,y, and z **/
                       
    double &pMoz_pX(void) { return pMoments_pMesh_[5][0]; };
    double &pMoz_pY(void) { return pMoments_pMesh_[5][1]; };
    double &pMoz_pZ(void) { return pMoments_pMesh_[5][2]; };
                                                 
    double &pMoz_pJ(int j) { return pMoments_pMesh_[5][j]; };

// Waked induced forces partial Derivatives
        
    /** Partial derivative of Waked induced Fx wrt x,y, and z **/

    double &pFwx_pX(void) { return pForces_pMesh_[6][0]; };
    double &pFwx_pY(void) { return pForces_pMesh_[6][1]; };
    double &pFwx_pZ(void) { return pForces_pMesh_[6][2]; };
                                                           
    double &pFwx_pJ(int j) { return pForces_pMesh_[6][j]; };
                       
    /** Partial derivative of Waked induced Fy wrt x,y, and z **/
                       
    double &pFwy_pX(void) { return pForces_pMesh_[7][0]; };
    double &pFwy_pY(void) { return pForces_pMesh_[7][1]; };
    double &pFwy_pZ(void) { return pForces_pMesh_[7][2]; };
                                                          
    double &pFwy_pJ(int j) { return pForces_pMesh_[7][j]; };
                       
    /** Partial derivative of Waked induced Fz wrt x,y, and z **/
                       
    double &pFwz_pX(void) { return pForces_pMesh_[8][0]; };
    double &pFwz_pY(void) { return pForces_pMesh_[8][1]; };
    double &pFwz_pZ(void) { return pForces_pMesh_[8][2]; };
                                                          
    double &pFwz_pJ(int j) { return pForces_pMesh_[8][j]; };

// Total forces partial Derivatives
        
    /** Partial derivative of total Fx wrt x,y, and z **/

    double pFtx_pX(void) { return pForces_pMesh_[0][0] + pForces_pMesh_[3][0]; };
    double pFtx_pY(void) { return pForces_pMesh_[0][1] + pForces_pMesh_[3][1]; };
    double pFtx_pZ(void) { return pForces_pMesh_[0][2] + pForces_pMesh_[3][2]; };
                    
    double pFtx_pJ(int j) { return pForces_pMesh_[0][j] + pForces_pMesh_[3][j]; };

    /** Partial derivative of total Fy wrt x,y, and z **/

    double pFty_pX(void) { return pForces_pMesh_[1][0] + pForces_pMesh_[4][0]; };
    double pFty_pY(void) { return pForces_pMesh_[1][1] + pForces_pMesh_[4][1]; };
    double pFty_pZ(void) { return pForces_pMesh_[1][2] + pForces_pMesh_[4][2]; };
                   
    double pFty_pJ(int j) { return pForces_pMesh_[1][j] + pForces_pMesh_[4][j]; };

    /** Partial derivative of total Fz wrt x,y, and z **/
    
    double pFtz_pX(void) { return pForces_pMesh_[2][0] + pForces_pMesh_[5][0]; };
    double pFtz_pY(void) { return pForces_pMesh_[2][1] + pForces_pMesh_[5][1]; };
    double pFtz_pZ(void) { return pForces_pMesh_[2][2] + pForces_pMesh_[5][1]; };
                                
    double pFtz_pJ(int j) { return pForces_pMesh_[2][j] + pForces_pMesh_[5][j]; };

    /** Partial derivative of total Mx wrt x,y, and z **/

    double pMtx_pX(void) { return pMoments_pMesh_[0][0] + pMoments_pMesh_[3][0]; };
    double pMtx_pY(void) { return pMoments_pMesh_[0][1] + pMoments_pMesh_[3][1]; };
    double pMtx_pZ(void) { return pMoments_pMesh_[0][2] + pMoments_pMesh_[3][2]; };
                    
    double pMtx_pJ(int j) { return pMoments_pMesh_[0][j] + pMoments_pMesh_[3][j]; };

    /** Partial derivative of total My wrt x,y, and z **/

    double pMty_pX(void) { return pMoments_pMesh_[1][0] + pMoments_pMesh_[4][0]; };
    double pMty_pY(void) { return pMoments_pMesh_[1][1] + pMoments_pMesh_[4][1]; };
    double pMty_pZ(void) { return pMoments_pMesh_[1][2] + pMoments_pMesh_[4][2]; };
                    
    double pMty_pJ(int j) { return pMoments_pMesh_[1][j] + pMoments_pMesh_[4][j]; };

    /** Partial derivative of total Mz wrt x,y, and z **/
   
    double pMtz_pX(void) { return pMoments_pMesh_[2][0] + pMoments_pMesh_[5][0]; };
    double pMtz_pY(void) { return pMoments_pMesh_[2][1] + pMoments_pMesh_[5][1]; };
    double pMtz_pZ(void) { return pMoments_pMesh_[2][2] + pMoments_pMesh_[5][2]; };
                                   
    double pMtz_pJ(int j) { return pMoments_pMesh_[2][j] + pMoments_pMesh_[5][j]; };

// Total wake induced + viscous forces partial Derivatives
        
    /** Partial derivative of total Fx wrt x,y, and z **/

    double pFtwx_pX(void) { return pForces_pMesh_[6][0] + pForces_pMesh_[3][0]; };
    double pFtwx_pY(void) { return pForces_pMesh_[6][1] + pForces_pMesh_[3][1]; };
    double pFtwx_pZ(void) { return pForces_pMesh_[6][2] + pForces_pMesh_[3][2]; };
                    
    double pFtwx_pJ(int j) { return pForces_pMesh_[6][j] + pForces_pMesh_[3][j]; };

    /** Partial derivative of total Fy wrt x,y, and z **/

    double pFtwy_pX(void) { return pForces_pMesh_[7][0] + pForces_pMesh_[4][0]; };
    double pFtwy_pY(void) { return pForces_pMesh_[7][1] + pForces_pMesh_[4][1]; };
    double pFtwy_pZ(void) { return pForces_pMesh_[7][2] + pForces_pMesh_[4][2]; };
                   
    double pFtwy_pJ(int j) { return pForces_pMesh_[7][j] + pForces_pMesh_[4][j]; };

    /** Partial derivative of total Fz wrt x,y, and z **/
    
    double pFtwz_pX(void) { return pForces_pMesh_[8][0] + pForces_pMesh_[5][0]; };
    double pFtwz_pY(void) { return pForces_pMesh_[8][1] + pForces_pMesh_[5][1]; };
    double pFtwz_pZ(void) { return pForces_pMesh_[8][2] + pForces_pMesh_[5][1]; };
                                
    double pFtwz_pJ(int j) { return pForces_pMesh_[8][j] + pForces_pMesh_[5][j]; };
    
// Inviscid total Derivatives

    /** Total derivative of inviscid Fx wrt x,y, and z **/

    double &DFix_DX(void) { return DForces_DMesh_[0][0]; };
    double &DFix_DY(void) { return DForces_DMesh_[0][1]; };
    double &DFix_DZ(void) { return DForces_DMesh_[0][2]; };

    double &DFix_DJ(int j) { return DForces_DMesh_[0][j]; };

    /** Total derivative of inviscid  Fy wrt x,y, and z **/

    double &DFiy_DX(void) { return DForces_DMesh_[1][0]; };
    double &DFiy_DY(void) { return DForces_DMesh_[1][1]; };
    double &DFiy_DZ(void) { return DForces_DMesh_[1][2]; };
                      
    double &DFiy_DJ(int j) { return DForces_DMesh_[1][j]; };

    /** Total derivative of inviscid  Fz wrt x,y, and z **/
    
    double &DFiz_DX(void) { return DForces_DMesh_[2][0]; };
    double &DFiz_DY(void) { return DForces_DMesh_[2][1]; };
    double &DFiz_DZ(void) { return DForces_DMesh_[2][2]; };
                            
    double &DFiz_DJ(int j) { return DForces_DMesh_[2][j]; };

    /** Total derivative of inviscid  Mx wrt x,y, and z **/

    double &DMix_DX(void) { return DMoments_DMesh_[0][0]; };
    double &DMix_DY(void) { return DMoments_DMesh_[0][1]; };
    double &DMix_DZ(void) { return DMoments_DMesh_[0][2]; };
                   
    double &DMix_DJ(int j) { return DMoments_DMesh_[0][j]; };

    /** Total derivative of inviscid  My wrt x,y, and z **/

    double &DMiy_DX(void) { return DMoments_DMesh_[1][0]; };
    double &DMiy_DY(void) { return DMoments_DMesh_[1][1]; };
    double &DMiy_DZ(void) { return DMoments_DMesh_[1][2]; };
                     
    double &DMiy_DJ(int j) { return DMoments_DMesh_[1][j]; };

    /** Total derivative of inviscid  Mz wrt x,y, and z **/
    
    double &DMiz_DX(void) { return DMoments_DMesh_[2][0]; };
    double &DMiz_DY(void) { return DMoments_DMesh_[2][1]; };
    double &DMiz_DZ(void) { return DMoments_DMesh_[2][2]; };
                           
    double &DMiz_DJ(int j) { return DMoments_DMesh_[2][j]; };

// Waked induced total Derivatives

    /** Total derivative of inviscid Fx wrt x,y, and z **/

    double &DFwakex_DX(void) { return DForces_DMesh_[6][0]; };
    double &DFwakex_DY(void) { return DForces_DMesh_[6][1]; };
    double &DFwakex_DZ(void) { return DForces_DMesh_[6][2]; };
            
    double &DFwakex_DJ(int j) { return DForces_DMesh_[6][j]; };

    /** Total derivative of inviscid  Fy wrt x,y, and z **/

    double &DFwakey_DX(void) { return DForces_DMesh_[7][0]; };
    double &DFwakey_DY(void) { return DForces_DMesh_[7][1]; };
    double &DFwakey_DZ(void) { return DForces_DMesh_[7][2]; };
                      
    double &DFwakey_DJ(int j) { return DForces_DMesh_[7][j]; };

    /** Total derivative of inviscid  Fz wrt x,y, and z **/
    
    double &DFwakez_DX(void) { return DForces_DMesh_[8][0]; };
    double &DFwakez_DY(void) { return DForces_DMesh_[8][1]; };
    double &DFwakez_DZ(void) { return DForces_DMesh_[8][2]; };
                        
    double &DFwakez_DJ(int j) { return DForces_DMesh_[8][j]; };

// Viscous total Derivatives

    /** Total derivative of viscous Fx wrt x,y, and z **/

    double &DFox_DX(void) { return DForces_DMesh_[3][0]; };
    double &DFox_DY(void) { return DForces_DMesh_[3][1]; };
    double &DFox_DZ(void) { return DForces_DMesh_[3][2]; };
                                                        
    double &DFox_DJ(int j) { return DForces_DMesh_[3][j]; };
                       
    /** Total derivative of viscous Fy wrt x,y, and z **/
                       
    double &DFoy_DX(void) { return DForces_DMesh_[4][0]; };
    double &DFoy_DY(void) { return DForces_DMesh_[4][1]; };
    double &DFoy_DZ(void) { return DForces_DMesh_[4][2]; };
                                                    
    double &DFoy_DJ(int j) { return DForces_DMesh_[4][j]; };
                       
    /** Total derivative of viscous Fz wrt x,y, and z **/
                       
    double &DFoz_DX(void) { return DForces_DMesh_[5][0]; };
    double &DFoz_DY(void) { return DForces_DMesh_[5][1]; };
    double &DFoz_DZ(void) { return DForces_DMesh_[5][2]; };
                                                  
    double &DFoz_DJ(int j) { return DForces_DMesh_[5][j]; };
                       
    /** Total derivative of viscous Mx wrt x,y, and z **/
                       
    double &DMox_DX(void) { return DMoments_DMesh_[3][0]; };
    double &DMox_DY(void) { return DMoments_DMesh_[3][1]; };
    double &DMox_DZ(void) { return DMoments_DMesh_[3][2]; };
                                                   
    double &DMox_DJ(int j) { return DMoments_DMesh_[3][j]; };
                       
    /** Total derivative of viscous My wrt x,y, and z **/
                       
    double &DMoy_DX(void) { return DMoments_DMesh_[4][0]; };
    double &DMoy_DY(void) { return DMoments_DMesh_[4][1]; };
    double &DMoy_DZ(void) { return DMoments_DMesh_[4][2]; };
                                                      
    double &DMoy_DJ(int j) { return DMoments_DMesh_[4][j]; };
                       
    /** Total derivative of viscous Mz wrt x,y, and z **/
                       
    double &DMoz_DX(void) { return DMoments_DMesh_[5][0]; };
    double &DMoz_DY(void) { return DMoments_DMesh_[5][1]; };
    double &DMoz_DZ(void) { return DMoments_DMesh_[5][2]; };
                                                         
    double &DMoz_DJ(int j) { return DMoments_DMesh_[5][j]; };

// Total total Derivatives

    /** Total derivative of total Fx wrt x,y, and z **/

    double DFtx_DX(void) { return DForces_DMesh_[0][0] + DForces_DMesh_[3][0]; };
    double DFtx_DY(void) { return DForces_DMesh_[0][1] + DForces_DMesh_[3][1]; };
    double DFtx_DZ(void) { return DForces_DMesh_[0][2] + DForces_DMesh_[3][2]; };
                   
    double DFtx_DJ(int j) { return DForces_DMesh_[0][j] + DForces_DMesh_[3][j]; };

    /** Total derivative of total  Fy wrt x,y, and z **/

    double DFty_DX(void) { return DForces_DMesh_[1][0] + DForces_DMesh_[4][0]; };
    double DFty_DY(void) { return DForces_DMesh_[1][1] + DForces_DMesh_[4][1]; };
    double DFty_DZ(void) { return DForces_DMesh_[1][2] + DForces_DMesh_[4][2]; };
                    
    double DFty_DJ(int j) { return DForces_DMesh_[1][j] + DForces_DMesh_[4][j]; };

    /** Total derivative of total  Fz wrt x,y, and z **/
    
    double DFtz_DX(void) { return DForces_DMesh_[2][0] + DForces_DMesh_[5][0]; };
    double DFtz_DY(void) { return DForces_DMesh_[2][1] + DForces_DMesh_[5][1]; };
    double DFtz_DZ(void) { return DForces_DMesh_[2][2] + DForces_DMesh_[5][2]; };
                       
    double DFtz_DJ(int j) { return DForces_DMesh_[2][j] + DForces_DMesh_[5][j]; };

    /** Total derivative of total  Mx wrt x,y, and z **/

    double DMtx_DX(void) { return DMoments_DMesh_[0][0] + DMoments_DMesh_[3][0]; };
    double DMtx_DY(void) { return DMoments_DMesh_[0][1] + DMoments_DMesh_[3][1]; };
    double DMtx_DZ(void) { return DMoments_DMesh_[0][2] + DMoments_DMesh_[3][2]; };
                   
    double DMtx_DJ(int j) { return DMoments_DMesh_[0][j]; };

    /** Total derivative of total  My wrt x,y, and z **/

    double DMty_DX(void) { return DMoments_DMesh_[1][0] + DMoments_DMesh_[4][0]; };
    double DMty_DY(void) { return DMoments_DMesh_[1][1] + DMoments_DMesh_[4][1]; };
    double DMty_DZ(void) { return DMoments_DMesh_[1][2] + DMoments_DMesh_[4][2]; };
                     
    double DMty_DJ(int j) { return DMoments_DMesh_[1][j] + DMoments_DMesh_[4][j]; };

    /** Total derivative of total  Mz wrt x,y, and z **/
    
    double DMtz_DX(void) { return DMoments_DMesh_[2][0] + DMoments_DMesh_[5][0]; };
    double DMtz_DY(void) { return DMoments_DMesh_[2][1] + DMoments_DMesh_[5][1]; };
    double DMtz_DZ(void) { return DMoments_DMesh_[2][2] + DMoments_DMesh_[5][2]; };
                       
    double DMtz_DJ(int j) { return DMoments_DMesh_[2][j] + DMoments_DMesh_[5][j]; };

// Total total Derivatives based on induced wake forces

    /** Total derivative of total Fx wrt x,y, and z **/

    double DFtwx_DX(void) { return DForces_DMesh_[6][0] + DForces_DMesh_[3][0]; };
    double DFtwx_DY(void) { return DForces_DMesh_[6][1] + DForces_DMesh_[3][1]; };
    double DFtwx_DZ(void) { return DForces_DMesh_[6][2] + DForces_DMesh_[3][2]; };
            
    double DFtwx_DJ(int j) { return DForces_DMesh_[6][j] + DForces_DMesh_[3][j]; };

    /** Total derivative of total  Fy wrt x,y, and z **/

    double DFtwy_DX(void) { return DForces_DMesh_[7][0] + DForces_DMesh_[4][0]; };
    double DFtwy_DY(void) { return DForces_DMesh_[7][1] + DForces_DMesh_[4][1]; };
    double DFtwy_DZ(void) { return DForces_DMesh_[7][2] + DForces_DMesh_[4][2]; };
                 
    double DFtwy_DJ(int j) { return DForces_DMesh_[7][j] + DForces_DMesh_[4][j]; };

    /** Total derivative of total  Fz wrt x,y, and z **/
    
    double DFtwz_DX(void) { return DForces_DMesh_[8][0] + DForces_DMesh_[5][0]; };
    double DFtwz_DY(void) { return DForces_DMesh_[8][1] + DForces_DMesh_[5][1]; };
    double DFtwz_DZ(void) { return DForces_DMesh_[8][2] + DForces_DMesh_[5][2]; };
                       
    double DFtwz_DJ(int j) { return DForces_DMesh_[8][j] + DForces_DMesh_[5][j]; };

// Aero forces and moments                
               
// Aero Inviscid forces partial Derivatives
        
    /** Partial derivative of inviscid CL wrt x,y, and z **/

    double &pCLi_pX(void) { return pAeroForces_pMesh_[0][0]; };
    double &pCLi_pY(void) { return pAeroForces_pMesh_[0][1]; };
    double &pCLi_pZ(void) { return pAeroForces_pMesh_[0][2]; };

    double &pCLi_pJ(int j) { return pAeroForces_pMesh_[0][j]; };

    /** Partial derivative of inviscid CD wrt x,y, and z **/

    double &pCDi_pX(void) { return pAeroForces_pMesh_[1][0]; };
    double &pCDi_pY(void) { return pAeroForces_pMesh_[1][1]; };
    double &pCDi_pZ(void) { return pAeroForces_pMesh_[1][2]; };
                      
    double &pCDi_pJ(int j) { return pAeroForces_pMesh_[1][j]; };

    /** Partial derivative of inviscid CS wrt x,y, and z **/
    
    double &pCSi_pX(void) { return pAeroForces_pMesh_[2][0]; };
    double &pCSi_pY(void) { return pAeroForces_pMesh_[2][1]; };
    double &pCSi_pZ(void) { return pAeroForces_pMesh_[2][2]; };
                                      
    double &pCSi_pJ(int j) { return pAeroForces_pMesh_[2][j]; };

    /** Partial derivative of inviscid CMl wrt x,y, and z **/

    double &pCMli_pX(void) { return pAeroMoments_pMesh_[0][0]; };
    double &pCMli_pY(void) { return pAeroMoments_pMesh_[0][1]; };
    double &pCMli_pZ(void) { return pAeroMoments_pMesh_[0][2]; };
                     
    double &pCMli_pJ(int j) { return pAeroMoments_pMesh_[0][j]; };

    /** Partial derivative of inviscid CMm wrt x,y, and z **/

    double &pCMmi_pX(void) { return pAeroMoments_pMesh_[1][0]; };
    double &pCMmi_pY(void) { return pAeroMoments_pMesh_[1][1]; };
    double &pCMmi_pZ(void) { return pAeroMoments_pMesh_[1][2]; };
                      
    double &pCMmi_pJ(int j) { return pAeroMoments_pMesh_[1][j]; };

    /** Partial derivative of inviscid Cmn wrt x,y, and z **/
   
    double &pCMni_pX(void) { return pAeroMoments_pMesh_[2][0]; };
    double &pCMni_pY(void) { return pAeroMoments_pMesh_[2][1]; };
    double &pCMni_pZ(void) { return pAeroMoments_pMesh_[2][2]; };
                                  
    double &pCmni_pJ(int j) { return pAeroMoments_pMesh_[2][j]; };

    /** Partial derivative of wake induced CL wrt x,y, and z **/

    double &pCLwake_pX(void) { return pAeroForces_pMesh_[6][0]; };
    double &pCLwake_pY(void) { return pAeroForces_pMesh_[6][1]; };
    double &pCLwake_pZ(void) { return pAeroForces_pMesh_[6][2]; };
              
    double &pCLwake_pJ(int j) { return pAeroForces_pMesh_[6][j]; };

    /** Partial derivative of wake induced CD wrt x,y, and z **/

    double &pCDwake_pX(void) { return pAeroForces_pMesh_[7][0]; };
    double &pCDwake_pY(void) { return pAeroForces_pMesh_[7][1]; };
    double &pCDwake_pZ(void) { return pAeroForces_pMesh_[7][2]; };
                     
    double &pCDwake_pJ(int j) { return pAeroForces_pMesh_[7][j]; };

    /** Partial derivative of wake induced CS wrt x,y, and z **/
    
    double &pCSwake_pX(void) { return pAeroForces_pMesh_[8][0]; };
    double &pCSwake_pY(void) { return pAeroForces_pMesh_[8][1]; };
    double &pCSwake_pZ(void) { return pAeroForces_pMesh_[8][2]; };
                                     
    double &pCSwake_pJ(int j) { return pAeroForces_pMesh_[8][j]; };

// Aero Viscous forces partial Derivatives
        
    /** Partial derivative of total CLo wrt x,y, and z **/

    double &pCLo_pX(void) { return pAeroForces_pMesh_[3][0]; };
    double &pCLo_pY(void) { return pAeroForces_pMesh_[3][1]; };
    double &pCLo_pZ(void) { return pAeroForces_pMesh_[3][2]; };
                                                           
    double &pCLo_pJ(int j) { return pAeroForces_pMesh_[3][j]; };
                       
    /** Partial derivatoive of inviscid CDo wrt x,y, and z **/
                       
    double &pCDo_pX(void) { return pAeroForces_pMesh_[4][0]; };
    double &pCDo_pY(void) { return pAeroForces_pMesh_[4][1]; };
    double &pCDo_pZ(void) { return pAeroForces_pMesh_[4][2]; };
                                                          
    double &pCDo_pJ(int j) { return pAeroForces_pMesh_[4][j]; };
                       
    /** Partial derivatoive of inviscid CSo wrt x,y, and z **/
                       
    double &pCSo_pX(void) { return pAeroForces_pMesh_[5][0]; };
    double &pCSo_pY(void) { return pAeroForces_pMesh_[5][1]; };
    double &pCSo_pZ(void) { return pAeroForces_pMesh_[5][2]; };
                                                          
    double &pCso_pJ(int j) { return pAeroForces_pMesh_[5][j]; };
                       
    /** Partial derivatoive of inviscid CMlo wrt x,y, and z **/
                       
    double &pCMlo_pX(void) { return pAeroMoments_pMesh_[3][0]; };
    double &pCMlo_pY(void) { return pAeroMoments_pMesh_[3][1]; };
    double &pCMlo_pZ(void) { return pAeroMoments_pMesh_[3][2]; };
                                                           
    double &pCMlo_pJ(int j) { return pAeroMoments_pMesh_[3][j]; };
                       
    /** Partial derivatoive of inviscid CMmo wrt x,y, and z **/
                       
    double &pCMmo_pX(void) { return pAeroMoments_pMesh_[4][0]; };
    double &pCMmo_pY(void) { return pAeroMoments_pMesh_[4][1]; };
    double &pCMmo_pZ(void) { return pAeroMoments_pMesh_[4][2]; };
                                               
    double &pCMmo_pJ(int j) { return pAeroMoments_pMesh_[4][j]; };
                       
    /** Partial derivatoive of inviscid CMno wrt x,y, and z **/
                       
    double &pCMno_pX(void) { return pAeroMoments_pMesh_[5][0]; };
    double &pCMno_pY(void) { return pAeroMoments_pMesh_[5][1]; };
    double &pCMno_pZ(void) { return pAeroMoments_pMesh_[5][2]; };
                                                   
    double &pCMno_pJ(int j) { return pAeroMoments_pMesh_[5][j]; };
    
// Aero total forces partial Derivatives
        
    /** Partial derivative of total CL wrt x,y, and z **/

    double pCLt_pX(void) { return pAeroForces_pMesh_[0][0] + pAeroForces_pMesh_[3][0]; };
    double pCLt_pY(void) { return pAeroForces_pMesh_[0][1] + pAeroForces_pMesh_[3][1]; };
    double pCLt_pZ(void) { return pAeroForces_pMesh_[0][2] + pAeroForces_pMesh_[3][2]; };
                     
    double pCLt_pJ(int j) { return pAeroForces_pMesh_[0][j] + pAeroForces_pMesh_[3][j]; };

    /** Partial derivative of total CD wrt x,y, and z **/

    double pCDt_pX(void) { return pAeroForces_pMesh_[1][0] + pAeroForces_pMesh_[4][0]; };
    double pCDt_pY(void) { return pAeroForces_pMesh_[1][1] + pAeroForces_pMesh_[4][1]; };
    double pCDt_pZ(void) { return pAeroForces_pMesh_[1][2] + pAeroForces_pMesh_[4][2]; };
                      
    double pCDt_pJ(int j) { return pAeroForces_pMesh_[1][j] + pAeroForces_pMesh_[4][j]; };

    /** Partial derivative of total CS wrt x,y, and z **/
    
    double pCSt_pX(void) { return pAeroForces_pMesh_[2][0] + pAeroForces_pMesh_[5][0]; };
    double pCSt_pY(void) { return pAeroForces_pMesh_[2][1] + pAeroForces_pMesh_[5][1]; };
    double pCSt_pZ(void) { return pAeroForces_pMesh_[2][2] + pAeroForces_pMesh_[5][2]; };
                               
    double pCSt_pJ(int j) { return pAeroForces_pMesh_[2][j] + pAeroForces_pMesh_[5][j]; };

    /** Partial derivative of total CMl wrt x,y, and z **/

    double pCMlt_pX(void) { return pAeroMoments_pMesh_[0][0] + pAeroMoments_pMesh_[3][0]; };
    double pCMlt_pY(void) { return pAeroMoments_pMesh_[0][1] + pAeroMoments_pMesh_[3][1]; };
    double pCMlt_pZ(void) { return pAeroMoments_pMesh_[0][2] + pAeroMoments_pMesh_[3][2]; };
                     
    double pCMlt_pJ(int j) { return pAeroMoments_pMesh_[0][j] + pAeroMoments_pMesh_[3][j]; };

    /** Partial derivative of total CMm wrt x,y, and z **/

    double pCMmt_pX(void) { return pAeroMoments_pMesh_[1][0] + pAeroMoments_pMesh_[4][0]; };
    double pCMmt_pY(void) { return pAeroMoments_pMesh_[1][1] + pAeroMoments_pMesh_[4][1]; };
    double pCMmt_pZ(void) { return pAeroMoments_pMesh_[1][2] + pAeroMoments_pMesh_[4][2]; };
                      
    double pCMmt_pJ(int j) { return pAeroMoments_pMesh_[1][j] + pAeroMoments_pMesh_[4][j]; };

    /** Partial derivative of total Cmn wrt x,y, and z **/
   
    double pCMnt_pX(void) { return pAeroMoments_pMesh_[2][0] + pAeroMoments_pMesh_[5][0]; };
    double pCMnt_pY(void) { return pAeroMoments_pMesh_[2][1] + pAeroMoments_pMesh_[5][1]; };
    double pCMnt_pZ(void) { return pAeroMoments_pMesh_[2][2] + pAeroMoments_pMesh_[5][2]; };
                             
    double pCmnt_pJ(int j) { return pAeroMoments_pMesh_[2][j] + pAeroMoments_pMesh_[5][j]; };

// Aero total forces partial Derivatives based on wake induced forces
        
    /** Partial derivative of total CL wrt x,y, and z **/

    double pCLtw_pX(void) { return pAeroForces_pMesh_[6][0] + pAeroForces_pMesh_[3][0]; };
    double pCLtw_pY(void) { return pAeroForces_pMesh_[6][1] + pAeroForces_pMesh_[3][1]; };
    double pCLtw_pZ(void) { return pAeroForces_pMesh_[6][2] + pAeroForces_pMesh_[3][2]; };
                     
    double pCLtw_pJ(int j) { return pAeroForces_pMesh_[6][j] + pAeroForces_pMesh_[3][j]; };

    /** Partial derivative of total CD wrt x,y, and z **/

    double pCDtw_pX(void) { return pAeroForces_pMesh_[7][0] + pAeroForces_pMesh_[4][0]; };
    double pCDtw_pY(void) { return pAeroForces_pMesh_[7][1] + pAeroForces_pMesh_[4][1]; };
    double pCDtw_pZ(void) { return pAeroForces_pMesh_[7][2] + pAeroForces_pMesh_[4][2]; };
                      
    double pCDtw_pJ(int j) { return pAeroForces_pMesh_[7][j] + pAeroForces_pMesh_[4][j]; };

    /** Partial derivative of total CS wrt x,y, and z **/
    
    double pCStw_pX(void) { return pAeroForces_pMesh_[8][0] + pAeroForces_pMesh_[5][0]; };
    double pCStw_pY(void) { return pAeroForces_pMesh_[8][1] + pAeroForces_pMesh_[5][1]; };
    double pCStw_pZ(void) { return pAeroForces_pMesh_[8][2] + pAeroForces_pMesh_[5][2]; };
                               
    double pCStw_pJ(int j) { return pAeroForces_pMesh_[8][j] + pAeroForces_pMesh_[5][j]; };

// Aero Inviscid total Derivatives

    /** Total derivative of inviscid  CL wrt x,y, and z **/

    double &DCLi_DX(void) { return DAeroForces_DMesh_[0][0]; };
    double &DCLi_DY(void) { return DAeroForces_DMesh_[0][1]; };
    double &DCLi_DZ(void) { return DAeroForces_DMesh_[0][2]; };
                       
    double &DCLi_DJ(int j) { return DAeroForces_DMesh_[0][j]; };

    /** Total derivative of inviscid  CD wrt x,y, and z **/

    double &DCDi_DX(void) { return DAeroForces_DMesh_[1][0]; };
    double &DCDi_DY(void) { return DAeroForces_DMesh_[1][1]; };
    double &DCDi_DZ(void) { return DAeroForces_DMesh_[1][2]; };
                     
    double &DCDi_DJ(int j) { return DAeroForces_DMesh_[1][j]; };

    /** Total derivative of inviscid  CS wrt x,y, and z **/
    
    double &DCSi_DX(void) { return DAeroForces_DMesh_[2][0]; };
    double &DCSi_DY(void) { return DAeroForces_DMesh_[2][1]; };
    double &DCSi_DZ(void) { return DAeroForces_DMesh_[2][2]; };
                       
    double &DCSi_DJ(int j) { return DAeroForces_DMesh_[2][j]; };

    /** Total derivative of inviscid  CMl wrt x,y, and z **/

    double &DCMli_DX(void) { return DAeroMoments_DMesh_[0][0]; };
    double &DCMli_DY(void) { return DAeroMoments_DMesh_[0][1]; };
    double &DCMli_DZ(void) { return DAeroMoments_DMesh_[0][2]; };
                        
    double &DCMli_DJ(int j) { return DAeroMoments_DMesh_[0][j]; };

    /** Total derivative of inviscid  CMm wrt x,y, and z **/

    double &DCMmi_DX(void) { return DAeroMoments_DMesh_[1][0]; };
    double &DCMmi_DY(void) { return DAeroMoments_DMesh_[1][1]; };
    double &DCMmi_DZ(void) { return DAeroMoments_DMesh_[1][2]; };
                       
    double &DCMmi_DJ(int j) { return DAeroMoments_DMesh_[1][j]; };

    /** Total derivative of inviscid  CMn wrt x,y, and z **/
    
    double &DCMni_DX(void) { return DAeroMoments_DMesh_[2][0]; };
    double &DCMni_DY(void) { return DAeroMoments_DMesh_[2][1]; };
    double &DCMni_DZ(void) { return DAeroMoments_DMesh_[2][2]; };
                        
    double &DCMni_DJ(int j) { return DAeroMoments_DMesh_[2][j]; };

// Aero wake induced total Derivatives

  //  /** Total derivative of waked induced  CL wrt x,y, and z **/
  //
  //  double &DCLwake_DX(void) { return DAeroForces_DMesh_[6][0]; };
  //  double &DCLwake_DY(void) { return DAeroForces_DMesh_[6][1]; };
  //  double &DCLwake_DZ(void) { return DAeroForces_DMesh_[6][2]; };
  //               
  //  double &DCLwake_DJ(int j) { return DAeroForces_DMesh_[6][j]; };
  //
  //  /** Total derivative of waked induced  CD wrt x,y, and z **/
  //
  //  double &DCDwake_DX(void) { return DAeroForces_DMesh_[7][0]; };
  //  double &DCDwake_DY(void) { return DAeroForces_DMesh_[7][1]; };
  //  double &DCDwake_DZ(void) { return DAeroForces_DMesh_[7][2]; };
  //                
  //  double &DCDwake_DJ(int j) { return DAeroForces_DMesh_[7][j]; };
  //
  //  /** Total derivative of waked induced  CS wrt x,y, and z **/
  //  
  //  double &DCSwake_DX(void) { return DAeroForces_DMesh_[8][0]; };
  //  double &DCSwake_DY(void) { return DAeroForces_DMesh_[8][1]; };
  //  double &DCSwake_DZ(void) { return DAeroForces_DMesh_[8][2]; };
  //                  
  //  double &DCSwake_DJ(int j) { return DAeroForces_DMesh_[8][j]; };
    
// Aero Viscous total Derivatives

    /** Total derivative of viscous CLo wrt x,y, and z **/

    double &DCLo_DX(void) { return DAeroForces_DMesh_[3][0]; };
    double &DCLo_DY(void) { return DAeroForces_DMesh_[3][1]; };
    double &DCLo_DZ(void) { return DAeroForces_DMesh_[3][2]; };
                                                         
    double &DCLo_DJ(int j) { return DAeroForces_DMesh_[3][j]; };
                       
    /** Total derivative of viscous CDo wrt x,y, and z **/
                       
    double &DCDo_DX(void) { return DAeroForces_DMesh_[4][0]; };
    double &DCDo_DY(void) { return DAeroForces_DMesh_[4][1]; };
    double &DCDo_DZ(void) { return DAeroForces_DMesh_[4][2]; };
                                                        
    double &DCDo_DJ(int j) { return DAeroForces_DMesh_[4][j]; };
                       
    /** Total derivative of viscous CSo wrt x,y, and z **/
                       
    double &DCSo_DX(void) { return DAeroForces_DMesh_[5][0]; };
    double &DCSo_DY(void) { return DAeroForces_DMesh_[5][1]; };
    double &DCSo_DZ(void) { return DAeroForces_DMesh_[5][2]; };
                                                         
    double &DCSo_DJ(int j) { return DAeroForces_DMesh_[5][j]; };
                       
    /** Total derivative of viscous CMlo wrt x,y, and z **/
                       
    double &DCMlo_DX(void) { return DAeroMoments_DMesh_[3][0]; };
    double &DCMlo_DY(void) { return DAeroMoments_DMesh_[3][1]; };
    double &DCMlo_DZ(void) { return DAeroMoments_DMesh_[3][2]; };
                                                         
    double &DCMlo_DJ(int j) { return DAeroMoments_DMesh_[3][j]; };
                       
    /** Total derivative of viscous CMmo wrt x,y, and z **/
                       
    double &DCMmo_DX(void) { return DAeroMoments_DMesh_[4][0]; };
    double &DCMmo_DY(void) { return DAeroMoments_DMesh_[4][1]; };
    double &DCMmo_DZ(void) { return DAeroMoments_DMesh_[4][2]; };
                                                         
    double &DCMmo_DJ(int j) { return DAeroMoments_DMesh_[4][j]; };
                       
    /** Total derivative of viscous CMno wrt x,y, and z **/
                       
    double &DCMno_DX(void) { return DAeroMoments_DMesh_[5][0]; };
    double &DCMno_DY(void) { return DAeroMoments_DMesh_[5][1]; };
    double &DCMno_DZ(void) { return DAeroMoments_DMesh_[5][2]; };
                                                         
    double &DCMno_DJ(int j) { return DAeroMoments_DMesh_[5][j]; };

// Aero total total Derivatives

    /** Total derivative of total  CL wrt x,y, and z **/

    double DCLt_DX(void) { return DAeroForces_DMesh_[0][0] + DAeroForces_DMesh_[3][0]; };
    double DCLt_DY(void) { return DAeroForces_DMesh_[0][1] + DAeroForces_DMesh_[3][1]; };
    double DCLt_DZ(void) { return DAeroForces_DMesh_[0][2] + DAeroForces_DMesh_[3][2]; };
                  
    double DCLt_DJ(int j) { return DAeroForces_DMesh_[0][j] + DAeroForces_DMesh_[3][j]; };

    /** Total derivative of total  CD wrt x,y, and z **/

    double DCDt_DX(void) { return DAeroForces_DMesh_[1][0] + DAeroForces_DMesh_[4][0]; };
    double DCDt_DY(void) { return DAeroForces_DMesh_[1][1] + DAeroForces_DMesh_[4][1]; };
    double DCDt_DZ(void) { return DAeroForces_DMesh_[1][2] + DAeroForces_DMesh_[4][2]; };
                   
    double DCDt_DJ(int j) { return DAeroForces_DMesh_[1][j] + DAeroForces_DMesh_[4][j]; };

    /** Total derivative of total  CS wrt x,y, and z **/
    
    double DCSt_DX(void) { return DAeroForces_DMesh_[2][0] + DAeroForces_DMesh_[5][0]; };
    double DCSt_DY(void) { return DAeroForces_DMesh_[2][1] + DAeroForces_DMesh_[5][1]; };
    double DCSt_DZ(void) { return DAeroForces_DMesh_[2][2] + DAeroForces_DMesh_[5][2]; };
                   
    double DCSt_DJ(int j) { return DAeroForces_DMesh_[2][j] + DAeroForces_DMesh_[5][j]; };

    /** Total derivative of total  CMl wrt x,y, and z **/

    double DCMlt_DX(void) { return DAeroMoments_DMesh_[0][0] + DAeroMoments_DMesh_[3][0]; };
    double DCMlt_DY(void) { return DAeroMoments_DMesh_[0][1] + DAeroMoments_DMesh_[3][1]; };
    double DCMlt_DZ(void) { return DAeroMoments_DMesh_[0][2] + DAeroMoments_DMesh_[3][2]; };
                     
    double DCMlt_DJ(int j) { return DAeroMoments_DMesh_[0][j] + DAeroMoments_DMesh_[3][j]; };

    /** Total derivative of total  CMm wrt x,y, and z **/

    double DCMmt_DX(void) { return DAeroMoments_DMesh_[1][0] + DAeroMoments_DMesh_[4][0]; };
    double DCMmt_DY(void) { return DAeroMoments_DMesh_[1][1] + DAeroMoments_DMesh_[4][1]; };
    double DCMmt_DZ(void) { return DAeroMoments_DMesh_[1][2] + DAeroMoments_DMesh_[4][2]; };
                      
    double DCMmt_DJ(int j) { return DAeroMoments_DMesh_[1][j] + DAeroMoments_DMesh_[4][j]; };

    /** Total derivative of total  CMn wrt x,y, and z **/
    
    double DCMnt_DX(void) { return DAeroMoments_DMesh_[2][0] + DAeroMoments_DMesh_[5][0]; };
    double DCMnt_DY(void) { return DAeroMoments_DMesh_[2][1] + DAeroMoments_DMesh_[5][1]; };
    double DCMnt_DZ(void) { return DAeroMoments_DMesh_[2][2] + DAeroMoments_DMesh_[5][2]; };
                    
    double DCMnt_DJ(int j) { return DAeroMoments_DMesh_[2][j] + DAeroMoments_DMesh_[5][j]; };

// Aero total total Derivatives based on waked induced forces

 //   /** Total derivative of total  CL wrt x,y, and z **/
 //
 //   double DCLtw_DX(void) { return DAeroForces_DMesh_[6][0] + DAeroForces_DMesh_[3][0]; };
 //   double DCLtw_DY(void) { return DAeroForces_DMesh_[6][1] + DAeroForces_DMesh_[3][1]; };
 //   double DCLtw_DZ(void) { return DAeroForces_DMesh_[6][2] + DAeroForces_DMesh_[3][2]; };
 //            
 //   double DCLtw_DJ(int j) { return DAeroForces_DMesh_[6][j] + DAeroForces_DMesh_[3][j]; };
 //
 //   /** Total derivative of total  CD wrt x,y, and z **/
 //
 //   double DCDtw_DX(void) { return DAeroForces_DMesh_[7][0] + DAeroForces_DMesh_[4][0]; };
 //   double DCDtw_DY(void) { return DAeroForces_DMesh_[7][1] + DAeroForces_DMesh_[4][1]; };
 //   double DCDtw_DZ(void) { return DAeroForces_DMesh_[7][2] + DAeroForces_DMesh_[4][2]; };
 //                
 //   double DCDtw_DJ(int j) { return DAeroForces_DMesh_[7][j] + DAeroForces_DMesh_[4][j]; };
 //
 //   /** Total derivative of total  CS wrt x,y, and z **/
 //   
 //   double DCStw_DX(void) { return DAeroForces_DMesh_[8][0] + DAeroForces_DMesh_[5][0]; };
 //   double DCStw_DY(void) { return DAeroForces_DMesh_[8][1] + DAeroForces_DMesh_[5][1]; };
 //   double DCStw_DZ(void) { return DAeroForces_DMesh_[8][2] + DAeroForces_DMesh_[5][2]; };
 //             
 //   double DCStw_DJ(int j) { return DAeroForces_DMesh_[8][j] + DAeroForces_DMesh_[5][j]; };

    /** Node is on trailing edge **/
    
    int &IsTrailingEdgeNode(void) { return IsTrailingEdgeNode_; };
    
    /** Node is on the wake, and is also on the trailing edge of a wing **/
    
    int &IsWakeTrailingEdgeNode(void) { return IsWakeTrailingEdgeNode_; };

    /** Node is on boundary edge **/

    int &IsBoundaryEdgeNode(void) { return IsBoundaryEdgeNode_; };
    
    /** Node is on intersection edge **/
    
    int &IsIntersectionNode(void) { return IsIntersectionNode_; };

    /** Node is on a corner **/

    int &IsBoundaryCornerNode(void) { return IsBoundaryCornerNode_; };
    
    /** Node is on symmetry plane **/
    
    int &IsSymmetryPlaneNode(void) { return IsSymmetryPlaneNode_; };
    
    /** VSPAERO Component ID **/
    
    int &ComponentID(void) { return ComponentID_; };

    /** OpenVSP ComponentID_ **/
    
    int &OpenVSP_ComponentID(void) { return OpenVSP_ComponentID_; };
    
    /** Surface ID **/
    
    int &SurfaceID(void) { return SurfaceID_; };

    /** Coarse grid node **/
  
    int &CoarseGridNode(void) { return CoarseGridNode_; };
    
    /** Fine grid node **/
    
    int &FineGridNode(void) { return FineGridNode_; };
    
    /** Update geometry given translation vector and quaternion rotation **/
    
    void UpdateGeometryLocation(double *TVec, double *OVec, QUAT &Quat, QUAT &InvQuat);

};

#include "END_NAME_SPACE.H"

#endif
