//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//

// XSec.h: Cross Section Base Class and Derived Classes
// J.R Gloudemans
//
//////////////////////////////////////////////////////////////////////

#if !defined(XSEC__INCLUDED_)
#define XSEC__INCLUDED_


#include "Parm.h"
#include "ParmContainer.h"
#include "Vec3d.h"
#include "VspSurf.h"
#include "XSecCurve.h"

#include <string>
#include <vector>
#include <memory>
#include <utility>

#define XSEC_NO_VAL (-1.0e11)

using std::string;

class Geom;

class XSec : public ParmContainer
{
public:
    XSec( XSecCurve *xsc );                                                      // Default Constructor
    virtual ~XSec();

    virtual void ParmChanged( Parm* parm_ptr, int type );
    virtual void Update() = 0;

    virtual void SetGroupDisplaySuffix( int num );

    void SetGroupAlias( const string & alias );
    string GetGroupAlias() const { return m_GroupAlias; };
    string GetDisplayGroupName( const string & group_name_overwrite = string() );

    virtual VspCurve& GetUntransformedCurve();
    virtual VspCurve& GetCurve();

    virtual void GetDrawLines( Matrix4d &transMat, vector< vec3d > &lines );

    virtual XSecCurve* GetXSecCurve()
    {
        return m_XSCurve;
    }

    virtual void SetXSecCurve( XSecCurve* xs_crv );

    virtual int  GetType()
    {
        return m_Type;
    }

    virtual Matrix4d* GetTransform();

    virtual void AddLinkableParms( vector< string > & linkable_parm_vec, const string & link_container_id = string() );

    //==== Copy Between Different Types ====//
    virtual void CopyFrom( XSec* xs );
    virtual void CopyBasePos( XSec* xs ) = 0;
    virtual xmlNodePtr EncodeXml( xmlNodePtr & node );
    virtual xmlNodePtr DecodeXml( xmlNodePtr & node );

    virtual xmlNodePtr EncodeXSec( xmlNodePtr & node );
    virtual xmlNodePtr DecodeXSec( xmlNodePtr & node );

    virtual void SetScale( double scale );

    virtual double GetProjectionCosine();

    virtual void GetSimpleBasis( vec3d &xdir, vec3d &ydir, vec3d &zdir );

    virtual void GetBasis( double t, Matrix4d &basis );
    virtual void GetTanNormVec( double t, double theta, double phi, vec3d &tangent, vec3d &normal );
    virtual void GetTanNormCrv( const vector< double > &ts, const vector< double > &thetas,
            const vector< double > &phis,
            const vector< double > &angstr, const vector< double > &crvstr,
            piecewise_curve_type &tangentcrv, piecewise_curve_type &normcrv );
    virtual void GetTanNormCrv( const vector< double > &thetas,
            const vector< double > &phis,
            const vector< double > &angstr, const vector< double > &crvstr,
            piecewise_curve_type &tangentcrv, piecewise_curve_type &normcrv );

    virtual void GetAngStrCrv( double t, int irib,
            double &thetaL, double &phiL, double &strengthL, double &curvatureL,
            double &thetaR, double &phiR, double &strengthR, double &curvatureR,
            const VspSurf &surf );

    virtual void ReadV2FileFuse2( xmlNodePtr &root );
    virtual void ReadV2FileFuse1( xmlNodePtr &root );

    virtual EditCurveXSec* ConvertToEdit();

    IntParm m_SectTessU;

protected:

    int m_Type;

    string m_GroupName;
    string m_GroupAlias;
    int m_GroupSuffix;

    VspCurve m_TransformedCurve;

    Matrix4d m_Transform;

    XSecCurve *m_XSCurve;

    virtual void ChangeID( const string &id );
};

// A user defined skinning station at an arbitrary position around the cross section.
//
// Skinning has always been controlled at four fixed places -- Top, Bottom, Left and Right.
// A spine adds another, anywhere in between, carrying its own conditions and values.  It
// runs the length of the body: its position and symmetry are synced across every XSec by
// the Geom, while the values stay per XSec, which is the point of having it.
//
// Symmetry works the opposite way round from the AllSym / TBSym / RLSym controls.  Those
// start on and tie the four sides together; a spine starts alone, and the flags below
// mirror it to the other side, the other half, or both.  Mirrored copies are generated
// rather than stored, so there is nothing to keep in sync and the browser shows only what
// the user made.
class SkinSpine : public ParmContainer
{
public:
    SkinSpine();

