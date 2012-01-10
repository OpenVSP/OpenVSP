//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//

//******************************************************************************
//    
//    External Geometry Class
//  
// 
//   J.R. Gloudemans - 11/12/03
//
//    
//******************************************************************************

#include "meshGeom.h"
#include "aircraft.h"
#include "materialMgr.h"
#include "scriptMgr.h"
#include "vorGeom.h"
#include "CfdMeshMgr.h"

#ifdef __APPLE__
#  include <OpenGL/gl.h>
#else
#  include <GL/gl.h>
#endif
#include "defines.h"
#include "tritri.h"
#include "bbox.h"

//==== Constructor =====//
MeshGeom::MeshGeom(Aircraft* aptr) : Geom(aptr)
{
  type = MESH_GEOM_TYPE;
  type_str = Stringc("mesh");
  meshType = INTERSECTION_MESH;
  fileName = Stringc("");

  char name[255];
  sprintf( name, "Mesh_%d", geomCnt ); 
  geomCnt++;
  name_str = Stringc(name);
  id_str = name_str;				//jrg fix
  setSymCode(NO_SYM);

  numPnts.deactivate();
  numXsecs.deactivate();
  origin.deactivate();

  bnd_box.update( vec3d(0,0,0 ) ); 

  //==== Test Big/Little Endian ====//
  bigEndianFlag = 0;
  int i = 0x1000000;
  unsigned char* byte = (unsigned char*) &i;
  if ( *byte )
	  bigEndianFlag = 1;
  else
	  bigEndianFlag = 0;

  totalTheoArea = totalWetArea = 0.0;
  totalTheoVol  = totalWetVol  = 0.0;

  massPropFlag = 0;
  centerOfGrav = vec3d(0,0,0);
  totalMass = 0.0;
  totalIxx = 0.0;
  totalIyy = 0.0;
  totalIzz = 0.0;
  totalIxy = 0.0;
  totalIxz = 0.0;
  totalIyz = 0.0;

   minTriDen = 0.0;
   maxTriDen = 1.0;

   meshFlag = 0;
   oneMesh  = 0;

   lastScaleFactor = 1.0;

}

//==== Destructor =====//
MeshGeom::~MeshGeom()
{
	int i;
	for ( i = 0 ; i < (int)tMeshVec.size() ; i++ )
	{
		delete tMeshVec[i];
	}

	for ( i = 0 ; i < (int)sliceVec.size() ; i++ )
	{
		delete sliceVec[i];
	}

}

void MeshGeom::copy( Geom* fromGeom )
{
	int i;
	Geom::copy( fromGeom );						// Copy Base Stuff

	if ( fromGeom->getType() != MESH_GEOM_TYPE )
		return;

	MeshGeom* g = (MeshGeom*)fromGeom;

	//==== Delete All Meshes
	for ( i = 0 ; i < (int)tMeshVec.size() ; i++ )
		delete tMeshVec[i];
	tMeshVec.clear();

	for ( i = 0 ; i < (int)sliceVec.size() ; i++ )
		delete sliceVec[i];
	sliceVec.clear();

	//==== Copy in New Ones ====//
	tMeshVec.resize( (int)g->tMeshVec.size() );
	for ( i = 0 ; i < (int)g->tMeshVec.size() ; i++ )
	{
		tMeshVec[i] = new TMesh();
		tMeshVec[i]->copy( g->tMeshVec[i] );
	}

	sliceVec.resize( g->sliceVec.size() );
	for ( i = 0 ; i < (int)g->sliceVec.size() ; i++ )
	{
		sliceVec[i] = new TMesh();
		sliceVec[i]->copy( g->sliceVec[i] );
	}

	meshType = g->meshType;

	compose_model_matrix();
	update_bbox();

	generate();

}


//==== Generate Fuse Component ====//
void MeshGeom::generate()
{
}

void MeshGeom::computeCenter()
{
	//==== Set Rotation Center ====//
//	center.set_x( origin()*length()*scaleFactor() ); 
}

//==== Parm Has Changed ReGenerate Fuse Component ====//
void MeshGeom::parm_changed(Parm* p)
{

	if ( p->get_update_grp() == UPD_XFORM )
	{
		if ( p == &xLoc || p == &yLoc || p == &zLoc || 
			 p == &xRot || p == &yRot || p == &zRot )
			updateAttach(1);
		else
			updateAttach(0);
	}
	if ( p == &scaleFactor )	
	{
		scale();
	}

	generate();
	updateAttach(0);

	compose_model_matrix();

	//==== Tell Aircraft You Have Changed ====//
	airPtr->geomMod( this );
}

int MeshGeom::read_xsec( const char* file_name )
{
	int i, j, c;
	FILE *fp;
	char str[256];

	//==== Make Sure File Exists ====//
	if ( (fp = fopen(file_name, "r")) == (FILE *)NULL )
	{
		return 0;
	}

    //==== Read first Line of file and compare against expected header ====//
    fscanf(fp,"%s INPUT FILE\n\n",str);
	if ( strcmp("HERMITE",str) != 0 )  
	{
		return 0;
	}
     //==== Read in number of components ====//
	int num_comps;
	fscanf(fp," NUMBER OF COMPONENTS = %d\n",&num_comps);

	if ( num_comps <= 0 )
		return 0;

	vector< HrmComp > compVec;
	compVec.resize(num_comps);

	for ( c = 0 ; c < num_comps ; c++ )
	{
		char name_str[256];
		float x, y, z;
		int group_num, type;
		int num_pnts, num_cross;

		fgets( name_str, 256, fp );
		fscanf(fp," GROUP NUMBER = %d\n",&group_num);
		fscanf(fp," TYPE = %d\n",&type);
		fscanf(fp," CROSS SECTIONS = %d\n",&(num_cross));
		fscanf(fp," PTS/CROSS SECTION = %d\n",&(num_pnts));

		//==== Set Component Values ====//
		compVec[c].name_str = name_str;
		compVec[c].groupNum = group_num;
		compVec[c].type     = type;
		compVec[c].numCross = num_cross;
		compVec[c].numPnts  = num_pnts;

		//===== Size Cross Vec ====//
		compVec[c].crossVec.resize( num_cross );
		for ( i = 0 ; i < num_cross ; i++)
		{
			compVec[c].crossVec[i].resize( num_pnts );
			for ( j = 0 ; j < num_pnts ; j++)
			{
				fscanf( fp, "%f %f %f\n", &x, &y, &z );
				compVec[c].crossVec[i][j] = vec3d(x,y,z);
			}
		}
	}
	fclose(fp);

	//==== Convert CrossSections to Triangles ====//
	TMesh*  tMesh = new TMesh();

	for ( c = 0 ; c < num_comps ; c++ )
	{
		for ( int i = 1 ; i < (int)compVec[c].crossVec.size() ; i++ )
			for ( int j = 1 ; j < (int)compVec[c].crossVec[i].size() ; j++ )
			{
				AddTri( tMesh, compVec[c].crossVec[i-1][j-1], 
						compVec[c].crossVec[i][j-1], compVec[c].crossVec[i][j] );

				AddTri( tMesh, compVec[c].crossVec[i-1][j-1], 
						compVec[c].crossVec[i][j], compVec[c].crossVec[i-1][j] );
			}
	}

	tMeshVec.push_back( tMesh );
	update_bbox();


	return 1;
}

void MeshGeom::AddTri( TMesh* tMesh, vec3d & p0, vec3d & p1, vec3d & p2 )
{
	double dist_tol = 1.0e-12;

	vec3d v01 = p1 - p0;
	vec3d v02 = p2 - p0;
	vec3d v12 = p2 - p1;

	if ( v01.mag() < dist_tol || v02.mag() < dist_tol || v12.mag() < dist_tol )
		return;

	vec3d norm = cross( v01, v02 );
	norm.normalize();
	
	tMesh->addTri( p0, p1, p2, norm );
}



int MeshGeom::read_stl( const char* file_name )
{ 
	fileName = file_name;

	FILE* file_id = fopen(file_name, "r");

	char str[256];
	float nx, ny, nz;
	float v0[3];
	float v1[3];
	float v2[3];
	TTri*  tPtr;
	TMesh*  tMesh = new TMesh();

	if ( file_id )
	{
		//==== Cheesy ASCII/Binary Test ====//
		int binaryFlag = 0;
		while ( fgets( str, 255, file_id ) )
		{
			for ( int i = 0 ; i < (int)strlen(str) ; i++ )
			{
				if ( (unsigned char)str[i] > 127 )
				{
					binaryFlag = 1;
					break;
				}
			}
		}

		rewind( file_id );

		if ( !binaryFlag )
		{
			fgets( str, 255, file_id );

			int stopFlag = 0;

			while ( !stopFlag )
			{
				if ( EOF == fscanf(file_id, "%*s %*s %f %f %f\n", &nx, &ny, &nz ) )
					break;

				if ( EOF == fscanf(file_id, "%*s %*s") )
					break;

				if ( EOF == fscanf(file_id, "%*s %f %f %f\n", &v0[0], &v0[1], &v0[2] ) )
					break;
				if ( EOF == fscanf(file_id, "%*s %f %f %f\n", &v1[0], &v1[1], &v1[2] ) )
					break;
				if ( EOF == fscanf(file_id, "%*s %f %f %f\n", &v2[0], &v2[1], &v2[2] ) )
					break;
				if ( EOF == fscanf(file_id, "%*s") )
					break;
				if ( EOF == fscanf(file_id, "%*s") )
					break;

				//==== Add Valid Facet ====//
				tPtr = new TTri();
				tPtr->interiorFlag = 0;
				tPtr->norm = vec3d( nx, ny, nz );
				tMesh->tVec.push_back( tPtr );

				//==== Put Nodes Into Tri ====//
				tPtr->n0 = new TNode();
				tPtr->n1 = new TNode();;
				tPtr->n2 = new TNode();
				tPtr->n0->pnt = vec3d( v0[0], v0[1], v0[2] );
				tPtr->n1->pnt = vec3d( v1[0], v1[1], v1[2] );
				tPtr->n2->pnt = vec3d( v2[0], v2[1], v2[2] );
				tMesh->nVec.push_back( tPtr->n0 );
				tMesh->nVec.push_back( tPtr->n1 );
				tMesh->nVec.push_back( tPtr->n2 );

			}
		}
		else
		{
			fclose( file_id );
			file_id = fopen(file_name, "rb");			// Reopen as Binary

			//==== Header ====//			
			fread( str, sizeof(char), 80, file_id);
			
			int numFacet = readBinInt( file_id );

			for ( int i = 0 ; i < numFacet ; i++ )
			{
				nx = readBinFloat( file_id );
				ny = readBinFloat( file_id );
				nz = readBinFloat( file_id );

				v0[0] = readBinFloat( file_id );
				v0[1] = readBinFloat( file_id );
				v0[2] = readBinFloat( file_id );
				v1[0] = readBinFloat( file_id );
				v1[1] = readBinFloat( file_id );
				v1[2] = readBinFloat( file_id );
				v2[0] = readBinFloat( file_id );
				v2[1] = readBinFloat( file_id );
				v2[2] = readBinFloat( file_id );

				//==== Padding ====//
				fgetc(file_id);
				fgetc(file_id);

				//==== Add Valid Facet ====//
				tPtr = new TTri();
				tPtr->interiorFlag = 0;
				tPtr->norm = vec3d( nx, ny, nz );
				tMesh->tVec.push_back( tPtr );

				//==== Put Nodes Into Tri ====//
				tPtr->n0 = new TNode();
				tPtr->n1 = new TNode();;
				tPtr->n2 = new TNode();
				tPtr->n0->pnt = vec3d( v0[0], v0[1], v0[2] );
				tPtr->n1->pnt = vec3d( v1[0], v1[1], v1[2] );
				tPtr->n2->pnt = vec3d( v2[0], v2[1], v2[2] );

			}
		}
		fclose(file_id);
	}


	if ( tMesh->tVec.size() == 0 )
	{
		delete tMesh;
		return 0;
	}

	tMeshVec.push_back( tMesh );
	update_bbox();

	return 1;

}

