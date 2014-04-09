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

    virtual int GetNumMainSurfs()
    {
        return 1;
    }

    virtual void ComputeCenter();

    virtual xmlNodePtr EncodeXml( xmlNodePtr & node );
    virtual xmlNodePtr DecodeXml( xmlNodePtr & node );

    virtual int NumXSec()
    {
        return m_XSecSurf.NumXSec();
    }
    virtual int GetActiveXSecIndex()
    {
        return m_ActiveXSec;
    }
    virtual void SetActiveXSecIndex( int index );

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

    virtual XSec* GetXSec( int index );

    void GetJointParams( int joint, VspJointInfo &jointInfo ) const;

    bool IsClosed() const;

    virtual void AddLinkableParms( vector< string > & linkable_parm_vec, const string & link_container_id = string() );
    virtual void Scale();

    //==== Set Drag Factors ====//
    virtual void LoadDragFactors( DragFactors& drag_factors );

    Parm m_Length;                  // Length of Fuselage

protected:
    virtual void ChangeID( string id );

    virtual void UpdateSurf();

    enum {FUSE_MONOTONIC, FUSE_DUCT, FUSE_FREE};
    virtual void EnforceOrder( FuseXSec* xs, int indx, int ile, int policy );

    int m_ActiveXSec;
    XSecSurf m_XSecSurf;

    bool m_Closed;

};
#endif // !defined(VSPPODGEOM__INCLUDED_)
