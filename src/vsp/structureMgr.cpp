//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//

// materialMgr.cpp: implementation of the materialMgr class.
//
//////////////////////////////////////////////////////////////////////

#include "structureMgr.h"

#include "geom.h"
#include "aircraft.h"
#include "part.h"

//==== Singleton ====//
SM_Single::SM_Single()
{
	static StructureMgr* ptr = 0; 

    if (!ptr) 
		ptr = new StructureMgr();

	structureMgr = ptr;     
}

//=============================================================//
//=============================================================//

StructureMgr::StructureMgr()
{
	aircraftPtr = NULL;
	activeFlag = false;
	updateFlag = false;
	currGeomID = 0;
	finalTriArea = 0.05;
	exportFileName = "FEA_Export.nas";
}

StructureMgr::~StructureMgr()
{

}

void StructureMgr::SetActiveFlag( bool flag )
{
	if ( flag != activeFlag )
		ForceDraw();

	activeFlag = flag; 
}

void StructureMgr::TriggerUpdate()
{ 
	updateFlag = true; 
}

void StructureMgr::ForceDraw()
{
	if ( aircraftPtr ) 
		aircraftPtr->triggerDraw();
}

Part* StructureMgr::GetCurrPart()
{
	Part* p = NULL;

	vector< Geom* > geomVec = aircraftPtr->getGeomVec();	
	if ( currGeomID >= 0 && currGeomID < (int)geomVec.size() )
	{
		int partID = geomVec[currGeomID]->GetCurrPartID();
		vector< Part* > pVec = geomVec[currGeomID]->getStructurePartVec();

		if ( partID >= 0 && partID < (int)pVec.size() )
				p = pVec[partID];
	}

	return p;
}

void StructureMgr::DeleteCurrPart()
{
	vector< Geom* > geomVec = aircraftPtr->getGeomVec();	
	if ( currGeomID >= 0 && currGeomID < (int)geomVec.size() )
	{
		geomVec[currGeomID]->DelCurrPart();
		ForceDraw();
	}
}

vector< Part* > StructureMgr::GetAllParts()
{
	vector< Part* > allPartVec;
	vector< Geom* > geomVec = aircraftPtr->getGeomVec();
	for ( int g = 0 ; g < (int)geomVec.size() ; g++ )
	{
		vector< Part* > pVec = geomVec[g]->getStructurePartVec();
		for ( int p = 0 ; p < (int)pVec.size() ; p++ )
		{
			allPartVec.push_back( pVec[p] );
		}
	}
	return allPartVec;
}

void StructureMgr::GUI_Val( Stringc name, double val )
{
	Part* currPart = GetCurrPart();

	if ( name == "FinalTriArea"  )
		SetFinalTriArea( val );

	if ( !currPart )
		return;

	if ( name == "U1" && currPart->GetType() == Part::THREE_PNT_SLICE )
		((ThreePntSlice*)currPart)->SetUPnt( val, 0);
	else if ( name == "W1" && currPart->GetType() == Part::THREE_PNT_SLICE )
		((ThreePntSlice*)currPart)->SetWPnt( val, 0);
	else if ( name == "U2" && currPart->GetType() == Part::THREE_PNT_SLICE )
		((ThreePntSlice*)currPart)->SetUPnt( val, 1);
	else if ( name == "W2" && currPart->GetType() == Part::THREE_PNT_SLICE )
		((ThreePntSlice*)currPart)->SetWPnt( val, 1);
	else if ( name == "U3" && currPart->GetType() == Part::THREE_PNT_SLICE )
		((ThreePntSlice*)currPart)->SetUPnt( val, 2);
	else if ( name == "W3" && currPart->GetType() == Part::THREE_PNT_SLICE )
		((ThreePntSlice*)currPart)->SetWPnt( val, 2);

	else if ( name == "RibSpan" && currPart->GetType() == Part::RIB )
		((Rib*)currPart)->SetPercentSpan( val );
	else if ( name == "SparChord" && currPart->GetType() == Part::SPAR )
		((Spar*)currPart)->SetPercentChord( val );


	//==== Make Sure  ====//
	vector< Part* > pVec = GetAllParts();
	for ( int p = 0 ; p < (int)pVec.size() ; p++ )
	{
		pVec[p]->DeleteFinalMesh();
	}
	
		
	aircraftPtr->triggerDraw();

}
	
