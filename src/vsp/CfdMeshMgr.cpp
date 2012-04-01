//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//

// CfdMeshMgr
//
//////////////////////////////////////////////////////////////////////

#include "CfdMeshMgr.h"
#include "cfdMeshScreen.h"
#include "feaStructScreen.h"
#include "geom.h"
#include "aircraft.h"
#include "SurfPatch.h"
#include "Tri.h"

#ifdef DEBUG_CFD_MESH
#include <direct.h>
#endif


//==== Singleton ====//
CFDM_Single::CFDM_Single()
{
	static CfdMeshMgr* ptr = 0; 

    if (!ptr) 
		ptr = new CfdMeshMgr();

	cfdMeshMgr = ptr;     
}

//=============================================================//
//=============================================================//




CfdMeshMgr::CfdMeshMgr()
{
	m_CurrGeomID = 0;
	m_HighlightChainIndex = 0;
	m_DrawMeshFlag = true;
	m_DrawSourceFlag = true;
	m_BatchFlag = false;
	m_HalfMeshFlag = false;
	m_FarXScale = m_FarYScale = m_FarZScale = 4.0;

	m_YSlicePlane = new Surf();

#ifdef DEBUG_CFD_MESH
	m_DebugDir  = Stringc("MeshDebug/");
	_mkdir( m_DebugDir.get_char_star() );
	m_DebugFile = fopen( "MeshDebug/log.txt", "w" );
	m_DebugDraw = false;
#endif
}

CfdMeshMgr::~CfdMeshMgr()
{
	CleanUp();
	delete m_YSlicePlane;

#ifdef DEBUG_CFD_MESH
	if (m_DebugFile)
		fclose( m_DebugFile );
#endif

}

void CfdMeshMgr::CleanUp()
{
	int i;
	//==== Delete Old Surfs ====//
	for ( i = 0 ; i < (int)m_SurfVec.size() ; i++ )
		delete m_SurfVec[i];
	m_SurfVec.clear();

	//==== Delete ICurves =====//
	for ( i = 0 ; i < (int)m_ICurveVec.size() ; i++ )
		delete m_ICurveVec[i];
	m_ICurveVec.clear();

	//==== Delete Seg Chains ====//
	list< ISegChain* >::iterator cl;
	for ( cl = m_ISegChainList.begin() ; cl != m_ISegChainList.end(); cl++ )
		delete (*cl);
	m_ISegChainList.clear();

	//==== Delete Puws ====//
	for ( i = 0 ; i < (int)m_DelPuwVec.size() ; i++ )
		delete m_DelPuwVec[i];
	m_DelPuwVec.clear();

	//==== Delete IPnts ====//
	for ( i = 0 ; i < (int)m_DelIPntVec.size() ; i++ )
		delete m_DelIPntVec[i];
	m_DelIPntVec.clear();

	//==== Delete IPntGroups ====//
	for ( i = 0 ; i < (int)m_DelIPntGroupVec.size() ; i++ )
		delete m_DelIPntGroupVec[i];
	m_DelIPntGroupVec.clear();

	//==== Delete IPntGroups ====//
	for ( i = 0 ; i < (int)m_DelISegChainVec.size() ; i++ )
		delete m_DelISegChainVec[i];
	m_DelISegChainVec.clear();

	m_BinMap.clear();

	debugPnts.clear();

}

Stringc CfdMeshMgr::GetExportFileName( int type )
{
	if ( type >= 0 && type < NUM_FILE_NAMES )
		return m_ExportFileNames[type];

	return Stringc();
}

void CfdMeshMgr::SetExportFileName( const char* fn, int type )
{
	if ( type >= 0 && type < NUM_FILE_NAMES )
		m_ExportFileNames[type] = Stringc(fn);
}

bool CfdMeshMgr::GetExportFileFlag( int type )
{
	if ( type >= 0 && type < NUM_FILE_NAMES )
		return m_ExportFileFlags[type];

	return false;
}

void CfdMeshMgr::SetExportFileFlag( bool flag, int type )
{
	if ( type >= 0 && type < NUM_FILE_NAMES )
		m_ExportFileFlags[type] = flag;
}

void CfdMeshMgr::ResetExportFileNames()
{
	int pos;
	char *suffix[]={".stl",".poly",".tri",".obj", "_NASCART.dat", "_NASCART.key", ".msh", ".srf"};

	for ( int i = 0 ; i < NUM_FILE_NAMES ; i++ )
	{
		m_ExportFileNames[i] = aircraftPtr->getFileName();
		pos = m_ExportFileNames[i].search_for_substring(".vsp");
		if (pos >= 0)
			m_ExportFileNames[i].delete_range(pos, m_ExportFileNames[i].get_length()-1);
		m_ExportFileNames[i].concatenate(suffix[i]);
	}
}

void CfdMeshMgr::addOutputText( const char* str, int output_type )
{
        ScreenMgr* screenMgr = aircraftPtr->getScreenMgr();
        if ( screenMgr )
        {
                if ( output_type == CFD_OUTPUT )
                        screenMgr->getCfdMeshScreen()->addOutputText( str );
                else if ( output_type == FEA_OUTPUT )
                        screenMgr->getFeaStructScreen()->addOutputText( str );
        }
        else
        {
                printf( str );
                fflush( stdout );
        }
}

void CfdMeshMgr::AdjustAllSourceLen( double mult )
{
	vector< Geom* > geomVec = aircraftPtr->getGeomVec();	
	for ( int g = 0 ; g < (int)geomVec.size() ; g++ )
	{
		vector< BaseSource* > sVec = geomVec[g]->getCfdMeshSourceVec();
		for ( int s = 0 ; s < (int)sVec.size() ; s++ )
		{
			sVec[s]->AdjustLen(mult);
		}
	}
}

void CfdMeshMgr::AdjustAllSourceRad( double mult )
{
	vector< Geom* > geomVec = aircraftPtr->getGeomVec();	
	for ( int g = 0 ; g < (int)geomVec.size() ; g++ )
	{
		vector< BaseSource* > sVec = geomVec[g]->getCfdMeshSourceVec();
		for ( int s = 0 ; s < (int)sVec.size() ; s++ )
		{
			sVec[s]->AdjustRad(mult);
		}
	}
}

void CfdMeshMgr::GUI_Val( Stringc name, double val )
{
	BaseSource* source = GetCurrSource();
	if ( name == "GlobalEdgeSize"  )
		m_GridDensity.SetBaseLen( val );
	else if ( name == "MinEdgeSize"  )
		m_GridDensity.SetMinLen( val );
	else if ( name == "MaxGapSize"  )
		m_GridDensity.SetMaxGap( val );
	else if ( name == "NumCircSeg"  )
		m_GridDensity.SetNCircSeg( val );
	else if ( name == "GrowRatio"  )
		m_GridDensity.SetGrowRatio( val );
	else if ( name == "Length1" && source )
		source->SetLen( val );
	else if ( name == "Radius1" && source )
		source->SetRad( val );
	else
	{
		if ( source ) 
			source->SetNamedVal( name, val );
	}
}

void CfdMeshMgr::GUI_Val( Stringc name, int val )
{
	if ( name == "SourceID" )
	{
		vector< Geom* > geomVec = aircraftPtr->getGeomVec();	
		if ( m_CurrGeomID >= 0 && m_CurrGeomID < (int)geomVec.size() )
		{
			vector< BaseSource* > sVec = geomVec[m_CurrGeomID]->getCfdMeshSourceVec();
			if ( val >= 0 && val < (int)sVec.size() )
				geomVec[m_CurrGeomID]->SetCurrSourceID( val );
		}
	}	
	aircraftPtr->triggerDraw();

}

void CfdMeshMgr::GUI_Val( Stringc name, Stringc val )
{
	if ( name == "SourceName" )
	{
		BaseSource* s = GetCurrSource();
		if ( s )
			s->SetName( val );
	}
}

BaseSource* CfdMeshMgr::GetCurrSource()
{
	BaseSource* s = NULL;
	vector< Geom* > geomVec = aircraftPtr->getGeomVec();	
	if ( m_CurrGeomID >= 0 && m_CurrGeomID < (int)geomVec.size() )
	{
		int sourceID = geomVec[m_CurrGeomID]->GetCurrSourceID();
 		vector< BaseSource* > sVec = geomVec[m_CurrGeomID]->getCfdMeshSourceVec();

		if ( sourceID >= 0 && sourceID < (int)sVec.size() )
				s = sVec[sourceID];
	}
	return s;
}
	
BaseSource* CfdMeshMgr::CreateSource( int type )
{
	if ( type == BaseSource::POINT_SOURCE )
	{
		return new PointSource();
	}
	else if ( type == BaseSource::LINE_SOURCE )
	{
		return new LineSource();
	}
	else if ( type == BaseSource::BOX_SOURCE )
	{
		return new BoxSource();
	}

	return NULL;
}

void CfdMeshMgr::AddSource( int type )
{
	Geom* curr_geom = NULL;
	vector< Geom* > geomVec = aircraftPtr->getGeomVec();	
	if ( m_CurrGeomID >= 0 && m_CurrGeomID < (int)geomVec.size() )
		curr_geom = geomVec[m_CurrGeomID];

	if ( !curr_geom )
		return;

	char str[256];
	int num_sources = curr_geom->getCfdMeshSourceVec().size();

	if ( type == BaseSource::POINT_SOURCE )
	{
		PointSource* source = new PointSource();
		sprintf( str, "PointSource_%d", num_sources );
		source->SetName( str );
		source->SetLen( 0.1 );
		source->SetRad( 1.0 );
		vec2d l = vec2d( 0.0, 0.0 );
		source->SetUWLoc( l );
		curr_geom->AddCfdMeshSource( source );
	}
	else if ( type == BaseSource::LINE_SOURCE )
	{
		LineSource* source = new LineSource();
		sprintf( str, "LineSource_%d", num_sources );
		source->SetName( str );
		source->SetLen( 0.1 );
		source->SetLen2( 0.1 );
		source->SetRad( 1.0 );
		source->SetRad2( 2.0 );
		vec2d l1 = vec2d( 0.0, 0.0 );
		source->SetUWLoc1( l1 );
		vec2d l2 = vec2d( 1.0, 0.0 );
		source->SetUWLoc2( l2 );

		curr_geom->AddCfdMeshSource( source );
		vector< BaseSource* > sVec = curr_geom->getCfdMeshSourceVec();
		geomVec[m_CurrGeomID]->SetCurrSourceID( (int)sVec.size()-1 );
	}
	else if ( type == BaseSource::BOX_SOURCE )
	{
		BoxSource* source = new BoxSource();
		sprintf( str, "BoxSource_%d", num_sources );
		source->SetName( str );
		source->SetLen( 0.1 );
		source->SetRad( 1.0 );
		vec2d l1 = vec2d( 0.0, 0.0 );
		source->SetUWLoc1( l1 );
		vec2d l2 = vec2d( 1.0, 0.0 );
		source->SetUWLoc2( l2 );

		curr_geom->AddCfdMeshSource( source );
		vector< BaseSource* > sVec = curr_geom->getCfdMeshSourceVec();
		geomVec[m_CurrGeomID]->SetCurrSourceID( (int)sVec.size()-1 );
	}

	//==== Highlight/Edit The New Source ====//
	vector< BaseSource* > sVec = curr_geom->getCfdMeshSourceVec();
	geomVec[m_CurrGeomID]->SetCurrSourceID( (int)sVec.size()-1 );

}

void CfdMeshMgr::DeleteCurrSource()
{
	Geom* curr_geom = NULL;
	vector< Geom* > geomVec = aircraftPtr->getGeomVec();	
	if ( m_CurrGeomID >= 0 && m_CurrGeomID < (int)geomVec.size() )
		curr_geom = geomVec[m_CurrGeomID];

	if ( curr_geom )
		curr_geom->DelCurrSource();
}

void CfdMeshMgr::UpdateSources()
{
	m_GridDensity.ClearSources();

	vector< Geom* > geomVec = aircraftPtr->getGeomVec();	
	for ( int g = 0 ; g < (int)geomVec.size() ; g++ )
	{
		geomVec[g]->UpdateSources();
		vector< BaseSource* > sVec = geomVec[g]->getCfdMeshSourceVec();

		for ( int s = 0 ; s < (int)sVec.size() ; s++ )
		{
			m_GridDensity.AddSource( sVec[s] );
			if ( sVec[s]->GetReflSource() )
				m_GridDensity.AddSource( sVec[s]->GetReflSource() );
		}
	}
}

void CfdMeshMgr::AddDefaultSources()
{
	if ( m_GridDensity.GetNumSources() == 0 )
	{
		vector< Geom* > geomVec = aircraftPtr->getGeomVec();	
		for ( int g = 0 ; g < (int)geomVec.size() ; g++ )
		{
			double base_len = m_GridDensity.GetBaseLen();
			geomVec[g]->AddDefaultSources(base_len);
		}
	}
}

void CfdMeshMgr::ScaleTriSize( double scale )
{
	m_GridDensity.SetBaseLen( scale*m_GridDensity.GetBaseLen() );
	m_GridDensity.SetMinLen( scale*m_GridDensity.GetMinLen() );
	m_GridDensity.SetMaxGap( scale*m_GridDensity.GetMaxGap() );
	m_GridDensity.SetNCircSeg( m_GridDensity.GetNCircSeg()/scale );
	m_GridDensity.ScaleAllSources( scale );
}

