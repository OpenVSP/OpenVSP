#include <assert.h>
#include <MultiImageLoader.h>
#include "OpenGLHeaders.h"

#include "Image.h"
#include <algorithm>
#include <cstring>

namespace VSPGraphic
{
Image::Image( const std::string &fileName )
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
        delete[] _image.data;
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

unsigned int Image::getBPP()
{
    if ( _image.type == GL_RGBA )
    {
        return 4;
    }
    return 3;
}

unsigned char * Image::getImageData()
{
    return _image.data;
}

void Image::transpose()
{
    unsigned int w = _image.width;
    unsigned int h = _image.height;

    unsigned int bpp = getBPP();
    unsigned int scanLen = bpp * w;
    unsigned int scanLen2 = bpp * h;

    unsigned char *data = new unsigned char[ _image.width * _image.height * bpp ];

    for ( unsigned int i = 0 ; i < h; i++ )
    {
        for ( unsigned int j = 0; j < w; j++ )
        {
            unsigned int k1 = i * scanLen + j * bpp;
            unsigned int k2 = j * scanLen2 + i * bpp;

            for ( unsigned int k = 0; k < bpp; k++ )
            {
                data[ k2 + k ] = _image.data[ k1 + k ];
            }
        }
    }

    delete[] _image.data;
    _image.data = data;

    _image.width = h;
    _image.height = w;
}

void Image::flipud()
{
    unsigned int w = _image.width;
    unsigned int h = _image.height;

    unsigned int bpp = getBPP();
    unsigned int scanLen = bpp * w;

    for ( unsigned int i = 0 ; i < h / 2; i++ )
    {
        for ( unsigned int j = 0; j < w; j++ )
        {
            unsigned int k1 = i * scanLen + j * bpp;
            unsigned int k2 = (h - 1 - i) * scanLen + j * bpp;

            for ( unsigned int k = 0; k < bpp; k++ )
            {
                std::swap( _image.data[ k1 + k ], _image.data[ k2 + k ] );
            }
        }
    }
}

void Image::fliplr()
{
    unsigned int w = _image.width;
    unsigned int h = _image.height;

    unsigned int bpp = getBPP();
    unsigned int scanLen = bpp * w;

    for ( unsigned int i = 0 ; i < h; i++ )
    {
        for ( unsigned int j = 0; j < w / 2; j++ )
        {
            unsigned int k1 = i * scanLen + j * bpp;
            unsigned int k2 = i * scanLen + ( w - 1 - j ) * bpp;

            for ( unsigned int k = 0; k < bpp; k++ )
            {
                std::swap( _image.data[ k1 + k ], _image.data[ k2 + k ] );
            }
        }
    }
}

void Image::rot90()
{
    transpose();
    flipud();
}

void Image::rot180()
{
    fliplr();
    flipud();
}

void Image::rot270()
{
    transpose();
    fliplr();
}

void Image::crop( unsigned int x0, unsigned int y0, unsigned int w, unsigned int h )
{
    unsigned int bpp = getBPP();

    if ( x0 + w > _image.width )
    {
        w = _image.width - x0;
    }

    if ( y0 + h > _image.height )
    {
        h = _image.height - y0;
    }

    unsigned int oScanLen = bpp * _image.width;

    unsigned int scanLen = bpp * w;
    unsigned int siz = h * scanLen;
    unsigned char *data = new unsigned char[ siz ];

    for ( unsigned int i = 0 ; i < h; i++ )
    {
        unsigned char* srcLine = &_image.data[ ( i + y0 ) * oScanLen + x0 * bpp ];
        unsigned char* dstLine = &data[ i * scanLen ];
        memcpy( dstLine, srcLine, scanLen );
    }

    _image.width = w;
    _image.height = h;
    memcpy( _image.data, data, siz );
    delete[] data;
}

void Image::alphabounds( unsigned int &x0, unsigned int &y0, unsigned int &xf, unsigned int &yf )
{
    unsigned int w = _image.width;
    unsigned int h = _image.height;

    unsigned int bpp = getBPP();
    unsigned int scanLen = bpp * w;

    y0 = -1;
    for ( unsigned int i = 0 ; i < h; i++ )
    {
        for ( unsigned int j = 0; j < w; j++ )
        {
            unsigned char alpha = _image.data[ i * scanLen + j * bpp + 3];

            if ( alpha != 0 ) // pixel is not transparent.
            {
                y0 = i;
                i = h;
                j = w;
            }
        }
    }

    x0 = -1;
    for ( unsigned int j = 0; j < w; j++ )
    {
        for ( unsigned int i = 0 ; i < h; i++ )
        {
            unsigned char alpha = _image.data[ i * scanLen + j * bpp + 3];

            if ( alpha != 0 ) // pixel is not transparent.
            {
                x0 = j;
                j = w;
                i = h;
            }
        }
    }

    yf = h - 1;
    if ( y0 != -1 )
    {
        for ( int i = h - 1; i >= 0; i-- ) // Must be signed.
        {
            for ( unsigned int j = 0; j < w; j++ )
            {
                unsigned char alpha = _image.data[ i * scanLen + j * bpp + 3 ];

                if ( alpha != 0 ) // pixel is not transparent.
                {
                    yf = i;
                    i = 0;
                    j = w;
                }
            }
        }
    }
    else
    {
        y0 = 0;
    }

    xf = w - 1;
    if ( x0 != -1 )
    {
        for ( int j = w - 1; j >= 0; j-- ) // Must be signed
        {
            for ( unsigned int i = 0 ; i < h; i++ )
            {
                unsigned char alpha = _image.data[ i * scanLen + j * bpp + 3 ];

                if ( alpha != 0 ) // pixel is not transparent.
                {
                    xf = j;
                    i = h;
                    j = 0;
                }
            }
        }
    }
    else
    {
        x0 = 0;
    }

}

void Image::addtransparentchannel()
{
    unsigned int bpp = getBPP();
    if ( bpp < 4 )
    {
        unsigned int w = _image.width;
        unsigned int h = _image.height;

        unsigned int scanLen = bpp * w;

        unsigned int bpp2 = 4;
        unsigned int scanLen2 = bpp2 * w;

        unsigned char *data = new unsigned char[ _image.width * _image.height * bpp2 ];

        for ( unsigned int i = 0 ; i < h; i++ )
        {
            for ( unsigned int j = 0; j < w; j++ )
            {
                unsigned int k1 = i * scanLen + j * bpp;
                unsigned int k2 = i * scanLen2 + j * bpp2;

                unsigned int k;
                for ( k = 0; k < bpp; k++ )
                {
                    data[ k2 + k ] = _image.data[ k1 + k ];
                }
                for ( ; k < bpp2; k++ )
                {
                    data[ k2 + k ] = 255;
                }

            }
        }

        _image.type = GL_RGBA;

        delete[] _image.data;
        _image.data = data;
    }
}

void Image::autotransparent()
{
    addtransparentchannel();

    unsigned int w = _image.width;
    unsigned int h = _image.height;

    unsigned int bpp = getBPP();
    unsigned int scanLen = bpp * w;

    for ( unsigned int i = 0 ; i < h; i++ )
    {
        for ( unsigned int j = 0; j < w; j++ )
        {
            unsigned int k = i * scanLen + j * bpp;

            int r = _image.data[ k + 0 ];
            int g = _image.data[ k + 1 ];
            int b = _image.data[ k + 2 ];
            int a = _image.data[ k + 3 ];

            if ( a == 255 ) // Only apply to fully opaque pixels.
            {
                a = ( 765 - ( r + b + g ) ) / 3;
                _image.data[ k + 0 ] = 0;
                _image.data[ k + 1 ] = 0;
                _image.data[ k + 2 ] = 0;
                _image.data[ k + 3 ] = a;
            }
        }
    }
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

bool Image::_loadWithExt( const std::string &fileName, std::string ext )
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

bool Image::_loadWithoutExt( const std::string &fileName )
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
