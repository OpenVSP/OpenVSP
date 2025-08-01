//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//
//
//////////////////////////////////////////////////////////////////////

#include "Geom.h"

#include "AuxiliaryGeom.h"
#include "Vehicle.h"
#include "WingGeom.h"
#include "StlHelper.h"
#include "DXFUtil.h"
#include "SVGUtil.h"
#include "StringUtil.h"
#include "ParmMgr.h"
#include "SubSurfaceMgr.h"
#include "HingeGeom.h"
#include "HumanGeom.h"
#include "VspUtil.h"
using namespace vsp;

#include <float.h>

//==== Constructor ====//
GeomType::GeomType()
{
    m_Type = POD_GEOM_TYPE;
    m_Name = "Pod";
    m_FixedFlag = false;
    m_AdoptableFlag = true;
}

//==== Constructor ====//
GeomType::GeomType( int id, const string& name, bool fixed_flag, const string& module_name, const string& display_name )
{
    m_Type = id;
    m_Name = name;
    m_FixedFlag = fixed_flag;
    m_ModuleName = module_name;
    m_DisplayName = display_name;
}

//==== Destructor ====//
GeomType::~GeomType()
{
}

void GeomType::CopyFrom( const GeomType & t )
{
    m_Type = t.m_Type;
    m_Name = t.m_Name;
    m_FixedFlag = t.m_FixedFlag;
    m_ModuleName = t.m_ModuleName;
    m_DisplayName = t.m_DisplayName;
    m_GeomID = t.m_GeomID;
}




//===============================================================================//
//===============================================================================//
//===============================================================================//

//==== Constructor ====//
GeomGuiDraw::GeomGuiDraw()
{
    m_DisplayType = DISPLAY_TYPE::DISPLAY_BEZIER;
    m_DrawType = GEOM_DRAW_WIRE;
    m_DisplayChildrenFlag = true;
    m_DispSubSurfFlag = true;
    m_DispFeatureFlag = true;
}

//==== Destructor ====//
GeomGuiDraw::~GeomGuiDraw()
{

}

void GeomGuiDraw::SetMaterialToDefault()
{
    m_Material.SetMaterialToDefault( );
}

void GeomGuiDraw::SetMaterial( const std::string &name, double ambi[], double diff[], double spec[], double emis[], double shin )
{
    m_Material.SetMaterial( name, ambi, diff, spec, emis, shin );
}

void GeomGuiDraw::SetMaterial( const std::string &name )
{
    m_Material.SetMaterial( name );
}

//===============================================================================//
//===============================================================================//
//===============================================================================//

//==== Constructor ====//
GeomBase::GeomBase( Vehicle* vehicle_ptr )
{
    SetParmContainerType( vsp::ATTROBJ_GEOM );
    AttachAttrCollection();

    m_Vehicle = vehicle_ptr;

    m_Name = "GeomBase";
    m_Type.m_Type = BASE_GEOM_TYPE;
    m_Type.m_Name = m_Name;
    m_ParentID = string( "NONE" );

    m_XFormDirty = true;
    m_SurfDirty = true;
    m_TessDirty = true;
    m_HighlightDirty = true;
    m_FeaDirty = true;
    // This flag is only set true when global scale is updated.
    m_GlobalScaleDirty = false;

    m_UpdateXForm = false;
    m_UpdateSurf = false;
}

//==== Destructor ====//
GeomBase::~GeomBase()
{

}

// SetDirtyFlags is a bit of an abomination.
// This implements the magic logic to allow more fine grained Geom::Update().  Essentially, it classifies each Parm
// changed (called from ParmChanged) by the kind of effect it has.  For now, there are three groups:
//
// XForm - position, orientation, symmetry, attachment of Geom
// Surf - everything else (requires re-lofting m_MainSurfVec)
// Tess - tessellation resolution (requires re-tessellation)
// Highlight - active section highlighting
//
// This works in conjunction with strategic caching.  m_MainSurfVec, m_SurfVec, m_MainTessVec, m_TessVec are all
// cached to allow minimal updates according to the classified dirty flags.
//
// In the future, additional groups may be added.  In particular, a group that only updates the OpenGL visualization,
// or possibly handles SubSurface updating.
//
// So far, this is pretty clever and elegant.  It would have been great to have this foresight at the start and to
// better design around this sort of thing.  However, it is instead a band-aid being retrofitted into a complex system.
//
// The 'abomoination' is in using string matching of Parm names and Group names as the means of classification.  This
// is inherently fragile and implements policy at a low level (GeomBase) for information that won't be implemented
// until a much higher level (where are these Parms created, named, etc?).  A better approach from the start would be
// to set up some enums for the flag types and then add a property to each Parm to allow it to be classified where
// it is created (or init'ed).  It may still be reasonable to change over to that kind of approach in the future.
// An even more elegant approach would be to use the Geom class hierarchy to implement the appropriate UpdateXX()
// methods and pair them with the corresponding Parms at each level.  Changing to that sort of design is not possible
// to do in a way that is compatible with files created by EncodeXML (file format change).
//
// Rob McDonald 11/6/2020
//
void GeomBase::SetDirtyFlags( Parm* parm_ptr )
{
    if ( !parm_ptr )
    {
        return;
    }

    string gname = parm_ptr->GetGroupName();
    string pname = parm_ptr->GetName();

    if ( gname == string("XForm") && pname != string("Scale") && pname != string("Last_Scale") )
    {
        m_XFormDirty = true;
    }
    else if ( gname == string( "Attach") || gname == string( "Sym") )
    {
        m_XFormDirty = true;
    }
    else if ( gname == string("Shape") && ( pname == string("Tess_U") || pname == string("Tess_W") ) )
    {
        m_TessDirty = true;
    }
    else if ( gname == string("XSec") && pname == string("SectTess_U") )
    {
        m_TessDirty = true;
    }
    else if ( gname == string("EndCap") && pname == string("CapUMinTess") )
    {
        // This captures all geoms
        m_TessDirty = true;
    }
    else if ( pname == string("LECluster") || pname == string("TECluster") ||
              pname == string("InCluster") || pname == string("OutCluster") )
    {
        // This captures wings, propellers, and bodies of revolution clustering
        m_TessDirty = true;
    }
    else if ( gname == string("BBox") )
    {
        // Don't dirty anything, BBox Parms are output Parms and should not trigger updates.
    }
    else if ( gname == string("Index") )
    {
        m_HighlightDirty = true;
        // GeomXSec::m_ActiveXSec
        // WingGeom::m_ActiveAirfoil
        // PropGeom::m_ActiveBlade
    }
    else if ( gname.substr(0, 3) == string("Fea") )
    {
        // Do nothing here, FeaParts set their own internal dirty flags.  However, it is important to prevent
        // "Fea" tagged Parms from setting m_SurfDirty below as that will defeat FeaPart's ability to do more
        // fine-grained marking.
    }
    else
    {
        m_SurfDirty = true;
    }

}

void GeomBase::SetDirtyFlag( int dflag )
{
    if ( dflag == XFORM )
    {
        m_XFormDirty = true;
    }
    else if ( dflag == TESS )
    {
        m_TessDirty = true;
    }
    else if ( dflag == SURF )
    {
        m_SurfDirty = true;
    }
    else if ( dflag == HIGHLIGHT )
    {
        m_HighlightDirty = true;
    }
    else if ( dflag == GLOBAL_SCALE )
    {
        m_GlobalScaleDirty = true;
    }
}

//==== Parm Changed ====//
void GeomBase::ParmChanged( Parm* parm_ptr, int type )
{
    if ( parm_ptr )
    {
        m_UpdatedParmVec.push_back( parm_ptr->GetID() );

        SetDirtyFlags( parm_ptr );
    }

    if ( type == Parm::SET )
    {
        m_LateUpdateFlag = true;
        return;
    }

    //==== Check For Interactive Collision Detection When Alt Key Is Pressed ====//
    if ( type == Parm::SET_FROM_DEVICE )
    {
        if ( parm_ptr )
        {
            m_Vehicle->GetSnapToPtr()->PreventCollision( GetID(), parm_ptr->GetID() );
        }
    }

    Update();
    m_Vehicle->ParmChanged( parm_ptr, type );
    m_UpdatedParmVec.clear();
}

void GeomBase::ForceUpdate()
{
    m_LateUpdateFlag = true;
    m_Vehicle->Update();
    m_Vehicle->UpdateGUI();

    m_UpdatedParmVec.clear();
}

//==== Check If Parm Is In Updated ParmVec ====//
bool GeomBase::UpdatedParm( const string & id )
{
    for ( int i = 0 ; i < (int)m_UpdatedParmVec.size() ; i++ )
    {
        if ( m_UpdatedParmVec[i] == id )
            return true;
    }

    return false;
}

//==== Recursively Load ID and Childrens ID Into Vec  =====//
void GeomBase::LoadIDAndChildren( vector< string > & id_vec, bool check_display_flag )
{
    id_vec.push_back( m_ID );

    if ( check_display_flag && ! m_GuiDraw.GetDisplayChildrenFlag() )
    {
        return;
    }

    for ( int i = 0 ; i < ( int )m_ChildIDVec.size() ; i++ )
    {
        GeomBase* gPtr = m_Vehicle->FindGeom( m_ChildIDVec[i] );
        if ( gPtr )
        {
            gPtr->LoadIDAndChildren( id_vec, check_display_flag );
        }
    }
}

//==== Count Number Parents ===//
int GeomBase::CountParents( int count )
{
    GeomBase* parentPtr = m_Vehicle->FindGeom( m_ParentID );
    if ( parentPtr )
    {
        count++;
        return parentPtr->CountParents( count );
    }
    return count;
}

bool GeomBase::IsParentJoint()
{
    GeomBase* parentPtr = m_Vehicle->FindGeom( m_ParentID );
    if ( parentPtr )
    {
        HingeGeom* hingeParentPtr = dynamic_cast < HingeGeom* > ( parentPtr );
        if ( hingeParentPtr )
        {
            return true;
        }
    }
    return false;
}

//==== Changes parent of existing Geom, places in new parent's child vector & removes from old parent's child vector ====//
void GeomBase::ChangeParentID( const string& reparent_id, const string& sibling_id )
{
    // check if geom type can change parents
    if ( !m_Type.m_AdoptableFlag )
    {
        return;
    }

    // check if geom_id is valid
    Vehicle* veh = VehicleMgr.GetVehicle();

    // ensure geom_id different than new_parent_id
    if ( GetID() == reparent_id )
    {
        return;
    }

    // then check if the new_parent_id is valid, either as a geom or the vehicle
    Geom* reparent_geom = veh->FindGeom( reparent_id );
    Vehicle* reparent_veh = nullptr;
    if ( veh->GetID() == reparent_id )
    {
        reparent_veh = veh;
    }

    if ( !reparent_geom && !reparent_veh )
    {
        return;
    }

    // get the existing parent geom or veh
    Geom* old_parent_geom = veh->FindGeom( GetParentID() );
    Vehicle* old_parent_veh = nullptr;
    {
        old_parent_veh = veh;
    }

    // check that the parent_id is in fact valid
    if ( !old_parent_geom && !old_parent_veh )
    {
        return;
    }

    // 3 options for reparenting:
    // 1: go from parent geom to parent geom
    // 2: go from parent geom to parent veh
    // 3: go from parent veh  to parent geom
    if ( old_parent_geom && reparent_geom )
    {
        old_parent_geom->RemoveChildID( GetID() );
        reparent_geom->AddChildID( GetID(), sibling_id );
        SetParentID( reparent_id );

        Update();
    }
    else if ( old_parent_geom && reparent_veh )
    {
        old_parent_geom->RemoveChildID( GetID() );
        reparent_veh->AddTopGeomID( GetID(), sibling_id );
        SetParentID( string("NONE") );

        Update();
    }
    else if ( old_parent_veh && reparent_geom )
    {
        old_parent_veh->RemoveTopGeomID( GetID() );
        reparent_geom->AddChildID( GetID(), sibling_id );
        SetParentID( reparent_id );

        Update();
    }
}

string GeomBase::GetAncestorID( int gen )
{
    if ( gen == 0 )
    {
        return m_ID;
    }

    if ( gen == 1 )
    {
        return m_ParentID;
    }

    GeomBase* parentPtr = m_Vehicle->FindGeom( m_ParentID );
    if ( parentPtr )
    {
        return parentPtr->GetAncestorID( gen - 1 );
    }

    return string( "NONE" );
}

void GeomBase::BuildAncestorList( vector< string > &ancestors )
{
    ancestors.push_back( GetName() );

    GeomBase* parentPtr = m_Vehicle->FindGeom( m_ParentID );
    if ( parentPtr )
    {
        parentPtr->BuildAncestorList( ancestors );
    }
}

//==== Does ID_String Match Geom's ID? ====//
bool GeomBase::IsMatch( const string& id_string )
{
    if ( m_ID.compare( id_string ) != 0 )
    {
        return false;
    }

    return true;
}

//==== Add ID to Child Vec with optional argument to place adjacent to existing geom in childVec ====//
void GeomBase::AddChildID( const string &id, const string &insert_after_id )
{
    vector < string > ::iterator index_iter;
    index_iter = find( m_ChildIDVec.begin(), m_ChildIDVec.end(), insert_after_id );

    if ( insert_after_id.empty() || index_iter == m_ChildIDVec.end() )
    {
        m_ChildIDVec.push_back( id );
    }
    else
    {
        m_ChildIDVec.insert( index_iter + 1, id );
    }
}

//==== Remove ID From Child Vec ====//
void GeomBase::RemoveChildID( const string &id )
{
    vector_remove_val( m_ChildIDVec, id );
}

void GeomBase::RemoveStepChildID( const string &id )
{
    vector_remove_val( m_StepChildIDVec, id );
}

////==== Copy Geometry ====//
//void GeomBase::CopyFrom( GeomBase* geom )
//{
//  //xmlDocPtr doc = xmlNewDoc((const xmlChar *)"1.0");
//
//  xmlNodePtr root = xmlNewNode(nullptr,(const xmlChar *)"Vsp_Geometry");
//  //xmlDocSetRootElement(doc, root);
//
//
////    xmlNodePtr node = xmlNewNode(nullptr, BAD_CAST "Data" );
//  geom->EncodeGeom( root );
//  DecodeGeom( root );
//
////xmlSaveFormatFile("debug.xml", doc, 1);
//
//
//  xmlFreeNode( root );
//
//}

//==== Encode Data Into XML Data Struct ====//
xmlNodePtr GeomBase::EncodeXml( xmlNodePtr & node )
{
    ParmContainer::EncodeXml( node );
    xmlNodePtr geombase_node = xmlNewChild( node, nullptr, BAD_CAST "GeomBase", nullptr );
    if ( geombase_node )
    {
        XmlUtil::AddStringNode( geombase_node, "TypeName", m_Type.m_Name );
        XmlUtil::AddIntNode( geombase_node, "TypeID", m_Type.m_Type );
        XmlUtil::AddIntNode( geombase_node, "TypeFixed", m_Type.m_FixedFlag );
        XmlUtil::AddStringNode( geombase_node, "ParentID", m_ParentID );

        xmlNodePtr clist_node = xmlNewChild( geombase_node, nullptr, BAD_CAST "Child_List", nullptr );
        for ( int i = 0 ; i < ( int )m_ChildIDVec.size() ; i++ )
        {
            xmlNodePtr child_node = xmlNewChild( clist_node, nullptr, BAD_CAST "Child", nullptr );
            XmlUtil::AddStringNode( child_node, "ID", m_ChildIDVec[i] );
        }

        xmlNodePtr sclist_node = xmlNewChild( geombase_node, nullptr, BAD_CAST "Step_Child_List", nullptr );
        for ( int i = 0 ; i < ( int )m_StepChildIDVec.size() ; i++ )
        {
            xmlNodePtr schild_node = xmlNewChild( sclist_node, nullptr, BAD_CAST "Step_Child", nullptr );
            XmlUtil::AddStringNode( schild_node, "ID", m_StepChildIDVec[i] );
        }
    }
    return geombase_node;
}

//==== Decode Data From XML Data Struct ====//
xmlNodePtr GeomBase::DecodeXml( xmlNodePtr & node )
{
    ParmContainer::DecodeXml( node );
    xmlNodePtr geombase_node = XmlUtil::GetNode( node, "GeomBase", 0 );
    if ( geombase_node )
    {
        //m_Type.m_Name   = XmlUtil::FindString( child_node, "TypeName", m_Type.m_Name );
        //m_Type.m_Type   = XmlUtil::FindInt( child_node, "TypeID", m_Type.m_Type );
        m_Type.m_FixedFlag = !!XmlUtil::FindInt( geombase_node, "TypeFixed", m_Type.m_FixedFlag );
        m_ParentID = ParmMgr.RemapID( XmlUtil::FindString( geombase_node, "ParentID", m_ParentID ) );

        m_ChildIDVec.clear();

        xmlNodePtr cl_node = XmlUtil::GetNode( geombase_node, "Child_List", 0 );
        if ( cl_node )
        {
            int num_children = XmlUtil::GetNumNames( cl_node, "Child" );

            for ( int i = 0 ; i < num_children ; i++ )
            {
                xmlNodePtr n = XmlUtil::GetNode( cl_node, "Child", i );
                m_ChildIDVec.push_back( ParmMgr.RemapID( XmlUtil::FindString( n, "ID", string() ) ) );
            }
        }

        m_StepChildIDVec.clear();

        xmlNodePtr scl_node = XmlUtil::GetNode( geombase_node, "Step_Child_List", 0 );
        if ( scl_node )
        {
            int num_stepchildren = XmlUtil::GetNumNames( scl_node, "Step_Child" );

            for ( int i = 0 ; i < num_stepchildren ; i++ )
            {
                xmlNodePtr n = XmlUtil::GetNode( scl_node, "Step_Child", i );
                m_StepChildIDVec.push_back( ParmMgr.RemapID( XmlUtil::FindString( n, "ID", string() ) ) );
            }
        }
    }
    return geombase_node;
}

//===============================================================================//
//===============================================================================//
//===============================================================================//
//==== Constructor ====//
GeomXForm::GeomXForm( Vehicle* vehicle_ptr ) : GeomBase( vehicle_ptr )
{
    m_Name = "GeomXForm";
    m_Type.m_Type = XFORM_GEOM_TYPE;
    m_Type.m_Name = m_Name;

    m_XLoc.Init( "X_Location", "XForm", this, 0.0, -1.0e12, 1.0e12 );
    m_XLoc.SetDescript( "Global X Location" );
    m_YLoc.Init( "Y_Location", "XForm", this, 0.0, -1.0e12, 1.0e12 );
    m_YLoc.SetDescript( "Global Y Location" );
    m_ZLoc.Init( "Z_Location", "XForm", this, 0.0, -1.0e12, 1.0e12 );
    m_ZLoc.SetDescript( "Global Z Location" );
    m_XRot.Init( "X_Rotation", "XForm", this, 0.0, -180, 180 );
    m_XRot.SetDescript( "Global X Rotation" );
    m_YRot.Init( "Y_Rotation", "XForm", this, 0.0, -180, 180 );
    m_YRot.SetDescript( "Global Y Rotation" );
    m_ZRot.Init( "Z_Rotation", "XForm", this, 0.0,  -180, 180 );
    m_ZRot.SetDescript( "Global Z Rotation" );
    m_Origin.Init( "Origin", "XForm", this, 0.0, 0, 1 );
    m_Origin.SetDescript( "Rotation Origin" );

    m_XRelLoc.Init( "X_Rel_Location", "XForm", this, 0.0, -1.0e12, 1.0e12 );
    m_XRelLoc.SetDescript( "X Location Relative to Parent" );
    m_YRelLoc.Init( "Y_Rel_Location", "XForm", this, 0.0, -1.0e12, 1.0e12 );
    m_YRelLoc.SetDescript( "Y Location Relative to Parent" );
    m_ZRelLoc.Init( "Z_Rel_Location", "XForm", this, 0.0, -1.0e12, 1.0e12 );
    m_ZRelLoc.SetDescript( "Z Location Relative to Parent" );
    m_XRelRot.Init( "X_Rel_Rotation", "XForm", this, 0.0, -180, 180 );
    m_XRelRot.SetDescript( "X Rotation Relative to Parent" );
    m_YRelRot.Init( "Y_Rel_Rotation", "XForm", this, 0.0, -180, 180 );
    m_YRelRot.SetDescript( "Y Rotation Relative to Parent" );
    m_ZRelRot.Init( "Z_Rel_Rotation", "XForm", this, 0.0, -180, 180 );
    m_ZRelRot.SetDescript( "Z Rotation Relative to Parent" );

    // Attachment Parms
    m_AbsRelFlag.Init( "Abs_Or_Relitive_flag", "XForm", this, vsp::REL, vsp::ABS, vsp::REL );
    m_TransAttachFlag.Init( "Trans_Attach_Flag", "Attach", this, vsp::ATTACH_TRANS_NONE, vsp::ATTACH_TRANS_NONE, vsp::ATTACH_TRANS_NUM_TYPES - 1 );
    m_TransAttachFlag.SetDescript( "Determines relative translation coordinate system" );
    m_RotAttachFlag.Init( "Rots_Attach_Flag", "Attach", this, vsp::ATTACH_ROT_NONE, vsp::ATTACH_ROT_NONE, vsp::ATTACH_ROT_NUM_TYPES - 1 );
    m_RotAttachFlag.SetDescript( "Determines relative rotation axes" );
    m_ULoc.Init( "U_Attach_Location", "Attach", this, 0.0, 0.0, 1.0 );
    m_ULoc.SetDescript( "U Location on parent's surface" );
    m_U01.Init( "U_01", "Attach", this, true, false, true );
    m_U01.SetDescript( "The U value is specified in [0, 1] basis or [0, N] basis." );
    m_U0NLoc.Init( "U_Attach_Location0N", "Attach", this, 0, 0, 1e12 );
    m_U0NLoc.SetDescript( "U Location on parent's surface on [0,N] basis." );
    m_WLoc.Init( "V_Attach_Location", "Attach", this, 0.0, 0.0, 1.0 );
    m_WLoc.SetDescript( "V Location on parent's surface" );

    m_RLoc.Init( "R_Attach_Location", "Attach", this, 0.0, 0.0, 1.0 );
    m_RLoc.SetDescript( "R Location in parent's volume" );
    m_R01.Init( "R_01", "Attach", this, true, false, true );
    m_R01.SetDescript( "The R value is specified in [0, 1] basis or [0, N] basis." );
    m_R0NLoc.Init( "R_Attach_Location0N", "Attach", this, 0, 0, 1e12 );
    m_R0NLoc.SetDescript( "R Location in parent's volume on [0,N] basis." );
    m_SLoc.Init( "S_Attach_Location", "Attach", this, 0.5, 0.0, 1.0 );
    m_SLoc.SetDescript( "S Location in parent's volume" );
    m_TLoc.Init( "T_Attach_Location", "Attach", this, 0.5, 0.0, 1.0 );
    m_TLoc.SetDescript( "T Location in parent's volume" );

    m_LLoc.Init( "L_Attach_Location", "Attach", this, 0.0, 0.0, 1.0 );
    m_LLoc.SetDescript( "L Location in parent's volume" );
    m_L01.Init( "L_01", "Attach", this, true, false, true );
    m_L01.SetDescript( "The L value is specified in [0, 1] basis or dimensional basis." );
    m_L0LenLoc.Init( "L_Attach_Location0Len", "Attach", this, 0, 0, 1e12 );
    m_L0LenLoc.SetDescript( "L Location in parent's volume on [0,Len] basis." );
    m_MLoc.Init( "M_Attach_Location", "Attach", this, 0.5, 0.0, 1.0 );
    m_MLoc.SetDescript( "M Location in parent's volume" );
    m_NLoc.Init( "N_Attach_Location", "Attach", this, 0.5, 0.0, 1.0 );
    m_NLoc.SetDescript( "N Location in parent's volume" );

    m_EtaLoc.Init( "Eta_Attach_Location", "Attach", this, 0.0, 0.0, 1.0 );
    m_EtaLoc.SetDescript( "Eta Location in parent's volume" );

    m_Scale.Init( "Scale", "XForm", this, 1, 1.0e-12, 1.0e12 );
    m_Scale.SetDescript( "Scale Geometry Size" );

    m_LastScale.Init( "Last_Scale", "XForm", this, 1, 1.0e-12, 1.0e12 );
    m_LastScale.SetDescript( "Last Scale Value" );
    m_LastScale = m_Scale();

    m_ignoreAbsFlag = false;
    m_applyIgnoreAbsFlag = true;

    m_ModelMatrix.loadIdentity();
}

//==== Destructor ====//
GeomXForm::~GeomXForm()
{
}

//==== Update ====//
void GeomXForm::UpdateXForm()
{
    UpdateAttachParms();

    DeactivateXForms();

    ComposeAttachMatrix();

    ComposeModelMatrix();

    double axlen = 1.0;

    Vehicle *veh = VehicleMgr.GetVehicle();
    if ( veh )
    {
        axlen = veh->m_AxisLength();
    }

    m_AttachOrigin = m_AttachMatrix.xform( vec3d( 0.0, 0.0, 0.0 ) );

    m_AttachAxis.clear();
    m_AttachAxis.resize( 3 );
    for ( int i = 0; i < 3; i++ )
    {
        vec3d pt = vec3d( 0.0, 0.0, 0.0 );
        pt.v[i] = axlen;
        m_AttachAxis[i] = m_AttachMatrix.xform( pt );
    }
}

void GeomXForm::UpdateAttachParms()
{
    Geom* parent = m_Vehicle->FindGeom( GetParentID() );

    if ( parent )
    {
        if ( parent->isNonSurfaceType() )
        {
            return;
        }

        WingGeom* wing_parent = dynamic_cast < WingGeom * > ( parent );

        double umax = parent->GetMainUMapMax( 0 );
        double lmax = parent->GetMainSurfPtr( 0 )->GetLMax();

        m_U0NLoc.SetUpperLimit( umax );
        m_R0NLoc.SetUpperLimit( umax );
        m_L0LenLoc.SetUpperLimit( lmax );

        if ( m_U01.Get() )
        {
            m_U0NLoc.Set( m_ULoc() * umax );
        }
        else
        {
            double val = clamp( m_U0NLoc(), 0.0, umax );
            m_U0NLoc.Set( val );
            m_ULoc.Set( val / umax );
        }

        if ( m_R01.Get() )
        {
            m_R0NLoc.Set( m_RLoc() * umax );
        }
        else
        {
            double val = clamp( m_R0NLoc(), 0.0, umax );
            m_R0NLoc.Set( val );
            m_RLoc.Set( val / umax );
        }

        if ( m_L01.Get() )
        {
            m_L0LenLoc.Set( m_LLoc() * lmax );
        }
        else
        {
            double val = clamp( m_L0LenLoc(), 0.0, lmax );
            m_L0LenLoc.Set( val );
            m_LLoc.Set( val / lmax );
        }

        if ( wing_parent )
        {
            if ( m_TransAttachFlag() == vsp::ATTACH_TRANS_EtaMN || m_RotAttachFlag() == vsp::ATTACH_ROT_EtaMN ) // Eta is active.
            {
                if ( m_TransAttachFlag() != vsp::ATTACH_TRANS_UV && m_RotAttachFlag() != vsp::ATTACH_ROT_UV ) // U is not active.
                {
                    double u = wing_parent->EtatoU( m_EtaLoc() ) / umax;

                    double r;
                    r = u;

                    double l;
                    parent->ConvertRtoL( 0, r, l );

                    double s, t;
                    parent->ConvertLMNtoRST( 0, l, m_MLoc(), m_NLoc(), r, s, t );

                    double w;
                    if ( t < 0.5 )
                    {
                        w = 0.5 * s;
                    }
                    else
                    {
                        w = 1.0 - 0.5 * s;
                    }

                    m_ULoc.Set( u );
                    m_U0NLoc.Set( m_ULoc() * umax );
                    m_WLoc.Set( w );
                }

                if ( m_TransAttachFlag() != vsp::ATTACH_TRANS_RST && m_RotAttachFlag() != vsp::ATTACH_ROT_RST ) // R is not active
                {
                    double u, w;
                    u = wing_parent->EtatoU( m_EtaLoc() ) / umax;
                    w = m_WLoc();
                    double r, s, t;
                    r = u;

                    double l;
                    parent->ConvertRtoL( 0, r, l );

                    parent->ConvertLMNtoRST( 0, m_LLoc(), m_MLoc(), m_NLoc(), r, s, t );
                    m_RLoc.Set( r );
                    m_R0NLoc.Set( m_RLoc() * umax );
                    m_SLoc.Set( s );
                    m_TLoc.Set( t );
                }

                if ( m_TransAttachFlag() != vsp::ATTACH_TRANS_LMN && m_RotAttachFlag() != vsp::ATTACH_ROT_LMN ) // L is not active
                {
                    double u = wing_parent->EtatoU( m_EtaLoc() ) / umax;

                    double r;
                    r = u;

                    double l;
                    parent->ConvertRtoL( 0, r, l );

                    m_LLoc.Set( l );
                    m_L0LenLoc.Set( m_LLoc() * lmax );
                }

            }
            else // Eta is not active
            {
                if ( m_TransAttachFlag() == vsp::ATTACH_TRANS_UV || m_RotAttachFlag() == vsp::ATTACH_ROT_UV ) // UV is active
                {
                    m_EtaLoc = wing_parent->UtoEta( m_ULoc() * umax );
                }
                else if ( m_TransAttachFlag() == vsp::ATTACH_TRANS_RST || m_RotAttachFlag() == vsp::ATTACH_ROT_RST ) // RST is active
                {
                    double r = m_RLoc();
                    double u = r;
                    m_EtaLoc = wing_parent->UtoEta( u * umax );
                }
                else if ( m_TransAttachFlag() == vsp::ATTACH_TRANS_LMN || m_RotAttachFlag() == vsp::ATTACH_ROT_LMN ) // LMN is active
                {
                    double l = m_LLoc();
                    double r;
                    parent->ConvertLtoR( 0, l, r );
                    double u = r;
                    m_EtaLoc = wing_parent->UtoEta( u * umax );
                }
                else // Nothing is active, use U value anyway.
                {
                    m_EtaLoc = wing_parent->UtoEta( m_ULoc() * umax );
                }
            }
        }

        // If UV is active in either way and RST is not active in any way, compute RST from UV.
        if ( ( m_TransAttachFlag() == vsp::ATTACH_TRANS_UV || m_RotAttachFlag() == vsp::ATTACH_ROT_UV ) &&
             ( m_TransAttachFlag() != vsp::ATTACH_TRANS_RST && m_RotAttachFlag() != vsp::ATTACH_ROT_RST ) )
        {
            double u, w;
            u = m_ULoc();
            w = m_WLoc();
            double r, s, t;
            r = u;
            s = 2.0 * w;
            t = 0.0;
            if ( w > 0.5 )
            {
                s = 2.0 * ( 1.0 - w );
                t = 1.0;
            }

            m_RLoc.Set( r );
            m_R0NLoc.Set( m_RLoc() * umax );
            m_SLoc.Set( s );
            m_TLoc.Set( t );
        }

        // If UV is active in either way and LMN is not active in any way, compute LMN from UV.
        if ( ( m_TransAttachFlag() == vsp::ATTACH_TRANS_UV || m_RotAttachFlag() == vsp::ATTACH_ROT_UV ) &&
             ( m_TransAttachFlag() != vsp::ATTACH_TRANS_LMN && m_RotAttachFlag() != vsp::ATTACH_ROT_LMN ) )
        {
            double u, w;
            u = m_ULoc();
            w = m_WLoc();
            double r, s, t;
            r = u;
            s = 2.0 * w;
            t = 0.0;
            if ( w > 0.5 )
            {
                s = 2.0 * ( 1.0 - w );
                t = 1.0;
            }


            double l, m, n;

            parent->ConvertRSTtoLMN( 0, r, s, t, l, m, n );
            m_LLoc.Set( l );
            m_L0LenLoc.Set( m_LLoc() * lmax );
            m_MLoc.Set( m );
            m_NLoc.Set( n );
        }

        // If RST is active in either way and LMN is not active in any way, compute LMN from RST.
        if ( ( m_TransAttachFlag() == vsp::ATTACH_TRANS_RST || m_RotAttachFlag() == vsp::ATTACH_ROT_RST ) &&
             ( m_TransAttachFlag() != vsp::ATTACH_TRANS_LMN && m_RotAttachFlag() != vsp::ATTACH_ROT_LMN ) )
        {
            double l, m, n;
            parent->ConvertRSTtoLMN( 0, m_RLoc(), m_SLoc(), m_TLoc(), l, m, n );
            m_LLoc.Set( l );
            m_L0LenLoc.Set( m_LLoc() * lmax );
            m_MLoc.Set( m );
            m_NLoc.Set( n );
        }

        // If LMN is active in either way and RST is not active in any way, compute RST from LMN.
        if ( ( m_TransAttachFlag() == vsp::ATTACH_TRANS_LMN || m_RotAttachFlag() == vsp::ATTACH_ROT_LMN ) &&
             ( m_TransAttachFlag() != vsp::ATTACH_TRANS_RST && m_RotAttachFlag() != vsp::ATTACH_ROT_RST ) )
        {
            double r, s, t;
            parent->ConvertLMNtoRST( 0, m_LLoc(), m_MLoc(), m_NLoc(), r, s, t );
            m_RLoc.Set( r );
            m_R0NLoc.Set( m_RLoc() * umax );
            m_SLoc.Set( s );
            m_TLoc.Set( t );
        }

        // If RST is active in either way and UV is not active in any way, compute UV from RST.
        if ( ( m_TransAttachFlag() == vsp::ATTACH_TRANS_RST || m_RotAttachFlag() == vsp::ATTACH_ROT_RST ) &&
             ( m_TransAttachFlag() != vsp::ATTACH_TRANS_UV && m_RotAttachFlag() != vsp::ATTACH_ROT_UV ) )
        {
            double u, w;
            double r = m_RLoc();
            double s = m_SLoc();
            double t = m_TLoc();

            u = r;
            if ( t < 0.5 )
            {
                w = 0.5 * s;
            }
            else
            {
                w = 1.0 - 0.5 * s;
            }

            m_ULoc.Set( u );
            m_U0NLoc.Set( m_ULoc() * umax );
            m_WLoc.Set( w );
        }

        // If LMN is active in either way and UV is not active in any way, compute UV from LMN.
        if ( ( m_TransAttachFlag() == vsp::ATTACH_TRANS_LMN || m_RotAttachFlag() == vsp::ATTACH_ROT_LMN ) &&
             ( m_TransAttachFlag() != vsp::ATTACH_TRANS_UV && m_RotAttachFlag() != vsp::ATTACH_ROT_UV ) )
        {
            double u, w;
            double r, s, t;

            parent->ConvertLMNtoRST( 0, m_LLoc(), m_MLoc(), m_NLoc(), r, s, t );

            u = r;
            if ( t < 0.5 )
            {
                w = 0.5 * s;
            }
            else
            {
                w = 1.0 - 0.5 * s;
            }

            m_ULoc.Set( u );
            m_U0NLoc.Set( m_ULoc() * umax );
            m_WLoc.Set( w );
        }
    }
}