void CfdMeshMgr::WriteSurfs( const char* filename )
{
	if ( m_HalfMeshFlag )
	{
		vector< Geom* > changedSymGeomVec;
		vector< Geom* > movedGeomVec;
		vector< Geom* > geomVec = aircraftPtr->getGeomVec();
		for ( int i = 0 ; i < (int)geomVec.size() ; i++ )
		{
			if ( geomVec[i]->getSymCode() == XZ_SYM )
			{
				if ( geomVec[i]->yLoc() < 0.0 )
				{
					geomVec[i]->yLoc.set( -geomVec[i]->yLoc() );
					movedGeomVec.push_back( geomVec[i] );
				}

				geomVec[i]->setSymCode( NO_SYM );
				changedSymGeomVec.push_back( geomVec[i] );
			}
		}
		aircraftPtr->write_bezier_file( filename );

		//==== Restore Changes =====//
		for ( int i = 0 ; i < (int)changedSymGeomVec.size() ; i++ )
			changedSymGeomVec[i]->setSymCode( XZ_SYM );

		for ( int i = 0 ; i < (int)movedGeomVec.size() ; i++ )
			movedGeomVec[i]->yLoc.set( -movedGeomVec[i]->yLoc() );
	}
	else
	{
		aircraftPtr->write_bezier_file( filename );
	}
}

	
void CfdMeshMgr::ReadSurfs( const char* filename )
{
	FILE* file_id = fopen(filename, "r");
	if ( file_id )
	{
		char buff[256];
		int comp_id, num_surfs;

		fgets( buff, 256, file_id );			sscanf( buff, "%d", &m_NumComps );

		int total_surfs = 0;
		for ( int c = 0 ; c < m_NumComps ; c++ )
		{
			fgets( buff, 256, file_id );			sscanf( buff, "%d", &comp_id );
			fgets( buff, 256, file_id );			sscanf( buff, "%d", &num_surfs );

			for ( int s = 0 ; s < num_surfs ; s++ )
			{
				Surf* surfPtr = new Surf();

				surfPtr->SetCfdMeshMgr(this);
				surfPtr->SetCompID( c );
				surfPtr->SetSurfID( s + total_surfs );

				surfPtr->ReadSurf( file_id );

				bool addSurfFlag = true;
				if ( m_HalfMeshFlag && surfPtr->LessThanY( -0.01 ) )
					addSurfFlag = false;

				if ( m_HalfMeshFlag && surfPtr->PlaneAtYZero() )
					addSurfFlag = false;

				if ( addSurfFlag )
					m_SurfVec.push_back( surfPtr );
				else
					delete surfPtr;
			}
			total_surfs += num_surfs;
		}
		fclose( file_id );
	}

	//==== Combine Components With Matching Surface Edges ====//
	map< int, int > mergeCompMap;
	for ( int s = 0 ; s < (int)m_SurfVec.size() ; s++ )
	{
		for ( int t = s+1 ; t < (int)m_SurfVec.size() ; t++ )
		{
			int compA = m_SurfVec[s]->GetCompID();
			int compB = m_SurfVec[t]->GetCompID();
			if ( compA != compB && m_SurfVec[s]->BorderMatch( m_SurfVec[t] ) )
			{
				mergeCompMap[compB] = compA;
			}
		}
	}
	//==== Change Comp IDs ====//
	for ( int s = 0 ; s < (int)m_SurfVec.size() ; s++ )
	{
		int compID = m_SurfVec[s]->GetCompID();
		if ( mergeCompMap.find( compID ) != mergeCompMap.end() )
		{
			int newCompID = mergeCompMap[compID];
			m_SurfVec[s]->SetCompID( newCompID );
		}
	}
}
	
void CfdMeshMgr::BuildGrid()
{

	int i, j;
	for ( i = 0 ; i < (int)m_SurfVec.size() ; i++ )
	{
		m_SurfVec[i]->BuildDistMap();
		m_SurfVec[i]->SetGridDensityPtr( &m_GridDensity );
		m_SurfVec[i]->FindBorderCurves();
	}

	//==== Find Matching Intersection/Border Curves =====//
	vector< SCurve* > scurve_vec;
	for ( i = 0 ; i < (int)m_SurfVec.size() ; i++ )
	{
		m_SurfVec[i]->LoadSCurves( scurve_vec );
	}

	for ( i = 0 ; i < (int)scurve_vec.size() ; i++ )
	{
		for ( j = i+1 ; j < (int)scurve_vec.size() ; j++ )
		{
			ICurve* icrv = new ICurve;
			if ( icrv->Match( scurve_vec[i], scurve_vec[j] ) )
			{
				m_ICurveVec.push_back( icrv );
			}
			else
			{
				delete icrv;
			}
		}
	}

	//==== Check For SCurves Not Matched ====//
	int num_unmatched = 0;
	for ( i = 0 ; i < (int)scurve_vec.size() ; i++ )
	{
		if ( !scurve_vec[i]->GetICurve() )
		{
			ICurve* icrv = new ICurve;
			icrv->SetACurve( scurve_vec[i] );
			m_ICurveVec.push_back( icrv );
			num_unmatched++;
		}
	}

#ifdef DEBUG_CFD_MESH
	fprintf( m_DebugFile, "CfdMeshMgr::BuildGrid \n");
	fprintf( m_DebugFile, "  Num unmatched SCurves = %d \n", num_unmatched );

	for ( int i = 0 ; i < (int)m_ICurveVec.size() ; i++ )
	{
		m_ICurveVec[i]->DebugEdgeMatching(m_DebugFile);
	}

#endif
}

void CfdMeshMgr::Remesh(int output_type)
{
	char str[256];
	int total_num_tris = 0;
	int nsurf = (int)m_SurfVec.size();
	for ( int i = 0 ; i < nsurf ; i++ )
	{
		int num_tris = 0;

		for ( int iter = 0 ; iter < 10 ; iter++ )
		{
			m_SurfVec[i]->GetMesh()->m_Iteration = iter;
			num_tris = 0;
			m_SurfVec[i]->GetMesh()->Remesh();

			num_tris += m_SurfVec[i]->GetMesh()->GetTriList().size();

			sprintf(str, "Surf %d/%d Iter %d/10 Num Tris = %d\n", i+1, nsurf, iter+1, num_tris );
			if ( output_type != CfdMeshMgr::NO_OUTPUT )
				addOutputText( str, output_type );
		}
		total_num_tris += num_tris;
		
		m_SurfVec[i]->GetMesh()->LoadSimpTris();
		m_SurfVec[i]->GetMesh()->Clear();
		m_SurfVec[i]->GetMesh()->CondenseSimpTris();
	}

	sprintf(str, "Total Num Tris = %d\n", total_num_tris );
	addOutputText( str, output_type );
}

void CfdMeshMgr::RemeshSingleComp(int comp_id, int output_type)
{
	char str[256];
	int total_num_tris = 0;
	int nsurf = (int)m_SurfVec.size();
	for ( int i = 0 ; i < nsurf ; i++ )
	{
		int num_tris = 0;
		if ( m_SurfVec[i]->GetCompID() == comp_id )
		{
			for ( int iter = 0 ; iter < 10 ; iter++ )
			{
				num_tris = 0;
				m_SurfVec[i]->GetMesh()->m_Iteration = iter;
				m_SurfVec[i]->GetMesh()->Remesh();

				num_tris += m_SurfVec[i]->GetMesh()->GetTriList().size();

				sprintf(str, "Surf %d/%d Iter %d/10 Num Tris = %d\n", i+1, nsurf, iter+1, num_tris );
				addOutputText( str, output_type );
			}
			total_num_tris += num_tris;
		}
		
		m_SurfVec[i]->GetMesh()->LoadSimpTris();
		m_SurfVec[i]->GetMesh()->Clear();
		m_SurfVec[i]->GetMesh()->CondenseSimpTris();
	}

	sprintf(str, "Total Num Tris = %d\n", total_num_tris );
	addOutputText( str, output_type );
}

Stringc CfdMeshMgr::GetQualString()
{
	//list< Tri* >::iterator t;
	//int total_num_tris = 0;
	//int num_bad_tris[5];					// Small Angles 0-5, 5-10, 10-15, 15-20, 20-25

	//for ( int i = 0 ; i < 5 ; i++ )	
	//	num_bad_tris[i] = 0;

	//for ( int i = 0 ; i < (int)m_SurfVec.size() ; i++ )
	//{
	//	list< Tri* > tlist = m_SurfVec[i]->GetMesh()->GetTriList();

	//	for ( t = tlist.begin() ; t != tlist.end(); t++ )
	//	{
	//		total_num_tris++;
	//		double q = (*t)->ComputeQual()*180.0/M_PI;
	//		if ( q < 5.0 )			num_bad_tris[0]++;
	//		else if ( q < 10.0 )	num_bad_tris[1]++;
	//		else if ( q < 15.0 )	num_bad_tris[2]++;
	//		else if ( q < 20.0 )	num_bad_tris[3]++;
	//		else if ( q < 25.0 )	num_bad_tris[4]++;
	//	}
	//}
	//Stringc ret_string;
	//if ( total_num_tris > 0 )
	//{
	//	char str[512];
	//	sprintf( str, "0-5 Deg = %3.5f, 5-10=%3.5f   10-15=%3.5f   15-20=%3.5f   20-25=%3.5f\n",  
	//		(float)num_bad_tris[0]/(float)total_num_tris,  (float)num_bad_tris[1]/(float)total_num_tris,
	//		(float)num_bad_tris[2]/(float)total_num_tris,  (float)num_bad_tris[3]/(float)total_num_tris,
	//		(float)num_bad_tris[4]/(float)total_num_tris );

	//	ret_string = str;
	//}
	//return ret_string;
	return "";
}

void CfdMeshMgr::PrintQual()
{
}

void CfdMeshMgr::ExportFiles()
{
	if ( m_ExportFileFlags[STL_FILE_NAME] )
		WriteSTL( m_ExportFileNames[STL_FILE_NAME] );
	if ( m_ExportFileFlags[POLY_FILE_NAME] )
		WriteTetGen( m_ExportFileNames[POLY_FILE_NAME] );

	char* dat_fn = NULL;
	if (  m_ExportFileFlags[DAT_FILE_NAME] )	dat_fn = m_ExportFileNames[DAT_FILE_NAME];
	char* key_fn = NULL;
	if (  m_ExportFileFlags[KEY_FILE_NAME] )	key_fn = m_ExportFileNames[KEY_FILE_NAME];
	char* obj_fn = NULL;
	if (  m_ExportFileFlags[OBJ_FILE_NAME] )	obj_fn = m_ExportFileNames[OBJ_FILE_NAME];
	char* tri_fn = NULL;
	if (  m_ExportFileFlags[TRI_FILE_NAME] )	tri_fn = m_ExportFileNames[TRI_FILE_NAME];
	char* gmsh_fn = NULL;
	if (  m_ExportFileFlags[GMSH_FILE_NAME] )	gmsh_fn = m_ExportFileNames[GMSH_FILE_NAME];

	WriteNASCART_Obj_Tri_Gmsh( dat_fn, key_fn, obj_fn, tri_fn, gmsh_fn );

	if ( m_ExportFileFlags[SRF_FILE_NAME] )
		WriteSurfsIntCurves( m_ExportFileNames[SRF_FILE_NAME] );
}

void CfdMeshMgr::WriteSTL( const char* filename )
{
	FILE* file_id = fopen(filename, "w");
	if ( file_id )
	{
		fprintf(file_id, "solid\n");
		for ( int i = 0 ; i < (int)m_SurfVec.size() ; i++ )
		{
			m_SurfVec[i]->GetMesh()->WriteSTL( file_id );
		}
		fprintf(file_id, "endsolid\n");
		fclose(file_id);
	}
}

