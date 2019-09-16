#include "OpenGLHeaders.h"
#include "Texture2D.h"
#include "Image.h"

namespace VSPGraphic
{
    Texture2D::Texture2D( Image * image )
    {
        _textureID = 0;
        _initialize( image );
        _imWidth = 0;
        _imHeight = 0;
    }
    Texture2D::~Texture2D()
    {
        if( _textureID != 0 )
        {
            glDeleteTextures( 1, &_textureID );
        }
    }

    void Texture2D::bind()
    {
        glBindTexture( GL_TEXTURE_2D, _textureID );
        glEnable( GL_TEXTURE_2D );
    }

    void Texture2D::unbind()
    {
        glDisable( GL_TEXTURE_2D );
    }

    void Texture2D::_initialize( Image * image )
    {
        glGenTextures( 1, &_textureID );

        if( !image->isValid() )
        {
            return;
        }

        if( !image->getImageData() )
        {
            return;
        }

        GLint max;
        glGetIntegerv( GL_MAX_TEXTURE_SIZE, &max );

        if( ( GLint )image->getHeight() > max )
        {
            return;
        }

        if( ( GLint )image->getWidth() > max )
        {
            return;
        }

        glBindTexture( GL_TEXTURE_2D, _textureID );
        glEnable( GL_TEXTURE_2D );

        glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER );
        glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER );
        glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
        glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR );
        // Generate MipMap using OpenGL 1.4
        glTexParameteri(GL_TEXTURE_2D, GL_GENERATE_MIPMAP, GL_TRUE);

        glTexEnvf( GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE );

        // Set border.
        GLfloat rgba[] = {1.0f, 1.0f, 1.0f, 0.0f};
        glTexParameterfv( GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, rgba );

        glPixelStorei( GL_UNPACK_ALIGNMENT, 1 );

        _imWidth = image->getWidth();
        _imHeight = image->getHeight();

        // Load Texture.
        if( image->getType() == GL_RGBA )
        {
            glTexImage2D( GL_TEXTURE_2D, 0, 4, image->getWidth(), image->getHeight(), 0, GL_RGBA, GL_UNSIGNED_BYTE, image->getImageData() );
        }
        else
        {
            glTexImage2D( GL_TEXTURE_2D, 0, 3, image->getWidth(), image->getHeight(), 0, GL_RGB, GL_UNSIGNED_BYTE, image->getImageData() );
        }

        // Unbind Texture.
        glDisable( GL_TEXTURE_2D );
        glBindTexture( GL_TEXTURE_2D, 0 );
    }
}
