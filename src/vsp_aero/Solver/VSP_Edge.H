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
 
    // Edge based data

    int Node1_;
    int Node2_;

    int Loop1_;
    int Loop2_;
    
    int LoopL_;
    int LoopR_;
    
    int VortexLoop1_;
    int VortexLoop2_;
    
    int VortexEdge_;
    
    int IsTrailingEdge_;
    int IsLeadingEdge_;
    int IsBoundaryEdge_;
    
    int CoarseGridEdge_;
    int FineGridEdge_;
    int Level_;
    
    VSP_EDGE *Child1_;
    VSP_EDGE *Child2_;
    
    VSPAERO_DOUBLE ReferenceLength_;
       
    VSPAERO_DOUBLE Sigma_;
    VSPAERO_DOUBLE S_;
    VSPAERO_DOUBLE T_;
    VSPAERO_DOUBLE Length_;

    VSPAERO_DOUBLE Forces_[3];
    VSPAERO_DOUBLE Unsteady_Forces_[3];

    VSPAERO_DOUBLE Trefftz_Forces_[3];
    
    VSPAERO_DOUBLE InducedForces_[3];
    
    VSPAERO_DOUBLE MinCoreWidth_;
    VSPAERO_DOUBLE CoreWidth_;
    VSPAERO_DOUBLE SuperSonicCoreWidth_;

//djk


    int NoInfluence_;
    
 //   VSPAERO_DOUBLE Xp_;
 //   VSPAERO_DOUBLE Yp_;
 //   VSPAERO_DOUBLE Zp_;
    
    VSPAERO_DOUBLE U2_;
    VSPAERO_DOUBLE U4_;
    
    VSPAERO_DOUBLE V2_;
    VSPAERO_DOUBLE V4_;
    
    VSPAERO_DOUBLE W2_;
    VSPAERO_DOUBLE W4_;
    
    VSPAERO_DOUBLE C_Gamma_;

    VSPAERO_DOUBLE a_;
    VSPAERO_DOUBLE b_;
    VSPAERO_DOUBLE c_;
    VSPAERO_DOUBLE d_;

    VSPAERO_DOUBLE dx_;
    VSPAERO_DOUBLE dy_;
    VSPAERO_DOUBLE dz_;
    
    VSPAERO_DOUBLE s_;
    VSPAERO_DOUBLE F_;
    VSPAERO_DOUBLE F1_;
    VSPAERO_DOUBLE F2_;
    VSPAERO_DOUBLE Test_;
    
    VSPAERO_DOUBLE R_;
    VSPAERO_DOUBLE Denom_; 
    VSPAERO_DOUBLE C1_; 
    
