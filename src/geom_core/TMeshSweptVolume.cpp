//
// Created by Rob McDonald on 7/28/25.
//

#include "TMeshSweptVolume.h"
#include "TMesh.h"
#include "MeshGeom.h"

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
