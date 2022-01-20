//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//

// SVGUtil.cpp
// Justin Gravett
//
//////////////////////////////////////////////////////////////////////

#include "SVGUtil.h"

void WriteSVGHeader( xmlNodePtr root, const BndBox &svgbox )
{
    double scale = svgbox.GetLargestDist();

    // Set Viewbox:
    double x_view = scale * 5;
    double y_view = scale * 4;

    // Convert vectors to strings:
    char sviewbox[255];

    sprintf( sviewbox, "%f %f %f %f", 0.0, 0.0, x_view, y_view );

    xmlSetProp( root, BAD_CAST "width", BAD_CAST "100%" );
    xmlSetProp( root, BAD_CAST "height", BAD_CAST "100%" );
    xmlSetProp( root, BAD_CAST "viewBox", BAD_CAST sviewbox );
    xmlSetProp( root, BAD_CAST "xmlns", BAD_CAST "http://www.w3.org/2000/svg" );
    xmlSetProp( root, BAD_CAST "version", BAD_CAST "1.1" );

    // Draw Border Box:
    double x_min = scale / 100;
    double y_min = x_min;
    double stroke_width = scale / 50;
    double width = x_view - stroke_width;
    double height = y_view - 2 * stroke_width;

    xmlNodePtr rect_node = xmlNewChild( root, NULL, BAD_CAST "rect", NULL );

    XmlUtil::SetDoubleProp( rect_node, "x", x_min );
    XmlUtil::SetDoubleProp( rect_node, "y", y_min );
    XmlUtil::SetDoubleProp( rect_node, "width", width );
    XmlUtil::SetDoubleProp( rect_node, "height", height );
    xmlSetProp( rect_node, BAD_CAST "fill", BAD_CAST "white" );
    xmlSetProp( rect_node, BAD_CAST "stroke", BAD_CAST "black" );
    XmlUtil::SetDoubleProp( rect_node, "stroke-width", stroke_width );
}

