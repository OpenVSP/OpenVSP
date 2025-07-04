#include "GlobalWakeLoop.H"

#include "START_NAME_SPACE.H"

/*##############################################################################
#                                                                              #
#                          GLOBAL_WAKE_LOOP contructor                         #
#                                                                              #
##############################################################################*/

GLOBAL_WAKE_LOOP::GLOBAL_WAKE_LOOP(void)
{

    GlobalLoop_   = 0;

    GlobalTrailingEdge_ = 0;

    KuttaStallEquationNumber_ = 0;
    
    VortexStretchingRatioEquationNumber_ = 0;

    StallFactor_ = 1.;
    
    dStallFactor_ = 0.;

    VortexStretchingRatio_ = 1.;

    dVortexStretchingRatio_ = 0.;
    
}

/*##############################################################################
#                                                                              #
#                          GLOBAL_WAKE_LOOP Copy                               #
#                                                                              #
##############################################################################*/

GLOBAL_WAKE_LOOP::GLOBAL_WAKE_LOOP(const GLOBAL_WAKE_LOOP &GlobalWakeLoop)
{

    // Just * use the operator = code

    *this = GlobalWakeLoop;
     
}


/*##############################################################################
#                                                                              #
#                           GLOBAL_WAKE_LOOP operator=                         #
#                                                                              #
##############################################################################*/

GLOBAL_WAKE_LOOP& GLOBAL_WAKE_LOOP::operator=(const GLOBAL_WAKE_LOOP &GlobalWakeLoop)
{


    GlobalLoop_ = GlobalWakeLoop.GlobalLoop_;

    GlobalTrailingEdge_ = GlobalWakeLoop.GlobalTrailingEdge_;

    KuttaStallEquationNumber_ = GlobalWakeLoop.KuttaStallEquationNumber_;
    
    VortexStretchingRatioEquationNumber_ = GlobalWakeLoop.VortexStretchingRatioEquationNumber_;

    StallFactor_ = GlobalWakeLoop.StallFactor_;

    dStallFactor_ = GlobalWakeLoop.dStallFactor_;

    VortexStretchingRatio_ = GlobalWakeLoop.VortexStretchingRatio_;

    dVortexStretchingRatio_ = GlobalWakeLoop.dVortexStretchingRatio_;
        
    return *this;

}

/*##############################################################################
#                                                                              #
#                        GLOBAL_WAKE_LOOP destructor                           #
#                                                                              #
##############################################################################*/

GLOBAL_WAKE_LOOP::~GLOBAL_WAKE_LOOP(void)
{

    // Nothing to do...

}

#include "END_NAME_SPACE.H"















