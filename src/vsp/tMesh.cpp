//******************************************************************************
//    
//    External Geometry Class
//  
// 
//   J.R. Gloudemans - 11/12/03
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

#include "tMesh.h"
#include "bbox.h"

#include "tritri.h"
#include "defines.h"

#include "triangle.h"
#include "util.h"
#include "geom.h"

#include <map>


//===============================================//
//					TNode
//===============================================//
TNode::TNode()
{
	id = -1;
//	mapNode = 0;
	isectFlag = 0;
}

TNode::~TNode()
{


}


//=======================================================================//
//=======================================================================//
//=======================================================================//
TetraMassProp::TetraMassProp(int id, double denIn, vec3d& p0, vec3d& p1, vec3d& p2, vec3d& p3)
{
	compId = id;
	pointMassFlag = false;
	den = denIn;

	v0 = p0;
	v1 = p1 - p0;
	v2 = p2 - p0;
	v3 = p3 - p0;

	cg = v1 + v2 + v3;
	cg = (cg*0.25) + p0;

	vol  = tetra_volume( v1, v2, v3 );
	mass = den*fabs(vol);

	double Ix = mass/10.0*(v1.x()*v1.x() + v2.x()*v2.x() + v3.x()*v3.x() +
						   v1.x()*v2.x() + v1.x()*v3.x() + v2.x()*v3.x());

	double Iy = mass/10.0*(v1.y()*v1.y() + v2.y()*v2.y() + v3.y()*v3.y() +
						   v1.y()*v2.y() + v1.y()*v3.y() + v2.y()*v3.y());

	double Iz = mass/10.0*(v1.z()*v1.z() + v2.z()*v2.z() + v3.z()*v3.z() +
						   v1.z()*v2.z() + v1.z()*v3.z() + v2.z()*v3.z());

	Ixx = Iy + Iz;
	Iyy = Ix + Iz;
	Izz = Ix + Iy;

	Ixy = mass/20.0*( 2.0*( v1.x()*v1.y() + v2.x()*v2.y() + v3.x()*v3.y() ) + 
		  v1.x()*v2.y() + v2.x()*v1.y() + v1.x()*v3.y() + v3.x()*v1.y() + v2.x()*v3.y() + v3.x()*v2.y() );

	Iyz = mass/20.0*( 2.0*( v1.y()*v1.z() + v2.y()*v2.z() + v3.y()*v3.z() ) + 
		  v1.y()*v2.z() + v2.y()*v1.z() + v1.y()*v3.z() + v3.y()*v1.z() + v2.y()*v3.z() + v3.y()*v2.z() );

	Ixz = mass/20.0*( 2.0*( v1.x()*v1.z() + v2.x()*v2.z() + v3.x()*v3.z() ) + 
		  v1.x()*v2.z() + v2.x()*v1.z() + v1.x()*v3.z() + v3.x()*v1.z() + v2.x()*v3.z() + v3.x()*v2.z() );

}


void TetraMassProp::SetPointMass(double massIn, vec3d& pos)
{
	compId = 0;
	pointMassFlag = true;
	den = 0.0;
	cg = pos;
	vol  = 0.0;
	mass = massIn;

	Ixx = 0.0;
	Iyy = 0.0;
	Izz = 0.0;

	Ixy = 0.0;
	Iyz = 0.0;
	Ixz = 0.0;

}








//=======================================================================//
//=======================================================================//
//=======================================================================//
TriShellMassProp::TriShellMassProp(int id, double mass_area_in, vec3d& p0, vec3d& p1, vec3d& p2)
{
	compId = id;

	mass_area = mass_area_in;

	cg = (p0 + p1 + p2)*(1.0/3.0);

	v0 = p0 - cg;
	v1 = p1 - cg;
	v2 = p2 - cg;
	
	tri_area = area(v0, v1, v2);

	mass = tri_area*mass_area;

	double Ix = mass/10.0*(v0.x()*v0.x() + v1.x()*v1.x() + v2.x()*v2.x() +
						   v0.x()*v1.x() + v0.x()*v2.x() + v1.x()*v2.x());

	double Iy = mass/10.0*(v0.y()*v0.y() + v1.y()*v1.y() + v2.y()*v2.y() +
						   v0.y()*v1.y() + v0.y()*v2.y() + v1.y()*v2.y());

	double Iz = mass/10.0*(v0.z()*v0.z() + v1.z()*v1.z() + v2.z()*v2.z() +
						   v0.z()*v1.z() + v0.z()*v2.z() + v1.z()*v2.z());

	Ixx = Iy + Iz;
	Iyy = Ix + Iz;
	Izz = Ix + Iy;

	Ixy = mass/20.0*( 2.0*( v0.x()*v0.y() + v1.x()*v1.y() + v2.x()*v2.y() ) + 
		  v0.x()*v1.y() + v1.x()*v0.y() + v0.x()*v2.y() + v2.x()*v0.y() + v1.x()*v2.y() + v2.x()*v1.y() );

	Iyz = mass/20.0*( 2.0*( v0.y()*v0.z() + v1.y()*v1.z() + v2.y()*v2.z() ) + 
		  v0.y()*v1.z() + v1.y()*v0.z() + v0.y()*v2.z() + v2.y()*v0.z() + v1.y()*v2.z() + v2.y()*v1.z() );

	Ixz = mass/20.0*( 2.0*( v0.x()*v0.z() + v1.x()*v1.z() + v2.x()*v2.z() ) + 
		  v0.x()*v1.z() + v1.x()*v0.z() + v0.x()*v2.z() + v2.x()*v0.z() + v1.x()*v2.z() + v2.x()*v1.z() );


}

//===============================================//
//===============================================//
//===============================================//
//===============================================//
//					TMesh
//===============================================//
//===============================================//
//===============================================//
//===============================================//
TMesh::TMesh()
{
	deleteMeFlag = false;
	theoArea = wetArea = 0.0;
	theoVol	 = wetVol = 0.0;
	reflected_flag = false;
	halfBoxFlag = false;
}

TMesh::~TMesh()
{

	int i;

	for ( i = 0 ; i < (int)tVec.size() ; i++ )
		delete tVec[i];
	for ( i = 0 ; i < (int)nVec.size() ; i++ )
		delete nVec[i];
	for ( i = 0 ; i < (int)eVec.size() ; i++ )
		delete eVec[i];

}

void TMesh::copy( TMesh* m )
{
	ptr_id     = m->ptr_id;
	name_str   = m->name_str;
	materialID = m->materialID;
	color      = m->color;

	theoArea   = m->theoArea;
	wetArea    = m->wetArea;
	theoVol    = m->theoVol;
	guessVol   = m->guessVol;
	wetVol     = m->wetVol;

	massPrior = m->massPrior;
	density   = m->density;
	shellFlag = m->shellFlag;
	shellMassArea = m->shellMassArea;

	drag_factors = m->drag_factors;

	tVec.clear();
	nVec.clear();

	for ( int i = 0 ; i < (int)m->tVec.size() ; i++ )
	{
		TTri* tri = new TTri();
		tri->n0   = new TNode();
		tri->n1   = new TNode();
		tri->n2   = new TNode();

		tri->norm    = m->tVec[i]->norm;
		tri->n0->pnt = m->tVec[i]->n0->pnt;
		tri->n1->pnt = m->tVec[i]->n1->pnt;
		tri->n2->pnt = m->tVec[i]->n2->pnt;

		tVec.push_back(tri);
		nVec.push_back(tri->n0);
		nVec.push_back(tri->n1);
		nVec.push_back(tri->n2);
	}
}

void TMesh::loadGeomAttributes( Geom* geomPtr )
{
	color		  = geomPtr->getColor();
	materialID    = geomPtr->getMaterialID();
	ptr_id        = geomPtr->getPtrID();
	if ( reflected_flag )
		ptr_id = -ptr_id;

	massPrior     = geomPtr->getMassPrior();
	density       = geomPtr->density.get();
	shellMassArea = geomPtr->shellMassArea.get();
	shellFlag     = geomPtr->getShellFlag();

	//==== Check for Alternate Output Name ====//
	name_str   = geomPtr->getName();

	geomPtr->LoadDragFactors( drag_factors );

}

int TMesh::removeDegenerate()
{
	vector< TTri* > degTVec;

	int num_deleted = 0;
	for ( int t = 0 ; t < (int)tVec.size() ; t++ )
	{
		TTri* tri = tVec[t];
		int addTriFlag = 1;

		if ( dist( tri->n0->pnt, tri->n1->pnt ) < 0.000001 )
			addTriFlag = 0;
		else if ( dist( tri->n0->pnt, tri->n2->pnt ) < 0.000001 )
			addTriFlag = 0;
		else if ( dist( tri->n1->pnt, tri->n2->pnt ) < 0.000001 )
			addTriFlag = 0;

		if ( addTriFlag )
			degTVec.push_back( tri );
		else
		{
			num_deleted++;
			delete tri;
		}
	}

	tVec = degTVec;

	return num_deleted;

}

void TMesh::checkIfClosed()
{
	int n, s, t;

	matchNodes();

	//==== Clear Refs to Tris ====//
	for ( n = 0 ; n < (int)nVec.size() ; n++ )
	{
		nVec[n]->triVec.clear();
	}
	
	//==== Check If All Tris Have 3 Edges ====//
	for ( t = 0 ; t < (int)tVec.size() ; t++ )
	{
		tVec[t]->n0->triVec.push_back( tVec[t] );
		tVec[t]->n1->triVec.push_back( tVec[t] );
		tVec[t]->n2->triVec.push_back( tVec[t] );
		tVec[t]->e0 = 0;
		tVec[t]->e1 = 0;
		tVec[t]->e2 = 0;
	}

	//==== Create Edges For Adjacent Tris ====//
	for ( n = 0 ; n < (int)nVec.size() ; n++ )
	{
		TNode* n0 = nVec[n];
		for ( t = 0 ; t < (int)n0->triVec.size() ; t++ )
		{
			for ( s = t+1 ; s < (int)n0->triVec.size() ; s++ )
			{
				findEdge( n0, n0->triVec[t], n0->triVec[s] );
			}
		}
	}

	//==== Check If All Tris Have 3 Edges ====//
	nonClosedTriVec.clear();
	vector< TTri* > ivTriVec;
	for ( t = 0 ; t < (int)tVec.size() ; t++ )
	{
		if ( !tVec[t]->e0 || !tVec[t]->e1 || !tVec[t]->e2 )
		{
			tVec[t]->invalidFlag = 1;
			nonClosedTriVec.push_back( tVec[t] );
		}	
	}
}

void TMesh::mergeNonClosed( TMesh* tm )
{
	if ( deleteMeFlag || tm->deleteMeFlag )		// Already Deleted
		return;

	if ( nonClosedTriVec.size() == 0 )			// No Open Tris
		return;

	//if ( nonClosedTriVec.size() != tm->nonClosedTriVec.size() )	// Num of Open Tris does not match
	//	return;

	bool match_flag;
	for ( int i = 0 ; i < (int)nonClosedTriVec.size() ; i++ )
	{
		TTri* t0 = nonClosedTriVec[i];
		match_flag = false;
		for ( int j = 0 ; j < (int)tm->nonClosedTriVec.size() ; j++ )
		{
			TTri* t1 = tm->nonClosedTriVec[j];
			if ( t0->shareEdge( t1 ) )
			{
				match_flag = true;
				break;
			}
		}
		if ( match_flag == false )
		{
			break;
		}
	}

	if ( match_flag )
	{
		for ( int t = 0 ; t < (int)tm->tVec.size() ; t++ )
		{
			TTri* tri = tm->tVec[t];
			addTri( tri->n0->pnt, tri->n1->pnt, tri->n2->pnt, tri->norm );
		}
		for ( int i = 0 ; i < (int)nonClosedTriVec.size() ; i++ )
		{
			nonClosedTriVec[i]->invalidFlag = 0;
		}
		nonClosedTriVec.clear();

		checkIfClosed();				// Recheck For NonClosed Tris

		tm->deleteMeFlag = true;
	}
}

void TMesh::intersect( TMesh* tm )
{
	tBox.intersect( &tm->tBox );
}

void TMesh::split(int meshFlag)
{
	int t;
	for ( t = 0 ; t < (int)tVec.size() ; t++ )
	{
		tVec[t]->splitTri(meshFlag);
	}
}


void TMesh::deterIntExt( vector< TMesh* >& meshVec )
{
	for ( int t = 0 ; t < (int)tVec.size() ; t++ )
	{
		TTri* tri = tVec[t];
		
		//==== Do Interior Tris ====//
		if ( tri->splitVec.size() )
		{
			tri->interiorFlag = 1;
			for ( int s = 0 ; s < (int)tri->splitVec.size() ; s++ )
			{
				deterIntExtTri( tri->splitVec[s], meshVec );
			}
		}
		else
		{
			deterIntExtTri( tri, meshVec );
		}
	}
}

void TMesh::deterIntExtTri( TTri* tri, vector< TMesh* >& meshVec )
{
	vec3d orig = (tri->n0->pnt + tri->n1->pnt)*0.5;
	orig = (orig + tri->n2->pnt)*0.5;
	tri->interiorFlag = 0;

 	for ( int m = 0 ; m < (int)meshVec.size() ; m++ )
	{
		if ( meshVec[m] != this )
		{	
			vector<double > tParmVec;
			meshVec[m]->tBox.numCrossXRay( orig, tParmVec );
			if ( tParmVec.size()%2 )
			{
				tri->interiorFlag = 1;
				return ;
			}
		}
	}
}

