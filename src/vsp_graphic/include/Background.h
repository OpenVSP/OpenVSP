#ifndef _VSP_GRAPHIC_BACKGROUND_H
#define _VSP_GRAPHIC_BACKGROUND_H

#include <vector>

#include "Renderable.h"
#include "Common.h"
#include "glm/glm.hpp"

namespace VSPGraphic
{
class Texture2D;
class TextureMgr;

/*!
* Background of a viewport.
*/
class Background : public Renderable
{
public:
    /*!
    * Constructor.
    */
    Background();
    /*!
    * Destructor.
    */
    virtual ~Background();

public:
    /*!
    * Set flag to preserve aspect ratio.
    * false by default.
    *
    * keepAR - flag to preserve aspect ratio.
    */
    virtual void preserveAR( bool keepAR );

    /*!
    * Scale Background Width.
    *
    * scale - scale value.
    */
    virtual void scaleW( float scale );
    /*!
    * Scale Background Height.
    *
    * scale - scale value.
    */
    virtual void scaleH( float scale );

    /*!
    * Offset on x-axis.
    *
    * offset - offset value.
    */
    virtual void offsetX( float offset );
    /*!
    * Offset on y-axis.
    *
    * offset - offset value.
    */
    virtual void offsetY( float offset );

    /*!
    * Set a flag to flip X coordinates of the image
    *
    * val - flag to flip X coordinates.
    */
    virtual void flipX( bool val );

    /*!
    * Attach Image to Background.
    *
    * texture - Texture image.
    */
    virtual void attachImage( Texture2D * texture );
    /*!
    * Remove Image from Background.
    */
    virtual void removeImage();

    /*!
    * Set Red value of the background.
    */
    virtual void setRed( float red );
    /*!
    * Set Green value of the background.
    */
    virtual void setGreen( float green );
    /*!
    * Set Blue value of the background.
    */
    virtual void setBlue( float blue );
    /*!
    * Set Alpha value of the background.
    */
    virtual void setAlpha( float alpha );

    virtual void setWidthHeight( int w, int h);

    /*!
    * Set Background render mode to Color or Image.
    * mode - VSP_BACKGROUND_COLOR, VSP_BACKGROUND_IMAGE.
    */
    void setBackgroundMode( Common::VSPenum mode );

    /*!
    * Reset to default.
    */
    virtual void reset();

public:
    Common::VSPenum getBackgroundMode();
    bool getARFlag();
    float getRed();
    float getGreen();
    float getBlue();
    float getScaleW();
    float getScaleH();
    float getOffsetX();
    float getOffsetY();
    unsigned int getTextureID();

protected:
    virtual void _predraw();
    virtual void _draw();

private:
    void _build();

private:
    std::vector<float> _background;
    std::vector<glm::vec3> _coord;
    std::vector<glm::vec2> _texCoord;

    bool _hasChanged;
    bool _keepAR;

    float _aspectRatioW;
    float _aspectRatioH;

    float _scaleX;
    float _scaleY;

    float _offsetX;
    float _offsetY;

    int _vWidth;
    int _vHeight;

    int _imWidth;
    int _imHeight;

    bool _flipX;

    TextureMgr * _textureMgr;
    unsigned int _texId;
    unsigned int _2DTextureID; // ID of Texture2d in _textureMgr

    Common::VSPenum _mode;
};
}
#endif
