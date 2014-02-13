#include "OpenGLHeaders.h"

#include "JPGLoader.h"
#include "loadjpg.h"

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
    iData.type = jpgData.d;
    iData.data = jpgData.data;

    return iData;
}

std::string JPGLoader::getExt()
{
    return ".JPG";
}
}