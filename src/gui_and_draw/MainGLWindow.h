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
class Selectable;
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

// Compatibility for FLTK before 1.3.4
#if FL_API_VERSION < 10304
    int pixel_w() { return w(); }
    int pixel_h() { return h(); }
#endif

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
     * Sets the Center of Rotation for the Camera.
     *
     * @param center center of rotation.
     */
    virtual void setCOR( glm::vec3 center );

    /*!
      * Retrieves the center of rotation values.
      * @return point of rotation.
      */
    virtual glm::vec3 getCOR();

    /*!
     * Pan current selected camera.
     *
     * x - Direction on x-axis. Positive moves right, negative moves left.
     * y - Direction on y-axis. Positive moves up, negative moves left.
     */
    virtual void relativePan( float x, float y );

    /*!
     * Retrieves the pan values as a glm vec2
     * x - Distance in x direction.
     * y - Distance in y direction.
     */
    virtual glm::vec2 getPanValues();

    /*!
    * Zoom current selected camera relative to a point in world space.
    *
    * delta - Zoom in or out.  Positive zooms out, negative zooms in.
    */
    virtual void relativeZoom( float zoomValue );

    /*!
     * Get Relative Zoom Value
     */
    virtual float getRelativeZoomValue();

    /*!
    * Rotate about an axis
    *
    * angle - Angle about rotation.
    * rotAxis - Axis of rotation.
    */
    virtual void rotateSphere( float angleX, float angleY, float angleZ );

    /*!
     * Retrieves euler angles, yitch as x, yaw as y, roll as z.
     */
    virtual glm::vec3 getRotationEulerAngles();

// Override Fl_Gl_Window Functions.
public:
    virtual void draw();
    virtual int  handle( int fl_event );

// Util Functions.
public:
    virtual void update();

// Private helper functions.
private:
    void _initGLEW();

    void _updateTextures( DrawObj * drawObj );

    static void _loadTrisData( VSPGraphic::Renderable * destObj, DrawObj * drawObj );
    static void _loadQuadsData( VSPGraphic::Renderable * destObj, DrawObj * drawObj );
    static void _loadXSecData( VSPGraphic::Renderable * destObj, DrawObj * drawObj );
    static void _loadMarkData( VSPGraphic::Renderable * destObj, DrawObj * drawObj );

    void _setLighting( DrawObj * drawObj );

    void _setClipping( DrawObj * drawObj );

    struct ID;

    ID * _findID( std::string geomID );
    ID * _findID( unsigned int bufferID );

    void _updateBuffer( std::vector<DrawObj *> objects );

    void _sendFeedback( VSPGraphic::Selectable * selected );
    void _sendFeedback( std::vector<VSPGraphic::Selectable *> listOfSelected );

    void OnPush( int x, int y );
    void OnDrag( int x, int y );
    void OnRelease( int x, int y );
    int OnKeyup( int x, int y );
    int OnKeydown();
    void OnWheelScroll( int dx, int dy );

private:
    VSPGraphic::GraphicEngine * m_GEngine;

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

        TextureID * find( std::string geomTexID )
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

    DrawObj::ScreenEnum m_LinkedScreen;

    bool m_initialized;

    // Boolean for setting default opengl screen size once
    bool m_hasSetSize;

    glm::vec2 m_prevLB;
    glm::vec2 m_prevMB;
    glm::vec2 m_prevRB;
    glm::vec2 m_prevAltLB;
    glm::vec2 m_prevCtrlLB;
    glm::vec2 m_prevMetaLB;
    glm::vec2 m_startShiftLB;
    glm::vec2 m_prevLBRB;

protected:

    void _update( std::vector<DrawObj*> objects );

    ScreenMgr* m_ScreenMgr;
    int m_mouse_x;
    int m_mouse_y;
    bool m_noRotate; // Flag to enable/disable rotations

};

class EditXSecWindow : public VspGlWindow
{
public:

    EditXSecWindow( int x, int y, int w, int h, ScreenMgr* mgr );

    virtual int handle( int fl_event );

    virtual void update();

    void InitZoom(); // centers pan values and resets scale for zoom

private:

    // Search for a point index within radius 'r_test' to the input pick location
    int ihit( const vec3d & mpt, double r_test );

    // Convert pixel in XSec editor to equivalent coordinate
    vec3d PixelToCoord( int x_pix, int y_pix );

    vector < vec3d > proj_pt_vec( const vector < vec3d > & pt_in );

    int m_LastHit; // Index of the previously selected point

};
}

#endif