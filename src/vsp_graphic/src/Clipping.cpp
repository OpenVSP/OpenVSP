#include <assert.h>

#include "OpenGLHeaders.h"

#include "Clipping.h"

namespace VSPGraphic
{


ClipPlane::ClipPlane()
{
    _enabled = false;
    _eqn[0] = _eqn[1] = _eqn[2] = _eqn[3] = 0.0;

    seticlip(0);
}

ClipPlane::~ClipPlane()
{

}

void ClipPlane::enable()
{
    if( !_enabled )
    {
        _enabled = true;
    }
}

void ClipPlane::disable()
{
    if( _enabled )
    {
        _enabled = false;
    }
}

bool ClipPlane::isEnabled()
{
    return _enabled;
}

void ClipPlane::setEqn( double e[4] )
{
    for( int i = 0; i < 4; i++ )
    {
        _eqn[i] = e[i];
    }
}

void ClipPlane::predraw()
{
    if( _enabled )
    {
        glClipPlane ( _iclip, _eqn );
        glEnable( _iclip );
    }
}

void ClipPlane::postdraw()
{
    if( _enabled )
    {
        glDisable( _iclip );
    }
}

void ClipPlane::seticlip( int indx )
{
    _iclip = GL_CLIP_PLANE0 + indx;
}

Clipping::Clipping()
{
	_cplanes.resize( 6 );

	for( int i = 0; i < (int)_cplanes.size(); i++ )
    {
        _cplanes[i].seticlip( i );
    }
}

Clipping::~Clipping()
{
}

void Clipping::predraw()
{
    for( int i = 0; i < (int)_cplanes.size(); i++ )
    {
        _cplanes[i].predraw();
    }
}

void Clipping::postdraw()
{
    for( int i = 0; i < (int)_cplanes.size(); i++ )
    {
        _cplanes[i].postdraw();
    }
}

ClipPlane * Clipping::getPlane( int i )
{
    return &_cplanes[i];
}


}
