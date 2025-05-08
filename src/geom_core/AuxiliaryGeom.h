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

#include "Defines.h"
#include "Geom.h"
#include "XSec.h"
#include "XSecSurf.h"

class GearGeom;

//==== Auxiliary Geom ====//
class AuxiliaryGeom : public Geom
{
public:
    AuxiliaryGeom( Vehicle* vehicle_ptr );
    virtual ~AuxiliaryGeom();

    virtual void ComputeCenter();

    virtual void Scale();

    virtual void AddDefaultSources( double base_len = 1.0 );

    virtual void OffsetXSecs( double off );

    virtual void UpdateBBox();
    virtual bool IsModelScaleSensitive()        { return true; }

    virtual xmlNodePtr EncodeXml( xmlNodePtr & node );
    virtual xmlNodePtr DecodeXml( xmlNodePtr & node );

    virtual void SetContactPt1ID( const std::string& id );
    virtual void SetContactPt2ID( const std::string& id );
    virtual void SetContactPt3ID( const std::string& id );

    virtual void GetCG( vec3d &cgnom, vector < vec3d > &cgbounds );
    virtual void GetPtNormal( vec3d &pt, vec3d &normal ) const;
    virtual void GetPtPivotAxis( vec3d &ptaxis, vec3d &axis );
    virtual void GetPtNormalMeanContactPtPivotAxis( vec3d &pt, vec3d &normal, vec3d &ptaxis, vec3d &axis, bool &usepivot, double &mintheta, double &maxtheta );
    virtual void GetSideContactPtRollAxisNormal( vec3d &pt, vec3d &axis, vec3d &normal, int &ysign );
    virtual void GetPtNormalAftAxleAxis( double thetabogie, vec3d &pt, vec3d &normal, vec3d &ptaxis, vec3d &axis );
    virtual void GetPtNormalFwdAxleAxis( double thetabogie, vec3d &pt, vec3d &normal, vec3d &ptaxis, vec3d &axis );
    virtual void GetTwoPtSideContactPtsNormal( vec3d &p1, vec3d &p2, vec3d &normal );
    virtual void GetContactPointVecNormal( vector < vec3d > &ptvec, vec3d &normal );
    virtual void CalculateTurn( vec3d &cor, vec3d &normal, vector<double> &rvec );

    IntParm m_AuxuliaryGeomMode;

    BoolParm m_AutoDiam;
    Parm m_Diameter;
    Parm m_FlapRadiusFract;

    Parm m_RootLength;
    Parm m_RootOffset;

    Parm m_ThetaThrust;
    Parm m_ThetaAntiThrust;

    int m_ParentType;


    string m_ContactPt1_ID;
    IntParm m_ContactPt1_Isymm;
    IntParm m_ContactPt1_SuspensionMode;
    IntParm m_ContactPt1_TireMode;

    string m_ContactPt2_ID;
    IntParm m_ContactPt2_Isymm;
    IntParm m_ContactPt2_SuspensionMode;
    IntParm m_ContactPt2_TireMode;

    string m_ContactPt3_ID;
    IntParm m_ContactPt3_Isymm;
    IntParm m_ContactPt3_SuspensionMode;
    IntParm m_ContactPt3_TireMode;


    Parm m_BogieTheta;
    Parm m_WheelTheta;
    Parm m_RollTheta;
protected:

    virtual void SetDirtyFlags( Parm* parm_ptr );

    virtual void UpdateSurf();
    virtual void UpdateMainTessVec();
    virtual void UpdateMainDegenGeomPreview();
    virtual void CopyDataFrom( Geom* geom_ptr );

    virtual void UpdateDrawObj();
    virtual void LoadDrawObjs( vector< DrawObj* > & draw_obj_vec );

    void AppendContact1Surfs( GearGeom * gear, double bogietheta = 0 );
    void AppendContact2Surfs( GearGeom * gear, double bogietheta = 0 );
    void AppendContact3Surfs( GearGeom * gear, double bogietheta = 0 );

    void TessContact1( GearGeom * gear, double bogietheta = 0 );
    void TessContact2( GearGeom * gear, double bogietheta = 0 );
    void TessContact3( GearGeom * gear, double bogietheta = 0 );

    void DegenContact1( GearGeom * gear, double bogietheta = 0 );
    void DegenContact2( GearGeom * gear, double bogietheta = 0 );
    void DegenContact3( GearGeom * gear, double bogietheta = 0 );

    vector < vec3d > m_ContactPts;

    vec3d m_BasisOrigin;
    vector < vec3d > m_BasisAxis;

    vector<DrawObj> m_BasisDrawObj_vec;
    DrawObj m_ContactDrawObj;
};

#endif // !defined(VSPAUXILIARYGEOM__INCLUDED_)