void StructureMgr::GUI_Val( Stringc name, int val )
{
	if ( name == "PartID" )				// Make this part active
	{
		vector< Geom* > geomVec = aircraftPtr->getGeomVec();	
		if ( currGeomID >= 0 && currGeomID < (int)geomVec.size() )
		{
			vector< Part* > pVec = geomVec[currGeomID]->getStructurePartVec();
//			if ( val >= 0 && val < (int)pVec.size() )
//			{
				geomVec[currGeomID]->SetCurrPartID(val);
//			}
		}
	}

	EditCurrPart(); 
	TriggerUpdate();
	aircraftPtr->triggerDraw();

}
	
void StructureMgr::GUI_Val( Stringc name, Stringc val )
{
	Part* currPart = GetCurrPart();


	if ( name == "PartName" && currPart  )
		currPart->SetName( val.get_char_star() );

	else if ( name == "ExportName" )
		exportFileName = val;

}

void StructureMgr::EditCurrPart()
{
	vector< Part* > pVec = GetAllParts();

	//==== Set All Edit Flags False ====//
	for ( int p = 0 ; p < (int)pVec.size() ; p++ )
	{
		pVec[p]->SetEditFlag(false);
	}

	Part* currPart = GetCurrPart();

	if ( currPart )
		currPart->SetEditFlag(true);

}
	
Part* StructureMgr::CreatePart( int part_type )
{
	Part* retPart = NULL;

	static int cnt = 0;
	char name_str[256];

	if ( part_type == Part::THREE_PNT_SLICE )
	{
		retPart = new ThreePntSlice();	
		sprintf( name_str, "Three_Pnt_Slice_%d", cnt );
	}
	else if ( part_type == Part::RIB )
	{
		retPart = new Rib();	
		sprintf( name_str, "Rib_%d", cnt );
	}
	else if ( part_type == Part::SPAR )
	{
		retPart = new Spar();	
		sprintf( name_str, "Spar_%d", cnt );
	}

	if ( retPart )
	{
		cnt++;
		retPart->SetName(name_str);
	}

	return retPart;

}
	
void StructureMgr::CreateAddPart( int part_type )
{
	vector< Geom* > geomVec = aircraftPtr->getGeomVec();	
	if ( currGeomID >= 0 && currGeomID < (int)geomVec.size() )
	{
		Part* part = CreatePart( part_type );

		if ( part )
		{
			geomVec[currGeomID]->AddStructurePart( part );
			part->SetGeomPtr( geomVec[currGeomID] );
			vector< Part* > pVec = geomVec[currGeomID]->getStructurePartVec();
			geomVec[currGeomID]->SetCurrPartID( pVec.size() - 1 );
			EditCurrPart();
		}
		aircraftPtr->triggerDraw();
	}
}

void StructureMgr::OneSecondUpdate()
{
	Part* part = GetCurrPart();
	vector< Part* > partVec = GetAllParts();

	if ( part )
		part->UpdateOneSecond();

	if ( updateFlag )
	{
		updateFlag = false;

		for ( int i = 0 ; i < (int)partVec.size() ; i++ )
		{
			partVec[i]->SetStaleFlag( true );
			partVec[i]->UpdateOneSecond();
		}
	}
}

vector<Stringc> StructureMgr::GetPartNames()
{
	vector<Stringc> nameVec;



	return nameVec;
}

