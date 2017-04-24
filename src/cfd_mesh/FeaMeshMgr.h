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
    virtual void SetFixPointSurfaceNodes();

    virtual void BuildFeaMesh();

    virtual void BuildSubSurfIntChains();
    virtual void SubTagTris();
    virtual void Remesh();

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

    virtual bool GetFeaMeshInProgress()
    {
        return m_FeaMeshInProgress;
    }
    virtual void SetFeaMeshInProgress( bool progress_flag )
    {
        m_FeaMeshInProgress = progress_flag;
    }

    virtual void UpdateDrawObjData();

    virtual void LoadDrawObjs( vector< DrawObj* > & draw_obj_vec );

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

protected:

    bool m_FeaMeshInProgress;

    double m_TotalMass;

    FeaStructure* m_FeaMeshStruct;
    int m_FeaMeshStructIndex;

    int m_NumFeaParts;
    int m_NumFeaSubSurfs;

    vector < vec3d > m_FixPntVec; // Vector 3D coordinates for FeaFixPoints 
    vector < vec2d > m_FixUWVec; // Vector UW coordinates for FeaFixPoints
    vector < int > m_FixPntFeaPartIndexVec; // Vector of FixPoint FeaPart indexes
    vector < vector < int > > m_FixPntSurfIndVec; // Vector of FeaFixPoint parent surface index, corresponding to index in m_SurfVec

    vector < string > m_DrawBrowserNameVec;
    vector < int > m_DrawBrowserPartIndexVec;
    vector < bool > m_DrawElementFlagVec;
    vector < bool > m_DrawCapFlagVec;

    vector< FeaElement* > m_FeaElementVec;

    vector< FeaNode* > m_FeaNodeVec;
    vector< vec3d* > m_AllPntVec;
    map< int, vector< int > > m_IndMap;
    vector< int > m_PntShift;

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
    DrawObj m_MeshBadEdgeDO;
    DrawObj m_MeshBadTriDO;
};

#define FeaMeshMgr FeaMeshMgrSingleton::getInstance()

#endif