void CfdMeshMgr::WriteTetGen( const char* filename )
{
	FILE* fp = fopen(filename, "w");
	if ( !fp )
		return;

	int tri_cnt = 0;
	vector< vec3d* > allPntVec;
	for ( int i = 0 ; i < (int)m_SurfVec.size() ; i++ )
	{
		vector< vec3d >& sPntVec = m_SurfVec[i]->GetMesh()->GetSimpPntVec(); 
		for ( int v = 0 ; v < (int)sPntVec.size() ; v++ )
		{
			allPntVec.push_back( &sPntVec[v] );
		}
		tri_cnt += m_SurfVec[i]->GetMesh()->GetSimpTriVec().size();
	}

	//==== Force Sliced Point onto Plane ====//
	if ( m_HalfMeshFlag )
	{
		for ( int i = 0 ; i < (int)allPntVec.size() ; i++ )
		{
			if ( fabs( allPntVec[i]->y() ) < 0.0001 )
				allPntVec[i]->set_y( 0.0 );
		}
	}

	//==== Build Map ====//
	map< int, vector< int > > indMap;
	vector< int > pntShift;
	int numPnts = BuildIndMap( allPntVec, indMap, pntShift );

	//===== Write Num Pnts and Tris ====//
	fprintf( fp, "# Part 1 - node list\n");
	fprintf( fp, "%d 3 0 0\n", numPnts + 8 );	// 8 Nodes For Far Field Box

	//==== Write Far Pnts ====//
	bbox box = aircraftPtr->getBndBox();
	vec3d cent = box.get_center();
	double max_d = 0.5*box.get_largest_dim();
	vec3d offset( max_d*m_FarXScale, max_d*m_FarYScale, max_d*m_FarZScale );
	vec3d far_min = cent - offset;
	vec3d far_max = cent + offset;

	if ( m_HalfMeshFlag )
		far_min.set_y( 0.0 );

	fprintf( fp, "1 %.16g %.16g %.16g\n", far_min.x(), far_min.y(), far_min.z() );
	fprintf( fp, "2 %.16g %.16g %.16g\n", far_max.x(), far_min.y(), far_min.z() );
	fprintf( fp, "3 %.16g %.16g %.16g\n", far_max.x(), far_max.y(), far_min.z() );
	fprintf( fp, "4 %.16g %.16g %.16g\n", far_min.x(), far_max.y(), far_min.z() );
	fprintf( fp, "5 %.16g %.16g %.16g\n", far_min.x(), far_min.y(), far_max.z() );
	fprintf( fp, "6 %.16g %.16g %.16g\n", far_max.x(), far_min.y(), far_max.z() );
	fprintf( fp, "7 %.16g %.16g %.16g\n", far_max.x(), far_max.y(), far_max.z() );
	fprintf( fp, "8 %.16g %.16g %.16g\n", far_min.x(), far_max.y(), far_max.z() );

	//==== Write Model Pnts ====//
	for ( int i = 0 ; i < (int)allPntVec.size() ; i++ )
	{
		if ( pntShift[i] >= 0 )
			fprintf( fp, "%d %.16g %.16g %.16g\n", i+1+8, allPntVec[i]->x(), allPntVec[i]->y(), allPntVec[i]->z() );
	}

	vector< int > planeIndVec;
	if ( m_HalfMeshFlag )
	{
		for ( int i = 0 ; i < (int)allPntVec.size() ; i++ )
		{
			if ( pntShift[i] >= 0 && fabs( allPntVec[i]->y() ) < 0.000001 )
				planeIndVec.push_back( i );
		}
	}

	//==== Write Tris ====//
	fprintf( fp, "# Part 2 - facet list\n");
	fprintf( fp, "%d 0\n", tri_cnt + 6 );		// 6 Cube Faces
	if ( !m_HalfMeshFlag )
	{
		fprintf( fp, "1\n" );
		fprintf( fp, "4  1 2 6 5\n" );
	}
	else
	{
		fprintf( fp, "%d\n", planeIndVec.size() + 1 );
		fprintf( fp, "4  1 2 6 5\n" );
		for ( int i = 0 ; i < (int)planeIndVec.size() ; i++ )
		{
			int a_ind = planeIndVec[i];
			int ind = FindPntIndex( *allPntVec[a_ind], allPntVec, indMap );
			int shift_ind = pntShift[ind] + 1 + 8;
			fprintf( fp, "1  %d\n", shift_ind);
		}
	}
	fprintf( fp, "1\n" );
	fprintf( fp, "4  5 6 7 8\n" );
	fprintf( fp, "1\n" );
	fprintf( fp, "4  1 2 3 4\n" );
	fprintf( fp, "1\n" );
	fprintf( fp, "4  2 3 7 6\n" );
	fprintf( fp, "1\n" );
	fprintf( fp, "4  3 4 8 7\n" );
	fprintf( fp, "1\n" );
	fprintf( fp, "4  4 1 5 8\n" );
	 
	for ( int i = 0 ; i < (int)m_SurfVec.size() ; i++ )
	{
		vector < SimpTri >& sTriVec = m_SurfVec[i]->GetMesh()->GetSimpTriVec();
		vector< vec3d >& sPntVec = m_SurfVec[i]->GetMesh()->GetSimpPntVec(); 
		for ( int t = 0 ; t <  (int)sTriVec.size() ; t++ )
		{
			int i0 = FindPntIndex( sPntVec[sTriVec[t].ind0], allPntVec, indMap );
			int i1 = FindPntIndex( sPntVec[sTriVec[t].ind1], allPntVec, indMap );
			int i2 = FindPntIndex( sPntVec[sTriVec[t].ind2], allPntVec, indMap );
			int ind1 = pntShift[i0] + 1 + 8;
			int ind2 = pntShift[i1] + 1 + 8;
			int ind3 = pntShift[i2] + 1 + 8;

			fprintf( fp, "1\n" );
			fprintf( fp, "3 %d %d %d\n", ind1, ind2, ind3, m_SurfVec[i]->GetCompID()+1 );
		}
	}

	fprintf( fp, "# Part 3 - Hole List\n");

	vector< Geom* > geomVec = aircraftPtr->getGeomVec();
	vector< vec3d > interiorPntVec; 
	for ( int i = 0 ; i < (int)geomVec.size() ; i++ )
	{
		if ( geomVec[i]->getOutputFlag() )
			geomVec[i]->GetInteriorPnts( interiorPntVec );
	}

	if ( m_HalfMeshFlag )
	{
		vector< vec3d > tmpPntVec;
		for ( int i = 0 ; i < (int)interiorPntVec.size() ; i++ )
		{
			if ( fabs( interiorPntVec[i].y() ) < 0.0001 )
				interiorPntVec[i].set_y( 0.00001 );

			if ( interiorPntVec[i].y() > 0.0 )
				tmpPntVec.push_back( interiorPntVec[i] );
		}
		interiorPntVec = tmpPntVec;
	}

	fprintf( fp, "%d\n", interiorPntVec.size() );
	for ( int i = 0 ; i < (int)interiorPntVec.size() ; i++ )
	{
		vec3d p = interiorPntVec[i];
		fprintf( fp, "%d  %.16g %.16g %.16g\n", i+1, p.x(), p.y(), p.z()  );
	}

	fprintf( fp, "# Part 4 - region list\n");
	fprintf( fp, "0\n" );
	
	fclose( fp );
}
	
void CfdMeshMgr::WriteNASCART_Obj_Tri_Gmsh( const char* dat_fn, const char* key_fn, const char* obj_fn, const char* tri_fn, const char* gmsh_fn )
{
#ifdef DEBUG_CFD_MESH
	//==== Find Smallest Edge ====//
	double small_edge = 1.0e12;
	for ( int i = 0 ; i < (int)m_SurfVec.size() ; i++ )
	{
		vector < SimpTri >& sTriVec = m_SurfVec[i]->GetMesh()->GetSimpTriVec();
		vector< vec3d >& sPntVec = m_SurfVec[i]->GetMesh()->GetSimpPntVec(); 
		for ( int t = 0 ; t <  (int)sTriVec.size() ; t++ )
		{
			double el0 = dist_squared( sPntVec[sTriVec[t].ind0], sPntVec[sTriVec[t].ind1] );
			double el1 = dist_squared( sPntVec[sTriVec[t].ind1], sPntVec[sTriVec[t].ind2] );
			double el2 = dist_squared( sPntVec[sTriVec[t].ind2], sPntVec[sTriVec[t].ind0] );
			if ( el0 < small_edge ) small_edge = el0;
			if ( el1 < small_edge ) small_edge = el1;
			if ( el2 < small_edge ) small_edge = el2;
		}
	}

	fprintf( m_DebugFile, "CfdMeshMgr::WriteNASCART Small Edge Length = %f \n", sqrt(small_edge) );
#endif

	if ( !dat_fn && !key_fn && !obj_fn && !tri_fn )
		return;

	int tri_cnt = 0;
	vector< vec3d* > allPntVec;
	for ( int i = 0 ; i < (int)m_SurfVec.size() ; i++ )
	{
		vector< vec3d >& sPntVec = m_SurfVec[i]->GetMesh()->GetSimpPntVec(); 
		for ( int v = 0 ; v < (int)sPntVec.size() ; v++ )
		{
			allPntVec.push_back( &sPntVec[v] );
		}
		tri_cnt += m_SurfVec[i]->GetMesh()->GetSimpTriVec().size();
	}

	//==== Build Map ====//
	map< int, vector< int > > indMap;
	vector< int > pntShift;
	int numPnts = BuildIndMap( allPntVec, indMap, pntShift );

	//==== Open file ====//
	if ( dat_fn )
	{
		FILE* fp = fopen(dat_fn, "w");

		if ( fp )
		{
			//===== Write Num Pnts and Tris ====//
			fprintf( fp, "%d %d\n", numPnts, tri_cnt );

			//==== Write Pnts ====//
			for ( int i = 0 ; i < (int)allPntVec.size() ; i++ )
			{
				if ( pntShift[i] >= 0 )
					fprintf( fp, "%.16g %.16g %.16g\n", allPntVec[i]->x(), allPntVec[i]->z(), -allPntVec[i]->y() );
			}

			//==== Write Tris ====//
			for ( int i = 0 ; i < (int)m_SurfVec.size() ; i++ )
			{
				vector < SimpTri >& sTriVec = m_SurfVec[i]->GetMesh()->GetSimpTriVec();
				vector< vec3d >& sPntVec = m_SurfVec[i]->GetMesh()->GetSimpPntVec(); 
				for ( int t = 0 ; t <  (int)sTriVec.size() ; t++ )
				{
					int i0 = FindPntIndex( sPntVec[sTriVec[t].ind0], allPntVec, indMap );
					int i1 = FindPntIndex( sPntVec[sTriVec[t].ind1], allPntVec, indMap );
					int i2 = FindPntIndex( sPntVec[sTriVec[t].ind2], allPntVec, indMap );
					int ind1 = pntShift[i0] + 1;
					int ind2 = pntShift[i1] + 1;
					int ind3 = pntShift[i2] + 1;

					fprintf( fp, "%d %d %d %d.0\n", ind1, ind2, ind3, m_SurfVec[i]->GetCompID()+1 );
				}
			}			
			fclose( fp );
		}
	}

	if ( key_fn )
	{
		//==== Open file ====//
		FILE* fp = fopen(key_fn, "w");

		if ( fp )
		{
			fprintf( fp, "Color	Name			BCType\n");

			for ( int i = 0 ; i < (int)m_SurfVec.size() ; i++ )		
			{
				fprintf( fp, "%d.0  Section_%d  0\n", m_SurfVec[i]->GetCompID()+1, i );
			}

			fclose(fp);
		}
	}

	//=====================================================================================//
	//==== Write OBJ File =================================================================//
	//=====================================================================================//
	if ( obj_fn )
	{
		FILE* fp = fopen(obj_fn, "w");

		if ( fp )
		{
			//==== Write Pnts ====//
			for ( int i = 0 ; i < (int)allPntVec.size() ; i++ )
			{
				if ( pntShift[i] >= 0 )
				   fprintf( fp, "v %16.10f %16.10f %16.10f\n", allPntVec[i]->x(), allPntVec[i]->z(), -allPntVec[i]->y() );
			}

			fprintf( fp, "\n");

			//==== Write Tris ====//
			for ( int i = 0 ; i < (int)m_SurfVec.size() ; i++ )
			{
				vector < SimpTri >& sTriVec = m_SurfVec[i]->GetMesh()->GetSimpTriVec();
				vector< vec3d >& sPntVec = m_SurfVec[i]->GetMesh()->GetSimpPntVec(); 
				for ( int t = 0 ; t <  (int)sTriVec.size() ; t++ )
				{
					int i0 = FindPntIndex( sPntVec[sTriVec[t].ind0], allPntVec, indMap );
					int i1 = FindPntIndex( sPntVec[sTriVec[t].ind1], allPntVec, indMap );
					int i2 = FindPntIndex( sPntVec[sTriVec[t].ind2], allPntVec, indMap );
					int ind1 = pntShift[i0] + 1;
					int ind2 = pntShift[i1] + 1;
					int ind3 = pntShift[i2] + 1;
					fprintf( fp, "f %d %d %d \n", ind1, ind2, ind3 );
				}
			}
			fclose(fp);
		}
	}


	//=====================================================================================//
	//==== Write TRI File for Cart3D ======================================================//
	//=====================================================================================//
	if ( tri_fn )
	{
 		FILE* fp = fopen(tri_fn, "w");

		if ( fp )
		{
			//==== Write Pnt Count and Tri Count ====//
			fprintf( fp, "%d, %d\n", numPnts, tri_cnt);

			//==== Write Pnts ====//
			for ( int i = 0 ; i < (int)allPntVec.size() ; i++ )
			{
				if ( pntShift[i] >= 0 )
				   fprintf( fp, "%16.10g, %16.10g, %16.10g\n", allPntVec[i]->x(), allPntVec[i]->y(), allPntVec[i]->z() );
			}

			//==== Write Tris ====//
			for ( int i = 0 ; i < (int)m_SurfVec.size() ; i++ )
			{
				vector < SimpTri >& sTriVec = m_SurfVec[i]->GetMesh()->GetSimpTriVec();
				vector< vec3d >& sPntVec = m_SurfVec[i]->GetMesh()->GetSimpPntVec();
				for ( int t = 0 ; t <  (int)sTriVec.size() ; t++ )
				{
					int i0 = FindPntIndex( sPntVec[sTriVec[t].ind0], allPntVec, indMap );
					int i1 = FindPntIndex( sPntVec[sTriVec[t].ind1], allPntVec, indMap );
					int i2 = FindPntIndex( sPntVec[sTriVec[t].ind2], allPntVec, indMap );
					int ind1 = pntShift[i0] + 1;
					int ind2 = pntShift[i1] + 1;
					int ind3 = pntShift[i2] + 1;
					fprintf( fp, "%d, %d, %d \n", ind1, ind2, ind3 );
				}
			}

			//==== Write Component ID ====//
			for ( int i = 0 ; i < (int)m_SurfVec.size() ; i++ )
			{
				vector < SimpTri >& sTriVec = m_SurfVec[i]->GetMesh()->GetSimpTriVec();
				for ( int t = 0 ; t <  (int)sTriVec.size() ; t++ )
				{
					fprintf( fp, "%d \n", m_SurfVec[i]->GetCompID()+1);
				}
			}

			fclose(fp);
		}
	}
	//=====================================================================================//
	//==== Write TRI File for Cart3D ======================================================//
	//=====================================================================================//
	if ( gmsh_fn )
	{
		FILE* fp = fopen(gmsh_fn, "w");
		if ( fp )
		{
			fprintf(fp, "$MeshFormat\n" );
			fprintf(fp, "2.2 0 %d\n", sizeof(double) );
			fprintf(fp, "$EndMeshFormat\n" );

			//==== Write Nodes ====//
			fprintf(fp, "$Nodes\n" );
			fprintf(fp, "%d\n", numPnts );
			for ( int i = 0 ; i < (int)allPntVec.size() ; i++ )
			{
				if ( pntShift[i] >= 0 )
				{
					fprintf( fp, "%d %16.10f %16.10f %16.10f\n", pntShift[i]+1,
						allPntVec[i]->x(), allPntVec[i]->y(), allPntVec[i]->z() );
				}
			}
			fprintf(fp, "$EndNodes\n" );

			//==== Write Tris ====//
			fprintf(fp, "$Elements\n" );
			fprintf(fp, "%d\n", tri_cnt );

			int ele_cnt = 1;
			for ( int i = 0 ; i < (int)m_SurfVec.size() ; i++ )
			{
				vector < SimpTri >& sTriVec = m_SurfVec[i]->GetMesh()->GetSimpTriVec();
				vector< vec3d >& sPntVec = m_SurfVec[i]->GetMesh()->GetSimpPntVec();
				for ( int t = 0 ; t <  (int)sTriVec.size() ; t++ )
				{
					int i0 = FindPntIndex( sPntVec[sTriVec[t].ind0], allPntVec, indMap );
					int i1 = FindPntIndex( sPntVec[sTriVec[t].ind1], allPntVec, indMap );
					int i2 = FindPntIndex( sPntVec[sTriVec[t].ind2], allPntVec, indMap );
					int ind1 = pntShift[i0] + 1;
					int ind2 = pntShift[i1] + 1;
					int ind3 = pntShift[i2] + 1;
					fprintf( fp, "%d 2 0 %d %d %d \n", ele_cnt, ind1, ind2, ind3 );
					ele_cnt++;
				}
			}

			fprintf(fp, "$EndElements\n" );
			fclose(fp);
		}
	}

}


