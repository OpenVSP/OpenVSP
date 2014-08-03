//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//
//
//////////////////////////////////////////////////////////////////////

#include "Geom.h"
#include "Vehicle.h"
#include "StlHelper.h"
#include "StringUtil.h"
#include "ParmMgr.h"
#include "SubSurfaceMgr.h"
#include "APIDefines.h"
using namespace vsp;

#include <time.h>
#include <stdlib.h>

//==== Constructor ====//
GeomType::GeomType()
{
    m_Type = POD_GEOM_TYPE;
    m_Name = "Pod";
    m_FixedFlag = false;

}

//==== Constructor ====//
GeomType::GeomType( int id, string name, bool fixed_flag, string module_name )
{
    m_Type = id;
    m_Name = name;
    m_FixedFlag = fixed_flag;
    m_ModuleName = module_name;
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
    m_GeomID = t.m_GeomID;
}




//===============================================================================//
//===============================================================================//
//===============================================================================//

//==== Constructor ====//
GeomGuiDraw::GeomGuiDraw()
{
    m_DrawType = GEOM_DRAW_WIRE;
    m_NoShowFlag = false;
    m_DisplayChildrenFlag = true;
    m_MaterialID = 0;
    m_DispSubSurfFlag = true;

}

//==== Destructor ====//
GeomGuiDraw::~GeomGuiDraw()
{

}

void GeomGuiDraw::SetMaterialToDefault()
{
    m_MaterialMgr.SetMaterial( m_MaterialMgr.getDefault() );
}

void GeomGuiDraw::SetMaterial( std::string name, double ambi[], double diff[], double spec[], double emis[], double shin )
{
    m_MaterialMgr.SetMaterial( name, ambi, diff, spec, emis, shin );
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

    if ( check_display_flag && m_GuiDraw.GetDisplayChildrenFlag() == false )
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
        m_Type.m_Name   = XmlUtil::FindString( child_node, "TypeName", m_Type.m_Name );
        m_Type.m_Type   = XmlUtil::FindInt( child_node, "TypeID", m_Type.m_Type );
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
    m_AbsRelFlag.Init( "Abs_Or_Relitive_flag", "XForm", this, RELATIVE_XFORM, ABSOLUTE_XFORM, RELATIVE_XFORM, false );
    m_TransAttachFlag.Init( "Trans_Attach_Flag", "Attach", this, ATTACH_TRANS_NONE, ATTACH_TRANS_NONE, ATTACH_TRANS_UV, false );
    m_TransAttachFlag.SetDescript( "Determines relative translation coordinate system" );
    m_RotAttachFlag.Init( "Rots_Attach_Flag", "Attach", this, ATTACH_ROT_NONE, ATTACH_ROT_NONE, ATTACH_ROT_UV, false );
    m_RotAttachFlag.SetDescript( "Determines relative rotation axes" );
    m_ULoc.Init( "U_Attach_Location", "Attach", this, 1e-6, 1e-6, 1 - 1e-6 );
    m_ULoc.SetDescript( "U Location of Parent's Surface" );
    m_WLoc.Init( "V_Attach_Location", "Attach", this, 1e-6, 1e-6, 1 - 1e-6 );
    m_WLoc.SetDescript( "V Location of Parent's Surface" );
    m_relFlag.Init( "Abs_Rel_Flag", "Attach", this, true, 0, 1 );

    m_Scale.Init( "Scale", "XForm", this, 1, 1.0e-5, 1.0e12 );
    m_Scale.SetDescript( "Scale Geometry Size" );

    m_ignoreAbsFlag = false;

    m_LastScale = m_Scale();

    m_ModelMatrix.loadIdentity();
}

//==== Destructor ====//
GeomXForm::~GeomXForm()
{
}

//==== Update ====//
void GeomXForm::Update()
{
    ComposeModelMatrix();
}

//==== Update XForm ====//
void GeomXForm::UpdateXForm()
{
    ComposeModelMatrix();
}

