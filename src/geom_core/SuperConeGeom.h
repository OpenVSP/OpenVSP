//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//

// SuperConeGeom.h:
// Rob McDonald
//
//////////////////////////////////////////////////////////////////////

#if !defined(VSP_SUPERCONE_GEOM__INCLUDED_)
#define VSP_SUPERCONE_GEOM__INCLUDED_

#include "Defines.h"
#include "Geom.h"
#include "XSec.h"


//==== SuperCone Geom ====//
class SuperConeGeom : public Geom
{
public:
    SuperConeGeom( Vehicle* vehicle_ptr );
    virtual ~SuperConeGeom();

    virtual void ComputeCenter();
    virtual void Scale();
    virtual void AddDefaultSources( double base_len = 1.0 );

    virtual void OffsetXSecs( double off );

    virtual void SetXSecCurveType( int type );
    virtual int GetXSecCurveType();

    XSecCurve* GetXSecCurve()       { return m_XSCurve; }

    virtual void LoadDrawObjs( vector< DrawObj* > & draw_obj_vec );

    virtual xmlNodePtr EncodeXml( xmlNodePtr & node );
    virtual xmlNodePtr DecodeXml( xmlNodePtr & node );

    virtual void AddLinkableParms( vector< string > & parm_vec, const string & link_container_id = string() );

    // Alternative to XSecSurf::ConvertToEdit for SuperCone components
    virtual EditCurveXSec* ConvertToEdit();

    Parm m_Distance;

protected:
    virtual void UpdateSurf();
    virtual void ChangeID( string id );

    virtual void UpdateDrawObj();
    virtual void UpdatePreTess();

    double m_Xoff;

    XSecCurve *m_XSCurve;

    DrawObj m_CurrentXSecDrawObj;

    VspSurf m_FoilSurf;
};


#endif // !defined(VSP_SUPERCONE_GEOM__INCLUDED_)
