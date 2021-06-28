#include "Vortex_Bound.H"

/*##############################################################################
#                                                                              #
#                              VORTEX_BOUND constructor                        #
#                                                                              #
##############################################################################*/

VORTEX_BOUND::VORTEX_BOUND(void)
{

    // Use init routine

    init();

}

/*##############################################################################
#                                                                              #
#                                 VORTEX_BOUND init                            #
#                                                                              #
##############################################################################*/

void VORTEX_BOUND::init(void)
{

    Verbose_ = 0;
 
    TimeAccurate_ = 0;
    
    TimeAnalysisType_ = 0;

    TimeStep_ = 0.;
    
    CurrentTimeStep_ = 0;
 
    NumberOfStartingVortices_ = 0;
    
    Gamma_ = NULL;
    
    BoundVortexList_ = NULL;
       
    CoreSize_ = 0.;
 
}

/*##############################################################################
#                                                                              #
#                                VORTEX_BOUND Copy                             #
#                                                                              #
##############################################################################*/

VORTEX_BOUND::VORTEX_BOUND(const VORTEX_BOUND &Bound_Vortex)
{

    init();

    // Just * use the operator = code

    *this = Bound_Vortex;

}

/*##############################################################################
#                                                                              #
#                               VORTEX_BOUND operator=                         #
#                                                                              #
##############################################################################*/

VORTEX_BOUND& VORTEX_BOUND::operator=(const VORTEX_BOUND &Bound_Vortex)
{

    PRINTF("VORTEX_BOUND operator= not implemented! \n");fflush(NULL);
    
    exit(1);
    
    return *this;

}

/*##############################################################################
#                                                                              #
#                               VORTEX_BOUND operator+=                        #
#                                                                              #
##############################################################################*/

VORTEX_BOUND& VORTEX_BOUND::operator+=(const VORTEX_BOUND &Bound_Vortex)
{

    int i;
    VSP_NODE NodeA, NodeB;

    // Update the bound vortices

    for ( i = 1 ; i <= NumberOfStartingVortices_ ; i++ ) {
  
       NodeA.x() = Bound_Vortex.BoundVortexList_[i].X1();
       NodeA.y() = Bound_Vortex.BoundVortexList_[i].Y1();
       NodeA.z() = Bound_Vortex.BoundVortexList_[i].Z1();
       
       NodeB.x() = Bound_Vortex.BoundVortexList_[i].X2();
       NodeB.y() = Bound_Vortex.BoundVortexList_[i].Y2();
       NodeB.z() = Bound_Vortex.BoundVortexList_[i].Z2();

       BoundVortexList_[i].Setup(NodeA, NodeB);

       BoundVortexList(i).Sigma() = Bound_Vortex.BoundVortexList_[i].Sigma();
       
       BoundVortexList(i).S() = Bound_Vortex.BoundVortexList_[i].S();
              
    }

    return *this;

}

/*##############################################################################
#                                                                              #
#                            VORTEX_BOUND destructor                           #
#                                                                              #
##############################################################################*/

VORTEX_BOUND::~VORTEX_BOUND(void)
{

    if ( BoundVortexList_ != NULL ) delete [] BoundVortexList_;
    
    if ( Gamma_ != NULL ) delete [] Gamma_;
 
}

/*##############################################################################
#                                                                              #
#                              VORTEX_BOUND Setup                              #
#                                                                              #
##############################################################################*/

void VORTEX_BOUND::Setup(VORTEX_TRAIL &Trail1, VORTEX_TRAIL &Trail2)
{

    int i;
    VSP_NODE NodeA, NodeB;

    NumberOfStartingVortices_ = Trail1.NumberOfSubVortices() + 1;

    // Fine level list of bound vortices

    BoundVortexList_ = new VSP_EDGE[NumberOfStartingVortices_ + 1];
     
    Gamma_ = new VSPAERO_DOUBLE[NumberOfStartingVortices_ + 1];

    for ( i = 1 ; i <= NumberOfStartingVortices_ ; i++ ) {

       NodeA.x() = Trail1.VortexEdge(i).X2();
       NodeA.y() = Trail1.VortexEdge(i).Y2();
       NodeA.z() = Trail1.VortexEdge(i).Z2();
       
       NodeB.x() = Trail2.VortexEdge(i).X2();
       NodeB.y() = Trail2.VortexEdge(i).Y2();
       NodeB.z() = Trail2.VortexEdge(i).Z2();

       BoundVortexList(i).Setup(NodeA, NodeB);

       BoundVortexList(i).Sigma() = 0.5*( Trail1.Sigma() + Trail2.Sigma() );
       
       BoundVortexList(i).S() = 0.5*( Trail1.S(i) + Trail2.S(i) );
       
    }
 
}

