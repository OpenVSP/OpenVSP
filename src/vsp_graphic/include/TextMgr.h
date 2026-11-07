#ifndef _VSP_GRAPHIC_TEXT_RENDER_H
#define _VSP_GRAPHIC_TEXT_RENDER_H

#include <vector>
#include <string>
#include <map>

namespace VSPGraphic
{
/*!
* TextMgr class.  Text is typeset in the embedded DejaVu Sans, with any math between dollar signs as LaTeX writes
* it, and rendered to a texture at the size it is drawn on screen.
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
    * Forget the rendered text.
    */
    virtual void clearFont();

    /*!
    * Draw Text, in screen pixels.
    * scale - scale of the text; a line at scale one is 1/126 of the viewport's larger side.
    * str - text to draw, UTF-8.
    * x0, y0 - center location of the text, in the current modelview.
    * xoffset, yoffset - offset from the center, as fractions of the text's width and height.
    */
    void drawText( float scale, const std::string &str, float x0, float y0, float xoffset, float yoffset );

    /*!
    * The width and height drawText draws text at, in the viewport's normalized device coordinates.
    */
    void getTextSize( float scale, const std::string &str, float &w, float &h );

private:

    // Text rendered to a texture, w by h pixels.
    struct TextImage
    {
        unsigned int tex;
        int w;
        int h;
    };

    const TextImage & getImage( const std::string &str, float scale );

    std::map< std::string, TextImage > _images;
};
}
#endif