void WriteSVGScaleBar( xmlNodePtr root, const int &View, const BndBox &svgbox, const int &LenUnit, const double &convert_scale )
{
    vec3d scalevec = svgbox.GetMax() - svgbox.GetMin();
    double scale = svgbox.GetLargestDist();

    if ( LenUnit == vsp::LEN_UNITLESS )
    {
        return; // Dimensionless drawing implies no scale bar
    }

    string unit;
    switch ( LenUnit )
    {
    case vsp::LEN_MM:
        unit = "(mm)";
        break;
    case vsp::LEN_CM:
        unit = "(cm)";
        break;
    case vsp::LEN_M:
        unit = "(m)";
        break;
    case vsp::LEN_IN:
        unit = "(in)";
        break;
    case vsp::LEN_FT:
        unit = "(ft)";
        break;
    case vsp::LEN_YD:
        unit = "(yd)";
        break;
    }

    // Establish new coordinate system with scale bar aligned with feature lines
    double xtrans_right = scale * 2 + scalevec.x();
    double xtrans_left = scale * 2;
    double ytrans = scale * 3.75;

    xmlNodePtr trans_node = xmlNewChild( root, NULL, BAD_CAST "g", NULL );

    if ( View == vsp::VIEW_NUM::VIEW_4 || View == vsp::VIEW_NUM::VIEW_2HOR )
    {
        string transform = "translate(" + std::to_string( xtrans_right ) + ", " + std::to_string( ytrans ) + ")";
        xmlSetProp( trans_node, BAD_CAST "transform", BAD_CAST transform.c_str() );
    }
    else
    {
        string transform = "translate(" + std::to_string( xtrans_left ) + ", " + std::to_string( ytrans ) + ")";
        xmlSetProp( trans_node, BAD_CAST "transform", BAD_CAST transform.c_str() );
    }

    // Add scale bar:
    double height = convert_scale / 22;
    double width = convert_scale;

    xmlNodePtr rect_node1 = xmlNewChild( trans_node, NULL, BAD_CAST "rect", NULL );

    xmlSetProp( rect_node1, BAD_CAST "x", BAD_CAST "0" );
    xmlSetProp( rect_node1, BAD_CAST "y", BAD_CAST "0" );
    XmlUtil::SetDoubleProp( rect_node1, "width", width );
    xmlSetProp( rect_node1, BAD_CAST "fill", BAD_CAST "black" );

    AddDefaultScalBarProps( rect_node1, convert_scale );

    double width_eighth = convert_scale / 8.0;

    xmlNodePtr rect_node2 = xmlNewChild( trans_node, NULL, BAD_CAST "rect", NULL );

    XmlUtil::SetDoubleProp( rect_node2, "x", width_eighth );
    xmlSetProp( rect_node2, BAD_CAST "y", BAD_CAST "0" );
    XmlUtil::SetDoubleProp( rect_node2, "width", width_eighth );
    xmlSetProp( rect_node2, BAD_CAST "fill", BAD_CAST "white" );

    AddDefaultScalBarProps( rect_node2, convert_scale );

    double x_mid = convert_scale / 2.0;

    double width_tot = double( convert_scale );

    xmlNodePtr rect_node3 = xmlNewChild( trans_node, NULL, BAD_CAST "rect", NULL );

    xmlSetProp( rect_node3, BAD_CAST "x", BAD_CAST "0" );
    XmlUtil::SetDoubleProp( rect_node3, "y", height );
    XmlUtil::SetDoubleProp( rect_node3, "width", width_tot );
    xmlSetProp( rect_node3, BAD_CAST "fill", BAD_CAST "white" );

    AddDefaultScalBarProps( rect_node3, convert_scale );

    xmlNodePtr rect_node4 = xmlNewChild( trans_node, NULL, BAD_CAST "rect", NULL );

    XmlUtil::SetDoubleProp( rect_node4, "x", width_eighth );
    XmlUtil::SetDoubleProp( rect_node4, "y", height );
    XmlUtil::SetDoubleProp( rect_node4, "width", width_eighth );
    xmlSetProp( rect_node4, BAD_CAST "fill", BAD_CAST "black" );

    AddDefaultScalBarProps( rect_node4, convert_scale );

    xmlNodePtr rect_node5 = xmlNewChild( trans_node, NULL, BAD_CAST "rect", NULL );

    XmlUtil::SetDoubleProp( rect_node5, "x", x_mid );
    xmlSetProp( rect_node5, BAD_CAST "y", BAD_CAST "0" );
    XmlUtil::SetDoubleProp( rect_node5, "width", x_mid );
    xmlSetProp( rect_node5, BAD_CAST "fill", BAD_CAST "white" );

    AddDefaultScalBarProps( rect_node5, convert_scale );

    xmlNodePtr rect_node6 = xmlNewChild( trans_node, NULL, BAD_CAST "rect", NULL );

    XmlUtil::SetDoubleProp( rect_node6, "x", x_mid  );
    XmlUtil::SetDoubleProp( rect_node6, "y", height );
    XmlUtil::SetDoubleProp( rect_node6, "width", x_mid );
    xmlSetProp( rect_node6, BAD_CAST "fill", BAD_CAST "black" );

    AddDefaultScalBarProps( rect_node6, convert_scale );

    double x_begin = convert_scale / -50;
    double y_begin = convert_scale / -100;
    double font_size = convert_scale / 13;

    // Convert to string:
    string sfont_size = std::to_string( font_size ) + "px";

    xmlNodePtr text_node1 = XmlUtil::AddIntNode( trans_node, "text", 0 );

    XmlUtil::SetDoubleProp( text_node1, "x", x_begin );
    XmlUtil::SetDoubleProp( text_node1, "y", y_begin );
    xmlSetProp( text_node1, BAD_CAST "font-family", BAD_CAST "sans-serif" );
    xmlSetProp( text_node1, BAD_CAST "font-size", BAD_CAST sfont_size.c_str() );
    xmlSetProp( text_node1, BAD_CAST "fill", BAD_CAST "black" );

    xmlNodePtr text_node2 = XmlUtil::AddStringNode( trans_node, "text", unit );

    // Align unit label
    double x_unit;
    if ( LenUnit == vsp::LEN_M )
    {
        x_unit = x_mid - font_size * 0.75;
    }
    else 
    {
        x_unit = x_mid - font_size * 0.73;
    }

    XmlUtil::SetDoubleProp( text_node2, "x", x_unit );

    double y_mid = convert_scale / -50.0;

    XmlUtil::SetDoubleProp( text_node2, "y", y_mid );
    xmlSetProp( text_node2, BAD_CAST "font-family", BAD_CAST "sans-serif" );
    xmlSetProp( text_node2, BAD_CAST "font-size", BAD_CAST sfont_size.c_str() );
    xmlSetProp( text_node2, BAD_CAST "fill", BAD_CAST "black" );

    double x_end = convert_scale * 0.95;

    string string_scale;
    char numstr[255];

    // Align scale label
    if ( convert_scale < 1 || convert_scale >= 1000 )
    {
        sprintf( numstr, "%.1e", convert_scale );
        string_scale = numstr;
        x_end -= 0.125 * x_end;
    }
    else
    {
        int int_scale = convert_scale;
        sprintf( numstr, "%d", int_scale );
        string_scale = numstr;
    }

    xmlNodePtr text_node3 = XmlUtil::AddStringNode( trans_node, "text", string_scale.c_str() );

    XmlUtil::SetDoubleProp( text_node3, "x", x_end );
    XmlUtil::SetDoubleProp( text_node3, "y", y_begin );
    xmlSetProp( text_node3, BAD_CAST "font-family", BAD_CAST "sans-serif" );
    xmlSetProp( text_node3, BAD_CAST "font-size", BAD_CAST sfont_size.c_str() );
    xmlSetProp( text_node3, BAD_CAST "fill", BAD_CAST "black" );

}

