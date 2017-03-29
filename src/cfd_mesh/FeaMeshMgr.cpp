//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//

// FeaMeshMgr.cpp
//
//////////////////////////////////////////////////////////////////////

#include "FeaMeshMgr.h"
#include "Util.h"
#include "APIDefines.h"
#include "SubSurfaceMgr.h"
#include "StructureMgr.h"

//=============================================================//
//=============================================================//

//FeaGeom::FeaGeom()
//{
//    m_GeomSurfPtr = NULL;
//}
//
//FeaGeom::~FeaGeom()
//{
//}
//
//
//void FeaGeom::BuildClean()
//{
//    if ( m_GeomSurfPtr )
//    {
//        m_GeomSurfPtr->BuildClean();
//    }
//}

//void WingSection::WriteData( xmlNodePtr root )
//{
//    int i;
//
//    //===== Ribs =====//
//    xmlNodePtr rib_list_node = xmlNewChild( root, NULL, ( const xmlChar * )"Rib_List", NULL );
//    for ( i = 0 ; i < ( int )m_RibVec.size() ; i++ )
//    {
//        xmlNodePtr rib_node = xmlNewChild( rib_list_node, NULL, ( const xmlChar * )"Rib", NULL );
//        XmlUtil::AddIntNode( rib_node, "SectionID",      m_RibVec[i]->m_SectID );
//        XmlUtil::AddDoubleNode( rib_node, "PerSpan",     m_RibVec[i]->m_PerSpan() );
//        XmlUtil::AddDoubleNode( rib_node, "Sweep",     m_RibVec[i]->m_Sweep() );
//        XmlUtil::AddIntNode( rib_node, "AbsSweepFlag",   m_RibVec[i]->m_AbsSweepFlag() );
//        XmlUtil::AddIntNode( rib_node, "TrimFlag",   m_RibVec[i]->m_TrimFlag() );
//        XmlUtil::AddDoubleNode( rib_node, "Thick",     m_RibVec[i]->m_Thick() );
//        XmlUtil::AddDoubleNode( rib_node, "Density",     m_RibVec[i]->m_Density() );
//    }
//
//    //===== Spars =====//
//    xmlNodePtr spar_list_node = xmlNewChild( root, NULL, ( const xmlChar * )"Spar_List", NULL );
//    for ( i = 0 ; i < ( int )m_SparVec.size() ; i++ )
//    {
//        xmlNodePtr spar_node = xmlNewChild( spar_list_node, NULL, ( const xmlChar * )"Spar", NULL );
//        XmlUtil::AddIntNode( spar_node, "SectionID",      m_SparVec[i]->m_SectID );
//        XmlUtil::AddDoubleNode( spar_node, "PerChord",     m_SparVec[i]->m_PerChord() );
//        XmlUtil::AddDoubleNode( spar_node, "Sweep",     m_SparVec[i]->m_Sweep() );
//        XmlUtil::AddIntNode( spar_node, "AbsSweepFlag",   m_SparVec[i]->m_AbsSweepFlag() );
//        XmlUtil::AddIntNode( spar_node, "TrimFlag",   m_SparVec[i]->m_TrimFlag() );
//        XmlUtil::AddDoubleNode( spar_node, "Thick",     m_SparVec[i]->m_Thick() );
//        XmlUtil::AddDoubleNode( spar_node, "Density",     m_SparVec[i]->m_Density() );
//    }
//
//    //===== Skins =====//
//    xmlNodePtr upper_skin_node = xmlNewChild( root, NULL, ( const xmlChar * )"Upper_Skin", NULL );
//    m_UpperSkin.WriteData( upper_skin_node );
//
//    xmlNodePtr lower_skin_node = xmlNewChild( root, NULL, ( const xmlChar * )"Lower_Skin", NULL );
//    m_LowerSkin.WriteData( lower_skin_node );
//
//
//}
//
//void WingSection::ReadData( xmlNodePtr root )
//{
//    //===== Ribs =====//
//    for ( int i = 0 ; i < ( int )m_RibVec.size() ; i++ )
//    {
//        delete m_RibVec[i];
//    }
//    m_RibVec.clear();
//
//    xmlNodePtr rib_list_node = XmlUtil::GetNode( root, "Rib_List", 0 );
//    if ( rib_list_node )
//    {
//        int num_ribs =  XmlUtil::GetNumNames( rib_list_node, "Rib" );
//        for ( int i = 0 ; i < num_ribs ; i++ )
//        {
//            xmlNodePtr rib_node = XmlUtil::GetNode( rib_list_node, "Rib", i );
//            if ( rib_node )
//            {
//                FeaRib* rib = new FeaRib();
//                rib->m_SectID = XmlUtil::FindInt( rib_node, "SectionID", 0 );
//                rib->m_PerSpan = XmlUtil::FindDouble( rib_node, "PerSpan", 0.5 );
//                rib->m_Sweep   = XmlUtil::FindDouble( rib_node, "Sweep", 0.0 );
//                rib->m_AbsSweepFlag = ( XmlUtil::FindInt( rib_node, "AbsSweepFlag", 1 ) != 0 ); // Bool
//                rib->m_TrimFlag     = ( XmlUtil::FindInt( rib_node, "TrimFlag", 1 ) != 0 );
//                rib->m_Thick   = XmlUtil::FindDouble( rib_node, "Thick", 0.1 );
//                rib->m_Density = XmlUtil::FindDouble( rib_node, "Density", 1.0 );
//                m_RibVec.push_back( rib );
//            }
//        }
//    }
//
//    //===== Spars =====//
//    for ( int i = 0 ; i < ( int )m_SparVec.size() ; i++ )
//    {
//        delete m_SparVec[i];
//    }
//    m_SparVec.clear();
//
//    xmlNodePtr spar_list_node = XmlUtil::GetNode( root, "Spar_List", 0 );
//    if ( spar_list_node )
//    {
//        int num_spars =  XmlUtil::GetNumNames( spar_list_node, "Spar" );
//        for ( int i = 0 ; i < num_spars ; i++ )
//        {
//            xmlNodePtr spar_node = XmlUtil::GetNode( spar_list_node, "Spar", i );
//            if ( spar_node )
//            {
//                FeaSpar* spar = new FeaSpar();
//                spar->m_SectID = XmlUtil::FindInt( spar_node, "SectionID", 0 );
//                spar->m_PerChord = XmlUtil::FindDouble( spar_node, "PerChord", 0.5 );
//                spar->m_Sweep   = XmlUtil::FindDouble( spar_node, "Sweep", 0.0 );
//                spar->m_AbsSweepFlag = ( XmlUtil::FindInt( spar_node, "AbsSweepFlag", 1 ) != 0 );
//                spar->m_TrimFlag     = ( XmlUtil::FindInt( spar_node, "TrimFlag", 1 ) != 0 );
//                spar->m_Thick   = XmlUtil::FindDouble( spar_node, "Thick", 0.1 );
//                spar->m_Density = XmlUtil::FindDouble( spar_node, "Density", 1.0 );
//                m_SparVec.push_back( spar );
//            }
//        }
//    }
//
//    //===== Skins =====//
//    xmlNodePtr upper_skin_node = XmlUtil::GetNode( root, "Upper_Skin", 0 );
//    if ( upper_skin_node )
//    {
//        m_UpperSkin.ReadData( upper_skin_node );
//    }
//
//    xmlNodePtr lower_skin_node = XmlUtil::GetNode( root, "Lower_Skin", 0 );
//    if ( lower_skin_node )
//    {
//        m_LowerSkin.ReadData( lower_skin_node );
//    }
//
//}
//
//
//void FeaGeom::Load( Surf* surf )
//{
//    // TODO: Fix this function. Currently all points are the same in m_Edge
//
//    int i, e;
//    vec2d uw;
//    vec3d pnt;
//
//    m_GeomSurfPtr = surf;
//
//    //m_TotalSkin.SetSurf( surf );
//    //m_TotalSkin.SetWingSection( this );
//
//    double minu = surf->GetSurfCore()->GetMinU();
//    double minw = surf->GetSurfCore()->GetMinW();
//    double maxu = surf->GetSurfCore()->GetMaxU();
//    double maxw = surf->GetSurfCore()->GetMaxW();
//
//    //==== Load Corner Pnts ====//
//    m_CornerPnts[UW00] = surf->GetSurfCore()->CompPnt( minu, minw ); // Inner TE
//    m_CornerPnts[UW10] = surf->GetSurfCore()->CompPnt( maxu, minw );  // Outer TE
//    m_CornerPnts[UW01] = surf->GetSurfCore()->CompPnt( minu, maxw ); // Inner LE
//    m_CornerPnts[UW11] = surf->GetSurfCore()->CompPnt( maxu, maxw );  // Outer LE
//
//    //==== Compute Surface Normal =====//
//    //vec3d vchd = m_CornerPnts[UW00] - m_CornerPnts[UW01];
//    //vec3d vle  = m_CornerPnts[UW11] - m_CornerPnts[UW01];
//    //m_Normal = cross( vchd, vle );
//    //m_Normal.normalize();
//
//    //==== Find Normal to Chord Line ====//
//    //m_OutNormal = cross( m_Normal, vchd  );
//    //m_OutNormal.normalize();
//
//    //==== Find Sweep Angle of LE ====//
//    //m_SweepLE = RAD_2_DEG * signed_angle( m_ChordNormal, vle, m_Normal * -1.0 );
//
//    //==== Load UW Pnts ====//
//    int num_pnts = 101;
//    for ( i = 0 ; i < num_pnts ; i++ )
//    {
//        double fract = ( double )i / ( double )( num_pnts - 1 );
//        m_Edges[LEADEDGE].m_UWVec.push_back( vec2d( fract, 1 ) );
//        m_Edges[TRAILEDGE].m_UWVec.push_back( vec2d( fract, 0 ) );
//        m_Edges[INNERSIDE].m_UWVec.push_back( vec2d( 0, fract ) );
//        m_Edges[OUTERSIDE].m_UWVec.push_back( vec2d( 1, fract ) );
//    }
//
//    //==== Load 3D Pnts ====//
//    for ( e = 0 ; e < NUM_EDGES ; e++ )
//    {
//        for ( i = 0 ; i < num_pnts ; i++ )
//        {
//            uw  = m_Edges[e].m_UWVec[i];
//            pnt = surf->GetSurfCore()->CompPnt01( uw[0], uw[1] );
//            m_Edges[e].m_PntVec.push_back( pnt );
//        }
//    }
//
//    //==== Load Line Proj Dist Fract ====//
//    for ( e = 0 ; e < NUM_EDGES ; e++ )
//    {
//        vec3d lp0 = m_Edges[e].m_PntVec[0];
//        vec3d lp1 = m_Edges[e].m_PntVec.back();
//        double line_dist = dist( lp0, lp1 );
//        for ( i = 0 ; i < num_pnts ; i++ )
//        {
//            vec3d proj_pnt = proj_pnt_on_line( lp0, lp1, m_Edges[e].m_PntVec[i] );
//            double ldf = dist( proj_pnt, lp0 );
//            if ( line_dist > DBL_EPSILON )
//            {
//                ldf = ldf / line_dist;
//            }
//
//            m_Edges[e].m_LineFractVec.push_back( ldf );
//        }
//    }
//}
//
//vec2d FeaGeom::GetUW( int edge_id, double fract )
//{
//    assert( edge_id >= 0 && edge_id < NUM_EDGES );
//
//    SectionEdge* se = &m_Edges[edge_id];
//    assert( se->m_LineFractVec.size() >= 2 );
//
//    if ( fract <= 0.0 )
//    {
//        return se->m_UWVec[0];
//    }
//
//    for ( int i = 0 ; i < ( int )se->m_LineFractVec.size() - 1 ; i++ )
//    {
//        if ( fract >= se->m_LineFractVec[i] && fract <= se->m_LineFractVec[i + 1] )
//        {
//            double denom = se->m_LineFractVec[i + 1] - se->m_LineFractVec[i];
//            double uw_fract = 0.0;
//            if ( denom > DBL_EPSILON )
//            {
//                uw_fract = ( fract - se->m_LineFractVec[i] ) / denom;
//            }
//
//            vec2d uw = se->m_UWVec[i] + ( se->m_UWVec[i + 1] - se->m_UWVec[i] ) * uw_fract;
//            return uw;
//        }
//    }
//    return se->m_UWVec.back();
//}
//
//vec3d FeaGeom::CompPnt( int edge_id, double fract )
//{
//    vec2d uw = GetUW( edge_id, fract );
//    vec3d p = m_GeomSurfPtr->GetSurfCore()->CompPnt01( uw[0], uw[1] );
//    return p;
//}
//
//bool FeaGeom::IntersectPlaneEdge( int edge_id, vec3d & orig, vec3d & norm, vec2d & result )
//{
//    assert( edge_id >= 0 && edge_id < NUM_EDGES );
//
//    SectionEdge* se = &m_Edges[edge_id];
//    assert( se->m_PntVec.size() >= 2 );
//
//    for ( int i = 0 ; i < ( int )se->m_PntVec.size() - 1 ; i++ )
//    {
//        int side1 = plane_half_space( orig, norm, se->m_PntVec[i] );
//        int side2 = plane_half_space( orig, norm, se->m_PntVec[i + 1] );
//
//        if ( side1 != side2 )
//        {
//            double t;
//            vec3d rayvec = se->m_PntVec[i + 1] - se->m_PntVec[i];
//            int valid = plane_ray_intersect( orig, norm, se->m_PntVec[i], rayvec, t );
//            if ( valid )
//            {
//                result = se->m_UWVec[i] + ( se->m_UWVec[i + 1] - se->m_UWVec[i] ) * t;
//                return true;
//            }
//        }
//    }
//    return false;
//}
//
//void FeaGeom::ComputePerUW( vec3d & pnt, double* per_U, double* per_W )
//{
//    // TODO: Confirm per_U and per_W match with per_span and per_chord
//
//    //==== Find Distance to Inside  Chord ====//
//    double d = dist_pnt_2_line( m_CornerPnts[UW01], m_CornerPnts[UW00], pnt );
//    double denom = dist_pnt_2_line( m_CornerPnts[UW01], m_CornerPnts[UW00], m_CornerPnts[UW11] );
//
//    *per_U = 0.0;
//    if ( denom > DBL_EPSILON )
//    {
//        *per_U = d / denom;
//    }
//
//    //==== Find Intersection of Projected Chordline ===//
//    vec3d dir = m_CornerPnts[UW01] - m_CornerPnts[UW00];
//    vec3d poff = pnt + dir;
//
//    double s, t;
//    bool valid = line_line_intersect( pnt, poff, m_CornerPnts[UW01], m_CornerPnts[UW11], &s, &t );
//    vec3d ple = m_CornerPnts[UW01];
//    if ( valid )
//    {
//        ple = m_CornerPnts[UW01] + ( m_CornerPnts[UW11] - m_CornerPnts[UW01] ) * t;
//    }
//
//
//    valid = line_line_intersect( pnt, poff, m_CornerPnts[UW00], m_CornerPnts[UW10], &s, &t );
//    vec3d pte = m_CornerPnts[UW00];
//    if ( valid )
//    {
//        pte = m_CornerPnts[UW00] + ( m_CornerPnts[UW10] - m_CornerPnts[UW00] ) * t;
//    }
//
//    d = dist( ple, pnt );
//    denom = dist( ple, pte );
//    *per_W = 0.0;
//    if ( denom > DBL_EPSILON )
//    {
//        *per_W = d / denom;
//    }
//
//}

