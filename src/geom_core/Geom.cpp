//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//
//
//////////////////////////////////////////////////////////////////////

#include "Geom.h"
#include "Vehicle.h"
#include "StlHelper.h"
#include "DXFUtil.h"
#include "SVGUtil.h"
#include "StringUtil.h"
#include "ParmMgr.h"
#include "SubSurfaceMgr.h"
#include "HingeGeom.h"
using namespace vsp;

//==== Constructor ====//
GeomType::GeomType()
{
    m_Type = POD_GEOM_TYPE;
    m_Name = "Pod";
    m_FixedFlag = false;

}

//==== Constructor ====//
GeomType::GeomType( int id, string name, bool fixed_flag, string module_name, string display_name )
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
    m_DisplayType = DISPLAY_BEZIER;
    m_DrawType = GEOM_DRAW_WIRE;
    m_NoShowFlag = false;
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

void GeomGuiDraw::SetMaterial( std::string name, double ambi[], double diff[], double spec[], double emis[], double shin )
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
    m_Vehicle = vehicle_ptr;

    m_Name = "GeomBase";
    m_Type.m_Type = BASE_GEOM_TYPE;
    m_Type.m_Name = m_Name;
    m_ParentID = string( "NONE" );
}

//==== Destructor ====//
GeomBase::~GeomBase()
{

}

//==== Parm Changed ====//
void GeomBase::ParmChanged( Parm* parm_ptr, int type )
{
    if ( parm_ptr )
    {
        m_UpdatedParmVec.push_back( parm_ptr->GetID() );
    }

    if ( type == Parm::SET )
    {
        m_LateUpdateFlag = true;
        return;
    }

    //==== Check For Interactive Collision Dectection When Alt Key Is Pressed ====//
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
    m_Vehicle->UpdateGui();

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

//==== Recusively Load ID and Childrens ID Into Vec  =====//
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

//==== Remove ID From Child Vec ====//
void GeomBase::RemoveChildID( string id )
{
    vector_remove_val( m_ChildIDVec, id );
}

////==== Copy Geometry ====//
//void GeomBase::CopyFrom( GeomBase* geom )
//{
//  //xmlDocPtr doc = xmlNewDoc((const xmlChar *)"1.0");
//
//  xmlNodePtr root = xmlNewNode(NULL,(const xmlChar *)"Vsp_Geometry");
//  //xmlDocSetRootElement(doc, root);
//
//
////    xmlNodePtr node = xmlNewNode(NULL, BAD_CAST "Data" );
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
    xmlNodePtr geombase_node = xmlNewChild( node, NULL, BAD_CAST "GeomBase", NULL );
    if ( geombase_node )
    {
        XmlUtil::AddStringNode( geombase_node, "TypeName", m_Type.m_Name );
        XmlUtil::AddIntNode( geombase_node, "TypeID", m_Type.m_Type );
        XmlUtil::AddIntNode( geombase_node, "TypeFixed", m_Type.m_FixedFlag );
        XmlUtil::AddStringNode( geombase_node, "ParentID", m_ParentID );

        xmlNodePtr clist_node = xmlNewChild( geombase_node, NULL, BAD_CAST "Child_List", NULL );
        for ( int i = 0 ; i < ( int )m_ChildIDVec.size() ; i++ )
        {
            xmlNodePtr child_node = xmlNewChild( clist_node, NULL, BAD_CAST "Child", NULL );
            XmlUtil::AddStringNode( child_node, "ID", m_ChildIDVec[i] );
        }
    }
    return geombase_node;
}