int TMesh::deterIntExtPnt( const vec3d& pnt, vector< TMesh* >& meshVec, TMesh* ignoreMesh )	// 1 Interior 0 Exterior
{
	return 0;
}

void TMesh::massDeterIntExt( vector< TMesh* >& meshVec )
{
	for ( int t = 0 ; t < (int)tVec.size() ; t++ )
	{
		TTri* tri = tVec[t];
		
		//==== Do Interior Tris ====//
		if ( tri->splitVec.size() )
		{
			tri->interiorFlag = 1;
			for ( int s = 0 ; s < (int)tri->splitVec.size() ; s++ )
			{
				massDeterIntExtTri( tri->splitVec[s], meshVec );
			}
		}
		else
		{
			massDeterIntExtTri( tri, meshVec );
		}
	}
}


void TMesh::massDeterIntExtTri( TTri* tri, vector< TMesh* >& meshVec )
{
	vec3d orig = (tri->n0->pnt + tri->n1->pnt)*0.5;
	orig = (orig + tri->n2->pnt)*0.5;
	tri->interiorFlag = 1;
	int prior = -1;

 	for ( int m = 0 ; m < (int)meshVec.size() ; m++ )
	{
		if ( meshVec[m] != this )
		{	
			vector<double > tParmVec;
			meshVec[m]->tBox.numCrossXRay( orig, tParmVec );
			if ( tParmVec.size()%2 )
			{
				if ( meshVec[m]->massPrior > prior )
				{
					tri->interiorFlag = 0;
					tri->id = meshVec[m]->ptr_id;
					tri->mass = meshVec[m]->density;
					prior = meshVec[m]->massPrior;
				}
			}
		}
	}
}

double TMesh::computeTheoArea()
{
	theoArea = 0;
	for ( int t = 0 ; t < (int)tVec.size() ; t++ )
	{
		theoArea += tVec[t]->computeArea(); 
	}
	return theoArea;
}

double TMesh::computeWetArea()
{
	wetArea = 0;

	for ( int t = 0 ; t < (int)tVec.size() ; t++ )
	{
		TTri* tri = tVec[t];

		//==== Do Interior Tris ====//
		if ( tri->splitVec.size() )
		{
			for ( int s = 0 ; s < (int)tri->splitVec.size() ; s++ )
			{
				if ( !tri->splitVec[s]->interiorFlag )
				{
					wetArea += tri->splitVec[s]->computeArea();
				}
			}
		}
		else if ( !tri->interiorFlag )
		{
			wetArea += tri->computeArea();
		}
	}
	return wetArea;
}

double TMesh::computeAwaveArea()
{
	wetArea = 0;

	for ( int t = 0 ; t < (int)tVec.size() ; t++ )
	{
		TTri* tri = tVec[t];

		//==== Do Interior Tris ====//
		if ( tri->splitVec.size() )
		{
			for ( int s = 0 ; s < (int)tri->splitVec.size() ; s++ )
			{
				if ( !tri->splitVec[s]->interiorFlag )
				{
					wetArea += tri->splitVec[s]->computeAwArea();
				}
			}
		}
		else if ( !tri->interiorFlag )
		{
			wetArea += tri->computeAwArea();
		}
	}
	return wetArea;
}

double TMesh::computeTheoVol()
{
	theoVol = 0.0;
	for ( int t = 0 ; t < (int)tVec.size() ; t++ )
	{
		TTri* tri = tVec[t];

		theoVol += tetra_volume( tri->n0->pnt, tri->n1->pnt, tri->n2->pnt );
	}

	return theoVol;

}
double TMesh::computeTrimVol()
{
	double trimVol = 0;

	for ( int t = 0 ; t < (int)tVec.size() ; t++ )
	{
		TTri* tri = tVec[t];

		//==== Do Interior Tris ====//
		if ( tri->splitVec.size() )
		{
			for ( int s = 0 ; s < (int)tri->splitVec.size() ; s++ )
			{
				if ( !tri->splitVec[s]->interiorFlag )
				{
					trimVol += tetra_volume( tri->splitVec[s]->n0->pnt, tri->splitVec[s]->n1->pnt, tri->splitVec[s]->n2->pnt );
				}
			}
		}
		else if ( !tri->interiorFlag )
		{
			trimVol += tetra_volume( tri->n0->pnt, tri->n1->pnt, tri->n2->pnt );
		}
	}
	return trimVol;
}


void TMesh::draw()
{


}



//==== Draw Wire Frame =====//
void TMesh::draw_wire( )
{
	int i, j;
	vec3d t1, t2, t3;
	vertVec.clear();

	double color[4];
	glGetDoublev(GL_CURRENT_COLOR, color);

	for ( i = 0 ; i < (int)tVec.size() ; i++ )
	{
		TTri* vec = tVec[i];
		if ( tVec[i] && tVec[i]->splitVec.size() ) // HACK fixed potential access violation
		{
			for ( j = 0 ; j < (int)tVec[i]->splitVec.size() ; j++ )
			{
				if ( tVec[i]->splitVec[j]->interiorFlag == 0 )
				{
					t1 = tVec[i]->splitVec[j]->n0->pnt;
					t2 = tVec[i]->splitVec[j]->n1->pnt;
					t3 = tVec[i]->splitVec[j]->n2->pnt;
					vertVec.push_back(t1);
					vertVec.push_back(t2);
					vertVec.push_back(t3);

					glBegin( GL_LINE_LOOP );
						glVertex3dv( t1.data() );
						glVertex3dv( t2.data() );
						glVertex3dv( t3.data() );
					glEnd();
				}
			}
		}
		else
		{
			if ( tVec[i]->invalidFlag )
			{
				glColor3d(1.0, 0.0, 0.0);
				glLineWidth( 3.0 );
			}
			else
			{
				glColor3dv(color);
				glLineWidth( 1.0 );
			}

			if ( tVec[i]->interiorFlag == 0 )
			{
				t1 = tVec[i]->n0->pnt;
				t2 = tVec[i]->n1->pnt;
				t3 = tVec[i]->n2->pnt;
				vertVec.push_back(t1);
				vertVec.push_back(t2);
				vertVec.push_back(t3);
				glBegin( GL_LINE_LOOP );
					glVertex3dv( t1.data() );
					glVertex3dv( t2.data() );
					glVertex3dv( t3.data() );
				glEnd();
			}

		}
	}

//Draw Normals 
	for ( int n = 0 ; n < (int)nVec.size() ; n++ )
	{
		glColor3d(1.0, 0.0, 0.0);
		glBegin( GL_LINES );
		if ( nVec[n]->isectFlag )
		{
			glVertex3dv( nVec[n]->pnt.data() );
			vec3d offnorm = nVec[n]->pnt + nVec[n]->norm;
			glVertex3dv( offnorm.data() );
		}
		glEnd();
	}
//Draw Isect Lines
	glColor3d(1.0, 0.0, 1.0);
	glBegin( GL_LINES );
	for ( i = 0 ; i < (int)isectPairs.size() ; i+=2 )
	{
		glVertex3dv( isectPairs[i].data() );
		glVertex3dv( isectPairs[i+1].data() );
	}
	glEnd();


	glColor3dv(color);		// Restore Color

/*
	//==== Draw Intersection Curve ====//
	glColor3ub(255, 255, 0);
	for ( i = 0 ; i < tVec.size() ; i++ )
	{
		for ( j = 0 ; j < tVec[i]->iSectEdgeVec.size() ; j++ )
		{
			glBegin( GL_LINES  );
				glVertex3dv( tVec[i]->iSectEdgeVec[j]->n0->pnt.data() );
				glVertex3dv( tVec[i]->iSectEdgeVec[j]->n1->pnt.data() );
			glEnd();			
		}
	}
*/

}

void TMesh::draw_shaded()
{
	int i,j;
	vec3d t1, t2, t3;
	vertVec.clear();

	for ( i = 0 ; i < (int)tVec.size() ; i++ )
	{
		if ( tVec[i]->splitVec.size() )
		{
			for ( j = 0 ; j < (int)tVec[i]->splitVec.size() ; j++ )
			{
				if ( tVec[i]->splitVec[j]->interiorFlag == 0 )
				{
					t1 = tVec[i]->splitVec[j]->n0->pnt;
					t2 = tVec[i]->splitVec[j]->n1->pnt;
					t3 = tVec[i]->splitVec[j]->n2->pnt;
					vertVec.push_back(t1);
					vertVec.push_back(t2);
					vertVec.push_back(t3);
					glBegin( GL_POLYGON );
						glNormal3dv(tVec[i]->norm.data());
						glVertex3dv( t1.data() );
						glVertex3dv( t2.data() );
						glVertex3dv( t3.data() );
					glEnd();
				}
			}
		}
		else
		{
			if ( tVec[i]->interiorFlag == 0 )
			{
				t1 = tVec[i]->n0->pnt;
				t2 = tVec[i]->n1->pnt;
				t3 = tVec[i]->n2->pnt;
				vertVec.push_back(t1);
				vertVec.push_back(t2);
				vertVec.push_back(t3);
				glBegin( GL_POLYGON );
					glNormal3dv(tVec[i]->norm.data());
					glVertex3dv( t1.data() );
					glVertex3dv( t2.data() );
					glVertex3dv( t3.data() );
				glEnd();
			}

		}
	}
}


void TMesh::addTri( const vec3d & v0, const vec3d & v1, const vec3d & v2, const vec3d & norm )
{
	TTri* ttri = new TTri();
	ttri->norm = norm;

	ttri->n0 = new TNode();
	ttri->n1 = new TNode();
	ttri->n2 = new TNode();

	ttri->n0->pnt = v0;
	ttri->n1->pnt = v1;
	ttri->n2->pnt = v2;

	tVec.push_back( ttri );
	nVec.push_back( ttri->n0 );
	nVec.push_back( ttri->n1 );
	nVec.push_back( ttri->n2 );
}

void TMesh::addTri( TNode* node0, TNode* node1, TNode* node2, const vec3d & norm )
{
	TTri* ttri = new TTri();
	ttri->norm = norm;

	ttri->n0 = new TNode();
	ttri->n1 = new TNode();
	ttri->n2 = new TNode();

	ttri->n0->pnt = node0->pnt;
	ttri->n1->pnt = node1->pnt;
	ttri->n2->pnt = node2->pnt;

	ttri->n0->isectFlag = node0->isectFlag;
	ttri->n1->isectFlag = node1->isectFlag;
	ttri->n2->isectFlag = node2->isectFlag;

	tVec.push_back( ttri );
	nVec.push_back( ttri->n0 );
	nVec.push_back( ttri->n1 );
	nVec.push_back( ttri->n2 );



}


void TMesh::loadBndBox()
{
	for ( int i = 0 ; i < (int)tVec.size() ; i++ )
	{
		tBox.addTri( tVec[i] );
	}

	tBox.splitBox();
}