//=============================================================//
//=============================================================//

FeaMeshMgrSingleton::FeaMeshMgrSingleton() : CfdMeshMgrSingleton()
{
    m_TotalMass = 0.0;
    m_DrawMeshFlag = false;
    //m_XmlDataNode = NULL;
    //m_DrawAttachPoints = false;
    m_FeaMeshInProgress = false;
}

FeaMeshMgrSingleton::~FeaMeshMgrSingleton()
{
    CleanUp();
    m_OutStream.clear();
}

void FeaMeshMgrSingleton::CleanUp()
{
    //==== Delete Old Elements ====//
    for ( unsigned int i = 0; i < m_FeaElementVec.size(); i++ )
    {
        m_FeaElementVec[i]->DeleteAllNodes();
        delete m_FeaElementVec[i];
    }
    m_FeaElementVec.clear();

    CfdMeshMgrSingleton::CleanUp();
}

//void FeaMeshMgrSingleton::WriteFeaStructData( Geom* geom_ptr, xmlNodePtr root )
//{
//    if ( geom_ptr != m_WingGeom )
//    {
//        return;
//    }
//
//    bool found = false;
//    for ( int i = 0 ; i < ( int )m_DataGeomVec.size() ; i++ )
//    {
//        if ( m_WingGeom == m_DataGeomVec[i] )
//        {
//            found = true;
//        }
//    }
//    if ( !found )
//    {
//        m_DataGeomVec.push_back( m_WingGeom );
//    }
//
//    xmlNodePtr fea_node = xmlNewChild( root, NULL, ( const xmlChar * )"FEA_Structure_Parms", NULL );
//
//    XmlUtil::AddDoubleNode( fea_node, "DefElemSize", GetGridDensityPtr()->m_BaseLen() );
//    XmlUtil::AddDoubleNode( fea_node, "FEA_Mesh_Min_Length", GetGridDensityPtr()->m_MinLen() );
//    XmlUtil::AddDoubleNode( fea_node, "FEA_Mesh_Max_Gap", GetGridDensityPtr()->m_MaxGap() );
//    XmlUtil::AddDoubleNode( fea_node, "FEA_Mesh_Num_Circle_Segments", GetGridDensityPtr()->m_NCircSeg() );
//    XmlUtil::AddDoubleNode( fea_node, "FEA_Mesh_Growth_Ratio", GetGridDensityPtr()->m_GrowRatio() );
//
//    XmlUtil::AddDoubleNode( fea_node, "ThickScale",  m_ThickScale() );
//
//    xmlNodePtr sec_node;
//    xmlNodePtr sec_list_node = xmlNewChild( fea_node, NULL, ( const xmlChar * )"Wing_Section_List", NULL );
//
//    for ( int i = 0 ; i < ( int )m_WingSections.size() ; i++ )
//    {
//        sec_node = xmlNewChild( sec_list_node, NULL, ( const xmlChar * )"Wing_Section", NULL );
//        m_WingSections[i].WriteData( sec_node );
//    }
//
//    //===== Point Masses ====//
//    xmlNodePtr point_mass_list_node = xmlNewChild( fea_node, NULL, ( const xmlChar * )"Point_Mass_List", NULL );
//    for ( int i = 0 ; i < ( int )m_PointMassVec.size() ; i++ )
//    {
//        xmlNodePtr point_mass_node = xmlNewChild( point_mass_list_node, NULL, ( const xmlChar * )"Point_Mass", NULL );
//        XmlUtil::AddDoubleNode( point_mass_node, "Pos_X", m_PointMassVec[i]->m_PosX() );
//        XmlUtil::AddDoubleNode( point_mass_node, "Pos_Y", m_PointMassVec[i]->m_PosY() );
//        XmlUtil::AddDoubleNode( point_mass_node, "Pos_Z", m_PointMassVec[i]->m_PosZ() );
//        XmlUtil::AddDoubleNode( point_mass_node, "Attach_X", m_PointMassVec[i]->m_AttachPos.x() );
//        XmlUtil::AddDoubleNode( point_mass_node, "Attach_Y", m_PointMassVec[i]->m_AttachPos.y() );
//        XmlUtil::AddDoubleNode( point_mass_node, "Attach_Z", m_PointMassVec[i]->m_AttachPos.z() );
//    }
//
//}

//void FeaMeshMgrSingleton::SetFeaStructData( Geom* geom_ptr, xmlNodePtr root )
//{
//    int numFEAStructureNodes = XmlUtil::GetNumNames( root, "FEA_Structure_Parms" );
//
//    if ( numFEAStructureNodes == 0 )
//    {
//        return;
//    }
//
//    m_DataGeomVec.push_back( geom_ptr );
//    m_XmlDataNode = xmlCopyNode( root, 100 );
//}
//
//void FeaMeshMgrSingleton::CopyGeomPtr( Geom* from_geom, Geom* to_geom )
//{
//    bool found = false;
//    for ( int i = 0 ; i < ( int )m_DataGeomVec.size() ; i++ )
//        if ( from_geom == m_DataGeomVec[i] )
//        {
//            found = true;
//        }
//
//    if ( found )
//    {
//        m_DataGeomVec.push_back( to_geom );
//    }
//}

