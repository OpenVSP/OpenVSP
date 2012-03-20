//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//

// FeaMeshMgr
//
//////////////////////////////////////////////////////////////////////

#include "FeaMeshMgr.h"
#include "feaStructScreen.h"
#include "screenMgr.h"
#include "geom.h"
#include "aircraft.h"
#include "util.cpp"			// To get the templates to work??

//==== Singleton ====//
FEAM_Single::FEAM_Single()
{
	static FeaMeshMgr* ptr = 0; 
    if (!ptr) 
		ptr = new FeaMeshMgr();
	feaMeshMgr = ptr;     
}

//=============================================================//
//=============================================================//
WingSection::WingSection()
{
	m_UpperSurfPtr = NULL;
	m_LowerSurfPtr = NULL;
}

WingSection::~WingSection()
{
}


void WingSection::BuildClean()
{
	if ( m_UpperSurfPtr )
		m_UpperSurfPtr->BuildClean();
	if ( m_LowerSurfPtr )
		m_LowerSurfPtr->BuildClean();
}

void WingSection::WriteData( xmlNodePtr root )
{
	int i;

	//===== Ribs =====//
	xmlNodePtr rib_list_node = xmlNewChild( root, NULL, (const xmlChar *)"Rib_List", NULL );
	for ( i = 0 ; i < (int)m_RibVec.size() ; i++ )
	{
		xmlNodePtr rib_node = xmlNewChild( rib_list_node, NULL, (const xmlChar *)"Rib", NULL );
		xmlAddIntNode( rib_node, "SectionID",      m_RibVec[i]->m_SectID );
		xmlAddDoubleNode( rib_node, "PerSpan",     m_RibVec[i]->m_PerSpan );
		xmlAddDoubleNode( rib_node, "Sweep",     m_RibVec[i]->m_Sweep );
		xmlAddIntNode( rib_node, "AbsSweepFlag",   m_RibVec[i]->m_AbsSweepFlag );
		xmlAddIntNode( rib_node, "TrimFlag",   m_RibVec[i]->m_TrimFlag );
		xmlAddDoubleNode( rib_node, "Thick",     m_RibVec[i]->m_Thick );
		xmlAddDoubleNode( rib_node, "Density",     m_RibVec[i]->GetDensity() );
	}

	//===== Spars =====//
	xmlNodePtr spar_list_node = xmlNewChild( root, NULL, (const xmlChar *)"Spar_List", NULL );
	for ( i = 0 ; i < (int)m_SparVec.size() ; i++ )
	{
		xmlNodePtr spar_node = xmlNewChild( spar_list_node, NULL, (const xmlChar *)"Spar", NULL );
		xmlAddIntNode( spar_node, "SectionID",      m_SparVec[i]->m_SectID );
		xmlAddDoubleNode( spar_node, "PerChord",     m_SparVec[i]->m_PerChord );
		xmlAddDoubleNode( spar_node, "Sweep",     m_SparVec[i]->m_Sweep );
		xmlAddIntNode( spar_node, "AbsSweepFlag",   m_SparVec[i]->m_AbsSweepFlag );
		xmlAddIntNode( spar_node, "TrimFlag",   m_SparVec[i]->m_TrimFlag );
		xmlAddDoubleNode( spar_node, "Thick",     m_SparVec[i]->m_Thick );
		xmlAddDoubleNode( spar_node, "Density",     m_SparVec[i]->GetDensity() );
	}

	//===== Skins =====//
	xmlNodePtr upper_skin_node = xmlNewChild( root, NULL, (const xmlChar *)"Upper_Skin", NULL );
	m_UpperSkin.WriteData( upper_skin_node );

	xmlNodePtr lower_skin_node = xmlNewChild( root, NULL, (const xmlChar *)"Lower_Skin", NULL );
	m_LowerSkin.WriteData( lower_skin_node );


}

void WingSection::ReadData( xmlNodePtr root )
{
	//===== Ribs =====//
	for ( int i = 0 ; i < (int)m_RibVec.size() ; i++ )
		delete m_RibVec[i];
	m_RibVec.clear();

	xmlNodePtr rib_list_node = xmlGetNode( root, "Rib_List", 0 );
	if ( rib_list_node )
	{
		int num_ribs =  xmlGetNumNames( rib_list_node, "Rib" );
		for ( int i = 0 ; i < num_ribs ; i++ )
		{
			xmlNodePtr rib_node = xmlGetNode( rib_list_node, "Rib", i );
			if ( rib_node )
			{
				FeaRib* rib = new FeaRib();
				rib->m_SectID = xmlFindInt( rib_node, "SectionID", 0 );
				rib->m_PerSpan = xmlFindDouble( rib_node, "PerSpan", 0.5 );
				rib->m_Sweep   = xmlFindDouble( rib_node, "Sweep", 0.0 );
				rib->m_AbsSweepFlag = (xmlFindInt( rib_node, "AbsSweepFlag", 1 ) != 0);	// Bool
				rib->m_TrimFlag     = (xmlFindInt( rib_node, "TrimFlag", 1 ) != 0);
				rib->m_Thick   = xmlFindDouble( rib_node, "Thick", 0.1 );
				rib->SetDensity( xmlFindDouble( rib_node, "Density", 1.0 ) );
				m_RibVec.push_back( rib );
			}
		}
	}

	//===== Spars =====//
	for ( int i = 0 ; i < (int)m_SparVec.size() ; i++ )
		delete m_SparVec[i];
	m_SparVec.clear();

	xmlNodePtr spar_list_node = xmlGetNode( root, "Spar_List", 0 );
	if ( spar_list_node )
	{
		int num_spars =  xmlGetNumNames( spar_list_node, "Spar" );
		for ( int i = 0 ; i < num_spars ; i++ )
		{
			xmlNodePtr spar_node = xmlGetNode( spar_list_node, "Spar", i );
			if ( spar_node )
			{
				FeaSpar* spar = new FeaSpar();
				spar->m_SectID = xmlFindInt( spar_node, "SectionID", 0 );
				spar->m_PerChord = xmlFindDouble( spar_node, "PerChord", 0.5 );
				spar->m_Sweep   = xmlFindDouble( spar_node, "Sweep", 0.0 );
				spar->m_AbsSweepFlag = (xmlFindInt( spar_node, "AbsSweepFlag", 1 ) != 0);
				spar->m_TrimFlag     = (xmlFindInt( spar_node, "TrimFlag", 1 )!= 0);
				spar->m_Thick   = xmlFindDouble( spar_node, "Thick", 0.1 );
				spar->SetDensity( xmlFindDouble( spar_node, "Density", 1.0 ) );
				m_SparVec.push_back( spar );
			}
		}
	}

	//===== Skins =====//
	xmlNodePtr upper_skin_node = xmlGetNode( root, "Upper_Skin", 0 );
	if ( upper_skin_node )
		m_UpperSkin.ReadData( upper_skin_node );

	xmlNodePtr lower_skin_node = xmlGetNode( root, "Lower_Skin", 0 );
	if ( lower_skin_node )
		m_LowerSkin.ReadData( lower_skin_node );

}


void WingSection::Load( Surf* upper_surf, Surf* lower_surf )
{
	int i, e;
	vec2d uw;
	vec3d pnt;

	m_UpperSurfPtr = upper_surf;
	m_LowerSurfPtr = lower_surf;

	m_UpperSkin.SetSurf( upper_surf );
	m_UpperSkin.SetWingSection( this );
	m_LowerSkin.SetSurf( lower_surf );
	m_LowerSkin.SetWingSection( this );

	//==== Load Corner Pnts ====//
	m_CornerPnts[UW00] = upper_surf->CompPnt01( 0, 0 );	// Inner TE
	m_CornerPnts[UW10] = upper_surf->CompPnt01( 1, 0 );  // Outer TE
	m_CornerPnts[UW01] = upper_surf->CompPnt01( 0, 1 );	// Inner LE
	m_CornerPnts[UW11] = upper_surf->CompPnt01( 1, 1 );  // Outer LE

	//==== Compute Section Normal =====//
	vec3d vchd = m_CornerPnts[UW00] - m_CornerPnts[UW01];
	vec3d vle  = m_CornerPnts[UW11] - m_CornerPnts[UW01];
	m_Normal = cross( vchd, vle );
	m_Normal.normalize();

	//==== Find Normal to Chord Line ====//
	m_ChordNormal = cross( m_Normal, vchd  );
	m_ChordNormal.normalize();

	//==== Find Sweep Angle of LE ====//
	m_SweepLE = RAD_2_DEG*signed_angle( m_ChordNormal, vle, m_Normal*-1.0 );

	//==== Load UW Pnts ====//
	int num_pnts = 101;
	for ( i = 0 ; i < num_pnts ; i++ )
	{
		double fract = (double)i/(double)(num_pnts-1);
		m_Edges[LE].m_UWVec.push_back( vec2d( fract, 1 ) );
		m_Edges[TE].m_UWVec.push_back( vec2d( fract, 0 ) );
		m_Edges[IN_CHORD].m_UWVec.push_back( vec2d( 0, fract ) );
		m_Edges[OUT_CHORD].m_UWVec.push_back( vec2d( 1, fract ) );
	}

	//==== Load 3D Pnts ====//
	for ( e = 0 ; e < NUM_EDGES ; e++ )
	{
		for ( i = 0 ; i < num_pnts ; i++ )
		{
			uw  = m_Edges[e].m_UWVec[i];
			pnt = upper_surf->CompPnt01( uw[0], uw[1] );
			m_Edges[e].m_PntVec.push_back( pnt );
		}
	}

	//==== Load Line Proj Dist Fract ====//
	for ( e = 0 ; e < NUM_EDGES ; e++ )
	{
		vec3d lp0 = m_Edges[e].m_PntVec[0];
		vec3d lp1 = m_Edges[e].m_PntVec.back();
		double line_dist = dist( lp0, lp1 );
		for ( i = 0 ; i < num_pnts ; i++ )
		{
			vec3d proj_pnt = proj_pnt_on_line( lp0, lp1, m_Edges[e].m_PntVec[i] );
			double ldf = dist( proj_pnt, lp0 );
			if ( line_dist > DBL_EPSILON )
				ldf = ldf/line_dist;

			m_Edges[e].m_LineFractVec.push_back( ldf );
		}
	}
}

vec2d WingSection::GetUW( int edge_id, double fract )
{
	assert( edge_id >= 0 && edge_id < NUM_EDGES );

	SectionEdge* se = &m_Edges[edge_id];
	assert( se->m_LineFractVec.size() >= 2 );

	if ( fract <= 0.0 )	return se->m_UWVec[0];

	for ( int i = 0 ; i < (int)se->m_LineFractVec.size()-1 ; i++ )
	{
		if ( fract >= se->m_LineFractVec[i] && fract <= se->m_LineFractVec[i+1] )
		{
			double denom = se->m_LineFractVec[i+1] - se->m_LineFractVec[i];
			double uw_fract = 0.0;
			if (denom > DBL_EPSILON)
				uw_fract = (fract - se->m_LineFractVec[i])/denom;

			vec2d uw = se->m_UWVec[i] + (se->m_UWVec[i+1] - se->m_UWVec[i])*uw_fract;
			return uw;
		}
	}
	return se->m_UWVec.back();
}

vec3d WingSection::CompPnt( int edge_id, double fract )
{
	vec2d uw = GetUW( edge_id, fract );
	vec3d p = m_UpperSurfPtr->CompPnt01( uw[0], uw[1] );
	return p;
}
	
bool WingSection::IntersectPlaneEdge( int edge_id, vec3d & orig, vec3d & norm, vec2d & result )
{
	assert( edge_id >= 0 && edge_id < NUM_EDGES );

	SectionEdge* se = &m_Edges[edge_id];
	assert( se->m_PntVec.size() >= 2 );

	for ( int i = 0 ; i < (int)se->m_PntVec.size()-1 ; i++ )
	{
		int side1 = plane_half_space( orig, norm, se->m_PntVec[i] );
		int side2 = plane_half_space( orig, norm, se->m_PntVec[i+1] );

		if ( side1 != side2 )
		{
			double t;
			vec3d rayvec = se->m_PntVec[i+1] - se->m_PntVec[i];
			int valid = plane_ray_intersect( orig, norm, se->m_PntVec[i], rayvec, t );
			if ( valid )
			{
				result = se->m_UWVec[i] + ( se->m_UWVec[i+1] - se->m_UWVec[i] )*t;
				return true;
			}
		}
	}
	return false;
}

