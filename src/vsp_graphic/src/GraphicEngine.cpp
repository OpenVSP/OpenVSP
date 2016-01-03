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

void GraphicEngine::dumpScreenJPEG( std::string fileName, int width, int height )
{
    GLuint color;
    GLuint depth;
    GLuint fbo;
    int oldWidth = _display->getLayoutMgr()->screenWidth();
    int oldHeight = _display->getLayoutMgr()->screenHeight();

    _display->resizeScreenshot( width, height );

    glGenTextures(1, &color);
    glBindTexture(GL_TEXTURE_2D, color);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
    glBindTexture(GL_TEXTURE_2D, 0);

    glGenRenderbuffers(1, &depth);
    glBindRenderbuffer(GL_RENDERBUFFER, depth);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, width, height);
    glBindRenderbuffer(GL_RENDERBUFFER, 0);

    glGenFramebuffers(1, &fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, color, 0);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depth);

    _display->draw( _scene, 0xFFFFFFFF, 0xFFFFFFFF );
    _display->resizeScreenshot( oldWidth, oldHeight );

    std::vector<unsigned char> data(width * height * 3, 0);
    glPixelStorei( GL_UNPACK_ALIGNMENT, 1 );
    glPixelStorei( GL_PACK_ALIGNMENT, 1 );
    glReadPixels( 0, 0, width, height, GL_RGB, GL_UNSIGNED_BYTE, &data[0] );
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

//    // width * height * RGB
//    std::vector<unsigned char> data(width * height * 3, 0);
//
//    _display->draw( _scene, 0xFFFFFFFF, 0xFFFFFFFF );
//
//    glReadBuffer( GL_BACK );
//
//    glPixelStorei( GL_UNPACK_ALIGNMENT, 1 );
//    glPixelStorei( GL_PACK_ALIGNMENT, 1 );
//    glReadPixels( 0, 0, width, height, GL_RGB, GL_UNSIGNED_BYTE, &data[0] );

    // Flip data top to bottom.
    std::vector<unsigned char> flipdat(width * height * 3, 0);
    int scanLen = 3 * width;
    for ( int i = 0 ; i < height; i++ )
    {
        unsigned char* srcLine = &data[ i * scanLen ];
        unsigned char* dstLine = &flipdat[ (height - i - 1) * scanLen ];
        memcpy(  dstLine, srcLine, scanLen );
    }

    writeJPEG( fileName.c_str(), width, height, &flipdat[0] );

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
