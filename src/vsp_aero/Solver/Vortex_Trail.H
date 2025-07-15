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
 
    // Wing this trailing vortex is attached to
    
    int Wing_;
    
    // Global trailing edge this trailing vortex is attached to
    
    int TE_Edge_;
    
    // Global trailing edge node this trailing vortex is attached to
    
    int TE_Node_;
    
    // Flag noting if this trailing vortex is shed from some sort of concave (root like) region
    
    int TE_Node_Region_Is_Concave_;
 
    // Trailing vortex is frozen 

    int IsFrozenTrailingEdge_;
    
    // Leading edge edge
    
    int LE_Edge_;
    
    // Component ID of wing this trailing edge is attached to
    
    int ComponentID_;

    // Number of wake nodes
 
    int NumberOfNodes_;

    // Wake node list
    
    VSP_NODE *NodeList_;
    
    // Wake node list for previous time step
    
    VSP_NODE *NodeList_Time_M_1_;

    // Global wake edge list
    
    VSP_EDGE **EdgeList_;

    // Wake node spacing, 0 ~ current, 1 ~ previous time step
    
    double *S_[2];

    // Global node
    
    int *GlobalNode_;

    // Global edge
    
    int *GlobalEdge_;
       
    // Free stream velocity

    double FreeStreamVelocity_[3];
    
    // Wake node residual global equation number
    
    int *WakeResidualEquationNumber_;
    
    // Stall equation global equation number
    
    int KuttaStallEquationNumber_;
    
    // Stall factor
    
    double StallFactor_;
    
    // dStall factor for GMRES solve
    
    double dStallFactor_;
    
    // Adjoint variable solution
    
    double Psi_;
 
    // Local chord
    
    double LocalChord_;
    
    // Local span
    
    double LocalSpan_;
        
    // Local inviscid Cl based on trailing edge circulation
    
    double Cli_;

    // Local inviscid Cd based on trailing edge circulation
    
    double Cdi_;
     
    // Local inviscid Cs based on trailing edge circulation

    double Csi_;

    // Local viscous Cd based on trailing edge circulation
        
    double Clo_;
                
    // Local viscous Cd based on trailing edge circulation
        
    double Cdo_;
                     
    // Local viscous Cd based on trailing edge circulation
        
    double Cso_;

    // Local invscid Cmx based on trailing edge circulation
    
    double CFix_;
    
    // Local invscid Cmy based on trailing edge circulation
    
    double CFiy_;
    
    // Local invscid Cmy based on trailing edge circulation
    
    double CFiz_;
                
    // Local viscous Cmx based on trailing edge circulation
    
    double CFox_;
    
    // Local viscous Cmy based on trailing edge circulation
    
    double CFoy_;
    
    // Local viscous Cmy based on trailing edge circulation
    
    double CFoz_;
    
    // Local invscid Cmx based on trailing edge circulation
    
    double CMix_;
    
    // Local invscid Cmy based on trailing edge circulation
    
    double CMiy_;
    
    // Local invscid Cmy based on trailing edge circulation
    
    double CMiz_;
                
    // Local viscous Cmx based on trailing edge circulation
    
    double CMox_;
    
    // Local viscous Cmy based on trailing edge circulation
    
    double CMoy_;
    
    // Local viscous Cmy based on trailing edge circulation
    
    double CMoz_;

    // Distance between trailing wakes at trailing edge
    
    double Sigma_;
    
    // Core size
    
    double CoreSize_;

    // Minimum Tolerance
    
    double Tolerance_;

    // Location along span 
    
    double SoverB_;

    // Time accurate flag
    
    int TimeAccurate_;
    
    // Time analaysis type... for rotor models
    
    int TimeAnalysisType_;
    
    // Current global time step
    
    int Time_;    
           
    // Wake convection type - implicit, explicit, or all
    
    int ConvectType_;
    
    // Turn on wake damping
    
    int WakeDampingIsOn_;

    // Turn on vortex stretching model
    
    int DoVortexStretching_;
    
    // Group this wake is attached to
    
    int Group_;
        
    // Wake is coming off a rotor
    
    int IsARotor_;
     
    // Rotor analysis type flag
    
    int RotorAnalysis_;    
    
    double BladeRPM_;
        
    // Rotor origin
        
    double RotorOrigin_[3];
    
    // Rotor thrust direction vector
    
    double RotorThrustVector_[3];
    
    // Free streaam direction
    
    double FreeStreamDirection_[3];

    // Free stream velocity magnitude
        
    double Vinf_;

    // Integration time step
    
    double DeltaTime_;

    // Baseline wake core width
     
    double CoreWidth_;

    // Ground effects
    
    int DoGroundEffectsAnalysis_;
    
    double GammaScale(int i);

    // Partial of inviscid, viscous, and waked induced forces wrt stall factor for this trailing edge

    double pForces_pStallFactor_[9];

    // Partial of total moments wrt stall factor for this trailing edge

    double pMoments_pStallFactor_[6];

