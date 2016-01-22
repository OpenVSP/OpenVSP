#ifndef _VSP_GRAPHIC_IMAGE_LOADER_BASE_H
#define _VSP_GRAPHIC_IMAGE_LOADER_BASE_H

#include <string>

namespace VSPGraphic
{
/*!
* Struct to hold image information.
*/
struct ImageData
{
    /*!
    * Image Validity.  Return if the image is successfully loaded.
    */
    bool valid;
    /*!
    * Image width in pixels.
    */
    unsigned int width;
    /*!
    * Image height in pixels.
    */
    unsigned int height;
    /*!
    * Image type.  (GL_RGB, GL_RGBA)
    */
    unsigned int type;
    /*!
    * Image data in bytes.
    */
    unsigned char * data;

    ImageData()
    {
        valid = false;
        width = 0;
        height = 0;
        type = 0;
        data = NULL;
    }
};

/*
* ImageLoader class is "Template" for image loaders.
*/
class ImageLoader
{
public:
    /*!
    * Constructor.
    */
    ImageLoader() {}
    /*!
    * Destructor.
    */
    virtual ~ImageLoader() {}

public:
    /*!
    * Load Image.  Must implement.
    */
    virtual ImageData load( std::string fileName ) = 0;

    /*!
    * Test if loader accepts image's extension (lower case).  Must implement.
    */
    virtual bool acceptExt( std::string e ) = 0;
};
}
#endif
