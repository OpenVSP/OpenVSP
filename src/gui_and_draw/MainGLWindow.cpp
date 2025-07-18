//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//

#ifdef WIN32
#undef _HAS_STD_BYTE
#define _HAS_STD_BYTE 0
#endif

#define _USE_MATH_DEFINES
#include <cmath>

#include "MainGLWindow.h"

#include "Background3DMgr.h"
#include "BORGeom.h"
#include "Camera.h"
#include "CfdMeshScreen.h"
#include "Clipping.h"
#include "ClippingScreen.h"
#include "CurveEditScreen.h"
#include "Display.h"
#include "Entity.h"
#include "FitModelScreen.h"
#include "GeometryAnalysisScreen.h"
#include "GraphicEngine.h"
#include "GraphicSingletons.h"
#include "Image.h"
#include "LayoutMgr.h"
#include "Lighting.h"
#include "ManageBackground3DScreen.h"
#include "ManageCORScreen.h"
#include "ManageGeomScreen.h"
#include "ManageLightingScreen.h"
#include "ManageMeasureScreen.h"
#include "ManageViewScreen.h"
#include "MassPropScreen.h"
#include "MeasureMgr.h"
#include "Pickable.h"
#include "PickablePnts.h"
#include "Probe.h"
#include "Protractor.h"
#include "Routing.h"
#include "Renderable.h"
#include "RoutingScreen.h"
#include "Ruler.h"
#include "Scene.h"
#include "ScreenMgr.h"
#include "Selectable.h"
#include "SelectedLoc.h"
#include "SelectedPnt.h"
#include "StructAssemblyScreen.h"
#include "StructScreen.h"
#include "SurfaceIntersectionScreen.h"
#include "Texture2D.h"
#include "Vehicle.h"
#include "VehicleMgr.h"
#include "Viewport.h"
#include "Watermark.h"
#include "VSPAEROScreen.h"
#include "WaveDragScreen.h"

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

    m_mouse_x = m_mouse_y = 0xFFFFFFF;

    m_initialized = false;

    m_prevViewport = -1;
    m_prevLB = m_prevRB = m_prevMB = glm::vec2( fNAN );
    m_prevAltLB = m_prevCtrlLB = m_prevMetaLB = glm::vec2( fNAN );
    m_prevLBRB = glm::vec2( fNAN );

    m_ScreenMgr = mgr;

    m_hasSetSize = false;
    m_noRotate = false;

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

void VspGlWindow::relativeZoom( float zoomValue )
{
    m_GEngine->getDisplay()->relativeZoom( zoomValue );
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
        ( m_ScreenMgr->GetScreen( vsp::VSP_VIEW_SCREEN ) );

        UpdateViewportParms();

        //Make sure the current width and height update
        m_ScreenMgr->GetScreen( vsp::VSP_SCREENSHOT_SCREEN )->Update();
    }

    m_GEngine->draw( m_mouse_x, m_mouse_y );
}

int VspGlWindow::handle( int fl_event )
{
    // Move origin from upper left corner to lower left corner.
    int x = Fl::event_x();
    int y = h() - Fl::event_y();

    x *= pixels_per_unit();
    y *= pixels_per_unit();

    m_mouse_x = x;
    m_mouse_y = y;

    int dx = Fl::event_dx();
    int dy = Fl::event_dy();

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
        OnWheelScroll( dx, dy, x, y );
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

    if ( vPtr && m_initialized )
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

            if ( AttributeMgr.GetDirtyFlag( vsp::ATTR_GROUP_WATERMARK ) )
            {
                AttributeMgr.ClearDirtyFlag( vsp::ATTR_GROUP_WATERMARK );

                VSPGraphic::Viewport *vp = display->getViewport();

                if ( vp )
                {
                    VSPGraphic::Watermark * wm = vp->getWatermark();

                    if ( wm )
                    {
                        if ( AttributeMgr.GetAttributePtr( ATTR_WM_SHOW ) )
                        {
                            display->getViewport()->showWatermark( AttributeMgr.GetAttributePtr( ATTR_WM_SHOW )->GetBool( 0 ) );
                            if ( AttributeMgr.GetAttributePtr( ATTR_WM_TEXT ) )
                            {
                                wm->setText( AttributeMgr.GetAttributePtr( ATTR_WM_TEXT )->GetString( 0 ) );
                            }

                            if ( AttributeMgr.GetAttributePtr( ATTR_WM_TEXTSCALE ) )
                            {
                                wm->setTextScale( AttributeMgr.GetAttributePtr( ATTR_WM_TEXTSCALE )->GetDouble( 0 ) );
                            }

                            vec3d tc = vec3d(0.,0.,0.);
                            vec3d bc = vec3d(0.,0.,0.);
                            vec3d fc = vec3d(0.,0.,0.);
                            double ta = 0.;
                            double ba = 0.;
                            double fa = 0.;

                            if ( AttributeMgr.GetAttributePtr( ATTR_WM_TEXTCOLOR ) )
                            {
                                tc = AttributeMgr.GetAttributePtr( ATTR_WM_TEXTCOLOR )->GetVec3d( 0 );
                            }
                            if ( AttributeMgr.GetAttributePtr( ATTR_WM_TEXTALPHA ) )
                            {
                                ta = AttributeMgr.GetAttributePtr( ATTR_WM_TEXTALPHA )->GetDouble( 0 );
                            }
                            if ( AttributeMgr.GetAttributePtr( ATTR_WM_EDGECOLOR ) )
                            {
                                bc = AttributeMgr.GetAttributePtr( ATTR_WM_EDGECOLOR )->GetVec3d( 0 );
                            }
                            if ( AttributeMgr.GetAttributePtr( ATTR_WM_EDGEALPHA ) )
                            {
                                ba = AttributeMgr.GetAttributePtr( ATTR_WM_EDGEALPHA )->GetDouble( 0 );
                            }
                            if ( AttributeMgr.GetAttributePtr( ATTR_WM_FILLCOLOR ) )
                            {
                                fc = AttributeMgr.GetAttributePtr( ATTR_WM_FILLCOLOR )->GetVec3d( 0 );
                            }
                            if ( AttributeMgr.GetAttributePtr( ATTR_WM_FILLALPHA ) )
                            {
                                fa = AttributeMgr.GetAttributePtr( ATTR_WM_FILLALPHA )->GetDouble( 0 );
                            }

                            wm->setTextColor( tc.x(), tc.y(), tc.z(), ta );
                            wm->setLineColor( bc.x(), bc.y(), bc.z(), ba );
                            wm->setFillColor( fc.x(), fc.y(), fc.z(), fa );

                        }
                        else
                        {
                            display->getViewport()->showWatermark( false );
                        }
                    }
                }
            }
        }

        vector<DrawObj *> drawObjs;
        LoadAllDrawObjs( drawObjs );

        // Load Objects to Renderer.
        _update( drawObjs );

        for( int i = 0; i < (int)drawObjs.size(); i++ )
        {
            if( drawObjs[i]->m_Screen == m_LinkedScreen )
            {
                drawObjs[i]->m_GeomChanged = false;
            }
        }
    }
}

