//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//

// WingGeom.h:
// J.R Gloudemans
//
//////////////////////////////////////////////////////////////////////

#if !defined(VSPWINGGEOM__INCLUDED_)
#define VSPWINGGEOM__INCLUDED_

#include "Geom.h"
#include "FuselageGeom.h"
#include "XSec.h"
#include "XSecSurf.h"

//==== Wing Driver Group ====//
class WingDriverGroup : public DriverGroup
{
public:
    enum { AR_WSECT_DRIVER, SPAN_WSECT_DRIVER, AREA_WSECT_DRIVER, TAPER_WSECT_DRIVER,
           AVEC_WSECT_DRIVER, ROOTC_WSECT_DRIVER, TIPC_WSECT_DRIVER, NUM_WSECT_DRIVER
         };

    WingDriverGroup();

    virtual void UpdateGroup( vector< string > parmIDs );
    virtual bool ValidDrivers( vector< int > choices );
};

//==== Wing Geom ====//
class WingGeom : public FuselageGeom
{
public:
	WingGeom( Vehicle* vehicle_ptr );

};
#endif // !defined(VSPWINGGEOM__INCLUDED_)
