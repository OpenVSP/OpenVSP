//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//
//////////////////////////////////////////////////////////////////////

#ifndef VORTEX_TRAIL_H
#define VORTEX_TRAIL_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <assert.h>
#include "utils.H"
#include "time.H"
#include "VSP_Edge.H"
#include "Search.H"

#include "START_NAME_SPACE.H"

#define IMPLICIT_WAKE_GAMMAS 1
#define EXPLICIT_WAKE_GAMMAS 2
#define      ALL_WAKE_GAMMAS 3 

#define IMPULSE_ANALYSIS 1
#define HEAVE_ANALYSIS   2
#define P_ANALYSIS       3
#define Q_ANALYSIS       4
#define R_ANALYSIS       5

class SEARCH;

// Definition of the VORTEX_TRAIL class

class VORTEX_TRAIL {

private:

    void init(void);
     
    int Verbose_;
 
    // Wing, edge, and trailing node this vortex belongs to
    
    int Wing_;
    
    int Edge_;
    
    int Node_;
    
    int ComponentID_;

    // Wake points list
    
    VSPAERO_DOUBLE Length_;
    
    VSPAERO_DOUBLE FarDist_;
    
    VSPAERO_DOUBLE *S_[2];
  
    int NumberOfNodes_;
    
    VSP_NODE TE_Node_;
    
    VSP_NODE *NodeList_;
    
    // Save state data
    
    VSP_NODE TE_NodeSave_;
    
    VSP_NODE *NodeListSave_;
 
    // Wake points residual
    
    int RecordingWasOn_;
    int NumberOfWakeResidualNodes_;
    int *WakeResidualEquationNumber_;
    
    VSPAERO_DOUBLE *WakeResidual_;

    // List of trailing vortices
    
    int NumberOfLevels_;
    
    int *NumberOfSubVortices_;
    
    int TotalNumberOfSubVortices_;
    
    VSP_EDGE **VortexEdgeList_;

    VSP_EDGE *VortexEdgeList(int Level) { return VortexEdgeList_[Level]; };

    VSPAERO_DOUBLE ***VortexEdgeVelocity_;
    
    VSPAERO_DOUBLE FreeStreamVelocity_[3];
    
    // Far away ratio
    
    static double FarAway_;
    
    // Distance between trailing wakes at trailing edge
    
    VSPAERO_DOUBLE Sigma_;
    
    // Core size
    
    VSPAERO_DOUBLE CoreSize_;
    
    // Wake relaxation factor
    
    VSPAERO_DOUBLE WakeRelax_;
    
    // Minimum Tolerance
    
    VSPAERO_DOUBLE Tolerance_;

    // Location along span 
    
    VSPAERO_DOUBLE SoverB_;

    // Circulation strength
    
    int Evaluate_;
    int TimeAccurate_;
    int ConvectType_;
    int TimeAnalysisType_;
    int CurrentTimeStep_;
    int WakeDampingIsOn_;
    int IsARotor_;
    
    VSPAERO_DOUBLE RotorOrigin_[3];
    VSPAERO_DOUBLE RotorThrustVector_[3];
    VSPAERO_DOUBLE FreeStreamDirection_[3];
    
    VSPAERO_DOUBLE TimeStep_;
    VSPAERO_DOUBLE Vinf_;
    VSPAERO_DOUBLE *Gamma_;
    VSPAERO_DOUBLE *GammaSave_;

    VSPAERO_DOUBLE *WakeAge_;
        
    VSPAERO_DOUBLE *a_;
    VSPAERO_DOUBLE *b_;
    VSPAERO_DOUBLE *c_;
    VSPAERO_DOUBLE *d_;
    VSPAERO_DOUBLE *r_;
    
    VSPAERO_DOUBLE *dx_;
    VSPAERO_DOUBLE *dy_;
    VSPAERO_DOUBLE *dz_;

    // Blade analysis parameters
    
    int RotorAnalysis_;    
    VSPAERO_DOUBLE BladeRPM_;
    
    // Smooth out the trailing wake shape
    
    void Smooth(void);
    void SmoothWake(void);
    void SmoothVelocity(VSPAERO_DOUBLE *Velocity);
    void LimitVelocity(VSPAERO_DOUBLE q[3]);
    
    // Test stuff
    
    VSPAERO_DOUBLE EvaluatedLength_;
    
    // Ground effects
    
    int DoGroundEffectsAnalysis_;
    
    // Induced Velocity calculation
    
    void InducedVelocity_(VSPAERO_DOUBLE xyz_p[3], VSPAERO_DOUBLE q[3]);
    
    // Search data structure
    
    int Searched_;
    
    VSPAERO_DOUBLE Distance_;
    
