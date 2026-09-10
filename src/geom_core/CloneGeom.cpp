//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//
//
//////////////////////////////////////////////////////////////////////

#include "CloneGeom.h"
#include "Vehicle.h"
#include "ParmMgr.h"
#include "IDMgr.h"
#include "StlHelper.h"

//==== Constructor ====//
CloneGeom::CloneGeom( Vehicle* vehicle_ptr ) : Geom( vehicle_ptr )
{
    m_Name = "CloneGeom";
    m_Type.m_Name = "Clone";
    m_Type.m_Type = CLONE_GEOM_TYPE;

    m_OriginalID = "NONE";

    // Tessellation comes from the original.
    m_TessU.Deactivate();
    m_TessW.Deactivate();
}

//==== Destructor ====//
CloneGeom::~CloneGeom()
{

}

Geom* CloneGeom::GetOriginalGeom() const
{
    if ( m_OriginalID == "NONE" || m_OriginalID.empty() || m_OriginalID == GetID() )
    {
        return nullptr;
    }
    return m_Vehicle->FindGeom( m_OriginalID );
}

void CloneGeom::UpdateMainBBox()
{
    Geom* original_geom = GetOriginalGeom();
    if ( !original_geom )
    {
        Geom::UpdateMainBBox();
        return;
    }

    original_geom->GetMainBBoxes( m_MainBBox, m_ScaleIndependentMainBBox );
}

// Answered by the original; on the Clone, this Geom's origin is the point included.
bool CloneGeom::PlacedBBoxIncludesOrigin() const
{
    Geom* original_geom = GetOriginalGeom();
    if ( !original_geom )
    {
        return Geom::PlacedBBoxIncludesOrigin();
    }

    return original_geom->PlacedBBoxIncludesOrigin();
}

// Whether following the chain of originals from id arrives back at this Geom.
bool CloneGeom::IsCloneAncestor( const string &id ) const
{
    string walk = id;

    // Guards against a ring from a file that does not pass through this Geom.
    set < string > visited;

    while ( walk != "NONE" && !walk.empty() )
    {
        if ( walk == GetID() )
        {
            return true;
        }

        if ( !visited.insert( walk ).second )
        {
            return false;
        }

        CloneGeom* clone_ptr = dynamic_cast< CloneGeom* >( m_Vehicle->FindGeom( walk ) );
        if ( !clone_ptr )
        {
            return false;
        }

        walk = clone_ptr->GetOriginalID();
    }

    return false;
}

// Safe to call before the original exists; it is looked up by ID.
void CloneGeom::ResolveOriginal()
{
    // Never chosen: take the parent as the original, if there is one.
    if ( m_OriginalID == "NONE" )
    {
        if ( m_ParentID != "NONE" && !m_ParentID.empty() )
        {
            SetOriginalID( m_ParentID );
        }
        return;
    }

    // Lost or cleared on purpose: do not take the parent.
    if ( m_OriginalID.empty() )
    {
        return;
    }

    Geom* original_geom = GetOriginalGeom();

    // A file names whatever it names.  SetOriginalID refuses a ring, but a file never went
    // through it, and a ring of clones would each be waiting on the one in front -- asking any
    // of them anything walks the ring forever.
    if ( original_geom && IsCloneAncestor( m_OriginalID ) )
    {
        original_geom = nullptr;
    }

    if ( !original_geom )
    {
        // Deleted, pasted without its original, or circular.  Go empty rather than silently
        // switch to copying the parent.
        m_OriginalID.clear();

        // Whatever was copied came from a Geom this one no longer follows.
        m_XFormDirty = true;
        m_SurfDirty = true;
        m_AppearanceDirty = true;
        m_NameDirty = true;
        m_SubSurfDirty = true;
        return;
    }

    // Register as a step child so the original updates this Geom.
    vector < string > stepchildren = original_geom->GetStepChildIDVec();
    if ( !vector_contains_val( stepchildren, GetID() ) )
    {
        original_geom->AddStepChildID( GetID() );

        m_XFormDirty = true;
        m_SurfDirty = true;
        m_AppearanceDirty = true;
        m_NameDirty = true;
        m_SubSurfDirty = true;
    }
}