void WingSection::ComputePerSpanChord( vec3d & pnt, double* per_span, double* per_chord )
{
	//==== Find Distance to Inside  Chord ====//
	double d = dist_pnt_2_line( m_CornerPnts[UW01], m_CornerPnts[UW00], pnt );
	double denom = dist_pnt_2_line( m_CornerPnts[UW01], m_CornerPnts[UW00], m_CornerPnts[UW11] );

	*per_span = 0.0;
	if ( denom > DBL_EPSILON )
		*per_span = d/denom;

	//==== Find Intersection of Projected Chordline ===//
	vec3d dir = m_CornerPnts[UW01] - m_CornerPnts[UW00];
	vec3d poff = pnt + dir;

	double s, t;
	bool valid = line_line_intersect( pnt, poff, m_CornerPnts[UW01], m_CornerPnts[UW11], &s, &t );
	vec3d ple = m_CornerPnts[UW01];
	if ( valid )
		ple = m_CornerPnts[UW01] + (m_CornerPnts[UW11] - m_CornerPnts[UW01])*t;


	valid = line_line_intersect( pnt, poff, m_CornerPnts[UW00], m_CornerPnts[UW10], &s, &t );
	vec3d pte = m_CornerPnts[UW00];
	if ( valid )
		pte = m_CornerPnts[UW00] + (m_CornerPnts[UW10] - m_CornerPnts[UW00])*t;

	d = dist( ple, pnt );
	denom = dist( ple, pte );
	*per_chord = 0.0;
	if ( denom > DBL_EPSILON )
		*per_chord = d/denom;

}

void WingSection::Draw( bool highlight )
{
	//if ( highlight )
	//{
	//	glColor3ub( 255, 0, 255 );
	//	glLineStipple( 1, 0x000F );
	//	glLineWidth( 2.0 );
	//	glEnable(GL_LINE_STIPPLE);
	//}
	//else
	//{
	//	glColor3ub( 150, 150, 150 );
	//	glLineStipple( 1, 0x000F );
	//	glLineWidth( 1.0 );
	//	glEnable(GL_LINE_STIPPLE);
	//}

	//glBegin( GL_LINE_LOOP );

	//glVertex3dv( m_CornerPnts[UW00].data() );
	//glVertex3dv( m_CornerPnts[UW10].data() );
	//glVertex3dv( m_CornerPnts[UW11].data() );
	//glVertex3dv( m_CornerPnts[UW01].data() );

	//glEnd();
	//glDisable(GL_LINE_STIPPLE);

	if ( highlight )
	{
		glColor4ub( 220, 220, 220, 240 );
	}
	else
	{
		glColor4ub( 170, 170, 170, 240 );
	}

	glBegin( GL_POLYGON );

	glVertex3dv( m_CornerPnts[UW00].data() );
	glVertex3dv( m_CornerPnts[UW10].data() );
	glVertex3dv( m_CornerPnts[UW11].data() );
	glVertex3dv( m_CornerPnts[UW01].data() );

	glEnd();

}

//=============================================================//
//=============================================================//

FeaMeshMgr::FeaMeshMgr()
{
	m_BatchFlag = false;
	m_CurrSectID = 0;
	m_CurrRibID = 0;
	m_CurrSparID = 0;
	m_CurrPointMassID = 0;
	m_DefElemSize = 0.5;
	m_ThickScale = 1.0;
	m_TotalMass = 0.0;
	m_CurrEditType = UP_SKIN_EDIT;
	m_DrawFlag = false;
	m_DrawMeshFlag = false;
	m_WingGeom = NULL;
	m_XmlDataNode = NULL;
	m_DrawAttachPoints = false;
	m_ClosestAttachPoint = -1;
}

FeaMeshMgr::~FeaMeshMgr()
{
	CleanUp();
}

void FeaMeshMgr::UpdateGUI()
{
	if ( !m_BatchFlag )
		aircraftPtr->getScreenMgr()->getFeaStructScreen()->update(); 
}

void FeaMeshMgr::AircraftDraw()
{
	aircraftPtr->triggerDraw();
}

void FeaMeshMgr::CleanUp()
{
	m_UpperSurfVec.clear();
	m_LowerSurfVec.clear();
	m_WingSections.clear();

	CfdMeshMgr::CleanUp();
}

Stringc FeaMeshMgr::GetFeaExportFileName( int type )
{
	if ( type >= 0 && type < NUM_FEA_FILE_NAMES )
		return m_ExportFeaFileNames[type];

	return Stringc();
}

void FeaMeshMgr::SetFeaExportFileName( const char* fn, int type )
{
	if ( type >= 0 && type < NUM_FEA_FILE_NAMES )
		m_ExportFeaFileNames[type] = Stringc(fn);
}

bool FeaMeshMgr::GetFeaExportFileFlag( int type )
{
	if ( type >= 0 && type < NUM_FEA_FILE_NAMES )
		return m_ExportFeaFileFlags[type];

	return false;
}

void FeaMeshMgr::SetFeaExportFileFlag( bool flag, int type )
{
	if ( type >= 0 && type < NUM_FEA_FILE_NAMES )
		m_ExportFeaFileFlags[type] = flag;
}

void FeaMeshMgr::ResetFeaExportFileNames()
{
	int pos;
	char *suffix[]={"_mass.dat","_NASTRAN.dat","_calculix_geom.dat","_calculix_thick.dat", ".stl" };

	for ( int i = 0 ; i < NUM_FEA_FILE_NAMES ; i++ )
	{
		m_ExportFeaFileNames[i] = aircraftPtr->getFileName();
		pos = m_ExportFeaFileNames[i].search_for_substring(".vsp");
		if (pos >= 0)
			m_ExportFeaFileNames[i].delete_range(pos, m_ExportFeaFileNames[i].get_length()-1);
		m_ExportFeaFileNames[i].concatenate(suffix[i]);
	}
}

void FeaMeshMgr::SaveData()
{
	if (!m_WingGeom )
		return;

	if ( m_XmlDataNode )
		xmlFreeNode(m_XmlDataNode);

	m_XmlDataNode = xmlNewNode( NULL, (const xmlChar *)"FEA_Struct_Data" );
	if (!m_XmlDataNode)
		return;

	WriteFeaStructData( m_WingGeom, m_XmlDataNode );
}

void FeaMeshMgr::WriteFeaStructData( Geom* geom_ptr, xmlNodePtr root )
{
	if ( geom_ptr != m_WingGeom )
		return;

	bool found = false;
	for ( int i = 0 ; i < (int)m_DataGeomVec.size() ; i++ )
	{
		if ( m_WingGeom == m_DataGeomVec[i] )
			found = true;
	}
	if ( !found )
		m_DataGeomVec.push_back( m_WingGeom );

	xmlNodePtr fea_node = xmlNewChild( root, NULL, (const xmlChar *)"FEA_Structure_Parms", NULL );

	xmlAddDoubleNode( fea_node, "DefElemSize", m_DefElemSize );

        xmlAddDoubleNode( fea_node, "FEA_Mesh_Min_Length", m_GridDensity.GetMinLen() );
        xmlAddDoubleNode( fea_node, "FEA_Mesh_Max_Gap", m_GridDensity.GetMaxGap() );
        xmlAddDoubleNode( fea_node, "FEA_Mesh_Num_Circle_Segments", m_GridDensity.GetNCircSeg() );

	xmlAddDoubleNode( fea_node, "ThickScale",  m_ThickScale );

	xmlNodePtr sec_node;
	xmlNodePtr sec_list_node = xmlNewChild( fea_node, NULL, (const xmlChar *)"Wing_Section_List", NULL );

	for ( int i = 0 ; i < (int)m_WingSections.size() ; i++ )
	{
		sec_node = xmlNewChild( sec_list_node, NULL, (const xmlChar *)"Wing_Section", NULL );		
		m_WingSections[i].WriteData( sec_node );
	}

	//===== Point Masses ====//
	xmlNodePtr point_mass_list_node = xmlNewChild( fea_node, NULL, (const xmlChar *)"Point_Mass_List", NULL );
	for ( int i = 0 ; i < (int)m_PointMassVec.size() ; i++ )
	{
		xmlNodePtr point_mass_node = xmlNewChild( point_mass_list_node, NULL, (const xmlChar *)"Point_Mass", NULL );
		xmlAddDoubleNode( point_mass_node, "Pos_X", m_PointMassVec[i]->m_Pos.x() );
		xmlAddDoubleNode( point_mass_node, "Pos_Y", m_PointMassVec[i]->m_Pos.y() );
		xmlAddDoubleNode( point_mass_node, "Pos_Z", m_PointMassVec[i]->m_Pos.z() );
		xmlAddDoubleNode( point_mass_node, "Attach_X", m_PointMassVec[i]->m_AttachPos.x() );
		xmlAddDoubleNode( point_mass_node, "Attach_Y", m_PointMassVec[i]->m_AttachPos.y() );
		xmlAddDoubleNode( point_mass_node, "Attach_Z", m_PointMassVec[i]->m_AttachPos.z() );
	}

}

void FeaMeshMgr::SetFeaStructData( Geom* geom_ptr, xmlNodePtr root )
{
	int numFEAStructureNodes = xmlGetNumNames( root, "FEA_Structure_Parms" );

	if ( numFEAStructureNodes == 0 )
		return;

	m_DataGeomVec.push_back( geom_ptr );
	m_XmlDataNode = xmlCopyNode( root, 100 );
}

void FeaMeshMgr::CopyGeomPtr( Geom* from_geom, Geom* to_geom )
{
	bool found = false;
	for ( int i = 0 ; i < (int)m_DataGeomVec.size() ; i++ )
		if ( from_geom == m_DataGeomVec[i] )
			found = true;

	if ( found )
		m_DataGeomVec.push_back( to_geom );
}

void FeaMeshMgr::ReadFeaStructData()
{
	bool found = false;
	for ( int i = 0 ; i < (int)m_DataGeomVec.size() ; i++ )
	{
		if ( m_WingGeom == m_DataGeomVec[i] )
			found = true;
	}

	if ( !found ) return;

	int numFEAStructureNodes = xmlGetNumNames( m_XmlDataNode, "FEA_Structure_Parms" );

	if ( numFEAStructureNodes == 0 )
		return;

	xmlNodePtr struct_parms_node = xmlGetNode( m_XmlDataNode, "FEA_Structure_Parms", 0 );

	m_DefElemSize = xmlFindDouble( struct_parms_node, "DefElemSize", m_DefElemSize );

	m_GridDensity.SetMinLen( xmlFindDouble( struct_parms_node, "FEA_Mesh_Min_Length", m_GridDensity.GetMinLen() ) );
	m_GridDensity.SetMaxGap( xmlFindDouble( struct_parms_node, "FEA_Mesh_Max_Gap", m_GridDensity.GetMaxGap() ) );
	m_GridDensity.SetNCircSeg( xmlFindDouble( struct_parms_node, "FEA_Mesh_Num_Circle_Segments", m_GridDensity.GetNCircSeg() ) );

	m_ThickScale  = xmlFindDouble( struct_parms_node, "ThickScale", m_ThickScale );

	xmlNodePtr wing_sec_list_node = xmlGetNode( struct_parms_node, "Wing_Section_List", 0 );
	if ( wing_sec_list_node )
	{
		int num_wing_sec =  xmlGetNumNames( wing_sec_list_node, "Wing_Section" );
		if ( num_wing_sec == (int)m_WingSections.size() )
		{
			for ( int i = 0 ; i < (int)m_WingSections.size() ; i++ )
			{
				xmlNodePtr sec_node = xmlGetNode( wing_sec_list_node, "Wing_Section", i );
				m_WingSections[i].ReadData( sec_node );
			}
		}
	}

	//==== Compute Rib and Spar End Points ====//
	for ( int i = 0 ; i < (int)m_WingSections.size() ; i++ )
	{
		for ( int j = 0 ; j < (int)m_WingSections[i].m_RibVec.size() ; j++ )
				m_WingSections[i].m_RibVec[j]->ComputeEndPoints();
		for ( int j = 0 ; j < (int)m_WingSections[i].m_SparVec.size() ; j++ )
				m_WingSections[i].m_SparVec[j]->ComputeEndPoints();
	}

	xmlNodePtr point_mass_list_node = xmlGetNode( struct_parms_node, "Point_Mass_List", 0 );
	if ( point_mass_list_node )
	{
		int num_point_masses =  xmlGetNumNames( point_mass_list_node, "Point_Mass" );
		for ( int i = 0 ; i < num_point_masses ; i++ )
		{
			xmlNodePtr point_mass_node = xmlGetNode( point_mass_list_node, "Point_Mass", i );
			if ( point_mass_node )
			{
				FeaPointMass* pm = new FeaPointMass();
				pm->m_Pos.set_x( xmlFindDouble( point_mass_node, "Pos_X", 0.0 ) );
				pm->m_Pos.set_y( xmlFindDouble( point_mass_node, "Pos_Y", 0.0 ) );
				pm->m_Pos.set_z( xmlFindDouble( point_mass_node, "Pos_Z", 0.0 ) );
				pm->m_AttachPos.set_x( xmlFindDouble( point_mass_node, "Attach_X", 0.0 ) );
				pm->m_AttachPos.set_y( xmlFindDouble( point_mass_node, "Attach_Y", 0.0 ) );
				pm->m_AttachPos.set_z( xmlFindDouble( point_mass_node, "Attach_Z", 0.0 ) );
				m_PointMassVec.push_back( pm );
			}
		}
	}
}

