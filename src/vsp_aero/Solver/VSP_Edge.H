//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//
//////////////////////////////////////////////////////////////////////

#ifndef VSP_EDGE_H
#define VSP_EDGE_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <assert.h>
#include "quat.H"
#include "VSP_Node.H"
#include "VSP_Loop.H"
#include "WakeEdgeData.H"
#include "quat.H"
#include "utils.H"

#include "START_NAME_SPACE.H"

#define INT_BOUNDARY_EDGE 1
#define VLM_BOUNDARY_EDGE 2  

// Small class for building edge list

class EDGE_ENTRY {

public:

    int node;
    int tri_1;
    int tri_2;
    int next;

};

// Definition of the VSP_EDGE class

class VSP_EDGE {

private:

    void init(void);

    // Debugging flag
    
    int Verbose_;
  
    // Edge nodes

    int Node1_;
    int Node2_;

    // Two loops attached to this edge

    int Loop1_;
    int Loop2_;
    
    // LoopL on left of edge... in a right hand rule kinda way
    
    int LoopL_;

    // LoopR is on right of edge... in a right hand rule kinda way
    
    int LoopR_;
    
    // Global vortex edge across all the multi-grid levels
    
    int VortexEdge_;
    
    // Edge is on trailing edge (kutta edge)
    
    int IsTrailingEdge_;
    
    // Edge is on wake surface, and on a wing trailing edge
    
    int IsWakeTrailingEdge_;
    
    // Edge is shed from concave region
    
    int IsConcaveTrailingEdge_;
    
    // Edge is on frozen portion of wake
    
    int IsFrozenTrailingEdge_;
    
    // Edge is on a surface boundary
    
    int IsBoundaryEdge_;
    
    // Edge is on surface intersection
    
    int IsIntersectionEdge_;

    // Edge is on symmetry plane
    
    int IsSymmetryPlaneEdge_;
    
    // Node 1 is on symmetry plane
    
    int Node1_IsOnSymmetryPlane_;

    // Node 2 is on symmetry plane
    
    int Node2_IsOnSymmetryPlane_;
            
    // Coarse grid edge this edge was merged into
    
    int CoarseGridEdge_;
    
    // Fine grid edge corresponding to this edge
    
    int FineGridEdge_;
    
    // Multi-grid level this edge belongs to
    
    int Level_;
    
    // Edge number on the level this edge belongs to
    
    int ThisEdge_;
    
    // Time accurate flag
    
    int TimeAccurate_;
    
    // Global time step
    
    int Time_;
    
    // If this edge is on the wake, this is the min time step it's valid for an unsteady analysis
    
    int MinValidTimeStep_;          

    // Forces on edge
    
    double Forces_[3];
    
    // Unsteady forces on edge
    
    double Unsteady_Forces_[3];
    
    // Trefftz analysis forces on edge (really trailing edge analysis forces...)
    
    double Trefftz_Forces_[3];    

    // Vortex core width for vortex core model
    
    double CoreWidth_;
    
    // Supersonic core width... experimental
    
    double SuperSonicCoreWidth_;

    // Surface ID
    
    int SurfaceID_;
    
    // VSPAERO Component ID
    
    int ComponentID_;
    
    // OpenVSP ComponentID_;
    
    int OpenVSP_ComponentID_;    
    
    // Vortex Sheet we might be part of
    
    int VortexSheet_;
    
    // Kutta nodes we might be assocated with... for spanwise loading calculations
    
    int KuttaNode_[2];

    // Group ID
    
    int Group_;

    // Edge node xyz coordinates at node 1
    
    double X1_;
    double Y1_;
    double Z1_;
    
    // Edge node xyz coordinates at node 2
    
    double X2_;
    double Y2_;
    double Z2_;
    
    // Edge node xyz coordinates at node 1 at previous time step
    
    double X1m_;
    double Y1m_;
    double Z1m_;
    
    // Edge node xyz coordinates at node 2 at previous time step
    
    double X2m_;
    double Y2m_;
    double Z2m_;    

    // Edge node delta xyz coordinate perturbations at node 1
    
    double dxyz1_[3];

    // Edge node delta xyz coordinate perturbations at node 2
    
    double dxyz2_[3];

    // Edge centroid
    
    double XYZc_[3];

    // Length of edge
    
    double Length_;

    // Edge normalized direction vector
    
    double Vec_[3];
    
    // Edge direction vector (so has magnitude of Length_)
    
    double u_;
    double v_;
    double w_;
    
    // Edge Normal vector
    
    double Normal_[3];
    
    // Distance tolerances for velocity evaluations
   
    static double Tolerance_1_;
    static double Tolerance_2_;
    static double Tolerance_4_;

    // Mach number
    
    double Mach_;
    
    // Leading compressiblity term on integral
    
    double Kappa_;

    // Karman-Tsien compressibility factor
    
    double KTFact_;

    // Trailing edge stall factor
    
    double TrailingEdgeStallFactor_;
    
    // Flag dermining which loops are down wind of this vortex edge
    
    int VortexLoopLIsDownWind_;
    int VortexLoopRIsDownWind_;
    
    // Vortex Loop down wind weights
    
    double VortexLoopLDownWindWeight_;
    double VortexLoopRDownWindWeight_;

    // Vorticty strength
    
    double Gamma_;
    
    // Perturbation vorticity strength
    
