//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//

// NavDevice.h: a 3D input device's motion, fed to a GL window's navigation.
//
// Built with VSP_OPENAXIS, it connects to the Rotatrix host through the OpenAxis SDK.  The SDK
// verifies the host it connects to and passes on input only when that succeeds.  Until then,
// and with no device or host, the device does nothing and nothing about it is shown.
//
//////////////////////////////////////////////////////////////////////

#ifndef VSP_GUI_NAV_DEVICE_H
#define VSP_GUI_NAV_DEVICE_H

#include <memory>
#include <string>

namespace VSPGUI
{
class VspGlWindow;

class NavDevice
{
public:

    NavDevice( VspGlWindow * win );
    ~NavDevice();

    // True if OpenVSP was built with device support.
    static bool IsBuilt();

    // Look for the device host, and keep looking until Stop.  Must be called from the main thread.
    void Start();
    void Stop();

    // True between Start and Stop.
    bool IsStarted() const;

    // True while connected to a verified host.
    bool IsAvailable() const;

    // The connection's state, for people.
    std::string GetStatus() const;

    // Called whenever the status changes.
    void SetStatusCallback( void ( *cb )( void * ), void * data );

protected:

    // Call the status callback if the status shown has changed.
    void StatusChanged();

    struct Impl;
    std::unique_ptr < Impl > m_Impl;
};
}

#endif