bool FeaMeshMgr::LoadSurfaces()
{
	CleanUp();

	Stringc bezTempFile = aircraftPtr->getTempDir();
	bezTempFile.concatenate( Stringc( "feawing.bez" ) );

	if ( !WriteWingBezierFile( bezTempFile ) )
		return false;

	ReadSurfs( bezTempFile );
	IdentifyUpperLowerSurfaces();

	ReadFeaStructData();

	return true;
}

void FeaMeshMgr::BuildClean()
{
	//==== Delete ICurves =====//
	int i;
	for ( i = 0 ; i < (int)m_ICurveVec.size() ; i++ )
		delete m_ICurveVec[i];
	m_ICurveVec.clear();

	//==== Delete Seg Chains ====//
	list< ISegChain* >::iterator cl;
	for ( cl = m_ISegChainList.begin() ; cl != m_ISegChainList.end(); cl++ )
		delete (*cl);
	m_ISegChainList.clear();

	//==== Build Clean Wing Sections ====//
	for ( i = 0 ; i < (int)m_WingSections.size() ; i++ )
	{
		m_WingSections[i].BuildClean();
	}

	m_AttachPoints.clear();
	m_ClosestAttachPoint = -1;

}


void FeaMeshMgr::Build()
{
	if ( m_WingSections.size() <= 0 )
		return;

	BuildClean();

	if ( !m_BatchFlag )
		aircraftPtr->getScreenMgr()->getFeaStructScreen()->addOutputText( "Add Structure Parts\n" );
	AddStructureParts();

	m_GridDensity.ClearSources();
	m_GridDensity.SetBaseLen( m_DefElemSize );
	if ( !m_BatchFlag )
		aircraftPtr->getScreenMgr()->getFeaStructScreen()->addOutputText( "Build Slice Planes\n" );
	BuildGrid();

	if ( !m_BatchFlag )
		aircraftPtr->getScreenMgr()->getFeaStructScreen()->addOutputText( "Intersect\n" );
	Intersect();
	RemoveSliceSurfaces();

	if ( !m_BatchFlag )
		aircraftPtr->getScreenMgr()->getFeaStructScreen()->addOutputText( "InitMesh\n" );
	InitMesh();
	if ( !m_BatchFlag )
		aircraftPtr->getScreenMgr()->getFeaStructScreen()->addOutputText( "Mesh Skins\n" );

	if ( !m_BatchFlag )
		Remesh(CfdMeshMgr::FEA_OUTPUT);
	else
		Remesh(CfdMeshMgr::NO_OUTPUT);
	
	if ( !m_BatchFlag )
		aircraftPtr->getScreenMgr()->getFeaStructScreen()->addOutputText( "Build Spar/Rib Mesh\n" );
	BuildSliceMesh();

	LoadAttachPoints();

	if ( !m_BatchFlag )
		aircraftPtr->getScreenMgr()->getFeaStructScreen()->addOutputText( "Finished\n" );

}

void FeaMeshMgr::Export()
{
	if ( !m_BatchFlag )
		aircraftPtr->getScreenMgr()->getFeaStructScreen()->addOutputText( "Write Results\n" );

	WriteNASTRAN( m_ExportFeaFileNames[NASTRAN_FILE_NAME] );
	WriteCalculix();
	WriteSTL( m_ExportFeaFileNames[STL_FEA_NAME] );

	if ( !m_BatchFlag )
	{
		aircraftPtr->getScreenMgr()->getFeaStructScreen()->addOutputText( "Wrote Calculix File: feageom.dat\n" );
		aircraftPtr->getScreenMgr()->getFeaStructScreen()->addOutputText( "Wrote Calculix File: feanodethick.dat\n" );
		aircraftPtr->getScreenMgr()->getFeaStructScreen()->addOutputText( "Wrote NASTRAN File: NASTRAN.dat\n" );
		aircraftPtr->getScreenMgr()->getFeaStructScreen()->addOutputText( "Wrote Mesh: feamesh.stl\n" );
	}

	ComputeWriteMass();
	if ( !m_BatchFlag )
		aircraftPtr->getScreenMgr()->getFeaStructScreen()->addOutputText( "Wrote Mass: feamass.dat\n" );
	char str[256];
	sprintf( str, "Total Mass = %f\n", m_TotalMass );
	if ( !m_BatchFlag )
		aircraftPtr->getScreenMgr()->getFeaStructScreen()->addOutputText( str );
	if ( !m_BatchFlag )
		aircraftPtr->getScreenMgr()->getFeaStructScreen()->addOutputText( "Finished\n" );

}

void FeaMeshMgr::Intersect()
{
	for ( int i = 0 ; i < (int)m_SurfVec.size() ; i++ )
		for ( int j = i+1 ; j < (int)m_SurfVec.size() ; j++ )
		{
			m_SurfVec[i]->Intersect( m_SurfVec[j] );
		}
		
	BuildChains();
//DebugWriteChains("Intersect_UW", false );
	//if ( !m_BatchFlag )
	//	aircraftPtr->getScreenMgr()->getFeaStructScreen()->addOutputText( "Merge Chains\n" );
//	MergeChains();
//DebugWriteChains("MergeChains_UW", false );
	if ( !m_BatchFlag )
		aircraftPtr->getScreenMgr()->getFeaStructScreen()->addOutputText( "Load Border Curves\n" );
	LoadBorderCurves();

	if ( !m_BatchFlag )
		aircraftPtr->getScreenMgr()->getFeaStructScreen()->addOutputText( "Merge Interior Chains IPnts\n" );
	MergeInteriorChainIPnts();
//DebugWriteChains("MergeInterior_UW", false );

	if ( !m_BatchFlag )
		aircraftPtr->getScreenMgr()->getFeaStructScreen()->addOutputText( "Split Border Curves\n" );
	SplitBorderCurves();
//DebugWriteChains("BorderSplit_UW", false );

	if ( !m_BatchFlag )
		aircraftPtr->getScreenMgr()->getFeaStructScreen()->addOutputText( "Intersect Split Chains\n" );
	IntersectSplitChains();
//DebugWriteChains("IntersectSplit_UW", false );

	TessellateChains( &m_GridDensity );
//DebugWriteChains("Tess_UW", true );

	MergeBorderEndPoints();
//	DebugWriteChains( "chains.dat", true );

	Mesh();

//	RemoveInteriorTris();

	ConnectBorderEdges();
}


bool FeaMeshMgr::WriteWingBezierFile(const char* file_name)
{
	vector< Geom* > geomVec = aircraftPtr->getGeomVec();
	if ( geomVec.size() == 0 )
		return false;

	Geom* wing_geom = NULL;
	Geom* curr_geom = aircraftPtr->getActiveGeom();
	if ( curr_geom && curr_geom->getType() == MS_WING_GEOM_TYPE )
		wing_geom = curr_geom;
	else
	{
		for ( int i = 0 ; i < (int)geomVec.size() ; i++ )
		{
			if ( geomVec[i]->getType() == MS_WING_GEOM_TYPE )
			{
				wing_geom = geomVec[i];
				break;
			}
		}
	}

	if ( !wing_geom )
		return false;

	m_WingGeom = wing_geom;

	//==== Open file ====//
	FILE* file_id = fopen(file_name, "w");

	if ( !file_id )
		return false;

	fprintf( file_id, "%d  Num_Components\n", 1 );
	int saveSymCode = wing_geom->getSymCode();
	wing_geom->setSymCode( NO_SYM );
	wing_geom->write_bezier_file( 0, file_id );
	wing_geom->setSymCode( saveSymCode );

	fclose( file_id );
	return true;
}

void FeaMeshMgr::IdentifyUpperLowerSurfaces()
{
	m_UpperSurfVec.clear();
	for ( int i = 2 ; i < (int)m_SurfVec.size()-2 ; i+= 2 )
		m_UpperSurfVec.push_back( m_SurfVec[i] );

	m_LowerSurfVec.clear();
	for ( int i = 3 ; i < (int)m_SurfVec.size()-2 ; i+= 2 )
		m_LowerSurfVec.push_back( m_SurfVec[i] );

	for ( int i = 0 ; i < 2 ; i++ )
		delete m_SurfVec[i];
	for ( int i = m_SurfVec.size()-2 ; i < (int)m_SurfVec.size() ; i++ )
		delete m_SurfVec[i];

	m_SurfVec.clear();
	for ( int i = 0 ; i < (int)m_UpperSurfVec.size() ; i++ )
		m_SurfVec.push_back( m_UpperSurfVec[i] );
	for ( int i = 0 ; i < (int)m_LowerSurfVec.size() ; i++ )
		m_SurfVec.push_back( m_LowerSurfVec[i] );

	//==== Load Upper Section Edges =====//
	m_WingSections.resize( (int)m_UpperSurfVec.size() );
	for ( int i = 0 ; i < (int)m_UpperSurfVec.size() ; i++ )
	{
		m_WingSections[i].Load( m_UpperSurfVec[i], m_LowerSurfVec[i] );
	}

}

vec3d FeaMeshMgr::GetNormal( int sectID )
{
	if ( sectID >= 0 && sectID < (int)m_WingSections.size() )
		return m_WingSections[sectID].m_Normal;

	return vec3d(1.0, 0, 0);
}

WingSection* FeaMeshMgr::GetWingSection( int sectID )
{
	if ( sectID >= 0 && sectID < (int)m_WingSections.size() )
		return &m_WingSections[sectID];
	
	return NULL;
}

vec3d FeaMeshMgr::ComputePoint( vec2d & uw, bool upperFlag )
{
	vec3d pnt;
	if ( m_UpperSurfVec.size() == 0 )
		return pnt;

	int sid = (int)uw[0];
	double u = uw[0] - (double)sid;

	if ( sid < 0 )
	{
		u = 0.0;
		sid = 0;
	}		
	else if ( sid >= (int)m_UpperSurfVec.size() )
	{
		u = 1.0;
		sid = (int)m_UpperSurfVec.size()-1;
	}

	if ( upperFlag  )
		pnt = m_UpperSurfVec[sid]->CompPnt01( u, uw[1] );
	else 
		pnt = m_LowerSurfVec[sid]->CompPnt01( u, 1.0 - uw[1] );

	return pnt;
}



void FeaMeshMgr::AddStructureParts()
{
	//==== Clean Out Ribs, Spars, Skins  ====//
	for ( int s = 0 ; s < (int)m_WingSections.size() ; s++ )
	{
		for ( int i = 0 ; i < (int)m_WingSections[s].m_RibVec.size() ; i++ )
			m_WingSections[s].m_RibVec[i]->Clean();

		for ( int i = 0 ; i < (int)m_WingSections[s].m_SparVec.size() ; i++ )
			m_WingSections[s].m_SparVec[i]->Clean();

		m_WingSections[s].m_UpperSkin.Clean();
		m_WingSections[s].m_LowerSkin.Clean();
	}
	//==== Clear Pointer Vecs ====//
	m_SliceVec.clear();
	m_SurfVec.clear();
	m_SkinVec.clear();

	for ( int i = 0 ; i < (int)m_UpperSurfVec.size() ; i++ )
		m_SurfVec.push_back( m_UpperSurfVec[i] );
	for ( int i = 0 ; i < (int)m_LowerSurfVec.size() ; i++ )
		m_SurfVec.push_back( m_LowerSurfVec[i] );

	for ( int s = 0 ; s < (int)m_WingSections.size() ; s++ )
	{
		//===== Add Ribs ====//
		int num_ribs = m_WingSections[s].m_RibVec.size();
		for ( int i = 0 ; i < num_ribs ; i++ )
		{
			FeaRib* rib = m_WingSections[s].m_RibVec[i];
			bool capFlag = rib->IsCap();
			if ( !capFlag )
			{
				rib->ComputeEndPoints();
				m_SurfVec.push_back( rib->GetSurf() );
			}
			else
			{
				if ( s == 0 && rib->m_CapInFlag )
				{
					rib->SetUpperCapSurfs( m_UpperSurfVec[s], m_UpperSurfVec[s] );
					rib->SetLowerCapSurfs( m_LowerSurfVec[s], m_LowerSurfVec[s] );
				}
				else if ( s == m_WingSections.size()-1 && !rib->m_CapInFlag )
				{
					rib->SetUpperCapSurfs( m_UpperSurfVec[s], m_UpperSurfVec[s] );
					rib->SetLowerCapSurfs( m_LowerSurfVec[s], m_LowerSurfVec[s] );
				}
				else if ( rib->m_CapInFlag )
				{
					rib->SetUpperCapSurfs( m_UpperSurfVec[s-1], m_UpperSurfVec[s] );
					rib->SetLowerCapSurfs( m_LowerSurfVec[s-1], m_LowerSurfVec[s] );
				}
				else
				{
					rib->SetUpperCapSurfs( m_UpperSurfVec[s], m_UpperSurfVec[s+1] );
					rib->SetLowerCapSurfs( m_LowerSurfVec[s], m_LowerSurfVec[s+1] );
				}
			}
			rib->SetNormal( m_WingSections[s].m_Normal );
			m_SliceVec.push_back( rib );
		}

		//==== Add Spar ====//
		for ( int i = 0 ; i < (int)m_WingSections[s].m_SparVec.size() ; i++ )
		{
			m_WingSections[s].m_SparVec[i]->ComputeEndPoints();
			m_SliceVec.push_back( m_WingSections[s].m_SparVec[i] );
			m_SurfVec.push_back( m_WingSections[s].m_SparVec[i]->GetSurf() );
		}
		//===== Add Upper Skin ====//
		if ( m_WingSections[s].m_UpperSkin.GetExportFlag() )
		{
			m_SkinVec.push_back( &m_WingSections[s].m_UpperSkin );
		}
		//===== Add Lower Skin ====//
		if ( m_WingSections[s].m_LowerSkin.GetExportFlag() )
		{
			m_SkinVec.push_back( &m_WingSections[s].m_LowerSkin );
		}
	}
}

