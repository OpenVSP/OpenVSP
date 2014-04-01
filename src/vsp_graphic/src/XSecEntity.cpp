#include "XSecEntity.h"
#include "Lighting.h"
#include <math.h>

namespace VSPGraphic
{
XSecEntity::XSecEntity() : Entity()
{
    _numOfXSec = _numOfPnts = 0;
}
XSecEntity::XSecEntity(Lighting * lights ) : Entity( lights )
{
    _numOfXSec = _numOfPnts = 0;
}
XSecEntity::~XSecEntity()
{
}

void XSecEntity::setNumXSec( unsigned int value )
{
    _numOfXSec = value;
}

unsigned int XSecEntity::getNumXSec()
{
    return _numOfXSec;
}

void XSecEntity::setNumPnts( unsigned int value )
{
    _numOfPnts = value;
}

unsigned int XSecEntity::getNumPnts()
{
    return _numOfPnts;
}

XSecEntity::Ratios XSecEntity::getRatiosAtIndex( unsigned int index )
{
    Ratios result;

    unsigned int pntsAtIndex = std::floor( ( index + 1 ) / ( double )getNumXSec() );
    unsigned int xsecAtIndex = index - ( pntsAtIndex * getNumXSec() - 1 );

    result.Pnts = ( xsecAtIndex > 0 ? pntsAtIndex + 1 : pntsAtIndex ) / ( double )getNumPnts();
    result.XSec = ( xsecAtIndex > 0 ? xsecAtIndex - 1 : 0 ) / ( ( double )getNumXSec() - 1 );

    return result;
}

unsigned int XSecEntity::getIndexWithRatio( Ratios ratios )
{
    unsigned int pntsAtIndex = ratios.Pnts * getNumPnts() - ( ratios.XSec == 0.0 ? 0 : 1 );
    unsigned int xsecAtIndex = ratios.XSec != 0.0 ? ratios.XSec * ( getNumXSec() - 1 ) + 1 : 0;

    int result = getNumXSec() * pntsAtIndex + xsecAtIndex - 1;

    return result >= 0 ? result : 0;
}

unsigned int XSecEntity::getIndexWithRatio( double pntsRatio, double xsecRatio )
{
    Ratios temp;
    temp.Pnts = pntsRatio;
    temp.XSec = xsecRatio;

    return getIndexWithRatio( temp );
}
}