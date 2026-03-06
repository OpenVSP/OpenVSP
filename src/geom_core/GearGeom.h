//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//

// GearGeom.h:
// Rob McDonald
//
//////////////////////////////////////////////////////////////////////

#if !defined(VSPGEARGEOM__INCLUDED_)
#define VSPGEARGEOM__INCLUDED_

#include "Geom.h"
#include "XSec.h"

class Bogie : public ParmContainer
{
public:
    Bogie();

    // virtual xmlNodePtr EncodeXml( xmlNodePtr & node );
    // virtual xmlNodePtr DecodeXml( xmlNodePtr & node );
    virtual void ParmChanged( Parm* parm_ptr, int type );
    virtual void SetDirtyFlags( Parm* parm_ptr );

    int GetNumSurf() const;
    void UpdateParms();
    void UpdateTireCurve();
    void UpdateStowAttachParms();
    void UpdateMechAttachParms();
    void DeactivateStowXForms();
    void DeactivateMechXForms();
    void ComposeStowAttachMatrix();
    void ComposeMechAttachMatrix();
    void BuildRetractMatrix( Matrix4d &ret_mat, vec3d &knee_pt, vec3d &knee_ax, double k, int isymm ) const;
    void UpdateRetract();
    void BackCalculateRetract();
    void Update();
    void UpdateRetractAttach();
    void UpdateTess();
    void UpdateDrawObj( const Matrix4d &relTrans );
    void LoadDrawObjs( vector< DrawObj* > & draw_obj_vec );

    virtual xmlNodePtr EncodeXml( xmlNodePtr & node );
    virtual xmlNodePtr DecodeXml( xmlNodePtr & node );

    string GetAcrossDesignation();
    string GetConfigDesignation();
    string GetTireDesignation();
    string GetDesignation( const char* sep = " " );

    double GetTireRadius( int tiremode ) const;
    vec3d GetTireDeflection( int tiremode ) const;
    vec3d GetCompressionUnitDirection( int isymm ) const;
    vec3d GetSuspensionDeflection( int isymm, int suspensionmode ) const;
    vec3d GetContinuiousSuspensionDeflection( int isymm, double d ) const;
    vec3d GetNominalMeanContactPoint( int isymm ) const;
    vec3d GetMeanContactPoint( int isymm, int tiremode, int suspensionmode, double thetabogie ) const;
    vec3d GetNominalPivotPoint( int isymm ) const;
    vec3d GetPivotPoint( int isymm, int suspensionmode ) const;
    vec3d GetContinuiousPivotPoint( int isymm, double d ) const;
    double GetAxleArm() const;
    double GetBogieSemiWidth() const;
    vec3d GetAxleDisplacement( double thetabogie ) const;
    vec3d GetFwdAxle( int isymm, int suspensionmode, double thetabogie ) const;
    vec3d GetAftAxle( int isymm, int suspensionmode, double thetabogie ) const;
    vec3d GetFwdContactPoint( int isymm, int suspensionmode, int tiremode, double thetabogie, double thetawheel ) const;
    vec3d GetAftContactPoint( int isymm, int suspensionmode, int tiremode, double thetabogie, double thetawheel ) const;
    vec3d GetSideContactPoint( int isymm, int suspensionmode, int tiremode, double thetabogie, double thetawheel, int ysign ) const;

    virtual bool IsStowParentJoint();
    virtual bool IsMechParentJoint();

    virtual void SetStowParentID( const string &id );
    virtual string GetStowParentID()
    {
        return m_StowParentID;
    }

    virtual void SetMechParentID( const string &id );
    virtual string GetMechParentID()
    {
        return m_MechParentID;
    }

