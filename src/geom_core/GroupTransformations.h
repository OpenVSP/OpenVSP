//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//

// GroupTransformations.h: Holds parameters for performaing group transformations
// 
//
//////////////////////////////////////////////////////////////////////
#if(!defined(VSPGROUP_TRANSFORMATIONS_INCLUDED_))
#define VSPGROUP_TRANSFORMATIONS_INCLUDED_

#include "Parm.h"
#include "ParmContainer.h"
#include <vector>

class Vehicle;

class GroupTransformations : ParmContainer
{
public:
    // Default Constructor
    GroupTransformations();
    
    // Destructor
    virtual ~GroupTransformations();

    // Custom handling of what to do when a paramter has changed
    virtual void ParmChanged( Parm * parm, int type );

    // Method to update
    virtual void Update();

    // Method used reset the transformations in prepartion for operating on a new group
    virtual void Reset();

    // Initialization function
    virtual void Init( Vehicle* vehicle );

    // Translations Parms
    Parm m_GroupXLoc;
    Parm m_GroupYLoc;
    Parm m_GroupZLoc;

    // Rotation Parms
    Parm m_GroupXRot;
    Parm m_GroupYRot;
    Parm m_GroupZRot;

    // Scale Parms
    Parm m_GroupScale;
    BoolParm m_scaleGroupTranslations;

protected:
    // Stores original group variable values
    vector< vector< double > > m_oldVarVals;

    // Pointer back to the vehicle that owns this class
    Vehicle* m_Vehicle;
};

#endif