    double DeltaGamma_;
    
    // Perturbation stall vorticity strength
    
    double DeltaStallGamma_;
    
    // Perturbation vortex stretching strength
    
    double DeltaVortexStretchingRatioGamma_;
    
    // Setup routines
    
    void Setup_(VSP_NODE &Node1, VSP_NODE &Node2);

    // Velocity induced by this bound vortex

    void BoundVortex(double Gamma, double xyz_p[3], double q[3]);
    
    // Gradient wrt xyz1, xyz2 for this bound vortex

    void GradientBoundVortex(double &Gamma, double xyz_p[3],
                             double &dq0_dx1,
                             double &dq0_dy1,
                             double &dq0_dz1,
                             double &dq0_dx2,
                             double &dq0_dy2,
                             double &dq0_dz2,
                                            
                             double &dq1_dx1,
                             double &dq1_dy1,
                             double &dq1_dz1,
                             double &dq1_dx2,
                             double &dq1_dy2,
                             double &dq1_dz2,
                                            
                             double &dq2_dx1,
                             double &dq2_dy1,
                             double &dq2_dz1,
                             double &dq2_dx2,
                             double &dq2_dy2,
                             double &dq2_dz2,
                              
                             double &dq0_dxp,
                             double &dq0_dyp,
                             double &dq0_dzp,
                                                
                             double &dq1_dxp,
                             double &dq1_dyp,
                             double &dq1_dzp,
                                                  
                             double &dq2_dxp,
                             double &dq2_dyp,
                             double &dq2_dzp,
                             
                             double &dq0_dMach,
                             double &dq1_dMach,
                             double &dq2_dMach);                               
                                                                       
    // F integral evaluation of generalized Biot-Savart law
    
    double Fint(double &a, double &b, double &c, double &d, double &s);

    // G integral evaluation of generalized Biot-Savart law
    
    double Gint(double &a, double &b, double &c, double &d, double &s);

    // Gradient wrt xyz1, and xyz2 of F integral evaluation of generalized Biot-Savart law

    double GradientFint(double &a, double &b, double &c, double &d, double &s,
                                 double &dF_da,
                                 double &dF_db,
                                 double &dF_dc,
                                 double &dF_dd,
                                 double &dF_ds);

    // Gradient wrt xyz1, and xyz2 of G integral evaluation of generalized Biot-Savart law
                                 
    double GradientGint(double &a, double &b, double &c, double &d, double &s,
                                 double &dF_da,
                                 double &dF_db,
                                 double &dF_dc,
                                 double &dF_dd,
                                 double &dF_ds);

    // Find the intersection of ray with conic defined by current location and Mach number
    
    void FindLineConicIntersection(double &Xp, double &Yp, double &Zp,
                                   double &X1, double &Y1, double &Z1,
                                   double &Xd, double &Yd, double &Zd,
                                   double &t1, double &t2);    

    // Local free stream velocity... may include rotor, engine velocities... etc.
    
    double LocalFreeStreamVelocity_[5];    
    
    // Local velocity at edge centroid
    
    double Velocity_[3];

    // Local downwash + free stream velocity at edge centroid
    
    double DownWashVelocity_[3];
    
    // Local velocity magnitude at edge centroid
    
    double Q_;
    
    // Velocity due to perturbation in gammas
    
    double dVelocity_dGamma_[3];

    // Velocity due to perturbation in mesh xyz's
    
    double dVelocity_dMesh_[3];

    // Velocity due to perturbation Mach
    
    double dVelocity_dMach_[3];
        
    // Velocity due to perturbation in Stall factor
    
    double dVelocity_dStall_[3];

    // Velocity due to perturbation in vortex stretching ratio
    
    double dVelocity_dRatio_[3];
                        
    // Adjoint perturbation velocities
    
    double dq0_dxyz_[2][3];
    double dq1_dxyz_[2][3];
    double dq2_dxyz_[2][3];
    
    // Wake equation stuff
    
    int IsWakeEdge_;
    
    WAKE_EDGE *WakeEdgeData_;

    // Adjoint fine edge list
    
    int NumberOfAdjointEdges_;
    int *AdjointEdgeList_;    
    double *AdjointEdgeFactor_;
    
public:

    // Constructor, Destructor, Copy

    VSP_EDGE(void);
   ~VSP_EDGE(void);
    VSP_EDGE(const VSP_EDGE &VSPEdge);
    VSP_EDGE& operator=(const VSP_EDGE &VSPEdge);

    /** Grid node 1 of the edge **/

    int &Node1(void) { return Node1_; };
    
    /** Grid node 2 of the edge **/
    
    int &Node2(void) { return Node2_; };

    /** Tri 1 attached to this edge **/
    
    int &Tri1(void) { return Loop1_; };
    
    /** Tri 2 attached to this edge **/
    
    int &Tri2(void) { return Loop2_; };
    
    /** Left tri attached to this edge **/    
    
    int &TriL(void) { return LoopL_; };
    
    /** Right tri attached to this edge **/    
    
    int &TriR(void) { return LoopR_; };
 
    /** Loop 1 attached to this edge, note this could also be tri 1 on the finest mesh... **/
    
    int &Loop1(void) { return Loop1_; };
    
    /** Loop 2 attached to this edge, note this could also be tri 1 on the finest mesh... **/
    
    int &Loop2(void) { return Loop2_; };
 
