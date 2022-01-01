//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//
//
//////////////////////////////////////////////////////////////////////

#include "Vehicle.h"

// ==== Constructor ==== //
GroupTransformations::GroupTransformations()
{
    // Initialize parameters
    m_GroupXLoc.Init( "Group_XLoc", "GroupXForm", this, 0, -1e12, 1e12 );
    m_GroupYLoc.Init( "Group_YLoc", "GroupXForm", this, 0, -1e12, 1e12 );
    m_GroupZLoc.Init( "Group_ZLoc", "GroupXForm", this, 0, -1e12, 1e12 );
    m_GroupXRot.Init( "Group_XRot", "GroupXForm", this, 0, -360.0, 360.0 );
    m_GroupYRot.Init( "Group_YRot", "GroupXForm", this, 0, -360.0, 360.0 );
    m_GroupZRot.Init( "Group_ZRot", "GroupXForm", this, 0, -360.0, 360.0 );
    m_GroupScale.Init( "Group_Scale", "GroupXForm", this, 1, 1.0e-3, 1.0e3 );
    m_scaleGroupTranslations.Init( "ScaleGroupTranslations", "GroupXForm", this, true, false, true );

    // Initialize Material and Color
    m_GroupMaterial.SetMaterialToDefault();
    m_GroupColor = vec3d( 0, 0, 255 );
}

GroupTransformations::~GroupTransformations()
{
}

// ==== Handle a changed parameter ==== //
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
    Vehicle* veh = VehicleMgr.GetVehicle();

    veh->ParmChanged( parm, type );
}

// ==== Applies transformations to active geoms ==== //
void GroupTransformations::Update()
{
    // Turn off the late update flag
    m_LateUpdateFlag = false;

    Vehicle* veh = VehicleMgr.GetVehicle();
    // Get the active Geoms
    vector< string > activeGroup = veh->GetActiveGeomVec();

    // m_oldVarVals should have the same number of rows as there are
    // active geoms
    assert( m_oldVarVals.size() == activeGroup.size() );

    for ( int i = 0; i < activeGroup.size(); i++ )
    {
        Geom* thisGeom = veh->FindGeom( activeGroup[i] );
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
        double new_xrot = 0.0;
        double new_yrot = 0.0;
        double new_zrot = 0.0;

        // Only apply transform (except scale) if either the geom has no active parent or its coordinate system
        // is not relative to another geometry's coordinate system
        if ( ! parent_in_group || ( thisGeom->m_TransAttachFlag.Get() == vsp::ATTACH_TRANS_NONE && !thisGeom->IsParentJoint() ) )
        {
            delta_x = m_GroupXLoc.Get();
            delta_y = m_GroupYLoc.Get();
            delta_z = m_GroupZLoc.Get();
        }

        // Only apply rotation if either the geom has no active parent or its
        // rotation coordinate system is not relative to another geometry's rotational coordinate system
        if ( !parent_in_group || ( thisGeom->m_RotAttachFlag.Get() == vsp::ATTACH_ROT_NONE && !thisGeom->IsParentJoint() ))
        {
            delta_xrot = m_GroupXRot.Get();
            delta_yrot = m_GroupYRot.Get();
            delta_zrot = m_GroupZRot.Get();
        }

        // Apply the deltas to stored original x,y,z
        new_x = m_oldVarVals[i][0] + delta_x;
        new_y = m_oldVarVals[i][1] + delta_y;
        new_z = m_oldVarVals[i][2] + delta_z;

        // Apply the delta to the stored original x,y,z rotations
        new_xrot = MakeValidRotation( m_oldVarVals[i][3] + delta_xrot );
        new_yrot = MakeValidRotation( m_oldVarVals[i][4] + delta_yrot );
        new_zrot = MakeValidRotation( m_oldVarVals[i][5] + delta_zrot );

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
        thisGeom->m_XRelRot = new_xrot;
        thisGeom->m_YRelRot = new_yrot;
        thisGeom->m_ZRelRot = new_zrot;
    }

    // Only update geoms that do not have parent's in the group
    for ( int i = 0; i < activeGroup.size(); i++ )
    {
        Geom* thisGeom = veh->FindGeom( activeGroup[i] );
        string parent_id = thisGeom->GetParentID();

        bool parent_in_group = std::find( activeGroup.begin(), activeGroup.end(), parent_id ) != activeGroup.end();

        if ( !parent_in_group )
            thisGeom->Update();
    }

    // Loop over all of the active geom's and set the ignore absolute transformation flag to false
    // to restore normal transformation behavior
    for ( int i = 0; i < activeGroup.size(); i++ )
    {
        Geom* thisGeom = veh->FindGeom( activeGroup[i] );
        thisGeom->SetIgnoreAbsFlag( false );
    }
}

