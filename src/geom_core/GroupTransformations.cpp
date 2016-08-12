//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//
//
//////////////////////////////////////////////////////////////////////

#include "Vehicle.h"
#include "Geom.h"
#include "GroupTransformations.h"
#include "Parm.h"

// ==== Constructor ==== //
GroupTransformations::GroupTransformations()
{
    // Initialize the vehicle pointer to null
    m_Vehicle = NULL;

    // Initialize paramters
    m_GroupXLoc.Init( "Group_XLoc", "Group", this, 0, -1e12, 1e12 );
    m_GroupYLoc.Init( "Group_YLoc", "Group", this, 0, -1e12, 1e12 );
    m_GroupZLoc.Init( "Group_ZLoc", "Group", this, 0, -1e12, 1e12 );
    m_GroupXRot.Init( "Group_XRot", "Group", this, 0, -180.0, 180.0 );
    m_GroupYRot.Init( "Group_YRot", "Group", this, 0, -180.0, 180.0 );
    m_GroupZRot.Init( "Group_ZRot", "Group", this, 0, -180.0, 180.0 );
    m_GroupScale.Init( "Group_Scale", "Group", this, 1, 1.0e-3, 1.0e3 );
    m_scaleGroupTranslations.Init( "ScaleGroupTranslations", "Group", this, true, false, true );
}

GroupTransformations::~GroupTransformations()
{
}

// ==== Initialization function which should be called ====//
// ==== before using a GroupTransformations object     ====//
void GroupTransformations::Init( Vehicle * vehicle )
{
    assert( vehicle );
    m_Vehicle = vehicle;

    // Make the vehicle the parent of this container
    SetParentContainer( m_Vehicle->GetID() );
}

// ==== Handle a changed paramter ==== //
void GroupTransformations::ParmChanged( Parm * parm, int type )
{
    // Check if this was not set from a device
    if ( type == Parm::SET )
    {
        m_LateUpdateFlag = true;
        return;
    }

    // Update
    Update();

    // Let vehicle know about the change
    m_Vehicle->ParmChanged( parm, type );
}

// ==== Applys transformations to active geoms ==== //
void GroupTransformations::Update()
{
    // Turn off the late update flag
    m_LateUpdateFlag = false;

    // Get the active Geoms
    vector< string > activeGroup = m_Vehicle->GetActiveGeomVec();

    // m_oldVarVals should have the same number of rows as there are
    // active geoms
    assert( m_oldVarVals.size() == activeGroup.size() );

    for ( int i = 0; i < activeGroup.size(); i++ )
    {
        Geom* thisGeom = m_Vehicle->FindGeom( activeGroup[i] );
        string parent_id = thisGeom->GetParentID();

        // Set the geom to ignore the abs/rel flag because
        // all of these group translations are going to be applied
        // to the relative property
        thisGeom->SetIgnoreAbsFlag( true );

        bool parent_in_group = std::find( activeGroup.begin(), activeGroup.end(), parent_id ) != activeGroup.end();

        // Create local variables to keep track of translation deltas and new x,y,z locations
        double delta_x = 0.0;
        double delta_y = 0.0;
        double delta_z = 0.0;
        double new_x = 0.0;
        double new_y = 0.0;
        double new_z = 0.0;
        double delta_xrot = 0.0;
        double delta_yrot = 0.0;
        double delta_zrot = 0.0;

        // Only apply transform (except scale) if either the geom has no active parent or its coordinate system
        // is not relative to another geometry's coordinate system
        if ( parent_in_group == false || thisGeom->m_TransAttachFlag.Get() == GeomXForm::ATTACH_TRANS_NONE )
        {
            delta_x = m_GroupXLoc.Get();
            delta_y = m_GroupYLoc.Get();
            delta_z = m_GroupZLoc.Get();
        }

        // Only apply rotation if either the geom has no active parent or its
        // rotation coordinate system is not relative to another geometry's rotational coordiante system
        if ( !parent_in_group || thisGeom->m_RotAttachFlag.Get() == GeomXForm::ATTACH_ROT_NONE )
        {
            delta_xrot = m_GroupXRot.Get();
            delta_yrot = m_GroupYRot.Get();
            delta_zrot = m_GroupZRot.Get();
        }

        // Apply the deltas to stored orignal x,y,z
        new_x = m_oldVarVals[i][0] + delta_x;
        new_y = m_oldVarVals[i][1] + delta_y;
        new_z = m_oldVarVals[i][2] + delta_z;

        double scale = m_GroupScale.Get();
        // Apply scaling to the translations if requested
        if ( m_scaleGroupTranslations.Get() )
        {
            new_x *= scale;
            new_y *= scale;
            new_z *= scale;
        }

        // Apply scaling to geom (not sure what makes most sense if the original scale
        // is not 1; for now will apply group scale on top of any current scaling)
        thisGeom->m_Scale = m_oldVarVals[i][6] * scale;

        // Set the new translations
        thisGeom->m_XRelLoc = new_x;
        thisGeom->m_YRelLoc = new_y;
        thisGeom->m_ZRelLoc = new_z;

        // Set the new rotations
        thisGeom->m_XRelRot = m_oldVarVals[i][3] + delta_xrot;
        thisGeom->m_YRelRot = m_oldVarVals[i][4] + delta_yrot;
        thisGeom->m_ZRelRot = m_oldVarVals[i][5] + delta_zrot;
    }

    // Only update geoms that do not have parent's in the group
    for ( int i = 0; i < activeGroup.size(); i++ )
    {
        Geom* thisGeom = m_Vehicle->FindGeom( activeGroup[i] );
        string parent_id = thisGeom->GetParentID();

        bool parent_in_group = std::find( activeGroup.begin(), activeGroup.end(), parent_id ) != activeGroup.end();

        if ( !parent_in_group )
            thisGeom->Update();
    }

    // Loop over all of the active geom's and set the ignore absolute transformation flag to false
    // to restore normal transformation behavior
    for ( int i = 0; i < activeGroup.size(); i++ )
    {
        Geom* thisGeom = m_Vehicle->FindGeom( activeGroup[i] );
        thisGeom->SetIgnoreAbsFlag( false );
    }
}

