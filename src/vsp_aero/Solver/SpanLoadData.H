//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//
//////////////////////////////////////////////////////////////////////

#ifndef SPAN_LOAD_DATA_H
#define SPAN_LOAD_DATA_H

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "utils.H"
#include "quat.H"

#include "START_NAME_SPACE.H"

class SPAN_LOAD_DATA {

private:

    int NumberOfSpanStations_;
    
    // Surface ID
    
    int *SurfaceID_;
    
    // Component ID
    
    int *ComponentID_;
    
    // Leading edge loop
    
    int *SpanLeadingEdgeVortexLoop_;
    
    // Viscous forces
      
    VSPAERO_DOUBLE *Span_Cxo_;
    VSPAERO_DOUBLE *Span_Cyo_;
    VSPAERO_DOUBLE *Span_Czo_;  
              
    // Inviscid forces
                
    VSPAERO_DOUBLE *Span_Cx_;
    VSPAERO_DOUBLE *Span_Cy_;
    VSPAERO_DOUBLE *Span_Cz_;

    // Trefftz, induced drag, forces
    
    VSPAERO_DOUBLE *Span_Cxi_; 
    VSPAERO_DOUBLE *Span_Cyi_;
    VSPAERO_DOUBLE *Span_Czi_;  
    
    // Viscous moments
      
    VSPAERO_DOUBLE *Span_Cmxo_; 
    VSPAERO_DOUBLE *Span_Cmyo_;
    VSPAERO_DOUBLE *Span_Cmzo_;
          
    // Inviscid moments
          
    VSPAERO_DOUBLE *Span_Cmx_;  
    VSPAERO_DOUBLE *Span_Cmy_;
    VSPAERO_DOUBLE *Span_Cmz_;

    // Trefftz, induced drag, moments

    VSPAERO_DOUBLE *Span_Cmxi_; 
    VSPAERO_DOUBLE *Span_Cmyi_;
    VSPAERO_DOUBLE *Span_Cmzi_;
    
    // Viscous lift, drag, spanwise forces
            
    VSPAERO_DOUBLE *Span_Clo_;
    VSPAERO_DOUBLE *Span_Cso_;
    VSPAERO_DOUBLE *Span_Cdo_;
        
    // Inviscid lift, drag, normal, spanwise forces
            
    VSPAERO_DOUBLE *Span_Cn_;
    VSPAERO_DOUBLE *Span_Cl_;
    VSPAERO_DOUBLE *Span_Cs_;
    VSPAERO_DOUBLE *Span_Cd_;
    
    // Mininum Local Cp

    VSPAERO_DOUBLE *Span_CpMin_;    
    
    // Geometrical information
    
    VSPAERO_DOUBLE *Span_XLE_;
    VSPAERO_DOUBLE *Span_YLE_;
    VSPAERO_DOUBLE *Span_ZLE_;
        
    VSPAERO_DOUBLE *Span_XTE_;
    VSPAERO_DOUBLE *Span_YTE_;
    VSPAERO_DOUBLE *Span_ZTE_;
    
    VSPAERO_DOUBLE *Span_XQC_;
    VSPAERO_DOUBLE *Span_YQC_;
    VSPAERO_DOUBLE *Span_ZQC_;
        
    VSPAERO_DOUBLE *Span_XLE_Def_;
    VSPAERO_DOUBLE *Span_YLE_Def_;
    VSPAERO_DOUBLE *Span_ZLE_Def_;
        
    VSPAERO_DOUBLE *Span_XTE_Def_;
    VSPAERO_DOUBLE *Span_YTE_Def_;
    VSPAERO_DOUBLE *Span_ZTE_Def_;
        
    VSPAERO_DOUBLE *Span_Xavg_;
    VSPAERO_DOUBLE *Span_Yavg_;
    VSPAERO_DOUBLE *Span_Zavg_;

    VSPAERO_DOUBLE **Span_Svec_;    
    
    VSPAERO_DOUBLE **Span_Nvec_;
    
    VSPAERO_DOUBLE *Span_Area_;
    
    VSPAERO_DOUBLE *Span_WettedArea_;
    
    VSPAERO_DOUBLE *Span_Chord_;
    
    VSPAERO_DOUBLE *Span_S_;