//==== Compose Model Matrix =====//
void GeomXForm::ComposeModelMatrix()
{
    m_ModelMatrix.loadIdentity();
    ComputeCenter();


    if (  m_AbsRelFlag() ==  vsp::REL || ( m_ignoreAbsFlag && m_applyIgnoreAbsFlag ) )
    {
        // Apply normal translations
        m_ModelMatrix.translatef( m_XRelLoc(), m_YRelLoc(), m_ZRelLoc() );
        // Translate to center, apply rotations, translate back
        m_ModelMatrix.translatef( m_Center.x(), m_Center.y(), m_Center.z() );
        m_ModelMatrix.rotateX( m_XRelRot() );
        m_ModelMatrix.rotateY( m_YRelRot() );
        m_ModelMatrix.rotateZ( m_ZRelRot() );
        m_ModelMatrix.translatef( -m_Center.x(), -m_Center.y(), -m_Center.z() );

        // Apply Attached Matrix to Relative Matrix
        m_ModelMatrix.postMult( m_AttachMatrix.data() );

        // Update Absolute Parameters
        double tempMat[16];
        m_ModelMatrix.getMat( tempMat );
        m_XLoc = tempMat[12];
        m_YLoc = tempMat[13];
        m_ZLoc = tempMat[14];

        vec3d angles = m_ModelMatrix.getAngles();
        m_XRot = angles.x();
        m_YRot = angles.y();
        m_ZRot = angles.z();
    }
    else if ( m_AbsRelFlag() ==  vsp::ABS )
    {
        // Apply normal translations
        m_ModelMatrix.translatef( m_XLoc(), m_YLoc(), m_ZLoc() );
        // Translate to center, apply rotations, translate back
        m_ModelMatrix.translatef( m_Center.x(), m_Center.y(), m_Center.z() );
        m_ModelMatrix.rotateX( m_XRot() );
        m_ModelMatrix.rotateY( m_YRot() );
        m_ModelMatrix.rotateZ( m_ZRot() );
        m_ModelMatrix.translatef( -m_Center.x(), -m_Center.y(), -m_Center.z() );

        // Update Relative Parameters
        Matrix4d attachedMat = m_AttachMatrix;
        double tempMat[16];
        attachedMat.affineInverse();
        attachedMat.matMult( m_ModelMatrix.data() );
        attachedMat.getMat( tempMat );
        m_XRelLoc = tempMat[12];
        m_YRelLoc = tempMat[13];
        m_ZRelLoc = tempMat[14];

        vec3d angles = attachedMat.getAngles();
        m_XRelRot = angles.x();
        m_YRelRot = angles.y();
        m_ZRelRot = angles.z();
    }

}

void GeomXForm::ComposeAttachMatrix()
{
    m_AttachMatrix.loadIdentity();

    Geom* parent = m_Vehicle->FindGeom( GetParentID() );

    if ( parent )
    {
        HingeGeom* hingeparent = dynamic_cast < HingeGeom* > ( parent );
        if ( hingeparent )
        {
            m_AttachMatrix = hingeparent->GetJointMatrix();
            return;
        }
    }

    // If both attachment flags set to none, return identity
    if ( m_TransAttachFlag() == vsp::ATTACH_TRANS_NONE && m_RotAttachFlag() == vsp::ATTACH_ROT_NONE )
    {
        return;
    }

    if ( parent )
    {
        Matrix4d transMat;
        Matrix4d rotMat;

        Matrix4d parentMat;
        parentMat = parent->getModelMatrix();

        if ( parent )
        {
            AuxiliaryGeom* auxthis = dynamic_cast < AuxiliaryGeom* > ( this );
            if ( auxthis )
            {
                if ( auxthis->m_AuxuliaryGeomMode() == vsp::AUX_GEOM_SUPER_CONE )
                {
                    bool axisaligned = auxthis->m_SCWorldAligned();

                    HumanGeom* humanparent = dynamic_cast < HumanGeom* > ( parent );
                    if ( humanparent )
                    {
                        parentMat = humanparent->GetDesignEyeMatrix( axisaligned );
                    }
                    else
                    {
                        if ( axisaligned )
                        {
                            const vec3d x( 1, 0, 0);
                            const vec3d y( 0, 1, 0 );
                            const vec3d z( 0, 0, 1 );
                            parentMat.setBasis( x, y, z );
                        }
                    }
                }
            }
        }

        double tempMat[16];
        parentMat.getMat( tempMat );

        bool revertCompTrans = false;
        bool revertCompRot = false;

        WingGeom* wing_parent = dynamic_cast < WingGeom * > ( parent );

        // Parent CompXXXCoordSys methods query the positioned m_SurfVec[0] surface,
        // not m_MainSurfVec[0].  Consequently, m_ModelMatrix is already implied in
        // these calculations and does not need to be applied again.
        if ( m_TransAttachFlag() == vsp::ATTACH_TRANS_UV )
        {
            if ( !( parent->CompTransCoordSys( 0, m_ULoc(), m_WLoc(), transMat )) )
            {
                revertCompTrans = true; // Any Geom without a surface reverts to the component matrix.
            }
        }

        if ( m_TransAttachFlag() == vsp::ATTACH_TRANS_RST )
        {
            if ( !( parent->CompTransCoordSysRST( 0, m_RLoc(), m_SLoc(), m_TLoc(), transMat )) )
            {
                revertCompTrans = true; // Any Geom without a surface reverts to the component matrix.
            }
        }

        if ( m_TransAttachFlag() == vsp::ATTACH_TRANS_LMN )
        {
            if ( !( parent->CompTransCoordSysLMN( 0, m_LLoc(), m_MLoc(), m_NLoc(), transMat )) )
            {
                revertCompTrans = true; // Any Geom without a surface reverts to the component matrix.
            }
        }

        if ( m_TransAttachFlag() == vsp::ATTACH_TRANS_EtaMN )
        {
            double l = m_EtaLoc();

            if ( wing_parent )
            {
                double umax = parent->GetMainUMapMax( 0 );
                double u = wing_parent->EtatoU( m_EtaLoc() ) / umax;

                double r = u;
                parent->ConvertRtoL( 0, r, l );
            }

            if ( !( parent->CompTransCoordSysLMN( 0, l, m_MLoc(), m_NLoc(), transMat )) )
            {
                revertCompTrans = true; // Any Geom without a surface reverts to the component matrix.
            }
        }

        if ( m_TransAttachFlag() == vsp::ATTACH_TRANS_COMP || revertCompTrans )
        {
            transMat.translatef( tempMat[12], tempMat[13], tempMat[14] );
        }

        if ( m_RotAttachFlag() == vsp::ATTACH_ROT_UV )
        {
            if ( !( parent->CompRotCoordSys( 0, m_ULoc(), m_WLoc(), rotMat )) )
            {
                revertCompRot = true; // Any Geom without a surface reverts to the component matrix.
            }
        }

        if ( m_RotAttachFlag() == vsp::ATTACH_ROT_RST )
        {
            if ( !( parent->CompRotCoordSysRST( 0, m_RLoc(), m_SLoc(), m_TLoc(), rotMat )) )
            {
                revertCompRot = true; // Any Geom without a surface reverts to the component matrix.
            }
        }

        if ( m_RotAttachFlag() == vsp::ATTACH_ROT_LMN )
        {
            if ( !( parent->CompRotCoordSysLMN( 0, m_LLoc(), m_MLoc(), m_NLoc(), rotMat )) )
            {
                revertCompRot = true; // Any Geom without a surface reverts to the component matrix.
            }
        }

        if ( m_RotAttachFlag() == vsp::ATTACH_ROT_EtaMN )
        {
            double l = m_EtaLoc();

            if ( wing_parent )
            {
                double umax = parent->GetMainUMapMax( 0 );
                double u = wing_parent->EtatoU( m_EtaLoc() ) / umax;

                double r = u;
                parent->ConvertRtoL( 0, r, l );
            }

            if ( !( parent->CompRotCoordSysLMN( 0, l, m_MLoc(), m_NLoc(), rotMat )) )
            {
                revertCompRot = true; // Any Geom without a surface reverts to the component matrix.
            }
        }

        if ( m_RotAttachFlag() == vsp::ATTACH_ROT_COMP || revertCompRot )
        {
            // Only take rotation matrix from parent so set translation part to zero
            tempMat[12] = tempMat[13] = tempMat[14] = 0;
            rotMat.initMat( tempMat );
        }

        transMat.matMult( rotMat.data() );
        m_AttachMatrix = transMat;
    }

}

//==== Set Rel or Abs Flag ====//
void GeomXForm::DeactivateXForms()
{
    // Deactivate non driving parms and Activate driving parms
    if ( m_AbsRelFlag() ==  vsp::REL  )
    {
        m_XLoc.Deactivate();
        m_YLoc.Deactivate();
        m_ZLoc.Deactivate();
        m_XRot.Deactivate();
        m_YRot.Deactivate();
        m_ZRot.Deactivate();

        m_XRelLoc.Activate();
        m_YRelLoc.Activate();
        m_ZRelLoc.Activate();
        m_XRelRot.Activate();
        m_YRelRot.Activate();
        m_ZRelRot.Activate();
    }
    else
    {
        m_XRelLoc.Deactivate();
        m_YRelLoc.Deactivate();
        m_ZRelLoc.Deactivate();
        m_XRelRot.Deactivate();
        m_YRelRot.Deactivate();
        m_ZRelRot.Deactivate();

        m_XLoc.Activate();
        m_YLoc.Activate();
        m_ZLoc.Activate();
        m_XRot.Activate();
        m_YRot.Activate();
        m_ZRot.Activate();
    }

    m_ULoc.Activate();
    m_U0NLoc.Activate();
    m_WLoc.Activate();
    m_RLoc.Activate();
    m_R0NLoc.Activate();
    m_SLoc.Activate();
    m_TLoc.Activate();
    m_LLoc.Activate();
    m_L0LenLoc.Activate();
    m_MLoc.Activate();
    m_NLoc.Activate();
    m_TransAttachFlag.Activate();
    m_RotAttachFlag.Activate();

    if ( IsParentJoint() )
    {
        m_ULoc.Deactivate();
        m_WLoc.Deactivate();
        m_RLoc.Deactivate();
        m_SLoc.Deactivate();
        m_TLoc.Deactivate();
        m_LLoc.Deactivate();
        m_MLoc.Deactivate();
        m_NLoc.Deactivate();
        m_TransAttachFlag.Deactivate();
        m_RotAttachFlag.Deactivate();
    }

    if ( m_U01() )
    {
        m_U0NLoc.Deactivate();
    }
    else
    {
        m_ULoc.Deactivate();
    }

    if ( m_R01() )
    {
        m_R0NLoc.Deactivate();
    }
    else
    {
        m_RLoc.Deactivate();
    }

    if ( m_L01() )
    {
        m_L0LenLoc.Deactivate();
    }
    else
    {
        m_LLoc.Deactivate();
    }

}

Matrix4d GeomXForm::GetAncestorAttachMatrix( int gen )
{
    Matrix4d atmat;

    if ( gen == -1 )
    {
        atmat.loadIdentity();
        return atmat;
    }

    if ( gen == 0 )
    {
        return getAttachMatrix();
    }

    string id = GetAncestorID( gen );

    GeomXForm* ancestPtr = m_Vehicle->FindGeom( id );
    if ( ancestPtr )
    {
        return ancestPtr->getAttachMatrix();
    }

    atmat.loadIdentity();
    return atmat;
}

Matrix4d GeomXForm::GetAncestorModelMatrix( int gen )
{
    Matrix4d atmat;

    if ( gen == -1 )
    {
        atmat.loadIdentity();
        return atmat;
    }

    if ( gen == 0 )
    {
        return getModelMatrix();
    }

    string id = GetAncestorID( gen );

    GeomXForm* ancestPtr = m_Vehicle->FindGeom( id );
    if ( ancestPtr )
    {
        return ancestPtr->getModelMatrix();
    }

    atmat.loadIdentity();
    return atmat;
}

//==== Reset Scale ====//
void GeomXForm::ResetScale()
{
    m_Scale = 1;
    Update();
}

//==== Accept Scale ====//
void GeomXForm::AcceptScale()
{
    m_Scale = 1;
    m_LastScale = 1;
}

////==== Compose Modeling Matrix ====//
//void Geom::compose_model_matrix()
//{
//  int i,j;
//  Matrix mat;
//  computeCenter();
//
//  vec3d tran = getTotalTranVec();
//  vec3d rot  = getTotalRotVec();
//
//  float tran_x = (float)tran.x();
//  float tran_y = (float)tran.y();
//  float tran_z = (float)tran.z();
//
//  float rot_x  = (float)rot.x();
//  float rot_y  = (float)rot.y();
//  float rot_z  = (float)rot.z();
//
//  mat.loadIdentity();
//  mat.translatef(  (float)center.x(), (float)center.y(), (float)center.z() );
//  mat.rotateX( rot_x );
//  mat.rotateY( rot_y );
//  mat.rotateZ( rot_z );
//  mat.translatef( -(float)center.x(),-(float)center.y(),-(float)center.z() );
//  mat.getMat( matFVec );
//
//  float xx = (matFVec[0]*tran_x + matFVec[1]*tran_y + matFVec[2]*tran_z);
//  float yy = (matFVec[4]*tran_x + matFVec[5]*tran_y + matFVec[6]*tran_z);
//  float zz = (matFVec[8]*tran_x + matFVec[9]*tran_y + matFVec[10]*tran_z);
//
//  mat.translatef( xx, yy, zz );
//
//  //==== Check if parent is rotation center ====//
//  if ( parentGeom )
//  {
//  if ( posAttachFlag == POS_ATTACH_MATRIX )
//  {
//      float* pm = parentGeom->getMat();
//      mat.postMult( pm );
//  }
//  }
//
//  mat.getMat( matFVec );
//
//  for( i = 0 ; i < 4 ; i++ )
//    for ( j = 0 ; j < 4 ; j++ )
//    {
//      model_mat[i][j] = matFVec[i*4 + j];
//    }
//
//  if (sym_code != NO_SYM)
//    compose_reflect_matrix();
//
//  //==== Update Children =====//
//  for ( i = 0 ; i < (int)childGeomVec.size() ; i++ )
//  {
//  childGeomVec[i]->compose_model_matrix();
//  }
//
//
//  update_xformed_bbox();
//}
//

//===============================================================================//
//===============================================================================//
//===============================================================================//
//==== Constructor ====//
Geom::Geom( Vehicle* vehicle_ptr ) : GeomXForm( vehicle_ptr )
{
    m_UpdateBlock = false;

    m_Name = "Geom";
    m_Type.m_Type = GEOM_GEOM_TYPE;
    m_Type.m_Name = m_Name;

    m_TessU.Init( "Tess_U", "Shape", this, 8, 2,  1000 );
    m_TessU.SetDescript( "Number of tessellated curves in the U direction" );
    m_TessW.Init( "Tess_W", "Shape", this, 9, 2,  1001 );
    m_TessW.SetDescript( "Number of tessellated curves in the W direction" );
    m_TessW.SetMultShift( 4, 1 );

    //==== Wake Parms ====//
    m_WakeActiveFlag.Init( "Wake", "Shape", this, false, 0, 1 );
    m_WakeActiveFlag.SetDescript( "Flag that indicates if this WingGeom has wakes attached" );

    m_WakeScale.Init( "WakeScale", "WakeSettings", this, 2.0, 1.0, 1.0e12 ); // decrease min???
    m_WakeScale.SetDescript( "Wake length scale" );

    m_WakeAngle.Init( "WakeAngle", "WakeSettings", this, 0.0, -89.9, 89.9 );
    m_WakeAngle.SetDescript( "Wake angle in degrees" );

    m_BbXLen.Init( "X_Len", "BBox", this, 0, 0, 1e12 );
    m_BbXLen.SetDescript( "X length of geom bounding box" );
    m_BbYLen.Init( "Y_Len", "BBox", this, 0, 0, 1e12 );
    m_BbYLen.SetDescript( "Y length of geom bounding box" );
    m_BbZLen.Init( "Z_Len", "BBox", this, 0, 0, 1e12 );
    m_BbZLen.SetDescript( "Z length of geom bounding box" );
    m_BbXMin.Init( "X_Min", "BBox", this, 0, -1e12, 1e12 );
    m_BbXMin.SetDescript( "Minimum X coordinate of geom bounding box" );
    m_BbYMin.Init( "Y_Min", "BBox", this, 0, -1e12, 1e12 );
    m_BbYMin.SetDescript( "Minimum Y coordinate of geom bounding box" );
    m_BbZMin.Init( "Z_Min", "BBox", this, 0, -1e12, 1e12 );
    m_BbZMin.SetDescript( "Minimum Z coordinate of geom bounding box" );

    m_SymAncestor.Init( "Sym_Ancestor", "Sym", this, 1, 0, 1e6 );
    m_SymAncestOriginFlag.Init( "Sym_Ancestor_Origin_Flag", "Sym", this, true, 0, 1 );
    m_SymPlanFlag.Init( "Sym_Planar_Flag", "Sym", this, 0, 0, SYM_XY | SYM_XZ | SYM_YZ );
    m_SymAxFlag.Init( "Sym_Axial_Flag", "Sym", this, 0, 0, SYM_ROT_Z );
    m_SymRotN.Init( "Sym_Rot_N", "Sym", this, 2, 2, 1000 );

    // Mass Properties
    m_Density.Init( "Density", "Mass_Props", this, 1, 0.0, 1e12 );
    m_Density.SetDescript("Volumetric density (mass/len^3)");
    m_MassArea.Init( "Mass_Area", "Mass_Props", this, 1, 0.0, 1e12 );
    m_MassArea.SetDescript("Areal density (mass/len^2)");
    m_MassPrior.Init( "Mass_Prior", "Mass_Props", this, 0, 0, 1e12 );
    m_MassPrior.SetDescript("Priority for volume overlap.  Highest priority wins.");
    m_ShellFlag.Init( "Shell_Flag", "Mass_Props", this, false, 0, 1 );
    m_ShellFlag.SetDescript("Flag to turn on/off area-based mass contribution");

    m_PointMass.Init( "PointMass", "Mass_Props", this, 0, 0,1e12 );
    m_CGx.Init( "CGx", "Mass_Props", this, 0, -1e12, 1e12 );
    m_CGy.Init( "CGy", "Mass_Props", this, 0, -1e12, 1e12 );
    m_CGz.Init( "CGz", "Mass_Props", this, 0, -1e12, 1e12 );
    m_Ixx.Init( "Ixx", "Mass_Props", this, 0, 0, 1e12 );
    m_Iyy.Init( "Iyy", "Mass_Props", this, 0, 0, 1e12 );
    m_Izz.Init( "Izz", "Mass_Props", this, 0, 0, 1e12 );
    m_Ixy.Init( "Ixy", "Mass_Props", this, 0, -1e12, 1e12 );
    m_Ixz.Init( "Ixz", "Mass_Props", this, 0, -1e12, 1e12 );
    m_Iyz.Init( "Iyz", "Mass_Props", this, 0, -1e12, 1e12 );


    // Negative Volume Properties
    m_NegativeVolumeFlag.Init( "Negative_Volume_Flag", "Negative_Volume_Props", this, false, 0, 1);

    // End Cap Options
    m_CapUMinOption.Init("CapUMinOption", "EndCap", this, NO_END_CAP, NO_END_CAP, NUM_END_CAP_OPTIONS-1);
    m_CapUMinOption.SetDescript("Type of End Cap on UMin end");

    m_CapUMinLength.Init( "CapUMinLength", "EndCap", this, 1, 0, 20 );
    m_CapUMinLength.SetDescript( "Scaled length of end cap" );
    m_CapUMinOffset.Init( "CapUMinOffset", "EndCap", this, 0, -20, 20 );
    m_CapUMinOffset.SetDescript( "Scaled offset of end cap" );
    m_CapUMinStrength.Init( "CapUMinStrength", "EndCap", this, 0.5, 0, 1 );
    m_CapUMinStrength.SetDescript( "Tangent strength of end cap" );
    m_CapUMinSweepFlag.Init( "CapUMinSweepFlag", "EndCap", this, 0, 0, 1 );
    m_CapUMinSweepFlag.SetDescript( "Flag to stretch end cap length for sweep" );

    m_CapUMinTess.Init("CapUMinTess", "EndCap", this, 3, 2, 51);
    m_CapUMinTess.SetDescript("Number of tessellated curves on capped ends");

    m_CapUMaxOption.Init("CapUMaxOption", "EndCap", this, NO_END_CAP, NO_END_CAP, NUM_END_CAP_OPTIONS-1);
    m_CapUMaxOption.SetDescript("Type of End Cap on UMax end");

    m_CapUMaxLength.Init( "CapUMaxLength", "EndCap", this, 1, 0, 20 );
    m_CapUMaxLength.SetDescript( "Scaled length of end cap" );
    m_CapUMaxOffset.Init( "CapUMaxOffset", "EndCap", this, 0, -20, 20 );
    m_CapUMaxOffset.SetDescript( "Scaled offset of end cap" );
    m_CapUMaxStrength.Init( "CapUMaxStrength", "EndCap", this, 0.5, 0, 1 );
    m_CapUMaxStrength.SetDescript( "Tangent strength of end cap" );
    m_CapUMaxSweepFlag.Init( "CapUMaxSweepFlag", "EndCap", this, 0, 0, 1 );
    m_CapUMaxSweepFlag.SetDescript( "Flag to stretch end cap length for sweep" );

    // Parasite Drag Parms
    m_PercLam.Init("PercLam", "ParasiteDragProps", this, 0, 0, 100 );
    m_PercLam.SetDescript("Percentage Laminar" );

    m_FFBodyEqnType.Init("FFBodyEqnType", "ParasiteDragProps", this, vsp::FF_B_HOERNER_STREAMBODY, vsp::FF_B_MANUAL, vsp::FF_B_JENKINSON_AFT_FUSE_NACELLE );
    m_FFBodyEqnType.SetDescript("Equation that defines the form factor of a body type surface included this Geom");

    m_FFWingEqnType.Init("FFWingEqnType", "ParasiteDragProps", this, vsp::FF_W_HOERNER, vsp::FF_W_MANUAL, vsp::FF_W_SCHEMENSKY_SUPERCRITICAL_AF );
    m_FFWingEqnType.SetDescript("Equation that defines the form factor of a wing type surface included this Geom");

    m_FFUser.Init("FFUser", "ParasiteDragProps", this, 1, -1, 10 );
    m_FFUser.SetDescript( "User Input Form Factor Value" );

    m_Q.Init("Q", "ParasiteDragProps", this, 1, 0, 3 );
    m_Q.SetDescript( "Interference Factor" );

    m_Roughness.Init("Roughness", "ParasiteDragProps", this, 0.0, 0, 1e3 );
    m_Roughness.SetDescript( "Roughness Height" );

    m_TeTwRatio.Init("TeTwRatio", "ParasiteDragProps", this, 1, -1, 1e6 );
    m_TeTwRatio.SetDescript("Temperature Ratio of Freestream to Wall" );

    m_TawTwRatio.Init("TawTwRatio", "ParasiteDragProps", this, 1, -1, 1e6 );
    m_TawTwRatio.SetDescript("Temperature Ratio of Ambient Wall to Wall" );

    m_GroupedAncestorGen.Init("IncorporatedGen", "ParasiteDragProps", this, 0, 0, 100);
    m_GroupedAncestorGen.SetDescript("Ancestor Generation that incorporates this geoms Swet");

    m_ExpandedListFlag.Init("ExpandedList", "ParasiteDragProps", this, false, false, true);
    m_ExpandedListFlag.SetDescript("Flag to determine whether or not this geom has a collapsed list in parasite drag");

    vector< string > set_name_vec = m_Vehicle->GetSetNameVec();
    m_SetFlags.resize( set_name_vec.size() );
    for ( int i = vsp::SET_NOT_SHOWN ; i < ( int )m_SetFlags.size() ; i++ )
    {
        m_SetFlags[i] = false;
    }
    m_SetFlags[vsp::SET_SHOWN] = true; // default to shown
    UpdateSets();

    // Geom needs at least one surf
    m_MainSurfVec.push_back( VspSurf() );

    currSourceID = 0;

    m_GeomProjectVec3d.resize( 3 );
    m_ForceXSecFlag = false;

    m_FeaStructCount = 0;
}
//==== Destructor ====//
Geom::~Geom()
{
    // Delete SubSurfaces
    for ( int i = 0 ; i < ( int )m_SubSurfVec.size() ; i++ )
    {
        delete m_SubSurfVec[i];
    }
    m_SubSurfVec.clear();

    // Delete FeaStructures
    for ( int i = 0; i < (int)m_FeaStructVec.size(); i++ )
    {
        delete m_FeaStructVec[i];
    }

    m_FeaStructVec.clear();
}

//==== Set Set Flag ====//
void Geom::SetSetFlag( int index, bool f )
{
    if ( index <= SET_ALL || index >= (int)m_SetFlags.size() )
    {
        return;
    }
    else if ( m_SetFlags[index] == f )
    {
        return; // flag is already set
    }
    else if ( index == SET_SHOWN )
    {
        // If added to SET_SHOWN, remove from SET_NOT_SHOWN. If removed
        // from SET_SHOWN, add to SET_NOT_SHOWN
        if ( m_SetFlags[SET_NOT_SHOWN] && f )
        {
            m_SetFlags[SET_NOT_SHOWN] = false;
        }
        else if ( !m_SetFlags[SET_NOT_SHOWN] && !f )
        {
            m_SetFlags[SET_NOT_SHOWN] = true;
        }
    }
    else if ( index == SET_NOT_SHOWN )
    {
        // If added to SET_NOT_SHOWN, remove from SET_SHOWN. If removed
        // from SET_NOT_SHOWN, add to SET_SHOWN
        if ( m_SetFlags[SET_SHOWN] && f )
        {
            m_SetFlags[SET_SHOWN] = false;
        }
        else if ( !m_SetFlags[SET_SHOWN] && !f )
        {
            m_SetFlags[SET_SHOWN] = true;
        }
    }

    m_SetFlags[index] = f;
}

//==== Get Set Flag ====//
bool Geom::GetSetFlag( int index ) const
{
    if ( index >= 0 && index < ( int )m_SetFlags.size() )
    {
        return m_SetFlags[index];
    }

    return false;
}


//==== Update Sets ====//
void Geom::UpdateSets()
{
    vector< string > set_name_vec = m_Vehicle->GetSetNameVec();

    if ( m_SetFlags.size() > set_name_vec.size() )
    {
        m_SetFlags.resize( set_name_vec.size() );
    }

    while ( m_SetFlags.size() < set_name_vec.size() )
    {
        m_SetFlags.push_back( false );
    }

    m_SetFlags[ SET_ALL ] = true;   // All

    if ( !m_SetFlags[SET_SHOWN] && !m_SetFlags[SET_NOT_SHOWN] )
    {
        // ensure Geom is always in SET_SHOWN or SET_NOT_SHOWN
        SetSetFlag( SET_SHOWN, true );
    }
}

void Geom::Show()
{
    m_SetFlags[SET_SHOWN] = true;
    m_SetFlags[SET_NOT_SHOWN] = false;
}

void Geom::NoShow()
{
    m_SetFlags[SET_SHOWN] = false;
    m_SetFlags[SET_NOT_SHOWN] = true;
}

//==== Copy Geometry ====//
void Geom::CopyFrom( Geom* geom )
{
    xmlNodePtr root = xmlNewNode( nullptr, ( const xmlChar * )"Vsp_Geometry" );

    geom->EncodeGeom( root );
    DecodeGeom( root );

    xmlFreeNode( root );
}