void StructureMgr::Draw()
{
	if ( !activeFlag )
		return;

	int i;
	vector< Part* > partVec = GetAllParts();

	bool drawFinal = true;
	for ( i = 0 ; i < (int)partVec.size() ; i++ )
	{
		if ( !partVec[i]->GetFinalMesh() )
			drawFinal = false;
	}

	if ( drawFinal )
	{
		//==== Draw Edge Lines ====//
		glLineWidth( 3.0 );
		glBegin( GL_LINES );
			glColor3f( 0.0, 1.0, 0.0 );								// Perimeter Edges
			for ( i = 0 ; i < (int)oneTriEdgeVec.size() ; i++ )
				glVertex3dv( oneTriEdgeVec[i].data() );
			glColor3f( 1.0, 1.0, 0.0 );								// Intersection Edges
			for ( i = 0 ; i < (int)fourTriEdgeVec.size() ; i++ )
				glVertex3dv( fourTriEdgeVec[i].data() );
			glColor3f( 1.0, 0.0, 0.0 );								// Other Edges
			for ( i = 0 ; i < (int)otherTriEdgeVec.size() ; i++ )
				glVertex3dv( otherTriEdgeVec[i].data() );
		glEnd();

		glLineWidth( 1.0 );
		glBegin( GL_LINES );
			glColor4f( 0.7f, 0.7f, 0.7f, 0.3f );								// Interior Edges
			for ( i = 0 ; i < (int)twoTriEdgeVec.size() ; i++ )
				glVertex3dv( twoTriEdgeVec[i].data() );
		glEnd();

		glLineWidth( 2.0 );
		glColor4f( 1.0f, 0.0f, 0.0f, 1.0f );								// Small Tris
		for ( i = 0 ; i < (int)smallTris.size() ; i+=3 )
		{
			glBegin( GL_LINE_LOOP );
				glVertex3dv( smallTris[i].data() );
				glVertex3dv( smallTris[i+1].data() );
				glVertex3dv( smallTris[i+2].data() );
			glEnd();
		}
		glLineWidth( 1.0 );

		//glBegin( GL_LINES );
		//	glColor4f( 1.0f, 0.2f, 0.2f, 1.0f );								// Draw Normals
		//	for ( i = 0 ; i < (int)centerVec.size() ; i++ )
		//	{
		//		glVertex3dv( centerVec[i].data() );
		//		vec3d poff = centerVec[i] + normVec[i]*0.25;
		//		glVertex3dv( poff.data() );
		//	}
		//glEnd();


	}

	for ( i = 0 ; i < (int)partVec.size() ; i++ )
	{
		partVec[i]->Draw();
	}



}

