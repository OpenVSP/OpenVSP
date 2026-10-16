//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//

// GesturePlayer.cpp: replays scripted mouse and key events into the main GL window.
// For development and regression testing only.
//
// One event per line, a command and up to four integers; '#' starts a comment.
//   push x y state button      FL_PUSH at FLTK window coordinates (origin upper left)
//   drag x y state button      FL_DRAG
//   release x y state button   FL_RELEASE; state holds the buttons still down
//   wheel x y state dy         FL_MOUSEWHEEL
//   keydown key state          FL_KEYDOWN, key is an FLTK key code
//   keyup key state            FL_KEYUP
//   layout rows cols           MainVSPScreen::SetWindowLayout
//   view n                     MainVSPScreen::SetView( VSP_CAM_TOP + n )
//   fit                        MainVSPScreen::FitView
//   ballclassic, ballarc       Ball mode with that ball type
//   turntable                  Turntable mode
//   zoombox                    the next left button drag is a zoom box
//   axis rx ry rz              six-axis device rotation, milliradians, eye axes
//   axist tx ty tz             six-axis device translation, milliradians of ball travel
//   grab n                     screenshot of the window to VSP_GESTURE_OUT.n.png
//   prefs n                    show the Preferences on tab n
//   grabprefs n                screenshot of the Preferences to VSP_GESTURE_OUT.prefs.n.png
//   prefsclick x y state b     FL_PUSH and FL_RELEASE at x y in the Preferences
//   grabmain n                 screenshot of the whole main window to VSP_GESTURE_OUT.main.n.png
//   probe x y                  write the surface point and Geom ID under x y, or "miss"
//   wheelreverse b, wheelanchor n, wheelstep n, zoomreverse b, zoomanchor n, zoomlaw n,
//   zoomspeed percent, pivot n, emulatemiddle b
//                              set that NavSettings member; n is the enum value
//   profile n                  NavSettings::ApplyPreset( n )
//   navstart                   an OpenAxis navigation session on the main window, with no host;
//                              what it sends is written as "nav> {json}"
//   navgesture g               as the host starts gesture g: motion_start and a navigation.query
//                              for the camera, bounds, cursor and picks
//   navstate g                 an orbit navigation.state and the camera.pivot, once the query
//                              is answered
//   navorbit g seq mdeg a      as the host turns the camera of gesture g's query by mdeg
//                              millidegrees through its pick, or the rotation center, about world
//                              Z, or world X when a is 1
//   navend g                   motion_end
//   wait                       nothing; one tick
//   dump                       nothing, but write the camera and Parms
//   select i                   make Geom i the only one selected, or none for -1
//   geomstyle i type glass     Geom i's draw type (vsp::GEOM_DRAW_*); glass 1 makes it
//                              transparent with the "Glass Light" material
// state is the FLTK event state, e.g. 16777216 = FL_BUTTON1.  After every event but wait,
// each viewport's model-view and projection matrices and the view Parms are written in
// hexadecimal floating point, so two runs can be compared exactly.
//
//////////////////////////////////////////////////////////////////////

#include <cstdio>
#include <cstdlib>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>

#include <FL/Fl.H>
#include <FL/Fl_PNG_Image.H>
#include <FL/fl_draw.H>

#include "GesturePlayer.h"
#include "MainVSPScreen.h"
#include "MainGLWindow.h"
#include "NavController.h"
#include "GraphicEngine.h"
#include "Display.h"
#include "LayoutMgr.h"
#include "Viewport.h"
#include "Camera.h"
#include "Vehicle.h"
#include "VehicleMgr.h"
#include "Geom.h"
#include "MaterialMgr.h"
#include "ScreenMgr.h"

#ifdef VSP_OPENAXIS
#include "NavView.h"
#endif

