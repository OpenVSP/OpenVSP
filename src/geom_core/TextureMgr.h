#if !defined(VSP_TEXTURE_MANAGER__INCLUDED_)
#define VSP_TEXTURE_MANAGER__INCLUDED_

#include "Texture.h"

#include <string>
#include <vector>

/*!
* This class manages textures in geometry.
*/
class TextureMgr
{
public:
    /*!
    * Construct a TextureMgr Object.
    */
    TextureMgr();
    /*!
    * Destructor.
    */
    virtual ~TextureMgr();

public:
    /*!
    * Attach a texture to geometry.  fileName is the file path plus texture file name.
    * Return an id for this texture.
    */
    virtual std::string AttachTexture( std::string fileName );

    /*!
    * Remove one attached texture.  texture_id is the id generated from AttachTexture().
    */
    virtual void RemoveTexture( std::string texture_id );

    /*!
    * Find Texture Reference.  texture_id is the id generated from AttachTexture().
    */
    virtual Texture * FindTexture( std::string texture_id );

    /*!
    * Get all texture ids.
    */
    virtual std::vector<std::string> GetTextureVec();

    /*!
    * Find and return Texture References from a list of texture id.  If search failed
    * on one of the texture id, that texture id will be ignored.  texture_id_vec is
    * the id vector.
    */
    virtual std::vector<Texture*> FindTextureVec( std::vector<std::string> texture_id_vec );

protected:
    std::vector<Texture*> m_TextureList;
};
#endif