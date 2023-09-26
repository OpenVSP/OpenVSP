#include "GraphicEngine.h"

#include "Scene.h"
#include "Display.h"
#include "Viewport.h"
#include "Background.h"
#include "Image.h"

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

void GraphicEngine::dumpScreenImage( std::string fileName, int width, int height, bool transparentBG, bool autocrop, bool framebufferSupported, int filetype )
{
    GLuint color;
    GLuint depth;
    GLuint fbo;
    int oldWidth, oldHeight;
    // width * height * RGBA
    Image image( width, height, 4 );;

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
        glReadPixels( 0, 0, width, height, GL_RGBA, GL_UNSIGNED_BYTE, &(image.getImageData())[0] );
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }
    else
    {
        _display->draw( _scene, 0xFFFFFFFF, 0xFFFFFFFF );

        glReadBuffer( GL_BACK );

        glPixelStorei( GL_UNPACK_ALIGNMENT, 1 );
        glPixelStorei( GL_PACK_ALIGNMENT, 1 );
        glReadPixels( 0, 0, width, height, GL_RGBA, GL_UNSIGNED_BYTE, &(image.getImageData())[0] );
    }

    for ( int i = 0; i < vports.size(); i++ )
    {
        if ( vports[i] )
        {
            vports[i]->getBackground()->setAlpha( 1.0f );
        }
    }

    image.flipud();

    if ( autocrop )
    {
        unsigned int x0, y0, xf, yf, w, h;
        image.alphabounds( x0, y0, xf, yf );

        w = xf - x0 + 1;
        h = yf - y0 + 1;

        image.crop( x0, y0, w, h );
    }

    if ( filetype == PNG )
    {
        stbi_write_png( fileName.c_str(), image.getWidth(), image.getHeight(), image.getBPP(), &(image.getImageData())[0], image.getWidth() * image.getBPP() );
    }
    else
    {
        printf( "Image type not supported.\n" );
    }
}

void GraphicEngine::initGlew()
{
    GLenum glew_status = glewInit();
    if (glew_status != GLEW_OK) {
        fprintf(stderr, "Error: %s\n", glewGetErrorString(glew_status));
        exit(-1);
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