//==== Write STL Tris =====//
void TMesh::write_stl_tris( int sym_code_in, float mat[4][4], float refl_mat[4][4], FILE* file_id)
{
	int t, s;
	vec3d norm;
	vec3d v0, v1, v2;
	vec3d d30, d21;

	for ( t = 0 ; t < (int)tVec.size() ; t++ )
	{
		TTri* tri = tVec[t];

		if ( tri->splitVec.size() )
		{
			for ( s = 0 ; s < (int)tri->splitVec.size() ; s++ )
			{
				if ( !tri->splitVec[s]->interiorFlag )
				{
					v0 = tri->splitVec[s]->n0->pnt.transform(mat);
					v1 = tri->splitVec[s]->n1->pnt.transform(mat);
					v2 = tri->splitVec[s]->n2->pnt.transform(mat);

					d21 = v2 - v1;

					if ( d21.mag() > 0.000001 )
					{
						norm = cross(d21, v0 - v1); 
						norm.normalize();

						fprintf(file_id, " facet normal  %2.10le %2.10le %2.10le\n",  norm.x(), norm.y(), norm.z() );
						fprintf(file_id, "   outer loop\n");
						fprintf(file_id, "     vertex %2.10le %2.10le %2.10le\n", v0.x(), v0.y(), v0.z());
						fprintf(file_id, "     vertex %2.10le %2.10le %2.10le\n", v1.x(), v1.y(), v1.z());
						fprintf(file_id, "     vertex %2.10le %2.10le %2.10le\n", v2.x(), v2.y(), v2.z());
						fprintf(file_id, "   endloop\n");
						fprintf(file_id, " endfacet\n");
					}
				}
			}
		}
		else
		{
			if ( !tri->interiorFlag )
			{
				v0 = tri->n0->pnt.transform(mat);
				v1 = tri->n1->pnt.transform(mat);
				v2 = tri->n2->pnt.transform(mat);

				d21 = v2 - v1;

				if ( d21.mag() > 0.000001 )
				{
					norm = cross(d21, v0 - v1); 
					norm.normalize();

					fprintf(file_id, " facet normal  %2.10le %2.10le %2.10le\n",  norm.x(), norm.y(), norm.z() );
					fprintf(file_id, "   outer loop\n");
					fprintf(file_id, "     vertex %2.10le %2.10le %2.10le\n", v0.x(), v0.y(), v0.z());
					fprintf(file_id, "     vertex %2.10le %2.10le %2.10le\n", v1.x(), v1.y(), v1.z());
					fprintf(file_id, "     vertex %2.10le %2.10le %2.10le\n", v2.x(), v2.y(), v2.z());
					fprintf(file_id, "   endloop\n");
					fprintf(file_id, " endfacet\n");
				}
			}
		}
	}

	if (sym_code_in == NO_SYM) return;

	for ( t = 0 ; t < (int)tVec.size() ; t++ )
	{
		TTri* tri = tVec[t];

		if ( tri->splitVec.size() )
		{
			for ( s = 0 ; s < (int)tri->splitVec.size() ; s++ )
			{
				if ( !tri->splitVec[s]->interiorFlag )
				{
					v0 = tri->splitVec[s]->n0->pnt.transform(refl_mat);
					v1 = tri->splitVec[s]->n1->pnt.transform(refl_mat);
					v2 = tri->splitVec[s]->n2->pnt.transform(refl_mat);

					d21 = v2 - v1;

					if ( d21.mag() > 0.000001 )
					{
						norm = cross(d21, v0 - v1); 
						norm.normalize();

						fprintf(file_id, " facet normal  %lf %lf %lf\n",  norm.x(), norm.y(), norm.z() );
						fprintf(file_id, "   outer loop\n");
						fprintf(file_id, "     vertex %lf %lf %lf\n", v0.x(), v0.y(), v0.z());
						fprintf(file_id, "     vertex %lf %lf %lf\n", v1.x(), v1.y(), v1.z());
						fprintf(file_id, "     vertex %lf %lf %lf\n", v2.x(), v2.y(), v2.z());
						fprintf(file_id, "   endloop\n");
						fprintf(file_id, " endfacet\n");
					}
				}
			}
		}
		else
		{
			if ( !tri->interiorFlag )
			{
				v0 = tri->n0->pnt.transform(refl_mat);
				v1 = tri->n1->pnt.transform(refl_mat);
				v2 = tri->n2->pnt.transform(refl_mat);

				d21 = v2 - v1;

				if ( d21.mag() > 0.000001 )
				{
					norm = cross(d21, v0 - v1); 
					norm.normalize();

					fprintf(file_id, " facet normal  %lf %lf %lf\n",  norm.x(), norm.y(), norm.z() );
					fprintf(file_id, "   outer loop\n");
					fprintf(file_id, "     vertex %lf %lf %lf\n", v0.x(), v0.y(), v0.z());
					fprintf(file_id, "     vertex %lf %lf %lf\n", v1.x(), v1.y(), v1.z());
					fprintf(file_id, "     vertex %lf %lf %lf\n", v2.x(), v2.y(), v2.z());
					fprintf(file_id, "   endloop\n");
					fprintf(file_id, " endfacet\n");
				}
			}
		}
  }
}

vec3d TMesh::getVertex(int index)
{
	if (index < 0) index = 0;
	if (index >= (int)vertVec.size()) index = (int)vertVec.size()-1;

	return vertVec[index];
}

int TMesh::numVerts()
{
	return vertVec.size();
}


//===============================================//
//===============================================//
//===============================================//
//===============================================//
//					TTri
//===============================================//
//===============================================//
//===============================================//
//===============================================//

TTri::TTri()			
{  
static int cnt = 0;
cnt++;
//printf("Tri Construct Cnt = %d \n", cnt);
	e0 = e1 = e2 = 0;
	n0 = n1 = n2 = 0; 
	interiorFlag = 0; 
	invalidFlag  = 0;
	mass = 0.0;
}

TTri::~TTri()
{
static int cnt = 0;
cnt++;
//printf("Tri Destruct Cnt = %d \n", cnt);
	int i;

	//==== Delete Edges ====//
	for ( i = 0 ; i < (int)eVec.size() ; i++ )
	{
		delete eVec[i];
	}

	//==== Delete Nodes - Not First 3 ====//
	for ( i = 3 ; i < (int)nVec.size() ; i++ )
	{
		delete nVec[i];
	}

	//==== Delete Sub Tris ====//
	for ( i = 0 ; i < (int)splitVec.size() ; i++ )
	{
		delete splitVec[i];
	}

	for ( i = 0 ; i < (int)iSectEdgeVec.size() ; i++ )
	{
		delete iSectEdgeVec[i]->n0;
		delete iSectEdgeVec[i]->n1;
		delete iSectEdgeVec[i];
	}

}

vec3d TTri::compNorm()
{
	vec3d p10 = n1->pnt - n0->pnt;
	vec3d p20 = n2->pnt - n0->pnt;
	vec3d cnorm = cross( p10, p20 );
	cnorm.normalize();
	norm = cnorm;
	return cnorm;
}

bool TTri::shareEdge( TTri* t )
{
	double tol = 0.0000001;

	if ( matchEdge( n0, n1, t->n0, t->n1, tol ) ) return true;
	if ( matchEdge( n0, n1, t->n1, t->n2, tol ) ) return true;
	if ( matchEdge( n0, n1, t->n0, t->n2, tol ) ) return true;
	if ( matchEdge( n1, n2, t->n0, t->n1, tol ) ) return true;
	if ( matchEdge( n1, n2, t->n1, t->n2, tol ) ) return true;
	if ( matchEdge( n1, n2, t->n0, t->n2, tol ) ) return true;
	if ( matchEdge( n0, n2, t->n0, t->n1, tol ) ) return true;
	if ( matchEdge( n0, n2, t->n1, t->n2, tol ) ) return true;
	if ( matchEdge( n0, n2, t->n0, t->n2, tol ) ) return true;

	return false;
}

bool TTri::matchEdge( TNode* n0, TNode* n1, TNode* nA, TNode* nB, double tol )
{
	if ( dist_squared( n0->pnt, nA->pnt) < tol && dist_squared( n1->pnt, nB->pnt) < tol )
	{
		n0->pnt = (n0->pnt + nA->pnt)*0.5;
		nA->pnt = n0->pnt;
		n1->pnt = (n1->pnt + nB->pnt)*0.5;
		nB->pnt = n1->pnt;
		return true;
	}
	if ( dist_squared( n0->pnt, nB->pnt) < tol && dist_squared( n1->pnt, nA->pnt) < tol )
	{
		n0->pnt = (n0->pnt + nB->pnt)*0.5;
		nB->pnt = n0->pnt;
		n1->pnt = (n1->pnt + nA->pnt)*0.5;
		nA->pnt = n1->pnt;
		return true;
	}

	return false;
}

//==== Load Nodes From Edges =====//
void TTri::loadNodesFromEdges()
{
	if ( !e0 || !e1 || !e2 )
		return;

//jrg TODO: Preserve Normal Direction 
	n0 = e0->n0;
	n1 = e0->n1;
	if ( e1->n0 != n0 && e1->n0 != n1 )
		n2 = e1->n0;
	else
		n2 = e1->n1;

	if ( n0 == n1 || n0 == n2 || n1 == n2 )
		printf("ERROR loadNodesFromEdges\n");
}


//==== Split A Triangle Along Edges in ISectEdges Vec =====//
void TTri::splitTri(int meshFlag)
{
	int i,j;
	double onEdgeTol = 0.00001;
	double uvMinTol  = 0.001;
	double uvMaxTol  = 0.999;

	//==== No Need To Split ====//
	if ( iSectEdgeVec.size() == 0 )
		return;

	//==== Delete Duplicate Edges ====//
	vector< TEdge* > noDupVec;
	noDupVec.push_back( iSectEdgeVec[0] );
	for ( i = 1 ; i < (int)iSectEdgeVec.size() ; i++ )
	{
		int dupFlag = 0;
		for ( j = 0 ; j < (int)noDupVec.size() ; j++ )
		{
			if ( dupEdge( iSectEdgeVec[i], noDupVec[j], onEdgeTol ) )
				dupFlag = 1;
		}

		if ( !dupFlag )
			noDupVec.push_back( iSectEdgeVec[i] );
		else
		{
			delete iSectEdgeVec[i]->n0;
			delete iSectEdgeVec[i]->n1;
			delete iSectEdgeVec[i];
		}
	}
	iSectEdgeVec = noDupVec;

	//==== Add Corners of Triangle ====//			//jrg figure who should allocate data...
	nVec.push_back( n0 );
	nVec.push_back( n1 );
	nVec.push_back( n2 );

	//==== Add Edges For Perimeter ====//
	for ( i = 0 ; i < 3 ; i++ )
		eVec.push_back(  new TEdge() );

	eVec[0]->n0 = n0;	eVec[0]->n1 = n1;
	eVec[1]->n0 = n1;	eVec[1]->n1 = n2;
	eVec[2]->n0 = n2;	eVec[2]->n1 = n0;

	//==== Load All Possible Nodes to Add ====//
	vector< vec3d > pVec;				// Pnts to be added
	vector< int > matchNodeIndex;		// Nodes Index Which Correspond to pVec
	for ( i = 0 ; i < (int)iSectEdgeVec.size() ; i++ )
	{
		pVec.push_back( iSectEdgeVec[i]->n0->pnt );
		pVec.push_back( iSectEdgeVec[i]->n1->pnt );
	}
	matchNodeIndex.resize( pVec.size() );
	for ( i = 0 ; i < (int)matchNodeIndex.size() ; i++ )
		matchNodeIndex[i] = -1;

	//==== Add Edge Nodes ====//
	for ( i = 0 ; i < (int)pVec.size() ; i++ )
	{
		for ( j = 0 ; j < (int)eVec.size() ; j++ )
		{
			if ( onEdge( pVec[i], eVec[j], onEdgeTol ) )
			{
				//==== SplitEdge ====//
				TNode* sn = new TNode();		// New node
				sn->isectFlag = 1;
				nVec.push_back( sn );
				matchNodeIndex[i] = nVec.size()-1;
				sn->pnt = pVec[i];
				TEdge* se = new TEdge();		// New Edge
				se->n0 = eVec[j]->n0;
				se->n1 = sn;
				eVec[j]->n0 = sn;				// Change Split Edge
				eVec.push_back(se);
				break;
			}
		}
	}

	//==== Add Other Points ====//
	for ( i = 0 ; i < (int)pVec.size() ; i++ )
	{
		//==== Look For Existing Node ====//
		if ( matchNodeIndex[i] == -1 )
		{
			for ( j = 0 ; j < (int)nVec.size() ; j++ )
			{
				double d = dist( pVec[i], nVec[j]->pnt );
				if ( d < onEdgeTol )
				{
					matchNodeIndex[i] = j;
					break;
				}
			}
		}
		//==== Didnt Find One - Add New ====//
		if ( matchNodeIndex[i] == -1 )
		{
			TNode* sn = new TNode();		// New node
			sn->isectFlag = 1;
			nVec.push_back( sn );
			matchNodeIndex[i] = nVec.size()-1;
			sn->pnt = pVec[i];
		}
	}

	//==== Add Edges ====//
	for ( i = 0 ; i < (int)matchNodeIndex.size() ; i+=2 )
	{
		int ind0 = matchNodeIndex[i];
		int ind1 = matchNodeIndex[i+1];

		if ( ind0 != ind1 )
		{
			int existFlag = 0;
			for ( j = 0 ; j < (int)eVec.size() ; j++ )
			{
				if      ( eVec[j]->n0 == nVec[ind0] && eVec[j]->n1 == nVec[ind1] )
					existFlag = 1;
				else if ( eVec[j]->n0 == nVec[ind1] && eVec[j]->n1 == nVec[ind0] )
					existFlag = 1;
			}

			if ( !existFlag )
			{
				TEdge* se = new TEdge();		// New Edge
				se->n0 = nVec[ind0];
				se->n1 = nVec[ind1];
				eVec.push_back(se);
			}
		}
	}
	
	//==== Look For Crossing Edges ====//
	//==== jrg - this code is suspect....
	int findCross = 1;
	double u, v;
	vec3d np0, np1;
	while ( findCross )
	{
		findCross = 0;
		for ( i = 0 ; i < (int)eVec.size() ; i++ )
		{
			TNode* en0 = eVec[i]->n0;
			TNode* en1 = eVec[i]->n1;
			for ( j = i+1 ; j < (int)eVec.size() ; j++ )
			{
				TNode* en2 = eVec[j]->n0;
				TNode* en3 = eVec[j]->n1;

				if ( en0 != en2 && en0 != en3 && en1 != en2 && en1 != en3 )
				{
					double d = dist3D_Segment_to_Segment(en0->pnt, en1->pnt, en2->pnt, en3->pnt, 
										 &u, &np0, &v, &np1 );

					if ( d < onEdgeTol*onEdgeTol )
					{
						findCross = 1;

						if      (  u < uvMinTol && v < uvMinTol )
							findCross = 0;
						else if (  u < uvMinTol && v > uvMaxTol )
							findCross = 0;
						else if (  u > uvMaxTol && v < uvMinTol )
							findCross = 0;
						else if (  u > uvMaxTol && v > uvMaxTol )
							findCross = 0;

						else if ( u < uvMinTol && v >= uvMinTol && v <= uvMaxTol ) 
						{
							//==== Break Along Line Seg 2 ====//
							TEdge* se = new TEdge();		// New Edge
							se->n0 = en0;
							se->n1 = en3;
							eVec[j]->n0 = en2;
							eVec[j]->n1 = en0;
							eVec.push_back( se );
						}
						else if ( u > uvMaxTol && v >= uvMinTol && v <= uvMaxTol ) 
						{
							//==== Break Along Line Seg 2 ====//
							TEdge* se = new TEdge();		// New Edge
							se->n0 = en1;
							se->n1 = en3;
							eVec[j]->n0 = en2;
							eVec[j]->n1 = en1;
							eVec.push_back( se );
						}
						else if ( v < uvMinTol && u >= uvMinTol && u <= uvMaxTol )
						{
							//==== Break Along Line Seg 1 ====//
							TEdge* se = new TEdge();		// New Edge
							se->n0 = en2;
							se->n1 = en1;
							eVec[i]->n0 = en2;
							eVec[i]->n1 = en0;
							eVec.push_back( se );
						}
						else if ( v > uvMaxTol && u >= uvMinTol && u <= uvMaxTol )
						{
							//==== Break Along Line Seg 1 ====//
							TEdge* se = new TEdge();		// New Edge
							se->n0 = en3;
							se->n1 = en1;
							eVec[i]->n0 = en3;
							eVec[i]->n1 = en0;
							eVec.push_back( se );
						}
						else 
						{						
							//==== New Node at Crossing Point ====//
							TNode* sn = new TNode();
							sn->isectFlag = 1;
							nVec.push_back( sn );
							sn->pnt = (np0 + np1)*0.5;
							TEdge* se0 = new TEdge();		// New Edge
							se0->n0 = en0;
							se0->n1 = sn;
							eVec[i]->n0 = sn;
							eVec[i]->n1 = en1;
							TEdge* se1 = new TEdge();		// New Edge
							se1->n0 = en2;
							se1->n1 = sn;
							eVec[j]->n0 = sn;
							eVec[j]->n1 = en3;

							eVec.push_back( se1 );
							eVec.push_back( se0 );
						}

						if ( findCross )
							j = eVec.size();
					}
				}
				if ( findCross )
					i = eVec.size();
			}
		}
	}
				
	//==== Determine Which Axis to Flatten ====//
	vec3d zn0, zn1, zn2;
	zn0 = n0->pnt;	zn0.set_x(0);
	zn1 = n1->pnt;	zn1.set_x(0);
	zn2 = n2->pnt;	zn2.set_x(0);
//	double dx = MIN( dist( zn0, zn1 ), MIN( dist( zn1, zn2 ), dist( zn2, zn0 ) ) );
	double dx = MAX( dist( zn0, zn1 ), MAX( dist( zn1, zn2 ), dist( zn2, zn0 ) ) );

	zn0 = n0->pnt;	zn0.set_y(0);
	zn1 = n1->pnt;	zn1.set_y(0);
	zn2 = n2->pnt;	zn2.set_y(0);
//	double dy = MIN( dist( zn0, zn1 ), MIN( dist( zn1, zn2 ), dist( zn2, zn0 ) ) );
	double dy = MAX( dist( zn0, zn1 ), MAX( dist( zn1, zn2 ), dist( zn2, zn0 ) ) );

	zn0 = n0->pnt;	zn0.set_z(0);
	zn1 = n1->pnt;	zn1.set_z(0);
	zn2 = n2->pnt;	zn2.set_z(0);
//	double dz = MIN( dist( zn0, zn1 ), MIN( dist( zn1, zn2 ), dist( zn2, zn0 ) ) );
	double dz = MAX( dist( zn0, zn1 ), MAX( dist( zn1, zn2 ), dist( zn2, zn0 ) ) );
	
	int flattenAxis = 0;
	if ( dy >= dx && dy > dz )
		flattenAxis = 1;
	else if ( dz >= dx && dz > dy )
		flattenAxis = 2;

	//==== Use Triangle to Split Tri ====//
	if ( meshFlag )
		niceTriSplit( flattenAxis );
	else
		triangulateSplit( flattenAxis );

}

