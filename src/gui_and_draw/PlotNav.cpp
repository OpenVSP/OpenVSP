//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//

// PlotNav.cpp: what the mouse does in a 2D plot or the Edit Curve view, from the 3D view's settings.

#include "PlotNav.h"

#include <FL/Fl.H>

#include <cmath>

namespace VSPGUI
{

static NavSettings s_PlotNavSettings;

void SetPlotNavSettings( const NavSettings & settings )
{
    s_PlotNavSettings = settings;
}

const NavSettings & GetPlotNavSettings()
{
    return s_PlotNavSettings;
}

PlotNavAction PlotNavPress( bool & pick_first )
{
    // A plot has no geometry, so every press is on the background, and no key is held over it.
    NavPress press;
    press.m_Buttons = Fl::event_state() & ( FL_BUTTON1 | FL_BUTTON2 | FL_BUTTON3 | FL_BUTTON4 | FL_BUTTON5 );
    press.m_Modifiers = Fl::event_state() & ( FL_SHIFT | FL_CTRL | FL_ALT | FL_META );
    press.m_PressedButton = FL_BUTTON( Fl::event_button() );
    press.m_Where = NAV_ON_BACKGROUND;
    press = s_PlotNavSettings.Effective( press );

    const NavBinding * binding = s_PlotNavSettings.m_Profile.Match( press, NAV_ON_DRAG );
    pick_first = false;
    if ( !binding )
    {
        return PLOT_NAV_NONE;
    }
    pick_first = binding->m_PickFirst;

    switch ( binding->m_Action )
    {
    case NAV_ROTATE:
    case NAV_ZOOM_BOX:
        return PLOT_NAV_BOX_ZOOM;
    case NAV_PAN:
        return PLOT_NAV_PAN;
    case NAV_ZOOM:
        return PLOT_NAV_ZOOM;
    case NAV_ZOOM_IN:
        return PLOT_NAV_ZOOM_IN;
    case NAV_ZOOM_OUT:
        return PLOT_NAV_ZOOM_OUT;
    default:
        return PLOT_NAV_NONE;
    }
}

double PlotNavDragFactor( int dy )
{
    // Down zooms in unless the drag direction is reversed; the zoom grows with the distance moved.
    if ( s_PlotNavSettings.m_DragZoomReverse )
    {
        dy = -dy;
    }
    return std::exp( -0.02 * s_PlotNavSettings.m_ZoomSpeed * dy );
}

double PlotNavWheelFactor( int dy )
{
    // As the 3D view's wheel: toward the user zooms out unless reversed, a step per event or per unit of scroll.
    if ( s_PlotNavSettings.m_WheelReverse )
    {
        dy = -dy;
    }

    const double step = 1.15;
    if ( s_PlotNavSettings.m_WheelStep == NAV_WHEEL_PROPORTIONAL )
    {
        return std::pow( step, s_PlotNavSettings.m_ZoomSpeed * dy );
    }
    if ( dy > 0 )
    {
        return step;
    }
    if ( dy < 0 )
    {
        return 1.0 / step;
    }
    return 1.0;
}

}