    // T must have methods .FlipNormal() and .Transform( Matrix4d )
    template <typename T>
    void TireToBogie( const T &source, vector<T> &dest, int isymm, int suspensionmode, double bogietheta, bool stow = false, bool ret = false, double kretract = 0.0 ) const
    {
        int idest = dest.size();

        int na = m_NAcross();
        int nt = m_NTandem();

        int n = na * nt;

        dest.resize( idest + n, source );

        double s = m_Spacing();
        double p = m_Pitch();


        double cenAcross = 0.5 * ( na - 1 ) * s;
        double cenTandem = 0.5 * ( nt - 1 ) * p;

        Matrix4d pivot;

        if ( stow )
        {
            if ( isymm == 0 )
            {
                pivot = m_StowTransform;
            }
            else
            {
                pivot = m_StowSymmTransform;
            }
        }
        else
        {
            if ( !ret )
            {
                pivot.translatev( GetPivotPoint( isymm, suspensionmode ) );

                pivot.rotateY( -bogietheta );
            }
            else
            {
                vec3d knee_pt, knee_ax;
                BuildRetractMatrix( pivot, knee_pt, knee_ax, kretract, isymm );
            }
        }

        Matrix4d symm;
        double ksymm = 1.0;
        if ( isymm > 0 )
        {
            ksymm = -1.0;
            symm.loadXZRef();
        }
        symm.postMult( pivot );

        for ( int i = 0; i < na; i++ )
        {
            Matrix4d col = symm;
            col.translatef( 0, ksymm * ( i * s - cenAcross ), 0 );

            for ( int j = 0; j < nt; j++ )
            {
                Matrix4d row = col;
                row.translatef( j * p - cenTandem, 0, 0 );

                dest[ idest ].Transform( row );

                if ( isymm > 0 )
                {
                    dest[ idest ].FlipNormal();
                }

                idest++;
            }
        }
    }

    template <typename T>
    void TireToBogie( const T &source, vector<T> &dest, int gear_config ) const
    {
        int nsymm = 1;
        if ( m_Symmetrical() )
        {
            nsymm = 2;
        }

        double bogietheta = 0;
        // if ( !m_DrawNominal() )
        // {
        //     bogietheta = m_BogieTheta();
        // }

        if ( gear_config == vsp::GEAR_CONFIGURATION_DOWN ||
             gear_config == vsp::GEAR_CONFIGURATION_UP_AND_DOWN ||
             gear_config == vsp::GEAR_CONFIGURATION_ALL )
        {
            for ( int isymm = 0; isymm < nsymm; isymm++ )
            {
                TireToBogie( source, dest, isymm, vsp::GEAR_SUSPENSION_NOMINAL, bogietheta, false );
            }
        }

        if ( gear_config == vsp::GEAR_CONFIGURATION_UP ||
             gear_config == vsp::GEAR_CONFIGURATION_UP_AND_DOWN ||
             gear_config == vsp::GEAR_CONFIGURATION_ALL )
        {
            for ( int isymm = 0; isymm < nsymm; isymm++ )
            {
                if ( m_RetMode() == vsp::GEAR_STOWED_POSITION )
                {
                    TireToBogie( source, dest, isymm, vsp::GEAR_SUSPENSION_NOMINAL, bogietheta, true );
                }
                else
                {
                    TireToBogie( source, dest, isymm, vsp::GEAR_SUSPENSION_NOMINAL, bogietheta, false, true, 1.0 );
                }
            }
        }

        if ( gear_config == vsp::GEAR_CONFIGURATION_INTERMEDIATE ||
             gear_config == vsp::GEAR_CONFIGURATION_ALL )
        {
            if ( m_RetMode() == vsp::GEAR_MECHANISM )
            {
                for ( int isymm = 0; isymm < nsymm; isymm++ )
                {
                    TireToBogie( source, dest, isymm, vsp::GEAR_SUSPENSION_NOMINAL, bogietheta, false, true, m_MechKRetract() );
                }
            }
        }
    }

    void AppendMainSurf( vector < VspSurf > &surfvec, int gear_config ) const;

    bool m_Visible;
    Matrix4d m_StowTransform;
    Matrix4d m_StowSymmTransform;
    Matrix4d m_StowAttachMatrix;
    Matrix4d m_MechTransform;
    Matrix4d m_MechSymmTransform;
    Matrix4d m_MechAttachMatrix;
    Matrix4d m_GearModelMatrix;

    VspCurve m_TireProfile;
    VspSurf m_TireSurface;