void VspGlWindow::LoadAllDrawObjs( vector< DrawObj* > & drawObjs )
{
    Vehicle* vPtr = VehicleMgr.GetVehicle();

    if ( vPtr )
    {
        // Get Render Objects from Vehicle.
        vPtr->LoadDrawObjs( drawObjs );

        // Load Render Objects from fitModelScreen.
        FitModelScreen * fitModelScreen = dynamic_cast< FitModelScreen* >
            ( m_ScreenMgr->GetScreen( vsp::VSP_FIT_MODEL_SCREEN ) );
        if ( fitModelScreen )
        {
            fitModelScreen->LoadDrawObjs( drawObjs );
        }

        // Load Render Objects from massPropScreen.
        MassPropScreen* massPropScreen = dynamic_cast<MassPropScreen*>
            ( m_ScreenMgr->GetScreen( vsp::VSP_MASS_PROP_SCREEN ) );
        if ( massPropScreen )
        {
            massPropScreen->LoadDrawObjs( drawObjs );
        }

        // Load Render Objects from CfdMeshScreen.
        CfdMeshScreen * cfdScreen = dynamic_cast< CfdMeshScreen* >
            ( m_ScreenMgr->GetScreen( vsp::VSP_CFD_MESH_SCREEN ) );
        if( cfdScreen )
        {
            cfdScreen->LoadDrawObjs( drawObjs );
        }

        // Load Render Objects from SurfaceIntersectionScreen.
        SurfaceIntersectionScreen * surfScreen = dynamic_cast< SurfaceIntersectionScreen* >
            ( m_ScreenMgr->GetScreen( vsp::VSP_SURFACE_INTERSECTION_SCREEN ) );
        if( surfScreen )
        {
            surfScreen->LoadDrawObjs( drawObjs );
        }

        // I don't like having this Update() here.  However, there does not appear
        // to be a better place to put it.  Vehicle::Update does not get called
        // frequently enough.
        MeasureMgr.Update();
        // Load Render Objects from measureScreen.
        ManageMeasureScreen * measureScreen = dynamic_cast< ManageMeasureScreen* >
            ( m_ScreenMgr->GetScreen( vsp::VSP_MEASURE_SCREEN ) );
        if( measureScreen )
        {
            measureScreen->LoadDrawObjs( drawObjs );
        }

        // Needed here because Background position can depend on bounding box size.
        Background3DMgr.Update();
        // Load Render Objects from ManageBackground3DScreen.
        ManageBackground3DScreen * bg3dScreen = dynamic_cast< ManageBackground3DScreen* >
        ( m_ScreenMgr->GetScreen( vsp::VSP_BACKGROUND3D_SCREEN ) );
        if( bg3dScreen )
        {
            bg3dScreen->LoadDrawObjs( drawObjs );
        }

        // Load Render Objects from FeaStructScreen.
        StructScreen * structScreen = dynamic_cast< StructScreen* >
            ( m_ScreenMgr->GetScreen( vsp::VSP_STRUCT_SCREEN ) );
        if( structScreen )
        {
            structScreen->LoadDrawObjs( drawObjs );
        }

        // Load Render Objects from FeaStructAssemblyScreen.
        StructAssemblyScreen * structAssemblyScreen = dynamic_cast< StructAssemblyScreen* >
            ( m_ScreenMgr->GetScreen( vsp::VSP_STRUCT_ASSEMBLY_SCREEN ) );
        if( structAssemblyScreen )
        {
            structAssemblyScreen->LoadDrawObjs( drawObjs );
        }

        // Load Render Objects from lightScreen.
        ManageLightingScreen * lightScreen = dynamic_cast< ManageLightingScreen* >
            ( m_ScreenMgr->GetScreen( vsp::VSP_LIGHTING_SCREEN ) );
        if( lightScreen )
        {
            lightScreen->LoadDrawObjs( drawObjs );
        }

        // Load Render Objects from geomScreen.
        ManageGeomScreen * geomScreen = dynamic_cast< ManageGeomScreen* >
            ( m_ScreenMgr->GetScreen( vsp::VSP_MANAGE_GEOM_SCREEN ) );
        if( geomScreen )
        {
            geomScreen->LoadDrawObjs( drawObjs );
        }

        // Load Render Objects for routing screen
        if ( geomScreen )
        {
            RoutingScreen * routingScreen = dynamic_cast<RoutingScreen*>
                ( geomScreen->GetGeomScreen( vsp::ROUTING_GEOM_SCREEN ) );
            if ( routingScreen )
            {
                routingScreen->LoadDrawObjs( drawObjs );
            }
        }

        // Load Render Objects from corScreen ( Center of Rotation ).
        ManageCORScreen * corScreen = dynamic_cast< ManageCORScreen* >
            ( m_ScreenMgr->GetScreen( vsp::VSP_COR_SCREEN ) );
        if( corScreen )
        {
            corScreen->LoadDrawObjs( drawObjs );
        }

        // Load Render Objects from clipScreen ( Clipping ).
        ClippingScreen * clipScreen = dynamic_cast< ClippingScreen* >
            ( m_ScreenMgr->GetScreen( vsp::VSP_CLIPPING_SCREEN ) );
        if( clipScreen )
        {
            clipScreen->LoadDrawObjs( drawObjs );
        }

        // Load Render Objects from WDSrceen ( Wave Drag Tool)
        WaveDragScreen * WDScreen = dynamic_cast< WaveDragScreen* >
            ( m_ScreenMgr->GetScreen( vsp::VSP_WAVEDRAG_SCREEN ) );
        if( WDScreen )
        {
            WDScreen->LoadDrawObjs( drawObjs );
        }

        // Load Render Objects for VSP AERO Screen
        VSPAEROScreen * AeroScreen = dynamic_cast < VSPAEROScreen* >
            ( m_ScreenMgr->GetScreen( vsp::VSP_VSPAERO_SCREEN ) );
        if ( AeroScreen )
        {
            AeroScreen->LoadDrawObjs( drawObjs );
        }

        // Load Render Objects for Geometry Analysis
        GeometryAnalysisScreen * InterfereScreen = dynamic_cast < GeometryAnalysisScreen* >
            ( m_ScreenMgr->GetScreen( vsp::VSP_GEOMETRY_ANALYSIS_SCREEN ) );
        if ( InterfereScreen )
        {
            InterfereScreen->LoadDrawObjs( drawObjs );
        }
    }
}

void VspGlWindow::ResetDrawObjsGeomChangedFlags( vector< DrawObj* > & drawObjs, bool flag )
{
    for( int i = 0; i < (int)drawObjs.size(); i++ )
    {
        drawObjs[i]->m_GeomChanged = flag;
    }
}

void VspGlWindow::ResetDrawObjsGeomChangedFlags( bool flag )
{
    vector<DrawObj *> drawObjs;
    LoadAllDrawObjs( drawObjs );

    ResetDrawObjsGeomChangedFlags( drawObjs, flag );
}

void VspGlWindow::clearScene()
{
    m_GEngine->getScene()->clearScene();
    m_idMap.clear();
    m_GEngine->getDisplay()->getViewport()->clearBackground();
    m_GEngine->getDisplay()->getViewport()->clearFont();
}

void VspGlWindow::UpdateViewportParms()
{
    MainVSPScreen* main = dynamic_cast< MainVSPScreen* >( m_ScreenMgr->GetScreen( vsp::VSP_MAIN_SCREEN ) );

    if ( main )
    {
        VSPGUI::VspGlWindow * glwin = main->GetGLWindow();

        if ( glwin == this )
        {
            Vehicle *veh = VehicleMgr.GetVehicle();
            if ( veh )
            {
                veh->m_ViewportSizeXValue.Set( pixel_w() );
                veh->m_ViewportSizeYValue.Set( pixel_h() );
            }
        }
    }
}

void VspGlWindow::UpdateCORParms()
{
    MainVSPScreen* main = dynamic_cast< MainVSPScreen* >( m_ScreenMgr->GetScreen( vsp::VSP_MAIN_SCREEN ) );

    if ( main )
    {
        VSPGUI::VspGlWindow * glwin = main->GetGLWindow();

        if ( glwin == this )
        {
            Vehicle *veh = VehicleMgr.GetVehicle();

            if ( veh )
            {
                glm::vec3 center = getCOR();

                veh->m_CORXValue.Set( center.x );
                veh->m_CORYValue.Set( center.y );
                veh->m_CORZValue.Set( center.z );
            }
        }
    }
}

void VspGlWindow::UpdatePanParms()
{
    MainVSPScreen* main = dynamic_cast< MainVSPScreen* >( m_ScreenMgr->GetScreen( vsp::VSP_MAIN_SCREEN ) );

    if ( main )
    {
        VSPGUI::VspGlWindow * glwin = main->GetGLWindow();

        if ( glwin == this )
        {
            Vehicle *veh = VehicleMgr.GetVehicle();

            if ( veh )
            {
                glm::vec2 currentPan = getPanValues();
                veh->m_PanXPosValue.Set( currentPan.x );
                veh->m_PanYPosValue.Set( currentPan.y );
            }
        }
    }
}

void VspGlWindow::UpdateZoomParms()
{
    MainVSPScreen* main = dynamic_cast< MainVSPScreen* >( m_ScreenMgr->GetScreen( vsp::VSP_MAIN_SCREEN ) );

    if ( main )
    {
        VSPGUI::VspGlWindow * glwin = main->GetGLWindow();

        if ( glwin == this )
        {
            Vehicle *veh = VehicleMgr.GetVehicle();

            if ( veh )
            {
                veh->m_ZoomValue.Set( getRelativeZoomValue() );
            }
        }
    }
}

void VspGlWindow::UpdateRotationParms()
{
    MainVSPScreen* main = dynamic_cast< MainVSPScreen* >( m_ScreenMgr->GetScreen( vsp::VSP_MAIN_SCREEN ) );

    if ( main )
    {
        VSPGUI::VspGlWindow * glwin = main->GetGLWindow();

        if ( glwin == this )
        {
            Vehicle *veh = VehicleMgr.GetVehicle();

            if ( veh )
            {
                glm::vec3 eulerValues = getRotationEulerAngles();

                veh->m_XRotationValue.Set( eulerValues[0] * ( 180.0 / M_PI ) );
                veh->m_YRotationValue.Set( eulerValues[1] * ( 180.0 / M_PI ) );
                veh->m_ZRotationValue.Set( eulerValues[2] * ( 180.0 / M_PI ) );
            }
        }
    }
}

void VspGlWindow::UpdateAllViewParms()
{
    UpdateViewportParms();
    UpdateCORParms();
    UpdatePanParms();
    UpdateZoomParms();
    UpdateRotationParms();
}

void VspGlWindow::_initGLEW()
{
    if( !m_initialized )
    {
        VSPGraphic::GraphicEngine::initGlew();

        m_initialized = true;
    }
}

