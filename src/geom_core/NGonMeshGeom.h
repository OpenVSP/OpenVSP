//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//
//
//////////////////////////////////////////////////////////////////////

#if !defined(VSPNGonMeshGeom__INCLUDED_)
#define VSPNGonMeshGeom__INCLUDED_

#include "Geom.h"
#include "GeomInterface.h"
#include "DrawObj.h"

#include "TMesh.h"
#include "PGMesh.h"

//==== Point Cloud Geom ====//
//==== A Geom whose shape is a polygon mesh ====//
// Implemented by NGonMeshGeom.  The mesh is held untransformed in its PGMulti and placed on
// output: bounding box, drawing, analysis triangles and the VSPGEOM file.  It is not written to
// the model file.  A Clone borrows the mesh.
class PGMeshRole : virtual public GeomInterface
{
public:
    // The behavior type this role belongs to; checked by Geom::CastTo.
    static int BehaviorType()   { return NGON_GEOM_TYPE; }

    virtual ~PGMeshRole()   {}

    // The mesh, untransformed.  Writable, but asking for it does not change the Geom.
    virtual PGMulti* GetPGMulti() const = 0;

    // Where this Geom stands it.
    virtual Matrix4d GetPGTransMat() const = 0;
    // The shape's own scaling, not part of the placement.  A Clone applies it too.
    virtual Matrix4d GetPGScaleMat() const = 0;

protected:
    // The mesh's extent, placed.
    void BuildPGBndBox( BndBox &bbox ) const;

    // Its triangles, placed and tagged as geom_ptr.
    vector < TMesh* > BuildPGTMeshVec( const Geom* geom_ptr ) const;

    // The faces and their outlines, placed -- one pair of draw objects per tag.
    void BuildPGDrawObjs( vector < DrawObj > &draw_obj_vec ) const;

    // Colour each pair per tag and pick the primitives (triangles and lines) for the draw mode.
    void LoadPGDrawObjs( vector < DrawObj > &draw_obj_vec, int drawtype, bool visible ) const;
};

class NGonMeshGeom : public Geom, public PGMeshRole
{
public:
    NGonMeshGeom( Vehicle* vehicle_ptr );
    virtual ~NGonMeshGeom();

    virtual int GetNumMainSurfs() const override
    {
        return 0;
    };

    virtual void UpdateSurf() override;
    virtual void UpdateDrawObj() override;
    virtual void BuildMarkerDrawObjs( Geom* placer, vector< DrawObj > &marker_vec ) override;
    virtual void SetMarkerVisibility( Geom* placer, vector< DrawObj > &marker_vec ) override;
    virtual void LoadDrawObjs(vector< DrawObj* > & draw_obj_vec) override;

    virtual void ApplyScale( double currentScale ) override;
    virtual void UpdateBBox() override;
    virtual Matrix4d GetTotalTransMat()const ;

    virtual xmlNodePtr EncodeXml( xmlNodePtr & node ) override;
    virtual xmlNodePtr DecodeXml( xmlNodePtr & node ) override;

    virtual void SplitLEGeom();
    virtual void Triangulate();
    virtual void Report();
    virtual void ClearTris();

    virtual void RemovePotentialFiles( const string& file_name );
    virtual void WriteVSPGEOM( string fname, vector < string > &all_fnames );

    virtual vector< TMesh* > CreateTMeshVec( bool skipnegflipnormal, const int & n_ref = 0 ) const override;

    // Scale Transformation Matrix
    Matrix4d m_ScaleMatrix;
    Parm m_ScaleFromOrig;

    BoolParm m_ShowNonManifoldEdges;

    IntParm m_ActiveMesh;

    // The markers after the wakes: the mesh's defects.
    enum { NGON_BAD_EDGE_FEW, NGON_BAD_EDGE_MANY, NGON_COLINEAR_LOOP, NGON_DOUBLE_BACK_NODE, NUM_NGON_DEFECT_MARKERS };

    vector<DrawObj> m_LabelDO_vec;

    virtual PGMulti* GetPGMulti() const override
    {
        return const_cast< PGMulti* >( &m_PGMulti );
    }
    virtual Matrix4d GetPGTransMat() const override
    {
        return GetTotalTransMat();
    }

    virtual Matrix4d GetPGScaleMat() const override
    {
        return m_ScaleMatrix;
    }

    PGMulti m_PGMulti;

protected:

};

#endif // !defined(VSPNGonMeshGeom__INCLUDED_)