//==== Decode Data From XML Data Struct ====//
xmlNodePtr GeomBase::DecodeXml( xmlNodePtr & node )
{
    ParmContainer::DecodeXml( node );
    xmlNodePtr child_node = XmlUtil::GetNode( node, "GeomBase", 0 );
    if ( child_node )
    {
        //m_Type.m_Name   = XmlUtil::FindString( child_node, "TypeName", m_Type.m_Name );
        //m_Type.m_Type   = XmlUtil::FindInt( child_node, "TypeID", m_Type.m_Type );
        m_Type.m_FixedFlag = !!XmlUtil::FindInt( child_node, "TypeFixed", m_Type.m_FixedFlag );
        m_ParentID = ParmMgr.RemapID( XmlUtil::FindString( child_node, "ParentID", m_ParentID ) );

        m_ChildIDVec.clear();

        xmlNodePtr cl_node = XmlUtil::GetNode( child_node, "Child_List", 0 );
        int num_children = XmlUtil::GetNumNames( cl_node, "Child" );

        for ( int i = 0 ; i < num_children ; i++ )
        {
            xmlNodePtr n = XmlUtil::GetNode( cl_node, "Child", i );
            m_ChildIDVec.push_back( ParmMgr.RemapID( XmlUtil::FindString( n, "ID", string() ) ) );
        }
    }
    return child_node;
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
    m_AbsRelFlag.Init( "Abs_Or_Relitive_flag", "XForm", this, RELATIVE_XFORM, ABSOLUTE_XFORM, RELATIVE_XFORM );
    m_TransAttachFlag.Init( "Trans_Attach_Flag", "Attach", this, ATTACH_TRANS_NONE, ATTACH_TRANS_NONE, ATTACH_TRANS_UV );
    m_TransAttachFlag.SetDescript( "Determines relative translation coordinate system" );
    m_RotAttachFlag.Init( "Rots_Attach_Flag", "Attach", this, ATTACH_ROT_NONE, ATTACH_ROT_NONE, ATTACH_ROT_UV );
    m_RotAttachFlag.SetDescript( "Determines relative rotation axes" );
    m_ULoc.Init( "U_Attach_Location", "Attach", this, 1e-6, 1e-6, 1 - 1e-6 );
    m_ULoc.SetDescript( "U Location of Parent's Surface" );
    m_WLoc.Init( "V_Attach_Location", "Attach", this, 1e-6, 1e-6, 1 - 1e-6 );
    m_WLoc.SetDescript( "V Location of Parent's Surface" );

    m_Scale.Init( "Scale", "XForm", this, 1, 1.0e-3, 1.0e3 );
    m_Scale.SetDescript( "Scale Geometry Size" );

    m_LastScale.Init( "Last_Scale", "XForm", this, 1, 1.0e-3, 1.0e3 );
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
void GeomXForm::Update( bool fullupdate )
{
    DeactivateXForms();
    ComposeModelMatrix();

    double axlen = 1.0;

    Vehicle *veh = VehicleMgr.GetVehicle();
    if ( veh )
    {
        axlen = veh->m_AxisLength();
    }

    Matrix4d attachedMat = ComposeAttachMatrix();

    m_AttachOrigin = attachedMat.xform( vec3d( 0.0, 0.0, 0.0 ) );

    m_AttachAxis.clear();
    m_AttachAxis.resize( 3 );
    for ( int i = 0; i < 3; i++ )
    {
        vec3d pt = vec3d( 0.0, 0.0, 0.0 );
        pt.v[i] = axlen;
        m_AttachAxis[i] = attachedMat.xform( pt );
    }
}

//==== Compose Model Matrix =====//
void GeomXForm::ComposeModelMatrix()
{
    m_ModelMatrix.loadIdentity();
    ComputeCenter();

    // Get Attament Matrix
    Matrix4d attachedMat = ComposeAttachMatrix();

    if (  m_AbsRelFlag() ==  RELATIVE_XFORM || ( m_ignoreAbsFlag && m_applyIgnoreAbsFlag ) )
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
        m_ModelMatrix.postMult( attachedMat.data() );

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
    else if ( m_AbsRelFlag() ==  ABSOLUTE_XFORM )
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

Matrix4d GeomXForm::ComposeAttachMatrix()
{
    Matrix4d attachedMat;
    attachedMat.loadIdentity();

    Geom* parent = m_Vehicle->FindGeom( GetParentID() );

    if ( parent )
    {
        HingeGeom* hingeparent = dynamic_cast < HingeGeom* > ( parent );
        if ( hingeparent )
        {
            attachedMat = hingeparent->GetJointMatrix();
            return attachedMat;
        }
    }

    // If both attachment flags set to none, return identity
    if ( m_TransAttachFlag() == ATTACH_TRANS_NONE && m_RotAttachFlag() == ATTACH_ROT_NONE )
    {
        return attachedMat;
    }

    if ( parent )
    {
        Matrix4d transMat;
        Matrix4d rotMat;

        Matrix4d parentMat;
        parentMat = parent->getModelMatrix();
        double tempMat[16];
        parentMat.getMat( tempMat );

        bool revertCompTrans = false;
        bool revertCompRot = false;

        // Parent CompXXXCoordSys methods query the positioned m_SurfVec[0] surface,
        // not m_MainSurfVec[0].  Consequently, m_ModelMatrix is already implied in
        // these calculations and does not need to be applied again.
        if ( m_TransAttachFlag() == ATTACH_TRANS_UV )
        {
            if ( !( parent->CompTransCoordSys( m_ULoc(), m_WLoc(), transMat ) ) )
            {
                revertCompTrans = true; // Blank components revert to the component matrix.
            }
        }

        if ( m_RotAttachFlag() == ATTACH_ROT_UV )
        {
            if ( !( parent->CompRotCoordSys( m_ULoc(), m_WLoc(), rotMat ) ) )
            {
                revertCompRot = true; // For blank component.
            }
        }

        if ( m_TransAttachFlag() == ATTACH_TRANS_COMP || revertCompTrans )
        {
            transMat.translatef( tempMat[12], tempMat[13], tempMat[14] );
        }

        if ( m_RotAttachFlag() == ATTACH_ROT_COMP || revertCompRot )
        {
            // Only take rotation matrix from parent so set translation part to zero
            tempMat[12] = tempMat[13] = tempMat[14] = 0;
            rotMat.initMat( tempMat );
        }

        transMat.matMult( rotMat.data() );
        attachedMat = transMat;
    }

    return attachedMat;
}

//==== Set Rel or Abs Flag ====//
void GeomXForm::DeactivateXForms()
{
    // Deactivate non driving parms and Activate driving parms
    if ( m_AbsRelFlag() ==  RELATIVE_XFORM  )
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

    if ( IsParentJoint() )
    {
        m_ULoc.Deactivate();
        m_WLoc.Deactivate();
        m_TransAttachFlag.Deactivate();
        m_RotAttachFlag.Deactivate();
    }
    else
    {
        m_ULoc.Activate();
        m_WLoc.Activate();
        m_TransAttachFlag.Activate();
        m_RotAttachFlag.Activate();
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
        return ComposeAttachMatrix();
    }

    string id = GetAncestorID( gen );

    GeomXForm* ancestPtr = m_Vehicle->FindGeom( id );
    if ( ancestPtr )
    {
        return ancestPtr->ComposeAttachMatrix();
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
//  //==== Uppdate Children =====//
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
    m_TessW.Init( "Tess_W", "Shape", this, 9, 2,  1000 );
    m_TessW.SetDescript( "Number of tessellated curves in the W direction" );
    m_TessW.SetMultShift( 4, 1 );

    m_WakeActiveFlag.Init( "Wake", "Shape", this, false, 0, 1 );

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

    vector< string > set_name_vec = m_Vehicle->GetSetNameVec();
    m_SetFlags.resize( set_name_vec.size() );
    for ( int i = 0 ; i < ( int )m_SetFlags.size() ; i++ )
    {
        m_SetFlags[i] = false;
    }
    UpdateSets();

    m_SymAncestor.Init( "Sym_Ancestor", "Sym", this, 1, 0, 1e6 );
    m_SymAncestOriginFlag.Init( "Sym_Ancestor_Origin_Flag", "Sym", this, true, 0, 1 );
    m_SymPlanFlag.Init( "Sym_Planar_Flag", "Sym", this, 0, 0, SYM_XY | SYM_XZ | SYM_YZ );
    m_SymAxFlag.Init( "Sym_Axial_Flag", "Sym", this, 0, 0, SYM_ROT_Z );
    m_SymRotN.Init( "Sym_Rot_N", "Sym", this, 2, 2, 1000 );

    // Mass Properties
    m_Density.Init( "Density", "Mass_Props", this, 1, 0.0, 1e12 );
    m_MassArea.Init( "Mass_Area", "Mass_Props", this, 1, 0.0, 1e12 );
    m_MassPrior.Init( "Mass_Prior", "Mass_Props", this, 0, 0, 1e12 );
    m_ShellFlag.Init( "Shell_Flag", "Mass_Props", this, false, 0, 1 );

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

    m_CapUMinTess.Init("CapUMinTess", "EndCap", this, 3, 3, 51);
    m_CapUMinTess.SetDescript("Number of tessellated curves on capped ends");
    m_CapUMinTess.SetMultShift(2, 1);

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

    m_CapWMinOption.Init("CapWMinOption", "EndCap", this, NO_END_CAP, NO_END_CAP, NUM_END_CAP_OPTIONS-1);
    m_CapWMinOption.SetDescript("Type of End Cap on WMin end");

    m_CapWMaxOption.Init("CapWMaxOption", "EndCap", this, NO_END_CAP, NO_END_CAP, NUM_END_CAP_OPTIONS-1);
    m_CapWMaxOption.SetDescript("Type of End Cap on WMax end");

    // Geom needs at least one surf
    m_MainSurfVec.push_back( VspSurf() );

    currSourceID = 0;

    m_GeomProjectVec3d.resize( 3 );
    m_ForceXSecFlag = false;

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
    m_ExpandedListFlag.SetDescript("Flag to determine whether or not this geom has a collapsed list in parasite drag")

    m_FeaStructCount = 0;
;
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
    if ( index == SET_ALL )
    {
        return;
    }
    else if ( index == SET_SHOWN )
    {
        m_GuiDraw.SetNoShowFlag( !f );
    }
    else if ( index == SET_NOT_SHOWN )
    {
        m_GuiDraw.SetNoShowFlag( f );
    }

    if ( index > SET_ALL && index < ( int )m_SetFlags.size() )
    {
        m_SetFlags[index] = f;
    }
}

//==== Get Set Flag ====//
bool Geom::GetSetFlag( int index )
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

    while ( m_SetFlags.size() < set_name_vec.size() )
    {
        m_SetFlags.push_back( false );
    }

    if ( m_SetFlags.size() < SET_FIRST_USER )
    {
        return;
    }

    m_SetFlags[ SET_ALL ] = true;   // All

    if ( ! m_GuiDraw.GetNoShowFlag() )
    {
        m_SetFlags[ SET_SHOWN ] = true; // Shown
        m_SetFlags[ SET_NOT_SHOWN ] = false;    // Not_Shown
    }
    else
    {
        m_SetFlags[ SET_SHOWN ] = false;    // Shown
        m_SetFlags[ SET_NOT_SHOWN ] = true; // Not_Shown
    }
}

//==== Copy Geometry ====//
void Geom::CopyFrom( Geom* geom )
{
    xmlNodePtr root = xmlNewNode( NULL, ( const xmlChar * )"Vsp_Geometry" );

    geom->EncodeGeom( root );
    DecodeGeom( root );

    xmlFreeNode( root );
}

//==== Update ====//
void Geom::Update( bool fullupdate )
{
    if ( m_UpdateBlock )
        return;

    m_UpdateBlock = true;

    m_LateUpdateFlag = false;

    m_CappingDone = false;

    Scale();

    UpdateSets();

    UpdateSurf();       // Must be implemented by subclass.

    GeomXForm::Update();

    UpdateEndCaps();

    if ( fullupdate )
    {
        UpdateFeatureLines();
    }

    UpdateFlags();

    UpdateSymmAttach();

    if ( fullupdate )
    {
        for ( int i = 0 ; i < ( int )m_SubSurfVec.size() ; i++ )
        {
            m_SubSurfVec[i]->Update();
        }
    }

    UpdateChildren( fullupdate );
    UpdateBBox();

    if ( fullupdate )
    {
        UpdateDrawObj();
    }

    m_UpdatedParmVec.clear();
    m_UpdateBlock = false;
}

void Geom::GetUWTess01( int indx, vector < double > &u, vector < double > &w )
{
    vector< vector< vec3d > > pnts;
    vector< vector< vec3d > > norms;
    vector< vector< vec3d > > uw_pnts;

    UpdateTesselate( indx, pnts, norms, uw_pnts, false );

    double umx = GetSurfPtr( indx )->GetUMax();
    double wmx = GetSurfPtr( indx )->GetWMax();

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

void Geom::UpdateTesselate( int indx, vector< vector< vec3d > > &pnts, vector< vector< vec3d > > &norms,
                            vector< vector< vec3d > > &uw_pnts, bool degen )
{
    m_SurfVec[indx].Tesselate( m_TessU(), m_TessW(), pnts, norms, uw_pnts, m_CapUMinTess(), degen );
}

void Geom::UpdateTesselate( int indx, vector< vector< vec3d > > &pnts, vector< vector< vec3d > > &norms, bool degen )
{
    vector< vector< vec3d > > uw_pnts;
    UpdateTesselate( indx, pnts, norms, uw_pnts, degen );
}

void Geom::UpdateSplitTesselate( int indx, vector< vector< vector< vec3d > > > &pnts, vector< vector< vector< vec3d > > > &norms )
{
    m_SurfVec[indx].SplitTesselate( m_TessU(), m_TessW(), pnts, norms, m_CapUMinTess() );
}

void Geom::CalcTexCoords( int indx, vector< vector< vector< double > > > &utex, vector< vector< vector< double > > > &vtex, const vector< vector< vector< vec3d > > > & pnts )
{
    int nu = m_SurfVec[indx].GetNumUFeature() - 1;
    int nv = m_SurfVec[indx].GetNumWFeature() - 1;

    int n = nu * nv;

    utex.resize(n);
    vtex.resize(n);

    int k = 0;
    for ( int i = 0; i < nu; i++ )
    {

        for ( int j = 0; j < nv; j++ )
        {
            int nui = pnts[k].size();

            utex[k].resize(nui);
            vtex[k].resize(nui);
            for ( int ii = 0; ii < nui; ii++ )
            {
                int nvj = pnts[k][0].size();

                utex[k][ii].resize(nvj);
                vtex[k][ii].resize(nvj);


                for ( int jj = 0; jj < nvj; jj++ )
                {
                    if ( ii == 0 )
                    {
                        if ( i == 0 )
                        {
                            utex[k][ii][jj] = 0.0;
                        }
                        else
                        {
                            int ilast = utex[(i-1)*nv+j].size() - 1;
                            utex[k][ii][jj] = utex[(i-1)*nv+j][ilast][jj]; // previous kpatch iend;
                        }
                    }
                    else
                    {
                        double du = dist( pnts[k][ii][jj], pnts[k][ii-1][jj] );
                        if ( du < 1e-6 )
                        {
                            du = 1.0;
                        }
                        utex[k][ii][jj] = utex[k][ii-1][jj] + du;
                    }

                    if ( jj == 0 )
                    {
                        if ( j == 0 )
                        {
                            vtex[k][ii][jj] = 0.0;
                        }
                        else
                        {
                            int jlast = vtex[i*nv+j-1][ii].size() - 1;
                            vtex[k][ii][jj] = vtex[i*nv+j-1][ii][jlast]; // previous kpatch jend;
                        }
                    }
                    else
                    {
                        double dv = dist( pnts[k][ii][jj], pnts[k][ii][jj-1] );
                        if ( dv < 1e-6 )
                        {
                            dv = 1.0;
                        }
                        vtex[k][ii][jj] = vtex[k][ii][jj-1] + dv;
                    }
                }
            }
            k++;
        }
    }

    k = 0;
    for ( int i = 0; i < nu; i++ )
    {
        for ( int j = 0; j < nv; j++ )
        {
            int nui = pnts[k].size();

            for ( int ii = 0; ii < nui; ii++ )
            {
                int nvj = pnts[k][0].size();

                for ( int jj = 0; jj < nvj; jj++ )
                {
                    int kjlast = i*nv+nv-1;
                    int kilast = (nu-1)*nv+j;

                    int imax = utex[kilast].size()-1;
                    int jmax = vtex[kjlast][0].size()-1;

                    utex[k][ii][jj] /= utex[kilast][imax][jj];
                    vtex[k][ii][jj] /= vtex[kjlast][ii][jmax];
                }
            }
            k++;
        }
    }
}

void Geom::UpdateEndCaps()
{
    if ( m_CappingDone )
    {
        return;
    }
    m_CappingDone = true;

    int nmain = m_MainSurfVec.size();
    m_CapUMinSuccess.resize( nmain );
    m_CapUMaxSuccess.resize( nmain );
    m_CapWMinSuccess.resize( nmain );
    m_CapWMaxSuccess.resize( nmain );

    // cycle through all vspsurfs, check if wing type then cap using new Code-Eli cap surface creator
    for ( int i = 0; i < nmain; i++ )
    {
        m_CapUMinSuccess[i] = false;
        m_CapUMaxSuccess[i] = false;
        m_CapWMinSuccess[i] = false;
        m_CapWMaxSuccess[i] = false;

        // NOTE: These return a bool that is true if it modified the surface to create a cap
        m_CapUMinSuccess[i] = m_MainSurfVec[i].CapUMin(m_CapUMinOption(), m_CapUMinLength(), m_CapUMinStrength(), m_CapUMinOffset(), m_CapUMinSweepFlag());
        m_CapUMaxSuccess[i] = m_MainSurfVec[i].CapUMax(m_CapUMaxOption(), m_CapUMaxLength(), m_CapUMaxStrength(), m_CapUMaxOffset(), m_CapUMaxSweepFlag());
        m_CapWMinSuccess[i] = m_MainSurfVec[i].CapWMin(m_CapWMinOption());
        m_CapWMaxSuccess[i] = m_MainSurfVec[i].CapWMax(m_CapWMaxOption());
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
    int num_surf = GetNumTotalSurfs();
    m_SurfVec.clear();
    m_SurfIndxVec.clear();
    m_SurfSymmMap.clear();
    m_SurfVec.resize( num_surf, VspSurf() );
    m_SurfIndxVec.resize( num_surf, -1 );
    m_SurfSymmMap.resize( num_surf );

    int num_main = GetNumMainSurfs();
    for ( int i = 0 ; i < ( int )num_main ; i++ )
    {
        m_SurfVec[i] = m_MainSurfVec[i];
        m_SurfIndxVec[i] = i;
        m_SurfSymmMap[ m_SurfIndxVec[i] ].push_back( i );
    }

    vector<Matrix4d> transMats;
    transMats.resize( num_surf, Matrix4d() );
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

    for ( int i = 0 ; i < ( int )transMats.size() ; i++ )
    {
        transMats[i].initMat( relTrans.data() );
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
                        m_SurfVec[j + k * numAddSurfs] = m_SurfVec[j - currentIndex];
                        m_SurfIndxVec[j + k * numAddSurfs] = m_SurfIndxVec[j - currentIndex];
                        m_SurfSymmMap[ m_SurfIndxVec[j + k * numAddSurfs] ].push_back( j + k * numAddSurfs );
                        transMats[j + k * numAddSurfs].initMat( transMats[j - currentIndex].data() );
                        transMats[j + k * numAddSurfs].postMult( Ref.data() ); // Apply Reflection

                        // Increment rotation by the angle
                        Ref.postMult( Ref_Orig.data() );
                        addIndex++;
                    }
                    // Reset reflection matrices to the beginning angle
                    Ref = Ref_Orig;
                }
                else
                {
                    m_SurfVec[j] = m_SurfVec[j - currentIndex];
                    m_SurfVec[j].FlipNormal();
                    m_SurfIndxVec[j] = m_SurfIndxVec[j - currentIndex];
                    m_SurfSymmMap[ m_SurfIndxVec[ j ] ].push_back( j );
                    transMats[j].initMat( transMats[j - currentIndex].data() );
                    transMats[j].postMult( Ref.data() ); // Apply Reflection
                    addIndex++;
                }
            }

            currentIndex += addIndex;
            radial = false;
        }
    }

    //==== Apply Transformations ====//
    for ( int i = 0 ; i < num_surf ; i++ )
    {
        transMats[i].postMult( symmOriginMat.data() );
        m_SurfVec[i].Transform( transMats[i] ); // Apply total transformation to main surfaces
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
}

void Geom::UpdateBBox()
{
    //==== Load Bounding Box ====//
    m_BBox.Reset();
    BndBox bb;
    for ( int i = 0 ; i < ( int )m_SurfVec.size() ; i++ )
    {
        m_SurfVec[i].GetBoundingBox( bb );
        m_BBox.Update( bb );
    }

    // If the surface vec size is zero ( like blank geom )
    // set bbox to zero size

    if ( !m_SurfVec.size() )
    {
        m_BBox.Update( vec3d(0,0,0) );
    }

    m_BbXLen = m_BBox.GetMax( 0 ) - m_BBox.GetMin( 0 );
    m_BbYLen = m_BBox.GetMax( 1 ) - m_BBox.GetMin( 1 );
    m_BbZLen = m_BBox.GetMax( 2 ) - m_BBox.GetMin( 2 );

    m_BbXMin = m_BBox.GetMin( 0 );
    m_BbYMin = m_BBox.GetMin( 1 );
    m_BbZMin = m_BBox.GetMin( 2 );
}

//Sets cfd surf types to negative if or normal depending on the state of the GUI negative button
void Geom::UpdateFlags( )
{
    for( int i = 0; i < (int)m_MainSurfVec.size(); i++ )
    {
        m_MainSurfVec[i].SetSurfCfdType( m_NegativeVolumeFlag.Get() );
    }
}

void Geom::WriteFeatureLinesDXF( FILE * file_name, const BndBox &dxfbox )
{
    double tol = 10e-2; // Feature line tesselation tolerance

    bool color = m_Vehicle->m_DXFColorFlag.Get();

    // Bounding box diagonal, used to separate multi-view drawings
    vec3d shiftvec = dxfbox.GetMax() - dxfbox.GetMin();

    // Shift the vehicle bounding box to align with the +x, +y, +z axes at the orgin
    vec3d to_orgin = GetVecToOrgin( dxfbox );

    for ( int i = 0; i < m_SurfVec.size(); i++ )
    {
        vector < vector < vec3d > > allflines, allflines1, allflines2, allflines3, allflines4;

        if ( m_GuiDraw.GetDispFeatureFlag() )
        {
            int nu = m_SurfVec[i].GetNumUFeature();
            int nw = m_SurfVec[i].GetNumWFeature();
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
                FeatureLinesShift( allflines1, shiftvec, vsp::VIEW_SHIFT::LEFT, m_Vehicle->m_DXF4View1_rot(), 0 );
                string layer_v1 = layer + "_v1";

                allflines2 = allflines;
                FeatureLinesManipulate( allflines2, m_Vehicle->m_DXF4View2(), m_Vehicle->m_DXF4View2_rot(), shiftvec );
                FeatureLinesShift( allflines2, shiftvec, vsp::VIEW_SHIFT::RIGHT, m_Vehicle->m_DXF4View2_rot(), 0 );
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
                FeatureLinesShift( allflines1, shiftvec, vsp::VIEW_SHIFT::UP, m_Vehicle->m_DXF4View1_rot(), 0 );

                allflines3 = allflines;
                FeatureLinesManipulate( allflines3, m_Vehicle->m_DXF4View3(), m_Vehicle->m_DXF4View3_rot(), shiftvec );
                FeatureLinesShift( allflines3, shiftvec, vsp::VIEW_SHIFT::DOWN, m_Vehicle->m_DXF4View3_rot(), 0 );

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
                FeatureLinesShift( allflines1, shiftvec, vsp::VIEW_SHIFT::UP, m_Vehicle->m_DXF4View1_rot(), m_Vehicle->m_DXF4View2_rot() );
                FeatureLinesShift( allflines1, shiftvec, vsp::VIEW_SHIFT::LEFT, m_Vehicle->m_DXF4View1_rot(), m_Vehicle->m_DXF4View3_rot() );

                allflines2 = allflines;
                FeatureLinesManipulate( allflines2, m_Vehicle->m_DXF4View2(), m_Vehicle->m_DXF4View2_rot(), shiftvec );
                FeatureLinesShift( allflines2, shiftvec, vsp::VIEW_SHIFT::UP, m_Vehicle->m_DXF4View2_rot(), m_Vehicle->m_DXF4View1_rot() );
                FeatureLinesShift( allflines2, shiftvec, vsp::VIEW_SHIFT::RIGHT, m_Vehicle->m_DXF4View2_rot(), m_Vehicle->m_DXF4View4_rot() );

                allflines3 = allflines;
                FeatureLinesManipulate( allflines3, m_Vehicle->m_DXF4View3(), m_Vehicle->m_DXF4View3_rot(), shiftvec );
                FeatureLinesShift( allflines3, shiftvec, vsp::VIEW_SHIFT::DOWN, m_Vehicle->m_DXF4View3_rot(), m_Vehicle->m_DXF4View4_rot() );
                FeatureLinesShift( allflines3, shiftvec, vsp::VIEW_SHIFT::LEFT, m_Vehicle->m_DXF4View3_rot(), m_Vehicle->m_DXF4View1_rot() );

                allflines4 = allflines;
                FeatureLinesManipulate( allflines4, m_Vehicle->m_DXF4View4(), m_Vehicle->m_DXF4View4_rot(), shiftvec );
                FeatureLinesShift( allflines4, shiftvec, vsp::VIEW_SHIFT::DOWN, m_Vehicle->m_DXF4View4_rot(), m_Vehicle->m_DXF4View3_rot() );
                FeatureLinesShift( allflines4, shiftvec, vsp::VIEW_SHIFT::RIGHT, m_Vehicle->m_DXF4View4_rot(), m_Vehicle->m_DXF4View2_rot() );

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
                FeatureLinesShift( projectionvec1, shiftvec, vsp::VIEW_SHIFT::LEFT, m_Vehicle->m_DXF4View1_rot(), 0 );
                string projectionlayer_v1 = projectionlayer + "_v1";

                WriteDXFPolylines2D( file_name, projectionvec1, projectionlayer_v1, color, m_Vehicle->m_ColorCount );
                m_Vehicle->m_ColorCount++;
            }

            vector < vector < vec3d > > projectionvec2 = GetGeomProjectionLines( m_Vehicle->m_DXF4View2(), to_orgin );

            if ( projectionvec2.size() > 0 )
            {
                FeatureLinesManipulate( projectionvec2, m_Vehicle->m_DXF4View2(), m_Vehicle->m_DXF4View2_rot(), shiftvec );
                FeatureLinesShift( projectionvec2, shiftvec, vsp::VIEW_SHIFT::RIGHT, m_Vehicle->m_DXF4View2_rot(), 0 );
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
                FeatureLinesShift( projectionvec1, shiftvec, vsp::VIEW_SHIFT::UP, m_Vehicle->m_DXF4View1_rot(), 0 );
                string projectionlayer_v1 = projectionlayer + "_v1";

                WriteDXFPolylines2D( file_name, projectionvec1, projectionlayer_v1, color, m_Vehicle->m_ColorCount );
                m_Vehicle->m_ColorCount++;
            }

            vector < vector < vec3d > > projectionvec3 = GetGeomProjectionLines( m_Vehicle->m_DXF4View3(), to_orgin );

            if ( projectionvec3.size() > 0 )
            {
                FeatureLinesManipulate( projectionvec3, m_Vehicle->m_DXF4View3(), m_Vehicle->m_DXF4View3_rot(), shiftvec );
                FeatureLinesShift( projectionvec3, shiftvec, vsp::VIEW_SHIFT::DOWN, m_Vehicle->m_DXF4View3_rot(), 0 );
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
                FeatureLinesShift( projectionvec1, shiftvec, vsp::VIEW_SHIFT::UP, m_Vehicle->m_DXF4View1_rot(), m_Vehicle->m_DXF4View2_rot() );
                FeatureLinesShift( projectionvec1, shiftvec, vsp::VIEW_SHIFT::LEFT, m_Vehicle->m_DXF4View1_rot(), m_Vehicle->m_DXF4View3_rot() );
                string projectionlayer_v1 = projectionlayer + "_v1";

                WriteDXFPolylines2D( file_name, projectionvec1, projectionlayer_v1, color, m_Vehicle->m_ColorCount );
                m_Vehicle->m_ColorCount++;
            }

            vector < vector < vec3d > > projectionvec2 = GetGeomProjectionLines( m_Vehicle->m_DXF4View2(), to_orgin );

            if ( projectionvec2.size() > 0 )
            {
                FeatureLinesManipulate( projectionvec2, m_Vehicle->m_DXF4View2(), m_Vehicle->m_DXF4View2_rot(), shiftvec );
                FeatureLinesShift( projectionvec2, shiftvec, vsp::VIEW_SHIFT::UP, m_Vehicle->m_DXF4View2_rot(), m_Vehicle->m_DXF4View1_rot() );
                FeatureLinesShift( projectionvec2, shiftvec, vsp::VIEW_SHIFT::RIGHT, m_Vehicle->m_DXF4View2_rot(), m_Vehicle->m_DXF4View4_rot() );
                string projectionlayer_v2 = projectionlayer + "_v2";

                WriteDXFPolylines2D( file_name, projectionvec2, projectionlayer_v2, color, m_Vehicle->m_ColorCount );
                m_Vehicle->m_ColorCount++;
            }

            vector < vector < vec3d > > projectionvec3 = GetGeomProjectionLines( m_Vehicle->m_DXF4View3(), to_orgin );

            if ( projectionvec3.size() > 0 )
            {
                FeatureLinesManipulate( projectionvec3, m_Vehicle->m_DXF4View3(), m_Vehicle->m_DXF4View3_rot(), shiftvec );
                FeatureLinesShift( projectionvec3, shiftvec, vsp::VIEW_SHIFT::DOWN, m_Vehicle->m_DXF4View3_rot(), m_Vehicle->m_DXF4View4_rot() );
                FeatureLinesShift( projectionvec3, shiftvec, vsp::VIEW_SHIFT::LEFT, m_Vehicle->m_DXF4View3_rot(), m_Vehicle->m_DXF4View1_rot() );
                string projectionlayer_v3 = projectionlayer + "_v3";

                WriteDXFPolylines2D( file_name, projectionvec3, projectionlayer_v3, color, m_Vehicle->m_ColorCount );
                m_Vehicle->m_ColorCount++;
            }

            vector < vector < vec3d > > projectionvec4 = GetGeomProjectionLines( m_Vehicle->m_DXF4View4(), to_orgin );

            if ( projectionvec4.size() > 0 )
            {
                FeatureLinesManipulate( projectionvec4, m_Vehicle->m_DXF4View4(), m_Vehicle->m_DXF4View4_rot(), shiftvec );
                FeatureLinesShift( projectionvec4, shiftvec, vsp::VIEW_SHIFT::DOWN, m_Vehicle->m_DXF4View4_rot(), m_Vehicle->m_DXF4View3_rot() );
                FeatureLinesShift( projectionvec4, shiftvec, vsp::VIEW_SHIFT::RIGHT, m_Vehicle->m_DXF4View4_rot(), m_Vehicle->m_DXF4View2_rot() );
                string projectionlayer_v4 = projectionlayer + "_v4";

                WriteDXFPolylines2D( file_name, projectionvec4, projectionlayer_v4, color, m_Vehicle->m_ColorCount );
                m_Vehicle->m_ColorCount++;
            }
        }
    }
}

