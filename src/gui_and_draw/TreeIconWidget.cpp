//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//
// TreeIconWidget.h: a modified Fl_Tree widget based on the code provided at Erco's
//             FLTK Cheat Page: https://www.seriss.com/people/erco/fltk/#TreeWithClickIcons
// Tim Cuatt
//
//////////////////////////////////////////////////////////////////////

#include <TreeIconWidget.h>
#include <TreeColumnWidget.h>
#include <iostream>

// Fl_Tree clickable custom item icons with right justification
// erco 11/28/21


//===== SVG Drawing data stored here ====//
const char* svg_show_icon = "<svg"
                            "   width=\"8.0319548mm\""
                            "   height=\"6.9383039mm\""
                            "   viewBox=\"0 0 8.0319548 6.9383039\""
                            "   version=\"1.1\""
                            "   id=\"svg1\""
                            "   xmlns=\"http://www.w3.org/2000/svg\""
                            "   xmlns:svg=\"http://www.w3.org/2000/svg\">"
                            "  <defs"
                            "     id=\"defs1\">"
                            "    <linearGradient"
                            "       id=\"swatch17\">"
                            "      <stop"
                            "         style=\"stop-color:#000000;stop-opacity:1;\""
                            "         offset=\"0\""
                            "         id=\"stop18\" />"
                            "    </linearGradient>"
                            "  </defs>"
                            "  <g"
                            "     id=\"layer1\""
                            "     transform=\"translate(-48.207771,-56.178031)\">"
                            "    <g"
                            "       id=\"g3\""
                            "       style=\"stroke:#737373;stroke-opacity:1\">"
                            "      <path"
                            "         d=\"m 48.835392,59.946724 c -0.04817,-0.0906 -0.07225,-0.135865 -0.08574,-0.205715 -0.01012,-0.05246 -0.01012,-0.135199 0,-0.187654 0.01349,-0.06984 0.03756,-0.115117 0.08574,-0.205716 0.398031,-0.748436 1.582823,-2.640443 3.388359,-2.640443 1.805536,0 2.990326,1.892007 3.388357,2.640443 0.04818,0.0906 0.07225,0.135866 0.08574,0.205716 0.01012,0.05246 0.01012,0.135199 0,0.187654 -0.01347,0.06984 -0.03756,0.115117 -0.08574,0.205715 -0.398031,0.748437 -1.582821,2.640444 -3.388357,2.640444 -1.805536,0 -2.990328,-1.892007 -3.388359,-2.640444 z\""
                            "         stroke=\"currentColor\""
                            "         stroke-linecap=\"round\""
                            "         stroke-linejoin=\"round\""
                            "         stroke-width=\"0.963537\""
                            "         id=\"path1-3\""
                            "         style=\"fill:none;stroke:#737373;stroke-width:1.05833;stroke-dasharray:none;stroke-opacity:1\" />"
                            "      <path"
                            "         d=\"m 52.218961,60.747056 c 0.595236,0 1.07774,-0.482503 1.07774,-1.07774 0,-0.595236 -0.482504,-1.07774 -1.07774,-1.07774 -0.595236,0 -1.07774,0.482504 -1.07774,1.07774 0,0.595237 0.482504,1.07774 1.07774,1.07774 z\""
                            "         stroke=\"currentColor\""
                            "         stroke-linecap=\"round\""
                            "         stroke-linejoin=\"round\""
                            "         stroke-width=\"0.898116\""
                            "         id=\"path2-6\""
                            "         style=\"fill:none;stroke:#737373;stroke-opacity:1\" />"
                            "    </g>"
                            "  </g>"
                            "</svg>";

