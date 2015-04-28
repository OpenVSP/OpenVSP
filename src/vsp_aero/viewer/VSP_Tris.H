//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//
//////////////////////////////////////////////////////////////////////

#ifndef VSP_TRIS_H
#define VSP_TRIS_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <assert.h>
#include "utils.H"

// Definition of the VSP_TRIS class

class VSP_TRIS {

private:

    // Tris based data

    int Node1_;
    int Node2_;
    int Node3_;

    int Edge1_;
    int Edge2_;
    int Edge3_;
    
    int Edge1IsUpWind_;
    int Edge2IsUpWind_;
    int Edge3IsUpWind_;

    int SurfaceID_;
    int IsTrailingEdgeTri_;
    int IsLeadingEdgeTri_;
    int VortexLoop_;
    
    int SpanStation_;

    double Area_;

    // Triangle Normal
    
    double Nx_;
    double Ny_;
    double Nz_; 
    
    // Local Camber Line Normal
    
    double NxCamber_;
    double NyCamber_;
    double NzCamber_;
    
    // Centroid
    
    double Xc_;
    double Yc_;
    double Zc_;
    
    // Upwind edge weights
    
    double Edge1UpwindWeight_;
    double Edge2UpwindWeight_;
    double Edge3UpwindWeight_;
    
    // Solution data
    
    double Gamma_;
    double U_;
    double V_;
    double W_;
    double dCp_;
    
    // Agglomeration data
    
    int CoarseGridLoop_;
    int FineGridLoop_;
    
    BBOX BoundBox_;

public:

    // Constructor, Destructor, Copy

    VSP_TRIS(void);
   ~VSP_TRIS(void);
    VSP_TRIS(const VSP_TRIS &VSPTri);
    VSP_TRIS& operator=(const VSP_TRIS &VSPTri);

    // Accesss to tri data

    int &Node1(void) { return Node1_; };
    int &Node2(void) { return Node2_; };
    int &Node3(void) { return Node3_; };

    int &Edge1(void) { return Edge1_; };
    int &Edge2(void) { return Edge2_; };
    int &Edge3(void) { return Edge3_; };

    int &Edge1IsUpWind(void) { return Edge1IsUpWind_; };
    int &Edge2IsUpWind(void) { return Edge2IsUpWind_; };
    int &Edge3IsUpWind(void) { return Edge3IsUpWind_; };

    double &Edge1UpwindWeight(void) { return Edge1UpwindWeight_; };
    double &Edge2UpwindWeight(void) { return Edge2UpwindWeight_; };
    double &Edge3UpwindWeight(void) { return Edge3UpwindWeight_; };
    
    int &SurfaceID(void) { return SurfaceID_; };
    int &IsTrailingEdgeTri(void) { return IsTrailingEdgeTri_; };
    int &IsLeadingEdgeTri(void) { return IsLeadingEdgeTri_; };
    int &VortexLoop(void) { return VortexLoop_; };
    
    int &SpanStation(void) { return SpanStation_; };
    
    double &Area(void) { return Area_; };

    double &Nx(void) { return Nx_; };
    double &Ny(void) { return Ny_; };
    double &Nz(void) { return Nz_; };

    double &NxCamber(void) { return NxCamber_; };
    double &NyCamber(void) { return NyCamber_; };
    double &NzCamber(void) { return NzCamber_; };
    
    double &Xc(void) { return Xc_; };
    double &Yc(void) { return Yc_; };
    double &Zc(void) { return Zc_; };
    
    double &Gamma(void) { return Gamma_; };
    double &U(void) { return U_; };
    double &V(void) { return V_; };
    double &W(void) { return W_; };
    double &dCp(void) { return dCp_; };
    
    int &CoarseGridLoop(void) { return CoarseGridLoop_; };
    int &FineGridLoop(void) { return FineGridLoop_; };
    
    BBOX &BoundBox(void) { return BoundBox_; };
    
};

#endif