//==== Compose Model Matrix =====//
void GeomXForm::ComposeModelMatrix()
{
    m_ModelMatrix.loadIdentity();
    ComputeCenter();

    // Get Attament Matrix
    Matrix4d attachedMat = ComposeAttachMatrix();

    if (  m_AbsRelFlag() ==  RELATIVE_XFORM || m_ignoreAbsFlag )
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
    // If both attachment flags set to none, return identity
    if ( m_TransAttachFlag() == ATTACH_TRANS_NONE && m_RotAttachFlag() == ATTACH_ROT_NONE )
    {
        return attachedMat;
    }

    Geom* parent = m_Vehicle->FindGeom( GetParentID() );
    if ( parent )
    {
        Matrix4d transMat;
        Matrix4d rotMat;

        Matrix4d parentMat;
        parentMat = parent->getModelMatrix();
        double tempMat[16];
        parentMat.getMat( tempMat );
        VspSurf* surf_ptr =  parent->GetSurfPtr();

        bool revertCompTrans = false;
        bool revertCompRot = false;

        if ( m_TransAttachFlag() == ATTACH_TRANS_UV )
        {
            if ( surf_ptr )
            {
                transMat = surf_ptr->CompTransCoordSys( m_ULoc(), m_WLoc() );
            }
            else
            {
                revertCompTrans = true;
            }
        }

        if ( m_RotAttachFlag() == ATTACH_ROT_UV )
        {
            if ( surf_ptr )
            {
                rotMat = surf_ptr->CompRotCoordSys( m_ULoc(), m_WLoc() );
            }
            else
            {
                revertCompRot = true;
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
    m_Name = "Geom";
    m_Type.m_Type = GEOM_GEOM_TYPE;
    m_Type.m_Name = m_Name;

    m_TessU.Init( "Tess_U", "Shape", this, 8, 2,  1000 );
    m_TessU.SetDescript( "Number of tessellated curves in the U direction" );
    m_TessW.Init( "Tess_W", "Shape", this, 9, 2,  1000 );
    m_TessW.SetDescript( "Number of tessellated curves in the W direction" );
    m_TessW.SetMultShift( 4, 1 );

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

    m_SymPlanFlag.Init( "Sym_Planar_Flag", "Sym", this, 0, 0, SYM_XY | SYM_XZ | SYM_YZ, true );
    m_SymAxFlag.Init( "Sym_Axial_Flag", "Sym", this, 0, 0, SYM_ROT_Z, true );
    m_SymRotN.Init( "Sym_Rot_N", "Sym", this, 2, 2, 1000 );

    // Mass Properties
    m_Density.Init( "Density", "Mass_Props", this, 1, 1e-12, 1e12 );
    m_MassArea.Init( "Mass_Area", "Mass_Props", this, 1, 1e-12, 1e12 );
    m_MassPrior.Init( "Mass_Prior", "Mass_Props", this, 0, 0, 1e12 );
    m_ShellFlag.Init( "Shell_Flag", "Mass_Props", this, false, 0, 1 );

    // Geom needs at least one surf
    m_MainSurfVec.push_back( VspSurf() );

    currSourceID = 0;

    m_WakeActiveFlag = false;

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

    while ( set_name_vec.size() < m_SetFlags.size() )
    {
        m_SetFlags.push_back( false );
    }

    if ( m_SetFlags.size() < SET_FIRST_USER )
    {
        return;
    }

    m_SetFlags[ SET_ALL ] = true;   // All

    if ( m_GuiDraw.GetNoShowFlag() == false )
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
void Geom::Update()
{
    m_LateUpdateFlag = false;

    Scale();
    GeomXForm::Update();

    UpdateSurf();       // Must be implemented by subclass.
    UpdateSymmAttach();

    for ( int i = 0 ; i < ( int )m_SubSurfVec.size() ; i++ )
    {
        m_SubSurfVec[i]->Update();
    }

    UpdateChildren();
    UpdateBBox();
    UpdateDrawObj();

    m_UpdatedParmVec.clear();
}

void Geom::UpdateTesselate( int indx, vector< vector< vec3d > > &pnts, vector< vector< vec3d > > &norms,
                            vector< vector< vec3d > > &uw_pnts )
{
    m_SurfVec[indx].Tesselate( m_TessU(), m_TessW(), pnts, norms, uw_pnts );
}

void Geom::UpdateTesselate( int indx, vector< vector< vec3d > > &pnts, vector< vector< vec3d > > &norms )
{
    vector< vector< vec3d > > uw_pnts;
    UpdateTesselate( indx, pnts, norms, uw_pnts );
}

void Geom::UpdateSymmAttach()
{
    int num_surf = GetNumTotalSurfs();
    m_SurfVec.clear();
    m_SurfVec.resize( num_surf, VspSurf() );
    m_SurfIndxVec.resize( num_surf, -1 );

    int num_main = GetNumMainSurfs();
    for ( int i = 0 ; i < ( int )num_main ; i++ )
    {
        m_SurfVec[i] = m_MainSurfVec[i];
        m_SurfIndxVec[i] = i;
    }

    vector<Matrix4d> transMats;
    transMats.resize( num_surf, Matrix4d() );
    // Compute Relative Translation Matrix
    Matrix4d attachMat;
    Matrix4d relTrans;
    attachMat = ComposeAttachMatrix();
    relTrans = attachMat;
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
        transMats[i].postMult( attachMat.data() );
        m_SurfVec[i].Transform( transMats[i] ); // Apply total transformation to main surfaces
    }
}

//==== Check If Children Exist and Update ====//
void Geom::UpdateChildren()
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
            child->Update();
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

void Geom::UpdateDrawObj()
{
    m_WireShadeDrawObj_vec.resize( m_SurfVec.size(), DrawObj() );

    //==== Tesselate Surface ====//
    for ( int i = 0 ; i < ( int )m_SurfVec.size() ; i++ )
    {
        UpdateTesselate( i, m_WireShadeDrawObj_vec[i].m_PntMesh, m_WireShadeDrawObj_vec[i].m_NormMesh );
        m_WireShadeDrawObj_vec[i].m_GeomChanged = true;
        m_WireShadeDrawObj_vec[i].m_FlipNormals = m_SurfVec[i].GetFlipNormal();
    }

    //==== Bounding Box ====//
    m_HighlightDrawObj.m_PntVec = m_BBox.GetBBoxDrawLines();
}

//==== Encode Data Into XML Data Struct ====//
xmlNodePtr Geom::EncodeXml( xmlNodePtr & node )
{
    GeomXForm::EncodeXml( node );

    // Encode Material Info.
    m_GuiDraw.getMaterialMgr()->EncodeXml( node );

    // Encode Color Info.
    m_GuiDraw.getColorMgr()->EncodeXml( node );

    // Encode Texture Info.
    m_GuiDraw.getTextureMgr()->EncodeXml( node );

    xmlNodePtr geom_node = xmlNewChild( node, NULL, BAD_CAST "Geom", NULL );
    if ( geom_node )
    {
        XmlUtil::AddVectorBoolNode( geom_node, "Set_List", m_SetFlags );

        for( int i = 0; i < ( int )sourceVec.size(); i++ )
        {
            sourceVec[i]->EncodeXml( geom_node );
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
    }
    return geom_node;

}

//==== Decode Data Into XML Data Struct ====//
xmlNodePtr Geom::DecodeXml( xmlNodePtr & node )
{
    GeomXForm::DecodeXml( node );

    // Decode Material Info.
    m_GuiDraw.getMaterialMgr()->DecodeXml( node );

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
                int type = XmlUtil::FindInt( src_node, "Type", BaseSource::POINT_SOURCE );

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
                        int type = XmlUtil::FindInt( ss_info_node, "Type", SubSurface::SS_LINE );
                        SubSurface* ssurf = AddSubSurf( type );
                        if ( ssurf )
                        {
                            ssurf->DecodeXml( ss_node );
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

void Geom::ResetGeomChangedFlag()
{
    for ( int i = 0 ; i < ( int )m_WireShadeDrawObj_vec.size() ; i++ )
    {
        m_WireShadeDrawObj_vec[i].m_GeomChanged = false;
    }
}

//==== Load All Draw Objects ====//
void Geom::LoadDrawObjs( vector< DrawObj* > & draw_obj_vec )
{
    char str[256];

    for ( int i = 0 ; i < ( int )m_WireShadeDrawObj_vec.size() ; i++ )
    {
        // Symmetry drawObjs have same m_ID. Make them unique by adding index
        // at the end of m_ID.
        sprintf( str, "_%d", i );
        m_WireShadeDrawObj_vec[i].m_GeomID = m_ID + str;
        m_WireShadeDrawObj_vec[i].m_Visible = !m_GuiDraw.GetNoShowFlag();

        // Set Render Destination to Main VSP Window.
        m_WireShadeDrawObj_vec[i].m_Screen = DrawObj::VSP_MAIN_SCREEN;

        Material * material = m_GuiDraw.getMaterialMgr()->getMaterial();

        m_WireShadeDrawObj_vec[i].m_MaterialInfo.Ambient[0] = (float)material->m_AmbientR.Get();
        m_WireShadeDrawObj_vec[i].m_MaterialInfo.Ambient[1] = (float)material->m_AmbientG.Get();
        m_WireShadeDrawObj_vec[i].m_MaterialInfo.Ambient[2] = (float)material->m_AmbientB.Get();
        m_WireShadeDrawObj_vec[i].m_MaterialInfo.Ambient[3] = (float)material->m_AmbientA.Get();

        m_WireShadeDrawObj_vec[i].m_MaterialInfo.Diffuse[0] = (float)material->m_DiffuseR.Get();
        m_WireShadeDrawObj_vec[i].m_MaterialInfo.Diffuse[1] = (float)material->m_DiffuseG.Get();
        m_WireShadeDrawObj_vec[i].m_MaterialInfo.Diffuse[2] = (float)material->m_DiffuseB.Get();
        m_WireShadeDrawObj_vec[i].m_MaterialInfo.Diffuse[3] = (float)material->m_DiffuseA.Get();

        m_WireShadeDrawObj_vec[i].m_MaterialInfo.Specular[0] = (float)material->m_SpecularR.Get();
        m_WireShadeDrawObj_vec[i].m_MaterialInfo.Specular[1] = (float)material->m_SpecularG.Get();
        m_WireShadeDrawObj_vec[i].m_MaterialInfo.Specular[2] = (float)material->m_SpecularB.Get();
        m_WireShadeDrawObj_vec[i].m_MaterialInfo.Specular[3] = (float)material->m_SpecularA.Get();

        m_WireShadeDrawObj_vec[i].m_MaterialInfo.Emission[0] = (float)material->m_EmissionR.Get();
        m_WireShadeDrawObj_vec[i].m_MaterialInfo.Emission[1] = (float)material->m_EmissionG.Get();
        m_WireShadeDrawObj_vec[i].m_MaterialInfo.Emission[2] = (float)material->m_EmissionB.Get();
        m_WireShadeDrawObj_vec[i].m_MaterialInfo.Emission[3] = (float)material->m_EmissionA.Get();

        m_WireShadeDrawObj_vec[i].m_MaterialInfo.Shininess = (float)material->m_Shininess.Get();

        vec3d lineColor = vec3d( m_GuiDraw.GetWireColor().x() / 255.0,
            m_GuiDraw.GetWireColor().y() / 255.0,
            m_GuiDraw.GetWireColor().z() / 255.0 );

        switch( m_GuiDraw.GetDrawType() )
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

        case GeomGuiDraw::GEOM_DRAW_TEXTURE:
            m_WireShadeDrawObj_vec[i].m_Type = DrawObj::VSP_TEXTURED_MESH;

            // Reload texture infos.
            m_WireShadeDrawObj_vec[i].m_TextureInfos.clear();
            vector<Texture*> texList = m_GuiDraw.getTextureMgr()->FindTextureVec( m_GuiDraw.getTextureMgr()->GetTextureVec() );
            for( int j = 0; j < ( int )texList.size(); j++ )
            {
                DrawObj::TextureInfo info;
                info.FileName = texList[j]->m_FileName;
                info.UScale = ( float )texList[j]->m_UScale.Get();
                info.WScale = ( float )texList[j]->m_WScale.Get();
                info.U = ( float )texList[j]->m_U.Get();
                info.W = ( float )texList[j]->m_W.Get();
                info.Transparency = ( float )texList[j]->m_Transparency.Get();
                info.UFlip = texList[j]->m_FlipU.Get();
                info.WFlip = texList[j]->m_FlipW.Get();
                info.ID = texList[j]->GetID();
                m_WireShadeDrawObj_vec[i].m_TextureInfos.push_back( info );
            }
            draw_obj_vec.push_back( &m_WireShadeDrawObj_vec[i] );
            break;
        }
    }
    if ( m_Vehicle->IsGeomActive( m_ID ) )
    {
        m_HighlightDrawObj.m_Screen = DrawObj::VSP_MAIN_SCREEN;
        m_HighlightDrawObj.m_GeomID = BBOXHEADER + m_ID;
        m_HighlightDrawObj.m_LineWidth = 2.0;
        m_HighlightDrawObj.m_LineColor = vec3d( 1.0, 0., 0.0 );
        m_HighlightDrawObj.m_Type = DrawObj::VSP_LINES;
        draw_obj_vec.push_back( &m_HighlightDrawObj );
    }

    // Load Subsurfaces
    RecolorSubSurfs( SubSurfaceMgr.GetCurrSurfInd() );
    if ( m_GuiDraw.GetDispSubSurfFlag() && !m_GuiDraw.GetNoShowFlag() )
    {
        for ( int i = 0 ; i < ( int )m_SubSurfVec.size() ; i++ )
        {
            m_SubSurfVec[i]->LoadDrawObjs( draw_obj_vec );
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
    return m_GuiDraw.getMaterialMgr()->getMaterial();
}

//==== Create Degenerate Geometry ====//
void Geom::CreateDegenGeom( vector<DegenGeom> &dgs)
{
    vector< vector< vec3d > > pnts;
    vector< vector< vec3d > > nrms;
    vector< vector< vec3d > > uwpnts;

    for ( int i = 0 ; i < ( int )m_SurfVec.size() ; i++ )
    {
        //==== Tesselate Surface ====//
        UpdateTesselate( i, pnts, nrms, uwpnts );

        DegenGeom degenGeom;
        degenGeom.setParentGeom( this );

        degenGeom.setNumXSecs( pnts.size() );
        degenGeom.setNumPnts( pnts[0].size() );
        degenGeom.setName( GetName() );

        degenGeom.createDegenSurface( pnts, uwpnts, m_SurfVec[i].GetFlipNormal() );

        if( m_SurfVec[i].GetSurfType() == VspSurf::WING_SURF )
        {
            degenGeom.setType(DegenGeom::SURFACE_TYPE);

            degenGeom.createSurfDegenPlate( pnts, uwpnts );
            degenGeom.createSurfDegenStick( pnts, uwpnts );
        }
        else
        {
            degenGeom.setType(DegenGeom::BODY_TYPE);

            degenGeom.createBodyDegenPlate( pnts, uwpnts );
            degenGeom.createBodyDegenStick( pnts, uwpnts );
        }

        dgs.push_back(degenGeom);
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
    int symFlag = GetSymFlag();
    int numSymSurfs = GetNumMainSurfs();
    int counter = 0;
    for ( int i = 0; i < SYM_NUM_TYPES ; i++ )
    {
        if ( symFlag & ( 1 << i ) )
        {
            if ( i < SYM_PLANAR_TYPES )
            {
                numSymSurfs *= 2;
            }
            else
            {
                numSymSurfs += ( numSymSurfs ) * ( m_SymRotN() - 1 );
            }
            counter++;
        }
    }
    return numSymSurfs;
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

vec3d Geom::GetUWPt( const double &u, const double &w )
{
    return GetSurfPtr()->CompPnt01( u, w );
}

vec3d Geom::GetUWPt( const int &indx, const double &u, const double &w )
{
    return GetSurfPtr( indx )->CompPnt01( u, w );
}

void Geom::WriteXSecFile( int geom_no, FILE* dump_file )
{
    for ( int i = 0 ; i < ( int )m_SurfVec.size() ; i++ )
    {
        //==== Tessellate Surface ====//
        vector< vector< vec3d > > pnts;
        vector< vector< vec3d > > norms;

        UpdateTesselate( i, pnts, norms );

        //==== Write XSec Header ====//
        fprintf( dump_file, "\n" );
        fprintf( dump_file, "%s \n", ( char* ) m_Name.c_str() );
        fprintf( dump_file, " GROUP NUMBER      = %d \n", geom_no );
        fprintf( dump_file, " TYPE              = 1  \n" );         // 1 -- Non Lifting, 0 -- Lifting
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

void Geom::CreateGeomResults( Results* res )
{
    res->Add( ResData( "Type", vsp::GEOM_XSECS ) );
    res->Add( ResData( "Num_Surfs", ( int )m_SurfVec.size() ) );

    for ( int i = 0 ; i < ( int )m_SurfVec.size() ; i++ )
    {
        //==== Tessellate Surface ====//
        vector< vector< vec3d > > pnts;
        vector< vector< vec3d > > norms;
        UpdateTesselate( i, pnts, norms );

        res->Add( ResData( "Num_XSecs", static_cast<int>( pnts.size() ) ) );

        if ( pnts.size() )
        {
            res->Add( ResData( "Num_Pnts_Per_XSec", static_cast<int>( pnts[0].size() ) ) );
        }

        //==== Write XSec Data ====//
        for ( int j = 0 ; j < ( int )pnts.size() ; j++ )
        {
            vector< vec3d > xsec_vec;
            for ( int k = 0 ; k < ( int )pnts[j].size() ; k++ )
            {
                xsec_vec.push_back(  pnts[j][k] );
            }
            res->Add( ResData( "XSec_Pnts", xsec_vec ) );
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
        UpdateTesselate( i, pnts, norms );
        int num_xsecs = pnts.size();
        int num_pnts = pnts[0].size();
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

                sprintf( numstr, "%d %d %d %d -1 ", i0, i1, i2, i3 );
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

        UpdateTesselate( i, pnts, norms );

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

    for ( int i = 0 ; i < ( int )m_SurfVec.size() ; i++ )
    {
        UpdateTesselate( i, pnts, norms, uw_pnts );

        TMeshVec.push_back( new TMesh() );
        TMeshVec[i]->LoadGeomAttributes( this );
        TMeshVec[i]->m_SurfNum = i;
        TMeshVec[i]->m_UWPnts = uw_pnts;
        TMeshVec[i]->m_XYZPnts = pnts;
        bool f_norm = m_SurfVec[i].GetFlipNormal();

        vec3d norm;
        vec3d v0, v1, v2, v3;
        vec3d uw0, uw1, uw2, uw3;
        vec3d d21, d01, d03, d23, d20;

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

                d21 = v2 - v1;
                d01 = v0 - v1;
                d20 = v2 - v0;

                if ( d21.mag() > 0.000001 && d01.mag() > 0.000001 && d20.mag() > 0.000001 )
                {
                    norm = cross( d21, d01 );
                    norm.normalize();
                    if ( f_norm )
                    {
                        TMeshVec[i]->AddTri( v0, v2, v1, norm * -1, uw0, uw2, uw1 );
                    }
                    else
                    {
                        TMeshVec[i]->AddTri( v0, v1, v2, norm, uw0, uw1, uw2 );
                    }
                }

                d03 = v0 - v3;
                d23 = v2 - v3;
                if ( d03.mag() > 0.000001 && d23.mag() > 0.000001 && d20.mag() > 0.000001 )
                {
                    norm = cross( d03, d23 );
                    norm.normalize();
                    if ( f_norm )
                    {
                        TMeshVec[i]->AddTri( v0, v3, v2, norm * -1, uw0, uw3, uw2 );
                    }
                    else
                    {
                        TMeshVec[i]->AddTri( v0, v2, v3, norm, uw0, uw2, uw3 );
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

SubSurface* Geom::AddSubSurf( int type )
{
    SubSurface* ssurf = NULL;

    if ( type == SubSurface::SS_LINE )
    {
        ssurf = new SSLine( m_ID );
        ssurf->SetName( string( "SS_LINE_" + to_string( ( long long )m_SubSurfVec.size() ) ) );
    }
    else if ( type == SubSurface::SS_RECTANGLE )
    {
        ssurf = new SSRectangle( m_ID );
        ssurf->SetName( string( "SS_RECT_" + to_string( ( long long )m_SubSurfVec.size() ) ) );
    }
    else if ( type == SubSurface::SS_ELLIPSE )
    {
        ssurf = new SSEllipse( m_ID );
        ssurf->SetName( string( "SS_ELLIP_" + to_string( ( long long )m_SubSurfVec.size() ) ) );
    }

    ssurf->SetParentContainer( GetID() );
    AddSubSurf( ssurf );

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

void Geom::DelAllSources()
{
    for ( int i = 0 ; i < ( int )sourceVec.size() ; i++ )
    {
        delete sourceVec[i];
    }
    sourceVec.clear();
}

void Geom::DelCurrSource()
{
    int id = GetCurrSourceID();

    vector< BaseSource* > tmpVec;
    for ( int i = 0 ; i < ( int )sourceVec.size() ; i++ )
    {
        if ( i != id )
        {
            tmpVec.push_back( sourceVec[i] );
        }
        else
        {
            delete sourceVec[i];
        }
    }
    sourceVec = tmpVec;
}

void Geom::UpdateSources()
{
    int i;
//  if ( getSymCode() != NO_SYM )
    for ( i = 0 ; i < ( int )sourceVec.size() ; i++ )
    {
        sourceVec[i]->Update( this );
    }
}

BaseSource* Geom::CreateSource( int type )
{
    BaseSource* src_ptr = NULL;
    if ( type == BaseSource::POINT_SOURCE )
    {
        src_ptr = new PointSource();
    }
    else if ( type == BaseSource::LINE_SOURCE )
    {
        src_ptr = new LineSource();
    }
    else if ( type == BaseSource::BOX_SOURCE )
    {
        src_ptr = new BoxSource();
    }

    return src_ptr;
}

bool Geom::HasWingTypeSurfs()
{
    for( int i = 0; i < m_MainSurfVec.size(); i++ )
    {
        if( m_MainSurfVec[i].GetSurfType() == VspSurf::WING_SURF )
        {
            return true;
        }
    }
    return false;
}

void Geom::AppendWakeEdges( vector< vector< vec3d > > & edges )
{
    if( m_WakeActiveFlag )
    {
        for( int i = 0; i < m_SurfVec.size(); i++ )
        {
            if( m_SurfVec[i].GetSurfType() == VspSurf::WING_SURF )
            {
                vector< vector< vec3d > > pnts;
                vector< vector< vec3d > > norms;

                m_SurfVec[i].Tesselate( 2, 2, pnts, norms );

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
        m_XSecDrawObj_vec[i].m_PntVec = m_XSecSurf.FindXSec( i )->GetDrawLines( m_TessW(), relTrans );
        m_XSecDrawObj_vec[i].m_GeomChanged = true;
    }

    m_HighlightXSecDrawObj.m_PntVec = m_XSecSurf.FindXSec( m_ActiveXSec )->GetDrawLines( m_TessW(), relTrans );
    m_HighlightXSecDrawObj.m_GeomChanged = true;

    double w = m_XSecSurf.FindXSec( m_ActiveXSec )->GetXSecCurve()->GetWidth();
    double h = m_XSecSurf.FindXSec( m_ActiveXSec )->GetXSecCurve()->GetHeight();
    double scale = 1.0;
    if( w > h ) scale = 1.0 / w;
    else scale = 1.0 / h;

    Matrix4d mat;
    m_XSecSurf.GetBasicTransformation( Z_DIR, X_DIR, XS_SHIFT_MID, false, w * scale, mat );
    mat.scale( scale );

    VspCurve crv = m_XSecSurf.FindXSec( m_ActiveXSec )->GetUntransformedCurve();
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
        crv.Tesselate( m_TessW(), pts );
        m_CurrentXSecDrawObj.m_PntVec = pts;
        m_CurrentXSecDrawObj.m_LineWidth = 1.0;
        m_CurrentXSecDrawObj.m_LineColor = vec3d( 0.0, 0.0, 0.0 );
        m_CurrentXSecDrawObj.m_Type = DrawObj::VSP_LINES;
    }
    m_CurrentXSecDrawObj.m_GeomChanged = true;
}

void GeomXSec::LoadDrawObjs( vector< DrawObj* > & draw_obj_vec )
{
    Geom::LoadDrawObjs( draw_obj_vec );


    if ( m_Vehicle->IsGeomActive( m_ID ) )
    {
        char str[256];

        int nxsec = m_XSecSurf.NumXSec();
        for ( int i = 0 ; i < nxsec ; i++ )
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