void TTri::triangulateSplit( int flattenAxis )
{
	int i, j;
	
	//==== Dump Into Triangle ====//
	struct triangulateio in;
	struct triangulateio out;

	memset(&in,0,sizeof(in));		// Load Zeros
	memset(&out,0,sizeof(out));

	//==== PreAllocate Data For In/Out ====//
	in.pointlist    = (REAL *) malloc(nVec.size() * 2 * sizeof(REAL));
	out.pointlist   = NULL;

	in.segmentlist  = (int *) malloc(eVec.size() * 2 * sizeof(int));
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

	//==== Load Points into Traingle Struct ====//
	in.numberofpoints = nVec.size();

	int cnt = 0;
	for ( j = 0 ; j < (int)nVec.size() ; j++ )
	{
		if ( flattenAxis == 0 )
		{
			in.pointlist[cnt] = nVec[j]->pnt.y();	cnt++;
			in.pointlist[cnt] = nVec[j]->pnt.z();	cnt++;
		}
		else if ( flattenAxis == 1 )
		{
			in.pointlist[cnt] = nVec[j]->pnt.x();	cnt++;
			in.pointlist[cnt] = nVec[j]->pnt.z();	cnt++;
		}
		else if ( flattenAxis == 2 )
		{
			in.pointlist[cnt] = nVec[j]->pnt.x();	cnt++;
			in.pointlist[cnt] = nVec[j]->pnt.y();	cnt++;
		}
	}
//static FILE* fp = fopen( "debug.txt", "w" );
//
//static int static_cnt = 0;
//fprintf(fp, "Triangle Cnt = %d\n", static_cnt++ );
//if ( static_cnt == 843 )
//{
//	for ( i = 0 ; i < in.numberofpoints ; i++ )
//	{
//		fprintf(fp, "%f %f \n", in.pointlist[i*2],  in.pointlist[i*2 + 1] );
//	}
//
//	for ( j = 0 ; j < (int)nVec.size() ; j++ )
//	{
//		fprintf(fp, "%f %f %f \n", nVec[j]->pnt.x(), nVec[j]->pnt.y(), nVec[j]->pnt.z() );
//	}
//
//}

	//==== Match Edge Nodes to Indices in NVec ====//
	vector< int > segIndList;
	for ( i = 0 ; i < (int)eVec.size() ; i++ )
	{
		for ( j = 0 ; j < (int)nVec.size() ; j++ )
		{
			if ( eVec[i]->n0 == nVec[j] )
			{
				segIndList.push_back(j);
				break;
			}
		}
		for ( j = 0 ; j < (int)nVec.size() ; j++ )
		{
			if ( eVec[i]->n1 == nVec[j] )
			{
				segIndList.push_back(j);
				break;
			}
		}
	}
	cnt = 0;
	for ( j = 0 ; j < (int)segIndList.size() ; j+=2 )
	{
		in.segmentlist[cnt] = segIndList[j];		cnt++;
		in.segmentlist[cnt] = segIndList[j+1];		cnt++;
	}

	in.numberofsegments = segIndList.size()/2;

	if ( in.numberofpoints > 3 && in.numberofsegments > 3 )
	{	
		//==== Check For Duplicate Points =====//
		int dupFlag = 0;
		for ( i = 0 ; i < in.numberofpoints ; i++ )
			for ( j = i+1 ; j < in.numberofpoints ; j++ )
			{
				double del = fabs( in.pointlist[i*2] - in.pointlist[j*2] ) + 
							 fabs( in.pointlist[i*2+1] - in.pointlist[j*2+1] );

				if ( del < 0.0000001 )
					dupFlag = 1;
			}

		if ( !dupFlag )
		{
			//==== Constrained Delaunay Trianglulation ====//
			triangulate ("zpQ", &in, &out, (struct triangulateio *) NULL);
		}
//fprintf(fp, "Triangulate in = %d out = %d \n", in.numberofpoints, out.numberofpoints );
	}


	//==== Load Triangles if No New Point Created ====//
	cnt = 0;
	for ( i = 0 ; i < out.numberoftriangles ; i++ )
	{
		if ( out.trianglelist[cnt]   < (int)nVec.size() && 
			 out.trianglelist[cnt+1] < (int)nVec.size() &&
			 out.trianglelist[cnt+2] < (int)nVec.size() )
		{
			TTri* t = new TTri();
			t->n0 = nVec[out.trianglelist[cnt]];
			t->n1 = nVec[out.trianglelist[cnt+1]];	
			t->n2 = nVec[out.trianglelist[cnt+2]];	
			splitVec.push_back( t );
		}
		else
		{
/* jrg problem - look into...
			printf("\n");
			printf("m\n");
			printf("c red\n");
			printf("%12.8f, %12.8f, %12.8f \n", n0->pnt.x(), n0->pnt.y(), n0->pnt.z());
			printf("%12.8f, %12.8f, %12.8f \n", n1->pnt.x(), n1->pnt.y(), n1->pnt.z());
			printf("%12.8f, %12.8f, %12.8f \n", n2->pnt.x(), n2->pnt.y(), n2->pnt.z());
			printf("c blue\n");
			for ( j = 0 ; j < iSectEdgeVec.size() ; j++ )
			{
				printf("m\n");
				printf("%12.8f, %12.8f, %12.8f \n", iSectEdgeVec[j]->n0->pnt.x(), iSectEdgeVec[j]->n0->pnt.y(), iSectEdgeVec[j]->n0->pnt.z());
				printf("%12.8f, %12.8f, %12.8f \n", iSectEdgeVec[j]->n0->pnt.x(), iSectEdgeVec[j]->n1->pnt.y(), iSectEdgeVec[j]->n1->pnt.z());
			}
*/

		}
		cnt += 3;
	}

	//==== Free Local Memory ====//
	if ( in.pointlist )				free( in.pointlist );
	if ( in.segmentlist )			free( in.segmentlist );

	if ( out.pointlist )			free( out.pointlist );
	if ( out.pointmarkerlist )		free( out.pointmarkerlist );
	if ( out.trianglelist )			free( out.trianglelist );
	if ( out.segmentlist )			free( out.segmentlist );
	if ( out.segmentmarkerlist )	free( out.segmentmarkerlist );

	//=== Orient Tris to Match Normal ====//
	for ( i = 0 ; i < (int)splitVec.size() ; i++ )
	{
		TTri* t = splitVec[i];
		vec3d d01 = t->n0->pnt - t->n1->pnt;
		vec3d d21 = t->n2->pnt - t->n1->pnt;

		vec3d cx = cross( d21, d01 );
		cx.normalize();
		norm.normalize();

		double del = dist_squared(cx, norm);
		if ( del > 1.0)
		{
			TNode* tmp = t->n1;
			t->n1 = t->n2;
			t->n2 = tmp;
		}
	}
}

