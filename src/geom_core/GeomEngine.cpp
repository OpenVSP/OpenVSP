//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//
//
//////////////////////////////////////////////////////////////////////

#include "GeomEngine.h"
using namespace vsp;

//===============================================================================//
//===============================================================================//
//===============================================================================//

//==== Constructor ====//
GeomEngine::GeomEngine( Vehicle* vehicle_ptr ) : GeomXSec( vehicle_ptr )
{

}

//==== Destructor ====//
GeomEngine::~GeomEngine()
{

}

void GeomEngine::ValidateParms()
{

}

void GeomEngine::Update( bool fullupdate )
{
    ValidateParms();

    GeomXSec::Update( fullupdate );
}

double GeomEngine::roll_t( const double &torig, const double &troll, const double &tmin, const double &tmax )
{
    double t = torig - troll;
    if ( t < tmin )
    {
        t = t + ( tmax - tmin );
    }
    return t;
}

double GeomEngine::unroll_t( const double &t, const double &troll, const double &tmin, const double &tmax )
{
    double torig = t + troll;
    if ( torig > tmax )
    {
        torig = torig - ( tmax - tmin );
    }
    return torig;
}

void GeomEngine::UpdateEngine()
{

}

void GeomEngine::UpdateHighlightDrawObj()
{

}

void GeomEngine::LoadDrawObjs( vector< DrawObj* > & draw_obj_vec )
{

}
