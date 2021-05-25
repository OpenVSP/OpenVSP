#include <assert.h>

#include "OpenGLHeaders.h"

#include "Background.h"

#include "Texture2D.h"
#include "TextureMgr.h"
#include "VertexBuffer.h"

#define TOPL	 0
#define TOPR	 1
#define BOTR	 2
#define BOTL	 3

namespace VSPGraphic
{
Background::Background() : Renderable()
{
    _coord.resize( 4 );
    _coord[TOPL] = glm::vec3( -1, 1, 0 );
    _coord[TOPR] = glm::vec3( 1, 1, 0 );
    _coord[BOTR] = glm::vec3( 1, -1, 0 );
    _coord[BOTL] = glm::vec3( -1, -1, 0 );

    _texCoord.resize( 4 );
    _texCoord[TOPL] = glm::vec2( 0, 1 );
    _texCoord[TOPR] = glm::vec2( 1, 1 );
    _texCoord[BOTR] = glm::vec2( 1, 0 );
    _texCoord[BOTL] = glm::vec2( 0, 0 );

    _aspectRatioW = 1;
    _aspectRatioH = 1;

    _scaleX = 1;
    _scaleY = 1;

    _offsetX = 0;
    _offsetY = 0;

    _vWidth = -1;
    _vHeight = -1;

    _textureMgr = new TextureMgr();
    _texId = 0xFFFFFFFF;

    _2DTextureID = 0;

    _keepAR = true;
    _hasChanged = true;

    _flipX = false;

    _mode = Common::VSP_BACKGROUND_COLOR;

    setMeshColor( 0.95f, 0.95f, 0.95f, 1.0f );
    setPrimType( Common::VSP_QUADS );
}
Background::~Background()
{
    delete _textureMgr;
}

void Background::preserveAR( bool keepAR )
{
    _keepAR = keepAR;

    if( _keepAR )
    {
        _aspectRatioW = _scaleX / _scaleY;
        _aspectRatioH = _scaleY / _scaleX;
    }
    else
    {
        _aspectRatioW = 1;
        _aspectRatioH = 1;
    }
}

void Background::scaleW( float scale )
{
    if( _keepAR )
    {
        _scaleY = ( scale - _scaleX ) * _aspectRatioH + _scaleY;
    }
    _scaleX = scale;

    _hasChanged = true;
}

void Background::scaleH( float scale )
{
    if( _keepAR )
    {
        _scaleX = ( scale - _scaleY ) * _aspectRatioW + _scaleX;
    }
    _scaleY = scale;

    _hasChanged = true;
}

void Background::offsetX( float offset )
{
    _offsetX = offset;
    _hasChanged = true;
}

void Background::offsetY( float offset )
{
    _offsetY = offset;
    _hasChanged = true;
}

void Background::flipX( bool val )
{
    _flipX = val;
    _hasChanged = true;
}

void Background::attachImage( Texture2D * texture )
{
    removeImage();

    _texId = _textureMgr->add( texture );
    _2DTextureID = texture->getTextureID();

    _imWidth = texture->getImWidth();
    _imHeight = texture->getImHeight();
}

void Background::removeImage()
{
    if( _texId != 0xFFFFFFFF )
    {
        _textureMgr->remove( _texId );
        _texId = 0xFFFFFFFF;
    }
}

void Background::setRed( float red )
{
    setMeshColor( red, _getMeshColor().green, _getMeshColor().blue, _getMeshColor().alpha );
}

void Background::setGreen( float green )
{
    setMeshColor( _getMeshColor().red, green, _getMeshColor().blue, _getMeshColor().alpha );
}

void Background::setBlue( float blue )
{
    setMeshColor( _getMeshColor().red, _getMeshColor().green, blue, _getMeshColor().alpha );
}

void Background::setAlpha( float alpha )
{
    setMeshColor( _getMeshColor().red, _getMeshColor().green, _getMeshColor().blue, alpha );
}

void Background::setBackgroundMode( Common::VSPenum mode )
{
    _mode = mode;
    _hasChanged = true;
}

void Background::reset()
{
    _scaleX = 1;
    _scaleY = 1;

    _offsetX = 0;
    _offsetY = 0;

    removeImage();
    setMeshColor( 242.0/255.0, 242.0/255.0, 242/255.0, 1.0f );

    _hasChanged = true;
}

Common::VSPenum Background::getBackgroundMode()
{
    return _mode;
}

bool Background::getARFlag()
{
    return _keepAR;
}

float Background::getRed()
{
    return _getMeshColor().red;
}

float Background::getGreen()
{
    return _getMeshColor().green;
}

float Background::getBlue()
{
    return _getMeshColor().blue;
}

float Background::getScaleW()
{
    return _scaleX;
}

float Background::getScaleH()
{
    return _scaleY;
}

float Background::getOffsetX()
{
    return _offsetX;
}

float Background::getOffsetY()
{
    return _offsetY;
}

unsigned int Background::getTextureID()
{
    return _2DTextureID;
}

void Background::setWidthHeight( int w, int h)
{
    if ( w == _vWidth && h == _vHeight )
    {
        return;
    }

    _vWidth = w;
    _vHeight = h;
    _hasChanged = true;
}

void Background::_predraw()
{
}

void Background::_draw()
{
    _build();

    if( _texId != 0xFFFFFFFF )
    {
        _textureMgr->bind();
    }

    if( _mode == Common::VSP_BACKGROUND_COLOR )
    {
        glColor4f( _getMeshColor().red, _getMeshColor().green, _getMeshColor().blue, _getMeshColor().alpha );
    }
    else
    {
        glColor4f( 0.95f, 0.95f, 0.95f, 1.0f );
    }

    glPolygonMode( GL_FRONT_AND_BACK, GL_FILL );
    _vBuffer->draw( GL_QUADS );

    if( _texId != 0xFFFFFFFF )
    {
        _textureMgr->unbind();
    }
}

void Background::_build()
{
    assert( _coord.size() == _texCoord.size() );

    if( _hasChanged )
    {
        float sw = 1.0;
        float sh = 1.0;

        if( _mode == Common::VSP_BACKGROUND_IMAGE )
        {
            float vAR = (float) _vHeight / (float) _vWidth;
            float iAR =  (float) _imHeight / (float) _imWidth;

            if ( vAR > iAR ) // Match image width
            {
                sh = iAR / vAR;
            }
            else   // Match image height
            {
                sw = vAR / iAR;
            }
        }

        _background.clear();
        for( int i = 0; i < (int)_coord.size(); i++ )
        {
            if( _mode == Common::VSP_BACKGROUND_IMAGE )
            {
                _background.push_back( ( _flipX ? -1 : 1 ) * _coord[i].x * _scaleX * sw + _offsetX );
                _background.push_back( _coord[i].y * _scaleY * sh + _offsetY );
                _background.push_back( _coord[i].z );
            }
            else
            {
                _background.push_back( _coord[i].x );
                _background.push_back( _coord[i].y );
                _background.push_back( _coord[i].z );
            }
            _background.push_back( 0 );
            _background.push_back( 0 );
            _background.push_back( 0 );
            _background.push_back( _texCoord[i].x );
            _background.push_back( _texCoord[i].y );
        }
        emptyVBuffer();
        appendVBuffer( _background.data(), _background.size() * sizeof(float) );

        _hasChanged = false;
    }
}
}