vector< vector < vec3d > > Geom::GetGeomProjectionLines( int view, vec3d offset )
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
    double tol = 10e-2; // Feature line tesselation tolerance
    
    // Bounding box diagonal, used to separate multi-view drawings
    vec3d shiftvec = svgbox.GetMax() - svgbox.GetMin();

    // Shift the vehicle bounding box to align with the +x, +y, +z axes at the orgin
    vec3d to_orgin = GetVecToOrgin( svgbox );

    for ( int i = 0 ; i < ( int )m_SurfVec.size() ; i++ )
    {
        vector < vector < vec3d > > allflines, allflines1, allflines2, allflines3, allflines4;

        if( m_GuiDraw.GetDispFeatureFlag() )
        {
            int nu = m_SurfVec[i].GetNumUFeature();
            int nw = m_SurfVec[i].GetNumWFeature();
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
            FeatureLinesShift( allflines1, shiftvec, vsp::VIEW_SHIFT::LEFT, m_Vehicle->m_SVGView1_rot(), 0 );

            allflines2 = allflines;
            FeatureLinesManipulate( allflines2, m_Vehicle->m_SVGView2(), m_Vehicle->m_SVGView2_rot(), shiftvec );
            FeatureLinesShift( allflines2, shiftvec, vsp::VIEW_SHIFT::RIGHT, m_Vehicle->m_SVGView2_rot(), 0 );

            WriteSVGPolylines2D( root, allflines1, svgbox );
            WriteSVGPolylines2D( root, allflines2, svgbox );
        }
        else if ( m_Vehicle->m_SVGView() == vsp::VIEW_NUM::VIEW_2VER )
        {
            allflines1 = allflines;
            FeatureLinesManipulate( allflines1, m_Vehicle->m_SVGView1(), m_Vehicle->m_SVGView1_rot(), shiftvec );
            FeatureLinesShift( allflines1, shiftvec, vsp::VIEW_SHIFT::UP, m_Vehicle->m_SVGView1_rot(), 0 );

            allflines3 = allflines;
            FeatureLinesManipulate( allflines3, m_Vehicle->m_SVGView3(), m_Vehicle->m_SVGView3_rot(), shiftvec );
            FeatureLinesShift( allflines3, shiftvec, vsp::VIEW_SHIFT::DOWN, m_Vehicle->m_SVGView3_rot(), 0 );

            WriteSVGPolylines2D( root, allflines1, svgbox );
            WriteSVGPolylines2D( root, allflines3, svgbox );
        }
        else if ( m_Vehicle->m_SVGView() == vsp::VIEW_NUM::VIEW_4 )
        {
            allflines1 = allflines;

            FeatureLinesManipulate( allflines1, m_Vehicle->m_SVGView1(), m_Vehicle->m_SVGView1_rot(), shiftvec );
            FeatureLinesShift( allflines1, shiftvec, vsp::VIEW_SHIFT::UP, m_Vehicle->m_SVGView1_rot(), m_Vehicle->m_SVGView2_rot() );
            FeatureLinesShift( allflines1, shiftvec, vsp::VIEW_SHIFT::LEFT, m_Vehicle->m_SVGView1_rot(), m_Vehicle->m_SVGView3_rot() );

            allflines2 = allflines;
            FeatureLinesManipulate( allflines2, m_Vehicle->m_SVGView2(), m_Vehicle->m_SVGView2_rot(), shiftvec );
            FeatureLinesShift( allflines2, shiftvec, vsp::VIEW_SHIFT::UP, m_Vehicle->m_SVGView2_rot(), m_Vehicle->m_SVGView1_rot() );
            FeatureLinesShift( allflines2, shiftvec, vsp::VIEW_SHIFT::RIGHT, m_Vehicle->m_SVGView2_rot(), m_Vehicle->m_SVGView4_rot() );

            allflines3 = allflines;
            FeatureLinesManipulate( allflines3, m_Vehicle->m_SVGView3(), m_Vehicle->m_SVGView3_rot(), shiftvec );
            FeatureLinesShift( allflines3, shiftvec, vsp::VIEW_SHIFT::DOWN, m_Vehicle->m_SVGView3_rot(), m_Vehicle->m_SVGView4_rot() );
            FeatureLinesShift( allflines3, shiftvec, vsp::VIEW_SHIFT::LEFT, m_Vehicle->m_SVGView3_rot(), m_Vehicle->m_SVGView1_rot() );

            allflines4 = allflines;
            FeatureLinesManipulate( allflines4, m_Vehicle->m_SVGView4(), m_Vehicle->m_SVGView4_rot(), shiftvec );
            FeatureLinesShift( allflines4, shiftvec, vsp::VIEW_SHIFT::DOWN, m_Vehicle->m_SVGView4_rot(), m_Vehicle->m_SVGView3_rot() );
            FeatureLinesShift( allflines4, shiftvec, vsp::VIEW_SHIFT::RIGHT, m_Vehicle->m_SVGView4_rot(), m_Vehicle->m_SVGView2_rot() );

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
        FeatureLinesShift( projectionvec1, shiftvec, vsp::VIEW_SHIFT::LEFT, m_Vehicle->m_SVGView1_rot(), 0 );
        WriteSVGPolylines2D( root, projectionvec1, svgbox );

        vector < vector < vec3d > > projectionvec2 = GetGeomProjectionLines( m_Vehicle->m_SVGView2(), to_orgin );

        FeatureLinesManipulate( projectionvec2, m_Vehicle->m_SVGView2(), m_Vehicle->m_SVGView2_rot(), shiftvec );
        FeatureLinesShift( projectionvec2, shiftvec, vsp::VIEW_SHIFT::RIGHT, m_Vehicle->m_SVGView2_rot(), 0 );
        WriteSVGPolylines2D( root, projectionvec2, svgbox );
    }
    else if ( m_Vehicle->m_SVGView() == vsp::VIEW_NUM::VIEW_2VER )
    {
        vector < vector < vec3d > > projectionvec1 = GetGeomProjectionLines( m_Vehicle->m_SVGView1(), to_orgin );

        FeatureLinesManipulate( projectionvec1, m_Vehicle->m_SVGView1(), m_Vehicle->m_SVGView1_rot(), shiftvec );
        FeatureLinesShift( projectionvec1, shiftvec, vsp::VIEW_SHIFT::UP, m_Vehicle->m_SVGView1_rot(), 0 );
        WriteSVGPolylines2D( root, projectionvec1, svgbox );

        vector < vector < vec3d > > projectionvec3 = GetGeomProjectionLines( m_Vehicle->m_SVGView3(), to_orgin );

        FeatureLinesManipulate( projectionvec3, m_Vehicle->m_SVGView3(), m_Vehicle->m_SVGView3_rot(), shiftvec );
        FeatureLinesShift( projectionvec3, shiftvec, vsp::VIEW_SHIFT::DOWN, m_Vehicle->m_SVGView3_rot(), 0 );
        WriteSVGPolylines2D( root, projectionvec3, svgbox );

    }
    else if ( m_Vehicle->m_SVGView() == vsp::VIEW_NUM::VIEW_4 )
    {
        vector < vector < vec3d > > projectionvec1 = GetGeomProjectionLines( m_Vehicle->m_SVGView1(), to_orgin );

        FeatureLinesManipulate( projectionvec1, m_Vehicle->m_SVGView1(), m_Vehicle->m_SVGView1_rot(), shiftvec );
        FeatureLinesShift( projectionvec1, shiftvec, vsp::VIEW_SHIFT::UP, m_Vehicle->m_SVGView1_rot(), m_Vehicle->m_SVGView2_rot() );
        FeatureLinesShift( projectionvec1, shiftvec, vsp::VIEW_SHIFT::LEFT, m_Vehicle->m_SVGView1_rot(), m_Vehicle->m_SVGView3_rot() );
        WriteSVGPolylines2D( root, projectionvec1, svgbox );

        vector < vector < vec3d > > projectionvec2 = GetGeomProjectionLines( m_Vehicle->m_SVGView2(), to_orgin );

        FeatureLinesManipulate( projectionvec2, m_Vehicle->m_SVGView2(), m_Vehicle->m_SVGView2_rot(), shiftvec );
        FeatureLinesShift( projectionvec2, shiftvec, vsp::VIEW_SHIFT::UP, m_Vehicle->m_SVGView2_rot(), m_Vehicle->m_SVGView1_rot() );
        FeatureLinesShift( projectionvec2, shiftvec, vsp::VIEW_SHIFT::RIGHT, m_Vehicle->m_SVGView2_rot(), m_Vehicle->m_SVGView4_rot() );
        WriteSVGPolylines2D( root, projectionvec2, svgbox );

        vector < vector < vec3d > > projectionvec3 = GetGeomProjectionLines( m_Vehicle->m_SVGView3(), to_orgin );

        FeatureLinesManipulate( projectionvec3, m_Vehicle->m_SVGView3(), m_Vehicle->m_SVGView3_rot(), shiftvec );
        FeatureLinesShift( projectionvec3, shiftvec, vsp::VIEW_SHIFT::DOWN, m_Vehicle->m_SVGView3_rot(), m_Vehicle->m_SVGView4_rot() );
        FeatureLinesShift( projectionvec3, shiftvec, vsp::VIEW_SHIFT::LEFT, m_Vehicle->m_SVGView3_rot(), m_Vehicle->m_SVGView1_rot() );
        WriteSVGPolylines2D( root, projectionvec3, svgbox );

        vector < vector < vec3d > > projectionvec4 = GetGeomProjectionLines( m_Vehicle->m_SVGView4(), to_orgin );

        FeatureLinesManipulate( projectionvec4, m_Vehicle->m_SVGView4(), m_Vehicle->m_SVGView4_rot(), shiftvec );
        FeatureLinesShift( projectionvec4, shiftvec, vsp::VIEW_SHIFT::DOWN, m_Vehicle->m_SVGView4_rot(), m_Vehicle->m_SVGView3_rot() );
        FeatureLinesShift( projectionvec4, shiftvec, vsp::VIEW_SHIFT::RIGHT, m_Vehicle->m_SVGView4_rot(), m_Vehicle->m_SVGView2_rot() );
        WriteSVGPolylines2D( root, projectionvec4, svgbox );
    }
}

