//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//

//////////////////////////////////////////////////////////////////////
// FeaMeshMgr.h
// J.R. Gloudemans
//////////////////////////////////////////////////////////////////////

#if !defined(FeaMeshMgr_FeaMeshMgr__INCLUDED_)
#define FeaMeshMgr_FeaMeshMgr__INCLUDED_

#include "CfdMeshMgr.h"
#include "FeaStructure.h"
#include "FeaElement.h"

#include <iostream>
#include <sstream>

using namespace std;

//class SectionEdge
//{
//public:
//    SectionEdge()                   {}
//    virtual ~SectionEdge()          {}
//
//    vector< double > m_LineFractVec;
//    vector< vec2d  > m_UWVec;
//    vector< vec3d  > m_PntVec;
//};
//
//class FeaGeom
//{
//public:
//
//    FeaGeom();
//    virtual ~FeaGeom();
//
//    void BuildClean();
//    void Load( Surf* surf );
//
//    //void WriteData( xmlNodePtr root );
//    //void ReadData(  xmlNodePtr root );
//
//
//    vec2d GetUW( int edge_id, double fract );
//    vec3d CompPnt( int edge_id, double fract );
//    bool IntersectPlaneEdge( int edge_id, vec3d & orig, vec3d & norm, vec2d & result );
//    void ComputePerUW( vec3d & pnt, double* per_U, double* per_W );
////  void Draw( bool highlight );
//
//    Surf* m_GeomSurfPtr;
//
//    //vec3d m_Normal;
//    //vec3d m_ChordNormal;
//    //double m_SweepLE;
//
//    enum { UW00, UW10, UW01, UW11, NUM_CORNER_PNTS };
//    vec3d m_CornerPnts[NUM_CORNER_PNTS];
//
//    enum { LEADEDGE, TRAILEDGE, INNERSIDE, OUTERSIDE, NUM_EDGES };
//    SectionEdge m_Edges[NUM_EDGES];
//
//    //vector< FeaPart* > m_FeaPartVec;
//
//    //FeaSkin* m_GeomSurfSkin;
//
//    //vector< FeaElement* > m_FeaGeomElementVec;
//};

//////////////////////////////////////////////////////////////////////
class FeaMeshMgrSingleton : public CfdMeshMgrSingleton
{
protected:
    FeaMeshMgrSingleton();
    //FeaMeshMgrSingleton( FeaMeshMgrSingleton const& copy );          // Not Implemented
    //FeaMeshMgrSingleton& operator=( FeaMeshMgrSingleton const& copy ); // Not Implemented

public:

    static FeaMeshMgrSingleton& getInstance()
    {
        static FeaMeshMgrSingleton instance;
        return instance;
    }

    virtual ~FeaMeshMgrSingleton();
    virtual void CleanUp();

    virtual GridDensity* GetGridDensityPtr()
    {
        return m_Vehicle->GetFeaGridDensityPtr();
    }

    virtual StructSettings* GetStructSettingsPtr()
    {
        return m_Vehicle->GetStructSettingsPtr();
    }

    virtual MeshCommonSettings* GetSettingsPtr()
    {
        return ( MeshCommonSettings* ) m_Vehicle->GetStructSettingsPtr();
    }

    virtual bool LoadSurfaces();
    virtual void LoadSkins();
    virtual void GenerateFeaMesh();
    virtual void ExportFeaMesh();

    virtual void AddStructureParts();

    virtual void BuildFeaMesh();

    virtual void TagFeaNodes();

    //virtual vec3d GetNormal( int sectID );
    //virtual FeaGeom* GetFeaGeom( int surfID );

    virtual int GetTotalNumSurfs()
    {
        return m_SurfVec.size();
    }

    virtual void WriteCalculix( );
    virtual void WriteNASTRAN( const string &base_filename );

    virtual void ComputeWriteMass();

//  virtual void Draw();

    virtual double GetTotalMass()
    {
        return m_TotalMass;
    }


    virtual FeaNode* FindNode( vector< FeaNode* > nodeVec, int id );

    virtual void SetFeaMeshStructIndex( int index )
    {
        m_FeaMeshStructIndex = index;
    }

    virtual void SetDrawMeshFlag( bool f )
    {
        m_DrawMeshFlag = f;
    }

    //virtual void SetDrawAttachPointsFlag( bool f )
    //{
    //    m_DrawAttachPoints = f;
    //}
    //virtual bool GetDrawAttachPointsFlag()
    //{
    //    return m_DrawAttachPoints;
    //}
    virtual bool GetFeaMeshInProgress()
    {
        return m_FeaMeshInProgress;
    }
    virtual void SetFeaMeshInProgress( bool progress_flag )
    {
        m_FeaMeshInProgress = progress_flag;
    }

    //virtual void SaveData();
    //virtual void WriteFeaStructData( Geom* geom_ptr, xmlNodePtr root );
    //virtual void SetFeaStructData( Geom* geom_ptr, xmlNodePtr root );
    //virtual void ReadFeaStructData( );
    //virtual void CopyGeomPtr( Geom* from_geom, Geom* to_geom );

    virtual void LoadDrawObjs( vector< DrawObj* > & draw_obj_vec );

    //virtual void BuildFeaPartIntChains();

protected:

    bool m_DrawMeshFlag;

    bool m_FeaMeshInProgress;

    //xmlNodePtr m_XmlDataNode;
    //vector< Geom* > m_DataGeomVec;

    double m_TotalMass;

    FeaStructure* m_FeaMeshStruct;
    int m_FeaMeshStructIndex;

    vector< FeaElement* > m_FeaElementVec;

    vector< FeaNode* > m_FeaNodeVec;
    vector< vec3d* > m_AllPntVec;
    map< int, vector< int > > m_IndMap;
    vector< int > m_PntShift;

    //bool m_DrawAttachPoints;
    //int m_ClosestAttachPoint;
    //vector< vec3d > m_AttachPoints;

    //vector< vec3d >debugPnts;

private:
    DrawObj m_SkinElemDO;
    DrawObj m_SliceElemDO;
    vector< DrawObj > m_SSTagDO;


};
//FEAMesh:
//feamass.dat        -> [modelname]_mass.dat
//feaNASTRAN.dat -> [modelname]_NASTRAN.dat
//feageom.dat        -> [modelname]_calculix_geom.dat
//feanodethick.dat ->  [modelname]_calculix_thick.dat

#define FeaMeshMgr FeaMeshMgrSingleton::getInstance()

#endif
