//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//

// PropGeom.h:
// Rob McDonald
//
//////////////////////////////////////////////////////////////////////

#if !defined(VSPPROPGEOM__INCLUDED_)
#define VSPPROPGEOM__INCLUDED_


#include "Geom.h"
#include "GeomInterface.h"
#include "XSec.h"
#include "XSecCurve.h"
#include "XSecSurf.h"
#include "PCurve.h"


class PropPositioner
{
public:

    PropPositioner();

    virtual void Update();

    virtual void SetCurve( const VspCurve &c );
    virtual VspCurve GetCurve();

    XSecSurf * m_ParentProp;

    double m_Chord;

    double m_Construct;
    double m_FeatherOffset;
    double m_FeatherAxis;
    double m_RootChord;
    double m_RootTwist;

    double m_Twist;
    double m_ZRotate;

    bool m_CurveSection;

    double m_Radius;
    double m_Rake;
    double m_Skew;
    double m_Sweep;
    double m_Axial;
    double m_Tangential;

    double m_Reverse;

protected:

    bool m_NeedsUpdate;

    VspCurve m_Curve;
    VspCurve m_TransformedCurve;

};

class PropXSec : public XSec
{
public:
    PropXSec( XSecCurve *xsc );

    virtual void Update();

    virtual void SetRefLength( double len );

    virtual void CopyBasePos( XSec* xs );

    virtual void GetDrawLines( Matrix4d &transMat, vector< vec3d > &lines );
    PropPositioner m_PropPos;

    FractionParm m_RadiusFrac;

    Parm m_RefLength;

protected:

    double m_RefLenVal;
};

//==== A Geom that turns blades about an axis ====//
// Implemented by PropGeom and by a Clone standing in for one.  What an auxiliary geom needs
// for a tip path or a burst.
class RotorRole : virtual public GeomInterface
{
public:
    // The behavior type this role belongs to; checked by Geom::CastTo.
    static int BehaviorType()   { return PROP_GEOM_TYPE; }

    virtual double GetRotorDiameter() const = 0;
    virtual double GetRotorR0() const = 0;

    // The flap hinge's radial place, fraction of radius.
    virtual double GetRotorFlapRadiusFract() const = 0;

    // Which way the blades turn.
    virtual bool GetRotorReverseFlag() const = 0;

    // The hub, if this rotor describes one; false otherwise.
    virtual bool GetRotorHubDiameter( double &hubdia ) const = 0;

    // The angle the blades are turned to.
    virtual double GetRotorRotate() const = 0;
    virtual void SetRotorRotate( double rotate ) = 0;

    // The main surfaces, their tessellation and their preview with the blades turned to rotate.
    virtual void GetMainSurfVecAtRotate( double rotate, vector< VspSurf > &surfs ) = 0;
    virtual void GetMainTessVecAtRotate( double rotate, vector< SimpleTess > &tess, vector< SimpleFeatureTess > &ftess ) = 0;
    virtual void GetMainDegenGeomPreviewAtRotate( double rotate, vector< DegenGeom > &degen ) = 0;

    // The blades placed as main surfaces, and blade iblade's placement at rotate: from the blade as
    // built to where it sits, before the Geom's own placement.
    virtual int NumPlacedBlades() = 0;
    virtual Matrix4d GetBladePlacement( int iblade, double rotate ) = 0;
};

//==== A hinge between a propeller's hub and blade ====//
struct BladeHinge
{
    double m_Radius;    // Radial place, fraction of radius.
    int m_Type;         // One of PropGeom's hinge types.
    vec3d m_Origin;     // A point on the hinge line, all hinges undeflected.
    vec3d m_Axis;
    double m_Angle;     // Turns right-handed about m_Axis, as Matrix4d::rotateAbout does.
};

//==== Propeller Geom ====//
class PropGeom : public GeomXSec, public RotorRole
{
public:
    PropGeom( Vehicle* vehicle_ptr );
    virtual ~PropGeom();

