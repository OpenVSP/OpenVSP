//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//
//
//////////////////////////////////////////////////////////////////////

#if !defined(VSPNGonMeshGeom__INCLUDED_)
#define VSPNGonMeshGeom__INCLUDED_

#include "Geom.h"
#include "DrawObj.h"

#include "TMesh.h"
#include "PGMesh.h"

//==== Point Cloud Geom ====//
class NGonMeshGeom : public Geom
{
public:
    NGonMeshGeom( Vehicle* vehicle_ptr );
    virtual ~NGonMeshGeom();

    virtual int GetNumMainSurfs() const
    {
        return 0;
    };

    virtual void UpdateSurf();
    virtual void UpdateDrawObj();
    virtual void LoadDrawObjs(vector< DrawObj* > & draw_obj_vec);

    virtual void Scale();
    virtual void UpdateBBox();
    virtual Matrix4d GetTotalTransMat()const ;

    virtual xmlNodePtr EncodeXml( xmlNodePtr & node );
    virtual xmlNodePtr DecodeXml( xmlNodePtr & node );

    virtual void SplitLEGeom();
    virtual void Triangulate();
    virtual void Report();
    virtual void ClearTris();

    virtual void RemovePotentialFiles( const string& file_name );
    virtual void WriteVSPGEOM( string fname, vector < string > &all_fnames );

    virtual vector< TMesh* > CreateTMeshVec();

    // Scale Transformation Matrix
    Matrix4d m_ScaleMatrix;
    Parm m_ScaleFromOrig;

    BoolParm m_ShowNonManifoldEdges;

    IntParm m_ActiveMesh;

    DrawObj m_BadEdgeTooFewDO;
    DrawObj m_BadEdgeTooManyDO;

    vector<DrawObj> m_WakeEdgeDrawObj_vec;
    vector<DrawObj> m_WakeNodeDrawObj_vec;

    DrawObj m_CoLinearLoopDO;
    DrawObj m_DoubleBackNodeDO;

    vector<DrawObj> m_LabelDO_vec;

    PGMulti m_PGMulti;

protected:

};

#endif // !defined(VSPNGonMeshGeom__INCLUDED_)
