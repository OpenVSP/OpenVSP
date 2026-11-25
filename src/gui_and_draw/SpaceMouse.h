//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//

// SpaceMouse.h: a six-axis device read straight from USB, such as 3Dconnexion's SpaceMouse,
// moving a GL window's view.
//
// Built with VSP_SPNAVDEV, it reads the device through libspnavdev, with no vendor driver.  The
// device is opened shared, so other programs can use it too; OpenVSP moves its view only while
// one of its windows has the keyboard focus.  What each axis does is a preference; by default the
// view moves as the cap does: push to pan, pull toward you to zoom in, tilt and twist to turn the
// model.  Its buttons act through the bindings.  With no device it looks again every two seconds,
// and nothing about it is shown.
//
//////////////////////////////////////////////////////////////////////

#ifndef VSP_GUI_SPACE_MOUSE_H
#define VSP_GUI_SPACE_MOUSE_H

#include <memory>
#include <string>
#include <vector>

namespace VSPGUI
{
class VspGlWindow;

class SpaceMouse
{
public:

    SpaceMouse( VspGlWindow * win );
    ~SpaceMouse();

    // True if OpenVSP was built with SpaceMouse support.
    static bool IsBuilt();

    // Look for a device, and keep looking until Stop.  Must be called from the main thread.
    void Start();
    void Stop();

    // True between Start and Stop.
    bool IsStarted() const;

    // True while a device is open.
    bool IsAvailable() const;

    // The device's state, for people.
    std::string GetStatus() const;

    // Called whenever the status changes.
    void SetStatusCallback( void ( *cb )( void * ), void * data );

    // The open device's buttons' names, as it gives them, or none.
    std::vector < std::string > GetButtonNames() const;

    // The axes as the device last reported them, in its order; all zero with no device.
    void GetCounts( int counts[6] ) const;

    // Counts at full deflection, and those about rest that do nothing.
    static double FullCounts();
    static double DeadCounts();

    // Move the view by the device's axes held for dt seconds, as the preferences map them.
    // Counts are the device's own, in its order; full deflection is about FullCounts().
    void Move( const int counts[6], double dt );

protected:

    // Call the status callback if the status shown has changed.
    void StatusChanged();

    struct Impl;
    std::unique_ptr < Impl > m_Impl;
};
}

#endif
