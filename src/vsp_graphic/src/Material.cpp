#include "OpenGLHeaders.h"

#include "Material.h"

namespace VSPGraphic
{
Material::Material()
{
    ambient[0] = ambient[1] = ambient[2] = 0.2f;
    ambient[3] = 1.0f;

    diffuse[0] = diffuse[1] = diffuse[2] = 0.8f;
    diffuse[3] = 1.0f;

    specular[0] = specular[1] = specular[2] = 0.0f;
    specular[3] = 1.0f;

    emission[0] = emission[1] = emission[2] = 0.0f;
    emission[3] = 1.0f;

    shininess = 128.0;
}
Material::Material(float ambi[], float diff[], float spec[], float emis[], float shin)
{
    ambient[0] = ambi[0]; ambient[1] = ambi[1]; ambient[2] = ambi[2]; ambient[3] = ambi[3];
    diffuse[0] = diff[0]; diffuse[1] = diff[1]; diffuse[2] = diff[2]; diffuse[3] = diff[3];
    specular[0] = spec[0]; specular[1] = spec[1]; specular[2] = spec[2]; specular[3] = spec[3];
    emission[0] = emis[0]; emission[1] = emis[1]; emission[2] = emis[2]; emission[3] = emis[3];
    shininess = shin;
}
Material::~Material()
{
}

void Material::bind()
{
    glMaterialfv( GL_FRONT_AND_BACK, GL_AMBIENT, ambient );
    glMaterialfv( GL_FRONT_AND_BACK, GL_DIFFUSE, diffuse );
    glMaterialfv( GL_FRONT_AND_BACK, GL_SPECULAR, specular );
    glMaterialfv( GL_FRONT_AND_BACK, GL_EMISSION, emission );
    glMaterialf( GL_FRONT_AND_BACK, GL_SHININESS, shininess );
}
}