void CfdMeshMgr::WriteSurfsIntCurves( const char* filename )
{
	FILE* fp = fopen(filename, "w");
	if ( fp )
	{
		map< int, vector< int > > compMap;
		for ( int i = 0 ; i < (int)m_SurfVec.size() ; i ++ )
		{
			int surfId = m_SurfVec[i]->GetSurfID();
			int compId = m_SurfVec[i]->GetCompID();

			compMap[compId].push_back( surfId );
		}

		fprintf( fp, "BEGIN Component_List\n" );

		fprintf( fp, "%d		// Number of Components \n", (int)compMap.size() );

		map< int, vector< int > > :: iterator iter;

		for ( iter = compMap.begin() ; iter != compMap.end() ; iter++ )
		{
			int compId = iter->first;
			vector< int > idVec = iter->second;
			fprintf( fp, "BEGIN Component\n" );
			fprintf( fp, "%d		// Comp ID \n",		compId );
			fprintf( fp, "%d		// Num Surfs \n",	idVec.size() );
			for (int i = 0 ; i < (int)idVec.size() ; i++ )
			{
				fprintf( fp, "%d		// Surf ID \n",	idVec[i] );
			}
			fprintf( fp, "END Component\n" );
		}
		fprintf( fp, "END Component_List\n" );

		fprintf( fp, "BEGIN Surface_List\n" );
		fprintf( fp, "%d		// Number of Cubic Bezier Surfaces \n", (int)m_SurfVec.size() );
		for ( int s = 0 ; s < (int)m_SurfVec.size() ; s++ )
		{
			fprintf( fp, "BEGIN Surface\n" );
			Surf* surfPtr = m_SurfVec[s];
			fprintf( fp, "%d		// Surface ID \n",	surfPtr->GetSurfID() );
			fprintf( fp, "%d		// Comp ID \n",		surfPtr->GetCompID() );

			vector< vector< vec3d > > pntVec = surfPtr->GetControlPnts();
			int numU = pntVec.size();
			int numW = pntVec[0].size();

			fprintf( fp, "%d		// Num Pnts U \n",	numU );
			fprintf( fp, "%d		// Num Pnts W \n",	numW );
			fprintf( fp, "%d		// Total Pnts (0,0),(0,1),(0,2)..(0,numW-1),(1,0)..(1,numW-1)..(numU-1,0)..(numU-1,numW-1)\n",	numU*numW );


			for ( int i = 0 ; i < numU ; i++ )
			{
				for ( int j = 0 ; j < numW ; j++ )
				{
					fprintf( fp, "%20.20lf, %20.20lf, %20.20lf \n", 
						pntVec[i][j].x(), pntVec[i][j].y(), pntVec[i][j].z() );
				}
			}

			fprintf( fp, "END Surface\n" );
		}
		fprintf( fp, "END Surface_List\n" );


		vector< ISegChain* > border_curves;
		vector< ISegChain* > intersect_curves;
		list< ISegChain* >::iterator c;
		for ( c = m_ISegChainList.begin() ; c != m_ISegChainList.end(); c++ )
		{
			if ( (*c)->m_BorderFlag )
				border_curves.push_back( (*c) );
			else
				intersect_curves.push_back( (*c) );
		}

		fprintf( fp, "BEGIN Border_Curve_List\n" );

		fprintf( fp, "%d		// Number of Border_Curves \n", (int)border_curves.size() );

		for ( int i = 0 ; i < (int)border_curves.size() ; i++ )
		{
			fprintf( fp, "BEGIN Border_Curve\n" ); 

			Surf* surfA =  border_curves[i]->m_SurfA;
			Surf* surfB =  border_curves[i]->m_SurfB;

			fprintf( fp, "%d		// Surface A ID \n", surfA->GetSurfID() );
			fprintf( fp, "%d		// Surface B ID \n", surfB->GetSurfID() );

			vector< IPnt* > ipntVec;

			for ( int j = 0 ; j < (int)border_curves[i]->m_ISegDeque.size() ; j++ )
			{	
				ipntVec.push_back( border_curves[i]->m_ISegDeque[j]->m_IPnt[0] );
			}
			ipntVec.push_back( border_curves[i]->m_ISegDeque.back()->m_IPnt[1] );
			fprintf( fp, "%d		// Number of Border Points (Au, Aw, Bu, Bw) \n", (int)ipntVec.size() );

			for ( int j = 0 ; j < (int)ipntVec.size() ; j++ )
			{
				Puw* pwA = ipntVec[j]->GetPuw( surfA );
				Puw* pwB = ipntVec[j]->GetPuw( surfB );
				fprintf( fp, "%d	%16.16lf, %16.16lf, %16.16lf, %16.16lf \n", j, 
					pwA->m_UW.x(), pwA->m_UW.y(), pwB->m_UW.x(), pwB->m_UW.y() );

			}
			fprintf( fp, "END Border_Curve\n" );
		}
		fprintf( fp, "END Border_Curve_List\n" );


		fprintf( fp, "BEGIN IntersectCurve_List\n" );

		fprintf( fp, "%d		// Number of Intersect_Curves \n", (int)intersect_curves.size() );

		for ( int i = 0 ; i < (int)intersect_curves.size() ; i++ )
		{
			fprintf( fp, "BEGIN Intersect_Curve\n" ); 

			Surf* surfA =  intersect_curves[i]->m_SurfA;
			Surf* surfB =  intersect_curves[i]->m_SurfB;

			fprintf( fp, "%d		// Surface A ID \n", surfA->GetSurfID() );
			fprintf( fp, "%d		// Surface B ID \n", surfB->GetSurfID() );

			vector< IPnt* > ipntVec;

			for ( int j = 0 ; j < (int)intersect_curves[i]->m_ISegDeque.size() ; j++ )
			{	
				ipntVec.push_back( intersect_curves[i]->m_ISegDeque[j]->m_IPnt[0] );
			}
			ipntVec.push_back( intersect_curves[i]->m_ISegDeque.back()->m_IPnt[1] );
			fprintf( fp, "%d		// Number of Intersect Points (Au, Aw, Bu, Bw) \n", (int)ipntVec.size() );

			for ( int j = 0 ; j < (int)ipntVec.size() ; j++ )
			{
				Puw* pwA = ipntVec[j]->GetPuw( surfA );
				Puw* pwB = ipntVec[j]->GetPuw( surfB );
				fprintf( fp, "%d	%16.16lf, %16.16lf, %16.16lf, %16.16lf \n", j, 
					pwA->m_UW.x(), pwA->m_UW.y(), pwB->m_UW.x(), pwB->m_UW.y() );

			}
			fprintf( fp, "END Intersect_Curve\n" );
		}


		fprintf( fp, "END IntersectCurve_List\n" );


		fclose(fp);
	}

}

Stringc CfdMeshMgr::CheckWaterTight()
{
	vector< Node* > nodeVec;
	vector< Tri* > triVec;

	int tri_cnt = 0;
	vector< vec3d* > allPntVec;
	for ( int i = 0 ; i < (int)m_SurfVec.size() ; i++ )
	{
		vector< vec3d >& sPntVec = m_SurfVec[i]->GetMesh()->GetSimpPntVec(); 
		for ( int v = 0 ; v < (int)sPntVec.size() ; v++ )
		{
			allPntVec.push_back( &sPntVec[v] );
		}
		tri_cnt += m_SurfVec[i]->GetMesh()->GetSimpTriVec().size();
	}

	//==== Build Map ====//
	map< int, vector< int > > indMap;
	vector< int > pntShift;
	int numPnts = BuildIndMap( allPntVec, indMap, pntShift );

	//==== Create Nodes ====//
	for ( int i = 0 ; i < (int)allPntVec.size() ; i++ )
	{
		if ( pntShift[i] >= 0 )
		{
			Node* n = new Node();
			n->pnt = *allPntVec[i];
			nodeVec.push_back(n);
		}
	}

	//==== Create Edges and Tris ====//
	int moreThanTwoTriPerEdge = 0;
	map< int, vector<Edge*> > edgeMap;
	for ( int i = 0 ; i < (int)m_SurfVec.size() ; i++ )
	{
		vector < SimpTri >& sTriVec = m_SurfVec[i]->GetMesh()->GetSimpTriVec();
		vector< vec3d >& sPntVec = m_SurfVec[i]->GetMesh()->GetSimpPntVec(); 
		for ( int t = 0 ; t <  (int)sTriVec.size() ; t++ )
		{
			int i0 = FindPntIndex( sPntVec[sTriVec[t].ind0], allPntVec, indMap );
			int i1 = FindPntIndex( sPntVec[sTriVec[t].ind1], allPntVec, indMap );
			int i2 = FindPntIndex( sPntVec[sTriVec[t].ind2], allPntVec, indMap );
			int ind1 = pntShift[i0];
			int ind2 = pntShift[i1];
			int ind3 = pntShift[i2];

			Edge* e0 = FindAddEdge( edgeMap, nodeVec, ind1, ind2 );
			Edge* e1 = FindAddEdge( edgeMap, nodeVec, ind2, ind3 );
			Edge* e2 = FindAddEdge( edgeMap, nodeVec, ind3, ind1 );

			Tri* tri = new Tri( nodeVec[ind1], nodeVec[ind2], nodeVec[ind3], e0, e1, e2 );

			if ( !e0->SetTri( tri ) ) moreThanTwoTriPerEdge++;
			if ( !e1->SetTri( tri ) ) moreThanTwoTriPerEdge++;
			if ( !e2->SetTri( tri ) ) moreThanTwoTriPerEdge++;
			triVec.push_back( tri );

		}
	}

	//==== Find Border Edges ====//
	int num_border_edges = 0;
	map<int, vector<Edge*> >::const_iterator iter;
	for ( iter = edgeMap.begin() ; iter != edgeMap.end() ; iter++ )
	{
		for ( int i = 0 ; i < (int)iter->second.size() ; i++ )
		{
			Edge* e = iter->second[i];
			if ( !(e->t0 && e->t1) )
				num_border_edges++;
		}
	}

	char resultStr[255];
	if ( num_border_edges || moreThanTwoTriPerEdge )
	{
		sprintf( resultStr, "NOT Water Tight : %d Border Edges, %d Edges > 2 Tris\n", 
			num_border_edges, moreThanTwoTriPerEdge);
	}
	else
		sprintf( resultStr, "Is Water Tight\n");

	return Stringc( resultStr );

}

Edge* CfdMeshMgr::FindAddEdge( map< int, vector<Edge*> > & edgeMap, 
							  vector< Node* > & nodeVec, int ind1, int ind2 )
{
	Edge* e = NULL;
	map<int, vector<Edge*> >::const_iterator iter;
	int combind = ind1 + ind2;
	iter = edgeMap.find( combind );

	if ( iter != edgeMap.end() )	// Found Edge Vec so Check
	{
		vector<Edge*> eVec = edgeMap[combind];
		for ( int i = 0 ; i < (int)eVec.size() ; i++ )
		{
			if ( eVec[i]->ContainsNodes( nodeVec[ind1], nodeVec[ind2] ) )
				return eVec[i];
		}
	}

	//==== No Edge Found so Create
	e = new Edge( nodeVec[ind1], nodeVec[ind2] );
	edgeMap[combind].push_back( e );

	return e;
}



int CfdMeshMgr::BuildIndMap( vector< vec3d* > & allPntVec, map< int, vector< int > >& indMap, vector< int > & pntShift )
{
//double max_dist = 0.0;
	double tol = 1.0e-12;
	map<int, vector<int> >::const_iterator iter;
	for ( int i = 0 ; i < (int)allPntVec.size() ; i++ )
	{
		int combind = (int)((allPntVec[i]->x() + allPntVec[i]->y() + allPntVec[i]->z())*10000.0);
		iter = indMap.find( combind );
		if ( iter == indMap.end() )	// Add Index
		{
			indMap[combind].push_back( i );
		}
		else
		{
			bool addIndexFlag = true;
			for ( int j = 0 ; j < (int)iter->second.size() ; j++ )
			{
				int testind = iter->second[j];

				if ( fabs( allPntVec[i]->x() - allPntVec[testind]->x() ) < tol  &&
					 fabs( allPntVec[i]->y() - allPntVec[testind]->y() ) < tol  &&
					 fabs( allPntVec[i]->z() - allPntVec[testind]->z() ) < tol  )
				{
					addIndexFlag = false;
				}
			}
			if ( addIndexFlag )
			{
				indMap[combind].push_back( i );
			}
		}
	}

	//==== Figure Out Point Shifts ====//
	pntShift.resize( allPntVec.size() );
	for ( int i = 0 ; i < (int)pntShift.size() ; i++ )
	{
		pntShift[i] = -999;
	}

	for ( iter = indMap.begin() ; iter != indMap.end() ; iter++ )
	{
		for ( int i = 0 ; i < (int)iter->second.size() ; i++ )
		{
			int ind = iter->second[i];
			pntShift[ind] = 1;
		}
	}

	int cnt = 0;
	for ( int i = 0 ; i < (int)pntShift.size() ; i++ )
	{
		if ( pntShift[i] > 0 )
		{
			pntShift[i] = cnt;
			cnt++;
		}
	}

	return cnt;

}


