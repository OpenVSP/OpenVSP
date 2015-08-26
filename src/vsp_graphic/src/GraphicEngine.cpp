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
    std::vector<unsigned char> data(w * h * 3, 0);

    _display->draw( _scene, 0xFFFFFFFF, 0xFFFFFFFF );

    glReadBuffer( GL_BACK );

    glPixelStorei( GL_UNPACK_ALIGNMENT, 1 );
    glPixelStorei( GL_PACK_ALIGNMENT, 1 );
    glReadPixels( 0, 0, w, h, GL_RGB, GL_UNSIGNED_BYTE, &data[0] );

    // Flip data top to bottom.
    std::vector<unsigned char> flipdat(w * h * 3, 0);
    int scanLen = 3 * w;
    for ( int i = 0 ; i < h; i++ )
    {
        unsigned char* srcLine = &data[ i * scanLen ];
        unsigned char* dstLine = &flipdat[ (h - i - 1) * scanLen ];
        memcpy(  dstLine, srcLine, scanLen );
    }

    writeJPEG( fileName.c_str(), w, h, &flipdat[0] );

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
