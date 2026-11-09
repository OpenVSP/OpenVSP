//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//

// ToolbarIcons.cpp: SVG icons for the main window's toolbar, drawn on a 24 x 24 grid in one
// set of colors taken from the rest of the GUI: dark gray lines, the blue of the Geom browser's
// icons as the accent and the pale gray-blue of output fields as the fill.  Arrowheads are filled
// triangles, since FLTK's SVG reader does not draw markers.
//
//////////////////////////////////////////////////////////////////////

#include "ToolbarIcons.h"

#define TB_OPEN "<svg width=\"24\" height=\"24\" viewBox=\"0 0 24 24\" version=\"1.1\" xmlns=\"http://www.w3.org/2000/svg\">"
#define TB_CLOSE "</svg>"

// Lines, accent lines, outlined pale shapes, outlined accent shapes and plain fills.
#define TB_LINE "fill:none;stroke:#404040;stroke-width:1.6;stroke-linecap:round;stroke-linejoin:round"
#define TB_THIN "fill:none;stroke:#404040;stroke-width:1.1;stroke-linecap:round;stroke-linejoin:round"
#define TB_ACCENT "fill:none;stroke:#4c4cff;stroke-width:2;stroke-linecap:round;stroke-linejoin:round"
#define TB_SHAPE "fill:#dcdce8;stroke:#404040;stroke-width:1.5;stroke-linejoin:round"
#define TB_SOLID "fill:#4c4cff;stroke:#404040;stroke-width:1.3;stroke-linejoin:round"
#define TB_WHITE "fill:#ffffff;stroke:#404040;stroke-width:1.2;stroke-linejoin:round"
#define TB_FILL_INK "fill:#404040;stroke:none"
#define TB_FILL_ACCENT "fill:#4c4cff;stroke:none"

const char * svg_tb_new =
    TB_OPEN
    "<path d=\"M6 2.5h8.5l4.5 4.5v14.5H6z\" style=\"" TB_SHAPE "\"/>"
    "<path d=\"M14.5 2.5V7H19\" style=\"" TB_LINE "\"/>"
    "<path d=\"M12.5 10.5v7M9 14h7\" style=\"" TB_ACCENT "\"/>"
    TB_CLOSE;

const char * svg_tb_open =
    TB_OPEN
    "<path d=\"M2.5 19.5V5.5H9l2 2h9.5v12z\" style=\"" TB_SHAPE "\"/>"
    "<path d=\"M2.5 19.5l3-8.5h17l-3 8.5z\" style=\"" TB_SOLID "\"/>"
    TB_CLOSE;

const char * svg_tb_save =
    TB_OPEN
    "<path d=\"M3.5 3.5h13.5l3.5 3.5v13.5h-17z\" style=\"" TB_SOLID "\"/>"
    "<path d=\"M7 3.5h9v6H7z\" style=\"" TB_WHITE "\"/>"
    "<path d=\"M13 5v3\" style=\"" TB_LINE "\"/>"
    "<path d=\"M6.5 13.5h11v7h-11z\" style=\"" TB_WHITE "\"/>"
    TB_CLOSE;

const char * svg_tb_undo =
    TB_OPEN
    "<path d=\"M7 9.5h8a5.25 5.25 0 0 1 0 10.5h-5\" style=\"" TB_ACCENT "\"/>"
    "<path d=\"M2.5 9.5l5.5-4.5v9z\" style=\"" TB_FILL_ACCENT "\"/>"
    TB_CLOSE;

const char * svg_tb_geoms =
    TB_OPEN
    "<path d=\"M5.5 8v11h2.5M5.5 12.5h2.5\" style=\"" TB_THIN "\"/>"
    "<path d=\"M3.5 3.5h4v4h-4z\" style=\"" TB_SOLID "\"/>"
    "<path d=\"M10 5.5h10.5\" style=\"" TB_LINE "\"/>"
    "<path d=\"M8 10.5h4v4H8z\" style=\"" TB_SHAPE "\"/>"
    "<path d=\"M14.5 12.5h6\" style=\"" TB_LINE "\"/>"
    "<path d=\"M8 17h4v4H8z\" style=\"" TB_SHAPE "\"/>"
    "<path d=\"M14.5 19h6\" style=\"" TB_LINE "\"/>"
    TB_CLOSE;