void StructureMgr::FinalMesh()
{
	int i, j;

	vector< Part* > partVec = GetAllParts();

	//====Find Perimeter ====//
	for ( i = 0 ; i < (int)partVec.size() ; i++ )
	{
		partVec[i]->FindPerimeter();
	}

	//==== Find Intersection Lines ====//
	for ( i = 0 ; i < (int)partVec.size() ; i++ )
	{
		for ( j = 0 ; j < (int)partVec.size() ; j++ )
		{
			//==== Find Intersections ====//
			if ( i != j )
				FindIntersection( partVec[i], partVec[j] );
		}
	}

	//==== Split And Add Intersections Lines To Perim ====//
	for ( i = 0 ; i < (int)partVec.size() ; i++ )
	{
		partVec[i]->perim.addSplitIntersections(sqrt(finalTriArea));
	}


	//==== Nice Triagulate ====//
	for ( i = 0 ; i < (int)partVec.size() ; i++ )
	{
		partVec[i]->NiceTriangulate(finalTriArea, false);
	}

	//==== Merge All Nodes and Tris ====//
	vector< TNode* > allNodeVec;
	vector< TTri > allTriVec;
	centerVec.clear();
	normVec.clear();

	double merge_tol = 0.01*sqrt( finalTriArea );
	for ( i = 0 ; i < (int)partVec.size() ; i++ )
	{
		TMesh* mesh = partVec[i]->GetFinalMesh();
		if ( mesh )
		{
			for ( j = 0 ; j < (int)mesh->tVec.size() ; j++ )
			{
				TTri* tri = mesh->tVec[j];
				TTri newtri;
				newtri.n0 = mesh->checkDupOrAdd( tri->n0, allNodeVec, merge_tol );
				newtri.n1 = mesh->checkDupOrAdd( tri->n1, allNodeVec, merge_tol );
				newtri.n2 = mesh->checkDupOrAdd( tri->n2, allNodeVec, merge_tol );

				//==== Center of Edge Nodes (for FEA) ====//
				vec3d cp0 = ( tri->n0->pnt + tri->n1->pnt )*0.5;
				newtri.cn0 = mesh->checkDupOrCreate( cp0, allNodeVec, merge_tol );
				vec3d cp1 = ( tri->n1->pnt + tri->n2->pnt )*0.5;
				newtri.cn1 = mesh->checkDupOrCreate( cp1, allNodeVec, merge_tol );
				vec3d cp2 = ( tri->n2->pnt + tri->n0->pnt )*0.5;
				newtri.cn2 = mesh->checkDupOrCreate( cp2, allNodeVec, merge_tol );

				if ( newtri.n0 != newtri.n1 && newtri.n0 != newtri.n2 && newtri.n1 != newtri.n2 )
					allTriVec.push_back( newtri );
			}
		}
	}
	//=== Renumber Nodes ====//
	for ( i = 0; i < (int)allNodeVec.size() ; i++ )
		allNodeVec[i]->id = i + 1;

	//=== Order Node So Normal Is Dominate Axis is Positive  ====//
	vec3d x_axis( 1.0, 0.01, 0.01 );
	for ( i = 0; i < (int)allTriVec.size() ; i++ )
	{
		TTri* tri = &allTriVec[i];
		//==== Compute Normal ====//
		vec3d norm = cross( tri->n1->pnt - tri->n0->pnt, tri->n2->pnt - tri->n0->pnt );
		double cang = cos_angle( x_axis, norm );

		if ( cang < 0.0 )
		{
			TNode* tmp = tri->n0;
			tri->n0 = tri->n1;
			tri->n1 = tmp;
			tmp = tri->cn2;
			tri->cn2 = tri->cn1;
			tri->cn1 = tmp;
		}
		vec3d center = (tri->n0->pnt + tri->n1->pnt + tri->n2->pnt)*0.333333333;
		centerVec.push_back( center );

		norm =  cross( tri->n1->pnt - tri->n0->pnt, tri->n2->pnt - tri->n0->pnt );
		norm.normalize();
		normVec.push_back( norm );
	}

	CheckValidity( allNodeVec, allTriVec );

//$ GRID 1001 is my example
//$--1---><--2---><--3---><--4---><--5---><--6---><--7---><--8---><--9--->
//GRID    1001    0       373.7990 41.5000  2.9010
//$ Pset: "pshell.1" will be imported as: "pshell.1"
//$2345678123456781234567812345678123456781234567812345678
//CTRIA3   1       1       1       2       3       0
//CTRIA3   2       1       4       5       6       0

	//===== Write NASTRAN Out File ====//
	FILE* fp = fopen( exportFileName.get_char_star(), "w" );
	if ( fp )
	{
		fprintf(fp, "$--1---><--2---><--3---><--4---><--5---><--6---><--7---><--8---><--9--->\n");
		fprintf(fp, "$TRIS\n");
		for ( i = 0; i < (int)allTriVec.size() ; i++ )
		{
			TTri* tri = &allTriVec[i];
			//fprintf(fp, "%-8s%-8d%-8d%-8d%-8d%-8d%-8d\n", "CTRIA3", i+1, 1, tri->n0->id, tri->n1->id, tri->n2->id, 0 );
			//fprintf(fp, "%-8s%-8d%-8d%-8d%-8d%-8d%-8d\n", "CTRIA3", i+1, 1, tri->cn0->id, tri->cn1->id, tri->cn2->id, 0 );
			fprintf(fp, "%-8s%-8d%-8d%-8d%-8d%-8d%-8d%-8d%-8d%-8d\n", 
				"CTRIA6", i+1, 1, tri->n0->id, tri->n1->id, tri->n2->id, tri->cn0->id, tri->cn2->id, tri->cn1->id,0 );
			//	"CTRIA6", i+1, 1, tri->n0->id, tri->cn0->id, tri->n1->id, tri->cn1->id, tri->n2->id, tri->cn2->id,0 );
		}
		fprintf(fp, "$--1---><--2---><--3---><--4---><--5---><--6---><--7---><--8---><--9--->\n");
		fprintf(fp, "$NODES\n");

		for ( i = 0; i < (int)allNodeVec.size() ; i++ )
		{
			TNode* n = allNodeVec[i];
			fprintf(fp, "%-8s%-8d%-8d%8.3f%8.3f%8.3f\n", "GRID", n->id, 0, n->pnt.x(), n->pnt.y(), n->pnt.z() );
		}
		fclose(fp);
	}

	//===== Write CalculiX File ====//
	fp = fopen( "FEA.msh", "w" );
	if ( fp )
	{
		fprintf(fp, "*node, nset=nall\n");
		for ( i = 0; i < (int)allNodeVec.size() ; i++ )
		{
			TNode* n = allNodeVec[i];
			fprintf(fp, "%d, %f, %f, %f\n", n->id, n->pnt.x(), n->pnt.y(), n->pnt.z() );
		}

		fprintf(fp, "*ELEMENT, TYPE=S6, ELSET=Eall\n");
		for ( i = 0; i < (int)allTriVec.size() ; i++ )
		{
			TTri* tri = &allTriVec[i];
			fprintf(fp, "%d, %d, %d, %d, %d, %d, %d\n", 
				i+1, tri->n0->id, tri->n1->id, tri->n2->id, tri->cn0->id, tri->cn1->id, tri->cn2->id );
		}

		fclose(fp);

	}

	


	ForceDraw();

}