namespace
{
struct GestureEvent
{
    std::string m_Cmd;
    int m_A;
    int m_B;
    int m_C;
    int m_D;
};

std::vector < GestureEvent > g_Events;
size_t g_Next = 0;
MainVSPScreen * g_Screen = nullptr;
FILE * g_Out = nullptr;

void Dump( const char * tag )
{
    VSPGraphic::Display * display = g_Screen->GetGLWindow()->getGraphicEngine()->getDisplay();
    std::vector < VSPGraphic::Viewport * > vps = display->getLayoutMgr()->getViewports();

    for ( size_t v = 0; v < vps.size(); v++ )
    {
        glm::mat4 m = vps[v]->getCamera()->getModelViewMatrix();
        glm::mat4 p = vps[v]->getCamera()->getProjectionMatrix();

        fprintf( g_Out, "%s vp%d MV", tag, ( int )v );
        for ( int i = 0; i < 4; i++ )
        {
            for ( int j = 0; j < 4; j++ )
            {
                fprintf( g_Out, " %a", m[i][j] );
            }
        }
        fprintf( g_Out, " P" );
        for ( int i = 0; i < 4; i++ )
        {
            for ( int j = 0; j < 4; j++ )
            {
                fprintf( g_Out, " %a", p[i][j] );
            }
        }
        fprintf( g_Out, "\n" );
    }

    Vehicle * veh = VehicleMgr.GetVehicle();
    fprintf( g_Out, "%s parms %a %a %a %a %a %a %a %a %a\n", tag,
             veh->m_CORXValue.Get(), veh->m_CORYValue.Get(), veh->m_CORZValue.Get(),
             veh->m_PanXPosValue.Get(), veh->m_PanYPosValue.Get(), veh->m_ZoomValue.Get(),
             veh->m_XRotationValue.Get(), veh->m_YRotationValue.Get(), veh->m_ZRotationValue.Get() );
    fflush( g_Out );
}

#ifdef VSP_OPENAXIS
std::unique_ptr < VSPGUI::NavViewAdapter > g_NavAdapter;
std::unique_ptr < openaxis::NavigationSession > g_NavSession;
openaxis::Value g_NavLastResult;

void NavReceive( const openaxis::Value & message )
{
    fprintf( g_Out, "nav< %s\n", message.dump().c_str() );
    g_NavSession->receive( message );
}

// The pick the last query answered, else the rotation center.
openaxis::Vec3 NavPivot( VSPGUI::VspGlWindow * win )
{
    glm::vec3 cor = win->getCOR();
    openaxis::Vec3 pivot = { cor.x, cor.y, cor.z };
    if ( g_NavLastResult.contains( "first" ) && g_NavLastResult["first"].is_object() )
    {
        openaxis::Value p = g_NavLastResult["first"]["value"]["point"];
        pivot = { p[0], p[1], p[2] };
    }
    return pivot;
}

// Commands that act as the device's host would.  False if e is not one.
bool NavCommand( VSPGUI::VspGlWindow * win, const GestureEvent & e )
{
    if ( e.m_Cmd == "navstart" )
    {
        g_NavAdapter.reset( new VSPGUI::NavViewAdapter( win ) );
        openaxis::NavigationOptions options = VSPGUI::NavOptions( g_NavAdapter.get() );
        g_NavSession.reset( new openaxis::NavigationSession( *g_NavAdapter, []( const openaxis::Value & m )
        {
            fprintf( g_Out, "nav> %s\n", m.dump().c_str() );
            if ( m.value( "type", "" ) == "response" && m.contains( "result" ) )
            {
                g_NavLastResult = m["result"];
            }
            return true;
        }, nullptr, options ) );
        g_NavSession->diagnostics = []( const openaxis::Diagnostic & d )
        {
            fprintf( g_Out, "navdiag %s %s %s\n", d.event.c_str(), d.target.c_str(), d.detail.c_str() );
        };
        g_NavSession->connection_changed();

        // The mouse moving the view, told to the session as the device would tell it.
        win->m_ViewChangedCB = []()
        {
            g_NavAdapter->NoteView();
            g_NavSession->native_camera_changed();
        };
    }
    else if ( e.m_Cmd == "navgesture" )
    {
        NavReceive( { { "type", "motion_start" }, { "gesture_id", e.m_A } } );
        NavReceive( { { "type", "request" }, { "id", 100 + e.m_A }, { "method", "navigation.query" },
                      { "params", { { "gesture_id", e.m_A },
                                    { "values", { "document.id", "world.orientation", "camera.pose", "viewport.aspect",
                                                  "viewport.cursor", "model.bounds", "selection.bounds", "navigation.preferences" } },
                                    { "first", { "pick.cursor.selection", "pick.viewport_center.selection", "pick.cursor", "pick.viewport_center" } } } } } );
    }
    else if ( e.m_Cmd == "navstate" )
    {
        openaxis::Vec3 pivot = NavPivot( win );
        NavReceive( { { "type", "navigation.state" }, { "gesture_id", e.m_A }, { "camera", { { "mode", "orbit" } } } } );
        NavReceive( { { "type", "camera.pivot" }, { "gesture_id", e.m_A }, { "point", { pivot.x, pivot.y, pivot.z } } } );
    }
    else if ( e.m_Cmd == "navorbit" )
    {
        if ( !g_NavLastResult.contains( "values" ) || !g_NavLastResult["values"].contains( "camera.pose" ) )
        {
            fprintf( g_Out, "navorbit: no camera.pose answered\n" );
            return true;
        }
        openaxis::Value cp = g_NavLastResult["values"]["camera.pose"];
        openaxis::Vec3 t = { cp["t"][0], cp["t"][1], cp["t"][2] };
        openaxis::Vec3 r = { cp["r"][0], cp["r"][1], cp["r"][2] };

        openaxis::Vec3 pivot = NavPivot( win );

        double a = 1e-3 * e.m_C * 3.14159265358979323846 / 180.0;
        openaxis::Quat turn = openaxis::Quat::from_rotvec( { 0.0, 0.0, a } );
        if ( e.m_D == 1 )
        {
            turn = openaxis::Quat::from_rotvec( { a, 0.0, 0.0 } );
        }
        openaxis::Vec3 t2 = pivot + turn.rotate( t - pivot );
        openaxis::Vec3 r2 = ( turn * openaxis::Quat::from_rotvec( r ) ).rotvec();

        fprintf( g_Out, "navpivot %.17g %.17g %.17g\n", pivot.x, pivot.y, pivot.z );
        NavReceive( { { "type", "camera.pose" }, { "gesture_id", e.m_A }, { "seq", e.m_B },
                      { "t", { t2.x, t2.y, t2.z } }, { "r", { r2.x, r2.y, r2.z } }, { "ortho_extent", cp["ortho_extent"] } } );
    }
    else if ( e.m_Cmd == "navend" )
    {
        NavReceive( { { "type", "motion_end" }, { "gesture_id", e.m_A } } );
    }
    else
    {
        return false;
    }
    fflush( g_Out );
    return true;
}
#endif

void SetMouse( const GestureEvent & e )
{
    Fl::e_x = e.m_A;
    Fl::e_y = e.m_B;
    Fl::e_state = e.m_C;
}

void SetBallType( VSPGUI::VspGlWindow * win, int type )
{
    VSPGUI::NavSettings settings = win->GetNav().GetSettings();
    settings.m_BallType = type;
    win->GetNav().SetSettings( settings );
    win->GetNav().SetRotateMode( VSPGUI::NAV_ROTATE_BALL );
}

bool SetNavOption( VSPGUI::VspGlWindow * win, const GestureEvent & e )
{
    VSPGUI::NavSettings settings = win->GetNav().GetSettings();

    if ( e.m_Cmd == "wheelreverse" )
    {
        settings.m_WheelReverse = ( e.m_A != 0 );
    }
    else if ( e.m_Cmd == "wheelanchor" )
    {
        settings.m_WheelAnchor = e.m_A;
    }
    else if ( e.m_Cmd == "wheelstep" )
    {
        settings.m_WheelStep = e.m_A;
    }
    else if ( e.m_Cmd == "zoomreverse" )
    {
        settings.m_DragZoomReverse = ( e.m_A != 0 );
    }
    else if ( e.m_Cmd == "zoomanchor" )
    {
        settings.m_DragZoomAnchor = e.m_A;
    }
    else if ( e.m_Cmd == "zoomlaw" )
    {
        settings.m_DragZoomLaw = e.m_A;
    }
    else if ( e.m_Cmd == "zoomspeed" )
    {
        settings.m_ZoomSpeed = 0.01 * e.m_A;
    }
    else if ( e.m_Cmd == "pivot" )
    {
        settings.m_Pivot = e.m_A;
    }
    else if ( e.m_Cmd == "profile" )
    {
        settings.ApplyPreset( e.m_A );
    }
    else if ( e.m_Cmd == "emulatemiddle" )
    {
        settings.m_EmulateMiddle = ( e.m_A != 0 );
    }
    else
    {
        return false;
    }

    win->GetNav().SetSettings( settings );
    return true;
}

void Step( void * )
{
    if ( g_Next >= g_Events.size() )
    {
#ifdef VSP_OPENAXIS
        if ( g_NavSession )
        {
            g_NavSession->close();
        }
#endif
        fclose( g_Out );
        exit( 0 );
    }

    VSPGUI::VspGlWindow * win = g_Screen->GetGLWindow();
    GestureEvent e = g_Events[ g_Next ];
    g_Next++;

    char tag[64];
    snprintf( tag, sizeof( tag ), "%zu:%s", g_Next, e.m_Cmd.c_str() );

    bool dump = true;
    if ( e.m_Cmd == "push" )
    {
        SetMouse( e );
        Fl::e_keysym = FL_Button + e.m_D;
        win->handle( FL_PUSH );
    }
    else if ( e.m_Cmd == "drag" )
    {
        SetMouse( e );
        Fl::e_keysym = FL_Button + e.m_D;
        win->handle( FL_DRAG );
    }
    else if ( e.m_Cmd == "release" )
    {
        SetMouse( e );
        Fl::e_keysym = FL_Button + e.m_D;
        win->handle( FL_RELEASE );
    }
    else if ( e.m_Cmd == "wheel" )
    {
        SetMouse( e );
        Fl::e_dx = 0;
        Fl::e_dy = e.m_D;
        win->handle( FL_MOUSEWHEEL );
    }
    else if ( e.m_Cmd == "keydown" )
    {
        Fl::e_keysym = e.m_A;
        Fl::e_state = e.m_B;
        win->handle( FL_KEYDOWN );
    }
    else if ( e.m_Cmd == "keyup" )
    {
        Fl::e_keysym = e.m_A;
        Fl::e_state = e.m_B;
        win->handle( FL_KEYUP );
    }
    else if ( e.m_Cmd == "layout" )
    {
        g_Screen->SetWindowLayout( e.m_A, e.m_B );
    }
    else if ( e.m_Cmd == "view" )
    {
        g_Screen->SetView( e.m_A + VSPGraphic::Common::VSP_CAM_TOP );
    }
    else if ( e.m_Cmd == "dump" )
    {
    }
    else if ( e.m_Cmd == "select" )
    {
        Vehicle * veh = VehicleMgr.GetVehicle();
        std::vector < std::string > ids = veh->GetGeomVec();
        std::vector < std::string > active;
        if ( e.m_A >= 0 && e.m_A < ( int )ids.size() )
        {
            active.push_back( ids[ e.m_A ] );
        }
        veh->SetActiveGeomVec( active );
        g_Screen->GetScreenMgr()->SetUpdateFlag( true );
    }
    else if ( e.m_Cmd == "geomstyle" )
    {
        Vehicle * veh = VehicleMgr.GetVehicle();
        std::vector < std::string > ids = veh->GetGeomVec();
        if ( e.m_A >= 0 && e.m_A < ( int )ids.size() )
        {
            Geom * geom = veh->FindGeom( ids[ e.m_A ] );
            geom->m_GuiDraw.SetDrawType( e.m_B );
            Material mat;
            if ( e.m_C != 0 && MaterialMgr.FindMaterial( "Glass Light", mat ) )
            {
                geom->SetMaterial( mat.m_Name, mat.m_Ambi, mat.m_Diff, mat.m_Spec, mat.m_Emis, mat.m_Shininess );
            }
            veh->Update();
            g_Screen->GetScreenMgr()->SetUpdateFlag( true );
        }
    }
    else if ( e.m_Cmd == "fit" )
    {
        g_Screen->FitView( false );
    }
    else if ( e.m_Cmd == "ballclassic" )
    {
        SetBallType( win, VSPGUI::NAV_BALL_CLASSIC );
    }
    else if ( e.m_Cmd == "ballarc" )
    {
        SetBallType( win, VSPGUI::NAV_BALL_ROUNDED_ARCBALL );
    }
    else if ( e.m_Cmd == "turntable" )
    {
        win->GetNav().SetRotateMode( VSPGUI::NAV_ROTATE_TURNTABLE );
    }
    else if ( e.m_Cmd == "zoombox" )
    {
        win->GetNav().ArmZoomBox();
    }
    else if ( e.m_Cmd == "axis" )
    {
        win->GetNav().ApplyAxes( 0.001 * glm::dvec3( e.m_A, e.m_B, e.m_C ), glm::dvec3( 0.0 ) );
    }
    else if ( e.m_Cmd == "axist" )
    {
        win->GetNav().ApplyAxes( glm::dvec3( 0.0 ), 0.001 * glm::dvec3( e.m_A, e.m_B, e.m_C ) );
    }
    else if ( e.m_Cmd == "probe" )
    {
        glm::vec3 hit;
        std::string geom;
        if ( win->ProbeSurface( e.m_A, e.m_B, hit, &geom ) )
        {
            fprintf( g_Out, "%s %d %d hit %.9g %.9g %.9g %s\n", tag, e.m_A, e.m_B, hit.x, hit.y, hit.z, geom.c_str() );
        }
        else
        {
            fprintf( g_Out, "%s %d %d miss\n", tag, e.m_A, e.m_B );
        }
        fflush( g_Out );
        dump = false;
    }
    else if ( e.m_Cmd == "grab" )
    {
        char fname[1024];
        snprintf( fname, sizeof( fname ), "%s.%d.png", getenv( "VSP_GESTURE_OUT" ), e.m_A );
        g_Screen->ScreenGrab( fname, win->pixel_w(), win->pixel_h(), false, false );
        dump = false;
    }
    else if ( e.m_Cmd == "prefs" )
    {
        TabScreen * prefs = dynamic_cast < TabScreen * > ( g_Screen->GetScreenMgr()->GetScreen( vsp::VSP_PREFERENCES_SCREEN ) );
        if ( prefs )
        {
            prefs->Show();
            prefs->GetTabs()->value( prefs->GetTab( e.m_A ) );
            prefs->GetFlWindow()->redraw();
        }
        dump = false;
    }
    else if ( e.m_Cmd == "prefsclick" )
    {
        BasicScreen * prefs = dynamic_cast < BasicScreen * > ( g_Screen->GetScreenMgr()->GetScreen( vsp::VSP_PREFERENCES_SCREEN ) );
        if ( prefs )
        {
            Fl_Window * pw = prefs->GetFlWindow();
            // Drags played into the main window leave it as the pushed widget, and with the
            // focus.  Moving the focus can handle events that replace the mouse location.
            Fl::pushed( nullptr );
            pw->take_focus();
            SetMouse( e );
            Fl::e_keysym = FL_Button + e.m_D;
            Fl::handle( FL_PUSH, pw );
            Fl::e_state = 0;
            Fl::handle( FL_RELEASE, pw );
        }
        dump = false;
    }
    else if ( e.m_Cmd == "grabmain" )
    {
        Fl_Window * mw = g_Screen->GetFlWindow();
        Fl_RGB_Image * img = fl_capture_window( mw, 0, 0, mw->w(), mw->h() );
        if ( img )
        {
            char fname[1024];
            snprintf( fname, sizeof( fname ), "%s.main.%d.png", getenv( "VSP_GESTURE_OUT" ), e.m_A );
            fl_write_png( fname, img );
            delete img;
        }
        dump = false;
    }
    else if ( e.m_Cmd == "grabprefs" )
    {
        BasicScreen * prefs = dynamic_cast < BasicScreen * > ( g_Screen->GetScreenMgr()->GetScreen( vsp::VSP_PREFERENCES_SCREEN ) );
        if ( prefs )
        {
            Fl_Window * pw = prefs->GetFlWindow();
            Fl_RGB_Image * img = fl_capture_window( pw, 0, 0, pw->w(), pw->h() );
            if ( img )
            {
                char fname[1024];
                snprintf( fname, sizeof( fname ), "%s.prefs.%d.png", getenv( "VSP_GESTURE_OUT" ), e.m_A );
                fl_write_png( fname, img );
                delete img;
            }
        }
        dump = false;
    }
#ifdef VSP_OPENAXIS
    else if ( NavCommand( win, e ) )
    {
    }
#endif
    else
    {
        dump = SetNavOption( win, e );
    }

    if ( dump )
    {
        Dump( tag );
    }

    Fl::repeat_timeout( 0.005, Step );
}
}

void StartGesturePlayer( MainVSPScreen * screen )
{
    const char * script = getenv( "VSP_GESTURE_SCRIPT" );
    const char * out = getenv( "VSP_GESTURE_OUT" );
    if ( !script || !out )
    {
        return;
    }

    g_Screen = screen;

    std::ifstream f( script );
    std::string line;
    while ( std::getline( f, line ) )
    {
        std::istringstream ss( line );
        GestureEvent e;
        e.m_A = 0;
        e.m_B = 0;
        e.m_C = 0;
        e.m_D = 0;
        if ( !( ss >> e.m_Cmd ) || e.m_Cmd[0] == '#' )
        {
            continue;
        }
        ss >> e.m_A >> e.m_B >> e.m_C >> e.m_D;
        g_Events.push_back( e );
    }

    g_Out = fopen( out, "w" );
    if ( !g_Out )
    {
        return;
    }

    Fl::add_timeout( 1.0, Step );
}