    SimpleTess m_TireTess;
    SimpleFeatureTess m_TireFeatureTess;

    bool m_TireDirty;
    bool m_TireTessDirty;

    DrawObj m_SuspensionTravelLinesDO;
    DrawObj m_SuspensionTravelPointsDO;

    vec3d m_StrutAttachPt;
    vec3d m_Axis;
    vec3d m_StowPivotPt;
    vector < vec3d > m_PivotPtVec;
    vector < vec3d > m_KneePtVec;
    vector < vec3d > m_KneeAxVec;

    DrawObj m_AxisDO;
    DrawObj m_AxisCircleDO;
    DrawObj m_AxisArrowDO;
    DrawObj m_StrutDO;

    // Bogie
    BoolParm m_Symmetrical;

    // BoolParm m_DrawNominal;

    IntParm m_NAcross;
    IntParm m_NTandem;

    IntParm m_SpacingType;
    Parm m_Spacing;
    Parm m_SpacingFrac;
    Parm m_SpacingGap;
    Parm m_SpacingGapFrac;

    IntParm m_PitchType;
    Parm m_Pitch;
    Parm m_PitchFrac;
    Parm m_PitchGap;
    Parm m_PitchGapFrac;

    Parm m_XContactPt;
    Parm m_YContactPt;
    Parm m_ZAboveGround;

    Parm m_TravelX;
    Parm m_TravelY;
    Parm m_TravelZ;

    // Parm m_Travel;
    Parm m_TravelCompressed;
    Parm m_TravelExtended;

    // Parm m_BogieTheta;
    Parm m_BogieThetaMax;
    Parm m_BogieThetaMin;

    Parm m_SteeringAngle;


    // Tire
    IntParm m_TireMode;
    BoolParm m_WidthMode;
    Parm m_WidthIn;
    Parm m_WidthModel;

    BoolParm m_DiameterMode;
    Parm m_DiameterIn;
    Parm m_DiameterModel;

    IntParm m_SLRMode;
    Parm m_DeflectionPct;
    Parm m_StaticRadiusIn;
    Parm m_StaticRadiusModel;

    IntParm m_DrimMode;
    Parm m_DrimFrac;
    Parm m_DrimIn;
    Parm m_DrimModel;

    IntParm m_WrimMode;
    Parm m_WrimFrac;
    Parm m_WrimIn;
    Parm m_WrimModel;

    Parm m_PlyRating;

    IntParm m_WsMode;
    Parm m_WsFrac;
    Parm m_WsIn;
    Parm m_WsModel;

    IntParm m_HsMode;
    Parm m_HsFrac;
    Parm m_HsIn;
    Parm m_HsModel;

    Parm m_DFlangeModel;

    Parm m_WGModel;
    Parm m_DGModel;
    Parm m_WsGModel;
    Parm m_DsGModel;

    // Retracted
    IntParm m_RetMode;

    string m_StowParentID;
    IntParm m_StowSurfIndx;

    Parm m_StowXLoc;
    Parm m_StowYLoc;
    Parm m_StowZLoc;

    Parm m_StowXRelLoc;
    Parm m_StowYRelLoc;
    Parm m_StowZRelLoc;

    Parm m_StowXRot;
    Parm m_StowYRot;
    Parm m_StowZRot;

    Parm m_StowXRelRot;
    Parm m_StowYRelRot;
    Parm m_StowZRelRot;

    IntParm m_StowAbsRelFlag;
    IntParm m_StowTransAttachFlag;
    IntParm m_StowRotAttachFlag;

    Parm m_StowULoc;
    Parm m_StowU0NLoc;
    BoolParm m_StowU01;
    Parm m_StowWLoc;

    Parm m_StowRLoc;
    BoolParm m_StowR01;
    Parm m_StowR0NLoc;
    Parm m_StowSLoc;
    Parm m_StowTLoc;

    Parm m_StowLLoc;
    BoolParm m_StowL01;
    Parm m_StowL0LenLoc;
    Parm m_StowMLoc;
    Parm m_StowNLoc;

    Parm m_StowEtaLoc;


    Parm m_MechKRetract;