//==== Update ====//
void Geom::Update( bool fullupdate )
{
    if ( m_UpdateBlock )
        return;

    if ( GetType().m_Type == HINGE_GEOM_TYPE )
    {
        if ( m_XFormDirty )
        {
            m_SurfDirty = true;
        }
    }

    m_UpdateBlock = true;

    m_LateUpdateFlag = false;

    m_CappingDone = false;

    if ( m_SurfDirty )
    {
        double sf = m_Scale() / m_LastScale();
        if ( sf != 1.0 )
        {
            Scale();
        }
    }

    UpdateSets();

    UpdateCopyParms();

    if ( m_XFormDirty )
        UpdateXForm();

    if ( m_SurfDirty )
        UpdateSurf();       // Must be implemented by subclass.

    if ( m_SurfDirty )
        UpdateEndCaps();

    if ( m_SurfDirty )
    {
        for ( int i = 0; i < m_MainSurfVec.size(); i++ )
        {
            m_MainSurfVec[i].InitUMapping();
            m_MainSurfVec[i].BuildLCurve();
        }
    }

    if ( m_SurfDirty )
        UpdateEngine();

    if ( fullupdate )
    {
        if ( m_SurfDirty )
            UpdateFeatureLines();
    }

    if ( m_SurfDirty )
    {
        UpdateFlags();  // Needs to be after m_MainSurfVec is populated, but before m_SurfVec
    }

    // Sets cluster parameters on m_MainSurfVec[0] for wings etc.
    // Needs to be before m_MainSurfVec is copied to m_SurfVec.
    if ( m_SurfDirty || m_TessDirty )
    {
        UpdatePreTess();
    }

    if ( m_XFormDirty || m_SurfDirty )
    {
        UpdateSymmAttach();  // Needs to happen for both XForm and Surf updates.

        // More aggressive optimization could eliminate this call, but at the complexity
        // of a lazy update any time m_SurfVec is accessed.  At this point, the speed
        // does not appear to be worth the complexity.  Typical worst case for this call
        // is 0.1 sec.  Typical cost is two orders smaller.
        UpdateSurfVec();
    }

    if ( fullupdate ) // Option to make FitModel and similar things faster.
    {
        for ( int i = 0 ; i < ( int )m_SubSurfVec.size() ; i++ )
        {
            m_SubSurfVec[i]->Update();  // Can be protected by m_SurfDirty, except for call to UpdateDrawObj - perhaps should be split out.  Some may depend on m_SurfVec, but could be switched to m_MainSurfVec instead.
        }

        if ( m_XFormDirty || m_SurfDirty ) // Everything except m_TessDirty
        {
            for ( int i = 0; i < (int)m_FeaStructVec.size(); i++ )
            {
                m_FeaStructVec[i]->SetDirtyFlag();
            }
        }
    }

    if ( fullupdate )
    {
        // Tessellate MainSurfVec
        if ( m_SurfDirty || m_TessDirty )
        {
            if ( m_GuiDraw.GetDisplayType() == DISPLAY_TYPE::DISPLAY_BEZIER )
            {
                UpdateMainTessVec();
            }
            else
            {
                UpdateMainDegenGeomPreview();
            }
        }

        // Copy Tessellation for symmetry and XForm
        if ( m_XFormDirty || m_SurfDirty || m_TessDirty )
        {
            if ( m_GuiDraw.GetDisplayType() == DISPLAY_TYPE::DISPLAY_BEZIER )
            {
                UpdateTessVec();
            }
            else
            {
                UpdateDegenGeomPreview();
            }
        }
    }

    if ( m_XFormDirty || m_SurfDirty )
    {
        UpdateBBox();  // Needs to happen for both XForm and Surf updates.
    }

    if ( fullupdate )
    {
        if ( m_XFormDirty || m_SurfDirty || m_TessDirty )
        {
            UpdateDrawObj();  // Needs to happen for both XForm and Surf updates.
        }

        if ( m_XFormDirty || m_SurfDirty || m_HighlightDirty )
        {
            UpdateHighlightDrawObj();
        }
    }

    m_UpdateXForm = false;
    if ( m_XFormDirty )
        m_UpdateXForm = true;
    m_XFormDirty = false;

    m_UpdateSurf = false;
    if ( m_SurfDirty )
        m_UpdateSurf = true;
    m_SurfDirty = false;

    m_TessDirty = false;

    m_HighlightDirty = false;

    m_GlobalScaleDirty = false;

    UpdateChildren( fullupdate );
    UpdateStepChildren( fullupdate );

    m_UpdatedParmVec.clear();
    m_UpdateBlock = false;
}

void Geom::GetUWTess01( const int &indx, vector < double > &u, vector < double > &w )
{
    vector< vector< vec3d > > pnts;
    vector< vector< vec3d > > norms;
    vector< vector< vec3d > > uw_pnts;

    UpdateTesselate( m_SurfVec[indx], m_CapUMinSuccess[ m_SurfIndxVec[indx] ], m_CapUMaxSuccess[ m_SurfIndxVec[indx] ], false, pnts, norms,
                     uw_pnts);

    double umx = GetUMax( indx );
    double wmx = GetWMax( indx );

    u.resize( uw_pnts.size() );
    for ( int i = 0; i < uw_pnts.size(); i++ )
    {
        u[i] = uw_pnts[i][0].x() / umx;
    }

    w.resize( uw_pnts[0].size() );
    for ( int j = 0; j < uw_pnts[0].size(); j++ )
    {
        w[j] = uw_pnts[0][j].y() / wmx;
    }
}

void Geom::UpdateTesselate( const VspSurf &surf, bool capUMinSuccess, bool capUMaxSuccess, bool degen, vector< vector< vec3d > > &pnts, vector< vector< vec3d > > &norms, vector< vector< vec3d > > &uw_pnts, const int & n_ref ) const
{
    surf.Tesselate( m_TessU(), m_TessW(), pnts, norms, uw_pnts, m_CapUMinTess(), m_TessU(), degen, n_ref );
}

void Geom::UpdateSplitTesselate( const VspSurf &surf, bool capUMinSuccess, bool capUMaxSuccess, vector< vector< vector< vec3d > > > &pnts, vector< vector< vector< vec3d > > > &norms ) const
{
    surf.SplitTesselate( m_TessU(), m_TessW(), pnts, norms, m_CapUMinTess(), m_TessU() );
}

void Geom::UpdateEndCaps()
{
    if ( m_CappingDone )
    {
        return;
    }
    m_CappingDone = true;

    unsigned int nmain = m_MainSurfVec.size();
    m_CapUMinSuccess.resize( nmain );
    m_CapUMaxSuccess.resize( nmain );

    // cycle through all vspsurfs, check if wing type then cap using new Code-Eli cap surface creator
    for ( int i = 0; i < nmain; i++ )
    {
        m_CapUMinSuccess[i] = false;
        m_CapUMaxSuccess[i] = false;

        vec3d ptoff;
        // NOTE: These return a bool that is true if it modified the surface to create a cap
        m_CapUMinSuccess[i] = m_MainSurfVec[i].CapUMin(m_CapUMinOption(), m_CapUMinLength(), m_CapUMinStrength(), m_CapUMinOffset(), ptoff, m_CapUMinSweepFlag());
        m_CapUMaxSuccess[i] = m_MainSurfVec[i].CapUMax(m_CapUMaxOption(), m_CapUMaxLength(), m_CapUMaxStrength(), m_CapUMaxOffset(), ptoff, m_CapUMaxSweepFlag());
    }

    switch( m_CapUMinOption() ){
        case NO_END_CAP:
        case FLAT_END_CAP:
            m_CapUMinLength = 1.0;
            m_CapUMinOffset = 0.0;
            m_CapUMinStrength = 0.5;
            break;
        case ROUND_END_CAP:
            m_CapUMinStrength = 1.0;
            break;
        case EDGE_END_CAP:
            m_CapUMinStrength = 0.0;
            break;
        case SHARP_END_CAP:
            break;
        case ROUND_EXT_END_CAP_NONE:
            m_CapUMinStrength = 1.0;
            break;
        case ROUND_EXT_END_CAP_LE:
            m_CapUMinStrength = 1.0;
            break;
        case ROUND_EXT_END_CAP_TE:
            m_CapUMinStrength = 1.0;
            break;
        case ROUND_EXT_END_CAP_BOTH:
            m_CapUMinStrength = 1.0;
            break;
    }

    switch( m_CapUMaxOption() ){
        case NO_END_CAP:
        case FLAT_END_CAP:
            m_CapUMaxLength = 1.0;
            m_CapUMaxOffset = 0.0;
            m_CapUMaxStrength = 0.5;
            break;
        case ROUND_END_CAP:
            m_CapUMaxStrength = 1.0;
            break;
        case EDGE_END_CAP:
            m_CapUMaxStrength = 0.0;
            break;
        case SHARP_END_CAP:
            break;
        case ROUND_EXT_END_CAP_NONE:
            m_CapUMaxStrength = 1.0;
            break;
        case ROUND_EXT_END_CAP_LE:
            m_CapUMaxStrength = 1.0;
            break;
        case ROUND_EXT_END_CAP_TE:
            m_CapUMaxStrength = 1.0;
            break;
        case ROUND_EXT_END_CAP_BOTH:
            m_CapUMaxStrength = 1.0;
            break;
    }
}

void Geom::UpdateFeatureLines( )
{
    for ( int i = 0; i < m_MainSurfVec.size(); i++ )
    {
        m_MainSurfVec[i].BuildFeatureLines( m_ForceXSecFlag );
    }
}

void Geom::UpdateSymmAttach()
{
    unsigned int num_surf = GetNumTotalSurfs();
    m_SurfIndxVec.clear();
    m_SurfSymmMap.clear();
    m_SurfCopyIndx.clear();
    m_FlipNormalVec.clear();
    m_SurfIndxVec.resize( num_surf, -1 );
    m_FlipNormalVec.resize( num_surf, false );
    m_SurfSymmMap.resize( num_surf );
    m_SurfCopyIndx.resize( num_surf );

    int num_main = GetNumMainSurfs();
    for ( int i = 0 ; i < ( int )num_main ; i++ )
    {
        m_SurfIndxVec[i] = i;
        m_FlipNormalVec[i] = GetMainFlipNormal( i );
        m_SurfSymmMap[ m_SurfIndxVec[i] ].push_back( i );
        m_SurfCopyIndx[i] = 0;
    }

    m_TransMatVec.resize( num_surf, Matrix4d() );
    // Compute Relative Translation Matrix
    Matrix4d symmOriginMat;
    Matrix4d relTrans;
    if ( m_SymAncestOriginFlag() )
    {
        symmOriginMat = GetAncestorAttachMatrix( m_SymAncestor() - 1 );
    }
    else
    {
        symmOriginMat = GetAncestorModelMatrix( m_SymAncestor() - 1 );
    }
    relTrans = symmOriginMat;
    relTrans.affineInverse();
    relTrans.matMult( m_ModelMatrix.data() );

    for ( int i = 0 ; i < ( int )m_TransMatVec.size() ; i++ )
    {
        m_TransMatVec[i].initMat( relTrans.data() );
    }

    // Copy main surfs
    int symFlag = GetSymFlag();
    if ( symFlag != 0 )
    {
        int numShifts = -1;
        Matrix4d Ref; // Reflection Matrix
        Matrix4d Ref_Orig; // Original Reflection Matrix
        Matrix4d Rel; // Relative Transformation matrix with Reflection applied ( this is for the main surfaces )

        double angle = ( 360 ) / ( double )m_SymRotN();
        int currentIndex = GetNumMainSurfs();
        bool radial = false;

        for ( int i = 0 ; i < GetNumSymFlags() ; i ++ ) // Loop through each of the set sym flags
        {
            // Find next set sym flag
            while ( true )
            {
                numShifts++;
                if ( ( ( symFlag >> numShifts ) & ( 1 << 0 ) ) || numShifts > SYM_NUM_TYPES )
                {
                    break;
                }
            }

            // Create Reflection Matrix
            if ( ( 1 << numShifts ) == SYM_XY )
            {
                Ref.loadXYRef();
            }
            else if ( ( 1 << numShifts ) == SYM_XZ )
            {
                Ref.loadXZRef();
            }
            else if ( ( 1 << numShifts ) == SYM_YZ )
            {
                Ref.loadYZRef();
            }
            else if ( ( 1 << numShifts ) == SYM_ROT_X )
            {
                Ref.loadIdentity();
                Ref.rotateX( angle );
                Ref_Orig = Ref;
                radial = true;
            }
            else if ( ( 1 << numShifts ) == SYM_ROT_Y )
            {
                Ref.loadIdentity();
                Ref.rotateY( angle );
                Ref_Orig = Ref;
                radial = true;
            }
            else if ( ( 1 << numShifts ) == SYM_ROT_Z )
            {
                Ref.loadIdentity();
                Ref.rotateZ( angle );
                Ref_Orig = Ref;
                radial = true;
            }

            // number of additional surfaces for a single reflection ( for rotational reflections it is m_SymRotN-1 times this number
            int numAddSurfs = currentIndex;
            int addIndex = 0;

            for ( int j = currentIndex ; j < currentIndex + numAddSurfs ; j++ )
            {
                if ( radial ) // rotational reflection
                {
                    for ( int k = 0 ; k < m_SymRotN() - 1 ; k++ )
                    {
                        m_SurfIndxVec[j + k * numAddSurfs] = m_SurfIndxVec[j - currentIndex];
                        m_FlipNormalVec[j + k * numAddSurfs] = m_FlipNormalVec[j - currentIndex];
                        m_SurfCopyIndx[j + k * numAddSurfs] = m_SurfSymmMap[ m_SurfIndxVec[j + k * numAddSurfs] ].size();
                        m_SurfSymmMap[ m_SurfIndxVec[j + k * numAddSurfs] ].push_back( j + k * numAddSurfs );
                        m_TransMatVec[j + k * numAddSurfs].initMat( m_TransMatVec[j - currentIndex].data() );
                        m_TransMatVec[j + k * numAddSurfs].postMult( Ref.data() ); // Apply Reflection

                        // Increment rotation by the angle
                        Ref.postMult( Ref_Orig.data() );
                        addIndex++;
                    }
                    // Reset reflection matrices to the beginning angle
                    Ref = Ref_Orig;
                }
                else
                {
                    m_FlipNormalVec[j] = m_FlipNormalVec[j - currentIndex];
                    m_FlipNormalVec[j] = !m_FlipNormalVec[j];
                    m_SurfIndxVec[j] = m_SurfIndxVec[j - currentIndex];
                    m_SurfCopyIndx[j] = m_SurfSymmMap[ m_SurfIndxVec[j] ].size();
                    m_SurfSymmMap[ m_SurfIndxVec[ j ] ].push_back( j );
                    m_TransMatVec[j].initMat( m_TransMatVec[j - currentIndex].data() );
                    m_TransMatVec[j].postMult( Ref.data() ); // Apply Reflection
                    addIndex++;
                }
            }

            currentIndex += addIndex;
            radial = false;
        }
    }

    Matrix4d retrun_relTrans = relTrans;
    retrun_relTrans.affineInverse();

    m_FeaTransMatVec.clear();
    m_FeaTransMatVec.resize( num_surf );

    //==== Save Transformation Matrix and Apply Transformations ====//
    for ( int i = 0 ; i < num_surf ; i++ )
    {
        m_TransMatVec[i].postMult( symmOriginMat.data() );

        m_FeaTransMatVec[i] = m_TransMatVec[i];
        m_FeaTransMatVec[i].matMult( retrun_relTrans.data() ); // m_FeaTransMatVec does not include the relTrans matrix
    }

    m_SymmTransMatVec.clear();
    if ( !m_SurfSymmMap.empty() )
    {
        int nsymm = m_SurfSymmMap[0].size();
        m_SymmTransMatVec.resize( nsymm );

        for ( int isymm = 0; isymm < nsymm; isymm++ )
        {
            m_SymmTransMatVec[ isymm ] = m_TransMatVec[ m_SurfSymmMap[0][isymm] ];
        }
    }
}

void Geom::UpdateSurfVec()
{
    ApplySymm( m_MainSurfVec, m_SurfVec );
}

// This routine performs a recursive search for grandchildren of a Geom that are symmetrical about the grandparent.
// It will traverse down the full ancestry tree of the grandparent.  It only runs when the grandparent's XForm was
// updated.  When it finds a match, it will mark XFormDirty in the descendent and trigger an update.
void Geom::UpdateGrandChildren( Geom* grandparent, bool fullupdate )
{
    if ( grandparent )
    {
        // Loop over all children of this generation.
        for ( int i = 0 ; i < (int)m_ChildIDVec.size() ; i++ )
        {
            Geom* child = m_Vehicle->FindGeom( m_ChildIDVec[i] );
            // Child uses symmetry of some sort
            if ( child && child->GetSymFlag() != 0 )
            {
                // Symmetry is about the grandparent
                string ancestorid = child->GetAncestorID( child->m_SymAncestor() - 1 );
                if ( grandparent->GetID() == ancestorid )
                {
                    child->m_XFormDirty = true;

                    // Ignore the abs location values and only use rel values for children so a child
                    // with abs button selected stays attached to parent if the parent moves
                    child->m_ignoreAbsFlag = true;
                    child->Update( fullupdate );
                    child->m_ignoreAbsFlag = false;
                }

                // Recurse to child's children.
                child->UpdateGrandChildren( grandparent, fullupdate );
            }
        }
    }
}

//==== Check If Children Exist and Update ====//
void Geom::UpdateChildren( bool fullupdate )
{
    vector< string > updated_child_vec;
    for ( int i = 0 ; i < (int)m_ChildIDVec.size() ; i++ )
    {
        Geom* child = m_Vehicle->FindGeom( m_ChildIDVec[i] );
        if ( child )
        {
            // Child is attached in some way.
            if ( ( child->m_RotAttachFlag() != vsp::ATTACH_ROT_NONE ) ||
                 ( child->m_TransAttachFlag() != vsp::ATTACH_TRANS_NONE ) )
            {
                // Parent was XFormed
                if ( m_UpdateXForm )
                {
                    child->m_XFormDirty = true;
                }
                // Parent surf changed and child is UV attached
                else if ( m_UpdateSurf &&
                      ( ( child->m_RotAttachFlag() >= vsp::ATTACH_ROT_UV ) ||
                        ( child->m_TransAttachFlag() >= vsp::ATTACH_TRANS_UV ) ) )
                {
                    child->m_XFormDirty = true;
                }
            }

            // We are a hinge, children are force attached.
            if ( GetType().m_Type == HINGE_GEOM_TYPE )
            {
                child->m_XFormDirty = true;
            }

            // If the child is a Conformal or Auxiliary
            if ( child->GetType().m_Type == CONFORMAL_GEOM_TYPE ||
                 child->GetType().m_Type == AUXILIARY_GEOM_TYPE)
            {
                // m_UpdateXForm here captures changes in parent symmetry.
                if ( m_UpdateSurf || m_UpdateXForm )
                {
                    child->m_SurfDirty = true;
                }
            }

            // Ignore the abs location values and only use rel values for children so a child
            // with abs button selected stays attached to parent if the parent moves
            child->m_ignoreAbsFlag = true;
            child->Update( fullupdate );
            child->m_ignoreAbsFlag = false;

            updated_child_vec.push_back( m_ChildIDVec[i] );
        }
    }

    // Update Children Vec
    m_ChildIDVec = updated_child_vec;

    if ( m_UpdateXForm )
    {
        UpdateGrandChildren( this, fullupdate );
    }
}

void Geom::UpdateStepChildren( bool fullupdate )
{
    vector< string > updated_child_vec;
    for ( int i = 0 ; i < (int)m_StepChildIDVec.size() ; i++ )
    {
        Geom* child = m_Vehicle->FindGeom( m_StepChildIDVec[i] );
        if ( child )
        {
            // Parent was XFormed
            if ( m_UpdateXForm || m_UpdateSurf )
            {
                child->m_XFormDirty = true;
                child->m_SurfDirty = true;
            }

            // Ignore the abs location values and only use rel values for children so a child
            // with abs button selected stays attached to parent if the parent moves
            child->m_ignoreAbsFlag = true;
            child->Update( fullupdate );
            child->m_ignoreAbsFlag = false;

            updated_child_vec.push_back( m_StepChildIDVec[i] );
        }
    }

    // Update Children Vec
    m_StepChildIDVec = updated_child_vec;
}

void Geom::UpdateBBox( )
{
    BndBox empty_box;
    UpdateBBox( 0, empty_box );
}

void Geom::UpdateBBox( int istart, const BndBox & start_box )
{
    BndBox new_box = start_box;

    //==== Load Bounding Box ====//
    BndBox main_box;
    for ( int i = istart ; i < GetNumMainSurfs() ; i++ )
    {
        BndBox bb;
        m_MainSurfVec[i].GetBoundingBox( bb );
        if ( !bb.IsEmpty() )
        {
            main_box.Update( bb );
        }
    }

    if ( !main_box.IsEmpty() )
    {
        BndBox placed_box;
        for ( int isymm = 0; isymm < m_SymmTransMatVec.size(); isymm++ )
        {
            BndBox bb = main_box;
            bb.Transform( m_SymmTransMatVec[ isymm ] );
            placed_box.Update( bb );
        }
        new_box.Update( placed_box );
    }

    // If the surface vec size is zero ( like blank geom )
    // set bbox to zero size

    if ( !GetNumTotalSurfs() )
    {
        new_box.Update( vec3d(0,0,0) );
    }

    if ( new_box != m_BBox )
    {
        m_BbXLen = new_box.GetMax( 0 ) - new_box.GetMin( 0 );
        m_BbYLen = new_box.GetMax( 1 ) - new_box.GetMin( 1 );
        m_BbZLen = new_box.GetMax( 2 ) - new_box.GetMin( 2 );

        m_BbXMin = new_box.GetMin( 0 );
        m_BbYMin = new_box.GetMin( 1 );
        m_BbZMin = new_box.GetMin( 2 );

        m_BBox = new_box;
        m_ScaleIndependentBBox = m_BBox;
    }
}

//Sets cfd surf types to negative if or normal depending on the state of the GUI negative button
void Geom::UpdateFlags( )
{
    if ( isNonSurfaceType() )
    {
        // m_MainSurfVec.size() == 0
        return;
    }

    for( int i = 0; i < (int)m_MainSurfVec.size(); i++ )
    {
        if ( m_MainSurfVec[i].GetSurfCfdType() == vsp::CFD_TRANSPARENT )
        {
            // Transparent surfaces (actuator disks) can't be negative
            continue;
        }

        if ( m_MainSurfVec[i].GetSurfCfdType() == vsp::CFD_NORMAL && m_NegativeVolumeFlag.Get() )
        {
            m_MainSurfVec[i].SetSurfCfdType( vsp::CFD_NEGATIVE );
        }
        else if ( m_MainSurfVec[i].GetSurfCfdType() == vsp::CFD_NEGATIVE && !m_NegativeVolumeFlag.Get() )
        {
            m_MainSurfVec[i].SetSurfCfdType( vsp::CFD_NORMAL );
        }
    }
}

void Geom::WriteFeatureLinesDXF( FILE * file_name, const BndBox &dxfbox )
{
    double tol = 10e-2; // Feature line Tessellation tolerance

    bool color = m_Vehicle->m_DXFColorFlag.Get();

    // Bounding box diagonal, used to separate multi-view drawings
    vec3d shiftvec = dxfbox.GetMax() - dxfbox.GetMin();

    // Shift the vehicle bounding box to align with the +x, +y, +z axes at the orgin
    vec3d to_orgin = GetVecToOrgin( dxfbox );

    for ( int i = 0; i < GetNumTotalSurfs(); i++ )
    {
        vector < vector < vec3d > > allflines, allflines1, allflines2, allflines3, allflines4;

        unsigned int nu = m_SurfVec[i].GetNumUFeature();
        unsigned int nw = m_SurfVec[i].GetNumWFeature();
        allflines.resize( nu + nw );
        for ( int j = 0; j < nu; j++ )
        {
            m_SurfVec[i].TessUFeatureLine( j, allflines[j], tol );

            // Shift Feature Lines back near the orgin for multi-view case:
            if ( m_Vehicle->m_DXF2D3DFlag() != vsp::DIMENSION_SET::SET_3D )
            {
                for ( unsigned int k = 0; k < allflines[j].size(); k++ )
                {
                    allflines[j][k].offset_x( -to_orgin.x() );
                    allflines[j][k].offset_y( -to_orgin.y() );
                    allflines[j][k].offset_z( -to_orgin.z() );
                }
            }
        }
        for ( int j = 0; j < nw; j++ )
        {
            m_SurfVec[i].TessWFeatureLine( j, allflines[j + nu], tol );

            // Shift Feature Lines back near the orgin for multi-view case:
            if ( m_Vehicle->m_DXF2D3DFlag() != vsp::DIMENSION_SET::SET_3D )
            {
                for ( unsigned int k = 0; k < allflines[j + nu].size(); k++ )
                {
                    allflines[j + nu][k].offset_x( -to_orgin.x() );
                    allflines[j + nu][k].offset_y( -to_orgin.y() );
                    allflines[j + nu][k].offset_z( -to_orgin.z() );
                }
            }
        }


        // Add layers:
        string layer = m_Name + string( "_Surf[" ) + to_string( i ) + string( "]" );

        if ( m_Vehicle->m_DXF2D3DFlag() == vsp::DIMENSION_SET::SET_3D )
        {
            WriteDXFPolylines3D( file_name, allflines, layer, color, m_Vehicle->m_ColorCount );
            m_Vehicle->m_ColorCount++;
        }
        else if ( m_Vehicle->m_DXF2D3DFlag() == vsp::DIMENSION_SET::SET_2D )
        {
            if ( m_Vehicle->m_DXF2DView() == vsp::VIEW_NUM::VIEW_1 )
            {
                allflines1 = allflines;
                FeatureLinesManipulate( allflines1, m_Vehicle->m_DXF4View1(), m_Vehicle->m_DXF4View1_rot(), shiftvec );
                WriteDXFPolylines2D( file_name, allflines1, layer, color, m_Vehicle->m_ColorCount );
                m_Vehicle->m_ColorCount++;
            }
            else if ( m_Vehicle->m_DXF2DView() == vsp::VIEW_NUM::VIEW_2HOR )
            {
                allflines1 = allflines;
                FeatureLinesManipulate( allflines1, m_Vehicle->m_DXF4View1(), m_Vehicle->m_DXF4View1_rot(), shiftvec );
                FeatureLinesShift( allflines1, shiftvec, VIEW_SHIFT::LEFT, m_Vehicle->m_DXF4View1_rot(), 0 );
                string layer_v1 = layer + "_v1";

                allflines2 = allflines;
                FeatureLinesManipulate( allflines2, m_Vehicle->m_DXF4View2(), m_Vehicle->m_DXF4View2_rot(), shiftvec );
                FeatureLinesShift( allflines2, shiftvec, VIEW_SHIFT::RIGHT, m_Vehicle->m_DXF4View2_rot(), 0 );
                string layer_v2 = layer + "_v2";

                WriteDXFPolylines2D( file_name, allflines1, layer_v1, color, m_Vehicle->m_ColorCount );
                m_Vehicle->m_ColorCount++;

                WriteDXFPolylines2D( file_name, allflines2, layer_v2, color, m_Vehicle->m_ColorCount );
                m_Vehicle->m_ColorCount++;
            }
            else if ( m_Vehicle->m_DXF2DView() == vsp::VIEW_NUM::VIEW_2VER )
            {
                allflines1 = allflines;
                FeatureLinesManipulate( allflines1, m_Vehicle->m_DXF4View1(), m_Vehicle->m_DXF4View1_rot(), shiftvec );
                FeatureLinesShift( allflines1, shiftvec, VIEW_SHIFT::UP, m_Vehicle->m_DXF4View1_rot(), 0 );

                allflines3 = allflines;
                FeatureLinesManipulate( allflines3, m_Vehicle->m_DXF4View3(), m_Vehicle->m_DXF4View3_rot(), shiftvec );
                FeatureLinesShift( allflines3, shiftvec, VIEW_SHIFT::DOWN, m_Vehicle->m_DXF4View3_rot(), 0 );

                string layer_v1 = layer + "_v1";
                string layer_v2 = layer + "_v2";

                WriteDXFPolylines2D( file_name, allflines1, layer_v1, color, m_Vehicle->m_ColorCount );
                m_Vehicle->m_ColorCount++;

                WriteDXFPolylines2D( file_name, allflines3, layer_v2, color, m_Vehicle->m_ColorCount );
                m_Vehicle->m_ColorCount++;
            }
            else if ( m_Vehicle->m_DXF2DView() == vsp::VIEW_NUM::VIEW_4 )
            {
                allflines1 = allflines;
                FeatureLinesManipulate( allflines1, m_Vehicle->m_DXF4View1(), m_Vehicle->m_DXF4View1_rot(), shiftvec );
                FeatureLinesShift( allflines1, shiftvec, VIEW_SHIFT::UP, m_Vehicle->m_DXF4View1_rot(), m_Vehicle->m_DXF4View2_rot() );
                FeatureLinesShift( allflines1, shiftvec, VIEW_SHIFT::LEFT, m_Vehicle->m_DXF4View1_rot(), m_Vehicle->m_DXF4View3_rot() );

                allflines2 = allflines;
                FeatureLinesManipulate( allflines2, m_Vehicle->m_DXF4View2(), m_Vehicle->m_DXF4View2_rot(), shiftvec );
                FeatureLinesShift( allflines2, shiftvec, VIEW_SHIFT::UP, m_Vehicle->m_DXF4View2_rot(), m_Vehicle->m_DXF4View1_rot() );
                FeatureLinesShift( allflines2, shiftvec, VIEW_SHIFT::RIGHT, m_Vehicle->m_DXF4View2_rot(), m_Vehicle->m_DXF4View4_rot() );

                allflines3 = allflines;
                FeatureLinesManipulate( allflines3, m_Vehicle->m_DXF4View3(), m_Vehicle->m_DXF4View3_rot(), shiftvec );
                FeatureLinesShift( allflines3, shiftvec, VIEW_SHIFT::DOWN, m_Vehicle->m_DXF4View3_rot(), m_Vehicle->m_DXF4View4_rot() );
                FeatureLinesShift( allflines3, shiftvec, VIEW_SHIFT::LEFT, m_Vehicle->m_DXF4View3_rot(), m_Vehicle->m_DXF4View1_rot() );

                allflines4 = allflines;
                FeatureLinesManipulate( allflines4, m_Vehicle->m_DXF4View4(), m_Vehicle->m_DXF4View4_rot(), shiftvec );
                FeatureLinesShift( allflines4, shiftvec, VIEW_SHIFT::DOWN, m_Vehicle->m_DXF4View4_rot(), m_Vehicle->m_DXF4View3_rot() );
                FeatureLinesShift( allflines4, shiftvec, VIEW_SHIFT::RIGHT, m_Vehicle->m_DXF4View4_rot(), m_Vehicle->m_DXF4View2_rot() );

                string layer_v1 = layer + "_v1";
                string layer_v2 = layer + "_v2";
                string layer_v3 = layer + "_v3";
                string layer_v4 = layer + "_v4";

                WriteDXFPolylines2D( file_name, allflines1, layer_v1, color, m_Vehicle->m_ColorCount );
                m_Vehicle->m_ColorCount++;

                WriteDXFPolylines2D( file_name, allflines2, layer_v2, color, m_Vehicle->m_ColorCount );
                m_Vehicle->m_ColorCount++;

                WriteDXFPolylines2D( file_name, allflines3, layer_v3, color, m_Vehicle->m_ColorCount );
                m_Vehicle->m_ColorCount++;

                WriteDXFPolylines2D( file_name, allflines4, layer_v4, color, m_Vehicle->m_ColorCount );
                m_Vehicle->m_ColorCount++;
            }
        }
    }
}

