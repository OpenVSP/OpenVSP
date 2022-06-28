//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//
//////////////////////////////////////////////////////////////////////

#ifndef VSP_LOOP_H
#define VSP_LOOP_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <assert.h>
#include "utils.H"
#include "quat.H"

#include "START_NAME_SPACE.H"

// Definition of the VSP_LOOP class

class VSP_LOOP {

private:

    // Loop based data

    int NumberOfNodes_;
    int *NodeList_;

    VSPAERO_DOUBLE *UVNodeList_;
    
    int NumberOfEdges_;
    int *EdgeList_;
    int *EdgeDirection_;

    // Upwind edge weights
   
    int *EdgeIsUpwind_;    
    VSPAERO_DOUBLE *EdgeUpwindWeight_;

    int SurfaceID_;
    int ComponentID_;
    int GeomID_;
    int IsTrailingEdgeTri_;
    int IsLeadingEdgeTri_;
    int VortexLoop_;
    int IsSonic_;
    
    int SpanStation_;

    VSPAERO_DOUBLE Area_;
    
    VSPAERO_DOUBLE Length_;
    
    VSPAERO_DOUBLE RefLength_;
    
    VSPAERO_DOUBLE CentroidOffSet_;

    // Triangle Normal
    
    VSPAERO_DOUBLE Normal_[3];

    // Centroid
    
    VSPAERO_DOUBLE XYZc_[3];
    
    // UV Centroid
    
    VSPAERO_DOUBLE UVc_[2];
    
    // Camber at centroid
    
    VSPAERO_DOUBLE Camber_;

    // Solution data
    
    VSPAERO_DOUBLE Gamma_;
    VSPAERO_DOUBLE dCp_;
    VSPAERO_DOUBLE dCp_Unsteady_;
    VSPAERO_DOUBLE Force_[3];
    VSPAERO_DOUBLE NormalForce_;
    VSPAERO_DOUBLE KTFact_;
    
    int SurfaceType_;
    int DegenBodyID_;
    int DegenWingID_;        
    
    // Airfoil data

    VSPAERO_DOUBLE ThicknessToChord_;
    VSPAERO_DOUBLE LocationOfMaxThickness_;
    VSPAERO_DOUBLE RadiusToChord_;      
    
    // Agglomeration data
    
    int CoarseGridLoop_;
    
    int NumberOfFineGridLoops_;
    int *FineGridLoopList_;
    
    BBOX BoundBox_;
    
    int LoopUsed_;
    
    // Velocities

    VSPAERO_DOUBLE LocalFreeStreamVelocity_[5];
    VSPAERO_DOUBLE Velocity_[3];
    VSPAERO_DOUBLE DownWash_Velocity_[3];        
    
    // Principal part
    
    VSPAERO_DOUBLE Ws_;

public:

    // Constructor, Destructor, Copy

    VSP_LOOP(void);
   ~VSP_LOOP(void);
    VSP_LOOP(const VSP_LOOP &VSPTri);
    VSP_LOOP& operator=(const VSP_LOOP &VSPTri);

    // Accesss to tri data

    void SizeNodeList(int NumberOfNodes);
    int NumberOfNodes(void) { return NumberOfNodes_; };
    int &Node1(void) { return NodeList_[0]; };
    int &Node2(void) { return NodeList_[1]; };
    int &Node3(void) { return NodeList_[2]; };
    int &Node4(void) { return NodeList_[3]; };
    int &Node(int i) { return NodeList_[i-1]; };

    // UV Parameters
    
    VSPAERO_DOUBLE &U_Node1(void) { return UVNodeList_[0]; };
    VSPAERO_DOUBLE &U_Node2(void) { return UVNodeList_[2]; };
    VSPAERO_DOUBLE &U_Node3(void) { return UVNodeList_[4]; };
    VSPAERO_DOUBLE &U_Node(int i) { return UVNodeList_[2*i-2]; };

    VSPAERO_DOUBLE &V_Node1(void) { return UVNodeList_[1]; };
    VSPAERO_DOUBLE &V_Node2(void) { return UVNodeList_[3]; };
    VSPAERO_DOUBLE &V_Node3(void) { return UVNodeList_[5]; };    
    VSPAERO_DOUBLE &V_Node(int i) { return UVNodeList_[2*i-1]; };

    // Access to edge data
    
    void SizeEdgeList(int NumberOfEdges);
    int NumberOfEdges(void) { return NumberOfEdges_;  };
    int &Edge1(void) { return EdgeList_[0]; };
    int &Edge2(void) { return EdgeList_[1]; };
    int &Edge3(void) { return EdgeList_[2]; };
    int &Edge4(void) { return EdgeList_[3]; };
    int &Edge(int i) { return EdgeList_[i-1]; };
    
    int &Edge1Direction(void) { return EdgeDirection_[0]; };
    int &Edge2Direction(void) { return EdgeDirection_[1]; };
    int &Edge3Direction(void) { return EdgeDirection_[2]; };
    int &Edge4Direction(void) { return EdgeDirection_[3]; };
    int &EdgeDirection(int i) { return EdgeDirection_[i-1]; };    
    
    int &Edge1IsUpWind(void) { return EdgeIsUpwind_[0]; };
    int &Edge2IsUpWind(void) { return EdgeIsUpwind_[1]; };
    int &Edge3IsUpWind(void) { return EdgeIsUpwind_[2]; };
    int &Edge4IsUpWind(void) { return EdgeIsUpwind_[3]; };
    int &EdgeIsUpWind(int i) { return EdgeIsUpwind_[i-1]; };

