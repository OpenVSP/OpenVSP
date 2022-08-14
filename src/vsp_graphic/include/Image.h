#ifndef _VSP_GRAPHIC_IMAGE_H
#define _VSP_GRAPHIC_IMAGE_H

#include <vector>
#include <string>

#include "ImageLoader.h"

namespace VSPGraphic
{
/*!
* Image Class.
* This class loads image and return image info.
*/
class Image
{
public:
    /*!
    * Constructor.
    * fileName - the path and name of the image file.
    */
    Image( std::string fileName );

    Image( unsigned int width, unsigned int height, unsigned int bpp );

    /*!
    * Destructor.
    */
    virtual ~Image();

public:
    /*!
    * Return if the image is successfully loaded.
    */
    bool isValid();

    /*!
    * Return the path and name of image file.
    */
    std::string getFileName();

    /*!
    * Width of image in pixels.
    */
    unsigned int getWidth();

    /*!
    * Height of image in pixels.
    */
    unsigned int getHeight();

    /*!
    * Return the type of image.  GL_RGB or GL_RGBA.
    */
    unsigned int getType();

    unsigned int getBPP();

    /*!
    * Return the image's byte data.
    */
    unsigned char * getImageData();

    void flipud();

    void crop( unsigned int x0, unsigned int y0, unsigned int w, unsigned int h );

    void alphabounds( unsigned int &x0, unsigned int &y0, unsigned int &xf, unsigned int &yf );

protected:
    /*!
    * Load image from file.
    */
    virtual void _loadImage( std::string fileName );

private:
    bool _loadWithExt( std::string fileName, std::string ext );
    bool _loadWithoutExt( std::string fileName );

    static std::vector<ImageLoader *> _initLoaders();

private:
    std::string _fileName;
    ImageData _image;

    static std::vector<ImageLoader *> _loaders;
};
}
#endif
