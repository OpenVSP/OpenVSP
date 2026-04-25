//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//

#ifdef WIN32
#define _HAS_STD_BYTE 0
#include <windows.h>
#endif

#define _USE_MATH_DEFINES
#include <cmath>

#include "Vehicle.h"
#include "VehicleMgr.h"

#include "TMesh.h"
#include "PGMesh.h"
#include "TMeshSweptVolume.h"

#include "tri_tri_intersect.h"
#include "predicates.h"
#include "Mathematics/ConvexHull3.h"
#include "Mathematics/DistRay3Triangle3.h"
#include "Mathematics/Ray.h"
#include "Mathematics/Triangle.h"

#include "Geom.h"
#include "SubSurfaceMgr.h"
#include "PntNodeMerge.h"
#include "VspCurve.h" // for #define TMAGIC

#include "triangle.h"
#include "triangle_api.h"

#include "VspUtil.h"

#include "delabella.h"
#include "StlHelper.h"

void WriteStl( const string &file_name, const vector< TMesh* >& meshVec )
{
    Matrix4d mat;

    FILE* fid = fopen( file_name.c_str(), "w" );
    if ( fid )
    {
        fprintf( fid, "solid\n" );

        for ( int i = 0; i < (int) meshVec.size(); i++ )
        {
            meshVec[i]->WriteSTLTris( fid, mat );
        }

        fprintf( fid, "endsolid\n" );
        fclose( fid );
    }
}

void WriteInsideMStl( const string &file_name, const vector< TMesh* >& meshVec, int minside )
{
    Matrix4d mat;

    FILE* fid = fopen( file_name.c_str(), "w" );
    if ( fid )
    {
        fprintf( fid, "solid\n" );

        for ( int i = 0; i < (int) meshVec.size(); i++ )
        {
            meshVec[i]->WriteInsideMStlTris( fid, mat, minside );
        }

        fprintf( fid, "endsolid\n" );
        fclose( fid );
    }
}

void WriteIgnoredSTL( const string &file_name, const vector< TMesh* >& meshVec )
{
    Matrix4d mat;

    FILE* fid = fopen( file_name.c_str(), "w" );
    if ( fid )
    {
        fprintf( fid, "solid\n" );

        for ( int i = 0; i < (int) meshVec.size(); i++ )
        {
            meshVec[i]->WriteIgnoredSTLTris( fid, mat );
        }

        fprintf( fid, "endsolid\n" );
        fclose( fid );
    }
}

void WriteStl( const string &file_name, TMesh* tm )
{
    Matrix4d mat;

    FILE* fid = fopen( file_name.c_str(), "w" );
    if ( fid && tm )
    {
        fprintf( fid, "solid\n" );

        tm->WriteSTLTris( fid, mat );

        fprintf( fid, "endsolid\n" );
        fclose( fid );
    }
}

void WriteInsideMStl( const string &file_name, TMesh* tm, int minside )
{
    Matrix4d mat;

    FILE* fid = fopen( file_name.c_str(), "w" );
    if ( fid && tm )
    {
        fprintf( fid, "solid\n" );

        tm->WriteInsideMStlTris( fid, mat, minside );

        fprintf( fid, "endsolid\n" );
        fclose( fid );
    }
}



void DumpMeshes( const vector < TMesh* > &tmv, const string &prefix )
{
    for ( int i = 0 ; i < ( int )tmv.size() ; i++ )
    {
        PGMulti pgmulti;
        PGMesh *pgm = pgmulti.GetActiveMesh();
        pgm->BuildFromTMesh( tmv[ i ] );

        char buf[255];
        Matrix4d mat;
        FILE *file_id = NULL;

        snprintf( buf, sizeof( buf ), "%s_%d.vspgeom", prefix.c_str(), i );
        file_id = fopen( buf, "w" );
        pgm->WriteVSPGeom( file_id, mat );
        fclose( file_id );
    }
}