const char *svg_hide_icon = "<svg"
                            "   width=\"7.9248714mm\""
                            "   height=\"6.8791671mm\""
                            "   viewBox=\"0 0 7.9248714 6.8791671\""
                            "   version=\"1.1\""
                            "   id=\"svg1\""
                            "   xmlns=\"http://www.w3.org/2000/svg\""
                            "   xmlns:svg=\"http://www.w3.org/2000/svg\">"
                            "  <defs"
                            "     id=\"defs1\">"
                            "    <linearGradient"
                            "       id=\"swatch17\">"
                            "      <stop"
                            "         style=\"stop-color:#000000;stop-opacity:1;\""
                            "         offset=\"0\""
                            "         id=\"stop18\" />"
                            "    </linearGradient>"
                            "  </defs>"
                            "  <g"
                            "     id=\"layer1\""
                            "     transform=\"translate(-48.272786,-73.268373)\">"
                            "    <path"
                            "       d=\"m 51.791287,73.790066 c 0.143499,-0.02539 0.291482,-0.039 0.443911,-0.039 1.802117,0 2.984665,1.902901 3.381943,2.655624 0.04808,0.09112 0.07212,0.136644 0.08557,0.20694 0.01014,0.05276 0.01014,0.13602 0,0.188774 -0.01349,0.07023 -0.03767,0.116117 -0.08609,0.207828 -0.105869,0.200434 -0.267229,0.482184 -0.481048,0.787758 M 50.3727,74.475524 c -0.763233,0.619535 -1.281381,1.480284 -1.519075,1.930534 -0.0483,0.0915 -0.07245,0.137235 -0.08591,0.20749 -0.01014,0.05276 -0.01014,0.13597 -1e-5,0.188774 0.01346,0.07024 0.03749,0.115789 0.08558,0.206899 0.397281,0.752739 1.57982,2.655624 3.381933,2.655624 0.726635,0 1.352557,-0.309376 1.866857,-0.727989 m -4.222595,-5.115105 4.846498,5.812801 m -3.239602,-3.822668 c -0.191649,0.229335 -0.310182,0.546146 -0.310182,0.896071 0,0.699898 0.474156,1.267237 1.059024,1.267237 0.292433,0 0.55719,-0.141849 0.748839,-0.371172\""
                            "       stroke=\"currentColor\""
                            "       stroke-width=\"2\""
                            "       stroke-linecap=\"round\""
                            "       stroke-linejoin=\"round\""
                            "       id=\"path1-0-4\""
                            "       style=\"fill:none;stroke:#b2b2b2ff;stroke-width:0.965388;stroke-dasharray:none;stroke-opacity:1\" />"
                            "  </g>"
                            "</svg>";

const char *svg_wire_icon = "<svg"
                            "   width=\"7.9375mm\""
                            "   height=\"6.8791671mm\""
                            "   viewBox=\"0 0 7.9375 6.8791671\""
                            "   version=\"1.1\""
                            "   id=\"svg1\""
                            "   xmlns=\"http://www.w3.org/2000/svg\""
                            "   xmlns:svg=\"http://www.w3.org/2000/svg\">"
                            "  <defs"
                            "     id=\"defs1\">"
                            "    <linearGradient"
                            "       id=\"swatch17\">"
                            "      <stop"
                            "         style=\"stop-color:#000000;stop-opacity:1;\""
                            "         offset=\"0\""
                            "         id=\"stop18\" />"
                            "    </linearGradient>"
                            "  </defs>"
                            "  <g"
                            "     id=\"layer1\""
                            "     transform=\"translate(-35.344311,-38.831077)\">"
                            "    <g"
                            "       id=\"g19\">"
                            "      <rect"
                            "         style=\"fill:none;stroke:#4c4cffff;stroke-width:1.05833;stroke-dasharray:none;stroke-opacity:1\""
                            "         id=\"rect18\""
                            "         width=\"6.8791666\""
                            "         height=\"5.8208332\""
                            "         x=\"35.873478\""
                            "         y=\"39.360245\""
                            "         rx=\"0.79374999\""
                            "         ry=\"0.79374999\" />"
                            "      <path"
                            "         style=\"fill:none;stroke:#4c4cffff;stroke-width:1.05833;stroke-dasharray:none;stroke-opacity:1\""
                            "         d=\"m 35.873478,42.270661 h 6.879164\""
                            "         id=\"path18\" />"
                            "      <path"
                            "         style=\"fill:none;stroke:#4c4cffff;stroke-width:1.05833;stroke-dasharray:none;stroke-opacity:1\""
                            "         d=\"m 39.313061,39.360244 v 5.820831\""
                            "         id=\"path19\" />"
                            "    </g>"
                            "  </g>"
                            "</svg>";