const char * svg_tb_pick =
    TB_OPEN
    "<path d=\"M5 2.5l13.5 10-6 1 3.6 6.4-2.7 1.5-3.6-6.4-4.8 4z\" style=\"" TB_SOLID "\"/>"
    TB_CLOSE;

const char * svg_tb_fit =
    TB_OPEN
    "<path d=\"M3 8V3h5M16 3h5v5M21 16v5h-5M8 21H3v-5\" style=\"" TB_LINE "\"/>"
    "<path d=\"M7.5 9h9v6h-9z\" style=\"" TB_SOLID "\"/>"
    TB_CLOSE;

const char * svg_tb_fit_all =
    TB_OPEN
    "<path d=\"M3 8V3h5M16 3h5v5M21 16v5h-5M8 21H3v-5\" style=\"" TB_LINE "\"/>"
    "<path d=\"M6 6.5h6v5H6z\" style=\"" TB_SOLID "\"/>"
    "<path d=\"M12 12.5h6v5h-6z\" style=\"" TB_SHAPE "\"/>"
    TB_CLOSE;

const char * svg_tb_center =
    TB_OPEN
    "<path d=\"M12 2.5v5M12 21.5v-5M2.5 12h5M21.5 12h-5\" style=\"" TB_LINE "\"/>"
    "<path d=\"M12 9l-2-2.5h4zM12 15l-2 2.5h4zM9 12l-2.5-2v4zM15 12l2.5-2v4z\" style=\"" TB_FILL_INK "\"/>"
    "<circle cx=\"12\" cy=\"12\" r=\"2\" style=\"" TB_FILL_ACCENT "\"/>"
    TB_CLOSE;

const char * svg_tb_views =
    TB_OPEN
    "<path d=\"M10.5 2.5l7.5 3.75-7.5 3.75L3 6.25z\" style=\"" TB_SHAPE "\"/>"
    "<path d=\"M3 6.25v8.5l7.5 3.75V10z\" style=\"" TB_SOLID "\"/>"
    "<path d=\"M18 6.25v8.5l-7.5 3.75V10z\" style=\"fill:#a8a8dc;stroke:#404040;stroke-width:1.5;stroke-linejoin:round\"/>"
    "<path d=\"M16.5 18.5h6l-3 3.5z\" style=\"" TB_FILL_INK "\"/>"
    TB_CLOSE;

const char * svg_tb_ball =
    TB_OPEN
    "<circle cx=\"12\" cy=\"12\" r=\"8.5\" style=\"" TB_SHAPE "\"/>"
    "<path d=\"M3.5 12a8.5 3 0 0 0 17 0M12 3.5a3 8.5 0 0 0 0 17\" style=\"" TB_THIN "\"/>"
    "<path d=\"M5.5 4.8a9.8 9.8 0 0 1 11.6-0.4\" style=\"" TB_ACCENT "\"/>"
    "<path d=\"M19.8 6.2l-4.6-0.2 2.6-3.8z\" style=\"" TB_FILL_ACCENT "\"/>"
    TB_CLOSE;

const char * svg_tb_turntable =
    TB_OPEN
    "<path d=\"M12 15.5V2.5\" style=\"" TB_LINE "\"/>"
    "<path d=\"M8.5 7.5h7v7h-7z\" style=\"" TB_SOLID "\"/>"
    "<path d=\"M3 15.5a9 3 0 1 0 18 0a9 3 0 1 0-18 0z\" style=\"" TB_SHAPE "\"/>"
    "<path d=\"M5.5 19.8a10 3.6 0 0 0 9.5 1.6\" style=\"" TB_ACCENT "\"/>"
    "<path d=\"M18.5 20.6l-4.4 2.2 0.4-4z\" style=\"" TB_FILL_ACCENT "\"/>"
    TB_CLOSE;

const char * svg_tb_zup =
    TB_OPEN
    "<path d=\"M5 19.5A8.5 8.5 0 0 1 4.2 8\" style=\"" TB_ACCENT "\"/>"
    "<path d=\"M2 9.6l3.6-4.2 1 5z\" style=\"" TB_FILL_ACCENT "\"/>"
    "<path d=\"M12 22V7\" style=\"" TB_LINE "\"/>"
    "<path d=\"M12 1.5l-3.2 5.5h6.4z\" style=\"" TB_FILL_INK "\"/>"
    "<path d=\"M15 10h6l-6 7h6\" style=\"" TB_LINE "\"/>"
    TB_CLOSE;

