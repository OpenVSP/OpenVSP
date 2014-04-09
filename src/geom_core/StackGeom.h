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

#include "Geom.h"
#include "XSec.h"
#include "XSecCurve.h"
#include "XSecSurf.h"

//==== Stack Geom ====//
class StackGeom : public GeomXSec
{
public:
    enum {LINEAR_CONNECT, PCHIP_CONNECT, CSPLINE_CONNECT, MANUAL_CONNECT, NUM_CONNECT};

    StackGeom( Vehicle* vehicle_ptr );
    virtual ~StackGeom();

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

protected:
    virtual void ChangeID( string id );

    virtual void UpdateSurf();

    int m_ActiveXSec;
    XSecSurf m_XSecSurf;

    bool m_Closed;

};
#endif // !defined(VSPSTACKGEOM__INCLUDED_)
