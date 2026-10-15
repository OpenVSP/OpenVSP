//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//

// NavIcons.cpp: SVG icons for the 3D view's rotation modes and input device.
// Each is drawn on a 24 x 24 grid.  Arrowheads are filled triangles, since FLTK's SVG
// reader does not draw markers.
//
//////////////////////////////////////////////////////////////////////

#include "NavIcons.h"

const char * svg_turntable_icon =
    "<svg width=\"24\" height=\"24\" viewBox=\"0 0 24 24\" version=\"1.1\" xmlns=\"http://www.w3.org/2000/svg\">"
    // Vertical axis.
    "<path d=\"M 12 2.5 L 12 21\" style=\"fill:none;stroke:#606060;stroke-width:1;stroke-dasharray:1.5,1\"/>"
    // Platter.
    "<ellipse cx=\"12\" cy=\"18\" rx=\"9.5\" ry=\"3.2\" style=\"fill:#d8d8d8;stroke:#404040;stroke-width:1.2\"/>"
    // A box standing on it.
    "<path d=\"M 8.5 17.5 L 8.5 11 L 12 9.5 L 15.5 11 L 15.5 17.5 L 12 19 Z\" style=\"fill:#f0f0f0;stroke:#404040;stroke-width:1\"/>"
    "<path d=\"M 8.5 11 L 12 12.5 L 15.5 11 M 12 12.5 L 12 19\" style=\"fill:none;stroke:#404040;stroke-width:1\"/>"
    // Arrow around the axis, across the front.
    "<path d=\"M 4 6 A 8 2.5 0 0 0 20 6\" style=\"fill:none;stroke:#1f5fbf;stroke-width:1.6\"/>"
    "<path d=\"M 20 3.6 L 18.4 6.8 L 21.6 6.8 Z\" style=\"fill:#1f5fbf;stroke:none\"/>"
    "</svg>";

const char * svg_ball_icon =
    "<svg width=\"24\" height=\"24\" viewBox=\"0 0 24 24\" version=\"1.1\" xmlns=\"http://www.w3.org/2000/svg\">"
    // Sphere.
    "<circle cx=\"12\" cy=\"13\" r=\"8\" style=\"fill:#d8d8d8;stroke:#404040;stroke-width:1.2\"/>"
    // Equator and meridian.
    "<ellipse cx=\"12\" cy=\"13\" rx=\"8\" ry=\"2.6\" style=\"fill:none;stroke:#606060;stroke-width:0.9\"/>"
    "<ellipse cx=\"12\" cy=\"13\" rx=\"2.6\" ry=\"8\" style=\"fill:none;stroke:#606060;stroke-width:0.9\"/>"
    // Arrow over the top, free in any direction.
    "<path d=\"M 3.3 8 A 10 10 0 0 1 15.4 2.4\" style=\"fill:none;stroke:#1f5fbf;stroke-width:1.6\"/>"
    "<path d=\"M 18.2 3.4 L 14.8 4.4 L 15.6 0.9 Z\" style=\"fill:#1f5fbf;stroke:none\"/>"
    "</svg>";
