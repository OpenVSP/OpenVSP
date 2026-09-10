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
#include "StringUtil.h"

//==== Constructor ====//
CloneGeom::CloneGeom( Vehicle* vehicle_ptr ) : Geom( vehicle_ptr )
{
    m_Name = "CloneGeom";
    m_Type.m_Name = "Clone";
    m_Type.m_Type = CLONE_GEOM_TYPE;

    m_OriginalID = "NONE";

    // All on except the transformation, so a Clone is placed on its own.  Symmetry is on
    // because a symmetric original's main surfaces describe only half of it.
    m_CloneSets.Init( "CloneSets", "Behavior", this, true, false, true );
    m_CloneSets.SetDescript( "Flag to copy the original's Set membership" );

    m_CloneSym.Init( "CloneSym", "Behavior", this, true, false, true );
    m_CloneSym.SetDescript( "Flag to copy the original's symmetry and flip" );

    m_CloneXForm.Init( "CloneXForm", "Behavior", this, false, false, true );
    m_CloneXForm.SetDescript( "Flag to copy the original's translation and rotation" );

    m_CloneAttach.Init( "CloneAttach", "Behavior", this, true, false, true );
    m_CloneAttach.SetDescript( "Flag to copy the original's attachment" );

    m_CloneAppearance.Init( "CloneAppearance", "Behavior", this, true, false, true );
    m_CloneAppearance.SetDescript( "Flag to copy the original's wireframe color and material" );

    m_CloneNegativeVolume.Init( "CloneNegativeVolume", "Behavior", this, true, false, true );
    m_CloneNegativeVolume.SetDescript( "Flag to copy the original's negative volume flag" );

    m_CloneMassProps.Init( "CloneMassProps", "Behavior", this, true, false, true );
    m_CloneMassProps.SetDescript( "Flag to copy the original's mass properties" );

    m_CloneSubSurfs.Init( "CloneSubSurfs", "Behavior", this, true, false, true );
    m_CloneSubSurfs.SetDescript( "Flag to copy the original's subsurfaces" );

    m_AutoName.Init( "AutoName", "Behavior", this, true, false, true );
    m_AutoName.SetDescript( "Flag to name this Geom after the original with the suffix appended" );

    m_NameSuffix = "_Clone";

    // Tessellation comes from the original.
    m_TessU.Deactivate();
    m_TessW.Deactivate();

    // Keep the base class's empty surface: AddSubSurf needs one, and a file is read before the
    // first update.
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

        // The name was being written from a Geom that is gone, so it stops being written at
        // all: the switch goes off, and what is on the Geom now is the user's to keep or to
        // change.  Leaving the switch on would leave a name nobody can edit and nothing
        // maintains -- and would throw the user's replacement away the moment a new original
        // was chosen.
        m_AutoName = false;

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

    if ( !m_CloneSets() )
    {
        return;
    }

    Geom* original_geom = GetOriginalGeom();
    if ( !original_geom )
    {
        return;
    }

    CopySetFlags( original_geom, this );
}

void CloneGeom::CopySymParms( Geom* from, Geom* to )
{
    to->m_SymPlanFlag.Set( from->m_SymPlanFlag() );
    to->m_SymAxFlag.Set( from->m_SymAxFlag() );
    to->m_SymRotN.Set( from->m_SymRotN() );

    to->m_SymAncestor.Set( from->m_SymAncestor() );
    to->m_SymAncestOriginFlag.Set( from->m_SymAncestOriginFlag() );
}

void CloneGeom::CopyXFormParms( Geom* from, Geom* to )
{
    to->m_XLoc.Set( from->m_XLoc() );
    to->m_YLoc.Set( from->m_YLoc() );
    to->m_ZLoc.Set( from->m_ZLoc() );

    to->m_XRelLoc.Set( from->m_XRelLoc() );
    to->m_YRelLoc.Set( from->m_YRelLoc() );
    to->m_ZRelLoc.Set( from->m_ZRelLoc() );

    to->m_XRot.Set( from->m_XRot() );
    to->m_YRot.Set( from->m_YRot() );
    to->m_ZRot.Set( from->m_ZRot() );

    to->m_XRelRot.Set( from->m_XRelRot() );
    to->m_YRelRot.Set( from->m_YRelRot() );
    to->m_ZRelRot.Set( from->m_ZRelRot() );

    to->m_Origin.Set( from->m_Origin() );
    to->m_AbsRelFlag.Set( from->m_AbsRelFlag() );
}

