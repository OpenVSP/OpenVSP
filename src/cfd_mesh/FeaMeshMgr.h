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

    virtual void BuildFeaMesh();

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

    virtual FeaNode* FindNode( vector< FeaNode* > nodeVec, int id );

    virtual void SetFeaMeshStructIndex( int index )
    {
        m_FeaMeshStructIndex = index;
    }

    virtual void SetDrawMeshFlag( bool f )
    {
        m_DrawMeshFlag = f;
    }

    virtual bool GetFeaMeshInProgress()
    {
        return m_FeaMeshInProgress;
    }
    virtual void SetFeaMeshInProgress( bool progress_flag )
    {
        m_FeaMeshInProgress = progress_flag;
    }

    virtual void LoadDrawObjs( vector< DrawObj* > & draw_obj_vec );


protected:

    bool m_DrawMeshFlag;

    bool m_FeaMeshInProgress;

    double m_TotalMass;

    FeaStructure* m_FeaMeshStruct;
    int m_FeaMeshStructIndex;

    int m_NumFeaParts;

    vector< FeaElement* > m_FeaElementVec;

    vector< FeaNode* > m_FeaNodeVec;
    vector< vec3d* > m_AllPntVec;
    map< int, vector< int > > m_IndMap;
    vector< int > m_PntShift;

private:

    vector< DrawObj > m_FeaElementDO;
    vector< DrawObj > m_FeaNodeDO;
};

#define FeaMeshMgr FeaMeshMgrSingleton::getInstance()

#endif
