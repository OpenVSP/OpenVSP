//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//
//
//////////////////////////////////////////////////////////////////////

#include "APIDefines.h"
#include "VspUtil.h"
#include "ConformalGeom.h"
#include "CloneGeom.h"
#include "Vehicle.h"
#include "VSP_Geom_API.h"
#include "WingGeom.h"
#include "HingeGeom.h"
#include "ParmMgr.h"
#include "IDMgr.h"
#include <cfloat>  //For DBL_EPSILON

using namespace vsp;

//==== Constructor ====//
ConformalGeom::ConformalGeom( Vehicle* vehicle_ptr ) : Geom( vehicle_ptr )
{
    m_Name = "ConformalGeom";
    m_Type.m_Name = "Conformal";
    m_Type.m_Type = CONFORMAL_GEOM_TYPE;
    // Possibly make dependent on m_DetachFlag()
    // m_Type.m_AdoptableFlag = false;


    // The end caps are copied from the parent, whatever type it allows.
    m_CapUMinOption.SetAllowedVals( CapTypes( true ) );
    m_CapUMaxOption.SetAllowedVals( CapTypes( true ) );

    m_DetachFlag.Init( "DetachFlag", "Design", this, false, false, true );
    m_DetachFlag.SetDescript( "Conformal detachment flag" );

    // Symmetry follows the conformal parent.
    m_SymAncestor.Deactivate();
    m_SymAncestOriginFlag.Deactivate();
    m_SymPlanFlag.Deactivate();
    m_SymAxFlag.Deactivate();
    m_SymRotN.Deactivate();

    m_HingeConformalPositionFlag.Init( "HingeConformalPositionFlag", "Design", this, true, false, true );
    m_HingeConformalPositionFlag.SetDescript( "Set REL position on hinge based on conformal parent position" );

    m_CapBack[0] = 0.0;
    m_CapBack[1] = 0.0;

    m_Offset.Init( "Offset", "Design", this, 0.1, -1e12, 1.0e12 );
    m_Offset.SetDescript( "Internal Offset Distance to Conformal Surface" );

    m_OffsetEnds.Init( "OffSetEnds", "Design", this, true, false, true );
    m_OffsetEnds.SetDescript( "Flag to offset the ends of a conformal component.");

    m_UTrimFlag.Init( "UTrimFlag", "Design", this, false, 0, 1 );
    m_UTrimFlag.SetDescript( "Trim in U Direction Flag" );
    m_CapBeforeTrimFlag.Init( "CapBeforeTrimFlag", "Design", this, false, 0, 1 );
    m_CapBeforeTrimFlag.SetDescript( "Flag to cap as the parent before U trimming, so a trim can fall in a cap" );

    m_UTrimMin.Init( "UTrimMin", "Design", this, 0.1, 0.0, 0.9999 );
    m_UTrimMin.SetDescript( "Min U Trim Value" );
    m_UTrimMax.Init( "UTrimMax", "Design", this, 0.9, 0.0, 0.9999 );
    m_UTrimMax.SetDescript( "Max U Trim Value" );

    m_UMinTrimTypeFlag.Init( "UMinTrimTypeFalg", "Design", this, vsp::U_TRIM, vsp::U_TRIM, vsp::NUM_TRIM_TYPES - 1 );
    m_UMaxTrimTypeFlag.Init( "UMaxTrimTypeFalg", "Design", this, vsp::U_TRIM, vsp::U_TRIM, vsp::NUM_TRIM_TYPES - 1 );

    m_LTrimMin.Init( "LTrimMin", "Design", this, 0.1, 0.0, 0.9999 );
    m_L01Min.Init( "L01Min", "Design", this, true, false, true );
    m_L0LenTrimMin.Init( "L0LenTrimMin", "Design", this, 0, 0, 1e12 );

    m_LTrimMax.Init( "LTrimMax", "Design", this, 0.9, 0.0, 0.9999 );
    m_L01Max.Init( "L01Max", "Design", this, true, false, true );
    m_L0LenTrimMax.Init( "L0LenTrimMax", "Design", this, 0, 0, 1e12 );

    m_EtaTrimMin.Init( "EtaTrimMin", "Design", this, 0.1, -1000.0, 1000.0 );
    m_EtaTrimMin.SetDescript( "Min eta Trim Value" );
    m_EtaTrimMax.Init( "EtaTrimMax", "Design", this, 0.9, -1000.0, 1000.0 );
    m_EtaTrimMax.SetDescript( "Max eta Trim Value" );

    // End Cap Options
    m_CapUMinTrimOption.Init("CapUMinTrimOption", "EndCap", this, FLAT_END_CAP, NO_END_CAP, ROUND_EXT_END_CAP_BOTH );
    m_CapUMinTrimOption.SetDescript("Type of End Cap on UMin end");

    m_CapUMinTrimLength.Init( "CapUMinTrimLength", "EndCap", this, 1, 0, 20 );
    m_CapUMinTrimLength.SetDescript( "Scaled length of end cap" );
    m_CapUMinTrimOffset.Init( "CapUMinTrimOffset", "EndCap", this, 0, -20, 20 );
    m_CapUMinTrimOffset.SetDescript( "Scaled offset of end cap" );
    m_CapUMinTrimStrength.Init( "CapUMinTrimStrength", "EndCap", this, 0.5, 0, 1 );
    m_CapUMinTrimStrength.SetDescript( "Tangent strength of end cap" );
    m_CapUMinTrimSweepFlag.Init( "CapUMinTrimSweepFlag", "EndCap", this, 0, 0, 1 );
    m_CapUMinTrimSweepFlag.SetDescript( "Flag to stretch end cap length for sweep" );

    m_CapUMaxTrimOption.Init("CapUMaxTrimOption", "EndCap", this, FLAT_END_CAP, NO_END_CAP, ROUND_EXT_END_CAP_BOTH );
    m_CapUMaxTrimOption.SetDescript("Type of End Cap on UMax end");

    m_CapUMaxTrimLength.Init( "CapUMaxTrimLength", "EndCap", this, 1, 0, 20 );
    m_CapUMaxTrimLength.SetDescript( "Scaled length of end cap" );
    m_CapUMaxTrimOffset.Init( "CapUMaxTrimOffset", "EndCap", this, 0, -20, 20 );
    m_CapUMaxTrimOffset.SetDescript( "Scaled offset of end cap" );
    m_CapUMaxTrimStrength.Init( "CapUMaxTrimStrength", "EndCap", this, 0.5, 0, 1 );
    m_CapUMaxTrimStrength.SetDescript( "Tangent strength of end cap" );
    m_CapUMaxTrimSweepFlag.Init( "CapUMaxTrimSweepFlag", "EndCap", this, 0, 0, 1 );
    m_CapUMaxTrimSweepFlag.SetDescript( "Flag to stretch end cap length for sweep" );

    m_V1TrimFlag.Init( "V1TrimFlag", "Design", this, false, 0, 1 );
    m_V1TrimFlag.SetDescript( "Trim in V Direction Flag" );
    m_V1TrimBegin.Init( "V1TrimBegin", "Design", this, 0.9, 0.0, 1.0 );
    m_V1TrimBegin.SetDescript( "Begin V Trim Value" );
    m_V1TrimEnd.Init( "V1TrimEnd", "Design", this, 0.1, 0.0, 1.0 );
    m_V1TrimEnd.SetDescript( "End V Trim Value" );

    m_V2TrimFlag.Init( "V2TrimFlag", "Design", this, false, 0, 1 );
    m_V2TrimFlag.SetDescript( "Trim in V Direction Flag" );
    m_V2TrimBegin.Init( "V2TrimBegin", "Design", this, 0.4, 0.0, 1.0 );
    m_V2TrimBegin.SetDescript( "Begin V Trim Value" );
    m_V2TrimEnd.Init( "V2TrimEnd", "Design", this, 0.6, 0.0, 1.0 );
    m_V2TrimEnd.SetDescript( "End V Trim Value" );

    m_ChordTrimFlag.Init( "ChordTrimFlag", "Design", this, false, 0, 1 );
    m_ChordTrimFlag.SetDescript( "Chord Span Flag" );
    m_ChordTrimMin.Init( "ChordTrimMin", "Design", this, 0.1, 0.0, 1.0 );
    m_ChordTrimMin.SetDescript( "Min Chord Trim Value" );
    m_ChordTrimMax.Init( "ChordTrimMax", "Design", this, 0.9, 0.0, 1.0 );
    m_ChordTrimMax.SetDescript( "Max Chord Trim Value" );

    m_Side1TrimFlag.Init( "Side1TrimFlag", "Design", this, false, 0, 1 );
    m_Side1TrimFlag.SetDescript( "Side1 trim flag" );
    m_Side1Trim.Init( "Side1Trim", "Design", this, 0.1, 0.0, 1.0 );
    m_Side1Trim.SetDescript( "Side1 Trim Value" );

    m_Side2TrimFlag.Init( "Side2TrimFlag", "Design", this, false, 0, 1 );
    m_Side2TrimFlag.SetDescript( "Side2 trim flag" );
    m_Side2Trim.Init( "Side2Trim", "Design", this, 0.1, 0.0, 1.0 );
    m_Side2Trim.SetDescript( "Side2 Trim Value" );

    m_Side3TrimFlag.Init( "Side3TrimFlag", "Design", this, false, 0, 1 );
    m_Side3TrimFlag.SetDescript( "Side3 trim flag" );
    m_Side3Trim.Init( "Side3Trim", "Design", this, 0.1, 0.0, 1.0 );
    m_Side3Trim.SetDescript( "Side3 Trim Value" );

    m_Side4TrimFlag.Init( "Side4TrimFlag", "Design", this, false, 0, 1 );
    m_Side4TrimFlag.SetDescript( "Side4 trim flag" );
    m_Side4Trim.Init( "Side4Trim", "Design", this, 0.1, 0.0, 1.0 );
    m_Side4Trim.SetDescript( "Side4 Trim Value" );

    m_WingParentFlag = false;
    m_ConformalParentReported = false;
    m_TessU = 41;
    m_TessW = 41;

    m_MainSurfVec.clear();

}

