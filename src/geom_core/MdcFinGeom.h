//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//

// MdcFinGeom.h:
// A fin set laid out as DATCOM lays one out.
//
//////////////////////////////////////////////////////////////////////

#if !defined(VSP_MDC_FIN_GEOM__INCLUDED_)
#define VSP_MDC_FIN_GEOM__INCLUDED_

#include "XSecSurfGeom.h"

class Wedge;

//==== MDC_FIN Geom ====//
// A fin set laid out as DATCOM lays one out, from NAMELIST FINSETn's inputs: up to ten span stations of
// SSPAN, CHORD, XLE, SWEEP, STA, LER and CFOC, NPANEL panels rolled to PHIF with dihedral GAM, and one section
// type, SECTYP, for every station.  The parms carry the namelist's names and meanings; an array is a Parm for
// each station or panel, numbered from one, as SSPAN_1 to SSPAN_n for NSPAN of n.  One component is one fin set;
// FINSET1 to FINSET9 are separate components.
//
// The planform is straight between its stations.  SSPAN, measured from the fin root when SSPAN_1 is zero and from
// the axis when it is not, is along the panel in true view.  The root's leading edge is the component's origin, so
// DATCOM's XLE(1) is its X location, and XLE_k of a later station is from the root's leading edge.  Where
// XLEFlag is set for a segment its aft station's XLE is the input and the segment's SWEEP follows, where it is
// not SWEEP is the input and XLE follows, measured at the chord station STA of the segment's inboard station.
//
// SECTYP is HEX, NACA, ARC or USER.  A HEX section is a hexagon, a diamond where LFLAT is zero, with ZUPPER and
// ZLOWER the heights of its sides above and below the chord line, at LMAXU and LMAXL from the leading edge, flat
// for LFLATU and LFLATL.  An ARC section is a circular arc above and below the chord line, ZUPPER and ZLOWER
// high, with no LMAX or LFLAT.  Both vary from station to station, and ZLOWER, LMAXL and LFLATL follow the upper
// surface's unless set.  LERType says what LER, the leading edge radius, does to them: nothing, the edge sharp as
// DATCOM draws it, or rounds the edge, the airfoil keeping its crest or its angle.  NACA and USER sections are the
// same at every station, scaled by the chord, and take no LER.  The NACA
// card is NACASER, the series, with NACADES, its designation as a number with the thickness as its last two
// digits, NACAMOD, a modified series' last two digits, and for the 6 series NACASEA and NACAMEAN.  They are
// OpenVSP's airfoils: the 1 series is the 16 series, the 6 series 63 to 67 and the A sections 63A to 65A.
//
// A USER section is the file airfoil User, the same at every station and scaled by the chord.  The Read Airfoil
// button reads an airfoil file into it, and its points are saved with the component.  It is a 5% biconvex section
// until a file is read.  A DATCOM user section given by XCORD, MEAN and THICK or by XCORD, YUPPER and YLOWER is
// converted to an airfoil file first.
//
// A panel at PHIF is rolled clockwise from the top looking forward, its root on the body RBODY from the axis, or
// at SSPAN_1 where it is not zero, and then turned GAM further about its root.  A panel on the left, PHIF
// between 180 and 360 degrees, or at the top, has its section reflected from the right's rather than rotated,
// so that camber lifts the same way on both sides.
//
// CFOC makes trailing edge flaps, a control surface FLAP_k on each segment k whose two stations' CFOC are not
// zero, from the segment's inboard to outboard station, as deep as those stations' CFOC times the fin's chord
// there, linearly between, on every panel.  CFOC_1 of 1 or more is an all moving fin, with no flap.  The component
// manages these control surfaces, adding, setting and deleting them as CFOC changes.
class MdcFinGeom : public XSecSurfGeom
{
public:
    enum { HEX, NACA, ARC, USER };
    enum { LER_SHARP, LER_KEEP_CREST, LER_KEEP_ANGLE };
    enum { NACA4, NACA4M, NACA5, NACA5M, NACA1, NACA6, NACAS1, NACAS2, NACAS3 };

    static constexpr int MAXSTN = 10;
    static constexpr int MAXPANEL = 8;

    MdcFinGeom( Vehicle* vehicle_ptr );
    virtual ~MdcFinGeom();

    virtual void ApplyScale( double currentScale );

    // It stands on its parent MDC_BOD's mold line.
    virtual bool ShapeFollowsParent() const                { return true; }

    //==== Encode/Decode XML ====//
    virtual xmlNodePtr EncodeXml( xmlNodePtr & node );
    virtual xmlNodePtr DecodeXml( xmlNodePtr & node );