public:

    // Constructor, Destructor, Copy

    VORTEX_TRAIL(void);
   ~VORTEX_TRAIL(void);
    VORTEX_TRAIL(const VORTEX_TRAIL &Trailing_Vortex);

    // Copy function

    VORTEX_TRAIL& operator=(const VORTEX_TRAIL &Trailing_Vortex);
    
    /** Number of wake nodes **/
    
    int NumberOfNodes(void) { return NumberOfNodes_; };

    /** Analysis is time accurate **/
    
    int &TimeAccurate(void) { return TimeAccurate_; };
    
    /** Time analysis type **/
    
    int &TimeAnalysisType(void ) { return TimeAnalysisType_; };
    
    /** Wing that this trailing vortex is shed from **/

    int &Wing(void) { return Wing_; };
    
    /** Edge attached to this trailing vortex **/
    
    int &TE_Edge(void) { return TE_Edge_; };
    
    /** Node on trailing edge of wing from which this trailing vortex is shed **/
    
    int &TE_Node(void) { return TE_Node_; };
    
    /** Flag noting if this trailing vortex is shed from some sort of concave (root like) region **/
    
    int &TE_Node_Region_Is_Concave(void) { return TE_Node_Region_Is_Concave_; };
    
    /** Trailing vortex is frozen **/

    int &IsFrozenTrailingEdge(void) { return IsFrozenTrailingEdge_; };   
        
    /** Edge attached to this trailing vortex's leading edge **/
    
    int &LE_Edge(void) { return LE_Edge_; };    
        
    /** Component ID for the wing this trailing vortex is shed from **/
    
    int &ComponentID(void) { return ComponentID_; };
 
    /** Set up the trailing vortex... we need the xyz location of the TE (Node1), a node at infinity (NODE2)...
     * FarDist is the distance the adapted portion of the wake extends to... so the wake will have NumSubVortices
     * segments from the trailing edge out to a distance of FarDist, then a single sub vortex out to NODE2 which
     * is taken to be 'infinfity'.
     * The number of subvortices must be a power of 2... ie... 32, 64, 128... 
     **/
     
    void Setup(int NumWakeNodes, double FarDist, VSP_NODE &Node1, VSP_NODE &Node2);
    
    /** Size a trailing vortex data structure **/
    
    void SizeList(int NumWakeNodes);
    
    /** Update the wake spacing **/
    
    void UpdateWakeSpacing(void);
    
     /** Set the distance/2 between wakes at trailing edge... this is used in various cut off 
     * routines to limit the 1/r behavior **/
    
    double &Sigma(void) { return Sigma_; };

    /** Set the core size at trailing edge for the finite core model **/
    
    double &CoreSize(void) { return CoreSize_; };
    
    /** Trailing edge location along span.. this is y/b... so ranges from 0 to 1 **/
    
    double &SoverB(void) { return SoverB_; };

    /** Wake points location, distance, from trailing edge **/
    
    double S(int i) { return S_[0][i]; };
    
    /** Wake points location, distance, from trailing edge **/
    
    double dS(int i) { return (S_[0][i+1] - S_[0][i]); };    
 
    /** Free stream velocity component in x,y, or z ... 0, 1, or 2 **/
    
    double &FreeStreamVelocity(int i) { return FreeStreamVelocity_[i];};
    
    /** Update the wake location **/
    
    void UpdateWakeLocation(double WakeRelax, double &ResMax, double &ResL2);

    double UpdateWakeLocationOld(double WakeRelax);

    /** Store the current wake location in the t-1 arrays **/
    
    void StoreWakeLocation(void);
    
    void UpdateUnsteadyWakeLocation(double WakeRelax, double &ResMax, double &ResL2);

    /** Ground effects analysis flag **/
    
    int &DoGroundEffectsAnalysis(void) { return DoGroundEffectsAnalysis_; };

    /** Bladed analysis flag **/
    
    int &RotorAnalysis(void) { return RotorAnalysis_; }; 
    
    /** Blade analysis RPM **/
    
    double &BladeRPM(void) { return BladeRPM_; };
    
    /** Unsteady analysis time step **/
    
    double &DeltaTime(void) { return DeltaTime_; };
    
    /** Free stream velocity magnitude **/
    
    double &Vinf(void) { return Vinf_; };
 
    /** Current global time for time accurate analysis **/
    
    int &Time(void) { return Time_; };
    
    /** Turn on wake damping models **/
    
    int &WakeDampingIsOn(void) { return WakeDampingIsOn_; };
    
    // Group this wake is attached to
    
    int &Group(void) { return Group_; };
            
    /** This trailing vortex is shed from a rotor... we use that knowledge to do
     * rotor like thingies... mostly we add in some extra damping via the core models **/
    
    int &IsARotor(void) { return IsARotor_; };
    
    /** Origin of the rotor for the blade analysis .,.. this is a vector **/
    
    double &RotorOrigin(int i) { return RotorOrigin_[i]; };

    double *RotorOrigin(void) { return RotorOrigin_; };
    
    /** Thrust direction vector for the rotor... this is a vector **/
    
    double &RotorThrustVector(int i) { return RotorThrustVector_[i]; }; 
    
    double *RotorThrustVector(void) { return RotorThrustVector_; }; 
    
    /** Free stream direction vector ... assumed to be normalized **/
    
    double &FreeStreamDirection(int i) { return FreeStreamDirection_[i]; };
    
    double *FreeStreamDirection(void) { return FreeStreamDirection_; };
    
    /** Create the binary search tree for fast search of trailing wake nodes to determine
     * how far we are from a point we need to calculate the induced velocities for **/
    
    /** Turn on vortex stretching model **/
    
    int &DoVortexStretching(void) { return DoVortexStretching_; };

    /** Write trailing vortex data to a file **/
    
    void WriteToFile(FILE *adb_file);
    
    /** Read in trailing vortex data from a file **/
    
    void ReadInFile(FILE *adb_file);
    
    /** Skip over trailing evortex data in file ... **/
    
    void SkipReadInFile(FILE *adb_file);

    /** Wake node x **/
    
    double &WakeNodeX(int i) { return NodeList_[i].x(); };
    
    /** Wake node y **/
    
    double &WakeNodeY(int i) { return NodeList_[i].y(); };

    /** Wake node z **/
    
    double &WakeNodeZ(int i) { return NodeList_[i].z(); };

    /** Wake node x at time - 1, for time accurate solutions **/
    
    double &WakeNodeX_TM1(int i) { return NodeList_Time_M_1_[i].x(); };
    
    /** Wake node y at time - 1, for time accurate solutions **/
    
    double &WakeNodeY_TM1(int i) { return NodeList_Time_M_1_[i].y(); };

    /** Wake node z at time - 1, for time accurate solutions **/
    
    double &WakeNodeZ_TM1(int i) { return NodeList_Time_M_1_[i].z(); };
        
    /** Edge pointer list **/

    VSP_EDGE *&EdgeList(int i) { return EdgeList_[i]; };
    
    /** Global wake node **/
    
    int &GlobalNode(int i) { return GlobalNode_[i]; };

    /** Wake node residual global equation number **/
    
    int &WakeResidualEquationNumber(int i) { return WakeResidualEquationNumber_[i]; };
    
    /** Wake x node residual global equation number **/
    
    int &WakeResidualEquationNumberX(int i) { return WakeResidualEquationNumber_[3*i-2]; };

    /** Wake y node residual global equation number **/
                                                                  
    int &WakeResidualEquationNumberY(int i) { return WakeResidualEquationNumber_[3*i-1]; };

    /** Wake z node residual global equation number **/
                                                                      
    int &WakeResidualEquationNumberZ(int i) { return WakeResidualEquationNumber_[3*i  ]; };

    /** Global grid edge **/
    
    int &GlobalEdge(int i) { return GlobalEdge_[i]; };
    
    /** Stall equation global equation number **/
    
    int &KuttaStallEquationNumber(void) { return KuttaStallEquationNumber_; };

    /** Stall factor **/
    
    double &StallFactor(void) { return StallFactor_; };
    
    /** dStall factor **/
    
    double &dStallFactor(void) { return dStallFactor_; };    

    /** Adjoint variable solution **/
    
    double &Psi(void) { return Psi_; };    
    
    /** Local chord **/
    
    double &LocalChord(void) { return LocalChord_; };
    
    /** Local span **/
    
    double &LocalSpan(void) { return LocalSpan_; };

    /** Local inviscid Cl based on trailing edge circulation **/

    double &Cli(void) { return Cli_; };

    /** Local inviscid Cd based on trailing edge circulation **/
    
    double &Cdi(void) { return Cdi_; };
     
    /** Local viscous Cs based on trailing edge circulation **/
    
    double &Csi(void) { return Csi_; };

    /** Local viscous Cl based on trailing edge circulation **/
        
    double &Clo(void) { return Clo_; };
                
    /** Local viscous Cd based on trailing edge circulation **/
        
    double &Cso(void) { return Cso_; };
                             
    /** Local viscous Cd based on trailing edge circulation **/
        
    double &Cdo(void) { return Cdo_; };
     
    /** Local inviscid CFx based on trailing edge circulation **/
    
    double &CFix(void) { return CFix_; };
    
    /** Local inviscid CFy based on trailing edge circulation **/
    
    double &CFiy(void) { return CFiy_; };
    
    /** Local inviscid CFy based on trailing edge circulation **/
    
    double &CFiz(void) { return CFiz_; };
                
    /** Local viscous CFx based on trailing edge circulation **/
    
    double &CFox(void) { return CFox_; };
    
    /** Local viscous CFy based on trailing edge circulation **/
    
    double &CFoy(void) { return CFoy_; };
    
    /** Local viscous CFz based on trailing edge circulation **/
    
    double &CFoz(void) { return CFoz_; };    
    
    /** Local invscid Cmx based on trailing edge circulation **/
    
    double &CMix(void) { return CMix_; };
    
    /** Local invscid Cmy based on trailing edge circulation **/
    
    double &CMiy(void) { return CMiy_; };
    
    /** Local invscid Cmy based on trailing edge circulation **/
    
    double &CMiz(void) { return CMiz_; };
                
    /** Local viscous Cmx based on trailing edge circulation **/
    
    double &CMox(void) { return CMox_; };
    
    /** Local viscous Cmy based on trailing edge circulation **/
    
    double &CMoy(void) { return CMoy_; };
    
    /** Local viscous Cmz based on trailing edge circulation **/
    
    double &CMoz(void) { return CMoz_; };    
            
    /** Partial of inviscid Fx wrt stall factor **/

    double &pFx_pStallFactor(void) { return pForces_pStallFactor_[0]; };
    
    /** Partial of inviscid Fy wrt stall factor **/
    
    double &pFy_pStallFactor(void) { return pForces_pStallFactor_[1]; };

    /** Partial of inviscid Fz wrt stall factor **/

    double &pFz_pStallFactor(void) { return pForces_pStallFactor_[2]; };

    /** Partial of inviscid Mx wrt stall factor **/

    double &pMx_pStallFactor(void) { return pMoments_pStallFactor_[0]; };
    
    /** Partial of inviscid My wrt stall factor **/
    
    double &pMy_pStallFactor(void) { return pMoments_pStallFactor_[1]; };

    /** Partial of inviscid Mz wrt stall factor **/

    double &pMz_pStallFactor(void) { return pMoments_pStallFactor_[2]; };    

    /** Partial of viscous Fx wrt stall factor **/

    double &pFxo_pStallFactor(void) { return pForces_pStallFactor_[3]; };
    
    /** Partial of viscous Fy wrt stall factor **/
    
    double &pFyo_pStallFactor(void) { return pForces_pStallFactor_[4]; };

    /** Partial of viscous Fz wrt stall factor **/

    double &pFzo_pStallFactor(void) { return pForces_pStallFactor_[5]; };

    /** Partial of viscous Mx wrt stall factor **/

    double &pMxo_pStallFactor(void) { return pMoments_pStallFactor_[3]; };
    
    /** Partial of viscous My wrt stall factor **/
    
    double &pMyo_pStallFactor(void) { return pMoments_pStallFactor_[4]; };

    /** Partial of viscous Mz wrt stall factor **/

    double &pMzo_pStallFactor(void) { return pMoments_pStallFactor_[5]; };    

    /** Partial of wake induced Fx wrt stall factor **/

    double &pFwx_pStallFactor(void) { return pForces_pStallFactor_[6]; };
    
    /** Partial of wake induced Fy wrt stall factor **/
    
    double &pFwy_pStallFactor(void) { return pForces_pStallFactor_[7]; };

    /** Partial of wake induced Fz wrt stall factor **/

    double &pFwz_pStallFactor(void) { return pForces_pStallFactor_[8]; };

};

#include "END_NAME_SPACE.H"

#endif