// ====  Method to store the current transformation values of the active geoms ==== //
void GroupTransformations::ReInitialize()
{
    vector< string > activeGroup = m_Vehicle->GetActiveGeomVec();
    m_oldVarVals.resize( activeGroup.size() );

    for ( int i = 0; i < activeGroup.size(); i++ )
    {
        Geom* geom = m_Vehicle->FindGeom( activeGroup[i] );
        m_oldVarVals[i].resize( 7 ); // 3 * translate + 3 * rotate + 1 * scale

        m_oldVarVals[i][0] = geom->m_XRelLoc.Get();
        m_oldVarVals[i][1] = geom->m_YRelLoc.Get();
        m_oldVarVals[i][2] = geom->m_ZRelLoc.Get();
        m_oldVarVals[i][3] = geom->m_XRelRot.Get();
        m_oldVarVals[i][4] = geom->m_YRelRot.Get();
        m_oldVarVals[i][5] = geom->m_ZRelRot.Get();
        m_oldVarVals[i][6] = geom->m_Scale.Get();
    }

    ResetParmeters();

    // Turn off the late update flag, everything should be up to date
    m_LateUpdateFlag = false;
}

// ==== Calls the reset scale method on all active geoms ==== //
void GroupTransformations::Reset()
{
    ResetParmeters();

    // Update the active geoms with the new values
    Update();

    // Trigger parm changed at the vehicle level
    m_Vehicle->ParmChanged( &m_GroupScale, Parm::SET );
}

// ==== Calls the accept scale method on all active geoms === //
void GroupTransformations::Accept()
{
        // Get the active geoms
    vector< string > activeGroup = m_Vehicle->GetActiveGeomVec();

    // m_oldVarVals should be the same size as the activeGroup
    assert( m_oldVarVals.size() == activeGroup.size() );

    for ( int i = 0; i < activeGroup.size(); i++ )
    {
        Geom* thisGeom = m_Vehicle->FindGeom( activeGroup[i] );
        thisGeom->AcceptScale();

        m_oldVarVals[i][0] = thisGeom->m_XRelLoc.Get();
        m_oldVarVals[i][1] = thisGeom->m_YRelLoc.Get();
        m_oldVarVals[i][2] = thisGeom->m_ZRelLoc.Get();
        m_oldVarVals[i][3] = thisGeom->m_XRelRot.Get();
        m_oldVarVals[i][4] = thisGeom->m_YRelRot.Get();
        m_oldVarVals[i][5] = thisGeom->m_ZRelRot.Get();
        m_oldVarVals[i][6] = thisGeom->m_Scale.Get();
    }

    ResetParmeters();

    m_LateUpdateFlag = false;

    // Trigger parm changed at the vehicle level
    m_Vehicle->ParmChanged( &m_GroupScale, Parm::SET );
}

// ==== Helper method that will set all the parameters back to default values ==== //
void GroupTransformations::ResetParmeters()
{
    // Set the values
    m_GroupXLoc.Set( 0 );
    m_GroupYLoc.Set( 0 );
    m_GroupZLoc.Set( 0 );
    m_GroupXRot.Set( 0 );
    m_GroupYRot.Set( 0 );
    m_GroupZRot.Set( 0 );
    m_GroupScale.Set( 1 );
}