void Geom::WriteProjectionLinesDXF( FILE * file_name, const BndBox &dxfbox )
{
    bool color = m_Vehicle->m_DXFColorFlag.Get();

    // Bounding box diagonal, used to separate multi-view drawings
    vec3d shiftvec = dxfbox.GetMax() - dxfbox.GetMin();

    // Shift the vehicle bounding box to align with the +x, +y, +z axes at the orgin
    vec3d to_orgin = GetVecToOrgin( dxfbox );

    // Add layers:
    string projectionlayer  = m_Name + string( "_Projection" );

    if ( m_Vehicle->m_DXF2D3DFlag() == vsp::DIMENSION_SET::SET_3D )
    {
        return; // Projection lines not valid for 3D view
    }
    else if ( m_Vehicle->m_DXF2D3DFlag() == vsp::DIMENSION_SET::SET_2D )
    {
        if ( m_Vehicle->m_DXF2DView() == vsp::VIEW_NUM::VIEW_1 )
        {
            vector < vector < vec3d > > projectionvec = GetGeomProjectionLines( m_Vehicle->m_DXF4View1(), to_orgin );

            if ( projectionvec.size() > 0 )
            {
                FeatureLinesManipulate( projectionvec, m_Vehicle->m_DXF4View1(), m_Vehicle->m_DXF4View1_rot(), shiftvec );
                WriteDXFPolylines2D( file_name, projectionvec, projectionlayer, color, m_Vehicle->m_ColorCount );
                m_Vehicle->m_ColorCount++;
            }
        }
        else if ( m_Vehicle->m_DXF2DView() == vsp::VIEW_NUM::VIEW_2HOR )
        {
            vector < vector < vec3d > > projectionvec1 = GetGeomProjectionLines( m_Vehicle->m_DXF4View1(), to_orgin );

            if ( projectionvec1.size() > 0 )
            {
                FeatureLinesManipulate( projectionvec1, m_Vehicle->m_DXF4View1(), m_Vehicle->m_DXF4View1_rot(), shiftvec );
                FeatureLinesShift( projectionvec1, shiftvec, VIEW_SHIFT::LEFT, m_Vehicle->m_DXF4View1_rot(), 0 );
                string projectionlayer_v1 = projectionlayer + "_v1";

                WriteDXFPolylines2D( file_name, projectionvec1, projectionlayer_v1, color, m_Vehicle->m_ColorCount );
                m_Vehicle->m_ColorCount++;
            }

            vector < vector < vec3d > > projectionvec2 = GetGeomProjectionLines( m_Vehicle->m_DXF4View2(), to_orgin );

            if ( projectionvec2.size() > 0 )
            {
                FeatureLinesManipulate( projectionvec2, m_Vehicle->m_DXF4View2(), m_Vehicle->m_DXF4View2_rot(), shiftvec );
                FeatureLinesShift( projectionvec2, shiftvec, VIEW_SHIFT::RIGHT, m_Vehicle->m_DXF4View2_rot(), 0 );
                string projectionlayer_v2 = projectionlayer + "_v2";

                WriteDXFPolylines2D( file_name, projectionvec2, projectionlayer_v2, color, m_Vehicle->m_ColorCount );
                m_Vehicle->m_ColorCount++;
            }
        }
        else if ( m_Vehicle->m_DXF2DView() == vsp::VIEW_NUM::VIEW_2VER )
        {
            vector < vector < vec3d > > projectionvec1 = GetGeomProjectionLines( m_Vehicle->m_DXF4View1(), to_orgin );

            if ( projectionvec1.size() > 0 )
            {
                FeatureLinesManipulate( projectionvec1, m_Vehicle->m_DXF4View1(), m_Vehicle->m_DXF4View1_rot(), shiftvec );
                FeatureLinesShift( projectionvec1, shiftvec, VIEW_SHIFT::UP, m_Vehicle->m_DXF4View1_rot(), 0 );
                string projectionlayer_v1 = projectionlayer + "_v1";

                WriteDXFPolylines2D( file_name, projectionvec1, projectionlayer_v1, color, m_Vehicle->m_ColorCount );
                m_Vehicle->m_ColorCount++;
            }

            vector < vector < vec3d > > projectionvec3 = GetGeomProjectionLines( m_Vehicle->m_DXF4View3(), to_orgin );

            if ( projectionvec3.size() > 0 )
            {
                FeatureLinesManipulate( projectionvec3, m_Vehicle->m_DXF4View3(), m_Vehicle->m_DXF4View3_rot(), shiftvec );
                FeatureLinesShift( projectionvec3, shiftvec, VIEW_SHIFT::DOWN, m_Vehicle->m_DXF4View3_rot(), 0 );
                string projectionlayer_v2 = projectionlayer + "_v2";

                WriteDXFPolylines2D( file_name, projectionvec3, projectionlayer_v2, color, m_Vehicle->m_ColorCount );
                m_Vehicle->m_ColorCount++;
            }
        }
        else if ( m_Vehicle->m_DXF2DView() == vsp::VIEW_NUM::VIEW_4 )
        {
            vector < vector < vec3d > > projectionvec1 = GetGeomProjectionLines( m_Vehicle->m_DXF4View1(), to_orgin );

            if ( projectionvec1.size() > 0 )
            {
                FeatureLinesManipulate( projectionvec1, m_Vehicle->m_DXF4View1(), m_Vehicle->m_DXF4View1_rot(), shiftvec );
                FeatureLinesShift( projectionvec1, shiftvec, VIEW_SHIFT::UP, m_Vehicle->m_DXF4View1_rot(), m_Vehicle->m_DXF4View2_rot() );
                FeatureLinesShift( projectionvec1, shiftvec, VIEW_SHIFT::LEFT, m_Vehicle->m_DXF4View1_rot(), m_Vehicle->m_DXF4View3_rot() );
                string projectionlayer_v1 = projectionlayer + "_v1";

                WriteDXFPolylines2D( file_name, projectionvec1, projectionlayer_v1, color, m_Vehicle->m_ColorCount );
                m_Vehicle->m_ColorCount++;
            }

            vector < vector < vec3d > > projectionvec2 = GetGeomProjectionLines( m_Vehicle->m_DXF4View2(), to_orgin );

            if ( projectionvec2.size() > 0 )
            {
                FeatureLinesManipulate( projectionvec2, m_Vehicle->m_DXF4View2(), m_Vehicle->m_DXF4View2_rot(), shiftvec );
                FeatureLinesShift( projectionvec2, shiftvec, VIEW_SHIFT::UP, m_Vehicle->m_DXF4View2_rot(), m_Vehicle->m_DXF4View1_rot() );
                FeatureLinesShift( projectionvec2, shiftvec, VIEW_SHIFT::RIGHT, m_Vehicle->m_DXF4View2_rot(), m_Vehicle->m_DXF4View4_rot() );
                string projectionlayer_v2 = projectionlayer + "_v2";

                WriteDXFPolylines2D( file_name, projectionvec2, projectionlayer_v2, color, m_Vehicle->m_ColorCount );
                m_Vehicle->m_ColorCount++;
            }

            vector < vector < vec3d > > projectionvec3 = GetGeomProjectionLines( m_Vehicle->m_DXF4View3(), to_orgin );

            if ( projectionvec3.size() > 0 )
            {
                FeatureLinesManipulate( projectionvec3, m_Vehicle->m_DXF4View3(), m_Vehicle->m_DXF4View3_rot(), shiftvec );
                FeatureLinesShift( projectionvec3, shiftvec, VIEW_SHIFT::DOWN, m_Vehicle->m_DXF4View3_rot(), m_Vehicle->m_DXF4View4_rot() );
                FeatureLinesShift( projectionvec3, shiftvec, VIEW_SHIFT::LEFT, m_Vehicle->m_DXF4View3_rot(), m_Vehicle->m_DXF4View1_rot() );
                string projectionlayer_v3 = projectionlayer + "_v3";

                WriteDXFPolylines2D( file_name, projectionvec3, projectionlayer_v3, color, m_Vehicle->m_ColorCount );
                m_Vehicle->m_ColorCount++;
            }

            vector < vector < vec3d > > projectionvec4 = GetGeomProjectionLines( m_Vehicle->m_DXF4View4(), to_orgin );

            if ( projectionvec4.size() > 0 )
            {
                FeatureLinesManipulate( projectionvec4, m_Vehicle->m_DXF4View4(), m_Vehicle->m_DXF4View4_rot(), shiftvec );
                FeatureLinesShift( projectionvec4, shiftvec, VIEW_SHIFT::DOWN, m_Vehicle->m_DXF4View4_rot(), m_Vehicle->m_DXF4View3_rot() );
                FeatureLinesShift( projectionvec4, shiftvec, VIEW_SHIFT::RIGHT, m_Vehicle->m_DXF4View4_rot(), m_Vehicle->m_DXF4View2_rot() );
                string projectionlayer_v4 = projectionlayer + "_v4";

                WriteDXFPolylines2D( file_name, projectionvec4, projectionlayer_v4, color, m_Vehicle->m_ColorCount );
                m_Vehicle->m_ColorCount++;
            }
        }
    }
}

vector< vector < vec3d > > Geom::GetGeomProjectionLines( int view, const vec3d &offset )
{
    vector < vector < vec3d > > PathVec;

    if ( view == vsp::VIEW_TYPE::VIEW_LEFT || view == vsp::VIEW_TYPE::VIEW_RIGHT )
    {
        PathVec = m_GeomProjectVec3d[vsp::Y_DIR]; // Y axis projection
    }
    else if ( view == vsp::VIEW_TYPE::VIEW_FRONT || view == vsp::VIEW_TYPE::VIEW_REAR )
    {
        PathVec = m_GeomProjectVec3d[vsp::X_DIR]; // X axis projection
    }
    else if ( view == vsp::VIEW_TYPE::VIEW_TOP || view == vsp::VIEW_TYPE::VIEW_BOTTOM )
    {
        PathVec = m_GeomProjectVec3d[vsp::Z_DIR]; // Z axis projection
    }

    for ( int j = 0; j < PathVec.size(); j++ )
    {
        // Shift Projection Lines back near the orgin:
        for ( unsigned int k = 0; k < PathVec[j].size(); k++ )
        {
            PathVec[j][k].offset_x( -offset.x() );
            PathVec[j][k].offset_y( -offset.y() );
            PathVec[j][k].offset_z( -offset.z() );
        }
    }

    return PathVec;
}

void Geom::WriteFeatureLinesSVG( xmlNodePtr root, const BndBox &svgbox )
{
    double tol = 10e-2; // Feature line Tessellation tolerance

    // Bounding box diagonal, used to separate multi-view drawings
    vec3d shiftvec = svgbox.GetMax() - svgbox.GetMin();

    // Shift the vehicle bounding box to align with the +x, +y, +z axes at the orgin
    vec3d to_orgin = GetVecToOrgin( svgbox );

    for ( int i = 0 ; i < GetNumTotalSurfs() ; i++ )
    {
        vector < vector < vec3d > > allflines, allflines1, allflines2, allflines3, allflines4;

        unsigned int nu = m_SurfVec[i].GetNumUFeature();
        unsigned int nw = m_SurfVec[i].GetNumWFeature();
        allflines.resize( nw + nu );
        for( int j = 0; j < nw; j++ )
        {
            m_SurfVec[i].TessWFeatureLine( j, allflines[ j ], tol );

            // To Do: multiple view ports instead of shifting feature lines in a single view port

            // Shift Feature Lines back near the orgin:
            for ( unsigned int k = 0; k < allflines[j].size(); k++ )
            {
                allflines[j][k].offset_x( -to_orgin.x() );
                allflines[j][k].offset_y( -to_orgin.y() );
                allflines[j][k].offset_z( -to_orgin.z() );
            }
        }
        for( int j = 0; j < nu; j++ )
        {
            m_SurfVec[i].TessUFeatureLine( j, allflines[ j + nw ], tol );

            // Shift Feature Lines back near the orgin :
            for ( unsigned int k = 0; k < allflines[j + nw].size(); k++ )
            {
                allflines[j + nw][k].offset_x( -to_orgin.x() );
                allflines[j + nw][k].offset_y( -to_orgin.y() );
                allflines[j + nw][k].offset_z( -to_orgin.z() );
            }
        }

        if ( m_Vehicle->m_SVGView() == vsp::VIEW_NUM::VIEW_1 )
        {
            allflines1 = allflines;
            FeatureLinesManipulate( allflines1, m_Vehicle->m_SVGView1(), m_Vehicle->m_SVGView1_rot(), shiftvec );
            WriteSVGPolylines2D( root, allflines1, svgbox );
        }
        else if ( m_Vehicle->m_SVGView() == vsp::VIEW_NUM::VIEW_2HOR )
        {
            allflines1 = allflines;
            FeatureLinesManipulate( allflines1, m_Vehicle->m_SVGView1(), m_Vehicle->m_SVGView1_rot(), shiftvec );
            FeatureLinesShift( allflines1, shiftvec, VIEW_SHIFT::LEFT, m_Vehicle->m_SVGView1_rot(), 0 );

            allflines2 = allflines;
            FeatureLinesManipulate( allflines2, m_Vehicle->m_SVGView2(), m_Vehicle->m_SVGView2_rot(), shiftvec );
            FeatureLinesShift( allflines2, shiftvec, VIEW_SHIFT::RIGHT, m_Vehicle->m_SVGView2_rot(), 0 );

            WriteSVGPolylines2D( root, allflines1, svgbox );
            WriteSVGPolylines2D( root, allflines2, svgbox );
        }
        else if ( m_Vehicle->m_SVGView() == vsp::VIEW_NUM::VIEW_2VER )
        {
            allflines1 = allflines;
            FeatureLinesManipulate( allflines1, m_Vehicle->m_SVGView1(), m_Vehicle->m_SVGView1_rot(), shiftvec );
            FeatureLinesShift( allflines1, shiftvec, VIEW_SHIFT::UP, m_Vehicle->m_SVGView1_rot(), 0 );

            allflines3 = allflines;
            FeatureLinesManipulate( allflines3, m_Vehicle->m_SVGView3(), m_Vehicle->m_SVGView3_rot(), shiftvec );
            FeatureLinesShift( allflines3, shiftvec, VIEW_SHIFT::DOWN, m_Vehicle->m_SVGView3_rot(), 0 );

            WriteSVGPolylines2D( root, allflines1, svgbox );
            WriteSVGPolylines2D( root, allflines3, svgbox );
        }
        else if ( m_Vehicle->m_SVGView() == vsp::VIEW_NUM::VIEW_4 )
        {
            allflines1 = allflines;

            FeatureLinesManipulate( allflines1, m_Vehicle->m_SVGView1(), m_Vehicle->m_SVGView1_rot(), shiftvec );
            FeatureLinesShift( allflines1, shiftvec, VIEW_SHIFT::UP, m_Vehicle->m_SVGView1_rot(), m_Vehicle->m_SVGView2_rot() );
            FeatureLinesShift( allflines1, shiftvec, VIEW_SHIFT::LEFT, m_Vehicle->m_SVGView1_rot(), m_Vehicle->m_SVGView3_rot() );

            allflines2 = allflines;
            FeatureLinesManipulate( allflines2, m_Vehicle->m_SVGView2(), m_Vehicle->m_SVGView2_rot(), shiftvec );
            FeatureLinesShift( allflines2, shiftvec, VIEW_SHIFT::UP, m_Vehicle->m_SVGView2_rot(), m_Vehicle->m_SVGView1_rot() );
            FeatureLinesShift( allflines2, shiftvec, VIEW_SHIFT::RIGHT, m_Vehicle->m_SVGView2_rot(), m_Vehicle->m_SVGView4_rot() );

            allflines3 = allflines;
            FeatureLinesManipulate( allflines3, m_Vehicle->m_SVGView3(), m_Vehicle->m_SVGView3_rot(), shiftvec );
            FeatureLinesShift( allflines3, shiftvec, VIEW_SHIFT::DOWN, m_Vehicle->m_SVGView3_rot(), m_Vehicle->m_SVGView4_rot() );
            FeatureLinesShift( allflines3, shiftvec, VIEW_SHIFT::LEFT, m_Vehicle->m_SVGView3_rot(), m_Vehicle->m_SVGView1_rot() );

            allflines4 = allflines;
            FeatureLinesManipulate( allflines4, m_Vehicle->m_SVGView4(), m_Vehicle->m_SVGView4_rot(), shiftvec );
            FeatureLinesShift( allflines4, shiftvec, VIEW_SHIFT::DOWN, m_Vehicle->m_SVGView4_rot(), m_Vehicle->m_SVGView3_rot() );
            FeatureLinesShift( allflines4, shiftvec, VIEW_SHIFT::RIGHT, m_Vehicle->m_SVGView4_rot(), m_Vehicle->m_SVGView2_rot() );

            WriteSVGPolylines2D( root, allflines1, svgbox );
            WriteSVGPolylines2D( root, allflines2, svgbox );
            WriteSVGPolylines2D( root, allflines3, svgbox );
            WriteSVGPolylines2D( root, allflines4, svgbox );
        }
    }
}

void Geom::WriteProjectionLinesSVG( xmlNodePtr root, const BndBox &svgbox )
{
    // Bounding box diagonal, used to separate multi-view drawings
    vec3d shiftvec = svgbox.GetMax() - svgbox.GetMin();

    // Shift the vehicle bounding box to align with the +x, +y, +z axes at the orgin
    vec3d to_orgin = GetVecToOrgin( svgbox );

    if ( m_Vehicle->m_SVGView() == vsp::VIEW_NUM::VIEW_1 )
    {
        vector < vector < vec3d > > projectionvec = GetGeomProjectionLines( m_Vehicle->m_SVGView1(), to_orgin );

        FeatureLinesManipulate( projectionvec, m_Vehicle->m_SVGView1(), m_Vehicle->m_SVGView1_rot(), shiftvec );
        WriteSVGPolylines2D( root, projectionvec, svgbox );
    }
    else if ( m_Vehicle->m_SVGView() == vsp::VIEW_NUM::VIEW_2HOR )
    {
        vector < vector < vec3d > > projectionvec1 = GetGeomProjectionLines( m_Vehicle->m_SVGView1(), to_orgin );

        FeatureLinesManipulate( projectionvec1, m_Vehicle->m_SVGView1(), m_Vehicle->m_SVGView1_rot(), shiftvec );
        FeatureLinesShift( projectionvec1, shiftvec, VIEW_SHIFT::LEFT, m_Vehicle->m_SVGView1_rot(), 0 );
        WriteSVGPolylines2D( root, projectionvec1, svgbox );

        vector < vector < vec3d > > projectionvec2 = GetGeomProjectionLines( m_Vehicle->m_SVGView2(), to_orgin );

        FeatureLinesManipulate( projectionvec2, m_Vehicle->m_SVGView2(), m_Vehicle->m_SVGView2_rot(), shiftvec );
        FeatureLinesShift( projectionvec2, shiftvec, VIEW_SHIFT::RIGHT, m_Vehicle->m_SVGView2_rot(), 0 );
        WriteSVGPolylines2D( root, projectionvec2, svgbox );
    }
    else if ( m_Vehicle->m_SVGView() == vsp::VIEW_NUM::VIEW_2VER )
    {
        vector < vector < vec3d > > projectionvec1 = GetGeomProjectionLines( m_Vehicle->m_SVGView1(), to_orgin );

        FeatureLinesManipulate( projectionvec1, m_Vehicle->m_SVGView1(), m_Vehicle->m_SVGView1_rot(), shiftvec );
        FeatureLinesShift( projectionvec1, shiftvec, VIEW_SHIFT::UP, m_Vehicle->m_SVGView1_rot(), 0 );
        WriteSVGPolylines2D( root, projectionvec1, svgbox );

        vector < vector < vec3d > > projectionvec3 = GetGeomProjectionLines( m_Vehicle->m_SVGView3(), to_orgin );

        FeatureLinesManipulate( projectionvec3, m_Vehicle->m_SVGView3(), m_Vehicle->m_SVGView3_rot(), shiftvec );
        FeatureLinesShift( projectionvec3, shiftvec, VIEW_SHIFT::DOWN, m_Vehicle->m_SVGView3_rot(), 0 );
        WriteSVGPolylines2D( root, projectionvec3, svgbox );

    }
    else if ( m_Vehicle->m_SVGView() == vsp::VIEW_NUM::VIEW_4 )
    {
        vector < vector < vec3d > > projectionvec1 = GetGeomProjectionLines( m_Vehicle->m_SVGView1(), to_orgin );

        FeatureLinesManipulate( projectionvec1, m_Vehicle->m_SVGView1(), m_Vehicle->m_SVGView1_rot(), shiftvec );
        FeatureLinesShift( projectionvec1, shiftvec, VIEW_SHIFT::UP, m_Vehicle->m_SVGView1_rot(), m_Vehicle->m_SVGView2_rot() );
        FeatureLinesShift( projectionvec1, shiftvec, VIEW_SHIFT::LEFT, m_Vehicle->m_SVGView1_rot(), m_Vehicle->m_SVGView3_rot() );
        WriteSVGPolylines2D( root, projectionvec1, svgbox );

        vector < vector < vec3d > > projectionvec2 = GetGeomProjectionLines( m_Vehicle->m_SVGView2(), to_orgin );

        FeatureLinesManipulate( projectionvec2, m_Vehicle->m_SVGView2(), m_Vehicle->m_SVGView2_rot(), shiftvec );
        FeatureLinesShift( projectionvec2, shiftvec, VIEW_SHIFT::UP, m_Vehicle->m_SVGView2_rot(), m_Vehicle->m_SVGView1_rot() );
        FeatureLinesShift( projectionvec2, shiftvec, VIEW_SHIFT::RIGHT, m_Vehicle->m_SVGView2_rot(), m_Vehicle->m_SVGView4_rot() );
        WriteSVGPolylines2D( root, projectionvec2, svgbox );

        vector < vector < vec3d > > projectionvec3 = GetGeomProjectionLines( m_Vehicle->m_SVGView3(), to_orgin );

        FeatureLinesManipulate( projectionvec3, m_Vehicle->m_SVGView3(), m_Vehicle->m_SVGView3_rot(), shiftvec );
        FeatureLinesShift( projectionvec3, shiftvec, VIEW_SHIFT::DOWN, m_Vehicle->m_SVGView3_rot(), m_Vehicle->m_SVGView4_rot() );
        FeatureLinesShift( projectionvec3, shiftvec, VIEW_SHIFT::LEFT, m_Vehicle->m_SVGView3_rot(), m_Vehicle->m_SVGView1_rot() );
        WriteSVGPolylines2D( root, projectionvec3, svgbox );

        vector < vector < vec3d > > projectionvec4 = GetGeomProjectionLines( m_Vehicle->m_SVGView4(), to_orgin );

        FeatureLinesManipulate( projectionvec4, m_Vehicle->m_SVGView4(), m_Vehicle->m_SVGView4_rot(), shiftvec );
        FeatureLinesShift( projectionvec4, shiftvec, VIEW_SHIFT::DOWN, m_Vehicle->m_SVGView4_rot(), m_Vehicle->m_SVGView3_rot() );
        FeatureLinesShift( projectionvec4, shiftvec, VIEW_SHIFT::RIGHT, m_Vehicle->m_SVGView4_rot(), m_Vehicle->m_SVGView2_rot() );
        WriteSVGPolylines2D( root, projectionvec4, svgbox );
    }
}

void Geom::UpdateDrawObj()
{
    m_FeatureDrawObj_vec.clear();
    m_WireShadeDrawObj_vec.clear();

    if ( m_GuiDraw.GetDisplayType() == DISPLAY_TYPE::DISPLAY_BEZIER )
    {
        m_FeatureDrawObj_vec.resize(1);
        m_FeatureDrawObj_vec[0].m_GeomChanged = true;
        m_FeatureDrawObj_vec[0].m_LineWidth = 1.0;
        m_FeatureDrawObj_vec[0].m_LineColor = vec3d( 0.0, 0.0, 0.0 );
        m_FeatureDrawObj_vec[0].m_Screen = DrawObj::VSP_MAIN_SCREEN;
        m_FeatureDrawObj_vec[0].m_GeomID = m_ID + "Feature_0";
        m_FeatureDrawObj_vec[0].m_Type = DrawObj::VSP_LINES;

        m_WireShadeDrawObj_vec.resize( 4 );
        m_WireShadeDrawObj_vec[0].m_FlipNormals = false;
        m_WireShadeDrawObj_vec[1].m_FlipNormals = true;
        m_WireShadeDrawObj_vec[2].m_FlipNormals = false;
        m_WireShadeDrawObj_vec[3].m_FlipNormals = true;

        m_WireShadeDrawObj_vec[0].m_GeomChanged = true;
        m_WireShadeDrawObj_vec[1].m_GeomChanged = true;
        m_WireShadeDrawObj_vec[2].m_GeomChanged = true;
        m_WireShadeDrawObj_vec[3].m_GeomChanged = true;

        // Pre-calculate and allocate for number of feature line segments.
        // Identified by profiling as a substantial cost.
        int numfealineseg = 0;
        for ( int i = 0 ; i < m_FeatureTessVec.size() ; i++ )
        {
            int nfl = m_FeatureTessVec[i].m_ptline.size();

            for( int j = 0; j < nfl; j++ )
            {
                int n = m_FeatureTessVec[i].m_ptline[j].size() - 1;

                numfealineseg += 2 * n;
            }
        }
        m_FeatureDrawObj_vec[0].m_PntVec.reserve( numfealineseg );

        int nsurf = GetNumTotalSurfs();
        if ( m_TessVec.size() == nsurf && m_SurfVec.size() == nsurf && m_FeatureTessVec.size() == nsurf )
        {
            //==== Tesselate Surface ====//
            for ( int i = 0 ; i < nsurf ; i++ )
            {
                int iflip = 0;
                if ( m_TessVec[i].GetFlipNormal() )
                {
                    iflip = 1;
                }

                if ( m_SurfVec[i].GetSurfCfdType() == vsp::CFD_TRANSPARENT )
                {
                    iflip += 2;
                }

                m_WireShadeDrawObj_vec[iflip].m_PntMesh.insert( m_WireShadeDrawObj_vec[iflip].m_PntMesh.end(),
                        m_TessVec[i].m_pnts.begin(), m_TessVec[i].m_pnts.end() );
                m_WireShadeDrawObj_vec[iflip].m_NormMesh.insert( m_WireShadeDrawObj_vec[iflip].m_NormMesh.end(),
                        m_TessVec[i].m_norms.begin(), m_TessVec[i].m_norms.end() );

                m_WireShadeDrawObj_vec[iflip].m_uTexMesh.insert( m_WireShadeDrawObj_vec[iflip].m_uTexMesh.end(),
                        m_TessVec[i].m_utex.begin(), m_TessVec[i].m_utex.end() );
                m_WireShadeDrawObj_vec[iflip].m_vTexMesh.insert( m_WireShadeDrawObj_vec[iflip].m_vTexMesh.end(),
                        m_TessVec[i].m_vtex.begin(), m_TessVec[i].m_vtex.end() );

                if( m_GuiDraw.GetDispFeatureFlag() )
                {
                    int nfl = m_FeatureTessVec[i].m_ptline.size();

                    for( int j = 0; j < nfl; j++ )
                    {
                        int n = m_FeatureTessVec[i].m_ptline[j].size() - 1;

                        for ( int k = 0; k < n; k++ )
                        {
                            m_FeatureDrawObj_vec[0].m_PntVec.push_back( m_FeatureTessVec[i].m_ptline[j][ k ] );
                            m_FeatureDrawObj_vec[0].m_PntVec.push_back( m_FeatureTessVec[i].m_ptline[j][ k + 1 ] );
                        }
                    }

                }
            }
        }
    }

    //==== Bounding Box ====//
    m_HighlightDrawObj.m_PntVec = m_BBox.GetBBoxDrawLines();
    m_HighlightDrawObj.m_GeomChanged = true;

    m_PtMassCGDrawObj.m_PntVec.clear();
    m_PtMassCGDrawObj.m_GeomChanged = true;
    if ( m_PointMass() > 0.0 )
    {
        for ( int j = 0; j < m_TransMatVec.size(); j++ )
        {
            vec3d cg = m_TransMatVec[ j ].xform( vec3d( m_CGx(), m_CGy(), m_CGz() ) );
            m_PtMassCGDrawObj.m_PntVec.push_back( cg );
        }
    }

    //=== Axis ===//
    m_AxisDrawObj_vec.clear();
    m_AxisDrawObj_vec.resize( 3 );
    for ( int i = 0; i < 3; i++ )
    {
        MakeDashedLine( m_AttachOrigin,  m_AttachAxis[i], 4, m_AxisDrawObj_vec[i].m_PntVec );
        vec3d c;
        c.v[i] = 1.0;
        m_AxisDrawObj_vec[i].m_LineColor = c;
        m_AxisDrawObj_vec[i].m_GeomChanged = true;
    }

    UpdateDegenDrawObj();
}