//==== Destructor ====//
ConformalGeom::~ConformalGeom()
{

}

void ConformalGeom::ApplyScale( double currentScale )
{
    m_Offset *= currentScale;
}

xmlNodePtr ConformalGeom::EncodeXml( xmlNodePtr & node )
{
    xmlNodePtr geom_node = Geom::EncodeXml( node );

    xmlNodePtr child_node = xmlNewChild( geom_node, NULL, BAD_CAST "Conformal", NULL );

    if ( child_node )
    {
        XmlUtil::AddStringNode( child_node, "ConformalParentID", GetConformalParent() );
    }

    return child_node;
}

xmlNodePtr ConformalGeom::DecodeXml( xmlNodePtr & node )
{
    xmlNodePtr geom_node = Geom::DecodeXml( node );

    xmlNodePtr child_node = XmlUtil::GetNode( geom_node, "Conformal", 0 );

    if ( child_node )
    {
        SetConformalParent( IDMgr.RemapRefID( XmlUtil::FindString( child_node, "ConformalParentID", GetConformalParent() ) ) );
    }

    return child_node;
}

// A detached conformal is built on its conformal parent as well as placed by its parent.
void ConformalGeom::GetDependencyIDs( vector< string > &ids )
{
    Geom::GetDependencyIDs( ids );
    ids.push_back( m_ConformalParentID );
}

bool ConformalGeom::SetConformalParent( const string &parent )
{
    Vehicle* veh = VehicleMgr.GetVehicle();

    bool parentchanged = false;
    if ( !veh )
    {
        return parentchanged;
    }

    // A conformal parent cannot be built from this Geom
    Geom* new_parent = veh->FindGeom( parent );
    if ( new_parent && new_parent->DependsOn( m_ID ) )
    {
        return parentchanged;
    }

    // Nor be a type a Conformal cannot follow, the same ones AddGeom refuses.
    if ( new_parent && ( new_parent->isNonSurfaceType() ||
                         new_parent->GetType().m_Type == CONFORMAL_GEOM_TYPE ||
                         new_parent->GetBehaviorType() == GEAR_GEOM_TYPE ) )
    {
        return parentchanged;
    }

    if ( m_ConformalParentID != parent )
    {
        Geom * g = veh->FindGeom( m_ConformalParentID );
        if ( g )
        {
            g->RemoveStepChildID( m_ID );
        }

        m_ConformalParentID = parent;

        g = veh->FindGeom( m_ConformalParentID );
        if ( g )
        {
            g->AddStepChildID( m_ID );
        }

        m_SurfDirty = true;
        m_XFormDirty = true;
        m_TessDirty = true;

        parentchanged = true;
        SetLateUpdateFlag( true );
    }
    return parentchanged;
}

void ConformalGeom::Update( bool fullupdate )
{
    // A file can name a conformal parent built from this Geom, which the flip would follow
    // around forever.  Drop it before anything uses it; UpdateSurf then takes the parent.
    Geom* conformal_parent = m_Vehicle->FindGeom( m_ConformalParentID );
    if ( conformal_parent && conformal_parent->DependsOn( m_ID ) )
    {
        string message = GetName() + " was built on " + conformal_parent->GetName() +
            ", which is built from " + GetName() + " itself.  It has been put back on its parent.";

        SetConformalParent( string() );

        // Reaches both the GUI (ScreenMgr message box) and scripts (ErrorMgr error stack).
        MessageData errMsgData;
        errMsgData.m_String = "Error";
        errMsgData.m_IntVec.push_back( vsp::VSP_CONFORMAL_PARENT_UNSUPPORTED );
        errMsgData.m_StringVec.push_back( "Error:  " + message );
        MessageMgr::getInstance().SendAll( errMsgData );
    }

    Geom::Update( fullupdate );
}

void ConformalGeom::UpdateSurf()
{
    if ( !m_DetachFlag() || GetConformalParent().empty() )
    {
        if ( SetConformalParent( m_ParentID ) ) // true if parent changed.
        {
            // Items from Update() that preceed UpdateSurf(), but that may depend on
            // m_ConformalParentID
            UpdateCopyXFormParms();

            UpdateCopySurfParms();

            UpdateCopyTessParms();

            UpdateXForm();
        }
    }

    //===== Find Parent ====//
    Geom* parent_geom = m_Vehicle->FindGeom( GetConformalParent() );
    if ( !parent_geom )
    {
        return;
    }

    // A Clone has no cross sections, so offset those of the Geom it shows; placement stays the Clone's.
    CloneGeom* clone_parent = dynamic_cast< CloneGeom* >( parent_geom );
    if ( clone_parent )
    {
        Geom* behavior_geom = clone_parent->GetBehaviorGeom();
        if ( behavior_geom && behavior_geom != parent_geom )
        {
            parent_geom = behavior_geom;
        }
    }

    // A Conformal of a Conformal does not work and never has: the offset is built by moving
    // the parent's skinning ribs, and a Conformal keeps none of its own to move.  It came out
    // collapsed on the origin.  Say so and stop, rather than produce that quietly.
    if ( parent_geom->GetType().m_Type == CONFORMAL_GEOM_TYPE )
    {
        // Once, not on every update.  UpdateSurf runs on every parameter change anywhere on
        // the Geom, and a model left in this state would otherwise put an error dialog in
        // front of the user on each one, which is not saying it more clearly -- it is
        // stopping them fixing it.  Reset when the parent changes to something that works.
        if ( !m_ConformalParentReported )
        {
            m_ConformalParentReported = true;

            MessageData errMsgData;
            errMsgData.m_String = "Error";
            errMsgData.m_StringVec.push_back( "Error: A Conformal Geom may not be the parent of another Conformal Geom" );
            MessageMgr::getInstance().SendAll( errMsgData );
        }
        return;
    }

    m_ConformalParentReported = false;

    //==== Wing Specific Stuff ====//
    m_WingParentFlag = false;
    if ( parent_geom->GetType().m_Type == MS_WING_GEOM_TYPE )
    {
        m_WingParentFlag = true;
    }

    bool COBRA_ParentFlag = false;
    if ( parent_geom->GetType().m_Type == COBRA_GEOM_TYPE )
    {
        COBRA_ParentFlag = true;
    }

    //===== Copy Parent ====//
    vector< string > parent_id_vec;
    parent_id_vec.push_back( parent_geom->GetID() );
    vector< string > copy_id_vec = m_Vehicle->CopyGeomVec( parent_id_vec );
    if ( copy_id_vec.size() != 1 )
    {
        return;
    }

    Geom* copy_geom = m_Vehicle->FindGeom( copy_id_vec[0] );
    if ( !copy_geom )
    {
        return;
    }

    copy_geom->Update();    // Make Sure Copy Is Current

    //==== Offset Cross Sections And Reskin ====//
    double offset = m_Offset();
    copy_geom->OffsetXSecs( offset );
    copy_geom->Update();
    copy_geom->GetMainSurfVecCopy( m_MainSurfVec );

    // The ribs are skinned where the parent's XSecs are, before its nose is shifted to the origin.
    vec3d nose_shift = parent_geom->GetNoseShift();

    // Offset Ends acts only on a ribbed surface of a parent other than a Cobra.
    bool offset_ends_used = false;

    for ( int i = 0 ; i < (int)m_MainSurfVec.size() ; i++ )
    {
        m_MainSurfVec[i].SetFoilSurf( nullptr );

        if ( m_MainSurfVec[i].IsClone() )
        {
            int clone_index = m_MainSurfVec[i].GetCloneIndex();
            Matrix4d clone_mat;
            m_MainSurfVec[i].GetCloneMat( clone_mat );

            m_MainSurfVec[i] = m_MainSurfVec[ clone_index ];
            m_MainSurfVec[i].Transform( clone_mat );
        }
        else
        {
            int skin_type = m_MainSurfVec[i].GetSkinType();

            UpdateParms( m_MainSurfVec[i] );

            //==== Only Works for Skin Ribs for Now ====//
            if (skin_type == VspSurf::SKIN_RIBS)
            {
                //==== Check If Wing ====//
                if (m_WingParentFlag)
                {
                    //==== Set Wing Specific Trimming Parms ====//
                    SetWingTrimParms(m_MainSurfVec[i]);
                }

                VspSurf parent_surf = *( parent_geom->GetMainSurfPtr( i ) );
                parent_surf.Offset( nose_shift * -1.0 );

                //==== Make Sure Ribs Are Centered ====//
                if ( !COBRA_ParentFlag )
                {
                    CenterRibCurves(m_MainSurfVec[i], parent_surf, offset);
                }

                //==== Offset Ribs ====//
                if ( !COBRA_ParentFlag )
                {
                    offset_ends_used = true;

                    if ( m_OffsetEnds() )
                    {
                        OffsetEndRibs(m_MainSurfVec[i], offset);
                    }
                }

                //==== Adjust Shape By Scaling Fp and Moving End Ribs ====//
                if ( !m_WingParentFlag && !COBRA_ParentFlag )
                {
// Measure Error - Expensive
//ComputeMaxOffsetError( m_MainSurfVec[i], parent_surf_vec[i], offset, 20, 8 );
                    AdjustShape(m_MainSurfVec[i], parent_surf, offset);
//ComputeMaxOffsetError( m_MainSurfVec[i], parent_surf_vec[i], offset, 20, 8 );
                }

                // Capped first, the trim measures and cuts the capped surface.
                if ( m_UTrimFlag() && m_CapBeforeTrimFlag() )
                {
                    CapAsParent( m_MainSurfVec[i] );
                    UpdateParms( m_MainSurfVec[i] );
                }

                //==== Trim U and V is Needed ====//
                TrimU(m_MainSurfVec[i]);
                TrimV(m_MainSurfVec[i]);

                m_MainSurfVec[i].Offset( nose_shift );
            }
                //==== To Do - Body of Rev (Pods) ====//
            else if (skin_type == VspSurf::SKIN_BODY_REV)
            {
                //==== Trim U and V is Needed ====//
                TrimU(m_MainSurfVec[i]);
                TrimV(m_MainSurfVec[i]);
            }
        }
    }

    if ( offset_ends_used )
    {
        m_OffsetEnds.Activate();
    }
    else
    {
        m_OffsetEnds.Deactivate();
    }

    //==== Delete Geom Copy ====//
    m_Vehicle->DeleteGeomVec( { copy_geom->GetID() } );

}

