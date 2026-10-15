//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//

// NavView.cpp: a GL window's view, as the OpenAxis SDK's navigation sees it.
//
//////////////////////////////////////////////////////////////////////

#include "NavView.h"

#ifdef VSP_OPENAXIS

#include <algorithm>
#include <any>
#include <cmath>

#include "Camera.h"
#include "Display.h"
#include "GraphicEngine.h"
#include "MainGLWindow.h"
#include "Vehicle.h"
#include "VehicleMgr.h"

namespace VSPGUI
{

// OpenVSP's world: Z up, the nose toward -X, right handed.
static openaxis::Value WorldOrientation()
{
    return { { "forward", { -1, 0, 0 } }, { "up", { 0, 0, 1 } }, { "handedness", "right" } };
}

static openaxis::Value BoundsValue( const BndBox & b )
{
    return { { "min", { b.GetMin( 0 ), b.GetMin( 1 ), b.GetMin( 2 ) } },
             { "max", { b.GetMax( 0 ), b.GetMax( 1 ), b.GetMax( 2 ) } } };
}

static openaxis::Pose ToPose( const VSPGraphic::CameraPose & c )
{
    openaxis::Pose p;
    p.t = { c.m_Eye.x, c.m_Eye.y, c.m_Eye.z };
    openaxis::Quat q = { c.m_Rotation.w, c.m_Rotation.x, c.m_Rotation.y, c.m_Rotation.z };
    p.r = q.rotvec();
    p.ortho_extent = c.m_OrthoExtent;
    return p;
}

static VSPGraphic::CameraPose FromPose( const openaxis::Pose & p )
{
    openaxis::Quat q = openaxis::Quat::from_rotvec( p.r );
    VSPGraphic::CameraPose c;
    c.m_Rotation = glm::dquat( q.w, q.x, q.y, q.z );
    c.m_Eye = glm::dvec3( p.t.x, p.t.y, p.t.z );
    c.m_OrthoExtent = p.ortho_extent;
    return c;
}

// The view a navigation gesture belongs to: the camera of the viewport selected when it began.
struct NavViewContext
{
    VSPGraphic::Camera * m_Camera;
};

// Answers the host's questions about the scene, from the view as it was when asked.
class NavSceneCapture : public openaxis::NavigationCapture
{
public:

    NavSceneCapture( VspGlWindow * win, VSPGraphic::Camera * camera, double eye_distance )
    {
        m_Win = win;
        m_Camera = camera;
        m_Pose = ToPose( camera->getPose( eye_distance ) );
    }

    std::optional < openaxis::Pose > initial_observation() override
    {
        return m_Pose;
    }

    openaxis::Value resolve( const std::string & name ) override
    {
        Vehicle * veh = VehicleMgr.GetVehicle();

        if ( name == "camera.pose" )
        {
            return openaxis::pose_value( m_Pose );
        }
        if ( name == "world.orientation" )
        {
            return WorldOrientation();
        }
        if ( name == "viewport.aspect" && m_Camera->viewportHeight() > 0 )
        {
            return ( double )m_Camera->viewportWidth() / m_Camera->viewportHeight();
        }
        if ( name == "document.id" && veh )
        {
            return veh->GetVSP3FileName();
        }
        if ( name == "model.bounds" && veh )
        {
            BndBox b;
            if ( veh->GetVisibleBndBox( b ) && !b.IsEmpty() )
            {
                return BoundsValue( b );
            }
            return nullptr;
        }
        if ( name == "selection.bounds" && veh )
        {
            BndBox b;
            std::vector < std::string > active = veh->GetActiveGeomVec();
            for ( int i = 0; i < ( int )active.size(); i++ )
            {
                Geom * g = veh->FindGeom( active[i] );
                if ( g )
                {
                    b.Update( g->GetBndBox() );
                }
            }
            if ( active.empty() || b.IsEmpty() )
            {
                return nullptr;
            }
            return BoundsValue( b );
        }

        int ex, ey;
        if ( name == "viewport.cursor" )
        {
            double nx, ny;
            if ( CursorInView( ex, ey, nx, ny ) )
            {
                return { { "x", nx }, { "y", ny } };
            }
            return nullptr;
        }
        if ( name == "pick.cursor" )
        {
            double nx, ny;
            if ( CursorInView( ex, ey, nx, ny ) )
            {
                return Pick( ex, ey );
            }
            return nullptr;
        }
        if ( name == "pick.viewport_center" )
        {
            double ppu = m_Win->pixels_per_unit();
            int cx = ( int )std::floor( ( m_Camera->viewportX() + 0.5 * m_Camera->viewportWidth() ) / ppu );
            int cy = m_Win->h() - ( int )std::floor( ( m_Camera->viewportY() + 0.5 * m_Camera->viewportHeight() ) / ppu );
            return Pick( cx, cy );
        }

        // Picks among the selected geometry alone, and other facts, are not known.
        return nullptr;
    }

protected:

    // The pointer in screen points, and in the view's normalized coordinates.  False if it is
    // not over this view.
    bool CursorInView( int & ex, int & ey, double & nx, double & ny )
    {
        if ( !m_Win->GetPointer( ex, ey ) )
        {
            return false;
        }
        double ppu = m_Win->pixels_per_unit();
        double px = ( ex + 0.5 ) * ppu - m_Camera->viewportX();
        double py = m_Win->pixel_h() - ( ey + 0.5 ) * ppu - m_Camera->viewportY();
        double w = m_Camera->viewportWidth();
        double h = m_Camera->viewportHeight();
        if ( w <= 0 || h <= 0 || px < 0 || py < 0 || px > w || py > h )
        {
            return false;
        }
        nx = 2.0 * px / w - 1.0;
        ny = 2.0 * py / h - 1.0;
        return true;
    }

