//
// Created by Rob McDonald on 7/28/25.
//

#include "TMeshSweptVolume.h"
#include "TMesh.h"
#include "MeshGeom.h"

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