void TTri::niceTriSplit( int flattenAxis )
{
	int i, j;
	
	//==== Dump Into Triangle ====//
	struct triangulateio in;
	struct triangulateio out;

	memset(&in,0,sizeof(in));		// Load Zeros
	memset(&out,0,sizeof(out));

	//==== PreAllocate Data For In/Out ====//
	in.pointlist    = (REAL *) malloc(nVec.size() * 2 * sizeof(REAL));
	out.pointlist   = NULL;

	in.segmentlist  = (int *) malloc(eVec.size() * 2 * sizeof(int));
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

	//==== Load Points into Traingle Struct ====//
	in.numberofpoints = nVec.size();

	int cnt = 0;
	for ( j = 0 ; j < (int)nVec.size() ; j++ )
	{
		if ( flattenAxis == 0 )
		{
			in.pointlist[cnt] = nVec[j]->pnt.y();	cnt++;
			in.pointlist[cnt] = nVec[j]->pnt.z();	cnt++;
		}
		else if ( flattenAxis == 1 )
		{
			in.pointlist[cnt] = nVec[j]->pnt.x();	cnt++;
			in.pointlist[cnt] = nVec[j]->pnt.z();	cnt++;
		}
		else if ( flattenAxis == 2 )
		{
			in.pointlist[cnt] = nVec[j]->pnt.x();	cnt++;
			in.pointlist[cnt] = nVec[j]->pnt.y();	cnt++;
		}
	}

	//==== Match Edge Nodes to Indices in NVec ====//
	vector< int > segIndList;
	for ( i = 0 ; i < (int)eVec.size() ; i++ )
	{
		for ( j = 0 ; j < (int)nVec.size() ; j++ )
		{
			if ( eVec[i]->n0 == nVec[j] )
			{
				segIndList.push_back(j);
				break;
			}
		}
		for ( j = 0 ; j < (int)nVec.size() ; j++ )
		{
			if ( eVec[i]->n1 == nVec[j] )
			{
				segIndList.push_back(j);
				break;
			}
		}
	}
	cnt = 0;
	for ( j = 0 ; j < (int)segIndList.size() ; j+=2 )
	{
		in.segmentlist[cnt] = segIndList[j];		cnt++;
		in.segmentlist[cnt] = segIndList[j+1];		cnt++;
	}

	in.numberofsegments = segIndList.size()/2;

	if ( in.numberofpoints > 3 && in.numberofsegments > 3 )
	{	
		//==== Check For Duplicate Points =====//
		int dupFlag = 0;
		for ( i = 0 ; i < in.numberofpoints ; i++ )
			for ( j = i+1 ; j < in.numberofpoints ; j++ )
			{
				double del = fabs( in.pointlist[i*2] - in.pointlist[j*2] ) + 
							 fabs( in.pointlist[i*2+1] - in.pointlist[j*2+1] );

				if ( del < 0.0000001 )
					dupFlag = 1;
			}

		if ( !dupFlag )
		{
			//==== Constrained Delaunay Trianglulation ====//
//			triangulate ("zpq20QjS5Y", &in, &out, (struct triangulateio *) NULL);
			triangulate ("zpQ", &in, &out, (struct triangulateio *) NULL);
		}
	}

//printf("Triangulate in = %d out = %d \n", in.numberofpoints, out.numberofpoints );

	//==== Map Points to Tri ====//
//printf("  n0 = %f %f %f \n", n0->pnt.x(), n0->pnt.y(), n0->pnt.z() );
//printf("  n1 = %f %f %f \n", n1->pnt.x(), n1->pnt.y(), n1->pnt.z() );
//printf("  n2 = %f %f %f \n", n2->pnt.x(), n2->pnt.y(), n2->pnt.z() );

	nVec.clear();			// jrg del mem...
	nVec.push_back( n0 );
	nVec.push_back( n1 );
	nVec.push_back( n2 );

	vec3d ray;		
	if ( flattenAxis == 0 )
		ray.set_xyz( 1.0, 0.0, 0.0 );
	else if ( flattenAxis == 1 )
		ray.set_xyz( 0.0, 1.0, 0.0 );
	else if ( flattenAxis == 2 )
		ray.set_xyz( 0.0, 0.0, 1.0 );

	for ( i = 3 ; i < out.numberofpoints ; i++ )
	{
		vec3d pnt;
		if ( flattenAxis == 0 )
			pnt.set_xyz( 0.0, out.pointlist[i*2], out.pointlist[i*2 + 1] );
		else if ( flattenAxis == 1 )
			pnt.set_xyz( out.pointlist[i*2], 0.0, out.pointlist[i*2 + 1] );
		else if ( flattenAxis == 2 )
			pnt.set_xyz( out.pointlist[i*2], out.pointlist[i*2 + 1], 0.0 );

		double t;
		vec3d n0pnt  = n0->pnt;
		vec3d n10pnt = n1->pnt - n0->pnt;
		vec3d n20pnt = n2->pnt - n0->pnt;
		plane_ray_intersect(n0pnt, n10pnt, n20pnt, pnt, ray, t);

		vec3d mapPnt = pnt + ray*t;

		TNode* sn = new TNode();		// New node
		sn->pnt = mapPnt;
		sn->isectFlag = 1;
		nVec.push_back( sn );
	}

	//==== Load Triangles if No New Point Created ====//
	cnt = 0;
	for ( i = 0 ; i < out.numberoftriangles ; i++ )
	{
		if ( out.trianglelist[cnt]   < (int)nVec.size() && 
			 out.trianglelist[cnt+1] < (int)nVec.size() &&
			 out.trianglelist[cnt+2] < (int)nVec.size() )
		{
			TTri* t = new TTri();
			t->n0 = nVec[out.trianglelist[cnt]];
			t->n1 = nVec[out.trianglelist[cnt+1]];	
			t->n2 = nVec[out.trianglelist[cnt+2]];	
			splitVec.push_back( t );
		}
		else
		{
/* jrg problem - look into...
			printf("\n");
			printf("m\n");
			printf("c red\n");
			printf("%12.8f, %12.8f, %12.8f \n", n0->pnt.x(), n0->pnt.y(), n0->pnt.z());
			printf("%12.8f, %12.8f, %12.8f \n", n1->pnt.x(), n1->pnt.y(), n1->pnt.z());
			printf("%12.8f, %12.8f, %12.8f \n", n2->pnt.x(), n2->pnt.y(), n2->pnt.z());
			printf("c blue\n");
			for ( j = 0 ; j < iSectEdgeVec.size() ; j++ )
			{
				printf("m\n");
				printf("%12.8f, %12.8f, %12.8f \n", iSectEdgeVec[j]->n0->pnt.x(), iSectEdgeVec[j]->n0->pnt.y(), iSectEdgeVec[j]->n0->pnt.z());
				printf("%12.8f, %12.8f, %12.8f \n", iSectEdgeVec[j]->n0->pnt.x(), iSectEdgeVec[j]->n1->pnt.y(), iSectEdgeVec[j]->n1->pnt.z());
			}
*/

		}
		cnt += 3;
	}

	//==== Free Local Memory ====//
	if ( in.pointlist )				free( in.pointlist );
	if ( in.segmentlist )			free( in.segmentlist );

	if ( out.pointlist )			free( out.pointlist );
	if ( out.pointmarkerlist )		free( out.pointmarkerlist );
	if ( out.trianglelist )			free( out.trianglelist );
	if ( out.segmentlist )			free( out.segmentlist );
	if ( out.segmentmarkerlist )	free( out.segmentmarkerlist );

	//=== Orient Tris to Match Normal ====//
	for ( i = 0 ; i < (int)splitVec.size() ; i++ )
	{
		TTri* t = splitVec[i];
		vec3d d01 = t->n0->pnt - t->n1->pnt;
		vec3d d21 = t->n2->pnt - t->n1->pnt;

		vec3d cx = cross( d21, d01 );
		cx.normalize();
		norm.normalize();

		double del = dist_squared(cx, norm);
		if ( del > 1.0)
		{
			TNode* tmp = t->n1;
			t->n1 = t->n2;
			t->n2 = tmp;
		}
	}
}


int TTri::onEdge( vec3d & p, TEdge* e, double onEdgeTol )
{
	//==== Make Sure Not Duplicate Points ====//
	if ( dist( p, e->n0->pnt ) < onEdgeTol )
		return 0;

	if ( dist( p, e->n1->pnt ) < onEdgeTol )
		return 0;

	double t;
	double d = pointSegDistSquared(p, e->n0->pnt, e->n1->pnt, &t );

	if ( d < onEdgeTol*onEdgeTol )
		return 1;

	return 0;
}

int TTri::dupEdge( TEdge* e0, TEdge* e1, double tol )
{
	if ( (dist(e0->n0->pnt, e1->n0->pnt) < tol ) &&
		 (dist(e0->n1->pnt, e1->n1->pnt) < tol ) )
		 return 1;

	if ( (dist(e0->n0->pnt, e1->n1->pnt) < tol ) &&
		 (dist(e0->n1->pnt, e1->n0->pnt) < tol ) )
		 return 1;

	return 0;
}


void TTri::draw()
{
	int i;

	//==== Draw Sub Tris ====//
	for ( i = 0 ; i < (int)splitVec.size() ; i++ )
	{
		if ( !splitVec[i]->interiorFlag )
		{
			glColor3ub( (i*50+100)%255, (i*80)%255, (i*100+50)%255 );
			glBegin( GL_POLYGON );				
				glVertex3dv( splitVec[i]->n0->pnt.data() );
				glVertex3dv( splitVec[i]->n1->pnt.data() );
				glVertex3dv( splitVec[i]->n2->pnt.data() );
			glEnd();
		}
	}


}

void TTri::computeCosAngles( double* ang0, double* ang1, double* ang2 )
{
	double dsqr01 = dist_squared( n0->pnt, n1->pnt );
	double dsqr12 = dist_squared( n1->pnt, n2->pnt );
	double dsqr20 = dist_squared( n2->pnt, n0->pnt );

	double d01 = sqrt(dsqr01);
	double d12 = sqrt(dsqr12);
	double d20 = sqrt(dsqr20);

	*ang0 = (-dsqr12+dsqr01+dsqr20)/(2.0*d01*d20);

	*ang1 = (-dsqr20+dsqr01+dsqr12)/(2.0*d01*d12);

	*ang2 = (-dsqr01+dsqr12+dsqr20)/(2.0*d12*d20);
}


void TTri::splitEdges( TNode* n01, TNode* n12, TNode* n20 )
{
	TTri* tri;
	if ( n01 && n12 && n20 )		// Three Split - Make Four Tris
	{
		tri = new TTri();
		tri->n0 = n0;	tri->n1 = n01;	tri->n2 = n20;	tri->norm = norm;
		splitVec.push_back( tri );

		tri = new TTri();
		tri->n0 = n1;	tri->n1 = n12;	tri->n2 = n01;	tri->norm = norm;
		splitVec.push_back( tri );

		tri = new TTri();
		tri->n0 = n2;	tri->n1 = n20;	tri->n2 = n12;	tri->norm = norm;
		splitVec.push_back( tri );

		tri = new TTri();
		tri->n0 = n01;	tri->n1 = n12;	tri->n2 = n20;	tri->norm = norm;
		splitVec.push_back( tri );
	
	}



}







//===============================================//
//===============================================//
//===============================================//
//===============================================//
//					TBndBox
//===============================================//
//===============================================//
//===============================================//
//===============================================//

TBndBox::TBndBox()
{
	for ( int i = 0 ; i < 8 ; i++ )
	{
		sBoxVec[i] = 0;
	}
}

TBndBox::~TBndBox()
{
	for ( int i = 0 ; i < 8 ; i++ )
	{
		if ( sBoxVec[i] )
			delete sBoxVec[i];
	}
}

//==== Create Oct Tree of Overlaping BndBoxes ====//
void TBndBox::splitBox()
{
	int i;
	if ( triVec.size() > 32 )
	{
		//==== Find Split Points ====//
		double hx = 0.5*(box.get_max(0) + box.get_min(0));
		double hy = 0.5*(box.get_max(1) + box.get_min(1));
		double hz = 0.5*(box.get_max(2) + box.get_min(2));

		for ( i = 0 ; i < 8 ; i++ )
		{
			sBoxVec[i] = new TBndBox();
		}

		for ( i = 0 ; i < (int)triVec.size() ; i++ )
		{
			int cnt = 0;
			if ( triVec[i]->n0->pnt.x() > hx ) cnt += 1;
			if ( triVec[i]->n0->pnt.y() > hy ) cnt += 2;
			if ( triVec[i]->n0->pnt.z() > hz ) cnt += 4;
			sBoxVec[cnt]->addTri( triVec[i] );
		}

		int contSplitFlag = 1;

		for ( i = 0 ; i < 8 ; i++ )
		{
			if ( triVec.size() == sBoxVec[i]->triVec.size() )
			{
				contSplitFlag = 0;
				break;
			}
		}
		if ( contSplitFlag )
		{
			for ( i = 0 ; i < 8 ; i++ )
			{
				sBoxVec[i]->splitBox();
			}
		}
	}
}

void TBndBox::addTri( TTri* t )
{
	triVec.push_back( t );
	box.update( t->n0->pnt );
	box.update( t->n1->pnt );
	box.update( t->n2->pnt );
}

void  TBndBox::addLeafNodes( vector< TBndBox* > & leafVec )
{
	int i;

	if ( sBoxVec[0] )		// Keep Moving Down
	{
		for ( i = 0 ; i < 8 ; i++ )
		{
			sBoxVec[i]->addLeafNodes( leafVec );
		}
	}
	else
	{
		leafVec.push_back( this );
	}
}

void TBndBox::draw()
{
	int i;
	if ( sBoxVec[0] )
	{
		for ( i = 0 ; i < 8 ; i++ )
		{
			sBoxVec[i]->draw();
		}
	}
	else
	{
	  double temp[3];
	  temp[0] = box.get_min(0);
	  temp[1] = box.get_min(1);
	  temp[2] = box.get_min(2);

	  glBegin( GL_LINE_STRIP );
		glVertex3dv(temp);		
		temp[0] = box.get_max(0);		glVertex3dv(temp);
		temp[1] = box.get_max(1);		glVertex3dv(temp);
		temp[2] = box.get_max(2);		glVertex3dv(temp);
		temp[0] = box.get_min(0);		glVertex3dv(temp);
		temp[2] = box.get_min(2);		glVertex3dv(temp);
		temp[1] = box.get_min(1);		glVertex3dv(temp);
		temp[2] = box.get_max(2);		glVertex3dv(temp);
		temp[0] = box.get_max(0);		glVertex3dv(temp);
		temp[2] = box.get_min(2);		glVertex3dv(temp);
	  glEnd();

	  glBegin( GL_LINE_STRIP );
		temp[2] = box.get_max(2);		glVertex3dv(temp);
		temp[1] = box.get_max(1);		glVertex3dv(temp);
	  glEnd();

	  glBegin( GL_LINE_STRIP );
		temp[2] = box.get_min(2);		glVertex3dv(temp);
		temp[0] = box.get_min(0);		glVertex3dv(temp);
	  glEnd();

	  glBegin( GL_LINE_STRIP );
		temp[2] = box.get_max(2);		glVertex3dv(temp);
		temp[1] = box.get_min(1);		glVertex3dv(temp);
	  glEnd();
 
	}
}
	
