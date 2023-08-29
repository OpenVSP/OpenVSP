//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//
//////////////////////////////////////////////////////////////////////

#ifndef SPAN_LOAD_ROTOR_DATA_H
#define SPAN_LOAD_ROTOR_DATA_H

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "utils.H"

#include "START_NAME_SPACE.H"

class SPAN_LOAD_ROTOR_DATA {

private:

    int WakeSurface_;
    int LiftingSurface_;
    
    // Spanwise forces and moments, for wings and rotors
    
    int NumberOfSpanStations_;
    int NumberOfTimeSamples_;
    int ActualTimeSamples_;
    
    VSPAERO_DOUBLE *Span_Cxo_;    // Local viscous x force coefficient
    VSPAERO_DOUBLE *Span_Cyo_;    // Local viscous y force coefficient
    VSPAERO_DOUBLE *Span_Czo_;    // Local viscous z force coefficient
                          
    VSPAERO_DOUBLE *Span_Cx_;     // Local inviscid x force coefficient
    VSPAERO_DOUBLE *Span_Cy_;     // Local inviscid y force coefficient
    VSPAERO_DOUBLE *Span_Cz_;     // Local inviscid z force coefficient
                         
    VSPAERO_DOUBLE *Span_Cmxo_;   // Local viscous x moment coefficient
    VSPAERO_DOUBLE *Span_Cmyo_;   // Local viscous y moment coefficient
    VSPAERO_DOUBLE *Span_Cmzo_;   // Local viscous z moment coefficient
     
    VSPAERO_DOUBLE *Span_Cmx_;    // Local inviscid x moment coefficient
    VSPAERO_DOUBLE *Span_Cmy_;    // Local inviscid y moment coefficient
    VSPAERO_DOUBLE *Span_Cmz_;    // Local inviscid z moment coefficient
        
    VSPAERO_DOUBLE *Span_Clo_;    // Local viscous lift force coefficient
    VSPAERO_DOUBLE *Span_Cdo_;    // Local viscous drag force coefficient
    VSPAERO_DOUBLE *Span_Cwo_;    // Local viscous side force coefficient
            
    VSPAERO_DOUBLE *Span_Cl_;     // Local inviscid lift force coefficient
    VSPAERO_DOUBLE *Span_Cd_;     // Local inviscid drag force coefficient
    VSPAERO_DOUBLE *Span_Cw_;     // Local inviscid side force coefficient
    
    VSPAERO_DOUBLE *Span_Cno_;    // Local viscous 2d normal      force coefficient
    VSPAERO_DOUBLE *Span_Cso_;    // Local viscous 2d tangential  force coefficient
    VSPAERO_DOUBLE *Span_Cto_;    // Local viscous 2d thrust      force coefficient
    VSPAERO_DOUBLE *Span_Cqo_;    // Local viscous 2d torque            coefficient   
    VSPAERO_DOUBLE *Span_Cpo_;    // Local viscous 2d power             coefficient   
                          
    VSPAERO_DOUBLE *Span_Cn_;     // Local inviscid 2d normal     force coefficient
    VSPAERO_DOUBLE *Span_Cs_;     // Local inviscid 2d tangential force coefficient    
    VSPAERO_DOUBLE *Span_Ct_;     // Local inviscid 2d thrust     force coefficient
    VSPAERO_DOUBLE *Span_Cq_;     // Local inviscid 2d torque           coefficient    
    VSPAERO_DOUBLE *Span_Cp_;     // Local inviscid 2d power            coefficient
    
    VSPAERO_DOUBLE *Time_;
    VSPAERO_DOUBLE *XYZ_QC_[3];
    VSPAERO_DOUBLE *RotationAngle_;
    VSPAERO_DOUBLE *Span_S_;
    VSPAERO_DOUBLE *Span_Area_;
    VSPAERO_DOUBLE *Span_Chord_;
    VSPAERO_DOUBLE *Local_Velocity_;
    
public:

    SPAN_LOAD_ROTOR_DATA(void);
   ~SPAN_LOAD_ROTOR_DATA(void);
    SPAN_LOAD_ROTOR_DATA(const SPAN_LOAD_ROTOR_DATA &SPAN_LOAD_ROTOR_DATA);

    SPAN_LOAD_ROTOR_DATA& operator=(const SPAN_LOAD_ROTOR_DATA &SPAN_LOAD_ROTOR_DATA);
    