    /** Left loop attached to this edge, note this could also be the left tri on the finest mesh... **/
 
    int &LoopL(void) { return LoopL_; };
    
    /** Right loop attached to this edge, note this could also be the right tri on the finest mesh... **/
    
    int &LoopR(void) { return LoopR_; };
    
    /** Global vortex edge number... ie, edge number over all edges in the agglomerated edge structures **/
    
    int &VortexEdge(void) { return VortexEdge_; };

    /** Finite core size **/
     
    double &CoreWidth(void) { return CoreWidth_; };
    
    /** Edge length **/
    
    double &Length(void) { return Length_; };
    
    /** Edge is on trailing edge of lifting surface **/
    
    int &IsTrailingEdge(void) { return IsTrailingEdge_; };
    
    /** Edge is a wake edge and is on trailing edge of a wing **/
    
    int &IsWakeTrailingEdge(void) { return IsWakeTrailingEdge_; };

    /** Edge is on wake surface, and on a wing trailing edge **/
    
    int &IsConcaveTrailingEdge(void) { return IsConcaveTrailingEdge_; };
    
    /** Edge is on frozen portion of wake **/
    
    int &IsFrozenTrailingEdge(void) { return IsFrozenTrailingEdge_; };   
    
    /** Edge is on boundary edge of a surface... this may be the boundary of a VLM surface, or the
     * intersection of two thick, two thin, or one thick and one thin surface(s) **/
    
    int &IsBoundaryEdge(void) { return IsBoundaryEdge_; };
    
    /** Edge is on intersection edge ... ie edge is only on the a true surface-surface intersection ...
     * this could be two thick, two thin, or one thick and one thin surface(s) **/
     
    int &IsIntersectionEdge(void) { return IsIntersectionEdge_; };
    
    /** Edge is on symmetry plane **/
    
    int &IsSymmetryPlaneEdge(void) { return IsSymmetryPlaneEdge_; };
    
    /** Node 1 is on symmetry plane **/
    
    int &Node1_IsOnSymmetryPlane(void) { return Node1_IsOnSymmetryPlane_; };

    /** Node 2 is on symmetry plane **/
    
    int &Node2_IsOnSymmetryPlane(void) { return Node2_IsOnSymmetryPlane_; };
        
    /** Coarse grid edge that this edge was agglomerated into **/
    
    int &CoarseGridEdge(void) { return CoarseGridEdge_; };
    
    /** Fine grid edge that this edge was created from... note that this is not unique... we only store one **/
  
    int &FineGridEdge(void) { return FineGridEdge_; };
    
    /** Agglomerated grid level this edge sits on **/
    
    int &Level(void) { return Level_; };
    
    /** This edges number on this grid level **/
    
    int &ThisEdge(void) { return ThisEdge_; };

    /** Time accurate flag **/
    
    int &TimeAccurate(void) { return TimeAccurate_; };
    
    /** Global time step **/
    
    int &Time(void) { return Time_; };

    /** If this edge is on the wake, this is the min time step it's valid for an unsteady analysis **/
    
    int &MinValidTimeStep(void) { return MinValidTimeStep_; };
    
    /** Edge is on vortex sheet **/

    int &VortexSheet(void) { return VortexSheet_; };
    
    // Kutta node we might be assocated with... for spanwise loading calculations
    
    int &KuttaNode(int i) { return KuttaNode_[i-1]; };
        
    /** Group ID **/
    
    int &Group(void) { return Group_; };

    /** Edge is on Surface **/
    
    int &SurfaceID(void) { return SurfaceID_; };
    
    /** Edge is on VSPAERO Component **/
    
    int &ComponentID(void) { return ComponentID_; };

    /** OpenVSP ComponentID_ **/
    
    int &OpenVSP_ComponentID(void) { return OpenVSP_ComponentID_; };
    
    /** Set up the edge data structure given these 2 mesh nodes **/

    void Setup(VSP_NODE &Node1, VSP_NODE &Node2) { Setup_(Node1,Node2); };                                     
                  
    /** Gradients of the setup variables **/
    
    void SetupGradients(double &dLength_dX1, 
                        double &dLength_dY1,
                        double &dLength_dZ1,
                        
                        double &dLength_dX2,
                        double &dLength_dY2,
                        double &dLength_dZ2,
                        
                        double &dVecX_dX1,
                        double &dVecX_dY1,
                        double &dVecX_dZ1,
                                                
                        double &dVecY_dX1,
                        double &dVecY_dY1,
                        double &dVecY_dZ1,
                                               
                        double &dVecZ_dX1,
                        double &dVecZ_dY1,
                        double &dVecZ_dZ1,
                                               
                        double &dVecX_dX2,
                        double &dVecX_dY2,
                        double &dVecX_dZ2,
                                               
                        double &dVecY_dX2,
                        double &dVecY_dY2,
                        double &dVecY_dZ2,
                                               
                        double &dVecZ_dX2,
                        double &dVecZ_dY2,
                        double &dVecZ_dZ2); 
                                  
                                                                     
    /** Update the edge data given these two mesh nodes .. we assume the initial set was performed
     * and we are just updating the location of edge node xyz locations and data that depends
     * on them... **/
                                  
    void Update(VSP_NODE &Node1, VSP_NODE &Node2) { Setup_(Node1,Node2); };
    
    /** Store the current wake locations for unsteady analysis **/
    
