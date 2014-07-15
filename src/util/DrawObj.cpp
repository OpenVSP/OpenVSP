//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//

// DrawObj.h: 
// J.R Gloudemans
//
//////////////////////////////////////////////////////////////////////


#include "DrawObj.h"
#include <math.h>

//====================== Contructor ======================//
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

    m_Priority = 0;

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
