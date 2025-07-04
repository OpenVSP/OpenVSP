//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//
//////////////////////////////////////////////////////////////////////

#include "ControlSurfaceGroup.H"

#include "START_NAME_SPACE.H"

/*##############################################################################
#                                                                              #
#                        CONTROL_SURFACE_GROUP constructor                     #
#                                                                              #
##############################################################################*/

CONTROL_SURFACE_GROUP::CONTROL_SURFACE_GROUP(void)
{

    // List of control surfaces
    
    NumberOfControlSurfaces_ = 0;
    
    // Control surface name
    
    ControlSurface_Name_ = NULL;
    
    // Control surface deflection sign
    
    ControlSurface_DeflectionDirection_ = NULL;
    
    // Control surface deflection
    
    ControlSurface_DeflectionAngle_ = 0.;;

    // Aero Control derivatives
    
    pCLi_pDelta_  = 0.;
    pCDi_pDelta_  = 0.;
    pCSi_pDelta_  = 0.;
                 
    pCMli_pDelta_ = 0.;    
    pCMmi_pDelta_ = 0.;    
    pCMni_pDelta_ = 0.;    
                 
    pCLo_pDelta_  = 0.;
    pCDo_pDelta_  = 0.;
    pCSo_pDelta_  = 0.;
               
    pCMlo_pDelta_ = 0.;    
    pCMmo_pDelta_ = 0.;    
    pCMno_pDelta_ = 0.;    

    // Force coefficient derivatives
    
    pCFix_pDelta_ = 0.;
    pCFiy_pDelta_ = 0.;
    pCFiz_pDelta_ = 0.;
                
    pCMix_pDelta_ = 0.;
    pCMiy_pDelta_ = 0.;
    pCMiz_pDelta_ = 0.;
             
    pCFox_pDelta_ = 0.;
    pCFoy_pDelta_ = 0.;
    pCFoz_pDelta_ = 0.;
                 
    pCMox_pDelta_ = 0.;
    pCMoy_pDelta_ = 0.;
    pCMoz_pDelta_ = 0.;      
            
}

/*##############################################################################
#                                                                              #
#                           CONTROL_SURFACE_GROUP Copy                         #
#                                                                              #
##############################################################################*/

CONTROL_SURFACE_GROUP::CONTROL_SURFACE_GROUP(const CONTROL_SURFACE_GROUP &ControlSurfaceGroup)
{

    // Just use operator = code
    
    *this = ControlSurfaceGroup;

}

/*##############################################################################
#                                                                              #
#                       CONTROL_SURFACE_GROUP SizeList                         #
#                                                                              #
##############################################################################*/

void CONTROL_SURFACE_GROUP::SizeList(int NumberOfControlSurfaces)
{

    int i;
    
    NumberOfControlSurfaces_ = NumberOfControlSurfaces;

    ControlSurface_Name_ = new char*[NumberOfControlSurfaces + 1];
    
    for ( i = 1 ; i <= NumberOfControlSurfaces_ ; i++ ) {
       
       ControlSurface_Name_[i] = new char[2000];
       
    }
    
    // Control surface deflection sign
    
    ControlSurface_DeflectionDirection_ = new double[NumberOfControlSurfaces + 1];

}

/*##############################################################################
#                                                                              #
#                    CONTROL_SURFACE_GROUP operator=                           #
#                                                                              #
##############################################################################*/

CONTROL_SURFACE_GROUP& CONTROL_SURFACE_GROUP::operator=(const CONTROL_SURFACE_GROUP &ControlSurface)
{

    int i;
    
    NumberOfControlSurfaces_ = ControlSurface.NumberOfControlSurfaces_;
    
    SizeList(NumberOfControlSurfaces_);
    
    for ( i = 1 ; i <= NumberOfControlSurfaces_ ; i++ ) {
       
       printf(ControlSurface_Name_[i],"%s",ControlSurface.ControlSurface_Name_[i]);
       
       ControlSurface_DeflectionDirection_[i] = ControlSurface.ControlSurface_DeflectionDirection_[i];

    }
    
    return *this;

}

/*##############################################################################
#                                                                              #
#                          CONTROL_SURFACE_GROUP destructor                    #
#                                                                              #
##############################################################################*/

CONTROL_SURFACE_GROUP::~CONTROL_SURFACE_GROUP(void)
{

    int i;
    
    for ( i = 1 ; i <= NumberOfControlSurfaces_ ; i++ ) {    
       
       if ( ControlSurface_Name_[i] != NULL ) delete [] ControlSurface_Name_[i];
       
       ControlSurface_Name_[i] = NULL;
       
    }
    
    if ( ControlSurface_Name_ != NULL ) delete [] ControlSurface_Name_;

    if ( ControlSurface_DeflectionDirection_ != NULL ) delete [] ControlSurface_DeflectionDirection_; 
    
    ControlSurface_Name_ = NULL;
    
    ControlSurface_DeflectionDirection_ = NULL;
            
    NumberOfControlSurfaces_= 0;            

}

#include "END_NAME_SPACE.H"




