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
    void draw();

    /*!
    * Draw scene with selection.
    *
    * x, y - mouse locations.
    */
    void draw( int x, int y );

    enum filetype { PNG
    };

    /*!
    * Dump screen data to JPEG image.
    */
    void dumpScreenImage( std::string fileName, int width, int height, bool transparentBG, bool framebufferSupported, int filetype );

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
