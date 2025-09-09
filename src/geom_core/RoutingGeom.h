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

    void SetPlaced( bool p ) { m_Placed = p; };
    bool IsPlaced() const { return m_Placed; };

    void UpdateParms();

    virtual xmlNodePtr EncodeXml( xmlNodePtr & node );
    virtual xmlNodePtr DecodeXml( xmlNodePtr & node );
    virtual void ParmChanged( Parm* parm_ptr, int type );

    IntParm m_SurfIndx;

    IntParm m_CoordType;

    Parm m_U;
    Parm m_U0N;
    BoolParm m_U01Flag;
    Parm m_W;

    Parm m_R;
    BoolParm m_R01Flag;
    Parm m_R0N;
    Parm m_S;
    Parm m_T;

    Parm m_L;
    BoolParm m_L01Flag;
    Parm m_L0Len;
    Parm m_M;
    Parm m_N;

    Parm m_Eta;

    IntParm m_DeltaType;

    Parm m_DeltaX;
    Parm m_DeltaY;
    Parm m_DeltaZ;

    Parm m_DeltaXRel;
    Parm m_DeltaYRel;
    Parm m_DeltaZRel;

    Parm m_Radius;

    vec3d m_AttachOrigin;
    vector < vec3d > m_AttachAxis;

protected:
    string m_ParentID;
    vec3d m_Pt;

    bool m_Placed;

};


//==== Routing Geom ====//
class RoutingGeom : public Geom
{
public:
    RoutingGeom( Vehicle* vehicle_ptr );
    virtual ~RoutingGeom();

    virtual xmlNodePtr EncodeXml( xmlNodePtr & node );
    virtual xmlNodePtr DecodeXml( xmlNodePtr & node );

    virtual void AddLinkableParms( vector< string > & linkable_parm_vec, const string & link_container_id = string() );
    virtual void ChangeID( const string &id );
    virtual void UpdateParents();

    virtual void ComputeCenter();

    virtual void Scale();

    virtual void AddDefaultSources( double base_len = 1.0 );

    virtual void OffsetXSecs( double off );

    virtual int GetNumPt() const          { return m_RoutingPointVec.size(); };
    virtual RoutingPoint* AddPt();
    virtual RoutingPoint* InsertPt( int index );
    virtual void DelPt( int index );
    virtual void DelAllPt();

    virtual int MovePt( int index, int reorder_type );

    virtual RoutingPoint * GetPt( int index );
    virtual vector < RoutingPoint* > GetAllPt()      { return m_RoutingPointVec; };

    virtual vec3d GetPtCoord( int index, int symm_index );
    virtual vector < vec3d > GetAllPtCoord( int symm_index );
    virtual vector < vec3d > GetCurve( int symm_index );

    virtual string GetPtID( int index ) const;
    virtual vector < string > GetAllPtIds() const;

    void SetPlaced( int index, bool p );
    bool IsPlaced( int index ) const;

    virtual void LoadDrawObjs( vector< DrawObj* > & draw_obj_vec );

    int m_ActivePointIndex;
    bool m_Picking;

    Parm m_Length;
    Parm m_SymmLength;

protected:

    virtual void UpdateSurf();
    virtual void DisableParms();

    virtual void UpdateSymmAttach();
    virtual void UpdateDrawObj();
    virtual void UpdateBBox();
    virtual void UpdateTessVec();
    virtual void UpdateXForm();

    vector < RoutingPoint* > m_RoutingPointVec;


    DrawObj m_RouteLineDO;

    DrawObj m_DynamicRouteDO;
    DrawObj m_ActivePointDO;
    vector < DrawObj > m_ActivePointAxisDrawObj_vec;

    std::size_t m_ParentHash;
    vector < string > m_ParentVec;

    vector < VspCurve > m_MainRouteCurveVec;

    vector <SimpleFeatureTess> m_MainRouteCurveTessVec;
    vector <SimpleFeatureTess> m_RouteTessCurveVec;

    vector <SimpleFeatureTess> m_MainRouteTessVec;
    vector <SimpleFeatureTess> m_RouteTessVec;
};

#endif // !defined(VSPROUTINGGEOM__INCLUDED_)