int  CfdMeshMgr::FindPntIndex(  vec3d& pnt, vector< vec3d* > & allPntVec, map< int, vector< int > >& indMap )
{
	double tol = 1.0e-12;
	map<int, vector<int> >::const_iterator iter;

	int combind = (int)((pnt.x() + pnt.y() + pnt.z())*10000.0);
	iter = indMap.find( combind );
	if ( iter != indMap.end() )
	{
		for ( int j = 0 ; j < (int)iter->second.size() ; j++ )
		{
			int testind = iter->second[j];

			if ( fabs( pnt.x() - allPntVec[testind]->x() ) < tol  &&
				 fabs( pnt.y() - allPntVec[testind]->y() ) < tol  &&
				 fabs( pnt.z() - allPntVec[testind]->z() ) < tol  )
			{
				return testind;
			}
		}
	}

	printf( "Error: CfdMeshMgr.FindPntIndex can't find index\n");
	return 0;
}

void CfdMeshMgr::Intersect()
{
	//==== Quad Tree Intersection - Intersection Segments Get Loaded at AddIntersectionSeg ===//
	for ( int i = 0 ; i < (int)m_SurfVec.size() ; i++ )
		for ( int j = i+1 ; j < (int)m_SurfVec.size() ; j++ )
		{
			m_SurfVec[i]->Intersect( m_SurfVec[j] );

		}

	if ( m_HalfMeshFlag )
		IntersectYSlicePlane();

	BuildChains();

	LoadBorderCurves();

	MergeInteriorChainIPnts();

	SplitBorderCurves();

	IntersectSplitChains();


}

void CfdMeshMgr::IntersectYSlicePlane()
{
	//==== Get Bounding Box ====//
	bbox bigbox;
	for ( int i = 0 ; i < (int)m_SurfVec.size() ; i++ )
	{
		bigbox.update( m_SurfVec[i]->GetBBox() );
	}

	double offset = 1.0;
	double min_x = bigbox.get_min(0) - offset;
	double min_z = bigbox.get_min(2) - offset;
	double max_x = bigbox.get_max(0) + offset;
	double max_z = bigbox.get_max(2) + offset;

	vec3d p0 = vec3d(min_x, 0.0, min_z);
	vec3d p1 = vec3d(max_x, 0.0, min_z);
	vec3d p2 = vec3d(min_x, 0.0, max_z);
	vec3d p3 = vec3d(max_x, 0.0, max_z);

	//==== Build Slice Surface ====//
	m_YSlicePlane->SetCfdMeshMgr(this);
	m_YSlicePlane->SetCompID( -999 );
	m_YSlicePlane->SetSurfID( 0 );

	vector< vector< vec3d > > cpVec;
	cpVec.resize(4);
	for ( int i = 0 ; i < 4 ; i++ )
		cpVec[i].resize(4);

	cpVec[0][0] = p0;
	cpVec[1][0] = p0 + (p1-p0)*0.333;
	cpVec[2][0] = p0 + (p1-p0)*0.667;
	cpVec[3][0] = p1;
	cpVec[0][3] = p2;
	cpVec[1][3] = p2 + (p3-p2)*0.333;
	cpVec[2][3] = p2 + (p3-p2)*0.667;
	cpVec[3][3] = p3;
	cpVec[0][1] = cpVec[0][0] + (cpVec[0][3] - cpVec[0][0])*.333;
	cpVec[1][1] = cpVec[1][0] + (cpVec[1][3] - cpVec[1][0])*.333;
	cpVec[2][1] = cpVec[2][0] + (cpVec[2][3] - cpVec[2][0])*.333;
	cpVec[3][1] = cpVec[3][0] + (cpVec[3][3] - cpVec[3][0])*.333;
	cpVec[0][2] = cpVec[0][0] + (cpVec[0][3] - cpVec[0][0])*.667;
	cpVec[1][2] = cpVec[1][0] + (cpVec[1][3] - cpVec[1][0])*.667;
	cpVec[2][2] = cpVec[2][0] + (cpVec[2][3] - cpVec[2][0])*.667;
	cpVec[3][2] = cpVec[3][0] + (cpVec[3][3] - cpVec[3][0])*.667;

	m_YSlicePlane->LoadControlPnts( cpVec );

	for ( int i = 0 ; i < (int)m_SurfVec.size() ; i++ )
	{
		if ( !m_SurfVec[i]->OnYZeroPlane() )
			m_YSlicePlane->Intersect( m_SurfVec[i] );
	}
}


void CfdMeshMgr::InitMesh()
{
	bool PrintProgress = false;
	#ifdef DEBUG_CFD_MESH
		PrintProgress = true;
	#endif

	if ( PrintProgress )	printf("TessellateChains\n");
	TessellateChains( &m_GridDensity );

//DebugWriteChains( "Tess_UW", true );

	if ( PrintProgress )	printf("MergeBorderEndPoints\n");
	MergeBorderEndPoints();

	if ( PrintProgress )	printf("BuildMesh\n");
	BuildMesh();

	if ( PrintProgress )	printf("RemoveInteriorTris\n");
	RemoveInteriorTris();

	if ( PrintProgress )	printf("ConnectBorderEdges\n");
	ConnectBorderEdges();
}

void CfdMeshMgr::AddIntersectionSeg( SurfPatch& pA, SurfPatch& pB, vec3d & ip0, vec3d & ip1 )
{
	double d = dist_squared( ip0, ip1 );
	if ( d < DBL_EPSILON ) 
		return;

	vec2d proj_uwA0;
	pA.find_closest_uw( ip0, proj_uwA0.v );
	Puw* puwA0 = new Puw( pA.get_surf_ptr(), proj_uwA0 );
	m_DelPuwVec.push_back( puwA0 );

	vec2d proj_uwB0;
	pB.find_closest_uw( ip0, proj_uwB0.v );
	Puw* puwB0 = new Puw( pB.get_surf_ptr(), proj_uwB0 );
	m_DelPuwVec.push_back( puwB0 );

	IPnt* ipnt0 = new IPnt( puwA0, puwB0 );
	ipnt0->m_Pnt = ip0;
	m_DelIPntVec.push_back( ipnt0 );

	vec2d proj_uwA1;
	pA.find_closest_uw( ip1, proj_uwA1.v );
	Puw* puwA1 = new Puw( pA.get_surf_ptr(), proj_uwA1 );
	m_DelPuwVec.push_back( puwA1 );

	vec2d proj_uwB1;
	pB.find_closest_uw( ip1, proj_uwB1.v );
	Puw* puwB1 = new Puw( pB.get_surf_ptr(), proj_uwB1 );
	m_DelPuwVec.push_back( puwB1 );

	IPnt* ipnt1 = new IPnt( puwA1, puwB1 );
	ipnt1->m_Pnt = ip1;
	m_DelIPntVec.push_back( ipnt1 );

	ISeg* iseg01 = new ISeg( pA.get_surf_ptr(), pB.get_surf_ptr(), ipnt0, ipnt1 );

	int id0 = IPntBin::ComputeID( ipnt0->m_Pnt );
	m_BinMap[id0].m_ID = id0;
	m_BinMap[id0].m_IPnts.push_back( ipnt0 );

	int id1 = IPntBin::ComputeID( ipnt1->m_Pnt );
	m_BinMap[id1].m_ID = id1;
	m_BinMap[id1].m_IPnts.push_back( ipnt1 );

	#ifdef DEBUG_CFD_MESH

		static bool onetime = true;
		static int ipntcnt = 0;
		static double max_dist = 0.0;
		if ( onetime )
		{
			fprintf( m_DebugFile, "CfdMeshMgr::AddIntersectionSeg \n");
			onetime = false;
		}

		double dA0 = dist( ip0, puwA0->m_Surf->CompPnt( puwA0->m_UW.x(), puwA0->m_UW.y() ) );
		double dB0 = dist( ip0, puwB0->m_Surf->CompPnt( puwB0->m_UW.x(), puwB0->m_UW.y() ) );

		double dA1 = dist( ip1, puwA0->m_Surf->CompPnt( puwA1->m_UW.x(), puwA1->m_UW.y() ) );
		double dB1 = dist( ip1, puwB0->m_Surf->CompPnt( puwB1->m_UW.x(), puwB1->m_UW.y() ) );

		double tol = 1.0e-8;
		double total_d = dA0 + dB0 + dA1 + dB1;

		if ( total_d > max_dist )
		{
			max_dist = total_d;
			fprintf( m_DebugFile, "  Proj Pnt Dist = %f    %d \n", max_dist, ipntcnt );
		}
		ipntcnt++;

	#endif


//if ( pA.get_surf_ptr() == m_SurfVec[0]  )
//{
//	debugUWs.push_back( proj_uwA0 );
//	debugUWs.push_back( proj_uwA1 );
//}


}

void CfdMeshMgr::BuildChains()
{
	//==== Load Adjoining Bins =====//
	map< int, IPntBin >::const_iterator iter;
	for ( iter = m_BinMap.begin() ; iter != m_BinMap.end() ; iter++ )
	{
		int id = (*iter).second.m_ID;
		map< int, IPntBin >::const_iterator adj;
		for ( int i = -3 ; i < 4 ; i++ )		// Check All Nearby Bins
		{
			if ( i != 0 &&  m_BinMap.find( id + i ) != m_BinMap.end()  )
			{
				m_BinMap[id].m_AdjBins.push_back( id + i );
			}
		}
	}

	//==== Create Chains ====//
	for ( iter = m_BinMap.begin() ; iter != m_BinMap.end() ; iter++ )
	{
		int id = (*iter).second.m_ID;
		for ( int i = 0 ; i < (int)m_BinMap[id].m_IPnts.size() ; i++ )
		{
			if ( !m_BinMap[id].m_IPnts[i]->m_UsedFlag )
			{
				ISeg* seg = m_BinMap[id].m_IPnts[i]->m_Segs[0];
				seg->m_IPnt[0]->m_UsedFlag = true;
				seg->m_IPnt[1]->m_UsedFlag = true;
				ISegChain* chain = new ISegChain;			// Create New Chain
				chain->m_SurfA = seg->m_SurfA;
				chain->m_SurfB = seg->m_SurfB;
				chain->m_ISegDeque.push_back( seg );
				ExpandChain( chain );
				if ( chain->Valid() )
					m_ISegChainList.push_back( chain );
			}
		}
	}

#ifdef DEBUG_CFD_MESH

	int num_bins = 0;
	int total_num_segs = 0;
	for ( iter = m_BinMap.begin() ; iter != m_BinMap.end() ; iter++ )
	{
		num_bins++;
		int id = (*iter).second.m_ID;
		total_num_segs += m_BinMap[id].m_IPnts.size();
	}

	double avg_num_segs = (double)total_num_segs/(double)num_bins;

	fprintf( m_DebugFile, "CfdMeshMgr::BuildChains \n" );
	fprintf( m_DebugFile, "   Num Bins = %d \n", num_bins );
	fprintf( m_DebugFile, "   Avg Num Segs per Bin = %f\n", avg_num_segs );

	fprintf( m_DebugFile, "   Num Chains %d \n", m_ISegChainList.size() );
#endif

}

void CfdMeshMgr::ExpandChain( ISegChain* chain )
{
	bool stillExpanding = true;
	bool expandFront = true;
	while ( stillExpanding )
	{
		IPnt* testIPnt;
		if ( expandFront )
			testIPnt = chain->m_ISegDeque.front()->m_IPnt[0];
		else
			testIPnt = chain->m_ISegDeque.back()->m_IPnt[1];

		int binID = IPntBin::ComputeID( testIPnt->m_Pnt );

 		IPnt* matchIPnt = m_BinMap[binID].Match( testIPnt, m_BinMap );

		if ( !matchIPnt && !expandFront )	// No more matches in back of chain
			stillExpanding = false;
		else if ( !matchIPnt )					// No more matches in front, try back
			expandFront = false;
		else
		{
			ISeg* seg = matchIPnt->m_Segs[0];
			chain->AddSeg( seg, expandFront );
			seg->m_IPnt[0]->m_UsedFlag = true;
			seg->m_IPnt[1]->m_UsedFlag = true;
		}
	}
}