void ConformalGeom::UpdateCopyXFormParms()
{
    //===== Find Parent ====//
    Geom* parent_geom = m_Vehicle->FindGeom( GetConformalParent() );
    if ( !parent_geom )
    {
        return;
    }

    JointRole* hingeparent = Geom::CastTo< JointRole >( m_Vehicle->FindGeom( GetParentID() ) );

    m_HingeConformalPositionFlag.Deactivate();

    bool ActiveAttach = false;
    bool ActiveRel = true;
    if ( !m_DetachFlag() )
    {
        //==== Force Attached So Conformal Moves With Parent =====//
        m_TransAttachFlag = vsp::ATTACH_TRANS_COMP;
        m_RotAttachFlag = vsp::ATTACH_ROT_COMP;

        // Deactivate attachment parms
        ActiveAttach = false;
    }
    else // Detached
    {
        // Detached, do not change
        // m_TransAttachFlag
        // m_RotAttachFlag

        // Activate attachment parms
        ActiveAttach = true;


        if ( hingeparent )
        {
            // Option only available if immediate parent is HingeGeom
            m_HingeConformalPositionFlag.Activate();

            if ( m_HingeConformalPositionFlag() )
            {
                // Attached to hinge, so attachment is forced, this does not matter.
                // m_TransAttachFlag
                // m_RotAttachFlag

                // Deactivate attachment parms
                ActiveAttach = false;
                // Force relative positioning.
                m_AbsRelFlag = vsp::REL;
                // Deactivate relative position parms
                ActiveRel = false;

                Matrix4d hingeMat = hingeparent->GetRoleModelMatrix();
                Matrix4d parentMat = parent_geom->getModelMatrix();

                hingeMat.affineInverse();

                hingeMat.matMult( parentMat.data() );
                double tempMat[16];

                hingeMat.getMat( tempMat );

                // The translation column of attachedMat equals T_rel_vec + C - R_rel*C,
                // because the REL branch builds T_rel * T_C * R * T_{-C} whose origin maps to
                // T_rel_vec + C - R*C.  Recover T_rel_vec by adding R_rel*C - C.
                vec3d rc = hingeMat.xformnorm( m_Center );
                m_XRelLoc = tempMat[12] + rc.x() - m_Center.x();
                m_YRelLoc = tempMat[13] + rc.y() - m_Center.y();
                m_ZRelLoc = tempMat[14] + rc.z() - m_Center.z();

                vec3d angles = hingeMat.getAngles();
                m_XRelRot = angles.x();
                m_YRelRot = angles.y();
                m_ZRelRot = angles.z();

            }
        }
    }


    if ( ActiveAttach )
    {
        m_TransAttachFlag.Activate();
        m_RotAttachFlag.Activate();

        m_ULoc.Activate();
        m_U0NLoc.Activate();
        m_U01.Activate();
        m_WLoc.Activate();
        m_RLoc.Activate();
        m_R01.Activate();
        m_R0NLoc.Activate();
        m_SLoc.Activate();
        m_TLoc.Activate();
        m_LLoc.Activate();
        m_L01.Activate();
        m_L0LenLoc.Activate();
        m_MLoc.Activate();
        m_NLoc.Activate();
        m_EtaLoc.Activate();
    }
    else
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

    m_SymAncestor = parent_geom->m_SymAncestor();
    if ( m_SymAncestor() != 0 ) // Not global ancestor.
    {
        m_SymAncestor = m_SymAncestor() + 1;  // + 1 increment for parent
    }
    m_SymAncestOriginFlag = parent_geom->m_SymAncestOriginFlag();
    m_SymPlanFlag = parent_geom->m_SymPlanFlag();
    m_SymAxFlag = parent_geom->m_SymAxFlag();
    m_SymRotN = parent_geom->m_SymRotN();


    if ( ActiveRel )
    {
        m_AbsRelFlag.Activate();

        m_XRelLoc.Activate();
        m_YRelLoc.Activate();
        m_ZRelLoc.Activate();

        m_XRelRot.Activate();
        m_YRelRot.Activate();
        m_ZRelRot.Activate();
    }
    else
    {
        m_AbsRelFlag.Deactivate();

        m_XRelLoc.Deactivate();
        m_YRelLoc.Deactivate();
        m_ZRelLoc.Deactivate();

        m_XRelRot.Deactivate();
        m_YRelRot.Deactivate();
        m_ZRelRot.Deactivate();
    }

}

void ConformalGeom::UpdateCopySurfParms()
{
    //===== Find Parent ====//
    Geom* parent_geom = m_Vehicle->FindGeom( GetConformalParent() );
    if ( !parent_geom )
    {
        return;
    }

    //==== Copy Cap Options ====//
    m_CapUMinOption = parent_geom->m_CapUMinOption();
    m_CapUMinTess   = parent_geom->m_CapUMinTess();
    m_CapUMaxOption = parent_geom->m_CapUMaxOption();

    m_CapUMinLength = parent_geom->m_CapUMinLength();
    m_CapUMinOffset = parent_geom->m_CapUMinOffset();
    m_CapUMinRadius = parent_geom->m_CapUMinRadius();
    m_CapUMinOgiveAngle = parent_geom->m_CapUMinOgiveAngle();
    m_CapUMinPowerExp = parent_geom->m_CapUMinPowerExp();
    m_CapUMinHaackC = parent_geom->m_CapUMinHaackC();
    m_CapUMinTruncFlag = parent_geom->m_CapUMinTruncFlag();
    m_CapUMinShearFlag = parent_geom->m_CapUMinShearFlag();
    m_CapUMinScaledFlag = parent_geom->m_CapUMinScaledFlag();
    m_CapUMinStrength = parent_geom->m_CapUMinStrength();
    m_CapUMinSweepFlag = parent_geom->m_CapUMinSweepFlag();

    m_CapUMaxLength = parent_geom->m_CapUMaxLength();
    m_CapUMaxOffset = parent_geom->m_CapUMaxOffset();
    m_CapUMaxRadius = parent_geom->m_CapUMaxRadius();
    m_CapUMaxOgiveAngle = parent_geom->m_CapUMaxOgiveAngle();
    m_CapUMaxPowerExp = parent_geom->m_CapUMaxPowerExp();
    m_CapUMaxHaackC = parent_geom->m_CapUMaxHaackC();
    m_CapUMaxTruncFlag = parent_geom->m_CapUMaxTruncFlag();
    m_CapUMaxShearFlag = parent_geom->m_CapUMaxShearFlag();
    m_CapUMaxScaledFlag = parent_geom->m_CapUMaxScaledFlag();
    m_CapUMaxStrength = parent_geom->m_CapUMaxStrength();
    m_CapUMaxSweepFlag = parent_geom->m_CapUMaxSweepFlag();

    // A cap that carries the body on from its end is the parent's offset inside it, set in length units.
    double off = m_Offset();
    for ( int end = 0; end < 2; end++ )
    {
        IntParm* type = &m_CapUMinOption;
        BoolParm* rel = &m_CapUMinRelFlag;
        Parm* lendim = &m_CapUMinLengthDim;
        Parm* raddim = &m_CapUMinRadiusDim;
        Parm* offdim = &m_CapUMinOffsetDim;
        bool trunc = parent_geom->m_CapUMinTruncFlag();
        double ang = parent_geom->m_CapUMinOgiveAngle();
        double pexp = parent_geom->m_CapUMinPowerExp();
        double len = parent_geom->m_CapUMinLength();
        double plendim = parent_geom->m_CapUMinLengthDim();
        double praddim = parent_geom->m_CapUMinRadiusDim();
        double poffdim = parent_geom->m_CapUMinOffsetDim();
        if ( end == 1 )
        {
            type = &m_CapUMaxOption;
            rel = &m_CapUMaxRelFlag;
            lendim = &m_CapUMaxLengthDim;
            raddim = &m_CapUMaxRadiusDim;
            offdim = &m_CapUMaxOffsetDim;
            trunc = parent_geom->m_CapUMaxTruncFlag();
            ang = parent_geom->m_CapUMaxOgiveAngle();
            pexp = parent_geom->m_CapUMaxPowerExp();
            len = parent_geom->m_CapUMaxLength();
            plendim = parent_geom->m_CapUMaxLengthDim();
            praddim = parent_geom->m_CapUMaxRadiusDim();
            poffdim = parent_geom->m_CapUMaxOffsetDim();
        }
        m_CapBack[end] = 0.0;
        if ( !CapContinuesBody( type->Get() ) )
        {
            continue;
        }

        // The end's half width is the length's scale.
        double hw = 0.0;
        if ( len > 0.0 )
        {
            hw = plendim / len;
        }

        double len_off, rad_off;
        OffsetCap( type->Get(), trunc, ang, pexp, hw, plendim, praddim, off, len_off, rad_off, m_CapBack[end] );
        rel->Set( false );
        lendim->Set( len_off );
        raddim->Set( rad_off );
        offdim->Set( poffdim );
    }
}

