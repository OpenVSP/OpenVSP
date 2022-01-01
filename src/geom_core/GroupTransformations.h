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
#include "MaterialMgr.h"
#include <vector>

class Vehicle;

class GroupTransformations : ParmContainer
{
public:
    // Default Constructor
    GroupTransformations();

    // Destructor
    virtual ~GroupTransformations();

    // Custom handling of what to do when a parameter has changed
    virtual void ParmChanged( Parm * parm, int type );

    // Method to update
    virtual void Update();

    // Method used reset the transformations in preparation for operating on a new group
    virtual void ReInitialize();

    // Resets all values
    virtual void Reset();

    // Accepts all values
    virtual void Accept();

    // Getters and setters
    virtual vec3d GetColor() { return m_GroupColor; }
    virtual void SetColor( const vec3d &color );
    virtual Material* GetMaterial() { return &m_GroupMaterial; }
    virtual void SetMaterial( const Material &material );

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
    // Helper method that will set all the parameters back to default values
    virtual void ResetParameters();

    // Stores original group variable values
    vector< vector< double > > m_oldVarVals;

    // Material properties
    Material m_GroupMaterial;
    vec3d m_GroupColor;

private:
    // Helper to force rotations to be between +/- 180
    static double MakeValidRotation( const double &rotation );
};

#endif
