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
}