    int &WakeSurface(void) { return WakeSurface_; };
    int &LiftingSurface(void) { return LiftingSurface_; };

    /** Set number of spanwise stations **/
    
    void SetNumberOfSpanStations(int NumberOfSpanStations) { NumberOfSpanStations_ = NumberOfSpanStations; };
    
    /** Set number of time samples for time averaging of time accurate solutions **/
    
    void SetNumberOfNumberOfTimeSamples(int NumberOfTimeSamples) { NumberOfTimeSamples_ = NumberOfTimeSamples; };
    
    /** Size the list **/
    
    void SizeSpanLoadingList(void);
    
    /** Number of spanwise stations **/
    
    int NumberOfSpanStations(void) { return NumberOfSpanStations_; };
    
    /** Number of time samples for time averaging of time accurate solutions **/
    
    int NumberOfTimeSamples(void) { return NumberOfTimeSamples_; };
    
    /** Viscous force coefficient in X ... for TimeStep, and SpanStation **/
    
    VSPAERO_DOUBLE &Span_Cxo(int TimeStep, int SpanStation) { return Span_Cxo_[TimeStep*NumberOfSpanStations_ + SpanStation]; };
    
    /** Viscous force coefficient in Y... for TimeStep, and SpanStation **/
    
    VSPAERO_DOUBLE &Span_Cyo(int TimeStep, int SpanStation) { return Span_Cyo_[TimeStep*NumberOfSpanStations_ + SpanStation]; };

    /** Viscous force coefficient in Z... for TimeStep, and SpanStation **/

    VSPAERO_DOUBLE &Span_Czo(int TimeStep, int SpanStation) { return Span_Czo_[TimeStep*NumberOfSpanStations_ + SpanStation]; };
        
    /** Inviscid force coefficient in X... for TimeStep, and SpanStation **/
        
    VSPAERO_DOUBLE &Span_Cx(int TimeStep, int SpanStation) { return Span_Cx_[TimeStep*NumberOfSpanStations_ + SpanStation]; };
    
    /** Inviscid force coefficient in Y... for TimeStep, and SpanStation **/
    
    VSPAERO_DOUBLE &Span_Cy(int TimeStep, int SpanStation) { return Span_Cy_[TimeStep*NumberOfSpanStations_ + SpanStation]; };

    /** Inviscid force coefficient in Z... for TimeStep, and SpanStation **/

    VSPAERO_DOUBLE &Span_Cz(int TimeStep, int SpanStation) { return Span_Cz_[TimeStep*NumberOfSpanStations_ + SpanStation]; };

    /** Viscous lift coefficient ... for TimeStep, and SpanStation **/
    
    VSPAERO_DOUBLE &Span_Clo(int TimeStep, int SpanStation) { return Span_Clo_[TimeStep*NumberOfSpanStations_ + SpanStation]; };

    /** Viscous drag coefficient ... for TimeStep, and SpanStation **/
    
    VSPAERO_DOUBLE &Span_Cdo(int TimeStep, int SpanStation) { return Span_Cdo_[TimeStep*NumberOfSpanStations_ + SpanStation]; };

    /** Viscous side force coefficient ... for TimeStep, and SpanStation **/

    VSPAERO_DOUBLE &Span_Cwo(int TimeStep, int SpanStation) { return Span_Cwo_[TimeStep*NumberOfSpanStations_ + SpanStation]; };

    /** Inviscid lift coefficient ... for TimeStep, and SpanStation **/

    VSPAERO_DOUBLE &Span_Cl(int TimeStep, int SpanStation) { return Span_Cl_[TimeStep*NumberOfSpanStations_ + SpanStation]; };

    /** Inviscid drag coefficient ... for TimeStep, and SpanStation **/

    VSPAERO_DOUBLE &Span_Cd(int TimeStep, int SpanStation) { return Span_Cd_[TimeStep*NumberOfSpanStations_ + SpanStation]; };

    /** Inviscid side force coefficient ... for TimeStep, and SpanStation **/

    VSPAERO_DOUBLE &Span_Cw(int TimeStep, int SpanStation) { return Span_Cw_[TimeStep*NumberOfSpanStations_ + SpanStation]; };
       
    /** Viscous moment about X ... for TimeStep, and SpanStation **/
       
