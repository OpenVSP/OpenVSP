//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//
//
//////////////////////////////////////////////////////////////////////

#if !defined(VSPNGonMeshGeom__INCLUDED_)
#define VSPNGonMeshGeom__INCLUDED_

#include "Geom.h"

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

    virtual void BuildFromTMesh( const vector< TNode* > nodeVec, const vector< TTri* > &triVec, const vector < deque < TEdge > > &wakes );
    virtual void PolygonizeMesh();
    virtual void CleanColinearVerts();
    virtual void SplitLEGeom();

    virtual void WriteVSPGEOM( string fname );

    // Scale Transformation Matrix
    Matrix4d m_ScaleMatrix;
    Parm m_ScaleFromOrig;

protected:


    PGMesh m_PGMesh;

};

#endif // !defined(VSPNGonMeshGeom__INCLUDED_)
