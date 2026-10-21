#ifndef _VSP_GRAPHIC_GRAPHIC_ENGINE_ENTRY_H
#define _VSP_GRAPHIC_GRAPHIC_ENGINE_ENTRY_H

#include <string>

namespace VSPGraphic
{
class Scene;
class Display;

/*!
* This is the Entry Object to access VSPGraphic.
*/
class GraphicEngine
{
public:
    /*!
    * Constructor.
    */
    GraphicEngine();
    /*
    * Destructor.
    */
    virtual ~GraphicEngine();

public:
    /*
    * Draw Scene.
    */
    void draw( bool vecout = false );

    /*!
    * Draw scene with selection.
    *
    * x, y - mouse locations.
    */
    void draw( int x, int y );

    enum filetype { PNG
    };

    enum vectorfiletype { SVG, PDF
    };

    /*!
    * Dump screen data to JPEG image.
    */
    void dumpScreenImage( const std::string &fileName, int width, int height, bool transparentBG, bool autcrop, bool framebufferSupported, int filetype );

    /*!
    * Write the scene to a vector file of the vectorfiletype given, through gl2ps.  With autocrop
    * the page is cropped to what is drawn.  scale is the size of a pixel on the page, in points.
    * The OpenGL context must be current.  Returns false when the file cannot be written.
    */
    bool dumpScreenVector( const std::string &fileName, int filetype, bool autocrop, double scale );

public:
    /*!
    * Initialize Glew.
    */
    static void initGlew();

public:
    /*!
    * Get Scene Object.
    */
    Scene * getScene();

    /*!
    * Get Display Object.
    */
    Display * getDisplay();

private:
    Scene * _scene;
    Display * _display;
};
}
#endif
