//
// Created by Rob McDonald on 7/28/25.
//

#include "TMeshSweptVolume.h"
#include "TMesh.h"
#include "MeshGeom.h"
#include "PGMesh.h"

#include <igl/winding_number.h>
#include <igl/fast_winding_number.h>
#include <Eigen/Core>

void TMeshToIGL( TMesh * tm, Eigen::MatrixXi &F, Eigen::MatrixXd &V )
{
    vector< TTri* > trivec;
    vector< TNode* > nodevec;

    MeshGeom::BuildTriVec( tm, trivec );
    MeshGeom::IndexTriVec( trivec, nodevec );

    MeshGeom::IgnoreDegenTris( trivec );

    V.resize( nodevec.size(), 3 );
    for ( int i = 0; i < nodevec.size(); ++i )
    {
        for ( int j = 0; j < 3; ++j )
        {
            V( i, j ) = nodevec[ i ]->m_Pnt.v[ j ];
        }
    }

    F.resize( trivec.size(), 3 );
    for ( int i = 0; i < trivec.size(); ++i )
    {
        F( i, 0 ) = trivec[i]->m_N0->m_ID;
        F( i, 1 ) = trivec[i]->m_N1->m_ID;
        F( i, 2 ) = trivec[i]->m_N2->m_ID;
    }
}

void TMeshToIGL( TMesh * tm, vector< TTri* > &trivec, Eigen::MatrixXi &F, Eigen::MatrixXd &V, Eigen::MatrixXd &C )
{
    vector< TNode* > nodevec;

    MeshGeom::BuildTriVec( tm, trivec );
    MeshGeom::IndexTriVec( trivec, nodevec );

    MeshGeom::IgnoreDegenTris( trivec );

    V.resize( nodevec.size(), 3 );
    for ( int i = 0; i < nodevec.size(); ++i )
    {
        for ( int j = 0; j < 3; ++j )
        {
            V( i, j ) = nodevec[ i ]->m_Pnt.v[ j ];
        }
    }

    F.resize( trivec.size(), 3 );
    C.resize( trivec.size(), 3 );
    for ( int i = 0; i < trivec.size(); ++i )
    {
        F( i, 0 ) = trivec[i]->m_N0->m_ID;
        F( i, 1 ) = trivec[i]->m_N1->m_ID;
        F( i, 2 ) = trivec[i]->m_N2->m_ID;

        vec3d n = trivec[i]->m_Norm;
        n.normalize();

        vec3d c = 1e-6 * n + ( trivec[i]->m_N0->m_Pnt + trivec[i]->m_N1->m_Pnt + trivec[i]->m_N2->m_Pnt ) / 3.0;

        C( i, 0 ) = c.v[ 0 ];
        C( i, 1 ) = c.v[ 1 ];
        C( i, 2 ) = c.v[ 2 ];
    }
}

TMesh * IGLToTMesh( const Eigen::MatrixXi &F, const Eigen::MatrixXd &V )
{
    TMesh * tm = new TMesh();

    for ( int i = 0; i < F.rows(); ++i )
    {
        int i0 = F( i, 0 );
        int i1 = F( i, 1 );
        int i2 = F( i, 2 );
        vec3d v0 = vec3d( V( i0, 0 ), V( i0, 1 ), V( i0, 2 ) );
        vec3d v1 = vec3d( V( i1, 0 ), V( i1, 1 ), V( i1, 2 ) );
        vec3d v2 = vec3d( V( i2, 0 ), V( i2, 1 ), V( i2, 2 ) );
        tm->AddTri( v0, v1, v2, i );
    }

    return tm;
}

TMesh * CreateTMeshCompGeomSweptVolumeTranslate( TMesh * tm, const vec3d & v )
{
    const int time_steps = 20;

    vector < TMesh * > tmv;

    for ( int it = 0; it < time_steps; ++it )
    {
        double t = ( ( double )it ) / ( ( double ) ( time_steps - 1 ) );

        Matrix4d m;
        m.translatev( t * v );

        TMesh *tmi = new TMesh();
        tmi->CopyFlatten( tm );

        tmi->Transform( m );

        tmv.push_back( tmi );
    }

    MeshUnion( tmv );
    FlattenTMeshVec( tmv );

    TMesh * tm_sv = MergeTMeshVec( tmv );

    DeleteTMeshVec( tmv );

    return tm_sv;
}

