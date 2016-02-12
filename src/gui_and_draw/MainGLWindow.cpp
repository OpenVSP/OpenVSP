#include "MainGLWindow.h"
#include "VehicleMgr.h"
#include "Vehicle.h"
#include "ScreenMgr.h"
#include "CfdMeshScreen.h"
#include "Display.h"
#include "Scene.h"
#include "Viewport.h"
#include "LayoutMgr.h"
#include "Camera.h"
#include "SceneObject.h"
#include "Renderable.h"
#include "Pickable.h"
#include "PickablePnts.h"
#include "Lighting.h"
#include "Image.h"
#include "Texture2D.h"
#include "TextureMgr.h"
#include "Entity.h"
#include "Ruler.h"
#include "GraphicEngine.h"
#include "ScreenMgr.h"
#include "ManageLabelScreen.h"
#include "ManageLightingScreen.h"
#include "ManageGeomScreen.h"
#include "ManageCORScreen.h"
#include "FitModelScreen.h"
#include "Common.h"
#include "GraphicSingletons.h"
#include "Selectable.h"
#include "SelectedPnt.h"
#include "SelectedLoc.h"
#include "Material.h"
#include "ClippingScreen.h"
#include "Clipping.h"
#include "BndBox.h"
#include "ManageViewScreen.h"

#define PRECISION_PAN_SPEED 0.005f
#define PAN_SPEED 0.025f
#define PRECISION_ZOOM_SPEED 0.00005f
#define ZOOM_SPEED 0.00025f

#pragma warning(disable:4244)

using namespace VSPGraphic;

