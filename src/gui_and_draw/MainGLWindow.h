#ifndef VSP_GUI_GL_WINDOW_H
#define VSP_GUI_GL_WINDOW_H

#include <FL/Fl.H>
#include <FL/Fl_Gl_Window.H>
#include <vector>
#include <string>

#include <glm/glm.hpp>

#include "Vec3d.h"
#include "Common.h"

#include "DrawObj.h"

namespace VSPGraphic
{
class GraphicEngine;
class Renderable;
}

class ScreenMgr;

namespace VSPGUI
{
class VspGlWindow : public Fl_Gl_Window
{
public:
    /*
    * Constructor.
    * drawObjScreen - specific drawObj screen to link.
    */
    VspGlWindow( int x, int y, int w, int h, ScreenMgr * mgr, DrawObj::ScreenEnum drawObjScreen );
    /*
    * Destructor.
    */
    virtual ~VspGlWindow();

public:
    /*!
    * Set Window Layout.
    * Row - Number of Rows.
    * Column - Number of Columns.
    */
    virtual void setWindowLayout( int row, int column );
    /*!
    * Set view for current selected viewport.
    * Acceptable type are VSP_CAM_TOP, VSP_CAM_FRONT, VSP_CAM_LEFT, VSP_CAM_LEFT_ISO,
    * VSP_CAM_BOTTOM, VSP_CAM_REAR, VSP_CAM_RIGHT, VSP_CAM_RIGHT_ISO, VSP_CAM_CENTER.
    */
    virtual void setView( VSPGraphic::Common::VSPenum type );

public:
    /*!
    * Get graphic engine of this window.
    */
    VSPGraphic::GraphicEngine * getGraphicEngine()
    {
        return m_GEngine;
    }

public:
    /*!
    * Pan current selected camera.
    *
    * dx - Direction on x-axis.  Positive moves right, negtive moves left.
    * dy - Direction on y-axis.  Positive moves up, negtive moves down.
    * precisionOn - pan camera slowly.
    */
    virtual void pan( int dx, int dy, bool precisionOn = false );

    /*!
    * Zoom current selected camera.
    *
    * delta - Zoom in or out.  Positive zooms out, negtive zooms in.
    * precisionOn - If true, zoom slowly.
    */
    virtual void zoom( int delta, bool precisionOn = false );

public:
    /*!
    * Load DrawObjs.
    */
    void LoadDrawObjs( vector< DrawObj* > & draw_obj_vec );

// Override Fl_Gl_Window Functions.
public:
    virtual void show();
    virtual void draw();
    virtual int  handle( int fl_event );

// Util Functions.
public:
    virtual void update();

// Private helper functions.
private:
    void _initGLEW();

    void _updateTextures( DrawObj * drawObj );

    void _loadTrisData( VSPGraphic::Renderable * destObj, DrawObj * drawObj );
    void _loadXSecData( VSPGraphic::Renderable * destObj, DrawObj * drawObj );
    void _loadMarkData( VSPGraphic::Renderable * destObj, DrawObj * drawObj );

    void _update( std::vector<DrawObj *> objects );

    void _setLighting( DrawObj * drawObj );

    bool _checkIfPickingIsNeeded( std::vector<DrawObj *> DOs );

    void _generatePickGeomDOs();

    void _generatePickVertDOs();
    void _clearPickVertDOs();

    struct ID;

    ID * _findID( std::string geomID );
    ID * _findID( unsigned int bufferID );

    void _updateBuffer( std::vector<DrawObj *> objects );

    std::vector<std::vector<vec3d>> _generateTexCoordFromXSec( DrawObj * drawObj );
    double _distance( vec3d pointA, vec3d pointB );

    void OnPush( int x, int y );
    void OnDrag( int x, int y );
    void OnRelease( int x, int y );
    void OnKeyup( int x, int y );
    void OnKeydown();

private:
    VSPGraphic::GraphicEngine * m_GEngine;
    ScreenMgr * m_ScreenMgr;

    struct TextureID
    {
        unsigned int bufferTexID;
        unsigned int geomTexID;
    };
    struct ID
    {
        unsigned int bufferID;

        std::string geomID;
        std::vector<TextureID> textureIDs;

        TextureID * find( unsigned int geomTexID )
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

    float m_LightAmb;
    float m_LightDiff;
    float m_LightSpec;

    int m_mouse_x;
    int m_mouse_y;

    DrawObj::ScreenEnum m_LinkedScreen;

    bool m_initialized;

    glm::vec2 m_prevLB;
    glm::vec2 m_prevMB;
    glm::vec2 m_prevRB;
    glm::vec2 m_prevAltLB;
    glm::vec2 m_prevCtrlLB;
    glm::vec2 m_prevMetaLB;
    glm::vec2 m_prevLBRB;

    std::vector<DrawObj> m_PickableGeoms;
    std::vector<DrawObj> m_PickableVertices;
};
}
#endif