    string m_MechParentID;
    IntParm m_MechSurfIndx;

    Parm m_MechXLoc;
    Parm m_MechYLoc;
    Parm m_MechZLoc;

    Parm m_MechXAxis;
    Parm m_MechYAxis;
    Parm m_MechZAxis;

    Parm m_MechXRelLoc;
    Parm m_MechYRelLoc;
    Parm m_MechZRelLoc;

    IntParm m_MechAbsRelFlag;
    IntParm m_MechTransAttachFlag;

    Parm m_MechULoc;
    Parm m_MechU0NLoc;
    BoolParm m_MechU01;
    Parm m_MechWLoc;

    Parm m_MechRLoc;
    BoolParm m_MechR01;
    Parm m_MechR0NLoc;
    Parm m_MechSLoc;
    Parm m_MechTLoc;

    Parm m_MechLLoc;
    BoolParm m_MechL01;
    Parm m_MechL0LenLoc;
    Parm m_MechMLoc;
    Parm m_MechNLoc;

    Parm m_MechEtaLoc;

    Parm m_MechKneePos;
    Parm m_MechKneeAngle;
    Parm m_MechKneeAzimuthAngle;
    Parm m_MechKneeElevationAngle;
    Parm m_MechKneeDownAngle;

    Parm m_MechRetAngle;
    Parm m_MechTwistAngle;
    Parm m_MechRollAngle;
    Parm m_MechBogieAngle;

    Parm m_MechStrutDL;
};


//==== Pod Geom ====//
class GearGeom : public Geom
{
public:
    GearGeom( Vehicle* vehicle_ptr );
    virtual ~GearGeom();

    virtual void ComputeCenter();
    virtual void Scale();
    virtual void AddDefaultSources( double base_len = 1.0 );

    Bogie * CreateAndAddBogie();
    string CreateAndAddBogie( int foo );

    Bogie * GetCurrentBogie();
    std::vector < Bogie * > GetBogieVec();

    virtual xmlNodePtr EncodeXml( xmlNodePtr & node );
    virtual xmlNodePtr DecodeXml( xmlNodePtr & node );

    virtual void AddLinkableParms( vector< string > & linkable_parm_vec, const string & link_container_id = string() );
    virtual void ChangeID( const string &id );
    virtual void UpdateParents();

    void SetCurrBogieIndex( int i )             { m_CurrBogieIndex = i; }
    int GetCurrBogieIndex()                     { return m_CurrBogieIndex; }
    void DelAllBogies();
    void ShowAllBogies();
    void HideAllBogies();
    Bogie *  GetBogie( const string &id ) const;
    vector < string > GetAllBogies();
    void DelBogie( const int &i );
    void DelBogie( const string &id );

    virtual void UpdateBBox();
    virtual bool IsModelScaleSensitive()        { return m_AutoPlaneFlag(); }

    virtual void BuildOnePtBasis( const string &cp1, int isymm1, int suspension1, int tire1,
                                  double thetabogie, double thetawheel, double thetaroll, Matrix4d &mat, vec3d &p1 );

    virtual void BuildTwoPtBasis( const string &cp1, int isymm1, int suspension1, int tire1,
                                  const string &cp2, int isymm2, int suspension2, int tire2,
                                  double thetabogie, Matrix4d &mat, vec3d &p1, vec3d &p2 );

    virtual void BuildThreePtBasis( const string &cp1, int isymm1, int suspension1, int tire1,
                                    const string &cp2, int isymm2, int suspension2, int tire2,
                                    const string &cp3, int isymm3, int suspension3, int tire3,
                                    Matrix4d &mat );

    virtual void BuildThreePtOffAxisBasis( const string &cp1, int isymm1, int suspension1, int tire1,
                                    const string &cp2, int isymm2, int suspension2, int tire2,
                                    const string &cp3, int isymm3, int suspension3, int tire3,
                                    double mainoffset,
                                    Matrix4d &mat );

    virtual bool GetTwoPtPivot( const string &cp1, int isymm1, int suspension1,
                                    const string &cp2, int isymm2, int suspension2,
                                    vec3d &ptaxis, vec3d &axis ) const;

