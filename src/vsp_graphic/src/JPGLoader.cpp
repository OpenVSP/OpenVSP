#include "OpenGLHeaders.h"

#include "JPGLoader.h"
#include "stb_image.h"
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


    return iData;
}

std::string JPGLoader::getExt()
{
    return ".JPG";
}
}
