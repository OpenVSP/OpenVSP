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