void CfdMeshMgr::WriteChains()
{
	FILE* fp;
	char str[256];
	sprintf( str, "RawInt_UW0.dat" );
	fp = fopen( str, "w" );
	
	for ( int i = 1 ; i < (int)debugUWs.size() ; i+=2 )
	{
		fprintf( fp, "MOVE \n" );
		fprintf( fp, "%f %f\n", debugUWs[i-1].x(), debugUWs[i-1].y() );
		fprintf( fp, "%f %f\n", debugUWs[i].x(), debugUWs[i].y() );
	}

	fclose(fp);


	//==== Write Chains ====//
	list< ISegChain* >::iterator c;
	for ( int i = 0 ; i < (int)m_SurfVec.size() ; i++ )
	{
		sprintf( str, "Intersection_UW%d.dat", i );
		fp = fopen( str, "w" );
		int c_ind = 0;
		for ( c = m_ISegChainList.begin() ; c != m_ISegChainList.end(); c++ )
		{
			if ( (*c)->m_SurfA == m_SurfVec[i] || (*c)->m_SurfB == m_SurfVec[i] )
			{
				int ncol = 10;
				if ( c_ind%ncol == 0 )
					fprintf( fp, "color WHITE\n");
				else if ( c_ind%ncol == 1 )
					fprintf( fp, "color RED\n");
				else if ( c_ind%ncol == 2 )
					fprintf( fp, "color PURPLE\n");
				else if ( c_ind%ncol == 3 )
					fprintf( fp, "color DARK_BLUE\n");
				else if ( c_ind%ncol == 4 )
					fprintf( fp, "color DARK_ORANGE\n");
				else if ( c_ind%ncol == 5 )
					fprintf( fp, "color AQUA\n");
				else if ( c_ind%ncol == 6 )
					fprintf( fp, "color GREEN\n");
				else if ( c_ind%ncol == 7 )
					fprintf( fp, "color DARK_RED\n");
				else if ( c_ind%ncol == 8 )
					fprintf( fp, "color DARK_PURPLE\n");
				else if ( c_ind%ncol == 9 )
					fprintf( fp, "color LIGHT_BLUE\n");
				c_ind++;
//BLACK,WHITE, RED, BLUE, GREEN, PURPLE, YELLOW, GREY or GRAY, 
//AQUA,DARK_BLUE,DARK_RED, DARK_PURPLE, DARK_ORANGE, DARK_GREY or 
//DARK_GRAY and LIGHT_BLUE				
				for ( int s = 0 ; s < (int)(*c)->m_ISegDeque.size() ; s++ )
				{
					IPnt* ip0 = (*c)->m_ISegDeque[s]->m_IPnt[0];
					IPnt* ip1 = (*c)->m_ISegDeque[s]->m_IPnt[1];

					int puwind = -1;
					for ( int p = 0 ; p < (int)ip0->m_Puws.size() ; p++ )
					{
						if ( ip0->m_Puws[p]->m_Surf == m_SurfVec[i] )
							puwind = p;
					}

					if ( puwind >= 0 )
					{
						Puw* puw0 = ip0->m_Puws[puwind];
						Puw* puw1 = ip1->m_Puws[puwind];

						fprintf( fp, "MOVE \n" );
						fprintf( fp, "%f %f\n", puw0->m_UW.x(), puw0->m_UW.y() );
						fprintf( fp, "%f %f\n", puw1->m_UW.x(), puw1->m_UW.y() );
					}
				}
				
				//==== Draw Intersections ====//
				for ( int s = 0 ; s < (int)(*c)->m_SplitVec.size() ; s++ )
				{
					ISegSplit* split = (*c)->m_SplitVec[s];
					if ( !(*c)->m_BorderFlag && split->m_Surf == m_SurfVec[i] )
					{
						fprintf( fp, "color YELLOW\n");
						fprintf( fp, "MOVE \n" );
						fprintf( fp, "%f %f\n", split->m_UW.x(), split->m_UW.y() );
						fprintf( fp, "%f %f\n", split->m_UW.x()+0.1, split->m_UW.y()+0.1 );
					}

				}
			}
		}
		fclose(fp);
	}
}

void CfdMeshMgr::LoadBorderCurves()
{

	//==== Tesselate Border Curves ====//
	for ( int i = 0 ; i < (int)m_ICurveVec.size() ; i++ )
	{
		if ( !m_ICurveVec[i]->m_SCurve_B )								// Non Closed Solid
			m_ICurveVec[i]->m_SCurve_B = m_ICurveVec[i]->m_SCurve_A;

		m_ICurveVec[i]->Tesselate( &m_GridDensity );

		//==== Create New Chain ====//
		ISegChain* chain = new ISegChain;
		m_ISegChainList.push_front( chain );
		chain->m_BorderFlag = true;

		Surf* surfA = m_ICurveVec[i]->m_SCurve_A->GetSurf();
		Surf* surfB = m_ICurveVec[i]->m_SCurve_B->GetSurf();

		chain->m_SurfA = surfA;
		chain->m_SurfB = surfB;

		vector< vec3d > uwA = m_ICurveVec[i]->m_SCurve_A->GetUWTessPnts();
		vector< vec3d > uwB = m_ICurveVec[i]->m_SCurve_B->GetUWTessPnts();

		for ( int j = 1 ; j < (int)uwA.size() ; j++ )
		{
			Puw* puwA0 = new Puw( surfA, vec2d( uwA[j-1][0], uwA[j-1][1] ) );
			Puw* puwA1 = new Puw( surfA, vec2d( uwA[j][0],   uwA[j][1] ) );
			Puw* puwB0 = new Puw( surfB, vec2d( uwB[j-1][0], uwB[j-1][1] ) );
			Puw* puwB1 = new Puw( surfB, vec2d( uwB[j][0],   uwB[j][1] ) );

			m_DelPuwVec.push_back( puwA0 );			// Save to delete later
			m_DelPuwVec.push_back( puwA1 );
			m_DelPuwVec.push_back( puwB0 );
			m_DelPuwVec.push_back( puwB1 );

			IPnt* p0 = new IPnt( puwA0, puwB0 );
			IPnt* p1 = new IPnt( puwA1, puwB1 );

			m_DelIPntVec.push_back( p0 );			// Save to delete later
			m_DelIPntVec.push_back( p1 );

			p0->CompPnt();
			p1->CompPnt();

			ISeg* seg = new ISeg( surfA, surfB, p0, p1 );

			chain->m_ISegDeque.push_back( seg );
		}
	}

#ifdef DEBUG_CFD_MESH
	fprintf( m_DebugFile, "CfdMeshMgr::LoadBorderCurves \n" );
	fprintf( m_DebugFile, "   Total Num Chains = %d \n", m_ISegChainList.size() );

	list< ISegChain* >::iterator c;
	for ( c = m_ISegChainList.begin() ; c != m_ISegChainList.end(); c++ )
	{
		fprintf( m_DebugFile, "   Chain Num Segs = %d Border %d \n", 
			(*c)->m_ISegDeque.size(), (int)((*c)->m_BorderFlag) );
	}
#endif

}


void CfdMeshMgr::SplitBorderCurves()
{
	vector< IPnt* > splitPnts;
	list< ISegChain* >::iterator c;
	for ( c = m_ISegChainList.begin() ; c != m_ISegChainList.end(); c++ )
	{
		if  ( !(*c)->m_BorderFlag )	// Non Border Chains
		{
			splitPnts.push_back( (*c)->m_ISegDeque.front()->m_IPnt[0] );
			splitPnts.push_back( (*c)->m_ISegDeque.back()->m_IPnt[1]  );
		}
	}

	for ( c = m_ISegChainList.begin() ; c != m_ISegChainList.end(); c++ )
	{
		if ( (*c)->m_BorderFlag )
		{
			for ( int i = 0 ; i < (int)splitPnts.size() ; i++ )
			{
				Puw* uw = splitPnts[i]->GetPuw( (*c)->m_SurfA );
				if ( uw )
				{
					(*c)->AddBorderSplit( splitPnts[i], uw );
				}
			}
		}
	}

	//==== Load Only Border Chains ====//
	vector< ISegChain* > chains;
	for ( c = m_ISegChainList.begin() ; c != m_ISegChainList.end(); c++ )
	{
		if ( (*c)->m_BorderFlag )
			chains.push_back( (*c) );
	}

	//==== Split Chains ====//
	for ( int i = 0 ; i < (int)chains.size() ; i++ )
	{
		chains[i]->MergeSplits();
		vector< ISegChain* > new_chains = chains[i]->SortAndSplit();
		for ( int j = 0 ; j < (int)new_chains.size() ; j++ )
		{
			new_chains[j]->m_BorderFlag = true;
			m_ISegChainList.push_back( new_chains[j] );
		}
	}


#ifdef DEBUG_CFD_MESH
	m_DebugDraw = true;
	for ( c = m_ISegChainList.begin() ; c != m_ISegChainList.end(); c++ )
	{
		vector< vec3d > crv;
		for ( int i = 0 ; i < (int)(*c)->m_ISegDeque.size() ; i++ )
		{
			IPnt* ip = (*c)->m_ISegDeque[i]->m_IPnt[0];
			crv.push_back( ip->m_Pnt );

			if ( i == (int)(*c)->m_ISegDeque.size() - 1 )
			{
				IPnt* ip = (*c)->m_ISegDeque[i]->m_IPnt[1];
				crv.push_back( ip->m_Pnt );
			}
		}
		m_DebugCurves.push_back( crv );
		int crv_cnt = (int)m_DebugCurves.size();
		int r = (crv_cnt%3)*85;
		int g = ((crv_cnt+2)%5)*55;
		int b = ((crv_cnt+4)%4)*63;
		m_DebugColors.push_back( vec3d( r,g, b ) );
	}
#endif

}


void CfdMeshMgr::IntersectSplitChains()
{
	//==== Intersect Intersection Curves (Not Border Curves) ====//
	list< ISegChain* >::iterator c;
	vector< ISegChain* > chains;
	for ( c = m_ISegChainList.begin() ; c != m_ISegChainList.end(); c++ )
	{
		if ( !(*c)->m_BorderFlag )
			chains.push_back( (*c) );
	}

	//==== Build Bounding Boxes Around Intersection Curves ====//
	for ( int i = 0 ; i < (int)chains.size() ; i++ )
	{
		chains[i]->BuildBoxes();
	}

	//==== Do Intersection ====//
	for ( int i = 0 ; i < (int)chains.size() ; i++ )
	{
		for ( int j = i+1 ; j < (int)chains.size() ; j++ )
		{
			if ( chains[i]->m_SurfA == chains[j]->m_SurfA || chains[i]->m_SurfA == chains[j]->m_SurfB )
				chains[i]->Intersect( chains[i]->m_SurfA, chains[j] );
			else if ( chains[i]->m_SurfB == chains[j]->m_SurfA || chains[i]->m_SurfB == chains[j]->m_SurfB )
				chains[i]->Intersect( chains[i]->m_SurfB, chains[j] );
		}
	}

	//==== Merge Splits ====//
	for ( int i = 0 ; i < (int)chains.size() ; i++ )
	{
		chains[i]->MergeSplits();
	}

	//==== Remove Chain End Splits ====//
	for ( int i = 0 ; i < (int)chains.size() ; i++ )
	{
		chains[i]->RemoveChainEndSplits();
	}

	//==== Split Chains ====//
	for ( int i = 0 ; i < (int)chains.size() ; i++ )
	{
		vector< ISegChain* > new_chains = chains[i]->SortAndSplit();
		for ( int j = 0 ; j < (int)new_chains.size() ; j++ )
		{
			m_ISegChainList.push_back( new_chains[j] );
			
		}
	}
}
	
void CfdMeshMgr::MergeInteriorChainIPnts()
{
	//==== Merge Interior IPnts in Chains ====//
	list< ISegChain* >::iterator c;
	for ( c = m_ISegChainList.begin() ; c != m_ISegChainList.end(); c++ )
	{
		(*c)->MergeInteriorIPnts();
	}
}

void CfdMeshMgr::TessellateChains(GridDensity* grid_density)
{
	//==== Tessellate Chains ====//
	list< ISegChain* >::iterator c;
	for ( c = m_ISegChainList.begin() ; c != m_ISegChainList.end(); c++ )
	{
		(*c)->Tessellate(grid_density);
	}

	////==== Check for Zero Length Chains ====//
	//for ( c = m_ISegChainList.begin() ; c != m_ISegChainList.end(); c++ )
	//{
	//	//double d = dist( (*c)->m_TessVec.front()->m_Pnt, (*c)->m_TessVec.back()->m_Pnt );
	//	//if ( d < 0.001 )
	//	//{
	//	//	printf("Zero Length Chain\n");
	//	//}

	//	////==== Compute Total Distance Betwee Points ====//
	//	//double total_dist = 0;
	//	//for ( int i = 0 ; i < (*c)->m_TessVec.size() ; i++ )
	//	//{
	//	//	Puw* puwa = (*c)->m_TessVec[i]->GetPuw( (*c)->m_SurfA );
	//	//	Puw* puwb = (*c)->m_TessVec[i]->GetPuw( (*c)->m_SurfB );
	//	//	vec2d auw = puwa->m_UW;
	//	//	vec2d buw = puwb->m_UW;
	//	//	vec3d pa = (*c)->m_SurfA->CompPnt( auw[0], auw[1] );
	//	//	vec3d pb = (*c)->m_SurfB->CompPnt( buw[0], buw[1] );
	//	//	double d = dist( pa, pb );
	//	//	total_dist += d;
	//	//}
	//	//printf("Total Chain Delta = %f \n", total_dist );
	//}
}


