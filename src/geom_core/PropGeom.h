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

#include "Defines.h"
#include "Geom.h"
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

    double m_Precone;

    double m_Twist;
    double m_PropRot;
    double m_Feather;
    double m_ZRotate;

    bool m_CurveSection;

    double m_Radius;
    double m_Rake;
    double m_Skew;
    double m_Sweep;
    double m_Axial;
    double m_Tangential;

    vec3d m_FoldOrigin;
    vec3d m_FoldDirection;
    double m_FoldAngle;

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

    virtual vector< vec3d > GetDrawLines( Matrix4d &transMat );
    PropPositioner m_PropPos;

    FractionParm m_RadiusFrac;

    Parm m_RefLength;

protected:

    double m_RefLenVal;
};

//==== Propeller Geom ====//
class PropGeom : public GeomXSec
{
public:
    PropGeom( Vehicle* vehicle_ptr );
    virtual ~PropGeom();

    virtual void UpdateDrawObj();
    virtual void LoadDrawObjs( vector< DrawObj* > & draw_obj_vec );

    virtual void ComputeCenter();

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
    virtual void Scale();
    virtual void AddDefaultSources( double base_len = 1.0 );

    virtual string BuildBEMResults();

    virtual int ReadBEM( const string &file_name );

    virtual PCurve* GetPCurve( int curveid );

    virtual void WriteAirfoilFiles( FILE* meta_fid );

    virtual vector< TMesh* > CreateTMeshVec() const;

    virtual void SetExportMainSurf( bool b )         { m_ExportMainSurf = b; }

    virtual const vector<VspSurf> & GetSurfVecConstRef() const;

    virtual void ApproxCubicAllPCurves();

    virtual void ResetThickness();

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

    BoolParm m_ReverseFlag;

    Parm m_RadFoldAxis;
    Parm m_AxialFoldAxis;
    Parm m_OffsetFoldAxis;
    Parm m_AzimuthFoldDir;
    Parm m_ElevationFoldDir;
    Parm m_FoldAngle;

    vec3d m_FoldAxOrigin;
    vec3d m_FoldAxDirection;

    IntParm m_Nblade;
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
    virtual void ChangeID( string id );

    virtual void UpdateSurf();
    virtual void UpdateMainTessVec();

    virtual void EnforceOrder( PropXSec* xs, int indx );
    virtual void EnforcePCurveOrder( double rfirst, double rlast );

    virtual void UpdateTesselate( const vector<VspSurf> &surf_vec, int indx, vector< vector< vec3d > > &pnts, vector< vector< vec3d > > &norms, vector< vector< vec3d > > &uw_pnts, bool degen ) const;
    virtual void UpdateSplitTesselate( const vector<VspSurf> &surf_vec, int indx, vector< vector< vector< vec3d > > > &pnts, vector< vector< vector< vec3d > > > &norms ) const;
    virtual void UpdatePreTess();

    virtual void CalculateMeshMetrics();

    DrawObj m_ArrowLinesDO;
    DrawObj m_ArrowHeadDO;

    Vsp1DCurve m_rtou;
    VspSurf m_FoilSurf;

    bool m_ExportMainSurf;

    vector < double > m_UPseudo;

};
#endif // !defined(VSPPROPGEOM__INCLUDED_)