void FeaMeshMgr::SetCurrSectID( int id )
{
	if ( id >= 0 && id < GetNumSections() )
		m_CurrSectID = id;

	if ( m_CurrRibID >= (int)m_WingSections[m_CurrSectID].m_RibVec.size() )
		SetCurrRibID( m_WingSections[m_CurrSectID].m_RibVec.size()-1 );
	if ( m_CurrSparID >= (int)m_WingSections[m_CurrSectID].m_SparVec.size() )
		SetCurrSparID( m_WingSections[m_CurrSectID].m_SparVec.size() - 1 );

	FeaSkin* skin =GetCurrUpperSkin();
	if ( skin )
	{
		if ( skin->GetCurrSpliceLineID() > (int)skin->m_SpliceLineVec.size() )
			skin->SetCurrSpliceLineID( (int)skin->m_SpliceLineVec.size()-1 );
	}
}

void FeaMeshMgr::SetCurrRibID( int id )
{
	if ( m_WingSections.size() <= 0 )
		return;

	if ( id >= 0 && id < (int)m_WingSections[m_CurrSectID].m_RibVec.size() )
		m_CurrRibID = id;
}

int FeaMeshMgr::GetNumRibs()
{
	if ( m_WingSections.size() <= 0 )
		return 0;

	return m_WingSections[m_CurrSectID].m_RibVec.size();
}

void FeaMeshMgr::SetCurrSparID( int id )
{
	if ( m_WingSections.size() <= 0 )
		return;

	if ( id >= 0 && id < (int)m_WingSections[m_CurrSectID].m_SparVec.size() )
		m_CurrSparID = id;
}

int FeaMeshMgr::GetNumSpars()
{
	if ( m_WingSections.size() <= 0 )
		return 0;

	return m_WingSections[m_CurrSectID].m_SparVec.size();
}

FeaRib* FeaMeshMgr::GetCurrRib()
{
	if ( m_CurrSectID >= 0 && m_CurrSectID < (int)m_WingSections.size() )
	{
		if ( m_CurrRibID >= 0 &&  m_CurrRibID < (int)m_WingSections[m_CurrSectID].m_RibVec.size() )
			return m_WingSections[m_CurrSectID].m_RibVec[m_CurrRibID];
	}
	return NULL;
}

FeaSpar* FeaMeshMgr::GetCurrSpar()
{
	if ( m_CurrSectID >= 0 && m_CurrSectID < (int)m_WingSections.size() )
	{
		if ( m_CurrSparID >= 0 &&  m_CurrSparID < (int)m_WingSections[m_CurrSectID].m_SparVec.size() )
			return m_WingSections[m_CurrSectID].m_SparVec[m_CurrSparID];
	}
	return NULL;
}

FeaSkin* FeaMeshMgr::GetCurrUpperSkin()
{
	if ( m_CurrSectID >= 0 && m_CurrSectID < (int)m_WingSections.size() )
	{
		return &m_WingSections[m_CurrSectID].m_UpperSkin;
	}
	return NULL;
}

FeaSkin* FeaMeshMgr::GetCurrLowerSkin()
{
	if ( m_CurrSectID >= 0 && m_CurrSectID < (int)m_WingSections.size() )
	{
		return &m_WingSections[m_CurrSectID].m_LowerSkin;
	}
	return NULL;
}

int FeaMeshMgr::GetNumPointMasses()
{
	return m_PointMassVec.size();
}

void FeaMeshMgr::SetCurrPointMassID( int id )
{
	if ( id >= 0 && id < (int)m_PointMassVec.size() )
		m_CurrPointMassID = id;
}

FeaPointMass* FeaMeshMgr::GetCurrPointMass()
{
	if ( m_CurrPointMassID >= 0 && m_CurrPointMassID < (int)m_PointMassVec.size() )
	{
		return m_PointMassVec[m_CurrPointMassID];
	}
	return NULL;
}

void FeaMeshMgr::AddRib()
{
	if ( m_WingSections.size() <= 0 )
		return;

	FeaRib* rib = new FeaRib();
	rib->m_PerSpan = 0.5;
	rib->m_AbsSweepFlag = true;
	rib->m_TrimFlag = true;
	rib->m_Sweep = 0.0;
	rib->m_SectID = m_CurrSectID;
	rib->ComputeEndPoints();

	m_WingSections[m_CurrSectID].m_RibVec.push_back( rib );
	m_CurrRibID = m_WingSections[m_CurrSectID].m_RibVec.size() - 1;
}

void FeaMeshMgr::DelCurrRib()
{
	if ( m_WingSections.size() <= 0 )
		return;

	vector< FeaRib* > tempVec;
	for ( int i = 0 ; i < (int)m_WingSections[m_CurrSectID].m_RibVec.size() ; i++ )
	{
		FeaRib* rib =  m_WingSections[m_CurrSectID].m_RibVec[i];
		if ( i != m_CurrRibID )
			tempVec.push_back( rib );
		else
			delete rib;
	}
	m_WingSections[m_CurrSectID].m_RibVec = tempVec;
	m_CurrRibID = 0;
}

void FeaMeshMgr::AddSpar()
{
	if ( m_WingSections.size() <= 0 )
		return;

	FeaSpar* spar = new FeaSpar();
	spar->m_PerChord = 0.5;
	spar->m_AbsSweepFlag = true;
	spar->m_TrimFlag = true;
	spar->m_Sweep = 0.0;
	spar->m_SectID = m_CurrSectID;
	spar->ComputeEndPoints();

	m_WingSections[m_CurrSectID].m_SparVec.push_back( spar );
	m_CurrSparID = m_WingSections[m_CurrSectID].m_SparVec.size() - 1;

}

void FeaMeshMgr::DelCurrSpar()
{
	if ( m_WingSections.size() <= 0 )
		return;

	vector< FeaSpar* > tempVec;
	for ( int i = 0 ; i < (int)m_WingSections[m_CurrSectID].m_SparVec.size() ; i++ )
	{
		FeaSpar* spar =  m_WingSections[m_CurrSectID].m_SparVec[i];
		if ( i != m_CurrSparID )
			tempVec.push_back( spar );
		else
			delete spar;
	}
	m_WingSections[m_CurrSectID].m_SparVec = tempVec;
	m_CurrSparID = 0;
}

void FeaMeshMgr::AddPointMass()
{
	if ( m_WingSections.size() <= 0 )
		return;

	FeaPointMass* pm = new FeaPointMass();
	pm->m_Pos = vec3d(0,0,0);
	pm->m_AttachPos = vec3d(1,0,0);

	m_PointMassVec.push_back( pm );

	m_CurrPointMassID = m_PointMassVec.size() - 1;

}

void FeaMeshMgr::DelCurrPointMass()
{
	vector< FeaPointMass* > tempVec;
	for ( int i = 0 ; i < (int)m_PointMassVec.size() ; i++ )
	{
		FeaPointMass* pm =  m_PointMassVec[i];
		if ( i != m_CurrPointMassID )
			tempVec.push_back( pm );
		else
			delete pm;
	}

	m_PointMassVec = tempVec;
	m_CurrPointMassID = 0;
}


void FeaMeshMgr::RemoveSliceSurfaces()
{
	vector< Surf* > slice_surf_vec;
	for ( int i = 0 ; i < (int)m_SliceVec.size() ; i++ )
		slice_surf_vec.push_back( m_SliceVec[i]->GetSurf() );

	vector< Surf* > tempVec;
	for ( int i = 0 ; i < (int)m_SurfVec.size() ; i++ )
	{
		if ( !(ContainsVal<Surf*> ( slice_surf_vec, m_SurfVec[i] )) )
		{
			tempVec.push_back(  m_SurfVec[i] );
		}
	}
	m_SurfVec = tempVec;
}


	
void FeaMeshMgr::LoadChains( Surf* sliceSurf, bool upperFlag, int sect_id, list< ISegChain* > & chain_list )
{
	Surf* skin_surf = NULL;
	if ( upperFlag )
		skin_surf = m_UpperSurfVec[sect_id];
	else
		skin_surf = m_LowerSurfVec[sect_id];

	list< ISegChain* >::iterator c;
	for ( c = m_ISegChainList.begin() ; c != m_ISegChainList.end(); c++ )
	{
		if ( (*c)->m_SurfA == skin_surf && (*c)->m_SurfB == sliceSurf )
		{
			chain_list.push_back( (*c) );
		}
		else if ( (*c)->m_SurfB == skin_surf && (*c)->m_SurfA == sliceSurf )
		{
			chain_list.push_back( (*c) );
		}
	}
}

void FeaMeshMgr::LoadCapChains( Surf* s0, Surf* s1, list< ISegChain* > & chain_list )
{
	list< ISegChain* >::iterator c;
	for ( c = m_ISegChainList.begin() ; c != m_ISegChainList.end(); c++ )
	{
		if ( (*c)->m_SurfA == s0 && (*c)->m_SurfB == s1 )
		{
			chain_list.push_back( (*c) );
		}
		else if ( (*c)->m_SurfB == s0 && (*c)->m_SurfA == s1 )
		{
			chain_list.push_back( (*c) );
		}
	}
}
void FeaMeshMgr::LoadCapChains( Surf* s0, double u, list< ISegChain* > & chain_list )
{
	list< ISegChain* >::iterator c;
	for ( c = m_ISegChainList.begin() ; c != m_ISegChainList.end(); c++ )
	{
		if ( (*c)->m_SurfA == s0 && (*c)->m_SurfB == s0 )
		{
			bool matchFlag = true;
			vector< vec3d > uwpnts = (*c)->m_ACurve.GetUWTessPnts();
			for ( int i = 0 ; i < (int)uwpnts.size() ; i++ )
			{
				double del = fabs( u - uwpnts[i].x() );
				if ( del > 0.001 )
					matchFlag = false;
			}
			if ( matchFlag )
				chain_list.push_back( (*c) );
		}
	}
}

void FeaMeshMgr::BuildSliceMesh()
{
	int i;

	//==== Find Upper/Lower Points ====//
	for ( i = 0 ; i < (int)m_SliceVec.size() ; i++ )
		m_SliceVec[i]->FindUpperLowerPoints();

	//==== Find Max Number of Divisions ====//
	int max_num_divisions = 1;
	for (  i = 0 ; i < (int)m_SliceVec.size() ; i++ )
	{
		int num = m_SliceVec[i]->ComputeNumDivisions();
		if ( num > max_num_divisions )
			max_num_divisions = num;
	}
	//==== Set Num Divisions For All Slices (So Elements Line Up ) ====//
	for (  i = 0 ; i < (int)m_SliceVec.size() ; i++ )
		m_SliceVec[i]->SetNumDivisions( max_num_divisions );

	//==== Build Skin FEA Elements ====//
	for (  i = 0 ; i < (int)m_SkinVec.size() ; i++ )
	{
		m_SkinVec[i]->BuildMesh();
		m_SkinVec[i]->SetNodeThick();
	}

	//==== Snap Slice Points to Skin Nodes ====//
	vector < FeaNode* > skinNodes;
	for (  i = 0 ; i < (int)m_SkinVec.size() ; i++ )
	{
		m_SkinVec[i]->LoadNodes( skinNodes );
	}
	for (  i = 0 ; i < (int)m_SliceVec.size() ; i++ )
		m_SliceVec[i]->SnapUpperLowerToSkin( skinNodes );

	//==== Build Slice FEA Elements ====//
	for (  i = 0 ; i < (int)m_SliceVec.size() ; i++ )
		m_SliceVec[i]->BuildMesh();


}

void FeaMeshMgr::LoadAttachPoints()
{
	int i;

	//==== Load Upper and Lower Points to Attach Point Masses ====//
	m_AttachPoints.clear();
	for (  i = 0 ; i < (int)m_SliceVec.size() ; i++ )
	{
		for ( int j = 0 ; j < (int)m_SliceVec[i]->m_UpperPnts.size() ; j++ )
			m_AttachPoints.push_back( m_SliceVec[i]->m_UpperPnts[j] );
		for ( int j = 0 ; j < (int)m_SliceVec[i]->m_LowerPnts.size() ; j++ )
			m_AttachPoints.push_back( m_SliceVec[i]->m_LowerPnts[j] );
	}
}


