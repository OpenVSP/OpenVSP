#include "WakeEdgeData.H"

#include "START_NAME_SPACE.H"

/*##############################################################################
#                                                                              #
#                          WAKE_EDGE contructor                                #
#                                                                              #
##############################################################################*/

WAKE_EDGE::WAKE_EDGE(void)
{

    GlobalTrailingEdgeLoopL_ = 0;
    
    GlobalTrailingEdgeLoopR_ = 0;
    
    GlobalTrailingEdgeEdge_ = 0;

    WakeResidualEquationNumberForX_ = 0;
    
    Direction_ = 0;
    
    dS_ = 0.;

    IsRotorWake_ = 0;

    RotorThrustVector_[0] = RotorThrustVector_[1] = RotorThrustVector_[2] = 0.;

}

/*##############################################################################
#                                                                              #
#                          WAKE_EDGE Copy                                      #
#                                                                              #
##############################################################################*/

WAKE_EDGE::WAKE_EDGE(const WAKE_EDGE &WakeEdge)
{

    // Just * use the operator = code

    *this = WakeEdge;
     
}

/*##############################################################################
#                                                                              #
#                           WAKE_EDGE operator=                                #
#                                                                              #
##############################################################################*/

WAKE_EDGE& WAKE_EDGE::operator=(const WAKE_EDGE &WakeEdge)
{
    
    GlobalTrailingEdgeLoopL_ = WakeEdge.GlobalTrailingEdgeLoopL_;
    
    GlobalTrailingEdgeLoopR_ = WakeEdge.GlobalTrailingEdgeLoopR_;
    
    GlobalTrailingEdgeEdge_ = WakeEdge.GlobalTrailingEdgeEdge_;

    WakeResidualEquationNumberForX_ = WakeEdge.WakeResidualEquationNumberForX_;
    
    Direction_ = WakeEdge.Direction_;
    
    dS_ = WakeEdge.dS_;
    
    IsRotorWake_ = WakeEdge.IsRotorWake_;

    RotorThrustVector_[0] = WakeEdge.RotorThrustVector_[0];
    RotorThrustVector_[1] = WakeEdge.RotorThrustVector_[1];
    RotorThrustVector_[2] = WakeEdge.RotorThrustVector_[2];
       
    return *this;

}

/*##############################################################################
#                                                                              #
#                        WAKE_EDGE destructor                                  #
#                                                                              #
##############################################################################*/

WAKE_EDGE::~WAKE_EDGE(void)
{

    // Nothing to do...

}

#include "END_NAME_SPACE.H"















