//******************************************************************************
//
//  Slightly Modified Font Manager Class
//
//  J.R. Gloudemans - 8/13/04
//   Phoenix Integration - 2012
//
//******************************************************************************

#ifndef _VSP_GRAPHIC_FONT_H
#define _VSP_GRAPHIC_FONT_H

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
* FontMgr class.
* This class loads font from file.
*/
class FontMgr
{
public:
    /*!
    * Constructor.
    */
    FontMgr();
    /*!
    * Destructor.
    */
    virtual ~FontMgr();

public:
    /*!
    * Load font from file and return the font object.
    */
    glfont::GLFont * loadFont();

private:
    glfont::GLFont * _font;
};
}
#endif