void Geom::UpdateDegenDrawObj()
{
    m_DegenSurfDrawObj_vec.clear();
    m_DegenPlateDrawObj_vec.clear();
    m_DegenCamberPlateDrawObj_vec.clear();
    m_DegenSubSurfDrawObj_vec.clear();

    if ( m_GuiDraw.GetDisplayType() != DISPLAY_TYPE::DISPLAY_BEZIER )
    {

        for ( int i = 0; i < (int)m_DegenGeomPreviewVec.size(); i++ )
        {
            //=== Degen Surface ===//
            DegenSurface degen_surf = m_DegenGeomPreviewVec[i].getDegenSurf();

            DrawObj degen_surf_draw_obj;
            degen_surf_draw_obj.m_GeomChanged = true;

            for ( int j = 0; j < degen_surf.x.size() - 1; j++ )
            {
                for ( int k = 0; k < degen_surf.x[j].size() - 1; k++ )
                {
                    // Define Quads
                    vec3d corner1, corner2, corner3, corner4, norm;

                    corner1 = degen_surf.x[j][k];
                    corner2 = degen_surf.x[j + 1][k];
                    corner3 = degen_surf.x[j + 1][k + 1];
                    corner4 = degen_surf.x[j][k + 1];

                    degen_surf_draw_obj.m_PntVec.push_back( corner1 );
                    degen_surf_draw_obj.m_PntVec.push_back( corner2 );
                    degen_surf_draw_obj.m_PntVec.push_back( corner3 );
                    degen_surf_draw_obj.m_PntVec.push_back( corner4 );

                    norm = degen_surf.nvec[j][k];

                    // Set Normal Vector
                    for ( int m = 0; m < 4; m++ )
                    {
                        degen_surf_draw_obj.m_NormVec.push_back( norm );
                    }
                }
            }

            m_DegenSurfDrawObj_vec.push_back( degen_surf_draw_obj );

            //=== Degen Plate and Cambered Plate ===//
            vector < DegenPlate > degen_plate_vec = m_DegenGeomPreviewVec[i].getDegenPlates();

            for ( int j = 0; j < degen_plate_vec.size(); j++ )
            {
                DrawObj degen_plate_draw_obj;
                DrawObj degen_camber_plate_draw_obj;

                degen_plate_draw_obj.m_GeomChanged = true;
                degen_camber_plate_draw_obj.m_GeomChanged = true;

                for ( int k = 0; k < degen_plate_vec[j].x.size() - 1; k++ )
                {
                    for ( int n = 0; n < degen_plate_vec[j].x[k].size() - 1; n++ )
                    {
                        // Define Plate Quads
                        vec3d corner1, corner2, corner3, corner4, norm;

                        corner1 = degen_plate_vec[j].x[k][n];
                        corner2 = degen_plate_vec[j].x[k][n + 1];
                        corner3 = degen_plate_vec[j].x[k + 1][n + 1];
                        corner4 = degen_plate_vec[j].x[k + 1][n];

                        degen_plate_draw_obj.m_PntVec.push_back( corner1 );
                        degen_plate_draw_obj.m_PntVec.push_back( corner2 );
                        degen_plate_draw_obj.m_PntVec.push_back( corner3 );
                        degen_plate_draw_obj.m_PntVec.push_back( corner4 );

                        norm = degen_plate_vec[j].nPlate[k];

                        // Set Normal Vectors
                        for ( int m = 0; m < 4; m++ )
                        {
                            degen_plate_draw_obj.m_NormVec.push_back( norm );
                        }

                        // Define Cambered Plate Quads
                        vec3d camber_corner1, camber_corner2, camber_corner3, camber_corner4;
                        vec3d norm1, norm2, norm3, norm4;

                        norm1 = degen_plate_vec[j].nCamber[k][n];
                        norm2 = degen_plate_vec[j].nCamber[k][n+1];
                        norm3 = degen_plate_vec[j].nCamber[k+1][n+1];
                        norm4 = degen_plate_vec[j].nCamber[k+1][n];

                        camber_corner1 = corner1 + ( degen_plate_vec[j].zcamber[k][n] * norm1 );
                        camber_corner2 = corner2 + ( degen_plate_vec[j].zcamber[k][n + 1] * norm2);
                        camber_corner3 = corner3 + ( degen_plate_vec[j].zcamber[k + 1][n + 1] * norm3 );
                        camber_corner4 = corner4 + ( degen_plate_vec[j].zcamber[k + 1][n] * norm4 );

                        degen_camber_plate_draw_obj.m_PntVec.push_back( camber_corner1 );
                        degen_camber_plate_draw_obj.m_PntVec.push_back( camber_corner2 );
                        degen_camber_plate_draw_obj.m_PntVec.push_back( camber_corner3 );
                        degen_camber_plate_draw_obj.m_PntVec.push_back( camber_corner4 );

                        // Calculate normal for cambered plate shading (camber normal is from DegenGeom is the direction between the top and bottom surface points)

                        vec3d u1, u2, v1, v2, u, v;

                        u1 = camber_corner4 - camber_corner1;
                        u2 = camber_corner3 - camber_corner2;

                        if ( u1.mag() > u2.mag() )
                        {
                            u = u1;
                        }
                        else
                        {
                            u = u2;
                        }

                        v1 = camber_corner2 - camber_corner1;
                        v2 = camber_corner3 - camber_corner4;

                        if ( v1.mag() > v2.mag() )
                        {
                            v = v1;
                        }
                        else
                        {
                            v = v2;
                        }

                        vec3d camber_draw_norm = cross( v, u );
                        camber_draw_norm.normalize();

                        vec3d approx_norm = 0.25 * ( norm1 + norm2 + norm3 + norm4 );
                        approx_norm.normalize();

                        if ( dot( camber_draw_norm, approx_norm ) < 0.0 )
                        {
                            camber_draw_norm = -1 * camber_draw_norm;
                        }

                        for ( int m = 0; m < 4; m++ )
                        {
                            degen_camber_plate_draw_obj.m_NormVec.push_back( camber_draw_norm );
                        }
                    }
                }

                m_DegenPlateDrawObj_vec.push_back( degen_plate_draw_obj );
                m_DegenCamberPlateDrawObj_vec.push_back( degen_camber_plate_draw_obj );
            }

            //=== Degen SubSurface ===//
            vector < DegenSubSurf > degen_subsurf_vec = m_DegenGeomPreviewVec[i].getDegenSubSurfs();

            for ( int j = 0; j < degen_subsurf_vec.size(); j++ )
            {
                DrawObj degen_subsurface_draw_obj;
                degen_subsurface_draw_obj.m_GeomChanged = true;

                if ( degen_subsurf_vec[j].typeId == SS_LINE )
                {
                    if ( degen_subsurf_vec[j].u[0] == degen_subsurf_vec[j].u.back() ) // Constant U
                    {
                        int u_index_low = 0;
                        int u_index_high = 0;
                        double u_value_low = 0;
                        double u_value_high = 0;

                        // Find uw indexes and values next to DegenSubSurface const u value
                        for ( int m = 1; m < degen_surf.u.size(); m++ )
                        {
                            if ( ( degen_surf.u[m][0] >= degen_subsurf_vec[j].u[0] ) && ( degen_surf.u[m - 1][0] <= degen_subsurf_vec[j].u[0] ) )
                            {
                                u_index_low = m - 1;
                                u_index_high = m;
                                u_value_low = degen_surf.u[m - 1][0];
                                u_value_high = degen_surf.u[m][0];

                                break;
                            }
                        }

                        // Linear Interpolation of DegenSurface uw indexes and values to DgenSubsurface uw index
                        double degen_surf_u_index = u_index_low + ( ( u_index_high - u_index_low ) * ( ( degen_subsurf_vec[j].u[0] - u_value_low ) / ( u_value_high - u_value_low ) ) );

                        for ( int n = 0; n < degen_surf.x[0].size(); n++ )
                        {
                            // Interpolation of uw indexes to vec3d coordinates
                            vec3d uw_pnt_low = degen_surf.x[u_index_low][n];
                            vec3d uw_pnt_high = degen_surf.x[u_index_high][n];

                            vec3d degen_subsurf_pnt = uw_pnt_low + ( ( degen_surf_u_index - u_index_low ) * ( uw_pnt_high - uw_pnt_low ) );

                            degen_subsurface_draw_obj.m_PntVec.push_back( degen_subsurf_pnt );
                        }
                    }
                    else if ( degen_subsurf_vec[j].w[0] == degen_subsurf_vec[j].w.back() ) // Constant W
                    {
                        int w_index_low = 0;
                        int w_index_high = 0;
                        double w_value_low = 0;
                        double w_value_high = 0;

                        // Find uw indexes and values next to DegenSubSurface const u value
                        for ( int m = 1; m < degen_surf.w.size(); m++ )
                        {
                            if ( ( degen_surf.w[0][m] >= degen_subsurf_vec[j].w[0] ) && ( degen_surf.w[0][m - 1] <= degen_subsurf_vec[j].w[0] ) )
                            {
                                w_index_low = m - 1;
                                w_index_high = m;
                                w_value_low = degen_surf.w[0][m - 1];
                                w_value_high = degen_surf.w[0][m];

                                break;
                            }
                        }

                        // Linear Interpolation of DegenSurface uw indexes and values to DgenSubsurface uw index
                        double degen_surf_w_index = w_index_low + ( ( w_index_high - w_index_low ) * ( ( degen_subsurf_vec[j].w[0] - w_value_low ) / ( w_value_high - w_value_low ) ) );

                        for ( int n = 0; n < degen_surf.x.size(); n++ )
                        {
                            // Interpolation of uw indexes to vec3d coordinates
                            vec3d uw_pnt_low = degen_surf.x[n][w_index_low];
                            vec3d uw_pnt_high = degen_surf.x[n][w_index_high];

                            vec3d degen_subsurf_pnt = uw_pnt_low + ( ( degen_surf_w_index - w_index_low ) * ( uw_pnt_high - uw_pnt_low ) );

                            degen_subsurface_draw_obj.m_PntVec.push_back( degen_subsurf_pnt );
                        }
                    }
                }
                else
                {
                    for ( int k = 0; k < degen_subsurf_vec[j].u.size(); k++ )
                    {
                        vec2d uw_index_low, uw_index_high, uw_value_low, uw_value_high, uw_value_degen_subsurf;

                        // Check for u or w values outside of the min amd max u and w for the DegenSurface
                        if ( degen_subsurf_vec[j].u[k] < degen_surf.u[0][0] )
                        {
                            uw_value_degen_subsurf.set_x( degen_surf.u[0][0] );
                        }
                        else if ( degen_subsurf_vec[j].u[k] > degen_surf.u[degen_surf.u.size() - 1][0] )
                        {
                            uw_value_degen_subsurf.set_x( degen_surf.u[degen_surf.u.size() - 1][0] );
                        }
                        else
                        {
                            uw_value_degen_subsurf.set_x( degen_subsurf_vec[j].u[k] );
                        }

                        if ( degen_subsurf_vec[j].w[k] < degen_surf.w[0][0] )
                        {
                            uw_value_degen_subsurf.set_y( degen_surf.w[0][0] );
                        }
                        else if ( degen_subsurf_vec[j].w[k] > degen_surf.w[0][degen_surf.w[0].size() - 1] )
                        {
                            uw_value_degen_subsurf.set_y( degen_surf.w[0][degen_surf.w[0].size() - 1] );
                        }
                        else
                        {
                            uw_value_degen_subsurf.set_y( degen_subsurf_vec[j].w[k] );
                        }

                        // Find the uw indexes and values next to the DegenSubsurface uw value
                        for ( int m = 1; m < degen_surf.u.size(); m++ )
                        {
                            if ( ( degen_surf.u[m][0] >= uw_value_degen_subsurf.x() ) && ( degen_surf.u[m - 1][0] <= uw_value_degen_subsurf.x() ) )
                            {
                                uw_index_low.set_x( m - 1 );
                                uw_index_high.set_x( m );
                                uw_value_low.set_x( degen_surf.u[m - 1][0] );
                                uw_value_high.set_x( degen_surf.u[m][0] );

                                for ( int n = 1; n < degen_surf.w[0].size(); n++ )
                                {
                                    if ( ( degen_surf.w[0][n] >= uw_value_degen_subsurf.y() ) && ( degen_surf.w[0][n - 1] <= uw_value_degen_subsurf.y() ) )
                                    {
                                        uw_index_low.set_y( n - 1 );
                                        uw_index_high.set_y( n );
                                        uw_value_low.set_y( degen_surf.w[0][n - 1] );
                                        uw_value_high.set_y( degen_surf.w[0][n] );
                                        break;
                                    }
                                }
                                break;
                            }
                        }

                        // Linear Interpolation of DegenSurface uw indexes and values to DgenSubsurface uw index
                        vec2d degen_subsurf_index;
                        degen_subsurf_index.set_x( uw_index_low.x() + ( ( uw_index_high.x() - uw_index_low.x() ) * ( ( uw_value_degen_subsurf.x() - uw_value_low.x() ) / ( uw_value_high.x() - uw_value_low.x() ) ) ) );
                        degen_subsurf_index.set_y( uw_index_low.y() + ( ( uw_index_high.y() - uw_index_low.y() ) * ( ( uw_value_degen_subsurf.y() - uw_value_low.y() ) / ( uw_value_high.y() - uw_value_low.y() ) ) ) );

                        // Bilinear Interpolation of uw indexes to vec3d coordinates
                        vec3d uw_pnt_low_low = degen_surf.x[(int)uw_index_low.x()][(int)uw_index_low.y()];
                        vec3d uw_pnt_low_high = degen_surf.x[(int)uw_index_low.x()][(int)uw_index_high.y()];
                        vec3d uw_pnt_high_high = degen_surf.x[(int)uw_index_high.x()][(int)uw_index_high.y()];
                        vec3d uw_pnt_high_low = degen_surf.x[(int)uw_index_high.x()][(int)uw_index_low.y()];

                        // Horizontal interpolation
                        vec3d uw_mid_low = uw_pnt_low_low + ( ( degen_subsurf_index.x() - uw_index_low.x() ) * ( uw_pnt_high_low - uw_pnt_low_low ) );
                        vec3d uw_mid_high = uw_pnt_low_high + ( ( degen_subsurf_index.x() - uw_index_low.x() ) * ( uw_pnt_high_high - uw_pnt_low_high ) );

                        // Vertical interpolation
                        vec3d degen_subsurf_pnt = uw_mid_low + ( ( degen_subsurf_index.y() - uw_index_low.y() ) * ( uw_mid_high - uw_mid_low ) );

                        degen_subsurface_draw_obj.m_PntVec.push_back( degen_subsurf_pnt );
                    }
                }

                m_DegenSubSurfDrawObj_vec.push_back( degen_subsurface_draw_obj );
            }
        }
    }
}

void Geom::UpdateTess( const VspSurf & surf, bool capUMinSuccess, bool capUMaxSuccess, SimpleTess &tess, SimpleFeatureTess &featureTess )
{
    double tol = 1e-3;

    UpdateSplitTesselate( surf, capUMinSuccess, capUMaxSuccess, tess.m_pnts, tess.m_norms );

    bool fn = surf.GetFlipNormal();
    tess.m_FlipNormal = fn;
    featureTess.m_FlipNormal = fn;

    int nu = surf.GetNumUFeature();
    int nw = surf.GetNumWFeature();

    tess.m_nufeat = nu;
    tess.m_nvfeat = nw;

    tess.CalcTexCoords();

    featureTess.m_ptline.resize( nu + nw );

    for( int j = 0; j < nu; j++ )
    {
        surf.TessUFeatureLine( j, featureTess.m_ptline[j], tol );
    }

    for( int j = 0; j < nw; j++ )
    {
        surf.TessWFeatureLine( j, featureTess.m_ptline[j+nu], tol );
    }
}

// Compute all the main surface tessellations
// Also compute the main surface feature line tessellations
void Geom::UpdateMainTessVec()
{
    double tol = 1e-3;

    int nmain = GetNumMainSurfs();

    m_MainTessVec.resize( nmain );
    m_MainFeatureTessVec.resize( nmain );

    for ( int i = 0 ; i < nmain ; i++ )
    {
        UpdateTess( m_MainSurfVec[i], m_CapUMinSuccess[ m_SurfIndxVec[ i ] ], m_CapUMaxSuccess[ m_SurfIndxVec[ i ] ], m_MainTessVec[i], m_MainFeatureTessVec[i]);
    }
}

// Propagate symmetry and position to tessellation and feature line tess.
void Geom::UpdateTessVec()
{
    ApplySymm( m_MainTessVec, m_TessVec );
    ApplySymm( m_MainFeatureTessVec, m_FeatureTessVec );
}

void Geom::UpdateMainDegenGeomPreview()
{
    m_MainDegenGeomPreviewVec.clear();
    CreateMainDegenGeomPreview( m_MainDegenGeomPreviewVec );
}

void Geom::UpdateDegenGeomPreview()
{
    ApplySymm( m_MainDegenGeomPreviewVec, m_DegenGeomPreviewVec );
}

//==== Encode Data Into XML Data Struct ====//
xmlNodePtr Geom::EncodeXml( xmlNodePtr & node )
{
    GeomXForm::EncodeXml( node );

    // Encode Material Info.
    m_GuiDraw.getMaterial()->EncodeNameXml( node );

    // Encode Color Info.
    m_GuiDraw.getColorMgr()->EncodeXml( node );

    // Encode Texture Info.
    m_GuiDraw.getTextureMgr()->EncodeXml( node );

    xmlNodePtr geom_node = xmlNewChild( node, nullptr, BAD_CAST "Geom", nullptr );
    if ( geom_node )
    {
        XmlUtil::AddVectorBoolNode( geom_node, "Set_List", GetSetFlags() );

        for( int i = 0; i < ( int )m_MainSourceVec.size(); i++ )
        {
            m_MainSourceVec[i]->EncodeXml( geom_node );
        }

        xmlNodePtr subsurfs_node = xmlNewChild( geom_node, nullptr, BAD_CAST "SubSurfaces", nullptr );

        if ( subsurfs_node )
        {
            for( int i = 0 ; i < ( int ) m_SubSurfVec.size() ; i++ )
            {
                xmlNodePtr sub_node = xmlNewChild( subsurfs_node, nullptr, BAD_CAST "SubSurface", nullptr );
                if ( sub_node )
                {
                    m_SubSurfVec[i]->EncodeXml( sub_node );
                }
            }
        }

        xmlNodePtr structvecnode = xmlNewChild( geom_node, nullptr, BAD_CAST"FeaStructures", nullptr );
        if ( structvecnode )
        {
            for ( int i = 0; i < m_FeaStructVec.size(); i++ )
            {
                m_FeaStructVec[i]->EncodeXml( structvecnode );
            }
        }
    }
    return geom_node;

}

//==== Decode Data Into XML Data Struct ====//
xmlNodePtr Geom::DecodeXml( xmlNodePtr & node )
{
    GeomXForm::DecodeXml( node );

    // Decode Material Info.
    m_GuiDraw.getMaterial()->DecodeNameXml( node );

    // Decode Color Info.
    m_GuiDraw.getColorMgr()->DecodeXml( node );

    // Decode Texture Info.
    m_GuiDraw.getTextureMgr()->DecodeXml( node );

    xmlNodePtr geom_node = XmlUtil::GetNode( node, "Geom", 0 );
    if ( geom_node )
    {
        m_SetFlags = XmlUtil::ExtractVectorBoolNode( geom_node, "Set_List" );

        int num_source = XmlUtil::GetNumNames( geom_node, "Source" );

        for ( int i = 0 ; i < num_source ; i++ )
        {
            xmlNodePtr src_node = XmlUtil::GetNode( geom_node, "Source", i );
            if ( src_node )
            {
                int type = XmlUtil::FindInt( src_node, "Type", POINT_SOURCE );

                BaseSource* src_ptr = CreateSource( type );  // Can change to CfdMeshMgr. when in scope.

                if ( src_ptr )
                {
                    src_ptr->DecodeXml( src_node );
                    AddCfdMeshSource( src_ptr );
                }
            }
        }

        // Decode SubSurfaces
        xmlNodePtr subsurfs_node = XmlUtil::GetNode( geom_node, "SubSurfaces", 0 );
        if ( subsurfs_node )
        {
            int num_ss = XmlUtil::GetNumNames( subsurfs_node, "SubSurface" );

            for ( int ss = 0 ; ss < num_ss ; ss++ )
            {
                xmlNodePtr ss_node = XmlUtil::GetNode( subsurfs_node, "SubSurface", ss );
                if ( ss_node )
                {
                    xmlNodePtr ss_info_node = XmlUtil::GetNode( ss_node, "SubSurfaceInfo", 0 );
                    if ( ss_info_node )
                    {
                        int type = XmlUtil::FindInt( ss_info_node, "Type", vsp::SS_LINE );
                        SubSurface* ssurf = AddSubSurf( type, -1 );
                        if ( ssurf )
                        {
                            ssurf->DecodeXml( ss_node );
                        }
                    }
                }
            }
        }

        // Decode FeaStructures
        xmlNodePtr structvecnode = XmlUtil::GetNode( geom_node, "FeaStructures", 0 );
        if ( structvecnode )
        {
            int num_struct = XmlUtil::GetNumNames( structvecnode, "FeaStructureInfo" );

            for ( int i = 0; i < num_struct; i++ )
            {
                xmlNodePtr structnode = XmlUtil::GetNode( structvecnode, "FeaStructureInfo", i );

                if ( structnode )
                {
                    int surf_index = XmlUtil::FindInt( structnode, "MainSurfIndx", 0 );

                    // Provide a new structure to decode to. Do not initialize the skin because it will be decoded
                    FeaStructure* feastruct = AddFeaStruct( false, surf_index );

                    if ( feastruct )
                    {
                        feastruct->DecodeXml( structnode );

                        xmlNodePtr setting_node = XmlUtil::GetNode( structnode, "StructSettings", 0 );
                        if ( setting_node )
                        {
                            feastruct->GetStructSettingsPtr()->DecodeXml( structnode );
                            feastruct->ResetExportFileNames();
                        }

                        xmlNodePtr dense_node = XmlUtil::GetNode( structnode, "FEAGridDensity", 0 );
                        if ( dense_node )
                        {
                            feastruct->GetFeaGridDensityPtr()->DecodeXml( structnode );
                        }
                    }
                }
            }
        }
    }
    return geom_node;
}

//==== Encode Data Into XML Data Struct ====//
xmlNodePtr Geom::EncodeGeom( xmlNodePtr & node )
{
    xmlNodePtr geom_node = xmlNewChild( node, nullptr, BAD_CAST "Geom", nullptr );
    if ( geom_node )
    {
        EncodeXml( geom_node );
    }
    return geom_node;
}

//==== Encode Data Into XML Data Struct ====//
xmlNodePtr Geom::DecodeGeom( xmlNodePtr & node )
{
    xmlNodePtr geom_node = XmlUtil::GetNode( node, "Geom", 0 );
    if ( geom_node )
    {
        DecodeXml( geom_node );
    }
    return geom_node;
}

void Geom::ReadV2File( xmlNodePtr &root )
{
    int i;

    SetName( XmlUtil::FindString( root, "Name", m_Name ) );

    double r = XmlUtil::FindDouble( root, "ColorR", 0 );
    double g = XmlUtil::FindDouble( root, "ColorG", 0 );
    double b = XmlUtil::FindDouble( root, "ColorB", 0 );

    m_GuiDraw.SetWireColor( r, g, b );

    int sym_code = XmlUtil::FindInt( root, "Symmetry", 0 );

    if ( sym_code == V2_XY_SYM )
    {
        m_SymPlanFlag = SYM_XY;
    }
    else if ( sym_code == V2_XZ_SYM )
    {
        m_SymPlanFlag = SYM_XZ;
    }
    else if ( sym_code == V2_YZ_SYM )
    {
        m_SymPlanFlag = SYM_YZ;
    }
    else
    {
        m_SymPlanFlag = 0;
    }

    m_AbsRelFlag = XmlUtil::FindInt( root, "RelXFormFlag", m_AbsRelFlag() );

    int materialID = XmlUtil::FindInt( root, "MaterialID", 0 );

    vector < string > v2materials;
    v2materials.push_back( "Default" );
    v2materials.push_back( "Red Default" );
    v2materials.push_back( "Green Default" );
    v2materials.push_back( "Blue Default" );
    v2materials.push_back( "Emerald" );
    v2materials.push_back( "Jade" );
    v2materials.push_back( "Obsidian" );
    v2materials.push_back( "Brass" );
    v2materials.push_back( "Chrome" );
    v2materials.push_back( "Gold" );
    v2materials.push_back( "Silver" );
    v2materials.push_back( "Black Plastic" );
    v2materials.push_back( "Cyan Plastic" );
    v2materials.push_back( "Green Plastic" );
    v2materials.push_back( "Red Plastic" );
    v2materials.push_back( "Blue Plastic" );
    v2materials.push_back( "Yellow Plastic" );
    v2materials.push_back( "White" );
    v2materials.push_back( "Aluminum" );
    v2materials.push_back( "Shiny Gold" );
    v2materials.push_back( "Glass Light" );
    v2materials.push_back( "Glass Med" );
    v2materials.push_back( "Glass Dark" );
    v2materials.push_back( "Glass Golden" );
    v2materials.push_back( "Blank" );
    m_GuiDraw.SetMaterial( v2materials[materialID] );

    m_TessW = XmlUtil::FindInt( root, "NumPnts", m_TessW() );
    m_TessU = XmlUtil::FindInt( root, "NumXsecs", m_TessU() );

    int outputFlag   = XmlUtil::FindInt( root, "OutputFlag", 0 );
    int outputNameID = XmlUtil::FindInt( root, "OutputNameID", 0 );
    int displayChildrenFlag = XmlUtil::FindInt( root, "DisplayChildrenFlag", 0 );

    m_MassPrior = XmlUtil::FindInt( root, "MassPrior", m_MassPrior() );
    m_ShellFlag = XmlUtil::FindInt( root, "ShellFlag", m_ShellFlag() );

    m_XLoc = XmlUtil::FindDouble( root, "Tran_X", m_XLoc() );
    m_YLoc = XmlUtil::FindDouble( root, "Tran_Y", m_YLoc() );
    m_ZLoc = XmlUtil::FindDouble( root, "Tran_Z", m_ZLoc() );
    m_XRelLoc = XmlUtil::FindDouble( root, "TranRel_X", m_XRelLoc() );
    m_YRelLoc = XmlUtil::FindDouble( root, "TranRel_Y", m_YRelLoc() );
    m_ZRelLoc = XmlUtil::FindDouble( root, "TranRel_Z", m_ZRelLoc() );

    m_XRot = XmlUtil::FindDouble( root, "Rot_X", m_XRot() );
    m_YRot = XmlUtil::FindDouble( root, "Rot_Y", m_YRot() );
    m_ZRot = XmlUtil::FindDouble( root, "Rot_Z", m_ZRot() );
    // m_XRelRot = XmlUtil::FindDouble( root, "RotRel_X", m_XRelRot() );
    // m_YRelRot = XmlUtil::FindDouble( root, "RotRel_Y", m_YRelRot() );
    // m_ZRelRot = XmlUtil::FindDouble( root, "RotRel_Z", m_ZRelRot() );
    // Relative rotation doesn't actually have meaning in v2.  Apply
    // overall values for later use as needed.
    m_XRelRot = m_XRot();
    m_YRelRot = m_YRot();
    m_ZRelRot = m_ZRot();


    m_Origin = XmlUtil::FindDouble( root, "Origin", m_Origin() );

    m_Density = XmlUtil::FindDouble( root, "Density", m_Density() );
    m_MassArea = XmlUtil::FindDouble( root, "ShellMassArea", m_MassArea() );

    int refFlag = XmlUtil::FindInt( root, "RefFlag", 0 );
    double refArea = XmlUtil::FindDouble( root, "RefArea", 0 );
    double refSpan = XmlUtil::FindDouble( root, "RefSpan", 0 );
    double refCbar = XmlUtil::FindDouble( root, "RefCbar", 0 );
    int autoRefAreaFlag = XmlUtil::FindInt( root, "AutoRefAreaFlag", 0 );
    int autoRefSpanFlag = XmlUtil::FindInt( root, "AutoRefSpanFlag", 0 );
    int autoRefCbarFlag = XmlUtil::FindInt( root, "AutoRefCbarFlag", 0 );
    vec3d aeroCenter;
    aeroCenter.set_x( XmlUtil::FindDouble( root, "AeroCenter_X", aeroCenter.x() ) );
    aeroCenter.set_y( XmlUtil::FindDouble( root, "AeroCenter_Y", aeroCenter.y() ) );
    aeroCenter.set_z( XmlUtil::FindDouble( root, "AeroCenter_Z", aeroCenter.z() ) );
    int autoAeroCenterFlag = XmlUtil::FindInt( root, "AutoAeroCenterFlag", 0 );

    m_WakeActiveFlag = !!(XmlUtil::FindInt( root, "WakeActiveFlag", m_WakeActiveFlag() ));

    //==== Read Attach Flags ====//
    int posAttachFlag = XmlUtil::FindInt( root, "PosAttachFlag", 0 );

    // A series of flags to override specific coordinates
    bool overrideRelTrans = false;

    if ( posAttachFlag == V2_POS_ATTACH_NONE )
    {
        m_TransAttachFlag = vsp::ATTACH_TRANS_NONE;
        m_RotAttachFlag = vsp::ATTACH_ROT_NONE;

        // override relative translation since the relative coordinates are arbitrary
        // when not attached to anything in V2
        overrideRelTrans = true;
    }
    else if ( posAttachFlag == V2_POS_ATTACH_FIXED )
    {
        m_TransAttachFlag = vsp::ATTACH_TRANS_COMP;
        m_RotAttachFlag = vsp::ATTACH_ROT_NONE;
    }
    else if ( posAttachFlag == V2_POS_ATTACH_UV )
    {
        m_TransAttachFlag = vsp::ATTACH_TRANS_UV;
        m_RotAttachFlag = vsp::ATTACH_ROT_NONE;
    }
    else if ( posAttachFlag == V2_POS_ATTACH_MATRIX )
    {
        m_TransAttachFlag = vsp::ATTACH_TRANS_COMP;
        m_RotAttachFlag = vsp::ATTACH_ROT_COMP;

        // override relative translation since the relative coordinates are arbitrary
        // when attached in matrix mode
        overrideRelTrans = true;

        // override the AbsRelFlag, the only valid value for this attachment mode is relative
        m_AbsRelFlag = vsp::REL;
    }

    if ( overrideRelTrans )
    {
        m_XRelLoc = m_XLoc();
        m_YRelLoc = m_YLoc();
        m_ZRelLoc = m_ZLoc();
    }

    m_ULoc = XmlUtil::FindDouble( root, "U_Attach", m_ULoc() );
    m_WLoc = XmlUtil::FindDouble( root, "V_Attach", m_WLoc() );

    //==== Read Pointer ID and Parent/Children Info ====//
    string newID = ParmMgr.ForceRemapID( XmlUtil::FindString( root, "PtrID", m_ID ), 10 );

    if( newID.compare( m_ID ) != 0 )
    {
        ChangeID( newID );
    }

    string parent = XmlUtil::FindString( root, "Parent_PtrID", m_ParentID );
    if ( parent != "0" )
    {
        m_ParentID = ParmMgr.ForceRemapID( parent , 10 );
    }

    m_ChildIDVec.clear();
    int numChildren =  XmlUtil::GetNumNames( root, "Children_PtrID" );
    for (  i = 0 ; i < numChildren ; i++ )
    {
        xmlNodePtr child_node = XmlUtil::GetNode( root, "Children_PtrID", i );
        m_ChildIDVec.push_back( ParmMgr.ForceRemapID( XmlUtil::ExtractString( child_node ) , 10 ) );
    }

    //==== Read CFD Mesh Sources ====//
    DelAllSources();

    int numSources = XmlUtil::GetNumNames( root, "CFD_Mesh_Source" );
    for ( i = 0 ; i < numSources ; i++ )
    {
        xmlNodePtr source_node = XmlUtil::GetNode( root, "CFD_Mesh_Source", i );
        int type = XmlUtil::FindInt( source_node, "Type", -1 );

        BaseSource* src_ptr = CreateSource( type );

        if ( src_ptr )
        {
            src_ptr->ReadV2File( source_node );
            AddCfdMeshSource( src_ptr );
        }
    }


    /*
    appTexVec.clear();
    int numAppliedTextures = XmlUtil::GetNumNames( root, "Applied_Texture" );

    AppliedTex apptex;
    for ( i = 0 ; i < numAppliedTextures ; i++ )
    {
        xmlNodePtr tex_node = XmlUtil::GetNode( root, "Applied_Texture", i );

        apptex.nameStr = string( XmlUtil::FindString( tex_node, "Name", "Default_Name" ) );
        apptex.texStr  = string( XmlUtil::FindString( tex_node, "Texture_Name", "Default_Name" ) );
        apptex.allSurfFlag  = !!XmlUtil::FindInt( tex_node, "All_Surf_Flag", 0 );
        apptex.surfID  = XmlUtil::FindInt( tex_node, "Surf_ID", 0 );
        apptex.u  = XmlUtil::FindDouble( tex_node, "U", 0.5 );
        apptex.w  = XmlUtil::FindDouble( tex_node, "W", 0.5 );
        apptex.scaleu  = XmlUtil::FindDouble( tex_node, "Scale_U", 1.0 );
        apptex.scalew  = XmlUtil::FindDouble( tex_node, "Scale_W", 1.0 );
        apptex.wrapUFlag  = !!XmlUtil::FindInt( tex_node, "Wrap_U_Flag", 0 );
        apptex.wrapWFlag  = !!XmlUtil::FindInt( tex_node, "Wrap_W_Flag", 0 );
        apptex.repeatFlag  = !!XmlUtil::FindInt( tex_node, "Repeat_Flag", 0 );
        apptex.bright  = XmlUtil::FindDouble( tex_node, "Bright", 1.0 );
        apptex.alpha  = XmlUtil::FindDouble( tex_node, "Alpha", 1.0 );
        apptex.flipUFlag  = !!XmlUtil::FindInt( tex_node, "Flip_U_Flag", 0 );
        apptex.flipWFlag  = !!XmlUtil::FindInt( tex_node, "Flip_W_Flag", 0 );
        apptex.reflFlipUFlag  = !!XmlUtil::FindInt( tex_node, "Refl_Flip_U_Flag", 0 );
        apptex.reflFlipWFlag  = !!XmlUtil::FindInt( tex_node, "Refl_Flip_W_Flag", 0 );

        apptex.texID = texMgrPtr->loadTex( apptex.texStr.get_char_star() );

        if ( apptex.texID > 0 )
            appTexVec.push_back( apptex );

    }

    //==== Clean Part Vec ====//
    for (  i = 0 ; i < (int)partVec.size() ; i++ )
        delete partVec[i];

    partVec.clear();

    int numParts = XmlUtil::GetNumNames( root, "Structure_Part" );
    for ( i = 0 ; i < numParts ; i++ )
    {
        xmlNodePtr part_node = XmlUtil::GetNode( root, "Structure_Part", i );

        int type = XmlUtil::FindInt( part_node, "Type", -1 );

        Part* partPtr = structureMgrPtr->CreatePart( type );

        if ( partPtr )
        {
            partPtr->ReadParms( part_node );

            AddStructurePart( partPtr );
            partPtr->SetGeomPtr( this );

        }

    }
*/
}

