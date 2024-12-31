//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//

// RoutingGeom.h:
// Rob McDonald
//
//////////////////////////////////////////////////////////////////////



#if !defined(VSPROUTINGGEOM__INCLUDED_)
#define VSPROUTINGGEOM__INCLUDED_

#include "Defines.h"
#include "Geom.h"
#include "XSec.h"
#include "XSecSurf.h"

class RoutingPoint : public ParmContainer
{
public:
    RoutingPoint();

    void Update();
    vec3d GetPt();
    string GetParentID() const { return m_ParentID; };
    void SetParentID( const string &id );

    virtual xmlNodePtr EncodeXml( xmlNodePtr & node );
    virtual xmlNodePtr DecodeXml( xmlNodePtr & node );
    virtual void ParmChanged( Parm* parm_ptr, int type );

    Parm m_U;
    Parm m_W;

protected:
    string m_ParentID;
    vec3d m_Pt;

};


//==== Clearance Geom ====//
class RoutingGeom : public Geom
{
public:
    RoutingGeom( Vehicle* vehicle_ptr );
    virtual ~RoutingGeom();

    virtual xmlNodePtr EncodeXml( xmlNodePtr & node );
    virtual xmlNodePtr DecodeXml( xmlNodePtr & node );

    virtual void ChangeID( string id );
    virtual void UpdateParents();

    virtual void ComputeCenter();

    virtual void Scale();

    virtual void AddDefaultSources( double base_len = 1.0 );

    virtual void OffsetXSecs( double off );

    virtual int GetNumPt() const          { return m_RoutingPointVec.size(); };
    virtual RoutingPoint* AddPt();
    virtual void DelPt( int index );
    virtual void DelAllPt();

    virtual RoutingPoint * GetPt( int index );
    virtual vector < RoutingPoint* > GetAllPt()      { return m_RoutingPointVec; };

    virtual void LoadDrawObjs( vector< DrawObj* > & draw_obj_vec );

protected:

    virtual void UpdateSurf();
    virtual void DisableParms();

    virtual void UpdateDrawObj();

    vector < RoutingPoint* > m_RoutingPointVec;


    DrawObj m_RouteLineDO;

    std::size_t m_ParentHash;
    vector < string > m_ParentVec;

};

#endif // !defined(VSPROUTINGGEOM__INCLUDED_)