void CloneGeom::CopyAttachParms( Geom* from, Geom* to )
{
    to->m_TransAttachFlag.Set( from->m_TransAttachFlag() );
    to->m_RotAttachFlag.Set( from->m_RotAttachFlag() );

    to->m_ULoc.Set( from->m_ULoc() );
    to->m_U0NLoc.Set( from->m_U0NLoc() );
    to->m_U01.Set( from->m_U01() );
    to->m_WLoc.Set( from->m_WLoc() );

    to->m_RLoc.Set( from->m_RLoc() );
    to->m_R01.Set( from->m_R01() );
    to->m_R0NLoc.Set( from->m_R0NLoc() );
    to->m_SLoc.Set( from->m_SLoc() );
    to->m_TLoc.Set( from->m_TLoc() );

    to->m_LLoc.Set( from->m_LLoc() );
    to->m_L01.Set( from->m_L01() );
    to->m_L0LenLoc.Set( from->m_L0LenLoc() );
    to->m_MLoc.Set( from->m_MLoc() );
    to->m_NLoc.Set( from->m_NLoc() );

    to->m_EtaLoc.Set( from->m_EtaLoc() );
}

void CloneGeom::CopyMassPropParms( Geom* from, Geom* to )
{
    to->m_MassPrior.Set( from->m_MassPrior() );
    to->m_Density.Set( from->m_Density() );
    to->m_MassArea.Set( from->m_MassArea() );
    to->m_ShellFlag.Set( from->m_ShellFlag() );

    to->m_PointMass.Set( from->m_PointMass() );
    to->m_CGx.Set( from->m_CGx() );
    to->m_CGy.Set( from->m_CGy() );
    to->m_CGz.Set( from->m_CGz() );
    to->m_Ixx.Set( from->m_Ixx() );
    to->m_Iyy.Set( from->m_Iyy() );
    to->m_Izz.Set( from->m_Izz() );
    to->m_Ixy.Set( from->m_Ixy() );
    to->m_Ixz.Set( from->m_Ixz() );
    to->m_Iyz.Set( from->m_Iyz() );
}

void CloneGeom::CopyNegativeVolumeParm( Geom* from, Geom* to )
{
    to->m_NegativeVolumeFlag.Set( from->m_NegativeVolumeFlag() );
}

void CloneGeom::CopySetFlags( Geom* from, Geom* to )
{
    vector < bool > fsets = from->GetSetFlags();
    vector < bool > tsets = to->GetSetFlags();

    if ( fsets.size() != tsets.size() )
    {
        return;
    }

    // NONE, ALL, SHOWN and NOT_SHOWN are the Geom's own business either way.
    for ( int i = vsp::SET_FIRST_USER; i < ( int )fsets.size(); i++ )
    {
        to->SetSetFlag( i, fsets[i] );
    }
}

void CloneGeom::CopyAppearance( Geom* from, Geom* to )
{
    to->SetColor( from->GetColor().x(), from->GetColor().y(), from->GetColor().z() );
    to->SetMaterial( *from->GetMaterial() );
}

void CloneGeom::UpdateCopyXFormParms()
{
    Geom* original_geom = GetOriginalGeom();
    if ( !original_geom )
    {
        return;
    }

    if ( m_CloneSym() )
    {
        CopySymParms( original_geom, this );
    }

    if ( m_CloneXForm() )
    {
        CopyXFormParms( original_geom, this );
    }

    if ( m_CloneAttach() )
    {
        CopyAttachParms( original_geom, this );
    }
}

