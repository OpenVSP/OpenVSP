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

//==== Stack Geom ====//
class StackGeom : public GeomEngine
{
public:
    enum {STACK_FREE, STACK_LOOP, NUM_STACK_POLICY};

    StackGeom( Vehicle* vehicle_ptr );
    virtual ~StackGeom();

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
