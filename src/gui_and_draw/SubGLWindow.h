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
    virtual void LoadAllDrawObjs( vector< DrawObj* > & draw_obj_vec );

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
    void _updateTextures( DrawObj * drawObj );
    static void _loadXSecData( VSPGraphic::Renderable * destObj, DrawObj * drawObj );
    void _setLighting( DrawObj * drawObj );

    struct ID;

    ID * _findID( const std::string &geomID );
    ID * _findID( unsigned int bufferID );

    void _updateBuffer( const std::vector<DrawObj *> &objects );

protected:
    /*
    * VSP Graphic Engine.
    */
    VSPGraphic::GraphicEngine * m_GEngine;
    unsigned int m_id;

private:
    DrawObj::ScreenEnum m_LinkedScreen;
    bool m_Initialized;

    struct TextureID
    {
        unsigned int bufferTexID;
        std::string geomTexID;
    };

    struct ID
    {
        unsigned int bufferID;

        std::string geomID;
        std::vector<TextureID> textureIDs;

        TextureID * find( const std::string &geomTexID )
        {
            for( int i = 0; i < ( int )textureIDs.size(); i++ )
            {
                if( textureIDs[i].geomTexID == geomTexID )
                {
                    return &textureIDs[i];
                }
            }
            return NULL;
        }
    };
    std::vector<ID> m_ids;

};

class BG3DSubGlWindow : public VspSubGlWindow
{
public:

    BG3DSubGlWindow( int x, int y, int w, int h, DrawObj::ScreenEnum drawObjScreen );

    virtual void LoadAllDrawObjs( vector< DrawObj* > & draw_obj_vec );
};

}
#endif