//==== Write Fuse File ====//
float MeshGeom::readBinFloat( FILE* fptr )
{
  float fval = 0x00;
  fread(&fval,4,1,fptr);

  if ( bigEndianFlag )
  {
	  char swap[4];
      unsigned char *cptr = (unsigned char *)&fval;
	  swap[0] = cptr[3];
	  swap[1] = cptr[2];
	  swap[2] = cptr[1];

	  cptr[3] = cptr[0];
      cptr[2] = swap[2];
	  cptr[1] = swap[1];
	  cptr[0] = swap[0];
  }
  return fval;
}

int MeshGeom::readBinInt( FILE* fptr )
{
  int ival = 0x00;
  fread(&ival,4,1,fptr);

  if ( bigEndianFlag )
  {
	  char swap[4];
      unsigned char *cptr = (unsigned char *)&ival;
	  swap[0] = cptr[3];
	  swap[1] = cptr[2];
	  swap[2] = cptr[1];

	  cptr[3] = cptr[0];
      cptr[2] = swap[2];
	  cptr[1] = swap[1];
	  cptr[0] = swap[0];
  }
  return ival;
}

//==== Write STL File ====//
void MeshGeom::write_stl_file(FILE* file_id )
{
	int m;

	for ( m = 0 ; m < (int)tMeshVec.size() ; m++ )
	{
		tMeshVec[m]->write_stl_tris(sym_code, model_mat, reflect_mat, file_id );
	}
}

int MeshGeom::read_nascart( const char* file_name )
{
	int i;
	FILE* file_id = fopen(file_name, "r");

	if ( !file_id )
		return 0;

	TMesh*  tMesh = new TMesh();

	//==== Read Number Tris and Nodes ====//
	float x, y, z;
	int n0, n1, n2;
	float col;
	int num_tris, num_nodes;

	fscanf(file_id, "%d", &num_nodes );
	fscanf(file_id, "%d", &num_tris  );

	vec3d p;
	vector< vec3d > pVec;
	for ( i = 0 ; i < num_nodes ; i++ )
	{
		fscanf( file_id, "%f %f %f", &x, &y, &z );
		p.set_xyz(x, -z, y);
		pVec.push_back(p);
	}

	for ( i = 0 ; i < num_tris ; i++ )
	{
		fscanf( file_id, "%d %d %d %f", &n0, &n2, &n1, &col );

		//==== Compute Normal ====//
		vec3d p10 = pVec[n1-1] - pVec[n0-1];
		vec3d p20 = pVec[n2-1] - pVec[n0-1];
		vec3d norm = cross( p10, p20 );
		norm.normalize();

		//==== Add Valid Facet ====//
		tMesh->addTri( pVec[n0-1], pVec[n1-1], pVec[n2-1], norm );
	}

	fclose( file_id );

	if ( tMesh->tVec.size() == 0 )
	{
		delete tMesh;
		return 0;
	}

	tMeshVec.push_back( tMesh );

	update_bbox();


	return 1;

}

//==== Build NASCART Mesh and Save ====//
void MeshGeom::buildNascartMesh(int partOffset)
{
	int m, s, t;

	nascartTriVec.clear();
	nascartNodeVec.clear();

	//==== Find All Exterior and Split Tris =====//
	for ( m = 0 ; m < (int)tMeshVec.size() ; m++ )
	{
		for ( t = 0 ; t < (int)tMeshVec[m]->tVec.size() ; t++ )
		{
			TTri* tri = tMeshVec[m]->tVec[t];
			if ( tri->splitVec.size() )
			{
				for ( s = 0 ; s < (int)tri->splitVec.size() ; s++ )
				{
					if ( !tri->splitVec[s]->interiorFlag )
					{
						tri->splitVec[s]->id = partOffset+m+1;
						nascartTriVec.push_back( tri->splitVec[s] );
					}
				}
			}
			else if ( !tri->interiorFlag )
			{
				tri->id = partOffset+m+1;
				nascartTriVec.push_back( tri );
			}
		}
	}

	//==== Collect All Points ====//
	vector< vec3d* > allPntVec;
	vector< TNode* > allNodeVec;
	for ( t = 0 ; t < (int)nascartTriVec.size() ; t++ )
	{
		nascartTriVec[t]->n0->id = (int)allPntVec.size();
		allPntVec.push_back( &nascartTriVec[t]->n0->pnt );
		allNodeVec.push_back( nascartTriVec[t]->n0 );
		nascartTriVec[t]->n1->id = (int)allPntVec.size();
		allPntVec.push_back( &nascartTriVec[t]->n1->pnt );
		allNodeVec.push_back( nascartTriVec[t]->n1 );
		nascartTriVec[t]->n2->id = (int)allPntVec.size();
		allPntVec.push_back( &nascartTriVec[t]->n2->pnt );
		allNodeVec.push_back( nascartTriVec[t]->n2 );
	}

	//==== Build Map ====//
	map< int, vector< int > > indMap;
	vector< int > pntShift;
	int numPnts = cfdMeshMgrPtr->BuildIndMap( allPntVec, indMap, pntShift );

	for ( t = 0 ; t < (int)nascartTriVec.size() ; t++ )
	{
		int i0 = cfdMeshMgrPtr->FindPntIndex( nascartTriVec[t]->n0->pnt, allPntVec, indMap );
		int i1 = cfdMeshMgrPtr->FindPntIndex( nascartTriVec[t]->n1->pnt, allPntVec, indMap );
		int i2 = cfdMeshMgrPtr->FindPntIndex( nascartTriVec[t]->n2->pnt, allPntVec, indMap );
		nascartTriVec[t]->n0->id = pntShift[i0];
		nascartTriVec[t]->n1->id = pntShift[i1];
		nascartTriVec[t]->n2->id = pntShift[i2];
	}

	////==== Look Thru nVecs and Redirect Dumplicates ====//
	//for ( t = 0 ; t < (int)nascartTriVec.size() ; t++ )
	//{
	//	checkDupOrAdd( nascartTriVec[t]->n0, nascartNodeVec );
	//	checkDupOrAdd( nascartTriVec[t]->n1, nascartNodeVec );
	//	checkDupOrAdd( nascartTriVec[t]->n2, nascartNodeVec );
	//}

	//==== Load Nodes ====//
	for ( int i = 0 ; i < (int)allNodeVec.size() ; i++ )
	{
		if ( pntShift[i] >= 0 )
			nascartNodeVec.push_back( allNodeVec[i] );
	}

	//==== Remove Any Bogus Tris ====//
	vector< TTri* > goodTriVec;

	//==== Write Out Tris ====//
	for ( t = 0 ; t < (int)nascartTriVec.size() ; t++ )
	{
		TTri* ttri = nascartTriVec[t];
		if ( ttri->n0->id != ttri->n1->id &&
			 ttri->n0->id != ttri->n2->id &&
			 ttri->n1->id != ttri->n2->id )
		{
			goodTriVec.push_back( ttri );
		}
	}
	nascartTriVec = goodTriVec;

}

void MeshGeom::writeNascartPnts( FILE* fp )
{
	//==== Write Out Nodes ====//
	for ( int i = 0 ; i < (int)nascartNodeVec.size() ; i++ )
	{
		TNode* tnode = nascartNodeVec[i];
		fprintf( fp, "%f %f %f\n", tnode->pnt.x(), tnode->pnt.z(), -tnode->pnt.y() );
	}
}

void MeshGeom::writeCart3DPnts( FILE* fp )
{
	//==== Write Out Nodes ====//
	for ( int i = 0 ; i < (int)nascartNodeVec.size() ; i++ )
	{
		TNode* tnode = nascartNodeVec[i];
		fprintf( fp, "%16.10g, %16.10g, %16.10g\n", tnode->pnt.x(), tnode->pnt.y(),  tnode->pnt.z() );
	}
}

int MeshGeom::writeGMshNodes( FILE* fp, int node_offset)
{
	for ( int i = 0 ; i < (int)nascartNodeVec.size() ; i++ )
	{
		TNode* tnode = nascartNodeVec[i];
		fprintf( fp, "%d %16.10f %16.10f %16.10f\n", i+node_offset+1,
			     tnode->pnt.x(), tnode->pnt.y(),  tnode->pnt.z() );
	}
	return node_offset + (int)nascartNodeVec.size();
}

int MeshGeom::writeNascartTris( FILE* fp, int off )
{
	//==== Write Out Tris ====//
	for ( int t = 0 ; t < (int)nascartTriVec.size() ; t++ )
	{
		TTri* ttri = nascartTriVec[t];
		fprintf( fp, "%d %d %d %d.0\n", ttri->n0->id+1+off,  ttri->n2->id+1+off, ttri->n1->id+1+off, ttri->id );
	}

	return (off + nascartNodeVec.size());
}

int MeshGeom::writeCart3DTris( FILE* fp, int off )
{
	//==== Write Out Tris ====//
	for ( int t = 0 ; t < (int)nascartTriVec.size() ; t++ )
	{
		TTri* ttri = nascartTriVec[t];
		fprintf( fp, "%d %d %d\n", ttri->n0->id+1+off,  ttri->n1->id+1+off, ttri->n2->id+1+off );
	}

	return (off + nascartNodeVec.size());
}

int MeshGeom::writeGMshTris( FILE* fp, int node_offset, int tri_offset )
{
	//==== Write Out Tris ====//
	for ( int t = 0 ; t < (int)nascartTriVec.size() ; t++ )
	{
		TTri* ttri = nascartTriVec[t];
		fprintf( fp, "%d 2 0 %d %d %d\n", t+tri_offset+1, 
			ttri->n0->id+1+node_offset,  ttri->n2->id+1+node_offset, ttri->n1->id+1+node_offset );
	}
	return (tri_offset + nascartTriVec.size());
}

int MeshGeom::writeNascartParts( FILE* fp, int off )
{
	//==== Find All Exterior and Split Tris =====//
	for ( int m = 0 ; m < (int)tMeshVec.size() ; m++ )
	{
		fprintf( fp, "%d.0  %s  0\n", off+m, tMeshVec[m]->name_str.get_char_star() );
	}
	return off + tMeshVec.size();
}

int MeshGeom::writeCart3DParts( FILE* fp, int off )
{
	//==== Write Component IDs for each Tri =====//
	for ( int t = 0 ; t < (int)nascartTriVec.size() ; t++ )
	{
		fprintf( fp, "%d \n", nascartTriVec[t]->id );
	}
	return 0;
}

