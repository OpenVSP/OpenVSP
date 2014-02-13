#include <assert.h>
#include <algorithm>

#include "Image.h"

#include "TGALoader.h"
#include "JPGLoader.h"

namespace VSPGraphic
{
Image::Image( std::string fileName )
{
    _loaders = _initLoaders();

    _fileName = fileName;
    _loadImage( fileName );
}
Image::~Image()
{
    if( _image.data )
    {
        free( _image.data );
    }

    for( int i = 0; i < ( int )_loaders.size(); i++ )
    {
        delete _loaders[i];
    }
    _loaders.clear();
}

bool Image::isValid()
{
    return _image.valid;
}

std::string Image::getFileName()
{
    return _fileName;
}

unsigned int Image::getWidth()
{
    return _image.width;
}

unsigned int Image::getHeight()
{
    return _image.height;
}

unsigned int Image::getType()
{
    return _image.type;
}

unsigned char * Image::getImageData()
{
    return _image.data;
}

void Image::_loadImage( std::string fileName )
{
    bool succeed;
    std::string name = fileName;

    unsigned int extIndex = name.find_last_of( '.' );
    if( extIndex == std::string::npos )
    {
        succeed = _loadWithoutExt( name );
    }
    else
    {
        std::string ext = name.substr( extIndex, name.size() - extIndex );
        succeed = _loadWithExt( name, ext );
    }

    if( !succeed )
    {
        _image.valid = false;
        _image.width = 0;
        _image.height = 0;
        _image.type = 0;
        _image.data = NULL;
    }
}

bool Image::_loadWithExt( std::string fileName, std::string ext )
{
    std::transform( ext.begin(), ext.end(), ext.begin(), ::tolower );

    for( int i = 0; i < ( int )_loaders.size(); i++ )
    {
        // Compare ext in lower case.
        std::string loaderExt = _loaders[i]->getExt();
        std::transform( loaderExt.begin(), loaderExt.end(), loaderExt.begin(), ::tolower );
        if( ext == loaderExt )
        {
            _image = _loaders[i]->load( fileName );
            return true;
        }
    }
    return false;
}

bool Image::_loadWithoutExt( std::string fileName )
{
    for( int i = 0; i < ( int )_loaders.size(); i++ )
    {
        _image = _loaders[i]->load( fileName );
        if( _image.valid )
        {
            return true;
        }
    }
    return false;
}

std::vector<ImageLoader *> Image::_loaders;

std::vector<ImageLoader *> Image::_initLoaders()
{
    std::vector<ImageLoader *> loaders;
    loaders.push_back( new TGALoader() );
    loaders.push_back( new JPGLoader() );
    return loaders;
}
}