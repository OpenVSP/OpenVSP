#include <assert.h>
#include <MultiImageLoader.h>
#include "OpenGLHeaders.h"

#include "Image.h"
#include <algorithm>

namespace VSPGraphic
{
Image::Image( std::string fileName )
{
    _loaders = _initLoaders();

    _fileName = fileName;
    _loadImage( fileName );
}

Image::Image( unsigned int width, unsigned int height, unsigned int bpp )
{
    _loaders = _initLoaders();

    _image.valid = true;
    _image.width = width;
    _image.height = height;
    if ( bpp == 4 )
    {
        _image.type = GL_RGBA;
    }
    else
    {
        _image.type = GL_RGB;
    }

    _image.data = new unsigned char[ _image.width * _image.height * bpp ];
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

void Image::flipud()
{
    int bpp = 3;
    if ( _image.type == GL_RGBA )
    {
        bpp = 4;
    }

    // Flip data top to bottom.
    unsigned int scanLen = bpp * _image.width;
    unsigned int siz = _image.height * scanLen;
    unsigned char *data = new unsigned char[ siz ];

    for ( unsigned int i = 0 ; i < _image.height; i++ )
    {
        unsigned char* srcLine = &_image.data[ i * scanLen ];
        unsigned char* dstLine = &data[ (_image.height - i - 1) * scanLen ];
        memcpy( dstLine, srcLine, scanLen );
    }
    memcpy( _image.data, data, siz );
    delete[] data;
}

void Image::_loadImage( std::string fileName )
{
    bool succeed;
    std::string name = fileName;

    std::string::size_type extIndex = name.find_last_of( '.' );
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
        if( _loaders[i]->acceptExt( ext ) )
        {
            _image = _loaders[i]->load( fileName );
            flipud();
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
            flipud();
            return true;
        }
    }
    return false;
}

std::vector<ImageLoader *> Image::_loaders;

std::vector<ImageLoader *> Image::_initLoaders()
{
    std::vector<ImageLoader *> loaders;
    loaders.push_back( new MultiImageLoader() );
    return loaders;
}
}
