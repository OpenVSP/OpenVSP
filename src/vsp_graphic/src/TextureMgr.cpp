#include "OpenGLHeaders.h"
#include "GraphicSingletons.h"

#include "TextureMgr.h"
#include "Texture2D.h"

#include "Shader.h"
#include "Lighting.h"

namespace VSPGraphic
{
static unsigned int _idTracker = 0;
static std::vector<unsigned int> _recycleBin;

TextureMgr::TextureMgr()
{
    _lights = NULL;
    _textureMode = Common::VSP_TEXTURE_LAYERED;
}
TextureMgr::~TextureMgr()
{
    for( int i = 0; i < ( int )_coordList.size(); i++ )
    {
        // recycle ids.
        _recycleBin.push_back( _coordList[i].id );
    }
    _coordList.clear();
}

unsigned int TextureMgr::add( Texture2D * texture )
{
    if( ( int )_coordList.size() <= getMaxCombineTextures() )
    {
        TexInfo info;

        info.id = _generateId();
        info.texture = texture;
        info.texCoordMat = glm::mat4( 1.0 );
        info.flipU = false;
        info.flipW = false;
        info.texAlpha = 1.0f;

        _coordList.push_back( info );

        return info.id;
    }
    return 0xFFFFFFFF;
}

void TextureMgr::remove( unsigned int id )
{
    for( int i = 0; i < ( int )_coordList.size(); i++ )
    {
        if( _coordList[i].id == id )
        {
            _recycleBin.push_back( id );

            _coordList.erase( _coordList.begin() + i );
            break;
        }
    }
}

void TextureMgr::translate( unsigned int id, float s, float t, float r )
{
    for( int i = 0; i < ( int )_coordList.size(); i++ )
    {
        if( _coordList[i].id == id )
        {
            _coordList[i].texCoordMat[3][0] = s;
            _coordList[i].texCoordMat[3][1] = t;
            _coordList[i].texCoordMat[3][2] = r;
            break;
        }
    }
}

void TextureMgr::scale( unsigned int id, float sRatio, float tRatio, float rRatio )
{
    for( int i = 0; i < ( int )_coordList.size(); i++ )
    {
        if( _coordList[i].id == id )
        {
            _coordList[i].texCoordMat[0][0] = sRatio;
            _coordList[i].texCoordMat[1][1] = tRatio;
            _coordList[i].texCoordMat[2][2] = rRatio;
            break;
        }
    }
}

void TextureMgr::flipCoord( unsigned int id, bool flipU, bool flipW )
{
    for( int i = 0; i < ( int )_coordList.size(); i++ )
    {
        if( _coordList[i].id == id )
        {
            _coordList[i].flipU = flipU;
            _coordList[i].flipW = flipW;
            break;
        }
    }
}

void TextureMgr::transparentize( unsigned int id, float alpha )
{
    for( int i = 0; i < ( int )_coordList.size(); i++ )
    {
        if( _coordList[i].id == id )
        {
            _coordList[i].texAlpha = alpha;
            break;
        }
    }
}

void TextureMgr::bind()
{
    std::vector<int> samplerIds;
    std::vector<glm::vec2> scales, translates, flipFlags;
    std::vector<float> alphas;

    for( int i = 0; i < ( int )_coordList.size(); i++ )
    {
        // Activate Texture Unit.
        glActiveTexture( GL_TEXTURE0 + i );

        // Attach Texture to Unit.
        _coordList[i].texture->bind();

        // Apply Texture Transformation Matrix.
        glMatrixMode( GL_TEXTURE );
        glLoadIdentity();
        glMultMatrixf( &_coordList[i].texCoordMat[0][0] );

        samplerIds.push_back( i );
        scales.push_back( glm::vec2( _coordList[i].texCoordMat[0][0], _coordList[i].texCoordMat[1][1] ) );
        translates.push_back( glm::vec2( _coordList[i].texCoordMat[3][0], _coordList[i].texCoordMat[3][1] ) );
        flipFlags.push_back( _flipToVector( _coordList[i].flipU, _coordList[i].flipW ) );
        alphas.push_back( _coordList[i].texAlpha );
    }

    // Set Shader Uniforms.
    int numOfTextures = _coordList.size();
    if( numOfTextures != 0 )
    {
        // Push Uniform Variables to Shader.
        Shader * shader = NULL;

        if( !_lights )
        {
            switch( numOfTextures )
            {
            case 1:
                shader = GlobalShaderRepo()->getShader( SHADER_TEXTURE01_NO_LIGHTING );
                break;
            }

            if( shader )
            {
                // Set Texture Sampler Id.
                shader->setUniform1iv( "u_SamplerIds", samplerIds.size(), &samplerIds[0] );

                // Texture Scales.
                shader->setUniform2fv( "u_Scales", scales.size(), &scales[0][0] );

                // Texture Flip Coordinate Info.
                shader->setUniform2fv( "u_FlipFlags", flipFlags.size(), &flipFlags[0][0] );

                // Set Alpha value.
                shader->setUniform1fv( "u_TexAlphas", alphas.size(), &alphas[0] );

                shader->bind();
            }
        }
        else
        {
            switch( numOfTextures )
            {
            case 1:
                shader = GlobalShaderRepo()->getShader( SHADER_TEXTURE01 );
                break;

            case 2:
                switch( _textureMode )
                {
                case Common::VSP_TEXTURE_BLENDED:
                    shader = GlobalShaderRepo()->getShader( SHADER_MULTITEXTURE02_BLEND );
                    break;

                case Common::VSP_TEXTURE_LAYERED:
                    shader = GlobalShaderRepo()->getShader( SHADER_MULTITEXTURE02_LAYERED );
                    break;

                default:
                    break;
                }
                break;

            case 3:
                switch( _textureMode )
                {
                case Common::VSP_TEXTURE_BLENDED:
                    shader = GlobalShaderRepo()->getShader( SHADER_MULTITEXTURE03_BLEND );
                    break;

                case Common::VSP_TEXTURE_LAYERED:
                    shader = GlobalShaderRepo()->getShader( SHADER_MULTITEXTURE03_LAYERED );
                    break;

                default:
                    break;
                }
                break;

            case 4:
                switch( _textureMode )
                {
                case Common::VSP_TEXTURE_BLENDED:
                    shader = GlobalShaderRepo()->getShader( SHADER_MULTITEXTURE04_BLEND );
                    break;

                case Common::VSP_TEXTURE_LAYERED:
                    shader = GlobalShaderRepo()->getShader( SHADER_MULTITEXTURE04_LAYERED );
                    break;

                default:
                    break;
                }
                break;

            case 5:
                switch( _textureMode )
                {
                case Common::VSP_TEXTURE_BLENDED:
                    shader = GlobalShaderRepo()->getShader( SHADER_MULTITEXTURE05_BLEND );
                    break;

                case Common::VSP_TEXTURE_LAYERED:
                    shader = GlobalShaderRepo()->getShader( SHADER_MULTITEXTURE05_LAYERED );
                    break;

                default:
                    break;
                }
                break;

            case 6:
                switch( _textureMode )
                {
                case Common::VSP_TEXTURE_BLENDED:
                    shader = GlobalShaderRepo()->getShader( SHADER_MULTITEXTURE06_BLEND );
                    break;

                case Common::VSP_TEXTURE_LAYERED:
                    shader = GlobalShaderRepo()->getShader( SHADER_MULTITEXTURE06_LAYERED );
                    break;

                default:
                    break;
                }
                break;

            case 7:
                switch( _textureMode )
                {
                case Common::VSP_TEXTURE_BLENDED:
                    shader = GlobalShaderRepo()->getShader( SHADER_MULTITEXTURE07_BLEND );
                    break;

                case Common::VSP_TEXTURE_LAYERED:
                    shader = GlobalShaderRepo()->getShader( SHADER_MULTITEXTURE07_LAYERED );
                    break;

                default:
                    break;
                }
                break;

            case 8:
                switch( _textureMode )
                {
                case Common::VSP_TEXTURE_BLENDED:
                    shader = GlobalShaderRepo()->getShader( SHADER_MULTITEXTURE08_BLEND );
                    break;

                case Common::VSP_TEXTURE_LAYERED:
                    shader = GlobalShaderRepo()->getShader( SHADER_MULTITEXTURE08_LAYERED );
                    break;

                default:
                    break;
                }
                break;
            }

            if( shader )
            {
                // Enable / disable light source.
                std::vector<glm::vec4> lightSwitchList = _getLightSwitches();
                shader->setUniform4fv( "u_LightSwitches", lightSwitchList.size(), &lightSwitchList[0][0] );

                // Set Texture Sampler Id.
                shader->setUniform1iv( "u_SamplerIds", samplerIds.size(), &samplerIds[0] );

                // Texture Scales.
                shader->setUniform2fv( "u_Scales", scales.size(), &scales[0][0] );

                // Texture Flip Coordinate Info.
                shader->setUniform2fv( "u_FlipFlags", flipFlags.size(), &flipFlags[0][0] );

                // Set Alpha value.
                shader->setUniform1fv( "u_TexAlphas", alphas.size(), &alphas[0] );

                shader->bind();
            }
        }
    }
}

void TextureMgr::unbind()
{
    for( int i = 0; i < ( int )_coordList.size(); i++ )
    {
        glActiveTexture( GL_TEXTURE0 + i );

        _coordList[i].texture->unbind();

        glMatrixMode( GL_TEXTURE );
        glLoadIdentity();
    }

    int numOfTextures = _coordList.size();
    if( numOfTextures != 0 )
    {
        Shader::unbind();
    }
}

void TextureMgr::setLighting( Lighting * lighting )
{
    _lights = lighting;
}

void TextureMgr::setTextureMode( Common::VSPenum mode )
{
    _textureMode = mode;
}

float TextureMgr::getTranslateS( unsigned int id )
{
    for( int i = 0; i < ( int )_coordList.size(); i++ )
    {
        if( _coordList[i].id == id )
        {
            return _coordList[i].texCoordMat[3][0];
        }
    }
    // id does not match.
    assert( false );

    return 0.f;
}

float TextureMgr::getTranslateT( unsigned int id )
{
    for( int i = 0; i < ( int )_coordList.size(); i++ )
    {
        if( _coordList[i].id == id )
        {
            return _coordList[i].texCoordMat[3][1];
        }
    }
    // id does not match.
    assert( false );

    return 0.f;
}

float TextureMgr::getTranslateR( unsigned int id )
{
    for( int i = 0; i < ( int )_coordList.size(); i++ )
    {
        if( _coordList[i].id == id )
        {
            return _coordList[i].texCoordMat[3][2];
        }
    }
    // id does not match.
    assert( false );

    return 0.f;
}

float TextureMgr::getScaleS( unsigned int id )
{
    for( int i = 0; i < ( int )_coordList.size(); i++ )
    {
        if( _coordList[i].id == id )
        {
            return _coordList[i].texCoordMat[0][0];
        }
    }
    // id does not match.
    assert( false );

    return 1.f;
}

float TextureMgr::getScaleT( unsigned int id )
{
    for( int i = 0; i < ( int )_coordList.size(); i++ )
    {
        if( _coordList[i].id == id )
        {
            return _coordList[i].texCoordMat[1][1];
        }
    }
    // id does not match.
    assert( false );

    return 1.f;
}

float TextureMgr::getScaleR( unsigned int id )
{
    for( int i = 0; i < ( int )_coordList.size(); i++ )
    {
        if( _coordList[i].id == id )
        {
            return _coordList[i].texCoordMat[2][2];
        }
    }
    // id does not match.
    assert( false );

    return 1.f;
}

std::vector<unsigned int> TextureMgr::getIds()
{
    std::vector<unsigned int> ids( _coordList.size() );
    for( int i = 0; i < ( int )_coordList.size(); i++ )
    {
        ids[i] = _coordList[i].id;
    }
    return ids;
}

int TextureMgr::getMaxTextureSize()
{
    static GLint max = -1;
    if( max == -1 )
    {
        glGetIntegerv( GL_MAX_TEXTURE_SIZE, &max );
    }
    return max;
}

int TextureMgr::getMaxCombineTextures()
{
    static GLint max = -1;
    if( max == -1 )
    {
        glGetIntegerv( GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS, &max );
    }
    return max;
}

unsigned int TextureMgr::_generateId()
{
    unsigned int id;

    // If no recycled id, create a new one.
    if( _recycleBin.empty() )
    {
        id = _idTracker++;
    }
    else
    {
        // Set id to the last id in bin.
        id = _recycleBin[_recycleBin.size() - 1];
        // Pop the last id.
        _recycleBin.pop_back();
    }
    return id;
}

/*
* This is a helper function to convert bool bool to float float.
* true = 1.f, false = 0.f.
*/
glm::vec2 TextureMgr::_flipToVector( bool flipU, bool flipW )
{
    float uFlag, wFlag;
    uFlag = wFlag = 0.f;

    if( flipU )
    {
        uFlag = 1.f;
    }
    if( flipW )
    {
        wFlag = 1.f;
    }
    return glm::vec2( uFlag, wFlag );
}

std::vector<glm::vec4> TextureMgr::_getLightSwitches()
{
    std::vector<glm::vec4> lightColorList;

    if( !_lights )
    {
        for( int i = 0; i < 8; i++ )
        {
            lightColorList.emplace_back( glm::vec4( 0.f, 0.f, 0.f, 0.f ) );
        }
    }

    // Collecting light information.
    std::vector<bool> lightEnableList = _lights->getLightEnableStatus();
    for( int i = 0; i < ( int )lightEnableList.size(); i++ )
    {
        // If light on, change light switch color to white.  Else change light switch color to black.
        // This light switch color is multiplied with light color to enable / disable light source.
        if( lightEnableList[i] )
        {
            lightColorList.emplace_back( glm::vec4( 1.f, 1.f, 1.f, 1.f ) );
        }
        else
        {
            lightColorList.emplace_back( glm::vec4( 0.f, 0.f, 0.f, 0.f ) );
        }
    }
    assert( lightColorList.size() == 8 );

    return lightColorList;
}
}