void FeaMeshMgr::ComputeWriteMass()
{
	double total_mass = 0.0;
	vector< double > upSkinMass;
	vector< double > lowSkinMass;
	vector< double > ribMass;
	vector< double > sparMass;

	for ( int i = 0 ; i < (int)m_WingSections.size() ; i++ )
	{
		double up_skin_mass  = m_WingSections[i].m_UpperSkin.ComputeMass()*m_ThickScale;
		upSkinMass.push_back( up_skin_mass );

		double low_skin_mass = m_WingSections[i].m_LowerSkin.ComputeMass()*m_ThickScale;
		lowSkinMass.push_back( low_skin_mass );

		double rib_mass = 0.0;
		for ( int r = 0 ; r < (int)m_WingSections[i].m_RibVec.size() ; r++ )
		{
			rib_mass += m_WingSections[i].m_RibVec[r]->ComputeMass()*m_ThickScale;
		}
		ribMass.push_back( rib_mass );

		double spar_mass = 0.0;
		for ( int s = 0 ; s < (int)m_WingSections[i].m_SparVec.size() ; s++ )
		{
			spar_mass += m_WingSections[i].m_SparVec[s]->ComputeMass()*m_ThickScale;
		}
		sparMass.push_back( spar_mass );

		total_mass += up_skin_mass + low_skin_mass + rib_mass + spar_mass;
	}

	m_TotalMass = total_mass;

	FILE* fp = fopen( m_ExportFeaFileNames[MASS_FILE_NAME], "w" );
	if ( fp )
	{
		fprintf( fp, "Section_ID, Upper_Skin_Mass, Lower_Skin_Mass, Rib_Mass, Spar_Mass, Total_Section_Mass\n");
		for ( int i = 0 ; i < (int)m_WingSections.size() ; i++ )
		{
			double sect_total = upSkinMass[i] + lowSkinMass[i] + ribMass[i]+ sparMass[i];
			fprintf( fp, "%d, %f, %f, %f, %f, %f\n", 
				i, upSkinMass[i], lowSkinMass[i], ribMass[i], sparMass[i], sect_total );
		}
		fprintf( fp, "Total Mass = %f \n", m_TotalMass );
		fclose(fp);
	}
}

FeaNode* FeaMeshMgr::FindNode(  vector< FeaNode* > nodeVec, int id )
{
	//==== jrg Brute Force for Now ====//
	for ( int i = 0 ; i < (int)nodeVec.size() ; i++ )
	{
		if ( nodeVec[i]->m_Index == id )
			return nodeVec[i];
	}

	return NULL;
}


void FeaMeshMgr::WriteNASTRAN( const char* filename )
{
	//==== Collect All FeaNodes ====//
	vector< FeaNode* > nodeVec;
	for ( int i = 0 ; i < (int)m_SkinVec.size() ; i++ )
		m_SkinVec[i]->LoadNodes( nodeVec );
	for ( int i = 0 ; i < (int)m_SliceVec.size() ; i++ )
		m_SliceVec[i]->LoadNodes( nodeVec );

	vector< vec3d* > allPntVec;
	for ( int i = 0 ; i < (int)nodeVec.size() ; i++ )
		allPntVec.push_back( &nodeVec[i]->m_Pnt );

	//==== Build Node Map ====//
	map< int, vector< int > > indMap;
	vector< int > pntShift;
	int numPnts = cfdMeshMgrPtr->BuildIndMap( allPntVec, indMap, pntShift );

	//==== Assign Index Numbers to Nodes ====//
	for ( int i = 0 ; i < (int)nodeVec.size() ; i++ )
	{
		nodeVec[i]->m_Tags.clear();
		int ind = cfdMeshMgrPtr->FindPntIndex( nodeVec[i]->m_Pnt, allPntVec, indMap );
		nodeVec[i]->m_Index = pntShift[ind]+1;
	}

	//Stringc fn( base_filename );
	//fn.concatenate( "NASTRAN.dat" );
	
	FILE* fp = fopen( filename, "w" );
	if ( fp )
	{
		//===== Write Ribs ====//
		int elem_id = 0;
		int rib_cnt = 0;
		for ( int s = 0 ; s < (int)m_WingSections.size() ; s++ )
		{
			for ( int r = 0 ; r < (int)m_WingSections[s].m_RibVec.size() ; r++ )
			{
				rib_cnt++;
				fprintf(fp, "$Rib,%d\n",r+1);
				FeaRib* rib = m_WingSections[s].m_RibVec[r];
				for ( int i = 0 ; i < (int)rib->m_Elements.size() ; i++ )
				{
					elem_id++;
					rib->m_Elements[i]->WriteNASTRAN( fp, elem_id );
				}
				fprintf(fp, "\n");

				//==== Tag Rib Upper/Lower Nodes ====//
				for ( int i = 0 ; i < (int)rib->m_UpperPnts.size() ; i++ )
				{
					int ind = cfdMeshMgrPtr->FindPntIndex(rib->m_UpperPnts[i], allPntVec, indMap);
					FeaNode* node = FindNode( nodeVec, pntShift[ind]+1 );
					if ( node )	
						node->AddTag( RIB_UPPER, rib_cnt );
				}
				for ( int i = 0 ; i < (int)rib->m_LowerPnts.size() ; i++ )
				{
					int ind = cfdMeshMgrPtr->FindPntIndex(rib->m_LowerPnts[i], allPntVec, indMap);
					FeaNode* node = FindNode( nodeVec, pntShift[ind]+1 );
					if ( node )	
						node->AddTag( RIB_LOWER, rib_cnt );
				}
			}
		}
		//===== Write Spars ====//
		int spar_cnt = 0;
		for ( int s = 0 ; s < (int)m_WingSections.size() ; s++ )
		{
			for ( int r = 0 ; r < (int)m_WingSections[s].m_SparVec.size() ; r++ )
			{
				spar_cnt++;
				fprintf(fp, "$Spar,%d\n",r+1);
				FeaSpar* spar = m_WingSections[s].m_SparVec[r];
				for ( int i = 0 ; i < (int)spar->m_Elements.size() ; i++ )
				{
					elem_id++;
					spar->m_Elements[i]->WriteNASTRAN( fp, elem_id );
				}
				fprintf(fp, "\n");

				//==== Tag Spar Upper/Lower Nodes ====//
				for ( int i = 0 ; i < (int)spar->m_UpperPnts.size() ; i++ )
				{
					int ind = cfdMeshMgrPtr->FindPntIndex(spar->m_UpperPnts[i], allPntVec, indMap);
					FeaNode* node = FindNode( nodeVec, pntShift[ind]+1 );
					if ( node )	
						node->AddTag( SPAR_UPPER, spar_cnt );
				}
				for ( int i = 0 ; i < (int)spar->m_LowerPnts.size() ; i++ )
				{
					int ind = cfdMeshMgrPtr->FindPntIndex(spar->m_LowerPnts[i], allPntVec, indMap);
					FeaNode* node = FindNode( nodeVec, pntShift[ind]+1 );
					if ( node )	
						node->AddTag( SPAR_LOWER, spar_cnt );
				}
			}
		}
		//===== Write Upper Skin ====//
		fprintf(fp, "$Upperskin\n");
		for ( int s = 0 ; s < (int)m_WingSections.size() ; s++ )
		{
			FeaSkin* skin = &m_WingSections[s].m_UpperSkin;
			for ( int i = 0 ; i < (int)skin->m_Elements.size() ; i++ )
			{
				elem_id++;
				skin->m_Elements[i]->WriteNASTRAN( fp, elem_id );
			}
		}
		fprintf(fp, "\n");

		//===== Write Lower Skin ====//
		fprintf(fp, "$Lowerskin\n");
		for ( int s = 0 ; s < (int)m_WingSections.size() ; s++ )
		{
			FeaSkin* skin = &m_WingSections[s].m_LowerSkin;
			for ( int i = 0 ; i < (int)skin->m_Elements.size() ; i++ )
			{
				elem_id++;
				skin->m_Elements[i]->WriteNASTRAN( fp, elem_id );
			}
		}
		fprintf(fp, "\n");
		fprintf(fp, "$Gridpoints\n\n");

		//==== Write Rib Spar Intersections =====//
		for ( int r = 0 ; r < rib_cnt ; r++ )
		{
			for ( int s = 0 ; s < spar_cnt ; s++ )
			{
				FeaNode* upperINode = NULL;
				FeaNode* lowerINode = NULL;
				for ( int i = 0 ; i < (int)nodeVec.size() ; i++ )
				{
					if ( nodeVec[i]->m_Tags.size() > 1 )
					{
						if ( nodeVec[i]->HasTag(RIB_UPPER, r+1) && nodeVec[i]->HasTag(SPAR_UPPER, s+1) )
							upperINode = nodeVec[i];
						if ( nodeVec[i]->HasTag(RIB_LOWER, r+1) && nodeVec[i]->HasTag(SPAR_LOWER, s+1) )
							lowerINode = nodeVec[i];
					}
				}
				if ( upperINode && lowerINode )
				{
					fprintf(fp, "\n");
					fprintf(fp, "$Intersection,%d,%d\n", r+1, s+1);
					upperINode->WriteNASTRAN( fp );
					lowerINode->WriteNASTRAN( fp );
				}
			}
		}
		//==== Write Out Rib LE/TE ====//
		for ( int r = 0 ; r < rib_cnt ; r++ )
		{
			vector< FeaNode* > letenodes;
			for ( int i = 0 ; i < (int)nodeVec.size() ; i++ )
			{
				if ( nodeVec[i]->m_Tags.size() == 2 )
				{
					if ( nodeVec[i]->HasTag( RIB_LOWER, r+1 ) && nodeVec[i]->HasTag( RIB_UPPER, r+1 ) )
					{
						letenodes.push_back( nodeVec[i] );
					}
				}
			}
			if ( letenodes.size() == 2 )
			{
				if ( letenodes[1]->m_Pnt.x() < letenodes[0]->m_Pnt.x() )
				{
					FeaNode* temp = letenodes[0];
					letenodes[0]  = letenodes[1];
					letenodes[1]  = temp;
				}
				fprintf(fp, "\n");
				fprintf(fp, "$RibLE,%d\n", r+1 );
				letenodes[0]->WriteNASTRAN( fp );

				fprintf(fp, "\n");
				fprintf(fp, "$RibTE,%d\n", r+1 );
				letenodes[1]->WriteNASTRAN( fp );
			}
		}

		//==== Write Rib Upper Boundary =====//
		for ( int r = 0 ; r < rib_cnt ; r++ )
		{
			fprintf(fp, "\n");
			fprintf(fp, "$RibUpperBoundary,%d\n", r+1 );
			for ( int i = 0 ; i < (int)nodeVec.size() ; i++ )
			{
				if ( nodeVec[i]->HasTag(RIB_UPPER, r+1) && nodeVec[i]->m_Tags.size() == 1 )
					nodeVec[i]->WriteNASTRAN( fp );
			}
		}
		//==== Write Spar Upper Boundary =====//
		for ( int s = 0 ; s < spar_cnt ; s++ )
		{
			fprintf(fp, "\n");
			fprintf(fp, "$SparUpperBoundary,%d\n", s+1 );
			for ( int i = 0 ; i < (int)nodeVec.size() ; i++ )
			{
				if ( nodeVec[i]->HasTag(SPAR_UPPER, s+1) && nodeVec[i]->m_Tags.size() == 1 )
					nodeVec[i]->WriteNASTRAN( fp );
			}
		}
		//==== Write Rib Lower Boundary  =====//
		for ( int r = 0 ; r < rib_cnt ; r++ )
		{
			fprintf(fp, "\n");
			fprintf(fp, "$RibLowerBoundary,%d\n", r+1 );
			for ( int i = 0 ; i < (int)nodeVec.size() ; i++ )
			{
				if ( nodeVec[i]->HasTag(RIB_LOWER, r+1) && nodeVec[i]->m_Tags.size() == 1 )
					nodeVec[i]->WriteNASTRAN( fp );
			}
		}
		//==== Write Spar Lower Boundary =====//
		for ( int s = 0 ; s < spar_cnt ; s++ )
		{
			fprintf(fp, "\n");
			fprintf(fp, "$SparLowerBoundary,%d\n", s+1 );
			for ( int i = 0 ; i < (int)nodeVec.size() ; i++ )
			{
				if ( nodeVec[i]->HasTag(SPAR_LOWER, s+1) && nodeVec[i]->m_Tags.size() == 1 )
					nodeVec[i]->WriteNASTRAN( fp );
			}
		}
		//==== Write Point Masses =====//
		for ( int p = 0 ; p < (int)m_PointMassVec.size() ; p++ )
		{
			//==== Snap To Nearest Attach Point ====//
			int close_ind  = 0;
			double close_d = 1.0e12;
			for ( int i = 0 ; i < (int)m_AttachPoints.size() ; i++ )
			{
				double d = dist_squared(  m_PointMassVec[p]->m_AttachPos, m_AttachPoints[i] );
				if ( d < close_d )
				{
					close_d = d;
					close_ind = i;
				}
			}
			if ( m_AttachPoints.size() )
				m_PointMassVec[p]->m_AttachPos = m_AttachPoints[close_ind];

			FeaNode node;
			node.m_Pnt = m_PointMassVec[p]->m_Pos;
			node.m_Index = numPnts + p + 1;
			fprintf(fp, "\n");
			fprintf(fp, "$Pointmass,%d\n", p+1 );
			node.WriteNASTRAN( fp );

			//==== Find Attach Point Index ====//
			int ind = cfdMeshMgrPtr->FindPntIndex( m_PointMassVec[p]->m_AttachPos, allPntVec, indMap );
			fprintf(fp, "$Connects,%d\n", pntShift[ind]+1 );
		}

		//==== Remaining Nodes ====//
		fprintf(fp, "\n");
		fprintf(fp, "$Remainingnodes\n" );
		for ( int i = 0 ; i < (int)nodeVec.size() ; i++ )
		{
			if ( pntShift[i] >= 0 && nodeVec[i]->m_Tags.size() == 0 )
			{
				 nodeVec[i]->WriteNASTRAN( fp );
			}
		}

		fclose(fp);
	}
}