const char *svg_hidden_icon = "<svg"
                              "   width=\"7.9374952mm\""
                              "   height=\"6.8791628mm\""
                              "   viewBox=\"0 0 7.9374952 6.8791628\""
                              "   version=\"1.1\""
                              "   id=\"svg1\""
                              "   xmlns=\"http://www.w3.org/2000/svg\""
                              "   xmlns:svg=\"http://www.w3.org/2000/svg\">"
                              "  <defs"
                              "     id=\"defs1\">"
                              "    <linearGradient"
                              "       id=\"swatch17\">"
                              "      <stop"
                              "         style=\"stop-color:#000000;stop-opacity:1;\""
                              "         offset=\"0\""
                              "         id=\"stop18\" />"
                              "    </linearGradient>"
                              "  </defs>"
                              "  <g"
                              "     id=\"layer1\""
                              "     transform=\"translate(-35.344313,-47.467532)\">"
                              "    <rect"
                              "       style=\"fill:none;stroke:#4c4cffff;stroke-width:1.05833;stroke-dasharray:none;stroke-opacity:1\""
                              "       id=\"rect18-8\""
                              "       width=\"6.8791666\""
                              "       height=\"5.8208332\""
                              "       x=\"35.873478\""
                              "       y=\"47.996696\""
                              "       rx=\"0.79374999\""
                              "       ry=\"0.79374999\" />"
                              "  </g>"
                              "</svg>";

const char *svg_shade_icon = "<svg"
                             "   width=\"7.9374952mm\""
                             "   height=\"6.8791628mm\""
                             "   viewBox=\"0 0 7.9374952 6.8791628\""
                             "   version=\"1.1\""
                             "   id=\"svg1\""
                             "   xmlns=\"http://www.w3.org/2000/svg\""
                             "   xmlns:svg=\"http://www.w3.org/2000/svg\">"
                             "  <defs"
                             "     id=\"defs1\">"
                             "    <linearGradient"
                             "       id=\"swatch17\">"
                             "      <stop"
                             "         style=\"stop-color:#000000;stop-opacity:1;\""
                             "         offset=\"0\""
                             "         id=\"stop18\" />"
                             "    </linearGradient>"
                             "  </defs>"
                             "  <g"
                             "     id=\"layer1\""
                             "     transform=\"translate(-35.344313,-56.103983)\">"
                             "    <g"
                             "       id=\"g22\">"
                             "      <path"
                             "         id=\"rect18-8-1-9\""
                             "         d=\"m 41.958894,62.420454 h -5.291666 c -0.438376,0 -0.79375,-0.355374 -0.79375,-0.79375 v -4.233333 c 0,0 1.555973,1.930802 2.573335,2.781754 1.017362,0.850952 3.512081,2.245329 3.512081,2.245329 z\""
                             "         style=\"fill:#b2b2b2ff;fill-opacity:1;stroke:none;stroke-width:1.05833;stroke-dasharray:none;stroke-opacity:1\" />"
                             "      <rect"
                             "         style=\"fill:none;stroke:#737373ff;stroke-width:1.05833;stroke-dasharray:none;stroke-opacity:1\""
                             "         id=\"rect18-8-1\""
                             "         width=\"6.8791666\""
                             "         height=\"5.8208332\""
                             "         x=\"35.873478\""
                             "         y=\"56.633148\""
                             "         rx=\"0.79374999\""
                             "         ry=\"0.79374999\" />"
                             "    </g>"
                             "  </g>"
                             "</svg>";

const char *svg_texture_icon = "<svg"
                               "   width=\"7.9374952mm\""
                               "   height=\"6.879159mm\""
                               "   viewBox=\"0 0 7.9374952 6.879159\""
                               "   version=\"1.1\""
                               "   id=\"svg1\""
                               "   xmlns=\"http://www.w3.org/2000/svg\""
                               "   xmlns:svg=\"http://www.w3.org/2000/svg\">"
                               "  <defs"
                               "     id=\"defs1\">"
                               "    <linearGradient"
                               "       id=\"swatch17\">"
                               "      <stop"
                               "         style=\"stop-color:#000000;stop-opacity:1;\""
                               "         offset=\"0\""
                               "         id=\"stop18\" />"
                               "    </linearGradient>"
                               "  </defs>"
                               "  <g"
                               "     id=\"layer1\""
                               "     transform=\"translate(-35.344313,-64.740442)\">"
                               "    <g"
                               "       id=\"g21\">"
                               "      <g"
                               "         id=\"g23\">"
                               "        <path"
                               "           id=\"rect20-7\""
                               "           style=\"fill:#b2b2b2ff;stroke-width:1.05833;stroke-dasharray:1.05833, 1.05833\""
                               "           d=\"m 42.752644,65.2696 -2e-6,2.910416 H 39.31306 l 10e-7,-2.910417 z\" />"
                               "        <path"
                               "           id=\"rect20-7-5\""
                               "           style=\"fill:#b2b2b2ff;stroke-width:1.05833;stroke-dasharray:1.05833, 1.05833\""
                               "           d=\"m 39.31306,68.180016 -2e-6,2.910416 h -3.439582 l 1e-6,-2.910417 z\" />"
                               "        <path"
                               "           style=\"fill:none;stroke:#737373ff;stroke-width:1.05833;stroke-dasharray:none;stroke-opacity:1\""
                               "           d=\"m 35.873478,68.180016 h 6.879164\""
                               "           id=\"path18-2\" />"
                               "        <path"
                               "           style=\"fill:none;stroke:#737373ff;stroke-width:1.05833;stroke-dasharray:none;stroke-opacity:1\""
                               "           d=\"M 39.313061,65.269599 V 71.09043\""
                               "           id=\"path19-1\" />"
                               "        <rect"
                               "           style=\"fill:none;stroke:#737373ff;stroke-width:1.05833;stroke-dasharray:none;stroke-opacity:1\""
                               "           id=\"rect18-8-5\""
                               "           width=\"6.8791666\""
                               "           height=\"5.8208332\""
                               "           x=\"35.873478\""
                               "           y=\"65.2696\""
                               "           rx=\"0.79374999\""
                               "           ry=\"0.79374999\" />"
                               "      </g>"
                               "    </g>"
                               "  </g>"
                               "</svg>";

