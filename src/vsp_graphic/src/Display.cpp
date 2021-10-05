#include "OpenGLHeaders.h"
#include "Display.h"
#include "LayoutMgr.h"
#include "Viewport.h"
#include "Camera.h"
#include "Scene.h"

#include <assert.h>

namespace VSPGraphic
{
static float _screenSizeDiffRatio;

Display::Display()
{
    _layoutList.push_back( new LayoutMgr( 1, 1 ) );
    _currLayout = _layoutList[0];

    _screenSizeDiffRatio = 1.0;
}
Display::~Display()
{
    for( int i = 0; i < ( int )_layoutList.size(); i++ )
    {
        delete _layoutList[i];
    }
    _layoutList.clear();
}

void Display::setDisplayLayout( int row, int column )
{
    for( int i = 0; i < ( int )_layoutList.size(); i++ )
    {
        if( _layoutList[i]->row() == row && _layoutList[i]->column() == column )
        {
            // If layout exist, set as current layout and exit.
            _currLayout = _layoutList[i];
            return;
        }
    }
    _layoutList.push_back( new LayoutMgr( row, column ) );

    // Set to last element in the list.
    _currLayout = _layoutList[_layoutList.size() - 1];
}

void Display::resize( int width, int height )
{
    assert( width > 0 && height > 0 );

    for( int i = 0; i < ( int )_layoutList.size(); i++ )
    {
        _layoutList[i]->resize( width, height );
    }
}

void Display::setDefaultScreenSize( int width, int height )
{
    _origWidth = width;
    _origHeight = height;
}

void Display::resizeScreenshot( int width, int height )
{
    assert( width > 0 && height > 0 );

    _screenSizeDiffRatio = ((float) width ) / ((float) _origWidth );

    for( int i = 0; i < ( int )_layoutList.size(); i++ )
    {
        _layoutList[i]->resizeScreenshot( width, height, _screenSizeDiffRatio );
    }
}

void Display::selectViewport( int index )
{
    _currLayout->selectViewport( index );
}

void Display::changeView( Common::VSPenum type )
{
    // Get current selected viewport, ptr is NULL if
    // no viewport is selected.
    Viewport * selectedVP = _currLayout->getViewport();
    if( selectedVP )
    {
        selectedVP->getCamera()->changeView( type );
    }
}

Viewport * Display::getViewport()
{
    return _currLayout->getViewport();
}

Camera * Display::getCamera()
{
    // Get current selected viewport, ptr is NULL if
    // no viewport is selected.
    Viewport * selectedVP = _currLayout->getViewport();

    if( selectedVP )
    {
        return selectedVP->getCamera();
    }

    return NULL;
}

LayoutMgr * Display::getLayoutMgr()
{
    return _currLayout;
}

void Display::predraw( Scene * scene, int x, int y )
{
    _currLayout->predraw( scene, x, y );
}

void Display::draw( Scene * scene, int x, int y )
{
    _currLayout->draw( scene, x, y );
}

void Display::center()
{
    Camera * camera = getCamera();
    if( camera )
    {
        camera->center();
    }
}

void Display::save( int index )
{
    Camera * camera = getCamera();
    if( camera )
    {
        camera->save( index );
    }
}

void Display::load( int index )
{
    Camera * camera = getCamera();
    if( camera )
    {
        camera->load( index );
    }
}

void Display::rotate( int px, int py, int cx, int cy )
{
    Camera * camera = getCamera();
    if( camera )
    {
        camera->rotate( px, py, cx, cy );
    }
}

void Display::rotateSphere( float angleX, float angleY, float angleZ )
{
    Camera * camera = getCamera();
    if( camera )
    {
        camera->rotateSphere( angleX, angleY, angleZ );
    }
}

glm::vec3 Display::getRotationEulerAngles()
{
    Camera * camera = getCamera();
    if( camera )
    {
        return camera->getRotationEulerAngles();
    }
    return glm::vec3();
}

void Display::pan( int px, int py, int cx, int cy )
{
    Camera * camera = getCamera();
    if( camera )
    {
        camera->pan( px, py, cx, cy );
    }
}

glm::vec2 Display::getPanValues()
{
    Camera * camera = getCamera();
    if( camera )
    {
        return camera->getPanValues();
    }
    return glm::vec2();
}

void Display::relativePan( float x, float y )
{
    Camera * camera = getCamera();
    if ( camera )
    {
        camera->relativePan( x, y );
    }
}

void Display::zoom( int px, int py, int cx, int cy )
{
    Camera * camera = getCamera();
    if( camera )
    {
        camera->zoom( px, py, cx, cy );
    }
}

void Display::zoom( int dx, int dy )
{
    Camera * camera = getCamera();
    if( camera )
    {
        camera->zoom( dx, dy );
    }
}

float Display::getRelativeZoomValue()
{
    Camera * camera = getCamera();
    if( camera )
    {
        return camera->getRelativeZoomValue();
    }
    return 0.0;
}


void Display::relativeZoom( float zoomvalue )
{
    Camera * camera = getCamera();
    if( camera )
    {
        camera->relativeZoom( zoomvalue );
    }
}

void Display::setCOR( float x, float y, float z )
{
    Camera * camera = getCamera();
    if( camera )
    {
        camera->setCOR( x, y, z );
    }
}

glm::vec3 Display::getCOR()
{
    Camera * camera = getCamera();
    if( camera )
    {
        return camera->getCOR();
    }
    return glm::vec3();
}

void Display::selectViewport( int x, int y )
{
    _currLayout->selectViewport( x, y );
}


float Display::getScreenSizeDiffRatio()
{
    return _screenSizeDiffRatio;
}
}
