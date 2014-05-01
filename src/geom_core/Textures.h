#if !defined(VSP_TEXTURES__INCLUDED_)
#define VSP_TEXTURES__INCLUDED_

#include "ParmContainer.h"
#include "Parm.h"

/*!
* Information needed to build a single texture.
*/
class Texture : public ParmContainer
{
public:
    /*!
    * Construct a texture.
    */
    Texture( std::string fileName );
    /*!
    * Destructor.
    */
    virtual ~Texture();

public:
    /*!
    * Rename display name.
    */
    virtual void Rename( std::string name );

public:
    /*!
    * On texture change, inform geometry to update.
    */
    virtual void ParmChanged( Parm* parm_ptr, int type );

public:
    /*!
    * Display or user defined name on GUI.
    */
    std::string m_DisplayName;
    /*!
    * File path + file name.
    */
    std::string m_FileName;

    /*!
    * Translate on U coordinate.
    */
    Parm m_U;
    /*!
    * Translate on W coordinate.
    */
    Parm m_W;

    /*!
    * Scale on U coordinate.
    */
    Parm m_UScale;
    /*!
    * Scale on W coordinate.
    */
    Parm m_WScale;

    /*!
    * Texture Alpha.
    */
    Parm m_Transparency;

    /*!
    * Flag to flip U coordinate.
    */
    BoolParm m_FlipU;
    /*!
    * Flag to flip W coordinate. 
    */
    BoolParm m_FlipW;
};

/*!
* Textures class manages textures in geometry.
*/
class Textures
{
public:
    /*!
    * Construct a Textures Object.
    */
    Textures();
    /*!
    * Destructor.
    */
    virtual ~Textures();

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
    virtual vector<std::string> GetTextureVec();

    /*!
    * Find and return Texture References from a list of texture id.  If search failed
    * on one of the texture id, that texture id will be ignored.  texture_id_vec is
    * the id vector.
    */
    virtual vector<Texture*> FindTextureVec( vector<std::string> texture_id_vec );

protected:
    std::vector<Texture*> m_TextureList;
};
#endif