//==== Create a Prism Made of Tetras - Extrude Tri +- len/2 ====//
void CreatePrism( vector< TetraMassProp* >& tetraVec, TTri* tri, double len, int idir )
{
    vec3d p0 = tri->m_N0->m_Pnt;
    vec3d p1 = tri->m_N1->m_Pnt;
    vec3d p2 = tri->m_N2->m_Pnt;
    p0.offset_i( len / 2.0, idir );
    p1.offset_i( len / 2.0, idir );
    p2.offset_i( len / 2.0, idir );

    vec3d p3 = tri->m_N0->m_Pnt;
    vec3d p4 = tri->m_N1->m_Pnt;
    vec3d p5 = tri->m_N2->m_Pnt;
    p3.offset_i( -len / 2.0, idir );
    p4.offset_i( -len / 2.0, idir );
    p5.offset_i( -len / 2.0, idir );

    tetraVec.push_back( new TetraMassProp( tri->m_GeomID, tri->m_Density, p0, p2, p1, p3 ) );
    tetraVec.push_back( new TetraMassProp( tri->m_GeomID, tri->m_Density, p2, p3, p5, p1 ) );
    tetraVec.push_back( new TetraMassProp( tri->m_GeomID, tri->m_Density, p5, p3, p4, p1 ) );
}

void BuildTriVec( const TMesh* mesh, vector< TTri* > &trivec )
{
    for ( int t = 0 ; t < ( int )mesh->m_TVec.size() ; t++ )
    {
        TTri* tri = mesh->m_TVec[t];
        if ( tri->m_SplitVec.size() )
        {
            for ( int s = 0 ; s < ( int )tri->m_SplitVec.size() ; s++ )
            {
                if ( !tri->m_SplitVec[s]->m_IgnoreTriFlag )
                {
                    trivec.push_back( tri->m_SplitVec[s] );
                }
            }
        }
        else if ( !tri->m_IgnoreTriFlag )
        {
            trivec.push_back( tri );
        }
    }
}

void BuildTriVec( const vector< TMesh* > &meshvec, vector< TTri* > &trivec )
{
    for ( int m = 0 ; m < ( int )meshvec.size() ; m++ )
    {
        BuildTriVec( meshvec[m], trivec );
    }
}

void IndexTriVec( vector< TTri* > &trivec, vector< TNode* > &nodvec )
{
    //==== Collect All Points ====//
    vector< TNode* > allNodeVec;
    allNodeVec.reserve( trivec.size() * 3 );
    for ( int t = 0 ; t < ( int )trivec.size() ; t++ )
    {
        trivec[t]->m_N0->m_ID = ( int )allNodeVec.size();
        allNodeVec.push_back( trivec[t]->m_N0 );
        trivec[t]->m_N1->m_ID = ( int )allNodeVec.size();
        allNodeVec.push_back( trivec[t]->m_N1 );
        trivec[t]->m_N2->m_ID = ( int )allNodeVec.size();
        allNodeVec.push_back( trivec[t]->m_N2 );
    }
    BndBox bb;
    vector< vec3d > allPntVec( allNodeVec.size() );
    for ( int i = 0 ; i < ( int )allNodeVec.size() ; i++ )
    {
        allPntVec[i] = allNodeVec[i]->m_Pnt;
        bb.Update( allPntVec[i] );
    }

    if ( allPntVec.size() == 0 )
    {
        return;
    }

    //==== Build Map ====//
    PntNodeCloud pnCloud;
    pnCloud.AddPntNodes( allPntVec );

    //==== Compute Tol ====//
    double tol = bb.GetLargestDist() * 1.0e-10;
    if ( tol < DBL_EPSILON )
    {
        tol = DBL_EPSILON;
    }

    //==== Use NanoFlann to Find Close Points and Group ====//
    IndexPntNodes( pnCloud, tol );

    //==== Load Used Nodes ====//
    nodvec.reserve( pnCloud.m_NumUsedPts );
    for ( int i = 0 ; i < ( int )allNodeVec.size() ; i++ )
    {
        if ( pnCloud.UsedNode( i ) )
        {
            nodvec.push_back( allNodeVec[i] );
        }
    }

    //==== Set Adjusted Node IDs ====//
    for ( int i = 0 ; i < ( int )allNodeVec.size() ; i++ )
    {
        allNodeVec[i]->m_ID = pnCloud.GetNodeUsedIndex( i );
    }
}