void CloneGeom::DeactivateXForms()
{
    GeomXForm::DeactivateXForms();

    if ( !GetOriginalGeom() )
    {
        return;
    }

    // A Clone has no dimensions of its own to scale.
    m_Scale.Deactivate();

    // Copied values are not editable.
    if ( m_CloneXForm() )
    {
        m_XLoc.Deactivate();
        m_YLoc.Deactivate();
        m_ZLoc.Deactivate();
        m_XRelLoc.Deactivate();
        m_YRelLoc.Deactivate();
        m_ZRelLoc.Deactivate();
        m_XRot.Deactivate();
        m_YRot.Deactivate();
        m_ZRot.Deactivate();
        m_XRelRot.Deactivate();
        m_YRelRot.Deactivate();
        m_ZRelRot.Deactivate();
        m_Origin.Deactivate();
        m_AbsRelFlag.Deactivate();
    }
    else
    {
        // The base class reactivates the Abs/Rel driver but not these.
        m_Origin.Activate();
        m_AbsRelFlag.Activate();
    }

    if ( m_CloneAttach() )
    {
        m_TransAttachFlag.Deactivate();
        m_RotAttachFlag.Deactivate();
        m_ULoc.Deactivate();
        m_U0NLoc.Deactivate();
        m_U01.Deactivate();
        m_WLoc.Deactivate();
        m_RLoc.Deactivate();
        m_R01.Deactivate();
        m_R0NLoc.Deactivate();
        m_SLoc.Deactivate();
        m_TLoc.Deactivate();
        m_LLoc.Deactivate();
        m_L01.Deactivate();
        m_L0LenLoc.Deactivate();
        m_MLoc.Deactivate();
        m_NLoc.Deactivate();
        m_EtaLoc.Deactivate();
    }
    else
    {
        m_U01.Activate();
        m_R01.Activate();
        m_L01.Activate();
        m_EtaLoc.Activate();
    }

    // Deactivation persists, so reactivate what is no longer copied.
    if ( m_CloneSym() )
    {
        m_SymPlanFlag.Deactivate();
        m_SymAxFlag.Deactivate();
        m_SymRotN.Deactivate();
        m_SymAncestor.Deactivate();
        m_SymAncestOriginFlag.Deactivate();
    }
    else
    {
        m_SymPlanFlag.Activate();
        m_SymAxFlag.Activate();
        m_SymRotN.Activate();
        m_SymAncestor.Activate();
        m_SymAncestOriginFlag.Activate();
    }
}

// This Clone's flip combined, when symmetry is copied, with the original's.  Two reflections
// about the same plane cancel, so the flags combine by XOR.  Ring-safe like FollowOriginals.
int CloneGeom::GetFlipFlag() const
{
    int flag = m_FlipFlag();

    set < string > visited;
    visited.insert( GetID() );

    const CloneGeom* clone_ptr = this;
    while ( clone_ptr->m_CloneSym() )
    {
        Geom* geom_ptr = clone_ptr->GetOriginalGeom();
        if ( !geom_ptr || !visited.insert( geom_ptr->GetID() ).second )
        {
            break;
        }

        // A non-Clone answers for itself, e.g. a conformal reports its parent's flip.
        CloneGeom* next = dynamic_cast< CloneGeom* >( geom_ptr );
        if ( !next )
        {
            flag ^= geom_ptr->GetFlipFlag();
            break;
        }

        flag ^= next->m_FlipFlag();
        clone_ptr = next;
    }

    return flag;
}

void CloneGeom::UpdateCopySurfParms()
{
    Geom* original_geom = GetOriginalGeom();
    if ( !original_geom )
    {
        return;
    }

    if ( m_CloneNegativeVolume() )
    {
        CopyNegativeVolumeParm( original_geom, this );
        m_NegativeVolumeFlag.Deactivate();
    }
    else
    {
        m_NegativeVolumeFlag.Activate();
    }

    if ( m_CloneMassProps() )
    {
        CopyMassPropParms( original_geom, this );

        m_MassPrior.Deactivate();
        m_Density.Deactivate();
        m_MassArea.Deactivate();
        m_ShellFlag.Deactivate();

        m_PointMass.Deactivate();
        m_CGx.Deactivate();
        m_CGy.Deactivate();
        m_CGz.Deactivate();
        m_Ixx.Deactivate();
        m_Iyy.Deactivate();
        m_Izz.Deactivate();
        m_Ixy.Deactivate();
        m_Ixz.Deactivate();
        m_Iyz.Deactivate();
    }
    else
    {
        m_MassPrior.Activate();
        m_Density.Activate();
        m_MassArea.Activate();
        m_ShellFlag.Activate();

        m_PointMass.Activate();
        m_CGx.Activate();
        m_CGy.Activate();
        m_CGz.Activate();
        m_Ixx.Activate();
        m_Iyy.Activate();
        m_Izz.Activate();
        m_Ixy.Activate();
        m_Ixz.Activate();
        m_Iyz.Activate();
    }
}

TextureMgr* CloneGeom::GetDrawTextureMgr()
{
    Geom* original_geom = GetOriginalGeom();
    if ( original_geom && m_CloneAppearance() )
    {
        return original_geom->GetDrawTextureMgr();
    }

    // Not copying: use this Geom's own textures.
    return Geom::GetDrawTextureMgr();
}

void CloneGeom::UpdateCopyAppearance()
{
    Geom* original_geom = GetOriginalGeom();
    if ( !original_geom || !m_CloneAppearance() )
    {
        return;
    }

    CopyAppearance( original_geom, this );
}