    // The surface point at a place in the window, and the bounds of its Geom.
    openaxis::Value Pick( int ex, int ey )
    {
        glm::vec3 hit;
        std::string id;
        if ( !m_Win->ProbeSurface( ex, ey, hit, &id ) )
        {
            return nullptr;
        }
        openaxis::Value v = { { "point", { hit.x, hit.y, hit.z } } };
        Vehicle * veh = VehicleMgr.GetVehicle();
        Geom * g = nullptr;
        if ( veh && !id.empty() )
        {
            g = veh->FindGeom( id );
        }
        if ( g && !g->GetBndBox().IsEmpty() )
        {
            v["bounds"] = BoundsValue( g->GetBndBox() );
        }
        return v;
    }

    VspGlWindow * m_Win;
    VSPGraphic::Camera * m_Camera;
    openaxis::Pose m_Pose;
};

openaxis::PoseDifference NavCompareCamera( const openaxis::Pose & a, const openaxis::Pose & b )
{
    openaxis::ComparisonOptions tol;
    tol.absolute = 1e-9;
    tol.relative = 1e-5;
    tol.angular = 1e-5;
    tol.projection = 1e-5;
    return openaxis::compare_poses( a, b, tol );
}

openaxis::NavigationOptions NavOptions( NavViewAdapter * adapter )
{
    openaxis::NavigationOptions options;
    options.scheduler = NavScheduler();
    options.compare_camera = NavCompareCamera;
    options.observation = [adapter]( const openaxis::NavigationContext & context )
    {
        return adapter->ReadCamera( context );
    };
    return options;
}

NavViewAdapter::NavViewAdapter( VspGlWindow * win )
{
    m_Win = win;
    m_EyeDistance = 1.0;
}

VSPGraphic::Camera * NavViewAdapter::CurrentCamera()
{
    VSPGraphic::Camera * camera = m_Win->getGraphicEngine()->getDisplay()->getCamera();
    if ( !camera || camera->viewportWidth() <= 0 || camera->viewportHeight() <= 0 )
    {
        return nullptr;
    }
    return camera;
}

double NavViewAdapter::EyeDistance()
{
    double d = 1.0;
    Vehicle * veh = VehicleMgr.GetVehicle();
    BndBox b;
    if ( veh && veh->GetVisibleBndBox( b ) && !b.IsEmpty() )
    {
        d = std::max( d, 2.0 * b.DiagDist() );
    }
    return d;
}

openaxis::NavigationContext NavViewAdapter::capture_context()
{
    VSPGraphic::Camera * camera = CurrentCamera();
    if ( !camera )
    {
        return {};
    }
    NavViewContext c;
    c.m_Camera = camera;
    return c;
}

bool NavViewAdapter::is_current( const openaxis::NavigationContext & context )
{
    const NavViewContext * c = std::any_cast < NavViewContext > ( &context );
    return c && c->m_Camera == CurrentCamera();
}

std::unique_ptr < openaxis::NavigationCapture > NavViewAdapter::begin_query( const openaxis::NavigationContext & context )
{
    if ( !is_current( context ) )
    {
        return {};
    }
    m_EyeDistance = EyeDistance();
    return std::unique_ptr < openaxis::NavigationCapture > ( new NavSceneCapture( m_Win, CurrentCamera(), m_EyeDistance ) );
}

std::optional < openaxis::Pose > NavViewAdapter::ReadCamera( const openaxis::NavigationContext & context )
{
    if ( !is_current( context ) )
    {
        return std::nullopt;
    }
    return ToPose( CurrentCamera()->getPose( m_EyeDistance ) );
}

openaxis::WriteResult NavViewAdapter::apply_pose( const openaxis::NavigationContext & context, const openaxis::NavigationPose & pose,
                                                  const openaxis::Value &, std::optional < openaxis::Vec3 > )
{
    openaxis::WriteResult result;
    if ( !is_current( context ) || !( pose.ortho_extent > 0.0 ) )
    {
        return result;
    }
    VSPGraphic::Camera * camera = CurrentCamera();

    // The zoom the view can hold.
    VSPGraphic::CameraPose c = FromPose( pose );
    double lo, hi;
    m_Win->GetZoomLimits( lo, hi );
    double zoom = c.m_OrthoExtent / camera->viewportHeight();
    zoom = std::min( std::max( zoom, lo ), hi );
    c.m_OrthoExtent = zoom * camera->viewportHeight();

    VSPGraphic::CameraPose realized = camera->setPose( c );
    result.realized = ToPose( realized );
    result.success = true;

    // Readings until the next query put the eye where this pose did.
    glm::dvec3 back = realized.m_Rotation * glm::dvec3( 0.0, 0.0, 1.0 );
    m_EyeDistance = glm::dot( realized.m_Eye - camera->getPose( 0.0 ).m_Eye, back );

    m_Win->UpdateRotationParms();
    m_Win->UpdatePanParms();
    m_Win->UpdateZoomParms();
    m_Win->redraw();

    if ( m_OnMotion )
    {
        m_OnMotion();
    }
    return result;
}

}

#endif