// djk

    // Vortex solution data
    
    int Verbose_;
 
    // Wing and edge, or trailing node this vortex belongs to
    
    int DegenWing_;
    int DegenBody_;
    int SurfaceID_;
    int ComponentID_;
    int GeomID_;

    // Edge nodes - true geometry (camber line, or actual)
    
    VSPAERO_DOUBLE X1_;
    VSPAERO_DOUBLE Y1_;
    VSPAERO_DOUBLE Z1_;
    
    VSPAERO_DOUBLE X2_;
    VSPAERO_DOUBLE Y2_;
    VSPAERO_DOUBLE Z2_;

    // Edge centroid
    
    VSPAERO_DOUBLE XYZc_[3];

    VSP_NODE VortexCentroid_;

    // Edge direction vector
    
    VSPAERO_DOUBLE Vec_[3];
    VSPAERO_DOUBLE u_;
    VSPAERO_DOUBLE v_;
    VSPAERO_DOUBLE w_;
    
    // Edge Normal
    
    VSPAERO_DOUBLE Normal_[3];
    
    // Distance tolerances for velocity evaluations
   
    static double Tolerance_1_;
    static double Tolerance_2_;
    static double Tolerance_4_;

    // Mach number and Beta
    
    VSPAERO_DOUBLE Mach_;
    
    VSPAERO_DOUBLE Kappa_;

    VSPAERO_DOUBLE Beta2_;

    // KT value
    
    VSPAERO_DOUBLE KTFact_;

    // Flag dermining which loops are down wind of this vortex edge
    
    int VortexLoop1IsDownWind_;
    int VortexLoop2IsDownWind_;
    
    VSPAERO_DOUBLE VortexLoop1DownWindWeight_;
    VSPAERO_DOUBLE VortexLoop2DownWindWeight_;

    // Circulation strength
    
    VSPAERO_DOUBLE Gamma_;
    
    // Edge matrix coefficients
    
    VSPAERO_DOUBLE EdgeCoef_[2];

    // Setup routines
    
    void Setup_(VSP_NODE &Node1, VSP_NODE &Node2);

    // Induced velocities
    
    void OldBoundVortex(VSPAERO_DOUBLE xyz_p[3], VSPAERO_DOUBLE q[3]);

    void NewBoundVortex(VSPAERO_DOUBLE xyz_p[3], VSPAERO_DOUBLE q[3]);

    VSPAERO_DOUBLE Fint(void);
    VSPAERO_DOUBLE Gint(void);

    VSPAERO_DOUBLE Fint(VSPAERO_DOUBLE &a, VSPAERO_DOUBLE &b, VSPAERO_DOUBLE &c, VSPAERO_DOUBLE &d, VSPAERO_DOUBLE &s);
    VSPAERO_DOUBLE Gint(VSPAERO_DOUBLE &a, VSPAERO_DOUBLE &b, VSPAERO_DOUBLE &c, VSPAERO_DOUBLE &d, VSPAERO_DOUBLE &s);
    
    void FindLineConicIntersection(VSPAERO_DOUBLE &Xp, VSPAERO_DOUBLE &Yp, VSPAERO_DOUBLE &Zp,
                                   VSPAERO_DOUBLE &X1, VSPAERO_DOUBLE &Y1, VSPAERO_DOUBLE &Z1,
                                   VSPAERO_DOUBLE &Xd, VSPAERO_DOUBLE &Yd, VSPAERO_DOUBLE &Zd,
                                   VSPAERO_DOUBLE &t1, VSPAERO_DOUBLE &t2);    

    // Velocities

    VSPAERO_DOUBLE Velocity_[3];
    VSPAERO_DOUBLE DownWash_Velocity_[3];                                 

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
    
    /** Vortex Loop 1 attached to this edge **/
    
    int &VortexLoop1(void) { return VortexLoop1_; };
    
    /** Vortex Loop 2 attached to this edge **/
    
    int &VortexLoop2(void) { return VortexLoop2_; };

    /** Finite ore size **/
     
    VSPAERO_DOUBLE &Sigma(void) { return Sigma_; };
    
    /** Edge Length **/
    
    VSPAERO_DOUBLE &S(void) { return S_; };
    
    /** Edge age, used in trailing wake, finite core size, model **/
    
    VSPAERO_DOUBLE &T(void) { return T_; };    
    
    /** Edge length **/
    
    VSPAERO_DOUBLE &Length(void) { return Length_; };
    
    /** Edge is on trailing edge of lifting surface **/
    
    int &IsTrailingEdge(void) { return IsTrailingEdge_; };
    
    /** Edge on leading edge of lifting surface **/
    
    int &IsLeadingEdge(void) { return IsLeadingEdge_; };
    
    /** Edge is on boundary edge (intersection) of two surface **/
    
    int &IsBoundaryEdge(void) { return IsBoundaryEdge_; };
    
    /** Coarse grid edge that this edge was agglomerated into **/
    
    int &CoarseGridEdge(void) { return CoarseGridEdge_; };
    
    /** Fine grid edge that this edge was created from... note that this is not unique... we only store one **/
  
    int &FineGridEdge(void) { return FineGridEdge_; };
    
    /** Agglomerated grid level this edge sits on **/
    
    int &Level(void) { return Level_; };
    
    /** Pointer to children of this edge... this is used in the trailing wake routines ... for
     * the local agglomeration of trailing wakes **/

    void SetupChildren(VSP_EDGE &Child1, VSP_EDGE &Child2) { Child1_ = &Child1 ; Child2_ = &Child2; };

    /** Flag to determine if this edge has children... this is used in the trailing wake routines ... for
     * the local agglomeration of trailing wakes **/
     
    int ThereAreChildren(void) { if ( Child1_ != NULL ) { return 1; } else { return 0; }; };
    
    /** Child 1 for this edge... may be 0 **/
    
    VSP_EDGE &Child1(void) { return *Child1_; };
    
    /** Child 2 for this edge... may be 0 **/
    
    VSP_EDGE &Child2(void) { return *Child2_; };
    
    /** Reference length for this edge... this may differ from the actual length for trailing
     * wakes where we have agglomerated edges and hence lost any curvature information. This
     * is an attempt to keep track of the actual length of the segment we have formed through
     * agglomeration
     **/
    
    VSPAERO_DOUBLE &ReferenceLength(void) { return ReferenceLength_; };

    /** Edge is on degen wing **/

    int &DegenWing(void) { return DegenWing_; };
    
    /** Edge is on degen body **/
    
    int &DegenBody(void) { return DegenBody_; };
    
    /** Edge is on Surface **/
    
    int &SurfaceID(void) { return SurfaceID_; };
    
    /** Edge is on Component **/
    
    int &ComponentID(void) { return ComponentID_; };
    
    /** Tied to OpenVSP unique GeomID flag **/
    
    int &GeomID(void) { return GeomID_; };
    
    /** Set up the edge data structure given these 2 mesh nodes **/

    void Setup(VSP_NODE &Node1, VSP_NODE &Node2) { Setup_(Node1,Node2); };
                               
    /** Update the edge data given these two mesh nodes .. we assume the initial set was performed
     * and we are just updating the location of edge node xyz locations and data that depends
     * on them... **/
                                  
    void Update(VSP_NODE &Node1, VSP_NODE &Node2) { Setup_(Node1,Node2); };
    
    /** Calculate the induced velocity from this edge **/

    void InducedVelocity(VSPAERO_DOUBLE xyz_p[3], VSPAERO_DOUBLE q[3]);
    
    /** Calculate the induced velocity from this edge, assuming a finite core model **/
    
    void InducedVelocity(VSPAERO_DOUBLE xyz_p[3], VSPAERO_DOUBLE q[3], VSPAERO_DOUBLE CoreWidth);
    
    /** Calculate forces acting on this edge **/
    
    void CalculateForces(void);
   
    /** Calculate the induced drag (forces) acting on this edge ***/
    
    void CalculateTrefftzForces(VSPAERO_DOUBLE WakeInducedVelocity[3]);
    
    /** Calculate the generalized principal part of the downwash integral for this edge **/

    VSPAERO_DOUBLE GeneralizedPrincipalPartOfDownWash(void);

    /** Determine if this edge is swept in front, or behind the Mach cone... hence (super,sub) sonic **/
    
    int IsSupersonic(void);
  
    /** Set zero tolerances for this edge **/
    
    void SetTolerance(VSPAERO_DOUBLE Tolerance);

    /** Mach number for this edge... it's actually static across ALL edges **/
    
    void SetMachNumber(VSPAERO_DOUBLE Mach);

    /** Edge Mach number **/
    
    VSPAERO_DOUBLE Mach(void) { return Mach_; };

    /** Karman-Tsien factor for this edge **/
    
    VSPAERO_DOUBLE &KTFact(void) { return KTFact_; };

    /** Vortex loop 1 is down wind of this edge, or not ... **/
    
    int &VortexLoop1IsDownWind(void) { return VortexLoop1IsDownWind_; };
    
    /** Vortex loop 2 is down wind of this edge, or not ... **/
    
    int &VortexLoop2IsDownWind(void) { return VortexLoop2IsDownWind_; };
    
    /** Edge edge contributes, a possibly zero, contribution to loops that are down wind
     * of said edge. This is the fraction that this edge provides to Loop 1... assuming
     * Loop 1 is down wind... 
     **/
    
    VSPAERO_DOUBLE &VortexLoop1DownWindWeight(void) { return VortexLoop1DownWindWeight_; };
    
    /** Edge edge contributes, a possibly zero, contribution to loops that are down wind
     * of said edge. This is the fraction that this edge provides to Loop 2... assuming
     * Loop 2 is down wind... 
     **/
         
    VSPAERO_DOUBLE &VortexLoop2DownWindWeight(void) { return VortexLoop2DownWindWeight_; };
    
    /** Circulation strength for this edge **/
    
    VSPAERO_DOUBLE &Gamma(void) { return Gamma_; };
    
    /** Edge coefficient data.. this is used for the sparse edge ssor preconditioning 
     * routine that is used by GMRES in the solution of the linear system of equations
     * formed in the solution of the flow equations **/

    VSPAERO_DOUBLE &EdgeCoef(int i) { return EdgeCoef_[i]; };
    
    /** Force in the x direction acting on this edge **/
    
    VSPAERO_DOUBLE &Fx(void) { return Forces_[0]; };
    
    /** Force in the y direction acting on this edge **/
    
    VSPAERO_DOUBLE &Fy(void) { return Forces_[1]; };

    /** Force in the z direction acting on this edge **/

    VSPAERO_DOUBLE &Fz(void) { return Forces_[2]; };
    
    /** Unsteady force in the x direction acting on this edge **/
           
    VSPAERO_DOUBLE &Unsteady_Fx(void) { return Unsteady_Forces_[0]; };
    
    /** Unsteady force in the y direction acting on this edge **/
    
    VSPAERO_DOUBLE &Unsteady_Fy(void) { return Unsteady_Forces_[1]; };

    /** Unsteady force in the z direction acting on this edge **/

    VSPAERO_DOUBLE &Unsteady_Fz(void) { return Unsteady_Forces_[2]; };   
    
    /** Induced (drag) force in the x direction acting on this edge **/
    
    VSPAERO_DOUBLE &Trefftz_Fx(void) { return Trefftz_Forces_[0]; };

    /** Induced (drag) force in the y direction acting on this edge **/
    
    VSPAERO_DOUBLE &Trefftz_Fy(void) { return Trefftz_Forces_[1]; };

    /** Induced (drag) force in the z direction acting on this edge **/

    VSPAERO_DOUBLE &Trefftz_Fz(void) { return Trefftz_Forces_[2]; };

    /** X coordinate for edge node 1 **/
    
    VSPAERO_DOUBLE X1(void) { return X1_; };
    
    /** Y coordinate for edge node 1 **/
    
    VSPAERO_DOUBLE Y1(void) { return Y1_; };
    
    /** Z coordinate for edge node 1 **/
    
    VSPAERO_DOUBLE Z1(void) { return Z1_; };

    /** X coordinate for edge node 2 **/
    
    VSPAERO_DOUBLE X2(void) { return X2_; };

    /** Y coordinate for edge node 2 **/

    VSPAERO_DOUBLE Y2(void) { return Y2_; };

    /** Z coordinate for edge node 2 **/

    VSPAERO_DOUBLE Z2(void) { return Z2_; };

    /** X coordinate of edge centroid **/
    
    VSPAERO_DOUBLE Xc(void) { return XYZc_[0]; };
    
    /** Y coordinate of edge centroid **/
    
    VSPAERO_DOUBLE Yc(void) { return XYZc_[1]; };

    /** Z coordinate of edge centroid **/

    VSPAERO_DOUBLE Zc(void) { return XYZc_[2]; };
    
    /** XYZ vector of edge centroid coordinates **/
    
    VSPAERO_DOUBLE *xyz_c(void) { return XYZc_; };
    
    /** NODE object of edge centroid coordinates **/
    
    VSP_NODE &VortexCentroid(void) { return VortexCentroid_; };

    /** Unit vector for edge, pointing from node 1 to 2 **/
    
    VSPAERO_DOUBLE *Vec(void) { return Vec_; };    
    
    /** Edge normal, this is an average of the left and right loop normals **/
    
    VSPAERO_DOUBLE *Normal(void) { return Normal_; };

    /** X component of velocity for this edge **/
    
    VSPAERO_DOUBLE &U(void) { return Velocity_[0]; };
    
    /** Y component of velocity for this edge **/
    
    VSPAERO_DOUBLE &V(void) { return Velocity_[1]; };

    /** Z component of velocity for this edge **/

    VSPAERO_DOUBLE &W(void) { return Velocity_[2]; };
    
    /** Velocity vector of velocity components for this edge **/
    
    VSPAERO_DOUBLE *Velocity(void) { return Velocity_; };
    
    /** Down wash, ie velocity due to the wakes, in X direction for this edge **/
    
    VSPAERO_DOUBLE &DownWash_U(void) { return DownWash_Velocity_[0]; };
    
    /** Down wash, ie velocity due to the wakes, in Y direction for this edge **/
    
    VSPAERO_DOUBLE &DownWash_V(void) { return DownWash_Velocity_[1]; };

    /** Down wash, ie velocity due to the wakes, in Z direction for this edge **/

    VSPAERO_DOUBLE &DownWash_W(void) { return DownWash_Velocity_[2]; };

    /** Update the geometry information for this edge by rotating and translating ... input
     * is a translation vector, TVec, a rotation origin, and a quaternion and it's inverse **/
    
    void UpdateGeometryLocation(VSPAERO_DOUBLE *TVec, VSPAERO_DOUBLE *OVec, QUAT &Quat, QUAT &InvQuat);
    
    /** Update geometry location given updates to the 2 defining nodes for this edge **/
    
    void UpdateGeometryLocation(VSP_NODE &Node1, VSP_NODE &Node2);

};

#include "END_NAME_SPACE.H"

#endif