    VSPAERO_DOUBLE &Edge1UpwindWeight(void) { return EdgeUpwindWeight_[0]; };
    VSPAERO_DOUBLE &Edge2UpwindWeight(void) { return EdgeUpwindWeight_[1]; };
    VSPAERO_DOUBLE &Edge3UpwindWeight(void) { return EdgeUpwindWeight_[2]; };
    VSPAERO_DOUBLE &Edge4UpwindWeight(void) { return EdgeUpwindWeight_[3]; };
    VSPAERO_DOUBLE &EdgeUpwindWeight(int i) { return EdgeUpwindWeight_[i-1]; };
   
    int &SurfaceID(void) { return SurfaceID_; };
    int &ComponentID(void) { return ComponentID_; };  
    int &GeomID(void) { return GeomID_; };  
    int &IsTrailingEdgeTri(void) { return IsTrailingEdgeTri_; };
    int &IsLeadingEdgeTri(void) { return IsLeadingEdgeTri_; };
    int &VortexLoop(void) { return VortexLoop_; };
    int &IsSonic(void) { return IsSonic_; };
    
    int &SpanStation(void) { return SpanStation_; };
    int &VortexSheet(void) { return DegenWingID_; }; // We are double book keeping DegenWingID_
    
    VSPAERO_DOUBLE &Area(void) { return Area_; };
    
    VSPAERO_DOUBLE &Length(void) { return Length_; };
    
    VSPAERO_DOUBLE &RefLength(void) { return RefLength_; };
    
    VSPAERO_DOUBLE &CentroidOffSet(void) { return CentroidOffSet_; };

    VSPAERO_DOUBLE &Nx(void) { return Normal_[0]; };
    VSPAERO_DOUBLE &Ny(void) { return Normal_[1]; };
    VSPAERO_DOUBLE &Nz(void) { return Normal_[2]; };
    VSPAERO_DOUBLE *Normal(void) { return Normal_; };

    VSPAERO_DOUBLE &Camber(void) { return Camber_; };
    
    VSPAERO_DOUBLE &Xc(void) { return XYZc_[0]; };
    VSPAERO_DOUBLE &Yc(void) { return XYZc_[1]; };
    VSPAERO_DOUBLE &Zc(void) { return XYZc_[2]; };
    VSPAERO_DOUBLE *xyz_c(void) { return XYZc_; };
    
    VSPAERO_DOUBLE &Uc(void) { return UVc_[0]; };
    VSPAERO_DOUBLE &Vc(void) { return UVc_[1]; };
    VSPAERO_DOUBLE *UVc(void) { return UVc_; };    
    
    VSPAERO_DOUBLE &Gamma(void) { return Gamma_; };
    VSPAERO_DOUBLE &dCp(void) { return dCp_; };
    VSPAERO_DOUBLE &dCp_Unsteady(void) { return dCp_Unsteady_; };
  
    VSPAERO_DOUBLE &Fx(void) { return Force_[0]; };
    VSPAERO_DOUBLE &Fy(void) { return Force_[1]; };
    VSPAERO_DOUBLE &Fz(void) { return Force_[2]; };
    VSPAERO_DOUBLE *Force(void) { return Force_; };
    
    VSPAERO_DOUBLE &NormalForce(void) { return NormalForce_; };
    
    VSPAERO_DOUBLE &KTFact(void) { return KTFact_; };
    
    int &SurfaceType(void) { return SurfaceType_; };
    int &DegenBodyID(void) { return DegenBodyID_; };
    int &DegenWingID(void) { return DegenWingID_; };    
    
    // Airfoil data
    
    VSPAERO_DOUBLE &ThicknessToChord(void) { return ThicknessToChord_; };
    VSPAERO_DOUBLE &LocationOfMaxThickness(void ) { return LocationOfMaxThickness_; };
    VSPAERO_DOUBLE &RadiusToChord(void) { return RadiusToChord_; };    
        
    // Agglomeration data
    
    int &CoarseGridLoop(void) { return CoarseGridLoop_; };
    
    void SizeFineGridLoopList(int NumberOfLoops);    
    int  NumberOfFineGridLoops(void) { return NumberOfFineGridLoops_; };
    int &FineGridLoop(void) { return FineGridLoopList_[0]; };
    int &FineGridLoop(int i) { return FineGridLoopList_[i-1]; };
    
    BBOX &BoundBox(void) { return BoundBox_; };
    
    int &LoopUsed(void) { return LoopUsed_; };
   
    // Local free stream velocity
    
    VSPAERO_DOUBLE &LocalFreeStreamVelocity(int i) { return LocalFreeStreamVelocity_[i]; };
    VSPAERO_DOUBLE *LocalFreeStreamVelocity(void) { return LocalFreeStreamVelocity_; };

    // Local total velocity, includes downwash
    
    VSPAERO_DOUBLE &U(void) { return Velocity_[0]; };
    VSPAERO_DOUBLE &V(void) { return Velocity_[1]; };
    VSPAERO_DOUBLE &W(void) { return Velocity_[2]; };
    
    VSPAERO_DOUBLE *Velocity(void) { return Velocity_; };
    
    // Just the down wash velocity
    
    VSPAERO_DOUBLE &DownWash_U(void) { return DownWash_Velocity_[0]; };
    VSPAERO_DOUBLE &DownWash_V(void) { return DownWash_Velocity_[1]; };
    VSPAERO_DOUBLE &DownWash_W(void) { return DownWash_Velocity_[2]; };
    
    VSPAERO_DOUBLE *DownWash_Velocity(void) { return DownWash_Velocity_; };

    // Principal part
    
    VSPAERO_DOUBLE &Ws(void) { return Ws_; };
        
    // Update the geometry
           
    void UpdateGeometryLocation(VSPAERO_DOUBLE *TVec, VSPAERO_DOUBLE *OVec, QUAT &Quat, QUAT &InvQuat);

};

#include "END_NAME_SPACE.H"

#endif
