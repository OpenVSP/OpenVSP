//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//

//******************************************************************************
//    
//    Part Class
//  
// 
//   J.R. Gloudemans - 9/24/06
//
//    
//******************************************************************************

#ifdef WIN32
#include <windows.h>		
#endif

#ifdef __APPLE__
#  include <OpenGL/gl.h>
#else
#  include <GL/gl.h>
#endif

#include <FL/Fl.H>


#include "part.h"
#include "geom.h"
#include "structureMgr.h"
#include "materialMgr.h"

#include "triangle.h"

//===============================================//
//					Perimeter
//===============================================//
Perimeter::~Perimeter()
{
	clear();
}

void Perimeter::clear()
{
	int i;
	for ( i = 0 ; i < (int)nVec.size() ; i++ )
		delete nVec[i];
	for ( i = 0 ; i < (int)eVec.size() ; i++ )
		delete eVec[i];
	for ( i = 0 ; i < (int)isectEdgeVec.size() ; i++ )
		delete isectEdgeVec[i];
	
	nVec.clear();
	eVec.clear();
	isectEdgeVec.clear();
	
}

void Perimeter::load( vector< TNode* > & nodeVec, vector< TEdge* > edgeVec )
{
	int i;
	clear();

	nVec.resize( nodeVec.size() );
	for ( i = 0 ; i < (int)nodeVec.size() ; i++ )
	{
		nodeVec[i]->id = i;
		nVec[i] = new PNode();
		nVec[i]->id  = i;
		nVec[i]->pnt = nodeVec[i]->pnt;
	}

	eVec.resize( edgeVec.size() );
	for ( i = 0 ; i < (int)edgeVec.size() ; i++ )
	{
		eVec[i] = new PEdge();
		eVec[i]->n0 = nVec[edgeVec[i]->n0->id];
		eVec[i]->n1 = nVec[edgeVec[i]->n1->id];
		eVec[i]->n0->edgeVec.push_back( eVec[i] );
		eVec[i]->n1->edgeVec.push_back( eVec[i] );
	}	
}

PNode* Perimeter::insertPnt( PEdge* edge, vec3d & pnt )
{
	double tol = 0.1*dist( edge->n0->pnt,  edge->n1->pnt );

	if ( dist( pnt, edge->n0->pnt ) < tol )
	{
		edge->n0->pnt = pnt;			// Move Node
		return edge->n0;
	}
	if ( dist( pnt, edge->n1->pnt ) < tol )
	{
		edge->n1->pnt = pnt;			// Move Node
		return edge->n1;
	}

	//==== Insert ====//
	PNode* node = new PNode();
	node->pnt = pnt;
	nVec.push_back( node );
	
	PEdge* newedge = new PEdge();
	eVec.push_back( newedge );
	newedge->n0 = node;
	newedge->n1 = edge->n1;
	edge->n1 = node;

	return node;
}

void Perimeter::addSplitIntersections(double targetLength)
{
	int i, j;

	//==== Split Edges ====//
	vector< PEdge* > newEdgeVec; 
	for ( i = 0 ; i < (int)eVec.size() ; i++ )
	{
		PEdge* e = eVec[i];
		double len = dist( e->n0->pnt, e->n1->pnt );
		int numSegs = (int)(len/targetLength) + 1;

		vector< PNode* > pnVec;
		for ( j = 1 ; j < numSegs ; j++ )
		{
			double fract = (double)j/(double)numSegs;
			PNode* newnode = new PNode();
			newnode->pnt = e->n0->pnt + (e->n1->pnt - e->n0->pnt)*fract;
			nVec.push_back(newnode);
			pnVec.push_back( newnode );
		}
		pnVec.push_back( e->n1 );

		e->n1 = pnVec[0];

		for ( j = 0 ; j < (int)pnVec.size() - 1 ; j++ ) 
		{
			PEdge* newedge = new PEdge();
			newEdgeVec.push_back( newedge );
			newedge->n0 = pnVec[j];
			newedge->n1 = pnVec[j+1];
		}
	}

	for ( i = 0 ; i < (int)newEdgeVec.size() ; i++ )
		eVec.push_back( newEdgeVec[i] );


	//==== Split and Add Intersection Edges ====//
	for ( i = 0 ; i < (int)isectEdgeVec.size() ; i++ )
	{
		PEdge* e = isectEdgeVec[i];
		double len = dist( e->n0->pnt, e->n1->pnt );
		int numSegs = (int)(len/targetLength) + 1;

		vector< PNode* > pnVec;
		pnVec.push_back( e->n0 );
		for ( j = 1 ; j < numSegs ; j++ )
		{
			double fract = (double)j/(double)numSegs;
			PNode* newnode = new PNode();
			newnode->pnt = e->n0->pnt + (e->n1->pnt - e->n0->pnt)*fract;
			nVec.push_back(newnode);
			pnVec.push_back( newnode );
		}
		pnVec.push_back( e->n1 );

		for ( j = 0 ; j < (int)pnVec.size() - 1 ; j++ )
		{
			PEdge* newedge = new PEdge();
			eVec.push_back( newedge );
			newedge->n0 = pnVec[j];
			newedge->n1 = pnVec[j+1];
		}
	}


}


