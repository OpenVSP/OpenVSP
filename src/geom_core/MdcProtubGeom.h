//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//

// MdcProtubGeom.h:
// A protuberance set laid out as DATCOM lays one out.
//
//////////////////////////////////////////////////////////////////////

#if !defined(VSP_MDC_PROTUB_GEOM__INCLUDED_)
#define VSP_MDC_PROTUB_GEOM__INCLUDED_

#include "XSecSurfGeom.h"

//==== MDC_PROTUB Geom ====//
// A protuberance laid out as DATCOM lays one out, from NAMELIST PROTUB's inputs for one protuberance
// set: NLOC identical protuberances of type PTYPE at the axial station XPROT, rolled around the body to the
// angles PHIPRO.  The parms carry the namelist's names and meanings, one parm per array entry: PHIPRO_k of
// protuberance k, and LPROT_m, WPROT_m, HPROT_m and OPROT_m of member m.  Each protuberance set is a component
// of its own.
//
// The component's origin is on the body's axis at the protuberance set's centroid, with x aft along the axis and
// z up, so DATCOM's XPROT is its X location.  The mold line is RBODY from the axis, the body's radius at XPROT,
// which DATCOM takes from the body and the namelist does not give.  PHIPRO is measured clockwise from the top looking forward, so 90 is to the right.  Unless
// PHIPROFlag is set, the protuberances are evenly spaced from PHIPRO = 0 and PHIPRO is written from NLOC.
//
// Each member is centered along x at the origin and stands out from the mold line, LPROT long along the body, WPROT
// wide across it and HPROT high, its bottom OPROT above the mold line.  A VCYL is an elliptic cylinder standing
// on the mold line; an HCYL an elliptic cylinder lying along x, flat at its ends; a BLOCK a box; a FAIRING
// half an ellipsoid.  A SHOE is a base plate, a web and a top plate centered across the body, and a LUG a base
// plate, two posts under the ends of the top plate, set in by half their widths unless they are its length, and
// the top plate across them, each member a box.  Each of the other protuberances is a clone of the first, rolled
// about x.
//
// No two surfaces touch exactly.  Members of one length that meet are one surface, its section their outline;
// where members of different lengths meet exactly, one is buried in the other half the other's height.
//
// A flat bottomed member whose OPROT is zero stands on the body's top line, touching the body along it, as DATCOM
// draws it.  With BuryRoot set it reaches down into the body, past where the body passes under its sides at its
// ends, and a tenth of its height further, so it meets the body all around on a nose, a boattail or an elliptical
// body as well.  A FAIRING is the top half of an ellipsoid, as the DATCOM manual defines one: LPROT long and WPROT
// wide at its flat base, which is on the body's top line, the body's radius above the axis, and HPROT high above it.
// Its OPROT is zero, as DATCOM takes it.  With BuryRoot set its base is carried straight down into the body as far.
class MdcProtubGeom : public XSecSurfGeom
{
public:
    enum { VCYL = 1, HCYL, LUG, SHOE, BLOCK, FAIRING };

    static constexpr int MAX_LOC = 20;
    static constexpr int MAX_MEM = 4;

    MdcProtubGeom( Vehicle* vehicle_ptr );
    virtual ~MdcProtubGeom();

    virtual void ApplyScale( double currentScale );

    // It stands on its parent MDC_BOD's mold line.
    virtual bool ShapeFollowsParent() const                { return true; }

    //==== Encode/Decode XML ====//
    virtual xmlNodePtr DecodeXml( xmlNodePtr & node );

    // The members a type of protuberance has.
    static int NumMembers( int ptype );

    // Sizes PHIPRO to NLOC and LPROT, WPROT, HPROT and OPROT to the members PTYPE has.
    void SizeParms();

    // Sets the shape of the preset Preset names: the protuberance as new, or a type and its members, sized to
    // RBODY as they are to the body they are for, its locations kept.  A type's are the protuberance of that type
    // of the namelist's sample body, 6 in radius, the HCYL and LUG, which it lacks, in proportion with them, and
    // the lugs are those of a rocket 5 in radius.  The members of a LUG or SHOE are stacked, each OPROT the height
    // of the members under it.
    void SetPreset();

    IntParm m_Preset;
    IntParm m_PType;
    Parm m_RBody;
    BoolParm m_BuryRoot;
    IntParm m_NLoc;
    IntParm m_LocIndex;
    BoolParm m_PhiProFlag;
    vector< Parm* > m_PhiPro;

    vector< Parm* > m_LProt;
    vector< Parm* > m_WProt;
    vector< Parm* > m_HProt;
    vector< Parm* > m_OProt;

protected:
    // The Design Parms as new, for the default preset.
    vector< PresetValue > m_DefaultValues;

    virtual void UpdateSurf();

    // The section shapes.
    enum { POINT, BOX, ELLIPSE };

    // A member's box at the top of the body: hl either side of the origin along x, from y0 to y1 across and from z0 to
    // z1 up.
    struct Member
    {
        double hl;
        double y0;
        double y1;
        double z0;
        double z1;
    };

    // The height of a member's bottom above the axis at the roll phi: OPROT above the mold line r, or, on the
    // mold line, the top line r, or with BuryRoot set below where the body passes under its sides, half across
    // out, at its ends, hl either side along it, and where a round body r in radius would, by a tenth of its
    // height h.
    double Bottom( double r, double phi, double hl, double half, double h, double o );

    static double Overlap( double a0, double a1, double b0, double b1 );

    // Where one of two members stands exactly on the other, buries one's end in the other half the other's
    // height, the one that puts less outside the other: one within the other across and along puts nothing
    // outside it.
    static void BuryIfTouching( Member & a, Member & b, double tol );
    static void Bury( Member & a, Member & b );

    // Member m's box, from y0 to y1 across, at the roll phi.
    Member MemberBox( int m, double r, double phi, double y0, double y1 );

    BareXSec* AddSection( XSecSurf* surf, int shape, double x, double y, double z, double w, double h );
    void Prism( XSecSurf* surf, int shape, double x0, double x1, double y, double z, double w, double h );
    // The top half of an ellipsoid l long and w wide at its flat base, built along x from the base at the origin:
    // the base is its one section, which the end caps close, carried down to -depth where depth is not zero.
    void HalfEllipsoid( XSecSurf* surf, double l, double w, double depth );
    void BoxSurf( double x, const Member & mem );
    void Extrude( double x0, double x1, const vector< double > & ys, const vector< double > & zs,
                  const vector< double > & sys, const vector< double > & szs );
    int Staircase( double x, const vector< Member > & group, double tol );
    void StairSurf( double x, double hl, const vector< double > & lo, const vector< double > & hi,
                    const vector< double > & hw );
    int Shoe( double x, double r, double phi );
    static bool Reaches( const Member & post, const Member & top, double tol );
    int Lug( double x, double r, double phi );
    // Adds the protuberance's surfaces on a body of radius r, at its top, for the roll phi, and answers how many.
    int AddSurfs( int ptype, double r, double phi );

    // What stands the surfaces AddSurfs built on a body of radius r up at the top of the body, for the roll phi.
    Matrix4d Stand( int ptype, double r, double phi );
};

#endif // !defined(VSP_MDC_PROTUB_GEOM__INCLUDED_)
