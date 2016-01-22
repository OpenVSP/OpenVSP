#ifndef _VSP_GRAPHIC_MULTI_IMAGE_LOADER_H
#define _VSP_GRAPHIC_MULTI_IMAGE_LOADER_H

#include <string>

#include "ImageLoader.h"

namespace VSPGraphic
{
class MultiImageLoader : public ImageLoader
{
public:
    MultiImageLoader();
    virtual ~MultiImageLoader();

public:
    ImageData load( std::string fileName );
    virtual bool acceptExt( std::string e );
};
}
#endif