void StructureMgr::CheckValidity( vector< TNode* > & nVec, vector< TTri > & tVec )
{
	int i, j;
	char str[256];

	finalResultStringVec.clear();
	smallTris.clear();

	sprintf(str, "Num Nodes = %d\n", (int)nVec.size() );			finalResultStringVec.push_back( str );
	sprintf(str, "NumTris = %d\n", (int)tVec.size());			finalResultStringVec.push_back( str );

	//==== Verify 3 Distinct Nodes for Each Tri ====//
	for ( i = 0; i < (int)tVec.size() ; i++ )
	{
		if ( tVec[i].n0 == tVec[i].n1 || tVec[i].n1 == tVec[i].n2 || tVec[i].n0 == tVec[i].n2 )
		{
				sprintf(str, "-> Duplicate Node in Tri %d\n", i);		finalResultStringVec.push_back( str );
		}
	}

	enum { MIN_VAL, MAX_VAL, AVG_VAL };

	//==== Check Triangle Areas ====//
	finalArea[MAX_VAL] = finalArea[AVG_VAL] = 0.0;
	finalArea[MIN_VAL] = 1e06;

	finalAngle[MAX_VAL] = 0.0;
	finalAngle[MIN_VAL] = 1.e06;

	for ( i = 0; i < (int)tVec.size() ; i++ )
	{
		double a = tVec[i].computeArea();
		finalArea[AVG_VAL] += a;

		if ( a > finalArea[MAX_VAL] ) finalArea[MAX_VAL] = a;
		if ( a < finalArea[MIN_VAL] ) finalArea[MIN_VAL] = a;

		double ang[3];
		ang[0] = RAD_2_DEG*angle( tVec[i].n1->pnt - tVec[i].n0->pnt, tVec[i].n2->pnt - tVec[i].n0->pnt );
		ang[1] = RAD_2_DEG*angle( tVec[i].n0->pnt - tVec[i].n1->pnt, tVec[i].n2->pnt - tVec[i].n1->pnt );
		ang[2] = RAD_2_DEG*angle( tVec[i].n1->pnt - tVec[i].n2->pnt, tVec[i].n0->pnt - tVec[i].n2->pnt );

		bool smallFlag = false;
		for ( j = 0 ; j < 3 ; j++ )
		{
			if ( ang[j] > finalAngle[MAX_VAL] ) finalAngle[MAX_VAL] = ang[j];
			if ( ang[j] < finalAngle[MIN_VAL] ) finalAngle[MIN_VAL] = ang[j];

			if ( ang[j] < 5.0 )
				smallFlag = true;
		}
		if ( smallFlag )
		{
			smallTris.push_back( tVec[i].n0->pnt );
			smallTris.push_back( tVec[i].n1->pnt );
			smallTris.push_back( tVec[i].n2->pnt );
		}

	}
	finalArea[AVG_VAL] /= (double)tVec.size();


	sprintf(str, "\nMin Area = %f\n", finalArea[MIN_VAL] );			finalResultStringVec.push_back( str );
	sprintf(str, "Max Area = %f\n", finalArea[MAX_VAL] );			finalResultStringVec.push_back( str );
	sprintf(str, "Avg Area = %f\n", finalArea[AVG_VAL] );			finalResultStringVec.push_back( str );

	sprintf(str, "\nMin Angle = %f\n", finalAngle[MIN_VAL] );		finalResultStringVec.push_back( str );
	sprintf(str, "Max Angle = %f\n", finalAngle[MAX_VAL] );			finalResultStringVec.push_back( str );

	//==== Check Edge Lengths ====//
	vector< TEdge > eVec;
	for ( i = 0; i < (int)tVec.size() ; i++ )
	{
		TEdge edge;
		edge.n0 = tVec[i].n0;
		edge.n1 = tVec[i].n1;
		eVec.push_back( edge );
		edge.n0 = tVec[i].n1;
		edge.n1 = tVec[i].n2;
		eVec.push_back( edge );
		edge.n0 = tVec[i].n0;
		edge.n1 = tVec[i].n2;
		eVec.push_back( edge );
	}

	finalEdgeLen[MAX_VAL] = finalEdgeLen[AVG_VAL] = 0.0;
	finalEdgeLen[MIN_VAL] = 1e06;
	for ( i = 0 ; i < (int)eVec.size() ; i++ )
	{
		double d = dist( eVec[i].n0->pnt, eVec[i].n1->pnt );
		finalEdgeLen[AVG_VAL] += d;
		if ( d > finalEdgeLen[MAX_VAL] ) finalEdgeLen[MAX_VAL] = d;
		if ( d < finalEdgeLen[MIN_VAL] ) finalEdgeLen[MIN_VAL] = d;
	}
	finalEdgeLen[AVG_VAL] /= (double)eVec.size();

	sprintf(str, "\nMin Edge Len = %f\n", finalEdgeLen[MIN_VAL] );		finalResultStringVec.push_back( str );
	sprintf(str, "Max Edge Len = %f\n", finalEdgeLen[MAX_VAL] );		finalResultStringVec.push_back( str );
	sprintf(str, "Avg Edge Len = %f\n", finalEdgeLen[AVG_VAL] );		finalResultStringVec.push_back( str );

	//==== Tally Num Tris That Share Each Edge ====//
	vector< int > numDups;
	numDups.resize( eVec.size() );

	for ( i = 0 ; i < (int)numDups.size() ; i++ )
		numDups[i] = 0;

	for ( i = 0 ; i < (int)eVec.size() ; i++ )
	{
		if ( numDups[i] == 0 )
		{
			numDups[i] = 1;
			for ( j = i+1 ; j < (int)eVec.size() ; j++ )
			{
					if (  ( eVec[i].n0 == eVec[j].n0 && eVec[i].n1 == eVec[j].n1 )||
						  ( eVec[i].n0 == eVec[j].n1 && eVec[i].n1 == eVec[j].n0 ) )
					{
						numDups[i]++;
						numDups[j] = -1;
					}
			}
		}
	}

	oneTriEdgeVec.clear();
	twoTriEdgeVec.clear();
	fourTriEdgeVec.clear();

	for ( i = 0 ; i < (int)eVec.size() ; i++ )
	{
		if ( numDups[i] == 1 )
		{	
			oneTriEdgeVec.push_back( eVec[i].n0->pnt ); 
			oneTriEdgeVec.push_back( eVec[i].n1->pnt );
		}
		else if ( numDups[i] == 2 )
		{
			twoTriEdgeVec.push_back( eVec[i].n0->pnt ); 
			twoTriEdgeVec.push_back( eVec[i].n1->pnt );
		}
		else if ( numDups[i] == 4 )
		{
			fourTriEdgeVec.push_back( eVec[i].n0->pnt );
			fourTriEdgeVec.push_back( eVec[i].n1->pnt );
		}
		else if ( numDups[i] != -1 )
		{
			otherTriEdgeVec.push_back( eVec[i].n0->pnt );
			otherTriEdgeVec.push_back( eVec[i].n1->pnt );
		}
	}

	sprintf(str, "\nInterior Edges = %d\n", (int)twoTriEdgeVec.size());			finalResultStringVec.push_back( str );
	sprintf(str, "Border Edges = %d\n", (int)oneTriEdgeVec.size());				finalResultStringVec.push_back( str );
	sprintf(str, "Isect Edges = %d\n", (int)fourTriEdgeVec.size());				finalResultStringVec.push_back( str );
	sprintf(str, "Bogus Edges = %d\n", (int)otherTriEdgeVec.size());				finalResultStringVec.push_back( str );



}

