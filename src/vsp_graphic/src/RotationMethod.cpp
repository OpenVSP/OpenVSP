#include "RotationMethod.h"
#include "Camera.h"

#include <algorithm>
#include <cmath>

namespace VSPGraphic
{
void RotationMethod::rebaseIfTurned( Camera * cam, int x, int y )
{
    if ( cam->rotationVersion() != _version )
    {
        begin( cam, x, y );
    }
}

ClassicBall::ClassicBall()
{
    _px = 0;
    _py = 0;
}

void ClassicBall::begin( Camera * cam, int x, int y )
{
    _px = x;
    _py = y;
}

void ClassicBall::drag( Camera * cam, int x, int y )
{
    cam->rotate( _px, _py, x, y );

    _px = x;
    _py = y;
}
}
