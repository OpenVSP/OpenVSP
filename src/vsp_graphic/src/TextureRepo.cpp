#include "TextureRepo.h"

#include "Texture2D.h"

#include "Image.h"

namespace VSPGraphic
{
TextureRepo::TextureRepo()
{
}
TextureRepo::~TextureRepo()
{
    for( int i = 0; i < ( int )_textureRepo.size(); i++ )
    {
        delete _textureRepo[i].texture;
    }
}

Texture2D * TextureRepo::get2DTexture( const char * fileName, int key, int rot, bool fliplr, bool flipud, bool autotrans )
{
    // First check if this file has been loaded before.
    std::string fn = fileName;
    for( int i = 0; i < ( int )_textureRepo.size(); i++ )
    {
        if( _textureRepo[i].fileName == fn && _textureRepo[i].modKey == key )
        {
            return _textureRepo[i].texture;
        }
    }

    // If new file, create new texture.
    Image image( fileName );

    if ( rot == 1 ) image.rot90();
    else if ( rot == 2 ) image.rot180();
    else if ( rot == 3 ) image.rot270();
    if ( fliplr ) image.fliplr();
    if ( flipud ) image.flipud();
    if ( autotrans ) image.autotransparent();

    Texture2D * texture = new Texture2D( &image );

    TextureInfo tInfo;
    tInfo.fileName = fileName;
    tInfo.modKey = key;
    tInfo.texture = texture;

    _textureRepo.push_back( tInfo );

    return texture;
}

unsigned int TextureRepo::getTextureID( const char * fileName, int key )
{
    std::string fn = fileName;
    for ( int i = 0; i < (int)_textureRepo.size(); i++ )
    {
        if ( _textureRepo[i].fileName == fn && _textureRepo[i].modKey == key )
        {
            return _textureRepo[i].texture->getTextureID();
        }
    }
    return 0; // Indicates texture has not been created
}
}