void Perimeter::removeSmallEdges(double thresh )
{
	int i;

	for ( i = 0 ; i < (int)nVec.size() ; i++ )
	{
		PNode* node = nVec[i];
		if ( node->edgeVec.size() == 2 )
		{
			//==== Compute Size ====//
			double len0 = node->edgeVec[0]->length();
			double len1 = node->edgeVec[1]->length();

			double denom = len0 + len1;
			if ( denom )
			{
				double per0 = len0/denom;
				double per1 = len1/denom;

				if ( per0 < thresh )
				{
					vec3d avgpnt = (node->edgeVec[0]->n0->pnt + node->edgeVec[0]->n1->pnt)*0.5;
					node->edgeVec[0]->n0->pnt = avgpnt;
					node->edgeVec[0]->n1->pnt = avgpnt;
				}
				else if ( per1 < thresh )
				{
					vec3d avgpnt = (node->edgeVec[1]->n0->pnt + node->edgeVec[1]->n1->pnt)*0.5;
					node->edgeVec[1]->n0->pnt = avgpnt;
					node->edgeVec[1]->n1->pnt = avgpnt;
				}
			}
		}
	}

	//==== Remove Zero Length Edges ====//
	vector< PEdge* > tmpEVec;
	for ( i = 0 ; i < (int)eVec.size() ; i++ )
	{
		if ( eVec[i]->length() < 0.0000001 )
			delete eVec[i];
		else
			tmpEVec.push_back( eVec[i] );
	}
	eVec = tmpEVec;
}

void Perimeter::writeToFile( const char* filename )
{
	FILE* fp = fopen(filename, "w");

	if ( !fp )
		return;
	
	for ( int i = 0 ; i < (int)eVec.size() ; i++ )
	{
		fprintf( fp, "%f %f\n", eVec[i]->n0->pnt.x(), eVec[i]->n0->pnt.y() );
		fprintf( fp, "%f %f\n", eVec[i]->n1->pnt.x(), eVec[i]->n1->pnt.y() );
		fprintf( fp, "MOVE\n" );
	}

	fclose(fp);
	


}


//===============================================//
//					Part
//===============================================//
Part::Part()
{
	name = "Default";
	geomPtr = NULL;
	staleFlag = true;
	staleMesh = true;
	editFlag = false;
	sliceMesh = NULL;
	finalMesh = NULL;

}

Part::~Part()
{

}

void Part::DeleteFinalMesh()
{
	if ( finalMesh )
		delete finalMesh;
	finalMesh = 0;
}

void Part::Copy( Part* fromPart )
{
	name = fromPart->GetName();
}

void Part::Draw()
{
}


void Part::MergeNodesRemoveEdges( vector< TNode* > & nodeVec, vector< TEdge* > & edgeVec, double tol )
{
	int i;

	if ( !edgeVec.size() )
		return;


	vector< TEdge* > redEVec;
	vector< TNode* > redNVec;
	for ( i = 0 ; i < (int)edgeVec.size() ; i++ )
	{
		edgeVec[i]->n0 = TMesh::checkDupOrAdd( edgeVec[i]->n0, redNVec, tol );
		edgeVec[i]->n1 = TMesh::checkDupOrAdd( edgeVec[i]->n1, redNVec, tol );
	}

	for ( i = 0 ; i < (int)edgeVec.size() ; i++ )
	{
		if ( edgeVec[i]->n0 != edgeVec[i]->n1 )
			redEVec.push_back( edgeVec[i] );
	}

	nodeVec = redNVec;
	edgeVec = redEVec;

}



//===============================================//
//					Slice
//===============================================//
ThreePntSlice::ThreePntSlice() : Part()
{
	type = THREE_PNT_SLICE;

	uwPnt[0] = vec2d( 0.5, 0.0 );
	uwPnt[1] = vec2d( 0.5, 0.5 );
	uwPnt[2] = vec2d( 0.5, 0.8 );
}

ThreePntSlice::~ThreePntSlice()
{
	if ( sliceMesh )
		delete sliceMesh;

	if ( finalMesh )
		delete finalMesh;
}

void ThreePntSlice::Copy( Part* fromPart )
{
	name = fromPart->GetName();
	uwPnt[0] = ((ThreePntSlice*)fromPart)->GetUWPnt(0);
	uwPnt[1] = ((ThreePntSlice*)fromPart)->GetUWPnt(1);
	uwPnt[2] = ((ThreePntSlice*)fromPart)->GetUWPnt(2);
}