void Geom::UpdateDrawObj()
{
    m_FeatureDrawObj_vec.clear();
    m_FeatureDrawObj_vec.resize(1);
    m_FeatureDrawObj_vec[0].m_GeomChanged = true;
    m_FeatureDrawObj_vec[0].m_LineWidth = 1.0;
    m_FeatureDrawObj_vec[0].m_LineColor = vec3d( 0.0, 0.0, 0.0 );

    double tol = 1e-2;

    m_WireShadeDrawObj_vec.clear();
    m_WireShadeDrawObj_vec.resize( 2 );
    m_WireShadeDrawObj_vec[0].m_FlipNormals = false;
    m_WireShadeDrawObj_vec[1].m_FlipNormals = true;
    m_WireShadeDrawObj_vec[0].m_GeomChanged = true;
    m_WireShadeDrawObj_vec[1].m_GeomChanged = true;

    //==== Tesselate Surface ====//
    for ( int i = 0 ; i < ( int )m_SurfVec.size() ; i++ )
    {
        vector< vector < vector < vec3d > > > pnts;
        vector< vector < vector < vec3d > > > norms;

        UpdateSplitTesselate( i, pnts, norms );

        vector< vector < vector < double > > > utex;
        vector< vector < vector < double > > > vtex;

        CalcTexCoords( i, utex, vtex, pnts );

        int iflip = 0;
        if ( m_SurfVec[i].GetFlipNormal() )
        {
            iflip = 1;
        }

        m_WireShadeDrawObj_vec[iflip].m_PntMesh.insert( m_WireShadeDrawObj_vec[iflip].m_PntMesh.end(),
                pnts.begin(), pnts.end() );
        m_WireShadeDrawObj_vec[iflip].m_NormMesh.insert( m_WireShadeDrawObj_vec[iflip].m_NormMesh.end(),
                norms.begin(), norms.end() );

        m_WireShadeDrawObj_vec[iflip].m_uTexMesh.insert( m_WireShadeDrawObj_vec[iflip].m_uTexMesh.end(),
                utex.begin(), utex.end() );
        m_WireShadeDrawObj_vec[iflip].m_vTexMesh.insert( m_WireShadeDrawObj_vec[iflip].m_vTexMesh.end(),
                vtex.begin(), vtex.end() );

        if( m_GuiDraw.GetDispFeatureFlag() )
        {
            int nu = m_SurfVec[i].GetNumUFeature();
            for( int j = 0; j < nu; j++ )
            {
                vector < vec3d > ptline;
                m_SurfVec[i].TessUFeatureLine( j, ptline, tol );

                int n = ptline.size() - 1;

                m_FeatureDrawObj_vec[0].m_PntVec.reserve( m_FeatureDrawObj_vec[0].m_PntVec.size() + 2 * n );

                for ( int k = 0; k < n; k++ )
                {
                    m_FeatureDrawObj_vec[0].m_PntVec.push_back( ptline[ k ] );
                    m_FeatureDrawObj_vec[0].m_PntVec.push_back( ptline[ k + 1 ] );
                }
            }

            int nw = m_SurfVec[i].GetNumWFeature();
            for( int j = 0; j < nw; j++ )
            {
                vector < vec3d > ptline;
                m_SurfVec[i].TessWFeatureLine( j, ptline, tol );

                int n = ptline.size() - 1;

                m_FeatureDrawObj_vec[0].m_PntVec.reserve( m_FeatureDrawObj_vec[0].m_PntVec.size() + 2 * n );

                for ( int k = 0; k < n; k++ )
                {
                    m_FeatureDrawObj_vec[0].m_PntVec.push_back( ptline[ k ] );
                    m_FeatureDrawObj_vec[0].m_PntVec.push_back( ptline[ k + 1 ] );
                }
            }
        }
    }

    //==== Bounding Box ====//
    m_HighlightDrawObj.m_PntVec = m_BBox.GetBBoxDrawLines();

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
}