bool ConformalGeom::CapContinuesBody( int cap_type )
{
    return cap_type == vsp::ROUND_END_CAP || cap_type == vsp::POINT_END_CAP || cap_type == vsp::CONE_END_CAP ||
           cap_type == vsp::OGIVE_END_CAP || cap_type == vsp::OGIVE_SURF_END_CAP ||
           cap_type == vsp::POWER_LAW_END_CAP || cap_type == vsp::VON_KARMAN_END_CAP ||
           cap_type == vsp::SEARS_HAACK_END_CAP;
}

void ConformalGeom::OffsetCap( int cap_type, bool trunc, double ang, double pexp, double hw, double len, double rad,
                               double off, double & len_off, double & rad_off, double & back )
{
    // The length from the end where it is; the end's move back is added last.
    len_off = len - off;
    rad_off = rad;
    back = 0.0;

    bool sphere = cap_type == vsp::CONE_END_CAP || cap_type == vsp::OGIVE_END_CAP ||
                  cap_type == vsp::OGIVE_SURF_END_CAP;
    bool profile = cap_type == vsp::POWER_LAW_END_CAP || cap_type == vsp::VON_KARMAN_END_CAP ||
                   cap_type == vsp::SEARS_HAACK_END_CAP;

    // The angle the cap leaves the end at, to the body: a cone's half angle, whose sides from the end's edge touch
    // the sphere and meet ( len - rad ) + rad / sin( theta ) from the end, hw across; an ogive's angle; a power
    // law's slope at its base; a point cap's to its point.  The others leave along the body.
    double theta = 0.0;
    if ( cap_type == vsp::CONE_END_CAP && len > rad && hw > rad )
    {
        double lo = 1.0e-6;
        double hi = 0.5 * M_PI - 1.0e-6;
        for ( int i = 0; i < 100; i++ )
        {
            double th = 0.5 * ( lo + hi );
            double f = ( ( len - rad ) + rad / sin( th ) ) * tan( th ) - hw;
            if ( f > 0.0 )
            {
                hi = th;
            }
            else
            {
                lo = th;
            }
        }
        theta = 0.5 * ( lo + hi );
    }
    else if ( ( cap_type == vsp::OGIVE_END_CAP || cap_type == vsp::OGIVE_SURF_END_CAP ) && ang > 0.0 )
    {
        theta = ang * M_PI / 180.0;
    }
    else if ( cap_type == vsp::POWER_LAW_END_CAP && len > 0.0 )
    {
        theta = atan( pexp * hw / len );
    }
    else if ( cap_type == vsp::POINT_END_CAP )
    {
        // With no length the point is on the end, a flat end square to the body.
        theta = 0.5 * M_PI;
        if ( len > 0.0 )
        {
            theta = atan2( hw, len );
        }
    }
    back = off * tan( 0.5 * theta );

    if ( trunc && ( sphere || profile ) )
    {
        // The face moves back by the offset and its edge in to where the offset side reaches it: across by
        // off ( 1 - sin( phi ) ) / cos( phi ) for a side at phi to the axis.
        double phi = 0.0;
        if ( len > 0.0 )
        {
            phi = atan2( hw - rad, len );
        }
        rad_off = std::max( rad - off * ( 1.0 - sin( phi ) ) / cos( phi ), 0.0 );
    }
    else if ( sphere && rad >= off )
    {
        // The same sphere, its radius less the offset; the sides are offset onto it.
        rad_off = rad - off;
    }
    else if ( cap_type == vsp::POINT_END_CAP && theta > 0.0 )
    {
        // Straight sides from the end's edge to the point, which the offset sides meet off / sin( theta ) back.
        len_off = len - off / sin( theta );
    }
    else if ( sphere )
    {
        // The sphere is gone, and the offset sides meet at the tip.
        rad_off = 0.0;
        if ( cap_type == vsp::CONE_END_CAP && theta > 0.0 )
        {
            len_off = ( len - rad ) + ( rad - off ) / sin( theta );
        }
        else if ( ang == 0.0 && len > rad && hw > rad )
        {
            // A tangent ogive's arc, square to the end at its edge, touches the sphere: its radius rho has its
            // center hw - rho across from the axis at the end.  The offset arc about the same center meets the
            // axis.
            double rho = ( ( len - rad ) * ( len - rad ) + hw * hw - rad * rad ) / ( 2.0 * ( hw - rad ) );
            double rho_off = rho - off;
            double y = rho - hw;
            if ( rho_off > std::abs( y ) )
            {
                len_off = sqrt( rho_off * rho_off - y * y );
            }
        }
    }

    len_off = std::max( len_off + back, 0.0 );
}

void ConformalGeom::UpdateCopyTessParms()
{
    //===== Find Parent ====//
    Geom* parent_geom = m_Vehicle->FindGeom( GetConformalParent() );
    if ( !parent_geom )
    {
        return;
    }

    //=== Let User Change Tess
    //m_TessU = parent_geom->m_TessU();
    //m_TessW = parent_geom->m_TessW();
}

void ConformalGeom::UpdateDrawObj()
{
    Geom::UpdateDrawObj();
}


void ConformalGeom::LoadDrawObjs( vector< DrawObj* > & draw_obj_vec )
{
    Geom::LoadDrawObjs( draw_obj_vec );
}

//==== Approximate Center ====//
vec3d ConformalGeom::ComputeCenter( curve_type & crv )
{
    vec3d sum(0,0,0);
    double tmax = crv.get_tmax();
    for ( int i = 0 ; i < 4 ; i++ )
    {
        double w0 = (double)i/4.0;
        vec3d pnt = vec3d( crv.f( w0 * tmax ) );
        sum = sum + pnt;
    }
    sum = sum*(1.0/4.0);

    return sum;
}

//==== Translate Curve ====//
void ConformalGeom::TranslateCurve( curve_type & crv, const vec3d & offset )
{
    piecewise_surface_type::point_type pnt;
    pnt << offset.x(), offset.y(), offset.z();
    crv.translate( pnt );
}

//==== Approx Offset Curve - Not Used ====//
void ConformalGeom::OffsetCurve( curve_type & crv, double offset )
{
    curve_type offset_crv;
    piecewise_curve_type::error_code err;
    vec3d center_pnt = ComputeCenter( crv );

    int ns = crv.number_segments();

    for ( int pp = 0; pp < ns; ++pp )
    {
        piecewise_surface_type::curve_type bez;
        crv.get( bez, pp );
        for ( int i=0; i<=bez.degree(); ++i)
        {
            double x = bez.get_control_point(i).x();
            double y = bez.get_control_point(i).y();
            double z = bez.get_control_point(i).z();
            vec3d cp = vec3d( x, y, z );
            vec3d dir = cp - center_pnt;
            double len = dir.mag();
            dir.normalize();


            double off_len = len - offset;
            off_len = MAX( 0.0, off_len );
            vec3d ncp = center_pnt + dir*off_len;

            piecewise_surface_type::point_type offset_cp;
            offset_cp << ncp.x(), ncp.y(), ncp.z();
            bez.set_control_point(offset_cp, i );

        }
        err = offset_crv.push_back( bez );
    }

    crv = offset_crv;
}

