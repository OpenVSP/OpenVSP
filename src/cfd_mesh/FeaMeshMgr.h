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

using namespace std;

enum
{
    SURFACE_FIX_POINT, BORDER_FIX_POINT, INTERSECT_FIX_POINT
};


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

    virtual FeaMesh* GetMeshPtr()
    {
        return &m_ThisMesh;
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

    virtual void WriteCalculix( );
    virtual void WriteNASTRAN( const string &base_filename );
    virtual void WriteGmsh();

    virtual void ComputeWriteMass();

    virtual double GetTotalMass()
    {
        return m_TotalMass;
    }

    virtual void SetFeaMeshStructIndex( int index )
    {
        m_FeaMeshStructIndex = index;
    }

    virtual int GetFeaMeshStructIndex()
    {
        return m_FeaMeshStructIndex;
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
        return m_SimplePropertyVec;
    }

    virtual vector < SimpleFeaMaterial > GetSimpleMaterialVec()
    {
        return m_SimpleMaterialVec;
    }

    virtual void RegisterAnalysis();

    virtual Surf* GetFeaSurf( int FeaPartID, int surf_num );

protected:

    virtual void GetMassUnit();

    virtual void WriteNASTRANSet( FILE* Nastran_fid, FILE* NKey_fid, int & set_num, vector < int > set_ids, const string &set_name, const int &offset );

    bool m_FeaMeshInProgress;
    bool m_CADOnlyFlag; // Indicates that ne meshing should be performed, but the surfaces are still exported

    double m_TotalMass;
    string m_MassUnit;

    string m_StructName;
    int m_FeaMeshStructIndex;

    unsigned int m_NumFeaParts;
    unsigned int m_NumFeaFixPoints;
    unsigned int m_NumFeaSubSurfs;

    vector < string > m_FeaPartNameVec;
    vector < int > m_FeaPartTypeVec;
    vector < int > m_FeaPartNumSurfVec;
    vector < int > m_FeaPartIncludedElementsVec;
    vector < int > m_FeaPartPropertyIndexVec;
    vector < int > m_FeaPartCapPropertyIndexVec;

    // Groups of trimming planes.
    vector < vector < vec3d > > m_TrimPt;
    vector < vector < vec3d > > m_TrimNorm;

    SimpleFeaMeshSettings m_StructSettings;
    SimpleGridDensity m_FeaGridDensity;

    vector < SimpleFeaProperty > m_SimplePropertyVec;
    vector < SimpleFeaMaterial > m_SimpleMaterialVec;


    FeaMesh m_ThisMesh;

};

#define FeaMeshMgr FeaMeshMgrSingleton::getInstance()

#endif
