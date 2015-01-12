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
class Texture
{
public:
    /*!
    * Constructor.
    */
    Texture( Image * image );
    /*!
    * Destructor.
    */
    virtual ~Texture();

public:
    /*!
    * Bind this texture to current state.  Must implement.
    */
    virtual void bind() = 0;
    /*!
    * Unbind textures from current state.  Must implement.
    */
    virtual void unbind() = 0;

    int getImWidth()
    {
        return _imWidth;
    }

    int getImHeight()
    {
        return _imHeight;
    }

protected:
    unsigned int _textureID;

    int _imWidth;
    int _imHeight;
};
}
#endif