    virtual void ParmChanged( Parm* parm_ptr, int type );

    // A spine's parms land in its Geom's flattened group map alongside every other parm
    // beneath that Geom, so they need to be told apart.  The suffix carries the XSec index,
    // the alias the spine's name -- giving SkinSpine_2_Spine_0 and matching how an XSec's
    // own parms already read.
    virtual void SetGroupDisplaySuffix( int num );
    virtual void SetGroupAlias( const string & alias );

    // Where this spine's mirror images sit.  Right is W = 0, Bottom 1, Left 2, Top 3, so a
    // left/right mirror is W -> 2 - W and a top/bottom mirror is W -> -W, both modulo the
    // cross section period.
    virtual void GetMirrorW( vector< double > &ws, vector< bool > &flipslew, double period );

    // Which spine this is one cross section's copy of.
    //
    // A spine runs the length of the body: every cross section holds a copy carrying the
    // values it enforces there, while the position and the name belong to the first.  Keeping
    // those copies in step by matching them on position in the array holds only for as long as
    // nothing reorders it -- and a paste rebuilds the array from whatever the source held, in
    // the source's order.  Two spines at W01 0.4 and 0.6 could then trade places on one cross
    // section and nothing would notice: one's values would be read as the other's, and moving
    // the first to 0.35 would move the wrong one.
    //
    // The tag is shared by every copy of one spine and generated once, when the spine is
    // created.  It is not a Parm and not a container ID -- each copy is its own container --
    // it says only which spine this copy is of.
    virtual const string & GetSpineID() const                   { return m_SpineID; }
    virtual void SetSpineID( const string & id )                { m_SpineID = id; }

    Parm m_W01;

    string m_SpineID;

    BoolParm m_LRSymFlag;
    BoolParm m_TBSymFlag;

    BoolParm m_LAngleSet;
    BoolParm m_LSlewSet;
    BoolParm m_LStrengthSet;
    BoolParm m_LCurveSet;
    BoolParm m_RAngleSet;
    BoolParm m_RSlewSet;
    BoolParm m_RStrengthSet;
    BoolParm m_RCurveSet;

    BoolParm m_LRAngleEq;
    BoolParm m_LRSlewEq;
    BoolParm m_LRStrengthEq;
    BoolParm m_LRCurveEq;

    Parm m_LAngle;
    Parm m_LSlew;
    Parm m_LStrength;
    Parm m_LCurve;
    Parm m_RAngle;
    Parm m_RSlew;
    Parm m_RStrength;
    Parm m_RCurve;
};

class SkinXSec : public XSec
{
public:

    SkinXSec( XSecCurve *xsc );
    virtual ~SkinXSec();

    virtual xmlNodePtr EncodeXml( xmlNodePtr & node );
    virtual xmlNodePtr DecodeXml( xmlNodePtr & node );

    // Carry the spines' Parm identity across as well as this container's own.
    virtual void SwapIDs( ParmContainer* from );

    virtual void AddLinkableParms( vector< string > & parm_vec, const string & link_container_id = string() );

    virtual void CopySetValidate( IntParm &m_TopCont,
            BoolParm &m_TopLAngleSet,
            BoolParm &m_TopLSlewSet,
            BoolParm &m_TopLStrengthSet,
            BoolParm &m_TopLCurveSet,
            BoolParm &m_TopRAngleSet,
            BoolParm &m_TopRSlewSet,
            BoolParm &m_TopRStrengthSet,
            BoolParm &m_TopRCurveSet,
            BoolParm &m_TopLRAngleEq,
            BoolParm &m_TopLRSlewEq,
            BoolParm &m_TopLRStrengthEq,
            BoolParm &m_TopLRCurveEq );

    virtual void ChangeID( const string &newid );

