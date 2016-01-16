#ifndef _VSP_GRAPHIC_TGA_IMAGE_LOADER_H
#define _VSP_GRAPHIC_TGA_IMAGE_LOADER_H
#include <string>
#include <vector>

#include "ImageLoader.h"

#include "OpenGLHeaders.h"

namespace VSPGraphic
{
/*!
* TGA File loader.
*/
class TGALoader : public ImageLoader
{
    typedef struct
    {
        GLubyte idlength;                   // size of string that follows header (bytes)
        GLubyte colormaptype;           // 0: BGR, 1: Paletted
        GLubyte datatypecode;           // 2: uncompr BGR, 10: compr BGR
        GLubyte lo_colormaporigin;      // offset in palettespace (lo byte of short)
        GLubyte hi_colormaporigin;      // offset in palettespace (hi byte of short)
        GLubyte lo_colormaplength;      // number of entries in colour map (lo byte)
        GLubyte hi_colormaplength;      // number of entries in colour map (hi byte)
        GLubyte colormapdepth;          // size of entries in color map (bits)
        GLshort x_origin;                   // x origin (lower left)
        GLshort y_origin;                   // y origin (lower left)
        GLshort width;                      // image width
        GLshort height;                 // image height
        GLubyte bpp;                        // bits per pixel
        GLubyte imagedescriptor;        // descriptor bits (horizontal / vertical flip bits)
    } TGAHeader;

public:
    /*!
    * Constructor.
    */
    TGALoader();
    /*!
    * Destructor.
    */
    virtual ~TGALoader();

public:
    /*!
    * Load TGA file.
    */
    ImageData load( std::string fileName );
    /*!
    * Get TGA file extension.
    */
    std::string getExt();

private:
    ImageData _loadUncompressedUnmapped( FILE * stream, TGAHeader header );
    ImageData _loadCompressedUnmapped( FILE * stream, TGAHeader header );
};
}
#endif