    VSPAERO_DOUBLE *Span_S_Def_;
    
    // Root LE, TE, and Quarter chord
    
    VSPAERO_DOUBLE Root_LE_[3];
    VSPAERO_DOUBLE Root_TE_[3];
    VSPAERO_DOUBLE Root_QC_[3];
    
    // Tip LE, TE, and Quarter chord
    
    VSPAERO_DOUBLE Tip_LE_[3];
    VSPAERO_DOUBLE Tip_TE_[3];
    VSPAERO_DOUBLE Tip_QC_[3];    
    
    // Local Velocity
    
    VSPAERO_DOUBLE **Local_Velocity_;
    
    // Stall flag
    
    VSPAERO_DOUBLE *StallFact_;
    
    int *IsStalled_;
    
    VSPAERO_DOUBLE *SlatPercentage_;
    VSPAERO_DOUBLE *SlatMachDeploy_;
    
    int *SpanStationHasSlat_;
    
    // Rotor flag
    
    int IsARotor_;
    
public:

    SPAN_LOAD_DATA(void);
   ~SPAN_LOAD_DATA(void);
    SPAN_LOAD_DATA(const SPAN_LOAD_DATA &SpanLoadData);

    SPAN_LOAD_DATA& operator=(const SPAN_LOAD_DATA &SpanLoadData);

    /** Size the lists **/
    
    void Size(int NumberOfSpanStations);

    /** Zero all data **/
    
    void ZeroAll(void);
    
    /** Zero just the force and moment data **/
    
    void ZeroForcesAndMoments(void);
    
    /** Number of span stations **/

    int NumberOfSpanStations(void) { return NumberOfSpanStations_ ; };
    
    /** Update the geometry information given a translation vector, and a quaternion rotation **/
    
    void UpdateGeometryLocation(VSPAERO_DOUBLE *TVec, VSPAERO_DOUBLE *OVec, QUAT &Quat, QUAT &InvQuat, int *ComponentInThisGroup);
   
    /** Surface ID **/
    
    int &SurfaceID(int i) { return SurfaceID_[i]; };
    
    /** Component ID **/
    
    int &ComponentID(int i) { return ComponentID_[i]; };

    // Leading edge loop
    
    int &SpanLeadingEdgeVortexLoop(int i) { return SpanLeadingEdgeVortexLoop_[i]; };
            
    /** Viscous force coefficient in X **/
      
    VSPAERO_DOUBLE &Span_Cxo(int i) { return Span_Cxo_[i]; };
    
    /** Viscous force coefficient in Y **/
    
    VSPAERO_DOUBLE &Span_Cyo(int i) { return Span_Cyo_[i]; };
    
    /** Viscous force coefficient in Z **/
    
    VSPAERO_DOUBLE &Span_Czo(int i) { return Span_Czo_[i]; };
              
    /** Inviscid force coefficient in X **/
                
    VSPAERO_DOUBLE &Span_Cx(int i) { return Span_Cx_[i]; };

    /** Inviscid force coefficient in Y **/
    
    VSPAERO_DOUBLE &Span_Cy(int i) { return Span_Cy_[i]; };

    /** Inviscid force coefficient in Z **/

    VSPAERO_DOUBLE &Span_Cz(int i) { return Span_Cz_[i]; };
           
    /** Trefftz drag force coefficient... component in X **/
    
    VSPAERO_DOUBLE &Span_Cxi(int i) { return Span_Cxi_[i]; }; 
    
    /** Trefftz drag force coefficient... component in y **/
    
    VSPAERO_DOUBLE &Span_Cyi(int i) { return Span_Cyi_[i]; };

    /** Trefftz drag force coefficient... component in Z **/

    VSPAERO_DOUBLE &Span_Czi(int i) { return Span_Czi_[i]; };  
    
    /** Viscous moment coefficient about X **/
      
    VSPAERO_DOUBLE &Span_Cmxo(int i) { return Span_Cmxo_[i]; }; 
    
    /** Viscous moment coefficient about Y **/
    
    VSPAERO_DOUBLE &Span_Cmyo(int i) { return Span_Cmyo_[i]; };

    /** Viscous moment coefficient about Z **/

    VSPAERO_DOUBLE &Span_Cmzo(int i) { return Span_Cmzo_[i]; };
          