const char *svg_none_icon = "<svg"
                            "   width=\"7.9374952mm\""
                            "   height=\"6.8791671mm\""
                            "   viewBox=\"0 0 7.9374952 6.8791671\""
                            "   version=\"1.1\""
                            "   id=\"svg1\""
                            "   xmlns=\"http://www.w3.org/2000/svg\""
                            "   xmlns:svg=\"http://www.w3.org/2000/svg\">"
                            "  <defs"
                            "     id=\"defs1\">"
                            "    <linearGradient"
                            "       id=\"swatch17\">"
                            "      <stop"
                            "         style=\"stop-color:#000000;stop-opacity:1;\""
                            "         offset=\"0\""
                            "         id=\"stop18\" />"
                            "    </linearGradient>"
                            "  </defs>"
                            "  <g"
                            "     id=\"layer1\""
                            "     transform=\"translate(-35.344313,-73.309838)\">"
                            "    <path"
                            "       id=\"rect18-8-14\""
                            "       style=\"fill:none;stroke:#737373;stroke-width:1.05833\""
                            "       d=\"m 35.873478,75.691082 v -1.058333 c 0,-0.439738 0.354012,-0.79375 0.79375,-0.79375 h 0.661458 m 1.322917,0 h 1.322917 m 1.322917,0 h 0.661458 c 0.439737,0 0.79375,0.354012 0.79375,0.79375 v 1.058333 m 0,2.116667 v 1.058333 c 0,0.439737 -0.354013,0.79375 -0.79375,0.79375 h -0.661458 m -1.322917,0 h -1.322917 m -1.322917,0 h -0.661458 c -0.439738,0 -0.79375,-0.354013 -0.79375,-0.79375 v -1.058333\" />"
                            "  </g>"
                            "</svg>";