void TBndBox::intersect( TBndBox* iBox )
{
	int i;

	if ( !compare( box, iBox->box ) )
		return;

	if ( sBoxVec[0] )
	{
		for ( i = 0 ; i < 8 ; i++ )
		{
			iBox->intersect( sBoxVec[i] );
		}
	}
	else if ( iBox->sBoxVec[0] )
	{
		for ( i = 0 ; i < 8 ; i++ )
		{
			iBox->sBoxVec[i]->intersect( this );
		}
	}
	else
	{
		int i, j;
		int coplanarFlag;
		vec3d e0;
		vec3d e1;

		int iCnt = 0;

		for ( i = 0 ; i < (int)triVec.size() ; i++ )
		{
			TTri* t0 = triVec[i];
			for ( j = 0 ; j < (int)iBox->triVec.size() ; j++ )
			{
				TTri* t1 = iBox->triVec[j];

				int iflag = tri_tri_intersect_with_isectline( 
					t0->n0->pnt.v, t0->n1->pnt.v, t0->n2->pnt.v,
					t1->n0->pnt.v, t1->n1->pnt.v, t1->n2->pnt.v,
					&coplanarFlag, e0.v, e1.v ); 

				iCnt += iflag;
				if ( iflag && !coplanarFlag )
				{
					TEdge* ie0 = new TEdge();
					ie0->n0 = new TNode();	ie0->n0->pnt = e0;	
					ie0->n1 = new TNode();	ie0->n1->pnt = e1;

					TEdge* ie1 = new TEdge();
					ie1->n0 = new TNode();	ie1->n0->pnt = e0;
					ie1->n1 = new TNode();	ie1->n1->pnt = e1;

					if ( dist( e0, e1 ) > 0.000001 )
					{
						t0->iSectEdgeVec.push_back( ie0 );
						t1->iSectEdgeVec.push_back( ie1 );
					}
					else
					{
						delete ie0->n0;
						delete ie0->n1;
						delete ie1->n0;
						delete ie1->n1;
						delete ie0;
						delete ie1;
					}

				}
			}
		}
	}
}
void  TBndBox::numCrossXRay( vec3d & orig, vector<double> & tParmVec )
{
	int i;

	if ( orig.y() < box.get_min(1) )
		return;
	if ( orig.y() > box.get_max(1) )
		return;
	if ( orig.z() < box.get_min(2) )
		return;
	if ( orig.z() > box.get_max(2) )
		return;

	if ( sBoxVec[0] )
	{
		for ( i = 0 ; i < 8 ; i++ )
		{
			sBoxVec[i]->numCrossXRay( orig, tParmVec );
		}
		return;
	}

	//==== Check All Tris In Box ====//
	double tparm, uparm, vparm;
	vec3d dir( 1.0, 0.0, 0.0 );
	for ( i = 0 ; i < (int)triVec.size() ; i++ )
	{
		TTri* tri = triVec[i];
		int iFlag = intersect_triangle( orig.v, dir.v,
			tri->n0->pnt.v, tri->n1->pnt.v, tri->n2->pnt.v, &tparm, &uparm, &vparm );

		if ( iFlag && tparm > 0.0 )
		{
			//==== Find If T is Already Included ====//
			int dupFlag = 0;
			for ( int j = 0 ; j < (int)tParmVec.size() ; j++ )
			{
				if ( fabs(tparm-tParmVec[j]) < 0.0000001 )
				{
					dupFlag = 1;
					break;
				}
			}

			if ( !dupFlag )
				tParmVec.push_back( tparm );
		}
	}

}

void TBndBox::segIntersect( vec3d & p0, vec3d & p1, vector< vec3d > & ipntVec )
{
	int i, t;

	if ( !box.check_pnt( p0.x(), p0.y(), p0.z() ) && !box.check_pnt( p1.x(), p1.y(), p1.z() ) )
		return;

	if ( sBoxVec[0] )
	{
		for ( i = 0 ; i < 8 ; i++ )
		{
			sBoxVec[i]->segIntersect( p0, p1, ipntVec );
		}
		return;
	}

	//==== Check All Tris In Box ====//
	double tparm, uparm, vparm;
	for ( t = 0 ; t < (int)triVec.size() ; t++ )
	{
		TTri* tri = triVec[t];
		vec3d n0pnt  = tri->n0->pnt;
		vec3d n10pnt = tri->n1->pnt - tri->n0->pnt;
		vec3d n20pnt = tri->n2->pnt - tri->n0->pnt;
		vec3d p10    = p1-p0;
		if ( tri_seg_intersect( n0pnt,  n10pnt, n20pnt,
			p0, p10, uparm, vparm, tparm ) )
		{
			vec3d pnt = p0 + (p1-p0)*tparm;
			ipntVec.push_back( pnt );
		}
	}

}


//===============================================//
//===============================================//
//===============================================//
//===============================================//
//					NBndBox
//===============================================//
//===============================================//
//===============================================//
//===============================================//
NBndBox::NBndBox()
{
	for ( int i = 0 ; i < 8 ; i++ )
	{
		sBoxVec[i] = 0;
	}
}

NBndBox::~NBndBox()
{
	for ( int i = 0 ; i < 8 ; i++ )
	{
		if ( sBoxVec[i] )
			delete sBoxVec[i];
	}
}

//==== Create Oct Tree of Overlaping BndBoxes ====//
void NBndBox::splitBox(double maxSize)
{
	int i;
	if ( nodeVec.size() > 64 && box.diag_dist() > maxSize  )
	{
		//==== Find Split Points ====//
		double hx = 0.5*(box.get_max(0) + box.get_min(0));
		double hy = 0.5*(box.get_max(1) + box.get_min(1));
		double hz = 0.5*(box.get_max(2) + box.get_min(2));

		for ( i = 0 ; i < 8 ; i++ )
		{
			sBoxVec[i] = new NBndBox();
		}

		for ( i = 0 ; i < (int)nodeVec.size() ; i++ )
		{
			int cnt = 0;
			if ( nodeVec[i]->pnt.x() > hx ) cnt += 1;
			if ( nodeVec[i]->pnt.y() > hy ) cnt += 2;
			if ( nodeVec[i]->pnt.z() > hz ) cnt += 4;
			sBoxVec[cnt]->addNode( nodeVec[i] );
		}

		int contSplitFlag = 1;
/* Not Needed for Nodes ???
		for ( i = 0 ; i < 8 ; i++ )
		{
			if ( triVec.size() == sBoxVec[i]->triVec.size() )
			{
				contSplitFlag = 0;
				break;
			}
		}
*/
		if ( contSplitFlag )
		{
			for ( i = 0 ; i < 8 ; i++ )
			{
				sBoxVec[i]->splitBox(maxSize);
			}
		}
	}
}

void NBndBox::addNode( TNode* n )
{
	nodeVec.push_back( n );
	box.update( n->pnt );
}

void NBndBox::addLeafNodes( vector< NBndBox* > & leafVec )
{
	int i;

	if ( sBoxVec[0] )		// Keep Moving Down
	{
		for ( i = 0 ; i < 8 ; i++ )
		{
			sBoxVec[i]->addLeafNodes( leafVec );
		}
	}
	else
	{
		if ( nodeVec.size() )
			leafVec.push_back( this );
	}
}

void NBndBox::draw()
{
	

	
}
//===============================================//
//===============================================//
//===============================================//
//===============================================//
//					WaterTight Check
//===============================================//
//===============================================//
//===============================================//
//===============================================//

void TMesh::waterTightCheck(FILE* fid, vector<TMesh*> & origTMeshVec)
{
	double minE, minA, maxA;

	fprintf( fid, "\n...WaterTight Check...\n");

	matchNodes();

	checkValid(0);

	meshStats(&minE, &minA, &maxA);

	fprintf( fid, "  Before Edge Swap and Needle Removal\n");
	fprintf( fid, "  Min Edge Length = %f\n", minE );
	fprintf( fid, "  Min Angle = %f\n", minA );
	fprintf( fid, "  Max Angle = %f\n", maxA );

	//==== Try To Improve Triangle Quality ====//
	for ( int i = 0 ; i < 10 ; i++ )
	{
		tagNeedles( 2.0, 0.005, 1 );
		matchNodes();

		swapEdges(178.0);

		tagNeedles( 2.0, 0.005, 1 );
		matchNodes();
	}

	meshStats(&minE, &minA, &maxA);

	fprintf( fid, "  After Edge Swap and Needle Removal\n");
	fprintf( fid, "  Min Edge Length = %f\n", minE );
	fprintf( fid, "  Min Angle = %f\n", minA );
	fprintf( fid, "  Max Angle = %f\n", maxA );

	checkValid(fid);



}

void TMesh::tagNeedles(double minTriAng, double minAspectRatio, int delFlag )
{
	int t;

	//==== Only Tag Intersection Triangle ====//
	for ( t = 0 ; t < (int)tVec.size() ; t++ )
	{
		if ( tVec[t]->n0->isectFlag || 	tVec[t]->n1->isectFlag || tVec[t]->n2->isectFlag )
		{
			double a0, a1, a2;
			tVec[t]->computeCosAngles( &a0, &a1, &a2 );
			a0 = acos(a0)*180.0/PI;
			a1 = acos(a1)*180.0/PI;
			a2 = acos(a2)*180.0/PI;

			double minAng = a0;
			if ( a1 < minAng ) minAng = a1;
			if ( a2 < minAng ) minAng = a2;

			double maxAng = a0;
			if ( a1 > maxAng ) maxAng = a1;
			if ( a2 > maxAng ) maxAng = a2;

			double d01, d12, d20;
			d01 = dist_squared( tVec[t]->n0->pnt, tVec[t]->n1->pnt );
			d12 = dist_squared( tVec[t]->n1->pnt, tVec[t]->n2->pnt );
			d20 = dist_squared( tVec[t]->n2->pnt, tVec[t]->n0->pnt );

			if ( a0 < minTriAng && d12/(d20+d01) < minAspectRatio )
			{
				if ( delFlag )
					moveNode( tVec[t]->n1, tVec[t]->n2 );
				else
					tVec[t]->invalidFlag = 1;
			}
			else if ( a1 < minTriAng && d20/(d01+d12) < minAspectRatio )
			{
				if ( delFlag )
					moveNode( tVec[t]->n2, tVec[t]->n0 );
				else
					tVec[t]->invalidFlag = 1;
			}
			else if ( a2 < minTriAng && d01/(d12+d20) < minAspectRatio )
			{
				if ( delFlag )
					moveNode( tVec[t]->n0, tVec[t]->n1 );
				else
					tVec[t]->invalidFlag = 1;
			}
		}
	}
}

void TMesh::moveNode( TNode* n0, TNode* n1 )
{
	if ( n0->isectFlag && n1->isectFlag )
	{
		vec3d p = (n0->pnt + n1->pnt)*0.5;
		n0->pnt = p;
		n1->pnt = p;
	}
	else if ( n0->isectFlag )
	{
		n1->pnt = n0->pnt;
	}
	else if ( n1->isectFlag )
	{
		n0->pnt = n1->pnt;
	}
	else
	{
		vec3d p = (n0->pnt + n1->pnt)*0.5;
		n0->pnt = p;
		n1->pnt = p;
	}
}

void TMesh::findEdge( TNode* node, TTri* tri0, TTri* tri1 )
{
	TNode* t0n0; TNode* t0n1;
	TNode* t1n0; TNode* t1n1;

	t0n0 = t0n1 = t1n0 = t1n1 = NULL;

	if ( tri0->n0 == node )
	{
		t0n0 = tri0->n1;
		t0n1 = tri0->n2;
	}
	else if ( tri0->n1 == node )
	{
		t0n0 = tri0->n0;
		t0n1 = tri0->n2;
	}
	else if ( tri0->n2 == node )
	{
		t0n0 = tri0->n0;
		t0n1 = tri0->n1;
	}
	if ( tri1->n0 == node )
	{
		t1n0 = tri1->n1;
		t1n1 = tri1->n2;
	}
	else if ( tri1->n1 == node )
	{
		t1n0 = tri1->n0;
		t1n1 = tri1->n2;
	}
	else if ( tri0->n2 == node )
	{
		t1n0 = tri1->n0;
		t1n1 = tri1->n1;
	}

	if ( t0n0 == t1n0 || t0n0 == t1n1 )
		addEdge( tri0, tri1, node, t0n0 );
	else if ( t0n1 == t1n0 || t0n1 == t1n1 )
		addEdge( tri0, tri1, node, t0n1 );
}

void TMesh::addEdge( TTri* tri0, TTri* tri1, TNode* node0, TNode* node1 )
{
	if ( tri0->e0 && ( tri0->e0->tri0 == tri1 || tri0->e0->tri1 == tri1 ) )	// Check If Edge Already There
		return;

	if ( tri0->e1 && ( tri0->e1->tri0 == tri1 || tri0->e1->tri1 == tri1 ) )		
		return;

	if ( tri0->e2 && ( tri0->e2->tri0 == tri1 || tri0->e2->tri1 == tri1 ) )		
		return;

	//==== Create Edge ====//
	TEdge* edge = new TEdge();

	edge->n0 = node0;
	edge->n1 = node1;
	edge->tri0 = tri0;
	edge->tri1 = tri1;

	if ( !tri0->e0 )
		tri0->e0 = edge;
	else if ( !tri0->e1 )
		tri0->e1 = edge;
	else if ( !tri0->e2 )
		tri0->e2 = edge;

	if ( !tri1->e0 )
		tri1->e0 = edge;
	else if ( !tri1->e1 )
		tri1->e1 = edge;
	else if ( !tri1->e2 )
		tri1->e2 = edge;

	node0->edgeVec.push_back( edge );
	node1->edgeVec.push_back( edge );

	eVec.push_back( edge );

}

TNode* TMesh::lowNode( TNode* node )
{
	int i;

	TNode* lowN = node;

	if ( node->mergeVec.size() <= 1 )
		return lowN;

	//==== Find Lowest Node Ptr ====//
	for ( i = 0 ; i < (int)node->mergeVec.size() ; i++ )
	{
		if ( (long)(node->mergeVec[i]) < (long)lowN )
			lowN = node->mergeVec[i];
	}

	for ( i = 0 ; i < (int)node->mergeVec.size() ; i++ )
	{
		if ( node->mergeVec[i] != lowN )
			node->mergeVec[i]->id = -999;
	}
	return lowN;
}