/*##############################################################################
#                                                                              #
#                         VORTEX_BOUND InducedVelocity                         #
#                                                                              #
##############################################################################*/

void VORTEX_BOUND::InducedVelocity(VSPAERO_DOUBLE xyz_p[3], VSPAERO_DOUBLE q[3])
{
   
   CoreSize_ = 0.;
   
   InducedVelocity_(xyz_p, q);

}

/*##############################################################################
#                                                                              #
#                         VORTEX_BOUND SimpleVortex                            #
#                                                                              #
##############################################################################*/

void VORTEX_BOUND::InducedVelocity(VSPAERO_DOUBLE xyz_p[3], VSPAERO_DOUBLE q[3], VSPAERO_DOUBLE CoreSize)
{
   
   CoreSize_ = CoreSize;
   
   InducedVelocity_(xyz_p, q);
   
}

/*##############################################################################
#                                                                              #
#                         VORTEX_BOUND InducedVelocity_                        #
#                                                                              #
##############################################################################*/

void VORTEX_BOUND::InducedVelocity_(VSPAERO_DOUBLE xyz_p[3], VSPAERO_DOUBLE q[3])
{
 
    int i, NumVortices;
    VSPAERO_DOUBLE dq[3];

    NumVortices = MIN(CurrentTimeStep_ , NumberOfStartingVortices_);

    // Calculate induced velocity
    
    q[0] = q[1] = q[2] = 0.;
    
    for ( i = 1 ; i <= NumVortices ; i++ ) {

       BoundVortexList_[i].InducedVelocity(xyz_p,dq,CoreSize_);

       q[0] += dq[0];
       q[1] += dq[1];
       q[2] += dq[2];

    }      
//PRINTF("q: %f %f %f \n",q[0],q[1],q[2]);    
    
// Uncomment to make unsteady wake go away... well, part of it anyway
// q[0] = q[1] = q[2] = 0.;

}

/*##############################################################################
#                                                                              #
#                        VORTEX_BOUND UpdateGamma                              #
#                                                                              #
##############################################################################*/

void VORTEX_BOUND::UpdateGamma(void)
{

    int i;
    
    for ( i = 1 ; i <= NumberOfStartingVortices_ ; i++ ) {
       
       BoundVortexList_[i].Gamma() = Gamma_[i];
       
    }
    
}

/*##############################################################################
#                                                                              #
#                    VORTEX_BOUND UpdateGeometryLocation                       #
#                                                                              #
##############################################################################*/

void VORTEX_BOUND::UpdateGeometryLocation(VORTEX_TRAIL &Trail1, VORTEX_TRAIL &Trail2)
{

    int i, NumVortices;
    VSP_NODE NodeA, NodeB;

    // Update the bound vortices
    
    NumVortices = MIN(CurrentTimeStep_, NumberOfStartingVortices_);

    for ( i = 1 ; i <= NumVortices ; i++ ) {
     
       NodeA.x() = Trail1.VortexEdge(i).X2();
       NodeA.y() = Trail1.VortexEdge(i).Y2();
       NodeA.z() = Trail1.VortexEdge(i).Z2();
       
       NodeB.x() = Trail2.VortexEdge(i).X2();
       NodeB.y() = Trail2.VortexEdge(i).Y2();
       NodeB.z() = Trail2.VortexEdge(i).Z2();

       BoundVortexList_[i].Setup(NodeA, NodeB);

       BoundVortexList(i).Sigma() = 0.5*( Trail1.Sigma() + Trail2.Sigma() );
       
       BoundVortexList(i).S() = 0.5*( Trail1.S(i) + Trail2.S(i) );
                     
    }
 
}    


