//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//

// StackGeom.h:
// J.R Gloudemans
//
//////////////////////////////////////////////////////////////////////

#if !defined(VSPSTACKGEOM__INCLUDED_)
#define VSPSTACKGEOM__INCLUDED_

#include "GeomEngine.h"
#include "XSec.h"
#include "XSecCurve.h"
#include "XSecSurf.h"

enum STACK_PRESETS {
    STACK_PRESET_DEFAULT=0,              ///< standard init type
    STACK_PRESET_CYLINDER_ENDPTS=1,      ///< init as a cylinder with points as first/last xsec
    STACK_PRESET_CYLINDER_ENDCAPS=2,     ///< init as a cylinder with endcaps instead of points
    STACK_PRESET_FLOWTHRU_OUTLIP_ORIG=3, ///< init as a flowthru nacelle starting/ending with outlet lip
    STACK_PRESET_FLOWTHRU_INLIP_ORIG=4,  ///< init as a flowthru nacelle starting/ending with inlet lip
    STACK_PRESET_FLOWTHRU_MID_ORIG=5,    ///< init as a flowthru nacelle starting/ending with outlet face
    STACK_PRESET_BOTHFACEFACE=6,         ///< init as a nacelle modeled from the inlet face around the outside to the outlet face
    STACK_PRESET_BOTHLIPFACE=7,          ///< init as a nacelle modeled from the inlet lip around the outside to the outlet face
    STACK_PRESET_BOTHFACELIP=8,          ///< init as a nacelle modeled from the inlet face around the outside to the outlet lip
    STACK_PRESET_BOTHLIPLIP=9,           ///< init as a nacelle modeled from the inlet lip around the outside to the outlet lip
    STACK_PRESET_OUTFACE=10,             ///< init as a nacelle modeled with no inlet, around the outside to the outlet face
    STACK_PRESET_OUTLIP=11,              ///< init as a nacelle modeled from no inlet, around the outside to the outlet face
    STACK_PRESET_INFACE=12,              ///< init as a nacelle modeled from inlet face, then close out with no outlet
    STACK_PRESET_INLIP=13,               ///< init as a nacelle modeled from inlet lip, then close out with no outlet
    STACK_PRESET_FLOWPATH_BOTH=14,       ///< init as a nacelle's flowpath only, modeling from inlet to outlet
    STACK_PRESET_FLOWPATH_IN=15,         ///< init as a nacelle's flowpath only, modeling inlet lip to face only
    STACK_PRESET_FLOWPATH_OUT=16,        ///< init as a nacelle's flowpath only, modeling outlet face to lip only

  };

//==== Stack Geom ====//
class StackGeom : public GeomEngine
{
public:
    enum {STACK_FREE, STACK_LOOP, NUM_STACK_POLICY};

    StackGeom( Vehicle* vehicle_ptr );
    virtual ~StackGeom();

    void InitXSecs( vector < StackXSec* > stack_xs );
    void InitParms( int stack_type );

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

    bool IsClosed() const;

    virtual void AddLinkableParms( vector< string > & linkable_parm_vec, const string & link_container_id = string() );
    virtual void Scale();
    virtual void AddDefaultSources( double base_len = 1.0 );

    IntParm m_OrderPolicy;

protected:
    virtual void ChangeID( const string &id );

    virtual void UpdateSurf();
    virtual void GetUWTess( const VspSurf &surf, bool capUMinSuccess, bool capUMaxSuccess, bool degen, vector< double > &utess, vector< double > &vtess, const int & n_ref = 0 ) const;
    virtual void UpdateTesselate( const VspSurf &surf, bool capUMinSuccess, bool capUMaxSuccess, bool degen, vector< vector< vec3d > > &pnts, vector< vector< vec3d > > &norms, vector< vector< vec3d > > &uw_pnts, const int & n_ref = 0 ) const;
    virtual void UpdateSplitTesselate( const VspSurf &surf, bool capUMinSuccess, bool capUMaxSuccess, vector< vector< vector< vec3d > > > &pnts, vector< vector< vector< vec3d > > > &norms ) const;

    virtual void EnforceOrder( StackXSec* xs, int indx, int policy );

    vector<int> m_TessUVec;
    vector < double > m_FwdClusterVec;
    vector < double > m_AftClusterVec;
    virtual void UpdatePreTess();

    bool m_Closed;

};
#endif // !defined(VSPSTACKGEOM__INCLUDED_)
