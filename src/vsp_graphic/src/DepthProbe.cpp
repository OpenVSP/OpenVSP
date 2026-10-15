#include "OpenGLHeaders.h"
#include "DepthProbe.h"
#include "Scene.h"
#include "Viewport.h"
#include "Camera.h"
#include "SceneObject.h"

#include "glm/ext.hpp"

#include <vector>

namespace VSPGraphic
{
DepthProbe::DepthProbe()
{
    _fbo = 0;
    _color = 0;
    _depth = 0;
    _size = 0;
}

DepthProbe::~DepthProbe()
{
    // The GL context may be gone; the buffers go with it.
}

bool DepthProbe::_setup( int size )
{
    if ( _fbo != 0 && _size == size )
    {
        return true;
    }

    if ( !glewIsSupported( "GL_ARB_framebuffer_object" ) )
    {
        return false;
    }

    if ( _fbo == 0 )
    {
        glGenFramebuffers( 1, &_fbo );
        glGenRenderbuffers( 1, &_color );
        glGenRenderbuffers( 1, &_depth );
    }

    // The color buffer holds object IDs.
    glBindRenderbuffer( GL_RENDERBUFFER, _color );
    glRenderbufferStorage( GL_RENDERBUFFER, GL_RGBA8, size, size );
    glBindRenderbuffer( GL_RENDERBUFFER, _depth );
    glRenderbufferStorage( GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, size, size );
    glBindRenderbuffer( GL_RENDERBUFFER, 0 );

    glBindFramebuffer( GL_FRAMEBUFFER, _fbo );
    glFramebufferRenderbuffer( GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, _color );
    glFramebufferRenderbuffer( GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, _depth );
    glDrawBuffer( GL_COLOR_ATTACHMENT0 );
    glReadBuffer( GL_COLOR_ATTACHMENT0 );
    bool ok = ( glCheckFramebufferStatus( GL_FRAMEBUFFER ) == GL_FRAMEBUFFER_COMPLETE );

    // Try again next time if it failed.
    _size = 0;
    if ( ok )
    {
        _size = size;
    }
    return ok;
}

bool DepthProbe::probe( Scene * scene, Viewport * vp, int x, int y, int radius, glm::vec3 & hit,
                        unsigned int * objectId, const std::set < unsigned int > * only )
{
    int n = 2 * radius + 1;

    GLint oldFbo = 0;
    glGetIntegerv( GL_FRAMEBUFFER_BINDING, &oldFbo );

    bool ready = _setup( n );
    glBindFramebuffer( GL_FRAMEBUFFER, oldFbo );
    if ( !ready )
    {
        return false;
    }

    Camera * cam = vp->getCamera();
    glm::mat4 mv = cam->getModelViewMatrix();
    glm::mat4 proj = cam->getProjectionMatrix();
    glm::vec4 rect( vp->x(), vp->y(), vp->width(), vp->height() );

    // Map the window pixels [x - radius, x + radius + 1) onto the whole buffer.
    float ax = rect[2] / n;
    float ay = rect[3] / n;
    float bx = ( rect[2] + 2.0f * ( rect[0] - x + radius ) ) / n - 1.0f;
    float by = ( rect[3] + 2.0f * ( rect[1] - y + radius ) ) / n - 1.0f;
    glm::mat4 pick = glm::translate( glm::mat4( 1.0f ), glm::vec3( bx, by, 0.0f ) ) *
                     glm::scale( glm::mat4( 1.0f ), glm::vec3( ax, ay, 1.0f ) );
    glm::mat4 pickProj = pick * proj;

    glPushAttrib( GL_ALL_ATTRIB_BITS );

    glBindFramebuffer( GL_FRAMEBUFFER, _fbo );
    glViewport( 0, 0, n, n );
    glDisable( GL_SCISSOR_TEST );

    glMatrixMode( GL_PROJECTION );
    glPushMatrix();
    glLoadMatrixf( &pickProj[0][0] );
    glMatrixMode( GL_MODELVIEW );
    glPushMatrix();
    glLoadMatrixf( &mv[0][0] );

    glDisable( GL_LIGHTING );
    glDisable( GL_BLEND );
    glDisable( GL_ALPHA_TEST );
    glDisable( GL_CULL_FACE );
    glDisable( GL_POLYGON_OFFSET_FILL );
    glDisable( GL_TEXTURE_2D );
    glDisable( GL_DITHER );
    glUseProgram( 0 );
    if ( objectId )
    {
        glColorMask( GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE );
    }
    else
    {
        glColorMask( GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE );
    }
    glEnable( GL_DEPTH_TEST );
    glDepthFunc( GL_LESS );
    glDepthMask( GL_TRUE );
    glClearDepth( 1.0 );
    glClearColor( 0.0f, 0.0f, 0.0f, 0.0f );
    glClear( GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT );

    std::vector < SceneObject * > objects;
    if ( objectId )
    {
        scene->drawDepth( &objects, only );
    }
    else
    {
        scene->drawDepth( nullptr, only );
    }

    std::vector < float > depth( n * n, 1.0f );
    std::vector < unsigned char > color( 4 * n * n, 0 );
    glPixelStorei( GL_PACK_ALIGNMENT, 1 );
    glReadPixels( 0, 0, n, n, GL_DEPTH_COMPONENT, GL_FLOAT, &depth[0] );
    if ( objectId )
    {
        glReadPixels( 0, 0, n, n, GL_RGBA, GL_UNSIGNED_BYTE, &color[0] );
    }

    glMatrixMode( GL_PROJECTION );
    glPopMatrix();
    glMatrixMode( GL_MODELVIEW );
    glPopMatrix();

    glBindFramebuffer( GL_FRAMEBUFFER, oldFbo );
    glPopAttrib();

    // The surface at the pixel itself; failing that, the closest one in the neighbourhood,
    // nearest the eye among equals.
    int best = -1;
    int bestDist = 0;
    for ( int i = 0; i < n * n; i++ )
    {
        if ( depth[i] >= 1.0f )
        {
            continue;
        }
        int dx = ( i % n ) - radius;
        int dy = ( i / n ) - radius;
        int dist = dx * dx + dy * dy;
        if ( best < 0 || dist < bestDist || ( dist == bestDist && depth[i] < depth[best] ) )
        {
            best = i;
            bestDist = dist;
        }
    }
    if ( best < 0 )
    {
        return false;
    }

    if ( objectId )
    {
        // Scene::drawDepth colors object i with i + 1 in 24 bits.
        unsigned int index = color[ 4 * best ] | ( color[ 4 * best + 1 ] << 8 ) | ( color[ 4 * best + 2 ] << 16 );
        *objectId = 0xFFFFFFFF;
        if ( index > 0 && index <= objects.size() )
        {
            *objectId = objects[ index - 1 ]->getID();
        }
    }

    float px = x - radius + ( best % n ) + 0.5f;
    float py = y - radius + ( best / n ) + 0.5f;
    hit = glm::unProject( glm::vec3( px, py, depth[best] ), mv, proj, rect );
    return true;
}
}
