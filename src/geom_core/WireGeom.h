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
    virtual void LoadDrawObjs(vector< DrawObj* > & draw_obj_vec);

    virtual void Scale();
    virtual void UpdateBBox();
    virtual Matrix4d GetTotalTransMat();

    virtual void ReadXSec( FILE* fp );

    virtual xmlNodePtr EncodeXml( xmlNodePtr & node );
    virtual xmlNodePtr DecodeXml( xmlNodePtr & node );


    // Scale Transformation Matrix
    Matrix4d m_ScaleMatrix;
    Parm m_ScaleFromOrig;

    IntParm m_WireType;

protected:

    vector < vector < vec3d > > m_WirePts;
    vector < vector < vec3d > > m_XFormPts;

    vector < vector < vec3d > > m_XFormNorm;

};

#endif // !defined(VSPWIREGEOM__INCLUDED_)