    virtual bool GetTwoPtAftAxleAxis( const string &cp1, int isymm1, int suspension1,
                                      const string &cp2, int isymm2, int suspension2,
                                      double thetabogie, vec3d &ptaxis, vec3d &axis ) const;

    virtual bool GetTwoPtFwdAxleAxis( const string &cp1, int isymm1, int suspension1,
                                      const string &cp2, int isymm2, int suspension2,
                                      double thetabogie, vec3d &ptaxis, vec3d &axis ) const;

    virtual bool GetTwoPtMeanContactPtNormal( const string &cp1, int isymm1, int suspension1, int tire1,
                                              const string &cp2, int isymm2, int suspension2, int tire2,
                                              double thetabogie, vec3d &pt, vec3d &normal, vec3d &p1, vec3d &p2, bool &usepivot, double &mintheta, double &maxtheta ) const;

    virtual bool GetTwoPtAftContactPtNormal( const string &cp1, int isymm1, int suspension1, int tire1,
                                             const string &cp2, int isymm2, int suspension2, int tire2,
                                             double thetabogie, double thetawheel, vec3d &pt, vec3d &normal, vec3d &p1, vec3d &p2 ) const;

    virtual bool GetTwoPtFwdContactPtNormal( const string &cp1, int isymm1, int suspension1, int tire1,
                                             const string &cp2, int isymm2, int suspension2, int tire2,
                                             double thetabogie, double thetawheel, vec3d &pt, vec3d &normal, vec3d &p1, vec3d &p2 ) const;

    virtual bool GetTwoPtSideContactPtsNormal( const string &cp1, int isymm1, int suspension1, int tire1,
                                               const string &cp2, int isymm2, int suspension2, int tire2,
                                               vec3d &p1, vec3d &p2, vec3d &normal ) const;

    virtual bool GetOnePtSideContactPtAxisNormal( const string &cp1, int isymm1, int suspension1, int tire1,
                                                  double thetabogie, double thetawheel, double thetaroll, vec3d &p1, vec3d &axis, vec3d &normal, int &ysign ) const;

    virtual bool GetPtNormal( const string &cp1, int isymm1, int suspension1, int tire1,
                              const string &cp2, int isymm2, int suspension2, int tire2,
                              const string &cp3, int isymm3, int suspension3, int tire3,
                              vec3d &pt, vec3d &normal ) const;

    virtual bool GetSteerAngle( const string &cp1, const string &cp2, const string &cp3, int &isteer, double &steerangle ) const;

    virtual bool GetTwoPtPivotInWorld( const string &cp1, int isymm1, int suspension1,
                                           const string &cp2, int isymm2, int suspension2,
                                           vec3d &ptaxis, vec3d &axis ) const;

    virtual bool GetTwoPtAftAxleAxisInWorld( const string &cp1, int isymm1, int suspension1,
                                             const string &cp2, int isymm2, int suspension2,
                                             double thetabogie, vec3d &ptaxis, vec3d &axis ) const;

    virtual bool GetTwoPtFwdAxleAxisInWorld( const string &cp1, int isymm1, int suspension1,
                                             const string &cp2, int isymm2, int suspension2,
                                             double thetabogie, vec3d &ptaxis, vec3d &axis ) const;

    virtual bool GetTwoPtMeanContactPtNormalInWorld( const string &cp1, int isymm1, int suspension1, int tire1,
                                                     const string &cp2, int isymm2, int suspension2, int tire2,
                                                     double thetabogie, vec3d &pt, vec3d &normal, bool &usepivot, double &mintheta, double &maxtheta ) const;

    virtual bool GetTwoPtAftContactPtNormalInWorld( const string &cp1, int isymm1, int suspension1, int tire1,
                                                    const string &cp2, int isymm2, int suspension2, int tire2,
                                                    double thetabogie, double thetawheel, vec3d &pt, vec3d &normal ) const;

    virtual bool GetTwoPtFwdContactPtNormalInWorld( const string &cp1, int isymm1, int suspension1, int tire1,
                                                    const string &cp2, int isymm2, int suspension2, int tire2,
                                                    double thetabogie, double thetawheel, vec3d &pt, vec3d &normal ) const;