    void StoreWakeLocation(void);
    
    /** Calculate the induced velocity from this edge using latest gamma **/

    void InducedVelocity(double xyz_p[3], double q[3]);

    /** Calculate the induced gamma velocity from this edge using latest perturbation gamma **/

    void dInducedVelocity_dGamma(double xyz_p[3], double q[3]);

    /** Calculate the induced mesh velocity from this edge **/

    void dInducedVelocity_dMesh(double xyz_p[3], double dxyz_i[3], double q[3]);

    /** Calculate the induced velocity for this edge from gradient wrt Mach **/

    void dInducedVelocity_dMach(double xyz_p[3], double q[3]);

    /** Calculate the induced gamma velocity from this edge **/

    void pVelocity_pGamma(double xyz_p[3], double q[3]);

    /** Calculate the induced stall gamma velocity from this edge **/

    void dInducedVelocity_dStallGamma(double xyz_p[3], double q[3]);
    
    /** Calculate the induced vortex stretching ratio gamma velocity from this edge **/
    
    void dInducedVelocity_dVortexStretchingRatio(double xyz_p[3], double q[3]);
    
    /** Calculate the induced mesh velocity from this edge for adjoint transpose matrix multiply**/

    void dInducedVelocity_dMesh_Transpose(double xyz_p[3],
                                          double &dq0_dx1,
                                          double &dq0_dy1,
                                          double &dq0_dz1,
                                          double &dq0_dx2,
                                          double &dq0_dy2,
                                          double &dq0_dz2,
                                                         
                                          double &dq1_dx1,
                                          double &dq1_dy1,
                                          double &dq1_dz1,
                                          double &dq1_dx2,
                                          double &dq1_dy2,
                                          double &dq1_dz2,
                                                         
                                          double &dq2_dx1,
                                          double &dq2_dy1,
                                          double &dq2_dz1,
                                          double &dq2_dx2,
                                          double &dq2_dy2,
                                          double &dq2_dz2,
                                          
                                          double &dq0_dxp,
                                          double &dq0_dyp,
                                          double &dq0_dzp,
                                                             
                                          double &dq1_dxp,
                                          double &dq1_dyp,
                                          double &dq1_dzp,
                                                               
                                          double &dq2_dxp,
                                          double &dq2_dyp,
                                          double &dq2_dzp,
                                                                                         
                                          double &dq0_dMach,
                                          double &dq1_dMach,
                                          double &dq2_dMach);    

    /** Calculate the induced mesh velocity from this edge for the forward matrix multiply **/

    void dInducedVelocity_dMesh_dMesh_Forward(double &dq0_dx1,
                                              double &dq0_dy1,
                                              double &dq0_dz1,
                                              double &dq0_dx2,
                                              double &dq0_dy2,
                                              double &dq0_dz2,
                                                              
                                              double &dq1_dx1,
                                              double &dq1_dy1,
                                              double &dq1_dz1,
                                              double &dq1_dx2,
                                              double &dq1_dy2,
                                              double &dq1_dz2,
                                                                  
                                              double &dq2_dx1,
                                              double &dq2_dy1,
                                              double &dq2_dz1,
                                              double &dq2_dx2,
                                              double &dq2_dy2,
                                              double &dq2_dz2,
                                                          
                                              double &dq0_dxp,
                                              double &dq0_dyp,
                                              double &dq0_dzp,
                                                          
                                              double &dq1_dxp,
                                              double &dq1_dyp,
                                              double &dq1_dzp,
                                                          
                                              double &dq2_dxp,
                                              double &dq2_dyp,
                                              double &dq2_dzp,    
                                                                                                                                          
                                              double Coef[3][9]);
                                                                                            
    /** Calculate the induced mesh velocity from this edge for adjoint transpose matrix multiply **/

    void dInducedVelocity_dMesh_dMesh_Transpose(double &dq0_dx1,
                                                double &dq0_dy1,
                                                double &dq0_dz1,
                                                double &dq0_dx2,
                                                double &dq0_dy2,
                                                double &dq0_dz2,
                                                                
                                                double &dq1_dx1,
                                                double &dq1_dy1,
                                                double &dq1_dz1,
                                                double &dq1_dx2,
                                                double &dq1_dy2,
                                                double &dq1_dz2,
                                                                    
                                                double &dq2_dx1,
                                                double &dq2_dy1,
                                                double &dq2_dz1,
                                                double &dq2_dx2,
                                                double &dq2_dy2,
                                                double &dq2_dz2,
                                                            
                                                double &dq0_dxp,
                                                double &dq0_dyp,
                                                double &dq0_dzp,
                                                            
                                                double &dq1_dxp,
                                                double &dq1_dyp,
                                                double &dq1_dzp,
                                                            
                                                double &dq2_dxp,
                                                double &dq2_dyp,
                                                double &dq2_dzp,    
                                                                                                                                            
                                                double Coef[9]);

    /** Calculate the induced mesh velocity from this edge for unsteady adjoint transpose matrix multiply **/

