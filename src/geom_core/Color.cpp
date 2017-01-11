#include "Color.h"

Color::Color()
{
    m_Red.Init( "Red", "Color_Parm", this, 0, 0, 255 );
    m_Green.Init( "Green", "Color_Parm", this, 0, 0, 255 );
    m_Blue.Init( "Blue", "Color_Parm", this, 255, 0, 255 );
    m_Alpha.Init( "Alpha", "Color_Parm", this, 255, 0, 255 );
}
Color::Color( double r, double g, double b, double a )
{
    m_Red.Init( "Red", "Color_Parm", this, r, 0, 255 );
    m_Green.Init( "Green", "Color_Parm", this, g, 0, 255 );
    m_Blue.Init( "Blue", "Color_Parm", this, b, 0, 255 );
    m_Alpha.Init( "Alpha", "Color_Parm", this, a, 0, 255 );
}
Color::~Color()
{
}

void Color::ParmChanged( Parm* parm_ptr, int type )
{
}