void ThreePntSlice::WriteParms( xmlNodePtr root )
{
	xmlAddStringNode( root, "Name", GetName() );
	xmlAddIntNode( root, "Type", type );

	xmlAddDoubleNode( root, "U0", uwPnt[0].x() );
	xmlAddDoubleNode( root, "W0", uwPnt[0].y() );

	xmlAddDoubleNode( root, "U1", uwPnt[1].x() );
	xmlAddDoubleNode( root, "W1", uwPnt[1].y() );

	xmlAddDoubleNode( root, "U2", uwPnt[2].x() );
	xmlAddDoubleNode( root, "W2", uwPnt[2].y() );

}

void ThreePntSlice::ReadParms( xmlNodePtr root )
{
	SetName( xmlFindString( root, "Name", GetName() ) );

	uwPnt[0].set_x( xmlFindDouble( root, "U0", uwPnt[0].x() ) );
	uwPnt[0].set_y( xmlFindDouble( root, "W0", uwPnt[0].y() ) );

	uwPnt[1].set_x( xmlFindDouble( root, "U1", uwPnt[1].x() ) );
	uwPnt[1].set_y( xmlFindDouble( root, "W1", uwPnt[1].y() ) );

	uwPnt[2].set_x( xmlFindDouble( root, "U2", uwPnt[2].x() ) );
	uwPnt[2].set_y( xmlFindDouble( root, "W2", uwPnt[2].y() ) );

	staleFlag = true;

}



void ThreePntSlice::SetUPnt( double u, int ind )
{
	if ( ind >= 0 && ind <= 2 )
		uwPnt[ind].set_x( u );

	staleFlag = true;
}

void ThreePntSlice::SetWPnt( double w, int ind )
{
	if ( ind >= 0 && ind <= 2 )
		uwPnt[ind].set_y( w );

	staleFlag = true;


}

vec2d ThreePntSlice::GetUWPnt( int ind )
{
	if ( ind >= 0 && ind <= 2 )
		return uwPnt[ind];

	return uwPnt[0];
}


void ThreePntSlice::UpdateOneSecond()
{
	if ( staleFlag )
		ComputePlanePnts();

	if ( !staleMesh )
		return;
	staleMesh = false;

	if ( sliceMesh )
		delete sliceMesh;
	sliceMesh = 0;

	if ( finalMesh )
		delete finalMesh;
	finalMesh = 0;

	sliceMesh = ComputeTMesh();

	structureMgrPtr->ForceDraw();

	staleFlag = false;

}
	


//==== Compute Plane Points =====//
void ThreePntSlice::ComputePlanePnts()
{
	int i;
	if ( !geomPtr )
		return;

	if ( !staleFlag )
		return;

	staleFlag = false;
	staleMesh = true;

	//==== Find Plane ====//
	projPnt[0] = geomPtr->getAttachUVPos( uwPnt[0].x(), uwPnt[0].y() );
	projPnt[1] = geomPtr->getAttachUVPos( uwPnt[1].x(), uwPnt[1].y() );
	projPnt[2] = geomPtr->getAttachUVPos( uwPnt[2].x(), uwPnt[2].y() );

	//==== Update And Get Bounding Box ====//
	geomPtr->update_xformed_bbox();
	bbox box = geomPtr->get_bnd_box();	
	double maxdist = box.diag_dist();

	//==== Orth Dir ====//
	vec3d p = proj_pnt_on_ray( projPnt[0], projPnt[1], projPnt[2] );
	vec3d orth = p - projPnt[2];
	orth.normalize();

	//==== Corner Points of Slice Plane ====//
	vec3d axis = projPnt[1] - projPnt[0];
	axis.normalize();

	//==== Find Center of Plane ====//
	vec3d cent = (projPnt[0] + projPnt[1] + projPnt[2])*(1.0/3.0);

	//==== Project Bound Box On Axis ====//
	double d;
	double axis_max_dist = 0.0;
	double orth_max_dist = 0.0;
	for ( i = 0 ; i < 8 ; i++ )
	{
		vec3d projp = proj_pnt_on_ray( cent, cent + axis, box.get_pnt(i) );		// Project on Axis
		d = dist( cent, projp );
		if ( d > axis_max_dist )
			axis_max_dist = d;

		projp = proj_pnt_on_ray( cent, cent + orth, box.get_pnt(i) );		// Project on Orth
		d = dist( cent, projp );
		if ( d > orth_max_dist )
			orth_max_dist = d;
	}
	axis_max_dist *= 1.0001;
	orth_max_dist *= 1.0001;

	plnPnt[0] = cent - axis*axis_max_dist - orth*orth_max_dist; 
	plnPnt[1] = cent - axis*axis_max_dist + orth*orth_max_dist; 
	plnPnt[2] = cent + axis*axis_max_dist + orth*orth_max_dist; 
	plnPnt[3] = cent + axis*axis_max_dist - orth*orth_max_dist; 
}

