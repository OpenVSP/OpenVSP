//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//

// BORGeom.h:
// Rob McDonald
//
//////////////////////////////////////////////////////////////////////

#if !defined(VSP_BOR_GEOM__INCLUDED_)
#define VSP_BOR_GEOM__INCLUDED_

#include "Geom.h"
#include "XSec.h"


//==== BOR Geom ====//
class BORGeom : public Geom
{
public:
    BORGeom( Vehicle* vehicle_ptr );
    virtual ~BORGeom();

    virtual void ComputeCenter();
    virtual void Scale();
    virtual void AddDefaultSources( double base_len = 1.0 );

    virtual void OffsetXSecs( double off );

    virtual void SetXSecCurveType( int type );

    XSecCurve* GetXSecCurve()       { return m_XSCurve; }

    virtual void LoadDrawObjs( vector< DrawObj* > & draw_obj_vec );

    virtual xmlNodePtr EncodeXml( xmlNodePtr & node );
    virtual xmlNodePtr DecodeXml( xmlNodePtr & node );

    Parm m_Diameter;

    Parm m_Angle;

    Parm m_LECluster;
    Parm m_TECluster;

    IntParm m_Mode;

    Parm m_A0;
    Parm m_Ae;
    Parm m_Amin;
    Parm m_AminW;

protected:
    virtual void UpdateSurf();

    virtual void UpdateDrawObj();

    double m_Xoff;

    XSecCurve *m_XSCurve;

    DrawObj m_CurrentXSecDrawObj;
};


#endif // !defined(VSP_BOR_GEOM__INCLUDED_)