//void FeaMeshMgrSingleton::ReadFeaStructData()
//{
//    bool found = false;
//    for ( int i = 0 ; i < ( int )m_DataGeomVec.size() ; i++ )
//    {
//        if ( m_WingGeom == m_DataGeomVec[i] )
//        {
//            found = true;
//        }
//    }
//
//    if ( !found )
//    {
//        return;
//    }
//
//    int numFEAStructureNodes = XmlUtil::GetNumNames( m_XmlDataNode, "FEA_Structure_Parms" );
//
//    if ( numFEAStructureNodes == 0 )
//    {
//        return;
//    }
//
//    xmlNodePtr struct_parms_node = XmlUtil::GetNode( m_XmlDataNode, "FEA_Structure_Parms", 0 );
//
//    GetGridDensityPtr()->m_BaseLen = XmlUtil::FindDouble( struct_parms_node, "DefElemSize", GetGridDensityPtr()->m_BaseLen() );
//
//    GetGridDensityPtr()->m_MinLen = XmlUtil::FindDouble( struct_parms_node, "FEA_Mesh_Min_Length", GetGridDensityPtr()->m_MinLen() );
//    GetGridDensityPtr()->m_MaxGap = XmlUtil::FindDouble( struct_parms_node, "FEA_Mesh_Max_Gap", GetGridDensityPtr()->m_MaxGap() );
//    GetGridDensityPtr()->m_NCircSeg = XmlUtil::FindDouble( struct_parms_node, "FEA_Mesh_Num_Circle_Segments", GetGridDensityPtr()->m_NCircSeg() );
//    GetGridDensityPtr()->m_GrowRatio = XmlUtil::FindDouble( struct_parms_node, "FEA_Mesh_Growth_Ratio", GetGridDensityPtr()->m_GrowRatio() );
//
//    m_ThickScale  = XmlUtil::FindDouble( struct_parms_node, "ThickScale", m_ThickScale() );
//
//    xmlNodePtr wing_sec_list_node = XmlUtil::GetNode( struct_parms_node, "Wing_Section_List", 0 );
//    if ( wing_sec_list_node )
//    {
//        int num_wing_sec =  XmlUtil::GetNumNames( wing_sec_list_node, "Wing_Section" );
//        if ( num_wing_sec == ( int )m_WingSections.size() )
//        {
//            for ( int i = 0 ; i < ( int )m_WingSections.size() ; i++ )
//            {
//                xmlNodePtr sec_node = XmlUtil::GetNode( wing_sec_list_node, "Wing_Section", i );
//                m_WingSections[i].ReadData( sec_node );
//            }
//        }
//    }
//
//    //==== Compute Rib and Spar End Points ====//
//    for ( int i = 0 ; i < ( int )m_WingSections.size() ; i++ )
//    {
//        for ( int j = 0 ; j < ( int )m_WingSections[i].m_RibVec.size() ; j++ )
//        {
//            m_WingSections[i].m_RibVec[j]->ComputeEndPoints();
//        }
//        for ( int j = 0 ; j < ( int )m_WingSections[i].m_SparVec.size() ; j++ )
//        {
//            m_WingSections[i].m_SparVec[j]->ComputeEndPoints();
//        }
//    }
//
//    xmlNodePtr point_mass_list_node = XmlUtil::GetNode( struct_parms_node, "Point_Mass_List", 0 );
//    if ( point_mass_list_node )
//    {
//        int num_point_masses =  XmlUtil::GetNumNames( point_mass_list_node, "Point_Mass" );
//        for ( int i = 0 ; i < num_point_masses ; i++ )
//        {
//            xmlNodePtr point_mass_node = XmlUtil::GetNode( point_mass_list_node, "Point_Mass", i );
//            if ( point_mass_node )
//            {
//                FeaPointMass* pm = new FeaPointMass();
//                pm->m_PosX = XmlUtil::FindDouble( point_mass_node, "Pos_X", 0.0 );
//                pm->m_PosY = XmlUtil::FindDouble( point_mass_node, "Pos_Y", 0.0 );
//                pm->m_PosZ = XmlUtil::FindDouble( point_mass_node, "Pos_Z", 0.0 );
//                pm->m_AttachPos.set_x( XmlUtil::FindDouble( point_mass_node, "Attach_X", 0.0 ) );
//                pm->m_AttachPos.set_y( XmlUtil::FindDouble( point_mass_node, "Attach_Y", 0.0 ) );
//                pm->m_AttachPos.set_z( XmlUtil::FindDouble( point_mass_node, "Attach_Z", 0.0 ) );
//                m_PointMassVec.push_back( pm );
//            }
//        }
//    }
//}

bool FeaMeshMgrSingleton::LoadSurfaces()
{
    CleanUp();

    if ( !StructureMgr.ValidTotalFeaStructInd( m_FeaMeshStructIndex ) )
    {
        addOutputText( "FeaMesh Failed: Invalid FeaStructure Selection\n" );
        m_FeaMeshInProgress = false;
        return false;
    }

    // Identify the structure to mesh (m_FeaMeshStructIndex must be set) 
    vector < FeaStructure* > structvec = StructureMgr.GetAllFeaStructs();
    m_FeaMeshStruct = structvec[m_FeaMeshStructIndex];

    LoadSkins();

    CleanMergeSurfs( );

    return true;
}

void FeaMeshMgrSingleton::LoadSkins()
{
    FeaPart* FeaSkin = m_FeaMeshStruct->GetFeaSkin();

    if ( FeaSkin )
    {
        //===== Add FeaSkins ====//
        vector< XferSurf > skinxfersurfs;

        int skin_index = m_FeaMeshStruct->GetFeaPartIndex( FeaSkin );

        FeaSkin->FetchFeaXFerSurf( skinxfersurfs, 0 );

        // Load Skin XFerSurf to m_SurfVec
        LoadSurfs( skinxfersurfs );

        // Not sure this is needed, could possibly be done in Fetch call above.
        for ( int j = 0; j < m_SurfVec.size(); j++ )
        {
            m_SurfVec[j]->SetFeaPartIndex( skin_index );
        }
    }
}

void FeaMeshMgrSingleton::GenerateFeaMesh()
{
    m_OutStream.clear();
    m_FeaMeshInProgress = true;

    addOutputText( "Load Surfaces\n" );
    LoadSurfaces();

    if ( m_SurfVec.size() <= 0 )
    {
        m_FeaMeshInProgress = false;
        return;
    }

    addOutputText( "Add Structure Parts\n" );
    AddStructureParts();

    // TODO: Update and Build Domain for Half Mesh?

    DeleteAllSources(); // TODO: Remove? No sources in FeaMesh

    addOutputText( "Build Slice Planes\n" );
    BuildGrid();

    addOutputText( "Intersect\n" );
    Intersect();

    addOutputText( "Build Target Map\n" );
    BuildTargetMap( CfdMeshMgrSingleton::VOCAL_OUTPUT );

    addOutputText( "InitMesh\n" );
    InitMesh();

    SubTagTris();

    addOutputText( "Remesh\n" );
    Remesh( CfdMeshMgrSingleton::VOCAL_OUTPUT );

    SubSurfaceMgr.BuildSingleTagMap();

    addOutputText( "Build Fea Mesh\n" );
    BuildFeaMesh();

    addOutputText( "Tag Fea Nodes\n" );
    TagFeaNodes();

    addOutputText( "Exporting Files\n" );
    ExportFeaMesh();

    addOutputText( "Finished\n" );

    m_FeaMeshInProgress = false;
}

void FeaMeshMgrSingleton::ExportFeaMesh()
{
    WriteNASTRAN( GetStructSettingsPtr()->GetExportFileName( vsp::NASTRAN_FILE_NAME ) );
    WriteCalculix();
    WriteSTL( GetStructSettingsPtr()->GetExportFileName( vsp::STL_FEA_NAME ) );
    WriteGmsh();

    ComputeWriteMass();

    string mass_output = "Total Mass = " + std::to_string( m_TotalMass ) + "\n";
    addOutputText( mass_output );
}

void FeaMeshMgrSingleton::AddStructureParts()
{
    vector < FeaPart* > FeaPartVec = m_FeaMeshStruct->GetFeaPartVec();

    //===== Add FeaParts ====//
    int num_parts = FeaPartVec.size();

    for ( int i = 1; i < num_parts; i++ ) // FeaSkin is index 0 and has been added already
    {
        int part_index = m_FeaMeshStruct->GetFeaPartIndex( FeaPartVec[i] );
        vector< XferSurf > partxfersurfs;

        FeaPartVec[i]->FetchFeaXFerSurf( partxfersurfs, -9999 );

        // Load Rib XFerSurf to m_SurfVec
        LoadSurfs( partxfersurfs );

        // Identify the FeaPart Type and ID. Add to m_FeaPartSurfVec
        int begin = m_SurfVec.size() - partxfersurfs.size();
        int end = m_SurfVec.size();

        for ( int j = begin; j < end; j++ )
        {
            m_SurfVec[j]->SetFeaPartIndex( part_index );
        }

        //else if ( FeaPartVec[i]->GetType() == vsp::FEA_FIX_POINT )
        //{
        //    FeaFixPoint* fixpt = dynamic_cast<FeaFixPoint*>( FeaPartVec[i] );
        //    assert( fixpt );
        //}
        //else if ( FeaPartVec[i]->GetType() == vsp::FEA_STIFFENER_PLANE )
        //{
        //    FeaStiffenerPlane* stiffener_plane = dynamic_cast<FeaStiffenerPlane*>( FeaPartVec[i] );
        //    assert( stiffener_plane );
        //}
        //else if ( FeaPartVec[i]->GetType() == vsp::FEA_STIFFENER_SUB_SURF )
        //{
        //    FeaStiffenerSubSurf* stiffener_subsurf = dynamic_cast<FeaStiffenerSubSurf*>( FeaPartVec[i] );
        //    assert( stiffener_subsurf );
        //}
    }
}

void FeaMeshMgrSingleton::BuildFeaMesh()
{
    for ( int s = 0; s < (int)m_SurfVec.size(); s++ )
    {
        int prop_index = m_FeaMeshStruct->GetFeaPropertyIndex( m_SurfVec[s]->GetFeaPartIndex() );

        vector < vec3d >pvec = m_SurfVec[s]->GetMesh()->GetSimpPntVec();
        vector < SimpTri > tvec = m_SurfVec[s]->GetMesh()->GetSimpTriVec();
        for ( int i = 0; i < (int)tvec.size(); i++ )
        {
            FeaTri* tri = new FeaTri;
            tri->Create( pvec[tvec[i].ind0], pvec[tvec[i].ind1], pvec[tvec[i].ind2] );
            tri->SetFeaPartIndex( m_SurfVec[s]->GetFeaPartIndex() );
            tri->SetFeaPropertyIndex( prop_index );
            m_FeaElementVec.push_back( tri );
        }
    }
}

