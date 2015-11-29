#include "SubGLWindow.h"
#include "VehicleMgr.h"
#include "Vehicle.h"
#include "GraphicEngine.h"
#include "Display.h"
#include "Scene.h"
#include "Viewport.h"
#include "Camera.h"
#include "Renderable.h"
#include "Common.h"
#include <vector>
#include <assert.h>

using namespace VSPGraphic;

namespace VSPGUI
{
VspSubGlWindow::VspSubGlWindow( int x, int y, int w , int h, DrawObj::ScreenEnum drawObjScreen )
    : Fl_Gl_Window( x, y, w, h, "VSP Sub GL Window" )
{
    mode( FL_RGB | FL_ALPHA | FL_DEPTH | FL_DOUBLE | FL_MULTISAMPLE );

    m_GEngine = new VSPGraphic::GraphicEngine();

    // Link this GUI to one of drawObj screen.
    m_LinkedScreen = drawObjScreen;

    m_GEngine->getDisplay()->setDisplayLayout( 1, 1 );
    m_GEngine->getDisplay()->selectViewport( 0 );
    m_GEngine->getDisplay()->changeView( VSPGraphic::Common::VSP_CAM_FRONT );

    Viewport * viewport = m_GEngine->getDisplay()->getViewport();
    assert( viewport );
    if( viewport )
    {
        // Disable border and arrows.
        viewport->showBorders( false );
        viewport->showXYZArrows( false );

        // Enable grid.
        viewport->showGridOverlay( true );
    }

    m_id = 0xFFFFFFFF;

    m_Initialized = false;
}
VspSubGlWindow::~VspSubGlWindow()
{
    delete m_GEngine;
}

void VspSubGlWindow::draw()
{
    // Initialize Glew when context is created.
    _initGLEW();

    if ( !valid() )
    {
        m_GEngine->getDisplay()->resize( w(), h() );
    }
    m_GEngine->draw();
}

void VspSubGlWindow::update()
{
    Vehicle* vPtr = VehicleMgr.GetVehicle();

    if ( vPtr )
    {
        if( this->context_valid() )
        {
            _update( vPtr->GetDrawObjs() );
        }
    }
}

void VspSubGlWindow::_update( std::vector<DrawObj *> objects )
{
    // Remove all Scene Objects.
    std::vector<unsigned int> currIds;
    currIds = m_GEngine->getScene()->getIds();
    for( int i = 0; i < ( int )currIds.size(); i++ )
    {
        m_GEngine->getScene()->removeObject( currIds[i] );
    }

    // Update Scene Objects.
    for( int i = 0; i < ( int )objects.size(); i++ )
    {
        // If this DrawObj is aimed for other screen, ignore.
        if( objects[i]->m_Screen != m_LinkedScreen )
        {
            continue;
        }

        // Load Settings.
        float red, green, blue, size;

        Renderable * rObj;
        unsigned int id;
        switch( objects[i]->m_Type )
        {
        case DrawObj::VSP_POINTS:
            m_GEngine->getScene()->createObject( Common::VSP_OBJECT_MARKER, &id );

            // Update scene object.
            red = ( float )objects[i]->m_PointColor.x();
            green = ( float )objects[i]->m_PointColor.y();
            blue = ( float )objects[i]->m_PointColor.z();

            size = ( float )objects[i]->m_PointSize;

            rObj = dynamic_cast<Renderable*>( m_GEngine->getScene()->getObject(id) );
            if( rObj )
            {
                rObj->setVisibility( objects[i]->m_Visible );
                rObj->setPrimType( VSPGraphic::Common::VSP_POINTS );
                rObj->setPointColor( red, green, blue );
                rObj->setPointSize( size );

                _loadPointData( rObj, objects[i] );
            }
            break;

        case DrawObj::VSP_LINES:
            m_GEngine->getScene()->createObject( Common::VSP_OBJECT_MARKER, &id );

            // Update scene object.
            red = ( float )objects[i]->m_LineColor.x();
            green = ( float )objects[i]->m_LineColor.y();
            blue = ( float )objects[i]->m_LineColor.z();

            size = ( float )objects[i]->m_LineWidth;

            rObj = dynamic_cast<Renderable*>( m_GEngine->getScene()->getObject( id ) );
            if( rObj )
            {
                rObj->setVisibility( objects[i]->m_Visible );
                rObj->setPrimType( VSPGraphic::Common::VSP_LINES );
                rObj->setLineColor( red, green, blue );
                rObj->setLineWidth( size );

                _loadLineData( rObj, objects[i] );
            }
            break;

        default:
            break;
        }
    }
}

void VspSubGlWindow::setZoomValue( float value )
{
    Camera * camera = m_GEngine->getDisplay()->getCamera();
    if( camera )
    {
        camera->setZoomValue( value );
    }
}

void VspSubGlWindow::_initGLEW()
{
    if( !m_Initialized )
    {
        VSPGraphic::GraphicEngine::initGlew();

        m_Initialized = true;
    }
}

void VspSubGlWindow::_loadPointData( Renderable * destObj, DrawObj * drawObj )
{
    std::vector<float> vdata;

    for( int i = 0; i < ( int )drawObj->m_PntVec.size(); i++ )
    {
        // Position x y z.
        vdata.push_back( ( float )drawObj->m_PntVec[i].x() );
        vdata.push_back( ( float )drawObj->m_PntVec[i].y() );
        vdata.push_back( ( float )drawObj->m_PntVec[i].z() );

        // Normal x y z.
        vdata.push_back( 0.0f );
        vdata.push_back( 0.0f );
        vdata.push_back( 0.0f );

        // Texture Coordinate u w.
        vdata.push_back( 0.0f );
        vdata.push_back( 0.0f );
    }
    destObj->emptyVBuffer();
    destObj->appendVBuffer( vdata.data(), sizeof( float ) * vdata.size() );
}

void VspSubGlWindow::_loadLineData( Renderable * destObj, DrawObj * drawObj )
{
    std::vector<float> vdata;

    for( int i = 1; i < ( int )drawObj->m_PntVec.size(); i++ )
    {
        // Position x y z.
        vdata.push_back( ( float )drawObj->m_PntVec[i-1].x() );
        vdata.push_back( ( float )drawObj->m_PntVec[i-1].y() );
        vdata.push_back( ( float )drawObj->m_PntVec[i-1].z() );

        // Normal x y z.
        vdata.push_back( 0.0f );
        vdata.push_back( 0.0f );
        vdata.push_back( 0.0f );

        // Texture Coordinate u w.
        vdata.push_back( 0.0f );
        vdata.push_back( 0.0f );

        vdata.push_back( ( float )drawObj->m_PntVec[i].x() );
        vdata.push_back( ( float )drawObj->m_PntVec[i].y() );
        vdata.push_back( ( float )drawObj->m_PntVec[i].z() );
        vdata.push_back( 0.0f );
        vdata.push_back( 0.0f );
        vdata.push_back( 0.0f );
        vdata.push_back( 0.0f );
        vdata.push_back( 0.0f );
    }
    destObj->emptyVBuffer();
    destObj->appendVBuffer( vdata.data(), sizeof( float ) * vdata.size() );
}
}