//==== Draw =====//
void ThreePntSlice::Draw()
{
	int i;
	if ( !geomPtr )
		return;

	ComputePlanePnts();

	if ( editFlag )
	{
		//==== Draw Grid Lines ====//
		int num_grid = 5;
		glColor4f( 0.0f, 0.0f, 0.0f, 1.0f );
		glBegin( GL_LINES );
		for ( i = 0 ; i < num_grid ; i++ )
		{
			double fract = (double)i/(double)(num_grid-1);
			vec3d g0 = plnPnt[0] + (plnPnt[1] - plnPnt[0])*fract;	
			glVertex3dv( g0.data() );

			vec3d g1 = plnPnt[3] + (plnPnt[2] - plnPnt[3])*fract;	
			glVertex3dv( g1.data() );

			vec3d g2 = plnPnt[1] + (plnPnt[2] - plnPnt[1])*fract;	
			glVertex3dv( g2.data() );

			vec3d g3 = plnPnt[0] + (plnPnt[3] - plnPnt[0])*fract;	
			glVertex3dv( g3.data() );

		}
		glEnd();	

		//==== Draw 3 Points ====//
		glPointSize( 10.0 );
		glBegin( GL_POINTS );
		glColor4f( 1.0f, 1.0f, 0.0f, 1.0f );
		glVertex3dv( projPnt[0].data() );
		glColor4f( 0.0f, 1.0f, 0.0f, 1.0f );
		glVertex3dv( projPnt[1].data() );
		glColor4f( 1.0f, 0.0f, 1.0f, 1.0f );
		glVertex3dv( projPnt[2].data() );
		glEnd();

		if ( staleMesh )
		{
			//==== Draw Plane ====//
			glColor4f( 0.1f, 0.1f, 0.1f, 0.2f );
			glBegin( GL_POLYGON );	
			glVertex3dv( plnPnt[0].data() );
			glVertex3dv( plnPnt[1].data() );
			glVertex3dv( plnPnt[2].data() );
			glVertex3dv( plnPnt[3].data() );
			glEnd();
		}

	}
	//==== Which Mesh To Draw ====//
	TMesh* mesh = sliceMesh;
	if ( finalMesh )
		mesh = finalMesh;

	if ( !staleMesh && mesh  )
	{
		glLineWidth( 2.0 );
		if ( editFlag )
		{
			glColor4f( 1.0, 0.7f, 0.0, 1.0 );
		}
		else
		{
			glColor4f( 0.3f, 0.3f, 0.3f, 1.0 );
		}

		if ( !finalMesh )
			mesh->draw_wire();
//		mesh->draw_shaded();
	}



}

////////////////////////////////////////////////////////////////////////////////////////////////////