void Geom::UpdateDegenDrawObj()
{
    //=== DegenGeom ===//
    vector< DegenGeom > DegenGeomVec; // Vector of geom in degenerate representation
    CreateDegenGeom( DegenGeomVec, true );

    m_DegenSurfDrawObj_vec.clear();
    m_DegenPlateDrawObj_vec.clear();
    m_DegenCamberPlateDrawObj_vec.clear();
    m_DegenSubSurfDrawObj_vec.clear();

    for ( int i = 0; i < (int)DegenGeomVec.size(); i++ )
    {
        //=== Degen Surface ===//
        DegenSurface degen_surf = DegenGeomVec[i].getDegenSurf();

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
        vector < DegenPlate > degen_plate_vec = DegenGeomVec[i].getDegenPlates();

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
        vector < DegenSubSurf > degen_subsurf_vec = DegenGeomVec[i].getDegenSubSurfs();

        for ( int j = 0; j < degen_subsurf_vec.size(); j++ )
        {
            DrawObj degen_subsurface_draw_obj;
            degen_subsurface_draw_obj.m_GeomChanged = true;

            if ( degen_subsurf_vec[j].typeId == SS_LINE )
            {
                if ( degen_subsurf_vec[j].u[0] == degen_subsurf_vec[j].u.back() ) // Constant U
                {
                    int u_index_low, u_index_high;
                    double u_value_low, u_value_high;

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
                    int w_index_low, w_index_high;
                    double w_value_low, w_value_high;

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

    xmlNodePtr geom_node = xmlNewChild( node, NULL, BAD_CAST "Geom", NULL );
    if ( geom_node )
    {
        XmlUtil::AddVectorBoolNode( geom_node, "Set_List", m_SetFlags );

        for( int i = 0; i < ( int )m_MainSourceVec.size(); i++ )
        {
            m_MainSourceVec[i]->EncodeXml( geom_node );
        }

        xmlNodePtr subsurfs_node = xmlNewChild( geom_node, NULL, BAD_CAST "SubSurfaces", NULL );

        if ( subsurfs_node )
        {
            for( int i = 0 ; i < ( int ) m_SubSurfVec.size() ; i++ )
            {
                xmlNodePtr sub_node = xmlNewChild( subsurfs_node, NULL, BAD_CAST "SubSurface", NULL );
                if ( sub_node )
                {
                    m_SubSurfVec[i]->EncodeXml( sub_node );
                }
            }
        }

        xmlNodePtr structvecnode = xmlNewChild( geom_node, NULL, BAD_CAST"FeaStructures", NULL );
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

        int num = XmlUtil::GetNumNames( geom_node, "Source" );

        for ( int i = 0 ; i < num ; i++ )
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
                        int prop_index = XmlUtil::FindInt( ss_info_node, "FeaPropertyIndex", 0 );
                        int cap_prop_index = XmlUtil::FindInt( ss_info_node, "CapFeaPropertyIndex", 0 );
                        int type = XmlUtil::FindInt( ss_info_node, "Type", vsp::SS_LINE );
                        SubSurface* ssurf = AddSubSurf( type, -1 );
                        if ( ssurf )
                        {
                            ssurf->DecodeXml( ss_node );
                            ssurf->SetFeaPropertyIndex( prop_index );
                            ssurf->SetCapFeaPropertyIndex( cap_prop_index );
                        }
                    }
                }
            }
        }

        // Decode FeaStructures
        xmlNodePtr structvecnode = XmlUtil::GetNode( geom_node, "FeaStructures", 0 );
        if ( structvecnode )
        {
            int num = XmlUtil::GetNumNames( structvecnode, "FeaStructureInfo" );

            for ( int i = 0; i < num; i++ )
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
    xmlNodePtr geom_node = xmlNewChild( node, NULL, BAD_CAST "Geom", NULL );
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
        m_TransAttachFlag = ATTACH_TRANS_NONE;
        m_RotAttachFlag = ATTACH_ROT_NONE;

        // override relative translation since the relative coordinates are arbitrary
        // when not attached to anything in V2
        overrideRelTrans = true;
    }
    else if ( posAttachFlag == V2_POS_ATTACH_FIXED )
    {
        m_TransAttachFlag = ATTACH_TRANS_COMP;
        m_RotAttachFlag = ATTACH_ROT_NONE;
    }
    else if ( posAttachFlag == V2_POS_ATTACH_UV )
    {
        m_TransAttachFlag = ATTACH_TRANS_UV;
        m_RotAttachFlag = ATTACH_ROT_NONE;
    }
    else if ( posAttachFlag == V2_POS_ATTACH_MATRIX )
    {
        m_TransAttachFlag = ATTACH_TRANS_COMP;
        m_RotAttachFlag = ATTACH_ROT_COMP;

        // override relative translation since the relative coordinates are arbitrary
        // when attached in matrix mode
        overrideRelTrans = true;

        // override the AbsRelFlag, the only valid value for this attachment mode is relative
        m_AbsRelFlag = RELATIVE_XFORM;
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

        apptex.nameStr = Stringc( XmlUtil::FindString( tex_node, "Name", "Default_Name" ) );
        apptex.texStr  = Stringc( XmlUtil::FindString( tex_node, "Texture_Name", "Default_Name" ) );
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

void Geom::ResetGeomChangedFlag()
{
    for ( int i = 0 ; i < ( int )m_WireShadeDrawObj_vec.size() ; i++ )
    {
        m_WireShadeDrawObj_vec[i].m_GeomChanged = false;
    }
}

void Geom::LoadMainDrawObjs( vector< DrawObj* > & draw_obj_vec )
{
    char str[256];

    for ( int i = 0; i < (int)m_WireShadeDrawObj_vec.size(); i++ )
    {
        // Symmetry drawObjs have same m_ID. Make them unique by adding index
        // at the end of m_ID.
        sprintf( str, "_%d", i );
        m_WireShadeDrawObj_vec[i].m_GeomID = m_ID + str;
        m_WireShadeDrawObj_vec[i].m_Visible = !m_GuiDraw.GetNoShowFlag();

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

        vec3d lineColor = vec3d( m_GuiDraw.GetWireColor().x() / 255.0,
                                 m_GuiDraw.GetWireColor().y() / 255.0,
                                 m_GuiDraw.GetWireColor().z() / 255.0 );

        switch ( m_GuiDraw.GetDrawType() )
        {
            case GeomGuiDraw::GEOM_DRAW_WIRE:
                m_WireShadeDrawObj_vec[i].m_LineWidth = 1.0;
                m_WireShadeDrawObj_vec[i].m_LineColor = lineColor;
                m_WireShadeDrawObj_vec[i].m_Type = DrawObj::VSP_WIRE_MESH;
                draw_obj_vec.push_back( &m_WireShadeDrawObj_vec[i] );
                break;

            case GeomGuiDraw::GEOM_DRAW_HIDDEN:
                m_WireShadeDrawObj_vec[i].m_LineColor = lineColor;
                m_WireShadeDrawObj_vec[i].m_Type = DrawObj::VSP_HIDDEN_MESH;
                draw_obj_vec.push_back( &m_WireShadeDrawObj_vec[i] );
                break;

            case GeomGuiDraw::GEOM_DRAW_SHADE:
                m_WireShadeDrawObj_vec[i].m_Type = DrawObj::VSP_SHADED_MESH;
                draw_obj_vec.push_back( &m_WireShadeDrawObj_vec[i] );
                break;

            case GeomGuiDraw::GEOM_DRAW_NONE:
                m_WireShadeDrawObj_vec[i].m_Type = DrawObj::VSP_SHADED_MESH;
                m_WireShadeDrawObj_vec[i].m_Visible = false;
                draw_obj_vec.push_back( &m_WireShadeDrawObj_vec[i] );
                break;

            case GeomGuiDraw::GEOM_DRAW_TEXTURE:
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
                    info.UFlip = texList[j]->m_FlipU.Get();
                    info.WFlip = texList[j]->m_FlipW.Get();
                    info.ID = texList[j]->GetID();
                    m_WireShadeDrawObj_vec[i].m_TextureInfos.push_back( info );
                }
                draw_obj_vec.push_back( &m_WireShadeDrawObj_vec[i] );
                break;
        }
    }

}

//==== Load All Draw Objects ====//
void Geom::LoadDrawObjs( vector< DrawObj* > & draw_obj_vec )
{
    char str[256];

    if ( m_GuiDraw.GetDisplayType() == GeomGuiDraw::DISPLAY_BEZIER )
    {
        LoadMainDrawObjs( draw_obj_vec );
    }
    else
    {
        UpdateDegenDrawObj();
    }

    // Load BoundingBox and Axes
    if ( m_Vehicle->IsGeomActive( m_ID ) )
    {
        m_HighlightDrawObj.m_Screen = DrawObj::VSP_MAIN_SCREEN;
        m_HighlightDrawObj.m_GeomID = BBOXHEADER + m_ID;
        m_HighlightDrawObj.m_LineWidth = 2.0;
        m_HighlightDrawObj.m_LineColor = vec3d( 1.0, 0., 0.0 );
        m_HighlightDrawObj.m_Type = DrawObj::VSP_LINES;
        draw_obj_vec.push_back( &m_HighlightDrawObj );

        for ( int i = 0; i < m_AxisDrawObj_vec.size(); i++ )
        {
            m_AxisDrawObj_vec[i].m_Screen = DrawObj::VSP_MAIN_SCREEN;
            sprintf( str, "_%d", i );
            m_AxisDrawObj_vec[i].m_GeomID = m_ID + "Axis_" + str;
            m_AxisDrawObj_vec[i].m_LineWidth = 2.0;
            m_AxisDrawObj_vec[i].m_Type = DrawObj::VSP_LINES;
            draw_obj_vec.push_back( &m_AxisDrawObj_vec[i] );
        }
    }

    if ( m_GuiDraw.GetDisplayType() == GeomGuiDraw::DISPLAY_BEZIER )
    {
        // Load Feature Lines
        if ( m_GuiDraw.GetDispFeatureFlag() && !m_GuiDraw.GetNoShowFlag() )
        {
            for ( int i = 0; i < m_FeatureDrawObj_vec.size(); i++ )
            {
                m_FeatureDrawObj_vec[i].m_Screen = DrawObj::VSP_MAIN_SCREEN;
                sprintf( str, "_%d", i );
                m_FeatureDrawObj_vec[i].m_GeomID = m_ID + "Feature_" + str;
                m_FeatureDrawObj_vec[i].m_Type = DrawObj::VSP_LINES;
                draw_obj_vec.push_back( &m_FeatureDrawObj_vec[i] );
            }
        }

        // Load Subsurfaces
        RecolorSubSurfs( SubSurfaceMgr.GetCurrSurfInd() );
        if ( m_GuiDraw.GetDispSubSurfFlag() && !m_GuiDraw.GetNoShowFlag() )
        {
            for ( int i = 0; i < (int)m_SubSurfVec.size(); i++ )
            {
                m_SubSurfVec[i]->LoadDrawObjs( draw_obj_vec );
            }
        }
    }
    else if ( m_GuiDraw.GetDisplayType() == GeomGuiDraw::DISPLAY_DEGEN_SURF )
    {
        // Load DegenGeom
        for ( int i = 0; i < m_DegenSurfDrawObj_vec.size(); i++ )
        {
            sprintf( str, "_%d", i );
            m_DegenSurfDrawObj_vec[i].m_GeomID = m_ID + "Degen_Surf_" + str;
            m_DegenSurfDrawObj_vec[i].m_Visible = !m_GuiDraw.GetNoShowFlag();

            // Set Render Destination to Main VSP Window.
            m_DegenSurfDrawObj_vec[i].m_Screen = DrawObj::VSP_MAIN_SCREEN;

            Material material;
            material.SetMaterialToDefault();

            for ( int j = 0; j < 4; j++ )
                m_DegenSurfDrawObj_vec[i].m_MaterialInfo.Ambient[j] = (float)material.m_Ambi[j];

            for ( int j = 0; j < 4; j++ )
                m_DegenSurfDrawObj_vec[i].m_MaterialInfo.Diffuse[j] = (float)material.m_Diff[j];

            for ( int j = 0; j < 4; j++ )
                m_DegenSurfDrawObj_vec[i].m_MaterialInfo.Specular[j] = (float)material.m_Spec[j];

            for ( int j = 0; j < 4; j++ )
                m_DegenSurfDrawObj_vec[i].m_MaterialInfo.Emission[j] = (float)material.m_Emis[j];

            m_DegenSurfDrawObj_vec[i].m_MaterialInfo.Shininess = (float)material.m_Shininess;

            vec3d lineColor = vec3d( m_GuiDraw.GetWireColor().x() / 255.0,
                                        m_GuiDraw.GetWireColor().y() / 255.0,
                                        m_GuiDraw.GetWireColor().z() / 255.0 );

            switch ( m_GuiDraw.GetDrawType() )
            {
            case GeomGuiDraw::GEOM_DRAW_WIRE:
                m_DegenSurfDrawObj_vec[i].m_LineWidth = 1.0;
                m_DegenSurfDrawObj_vec[i].m_LineColor = lineColor;
                m_DegenSurfDrawObj_vec[i].m_Type = DrawObj::VSP_WIRE_QUADS;
                draw_obj_vec.push_back( &m_DegenSurfDrawObj_vec[i] );
                break;

            case GeomGuiDraw::GEOM_DRAW_HIDDEN:
                m_DegenSurfDrawObj_vec[i].m_LineColor = lineColor;
                m_DegenSurfDrawObj_vec[i].m_Type = DrawObj::VSP_HIDDEN_QUADS;
                draw_obj_vec.push_back( &m_DegenSurfDrawObj_vec[i] );
                break;

            case GeomGuiDraw::GEOM_DRAW_SHADE:
                m_DegenSurfDrawObj_vec[i].m_Type = DrawObj::VSP_SHADED_QUADS;
                draw_obj_vec.push_back( &m_DegenSurfDrawObj_vec[i] );
                break;

            case GeomGuiDraw::GEOM_DRAW_NONE:
                m_DegenSurfDrawObj_vec[i].m_Type = DrawObj::VSP_WIRE_QUADS;
                m_DegenSurfDrawObj_vec[i].m_Visible = false;
                draw_obj_vec.push_back( &m_DegenSurfDrawObj_vec[i] );
                break;

            case GeomGuiDraw::GEOM_DRAW_TEXTURE:
                m_DegenSurfDrawObj_vec[i].m_Type = DrawObj::VSP_SHADED_QUADS;
                draw_obj_vec.push_back( &m_DegenSurfDrawObj_vec[i] );
                break;
            }
        }
    }
    else if ( m_GuiDraw.GetDisplayType() == GeomGuiDraw::DISPLAY_DEGEN_PLATE )
    {
        for ( int i = 0; i < m_DegenPlateDrawObj_vec.size(); i++ )
        {
            sprintf( str, "_%d", i );
            m_DegenPlateDrawObj_vec[i].m_GeomID = m_ID + "Degen_Plate_" + str;
            m_DegenPlateDrawObj_vec[i].m_Visible = !m_GuiDraw.GetNoShowFlag();

            // Set Render Destination to Main VSP Window.
            m_DegenPlateDrawObj_vec[i].m_Screen = DrawObj::VSP_MAIN_SCREEN;

            Material material;
            material.SetMaterialToDefault();

            for ( int j = 0; j < 4; j++ )
                m_DegenPlateDrawObj_vec[i].m_MaterialInfo.Ambient[j] = (float)material.m_Ambi[j];

            for ( int j = 0; j < 4; j++ )
                m_DegenPlateDrawObj_vec[i].m_MaterialInfo.Diffuse[j] = (float)material.m_Diff[j];

            for ( int j = 0; j < 4; j++ )
                m_DegenPlateDrawObj_vec[i].m_MaterialInfo.Specular[j] = (float)material.m_Spec[j];

            for ( int j = 0; j < 4; j++ )
                m_DegenPlateDrawObj_vec[i].m_MaterialInfo.Emission[j] = (float)material.m_Emis[j];

            m_DegenPlateDrawObj_vec[i].m_MaterialInfo.Shininess = (float)material.m_Shininess;

            vec3d lineColor = vec3d( m_GuiDraw.GetWireColor().x() / 255.0,
                                        m_GuiDraw.GetWireColor().y() / 255.0,
                                        m_GuiDraw.GetWireColor().z() / 255.0 );

            switch ( m_GuiDraw.GetDrawType() )
            {
            case GeomGuiDraw::GEOM_DRAW_WIRE:
                m_DegenCamberPlateDrawObj_vec[i].m_LineWidth = 1.0;
                m_DegenCamberPlateDrawObj_vec[i].m_LineColor = lineColor;
                m_DegenCamberPlateDrawObj_vec[i].m_Type = DrawObj::VSP_WIRE_QUADS;
                draw_obj_vec.push_back( &m_DegenCamberPlateDrawObj_vec[i] );
                break;

            case GeomGuiDraw::GEOM_DRAW_HIDDEN:
                m_DegenCamberPlateDrawObj_vec[i].m_LineColor = lineColor;
                m_DegenCamberPlateDrawObj_vec[i].m_Type = DrawObj::VSP_HIDDEN_QUADS;
                draw_obj_vec.push_back( &m_DegenCamberPlateDrawObj_vec[i] );
                break;

            case GeomGuiDraw::GEOM_DRAW_SHADE:
                m_DegenCamberPlateDrawObj_vec[i].m_Type = DrawObj::VSP_SHADED_QUADS;
                draw_obj_vec.push_back( &m_DegenCamberPlateDrawObj_vec[i] );
                break;

            case GeomGuiDraw::GEOM_DRAW_NONE:
                m_DegenCamberPlateDrawObj_vec[i].m_Type = DrawObj::VSP_WIRE_QUADS;
                m_DegenCamberPlateDrawObj_vec[i].m_Visible = false;
                draw_obj_vec.push_back( &m_DegenCamberPlateDrawObj_vec[i] );
                break;

            case GeomGuiDraw::GEOM_DRAW_TEXTURE:
                m_DegenCamberPlateDrawObj_vec[i].m_Type = DrawObj::VSP_SHADED_QUADS;
                draw_obj_vec.push_back( &m_DegenCamberPlateDrawObj_vec[i] );
                break;
            }
        }
    }
    else if ( m_GuiDraw.GetDisplayType() == GeomGuiDraw::DISPLAY_DEGEN_CAMBER )
    {
        for ( int i = 0; i < m_DegenCamberPlateDrawObj_vec.size(); i++ )
        {
            sprintf( str, "_%d", i );
            m_DegenCamberPlateDrawObj_vec[i].m_GeomID = m_ID + "Degen_Camber_Plate_" + str;
            m_DegenCamberPlateDrawObj_vec[i].m_Visible = !m_GuiDraw.GetNoShowFlag();

            // Set Render Destination to Main VSP Window.
            m_DegenCamberPlateDrawObj_vec[i].m_Screen = DrawObj::VSP_MAIN_SCREEN;

            Material material;
            material.SetMaterialToDefault();

            for ( int j = 0; j < 4; j++ )
                m_DegenCamberPlateDrawObj_vec[i].m_MaterialInfo.Ambient[j] = (float)material.m_Ambi[j];

            for ( int j = 0; j < 4; j++ )
                m_DegenCamberPlateDrawObj_vec[i].m_MaterialInfo.Diffuse[j] = (float)material.m_Diff[j];

            for ( int j = 0; j < 4; j++ )
                m_DegenCamberPlateDrawObj_vec[i].m_MaterialInfo.Specular[j] = (float)material.m_Spec[j];

            for ( int j = 0; j < 4; j++ )
                m_DegenCamberPlateDrawObj_vec[i].m_MaterialInfo.Emission[j] = (float)material.m_Emis[j];

            m_DegenCamberPlateDrawObj_vec[i].m_MaterialInfo.Shininess = (float)material.m_Shininess;

            vec3d lineColor = vec3d( m_GuiDraw.GetWireColor().x() / 255.0,
                                        m_GuiDraw.GetWireColor().y() / 255.0,
                                        m_GuiDraw.GetWireColor().z() / 255.0 );

            switch ( m_GuiDraw.GetDrawType() )
            {
            case GeomGuiDraw::GEOM_DRAW_WIRE:
                m_DegenCamberPlateDrawObj_vec[i].m_LineWidth = 1.0;
                m_DegenCamberPlateDrawObj_vec[i].m_LineColor = lineColor;
                m_DegenCamberPlateDrawObj_vec[i].m_Type = DrawObj::VSP_WIRE_QUADS;
                draw_obj_vec.push_back( &m_DegenCamberPlateDrawObj_vec[i] );
                break;

            case GeomGuiDraw::GEOM_DRAW_HIDDEN:
                m_DegenCamberPlateDrawObj_vec[i].m_LineColor = lineColor;
                m_DegenCamberPlateDrawObj_vec[i].m_Type = DrawObj::VSP_HIDDEN_QUADS;
                draw_obj_vec.push_back( &m_DegenCamberPlateDrawObj_vec[i] );
                break;

            case GeomGuiDraw::GEOM_DRAW_SHADE:
                m_DegenCamberPlateDrawObj_vec[i].m_Type = DrawObj::VSP_SHADED_QUADS;
                draw_obj_vec.push_back( &m_DegenCamberPlateDrawObj_vec[i] );
                break;

            case GeomGuiDraw::GEOM_DRAW_NONE:
                m_DegenCamberPlateDrawObj_vec[i].m_Type = DrawObj::VSP_WIRE_QUADS;
                m_DegenCamberPlateDrawObj_vec[i].m_Visible = false;
                draw_obj_vec.push_back( &m_DegenCamberPlateDrawObj_vec[i] );
                break;

            case GeomGuiDraw::GEOM_DRAW_TEXTURE:
                m_DegenCamberPlateDrawObj_vec[i].m_Type = DrawObj::VSP_SHADED_QUADS;
                draw_obj_vec.push_back( &m_DegenCamberPlateDrawObj_vec[i] );
                break;
            }
        }
    }

    //if ( m_GuiDraw.GetDispDegenStickFlag() )
    //{
    //    for ( int i = 0; i < m_DegenStickDrawObj_vec.size(); i++ )
    //    {
    //        sprintf( str, "_%d", i );
    //        m_DegenStickDrawObj_vec[i].m_GeomID = m_ID + "Degen_Stick_" + str;

    //        // Set Render Destination to Main VSP Window.
    //        m_DegenStickDrawObj_vec[i].m_Screen = DrawObj::VSP_MAIN_SCREEN;

    //        vec3d lineColor = vec3d( m_GuiDraw.GetWireColor().x() / 255.0,
    //                                 m_GuiDraw.GetWireColor().y() / 255.0,
    //                                 m_GuiDraw.GetWireColor().z() / 255.0 );

    //        m_DegenStickDrawObj_vec[i].m_LineWidth = 1.0;
    //        m_DegenStickDrawObj_vec[i].m_LineColor = lineColor;

    //        if ( m_GuiDraw.GetDegenDrawType() == m_GuiDraw.GEOM_DRAW_WIRE )
    //        {
    //            m_DegenStickDrawObj_vec[i].m_Type = DrawObj::VSP_LINE_STRIP;
    //        }

    //        draw_obj_vec.push_back( &m_DegenStickDrawObj_vec[i] );
    //    }
    //}

    if ( m_GuiDraw.GetDispSubSurfFlag() && m_GuiDraw.GetDisplayType() != GeomGuiDraw::DISPLAY_BEZIER )
    {
        for ( int i = 0; i < m_DegenSubSurfDrawObj_vec.size(); i++ )
        {
            sprintf( str, "_%d", i );
            m_DegenSubSurfDrawObj_vec[i].m_GeomID = m_ID + "Degen_SubSurf_" + str;
            m_DegenSubSurfDrawObj_vec[i].m_Visible = !m_GuiDraw.GetNoShowFlag();

            // Set Render Destination to Main VSP Window.
            m_DegenSubSurfDrawObj_vec[i].m_Screen = DrawObj::VSP_MAIN_SCREEN;

            m_DegenSubSurfDrawObj_vec[i].m_LineWidth = 2.0;
            m_DegenSubSurfDrawObj_vec[i].m_LineColor = vec3d( 0.0, 0.0, 0.0 );

            m_DegenSubSurfDrawObj_vec[i].m_Type = DrawObj::VSP_LINE_STRIP;

            draw_obj_vec.push_back( &m_DegenSubSurfDrawObj_vec[i] );
        }
    }
}

void Geom::SetColor( int r, int g, int b )
{
    m_GuiDraw.SetWireColor( r, g, b );
}

vec3d Geom::GetColor()
{
    return m_GuiDraw.GetWireColor();
}

void Geom::SetMaterialToDefault()
{
    m_GuiDraw.SetMaterialToDefault();
}

void Geom::SetMaterial( std::string name, double ambi[], double diff[], double spec[], double emis[], double shin )
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
void Geom::CreateDegenGeom( vector<DegenGeom> &dgs, bool preview )
{
    vector< vector< vec3d > > pnts;
    vector< vector< vec3d > > nrms;
    vector< vector< vec3d > > uwpnts;

    for ( int i = 0 ; i < ( int )m_SurfVec.size() ; i++ )
    {
        bool urootcap = false;

        m_SurfVec[i].ResetUWSkip();
        if ( m_CapUMinSuccess[ m_SurfIndxVec[i] ] )
        {
            m_SurfVec[i].SetUSkipFirst( true );
            urootcap = true;
        }
        if ( m_CapUMaxSuccess[ m_SurfIndxVec[i] ] )
        {
            m_SurfVec[i].SetUSkipLast( true );
        }
        if ( m_CapWMinSuccess[ m_SurfIndxVec[i] ] )
        {
            m_SurfVec[i].SetWSkipFirst( true );
        }
        if ( m_CapWMaxSuccess[ m_SurfIndxVec[i] ] )
        {
            m_SurfVec[i].SetWSkipLast( true );
        }

        //==== Tesselate Surface ====//
        UpdateTesselate( i, pnts, nrms, uwpnts, true );
        m_SurfVec[i].ResetUWSkip();

        int surftype = DegenGeom::BODY_TYPE;
        if( m_SurfVec[i].GetSurfType() == vsp::WING_SURF || m_SurfVec[i].GetSurfType() == vsp::PROP_SURF )
        {
            surftype = DegenGeom::SURFACE_TYPE;
        }
        else if( m_SurfVec[i].GetSurfType() == vsp::DISK_SURF )
        {
            surftype = DegenGeom::DISK_TYPE;
        }

        CreateDegenGeom( dgs, pnts, nrms, uwpnts, urootcap, i, preview, m_SurfVec[i].GetFlipNormal(), surftype, m_SurfVec[i].GetFoilSurf() );
    }
}


void Geom::CreateDegenGeom( vector<DegenGeom> &dgs, const vector< vector< vec3d > > &pnts, const vector< vector< vec3d > > &nrms, const vector< vector< vec3d > > &uwpnts,
                            bool urootcap, int isurf, bool preview, bool flipnormal, int surftype, VspSurf *fs )
{
    DegenGeom degenGeom;
    degenGeom.setParentGeom( this );
    degenGeom.setSurfNum( isurf );

    degenGeom.setNumXSecs( pnts.size() );
    degenGeom.setNumPnts( pnts[0].size() );
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
        if ( m_SurfIndxVec[isurf] == m_SubSurfVec[j]->m_MainSurfIndx() )
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

    dgs.push_back(degenGeom);
}

//==== Create Degenerate Geometry Preview ====//
void Geom::CreateDegenGeomPreview( vector<DegenGeom> &dgs )
{
    // This function is similar to CreateDegenGeom, but has been simplified to generate only the
    // required degen plate,surface, and subsurface for updating the preview DrawObj vectors

    vector< vector< vec3d > > pnts;
    vector< vector< vec3d > > nrms;
    vector< vector< vec3d > > uwpnts;

    for ( int i = 0; i < (int)m_SurfVec.size(); i++ )
    {
        m_SurfVec[i].ResetUWSkip();
        if ( m_CapUMinSuccess[m_SurfIndxVec[i]] )
        {
            m_SurfVec[i].SetUSkipFirst( true );
        }
        if ( m_CapUMaxSuccess[m_SurfIndxVec[i]] )
        {
            m_SurfVec[i].SetUSkipLast( true );
        }
        if ( m_CapWMinSuccess[m_SurfIndxVec[i]] )
        {
            m_SurfVec[i].SetWSkipFirst( true );
        }
        if ( m_CapWMaxSuccess[m_SurfIndxVec[i]] )
        {
            m_SurfVec[i].SetWSkipLast( true );
        }

        //==== Tesselate Surface ====//
        UpdateTesselate( i, pnts, nrms, uwpnts, true );
        m_SurfVec[i].ResetUWSkip();

        DegenGeom degenGeom;
        degenGeom.setParentGeom( this );
        degenGeom.setSurfNum( i );

        degenGeom.setNumXSecs( pnts.size() );
        degenGeom.setNumPnts( pnts[0].size() );
        degenGeom.setName( GetName() );

        degenGeom.createDegenSurface( pnts, uwpnts, m_SurfVec[i].GetFlipNormal() );

        if ( m_SurfVec[i].GetSurfType() == vsp::WING_SURF )
        {
            degenGeom.setType( DegenGeom::SURFACE_TYPE );

            degenGeom.createSurfDegenPlate( pnts, uwpnts );
        }
        else if ( m_SurfVec[i].GetSurfType() == vsp::DISK_SURF )
        {
            degenGeom.setType( DegenGeom::DISK_TYPE );

            //degenGeom.createDegenDisk( pnts, m_SurfVec[i].GetFlipNormal() );
        }
        else
        {
            degenGeom.setType( DegenGeom::BODY_TYPE );

            degenGeom.createBodyDegenPlate( pnts, uwpnts );
        }

        for ( int j = 0; j < m_SubSurfVec.size(); j++ )
        {
            if ( m_SurfIndxVec[i] == m_SubSurfVec[j]->m_MainSurfIndx() )
            {
                degenGeom.addDegenSubSurf( m_SubSurfVec[j], i );
            }
        }

        dgs.push_back( degenGeom );
    }
}

//==== Set Sym Flag ====//
int Geom::GetSymFlag()
{
    return m_SymPlanFlag() | m_SymAxFlag();
}

//==== Return Pointer to First Surface ====//
VspSurf* Geom::GetSurfPtr()
{
    if ( m_SurfVec.size() )
    {
        return &m_SurfVec[0];
    }
    return NULL;
}

//==== Return Pointer to Surface indx ====//
VspSurf* Geom::GetSurfPtr( int indx )
{
    if ( indx >= 0 && indx < m_SurfVec.size() )
    {
        return &m_SurfVec[ indx ];
    }
    assert( true );
    return NULL;
}

//==== Count Number of Sym Surfaces ====//
int Geom::GetNumTotalSurfs()
{
    return GetNumSymmCopies() * GetNumMainSurfs();
}

int Geom::GetNumTotalHrmSurfs()
{
    return GetNumTotalSurfs();
}

//==== Count Number of Sym Copies of Each Surface ====//
int Geom::GetNumSymmCopies()
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
int Geom::GetNumSymFlags()
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

vec3d Geom::CompPnt01(const double &u, const double &w)
{
    return GetSurfPtr()->CompPnt01( u, w );
}

vec3d Geom::CompPnt01(const int &indx, const double &u, const double &w)
{
    return GetSurfPtr( indx )->CompPnt01( u, w );
}

bool Geom::CompRotCoordSys( const double &u, const double &w, Matrix4d &rotMat )
{
    VspSurf* surf_ptr = GetSurfPtr();
    if ( surf_ptr )
    {
        rotMat = surf_ptr->CompRotCoordSys( u, w );
        return true;
    }
    return false;
}

bool Geom::CompTransCoordSys( const double &u, const double &w, Matrix4d &transMat )
{
    VspSurf* surf_ptr = GetSurfPtr();
    if ( surf_ptr )
    {
        transMat = surf_ptr->CompTransCoordSys( u, w );
        return true;
    }
    return false;
}

void Geom::WriteXSecFile( int geom_no, FILE* dump_file )
{
    for ( int i = 0 ; i < ( int )m_SurfVec.size() ; i++ )
    {
        //==== Tessellate Surface ====//
        vector< vector< vec3d > > pnts;
        vector< vector< vec3d > > norms;

        UpdateTesselate( i, pnts, norms, false );

        //==== Write XSec Header ====//
        fprintf( dump_file, "\n" );
        fprintf( dump_file, "%s \n", ( char* ) m_Name.c_str() );
        fprintf( dump_file, " GROUP NUMBER      = %d \n", geom_no );

        if( m_SurfVec[i].GetSurfType() == vsp::WING_SURF )
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
    for ( int i = 0 ; i < ( int )m_SurfVec.size() ; i++ )
    {
        //==== Tessellate Surface ====//
        vector< vector< vec3d > > pnts;
        vector< vector< vec3d > > norms;

        UpdateTesselate( i, pnts, norms, false );
        //==== Write surface boundary extents ====//
        fprintf( dump_file, " %d %d %d\n", static_cast<int>( pnts[0].size() ), static_cast<int>( pnts.size() ), 1 );
    }
}

void Geom::WritePLOT3DFileXYZ( FILE* dump_file )
{
    for ( int i = 0 ; i < ( int )m_SurfVec.size() ; i++ )
    {
        //==== Tessellate Surface ====//
        vector< vector< vec3d > > pnts;
        vector< vector< vec3d > > norms;

        UpdateTesselate( i, pnts, norms, false );

        //==== Write XSec Data ====//
        for ( int j = 0 ; j < ( int )pnts.size() ; j++ )
        {
            for ( int k = 0 ; k < ( int )pnts[j].size() ; k++ )
            {
                int kflip = k;
                if ( !m_SurfVec[i].GetFlipNormal() )
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
                if ( !m_SurfVec[i].GetFlipNormal() )
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
                if ( !m_SurfVec[i].GetFlipNormal() )
                {
                    kflip = pnts[j].size() - 1 - k;
                }
                fprintf( dump_file, "%25.17e ", pnts[j][kflip].z() );
            }
        }
        fprintf( dump_file, "\n" );
    }
}

void Geom::CreateGeomResults( Results* res )
{
    res->Add( NameValData( "Type", vsp::GEOM_XSECS ) );
    res->Add( NameValData( "Num_Surfs", ( int )m_SurfVec.size() ) );

    for ( int i = 0 ; i < ( int )m_SurfVec.size() ; i++ )
    {
        //==== Tessellate Surface ====//
        vector< vector< vec3d > > pnts;
        vector< vector< vec3d > > norms;
        UpdateTesselate( i, pnts, norms, false );

        res->Add( NameValData( "Num_XSecs", static_cast<int>( pnts.size() ) ) );

        if ( pnts.size() )
        {
            res->Add( NameValData( "Num_Pnts_Per_XSec", static_cast<int>( pnts[0].size() ) ) );
        }

        //==== Write XSec Data ====//
        for ( int j = 0 ; j < ( int )pnts.size() ; j++ )
        {
            vector< vec3d > xsec_vec;
            for ( int k = 0 ; k < ( int )pnts[j].size() ; k++ )
            {
                xsec_vec.push_back(  pnts[j][k] );
            }
            res->Add( NameValData( "XSec_Pnts", xsec_vec ) );
        }
    }
}


void Geom::WriteX3D( xmlNodePtr node )
{
    xmlNodePtr set_node = xmlNewChild( node, NULL, BAD_CAST "IndexedFaceSet", NULL );
    xmlSetProp( set_node, BAD_CAST "solid", BAD_CAST "true" );
    xmlSetProp( set_node, BAD_CAST "creaseAngle", BAD_CAST "0.5"  );

    string indstr, crdstr;
    int offset = 0;
    char numstr[255];

    for ( int i = 0 ; i < ( int )m_SurfVec.size() ; i++ )
    {
        vector< vector< vec3d > > pnts;
        vector< vector< vec3d > > norms;
        UpdateTesselate( i, pnts, norms, false );
        int num_xsecs = pnts.size();
        int num_pnts = pnts[0].size();
        bool f_norm = m_SurfVec[i].GetFlipNormal();
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
                sprintf( numstr, "%lf %lf %lf ", pnt.x(), pnt.y(), pnt.z() );
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
                    sprintf( numstr, "%d %d %d %d -1 ", i3, i2, i1, i0 );
                }
                else
                {
                    sprintf( numstr, "%d %d %d %d -1 ", i0, i1, i2, i3 );
                }
                indstr += numstr;
            }
        }
    }

    xmlSetProp( set_node, BAD_CAST "coordIndex", BAD_CAST indstr.c_str() );

    xmlNodePtr coord_node = xmlNewChild( set_node, NULL, BAD_CAST "Coordinate", BAD_CAST " " );
    xmlSetProp( coord_node, BAD_CAST "point", BAD_CAST crdstr.c_str() );
}