//==== Check if Dupicate Node - if Not Add ====//
void MeshGeom::checkDupOrAdd( TNode* node, vector< TNode* > & nodeVec )
{
	double tol = 0.00000001;
	int dupFlag = 0;
	for ( int i = 0 ; i < (int)nodeVec.size() ; i++ )
	{
		TNode* n = nodeVec[i];
		if ( fabs( n->pnt.x() - node->pnt.x() ) < tol )
			if ( fabs( n->pnt.y() - node->pnt.y() ) < tol )
				if ( fabs( n->pnt.z() - node->pnt.z() ) < tol )
				{
					dupFlag = 1;
					node->id = i;
				}
		if ( dupFlag )
			break;
	}

	//==== Add To List ====//
	if ( !dupFlag )
	{
		node->id = nodeVec.size();
		nodeVec.push_back( node );
	}
}



//==== Write Fuse File ====//
void MeshGeom::write(FILE* file_id)
{
}

//==== Write External File ====//
void MeshGeom::write(xmlNodePtr root)
{
  xmlAddStringNode( root, "Type", "Mesh");

  //==== Write General Parms ====//
  xmlNodePtr gen_node = xmlNewChild( root, NULL, (const xmlChar *)"General_Parms", NULL );
  write_general_parms( gen_node );

  //==== Write Fuse Parms ====//
  xmlNodePtr mesh_node = xmlNewChild( root, NULL, (const xmlChar *)"Mesh_Parms", NULL );

  //===== Write Tris ====//
  if ( tMeshVec.size() )
  {
	char str[512];
	int num_tris = (int)tMeshVec[0]->tVec.size();
	xmlAddIntNode( mesh_node, "Num_Tris", num_tris );
	xmlNodePtr tri_list_node = xmlNewChild( mesh_node, NULL, (const xmlChar *)"Tri_List", NULL );

	for ( int i = 0 ; i < (int)num_tris ; i++ )
	{
		TTri* t = tMeshVec[0]->tVec[i];
		sprintf( str, "%f, %f, %f, %f, %f, %f, %f, %f, %f, %f, %f, %f", 
			t->n0->pnt[0], t->n0->pnt[1], t->n0->pnt[2],
			t->n1->pnt[0], t->n1->pnt[1], t->n1->pnt[2],
			t->n2->pnt[0], t->n2->pnt[1], t->n2->pnt[2],
			t->norm[0],    t->norm[1],    t->norm[2] );
		xmlAddStringNode( tri_list_node, "Tri", str );
	}
  }
}

//==== Read External File ====//
void MeshGeom::read(xmlNodePtr root)
{
   meshType = MODEL_MESH;

 //===== Read General Parameters =====//
  xmlNodePtr node = xmlGetNode( root, "General_Parms", 0 );
  if ( node )
    read_general_parms( node );

  //===== Read Fuse Parameters =====//
  xmlNodePtr mesh_node = xmlGetNode( root, "Mesh_Parms", 0 );
  if ( mesh_node )
  {
	for ( int i = 0 ; i < (int)tMeshVec.size() ; i++ )
		delete tMeshVec[i];
	tMeshVec.clear();

	TMesh* tmeshPtr = new TMesh();
	tMeshVec.push_back( tmeshPtr );

  	xmlNodePtr tri_list_node = xmlGetNode( mesh_node, "Tri_List", 0 );
	if ( tri_list_node )
	{
		int num_tris =  xmlGetNumNames( tri_list_node, "Tri" );
		double* arr = (double*)malloc( 12*sizeof(double) );
		for ( int i = 0 ; i < num_tris ; i++ )
		{
  			xmlNodePtr tri_node = xmlGetNode( tri_list_node, "Tri", i );
			int num_arr = xmlGetNumArray( tri_node, ',' );
			if ( num_arr == 12 )
			{
				xmlExtractDoubleArray( tri_node, ',', arr, num_arr );

				//==== Add Valid Facet ====//
				TTri* tPtr = new TTri();
				tPtr->interiorFlag = 0;
				tPtr->norm = vec3d( arr[9], arr[10], arr[11] );
				tmeshPtr->tVec.push_back( tPtr );

				//==== Put Nodes Into Tri ====//
				tPtr->n0 = new TNode();
				tPtr->n1 = new TNode();;
				tPtr->n2 = new TNode();
				tPtr->n0->pnt = vec3d( arr[0], arr[1], arr[2] );
				tPtr->n1->pnt = vec3d( arr[3], arr[4], arr[5] );
				tPtr->n2->pnt = vec3d( arr[6], arr[7], arr[8] );
			}
		}
		delete arr;
	}
  }
  update_bbox();
  generate();
}

//==== Read Fuse File ====//
void MeshGeom::read(FILE* file_id)
{
}

//==== Generate Cross Sections =====//
void MeshGeom::regenerate()
{
}



//==== Generate Cross Sections =====//
void MeshGeom::update_bbox()
{
  int i, j;
  vec3d tmp_pnt;
  bbox new_box;

  for ( i = 0 ; i < (int)tMeshVec.size() ; i++ )
  {
	  for ( j = 0 ; j < (int)tMeshVec[i]->tVec.size() ; j++ )
	  {
		  new_box.update( tMeshVec[i]->tVec[j]->n0->pnt );
	  }
  }


  bnd_box = new_box;

  update_xformed_bbox();			// Load Xform BBox

}


void MeshGeom::draw()
{
	int i;
	vec3d p;

	//==== Draw Highlighting Boxes ====//
	draw_highlight_boxes();

	//==== Check Noshow Flag ====//
	if ( noshowFlag ) return;

	if ( massPropFlag )
	{
		for ( i = 0 ; i < (int)tMeshVec.size() ; i++ )
		{
			if ( tMeshVec[i]->shellFlag )
			{
				glColor3ub( (int)tMeshVec[i]->color.x(), 
                            (int)tMeshVec[i]->color.y(), 
                            (int)tMeshVec[i]->color.z() );	
				tMeshVec[i]->draw_wire();
			}
		}

		//==== Find Min/Max Densities ====//
		double delDen = maxTriDen - minTriDen;

		for ( i = 0 ; i < (int)mpTriVec.size() ; i++ )
		{
			double fract = 0.0;
			if ( delDen > 0.0 )
				fract = (mpTriVec[i]->mass - minTriDen)/delDen;

			int red  = (int)(fract*255.0);
			int blue = (int)((1.0-fract)*255.0);

			glColor3ub( red, 0, blue );

			glBegin( GL_POLYGON );

			glVertex3dv( mpTriVec[i]->n0->pnt.data() );
			glVertex3dv( mpTriVec[i]->n1->pnt.data() );
 			glVertex3dv( mpTriVec[i]->n2->pnt.data() );

			glEnd();
		}
		return;
	}

	if ( displayFlag == GEOM_SHADE_FLAG )
	{
		glEnable( GL_LIGHTING );
          
		//==== Draw Geom ====//
		glPushMatrix();
		glMultMatrixf((GLfloat*)model_mat); 
		glEnable( GL_LIGHTING );
 
		Material* mat = matMgrPtr->getMaterial( materialID );
		if ( mat )
		{
			mat->bind();
			if ( mat->diff[3] > 0.99 )
			{
				for ( i = 0 ; i < (int)tMeshVec.size() ; i++ )
				{
					tMeshVec[i]->draw_shaded();
				}
				for ( i = 0 ; i < (int)sliceVec.size() ; i++ )
				{
					sliceVec[i]->draw_shaded();
				}
			}
		}
		glDisable( GL_LIGHTING );
		glPopMatrix();
	}
	if ( displayFlag == GEOM_HIDDEN_FLAG ) 
	{
		//==== Draw Geom ====//
		glPushMatrix();
		glMultMatrixf((GLfloat*)model_mat); 

		glColor3ub( 255, 255, 255 );
		for ( i = 0 ; i < (int)tMeshVec.size() ; i++ )
		{
			tMeshVec[i]->draw_shaded();
		}
		for ( i = 0 ; i < (int)sliceVec.size() ; i++ )
		{
			sliceVec[i]->draw_shaded();
		}

		glPopMatrix();
	}
	if ( displayFlag == GEOM_WIRE_FLAG || displayFlag == GEOM_HIDDEN_FLAG )
	{
		//==== Set Line Width ====//  
		glLineWidth(1);

		if ( displayFlag == GEOM_HIDDEN_FLAG )
			glLineWidth(2);

		//==== Draw Geom ====//
		glPushMatrix();
		glMultMatrixf((GLfloat*)model_mat); 

		if (meshType == MODEL_MESH )
			glColor3ub( (int)color.x(), (int)color.y(), (int)color.z() );	

		for ( i = 0 ; i < (int)tMeshVec.size() ; i++ )
		{
			TMesh* tmesh = tMeshVec[i];
			if ( tmesh )
			{
				if ( meshType == INTERSECTION_MESH )
					glColor3ub( (int)tmesh->color.x(), (int)tmesh->color.y(), (int)tmesh->color.z() );	
				tmesh->draw_wire();
			}
		}
		for ( i = 0 ; i < (int)sliceVec.size() ; i++ )
		{
			glColor3ub( (int)color.x(), (int)color.y(), (int)color.z() );	
			sliceVec[i]->draw_wire();
		}

		glPopMatrix();
	}
}

//==== Draw If Alpha < 1 and Shaded ====//
void MeshGeom::drawAlpha()
{

	if ( displayFlag != GEOM_SHADE_FLAG )
		return;

	vec3d p;

	Material* mat = matMgrPtr->getMaterial( materialID );
	if ( mat && mat->diff[3] <= 0.99 )
	{
		//==== Draw Geom ====//
		glPushMatrix();
		glMultMatrixf((GLfloat*)model_mat); 

		mat->bind();

		glEnable( GL_LIGHTING );

		glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
		glEnable( GL_BLEND );
		glEnable( GL_CULL_FACE );
		glCullFace( GL_BACK );

		for ( int i = 0 ; i < (int)tMeshVec.size() ; i++ )
		{
			tMeshVec[i]->draw_shaded();
		}

		glPopMatrix();

		glDisable( GL_LIGHTING );
	}

}

//==== Compute And Load Normals ====//
void MeshGeom::load_normals()
{
}

//==== Draw Other Pnts XSecs====//
void MeshGeom::load_hidden_surf()
{
}


void MeshGeom::scale()
{
	if ( fabs( lastScaleFactor - scaleFactor() ) < 0.0000001 ) 
		return;

	map< TNode*, int > nodeMap;
	for ( int i = 0 ; i < (int)tMeshVec.size() ; i++ )
	{
		for ( int j = 0 ; j < (int)tMeshVec[i]->nVec.size() ; j++ )
		{
			TNode* n = tMeshVec[i]->nVec[j];
			nodeMap[n] = 1;
//			n->pnt = n->pnt*( scaleFactor()/lastScaleFactor );
		}
		//==== Split Tris ====//
		for ( int j = 0 ; j < (int)tMeshVec[i]->tVec.size() ; j++ )
		{
			TTri* t = tMeshVec[i]->tVec[j];
			for ( int k = 0 ; k < (int)t->nVec.size() ; k++ )
			{
				TNode* n = t->nVec[k];
				nodeMap[n] = 1;
//				n->pnt = n->pnt*( scaleFactor()/lastScaleFactor );
			}
		}
	}
	map<TNode*, int >::const_iterator iter;
	for ( iter = nodeMap.begin() ; iter != nodeMap.end() ; iter++ )
	{
		TNode* n = iter->first;
		n->pnt = n->pnt*( scaleFactor()/lastScaleFactor );
	}


	lastScaleFactor = scaleFactor();

}