//==== Compute Slice ====//
TMesh* ThreePntSlice::ComputeTMesh( )
{
	int i, j;
	if ( !geomPtr )
		return NULL;

	vec3d norm = cross(plnPnt[1] - plnPnt[0], plnPnt[2] - plnPnt[0]);

	////==== Create TMesh ====//
	TMesh* tm = new TMesh();

	tm->addTri( plnPnt[0], plnPnt[1], plnPnt[2], norm );
	tm->addTri( plnPnt[0], plnPnt[2], plnPnt[3], norm );
	tm->loadBndBox();

	//==== Create TMeshs ====//
	vector< TMesh* > tMeshVec = geomPtr->createTMeshVec();

	//==== Intersect All Mesh Geoms ====//
	for ( i = 0 ; i < (int)tMeshVec.size() ; i++ )
	{
		tMeshVec[i]->loadBndBox();
		tm->intersect( tMeshVec[i] );

		for ( j = 0 ; j < (int)tMeshVec[i]->tVec.size() ; j++ )
		{
			TTri* tri = tMeshVec[i]->tVec[j];
			for ( int e = 0 ; e < (int)tri->iSectEdgeVec.size() ; e++ )
			{
				delete tri->iSectEdgeVec[e]->n0;
				delete tri->iSectEdgeVec[e]->n1;
				delete tri->iSectEdgeVec[e]; 
			}
			tri->iSectEdgeVec.erase( tri->iSectEdgeVec.begin(), tri->iSectEdgeVec.end() );
		}
	}

	//==== Split Intersected Tri in Mesh ====//
	tm->split();

	//==== Determine Which Triangle Are Interior/Exterior ====//
	tm->deterIntExt( tMeshVec );

	//==== Flip Int/Ext Flags ====//
	for ( i = 0 ; i < (int)tm->tVec.size() ; i++ )
	{
		TTri* tri = tm->tVec[i];
		if ( tri->splitVec.size() )
		{
			for ( j = 0 ; j < (int)tri->splitVec.size() ; j++ )
			{
				tri->splitVec[j]->interiorFlag = !(tri->splitVec[j]->interiorFlag);
			}
		}
		else
		{
			tri->interiorFlag = !(tri->interiorFlag);
		}
	}

	//==== Delete tMeshVec
	for ( i = 0 ; i < (int)tMeshVec.size() ; i++ )
		delete tMeshVec[i];

	return tm;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void ThreePntSlice::FindPerimeter()
{
	int i,j;

	//==== Make Sure Mesh Is Up To Date ====//
	if ( staleFlag )
	{
		ComputePlanePnts();
		if ( sliceMesh )
			delete sliceMesh;
	}

	if ( !sliceMesh )
		sliceMesh = ComputeTMesh();

	if ( !sliceMesh )
		return;

	//==== Loads All Exterior Tris/Nodes into Vecs
	vector< TTri* > triVec;
	vector< TNode* > nodeVec;
	sliceMesh->mergeSplitNodes(triVec, nodeVec);		

	//==== Create Edge List ====//
	TEdge* e;
	vector< TEdge* > edgeVec;
	for ( i = 0 ; i < (int)triVec.size() ; i++ )
	{
		TTri* t = triVec[i];		
		e = new TEdge;	e->n0 = t->n0;	e->n1 = t->n1;	edgeVec.push_back( e );
		e = new TEdge;	e->n0 = t->n1;	e->n1 = t->n2;	edgeVec.push_back( e );
		e = new TEdge;	e->n0 = t->n2;	e->n1 = t->n0;	edgeVec.push_back( e );
	}

	//==== Find Edges That Dont Share Two Tris
	vector < int > perimFlags;
	perimFlags.resize( edgeVec.size() );
	for ( i = 0 ; i < (int)perimFlags.size() ; i++ )
		perimFlags[i] = 1;

	for ( i = 0 ; i < (int)edgeVec.size() ; i++ )
	{
		for ( j = i+1 ; j < (int)edgeVec.size() ; j++ )
		{
			if ( edgeVec[i]->n0 == edgeVec[j]->n0 && edgeVec[i]->n1 == edgeVec[j]->n1 || 
				 edgeVec[i]->n1 == edgeVec[j]->n0 && edgeVec[i]->n0 == edgeVec[j]->n1 )
			{
				perimFlags[i] = 0;
				perimFlags[j] = 0;
			}
		}
	}

	vector< TEdge* > perimEdges;
	for ( i = 0 ; i < (int)edgeVec.size() ; i++ )
	{
		if ( perimFlags[i] )
			perimEdges.push_back( edgeVec[i] );
	}

	if ( perimEdges.size() < 3 )
		return;

	//==== Find Average Edge Length ====//
	double avg = 0;
	for ( i = 0 ; i < (int)perimEdges.size() ; i++ )
		avg += dist( perimEdges[i]->n0->pnt, perimEdges[i]->n1->pnt );
	avg = avg/(double)perimEdges.size();

	//==== Merge Nodes and Remove Small Edges ====//
	MergeNodesRemoveEdges( nodeVec, perimEdges, 0.25*avg );

	perim.load( nodeVec, perimEdges );

	//==== Delete Edges ====//
	for ( i = 0 ; i < (int)edgeVec.size() ; i++ )
		delete edgeVec[i];

//	perim.removeSmallEdges( 0.10 );

	//char str[256];
	//sprintf( str, "%s.dat", name.get_char_star() );
	//perim.writeToFile( str ); 

}

////////////////////////////////////////////////////////////////////////////////////////////////////
void ThreePntSlice::NiceTriangulate( double size, bool finalFlag )
{
	int i;

	//==== Convert To UW Points =====//
	vec2d uw;
	vec3d orig  = GetOrig();
	vec3d axis0 = GetAxis0();
	vec3d axis1 = GetAxis1();

	for ( i = 0 ; i < (int)perim.nVec.size() ; i++ )
	{
		perim.nVec[i]->uw = MapToPlane( perim.nVec[i]->pnt, orig, axis0, axis1 );
	}

	//==== Dump Into Triangle ====//
	struct triangulateio in;
	struct triangulateio out;

	memset(&in,0,sizeof(in));		// Load Zeros
	memset(&out,0,sizeof(out));

	//==== PreAllocate Data For In/Out ====//
	in.numberofpoints = perim.nVec.size();
	in.pointlist    = (REAL *) malloc(in.numberofpoints * 2 * sizeof(REAL));
	out.pointlist   = NULL;

	in.numberofsegments = perim.eVec.size();
	in.segmentlist  = (int *) malloc(in.numberofsegments * 2 * sizeof(int));
	out.segmentlist  = NULL;
	out.trianglelist  = NULL;

    in.numberofpointattributes = 0;
    in.pointattributelist = NULL;
    in.pointmarkerlist = NULL;
    in.numberofholes = 0;
    in.numberoftriangles = 0;
    in.numberofpointattributes = 0;
    in.numberofedges = 0;
    in.trianglelist = NULL;
    in.trianglearealist = NULL;
    in.edgelist = NULL;
    in.edgemarkerlist = NULL;
    in.segmentmarkerlist = NULL; 

	for ( i = 0 ; i < (int)perim.nVec.size() ; i++ )
	{
		perim.nVec[i]->id = i;
		in.pointlist[i*2]   = perim.nVec[i]->uw.x();
		in.pointlist[i*2+1] = perim.nVec[i]->uw.y();
	}
	for ( i = 0 ; i < (int)perim.eVec.size() ; i++ )
	{
		in.segmentlist[i*2]   = perim.eVec[i]->n0->id;
		in.segmentlist[i*2+1] = perim.eVec[i]->n1->id;
	}

	char cmd[256];
	sprintf( cmd, "YYzpQa%0.3fq15.0", size );		
	triangulate (cmd, &in, &out, (struct triangulateio *) NULL);

	if ( finalMesh )
		delete finalMesh;
	finalMesh = new TMesh();

	vector< TNode* > nodeVec;

	//==== Load Nodes ====//
	for ( i = 0 ; i < out.numberofpoints ; i++ )
	{
		TNode* node = new TNode();
		vec2d uw( out.pointlist[i*2], out.pointlist[i*2+1] );
		node->pnt = MapFromPlane( uw, orig, axis0, axis1 );
		nodeVec.push_back( node );
	}

	//==== Load Tris ====//
	vec3d norm;
	for ( i = 0 ; i < out.numberoftriangles ; i++ )
	{
		TNode* n0 = nodeVec[ out.trianglelist[i*3] ];
		TNode* n1 = nodeVec[ out.trianglelist[i*3+1] ];
		TNode* n2 = nodeVec[ out.trianglelist[i*3+2] ];
		finalMesh->addTri( n0, n1, n2, norm );
	}

	//==== Free Local Memory ====//
	if ( in.pointlist )				free( in.pointlist );
	if ( in.segmentlist )			free( in.segmentlist );

	if ( out.pointlist )			free( out.pointlist );
	if ( out.pointmarkerlist )		free( out.pointmarkerlist );
	if ( out.trianglelist )			free( out.trianglelist );
	if ( out.segmentlist )			free( out.segmentlist );
	if ( out.segmentmarkerlist )	free( out.segmentmarkerlist );



}


//
//void ThreePntSlice::NiceTriangulate( double size, bool finalFlag )
//{
//	//if ( staleFlag )
//	//{
//	//	ComputePlanePnts();
//
//	//	if ( sliceMesh )
//	//		delete sliceMesh;
//	//}
//
//	//if ( !sliceMesh )
//	//	sliceMesh = ComputeTMesh();
//
//	//if ( !sliceMesh )
//	//	return;
//
//
//	//int i, j;
//	//vector< TTri* > triVec;
//	//vector< TNode* > nodeVec;
//	//vector< TEdge* > isectEdgeVec;
//	//vector< TNode* > isectNodeVec;
//
//	//if ( finalFlag )
//	//{
//	//	for ( i = 0 ; i < (int)finalMesh->tVec.size() ; i++ )
//	//	{
//	//		TTri* t = finalMesh->tVec[i];
//	//		for ( j = 0 ; j < (int)t->iSectEdgeVec.size() ; j++ )
//	//		{
//	//			TEdge* e = t->iSectEdgeVec[j];
//	//			isectEdgeVec.push_back( e );
//	//			isectNodeVec.push_back( e->n0 );
//	//			isectNodeVec.push_back( e->n1 );
//	//		}
//	//	}
//	//}
//
//	//sliceMesh->mergeSplitNodes(triVec, nodeVec);		// Loads All Exterior Tris/Nodes into Vecs
//
//	//if ( finalFlag )
//	//{
//	//	for ( i = 0 ; i < (int)isectNodeVec.size() ; i++ )
//	//	{
//	//		nodeVec.push_back( isectNodeVec[i] );
//	//	}
//	//}
//
//	////==== Create Edge List ====//
//	//TEdge* e;
//	//vector< TEdge* > edgeVec;
//	//for ( i = 0 ; i < (int)triVec.size() ; i++ )
//	//{
//	//	TTri* t = triVec[i];
//	//	
//	//	e = new TEdge;
//	//	e->n0 = t->n0;
//	//	e->n1 = t->n1;
//	//	edgeVec.push_back( e );
//
//	//	e = new TEdge;
//	//	e->n0 = t->n1;
//	//	e->n1 = t->n2;
//	//	edgeVec.push_back( e );
//
//	//	e = new TEdge;
//	//	e->n0 = t->n2;
//	//	e->n1 = t->n0;
//	//	edgeVec.push_back( e );
//	//}
//
//	////==== Find Edges That Dont Share Two Tris
//	//vector < int > perimFlags;
//	//perimFlags.resize( edgeVec.size() );
//	//for ( i = 0 ; i < (int)perimFlags.size() ; i++ )
//	//	perimFlags[i] = 1;
//
//	//for ( i = 0 ; i < (int)edgeVec.size() ; i++ )
//	//{
//	//	for ( j = i+1 ; j < (int)edgeVec.size() ; j++ )
//	//	{
//	//		if ( edgeVec[i]->n0 == edgeVec[j]->n0 && edgeVec[i]->n1 == edgeVec[j]->n1 || 
//	//			 edgeVec[i]->n1 == edgeVec[j]->n0 && edgeVec[i]->n0 == edgeVec[j]->n1 )
//	//		{
//	//			perimFlags[i] = 0;
//	//			perimFlags[j] = 0;
//	//		}
//	//	}
//	//}
//
//	//vector< TEdge* > perimEdges;
//	//for ( i = 0 ; i < (int)edgeVec.size() ; i++ )
//	//{
//	//	if ( perimFlags[i] )
//	//		perimEdges.push_back( edgeVec[i] );
//	//}
//
//	//if ( perimEdges.size() < 3 )
//	//	return;
//
//	//if ( finalFlag )
//	//{
//	//	for ( i = 0 ; i < (int)isectEdgeVec.size() ; i++ )
//	//	{
//	//		perimEdges.push_back( isectEdgeVec[i] );
//	//	}
//	//}
//
//	////==== Merge Nodes and Remove Small Edges ====//
//	//MergeNodesRemoveEdges( nodeVec, perimEdges, 0.001 );
//
//	//==== Convert To UW Points =====//
//	vec2d uw;
//	vec3d orig  = plnPnt[0];
//	vec3d axis0 = plnPnt[1] - plnPnt[0];	axis0.normalize();
//	vec3d axis1 = plnPnt[3] - plnPnt[0];	axis1.normalize();
//
//	for ( i = 0 ; i < (int)nodeVec.size() ; i++ )
//	{
//		uw = MapToPlane( nodeVec[i]->pnt, orig, axis0, axis1 );
//		nodeVec[i]->norm.set_x( uw.x() );			// Store in Norm for Now
//		nodeVec[i]->norm.set_y( uw.y() );
//	}
//
//	//==== Dump Into Triangle ====//
//	struct triangulateio in;
//	struct triangulateio out;
//
//	memset(&in,0,sizeof(in));		// Load Zeros
//	memset(&out,0,sizeof(out));
//
//	//==== PreAllocate Data For In/Out ====//
//	in.numberofpoints = nodeVec.size();
//	in.pointlist    = (REAL *) malloc(in.numberofpoints * 2 * sizeof(REAL));
//	out.pointlist   = NULL;
//
//	in.numberofsegments = perimEdges.size();
//	in.segmentlist  = (int *) malloc(in.numberofsegments * 2 * sizeof(int));
//	out.segmentlist  = NULL;
//	out.trianglelist  = NULL;
//
//    in.numberofpointattributes = 0;
//    in.pointattributelist = NULL;
//    in.pointmarkerlist = NULL;
//    in.numberofholes = 0;
//    in.numberoftriangles = 0;
//    in.numberofpointattributes = 0;
//    in.numberofedges = 0;
//    in.trianglelist = NULL;
//    in.trianglearealist = NULL;
//    in.edgelist = NULL;
//    in.edgemarkerlist = NULL;
//    in.segmentmarkerlist = NULL; 
//
//	for ( i = 0 ; i < (int)nodeVec.size() ; i++ )
//	{
//		nodeVec[i]->id = i;
//		in.pointlist[i*2]   = nodeVec[i]->norm.x();
//		in.pointlist[i*2+1] = nodeVec[i]->norm.y();
//	}
//	for ( i = 0 ; i < (int)perimEdges.size() ; i++ )
//	{
//		in.segmentlist[i*2] = perimEdges[i]->n0->id;
//		in.segmentlist[i*2+1] = perimEdges[i]->n1->id;
//	}
//
//	char cmd[256];
//	sprintf( cmd, "YYzpQa%0.3f", size );		
//	triangulate (cmd, &in, &out, (struct triangulateio *) NULL);
//
//	//==== Delete Edges ====//
//	for ( i = 0 ; i < (int)edgeVec.size() ; i++ )
//		delete edgeVec[i];
//
//	if ( finalFlag )		// Clean Up Mess
//	{
//		for ( i = 0 ; i < (int)isectEdgeVec.size() ; i++ )
//			delete isectEdgeVec[i];
//
//		for ( i = 0 ; i < (int)isectNodeVec.size() ; i++ )
//			delete isectNodeVec[i];
//
//		for ( i = 0 ; i < (int)finalMesh->tVec.size() ; i++ )
//			finalMesh->tVec[i]->iSectEdgeVec.clear();	}
//
//	if ( finalMesh )
//		delete finalMesh;
//	finalMesh = new TMesh();
//	nodeVec.clear();
//	triVec.clear();
//
//	//==== Load Nodes ====//
//	for ( i = 0 ; i < out.numberofpoints ; i++ )
//	{
//		TNode* node = new TNode();
//		vec2d uw( out.pointlist[i*2], out.pointlist[i*2+1] );
//		node->pnt = MapFromPlane( uw, orig, axis0, axis1 );
//		nodeVec.push_back( node );
//	}
//
//	//==== Load Tris ====//
//	vec3d norm;
//	for ( i = 0 ; i < out.numberoftriangles ; i++ )
//	{
//		TNode* n0 = nodeVec[ out.trianglelist[i*3] ];
//		TNode* n1 = nodeVec[ out.trianglelist[i*3+1] ];
//		TNode* n2 = nodeVec[ out.trianglelist[i*3+2] ];
//		finalMesh->addTri( n0, n1, n2, norm );
//	}
//
//	//==== Free Local Memory ====//
//	if ( in.pointlist )				free( in.pointlist );
//	if ( in.segmentlist )			free( in.segmentlist );
//
//	if ( out.pointlist )			free( out.pointlist );
//	if ( out.pointmarkerlist )		free( out.pointmarkerlist );
//	if ( out.trianglelist )			free( out.trianglelist );
//	if ( out.segmentlist )			free( out.segmentlist );
//	if ( out.segmentmarkerlist )	free( out.segmentmarkerlist );

//	FILE* fp = fopen("test.dat", "w");
//
//	fprintf( fp, "X Y \n");
//
//	for ( i = 0 ; i < out.numberoftriangles ; i++ )
//	{
//		for ( j = 0 ; j < 4 ; j++ )		/// Close curve
//		{
//			int tind = out.trianglelist[i*3 + j%3];
//
//			double x = out.pointlist[tind*2];
//			double y = out.pointlist[tind*2+1];
//
//			fprintf( fp, "%f %f\n", x, y );
//
//		}
//		fprintf( fp, "MOVE\n" );
//
////		fprintf( fp, "%d %d  %d\n", out.trianglelist[i*3], out.trianglelist[i*3+1], out.trianglelist[i*3+2] );
//	}
//
//
//	fclose( fp );
//
//	FILE* fp = fopen("test.dat", "w");
//	
//	double x, y;
//	for ( i = 0 ; i < (int)perimEdges.size() ; i++ )
//	{
////		fprintf(fp, "%d %d\n", perimEdges[i]->n0, perimEdges[i]->n1 );
//		x = perimEdges[i]->n0->norm.x(), 
//		y = perimEdges[i]->n0->norm.y(), 
//		fprintf( fp, "%f %f\n", x, y );
//
//		x = perimEdges[i]->n1->norm.x(), 
//		y = perimEdges[i]->n1->norm.y(), 
//		fprintf( fp, "%f %f\n", x, y );
//		fprintf( fp, "MOVE\n" );
//	}
//
//	fclose(fp);

//}

////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////

Rib::Rib() : ThreePntSlice()
{
	type = RIB;

	percentSpan = 0.5;
	SetPercentSpan( percentSpan );
}

Rib::~Rib()
{
}

void Rib::Copy( Part* fromPart )
{
	ThreePntSlice::Copy(fromPart);
	percentSpan = ((Rib*)fromPart)->GetPercentSpan();
}

void Rib::WriteParms( xmlNodePtr root )
{
	xmlAddStringNode( root, "Name", GetName() );
	xmlAddIntNode( root, "Type", type );

	xmlAddDoubleNode( root, "PercentSpan", percentSpan );
}

void Rib::ReadParms( xmlNodePtr root )
{
	SetName( xmlFindString( root, "Name", GetName() ) );
	
	SetPercentSpan( xmlFindDouble( root, "PercentSpan", percentSpan ) );
}

void Rib::SetPercentSpan( double p )
{
	percentSpan = p;
	uwPnt[0] = vec2d( percentSpan, 0.1 );
	uwPnt[1] = vec2d( percentSpan, 0.5 );
	uwPnt[2] = vec2d( percentSpan, 0.9 );

	staleFlag = true;

}

////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////

Spar::Spar() : ThreePntSlice()
{
	type = SPAR;

	percentChord = 0.5;
	SetPercentChord( percentChord );
}

Spar::~Spar()
{
}

void Spar::Copy( Part* fromPart )
{
	ThreePntSlice::Copy(fromPart);
	percentChord = ((Spar*)fromPart)->GetPercentChord();
}

void Spar::WriteParms( xmlNodePtr root )
{
	xmlAddStringNode( root, "Name", GetName() );
	xmlAddIntNode( root, "Type", type );

	xmlAddDoubleNode( root, "PercentChord", percentChord );
}

void Spar::ReadParms( xmlNodePtr root )
{
	SetName( xmlFindString( root, "Name", GetName() ) );

	SetPercentChord( xmlFindDouble( root, "PercentChord", percentChord ) );
}

void Spar::SetPercentChord( double p )
{
	percentChord = p;
	uwPnt[0] = vec2d( 0.0, percentChord );
	uwPnt[1] = vec2d( 1.0, percentChord );
	uwPnt[2] = vec2d( 0.5, percentChord );

	staleFlag = true;

}