void CfdMeshMgr::MergeBorderEndPoints()
{
	//==== Load Chain End Points into Groups - Border Points First ====//
	list< ISegChain* >::iterator c;
	list < IPntGroup* > iPntGroupList;
	for ( c = m_ISegChainList.begin() ; c != m_ISegChainList.end(); c++ )
	{
		if ( (*c)->m_BorderFlag )
		{
			iPntGroupList.push_back( new IPntGroup );
			m_DelIPntGroupVec.push_back( iPntGroupList.back() );
			iPntGroupList.back()->m_IPntVec.push_back( (*c)->m_TessVec.front() );	// Add Front Point
			iPntGroupList.push_back( new IPntGroup );				
			m_DelIPntGroupVec.push_back( iPntGroupList.back() );
			iPntGroupList.back()->m_IPntVec.push_back( (*c)->m_TessVec.back() );	// Add Back Point
			
		}	
	}
	//==== Add Rest of Chain Points ====//
	for ( c = m_ISegChainList.begin() ; c != m_ISegChainList.end(); c++ )
	{
		if ( !(*c)->m_BorderFlag )
		{
			iPntGroupList.push_back( new IPntGroup );				
			m_DelIPntGroupVec.push_back( iPntGroupList.back() );
			iPntGroupList.back()->m_IPntVec.push_back( (*c)->m_TessVec.front() );	// Add Front Point
			iPntGroupList.push_back( new IPntGroup );				
			m_DelIPntGroupVec.push_back( iPntGroupList.back() );
			iPntGroupList.back()->m_IPntVec.push_back( (*c)->m_TessVec.back() );	// Add Back Point
		}	
	}

	double tol_fract = 0.01;
	MergeIPntGroups( iPntGroupList, tol_fract );

	//==== Merge Ipnts In Groups ====//
	list< IPntGroup* >::iterator g;
	vector< IPnt* > merged_ipnts;
	for ( g = iPntGroupList.begin() ; g != iPntGroupList.end(); g++ )
	{
		IPnt* mip = new IPnt();
		m_DelIPntVec.push_back( mip );
		for ( int j = 0 ; j < (int)(*g)->m_IPntVec.size() ; j++ )
		{
			mip->AddPuws( (*g)->m_IPntVec[j] );
		}
		mip->CompPnt();
		merged_ipnts.push_back( mip );
	}

	//==== Replace IPnts in Chains ====//
	for ( c = m_ISegChainList.begin() ; c != m_ISegChainList.end(); c++ )
	{
		IPnt* ip = (*c)->m_TessVec.front();
		int cnt = 0;
		for ( g = iPntGroupList.begin() ; g != iPntGroupList.end(); g++ )
		{
			for ( int j = 0 ; j < (int)(*g)->m_IPntVec.size() ; j++ )
			{
				if ( ip == (*g)->m_IPntVec[j] )
				{
					(*c)->m_TessVec.front() = merged_ipnts[cnt];
					break;
				}
			}
			cnt++;
		}
		cnt = 0;
		ip = (*c)->m_TessVec.back();
		for ( g = iPntGroupList.begin() ; g != iPntGroupList.end(); g++ )
		{
			for ( int j = 0 ; j < (int)(*g)->m_IPntVec.size() ; j++ )
			{
				if ( ip == (*g)->m_IPntVec[j] )
				{
					(*c)->m_TessVec.back() = merged_ipnts[cnt];
					break;
				}
			}
			cnt++;
		}
	}
}

void CfdMeshMgr::MergeIPntGroups( list< IPntGroup* > & iPntGroupList, double tol_fract )
{
	list< IPntGroup* >::iterator g;

	//===== Merge Two Closest Groups While Under Tol ====//
	IPntGroup* nearG1;
	IPntGroup* nearG2;
	double nearDistFract;
	bool stopFlag = false;
	while( !stopFlag )
	{
		stopFlag = true;
		nearDistFract = 1.0e12;

		//==== Find Closest Two Groups ====//
		list< IPntGroup* >::iterator g;
		for ( g = iPntGroupList.begin() ; g != iPntGroupList.end(); g++ )
		{
			list< IPntGroup* >::iterator h;
			for ( h = iPntGroupList.begin() ; h != iPntGroupList.end(); h++ )
			{
				if ( (*g) != (*h) )
				{
					double df = (*g)->GroupDist( (*h) );		
					if ( df < nearDistFract )
					{
						nearDistFract = df;
						nearG1 = (*g);
						nearG2 = (*h);
					}
				}
			}
		}

		if ( nearDistFract < tol_fract )
		{
			nearG1->AddGroup( nearG2 );
//			delete nearG2;
			iPntGroupList.remove( nearG2 );
			stopFlag = false;
		}
	}
}

void CfdMeshMgr::BuildMesh()
{
	//==== Mesh Each Surface ====//
	for ( int s = 0 ; s < (int)m_SurfVec.size() ; s++ )
	{
		vector< ISegChain* > surf_chains;
		list< ISegChain* >::iterator c;
		for ( c = m_ISegChainList.begin() ; c != m_ISegChainList.end(); c++ )
		{
			if ( ((*c)->m_SurfA == m_SurfVec[s] || (*c)->m_SurfB == m_SurfVec[s]) )
			{
				surf_chains.push_back( (*c) );
			}
		}
		m_SurfVec[s]->InitMesh( surf_chains );
	}
}

void CfdMeshMgr::RemoveInteriorTris()
{
	debugRayIsect.clear();

	//==== Find Max Bound Box of All Components ====//
	int s;
	bbox big_box;
	for (  s = 0 ; s < (int)m_SurfVec.size() ; s++ )
	{
		big_box.update( m_SurfVec[s]->GetBBox() );
	}
	double x_dist = 1.0 + big_box.get_max(0) - big_box.get_min(0);
	double y_dist = 1.0 + big_box.get_max(1) - big_box.get_min(1);
	double z_dist = 1.0 + big_box.get_max(2) - big_box.get_min(2);

	//==== Count Number of Component Crossings for Each Component =====//
	list< Tri* >::iterator t;
	for ( s = 0 ; s < (int)m_SurfVec.size() ; s++ )
	{
		int tri_comp_id = m_SurfVec[s]->GetCompID();
		list <Tri*> triList = m_SurfVec[s]->GetMesh()->GetTriList();
		for ( t = triList.begin() ; t != triList.end(); t++ )
		{
			vector< vector< double > > t_vec_vec;
			t_vec_vec.resize( m_NumComps );
			vec3d cp = (*t)->ComputeCenterPnt( m_SurfVec[s] );
			vec3d ep = cp + vec3d( x_dist, 0.0001, 0.0001 );

			for ( int i = 0 ; i < (int)m_SurfVec.size() ; i++ )
			{
				int comp_id = m_SurfVec[i]->GetCompID();
				if ( comp_id != tri_comp_id )
				{
					m_SurfVec[i]->IntersectLineSeg( cp, ep, t_vec_vec[comp_id] );
				}
			}
			bool interiorFlag = false;
			for ( int c = 0 ; c < m_NumComps ; c++ )
			{
				if ( (int)t_vec_vec[c].size()%2 == 1 )
				{
					interiorFlag = true;
				}
			}
			(*t)->interiorFlag = interiorFlag;
			//==== Load Adjoining Tris - NOT Crossing Borders ====//
			set< Tri* > triSet;
			(*t)->LoadAdjTris( 3, triSet );

			set<Tri*>::iterator st;
			for ( st = triSet.begin() ; st != triSet.end() ; st++ )
			{
				if ( interiorFlag ) (*st)->intExtCount++;
				else				(*st)->intExtCount--;

			}
		}
	}

	//==== Check Vote and Mark Interior Tris =====//
	for ( s = 0 ; s < (int)m_SurfVec.size() ; s++ )
	{
		list <Tri*> triList = m_SurfVec[s]->GetMesh()->GetTriList();
		for ( t = triList.begin() ; t != triList.end(); t++ )
		{
			if ( (*t)->intExtCount > 0 )
				(*t)->interiorFlag = true;
			else if ( (*t)->intExtCount < 0 )
				(*t)->interiorFlag = false;
			else
				printf( "IntExtCount ZERO!\n");
		}
	}

	//==== Check For Half Mesh ====//
	if ( m_HalfMeshFlag )
	{
		for ( s = 0 ; s < (int)m_SurfVec.size() ; s++ )
		{
			list <Tri*> triList = m_SurfVec[s]->GetMesh()->GetTriList();
			for ( t = triList.begin() ; t != triList.end(); t++ )
			{
				vec3d cp = (*t)->ComputeCenterPnt( m_SurfVec[s] );
				if ( cp[1] < 0.0000000001 )
					(*t)->interiorFlag = true;
			}
		}
	}

	//==== Remove Tris, Edges and Nodes ====//
	for ( s = 0 ; s < (int)m_SurfVec.size() ; s++ )
	{
		m_SurfVec[s]->GetMesh()->RemoveInteriorTrisEdgesNodes();
	}
}

void CfdMeshMgr::ConnectBorderEdges()
{
	list< Edge* >::iterator e;
	list< Edge* > edgeList;
	list< Tri* >::iterator t;
	for ( int s = 0 ; s < (int)m_SurfVec.size() ; s++ )
	{
		list <Tri*> triList = m_SurfVec[s]->GetMesh()->GetTriList();
		for ( t = triList.begin() ; t != triList.end(); t++ )
		{
			if ( (*t)->e0->OtherTri( (*t) ) == NULL )
				edgeList.push_back( (*t)->e0 );
			if ( (*t)->e1->OtherTri( (*t) ) == NULL )
				edgeList.push_back( (*t)->e1 );
			if ( (*t)->e2->OtherTri( (*t) ) == NULL )
				edgeList.push_back( (*t)->e2 );
		}
	}

	int i, j, k;
//	int num_grid = 10; 
	int num_grid = 1;  // jrg change back to 10????

	vector< vector< vector< list< Edge* > > > > edgeGrid;
	edgeGrid.resize( num_grid );
	for (  i = 0 ; i < num_grid ; i++ )
		edgeGrid[i].resize( num_grid );
	for (  i = 0 ; i < num_grid ; i++ )
		for (  j = 0 ; j < num_grid ; j++ )
			edgeGrid[i][j].resize( num_grid );

	bbox box;
	for ( e = edgeList.begin() ; e != edgeList.end() ; e++ )
	{
		box.update( (*e)->n0->pnt );
		box.update( (*e)->n1->pnt );
	}

	double slop = 0.00001;
	double min_x = box.get_min(0) - slop;
	double min_y = box.get_min(1) - slop;
	double min_z = box.get_min(2) - slop;
	double dx = 2*slop+(box.get_max(0) - box.get_min(0))/(double)(num_grid);
	double dy = 2*slop+(box.get_max(1) - box.get_min(1))/(double)(num_grid);
	double dz = 2*slop+(box.get_max(2) - box.get_min(2))/(double)(num_grid);

	for ( e = edgeList.begin() ; e != edgeList.end() ; e++ )
	{
		double mx = min( (*e)->n0->pnt[0], (*e)->n1->pnt[0] );
		double my = min( (*e)->n0->pnt[1], (*e)->n1->pnt[1] );
		double mz = min( (*e)->n0->pnt[2], (*e)->n1->pnt[2] );

		int ix = (int)((mx - min_x)/dx);
		int iy = (int)((my - min_y)/dy);
		int iz = (int)((mz - min_z)/dz);
		edgeGrid[ix][iy][iz].push_back( (*e) );
	}
int es = edgeList.size();
int eg = edgeGrid[0][0][0].size();

	for ( i = 0 ; i < num_grid ; i++ )
		for ( j = 0 ; j < num_grid ; j++ )
			for ( k = 0 ; k < num_grid ; k++ )
			{
				if ( edgeGrid[i][j][k].size() > 0 )
					MatchBorderEdges( edgeGrid[i][j][k] );
			}

}

void CfdMeshMgr::MatchBorderEdges( list< Edge* > edgeList )
{
	list< Edge* >::iterator e;
	list< Edge* >::iterator f;

	//==== Match Edges ====//
	double dist_tol = 0.01*0.01;
	bool stopFlag = false;
	if ( edgeList.size() <= 1  )
		stopFlag = true;

	while( !stopFlag )
	{
		double close_dist = 1.0e12;
		Edge* close_e = NULL;
		Edge* close_f = NULL;
		for ( e = edgeList.begin() ; e != edgeList.end() ; e++ )
		{
			for ( f = edgeList.begin() ; f != edgeList.end() ; f++ )
			{
				if ( (*e) != (*f) )
				{
					double d0011 = dist_squared( (*e)->n0->pnt, (*f)->n0->pnt ) + 
								   dist_squared( (*e)->n1->pnt, (*f)->n1->pnt );
					double d0110 = dist_squared( (*e)->n0->pnt, (*f)->n1->pnt ) + 
								   dist_squared( (*e)->n1->pnt, (*f)->n0->pnt );

					double d = min( d0011, d0110 );
					if ( d < close_dist )
					{
						close_dist = d;
						close_e = (*e);
						close_f = (*f);
					}
				}
			}
			if ( close_dist < dist_tol )
			{
				break;
			}
		}
		if ( close_dist < dist_tol )
		{
//printf("Match Edge %f %d \n",close_dist, edgeList.size()  );
			//==== Merge the 2 Edges ====//
			double d0011 = dist_squared( close_e->n0->pnt, close_f->n0->pnt ) + 
						   dist_squared( close_e->n1->pnt, close_f->n1->pnt );

			double d0110 = dist_squared( close_e->n0->pnt, close_f->n1->pnt ) + 
						   dist_squared( close_e->n1->pnt, close_f->n0->pnt );

			if ( d0011 < d0110 )
			{
				//close_e->n0->pnt = (close_e->n0->pnt + close_f->n0->pnt)*0.5;
				//close_e->n1->pnt = (close_e->n1->pnt + close_f->n1->pnt)*0.5;
				close_f->n0->pnt = close_e->n0->pnt;
				close_f->n1->pnt = close_e->n1->pnt;
			}
			else
			{
				//close_e->n0->pnt = (close_e->n0->pnt + close_f->n1->pnt)*0.5;
				//close_e->n1->pnt = (close_e->n1->pnt + close_f->n0->pnt)*0.5;
				close_f->n1->pnt = close_e->n0->pnt;
				close_f->n0->pnt = close_e->n1->pnt;
			}
			edgeList.remove( close_e );
			edgeList.remove( close_f );
		}
		else
		{
			close_e->debugFlag = true;
//			printf("Close Dist = %f\n", close_dist );
			edgeList.remove( close_e );
			edgeList.remove( close_f );

		}

		if ( edgeList.size() <= 1  )
			stopFlag = true;
	}

#ifdef DEBUG_CFD_MESH
	if ( edgeList.size() != 0 )
	{
		for ( e = edgeList.begin() ; e != edgeList.end() ; e++ )
			(*e)->debugFlag = true;

		fprintf(m_DebugFile, "CfdMeshMgr::MatchBorderEdges Missing Edges %d\n", edgeList.size() );
		for ( e = edgeList.begin() ; e != edgeList.end() ; e++ )
		{
			fprintf(m_DebugFile, "      Missing Edge : %f %f %f   %f %f %f\n", 
				(*e)->n0->pnt.x(), (*e)->n0->pnt.y(), (*e)->n0->pnt.z(),
				(*e)->n1->pnt.x(), (*e)->n1->pnt.y(), (*e)->n1->pnt.z() );
		}	
	}
#endif
}