    SEARCH *Search_;
    
    void CreateSearchTree_(void);
    
    // Calculate the velocity due to a sub vortex on the trailing vortex 

    void CalculateVelocityForSubVortex(VSP_EDGE &VortexEdge, VSPAERO_DOUBLE xyz_p[3], VSPAERO_DOUBLE q[3]);
     
    VSPAERO_DOUBLE GammaScale(int i);
    
    // Update the wake location 
    
    VSPAERO_DOUBLE UpdateWakeLocation_(int AdjointSolve);   
    
    /** Update the geometry given a translation vector and a rotating quaternion, this also shift the wake back in time  **/
    
    void UpdateGeometryLocation_(VSPAERO_DOUBLE *TVec, VSPAERO_DOUBLE *OVec, QUAT &Quat, QUAT &InvQuat, int AdjointSolve);
 
public:

    // Constructor, Destructor, Copy

    VORTEX_TRAIL(void);
   ~VORTEX_TRAIL(void);
    VORTEX_TRAIL(const VORTEX_TRAIL &Trailing_Vortex);

    // Copy function

    VORTEX_TRAIL& operator=(const VORTEX_TRAIL &Trailing_Vortex);
    VORTEX_TRAIL& operator+=(const VORTEX_TRAIL &Trailing_Vortex);
    
    /** Analysis is time accurate **/
    
    int &TimeAccurate(void) { return TimeAccurate_; };
    
    /** Time analysis type **/
    
    int &TimeAnalysisType(void ) { return TimeAnalysisType_; };
    
    /** Wing that this trailing vortex is shed from **/

    int &Wing(void) { return Wing_; };
    
    /** Edge attached to this trailing vortex **/
    
    int &Edge(void) { return Edge_; };
    
    /** Node on trailing edge of wing from which this trailing vortex is shed **/
    
    int &Node(void) { return Node_; };
    
    /** Component ID for the wing this trailing vortex is shed from **/
    
    int &ComponentID(void) { return ComponentID_; };
 
    /** Set up the trailing vortex... we need the xyz location of the TE (Node1), a node at infinity (NODE2)...
     * FarDist is the distance the adapted portion of the wake extends to... so the wake will have NumSubVortices
     * segments from the trailing edge out to a distance of FarDist, then a single sub vortex out to NODE2 which
     * is taken to be 'infinfity'.
     * The number of subvortices must be a power of 2... ie... 32, 64, 128... 
     **/
     
    void Setup(int NumSubVortices, VSPAERO_DOUBLE FarDist, VSP_NODE &Node1, VSP_NODE &Node2);
    
    /** Update the trailing wake geometry, does the same as setup() but does not allocate any of the data structures
     * ... this is currently used for the adjoint taping process after the mesh, or other relevant parameter has
     * changed that would impact the wake geometry
     **/
    
    void Update(VSPAERO_DOUBLE FarDist, VSP_NODE &Node1, VSP_NODE &Node2);
    void UpdateNew(VSP_NODE &TE_NODE);
    
    /** Calculate the induced velocity at location xyz **/
          
    void InducedVelocity(VSPAERO_DOUBLE xyz_p[3], VSPAERO_DOUBLE q[3]);

    /** Calculate the induced velocity at location xyz assuming a finite core size CorSize **/
    
    void InducedVelocity(VSPAERO_DOUBLE xyz_p[3], VSPAERO_DOUBLE q[3], VSPAERO_DOUBLE CorSize);

    /** Set the distance/2 between wakes at trailing edge... this is used in various cut off 
     * routines to limit the 1/r behavior **/
    
    VSPAERO_DOUBLE &Sigma(void) { return Sigma_; };

    /** Set the core size at trailing edge for the finite core model **/
    
    VSPAERO_DOUBLE &CoreSize(void) { return CoreSize_; };
    
    /** Wake relaxation factor **/
    
    VSPAERO_DOUBLE &WakeRelax(void) { return WakeRelax_; };

    /** Trailing edge location along span.. this is y/b... so ranges from 0 to 1 **/
    
    VSPAERO_DOUBLE &SoverB(void) { return SoverB_; };

    /** Trailing edge Circulation strength **/

    VSPAERO_DOUBLE &Gamma(void) { return Gamma_[0]; };
    
    /** Circulation strength along trailing vortex... for unsteady cases **/
    
    VSPAERO_DOUBLE &Gamma(int i) {  return Gamma_[i]; };
    
    /** Update the vorticity (gamma) values for sub vortices **/
    
    void UpdateGamma(void);    
     
    /** Trailing edge node **/
    
    VSP_NODE &TE_Node(void) { return TE_Node_; };
    
    /** Evaluation flag... for agglomeration multipole routine **/
    
