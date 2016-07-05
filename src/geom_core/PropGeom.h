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

    double m_Twist;
    double m_PropRot;
    double m_Feather;
    double m_XRotate;
    double m_ZRotate;

    double m_Radius;
    double m_Rake;
    double m_Skew;

    vec3d m_FoldOrigin;
    vec3d m_FoldDirection;
    double m_FoldAngle;

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

    virtual void SetActiveXSecType( int type );
    virtual void CutActiveXSec();
    virtual void CopyActiveXSec();
    virtual void PasteActiveXSec();
    virtual void InsertXSec( );
    virtual void InsertXSec( int type );

    virtual int GetNumXSecSurfs()
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

    //==== Set Drag Factors ====//
    virtual void LoadDragFactors( DragFactors& drag_factors );

    Parm m_Diameter;

    Parm m_Rotate;
    Parm m_Beta34;
    Parm m_Feather;
    IntParm m_UseBeta34Flag;

    Parm m_RadFoldAxis;
    Parm m_AxialFoldAxis;
    Parm m_OffsetFoldAxis;
    Parm m_AzimuthFoldDir;
    Parm m_ElevationFoldDir;
    Parm m_FoldAngle;

    vec3d m_FoldAxOrigin;
    vec3d m_FoldAxDirection;

    IntParm m_Nblade;

    PCurve m_ChordCurve;

    PCurve m_TwistCurve;

    PCurve m_XRotateCurve;
    PCurve m_ZRotateCurve;

    PCurve m_RakeCurve;
    PCurve m_SkewCurve;

    vector < PCurve* > m_pcurve_vec;

    Parm m_LECluster;
    Parm m_TECluster;
    Parm m_RootCluster;
    Parm m_TipCluster;

    Parm m_AFLimit;
    Parm m_AF;

protected:
    virtual void ChangeID( string id );

    virtual void UpdateSurf();

    virtual void EnforceOrder( PropXSec* xs, int indx );
    virtual void EnforcePCurveOrder( double rfirst, double rlast );

    virtual void UpdateTesselate( int indx, vector< vector< vec3d > > &pnts, vector< vector< vec3d > > &norms, vector< vector< vec3d > > &uw_pnts, bool degen );
    virtual void UpdateSplitTesselate( int indx, vector< vector< vector< vec3d > > > &pnts, vector< vector< vector< vec3d > > > &norms );

};
#endif // !defined(VSPPROPGEOM__INCLUDED_)
