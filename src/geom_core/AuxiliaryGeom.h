//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//

// AuxiliaryGeom.h:
// Rob McDonald
//
//////////////////////////////////////////////////////////////////////



#if !defined(VSPAUXILIARYGEOM__INCLUDED_)
#define VSPAUXILIARYGEOM__INCLUDED_

#include <cmath>

#include "Geom.h"
#include "GeomInterface.h"
#include "XSec.h"
#include "XSecSurf.h"

class GearGeom;
class GearContactRole;

//==== What an auxiliary geom describes ====//
// Implemented by AuxiliaryGeom and by a Clone standing in for one; one interface covers every mode.
// The *InGear answers are in the frame of the gear the asked Geom hangs off, so a Clone
// measures against its own parent.
class AuxiliaryRole : virtual public GeomInterface
{
public:
    // The behavior type this role belongs to; checked by Geom::CastTo.
    static int BehaviorType()   { return AUXILIARY_GEOM_TYPE; }

    // The gear these answers are measured against.
    virtual GearContactRole* GetContactGear() const = 0;

    // Which arrangement this auxiliary describes.
    virtual int GetAuxiliaryMode() const = 0;

    // Whether a super cone is aligned to the world rather than to the head it hangs off.
    virtual bool GetAuxWorldAligned() const = 0;

    //==== In the gear's frame ====//
    virtual bool GetCGInGear( vec3d &cgnom, vector < vec3d > &cgbounds ) = 0;
    virtual bool GetPtNormalInGear( vec3d &pt, vec3d &normal ) const = 0;
    virtual bool GetPtPivotAxisInGear( vec3d &ptaxis, vec3d &axis ) = 0;
    virtual bool GetPtNormalMeanContactPtPivotAxisInGear( vec3d &pt, vec3d &normal, vec3d &ptaxis, vec3d &axis, bool &usepivot, double &mintheta, double &maxtheta ) = 0;
    virtual bool GetSideContactPtRollAxisNormalInGear( vec3d &pt, vec3d &axis, vec3d &normal, int &ysign ) = 0;
    virtual bool GetPtNormalAftAxleAxisInGear( double thetabogie, vec3d &pt, vec3d &normal, vec3d &ptaxis, vec3d &axis ) = 0;
    virtual bool GetPtNormalFwdAxleAxisInGear( double thetabogie, vec3d &pt, vec3d &normal, vec3d &ptaxis, vec3d &axis ) = 0;
    virtual bool GetTwoPtSideContactPtsNormalInGear( vec3d &p1, vec3d &p2, vec3d &normal ) = 0;
    virtual bool GetContactPointVecNormalInGear( vector < vec3d > &ptvec, vec3d &normal ) = 0;
    virtual bool CalculateTurnInGear( vec3d &cor, vec3d &normal, vector<double> &rvec ) = 0;

    //==== In this Geom's own frame ====//
    // The rotor burst spread.  It comes from this Geom's own Parms, so this Geom places it.
    virtual bool GetSpreadTriInSelf( vec3d &pt, vec3d &axis, vector < vec3d > &t, int &flip ) const = 0;

    //==== The same, in world coordinates ====//
    bool GetSpreadTri( vec3d &pt, vec3d &axis, vector < vec3d > &t, int &flip ) const;

    bool GetCG( vec3d &cgnom, vector < vec3d > &cgbounds );
    bool GetPtNormal( vec3d &pt, vec3d &normal ) const;
    bool GetPtNormalMeanContactPtPivotAxis( vec3d &pt, vec3d &normal, vec3d &ptaxis, vec3d &axis, bool &usepivot, double &mintheta, double &maxtheta );
    bool GetSideContactPtRollAxisNormal( vec3d &pt, vec3d &axis, vec3d &normal, int &ysign );
    bool GetPtNormalAftAxleAxis( double thetabogie, vec3d &pt, vec3d &normal, vec3d &ptaxis, vec3d &axis );
    bool GetPtNormalFwdAxleAxis( double thetabogie, vec3d &pt, vec3d &normal, vec3d &ptaxis, vec3d &axis );
    bool GetPtPivotAxis( vec3d &ptaxis, vec3d &axis );
    bool GetTwoPtSideContactPtsNormal( vec3d &p1, vec3d &p2, vec3d &normal );
    bool GetContactPointVecNormal( vector < vec3d > &ptvec, vec3d &normal );
    bool CalculateTurn( vec3d &cor, vec3d &normal, vector<double> &rvec );

protected:
    // Placement of that gear, or identity if there is none (every query above then returns false).
    Matrix4d GetContactGearMatrix() const;
};

