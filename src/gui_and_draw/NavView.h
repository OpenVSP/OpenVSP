//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//

// NavView.h: a GL window's view, as the OpenAxis SDK's navigation sees it.
//
// The device's host computes the motion.  This reports the camera, the model's bounds and the
// surface under the cursor, and applies the camera poses the host sends back.
//
//////////////////////////////////////////////////////////////////////

#ifndef VSP_GUI_NAV_VIEW_H
#define VSP_GUI_NAV_VIEW_H

#ifdef VSP_OPENAXIS

#include <functional>
#include <optional>

#include <openaxis/diagnostics.hpp>
#include <openaxis/navigation.hpp>

namespace VSPGraphic
{
class Camera;
}

namespace VSPGUI
{
class VspGlWindow;

// Runs the SDK's work on the FLTK main thread.
openaxis::Scheduler * NavScheduler();

// Whether two camera poses differ by more than the camera's single precision can hold.
openaxis::PoseDifference NavCompareCamera( const openaxis::Pose & a, const openaxis::Pose & b );

// Navigation options for a session on this window's view.
openaxis::NavigationOptions NavOptions( class NavViewAdapter * adapter );

// Lets the host's navigation read and move the selected viewport's camera.
class NavViewAdapter : public openaxis::NavigationAdapter
{
public:

    NavViewAdapter( VspGlWindow * win );

    openaxis::NavigationContext capture_context() override;
    bool is_current( const openaxis::NavigationContext & context ) override;
    std::unique_ptr < openaxis::NavigationCapture > begin_query( const openaxis::NavigationContext & context ) override;
    openaxis::WriteResult apply_pose( const openaxis::NavigationContext & context, const openaxis::NavigationPose & pose,
                                      const openaxis::Value & navigation, std::optional < openaxis::Vec3 > pivot ) override;

    // Mark the host's pivot on the view, or clear the mark when there is no point.
    void show_pivot( const openaxis::NavigationContext & context, std::optional < openaxis::Vec3 > point ) override;

    // The camera's pose, if the context is still current.
    std::optional < openaxis::Pose > ReadCamera( const openaxis::NavigationContext & context );

    // The view moved in a way the host is told of; a gesture goes on from where it now is.
    void NoteView();

    // Called after each pose moves the view.
    std::function < void() > m_OnMotion;

protected:

    // The selected viewport's camera, or null if it has no area.
    VSPGraphic::Camera * CurrentCamera();

    // An eye far enough back to see the whole model.
    double EyeDistance();

    // Put the pivot's mark where the pivot now is on screen, or take it away.
    void PlacePivot();

    // The camera's pose about the center of the view, which no eye distance enters.
    static openaxis::Pose ViewPose( VSPGraphic::Camera * camera );

    VspGlWindow * m_Win;

    // How far behind the center of the view the reported eye is.
    double m_EyeDistance;

    // The view as the host last knew it.  Anything else that moves it ends the gesture.
    std::optional < openaxis::Pose > m_Known;

    // The host's pivot, and the camera it is shown in.
    std::optional < openaxis::Vec3 > m_Pivot;
    VSPGraphic::Camera * m_PivotCamera;
};
}

#endif

#endif
