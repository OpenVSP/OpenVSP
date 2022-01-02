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