//==== Auxiliary Geom ====//
class AuxiliaryGeom : public Geom, public AuxiliaryRole
{
public:
    AuxiliaryGeom( Vehicle* vehicle_ptr );
    virtual ~AuxiliaryGeom();

    virtual void ComputeCenter();

    virtual void ApplyScale( double currentScale );

    virtual void AddDefaultSources( double base_len = 1.0 );

    virtual void OffsetXSecs( double off );

    virtual bool ReadCCEFile( const string &fname );
    virtual bool ReadCCEFile( FILE* file_id );
    virtual void SetPnts( const vector<vec3d> &pnt_vec );
    virtual void UpdateCCECurve();

    virtual string GetNotes();

    virtual void SetXSecCurveType( int type );
    virtual int GetXSecCurveType();

    void CopyXSecCurve();
    void PasteXSecCurve();

    XSecCurve* GetXSecCurve()       { return m_XSCurve; }

    // Alternative to XSecSurf::ConvertToEdit for Auxiliary Geom SuperCone components
    virtual EditCurveXSec* ConvertToEdit();

    virtual void UpdateMainBBox();
    virtual bool IsModelScaleSensitive()        { return true; }

    virtual xmlNodePtr EncodeXml( xmlNodePtr & node );
    virtual xmlNodePtr DecodeXml( xmlNodePtr & node );
    virtual void AddLinkableParms( vector< string > & parm_vec, const string & link_container_id = string() );

    virtual void SetContactPt1ID( const std::string& id );
    virtual void SetContactPt2ID( const std::string& id );
    virtual void SetContactPt3ID( const std::string& id );

    virtual std::string GetContactPt1ID() const           { return m_ContactPt1_ID; }
    virtual std::string GetContactPt2ID() const           { return m_ContactPt2_ID; }
    virtual std::string GetContactPt3ID() const           { return m_ContactPt3_ID; }

    // The shape is built in the parent's frame, and the attachment carries no flip, so the
    // parent's flip is applied here.
    virtual Matrix4d GetFlipMat() const;
    virtual bool GetFlipReversesNormal() const;
    virtual int GetFlipFlag() const;

    // Its parent's flip, not one of its own.
    virtual bool FlipApplies() const
    {
        return false;
    }

    //==== The gear-frame halves of the interface ====//
    virtual GearContactRole* GetContactGear() const;
    virtual bool GetAuxWorldAligned() const
    {
        return m_SCWorldAligned();
    }

    virtual int GetAuxiliaryMode() const
    {
        return m_AuxuliaryGeomMode();
    }

    virtual bool GetCGInGear( vec3d &cgnom, vector < vec3d > &cgbounds );
    virtual bool GetPtNormalInGear( vec3d &pt, vec3d &normal ) const;
    virtual bool GetPtPivotAxisInGear( vec3d &ptaxis, vec3d &axis );
    virtual bool GetPtNormalMeanContactPtPivotAxisInGear( vec3d &pt, vec3d &normal, vec3d &ptaxis, vec3d &axis, bool &usepivot, double &mintheta, double &maxtheta );
    virtual bool GetSideContactPtRollAxisNormalInGear( vec3d &pt, vec3d &axis, vec3d &normal, int &ysign );
    virtual bool GetPtNormalAftAxleAxisInGear( double thetabogie, vec3d &pt, vec3d &normal, vec3d &ptaxis, vec3d &axis );
    virtual bool GetPtNormalFwdAxleAxisInGear( double thetabogie, vec3d &pt, vec3d &normal, vec3d &ptaxis, vec3d &axis );
    virtual bool GetTwoPtSideContactPtsNormalInGear( vec3d &p1, vec3d &p2, vec3d &normal );
    virtual bool GetContactPointVecNormalInGear( vector < vec3d > &ptvec, vec3d &normal );
    virtual bool CalculateTurnInGear( vec3d &cor, vec3d &normal, vector<double> &rvec );
    virtual bool GetSpreadTriInSelf( vec3d &pt, vec3d &axis, vector < vec3d > &t, int &flip ) const;

