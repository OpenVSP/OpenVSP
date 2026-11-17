//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//

// MdcInletGeom.h:
// An inlet set laid out as DATCOM lays one out.
//
//////////////////////////////////////////////////////////////////////

#if !defined(VSP_MDC_INLET_GEOM__INCLUDED_)
#define VSP_MDC_INLET_GEOM__INCLUDED_

#include "XSecSurfGeom.h"

//==== MDC_INLET Geom ====//
// An inlet laid out as DATCOM lays one out, from NAMELIST INLET's inputs for one set of inlets: NIN
// identical inlets of type INTYPE at the axial station XINLT, rolled around the body to the angles PHI.  The
// parms carry the namelist's names and meanings, one parm per array entry: PHI_k of inlet k and Xi, Hi and Wi at
// station i.  Each inlet set is a component of its own.
//
// The component's origin is on the body's axis at the inlet's leading edge, with x aft along the axis and z up,
// so DATCOM's XINLT is its X location.  The mold line is RBODY from the axis, the body's radius under the inlet,
// which DATCOM takes from the body and the namelist does not give.  PHI is measured clockwise from the top looking forward, so 90 is to the right.  Unless
// PHIFlag is set, the inlets are evenly spaced from PHI = 0 and PHI is written from NIN.
//
// The inlet is a solid laid out at the namelist's five stations: 1 the leading edge or tip, 2 the cowl lip, 3
// the midbody start, 4 the boattail start and 5 the boattail end, each Xi along from the leading edge and
// straight between them.  X1 and H1 are zero by the namelist's definition and are not inputs.  A 2DTOP inlet is
// a box with H out from the body and W across it.  A 2DSIDE inlet is the same box with H across the body, from
// the top of its extent down, centered on the axis at its tallest, and W out from the body.  An AXI inlet is a
// body of revolution of diameter Wi, with its axis W3 / 2 out from the line the cowl's top touches, and its tip
// W1 across, a point where W1 is zero.  The inner face is HDIV from the mold line: for a 2D inlet a plane, for
// an AXI inlet the line the cowl's top touches.  Where HDIV is zero the inlet is sunk a hundredth of RBODY into
// the body, so it crosses the body rather than touching it along a line.
//
// An open inlet has a ramp in front of the cowl lip: a wedge, or for AXI a cone, from the leading edge to
// station 2 at the angle RAMP, flat across the lip's face between it and the cowl.  A covered inlet is plugged
// flush with the inlet face, the plane or cone from the leading edge to the lip.  No duct is modeled.  A ramp
// that would reach the cowl is the plug.  A 2D inlet's ramp ends a hundredth of its length short of the lip, and
// the face between it and the cowl leans back across that hundredth.
//
// Where HDIV is greater than zero a diverter fills the gap between the body and the inlet: a plate XDIV from
// the leading edge, LDIV long, tapering in plan from nothing to 95 percent of the inlet's extent across the
// body at its aft end, standing from below the mold line to halfway up the inlet.  It is a little narrower than
// the inlet and ends short of its aft face, so that no faces meet exactly.  It ends no further aft than the
// inlet does.
class MdcInletGeom : public XSecSurfGeom
{
public:
    enum { TOP2D = 1, SIDE2D, AXI };

    static constexpr int MAX_IN = 20;
    static constexpr int NUM_STN = 5;

    MdcInletGeom( Vehicle* vehicle_ptr );
    virtual ~MdcInletGeom();

    virtual void ApplyScale( double currentScale );

    // Sets the shape of the preset Preset names: the inlet as new, or the generic rocket's, sized to the largest
    // radius of the body it stands on as the rocket's is to its body of radius 5, wherever along the body it is.
    void SetPreset();

    // It stands on its parent MDC_BOD's mold line.
    virtual bool ShapeFollowsParent() const                { return true; }

    //==== Encode/Decode XML ====//
    virtual xmlNodePtr DecodeXml( xmlNodePtr & node );

    // Sizes PHI to NIN.
    void SizeParms();

    IntParm m_Preset;
    IntParm m_InType;
    Parm m_RBody;
    Parm m_XDiv;
    Parm m_HDiv;
    Parm m_LDiv;

    IntParm m_NIn;
    IntParm m_Inlet;
    BoolParm m_PhiFlag;
    vector< Parm* > m_Phi;

    // Station k's inputs, k from 1: W at index k - 1, and from station 2 on, X and H at index k - 2.  Station 1, the
    // leading edge or tip, is at X and H zero.
    Parm m_X[NUM_STN - 1];
    Parm m_H[NUM_STN - 1];
    Parm m_W[NUM_STN];

    BoolParm m_Cover;
    Parm m_Ramp;

protected:
    virtual void UpdateSurf();

    // The shape as the inlet was made, the default preset.
    vector< PresetValue > m_DefaultValues;

    // The fraction of the ramp's length the face between its end and the cowl lip leans back across.
    static const double RAMP_END;

    // A diverter's width as a fraction of the extent of the inlet it stands against.
    static const double DIV_FIT;

    // A section at x, the box from ya to yb across and za to zb up, or the circle in it.
    struct Sec
    {
        double x;
        double ya;
        double yb;
        double za;
        double zb;
    };

    // The inlet's sections from its leading edge to its aft end, the shapes it is lofted straight between.
    vector< Sec > m_Secs;

    static Sec NewSec( double x, double ya, double yb, double za, double zb );

    // A 2D inlet's section at x, h high from the inner line and w wide: across the body for 2DTOP, and for 2DSIDE
    // out from it, hmax being the tallest.
    static Sec Box( int intype, double x, double h, double w, double hmax, double zf );

    // An AXI inlet's section at x, d across, its axis ha above the body's.
    static Sec Round( double x, double d, double ha );

    // Fills the sections from the namelist's stations.  The inner line is hd out from the mold line r.
    void BuildSecs( int intype, double r, double hd );

    // Adds an inlet's surfaces on a body of radius r, at its top, and answers how many.
    int AddSurfs( int intype, double r );

    // The extents at x that every section there shares: where two sections stand at x the narrower across and the
    // lower middle up.  Its za and zb are both that middle height.
    Sec Shared( double x, double tol );

    void AddBoxSection( XSecSurf* surf, double x, double ya, double yb, double za, double zb );

    // The inlet as one surface through the sections, closed at its aft end by a point at the middle of its last
    // section, and an AXI inlet at its tip by a point.  A 2D inlet's leading edge is a line, its first section
    // flat.
    void InletSurf( int intype, double tol );

    // The diverter as one surface, a plate from the apex at its leading edge to its aft end, its width at each
    // section where the inlet changes or the diverter ends as the narrower of the linear taper and DIV_FIT of the
    // inlet's extent.  Answers whether there is one.
    bool DiverterSurf( double r, double hd, double tol );
};

#endif // !defined(VSP_MDC_INLET_GEOM__INCLUDED_)
