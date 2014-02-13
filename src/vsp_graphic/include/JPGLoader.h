#ifndef _VSP_GRAPHIC_JPG_IMAGE_LOADER_H
#define _VSP_GRAPHIC_JPG_IMAGE_LOADER_H

#include <string>

#include "ImageLoader.h"

namespace VSPGraphic
{
class JPGLoader : public ImageLoader
{
public:
    JPGLoader();
    virtual ~JPGLoader();

public:
    ImageData load( std::string fileName );
    std::string getExt();
};
}
#endif