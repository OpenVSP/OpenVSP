#ifndef _VSP_GRAPHIC_GLOBAL_SINGLETONS_H
#define _VSP_GRAPHIC_GLOBAL_SINGLETONS_H

#include "ShaderRepo.h"
#include "TextureRepo.h"

namespace VSPGraphic
{
/*!
* Shared shader resources.
*/
static ShaderRepo * GlobalShaderRepo()
{
    static ShaderRepo shaderRepo;
    return &shaderRepo;
}

/*!
* Shared texture resources.
*/
static TextureRepo * GlobalTextureRepo()
{
    static TextureRepo textureRepo;
    return &textureRepo;
}
}
#endif