    int &Evaluate(void) { return Evaluate_; };

    /** Number of agglomeration levels for this trailing vortex **/

    int NumberOfLevels(void) { return NumberOfLevels_; };
    
    /** Total number of subvortices across all the agglomeration levels **/
    
    int TotalNumberOfSubVortices(void) { return TotalNumberOfSubVortices_; };

    /** Number of subvortices for level i **/
    
    int NumberOfSubVortices(int i) { return NumberOfSubVortices_[i]; };
    
    /** Number of subvortices on the top level **/
    
    int NumberOfSubVortices(void) { return NumberOfSubVortices_[1]; };
    
    /** Access to each vortex edge on the finest level **/
    
    VSP_EDGE &VortexEdge(int i) { return VortexEdgeList_[1][i]; };
    
    /** Access to each vortex edge on each level **/
        
    VSP_EDGE &VortexEdge(int Level, int i) { return VortexEdgeList_[Level][i]; };
    
    /** Centroid of each sub vortex **/
    
    VSPAERO_DOUBLE *xyz_c(int i) { return VortexEdgeList_[1][i].xyz_c(); };
    
    /** Wake points location, distance, from trailing edge **/
    
    VSPAERO_DOUBLE S(int i) { return S_[0][i]; };

    /** X component of velocity for vortex edge i at the top level **/
    
    VSPAERO_DOUBLE &U(int i) { return VortexEdgeVelocity_[1][i][0]; };
    
    /** Y component of velocity for vortex edge i at the top level **/
    
    VSPAERO_DOUBLE &V(int i) { return VortexEdgeVelocity_[1][i][1]; };

    /** Z component of velocity for vortex edge i at the top level **/

    VSPAERO_DOUBLE &W(int i) { return VortexEdgeVelocity_[1][i][2]; };

    /** X component of velocity for vortex edge i at the chosen level **/

    VSPAERO_DOUBLE &U(int Level, int i) { return VortexEdgeVelocity_[Level][i][0]; };
    
    /** Y component of velocity for vortex edge i at the chosen level **/
    
    VSPAERO_DOUBLE &V(int Level, int i) { return VortexEdgeVelocity_[Level][i][1]; };

    /** Z component of velocity for vortex edge i at the chosen level **/

    VSPAERO_DOUBLE &W(int Level, int i) { return VortexEdgeVelocity_[Level][i][2]; };
    
    /** Free stream velocity component in x,y, or z ... 0, 1, or 2 **/
    
    VSPAERO_DOUBLE &FreeStreamVelocity(int i) { return FreeStreamVelocity_[i];};
 
    /** Vector for velocity at sub vortex i at the top level **/
    
    VSPAERO_DOUBLE *VortexEdgeVelocity(int i) { return VortexEdgeVelocity_[1][i]; };
    
    /** Zero out edge velocities along the trailing vortex at all levels **/
    
    void ZeroEdgeVelocities(void);
    
    /** Prolongate the edge velocites from coarse up to the finest grid **/
    
    void ProlongateEdgeVelocities(void);
    
    /** Update the wake location **/
    
    VSPAERO_DOUBLE UpdateWakeLocation(void) { return UpdateWakeLocation_(0); }; 
    
    /** Update the wake location for adjoint solve **/
    
    VSPAERO_DOUBLE UpdateWakeLocationForAdjointSolve(void) { return UpdateWakeLocation_(1); };     
    
    /** Convec the wake vorticity in time down stream ... **/ 
    
    void ConvectWakeVorticity(int ConvectType);
    
    /** Ground effects analysis flag **/
    
    int &DoGroundEffectsAnalysis(void) { return DoGroundEffectsAnalysis_; };

    /** Bladed analysis flag **/
    
    int &RotorAnalysis(void) { return RotorAnalysis_; }; 
    
    /** Blade analysis RPM **/
    
    VSPAERO_DOUBLE &BladeRPM(void) { return BladeRPM_; };
    
    /** Unsteady analysis time step **/
    
    VSPAERO_DOUBLE &TimeStep(void) { return TimeStep_; };
    
    /** Free stream velocity magnitude **/
    
    VSPAERO_DOUBLE &Vinf(void) { return Vinf_; };
 
    /** Current global time for time accurate analysis **/
    
    int &CurrentTimeStep(void) { return CurrentTimeStep_; };
    
    /** Turn on wake damping models **/
    
    int &WakeDampingIsOn(void) { return WakeDampingIsOn_; };
    
    /** This trailing vortex is shed from a rotor... we use that knowledge to do
     * rotor like thingies... mostly we add in some extra damping via the core models **/
    
    int &IsARotor(void) { return IsARotor_; };
    
