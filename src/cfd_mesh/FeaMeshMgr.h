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
#include "AnalysisMgr.h"
#include "FeaMesh.h"

#include "StructureMgr.h"

#include <unordered_map>

using namespace std;

enum
{
    SURFACE_FIX_POINT, BORDER_FIX_POINT, INTERSECT_FIX_POINT
};

typedef unordered_map < string, FeaMesh* > meshmaptype;

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
    virtual void CleanMeshMap();

    virtual SimpleGridDensity* GetGridDensityPtr()
    {
        return &m_FeaGridDensity;
    }

    virtual SimpleFeaMeshSettings* GetStructSettingsPtr()
    {
        return &m_StructSettings;
    }

    virtual SimpleMeshCommonSettings* GetSettingsPtr()
    {
        return (SimpleMeshCommonSettings* ) &m_StructSettings;
    }

    virtual void SetActiveMesh( string struct_id );

    virtual FeaMesh* GetMeshPtr()
    {
        return m_ActiveMesh;
    }

    virtual bool LoadSurfaces();
    virtual void LoadSkins();
    virtual void GenerateFeaMesh();
    virtual void ExportFeaMesh();
    virtual void TransferMeshSettings();
    virtual void IdentifyCompIDNames();
    virtual void TransferFeaData();
    virtual void TransferSubSurfData();
    virtual void MergeCoplanarParts();
    virtual void AddStructureSurfParts();
    virtual void AddStructureFixPoints();
    virtual void AddStructureTrimPlanes();
    virtual void BuildMeshOrientationLookup();
    virtual bool CullPtByTrimGroup( const vec3d &pt, const vector < vec3d > & pplane, const vector < vec3d > & nplane );
    virtual void RemoveTrimTris();
    virtual void SetFixPointSurfaceNodes();
    virtual void SetFixPointBorderNodes();
    virtual void CheckFixPointIntersects();
    virtual void BuildFeaMesh();
    virtual void CheckSubSurfBorderIntersect();
    virtual void CheckDuplicateSSIntersects();
    virtual void MergeFeaPartSSEdgeOverlap();
    virtual void RemoveSubSurfFeaTris();
    virtual void TagFeaNodes();

    virtual int GetTotalNumSurfs()
    {
        return m_SurfVec.size();
    }

    virtual void SetFeaMeshStructID( string struct_id )
    {
        FeaStructure* feastruct = StructureMgr.GetFeaStruct( struct_id );
        int index = StructureMgr.GetTotFeaStructIndex( feastruct );
        m_FeaStructID = struct_id;
        SetActiveMesh( struct_id );
    }

    virtual string GetFeaMeshStructID()
    {
        return m_FeaStructID;
    }

    virtual bool GetFeaMeshInProgress()
    {
        return m_FeaMeshInProgress;
    }
    virtual void SetFeaMeshInProgress( bool progress_flag )
    {
        m_FeaMeshInProgress = progress_flag;
    }

    virtual void SetCADOnlyFlag( bool cad_only_flag )
    {
        m_CADOnlyFlag = cad_only_flag;
    }

    virtual void TransferDrawObjData();

    virtual void UpdateDrawObjs();
    virtual void LoadDrawObjs( vector< DrawObj* > & draw_obj_vec );

    virtual void UpdateDisplaySettings();

    virtual vector < SimpleFeaProperty > GetSimplePropertyVec()
    {
        return GetMeshPtr()->m_SimplePropertyVec;
    }

    virtual vector < SimpleFeaMaterial > GetSimpleMaterialVec()
    {
        return GetMeshPtr()->m_SimpleMaterialVec;
    }

    virtual void RegisterAnalysis();

    virtual Surf* GetFeaSurf( int FeaPartID, int surf_num );

protected:

    virtual void GetMassUnit();

    bool m_FeaMeshInProgress;
    bool m_CADOnlyFlag; // Indicates that ne meshing should be performed, but the surfaces are still exported

    string m_FeaStructID;

    // Groups of trimming planes.
    vector < vector < vec3d > > m_TrimPt;
    vector < vector < vec3d > > m_TrimNorm;

    SimpleFeaMeshSettings m_StructSettings;
    SimpleGridDensity m_FeaGridDensity;

    meshmaptype m_MeshPtrMap;

    FeaMesh* m_ActiveMesh;

};

#define FeaMeshMgr FeaMeshMgrSingleton::getInstance()

#endif
