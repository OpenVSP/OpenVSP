//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//

// NavIcons.cpp: SVG icons for the 3D input device, drawn on a 24 x 24 grid.
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