void MeshGeom::acceptScaleFactor()
{
	update_bbox();
	scaleFactor.set(1.0);
	lastScaleFactor = 1.0;
}

void MeshGeom::resetScaleFactor()
{

	scaleFactor.set( 1.0/lastScaleFactor );
	lastScaleFactor = 1.0;
	scale();

	scaleFactor.set(1.0);
	lastScaleFactor = 1.0;

	//==== Tell Aircraft You Have Changed ====//
	airPtr->geomMod( this );
}


void MeshGeom::intersectTrim(int meshf, int halfFlag )
{
	int i, j;

	meshFlag = meshf;

//	FILE* fid = fopen("comp_geom.txt", "w");
	Stringc txtfn = airPtr->getExportFileName( Aircraft::COMP_GEOM_TXT_TYPE );
	FILE* fid = fopen(txtfn, "w");

	//==== Check For Open Meshes and Merge or Delete Them ====//
	MeshInfo info;
	mergeRemoveOpenMeshes( &info );

		
	//==== Count Tris ====//
	int numTris = 0;
	for ( i = 0 ; i < (int)tMeshVec.size() ; i++ )
	{
		if ( !tMeshVec[i]->halfBoxFlag )
			numTris += tMeshVec[i]->tVec.size();
	}

	//==== Count Components ====//
	vector< int > compIdVec;
	for ( i = 0 ; i < (int)tMeshVec.size() ; i++ )
	{
		if ( !tMeshVec[i]->halfBoxFlag )
		{
			int id = tMeshVec[i]->ptr_id;
			vector<int>::iterator iter;

			iter = find(compIdVec.begin(), compIdVec.end(), id );

			if ( iter == compIdVec.end() )
				compIdVec.push_back( id );
		}
	}

	fprintf( fid, "...Comp Geom...\n" );
	fprintf( fid, "%d Num Comps\n", compIdVec.size() );
	fprintf( fid, "%d Total Num Meshes\n", tMeshVec.size() );
	fprintf( fid, "%d Total Num Tris\n", numTris );

	//==== Scale To 10 Units ====//
	update_bbox();
	lastScaleFactor = 1.0;
	scaleFactor = 1000.0/bnd_box.get_largest_dim();
	scale();

	//==== Create Bnd Box for  Mesh Geoms ====//
	for ( i = 0 ; i < (int)tMeshVec.size() ; i++ )
	{
		tMeshVec[i]->loadBndBox();
	}

	//==== Update Bnd Box for  Combined ====//
	bbox b;
	for ( i = 0 ; i < (int)tMeshVec.size() ; i++ )
	{
		b.update( tMeshVec[i]->tBox.box );
	}
	bnd_box = b;
	update_xformed_bbox();			// Load Xform BBox

	//==== Intersect All Mesh Geoms ====//
	for ( i = 0 ; i < (int)tMeshVec.size() ; i++ )
	{
		for ( j = i+1 ; j < (int)tMeshVec.size() ; j++ )
		{
			tMeshVec[i]->intersect( tMeshVec[j] );			
		}
	}

	//==== Split Intersected Tri in Mesh ====//
	for ( i = 0 ; i < (int)tMeshVec.size() ; i++ )
	{	
		tMeshVec[i]->split(meshFlag);
	}

	//==== Determine Which Triangle Are Interior/Exterior ====//
	for ( i = 0 ; i < (int)tMeshVec.size() ; i++ )
	{
		tMeshVec[i]->deterIntExt( tMeshVec );
	}

	if ( halfFlag )
	{
		//==== Remove Half Mesh Box ===//
		vector< TMesh* > tempVec;
		for ( i = 0 ; i < (int)tMeshVec.size() ; i++ )
		{
			if ( !tMeshVec[i]->halfBoxFlag )
				tempVec.push_back( tMeshVec[i] );
			else
				delete tMeshVec[i];
		}
		tMeshVec = tempVec;
	}


	//===== Reset Scale =====//
	resetScaleFactor();
	update_bbox();

	//==== Compute Areas ====//
	totalTheoArea = totalWetArea = 0.0;
	for ( i = 0 ; i < (int)tMeshVec.size() ; i++ )
	{
		if ( !tMeshVec[i]->halfBoxFlag )
		{
			totalTheoArea += tMeshVec[i]->computeTheoArea();
			totalWetArea  += tMeshVec[i]->computeWetArea();
		}
	}

	//==== Compute Theo Vols ====//
	totalTheoVol = 0;
	for ( i = 0 ; i < (int)tMeshVec.size() ; i++ )
	{
		if ( !tMeshVec[i]->halfBoxFlag )
		{
			totalTheoVol += tMeshVec[i]->computeTheoVol();	
		}
	}

	//==== Compute Total Volume ====//
	totalWetVol = 0.0;
	for ( i = 0 ; i < (int)tMeshVec.size() ; i++ )
	{
		if ( !tMeshVec[i]->halfBoxFlag )
		{
			totalWetVol += tMeshVec[i]->computeTrimVol();
		}
	}

	//==== Load TMeshs Into Component Based Groups ====//
	vector < vector< TMesh* > > tMeshCompVec;
	for ( i = 0 ; i < (int)compIdVec.size() ; i++ )
	{
		vector<TMesh*> cidVec;
		for ( j = 0 ; j < (int)tMeshVec.size() ; j++ )
		{
			if ( compIdVec[i] == tMeshVec[j]->ptr_id )
				cidVec.push_back( tMeshVec[j] );
		}
		tMeshCompVec.push_back( cidVec );
	}

	//==== Sum Area/Vol Data and Place in First TMesh Data ====//
	double guessTotalWetVol = 0;
	for ( i = 0 ; i < (int)tMeshCompVec.size() ; i++ )
	{
		double ta, wa, tv, wv;			// Theo Area, Wet Area, Theo Vol, Wet Vol
		ta = wa = tv = wv = 0.0;

		for ( j = 0 ; j < (int)tMeshCompVec[i].size() ; j++ )
		{
			ta += tMeshCompVec[i][j]->theoArea;
			wa += tMeshCompVec[i][j]->wetArea;
			tv += tMeshCompVec[i][j]->theoVol;
		}

		tMeshCompVec[i][0]->theoArea = ta;
		tMeshCompVec[i][0]->wetArea  = wa;
		tMeshCompVec[i][0]->theoVol  = tv;
		tMeshCompVec[i][0]->guessVol = tv*wa/ta;			// Guess
		tMeshCompVec[i][0]->wetVol   = 0.0;

		guessTotalWetVol += tMeshCompVec[i][0]->guessVol;

	}

	double leftOver = totalWetVol;
	int leftOverCnt = 20;
	while (leftOverCnt > 0)
	{
		leftOverCnt--;

		double sumWetVol = 0.0;
		for ( i = 0 ; i < (int)tMeshCompVec.size() ; i++ )
		{
			double perWetVol = tMeshCompVec[i][0]->guessVol/guessTotalWetVol;
			tMeshCompVec[i][0]->wetVol += perWetVol*(leftOver);

			if ( tMeshCompVec[i][0]->wetVol > tMeshCompVec[i][0]->theoVol )
			{
				tMeshCompVec[i][0]->wetVol = tMeshCompVec[i][0]->theoVol;
			}
			sumWetVol += tMeshCompVec[i][0]->wetVol;
		}
		
		if ( sumWetVol < totalWetVol )
		{
			leftOver = totalWetVol - sumWetVol;
		}
		else
		{
			leftOver = 0.0;
		}

		if ( leftOver < 0.00001 )
			leftOverCnt = 0;
	}

	

	fprintf(fid, "\n");
	fprintf(fid, "Theo_Area   Wet_Area   Theo_Vol    Wet_Vol  Name\n");
	for ( i = 0 ; i < (int)tMeshCompVec.size() ; i++ )
	{
		fprintf(fid, "%9.3f  %9.3f  %9.3f  %9.3f  %-15s\n",
			tMeshCompVec[i][0]->theoArea, tMeshCompVec[i][0]->wetArea,
			tMeshCompVec[i][0]->theoVol,  tMeshCompVec[i][0]->wetVol,
			tMeshCompVec[i][0]->name_str.get_char_star());
	}

	fprintf(fid, "-------------------------------------------------\n");
	fprintf(fid, "%9.3f  %9.3f  %9.3f  %9.3f  %-15s\n",
			totalTheoArea, totalWetArea, totalTheoVol, totalWetVol, "Totals" );

	if ( meshFlag && !halfFlag )
	{
		waterTightCheck(fid);
	}

	if ( info.numDegenerateTriDeleted )
		fprintf( fid, "WARNING: %d degenerate triangle removed\n", info.numDegenerateTriDeleted );
	if ( info.numOpenMeshedDeleted )
		fprintf( fid, "WARNING: %d open meshes removed\n", info.numOpenMeshedDeleted );
	if ( info.numOpenMeshesMerged )
		fprintf( fid, "WARNING: %d open meshes merged\n", info.numOpenMeshesMerged );


	fclose(fid);

	//==== Write CSV File ====//
	if ( !meshFlag && airPtr->getExportCompGeomCsvFile() )
	{
		Stringc csvfn = airPtr->getExportFileName( Aircraft::COMP_GEOM_CSV_TYPE );
		fid = fopen( csvfn, "w");

		fprintf(fid, "Name, Theo_Area, Wet_Area, Theo_Vol, Wet_Vol\n" );
		for ( i = 0 ; i < (int)tMeshCompVec.size() ; i++ )
		{
			fprintf(fid, "%s,%f,%f,%f,%f\n",
				tMeshCompVec[i][0]->name_str.get_char_star(),
				tMeshCompVec[i][0]->theoArea, tMeshCompVec[i][0]->wetArea,
				tMeshCompVec[i][0]->theoVol,  tMeshCompVec[i][0]->wetVol );
		}
		fprintf(fid, "%s,%f,%f,%f,%f\n",
				"Totals", totalTheoArea, totalWetArea, totalTheoVol, totalWetVol );
		fclose( fid );
	}

	//==== Write Drag BuildUp File ====//
	if ( !meshFlag && airPtr->getExportDragBuildTsvFile() )
	{
		Stringc tsvfn = airPtr->getExportFileName( Aircraft::DRAG_BUILD_TSV_TYPE );
		fid = fopen( tsvfn, "w");

		if ( fid )
		{
			fprintf( fid, "Name\tTheo_Area\tWet_Area\tTheo_Vol\tWet_Vol\tMin_Chord\tAve_Chord\tMax_Chord\t");
			fprintf( fid, "Min_TC_Ratio\tAvg_TC_Ratio\tMax_TC_Ratio\tAve_Sweep\tLength\tMax_Xsec_Area\tLen_Dia_Ratio\n");

			for ( i = 0 ; i < (int)tMeshCompVec.size() ; i++ )
			{
				TMesh* tmsh = tMeshCompVec[i][0];
				fprintf(fid, "%s\t%f\t%f\t%f\t%f\t%f\t%f\t%f\t%f\t%f\t%f\t%f\t%f\t%f\t%f\n",
					tmsh->name_str.get_char_star(),
					tmsh->theoArea, tmsh->wetArea, tmsh->theoVol,  tmsh->wetVol,
					tmsh->drag_factors.m_MinChord, tmsh->drag_factors.m_AvgChord, tmsh->drag_factors.m_MaxChord,
					tmsh->drag_factors.m_MinThickToChord, tmsh->drag_factors.m_AvgThickToChord, tmsh->drag_factors.m_MaxThickToChord,
					tmsh->drag_factors.m_AvgSweep, tmsh->drag_factors.m_Length, 
					tmsh->drag_factors.m_MaxXSecArea, tmsh->drag_factors.m_LengthToDia );

			}

			fclose( fid );
		}

	}
}