    void dInducedVelocity_dMesh_dMesh_Unsteady_Transpose(double &dq0_dx1,
                                                         double &dq0_dy1,
                                                         double &dq0_dz1,
                                                         double &dq0_dx2,
                                                         double &dq0_dy2,
                                                         double &dq0_dz2,
                                                                         
                                                         double &dq1_dx1,
                                                         double &dq1_dy1,
                                                         double &dq1_dz1,
                                                         double &dq1_dx2,
                                                         double &dq1_dy2,
                                                         double &dq1_dz2,
                                                                             
                                                         double &dq2_dx1,
                                                         double &dq2_dy1,
                                                         double &dq2_dz1,
                                                         double &dq2_dx2,
                                                         double &dq2_dy2,
                                                         double &dq2_dz2,
                                                                     
                                                         double &dq0_dxp,
                                                         double &dq0_dyp,
                                                         double &dq0_dzp,
                                                                     
                                                         double &dq1_dxp,
                                                         double &dq1_dyp,
                                                         double &dq1_dzp,
                                                                     
                                                         double &dq2_dxp,
                                                         double &dq2_dyp,
                                                         double &dq2_dzp,    
                                                         
                                                         double &DeltaTime,
                                                                                                                                                     
                                                         double Coef[9]);
                                                
    /** Calculate Psi_T times partial_Residual_Partial_Mesh **/
    
    void PsiT_pR_pMesh(double &dq0_dx1,
                       double &dq0_dy1,
                       double &dq0_dz1,
                       double &dq0_dx2,
                       double &dq0_dy2,
                       double &dq0_dz2,
                                       
                       double &dq1_dx1,
                       double &dq1_dy1,
                       double &dq1_dz1,
                       double &dq1_dx2,
                       double &dq1_dy2,
                       double &dq1_dz2,
                                           
                       double &dq2_dx1,
                       double &dq2_dy1,
                       double &dq2_dz1,
                       double &dq2_dx2,
                       double &dq2_dy2,
                       double &dq2_dz2,
                                   
                       double &dq0_dxp,
                       double &dq0_dyp,
                       double &dq0_dzp,
                                   
                       double &dq1_dxp,
                       double &dq1_dyp,
                       double &dq1_dzp,
                                   
                       double &dq2_dxp,
                       double &dq2_dyp,
                       double &dq2_dzp,    

                       double Psi[3],
                                                                                                 
                       double Coef[9]);
   
    /** Calculate unsteady Psi_T times partial_Residual_Partial_Mesh **/
                       
    void UnsteadyPsiT_pR_pMesh(double &dq0_dx1,
                               double &dq0_dy1,
                               double &dq0_dz1,
                               double &dq0_dx2,
                               double &dq0_dy2,
                               double &dq0_dz2,
                                               
                               double &dq1_dx1,
                               double &dq1_dy1,
                               double &dq1_dz1,
                               double &dq1_dx2,
                               double &dq1_dy2,
                               double &dq1_dz2,
                                                   
                               double &dq2_dx1,
                               double &dq2_dy1,
                               double &dq2_dz1,
                               double &dq2_dx2,
                               double &dq2_dy2,
                               double &dq2_dz2,
                                           
                               double &dq0_dxp,
                               double &dq0_dyp,
                               double &dq0_dzp,
                                           
                               double &dq1_dxp,
                               double &dq1_dyp,
                               double &dq1_dzp,
                                           
                               double &dq2_dxp,
                               double &dq2_dyp,
                               double &dq2_dzp,    
                               
                               double Psi[3],
                               
                               double &DeltaTime,
                                                                                               
                               double Coef[9]);
                                                                                                      
    /** Calculate the induced mesh velocity from this edge for adjoint transpose matrix multiply**/

    void dInducedVelocity_dMesh_dMesh_Transpose(double xyz_p[3], double Coef[6]);
                                             
    /** Calculate forces acting on this edge **/
    
    void CalculateForces(void);

    /** Calculate partial of the forces wrt gamma, holding velocity constant **/
    
    void CalculateForces_dGamma(double dForce[3]);
    
    /** Calculate partial of the Trefftz forces wrt gamma, holding velocity constant **/
    
    void CalculateTrefftzForces_dGamma(double dForce[3]);
        
    /** Calculate delta forces acting on this edge given a velocity **/
    
    void CalculateForces_dVelocity(double dVelocity[3], double dForce[3]);
    
    /** Calculate gradient of forces acting on this edge with respect to the mesh **/
    
    void CalculateForces_dMesh(double dForce[18]);
    
    /** Calculate gradient of Trefftz forces acting on this edge with respect to the mesh **/
    
    void CalculateTrefftzForces_dMesh(double dForce[18]);
    
    /** Calculate the induced drag (forces) acting on this edge ***/
    
    void CalculateTrefftzForces(void);

    /** Partial derivative of forces wrt free stream **/
    
    void pForces_pVinf(double pForces_pVinf[3][3]);

    /** Partial derivative of moments wrt free stream **/
    
    void pMoments_pVinf(double MomArm[3], double pMoments_pVinf[3][3]);
    
    /** Partial derivative of forces wrt Mach **/

    void pForces_pMach(double pForces_pMach[3]);

    /** Partial derivative of moments wrt Mach **/

    void pMoments_pMach(double MomArm[3], double pMoments_pMach[3]);

    /** Partial derivative of Trefftz forces wrt free stream **/

    void pTrefftzForces_pVinf(double pForces_pVinf[3][3]);

    /** Partial derivative of Trefftz forces wrt Mach **/

    void pTrefftzForces_pMach(double pForces_pMach[3]);
    
    /** Calculate the generalized principal part of the downwash integral for this edge **/

