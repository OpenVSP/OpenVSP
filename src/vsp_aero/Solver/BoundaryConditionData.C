//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//
//////////////////////////////////////////////////////////////////////

#include "BoundaryConditionData.H"

#include "START_NAME_SPACE.H"

/*##############################################################################
#                                                                              #
#                   BOUNDARY_CONDITION_DATA Constructor                        #
#                                                                              #
##############################################################################*/

BOUNDARY_CONDITION_DATA::BOUNDARY_CONDITION_DATA(void)
{

    HasHightLiftSlat_ = 0;
    
    HighLiftSlatPercentage_ = 0.;
    HighLiftSlatMachDeploy_ = 0.;

}

/*##############################################################################
#                                                                              #
#                  BOUNDARY_CONDITION_DATA Destructor                          #
#                                                                              #
##############################################################################*/

BOUNDARY_CONDITION_DATA::~BOUNDARY_CONDITION_DATA(void)
{
       
    // Nothing to do

}

/*##############################################################################
#                                                                              #
#                    BOUNDARY_CONDITION_DATA Copy                              #
#                                                                              #
##############################################################################*/

BOUNDARY_CONDITION_DATA::BOUNDARY_CONDITION_DATA(const BOUNDARY_CONDITION_DATA &BoundaryConditionData)
{

    // Use operator =

    (*this)= BoundaryConditionData;

}

/*##############################################################################
#                                                                              #
#                  BOUNDARY_CONDITION_DATA Operator =                          #
#                                                                              #
##############################################################################*/

BOUNDARY_CONDITION_DATA &BOUNDARY_CONDITION_DATA::operator=(const BOUNDARY_CONDITION_DATA &BoundaryConditionData)
{

    HasHightLiftSlat_ = BoundaryConditionData.HasHightLiftSlat_;
    
    HighLiftSlatPercentage_ = BoundaryConditionData.HighLiftSlatPercentage_;
    HighLiftSlatMachDeploy_ = BoundaryConditionData.HighLiftSlatMachDeploy_;
        
    return *this;

}

#include "END_NAME_SPACE.H"


