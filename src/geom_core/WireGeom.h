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
class WireGeom : public Geom
{
public:
    WireGeom( Vehicle* vehicle_ptr );
    virtual ~WireGeom();

    virtual int GetNumMainSurfs()
    {
        return 0;
    };

    virtual void UpdateSurf();
    virtual void UpdateDrawObj();

    virtual void Scale();
    virtual void UpdateBBox();
    virtual Matrix4d GetTotalTransMat();

    virtual void ReadXSec( FILE* fp );

    virtual xmlNodePtr EncodeXml( xmlNodePtr & node );
    virtual xmlNodePtr DecodeXml( xmlNodePtr & node );

    virtual vector< TMesh* > CreateTMeshVec();

    virtual void CreateDegenGeom( vector<DegenGeom> &dgs, bool preview = false );

    virtual int GetNumTotalHrmSurfs();
    virtual void WriteXSecFile( int geom_no, FILE* dump_file );

    // Scale Transformation Matrix
    Matrix4d m_ScaleMatrix;
    Parm m_ScaleFromOrig;

    IntParm m_WireType;
    BoolParm m_InvertFlag;

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
    vector < vector < vec3d > > m_XFormPts;

    vector < vector < vec3d > > m_XFormNorm;

    bool CheckInverted();

    void PatchRow( const vector < vec3d > &oldrow, const vector < vec3d > &oppositerow, int type, vector < vec3d > &newrow );

};

#endif // !defined(VSPWIREGEOM__INCLUDED_)
