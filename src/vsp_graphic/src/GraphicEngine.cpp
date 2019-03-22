#include "GraphicEngine.h"

#include "Scene.h"
#include "Display.h"
#include "Viewport.h"
#include "Background.h"

#include "OpenGLHeaders.h"
#include "stb_image_write.h"
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

void GraphicEngine::dumpScreenImage( std::string fileName, int width, int height, bool transparentBG, bool framebufferSupported, int filetype )
{
    GLuint color;
    GLuint depth;
    GLuint fbo;
    int oldWidth, oldHeight;
    // width * height * RGB
    std::vector<unsigned char> data(width * height * 4, 0);

    std::vector< VSPGraphic::Viewport * > vports = _display->getLayoutMgr()->getViewports();
    if ( transparentBG )
    {
        for ( int i = 0; i < vports.size(); i++ )
        {
            if ( vports[i] )
            {
                vports[i]->getBackground()->setAlpha( 0.0f );
            }
        }
    }

    if ( framebufferSupported )
    {
        oldWidth = _display->getLayoutMgr()->screenWidth();
        oldHeight = _display->getLayoutMgr()->screenHeight();

        _display->resizeScreenshot( width, height );

        glGenTextures(1, &color);
        glBindTexture(GL_TEXTURE_2D, color);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
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

        glPixelStorei( GL_UNPACK_ALIGNMENT, 1 );
        glPixelStorei( GL_PACK_ALIGNMENT, 1 );
        glReadPixels( 0, 0, width, height, GL_RGBA, GL_UNSIGNED_BYTE, &data[0] );
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }
    else
    {
        _display->draw( _scene, 0xFFFFFFFF, 0xFFFFFFFF );

        glReadBuffer( GL_BACK );

        glPixelStorei( GL_UNPACK_ALIGNMENT, 1 );
        glPixelStorei( GL_PACK_ALIGNMENT, 1 );
        glReadPixels( 0, 0, width, height, GL_RGBA, GL_UNSIGNED_BYTE, &data[0] );
    }

    for ( int i = 0; i < vports.size(); i++ )
    {
        if ( vports[i] )
        {
            vports[i]->getBackground()->setAlpha( 1.0f );
        }
    }

    // Flip data top to bottom.
    std::vector<unsigned char> flipdat(width * height * 4, 0);
    int scanLen = 4 * width;
    for ( int i = 0 ; i < height; i++ )
    {
        unsigned char* srcLine = &data[ i * scanLen ];
        unsigned char* dstLine = &flipdat[ (height - i - 1) * scanLen ];
        memcpy(  dstLine, srcLine, scanLen );
    }

    if ( filetype == PNG )
    {
        stbi_write_png( fileName.c_str(), width, height, 4, &flipdat[0], width * 4 );
    }
    else
    {
        printf( "Image type not supported.\n" );
    }
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