    virtual void UpdateDrawObj();
    virtual void UpdateHighlightDrawObj();
    virtual void LoadDrawObjs( vector< DrawObj* > & draw_obj_vec );
    virtual void BuildMarkerDrawObjs( Geom* placer, vector< DrawObj > &marker_vec );
    virtual void SetMarkerVisibility( Geom* placer, vector< DrawObj > &marker_vec );

    virtual void ComputeCenter();

    // Blade hinge types, in their order where hinges share a radial place.
    enum
    {
        FLAP_HINGE,
        LAG_HINGE,
        FOLD_HINGE,
        FEATHER_HINGE,
        NUM_HINGE_TYPES
    };

    // The DrawObj colour each hinge type's marker is drawn in.  Shared with the Motion tab, which
    // keys its dividers with the same answer.
    static int HingeDrawColor( int type );

    virtual xmlNodePtr EncodeXml( xmlNodePtr & node );
    virtual xmlNodePtr DecodeXml( xmlNodePtr & node );

    virtual int NumXSec()
    {
        return m_XSecSurf.NumXSec();
    }

    //==== Override Geom Cut/Copy/Paste/Insert ====//
    virtual void CutXSec( int index );
    virtual void CopyXSec( int index );
    virtual void PasteXSec( int index );
    virtual void InsertXSec( int index, int type );

    virtual void CutActiveXSec();
    virtual void CopyActiveXSec();
    virtual void PasteActiveXSec();
    virtual void InsertXSec( );
    virtual void InsertXSec( int type );

    virtual int GetNumXSecSurfs() const
    {
        return 1;
    }
    virtual XSecSurf* GetXSecSurf( int index )
    {
        return &m_XSecSurf;
    }

    virtual void AddLinkableParms( vector< string > & linkable_parm_vec, const string & link_container_id = string() );
    virtual void ApplyScale( double currentScale );
    virtual void AddDefaultSources( double base_len = 1.0 );

    virtual string BuildBEMResults();

    virtual int ReadBEM( const string &file_name );

    virtual PCurve* GetPCurve( int curveid );

    virtual void WriteAirfoilFiles( FILE* meta_fid, const string &name = string(), const string &id = string() );

    virtual void ApproxCubicAllPCurves();

    virtual void ResetThickness();

    // Replace the sections, blade curves, pitch and rotation with a starting design (PROP_PRESETS).
    virtual void InitPreset( int preset );

    virtual double GetR0();
    virtual double ComputeMAC();

    virtual double GetRotorDiameter() const
    {
        return m_Diameter();
    }
    virtual double GetRotorR0() const
    {
        return const_cast< PropGeom* >( this )->GetR0();
    }
    virtual double GetRotorFlapRadiusFract() const
    {
        return m_RadFlapAxis();
    }
    virtual bool GetRotorReverseFlag() const
    {
        return m_ReverseFlag();
    }
    virtual bool GetRotorHubDiameter( double &hubdia ) const;
    virtual double GetRotorRotate() const
    {
        return m_Rotate();
    }
    virtual void SetRotorRotate( double rotate )
    {
        m_Rotate.Set( rotate );
    }
    virtual void GetMainSurfVecAtRotate( double rotate, vector< VspSurf > &surfs );
    virtual void GetMainTessVecAtRotate( double rotate, vector< SimpleTess > &tess, vector< SimpleFeatureTess > &ftess );
    virtual void GetMainDegenGeomPreviewAtRotate( double rotate, vector< DegenGeom > &degen );
    virtual int NumPlacedBlades();
    virtual Matrix4d GetBladePlacement( int iblade, double rotate );
    virtual double UtoEta( const double &u, bool ignoreCap = false );
    virtual int FeatherMode();
    virtual double EtatoU( const double &eta, bool ignoreCap = false );

    Parm m_Diameter;

    Parm m_Rotate;
    Parm m_Beta34;
    Parm m_Feather;
    IntParm m_UseBeta34Flag;
    BoolParm m_CylindricalSectionsFlag;

    Parm m_Precone;

    Parm m_Construct;

    Parm m_FeatherOffset;
    Parm m_FeatherAxis;
    Parm m_RadFeatherAxis;

    BoolParm m_ReverseFlag;

