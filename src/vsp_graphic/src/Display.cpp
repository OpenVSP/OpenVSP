#include <assert.h>

#include "OpenGLHeaders.h"

#include "Display.h"
#include "LayoutMgr.h"

#include "Viewport.h"

#include "Camera.h"
#include "CameraMgr.h"

#include "Scene.h"

namespace VSPGraphic
{
Display::Display()
{
    _layoutList.push_back( new LayoutMgr( 1, 1 ) );
    _currLayout = _layoutList[0];
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
        selectedVP->getCameraMgr()->getCamera()->changeView( type );
    }
}

Viewport * Display::getViewport()
{
    return _currLayout->getViewport();
}

Camera * Display::getCamera()
{
    CameraMgr * cameraMgr = getCameraMgr();

    if( cameraMgr )
    {
        return cameraMgr->getCamera();
    }
    return NULL;
}

CameraMgr * Display::getCameraMgr()
{
    // Get current selected viewport, ptr is NULL if
    // no viewport is selected.
    Viewport * selectedVP = _currLayout->getViewport();

    if( selectedVP )
    {
        return selectedVP->getCameraMgr();
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

void Display::draw( Scene * scene )
{
    _currLayout->draw( scene );
}

void Display::postdraw( Scene * scene )
{
    _currLayout->postdraw( scene );
}

void Display::resetView()
{
    Camera * camera = getCamera();
    if( camera )
    {
        camera->resetView();
    }
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

void Display::pan( int px, int py, int cx, int cy )
{
    Camera * camera = getCamera();
    if( camera )
    {
        camera->pan( px, py, cx, cy );
    }
}

void Display::pan( float x, float y )
{
    Camera * camera = getCamera();
    if( camera )
    {
        camera->pan( x, y );
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

void Display::zoom( float zoomvalue )
{
    Camera * camera = getCamera();
    if( camera )
    {
        camera->zoom( zoomvalue );
    }
}

void Display::selectViewport( int x, int y )
{
    _currLayout->selectViewport( x, y );
}
}