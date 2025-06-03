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

    int GetNumSurf() const;
    void UpdateParms();
    void UpdateTireCurve();
    void Update();
    void UpdateDrawObj( const Matrix4d &relTrans );
    void LoadDrawObjs( vector< DrawObj* > & draw_obj_vec );

    string GetAcrossDesignation();
    string GetConfigDesignation();
    string GetTireDesignation();
    string GetDesignation( const char* sep = " " );

    double GetTireRadius( int tiremode ) const;
    vec3d GetTireDeflection( int tiremode ) const;
    vec3d GetCompressionUnitDirection( int isymm ) const;
    vec3d GetSuspensionDeflection( int isymm, int suspensionmode ) const;
    vec3d GetNominalMeanContactPoint( int isymm ) const;
    vec3d GetMeanContactPoint( int isymm, int tiremode, int suspensionmode, double thetabogie ) const;
    vec3d GetNominalPivotPoint( int isymm ) const;
    vec3d GetPivotPoint( int isymm, int suspensionmode ) const;
    double GetAxleArm() const;
    double GetBogieSemiWidth() const;
    vec3d GetAxleDisplacement( double thetabogie ) const;
    vec3d GetFwdAxle( int isymm, int suspensionmode, double thetabogie ) const;
    vec3d GetAftAxle( int isymm, int suspensionmode, double thetabogie ) const;
    vec3d GetFwdContactPoint( int isymm, int suspensionmode, int tiremode, double thetabogie, double thetawheel ) const;
    vec3d GetAftContactPoint( int isymm, int suspensionmode, int tiremode, double thetabogie, double thetawheel ) const;
    vec3d GetSideContactPoint( int isymm, int suspensionmode, int tiremode, double thetabogie, double thetawheel, int ysign ) const;

    // T must have methods .FlipNormal() and .Transform( Matrix4d )
    template <typename T>
    void TireToBogie( const T &source, vector<T> &dest, int isymm, int suspensionmode, double bogietheta ) const
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

        pivot.translatev( GetPivotPoint( isymm, suspensionmode ) );

        pivot.rotateY( -bogietheta );

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
    void TireToBogie( const T &source, vector<T> &dest ) const
    {
        int nsymm = 1;
        if ( m_Symmetrical() )
        {
            nsymm = 2;
        }

        double bogietheta = 0;
        if ( !m_DrawNominal() )
        {
            bogietheta = m_BogieTheta();
        }

        for ( int isymm = 0; isymm < nsymm; isymm++ )
        {
            TireToBogie( source, dest, isymm, vsp::GEAR_SUSPENSION_NOMINAL, bogietheta );
        }
    }

    void AppendMainSurf( vector < VspSurf > &surfvec ) const;

    bool m_Visible;

    VspCurve m_TireProfile;
    VspSurf m_TireSurface;

    DrawObj m_SuspensionTravelLinesDO;
    DrawObj m_SuspensionTravelPointsDO;


    // Bogie
    BoolParm m_Symmetrical;

    BoolParm m_DrawNominal;

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

    Parm m_Travel;
    Parm m_TravelCompressed;
    Parm m_TravelExtended;

    Parm m_BogieTheta;
    Parm m_BogieThetaMax;
    Parm m_BogieThetaMin;

    Parm m_SteeringAngle;


    // Tire
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
    virtual void UpdateSurf();
    virtual void UpdateMainTessVec();
    virtual void UpdateTessVec();
    virtual void UpdateMainDegenGeomPreview();
    virtual void UpdateDrawObj();
    virtual void LoadDrawObjs( vector< DrawObj* > & draw_obj_vec );

    std::vector < Bogie * > m_Bogies;
    std::vector < int > m_BogieMainSurfIndex;
    int m_CurrBogieIndex;

    vector < vec3d > m_MainNominalCGPointVec;
    vec3d m_MainMinCGPoint;
    vec3d m_MainMaxCGPoint;

    vector < vec3d > m_NominalCGPointVec;
    vector < SimpleFeatureTess > m_LimitsCGPointVec;

    DrawObj m_CGNominalDrawObj;
    DrawObj m_CGLimitsDrawObj;

};


#endif // !defined(VSPGEARGEOM__INCLUDED_)