void IgnoreDegenTris( vector< TTri* > &trivec )
{
    vector< TTri* > goodTriVec;
    goodTriVec.reserve( trivec.size() );
    for ( int t = 0 ; t < ( int )trivec.size() ; t++ )
    {
        TTri* ttri = trivec[t];
        if ( ttri )
        {
            if ( ttri->m_N0->m_ID != ttri->m_N1->m_ID &&
                 ttri->m_N0->m_ID != ttri->m_N2->m_ID &&
                 ttri->m_N1->m_ID != ttri->m_N2->m_ID )
            {
                goodTriVec.push_back( ttri );
            }
        }
    }
    swap( trivec, goodTriVec );
}

// Note: these vectors still point into the base tmv/slicevec.  Tris and nodes
// are not changed beyond assignment of m_ID values.
// nodvec contains only the unique TNode's, but trivec still contains
// pointers to TNode's not in nodvec.
void BuildIndexedMesh( const vector< TMesh* > &tmv, const vector< TMesh* > &slicevec,
                       bool viewMesh, bool viewSlice,
                       vector< TTri* > &trivec, vector< TNode* > &nodvec )
{
    trivec.clear();
    nodvec.clear();

    if ( viewMesh )
    {
        BuildTriVec( tmv, trivec );
    }

    if ( viewSlice )
    {
        BuildTriVec( slicevec, trivec );
    }

    IndexTriVec( trivec, nodvec );

    IgnoreDegenTris( trivec );
}

//=============================================================================
// Indexed Mesh API
// These functions operate on a pre-built indexed representation of a mesh
// (vector<TTri*> + vector<TNode*>).  They may eventually become part of a
// dedicated IndexedTriMesh class.
//=============================================================================

void WriteStlByTag( FILE* file_id, int tag, const vector< TTri* > &trivec )
{
    for ( int i = 0 ; i < ( int )trivec.size() ; i++ )
    {
        TTri* ttri = trivec[i];
        if ( SubSurfaceMgr.GetTag( ttri->m_Tags ) == tag )
        {
            vec3d p0 = ttri->m_N0->m_Pnt;
            vec3d p1 = ttri->m_N1->m_Pnt;
            vec3d p2 = ttri->m_N2->m_Pnt;
            vec3d v10 = p1 - p0;
            vec3d v20 = p2 - p1;
            vec3d norm = cross( v10, v20 );
            norm.normalize();

            fprintf( file_id, " facet normal  %2.10le %2.10le %2.10le\n",  norm.x(), norm.y(), norm.z() );
            fprintf( file_id, "   outer loop\n" );
            fprintf( file_id, "     vertex %2.10le %2.10le %2.10le\n", p0.x(), p0.y(), p0.z() );
            fprintf( file_id, "     vertex %2.10le %2.10le %2.10le\n", p1.x(), p1.y(), p1.z() );
            fprintf( file_id, "     vertex %2.10le %2.10le %2.10le\n", p2.x(), p2.y(), p2.z() );
            fprintf( file_id, "   endloop\n" );
            fprintf( file_id, " endfacet\n" );
        }
    }
}

void WriteNascartPnts( FILE* fp, const vector< TNode* > &nodvec, const Matrix4d &xfm )
{
    vec3d v;
    for ( int i = 0 ; i < ( int )nodvec.size() ; i++ )
    {
        TNode* tnode = nodvec[i];
        if ( tnode )
        {
            v = xfm.xform( tnode->m_Pnt );
            fprintf( fp, "%16.10g %16.10g %16.10g\n", v.x(), v.z(), -v.y() );
        }
    }
}

void WriteCart3DPnts( FILE* fp, const vector< TNode* > &nodvec, const Matrix4d &xfm )
{
    vec3d v;
    for ( int i = 0 ; i < ( int )nodvec.size() ; i++ )
    {
        TNode* tnode = nodvec[i];
        if ( tnode )
        {
            v = xfm.xform( tnode->m_Pnt );
            fprintf( fp, "%16.10g %16.10g %16.10g\n", v.x(), v.y(), v.z() );
        }
    }
}

void WriteOBJPnts( FILE* fp, const vector< TNode* > &nodvec, const Matrix4d &xfm )
{
    vec3d v;
    for ( int i = 0 ; i < ( int )nodvec.size() ; i++ )
    {
        TNode* tnode = nodvec[i];
        if ( tnode )
        {
            v = xfm.xform( tnode->m_Pnt );
            fprintf( fp, "v %16.10g %16.10g %16.10g\n", v.x(), v.y(), v.z() );
        }
    }
}