void FeaMeshMgr::WriteCalculix( )
{
	//==== Collect All FeaNodes ====//
	vector< FeaNode* > nodeVec;
	for ( int s = 0 ; s < (int)m_WingSections.size() ; s++ )
	{
		if ( m_WingSections[s].m_UpperSkin.GetExportFlag() )
			m_WingSections[s].m_UpperSkin.LoadNodes( nodeVec );
		if ( m_WingSections[s].m_LowerSkin.GetExportFlag() )
			m_WingSections[s].m_LowerSkin.LoadNodes( nodeVec );
		for ( int r = 0 ; r < (int)m_WingSections[s].m_RibVec.size() ; r++ )
		{
			m_WingSections[s].m_RibVec[r]->LoadNodes( nodeVec );
		}
		for ( int r = 0 ; r < (int)m_WingSections[s].m_SparVec.size() ; r++ )
		{
			m_WingSections[s].m_SparVec[r]->LoadNodes( nodeVec );
		}
	}

	vector< vec3d* > allPntVec;
	for ( int i = 0 ; i < (int)nodeVec.size() ; i++ )
		allPntVec.push_back( &nodeVec[i]->m_Pnt );

	//==== Build Node Map ====//
	map< int, vector< int > > indMap;
	vector< int > pntShift;
	int numPnts = cfdMeshMgrPtr->BuildIndMap( allPntVec, indMap, pntShift );

	//==== Assign Index Numbers to Nodes ====//
	for ( int i = 0 ; i < (int)nodeVec.size() ; i++ )
	{
		nodeVec[i]->m_Tags.clear();
		int ind = cfdMeshMgrPtr->FindPntIndex( nodeVec[i]->m_Pnt, allPntVec, indMap );
		nodeVec[i]->m_Index = pntShift[ind]+1;
	}

	//==== Tag Nodes ====//
	vector< FeaSkin* > upperSkins;
	vector< FeaSkin* > lowerSkins;
	vector< FeaRib*  > ribs;
	vector< FeaSpar* > spars;
	for ( int s = 0 ; s < (int)m_WingSections.size() ; s++ )
	{
		if ( m_WingSections[s].m_UpperSkin.GetExportFlag() )
		{
			vector< FeaNode* > nVec;
			m_WingSections[s].m_UpperSkin.LoadNodes( nVec );
			for ( int i = 0 ; i < (int)nVec.size() ; i++ )
			{
				int ind = cfdMeshMgrPtr->FindPntIndex(nVec[i]->m_Pnt, allPntVec, indMap);
				nodeVec[ind]->AddTag( SKIN_UPPER, s );
			}
			upperSkins.push_back( &m_WingSections[s].m_UpperSkin );
		}
		if ( m_WingSections[s].m_LowerSkin.GetExportFlag() )
		{
			vector< FeaNode* > nVec;
			m_WingSections[s].m_LowerSkin.LoadNodes( nVec );
			for ( int i = 0 ; i < (int)nVec.size() ; i++ )
			{
				int ind = cfdMeshMgrPtr->FindPntIndex(nVec[i]->m_Pnt, allPntVec, indMap);
				nodeVec[ind]->AddTag( SKIN_LOWER, s );
			}
			lowerSkins.push_back( &m_WingSections[s].m_LowerSkin );
		}

		//===== Tag Ribs ====//
		for ( int r = 0 ; r < (int)m_WingSections[s].m_RibVec.size() ; r++ )
		{
			vector< FeaNode* > nVec;
			m_WingSections[s].m_RibVec[r]->LoadNodes( nVec );
			for ( int i = 0 ; i < (int)nVec.size() ; i++ )
			{
				int ind = cfdMeshMgrPtr->FindPntIndex(nVec[i]->m_Pnt, allPntVec, indMap);
				nodeVec[ind]->AddTag( RIB_ALL, r );
			}
			ribs.push_back( m_WingSections[s].m_RibVec[r] );
		}
		//===== Tag Spars ====//
		for ( int r = 0 ; r < (int)m_WingSections[s].m_SparVec.size() ; r++ )
		{
			vector< FeaNode* > nVec;
			m_WingSections[s].m_SparVec[r]->LoadNodes( nVec );
			for ( int i = 0 ; i < (int)nVec.size() ; i++ )
			{
				int ind = cfdMeshMgrPtr->FindPntIndex(nVec[i]->m_Pnt, allPntVec, indMap);
				nodeVec[ind]->AddTag( SPAR_ALL, r );
			}
			spars.push_back( m_WingSections[s].m_SparVec[r] );
		}
	}

//for ( int i = 0 ; i < (int)nodeVec.size() ; i++ )
//{
//	if ( nodeVec[i]->HasTag( SPAR_ALL ) &&  nodeVec[i]->HasTag( SKIN_UPPER ) )
//	{
//		debugPnts.push_back( nodeVec[i]->m_Pnt );
//	}
//}

	//Stringc fn( base_filename );
	//fn.concatenate( "geom.dat" );

	Stringc fn = m_ExportFeaFileNames[GEOM_FILE_NAME];
	FILE* fp = fopen( fn.get_char_star(), "w" );
	if ( fp )
	{
		int elem_id = 1;

		//==== Upper Skin Nodes ====//
		fprintf(fp, "**%%Upper Skin\n" );
		fprintf(fp, "*NODE, NSET=Nupperskin\n");
		for ( int i = 0 ; i < (int)nodeVec.size() ; i++ )
		{
			if ( pntShift[i] >= 0 )
			{
				if ( nodeVec[i]->m_Tags.size() == 1 && nodeVec[i]->HasTag( SKIN_UPPER ) )
				{
					nodeVec[i]->WriteCalculix(fp);
				}
				if ( nodeVec[i]->m_Tags.size() == 2 && 
					 nodeVec[i]->HasTag( SKIN_UPPER ) && nodeVec[i]->HasTag( SKIN_LOWER  ) )
				{
					nodeVec[i]->WriteCalculix(fp);
				}
			}
		}
		fprintf(fp, "\n" );
		fprintf(fp, "*ELEMENT, TYPE=S6, ELSET=Eupperskin\n");
		for ( int s = 0 ; s < (int)upperSkins.size() ; s++ )
		{
			for ( int e = 0 ; e < (int)upperSkins[s]->m_Elements.size() ; e++ )
			{
				if ( upperSkins[s]->m_Elements[e]->GetType() == FeaElement::FEA_TRI_6 )
				{
					upperSkins[s]->m_Elements[e]->WriteCalculix( fp, elem_id );
					elem_id++;
				}
			}
		}
		//==== Upper Skin Nodes ====//
		fprintf(fp, "\n" );
		fprintf(fp, "**%%Lower Skin\n" );
		fprintf(fp, "*NODE, NSET=Nlowerskin\n");
		for ( int i = 0 ; i < (int)nodeVec.size() ; i++ )
		{
			if ( pntShift[i] >= 0 )
			{
				if ( nodeVec[i]->m_Tags.size() == 1 && nodeVec[i]->HasTag( SKIN_LOWER ) )
				{
					nodeVec[i]->WriteCalculix(fp);
				}
			}
		}
		fprintf(fp, "\n" );
		fprintf(fp, "*ELEMENT, TYPE=S6, ELSET=Elowerskin\n");
		for ( int s = 0 ; s < (int)lowerSkins.size() ; s++ )
		{
			for ( int e = 0 ; e < (int)lowerSkins[s]->m_Elements.size() ; e++ )
			{
				if ( lowerSkins[s]->m_Elements[e]->GetType() == FeaElement::FEA_TRI_6 )
				{
					lowerSkins[s]->m_Elements[e]->WriteCalculix( fp, elem_id );
					elem_id++;
				}
			}
		}

		//==== Spars ====//
		for ( int s = 0 ; s < (int)spars.size() ; s++ )
		{
			fprintf(fp, "\n" );
			fprintf(fp, "**%%Spar %d\n", s );
			fprintf(fp, "*NODE, NSET=Nspar%d\n", s);

			for ( int i = 0 ; i < (int)nodeVec.size() ; i++ )
			{
				if ( pntShift[i] >= 0 && nodeVec[i]->HasTag(SPAR_ALL, s) )
				{
					if ( !nodeVec[i]->HasTag( RIB_ALL ) )
					{
						nodeVec[i]->m_Thick = spars[s]->m_Thick;
						nodeVec[i]->WriteCalculix(fp);
					}
				}
			}

			fprintf(fp, "\n" );
			fprintf(fp, "*ELEMENT, TYPE=S6, ELSET=Espartri%d\n", s);
			for ( int e = 0 ; e < (int)spars[s]->m_Elements.size() ; e++ )
			{
				if ( spars[s]->m_Elements[e]->GetType() == FeaElement::FEA_TRI_6 )
				{
					spars[s]->m_Elements[e]->WriteCalculix( fp, elem_id );
					elem_id++;
				}
			}
			fprintf(fp, "\n" );
			fprintf(fp, "*ELEMENT, TYPE=S8, ELSET=Esparquad%d\n", s);
			for ( int e = 0 ; e < (int)spars[s]->m_Elements.size() ; e++ )
			{
				if ( spars[s]->m_Elements[e]->GetType() == FeaElement::FEA_QUAD_8 )
				{
					spars[s]->m_Elements[e]->WriteCalculix( fp, elem_id );
					elem_id++;
				}
			}
		}
		//==== Ribs ====//
		for ( int r = 0 ; r < (int)ribs.size() ; r++ )
		{
			fprintf(fp, "\n" );
			fprintf(fp, "**%%Rib %d\n", r );
			fprintf(fp, "*NODE, NSET=Nrib%d\n", r);

			for ( int i = 0 ; i < (int)nodeVec.size() ; i++ )
			{
				if ( pntShift[i] >= 0 && nodeVec[i]->HasTag(RIB_ALL, r) )
				{
					if ( !nodeVec[i]->HasTag( SPAR_ALL ) )
					{
						nodeVec[i]->m_Thick = ribs[r]->m_Thick;
						nodeVec[i]->WriteCalculix(fp);
					}
				}
			}

			fprintf(fp, "\n" );
			fprintf(fp, "*ELEMENT, TYPE=S6, ELSET=Eribtri%d\n", r);
			for ( int e = 0 ; e < (int)ribs[r]->m_Elements.size() ; e++ )
			{
				if ( ribs[r]->m_Elements[e]->GetType() == FeaElement::FEA_TRI_6 )
				{
					ribs[r]->m_Elements[e]->WriteCalculix( fp, elem_id );
					elem_id++;
				}
			}
			fprintf(fp, "\n" );
			fprintf(fp, "*ELEMENT, TYPE=S8, ELSET=Eribquad%d\n", r);
			for ( int e = 0 ; e < (int)ribs[r]->m_Elements.size() ; e++ )
			{
				if ( ribs[r]->m_Elements[e]->GetType() == FeaElement::FEA_QUAD_8 )
				{
					ribs[r]->m_Elements[e]->WriteCalculix( fp, elem_id );
					elem_id++;
				}
			}
		}

		//==== Rib Spar Intersections ====//
		for ( int r = 0 ; r < (int)ribs.size() ; r++ )
		{
			for ( int s = 0 ; s < (int)spars.size() ; s++ )
			{
				fprintf(fp, "\n" );
				fprintf(fp, "**%%Rib-Spar connections %d %d\n", r, s);
				fprintf(fp, "*NODE, NSET=Nconnections%d%d\n", r, s);
				for ( int i = 0 ; i < (int)nodeVec.size() ; i++ )
				{
					if ( pntShift[i] >= 0 )
					{
						if ( nodeVec[i]->HasTag(RIB_ALL, r) && nodeVec[i]->HasTag( SPAR_ALL, s ) )
						{
							nodeVec[i]->m_Thick = 0.5*(ribs[r]->m_Thick+spars[s]->m_Thick);
							nodeVec[i]->WriteCalculix(fp);
						}
					}
				}
			}
		}
		fclose(fp);
	}
		
	//==== Write Node Thickness Include File ====//
	//Stringc node_fn( base_filename );
	//node_fn.concatenate( "nodethick.dat" );
		
	Stringc node_fn = m_ExportFeaFileNames[THICK_FILE_NAME];
	fp = fopen( node_fn.get_char_star(), "w" );
	if ( fp )
	{
		fprintf( fp, "*NODAL THICKNESS\n");

		//==== Upper Skin Nodes ====//
		fprintf(fp, "**%%Upper Skin\n" );
		for ( int i = 0 ; i < (int)nodeVec.size() ; i++ )
		{
			if ( pntShift[i] >= 0 )
			{
				if ( nodeVec[i]->m_Tags.size() == 1 && nodeVec[i]->HasTag( SKIN_UPPER ) )
				{
					fprintf(fp, "%d,%f\n", nodeVec[i]->m_Index, nodeVec[i]->m_Thick*m_ThickScale );
				}
				if ( nodeVec[i]->m_Tags.size() == 2 && 
					 nodeVec[i]->HasTag( SKIN_UPPER ) && nodeVec[i]->HasTag( SKIN_LOWER  ) )
				{
					fprintf(fp, "%d,%f\n", nodeVec[i]->m_Index, nodeVec[i]->m_Thick*m_ThickScale );
				}

			}
		}

		fprintf(fp, "**%%Lower Skin\n" );
		for ( int i = 0 ; i < (int)nodeVec.size() ; i++ )
		{
			if ( pntShift[i] >= 0 )
			{
				if ( nodeVec[i]->m_Tags.size() == 1 && nodeVec[i]->HasTag( SKIN_LOWER ) )
				{
					fprintf(fp, "%d,%f\n", nodeVec[i]->m_Index, nodeVec[i]->m_Thick*m_ThickScale );
				}
			}
		}

		//==== Spars ====//
		for ( int s = 0 ; s < (int)spars.size() ; s++ )
		{
			fprintf(fp, "\n" );
			fprintf(fp, "**%%Spar %d\n", s );
			for ( int i = 0 ; i < (int)nodeVec.size() ; i++ )
			{
				if ( pntShift[i] >= 0 && nodeVec[i]->HasTag(SPAR_ALL, s) )
				{
					if ( nodeVec[i]->m_Tags.size() == 1 )
					{
						fprintf(fp, "%d,%f\n", nodeVec[i]->m_Index, nodeVec[i]->m_Thick*m_ThickScale );
					}
				}
			}
		}

		//==== Ribs ====//
		for ( int r = 0 ; r < (int)ribs.size() ; r++ )
		{
			fprintf(fp, "\n" );
			fprintf(fp, "**%%Rib %d\n", r );
			for ( int i = 0 ; i < (int)nodeVec.size() ; i++ )
			{
				if ( pntShift[i] >= 0 && nodeVec[i]->HasTag(RIB_ALL, r) )
				{
					if ( nodeVec[i]->m_Tags.size() == 1 )
					{
						fprintf(fp, "%d,%f\n", nodeVec[i]->m_Index, nodeVec[i]->m_Thick*m_ThickScale );
					}
				}
			}
		}


		//==== Rib Spar Intersections ====//
		for ( int r = 0 ; r < (int)ribs.size() ; r++ )
		{
			for ( int s = 0 ; s < (int)spars.size() ; s++ )
			{
				fprintf(fp, "\n" );
				fprintf(fp, "**%%Rib-Spar connections %d %d\n", r, s);
				for ( int i = 0 ; i < (int)nodeVec.size() ; i++ )
				{
					if ( pntShift[i] >= 0 && nodeVec[i]->m_Tags.size() == 2 )
					{
						if ( nodeVec[i]->HasTag(RIB_ALL, r) && nodeVec[i]->HasTag( SPAR_ALL, s ) )
						{
							fprintf(fp, "%d,%f\n", nodeVec[i]->m_Index, nodeVec[i]->m_Thick*m_ThickScale );
						}
					}
				}
			}
		}

		//==== Spar Upper Skin Intersections ====//
		for ( int s = 0 ; s < (int)spars.size() ; s++ )
		{
			fprintf(fp, "\n" );
			fprintf(fp, "**%%Spar-Skin connections %d upperskin \n", s );
			for ( int i = 0 ; i < (int)nodeVec.size() ; i++ )
			{
				if ( pntShift[i] >= 0 )
				{
					if ( nodeVec[i]->HasTag(SPAR_ALL, s) && nodeVec[i]->HasTag( SKIN_UPPER ) )
					{
						fprintf(fp, "%d,%f\n", nodeVec[i]->m_Index, nodeVec[i]->m_Thick*m_ThickScale );
					}
				}
			}
		}

		//==== Spar Lower Skin Intersections ====//
		for ( int s = 0 ; s < (int)spars.size() ; s++ )
		{
			fprintf(fp, "\n" );
			fprintf(fp, "**%%Spar-Skin connections %d lowerskin \n", s );
			for ( int i = 0 ; i < (int)nodeVec.size() ; i++ )
			{
				if ( pntShift[i] >= 0 )
				{
					if ( nodeVec[i]->HasTag(SPAR_ALL, s) && nodeVec[i]->HasTag( SKIN_LOWER ) )
					{
						fprintf(fp, "%d,%f\n", nodeVec[i]->m_Index, nodeVec[i]->m_Thick*m_ThickScale );
					}
				}
			}
		}

		//==== Rib Upper Skin Intersections ====//
		for ( int r = 0 ; r < (int)ribs.size() ; r++ )
		{
			fprintf(fp, "\n" );
			fprintf(fp, "**%%Rib-Skin connections %d upperskin \n", r );
			for ( int i = 0 ; i < (int)nodeVec.size() ; i++ )
			{
				if ( pntShift[i] >= 0 )
				{
					if ( nodeVec[i]->HasTag(RIB_ALL, r) && nodeVec[i]->HasTag( SKIN_UPPER ) )
					{
						fprintf(fp, "%d,%f\n", nodeVec[i]->m_Index, nodeVec[i]->m_Thick*m_ThickScale );
					}
				}
			}
		}

		//==== Rib Lower Skin Intersections ====//
		for ( int r = 0 ; r < (int)ribs.size() ; r++ )
		{
			fprintf(fp, "\n" );
			fprintf(fp, "**%%Rib-Skin connections %d lowerskin \n", r );
			for ( int i = 0 ; i < (int)nodeVec.size() ; i++ )
			{
				if ( pntShift[i] >= 0 )
				{
					if ( nodeVec[i]->HasTag(RIB_ALL, r) && nodeVec[i]->HasTag( SKIN_LOWER ) )
					{
						fprintf(fp, "%d,%f\n", nodeVec[i]->m_Index, nodeVec[i]->m_Thick*m_ThickScale );
					}
				}
			}
		}

		fclose(fp);
	}

//	//==== Write Out All Nodes < YTol ====/
//	double ytol = 0.01;
//	fp = fopen( "fixed.nam", "w" );
//	if ( fp )
//	{
//		fprintf( fp, "**Fixed Nodes\n");
//		fprintf( fp, "*NSET,NSET=NFIXED\n");
//
//		int ncnt = 0;
//		for ( int i = 0 ; i < (int)unique_node_vec.size() ; i++ )
//		{
//			if ( unique_node_vec[i]->m_Pnt[1] < ytol )
//			{
//				fprintf(fp, "%d,", unique_node_vec[i]->m_Index );
//				ncnt++;
//				if ( ncnt%10 == 9 )
//					fprintf(fp, "\n" );
//			}
//		}
//		fprintf(fp, "\n" );
//		fclose(fp);
//	}


}


