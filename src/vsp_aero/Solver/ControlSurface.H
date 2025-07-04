//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//
//////////////////////////////////////////////////////////////////////

#ifndef CONTROL_SURFACE_H
#define CONTROL_SURFACE_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <assert.h>
#include "utils.H"
#include "quat.H"
#include "CharSizes.H"

#include "START_NAME_SPACE.H"

#define LOWER_SURFACE           1
#define UPPER_SURFACE           2
#define LOWER_AND_UPPER_SURFACE 3

// Definition of the CONTROL_SURFACE class

class CONTROL_SURFACE {

private:

    // Control surface type
    
    int Type_;
    
    char TypeName_[MAX_CHAR_SIZE];

    // Control surface name
    
    char Name_[MAX_CHAR_SIZE];
    
    // Control surface short name
    
    char ShortName_[MAX_CHAR_SIZE];

    // XYZ coordinates of control surface box
    
    int NumberOfNodes_;
    
    double *XYZ_NodeList_;
    
    // List of loops on the control surface
    
    int NumberOfLoops_;
    int *LoopList_;
    
    // Hinge line
    
    double HingeNode_1_[3];
    double HingeNode_2_[3];
    double HingeVec_[3];
    
    // Deflection Angle
    
    double DeflectionAngle_;
    
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

    CONTROL_SURFACE(void);
   ~CONTROL_SURFACE(void);
    CONTROL_SURFACE(const CONTROL_SURFACE &ControlSurface);
    CONTROL_SURFACE& operator=(const CONTROL_SURFACE &ControlSurface);
    
    /** Size the loop list... this is the list of of vortex loops contained within
     * the control surface definition **/
    
    void SizeLoopList(int NumberOfLoops);
 
    /** Number of loops contained in this control surface definition **/
    
    int NumberOfLoops(void) { return NumberOfLoops_; };
    
    /** ith vortex loop contained in this control surface definition **/
    
    int &LoopList(int i) { return LoopList_[i]; };

    /** Size the node list that defines the polygon for this control surface **/
    
    void SizeNodeList(int NumberOfNodes);
    
    /** Number of nodes that define the control surface polygon **/
    
    int NumberOfNodes(void) { return NumberOfNodes_; };
    
    /** x location of i'th polygon node **/
    
    double &X(int i) { return XYZ_NodeList_[3*i-2]; };
    
    /** y location of i'th polygon node **/
    
    double &Y(int i) { return XYZ_NodeList_[3*i-1]; };

    /** z location of i'th polygon node **/
    
    double &Z(int i) { return XYZ_NodeList_[3*i  ]; };

    /** Hinge node 1 **/
    
    double &HingeNode_1(int i) { return HingeNode_1_[i]; };
    
    /** Hinge node 2 **/
    
    double &HingeNode_2(int i) { return HingeNode_2_[i]; };
    
    /** Hinge vector defined as vector from hinge node 1 to hinge node 2 **/
    
    double &HingeVec(int i) { return HingeVec_[i]; };

    /** Rotate xyz point about hinge line **/
    
    void RotateXYZPoint(double *xyz);

    /** Un-Rotate xyz point about hinge line **/
    
    void UnRotateXYZPoint(double *xyz);
    
    /** Rotate the user supplied normal vector about the hinge line by an amount DeflectionAngle_ **/
    
    void RotateNormal(double *Normal);
    
    /** Determine in a tri/loop is inside the control surface definition **/
    
    int TriInside(double *UV);
    
    /** Control surface name **/
    
    char *Name(void) { return Name_; };
    
    /** Short name given by openvsp to descriminate between control surfaces on various componets **/
    
    char *ShortName(void) { return ShortName_; };    
    
    /** Type of control surface by name **/
    
    char *TypeName(void) { return TypeName_; };

    /** Type of control surface **/
    
    int &Type(void) { return Type_; };
    
    /** Control surface deflection angle **/
    
    double &DeflectionAngle(void) { return DeflectionAngle_; };

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
    
    /** Load in the tag region file data **/
    
    void LoadFile(char *FileName, char *TagFileName);
    
};

#include "END_NAME_SPACE.H"

#endif