    virtual void ValidateParms( IntParm &m_TopCont,
            BoolParm &m_TopLAngleSet,
            BoolParm &m_TopLSlewSet,
            BoolParm &m_TopLStrengthSet,
            BoolParm &m_TopLCurveSet,
            BoolParm &m_TopRAngleSet,
            BoolParm &m_TopRSlewSet,
            BoolParm &m_TopRStrengthSet,
            BoolParm &m_TopRCurveSet,
            BoolParm &m_TopLRAngleEq,
            BoolParm &m_TopLRSlewEq,
            BoolParm &m_TopLRStrengthEq,
            BoolParm &m_TopLRCurveEq );

    virtual void ValidateParms( );

    // Sides in station order, matching XSec::GetTanNormCrv's control value layout and the
    // order the skinning blend weights are built in.
    enum { SKIN_SIDE_RIGHT = 0, SKIN_SIDE_BOTTOM, SKIN_SIDE_LEFT, SKIN_SIDE_TOP, NUM_SKIN_SIDES };

    // One place where skinning is controlled around the cross section.  The four sides are
    // stations at W = 0, 1, 2, and 3; user defined spines add more at arbitrary W.  The
    // skinning path reads stations rather than sides, so it does not care how many there
    // are or where they sit.
    struct SkinStation
    {
        double m_W;

        // Whether this is one of the four fixed sides or came from a spine.
        // The sides anchor the control spline and always stand; a spine is the user's, and
        // one that enforces nothing anywhere is dropped instead.
        bool m_IsSide;

        // Which spine this came from, or -1 for a side.  A spine's mirror images carry the
        // same index, since they are that spine seen elsewhere on the section.
        int m_SpineIndex;

        bool m_LAngleSet;
        bool m_LSlewSet;
        bool m_LStrengthSet;
        bool m_LCurveSet;
        bool m_RAngleSet;
        bool m_RSlewSet;
        bool m_RStrengthSet;
        bool m_RCurveSet;

        double m_LAngle;
        double m_LSlew;
        double m_LStrength;
        double m_LCurve;
        double m_RAngle;
        double m_RSlew;
        double m_RStrength;
        double m_RCurve;
    };

    // One station built from a side's parameters.
    virtual void GetSideStation( int side, SkinStation &st );

    // Every station around the cross section, ordered by W.
    virtual void GetStations( vector< SkinStation > &stations );

    // W of every station, ordered.  What the blend weights are built from.
    virtual void GetStationW( vector< double > &ws );

    // One rib per station, each enforcing only that station's conditions.
    virtual void PrepRibs( bool first, bool last );

    virtual void GetRibs( bool first, bool last, vector< rib_data_type > &ribs );

    // Build the ribs from a supplied station list rather than the parms.
    virtual void GetRibs( bool first, bool last, vector< rib_data_type > &ribs,
                          const vector< SkinStation > &stations );

    // The one rib a blending pass uses: conditions confined to the spans its own stations
    // touch, values drawn only from those stations.
    virtual void GetGroupRib( bool first, bool last, const vector< SkinStation > &stations,
                              const vector< bool > &ingroup, rib_data_type &rib );

    // Replace every value this station does not enforce with what surf actually did there.
    // These are the values with no authority of their own, so a pass that needs them takes
    // them from a solution that left them free.
    virtual void FillUnsetFromSurf( int irib, const VspSurf &surf, vector< SkinStation > &stations );

    // The Top side's rib.  Retained for callers that do not blend.
    virtual rib_data_type GetRib( bool first, bool last );

    virtual bool AnyAngleSet( bool left, const vector< SkinStation > &stations );
    virtual bool AnyCurveSet( bool left, const vector< SkinStation > &stations );

    // Whether every station enforces the same conditions, in which case one skin suffices.

    virtual void SetGroupDisplaySuffix( int num );

    //==== User defined spines ====//
    // A position for a new spine that will not land on a station already there.
    virtual double SuggestSpineW01();

    virtual SkinSpine* AddSpine( double w01 );

    // Name the spines Spine_0, Spine_1 ... and push the XSec index and each spine's name
    // down onto its parms, so the display group names stay right as spines come and go.
    virtual void RenumberSpines();

    // The lowest numbered Spine_ name not already in use here.
    virtual string UnusedSpineName() const;

