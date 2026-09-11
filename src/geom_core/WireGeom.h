//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//
//
//////////////////////////////////////////////////////////////////////

#if !defined(VSPWIREGEOM__INCLUDED_)
#define VSPWIREGEOM__INCLUDED_

#include "Geom.h"


//==== Wireframe Geom ====//
//==== A Geom whose shape is a grid of points ====//
class WireGeom : public Geom
{
public:
    WireGeom( Vehicle* vehicle_ptr );
    virtual ~WireGeom();

    virtual void UpdateSurf() override;
    virtual void UpdateDrawObj() override;

    virtual void Scale() override;
    // Put the main points where this Geom sits, and work out the normals there.  Called
    // whenever the placement or the shape changes, so moving the Geom moves what is drawn.
    virtual void UpdateXFormPts();

    virtual void UpdateBBox() override;
    virtual Matrix4d GetTotalTransMat() const;

    virtual void ReadP3D( FILE* fp, int ni, int nj, int nk );
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

    vector < vector < vec3d > > m_WirePts;

    // The grid, placed, and the normals worked out there so a scale in the transform counts.
    static void BuildWireXFormPts( const vector < vector < vec3d > > &main_pts, const Matrix4d &trans,
                                   bool invert,
                                   vector < vector < vec3d > > &xform_pts,
                                   vector < vector < vec3d > > &xform_norm );

    // Its extent.
    static void BuildWireBndBox( const vector < vector < vec3d > > &xform_pts, BndBox &bbox );

    // Draw it as a grid of quads.
    static void BuildWireDrawObjs( const vector < vector < vec3d > > &xform_pts,
                                   const vector < vector < vec3d > > &xform_norm,
                                   vector < DrawObj > &draw_obj_vec );


    // The points after they have been rearranged -- swapped, reversed, skipped, strided,
    // patched -- but before this Geom's placement is applied.  This is the shape itself, which
    // is what another Geom can stand in for.
public:
    virtual const vector < vector < vec3d > > & GetMainWirePts() const
    {
        return m_MainPts;
    }
    virtual Matrix4d GetWireTransMat() const
    {
        return GetTotalTransMat();
    }
    virtual bool GetWireInvert() const
    {
        return m_InvertFlag() ^ m_OtherInvertFlag;
    }

protected:
    vector < vector < vec3d > > m_MainPts;
    vector < vector < vec3d > > m_XFormPts;

    vector < vector < vec3d > > m_XFormNorm;

    bool CheckInverted();

    static void PatchRow( const vector < vec3d > &oldrow, const vector < vec3d > &oppositerow, int type, vector < vec3d > &newrow );

};

#endif // !defined(VSPWIREGEOM__INCLUDED_)
