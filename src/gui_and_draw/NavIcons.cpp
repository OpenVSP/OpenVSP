//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//

// NavIcons.cpp: SVG icons for the 3D input devices, drawn on a 24 x 24 grid.
//
//////////////////////////////////////////////////////////////////////

#include "NavIcons.h"

const char * svg_trackball_icon =
    "<svg width=\"24\" height=\"24\" viewBox=\"0 0 24 24\" version=\"1.1\" xmlns=\"http://www.w3.org/2000/svg\">"
    // Base.
    "<path d=\"M 2.5 17 L 4 21.5 L 20 21.5 L 21.5 17 Z\" style=\"fill:#808080;stroke:#404040;stroke-width:1\"/>"
    // Ball, with a highlight.
    "<circle cx=\"12\" cy=\"11\" r=\"8\" style=\"fill:#c43c3c;stroke:#5a1a1a;stroke-width:1\"/>"
    "<ellipse cx=\"9\" cy=\"7.5\" rx=\"2.6\" ry=\"1.7\" style=\"fill:#f2c4c4;stroke:none\"/>"
    "</svg>";

const char * svg_spacemouse_icon =
    "<svg width=\"24\" height=\"24\" viewBox=\"0 0 24 24\" version=\"1.1\" xmlns=\"http://www.w3.org/2000/svg\">"
    // Base, with the light ring around the cap.
    "<path d=\"M 2 18.5 C 2 15.8 6.5 14.8 12 14.8 C 17.5 14.8 22 15.8 22 18.5 C 22 20.6 17.5 21.6 12 21.6 C 6.5 21.6 2 20.6 2 18.5 Z\" style=\"fill:#606060;stroke:#303030;stroke-width:1\"/>"
    "<ellipse cx=\"12\" cy=\"15.6\" rx=\"7.4\" ry=\"2.3\" style=\"fill:#3a8ee6;stroke:none\"/>"
    // Cap: its side, then its top, with a highlight.
    "<path d=\"M 5 7.5 L 5 14 A 7 2.6 0 0 0 19 14 L 19 7.5 Z\" style=\"fill:#8c939c;stroke:#40454b;stroke-width:1\"/>"
    "<ellipse cx=\"12\" cy=\"7.5\" rx=\"7\" ry=\"2.6\" style=\"fill:#d6dade;stroke:#40454b;stroke-width:1\"/>"
    "<ellipse cx=\"9.5\" cy=\"6.9\" rx=\"2.4\" ry=\"0.8\" style=\"fill:#f6f7f8;stroke:none\"/>"
    "</svg>";
