#include "OpenGLHeaders.h"

#define GLM_FORCE_RADIANS
#include "glm/ext.hpp"

#include "Viewport.h"
#include "TextMgr.h"

#include "Camera.h"
#include "ArcballCam.h"

#include "Background.h"

#define BORDER_LINEWIDTH 1.0f
#define BORDER_OFFSET 0.009f
#define ARROW_LENGTH 0.15f
#define ARROW_TEXT_OFFSET 0.05f
#define ARROW_XPOS -(1.0f - (ARROW_LENGTH + ARROW_TEXT_OFFSET + /*text room*/0.05f))
#define ARROW_YPOS -(1.0f - (ARROW_LENGTH + ARROW_TEXT_OFFSET + /*text room*/0.05f))

namespace VSPGraphic
{
Viewport::Viewport( int x, int y, int width, int height )
{
    _x = x;
    _y = y;
    _vWidth = width;
    _vHeight = height;

    _camera = new ArcballCam();
    _camera->resize( _x, _y, _vWidth, _vHeight );

    _textMgr = new TextMgr();

    // can't initialize here because no context is created.
    _background = NULL;

    _showBorders = _showArrows = true;
    _showGrid = false;
}
Viewport::~Viewport()
{
    delete _camera;
    delete _textMgr;

    if( _background )
    {
        delete _background;
    }
}

void Viewport::resize( int x, int y, int width, int height )
{
    _x = x;
    _y = y;
    _vWidth = width;
    _vHeight = height;

    // Update Camera
    _camera->resize( _x, _y, _vWidth, _vHeight );
}

void Viewport::bind()
{
    glViewport( _x, _y, _vWidth, _vHeight );
    glScissor( _x, _y, _vWidth, _vHeight );
    glEnable( GL_SCISSOR_TEST );

    glPushMatrix();

    // Apply Projection.
    glMatrixMode( GL_PROJECTION );
    glLoadIdentity();
    glMultMatrixf( &_camera->getProjectionMatrix()[0][0] );

    // Apply Model View.
    glMatrixMode( GL_MODELVIEW );
    glLoadIdentity();
    glMultMatrixf( &_camera->getModelViewMatrix()[0][0] );
}

void Viewport::unbind()
{
    glPopMatrix();
    glDisable( GL_SCISSOR_TEST );
}

void Viewport::drawBorder( bool selected )
{
    if( !_showBorders )
    {
        return;
    }

    if( selected )
    {
        glColor3f( 1.0f, 0.0f, 0.0f );
    }
    else
    {
        glColor3f( 0.0f, 0.0f, 0.0f );
    }

    glMatrixMode( GL_PROJECTION );
    glPushMatrix();

    glLoadIdentity();

    glMatrixMode( GL_MODELVIEW );
    glLoadIdentity();

    // Keep Border Offset in proportion to the aspect ratio.
    float offsetH = BORDER_OFFSET * ( float )( _vHeight > _vWidth ? ( float )_vWidth / _vHeight : 1.0f );
    float offsetW = BORDER_OFFSET * ( float )( _vWidth > _vHeight ? ( float )_vHeight / _vWidth : 1.0f );

    glLineWidth( BORDER_LINEWIDTH );
    glBegin( GL_LINE_LOOP );
    glVertex2f( -1 + offsetW, 1 - offsetH );
    glVertex2f( 1 - offsetW, 1 - offsetH );
    glVertex2f( 1 - offsetW, -1 + offsetH );
    glVertex2f( -1 + offsetW, -1 + offsetH );
    glEnd();

    glPopMatrix();
    glMatrixMode( GL_MODELVIEW );
}

void Viewport::drawXYZArrows()
{
    if( !_showArrows )
    {
        return;
    }

    float aspectRatioH = _vHeight > _vWidth ? _vHeight / ( float )_vWidth : 1.0f;
    float aspectRatioW = _vWidth > _vHeight ? _vWidth / ( float )_vHeight : 1.0f;

    // Matrix for moving arrows to lower left corner of the screen.
    glm::mat4 arrowTMat = glm::translate( glm::vec3( ARROW_XPOS * aspectRatioW, ARROW_YPOS * aspectRatioH, 0.f ) );

    float borderH = 1.0f * aspectRatioH;
    float borderW = 1.0f * aspectRatioW;

    glm::mat4 projectionMatrix = glm::ortho( -borderW, borderW, -borderH, borderH, -2500.0f, 2500.0f );
    glm::mat4 modelviewMatrix = _camera->getModelViewMatrix();

    // Set Translation to 0. We only care about rotation.
    modelviewMatrix[3][0] = 0.0f;
    modelviewMatrix[3][1] = 0.0f;
    modelviewMatrix[3][2] = 0.0f;

    // Apply Projection and Modelview matrix.
    glPushMatrix();
    glMatrixMode( GL_PROJECTION );
    glLoadIdentity();
    glMultMatrixf( &projectionMatrix[0][0] );

    glMatrixMode( GL_MODELVIEW );
    glLoadIdentity();
    // Move arrows to position after rotation.
    glMultMatrixf( &arrowTMat[0][0] );
    glMultMatrixf( &modelviewMatrix[0][0] );

    // Draw Axis.
    glLineWidth( 1.5f );
    glClear( GL_DEPTH_BUFFER_BIT );

    glBegin( GL_LINES );
    glColor3ub( 255, 0, 0 );
    glVertex3d( 0.0, 0.0, 0.0 );
    glVertex3d( ARROW_LENGTH, 0.0, 0.0 );

    glColor3ub( 0, 255, 0 );
    glVertex3d( 0.0, 0.0, 0.0 );
    glVertex3d( 0.0, ARROW_LENGTH, 0.0 );

    glColor3ub( 0, 0, 255 );
    glVertex3d( 0.0, 0.0, 0.0 );
    glVertex3d( 0.0, 0.0, ARROW_LENGTH );
    glEnd();

    glEnable( GL_POINT_SMOOTH );
    glPointSize( 3.0 );
    glBegin( GL_POINTS );
    glColor3ub( 0, 0, 0 );
    glVertex3d( 0.0, 0.0, 0.0 );
    glEnd();

    glDisable( GL_POINT_SMOOTH );
    glDisable( GL_LINE_SMOOTH );
    glDepthMask( GL_FALSE );
    glDisable( GL_DEPTH_TEST );

    // Draw label.
    // Calculate translation for X, Y and Z.
    glm::mat4 xt = glm::translate( glm::vec3( modelviewMatrix * glm::vec4( ARROW_LENGTH + ARROW_TEXT_OFFSET, 0.f, 0.f, 1.f ) ) );
    glm::mat4 yt = glm::translate( glm::vec3( modelviewMatrix * glm::vec4( 0.f, ARROW_LENGTH + ARROW_TEXT_OFFSET, 0.f, 1.f ) ) );
    glm::mat4 zt = glm::translate( glm::vec3( modelviewMatrix * glm::vec4( 0.f, 0.f, ARROW_LENGTH + ARROW_TEXT_OFFSET, 1.f ) ) );

    glColor3ub( 0, 0, 0 );

    // Active Texture Image Unit 0.
    glActiveTexture( GL_TEXTURE0 );

    glLoadIdentity();
    glMultMatrixf( &arrowTMat[0][0] );

    // X
    glPushMatrix();
    glMultMatrixf( &xt[0][0] );
    _textMgr->drawText( _textMgr->loadFont(),
                        2.0f, "X", 0.f, 0.f, 0.f, 0.f );
    glPopMatrix();

    // Y
    glPushMatrix();
    glMultMatrixf( &yt[0][0] );
    _textMgr->drawText( _textMgr->loadFont(),
                        2.0f, "Y", 0.f, 0.f, 0.f, 0.f );
    glPopMatrix();

    // Z
    glPushMatrix();
    glMultMatrixf( &zt[0][0] );
    _textMgr->drawText( _textMgr->loadFont(),
                        2.0f, "Z", 0.f, 0.f, 0.f, 0.f );
    glPopMatrix();

    glPopMatrix();
}

void Viewport::drawGridOverlay()
{
    if( !_showGrid )
    {
        return;
    }

    // Apply Projection and Modelview matrix.
    glPushMatrix();

    glMatrixMode( GL_PROJECTION );
    glLoadIdentity();
    glMultMatrixf( &_camera->getProjectionMatrix()[0][0] );

    glMatrixMode( GL_MODELVIEW );
    glLoadIdentity();

    glColor4f( 0.8f, 0.8f, 0.8f, 1.0f );
    glLineWidth( 0.3f );

    glBegin( GL_LINES );
    for ( float i = -5.0f; i <= 5.0f; i += 0.5f )
    {
        glVertex2f( -5, i );
        glVertex2f( 5, i );
    }
    for ( float i = -5.0f; i <= 5.0f; i += 0.5f )
    {
        glVertex2f( i, -5 );
        glVertex2f( i, 5 );
    }
    glEnd();

    glColor3f( 0.1f, 0.1f, 0.1f );
    glBegin( GL_LINES );
    glVertex2f( -5, 0 );
    glVertex2f( 5, 0 );
    glVertex2f( 0, -5 );
    glVertex2f( 0, 5 );
    glEnd();

    glPopMatrix();
}

void Viewport::drawBackground()
{
    if( !_background )
    {
        _background = new Background();
    }

    glPushMatrix();

    glMatrixMode( GL_PROJECTION );
    glLoadIdentity();

    glMatrixMode( GL_MODELVIEW );
    glLoadIdentity();

    _background->setWidthHeight( _vWidth, _vHeight );
    _background->draw();

    glPopMatrix();
}

Camera* Viewport::getCamera()
{
    return _camera;
}

Background * Viewport::getBackground()
{
    if( !_background )
    {
        _background = new Background();
    }
    return _background;
}

void Viewport::showBorders( bool showFlag )
{
    _showBorders = showFlag;
}

void Viewport::showXYZArrows( bool showFlag )
{
    _showArrows = showFlag;
}

void Viewport::showGridOverlay( bool showFlag )
{
    _showGrid = showFlag;
}

glm::vec3 Viewport::screenToWorld(glm::vec2 screenCoord)
{
    return glm::unProject(glm::vec3(screenCoord, 0.5), 
        _camera->getModelViewMatrix(),
        _camera->getProjectionMatrix(),
        glm::vec4(_x, _y, _vWidth, _vHeight));
}

glm::vec2 Viewport::screenToNDC(glm::vec2 screenCoord)
{
    glm::vec2 NDC = screenCoord;

    NDC.x = NDC.x / (_x + _vWidth) * 2 - 1.0f;
    NDC.y = (NDC.y / (_y + _vHeight) * 2 - 1.0f) * -1;

    return NDC;
}

void Viewport::drawRectangle( int startx, int starty, int x, int y )
{

    glMatrixMode( GL_PROJECTION );
    glPushMatrix();

    glLoadIdentity();

    glMatrixMode( GL_MODELVIEW );
    glLoadIdentity();

    glColor4f( 1.0f, 0.0f, 0.0f, 1.0f );
    glLineWidth( 1.0f );

    float sstartx = 2.0f*(float)startx/(float)_vWidth - 1.0f;
    float sstarty = 2.0f*(float)starty/(float)_vHeight - 1.0f;
    float sx = 2.0f*(float)x/(float)_vWidth - 1.0f;
    float sy = 2.0f*(float)y/(float)_vHeight - 1.0f;

    glLineWidth( BORDER_LINEWIDTH );
    glBegin( GL_LINE_LOOP );
    glVertex2f( sstartx , sstarty );
    glVertex2f( sx , sstarty );
    glVertex2f( sx , sy );
    glVertex2f( sstartx , sy );
    glEnd();

    glPopMatrix();
    glMatrixMode( GL_MODELVIEW );

}
}