    IntParm m_AuxuliaryGeomMode;

    BoolParm m_AutoDiam;
    Parm m_Diameter;
    Parm m_FlapRadiusFract;

    Parm m_RootLength;
    Parm m_RootOffset;

    IntParm m_RotorFragmentMode;
    Parm m_ThetaThrust;
    Parm m_ThetaAntiThrust;

    Parm m_DiskRadius;
    Parm m_BladeLength;
    Parm m_BladeRootRadius;

    Parm m_FragLength;
    Parm m_CGRadius;

    Parm m_ReleaseAngle;

    BoolParm m_RotDir;

    IntParm m_ThrownBladeMode;
    Parm m_ThrownBladeCGFrac;

    int m_ParentType;


    string m_ContactPt1_ID;
    IntParm m_ContactPt1_Isymm;
    IntParm m_ContactPt1_SuspensionMode;
    IntParm m_ContactPt1_TireMode;
    IntParm m_ContactPt1_ClearanceMode;
    IntParm m_ContactPt1_GearMode;
    Parm m_ContactPt1_KRetract;

    string m_ContactPt2_ID;
    IntParm m_ContactPt2_Isymm;
    IntParm m_ContactPt2_SuspensionMode;
    IntParm m_ContactPt2_TireMode;

    string m_ContactPt3_ID;
    IntParm m_ContactPt3_Isymm;
    IntParm m_ContactPt3_SuspensionMode;
    IntParm m_ContactPt3_TireMode;

    IntParm m_CCEUnits;
    Parm m_CCEMainGearOffset;

    BoolParm m_SCWorldAligned;

    Parm m_BogieTheta;
    Parm m_WheelTheta;
    Parm m_RollTheta;


    Parm m_SprayTireContactWidth;
    Parm m_SprayTireContactHalfLength;

    Parm m_SpraySideElevationAngle;
    Parm m_SpraySidePlanAngle;
    Parm m_SpraySideIncrementalAngle;
    Parm m_SpraySideInclinationAngle;

    Parm m_SprayCenterElevationAngle;
    Parm m_SprayCenterWidth;


    IntParm m_WheelTireFailureMode;

protected:

    virtual void SetDirtyFlags( Parm* parm_ptr );

    virtual void UpdateSurf();
    virtual void UpdateFeatureLines();
    virtual void UpdateLCurve();
    virtual void UpdateMainTessVec();
    virtual void UpdateMainDegenGeomPreview();
    virtual void UpdateCopyXFormParms();
    virtual void UpdateCopySurfParms();
    virtual void UpdateCopyTessParms();
    virtual void UpdateFlags();

    virtual void UpdateDrawObj();
    virtual void LoadDrawObjs( vector< DrawObj* > & draw_obj_vec );

    void AppendContact1Surfs( GearContactRole * gear, double bogietheta = 0 );
    void AppendContact2Surfs( GearContactRole * gear, double bogietheta = 0 );
    void AppendContact3Surfs( GearContactRole * gear, double bogietheta = 0 );

    // The Tess and Degen contact methods write into m_MainTessVec / m_MainFeatureTessVec and
    // m_MainDegenGeomPreviewVec starting at index itess / idegen and return the index one past the last
    // element written.  The tess and feature tess vectors advance in lockstep, so one cursor serves both.
    int TessContact1( GearContactRole * gear, int itess, double bogietheta = 0 );
    int TessContact2( GearContactRole * gear, int itess, double bogietheta = 0 );
    int TessContact3( GearContactRole * gear, int itess, double bogietheta = 0 );

    int DegenContact1( GearContactRole * gear, int idegen, double bogietheta = 0 );
    int DegenContact2( GearContactRole * gear, int idegen, double bogietheta = 0 );
    int DegenContact3( GearContactRole * gear, int idegen, double bogietheta = 0 );

    vector < vec3d > m_ContactPts;

    vec3d m_BasisOrigin;
    vector < vec3d > m_BasisAxis;

    vector<DrawObj> m_BasisDrawObj_vec;
    DrawObj m_ContactDrawObj;

    vector< vec3d > m_CCEFilePnts;

    VspCurve m_CCECurve;

    XSecCurve *m_XSCurve;
};

#endif // !defined(VSPAUXILIARYGEOM__INCLUDED_)