    VSPAERO_DOUBLE &Span_Cmxo(int TimeStep, int SpanStation) { return Span_Cmxo_[TimeStep*NumberOfSpanStations_ + SpanStation]; };

    /** Viscous moment about Y ... for TimeStep, and SpanStation **/

    VSPAERO_DOUBLE &Span_Cmyo(int TimeStep, int SpanStation) { return Span_Cmyo_[TimeStep*NumberOfSpanStations_ + SpanStation]; };

    /** Viscous moment about Z ... for TimeStep, and SpanStation **/

    VSPAERO_DOUBLE &Span_Cmzo(int TimeStep, int SpanStation) { return Span_Cmzo_[TimeStep*NumberOfSpanStations_ + SpanStation]; };
           
    /** Inviscid moment about X ... for TimeStep, and SpanStation **/
           
    VSPAERO_DOUBLE &Span_Cmx(int TimeStep, int SpanStation) { return Span_Cmx_[TimeStep*NumberOfSpanStations_ + SpanStation]; };

    /** Inviscid moment about Y ... for TimeStep, and SpanStation **/

    VSPAERO_DOUBLE &Span_Cmy(int TimeStep, int SpanStation) { return Span_Cmy_[TimeStep*NumberOfSpanStations_ + SpanStation]; };

    /** Inviscid moment about Z ... for TimeStep, and SpanStation **/

    VSPAERO_DOUBLE &Span_Cmz(int TimeStep, int SpanStation) { return Span_Cmz_[TimeStep*NumberOfSpanStations_ + SpanStation]; };

    /** Viscous normal force coefficient... for TimeStep, and SpanStation **/
    
    VSPAERO_DOUBLE &Span_Cno(int TimeStep, int SpanStation) { return Span_Cno_[TimeStep*NumberOfSpanStations_ + SpanStation]; };
    
    /** Viscous tangential force coefficient... for TimeStep, and SpanStation **/

    VSPAERO_DOUBLE &Span_Cso(int TimeStep, int SpanStation) { return Span_Cso_[TimeStep*NumberOfSpanStations_ + SpanStation]; };
    
    /** Viscous thrust coefficient... for TimeStep, and SpanStation **/
    
    VSPAERO_DOUBLE &Span_Cto(int TimeStep, int SpanStation) { return Span_Cto_[TimeStep*NumberOfSpanStations_ + SpanStation]; };
    
    /** Viscous torque coefficient... for TimeStep, and SpanStation **/
    
    VSPAERO_DOUBLE &Span_Cqo(int TimeStep, int SpanStation) { return Span_Cqo_[TimeStep*NumberOfSpanStations_ + SpanStation]; };
    
    /** Viscous power coefficient... for TimeStep, and SpanStation **/
    
    VSPAERO_DOUBLE &Span_Cpo(int TimeStep, int SpanStation) { return Span_Cpo_[TimeStep*NumberOfSpanStations_ + SpanStation]; };
           
    /** Inviscid normal force coefficient... for TimeStep, and SpanStation **/
           
    VSPAERO_DOUBLE &Span_Cn(int TimeStep, int SpanStation) { return Span_Cn_[TimeStep*NumberOfSpanStations_ + SpanStation]; };
    
    /** Inviscid tangential force coefficient... for TimeStep, and SpanStation **/
    
    VSPAERO_DOUBLE &Span_Cs(int TimeStep, int SpanStation) { return Span_Cs_[TimeStep*NumberOfSpanStations_ + SpanStation]; };
    
    /** Inviscid thrust coefficient... for TimeStep, and SpanStation **/
    
    VSPAERO_DOUBLE &Span_Ct(int TimeStep, int SpanStation) { return Span_Ct_[TimeStep*NumberOfSpanStations_ + SpanStation]; };
    
    /** Inviscid torque coefficient... for TimeStep, and SpanStation **/
    
    VSPAERO_DOUBLE &Span_Cq(int TimeStep, int SpanStation) { return Span_Cq_[TimeStep*NumberOfSpanStations_ + SpanStation]; };
    
    /** Inviscid power coefficient... for TimeStep, and SpanStation **/
    
    VSPAERO_DOUBLE &Span_Cp(int TimeStep, int SpanStation) { return Span_Cp_[TimeStep*NumberOfSpanStations_ + SpanStation]; };
    
    /** X coordinate of quarter chord location **/
    
    VSPAERO_DOUBLE &X_QC(int TimeStep, int SpanStation) { return XYZ_QC_[0][TimeStep*NumberOfSpanStations_ + SpanStation]; };