void FeaMeshMgrSingleton::ComputeWriteMass()
{
    m_TotalMass = 0.0;

    FILE* fp = fopen( GetStructSettingsPtr()->GetExportFileName( vsp::MASS_FILE_NAME ).c_str(), "w" );
    if ( fp )
    {
        fprintf( fp, "FeaStruct_Name: %s\n", m_FeaMeshStruct->GetFeaStructName().c_str() );

        int num_fea_parts = m_FeaMeshStruct->NumFeaParts();

        // Iterate over each FeaPart index and calculate mass of each FeaElement if the current indexes match
        for ( unsigned int i = 0; i < num_fea_parts; i++ )
        {
            double mass = 0;

            for ( int j = 0; j < m_FeaElementVec.size(); j++ )
            {
                if ( m_FeaElementVec[j]->GetFeaPartIndex() == i )
                {
                    mass += m_FeaElementVec[j]->ComputeMass();
                }
            }

            string name = m_FeaMeshStruct->GetFeaPartName( i );

            fprintf( fp, "\tFeaPartName: %s, Mass = %f\n", name.c_str(), mass );
            m_TotalMass += mass;
        }

        fprintf( fp, "Total Mass = %f\n", m_TotalMass );

        fclose( fp );
    }
}

FeaNode* FeaMeshMgrSingleton::FindNode( vector< FeaNode* > nodeVec, int id )
{
    //==== jrg Brute Force for Now ====//
    for ( int i = 0 ; i < ( int )nodeVec.size() ; i++ )
    {
        if ( nodeVec[i]->m_Index == id )
        {
            return nodeVec[i];
        }
    }

    return NULL;
}

void FeaMeshMgrSingleton::TagFeaNodes()
{
    //==== Collect All FeaNodes ====//
    m_FeaNodeVec.clear();
    for ( int i = 0; i < (int)m_FeaElementVec.size(); i++ )
    {
        m_FeaElementVec[i]->LoadNodes( m_FeaNodeVec );
    }

    vector< vec3d* > m_AllPntVec;
    for ( int i = 0; i < (int)m_FeaNodeVec.size(); i++ )
    {
        m_AllPntVec.push_back( &m_FeaNodeVec[i]->m_Pnt );
    }

    //==== Build Node Map ====//
    m_IndMap.clear();
    m_PntShift.clear();
    int numPnts = BuildIndMap( m_AllPntVec, m_IndMap, m_PntShift );

    //==== Assign Index Numbers to Nodes ====//
    for ( int i = 0; i < (int)m_FeaNodeVec.size(); i++ )
    {
        m_FeaNodeVec[i]->m_Tags.clear();
        int ind = FindPntIndex( m_FeaNodeVec[i]->m_Pnt, m_AllPntVec, m_IndMap );
        m_FeaNodeVec[i]->m_Index = m_PntShift[ind] + 1;
    }

    int num_fea_parts = m_FeaMeshStruct->NumFeaParts();

    for ( unsigned int i = 0; i < num_fea_parts; i++ )
    {
        vector< FeaNode* > temp_nVec;

        for ( int j = 0; j < m_FeaElementVec.size(); j++ )
        {
            if ( m_FeaElementVec[j]->GetFeaPartIndex() == i )
            {
                m_FeaElementVec[j]->LoadNodes( temp_nVec );
            }
        }

        for ( int j = 0; j < (int)temp_nVec.size(); j++ )
        {
            int ind = FindPntIndex( temp_nVec[j]->m_Pnt, m_AllPntVec, m_IndMap );
            m_FeaNodeVec[ind]->AddTag( i );
        }
    }
}

void FeaMeshMgrSingleton::WriteNASTRAN( const string &filename )
{
    FILE* fp = fopen( filename.c_str(), "w" );
    if ( fp )
    {
        //fprintf( fp, "CEND\n" );
        fprintf( fp, "BEGIN BULK\n" );

        int elem_id = 0;
        int num_fea_parts = m_FeaMeshStruct->NumFeaParts();

        for ( unsigned int i = 0; i < num_fea_parts; i++ )
        {
            fprintf( fp, "\n" );
            fprintf( fp, "$%s\n", m_FeaMeshStruct->GetFeaPartName( i ).c_str() );

            for ( int j = 0; j < m_FeaElementVec.size(); j++ )
            {
                if ( m_FeaElementVec[j]->GetFeaPartIndex() == i )
                {
                    elem_id++;
                    m_FeaElementVec[j]->WriteNASTRAN( fp, elem_id );
                }
            }
        }

        for ( unsigned int i = 0; i < num_fea_parts; i++ )
        {
            fprintf( fp, "\n" );
            fprintf( fp, "$%s Gridpoints\n", m_FeaMeshStruct->GetFeaPartName( i ).c_str() );

            for ( unsigned int j = 0; j < (int)m_FeaNodeVec.size(); j++ )
            {
                if ( m_PntShift[j] >= 0 )
                {
                    if ( m_FeaNodeVec[j]->HasOnlyIndex( i ) )
                    {
                        m_FeaNodeVec[j]->WriteNASTRAN( fp );
                    }
                }
            }
        }

        // TODO: Write and improve intersection elements/nodes

        fprintf( fp, "\n" );
        fprintf( fp, "$Intersections\n" );

        for ( unsigned int j = 0; j < (int)m_FeaNodeVec.size(); j++ )
        {
            if ( m_PntShift[j] >= 0 )
            {
                if ( m_FeaNodeVec[j]->m_Tags.size() > 1 )
                {
                    m_FeaNodeVec[j]->WriteNASTRAN( fp );
                }
            }
        }



        ////==== Write Rib Spar Intersections =====//
        //for ( int r = 0 ; r < m_SurfVec.size(); r++ )
        //{
        //    if ( m_SurfVec[r]->GetFeaPartType() == vsp::FEA_RIB )
        //    {
        //        for ( int s = 0; s < m_SurfVec.size(); s++ )
        //        {
        //            if ( m_SurfVec[s]->GetFeaPartType() == vsp::FEA_SPAR )
        //            {
        //                fprintf( fp, "\n" );
        //                fprintf( fp, "$Intersection,%d,%d\n", r, s );

        //                for ( int i = 0; i < (int)m_FeaNodeVec.size(); i++ )
        //                {
        //                    if ( m_PntShift[i] >= 0 )
        //                    {
        //                        if ( m_FeaNodeVec[i]->HasTag( vsp::FEA_RIB, r ) && m_FeaNodeVec[i]->HasTag( vsp::FEA_SPAR, s ) )
        //                        {
        //                            m_FeaNodeVec[i]->WriteNASTRAN( fp );
        //                        }
        //                    }
        //                }
        //                fprintf( fp, "\n" );
        //            }
        //        }
        //    }
        //}

        ////==== Write Out Rib LE/TE ====//
        //for ( int r = 0 ; r < rib_cnt ; r++ )
        //{
        //    vector< FeaNode* > letenodes;
        //    for ( int i = 0 ; i < ( int )nodeVec.size() ; i++ )
        //    {
        //        if ( nodeVec[i]->m_Tags.size() == 2 )
        //        {
        //            if ( nodeVec[i]->HasTag( RIB_LOWER, r + 1 ) && nodeVec[i]->HasTag( RIB_UPPER, r + 1 ) )
        //            {
        //                letenodes.push_back( nodeVec[i] );
        //            }
        //        }
        //    }
        //    if ( letenodes.size() == 2 )
        //    {
        //        if ( letenodes[1]->m_Pnt.x() < letenodes[0]->m_Pnt.x() )
        //        {
        //            FeaNode* temp = letenodes[0];
        //            letenodes[0]  = letenodes[1];
        //            letenodes[1]  = temp;
        //        }
        //        fprintf( fp, "\n" );
        //        fprintf( fp, "$RibLE,%d\n", r + 1 );
        //        letenodes[0]->WriteNASTRAN( fp );

        //        fprintf( fp, "\n" );
        //        fprintf( fp, "$RibTE,%d\n", r + 1 );
        //        letenodes[1]->WriteNASTRAN( fp );
        //    }
        //}

        ////==== Write Rib Upper Boundary =====//
        //for ( int r = 0 ; r < rib_cnt ; r++ )
        //{
        //    fprintf( fp, "\n" );
        //    fprintf( fp, "$RibUpperBoundary,%d\n", r + 1 );
        //    for ( int i = 0 ; i < ( int )nodeVec.size() ; i++ )
        //    {
        //        if ( nodeVec[i]->HasTag( RIB_UPPER, r + 1 ) && nodeVec[i]->m_Tags.size() == 1 )
        //        {
        //            nodeVec[i]->WriteNASTRAN( fp );
        //        }
        //    }
        //}
        ////==== Write Spar Upper Boundary =====//
        //for ( int s = 0 ; s < spar_cnt ; s++ )
        //{
        //    fprintf( fp, "\n" );
        //    fprintf( fp, "$SparUpperBoundary,%d\n", s + 1 );
        //    for ( int i = 0 ; i < ( int )nodeVec.size() ; i++ )
        //    {
        //        if ( nodeVec[i]->HasTag( SPAR_UPPER, s + 1 ) && nodeVec[i]->m_Tags.size() == 1 )
        //        {
        //            nodeVec[i]->WriteNASTRAN( fp );
        //        }
        //    }
        //}
        ////==== Write Rib Lower Boundary  =====//
        //for ( int r = 0 ; r < rib_cnt ; r++ )
        //{
        //    fprintf( fp, "\n" );
        //    fprintf( fp, "$RibLowerBoundary,%d\n", r + 1 );
        //    for ( int i = 0 ; i < ( int )nodeVec.size() ; i++ )
        //    {
        //        if ( nodeVec[i]->HasTag( RIB_LOWER, r + 1 ) && nodeVec[i]->m_Tags.size() == 1 )
        //        {
        //            nodeVec[i]->WriteNASTRAN( fp );
        //        }
        //    }
        //}
        ////==== Write Spar Lower Boundary =====//
        //for ( int s = 0 ; s < spar_cnt ; s++ )
        //{
        //    fprintf( fp, "\n" );
        //    fprintf( fp, "$SparLowerBoundary,%d\n", s + 1 );
        //    for ( int i = 0 ; i < ( int )nodeVec.size() ; i++ )
        //    {
        //        if ( nodeVec[i]->HasTag( SPAR_LOWER, s + 1 ) && nodeVec[i]->m_Tags.size() == 1 )
        //        {
        //            nodeVec[i]->WriteNASTRAN( fp );
        //        }
        //    }
        //}
        ////==== Write Point Masses =====//
        //for ( int p = 0 ; p < ( int )m_PointMassVec.size() ; p++ )
        //{
        //    //==== Snap To Nearest Attach Point ====//
        //    int close_ind  = 0;
        //    double close_d = 1.0e12;

        //    FeaNode node;
        //    node.m_Pnt = vec3d( m_PointMassVec[p]->m_PosX(), m_PointMassVec[p]->m_PosY(), m_PointMassVec[p]->m_PosZ() );
        //    node.m_Index = numPnts + p + 1;
        //    fprintf( fp, "\n" );
        //    fprintf( fp, "$Pointmass,%d\n", p + 1 );
        //    node.WriteNASTRAN( fp );

        //    //==== Find Attach Point Index ====//
        //    int ind = FindPntIndex( m_PointMassVec[p]->m_AttachPos, allPntVec, indMap );
        //    fprintf( fp, "$Connects,%d\n", pntShift[ind] + 1 );
        //}

        //==== Remaining Nodes ====//
        fprintf( fp, "\n" );
        fprintf( fp, "$Remainingnodes\n" );
        for ( int i = 0 ; i < ( int )m_FeaNodeVec.size() ; i++ )
        {
            if ( m_PntShift[i] >= 0 && m_FeaNodeVec[i]->m_Tags.size() == 0 )
            {
                m_FeaNodeVec[i]->WriteNASTRAN( fp );
            }
        }

        //==== Properties ====//
        fprintf( fp, "\n" );
        fprintf( fp, "$Properties\n" );

        vector < FeaProperty* > property_vec = StructureMgr.GetFeaPropertyVec();

        for ( unsigned int i = 0; i < property_vec.size(); i++ )
        {
            property_vec[i]->WriteNASTRAN( fp, i );
        }

        //==== Materials ====//
        fprintf( fp, "\n" );
        fprintf( fp, "$Materials\n" );

        vector < FeaMaterial* > material_vec = StructureMgr.GetFeaMaterialVec();

        for ( unsigned int i = 0; i < material_vec.size(); i++ )
        {
            material_vec[i]->WriteNASTRAN( fp, i );
        }

        fprintf( fp, "END DATA\n" );

        fclose( fp );
    }
}