const char *svg_mix_show = "<svg"
                           "   width=\"7.9248714mm\""
                           "   height=\"6.9490752mm\""
                           "   viewBox=\"0 0 7.9248714 6.9490752\""
                           "   version=\"1.1\""
                           "   id=\"svg1\""
                           "   xml:space=\"preserve\""
                           "   xmlns=\"http://www.w3.org/2000/svg\""
                           "   xmlns:svg=\"http://www.w3.org/2000/svg\"><defs"
                           "     id=\"defs1\"><linearGradient"
                           "       id=\"swatch17\"><stop"
                           "         style=\"stop-color:#000000;stop-opacity:1;\""
                           "         offset=\"0\""
                           "         id=\"stop18\" /></linearGradient></defs><g"
                           "     id=\"layer1\""
                           "     transform=\"translate(-46.903527,-56.986851)\"><g"
                           "       id=\"g1\"><path"
                           "         d=\"m 53.602992,59.15935 c 0.31508,0.413838 0.533883,0.802218 0.644888,1.012541 0.04808,0.09112 0.07212,0.136644 0.08557,0.20694 0.01014,0.05276 0.01014,0.13602 0,0.188774 -0.01349,0.07023 -0.03767,0.116117 -0.08609,0.207828 -0.195968,0.371013 -0.736256,1.277673 -1.672704,2.020046 m -4.436045,-3.648837 c -0.304534,0.39798 -0.525793,0.781296 -0.654247,1.024617 -0.0483,0.0915 -0.07245,0.137235 -0.08591,0.20749 -0.01014,0.05276 -0.01014,0.13597 -1e-5,0.188774 0.01346,0.07024 0.03749,0.115789 0.08558,0.206899 0.223567,0.423579 0.706473,1.231019 1.415596,1.844161\""
                           "         stroke=\"currentColor\""
                           "         stroke-width=\"2\""
                           "         stroke-linecap=\"round\""
                           "         stroke-linejoin=\"round\""
                           "         id=\"path1-0-4-5\""
                           "         style=\"fill:none;stroke:#b2b2b2;stroke-width:0.965388;stroke-dasharray:none;stroke-opacity:1\" /><circle"
                           "         id=\"path8-1\""
                           "         style=\"fill:#b2b2b2;fill-opacity:1;stroke:#b2b2b2;stroke-width:0.100957;stroke-dasharray:none;stroke-opacity:1;paint-order:stroke fill markers\""
                           "         cx=\"50.73822\""
                           "         cy=\"63.356277\""
                           "         r=\"0.52916664\" /><path"
                           "         style=\"fill:none;fill-opacity:1;stroke:#b2b2b2;stroke-width:1.05833;stroke-linecap:butt;stroke-dasharray:none;stroke-opacity:1;paint-order:stroke fill markers\""
                           "         d=\"m 50.751147,62.227423 -0.01744,-1.074237 c 0,0 1.315828,-0.442877 1.324046,-1.914951 0.0051,-0.918266 -0.297385,-1.741379 -1.191816,-1.721968 -1.254124,0.02722 -1.185834,1.470199 -1.185834,1.470199\""
                           "         id=\"path9-1\" /></g></g></svg>";

const char *svg_mix_surf = "<svg"
                           "   width=\"7.9371648mm\""
                           "   height=\"6.898602mm\""
                           "   viewBox=\"0 0 7.9371648 6.898602\""
                           "   version=\"1.1\""
                           "   id=\"svg1\""
                           "   xml:space=\"preserve\""
                           "   xmlns=\"http://www.w3.org/2000/svg\""
                           "   xmlns:svg=\"http://www.w3.org/2000/svg\"><defs"
                           "     id=\"defs1\"><linearGradient"
                           "       id=\"swatch17\"><stop"
                           "         style=\"stop-color:#000000;stop-opacity:1;\""
                           "         offset=\"0\""
                           "         id=\"stop18\" /></linearGradient></defs><g"
                           "     id=\"layer1\""
                           "     transform=\"translate(-35.344479,-82.286063)\"><g"
                           "       id=\"g2\"><path"
                           "         id=\"rect18-8-14-8\""
                           "         style=\"fill:none;stroke:#b2b2b2;stroke-width:1.058;stroke-dasharray:none;stroke-opacity:1\""
                           "         d=\"m 35.873478,84.68691 v -1.058333 c 0,-0.439738 0.354012,-0.79375 0.79375,-0.79375 h 0.661458 m 3.968751,0 h 0.661458 c 0.439737,0 0.79375,0.354012 0.79375,0.79375 v 1.058333 m 0,2.116667 v 1.058333 c 0,0.439737 -0.354013,0.79375 -0.79375,0.79375 h -0.661458 m -3.968751,0 h -0.661458 c -0.439738,0 -0.79375,-0.354013 -0.79375,-0.79375 v -1.058333\" /><path"
                           "         style=\"fill:#808080;stroke:#808080;stroke-width:0.926;stroke-linecap:round;stroke-dasharray:none\""
                           "         d=\"M 38.766916,87.295288 Z\""
                           "         id=\"path7\" /><circle"
                           "         id=\"path8\""
                           "         style=\"fill:#b2b2b2;fill-opacity:1;stroke:none;stroke-width:0.100957;stroke-dasharray:none;stroke-opacity:1;paint-order:stroke fill markers\""
                           "         cx=\"39.21442\""
                           "         cy=\"88.655495\""
                           "         r=\"0.52916664\" /><path"
                           "         style=\"fill:none;fill-opacity:1;stroke:#b2b2b2;stroke-width:1.05833;stroke-linecap:butt;stroke-dasharray:none;stroke-opacity:1;paint-order:stroke fill markers\""
                           "         d=\"m 39.227347,87.526635 -0.01744,-1.074237 c 0,0 1.315828,-0.442877 1.324046,-1.914951 0.0051,-0.918266 -0.297385,-1.741379 -1.191816,-1.721968 -1.254124,0.02722 -1.185834,1.470199 -1.185834,1.470199\""
                           "         id=\"path9\" /></g></g></svg>";