//==== Force Reskin ====//
void ConformalGeom::ReSkin( VspSurf & surf, vector< rib_data_type > & rib_vec )
{
    if ( surf.GetSkinBlendedFlag() )
    {
        // The parent was blended: one skin per group of stations enforcing alike, summed with
        // weights.  Its conditions cannot be carried by a single rib set -- taking one meant
        // applying that group's conditions right around the cross section, so the sides that
        // asked for something else silently lost it, and once a group's conditions are
        // confined to its own spans that rib is not one the uniform creator will even accept.
        //
        // Rebuild the same way instead.  The cross section curves are what this class moves,
        // and every group carries the same ones, so put them back into each group and blend
        // again with the weights the parent used.
        vector< vector< rib_data_type > > sets;
        surf.GetSkinRibSets( sets );

        for ( int s = 0; s < ( int )sets.size(); s++ )
        {
            for ( int i = 0; i < ( int )sets[s].size() && i < ( int )rib_vec.size(); i++ )
            {
                sets[s][i].set_f( rib_vec[i].get_f() );
            }
        }

        surf.ReSkinBlended( sets );
    }
    else
    {
        //==== Reskin Surface ====//
        bool closed_flag = surf.GetSkinClosedFlag();
        vector< int > degree_vec;
        surf.GetSkinDegreeVec( degree_vec );
        vector< double > parm_vec;
        surf.GetSkinParmVec( parm_vec );
        surf.SkinRibsUniform( rib_vec, degree_vec, parm_vec, closed_flag );
    }

    if ( m_WingParentFlag )
    {
        surf.FlipNormal();
        surf.SetSurfType( vsp::WING_SURF );
        surf.SetMagicVParm( true );
    }
}

//==== Check For Point XSec ====//
bool ConformalGeom::CheckIfRibIsPoint( rib_data_type & rib )
{
    const piecewise_curve_type& crv = rib.get_f();

    double tmax = crv.get_tmax();
    vec3d first_pnt = vec3d( crv.f( 0.0 ) );
    for ( int i = 1 ; i < 10 ; i++ )
    {
        double w0 = (double)i/10.0;
        vec3d pnt = vec3d( crv.f( w0*tmax ) );

        if ( dist( first_pnt, pnt ) > 1.0e-12 )
        {
            return false;
        }
    }
    return true;
}

//==== Move Rib To Center s ====//
void ConformalGeom::CenterRibCurves( VspSurf & surf, const VspSurf & ref_surf, double offset )
{
    vector< rib_data_type > rib_vec;
    surf.GetSkinRibVec( rib_vec );

    vector< rib_data_type > ref_rib_vec;
    ref_surf.GetSkinRibVec( ref_rib_vec );

    for ( int i = 0 ; i < rib_vec.size() ; i++ )
    {
        piecewise_curve_type ref_crv = ref_rib_vec[i].get_f();
        vec3d ref_center = ComputeCenter( ref_crv );

        piecewise_curve_type crv = rib_vec[i].get_f();
        vec3d center = ComputeCenter( crv);

        vec3d del_center = ref_center - center;
        TranslateCurve( crv, del_center );

        rib_vec[i].set_f( crv );
    }

    ReSkin( surf, rib_vec );
}

//==== Adjust Tangent to Reshape To Reduce Error ====//
void ConformalGeom::AdjustShape( VspSurf & surf, const VspSurf & ref_surf, double offset )
{
    vector< rib_data_type > rib_vec;
    surf.GetSkinRibVec( rib_vec );

    if ( rib_vec.size() < 2 )
        return;

    // This tunes the offset by scaling the tangents on one rib set.  A blended surface has one
    // set per group and they carry different tangents, so scaling the first group's and
    // reusing it for the rest would put that group's skinning everywhere -- the shape error it
    // is trying to reduce.  The offset is a little less closely held without it; the shape is
    // right, which matters more.
    if ( surf.GetSkinBlendedFlag() )
    {
        return;
    }

    double u_r = 1.0/3.0;
    double u_l = 2.0/3.0;
    double del_scale = 0.01;
    double fraction_target_scale = 0.9;

    unsigned int num_ribs = rib_vec.size();
    vector< double > orig_offset_r(num_ribs, 0.0);
    vector< double > change_offset_r(num_ribs, 0.0);
    vector< double > orig_offset_l(num_ribs, 0.0);
    vector< double > change_offset_l(num_ribs, 0.0);
    vector< rib_data_type > orig_rib_vec = rib_vec;

    bool reskin_flag = false;
    //==== Save Orig Offset and Adjust Rib Fp Scales ====//
    for ( int i = 0 ; i < num_ribs-1 ; i++ )
    {
        orig_offset_r[i] = ComputeAvgOffset( surf, ref_surf, double(i) + u_r );
        orig_offset_l[i] = ComputeAvgOffset( surf, ref_surf, double(i) + u_l );

        if ( rib_vec[i].use_right_fp() )
        {
            piecewise_curve_type fp_r = rib_vec[i].get_right_fp();
            fp_r.scale(1.0 + del_scale);
            rib_vec[i].set_right_fp(fp_r);
            reskin_flag = true;
        }

        if ( rib_vec[i+1].use_left_fp() )
        {
            piecewise_curve_type fp_l = rib_vec[i + 1].get_left_fp();
            fp_l.scale(1.0 + del_scale);
            rib_vec[i + 1].set_left_fp(fp_l);
            reskin_flag = true;
        }
    }

    // This object never uses right_fp or left_fp for skinning.  AdjustShape will
    // not do anything, so bail now.
    if ( !reskin_flag )
    {
        return;
    }

    //==== Reskin All - Changes Will Effect Other Ribs =====//
    ReSkin( surf, rib_vec );

    //==== Compute Changed Offset And Del Fp Scale RIGHT ====//
    for ( int i = 0 ; i < num_ribs-1 ; i++ )
    {
        change_offset_r[i] = ComputeAvgOffset(surf, ref_surf, double(i) + u_r);
        if (rib_vec[i].use_right_fp())
        {
            double del_offset = orig_offset_r[i] - change_offset_r[i];
            if (std::abs(del_offset) > DBL_EPSILON)
            {
                double target_del_offset = orig_offset_r[i] - offset;
                double target_del_scale = del_scale * (target_del_offset / del_offset);
                double target_scale = 1.0 + target_del_scale * fraction_target_scale;
                if (target_scale > 0.5 && target_scale < 2.0)
                {
                    piecewise_curve_type fp_r = orig_rib_vec[i].get_right_fp();
                    fp_r.scale(target_scale);
                    rib_vec[i].set_right_fp(fp_r);
                }
            }
        }
    }

    //==== Compute Changed Offset And Del Fp Scale LEFT ====//
    for ( int i = 0 ; i < num_ribs-1 ; i++ )
    {
        change_offset_l[i] = ComputeAvgOffset( surf, ref_surf, double(i) + u_l );
        if ( rib_vec[i+1].use_left_fp() )
        {
            double del_offset = orig_offset_l[i] - change_offset_l[i];
            if (std::abs(del_offset) > DBL_EPSILON)
            {
                double target_del_offset = orig_offset_l[i] - offset;
                double target_del_scale = del_scale * (target_del_offset / del_offset);
                double target_scale = 1.0 + target_del_scale * fraction_target_scale;
                if (target_scale > 0.5 && target_scale < 2.0)
                {
                    piecewise_curve_type fp_l = orig_rib_vec[i + 1].get_left_fp();
                    fp_l.scale(target_scale);
                    rib_vec[i + 1].set_left_fp(fp_l);
                }
            }
        }
    }

    ReSkin( surf, rib_vec );

    //==== Check For Error Increase ===//
    reskin_flag = false;
    for ( int i = 0 ; i < num_ribs-1 ; i++ )
    {
        double off_r = ComputeAvgOffset( surf, ref_surf, double(i) + u_r );
        double off_l = ComputeAvgOffset( surf, ref_surf, double(i) + u_l );
        double err_r = std::abs( off_r - offset);
        double err_l = std::abs( off_l - offset);
        double orig_err_r = std::abs( orig_offset_r[i] - offset );
        double orig_err_l = std::abs( orig_offset_l[i] - offset );

        if ( err_r > orig_err_r && rib_vec[i].use_right_fp() )
        {
            piecewise_curve_type fp_r = orig_rib_vec[i].get_right_fp();
            rib_vec[i].set_right_fp( fp_r );
            reskin_flag = true;
        }
        if ( err_l > orig_err_l && rib_vec[i+1].use_left_fp() )
        {
            piecewise_curve_type fp_l = orig_rib_vec[i+1].get_left_fp();
            rib_vec[i+1].set_left_fp( fp_l );
            reskin_flag = true;
        }
    }

    if ( reskin_flag )
    {
        ReSkin( surf, rib_vec );
    }

}