void TMesh::swapEdge( TEdge* edge )
{
	if ( !edge->tri0 )
		return;
	if ( !edge->tri1 )
		return;

	TNode *n0, *n1, *n2, *n3;
	n0 = edge->n0;

	if ( edge->tri0->n0 != edge->n0 && edge->tri0->n0 != edge->n1 )
		n1 = edge->tri0->n0;
	else if ( edge->tri0->n1 != edge->n0 && edge->tri0->n1 != edge->n1 )
		n1 = edge->tri0->n1;
	else if ( edge->tri0->n2 != edge->n0 && edge->tri0->n2 != edge->n1 )
		n1 = edge->tri0->n2;

	n2 = edge->n1;

	if ( edge->tri1->n0 != edge->n0 && edge->tri1->n0 != edge->n1 )
		n3 = edge->tri1->n0;
	else if ( edge->tri1->n1 != edge->n0 && edge->tri1->n1 != edge->n1 )
		n3 = edge->tri1->n1;
	else if ( edge->tri1->n2 != edge->n0 && edge->tri1->n2 != edge->n1 )
		n3 = edge->tri1->n2;

	edge->tri0->n0 = n0;
	edge->tri0->n1 = n1;
	edge->tri0->n2 = n3;

	edge->tri0->norm = edge->tri1->norm;

	edge->tri1->n0 = n1;
	edge->tri1->n1 = n2;
	edge->tri1->n2 = n3;

}

void TMesh::matchNodes()
{
	int i, n, t;

	double tol = 1.0e-12;
	double sqtol = sqrt(tol);

	for ( n = 0 ; n < (int)nVec.size() ; n++ )
		nVec[n]->mergeVec.clear();

	NBndBox nBox;	
	for ( n = 0 ; n < (int)nVec.size() ; n++ )
	{
		nBox.addNode( nVec[n] );
	}
	nBox.splitBox(sqrt(tol));

	//==== Find All Leaves of Oct Tree ====//
	vector< NBndBox* > leafVec;
	nBox.addLeafNodes( leafVec );

	for ( i = 0 ; i < (int)leafVec.size() ; i++ )
	{
		leafVec[i]->box.expand( sqtol );
	}

	for ( n = 0 ; n < (int)nVec.size() ; n++ )
	{
		for ( i = 0 ; i < (int)leafVec.size() ; i++ )
		{
			if ( leafVec[i]->box.check_pnt( nVec[n]->pnt.x(), nVec[n]->pnt.y(), nVec[n]->pnt.z() ) )
			{
				for ( int m = 0 ; m < (int)leafVec[i]->nodeVec.size() ; m++ )
				{
					if ( dist_squared( nVec[n]->pnt, leafVec[i]->nodeVec[m]->pnt ) < tol )
					{
						nVec[n]->mergeVec.push_back( leafVec[i]->nodeVec[m] );
					}
				}
			}
		}
	}

/*
	for ( i = 0 ; i < leafVec.size() ; i++ )
	{
		for ( n = 0 ; n < leafVec[i]->nodeVec.size() ; n++ )
		{
			leafVec[i]->nodeVec[n]->mergeVec.push_back( leafVec[i]->nodeVec[n] );
			for ( int m = n+1 ; m < leafVec[i]->nodeVec.size() ; m++ )
			{
				if ( dist_squared( leafVec[i]->nodeVec[n]->pnt, leafVec[i]->nodeVec[m]->pnt ) < tol )
				{
					leafVec[i]->nodeVec[n]->mergeVec.push_back( leafVec[i]->nodeVec[m] );
					leafVec[i]->nodeVec[m]->mergeVec.push_back( leafVec[i]->nodeVec[n] );
				}
			}
		}
	}
*/

	//==== Go Thru All Tri And Refernce Lowest Node Ptr and Add Tag Other for Deletion ====//
	for ( t = 0 ; t < (int)tVec.size() ; t++ )
	{
		tVec[t]->n0 = lowNode( tVec[t]->n0 );
		tVec[t]->n1 = lowNode( tVec[t]->n1 );
		tVec[t]->n2 = lowNode( tVec[t]->n2 );
	}

	//==== Nuke Degenerate Tris ====//
	vector< TTri* > tempTVec;
	for ( t = 0 ; t < (int)tVec.size() ; t++ )
	{
		if ( tVec[t]->n0 != tVec[t]->n1 && 
			 tVec[t]->n0 != tVec[t]->n2 &&
			 tVec[t]->n1 != tVec[t]->n2 )
		  tempTVec.push_back( tVec[t] );
	}
	tVec = tempTVec;

	for ( t = 0 ; t < (int)tVec.size() ; t++ )
	{
		tVec[t]->n0->triVec.push_back( tVec[t] );
		tVec[t]->n1->triVec.push_back( tVec[t] );
		tVec[t]->n2->triVec.push_back( tVec[t] );
	if ( tVec[t]->n0->id == -999 || tVec[t]->n1->id == -999 || tVec[t]->n2->id == -999 )
		printf("Found -999\n");
	}

	//==== Nuke Redundant Nodes And Update NVec ====//
	vector< TNode* > tempNVec;
	for ( n = 0 ; n < (int)nVec.size() ; n++ )
	{
		nVec[n]->triVec.clear();
		nVec[n]->mergeVec.clear();
		if ( nVec[n]->id == -999 )
			delete nVec[n];
		else
			tempNVec.push_back( nVec[n] );
	}
	nVec = tempNVec;


	//==== Brute Force Check For Duplicate Nodes ====//
/****************************************************************
	for ( n = 0 ; n < nVec.size() ; n++ )
	{
		for ( int m = n+1 ; m < nVec.size() ; m++ )
		{
			if ( dist_squared( nVec[n]->pnt, nVec[m]->pnt ) < tol )
			{
				printf("Duplicate Node %d %d\n", nVec[n], nVec[m] );
			}
		}
	}
*****************************************************************/

	//==== Reassign Triangles ====//
	for ( t = 0 ; t < (int)tVec.size() ; t++ )
	{
		tVec[t]->n0->triVec.push_back( tVec[t] );
		tVec[t]->n1->triVec.push_back( tVec[t] );
		tVec[t]->n2->triVec.push_back( tVec[t] );
	}
}

void TMesh::swapEdges( double ang )
{
	int t, ot;

	//==== Remove Really Thin Triangles ====//
	for ( t = 0 ; t < (int)tVec.size() ; t++ )
	{

		double a0, a1, a2;
		tVec[t]->computeCosAngles( &a0, &a1, &a2 );

		a0 = acos(a0)*180.0/PI;
		a1 = acos(a1)*180.0/PI;
		a2 = acos(a2)*180.0/PI;

		TEdge* edge = 0;
		if ( a0 > ang )
		{
			edge = new TEdge();
			edge->n0 = tVec[t]->n1;
			edge->n1 = tVec[t]->n2;
		}
		else if ( a1 > ang  )
		{
			edge = new TEdge();
			edge->n0 = tVec[t]->n0;
			edge->n1 = tVec[t]->n2;
		}
		else if ( a2 > ang  )
		{
			edge = new TEdge();
			edge->n0 = tVec[t]->n0;
			edge->n1 = tVec[t]->n1;
		}

		if ( edge )
		{
			edge->tri0 = tVec[t];
			edge->tri1 = 0;

			//==== Find Other Tri ====//
			for ( ot = 0 ; ot < (int)edge->n0->triVec.size() ; ot++ )
			{
				TTri* tri = edge->n0->triVec[ot];
				if ( tri != edge->tri0 )
				{
					int match = 0;
					if      ( tri->n0 == edge->n0 ) match++;
					else if ( tri->n1 == edge->n0 ) match++;
					else if ( tri->n2 == edge->n0 ) match++;
					if      ( tri->n0 == edge->n1 ) match++;
					else if ( tri->n1 == edge->n1 ) match++;
					else if ( tri->n2 == edge->n1 ) match++;

					if ( match == 2 )
					{
						edge->tri1 = tri;
					}
				}
				if ( edge->tri1 )
					break;
			}
			//==== Swap It ====//
			if ( edge->tri1 )
			{
				swapEdge( edge );
			}
		}
	}
}


void TMesh::checkValid(FILE* fid)
{
	int n, s, t;

	//==== Clear Refs to Tris ====//
	for ( n = 0 ; n < (int)nVec.size() ; n++ )
	{
		nVec[n]->triVec.clear();
	}
	
	//==== Check If All Tris Have 3 Edges ====//
	for ( t = 0 ; t < (int)tVec.size() ; t++ )
	{
		tVec[t]->n0->triVec.push_back( tVec[t] );
		tVec[t]->n1->triVec.push_back( tVec[t] );
		tVec[t]->n2->triVec.push_back( tVec[t] );
		tVec[t]->e0 = 0;
		tVec[t]->e1 = 0;
		tVec[t]->e2 = 0;
	}

	//==== Create Edges For Adjacent Tris ====//
	for ( n = 0 ; n < (int)nVec.size() ; n++ )
	{
		TNode* n0 = nVec[n];
		for ( t = 0 ; t < (int)n0->triVec.size() ; t++ )
		{
			for ( s = t+1 ; s < (int)n0->triVec.size() ; s++ )
			{
				findEdge( n0, n0->triVec[t], n0->triVec[s] );
			}
		}
	}

	//==== Check If All Tris Have 3 Edges ====//
	vector< TTri* > ivTriVec;
	for ( t = 0 ; t < (int)tVec.size() ; t++ )
	{
		if ( !tVec[t]->e0 || !tVec[t]->e1 || !tVec[t]->e2 )
		{
			tVec[t]->invalidFlag = 1;
			ivTriVec.push_back( tVec[t] );
		}	
	}

/**************************
	for ( it = 0 ; it < ivTriVec.size() ; it++ )
	{
	
		TTri* t0 = findTriPnts( ivTriVec[it], ivTriVec[it]->n0,  ivTriVec[it]->n1 );
		TTri* t1 = findTriPnts( ivTriVec[it], ivTriVec[it]->n1,  ivTriVec[it]->n2 );
		TTri* t2 = findTriPnts( ivTriVec[it], ivTriVec[it]->n2,  ivTriVec[it]->n0 );

		printf( "Invalid Tri: %d \n", ivTriVec[it] );
		printf(	"	Tris: %d %d %d\n", t0, t1, t2 );
		printf(	"	Edges: %d %d %d\n", ivTriVec[it]->e0, ivTriVec[it]->e1, ivTriVec[it]->e2 );
		printf(	"	Nodes: %d %d %d\n", ivTriVec[it]->n0, ivTriVec[it]->n1, ivTriVec[it]->n2 );
	}
**************************/

	if ( fid )
	{
		if ( ivTriVec.size() == 0 )
			fprintf( fid, "Mesh IS WaterTight\n" );
		else
			fprintf( fid, "There are %d Invalid Triangles\n", ivTriVec.size() );
	}

}


void TMesh::meshStats(double* minEdgeLen, double* minTriAng, double* maxTriAng )
{
	int t;
	double d, a0, a1, a2;

	double minLenSq = 1.0e06;
	for ( t = 0 ; t < (int)tVec.size() ; t++ )
	{
		d = dist_squared( tVec[t]->n0->pnt, tVec[t]->n1->pnt );
		if ( d < minLenSq )
			minLenSq = d;

		d = dist_squared( tVec[t]->n1->pnt, tVec[t]->n2->pnt );
		if ( d < minLenSq )
			minLenSq = d;

		d = dist_squared( tVec[t]->n2->pnt, tVec[t]->n0->pnt );
		if ( d < minLenSq )
			minLenSq = d;
	}
	*minEdgeLen = sqrt(minLenSq);

//	printf("Mesh Stats:\n");
//	printf("  Num Tris = %d\n", tVec.size() );
//	printf("  Min Edge Length = %f\n", *minEdgeLen );

	int minTri = 0;
	int maxTri = 0;
	double minAng = 1.0e06;
	double maxAng = -1.0e06;
	for ( t = 0 ; t < (int)tVec.size() ; t++ )
	{
		tVec[t]->computeCosAngles( &a0, &a1, &a2 );

		a0 = acos(a0)*180.0/PI;
		a1 = acos(a1)*180.0/PI;
		a2 = acos(a2)*180.0/PI;

		if ( a0 < minAng )	{	minAng = a0;	minTri = t; }
		if ( a0 > maxAng )	{   maxAng = a0;	maxTri = t; }
		if ( a1 < minAng )	{	minAng = a1;	minTri = t; }
		if ( a1 > maxAng )	{   maxAng = a1;	maxTri = t; }
		if ( a2 < minAng )	{	minAng = a2;	minTri = t; }
		if ( a2 > maxAng )	{   maxAng = a2;	maxTri = t; }

	}

	*minTriAng = minAng;
	*maxTriAng = maxAng;
		
//	printf("  Min Angle = %f\n", minAng);
//	printf("  Max Angle = %f\n", maxAng);
}

