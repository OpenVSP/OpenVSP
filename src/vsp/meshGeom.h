//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//

//******************************************************************************
//    
//   Mesh Geometry Class
//  
// 
//   J.R. Gloudemans - 11/7/94
//   Sterling Software
//
//    
//******************************************************************************

#ifndef MESH_GEOM_H
#define MESH_GEOM_H


#include "vec2d.h"
#include "vec3d.h"
#include "parm.h"
#include "bbox.h"
#include "xmlvsp.h"
#include "geom.h"
#include "fuse_xsec.h"

#include "xsec_surf.h"
#include "dl_list.h"

#include "af.h"

#include "tMesh.h"

class MeshInfo
{
public:
	MeshInfo()		
	{ numOpenMeshesMerged = numOpenMeshedDeleted = numDegenerateTriDeleted = 0; }

	int numOpenMeshesMerged;
	int numOpenMeshedDeleted;
	int numDegenerateTriDeleted;
};


class VSPDLL MeshGeom : public Geom
{

	int bigEndianFlag;

	vector< TTri* > nascartTriVec;
	vector< TNode* > nascartNodeVec;

	vector< TTri* > tecplotTriVec[30];
	vector< TNode* > tecplotNodeVec[30];


public:
	enum { SLICE_PLANAR, SLICE_CONIC, SLICE_AWAVE };

	//==== How Mesh Is Drawn and Saved ====//
	enum { INTERSECTION_MESH, MODEL_MESH };

   MeshGeom(Aircraft* aptr);
   ~MeshGeom();

   Stringc fileName;

   double totalTheoArea;
   double totalWetArea;
   double totalTheoVol;
   double totalWetVol;

   int meshFlag;				// Do WaterTight Check and Quality Mesh
   int massPropFlag;
   int meshType;
   vec3d centerOfGrav;

   double totalMass;
   double totalIxx;
   double totalIyy;
   double totalIzz;
   double totalIxy;
   double totalIxz;
   double totalIyz;

   double minTriDen;
   double maxTriDen;
   vector < TTri* > mpTriVec;

   virtual void copy( Geom* fromGeom );

   vector < TMesh* > tMeshVec;
   vector < TMesh* > sliceVec;

   TMesh* oneMesh;

   virtual void setMeshType( int type )			{ meshType = type; }
   virtual int  getMeshType()					{ return meshType; }
  
   virtual void load_hidden_surf();
   virtual void load_normals();
   virtual void update_bbox();
  
   virtual void draw();
   virtual void generate();
   virtual void regenerate();

   virtual void write(FILE* dump_file);
   virtual void write(xmlNodePtr node);
   virtual void read(FILE* dump_file);
   virtual void read(xmlNodePtr node);

   virtual int  getNumXSecSurfs()							{ return 0; }
   virtual int  read_stl( const char* file_name );
   virtual int  read_xsec( const char* file_name );
   virtual void AddTri( TMesh* tMesh, vec3d & p0, vec3d & p1, vec3d & p2 ); 
   virtual int  read_nascart( const char* file_name );
   virtual float readBinFloat( FILE* fptr );
   virtual int   readBinInt  ( FILE* fptr );
   virtual void write_stl_file(FILE* pov_file);

   virtual void buildNascartMesh(int partOffset);
   virtual int  getNumNascartPnts()							{ return nascartNodeVec.size(); }
   virtual int  getNumTecplotPnts(int m)					{ return tecplotNodeVec[m].size(); }
   virtual int  getNumNascartTris()							{ return nascartTriVec.size();  }
   virtual int  getNumTecplotTris(int m)					{ return tecplotTriVec[m].size(); }
   virtual int  getNumNascartParts()						{ return tMeshVec.size(); }
   virtual void writeNascartPnts( FILE* file_id );
   virtual void writeCart3DPnts( FILE* file_id );
   virtual int  writeGMshNodes( FILE* file_id, int node_offset); 
   virtual int  writeNascartTris( FILE* file_id, int offset );
   virtual int  writeCart3DTris( FILE* file_id, int offset );
   virtual int  writeGMshTris( FILE* file_id, int node_offset, int tri_offset );
   virtual int  writeNascartParts( FILE* file_id, int offset );
   virtual int  writeCart3DParts( FILE* file_id, int offset );
   virtual void checkDupOrAdd( TNode* node, vector< TNode* > & nodeVec );

   virtual void dump_xsec_file(int, FILE* )	{}

   virtual void computeCenter();
   virtual void parm_changed(Parm* chg_parm);

   virtual void drawAlpha(void);
   virtual void scale();		
   virtual void acceptScaleFactor();
   virtual void resetScaleFactor();

   //==== Intersection, Splitting and Trimming ====//
   virtual void intersectTrim(int meshFlag, int halfFlag = 0 );
   virtual void sliceX(int numSlice);
   virtual void massSliceX(int numSlice);
   virtual void newSlice(int style, int numSlices, double sliceAngle, double coneSections, Stringc filename);
   virtual vector<vec3d> tessTriangles(vector<vec3d> &tri);
   virtual vector<vec3d> tessTri(vec3d t1, vec3d t2, vec3d t3, int iterations);

   virtual void mergeRemoveOpenMeshes( MeshInfo* info );

	virtual vec3d getVertex3d(int surf, double x, double p, int r);
	virtual void  getVertexVec(vector< VertexID > *vertVec);

	virtual void createPrism( vector< TetraMassProp* >& tetraVec, TTri* tri, double len );
	virtual void addPointMass( TetraMassProp* pm )		{ pointMassVec.push_back( pm ); }
	vector< TetraMassProp* > pointMassVec;

	virtual void waterTightCheck(FILE* fid);
	virtual void addHalfBox();



};

#endif