void StructureMgr::FindIntersection( Part* part0, Part* part1 )
{
	int i;

	double t;
	vec3d p0, p1;
	vec3d orig  = part0->GetOrig();
	vec3d axis0 = part0->GetAxis0();
	vec3d axis1 = part0->GetAxis1();

	vector< ISectNode > isectNodeVec;

	for ( i = 0 ; i <(int)part1->perim.eVec.size() ; i++ )
	{
		p0 = part1->perim.eVec[i]->n0->pnt;
		p1 = part1->perim.eVec[i]->n1->pnt;
		vec3d p1_p0 = p1 - p0;
		int isectFlag = plane_ray_intersect( orig, axis0, axis1, p0, p1_p0, t );

		if ( isectFlag && t >= 0.0 && t <= 1.0 )
		{
			//==== Create Node ====//
			ISectNode inode;
			inode.pnt = p0 + (p1-p0)*t;
			inode.edge = part1->perim.eVec[i];

			//==== Check for Duplicate Points ====//
			bool addFlag = true;
			for ( int j = 0 ; j < (int)isectNodeVec.size() ; j++ )
			{
				double d = dist( inode.pnt, isectNodeVec[j].pnt );
				if ( d < 0.000001 )
					addFlag = false;
			}
			if ( addFlag )
				isectNodeVec.push_back( inode );
		}
	}

	if ( isectNodeVec.size() >= 2 )
	{
		PNode* anode0 = part1->perim.insertPnt( isectNodeVec[0].edge, isectNodeVec[0].pnt );
		PNode* anode1 = part1->perim.insertPnt( isectNodeVec[1].edge, isectNodeVec[1].pnt );

		PEdge* aedge = new PEdge();
		aedge->n0 = anode0;
		aedge->n1 = anode1;
		part1->perim.isectEdgeVec.push_back( aedge );
	}

}













