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

class SPAN_LOAD_DATA {

private:

    int NumberOfSpanStations_;
    
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
    
    // Lift, drag, normal, spanwise forces
            
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
    
    // Rotor flag
    
    int IsARotor_;
    
public:

    SPAN_LOAD_DATA(void);
   ~SPAN_LOAD_DATA(void);
    SPAN_LOAD_DATA(const SPAN_LOAD_DATA &SPAN_LOAD_DATA);

    SPAN_LOAD_DATA& operator=(const SPAN_LOAD_DATA &SPAN_LOAD_DATA);

    void Size(int NumberOfSpanStations);

    void ZeroAll(void);
    void ZeroForcesAndMoments(void);

    int NumberOfSpanStations(void) { return NumberOfSpanStations_ ; };
    
    void UpdateGeometryLocation(VSPAERO_DOUBLE *TVec, VSPAERO_DOUBLE *OVec, QUAT &Quat, QUAT &InvQuat);
    
    // Viscous forces
      
    VSPAERO_DOUBLE &Span_Cxo(int i) { return Span_Cxo_[i]; };
    VSPAERO_DOUBLE &Span_Cyo(int i) { return Span_Cyo_[i]; };
    VSPAERO_DOUBLE &Span_Czo(int i) { return Span_Czo_[i]; };
              
    // Inviscid forces
                
    VSPAERO_DOUBLE &Span_Cx(int i) { return Span_Cx_[i]; };
    VSPAERO_DOUBLE &Span_Cy(int i) { return Span_Cy_[i]; };
    VSPAERO_DOUBLE &Span_Cz(int i) { return Span_Cz_[i]; };
           
    // Trefftz, induced drag, forces
    
    VSPAERO_DOUBLE &Span_Cxi(int i) { return Span_Cxi_[i]; }; 
    VSPAERO_DOUBLE &Span_Cyi(int i) { return Span_Cyi_[i]; };
    VSPAERO_DOUBLE &Span_Czi(int i) { return Span_Czi_[i]; };  
    
    // Viscous moments
      
    VSPAERO_DOUBLE &Span_Cmxo(int i) { return Span_Cmxo_[i]; }; 
    VSPAERO_DOUBLE &Span_Cmyo(int i) { return Span_Cmyo_[i]; };
    VSPAERO_DOUBLE &Span_Cmzo(int i) { return Span_Cmzo_[i]; };
          
    // Inviscid moments
          
    VSPAERO_DOUBLE &Span_Cmx(int i) { return Span_Cmx_[i]; };  
    VSPAERO_DOUBLE &Span_Cmy(int i) { return Span_Cmy_[i]; };
    VSPAERO_DOUBLE &Span_Cmz(int i) { return Span_Cmz_[i]; };
             
    // Trefftz, induced drag, moments

    VSPAERO_DOUBLE &Span_Cmxi(int i) { return Span_Cmxi_[i]; }; 
    VSPAERO_DOUBLE &Span_Cmyi(int i) { return Span_Cmyi_[i]; };
    VSPAERO_DOUBLE &Span_Cmzi(int i) { return Span_Cmzi_[i]; };
    
    // Lift, drag, normal, spanwise forces
            
    VSPAERO_DOUBLE &Span_Cn(int i) { return Span_Cn_[i]; };
    VSPAERO_DOUBLE &Span_Cl(int i) { return Span_Cl_[i]; };
    VSPAERO_DOUBLE &Span_Cs(int i) { return Span_Cs_[i]; };
    VSPAERO_DOUBLE &Span_Cd(int i) { return Span_Cd_[i]; };
    
    // Mininum Local Cp
    
    VSPAERO_DOUBLE &Span_CpMin(int i) { return Span_CpMin_[i]; };
    
    // Geometrical information
    
    VSPAERO_DOUBLE &Span_XLE( int i) { return Span_XLE_[i]; };
    VSPAERO_DOUBLE &Span_YLE( int i) { return Span_YLE_[i]; };
    VSPAERO_DOUBLE &Span_ZLE( int i) { return Span_ZLE_[i]; };
    
    VSPAERO_DOUBLE &Span_XTE( int i) { return Span_XTE_[i]; };
    VSPAERO_DOUBLE &Span_YTE( int i) { return Span_YTE_[i]; };
    VSPAERO_DOUBLE &Span_ZTE( int i) { return Span_ZTE_[i]; };

    VSPAERO_DOUBLE &Span_XLE_Def( int i) { return Span_XLE_Def_[i]; };
    VSPAERO_DOUBLE &Span_YLE_Def( int i) { return Span_YLE_Def_[i]; };
    VSPAERO_DOUBLE &Span_ZLE_Def( int i) { return Span_ZLE_Def_[i]; };
    
    VSPAERO_DOUBLE &Span_XTE_Def( int i) { return Span_XTE_Def_[i]; };
    VSPAERO_DOUBLE &Span_YTE_Def( int i) { return Span_YTE_Def_[i]; };
    VSPAERO_DOUBLE &Span_ZTE_Def( int i) { return Span_ZTE_Def_[i]; };
        
    VSPAERO_DOUBLE &Span_Xavg(int i) { return Span_Xavg_[i]; };
    VSPAERO_DOUBLE &Span_Yavg(int i) { return Span_Yavg_[i]; };
    VSPAERO_DOUBLE &Span_Zavg(int i) { return Span_Zavg_[i]; };
    
    VSPAERO_DOUBLE &Span_Area(int i) { return Span_Area_[i]; };
    
    VSPAERO_DOUBLE &Span_Chord(int i) { return Span_Chord_[i]; };
    
    VSPAERO_DOUBLE &Span_S(int i) { return Span_S_[i]; };

    VSPAERO_DOUBLE &Span_S_Def(int i) { return Span_S_Def_[i]; };

    VSPAERO_DOUBLE *Span_Svec(int i) { return Span_Svec_[i]; };
    
    VSPAERO_DOUBLE *Span_Nvec(int i) { return Span_Nvec_[i]; };

    // Root LE, TE, and Quarter chord
    
    VSPAERO_DOUBLE &Root_LE(int i) { return Root_LE_[i]; };
    VSPAERO_DOUBLE &Root_TE(int i) { return Root_TE_[i]; };
    VSPAERO_DOUBLE &Root_QC(int i) { return Root_QC_[i]; };
    
    // Tip LE, TE, and Quarter chord
    
    VSPAERO_DOUBLE &Tip_LE(int i) { return Tip_LE_[i]; };
    VSPAERO_DOUBLE &Tip_TE(int i) { return Tip_TE_[i]; };
    VSPAERO_DOUBLE &Tip_QC(int i) { return Tip_QC_[i]; };  
    
    // Local velocity
    
    VSPAERO_DOUBLE *Span_Local_Velocity(int i) { return Local_Velocity_[i]; };
    
    // Stall Flag
    
    VSPAERO_DOUBLE &StallFact(int i) { return StallFact_[i]; };
     
    int &IsStalled(int i) { return IsStalled_[i]; };
    
    // Rotor Flag
    
    int &IsARotor(void) { return IsARotor_; };
     

};

#endif