    virtual bool GetTwoPtSideContactPtsNormalInWorld( const string &cp1, int isymm1, int suspension1, int tire1,
                                                      const string &cp2, int isymm2, int suspension2, int tire2,
                                                      vec3d &p1, vec3d &p2, vec3d &normal ) const;

    virtual bool GetOnePtSideContactPtAxisNormalInWorld( const string &cp1, int isymm1, int suspension1, int tire1,
                                                         double thetabogie, double thetawheel, double thetaroll, vec3d &p1, vec3d &axis, vec3d &normal, int &ysign ) const;

    virtual bool GetPtNormalInWorld( const string &cp1, int isymm1, int suspension1, int tire1,
                                     const string &cp2, int isymm2, int suspension2, int tire2,
                                     const string &cp3, int isymm3, int suspension3, int tire3,
                                     vec3d &pt, vec3d &normal ) const;

    virtual void GetNominalPtNormalInWorld( vec3d &pt, vec3d &normal ) const;

    virtual void GetCGInWorld( vec3d &cgnom, vector < vec3d > &cgbounds ) const;

    virtual bool GetContactPointVecNormal( const string &cp1, int isymm1, int suspension1, int tire1,
                                           const string &cp2, int isymm2, int suspension2, int tire2,
                                           const string &cp3, int isymm3, int suspension3, int tire3,
                                           vector < vec3d > &ptvec, vec3d &normal ) const;

    virtual bool GetContactPointVecNormalInWorld( const string &cp1, int isymm1, int suspension1, int tire1,
                                                  const string &cp2, int isymm2, int suspension2, int tire2,
                                                  const string &cp3, int isymm3, int suspension3, int tire3,
                                                  vector < vec3d > &ptvec, vec3d &normal ) const;

    IntParm m_ModelLenUnits;

    BoolParm m_IncludeNominalGroundPlane;

    IntParm m_GearConfigMode;

    Parm m_PlaneSize;
    BoolParm m_AutoPlaneFlag;

    BoolParm m_CGLocalFlag;

    Parm m_XCGMinLocal;
    Parm m_XCGMaxLocal;
    Parm m_XCGNominalLocal;
    Parm m_YCGMinLocal;
    Parm m_YCGMaxLocal;
    Parm m_YCGNominalLocal;
    Parm m_ZCGMinLocal;
    Parm m_ZCGMaxLocal;
    Parm m_ZCGNominalLocal;

    Parm m_XCGMinGlobal;
    Parm m_XCGMaxGlobal;
    Parm m_XCGNominalGlobal;
    Parm m_YCGMinGlobal;
    Parm m_YCGMaxGlobal;
    Parm m_YCGNominalGlobal;
    Parm m_ZCGMinGlobal;
    Parm m_ZCGMaxGlobal;
    Parm m_ZCGNominalGlobal;

protected:
    virtual void SetDirtyFlags( Parm* parm_ptr );

    virtual void UpdateSurf();
    virtual void UpdateXForm();
    virtual void UpdateFeatureLines();
    virtual void UpdateMainTessVec();
    virtual void UpdateTessVec();
    virtual void UpdateMainDegenGeomPreview();
    virtual void UpdateDrawObj();
    virtual void LoadDrawObjs( vector< DrawObj* > & draw_obj_vec );

    std::vector < Bogie * > m_Bogies;
    std::vector < int > m_BogieMainSurfIndex;
    int m_CurrBogieIndex;

    std::size_t m_ParentHash;
    vector < string > m_ParentVec;

    vector < vec3d > m_MainNominalCGPointVec;
    vec3d m_MainMinCGPoint;
    vec3d m_MainMaxCGPoint;

    vector < vec3d > m_NominalCGPointVec;
    vector < SimpleFeatureTess > m_LimitsCGPointVec;

    DrawObj m_CGNominalDrawObj;
    DrawObj m_CGLimitsDrawObj;

};


#endif // !defined(VSPGEARGEOM__INCLUDED_)