void FeaMeshMgrSingleton::WriteCalculix()
{
    string fn = GetStructSettingsPtr()->GetExportFileName( vsp::GEOM_FILE_NAME );
    FILE* fp = fopen( fn.c_str(), "w" );
    if ( fp )
    {
        int elem_id = 0;
        int property_id = 0;
        char str[256];

        int num_fea_parts = m_FeaMeshStruct->NumFeaParts();

        for ( unsigned int i = 0; i < num_fea_parts; i++ )
        {
            fprintf( fp, "$**%%%s\n", m_FeaMeshStruct->GetFeaPartName( i ).c_str() );
            fprintf( fp, "*NODE, NSET=N%s\n", m_FeaMeshStruct->GetFeaPartName( i ).c_str() );

            for ( unsigned int j = 0; j < (int)m_FeaNodeVec.size(); j++ )
            {
                if ( m_PntShift[j] >= 0 )
                {
                    if ( m_FeaNodeVec[j]->HasOnlyIndex( i ) )
                    {
                        m_FeaNodeVec[j]->WriteCalculix( fp );
                    }
                }
            }

            fprintf( fp, "\n" );
            fprintf( fp, "*ELEMENT, TYPE=S6, ELSET=E%s\n", m_FeaMeshStruct->GetFeaPartName( i ).c_str() );

            for ( int j = 0; j < m_FeaElementVec.size(); j++ )
            {
                if ( m_FeaElementVec[j]->GetFeaPartIndex() == i )
                {
                    elem_id++;
                    m_FeaElementVec[j]->WriteCalculix( fp, elem_id );
                    property_id = m_FeaElementVec[j]->GetFeaPropertyIndex();
                }
            }

            fprintf( fp, "\n" );
            sprintf( str, "E%s", m_FeaMeshStruct->GetFeaPartName( i ).c_str() );

            StructureMgr.GetFeaProperty( property_id )->WriteCalculix( fp, str );
            fprintf( fp, "\n" );
        }

        // TODO: Identify and improve intersection elements and nodes

        fprintf( fp, "**%%Intersections\n" );
        fprintf( fp, "*NODE, NSET=Nintersections\n" );

        for ( unsigned int j = 0; j < (int)m_FeaNodeVec.size(); j++ )
        {
            if ( m_PntShift[j] >= 0 )
            {
                if ( m_FeaNodeVec[j]->m_Tags.size() > 1 )
                {
                    m_FeaNodeVec[j]->WriteCalculix( fp );
                }
            }
        }







        ////==== Rib Spar Intersections ====//
        //for ( int r = 0 ; r < ( int )m_SurfVec.size(); r++ )
        //{
        //    if ( m_SurfVec[r]->GetFeaPartType() == vsp::FEA_RIB )
        //    {
        //        rib_cnt++;
        //        spar_cnt = 0;

        //        for ( int s = 0; s < (int)m_SurfVec.size(); s++ )
        //        {
        //            if ( m_SurfVec[s]->GetFeaPartType() == vsp::FEA_SPAR )
        //            {
        //                spar_cnt++;
        //                fprintf( fp, "\n" );
        //                fprintf( fp, "**%%Rib-Spar connections %d %d\n", rib_cnt, spar_cnt );
        //                fprintf( fp, "*NODE, NSET=Nconnections%d%d\n", rib_cnt, spar_cnt );
        //                for ( int i = 0; i < (int)m_FeaNodeVec.size(); i++ )
        //                {
        //                    if ( m_PntShift[i] >= 0 )
        //                    {
        //                        if ( m_FeaNodeVec[i]->HasTag( vsp::FEA_RIB, r ) && m_FeaNodeVec[i]->HasTag( vsp::FEA_SPAR, s ) )
        //                        {
        //                            //nodeVec[i]->m_Thick = 0.5 * ( ribs[r]->m_Thickness + spars[s]->m_Thickness );
        //                            m_FeaNodeVec[i]->WriteCalculix( fp );
        //                        }
        //                    }
        //                }
        //            }
        //        }
        //    }
        //}

        //==== Materials ====//
        fprintf( fp, "\n" );
        fprintf( fp, "$Materials\n" );

        vector < FeaMaterial* > material_vec = StructureMgr.GetFeaMaterialVec();

        for ( unsigned int i = 0; i < material_vec.size(); i++ )
        {
            material_vec[i]->WriteCalculix( fp, i );
            fprintf( fp, "\n" );
        }

        fclose( fp );
    }
}

void FeaMeshMgrSingleton::WriteGmsh()
{
    string fn = GetStructSettingsPtr()->GetExportFileName( vsp::GMSH_FEA_NAME );
    FILE* fp = fopen( fn.c_str(), "w" );
    if ( fp )
    {
        //=====================================================================================//
        //============== Write Gmsh File ======================================================//
        //=====================================================================================//
        int num_fea_parts = m_FeaMeshStruct->NumFeaParts();

        fprintf( fp, "$MeshFormat\n" );
        fprintf( fp, "2.2 0 %d\n", ( int )sizeof( double ) );
        fprintf( fp, "$EndMeshFormat\n" );

        // Count FeaNodes
        int node_count = 0;
        for ( unsigned int j = 0; j < (int)m_FeaNodeVec.size(); j++ )
        {
            if ( m_PntShift[j] >= 0 )
            {
                node_count++;
            }
        }

        //==== Group and Name FeaParts ====//
        fprintf( fp, "$PhysicalNames\n" );
        fprintf( fp, "%d\n", num_fea_parts );
        for ( unsigned int i = 0; i < num_fea_parts; i++ )
        {
            fprintf( fp, "9 %d \"%s\"\n", i + 1, m_FeaMeshStruct->GetFeaPartName( i ).c_str() );
        }
        fprintf( fp, "$EndPhysicalNames\n" );

        //==== Write Nodes ====//
        fprintf( fp, "$Nodes\n" );
        fprintf( fp, "%d\n", node_count );

        for ( unsigned int j = 0; j < (int)m_FeaNodeVec.size(); j++ )
        {
            if ( m_PntShift[j] >= 0 )
            {
                m_FeaNodeVec[j]->WriteGmsh( fp );
            }
        }

        fprintf( fp, "$EndNodes\n" );

        //==== Write FeaElements ====//
        fprintf( fp, "$Elements\n" );
        fprintf( fp, "%d\n", (int)m_FeaElementVec.size() );

        int ele_cnt = 1;

        for ( unsigned int j = 0; j < num_fea_parts; j++ )
        {
            for ( int i = 0; i < (int)m_FeaElementVec.size(); i++ )
            {
                if ( m_FeaElementVec[i]->GetFeaPartIndex() == j )
                {
                    m_FeaElementVec[i]->WriteGmsh( fp, ele_cnt, j + 1 );
                    ele_cnt++;
                }
            }
        }

        fprintf( fp, "$EndElements\n" );
        fclose( fp );

        // Note: Material properties are not supported in *.msh file
    }
}

