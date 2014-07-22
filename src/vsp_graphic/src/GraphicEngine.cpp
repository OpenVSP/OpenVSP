#include "GraphicEngine.h"

#include "Scene.h"
#include "Display.h"

#include "OpenGLHeaders.h"
#include "loadjpg.h"
#include "LayoutMgr.h"
#include <string.h>

#include <assert.h>

namespace VSPGraphic
{
GraphicEngine::GraphicEngine()
{
    _scene = new Scene();
    _display = new Display();
}
GraphicEngine::~GraphicEngine()
{
    delete _scene;
    delete _display;
}

void GraphicEngine::draw()
{
    _display->draw( _scene, 0xFFFFFFFF, 0xFFFFFFFF );
}

void GraphicEngine::draw( int mouseX, int mouseY )
{
    _display->predraw( _scene, mouseX, mouseY );
    _display->draw( _scene, mouseX, mouseY );
}

void GraphicEngine::dumpScreenJPEG( std::string fileName )
{
    int w = _display->getLayoutMgr()->screenWidth();
    int h = _display->getLayoutMgr()->screenHeight();

    // width * height * RGB
    unsigned char * data = new unsigned char[( w + 1 ) * ( h + 1 ) * 3];

    _display->draw( _scene, 0xFFFFFFFF, 0xFFFFFFFF );

    glReadBuffer( GL_BACK );
    glReadPixels( 0, 0, w, h, GL_RGB, GL_UNSIGNED_BYTE, data );

    // Flip data top to bottom.
    unsigned char * flipdat = new unsigned char[( w + 1 ) * ( h + 1 ) * 3];
    int scanLen = 3 * ( w + 1 );
    for ( int i = 0 ; i < h + 1 ; i++ )
    {
        unsigned char* srcLine = &data[ i * scanLen ];
        unsigned char* dstLine = &flipdat[ (h - i) * scanLen ];
        memcpy(  dstLine, srcLine, scanLen );
    }
    delete data;

    writeJPEG( fileName.c_str(), w + 1, h + 1, flipdat );

    delete flipdat;
}

void GraphicEngine::initGlew()
{
    GLenum error = glewInit();
    if( error != GLEW_OK )
    {
        // Failed to initialize glew.
        assert( false );
    }
}

Scene * GraphicEngine::getScene()
{
    return _scene;
}

Display * GraphicEngine::getDisplay()
{
    return _display;
}
}