void Geom::LoadMainDrawObjs( vector< DrawObj* > & draw_obj_vec )
{
    char str[256];

    for ( int i = 0; i < (int)m_WireShadeDrawObj_vec.size(); i++ )
    {
        // Symmetry drawObjs have same m_ID. Make them unique by adding index
        // at the end of m_ID.
        snprintf( str, sizeof( str ),  "_%d", i );
        m_WireShadeDrawObj_vec[i].m_GeomID = m_ID + str;
        m_WireShadeDrawObj_vec[i].m_Visible = GetSetFlag( vsp::SET_SHOWN ) && m_GuiDraw.GetDisplayType() == DISPLAY_TYPE::DISPLAY_BEZIER;

        // Set Render Destination to Main VSP Window.
        m_WireShadeDrawObj_vec[i].m_Screen = DrawObj::VSP_MAIN_SCREEN;

        Material * material = m_GuiDraw.getMaterial();

        for ( int j = 0; j < 4; j++ )
            m_WireShadeDrawObj_vec[i].m_MaterialInfo.Ambient[j] = (float)material->m_Ambi[j];

        for ( int j = 0; j < 4; j++ )
            m_WireShadeDrawObj_vec[i].m_MaterialInfo.Diffuse[j] = (float)material->m_Diff[j];

        for ( int j = 0; j < 4; j++ )
            m_WireShadeDrawObj_vec[i].m_MaterialInfo.Specular[j] = (float)material->m_Spec[j];

        for ( int j = 0; j < 4; j++ )
            m_WireShadeDrawObj_vec[i].m_MaterialInfo.Emission[j] = (float)material->m_Emis[j];

        m_WireShadeDrawObj_vec[i].m_MaterialInfo.Shininess = (float)material->m_Shininess;

        // Surfaces with vsp::CFD_TRANSPARENT set -- i.e. propeller disk.
        if ( i >= 2 )
        {
            if ( m_WireShadeDrawObj_vec[i].m_MaterialInfo.Diffuse[3] == (float)1.0 )
            {
                m_WireShadeDrawObj_vec[i].m_MaterialInfo.Diffuse[3] = 0.5;
            }
        }

        vec3d lineColor = vec3d( m_GuiDraw.GetWireColor().x() / 255.0,
                                 m_GuiDraw.GetWireColor().y() / 255.0,
                                 m_GuiDraw.GetWireColor().z() / 255.0 );

        switch ( m_GuiDraw.GetDrawType() )
        {
            case DRAW_TYPE::GEOM_DRAW_WIRE:
                m_WireShadeDrawObj_vec[i].m_LineWidth = 1.0;
                m_WireShadeDrawObj_vec[i].m_LineColor = lineColor;
                m_WireShadeDrawObj_vec[i].m_Type = DrawObj::VSP_WIRE_MESH;
                break;

            case DRAW_TYPE::GEOM_DRAW_HIDDEN:
                m_WireShadeDrawObj_vec[i].m_LineColor = lineColor;
                m_WireShadeDrawObj_vec[i].m_Type = DrawObj::VSP_WIRE_HIDDEN_MESH;
                break;

            case DRAW_TYPE::GEOM_DRAW_SHADE:
                m_WireShadeDrawObj_vec[i].m_Type = DrawObj::VSP_SHADED_MESH;
                break;

            case DRAW_TYPE::GEOM_DRAW_NONE:
                m_WireShadeDrawObj_vec[i].m_Type = DrawObj::VSP_SHADED_MESH;
                m_WireShadeDrawObj_vec[i].m_Visible = false;
                break;

            case DRAW_TYPE::GEOM_DRAW_TEXTURE:
                m_WireShadeDrawObj_vec[i].m_Type = DrawObj::VSP_TEXTURED_MESH;

                // Reload texture infos.
                m_WireShadeDrawObj_vec[i].m_TextureInfos.clear();
                vector<Texture*> texList = m_GuiDraw.getTextureMgr()->FindTextureVec( m_GuiDraw.getTextureMgr()->GetTextureVec() );
                for ( int j = 0; j < (int)texList.size(); j++ )
                {
                    DrawObj::TextureInfo info;
                    info.FileName = texList[j]->m_FileName;
                    info.UScale = (float)texList[j]->m_UScale.Get();
                    info.WScale = (float)texList[j]->m_WScale.Get();
                    info.U = (float)texList[j]->m_U.Get();
                    info.W = (float)texList[j]->m_W.Get();
                    info.Transparency = (float)texList[j]->m_Transparency.Get();
                    info.BlendTransparency = true;
                    info.UFlip = texList[j]->m_FlipU.Get();
                    info.WFlip = texList[j]->m_FlipW.Get();

                    // Modifications in 3D background, but not for general Texture's yet.  These may be enabled
                    // if Texture is enhanced, but there is no need now.
                    info.Rot = 0;
                    info.FlipLR = false;
                    info.FlipUD = false;
                    info.AutoTrans = false;
                    info.ModificationKey = 0;

                    info.ID = texList[j]->GetID();
                    m_WireShadeDrawObj_vec[i].m_TextureInfos.push_back( info );
                }
                break;
        }
        draw_obj_vec.push_back( &m_WireShadeDrawObj_vec[i] );
    }

}

//==== Load All Draw Objects ====//
void Geom::LoadDrawObjs( vector< DrawObj* > & draw_obj_vec )
{
    char str[256];

    LoadMainDrawObjs( draw_obj_vec );

    bool isactive = m_Vehicle->IsGeomActive( m_ID );

    // Load BoundingBox and Axes
    m_HighlightDrawObj.m_Screen = DrawObj::VSP_MAIN_SCREEN;
    m_HighlightDrawObj.m_GeomID = BBOXHEADER + m_ID;
    m_HighlightDrawObj.m_Visible = isactive;
    m_HighlightDrawObj.m_LineWidth = 4.0;
    m_HighlightDrawObj.m_LineColor = vec3d( 1.0, 0., 0.0 );
    m_HighlightDrawObj.m_Type = DrawObj::VSP_LINES;
    draw_obj_vec.push_back( &m_HighlightDrawObj );

    m_PtMassCGDrawObj.m_Screen = DrawObj::VSP_MAIN_SCREEN;
    m_PtMassCGDrawObj.m_GeomID = m_ID + string( "PtMassCG" );
    m_PtMassCGDrawObj.m_Visible = isactive && GetSetFlag( vsp::SET_SHOWN );
    m_PtMassCGDrawObj.m_PointSize = 10.0;
    m_PtMassCGDrawObj.m_PointColor = vec3d( 0, 0, 1 );
    m_PtMassCGDrawObj.m_Type = DrawObj::VSP_POINTS;
    draw_obj_vec.push_back( &m_PtMassCGDrawObj );

    for ( int i = 0; i < m_AxisDrawObj_vec.size(); i++ )
    {
        m_AxisDrawObj_vec[i].m_Screen = DrawObj::VSP_MAIN_SCREEN;
        snprintf( str, sizeof( str ),  "_%d", i );
        m_AxisDrawObj_vec[i].m_GeomID = m_ID + "Axis_" + str;
        m_AxisDrawObj_vec[i].m_Visible = isactive;
        m_AxisDrawObj_vec[i].m_LineWidth = 2.0;
        m_AxisDrawObj_vec[i].m_Type = DrawObj::VSP_LINES;
        draw_obj_vec.push_back( &m_AxisDrawObj_vec[i] );
    }


    // Load Feature Lines
    for ( int i = 0; i < m_FeatureDrawObj_vec.size(); i++ )
    {
        m_FeatureDrawObj_vec[i].m_Visible = m_GuiDraw.GetDisplayType() == DISPLAY_TYPE::DISPLAY_BEZIER && m_GuiDraw.GetDispFeatureFlag() && GetSetFlag( vsp::SET_SHOWN );
        draw_obj_vec.push_back( &m_FeatureDrawObj_vec[i] );
    }

    // Load Subsurfaces
    RecolorSubSurfs( SubSurfaceMgr.GetCurrSurfInd() );
    for ( int i = 0; i < (int)m_SubSurfVec.size(); i++ )
    {
        vector < DrawObj* > ssdo;

        m_SubSurfVec[i]->LoadDrawObjs( ssdo );

        for ( int j = 0; j < ssdo.size(); j++ )
        {
            ssdo[j]->m_Visible = m_GuiDraw.GetDisplayType() == DISPLAY_TYPE::DISPLAY_BEZIER && m_GuiDraw.GetDispSubSurfFlag() && GetSetFlag( vsp::SET_SHOWN );
            draw_obj_vec.push_back( ssdo[j] );
        }

    }

    if ( m_GuiDraw.GetDisplayType() == DISPLAY_TYPE::DISPLAY_DEGEN_SURF )
    {
        // Load DegenGeom
        for ( int i = 0; i < m_DegenSurfDrawObj_vec.size(); i++ )
        {
            m_DegenSurfDrawObj_vec[i].m_GeomID = m_ID + "Degen_Surf_" + std::to_string( i );
            m_DegenSurfDrawObj_vec[i].m_Visible = GetSetFlag( vsp::SET_SHOWN );

            // Set Render Destination to Main VSP Window.
            m_DegenSurfDrawObj_vec[i].m_Screen = DrawObj::VSP_MAIN_SCREEN;

            Material * material = m_GuiDraw.getMaterial();

            for ( int j = 0; j < 4; j++ )
                m_DegenSurfDrawObj_vec[i].m_MaterialInfo.Ambient[j] = (float)material->m_Ambi[j];

            for ( int j = 0; j < 4; j++ )
                m_DegenSurfDrawObj_vec[i].m_MaterialInfo.Diffuse[j] = (float)material->m_Diff[j];

            for ( int j = 0; j < 4; j++ )
                m_DegenSurfDrawObj_vec[i].m_MaterialInfo.Specular[j] = (float)material->m_Spec[j];

            for ( int j = 0; j < 4; j++ )
                m_DegenSurfDrawObj_vec[i].m_MaterialInfo.Emission[j] = (float)material->m_Emis[j];

            m_DegenSurfDrawObj_vec[i].m_MaterialInfo.Shininess = (float)material->m_Shininess;

            vec3d lineColor = vec3d( m_GuiDraw.GetWireColor().x() / 255.0,
                                        m_GuiDraw.GetWireColor().y() / 255.0,
                                        m_GuiDraw.GetWireColor().z() / 255.0 );

            switch ( m_GuiDraw.GetDrawType() )
            {
            case DRAW_TYPE::GEOM_DRAW_WIRE:
                m_DegenSurfDrawObj_vec[i].m_LineWidth = 1.0;
                m_DegenSurfDrawObj_vec[i].m_LineColor = lineColor;
                m_DegenSurfDrawObj_vec[i].m_Type = DrawObj::VSP_WIRE_QUADS;
                draw_obj_vec.push_back( &m_DegenSurfDrawObj_vec[i] );
                break;

            case DRAW_TYPE::GEOM_DRAW_HIDDEN:
                m_DegenSurfDrawObj_vec[i].m_LineColor = lineColor;
                m_DegenSurfDrawObj_vec[i].m_Type = DrawObj::VSP_WIRE_HIDDEN_QUADS;
                draw_obj_vec.push_back( &m_DegenSurfDrawObj_vec[i] );
                break;

            case DRAW_TYPE::GEOM_DRAW_SHADE:
                m_DegenSurfDrawObj_vec[i].m_Type = DrawObj::VSP_SHADED_QUADS;
                draw_obj_vec.push_back( &m_DegenSurfDrawObj_vec[i] );
                break;

            case DRAW_TYPE::GEOM_DRAW_NONE:
                m_DegenSurfDrawObj_vec[i].m_Type = DrawObj::VSP_WIRE_QUADS;
                m_DegenSurfDrawObj_vec[i].m_Visible = false;
                draw_obj_vec.push_back( &m_DegenSurfDrawObj_vec[i] );
                break;

            case DRAW_TYPE::GEOM_DRAW_TEXTURE:
                m_DegenSurfDrawObj_vec[i].m_Type = DrawObj::VSP_SHADED_QUADS;
                draw_obj_vec.push_back( &m_DegenSurfDrawObj_vec[i] );
                break;
            }
        }
    }
    else if ( m_GuiDraw.GetDisplayType() == DISPLAY_TYPE::DISPLAY_DEGEN_PLATE )
    {
        for ( int i = 0; i < m_DegenPlateDrawObj_vec.size(); i++ )
        {
            m_DegenPlateDrawObj_vec[i].m_GeomID = m_ID + "Degen_Plate_" + std::to_string( i );
            m_DegenPlateDrawObj_vec[i].m_Visible = GetSetFlag( vsp::SET_SHOWN );

            // Set Render Destination to Main VSP Window.
            m_DegenPlateDrawObj_vec[i].m_Screen = DrawObj::VSP_MAIN_SCREEN;

            Material * material = m_GuiDraw.getMaterial();

            for ( int j = 0; j < 4; j++ )
                m_DegenPlateDrawObj_vec[i].m_MaterialInfo.Ambient[j] = (float)material->m_Ambi[j];

            for ( int j = 0; j < 4; j++ )
                m_DegenPlateDrawObj_vec[i].m_MaterialInfo.Diffuse[j] = (float)material->m_Diff[j];

            for ( int j = 0; j < 4; j++ )
                m_DegenPlateDrawObj_vec[i].m_MaterialInfo.Specular[j] = (float)material->m_Spec[j];

            for ( int j = 0; j < 4; j++ )
                m_DegenPlateDrawObj_vec[i].m_MaterialInfo.Emission[j] = (float)material->m_Emis[j];

            m_DegenPlateDrawObj_vec[i].m_MaterialInfo.Shininess = (float)material->m_Shininess;

            vec3d lineColor = vec3d( m_GuiDraw.GetWireColor().x() / 255.0,
                                        m_GuiDraw.GetWireColor().y() / 255.0,
                                        m_GuiDraw.GetWireColor().z() / 255.0 );

            switch ( m_GuiDraw.GetDrawType() )
            {
            case vsp::DRAW_TYPE::GEOM_DRAW_WIRE:
                m_DegenPlateDrawObj_vec[i].m_LineWidth = 1.0;
                m_DegenPlateDrawObj_vec[i].m_LineColor = lineColor;
                m_DegenPlateDrawObj_vec[i].m_Type = DrawObj::VSP_WIRE_QUADS;
                draw_obj_vec.push_back( &m_DegenPlateDrawObj_vec[i] );
                break;

            case vsp::DRAW_TYPE::GEOM_DRAW_HIDDEN:
                m_DegenPlateDrawObj_vec[i].m_LineColor = lineColor;
                m_DegenPlateDrawObj_vec[i].m_Type = DrawObj::VSP_WIRE_HIDDEN_QUADS;
                draw_obj_vec.push_back( &m_DegenPlateDrawObj_vec[i] );
                break;

            case vsp::DRAW_TYPE::GEOM_DRAW_SHADE:
                m_DegenPlateDrawObj_vec[i].m_Type = DrawObj::VSP_SHADED_QUADS;
                draw_obj_vec.push_back( &m_DegenPlateDrawObj_vec[i] );
                break;

            case vsp::DRAW_TYPE::GEOM_DRAW_NONE:
                m_DegenPlateDrawObj_vec[i].m_Type = DrawObj::VSP_WIRE_QUADS;
                m_DegenPlateDrawObj_vec[i].m_Visible = false;
                draw_obj_vec.push_back( &m_DegenPlateDrawObj_vec[i] );
                break;

            case vsp::DRAW_TYPE::GEOM_DRAW_TEXTURE:
                m_DegenPlateDrawObj_vec[i].m_Type = DrawObj::VSP_SHADED_QUADS;
                draw_obj_vec.push_back( &m_DegenPlateDrawObj_vec[i] );
                break;
            }
        }
    }
    else if ( m_GuiDraw.GetDisplayType() == DISPLAY_TYPE::DISPLAY_DEGEN_CAMBER )
    {
        for ( int i = 0; i < m_DegenCamberPlateDrawObj_vec.size(); i++ )
        {
            m_DegenCamberPlateDrawObj_vec[i].m_GeomID = m_ID + "Degen_Camber_Plate_" + std::to_string( i );
            m_DegenCamberPlateDrawObj_vec[i].m_Visible = GetSetFlag( vsp::SET_SHOWN );

            // Set Render Destination to Main VSP Window.
            m_DegenCamberPlateDrawObj_vec[i].m_Screen = DrawObj::VSP_MAIN_SCREEN;

            Material * material = m_GuiDraw.getMaterial();

            for ( int j = 0; j < 4; j++ )
                m_DegenCamberPlateDrawObj_vec[i].m_MaterialInfo.Ambient[j] = (float)material->m_Ambi[j];

            for ( int j = 0; j < 4; j++ )
                m_DegenCamberPlateDrawObj_vec[i].m_MaterialInfo.Diffuse[j] = (float)material->m_Diff[j];

            for ( int j = 0; j < 4; j++ )
                m_DegenCamberPlateDrawObj_vec[i].m_MaterialInfo.Specular[j] = (float)material->m_Spec[j];

            for ( int j = 0; j < 4; j++ )
                m_DegenCamberPlateDrawObj_vec[i].m_MaterialInfo.Emission[j] = (float)material->m_Emis[j];

            m_DegenCamberPlateDrawObj_vec[i].m_MaterialInfo.Shininess = (float)material->m_Shininess;

            vec3d lineColor = vec3d( m_GuiDraw.GetWireColor().x() / 255.0,
                                        m_GuiDraw.GetWireColor().y() / 255.0,
                                        m_GuiDraw.GetWireColor().z() / 255.0 );

            switch ( m_GuiDraw.GetDrawType() )
            {
            case DRAW_TYPE::GEOM_DRAW_WIRE:
                m_DegenCamberPlateDrawObj_vec[i].m_LineWidth = 1.0;
                m_DegenCamberPlateDrawObj_vec[i].m_LineColor = lineColor;
                m_DegenCamberPlateDrawObj_vec[i].m_Type = DrawObj::VSP_WIRE_QUADS;
                draw_obj_vec.push_back( &m_DegenCamberPlateDrawObj_vec[i] );
                break;

            case DRAW_TYPE::GEOM_DRAW_HIDDEN:
                m_DegenCamberPlateDrawObj_vec[i].m_LineColor = lineColor;
                m_DegenCamberPlateDrawObj_vec[i].m_Type = DrawObj::VSP_WIRE_HIDDEN_QUADS;
                draw_obj_vec.push_back( &m_DegenCamberPlateDrawObj_vec[i] );
                break;

            case DRAW_TYPE::GEOM_DRAW_SHADE:
                m_DegenCamberPlateDrawObj_vec[i].m_Type = DrawObj::VSP_SHADED_QUADS;
                draw_obj_vec.push_back( &m_DegenCamberPlateDrawObj_vec[i] );
                break;

            case DRAW_TYPE::GEOM_DRAW_NONE:
                m_DegenCamberPlateDrawObj_vec[i].m_Type = DrawObj::VSP_WIRE_QUADS;
                m_DegenCamberPlateDrawObj_vec[i].m_Visible = false;
                draw_obj_vec.push_back( &m_DegenCamberPlateDrawObj_vec[i] );
                break;

            case DRAW_TYPE::GEOM_DRAW_TEXTURE:
                m_DegenCamberPlateDrawObj_vec[i].m_Type = DrawObj::VSP_SHADED_QUADS;
                draw_obj_vec.push_back( &m_DegenCamberPlateDrawObj_vec[i] );
                break;
            }
        }
    }

    if ( m_GuiDraw.GetDispSubSurfFlag() && m_GuiDraw.GetDisplayType() != DISPLAY_TYPE::DISPLAY_BEZIER )
    {
        for ( int i = 0; i < m_DegenSubSurfDrawObj_vec.size(); i++ )
        {
            m_DegenSubSurfDrawObj_vec[i].m_GeomID = m_ID + "Degen_SubSurf_" + std::to_string( i );
            m_DegenSubSurfDrawObj_vec[i].m_Visible = GetSetFlag( vsp::SET_SHOWN );

            // Set Render Destination to Main VSP Window.
            m_DegenSubSurfDrawObj_vec[i].m_Screen = DrawObj::VSP_MAIN_SCREEN;

            m_DegenSubSurfDrawObj_vec[i].m_LineWidth = 2.0;
            m_DegenSubSurfDrawObj_vec[i].m_LineColor = vec3d( 0.0, 0.0, 0.0 );

            m_DegenSubSurfDrawObj_vec[i].m_Type = DrawObj::VSP_LINE_STRIP;

            draw_obj_vec.push_back( &m_DegenSubSurfDrawObj_vec[i] );
        }
    }
}

void Geom::SetColor( double r, double g, double b )
{
    m_GuiDraw.SetWireColor( r, g, b );
}

vec3d Geom::GetColor() const
{
    return m_GuiDraw.GetWireColor();
}

void Geom::SetMaterialToDefault()
{
    m_GuiDraw.SetMaterialToDefault();
}

void Geom::SetMaterial( const std::string &name, double ambi[], double diff[], double spec[], double emis[], double shin )
{
    m_GuiDraw.SetMaterial( name, ambi, diff, spec, emis, shin );
}

Material * Geom::GetMaterial()
{
    return m_GuiDraw.getMaterial();
}

//==== Create Degenerate Geometry ====//
// When preview = true, this simplifies to generate only the
// required degen plate,surface, and subsurface for updating the preview DrawObj vectors
void Geom::CreateDegenGeom( vector<DegenGeom> &dgs, bool preview, const int & n_ref )
{
    CreateDegenGeom( m_SurfVec, GetNumTotalSurfs(), dgs, preview, n_ref );
}

void Geom::CreateMainDegenGeomPreview( vector<DegenGeom> &dgs )
{
    CreateDegenGeom( m_MainSurfVec, GetNumMainSurfs(),dgs, true, 1 );
}

void Geom::CreateDegenGeom( VspSurf &surf, int isurf, DegenGeom &degenGeom, bool preview, const int & n_ref )
{
    vector< vector< vec3d > > pnts;
    vector< vector< vec3d > > nrms;
    vector< vector< vec3d > > uwpnts;

    bool urootcap = false;

    surf.ResetUSkip();
    if ( m_CapUMinSuccess[ m_SurfIndxVec[isurf] ] )
    {
        int nskip = 1;
        if ( m_CapUMinOption() >= ROUND_EXT_END_CAP_NONE )
        {
            nskip++;
        }
        surf.SetUSkipFirst( nskip, true );
        urootcap = true;
    }
    if ( m_CapUMaxSuccess[ m_SurfIndxVec[isurf] ] )
    {
        int nskip = 1;
        if ( m_CapUMaxOption() >= ROUND_EXT_END_CAP_NONE )
        {
            nskip++;
        }
        surf.SetUSkipLast( nskip, true );
    }

    //==== Tesselate Surface ====//
    UpdateTesselate( surf, m_CapUMinSuccess[ m_SurfIndxVec[isurf] ], m_CapUMaxSuccess[ m_SurfIndxVec[isurf] ], true, pnts, nrms, uwpnts, n_ref );
    surf.ResetUSkip();

    int surftype = DegenGeom::BODY_TYPE;
    if( surf.GetSurfType() == vsp::WING_SURF )
    {
        surftype = DegenGeom::SURFACE_TYPE;
    }
    else if( surf.GetSurfType() == vsp::DISK_SURF )
    {
        surftype = DegenGeom::DISK_TYPE;
    }

    CreateDegenGeom( degenGeom, pnts, nrms, uwpnts, urootcap, isurf, preview, surf.GetFlipNormal(), surftype, surf.GetSurfCfdType(), surf.GetFoilSurf() );
}

void Geom::CreateDegenGeom( vector <VspSurf> &surf_vec, const int &nsurf, vector<DegenGeom> &dgs, bool preview, const int & n_ref )
{
    dgs.resize( nsurf );
    for ( int i = 0 ; i < nsurf ; i++ )
    {
        CreateDegenGeom( surf_vec[i], i, dgs[i], preview, n_ref );
    }
}

void Geom::CreateDegenGeom( DegenGeom &degenGeom, const vector< vector< vec3d > > &pnts, const vector< vector< vec3d > > &nrms, const vector< vector< vec3d > > &uwpnts,
                            bool urootcap, int isurf, bool preview, bool flipnormal, int surftype, int cfdsurftype, VspSurf *fs )
{
    degenGeom.setParentGeom( this );
    degenGeom.setSurfNum( isurf );
    degenGeom.setFlipNormal( flipnormal );
    degenGeom.setMainSurfInd( m_SurfIndxVec[isurf] );
    degenGeom.setSymCopyInd( m_SurfCopyIndx[isurf] );
    degenGeom.setCfdSurfType( cfdsurftype );

    vector < double > tmatvec( 16 );
    for ( int j = 0; j < 16; j++ )
    {
        tmatvec[j] = m_TransMatVec[isurf].data()[ j ];
    }
    degenGeom.setTransMat( tmatvec );

    degenGeom.setNumXSecs( pnts.size() );
    if ( pnts.size() > 0 )
    {
        degenGeom.setNumPnts( pnts[0].size() );
    }
    else
    {
        degenGeom.setNumPnts( 0 );
        return;
    }
    degenGeom.setName( GetName() );

    degenGeom.createDegenSurface( pnts, uwpnts, flipnormal );

    if( surftype == DegenGeom::SURFACE_TYPE )
    {
        degenGeom.setType(DegenGeom::SURFACE_TYPE);

        degenGeom.createSurfDegenPlate( pnts, uwpnts );
        if ( !preview )
        {
            degenGeom.createSurfDegenStick( pnts, uwpnts, fs, urootcap );
        }
    }
    else if( surftype == DegenGeom::DISK_TYPE )
    {
        degenGeom.setType(DegenGeom::DISK_TYPE);

        if ( !preview )
        {
            degenGeom.createDegenDisk( pnts, flipnormal );
        }
    }
    else
    {
        degenGeom.setType(DegenGeom::BODY_TYPE);

        degenGeom.createBodyDegenPlate( pnts, uwpnts );

        if ( !preview )
        {
            degenGeom.createBodyDegenStick( pnts, uwpnts );
        }
    }

    // degenerate subsurfaces
    for ( int j = 0; j < m_SubSurfVec.size(); j++ )
    {
        if ( m_SubSurfVec[j]->m_MainSurfIndx() == -1 || m_SurfIndxVec[isurf] == m_SubSurfVec[j]->m_MainSurfIndx() )
        {
            degenGeom.addDegenSubSurf( m_SubSurfVec[j], isurf );    //TODO is there a way to eliminate having to send in the surf index "i"

            if ( !preview )
            {
                SSControlSurf *csurf = dynamic_cast < SSControlSurf* > ( m_SubSurfVec[j] );
                if ( csurf )
                {
                    degenGeom.addDegenHingeLine( csurf, isurf );
                }
            }
        }
    }
}

//==== Set Sym Flag ====//
int Geom::GetSymFlag() const
{
    return m_SymPlanFlag() | m_SymAxFlag();
}

//==== Return Pointer to Surface indx ====//
const VspSurf* Geom::GetSurfPtr( int indx ) const
{
    if ( indx >= 0 && indx < GetNumTotalSurfs() )
    {
        return &m_SurfVec[ indx ];
    }
    assert( true );
    return nullptr;
}

//==== Return Pointer to Surface indx ====//
const VspSurf* Geom::GetMainSurfPtr( int indx ) const
{
    if ( indx >= 0 && indx < GetNumMainSurfs() )
    {
        return &m_MainSurfVec[ indx ];
    }
    assert( true );
    return nullptr;
}

// This is a massive layering violation.  It requires knowledge of the numeric values of the different enums.
// This could instead be a pure virtual method that is required to be implemented by each concrete Geom type.
// Or an optional method implemented by only non-surface types.
bool Geom::isNonSurfaceType()
{
    if ( m_Type.m_Type == BLANK_GEOM_TYPE ||
         m_Type.m_Type == HINGE_GEOM_TYPE ||
         m_Type.m_Type == HUMAN_GEOM_TYPE ||
         m_Type.m_Type == MESH_GEOM_TYPE ||
         m_Type.m_Type == NGON_GEOM_TYPE ||
         m_Type.m_Type == PT_CLOUD_GEOM_TYPE ||
         m_Type.m_Type == WIRE_FRAME_GEOM_TYPE ||
         GetNumMainSurfs() == 0 )
    {
        return true;
    }
    return false;
}

//==== Count Number of Sym Surfaces ====//
int Geom::GetNumTotalSurfs() const
{
    return GetNumSymmCopies() * GetNumMainSurfs();
}

int Geom::GetNumTotalHrmSurfs() const
{
    return GetNumTotalSurfs();
}

//==== Count Number of Sym Copies of Each Surface ====//
int Geom::GetNumSymmCopies() const
{
    int symFlag = GetSymFlag();
    int numSymCopies = 1;
    for ( int i = 0; i < SYM_NUM_TYPES ; i++ )
    {
        if ( symFlag & ( 1 << i ) )
        {
            if ( i < SYM_PLANAR_TYPES )
            {
                numSymCopies *= 2;
            }
            else
            {
                numSymCopies += ( numSymCopies ) * ( m_SymRotN() - 1 );
            }
        }
    }
    return numSymCopies;
}

//==== Count Number of Sym Flags ====//
int Geom::GetNumSymFlags() const
{
    int numSymFlags = 0;
    int symFlag = GetSymFlag();
    for ( int i = 0; i < SYM_NUM_TYPES ; i++ )
    {
        if ( symFlag & ( 1 << i ) )
        {
            numSymFlags++;
        }
    }
    return numSymFlags;
}

vector<VspSurf> Geom::GetDegenSurfVec()
{
    vector<VspSurf> degen_surf_vec;
    degen_surf_vec.reserve( m_SurfVec.size() * 2 );
    for ( int i = 0; i < m_SurfVec.size(); i++ )
    {
        if ( m_SurfVec[i].GetSurfType() == vsp::WING_SURF )
        {
            int isurf = degen_surf_vec.size();
            degen_surf_vec.push_back( m_SurfVec[i] ); // Start with copy.
            degen_surf_vec[isurf].DegenCamberSurf( m_SurfVec[i] );
            degen_surf_vec[isurf].SetPlateNum( 0 );
        }
        else
        {
            int isurf = degen_surf_vec.size();
            degen_surf_vec.push_back( m_SurfVec[i] ); // Start with copy.
            degen_surf_vec[isurf].DegenPlanarSurf( m_SurfVec[i], 1 ); // Vertical
            degen_surf_vec[isurf].SetPlateNum( 0 );

            isurf = degen_surf_vec.size();
            degen_surf_vec.push_back( m_SurfVec[i] ); // Start with copy.
            degen_surf_vec[isurf].DegenPlanarSurf( m_SurfVec[i], 0 ); // Horizontal
            degen_surf_vec[isurf].SetPlateNum( 1 );
        }
    }
    return degen_surf_vec;
}

int Geom::GetSurfType( int indx ) const
{
    if ( indx >=0 && indx < m_SurfIndxVec.size() )
    {
        return GetMainSurfType( m_SurfIndxVec[indx] );
    }
    return vsp::NORMAL_SURF;
}

int Geom::GetMainSurfType( int indx ) const
{
    if ( indx >=0 && indx < m_MainSurfVec.size() )
    {
        return m_MainSurfVec[indx].GetSurfType();
    }
    return vsp::NORMAL_SURF;
}

int Geom::GetMainCFDSurfType( int indx ) const
{
    if ( indx >=0 && indx < m_MainSurfVec.size() )
    {
        return m_MainSurfVec[indx].GetSurfCfdType();
    }
    return vsp::CFD_NORMAL;
}

bool Geom::GetFlipNormal( int indx ) const
{
    if ( indx >=0 && indx < m_FlipNormalVec.size() )
    {
        return m_FlipNormalVec[indx];
    }
    return false;
}

bool Geom::GetMainFlipNormal( int indx ) const
{
    if ( indx >=0 && indx < m_MainSurfVec.size() )
    {
        return m_MainSurfVec[indx].GetFlipNormal();
    }
    return false;
}

double Geom::GetUMax( int indx ) const
{
    if ( indx >=0 && indx < m_SurfIndxVec.size() )
    {
        return GetMainUMax( m_SurfIndxVec[indx] );
    }
    return 0.0;
}

double Geom::GetMainUMax( int indx ) const
{
    if ( indx >=0 && indx < m_MainSurfVec.size() )
    {
        return m_MainSurfVec[indx].GetUMax();
    }
    return 0.0;
}

double Geom::GetUMapMax( int indx ) const
{
    if ( indx >=0 && indx < m_SurfIndxVec.size() )
    {
        return GetMainUMapMax( m_SurfIndxVec[indx] );
    }
    return 0.0;
}

double Geom::GetMainUMapMax( int indx ) const
{
    if ( indx >=0 && indx < m_MainSurfVec.size() )
    {
        return m_MainSurfVec[indx].GetUMapMax();
    }
    return 0.0;
}

double Geom::GetWMax( int indx ) const
{
    if ( indx >=0 && indx < m_SurfIndxVec.size() )
    {
        return GetMainWMax( m_SurfIndxVec[indx] );
    }
    return 0.0;
}

