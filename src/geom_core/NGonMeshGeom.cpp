//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//
//
//////////////////////////////////////////////////////////////////////

#include "NGonMeshGeom.h"
#include "Vehicle.h"

//==== Constructor ====//
NGonMeshGeom::NGonMeshGeom( Vehicle* vehicle_ptr ) : Geom( vehicle_ptr )
{
    m_Name = "NGonMeshGeom";
    m_Type.m_Name = "NGonMesh";
    m_Type.m_Type = NGON_GEOM_TYPE;

    // Disable Parameters that don't make sense for NGonMeshGeom
    m_SymPlanFlag.Deactivate();
    m_SymAxFlag.Deactivate();
    m_SymRotN.Deactivate();
    m_Density.Deactivate();
    m_ShellFlag.Deactivate();
    m_MassArea.Deactivate();
    m_MassPrior.Deactivate();

    m_ScaleMatrix.loadIdentity();
    m_ScaleFromOrig.Init( "Scale_From_Original", "XForm", this, 1, 1.0e-5, 1.0e12 );

    Update();
}

//==== Destructor ====//
NGonMeshGeom::~NGonMeshGeom()
{
}

void NGonMeshGeom::UpdateSurf()
{
}

void NGonMeshGeom::UpdateDrawObj()
{

}

void NGonMeshGeom::LoadDrawObjs(vector< DrawObj* > & draw_obj_vec)
{

}

//==== Get Total Transformation Matrix from Original Points ====//
Matrix4d NGonMeshGeom::GetTotalTransMat() const
{
    Matrix4d retMat;
    retMat.initMat( m_ScaleMatrix );
    retMat.postMult( m_ModelMatrix );

    return retMat;
}

void NGonMeshGeom::Scale()
{
    double currentScale = m_Scale() / m_LastScale();
    m_ScaleFromOrig *= currentScale;
    m_ScaleMatrix.loadIdentity();
    m_ScaleMatrix.scale( m_ScaleFromOrig() );
    m_LastScale = m_Scale();
}

void NGonMeshGeom::UpdateBBox()
{
    int i;
    m_BBox.Reset();
    Matrix4d transMat = GetTotalTransMat();

}

//==== Encode XML ====//
xmlNodePtr NGonMeshGeom::EncodeXml( xmlNodePtr & node )
{
    Geom::EncodeXml( node );
    xmlNodePtr ngon_node = xmlNewChild( node, NULL, BAD_CAST "NGonMeshGeom", NULL );

    return ngon_node;
}

//==== Decode XML ====//
xmlNodePtr NGonMeshGeom::DecodeXml( xmlNodePtr & node )
{
    Geom::DecodeXml( node );

    xmlNodePtr ngon_node = XmlUtil::GetNode( node, "NGonMeshGeom", 0 );
    if ( ngon_node )
    {

    }

    return ngon_node;
}
