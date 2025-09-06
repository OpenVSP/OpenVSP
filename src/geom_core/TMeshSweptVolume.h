//
// Created by Rob McDonald on 7/28/25.
//

#ifndef TMESHSWEPTVOLUME_H
#define TMESHSWEPTVOLUME_H

#include <vector>

class TMesh;
class vec3d;

TMesh * CreateTMeshCompGeomSweptVolumeTranslate( TMesh * tm, const vec3d & v );

TMesh * CreateTMeshPGMeshSweptVolumeTranslate( TMesh * tm, const vec3d & v );
TMesh * CreateTMeshPGMeshSweptVolumeTranslate( TMesh * tm, const std::vector < vec3d > & vvec );
#endif //TMESHSWEPTVOLUME_H