void WriteVSPGeomPnts( FILE* file_id, const vector< TNode* > &nodvec, const Matrix4d &xfm )
{
    vec3d v;
    for ( int i = 0 ; i < ( int )nodvec.size() ; i++ )
    {
        TNode* tnode = nodvec[i];
        v = xfm.xform( tnode->m_Pnt );
        fprintf( file_id, "%16.10g %16.10g %16.10g\n", v.x(), v.y(), v.z() );
    }
}

int WriteGMshNodes( FILE* fp, int node_offset, const vector< TNode* > &nodvec, const Matrix4d &xfm )
{
    vec3d v;
    for ( int i = 0 ; i < ( int )nodvec.size() ; i++ )
    {
        TNode* tnode = nodvec[i];
        if ( tnode )
        {
            v = xfm.xform( tnode->m_Pnt );
            fprintf( fp, "%d %16.10f %16.10f %16.10f\n", i + node_offset + 1, v.x(), v.y(), v.z() );
        }
    }
    return node_offset + ( int )nodvec.size();
}

void WriteFacetNodes( FILE* fp, const vector< TNode* > &nodvec, const Matrix4d &xfm )
{
    vec3d v;
    for ( int i = 0; i < ( int )nodvec.size(); i++ )
    {
        TNode* tnode = nodvec[i];
        v = xfm.xform( tnode->m_Pnt );
        fprintf( fp, "%16.10g %16.10g %16.10g\n", v.x(), v.y(), v.z() );
    }
}

int WriteNascartTris( FILE* fp, int off, const vector< TTri* > &trivec, const vector< TNode* > &nodvec )
{
    for ( int t = 0 ; t < ( int )trivec.size() ; t++ )
    {
        TTri* ttri = trivec[t];
        if ( ttri )
        {
            fprintf( fp, "%d %d %d %d.0\n", ttri->m_N0->m_ID + 1 + off, ttri->m_N2->m_ID + 1 + off,
                     ttri->m_N1->m_ID + 1 + off, SubSurfaceMgr.GetTag( ttri->m_Tags ) );
        }
    }
    return off + ( int )nodvec.size();
}

int WriteCart3DTris( FILE* fp, int off, const vector< TTri* > &trivec, const vector< TNode* > &nodvec )
{
    for ( int t = 0 ; t < ( int )trivec.size() ; t++ )
    {
        TTri* ttri = trivec[t];
        if ( ttri )
        {
            fprintf( fp, "%d %d %d\n", ttri->m_N0->m_ID + 1 + off, ttri->m_N1->m_ID + 1 + off, ttri->m_N2->m_ID + 1 + off );
        }
    }
    return off + ( int )nodvec.size();
}

int WriteOBJTris( FILE* fp, int off, const vector< TTri* > &trivec, const vector< TNode* > &nodvec )
{
    for ( int t = 0 ; t < ( int )trivec.size() ; t++ )
    {
        TTri* ttri = trivec[t];
        if ( ttri )
        {
            fprintf( fp, "f %d %d %d\n", ttri->m_N0->m_ID + 1 + off, ttri->m_N1->m_ID + 1 + off, ttri->m_N2->m_ID + 1 + off );
        }
    }
    return off + ( int )nodvec.size();
}

int WriteVSPGeomTris( FILE* file_id, int offset, const vector< TTri* > &trivec, const vector< TNode* > &nodvec )
{
    for ( int t = 0 ; t < ( int )trivec.size() ; t++ )
    {
        TTri* ttri = trivec[t];
        fprintf( file_id, "3 %d %d %d\n", ttri->m_N0->m_ID + 1 + offset, ttri->m_N1->m_ID + 1 + offset, ttri->m_N2->m_ID + 1 + offset );
    }
    return offset + ( int )nodvec.size();
}

int WriteVSPGeomAlternateTris( FILE* file_id, int noffset, int &tcount, const vector< TTri* > &trivec, const vector< TNode* > &nodvec )
{
    for ( int t = 0 ; t < ( int )trivec.size() ; t++ )
    {
        TTri* ttri = trivec[t];
        fprintf( file_id, "%d 1 %d %d %d\n", tcount, ttri->m_N0->m_ID + 1 + noffset, ttri->m_N1->m_ID + 1 + noffset, ttri->m_N2->m_ID + 1 + noffset );
        tcount++;
    }
    return noffset + ( int )nodvec.size();
}

