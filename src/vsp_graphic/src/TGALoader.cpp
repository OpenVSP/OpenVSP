#include "OpenGLHeaders.h"

#include "TGALoader.h"
#include "stdlib.h"

namespace VSPGraphic
{
TGALoader::TGALoader() : ImageLoader()
{
}
TGALoader::~TGALoader()
{
}

ImageData TGALoader::load( std::string fileName )
{
    TGAHeader tgaheader;
    ImageData image = ImageData();

    FILE * TGAFile = fopen( fileName.c_str(), "rb" );
    if( !TGAFile )
    {
        return image;
    }

    if( fread( &tgaheader, sizeof( TGAHeader ), 1, TGAFile ) == 0 )
    {
        fclose( TGAFile );
        return image;
    }

    if( ( tgaheader.width <= 0 ) || ( tgaheader.height <= 0 ) || ( ( tgaheader.bpp != 24 ) && ( tgaheader.bpp != 32 ) ) )
    {
        fclose( TGAFile );
        return image;
    }

    switch( tgaheader.datatypecode )
    {
    case 2:
        image = _loadUncompressedUnmapped( TGAFile, tgaheader );
        fclose( TGAFile );
        return image;

    case 10:
        image = _loadCompressedUnmapped( TGAFile, tgaheader );
        fclose( TGAFile );
        return image;

    default:
        fclose( TGAFile );
        return image;
    }
}

std::string TGALoader::getExt()
{
    return ".TGA";
}

ImageData TGALoader::_loadUncompressedUnmapped( FILE * stream, TGAHeader header )
{
    ImageData image = ImageData();

    // Rewind stream so we don't make assumption on current location.
    rewind( stream );

    // Skip Header, Id, and Color Map block.
    unsigned int skippable = sizeof( header );
    skippable += header.idlength;
    skippable += header.colormapdepth / 8 * ( header.hi_colormaplength * 256 + header.lo_colormaplength );

    fseek( stream, skippable, SEEK_SET );
    if( ferror( stream ) )
    {
        return image;
    }

    // Update Image Attributes.
    image.width = header.width;
    image.height = header.height;

    switch( header.bpp )
    {
    case 24:
        image.type = GL_RGB;
        break;

    case 32:
        image.type = GL_RGBA;
        break;
    }

    // Calculate image size.
    unsigned int bytePerPixel = header.bpp / 8;
    unsigned int imageSizeInBytes = bytePerPixel * header.width * header.height;

    image.data = ( GLubyte * )malloc( imageSizeInBytes );
    if( !image.data )
    {
        return image;
    }

    // Load image.
    if( fread( image.data, 1, imageSizeInBytes, stream ) != imageSizeInBytes )
    {
        if( image.data )
        {
            free( image.data );
            image.data = NULL;
            return image;
        }
    }

    // Swap BGRA to RGBA.
    for( int i = 0; i < ( int )imageSizeInBytes; i += bytePerPixel )
    {
        image.data[i] ^= image.data[i + 2] ^= image.data[i] ^= image.data[i + 2];
    }

    image.valid = true;
    return image;
}

ImageData TGALoader::_loadCompressedUnmapped( FILE * stream, TGAHeader header )
{
    ImageData image = ImageData();

    // Rewind stream so we don't make assumption on current location.
    rewind( stream );

    unsigned int skippable = sizeof( header );
    skippable += header.idlength;
    skippable += header.colormapdepth / 8 * ( header.hi_colormaplength * 256 + header.lo_colormaplength );

    fseek( stream, skippable, SEEK_SET );
    if( ferror( stream ) )
    {
        return image;
    }

    // Update Image Attributes.
    image.width = header.width;
    image.height = header.height;

    switch( header.bpp )
    {
    case 24:
        image.type = GL_RGB;
        break;

    case 32:
        image.type = GL_RGBA;
        break;
    }

    // Calculate image size.
    unsigned int bytePerPixel = header.bpp / 8;
    unsigned int imageSizeInBytes = bytePerPixel * header.width * header.height;

    image.data = ( GLubyte * )malloc( imageSizeInBytes );
    if( !image.data )
    {
        return image;
    }

    // Decipher Run Length Encoding (RLE).

    GLubyte * rgb = new GLubyte[bytePerPixel];
    unsigned int byteCounter = 0;

    do
    {
        GLubyte runCount = 0;
        if( fread( &runCount, sizeof( GLubyte ), 1, stream ) == 0 )
        {
            free( image.data );
            image.data = NULL;
            return image;
        }

        // Process raw data.
        if( runCount < 128 )
        {
            runCount++;
            for( short i = 0; i < runCount; i++ )
            {
                if( fread( rgb, sizeof( unsigned char ), bytePerPixel, stream ) != bytePerPixel )
                {
                    delete rgb;
                    free( image.data );
                    image.data = NULL;
                    return image;
                }
                image.data[byteCounter] = rgb[0];
                image.data[byteCounter + 1] = rgb[1];
                image.data[byteCounter + 2] = rgb[2];

                byteCounter += bytePerPixel;
            }
        }
        else
        {
            runCount -= 127;
            if( fread( rgb, sizeof( unsigned char ), bytePerPixel, stream ) != bytePerPixel )
            {
                delete rgb;
                free( image.data );
                image.data = NULL;
                return image;
            }
            for( short i = 0; i < runCount; i++ )
            {
                image.data[byteCounter] = rgb[0];
                image.data[byteCounter + 1] = rgb[1];
                image.data[byteCounter + 2] = rgb[2];

                if( bytePerPixel == 4 )
                {
                    image.data[byteCounter + 3] = rgb[3];
                }

                byteCounter += bytePerPixel;
            }
        }
    }
    while( byteCounter < imageSizeInBytes );

    delete rgb;

    // Swap BGRA to RGBA.
    for( int i = 0; i < ( int )imageSizeInBytes; i += bytePerPixel )
    {
        image.data[i] ^= image.data[i + 2] ^= image.data[i] ^= image.data[i + 2];
    }

    image.valid = true;
    return image;
}
}