//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//

// WingGeom.h:
// J.R Gloudemans
//
//////////////////////////////////////////////////////////////////////

#if !defined(VSPWINGGEOM__INCLUDED_)
#define VSPWINGGEOM__INCLUDED_

#include "Defines.h"
#include "Geom.h"
#include "StackGeom.h"
#include "XSec.h"
#include "XSecSurf.h"

typedef eli::geom::curve::piecewise_general_creator< double, 3, surface_tolerance_type > piecewise_general_curve_creator_type;
typedef piecewise_general_curve_creator_type::joint_data joint_data_type;

double CalcTanSweepAt( double loc, double sweep, double baseloc, double aspect, double taper );

//==== Wing Driver Group ====//
class WingDriverGroup : public DriverGroup
{
public:

    WingDriverGroup();

    virtual void UpdateGroup( vector< string > parmIDs );
    virtual bool ValidDrivers( vector< int > choices );
};

class BlendWingSect : public XSec
{
public:
    BlendWingSect( XSecCurve *xsc );

    virtual void ValidateParms( bool first, bool last );

    Parm m_InLESweep;
    Parm m_InTESweep;
    Parm m_InLEDihedral;
    Parm m_InTEDihedral;
    Parm m_InLEStrength;
    Parm m_InTEStrength;

    Parm m_OutLESweep;
    Parm m_OutTESweep;
    Parm m_OutLEDihedral;
    Parm m_OutTEDihedral;
    Parm m_OutLEStrength;
    Parm m_OutTEStrength;

    IntParm m_InLEMode;
    IntParm m_OutLEMode;
    IntParm m_InTEMode;
    IntParm m_OutTEMode;
};

//==== Wing Section ====//
class WingSect : public BlendWingSect
{
public:

    WingSect( XSecCurve *xsc );

    virtual void SetScale( double scale );
    virtual void Update();
    virtual void UpdateFromWing();
    virtual void CopyBasePos( XSec* xs );
    virtual vector< string > GetDriverParms();
    virtual void ForceChordVal( double val, bool root_chord_flag );
    virtual void ForceSpanRcTc( double span, double rc, double tc );
    virtual void ForceAspectTaperArea( double aspect, double taper, double area );
    virtual double GetTanSweepAt( double sweep, double loc  );

    virtual xmlNodePtr EncodeXml( xmlNodePtr & node );
    virtual xmlNodePtr DecodeXml( xmlNodePtr & node );

    virtual void ReadV2File( xmlNodePtr &root );

    virtual void GetJoints( bool first, bool last,
                            joint_data_type &te_joint, const curve_point_type &te_point, const curve_point_type &te_in, const curve_point_type &te_out,
                            const double &te_in_str, const double &te_out_str,
                            joint_data_type &le_joint, const curve_point_type &le_point, const curve_point_type &le_in, const curve_point_type &le_out,
                            const double &le_in_str, const double &le_out_str );

    virtual void GetPoints( curve_point_type &te_point, curve_point_type &le_point );

    virtual void SetUnsetParms( int irib, const VspSurf &surf, const double &te_in_str, const double &te_out_str, const double &le_in_str, const double &le_out_str );

    WingDriverGroup m_DriverGroup;

    Parm m_Aspect;
    Parm m_Taper;
    Parm m_Area;
    Parm m_Span;
    Parm m_AvgChord;
    Parm m_TipChord;
    Parm m_RootChord;

    Parm m_RootCluster;
    Parm m_TipCluster;

    Parm m_Sweep;
    NotEqParm m_SweepLoc;
    Parm m_SecSweep;
    NotEqParm m_SecSweepLoc;

    Parm m_Twist;
    Parm m_TwistLoc;

    Parm m_Dihedral;
    BoolParm m_RotateMatchDiedralFlag;

    Parm m_ProjectedSpan;

    double m_XDelta;
    double m_YDelta;
    double m_ZDelta;

    double m_XRotate;
    double m_YRotate;
    double m_ZRotate;

    double m_XCenterRot;
    double m_YCenterRot;
    double m_ZCenterRot;

    double m_ThickScale;

protected:

    enum { V2_AR_TR_A = 0,
           V2_AR_TR_S = 1,
           V2_AR_TR_TC = 2,
           V2_AR_TR_RC = 3,
           V2_S_TC_RC = 4,
           V2_A_TC_RC = 5,
           V2_TR_S_A = 6
    };

};