static Fl_SVG_Image       show_svg(nullptr, svg_show_icon);
static Fl_SVG_Image       hide_svg(nullptr, svg_hide_icon);
static Fl_SVG_Image indet_show_svg(nullptr, svg_mix_show);

static Fl_SVG_Image       wire_svg(nullptr, svg_wire_icon);
static Fl_SVG_Image     hidden_svg(nullptr, svg_hidden_icon);
static Fl_SVG_Image      shade_svg(nullptr, svg_shade_icon);
static Fl_SVG_Image    texture_svg(nullptr, svg_texture_icon);
static Fl_SVG_Image       none_svg(nullptr, svg_none_icon);
static Fl_SVG_Image indet_surf_svg(nullptr, svg_mix_surf);

TreeIconItem::TreeIconItem( Fl_Tree* tree, const char* name ) : Fl_Tree_Item( tree )
{
    label( name );
    // Initialize 'clickable' pixmap icon

    m_ShowIconSvg = &show_svg;

    m_SurfSvgVec[ vsp::DRAW_TYPE::GEOM_DRAW_WIRE ] = &wire_svg;
    m_SurfSvgVec[ vsp::DRAW_TYPE::GEOM_DRAW_HIDDEN ] = &hidden_svg;
    m_SurfSvgVec[ vsp::DRAW_TYPE::GEOM_DRAW_SHADE ] = &shade_svg;
    m_SurfSvgVec[ vsp::DRAW_TYPE::GEOM_DRAW_TEXTURE ] = &texture_svg;
    m_SurfSvgVec[ vsp::DRAW_TYPE::GEOM_DRAW_NONE ] = &none_svg;

    m_SurfIconSvg = m_SurfSvgVec[ vsp::DRAW_TYPE::GEOM_DRAW_WIRE ];

    m_ShowState = true;
    m_SurfState = 0;
    m_EventType = 0;

    m_IndetShowState = false;
    m_IndetSurfState = false;

    m_CollapseIcon = true;
    m_HConnLineColor = tree->prefs().connectorcolor();
    m_HConnLine = tree->prefs().connectorstyle();
    m_VConnLine = tree->prefs().connectorstyle();
    m_ChildVConnLine = tree->prefs().connectorstyle();
}

// Handle custom drawing of the item
int TreeIconItem::draw_item_content( int render )
{
    int X = label_x();
    int Y = label_y();
    int W = label_w();
    int H = label_h();

    int w_icons = 0;
    int buffer = 1;
    Fl_SVG_Image* render_show = m_ShowIconSvg;
    Fl_SVG_Image* render_surf = m_SurfIconSvg;

    if ( m_IndetShowState )
    {
        render_show = &indet_show_svg;
    }
    if ( m_IndetSurfState )
    {
        render_surf = &indet_surf_svg;
    }

    // Resize svgs
    render_show->scale( 13, 11 );
    render_surf->scale( 13, 11 );

    // right justify both icons
    int yoff1 = ( H - render_show->h() ) / 2;
    int yoff2 = ( H - render_surf->h() ) / 2;

    w_icons = render_show->w() + render_surf->w() + 2*buffer;

    X = label_x() + label_w() - w_icons;

    m_ShowIconXy[0] = X;
    m_ShowIconXy[1] = Y+yoff1;
    m_ShowIconXy[2] = render_show->w();
    m_ShowIconXy[3] = render_show->h();

    X += render_show->w() + buffer;

    m_SurfIconXy[0] = X;
    m_SurfIconXy[1] = Y+yoff2;
    m_SurfIconXy[2] = render_surf->w();
    m_SurfIconXy[3] = render_surf->h();

    X = Fl_Tree_Item::draw_item_content( render );
    X += w_icons + 2*buffer;

    // Draw the svg images
    if (  render )
    {
        render_show->draw( m_ShowIconXy[0], m_ShowIconXy[1], m_ShowIconXy[2], m_ShowIconXy[3] );
        render_surf->draw( m_SurfIconXy[0], m_SurfIconXy[1], m_SurfIconXy[2], m_SurfIconXy[3] );
    }

    // return right most edge of what we've rendered
    return X;
}

