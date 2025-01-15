#ifndef _VSP_GRAPHIC_WATERMARK_H
#define _VSP_GRAPHIC_WATERMARK_H

#include <vector>
#include <string>

#include "Renderable.h"
#include "Common.h"
#include "glm/glm.hpp"

namespace VSPGraphic
{
class Texture2D;
class TextureMgr;
class TextMgr;

/*!
* Watermark in front of a viewport.
*/
class Watermark : public Renderable
{
public:
    /*!
    * Constructor.
    */
    Watermark();
    /*!
    * Destructor.
    */
    virtual ~Watermark();

public:

    virtual void setWidthHeight( int w, int h, const float & screenSizeDiffRatio );

    /*!
    * Reset to default.
    */
    virtual void reset();

    virtual void setText( const std::string &str );

    virtual void setTextScale( const float & s );

    /*!
    * Set fill color.
    */
    void setFillColor( float r, float g, float b, float a = 1.f );

    virtual void draw_watermark( TextMgr * textMgr );

public:

protected:
    virtual void _predraw();
    virtual void _draw();

private:

    // Font scale.
    float _textScale;
    // String
    std::string _text;




    int _vWidth;
    int _vHeight;

    float _screenSizeDiffRatio;
};
}
#endif