    // The cross section the USER section is, and its file airfoil.  It keeps the points read from a file with the
    // component, apart from the surfaces the component rebuilds.
    XSec* GetUserXSec();
    FileAirfoil* GetUserAirfoil();

    // True where a panel at roll phi, in degrees, has its section reflected from the right's: on the left, or at
    // the top.
    static bool Reflected( double phi );

    // The distance of a panel's root from the axis, as the last update placed it: SSPAN_1 where it is not zero,
    // else the radius of a parent MDC_BOD under the root chord's middle at the panel's roll, else RBODY.
    double RootRadius( int panel ) const;

    // Sizes the station and segment Parms to NSPAN and the panel Parms to NPANEL.
    void SizeParms();

    IntParm m_SecTyp;
    IntParm m_LERType;
    IntParm m_NPanel;
    IntParm m_NSpan;
    Parm m_RBody;

    IntParm m_Station;
    IntParm m_Panel;

    // One of each per station, station k at index k - 1.  SWEEP, STA and XLEFlag are those of the segment
    // station k starts, so there is one fewer of them.  XLE is from station 2 on, at index k - 2: station 1's
    // leading edge is the origin.
    vector< Parm* > m_SSpan;
    vector< Parm* > m_Chord;
    vector< Parm* > m_XLE;
    vector< Parm* > m_LER;
    vector< Parm* > m_CFOC;
    vector< Parm* > m_Sweep;
    vector< Parm* > m_Sta;
    vector< BoolParm* > m_XLEFlag;

    vector< Parm* > m_ZUpper;
    vector< Parm* > m_ZLower;
    vector< Parm* > m_ZLowerRatio;
    vector< Parm* > m_LMaxU;
    vector< Parm* > m_LMaxL;
    vector< Parm* > m_LMaxLRatio;
    vector< Parm* > m_LFlatU;
    vector< Parm* > m_LFlatL;
    vector< Parm* > m_LFlatLRatio;
    BoolParm m_ZLowerRelFlag;
    BoolParm m_LMaxLRelFlag;
    BoolParm m_LFlatLRelFlag;

    // One of each per panel.
    BoolParm m_PhiFFlag;
    vector< Parm* > m_PhiF;
    vector< Parm* > m_Gam;

    IntParm m_NacaSer;
    Parm m_NacaDes;
    IntParm m_NacaMod;
    BoolParm m_NacaSea;
    Parm m_NacaMean;
    Parm m_NacaXMax;
    Parm m_NacaFlat;

protected:
    virtual void UpdateSurf();

    XSecSurf* m_UserXSecSurf;

    // Sets a wedge airfoil of chord c ZUPPER zu high at LMAXU mu with the flat fu, and the same for the lower side.
    static void SetWedge( Wedge* wedge, double c, double zu, double zl, double mu, double ml, double fu, double fl );

    // A HEX section at station k, the wedge airfoil of the half thickness ZUPPER + ZLOWER over two and the
    // camber ZUPPER - ZLOWER over two.
    BareXSec* WedgeSection( XSecSurf* surf, int k, double c );

    // An ARC section at station k, the biconvex airfoil ZUPPER + ZLOWER thick, ZUPPER of it above the chord line.
    BareXSec* BiconvexSection( XSecSurf* surf, int k, double c );

    // A NACA section of the card's series and designation, of chord c: a built-in airfoil of the designation's
    // parts, or for a supersonic series a wedge airfoil, diamond or hexagon, or a biconvex airfoil.
    BareXSec* NacaSection( XSecSurf* surf, double c );

    // LER rounds station k's leading edge, of chord c, as LERType says.
    void RoundLeadingEdge( BareXSec* xs, int k, double c );

    // Station k's section of chord c on surf.
    BareXSec* StationSection( XSecSurf* surf, int k, double c, int sectyp );

    // The panel angles: PHIF where it is given, else the panels evenly spaced from zero, written to the parms.
    void PanelAngles( int npanel, vector< double > & phif );

    // The flaps CFOC gives: a control surface, first named FLAP_k, on the trailing edge of each segment k whose two
    // stations' CFOC are not zero, unless CFOC_1 is 1 or more and the whole fin moves.  Its ends are the segment's
    // stations, which are the surface's U of a section's place among the surface's patches, with a patch for each
    // end cap, and its chord fractions are the stations' CFOC.  The control surfaces are on every panel.  Those
    // not wanted are deleted.
    void UpdateFlaps( int nspan, const vector< double > & chord );

    // Each panel's root's distance from the axis.
    vector< double > m_RootRadiusVec;

    // The ID of segment k's flap at index k - 1, or empty where it has none.
    vector< string > m_FlapIDVec;
};

#endif // !defined(VSP_MDC_FIN_GEOM__INCLUDED_)
