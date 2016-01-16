#include <assert.h>

#include "OpenGLHeaders.h"

#include "Lighting.h"

namespace VSPGraphic
{
LightSource::LightSource( unsigned int id )
{
    _id = id;
    _enabled = false;

    _pos[0] = 0;
    _pos[1] = 0;
    _pos[2] = 1;
    _pos[3] = 0;
    _amb[0] = 0;
    _amb[1] = 0;
    _amb[2] = 0;
    _amb[3] = 1;

    if( _id == GL_LIGHT0 )
    {
        _diff[0] = _diff[1] = _diff[2] = _diff[3] = 1;
        _spec[0] = _spec[1] = _spec[2] = _spec[3] = 1;
    }
    else
    {
        _diff[0] = _diff[1] = _diff[2] = 0;
        _diff[3] = 1;
        _spec[0] = _spec[1] = _spec[2] = 0;
        _spec[3] = 1;
    }
}
LightSource::~LightSource()
{
}

void LightSource::update()
{
    glLightfv( ( GLenum )_id, GL_AMBIENT, _amb );
    glLightfv( ( GLenum )_id, GL_DIFFUSE, _diff );
    glLightfv( ( GLenum )_id, GL_SPECULAR, _spec );
    glLightfv( ( GLenum )_id, GL_POSITION, _pos );

    if( _enabled )
    {
        glEnable( ( GLenum )_id );
    }
    else
    {
        glDisable( ( GLenum )_id );
    }
}

void LightSource::enable()
{
    if( !_enabled )
    {
        _enabled = true;
    }
}

void LightSource::disable()
{
    if( _enabled )
    {
        _enabled = false;
    }
}

bool LightSource::isEnabled()
{
    return _enabled;
}

void LightSource::position( float * posArray )
{
    for( int i = 0; i < 4; i++ )
    {
        _pos[i] = posArray[i];
    }
}

void LightSource::position( float x, float y, float z, float w )
{
    GLfloat pos[] = {x, y, z, w};
    for( int i = 0; i < 4; i++ )
    {
        _pos[i] = pos[i];
    }
}

void LightSource::ambient( float * ambArray )
{
    for( int i = 0; i < 4; i++ )
    {
        _amb[i] = ambArray[i];
    }
}

void LightSource::ambient( float r, float g, float b, float a )
{
    GLfloat amb[] = {r, g, b, a};
    for( int i = 0; i < 4; i++ )
    {
        _amb[i] = amb[i];
    }
}

void LightSource::diffuse( float * diffArray )
{
    for( int i = 0; i < 4; i++ )
    {
        _diff[i] = diffArray[i];
    }
}

void LightSource::diffuse( float r, float g, float b, float a )
{
    GLfloat diff[] = {r, g, b, a};
    for( int i = 0; i < 4; i++ )
    {
        _diff[i] = diff[i];
    }
}

void LightSource::specular( float * specArray )
{
    for( int i = 0; i < 4; i++ )
    {
        _spec[i] = specArray[i];
    }
}

void LightSource::specular( float r, float g, float b, float a )
{
    GLfloat spec[] = {r, g, b, a};
    for( int i = 0; i < 4; i++ )
    {
        _spec[i] = spec[i];
    }
}

Lighting::Lighting()
{
    light0 = new LightSource( GL_LIGHT0 );
    light1 = new LightSource( GL_LIGHT1 );
    light2 = new LightSource( GL_LIGHT2 );
    light3 = new LightSource( GL_LIGHT3 );
    light4 = new LightSource( GL_LIGHT4 );
    light5 = new LightSource( GL_LIGHT5 );
    light6 = new LightSource( GL_LIGHT6 );
    light7 = new LightSource( GL_LIGHT7 );

    _lightSources.push_back( light0 );
    _lightSources.push_back( light1 );
    _lightSources.push_back( light2 );
    _lightSources.push_back( light3 );
    _lightSources.push_back( light4 );
    _lightSources.push_back( light5 );
    _lightSources.push_back( light6 );
    _lightSources.push_back( light7 );

    _enabled = false;
    _isChanged = false;
}
Lighting::~Lighting()
{
    for( int i = 0; i < ( int )_lightSources.size(); i++ )
    {
        delete _lightSources[i];
    }
    _lightSources.clear();
}

void Lighting::update()
{
    for( int i = 0; i < ( int )_lightSources.size(); i++ )
    {
        _lightSources[i]->update();
    }
    _isChanged = false;
}

std::vector<bool> Lighting::getLightEnableStatus()
{
    std::vector<bool> statusList;

    for( int i = 0; i < ( int )_lightSources.size(); i++ )
    {
        statusList.push_back( _lightSources[i]->isEnabled() );
    }
    return statusList;
}

LightSource * Lighting::getLightSource( unsigned int index )
{
    if( index >= _lightSources.size() )
    {
        return NULL;
    }

    return _lightSources[index];
}
}