int WriteGMshTris( FILE* fp, int node_offset, int tri_offset, const vector< TTri* > &trivec )
{
    for ( int t = 0 ; t < ( int )trivec.size() ; t++ )
    {
        TTri* ttri = trivec[t];
        if ( ttri )
        {
            fprintf( fp, "%d 2 0 %d %d %d\n", t + tri_offset + 1,
                     ttri->m_N0->m_ID + 1 + node_offset, ttri->m_N2->m_ID + 1 + node_offset, ttri->m_N1->m_ID + 1 + node_offset );
        }
    }
    return tri_offset + ( int )trivec.size();
}

void WriteFacetTriParts( FILE* fp, int &offset, int &tri_count, int &part_count,
                         const vector< TMesh* > &tmv, const vector< TTri* > &trivec, const vector< TNode* > &nodvec )
{
    vector < string > geom_ID_vec;
    geom_ID_vec.resize( tmv.size() );
    for ( unsigned int i = 0; i < tmv.size(); i++ )
    {
        geom_ID_vec[i] = tmv[i]->m_OriginGeomID;
    }

    vector < int > tri_offset;
    int materialID = 0;
    vector < int > all_tag_vec = SubSurfaceMgr.GetAllTags();

    for ( unsigned int i = 0; i < all_tag_vec.size(); i++ )
    {
        int tag_count = 0;
        for ( unsigned int j = 0; j < trivec.size(); j++ )
        {
            if ( all_tag_vec[i] == SubSurfaceMgr.GetTag( trivec[j]->m_Tags ) )
            {
                tag_count++;
            }
        }
        tri_offset.push_back( tag_count );
    }

    for ( int j = tri_offset.size() - 1; j >= 0; j-- )
    {
        if ( tri_offset[j] == 0 )
        {
            tri_offset.erase( tri_offset.begin() + j );
            all_tag_vec.erase( all_tag_vec.begin() + j );
            j--;
        }
    }

    fprintf( fp, "%ld \n", tri_offset.size() );

    for ( unsigned int i = 0; i < all_tag_vec.size(); i++ )
    {
        int curr_tag = all_tag_vec[i];
        bool new_section = true;

        for ( unsigned int j = 0; j < trivec.size(); j++ )
        {
            if ( curr_tag == SubSurfaceMgr.GetTag( trivec[j]->m_Tags ) )
            {
                if ( new_section )
                {
                    string name = SubSurfaceMgr.GetTagNames( trivec[j]->m_Tags );
                    fprintf( fp, "%s\n", name.c_str() );
                    fprintf( fp, "%d 3\n", tri_offset[i] );
                    new_section = false;
                }

                TTri* ttri = trivec[j];
                tri_count++;
                fprintf( fp, "%d %d %d %d %u %d\n", ttri->m_N0->m_ID + 1 + offset, ttri->m_N1->m_ID + 1 + offset,
                         ttri->m_N2->m_ID + 1 + offset, materialID, i + 1 + part_count, tri_count );
            }
        }
    }

    part_count += tri_offset.size();
    offset += ( int )nodvec.size();
}

int WriteNascartParts( FILE* fp, int off, const vector< TMesh* > &tmv )
{
    for ( int m = 0 ; m < ( int )tmv.size() ; m++ )
    {
        fprintf( fp, "%d.0  %s  0\n", off + m, tmv[m]->m_NameStr.c_str() );
    }
    return off + ( int )tmv.size();
}

int WriteCart3DParts( FILE* fp, const vector< TTri* > &trivec )
{
    for ( int t = 0 ; t < ( int )trivec.size() ; t++ )
    {
        fprintf( fp, "%d \n", SubSurfaceMgr.GetTag( trivec[t]->m_Tags ) );
    }
    return 0;
}