void Geom::WritePovRay( FILE* fid, int comp_num )
{
    string name = GetName();
    StringUtil::chance_space_to_underscore( name );
    char str[4];
    sprintf( str, "_%d", comp_num );
    name.append( str );

    fprintf( fid, "#declare %s = mesh { \n", name.c_str() );
    for ( int i = 0 ; i < ( int )m_SurfVec.size() ; i++ )
    {
        vector< vector< vec3d > > pnts;
        vector< vector< vec3d > > norms;
        vec3d n0, n1, n2, n3, v0, v1, v2, v3;

        UpdateTesselate( i, pnts, norms, false );

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
vector< TMesh* > Geom::CreateTMeshVec()
{
    vector< TMesh* > TMeshVec;
    vector< vector<vec3d> > pnts;
    vector< vector<vec3d> > norms;
    vector< vector<vec3d> > uw_pnts;
    double tol=1.0e-12;

    for ( int i = 0 ; i < ( int )m_SurfVec.size(); i++ )
    {
        m_SurfVec[i].ResetUWSkip();
    }

    for ( int i = 0 ; i < ( int )m_SurfVec.size() - 1 ; i++ )
    {
        for ( int j = i + 1 ; j < ( int )m_SurfVec.size() ; j++ )
        {
            if ( m_SurfIndxVec[i] == m_SurfIndxVec[j] )
            {
                m_SurfVec[i].FlagDuplicate( &m_SurfVec[j] );
            }
        }
    }

    for ( int i = 0 ; i < ( int )m_SurfVec.size() ; i++ )
    {
        if ( m_SurfVec[i].GetNumSectU() != 0 && m_SurfVec[i].GetNumSectW() != 0 )
        {
            UpdateTesselate( i, pnts, norms, uw_pnts, false );
            m_SurfVec[i].ResetUWSkip(); // Done with skip flags.

            TMeshVec.push_back( new TMesh() );
            int itmesh = TMeshVec.size() - 1;
            TMeshVec[itmesh]->LoadGeomAttributes( this );
            TMeshVec[itmesh]->m_SurfNum = i;
            TMeshVec[itmesh]->m_UWPnts = uw_pnts;
            TMeshVec[itmesh]->m_XYZPnts = pnts;
            bool f_norm = m_SurfVec[i].GetFlipNormal();

            vec3d norm;
            vec3d v0, v1, v2, v3;
            vec3d uw0, uw1, uw2, uw3;
            vec3d d21, d01, d03, d23, d20, d31;

            for ( int j = 0 ; j < ( int )pnts.size() - 1 ; j++ )
            {
                for ( int k = 0 ; k < ( int )pnts[0].size() - 1 ; k++ )
                {
                    v0 = pnts[j][k];
                    v1 = pnts[j + 1][k];
                    v2 = pnts[j + 1][k + 1];
                    v3 = pnts[j][k + 1];

                    uw0 = uw_pnts[j][k];
                    uw1 = uw_pnts[j + 1][k];
                    uw2 = uw_pnts[j + 1][k + 1];
                    uw3 = uw_pnts[j][k + 1];

                    double quadrant = ( uw0.y() + uw1.y() + uw2.y() + uw3.y() ) / m_SurfVec[i].GetWMax(); // * 4 * 0.25 canceled.

                    d21 = v2 - v1;
                    d01 = v0 - v1;
                    d03 = v0 - v3;
                    d23 = v2 - v3;

                    if ( ( quadrant > 0 && quadrant < 1 ) || ( quadrant > 2 && quadrant < 3 ) )
                    {
                        d20 = v2 - v0;
                        if ( d21.mag() > tol && d01.mag() > tol && d20.mag() > tol )
                        {
                            norm = cross( d21, d01 );
                            norm.normalize();
                            if ( f_norm )
                            {
                                TMeshVec[itmesh]->AddTri( v0, v2, v1, norm * -1, uw0, uw2, uw1 );
                            }
                            else
                            {
                                TMeshVec[itmesh]->AddTri( v0, v1, v2, norm, uw0, uw1, uw2 );
                            }
                        }

                        if ( d03.mag() > tol && d23.mag() > tol && d20.mag() > tol )
                        {
                            norm = cross( d03, d23 );
                            norm.normalize();
                            if ( f_norm )
                            {
                                TMeshVec[itmesh]->AddTri( v0, v3, v2, norm * -1, uw0, uw3, uw2 );
                            }
                            else
                            {
                                TMeshVec[itmesh]->AddTri( v0, v2, v3, norm, uw0, uw2, uw3 );
                            }
                        }
                    }
                    else
                    {
                        d31 = v3 - v1;
                        if ( d01.mag() > tol && d31.mag() > tol && d03.mag() > tol )
                        {
                            norm = cross( d01, d03 );
                            norm.normalize();
                            if ( f_norm )
                            {
                                TMeshVec[itmesh]->AddTri( v0, v3, v1, norm * -1, uw0, uw3, uw1 );
                            }
                            else
                            {
                                TMeshVec[itmesh]->AddTri( v0, v1, v3, norm, uw0, uw1, uw3 );
                            }
                        }

                        if ( d21.mag() > tol && d23.mag() > tol && d31.mag() > tol )
                        {
                            norm = cross( d23, d21 );
                            norm.normalize();
                            if ( f_norm )
                            {
                                TMeshVec[itmesh]->AddTri( v1, v3, v2, norm * -1, uw1, uw3, uw2 );
                            }
                            else
                            {
                                TMeshVec[itmesh]->AddTri( v1, v2, v3, norm, uw1, uw2, uw3 );
                            }
                        }
                    }
                }
            }
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

void Geom::ChangeID( string id )
{
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
    SubSurface* ssurf = NULL;

    if ( m_MainSurfVec.size() <= 0 )
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
    return NULL;
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
    return NULL;
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
    FeaStructure* feastruct = NULL;

    if ( m_MainSurfVec.size() <= 0 || surf_index > m_MainSurfVec.size() || surf_index < 0 )
    {
        return feastruct;
    }

    if ( GetType().m_Type != BLANK_GEOM_TYPE && GetType().m_Type != PT_CLOUD_GEOM_TYPE && GetType().m_Type != HINGE_GEOM_TYPE && GetType().m_Type != MESH_GEOM_TYPE )
    {
        feastruct = new FeaStructure( GetID(), surf_index );

        if ( feastruct )
        {
            string defaultname = m_Name + "_Struct_" + to_string( m_FeaStructCount );
            feastruct->SetFeaStructName( defaultname );

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
    else
    {
        return false;
    }
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
    BaseSource* src_ptr = NULL;
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
    BaseSimpleSource* src_ptr = NULL;
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
        if( m_MainSurfVec[i].GetSurfType() == vsp::WING_SURF )
        {
            return true;
        }
    }
    return false;
}

void Geom::AppendWakeEdges( vector< vector< vec3d > > & edges )
{
    if( m_WakeActiveFlag() )
    {
        for( int i = 0; i < m_SurfVec.size(); i++ )
        {
            if( m_SurfVec[i].GetSurfType() == vsp::WING_SURF )
            {
                vector< vector< vec3d > > pnts;
                vector< vector< vec3d > > norms;

                m_SurfVec[i].TesselateTEforWake( pnts );

                vector< vec3d > edge( pnts.size() );

                for( int j = 0; j < pnts.size(); j++ )
                {
                    edge[j] = pnts[j][0];
                }

                edges.push_back( edge );
            }
        }
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

    m_MinActiveXSec = 0;
}
//==== Destructor ====//
GeomXSec::~GeomXSec()
{

}

void GeomXSec::UpdateDrawObj()
{
    Geom::UpdateDrawObj();

    Matrix4d attachMat;
    Matrix4d relTrans;
    attachMat = ComposeAttachMatrix();
    relTrans = attachMat;
    relTrans.affineInverse();
    relTrans.matMult( m_ModelMatrix.data() );
    relTrans.postMult( attachMat.data() );

    int nxsec = m_XSecSurf.NumXSec();
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

    XSec* axs = m_XSecSurf.FindXSec( m_ActiveXSec );
    if ( axs )
    {
        m_HighlightXSecDrawObj.m_PntVec = axs->GetDrawLines( relTrans );

        double w = axs->GetXSecCurve()->GetWidth();
        double h = axs->GetXSecCurve()->GetHeight();
        double scale = 1.0;
        if( w > h ) scale = 1.0 / w;
        else scale = 1.0 / h;

        Matrix4d mat;
        m_XSecSurf.GetBasicTransformation( Z_DIR, X_DIR, XS_SHIFT_MID, false, w * scale, mat );
        mat.scale( scale );

        VspCurve crv = axs->GetUntransformedCurve();
        crv.Transform( mat );

        if( w == 0 && h == 0 )
        {
            vector< vec3d > pts( 1, vec3d( 0, 0, 0 ) );
            m_CurrentXSecDrawObj.m_PntVec = pts;
            m_CurrentXSecDrawObj.m_PointSize = 5.0;
            m_CurrentXSecDrawObj.m_PointColor = vec3d( 0.0, 0.0, 0.0 );
            m_CurrentXSecDrawObj.m_Type = DrawObj::VSP_POINTS;
        }
        else
        {
            vector< vec3d > pts;
            crv.TessAdapt( pts, 1e-2, 10 );
            m_CurrentXSecDrawObj.m_PntVec = pts;
            m_CurrentXSecDrawObj.m_LineWidth = 1.5;
            m_CurrentXSecDrawObj.m_LineColor = vec3d( 0.0, 0.0, 0.0 );
            m_CurrentXSecDrawObj.m_Type = DrawObj::VSP_LINES;
        }
    }
    else
    {
        m_HighlightXSecDrawObj.m_PntVec = vector < vec3d > (0);
        m_CurrentXSecDrawObj.m_PntVec = vector < vec3d > (0);
    }

    m_HighlightXSecDrawObj.m_GeomChanged = true;
    m_CurrentXSecDrawObj.m_GeomChanged = true;
}

void GeomXSec::LoadDrawObjs( vector< DrawObj* > & draw_obj_vec )
{
    Geom::LoadDrawObjs( draw_obj_vec );


    if ( m_Vehicle->IsGeomActive( m_ID ) && m_GuiDraw.GetDisplayType() == GeomGuiDraw::DISPLAY_BEZIER )
    {
        char str[256];

        for ( int i = 0 ; i < m_XSecDrawObj_vec.size() ; i++ )
        {
            sprintf( str, "_%d", i );

            m_XSecDrawObj_vec[i].m_Screen = DrawObj::VSP_MAIN_SCREEN;
            m_XSecDrawObj_vec[i].m_GeomID = XSECHEADER + m_ID + str;
            m_XSecDrawObj_vec[i].m_LineWidth = 2.0;
            m_XSecDrawObj_vec[i].m_LineColor = vec3d( 0.0, 0.0, 0.0 );
            m_XSecDrawObj_vec[i].m_Type = DrawObj::VSP_LINE_STRIP;
            m_XSecDrawObj_vec[i].m_Visible = !m_GuiDraw.GetNoShowFlag();
            draw_obj_vec.push_back( &m_XSecDrawObj_vec[i] );
        }

        m_HighlightXSecDrawObj.m_Screen = DrawObj::VSP_MAIN_SCREEN;
        m_HighlightXSecDrawObj.m_GeomID = XSECHEADER + m_ID + "ACTIVE";
        m_HighlightXSecDrawObj.m_LineWidth = 4.0;
        m_HighlightXSecDrawObj.m_LineColor = vec3d( 1.0, 0.0, 0.0 );
        m_HighlightXSecDrawObj.m_Type = DrawObj::VSP_LINE_STRIP;
        m_HighlightXSecDrawObj.m_Visible = !m_GuiDraw.GetNoShowFlag();
        draw_obj_vec.push_back( &m_HighlightXSecDrawObj );

        m_CurrentXSecDrawObj.m_Screen = DrawObj::VSP_XSEC_SCREEN;
        m_CurrentXSecDrawObj.m_GeomID = XSECHEADER + m_ID + "CURRENT";
        draw_obj_vec.push_back( &m_CurrentXSecDrawObj );
    }
}

//==== Set Index For Active XSec ====//
void GeomXSec::SetActiveXSecIndex( int index )
{
    index = Clamp<int>( index, m_MinActiveXSec, m_XSecSurf.NumXSec() - 1 );

    m_ActiveXSec = index;
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
                sprintf( str, "Def_PS_%d", ixsec );
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
                    sprintf( str, "Def_LS_%d_%d", ixsec, i );
                    lsource->SetName( str );
                    lsource->m_Len = 0.1 * base_len;
                    lsource->m_Len2 = 0.1 * base_len;
                    lsource->m_Rad = 0.2 * r;
                    lsource->m_Rad2 = 0.2 * r;
                    lsource->m_ULoc1 = u;
                    lsource->m_WLoc1 = w0;
                    lsource->m_ULoc2 = u;
                    lsource->m_WLoc2 = w1;
                    AddCfdMeshSource( lsource );
                }
            }
        }
    }
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
