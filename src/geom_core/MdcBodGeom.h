//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//

// MdcBodGeom.h:
// A body laid out as DATCOM lays one out.
//
//////////////////////////////////////////////////////////////////////

#if !defined(VSP_MDC_BOD_GEOM__INCLUDED_)
#define VSP_MDC_BOD_GEOM__INCLUDED_

#include "XSecSurfGeom.h"

//==== MDC_BOD Geom ====//
// A body laid out as DATCOM lays one out, from NAMELIST AXIBOD's or ELLBOD's Option 1 inputs: a nose,
// a centerbody and an aft body, each given by its shape, its length and its size at its aft end.  NAMELIST
// chooses AXIBOD, whose sections are circles DNOSE, DCENTR and DAFT across, or ELLBOD, whose sections are
// ellipses WNOSE, WCENTR and WAFT wide and ENOSE, ECENTR and EAFT times as high.  The parms carry the
// namelists' names and meanings.  The nose's tip is at the component's origin, so DATCOM's XO is its X location.
//
// As DATCOM does, the body is lofted uncambered, straight between its stations' sections, the nose's base and
// the centerbody's and aft body's aft ends, and then moved up by the camber, ZTIP, ZNOSE, ZCENTR and ZAFT, each
// linear between the stations where it is given, the nose's tip first.
//
// The nose is the surface's first end cap with scaled rows: each row the base scaled along the nose's
// profile, found for a round nose as wide as the base, so its ellipticity is ENOSE throughout.  It is blunted
// by a sphere BNOSE in radius, which the ellipticity makes BNOSE * ENOSE high, or with TRUNC truncated at a
// flat face BNOSE in half width.  The centerbody is a frustum, a cylinder when it keeps the nose's size.  The
// aft body is a frustum or an ogive, a boattail when it narrows and a flare when it widens.  The base is flat,
// in to a point at its middle.
//
// An ogive is a circular arc in the side and plan views, at the slope SLPNOS or SLPTAL from the axis at its
// larger end, positive towards the axis, and zero for a tangent ogive.
//
// BNOSE blunts only a cone or ogive nose, but truncates any.  A nose alone is closed by a flat section in to a
// point.
//
// DEXIT, the nozzle's exit diameter, is drawn as DATCOM draws it: the base is flat from its edge in to a circle DEXIT
// across, and from there a cone goes in to a point DEXIT / 4 inside the base.  It is no larger than the base.
class MdcBodGeom : public XSecSurfGeom
{
public:
    enum { AXIBOD, ELLBOD };
    enum { CONE, OGIVE, POWER, HAACK, KARMAN };

    MdcBodGeom( Vehicle* vehicle_ptr );
    virtual ~MdcBodGeom();

    virtual void ApplyScale( double currentScale );

    // The rotation center is Origin along the body's length from the tip of its nose.
    virtual void ComputeCenter();

    // The stations along x and the shifts in z the camber lifts the surface by, as the last update made them.
    void GetCamber( vector< double > & x, vector< double > & z ) const;

    // The distance r from the body's axis at x, in its own frame, to its mold line in the direction phi, degrees
    // clockwise from the top looking forward.  False where x is not along the body.
    bool Radius( double x, double phi, double & r ) const;

    // The radius under the point x along a component, in the component's frame, in the direction phi, when the
    // component's parent is an MDC_BOD: DATCOM takes a protuberance's, an inlet's and a fin's from the body.
    static bool RadiusUnder( Geom* geom_ptr, double x, double phi, double & r );

    virtual void UpdateEndCaps();

    IntParm m_Namelist;


    IntParm m_TNose;
    Parm m_Power;
    Parm m_LNose;
    Parm m_DNose;
    Parm m_WNose;
    Parm m_ENose;
    Parm m_BNose;
    BoolParm m_Trunc;
    Parm m_ZTip;
    Parm m_ZNose;
    Parm m_SlpNos;

    Parm m_LCentr;
    BoolParm m_CentrDefault;
    Parm m_DCentr;
    Parm m_WCentr;
    Parm m_ECentr;
    Parm m_ZCentr;

    IntParm m_TAft;
    Parm m_LAft;
    Parm m_DAft;
    Parm m_WAft;
    Parm m_EAft;
    Parm m_ZAft;
    Parm m_SlpTal;
    Parm m_DExit;

protected:
    virtual void UpdateSurf();

    // A child that stands on the body is under a new part of it when the body moves.
    virtual void UpdateChildren( bool fullupdate );

    // Marks the children that stand on the body to rebuild their surfaces.
    void DirtyStandingChildren();

    vector< double > m_CamberX;
    vector< double > m_CamberZ;

    // The finished surface along x, its half width, half height and the height of its middle, sampled from it once
    // its caps are on.  x increases.
    void UpdateProfile();
    vector< double > m_ProfileX;
    vector< double > m_ProfileHalfW;
    vector< double > m_ProfileHalfH;
    vector< double > m_ProfileZMid;

    // The derivatives at its ends of the circular arc from ( x0, r0 ) to ( x1, r1 ) that leaves its larger end at
    // slope slp from the axis, positive towards the axis, and passes through its smaller end, in x and r at its
    // start and then its end.
    static void OgiveEnds( double x0, double r0, double x1, double r1, double slp, double d[4] );

    // Skin the section from xsec a at x0 to xsec b at x1 as circular arcs, through half heights h0 and h1 in the
    // side view and half widths w0 and w1 in the plan view.
    static void OgiveSection( BareXSec* xa, BareXSec* xb, double x0, double h0, double w0, double x1, double h1, double w1,
                              double slp, bool first );
};

#endif // !defined(VSP_MDC_BOD_GEOM__INCLUDED_)