//void FeaMeshMgr::WriteCalculix( const char* base_filename )
//{
//	//==== Get All FeaNodes ====//
//	vector< FeaNode* > nodeVec;
//	for ( int i = 0 ; i < (int)m_SkinVec.size() ; i++ )
//		m_SkinVec[i]->LoadNodes( nodeVec );
//	for ( int i = 0 ; i < (int)m_SliceVec.size() ; i++ )
//		m_SliceVec[i]->LoadNodes( nodeVec );
//
//	
//	double tol = 0.0001*0.0001;
//	vector< FeaNode* > unique_node_vec;
//	for ( int i = 0 ; i < (int)nodeVec.size() ; i++ )
//	{
//		for ( int j = 0 ; j < (int)unique_node_vec.size() ; j++ )
//		{
//			if ( dist_squared( unique_node_vec[j]->m_Pnt, nodeVec[i]->m_Pnt ) < tol )
//			{
//				nodeVec[i]->m_CloseNode = unique_node_vec[j];
//				break;
//			}
//		}
//		if ( !nodeVec[i]->m_CloseNode )
//			unique_node_vec.push_back( nodeVec[i] );
//	}
//
//	//==== Number Unique Nodes ====//
//	for ( int i = 0 ; i < (int)unique_node_vec.size() ; i++ )
//	{
//		unique_node_vec[i]->m_Index = i+1;
//	}
//
//	Stringc fn( base_filename );
//	fn.concatenate( "geom.dat" );
//	
//	FILE* fp = fopen( fn.get_char_star(), "w" );
//	if ( fp )
//	{
//		fprintf( fp, "*NODE, NSET=Nall\n" );
//		for ( int i = 0 ; i < (int)unique_node_vec.size() ; i++ )
//		{
//			FeaNode* n = unique_node_vec[i];
//			fprintf(fp, "%d,%f,%f,%f\n", n->m_Index, n->m_Pnt.x(), n->m_Pnt.y(), n->m_Pnt.z() );
//		}
//
//		int elem_id = 1;
//		fprintf( fp, "*ELEMENT, TYPE=S6, ELSET=Ealltris\n" );
//		for ( int i = 0 ; i < (int)m_SliceVec.size() ; i++ )
//		{
//			for ( int e = 0 ; e < (int)m_SliceVec[i]->m_Elements.size() ; e++ )
//			{
//				if ( m_SliceVec[i]->m_Elements[e]->GetType() == FeaElement::FEA_TRI_6 )
//				{
//					m_SliceVec[i]->m_Elements[e]->WriteCalculix( fp, elem_id );
//					elem_id++;
//				}
//			}
//		}
//		for ( int i = 0 ; i < (int)m_SkinVec.size() ; i++ )
//		{
//			for ( int e = 0 ; e < (int)m_SkinVec[i]->m_Elements.size() ; e++ )
//			{
//				if ( m_SkinVec[i]->m_Elements[e]->GetType() == FeaElement::FEA_TRI_6 )
//				{
//					m_SkinVec[i]->m_Elements[e]->WriteCalculix( fp, elem_id );
//					elem_id++;
//				}
//			}
//		}
//
//		fprintf( fp, "*ELEMENT, TYPE=S8, ELSET=Eallquads\n" );
//		for ( int i = 0 ; i < (int)m_SliceVec.size() ; i++ )
//		{
//			for ( int e = 0 ; e < (int)m_SliceVec[i]->m_Elements.size() ; e++ )
//			{
//				if ( m_SliceVec[i]->m_Elements[e]->GetType() == FeaElement::FEA_QUAD_8 )
//				{
//					m_SliceVec[i]->m_Elements[e]->WriteCalculix( fp, elem_id );
//					elem_id++;
//				}
//			}
//		}
//		fclose(fp);
//	}
//
//	//==== Write Out All Nodes < YTol ====/
//	double ytol = 0.01;
//	fp = fopen( "fixed.nam", "w" );
//	if ( fp )
//	{
//		fprintf( fp, "**Fixed Nodes\n");
//		fprintf( fp, "*NSET,NSET=NFIXED\n");
//
//		int ncnt = 0;
//		for ( int i = 0 ; i < (int)unique_node_vec.size() ; i++ )
//		{
//			if ( unique_node_vec[i]->m_Pnt[1] < ytol )
//			{
//				fprintf(fp, "%d,", unique_node_vec[i]->m_Index );
//				ncnt++;
//				if ( ncnt%10 == 9 )
//					fprintf(fp, "\n" );
//			}
//		}
//		fprintf(fp, "\n" );
//		fclose(fp);
//	}
//
//	//==== Write Node Thickness Include File ====//
//	Stringc node_fn( base_filename );
//	node_fn.concatenate( "nodethick.dat" );
//	fp = fopen( node_fn.get_char_star(), "w" );
//	if ( fp )
//	{
//		fprintf( fp, "*NODAL THICKNESS\n");
//		for ( int i = 0 ; i < (int)unique_node_vec.size() ; i++ )
//		{
//			fprintf(fp, "%d,%f\n", unique_node_vec[i]->m_Index, unique_node_vec[i]->m_Thick*m_ThickScale );
//		}
//		fclose(fp);
//	}
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
//	//===== Write NASTRAN Out File ====//
//	Stringc filenm( base_filename );
//	filenm.concatenate( "NASTRAN.dat" );
//	fp = fopen( filenm.get_char_star(), "w" );
//	if ( fp )
//	{
//		fprintf(fp, "$--1---><--2---><--3---><--4---><--5---><--6---><--7---><--8---><--9--->\n");
//		fprintf(fp, "$TRIS\n");
//		int elem_id = 1;
//		for ( int i = 0 ; i < (int)m_SliceVec.size() ; i++ )
//		{
//			for ( int e = 0 ; e < (int)m_SliceVec[i]->m_Elements.size() ; e++ )
//			{
//				if ( m_SliceVec[i]->m_Elements[e]->GetType() == FeaElement::FEA_TRI_6 )
//				{
//					m_SliceVec[i]->m_Elements[e]->WriteNASTRAN( fp, elem_id );
//					elem_id++;
//				}
//			}
//		}
//		for ( int i = 0 ; i < (int)m_SkinVec.size() ; i++ )
//		{
//			for ( int e = 0 ; e < (int)m_SkinVec[i]->m_Elements.size() ; e++ )
//			{
//				if ( m_SkinVec[i]->m_Elements[e]->GetType() == FeaElement::FEA_TRI_6 )
//				{
//					m_SkinVec[i]->m_Elements[e]->WriteNASTRAN( fp, elem_id );
//					elem_id++;
//				}
//			}
//		}
//		fprintf(fp, "$--1---><--2---><--3---><--4---><--5---><--6---><--7---><--8---><--9--->\n");
//		fprintf(fp, "$QUADS\n");
//		for ( int i = 0 ; i < (int)m_SliceVec.size() ; i++ )
//		{
//			for ( int e = 0 ; e < (int)m_SliceVec[i]->m_Elements.size() ; e++ )
//			{
//				if ( m_SliceVec[i]->m_Elements[e]->GetType() == FeaElement::FEA_QUAD_8 )
//				{
//					m_SliceVec[i]->m_Elements[e]->WriteNASTRAN( fp, elem_id );
//					elem_id++;
//				}
//			}
//		}
//		//for ( i = 0; i < (int)allTriVec.size() ; i++ )
//		//{
//		//	TTri* tri = &allTriVec[i];
//		//	//fprintf(fp, "%-8s%-8d%-8d%-8d%-8d%-8d%-8d\n", "CTRIA3", i+1, 1, tri->n0->id, tri->n1->id, tri->n2->id, 0 );
//		//	//fprintf(fp, "%-8s%-8d%-8d%-8d%-8d%-8d%-8d\n", "CTRIA3", i+1, 1, tri->cn0->id, tri->cn1->id, tri->cn2->id, 0 );
//		//	fprintf(fp, "%-8s%-8d%-8d%-8d%-8d%-8d%-8d%-8d%-8d%-8d\n", 
//		//		"CTRIA6", i+1, 1, tri->n0->id, tri->n1->id, tri->n2->id, tri->cn0->id, tri->cn2->id, tri->cn1->id,0 );
//		//	//	"CTRIA6", i+1, 1, tri->n0->id, tri->cn0->id, tri->n1->id, tri->cn1->id, tri->n2->id, tri->cn2->id,0 );
//		//}
//		fprintf(fp, "$--1---><--2---><--3---><--4---><--5---><--6---><--7---><--8---><--9--->\n");
//		fprintf(fp, "$NODES\n");
//		for ( int i = 0 ; i < (int)unique_node_vec.size() ; i++ )
//		{
//			FeaNode* n = unique_node_vec[i];
//			fprintf(fp, "%-8s%-8d%-8d%8.3f%8.3f%8.3f\n", "GRID", n->m_Index, 0, n->m_Pnt.x(), n->m_Pnt.y(), n->m_Pnt.z() );
//		}
//
//		//for ( i = 0; i < (int)allNodeVec.size() ; i++ )
//		//{
//		//	TNode* n = allNodeVec[i];
//		//	fprintf(fp, "%-8s%-8d%-8d%8.3f%8.3f%8.3f\n", "GRID", n->id, 0, n->pnt.x(), n->pnt.y(), n->pnt.z() );
//		//}
//		fclose(fp);
//	}
//
//
//}