void VspGlWindow::_update( const std::vector<DrawObj *> &objects )
{
    // Check for changes in DrawObjs and adjust accordingly.
    _updateBuffer( objects );

    // Get view Z vector for 3D background visibility check.
    Display * dis = m_GEngine->getDisplay();
    if ( dis )
    {
        Viewport * vpt = dis->getViewport();
        if ( vpt )
        {
            Camera * cam = vpt->getCamera();
            if ( cam )
            {
                glm::mat4 mvm = cam->getModelViewMatrix();

                vec3d v( mvm[0][2], mvm[1][2], mvm[2][2] );

                for( int i = 0; i < (int)objects.size(); i++ )
                {
                    if ( objects[i]->m_VisibleDirFlag )
                    {
                        if ( acos( std::abs( dot( objects[ i ]->m_VisibleDir, v ) ) ) * ( 180.0 / M_PI ) > objects[ i ]->m_VisTol )
                        {
                            objects[ i ]->m_Visible = false;
                        }
                    }
                }
            }
        }
    }

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
                m_idMap[ idInfo.geomID ] = idInfo;
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
                m_idMap[ idInfo.geomID ] = idInfo;
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

        case DrawObj::VSP_WIRE_HIDDEN_MESH:
            if( id == 0xFFFFFFFF )
            {
                m_GEngine->getScene()->createObject( Common::VSP_OBJECT_ENTITY, &id );

                ID idInfo;
                idInfo.bufferID = id;
                idInfo.geomID = objects[i]->m_GeomID;
                m_idMap[ idInfo.geomID ] = idInfo;
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
                m_idMap[ idInfo.geomID ] = idInfo;
            }
            eObj = dynamic_cast<VSPGraphic::Entity*> ( m_GEngine->getScene()->getObject( id ) );
            if( eObj )
            {
                eObj->setVisibility( objects[i]->m_Visible );
                eObj->setPrimType( Common::VSP_TRIANGLES );
                eObj->setRenderStyle( Common::VSP_DRAW_SOLID );
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

        case DrawObj::VSP_WIRE_HIDDEN_TRIS:
            if( id == 0xFFFFFFFF )
            {
                m_GEngine->getScene()->createObject( Common::VSP_OBJECT_ENTITY, &id );

                ID idInfo;
                idInfo.bufferID = id;
                idInfo.geomID = objects[i]->m_GeomID;
                m_idMap[ idInfo.geomID ] = idInfo;
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

        case DrawObj::VSP_MAPPED_TRIS:
            if( id == 0xFFFFFFFF )
            {
                m_GEngine->getScene()->createObject( Common::VSP_OBJECT_ENTITY, &id );

                ID idInfo;
                idInfo.bufferID = id;
                idInfo.geomID = objects[i]->m_GeomID;
                m_idMap[ idInfo.geomID ] = idInfo;
            }
            eObj = dynamic_cast<VSPGraphic::Entity*> ( m_GEngine->getScene()->getObject( id ) );
            if( eObj )
            {
                eObj->setVisibility( objects[i]->m_Visible );
                eObj->setPrimType( Common::VSP_TRIANGLES );
                eObj->setRenderStyle( Common::VSP_DRAW_MAPPED );
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

        case DrawObj::VSP_WIRE_MAPPED_TRIS:
            if( id == 0xFFFFFFFF )
            {
                m_GEngine->getScene()->createObject( Common::VSP_OBJECT_ENTITY, &id );

                ID idInfo;
                idInfo.bufferID = id;
                idInfo.geomID = objects[i]->m_GeomID;
                m_idMap[ idInfo.geomID ] = idInfo;
            }
            eObj = dynamic_cast<VSPGraphic::Entity*> ( m_GEngine->getScene()->getObject( id ) );
            if( eObj )
            {
                eObj->setVisibility( objects[i]->m_Visible );
                eObj->setPrimType( Common::VSP_TRIANGLES );
                eObj->setRenderStyle( Common::VSP_DRAW_WIRE_FRAME_MAPPED );
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

        case DrawObj::VSP_CFD_HIDDEN_TRIS:
            if( id == 0xFFFFFFFF )
            {
                m_GEngine->getScene()->createObject( Common::VSP_OBJECT_CFD_ENTITY, &id );

                ID idInfo;
                idInfo.bufferID = id;
                idInfo.geomID = objects[i]->m_GeomID;
                m_idMap[ idInfo.geomID ] = idInfo;
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

        case DrawObj::VSP_WIRE_SHADED_TRIS:
            if( id == 0xFFFFFFFF )
            {
                m_GEngine->getScene()->createObject( Common::VSP_OBJECT_CFD_ENTITY, &id );

                ID idInfo;
                idInfo.bufferID = id;
                idInfo.geomID = objects[i]->m_GeomID;
                m_idMap[ idInfo.geomID ] = idInfo;
            }
            eObj = dynamic_cast<VSPGraphic::Entity*> ( m_GEngine->getScene()->getObject( id ) );
            if( eObj )
            {
                eObj->setVisibility( objects[i]->m_Visible );
                eObj->setPrimType( Common::VSP_TRIANGLES );
                eObj->setRenderStyle( Common::VSP_DRAW_WIRE_FRAME_SHADED );
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

        case DrawObj::VSP_WIRE_HIDDEN_QUADS:
            if( id == 0xFFFFFFFF )
            {
                m_GEngine->getScene()->createObject( Common::VSP_OBJECT_ENTITY, &id );

                ID idInfo;
                idInfo.bufferID = id;
                idInfo.geomID = objects[i]->m_GeomID;
                m_idMap[ idInfo.geomID ] = idInfo;
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
                    _loadQuadsData( eObj, objects[i] );
                }
            }
            break;

        case DrawObj::VSP_CFD_HIDDEN_QUADS:
            if( id == 0xFFFFFFFF )
            {
                m_GEngine->getScene()->createObject( Common::VSP_OBJECT_CFD_ENTITY, &id );

                ID idInfo;
                idInfo.bufferID = id;
                idInfo.geomID = objects[i]->m_GeomID;
                m_idMap[ idInfo.geomID ] = idInfo;
            }
            eObj = dynamic_cast<VSPGraphic::Entity*> ( m_GEngine->getScene()->getObject( id ) );
            if( eObj )
            {
                eObj->setVisibility( objects[i]->m_Visible );
                eObj->setPrimType( Common::VSP_QUADS );
                eObj->setRenderStyle( Common::VSP_DRAW_WIRE_FRAME_TRANSPARENT_BACK );
                eObj->setLineColor( red, green, blue );
                eObj->setLineWidth( lineWidth );

                eObj->setMaterial( objects[i]->m_MaterialInfo.Ambient, objects[i]->m_MaterialInfo.Diffuse,
                    objects[i]->m_MaterialInfo.Specular, objects[i]->m_MaterialInfo.Emission,
                    objects[i]->m_MaterialInfo.Shininess );

                if( objects[i]->m_GeomChanged )
                {
                    _loadQuadsData( eObj, objects[i] );
                }
            }
            break;

        case DrawObj::VSP_SHADED_QUADS:
            if ( id == 0xFFFFFFFF )
            {
                m_GEngine->getScene()->createObject( Common::VSP_OBJECT_ENTITY, &id );

                ID idInfo;
                idInfo.bufferID = id;
                idInfo.geomID = objects[i]->m_GeomID;
                m_idMap[ idInfo.geomID ] = idInfo;
            }
            eObj = dynamic_cast<VSPGraphic::Entity*> ( m_GEngine->getScene()->getObject( id ) );
            if ( eObj )
            {
                eObj->setVisibility( objects[i]->m_Visible );
                eObj->setPrimType( Common::VSP_QUADS );
                eObj->setRenderStyle( Common::VSP_DRAW_SHADED );

                eObj->setMaterial( objects[i]->m_MaterialInfo.Ambient, objects[i]->m_MaterialInfo.Diffuse,
                                   objects[i]->m_MaterialInfo.Specular, objects[i]->m_MaterialInfo.Emission,
                                   objects[i]->m_MaterialInfo.Shininess );

                if ( objects[i]->m_GeomChanged )
                {
                    _loadQuadsData( eObj, objects[i] );
                }
            }
            break;

        case DrawObj::VSP_WIRE_QUADS:
            if ( id == 0xFFFFFFFF )
            {
                m_GEngine->getScene()->createObject( Common::VSP_OBJECT_ENTITY, &id );

                ID idInfo;
                idInfo.bufferID = id;
                idInfo.geomID = objects[i]->m_GeomID;
                m_idMap[ idInfo.geomID ] = idInfo;
            }
            eObj = dynamic_cast<VSPGraphic::Entity*> ( m_GEngine->getScene()->getObject( id ) );
            if ( eObj )
            {
                eObj->setVisibility( objects[i]->m_Visible );
                eObj->setPrimType( Common::VSP_QUADS );
                eObj->setRenderStyle( Common::VSP_DRAW_WIRE_FRAME );
                eObj->setLineColor( red, green, blue );
                eObj->setLineWidth( lineWidth );

                eObj->setMaterial( objects[i]->m_MaterialInfo.Ambient, objects[i]->m_MaterialInfo.Diffuse,
                                   objects[i]->m_MaterialInfo.Specular, objects[i]->m_MaterialInfo.Emission,
                                   objects[i]->m_MaterialInfo.Shininess );

                if ( objects[i]->m_GeomChanged )
                {
                    _loadQuadsData( eObj, objects[i] );
                }
            }
            break;

        case DrawObj::VSP_WIRE_SHADED_QUADS:
            if ( id == 0xFFFFFFFF )
            {
                m_GEngine->getScene()->createObject( Common::VSP_OBJECT_ENTITY, &id );

                ID idInfo;
                idInfo.bufferID = id;
                idInfo.geomID = objects[i]->m_GeomID;
                m_idMap[ idInfo.geomID ] = idInfo;
            }
            eObj = dynamic_cast<VSPGraphic::Entity*> ( m_GEngine->getScene()->getObject( id ) );
            if ( eObj )
            {
                eObj->setVisibility( objects[i]->m_Visible );
                eObj->setPrimType( Common::VSP_QUADS );
                eObj->setRenderStyle( Common::VSP_DRAW_WIRE_FRAME_SHADED );
                eObj->setLineColor( red, green, blue );
                eObj->setLineWidth( lineWidth );

                eObj->setMaterial( objects[i]->m_MaterialInfo.Ambient, objects[i]->m_MaterialInfo.Diffuse,
                                   objects[i]->m_MaterialInfo.Specular, objects[i]->m_MaterialInfo.Emission,
                                   objects[i]->m_MaterialInfo.Shininess );

                if ( objects[i]->m_GeomChanged )
                {
                    _loadQuadsData( eObj, objects[i] );
                }
            }
            break;

        case DrawObj::VSP_MAPPED_QUADS:
            if ( id == 0xFFFFFFFF )
            {
                m_GEngine->getScene()->createObject( Common::VSP_OBJECT_ENTITY, &id );

                ID idInfo;
                idInfo.bufferID = id;
                idInfo.geomID = objects[i]->m_GeomID;
                m_idMap[ idInfo.geomID ] = idInfo;
            }
            eObj = dynamic_cast<VSPGraphic::Entity*> ( m_GEngine->getScene()->getObject( id ) );
            if ( eObj )
            {
                eObj->setVisibility( objects[i]->m_Visible );
                eObj->setPrimType( Common::VSP_QUADS );
                eObj->setRenderStyle( Common::VSP_DRAW_MAPPED );
                eObj->setLineColor( red, green, blue );
                eObj->setLineWidth( lineWidth );

                eObj->setMaterial( objects[i]->m_MaterialInfo.Ambient, objects[i]->m_MaterialInfo.Diffuse,
                                   objects[i]->m_MaterialInfo.Specular, objects[i]->m_MaterialInfo.Emission,
                                   objects[i]->m_MaterialInfo.Shininess );

                if ( objects[i]->m_GeomChanged )
                {
                    _loadQuadsData( eObj, objects[i] );
                }
            }
            break;

        case DrawObj::VSP_WIRE_MAPPED_QUADS:
            if ( id == 0xFFFFFFFF )
            {
                m_GEngine->getScene()->createObject( Common::VSP_OBJECT_ENTITY, &id );

                ID idInfo;
                idInfo.bufferID = id;
                idInfo.geomID = objects[i]->m_GeomID;
                m_idMap[ idInfo.geomID ] = idInfo;
            }
            eObj = dynamic_cast<VSPGraphic::Entity*> ( m_GEngine->getScene()->getObject( id ) );
            if ( eObj )
            {
                eObj->setVisibility( objects[i]->m_Visible );
                eObj->setPrimType( Common::VSP_QUADS );
                eObj->setRenderStyle( Common::VSP_DRAW_WIRE_FRAME_MAPPED );
                eObj->setLineColor( red, green, blue );
                eObj->setLineWidth( lineWidth );

                eObj->setMaterial( objects[i]->m_MaterialInfo.Ambient, objects[i]->m_MaterialInfo.Diffuse,
                                   objects[i]->m_MaterialInfo.Specular, objects[i]->m_MaterialInfo.Emission,
                                   objects[i]->m_MaterialInfo.Shininess );

                if ( objects[i]->m_GeomChanged )
                {
                    _loadQuadsData( eObj, objects[i] );
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
                m_idMap[ idInfo.geomID ] = idInfo;
            }
            eObj = dynamic_cast<VSPGraphic::Entity*> ( m_GEngine->getScene()->getObject( id ) );
            if( eObj )
            {
                eObj->setVisibility( objects[i]->m_Visible );
                eObj->setPrimType( Common::VSP_QUADS );
                eObj->setRenderStyle( Common::VSP_DRAW_SHADED );

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
                m_idMap[ idInfo.geomID ] = idInfo;
            }
            eObj = dynamic_cast<VSPGraphic::Entity*> ( m_GEngine->getScene()->getObject( id ) );
            if( eObj )
            {
                eObj->setVisibility( objects[i]->m_Visible );
                eObj->setPrimType( Common::VSP_TRIANGLES );
                eObj->setRenderStyle( Common::VSP_DRAW_SHADED );

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
                m_idMap[ idInfo.geomID ] = idInfo;
            }
            eObj = dynamic_cast<VSPGraphic::Entity*> ( m_GEngine->getScene()->getObject( id ) );
            if( eObj )
            {
                eObj->setVisibility( objects[i]->m_Visible );
                eObj->setPrimType( Common::VSP_QUADS );
                eObj->setRenderStyle( Common::VSP_DRAW_TEXTURED );

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

        case DrawObj::VSP_TEXTURED_MESH_TRANSPARENT_BACK:
            if( id == 0xFFFFFFFF )
            {
                m_GEngine->getScene()->createObject( Common::VSP_OBJECT_ENTITY, &id );

                ID idInfo;
                idInfo.bufferID = id;
                idInfo.geomID = objects[i]->m_GeomID;
                m_idMap[ idInfo.geomID ] = idInfo;
            }
            eObj = dynamic_cast<VSPGraphic::Entity*> ( m_GEngine->getScene()->getObject( id ) );
            if( eObj )
            {
                eObj->setVisibility( objects[i]->m_Visible );
                eObj->setPrimType( Common::VSP_QUADS );
                eObj->setRenderStyle( Common::VSP_DRAW_TEXTURED_TRANSPARENT_BACK );

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

        VSPGraphic::Probe * probe;
        VSPGraphic::Ruler * ruler;
        VSPGraphic::Protractor * protractor;
        VSPGraphic::Routing * routing;

        switch( objects[i]->m_Type )
        {
        case DrawObj::VSP_PROTRACTOR:
            if( id == 0xFFFFFFFF )
            {
                m_GEngine->getScene()->createObject( Common::VSP_OBJECT_PROTRACTOR, &id );

                ID idInfo;
                idInfo.bufferID = id;
                idInfo.geomID = objects[i]->m_GeomID;
                m_idMap[ idInfo.geomID ] = idInfo;
            }
            protractor = dynamic_cast<VSPGraphic::Protractor*> ( m_GEngine->getScene()->getObject( id ) );
            if( protractor )
            {
                protractor->setVisibility( objects[i]->m_Visible );

                protractor->setTextColor( (float)objects[i]->m_TextColor.x(),
                    (float)objects[i]->m_TextColor.y(),
                    (float)objects[i]->m_TextColor.z() );

                protractor->setTextSize( (float)objects[i]->m_TextSize );

                glm::vec3 start, mid, end, label_dir;

                switch( objects[i]->m_Protractor.Step )
                {
                case DrawObj::VSP_PROTRACTOR_STEP_ZERO:
                    protractor->reset();
                    break;

                case DrawObj::VSP_PROTRACTOR_STEP_ONE:
                    start = glm::vec3( objects[i]->m_Protractor.Start.x(),
                        objects[i]->m_Protractor.Start.y(),
                        objects[i]->m_Protractor.Start.z() );
                    protractor->placeProtractor( start );
                    break;

                case DrawObj::VSP_PROTRACTOR_STEP_TWO:
                    start = glm::vec3( objects[i]->m_Protractor.Start.x(),
                        objects[i]->m_Protractor.Start.y(),
                        objects[i]->m_Protractor.Start.z() );
                    mid = glm::vec3( objects[i]->m_Protractor.Mid.x(),
                        objects[i]->m_Protractor.Mid.y(),
                        objects[i]->m_Protractor.Mid.z() );
                    protractor->placeProtractor( start, mid );
                    break;

                case DrawObj::VSP_PROTRACTOR_STEP_THREE:
                    start = glm::vec3( objects[i]->m_Protractor.Start.x(),
                        objects[i]->m_Protractor.Start.y(),
                        objects[i]->m_Protractor.Start.z() );
                    mid = glm::vec3( objects[i]->m_Protractor.Mid.x(),
                        objects[i]->m_Protractor.Mid.y(),
                        objects[i]->m_Protractor.Mid.z() );
                    end = glm::vec3( objects[i]->m_Protractor.End.x(),
                        objects[i]->m_Protractor.End.y(),
                        objects[i]->m_Protractor.End.z() );
                    label_dir = glm::vec3( objects[i]->m_Protractor.Label_Dir.x(),
                                           objects[i]->m_Protractor.Label_Dir.y(),
                                           objects[i]->m_Protractor.Label_Dir.z() );
                    protractor->placeProtractor( start, mid, end, label_dir, objects[i]->m_Protractor.Label );
                    break;

                case DrawObj::VSP_PROTRACTOR_STEP_COMPLETE:
                    start = glm::vec3( objects[i]->m_Protractor.Start.x(),
                        objects[i]->m_Protractor.Start.y(),
                        objects[i]->m_Protractor.Start.z() );
                    mid = glm::vec3( objects[i]->m_Protractor.Mid.x(),
                        objects[i]->m_Protractor.Mid.y(),
                        objects[i]->m_Protractor.Mid.z() );
                    end = glm::vec3( objects[i]->m_Protractor.End.x(),
                        objects[i]->m_Protractor.End.y(),
                        objects[i]->m_Protractor.End.z() );
                    label_dir = glm::vec3( objects[i]->m_Protractor.Label_Dir.x(),
                                           objects[i]->m_Protractor.Label_Dir.y(),
                                           objects[i]->m_Protractor.Label_Dir.z() );
                    protractor->placeProtractor( start, mid, end, label_dir, objects[i]->m_Protractor.Offset, objects[i]->m_Protractor.Label, objects[i]->m_Protractor.Dir );
                    break;

                default:
                    break;
                }
            }
            break;

        case DrawObj::VSP_RULER:
            if( id == 0xFFFFFFFF )
            {
                m_GEngine->getScene()->createObject( Common::VSP_OBJECT_RULER, &id );

                ID idInfo;
                idInfo.bufferID = id;
                idInfo.geomID = objects[i]->m_GeomID;
                m_idMap[ idInfo.geomID ] = idInfo;
            }
            ruler = dynamic_cast<VSPGraphic::Ruler*> ( m_GEngine->getScene()->getObject( id ) );
            if( ruler )
            {
                ruler->setVisibility( objects[i]->m_Visible );

                ruler->setTextColor( (float)objects[i]->m_TextColor.x(),
                    (float)objects[i]->m_TextColor.y(),
                    (float)objects[i]->m_TextColor.z() );

                ruler->setTextSize( (float)objects[i]->m_TextSize );

                glm::vec3 start, end, offset;

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
                    ruler->placeRuler( start, end, objects[i]->m_Ruler.Label );
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
                    ruler->placeRuler( start, end, offset, objects[i]->m_Ruler.Label, objects[i]->m_Ruler.Dir );
                    break;

                default:
                    break;
                }
            }
            break;

        case DrawObj::VSP_PROBE:
            if( id == 0xFFFFFFFF )
            {
                m_GEngine->getScene()->createObject( Common::VSP_OBJECT_PROBE, &id );

                ID idInfo;
                idInfo.bufferID = id;
                idInfo.geomID = objects[i]->m_GeomID;
                m_idMap[ idInfo.geomID ] = idInfo;
            }
            probe = dynamic_cast<VSPGraphic::Probe*> ( m_GEngine->getScene()->getObject( id ) );
            if( probe )
            {
                probe->setVisibility( objects[i]->m_Visible );

                probe->setTextColor( (float)objects[i]->m_TextColor.x(),
                    (float)objects[i]->m_TextColor.y(),
                    (float)objects[i]->m_TextColor.z() );

                probe->setTextSize( (float)objects[i]->m_TextSize );

                glm::vec3 pt, norm;
                float len;

                switch( objects[i]->m_Probe.Step )
                {
                case DrawObj::VSP_PROBE_STEP_ZERO:
                    probe->reset();
                    break;

                case DrawObj::VSP_PROBE_STEP_ONE:
                    pt = glm::vec3( objects[i]->m_Probe.Pt.x(),
                        objects[i]->m_Probe.Pt.y(),
                        objects[i]->m_Probe.Pt.z() );
                    norm = glm::vec3( objects[i]->m_Probe.Norm.x(),
                        objects[i]->m_Probe.Norm.y(),
                        objects[i]->m_Probe.Norm.z() );
                    probe->placeProbe( pt, norm, objects[i]->m_Probe.Label );
                    break;

                case DrawObj::VSP_PROBE_STEP_COMPLETE:
                    pt = glm::vec3( objects[i]->m_Probe.Pt.x(),
                        objects[i]->m_Probe.Pt.y(),
                        objects[i]->m_Probe.Pt.z() );
                    norm = glm::vec3( objects[i]->m_Probe.Norm.x(),
                        objects[i]->m_Probe.Norm.y(),
                        objects[i]->m_Probe.Norm.z() );
                    len = objects[i]->m_Probe.Len;
                    probe->placeProbe( pt, norm, len, objects[i]->m_Probe.Label );
                    break;

                default:
                    break;
                }
            }
            break;

        case DrawObj::VSP_ROUTING:
            if( id == 0xFFFFFFFF )
            {
                m_GEngine->getScene()->createObject( Common::VSP_OBJECT_ROUTING, &id );

                ID idInfo;
                idInfo.bufferID = id;
                idInfo.geomID = objects[i]->m_GeomID;
                m_idMap[ idInfo.geomID ] = idInfo;
            }
            routing = dynamic_cast<VSPGraphic::Routing*> ( m_GEngine->getScene()->getObject( id ) );
            if( ruler )
            {
                routing->setVisibility( objects[i]->m_Visible );

                routing->setLiveIndex( objects[i]->m_Routing.LiveIndex );

                if( objects[i]->m_GeomChanged )
                {
                    routing->_v.clear();
                    int npt = objects[i]->m_PntVec.size();
                    routing->_v.reserve( npt );

                    for ( int j = 0; j < npt; j++ )
                    {
                        vec3d &pt = objects[i]->m_PntVec[j];
                        routing->_v.push_back( glm::vec3( pt.v[0], pt.v[1], pt.v[2] ) );
                    }

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
                    m_idMap[ idInfo.geomID ] = idInfo;
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
                    m_idMap[ idInfo.geomID ] = idInfo;
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
                m_idMap[ idInfo.geomID ] = idInfo;
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

        int stippleFactor;
        int stipplePattern;
        bool stippleFlag;

        red = ( float )objects[i]->m_LineColor.x();
        green = ( float )objects[i]->m_LineColor.y();
        blue = ( float )objects[i]->m_LineColor.z();

        lineWidth = ( float )objects[i]->m_LineWidth;

        stippleFactor = objects[i]->m_StippleFactor;
        stipplePattern = objects[i]->m_StipplePattern;
        stippleFlag = objects[i]->m_StippleFlag;

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
                m_idMap[ idInfo.geomID ] = idInfo;
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
                m_idMap[ idInfo.geomID ] = idInfo;
            }

            rObj = dynamic_cast<Renderable*> ( m_GEngine->getScene()->getObject( id ) );
            if( rObj )
            {
                rObj->setVisibility( objects[i]->m_Visible );
                rObj->setPrimType( VSPGraphic::Common::VSP_LINES );
                rObj->setLineColor( red, green, blue );
                rObj->setLineWidth( lineWidth );
                rObj->setStippleFactor( stippleFactor );
                rObj->setStipplePattern( stipplePattern );
                rObj->setStippleFlag( stippleFlag );

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
                m_idMap[ idInfo.geomID ] = idInfo;
            }
            rObj = dynamic_cast<Renderable*> ( m_GEngine->getScene()->getObject( id ) );
            if( rObj )
            {
                rObj->setVisibility( objects[i]->m_Visible );
                rObj->setPrimType( VSPGraphic::Common::VSP_LINE_LOOP );
                rObj->setLineColor( red, green, blue );
                rObj->setLineWidth( lineWidth );
                rObj->setStippleFactor( stippleFactor );
                rObj->setStipplePattern( stipplePattern );
                rObj->setStippleFlag( stippleFlag );

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
                m_idMap[ idInfo.geomID ] = idInfo;
            }
            rObj = dynamic_cast<Renderable*> ( m_GEngine->getScene()->getObject( id ) );
            if( rObj )
            {
                rObj->setVisibility( objects[i]->m_Visible );
                rObj->setPrimType( VSPGraphic::Common::VSP_LINE_STRIP );
                rObj->setLineColor( red, green, blue );
                rObj->setLineWidth( lineWidth );
                rObj->setStippleFactor( stippleFactor );
                rObj->setStipplePattern( stipplePattern );
                rObj->setStippleFlag( stippleFlag );

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

VspGlWindow::ID * VspGlWindow::_findID( const std::string &geomID )
{
    auto it = m_idMap.find( geomID );
    if( it == m_idMap.end() )
    {
        return nullptr;
    }
    return &( it->second );
}

VspGlWindow::ID * VspGlWindow::_findID( unsigned int bufferID )
{
    for ( auto it = m_idMap.begin(); it != m_idMap.end(); ++it )
    {
        if ( it->second.bufferID ==  bufferID )
        {
            return &( it->second );
        }
    }
    return nullptr;
}

void VspGlWindow::_updateBuffer( const std::vector<DrawObj *> &objects )
{
    std::unordered_map< std::string, DrawObj * > objects_map;
    for( int j = 0; j < ( int )objects.size(); j++ )
    {
        objects_map[ objects[j]->m_GeomID ] = objects[j];
    }

    // Copy map ID's into vector so we can iterate on vector while removing from map.
    std::vector < std::string > idvec;
    idvec.reserve( m_idMap.size() );
    for ( auto mit = m_idMap.begin(); mit != m_idMap.end(); ++mit )
    {
        idvec.push_back( mit->first );
    }

    // Figure out if any buffer object no longer exists in DrawObjs.
    for ( int i = 0; i < idvec.size(); i++ )
    {
        const string &id = idvec[i];
        if ( id != std::string( "Default" ) )
        {
            auto it = objects_map.find( id );
            if( it == objects_map.end() )
            {
                m_GEngine->getScene()->removeObject( m_idMap[ id ].bufferID );
                m_idMap.erase( id );
            }
        }
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

            id->textureIDs = texToKeep;

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
                    newTex.bufferTexID = entity->getTextureMgr()->add(
                            VSPGraphic::GlobalTextureRepo()->get2DTexture(
                                    drawObjTexList[i].FileName.c_str(),
                                    drawObjTexList[i].ModificationKey,
                                    drawObjTexList[i].Rot,
                                    drawObjTexList[i].FlipLR,
                                    drawObjTexList[i].FlipUD,
                                    drawObjTexList[i].AutoTrans ) );
                    id->textureIDs.push_back( newTex );

                    texBufferID = newTex.bufferTexID;
                }

                float UScale = ( float )( 1 / drawObjTexList[i].UScale );
                float WScale = ( float )( 1 / drawObjTexList[i].WScale );
                float U = ( float )( drawObjTexList[i].U * -1.f * UScale );
                float W = ( float )( drawObjTexList[i].W * WScale );

                // Update Texture Properties.
                entity->getTextureMgr()->translate( texBufferID, U, W );
                entity->getTextureMgr()->scale( texBufferID, UScale, WScale );
                entity->getTextureMgr()->transparentize( texBufferID, drawObjTexList[ i ].Transparency, drawObjTexList[ i ].BlendTransparency );
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

    if ( drawObj->m_NormMesh.size() != num_mesh || drawObj->m_uTexMesh.size() != num_mesh || drawObj->m_vTexMesh.size() != num_mesh )
    {
        destObj->emptyVBuffer();
        destObj->emptyEBuffer();
        destObj->enableEBuffer( false );
        return;
    }

    unsigned int vtotal = 0;
    unsigned int etotal = 0;

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

    unsigned int offset = 0;

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

    unsigned int n = drawObj->m_PntVec.size();

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

    if ( drawObj->m_WireColorVec.size() == drawObj->m_PntVec.size() )
    {
        std::vector<float> cdata( n * 4, 0.0f );

        for ( int i = 0; i < ( int )drawObj->m_WireColorVec.size(); i++ )
        {
            const int j = i * 4;
            cdata[ j + 0 ] = ( float )drawObj->m_WireColorVec[i].x();
            cdata[ j + 1 ] = ( float )drawObj->m_WireColorVec[i].y();
            cdata[ j + 2 ] = ( float )drawObj->m_WireColorVec[i].z();

            cdata[ j + 3 ] = ( float )drawObj->m_WireAlphaVec[i];
        }

        destObj->emptyLineCBuffer();
        destObj->appendLineCBuffer( cdata.data(), sizeof( float ) * cdata.size());
        destObj->enableLineCBuffer( true );
    }
    else
    {
        destObj->emptyLineCBuffer();
        destObj->enableLineCBuffer( false );
    }

    if ( drawObj->m_FaceColorVec.size() == drawObj->m_PntVec.size() )
    {
        std::vector<float> cdata( n * 4, 0.0f );

        for ( int i = 0; i < ( int )drawObj->m_FaceColorVec.size(); i++ )
        {
            const int j = i * 4;
            cdata[ j + 0 ] = ( float )drawObj->m_FaceColorVec[i].x();
            cdata[ j + 1 ] = ( float )drawObj->m_FaceColorVec[i].y();
            cdata[ j + 2 ] = ( float )drawObj->m_FaceColorVec[i].z();

            cdata[ j + 3 ] = ( float )drawObj->m_FaceAlphaVec[i];
        }

        destObj->emptyMeshCBuffer();
        destObj->appendMeshCBuffer( cdata.data(), sizeof( float ) * cdata.size());
        destObj->enableMeshCBuffer( true );
    }
    else
    {
        destObj->emptyMeshCBuffer();
        destObj->enableMeshCBuffer( false );
    }
}

// Tri and Quad data is loaded identically.  Differences are actually based on
// the way the data is stored in the drawObj (vector or mesh) and whether
// it goes into a vertex buffer or a vertex and edge buffer.
// In addition, the amount of data -- vert, vert & norm, or vert, norm & texture.
void VspGlWindow::_loadQuadsData( Renderable * destObj, DrawObj * drawObj )
{
    _loadTrisData( destObj, drawObj );
}

void VspGlWindow::_loadMarkData( Renderable * destObj, DrawObj * drawObj )
{
    unsigned int n = drawObj->m_PntVec.size();
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

    for( unsigned int i = 0; i < (int)drawObj->m_LightingInfos.size(); i++ )
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
    int ivp = display->selectViewport( x, y );
    Vehicle* vPtr = VehicleMgr.GetVehicle();

    if ( ivp != m_prevViewport )
    {
        UpdateAllViewParms();
        m_prevViewport = ivp;
    }

    if( Fl::event_button1() && Fl::event_button3() )
    {
        // LB + RB 
        m_prevLBRB = glm::vec2( x, y );
    }
    else if( Fl::event_button1() )
    {
        bool alltrue = false;

        if( Fl::event_shift() || vPtr->m_SelectBoxFlag() )
        {
            FitModelScreen * fitModelScreen = dynamic_cast< FitModelScreen* >
                ( m_ScreenMgr->GetScreen( vsp::VSP_FIT_MODEL_SCREEN ) );
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
            glm::vec3 mouseInWorld = glm::vec3( fNAN );

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
            else if( !glm::any(glm::isnan(mouseInWorld)) &&
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
    Vehicle* vPtr = VehicleMgr.GetVehicle();

    if( Fl::event_button1() && Fl::event_button3() )
    {
        // LB + RB 
        if( !glm::any(glm::isnan(m_prevLBRB)) )
        {
            display->zoom( ( int )m_prevLBRB.x, ( int )m_prevLBRB.y, x, y );

            ManageViewScreen * viewScreen = dynamic_cast< ManageViewScreen* >
            ( m_ScreenMgr->GetScreen( vsp::VSP_VIEW_SCREEN ) );

            UpdateZoomParms();
        }
        m_prevLBRB = glm::vec2( x, y );
    }
    else if( Fl::event_button1() )
    {
        bool alltrue = false;
        if( Fl::event_shift() || vPtr->m_SelectBoxFlag() )
        {
            FitModelScreen * fitModelScreen = dynamic_cast< FitModelScreen* >
                ( m_ScreenMgr->GetScreen( vsp::VSP_FIT_MODEL_SCREEN ) );
            if ( fitModelScreen )
            {
                if (fitModelScreen->IsShown() )
                {
                    if( glm::any(glm::isnan(m_startShiftLB)) )
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
            if( !glm::any(glm::isnan(m_prevAltLB)) )
            {
                display->pan( ( int )m_prevAltLB.x, ( int )m_prevAltLB.y, x, y );

                ManageViewScreen * viewScreen = dynamic_cast< ManageViewScreen* >
                ( m_ScreenMgr->GetScreen( vsp::VSP_VIEW_SCREEN ) );

                UpdatePanParms();
            }
            m_prevAltLB = glm::vec2( x, y );
        }
        else if( Fl::event_ctrl() )
        {
            // Ctrl + LB
            if( !glm::any(glm::isnan(m_prevCtrlLB)) )
            {
                display->zoom( ( int )m_prevCtrlLB.x, ( int )m_prevCtrlLB.y, x, y );

                ManageViewScreen * viewScreen = dynamic_cast< ManageViewScreen* >
                ( m_ScreenMgr->GetScreen( vsp::VSP_VIEW_SCREEN ) );

                UpdateZoomParms();
            }
            m_prevCtrlLB = glm::vec2( x, y );
        }
        else if( Fl::event_state( FL_META ) )
        {
            if( !glm::any(glm::isnan(m_prevMetaLB)) )
            {
                display->zoom( ( int )m_prevMetaLB.x, ( int )m_prevMetaLB.y, x, y );

                ManageViewScreen * viewScreen = dynamic_cast< ManageViewScreen* >
                ( m_ScreenMgr->GetScreen( vsp::VSP_VIEW_SCREEN ) );

                UpdateZoomParms();
            }
            m_prevMetaLB = glm::vec2( x, y );
        }
        else if ( !m_noRotate )
        {
            // LB
            if( !glm::any(glm::isnan(m_prevLB)) )
            {
                display->rotate( ( int )m_prevLB.x, ( int )m_prevLB.y, x, y );

                ManageViewScreen * viewScreen = dynamic_cast< ManageViewScreen* >
                ( m_ScreenMgr->GetScreen( vsp::VSP_VIEW_SCREEN ) );

                UpdateRotationParms();
            }
            m_prevLB = glm::vec2( x, y );
        }
    }
    else if( Fl::event_button2() )
    {
        // MB
        if( !glm::any(glm::isnan(m_prevMB)) )
        {
            display->zoom( ( int )m_prevMB.x, ( int )m_prevMB.y, x, y );

            ManageViewScreen * viewScreen = dynamic_cast< ManageViewScreen* >
            ( m_ScreenMgr->GetScreen( vsp::VSP_VIEW_SCREEN ) );

            UpdateZoomParms();
        }
        m_prevMB = glm::vec2( x, y );
    }
    else if( Fl::event_button3() )
    {
        // RB
        if( !glm::any(glm::isnan(m_prevRB)) )
        {
            display->pan( ( int )m_prevRB.x, ( int )m_prevRB.y, x, y );

            ManageViewScreen * viewScreen = dynamic_cast< ManageViewScreen* >
            ( m_ScreenMgr->GetScreen( vsp::VSP_VIEW_SCREEN ) );

            UpdatePanParms();
        }
        m_prevRB = glm::vec2( x, y );
    }
    redraw();
}

void VspGlWindow::OnRelease( int x, int y )
{
    VSPGraphic::Display * display = m_GEngine->getDisplay();
    Vehicle* vPtr = VehicleMgr.GetVehicle();

    // Reset buttons positions.
    switch( Fl::event_button() )
    {
    case FL_LEFT_MOUSE:
        m_prevLB = m_prevAltLB = m_prevCtrlLB = glm::vec2( fNAN );
        m_prevMetaLB = glm::vec2( fNAN );
        if( Fl::event_shift() || vPtr->m_SelectBoxFlag() )
        {
            FitModelScreen * fitModelScreen = dynamic_cast< FitModelScreen* >
                ( m_ScreenMgr->GetScreen( vsp::VSP_FIT_MODEL_SCREEN ) );
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
        m_startShiftLB = glm::vec2( fNAN );
        display->getLayoutMgr()->setStartXY( -1, -1 );

        break;

    case FL_RIGHT_MOUSE:
        m_prevRB = glm::vec2( fNAN );
        break;

    case FL_MIDDLE_MOUSE:
        m_prevMB = glm::vec2( fNAN );
        break;
    }
    redraw();
}

int VspGlWindow::OnKeyup( int x, int y )
{
    VSPGraphic::Display * display = m_GEngine->getDisplay();

    ManageViewScreen * viewScreen = dynamic_cast< ManageViewScreen* >
    ( m_ScreenMgr->GetScreen( vsp::VSP_VIEW_SCREEN ) );

    int handled = 0;

    switch( Fl::event_key() )
    {
    case FL_Escape:
    {
        Vehicle* vPtr = VehicleMgr.GetVehicle();
        if ( vPtr )
        {
            vector< string > none;
            vPtr->SetActiveGeomVec( none );
            m_ScreenMgr->SetUpdateFlag( true );
        }
        handled = 1;
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
            UpdateAllViewParms();

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

            UpdateAllViewParms();
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

            UpdateAllViewParms();
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
            UpdateAllViewParms();
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
            m_prevAltLB = glm::vec2( fNAN );
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
            m_prevCtrlLB = glm::vec2( fNAN );
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
            m_prevCtrlLB = glm::vec2( fNAN );
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
    int handled = 0;

    switch( Fl::event_key() )
    {
    }
    redraw();

    return handled;
}

int VspGlWindow::OnWheelScroll( int dx, int dy, int x, int y )
{
    int handled = 1;

    float zoomChange = 1;
    float zoomSpeed = 1.15; // size increase when zooming in

    if ( dy > 0 ) // wheel pulled toward user
    {
        zoomChange = zoomSpeed;
    }
    else if ( dy < 0 ) // wheel pushed away from user
    {
        zoomChange = 1 / zoomSpeed;
    }

    if ( zoomChange != 1 )
    {
        Viewport* viewport = m_GEngine->getDisplay()->getViewport();
        float width = viewport->width();
        float height = viewport->height();
        int mx = x - viewport->x();
        int my = y - viewport->y();

        float zoom = getRelativeZoomValue();
        glm::vec2 pan = getPanValues();

        float mousePosBeforeX, mousePosBeforeY;
        mousePosBeforeX = ( ( ( zoom * width ) * ( ( mx / width ) - 0.5 ) - pan.x ) );
        mousePosBeforeY = ( ( ( zoom * height ) * ( ( my / height ) - 0.5 ) - pan.y ) );

        relativeZoom( getRelativeZoomValue() * zoomChange );

        zoom *= zoomChange;
        pan *= zoomChange;

        float mousePosAfterX, mousePosAfterY;
        mousePosAfterX = ( ( ( zoom * width ) * ( ( mx / width ) - 0.5 ) - pan.x ) );
        mousePosAfterY = ( ( ( zoom * height ) * ( ( my / height ) - 0.5 ) - pan.y )  );

        // cancel out any effect the zoom had on pan position, so that the same location remains under the mouse
        relativePan( pan.x + mousePosAfterX - mousePosBeforeX , pan.y + mousePosAfterY - mousePosBeforeY );

        ManageViewScreen * viewScreen = dynamic_cast< ManageViewScreen* > ( m_ScreenMgr->GetScreen( vsp::VSP_VIEW_SCREEN ) );

        UpdateZoomParms();
        UpdatePanParms();

        redraw();
    }

    return handled;
}

void VspGlWindow::_sendFeedback( Selectable * selected )
{
    // Find out where feedback is heading...
    std::string selectedFeedbackName = selected->getGroup();

    // Probe Screen Feedback
    ManageMeasureScreen * measureScreen = dynamic_cast<ManageMeasureScreen*>
    ( m_ScreenMgr->GetScreen( vsp::VSP_MEASURE_SCREEN ) );

    if( measureScreen && measureScreen->getFeedbackGroupName() == selectedFeedbackName )
    {
        // Location feedback
        SelectedLoc * loc = dynamic_cast<SelectedLoc*> ( selected );
        if( loc )
        {
            glm::vec3 placement = loc->getLoc();
            measureScreen->Set( vec3d( placement.x, placement.y, placement.z ) );

            // Only one selection is needed for label, remove this 'selected' from selection list.
            m_GEngine->getScene()->removeSelected( selected );
            selected = nullptr;
        }

        // Vertex feedback
        // Cast selectable to SelectedPnt object, so that we can get Render Source Ptr.
        SelectedPnt * pnt = dynamic_cast<SelectedPnt*>( selected );
        if( pnt )
        {
            VSPGraphic::Renderable * e = dynamic_cast<VSPGraphic::Renderable*>(pnt->getSource());
            if(e)
            {
                ID * id = _findID( e->getID() );
                if( id )
                {
                    unsigned int index = id->geomID.find_first_of( '_' );
                    std::string baseId = id->geomID.substr( 0, index );
                    glm::vec3 placement = e->getVertexVec(pnt->getIndex());
                    measureScreen->Set( vec3d( placement.x, placement.y, placement.z ), baseId );

                    // Only one selection is needed for label, remove this 'selected' from selection list.
                    m_GEngine->getScene()->removeSelected( selected );
                    selected = nullptr;
                }
            }
        }
    }

    // Geom Screen Feedback
    ManageGeomScreen * geomScreen = dynamic_cast<ManageGeomScreen *>
        ( m_ScreenMgr->GetScreen( vsp::VSP_MANAGE_GEOM_SCREEN ) );

    if( geomScreen && geomScreen->getFeedbackGroupName() == selectedFeedbackName )
    {
        SelectedGeom * geom = dynamic_cast<SelectedGeom*>( selected );
        if( geom )
        {
            ID * id = _findID( geom->getSource()->getID() );
            if( id )
            {
                unsigned int index = id->geomID.find_last_of( '_' );
                std::string baseId = id->geomID.substr( 0, index );
                geomScreen->Set( baseId );

                // Only one selection is needed for Geom, remove this 'selected' from selection list.
                m_GEngine->getScene()->removeSelected( selected );
                selected = nullptr;
            }
        }
        // Mac Fix:  On Mac, FLTK window is always focus on the last shown GUI.  In
        // this case where a geometry is selected, geometry's window becomes the new
        // focus.  The behavior locks up geometry selection process.  Set OpenGL
        // window back on focus so user can proceed without interruption.
        focus(this);
    }

    // Routing Screen Feedback
    if ( geomScreen )
    {
        RoutingScreen * routingScreen = dynamic_cast<RoutingScreen*>
            ( geomScreen->GetGeomScreen( vsp::ROUTING_GEOM_SCREEN ) );

        if( routingScreen && routingScreen->getFeedbackGroupName() == selectedFeedbackName )
        {
            // Vertex feedback
            // Cast selectable to SelectedPnt object, so that we can get Render Source Ptr.
            SelectedPnt * pnt = dynamic_cast<SelectedPnt*>( selected );
            if( pnt )
            {
                VSPGraphic::Renderable * e = dynamic_cast<VSPGraphic::Renderable*>(pnt->getSource());
                if(e)
                {
                    ID * id = _findID( e->getID() );
                    if( id )
                    {
                        unsigned int index = id->geomID.find_first_of( '_' );
                        std::string baseId = id->geomID.substr( 0, index );
                        glm::vec3 placement = e->getVertexVec(pnt->getIndex());
                        routingScreen->Set( vec3d( placement.x, placement.y, placement.z ), baseId );

                        // Only one selection is needed for label, remove this 'selected' from selection list.
                        m_GEngine->getScene()->removeSelected( selected );
                        selected = nullptr;
                    }
                }
            }
        }
    }

    // Center of rotation Screen Feedback
    ManageCORScreen * corScreen = dynamic_cast<ManageCORScreen*>
        ( m_ScreenMgr->GetScreen( vsp::VSP_COR_SCREEN ) );

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
                m_GEngine->getDisplay()->center();

                UpdateCORParms();
                UpdatePanParms();

                // This is a dummy call to let corScreen know the job is done.
                corScreen->Set();

                // Only one selection is needed for Center of Rotation, remove this 'selected' from selection list.
                m_GEngine->getScene()->removeSelected( selected );
                selected = nullptr;
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
                selected = nullptr;
            }
        }
    }
}

void VspGlWindow::_sendFeedback( const std::vector<Selectable *> &listOfSelected )
{
    for ( int i = 0; i < (int) listOfSelected.size(); i++ )
    {
        _sendFeedback( listOfSelected[i] );
    }
}

//**********************************************************************//
//*************************** EditXSecWindow ***************************//
//**********************************************************************//

EditXSecWindow::EditXSecWindow( int x, int y, int w, int h, ScreenMgr* mgr )
    : VspGlWindow( x, y, w, h, mgr, DrawObj::VSP_EDIT_CURVE_SCREEN )
{
    m_LastHit = -1;
    m_noRotate = true;
}

void EditXSecWindow::update()
{
    Vehicle* vPtr = VehicleMgr.GetVehicle();

    if ( vPtr )
    {
        make_current();

        // Get Render Objects from Vehicle.
        vector<DrawObj*> drawObjs;
        vPtr->LoadDrawObjs( drawObjs );

        // Load Render Objects for Curve Edit Screen
        CurveEditScreen* curve_editor = dynamic_cast <CurveEditScreen*>
            ( m_ScreenMgr->GetScreen( vsp::VSP_CURVE_EDIT_SCREEN ) );
        if ( curve_editor )
        {
            curve_editor->LoadDrawObjs( drawObjs );
        }

        // Load Objects to Renderer.
        _update( drawObjs ); // VspGlWindow
    }
}

int EditXSecWindow::handle( int fl_event )
{
    int ret_val = VspGlWindow::handle( fl_event );

    CurveEditScreen* curve_editor = dynamic_cast <CurveEditScreen*>
        ( m_ScreenMgr->GetScreen( vsp::VSP_CURVE_EDIT_SCREEN ) );
    if ( !curve_editor )
    {
        return ret_val;
    }

    XSecCurve* xsc = curve_editor->GetXSecCurve();

    if ( !xsc || xsc->GetType() != vsp::XS_EDIT_CURVE )
    {
        return ret_val;
    }

    EditCurveXSec* edit_curve_xs = dynamic_cast<EditCurveXSec*>( xsc );
    assert( edit_curve_xs );


    int izero = -1;
    if ( edit_curve_xs->m_View() == vsp::VIEW_FRONT ) // X,Y
    {
        izero = 2;
    }
    else if ( edit_curve_xs->m_View() == vsp::VIEW_TOP ) // X,Z
    {
        izero = 1;
    }
    else if ( edit_curve_xs->m_View() == vsp::VIEW_LEFT ) // Z,Y
    {
        izero = 0;
    }


    if ( fl_event == FL_PUSH && !Fl::event_button2() )
    {
        vec3d coord = PixelToCoord( m_mouse_x, m_mouse_y );

        vec3d scalefac( 1, 1, 1 );
        if ( !edit_curve_xs->m_AbsoluteFlag() )
        {

            scalefac.scale_x( edit_curve_xs->GetWidth() );
            scalefac.scale_y( edit_curve_xs->GetHeight() );
            scalefac.scale_z( edit_curve_xs->m_Depth() );
            scalefac[izero] = 0;
        }

        // Diameter of point + 10% considered "hit"
        double hit_r = pixels_per_unit() * 1.2 * edit_curve_xs->m_XSecPointSize.Get() / 2;

        float zoom = getRelativeZoomValue();
        m_LastHit = ihit( coord, hit_r * zoom, scalefac );

        if ( m_LastHit != -1 )
        {
            edit_curve_xs->SetSelectPntID( m_LastHit );
            curve_editor->UpdateIndexSelector( m_LastHit, false );
        }

        if ( curve_editor->GetDeleteActive() )
        {
            curve_editor->SetDeleteActive( false );

            if ( m_LastHit >= 0 )
            {
                edit_curve_xs->DeletePt( m_LastHit );
            }
        }

        if ( curve_editor->GetSplitActive() )
        {
            curve_editor->SetSplitActive( false );

            // Scale by width and height
            vec3d split_pnt = coord;
            if ( !edit_curve_xs->m_AbsoluteFlag() )
            {
                split_pnt.scale_x( edit_curve_xs->GetWidth() );
                split_pnt.scale_y( edit_curve_xs->GetHeight() );
                split_pnt.scale_z( edit_curve_xs->m_Depth() );
            }

            double u_curve;
            VspCurve base_curve = edit_curve_xs->GetBaseEditCurve();

            base_curve.OffsetX( -0.5 * edit_curve_xs->GetWidth() );
            base_curve.ZeroI( izero );
            base_curve.FindNearest01( u_curve, split_pnt );

            edit_curve_xs->m_SplitU = u_curve;
            int new_pnt = edit_curve_xs->Split01();
            curve_editor->UpdateIndexSelector( new_pnt );
        }
    }
    else if ( fl_event == FL_DRAG && m_LastHit != -1 && !Fl::event_button2() )
    {
        vec3d coord = PixelToCoord( m_mouse_x, m_mouse_y );

        // Scale by width and height
        vec3d move_pnt = coord;
        if ( edit_curve_xs->m_AbsoluteFlag() )
        {
            move_pnt.scale_x( 1.0 / edit_curve_xs->GetWidth() );
            move_pnt.scale_y( 1.0 / edit_curve_xs->GetHeight() );
            move_pnt.scale_z( 1.0 / edit_curve_xs->m_Depth() );
        }

        edit_curve_xs->MovePnt( move_pnt, izero );

        curve_editor->SetFreezeAxis( true );
    }
    else if ( fl_event == FL_RELEASE )
    {
        curve_editor->SetFreezeAxis( false );
        // Force update the parent Geom once the point is released
        edit_curve_xs->ParmChanged( nullptr, Parm::SET_FROM_DEVICE );
    }

    if ( fl_event == FL_RELEASE || fl_event == FL_DRAG || fl_event == FL_PUSH )
    {
        curve_editor->Update();
    }

    return ret_val;
}


vec3d EditXSecWindow::PixelToCoord( int x_pix, int y_pix )
{
    vec3d coord;

    //==== Find EditCurveXSec Ptr ====//
    CurveEditScreen* curve_editor = dynamic_cast <CurveEditScreen*>
        ( m_ScreenMgr->GetScreen( vsp::VSP_CURVE_EDIT_SCREEN ) );
    if ( !curve_editor )
    {
        return coord;
    }

    XSecCurve* xsc = curve_editor->GetXSecCurve();

    if ( !xsc || xsc->GetType() != vsp::XS_EDIT_CURVE )
    {
        return coord;
    }

    EditCurveXSec* edit_curve_xs = dynamic_cast<EditCurveXSec*>( xsc );
    assert( edit_curve_xs );

    float zoom = getRelativeZoomValue();
    glm::vec2 pan = getPanValues();
    double w = pixel_w();
    double h = pixel_h();

    // Convert pixels to normalized coordinates
    double data1 = ( zoom * w ) * ( ( x_pix / w ) - 0.5 ) - pan.x;
    double data2 = ( zoom * h ) * ( ( y_pix / h ) - 0.5 ) - pan.y;

    // Modulate 2D projection into 3D vector
    if ( edit_curve_xs->m_View() == vsp::VIEW_FRONT ) // X,Y
    {
        coord.set_xyz( data1, data2, 0.0 );
    }
    else if ( edit_curve_xs->m_View() == vsp::VIEW_TOP ) // X,Z
    {
        coord.set_xyz( data1, 0.0, data2 );
    }
    else if ( edit_curve_xs->m_View() == vsp::VIEW_LEFT ) // Z,Y
    {
        coord.set_xyz( 0.0, data2, data1 );
    }

    // Non-dimensionalize coordinates if needed.
    if ( !edit_curve_xs->m_AbsoluteFlag() )
    {
        coord.scale_x( 1.0 / edit_curve_xs->GetWidth() );
        coord.scale_y( 1.0 / edit_curve_xs->GetHeight() );
        coord.scale_z( 1.0 / edit_curve_xs->m_Depth() );
    }

    return coord;
}

int EditXSecWindow::ihit( const vec3d & mpt, double r_test, const vec3d &scale )
{
    //==== Find EditCurveXSec Ptr ====//
    CurveEditScreen* curve_editor = dynamic_cast <CurveEditScreen*>
        ( m_ScreenMgr->GetScreen( vsp::VSP_CURVE_EDIT_SCREEN ) );
    if ( !curve_editor )
    {
        return -1;
    }

    XSecCurve* xsc = curve_editor->GetXSecCurve();

    if ( !xsc || xsc->GetType() != vsp::XS_EDIT_CURVE )
    {
        return -1;
    }

    EditCurveXSec* edit_curve_xs = dynamic_cast<EditCurveXSec*>( xsc );
    assert( edit_curve_xs );

    bool symflag = ( edit_curve_xs->m_SymType() == vsp::SYM_RL );

    vector < vec3d > control_pts = edit_curve_xs->GetCtrlPntVec( false );
    int ndata = (int)control_pts.size();

    vector < vec3d > cpproj = proj_pt_vec( control_pts );

    double min_dist = 1e9;
    int i_hit = -1;

    for ( int i = 0; i < ndata; i++ )
    {
        if ( !symflag || control_pts[i].x() >= 0 )
        {
            vec3d delta = cpproj[i] - mpt;
            delta.scale_x( scale.x() );
            delta.scale_y( scale.y() );
            delta.scale_z( scale.z() );

            double dist_out = delta.mag();
            if ( dist_out < min_dist )
            {
                min_dist = dist_out;

                if ( dist_out < r_test )
                {
                    i_hit = i;
                }
            }
        }
    }

    return i_hit;
}

vector < vec3d > EditXSecWindow::proj_pt_vec( const vector < vec3d > & pt_in )
{
    //==== Find EditCurveXSec Ptr ====//
    CurveEditScreen* curve_editor = dynamic_cast <CurveEditScreen*>
            ( m_ScreenMgr->GetScreen( vsp::VSP_CURVE_EDIT_SCREEN ) );
    if ( !curve_editor )
    {
        return pt_in;
    }

    XSecCurve* xsc = curve_editor->GetXSecCurve();

    if ( !xsc || xsc->GetType() != vsp::XS_EDIT_CURVE )
    {
        return pt_in;
    }

    EditCurveXSec* edit_curve_xs = dynamic_cast<EditCurveXSec*>( xsc );

    vector < vec3d > pt_out = pt_in;

    int izero = -1;
    if ( edit_curve_xs->m_View() == vsp::VIEW_FRONT ) // X,Y
    {
        izero = 2;
    }
    else if ( edit_curve_xs->m_View() == vsp::VIEW_TOP ) // X,Z
    {
        izero = 1;
    }
    else if ( edit_curve_xs->m_View() == vsp::VIEW_LEFT ) // Z,Y
    {
        izero = 0;
    }

    int npt = pt_in.size();
    for ( int i = 0; i < npt; i++ )
    {
        // Non-dimensionalize control point if needed.
        if ( !edit_curve_xs->m_AbsoluteFlag() )
        {
            pt_out[i].set_x( pt_out[i].x() / edit_curve_xs->GetWidth() );
            pt_out[i].set_y( pt_out[i].y() / edit_curve_xs->GetHeight() );
            pt_out[i].set_z( pt_out[i].z() / edit_curve_xs->m_Depth() );
        }

        // Project into 2D view
        pt_out[i].v[izero] = 0.0;
    }

    return pt_out;
}

void EditXSecWindow::InitZoom()
{
    assert( m_ScreenMgr );

    VSPGraphic::Viewport* viewport = getGraphicEngine()->getDisplay()->getViewport();
    assert( viewport );

    //==== Find EditCurveXSec Ptr ====//
    CurveEditScreen* curve_editor = dynamic_cast <CurveEditScreen*>
        ( m_ScreenMgr->GetScreen( vsp::VSP_CURVE_EDIT_SCREEN ) );
    if ( !curve_editor )
    {
        return;
    }

    XSecCurve* xsc = curve_editor->GetXSecCurve();

    if ( !xsc || xsc->GetType() != vsp::XS_EDIT_CURVE )
    {
        return;
    }

    EditCurveXSec* edit_curve_xs = dynamic_cast<EditCurveXSec*>( xsc );
    assert( edit_curve_xs );

    double gl_w = pixel_w();
    double gl_h = pixel_h();
    float oz;

    double wh = max( edit_curve_xs->GetWidth(), edit_curve_xs->GetHeight() );
    oz = ( 1.5 * wh ) * ( gl_w < gl_h ? 1.f / gl_w : 1.f / gl_h );

    getGraphicEngine()->getDisplay()->getCamera()->relativeZoom( oz );
}

} // Close out namespace VSPGUI

