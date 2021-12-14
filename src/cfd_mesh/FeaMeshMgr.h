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

    virtual bool LoadSurfaces();
    virtual void LoadSkins();
    virtual void GenerateFeaMesh();
    virtual void ExportFeaMesh();
    virtual void TransferMeshSettings();
    virtual void IdentifyCompIDNames();
    virtual void TransferFeaData();
    virtual void TransferSubSurfData();
    virtual void MergeCoplanarParts();
    virtual void AddStructureParts();
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
    virtual bool FeaDataAvailable();
    virtual void SetAllDisplayFlags( bool flag );
    virtual void LoadDrawObjs( vector< DrawObj* > & draw_obj_vec );

    virtual void UpdateDisplaySettings();

    virtual vector < string > GetDrawBrowserNameVec()
    {
        return m_DrawBrowserNameVec;
    }

    virtual vector < int > GetDrawBrowserIndexVec()
    {
        return m_DrawBrowserPartIndexVec;
    }

    virtual vector < bool > GetDrawElementFlagVec()
    {
        return m_DrawElementFlagVec;
    }

    virtual void SetDrawElementFlag( int index, bool flag );

    virtual vector < bool > GetDrawCapFlagVec()
    {
        return m_DrawCapFlagVec;
    }

    virtual void SetDrawCapFlag( int index, bool flag );

    virtual vector < SimpleFeaProperty > GetSimplePropertyVec()
    {
        return m_SimplePropertyVec;
    }

    virtual vector < SimpleFeaMaterial > GetSimpleMaterialVec()
    {
        return m_SimpleMaterialVec;
    }

    virtual void RegisterAnalysis();

protected:

    virtual void GetMassUnit();

    virtual void WriteNASTRANSet( FILE* Nastran_fid, FILE* NKey_fid, int & set_num, vector < int > set_ids, const string &set_name );

    bool m_FeaMeshInProgress;
    bool m_CADOnlyFlag; // Indicates that ne meshing should be performed, but the surfaces are still exported

    double m_TotalMass;
    string m_MassUnit;

    string m_StructName;
    int m_FeaMeshStructIndex;

    unsigned int m_NumFeaParts;
    unsigned int m_NumFeaSubSurfs;
    unsigned int m_NumFeaFixPoints;
    unsigned int m_NumTris;
    unsigned int m_NumBeams;

    vector < string > m_FeaPartNameVec;
    vector < int > m_FeaPartTypeVec;
    vector < int > m_FeaPartIncludedElementsVec;
    vector < int > m_FeaPartPropertyIndexVec;
    vector < int > m_FeaPartCapPropertyIndexVec;

    // The following vectors are mapped to FeaFixPoint count index
    map < int, vector < vec3d > > m_FixPntMap; // Vector 3D coordinates for FeaFixPoints 
    map < int, vector < vec2d > > m_FixUWMap; // Vector UW coordinates for FeaFixPoints
    map < int, vector < int > > m_FixPntFeaPartIndexMap; // Vector of FixPoint FeaPart indexes
    map < int, vector < int > > m_FixPntBorderFlagMap; // Indicates if the FixPoint lies on a surface, border, or intersection
    map < int, vector < vector < int > > > m_FixPntSurfIndMap; // Vector of FeaFixPoint parent surface index, corresponding to index in m_SurfVec (Note: not the surf ID)
    map < int, vector < bool > > m_FixPointMassFlagMap;
    map < int, vector < double > > m_FixPointMassMap;

    vector < string > m_DrawBrowserNameVec;
    vector < int > m_DrawBrowserPartIndexVec;
    vector < bool > m_DrawElementFlagVec;
    vector < bool > m_FixPointFeaPartFlagVec;
    vector < bool > m_DrawCapFlagVec;

    vector< FeaElement* > m_FeaElementVec;
    vector < SimpleFeaProperty > m_SimplePropertyVec;
    vector < SimpleFeaMaterial > m_SimpleMaterialVec;

    vector< FeaNode* > m_FeaNodeVec;
    vector< vec3d* > m_AllPntVec;
    map< int, vector< int > > m_IndMap;
    vector< int > m_PntShift;

    SimpleFeaMeshSettings m_StructSettings;
    SimpleGridDensity m_FeaGridDensity;

private:

    vector< DrawObj > m_FeaElementDO;
    vector< DrawObj > m_CapFeaElementDO;
    vector< DrawObj > m_FeaNodeDO;
    vector< DrawObj > m_TriOrientationDO;
    vector< DrawObj > m_CapNormDO;
    vector< DrawObj > m_SSFeaElementDO;
    vector< DrawObj > m_SSCapFeaElementDO;
    vector< DrawObj > m_SSFeaNodeDO;
    vector< DrawObj > m_SSTriOrientationDO;
    vector< DrawObj > m_SSCapNormDO;
};

#define FeaMeshMgr FeaMeshMgrSingleton::getInstance()

#endif
