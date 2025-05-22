//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//

// GeometryAnalysisMgr.h
//
// Rob McDonald
//////////////////////////////////////////////////////////////////////

#if !defined(GEOMETRYANALYSISMGR__INCLUDED_)
#define GEOMETRYANALYSISMGR__INCLUDED_

#include "ParmContainer.h"
#include "Parm.h"
#include "TMesh.h"
#include "DrawObj.h"

class GeometryAnalysisCase : public ParmContainer
{
public:

    GeometryAnalysisCase();

    void Update();

    string GetPrimaryName() const;
    string GetSecondaryName() const;

    vector< TMesh* > GetPrimaryTMeshVec();
    vector< TMesh* > GetSecondaryTMeshVec();

    void GetPrimaryTwoPtSideContactPtsNormal( vec3d &p1, vec3d &p2, vec3d &normal );
    void GetPrimaryContactPointVecNormal( vector < vec3d > &ptvec, vec3d &normal );
    void GetPrimaryCG( vec3d &cgnom, vector < vec3d > &cgbounds );

    void GetSecondaryPtNormal( vec3d &pt, vec3d &normal );
    void GetSecondarySideContactPtRollAxisNormal( vec3d &pt, vec3d &axis, vec3d &normal, int &ysign );
    void GetSecondaryPtNormalMeanContactPivotAxis( vec3d &pt, vec3d &normal, vec3d &ptaxis, vec3d &axis, bool &usepivot, double &mintheta, double &maxtheta );
    void GetSecondaryPtNormalAftAxleAxis( double thetabogie, vec3d &pt, vec3d &normal, vec3d &ptaxis, vec3d &axis );
    void GetSecondaryPtNormalFwdAxleAxis( double thetabogie, vec3d &pt, vec3d &normal, vec3d &ptaxis, vec3d &axis );

    virtual xmlNodePtr EncodeXml( xmlNodePtr & node );
    virtual xmlNodePtr DecodeXml( xmlNodePtr & node );

    string Evaluate();

    vec3d weightdist( const vec3d &cg, const vector < vec3d > &ptvec, const vec3d &normal );
    double tipback( const vec3d &cg, const vec3d &normal, const vec3d &ptaxis, const vec3d &axis, vec3d &p0, vec3d &p1 );
    double tipover( const vec3d &cg, const vec3d &normal, const vec3d &ptaxis, const vec3d &axis, vec3d &p0, vec3d &p1 );

    void ShowBoth();
    void ShowOnlyBoth();
    void ShowPrimary();
    void ShowOnlyPrimary();
    void ShowSecondary();
    void ShowOnlySecondary();

    void UpdateDrawObj_PostAnalysis();
    void UpdateDrawObj_Live();

    void LoadDrawObjs( vector< DrawObj* > & draw_obj_vec );

    IntParm m_PrimarySet;
    IntParm m_PrimaryType; // Mode, Set, or Geom
    string m_PrimaryModeID;
    string m_PrimaryGeomID;

    IntParm m_SecondarySet;
    IntParm m_SecondaryType; // Set or Geom
    string m_SecondaryGeomID;

    Parm m_SecondaryZGround;
    BoolParm m_SecondaryUseZGround;

    BoolParm m_SecondaryCCWFlag;

    BoolParm m_PolyVisibleFlag;

    BoolParm m_SecondaryUsePoint;
    Parm m_SecondaryX;
    Parm m_SecondaryY;
    Parm m_SecondaryZ;

    IntParm m_GeometryAnalysisType;

    string m_LastResult;
    Parm m_LastResultValue;

    vector< TMesh* > m_TMeshVec;
    vector < vec3d > m_PtsVec;


    vector < DrawObj > m_MeshResultDO_vec;
    DrawObj m_LineResultDO;
};


class GeometryAnalysisMgrSingleton
{
protected:
    GeometryAnalysisMgrSingleton();

public:

    static GeometryAnalysisMgrSingleton& getInstance()
    {
        static GeometryAnalysisMgrSingleton instance;
        return instance;
    }

    virtual ~GeometryAnalysisMgrSingleton();

    virtual xmlNodePtr EncodeXml( xmlNodePtr & node );
    virtual xmlNodePtr DecodeXml( xmlNodePtr & node );

    void Renew();
    void Wype();

    virtual void Update();

    string EvaluateAll();

    void AddLinkableContainers( vector< string > & linkable_container_vec );


    string AddGeometryAnalysis();
    void DeleteGeometryAnalysis( const string &id );
    void DeleteGeometryAnalysis( int indx );
    void DeleteAllGeometryAnalyses();

    GeometryAnalysisCase * GetGeometryAnalysis( int indx ) const;
    GeometryAnalysisCase * GetGeometryAnalysis( const string &id ) const;
    int GetGeometryAnalysisIndex( const string &id ) const;

    vector < GeometryAnalysisCase* > GetAllGeometryAnalyses() const            { return m_GeometryAnalysisVec; };


protected:
    vector < GeometryAnalysisCase* > m_GeometryAnalysisVec;

};

#define GeometryAnalysisMgr GeometryAnalysisMgrSingleton::getInstance()

#endif