void CloneGeom::UpdateCopyName()
{
    Geom* original_geom = GetOriginalGeom();
    if ( !original_geom || !m_AutoName() )
    {
        return;
    }

    string autoname = original_geom->GetName() + m_NameSuffix;
    if ( GetName() != autoname )
    {
        SetName( autoname );
    }
}

bool CloneGeom::NameIsAutomatic() const
{
    // The switch alone, even with no original yet: ResolveOriginal may take the parent on the
    // next update and overwrite a typed name.  Losing the original turns the switch off.
    return m_AutoName();
}

// The suffix is not a Parm, so mark the name dirty to rebuild it on the next update.
void CloneGeom::SetNameSuffix( const string &suffix )
{
    // Strip slashes as SetName does, so the built name matches and UpdateCopyName does not
    // rename on every pass.
    string clean = suffix;
    StringUtil::remove_all( clean, '/' );

    if ( m_NameSuffix == clean )
    {
        return;
    }

    m_NameSuffix = clean;
    m_NameDirty = true;
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

// When the flag is copied, skip the base pass: it would overwrite the per-surface CFD types
// that came with the surfaces.
void CloneGeom::UpdateFlags()
{
    if ( GetOriginalGeom() && m_CloneNegativeVolume() )
    {
        return;
    }

    Geom::UpdateFlags();
}

// The surface is this Clone's own; only the sampling comes from the original.
void CloneGeom::GetUWTess( const VspSurf &surf, bool capUMinSuccess, bool capUMaxSuccess, bool degen,
                           vector< double > &utess, vector< double > &vtess, const int & n_ref ) const
{
    Geom* original_geom = GetOriginalGeom();
    if ( original_geom )
    {
        original_geom->GetUWTess( surf, capUMinSuccess, capUMaxSuccess, degen, utess, vtess, n_ref );
        return;
    }

    Geom::GetUWTess( surf, capUMinSuccess, capUMaxSuccess, degen, utess, vtess, n_ref );
}

void CloneGeom::UpdateTesselate( const VspSurf &surf, bool capUMinSuccess, bool capUMaxSuccess, bool degen,
                                 vector< vector< vec3d > > &pnts, vector< vector< vec3d > > &norms,
                                 vector< vector< vec3d > > &uw_pnts, const int & n_ref ) const
{
    Geom* original_geom = GetOriginalGeom();
    if ( original_geom )
    {
        original_geom->UpdateTesselate( surf, capUMinSuccess, capUMaxSuccess, degen, pnts, norms, uw_pnts, n_ref );
        return;
    }

    Geom::UpdateTesselate( surf, capUMinSuccess, capUMaxSuccess, degen, pnts, norms, uw_pnts, n_ref );
}

void CloneGeom::UpdateSplitTesselate( const VspSurf &surf, bool capUMinSuccess, bool capUMaxSuccess,
                                      vector< vector< vector< vec3d > > > &pnts,
                                      vector< vector< vector< vec3d > > > &norms ) const
{
    Geom* original_geom = GetOriginalGeom();
    if ( original_geom )
    {
        original_geom->UpdateSplitTesselate( surf, capUMinSuccess, capUMaxSuccess, pnts, norms );
        return;
    }

    Geom::UpdateSplitTesselate( surf, capUMinSuccess, capUMaxSuccess, pnts, norms );
}

void CloneGeom::UpdateEndCaps( int ncap )
{
    if ( m_CappingDone )
    {
        return;
    }
    m_CappingDone = true;

    m_CapUMinSuccess.clear();
    m_CapUMaxSuccess.clear();

    Geom* original_geom = GetOriginalGeom();
    if ( !original_geom )
    {
        return;
    }

    original_geom->GetCapSuccessCopy( m_CapUMinSuccess, m_CapUMaxSuccess );
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
        XmlUtil::AddStringNode( clone_node, "NameSuffix", m_NameSuffix );
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

        // A missing node keeps the default; an empty node is an empty suffix.
        if ( XmlUtil::GetNode( clone_node, "NameSuffix", 0 ) )
        {
            // Through the setter so the suffix is cleaned.
            SetNameSuffix( XmlUtil::FindString( clone_node, "NameSuffix", string() ) );
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
    else
    {
        // No original to name it after.
        m_AutoName = false;
    }

    m_XFormDirty = true;
    m_SurfDirty = true;
    m_AppearanceDirty = true;
    m_NameDirty = true;
    m_SubSurfDirty = true;

    Update();

    return true;
}