//==== Offset End Ribs If Capped or Point ====//
void ConformalGeom::OffsetEndRibs( VspSurf & surf, double offset )
{
    vector< rib_data_type > rib_vec;
    surf.GetSkinRibVec( rib_vec );
    vector < double > parmvec;
    surf.GetSkinParmVec( parmvec );

    int num_ribs = rib_vec.size();
    if ( num_ribs < 2 )
        return;

    bool reskin_flag = false;

    bool u_min_rib_is_point = CheckIfRibIsPoint( rib_vec[0] );
    // An end moves back by the offset, or for a cap that carries the body on, to where the offset body meets the
    // offset cap.
    bool min_carries = CapContinuesBody( m_CapUMinOption() );
    double min_back = offset;
    if ( min_carries )
    {
        min_back = m_CapBack[0];
    }
    if ( ( m_CapUMinOption() != vsp::NO_END_CAP && ( !min_carries || min_back > 0.0 ) ) || u_min_rib_is_point )
    {
        piecewise_curve_type crv;
        double uoffset = FindEndUOffsetCurve( surf, min_back, true, crv, !min_carries );

        if ( u_min_rib_is_point )
        {
            vec3d crv_center = ComputeCenter( crv );

            piecewise_curve_type orig_crv = rib_vec[0].get_f();
            vec3d orig_center = ComputeCenter( orig_crv );
            TranslateCurve( orig_crv, crv_center - orig_center );
            crv = orig_crv;
        }
        rib_vec[0].set_f( crv );

        // Check for blended ribs.
        for ( int irib = 1; irib < num_ribs; irib++ )
        {
            double unext = 0.5; // Parameter to stop checking.
            // Check rib parameter to check for blending
            if ( parmvec[irib] < ( unext - 1e-4 )) // num_ribs >= 2, so [1] is OK.
            {
                double unew = unext - ( (unext - uoffset) * ( unext - parmvec[irib] ) ) * 2.0;

                VspCurve c;
                surf.GetUConstCurve( c, unew );
                rib_vec[ irib ].set_f( c.GetCurve() );
            }
            else // All ribs in order -- once one fails, don't check the rest.
            {
                break;
            }
        }
        reskin_flag = true;
    }

    bool u_max_rib_is_point = CheckIfRibIsPoint( rib_vec[num_ribs-1] );
    // An end moves back by the offset, or for a cap that carries the body on, to where the offset body meets the
    // offset cap.
    bool max_carries = CapContinuesBody( m_CapUMaxOption() );
    double max_back = offset;
    if ( max_carries )
    {
        max_back = m_CapBack[1];
    }
    if ( ( m_CapUMaxOption() != vsp::NO_END_CAP && ( !max_carries || max_back > 0.0 ) ) || u_max_rib_is_point )
    {
        piecewise_curve_type crv;
        double uoffset = FindEndUOffsetCurve( surf, max_back, false, crv, !max_carries );

        if ( u_max_rib_is_point )
        {
            vec3d crv_center = ComputeCenter( crv );

            piecewise_curve_type orig_crv = rib_vec[num_ribs-1].get_f();
            vec3d orig_center = ComputeCenter( orig_crv );
            TranslateCurve( orig_crv, crv_center - orig_center );
            crv = orig_crv;
        }

        rib_vec[num_ribs-1].set_f( crv );

        // Check for blended ribs.
        for ( int irib = num_ribs - 2; irib >= 0; irib-- )
        {
            double unext = parmvec[ num_ribs - 1 ] - 0.5; // Next non-blended rib.
            // Check rib parameter to check for blending
            if ( parmvec[irib] > ( unext + 1e-4 ) )
            {
                double unew = unext + ( ( uoffset - unext ) * ( parmvec[irib] - unext ) ) * 2.0;

                VspCurve c;
                surf.GetUConstCurve( c, unew );
                rib_vec[ irib ].set_f( c.GetCurve() );
            }
            else // All ribs in order -- once one fails, don't check the rest.
            {
                break;
            }
        }
        reskin_flag = true;
    }

    if ( reskin_flag )
    {
        ReSkin( surf, rib_vec );
    }
}

//==== Move Offset Curve To Get Specified Offset ====//
double ConformalGeom::FindEndUOffsetCurve( VspSurf & surf, double offset, bool start_flag, curve_type & crv,
                                           bool adjust )
{
    ConformalSpine cs;
    cs.Build( surf );
    double u = 0.0;
    VspCurve vsp_crv;

    // A wing's end rib is a whole section, moved along the span by the offset.  The walls
    // beside it are already the offset away, since every section is offset, so measuring
    // clearance there measures the section's thickness -- which a tapered wing never gains
    // going inward, and the end would be walked most of the way to the middle.
    if ( start_flag )
    {
        u = cs.FindUGivenDist( offset );
        if ( !m_WingParentFlag && adjust )
        {
            u = AdjustForSurfaceDist( surf, cs, u, offset, false );
        }
    }
    else
    {
        double max_d = cs.GetMaxDist();
        u = cs.FindUGivenDist( max_d - offset );
        if ( !m_WingParentFlag && adjust )
        {
            u = AdjustForSurfaceDist( surf, cs, u, offset, true );
        }
    }

    surf.GetUConstCurve( vsp_crv, u );
    crv = vsp_crv.GetCurve();
    return u;
}

//==== End Points Of Conformal Surf Are Moved Offset Dist But Still May Be Too Close To Ref Surf  ===//
//==== Adjust Location Until Closest Dist Matches Offset ====//
double ConformalGeom::AdjustForSurfaceDist( const VspSurf & surf, const ConformalSpine & spine, double u, double offset, bool backward_flag )
{
    double su, sw;
    double start_u = u;
    double stop_u = u;

    vec3d center = spine.FindCenterGivenU( u );
    double w = surf.GetWMax() / 2.0;
    double surf_d = surf.FindNearest( su, sw, center, u, w );

    if ( surf_d >= offset )
    {
        return u;
    }

    ///==== Find U Endpoint Where Suf Dist Is Greater Than Offset ====//
    double del_u = 0.001;
    while ( del_u < 0.5 )
    {
        stop_u = u + del_u;
        if ( backward_flag )
        {
            stop_u = u - del_u;
        }
        center = spine.FindCenterGivenU( stop_u );
        surf_d = surf.FindNearest( su, sw, center, stop_u, w );

        //==== Found U ====//
        if ( surf_d > offset )
        {
            break;
        }
        else
        {
            start_u = stop_u;
            del_u *= 2.0;
        }
    }

    if ( del_u > 0.5 )
    {
        return start_u;
    }

    //==== Have Starting and Stopping U's - Use Bisection ====//
    int num_steps = 6;
    for ( int i = 0 ; i < num_steps ; i++ )
    {
        double bi_u = 0.5 * ( start_u + stop_u );
        center = spine.FindCenterGivenU( bi_u );
        surf_d = surf.FindNearest( su, sw, center, bi_u, w );

        if ( surf_d > offset )
        {
            stop_u = bi_u;
        }
        else
        {
            start_u = bi_u;
        }

    }
    return start_u;
}

//==== Solve For V Trim Parm Given Chord Trim Values ====//
void ConformalGeom::SetWingTrimParms(  VspSurf & surf )
{
    piecewise_surface_type* bez_surface = surf.GetBezierSurface();

    double u_max = bez_surface->get_umax();
    double v_max = bez_surface->get_vmax();

    if ( m_ChordTrimFlag() )
    {
        m_V1TrimFlag = true;
        m_V2TrimFlag = true;

        vec3d p0 = surf.CompPnt01( 0, 0 );
        vec3d p1 = surf.CompPnt01( 0, 0.5 );

        vec3d pmin = p0 + (p1 - p0)*m_ChordTrimMin();
        vec3d pmax = p0 + (p1 - p0)*m_ChordTrimMax();

        double u, vmin, vmax;
        surf.FindNearest01( u, vmin, pmin, 0, m_ChordTrimMin()/2.0 );
        surf.FindNearest01( u, vmax, pmax, 0, m_ChordTrimMax()/2.0 );

        if ( vmin < 0.5 )
        {
            m_V1TrimBegin = 1.0 - vmin;
            m_V1TrimEnd = vmin;
        }
        else
        {
            m_V1TrimBegin = vmin;
            m_V1TrimEnd = 1.0 - vmin;
        }
        if ( vmax < 0.5 )
        {
            m_V2TrimBegin = vmax;
            m_V2TrimEnd = 1.0 - vmax;
        }
        else
        {
            m_V2TrimBegin = 1.0 - vmax;
            m_V2TrimEnd = vmax;
        }
    }

}

Matrix4d ConformalGeom::GetFlipMat() const
{
    Geom* parent_geom = m_Vehicle->FindGeom( GetConformalParent() );
    if ( !parent_geom )
    {
        return Matrix4d();
    }

    return parent_geom->GetFlipMat();
}

int ConformalGeom::GetFlipFlag() const
{
    Geom* parent_geom = m_Vehicle->FindGeom( GetConformalParent() );
    if ( !parent_geom )
    {
        return 0;
    }

    return parent_geom->GetFlipFlag();
}

bool ConformalGeom::GetFlipReversesNormal() const
{
    Geom* parent_geom = m_Vehicle->FindGeom( GetConformalParent() );
    if ( !parent_geom )
    {
        return false;
    }

    return parent_geom->GetFlipReversesNormal();
}