    double GeneralizedPrincipalPartOfDownWash(void);

    /** Determine if this edge is swept in front, or behind the Mach cone... hence (super,sub) sonic **/
    
    int IsSupersonic(void);
  
    /** Set zero tolerances for this edge **/
    
    void SetTolerance(double Tolerance);

    /** Mach number for this edge... **/
    
    void SetMachNumber(double Mach);

    /** Edge Mach number **/
    
    double Mach(void) { return Mach_; };

    /** Karman-Tsien factor for this edge **/
    
    double &KTFact(void) { return KTFact_; };

    /** Vortex loop 1 is down wind of this edge, or not ... **/
    
    /** Trailing edge stall factor **/
    
    double &TrailingEdgeStallFactor(void) { return TrailingEdgeStallFactor_; };
        
    int &VortexLoopLIsDownWind(void) { return VortexLoopLIsDownWind_; };
    
    /** Vortex loop 2 is down wind of this edge, or not ... **/
    
    int &VortexLoopRIsDownWind(void) { return VortexLoopRIsDownWind_; };
    
    /** Edge edge contributes, a possibly zero, contribution to loops that are down wind
     * of said edge. This is the fraction that this edge provides to Loop 1... assuming
     * Loop 1 is down wind... 
     **/
    
    double &VortexLoopLDownWindWeight(void) { return VortexLoopLDownWindWeight_; };
    
    /** Edge edge contributes, a possibly zero, contribution to loops that are down wind
     * of said edge. This is the fraction that this edge provides to Loop 2... assuming
     * Loop 2 is down wind... 
     **/
         
    double &VortexLoopRDownWindWeight(void) { return VortexLoopRDownWindWeight_; };
    
    /** Circulation strength for this edge **/
    
    double &Gamma(void) { return Gamma_; };

    /** Perturbation circulation strength for this edge **/
    
    double &dGamma(void) { return DeltaGamma_; };
    
    /** Perturbation stall circulation strength for this edge **/
    
    double &dStallGamma(void) { return DeltaStallGamma_; };
    
    /** Perturbation vortex stretching strength for this edge **/
    
    double &dVortexStretchingRatioGamma(void) { return DeltaVortexStretchingRatioGamma_; };
        
    /** Force in the x direction acting on this edge **/
    
    double &Fx(void) { return Forces_[0]; };
    
    /** Force in the y direction acting on this edge **/
    
    double &Fy(void) { return Forces_[1]; };

    /** Force in the z direction acting on this edge **/

    double &Fz(void) { return Forces_[2]; };
    
    /** Unsteady force in the x direction acting on this edge **/
           
    double &Unsteady_Fx(void) { return Unsteady_Forces_[0]; };
    
    /** Unsteady force in the y direction acting on this edge **/
    
    double &Unsteady_Fy(void) { return Unsteady_Forces_[1]; };

    /** Unsteady force in the z direction acting on this edge **/

    double &Unsteady_Fz(void) { return Unsteady_Forces_[2]; };   
    
    /** Induced (drag) force in the x direction acting on this edge **/
    
    double &Trefftz_Fx(void) { return Trefftz_Forces_[0]; };

    /** Induced (drag) force in the y direction acting on this edge **/
    
    double &Trefftz_Fy(void) { return Trefftz_Forces_[1]; };

    /** Induced (drag) force in the z direction acting on this edge **/

    double &Trefftz_Fz(void) { return Trefftz_Forces_[2]; };

    /** X coordinate for edge node 1 **/
    
    double X1(void) { return X1_; };
    
    /** Y coordinate for edge node 1 **/
    
    double Y1(void) { return Y1_; };
    
    /** Z coordinate for edge node 1 **/
    
    double Z1(void) { return Z1_; };

    /** X coordinate for edge node 2 **/
    
    double X2(void) { return X2_; };

    /** Y coordinate for edge node 2 **/

    double Y2(void) { return Y2_; };

    /** Z coordinate for edge node 2 **/

    double Z2(void) { return Z2_; };
    
    /** X coordinate for edge node 1 at previous time step **/
    
    double &X1m(void) { return X1m_; };
    
    /** Y coordinate for edge node 1 at previous time step **/
    
    double &Y1m(void) { return Y1m_; };
    
    /** Z coordinate for edge node 1 at previous time step **/
    
    double &Z1m(void) { return Z1m_; };

    /** X coordinate for edge node 2 at previous time step **/
    
    double &X2m(void) { return X2m_; };

    /** Y coordinate for edge node 2 at previous time step **/

    double &Y2m(void) { return Y2m_; };

    /** Z coordinate for edge node 2 at previous time step **/

    double &Z2m(void) { return Z2m_; };

    /** delta X coordinate perturbation for edge node 1 **/
    
    double &dX1(void) { return dxyz1_[0]; };
    
    /** delta Y coordinate perturbation for edge node 1 **/
    
    double &dY1(void) { return dxyz1_[1]; };
    
    /** delta Z coordinate perturbation for edge node 1 **/
    
    double &dZ1(void) { return dxyz1_[2]; };

    /** delta XYZ coordinates perturbation for edge node 1 **/
    
    double *dXYZ1(void) { return dxyz1_; };
    
    /** delta X coordinate perturbation for edge node 2 **/
    
    double &dX2(void) { return dxyz2_[0]; };

    /** delta Y coordinate perturbation for edge node 2 **/

