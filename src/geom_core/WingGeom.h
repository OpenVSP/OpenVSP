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

#include "Geom.h"
#include "StackGeom.h"
#include "XSec.h"
#include "XSecSurf.h"

double CalcTanSweepAt( double loc, double sweep, double baseloc, double aspect, double taper );

//==== Wing Driver Group ====//
class WingDriverGroup : public DriverGroup
{
public:

    WingDriverGroup();

    virtual void UpdateGroup( vector< string > parmIDs );
    virtual bool ValidDrivers( vector< int > choices );
};

//==== Wing Section ====//
class WingSect : public XSec
{
public:

    WingSect( XSecCurve *xsc );

    virtual void SetScale( double scale );
    virtual void Update();
    virtual void CopyBasePos( XSec* xs );
    virtual vector< string > GetDriverParms();
    virtual void ForceChordVal( double val, bool root_chord_flag );
    virtual void ForceSpanRcTc( double span, double rc, double tc );
    virtual void ForceAspectTaperArea( double aspect, double taper, double area );
    virtual double GetProjectedSpan()                                { return m_ProjectedSpan; }
    virtual void SetProjectedSpan( double v )                        { m_ProjectedSpan = v; }
    virtual double GetTanSweepAt( double sweep, double loc  );

    virtual xmlNodePtr EncodeXml( xmlNodePtr & node );
    virtual xmlNodePtr DecodeXml( xmlNodePtr & node );

    WingDriverGroup m_DriverGroup;

    Parm m_Aspect;
    Parm m_Taper;
    Parm m_Area;
    Parm m_Span;
    Parm m_AvgChord;
    Parm m_TipChord;
    Parm m_RootChord;

    Parm m_Sweep;
    NotEqParm m_SweepLoc;
    Parm m_SecSweep;
    NotEqParm m_SecSweepLoc;

    Parm m_Twist;
    Parm m_TwistLoc;

    Parm m_Dihedral;


    double m_XDelta;
    double m_YDelta;
    double m_ZDelta;

    double m_XRotate;
    double m_YRotate;
    double m_ZRotate;

    double m_XCenterRot;
    double m_YCenterRot;
    double m_ZCenterRot;


protected:

    double m_ProjectedSpan;



};

//==== Wing Geom ====//
class WingGeom : public GeomXSec
{
public:

    WingGeom( Vehicle* vehicle_ptr );
    virtual ~WingGeom();

    virtual void ComputeCenter();

    virtual xmlNodePtr EncodeXml( xmlNodePtr & node );
    virtual xmlNodePtr DecodeXml( xmlNodePtr & node );

    virtual int NumXSec()                                { return m_XSecSurf.NumXSec(); }

    virtual int GetActiveAirfoilIndex()                    { return m_ActiveAirfoil; }
    virtual void SetActiveAirfoilIndex( int index );

    virtual void SetActiveAirfoilType( int type );

    virtual int GetNumXSecSurfs()                        { return 1; }
    virtual XSecSurf* GetXSecSurf( int index )            { return &m_XSecSurf; }


    bool IsClosed() const;

    virtual void AddLinkableParms( vector< string > & linkable_parm_vec, const string & link_container_id = string() );
    virtual void Scale();
    virtual void AddDefaultSources( double base_len = 1.0 );

    //==== Set Drag Factors ====//
    virtual void LoadDragFactors( DragFactors& drag_factors );

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

    //==== Planform Parms ====//
    Parm m_TotalSpan;
    Parm m_TotalProjSpan;
    Parm m_TotalChord;
    Parm m_TotalArea;
    double m_TotalAspectRatio;

    BoolParm m_RelativeDihedralFlag;
    BoolParm m_RelativeTwistFlag;
    BoolParm m_RotateAirfoilMatchDiedralFlag;

protected:

    virtual void ChangeID( string id );
    virtual void UpdateSurf();
    virtual void UpdateTesselate( int indx, vector< vector< vec3d > > &pnts, vector< vector< vec3d > > &norms, vector< vector< vec3d > > &uw_pnts, bool degen );
    virtual void UpdateDrawObj();
    virtual void MatchWingSections();

    virtual double ComputeTotalSpan();
    virtual double ComputeTotalProjSpan();
    virtual double ComputeTotalChord();
    virtual double ComputeTotalArea();
    virtual void UpdateTotalSpan();
    virtual void UpdateTotalProjSpan();
    virtual void UpdateTotalChord();
    virtual void UpdateTotalArea();

    virtual void SetTempActiveXSec();

    virtual vector< WingSect* > GetWingSectVec();

    virtual double GetSumDihedral( int sect_id );

    int m_ActiveAirfoil;
    DrawObj m_HighlightWingSecDrawObj;

    vector<int> m_TessUVec;

    bool m_Closed;


};
#endif // !defined(VSPWINGGEOM__INCLUDED_)