const char * svg_tb_zoom_box =
    TB_OPEN
    "<path d=\"M2.5 2.5h12v12h-12z\" style=\"fill:none;stroke:#404040;stroke-width:1.3;stroke-dasharray:2,1.6\"/>"
    "<path d=\"M18.2 18.2l3.6 3.6\" style=\"fill:none;stroke:#404040;stroke-width:2.8;stroke-linecap:round\"/>"
    "<circle cx=\"14.5\" cy=\"14.5\" r=\"5\" style=\"" TB_SHAPE "\"/>"
    "<path d=\"M14.5 12v5M12 14.5h5\" style=\"" TB_ACCENT "\"/>"
    TB_CLOSE;

const char * svg_tb_rotation_center =
    TB_OPEN
    "<circle cx=\"12\" cy=\"13\" r=\"6\" style=\"" TB_SHAPE "\"/>"
    "<path d=\"M12 4.5v4M12 17.5v4M3.5 13h4M16.5 13h4\" style=\"" TB_LINE "\"/>"
    "<circle cx=\"12\" cy=\"13\" r=\"1.8\" style=\"" TB_FILL_ACCENT "\"/>"
    "<path d=\"M4.5 6.5A9.5 9.5 0 0 1 16 3\" style=\"" TB_ACCENT "\"/>"
    "<path d=\"M19.5 4.8l-4.6 0.8 1.4-4.4z\" style=\"" TB_FILL_ACCENT "\"/>"
    TB_CLOSE;

const char * svg_tb_view_normal =
    TB_OPEN
    "<path d=\"M2.5 19.5l4.5-8h14.5l-4.5 8z\" style=\"" TB_SHAPE "\"/>"
    "<path d=\"M12 15.5V6\" style=\"" TB_ACCENT "\"/>"
    "<path d=\"M12 1.5l-3 5h6z\" style=\"" TB_FILL_ACCENT "\"/>"
    "<circle cx=\"12\" cy=\"15.5\" r=\"1.5\" style=\"" TB_FILL_INK "\"/>"
    TB_CLOSE;

const char * svg_tb_axes =
    TB_OPEN
    "<path d=\"M7 17h11.5\" style=\"fill:none;stroke:#d0352b;stroke-width:2;stroke-linecap:round\"/>"
    "<path d=\"M22.5 17l-4.5-2.5v5z\" style=\"fill:#d0352b;stroke:none\"/>"
    "<path d=\"M7 17V5.5\" style=\"fill:none;stroke:#4c4cff;stroke-width:2;stroke-linecap:round\"/>"
    "<path d=\"M7 1.5l-2.5 4.5h5z\" style=\"fill:#4c4cff;stroke:none\"/>"
    "<path d=\"M7 17l6.5-5\" style=\"fill:none;stroke:#2e9e44;stroke-width:2;stroke-linecap:round\"/>"
    "<path d=\"M16.5 9.5l-5 1 2.8 3.4z\" style=\"fill:#2e9e44;stroke:none\"/>"
    "<circle cx=\"7\" cy=\"17\" r=\"1.6\" style=\"" TB_FILL_INK "\"/>"
    TB_CLOSE;

const char * svg_tb_screenshot =
    TB_OPEN
    "<path d=\"M4 7.5h3.5l1.5-2.5h6l1.5 2.5h3.5a1.5 1.5 0 0 1 1.5 1.5v10a1.5 1.5 0 0 1-1.5 1.5H4a1.5 1.5 0 0 1-1.5-1.5V9A1.5 1.5 0 0 1 4 7.5z\" style=\"" TB_SHAPE "\"/>"
    // A grid of pixels.
    "<path d=\"M7.6 9.9h2.4v2.4H7.6zM10.8 9.9h2.4v2.4h-2.4zM14 9.9h2.4v2.4H14z"
    "M7.6 13.1h2.4v2.4H7.6zM10.8 13.1h2.4v2.4h-2.4zM14 13.1h2.4v2.4H14z"
    "M7.6 16.3h2.4v2.4H7.6zM10.8 16.3h2.4v2.4h-2.4zM14 16.3h2.4v2.4H14z\" style=\"" TB_FILL_ACCENT "\"/>"
    TB_CLOSE;

