#ifndef _VSP_GUI_2D_GL_WINDOW_H
#define _VSP_GUI_2D_GL_WINDOW_H

#include <FL/Fl.H>
#include <FL/Fl_Gl_Window.H>

#include "DrawObj.h"

namespace VSPGraphic
{
class GraphicEngine;
class Renderable;
}

class FuselageGeom;

namespace VSPGUI
{
/*
* This class provides all functionalites of a 2D opengl window.
*/
class VspSubGlWindow : public Fl_Gl_Window
{
public:
    /*
    * Constructor.
    * Parameters are left upper corner position of the window (x, y) and
    * window's width and height (w, h).
    *
    * drawObjScreen - specific drawObj screen to link.
    */
    VspSubGlWindow( int x, int y, int w, int h, DrawObj::ScreenEnum drawObjScreen );
    /*
    * Destructor.
    */
    virtual ~VspSubGlWindow();

// Compatibility for FLTK before 1.3.4
#if FL_API_VERSION < 10304
    int pixel_w() { return w(); }
    int pixel_h() { return h(); }
#endif

public:
    /*
    * Draw 2D Scene.
    */
    virtual void draw();

public:
    /*
    * Update window.
    */
    virtual void update();

public:
    /*!
    * Get graphic engine of this window.
    */
    VSPGraphic::GraphicEngine * getGraphicEngine()
    {
        return m_GEngine;
    }

protected:
    /*
    * Initialize GLEW.
    */
    virtual void _initGLEW();
    /*
    * Update Scene.
    */
    virtual void _update( std::vector<DrawObj *> objects );

private:
    static void _loadPointData( VSPGraphic::Renderable * destObj, DrawObj * drawObj );
    static void _loadLineData( VSPGraphic::Renderable * destObj, DrawObj * drawObj );

protected:
    /*
    * VSP Graphic Engine.
    */
    VSPGraphic::GraphicEngine * m_GEngine;
    unsigned int m_id;

private:
    DrawObj::ScreenEnum m_LinkedScreen;
    bool m_Initialized;
};
}
#endif