void ConformalGeom::UpdateParms( VspSurf & surf )
{
    Geom* parent_geom = m_Vehicle->FindGeom( GetConformalParent() );
    if ( !parent_geom )
    {
        return;
    }

    // Only EtatoU and UtoEta are used; they depend on shape alone, so a Clone of a wing works too.
    WingGeom* behavior_wing = dynamic_cast < WingGeom* > ( parent_geom->GetBehaviorGeom() );

    double lmax = surf.GetLMax();

    if ( m_UMinTrimTypeFlag() == vsp::U_TRIM ) // Trim based on U.
    {
        double l, m, n;
        surf.ConvertRSTtoLMN( m_UTrimMin(), 0.5, 0.5, l, m, n );
        m_LTrimMin.Set( l );
        m_L0LenTrimMin.Set( m_LTrimMin() * lmax );

        if ( behavior_wing )
        {
            double umax = behavior_wing->NumXSec() - 1;
            m_EtaTrimMin = behavior_wing->UtoEta( m_UTrimMin() * umax, true );
        }
    }
    else if ( m_UMinTrimTypeFlag() == vsp::L_TRIM ) // Trim based on L.
    {
        if ( m_L01Min() )
        {
            m_L0LenTrimMin.Set( m_LTrimMin() * lmax );
        }
        else
        {
            double val = clamp( m_L0LenTrimMin(), 0.0, lmax );
            m_L0LenTrimMin.Set( val );
            m_LTrimMin.Set( val / lmax );
        }

        double r, s, t;
        surf.ConvertLMNtoRST( m_LTrimMin(), 0.5, 0.5, r, s, t );
        m_UTrimMin.Set( r );

        if ( behavior_wing )
        {
            double umax = behavior_wing->NumXSec() - 1;
            m_EtaTrimMin = behavior_wing->UtoEta( m_UTrimMin() * umax, true );
        }
    }
    else // Trim based on eta
    {
        if ( behavior_wing )
        {
            double umax = behavior_wing->NumXSec() - 1;
            m_UTrimMin = behavior_wing->EtatoU( m_EtaTrimMin(), true ) / umax;

            double l, m, n;
            surf.ConvertRSTtoLMN( m_UTrimMin(), 0.5, 0.5, l, m, n );
            m_LTrimMin.Set( l );
            m_L0LenTrimMin.Set( m_LTrimMin() * lmax );
        }
    }

    if ( m_UMaxTrimTypeFlag() == U_TRIM ) // Trim based on U.
    {
        double l, m, n;
        surf.ConvertRSTtoLMN( m_UTrimMax(), 0.5, 0.5, l, m, n );
        m_LTrimMax.Set( l );
        m_L0LenTrimMax.Set( m_LTrimMax() * lmax );

        if ( behavior_wing )
        {
            double umax = behavior_wing->NumXSec() - 1;
            m_EtaTrimMax = behavior_wing->UtoEta( m_UTrimMax() * umax, true );
        }
    }
    else if ( m_UMaxTrimTypeFlag() == L_TRIM ) // Trim based on L.
    {
        if ( m_L01Max() )
        {
            m_L0LenTrimMax.Set( m_LTrimMax() * lmax );
        }
        else
        {
            double val = clamp( m_L0LenTrimMax(), 0.0, lmax );
            m_L0LenTrimMax.Set( val );
            m_LTrimMax.Set( val / lmax );
        }

        double r, s, t;
        surf.ConvertLMNtoRST( m_LTrimMax(), 0.5, 0.5, r, s, t );
        m_UTrimMax.Set( r );

        if ( behavior_wing )
        {
            double umax = behavior_wing->NumXSec() - 1;
            m_EtaTrimMax = behavior_wing->UtoEta( m_UTrimMax() * umax, true );
        }
    }
    else // Trim based on eta
    {
        if ( behavior_wing )
        {
            double umax = behavior_wing->NumXSec() - 1;
            m_UTrimMax = behavior_wing->EtatoU( m_EtaTrimMax(), true ) / umax;

            double l, m, n;
            surf.ConvertRSTtoLMN( m_UTrimMax(), 0.5, 0.5, l, m, n );
            m_LTrimMax.Set( l );
            m_L0LenTrimMax.Set( m_LTrimMax() * lmax );
        }
    }

    // Prevent UTrim crossover.
    m_UTrimMin.SetUpperLimit( m_UTrimMax() - 0.001 );
    m_UTrimMax.SetLowerLimit( m_UTrimMin() + 0.001 );

    if ( m_UTrimMin() >= m_UTrimMax() )
    {
        m_UTrimMin = MAX( m_UTrimMax() - 0.001, 0.0 );
    }
}

// Possibly disable spanwise offset for wings.  As option.  Probably future work.
// Someday add span fraction coordinate for wings (eta).  do when spanwise coordinate for control surfaces is done.
// Possibly make U1 and U2 trimming separate options - if compatible means can be figured out.

//==== Trim In U Direction - Use Flat Cap ====//
void ConformalGeom::TrimU( VspSurf & surf )
{
    if ( !m_UTrimFlag() )
    {
        return;
    }

    piecewise_surface_type* bez_surface = surf.GetBezierSurface();
    piecewise_surface_type::index_type nupatch, nvpatch;
    piecewise_surface_type::error_code err;

    nupatch = bez_surface->number_u_patches();
    nvpatch = bez_surface->number_v_patches();

    piecewise_surface_type s1, s2, s3, s4;
    double u_min = bez_surface->get_u0();
    double u_max = bez_surface->get_umax();

    bool max_trimmed = true;
    err = bez_surface->split_u( s1, s2, u_max*m_UTrimMax() );
    if ( err != piecewise_surface_type::NO_ERRORS )
    {
        s1 = *bez_surface;          // Problems Dont Use This Split Val
        max_trimmed = false;
    }

    bool min_trimmed = true;
    err = s1.split_u( s3, s4, u_max*m_UTrimMin() );
    if ( err != piecewise_surface_type::NO_ERRORS )
    {
        s4 = s1;                    // Problems Dont Use This Split Val
        min_trimmed = false;
    }

    *bez_surface = s4;

    bez_surface->set_u0( u_min );
    bez_surface->set_umax( ceil( bez_surface->get_umax() ) );

    m_CapUMinOption = m_CapUMinTrimOption();
    m_CapUMaxOption = m_CapUMaxTrimOption();

    // An end capped before the trim and not trimmed keeps that cap alone.
    if ( m_CapBeforeTrimFlag() )
    {
        if ( !min_trimmed )
        {
            m_CapUMinOption = vsp::NO_END_CAP;
        }
        if ( !max_trimmed )
        {
            m_CapUMaxOption = vsp::NO_END_CAP;
        }
    }

    // The trimmed ends' caps are in relative units, with none of the parent's cap modifiers.
    m_CapUMinRelFlag = true;
    m_CapUMinLength = m_CapUMinTrimLength();
    m_CapUMinOffset = m_CapUMinTrimOffset();
    m_CapUMinStrength = m_CapUMinTrimStrength();
    m_CapUMinSweepFlag = m_CapUMinTrimSweepFlag();
    m_CapUMinTruncFlag = false;
    m_CapUMinShearFlag = false;
    m_CapUMinScaledFlag = false;

    m_CapUMaxRelFlag = true;
    m_CapUMaxLength = m_CapUMaxTrimLength();
    m_CapUMaxOffset = m_CapUMaxTrimOffset();
    m_CapUMaxStrength = m_CapUMaxTrimStrength();
    m_CapUMaxSweepFlag = m_CapUMaxTrimSweepFlag();
    m_CapUMaxTruncFlag = false;
    m_CapUMaxShearFlag = false;
    m_CapUMaxScaledFlag = false;
}

void ConformalGeom::CapAsParent( VspSurf & surf )
{
    vec3d ptoff;
    UpdateCapDims( surf, false );
    surf.CapUMin( m_CapUMinOption(), m_CapUMinLength(), m_CapUMinStrength(), m_CapUMinOffset(), ptoff,
                  m_CapUMinSweepFlag(), CapRadius( false ), m_CapUMinOgiveAngle() * M_PI / 180.0, m_CapUMinPowerExp(),
                  m_CapUMinHaackC(), m_CapUMinTruncFlag(), m_CapUMinShearFlag(), m_CapUMinScaledFlag() );
    UpdateCapDims( surf, true );
    surf.CapUMax( m_CapUMaxOption(), m_CapUMaxLength(), m_CapUMaxStrength(), m_CapUMaxOffset(), ptoff,
                  m_CapUMaxSweepFlag(), CapRadius( true ), m_CapUMaxOgiveAngle() * M_PI / 180.0, m_CapUMaxPowerExp(),
                  m_CapUMaxHaackC(), m_CapUMaxTruncFlag(), m_CapUMaxShearFlag(), m_CapUMaxScaledFlag() );
}

