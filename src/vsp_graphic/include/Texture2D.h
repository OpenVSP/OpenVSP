#ifndef _VSP_GRAPHIC_TEXTURE_2D_H
#define _VSP_GRAPHIC_TEXTURE_2D_H

#include "Texture.h"

namespace VSPGraphic
{
class Image;

/*!
* 2D Texture.  Derived from Texture.
*/
class Texture2D : public Texture
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

public:
    /*!
    * Bind this 2D Texture to current state.
    */
    virtual void bind();
    /*!
    * Unbind texture from current state.
    */
    virtual void unbind();

protected:
    /*!
    * Initialize Texture2D.
    */
    void _initialize( Image * image );
};
}
#endif