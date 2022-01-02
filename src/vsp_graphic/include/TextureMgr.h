#ifndef _VSP_GRAPHIC_TEXTURE_MANAGER_H
#define _VSP_GRAPHIC_TEXTURE_MANAGER_H

#include <string>
#include <vector>

#include "Common.h"
#include "glm/glm.hpp"

namespace VSPGraphic
{
class Lighting;
class Texture2D;

/*!
* Texture Manager.
*/
class TextureMgr
{
public:
    /*!
    * Constructor.
    */
    TextureMgr();
    /*!
    * Destructor.
    */
    virtual ~TextureMgr();

    /*!
    * Add a texture to manager.
    *
    * texture - texture to add.
    * return id for this texture.
    */
    virtual unsigned int add( Texture2D * texture );

    /*!
    * Remove texture from manager.
    *
    * id - texture id to remove.
    */
    virtual void remove( unsigned int id );

    /*!
    * Bind texture information to render pipeline.
    */
    virtual void bind();

    /*!
    * Unbind textures from render pipeline.
    */
    virtual void unbind();

    /*!
    * Set Lighting.
    */
    void setLighting( Lighting * lighting );

    /*!
    * Set Texture Blend Mode.
    *
    * Textures can be blended or layered.
    */
    void setTextureMode( Common::VSPenum textureMode );

    /*!
    * Translate Texture.
    *
    * id - texture id to translate.
    * s - translate on s or x axis.
    * t - translate on t or y axis.
    * r - translate on r or z axis.
    */
    virtual void translate( unsigned int id, float s, float t, float r = 0.f );
    /*!
    * Scale texture.
    *
    * id - texture id to scale.
    * s - scale on s or x axis.
    * t - scale on t or y axis.
    * r - scale on r or z axis.
    */
    virtual void scale( unsigned int id, float sRatio, float tRatio, float rRatio = 1.f );
    /*!
    * Flip Texture Coordinates.
    *
    * id - texture id to flip.
    * flipU - flip u or x axis.
    * flipW - flip w or y axis.
    */
    virtual void flipCoord( unsigned int id, bool flipU, bool flipW );
    /*!
    * Set Alpha.
    *
    * id - texture id.
    * alpha - alpha value.
    */
    virtual void transparentize( unsigned int id, float alpha );
    /*!
    * Get ids of all textures that are added to this manager.
    */
    virtual std::vector<unsigned int> getIds();

    /*!
    * Get hardware limitation of max texture size.
    */
    static int getMaxTextureSize();
    /*!
    * Get hardware limitation of max number of combined textures.
    */
    static int getMaxCombineTextures();

    /*!
    * Get translate value of s.
    *
    * id - texture id.
    */
    virtual float getTranslateS( unsigned int id );
    /*!
    * Get translate value of t.
    *
    * id - texture id.
    */
    virtual float getTranslateT( unsigned int id );
    /*!
    * Get translate value of r.
    *
    * id - texture id.
    */
    virtual float getTranslateR( unsigned int id );
    /*!
    * Get scale value of s.
    *
    * id - texture id.
    */
    virtual float getScaleS( unsigned int id );
    /*!
    * Get scale value of t.
    *
    * id - texture id.
    */
    virtual float getScaleT( unsigned int id );
    /*!
    * Get scale value of r.
    *
    * id - texture id.
    */
    virtual float getScaleR( unsigned int id );

protected:
    /*!
    * Generate unique id for textures.
    */
    virtual unsigned int _generateId();

    /*
    * Texture Properties.
    *
    * id - Id linked to this Texture.  Generated from add(), unique to all TexInfos.
    *
    * texture - Texture info and data.
    *
    * texCoordMat - Provides Texture Transformation Info.
    *
    * flipU - Flip U Flag.
    *
    * flipW - Flip W Flag.
    *
    * texAlpha - Alpha value of the Texture.
    */
    struct TexInfo
    {
        unsigned int id;

        Texture2D * texture;

        glm::mat4 texCoordMat;

        bool flipU;
        bool flipW;

        float texAlpha;
    };
    /*
    * List of attached textures and their transformation info.
    */
    std::vector<TexInfo> _coordList;

private:
    static glm::vec2 _flipToVector( bool flipU, bool flipW );
    std::vector<glm::vec4> _getLightSwitches();

    Lighting * _lights;
    Common::VSPenum _textureMode;
};
}
#endif
