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
    
    int EdgeType_;
    int IsTrailingEdge_;
    int IsLeadingEdge_;
    int IsBoundaryEdge_;
    
    int CourseGridEdge_;
    int FineGridEdge_;
    
    int ThereAreChildren_;
    VSP_EDGE *Child1_;
    VSP_EDGE *Child2_;
    
    int EdgeWasUsedForLoop_;
    
    double Sigma_;
    double S_;
    double Length_;
    double Forces_[3];
    double Trefftz_Forces_[3];
    double Unsteady_Forces_[3];
    
    // Vortex solution data
    
    int Verbose_;
 
    // Wing and edge, or trailing node this vortex belongs to
    
    int DegenWing_;
    int DegenBody_;
    int Cart3DSurface_;
    int Node_;

    // Rotation Quaternions
    
    QUAT Quat_;
    QUAT InverseQuat_;
    QUAT VecA_, VecB_;

    // Edge nodes - true geometry (camber line, or actual)
    
    double X1_;
    double Y1_;
    double Z1_;
    
    double X2_;
    double Y2_;
    double Z2_;

    // Edge centroid
    
    double XYZc_[3];

    VSP_NODE VortexCentroid_;

    // Edge direction vector
    
    double Vec_[3];
    
    // Distance tolerances for velocity evaluations
   
    double Tolerance_1_;
    double Tolerance_2_;
    double Tolerance_4_;
    
    // Local grid spacing estimate

    double LocalSpacing_;
    
    // Mach number
    
    static double Mach_;

    // Flag dermining which loops are down wind of this vortex edge
    
    int VortexLoop1IsDownWind_;
    int VortexLoop2IsDownWind_;
    
    double VortexLoop1DownWindWeight_;
    double VortexLoop2DownWindWeight_;

    // Circulation strength
    
    double Gamma_;
    
    // Edge matrix coefficients
    
    double EdgeCoef_[2];

    // Setup routines
    
    void Setup_(VSP_NODE &Node1, VSP_NODE &Node2);

    // Induced velocities
    
    void NewBoundVortex(double xyz_p[3], double q[3]);
    double Fint(double &a, double &b, double &c, double &d, double &s);
    double Gint(double &a, double &b, double &c, double &d, double &s);
    void FindLineConicIntersection(double &Xp, double &Yp, double &Zp,
                                   double &X1, double &Y1, double &Z1,
                                   double &Xd, double &Yd, double &Zd,
                                   double &t1, double &t2);    

public:

    // Constructor, Destructor, Copy

    VSP_EDGE(void);
   ~VSP_EDGE(void);
    VSP_EDGE(const VSP_EDGE &VSPEdge);
    VSP_EDGE& operator=(const VSP_EDGE &VSPEdge);

    // Accesss to node data

    int &Node1(void) { return Node1_; };
    int &Node2(void) { return Node2_; };

    // Access to tri data
    
    int &Tri1(void) { return Loop1_; };
    int &Tri2(void) { return Loop2_; };
    
    int &TriL(void) { return LoopL_; };
    int &TriR(void) { return LoopR_; };
 
    // Access to loop data
    
    int &Loop1(void) { return Loop1_; };
    int &Loop2(void) { return Loop2_; };
 
    int &LoopL(void) { return LoopL_; };
    int &LoopR(void) { return LoopR_; };
    
    // Access to global vortex edge, and vortex loop data
    
    int &VortexEdge(void) { return VortexEdge_; };
    
    int &VortexLoop1(void) { return VortexLoop1_; };
    int &VortexLoop2(void) { return VortexLoop2_; };

    // Edge Sigma, S, and length
    
    double &Sigma(void) { return Sigma_; };
    double &S(void) { return S_; };
    double &Length(void) { return Length_; };
    
    int &EdgeType(void) { return EdgeType_; };
    int &IsTrailingEdge(void) { return IsTrailingEdge_; };
    int &IsLeadingEdge(void) { return IsLeadingEdge_; };
    int &IsBoundaryEdge(void) { return IsBoundaryEdge_; };
    
    int &CourseGridEdge(void) { return CourseGridEdge_; };
    int &FineGridEdge(void) { return FineGridEdge_; };
    int &EdgeWasUsedForLoop(void) { return EdgeWasUsedForLoop_; };

    void SetupChildren(VSP_EDGE &Child1, VSP_EDGE &Child2) { Child1_ = &Child1 ; Child2_ = &Child2; };

    int ThereAreChildren(void) { if ( Child1_ != NULL ) { return 1; } else { return 0; }; };
    VSP_EDGE &Child1(void) { return *Child1_; };
    VSP_EDGE &Child2(void) { return *Child2_; };

    // Accesss to Vortex edge data

    int &DegenWing(void) { return DegenWing_; };
    int &DegenBody(void) { return DegenBody_; };
    int &Cart3DSurface(void) { return Cart3DSurface_; };
    int &Node(void) { return Node_; };
    
    // Solver routines

    void Setup(VSP_NODE &Node1, VSP_NODE &Node2) { Setup_(Node1,Node2); };
                               
    void InducedVelocity(double xyz_p[3], double q[3]);
    
    void CalculateForces(VSP_LOOP &VortexLoop);
   
    void CalculateTrefftzForces(double FreeStream[3]);
    
    double GeneralizedPrincipalPartOfDownWash(void);
    
    // Access to Mach number
    
    double &Mach(void) { return Mach_; };

    // Down wind vortex loops
    
    int &VortexLoop1IsDownWind(void) { return VortexLoop1IsDownWind_; };
    int &VortexLoop2IsDownWind(void) { return VortexLoop2IsDownWind_; };
    
    double &VortexLoop1DownWindWeight(void) { return VortexLoop1DownWindWeight_; };
    double &VortexLoop2DownWindWeight(void) { return VortexLoop2DownWindWeight_; };
    
    // Access to the circulation strength
    
    double &Gamma(void) { return Gamma_; };
    
    // Access to the edge matrix coefficients

    double &EdgeCoef(int i) { return EdgeCoef_[i]; };
    
    // Access to the forces
    
    double &Fx(void) { return Forces_[0]; };
    double &Fy(void) { return Forces_[1]; };
    double &Fz(void) { return Forces_[2]; };
 
    double &Trefftz_Fx(void) { return Trefftz_Forces_[0]; };
    double &Trefftz_Fy(void) { return Trefftz_Forces_[1]; };
    double &Trefftz_Fz(void) { return Trefftz_Forces_[2]; };
    
    double &Unsteady_Fx(void) { return Unsteady_Forces_[0]; };
    double &Unsteady_Fy(void) { return Unsteady_Forces_[1]; };
    double &Unsteady_Fz(void) { return Unsteady_Forces_[2]; };    
    
    // Access to local spacing
    
    double &LocalSpacing(void) { return LocalSpacing_; };
    
    // Acess to the ends of the edge
    
    double X1(void) { return X1_; };
    double Y1(void) { return Y1_; };
    double Z1(void) { return Z1_; };
    
    double X2(void) { return X2_; };
    double Y2(void) { return Y2_; };
    double Z2(void) { return Z2_; };
    
    // Access to the center of the edge
    
    double Xc(void) { return XYZc_[0]; };
    double Yc(void) { return XYZc_[1]; };
    double Zc(void) { return XYZc_[2]; };
    
    double *xyz_c(void) { return XYZc_; };
    
    VSP_NODE &VortexCentroid(void) { return VortexCentroid_; };

    // Vortex unit vector
    
    double *Vec(void) { return Vec_; };    

};

#endif