double Geom::GetMainWMax( int indx ) const
{
    if ( indx >=0 && indx < m_MainSurfVec.size() )
    {
        return m_MainSurfVec[indx].GetWMax();
    }
    return 0.0;
}

vec3d Geom::CompPnt01(const int &indx, const double &u, const double &w)
{
    return GetSurfPtr( indx )->CompPnt01( u, w );
}

vec3d Geom::CompTanU( const int &indx, const double &u, const double &w )
{
    if ( !GetSurfPtr( indx ) )
    {
        return vec3d();
    }
    double uu = clamp( u, 0.0, GetSurfPtr( indx )->GetUMax() );
    double ww = clamp( w, 0.0, GetSurfPtr( indx )->GetWMax() );
    return GetSurfPtr( indx )->CompTanU( uu, ww );
}

vec3d Geom::CompTanW( const int &indx, const double &u, const double &w )
{
    if ( !GetSurfPtr( indx ) )
    {
        return vec3d();
    }
    double uu = clamp( u, 0.0, GetSurfPtr( indx )->GetUMax() );
    double ww = clamp( w, 0.0, GetSurfPtr( indx )->GetWMax() );
    return GetSurfPtr( indx )->CompTanW( uu, ww );
}

vec3d Geom::CompPntRST( const int &indx, const double &r, const double &s, const double &t )
{
    if ( !GetSurfPtr( indx ) )
    {
        return vec3d();
    }
    return GetSurfPtr( indx )->CompPntRST( r, s, t );
}

vec3d Geom::CompTanR( const int &indx, const double &r, const double &s, const double &t )
{
    if ( !GetSurfPtr( indx ) )
    {
        return vec3d();
    }
    return GetSurfPtr( indx )->CompTanR( r, s, t );
}

vec3d Geom::CompTanS( const int &indx, const double &r, const double &s, const double &t )
{
    if ( !GetSurfPtr( indx ) )
    {
        return vec3d();
    }
    return GetSurfPtr( indx )->CompTanS( r, s, t );
}

vec3d Geom::CompTanT( const int &indx, const double &r, const double &s, const double &t )
{
    if ( !GetSurfPtr( indx ) )
    {
        return vec3d();
    }
    return GetSurfPtr( indx )->CompTanT( r, s, t );
}

bool Geom::CompRotCoordSys( const int &indx, const double &u, const double &w, Matrix4d &rotmat )
{
    const VspSurf* surf_ptr = GetSurfPtr( indx );
    if ( surf_ptr && surf_ptr->GetNumSectU() > 0 && surf_ptr->GetNumSectW() > 0 )
    {
        rotmat = surf_ptr->CompRotCoordSys( u, w );
        return true;
    }
    return false;
}

bool Geom::CompTransCoordSys( const int &indx, const double &u, const double &w, Matrix4d &transmat )
{
    const VspSurf* surf_ptr = GetSurfPtr( indx );
    if ( surf_ptr && surf_ptr->GetNumSectU() > 0 && surf_ptr->GetNumSectW() > 0 )
    {
        transmat = surf_ptr->CompTransCoordSys( u, w );
        return true;
    }
    return false;
}

bool Geom::CompRotCoordSysRST( const int &indx, const double &r, const double &s, const double &t, Matrix4d &rotmat )
{
    const VspSurf* surf_ptr = GetSurfPtr( indx );
    if ( surf_ptr && surf_ptr->GetNumSectU() > 0 && surf_ptr->GetNumSectW() > 0 )
    {
        rotmat = surf_ptr->CompRotCoordSysRST( r, s, t );
        return true;
    }
    return false;
}

bool Geom::CompTransCoordSysRST( const int &indx, const double &r, const double &s, const double &t, Matrix4d &transmat )
{
    const VspSurf* surf_ptr = GetSurfPtr( indx );
    if ( surf_ptr && surf_ptr->GetNumSectU() > 0 && surf_ptr->GetNumSectW() > 0 )
    {
        transmat = surf_ptr->CompTransCoordSysRST( r, s, t );
        return true;
    }
    return false;
}

bool Geom::CompRotCoordSysLMN( const int &indx, const double &l, const double &m, const double &n, Matrix4d &rotmat )
{
    const VspSurf* surf_ptr = GetSurfPtr( indx );
    if ( surf_ptr && surf_ptr->GetNumSectU() > 0 && surf_ptr->GetNumSectW() > 0 )
    {
        rotmat = surf_ptr->CompRotCoordSysLMN( l, m, n );
        return true;
    }
    return false;
}

bool Geom::CompTransCoordSysLMN( const int &indx, const double &l, const double &m, const double &n, Matrix4d &transmat )
{
    const VspSurf* surf_ptr = GetSurfPtr( indx );
    if ( surf_ptr && surf_ptr->GetNumSectU() > 0 && surf_ptr->GetNumSectW() > 0 )
    {
        transmat = surf_ptr->CompTransCoordSysLMN( l, m, n );
        return true;
    }
    return false;
}

void Geom::ConvertRSTtoLMN( const int &indx, const double &r, const double &s, const double &t, double &l_out, double &m_out, double &n_out )
{
    const VspSurf* surf_ptr = GetSurfPtr( indx );
    if ( surf_ptr && surf_ptr->GetNumSectU() > 0 && surf_ptr->GetNumSectW() > 0 )
    {
        surf_ptr->ConvertRSTtoLMN( r, s, t, l_out, m_out, n_out );
        return;
    }
    l_out = r;
    m_out = s * 2.0;
    n_out = t;
}

void Geom::ConvertRtoL( const int &indx, const double &r, double &l_out )
{
    const VspSurf* surf_ptr = GetSurfPtr( indx );
    if ( surf_ptr && surf_ptr->GetNumSectU() > 0 && surf_ptr->GetNumSectW() > 0 )
    {
        surf_ptr->ConvertRtoL( r, l_out );
        return;
    }
    l_out = r;
}

void Geom::ConvertLMNtoRST( const int &indx, const double &l, const double &m, const double &n, double &r, double &s, double &t )
{
    const VspSurf* surf_ptr = GetSurfPtr( indx );
    if ( surf_ptr && surf_ptr->GetNumSectU() > 0 && surf_ptr->GetNumSectW() > 0 )
    {
        surf_ptr->ConvertLMNtoRST( l, m, n, r, s, t );
        return;
    }
    r = l;
    s = m/2.0;
    t = n;
}

void Geom::ConvertLtoR( const int &indx, const double &l, double &r_out )
{
    const VspSurf* surf_ptr = GetSurfPtr( indx );
    if ( surf_ptr && surf_ptr->GetNumSectU() > 0 && surf_ptr->GetNumSectW() > 0 )
    {
        surf_ptr->ConvertLtoR( l, r_out );
        return;
    }
    r_out = l;
}

double Geom::ProjPnt01I( const vec3d & pt, int &surf_indx, double &u, double &w )
{
    double tol = 1e-12;

    double dmin = std::numeric_limits<double>::max();

    int nsurf = GetNumTotalSurfs();
    for ( int i = 0; i < nsurf; i++ )
    {
        double utest, wtest;

        double d = GetSurfPtr(i)->FindNearest01( utest, wtest, pt );

        if ( d < dmin )
        {
            dmin = d;
            u = utest;
            w = wtest;
            surf_indx = i;

            if ( d < tol )
            {
                break;
            }
        }
    }

    return dmin;
}

double Geom::AxisProjPnt01I( const int &iaxis, const vec3d &pt, int &surf_indx_out, double &u_out, double &w_out )
{
    double idmin = std::numeric_limits<double>::max();

    bool converged = false;

    int nsurf = GetNumTotalSurfs();
    for ( int i = 0; i < nsurf; i++ )
    {
        double utest, wtest;

        double id = GetSurfPtr( i )->ProjectPt01( pt, iaxis, utest, wtest );

        if ( id >= 0 && id < idmin )
        {
            idmin = id;
            u_out = utest;
            w_out = wtest;
            surf_indx_out = i;
            converged = true;
        }
    }

    if ( converged )
    {
        return idmin;
    }

    u_out = -1;
    w_out = -1;
    surf_indx_out = -1;
    idmin = -1;

    return idmin;
}

void Geom::WriteBezierAirfoil( const string & file_name, double foilsurf_u_location )
{
    // This function writes out the all Bezier segments (order, t0, tend, and control points)
    //  for a single airfoil (constant u curve). Input foilsurf_u_location must be 0 <= u <= 1

    FILE* file_id = fopen( file_name.c_str(), "w" );
    Vehicle* veh = VehicleMgr.GetVehicle();
    if ( !file_id || !veh || m_MainSurfVec.size() == 0 || foilsurf_u_location < 0.0 || foilsurf_u_location > 1.0 )
    {
        return;
    }

    // Get the untwisted wing surface
    VspSurf* foil_surf = m_MainSurfVec[0].GetFoilSurf();

    // Get the unit length airfoil curve
    VspCurve foil_curve;
    foil_surf->GetU01ConstCurve( foil_curve, foilsurf_u_location );

    fprintf( file_id, "File Name, %s\n", file_name.c_str() );

    // Get the Bezier segments
    vector < BezierSegment > seg_vec = foil_curve.GetBezierSegments();

    fprintf( file_id, "Num Bezier Seg, %zu\n", seg_vec.size() );

    fprintf( file_id, "# Order, t_0, t_end, Ctrl Pnt X1, Ctrl Pnt Y1, Ctrl Pnt X2, Ctrl Pnt Y2, ...\n" );

    for ( size_t j = 0; j < seg_vec.size(); j++ )
    {
        fprintf( file_id, "%d, %f, %f", seg_vec[j].order, seg_vec[j].t0, seg_vec[j].tmax );

        for ( size_t i = 0; i < seg_vec[j].control_pnt_vec.size(); i++ )
        {
            fprintf( file_id, ", %17.16f, %17.16f", seg_vec[j].control_pnt_vec[i].x(), seg_vec[j].control_pnt_vec[i].y() );
        }

        fprintf( file_id, "\n" );
    }

    fprintf( file_id, "\n" );
    fclose( file_id );
}

void Geom::WriteSeligAirfoil( const string & file_name, double foilsurf_u_location )
{
    // This function writes out the all coordinate points in Selig format for a single
    //  airfoil (constant u curve). Input foilsurf_u_location must be 0 <= u <= 1

    FILE* file_id = fopen( file_name.c_str(), "w" );
    if ( !file_id || m_MainSurfVec.size() == 0 || foilsurf_u_location < 0.0 || foilsurf_u_location > 1.0 )
    {
        return;
    }

    vector < vec3d > ordered_vec = GetAirfoilCoordinates( foilsurf_u_location );

    fprintf( file_id, "%s\n", file_name.c_str() ); // Write file name as header

    for ( size_t i = 0; i < ordered_vec.size(); i++ )
    {
        fprintf( file_id, "%17.16f %17.16f\n", ordered_vec[i].x(), ordered_vec[i].y() );
    }

    fclose( file_id );
}

vector < vec3d > Geom::GetAirfoilCoordinates( double foilsurf_u_location )
{
    // This function returns the coordinate points of a Foil Surf airfoil in Selig format.
    //  Input foilsurf_u_location must be 0 <= u <= 1

    vector < vec3d > ordered_vec;

    if ( m_MainSurfVec.size() == 0 || foilsurf_u_location < 0.0 || foilsurf_u_location > 1.0 )
    {
        return ordered_vec;
    }

    VspSurf* foil_surf = m_MainSurfVec[0].GetFoilSurf();

    VspCurve foil_curve;
    foil_surf->GetU01ConstCurve( foil_curve, foilsurf_u_location );

    // Get V Tessellation values
    vector < double > Vtess, Vup, Vlow;
    foil_surf->MakeVTess( m_TessW(), Vtess, m_CapUMinTess(), false );

    // Identify upper and lower Tessellation values
    vector < vec3d > lower_pnts, upper_pnts;

    for ( size_t i = 1; i < Vtess.size() - 1; i++ ) // Note: LE and TE not included
    {
        if ( Vtess[i] < ( 2.0 - FLT_EPSILON ) )
        {
            Vlow.push_back( Vtess[i] );
        }
        else if ( Vtess[i] > ( 2.0 + FLT_EPSILON ) )
        {
            Vup.push_back( Vtess[i] );
        }
    }

    // Tessellate along upper and lower V values
    foil_curve.Tesselate( Vlow, lower_pnts );
    foil_curve.Tesselate( Vup, upper_pnts );

    // Sort in ascending/descending order
    std::reverse( upper_pnts.begin(), upper_pnts.end() );
    std::reverse( lower_pnts.begin(), lower_pnts.end() );

    ordered_vec.resize( upper_pnts.size() + lower_pnts.size() + 3 );

    // Identify TE/LE
    vec3d TE_pnt = foil_curve.CompPnt01( 0.0 );
    vec3d LE_pnt = foil_curve.CompPnt01( 0.5 );

    // organize the coordinate points into a single vector
    ordered_vec[0] = TE_pnt; // Start at TE

    for ( size_t i = 0; i < upper_pnts.size(); i++ )
    {
        ordered_vec[i + 1] = upper_pnts[i];
    }

    ordered_vec[upper_pnts.size() + 1] = LE_pnt; // Include LE

    for ( size_t i = 0; i < lower_pnts.size(); i++ )
    {
        ordered_vec[i + upper_pnts.size() + 2] = lower_pnts[i];
    }

    ordered_vec[upper_pnts.size() + lower_pnts.size() + 2] = TE_pnt; // End at TE

    return ordered_vec;
}

void Geom::WriteAirfoilFiles( FILE* meta_fid )
{
    // This function writes out the Bezier control points for all untwisted unit length airfoils.
    Vehicle* veh = VehicleMgr.GetVehicle();
    if ( !veh || !meta_fid || m_MainSurfVec.size() == 0 )
    {
        return;
    }

    // Adjust Tessellation
    if ( veh->m_AFExportType() == vsp::SELIG_AF_EXPORT && std::abs( veh->m_AFWTessFactor() - 1.0 ) >= FLT_EPSILON )
    {
        m_TessW.Set( m_TessW() * veh->m_AFWTessFactor() );
    }

    // Get sectional U Tessellation for interpolated airfoils. Ignore end caps
    vector < int > utess_vec;
    int num_foil = 0;

    string xsecsurf_id;
    int numXsurf = GetNumXSecSurfs(); // Should always be 1 for wing and prop geoms

    for ( size_t i = 0; i < numXsurf; i++ )
    {
        XSecSurf* sec_surf = GetXSecSurf( i );
        if ( sec_surf )
        {
            xsecsurf_id = sec_surf->GetID();

            int numXsec = sec_surf->NumXSec();

            for ( size_t j = 1; j < numXsec; j++ )
            {
                XSec* sec = sec_surf->FindXSec( j );
                if ( sec )
                {
                    utess_vec.push_back( sec->m_SectTessU() );

                    if ( j > 1 ) // Only include airfoils at start and end of XSec for first XSec
                    {
                        num_foil += sec->m_SectTessU() - 1;
                    }
                    else
                    {
                        num_foil += sec->m_SectTessU();
                    }
                }
            }
        }
    }

    // Identify starting and ending U values from end cap options
    double Umin = 0;
    if ( m_CapUMinOption() != NO_END_CAP && m_CapUMinSuccess[0] )
    {
        Umin = 1.0;
    }

    double Umax = m_MainSurfVec[0].GetUMax();
    if ( m_CapUMaxOption() != NO_END_CAP && m_CapUMaxSuccess[0] )
    {
        Umax -= 1.0;
    }

    // Get the untwisted wing surface
    VspSurf* foil_surf = m_MainSurfVec[0].GetFoilSurf();

    int numU = foil_surf->GetNumSectU();

    double ustep = ( ( Umax - Umin ) / numU ) / m_MainSurfVec[0].GetUMax();
    double umin = Umin / m_MainSurfVec[0].GetUMax();

    double Vmin = 0.0;
    double Vmax = GetMainWMax(0);
    double Vle = ( Vmin + Vmax ) * 0.5;

    int foil_cnt = 0;
    int xsec_cnt = 0;

    for ( size_t i = 0; i < numU; i++ )
    {
        double sec_umin = (double)i / numU;
        double sec_umax = (double)( i + 1 ) / numU;

        size_t j = 1;
        if ( i == 0 )
        {
            j = 0; // Only include airfoils at start and end of XSec for first XSec
        }

        for ( /* j */; j < utess_vec[i]; j++ )
        {
            string af_file_name = m_Name + "_";

            if ( veh->m_AFAppendGeomIDFlag() )
            {
                af_file_name += ( m_ID + "_" );
            }

            af_file_name += to_string( foil_cnt );

            if ( veh->m_AFExportType() == vsp::SELIG_AF_EXPORT )
            {
                af_file_name += ".dat";
            }
            else if ( veh->m_AFExportType() == vsp::BEZIER_AF_EXPORT )
            {
                af_file_name += ".bz";
            }

            double u = sec_umin + ( j * ( sec_umax - sec_umin ) / ( utess_vec[i] - 1 ) );

            bool xsec_flag = false;
            if ( j == 0 || j == utess_vec[i] - 1 )
            {
                xsec_flag = true;
            }

            fprintf( meta_fid, "########################################\n" );
            fprintf( meta_fid, "Airfoil File Name, %s\n", af_file_name.c_str() );
            fprintf( meta_fid, "Geom Name, %s\n", m_Name.c_str() );
            fprintf( meta_fid, "Geom ID, %s\n", m_ID.c_str() );
            fprintf( meta_fid, "Airfoil Index, %d\n", foil_cnt );
            fprintf( meta_fid, "XSec Flag, %d\n", xsec_flag );

            if ( xsec_flag )
            {
                fprintf( meta_fid, "XSec Index, %d\n", xsec_cnt );
                fprintf( meta_fid, "XSecSurf ID, %s\n", xsecsurf_id.c_str() );
            }

            double u_global = umin + ( i  * ustep ) + ( ustep * ( (double)j / ( utess_vec[i] - 1.0 ) ) ); // Get u value on main surface

            vec3d le_pnt = m_MainSurfVec[0].CompPnt01( u_global, ( Vle / Vmax ) );
            vec3d te_pnt = m_MainSurfVec[0].CompPnt01( u_global, ( Vmin / Vmax ) );
            double chord = dist( le_pnt, te_pnt );

            fprintf( meta_fid, "FoilSurf u Value, %f\n", u );
            fprintf( meta_fid, "Global u Value, %f\n", u_global );
            fprintf( meta_fid, "Leading Edge Point, %f, %f, %f\n", le_pnt.x(), le_pnt.y(), le_pnt.z() );
            fprintf( meta_fid, "Trailing Edge Point, %f, %f, %f\n", te_pnt.x(), te_pnt.y(), te_pnt.z() );
            fprintf( meta_fid, "Chord, %f\n", chord );
            fprintf( meta_fid, "########################################\n\n" );

            if ( veh->m_AFExportType() == vsp::SELIG_AF_EXPORT )
            {
                WriteSeligAirfoil( ( veh->m_AFFileDir + af_file_name ), u );
            }
            else if ( veh->m_AFExportType() == vsp::BEZIER_AF_EXPORT )
            {
                WriteBezierAirfoil( ( veh->m_AFFileDir + af_file_name ), u );
            }

            foil_cnt++;

            if ( xsec_flag )
            {
                xsec_cnt++;
            }
        }
    }

    // Restore Tessellation
    if ( veh->m_AFExportType() == vsp::SELIG_AF_EXPORT && std::abs( veh->m_AFWTessFactor() - 1.0 ) >= FLT_EPSILON )
    {
        m_TessW.Set( m_TessW.GetLastVal() );
    }
}

void Geom::WriteXSecFile( int geom_no, FILE* dump_file )
{
    for ( int i = 0 ; i < GetNumTotalSurfs() ; i++ )
    {
        //==== Tessellate Surface ====//
        vector< vector< vec3d > > pnts;
        vector< vector< vec3d > > norms;
        vector< vector< vec3d > > uw_pnts;

        UpdateTesselate( m_SurfVec[i], m_CapUMinSuccess[ m_SurfIndxVec[i] ], m_CapUMaxSuccess[ m_SurfIndxVec[i] ], false, pnts, norms,
                         uw_pnts);

        //==== Write XSec Header ====//
        fprintf( dump_file, "\n" );
        fprintf( dump_file, "%s \n", ( char* ) m_Name.c_str() );
        fprintf( dump_file, " GROUP NUMBER      = %d \n", geom_no );

        if( GetSurfType(i) == vsp::WING_SURF )
        {
            fprintf( dump_file, " TYPE              = 0  \n" );         // 1 -- Non Lifting, 0 -- Lifting
        }
        else
        {
            fprintf( dump_file, " TYPE              = 1  \n" );         // 1 -- Non Lifting, 0 -- Lifting
        }

        fprintf( dump_file, " CROSS SECTIONS    = %d \n", static_cast<int>( pnts.size() ) );
        fprintf( dump_file, " PTS/CROSS SECTION = %d \n", static_cast<int>( pnts[0].size() ) );

        //==== Write XSec Data ====//
        for ( int j = 0 ; j < ( int )pnts.size() ; j++ )
        {
            for ( int k = 0 ; k < ( int )pnts[j].size() ; k++ )
            {
                fprintf( dump_file, "%25.17e  %25.17e  %25.17e\n", pnts[j][k].x(), pnts[j][k].y(), pnts[j][k].z() );
            }
        }
    }
}

void Geom::WritePLOT3DFileExtents( FILE* dump_file )
{
    for ( int i = 0 ; i < GetNumTotalSurfs() ; i++ )
    {
        //==== Tessellate Surface ====//
        vector< vector< vec3d > > pnts;
        vector< vector< vec3d > > norms;
        vector< vector< vec3d > > uw_pnts;

        UpdateTesselate( m_SurfVec[i], m_CapUMinSuccess[ m_SurfIndxVec[i] ], m_CapUMaxSuccess[ m_SurfIndxVec[i] ], false, pnts, norms,
                         uw_pnts);
        //==== Write surface boundary extents ====//
        fprintf( dump_file, " %d %d %d\n", static_cast<int>( pnts[0].size() ), static_cast<int>( pnts.size() ), 1 );
    }
}

void Geom::WritePLOT3DFileXYZ( FILE* dump_file )
{
    for ( int i = 0 ; i < GetNumTotalSurfs() ; i++ )
    {
        //==== Tessellate Surface ====//
        vector< vector< vec3d > > pnts;
        vector< vector< vec3d > > norms;
        vector< vector< vec3d > > uw_pnts;

        UpdateTesselate( m_SurfVec[i], m_CapUMinSuccess[ m_SurfIndxVec[i] ], m_CapUMaxSuccess[ m_SurfIndxVec[i] ], false, pnts, norms,
                         uw_pnts);

        //==== Write XSec Data ====//
        for ( int j = 0 ; j < ( int )pnts.size() ; j++ )
        {
            for ( int k = 0 ; k < ( int )pnts[j].size() ; k++ )
            {
                int kflip = k;
                if ( !GetFlipNormal(i) )
                {
                    kflip = pnts[j].size() - 1 - k;
                }
                fprintf( dump_file, "%25.17e ", pnts[j][kflip].x() );
            }
        }
        fprintf( dump_file, "\n" );
        for ( int j = 0 ; j < ( int )pnts.size() ; j++ )
        {
            for ( int k = 0 ; k < ( int )pnts[j].size() ; k++ )
            {
                int kflip = k;
                if ( !GetFlipNormal(i) )
                {
                    kflip = pnts[j].size() - 1 - k;
                }
                fprintf( dump_file, "%25.17e ", pnts[j][kflip].y() );
            }
        }
        fprintf( dump_file, "\n" );
        for ( int j = 0 ; j < ( int )pnts.size() ; j++ )
        {
            for ( int k = 0 ; k < ( int )pnts[j].size() ; k++ )
            {
                int kflip = k;
                if ( !GetFlipNormal(i) )
                {
                    kflip = pnts[j].size() - 1 - k;
                }
                fprintf( dump_file, "%25.17e ", pnts[j][kflip].z() );
            }
        }
        fprintf( dump_file, "\n" );
    }
}

void Geom::SetupPMARCFile( int &ipatch, vector < int > &idpat )
{
    for ( int i = 0 ; i < GetNumTotalSurfs() ; i++ )
    {
        if( GetSurfType(i) == vsp::WING_SURF )
        {
            idpat[ipatch] = 1;
        }
        else
        {
            idpat[ipatch] = 2;
        }

        ipatch++;
    }
}

void Geom::WritePMARCGeomFile(FILE *fp, int &ipatch, vector<int> &idpat, vector < int > &wstart, vector < int > &wend)
{
    bool pmtippatch = false; // WARNING: Always false

    for ( int i = 0 ; i < GetNumTotalSurfs() ; i++ )
    {
        //==== Tessellate Surface ====//
        vector< vector< vec3d > > pnts;
        vector< vector< vec3d > > norms;
        vector< vector< vec3d > > uw_pnts;

        UpdateTesselate( m_SurfVec[i], m_CapUMinSuccess[ m_SurfIndxVec[i] ], m_CapUMaxSuccess[ m_SurfIndxVec[i] ], false, pnts, norms,
                         uw_pnts);

        wstart[ipatch] = 0;
        wend[ipatch] = 0;

        if ( idpat[ipatch] == 1 ) // WING_SURF
        {
            if( m_CapUMinOption()!=NO_END_CAP && m_CapUMinSuccess[ m_SurfIndxVec[i] ] )
            {
                wstart[ipatch] = m_CapUMinTess();
            }

            if( m_CapUMaxOption()!=NO_END_CAP && m_CapUMaxSuccess[ m_SurfIndxVec[i] ] )
            {
                wend[ipatch] = pnts.size() - m_CapUMinTess();
            }
        }

        int irev = 0;
        if ( !GetFlipNormal(i) )
        {
            irev = -1;
        }

        // Start of wing patch.
        // Patch definition.
        // IDPAT = 1 -- Wing type, 2 -- Body type
        fprintf(fp," &PATCH1  IREV= %d, IDPAT= %d, MAKE= 0, KCOMP= 1, KASS= 1, IPATSYM= 0, IPATCOP= 0, &END\n", irev, idpat[ipatch] );
        fprintf(fp," %s\n", GetName().c_str() );

        for ( int ll = 0; ll < pnts.size(); ll++ )
        {
            // Column header.
            // Section coordinate system information.
            fprintf(fp, " &SECT1  STX= 0.0, STY= 0.0, STZ= 0.0, SCALE= 1.0,\n   ALF= 0.0, THETA= 0.0,\n");

            // Column header continued.
            // Last line of column header changes for last row of points.
            if (ll == pnts.size() - 1)
            {
                if ( ipatch < idpat.size() - 1 )
                {
                    // Last section of this patch. (TNODS)
                    fprintf(fp, "   INMODE= 4, TNODS= 3, TNPS= 0, TINTS= 3, &END\n");
                }
                else
                {
                    // Last section of last patch.
                    fprintf(fp, "   INMODE= 4, TNODS= 5, TNPS= 0, TINTS= 3, &END\n");
                }
            }
            else
            {
                // More sections of this patch to follow.
                fprintf(fp, "   INMODE= 4, TNODS= 0, TNPS= 0, TINTS= 3, &END\n");
            }

            // Print out the actual points.
            // TE bottom surface to LE to TE top surface.
            for ( int mm  = 0; mm < pnts[ll].size(); mm++ )
            {
                for ( int nn = 0; nn < 3; nn++ )
                {
                    fprintf(fp, "%16.8f ", pnts[ll][mm].v[nn]);
                }
                fprintf(fp, "\n");
            }
            // Column footer.
            // Break point ending this airfoil (section).
            fprintf(fp, " &BPNODE TNODE= 3, TNPC= 0, TINTC= 3, &END\n");
        }

        ipatch++;
    }


    // Wing tip patch generation (flat)
    if ( pmtippatch )
    {
        // Patch definition.  Note symmetrical patch is automatically generated.
        fprintf(fp," &PATCH1  IREV= 0, IDPAT= 2, MAKE=1, KCOMP= 1, KASS= 1,");
        fprintf(fp," IPATSYM=1, IPATCOP =0, &END\n");
        fprintf(fp," Wing Tip\n");
        // Automatic tip patch definition.
        fprintf(fp," &PATCH2  ITYP=1,   TNODS=5, TNPS=3, TINTS=3,       &END\n");
    }
}

void Geom::WritePMARCWakeFile( FILE *fp, int &ipatch, vector<int> &idpat, vector < int > &wstart, vector < int > &wend )
{
    int ilastwake = -1;
    for ( int i = 0; i < idpat.size(); i++ )
    {
        if ( idpat[i] == 1 ) // Wing-type
        {
            ilastwake = i;
        }
    }

    for ( int i = 0 ; i < GetNumTotalSurfs() ; i++ )
    {
        if ( idpat[ipatch] == 1 )
        {
            int nodew = 3;
            if ( ipatch == ilastwake )
            {
                nodew = 5;
            }

            // Wake definition
            fprintf(fp," &WAKE1   IDWAK=1,  IFLXW= 0,   ITRFTZ=1,  INTRW=1,  &END\n" );
            fprintf(fp," Wing Wake\n");
            // Wake separation information.  Patch 1, side 2.
            fprintf(fp," &WAKE2   KWPACH=%d, KWSIDE=2, KWLINE=0,  KWPAN1=%d,\n", ipatch + 1, wstart[ipatch] );
            // More wakes to follow. (NODEW)
            fprintf(fp,"          KWPAN2=%d, NODEW=%d,  INITIAL=0,             &END\n", wend[ipatch], nodew);
        }

        ipatch++;
    }
}

void Geom::CreateGeomResults( Results* res )
{
    res->Add( new NameValData( "Type", vsp::GEOM_XSECS, "Geom results type." ) );
    res->Add( new NameValData( "Num_Surfs", GetNumTotalSurfs(), "Number of surfaces." ) );

    for ( int i = 0 ; i < GetNumTotalSurfs() ; i++ )
    {
        //==== Tessellate Surface ====//
        vector< vector< vec3d > > pnts;
        vector< vector< vec3d > > norms;
        vector< vector< vec3d > > uw_pnts;
        UpdateTesselate( m_SurfVec[i], m_CapUMinSuccess[ m_SurfIndxVec[i] ], m_CapUMaxSuccess[ m_SurfIndxVec[i] ], false, pnts, norms,
                         uw_pnts);

        res->Add( new NameValData( "Num_XSecs", static_cast<int>( pnts.size() ), "Number of cross sections." ) );

        if ( pnts.size() )
        {
            res->Add( new NameValData( "Num_Pnts_Per_XSec", static_cast<int>( pnts[0].size() ), "Number of points per cross section." ) );
        }

        //==== Write XSec Data ====//
        for ( int j = 0 ; j < ( int )pnts.size() ; j++ )
        {
            vector< vec3d > xsec_vec( pnts[j].size() );
            for ( int k = 0 ; k < ( int )pnts[j].size() ; k++ )
            {
                xsec_vec[k] = pnts[j][k];
            }
            res->Add( new NameValData( "XSec_Pnts", xsec_vec, "Coordinates of surface points." ) );
        }
    }
}