//void FeaMeshMgrSingleton::WriteCalculix( const char* base_filename )
//{
//  //==== Get All FeaNodes ====//
//  vector< FeaNode* > nodeVec;
//  for ( int i = 0 ; i < (int)m_SkinVec.size() ; i++ )
//      m_SkinVec[i]->LoadNodes( nodeVec );
//  for ( int i = 0 ; i < (int)m_SliceVec.size() ; i++ )
//      m_SliceVec[i]->LoadNodes( nodeVec );
//
//
//  double tol = 0.0001*0.0001;
//  vector< FeaNode* > unique_node_vec;
//  for ( int i = 0 ; i < (int)nodeVec.size() ; i++ )
//  {
//      for ( int j = 0 ; j < (int)unique_node_vec.size() ; j++ )
//      {
//          if ( dist_squared( unique_node_vec[j]->m_Pnt, nodeVec[i]->m_Pnt ) < tol )
//          {
//              nodeVec[i]->m_CloseNode = unique_node_vec[j];
//              break;
//          }
//      }
//      if ( !nodeVec[i]->m_CloseNode )
//          unique_node_vec.push_back( nodeVec[i] );
//  }
//
//  //==== Number Unique Nodes ====//
//  for ( int i = 0 ; i < (int)unique_node_vec.size() ; i++ )
//  {
//      unique_node_vec[i]->m_Index = i+1;
//  }
//
//  Stringc fn( base_filename );
//  fn.concatenate( "geom.dat" );
//
//  FILE* fp = fopen( fn.get_char_star(), "w" );
//  if ( fp )
//  {
//      fprintf( fp, "*NODE, NSET=Nall\n" );
//      for ( int i = 0 ; i < (int)unique_node_vec.size() ; i++ )
//      {
//          FeaNode* n = unique_node_vec[i];
//          fprintf(fp, "%d,%f,%f,%f\n", n->m_Index, n->m_Pnt.x(), n->m_Pnt.y(), n->m_Pnt.z() );
//      }
//
//      int elem_id = 1;
//      fprintf( fp, "*ELEMENT, TYPE=S6, ELSET=Ealltris\n" );
//      for ( int i = 0 ; i < (int)m_SliceVec.size() ; i++ )
//      {
//          for ( int e = 0 ; e < (int)m_SliceVec[i]->m_Elements.size() ; e++ )
//          {
//              if ( m_SliceVec[i]->m_Elements[e]->GetType() == FeaElement::FEA_TRI_6 )
//              {
//                  m_SliceVec[i]->m_Elements[e]->WriteCalculix( fp, elem_id );
//                  elem_id++;
//              }
//          }
//      }
//      for ( int i = 0 ; i < (int)m_SkinVec.size() ; i++ )
//      {
//          for ( int e = 0 ; e < (int)m_SkinVec[i]->m_Elements.size() ; e++ )
//          {
//              if ( m_SkinVec[i]->m_Elements[e]->GetType() == FeaElement::FEA_TRI_6 )
//              {
//                  m_SkinVec[i]->m_Elements[e]->WriteCalculix( fp, elem_id );
//                  elem_id++;
//              }
//          }
//      }
//
//      fprintf( fp, "*ELEMENT, TYPE=S8, ELSET=Eallquads\n" );
//      for ( int i = 0 ; i < (int)m_SliceVec.size() ; i++ )
//      {
//          for ( int e = 0 ; e < (int)m_SliceVec[i]->m_Elements.size() ; e++ )
//          {
//              if ( m_SliceVec[i]->m_Elements[e]->GetType() == FeaElement::FEA_QUAD_8 )
//              {
//                  m_SliceVec[i]->m_Elements[e]->WriteCalculix( fp, elem_id );
//                  elem_id++;
//              }
//          }
//      }
//      fclose(fp);
//  }
//
//  //==== Write Out All Nodes < YTol ====/
//  double ytol = 0.01;
//  fp = fopen( "fixed.nam", "w" );
//  if ( fp )
//  {
//      fprintf( fp, "**Fixed Nodes\n");
//      fprintf( fp, "*NSET,NSET=NFIXED\n");
//
//      int ncnt = 0;
//      for ( int i = 0 ; i < (int)unique_node_vec.size() ; i++ )
//      {
//          if ( unique_node_vec[i]->m_Pnt[1] < ytol )
//          {
//              fprintf(fp, "%d,", unique_node_vec[i]->m_Index );
//              ncnt++;
//              if ( ncnt%10 == 9 )
//                  fprintf(fp, "\n" );
//          }
//      }
//      fprintf(fp, "\n" );
//      fclose(fp);
//  }
//
//  //==== Write Node Thickness Include File ====//
//  Stringc node_fn( base_filename );
//  node_fn.concatenate( "nodethick.dat" );
//  fp = fopen( node_fn.get_char_star(), "w" );
//  if ( fp )
//  {
//      fprintf( fp, "*NODAL THICKNESS\n");
//      for ( int i = 0 ; i < (int)unique_node_vec.size() ; i++ )
//      {
//          fprintf(fp, "%d,%f\n", unique_node_vec[i]->m_Index, unique_node_vec[i]->m_Thick*m_ThickScale );
//      }
//      fclose(fp);
//  }
//
////CQUAD8 EID PID G1 G2 G3 G4 G5 G6
////       G7  G8  T1 T2 T3 T4 THETA ZOFFS
////EID - Element ID (Int > 0)
////PID - Property id number of a PSHELL, PCOMP, PLPLANE (Int > 0)
////G1-G4 ID of corner grid points
////G5-G8 ID of edge points
////T1-T4 Thickness at grid points
////Theta Material property orientation angle in degrees
////MCID  Material coordinate sys id number -
////ZOFFS Offset from the surface of grid points to the element reference plane
//
//  //===== Write NASTRAN Out File ====//
//  Stringc filenm( base_filename );
//  filenm.concatenate( "NASTRAN.dat" );
//  fp = fopen( filenm.get_char_star(), "w" );
//  if ( fp )
//  {
//      fprintf(fp, "$--1---><--2---><--3---><--4---><--5---><--6---><--7---><--8---><--9--->\n");
//      fprintf(fp, "$TRIS\n");
//      int elem_id = 1;
//      for ( int i = 0 ; i < (int)m_SliceVec.size() ; i++ )
//      {
//          for ( int e = 0 ; e < (int)m_SliceVec[i]->m_Elements.size() ; e++ )
//          {
//              if ( m_SliceVec[i]->m_Elements[e]->GetType() == FeaElement::FEA_TRI_6 )
//              {
//                  m_SliceVec[i]->m_Elements[e]->WriteNASTRAN( fp, elem_id );
//                  elem_id++;
//              }
//          }
//      }
//      for ( int i = 0 ; i < (int)m_SkinVec.size() ; i++ )
//      {
//          for ( int e = 0 ; e < (int)m_SkinVec[i]->m_Elements.size() ; e++ )
//          {
//              if ( m_SkinVec[i]->m_Elements[e]->GetType() == FeaElement::FEA_TRI_6 )
//              {
//                  m_SkinVec[i]->m_Elements[e]->WriteNASTRAN( fp, elem_id );
//                  elem_id++;
//              }
//          }
//      }
//      fprintf(fp, "$--1---><--2---><--3---><--4---><--5---><--6---><--7---><--8---><--9--->\n");
//      fprintf(fp, "$QUADS\n");
//      for ( int i = 0 ; i < (int)m_SliceVec.size() ; i++ )
//      {
//          for ( int e = 0 ; e < (int)m_SliceVec[i]->m_Elements.size() ; e++ )
//          {
//              if ( m_SliceVec[i]->m_Elements[e]->GetType() == FeaElement::FEA_QUAD_8 )
//              {
//                  m_SliceVec[i]->m_Elements[e]->WriteNASTRAN( fp, elem_id );
//                  elem_id++;
//              }
//          }
//      }
//      //for ( i = 0; i < (int)allTriVec.size() ; i++ )
//      //{
//      //  TTri* tri = &allTriVec[i];
//      //  //fprintf(fp, "%-8s%-8d%-8d%-8d%-8d%-8d%-8d\n", "CTRIA3", i+1, 1, tri->n0->id, tri->n1->id, tri->n2->id, 0 );
//      //  //fprintf(fp, "%-8s%-8d%-8d%-8d%-8d%-8d%-8d\n", "CTRIA3", i+1, 1, tri->cn0->id, tri->cn1->id, tri->cn2->id, 0 );
//      //  fprintf(fp, "%-8s%-8d%-8d%-8d%-8d%-8d%-8d%-8d%-8d%-8d\n",
//      //      "CTRIA6", i+1, 1, tri->n0->id, tri->n1->id, tri->n2->id, tri->cn0->id, tri->cn2->id, tri->cn1->id,0 );
//      //  //  "CTRIA6", i+1, 1, tri->n0->id, tri->cn0->id, tri->n1->id, tri->cn1->id, tri->n2->id, tri->cn2->id,0 );
//      //}
//      fprintf(fp, "$--1---><--2---><--3---><--4---><--5---><--6---><--7---><--8---><--9--->\n");
//      fprintf(fp, "$NODES\n");
//      for ( int i = 0 ; i < (int)unique_node_vec.size() ; i++ )
//      {
//          FeaNode* n = unique_node_vec[i];
//          fprintf(fp, "%-8s%-8d%-8d%8.3f%8.3f%8.3f\n", "GRID", n->m_Index, 0, n->m_Pnt.x(), n->m_Pnt.y(), n->m_Pnt.z() );
//      }
//
//      //for ( i = 0; i < (int)allNodeVec.size() ; i++ )
//      //{
//      //  TNode* n = allNodeVec[i];
//      //  fprintf(fp, "%-8s%-8d%-8d%8.3f%8.3f%8.3f\n", "GRID", n->id, 0, n->pnt.x(), n->pnt.y(), n->pnt.z() );
//      //}
//      fclose(fp);
//  }
//
//
//}

