//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//
//
//////////////////////////////////////////////////////////////////////

#if !defined(VSPPTCLOUDGEOM__INCLUDED_)
#define VSPPTCLOUDGEOM__INCLUDED_

#include "Geom.h"
#include "GeomInterface.h"


//==== Point Cloud Geom ====//
//==== A Geom whose shape is a cloud of points ====//
// Implemented by PtCloudGeom.  Points are kept in the Geom's own frame and placed on output,
// so a Clone can borrow them.  A Clone shows the whole cloud, ignoring selection and hiding.
class PointCloudRole : virtual public GeomInterface
{
public:
    // The behavior type this role belongs to; checked by Geom::CastTo.
    static int BehaviorType()   { return PT_CLOUD_GEOM_TYPE; }

    virtual ~PointCloudRole()   {}

    // The points, in this Geom's own frame.
    virtual const vector < vec3d > & GetPtsInSelf() const = 0;

    // Where this Geom stands them.
    virtual Matrix4d GetPtsTransMat() const = 0;
    // The shape's own scaling, not part of the placement.  A Clone applies it too.
    virtual Matrix4d GetPtsScaleMat() const = 0;

protected:
    // The points, placed.
    void BuildXFormPts( vector < vec3d > &xform_pts ) const;

    // Their extent.
    void BuildPtsBndBox( BndBox &bbox ) const;
};

class PtCloudGeom : public Geom, public PointCloudRole
{
public:
    PtCloudGeom( Vehicle* vehicle_ptr );
    virtual ~PtCloudGeom();

    virtual void UpdateSurf();
    virtual void UpdateDrawObj();
    virtual void LoadDrawObjs(vector< DrawObj* > & draw_obj_vec);
    virtual string getFeedbackGroupName();

    virtual void ApplyScale( double currentScale );
    virtual void UpdateBBox();
    virtual Matrix4d GetTotalTransMat()const ;

    virtual int ReadPTS( const char* file_name );
    virtual void WritePTS( const char* file_name );

    virtual void UniquePts();
    virtual void InitPts();

    virtual xmlNodePtr EncodeXml( xmlNodePtr & node );
    virtual xmlNodePtr DecodeXml( xmlNodePtr & node );

    void SelectPoint( int index );
    void UnSelectLastSel();
    void SelectLastSel();

    void SelectAllShown();
    void SelectNone();
    void SelectInv();
    void HideSelection();
    void HideUnselected();
    void HideAll();
    void HideInv();
    void ShowAll();

    void GetSelectedPoints( vector < vec3d > &selpts );

    void CreateConvexHull();

    int GetNumSelected()
    {
        return m_NumSelected;
    }

    void ProjectPts( const string &geomid, int surfid, int idir );

    virtual const vector < vec3d > & GetPtsInSelf() const
    {
        return m_Pts;
    }
    virtual Matrix4d GetPtsTransMat() const
    {
        return GetTotalTransMat();
    }

    virtual Matrix4d GetPtsScaleMat() const
    {
        return m_ScaleMatrix;
    }

    vector < vec3d > m_Pts;
    vector < int > m_ShownIndx;
    vector < bool > m_Selected;
    vector < bool > m_Hidden;
    int m_NumSelected;
    int m_LastSelected;

    // Scale Transformation Matrix
    Matrix4d m_ScaleMatrix;
    Parm m_ScaleFromOrig;

protected:

    vector < vec3d > m_XformPts;

    DrawObj m_PtsDrawObj;
    DrawObj m_SelDrawObj;
    DrawObj m_PickDrawObj;

};

#endif // !defined(VSPPTCLOUDGEOM__INCLUDED_)
