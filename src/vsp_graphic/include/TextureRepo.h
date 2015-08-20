#ifndef _VSP_GRAPHIC_TEXTURE_REPO_H
#define _VSP_GRAPHIC_TEXTURE_REPO_H

#include <string>
#include <vector>

namespace VSPGraphic
{
class Texture;

/*!
* Texture repositiory.
*/
class TextureRepo
{
public:
    /*!
    * Constructor.
    */
    TextureRepo();
    /*!
    * Destructor.
    */
    virtual ~TextureRepo();

    /*!
    * Get 2D texture to repository.  If the texture does
    * not exist, add it to repository.
    *
    * fileName - file path + name of the texture file.
    */
    Texture * get2DTexture( const char * fileName );

private:
    struct TextureInfo
    {
        std::string fileName;
        Texture * texture;
    };
    std::vector<TextureInfo> _textureRepo;
};
}
#endif