    /** Y coordinate of quarter chord location **/

    VSPAERO_DOUBLE &Y_QC(int TimeStep, int SpanStation) { return XYZ_QC_[1][TimeStep*NumberOfSpanStations_ + SpanStation]; };

    /** Z coordinate of quarter chord location **/

    VSPAERO_DOUBLE &Z_QC(int TimeStep, int SpanStation) { return XYZ_QC_[2][TimeStep*NumberOfSpanStations_ + SpanStation]; };
    
    /** Rotation angle for this time step and span location... **/
    
    VSPAERO_DOUBLE &RotationAngle(int TimeStep, int SpanStation) { return RotationAngle_[TimeStep*NumberOfSpanStations_ + SpanStation]; };

    /** Current analysis time **/
    
    VSPAERO_DOUBLE &Time(int TimeStep, int SpanStation) { return Time_[TimeStep*NumberOfSpanStations_ + SpanStation]; };
   
    /** Span fraction for this TimeStep, and SpanStation **/
    
    VSPAERO_DOUBLE         &Span_S(int TimeStep, int SpanStation) { return         Span_S_[TimeStep*NumberOfSpanStations_ + SpanStation]; };
    
    /** Span area for this TimeStep, and SpanStation **/
    
    VSPAERO_DOUBLE      &Span_Area(int TimeStep, int SpanStation) { return      Span_Area_[TimeStep*NumberOfSpanStations_ + SpanStation]; };
    
    /** Span chord for this TimeStep, and SpanStation **/
    
    VSPAERO_DOUBLE     &Span_Chord(int TimeStep, int SpanStation) { return     Span_Chord_[TimeStep*NumberOfSpanStations_ + SpanStation]; };
    
    /** Span averaged velocity for this TimeStep, and SpanStation **/
    
    VSPAERO_DOUBLE &Local_Velocity(int TimeStep, int SpanStation) { return Local_Velocity_[TimeStep*NumberOfSpanStations_ + SpanStation]; };
                        
    /** Averaged viscous force coefficient in X ... for SpanStation **/
    
    VSPAERO_DOUBLE &Span_Cxo(int SpanStation) { return Span_Cxo_[SpanStation]; };

    /** Averaged viscous force coefficient in Y ... for SpanStation **/

    VSPAERO_DOUBLE &Span_Cyo(int SpanStation) { return Span_Cyo_[SpanStation]; };

    /** Averaged viscous force coefficient in Z ... for SpanStation **/

    VSPAERO_DOUBLE &Span_Czo(int SpanStation) { return Span_Czo_[SpanStation]; };
        
    /** Averaged inviscid force coefficient in X ... for SpanStation **/

    VSPAERO_DOUBLE &Span_Cx(int SpanStation) { return Span_Cx_[SpanStation]; };

    /** Averaged inviscid force coefficient in Y ... for SpanStation **/

    VSPAERO_DOUBLE &Span_Cy(int SpanStation) { return Span_Cy_[SpanStation]; };

    /** Averaged inviscid force coefficient in Z ... for SpanStation **/

    VSPAERO_DOUBLE &Span_Cz(int SpanStation) { return Span_Cz_[SpanStation]; };
    
    /** Averaged viscous lift coefficient ... for SpanStation **/

    VSPAERO_DOUBLE &Span_Clo(int SpanStation) { return Span_Clo_[SpanStation]; };

    /** Averaged viscous drag coefficient ... for SpanStation **/

    VSPAERO_DOUBLE &Span_Cdo(int SpanStation) { return Span_Cdo_[SpanStation]; };

    /** Averaged viscous side force coefficient ... for SpanStation **/

    VSPAERO_DOUBLE &Span_Cwo(int SpanStation) { return Span_Cwo_[SpanStation]; };

    /** Averaged inviscid lift coefficient ... for SpanStation **/

    VSPAERO_DOUBLE &Span_Cl(int SpanStation) { return Span_Cl_[SpanStation]; };

    /** Averaged inviscid drag coefficient ... for TimeStep, and SpanStation **/

    VSPAERO_DOUBLE &Span_Cd(int SpanStation) { return Span_Cd_[SpanStation]; };

    /** Averaged inviscid side force coefficient ... for SpanStation **/

