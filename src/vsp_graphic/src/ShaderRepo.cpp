#include <assert.h>
#include <cstddef>

#include "ShaderRepo.h"
#include "Shader.h"
#include "ShaderSources.h"

namespace VSPGraphic
{
ShaderRepo::ShaderRepo()
{
    // Initialize texture shaders.
    _shaderRepo.resize( NUM_OF_SHADERS );

    ShaderInfo sInfo;

    // Shader Info for render a single texture with lighting.
    sInfo.shaderPtr = NULL;
    sInfo.vShader = vShader_Texture01;
    sInfo.fShader = fShader_Texture01;

    _shaderRepo[SHADER_TEXTURE01] = sInfo;

    // Shader Info for render a single texture with no lighting.
    sInfo.shaderPtr = NULL;
    sInfo.vShader = vShader_Texture01_No_Lighting;
    sInfo.fShader = fShader_Texture01_No_Lighting;

    _shaderRepo[SHADER_TEXTURE01_NO_LIGHTING] = sInfo;

    // Shader Info for render two textures.  Texture colors are blended.
    sInfo.shaderPtr = NULL;
    sInfo.vShader = vShader_MultiTexture02;
    sInfo.fShader = fShader_MultiTexture02_Blending;

    _shaderRepo[SHADER_MULTITEXTURE02_BLEND] = sInfo;

    // Shader Info for render two textures.  Texture colors are layered.
    sInfo.shaderPtr = NULL;
    sInfo.vShader = vShader_MultiTexture02;
    sInfo.fShader = fShader_MultiTexture02_Layering;

    _shaderRepo[SHADER_MULTITEXTURE02_LAYERED] = sInfo;

    // Shader Info for render three textures.  Texture colors are blended.
    sInfo.shaderPtr = NULL;
    sInfo.vShader = vShader_MultiTexture03;
    sInfo.fShader = fShader_MultiTexture03_Blending;

    _shaderRepo[SHADER_MULTITEXTURE03_BLEND] = sInfo;

    // Shader Info for render three textures.  Texture colors are layered.
    sInfo.shaderPtr = NULL;
    sInfo.vShader = vShader_MultiTexture03;
    sInfo.fShader = fShader_MultiTexture03_Layering;

    _shaderRepo[SHADER_MULTITEXTURE03_LAYERED] = sInfo;

    // Shader Info for render four textures.  Texture colors are blended.
    sInfo.shaderPtr = NULL;
    sInfo.vShader = vShader_MultiTexture04;
    sInfo.fShader = fShader_MultiTexture04_Blending;

    _shaderRepo[SHADER_MULTITEXTURE04_BLEND] = sInfo;

    // Shader Info for render four textures.  Texture colors are layered.
    sInfo.shaderPtr = NULL;
    sInfo.vShader = vShader_MultiTexture04;
    sInfo.fShader = fShader_MultiTexture04_Layering;

    _shaderRepo[SHADER_MULTITEXTURE04_LAYERED] = sInfo;

    // Shader Info for render five textures.  Texture colors are blended.
    sInfo.shaderPtr = NULL;
    sInfo.vShader = vShader_MultiTexture05;
    sInfo.fShader = fShader_MultiTexture05_Blending;

    _shaderRepo[SHADER_MULTITEXTURE05_BLEND] = sInfo;

    // Shader Info for render five textures.  Texture colors are layered.
    sInfo.shaderPtr = NULL;
    sInfo.vShader = vShader_MultiTexture05;
    sInfo.fShader = fShader_MultiTexture05_Layering;

    _shaderRepo[SHADER_MULTITEXTURE05_LAYERED] = sInfo;

    // Shader Info for render six textures.  Texture colors are blended.
    sInfo.shaderPtr = NULL;
    sInfo.vShader = vShader_MultiTexture06;
    sInfo.fShader = fShader_MultiTexture06_Blending;

    _shaderRepo[SHADER_MULTITEXTURE06_BLEND] = sInfo;

    // Shader Info for render six textures.  Texture colors are layered.
    sInfo.shaderPtr = NULL;
    sInfo.vShader = vShader_MultiTexture06;
    sInfo.fShader = fShader_MultiTexture06_Layering;

    _shaderRepo[SHADER_MULTITEXTURE06_LAYERED] = sInfo;

    // Shader Info for render seven textures.  Texture colors are blended.
    sInfo.shaderPtr = NULL;
    sInfo.vShader = vShader_MultiTexture07;
    sInfo.fShader = fShader_MultiTexture07_Blending;

    _shaderRepo[SHADER_MULTITEXTURE07_BLEND] = sInfo;

    // Shader Info for render seven textures.  Texture colors are layered.
    sInfo.shaderPtr = NULL;
    sInfo.vShader = vShader_MultiTexture07;
    sInfo.fShader = fShader_MultiTexture07_Layering;

    _shaderRepo[SHADER_MULTITEXTURE07_LAYERED] = sInfo;

    // Shader Info for render eight textures.  Texture colors are blended.
    sInfo.shaderPtr = NULL;
    sInfo.vShader = vShader_MultiTexture08;
    sInfo.fShader = fShader_MultiTexture08_Blending;

    _shaderRepo[SHADER_MULTITEXTURE08_BLEND] = sInfo;

    // Shader Info for render eight textures.  Texture colors are layered.
    sInfo.shaderPtr = NULL;
    sInfo.vShader = vShader_MultiTexture08;
    sInfo.fShader = fShader_MultiTexture08_Layering;

    _shaderRepo[SHADER_MULTITEXTURE08_LAYERED] = sInfo;
}
ShaderRepo::~ShaderRepo()
{
    for( int i = 0; i < ( int )_shaderRepo.size(); i++ )
    {
        if( _shaderRepo[i].shaderPtr != NULL )
        {
            delete _shaderRepo[i].shaderPtr;
            _shaderRepo[i].shaderPtr = NULL;
        }
    }
}

Shader * ShaderRepo::getShader( VSPShaderEnum shaderType )
{
    switch( shaderType )
    {
    case SHADER_TEXTURE01:
        if( _shaderRepo[SHADER_TEXTURE01].shaderPtr == NULL )
        {
            _build( &_shaderRepo[SHADER_TEXTURE01] );
        }
        return _shaderRepo[SHADER_TEXTURE01].shaderPtr;

    case SHADER_TEXTURE01_NO_LIGHTING:
        if( _shaderRepo[SHADER_TEXTURE01_NO_LIGHTING].shaderPtr == NULL )
        {
            _build( &_shaderRepo[SHADER_TEXTURE01_NO_LIGHTING] );
        }
        return _shaderRepo[SHADER_TEXTURE01_NO_LIGHTING].shaderPtr;

    case SHADER_MULTITEXTURE02_BLEND:
        if( _shaderRepo[SHADER_MULTITEXTURE02_BLEND].shaderPtr == NULL )
        {
            _build( &_shaderRepo[SHADER_MULTITEXTURE02_BLEND] );
        }
        return _shaderRepo[SHADER_MULTITEXTURE02_BLEND].shaderPtr;

    case SHADER_MULTITEXTURE02_LAYERED:
        if( _shaderRepo[SHADER_MULTITEXTURE02_LAYERED].shaderPtr == NULL )
        {
            _build( &_shaderRepo[SHADER_MULTITEXTURE02_LAYERED] );
        }
        return _shaderRepo[SHADER_MULTITEXTURE02_LAYERED].shaderPtr;

    case SHADER_MULTITEXTURE03_BLEND:
        if( _shaderRepo[SHADER_MULTITEXTURE03_BLEND].shaderPtr == NULL )
        {
            _build( &_shaderRepo[SHADER_MULTITEXTURE03_BLEND] );
        }
        return _shaderRepo[SHADER_MULTITEXTURE03_BLEND].shaderPtr;

    case SHADER_MULTITEXTURE03_LAYERED:
        if( _shaderRepo[SHADER_MULTITEXTURE03_LAYERED].shaderPtr == NULL )
        {
            _build( &_shaderRepo[SHADER_MULTITEXTURE03_LAYERED] );
        }
        return _shaderRepo[SHADER_MULTITEXTURE03_LAYERED].shaderPtr;

    case SHADER_MULTITEXTURE04_BLEND:
        if( _shaderRepo[SHADER_MULTITEXTURE04_BLEND].shaderPtr == NULL )
        {
            _build( &_shaderRepo[SHADER_MULTITEXTURE04_BLEND] );
        }
        return _shaderRepo[SHADER_MULTITEXTURE04_BLEND].shaderPtr;

    case SHADER_MULTITEXTURE04_LAYERED:
        if( _shaderRepo[SHADER_MULTITEXTURE04_LAYERED].shaderPtr == NULL )
        {
            _build( &_shaderRepo[SHADER_MULTITEXTURE04_LAYERED] );
        }
        return _shaderRepo[SHADER_MULTITEXTURE04_LAYERED].shaderPtr;

    case SHADER_MULTITEXTURE05_BLEND:
        if( _shaderRepo[SHADER_MULTITEXTURE05_BLEND].shaderPtr == NULL )
        {
            _build( &_shaderRepo[SHADER_MULTITEXTURE05_BLEND] );
        }
        return _shaderRepo[SHADER_MULTITEXTURE05_BLEND].shaderPtr;

    case SHADER_MULTITEXTURE05_LAYERED:
        if( _shaderRepo[SHADER_MULTITEXTURE05_LAYERED].shaderPtr == NULL )
        {
            _build( &_shaderRepo[SHADER_MULTITEXTURE05_LAYERED] );
        }
        return _shaderRepo[SHADER_MULTITEXTURE05_LAYERED].shaderPtr;

    case SHADER_MULTITEXTURE06_BLEND:
        if( _shaderRepo[SHADER_MULTITEXTURE06_BLEND].shaderPtr == NULL )
        {
            _build( &_shaderRepo[SHADER_MULTITEXTURE06_BLEND] );
        }
        return _shaderRepo[SHADER_MULTITEXTURE06_BLEND].shaderPtr;

    case SHADER_MULTITEXTURE06_LAYERED:
        if( _shaderRepo[SHADER_MULTITEXTURE06_LAYERED].shaderPtr == NULL )
        {
            _build( &_shaderRepo[SHADER_MULTITEXTURE06_LAYERED] );
        }
        return _shaderRepo[SHADER_MULTITEXTURE06_LAYERED].shaderPtr;

    case SHADER_MULTITEXTURE07_BLEND:
        if( _shaderRepo[SHADER_MULTITEXTURE07_BLEND].shaderPtr == NULL )
        {
            _build( &_shaderRepo[SHADER_MULTITEXTURE07_BLEND] );
        }
        return _shaderRepo[SHADER_MULTITEXTURE07_BLEND].shaderPtr;

    case SHADER_MULTITEXTURE07_LAYERED:
        if( _shaderRepo[SHADER_MULTITEXTURE07_LAYERED].shaderPtr == NULL )
        {
            _build( &_shaderRepo[SHADER_MULTITEXTURE07_LAYERED] );
        }
        return _shaderRepo[SHADER_MULTITEXTURE07_LAYERED].shaderPtr;

    case SHADER_MULTITEXTURE08_BLEND:
        if( _shaderRepo[SHADER_MULTITEXTURE08_BLEND].shaderPtr == NULL )
        {
            _build( &_shaderRepo[SHADER_MULTITEXTURE08_BLEND] );
        }
        return _shaderRepo[SHADER_MULTITEXTURE08_BLEND].shaderPtr;

    case SHADER_MULTITEXTURE08_LAYERED:
        if( _shaderRepo[SHADER_MULTITEXTURE08_LAYERED].shaderPtr == NULL )
        {
            _build( &_shaderRepo[SHADER_MULTITEXTURE08_LAYERED] );
        }
        return _shaderRepo[SHADER_MULTITEXTURE08_LAYERED].shaderPtr;

    default:
        // Shouldn't reach here.
        assert( false );
        return NULL;
    }
}

void ShaderRepo::_build( ShaderInfo * sInfo )
{
    if( sInfo->shaderPtr != NULL )
    {
        return;
    }
    sInfo->shaderPtr = new Shader( sInfo->vShader, sInfo->fShader );
}
}