void TreeIconItem::draw_horizontal_connector( int x1, int x2, int y, const Fl_Tree_Prefs &prefs )
{
    // do not draw hline for root of tree
    if ( !parent() )
    {
        return;
    }

    int x2b = x2;

    // extend line past "center" to reduce whitespace
    if ( !has_children() )
    {
        x2b += prefs.openicon_w() / 2 - 1;
    }

    fl_color( m_HConnLineColor );
    switch ( m_HConnLine )
    {
        case TREE_LINE_CONN::STYLE_NONE:
            return;
        case TREE_LINE_CONN::STYLE_DOTTED:
            {
                x1 |= 1;                          // force alignment w/dot pattern
                for ( int xx=x1; xx<=x2b; xx+=2 )
                {
                    fl_point(xx, y);
                }
                return;
            }
        case TREE_LINE_CONN::STYLE_SOLID:
            fl_line(x1,y,x2b,y);
            return;
        case TREE_LINE_CONN::STYLE_THICK:
            {
                // 3x thick line for attach
                fl_line(x1,y+1,x2b,y+1);
                fl_line(x1,y  ,x2b,y  );
                fl_line(x1,y-1,x2b,y-1);
                return;
            }
        case TREE_LINE_CONN::STYLE_DOUBLE:
            {
                // single line for hinge attach with offset for double vertical line
                fl_line(x1+1,y,x2b,y);
                return;
            }
    }
}

void TreeIconItem::draw_vertical_connector( int x, int y1, int y2, const Fl_Tree_Prefs &prefs )
{
    fl_color( prefs.connectorcolor() );
    int connLine;

    int xloc = this->x();

    // copy from draw method of Fl_Tree to re-derive hconn_x_center
    int icon_w = prefs.openicon_w();
    int hconn_x  = xloc + icon_w / 2 - 1;
    int hconn_x2 = hconn_x + prefs.connectorwidth();
    int hconn_x_center = xloc + icon_w + ( ( hconn_x2 - ( xloc + icon_w ) ) / 2 );

    // switch line type based on x location ( for self vs for children )
    if ( x >= hconn_x_center )
    {
        connLine = m_ChildVConnLine;
    }
    else
    {
        connLine = m_VConnLine;
    }

    switch ( connLine )
    {
        case TREE_LINE_CONN::STYLE_NONE:
            return;
        case TREE_LINE_CONN::STYLE_DOTTED:
            {
                y1 |= 1;                          // force alignment w/dot pattern
                y2 |= 1;                          // force alignment w/dot pattern
                for ( int yy = y1; yy <= y2; yy += 2 )
                {
                    fl_point( x, yy );
                }
                return;
            }
        case TREE_LINE_CONN::STYLE_SOLID:
            {
                y1 |= 1;                          // force alignment w/dot pattern
                y2 |= 1;                          // force alignment w/dot pattern
                fl_line( x, y1, x, y2 );
                return;
            }
        case TREE_LINE_CONN::STYLE_THICK:
            {
                // 3x line for attach
                y1 |= 1;                          // force alignment w/dot pattern
                y2 |= 1;                          // force alignment w/dot pattern
                fl_line( x+1, y1, x+1, y2 );
                fl_line( x  , y1, x  , y2 );
                fl_line( x-1, y1, x-1, y2 );
                return;
            }
        case TREE_LINE_CONN::STYLE_DOUBLE:
            {
                // double line for hinge attach
                y1 |= 1;                          // force alignment w/dot pattern
                y2 |= 1;                          // force alignment w/dot pattern
                fl_line( x+1, y1, x+1, y2 );
                fl_line( x-1, y1, x-1, y2 );
                return;
            }
    }
}

int TreeIconItem::IconHandle()
{
    int ret = 0;
    if ( EventInShow() )
    {
        m_EventType = 1;
        ret = 1;
    }
    else if ( EventInSurf() )
    {
        m_EventType = 2;
        ret = 2;
    }
    
    if ( ret > 0 )
    {
        TreeWithIcons* parent_tree = static_cast< TreeWithIcons* >( tree() );
        if ( parent_tree )
        {
            parent_tree->SetEventItem( this );
        }
    }
    return ret;
}

void TreeIconItem::IndexIcon()
{
    if ( m_EventType == 1 )
    {
        m_ShowState = !m_ShowState;
    }
    else if ( m_EventType == 2 )
    {
        m_SurfState++;
        if ( m_SurfState >= size( m_SurfSvgVec ) )
        {
            m_SurfState = 0;
        }
    }

    update_icon();
    m_EventType = 0;
}

