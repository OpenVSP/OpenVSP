//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//
//
//////////////////////////////////////////////////////////////////////

#if !defined(VSPWIREGEOM__INCLUDED_)
#define VSPWIREGEOM__INCLUDED_

#include "Geom.h"
#include "GeomInterface.h"

class UnformattedIn;

//==== Wireframe Geom ====//
//==== A Geom whose shape is a grid of points ====//
// Implemented by WireGeom.  The points are rearranged (swapped, reversed, skipped, strided,
// patched) into a main grid in the Geom's own frame and placed on output.  A Clone borrows the
// rearranged grid.
class WirePtRole : virtual public GeomInterface
{
public:
    // The behavior type this role belongs to; checked by Geom::CastTo.
    static int BehaviorType()   { return WIRE_FRAME_GEOM_TYPE; }

    virtual ~WirePtRole()   {}

    // The rearranged grid, in this Geom's own frame.
    virtual const vector < vector < vec3d > > & GetMainWirePts() const = 0;

    // Where this Geom stands it.
    virtual Matrix4d GetWireTransMat() const = 0;
    // The shape's own scaling, not part of the placement.  A Clone applies it too.
    virtual Matrix4d GetWireScaleMat() const = 0;

    // Which way the surface faces.
    virtual bool GetWireInvert() const = 0;

    // Whether the grid is a surface (lifting) or a body (non-lifting).
    virtual int GetWireDegenType() const = 0;

protected:
    // The grid, placed, and the normals worked out there so a scale in the transform counts.
    static void BuildWireXFormPts( const vector < vector < vec3d > > &main_pts, const Matrix4d &trans,
                                   bool invert,
                                   vector < vector < vec3d > > &xform_pts,
                                   vector < vector < vec3d > > &xform_norm );

    // Its extent.
    static void BuildWireBndBox( const vector < vector < vec3d > > &xform_pts, BndBox &bbox );

    // Draw it as a grid of quads.  A single row or column of points (e.g. a Plot3D file of curves)
    // is drawn as a polyline into line_do instead, leaving draw_obj_vec empty.
    static void BuildWireDrawObjs( const vector < vector < vec3d > > &xform_pts,
                                   const vector < vector < vec3d > > &xform_norm,
                                   vector < DrawObj > &draw_obj_vec, DrawObj &line_do );

    // The polyline, shown the way this Geom shows its wireframe.
    void LoadWireLineDrawObj( DrawObj &line_do, vector< DrawObj* > &draw_obj_vec );

    // The same quads as triangles, two per cell, wound to face the way the surface does, for
    // analyses.  Tagged as geom_ptr.
    static vector< TMesh* > BuildWireTMeshVec( const vector < vector < vec3d > > &xform_pts,
                                               bool invert, const Geom* geom_ptr );
};

class WireGeom : public Geom, public WirePtRole
{
public:
    WireGeom( Vehicle* vehicle_ptr );
    virtual ~WireGeom();

    virtual void UpdateSurf() override;
    virtual void UpdateXForm() override;
    virtual void UpdateDrawObj() override;
    virtual void LoadDrawObjs( vector< DrawObj* > & draw_obj_vec ) override;

    virtual void ApplyScale( double currentScale ) override;
    // Put the main points where this Geom sits, and work out the normals there.  Called
    // whenever the placement or the shape changes, so moving the Geom moves what is drawn.
    virtual void UpdateXFormPts();

    virtual void UpdateBBox() override;
    virtual Matrix4d GetTotalTransMat() const;

    virtual void ReadP3D( FILE* fp, int ni, int nj, int nk, int nvar = 3 );
    virtual void ReadP3D( UnformattedIn &fp, int ni, int nj, int nk, int nvar = 3 );
    virtual void ReadXSec( FILE* fp );

    virtual xmlNodePtr EncodeXml( xmlNodePtr & node ) override;
    virtual xmlNodePtr DecodeXml( xmlNodePtr & node ) override;

    virtual vector< TMesh* > CreateTMeshVec( bool skipnegflipnormal, const int & n_ref = 0 ) const override;

    virtual void CreateDegenGeom( vector<DegenGeom> &dgs, bool preview = false, const int & n_ref = 0 ) override;

    virtual int GetNumTotalHrmSurfs() const override;
    virtual void WriteXSecFile( int geom_no, FILE* dump_file ) override;

    // Scale Transformation Matrix
    Matrix4d m_ScaleMatrix;
    Parm m_ScaleFromOrig;

    IntParm m_WireType;
    BoolParm m_InvertFlag;
    bool m_OtherInvertFlag;

    BoolParm m_SwapIJFlag;
    BoolParm m_RevIFlag;
    BoolParm m_RevJFlag;

    IntParm m_IStride;
    IntParm m_JStride;

    IntParm m_ISkipStart;
    IntParm m_ISkipEnd;
    IntParm m_JSkipStart;
    IntParm m_JSkipEnd;

    IntParm m_IStartPatchType;
    IntParm m_IEndPatchType;
    IntParm m_JStartPatchType;
    IntParm m_JEndPatchType;

protected:

    // The wireframe drawn as a polyline, for the case where it is a single row or column of
    // points and there are no quads to make a mesh out of.  Empty otherwise.
    DrawObj m_LineDO;

    vector < vector < vec3d > > m_WirePts;

    // The points after they have been rearranged -- swapped, reversed, skipped, strided,
    // patched -- but before this Geom's placement is applied.  This is the shape itself, which
    // is what another Geom can stand in for.
public:
    virtual const vector < vector < vec3d > > & GetMainWirePts() const override
    {
        return m_MainPts;
    }
    virtual Matrix4d GetWireTransMat() const override
    {
        return GetTotalTransMat();
    }

    virtual Matrix4d GetWireScaleMat() const override
    {
        return m_ScaleMatrix;
    }
    // A flip reverses the normals worked out from the placed points.
    virtual bool GetWireInvert() const override
    {
        return ( m_InvertFlag() ^ m_OtherInvertFlag ) != GetFlipReversesNormal();
    }
    virtual int GetWireDegenType() const override
    {
        if ( m_WireType() == 1 )
        {
            return DegenGeom::BODY_TYPE;
        }
        return DegenGeom::SURFACE_TYPE;
    }

protected:
    vector < vector < vec3d > > m_MainPts;
    vector < vector < vec3d > > m_XFormPts;

    vector < vector < vec3d > > m_XFormNorm;

    bool CheckInverted();

    static void PatchRow( const vector < vec3d > &oldrow, const vector < vec3d > &oppositerow, int type, vector < vec3d > &newrow );

};

#endif // !defined(VSPWIREGEOM__INCLUDED_)