    Parm m_RadFlapAxis;
    Parm m_AxialFlapAxis;
    Parm m_OffsetFlapAxis;
    Parm m_AzimuthFlapDir;
    Parm m_ElevationFlapDir;
    Parm m_FlapAngle;

    vector< Parm* > m_FlapAngleParmVec;
    Parm m_FlapMean;
    vector< Parm* > m_FlapCosParmVec;
    vector< Parm* > m_FlapSinParmVec;

    Parm m_RadLagAxis;
    Parm m_AxialLagAxis;
    Parm m_OffsetLagAxis;
    Parm m_AzimuthLagDir;
    Parm m_CantLagDir;
    Parm m_LagAngle;

    vector< Parm* > m_LagAngleParmVec;
    Parm m_LagMean;
    vector< Parm* > m_LagCosParmVec;
    vector< Parm* > m_LagSinParmVec;

    Parm m_RadFoldAxis;
    Parm m_AxialFoldAxis;
    Parm m_OffsetFoldAxis;
    Parm m_AzimuthFoldDir;
    Parm m_ElevationFoldDir;
    Parm m_FoldAngle;

    BoolParm m_IndividualBladeFoldFlag;
    vector< Parm* > m_FoldAngleParmVec;

    vector< Parm* > m_FeatherParmVec;
    Parm m_FeatherMean;
    vector< Parm* > m_FeatherCosParmVec;
    vector< Parm* > m_FeatherSinParmVec;
    IntParm m_ActiveBlade;
    Parm m_PsiRef;

    // How the blades take the flap, lag and feather angles, and the harmonics of the harmonic mode.
    IntParm m_BladeMotionMode;
    IntParm m_NumHarmonics;

    vec3d m_FlapAxOrigin;
    vec3d m_FlapAxDirection;

    vec3d m_LagAxOrigin;
    vec3d m_LagAxDirection;

    vec3d m_FoldAxOrigin;
    vec3d m_FoldAxDirection;

    vec3d m_FeatherAxOrigin;
    vec3d m_FeatherAxDirection;

    IntParm m_Nblade;

    vector< Parm* > m_BladeAzimuthParmVec;
    vector< Parm* > m_BladeDeltaAzimuthParmVec;
    IntParm m_BladeAzimuthMode;
    BoolParm m_BladeAzimuthDeltaFlag;
    Parm m_BalanceX1;
    Parm m_BalanceX2;

    BoolParm m_TipMarkerScaleFlag;
    Parm m_TipMarkerScale;

    IntParm m_PropMode;

    PCurve m_ChordCurve;

    PCurve m_TwistCurve;

    PCurve m_RakeCurve;
    PCurve m_SkewCurve;
    PCurve m_SweepCurve;
    PCurve m_AxialCurve;
    PCurve m_TangentialCurve;
    PCurve m_ThickCurve;
    PCurve m_CLICurve;

    vector < PCurve* > m_pcurve_vec;

    Parm m_LECluster;
    Parm m_TECluster;
    Parm m_RootCluster;
    Parm m_TipCluster;

    Parm m_SmallPanelW;
    Parm m_MaxGrowth;

    Parm m_AFLimit;
    Parm m_AF;
    Parm m_CLi;
    Parm m_Solidity;
    Parm m_TSolidity;
    Parm m_PSolidity;
    Parm m_Chord;
    Parm m_TChord;
    Parm m_PChord;

protected:
    virtual void ChangeID( const string &id );

    virtual void UpdateSurf();
    virtual void UpdateEndCaps();
    virtual void UpdateFeatureLines();
    virtual void UpdateLCurve();
    virtual void UpdateFlags();
    virtual void UpdatePostSurf();
    virtual void SetDirtyFlags( Parm* parm_ptr );
    virtual bool MovesBladesOnly( Parm* parm_ptr );
    virtual void UpdateMainTessVec();
    virtual void UpdateMainDegenGeomPreview();
    virtual void UpdateBladeAzimuth();