xmlNodePtr AddDefaultScalBarProps( xmlNodePtr & node, const double &convert_scale )
{
    double height = convert_scale / 22;
    double stroke_width = convert_scale / 250;

    XmlUtil::SetDoubleProp( node, "height", height );
    xmlSetProp( node, BAD_CAST "fill-opacity", BAD_CAST "1" );
    XmlUtil::SetDoubleProp( node, "stroke-width", stroke_width );
    xmlSetProp( node, BAD_CAST "stroke", BAD_CAST "black" );
    xmlSetProp( node, BAD_CAST "stroke-miterlimit", BAD_CAST "4" );
    xmlSetProp( node, BAD_CAST "stroke-dasharray", BAD_CAST "none" );

    return node;
}

void WriteSVGPolylines2D( xmlNodePtr root, const vector < vector < vec3d > > &allflines, const BndBox &svgbox )
{
    vec3d scalevec = svgbox.GetMax() - svgbox.GetMin();
    int maj_comp = scalevec.major_comp();
    double scale = scalevec[maj_comp];

    //Establish a new coordinate system:
    double x_trans = scale * 2;
    double y_trans = scale * 2.5;

    string transform = "translate(" + std::to_string( x_trans ) + ", " + std::to_string( y_trans ) + ")";

    xmlNodePtr trans_node = xmlNewChild( root, NULL, BAD_CAST "g", NULL );
    
    xmlSetProp( trans_node, BAD_CAST "transform", BAD_CAST transform.c_str() );

    //Feature Line Points:
    xmlNodePtr group_node = xmlNewChild( trans_node, NULL, BAD_CAST "g", NULL );

    double stroke_width = scale / 300;

    for ( unsigned int l = 0; l < allflines.size(); l++ )
    {        
        xmlNodePtr poly_node = xmlNewChild( group_node, NULL, BAD_CAST "polyline", NULL );

        xmlSetProp( poly_node, BAD_CAST "fill", BAD_CAST "none" );
        xmlSetProp( poly_node, BAD_CAST "stroke", BAD_CAST "black" );
        XmlUtil::SetDoubleProp( poly_node, "stroke-width", stroke_width );
        
        string sflines;

        // Note: Flip y component to match svg coordinate axis directions
        for ( unsigned int j = 0; j < allflines[l].size(); j++ )
        {
            sflines += std::to_string( allflines[l][j].x() ) + " " + std::to_string( allflines[l][j].y() * -1 ) + " ";
        }
        xmlSetProp( poly_node, BAD_CAST "points", BAD_CAST sflines.c_str() );
    }
}