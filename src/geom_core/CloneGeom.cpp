//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//
//
//////////////////////////////////////////////////////////////////////

#include "CloneGeom.h"
#include "HumanGeom.h"
#include "Vehicle.h"
#include "ParmMgr.h"
#include "IDMgr.h"
#include "StlHelper.h"
#include "StringUtil.h"
#include "SubSurfaceMgr.h"
#include "XSec.h"

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

    m_CloneJoint.Init( "CloneJoint", "Behavior", this, true, false, true );
    m_CloneJoint.SetDescript( "Flag to copy the original's joint deflection" );

    m_AutoName.Init( "AutoName", "Behavior", this, true, false, true );
    m_AutoName.SetDescript( "Flag to name this Geom after the original with the suffix appended" );

    m_NameSuffix = "_Clone";

    m_JointTranslate.Init( "JointTranslate", "Hinge", this, 0.0, -1e12, 1e12 );
    m_JointTranslate.SetDescript( "Joint translation, when the original is a hinge" );

    m_JointRotate.Init( "JointRotate", "Hinge", this, 0.0, -360.0, 360.0 );
    m_JointRotate.SetDescript( "Joint rotation, when the original is a hinge" );

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

// The non-Clone Geom at the end of the chain of originals, or null if the chain ends in nothing
// or a ring.  Iterative and ring-safe: it is called while a file is read, before ResolveOriginal
// has rejected a ring.
Geom* CloneGeom::FollowOriginals() const
{
    set < string > visited;
    visited.insert( GetID() );

    Geom* geom_ptr = GetOriginalGeom();
    while ( geom_ptr )
    {
        if ( !visited.insert( geom_ptr->GetID() ).second )
        {
            return nullptr;
        }

        CloneGeom* clone_ptr = dynamic_cast< CloneGeom* >( geom_ptr );
        if ( !clone_ptr )
        {
            return geom_ptr;
        }

        geom_ptr = clone_ptr->GetOriginalGeom();
    }

    return nullptr;
}

// Follows the chain, so a Clone of a Clone reports the type at its end.
int CloneGeom::GetBehaviorType() const
{
    Geom* original_geom = FollowOriginals();
    if ( original_geom )
    {
        return original_geom->GetBehaviorType();
    }

    return Geom::GetBehaviorType();
}

Geom* CloneGeom::GetBehaviorGeom()
{
    Geom* original_geom = FollowOriginals();
    if ( original_geom )
    {
        return original_geom->GetBehaviorGeom();
    }

    return Geom::GetBehaviorGeom();
}

Geom* CloneGeom::GetMarkerGeom()
{
    // A Clone with no original has no markers.
    Geom* source = GetBehaviorGeom();
    if ( source == this )
    {
        return nullptr;
    }
    return source;
}

