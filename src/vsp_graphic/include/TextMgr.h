#ifndef _VSP_GRAPHIC_TEXT_RENDER_H
#define _VSP_GRAPHIC_TEXT_RENDER_H

#include <vector>
#include <string>
#include <map>

namespace glfont
{
class GLFont;
}

namespace VSPGraphic
{
/*!
* TextMgr class.
*/
class TextMgr
{
public:
    /*!
    * Constructor.
    */
    TextMgr();
    /*!
    * Destructor.
    */
    virtual ~TextMgr();

    /*!
    * Load font from file and return the font object.
    */
    glfont::GLFont * loadFont();

    /*!
    * Draw Text.
    * font - type of font.
    * scale - scale of the text.
    * str - text to draw.
    * x0, y0 - center location of the text.
    * xoffset, yoffset - offset from the center.
    */
    static void drawText( glfont::GLFont * font, float scale, const std::string &str, float x0, float y0, float xoffset, float yoffset );

private:

    unsigned int texID;
    glfont::GLFont * _font;
};
}
#endif