const char * svg_tb_preferences =
    TB_OPEN
    "<path d=\"M3.5 6h17M3.5 12h17M3.5 18h17\" style=\"" TB_LINE "\"/>"
    "<circle cx=\"9\" cy=\"6\" r=\"2.4\" style=\"" TB_SOLID "\"/>"
    "<circle cx=\"15.5\" cy=\"12\" r=\"2.4\" style=\"" TB_SOLID "\"/>"
    "<circle cx=\"7\" cy=\"18\" r=\"2.4\" style=\"" TB_SOLID "\"/>"
    TB_CLOSE;

const char * svg_tb_help =
    TB_OPEN
    "<circle cx=\"12\" cy=\"12\" r=\"9.5\" style=\"" TB_SHAPE "\"/>"
    "<path d=\"M9.2 9.4a2.9 2.9 0 1 1 4.4 2.5c-1.1 0.6-1.6 1.2-1.6 2.6\" style=\"fill:none;stroke:#4c4cff;stroke-width:2.2;stroke-linecap:round\"/>"
    "<circle cx=\"12\" cy=\"17.6\" r=\"1.3\" style=\"" TB_FILL_ACCENT "\"/>"
    TB_CLOSE;

// A window divided into the views of a layout.
#define TB_FRAME "<path d=\"M2.5 4.5h19v15h-19z\" style=\"" TB_SHAPE "\"/>"

const char * svg_tb_layout_1 =
    TB_OPEN
    TB_FRAME
    "<path d=\"M5 7h14v10H5z\" style=\"fill:#4c4cff;stroke:none;fill-opacity:0.35\"/>"
    TB_CLOSE;

const char * svg_tb_layout_2h =
    TB_OPEN
    TB_FRAME
    "<path d=\"M12 4.5v15\" style=\"" TB_LINE "\"/>"
    "<path d=\"M4.5 7h5.5v10H4.5z\" style=\"fill:#4c4cff;stroke:none;fill-opacity:0.35\"/>"
    TB_CLOSE;

const char * svg_tb_layout_2v =
    TB_OPEN
    TB_FRAME
    "<path d=\"M2.5 12h19\" style=\"" TB_LINE "\"/>"
    "<path d=\"M5 6.5h14v3.5H5z\" style=\"fill:#4c4cff;stroke:none;fill-opacity:0.35\"/>"
    TB_CLOSE;

const char * svg_tb_layout_4 =
    TB_OPEN
    TB_FRAME
    "<path d=\"M12 4.5v15M2.5 12h19\" style=\"" TB_LINE "\"/>"
    "<path d=\"M4.5 6.5h5.5v3.5H4.5z\" style=\"fill:#4c4cff;stroke:none;fill-opacity:0.35\"/>"
    TB_CLOSE;

const char * svg_tb_border =
    TB_OPEN
    "<path d=\"M3 4.5h18v15H3z\" style=\"fill:#f4f4f4;stroke:#d0352b;stroke-width:2;stroke-linejoin:round\"/>"
    "<path d=\"M7 14l5-4.5 5 4.5-5 2.5z\" style=\"" TB_SHAPE "\"/>"
    TB_CLOSE;

const char * svg_tb_vector_screenshot =
    TB_OPEN
    "<path d=\"M4 7.5h3.5l1.5-2.5h6l1.5 2.5h3.5a1.5 1.5 0 0 1 1.5 1.5v10a1.5 1.5 0 0 1-1.5 1.5H4a1.5 1.5 0 0 1-1.5-1.5V9A1.5 1.5 0 0 1 4 7.5z\" style=\"" TB_SHAPE "\"/>"
    // A Bezier curve with its points.
    "<path d=\"M7 16.5C7 9 17 9 17 16.5\" style=\"" TB_ACCENT "\"/>"
    "<path d=\"M5.7 15.2h2.6v2.6H5.7zM15.7 15.2h2.6v2.6h-2.6zM10.7 9.6h2.6v2.6h-2.6z\" style=\"" TB_WHITE "\"/>"
    TB_CLOSE;