TMesh * CreateTMeshPGMeshSweptVolumeTranslate( TMesh * tm, const vec3d & v )
{
    // Copy tm to PGMesh
    PGMulti pgmulti;
    PGMesh *pgm = pgmulti.GetActiveMesh();
    pgm->BuildFromTMesh( tm );
    // Merge coincident nodes.  This fixes the mesh topology, changing from disconnected
    // triangles with duplicate nodes and edges to a coehesive mesh that can be walked.
    pgm->MergeCoincidentNodes();

    // Identify silhouette edge loops.  Silhouette edges are those where the dot product
    // between the face normal vector and v differs in sign for the two faces across the edge.
    // This step requires the walkable mesh data structure.
    vector < vector < PGEdge * > > silhouetteLoopVec;
    pgm->IdentifySilhouettes( silhouetteLoopVec, v );

    // Identify shadow faces.  Shadow faces are those where the dot product between the face
    // normal vector and v is positive.
    vector < PGFace* > shadow;
    pgm->IdentifyShadow( shadow, v );

    // Copy node list to vector for easy access by index.
    vector < PGNode* > nvec( pgm->m_NodeList.begin(), pgm->m_NodeList.end() );
    int nnod = nvec.size();

    int nlevel = 10;
    nvec.reserve( ( nlevel + 1 ) * nnod );

    for ( int ilevel = 0; ilevel < nlevel; ++ilevel )
    {
        vec3d dv = ( ( double ) ( ilevel + 1 ) / ( double ) nlevel ) * v;
        // Add duplicate shifted nodes
        for ( int i = 0; i < nnod ; i++ )
        {
            // Add new shifted point
            PGPoint * pnew = pgmulti.AddPoint( nvec[ i ]->m_Pt->m_Pnt + dv );
            // Make point ID match vector index
            nvec[ i ]->m_Pt->m_ID = i;
            pnew->m_ID = i + nnod * ( ilevel + 1 );
            // Make node for shifted point
            nvec.push_back( pgm->AddNode( pnew ) );
        }
    }

    // Move shadow faces to end of sweep.
    // This actually creates new faces in the new position and destroys the old ones.  This
    // is an easier way to preserve the mesh topology through the transformation.
    pgm->MoveFaces( shadow, nvec, nnod * nlevel );

    for ( int ilevel = 0; ilevel < nlevel; ++ilevel )
    {
        // Extrude the silouette loops to create candidate 'sides' of the swept volume.
        pgm->ExtrudeEdgeLoopVec( silhouetteLoopVec, nvec, nnod * ilevel, nnod * ( ilevel + 1 ) );
    }

    // The shadowed side of the original mesh and the non-shadowed side of the end-point
    // mesh each result in unused points / nodes.
    pgm->CleanUnused();


    // Convert pgm to TMesh for intersection
    TMesh *tm_sv = new TMesh();
    tm_sv->MakeFromPGMesh( pgm );

    // Prepare mesh for intersection
    tm_sv->LoadBndBox();
    // Self-intersect
    tm_sv->Intersect( tm_sv );
    // Split intersected triangles
    tm_sv->Split();

    // Create IGL triangulation
    Eigen::MatrixXi F;
    Eigen::MatrixXd V, C, W;
    vector< TTri* > trivec;
    TMeshToIGL( tm_sv, trivec, F, V, C );

    // Calculate winding number for each face using parallel and clever IGL algorithm.
    igl::winding_number( V, F, C, W );
    // igl::fast_winding_number( V, F, C, W );

    // Use winding number to classify tris
    for ( int i = 0; i < trivec.size(); i++ )
    {
        if ( round( W( i ) ) != 0 )
        {
            trivec[i]->m_IgnoreTriFlag = true;
        }
    }

    // Flatten mesh data structure
    tm_sv->FlattenInPlace();

    return tm_sv;
}
