#include "MainGLWindow.h"
#include "VehicleMgr.h"
#include "Vehicle.h"
#include "ScreenMgr.h"
#include "CfdMeshScreen.h"
#include "Display.h"
#include "Scene.h"
#include "Viewport.h"
#include "LayoutMgr.h"
#include "CameraMgr.h"
#include "Camera.h"
#include "SceneObject.h"
#include "Renderable.h"
#include "Pickable.h"
#include "Lighting.h"
#include "Image.h"
#include "Texture2D.h"
#include "TextureMgr.h"
#include "Entity.h"
#include "XSecEntity.h"
#include "Ruler.h"
#include "GraphicEngine.h"
#include "ScreenMgr.h"
#include "ManageLabelScreen.h"
#include "ManageLightingScreen.h"
#include "ManageGeomScreen.h"
#include "Common.h"
#include "GraphicSingletons.h"
#include "SelectedPnt.h"
#include "SelectedLoc.h"

#define PRECISION_PAN_SPEED 0.005f
#define PAN_SPEED 0.025f
#define PRECISION_ZOOM_SPEED 0.00005f
#define ZOOM_SPEED 0.00025f

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
}
VspGlWindow::~VspGlWindow()
{
    delete m_GEngine;
}

void VspGlWindow::setWindowLayout( int row, int column )
{
    m_GEngine->getDisplay()->setDisplayLayout( row, column );
    m_GEngine->getDisplay()->resize( w(), h() );
    m_GEngine->getDisplay()->selectViewport( 0 );
}

void VspGlWindow::setView( VSPGraphic::Common::VSPenum type )
{
    m_GEngine->getDisplay()->changeView( type );
}

void VspGlWindow::pan( int dx, int dy, bool precisionOn )
{
    float x = 0;
    float y = 0;

    if( precisionOn )
    {
        if( dx != 0 )
        {
            x = PRECISION_PAN_SPEED * ( dx < 0 ? -1 : 1 );
        }

        if( dy != 0 )
        {
            y = PRECISION_PAN_SPEED * ( dy < 0 ? -1 : 1 );
        }
    }
    else
    {
        if( dx != 0 )
        {
            x = PAN_SPEED * ( dx < 0 ? -1 : 1 );
        }

        if( dy != 0 )
        {
            y = PAN_SPEED * ( dy < 0 ? -1 : 1 );
        }
    }
    m_GEngine->getDisplay()->pan( x, y );
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

void VspGlWindow::show()
{
    Fl_Gl_Window::show();

    // Initialize Glew when context is created.
    _initGLEW();
}

void VspGlWindow::draw()
{
    make_current();
    if ( !valid() )
    {
        m_GEngine->getDisplay()->resize( w(), h() );
    }

    m_GEngine->draw( m_mouse_x, m_mouse_y );
}

int VspGlWindow::handle( int fl_event )
{
    // Move origin from upper left corner to lower left corner.
    int x = Fl::event_x();
    int y = h() - Fl::event_y();

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
        OnKeydown();
        return 1;

    case FL_KEYUP:
        OnKeyup( x, y );
        return 1;

    case FL_FOCUS:
        return 1;

    case FL_UNFOCUS:
        return 1;
    }
    return Fl_Gl_Window::handle( fl_event );
}

void VspGlWindow::update()
{
    Vehicle* vPtr = VehicleMgr::getInstance().GetVehicle();

    if ( vPtr )
    {
        make_current();

        // Get Render Objects from Vehicle.
        vector<DrawObj *> drawObjs = vPtr->GetDrawObjs();

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
        make_current();
        VSPGraphic::GraphicEngine::initGlew();

        m_initialized = true;
    }
}

