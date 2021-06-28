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
    
    int CoarseGridEdge_;
    int FineGridEdge_;
    int Level_;
    
    VSP_EDGE *Child1_;
    VSP_EDGE *Child2_;
    
    VSPAERO_DOUBLE ReferenceLength_;
    
    int EdgeWasUsedForLoop_;
    
    int WakeNode_;
    
    VSPAERO_DOUBLE Sigma_;
    VSPAERO_DOUBLE S_;
    VSPAERO_DOUBLE T_;
    VSPAERO_DOUBLE Length_;

    VSPAERO_DOUBLE Forces_[3];
    VSPAERO_DOUBLE ForcesNM1_[3];
    VSPAERO_DOUBLE Unsteady_Forces_[3];
    
    VSPAERO_DOUBLE LocalForces_[3];
    VSPAERO_DOUBLE LocalForcesNM1_[3];    
    VSPAERO_DOUBLE Unsteady_LocalForces[3];
    
    VSPAERO_DOUBLE Trefftz_Forces_[3];
    
    VSPAERO_DOUBLE InducedForces_[3];
    
    VSPAERO_DOUBLE MinCoreWidth_;
    VSPAERO_DOUBLE CoreWidth_;
    VSPAERO_DOUBLE FreeStreamDirection_[3];
     
    // Vortex solution data
    
    int Verbose_;
 
    // Wing and edge, or trailing node this vortex belongs to
    
    int DegenWing_;
    int DegenBody_;
    int Cart3DSurface_;
    int ComponentID_;

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
   
    VSPAERO_DOUBLE Tolerance_1_;
    VSPAERO_DOUBLE Tolerance_2_;
    VSPAERO_DOUBLE Tolerance_4_;
    
    // Local grid spacing estimate

    VSPAERO_DOUBLE LocalSpacing_;
    
    // Mach number and Beta
    
    static double Mach_;
    
    static double Kappa_;

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
    VSPAERO_DOUBLE Fint(VSPAERO_DOUBLE &a, VSPAERO_DOUBLE &b, VSPAERO_DOUBLE &c, VSPAERO_DOUBLE &d, VSPAERO_DOUBLE &s);
    VSPAERO_DOUBLE Gint(VSPAERO_DOUBLE &a, VSPAERO_DOUBLE &b, VSPAERO_DOUBLE &c, VSPAERO_DOUBLE &d, VSPAERO_DOUBLE &s);
    
    void FindLineConicIntersection(VSPAERO_DOUBLE &Xp, VSPAERO_DOUBLE &Yp, VSPAERO_DOUBLE &Zp,
                                   VSPAERO_DOUBLE &X1, VSPAERO_DOUBLE &Y1, VSPAERO_DOUBLE &Z1,
                                   VSPAERO_DOUBLE &Xd, VSPAERO_DOUBLE &Yd, VSPAERO_DOUBLE &Zd,
                                   VSPAERO_DOUBLE &t1, VSPAERO_DOUBLE &t2);    
                                                    
    // Airfoil data

    VSPAERO_DOUBLE ThicknessToChord_;
    VSPAERO_DOUBLE LocationOfMaxThickness_;
    VSPAERO_DOUBLE RadiusToChord_;            
    
    // Velocities

    VSPAERO_DOUBLE Velocity_[3];
    VSPAERO_DOUBLE DownWash_Velocity_[3];                                 

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
    
    int &WakeNode(void) { return WakeNode_; };
    
    VSPAERO_DOUBLE &Sigma(void) { return Sigma_; };
    VSPAERO_DOUBLE &S(void) { return S_; };
    VSPAERO_DOUBLE &T(void) { return T_; };    
    VSPAERO_DOUBLE &Length(void) { return Length_; };
    
    int &EdgeType(void) { return EdgeType_; };
    int &IsTrailingEdge(void) { return IsTrailingEdge_; };
    int &IsLeadingEdge(void) { return IsLeadingEdge_; };
    int &IsBoundaryEdge(void) { return IsBoundaryEdge_; };
    
    int &CoarseGridEdge(void) { return CoarseGridEdge_; };
    int &FineGridEdge(void) { return FineGridEdge_; };
    int &Level(void) { return Level_; };
    int &EdgeWasUsedForLoop(void) { return EdgeWasUsedForLoop_; };

    void SetupChildren(VSP_EDGE &Child1, VSP_EDGE &Child2) { Child1_ = &Child1 ; Child2_ = &Child2; };

    int ThereAreChildren(void) { if ( Child1_ != NULL ) { return 1; } else { return 0; }; };
    
    VSP_EDGE &Child1(void) { return *Child1_; };
    VSP_EDGE &Child2(void) { return *Child2_; };
    
    VSPAERO_DOUBLE &ReferenceLength(void) { return ReferenceLength_; };

    // Accesss to Vortex edge data

    int &DegenWing(void) { return DegenWing_; };
    int &DegenBody(void) { return DegenBody_; };
    int &Cart3DSurface(void) { return Cart3DSurface_; };
    int &ComponentID(void) { return ComponentID_; };
    
    // Solver routines

    void Setup(VSP_NODE &Node1, VSP_NODE &Node2) { Setup_(Node1,Node2); };
                               
    void Update(VSP_NODE &Node1, VSP_NODE &Node2) { Setup_(Node1,Node2); };

    void InducedVelocity(VSPAERO_DOUBLE xyz_p[3], VSPAERO_DOUBLE q[3]);
    
    void InducedVelocity(VSPAERO_DOUBLE xyz_p[3], VSPAERO_DOUBLE q[3], VSPAERO_DOUBLE CoreWidth);
    
    void CalculateForces(void);
   
    void CalculateTrefftzForces(VSPAERO_DOUBLE WakeInducedVelocity[3]);

    VSPAERO_DOUBLE GeneralizedPrincipalPartOfDownWash(void);
    
    VSPAERO_DOUBLE &MinCoreWidth(void) { return MinCoreWidth_; };
    
    // Access to Mach number
    
    void SetMach(VSPAERO_DOUBLE Mach);

    double Mach(void) { return Mach_; };

    VSPAERO_DOUBLE &KTFact(void) { return KTFact_; };

    // Down wind vortex loops
    
    int &VortexLoop1IsDownWind(void) { return VortexLoop1IsDownWind_; };
    int &VortexLoop2IsDownWind(void) { return VortexLoop2IsDownWind_; };
    
    VSPAERO_DOUBLE &VortexLoop1DownWindWeight(void) { return VortexLoop1DownWindWeight_; };
    VSPAERO_DOUBLE &VortexLoop2DownWindWeight(void) { return VortexLoop2DownWindWeight_; };
    
    // Access to the circulation strength
    
    VSPAERO_DOUBLE &Gamma(void) { return Gamma_; };
    
    // Access to the edge matrix coefficients

    VSPAERO_DOUBLE &EdgeCoef(int i) { return EdgeCoef_[i]; };
    
    // Access to the total steady and unstready forces
    
    VSPAERO_DOUBLE &Fx(void) { return Forces_[0]; };
    VSPAERO_DOUBLE &Fy(void) { return Forces_[1]; };
    VSPAERO_DOUBLE &Fz(void) { return Forces_[2]; };

    VSPAERO_DOUBLE &FxNM1(void) { return ForcesNM1_[0]; };
    VSPAERO_DOUBLE &FyNM1(void) { return ForcesNM1_[1]; };
    VSPAERO_DOUBLE &FzNM1(void) { return ForcesNM1_[2]; };
        
    VSPAERO_DOUBLE &Unsteady_Fx(void) { return Unsteady_Forces_[0]; };
    VSPAERO_DOUBLE &Unsteady_Fy(void) { return Unsteady_Forces_[1]; };
    VSPAERO_DOUBLE &Unsteady_Fz(void) { return Unsteady_Forces_[2]; };   

    // Access to the local steady and unsteady forces
    
    VSPAERO_DOUBLE &Local_Fx(void) { return LocalForces_[0]; };
    VSPAERO_DOUBLE &Local_Fy(void) { return LocalForces_[1]; };
    VSPAERO_DOUBLE &Local_Fz(void) { return LocalForces_[2]; };

    VSPAERO_DOUBLE &Local_FxNM1(void) { return LocalForcesNM1_[0]; };
    VSPAERO_DOUBLE &Local_FyNM1(void) { return LocalForcesNM1_[1]; };
    VSPAERO_DOUBLE &Local_FzNM1(void) { return LocalForcesNM1_[2]; };
        
    VSPAERO_DOUBLE &Unsteady_Local_Fx(void) { return Unsteady_LocalForces[0]; };
    VSPAERO_DOUBLE &Unsteady_Local_Fy(void) { return Unsteady_LocalForces[1]; };
    VSPAERO_DOUBLE &Unsteady_Local_Fz(void) { return Unsteady_LocalForces[2]; };    
      
    // Access to the Trefftz plane (kinda) forces
    
    VSPAERO_DOUBLE &Trefftz_Fx(void) { return Trefftz_Forces_[0]; };
    VSPAERO_DOUBLE &Trefftz_Fy(void) { return Trefftz_Forces_[1]; };
    VSPAERO_DOUBLE &Trefftz_Fz(void) { return Trefftz_Forces_[2]; };
          
    // Access to local spacing
    
    VSPAERO_DOUBLE &LocalSpacing(void) { return LocalSpacing_; };
    
    // Acess to the ends of the edge
    
    VSPAERO_DOUBLE X1(void) { return X1_; };
    VSPAERO_DOUBLE Y1(void) { return Y1_; };
    VSPAERO_DOUBLE Z1(void) { return Z1_; };
    
    VSPAERO_DOUBLE X2(void) { return X2_; };
    VSPAERO_DOUBLE Y2(void) { return Y2_; };
    VSPAERO_DOUBLE Z2(void) { return Z2_; };
    
    // Access to the center of the edge
    
    VSPAERO_DOUBLE Xc(void) { return XYZc_[0]; };
    VSPAERO_DOUBLE Yc(void) { return XYZc_[1]; };
    VSPAERO_DOUBLE Zc(void) { return XYZc_[2]; };
    
    VSPAERO_DOUBLE *xyz_c(void) { return XYZc_; };
    
    VSP_NODE &VortexCentroid(void) { return VortexCentroid_; };

    // Vortex unit vector
    
    VSPAERO_DOUBLE *Vec(void) { return Vec_; };    
    
    // Normal
    
    VSPAERO_DOUBLE *Normal(void) { return Normal_; };
    
    // Airfoil data
    
    VSPAERO_DOUBLE &ThicknessToChord(void) { return ThicknessToChord_; };
    VSPAERO_DOUBLE &LocationOfMaxThickness(void ) { return LocationOfMaxThickness_; };
    VSPAERO_DOUBLE &RadiusToChord(void) { return RadiusToChord_; };        

    // Local total velocity, includes downwash
    
    VSPAERO_DOUBLE &U(void) { return Velocity_[0]; };
    VSPAERO_DOUBLE &V(void) { return Velocity_[1]; };
    VSPAERO_DOUBLE &W(void) { return Velocity_[2]; };
    
    VSPAERO_DOUBLE *Velocity(void) { return Velocity_; };
    
    // Just the down wash velocity
    
    VSPAERO_DOUBLE &DownWash_U(void) { return DownWash_Velocity_[0]; };
    VSPAERO_DOUBLE &DownWash_V(void) { return DownWash_Velocity_[1]; };
    VSPAERO_DOUBLE &DownWash_W(void) { return DownWash_Velocity_[2]; };
   
    // Update geometry
    
    void UpdateGeometryLocation(VSPAERO_DOUBLE *TVec, VSPAERO_DOUBLE *OVec, QUAT &Quat, QUAT &InvQuat);
    
};

#endif