// ====  Method to store the current transformation values of the active geoms ==== //
void GroupTransformations::ReInitialize()
{
    Vehicle* veh = VehicleMgr.GetVehicle();
    vector< string > activeGroup = veh->GetActiveGeomVec();
    m_oldVarVals.resize( activeGroup.size() );

    for ( int i = 0; i < activeGroup.size(); i++ )
    {
        Geom* geom = veh->FindGeom( activeGroup[i] );
        m_oldVarVals[i].resize( 7 ); // 3 * translate + 3 * rotate + 1 * scale

        m_oldVarVals[i][0] = geom->m_XRelLoc.Get();
        m_oldVarVals[i][1] = geom->m_YRelLoc.Get();
        m_oldVarVals[i][2] = geom->m_ZRelLoc.Get();
        m_oldVarVals[i][3] = geom->m_XRelRot.Get();
        m_oldVarVals[i][4] = geom->m_YRelRot.Get();
        m_oldVarVals[i][5] = geom->m_ZRelRot.Get();
        m_oldVarVals[i][6] = geom->m_Scale.Get();
    }

    ResetParameters();

    // Set Material and Color to defaults
    m_GroupMaterial.SetMaterialToDefault();
    m_GroupColor = vec3d( 0, 0, 255 );

    // Turn off the late update flag, everything should be up to date
    m_LateUpdateFlag = false;
}

// ==== Calls the reset scale method on all active geoms ==== //
void GroupTransformations::Reset()
{
    ResetParameters();

    // Update the active geoms with the new values
    Update();
}

// ==== Calls the accept scale method on all active geoms === //
void GroupTransformations::Accept()
{
    Vehicle* veh = VehicleMgr.GetVehicle();
    // Get the active geoms
    vector< string > activeGroup = veh->GetActiveGeomVec();

    // m_oldVarVals should be the same size as the activeGroup
    assert( m_oldVarVals.size() == activeGroup.size() );

    for ( int i = 0; i < activeGroup.size(); i++ )
    {
        Geom* thisGeom = veh->FindGeom( activeGroup[i] );
        thisGeom->AcceptScale();

        m_oldVarVals[i][0] = thisGeom->m_XRelLoc.Get();
        m_oldVarVals[i][1] = thisGeom->m_YRelLoc.Get();
        m_oldVarVals[i][2] = thisGeom->m_ZRelLoc.Get();
        m_oldVarVals[i][3] = thisGeom->m_XRelRot.Get();
        m_oldVarVals[i][4] = thisGeom->m_YRelRot.Get();
        m_oldVarVals[i][5] = thisGeom->m_ZRelRot.Get();
        m_oldVarVals[i][6] = thisGeom->m_Scale.Get();
    }

    ResetParameters();

    m_LateUpdateFlag = false;
}

// ==== Helper method that will set all the parameters back to default values ==== //
void GroupTransformations::ResetParameters()
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

// ==== Sets all of the active Geom's material to the specified material
void GroupTransformations::SetMaterial( const Material & material )
{
    Vehicle* veh = VehicleMgr.GetVehicle();

    // Set the group material to the input material
    m_GroupMaterial = Material( material );

    // Get the active geoms
    vector< string > activeGroup = veh->GetActiveGeomVec();
    for ( int i = 0; i < activeGroup.size(); i++ )
    {
        Geom* geom = veh->FindGeom( activeGroup[i] );

        // Assign the material values to each geom
        geom->SetMaterial( m_GroupMaterial.m_Name, m_GroupMaterial.m_Ambi,
            m_GroupMaterial.m_Diff, m_GroupMaterial.m_Spec, m_GroupMaterial.m_Emis,
            m_GroupMaterial.m_Shininess );
    }
}

// === Sets all of the active Geom's color to the specified color
void GroupTransformations::SetColor( const vec3d & color )
{
    Vehicle* veh = VehicleMgr.GetVehicle();

    // Store the group color
    m_GroupColor = vec3d( color );

    // Get the active geoms
    vector< string > activeGroup = veh->GetActiveGeomVec();
    for ( int i = 0; i < activeGroup.size(); i++ )
    {
        Geom* geom = veh->FindGeom( activeGroup[i] );

        // Set the geom's color
        geom->SetColor( color.x(), color.y(), color.z() );
    }
}
// ==== Helper to force rotations to be between +/- 180 ==== //
double GroupTransformations::MakeValidRotation( const double &rotation )
{
    // get the angle to be between 0 and 360
    double angle = std::abs( rotation ) - floor( std::abs( rotation / 360.0 ) )*360.0;
    if ( rotation < 0 ) angle *= -1.0;

    // convert the angle to be between +/- 180
    if ( angle > 180.0 )
    {
        angle -= 360.0;
    }
    else if ( angle < -180.0 )
    {
        angle += 360.0;
    }

    return angle;
}