void FeaMeshMgr::CursorPos(vec2d & cursor)
{
	m_ClosestAttachPoint = -1;
	if ( !m_DrawMeshFlag || !m_DrawAttachPoints  )
		return;
	if ( !m_WingGeom )
		return;
	if ( m_CurrEditType != POINT_MASS_EDIT )
		return;

	int i;
	int nearCPnt = -1;
	double nearDist = 0.05;
	for ( i = 0 ; i < (int)m_AttachPoints.size() ; i++ )
	{
		vec2d p2 = m_WingGeom->projectPoint( m_AttachPoints[i], 0 );
		double dist = dist_squared( cursor, p2 );
		if ( dist < nearDist )
		{
			nearDist = dist;
			nearCPnt = i;
		}
	}
	if ( nearCPnt >= 0 )
	{
		m_ClosestAttachPoint = nearCPnt;
		UpdateGUI();
	}
		
}

void FeaMeshMgr::MouseClick(vec2d & cursor)
{
	CursorPos( cursor );

	if ( !m_DrawMeshFlag || !m_DrawAttachPoints  )
		return;
	if ( !m_WingGeom )
		return;
	if ( m_CurrEditType != POINT_MASS_EDIT )
		return;
	
	FeaPointMass* pm = GetCurrPointMass();
	if ( pm )
	{
		if ( m_ClosestAttachPoint >= 0 && m_ClosestAttachPoint < (int)m_AttachPoints.size() )
		{
			pm->m_AttachPos = m_AttachPoints[m_ClosestAttachPoint];
			m_DrawAttachPoints = false;
			UpdateGUI();
		}
	}
}


void FeaMeshMgr::Draw()
{
	if ( !m_DrawFlag )
		return;

	GLboolean smooth_flag = glIsEnabled( GL_LINE_SMOOTH );
	glDisable( GL_LINE_SMOOTH );
	glDisable( GL_POINT_SMOOTH );

	if ( !m_DrawMeshFlag )
	{
		FeaRib* curr_rib = GetCurrRib();
		for ( int i = 0 ; i < (int)m_WingSections.size() ; i++ )
		{
			for ( int j = 0 ; j < (int)m_WingSections[i].m_RibVec.size() ; j++ )
			{
				FeaRib* rib = m_WingSections[i].m_RibVec[j];
				if ( m_CurrEditType == RIB_EDIT && rib == curr_rib )
					rib->Draw( true );
				else
					rib->Draw( false );
			}
		}

		FeaSpar* curr_spar = GetCurrSpar();
		for ( int i = 0 ; i < (int)m_WingSections.size() ; i++ )
		{
			for ( int j = 0 ; j < (int)m_WingSections[i].m_SparVec.size() ; j++ )
			{
				FeaSpar* spar = m_WingSections[i].m_SparVec[j];
				if ( m_CurrEditType == SPAR_EDIT && spar == curr_spar )
					spar->Draw( true );
				else
					spar->Draw( false );
			}
		}
		
		//==== Wing Sections ====//
		for ( int i = 0 ; i < (int)m_WingSections.size() ; i++ )
		{
			if ( i == m_CurrSectID )
			{
				if ( m_CurrEditType == UP_SKIN_EDIT )
					m_WingSections[i].m_UpperSkin.Draw( true );
				else if ( m_CurrEditType == LOW_SKIN_EDIT )
					m_WingSections[i].m_LowerSkin.Draw( true );
				m_WingSections[i].Draw( true );
			}
			else
			{
				if ( m_CurrEditType == UP_SKIN_EDIT )
					m_WingSections[i].m_UpperSkin.Draw( false );
				else if ( m_CurrEditType == LOW_SKIN_EDIT )
					m_WingSections[i].m_LowerSkin.Draw( false );
				m_WingSections[i].Draw( false );
			}
		}

	}

	if ( m_DrawMeshFlag )
	{
		glPointSize( 6.0 );
		//glColor4ub( 255, 0, 250, 255);
		//glBegin( GL_POINTS );
		//for ( int i = 0 ; i < debugPnts.size() ; i++ )
		//{
		//	glVertex3dv( debugPnts[i].data() );

		//}
		//glEnd();
		//////==== Collect All FeaNodes ====//
		////vector< FeaNode* > nodeVec;
		////for ( int i = 0 ; i < (int)m_SkinVec.size() ; i++ )
		////	m_SkinVec[i]->LoadNodes( nodeVec );
		////for ( int i = 0 ; i < (int)m_SliceVec.size() ; i++ )
		////	m_SliceVec[i]->LoadNodes( nodeVec );

		////for ( int i = 0 ; i < (int)nodeVec.size() ; i++ )
		////{
		////	if ( nodeVec[i]->m_Tags.size() > 0 )
		////	{
		////		glPointSize( 6.0 );
		////		glColor4ub( 255, 0, 250, 255);
		////		glBegin( GL_POINTS );
		////			glVertex3dv( nodeVec[i]->m_Pnt.data() );
		////		glEnd();
		////	}
		////}

		//==== Draw Ribs Spars ====//
		glColor4ub( 0, 0, 250, 255);
		for ( int i = 0 ; i < (int)m_SliceVec.size() ; i++ )
		{
			for ( int e = 0 ; e < (int)m_SliceVec[i]->m_Elements.size() ; e++ )
			{
				FeaElement* fe = m_SliceVec[i]->m_Elements[e];
				glBegin( GL_POLYGON );
				for ( int p = 0 ; p < (int)fe->m_Corners.size() ; p++ )
					glVertex3dv( fe->m_Corners[p]->m_Pnt.data() );
				glEnd();
			}
		}
		glLineWidth(2.0);
		glColor4ub( 0, 0, 0, 255 );
		for ( int i = 0 ; i < (int)m_SliceVec.size() ; i++ )
		{
			for ( int e = 0 ; e < (int)m_SliceVec[i]->m_Elements.size() ; e++ )
			{
				FeaElement* fe = m_SliceVec[i]->m_Elements[e];
				glBegin( GL_LINE_LOOP );
				for ( int p = 0 ; p < (int)fe->m_Corners.size() ; p++ )
					glVertex3dv( fe->m_Corners[p]->m_Pnt.data() );
				glEnd();
			}
		}

		//==== Draw Potential Point Mass Attachment Points ====//
		if ( m_CurrEditType == POINT_MASS_EDIT && m_DrawAttachPoints )
		{
			glPointSize(6.0);
			glColor4ub( 100, 100, 100, 255 );
			glBegin( GL_POINTS );
			for ( int i = 0 ; i < (int)m_AttachPoints.size() ; i++ )
			{
				if ( i != m_ClosestAttachPoint )
				{
					glVertex3dv( m_AttachPoints[i].data() );
				}	
			}
			glEnd();

			if ( m_ClosestAttachPoint >= 0 && m_ClosestAttachPoint < (int)m_AttachPoints.size() )
			{
				glPointSize(8.0);
				glColor4ub( 255, 0, 0, 255 );
				glBegin( GL_POINTS );
				glVertex3dv( m_AttachPoints[m_ClosestAttachPoint].data() );
				glEnd();
			}
		}

		//==== Draw Skin ====//
		glCullFace( GL_BACK );						// Cull Back Faces For Trans
		glEnable( GL_CULL_FACE );

		glColor4ub( 150, 150, 150, 50 );
		for ( int i = 0 ; i < (int)m_SkinVec.size() ; i++ )
		{
			for ( int e = 0 ; e < (int)m_SkinVec[i]->m_Elements.size() ; e++ )
			{
				FeaElement* fe = m_SkinVec[i]->m_Elements[e];
				glBegin( GL_POLYGON );
				for ( int p = 0 ; p < (int)fe->m_Corners.size() ; p++ )
					glVertex3dv( fe->m_Corners[p]->m_Pnt.data() );
				glEnd();
			}
		}
		glLineWidth(2.0);
		glColor4ub( 0, 0, 0, 100 );
		for ( int i = 0 ; i < (int)m_SkinVec.size() ; i++ )
		{
			for ( int e = 0 ; e < (int)m_SkinVec[i]->m_Elements.size() ; e++ )
			{
				FeaElement* fe = m_SkinVec[i]->m_Elements[e];
				glBegin( GL_LINE_LOOP );
				for ( int p = 0 ; p < (int)fe->m_Corners.size() ; p++ )
					glVertex3dv( fe->m_Corners[p]->m_Pnt.data() );
				glEnd();
			}
		}

		glDisable( GL_CULL_FACE );
	}

	//==== Draw Potential Point Mass Attachment Points ====//
	if ( m_CurrEditType == POINT_MASS_EDIT  )
	{
		for ( int i = 0 ; i < (int)m_PointMassVec.size() ; i++ )
		{
			if ( i == m_CurrPointMassID )
				m_PointMassVec[i]->Draw( true );
			else
				m_PointMassVec[i]->Draw( false );
		}
	}





	if ( smooth_flag )
	{
		glEnable( GL_LINE_SMOOTH );
		glEnable( GL_POINT_SMOOTH );
	}

}