namespace VSPGUI
{
VspGlWindow::VspGlWindow( int x, int y, int w, int h, ScreenMgr * mgr, DrawObj::ScreenEnum drawObjScreen )
    : Fl_Gl_Window( x, y, w, h, "VSP GL Window" )
{
    mode( FL_RGB | FL_ALPHA | FL_DEPTH | FL_DOUBLE | FL_MULTISAMPLE );
    m_GEngine = new VSPGraphic::GraphicEngine();

    m_ScreenMgr = mgr;

    // Link this GUI to one of drawObj screen.
    m_LinkedScreen = drawObjScreen;

    // One Screen
    m_GEngine->getDisplay()->setDisplayLayout( 1, 1 );
    m_GEngine->getDisplay()->selectViewport( 0 );

    m_LightAmb = m_LightSpec = m_LightDiff = 0.5f;
    m_mouse_x = m_mouse_y = 0xFFFFFFF;

    m_initialized = false;

    m_prevLB = m_prevRB = m_prevMB = glm::vec2( 0xFFFFFFFF );
    m_prevAltLB = m_prevCtrlLB = m_prevMetaLB = glm::vec2( 0xFFFFFFFF );
    m_prevLBRB = glm::vec2( 0xFFFFFFFF );

    m_ScreenMgr = mgr;

    m_hasSetSize = false;

#ifdef __APPLE__
#if FL_API_VERSION >= 10304
    Fl::use_high_res_GL( true );
#endif
#endif
}
VspGlWindow::~VspGlWindow()
{
    delete m_GEngine;
}

void VspGlWindow::setWindowLayout( int row, int column )
{
    m_GEngine->getDisplay()->setDisplayLayout( row, column );
    m_GEngine->getDisplay()->resize( pixel_w(), pixel_h() );
    m_GEngine->getDisplay()->selectViewport( 0 );
}

void VspGlWindow::setView( VSPGraphic::Common::VSPenum type )
{
    m_GEngine->getDisplay()->changeView( type );
}

void VspGlWindow::relativePan( float x, float y )
{
    m_GEngine->getDisplay()->relativePan( x, y );
}

void VspGlWindow::setCOR( glm::vec3 center )
{
    m_GEngine->getDisplay()->setCOR( -center.x, -center.y, -center.z );
    m_GEngine->getDisplay()->center();
}

glm::vec3 VspGlWindow::getCOR()
{
    return m_GEngine->getDisplay()->getCOR() * -1.0f;
}

glm::vec2 VspGlWindow::getPanValues()
{
    return m_GEngine->getDisplay()->getPanValues();
}

void VspGlWindow::zoom( int delta, bool precisionOn )
{
    float zoomvalue = 0;

    if( delta != 0 )
    {
        if( precisionOn )
        {
            zoomvalue = PRECISION_ZOOM_SPEED * ( delta < 0 ? -1 : 1 );
        }
        else
        {
            zoomvalue = ZOOM_SPEED * ( delta < 0 ? -1 : 1 );
        }
    }
    m_GEngine->getDisplay()->zoom( zoomvalue );
}

void VspGlWindow::relativeZoom( float zoomValue )
{
    m_GEngine->getDisplay()->relativeZoom( zoomValue );
}

float VspGlWindow::getZoomValue()
{
    return m_GEngine->getDisplay()->getZoomValue();
}

float VspGlWindow::getRelativeZoomValue()
{
    return m_GEngine->getDisplay()->getRelativeZoomValue();
}

void VspGlWindow::rotateSphere( float angleX, float angleY, float angleZ )
{
    m_GEngine->getDisplay()->rotateSphere( angleX, angleY, angleZ );
}

glm::vec3 VspGlWindow::getRotationEulerAngles()
{
    return m_GEngine->getDisplay()->getRotationEulerAngles();
}

void VspGlWindow::resetView()
{
    m_GEngine->getDisplay()->resetView();
}

void VspGlWindow::draw()
{
    // Initialize Glew when context is created.
    _initGLEW();

    if ( !valid() )
    {
        //===== Quick hack to set default opengl screen size (only run once) =====//
        if ( !m_hasSetSize )
        {
            m_GEngine->getDisplay()->setDefaultScreenSize( pixel_w(), pixel_h() );
            m_hasSetSize = true;
        }

        m_GEngine->getDisplay()->resize( pixel_w(), pixel_h() );

        ManageViewScreen * viewScreen = dynamic_cast< ManageViewScreen* >
        ( m_ScreenMgr->GetScreen( ScreenMgr::VSP_VIEW_SCREEN ) );

        if ( viewScreen->IsShown() )
        {
            viewScreen->UpdateViewport();
        }

        //Make sure the current width and height update
        m_ScreenMgr->GetScreen( ScreenMgr::VSP_SCREENSHOT_SCREEN )->Update();
    }

    m_GEngine->draw( m_mouse_x, m_mouse_y );
}

int VspGlWindow::handle( int fl_event )
{
    // Move origin from upper left corner to lower left corner.
    int x = Fl::event_x();
    int y = h() - Fl::event_y();

#ifdef __APPLE__
    if ( shown() )
    {
        double factor = pixel_w()/w();
        x *= factor;
        y *= factor;
    }
#endif

    m_mouse_x = x;
    m_mouse_y = y;

    switch( fl_event )
    {
    case FL_ENTER:
        cursor( FL_CURSOR_CROSS );
        return 1;

    case FL_LEAVE:
        cursor( FL_CURSOR_DEFAULT );
        return 1;

    case FL_PUSH:
        OnPush( x, y );
        return 1;

    case FL_DRAG:
        OnDrag( x, y );
        return 1;

    case FL_RELEASE:
        OnRelease( x, y );
        return 1;

    case FL_MOUSEWHEEL:
        return 1;

    case FL_MOVE:
        if( m_GEngine->getScene()->isPickingEnabled() )
        {
            redraw();
        }
        return 1;

    case FL_KEYDOWN:
        return OnKeydown();

    case FL_KEYUP:
        return OnKeyup( x, y );

    case FL_FOCUS:
        return 1;

    case FL_UNFOCUS:
        return 1;
    }
    return Fl_Gl_Window::handle( fl_event );
}

void VspGlWindow::update()
{
    Vehicle* vPtr = VehicleMgr.GetVehicle();

    if ( vPtr )
    {
        make_current();

        VSPGraphic::Display * display = m_GEngine->getDisplay();
        if ( display )
        {
            VSPGraphic::Camera * camera = display->getCamera();
            if ( camera )
            {
                float zn, zf;

                vPtr->UpdateBBox();
                BndBox bb = vPtr->GetBndBox();

                float d = std::max( 10.0, bb.GetLargestDist() );

                vec3d cen = bb.GetCenter();

                zf = 2.0f * ( std::abs( cen[ cen.major_comp() ] ) + d );

                zf = std::max( zf, 2500.0f );

                zn = -zf;

                camera->setZNearFar( zn, zf );
            }
        }

        // Get Render Objects from Vehicle.
        vector<DrawObj *> drawObjs = vPtr->GetDrawObjs();

        // Load Render Objects from fitModelScreen.
        FitModelScreen * fitModelScreen = dynamic_cast< FitModelScreen* >
            ( m_ScreenMgr->GetScreen( ScreenMgr::VSP_FIT_MODEL_SCREEN ) );
        if ( fitModelScreen )
        {
            fitModelScreen->LoadDrawObjs( drawObjs );
        }

        // Load Render Objects from CfdMeshScreen.
        CfdMeshScreen * cfdScreen = dynamic_cast< CfdMeshScreen* >
            ( m_ScreenMgr->GetScreen( ScreenMgr::VSP_CFD_MESH_SCREEN ) );
        if( cfdScreen )
        {
            cfdScreen->LoadDrawObjs( drawObjs );
        }

        // Load Render Objects from labelScreen.
        ManageLabelScreen * labelScreen = dynamic_cast< ManageLabelScreen* >
            ( m_ScreenMgr->GetScreen( ScreenMgr::VSP_LABEL_SCREEN ) );
        if( labelScreen )
        {
            labelScreen->LoadDrawObjs( drawObjs );
        }

        // Load Render Objects from lightScreen.
        ManageLightingScreen * lightScreen = dynamic_cast< ManageLightingScreen* >
            ( m_ScreenMgr->GetScreen( ScreenMgr::VSP_LIGHTING_SCREEN ) );
        if( lightScreen )
        {
            lightScreen->LoadDrawObjs( drawObjs );
        }

        // Load Render Objects from geomScreen.
        ManageGeomScreen * geomScreen = dynamic_cast< ManageGeomScreen* >
            ( m_ScreenMgr->GetScreen( ScreenMgr::VSP_MANAGE_GEOM_SCREEN ) );
        if( geomScreen )
        {
            geomScreen->LoadDrawObjs( drawObjs );
        }

        // Load Render Objects from corScreen ( Center of Rotation ).
        ManageCORScreen * corScreen = dynamic_cast< ManageCORScreen* >
            ( m_ScreenMgr->GetScreen( ScreenMgr::VSP_COR_SCREEN ) );
        if( corScreen )
        {
            corScreen->LoadDrawObjs( drawObjs );
        }

        // Load Render Objects from clipScreen ( Clipping ).
        ClippingScreen * clipScreen = dynamic_cast< ClippingScreen* >
            ( m_ScreenMgr->GetScreen( ScreenMgr::VSP_CLIPPING_SCREEN ) );
        if( clipScreen )
        {
            clipScreen->LoadDrawObjs( drawObjs );
        }

        // Load Objects to Renderer.
        _update( drawObjs );

        // Once updated data is stored in buffer, 
        // reset geometry changed flag to false.
        vPtr->ResetDrawObjsGeomChangedFlags();
    }
}

void VspGlWindow::_initGLEW()
{
    if( !m_initialized )
    {
        VSPGraphic::GraphicEngine::initGlew();

        m_initialized = true;
    }
}

void VspGlWindow::_update( std::vector<DrawObj *> objects )
{
    // Check for changes in DrawObjs and adjust accordingly.
    _updateBuffer( objects );

    // Process all geometry renderable first, then all labels, pickables, lastly the markers.  Order matters.
    for( int i = 0; i < (int)objects.size(); i++ )
    {
        // If this DrawObj is aimed for other screen, ignore.
        if( objects[i]->m_Screen != m_LinkedScreen )
        {
            continue;
        }

        // Load Settings.
        float red, green, blue;
        float lineWidth;

        red = ( float )objects[i]->m_LineColor.x();
        green = ( float )objects[i]->m_LineColor.y();
        blue = ( float )objects[i]->m_LineColor.z();

        lineWidth = ( float )objects[i]->m_LineWidth;

        unsigned int id;
        ID * idPtr = _findID( objects[i]->m_GeomID );
        if( idPtr )
        {
            id = idPtr->bufferID;
        }
        else
        {
            id = 0xFFFFFFFF;
        }

        Renderable * rObj;
        VSPGraphic::Entity * eObj;

        switch( objects[i]->m_Type )
        {
        case DrawObj::VSP_SETTING:
            _setLighting( objects[i] );
            break;

        case DrawObj::VSP_CLIP:
            _setClipping( objects[i] );
            break;

        case DrawObj::VSP_WIRE_MESH:
            // Create new scene object if needed.
            if( id == 0xFFFFFFFF )
            {
                m_GEngine->getScene()->createObject( Common::VSP_OBJECT_ENTITY, &id );

                ID idInfo;
                idInfo.bufferID = id;
                idInfo.geomID = objects[i]->m_GeomID;
                m_ids.push_back( idInfo );
            }
            eObj = dynamic_cast<VSPGraphic::Entity*> ( m_GEngine->getScene()->getObject( id ) );
            if( eObj )
            {
                // Update scene object.
                eObj->setVisibility( objects[i]->m_Visible );
                eObj->setPrimType( Common::VSP_QUADS );
                eObj->setRenderStyle( Common::VSP_DRAW_WIRE_FRAME );
                eObj->setLineColor( red, green, blue );
                eObj->setLineWidth( lineWidth );

                eObj->setMaterial( objects[i]->m_MaterialInfo.Ambient, objects[i]->m_MaterialInfo.Diffuse,
                    objects[i]->m_MaterialInfo.Specular, objects[i]->m_MaterialInfo.Emission,
                    objects[i]->m_MaterialInfo.Shininess );

                if( objects[i]->m_GeomChanged )
                {
                    _loadXSecData( eObj, objects[i] );
                }
            }
            break;

        case DrawObj::VSP_WIRE_TRIS:
            if( id == 0xFFFFFFFF )
            {
                m_GEngine->getScene()->createObject( Common::VSP_OBJECT_ENTITY, &id );

                ID idInfo;
                idInfo.bufferID = id;
                idInfo.geomID = objects[i]->m_GeomID;
                m_ids.push_back( idInfo );
            }
            eObj = dynamic_cast<VSPGraphic::Entity*> ( m_GEngine->getScene()->getObject( id ) );
            if( eObj )
            {
                eObj->setVisibility( objects[i]->m_Visible );
                eObj->setPrimType( Common::VSP_TRIANGLES );
                eObj->setRenderStyle( Common::VSP_DRAW_WIRE_FRAME );
                eObj->setLineColor( red, green, blue );
                eObj->setLineWidth( lineWidth );

                eObj->setMaterial( objects[i]->m_MaterialInfo.Ambient, objects[i]->m_MaterialInfo.Diffuse,
                    objects[i]->m_MaterialInfo.Specular, objects[i]->m_MaterialInfo.Emission,
                    objects[i]->m_MaterialInfo.Shininess );

                if( objects[i]->m_GeomChanged )
                {
                    _loadTrisData( eObj, objects[i] );
                }
            }
            break;

        case DrawObj::VSP_HIDDEN_MESH:
            if( id == 0xFFFFFFFF )
            {
                m_GEngine->getScene()->createObject( Common::VSP_OBJECT_ENTITY, &id );

                ID idInfo;
                idInfo.bufferID = id;
                idInfo.geomID = objects[i]->m_GeomID;
                m_ids.push_back( idInfo );
            }
            eObj = dynamic_cast<VSPGraphic::Entity*> ( m_GEngine->getScene()->getObject( id ) );
            if( eObj )
            {
                eObj->setVisibility( objects[i]->m_Visible );
                eObj->setPrimType( Common::VSP_QUADS );
                eObj->setRenderStyle( Common::VSP_DRAW_WIRE_FRAME_SOLID );
                eObj->setLineColor( red, green, blue );
                eObj->setLineWidth( lineWidth );

                eObj->setMaterial( objects[i]->m_MaterialInfo.Ambient, objects[i]->m_MaterialInfo.Diffuse,
                    objects[i]->m_MaterialInfo.Specular, objects[i]->m_MaterialInfo.Emission,
                    objects[i]->m_MaterialInfo.Shininess );

                if( objects[i]->m_GeomChanged )
                {
                    _loadXSecData( eObj, objects[i] );
                }
            }
            break;

        case DrawObj::VSP_HIDDEN_TRIS:
            if( id == 0xFFFFFFFF )
            {
                m_GEngine->getScene()->createObject( Common::VSP_OBJECT_ENTITY, &id );

                ID idInfo;
                idInfo.bufferID = id;
                idInfo.geomID = objects[i]->m_GeomID;
                m_ids.push_back( idInfo );
            }
            eObj = dynamic_cast<VSPGraphic::Entity*> ( m_GEngine->getScene()->getObject( id ) );
            if( eObj )
            {
                eObj->setVisibility( objects[i]->m_Visible );
                eObj->setPrimType( Common::VSP_TRIANGLES );
                eObj->setRenderStyle( Common::VSP_DRAW_WIRE_FRAME_SOLID );
                eObj->setLineColor( red, green, blue );
                eObj->setLineWidth( lineWidth );

                eObj->setMaterial( objects[i]->m_MaterialInfo.Ambient, objects[i]->m_MaterialInfo.Diffuse,
                    objects[i]->m_MaterialInfo.Specular, objects[i]->m_MaterialInfo.Emission,
                    objects[i]->m_MaterialInfo.Shininess );

                if( objects[i]->m_GeomChanged )
                {
                    _loadTrisData( eObj, objects[i] );
                }
            }
            break;

        case DrawObj::VSP_HIDDEN_TRIS_CFD:
            if( id == 0xFFFFFFFF )
            {
                m_GEngine->getScene()->createObject( Common::VSP_OBJECT_CFD_ENTITY, &id );

                ID idInfo;
                idInfo.bufferID = id;
                idInfo.geomID = objects[i]->m_GeomID;
                m_ids.push_back( idInfo );
            }
            eObj = dynamic_cast<VSPGraphic::Entity*> ( m_GEngine->getScene()->getObject( id ) );
            if( eObj )
            {
                eObj->setVisibility( objects[i]->m_Visible );
                eObj->setPrimType( Common::VSP_TRIANGLES );
                eObj->setRenderStyle( Common::VSP_DRAW_WIRE_FRAME_TRANSPARENT_BACK );
                eObj->setLineColor( red, green, blue );
                eObj->setLineWidth( lineWidth );

                eObj->setMaterial( objects[i]->m_MaterialInfo.Ambient, objects[i]->m_MaterialInfo.Diffuse,
                    objects[i]->m_MaterialInfo.Specular, objects[i]->m_MaterialInfo.Emission,
                    objects[i]->m_MaterialInfo.Shininess );

                if( objects[i]->m_GeomChanged )
                {
                    _loadTrisData( eObj, objects[i] );
                }
            }
            break;

        case DrawObj::VSP_SHADED_MESH:
            if( id == 0xFFFFFFFF )
            {
                m_GEngine->getScene()->createObject( Common::VSP_OBJECT_ENTITY, &id );

                ID idInfo;
                idInfo.bufferID = id;
                idInfo.geomID = objects[i]->m_GeomID;
                m_ids.push_back( idInfo );
            }
            eObj = dynamic_cast<VSPGraphic::Entity*> ( m_GEngine->getScene()->getObject( id ) );
            if( eObj )
            {
                eObj->setVisibility( objects[i]->m_Visible );
                eObj->setPrimType( Common::VSP_QUADS );
                eObj->setRenderStyle( Common::VSP_DRAW_MESH_SHADED );

                eObj->setMaterial( objects[i]->m_MaterialInfo.Ambient, objects[i]->m_MaterialInfo.Diffuse,
                    objects[i]->m_MaterialInfo.Specular, objects[i]->m_MaterialInfo.Emission,
                    objects[i]->m_MaterialInfo.Shininess );

                if( objects[i]->m_GeomChanged )
                {
                    _loadXSecData( eObj, objects[i] );
                }
            }
            break;

        case DrawObj::VSP_SHADED_TRIS:
            if( id == 0xFFFFFFFF )
            {
                m_GEngine->getScene()->createObject( Common::VSP_OBJECT_ENTITY, &id );

                ID idInfo;
                idInfo.bufferID = id;
                idInfo.geomID = objects[i]->m_GeomID;
                m_ids.push_back( idInfo );
            }
            eObj = dynamic_cast<VSPGraphic::Entity*> ( m_GEngine->getScene()->getObject( id ) );
            if( eObj )
            {
                eObj->setVisibility( objects[i]->m_Visible );
                eObj->setPrimType( Common::VSP_TRIANGLES );
                eObj->setRenderStyle( Common::VSP_DRAW_MESH_SHADED );

                eObj->setMaterial( objects[i]->m_MaterialInfo.Ambient, objects[i]->m_MaterialInfo.Diffuse,
                    objects[i]->m_MaterialInfo.Specular, objects[i]->m_MaterialInfo.Emission,
                    objects[i]->m_MaterialInfo.Shininess );

                if( objects[i]->m_GeomChanged )
                {
                    _loadTrisData( eObj, objects[i] );
                }
            }
            break;

        case DrawObj::VSP_TEXTURED_MESH:
            if( id == 0xFFFFFFFF )
            {
                m_GEngine->getScene()->createObject( Common::VSP_OBJECT_ENTITY, &id );

                ID idInfo;
                idInfo.bufferID = id;
                idInfo.geomID = objects[i]->m_GeomID;
                m_ids.push_back( idInfo );
            }
            eObj = dynamic_cast<VSPGraphic::Entity*> ( m_GEngine->getScene()->getObject( id ) );
            if( eObj )
            {
                eObj->setVisibility( objects[i]->m_Visible );
                eObj->setPrimType( Common::VSP_QUADS );
                eObj->setRenderStyle( Common::VSP_DRAW_MESH_TEXTURED );

                eObj->setMaterial( objects[i]->m_MaterialInfo.Ambient, objects[i]->m_MaterialInfo.Diffuse,
                    objects[i]->m_MaterialInfo.Specular, objects[i]->m_MaterialInfo.Emission,
                    objects[i]->m_MaterialInfo.Shininess );

                if( objects[i]->m_GeomChanged )
                {
                    _loadXSecData( eObj, objects[i] );
                }
                _updateTextures( objects[i] );
            }
            break;

        default:
            break;
        }
    }

    // Process Labels.  Order Matters.
    for( int i = 0; i < ( int )objects.size(); i++ )
    {
        // If this DrawObj is aimed for other screen, ignore.
        if( objects[i]->m_Screen != m_LinkedScreen )
        {
            continue;
        }

        unsigned int id;
        ID * idPtr = _findID( objects[i]->m_GeomID );
        if( idPtr )
        {
            id = idPtr->bufferID;
        }
        else
        {
            id = 0xFFFFFFFF;
        }

        VSPGraphic::Ruler * ruler;

        glm::vec3 start, end, offset;

        switch( objects[i]->m_Type )
        {
        case DrawObj::VSP_RULER:
            if( id == 0xFFFFFFFF )
            {
                m_GEngine->getScene()->createObject( Common::VSP_OBJECT_RULER, &id );

                ID idInfo;
                idInfo.bufferID = id;
                idInfo.geomID = objects[i]->m_GeomID;
                m_ids.push_back( idInfo );
            }
            ruler = dynamic_cast<VSPGraphic::Ruler*> ( m_GEngine->getScene()->getObject( id ) );
            if( ruler )
            {
                ruler->setTextColor( (float)objects[i]->m_TextColor.x(),
                    (float)objects[i]->m_TextColor.y(),
                    (float)objects[i]->m_TextColor.z() );

                ruler->setTextSize( (float)objects[i]->m_TextSize );

                switch( objects[i]->m_Ruler.Step )
                {
                case DrawObj::VSP_RULER_STEP_ZERO:
                    ruler->reset();
                    break;

                case DrawObj::VSP_RULER_STEP_ONE:
                    start = glm::vec3( objects[i]->m_Ruler.Start.x(),
                        objects[i]->m_Ruler.Start.y(),
                        objects[i]->m_Ruler.Start.z() );
                    ruler->placeRuler( start );
                    break;

                case DrawObj::VSP_RULER_STEP_TWO:
                    start = glm::vec3( objects[i]->m_Ruler.Start.x(),
                        objects[i]->m_Ruler.Start.y(),
                        objects[i]->m_Ruler.Start.z() );
                    end = glm::vec3( objects[i]->m_Ruler.End.x(),
                        objects[i]->m_Ruler.End.y(),
                        objects[i]->m_Ruler.End.z() );
                    ruler->placeRuler( start, end );
                    break;

                case DrawObj::VSP_RULER_STEP_COMPLETE:
                    start = glm::vec3( objects[i]->m_Ruler.Start.x(),
                        objects[i]->m_Ruler.Start.y(),
                        objects[i]->m_Ruler.Start.z() );
                    end = glm::vec3( objects[i]->m_Ruler.End.x(),
                        objects[i]->m_Ruler.End.y(),
                        objects[i]->m_Ruler.End.z() );
                    offset = glm::vec3( objects[i]->m_Ruler.Offset.x(),
                        objects[i]->m_Ruler.Offset.y(),
                        objects[i]->m_Ruler.Offset.z() );
                    ruler->placeRuler( start, end, offset );
                    break;

                default:
                    break;
                }
            }
            break;
 
        default:
            break;
       }
    }

    // Now process all pickables.  Order matters.

    // Reset flag to show all selections.  This prevents selections from permanently hidden
    // due to misuse of VSP_PICK_VERTEX_HIDE_SELECTION object.
    m_GEngine->getScene()->showSelection();

    for( int i = 0; i < ( int )objects.size(); i++ )
    {
        // If this DrawObj is aimed for other screen, ignore.
        if( objects[i]->m_Screen != m_LinkedScreen )
        {
            continue;
        }

        unsigned int id;
        ID * idPtr = _findID( objects[i]->m_GeomID );
        if( idPtr )
        {
            id = idPtr->bufferID;
        }
        else
        {
            id = 0xFFFFFFFF;
        }

        Pickable * pObj;
        PickablePnts * ppntObj;

        switch( objects[i]->m_Type )
        {
        case DrawObj::VSP_PICK_GEOM:
            if( id == 0xFFFFFFFF )
            {
                ID * sourceId = _findID( objects[i]->m_PickSourceID );
                if( sourceId )
                {
                    m_GEngine->getScene()->createObject( Common::VSP_OBJECT_PICK_GEOM, &id, sourceId->bufferID );

                    ID idInfo;
                    idInfo.bufferID = id;
                    idInfo.geomID = objects[i]->m_GeomID;
                    m_ids.push_back( idInfo );
                }
            }
            pObj = dynamic_cast<Pickable*> ( m_GEngine->getScene()->getObject( id ) );
            if( pObj )
            {
                pObj->setVisibility( objects[i]->m_Visible );
                pObj->setGroup( objects[i]->m_FeedbackGroup );
                pObj->update();
            }
            break;

        case DrawObj::VSP_PICK_VERTEX:
            if( id == 0xFFFFFFFF )
            {
                ID * sourceId = _findID( objects[i]->m_PickSourceID );
                if(sourceId)
                {
                    m_GEngine->getScene()->createObject( Common::VSP_OBJECT_PICK_VERTEX, &id, sourceId->bufferID );

                    ID idInfo;
                    idInfo.bufferID = id;
                    idInfo.geomID = objects[i]->m_GeomID;
                    m_ids.push_back( idInfo );
                }
            }
            ppntObj = dynamic_cast<PickablePnts*> ( m_GEngine->getScene()->getObject( id ) );
            if( ppntObj )
            {
                ppntObj->setVisibility( objects[i]->m_Visible );
                ppntObj->setGroup( objects[i]->m_FeedbackGroup );
                ppntObj->setPointSize( objects[i]->m_PointSize );
                ppntObj->update();
            }
            break;

        case DrawObj::VSP_PICK_LOCATION:
            if( id == 0xFFFFFFFF )
            {
                m_GEngine->getScene()->createObject( Common::VSP_OBJECT_PICK_LOCATION, &id, 0 );

                ID idInfo;
                idInfo.bufferID = id;
                idInfo.geomID = objects[i]->m_GeomID;
                m_ids.push_back( idInfo );
            }
            pObj = dynamic_cast<Pickable*> ( m_GEngine->getScene()->getObject( id ) );
            if( pObj )
            {
                pObj->setVisibility( objects[i]->m_Visible );
                pObj->setGroup( objects[i]->m_FeedbackGroup );
                pObj->update();
            }
            break;

        default:
            break;
        }
    }

    // Now process all markers.  Order matters.
    for( int i = 0; i < ( int )objects.size(); i++ )
    {
        // If this DrawObj is aimed for other screen, ignore.
        if( objects[i]->m_Screen != m_LinkedScreen )
        {
            continue;
        }

        // Load Settings.
        float red, green, blue;
        float lineWidth;

        red = ( float )objects[i]->m_LineColor.x();
        green = ( float )objects[i]->m_LineColor.y();
        blue = ( float )objects[i]->m_LineColor.z();

        lineWidth = ( float )objects[i]->m_LineWidth;

        unsigned int id;
        ID * idPtr = _findID( objects[i]->m_GeomID );
        if( idPtr )
        {
            id = idPtr->bufferID;
        }
        else
        {
            id = 0xFFFFFFFF;
        }

        Renderable * rObj;

        switch( objects[i]->m_Type )
        {
        case DrawObj::VSP_POINTS:
            if( id == 0xFFFFFFFF )
            {
                m_GEngine->getScene()->createObject( Common::VSP_OBJECT_MARKER, &id );

                ID idInfo;
                idInfo.bufferID = id;
                idInfo.geomID = objects[i]->m_GeomID;
                m_ids.push_back( idInfo );
            }

            rObj = dynamic_cast<Renderable*> ( m_GEngine->getScene()->getObject( id ) );
            if( rObj )
            {
                rObj->setVisibility( objects[i]->m_Visible );
                rObj->setPrimType( VSPGraphic::Common::VSP_POINTS );
                rObj->setPointColor( objects[i]->m_PointColor.x(),
                    objects[i]->m_PointColor.y(), objects[i]->m_PointColor.z() );
                rObj->setPointSize( objects[i]->m_PointSize );

                // Update buffer data if needed.
                if( objects[i]->m_GeomChanged )
                {
                    _loadMarkData( rObj, objects[i] );
                }
            }
            break;

        case DrawObj::VSP_LINES:
            if( id == 0xFFFFFFFF )
            {
                m_GEngine->getScene()->createObject( Common::VSP_OBJECT_MARKER, &id );

                ID idInfo;
                idInfo.bufferID = id;
                idInfo.geomID = objects[i]->m_GeomID;
                m_ids.push_back( idInfo );
            }

            rObj = dynamic_cast<Renderable*> ( m_GEngine->getScene()->getObject( id ) );
            if( rObj )
            {
                rObj->setVisibility( objects[i]->m_Visible );
                rObj->setPrimType( VSPGraphic::Common::VSP_LINES );
                rObj->setLineColor( red, green, blue );
                rObj->setLineWidth( lineWidth );

                if( objects[i]->m_GeomChanged )
                {
                    _loadMarkData( rObj, objects[i] );
                }
            }
            break;

        case DrawObj::VSP_LINE_LOOP:
            if( id == 0xFFFFFFFF )
            {
                m_GEngine->getScene()->createObject( Common::VSP_OBJECT_MARKER, &id );

                ID idInfo;
                idInfo.bufferID = id;
                idInfo.geomID = objects[i]->m_GeomID;
                m_ids.push_back( idInfo );
            }
            rObj = dynamic_cast<Renderable*> ( m_GEngine->getScene()->getObject( id ) );
            if( rObj )
            {
                rObj->setVisibility( objects[i]->m_Visible );
                rObj->setPrimType( VSPGraphic::Common::VSP_LINE_LOOP );
                rObj->setLineColor( red, green, blue );
                rObj->setLineWidth( lineWidth );

                if( objects[i]->m_GeomChanged )
                {
                    _loadMarkData( rObj, objects[i] );
                }
            }
            break;

        case DrawObj::VSP_LINE_STRIP:
            if( id == 0xFFFFFFFF )
            {
                m_GEngine->getScene()->createObject( Common::VSP_OBJECT_MARKER, &id );

                ID idInfo;
                idInfo.bufferID = id;
                idInfo.geomID = objects[i]->m_GeomID;
                m_ids.push_back( idInfo );
            }
            rObj = dynamic_cast<Renderable*> ( m_GEngine->getScene()->getObject( id ) );
            if( rObj )
            {
                rObj->setVisibility( objects[i]->m_Visible );
                rObj->setPrimType( VSPGraphic::Common::VSP_LINE_STRIP );
                rObj->setLineColor( red, green, blue );
                rObj->setLineWidth( lineWidth );

                if( objects[i]->m_GeomChanged )
                {
                    _loadMarkData( rObj, objects[i] );
                }
            }
            break;

        default:
            break;
        }
    }
}

VspGlWindow::ID * VspGlWindow::_findID( std::string geomID )
{
    for( int i = 0; i < ( int )m_ids.size(); i++ )
    {
        if( m_ids[i].geomID == geomID )
        {
            return &m_ids[i];
        }
    }
    return NULL;
}

VspGlWindow::ID * VspGlWindow::_findID( unsigned int bufferID )
{
    for( int i = 0; i < ( int )m_ids.size(); i++ )
    {
        if( m_ids[i].bufferID == bufferID )
        {
            return &m_ids[i];
        }
    }
    return NULL;
}

void VspGlWindow::_updateBuffer( std::vector<DrawObj *> objects )
{
    std::vector<ID> idsToRemove;
    std::vector<ID> idsToKeep;

    // Figure out if any buffer object no longer exists in DrawObjs.
    for( int i = 0; i < ( int )m_ids.size(); i++ )
    {
        bool exist = false;
        for( int j = 0; j < ( int )objects.size(); j++ )
        {
            if( m_ids[i].geomID == objects[j]->m_GeomID && m_ids[i].geomID != std::string( "Default" ) )
            {
                idsToKeep.push_back( m_ids[i] );
                exist = true;
                break;
            }
        }
        if( !exist )
        {
            idsToRemove.push_back( m_ids[i] );
        }
    }

    // Remove unnecessary buffers.
    for( int i = 0; i < ( int )idsToRemove.size(); i++ )
    {
        m_GEngine->getScene()->removeObject( idsToRemove[i].bufferID );
    }

    // Update m_ids.
    m_ids.clear();
    for( int i = 0; i < ( int )idsToKeep.size(); i++ )
    {
        m_ids.push_back( idsToKeep[i] );
    }
}

void VspGlWindow::_updateTextures( DrawObj * drawObj )
{
    VspGlWindow::ID * id = _findID( drawObj->m_GeomID );
    if( !id )
    {
        // Can't find Object that is linked to this drawObj
        assert( false );
        return;
    }

    Renderable * renderable = dynamic_cast<Renderable*>( m_GEngine->getScene()->getObject( id->bufferID ) );
    if( renderable )
    {
        VSPGraphic::Entity * entity = dynamic_cast<VSPGraphic::Entity *>( renderable );
        if( entity )
        {
            // Check if any textureID no long exists in drawObj.
            // Remove Deleted Textures.

            std::vector<TextureID> texToRemove;
            std::vector<TextureID> texToKeep;

            for( int i = 0; i < ( int )id->textureIDs.size(); i++ )
            {
                bool exist = false;
                for( int j = 0; j < ( int )drawObj->m_TextureInfos.size(); j++ )
                {
                    if( id->textureIDs[i].geomTexID == drawObj->m_TextureInfos[j].ID )
                    {
                        texToKeep.push_back( id->textureIDs[i] );
                        exist = true;
                        break;
                    }
                }
                if( !exist )
                {
                    texToRemove.push_back( id->textureIDs[i] );
                }
            }

            for(int i = 0; i < ( int )texToRemove.size(); i++)
            {
                entity->getTextureMgr()->remove( texToRemove[i].bufferTexID );
            }

            id->textureIDs.clear();
            for( int i = 0; i < ( int )texToKeep.size(); i++ )
            {
                id->textureIDs.push_back( texToKeep[i] );
            }

            // Update / add textures.

            std::vector<DrawObj::TextureInfo> drawObjTexList = drawObj->m_TextureInfos;
            for( int i = 0; i < ( int )drawObjTexList.size(); i++ )
            {
                unsigned int texBufferID;

                TextureID * texID = id->find( drawObjTexList[i].ID );
                if( texID )
                {
                    texBufferID = texID->bufferTexID;
                }
                else
                {
                    // New texture.
                    TextureID newTex;
                    newTex.geomTexID = drawObjTexList[i].ID;
                    newTex.bufferTexID = entity->getTextureMgr()
                        ->add( VSPGraphic::GlobalTextureRepo()->get2DTexture(drawObjTexList[i].FileName.c_str() ) );
                    id->textureIDs.push_back( newTex );

                    texBufferID = newTex.bufferTexID;
                }

                float UScale = ( float )( 1 / drawObjTexList[i].UScale );
                float WScale = ( float )( 1 / drawObjTexList[i].WScale );
                float U = ( float )( drawObjTexList[i].U * -1.f * UScale );
                float W = ( float )( drawObjTexList[i].W * WScale );

                // Update Texture Properities.
                entity->getTextureMgr()->translate( texBufferID, U, W );
                entity->getTextureMgr()->scale( texBufferID, UScale, WScale );
                entity->getTextureMgr()->transparentize( texBufferID, drawObjTexList[i].Transparency );
                entity->getTextureMgr()->flipCoord( texBufferID, drawObjTexList[i].UFlip, drawObjTexList[i].WFlip );
            }
        }
        else
        {
            // Render Object is not type of Textureable Entity.
            assert( false );
        }
    }
    else
    {
        // Can't find Render Object.
        assert( false );
    }
}

void VspGlWindow::_loadXSecData( Renderable * destObj, DrawObj * drawObj )
{
    std::vector<float> vdata;
    std::vector<unsigned int> edata;

    // Generate Texture Coordinate.
//    std::vector<std::vector<vec3d>> textureCoords = _generateTexCoordFromXSec( drawObj );

    int num_mesh = drawObj->m_PntMesh.size();

    int vtotal = 0;
    int etotal = 0;

    for ( int k = 0; k < num_mesh; k++ )
    {
        int num_pnts = drawObj->m_PntMesh[k].size();
        int num_xsecs = 0;
        if ( num_pnts )
            num_xsecs = drawObj->m_PntMesh[k][0].size();

        vtotal += num_xsecs * num_pnts;

        etotal += (num_pnts - 1) * (num_xsecs - 1);
    }

    vdata.reserve( vtotal * 8 );
    edata.reserve( etotal * 4 );

    int offset = 0;

    for ( int k = 0; k < num_mesh; k++ )
    {
        int increment = 0;

        int num_pnts = drawObj->m_PntMesh[k].size();
        int num_xsecs = 0;
        if ( num_pnts )
            num_xsecs = drawObj->m_PntMesh[k][0].size();

        // Vertex Buffer.
        for ( int i = 0 ; i < num_pnts ; i++ )
        {
            for ( int j = 0 ; j < num_xsecs ; j++ )
            {
                vdata.push_back( (float)drawObj->m_PntMesh[k][i][j].x() );
                vdata.push_back( (float)drawObj->m_PntMesh[k][i][j].y() );
                vdata.push_back( (float)drawObj->m_PntMesh[k][i][j].z() );

                vdata.push_back( (float)drawObj->m_NormMesh[k][i][j].x() );
                vdata.push_back( (float)drawObj->m_NormMesh[k][i][j].y() );
                vdata.push_back( (float)drawObj->m_NormMesh[k][i][j].z() );

                vdata.push_back( (float)drawObj->m_uTexMesh[k][i][j] );
                vdata.push_back( (float)drawObj->m_vTexMesh[k][i][j] );
                increment++;
            }
        }


        // Element Buffer.
        for( int i = 0; i < num_pnts - 1; i++ )
        {
            for( int j = 0; j < num_xsecs - 1; j++ )
            {
                edata.push_back( offset + i * num_xsecs + j );
                edata.push_back( offset + ( i + 1 ) * num_xsecs + j );
                edata.push_back( offset + ( i + 1 ) * num_xsecs + j + 1 );
                edata.push_back( offset + i * num_xsecs + j + 1 );
            }
        }

        offset += increment;

    }

    destObj->setFacingCW( drawObj->m_FlipNormals );

    destObj->emptyVBuffer();
    destObj->appendVBuffer( vdata.data(), sizeof(float) * vdata.size() );

    destObj->emptyEBuffer();
    destObj->appendEBuffer( edata.data(), sizeof( unsigned int ) * edata.size() );
    destObj->enableEBuffer( true );

}

void VspGlWindow::_loadTrisData( Renderable * destObj, DrawObj * drawObj )
{
    assert( drawObj->m_PntVec.size() == drawObj->m_NormVec.size() );

    int n = drawObj->m_PntVec.size();

    std::vector<float> data( n * 8, 0.0f );

    for ( int i = 0; i < ( int )drawObj->m_PntVec.size(); i++ )
    {
        const int j = i * 8;
        data[ j + 0 ] = ( float )drawObj->m_PntVec[i].x();
        data[ j + 1 ] = ( float )drawObj->m_PntVec[i].y();
        data[ j + 2 ] = ( float )drawObj->m_PntVec[i].z();

        data[ j + 3 ] = ( float )drawObj->m_NormVec[i].x();
        data[ j + 4 ] = ( float )drawObj->m_NormVec[i].y();
        data[ j + 5 ] = ( float )drawObj->m_NormVec[i].z();
    }
    destObj->setFacingCW( drawObj->m_FlipNormals );

    destObj->emptyVBuffer();
    destObj->appendVBuffer( data.data(), sizeof( float ) * data.size() );
}

void VspGlWindow::_loadMarkData( Renderable * destObj, DrawObj * drawObj )
{
    int n = drawObj->m_PntVec.size();
    std::vector<float> data( n * 8, 0.0f );

    for ( int i = 0; i < n; i++ )
    {
        const int j = i * 8;
        data[ j + 0 ] = ( float )drawObj->m_PntVec[i].x();
        data[ j + 1 ] = ( float )drawObj->m_PntVec[i].y();
        data[ j + 2 ] = ( float )drawObj->m_PntVec[i].z();
    }
    destObj->emptyVBuffer();
    destObj->appendVBuffer(data.data(), sizeof( float ) * data.size());
}

void VspGlWindow::_setLighting( DrawObj * drawObj )
{
    if( drawObj->m_Type != DrawObj::VSP_SETTING )
        return;

    // Currently only support up to eight light sources.
    assert( drawObj->m_LightingInfos.size() <= 8 );
    if( drawObj->m_LightingInfos.size() > 8 )
        return;

    DrawObj::LightSourceInfo lInfo;

    LightSource * lSource;

    Lighting * lights = m_GEngine->getScene()->getLights();

    for( int i = 0; i < (int)drawObj->m_LightingInfos.size(); i++ )
    {
        lInfo = drawObj->m_LightingInfos[i];

        lSource = lights->getLightSource( i );

        lSource->position( lInfo.X, lInfo.Y, lInfo.Z, 0.0f );
        lSource->ambient( lInfo.Amb, lInfo.Amb, lInfo.Amb, 1.0f );
        lSource->diffuse( lInfo.Diff, lInfo.Diff, lInfo.Diff, 1.0f );
        lSource->specular( lInfo.Spec, lInfo.Spec, lInfo.Spec, 1.0f );

        if( lInfo.Active )
            lSource->enable();
        else
            lSource->disable();
    }
}

void VspGlWindow::_setClipping( DrawObj * drawObj )
{
    if( drawObj->m_Type != DrawObj::VSP_CLIP )
        return;

    Clipping * clip = m_GEngine->getScene()->GetClipping();

    for( int i = 0; i < 6; i++ )
    {
        ClipPlane * cplane = clip->getPlane( i );

        if( drawObj->m_ClipFlag[ i ] )
        {
            cplane->enable();
        }
        else
        {
            cplane->disable();
        }

        vec3d n(0, 0, 0);
        vec3d p(0, 0, 0);

        int jaxis = i / 2;  // Integer division implies floor.

        if( i % 2 == 0 )
        {
            n.v[ jaxis ] = -1.0;
        }
        else
        {
            n.v[ jaxis ] = 1.0;
        }

        p.v[ jaxis ] = drawObj->m_ClipLoc[ i ];

        double e[4] = {n.v[0], n.v[1], n.v[2], -dot( n, p )};

        cplane->setEqn( e );
    }
}

void VspGlWindow::OnPush( int x, int y )
{
    VSPGraphic::Display * display = m_GEngine->getDisplay();
    display->selectViewport( x, y );

    if( Fl::event_button1() && Fl::event_button3() )
    {
        // LB + RB 
        m_prevLBRB = glm::vec2( x, y );
    }
    else if( Fl::event_button1() )
    {
        bool alltrue = false;

        if( Fl::event_shift() || FitModelMgr.m_SelectBoxFlag() )
        {
            FitModelScreen * fitModelScreen = dynamic_cast< FitModelScreen* >
                ( m_ScreenMgr->GetScreen( ScreenMgr::VSP_FIT_MODEL_SCREEN ) );
            if ( fitModelScreen )
            {
                if (fitModelScreen->IsShown() )
                {
                    m_startShiftLB = glm::vec2( x, y );
                    display->getLayoutMgr()->setStartXY( x, y );
                    alltrue = true;
                }
            }
        }

        if( alltrue )
        {
        }
        else if( Fl::event_alt() )
        {
            // Alt + LB
            m_prevAltLB = glm::vec2( x, y );
        }
        else if( Fl::event_ctrl() )
        {
            // Ctrl + LB
            m_prevCtrlLB = glm::vec2( x, y );
        }
        else if( Fl::event_state( FL_META ) )
        {
            // Meta + LB
            m_prevMetaLB = glm::vec2( x, y );
        }
        else
        {
            // LB
            glm::vec3 mouseInWorld = glm::vec3( 0xFFFFFFFF );

            // Getting mouse location in world space.  This is for selectLocation().
            Viewport * vp = m_GEngine->getDisplay()->getViewport();
            if( vp )
            {
                mouseInWorld = vp->screenToWorld( glm::vec2( m_mouse_x, m_mouse_y ) );
            }

            // Select highlighted.
            if( m_GEngine->getScene()->selectHighlight() )
            {
                Selectable * selected = m_GEngine->getScene()->getLastSelected();
                _sendFeedback( selected );
            }
            // Select location in world.
            else if( mouseInWorld != glm::vec3( 0xFFFFFFFF ) && 
                m_GEngine->getScene()->selectLocation( mouseInWorld.x, mouseInWorld.y, mouseInWorld.z ) )
            {
                Selectable * selected = m_GEngine->getScene()->getLastSelected();
                _sendFeedback( selected );
            }
            // Rotation.
            else
            {
                m_prevLB = glm::vec2( x, y );
            }
        }
    }
    else if( Fl::event_button2() )
    {
        // MB
        m_prevMB = glm::vec2( x, y );
    }
    else if( Fl::event_button3() )
    {
        // RB
        m_prevRB = glm::vec2( x, y );
    }
    redraw();
}

void VspGlWindow::OnDrag( int x, int y )
{
    VSPGraphic::Display * display = m_GEngine->getDisplay();

    if( Fl::event_button1() && Fl::event_button3() )
    {
        // LB + RB 
        if( m_prevLBRB != glm::vec2( 0xFFFFFFFF ) )
        {
            display->zoom( ( int )m_prevLBRB.x, ( int )m_prevLBRB.y, x, y );

            ManageViewScreen * viewScreen = dynamic_cast< ManageViewScreen* >
            ( m_ScreenMgr->GetScreen( ScreenMgr::VSP_VIEW_SCREEN ) );

            viewScreen->UpdateZoom();
        }
        m_prevLBRB = glm::vec2( x, y );
    }
    else if( Fl::event_button1() )
    {
        bool alltrue = false;
        if( Fl::event_shift() || FitModelMgr.m_SelectBoxFlag() )
        {
            FitModelScreen * fitModelScreen = dynamic_cast< FitModelScreen* >
                ( m_ScreenMgr->GetScreen( ScreenMgr::VSP_FIT_MODEL_SCREEN ) );
            if ( fitModelScreen )
            {
                if (fitModelScreen->IsShown() )
                {
                    if( m_startShiftLB == glm::vec2( 0xFFFFFFFF ) )
                    {
                        m_startShiftLB = glm::vec2( x, y );
                        display->getLayoutMgr()->setStartXY( x, y );
                    }
                    alltrue = true;
                }
            }
        }

        if( alltrue )
        {
        }
        else if( Fl::event_alt() )
        {
            // Alt + LB
            if( m_prevAltLB != glm::vec2( 0xFFFFFFFF ) )
            {
                display->pan( ( int )m_prevAltLB.x, ( int )m_prevAltLB.y, x, y );

                ManageViewScreen * viewScreen = dynamic_cast< ManageViewScreen* >
                ( m_ScreenMgr->GetScreen( ScreenMgr::VSP_VIEW_SCREEN ) );

                if ( viewScreen->IsShown() )
                {
                    viewScreen->UpdatePan();
                }
            }
            m_prevAltLB = glm::vec2( x, y );
        }
        else if( Fl::event_ctrl() )
        {
            // Ctrl + LB
            if( m_prevCtrlLB != glm::vec2( 0xFFFFFFFF ) )
            {
                display->zoom( ( int )m_prevCtrlLB.x, ( int )m_prevCtrlLB.y, x, y );

                ManageViewScreen * viewScreen = dynamic_cast< ManageViewScreen* >
                ( m_ScreenMgr->GetScreen( ScreenMgr::VSP_VIEW_SCREEN ) );

                viewScreen->UpdateZoom();
            }
            m_prevCtrlLB = glm::vec2( x, y );
        }
        else if( Fl::event_state( FL_META ) )
        {
            if( m_prevMetaLB != glm::vec2( 0xFFFFFFFF ) )
            {
                display->zoom( ( int )m_prevMetaLB.x, ( int )m_prevMetaLB.y, x, y );

                ManageViewScreen * viewScreen = dynamic_cast< ManageViewScreen* >
                ( m_ScreenMgr->GetScreen( ScreenMgr::VSP_VIEW_SCREEN ) );

                viewScreen->UpdateZoom();
            }
            m_prevMetaLB = glm::vec2( x, y );
        }
        else
        {
            // LB
            if( m_prevLB != glm::vec2( 0xFFFFFFFF ) )
            {
                display->rotate( ( int )m_prevLB.x, ( int )m_prevLB.y, x, y );

                ManageViewScreen * viewScreen = dynamic_cast< ManageViewScreen* >
                ( m_ScreenMgr->GetScreen( ScreenMgr::VSP_VIEW_SCREEN ) );

                viewScreen->UpdateRotations();
            }
            m_prevLB = glm::vec2( x, y );
        }
    }
    else if( Fl::event_button2() )
    {
        // MB
        if( m_prevMB != glm::vec2( 0xFFFFFFFF ) )
        {
            display->zoom( ( int )m_prevMB.x, ( int )m_prevMB.y, x, y );

            ManageViewScreen * viewScreen = dynamic_cast< ManageViewScreen* >
            ( m_ScreenMgr->GetScreen( ScreenMgr::VSP_VIEW_SCREEN ) );

            viewScreen->UpdateZoom();
        }
        m_prevMB = glm::vec2( x, y );
    }
    else if( Fl::event_button3() )
    {
        // RB
        if( m_prevRB != glm::vec2( 0xFFFFFFFF ) )
        {
            display->pan( ( int )m_prevRB.x, ( int )m_prevRB.y, x, y );

            ManageViewScreen * viewScreen = dynamic_cast< ManageViewScreen* >
            ( m_ScreenMgr->GetScreen( ScreenMgr::VSP_VIEW_SCREEN ) );

            if ( viewScreen->IsShown() )
            {
                viewScreen->UpdatePan();
            }
        }
        m_prevRB = glm::vec2( x, y );
    }
    redraw();
}

void VspGlWindow::OnRelease( int x, int y )
{
    VSPGraphic::Display * display = m_GEngine->getDisplay();

    // Reset buttons positions.
    switch( Fl::event_button() )
    {
    case FL_LEFT_MOUSE:
        m_prevLB = m_prevAltLB = m_prevCtrlLB = glm::vec2( 0xFFFFFFFF );
        m_prevMetaLB = glm::vec2( 0xFFFFFFFF );
        if( Fl::event_shift() || FitModelMgr.m_SelectBoxFlag() )
        {
            FitModelScreen * fitModelScreen = dynamic_cast< FitModelScreen* >
                ( m_ScreenMgr->GetScreen( ScreenMgr::VSP_FIT_MODEL_SCREEN ) );
            if ( fitModelScreen )
            {
                if (fitModelScreen->IsShown() )
                {

                    if( m_GEngine->getScene()->selectBox() )
                    {
                        _sendFeedback( m_GEngine->getScene()->getSelected() );
                    }
                }
            }
        }
        m_startShiftLB = glm::vec2( 0xFFFFFFFF );
        display->getLayoutMgr()->setStartXY( -1, -1 );

        break;

    case FL_RIGHT_MOUSE:
        m_prevRB = glm::vec2( 0xFFFFFFFF );
        break;

    case FL_MIDDLE_MOUSE:
        m_prevMB = glm::vec2( 0xFFFFFFFF );
        break;
    }
    redraw();
}

int VspGlWindow::OnKeyup( int x, int y )
{
    VSPGraphic::Display * display = m_GEngine->getDisplay();

    ManageViewScreen * viewScreen = dynamic_cast< ManageViewScreen* >
    ( m_ScreenMgr->GetScreen( ScreenMgr::VSP_VIEW_SCREEN ) );

    int handled = 0;

    switch( Fl::event_key() )
    {
    case 0x43:
    case 0x63: //Key 'C'; center "Center of Rotation" on the screen.
    {
        Vehicle* vPtr = VehicleMgr.GetVehicle();
        if ( vPtr )
        {
//            BndBox bbox = vPtr->GetBndBox();
//            vec3d p = bbox.GetCenter();
//            m_GEngine->getDisplay()->setCOR( -p.x(), -p.y(), -p.z() );

            m_GEngine->getDisplay()->relativePan( 0.0f, 0.0f );

            if ( viewScreen->IsShown() )
            {
                viewScreen->UpdateCOR();
                viewScreen->UpdatePan();
            }
        }
        display->center();
        break;
    }
    case 0x46:
    case 0x66: //Key 'F'; Fits bounding box within each viewport.
    {
        Vehicle* vPtr = VehicleMgr.GetVehicle();
        if ( vPtr )
        {
            BndBox bbox = vPtr->GetBndBox();
            vec3d p = bbox.GetCenter();
            m_GEngine->getDisplay()->setCOR( -p.x(), -p.y(), -p.z() );

            m_GEngine->getDisplay()->relativePan( 0.0f, 0.0f );

            // TODO: Now we need to determine how much to zoom in order to fit the entire object on each viewport.


            if ( viewScreen->IsShown() )
            {
                viewScreen->UpdateCOR();
                viewScreen->UpdatePan();
            }
        }
        display->center();
        break;
    }
    case FL_F+1:
        if( Fl::event_state( FL_SHIFT ) )
        {
            display->save( 0 );
        }
        else
        {
            display->load( 0 );

            //===== Update Adjust View Screen with new values =====//
            if ( viewScreen->IsShown() )
            {
                viewScreen->UpdateAll();
            }
        }
        handled = 1;
        break;

    case FL_F+2:
        if( Fl::event_state( FL_SHIFT ) )
        {
            display->save( 1 );
        }
        else
        {
            display->load( 1 );

            //===== Update Adjust View Screen with new values =====//
            if ( viewScreen->IsShown() )
            {
                viewScreen->UpdateAll();
            }
        }
        handled = 1;
        break;

    case FL_F+3:
        if( Fl::event_state( FL_SHIFT ) )
        {
            display->save( 2 );
        }
        else
        {
            display->load( 2 );

            //===== Update Adjust View Screen with new values =====//
            if ( viewScreen->IsShown() )
            {
                viewScreen->UpdateAll();
            }
        }
        handled = 1;
        break;

    case FL_F+4:
        if( Fl::event_state( FL_SHIFT ) )
        {
            display->save( 3 );
        }
        else
        {
            display->load( 3 );

            //===== Update Adjust View Screen with new values =====//
            if ( viewScreen->IsShown() )
            {
                viewScreen->UpdateAll();
            }
        }
        handled = 1;
        break;

    case FL_Alt_L:
    case FL_Alt_R:

        // If mouse still pressed, start perform mouse action.
        if( Fl::event_button1() )
        {
            m_prevLB = glm::vec2( x, y );

            // Stop Alt+B control.
            m_prevAltLB = glm::vec2( 0xFFFFFFFF );
        }
        if( Fl::event_button2() )
        {
            m_prevMB = glm::vec2( x, y );
        }
        if( Fl::event_button3() )
        {
            m_prevRB = glm::vec2( x, y );
        }		
        break;

    case FL_Control_L:
    case FL_Control_R:

        // If mouse still pressed, start perform mouse action.
        if( Fl::event_button1() )
        {
            m_prevLB = glm::vec2( x, y );

            // Stop Alt+B control.
            m_prevCtrlLB = glm::vec2( 0xFFFFFFFF );
        }
        if( Fl::event_button2() )
        {
            m_prevMB = glm::vec2( x, y );
        }
        if( Fl::event_button3() )
        {
            m_prevRB = glm::vec2( x, y );
        }		
        break;

    case FL_Meta_L:
    case FL_Meta_R:

        // If mouse still pressed, start perform mouse action.
        if( Fl::event_button1() )
        {
            m_prevLB = glm::vec2( x, y );

            // Stop Alt+B control.
            m_prevCtrlLB = glm::vec2( 0xFFFFFFFF );
        }
        if( Fl::event_button2() )
        {
            m_prevMB = glm::vec2( x, y );
        }
        if( Fl::event_button3() )
        {
            m_prevRB = glm::vec2( x, y );
        }		
        break;
    }
    redraw();

    return handled;
}

int VspGlWindow::OnKeydown()
{
    ManageCORScreen * corScreen = NULL;
    int handled = 0;

    switch( Fl::event_key() )
    {
    // 'r'
    case 0x52:
    case 0x72:
        corScreen = dynamic_cast<ManageCORScreen *> 
            ( m_ScreenMgr->GetScreen( ScreenMgr::VSP_COR_SCREEN ) );
        if( corScreen )
        {
            corScreen->EnableSelection();
        }
        break;
    case FL_Escape:
        Vehicle* vPtr = VehicleMgr.GetVehicle();
        if ( vPtr )
        {
            vector< string > none;
            vPtr->SetActiveGeomVec( none );
            m_ScreenMgr->SetUpdateFlag( true );
        }
        break;
    }
    redraw();

    return handled;
}

void VspGlWindow::_sendFeedback( Selectable * selected )
{
    // Find out where feedback is heading...
    std::string selectedFeedbackName = selected->getGroup();

    // Label Screen Feedback
    ManageLabelScreen * labelScreen = dynamic_cast<ManageLabelScreen*>
        ( m_ScreenMgr->GetScreen( ScreenMgr::VSP_LABEL_SCREEN ) );

    if( labelScreen && labelScreen->getFeedbackGroupName() == selectedFeedbackName )
    {
        // Location feedback
        SelectedLoc * loc = dynamic_cast<SelectedLoc*> ( selected );
        if( loc )
        {
            glm::vec3 placement = loc->getLoc();
            labelScreen->Set( vec3d(placement.x, placement.y, placement.z ) );

            // Only one selection is needed for label, remove this 'selected' from selection list.
            m_GEngine->getScene()->removeSelected( selected );
            selected = NULL;
        }

        // Vertex feedback
        // Cast selectable to SelectedPnt object, so that we can get Render Source Ptr.
        SelectedPnt * pnt = dynamic_cast<SelectedPnt*>( selected );
        if( pnt )
        {
            VSPGraphic::Entity * e = dynamic_cast<VSPGraphic::Entity*>(pnt->getSource());
            if(e)
            {
                ID * id = _findID( e->getID() );
                if( id )
                {
                    int index = id->geomID.find_last_of( '_' );
                    std::string baseId = id->geomID.substr( 0, index );
                    glm::vec3 placement = e->getVertexVec(pnt->getIndex());
                    labelScreen->Set( vec3d( placement.x, placement.y, placement.z ), baseId );

                    // Only one selection is needed for label, remove this 'selected' from selection list.
                    m_GEngine->getScene()->removeSelected( selected );
                    selected = NULL;
                }
            }
        }
    }

    // Geom Screen Feedback
    ManageGeomScreen * geomScreen = dynamic_cast<ManageGeomScreen *>
        ( m_ScreenMgr->GetScreen( ScreenMgr::VSP_MANAGE_GEOM_SCREEN ) );

    if( geomScreen && geomScreen->getFeedbackGroupName() == selectedFeedbackName )
    {
        SelectedGeom * geom = dynamic_cast<SelectedGeom*>( selected );
        if( geom )
        {
            ID * id = _findID( geom->getSource()->getID() );
            if( id )
            {
                int index = id->geomID.find_last_of( '_' );
                std::string baseId = id->geomID.substr( 0, index );
                geomScreen->Set( baseId );

                // Only one selection is needed for Geom, remove this 'selected' from selection list.
                m_GEngine->getScene()->removeSelected( selected );
                selected = NULL;
            }
        }
        // Mac Fix:  On Mac, FLTK window is always focus on the last shown GUI.  In
        // this case where a geometry is selected, geometry's window becomes the new
        // focus.  The behavior locks up geometry selection process.  Set OpenGL
        // window back on focus so user can proceed without interruption.
        focus(this);
    }

    // Center of rotation Screen Feedback
    ManageCORScreen * corScreen = dynamic_cast<ManageCORScreen*>
        ( m_ScreenMgr->GetScreen( ScreenMgr::VSP_COR_SCREEN ) );

    if( corScreen && corScreen->getFeedbackGroupName() == selectedFeedbackName )
    {
        SelectedPnt * pnt = dynamic_cast<SelectedPnt*>( selected );
        if( pnt )
        {
            VSPGraphic::Renderable * entity = dynamic_cast<VSPGraphic::Renderable*>(pnt->getSource());
            if(entity)
            {
                glm::vec3 placement = entity->getVertexVec(pnt->getIndex());

                m_GEngine->getDisplay()->setCOR( -placement.x, -placement.y, -placement.z );

                ManageViewScreen * viewScreen = dynamic_cast< ManageViewScreen* >
                ( m_ScreenMgr->GetScreen( ScreenMgr::VSP_VIEW_SCREEN ) );

                if ( viewScreen->IsShown() )
                {
                    viewScreen->UpdateCOR();
                }

                m_GEngine->getDisplay()->center();

                // This is a dummy call to let corScreen know the job is done.
                corScreen->Set( vec3d( placement.x, placement.y, placement.z ) );

                // Only one selection is needed for Center of Rotation, remove this 'selected' from selection list.
                m_GEngine->getScene()->removeSelected( selected );
                selected = NULL;
            }
        }
    }

    // FitModel Feedback
    if ( FitModelMgr.getFeedbackGroupName() == selectedFeedbackName )
    {
        SelectedPnt * pnt = dynamic_cast<SelectedPnt*>( selected );
        if( pnt )
        {
            VSPGraphic::Marker * marker = dynamic_cast<VSPGraphic::Marker*>(pnt->getSource());
            if(marker)
            {
                ID *mid = _findID( marker->getID() );
                FitModelMgr.SelectPoint( mid->geomID, pnt->getIndex() );

                m_GEngine->getScene()->removeSelected( selected );
                selected = NULL;
            }
        }
    }
}

void VspGlWindow::_sendFeedback( std::vector<Selectable *> listOfSelected )
{
    for ( int i = 0; i < (int) listOfSelected.size(); i++ )
    {
        _sendFeedback( listOfSelected[i] );
    }
}
} // Close out namespace VSPGUI
