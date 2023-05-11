//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//

// DrawObj.h: 
// J.R Gloudemans
//
//////////////////////////////////////////////////////////////////////


#include "DrawObj.h"
#include "Matrix4d.h"

void MakeArrowhead( const vec3d &ptip, const vec3d &uref, double len, vector < vec3d > &pts )
{
    double fr = 0.2;

    vec3d u = uref;
    u.normalize();

    if ( u.mag() < 1e-6 )
    {
        printf("Zero direction vector in MakeArrowhead!\n");
        return;
    }

    vec3d v, w;
    v.v[ u.minor_comp() ] = 1.0;
    w = cross( u, v );
    w.normalize();
    v = cross( w, u );
    v.normalize();

    vec3d p = ptip - len * u;
    vec3d p1 = p + fr * len * v;
    vec3d p2 = p + fr * len * w;
    vec3d p3 = p - fr * len * v;
    vec3d p4 = p - fr * len * w;

    pts.reserve( pts.size() + 18 );
    pts.push_back( p1 );
    pts.push_back( p2 );
    pts.push_back( p3 );

    pts.push_back( p1 );
    pts.push_back( p3 );
    pts.push_back( p4 );

    pts.push_back( p1 );
    pts.push_back( p2 );
    pts.push_back( ptip );

    pts.push_back( p2 );
    pts.push_back( p3 );
    pts.push_back( ptip );

    pts.push_back( p3 );
    pts.push_back( p4 );
    pts.push_back( ptip );

    pts.push_back( p4 );
    pts.push_back( p1 );
    pts.push_back( ptip );
}

void MakeArrowhead( const vec3d &ptip, const vec3d &uref, double len, DrawObj &dobj )
{
    MakeArrowhead( ptip, uref, len, dobj.m_PntVec );

    dobj.m_LineWidth = 1.0;
    dobj.m_Type = DrawObj::VSP_SHADED_TRIS;
    dobj.m_NormVec = vector <vec3d> ( dobj.m_PntVec.size() );

    for ( int i = 0; i < 4; i++ )
    {
        dobj.m_MaterialInfo.Ambient[i] = 0.2f;
        dobj.m_MaterialInfo.Diffuse[i] = 0.1f;
        dobj.m_MaterialInfo.Specular[i] = 0.7f;
        dobj.m_MaterialInfo.Emission[i] = 0.0f;
    }
    dobj.m_MaterialInfo.Diffuse[3] = 0.5;
    dobj.m_MaterialInfo.Shininess = 5.0;

    dobj.m_GeomChanged = true;
}

void MakeCircle( const vec3d &pcen, const vec3d &norm, const vec3d &pstart, vector < vec3d > &pts, int nseg )
{
    pts.reserve( pts.size() + 2 * nseg );
    for ( int i = 0; i < nseg; i++ )
    {
        double theta = i * 2.0 * PI / nseg;
        pts.push_back( RotateArbAxis( pstart, theta, norm ) + pcen );
        theta = ( i + 1 ) * 2.0 * PI / nseg;
        pts.push_back( RotateArbAxis( pstart, theta, norm ) + pcen );
    }
}

void MakeCircle( const vec3d &pcen, const vec3d &norm, double rad, vector < vec3d > &pts, int nseg )
{
    vec3d n = norm;
    n.normalize();

    vec3d ref;
    ref.v[ n.minor_comp() ] = 1.0;

    vec3d v = cross( n, ref );
    v.normalize();
    vec3d u = cross( v, n );
    u.normalize();

    vec3d pstart = u * rad;

    MakeCircle( pcen, n, pstart, pts, nseg );

}

void MakeCircle( const vec3d &pcen, const vec3d &norm, double rad, DrawObj &dobj )
{
    MakeCircle( pcen, norm, rad, dobj.m_PntVec );

    dobj.m_LineWidth = 2.0;
    dobj.m_Type = DrawObj::VSP_LINES;
    dobj.m_LineColor = vec3d( 0, 0, 0 );
    dobj.m_GeomChanged = true;
}

