#include "OpenGLHeaders.h"

#include "JPGLoader.h"
#include "loadjpg.h"
#include <string.h>

namespace VSPGraphic
{
JPGLoader::JPGLoader() : ImageLoader()
{
}
JPGLoader::~JPGLoader()
{
}

ImageData JPGLoader::load( std::string fileName )
{
    ImageData iData;

    JpgData jpgData = loadJPEG( fileName.c_str() );

    iData.valid = true;
    if( jpgData.data == 0 )
    {
        iData.valid = false;
    }

    iData.width = jpgData.w;
    iData.height = jpgData.h;
    if( jpgData.d == 4 )
    {
        iData.type = GL_RGBA;
    }
    else
    {
        iData.type = GL_RGB;
    }

    // Flip data top to bottom.
    iData.data = new unsigned char[ iData.width * iData.height * jpgData.d];
    int scanLen = jpgData.d * iData.width;

    for ( int i = 0 ; i < (int)iData.height; i++ )
    {
        unsigned char* srcLine = &jpgData.data[ i * scanLen ];
        unsigned char* dstLine = &iData.data[ (iData.height - i - 1) * scanLen ];
        memcpy(  dstLine, srcLine, scanLen );
    }

    deleteJPEG( jpgData );

    return iData;
}

std::string JPGLoader::getExt()
{
    return ".JPG";
}
}
