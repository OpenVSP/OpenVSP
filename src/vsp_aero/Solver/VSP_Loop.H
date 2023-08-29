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
    int IsTrailingEdgeTri_;
    int VortexLoop_;
    int IsSonic_;
    
    int MGVortexLoop_;
    
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
    int VortexSheetID_;        

    // Agglomeration data
    
    int CoarseGridLoop_;
    
    int NumberOfFineGridLoops_;
    int *FineGridLoopList_;
    
    int Level_;
    
    BBOX BoundBox_;
    
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

    /** Size the node list .. ie number of nodes for this loop **/

    void SizeNodeList(int NumberOfNodes);
    
    /** Number of nodes for this loop **/
    
    int NumberOfNodes(void) { return NumberOfNodes_; };
    
    /** Node 1 of loop **/
    
    int &Node1(void) { return NodeList_[0]; };
    
    /** Node 2 of the loop **/
    
    int &Node2(void) { return NodeList_[1]; };
    
    /** Node 3 of the loop **/
    
    int &Node3(void) { return NodeList_[2]; };

    /** Node 4 of the loop **/

    int &Node4(void) { return NodeList_[3]; };

    /** Node i of the loop ... i starts at 1 **/

    int &Node(int i) { return NodeList_[i-1]; };

    /** U parametric value for node 1 **/
    
    VSPAERO_DOUBLE &U_Node1(void) { return UVNodeList_[0]; };
    
    /** U parametric value for node 2 **/
    
    VSPAERO_DOUBLE &U_Node2(void) { return UVNodeList_[2]; };

    /** U parametric value for node 3 **/

    VSPAERO_DOUBLE &U_Node3(void) { return UVNodeList_[4]; };

    /** U parametric value for node i, i starts with 1 **/

    VSPAERO_DOUBLE &U_Node(int i) { return UVNodeList_[2*i-2]; };
    
    /** V parametric value for node 1 **/

    VSPAERO_DOUBLE &V_Node1(void) { return UVNodeList_[1]; };

    /** V parametric value for node 2 **/

    VSPAERO_DOUBLE &V_Node2(void) { return UVNodeList_[3]; };

    /** V parametric value for node 3 **/

    VSPAERO_DOUBLE &V_Node3(void) { return UVNodeList_[5]; };    

    /** v parametric value for node i, i starts with 1 **/

    VSPAERO_DOUBLE &V_Node(int i) { return UVNodeList_[2*i-1]; };

    /** Size the edge list for this loop **/
    
    void SizeEdgeList(int NumberOfEdges);
    
    /** Number of edges for this loop **/
    
    int NumberOfEdges(void) { return NumberOfEdges_;  };
    
    /** Edge 1 for loop **/
    
    int &Edge1(void) { return EdgeList_[0]; };

    /** Edge 2 for loop **/
    
    int &Edge2(void) { return EdgeList_[1]; };

    /** Edge 3 for loop **/

    int &Edge3(void) { return EdgeList_[2]; };

    /** Edge 4 for loop **/

    int &Edge4(void) { return EdgeList_[3]; };

    /** Edge i for loop, i starts with 1 **/

    int &Edge(int i) { return EdgeList_[i-1]; };
    
    /** Edge 1 direction for loop ... +/- relative to global edge direction **/
    
    int &Edge1Direction(void) { return EdgeDirection_[0]; };
    
    /** Edge 2 direction for loop ... +/- relative to global edge direction **/
    
    int &Edge2Direction(void) { return EdgeDirection_[1]; };

    /** Edge 3 direction for loop ... +/- relative to global edge direction **/

    int &Edge3Direction(void) { return EdgeDirection_[2]; };

    /** Edge 4 direction for loop ... +/- relative to global edge direction **/

    int &Edge4Direction(void) { return EdgeDirection_[3]; };

    /** Edge i direction for loop ... +/- relative to global edge direction ... i starts from 1 **/

    int &EdgeDirection(int i) { return EdgeDirection_[i-1]; };    
    
    /** Flag if edge 1 is upwind of loop **/
    
    int &Edge1IsUpWind(void) { return EdgeIsUpwind_[0]; };
    
    /** Flag if edge 2 is upwind of loop **/
    
    int &Edge2IsUpWind(void) { return EdgeIsUpwind_[1]; };

    /** Flag if edge 3 is upwind of loop **/

    int &Edge3IsUpWind(void) { return EdgeIsUpwind_[2]; };

    /** Flag if edge 4 is upwind of loop **/

    int &Edge4IsUpWind(void) { return EdgeIsUpwind_[3]; };

    /** Flag if edge i is upwind of loop, i starts with 1 **/

    int &EdgeIsUpWind(int i) { return EdgeIsUpwind_[i-1]; };
    
    /** Edge 1 upwind weight **/

    VSPAERO_DOUBLE &Edge1UpwindWeight(void) { return EdgeUpwindWeight_[0]; };
    
    /** Edge 1 upwind weight **/
    
    VSPAERO_DOUBLE &Edge2UpwindWeight(void) { return EdgeUpwindWeight_[1]; };

    /** Edge 2 upwind weight **/

    VSPAERO_DOUBLE &Edge3UpwindWeight(void) { return EdgeUpwindWeight_[2]; };

    /** Edge 3 upwind weight **/

    VSPAERO_DOUBLE &Edge4UpwindWeight(void) { return EdgeUpwindWeight_[3]; };

    /** Edge i upwind weight, i starts with 1 **/

    VSPAERO_DOUBLE &EdgeUpwindWeight(int i) { return EdgeUpwindWeight_[i-1]; };
    
    /** Surface ID for this loop **/
   
    int &SurfaceID(void) { return SurfaceID_; };
    
    /** Component ID for this loop **/
    
    int &ComponentID(void) { return ComponentID_; };  
    
    /** Geom ID for this loop **/

    /** Tri/Loop is on trailing edge **/
    
    int &IsTrailingEdgeTri(void) { return IsTrailingEdgeTri_; };

    /** Vortex loop value for the mesh level this loop sits on **/
    
    int &VortexLoop(void) { return VortexLoop_; };
    
    /** Vortex loop value for the over all multi-grid mesh system **/
    
    int &MGVortexLoop(void) { return MGVortexLoop_; };
    
    /** Loop is sonic, ie upwind edges are swept in front of Mach lines **/
    
    int &IsSonic(void) { return IsSonic_; };
    
    /** Span stations for this loop **/
    
    int &SpanStation(void) { return SpanStation_; };
    
    /** Vortex Sheet this loop belongs to **/
    
    int &VortexSheet(void) { return VortexSheetID_; }; 
    
    /** Area of loop **/
    
    VSPAERO_DOUBLE &Area(void) { return Area_; };
    
    /** Characteristic length of loop **/
    
    VSPAERO_DOUBLE &Length(void) { return Length_; };
    
    /** Reference length for loop ... this is typically the shortest edge length on the finest mesh **/
    
    VSPAERO_DOUBLE &RefLength(void) { return RefLength_; };
    
    /** Centroid off set for loop ... basically distance between centroid and center of bounding box **/
    
    VSPAERO_DOUBLE &CentroidOffSet(void) { return CentroidOffSet_; };
    
    /** X component of loop normal **/

    VSPAERO_DOUBLE &Nx(void) { return Normal_[0]; };
    
    /** Y component of loop normal **/
    
    VSPAERO_DOUBLE &Ny(void) { return Normal_[1]; };

    /** Z component of loop normal **/

    VSPAERO_DOUBLE &Nz(void) { return Normal_[2]; };
    
    /** Loop normal vector **/
    
    VSPAERO_DOUBLE *Normal(void) { return Normal_; };
    
    /** Camber **/

    VSPAERO_DOUBLE &Camber(void) { return Camber_; };
    
    /** X component of centroid **/
    
    VSPAERO_DOUBLE &Xc(void) { return XYZc_[0]; };
    
    /** Y component of centroid **/
    
    VSPAERO_DOUBLE &Yc(void) { return XYZc_[1]; };

    /** Z component of centroid **/

    VSPAERO_DOUBLE &Zc(void) { return XYZc_[2]; };
    
    /** Centroid vector **/
    
    VSPAERO_DOUBLE *xyz_c(void) { return XYZc_; };
    
    /** Parametric U value at centroid **/
    
    VSPAERO_DOUBLE &Uc(void) { return UVc_[0]; };
    
    /** Parametric V value at centroid **/
    
    VSPAERO_DOUBLE &Vc(void) { return UVc_[1]; };

    /** Parameter space centroid vector **/

    VSPAERO_DOUBLE *UVc(void) { return UVc_; };    
    
    /** Vortex strength **/
    
    VSPAERO_DOUBLE &Gamma(void) { return Gamma_; };
    
    /** Delta Cp ... also doubles as Cp for panel solves **/
    
    VSPAERO_DOUBLE &dCp(void) { return dCp_; };
    
    /** Unsteady Delta Cp ... also doubles as unsteady Cp for panel solves **/
    
    VSPAERO_DOUBLE &dCp_Unsteady(void) { return dCp_Unsteady_; };
  
    /** X component of inviscid, KJ forces on this loop **/
    
    VSPAERO_DOUBLE &Fx(void) { return Force_[0]; };
    
    /** Y component of inviscid, KJ forces on this loop **/
    
    VSPAERO_DOUBLE &Fy(void) { return Force_[1]; };

    /** Z component of inviscid, KJ forces on this loop **/

    VSPAERO_DOUBLE &Fz(void) { return Force_[2]; };

    /** KJ force vector on this loop **/

    VSPAERO_DOUBLE *Force(void) { return Force_; };
    
    /** Normal force on this loop **/
    
    VSPAERO_DOUBLE &NormalForce(void) { return NormalForce_; };
    
    /** Karman-Tsien factor for this loop **/
    
    VSPAERO_DOUBLE &KTFact(void) { return KTFact_; };
    
    /** Surface type **/
    
    int &SurfaceType(void) { return SurfaceType_; };
   
    /** Coarse grid loop this loop was agglomerated into **/
    
    int &CoarseGridLoop(void) { return CoarseGridLoop_; };
    
    /** Size the fine grid loop list **/
    
    void SizeFineGridLoopList(int NumberOfLoops);    
    
    /** Number of fine grid loops that make up this loop **/
    
    int  NumberOfFineGridLoops(void) { return NumberOfFineGridLoops_; };
    
    /** First fine grid loop **/
    
    int &FineGridLoop(void) { return FineGridLoopList_[0]; };
    
    /** Fine grid loop list **/
    
    int &FineGridLoop(int i) { return FineGridLoopList_[i-1]; };
    
    /** Agglomeration level this loop sits on **/
    
    int &Level(void) { return Level_; };
    
    /** Bounding box **/
    
    BBOX &BoundBox(void) { return BoundBox_; };
 
    /** Local free stream components **/
    
    VSPAERO_DOUBLE &LocalFreeStreamVelocity(int i) { return LocalFreeStreamVelocity_[i]; };
    
    /** Local free stream vector **/
    
    VSPAERO_DOUBLE *LocalFreeStreamVelocity(void) { return LocalFreeStreamVelocity_; };

    /** U component of total velocity... includes free stream, rotors, down wash, jet... everything there is **/
    
    VSPAERO_DOUBLE &U(void) { return Velocity_[0]; };
    
    /** V component of total velocity... includes free stream, rotors, down wash, jet... everything there is **/
    
    VSPAERO_DOUBLE &V(void) { return Velocity_[1]; };

    /** W component of total velocity... includes free stream, rotors, down wash, jet... everything there is **/

    VSPAERO_DOUBLE &W(void) { return Velocity_[2]; };

    /** Total velocity vector... includes free stream, rotors, down wash, jet... everything there is **/
    
    VSPAERO_DOUBLE *Velocity(void) { return Velocity_; };
    
    /** U component just the wake down wash induced velocity **/
    
    VSPAERO_DOUBLE &DownWash_U(void) { return DownWash_Velocity_[0]; };
    
    /** V component just the wake down wash induced velocity **/
    
    VSPAERO_DOUBLE &DownWash_V(void) { return DownWash_Velocity_[1]; };

    /** W component just the wake down wash induced velocity **/

    VSPAERO_DOUBLE &DownWash_W(void) { return DownWash_Velocity_[2]; };
    
    /** Velocity vector of just the wake down wash induced velocity **/
        
    VSPAERO_DOUBLE *DownWash_Velocity(void) { return DownWash_Velocity_; };

    /** Principlar part of the down wash integral **/
    
    VSPAERO_DOUBLE &Ws(void) { return Ws_; };
        
    /** Update the geometry given a translation vector and a quaternion rotation **/
           
    void UpdateGeometryLocation(VSPAERO_DOUBLE *TVec, VSPAERO_DOUBLE *OVec, QUAT &Quat, QUAT &InvQuat);

};

#include "END_NAME_SPACE.H"

#endif