    /** Origin of the rotor for the blade analysis .,.. this is a vector **/
    
    VSPAERO_DOUBLE &RotorOrigin(int i) { return RotorOrigin_[i]; };
    
    /** Thrust direction vector for the rotor... this is a vector **/
    
    VSPAERO_DOUBLE &RotorThrustVector(int i) { return RotorThrustVector_[i]; }; 
    
    /** Free stream direction vector ... assumed to be normalized **/
    
    VSPAERO_DOUBLE &FreeStreamDirection(int i) { return FreeStreamDirection_[i]; };
    
    /** Create the binary search tree for fast search of trailing wake nodes to determine
     * how far we are from a point we need to calculate the induced velocities for **/
    
    void CreateSearchTree(void) { CreateSearchTree_(); };

    /** Save the current vortex (circulation) state... both now, and the past few time steps **/
    
    void SaveVortexState(void);

    /** Write trailing edge vortex data to a file **/
    
    void WriteToFile(FILE *adb_file);
    
    /** Read in trailing edge vortex data from a file **/
    
    void ReadInFile(FILE *adb_file);
    
    /** Skip over trailing edge vortex data in file ... **/
    
    void SkipReadInFile(FILE *adb_file);
    
    /** Update the geometry given a translation vector and a rotating quaternion, this also shift the wake back in time  **/
    
    void UpdateGeometryLocation(VSPAERO_DOUBLE *TVec, VSPAERO_DOUBLE *OVec, QUAT &Quat, QUAT &InvQuat);
 
    /** Update the trailing edge location given a translation vector and a rotating quaternion **/
    
    void UpdateTrailingEdgeGeometryLocation(VSPAERO_DOUBLE *TVec, VSPAERO_DOUBLE *OVec, QUAT &Quat, QUAT &InvQuat);

    /** Update the trailing edge location given just the new trailing edge xyz location **/
    
    void UpdateTrailingEdgeGeometryLocation(VSP_NODE &Node);
       
    /** Set Mach number **/
     
    void SetMachNumber(VSPAERO_DOUBLE Mach);

    /** This trailing vortex was searched by the tree routine **/
    
    int &Searched(void) { return Searched_; };
    
    /** Distance from the sample point **/
    
    VSPAERO_DOUBLE &Distance(void) { return Distance_; };
    
    /** Pointer to the binary tree search **/

    SEARCH &Search(void) { return *Search_; };
    
    /** Ratio that determines how far away a point has to be relative to the size of the 
     * trailing vortex to move up an agglomeration level **/
    
    double &FarAwayRatio(void) { return FarAway_; };

    /** Save the current wake shape state **/
    
    void SaveWakeShapeState(void);
    
    /** Restore the wake shape from previously saved state **/
    
    void RestoreWakeShapeState(void);    
    
    /** Calculate the unsteady wake residual  **/
    
    void UpdateGeometryLocationForAdjointSolve(VSP_NODE TENode);       
        
    /** Number of nodes in wake that we are solving for, this is 1 less than the total
     * since the last node is simply out at infinity from the n-1 node ... */
     
    int NumberOfWakeResidualNodes(void) { return NumberOfWakeResidualNodes_; };
  
    /** Wake node x **/
    
    VSPAERO_DOUBLE &WakeNodeX(int i) { return NodeList_[i].x(); };
    
    /** Wake node y **/
    
    VSPAERO_DOUBLE &WakeNodeY(int i) { return NodeList_[i].y(); };

    /** Wake node z **/
    
    VSPAERO_DOUBLE &WakeNodeZ(int i) { return NodeList_[i].z(); };

    /** Wake residual for x location at node i **/
    
    VSPAERO_DOUBLE &WakeResidualX(int i) { return WakeResidual_[3*i-2]; };
    
    /** Wake residual for y location at node i **/
    
    VSPAERO_DOUBLE &WakeResidualY(int i) { return WakeResidual_[3*i-1]; };

    /** Wake residual for z location at node i **/
    
    VSPAERO_DOUBLE &WakeResidualZ(int i) { return WakeResidual_[3*i  ]; };
    
    /** Global wake residual equation number for x location at node i **/
    
    int &WakeResidualX_EquationNumber(int i) { return WakeResidualEquationNumber_[3*i-2]; };

    /** Global wake residual equation number for x location at node i **/
    
    int &WakeResidualY_EquationNumber(int i) { return WakeResidualEquationNumber_[3*i-1]; };

    /** Global wake residual equation number for x location at node i **/
    
    int &WakeResidualZ_EquationNumber(int i) { return WakeResidualEquationNumber_[3*i  ]; };
                    
};

#include "END_NAME_SPACE.H"

#endif
