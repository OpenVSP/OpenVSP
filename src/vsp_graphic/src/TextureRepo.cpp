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

Texture2D * TextureRepo::get2DTexture( const char * fileName )
{
    // First check if this file has been loaded before.
    std::string fn = fileName;
    for( int i = 0; i < ( int )_textureRepo.size(); i++ )
    {
        if( _textureRepo[i].fileName == fn )
        {
            return _textureRepo[i].texture;
        }
    }

    // If new file, create new texture.
    Image image( fileName );
    Texture2D * texture = new Texture2D( &image );

    TextureInfo tInfo;
    tInfo.fileName = fileName;
    tInfo.texture = texture;

    _textureRepo.push_back( tInfo );

    return texture;
}
}