int TreeIconItem::EventInShow()
{
    return event_inside( m_ShowIconXy );
}

int TreeIconItem::EventInSurf()
{
    return event_inside( m_SurfIconXy );
}

void TreeIconItem::SetShowState( bool show_state )
{
    m_ShowState = show_state;
    update_icon();
}

void TreeIconItem::SetSurfState( int surf_state )
{
    m_SurfState = surf_state;
    update_icon();
}

void TreeIconItem::update_icon()
{
    m_ShowIconSvg = m_ShowState ? &show_svg : &hide_svg;
    m_SurfIconSvg = m_SurfSvgVec[ m_SurfState ];
}

int TreeIconItem::event_inside(int v[4])
{
    int ret = 0;
    if ( is_visible_r() )
    {
        ret = Fl::event_inside(v[0],v[1],v[2],v[3]);
    }
    return ret;
}

int TreeIconItem::CheckParent( TreeIconItem* parent_item )
{
    for ( const Fl_Tree_Item* p = parent(); p; p = p->parent() )// move up through parents
    {
        if ( p == parent_item )
        {
            return 1;
        }
    }
    return 0;
}

TreeWithIcons::TreeWithIcons( int X, int Y, int W, int H, const char *L ) : Fl_Tree( X, Y, W, H, L )
{
    // We need the default tree icons on all platforms.
    //    For some reason someone made Fl_Tree have different icons on the Mac,
    //    which doesn't look good for this application, so we force the icons
    //    to be consistent with the '+' and '-' icons and dotted connection lines.
    connectorstyle( FL_TREE_CONNECTOR_DOTTED );
    openicon( new Fl_Pixmap( mini_tree_open_xpm ) );
    closeicon( new Fl_Pixmap( mini_tree_close_xpm ) );
    ClearEventItem();
}

TreeIconItem* TreeWithIcons::AddRow( const char *s, TreeIconItem *parent_item )
{
    TreeIconItem *item = new TreeIconItem( this, s );

    // if called without a parent item argument, check root assignment
    if ( !parent_item )
    {
        // if TreeWithIcons has no root assigned or is on default root, designate this new item as root
        if ( !root() || ( strcmp( root()->label(), "ROOT" ) == 0 ) )
        {
            this->root( item );

            // return created TreeIconItem
            return item;
        }
        else
        {
            // use root as parent
            parent_item = ( TreeIconItem* )root();
        }
    }

    // if parent item is in args, attach new item to it.
    parent_item->add( prefs(), "", item );

    // return created TreeIconItem
    return item;
}

// Get first item with matching ref_id
TreeIconItem* TreeWithIcons::GetItemByRefId( const string & ref_id )
{
    TreeIconItem* ret_item = nullptr;

    for ( Fl_Tree_Item *tree_item = first(); tree_item; tree_item = next(tree_item) )
    {
        TreeIconItem* tree_icon_item = static_cast< TreeIconItem* >( tree_item );
        if ( tree_icon_item && tree_icon_item->GetRefID() == ref_id )
        {
            return tree_icon_item;
        }
    }

    return ret_item;
}

void TreeWithIcons::GetSelectedItems( vector < TreeIconItem* > *item_vec )
{
    Fl_Tree_Item_Array base_array;

    get_selected_items( base_array );

    for( int i = 0; i != base_array.total(); ++i )
    {
        TreeIconItem* tree_icon_item = static_cast < TreeIconItem* >( base_array[ i ] );
        if ( tree_icon_item )
        {
            item_vec->push_back( tree_icon_item );
        }
    }
}

int TreeWithIcons::handle( int e )
{
    int ret = 0;
    switch ( e )
    {
        case FL_PUSH:
        {
            // Check all icons for events prior to normal tree event handling
            // Icon events MUST precede standard events in the geomtree to avoid successive errant callbacks from deselect method
            for ( Fl_Tree_Item *tree_item = first(); tree_item; tree_item = next(tree_item) )
            {
                TreeIconItem* tree_icon_item = static_cast< TreeIconItem* >( tree_item );

                // check for positive icon event; trigger icon callback for first successful event detection
                if ( tree_icon_item->IconHandle() )
                {
                    do_callback_for_item( tree_icon_item, FL_TREE_REASON_NONE );

                    // eclipse event handling for Fl_Tree if performing icon callback
                    return 1;
                }
            }
            ret = 1;
            break;
        }
        default:
            ret = 1;
            break;
    }
    return( Fl_Tree::handle(e) ? 1 : ret );
}