int WriteVSPGeomParts( FILE* file_id, const vector< TTri* > &trivec )
{
    for ( int t = 0 ; t < ( int )trivec.size() ; t++ )
    {
        TTri* ttri = trivec[t];
        int tag  = SubSurfaceMgr.GetTag( ttri->m_Tags );
        int part = SubSurfaceMgr.GetPart( ttri->m_Tags );
        double uscale = SubSurfaceMgr.m_CompUscale[ part - 1 ];
        double wscale = SubSurfaceMgr.m_CompWscale[ part - 1 ];
        fprintf( file_id, "%d %d %16.10g %16.10g %16.10g %16.10g %16.10g %16.10g\n", part, tag,
                 ttri->m_N0->m_UWPnt.x() / uscale, ttri->m_N0->m_UWPnt.y() / wscale,
                 ttri->m_N1->m_UWPnt.x() / uscale, ttri->m_N1->m_UWPnt.y() / wscale,
                 ttri->m_N2->m_UWPnt.x() / uscale, ttri->m_N2->m_UWPnt.y() / wscale );
    }
    return 0;
}

int WriteVSPGeomAlternateParts( FILE* file_id, int &tcount, const vector< TTri* > &trivec )
{
    for ( int t = 0 ; t < ( int )trivec.size() ; t++ )
    {
        TTri* ttri = trivec[t];
        int tag  = SubSurfaceMgr.GetTag( ttri->m_Tags );
        int part = SubSurfaceMgr.GetPart( ttri->m_Tags );
        double uscale = SubSurfaceMgr.m_CompUscale[ part - 1 ];
        double wscale = SubSurfaceMgr.m_CompWscale[ part - 1 ];
        fprintf( file_id, "%d %d %d %16.10g %16.10g %16.10g %16.10g %16.10g %16.10g\n", tcount, part, tag,
                 ttri->m_N0->m_UWPnt.x() / uscale, ttri->m_N0->m_UWPnt.y() / wscale,
                 ttri->m_N1->m_UWPnt.x() / uscale, ttri->m_N1->m_UWPnt.y() / wscale,
                 ttri->m_N2->m_UWPnt.x() / uscale, ttri->m_N2->m_UWPnt.y() / wscale );
        tcount++;
    }
    return 0;
}

int WriteVSPGeomPartTagTris( FILE* file_id, int tri_offset, int part, int tag, const vector< TTri* > &trivec )
{
    for ( int t = 0 ; t < ( int )trivec.size() ; t++ )
    {
        TTri* ttri = trivec[t];
        if ( SubSurfaceMgr.MatchPartAndTag( ttri->m_Tags, part, tag ) )
        {
            fprintf( file_id, "%d\n", t + tri_offset + 1 );
        }
    }
    return tri_offset + ( int )trivec.size();
}

int CountVSPGeomPartTagTris( int part, int tag, const vector< TTri* > &trivec )
{
    int count = 0;
    for ( int t = 0 ; t < ( int )trivec.size() ; t++ )
    {
        if ( SubSurfaceMgr.MatchPartAndTag( trivec[t]->m_Tags, part, tag ) )
        {
            count++;
        }
    }
    return count;
}

void WriteVSPGeomParents( FILE* file_id, int &tcount, const vector< TTri* > &trivec )
{
    for ( int i = 0; i < ( int )trivec.size(); i++ )
    {
        fprintf( file_id, "%d %d\n", tcount, tcount );
        tcount++;
    }
}


int WriteVSPGeomWakes( FILE* file_id, int offset, const vector< deque< TEdge > > &wakes, const vector< TNode* > &nodvec )
{
    int nwake = wakes.size();

    for ( int iwake = 0; iwake < nwake; iwake++ )
    {
        int iprt = 0;
        int iwe;
        int nwe = wakes[iwake].size();

        int part = 0;
        if ( nwe > 1 )
        {
            TTri *t = wakes[iwake][0].GetParTri();
            part = SubSurfaceMgr.GetPart( t->m_Tags );
        }

        fprintf( file_id, "%d %d ", nwe + 1, part );

        for ( iwe = 0; iwe < nwe; iwe++ )
        {
            fprintf( file_id, "%d", wakes[iwake][iwe].m_N0->m_ID + 1 + offset );

            if ( iprt < 9 )
            {
                fprintf( file_id, " " );
                iprt++;
            }
            else
            {
                fprintf( file_id, "\n" );
                iprt = 0;
            }
        }
        fprintf( file_id, "%d\n", wakes[iwake][iwe - 1].m_N1->m_ID + 1 + offset );
    }

    return offset + ( int )nodvec.size();
}