void CfdMeshMgr::HighlightNextChain()
{ 
		m_HighlightChainIndex++; 
		if ( m_HighlightChainIndex >= (int)m_ISegChainList.size() ) 
			m_HighlightChainIndex = 0; 
}

void CfdMeshMgr::DebugWriteChains( const char* name, bool tessFlag )
{
	
	for ( int i = 0 ; i < (int)m_SurfVec.size() ; i++ )
	{
		char str[256];
		sprintf( str, "%s%d.dat", name, i );
		FILE* fp = fopen( str, "w" );

		int cnt = 0;
		list< ISegChain* >::iterator c;
		for ( c = m_ISegChainList.begin() ; c != m_ISegChainList.end(); c++ )
		{
			if ( m_SurfVec[i] == (*c)->m_SurfA || m_SurfVec[i] == (*c)->m_SurfB )
			{
				if ( cnt%9 == 0 ) fprintf( fp, "COLOR RED\n" );
				else if ( cnt % 9 == 1 ) fprintf( fp, "COLOR BLUE\n" );
				else if ( cnt % 9 == 2 ) fprintf( fp, "COLOR GREEN\n" );
				else if ( cnt % 9 == 3 ) fprintf( fp, "COLOR PURPLE\n" );
				else if ( cnt % 9 == 4 ) fprintf( fp, "COLOR YELLOW\n" );
				else if ( cnt % 9 == 5 ) fprintf( fp, "COLOR DARK_ORANGE\n" );
				else if ( cnt % 9 == 6 ) fprintf( fp, "COLOR GREY\n" );
				else if ( cnt % 9 == 7 ) fprintf( fp, "COLOR DARK_PURPLE\n" );
				else if ( cnt % 9 == 8 ) fprintf( fp, "COLOR AQUA\n" );
				

				if ( tessFlag == FALSE )
				{
					for ( int j = 0 ; j < (int)(*c)->m_ISegDeque.size() ; j++ )
					{
						fprintf( fp, "MOVE \n" );
						vec2d uw0 = (*c)->m_ISegDeque[j]->m_IPnt[0]->GetPuw( m_SurfVec[i] )->m_UW;
						vec2d uw1 = (*c)->m_ISegDeque[j]->m_IPnt[1]->GetPuw( m_SurfVec[i] )->m_UW;
		vec2d tmp = uw0 + (uw1 - uw0)*0.1;
		uw1 = uw1 + (uw0 - uw1)*0.1;
		uw0 = tmp;
						fprintf( fp, "%f %f\n", uw0[0], uw0[1] );
						fprintf( fp, "%f %f\n", uw1[0], uw1[1] );
					}
				}
				else
				{
					fprintf( fp, "MOVE \n" );
						for ( int j = 1 ; j < (int)(*c)->m_TessVec.size() ; j++ )
						{
							vec2d uw0 = (*c)->m_TessVec[j-1]->GetPuw( m_SurfVec[i] )->m_UW;
							vec2d uw1 = (*c)->m_TessVec[j]->GetPuw( m_SurfVec[i] )->m_UW;
		vec2d tmp = uw0 + (uw1 - uw0)*0.1;
		uw1 = uw1 + (uw0 - uw1)*0.1;
		uw0 = tmp;
							fprintf( fp, "%f %f\n", uw0[0], uw0[1] );
							fprintf( fp, "%f %f\n", uw1[0], uw1[1] );
						}
				}
				cnt++;
			}
		}
		fclose(fp);
	}
}

void CfdMeshMgr::TestStuff()
{
	if ( !m_SurfVec.size() )
		return;

	Surf* sPtr = m_SurfVec[0];

	vector< SurfPatch* > pVec = sPtr->GetPatchVec();

	if ( !pVec.size() )
		return;

	SurfPatch sp0;
	SurfPatch sp1;
	SurfPatch sp2;
	SurfPatch sp3;

	pVec[0]->split_patch( sp0, sp1, sp2, sp3 );

	vec3d psurf = pVec[0]->comp_pnt_01( 0.3, 0.3 );
	vec3d ppatch = sp0.comp_pnt_01( 0.6, 0.6 );
	double d = dist( psurf, ppatch );

	psurf = pVec[0]->comp_pnt_01( 0.6, 0.6 );
	ppatch = sp3.comp_pnt_01( 0.2, 0.2 );
	d = dist( psurf, ppatch );

	psurf = pVec[0]->comp_pnt_01( 0.3, 0.6 );
	ppatch = sp2.comp_pnt_01( 0.6, 0.2 );
	d = dist( psurf, ppatch );

	psurf = pVec[0]->comp_pnt_01( 0.6, 0.3 );
	ppatch = sp1.comp_pnt_01( 0.2, 0.6 );
	d = dist( psurf, ppatch );

	sp3.split_patch( sp0, sp1, sp2, sp3 );
	psurf = pVec[0]->comp_pnt_01( 0.6, 0.6 );
	ppatch = sp0.comp_pnt_01( 0.4, 0.4 );
	d = dist( psurf, ppatch );





}

void CfdMeshMgr::Draw()
{
	bool isShown = aircraftPtr->getScreenMgr()->getCfdMeshScreen()->isShown();
	if ( !isShown )
		return;

	glLineWidth( 1.0 );
	glColor4ub( 255, 0, 0, 255 );

	BaseSource* source = GetCurrSource();

	if ( m_DrawSourceFlag )
		m_GridDensity.Draw(source);

	if ( m_DrawMeshFlag )
	{
	////////glLineWidth( 1.0 );
	////////glColor4ub( 255, 0, 0, 255 );
	////////glBegin( GL_LINES );
	////////for ( int i = 0 ; i < debugPnts.size() ; i+=2 )
	////////{
	////////	if ( i%4 == 0 )
	////////		glColor4ub( 255, 0, 0, 255 );
	////////	else
	////////		glColor4ub( 0, 0, 255, 255 );
	////////	glVertex3dv( debugPnts[i].data() );
	////////	glVertex3dv( debugPnts[i+1].data() );
	////////}
	////////glEnd();
	////////glColor4ub( 0, 0, 0, 255 );
	////////glPointSize(4.0);
	////////glBegin( GL_POINTS );
	////////for ( int i = 0 ; i < debugPnts.size() ; i++ )
	////////{
	////////	glVertex3dv( debugPnts[i].data() );
	////////}
	////////glEnd();


		//==== Draw Mesh ====//
		glPolygonOffset(2.0, 1);

		glCullFace( GL_BACK );						// Cull Back Faces For Trans
		glEnable( GL_CULL_FACE );

#ifndef __APPLE__
		glEnable(GL_POLYGON_OFFSET_EXT);
#endif
 
		glColor4ub( 220, 220, 220, 255 );
		for ( int i = 0 ; i < (int)m_SurfVec.size() ; i++ )
		{
			//list< Tri* >::iterator t;
			//list <Tri*> tlist = m_SurfVec[i]->GetMesh()->GetTriList();

			//for ( t = tlist.begin() ; t != tlist.end(); t++ )
			//{
			//	glBegin( GL_POLYGON );
			//		glVertex3dv( (*t)->n0->pnt.data() );
			//		glVertex3dv( (*t)->n1->pnt.data() );
			//		glVertex3dv( (*t)->n2->pnt.data() );
			//	glEnd();
			//}

			vector< vec3d > pVec = m_SurfVec[i]->GetMesh()->GetSimpPntVec();
			for ( int t = 0 ; t < (int)m_SurfVec[i]->GetMesh()->GetSimpTriVec().size() ; t++ )
			{
				SimpTri* stri = &m_SurfVec[i]->GetMesh()->GetSimpTriVec()[t];
				glBegin( GL_POLYGON );
					glVertex3dv( pVec[stri->ind0].data() );
					glVertex3dv( pVec[stri->ind1].data() );
					glVertex3dv( pVec[stri->ind2].data() );
				glEnd();
			}
		}

		glLineWidth(1.0);
		glColor4ub( 100, 0, 100, 255 );
		for ( int i = 0 ; i < (int)m_SurfVec.size() ; i++ )
		{
			//list< Tri* >::iterator t;
			//list <Tri*> tlist = m_SurfVec[i]->GetMesh()->GetTriList();

			//for ( t = tlist.begin() ; t != tlist.end(); t++ )
			//{
			//	glBegin( GL_LINE_LOOP );
			//		glVertex3dv( (*t)->n0->pnt.data() );
			//		glVertex3dv( (*t)->n1->pnt.data() );
			//		glVertex3dv( (*t)->n2->pnt.data() );
			//	glEnd();
			//}
			vector< vec3d > pVec = m_SurfVec[i]->GetMesh()->GetSimpPntVec();
			for ( int t = 0 ; t < (int)m_SurfVec[i]->GetMesh()->GetSimpTriVec().size() ; t++ )
			{
				SimpTri* stri = &m_SurfVec[i]->GetMesh()->GetSimpTriVec()[t];
				glBegin( GL_LINE_LOOP );
					glVertex3dv( pVec[stri->ind0].data() );
					glVertex3dv( pVec[stri->ind1].data() );
					glVertex3dv( pVec[stri->ind2].data() );
				glEnd();
			}
		}
		glDisable( GL_CULL_FACE );
#ifndef __APPLE__
		glDisable(GL_POLYGON_OFFSET_EXT);
#endif
	}


#ifdef DEBUG_CFD_MESH
	if ( m_DebugDraw )
	{
		for ( int i = 0 ; i < (int)m_DebugCurves.size() ; i++ )
		{
			glPointSize( 4.0 );
			glLineWidth( 2.0 );
			vec3d rgb = m_DebugColors[i];
			glColor4ub( (GLbyte)rgb[0], (GLbyte)rgb[1], (GLbyte)rgb[2], 255 );

			glBegin( GL_LINE_STRIP );
			for ( int j = 0 ; j < (int)m_DebugCurves[i].size() ; j++ )
			{
				glVertex3dv( m_DebugCurves[i][j].data() );
			}
			glEnd();
		
			glBegin( GL_POINTS );
			for ( int j = 0 ; j < (int)m_DebugCurves[i].size() ; j++ )
			{
				glVertex3dv( m_DebugCurves[i][j].data() );
			}
			glEnd();

		}
	}

#endif

	//glLineWidth( 1.0 );
	//glColor4ub( 150, 150, 150, 255 );
	//for ( int s = 0 ; s < (int)m_SurfVec.size() ; s++ )
	//{
	//	m_SurfVec[s]->Draw();
	//}

	//glPointSize( 4.0 );
	//glLineWidth( 1.0 );
	//glColor4ub( 255, 0, 0, 255 );
	//for ( int s = 0 ; s < (int)debugPatches.size() ; s++ )
	//{
	//	debugPatches[s]->Draw();
	//}


	//////for ( int r = 0 ; r < debugRayIsect.size() ; r++ )
	//////{
	//////	glColor4ub( 255, 0, 0, 255 );
	//////	glBegin( GL_LINE_STRIP );
	//////	for ( int i = 0 ; i < debugRayIsect[r].size() ; i++ )
	//////	{
	//////		glVertex3dv( debugRayIsect[r][i].data() );
	//////	}
	//////	glEnd();
	//////	
	//////	glColor4ub( 255, 0, 255, 255 );
	//////	glBegin( GL_POINTS );
	//////	for ( int i = 0 ; i < debugRayIsect[r].size() ; i++ )
	//////	{
	//////		glVertex3dv( debugRayIsect[r][i].data() );
	//////	}
	//////	glEnd();
	//////}

	//glLineWidth( 2.0 );
	//int cnt = 0;
	//list< ISegChain* >::iterator c;
	//for ( c = m_ISegChainList.begin() ; c != m_ISegChainList.end(); c++ )
	//{
	//	if ( cnt%4 == 0 )				glColor4ub( 0, 255, 255, 100 );
	//	else if ( cnt%4 == 1 )			glColor4ub( 255, 0, 0, 100 );
	//	else if ( cnt%4 == 2 )			glColor4ub( 0, 255, 0, 100 );
	//	else if ( cnt%4 == 3 )			glColor4ub( 0, 0, 255, 100 );
	//	if ( cnt == m_HighlightChainIndex )
	//	{
	//		glColor4ub( 255, 255, 255, 100 );
	//		//if ( (*c)->m_SurfA ) (*c)->m_SurfA->Draw();
	//		//if ( (*c)->m_SurfB ) (*c)->m_SurfB->Draw();

	//		glColor4ub( 0, 255, 255, 255 );
	//		glLineWidth( 2.0 );
	//		glPointSize( 6.0 );
	//	}
	//	else
	//	{
	//		glColor4ub( 255, 0, 0, 255 );
	//		glLineWidth( 1.0 );
	//		glPointSize( 3.0 );
	//	}


	//	(*c)->Draw();

	//	cnt++;

	////	//(*c)->m_ISegBoxA.Draw();
	////	//(*c)->m_ISegBoxB.Draw();
	//}

}

