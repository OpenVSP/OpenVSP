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

#include "Defines.h"
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

    virtual string GetName();


    virtual VspCurve& GetUntransformedCurve();
    virtual VspCurve& GetCurve();

    virtual vector< vec3d > GetDrawLines( Matrix4d &transMat );

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
    int m_GroupSuffix;

    VspCurve m_TransformedCurve;

    Matrix4d m_Transform;

    XSecCurve *m_XSCurve;

    virtual void ChangeID( string id );
};

class SkinXSec : public XSec
{
public:

    SkinXSec( XSecCurve *xsc );

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

    virtual void CrossValidateParms( BoolParm &topEq,
            BoolParm &rightEq,
            BoolParm &bottomEq,
            BoolParm &leftEq,
            BoolParm &topRSet,
            BoolParm &topLSet,
            bool CX );

    virtual void ValidateParms( );

    virtual rib_data_type GetRib( bool first, bool last );

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
    virtual void SetTanAngles( int side, double top, double right, double bottom, double left );
    virtual void SetTanSlews( int side, double top, double right, double bottom, double left );
    virtual void SetTanStrengths( int side, double top, double right, double bottom, double left );
    virtual void SetCurvatures( int side, double top, double right, double bottom, double left );

    virtual void SetV2DefaultBehavior();

    virtual void ReadV2FileFuse2( xmlNodePtr &root );
    virtual void ReadV2FileFuse1( xmlNodePtr &root );

protected:
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

    Parm m_XDelta;
    Parm m_YDelta;
    Parm m_ZDelta;

    Parm m_Spin;

    Parm m_XRotate;
    Parm m_YRotate;
    Parm m_ZRotate;

    Parm m_XCenterRot;
    Parm m_YCenterRot;
    Parm m_ZCenterRot;

protected:
};

#endif // !defined(XSEC__INCLUDED_)