//void FeaMeshMgrSingleton::Draw()
//{
//  if ( !m_DrawFlag )
//      return;
//
//  GLboolean smooth_flag = glIsEnabled( GL_LINE_SMOOTH );
//  glDisable( GL_LINE_SMOOTH );
//  glDisable( GL_POINT_SMOOTH );
//
//  if ( !m_DrawMeshFlag )
//  {
//      FeaRib* curr_rib = GetCurrRib();
//      for ( int i = 0 ; i < (int)m_WingSections.size() ; i++ )
//      {
//          for ( int j = 0 ; j < (int)m_WingSections[i].m_RibVec.size() ; j++ )
//          {
//              FeaRib* rib = m_WingSections[i].m_RibVec[j];
//              if ( m_CurrEditType == RIB_EDIT && rib == curr_rib )
//                  rib->Draw( true );
//              else
//                  rib->Draw( false );
//          }
//      }
//
//      FeaSpar* curr_spar = GetCurrSpar();
//      for ( int i = 0 ; i < (int)m_WingSections.size() ; i++ )
//      {
//          for ( int j = 0 ; j < (int)m_WingSections[i].m_SparVec.size() ; j++ )
//          {
//              FeaSpar* spar = m_WingSections[i].m_SparVec[j];
//              if ( m_CurrEditType == SPAR_EDIT && spar == curr_spar )
//                  spar->Draw( true );
//              else
//                  spar->Draw( false );
//          }
//      }
//
//      //==== Wing Sections ====//
//      for ( int i = 0 ; i < (int)m_WingSections.size() ; i++ )
//      {
//          if ( i == m_CurrSectID )
//          {
//              if ( m_CurrEditType == UP_SKIN_EDIT )
//                  m_WingSections[i].m_UpperSkin.Draw( true );
//              else if ( m_CurrEditType == LOW_SKIN_EDIT )
//                  m_WingSections[i].m_LowerSkin.Draw( true );
//              m_WingSections[i].Draw( true );
//          }
//          else
//          {
//              if ( m_CurrEditType == UP_SKIN_EDIT )
//                  m_WingSections[i].m_UpperSkin.Draw( false );
//              else if ( m_CurrEditType == LOW_SKIN_EDIT )
//                  m_WingSections[i].m_LowerSkin.Draw( false );
//              m_WingSections[i].Draw( false );
//          }
//      }
//
//  }
//
//  if ( m_DrawMeshFlag )
//  {
//      glPointSize( 6.0 );
//      //glColor4ub( 255, 0, 250, 255);
//      //glBegin( GL_POINTS );
//      //for ( int i = 0 ; i < debugPnts.size() ; i++ )
//      //{
//      //  glVertex3dv( debugPnts[i].data() );
//
//      //}
//      //glEnd();
//      //////==== Collect All FeaNodes ====//
//      ////vector< FeaNode* > nodeVec;
//      ////for ( int i = 0 ; i < (int)m_SkinVec.size() ; i++ )
//      ////    m_SkinVec[i]->LoadNodes( nodeVec );
//      ////for ( int i = 0 ; i < (int)m_SliceVec.size() ; i++ )
//      ////    m_SliceVec[i]->LoadNodes( nodeVec );
//
//      ////for ( int i = 0 ; i < (int)nodeVec.size() ; i++ )
//      ////{
//      ////    if ( nodeVec[i]->m_Tags.size() > 0 )
//      ////    {
//      ////        glPointSize( 6.0 );
//      ////        glColor4ub( 255, 0, 250, 255);
//      ////        glBegin( GL_POINTS );
//      ////            glVertex3dv( nodeVec[i]->m_Pnt.data() );
//      ////        glEnd();
//      ////    }
//      ////}
//
//      //==== Draw Ribs Spars ====//
//      glColor4ub( 0, 0, 250, 255);
//      for ( int i = 0 ; i < (int)m_SliceVec.size() ; i++ )
//      {
//          for ( int e = 0 ; e < (int)m_SliceVec[i]->m_Elements.size() ; e++ )
//          {
//              FeaElement* fe = m_SliceVec[i]->m_Elements[e];
//              glBegin( GL_POLYGON );
//              for ( int p = 0 ; p < (int)fe->m_Corners.size() ; p++ )
//                  glVertex3dv( fe->m_Corners[p]->m_Pnt.data() );
//              glEnd();
//          }
//      }
//      glLineWidth(2.0);
//      glColor4ub( 0, 0, 0, 255 );
//      for ( int i = 0 ; i < (int)m_SliceVec.size() ; i++ )
//      {
//          for ( int e = 0 ; e < (int)m_SliceVec[i]->m_Elements.size() ; e++ )
//          {
//              FeaElement* fe = m_SliceVec[i]->m_Elements[e];
//              glBegin( GL_LINE_LOOP );
//              for ( int p = 0 ; p < (int)fe->m_Corners.size() ; p++ )
//                  glVertex3dv( fe->m_Corners[p]->m_Pnt.data() );
//              glEnd();
//          }
//      }
//
//      //==== Draw Potential Point Mass Attachment Points ====//
//      if ( m_CurrEditType == POINT_MASS_EDIT && m_DrawAttachPoints )
//      {
//          glPointSize(6.0);
//          glColor4ub( 100, 100, 100, 255 );
//          glBegin( GL_POINTS );
//          for ( int i = 0 ; i < (int)m_AttachPoints.size() ; i++ )
//          {
//              if ( i != m_ClosestAttachPoint )
//              {
//                  glVertex3dv( m_AttachPoints[i].data() );
//              }
//          }
//          glEnd();
//
//          if ( m_ClosestAttachPoint >= 0 && m_ClosestAttachPoint < (int)m_AttachPoints.size() )
//          {
//              glPointSize(8.0);
//              glColor4ub( 255, 0, 0, 255 );
//              glBegin( GL_POINTS );
//              glVertex3dv( m_AttachPoints[m_ClosestAttachPoint].data() );
//              glEnd();
//          }
//      }
//
//      //==== Draw Skin ====//
//      glCullFace( GL_BACK );                      // Cull Back Faces For Trans
//      glEnable( GL_CULL_FACE );
//
//      glColor4ub( 150, 150, 150, 50 );
//      for ( int i = 0 ; i < (int)m_SkinVec.size() ; i++ )
//      {
//          if ( m_SkinVec[i]->GetExportFlag() )
//          {
//              for ( int e = 0 ; e < (int)m_SkinVec[i]->m_Elements.size() ; e++ )
//              {
//                  FeaElement* fe = m_SkinVec[i]->m_Elements[e];
//                  glBegin( GL_POLYGON );
//                  for ( int p = 0 ; p < (int)fe->m_Corners.size() ; p++ )
//                      glVertex3dv( fe->m_Corners[p]->m_Pnt.data() );
//                  glEnd();
//              }
//          }
//      }
//      glLineWidth(2.0);
//      glColor4ub( 0, 0, 0, 100 );
//      for ( int i = 0 ; i < (int)m_SkinVec.size() ; i++ )
//      {
//          if ( m_SkinVec[i]->GetExportFlag() )
//          {
//              for ( int e = 0 ; e < (int)m_SkinVec[i]->m_Elements.size() ; e++ )
//              {
//                  FeaElement* fe = m_SkinVec[i]->m_Elements[e];
//                  glBegin( GL_LINE_LOOP );
//                  for ( int p = 0 ; p < (int)fe->m_Corners.size() ; p++ )
//                      glVertex3dv( fe->m_Corners[p]->m_Pnt.data() );
//                  glEnd();
//              }
//          }
//      }
//
//      glDisable( GL_CULL_FACE );
//  }
//
//  //==== Draw Potential Point Mass Attachment Points ====//
//  if ( m_CurrEditType == POINT_MASS_EDIT  )
//  {
//      for ( int i = 0 ; i < (int)m_PointMassVec.size() ; i++ )
//      {
//          if ( i == m_CurrPointMassID )
//              m_PointMassVec[i]->Draw( true );
//          else
//              m_PointMassVec[i]->Draw( false );
//      }
//  }
//
//
//
//
//
//  if ( smooth_flag )
//  {
//      glEnable( GL_LINE_SMOOTH );
//      glEnable( GL_POINT_SMOOTH );
//  }
//
//}