TTri* TMesh::findTriNodes( TTri* ignoreTri, TNode* n0, TNode* n1 )
{
	int t;
	for ( t = 0 ; t < (int)tVec.size() ; t++ )
	{
		if ( tVec[t] != ignoreTri )
		{
			if ( tVec[t]->n0 == n0 && tVec[t]->n1 == n1 )
				return tVec[t];
			if ( tVec[t]->n1 == n0 && tVec[t]->n0 == n1 )
				return tVec[t];
			if ( tVec[t]->n1 == n0 && tVec[t]->n2 == n1 )
				return tVec[t];
			if ( tVec[t]->n2 == n0 && tVec[t]->n1 == n1 )
				return tVec[t];
			if ( tVec[t]->n2 == n0 && tVec[t]->n0 == n1 )
				return tVec[t];
			if ( tVec[t]->n0 == n0 && tVec[t]->n2 == n1 )
				return tVec[t];
		}
	}
	return 0;
}

TTri* TMesh::findTriPnts( TTri* ignoreTri, TNode* n0, TNode* n1 )
{
	int t;
	double tol = 0.0000001;
	vec3d p0 = n0->pnt;
	vec3d p1 = n1->pnt;

	for ( t = 0 ; t < (int)tVec.size() ; t++ )
	{
		if ( tVec[t] != ignoreTri )
		{
			vec3d pA = tVec[t]->n0->pnt;
			vec3d pB = tVec[t]->n1->pnt;
			vec3d pC = tVec[t]->n2->pnt;

			if ( dist_squared( p0, pA ) < tol &&   dist_squared( p1, pB ) < tol )
				return tVec[t];
			if ( dist_squared( p0, pB ) < tol &&   dist_squared( p1, pA ) < tol )
				return tVec[t];
			if ( dist_squared( p0, pB ) < tol &&   dist_squared( p1, pC ) < tol )
				return tVec[t];
			if ( dist_squared( p0, pC ) < tol &&   dist_squared( p1, pB ) < tol )
				return tVec[t];
			if ( dist_squared( p0, pC ) < tol &&   dist_squared( p1, pA ) < tol )
				return tVec[t];
			if ( dist_squared( p0, pA ) < tol &&   dist_squared( p1, pC ) < tol )
				return tVec[t];
		}
	}
	return 0;
}

void TMesh::relaxMesh( vector<TMesh*> & origTMeshVec )
{
	int n, t, e;
	TNode* node[3];
	TEdge* edge[3];

	//==== Save Edges of Intersection Curves  =====//
//	vector< vec3d > isectPairs;
	isectPairs.clear();

	for ( n = 0 ; n < (int)nVec.size() ; n++ )
	{
		if ( nVec[n]->isectFlag == 1 )
		{
			for ( t = 0 ; t < (int)nVec[n]->triVec.size() ; t++ )
			{
				if ( nVec[n]->triVec[t]->n0->isectFlag == 1 && 	nVec[n]->triVec[t]->n0 != nVec[n] )
				{
					isectPairs.push_back( nVec[n]->pnt );
					isectPairs.push_back( nVec[n]->triVec[t]->n0->pnt );
				}
				if ( nVec[n]->triVec[t]->n1->isectFlag == 1 && 	nVec[n]->triVec[t]->n0 != nVec[n] )
				{
					isectPairs.push_back( nVec[n]->pnt );
					isectPairs.push_back( nVec[n]->triVec[t]->n1->pnt );
				}
				if ( nVec[n]->triVec[t]->n2->isectFlag == 1 && 	nVec[n]->triVec[t]->n0 != nVec[n] )
				{
					isectPairs.push_back( nVec[n]->pnt );
					isectPairs.push_back( nVec[n]->triVec[t]->n2->pnt );
				}
			}
		}
	}

	//==== Copy This Mesh To Project Relaxed Points On To =====//
	TMesh* saveMesh = new TMesh;
	for ( t = 0 ; t < (int)tVec.size() ; t++ )
	{
		saveMesh->addTri( tVec[t]->n0, tVec[t]->n1, tVec[t]->n2, tVec[t]->norm );
	}
	saveMesh->loadBndBox();

	//==== Load Current Edges in Nodes ====//
	for ( n = 0 ; n < (int)nVec.size() ; n++ )
	{
		nVec[n]->edgeVec.clear();
	}

	for ( t = 0 ; t < (int)tVec.size() ; t++ )
	{
		node[0] = tVec[t]->n0;	node[1] = tVec[t]->n1;	node[2] = tVec[t]->n2;
		edge[0] = tVec[t]->e0;	edge[1] = tVec[t]->e1;	edge[2] = tVec[t]->e2;

		for ( n = 0 ; n < 3 ; n++ )
		{
			for ( e = 0 ; e < 3 ; e++ )
			{
				if ( edge[e]->n0 == node[n] )
					node[n]->edgeVec.push_back( edge[e] );
				else if ( edge[e]->n1 == node[n] )
					node[n]->edgeVec.push_back( edge[e] );
			}
		}
	}

	//==== Create An Offset Vector for Each Node ====//
for ( int i = 0 ; i < 10 ; i++ )
{
	vector< vec3d > offVec;
	for ( n = 0 ; n < (int)nVec.size() ; n++ )
	{
		//==== Sum Up Offset Vectors ====//
		vec3d off;
		for ( e = 0 ; e < (int)nVec[n]->edgeVec.size() ; e++ )
		{
			if ( nVec[n]->edgeVec[e]->n0 == nVec[n] )
				off = off + (nVec[n]->edgeVec[e]->n1->pnt - nVec[n]->pnt);
			else
				off = off + (nVec[n]->edgeVec[e]->n0->pnt - nVec[n]->pnt);
		}
		offVec.push_back( off );

		//==== Find Normal ====//
		nVec[n]->norm.set_xyz(0,0,0);
		for ( t = 0 ; t < (int)nVec[n]->triVec.size() ; t++ )
		{
			nVec[n]->norm = nVec[n]->norm + nVec[n]->triVec[t]->compNorm();
		}
		nVec[n]->norm.normalize();
	}

	//==== Only Move Node Attached to Isect Points ====//
	for ( t = 0 ; t < (int)tVec.size() ; t++ )
	{
		if ( tVec[t]->n0->isectFlag == 1 || tVec[t]->n1->isectFlag == 1 || tVec[t]->n2->isectFlag == 1 )
		{
			if ( tVec[t]->n0->isectFlag == 0 )
				tVec[t]->n0->isectFlag = 2;
			if ( tVec[t]->n1->isectFlag == 0 )
				tVec[t]->n1->isectFlag = 2;
			if ( tVec[t]->n2->isectFlag == 0 )
				tVec[t]->n2->isectFlag = 2;
		}
	}


	//==== Move Each Node ====//
	double moveFract = 0.001;
	for ( n = 0 ; n < (int)nVec.size() ; n++ )
	{
		if ( nVec[n]->isectFlag == 1 )
		{
			vec3d offPnt = nVec[n]->pnt + offVec[n]*moveFract;

			double moveDist = offVec[n].mag()*moveFract;
			vec3d segPnt1 = offPnt + nVec[n]->norm*moveDist;
			vec3d segPnt2 = offPnt - nVec[n]->norm*moveDist;

			//==== Put the Point Back on the Surface =====//
			vector< vec3d > ipVec;

			if ( nVec[n]->isectFlag == 2 )
			{
				saveMesh->tBox.segIntersect( segPnt1, segPnt2, ipVec );
				if ( ipVec.size() )
					nVec[n]->pnt = ipVec[0];
			}
			else if ( nVec[n]->isectFlag == 1 )
			{
				nVec[n]->pnt = projectOnISectPairs( offPnt, isectPairs );
			}
		}
	}


}


}

vec3d TMesh::projectOnISectPairs( vec3d & offPnt, vector< vec3d > & pairVec )
{
	int i;
	double t;
	double closeDist = 1e06;
	vec3d closePnt;

	for ( i = 0 ; i < (int)pairVec.size() ; i+=2 )
	{
		double d = pointSegDistSquared( offPnt, pairVec[i], pairVec[i+1], &t );
		if ( d < closeDist )
		{
			closeDist = d;
			closePnt = pairVec[i] + (pairVec[i+1] - pairVec[i])*t;
		}

	}

	return closePnt;
}

void TMesh::mergeSplitNodes( vector< TTri* > & triVec, vector< TNode* > & nodeVec )
{
	int s, t;

	//==== Find All Exterior and Split Tris =====//
	for ( t = 0 ; t < (int)tVec.size() ; t++ )
	{
		TTri* tri = tVec[t];
		if ( tri->splitVec.size() )
		{
			for ( s = 0 ; s < (int)tri->splitVec.size() ; s++ )
			{
				if ( !tri->splitVec[s]->interiorFlag )
				{
					triVec.push_back( tri->splitVec[s] );
				}
			}
		}
		else if ( !tri->interiorFlag )
		{
			triVec.push_back( tri );
		}
	}

	//==== Look Thru nVecs and Redirect Dumplicates ====//
	for ( t = 0 ; t < (int)triVec.size() ; t++ )
	{
		triVec[t]->n0 = checkDupOrAdd( triVec[t]->n0, nodeVec );
		triVec[t]->n1 = checkDupOrAdd( triVec[t]->n1, nodeVec );
		triVec[t]->n2 = checkDupOrAdd( triVec[t]->n2, nodeVec );
	}

	//==== Remove Any Bogus Tris ====//
	vector< TTri* > goodTriVec;
	for ( t = 0 ; t < (int)triVec.size() ; t++ )
	{
		TTri* ttri = triVec[t];
		if ( ttri->n0->id != ttri->n1->id &&
			 ttri->n0->id != ttri->n2->id &&
			 ttri->n1->id != ttri->n2->id )
		{
			goodTriVec.push_back( ttri );
		}
	}
	triVec = goodTriVec;

}

//==== Check if Dupicate Node - if Not Add ====//
TNode* TMesh::checkDupOrAdd( TNode* node, vector< TNode* > & nodeVec, double tol )
{
	for ( int i = 0 ; i < (int)nodeVec.size() ; i++ )
	{
		TNode* n = nodeVec[i];
		if ( fabs( n->pnt.x() - node->pnt.x() ) < tol )
			if ( fabs( n->pnt.y() - node->pnt.y() ) < tol )
				if ( fabs( n->pnt.z() - node->pnt.z() ) < tol )
				{
					node->id = i;
					return n;
				}
	}

	//==== Add To List ====//
	node->id = nodeVec.size();
	nodeVec.push_back( node );
	return node;
}

//==== Check if Dupicate Node - if Not Add ====//
TNode* TMesh::checkDupOrCreate( vec3d & p, vector< TNode* > & nodeVec, double tol )
{
	for ( int i = 0 ; i < (int)nodeVec.size() ; i++ )
	{
		TNode* n = nodeVec[i];
		if ( fabs( n->pnt.x() - p.x() ) < tol )
			if ( fabs( n->pnt.y() - p.y() ) < tol )
				if ( fabs( n->pnt.z() - p.z() ) < tol )
				{
					return n;
				}
	}

	//==== Add To List ====//
	TNode* node = new TNode();
	node->pnt = p;
	node->id = nodeVec.size();
	nodeVec.push_back( node );
	return node;
}

double TMesh::rand01()
{
	double val = (double)rand()/(double)RAND_MAX;
	return val;
}

void TMesh::stressTest()
{
	srand ( 2 );
	
	for ( int i = 0 ; i < 10000000 ; i++ )
	{
		if ( i%10000 == 0 )
			printf("Stress Test Iter = %d\n", i );
		TTri* t0 = new TTri();
		t0->n0   = new TNode();
		t0->n1   = new TNode();
		t0->n2   = new TNode();
		TTri* t1 = new TTri();
		t1->n0   = new TNode();
		t1->n1   = new TNode();
		t1->n2   = new TNode();

		t0->n0->pnt = vec3d( rand01(), rand01(), rand01() );
		t0->n1->pnt = vec3d( rand01(), rand01(), rand01() );
		t0->n2->pnt = vec3d( rand01(), rand01(), rand01() );
		t1->n0->pnt = vec3d( rand01(), rand01(), rand01() );
		t1->n1->pnt = vec3d( rand01(), rand01(), rand01() );
		t1->n2->pnt = vec3d( rand01(), rand01(), rand01() );

		vec3d e0;
		vec3d e1;
		int coplanarFlag;
		int iflag = tri_tri_intersect_with_isectline( 
			t0->n0->pnt.v, t0->n1->pnt.v, t0->n2->pnt.v,
			t1->n0->pnt.v, t1->n1->pnt.v, t1->n2->pnt.v,
			&coplanarFlag, e0.v, e1.v ); 

		if ( iflag && !coplanarFlag )
		{
			TEdge* ie0 = new TEdge();
			ie0->n0 = new TNode();	ie0->n0->pnt = e0;	
			ie0->n1 = new TNode();	ie0->n1->pnt = e1;

			TEdge* ie1 = new TEdge();
			ie1->n0 = new TNode();	ie1->n0->pnt = e0;
			ie1->n1 = new TNode();	ie1->n1->pnt = e1;

			if ( dist( e0, e1 ) > 0.000001 )
			{
				t0->iSectEdgeVec.push_back( ie0 );
				t1->iSectEdgeVec.push_back( ie1 );
			}
		}
		t0->splitTri();
		t1->splitTri();

		delete t0->n0;
		delete t0->n1;
		delete t0->n2;
		delete t0;
		delete t1->n0;
		delete t1->n1;
		delete t1->n2;
		delete t1;

	}

	int done = 23;


}

