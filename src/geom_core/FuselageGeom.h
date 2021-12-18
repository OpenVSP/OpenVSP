//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//

// FuselageGeom.h:
// J.R Gloudemans
//
//////////////////////////////////////////////////////////////////////

#if !defined(VSPFUSELAGEGEOM__INCLUDED_)
#define VSPFUSELAGEGEOM__INCLUDED_

#include "Defines.h"
#include "Geom.h"
#include "XSec.h"
#include "XSecCurve.h"
#include "XSecSurf.h"

//==== Fuselage Geom ====//
class FuselageGeom : public GeomXSec
{
public:
    enum {FUSE_MONOTONIC, FUSE_LOOP, FUSE_FREE, NUM_FUSE_POLICY};

    FuselageGeom( Vehicle* vehicle_ptr );
    virtual ~FuselageGeom();

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

    virtual void ReadV2File( xmlNodePtr &root );
    virtual void ReadV2FileFuse2( xmlNodePtr &root );
    virtual void ReadV2FileFuse1( xmlNodePtr &root );

    Parm m_Length;                  // Length of Fuselage
    IntParm m_OrderPolicy;

    enum { V2_FXS_POINT = 0,
           V2_FXS_CIRCLE = 1,
           V2_FXS_ELLIPSE = 2,
           V2_FXS_BOX = 3,
           V2_FXS_RND_BOX = 4,
           V2_FXS_GENERAL = 5,
           V2_FXS_FROM_FILE = 6,
           V2_FXS_EDIT_CRV = 7
    };

    enum { V2_XSEC_POINT = 0,
           V2_CIRCLE = 1,
           V2_ELLIPSE = 2,
           V2_RND_BOX = 3,
           V2_GENERAL = 4,
           V2_FROM_FILE = 5,
           V2_EDIT_CRV = 6
    };


protected:
    virtual void ChangeID( string id );

    virtual void UpdateSurf();
    virtual void UpdateTesselate( const vector<VspSurf> &surf_vec, int indx, vector< vector< vec3d > > &pnts, vector< vector< vec3d > > &norms, vector< vector< vec3d > > &uw_pnts, bool degen ) const;
    virtual void UpdateSplitTesselate( const vector<VspSurf> &surf_vec, int indx, vector< vector< vector< vec3d > > > &pnts, vector< vector< vector< vec3d > > > &norms ) const;

    virtual void EnforceOrder( FuseXSec* xs, int indx, int policy );


    vector<int> m_TessUVec;
    virtual void UpdatePreTess();
};
#endif // !defined(VSPPODGEOM__INCLUDED_)