void FeaMeshMgrSingleton::LoadDrawObjs( vector< DrawObj* > &draw_obj_vec )
{
    // TODO: Select a method for drawing FeaMesh. Both ways currently work

    if ( GetStructSettingsPtr()->m_DrawMeshFlag() && !GetFeaMeshInProgress() )
    {
        // Render Tag Colors
        int num_tags = SubSurfaceMgr.GetNumTags();
        m_SSTagDO.resize( num_tags );
        map<int, DrawObj*> tag_dobj_map;
        map< std::vector<int>, int >::const_iterator mit;
        map< int, DrawObj* >::const_iterator dmit;
        map< std::vector<int>, int > tagMap = SubSurfaceMgr.GetSingleTagMap();
        int cnt = 0;

        // Calculate constants for color sequence.
        const int ncgrp = 6; // Number of basic colors
        const int ncstep = (int)ceil( (double)num_tags / (double)ncgrp );
        const double nctodeg = 360.0 / ( ncgrp*ncstep );

        char str[256];
        for ( mit = tagMap.begin(); mit != tagMap.end(); mit++ )
        {
            m_SSTagDO[cnt] = DrawObj();
            tag_dobj_map[mit->second] = &m_SSTagDO[cnt];
            sprintf( str, "%s_TAG_%d", GetID().c_str(), cnt );
            m_SSTagDO[cnt].m_GeomID = string( str );

            m_SSTagDO[cnt].m_Type = DrawObj::VSP_SHADED_TRIS;
            m_SSTagDO[cnt].m_Visible = false;
            if ( GetStructSettingsPtr()->m_DrawMeshFlag.Get() ||
                 GetStructSettingsPtr()->m_ColorTagsFlag.Get() )   // At least mesh or tags are visible.
            {
                m_SSTagDO[cnt].m_Visible = true;

                if ( GetStructSettingsPtr()->m_DrawMeshFlag.Get() &&
                     GetStructSettingsPtr()->m_ColorTagsFlag.Get() ) // Both are visible.
                {
                    m_SSTagDO[cnt].m_Type = DrawObj::VSP_HIDDEN_TRIS_CFD;
                    m_SSTagDO[cnt].m_LineColor = vec3d( 0.4, 0.4, 0.4 );
                }
                else if ( GetStructSettingsPtr()->m_DrawMeshFlag.Get() ) // Mesh only
                {
                    m_SSTagDO[cnt].m_Type = DrawObj::VSP_HIDDEN_TRIS_CFD;
                    m_SSTagDO[cnt].m_LineColor = vec3d( 0.4, 0.4, 0.4 );
                }
                else // Tags only
                {
                    m_SSTagDO[cnt].m_Type = DrawObj::VSP_SHADED_TRIS;
                }
            }

            if ( GetStructSettingsPtr()->m_ColorTagsFlag.Get() )
            {
                // Color sequence -- go around color wheel ncstep times with slight
                // offset from ncgrp basic colors.
                // Note, (cnt/ncgrp) uses integer division resulting in floor.
                double deg = ( ( cnt % ncgrp ) * ncstep + ( cnt / ncgrp ) ) * nctodeg;
                vec3d rgb = m_SSTagDO[cnt].ColorWheel( deg );
                rgb.normalize();

                for ( int i = 0; i < 3; i++ )
                {
                    m_SSTagDO[cnt].m_MaterialInfo.Ambient[i] = (float)rgb.v[i] / 5.0f;
                    m_SSTagDO[cnt].m_MaterialInfo.Diffuse[i] = 0.4f + (float)rgb.v[i] / 10.0f;
                    m_SSTagDO[cnt].m_MaterialInfo.Specular[i] = 0.04f + 0.7f * (float)rgb.v[i];
                    m_SSTagDO[cnt].m_MaterialInfo.Emission[i] = (float)rgb.v[i] / 20.0f;
                }
                m_SSTagDO[cnt].m_MaterialInfo.Ambient[3] = 1.0f;
                m_SSTagDO[cnt].m_MaterialInfo.Diffuse[3] = 1.0f;
                m_SSTagDO[cnt].m_MaterialInfo.Specular[3] = 1.0f;
                m_SSTagDO[cnt].m_MaterialInfo.Emission[3] = 1.0f;

                m_SSTagDO[cnt].m_MaterialInfo.Shininess = 32.0f;
            }
            else
            {
                // No color needed for mesh only.
            }

            draw_obj_vec.push_back( &m_SSTagDO[cnt] );
            cnt++;
        }

        for ( int i = 0; i < (int)m_SurfVec.size(); i++ )
        {
            vector < vec3d > pVec = m_SurfVec[i]->GetMesh()->GetSimpPntVec();
            vector < SimpTri > tVec = m_SurfVec[i]->GetMesh()->GetSimpTriVec();

            for ( int t = 0; t < (int)tVec.size(); t++ )
            {
                if ( ( !m_SurfVec[i]->GetWakeFlag() ) && ( !m_SurfVec[i]->GetFarFlag() ) && ( !m_SurfVec[i]->GetSymPlaneFlag() ) )
                {
                    SimpTri* stri = &tVec[t];
                    dmit = tag_dobj_map.find( SubSurfaceMgr.GetTag( stri->m_Tags ) );
                    if ( dmit == tag_dobj_map.end() )
                    {
                        continue;
                    }

                    DrawObj* obj = dmit->second;
                    vec3d norm = cross( pVec[stri->ind1] - pVec[stri->ind0], pVec[stri->ind2] - pVec[stri->ind0] );
                    norm.normalize();
                    obj->m_PntVec.push_back( pVec[stri->ind0] );
                    obj->m_PntVec.push_back( pVec[stri->ind1] );
                    obj->m_PntVec.push_back( pVec[stri->ind2] );
                    obj->m_NormVec.push_back( norm );
                    obj->m_NormVec.push_back( norm );
                    obj->m_NormVec.push_back( norm );
                }
            }
        }

        //m_SkinElemDO.m_GeomID = string( "FEASkin" );
        //m_SkinElemDO.m_Type = DrawObj::VSP_HIDDEN_TRIS_CFD;
        //m_SkinElemDO.m_Visible = true;
        //m_SkinElemDO.m_LineColor = vec3d( 0.4, 0.4, 0.4 );
        //m_SkinElemDO.m_PntVec.clear();
        //m_SkinElemDO.m_NormVec.clear();

        //m_SkinElemDO.m_FlipNormals = true;  // Display skin tris backwards for internal visibility.

        ////==== Draw Skin ====//
        //for ( int j = 0; j < m_FeaSkinElementVec.size(); j++ )
        //{
        //    FeaElement* fe = m_FeaSkinElementVec[j];

        //    for ( int p = 0; p < (int)fe->m_Corners.size(); p++ )
        //    {
        //        m_SkinElemDO.m_PntVec.push_back( fe->m_Corners[p]->m_Pnt );
        //        m_SkinElemDO.m_NormVec.push_back( vec3d( 0, 0, 0 ) );
        //    }
        //}

        //draw_obj_vec.push_back( &m_SkinElemDO );

        //m_SliceElemDO.m_GeomID = string( "FEASlice" );
        //m_SliceElemDO.m_Type = DrawObj::VSP_HIDDEN_QUADS;
        //m_SliceElemDO.m_Visible = true;
        //m_SliceElemDO.m_LineColor = vec3d( 0.4, 0.4, 0.4 );
        //m_SliceElemDO.m_PntVec.clear();
        //m_SliceElemDO.m_NormVec.clear();

        //for ( int i = 0; i < (int)m_FeaPartElementVec.size(); i++ )
        //{
        //    FeaElement* fe = m_FeaPartElementVec[i];
        //    int p;
        //    for ( p = 0; p < (int)fe->m_Corners.size(); p++ )
        //    {
        //        m_SliceElemDO.m_PntVec.push_back( fe->m_Corners[p]->m_Pnt );
        //        m_SliceElemDO.m_NormVec.push_back( vec3d( 0, 0, 0 ) );
        //    }
        //    // Handle degenerate quads (triangles).
        //    while ( p < 4 ) // Repeat first point until we have 4 points.
        //    {
        //        m_SliceElemDO.m_PntVec.push_back( fe->m_Corners[0]->m_Pnt );
        //        m_SliceElemDO.m_NormVec.push_back( vec3d( 0, 0, 0 ) );
        //        p++;
        //    }
        //}

        //draw_obj_vec.push_back( &m_SliceElemDO );

    }
}

//void FeaMeshMgrSingleton::BuildFeaPartIntChains()
//{
//    // Adds subsurface intersection chains
//    vec2d uw_pnt0;
//    vec2d uw_pnt1;
//    int num_sects = 100; // Number of segments to break FeaParts up into
//
//    StructureMgr.PrepareToSplit(); // Prepare All FeaParts for Split
//    for ( int s = 0; s < (int)m_SurfVec.size(); s++ )
//    {
//        // Get all FeaParts for the specified geom
//        Surf* surf = m_SurfVec[s];
//        vector< FeaPart* > fea_vec = StructureMgr.GetFeaStructs( surf->GetGeomID(), surf->GetMainSurfID() );
//
//        // Split SubSurfs
//        for ( int f = 0; f < (int)fea_vec.size(); f++ )
//        {
//            fea_vec[f]->SplitSegsU( surf->GetSurfCore()->GetMinU() );
//            fea_vec[f]->SplitSegsU( surf->GetSurfCore()->GetMaxU() );
//            fea_vec[f]->SplitSegsW( surf->GetSurfCore()->GetMinW() );
//            fea_vec[f]->SplitSegsW( surf->GetSurfCore()->GetMaxW() );
//
//            //vector< SSLineSeg >& segs = ss_vec[ss]->GetSplitSegs();
//            ISegChain* chain = NULL;
//
//            bool new_chain = true;
//            bool is_poly = fea_vec[f]->GetPolyFlag();
//
//            // Build Intersection Chains
//            for ( int ls = 0; ls < (int)segs.size(); ls++ )
//            {
//                if ( new_chain && chain )
//                {
//                    if ( chain->Valid() )
//                    {
//                        m_ISegChainList.push_back( chain );
//                    }
//                    else
//                    {
//                        delete chain;
//                        chain = NULL;
//                    }
//                }
//
//                if ( new_chain )
//                {
//                    chain = new ISegChain;
//                    chain->m_SurfA = surf;
//                    chain->m_SurfB = surf;
//                    if ( !is_poly )
//                    {
//                        new_chain = false;
//                    }
//                }
//
//                SSLineSeg l_seg = segs[ls];
//                vec3d lp0, lp1;
//
//                lp0 = l_seg.GetP0();
//                lp1 = l_seg.GetP1();
//                uw_pnt0 = vec2d( lp0.x(), lp0.y() );
//                uw_pnt1 = vec2d( lp1.x(), lp1.y() );
//                double max_u, max_w, tol;
//                double min_u, min_w;
//                tol = 1e-6;
//                min_u = surf->GetSurfCore()->GetMinU();
//                min_w = surf->GetSurfCore()->GetMinW();
//                max_u = surf->GetSurfCore()->GetMaxU();
//                max_w = surf->GetSurfCore()->GetMaxW();
//
//                if ( uw_pnt0[0] < min_u || uw_pnt0[1] < min_w || uw_pnt1[0] < min_u || uw_pnt1[1] < min_w )
//                {
//                    new_chain = true;
//                    continue; // Skip if either point has a value not on this surface
//                }
//                if ( uw_pnt0[0] > max_u || uw_pnt0[1] > max_w || uw_pnt1[0] > max_u || uw_pnt1[1] > max_w )
//                {
//                    new_chain = true;
//                    continue; // Skip if either point has a value not on this surface
//                }
//                if ( ( ( fabs( uw_pnt0[0] - max_u ) < tol && fabs( uw_pnt1[0] - max_u ) < tol ) ||
//                    ( fabs( uw_pnt0[1] - max_w ) < tol && fabs( uw_pnt1[1] - max_w ) < tol ) ||
//                     ( fabs( uw_pnt0[0] - min_u ) < tol && fabs( uw_pnt1[0] - min_u ) < tol ) ||
//                     ( fabs( uw_pnt0[1] - min_w ) < tol && fabs( uw_pnt1[1] - min_w ) < tol ) )
//                     && is_poly )
//                {
//                    new_chain = true;
//                    continue; // Skip if both end points are on the same edge of the surface
//                }
//
//                double delta_u = ( uw_pnt1[0] - uw_pnt0[0] ) / num_sects;
//                double delta_w = ( uw_pnt1[1] - uw_pnt0[1] ) / num_sects;
//
//                vector< vec2d > uw_pnts;
//                uw_pnts.resize( num_sects + 1 );
//                uw_pnts[0] = uw_pnt0;
//                uw_pnts[num_sects] = uw_pnt1;
//
//                // Add additional points between the segment endpoints to hopefully make the curve planar with the surface
//                for ( int p = 1; p < num_sects; p++ )
//                {
//                    uw_pnts[p] = vec2d( uw_pnt0[0] + delta_u * p, uw_pnt0[1] + delta_w * p );
//                }
//
//                for ( int p = 1; p < (int)uw_pnts.size(); p++ )
//                {
//                    Puw* puwA0 = new Puw( surf, uw_pnts[p - 1] );
//                    Puw* puwA1 = new Puw( surf, uw_pnts[p] );
//                    Puw* puwB0 = new Puw( surf, uw_pnts[p - 1] );
//                    Puw* puwB1 = new Puw( surf, uw_pnts[p] );
//
//                    m_DelPuwVec.push_back( puwA0 );         // Save to delete later
//                    m_DelPuwVec.push_back( puwA1 );
//                    m_DelPuwVec.push_back( puwB0 );
//                    m_DelPuwVec.push_back( puwB1 );
//
//                    IPnt* p0 = new IPnt( puwA0, puwB0 );
//                    IPnt* p1 = new IPnt( puwA1, puwB1 );
//
//                    m_DelIPntVec.push_back( p0 );           // Save to delete later
//                    m_DelIPntVec.push_back( p1 );
//
//                    p0->CompPnt();
//                    p1->CompPnt();
//
//                    ISeg* seg = new ISeg( surf, surf, p0, p1 );
//                    chain->m_ISegDeque.push_back( seg );
//                }
//            }
//            if ( chain )
//            {
//                if ( chain->Valid() )
//                {
//                    m_ISegChainList.push_back( chain );
//                }
//                else
//                {
//                    delete chain;
//                    chain = NULL;
//                }
//            }
//        }
//    }
//}