void CloneGeom::UpdateSets()
{
    // First step of Update, so the original is resolved before anything uses it.
    ResolveOriginal();

    // Always copied: the Clone shows whichever tessellation the original rebuilt.
    Geom* display_from = GetOriginalGeom();
    if ( display_from )
    {
        m_GuiDraw.SetDisplayType( display_from->m_GuiDraw.GetDisplayType() );
    }

    Geom::UpdateSets();  // Makes sure it has the right number of Sets, SET_ALL is true, and SHOWN/NOT_SHOWN is consistent.
}

void CloneGeom::UpdateSurf()
{
    // UpdateSets has already called ResolveOriginal.
    Geom* original_geom = GetOriginalGeom();
    if ( !original_geom )
    {
        m_MainSurfVec.clear();
        return;
    }

    original_geom->GetMainSurfVecCopy( m_MainSurfVec );
}

void CloneGeom::UpdateMainTessVec()
{
    Geom* original_geom = GetOriginalGeom();
    if ( !original_geom )
    {
        // Match the surfaces UpdateSurf emptied.
        m_MainTessVec.clear();
        m_MainFeatureTessVec.clear();
        return;
    }

    original_geom->GetMainTessVecCopy( m_MainTessVec );
    original_geom->GetMainFeatureTessVecCopy( m_MainFeatureTessVec );
}

void CloneGeom::UpdateMainDegenGeomPreview()
{
    Geom* original_geom = GetOriginalGeom();
    if ( !original_geom )
    {
        m_MainDegenGeomPreviewVec.clear();
        return;
    }

    original_geom->GetMainDegenGeomPreviewCopy( m_MainDegenGeomPreviewVec );
}

//==== Compute Rotation Center ====//
void CloneGeom::ComputeCenter()
{
    Geom* original_geom = GetOriginalGeom();
    if ( !original_geom )
    {
        return;
    }

    m_Center = original_geom->m_Center;
}

// A Clone has no cross sections and no shape of its own to put a mesh source on.
void CloneGeom::AddDefaultSources( double base_len )
{
}

//==== Encode Data Into XML Data Struct ====//
xmlNodePtr CloneGeom::EncodeXml( xmlNodePtr & node )
{
    Geom::EncodeXml( node );
    xmlNodePtr clone_node = xmlNewChild( node, NULL, BAD_CAST "CloneGeom", NULL );
    if ( clone_node )
    {
        XmlUtil::AddStringNode( clone_node, "OriginalID", m_OriginalID );
    }
    return clone_node;
}

//==== Decode Data From XML Data Struct ====//
xmlNodePtr CloneGeom::DecodeXml( xmlNodePtr & node )
{
    Geom::DecodeXml( node );

    xmlNodePtr clone_node = XmlUtil::GetNode( node, "CloneGeom", 0 );
    if ( clone_node )
    {
        // The original may not be decoded yet; the first update links it.  An empty node means
        // cleared; a missing node keeps "NONE" so the parent is taken.  Test for the node, since
        // FindString returns the default for both.
        if ( XmlUtil::GetNode( clone_node, "OriginalID", 0 ) )
        {
            m_OriginalID = IDMgr.RemapRefID( XmlUtil::FindString( clone_node, "OriginalID", string() ) );
        }
    }

    return clone_node;
}

bool CloneGeom::SetOriginalID( const string &id )
{
    // A Clone of itself has nothing to copy, and neither does a ring of Clones -- each would be
    // waiting on the one in front of it.
    if ( id == GetID() || IsCloneAncestor( id ) )
    {
        return false;
    }

    if ( id == m_OriginalID )
    {
        return true;
    }

    Geom* original_geom = GetOriginalGeom();
    if ( original_geom )
    {
        original_geom->RemoveStepChildID( GetID() );
    }

    // Empty, not "NONE", so the next update does not take the parent.
    m_OriginalID.clear();

    original_geom = m_Vehicle->FindGeom( id );
    if ( original_geom )
    {
        m_OriginalID = id;
        original_geom->AddStepChildID( GetID() );
    }

    // A different original means everything is copied afresh.
    m_XFormDirty = true;
    m_SurfDirty = true;
    m_AppearanceDirty = true;
    m_NameDirty = true;
    m_SubSurfDirty = true;

    Update();

    return true;
}