    double &dY2(void) { return dxyz2_[1]; };

    /** delta Z coordinate perturbation for edge node 2 **/

    double &dZ2(void) { return dxyz2_[2]; };

    /** delta XYZ coordinates perturbation for edge node 1 **/
    
    double *dXYZ2(void) { return dxyz2_; };
    
    /** X coordinate of edge centroid **/
    
    double Xc(void) { return XYZc_[0]; };
    
    /** Y coordinate of edge centroid **/
    
    double Yc(void) { return XYZc_[1]; };

    /** Z coordinate of edge centroid **/

    double Zc(void) { return XYZc_[2]; };
    
    /** XYZ vector of edge centroid coordinates **/
    
    double *xyz_c(void) { return XYZc_; };

    /** Edge normal, this is an average of the left and right loop normals **/
    
    double *Normal(void) { return Normal_; };

    /** Local free stream components **/
    
    double &LocalFreeStreamVelocity(int i) { return LocalFreeStreamVelocity_[i]; };
    
    /** Local free stream vector **/
    
    double *LocalFreeStreamVelocity(void) { return LocalFreeStreamVelocity_; };
    
    /** X component of velocity for this edge **/
    
    double &U(void) { return Velocity_[0]; };
    
    /** Y component of velocity for this edge **/
    
    double &V(void) { return Velocity_[1]; };

    /** Z component of velocity for this edge **/

    double &W(void) { return Velocity_[2]; };
    
    /** Velocity magnitude for this edge **/
    
    double &Q(void) { return Q_; };
    
    /** Velocity vector of velocity components for this edge **/
    
    double *Velocity(void) { return Velocity_; };
    
    /** X component of down wash + free stream velocity for this edge **/
    
    double &DownWash_U(void) { return DownWashVelocity_[0]; };
    
    /** Y component of down wash + free stream velocity for this edge **/
    
    double &DownWash_V(void) { return DownWashVelocity_[1]; };

    /** Z component of down wash + free stream velocity for this edge **/

    double &DownWash_W(void) { return DownWashVelocity_[2]; };
    
    /** Velocity vector of down wash + free stream components for this edge **/
    
    double *DownWash_Velocity(void) { return DownWashVelocity_; };
        
    /** X component of gamma perturbation velocity for this edge **/
    
    double &dU_dGamma(void) { return dVelocity_dGamma_[0]; };
    
    /** Y component of gamma perturbation velocity for this edge **/
    
    double &dV_dGamma(void) { return dVelocity_dGamma_[1]; };

    /** Z component of gamma perturbation velocity for this edge **/

    double &dW_dGamma(void) { return dVelocity_dGamma_[2]; };
    
    /** Velocity vector of gamma perturbation velocity components for this edge **/
    
    double *dVelocity_dGamma(void) { return dVelocity_dGamma_; };

    /** X component of mesh perturbation velocity for this edge **/
    
    double &dU_dMesh(void) { return dVelocity_dMesh_[0]; };
    
    /** Y component of mesh perturbation velocity for this edge **/
    
    double &dV_dMesh(void) { return dVelocity_dMesh_[1]; };

    /** Z component of mesh perturbation velocity for this edge **/

    double &dW_dMesh(void) { return dVelocity_dMesh_[2]; };
    
    /** Velocity vector of mesh perturbation velocity components for this edge **/
    
    double *dVelocity_dMesh(void) { return dVelocity_dMesh_; };

    /** X component of Mach perturbation velocity for this edge **/
    
    double &dU_dMach(void) { return dVelocity_dMach_[0]; };
    
    /** Y component of Mach perturbation velocity for this edge **/
    
    double &dV_dMach(void) { return dVelocity_dMach_[1]; };

    /** Z component of Mach perturbation velocity for this edge **/

    double &dW_dMach(void) { return dVelocity_dMach_[2]; };
    
    /** Velocity vector of Mach perturbation velocity components for this edge **/
    
    double *dVelocity_dMach(void) { return dVelocity_dMach_; };
    
    /** X component of stall factor perturbation velocity for this edge **/
    
    double &dU_dStall(void) { return dVelocity_dStall_[0]; };
    
    /** Y component of stall factor perturbation velocity for this edge **/
    
    double &dV_dStall(void) { return dVelocity_dStall_[1]; };

    /** Z component of stall factor perturbation velocity for this edge **/

    double &dW_dStall(void) { return dVelocity_dStall_[2]; };
    
    /** Velocity vector of stall factor perturbation velocity components for this edge **/
    
    double *dVelocity_dStall(void) { return dVelocity_dStall_; };
    
    /** U component of vortex stretching perturbation velocity... **/
    
    double &dU_dRatio(void) { return dVelocity_dRatio_[0]; };
    
    /** V component of vortex stretching factor perturbation velocity... **/
    
    double &dV_dRatio(void) { return dVelocity_dRatio_[1]; };

    /** W component of vortex stretching factor perturbation velocity... **/

    double &dW_dRatio(void) { return dVelocity_dRatio_[2]; };

    /** Perturbation vortex stretching factor velocity vector... **/
    
    double *dVelocity_dRatio(void) { return dVelocity_dRatio_; };         
    
    /** Node 1 adjoint perturbation velocities **/
    
