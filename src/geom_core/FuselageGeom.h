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

#include "Geom.h"
#include "XSec.h"
#include "XSecCurve.h"
#include "XSecSurf.h"

//==== Fuselage Geom ====//
class FuselageGeom : public GeomXSec
{
public:
    enum {LINEAR_CONNECT, PCHIP_CONNECT, CSPLINE_CONNECT, MANUAL_CONNECT, NUM_CONNECT};

    FuselageGeom( Vehicle* vehicle_ptr );
    virtual ~FuselageGeom();

    virtual void ComputeCenter();

    virtual xmlNodePtr EncodeXml( xmlNodePtr & node );
    virtual xmlNodePtr DecodeXml( xmlNodePtr & node );

    virtual int NumXSec()
    {
        return m_XSecSurf.NumXSec();
    }


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

    //==== Set Drag Factors ====//
    virtual void LoadDragFactors( DragFactors& drag_factors );

    Parm m_Length;                  // Length of Fuselage
    IntParm m_OrderPolicy;

protected:
    virtual void ChangeID( string id );

    virtual void UpdateSurf();
    virtual void UpdateTesselate( int indx, vector< vector< vec3d > > &pnts, vector< vector< vec3d > > &norms, vector< vector< vec3d > > &uw_pnts );

    enum {FUSE_MONOTONIC, FUSE_DUCT, FUSE_FREE, NUM_FUSE_POLICY};
    virtual void EnforceOrder( FuseXSec* xs, int indx, int policy );


    vector<int> m_TessUVec;
};
#endif // !defined(VSPPODGEOM__INCLUDED_)