    /** Inviscid moment coefficient about X **/
          
    VSPAERO_DOUBLE &Span_Cmx(int i) { return Span_Cmx_[i]; };  
    
    /** Inviscid moment coefficient about Y **/
    
    VSPAERO_DOUBLE &Span_Cmy(int i) { return Span_Cmy_[i]; };

    /** Inviscid moment coefficient about Z **/

    VSPAERO_DOUBLE &Span_Cmz(int i) { return Span_Cmz_[i]; };
             
    /** Trefftz drag induced moment coefficient about X **/

    VSPAERO_DOUBLE &Span_Cmxi(int i) { return Span_Cmxi_[i]; }; 
    
    /** Trefftz drag induced moment coefficient about Y **/
    
    VSPAERO_DOUBLE &Span_Cmyi(int i) { return Span_Cmyi_[i]; };

    /** Trefftz drag induced moment coefficient about Z **/

    VSPAERO_DOUBLE &Span_Cmzi(int i) { return Span_Cmzi_[i]; };
    
    /** Inviscid normal force coefficient **/
            
    VSPAERO_DOUBLE &Span_Cn(int i) { return Span_Cn_[i]; };
    
    /** Inviscid lift coefficient **/
    
    VSPAERO_DOUBLE &Span_Cl(int i) { return Span_Cl_[i]; };
    
    /** Inviscid side force coefficient **/
    
    VSPAERO_DOUBLE &Span_Cs(int i) { return Span_Cs_[i]; };
    
    /** Inviscid drag coefficient **/
    
    VSPAERO_DOUBLE &Span_Cd(int i) { return Span_Cd_[i]; };

    /** Viscous lift coefficient **/
    
    VSPAERO_DOUBLE &Span_Clo(int i) { return Span_Clo_[i]; };
    
    /** Viscous side force coefficient **/
    
    VSPAERO_DOUBLE &Span_Cso(int i) { return Span_Cso_[i]; };
    
    /** Viscous drag coefficient **/
    
    VSPAERO_DOUBLE &Span_Cdo(int i) { return Span_Cdo_[i]; };
    
    /** Minimum Cp at this span station **/
    
    VSPAERO_DOUBLE &Span_CpMin(int i) { return Span_CpMin_[i]; };
    
    /** X coordinate of leading edge **/
    
    VSPAERO_DOUBLE &Span_XLE( int i) { return Span_XLE_[i]; };

    /** Y coordinate of leading edge **/
    
    VSPAERO_DOUBLE &Span_YLE( int i) { return Span_YLE_[i]; };

    /** Z coordinate of leading edge **/

    VSPAERO_DOUBLE &Span_ZLE( int i) { return Span_ZLE_[i]; };
    
    /** X coordinate of trailing edge **/
        
    VSPAERO_DOUBLE &Span_XTE( int i) { return Span_XTE_[i]; };
    
    /** Y coordinate of trailing edge **/
    
    VSPAERO_DOUBLE &Span_YTE( int i) { return Span_YTE_[i]; };

    /** Z coordinate of trailing edge **/

    VSPAERO_DOUBLE &Span_ZTE( int i) { return Span_ZTE_[i]; };

    /** X coordinate of quarter chord **/

    VSPAERO_DOUBLE &Span_XQC( int i) { return Span_XQC_[i]; };
    
    /** Y coordinate of quarter chord **/
    
    VSPAERO_DOUBLE &Span_YQC( int i) { return Span_YQC_[i]; };

    /** Z coordinate of quarter chord **/

    VSPAERO_DOUBLE &Span_ZQC( int i) { return Span_ZQC_[i]; };
        
    /** Deflected X coordinate of trailing edge **/

    VSPAERO_DOUBLE &Span_XLE_Def( int i) { return Span_XLE_Def_[i]; };
    
    /** Deflected Y coordinate of trailing edge **/
    
    VSPAERO_DOUBLE &Span_YLE_Def( int i) { return Span_YLE_Def_[i]; };

    /** Deflected Z coordinate of trailing edge **/

    VSPAERO_DOUBLE &Span_ZLE_Def( int i) { return Span_ZLE_Def_[i]; };
    
    /** Deflected X coordinate of trailing edge **/
    
