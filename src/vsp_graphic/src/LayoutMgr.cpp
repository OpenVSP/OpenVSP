#include "OpenGLHeaders.h"
#include "LayoutMgr.h"
#include "Viewport.h"
#include "Camera.h"
#include "Scene.h"
#include "Protractor.h"
#include "Ruler.h"
#include "Probe.h"

#include <assert.h>

#include "Routing.h"

namespace VSPGraphic
{
LayoutMgr::LayoutMgr( int row, int column )
{
    assert( row > 0 && column > 0 );

    _row = row;
    _column = column;
    int nview = _row * _column;

    _sWidth = _sHeight = 1;

    _startx = _starty = -1;

    std::vector < Common::VSPenum > vOrder;
    // Set View order for viewports.
    // Note:  Changes here should be matched in MainVSPScreen::ResetViews()
    switch ( nview )
    {
        case 1:
            vOrder.push_back( Common::VSP_CAM_TOP );
            break;
        case 2:
            vOrder.push_back( Common::VSP_CAM_FRONT );
            vOrder.push_back( Common::VSP_CAM_TOP );
            break;
        case 4:
        default:
            vOrder.push_back( Common::VSP_CAM_LEFT_ISO );
            vOrder.push_back( Common::VSP_CAM_TOP );
            vOrder.push_back( Common::VSP_CAM_FRONT );
            vOrder.push_back( Common::VSP_CAM_LEFT );
            break;
    }

    for( int i = 0; i < nview; i++ )
    {
        // Placeholder viewports.
        Viewport * viewport = new Viewport( 0, 0, 1, 1 );
        viewport->getCamera()->changeView( vOrder[i % ( int )vOrder.size()] );

        _viewportList.push_back( viewport );
    }
    _selected = -1;
}
LayoutMgr::~LayoutMgr()
{
    for( int i = 0; i < ( int )_viewportList.size(); i++ )
    {
        delete _viewportList[i];
    }
    _viewportList.clear();
}

void LayoutMgr::resize( int width, int height )
{
    assert( width > 0 && height > 0 );

    _sWidth = width;
    _sHeight = height;

    float ratioW = 1.0f / _column;
    float ratioH = 1.0f / _row;

    int x = 0;
    int y = ( int )( height - height * ratioH );

    for( int i = 0; i < _row; i++ )
    {
        for( int j = 0; j < _column; j++ )
        {
            _viewportList[i * _column + j]->resize( x, y, ( int )( width * ratioW ), ( int )( height * ratioH ) );
            x += ( int )( width * ratioW );
        }
        y = ( int )( y - height * ratioH );
        x = 0;
    }
}

void LayoutMgr::resizeScreenshot( int width, int height, float screenSizeDiffRatio )
{
    assert( width > 0 && height > 0 );

    _sWidth = width;
    _sHeight = height;

    float ratioW = 1.0f / _column;
    float ratioH = 1.0f / _row;

    int x = 0;
    int y = ( int )( height - height * ratioH );

    for( int i = 0; i < _row; i++ )
    {
        for( int j = 0; j < _column; j++ )
        {
            _viewportList[i * _column + j]->resizeViewport( x, y, ( int )( width * ratioW ), ( int )( height * ratioH ), screenSizeDiffRatio );
            x += ( int )( width * ratioW );
        }
        y = ( int )( y - height * ratioH );
        x = 0;
    }
}

void LayoutMgr::predraw( Scene * scene, int x, int y )
{
    // Preprocessing
    for( int i = 0; i < ( int )_viewportList.size(); i++ )
    {
        _viewportList[i]->bind();

        glClearColor( 0.0f, 0.0f, 0.0f, 0.0f );
        glClearDepth( 1.0 );

        glEnable( GL_DEPTH_TEST );
        glDepthFunc( GL_LEQUAL );

        glDepthMask( GL_TRUE );

        glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

        glDisable( GL_LIGHTING );

        glDisable( GL_BLEND );
        glDisable( GL_MULTISAMPLE );

        scene->predraw();

        _viewportList[i]->unbind();

        glDisable( GL_DEPTH_TEST );
        glDepthMask( GL_FALSE );
    }

    // Color Picking.
    scene->activatePicking( x, y );

    if ( _startx != -1  )
    {
        scene->preSelectBox( _startx, _starty, x, y );
    }
}

void LayoutMgr::draw( Scene * scene, int x, int y )
{
    for( int i = 0; i < ( int )_viewportList.size(); i++ )
    {
        // Set projection and modelview matrix.
        _viewportList[i]->bind();

        // Set mouse location for ruler.
        glm::vec3 mouseInWorld = _viewportList[i]->screenToWorld( glm::vec2( x, y ) );
        Protractor::updateMouseLocation( mouseInWorld );
        Ruler::updateMouseLocation( mouseInWorld );
        Probe::updateMouseLocation( mouseInWorld );
        Routing::updateMouseLocation( mouseInWorld );

        // Clear Color and Depth Buffer.
        glClearColor( 1.0f, 1.0f, 1.0f, 0.0f );

        glClearDepth( 1.0 );

        glDepthMask( GL_TRUE );
        glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

        // Set Viewport Properties.
        glDisable( GL_LIGHTING );

        glBlendFuncSeparate( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE_MINUS_DST_ALPHA, GL_ONE );
        glEnable( GL_BLEND );

        glAlphaFunc( GL_GREATER, 0 );
        glEnable( GL_ALPHA_TEST );

        glDepthFunc( GL_LEQUAL );
        glEnable( GL_DEPTH_TEST );

        glEnable( GL_MULTISAMPLE );

        glShadeModel( GL_SMOOTH );

        // Draw Background
        glDisable( GL_DEPTH_TEST );
        _viewportList[i]->drawBackground();

        glClear( GL_DEPTH_BUFFER_BIT );

        // Draw 2D Grid Overlay.
        _viewportList[i]->drawGridOverlay();
        glEnable( GL_DEPTH_TEST );

        // Set projection and modelview matrix.
        _viewportList[i]->bind();

        // Draw scene.
        scene->draw();

        // Draw arrows.
        _viewportList[i]->drawXYZArrows();

        if ( _startx != -1 )
        {
            _viewportList[i]->drawRectangle( _startx, _starty, x, y );
        }

        // Draw border.
        if( i == _selected )
        {
            _viewportList[i]->drawBorder( true );
        }
        else
        {
            _viewportList[i]->drawBorder( false );
        }

        _viewportList[i]->drawWatermark();

        glDisable( GL_BLEND );
        glDisable( GL_ALPHA_TEST );
        glDisable( GL_DEPTH_TEST );
        glDepthMask( GL_FALSE );

        _viewportList[i]->unbind();
    }
}

void LayoutMgr::selectViewport( int index )
{
    assert( index < ( int )_viewportList.size() );
    if( index >= ( int )_viewportList.size() )
    {
        return;
    }

    _selected = index;
}

int LayoutMgr::selectViewport( int x, int y )
{
    // Select viewport if push is within a viewport.
    for( int i = 0; i < ( int )_viewportList.size(); i++ )
    {
        int vx = _viewportList[i]->x();
        int vy = _viewportList[i]->y();
        int width = _viewportList[i]->width();
        int height = _viewportList[i]->height();

        if( x > vx && x < vx + width && y > vy && y < vy + height )
        {
            _selected = i;
            return _selected;
        }
    }
    _selected = -1;
    return _selected;
}

Viewport * LayoutMgr::getViewport()
{
    if( _selected < 0 )
    {
        return nullptr;
    }
    return _viewportList[_selected];
}

std::vector<Viewport *> LayoutMgr::getViewports()
{
    return _viewportList;
}
}