bool CloneGeom::LoadsMarkersAsMain()
{
    Geom* source = GetMarkerGeom();
    if ( source )
    {
        return source->LoadsMarkersAsMain();
    }
    return false;
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

int CloneGeom::GetJointPrimaryDir(  ) const
{
    JointRole* joint = GetOriginalJoint();
    if ( joint )
    {
        return joint->GetJointPrimaryDir(  );
    }

    return vsp::X_DIR;
}

JointRole* CloneGeom::GetOriginalJoint() const
{
    return Geom::CastTo< JointRole >( GetOriginalGeom() );
}

// Uses the hinge at the end of the chain, posed through this Geom's flip like its shape.
Matrix4d CloneGeom::BuildJointMatrix( double translate, double rotate, const Matrix4d &model_matrix ) const
{
    // An inner Clone's flip is already part of this Geom's flip flag, so the joint is
    // reflected once, by this Geom's flip.
    JointRole* joint = Geom::CastTo< JointRole >( FollowOriginals() );
    if ( !joint )
    {
        return model_matrix;
    }

    return joint->BuildFlippedJointMatrix( translate, rotate, model_matrix, GetFlipMat() );
}

bool CloneGeom::GetJointTransMotion( bool &min_set, double &min_val, bool &max_set, double &max_val ) const
{
    JointRole* joint = GetOriginalJoint();
    if ( joint )
    {
        return joint->GetJointTransMotion( min_set, min_val, max_set, max_val );
    }

    return false;
}

bool CloneGeom::GetJointRotMotion( bool &min_set, double &min_val, bool &max_set, double &max_val ) const
{
    JointRole* joint = GetOriginalJoint();
    if ( joint )
    {
        return joint->GetJointRotMotion( min_set, min_val, max_set, max_val );
    }

    return false;
}

void CloneGeom::SetJointParmLimits( Parm &translate, Parm &rotate )
{
    JointRole* joint = GetOriginalJoint();
    if ( joint )
    {
        joint->SetJointParmLimits( translate, rotate );
    }
}

// All children and step children below this Geom, recursively.  Ring-safe, because a file can
// name any parent or step child.
void CloneGeom::CollectDescendantIDs( set < string > &ids ) const
{
    vector < string > walk;

    walk.push_back( GetID() );

    while ( !walk.empty() )
    {
        Geom* geom_ptr = m_Vehicle->FindGeom( walk.back() );
        walk.pop_back();

        if ( !geom_ptr )
        {
            continue;
        }

        vector < string > below = geom_ptr->GetChildIDVec();
        vector < string > stepchildren = geom_ptr->GetStepChildIDVec();
        below.insert( below.end(), stepchildren.begin(), stepchildren.end() );

        for ( int i = 0; i < ( int )below.size(); i++ )
        {
            if ( below[i] != GetID() && ids.insert( below[i] ).second )
            {
                walk.push_back( below[i] );
            }
        }
    }
}

bool CloneGeom::IsDescendant( const string &id ) const
{
    // A descendant updates inside this Geom's update, so copying it would lag one pass behind.
    set < string > below;
    CollectDescendantIDs( below );

    return below.count( id ) > 0;
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

    // A file bypasses SetOriginalID, so a ring or a descendant is rejected here too.
    bool circular = false;

    if ( original_geom && ( IsCloneAncestor( m_OriginalID ) || IsDescendant( m_OriginalID ) ) )
    {
        // It is there, it just cannot be copied.  Let go of it properly: the Geom is still
        // holding this one in its step-child list.
        original_geom->RemoveStepChildID( GetID() );
        original_geom = nullptr;
        circular = true;
    }

    if ( !original_geom )
    {
        // Deleted, pasted without its original, or circular.  Go empty rather than silently
        // switch to copying the parent.
        m_OriginalID.clear();

        // Tell the user; only they can fix a circular case, by moving the Clone.
        string message;
        if ( circular )
        {
            message = GetName() +
                " can no longer copy what it was copying, because that Geom now hangs off " +
                GetName() + " itself.  Its original has been cleared.  Move " + GetName() +
                " out from under that Geom and choose an original again.";
        }
        else
        {
            message = GetName() +
                " has lost the Geom it was copying, and has been cleared.  Choose an original"
                " for it again.";
        }

        // Reaches both the GUI (ScreenMgr message box) and scripts (ErrorMgr error stack).
        MessageData errMsgData;
        errMsgData.m_String = "Error";
        errMsgData.m_IntVec.push_back( vsp::VSP_CLONE_ORIGINAL_LOST );
        errMsgData.m_StringVec.push_back( "Error:  " + message );
        MessageMgr::getInstance().SendAll( errMsgData );

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

SubSurface* CloneGeom::FindCopiedSubSurf( const string &source_id, int type )
{
    map < string, string >::iterator it = m_SubSurfSourceMap.find( source_id );
    if ( it == m_SubSurfSourceMap.end() )
    {
        return nullptr;
    }

    for ( int i = 0; i < ( int )m_SubSurfVec.size(); i++ )
    {
        if ( m_SubSurfVec[i]->GetID() == it->second )
        {
            // A subsurface cannot change type, so a mismatch is not this one.
            if ( m_SubSurfVec[i]->GetType() == type )
            {
                return m_SubSurfVec[i];
            }
            return nullptr;
        }
    }

    return nullptr;
}

// Match the copies to the original's subsurfaces, updating in place so IDs stay stable.
void CloneGeom::UpdateCopySubSurfs()
{
    Geom* original_geom = GetOriginalGeom();
    if ( !original_geom || !m_CloneSubSurfs() )
    {
        // Existing copies stay and become editable.  The pairing is kept so turning the switch
        // back on reuses them instead of adding duplicates.
        return;
    }

    vector< SubSurface* > oss = original_geom->GetSubSurfVec();

    vector< SubSurface* > keep;
    map < string, string > newmap;

    for ( int i = 0; i < ( int )oss.size(); i++ )
    {
        SubSurface* mine = FindCopiedSubSurf( oss[i]->GetID(), oss[i]->GetType() );

        if ( !mine )
        {
            mine = Geom::AddSubSurf( oss[i]->GetType(), oss[i]->m_MainSurfIndx() );
            if ( !mine )
            {
                continue;
            }
        }

        mine->CopyVals( oss[i] );
        mine->SetName( oss[i]->GetName() );

        // CopyVals does not recurse into child containers, so copy the cross section by hand.
        SSXSecCurve* mine_xsc = dynamic_cast< SSXSecCurve* >( mine );
        SSXSecCurve* orig_xsc = dynamic_cast< SSXSecCurve* >( oss[i] );
        if ( mine_xsc && orig_xsc && orig_xsc->GetXSecCurve() )
        {
            mine_xsc->SetXSecCurveType( orig_xsc->GetXSecCurve()->GetType() );

            if ( mine_xsc->GetXSecCurve() )
            {
                mine_xsc->GetXSecCurve()->CopyFrom( orig_xsc->GetXSecCurve() );
            }
        }

        if ( vector_contains_val( keep, mine ) )
        {
            // Two sources paired to one copy (hand-edited file); keep it once.
            continue;
        }

        newmap[ oss[i]->GetID() ] = mine->GetID();
        keep.push_back( mine );
    }

    // Copies as of the last update.  Any other subsurface is the user's and is kept.
    set < string > was_copy;
    map < string, string >::iterator iwas;
    for ( iwas = m_SubSurfSourceMap.begin(); iwas != m_SubSurfSourceMap.end(); ++iwas )
    {
        was_copy.insert( iwas->second );
    }

    vector< SubSurface* > own;
    for ( int i = 0; i < ( int )m_SubSurfVec.size(); i++ )
    {
        if ( vector_contains_val( keep, m_SubSurfVec[i] ) )
        {
            continue;
        }

        if ( was_copy.count( m_SubSurfVec[i]->GetID() ) > 0 )
        {
            // A copy of something the original no longer has.
            delete m_SubSurfVec[i];
        }
        else
        {
            own.push_back( m_SubSurfVec[i] );
        }
    }

    // The copies in the original's order, then whatever this Geom owns itself.
    m_SubSurfVec = keep;
    m_SubSurfVec.insert( m_SubSurfVec.end(), own.begin(), own.end() );
    m_SubSurfSourceMap = newmap;

    SubSurfaceMgr.ReSuffixGroupNames( GetID() );
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

// True when the original's shape is a mesh, point cloud, point grid or polygon mesh, placed by a
// single matrix.  Such Geoms ignore symmetry.
bool CloneGeom::ShowsOnePlacedShape() const
{
    return GetOriginalTMesh() || GetOriginalPointCloud() || GetOriginalWirePts() || GetOriginalPGMesh();
}

// No symmetry for a single placed shape.  Copy count, transforms and mass split all derive from
// this flag, so they stay consistent.
int CloneGeom::GetSymFlag() const
{
    if ( ShowsOnePlacedShape() )
    {
        return 0;
    }

    return Geom::GetSymFlag();
}

// Like a Blank or Hinge, a Clone with no surfaces still needs one symmetry copy per placement.
void CloneGeom::UpdateSymmAttach()
{
    if ( GetNumMainSurfs() < 1 )
    {
        Geom::UpdateSymmAttach( 1 );
        return;
    }

    Geom::UpdateSymmAttach();
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

    JointRole* joint = GetOriginalJoint();
    if ( joint )
    {
        // Limits are always the original's, even when the deflection is not copied.
        SetJointParmLimits( m_JointTranslate, m_JointRotate );

        if ( m_CloneJoint() )
        {
            m_JointTranslate.Set( joint->GetJointTranslate() );
            m_JointRotate.Set( joint->GetJointRotate() );
        }
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

    // Disable a deflection that is copied or that the original does not allow.
    bool min_set;
    bool max_set;
    double min_val;
    double max_val;

    if ( m_CloneJoint() || !GetJointTransMotion( min_set, min_val, max_set, max_val ) )
    {
        m_JointTranslate.Deactivate();
    }
    else
    {
        m_JointTranslate.Activate();
    }

    if ( m_CloneJoint() || !GetJointRotMotion( min_set, min_val, max_set, max_val ) )
    {
        m_JointRotate.Deactivate();
    }
    else
    {
        m_JointRotate.Activate();
    }

    // Deactivation persists, so reactivate what is no longer copied.
    if ( m_CloneSym() || ShowsOnePlacedShape() )
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

    // Assigned in place to reuse storage.  Includes the skinning inputs a conformal lofts from.
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

void CloneGeom::UpdateFeatureLines()
{
    if ( GetOriginalGeom() )
    {
        return;
    }

    Geom::UpdateFeatureLines();
}

void CloneGeom::UpdateLCurve()
{
    if ( GetOriginalGeom() )
    {
        return;
    }

    Geom::UpdateLCurve();
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

void CloneGeom::UpdateTessVec()
{
    Geom::UpdateTessVec();

    // Place the borrowed route with this Clone's symmetry transforms.
    if ( GetOriginalRoute() )
    {
        ApplySymm( GetMainRouteTessVec(), m_RouteTessVec );
        ApplySymm( GetMainRouteCurveTessVec(), m_RouteTessCurveVec );
    }
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

// Have the original build its default sources, then take ownership of them.
void CloneGeom::AddDefaultSources( double base_len )
{
    Geom* original_geom = GetOriginalGeom();
    if ( !original_geom )
    {
        return;
    }

    // Sources are appended to the original's list, so take the new ones off the end.
    int nbefore = original_geom->GetCfdMeshMainSourceVec().size();

    original_geom->AddDefaultSources( base_len );

    original_geom->TakeCfdMeshSourcesAfter( nbefore, m_MainSourceVec );

    SetCurrSourceID( ( int )m_MainSourceVec.size() - 1 );
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

        map < string, string >::iterator it;
        for ( it = m_SubSurfSourceMap.begin(); it != m_SubSurfSourceMap.end(); ++it )
        {
            xmlNodePtr pair_node = xmlNewChild( clone_node, NULL, BAD_CAST "SubSurfSource", NULL );
            if ( pair_node )
            {
                XmlUtil::AddStringNode( pair_node, "SourceID", it->first );
                XmlUtil::AddStringNode( pair_node, "CopyID", it->second );
            }
        }
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

        m_SubSurfSourceMap.clear();
        int npair = XmlUtil::GetNumNames( clone_node, "SubSurfSource" );
        for ( int i = 0; i < npair; i++ )
        {
            xmlNodePtr pair_node = XmlUtil::GetNode( clone_node, "SubSurfSource", i );
            if ( pair_node )
            {
                string sid = IDMgr.RemapRefID( XmlUtil::FindString( pair_node, "SourceID", string() ) );
                string mid = IDMgr.RemapRefID( XmlUtil::FindString( pair_node, "CopyID", string() ) );
                m_SubSurfSourceMap[ sid ] = mid;
            }
        }
    }

    return clone_node;
}

bool CloneGeom::IsCopiedSubSurf( const string &id ) const
{
    // Nothing is a copy while copying is off, even though the pairing is kept.
    if ( !m_CloneSubSurfs() )
    {
        return false;
    }

    map < string, string >::const_iterator it;
    for ( it = m_SubSurfSourceMap.begin(); it != m_SubSurfSourceMap.end(); ++it )
    {
        if ( it->second == id )
        {
            return true;
        }
    }

    return false;
}

bool CloneGeom::SetOriginalID( const string &id )
{
    // Refuse itself, a ring of Clones, and a descendant (which would lag one update behind).
    if ( id == GetID() || IsCloneAncestor( id ) || IsDescendant( id ) )
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

//==== Standing in for a landing gear ====//

GearContactRole* CloneGeom::GetOriginalGearContact() const
{
    return Geom::CastTo< GearContactRole >( GetOriginalGeom() );
}

void CloneGeom::BuildOnePtBasis( const string &cp1, int isymm1, int suspension1, int tire1, double thetabogie, double thetawheel, double thetaroll, Matrix4d &mat, vec3d &p1 )
{
    GearContactRole* gear = GetOriginalGearContact();
    if ( !gear )
    {
        return;
    }

    gear->BuildOnePtBasis( cp1, isymm1, suspension1, tire1, thetabogie, thetawheel, thetaroll, mat, p1 );
}

void CloneGeom::BuildTwoPtBasis( const string &cp1, int isymm1, int suspension1, int tire1, const string &cp2, int isymm2, int suspension2, int tire2, double thetabogie, Matrix4d &mat, vec3d &p1, vec3d &p2 )
{
    GearContactRole* gear = GetOriginalGearContact();
    if ( !gear )
    {
        return;
    }

    gear->BuildTwoPtBasis( cp1, isymm1, suspension1, tire1, cp2, isymm2, suspension2, tire2, thetabogie, mat, p1, p2 );
}

void CloneGeom::BuildThreePtBasis( const string &cp1, int isymm1, int suspension1, int tire1, const string &cp2, int isymm2, int suspension2, int tire2, const string &cp3, int isymm3, int suspension3, int tire3, Matrix4d &mat )
{
    GearContactRole* gear = GetOriginalGearContact();
    if ( !gear )
    {
        return;
    }

    gear->BuildThreePtBasis( cp1, isymm1, suspension1, tire1, cp2, isymm2, suspension2, tire2, cp3, isymm3, suspension3, tire3, mat );
}

void CloneGeom::BuildThreePtOffAxisBasis( const string &cp1, int isymm1, int suspension1, int tire1, const string &cp2, int isymm2, int suspension2, int tire2, const string &cp3, int isymm3, int suspension3, int tire3, double mainoffset, Matrix4d &mat )
{
    GearContactRole* gear = GetOriginalGearContact();
    if ( !gear )
    {
        return;
    }

    gear->BuildThreePtOffAxisBasis( cp1, isymm1, suspension1, tire1, cp2, isymm2, suspension2, tire2, cp3, isymm3, suspension3, tire3, mainoffset, mat );
}

bool CloneGeom::GetTwoPtPivot( const string &cp1, int isymm1, int suspension1, const string &cp2, int isymm2, int suspension2, vec3d &ptaxis, vec3d &axis ) const
{
    GearContactRole* gear = GetOriginalGearContact();
    if ( !gear )
    {
        return false;
    }

    return gear->GetTwoPtPivot( cp1, isymm1, suspension1, cp2, isymm2, suspension2, ptaxis, axis );
}

bool CloneGeom::GetTwoPtAftAxleAxis( const string &cp1, int isymm1, int suspension1, const string &cp2, int isymm2, int suspension2, double thetabogie, vec3d &ptaxis, vec3d &axis ) const
{
    GearContactRole* gear = GetOriginalGearContact();
    if ( !gear )
    {
        return false;
    }

    return gear->GetTwoPtAftAxleAxis( cp1, isymm1, suspension1, cp2, isymm2, suspension2, thetabogie, ptaxis, axis );
}

bool CloneGeom::GetTwoPtFwdAxleAxis( const string &cp1, int isymm1, int suspension1, const string &cp2, int isymm2, int suspension2, double thetabogie, vec3d &ptaxis, vec3d &axis ) const
{
    GearContactRole* gear = GetOriginalGearContact();
    if ( !gear )
    {
        return false;
    }

    return gear->GetTwoPtFwdAxleAxis( cp1, isymm1, suspension1, cp2, isymm2, suspension2, thetabogie, ptaxis, axis );
}

bool CloneGeom::GetTwoPtMeanContactPtNormal( const string &cp1, int isymm1, int suspension1, int tire1, const string &cp2, int isymm2, int suspension2, int tire2, double thetabogie, vec3d &pt, vec3d &normal, vec3d &p1, vec3d &p2, bool &usepivot, double &mintheta, double &maxtheta ) const
{
    GearContactRole* gear = GetOriginalGearContact();
    if ( !gear )
    {
        return false;
    }

    return gear->GetTwoPtMeanContactPtNormal( cp1, isymm1, suspension1, tire1, cp2, isymm2, suspension2, tire2, thetabogie, pt, normal, p1, p2, usepivot, mintheta, maxtheta );
}

bool CloneGeom::GetTwoPtAftContactPtNormal( const string &cp1, int isymm1, int suspension1, int tire1, const string &cp2, int isymm2, int suspension2, int tire2, double thetabogie, double thetawheel, vec3d &pt, vec3d &normal, vec3d &p1, vec3d &p2 ) const
{
    GearContactRole* gear = GetOriginalGearContact();
    if ( !gear )
    {
        return false;
    }

    return gear->GetTwoPtAftContactPtNormal( cp1, isymm1, suspension1, tire1, cp2, isymm2, suspension2, tire2, thetabogie, thetawheel, pt, normal, p1, p2 );
}

bool CloneGeom::GetTwoPtFwdContactPtNormal( const string &cp1, int isymm1, int suspension1, int tire1, const string &cp2, int isymm2, int suspension2, int tire2, double thetabogie, double thetawheel, vec3d &pt, vec3d &normal, vec3d &p1, vec3d &p2 ) const
{
    GearContactRole* gear = GetOriginalGearContact();
    if ( !gear )
    {
        return false;
    }

    return gear->GetTwoPtFwdContactPtNormal( cp1, isymm1, suspension1, tire1, cp2, isymm2, suspension2, tire2, thetabogie, thetawheel, pt, normal, p1, p2 );
}

bool CloneGeom::GetTwoPtSideContactPtsNormal( const string &cp1, int isymm1, int suspension1, int tire1, const string &cp2, int isymm2, int suspension2, int tire2, vec3d &p1, vec3d &p2, vec3d &normal ) const
{
    GearContactRole* gear = GetOriginalGearContact();
    if ( !gear )
    {
        return false;
    }

    return gear->GetTwoPtSideContactPtsNormal( cp1, isymm1, suspension1, tire1, cp2, isymm2, suspension2, tire2, p1, p2, normal );
}

bool CloneGeom::GetOnePtSideContactPtAxisNormal( const string &cp1, int isymm1, int suspension1, int tire1, double thetabogie, double thetawheel, double thetaroll, vec3d &p1, vec3d &axis, vec3d &normal, int &ysign ) const
{
    GearContactRole* gear = GetOriginalGearContact();
    if ( !gear )
    {
        return false;
    }

    return gear->GetOnePtSideContactPtAxisNormal( cp1, isymm1, suspension1, tire1, thetabogie, thetawheel, thetaroll, p1, axis, normal, ysign );
}

bool CloneGeom::GetPtNormal( const string &cp1, int isymm1, int suspension1, int tire1, const string &cp2, int isymm2, int suspension2, int tire2, const string &cp3, int isymm3, int suspension3, int tire3, vec3d &pt, vec3d &normal ) const
{
    GearContactRole* gear = GetOriginalGearContact();
    if ( !gear )
    {
        return false;
    }

    return gear->GetPtNormal( cp1, isymm1, suspension1, tire1, cp2, isymm2, suspension2, tire2, cp3, isymm3, suspension3, tire3, pt, normal );
}

bool CloneGeom::GetSteerAngle( const string &cp1, const string &cp2, const string &cp3, int &isteer, double &steerangle ) const
{
    GearContactRole* gear = GetOriginalGearContact();
    if ( !gear )
    {
        return false;
    }

    return gear->GetSteerAngle( cp1, cp2, cp3, isteer, steerangle );
}

void CloneGeom::GetNominalPtNormal( vec3d &pt, vec3d &normal ) const
{
    GearContactRole* gear = GetOriginalGearContact();
    if ( !gear )
    {
        return;
    }

    gear->GetNominalPtNormal( pt, normal );
}

void CloneGeom::GetCG( vec3d &cgnom, vector < vec3d > &cgbounds ) const
{
    GearContactRole* gear = GetOriginalGearContact();
    if ( !gear )
    {
        return;
    }

    gear->GetCG( cgnom, cgbounds );
}

bool CloneGeom::GetContactPointVecNormal( const string &cp1, int isymm1, int suspension1, int tire1, const string &cp2, int isymm2, int suspension2, int tire2, const string &cp3, int isymm3, int suspension3, int tire3, vector < vec3d > &ptvec, vec3d &normal ) const
{
    GearContactRole* gear = GetOriginalGearContact();
    if ( !gear )
    {
        return false;
    }

    return gear->GetContactPointVecNormal( cp1, isymm1, suspension1, tire1, cp2, isymm2, suspension2, tire2, cp3, isymm3, suspension3, tire3, ptvec, normal );
}

Bogie* CloneGeom::GetBogie( const string &id ) const
{
    GearContactRole* gear = GetOriginalGearContact();
    if ( !gear )
    {
        return nullptr;
    }

    return gear->GetBogie( id );
}

vector < Bogie* > CloneGeom::GetBogieVec()
{
    GearContactRole* gear = GetOriginalGearContact();
    if ( !gear )
    {
        return vector < Bogie* >();
    }

    return gear->GetBogieVec();
}

int CloneGeom::GetGearModelLenUnits() const
{
    GearContactRole* gear = GetOriginalGearContact();
    if ( !gear )
    {
        return 0;
    }

    return gear->GetGearModelLenUnits();
}

//==== Standing in for an auxiliary geom ====//

AuxiliaryRole* CloneGeom::GetOriginalAuxiliary() const
{
    return Geom::CastTo< AuxiliaryRole >( GetOriginalGeom() );
}

// This Clone's parent, not the original's.
GearContactRole* CloneGeom::GetContactGear() const
{
    return Geom::CastTo< GearContactRole >( m_Vehicle->FindGeom( m_ParentID ) );
}

int CloneGeom::GetAuxiliaryMode() const
{
    AuxiliaryRole* aux = GetOriginalAuxiliary();
    if ( !aux )
    {
        return vsp::AUX_GEOM_ROTOR_TIP_PATH;
    }

    return aux->GetAuxiliaryMode();
}

bool CloneGeom::GetCGInGear( vec3d &cgnom, vector < vec3d > &cgbounds )
{
    AuxiliaryRole* aux = GetOriginalAuxiliary();
    if ( !aux )
    {
        return false;
    }

    return aux->GetCGInGear( cgnom, cgbounds );
}

bool CloneGeom::GetPtNormalInGear( vec3d &pt, vec3d &normal ) const
{
    AuxiliaryRole* aux = GetOriginalAuxiliary();
    if ( !aux )
    {
        return false;
    }

    return aux->GetPtNormalInGear( pt, normal );
}

bool CloneGeom::GetPtNormalMeanContactPtPivotAxisInGear( vec3d &pt, vec3d &normal, vec3d &ptaxis, vec3d &axis, bool &usepivot, double &mintheta, double &maxtheta )
{
    AuxiliaryRole* aux = GetOriginalAuxiliary();
    if ( !aux )
    {
        return false;
    }

    return aux->GetPtNormalMeanContactPtPivotAxisInGear( pt, normal, ptaxis, axis, usepivot, mintheta, maxtheta );
}

bool CloneGeom::GetSideContactPtRollAxisNormalInGear( vec3d &pt, vec3d &axis, vec3d &normal, int &ysign )
{
    AuxiliaryRole* aux = GetOriginalAuxiliary();
    if ( !aux )
    {
        return false;
    }

    return aux->GetSideContactPtRollAxisNormalInGear( pt, axis, normal, ysign );
}

bool CloneGeom::GetPtNormalAftAxleAxisInGear( double thetabogie, vec3d &pt, vec3d &normal, vec3d &ptaxis, vec3d &axis )
{
    AuxiliaryRole* aux = GetOriginalAuxiliary();
    if ( !aux )
    {
        return false;
    }

    return aux->GetPtNormalAftAxleAxisInGear( thetabogie, pt, normal, ptaxis, axis );
}

bool CloneGeom::GetPtNormalFwdAxleAxisInGear( double thetabogie, vec3d &pt, vec3d &normal, vec3d &ptaxis, vec3d &axis )
{
    AuxiliaryRole* aux = GetOriginalAuxiliary();
    if ( !aux )
    {
        return false;
    }

    return aux->GetPtNormalFwdAxleAxisInGear( thetabogie, pt, normal, ptaxis, axis );
}

bool CloneGeom::GetPtPivotAxisInGear( vec3d &ptaxis, vec3d &axis )
{
    AuxiliaryRole* aux = GetOriginalAuxiliary();
    if ( !aux )
    {
        return false;
    }

    return aux->GetPtPivotAxisInGear( ptaxis, axis );
}

bool CloneGeom::GetTwoPtSideContactPtsNormalInGear( vec3d &p1, vec3d &p2, vec3d &normal )
{
    AuxiliaryRole* aux = GetOriginalAuxiliary();
    if ( !aux )
    {
        return false;
    }

    return aux->GetTwoPtSideContactPtsNormalInGear( p1, p2, normal );
}

bool CloneGeom::GetContactPointVecNormalInGear( vector < vec3d > &ptvec, vec3d &normal )
{
    AuxiliaryRole* aux = GetOriginalAuxiliary();
    if ( !aux )
    {
        return false;
    }

    return aux->GetContactPointVecNormalInGear( ptvec, normal );
}

bool CloneGeom::CalculateTurnInGear( vec3d &cor, vec3d &normal, vector<double> &rvec )
{
    AuxiliaryRole* aux = GetOriginalAuxiliary();
    if ( !aux )
    {
        return false;
    }

    return aux->CalculateTurnInGear( cor, normal, rvec );
}

//==== Standing in for a rotor ====//

double CloneGeom::GetRotorDiameter() const
{
    RotorRole* rotor = Geom::CastTo< RotorRole >( GetOriginalGeom() );
    if ( !rotor )
    {
        return 0.0;
    }

    return rotor->GetRotorDiameter();
}

double CloneGeom::GetRotorR0() const
{
    RotorRole* rotor = Geom::CastTo< RotorRole >( GetOriginalGeom() );
    if ( !rotor )
    {
        return 0.0;
    }

    return rotor->GetRotorR0();
}

bool CloneGeom::GetRotorReverseFlag() const
{
    RotorRole* rotor = Geom::CastTo< RotorRole >( GetOriginalGeom() );
    if ( !rotor )
    {
        return false;
    }

    return rotor->GetRotorReverseFlag();
}

bool CloneGeom::GetRotorHubDiameter( double &hubdia ) const
{
    RotorRole* rotor = Geom::CastTo< RotorRole >( GetOriginalGeom() );
    if ( !rotor )
    {
        return false;
    }

    return rotor->GetRotorHubDiameter( hubdia );
}

bool CloneGeom::GetSpreadTriInSelf( vec3d &pt, vec3d &axis, vector < vec3d > &t, int &flip ) const
{
    AuxiliaryRole* aux = GetOriginalAuxiliary();
    if ( !aux )
    {
        return false;
    }

    return aux->GetSpreadTriInSelf( pt, axis, t, flip );
}

//==== Standing in for a Geom whose shape is a mesh ====//

HumanVertRole* CloneGeom::GetOriginalHumanVert() const
{
    return Geom::CastTo< HumanVertRole >( GetOriginalGeom() );
}

const vector < vec3d > & CloneGeom::GetMainVerts() const
{
    static const vector < vec3d > empty;

    HumanVertRole* verts = GetOriginalHumanVert();
    if ( !verts )
    {
        return empty;
    }

    return verts->GetMainVerts();
}

// The original's vertices, placed by this Geom's transforms.
void CloneGeom::BuildCloneVerts( vector < vector < vec3d > > &verts, vector < bool > &flipnormal ) const
{
    ExpandMainVerts( GetMainVerts(), m_TransMatVec, verts );

    flipnormal = m_FlipNormalVec;
    flipnormal.resize( verts.size(), false );
}

RouteRole* CloneGeom::GetOriginalRoute() const
{
    return Geom::CastTo< RouteRole >( GetOriginalGeom() );
}

const vector < VspCurve > & CloneGeom::GetMainRouteCurveVec() const
{
    static const vector < VspCurve > empty;

    RouteRole* route = GetOriginalRoute();
    if ( !route )
    {
        return empty;
    }

    return route->GetMainRouteCurveVec();
}

const vector < SimpleFeatureTess > & CloneGeom::GetMainRouteCurveTessVec() const
{
    static const vector < SimpleFeatureTess > empty;

    RouteRole* route = GetOriginalRoute();
    if ( !route )
    {
        return empty;
    }

    return route->GetMainRouteCurveTessVec();
}

const vector < SimpleFeatureTess > & CloneGeom::GetMainRouteTessVec() const
{
    static const vector < SimpleFeatureTess > empty;

    RouteRole* route = GetOriginalRoute();
    if ( !route )
    {
        return empty;
    }

    return route->GetMainRouteTessVec();
}

double CloneGeom::GetRouteLinearDensity() const
{
    RouteRole* route = GetOriginalRoute();
    if ( !route )
    {
        return 0.0;
    }

    return route->GetRouteLinearDensity();
}

int CloneGeom::GetNumRoutePts() const
{
    RouteRole* route = GetOriginalRoute();
    if ( !route )
    {
        return 0;
    }

    // The count is the original's; the positions come from GetRoutePtCoord.
    return route->GetNumRoutePts();
}

PGMeshRole* CloneGeom::GetOriginalPGMesh() const
{
    return Geom::CastTo< PGMeshRole >( GetOriginalGeom() );
}

PGMulti* CloneGeom::GetPGMulti() const
{
    PGMeshRole* pgmesh = GetOriginalPGMesh();
    if ( !pgmesh )
    {
        return nullptr;
    }

    return pgmesh->GetPGMulti();
}

Matrix4d CloneGeom::GetPGScaleMat() const
{
    PGMeshRole* pgmesh = GetOriginalPGMesh();
    if ( !pgmesh )
    {
        return Matrix4d();
    }

    return pgmesh->GetPGScaleMat();
}

Matrix4d CloneGeom::GetPGTransMat() const
{
    return PlaceBorrowedShape( GetPGScaleMat() );
}

WirePtRole* CloneGeom::GetOriginalWirePts() const
{
    return Geom::CastTo< WirePtRole >( GetOriginalGeom() );
}

const vector < vector < vec3d > > & CloneGeom::GetMainWirePts() const
{
    static const vector < vector < vec3d > > empty;

    WirePtRole* wire = GetOriginalWirePts();
    if ( !wire )
    {
        return empty;
    }

    return wire->GetMainWirePts();
}

Matrix4d CloneGeom::GetWireScaleMat() const
{
    WirePtRole* wire = GetOriginalWirePts();
    if ( !wire )
    {
        return Matrix4d();
    }

    return wire->GetWireScaleMat();
}

Matrix4d CloneGeom::GetWireTransMat() const
{
    return PlaceBorrowedShape( GetWireScaleMat() );
}

// The original's facing, corrected for both Geoms' flips.
bool CloneGeom::GetWireInvert() const
{
    Geom* original_geom = GetOriginalGeom();
    WirePtRole* wire = GetOriginalWirePts();
    if ( !wire || !original_geom )
    {
        return false;
    }

    // A flip reverses the grid's normals.  Remove the original's flip, then apply this
    // Geom's.  This flag also sets the winding of triangles given to analyses.
    bool invert = wire->GetWireInvert() != original_geom->GetFlipReversesNormal();
    return invert != GetFlipReversesNormal();
}

int CloneGeom::GetWireDegenType() const
{
    WirePtRole* wire = GetOriginalWirePts();
    if ( !wire )
    {
        return DegenGeom::SURFACE_TYPE;
    }

    return wire->GetWireDegenType();
}

PointCloudRole* CloneGeom::GetOriginalPointCloud() const
{
    return Geom::CastTo< PointCloudRole >( GetOriginalGeom() );
}

const vector < vec3d > & CloneGeom::GetPtsInSelf() const
{
    static const vector < vec3d > empty;

    PointCloudRole* cloud = GetOriginalPointCloud();
    if ( !cloud )
    {
        return empty;
    }

    return cloud->GetPtsInSelf();
}

Matrix4d CloneGeom::GetPtsScaleMat() const
{
    PointCloudRole* cloud = GetOriginalPointCloud();
    if ( !cloud )
    {
        return Matrix4d();
    }

    return cloud->GetPtsScaleMat();
}

Matrix4d CloneGeom::GetPtsTransMat() const
{
    return PlaceBorrowedShape( GetPtsScaleMat() );
}

TMeshRole* CloneGeom::GetOriginalTMesh() const
{
    return Geom::CastTo< TMeshRole >( GetOriginalGeom() );
}

// Returned unchanged, in the original's own frame.
const vector< TMesh* > & CloneGeom::GetTMeshVecInSelf() const
{
    static const vector< TMesh* > empty;

    TMeshRole* mesh = GetOriginalTMesh();
    if ( !mesh )
    {
        return empty;
    }

    return mesh->GetTMeshVecInSelf();
}

vector< TMesh* > CloneGeom::CreateTMeshVecInSelf( bool skipnegflipnormal, const int &n_ref ) const
{
    TMeshRole* mesh = Geom::CastTo< TMeshRole >( GetOriginalGeom() );
    if ( !mesh )
    {
        return vector< TMesh* >();
    }

    return mesh->CreateTMeshVecInSelf( skipnegflipnormal, n_ref );
}

int CloneGeom::GetTMeshColorStartDegree() const
{
    TMeshRole* mesh = GetOriginalTMesh();
    if ( !mesh )
    {
        return 0;
    }

    return mesh->GetTMeshColorStartDegree();
}

void CloneGeom::WriteStl( FILE* file_id )
{
    if ( GetOriginalTMesh() )
    {
        WriteTMeshStl( file_id );
        return;
    }

    Geom::WriteStl( file_id );
}

// The original's slices, in its own frame.  Needed because the view flags are copied too: a Clone
// showing the mesh while the original shows only slices would export triangles the original omits.
const vector< TMesh* > & CloneGeom::GetTMeshSliceVec() const
{
    static const vector< TMesh* > empty;

    TMeshRole* mesh = GetOriginalTMesh();
    if ( !mesh )
    {
        return empty;
    }

    return mesh->GetTMeshSliceVec();
}

bool CloneGeom::GetTMeshViewMeshFlag() const
{
    TMeshRole* mesh = GetOriginalTMesh();
    if ( !mesh )
    {
        return true;
    }

    return mesh->GetTMeshViewMeshFlag();
}

bool CloneGeom::GetTMeshViewSliceFlag() const
{
    TMeshRole* mesh = GetOriginalTMesh();
    if ( !mesh )
    {
        return false;
    }

    return mesh->GetTMeshViewSliceFlag();
}

Matrix4d CloneGeom::GetTMeshScaleMat() const
{
    TMeshRole* mesh = GetOriginalTMesh();
    if ( !mesh )
    {
        return Matrix4d();
    }

    return mesh->GetTMeshScaleMat();
}

// The original's tag-to-draw-object map, so triangles are grouped the same way.
const map< vector < int >, int > & CloneGeom::GetTMeshSingleTagMap() const
{
    static const map< vector < int >, int > empty;

    TMeshRole* mesh = GetOriginalTMesh();
    if ( !mesh )
    {
        return empty;
    }

    return mesh->GetTMeshSingleTagMap();
}

// Placed at this Geom, not at the original.
Matrix4d CloneGeom::GetTMeshTransMat() const
{
    return PlaceBorrowedShape( GetTMeshScaleMat() );
}

// The original's scale, then this Geom's flip and placement.
Matrix4d CloneGeom::PlaceBorrowedShape( const Matrix4d &scale_mat ) const
{
    Matrix4d mat;
    mat.initMat( scale_mat );

    // These shapes are not placed through m_TransMatVec, so apply the flip here, innermost as
    // there.  TMesh::copyPlaced or the point grid's facing flag fixes the normals.
    mat.postMult( GetFlipMat() );

    mat.postMult( m_ModelMatrix );

    return mat;
}

vector< TMesh* > CloneGeom::CreateTMeshVec( bool skipnegflipnormal, const int &n_ref ) const
{
    // No surfaces to tessellate: borrow the mesh and place it at this Geom.
    if ( GetOriginalTMesh() )
    {
        return BuildTMeshVec( this );
    }

    if ( GetOriginalHumanVert() )
    {
        vector < vector < vec3d > > verts;
        vector < bool > flipnormal;
        BuildCloneVerts( verts, flipnormal );

        return BuildHumanTMeshVec( verts, flipnormal, this );
    }

    if ( GetOriginalPGMesh() )
    {
        return BuildPGTMeshVec( this );
    }

    // Two triangles per grid cell, as the original gives to analyses.
    if ( GetOriginalWirePts() )
    {
        vector < vector < vec3d > > xform_pts;
        vector < vector < vec3d > > xform_norm;
        BuildWireXFormPts( GetMainWirePts(), GetWireTransMat(), GetWireInvert(), xform_pts, xform_norm );

        return BuildWireTMeshVec( xform_pts, GetWireInvert(), this );
    }

    return Geom::CreateTMeshVec( skipnegflipnormal, n_ref );
}

void CloneGeom::LoadDrawObjs( vector< DrawObj* > & draw_obj_vec )
{
    Geom::LoadDrawObjs( draw_obj_vec );

    // Draw a point cloud as points; there is no surface to set visibility from.
    if ( GetOriginalPointCloud() )
    {
        for ( int i = 0 ; i < ( int )m_WireShadeDrawObj_vec.size() ; i++ )
        {
            m_WireShadeDrawObj_vec[i].m_Type = DrawObj::VSP_POINTS;
            m_WireShadeDrawObj_vec[i].m_PointSize = 4.0;
            m_WireShadeDrawObj_vec[i].m_PointColor = vec3d( m_GuiDraw.GetWireColor().x() / 255.0,
                                                            m_GuiDraw.GetWireColor().y() / 255.0,
                                                            m_GuiDraw.GetWireColor().z() / 255.0 );
            m_WireShadeDrawObj_vec[i].m_Visible = GetSetFlag( vsp::SET_SHOWN );
        }
        return;
    }

    // A grid of a single row or column is a polyline, not a mesh.
    if ( GetOriginalWirePts() )
    {
        LoadWireLineDrawObj( m_WireLineDO, draw_obj_vec );
        return;
    }

    if ( GetOriginalRoute() )
    {
        m_RouteLineDO.m_Visible = GetSetFlag( vsp::SET_SHOWN );
        m_RouteLineDO.m_LineColor = vec3d( m_GuiDraw.GetWireColor().x() / 255.0,
                                           m_GuiDraw.GetWireColor().y() / 255.0,
                                           m_GuiDraw.GetWireColor().z() / 255.0 );
        draw_obj_vec.push_back( &m_RouteLineDO );
        return;
    }

    // Faces and outlines per tag, coloured by tag.
    if ( GetOriginalPGMesh() )
    {
        LoadPGDrawObjs( m_WireShadeDrawObj_vec, m_GuiDraw.GetDrawType(), GetSetFlag( vsp::SET_SHOWN ) );
        return;
    }

    if ( GetOriginalTMesh() || GetOriginalHumanVert() )
    {
        for ( int i = 0 ; i < ( int )m_WireShadeDrawObj_vec.size() ; i++ )
        {
            m_WireShadeDrawObj_vec[i].m_Visible = GetSetFlag( vsp::SET_SHOWN );
        }

        // Colour by tag when subsurfaces are shown, as the original does.
        if ( GetOriginalTMesh() && m_GuiDraw.GetDispSubSurfFlag() )
        {
            TMeshRole::SetTagDrawObjColors( m_WireShadeDrawObj_vec, GetTMeshColorStartDegree(), GetTMeshSingleTagMap().size() );
        }

        TMeshRole::SetTriDrawObjTypes( m_WireShadeDrawObj_vec, m_GuiDraw.GetDrawType() );
    }
}

void CloneGeom::UpdateBBox()
{
    // No main surfaces: bound the borrowed shape at this Geom's position.
    if ( GetOriginalTMesh() || GetOriginalHumanVert() || GetOriginalPointCloud() || GetOriginalWirePts() || GetOriginalPGMesh() || GetOriginalRoute() )
    {
        BndBox new_box;

        if ( GetOriginalRoute() )
        {
            BuildRouteBndBox( new_box );
        }
        else if ( GetOriginalTMesh() )
        {
            BuildTMeshBndBox( new_box );
        }
        else if ( GetOriginalHumanVert() )
        {
            vector < vector < vec3d > > verts;
            vector < bool > flipnormal;
            BuildCloneVerts( verts, flipnormal );

            BuildHumanBndBox( verts, new_box );
        }
        else if ( GetOriginalPGMesh() )
        {
            BuildPGBndBox( new_box );
        }
        else if ( GetOriginalWirePts() )
        {
            vector < vector < vec3d > > xform_pts;
            vector < vector < vec3d > > xform_norm;
            BuildWireXFormPts( GetMainWirePts(), GetWireTransMat(), GetWireInvert(), xform_pts, xform_norm );

            BuildWireBndBox( xform_pts, new_box );
        }
        else
        {
            BuildPtsBndBox( new_box );
        }

        if ( new_box.IsEmpty() )
        {
            new_box.Update( vec3d( 0.0, 0.0, 0.0 ) );
        }

        m_BbXLen = new_box.GetMax( 0 ) - new_box.GetMin( 0 );
        m_BbYLen = new_box.GetMax( 1 ) - new_box.GetMin( 1 );
        m_BbZLen = new_box.GetMax( 2 ) - new_box.GetMin( 2 );

        m_BbXMin = new_box.GetMin( 0 );
        m_BbYMin = new_box.GetMin( 1 );
        m_BbZMin = new_box.GetMin( 2 );

        m_BBox = new_box;
        m_ScaleIndependentBBox = m_BBox;
        return;
    }

    Geom::UpdateBBox();
}

void CloneGeom::UpdateDrawObj()
{
    // No tessellation: draw the mesh the way the original does.
    if ( GetOriginalTMesh() )
    {
        BuildTMeshDrawObjs( GetTMeshVecInSelf(), m_GuiDraw.GetDispSubSurfFlag(), m_WireShadeDrawObj_vec );

        m_HighlightDrawObj.m_PntVec = m_BBox.GetBBoxDrawLines();
        m_HighlightDrawObj.m_GeomChanged = true;
        return;
    }

    if ( GetOriginalHumanVert() )
    {
        vector < vector < vec3d > > verts;
        vector < bool > flipnormal;
        BuildCloneVerts( verts, flipnormal );

        BuildHumanDrawObjs( verts, flipnormal, m_WireShadeDrawObj_vec );

        m_HighlightDrawObj.m_PntVec = m_BBox.GetBBoxDrawLines();
        m_HighlightDrawObj.m_GeomChanged = true;
        return;
    }

    if ( GetOriginalPGMesh() )
    {
        BuildPGDrawObjs( m_WireShadeDrawObj_vec );

        m_HighlightDrawObj.m_PntVec = m_BBox.GetBBoxDrawLines();
        m_HighlightDrawObj.m_GeomChanged = true;
        return;
    }

    if ( GetOriginalRoute() )
    {
        // Line only; the route points are edited on the original.
        BuildRouteLineDrawObj( m_RouteLineDO );
        m_RouteLineDO.m_GeomID = "Rte_" + m_ID;

        m_HighlightDrawObj.m_PntVec = m_BBox.GetBBoxDrawLines();
        m_HighlightDrawObj.m_GeomChanged = true;
        return;
    }

    if ( GetOriginalWirePts() )
    {
        vector < vector < vec3d > > xform_pts;
        vector < vector < vec3d > > xform_norm;
        BuildWireXFormPts( GetMainWirePts(), GetWireTransMat(), GetWireInvert(), xform_pts, xform_norm );

        BuildWireDrawObjs( xform_pts, xform_norm, m_WireShadeDrawObj_vec, m_WireLineDO );

        m_HighlightDrawObj.m_PntVec = m_BBox.GetBBoxDrawLines();
        m_HighlightDrawObj.m_GeomChanged = true;
        return;
    }

    if ( GetOriginalPointCloud() )
    {
        // All points; hiding and picking belong to the original.
        m_WireShadeDrawObj_vec.resize( 1, DrawObj() );
        BuildXFormPts( m_WireShadeDrawObj_vec[0].m_PntVec );
        m_WireShadeDrawObj_vec[0].m_GeomChanged = true;

        m_HighlightDrawObj.m_PntVec = m_BBox.GetBBoxDrawLines();
        m_HighlightDrawObj.m_GeomChanged = true;
        return;
    }

    Geom::UpdateDrawObj();
}
