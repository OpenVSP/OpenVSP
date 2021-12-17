//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//
//
//////////////////////////////////////////////////////////////////////

#if !defined(VSPPTCLOUDGEOM__INCLUDED_)
#define VSPPTCLOUDGEOM__INCLUDED_

#include "Geom.h"


//==== Point Cloud Geom ====//
class PtCloudGeom : public Geom
{
public:
    PtCloudGeom( Vehicle* vehicle_ptr );
    virtual ~PtCloudGeom();

    virtual int GetNumMainSurfs() const
    {
        return 0;
    };

    virtual void UpdateSurf();
    virtual void UpdateDrawObj();
    virtual void LoadDrawObjs(vector< DrawObj* > & draw_obj_vec);
    virtual string getFeedbackGroupName();

    virtual void Scale();
    virtual void UpdateBBox();
    virtual Matrix4d GetTotalTransMat()const ;

    virtual int ReadPTS( const char* file_name );

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


    int GetNumSelected()
    {
        return m_NumSelected;
    }

    void ProjectPts( string geomid, int surfid, int idir );

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