    // Put this cross section's spines in the same order as another's, matching by tag.
    virtual void OrderSpinesLike( const SkinXSec* other );
    virtual void DelSpine( int index );
    virtual void DelAllSpines();
    virtual int NumSpines() const                       { return m_SpineVec.size(); }
    virtual SkinSpine* GetSpine( int index );

    // The smallest gap allowed between stations.  Two stations at the same parameter would
    // give the control spline a zero length segment.
    static double GetMinStationGap()                    { return 1.0e-3; }

    // Build the tangent (fp) and normal (fpp) curves that control the loft on one side
    // of this XSec.  'left' selects the parameters that control the loft before this
    // XSec, otherwise the parameters controlling the loft after it are used.
    virtual void GetSkinCrvs( bool left, piecewise_curve_type &tangentcrv, piecewise_curve_type &normcrv );
    virtual void GetSkinCrvs( bool left, const vector< SkinStation > &stations,
                              piecewise_curve_type &tangentcrv, piecewise_curve_type &normcrv );

    virtual void SetUnsetParms( int irib, const VspSurf &surf );

    virtual void SetUnsetParms( double t, bool flipslew, int irib, const VspSurf &surf,
            BoolParm &LAngleSet,
            BoolParm &LSlewSet,
            BoolParm &LStrengthSet,
            BoolParm &LCurveSet,
            BoolParm &RAngleSet,
            BoolParm &RSlewSet,
            BoolParm &RStrengthSet,
            BoolParm &RCurveSet,
            Parm &LAngle,
            Parm &LSlew,
            Parm &LStrength,
            Parm &LCurve,
            Parm &RAngle,
            Parm &RSlew,
            Parm &RStrength,
            Parm &RCurve );

    virtual double GetScale() = 0;

    BoolParm m_AllSymFlag;
    BoolParm m_TBSymFlag;
    BoolParm m_RLSymFlag;

    IntParm m_TopCont;
    BoolParm m_TopLAngleSet;
    BoolParm m_TopLSlewSet;
    BoolParm m_TopLStrengthSet;
    BoolParm m_TopLCurveSet;
    BoolParm m_TopRAngleSet;
    BoolParm m_TopRSlewSet;
    BoolParm m_TopRStrengthSet;
    BoolParm m_TopRCurveSet;
    BoolParm m_TopLRAngleEq;
    BoolParm m_TopLRSlewEq;
    BoolParm m_TopLRStrengthEq;
    BoolParm m_TopLRCurveEq;
    Parm m_TopLAngle;
    Parm m_TopLSlew;
    Parm m_TopLStrength;
    Parm m_TopLCurve;
    Parm m_TopRAngle;
    Parm m_TopRSlew;
    Parm m_TopRStrength;
    Parm m_TopRCurve;

    IntParm m_RightCont;
    BoolParm m_RightLAngleSet;
    BoolParm m_RightLSlewSet;
    BoolParm m_RightLStrengthSet;
    BoolParm m_RightLCurveSet;
    BoolParm m_RightRAngleSet;
    BoolParm m_RightRSlewSet;
    BoolParm m_RightRStrengthSet;
    BoolParm m_RightRCurveSet;
    BoolParm m_RightLRAngleEq;
    BoolParm m_RightLRSlewEq;
    BoolParm m_RightLRStrengthEq;
    BoolParm m_RightLRCurveEq;
    Parm m_RightLAngle;
    Parm m_RightLSlew;
    Parm m_RightLStrength;
    Parm m_RightLCurve;
    Parm m_RightRAngle;
    Parm m_RightRSlew;
    Parm m_RightRStrength;
    Parm m_RightRCurve;

    IntParm m_BottomCont;
    BoolParm m_BottomLAngleSet;
    BoolParm m_BottomLSlewSet;
    BoolParm m_BottomLStrengthSet;
    BoolParm m_BottomLCurveSet;
    BoolParm m_BottomRAngleSet;
    BoolParm m_BottomRSlewSet;
    BoolParm m_BottomRStrengthSet;
    BoolParm m_BottomRCurveSet;
    BoolParm m_BottomLRAngleEq;
    BoolParm m_BottomLRSlewEq;
    BoolParm m_BottomLRStrengthEq;
    BoolParm m_BottomLRCurveEq;
    Parm m_BottomLAngle;
    Parm m_BottomLSlew;
    Parm m_BottomLStrength;
    Parm m_BottomLCurve;
    Parm m_BottomRAngle;
    Parm m_BottomRSlew;
    Parm m_BottomRStrength;
    Parm m_BottomRCurve;