    VSPAERO_DOUBLE &Span_Cw(int SpanStation) { return Span_Cw_[SpanStation]; };
    
    /** Averaged viscous moment about X  ... for SpanStation **/

    VSPAERO_DOUBLE &Span_Cmxo(int SpanStation) { return Span_Cmxo_[SpanStation]; };

    /** Averaged viscous moment about Y ... for TimeStep, and SpanStation **/

    VSPAERO_DOUBLE &Span_Cmyo(int SpanStation) { return Span_Cmyo_[SpanStation]; };

    /** Averaged viscous moment about Z ... for SpanStation **/

    VSPAERO_DOUBLE &Span_Cmzo(int SpanStation) { return Span_Cmzo_[SpanStation]; };

    /** Averaged inviscid moment about X ... for SpanStation **/
                                      
    VSPAERO_DOUBLE &Span_Cmx(int SpanStation) { return Span_Cmx_[SpanStation]; };

    /** Averaged inviscid moment about Y ... for SpanStation **/

    VSPAERO_DOUBLE &Span_Cmy(int SpanStation) { return Span_Cmy_[SpanStation]; };

    /** Averaged inviscid moment about Z ... for SpanStation **/

    VSPAERO_DOUBLE &Span_Cmz(int SpanStation) { return Span_Cmz_[SpanStation]; };
                           
    /** Averaged viscous normal force coefficient ... for SpanStation **/
    
    VSPAERO_DOUBLE &Span_Cno(int SpanStation) { return Span_Cno_[SpanStation]; };

    /** Averaged viscous tangential force coefficient ... for SpanStation **/

    VSPAERO_DOUBLE &Span_Cso(int SpanStation) { return Span_Cso_[SpanStation]; };

    /** Averaged viscous thrust coefficient ... for SpanStation **/

    VSPAERO_DOUBLE &Span_Cto(int SpanStation) { return Span_Cto_[SpanStation]; };

    /** Averaged viscous torque coefficient ... for SpanStation **/

    VSPAERO_DOUBLE &Span_Cqo(int SpanStation) { return Span_Cqo_[SpanStation]; };

    /** Averaged viscous power coefficient ... for SpanStation **/

    VSPAERO_DOUBLE &Span_Cpo(int SpanStation) { return Span_Cpo_[SpanStation]; };
           
    /** Averaged inviscid normal force coefficient ... for SpanStation **/

    VSPAERO_DOUBLE &Span_Cn(int SpanStation) { return Span_Cn_[SpanStation]; };

    /** Averaged inviscid tangential force coefficient ... for SpanStation **/

    VSPAERO_DOUBLE &Span_Cs(int SpanStation) { return Span_Cs_[SpanStation]; };

    /** Averaged inviscid thrust coefficient ... for SpanStation **/

    VSPAERO_DOUBLE &Span_Ct(int SpanStation) { return Span_Ct_[SpanStation]; };

    /** Averaged inviscid torque coefficient ... for SpanStation **/

    VSPAERO_DOUBLE &Span_Cq(int SpanStation) { return Span_Cq_[SpanStation]; };

    /** Averaged inviscid power coefficient ... for SpanStation **/

    VSPAERO_DOUBLE &Span_Cp(int SpanStation) { return Span_Cp_[SpanStation]; };
 
    /** Averaged span fraction for this TimeStep, and SpanStation **/

    VSPAERO_DOUBLE         &Span_S(int SpanStation) { return         Span_S_[SpanStation]; };

    /** Averaged span area for this TimeStep, and SpanStation **/

    VSPAERO_DOUBLE      &Span_Area(int SpanStation) { return      Span_Area_[SpanStation]; };

    /** Averaged span chord for this TimeStep, and SpanStation **/

    VSPAERO_DOUBLE     &Span_Chord(int SpanStation) { return     Span_Chord_[SpanStation]; };

    /** Averaged span averaged velocity for this TimeStep, and SpanStation **/

    VSPAERO_DOUBLE &Local_Velocity(int SpanStation) { return Local_Velocity_[SpanStation]; };

    /** Actual number of time samples that have been collected **/
        
    int &ActualTimeSamples(void) { return ActualTimeSamples_; };

    /** Calculate the average forces and moments given the time history data collected **/

    void CalculateAverageForcesAndMoments(void);

    /** Zero out the forces and moments **/

    void ZeroForcesAndMoments(void);

};

#include "END_NAME_SPACE.H"

#endif
