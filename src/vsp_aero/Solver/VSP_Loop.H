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

    // Number of nodes making up this loop

    int NumberOfNodes_;
    
    // Global node list for this loop
    
    int *NodeList_;

    // UV values for each loop node
    
    double *UVNodeList_;
    
    // Number of edges making up this loop
    
    int NumberOfEdges_;
    
    // Global edge list for this loop
    
    int *EdgeList_;
    
    // Edge direction flag, ie does local loop edge direction align
    // with the global edge diredtion (min node --> max node), or is
    // it reversed
    
    int *EdgeDirection_;

    // Flag to determine if edge is upwind of loop centroid
    
    int *EdgeIsUpwind_;    
    
    // Upwind edge weights
    
    double *EdgeUpwindWeight_;

    // Surface ID
    
    int SurfaceID_;

    // VSPAERO Component ID
    
    int ComponentID_;
    
    // OpenVSP ComponentID_;
    
    int OpenVSP_ComponentID_;
    
    // Loop is on trailing wing edge (kutta edge)
    
    int IsTrailingEdgeTri_;
    
    // Global vortex loop ID
    
    int VortexLoop_;
    
    // Overall vortex loop ID across all the multi-grid levels

    int MGVortexLoop_;

    // If this loop is on the wake this is the min time step it's valid for an unsteady analysis
    
    int MinValidTimeStep_;      
    
    // If this loop is on the wake this is the upwind loop
    
    int UpwindWakeLoop_;
    
    // SpanStation for this loop, based on UV mapping
    
    int SpanStation_;

    // Area for loop
    
    double Area_;
    
    // Length of loop, ~ sqrt(Area), or max edge length... which ever is bigger
    
    double Length_;
    
    // Smallest edge length for loop
    
    double RefLength_;
    
    // Circumference of loop.. [0] is previous time step, [1] is current value
    
    double Circumference_[2];
    
    // Distance betwen centroid of loop and center of loop bounding box
    
    double CentroidOffSet_;

    // Triangle Normal
    
    double Normal_[3];

    // Centroid
    
    double XYZc_[3];
    
    // UV Centroid
    
    double UVc_[2];

    // Vorticity
    
    double Gamma_;
    
    // Perturbation vorticity for GMRES solve
    
    double DeltaGamma_;

    // Stall factor
    
    double StallFactor_;

    // Delta Stall factor
    
    double DeltaStallFactor_;
    
    // Vortex stretching ratio
    
    double VortexStretchingRatio_;
    
    // Delta vortex stretching ratio
    
    double DeltaVortexStretchingRatio_;      
          
    // Adjoint solution, Psi
    
    double Psi_;
    
    // Delta Cp, or CP depending on context
    
    double dCp_;

    // Unsteady delta Cp, or CP depending on context

    double dCp_Unsteady_;
    
    // Force on this loop
    
    double Force_[3];
    
    // Karman-Tsien compressibility factor
    
    double KTFact_;
    
    // Vortex core width for wake loops
    
    double CoreWidth_;
    
    // Surface type... thick or thin
    
    int SurfaceType_;
    
    // Vortex sheet ID
    
    int VortexSheetID_;        

    // Coarse grid loop this fine grid loop was agglomerated into
    
    int CoarseGridLoop_;
    
    // Number of fine grid loops that make up this coarse grid loop
    
    int NumberOfFineGridLoops_;
    
    // Fine grid loop list
    
    int *FineGridLoopList_;
    
    // What grid level we are at
    
    int Level_;
    
    // Loop bounding box
    
    BBOX BoundBox_;
    
    // Free stream velocity ... could include props, engine flow fields...

    double LocalFreeStreamVelocity_[5];
    
    // Velocity at centroid of this loop
    
    double Velocity_[3];
    
    // Velocity due to perturbation in gammas
    
    double dVelocity_dGamma_[3];

    // Velocity due to perturbation in mesh xyz's
    
    double dVelocity_dMesh_[3];

    // Velocity due to perturbation in Mach
    
    double dVelocity_dMach_[3];
    
    // Velocity due to perturbation in Stall factor
    
    double dVelocity_dStall_[3];
    
    // Velocity due to perturbation in Vortex Stretching ratio
    
    double dVelocity_dRatio_[3];
    
    // Partials of invisicid, viscous, and induced forces wrt gamma for this loop

    double pForces_pGamma_[9];

    // Partials of invisicid and viscous moments wrt gamma for this loop

    double pMoments_pGamma_[6];
    
    // Partials of invisicid, viscous, and induced forces wrt gamma for this loop

    double pForces_pRatio_[9];

    // Partials of invisicid and viscous moments wrt gamma for this loop

    double pMoments_pRatio_[6];        

    // Principal part of supersonic downwash integral
    
    double Ws_;

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
    
    double &U_Node1(void) { return UVNodeList_[0]; };
    
    /** U parametric value for node 2 **/
    
    double &U_Node2(void) { return UVNodeList_[2]; };

    /** U parametric value for node 3 **/

    double &U_Node3(void) { return UVNodeList_[4]; };

    /** U parametric value for node i, i starts with 1 **/

    double &U_Node(int i) { return UVNodeList_[2*i-2]; };
    
    /** V parametric value for node 1 **/

    double &V_Node1(void) { return UVNodeList_[1]; };

    /** V parametric value for node 2 **/

    double &V_Node2(void) { return UVNodeList_[3]; };

    /** V parametric value for node 3 **/

    double &V_Node3(void) { return UVNodeList_[5]; };    

    /** v parametric value for node i, i starts with 1 **/

    double &V_Node(int i) { return UVNodeList_[2*i-1]; };

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

    double &Edge1UpwindWeight(void) { return EdgeUpwindWeight_[0]; };
    
    /** Edge 1 upwind weight **/
    
    double &Edge2UpwindWeight(void) { return EdgeUpwindWeight_[1]; };

    /** Edge 2 upwind weight **/

    double &Edge3UpwindWeight(void) { return EdgeUpwindWeight_[2]; };

    /** Edge 3 upwind weight **/

    double &Edge4UpwindWeight(void) { return EdgeUpwindWeight_[3]; };

    /** Edge i upwind weight, i starts with 1 **/

    double &EdgeUpwindWeight(int i) { return EdgeUpwindWeight_[i-1]; };
    
    /** Surface ID for this loop **/
   
    int &SurfaceID(void) { return SurfaceID_; };

    /** VSPAERO Component ID for this loop **/
    
    int &ComponentID(void) { return ComponentID_; };  

    /** OpenVSP ComponentID_ **/
    
    int &OpenVSP_ComponentID(void) { return OpenVSP_ComponentID_; };

    /** Tri/Loop is on trailing edge **/
    
    int &IsTrailingEdgeTri(void) { return IsTrailingEdgeTri_; };

    /** Vortex loop value for the mesh level this loop sits on **/
    
    int &VortexLoop(void) { return VortexLoop_; };
    
    /** Vortex loop value for the over all multi-grid mesh system **/
    
    int &MGVortexLoop(void) { return MGVortexLoop_; };

    /** If this loop is on the wake this is the min time step it's valid for an unsteady analysis **/
    
    int &MinValidTimeStep(void) { return MinValidTimeStep_; };
        
    // If this loop is on the wake this is the upwind loop
    
    int &UpwindWakeLoop(void) { return UpwindWakeLoop_; };
     
    /** Span stations for this loop **/
    
    int &SpanStation(void) { return SpanStation_; };
    
    /** Vortex Sheet this loop belongs to **/
    
    int &VortexSheet(void) { return VortexSheetID_; }; 
    
    /** Area of loop **/
    
    double &Area(void) { return Area_; };
    
    /** Characteristic length of loop **/
    
    double &Length(void) { return Length_; };
    
    /** Reference length for loop ... this is typically the shortest edge length on the finest mesh **/
    
    double &RefLength(void) { return RefLength_; };
    
    // Circumference of loop
    
    double &Circumference(void) { return Circumference_[1]; };  
       
    // Circumference of loop.. [0] is previous time step, [1] is current value
    
    double &Circumference(int i) { return Circumference_[i]; };  
    
    /** Centroid off set for loop ... basically distance between centroid and center of bounding box **/
    
    double &CentroidOffSet(void) { return CentroidOffSet_; };
    
    /** X component of loop normal **/

    double &Nx(void) { return Normal_[0]; };
    
    /** Y component of loop normal **/
    
    double &Ny(void) { return Normal_[1]; };

    /** Z component of loop normal **/

    double &Nz(void) { return Normal_[2]; };
    
    /** Loop normal vector **/
    
    double *Normal(void) { return Normal_; };

    /** X component of centroid **/
    
    double &Xc(void) { return XYZc_[0]; };
    
    /** Y component of centroid **/
    
    double &Yc(void) { return XYZc_[1]; };

    /** Z component of centroid **/

    double &Zc(void) { return XYZc_[2]; };
    
    /** Centroid vector **/
    
    double *xyz_c(void) { return XYZc_; };
    
    /** Parametric U value at centroid **/
    
    double &Uc(void) { return UVc_[0]; };
    
    /** Parametric V value at centroid **/
    
    double &Vc(void) { return UVc_[1]; };

    /** Parameter space centroid vector **/

    double *UVc(void) { return UVc_; };    
    
    /** Vortex strength **/
    
    double &Gamma(void) { return Gamma_; };

    /** Perturbation Vortex strength **/
    
    double &dGamma(void) { return DeltaGamma_; };

    /** Stall factor **/
    
    double &StallFactor(void) { return StallFactor_; };    
    
    /** Stall factor **/
    
    double &dStallFactor(void) { return DeltaStallFactor_; };    

    /** Vortex stretching ratio **/
    
    double &VortexStretchingRatio(void) { return VortexStretchingRatio_; };
    
    /** Delta vortex stretching ratio **/
    
    double &dVortexStretchingRatio(void) { return DeltaVortexStretchingRatio_; };   
    
    /** Adjoint solution, Psi **/
        
    double &Psi(void) { return Psi_; };
        
    /** Delta Cp ... also doubles as Cp for panel solves **/
    
    double &dCp(void) { return dCp_; };
    
    /** Unsteady Delta Cp ... also doubles as unsteady Cp for panel solves **/
    
    double &dCp_Unsteady(void) { return dCp_Unsteady_; };
  
    /** X component of inviscid, KJ forces on this loop **/
    
    double &Fx(void) { return Force_[0]; };
    
    /** Y component of inviscid, KJ forces on this loop **/
    
    double &Fy(void) { return Force_[1]; };

    /** Z component of inviscid, KJ forces on this loop **/

    double &Fz(void) { return Force_[2]; };

    /** KJ force vector on this loop **/

    double *Force(void) { return Force_; };
    
    /** Vortex core width **/
    
    double &CoreWidth(void) { return CoreWidth_; };

    /** Karman-Tsien factor for this loop **/
    
    double &KTFact(void) { return KTFact_; };
    
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
    
    double &LocalFreeStreamVelocity(int i) { return LocalFreeStreamVelocity_[i]; };
    
    /** Local free stream vector **/
    
    double *LocalFreeStreamVelocity(void) { return LocalFreeStreamVelocity_; };

    /** U component of total velocity... includes free stream, rotors, down wash, jet... everything there is **/
    
    double &U(void) { return Velocity_[0]; };
    
    /** V component of total velocity... includes free stream, rotors, down wash, jet... everything there is **/
    
    double &V(void) { return Velocity_[1]; };

    /** W component of total velocity... includes free stream, rotors, down wash, jet... everything there is **/

    double &W(void) { return Velocity_[2]; };

    /** Total velocity vector... includes free stream, rotors, down wash, jet... everything there is **/
    
    double *Velocity(void) { return Velocity_; };

    /** U component of gamma perturbation velocity... **/
    
    double &dU_dGamma(void) { return dVelocity_dGamma_[0]; };
    
    /** V component of gamma perturbation velocity... **/
    
    double &dV_dGamma(void) { return dVelocity_dGamma_[1]; };

    /** W component of gamma perturbation velocity... **/

    double &dW_dGamma(void) { return dVelocity_dGamma_[2]; };

    /** Perturbation gamma velocity vector... **/
    
    double *dVelocity_dGamma(void) { return dVelocity_dGamma_; };

    /** U component of mesh perturbation velocity... **/
    
    double &dU_dMesh(void) { return dVelocity_dMesh_[0]; };
    
    /** V component of mesh perturbation velocity... **/
    
    double &dV_dMesh(void) { return dVelocity_dMesh_[1]; };

    /** W component of mesh perturbation velocity... **/

    double &dW_dMesh(void) { return dVelocity_dMesh_[2]; };

    /** Perturbation mesh velocity vector... **/
    
    double *dVelocity_dMesh(void) { return dVelocity_dMesh_; };    
    
    /** U component of Mach perturbation velocity... **/
    
    double &dU_dMach(void) { return dVelocity_dMach_[0]; };
    
    /** V component of Mach perturbation velocity... **/
    
    double &dV_dMach(void) { return dVelocity_dMach_[1]; };

    /** W component of Mach perturbation velocity... **/

    double &dW_dMach(void) { return dVelocity_dMach_[2]; };

    /** Perturbation Mach velocity vector... **/
    
    double *dVelocity_dMach(void) { return dVelocity_dMach_; };    
    
    /** U component of stall factor perturbation velocity... **/
    
    double &dU_dStall(void) { return dVelocity_dStall_[0]; };
    
    /** V component of stall factor perturbation velocity... **/
    
    double &dV_dStall(void) { return dVelocity_dStall_[1]; };

    /** W component of stall factor perturbation velocity... **/

    double &dW_dStall(void) { return dVelocity_dStall_[2]; };

    /** Perturbation stall factor velocity vector... **/
    
    double *dVelocity_dStall(void) { return dVelocity_dStall_; };    
    
    /** U component of vortex stretching perturbation velocity... **/
    
    double &dU_dRatio(void) { return dVelocity_dRatio_[0]; };
    
    /** V component of vortex stretching factor perturbation velocity... **/
    
    double &dV_dRatio(void) { return dVelocity_dRatio_[1]; };

    /** W component of vortex stretching factor perturbation velocity... **/

    double &dW_dRatio(void) { return dVelocity_dRatio_[2]; };

    /** Perturbation vortex stretching factor velocity vector... **/
    
    double *dVelocity_dRatio(void) { return dVelocity_dRatio_; };        

    /** Partial of inviscid Fx wrt gamma **/

    double &pFx_pGamma(void) { return pForces_pGamma_[0]; };
    
    /** Partial of inviscid Fy wrt gamma **/
    
    double &pFy_pGamma(void) { return pForces_pGamma_[1]; };

    /** Partial of inviscid Fz wrt gamma **/

    double &pFz_pGamma(void) { return pForces_pGamma_[2]; };

    /** Partial of inviscid Mx wrt gamma **/

    double &pMx_pGamma(void) { return pMoments_pGamma_[0]; };
    
    /** Partial of inviscid My wrt gamma **/
    
    double &pMy_pGamma(void) { return pMoments_pGamma_[1]; };

    /** Partial of inviscid Mz wrt gamma **/

    double &pMz_pGamma(void) { return pMoments_pGamma_[2]; };

    /** Partial of viscous Fx wrt gamma **/

    double &pFxo_pGamma(void) { return pForces_pGamma_[3]; };
    
    /** Partial of viscous Fy wrt gamma **/
    
    double &pFyo_pGamma(void) { return pForces_pGamma_[4]; };

    /** Partial of viscous Fz wrt gamma **/

    double &pFzo_pGamma(void) { return pForces_pGamma_[5]; };

    /** Partial of wake induced Fx wrt gamma **/

    double &pFwx_pGamma(void) { return pForces_pGamma_[6]; };
    
    /** Partial of wake induced Fy wrt gamma **/
    
    double &pFwy_pGamma(void) { return pForces_pGamma_[7]; };

    /** Partial of wake induced Fz wrt gamma **/

    double &pFwz_pGamma(void) { return pForces_pGamma_[8]; };

    /** Partial of viscous Mx wrt gamma **/

    double &pMxo_pGamma(void) { return pMoments_pGamma_[3]; };
    
    /** Partial of viscous My wrt gamma **/
    
    double &pMyo_pGamma(void) { return pMoments_pGamma_[4]; };

    /** Partial of viscous Mz wrt gamma **/

    double &pMzo_pGamma(void) { return pMoments_pGamma_[5]; };
     
    /** Partial of inviscid Fx wrt Vortex Stretching Ratio **/

    double &pFx_pRatio(void) { return pForces_pRatio_[0]; };
    
    /** Partial of inviscid Fy wrt Vortex Stretching Ratio **/
    
    double &pFy_pRatio(void) { return pForces_pRatio_[1]; };

    /** Partial of inviscid Fz wrt Vortex Stretching Ratio **/

    double &pFz_pRatio(void) { return pForces_pRatio_[2]; };

    /** Partial of inviscid Mx wrt Vortex Stretching Ratio **/

    double &pMx_pRatio(void) { return pMoments_pRatio_[0]; };
    
    /** Partial of inviscid My wrt Vortex Stretching Ratio **/
    
    double &pMy_pRatio(void) { return pMoments_pRatio_[1]; };

    /** Partial of inviscid Mz wrt Vortex Stretching Ratio **/

    double &pMz_pRatio(void) { return pMoments_pRatio_[2]; };

    /** Partial of viscous Fx wrt Vortex Stretching Ratio **/

    double &pFxo_pRatio(void) { return pForces_pRatio_[3]; };
    
    /** Partial of viscous Fy wrt Vortex Stretching Ratio **/
    
    double &pFyo_pRatio(void) { return pForces_pRatio_[4]; };

    /** Partial of viscous Fz wrt Vortex Stretching Ratio **/

    double &pFzo_pRatio(void) { return pForces_pRatio_[5]; };

    /** Partial of wake induced Fx wrt Vortex Stretching Ratio **/

    double &pFiwx_pRatio(void) { return pForces_pRatio_[6]; };
    
    /** Partial of wake induced Fy wrt Vortex Stretching Ratio **/
    
    double &pFiwy_pRatio(void) { return pForces_pRatio_[7]; };

    /** Partial of wake induced Fz wrt Vortex Stretching Ratio **/

    double &pFiwz_pRatio(void) { return pForces_pRatio_[8]; };
    
    /** Partial of viscous Mx wrt Vortex Stretching Ratio **/

    double &pMxo_pRatio(void) { return pMoments_pRatio_[3]; };
    
    /** Partial of viscous My wrt Vortex Stretching Ratio **/
    
    double &pMyo_pRatio(void) { return pMoments_pRatio_[4]; };

    /** Partial of viscous Mz wrt Vortex Stretching Ratio **/

    double &pMzo_pRatio(void) { return pMoments_pRatio_[5]; };

    /** Principal part of the down wash integral **/
    
    double &Ws(void) { return Ws_; };
        
    /** Update the geometry given a translation vector and a quaternion rotation **/
           
    void UpdateGeometryLocation(double *TVec, double *OVec, QUAT &Quat, QUAT &InvQuat);

};

#include "END_NAME_SPACE.H"

#endif