    IntParm m_LeftCont;
    BoolParm m_LeftLAngleSet;
    BoolParm m_LeftLSlewSet;
    BoolParm m_LeftLStrengthSet;
    BoolParm m_LeftLCurveSet;
    BoolParm m_LeftRAngleSet;
    BoolParm m_LeftRSlewSet;
    BoolParm m_LeftRStrengthSet;
    BoolParm m_LeftRCurveSet;
    BoolParm m_LeftLRAngleEq;
    BoolParm m_LeftLRSlewEq;
    BoolParm m_LeftLRStrengthEq;
    BoolParm m_LeftLRCurveEq;
    Parm m_LeftLAngle;
    Parm m_LeftLSlew;
    Parm m_LeftLStrength;
    Parm m_LeftLCurve;
    Parm m_LeftRAngle;
    Parm m_LeftRSlew;
    Parm m_LeftRStrength;
    Parm m_LeftRCurve;

    virtual void Reset();
    virtual void SetContinuity( int cx );
    virtual int GetContinuity() const
    {
        return m_TopCont();
    }
    //==== Read the four skin values back for one side.  XSEC_BOTH_SIDES is not
    //==== a side, so the caller has to name XSEC_LEFT_SIDE or XSEC_RIGHT_SIDE.
    //==== The values come back in the order the setters take them: top, right,
    //==== bottom, left. ====//
    virtual vector < double > GetTanAngles( int side ) const;
    virtual vector < double > GetTanSlews( int side ) const;
    virtual vector < double > GetTanStrengths( int side ) const;
    virtual vector < double > GetCurvatures( int side ) const;

    virtual void SetTanAngles( int side, double top, double right, double bottom, double left );
    virtual void SetTanSlews( int side, double top, double right, double bottom, double left );
    virtual void SetTanStrengths( int side, double top, double right, double bottom, double left );
    virtual void SetCurvatures( int side, double top, double right, double bottom, double left );
    virtual void FlipLRSkinning();

    virtual void SetV2DefaultBehavior();
    virtual void ClearSkinning();

    virtual void ReadV2FileFuse2( xmlNodePtr &root );
    virtual void ReadV2FileFuse1( xmlNodePtr &root );

protected:

    vector< SkinSpine* > m_SpineVec;

};


class FuseXSec : public SkinXSec
{
public:
    FuseXSec( XSecCurve *xsc );

    virtual void Update();

    virtual void SetRefLength( double len );

    virtual void CopyBasePos( XSec* xs );
    virtual void CopyFuseXSParms( XSec* xs );

    virtual double GetScale();

    virtual void ReadV2FileFuse2( xmlNodePtr &root );
    virtual void ReadV2FileFuse1( xmlNodePtr &root );

    Parm m_FwdCluster;
    Parm m_AftCluster;

    FractionParm m_XLocPercent;
    FractionParm m_YLocPercent;
    FractionParm m_ZLocPercent;

    Parm m_Spin;

    Parm m_XRotate;
    Parm m_YRotate;
    Parm m_ZRotate;

    Parm m_RefLength;

protected:

    double m_RefLenVal;
};

class StackXSec : public SkinXSec
{
public:
    StackXSec( XSecCurve *xsc );

    virtual void SetScale( double scale );

    virtual void Update();

    virtual void CopyBasePos( XSec* xs );

    virtual double GetScale();

    Parm m_FwdCluster;
    Parm m_AftCluster;

    Parm m_VAlign;

    Parm m_XDelta;
    Parm m_YDelta;
    Parm m_ZDelta;

    IntParm m_XSAbsRelFlag;
    Parm m_XAbs;
    Parm m_YAbs;
    Parm m_ZAbs;

    Parm m_Spin;

    Parm m_XRotate;
    Parm m_YRotate;
    Parm m_ZRotate;

    Parm m_XRotateAbs;
    Parm m_YRotateAbs;
    Parm m_ZRotateAbs;

protected:
};

#endif // !defined(XSEC__INCLUDED_)