void Geom::WriteX3D( xmlNodePtr node )
{
    xmlNodePtr set_node = xmlNewChild( node, nullptr, BAD_CAST "IndexedFaceSet", nullptr );
    xmlSetProp( set_node, BAD_CAST "solid", BAD_CAST "true" );
    xmlSetProp( set_node, BAD_CAST "creaseAngle", BAD_CAST "0.5"  );

    string indstr, crdstr;
    int offset = 0;
    char numstr[255];

    for ( int i = 0 ; i < GetNumTotalSurfs() ; i++ )
    {
        vector< vector< vec3d > > pnts;
        vector< vector< vec3d > > norms;
        vector< vector< vec3d > > uw_pnts;
        UpdateTesselate( m_SurfVec[i], m_CapUMinSuccess[ m_SurfIndxVec[i] ], m_CapUMaxSuccess[ m_SurfIndxVec[i] ], false, pnts, norms,
                         uw_pnts);
        unsigned int num_xsecs = pnts.size();
        unsigned int num_pnts = pnts[0].size();
        bool f_norm = GetFlipNormal( i );
        vector< vector<int> > pntIndex;
        // Resize vector
        pntIndex.resize( num_xsecs );
        for ( int j = 0 ; j < num_xsecs ; j++ )
        {
            pntIndex[j].resize( num_pnts );
        }

        for ( int xs = 0 ; xs < num_xsecs ; xs++ )
        {
            for ( int p = 0 ; p < num_pnts ; p++ )
            {
                pntIndex[xs][p] = offset;
                offset++;

                vec3d pnt = pnts[xs][p];
                snprintf( numstr, sizeof( numstr ), "%lf %lf %lf ", pnt.x(), pnt.y(), pnt.z() );
                crdstr += numstr;
            }
        }

        for ( int xs = 0 ; xs < num_xsecs - 1 ; xs++ )
        {
            for ( int p = 0 ; p < num_pnts - 1 ; p++ )
            {
                int i0, i1, i2, i3;
                i0 = pntIndex[xs][p];
                i1 = pntIndex[xs + 1][p];
                i2 = pntIndex[xs + 1][p + 1];
                i3 = pntIndex[xs][p + 1];

                if ( f_norm )
                {
                    snprintf( numstr, sizeof( numstr ), "%d %d %d %d -1 ", i3, i2, i1, i0 );
                }
                else
                {
                    snprintf( numstr, sizeof( numstr ), "%d %d %d %d -1 ", i0, i1, i2, i3 );
                }
                indstr += numstr;
            }
        }
    }

    xmlSetProp( set_node, BAD_CAST "coordIndex", BAD_CAST indstr.c_str() );

    xmlNodePtr coord_node = xmlNewChild( set_node, nullptr, BAD_CAST "Coordinate", BAD_CAST " " );
    xmlSetProp( coord_node, BAD_CAST "point", BAD_CAST crdstr.c_str() );
}

void Geom::WritePovRay( FILE* fid, int comp_num )
{
    string name = GetName();
    StringUtil::change_space_to_underscore( name );
    char str[4];
    snprintf( str, sizeof( str ),  "_%d", comp_num );
    name.append( str );

    fprintf( fid, "#declare %s = mesh { \n", name.c_str() );
    for ( int i = 0 ; i < GetNumTotalSurfs() ; i++ )
    {
        vector< vector< vec3d > > pnts;
        vector< vector< vec3d > > norms;
        vec3d n0, n1, n2, n3, v0, v1, v2, v3;

        vector< vector< vec3d > > uw_pnts;
        UpdateTesselate( m_SurfVec[i], m_CapUMinSuccess[ m_SurfIndxVec[i] ], m_CapUMaxSuccess[ m_SurfIndxVec[i] ], false, pnts,
                         norms, uw_pnts);

        for ( int xs = 0 ; xs < ( int )pnts.size() - 1 ; xs++ )
        {
            for ( int p = 0 ; p < ( int )pnts[xs].size() - 1 ; p++ )
            {
                n0 = norms[xs][p];
                n1 = norms[xs + 1][p];
                n2 = norms[xs + 1][p + 1];
                n3 = norms[xs][p + 1];

                v0 = pnts[xs][p];
                v1 = pnts[xs + 1][p];
                v2 = pnts[xs + 1][p + 1];
                v3 = pnts[xs][p + 1];

                fprintf( fid, "smooth_triangle { \n" );
                WritePovRayTri( fid, v0, n0 );
                WritePovRayTri( fid, v2, n2 );
                WritePovRayTri( fid, v1, n1, false );

                fprintf( fid, "smooth_triangle { \n" );
                WritePovRayTri( fid, v0, n0 );
                WritePovRayTri( fid, v3, n3 );
                WritePovRayTri( fid, v2, n2, false );
            }
        }
    }
    fprintf( fid, " }\n" );
}

void Geom::WritePovRayTri( FILE* fid, const vec3d& v, const vec3d& n, bool comma )
{
    fprintf( fid, "< %12.8f,  %12.8f,  %12.8f >, ",    v.x(), v.z(), v.y() );

    if ( comma )
    {
        fprintf( fid, "< %12.8f,  %12.8f,  %12.8f >,  \n", n.x(), n.z(), n.y() );
    }
    else
    {
        fprintf( fid, "< %12.8f,  %12.8f,  %12.8f >  }\n", n.x(), n.z(), n.y() );
    }
}

//==== Create TMesh Vector ====//
vector< TMesh* > Geom::CreateTMeshVec( bool skipnegflipnormal, const int & n_ref ) const
{
    return CreateTMeshVec( m_SurfVec, skipnegflipnormal, n_ref );
}

vector< TMesh* > Geom::CreateTMeshVec( const vector<VspSurf> &surf_vec, bool skipnegflipnormal, const int & n_ref ) const
{
    vector< TMesh* > TMeshVec;
    vector< vector<vec3d> > pnts;
    vector< vector<vec3d> > norms;
    vector< vector<vec3d> > uw_pnts;

    int nsurf = surf_vec.size();

    for ( int i = 0 ; i < nsurf; i++ )
    {
        surf_vec[i].ResetUSkip();
    }

    // Check for duplicate patches.  Typically occur at symmetrical wing root.  When they occur, skip both copies
    // in tessellation.
    if ( nsurf == GetNumTotalSurfs() ) // Detect if we're working m_SurfVec
    {
        for ( int i = 0 ; i < nsurf - 1 ; i++ )
        {
            for ( int j = i + 1 ; j < nsurf ; j++ )
            {
                if ( m_SurfIndxVec[i] == m_SurfIndxVec[j] ) // Check if they come from same main surface
                {
                    surf_vec[i].FlagDuplicate( surf_vec[j] );
                }
            }
        }
    }
    else                                         // We're working copy of m_MainSurfVec
    {
        for ( int i = 0 ; i < nsurf - 1 ; i++ )
        {
            for ( int j = i + 1 ; j < nsurf ; j++ )
            {
                surf_vec[i].FlagDuplicate( surf_vec[j] );
            }
        }
    }

    int iQuad = 0;
    for ( int i = 0 ; i < nsurf ; i++ )
    {
        if ( surf_vec[i].GetNumSectU() != 0 && surf_vec[i].GetNumSectW() != 0 )
        {
            UpdateTesselate( surf_vec[i], m_CapUMinSuccess[ m_SurfIndxVec[i] ], m_CapUMaxSuccess[ m_SurfIndxVec[i] ], false, pnts, norms, uw_pnts, n_ref );
            surf_vec[i].ResetUSkip(); // Done with skip flags.

            bool thicksurf = true;
            CreateTMeshVecFromPts( this, TMeshVec, pnts, uw_pnts,
                                   n_ref,
                                   i, -1, surf_vec[i].GetSurfType(), surf_vec[i].GetSurfCfdType(),
                                   thicksurf, surf_vec[i].GetFlipNormal(), skipnegflipnormal, iQuad );
        }
    }
    return TMeshVec;
}

void Geom::AddLinkableParms( vector< string > & linkable_parm_vec, const string & link_container_id )
{
    ParmContainer::AddLinkableParms( linkable_parm_vec );

    for ( int i = 0 ; i < ( int )m_SubSurfVec.size() ; i++ )
    {
        m_SubSurfVec[i]->AddLinkableParms( linkable_parm_vec, m_ID );
    }
}

void Geom::ChangeID( const string &id )
{
    Vehicle *veh = VehicleMgr.GetVehicle();

    if ( veh )
    {
        veh->ChangeGeomID( m_ID, id );
    }

    ParmContainer::ChangeID( id );

    for ( int i = 0 ; i < ( int )m_SubSurfVec.size() ; i ++ )
    {
        m_SubSurfVec[i]->SetParentContainer( GetID() );
    }
}


//==== Sub Surface Methods ====//
bool Geom::ValidSubSurfInd( int ind )
{
    if ( ( int )m_SubSurfVec.size() > 0 && ind >= 0 && ind < ( int )m_SubSurfVec.size() )
    {
        return true;
    }
    else
    {
        return false;
    }
}

void Geom::DelSubSurf( int ind )
{
    if ( ValidSubSurfInd( ind ) )
    {
        delete m_SubSurfVec[ind];
        m_SubSurfVec.erase( m_SubSurfVec.begin() + ind );
    }

    SubSurfaceMgr.ReSuffixGroupNames( GetID() );
}

SubSurface* Geom::AddSubSurf( int type, int surfindex )
{
    SubSurface* ssurf = nullptr;

    if ( m_MainSurfVec.size() == 0 )
    {
        return ssurf;
    }

    if ( type == vsp::SS_LINE )
    {
        ssurf = new SSLine( m_ID );
        ssurf->SetName( string( "SS_LINE_" + to_string( ( long long )m_SubSurfVec.size() ) ) );
    }
    else if ( type == vsp::SS_RECTANGLE )
    {
        ssurf = new SSRectangle( m_ID );
        ssurf->SetName( string( "SS_RECT_" + to_string( ( long long )m_SubSurfVec.size() ) ) );
    }
    else if ( type == vsp::SS_ELLIPSE )
    {
        ssurf = new SSEllipse( m_ID );
        ssurf->SetName( string( "SS_ELLIP_" + to_string( ( long long )m_SubSurfVec.size() ) ) );
    }
    else if (type == vsp::SS_CONTROL)
    {
        ssurf = new SSControlSurf(m_ID);
        ssurf->SetName(string("SS_CONT_" + to_string((long long)m_SubSurfVec.size())));
    }
    else if ( type == vsp::SS_FINITE_LINE )
    {
        ssurf = new SSFiniteLine( m_ID );
        ssurf->SetName( string( "SS_FLINE_" + to_string( ( long long )m_SubSurfVec.size() ) ) );
    }
    else if ( type == vsp::SS_XSEC_CURVE )
    {
        ssurf = new SSXSecCurve( m_ID );
        ssurf->SetName( string( "SS_XSC_" + to_string( ( long long )m_SubSurfVec.size() ) ) );
    }

    if ( ssurf )
    {
        ssurf->SetParentContainer( GetID() );
        ssurf->m_MainSurfIndx.Set( surfindex );
        AddSubSurf( ssurf );
    }

    SubSurfaceMgr.ReSuffixGroupNames( GetID() );

    return ssurf;
}

SubSurface* Geom::GetSubSurf( int ind )
{
    if ( ValidSubSurfInd( ind ) )
    {
        return m_SubSurfVec[ind];
    }
    return nullptr;
}

SubSurface* Geom::GetSubSurf( const string & id )
{
    for ( int i = 0 ; i < (int)m_SubSurfVec.size() ; i++ )
    {
        if ( m_SubSurfVec[i]->GetID() == id )
        {
            if ( ValidSubSurfInd( i ) )
                return m_SubSurfVec[i];
        }
    }
    return nullptr;
}

int Geom::GetSubSurfIndex( const string & id )
{
    for ( int i = 0; i < (int)m_SubSurfVec.size(); i++ )
    {
        if ( m_SubSurfVec[i]->GetID() == id )
        {
            if ( ValidSubSurfInd( i ) )
                return i;
        }
    }
    return -1;
}

//==== Highlight Active Subsurface ====//
void Geom::RecolorSubSurfs( int active_ind )
{
    bool active_geom = ( m_Vehicle->IsGeomActive( m_ID ) && m_Vehicle->GetActiveGeomVec().size() == 1 ); // Is this geom the only active geom

    for ( int i = 0; i < ( int )m_SubSurfVec.size() ; i++ )
    {
        if ( active_geom )
        {
            if ( i == active_ind )
            {
                m_SubSurfVec[i]->SetLineColor( vec3d( 1, 0, 0 ) );
            }
            else
            {
                m_SubSurfVec[i]->SetLineColor( vec3d( 0, 0, 0 ) );
            }
        }
        else
        {
            m_SubSurfVec[i]->SetLineColor( vec3d( 0, 0, 0 ) );
        }
    }
}

//==== Add FeaStructure =====//
FeaStructure* Geom::AddFeaStruct( bool initskin, int surf_index )
{
    FeaStructure* feastruct = nullptr;

    if ( m_MainSurfVec.size() == 0 || surf_index > m_MainSurfVec.size() || surf_index < 0 )
    {
        return feastruct;
    }

    if ( isSurfaceType() )
    {
        feastruct = new FeaStructure( GetID(), surf_index );

        if ( feastruct )
        {
            string defaultname = m_Name + "_Struct" + to_string( m_FeaStructCount );
            feastruct->SetName( defaultname );

            if ( feastruct->GetStructSettingsPtr() )
            {
                feastruct->ResetExportFileNames();
            }

            if ( initskin )
            {
                feastruct->InitFeaSkin();
            }

            m_FeaStructVec.push_back( feastruct );
            m_FeaStructCount++;
        }
    }

    return feastruct;
}

FeaStructure* Geom::GetFeaStruct( int fea_struct_ind )
{
    if ( ValidGeomFeaStructInd( fea_struct_ind ) )
    {
        return m_FeaStructVec[fea_struct_ind];
    }
    return nullptr;
}

int Geom::GetFeaStructIndex( const string & structure_id )
{
    for ( size_t i = 0; i < m_FeaStructVec.size(); i++ )
    {
        if ( strcmp( m_FeaStructVec[i]->GetID().c_str(), structure_id.c_str() ) == 0 )
        {
            return i;
        }
    }
    return -1; // indicates an error
}

//==== Delete FeaStructure =====//
void Geom::DeleteFeaStruct( int index )
{
    if ( !ValidGeomFeaStructInd( index ) )
        return;

    delete m_FeaStructVec[index];
    m_FeaStructVec.erase( m_FeaStructVec.begin() + index );
}

bool Geom::ValidGeomFeaStructInd( int index )
{
    if ( (int)m_FeaStructVec.size() > 0 && index >= 0 && index < (int)m_FeaStructVec.size() )
    {
        return true;
    }
    return false;
}

void Geom::DelAllSources()
{
    for ( int i = 0 ; i < ( int )m_MainSourceVec.size() ; i++ )
    {
        delete m_MainSourceVec[i];
    }
    m_MainSourceVec.clear();
}

void Geom::DelCurrSource()
{
    int id = GetCurrSourceID();

    vector< BaseSource* > tmpVec;
    for ( int i = 0 ; i < ( int )m_MainSourceVec.size() ; i++ )
    {
        if ( i != id )
        {
            tmpVec.push_back( m_MainSourceVec[i] );
        }
        else
        {
            delete m_MainSourceVec[i];
        }
    }
    m_MainSourceVec = tmpVec;
}

void Geom::UpdateSources()
{
    for ( int i = 0; i < m_SimpSourceVec.size(); i++ )
    {
        delete m_SimpSourceVec[i];
    }
    m_SimpSourceVec.clear();

    int nmain = m_MainSourceVec.size();
    int ncopy = GetNumSymmCopies();

    for ( int i = 0 ; i < nmain ; i++ )
    {
        assert( ncopy == m_SurfSymmMap[ m_MainSourceVec[i]->m_MainSurfIndx.Get() ].size() );

        for ( int j = 0; j < ncopy; j++ )
        {
            m_SimpSourceVec.push_back( CreateSimpleSource( m_MainSourceVec[i]->GetType() ) );
            int k = m_SimpSourceVec.size() - 1;
            m_SimpSourceVec[k]->CopyFrom( m_MainSourceVec[i] );
            m_SimpSourceVec[k]->m_SurfIndx = m_SurfSymmMap[ m_MainSourceVec[i]->m_MainSurfIndx.Get() ][j];
            m_SimpSourceVec[k]->Update( this );
        }
    }
}

BaseSource* Geom::CreateSource( int type )
{
    BaseSource* src_ptr = nullptr;
    if ( type == POINT_SOURCE )
    {
        src_ptr = new PointSource();
    }
    else if ( type == LINE_SOURCE )
    {
        src_ptr = new LineSource();
    }
    else if ( type == BOX_SOURCE )
    {
        src_ptr = new BoxSource();
    }
    else if ( type == ULINE_SOURCE )
    {
        src_ptr = new ULineSource();
    }
    else if ( type == WLINE_SOURCE )
    {
        src_ptr = new WLineSource();
    }

    return src_ptr;
}

BaseSimpleSource* Geom::CreateSimpleSource( int type )
{
    BaseSimpleSource* src_ptr = nullptr;
    if ( type ==POINT_SOURCE )
    {
        src_ptr = new PointSimpleSource();
    }
    else if ( type == LINE_SOURCE )
    {
        src_ptr = new LineSimpleSource();
    }
    else if ( type == BOX_SOURCE )
    {
        src_ptr = new BoxSimpleSource();
    }
    else if ( type == ULINE_SOURCE )
    {
        src_ptr = new ULineSimpleSource();
    }
    else if ( type == WLINE_SOURCE )
    {
        src_ptr = new WLineSimpleSource();
    }

    return src_ptr;
}


bool Geom::HasWingTypeSurfs()
{
    for( int i = 0; i < m_MainSurfVec.size(); i++ )
    {
        if( GetMainSurfType(i) == vsp::WING_SURF )
        {
            return true;
        }
    }
    return false;
}

void Geom::AppendWakeData( vector < piecewise_curve_type >& curve_vec, vector < double > & wake_scale_vec, vector < double > & wake_angle_vec )
{
    if( m_WakeActiveFlag() )
    {
        for( int i = 0; i < GetNumTotalSurfs(); i++ )
        {
            if( GetSurfType(i) == vsp::WING_SURF )
            {
                piecewise_curve_type curve;
                m_SurfVec[i].GetWakeTECurve( curve );

                curve_vec.push_back( curve );
                wake_scale_vec.push_back( m_WakeScale() );
                wake_angle_vec.push_back( m_WakeAngle() );
            }
        }
    }
}

void Geom::ExportSurfacePatches( vector<string> &surf_res_ids )
{
    // Loop over all surfaces and tesselate
    for ( int i = 0 ; i < GetNumTotalSurfs() ; i++ )
    {
        vector< vector< vector< vec3d > > > pnts, norms;
        UpdateSplitTesselate( m_SurfVec[i], m_CapUMinSuccess[ m_SurfIndxVec[i] ], m_CapUMaxSuccess[ m_SurfIndxVec[i] ],
                              pnts, norms );

        // Add a results entity for each patch to the surface
        Results* res = ResultsMgr.CreateResults( "Surface", "Surfaces within a Geom." );
        res->Add( new NameValData( "comp_id", GetID(), "GeomID." ) );
        res->Add( new NameValData( "surf_index", i, "Surface index." ) );

        vector< string > patch_ids;
        for ( int ipatch = 0 ; ipatch < ( int )pnts.size() ; ipatch++ )
        {
            Results* patch_res = ResultsMgr.CreateResults( "SurfacePatch", "Patches (determined by feature lines) within a surface." );
            patch_res->Add( new NameValData( "comp_id", GetID(), "GeomID." ) );
            patch_res->Add( new NameValData( "surf_index", i, "Surface index." ) );
            patch_res->Add( new NameValData( "patch_index", ipatch, "Patch index.") );
            patch_res->Add( pnts[ipatch], "", "Patch node coordinates" );
            patch_res->Add( norms[ipatch], "n", "Patch node normal vector" );
            patch_ids.push_back( patch_res->GetID() );
        }

        res->Add( new NameValData( "patches", patch_ids, "ID's of patch results." ) );

        surf_res_ids.push_back( res->GetID() );
    }
}

//===============================================================================//
//===============================================================================//
//===============================================================================//
//==== Constructor ====//
GeomXSec::GeomXSec( Vehicle* vehicle_ptr ) : Geom( vehicle_ptr )
{
    m_Name = "GeomXSec";
    m_Type.m_Type = GEOM_GEOM_TYPE;
    m_Type.m_Name = m_Name;

    m_ActiveXSec.Init( "ActiveXSec", "Index", this, 0, 0, 1e6 );
}
//==== Destructor ====//
GeomXSec::~GeomXSec()
{

}

void GeomXSec::Update( bool fullupdate )
{
    m_ActiveXSec.SetUpperLimit( m_XSecSurf.NumXSec() - 1 );

    Geom::Update( fullupdate );
}

void GeomXSec::UpdateDrawObj()
{
    Geom::UpdateDrawObj();

    Matrix4d relTrans;

    relTrans = m_AttachMatrix;
    relTrans.affineInverse();
    relTrans.matMult( m_ModelMatrix.data() );
    relTrans.postMult( m_AttachMatrix.data() );

    unsigned int nxsec = m_XSecSurf.NumXSec();
    m_XSecDrawObj_vec.resize( nxsec, DrawObj() );

    for ( int i = 0 ; i < nxsec ; i++ )
    {
        XSec* xs = m_XSecSurf.FindXSec( i );
        if ( xs )
        {
            m_XSecDrawObj_vec[i].m_PntVec = xs->GetDrawLines( relTrans );
        }
        else
        {
            m_XSecDrawObj_vec[i].m_PntVec = vector < vec3d > (0);
        }
        m_XSecDrawObj_vec[i].m_GeomChanged = true;
    }
}

void GeomXSec::UpdateHighlightDrawObj()
{
    Matrix4d relTrans;
    relTrans = m_AttachMatrix;
    relTrans.affineInverse();
    relTrans.matMult( m_ModelMatrix.data() );
    relTrans.postMult( m_AttachMatrix.data() );

    XSec* axs = m_XSecSurf.FindXSec( m_ActiveXSec() );
    if ( axs )
    {
        m_HighlightXSecDrawObj.m_PntVec = axs->GetDrawLines( relTrans );
    }
    else
    {
        m_HighlightXSecDrawObj.m_PntVec = vector < vec3d > (0);
    }

    m_HighlightXSecDrawObj.m_GeomChanged = true;
}

void GeomXSec::LoadDrawObjs( vector< DrawObj* > & draw_obj_vec )
{
    Geom::LoadDrawObjs( draw_obj_vec );

    bool isactive = m_Vehicle->IsGeomActive( m_ID );
    bool isshown = GetSetFlag( vsp::SET_SHOWN );
    char str[256];

    for ( int i = 0 ; i < m_XSecDrawObj_vec.size() ; i++ )
    {
        snprintf( str, sizeof( str ),  "_%d", i );

        m_XSecDrawObj_vec[i].m_Screen = DrawObj::VSP_MAIN_SCREEN;
        m_XSecDrawObj_vec[i].m_GeomID = XSECHEADER + m_ID + str;
        m_XSecDrawObj_vec[i].m_Visible = isshown && isactive && m_GuiDraw.GetDisplayType() == DISPLAY_TYPE::DISPLAY_BEZIER;
        m_XSecDrawObj_vec[i].m_LineWidth = 2.0;
        m_XSecDrawObj_vec[i].m_LineColor = vec3d( 0.0, 0.0, 0.0 );
        m_XSecDrawObj_vec[i].m_Type = DrawObj::VSP_LINE_STRIP;
        draw_obj_vec.push_back( &m_XSecDrawObj_vec[i] );
    }

    m_HighlightXSecDrawObj.m_Screen = DrawObj::VSP_MAIN_SCREEN;
    m_HighlightXSecDrawObj.m_GeomID = XSECHEADER + m_ID + "ACTIVE";
    m_HighlightXSecDrawObj.m_Visible = isshown && isactive && m_GuiDraw.GetDisplayType() == DISPLAY_TYPE::DISPLAY_BEZIER;
    m_HighlightXSecDrawObj.m_LineWidth = 4.0;
    m_HighlightXSecDrawObj.m_LineColor = vec3d( 0.0, 0.0, 1.0 );
    m_HighlightXSecDrawObj.m_Type = DrawObj::VSP_LINE_STRIP;
    draw_obj_vec.push_back( &m_HighlightXSecDrawObj );
}

void GeomXSec::UpdateDrawObjUtil()
{
    Geom::UpdateDrawObj();

    Matrix4d relTrans;
    relTrans = m_AttachMatrix;
    relTrans.affineInverse();
    relTrans.matMult( m_ModelMatrix.data() );
    relTrans.postMult( m_AttachMatrix.data() );

    unsigned int nxsec = m_XSecSurf.NumXSec();
    m_XSecDrawObj_vec.resize( nxsec, DrawObj() );

    //==== Tesselate Surface ====//
    for ( int i = 0 ; i < nxsec ; i++ )
    {
        m_XSecDrawObj_vec[i].m_PntVec = m_XSecSurf.FindXSec( i )->GetDrawLines( relTrans );
        m_XSecDrawObj_vec[i].m_GeomChanged = true;
    }
}

void GeomXSec::UpdateHighlightDrawObjUtil( int bbox_index )
{
    Matrix4d attachMat;
    Matrix4d relTrans;
    relTrans = m_AttachMatrix;
    relTrans.affineInverse();
    relTrans.matMult( m_ModelMatrix.data() );
    relTrans.postMult( m_AttachMatrix.data() );

    m_HighlightXSecDrawObj.m_PntVec = m_XSecSurf.FindXSec( m_ActiveXSec() )->GetDrawLines( relTrans );
    m_HighlightXSecDrawObj.m_GeomChanged = true;

    // make bounding box over current XSec if not first xsec in stack/fuse
    if ( bbox_index > 0 )
    {
        VspCurve inbd = m_XSecSurf.FindXSec( bbox_index - 1 )->GetCurve(); // FIXME: Crash when loading a model
        inbd.Transform( relTrans );

        VspCurve outbd = m_XSecSurf.FindXSec( bbox_index )->GetCurve();
        outbd.Transform( relTrans );

        BndBox iBBox, oBBox;
        inbd.GetBoundingBox( iBBox );
        outbd.GetBoundingBox( oBBox );
        oBBox.Update( iBBox );

        m_HighlightXSecLoftDrawObj.m_PntVec = oBBox.GetBBoxDrawLines();
        m_HighlightXSecLoftDrawObj.m_GeomChanged = true;
    }
    else
    {
        m_HighlightXSecLoftDrawObj.m_PntVec.clear();
        m_HighlightXSecLoftDrawObj.m_GeomChanged = true;
    }
}

void GeomXSec::LoadDrawObjsUtil( vector< DrawObj* > & draw_obj_vec )
{
    GeomXSec::LoadDrawObjs( draw_obj_vec );
    m_HighlightXSecLoftDrawObj.m_Screen = DrawObj::VSP_MAIN_SCREEN;
    m_HighlightXSecLoftDrawObj.m_GeomID = BBOXHEADER + m_ID + "ACTIVE_SECT";
    m_HighlightXSecLoftDrawObj.m_Visible = m_Vehicle->IsGeomActive( m_ID );
    m_HighlightXSecLoftDrawObj.m_LineWidth = 4.0;
    m_HighlightXSecLoftDrawObj.m_LineColor = vec3d( 0.0, 1.0, 0.0 );
    m_HighlightXSecLoftDrawObj.m_Type = DrawObj::VSP_LINES;
    draw_obj_vec.push_back( &m_HighlightXSecLoftDrawObj );
}

//==== Get XSec ====//
XSec* GeomXSec::GetXSec( int index )
{
    return m_XSecSurf.FindXSec( index );
}

void GeomXSec::AddDefaultSourcesXSec( double base_len, double len_ref, int ixsec )
{
    PointSource* psource;
    LineSource* lsource;
    char str[256];

    FuseXSec* xs = ( FuseXSec* ) m_XSecSurf.FindXSec( ixsec );
    if ( xs )
    {
        XSecCurve* xsc = xs->GetXSecCurve();
        if ( xsc )
        {
            if ( xsc->GetType() == XS_POINT )
            {
                psource = new PointSource();
                snprintf( str, sizeof( str ),  "Def_PS_%d", ixsec );
                psource->SetName( str );
                psource->m_Len = 0.1 * base_len;
                psource->m_Rad = 0.2 * len_ref;
                psource->m_ULoc = ixsec / ( m_XSecSurf.NumXSec() - 1 );
                psource->m_WLoc = 0.0;
                psource->m_MainSurfIndx = 0;
                AddCfdMeshSource( psource );
            }
            else
            {
                double w = xsc->GetWidth();
                double h = xsc->GetHeight();

                double r = sqrt( w * w + h * h );

                int nls = 8;

                double u = ( double ) ixsec / ( ( double ) m_XSecSurf.NumXSec() - 1 );

                for ( int i = 0; i < nls; i++ )
                {
                    double w0 = ( double ) i / ( ( double ) nls );
                    double w1 = ( ( double ) (i+1) ) / ( ( double ) nls );

                    lsource = new LineSource();
                    snprintf( str, sizeof( str ),  "Def_LS_%d_%d", ixsec, i );
                    lsource->SetName( str );
                    lsource->m_Len = 0.1 * base_len;
                    lsource->m_Len2 = 0.1 * base_len;
                    lsource->m_Rad = 0.2 * r;
                    lsource->m_Rad2 = 0.2 * r;
                    lsource->m_ULoc1 = u;
                    lsource->m_WLoc1 = w0;
                    lsource->m_ULoc2 = u;
                    lsource->m_WLoc2 = w1;
                    lsource->m_MainSurfIndx = 0;
                    AddCfdMeshSource( lsource );
                }
            }
        }
    }
}

//==== Set Active XSec Type ====//
void GeomXSec::SetActiveXSecType(int type)
{
    XSec* xs = GetXSec(m_ActiveXSec());

    if (!xs)
    {
        return;
    }

    if (type == xs->GetXSecCurve()->GetType())
    {
        return;
    }

    m_XSecSurf.ChangeXSecShape(m_ActiveXSec(), type);

    Update();
}

void GeomXSec::OffsetXSecs( double off )
{
    int nxsec = m_XSecSurf.NumXSec();
    for ( int i = 0 ; i < nxsec ; i++ )
    {
        XSec* xs = m_XSecSurf.FindXSec( i );
        if ( xs )
        {
            XSecCurve* xsc = xs->GetXSecCurve();
            if ( xsc )
            {
                xsc->OffsetCurve( off );
            }
        }
    }
}

void GeomXSec::NormalizeFlaps()
{
    int nflap = 0;
    double flapdeflection = 0;
    double flapxchord = 0;
    double arclen = 0;
    int nxsec = m_XSecSurf.NumXSec();

    for ( int i = 0 ; i < nxsec ; i++ )
    {
        WingSect* ws = ( WingSect* ) m_XSecSurf.FindXSec( i );
        if ( ws )
        {
            XSecCurve* xsc = ws->GetXSecCurve();
            if ( xsc )
            {
                string width_id = xsc->GetWidthParmID();
                Parm* width_parm = ParmMgr.FindParm( width_id );

                double w = ws->m_TipChord();

                if ( width_parm && xsc->m_TEFlapType() != vsp::FLAP_NONE )
                {
                    nflap++;
                    flapdeflection += std::abs( xsc->m_TEFlapDeflection() );
                    arclen += xsc->EstimateFlapArcLen();

                    if ( xsc->m_TEFlapAbsRel() == ABS )
                    {
                        flapxchord += xsc->m_TEFlapX() / w;
                    }
                    else
                    {
                        flapxchord += xsc->m_TEFlapXChord();
                    }
                }
            }
        }
    }

    if ( nflap > 0 )
    {
        flapdeflection = flapdeflection / nflap;
        flapxchord = flapxchord / nflap;
        arclen = arclen / nflap;
        double fac = 100;
        flapxchord = round( flapxchord * fac ) / fac;
        double fac2 = 1000;
        arclen = round( arclen * fac2 ) / fac2;
    }



    for ( int i = 0 ; i < nxsec ; i++ )
    {
        WingSect* ws = ( WingSect* ) m_XSecSurf.FindXSec( i );
        if ( ws )
        {
            XSecCurve* xsc = ws->GetXSecCurve();
            if ( xsc )
            {
                if ( nflap > 0 )
                {
                    xsc->m_TEFlapFlag = true;
                    xsc->m_TEFlapT = flapxchord;
                    xsc->m_TEFlapDT = arclen;
                }
                else
                {
                    xsc->m_TEFlapFlag = false;
                    xsc->m_TEFlapT = 0.2;
                    xsc->m_TEFlapDT = 0.0;
                }
            }
        }
    }
}