void ConformalGeom::TrimV( VspSurf & surf )
{
    double v_max = surf.GetWMax();

    if ( m_V1TrimFlag() )
    {
        double tb1 = v_max * m_V1TrimBegin();
        double te1 = v_max * m_V1TrimEnd();

        while ( tb1 >= v_max )
        {
            tb1 -= v_max;
        }
        while ( te1 >= v_max )
        {
            te1 -= v_max;
        }

        surf.TrimClosedV( tb1, te1 );
    }

    if ( m_V2TrimFlag() )
    {
        double tb2 = v_max * m_V2TrimBegin();
        double te2 = v_max * m_V2TrimEnd();

        while ( tb2 >= v_max )
        {
            tb2 -= v_max;
        }
        while ( te2 >= v_max )
        {
            te2 -= v_max;
        }

        surf.TrimClosedV( tb2, te2 );
    }

    if ( m_Side1TrimFlag() )
    {
        double b = v_max * ( 1.0 - m_Side1Trim() * 0.5 );
        double e = v_max * m_Side1Trim() * 0.5;

        b = clampCyclic( b, 0.0, v_max );
        e = clampCyclic( e, 0.0, v_max );

        surf.TrimClosedV( b, e );
    }

    if ( m_Side2TrimFlag() )
    {
        double b = v_max * ( 0.5 - m_Side2Trim() * 0.5 );
        double e = v_max * ( 0.5 + m_Side2Trim() * 0.5 );

        b = clampCyclic( b, 0.0, v_max );
        e = clampCyclic( e, 0.0, v_max );

        surf.TrimClosedV( b, e );
    }

    if ( m_Side3TrimFlag() )
    {
        double b = v_max * ( 0.25 - m_Side3Trim() * 0.5 );
        double e = v_max * ( 0.25 + m_Side3Trim() * 0.5 );

        b = clampCyclic( b, 0.0, v_max );
        e = clampCyclic( e, 0.0, v_max );

        surf.TrimClosedV( b, e );
    }

    if ( m_Side4TrimFlag() )
    {
        double b = v_max * ( 0.75 - m_Side4Trim() * 0.5 );
        double e = v_max * ( 0.75 + m_Side4Trim() * 0.5 );

        b = clampCyclic( b, 0.0, v_max );
        e = clampCyclic( e, 0.0, v_max );

        surf.TrimClosedV( b, e );
    }

}

//==== Find Distances At Point Along Curve To Surf =====//
void ConformalGeom::FindDists( const VspSurf & surf, piecewise_curve_type & curve, double u0, vector< double > & dist_vec )
{
    int num_samps = dist_vec.size();
    if ( num_samps <= 0 )
        return;

    double u, w;
    double tmax = curve.get_tmax();
    for ( int i = 0 ; i < num_samps ; i++ )
    {
        double w0 = (double)i/(double)num_samps;
        vec3d pnt = vec3d( curve.f( w0*tmax ) );

        dist_vec[i] = surf.FindNearest( u, w, pnt, u0, w0*tmax );
    }
}

double ConformalGeom::ComputeAvgOffset( VspSurf & surf, const VspSurf & ref_surf, double u )
{
    VspCurve crv;
    surf.GetUConstCurve( crv, u );
    piecewise_curve_type samp_crv = crv.GetCurve();

    vector< double > dist_vec;
    const unsigned int num_samps = 4;
    dist_vec.resize( num_samps, 0.0 );

    FindDists( ref_surf, samp_crv, u, dist_vec );

    double avg_offset = 0.0;
    for ( int i = 0 ; i < num_samps ; i++ )
    {
        avg_offset += dist_vec[i];
    }

    avg_offset /= (double)num_samps;

    return avg_offset;
}

//==== Compute Avg And Max Offset Errors and Print - Expensive Use for Debugging ====//
double ConformalGeom::ComputeMaxOffsetError( VspSurf & surf, VspSurf &  ref_surf, double offset, int num_u, int num_w )
{
    VspCurve crv;
    double max_err = 0.0;
    int sum_cnt = 0;
    double sum_err = 0.0;
    vector< double > dist_vec;

    for ( int i = 1 ; i < num_u-2 ; i++ )
    {
        double u = (surf.GetUMax()*(double)i)/(double)(num_u-1);

        surf.GetUConstCurve( crv, u );
        piecewise_curve_type samp_crv = crv.GetCurve();

        dist_vec.resize( num_w, 0.0 );
        FindDists( ref_surf, samp_crv, u, dist_vec );

        for ( int j = 0 ; j < (int)dist_vec.size() ; j++ )
        {
            double err = std::abs( dist_vec[j] - offset );
            if ( err > max_err )
                max_err = err;
            sum_err += err;
            sum_cnt++;

        }
    }
    double avg_err = sum_err/(double)sum_cnt;

printf( "Avg_Err = %f   Max_Err = %f\n", avg_err, max_err );

    return max_err;
}


////////////////////////////////////////////////////////////////////////////////////////////////
//============================================================================================//
//================================  Spine  ===================================================//
////////////////////////////////////////////////////////////////////////////////////////////////

//==== Build Spine - Vector of U, Center Points, Distance Along Spine ====//
void ConformalSpine::Build( const VspSurf & surf )
{
    //===== Clear Data ====//
    m_UVec.clear();
    m_CenterVec.clear();
    m_DistVec.clear();
    m_MaxDist = 0.0;

    const unsigned int num_samps = 100;
    m_UVec.reserve( num_samps );
    m_CenterVec.reserve( num_samps );
    m_DistVec.reserve( num_samps );

    m_DistVec.push_back(0.0);

    for ( int i = 0 ; i < num_samps ; i++ )
    {
        double u = (double)( i * surf.GetUMax() ) / (double)(num_samps - 1);
        vec3d sum;

        for ( int j = 0 ; j < 4 ; j++ )
        {
            double w = (double)( j * surf.GetWMax() ) / 4.0;
            vec3d pnt = surf.CompPnt( u, w );

            sum = sum + pnt;
        }
        vec3d center = sum*0.25;

        m_UVec.push_back( u );
        m_CenterVec.push_back( center );

        if ( m_CenterVec.size() >= 2 )
        {
            m_DistVec.push_back(m_DistVec[i - 1] + dist(m_CenterVec[i], m_CenterVec[i - 1]));
        }
    }
    m_MaxDist = m_DistVec.back();
}

//==== Given Dist Along Spine - Find U ====//
double ConformalSpine::FindUGivenDist( double d ) const
{
    double ret_u = 0.0;

    if ( m_DistVec.size() < 2 )
    {
        return ret_u;
    }

    if ( d > m_DistVec.back() )
    {
        return m_UVec.back();
    }

    for ( int i = 1 ; i < m_DistVec.size() ; i++ )
    {
        if ( d >= m_DistVec[i-1] && d <= m_DistVec[i] )
        {
            double del_d = m_DistVec[i] - m_DistVec[i-1];
            if ( del_d < DBL_EPSILON )
            {
                return m_UVec[i - 1];
            }

            double fract = ( d - m_DistVec[i-1] ) / del_d;
            ret_u = m_UVec[i-1] + fract * ( m_UVec[i] - m_UVec[i-1] );

            return ret_u;
        }
    }

    return ret_u;
}

//==== Find Center Of Spine Given U ====//
vec3d ConformalSpine::FindCenterGivenU( double u ) const
{
    vec3d center;

    if ( m_CenterVec.size() < 2 )
    {
        return center;
    }

    center = m_CenterVec[0];
    for ( int i = 1 ; i < m_UVec.size() ; i++ )
    {
        if ( u >= m_UVec[i-1] && u <= m_UVec[i] )
        {
            double del_u = m_UVec[i] - m_UVec[i-1];
            if ( del_u < DBL_EPSILON )
            {
                return m_CenterVec[i - 1];
            }

            double fract = ( u - m_UVec[i-1] ) / del_u;
            center = m_CenterVec[i-1] + fract * ( m_CenterVec[i] - m_CenterVec[i-1] );
            return center;
        }
    }
    return center;
}

double ConformalSpine::GetSpineLength() const
{
    double length = 0;

    if ( m_CenterVec.size() < 2 )
    {
        return length;
    }

    for ( size_t i = 1; i < m_CenterVec.size(); i++ )
    {
        length += dist( m_CenterVec[i - 1], m_CenterVec[i] );
    }

    return length;
}

//==== Find U Given Length AlongSpine ====//
double ConformalSpine::FindUGivenLengthAlongSpine( double length ) const
{
    double u = 0;

    if ( m_UVec.size() < 2 || length < 0 || length > GetSpineLength() )
    {
        return u;
    }

    // Init values
    double length_0 = 0;
    double length_1 = 0;

    for ( size_t j = 1; j < m_CenterVec.size(); j++ )
    {
        length_1 = length_0 + dist( m_CenterVec[j - 1], m_CenterVec[j] );
        double u_0 = m_UVec[j - 1];
        double u_1 = m_UVec[j];

        if ( length >= length_0 && length <= length_1 )
        {
            if ( std::abs( length_0 - length ) <= FLT_EPSILON )
            {
                u = u_0;
            }
            else if ( std::abs( length_1 - length ) <= FLT_EPSILON )
            {
                u = u_1;
            }
            else
            {
                // Linear interpolation
                u = u_0 + ( length - length_0 ) * ( ( u_1 - u_0 ) / ( length_1 - length_0 ) );
            }
            return u;
        }
        else
        {
            length_0 = length_1;
        }
    }
    return u;
}
