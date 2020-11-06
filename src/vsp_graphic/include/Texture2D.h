#ifndef _VSP_GRAPHIC_TEXTURE_BASE_H
#define _VSP_GRAPHIC_TEXTURE_BASE_H

#include "glm/glm.hpp"
#include "Common.h"

namespace VSPGraphic
{
    class Image;

    /*!
    * Texture class.
    * This is the base class for Textures.
    */
    class Texture2D
    {
    public:
        /*!
        * Constructor.
        */
        Texture2D( Image * image );
        /*!
        * Destructor.
        */
        virtual ~Texture2D();

        /*!
        * Bind this 2D Texture to current state.
        */
        virtual void bind();
        /*!
        * Unbind texture from current state.
        */
        virtual void unbind();

        int getImWidth()
        {
            return _imWidth;
        }

        int getImHeight()
        {
            return _imHeight;
        }

        unsigned int getTextureID()
        {
            return _textureID;
        }

    protected:
        /*!
        * Initialize Texture2D.
        */
        void _initialize( Image * image );

        unsigned int _textureID;

        int _imWidth;
        int _imHeight;
    };
}
#endif
