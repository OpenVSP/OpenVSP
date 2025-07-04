//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//
//////////////////////////////////////////////////////////////////////

#ifndef CONTROL_SURFACE_GROUP_H
#define CONTROL_SURFACE_GROUP_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <assert.h>
#include "utils.H"
#include "CharSizes.H"

#include "START_NAME_SPACE.H"

// Definition of the CONTROL_SURFACE_GROUP class

class CONTROL_SURFACE_GROUP {

private:

    // Control group name
    
    char Name_[MAX_CHAR_SIZE];
    
    // List of control surfaces
    
    int NumberOfControlSurfaces_;
    
    char **ControlSurface_Name_;
    
    // Control surface deflection sign
    
    double *ControlSurface_DeflectionDirection_;
    
    // Control surface deflection
    
    double ControlSurface_DeflectionAngle_;

    // Aero Control derivatives
    
    double pCLi_pDelta_;
    double pCDi_pDelta_;
    double pCSi_pDelta_;
    
    double pCMli_pDelta_;    
    double pCMmi_pDelta_;    
    double pCMni_pDelta_;    

    double pCLo_pDelta_;
    double pCDo_pDelta_;
    double pCSo_pDelta_;
    
    double pCMlo_pDelta_;    
    double pCMmo_pDelta_;    
    double pCMno_pDelta_;    

    // Force coefficient derivatives
    
    double pCFix_pDelta_;
    double pCFiy_pDelta_;
    double pCFiz_pDelta_;
 
    double pCMix_pDelta_;
    double pCMiy_pDelta_;
    double pCMiz_pDelta_;
    
    double pCFox_pDelta_;
    double pCFoy_pDelta_;
    double pCFoz_pDelta_;
 
    double pCMox_pDelta_;
    double pCMoy_pDelta_;
    double pCMoz_pDelta_;      
        
public:

    // Constructor, Destructor, Copy

    CONTROL_SURFACE_GROUP(void);
   ~CONTROL_SURFACE_GROUP(void);
    CONTROL_SURFACE_GROUP(const CONTROL_SURFACE_GROUP &ControlSurfaceGroup);
    CONTROL_SURFACE_GROUP& operator=(const CONTROL_SURFACE_GROUP &ControlSurfaceGroup);
    
    /** Control surface group name **/
    
    char *Name(void) { return Name_; };
    
    /** Size the list for the number of control surfaces **/
    
    void SizeList(int NumberOfControlSurfaces);
    
    /** Number of control surfaces in this group **/
    
    int NumberOfControlSurfaces(void) { return NumberOfControlSurfaces_; };
    
    /** Set, or access the control surface name **/
    
    char *ControlSurface_Name(int i) { return ControlSurface_Name_[i]; };
    
    /** Control surface deflection sign ... this allows for async motion... like ailerons **/
    
    double &ControlSurface_DeflectionDirection(int i) { return ControlSurface_DeflectionDirection_[i]; };
    
    /** Control surface deflection angle, degrees **/
    
    double &ControlSurface_DeflectionAngle(void) { return ControlSurface_DeflectionAngle_; };

    // Aero Control derivatives
    
    double &pCLi_pDelta(void) { return pCLi_pDelta_; };
    double &pCDi_pDelta(void) { return pCDi_pDelta_; };
    double &pCSi_pDelta(void) { return pCSi_pDelta_; };
                  
    double &pCMli_pDelta(void) { return pCMli_pDelta_; }; 
    double &pCMmi_pDelta(void) { return pCMmi_pDelta_; };  
    double &pCMni_pDelta(void) { return pCMni_pDelta_; };  
                 
    double &pCLo_pDelta(void) { return pCLo_pDelta_; };
    double &pCDo_pDelta(void) { return pCDo_pDelta_; };
    double &pCSo_pDelta(void) { return pCSo_pDelta_; };
                 
    double &pCMlo_pDelta(void) { return pCMlo_pDelta_; };
    double &pCMmo_pDelta(void) { return pCMmo_pDelta_; }; 
    double &pCMno_pDelta(void) { return pCMno_pDelta_; };

    double pCLt_pDelta(void) { return pCLo_pDelta_ + pCLi_pDelta_; };
    double pCDt_pDelta(void) { return pCDo_pDelta_ + pCDi_pDelta_; };
    double pCSt_pDelta(void) { return pCSo_pDelta_ + pCSi_pDelta_; };
                                                                         ;
    double pCMlt_pDelta(void) { return pCMlo_pDelta_ + pCMli_pDelta_; };
    double pCMmt_pDelta(void) { return pCMmo_pDelta_ + pCMmi_pDelta_; }; 
    double pCMnt_pDelta(void) { return pCMno_pDelta_ + pCMni_pDelta_; };
    
    // Force coefficient derivatives
    
    double &pCFix_pDelta(void) { return pCFix_pDelta_; };
    double &pCFiy_pDelta(void) { return pCFiy_pDelta_; };
    double &pCFiz_pDelta(void) { return pCFiz_pDelta_; };
                  
    double &pCMix_pDelta(void) { return pCMix_pDelta_; };
    double &pCMiy_pDelta(void) { return pCMiy_pDelta_; };
    double &pCMiz_pDelta(void) { return pCMiz_pDelta_; };
                  
    double &pCFox_pDelta(void) { return pCFox_pDelta_; };
    double &pCFoy_pDelta(void) { return pCFoy_pDelta_; };
    double &pCFoz_pDelta(void) { return pCFoz_pDelta_; };
                  
    double &pCMox_pDelta(void) { return pCMox_pDelta_; };
    double &pCMoy_pDelta(void) { return pCMoy_pDelta_; };
    double &pCMoz_pDelta(void) { return pCMoz_pDelta_; };
    
    double pCFtx_pDelta(void) { return pCFox_pDelta_ + pCFix_pDelta_; };
    double pCFty_pDelta(void) { return pCFoy_pDelta_ + pCFiy_pDelta_; };
    double pCFtz_pDelta(void) { return pCFoz_pDelta_ + pCFiz_pDelta_; };
                  
    double pCMtx_pDelta(void) { return pCMox_pDelta_ + pCMix_pDelta_; };
    double pCMty_pDelta(void) { return pCMoy_pDelta_ + pCMiy_pDelta_; };
    double pCMtz_pDelta(void) { return pCMoz_pDelta_ + pCMiz_pDelta_; };    
        
};

#include "END_NAME_SPACE.H"

#endif