void MakeCircleArrow( const vec3d &pcen, const vec3d &norm, double rad, DrawObj &dobj, DrawObj &arrow )
{
    double lenfrac = 0.6;

    vec3d n = norm;
    n.normalize();

    vec3d ref;
    ref.v[ n.minor_comp() ] = 1.0;

    vec3d v = cross( n, ref );
    v.normalize();
    vec3d u = cross( v, n );
    u.normalize();

    vec3d pstart = u * rad;

    MakeCircle( pcen, n, pstart, dobj.m_PntVec );
    dobj.m_LineWidth = 2.0;
    dobj.m_Type = DrawObj::VSP_LINES;
    dobj.m_LineColor = vec3d( 0, 0, 0 );
    dobj.m_GeomChanged = true;


    vector < vec3d > arrowpts;
    MakeArrowhead( vec3d( 0, 0, 0 ), v, rad * lenfrac, arrowpts );

    Matrix4d mat;
    mat.translatev( pcen + pstart );
    mat.rotate( atan( lenfrac * 0.5 ), n );
    mat.xformvec( arrowpts );

    arrow.m_PntVec.insert( arrow.m_PntVec.end(), arrowpts.begin(), arrowpts.end() );

    arrow.m_LineWidth = 1.0;
    arrow.m_Type = DrawObj::VSP_SHADED_TRIS;
    arrow.m_NormVec = vector <vec3d> ( dobj.m_PntVec.size() );

    for ( int i = 0; i < 4; i++ )
    {
        arrow.m_MaterialInfo.Ambient[i] = 0.2f;
        arrow.m_MaterialInfo.Diffuse[i] = 0.1f;
        arrow.m_MaterialInfo.Specular[i] = 0.7f;
        arrow.m_MaterialInfo.Emission[i] = 0.0f;
    }
    arrow.m_MaterialInfo.Diffuse[3] = 0.5f;
    arrow.m_MaterialInfo.Shininess = 5.0f;

    arrow.m_GeomChanged = true;
}

void MakeDashedLine( const vec3d &pstart, const vec3d &pend, int ndash, vector < vec3d > &dashpts )
{
    int npt = 2 * ndash;
    dashpts.reserve( dashpts.size() + npt );

    vec3d dpt = ( pend - pstart ) * ( 1.0 / ( npt - 1 ) );
    for ( int i = 0; i < npt; i++ )
    {
        dashpts.push_back( pstart + static_cast < double > ( i ) * dpt );
    }
}

//====================== Constructor ======================//
DrawObj::DrawObj()
{
    m_GeomID = "Default";

    m_Visible = true;

    m_GeomChanged = true;

    m_FlipNormals = false;

    m_Type = DrawObj::VSP_POINTS;

    m_Screen = DrawObj::VSP_MAIN_SCREEN;

    m_LineWidth = 1.0;
    m_LineColor = vec3d( 0, 0, 1 );

    m_PointSize = 10.0;
    m_PointColor = vec3d( 1, 0, 0 );

    m_TextSize = 0;

    m_Ruler.Step = DrawObj::VSP_RULER_STEP_ZERO;

    m_MaterialInfo.Ambient[0] = m_MaterialInfo.Ambient[1] = 
        m_MaterialInfo.Ambient[2] = m_MaterialInfo.Ambient[3] = 1.0f;

    m_MaterialInfo.Diffuse[0] = m_MaterialInfo.Diffuse[1] = 
        m_MaterialInfo.Diffuse[2] = m_MaterialInfo.Diffuse[3] = 1.0f;

    m_MaterialInfo.Specular[0] = m_MaterialInfo.Specular[1] = 
        m_MaterialInfo.Specular[2] = m_MaterialInfo.Specular[3] = 1.0f;

    m_MaterialInfo.Emission[0] = m_MaterialInfo.Emission[1] = 
        m_MaterialInfo.Emission[2] = m_MaterialInfo.Emission[3] = 1.0f;

    m_MaterialInfo.Shininess = 1.0f;

    m_ClipLoc = vector< double >( 6, 0 );
    m_ClipFlag = vector< bool >( 6, false );
}