    double &dq0_dx1(void) { return dq0_dxyz_[0][0]; };
    double &dq0_dy1(void) { return dq0_dxyz_[0][1]; };
    double &dq0_dz1(void) { return dq0_dxyz_[0][2]; };

    double &dq1_dx1(void) { return dq1_dxyz_[0][0]; };
    double &dq1_dy1(void) { return dq1_dxyz_[0][1]; };
    double &dq1_dz1(void) { return dq1_dxyz_[0][2]; };
    
    double &dq2_dx1(void) { return dq2_dxyz_[0][0]; };
    double &dq2_dy1(void) { return dq2_dxyz_[0][1]; };
    double &dq2_dz1(void) { return dq2_dxyz_[0][2]; };
        
    /** Node 2 adjoint perturbation velocities **/
        
    double &dq0_dx2(void) { return dq0_dxyz_[1][0]; };
    double &dq0_dy2(void) { return dq0_dxyz_[1][1]; };
    double &dq0_dz2(void) { return dq0_dxyz_[1][2]; };
                        
    double &dq1_dx2(void) { return dq1_dxyz_[1][0]; };
    double &dq1_dy2(void) { return dq1_dxyz_[1][1]; };
    double &dq1_dz2(void) { return dq1_dxyz_[1][2]; };
                        
    double &dq2_dx2(void) { return dq2_dxyz_[1][0]; };
    double &dq2_dy2(void) { return dq2_dxyz_[1][1]; };
    double &dq2_dz2(void) { return dq2_dxyz_[1][2]; };
        
    /** Update the geometry information for this edge by rotating and translating ... input
     * is a translation vector, TVec, a rotation origin, and a quaternion and it's inverse **/
    
    void UpdateGeometryLocation(double *TVec, double *OVec, QUAT &Quat, QUAT &InvQuat);
    
    /** Update geometry location given updates to the 2 defining nodes for this edge **/
    
    void UpdateGeometryLocation(VSP_NODE &Node1, VSP_NODE &Node2);

    /** Create data structure for wake edge equations **/

    void CreateWakeEdgeSpace(void);
        
    /** Flag to determine if this edge lies on the wake **/
    
    int IsWakeEdge(void) { return IsWakeEdge_; };
    
    /** Wake edge data **/
    
    WAKE_EDGE &WakeEdgeData(void) { return *WakeEdgeData_; };

    /** Size the right adjoint edge list **/
    
    void SizeAdjointEdgeList(int NumberOfEdges);
        
    /** Number of adjoint fine grid edges **/
    
    int NumberOfAdjointEdges(void) { return NumberOfAdjointEdges_; };
    
    /** List of fine grid adjoint edges **/
    
    int &AdjointEdgeList(int i) { return AdjointEdgeList_[i]; };  
    
    /** Adjoint edge weight **/
    
    double &AdjointEdgeFactor(int i) { return AdjointEdgeFactor_[i]; };
            
    /** Calculate residual contribution of this edge to wake equations **/
    
    void CalculateWakeResidual(double *Residual);

    /** Calculate unsteady residual contribution of this edge to wake equations **/
    
    void CalculateUnsteadyWakeResidualOld(double *Residual, double &DeltaTime);
    
    void CalculateUnsteadyWakeResidual(double *Residual, double &DeltaTime);
    
    /** Calculate the Jacobian matrix terms **/
    
    void CalculateWakeJacobian(void);
    
    /** Do forward solve matrix multiply **/
    
    void WakeMatrixMultiply(double *VecIn, double *VecOut);

    /** Do forward unsteady solve matrix multiply **/

    void UnsteadyWakeMatrixMultiply(double *VecIn, double *VecOut, double &DeltaTime);

    /** Do forward matrix preconditioning **/
    
    void DoWakeForwardMatrixPrecondition(double *VecIn);
   
    /** Do adjoint solver adjoint tranpose matrix multiply of gamma perturbation terms **/
    
    double WakeTranposeMatrixMultiply(double q[3][3]);

    /** Do adjoint solver unsteady adjoint tranpose matrix multiply of gamma perturbation terms **/

    double UnsteadyWakeTranposeMatrixMultiply(double q[3][3], double &DeltaTime);

    /** Do adjoint solver adjoint tranpose matrix multiply of mesh pertrubation terms **/

    void WakeTranposeMatrixMultiplyMeshPerturbations(double *VecIn, double *VecOut);
    
    /** Do adjoint solver unsteady adjoint tranpose matrix multiply of mesh pertrubation terms **/
    
    void UnsteadyWakeTranposeMatrixMultiplyMeshPerturbations(double *VecIn, double *VecOut);
    
    /** Calculate partial of wake residuals wrt free stream velocity **/
    
    void Wake_pResidual_pVinf(double pResidual_pVinf[3][3]);
    
    void Wake_pResidual_pVinfOld(double pResidual_pVinf[3][3]);

    /** Calculate partial of unsteady wake residuals wrt free stream velocity **/
    
    void UnsteadyWake_pResidual_pVinf(double pResidual_pVinf[3][3], double &DeltaTime);
    
    /** Partial derivative of wake residual wrt Mach **/
    
    void Wake_pResidual_pMach(double pResidual_pMach[3]);
    
    /** Partial derivative of wake residual wrt Mach **/
    
    void UnsteadyWake_pResidual_pMach(double pResidual_pMach[3], double &DeltaTime);
    

};

#include "END_NAME_SPACE.H"

#endif
