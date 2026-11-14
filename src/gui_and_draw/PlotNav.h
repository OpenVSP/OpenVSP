//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//

// PlotNav.h: what the mouse does in a 2D plot or the Edit Curve view, from the 3D view's settings.

#ifndef VSP_GUI_PLOT_NAV_H
#define VSP_GUI_PLOT_NAV_H

#include "NavController.h"

namespace VSPGUI
{

// What a mouse press does in a plot
enum PlotNavAction
{
    PLOT_NAV_NONE,
    PLOT_NAV_BOX_ZOOM,      // Drag a box; on release the plot zooms to it.
    PLOT_NAV_PAN,
    PLOT_NAV_ZOOM,          // Drag up or down to zoom.
    PLOT_NAV_ZOOM_IN,       // One wheel step in, at the press.
    PLOT_NAV_ZOOM_OUT       // One wheel step out, at the press.
};

// The user's mouse settings, which the plots share with the 3D view
void SetPlotNavSettings( const NavSettings & settings );
const NavSettings & GetPlotNavSettings();

// What the press just made does in a plot, as the 3D view's binding for it: what rotates the view draws a box to
// zoom to, and what pans or zooms the view pans or zooms the plot.  pick_first is set when the press goes first to
// a point or line under it, as a waiting tool pick takes such a press first in the 3D view.
PlotNavAction PlotNavPress( bool & pick_first );

// The factor a zoom drag scales a plot's range by, for a move of dy points down
double PlotNavDragFactor( int dy );

// The factor the wheel scales a plot's range by, for a scroll of dy
double PlotNavWheelFactor( int dy );

}

#endif