    virtual void BalanceBlades( vector < double > & thetavec );
    virtual void BalanceBladesAnalytic( vector < double > & thetavec );
    // virtual void BalanceBladesFourier( std::vector<double>& thetavec, const std::vector<int>& fixed_idx, int M );

    virtual void CheckBalance();

    virtual double GetBladeFlapAngle( int iblade, double rotate );
    virtual double GetBladeLagAngle( int iblade, double rotate );
    virtual double GetBladeFoldAngle( int iblade );
    virtual double GetBladeFeather( int iblade, double rotate );
    virtual double GetBladePsi( int iblade, double rotate );
    virtual void UpdateBladeMotion( int mode, Parm & angle, vector< Parm* > & bladevec, Parm & mean,
                                    vector< Parm* > & cosvec, vector< Parm* > & sinvec );
    // Each places blade iblade, counting from zero, as it sits with the propeller turned to rotate.
    virtual void BladeRootMotion( Matrix4d & mat, int iblade, double rotate );
    virtual vector< BladeHinge > GetBladeHinges( int iblade, double rotate );
    virtual void RigidBladeMotion( Matrix4d & mat, int iblade, double rotate );

    virtual void EnforceOrder( PropXSec* xs, int indx );
    virtual void EnforcePCurveOrder( double rfirst, double rlast );

    virtual void GetUWTess( const VspSurf &surf, bool capUMinSuccess, bool capUMaxSuccess, bool degen, vector< double > &utess, vector< double > &vtess, const int & n_ref = 0 ) const;
    virtual void UpdateTesselate( const VspSurf &surf, bool capUMinSuccess, bool capUMaxSuccess, bool degen, vector< vector< vec3d > > &pnts, vector< vector< vec3d > > &norms, vector< vector< vec3d > > &uw_pnts, const int & n_ref = 0 ) const;
    virtual void UpdateSplitTesselate( const VspSurf &surf, bool capUMinSuccess, bool capUMaxSuccess, vector< vector< vector< vec3d > > > &pnts, vector< vector< vector< vec3d > > > &norms ) const;
    virtual void UpdatePreTess();

    virtual void ReserveBlades( int n );
    virtual void AddBlade();
    virtual void ReserveHarmonics();
    virtual void ReserveHarmonicParms( vector< Parm* > & cosvec, vector< Parm* > & sinvec, const string & name,
                                       const string & angle, int n );
    virtual void AddBladeParm( vector< Parm* > & vec, const string & name, const string & group, int num, double val, double lower, double upper, const string & descript );

    virtual void CalculateMeshMetrics();

    // The thrust and rotation markers, then the lines and heads of each hinge type in turn.
    enum { PROP_MARKER_LINES, PROP_MARKER_HEADS, PROP_MARKER_HINGES,
           NUM_PROP_MARKERS = PROP_MARKER_HINGES + 2 * NUM_HINGE_TYPES };
    BndBox m_MainBladeBBox;
    DrawObj m_HighlightBladeDrawObj;

    Vsp1DCurve m_rtou;
    VspSurf m_FoilSurf;
    // The one blade and the disk.  UpdateSurf builds them and the finishing passes work on them;
    // UpdatePostSurf copies them into the main surfaces.
    VspSurf m_BladeSurf;
    VspSurf m_DiskSurf;
    bool m_BladeCapUMinSuccess;
    bool m_BladeCapUMaxSuccess;

    // Twist at 3/4 of radius, or zero for a blade that starts outboard of it.
    double m_Twist34;
    double m_BladeRootChord;

    // Tessellation of the single computed blade and of the disk, kept across updates so the per-blade
    // duplicate tessellations can be assigned into existing buffers rather than cleared and reallocated.
    SimpleTess m_BladeTess;
    SimpleFeatureTess m_BladeFeatureTess;
    SimpleTess m_DiskTess;
    SimpleFeatureTess m_DiskFeatureTess;
    bool m_BladeTessDirty;

    // Preview of the single computed blade, kept for the same reason.
    DegenGeom m_BladeDegenGeom;
    bool m_BladeDegenDirty;

    vector < double > m_UPseudo;

};
#endif // !defined(VSPPROPGEOM__INCLUDED_)