//==== Call After BndBoxes Have Been Create But Before Intersect ====//
void MeshGeom::sliceX(int numSlices)
{
	int i, j, s;

	FILE* fid = fopen("comp_geom.txt", "w");

	//==== Check For Open Meshes and Merge or Delete Them ====//
	MeshInfo info;
	mergeRemoveOpenMeshes( &info );



	//==== Count Tris ====//
	int numTris = 0;
	for ( i = 0 ; i < (int)tMeshVec.size() ; i++ )
		numTris += tMeshVec[i]->tVec.size();

	//==== Count Components ====//
	vector< int > compIdVec;
	for ( i = 0 ; i < (int)tMeshVec.size() ; i++ )
	{
		int id = tMeshVec[i]->ptr_id;
		vector<int>::iterator iter;

		iter = find(compIdVec.begin(), compIdVec.end(), id );

		if ( iter == compIdVec.end() )
			compIdVec.push_back( id );
	}

	fprintf( fid, "...Slice...\n" );
	fprintf( fid, "%d Num Comps\n", compIdVec.size() );
	fprintf( fid, "%d Total Num Meshes\n", tMeshVec.size() );
	fprintf( fid, "%d Total Num Tris\n", numTris );

	//==== Create Bnd Box for  Mesh Geoms ====//
	for ( i = 0 ; i < (int)tMeshVec.size() ; i++ )
	{
		tMeshVec[i]->loadBndBox();
	}

	//==== Update Bnd Box for  Combined ====//
	bbox b;
	for ( i = 0 ; i < (int)tMeshVec.size() ; i++ )
	{
		b.update( tMeshVec[i]->tBox.box );
	}
	bnd_box = b;
	update_xformed_bbox();			// Load Xform BBox

	double xMin = bnd_box.get_min(0) - 0.0001;
	double xMax = bnd_box.get_max(0) + 0.0001;

	//==== Build Slice Mesh Object =====//
	if ( numSlices < 3 ) 
		numSlices = 3;

	for ( s = 0 ; s < numSlices ; s++ )
	{
		TMesh* tm = new TMesh();
		sliceVec.push_back( tm );

		double x = xMin + ((double)s/(double)(numSlices-1))*(xMax-xMin);

		double ydel = 1.02*(bnd_box.get_max(1) - bnd_box.get_min(1));
		double ys   = bnd_box.get_min(1) - 0.01*ydel;
		double zdel = 1.02*(bnd_box.get_max(2) - bnd_box.get_min(2));
		double zs   = bnd_box.get_min(2) - 0.01*zdel;

		for ( i = 0 ; i < 10 ; i++ )
		{
			double y0 = ys + ydel*0.1*(double)i;
			double y1 = ys + ydel*0.1*(double)(i+1);

			for ( j = 0 ; j < 10 ; j++ )
			{
				double z0 = zs + zdel*0.1*(double)j;
				double z1 = zs + zdel*0.1*(double)(j+1);

				tm->addTri( vec3d(x, y0, z0), vec3d( x, y1, z0 ), vec3d( x, y1, z1 ), vec3d(1,0,0) );
				tm->addTri( vec3d(x, y0, z0), vec3d( x, y1, z1 ), vec3d( x, y0, z1 ), vec3d(1,0,0) );
			}
		}
	}

	//==== Load Bnding Box ====//
	for ( s = 0 ; s < (int)sliceVec.size() ; s++ )
	{
		TMesh* tm = sliceVec[s];
		tm->loadBndBox();

		//==== Intersect All Mesh Geoms ====//
		for ( i = 0 ; i < (int)tMeshVec.size() ; i++ )
		{
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
	}

	//==== Delete Mesh Geometry ====//
	for ( i = 0 ; i < (int)tMeshVec.size() ; i++ )
	{
		delete tMeshVec[i];
	}

	tMeshVec.erase( tMeshVec.begin(), tMeshVec.end() );

	fprintf(fid, "\n");
	fprintf(fid, "X       Area\n");
	for ( s = 0 ; s < (int)sliceVec.size() ; s++ )
	{
		double x = xMin + ((double)s/(double)(numSlices-1))*(xMax-xMin);
		sliceVec[s]->computeWetArea();

		fprintf(fid, "%9.3f  %9.3f\n", x, sliceVec[s]->wetArea );
	}

	if ( info.numDegenerateTriDeleted )
		fprintf( fid, "WARNING: %d degenerate triangle removed\n", info.numDegenerateTriDeleted );
	if ( info.numOpenMeshedDeleted )
		fprintf( fid, "WARNING: %d open meshes removed\n", info.numOpenMeshedDeleted );
	if ( info.numOpenMeshesMerged )
		fprintf( fid, "WARNING: %d open meshes merged\n", info.numOpenMeshesMerged );

		
	fclose(fid);

}


//==== Call After BndBoxes Have Been Create But Before Intersect ====//
void MeshGeom::newSlice(int style, int numSlices, double sliceAngle, double coneSections, Stringc filename)
{
	int tesselate = 0;
	int i, j, s;

	FILE* fid = fopen(filename.get_char_star(), "w");

	//==== Check For Open Meshes and Merge or Delete Them ====//
	MeshInfo info;
	mergeRemoveOpenMeshes( &info );

	if ( info.numDegenerateTriDeleted )
		fprintf( fid, "WARNING: %d degenerate triangle removed\n", info.numDegenerateTriDeleted );
	if ( info.numOpenMeshedDeleted )
		fprintf( fid, "WARNING: %d open meshes removed\n", info.numOpenMeshedDeleted );
	if ( info.numOpenMeshesMerged )
		fprintf( fid, "WARNING: %d open meshes merged\n", info.numOpenMeshesMerged );

	//==== Count Tris ====//
	int numTris = 0;
	for ( i = 0 ; i < (int)tMeshVec.size() ; i++ )
		numTris += tMeshVec[i]->tVec.size();

	//==== Count Components ====//
	vector< int > compIdVec;
	for ( i = 0 ; i < (int)tMeshVec.size() ; i++ )
	{
		int id = tMeshVec[i]->ptr_id;
		vector<int>::iterator iter;

		iter = find(compIdVec.begin(), compIdVec.end(), id );

		if ( iter == compIdVec.end() )
			compIdVec.push_back( id );
	}

	fprintf( fid, "...Slice...\n" );
	fprintf( fid, "%d Num Comps\n", compIdVec.size() );
	fprintf( fid, "%d Total Num Meshes\n", tMeshVec.size() );
	fprintf( fid, "%d Total Num Tris\n", numTris );

	//==== Create Bnd Box for  Mesh Geoms ====//
	for ( i = 0 ; i < (int)tMeshVec.size() ; i++ )
	{
		tMeshVec[i]->loadBndBox();
	}

	//==== Update Bnd Box for  Combined ====//
	bbox b;
	for ( i = 0 ; i < (int)tMeshVec.size() ; i++ )
	{
		b.update( tMeshVec[i]->tBox.box );
	}
	bnd_box = b;
	update_xformed_bbox();			// Load Xform BBox

	double xMin = bnd_box.get_min(0) - 0.0001;
	double xMax = bnd_box.get_max(0) + 0.0001;

	//==== Build Slice Mesh Object =====//
	if ( numSlices < 3 ) 
		numSlices = 3;

	if (style == SLICE_PLANAR)
	{
		vec3d norm(1,0,0);

		for ( s = 0 ; s < numSlices ; s++ )
		{
			TMesh* tm = new TMesh();
			sliceVec.push_back( tm );

			double x = xMin + ((double)s/(double)(numSlices-1))*(xMax-xMin);

			double ydel = 1.02*(bnd_box.get_max(1) - bnd_box.get_min(1));
			double ys   = bnd_box.get_min(1) - 0.01*ydel;
			double zdel = 1.02*(bnd_box.get_max(2) - bnd_box.get_min(2));
			double zs   = bnd_box.get_min(2) - 0.01*zdel;

			if (tesselate)
			{
				for ( i = 0 ; i < 10 ; i++ )
				{
					double y0 = ys + ydel*0.1*(double)i;
					double y1 = ys + ydel*0.1*(double)(i+1);

					for ( j = 0 ; j < 10 ; j++ )
					{
						double z0 = zs + zdel*0.1*(double)j;
						double z1 = zs + zdel*0.1*(double)(j+1);

						tm->addTri( vec3d(x, y0, z0), vec3d( x, y1, z0 ), vec3d( x, y1, z1 ), norm );
						tm->addTri( vec3d(x, y0, z0), vec3d( x, y1, z1 ), vec3d( x, y0, z1 ), norm );
					}
				}
			} 
			else
			{
				tm->addTri( vec3d(x, ys, zs), vec3d( x, ys+ydel, zs ), vec3d( x, ys+ydel, zs+zdel ), norm );
				tm->addTri( vec3d(x, ys, zs), vec3d( x, ys+ydel, zs+zdel ), vec3d( x, ys, zs+zdel ), norm );
			}
		}

	}
	else if (style == SLICE_CONIC)
	{
		for ( s = 0 ; s < numSlices ; s++ )
		{
			TMesh* tm = new TMesh();
			sliceVec.push_back( tm );

			double x = xMin + ((double)s/(double)(numSlices-1))*(xMax-xMin);

			double ydel = bnd_box.get_max(1) - bnd_box.get_min(1);
			double zdel = bnd_box.get_max(2) - bnd_box.get_min(2);
			double ycenter = bnd_box.get_min(1) + ydel * 0.5;
			double zcenter = bnd_box.get_min(2) + zdel * 0.5;

			double radius = max(ydel, zdel) * 1.02;

			if (sliceAngle == 0) sliceAngle = 1;
			double coneOffset = radius / tan(DEG2RAD(sliceAngle));

			for (double a = 0; a < coneSections; a++)
			{
				
				double theta0 = (2.0*PI * a) / ((double) coneSections);
				double theta1 = (2.0*PI * (a+1)) / ((double) coneSections);
				double y0 = ycenter + radius * sin(theta0);
				double z0 = zcenter + radius * cos(theta0);
				double y1 = ycenter + radius * sin(theta1);
				double z1 = zcenter + radius * cos(theta1);

				vec3d c0(x, ycenter, zcenter);
				vec3d a0(x+coneOffset, y0, z0);
				vec3d b0(x+coneOffset, y1, z1);
				vec3d norm = cross(b0 - c0, a0 - c0);

				if (tesselate)
				{
					vector<vec3d> triangles = tessTri(c0, b0, a0, 3);
					for (int b = 0; b < (int)triangles.size(); b+=3)
					{
						tm->addTri( triangles[b], triangles[b+1], triangles[b+2], norm );
					}
				}
				else {
					tm->addTri( c0, b0, a0, norm );
				}
			}
		}
	}
	else if (style == SLICE_AWAVE)
	{
		for ( s = 0 ; s < numSlices ; s++ )
		{

			double xcenter = xMin + ((double)s/(double)(numSlices-1))*(xMax-xMin);

			double ydel = bnd_box.get_max(1) - bnd_box.get_min(1);
			double zdel = bnd_box.get_max(2) - bnd_box.get_min(2);
			double ycenter = bnd_box.get_min(1) + ydel * 0.5;
			double zcenter = bnd_box.get_min(2) + zdel * 0.5;

			double size = max(ydel, zdel) * 1.02;
			if (sliceAngle == 0) sliceAngle = 1;
			double xdel = 2 * size / tan(DEG2RAD(sliceAngle));

			double radius = size/2.0;

			// for num sections, rotate about x-axis
			for (double a = 0; a < coneSections; a++)
			{
				TMesh* tm = new TMesh();
				sliceVec.push_back( tm );
				
				double theta = (2.0*PI * a) / ((double) coneSections);
				double rsintheta = radius * sin(theta);
				double rcostheta = radius * cos(theta);

				vec3d offset(xcenter, ycenter, zcenter);
				vec3d tr(-xdel/2, rcostheta + rsintheta, rcostheta - rsintheta);
				vec3d tl(-xdel/2, rsintheta - rcostheta, rcostheta + rsintheta);
				vec3d br = vec3d(0,0,0) - tl;
				vec3d bl = vec3d(0,0,0) - tr;
				vec3d norm = cross(br - bl, tl - bl);

				if (tesselate)
				{
					vec3d bl0, br0, tl0, tr0;
					vec3d bl1, br1, tl1, tr1;
					double u, v, u1, v1;
					double increment = 0.1;
					for ( u = 0; u < 1.0; u+=increment )
					{	// tesselate
						u1 = u+increment;
						bl0 = ( bl*u  + br*(1.0-u ) );
						br0 = ( bl*u1 + br*(1.0-u1) );
						tl0 = ( tl*u  + tr*(1.0-u ) );
						tr0 = ( tl*u1 + tr*(1.0-u1) );
						for ( v = 0; v < 1.0; v+=increment )
						{	// tesselate
							v1 = v+increment;
							bl1 = ( bl0*v  + tl0*(1.0-v ) );
							tl1 = ( bl0*v1 + tl0*(1.0-v1) );
							br1 = ( br0*v  + tr0*(1.0-v ) );
							tr1 = ( br0*v1 + tr0*(1.0-v1) );

							tm->addTri( bl1 + offset, br1 + offset, tr1 + offset, norm );
							tm->addTri( bl1 + offset, tr1 + offset, tl1 + offset, norm );
						}
					}
				}
				else 
				{
					tm->addTri( bl + offset, br + offset, tr + offset, norm );
					tm->addTri( bl + offset, tr + offset, tl + offset, norm );
				}
			}
		}

	}
	else return;


	//==== Load Bnding Box ====//
	for ( s = 0 ; s < (int)sliceVec.size() ; s++ )
	{
		TMesh* tm = sliceVec[s];
		tm->loadBndBox();

		//==== Intersect All Mesh Geoms ====//
		for ( i = 0 ; i < (int)tMeshVec.size() ; i++ )
		{
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
	}
	//==== Delete Mesh Geometry ====//
	for ( i = 0 ; i < (int)tMeshVec.size() ; i++ )
	{
		delete tMeshVec[i];
	}

	tMeshVec.erase( tMeshVec.begin(), tMeshVec.end() );

	if (style == SLICE_PLANAR || style == SLICE_CONIC)
	{
		fprintf(fid, "\n");
		fprintf(fid, "    X          Area\n");
		for ( s = 0 ; s < (int)sliceVec.size() ; s++ )
		{
			double x = xMin + ((double)s/(double)(numSlices-1))*(xMax-xMin);
			sliceVec[s]->computeWetArea();

			fprintf(fid, "%9.3f  %9.3f\n", x, sliceVec[s]->wetArea );
		}
	}	
	else if (style == SLICE_AWAVE)
	{
		fprintf(fid, "\n");
		fprintf(fid, "           X        ");
		for ( i = 0; i < coneSections; i++ )
		{
			fprintf(fid, "        A%02d(%06.2f°)", i, 360.0 * i / (float) coneSections);
		}
		fprintf(fid, "        TotalArea            Average\n");
		for ( s = 0 ; s < numSlices ; s++ )
		{
			double sum = 0;
			double x = xMin + ((double)s/(double)(numSlices-1))*(xMax-xMin);

			fprintf(fid, "%19.8f", x);
			for ( int r = 0; r < coneSections; r++ )
			{
				int sindex = (int)(s * coneSections + r);
				sliceVec[sindex]->computeAwaveArea();
				// sliceVec[sindex]->computeWetArea();
				sum += sliceVec[sindex]->wetArea;

				fprintf(fid, " %19.8f", sliceVec[sindex]->wetArea);
			}
			fprintf(fid, " %19.8f", sum);
			fprintf(fid, " %19.8f", sum/(double)coneSections);
			fprintf(fid, "\n");
		}

	}
	fclose(fid);
}


vector<vec3d> MeshGeom::tessTri(vec3d t1, vec3d t2, vec3d t3, int iterations)
{
	vector<vec3d> triangles;
	triangles.push_back(t1);
	triangles.push_back(t2);
	triangles.push_back(t3);
	
	for (int i = 0; i < iterations; i++)
	{
		triangles = tessTriangles(triangles);
	}
	return triangles;
}

vector<vec3d> MeshGeom::tessTriangles(vector<vec3d> &tri)
{
	assert((tri.size())%3 == 0);
	vector< vec3d > triangles;
	for (int i = 0; i < (int)tri.size(); i+=3)
	{
		vec3d t1 = tri[ i ];
		vec3d t2 = tri[i+1];
		vec3d t3 = tri[i+2];

		vec3d c12 = (t1 + t2) * 0.5;
		vec3d c13 = (t1 + t3) * 0.5;
		vec3d c23 = (t2 + t3) * 0.5;

		triangles.push_back(t1);
		triangles.push_back(c12);
		triangles.push_back(c13);

		triangles.push_back(t2);
		triangles.push_back(c23);
		triangles.push_back(c12);

		triangles.push_back(t3);
		triangles.push_back(c13);
		triangles.push_back(c23);

		triangles.push_back(c23);
		triangles.push_back(c13);
		triangles.push_back(c12);
	}
	return triangles;
}

vec3d MeshGeom::getVertex3d(int surf, double x, double p, int r)	
{
	if (x < 0)	x = 0;
	if (p < 0)	p = 0;
	if (x > 1)	x = 1;
	if (p > 1)  p = 1;


	if (surf == 0) // tmesh
	{
		int t = (int)(x * (tMeshVec.size()-1) + 0.5);
		int v = (int)(p * (tMeshVec[t]->numVerts()-1) + 0.5);
		return tMeshVec[t]->getVertex(v);
	}
	else if (surf == 1) //slice
	{
		int s = (int)(x * (sliceVec.size()-1) + 0.5);
		int v = (int)(p * (sliceVec[s]->numVerts()-1) + 0.5);
		return sliceVec[s]->getVertex(v);
	}
	return vec3d();
}
void  MeshGeom::getVertexVec(vector< VertexID > *vertVec)
{ 
	buildVertexVec(&tMeshVec, 0, vertVec); 
	buildVertexVec(&sliceVec, 1, vertVec); 
}

//==== Call After BndBoxes Have Been Create But Before Intersect ====//
void MeshGeom::massSliceX(int numSlices)
{
	int i, j, s;

	FILE* fid = fopen("massprop.txt", "w");

	//==== Check For Open Meshes and Merge or Delete Them ====//
	MeshInfo info;
	mergeRemoveOpenMeshes( &info );

	if ( info.numDegenerateTriDeleted )
		fprintf( fid, "WARNING: %d degenerate triangle removed\n", info.numDegenerateTriDeleted );
	if ( info.numOpenMeshedDeleted )
		fprintf( fid, "WARNING: %d open meshes removed\n", info.numOpenMeshedDeleted );
	if ( info.numOpenMeshesMerged )
		fprintf( fid, "WARNING: %d open meshes merged\n", info.numOpenMeshesMerged );


	//==== Count Tris ====//
	int numTris = 0;
	for ( i = 0 ; i < (int)tMeshVec.size() ; i++ )
		numTris += tMeshVec[i]->tVec.size();

	//==== Count Components ====//
	vector< int > compIdVec;
	for ( i = 0 ; i < (int)tMeshVec.size() ; i++ )
	{
		int id = tMeshVec[i]->ptr_id;
		vector<int>::iterator iter;

		iter = find(compIdVec.begin(), compIdVec.end(), id );

		if ( iter == compIdVec.end() )
			compIdVec.push_back( id );
	}

	fprintf( fid, "...Mass Properties...\n" );
	fprintf( fid, "%d Num Comps\n", compIdVec.size() );
	fprintf( fid, "%d Total Num Meshes\n", tMeshVec.size() );
	fprintf( fid, "%d Total Num Tris\n", numTris );

	//==== Create Bnd Box for  Mesh Geoms ====//
	for ( i = 0 ; i < (int)tMeshVec.size() ; i++ )
	{
		tMeshVec[i]->loadBndBox();
	}

	//==== Update Bnd Box for  Combined ====//
	bbox b;
	for ( i = 0 ; i < (int)tMeshVec.size() ; i++ )
	{
		b.update( tMeshVec[i]->tBox.box );
	}
	bnd_box = b;
	update_xformed_bbox();			// Load Xform BBox

	double xMin = bnd_box.get_min(0);
	double xMax = bnd_box.get_max(0);

	double sliceW = (xMax - xMin)/(double)(numSlices);

	//==== Build Slice Mesh Object =====//
	if ( numSlices < 3 ) 
		numSlices = 3;

	for ( s = 0 ; s < numSlices ; s++ )
	{
		TMesh* tm = new TMesh();
		sliceVec.push_back( tm );

		double x = xMin + (double)s*sliceW + 0.5*sliceW;

		double ydel = 1.02*(bnd_box.get_max(1) - bnd_box.get_min(1));
		double ys   = bnd_box.get_min(1) - 0.01*ydel;
		double zdel = 1.02*(bnd_box.get_max(2) - bnd_box.get_min(2));
		double zs   = bnd_box.get_min(2) - 0.01*zdel;

		for ( i = 0 ; i < 10 ; i++ )
		{
			double y0 = ys + ydel*0.1*(double)i;
			double y1 = ys + ydel*0.1*(double)(i+1);

			for ( j = 0 ; j < 10 ; j++ )
			{
				double z0 = zs + zdel*0.1*(double)j;
				double z1 = zs + zdel*0.1*(double)(j+1);

				tm->addTri( vec3d(x, y0, z0), vec3d( x, y1, z0 ), vec3d( x, y1, z1 ), vec3d(1,0,0) );
				tm->addTri( vec3d(x, y0, z0), vec3d( x, y1, z1 ), vec3d( x, y0, z1 ), vec3d(1,0,0) );
			}
		}
	}

	//==== Load Bnding Box ====//
	for ( s = 0 ; s < (int)sliceVec.size() ; s++ )
	{
		TMesh* tm = sliceVec[s];
		tm->loadBndBox();

		//==== Intersect All Mesh Geoms ====//
		for ( i = 0 ; i < (int)tMeshVec.size() ; i++ )
		{
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
		tm->massDeterIntExt( tMeshVec );



	}
/**********
	//==== Delete Mesh Geometry ====//
	for ( i = 0 ; i < (int)tMeshVec.size() ; i++ )
	{
		delete tMeshVec[i];
	}
	tMeshVec.erase( tMeshVec.begin(), tMeshVec.end() );
*********/
	//==== Intersect All Mesh Geoms ====//
	for ( i = 0 ; i < (int)tMeshVec.size() ; i++ )
	{
		for ( j = i+1 ; j < (int)tMeshVec.size() ; j++ )
		{
			tMeshVec[i]->intersect( tMeshVec[j] );			
		}
	}

	//==== Split Intersected Tri in Mesh ====//
	for ( i = 0 ; i < (int)tMeshVec.size() ; i++ )
	{
		tMeshVec[i]->split();
	}

	//==== Determine Which Triangle Are Interior/Exterior ====//
	for ( i = 0 ; i < (int)tMeshVec.size() ; i++ )
	{
		tMeshVec[i]->deterIntExt( tMeshVec );
	}

	//==== Do Shell Calcs ====//
	vector< TriShellMassProp* > triShellVec;
	for ( s = 0 ; s < (int)tMeshVec.size() ; s++ )
	{
		TMesh* tm = tMeshVec[s];
		if ( tm->shellFlag )
		{
			for ( i = 0 ; i < (int)tm->tVec.size() ; i++ )
			{
				TTri* tri = tm->tVec[i];
				if ( tri->splitVec.size() )
				{
					for ( j = 0 ; j < (int)tri->splitVec.size() ; j++ )
					{
						if ( tri->splitVec[j]->interiorFlag == 0 )
						{
							TriShellMassProp* tsmp = new TriShellMassProp( tm->ptr_id, tm->shellMassArea, 
																tri->splitVec[j]->n0->pnt, 
																tri->splitVec[j]->n1->pnt, 
																tri->splitVec[j]->n2->pnt );
							triShellVec.push_back( tsmp );
						} 
					}
				}
				else if ( tri->interiorFlag == 0 )
				{
					TriShellMassProp* tsmp = new TriShellMassProp( tm->ptr_id, tm->shellMassArea, 
											tri->n0->pnt, tri->n1->pnt, tri->n2->pnt );
					triShellVec.push_back( tsmp );
				}
			}
		}
	}

	//==== Build Tetrahedrons ====//
	double prismLength = sliceW;
	vector< TetraMassProp* > tetraVec;
	minTriDen = 1.0e06;
    maxTriDen = 0.0;
	mpTriVec.clear();

	for ( s = 0 ; s < (int)sliceVec.size() ; s++ )
	{
		TMesh* tm = sliceVec[s];
		for ( i = 0 ; i < (int)tm->tVec.size() ; i++ )
		{
			TTri* tri = tm->tVec[i];

			if ( tri->splitVec.size() )
			{
				for ( j = 0 ; j < (int)tri->splitVec.size() ; j++ )
				{
					if ( tri->splitVec[j]->interiorFlag == 0 )
					{
						createPrism( tetraVec, tri->splitVec[j], prismLength );
						mpTriVec.push_back(tri->splitVec[j]);
					} 
				}
			}
			else if ( tri->interiorFlag == 0 )
			{
				createPrism( tetraVec, tri, prismLength );
				mpTriVec.push_back(tri);
			}
		}
	}

	//==== Add in Point Masses ====//
	for ( i = 0 ; i < (int)pointMassVec.size() ; i++ )
		tetraVec.push_back( pointMassVec[i] );

	double totalVol = 0.0;
	for ( i = 0 ; i < (int)tetraVec.size() ; i++ )
		totalVol += fabs(tetraVec[i]->vol);

	vec3d cg(0,0,0);
	totalMass = 0.0;
	for ( i = 0 ; i < (int)tetraVec.size() ; i++ )
	{
		totalMass += tetraVec[i]->mass;
		cg = cg + tetraVec[i]->cg*tetraVec[i]->mass;
	}
	for ( i = 0 ; i < (int)triShellVec.size() ; i++ )
	{
		totalMass += triShellVec[i]->mass;
		cg = cg + triShellVec[i]->cg*triShellVec[i]->mass;
	}

	if ( totalMass )
		cg = cg*(1.0/totalMass);

	massPropFlag = 1;
	centerOfGrav = cg;

	totalIxx = totalIyy = totalIzz = 0.0;
	totalIxy = totalIxz = totalIyz = 0.0;
	for ( i = 0 ; i < (int)tetraVec.size() ; i++ )
	{
		TetraMassProp* tet = tetraVec[i];
		totalIxx += tet->Ixx + 
			tet->mass*( (cg.y()-tet->cg.y())*(cg.y()-tet->cg.y()) + (cg.z()-tet->cg.z())*(cg.z()-tet->cg.z()));
		totalIyy += tet->Iyy + 
			tet->mass*( (cg.x()-tet->cg.x())*(cg.x()-tet->cg.x()) + (cg.z()-tet->cg.z())*(cg.z()-tet->cg.z()));
		totalIzz += tet->Izz + 
			tet->mass*( (cg.x()-tet->cg.x())*(cg.x()-tet->cg.x()) + (cg.y()-tet->cg.y())*(cg.y()-tet->cg.y()));

		totalIxy += tet->Ixy + 
			tet->mass*( (cg.x()-tet->cg.x()) * (cg.y()-tet->cg.y()));
		totalIxz += tet->Ixz + 
			tet->mass*( (cg.x()-tet->cg.x()) * (cg.z()-tet->cg.z()));
		totalIyz += tet->Iyz + 
			tet->mass*( (cg.y()-tet->cg.y()) * (cg.z()-tet->cg.z()));
	}
	for ( i = 0 ; i < (int)triShellVec.size() ; i++ )
	{
		TriShellMassProp* trs = triShellVec[i];
		totalIxx += trs->Ixx + 
			trs->mass*( (cg.y()-trs->cg.y())*(cg.y()-trs->cg.y()) + (cg.z()-trs->cg.z())*(cg.z()-trs->cg.z()));
		totalIyy += trs->Iyy + 
			trs->mass*( (cg.x()-trs->cg.x())*(cg.x()-trs->cg.x()) + (cg.z()-trs->cg.z())*(cg.z()-trs->cg.z()));
		totalIzz += trs->Izz + 
			trs->mass*( (cg.x()-trs->cg.x())*(cg.x()-trs->cg.x()) + (cg.y()-trs->cg.y())*(cg.y()-trs->cg.y()));

		totalIxy += trs->Ixy + 
			trs->mass*( (cg.x()-trs->cg.x()) * (cg.y()-trs->cg.y()));
		totalIxz += trs->Ixz + 
			trs->mass*( (cg.x()-trs->cg.x()) * (cg.z()-trs->cg.z()));
		totalIyz += trs->Iyz + 
			trs->mass*( (cg.y()-trs->cg.y()) * (cg.z()-trs->cg.z()));
	}

	fprintf(fid, "\n");
	fprintf(fid, "%f             Total Mass\n", totalMass );
	fprintf(fid, "%f %f %f       Center of Gravity\n", cg.x(), cg.y(), cg.z());
	fprintf(fid, "%f %f %f       Ixx, Iyy, Izz\n", totalIxx, totalIyy, totalIzz );
	fprintf(fid, "%f %f %f       Ixy, Ixz, Iyz\n", totalIxy, totalIxz, totalIyz );
	fprintf(fid, "%f             Volume\n", totalVol);

	//==== Redo This on a Per Component Basis ====//
	fprintf(fid, "\n");
	fprintf(fid, "Name\tMass\tcgX\tcgY\tcgZ\tIxx\tIyy\tIzz\tIxy\tIxz\tIyz\tVolume\n");
	for ( s = 0 ; s < (int)tMeshVec.size() ; s++ )
	{
		TMesh* tm = tMeshVec[s];
		int id = tm->ptr_id;

		double compVol = 0.0;
		for ( i = 0 ; i < (int)tetraVec.size() ; i++ )
		{
			if ( tetraVec[i]->compId == id )
				compVol += fabs(tetraVec[i]->vol);
		}

		vec3d cg(0,0,0);
		double compMass = 0.0;
		for ( i = 0 ; i < (int)tetraVec.size() ; i++ )
		{
			if ( tetraVec[i]->compId == id )
			{
				compMass += tetraVec[i]->mass;
				cg = cg + tetraVec[i]->cg*tetraVec[i]->mass;
			}
		}
		for ( i = 0 ; i < (int)triShellVec.size() ; i++ )
		{
			if ( triShellVec[i]->compId == id )
			{
				compMass += triShellVec[i]->mass;
				cg = cg + triShellVec[i]->cg*triShellVec[i]->mass;
			}
		}

		if ( compMass )
			cg = cg*(1.0/compMass);

		double compIxx = 0.0; double compIyy = 0.0; double compIzz = 0.0;
		double compIxy = 0.0; double compIxz = 0.0; double compIyz = 0.0;
		for ( i = 0 ; i < (int)tetraVec.size() ; i++ )
		{
			TetraMassProp* tet = tetraVec[i];
			if ( tet->compId == id )
			{
				compIxx += tet->Ixx + 
					tet->mass*( (cg.y()-tet->cg.y())*(cg.y()-tet->cg.y()) + (cg.z()-tet->cg.z())*(cg.z()-tet->cg.z()));
				compIyy += tet->Iyy + 
					tet->mass*( (cg.x()-tet->cg.x())*(cg.x()-tet->cg.x()) + (cg.z()-tet->cg.z())*(cg.z()-tet->cg.z()));
				compIzz += tet->Izz + 
					tet->mass*( (cg.x()-tet->cg.x())*(cg.x()-tet->cg.x()) + (cg.y()-tet->cg.y())*(cg.y()-tet->cg.y()));

				compIxy += tet->Ixy + 
					tet->mass*( (cg.x()-tet->cg.x()) * (cg.y()-tet->cg.y()));
				compIxz += tet->Ixz + 
					tet->mass*( (cg.x()-tet->cg.x()) * (cg.z()-tet->cg.z()));
				compIyz += tet->Iyz + 
					tet->mass*( (cg.y()-tet->cg.y()) * (cg.z()-tet->cg.z()));
			}
		}
		for ( i = 0 ; i < (int)triShellVec.size() ; i++ )
		{
			TriShellMassProp* trs = triShellVec[i];
			if ( trs->compId == id )
			{
				compIxx += trs->Ixx + 
					trs->mass*( (cg.y()-trs->cg.y())*(cg.y()-trs->cg.y()) + (cg.z()-trs->cg.z())*(cg.z()-trs->cg.z()));
				compIyy += trs->Iyy + 
					trs->mass*( (cg.x()-trs->cg.x())*(cg.x()-trs->cg.x()) + (cg.z()-trs->cg.z())*(cg.z()-trs->cg.z()));
				compIzz += trs->Izz + 
					trs->mass*( (cg.x()-trs->cg.x())*(cg.x()-trs->cg.x()) + (cg.y()-trs->cg.y())*(cg.y()-trs->cg.y()));

				compIxy += trs->Ixy + 
					trs->mass*( (cg.x()-trs->cg.x()) * (cg.y()-trs->cg.y()));
				compIxz += trs->Ixz + 
					trs->mass*( (cg.x()-trs->cg.x()) * (cg.z()-trs->cg.z()));
				compIyz += trs->Iyz + 
					trs->mass*( (cg.y()-trs->cg.y()) * (cg.z()-trs->cg.z()));
			}
		}

		fprintf(fid, "%s\t%f\t%f\t%f\t%f\t%f\t%f\t%f\t%f\t%f\n",
			tm->name_str.get_char_star(), compMass, cg.x(), cg.y(), cg.z(),
			compIxx, compIyy, compIzz, compIxy, compIxz, compIyz, compVol );


		//fprintf(fid, "%s			 Name\n", tm->name_str.get_char_star() );
		//fprintf(fid, "%f             Total Mass\n", compMass );
		//fprintf(fid, "%f %f %f       Center of Gravity\n", cg.x(), cg.y(), cg.z());
		//fprintf(fid, "%f %f %f       Ixx, Iyy, Izz\n", compIxx, compIyy, compIzz );
		//fprintf(fid, "%f %f %f       Ixy, Ixz, Iyz\n", compIxy, compIxz, compIyz );
		//fprintf(fid, "%f             Volume\n", compVol);

	}
	fprintf(fid, "%s\t%f\t%f\t%f\t%f\t%f\t%f\t%f\t%f\t%f\n",
		"Totals", totalMass, centerOfGrav.x(), centerOfGrav.y(), centerOfGrav.z(),
		totalIxx, totalIyy, totalIzz, totalIxy, totalIxz, totalIyz, totalVol );

	//==== Clean Up Mess ====//
	for ( i = 0 ; i < (int)tetraVec.size() ; i++ )
		delete tetraVec[i];

	for ( i = 0 ; i < (int)triShellVec.size() ; i++ )
		delete triShellVec[i];

	fclose(fid);

}

//==== Create a Prism Made of Tetras - Extrude Tri +- len/2 ====//
void MeshGeom::createPrism( vector< TetraMassProp* >& tetraVec, TTri* tri, double len )
{
	if ( tri->mass < minTriDen )
		minTriDen = tri->mass;
    if ( tri->mass > maxTriDen )
		maxTriDen = tri->mass;

	vec3d cnt = (tri->n0->pnt + tri->n1->pnt + tri->n2->pnt)*(1.0/3.0);

	vec3d p0 = tri->n0->pnt;
	vec3d p1 = tri->n1->pnt;
	vec3d p2 = tri->n2->pnt;
	p0.offset_x( len/2.0 );
	p1.offset_x( len/2.0 );
	p2.offset_x( len/2.0 );

	vec3d p3 = tri->n0->pnt;
	vec3d p4 = tri->n1->pnt;
	vec3d p5 = tri->n2->pnt;
	p3.offset_x( -len/2.0 );
	p4.offset_x( -len/2.0 );
	p5.offset_x( -len/2.0 );

	tetraVec.push_back( new TetraMassProp(tri->id, tri->mass, cnt, p0, p1, p2 ) );
	tetraVec.push_back( new TetraMassProp(tri->id, tri->mass, cnt, p3, p4, p5 ) );
	tetraVec.push_back( new TetraMassProp(tri->id, tri->mass, cnt, p0, p1, p3 ) );
	tetraVec.push_back( new TetraMassProp(tri->id, tri->mass, cnt, p3, p4, p1 ) );
	tetraVec.push_back( new TetraMassProp(tri->id, tri->mass, cnt, p1, p2, p4 ) );
	tetraVec.push_back( new TetraMassProp(tri->id, tri->mass, cnt, p4, p5, p2 ) );
	tetraVec.push_back( new TetraMassProp(tri->id, tri->mass, cnt, p0, p2, p3 ) );
	tetraVec.push_back( new TetraMassProp(tri->id, tri->mass, cnt, p3, p5, p2 ) );
}

//==== Check Current Geom For Problems ====//
void MeshGeom::waterTightCheck(FILE* fid)
{
	int i, t, m;

	if ( !tMeshVec.size() )
		return;

	//==== Compute Min Edge Size Before Split ====//
	double minEdgeLen =  1.0e06;
	double minTriAng  =  1.0e06;
	double maxTriAng  = -1.0e06;
	for ( m = 0 ; m < (int)tMeshVec.size() ; m++ )
	{
		double minE, minA, maxA;
		tMeshVec[m]->meshStats( &minE, &minA, &maxA );
		if ( minE < minEdgeLen )
			minEdgeLen = minE;
		if ( minA < minTriAng )
			minTriAng = minA;
		if ( maxA > maxTriAng )
			maxTriAng = maxA;
	}

	//==== Load All Meshes into One ====//
	oneMesh = new TMesh();
	oneMesh->color = tMeshVec[0]->color;

	for ( m = 0 ; m < (int)tMeshVec.size() ; m++ )
	{
		TMesh* mesh = tMeshVec[m];
		for ( t = 0 ; t < (int)mesh->tVec.size() ; t++ )
		{
			if ( mesh->tVec[t]->splitVec.size() )
			{
				for ( i = 0 ; i < (int)mesh->tVec[t]->splitVec.size() ; i++ )
				{
					if ( mesh->tVec[t]->splitVec[i]->interiorFlag == 0 )
					{
						TTri* tri = mesh->tVec[t]->splitVec[i];
						oneMesh->addTri( tri->n0, tri->n1, tri->n2, mesh->tVec[t]->norm );
					}
				}
			}
			else if ( mesh->tVec[t]->interiorFlag == 0 )
			{
				TTri* tri = mesh->tVec[t];
				oneMesh->addTri( tri->n0, tri->n1, tri->n2, tri->norm );
			}
		}
	}

	//==== Bound Box with Oct Tree ====//
	oneMesh->loadBndBox();

	oneMesh->waterTightCheck(fid, tMeshVec);

	//==== Delete Old Meshes and Add One Mesh ====//
	for ( i = 0 ; i < (int)tMeshVec.size() ; i++ )
		delete tMeshVec[i];

	tMeshVec.clear();
	tMeshVec.push_back( oneMesh );
}
   
void MeshGeom::mergeRemoveOpenMeshes(MeshInfo* info)
{
	int i, j;

	//==== Check If All Closed ====//
	for ( i = 0 ; i < (int)tMeshVec.size() ; i++ )
	{
		tMeshVec[i]->checkIfClosed();
	}

	//==== Try to Merge Non Closed Meshes ====//
	for ( i = 0 ; i < (int)tMeshVec.size() ; i++ )
	{
		for ( j = i+1 ; j < (int)tMeshVec.size() ; j++ )
		{
			tMeshVec[i]->mergeNonClosed( tMeshVec[j] );			
		}
	}
	for ( i = 0 ; i < (int)tMeshVec.size() ; i++ )
	{
		if ( tMeshVec[i]->deleteMeFlag )
			info->numOpenMeshesMerged++;
	}

	for ( i = 0 ; i < (int)tMeshVec.size() ; i++ )
	{
		if ( tMeshVec[i]->nonClosedTriVec.size() )
		{
			if ( !tMeshVec[i]->deleteMeFlag )
				info->numOpenMeshedDeleted++;

			tMeshVec[i]->deleteMeFlag = true;
		}
	}

	//==== Remove Merged Meshes ====//
	vector< TMesh* > newTMeshVec;
	for ( i = 0 ; i < (int)tMeshVec.size() ; i++ )
	{
		if ( !tMeshVec[i]->deleteMeFlag )
			newTMeshVec.push_back( tMeshVec[i] );
		else
			delete tMeshVec[i];
	}
	tMeshVec = newTMeshVec;

	//==== Remove Any Degenerate Tris ====//
	for ( i = 0 ; i < (int)tMeshVec.size() ; i++ )
	{
		info->numDegenerateTriDeleted += tMeshVec[i]->removeDegenerate();
	}

}

void MeshGeom::addHalfBox()
{
	//==== Find Bound Box ====//
	bbox box;
	for ( int m = 0 ; m < (int)tMeshVec.size() ; m++ )
	{
		for ( int t = 0 ; t < (int)tMeshVec[m]->tVec.size() ; t++ )
		{
			box.update( tMeshVec[m]->tVec[t]->n0->pnt );
		}
	}

	//==== Make The Box a Bit Bigger ===//
	vec3d bscale = vec3d( 2, 2, 2 );
	box.scale( bscale );

	//==== Build Box Triangles =====//
	TMesh* tm = new TMesh();
	tm->halfBoxFlag = true;

	tMeshVec.push_back( tm );

	int num_div = 10;
	double xmin = box.get_min(0);
	double xmax = box.get_max(0);
	double xdel = (xmax - xmin)/(double)(num_div-1);

	double zmin = box.get_min(2);
	double zmax = box.get_max(2);
	double zdel = (zmax - zmin)/(double)(num_div-1);

	double ymin = box.get_min(1);

	////==== Y=0 Plane ====//
	//for ( int i = 0 ; i < num_div-1 ; i++ )
	//{
	//	for ( int j = 0 ; j < num_div-1 ; j++ )
	//	{	
	//		double x0 = xmin + (double)i*xdel;
	//		double x1 = xmin + (double)(i+1)*xdel;
	//		double z0 = zmin + (double)j*zdel;
	//		double z1 = zmin + (double)(j+1)*zdel;

	//		tm->addTri( vec3d(x0, 0, z0), vec3d( x1, 0, z0 ), vec3d( x0,  0, z1 ), vec3d(0,1,0) );
	//		tm->addTri( vec3d(x1, 0, z0), vec3d( x1, 0, z1 ), vec3d( x0,  0, z1 ), vec3d(0,1,0) );
	//	}
	//}

	//==== Add Other Sides ====//
	vec3d A = vec3d(xmin, 0, zmin);
	vec3d B = vec3d(xmax, 0, zmin);
	vec3d C = vec3d(xmin, 0, zmax);
	vec3d D = vec3d(xmax, 0, zmax);
	vec3d E = vec3d(xmin, ymin, zmin);
	vec3d F = vec3d(xmax, ymin, zmin);
	vec3d G = vec3d(xmin, ymin, zmax);
	vec3d H = vec3d(xmax, ymin, zmax);

	//tm->addTri( E, G, H, vec3d(0,-1,0) );
	//tm->addTri( E, H, F, vec3d(0,-1,0) );

	//tm->addTri( A, B, D, vec3d(0, 1,0) );
	//tm->addTri( A, D, C, vec3d(0, 1,0) );

	//tm->addTri( A, C, E, vec3d( -1, 0, 0) );
	//tm->addTri( C, G, E, vec3d( -1, 0, 0) );

	//tm->addTri( B, F, D, vec3d(  1, 0, 0) );
	//tm->addTri( D, F, H, vec3d(  1, 0, 0) );

	//tm->addTri( C, D, G, vec3d( 0, 0, 1) );
	//tm->addTri( D, H, G, vec3d( 0, 0, 1) );

	//tm->addTri( A, E, B, vec3d( 0, 0, -1) );
	//tm->addTri( B, E, F, vec3d( 0, 0, -1) );

	tm->addTri( G, E, H, vec3d(0,-1,0) );
	tm->addTri( H, E, F, vec3d(0,-1,0) );

	tm->addTri( B, A, D, vec3d(0, 1,0) );
	tm->addTri( D, A, C, vec3d(0, 1,0) );

	tm->addTri( C, A, E, vec3d( -1, 0, 0) );
	tm->addTri( G, C, E, vec3d( -1, 0, 0) );

	tm->addTri( F, B, D, vec3d(  1, 0, 0) );
	tm->addTri( F, D, H, vec3d(  1, 0, 0) );

	tm->addTri( D, C, G, vec3d( 0, 0, 1) );
	tm->addTri( H, D, G, vec3d( 0, 0, 1) );

	tm->addTri( E, A, B, vec3d( 0, 0, -1) );
	tm->addTri( E, B, F, vec3d( 0, 0, -1) );

}