DrawObj::~DrawObj()
{
}

vec3d DrawObj::ColorWheel( double angle )
{
    // Returns rgb for an angle in degrees on color wheel
    // 0 degrees is Red, 120 degrees is Green, 240 degrees is Blue
    double r, g, b;

    if ( angle >= 0 && angle < 120 )
    {
        r = 1 - angle / 120;
        g = angle / 120;
        b = 0;
    }
    else if ( angle >= 120 && angle < 240 )
    {
        r = 0;
        g = 1 - ( angle - 120.0 ) / 120;
        b = ( angle - 120.0 ) / 120;
    }
    else if ( angle >= 240 && angle < 360 )
    {
        r = ( angle - 240.0 ) / 120.0;
        g = 0;
        b = 1 - ( angle - 240.0 ) / 120.0;
    }
    else
    {
        r = g = b = 0;
    }

    return vec3d( r, g, b );

}

vec3d DrawObj::Color( int color )
{
    switch( color ) {
        case AQUA:
            return vec3d( 0.0, 1.0, 1.0 );
            break;
        case BLACK:
            return vec3d( 0.0, 0.0, 0.0 );
            break;
        case BLUE:
            return vec3d( 0.0, 0.0, 1.0 );
            break;
        case FUCHSIA:
            return vec3d( 1.0, 0.0, 1.0 );
            break;
        case GRAY:
            return vec3d( 0.5, 0.5, 0.5 );
            break;
        case GREEN:
            return vec3d( 0.0, 0.5, 0.0 );
            break;
        case LIME:
            return vec3d( 0.0, 1.0, 0.0 );
            break;
        case MAROON:
            return vec3d( 0.5, 0.0, 0.0 );
            break;
        case NAVY:
            return vec3d( 0.0, 0.0, 0.5 );
            break;
        case OLIVE:
            return vec3d( 0.5, 0.5, 0.0 );
            break;
        case PURPLE:
            return vec3d( 0.5, 0.0, 0.5 );
            break;
        case RED:
            return vec3d( 1.0, 0.0, 0.0 );
            break;
        case SILVER:
            return vec3d( 0.75, 0.75, 0.75 );
            break;
        case TEAL:
            return vec3d( 0.0, 0.5, 0.5 );
            break;
        case WHITE:
            return vec3d( 1.0, 1.0, 1.0 );
            break;
        case YELLOW:
            return vec3d( 1.0, 1.0, 0.0 );
            break;
        case ALICE_BLUE:
            return vec3d( 240, 248, 255 ) / 255.0;
            break;
        case ANTIQUE_WHITE:
            return vec3d( 250, 235, 215 ) / 255.0;
            break;
        case AQUA_MARINE:
            return vec3d( 127, 255, 212 ) / 255.0;
            break;
        case AZURE:
            return vec3d( 240, 255, 255 ) / 255.0;
            break;
        case BEIGE:
            return vec3d( 245, 245, 220 ) / 255.0;
            break;
        case BISQUE:
            return vec3d( 255, 228, 196 ) / 255.0;
            break;
        case BLANCHED_ALMOND:
            return vec3d( 255, 235, 205 ) / 255.0;
            break;
        case BLUE_VIOLET:
            return vec3d( 138, 43, 226 ) / 255.0;
            break;
        case BROWN:
            return vec3d( 165, 42, 42 ) / 255.0;
            break;
        case BURLY_WOOD:
            return vec3d( 222, 184, 135 ) / 255.0;
            break;
        case CADET_BLUE:
            return vec3d( 95, 158, 160 ) / 255.0;
            break;
        case CHARTREUSE:
            return vec3d( 127, 255, 0 ) / 255.0;
            break;
        case CHOCOLATE:
            return vec3d( 210, 105, 30 ) / 255.0;
            break;
        case CORAL:
            return vec3d( 255, 127, 80 ) / 255.0;
            break;
        case CORN_FLOWER_BLUE:
            return vec3d( 100, 149, 237 ) / 255.0;
            break;
        case CORN_SILK:
            return vec3d( 255, 248, 220 ) / 255.0;
            break;
        case CRIMSON:
            return vec3d( 220, 20, 60 ) / 255.0;
            break;
        case DARK_BLUE:
            return vec3d( 0, 0, 139 ) / 255.0;
            break;
        case DARK_CYAN:
            return vec3d( 0, 139, 139 ) / 255.0;
            break;
        case DARK_GOLDEN_ROD:
            return vec3d( 184, 134, 11 ) / 255.0;
            break;
        case DARK_GRAY:
            return vec3d( 169, 169, 169 ) / 255.0;
            break;
        case DARK_GREEN:
            return vec3d( 0, 100, 0 ) / 255.0;
            break;
        case DARK_KHAKI:
            return vec3d( 189, 183, 107 ) / 255.0;
            break;
        case DARK_MAGENTA:
            return vec3d( 139, 0, 139 ) / 255.0;
            break;
        case DARK_OLIVE_GREEN:
            return vec3d( 85, 107, 47 ) / 255.0;
            break;
        case DARK_ORANGE:
            return vec3d( 255, 140, 0 ) / 255.0;
            break;
        case DARK_ORCHID:
            return vec3d( 153, 50, 204 ) / 255.0;
            break;
        case DARK_RED:
            return vec3d( 139, 0, 0 ) / 255.0;
            break;
        case DARK_SALMON:
            return vec3d( 233, 150, 122 ) / 255.0;
            break;
        case DARK_SEA_GREEN:
            return vec3d( 143, 188, 143 ) / 255.0;
            break;
        case DARK_SLATE_BLUE:
            return vec3d( 72, 61, 139 ) / 255.0;
            break;
        case DARK_SLATE_GRAY:
            return vec3d( 47, 79, 79 ) / 255.0;
            break;
        case DARK_TURQUOISE:
            return vec3d( 0, 206, 209 ) / 255.0;
            break;
        case DARK_VIOLET:
            return vec3d( 148, 0, 211 ) / 255.0;
            break;
        case DEEP_PINK:
            return vec3d( 255, 20, 147 ) / 255.0;
            break;
        case DEEP_SKY_BLUE:
            return vec3d( 0, 191, 255 ) / 255.0;
            break;
        case DIM_GRAY:
            return vec3d( 105, 105, 105 ) / 255.0;
            break;
        case DODGER_BLUE:
            return vec3d( 30, 144, 255 ) / 255.0;
            break;
        case FIREBRICK:
            return vec3d( 178, 34, 34 ) / 255.0;
            break;
        case FLORAL_WHITE:
            return vec3d( 255, 250, 240 ) / 255.0;
            break;
        case FOREST_GREEN:
            return vec3d( 34, 139, 34 ) / 255.0;
            break;
        case GAINSBORO:
            return vec3d( 220, 220, 220 ) / 255.0;
            break;
        case GHOST_WHITE:
            return vec3d( 248, 248, 255 ) / 255.0;
            break;
        case GOLD:
            return vec3d( 255, 215, 0 ) / 255.0;
            break;
        case GOLDEN_ROD:
            return vec3d( 218, 165, 32 ) / 255.0;
            break;
        case GREEN_YELLOW:
            return vec3d( 173, 255, 47 ) / 255.0;
            break;
        case HONEYDEW:
            return vec3d( 240, 255, 240 ) / 255.0;
            break;
        case HOT_PINK:
            return vec3d( 255, 105, 180 ) / 255.0;
            break;
        case INDIAN_RED:
            return vec3d( 205, 92, 92 ) / 255.0;
            break;
        case INDIGO:
            return vec3d( 75, 0, 130 ) / 255.0;
            break;
        case IVORY:
            return vec3d( 255, 255, 240 ) / 255.0;
            break;
        case KHAKI:
            return vec3d( 240, 230, 140 ) / 255.0;
            break;
        case LAVENDER:
            return vec3d( 230, 230, 250 ) / 255.0;
            break;
        case LAVENDER_BLUSH:
            return vec3d( 255, 240, 245 ) / 255.0;
            break;
        case LAWN_GREEN:
            return vec3d( 124, 252, 0 ) / 255.0;
            break;
        case LEMON_CHIFFON:
            return vec3d( 255, 250, 205 ) / 255.0;
            break;
        case LIGHT_BLUE:
            return vec3d( 173, 216, 230 ) / 255.0;
            break;
        case LIGHT_CORAL:
            return vec3d( 240, 128, 128 ) / 255.0;
            break;
        case LIGHT_CYAN:
            return vec3d( 224, 255, 255 ) / 255.0;
            break;
        case LIGHT_GOLDEN_ROD_YELLOW:
            return vec3d( 250, 250, 210 ) / 255.0;
            break;
        case LIGHT_GRAY:
            return vec3d( 211, 211, 211 ) / 255.0;
            break;
        case LIGHT_GREEN:
            return vec3d( 144, 238, 144 ) / 255.0;
            break;
        case LIGHT_PINK:
            return vec3d( 255, 182, 193 ) / 255.0;
            break;
        case LIGHT_SALMON:
            return vec3d( 255, 160, 122 ) / 255.0;
            break;
        case LIGHT_SEA_GREEN:
            return vec3d( 32, 178, 170 ) / 255.0;
            break;
        case LIGHT_SKY_BLUE:
            return vec3d( 135, 206, 250 ) / 255.0;
            break;
        case LIGHT_SLATE_GRAY:
            return vec3d( 119, 136, 153 ) / 255.0;
            break;
        case LIGHT_STEEL_BLUE:
            return vec3d( 176, 196, 222 ) / 255.0;
            break;
        case LIGHT_YELLOW:
            return vec3d( 255, 255, 224 ) / 255.0;
            break;
        case LIME_GREEN:
            return vec3d( 50, 205, 50 ) / 255.0;
            break;
        case LINEN:
            return vec3d( 250, 240, 230 ) / 255.0;
            break;
        case MEDIUM_AQUA_MARINE:
            return vec3d( 102, 205, 170 ) / 255.0;
            break;
        case MEDIUM_BLUE:
            return vec3d( 0, 0, 205 ) / 255.0;
            break;
        case MEDIUM_ORCHID:
            return vec3d( 186, 85, 211 ) / 255.0;
            break;
        case MEDIUM_PURPLE:
            return vec3d( 147, 112, 219 ) / 255.0;
            break;
        case MEDIUM_SEA_GREEN:
            return vec3d( 60, 179, 113 ) / 255.0;
            break;
        case MEDIUM_SLATE_BLUE:
            return vec3d( 123, 104, 238 ) / 255.0;
            break;
        case MEDIUM_SPRING_GREEN:
            return vec3d( 0, 250, 154 ) / 255.0;
            break;
        case MEDIUM_TURQUOISE:
            return vec3d( 72, 209, 204 ) / 255.0;
            break;
        case MEDIUM_VIOLET_RED:
            return vec3d( 199, 21, 133 ) / 255.0;
            break;
        case MIDNIGHT_BLUE:
            return vec3d( 25, 25, 112 ) / 255.0;
            break;
        case MINT_CREAM:
            return vec3d( 245, 255, 250 ) / 255.0;
            break;
        case MISTY_ROSE:
            return vec3d( 255, 228, 225 ) / 255.0;
            break;
        case MOCCASIN:
            return vec3d( 255, 228, 181 ) / 255.0;
            break;
        case NAVAJO_WHITE:
            return vec3d( 255, 222, 173 ) / 255.0;
            break;
        case OLD_LACE:
            return vec3d( 253, 245, 230 ) / 255.0;
            break;
        case OLIVE_DRAB:
            return vec3d( 107, 142, 35 ) / 255.0;
            break;
        case ORANGE:
            return vec3d( 255, 165, 0 ) / 255.0;
            break;
        case ORANGE_RED:
            return vec3d( 255, 69, 0 ) / 255.0;
            break;
        case ORCHID:
            return vec3d( 218, 112, 214 ) / 255.0;
            break;
        case PALE_GOLDEN_ROD:
            return vec3d( 238, 232, 170 ) / 255.0;
            break;
        case PALE_GREEN:
            return vec3d( 152, 251, 152 ) / 255.0;
            break;
        case PALE_TURQUOISE:
            return vec3d( 175, 238, 238 ) / 255.0;
            break;
        case PALE_VIOLET_RED:
            return vec3d( 219, 112, 147 ) / 255.0;
            break;
        case PAPAYA_WHIP:
            return vec3d( 255, 239, 213 ) / 255.0;
            break;
        case PEACH_PUFF:
            return vec3d( 255, 218, 185 ) / 255.0;
            break;
        case PERU:
            return vec3d( 205, 133, 63 ) / 255.0;
            break;
        case PINK:
            return vec3d( 255, 192, 203 ) / 255.0;
            break;
        case PLUM:
            return vec3d( 221, 160, 221 ) / 255.0;
            break;
        case POWDER_BLUE:
            return vec3d( 176, 224, 230 ) / 255.0;
            break;
        case ROSY_BROWN:
            return vec3d( 188, 143, 143 ) / 255.0;
            break;
        case ROYAL_BLUE:
            return vec3d( 65, 105, 225 ) / 255.0;
            break;
        case SADDLE_BROWN:
            return vec3d( 139, 69, 19 ) / 255.0;
            break;
        case SALMON:
            return vec3d( 250, 128, 114 ) / 255.0;
            break;
        case SANDY_BROWN:
            return vec3d( 244, 164, 96 ) / 255.0;
            break;
        case SEA_GREEN:
            return vec3d( 46, 139, 87 ) / 255.0;
            break;
        case SEA_SHELL:
            return vec3d( 255, 245, 238 ) / 255.0;
            break;
        case SIENNA:
            return vec3d( 160, 82, 45 ) / 255.0;
            break;
        case SKY_BLUE:
            return vec3d( 135, 206, 235 ) / 255.0;
            break;
        case SLATE_BLUE:
            return vec3d( 106, 90, 205 ) / 255.0;
            break;
        case SLATE_GRAY:
            return vec3d( 112, 128, 144 ) / 255.0;
            break;
        case SNOW:
            return vec3d( 255, 250, 250 ) / 255.0;
            break;
        case SPRING_GREEN:
            return vec3d( 0, 255, 127 ) / 255.0;
            break;
        case STEEL_BLUE:
            return vec3d( 70, 130, 180 ) / 255.0;
            break;
        case TAN:
            return vec3d( 210, 180, 140 ) / 255.0;
            break;
        case THISTLE:
            return vec3d( 216, 191, 216 ) / 255.0;
            break;
        case TOMATO:
            return vec3d( 255, 99, 71 ) / 255.0;
            break;
        case TURQUOISE:
            return vec3d( 64, 224, 208 ) / 255.0;
            break;
        case VIOLET:
            return vec3d( 238, 130, 238 ) / 255.0;
            break;
        case WHEAT:
            return vec3d( 245, 222, 179 ) / 255.0;
            break;
        case WHITE_SMOKE:
            return vec3d( 245, 245, 245 ) / 255.0;
            break;
        case YELLOW_GREEN:
            return vec3d( 154, 205, 50 ) / 255.0;
            break;
        default:
            return vec3d( 0.0, 0.0, 0.0 );
            break;
    }
}