//==== Wing Geom ====//
class WingGeom : public GeomXSec
{
public:

    WingGeom( Vehicle* vehicle_ptr );
    virtual ~WingGeom();

    virtual void Update( bool fullupdate = true );

    virtual void ComputeCenter();

    virtual xmlNodePtr EncodeXml( xmlNodePtr & node );
    virtual xmlNodePtr DecodeXml( xmlNodePtr & node );

    virtual int NumXSec()                                { return m_XSecSurf.NumXSec(); }

    virtual int GetNumXSecSurfs() const                       { return 1; }
    virtual XSecSurf* GetXSecSurf( int index )            { return &m_XSecSurf; }


    bool IsClosed() const;

    virtual void AddLinkableParms( vector< string > & linkable_parm_vec, const string & link_container_id = string() );
    virtual void Scale();
    virtual void AddDefaultSources( double base_len = 1.0 );

    //==== Override Geom Cut/Copy/Paste/Insert ====//
    virtual void CutXSec( int index );
    virtual void CopyXSec( int index );
    virtual void PasteXSec( int index );
    virtual void InsertXSec( int index, int type );

    virtual WingSect* GetWingSect( int index );
    virtual void SplitWingSect( int index );
    virtual void CutWingSect( int index );
    virtual void CopyWingSect( int index );
    virtual void PasteWingSect( int index );
    virtual void InsertWingSect( int index );

    virtual void CopyAirfoil( int index );
    virtual void PasteAirfoil( int index );

    virtual void LoadDrawObjs( vector< DrawObj* > & draw_obj_vec );

    virtual void ReadV2File( xmlNodePtr &root );
    virtual void OffsetXSecs( double off );

    //==== Planform Parms ====//
    Parm m_TotalSpan;
    Parm m_TotalProjSpan;
    Parm m_TotalChord;
    Parm m_TotalArea;
    Parm m_TotalAR;

    Parm m_LECluster;
    Parm m_TECluster;

    Parm m_SmallPanelW;
    Parm m_MaxGrowth;

    BoolParm m_RelativeDihedralFlag;
    BoolParm m_RelativeTwistFlag;
    BoolParm m_RotateAirfoilMatchDiedralFlag;
    BoolParm m_CorrectAirfoilThicknessFlag;

    IntParm m_ActiveWingSection;

    enum { V2_NACA_4_SERIES = 1,
           V2_BICONVEX = 2,
           V2_WEDGE = 3,
           V2_AIRFOIL_FILE = 4,
           V2_NACA_6_SERIES = 5
    };


protected:

    virtual void ChangeID( string id );
    virtual void UpdateSurf();
    virtual void UpdateTesselate( const vector<VspSurf> &surf_vec, int indx, vector< vector< vec3d > > &pnts, vector< vector< vec3d > > &norms, vector< vector< vec3d > > &uw_pnts, bool degen ) const;
    virtual void UpdateSplitTesselate( const vector<VspSurf> &surf_vec, int indx, vector< vector< vector< vec3d > > > &pnts, vector< vector< vector< vec3d > > > &norms ) const;
    virtual void UpdatePreTess();
    virtual void UpdateDrawObj();
    virtual void UpdateHighlightDrawObj();
    virtual void MatchWingSections();

    virtual void CalculateMeshMetrics();

    virtual double ComputeTotalSpan();
    virtual double ComputeTotalProjSpan();
    virtual double ComputeTotalChord();
    virtual double ComputeTotalArea();
    virtual void UpdateTotalParameters();
    virtual void UpdateTotalSpan();
    virtual void UpdateTotalProjSpan();
    virtual void UpdateTotalChord();
    virtual void UpdateTotalArea();

    virtual void SetTempActiveXSec();

    virtual vector< WingSect* > GetWingSectVec();

    virtual double GetSumDihedral( int sect_id );

    DrawObj m_HighlightWingSecDrawObj;

    vector<int> m_TessUVec;
    vector<int> m_UMergeVec;

    vector < double > m_RootClusterVec;
    vector < double > m_TipClusterVec;

    bool m_Closed;

    VspSurf m_FoilSurf;

};
#endif // !defined(VSPWINGGEOM__INCLUDED_)