void VspGlWindow::_update( std::vector<DrawObj *> objects )
{
    // Check for changes in DrawObjs and adjust accordingly.
    _updateBuffer( objects );

    // Process all renderable first, then all labels, lastly pickables.  Order matters.
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

        VSPGraphic::Ruler * ruler;

        switch( objects[i]->m_Type )
        {
        case DrawObj::VSP_SETTING:
            _setLighting( objects[i] );
            break;

        case DrawObj::VSP_LINES:
            // Create new scene object if needed.
            if( id == 0xFFFFFFFF )
            {
                m_GEngine->getScene()->createObject( Common::VSP_OBJECT_MARKER, &id );

                ID idInfo;
                idInfo.bufferID = id;
                idInfo.geomID = objects[i]->m_GeomID;
                m_ids.push_back( idInfo );
            }

            // Update scene object.
            rObj = dynamic_cast<Renderable*> ( m_GEngine->getScene()->getObject( id ) );
            if( rObj )
            {
                rObj->setVisibility( objects[i]->m_Visible );
                rObj->setPrimType( VSPGraphic::Common::VSP_LINES );
                rObj->setLineColor( red, green, blue );
                rObj->setLineWidth( lineWidth );

                // Update buffer data if needed.
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

        case DrawObj::VSP_WIRE_MESH:
            if( id == 0xFFFFFFFF )
            {
                m_GEngine->getScene()->createObject( Common::VSP_OBJECT_XSEC_ENTITY, &id );

                ID idInfo;
                idInfo.bufferID = id;
                idInfo.geomID = objects[i]->m_GeomID;
                m_ids.push_back( idInfo );
            }
            rObj = dynamic_cast<Renderable*> ( m_GEngine->getScene()->getObject( id ) );
            if( rObj )
            {
                rObj->setVisibility( objects[i]->m_Visible );
                rObj->setPrimType( Common::VSP_QUADS );
                rObj->setRenderStyle( Common::VSP_DRAW_WIRE_FRAME );
                rObj->setLineColor( red, green, blue );
                rObj->setLineWidth( lineWidth );

                if( objects[i]->m_GeomChanged )
                {
                    _loadXSecData( rObj, objects[i] );
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
            rObj = dynamic_cast<Renderable*> ( m_GEngine->getScene()->getObject( id ) );
            if( rObj )
            {
                rObj->setVisibility( objects[i]->m_Visible );
                rObj->setPrimType( Common::VSP_TRIANGLES );
                rObj->setRenderStyle( Common::VSP_DRAW_WIRE_FRAME );
                rObj->setLineColor( red, green, blue );
                rObj->setLineWidth( lineWidth );

                if( objects[i]->m_GeomChanged )
                {
                    _loadTrisData( rObj, objects[i] );
                }
            }
            break;

        case DrawObj::VSP_HIDDEN_MESH:
            if( id == 0xFFFFFFFF )
            {
                m_GEngine->getScene()->createObject( Common::VSP_OBJECT_XSEC_ENTITY, &id );

                ID idInfo;
                idInfo.bufferID = id;
                idInfo.geomID = objects[i]->m_GeomID;
                m_ids.push_back( idInfo );
            }
            rObj = dynamic_cast<Renderable*> ( m_GEngine->getScene()->getObject( id ) );
            if( rObj )
            {
                rObj->setVisibility( objects[i]->m_Visible );
                rObj->setPrimType( Common::VSP_QUADS );
                rObj->setRenderStyle( Common::VSP_DRAW_WIRE_FRAME_SOLID );
                rObj->setLineColor( red, green, blue );
                rObj->setLineWidth( lineWidth );

                if( objects[i]->m_GeomChanged )
                {
                    _loadXSecData( rObj, objects[i] );
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
            rObj = dynamic_cast<Renderable*> ( m_GEngine->getScene()->getObject( id ) );
            if( rObj )
            {
                rObj->setVisibility( objects[i]->m_Visible );
                rObj->setPrimType( Common::VSP_TRIANGLES );
                rObj->setRenderStyle( Common::VSP_DRAW_WIRE_FRAME_SOLID );
                rObj->setLineColor( red, green, blue );
                rObj->setLineWidth( lineWidth );

                if( objects[i]->m_GeomChanged )
                {
                    _loadTrisData( rObj, objects[i] );
                }
            }
            break;

        case DrawObj::VSP_SHADED_MESH:
            if( id == 0xFFFFFFFF )
            {
                m_GEngine->getScene()->createObject( Common::VSP_OBJECT_XSEC_ENTITY, &id );

                ID idInfo;
                idInfo.bufferID = id;
                idInfo.geomID = objects[i]->m_GeomID;
                m_ids.push_back( idInfo );
            }
            rObj = dynamic_cast<Renderable*> ( m_GEngine->getScene()->getObject( id ) );
            if( rObj )
            {
                rObj->setVisibility( objects[i]->m_Visible );
                rObj->setPrimType( Common::VSP_QUADS );
                rObj->setRenderStyle( Common::VSP_DRAW_MESH_SHADED );

                if( objects[i]->m_GeomChanged )
                {
                    _loadXSecData( rObj, objects[i] );
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
            rObj = dynamic_cast<Renderable*> ( m_GEngine->getScene()->getObject( id ) );
            if( rObj )
            {
                rObj->setVisibility( objects[i]->m_Visible );
                rObj->setPrimType( Common::VSP_TRIANGLES );
                rObj->setRenderStyle( Common::VSP_DRAW_MESH_SHADED );

                if( objects[i]->m_GeomChanged )
                {
                    _loadTrisData( rObj, objects[i] );
                }
            }
            break;

        case DrawObj::VSP_TEXTURED_MESH:
            if( id == 0xFFFFFFFF )
            {
                m_GEngine->getScene()->createObject( Common::VSP_OBJECT_XSEC_ENTITY, &id );

                ID idInfo;
                idInfo.bufferID = id;
                idInfo.geomID = objects[i]->m_GeomID;
                m_ids.push_back( idInfo );
            }
            rObj = dynamic_cast<Renderable*> ( m_GEngine->getScene()->getObject( id ) );
            if( rObj )
            {
                rObj->setVisibility( objects[i]->m_Visible );
                rObj->setPrimType( Common::VSP_QUADS );
                rObj->setRenderStyle( Common::VSP_DRAW_MESH_TEXTURED );

                if( objects[i]->m_GeomChanged )
                {
                    _loadXSecData( rObj, objects[i] );
                }
                _updateTextures( objects[i] );
            }
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
                ruler->setTextColor( objects[i]->m_TextColor.x(),
                    objects[i]->m_TextColor.y(),
                    objects[i]->m_TextColor.z() );

                ruler->setTextSize( objects[i]->m_TextSize );

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
                }
            }
            break;
        }
    }

    // Now process all pickables.  Order matters.
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
            pObj = dynamic_cast<Pickable*> ( m_GEngine->getScene()->getObject( id ) );
            if( pObj )
            {
                pObj->setGroup( objects[i]->m_FeedbackGroup );
                pObj->update();
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
                pObj->setGroup( objects[i]->m_FeedbackGroup );
                pObj->update();
            }
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
        Entity * entity = dynamic_cast<Entity *>( renderable );
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
    std::vector<std::vector<vec3d>> textureCoords = _generateTexCoordFromXSec( drawObj );

    int num_pnts = drawObj->m_PntMesh.size();
    int num_xsecs = 0;
    if ( num_pnts )
        num_xsecs = drawObj->m_PntMesh[0].size();

    // Vertex Buffer.
    for ( int i = 0 ; i < num_pnts ; i++ )
    {
        for ( int j = 0 ; j < num_xsecs ; j++ )
        {
            vdata.push_back( (float)drawObj->m_PntMesh[i][j].x() );
            vdata.push_back( (float)drawObj->m_PntMesh[i][j].y() );
            vdata.push_back( (float)drawObj->m_PntMesh[i][j].z() );
            vdata.push_back( (float)drawObj->m_NormMesh[i][j].x() );
            vdata.push_back( (float)drawObj->m_NormMesh[i][j].y() );
            vdata.push_back( (float)drawObj->m_NormMesh[i][j].z() );
            vdata.push_back( (float)textureCoords[i][j].x() );
            vdata.push_back( (float)textureCoords[i][j].y() );
        }
    }
    destObj->emptyVBuffer();
    destObj->appendVBuffer( vdata.data(), sizeof(float) * vdata.size() );

    // Element Buffer.
    for( int i = 0; i < num_pnts - 1; i++ )
    {
        for( int j = 0; j < num_xsecs; j++ )
        {
            edata.push_back( i * num_xsecs + j );
            edata.push_back( ( i + 1 ) * num_xsecs + j );

            if( j == num_xsecs - 1 )
            {	
                edata.push_back( ( i + 1 ) * num_xsecs );
                edata.push_back( i * num_xsecs );
            }
            else
            {
                edata.push_back( ( i + 1 ) * num_xsecs + j + 1 );
                edata.push_back( i * num_xsecs + j + 1 );
            }
        }
    }
    destObj->emptyEBuffer();
    destObj->appendEBuffer( edata.data(), sizeof( unsigned int ) * edata.size() );
    destObj->enableEBuffer( true );

    // Update number of xsec and pnts.
    XSecEntity * xEntity = dynamic_cast<XSecEntity*>(destObj);
    if(xEntity)
    {
        xEntity->setNumXSec(num_xsecs);
        xEntity->setNumPnts(num_pnts);
    }
}

void VspGlWindow::_loadTrisData( Renderable * destObj, DrawObj * drawObj )
{
    assert( drawObj->m_PntVec.size() == drawObj->m_NormVec.size() );

    std::vector<float> data;

    for ( int i = 0; i < ( int )drawObj->m_PntVec.size(); i++ )
    {
        data.push_back( ( float )drawObj->m_PntVec[i].x() );
        data.push_back( ( float )drawObj->m_PntVec[i].y() );
        data.push_back( ( float )drawObj->m_PntVec[i].z() );
        data.push_back( ( float )drawObj->m_NormVec[i].x() );
        data.push_back( ( float )drawObj->m_NormVec[i].y() );
        data.push_back( ( float )drawObj->m_NormVec[i].z() );
        data.push_back( 0.0f );
        data.push_back( 0.0f );
    }
    destObj->emptyVBuffer();
    destObj->appendVBuffer( data.data(), sizeof( float ) * data.size() );
}

void VspGlWindow::_loadMarkData( Renderable * destObj, DrawObj * drawObj )
{
    std::vector<float> data;

    for ( int i = 0; i < (int)drawObj->m_PntVec.size(); i++ )
    {
        data.push_back( ( float )drawObj->m_PntVec[i].x() );
        data.push_back( ( float )drawObj->m_PntVec[i].y() );
        data.push_back( ( float )drawObj->m_PntVec[i].z() );
        data.push_back( 0.0f );
        data.push_back( 0.0f );
        data.push_back( 0.0f );
        data.push_back( 0.0f );
        data.push_back( 0.0f );
    }
    destObj->emptyVBuffer();
    destObj->appendVBuffer(data.data(), sizeof( float ) * data.size());
}

std::vector<std::vector<vec3d>> VspGlWindow::_generateTexCoordFromXSec( DrawObj * drawObj )
{
    int i, j;
    std::vector<std::vector<vec3d>> coordinates;

    int num_pnts = drawObj->m_PntMesh.size();
    int num_xsecs = 0;
    if ( num_pnts )
        num_xsecs = drawObj->m_PntMesh[0].size();

    // Initialize coordinates vector.
    coordinates.resize( num_pnts );
    for ( i = 0; i < num_pnts; i++ )
    {
        coordinates[i].resize( num_xsecs );
    }

    // Calculate Texture Coordinate.
    for ( i = 0 ; i < num_pnts ; i++ )
    {
        double totalDistance = 0.0;
        double currPos = 0.0;

        // Calculate the distance between each vertex and total distance.
        coordinates[i][0].set_y( 0.0 );
        for ( j = 1 ; j < num_xsecs ; j++ )
        {
            double distance = _distance( drawObj->m_PntMesh[i][j - 1], drawObj->m_PntMesh[i][j] );
            totalDistance += distance;
            coordinates[i][j].set_y( distance );
        }

        // Normalize y.
        for ( j = 0; j < num_xsecs; j++ )
        {
            currPos += coordinates[i][j].y();

            // In case totalDistance equals 0 (pointy ends of pods), 
            // set normalized x to 0.0.
            coordinates[i][j].set_y( totalDistance <= 0.0 ? (j + 1) * (1.0 / num_xsecs) : currPos / totalDistance );
        }
    }

    for ( i = 0 ; i < num_xsecs ; i++ )
    {
        double totalDistance = 0.0;
        double currPos = 0.0;

        // Calculate the distance between each vertex and total distance.
        coordinates[0][i].set_x( 0.0 );
        for( j = 1; j < num_pnts ; j++ )
        {
            double distance = _distance( drawObj->m_PntMesh[j - 1][i], drawObj->m_PntMesh[j][i] );
            totalDistance += distance;
            coordinates[j][i].set_x( distance );
        }

        // Normalize x.
        for ( j = 0; j < num_pnts; j++ )
        {
            currPos += coordinates[j][i].x();

            // In case totalDistance equals 0 (pointy ends of pods), 
            // set normalized y to 0.0.
            coordinates[j][i].set_x( totalDistance <= 0.0 ? ( j + 1 ) * ( 1.0 / num_pnts ) : currPos / totalDistance );
        }
    }
    return coordinates;
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

double VspGlWindow::_distance( vec3d pointA, vec3d pointB )
{
    double x = pointB.x() - pointA.x();
    double y = pointB.y() - pointA.y();
    double z = pointB.z() - pointA.z();

    return std::sqrt( x * x + y * y + z * z );
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
        if( Fl::event_alt() )
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

            Viewport * vp = m_GEngine->getDisplay()->getViewport();
            if( vp )
            {
                mouseInWorld = vp->screenToWorld( glm::vec2( m_mouse_x, m_mouse_y ) );
            }

            if( m_GEngine->getScene()->selectHighlight() )
            {
                Selectable * selected = m_GEngine->getScene()->getLastSelected();
                std::string selectedFeedbackName = selected->getGroup();

                ManageLabelScreen * labelScreen = dynamic_cast<ManageLabelScreen*>
                    ( m_ScreenMgr->GetScreen( ScreenMgr::VSP_LABEL_SCREEN ) );

                if( labelScreen && labelScreen->getFeedbackGroupName() == selectedFeedbackName )
                {
                    // Cast selectable to SelectedPnt object, so that we can get Render Source Ptr.
                    SelectedPnt * pnt = dynamic_cast<SelectedPnt*>( selected );
                    if( pnt )
                    {
                        XSecEntity * xEntity = dynamic_cast<XSecEntity*>(pnt->getSource());
                        if(xEntity)
                        {
                            glm::vec3 placement = xEntity->getVertexVec(pnt->getIndex());
                            labelScreen->Set( vec3d( placement.x, placement.y, placement.z ) );
                        }
                    }
                }

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
                        }
                    }
                    // Mac Fix:  On Mac, FLTK window is always focus on the last shown GUI.  In 
                    // this case where a geometry is selected, geometry's window becomes the new
                    // focus.  The behavior locks up geometry selection process.  Set OpenGL 
                    // window back on focus so user can proceed without interruption.
                    focus(this);
                }
            }
            else if( mouseInWorld != glm::vec3( 0xFFFFFFFF ) && 
                m_GEngine->getScene()->selectLocation( mouseInWorld.x, mouseInWorld.y, mouseInWorld.z ) )
            {
                ManageLabelScreen * labelScreen = dynamic_cast<ManageLabelScreen*>
                    ( m_ScreenMgr->GetScreen( ScreenMgr::VSP_LABEL_SCREEN ) );

                if( labelScreen )
                {
                    SelectedLoc * loc = dynamic_cast<SelectedLoc*>
                        ( m_GEngine->getScene()->getLastSelected() );
                    if( loc )
                    {
                        glm::vec3 placement = loc->getLoc();
                        labelScreen->Set( vec3d(placement.x, placement.y, placement.z ) );
                    }
                }
            }
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
        }
        m_prevLBRB = glm::vec2( x, y );
    }
    else if( Fl::event_button1() )
    {
        if( Fl::event_alt() )
        {
            // Alt + LB
            if( m_prevAltLB != glm::vec2( 0xFFFFFFFF ) )
            {
                display->pan( ( int )m_prevAltLB.x, ( int )m_prevAltLB.y, x, y );
            }
            m_prevAltLB = glm::vec2( x, y );
        }
        else if( Fl::event_ctrl() )
        {
            // Ctrl + LB
            if( m_prevCtrlLB != glm::vec2( 0xFFFFFFFF ) )
            {
                display->zoom( ( int )m_prevCtrlLB.x, ( int )m_prevCtrlLB.y, x, y );
            }
            m_prevCtrlLB = glm::vec2( x, y );
        }
        else if( Fl::event_state( FL_META ) )
        {
            if( m_prevMetaLB != glm::vec2( 0xFFFFFFFF ) )
            {
                display->zoom( ( int )m_prevMetaLB.x, ( int )m_prevMetaLB.y, x, y );
            }
            m_prevMetaLB = glm::vec2( x, y );
        }
        else
        {
            // LB
            if( m_prevLB != glm::vec2( 0xFFFFFFFF ) )
            {
                display->rotate( ( int )m_prevLB.x, ( int )m_prevLB.y, x, y );
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
        }
        m_prevMB = glm::vec2( x, y );
    }
    else if( Fl::event_button3() )
    {
        // RB
        if( m_prevRB != glm::vec2( 0xFFFFFFFF ) )
        {
            display->pan( ( int )m_prevRB.x, ( int )m_prevRB.y, x, y );
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

void VspGlWindow::OnKeyup( int x, int y )
{
    VSPGraphic::Display * display = m_GEngine->getDisplay();

    switch( Fl::event_key() )
    {
    case 0x43:
    case 0x63:
        // Key 'C', center.
        display->center();
        break;

    case FL_F+1:
        if( Fl::event_state( FL_SHIFT ) )
        {
            display->save( 0 );
        }
        else
        {
            display->load( 0 );
        }
        break;

    case FL_F+2:
        if( Fl::event_state( FL_SHIFT ) )
        {
            display->save( 1 );
        }
        else
        {
            display->load( 1 );
        }
        break;

    case FL_F+3:
        if( Fl::event_state( FL_SHIFT ) )
        {
            display->save( 2 );
        }
        else
        {
            display->load( 2 );
        }
        break;

    case FL_F+4:
        if( Fl::event_state( FL_SHIFT ) )
        {
            display->save( 3 );
        }
        else
        {
            display->load( 3 );
        }
        break;

    case FL_F+5:
        display->changeView( Common::VSP_CAM_TOP );
        break;

    case FL_F+6:
        display->changeView( Common::VSP_CAM_FRONT );
        break;

    case FL_F+7:
        display->changeView( Common::VSP_CAM_LEFT );
        break;

    case FL_F+8:
        display->changeView( Common::VSP_CAM_LEFT_ISO );
        break;

    case FL_F+9:
        display->changeView( Common::VSP_CAM_BOTTOM );
        break;

    case FL_F+10:
        display->changeView( Common::VSP_CAM_REAR );
        break;

    case FL_F+11:
        display->changeView( Common::VSP_CAM_RIGHT );
        break;

    case FL_F+12:
        display->changeView( Common::VSP_CAM_RIGHT_ISO );
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
}

void VspGlWindow::OnKeydown()
{
    switch( Fl::event_key() )
    {
    case 'p':
        ManageGeomScreen * geomScreen = dynamic_cast<ManageGeomScreen *>
            ( m_ScreenMgr->GetScreen( ScreenMgr::VSP_MANAGE_GEOM_SCREEN ) );
        if( geomScreen )
        {
            geomScreen->TriggerPickSwitch();
        }
        break;
    }
    redraw();
}
}