    VSPAERO_DOUBLE &Span_XTE_Def( int i) { return Span_XTE_Def_[i]; };
    
    /** Deflected Y coordinate of trailing edge **/
    
    VSPAERO_DOUBLE &Span_YTE_Def( int i) { return Span_YTE_Def_[i]; };

    /** Deflected Z coordinate of trailing edge **/

    VSPAERO_DOUBLE &Span_ZTE_Def( int i) { return Span_ZTE_Def_[i]; };
        
    /** Average X coordinate of this spanwise station **/
        
    VSPAERO_DOUBLE &Span_Xavg(int i) { return Span_Xavg_[i]; };
    
    /** Average Y coordinate of this spanwise station **/
    
    VSPAERO_DOUBLE &Span_Yavg(int i) { return Span_Yavg_[i]; };

    /** Average Z coordinate of this spanwise station **/

    VSPAERO_DOUBLE &Span_Zavg(int i) { return Span_Zavg_[i]; };
    
    /** Area for this spanwise slice **/
    
    VSPAERO_DOUBLE &Span_Area(int i) { return Span_Area_[i]; };

    /** Wetted area for this spanwise slice **/
    
    VSPAERO_DOUBLE &Span_WettedArea(int i) { return Span_WettedArea_[i]; };    
    
    /** Chord for this spanwise slice **/
    
    VSPAERO_DOUBLE &Span_Chord(int i) { return Span_Chord_[i]; };
    
    /** Span of this spanwise slice **/
    
    VSPAERO_DOUBLE &Span_S(int i) { return Span_S_[i]; };

    /** Deflected span of this spanwise slice **/
    
    VSPAERO_DOUBLE &Span_S_Def(int i) { return Span_S_Def_[i]; };

    /** Vector pointing from trailing edge to leading edge of this spanwise slice **/
    
    VSPAERO_DOUBLE *Span_Svec(int i) { return Span_Svec_[i]; };
    
    /** Normal vector for this spanwise slice **/
    
    VSPAERO_DOUBLE *Span_Nvec(int i) { return Span_Nvec_[i]; };

    /** Root leading edge coordinates **/
    
    VSPAERO_DOUBLE &Root_LE(int i) { return Root_LE_[i]; };
    
    /** Root trailing edge coordinates **/
    
    VSPAERO_DOUBLE &Root_TE(int i) { return Root_TE_[i]; };
       
    /** Root quater chord coordinates **/
       
    VSPAERO_DOUBLE &Root_QC(int i) { return Root_QC_[i]; };
    
    /** Tip leading edge coordinates **/
    
    VSPAERO_DOUBLE &Tip_LE(int i) { return Tip_LE_[i]; };
    
    /** Tip trailing edge coordinates **/
    
    VSPAERO_DOUBLE &Tip_TE(int i) { return Tip_TE_[i]; };
    
    /** Tip quarter chord coordinates **/
    
    VSPAERO_DOUBLE &Tip_QC(int i) { return Tip_QC_[i]; };  
    
    /** Local averaged velocity for this spanwise strip **/
    
    VSPAERO_DOUBLE *Span_Local_Velocity(int i) { return Local_Velocity_[i]; };
    
    /** Fraction of this spanwise slice that is stalled **/
    
    VSPAERO_DOUBLE &StallFact(int i) { return StallFact_[i]; };
     
    /** Flag indicating if this spanwise section has stalled based on 2D models ... **/
     
    int &IsStalled(int i) { return IsStalled_[i]; };
    
    /** Flag indicating that this spanwise station has a leading edge slat **/
    
    int &SpanStationHasSlat(int i ) { return SpanStationHasSlat_[i]; };
    
    /** Percentage of chord (actually a fraction) that the slat makes up for this spanwise slice **/
    
    VSPAERO_DOUBLE &SlatPercentage(int i) { return SlatPercentage_[i]; };
    
    /** Mach number, below which, any possible slat for this spanwise slice would deploy **/
    
    VSPAERO_DOUBLE &SlatMachDeploy(int i) { return SlatMachDeploy_[i]; };
    
    /** Flag indicating if this span load data is tied to a rotor blade **/
    
    int &IsARotor(void) { return IsARotor_; };
     

};

#include "END_NAME_SPACE.H"

#endif
