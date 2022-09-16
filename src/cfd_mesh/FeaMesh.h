//
// Created by Rob McDonald on 9/13/22.
//

#ifndef VSP_TOP_FEAMESH_H
#define VSP_TOP_FEAMESH_H

#include "FeaElement.h"
#include "SimpleMeshSettings.h"
#include "SimpleSubSurface.h"

class FixPoint
{
public:
    vector < vec3d > m_Pnt;              // Vector 3D coordinates for FeaFixPoints
    vector < vec2d > m_UW;               // Vector UW coordinates for FeaFixPoints
    vector < int > m_FeaPartIndex;       // Vector of FixPoint FeaPart indexes
    vector < int > m_BorderFlag;         // Indicates if the FixPoint lies on a surface, border, or intersection
    vector < vector < int > > m_SurfInd; // Vector of FeaFixPoint parent surface index, corresponding to index in m_SurfVec (Note: not the surf ID)
    vector < bool > m_PtMassFlag;
    vector < double > m_PtMass;
};

class FeaMesh
{
public:
    FeaMesh();

    void Cleanup();

    string GetID()   { return m_ID; }

    virtual SimpleGridDensity* GetGridDensityPtr()
    {
        if ( m_FeaGridDensityPtr )
        {
            return m_FeaGridDensityPtr;
        }
        else
        {
            assert( true );
            return NULL;
        }
    }

    virtual SimpleFeaMeshSettings* GetStructSettingsPtr()
    {
        return m_StructSettingsPtr;
    }


    virtual bool FeaDataAvailable();
    virtual void SetAllDisplayFlags( bool flag );
    virtual void UpdateDrawObjs();
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

    virtual void WriteCalculix( );
    virtual void WriteNASTRAN( const string &base_filename );
    virtual void WriteGmsh();

    // Was protected.
    virtual void WriteNASTRANSet( FILE* Nastran_fid, FILE* NKey_fid, int & set_num, vector < int > set_ids, const string &set_name, const int &offset );

    virtual void ComputeWriteMass();

    // This is a duplicate of SurfaceIntersectionMgr::m_SimpleSubSurfaceVec
    // We need this to access the subsurface name and property index post-mesh generation.
    // Copying the full data structure is overkill, but it requires less maintenance.
    vector < SimpleSubSurface > m_SimpleSubSurfaceVec;

    vector < SimpleFeaProperty > m_SimplePropertyVec;
    vector < SimpleFeaMaterial > m_SimpleMaterialVec;

    SimpleFeaMeshSettings* m_StructSettingsPtr;
    SimpleGridDensity* m_FeaGridDensityPtr;

    string m_StructName;

    bool m_QuadMesh;
    bool m_HighOrder;

    double m_TotalMass;
    string m_MassUnit;

    unsigned int m_NumFeaParts;
    unsigned int m_NumFeaFixPoints;
    unsigned int m_NumFeaSubSurfs;

    vector < string > m_FeaPartNameVec;
    vector < int > m_FeaPartTypeVec;
    vector < int > m_FeaPartNumSurfVec;
    vector < int > m_FeaPartIncludedElementsVec;
    vector < int > m_FeaPartPropertyIndexVec;
    vector < int > m_FeaPartCapPropertyIndexVec;

    unsigned int m_NumEls;
    unsigned int m_NumTris;
    unsigned int m_NumQuads;
    unsigned int m_NumBeams;

    int m_NodeOffset;
    int m_ElementOffset;

    vector < string > m_DrawBrowserNameVec;
    vector < int > m_DrawBrowserPartIndexVec;
    vector < bool > m_DrawElementFlagVec;
    vector < bool > m_FixPointFeaPartFlagVec;
    vector < bool > m_DrawCapFlagVec;

    vector< FeaElement* > m_FeaElementVec;


    vector< FeaNode* > m_FeaNodeVec;
    vector< vec3d* > m_AllPntVec;
    map< int, vector< int > > m_IndMap;
    vector< int > m_PntShift;

    vector < FixPoint > m_FixPntVec; // Fix point data map.

    // m_PartSurfOrientation[i][j] == GetFeaSurf(i,j)->GetFeaElementOrientation()
    vector < vector < vec3d > > m_PartSurfOrientation;


// Was private.

    vector< DrawObj > m_FeaTriElementDO;
    vector< DrawObj > m_FeaQuadElementDO;
    vector< DrawObj > m_CapFeaElementDO;
    vector< DrawObj > m_FeaNodeDO;
    vector< DrawObj > m_ElOrientationDO;
    vector< DrawObj > m_CapNormDO;
    vector< DrawObj > m_SSTriElementDO;
    vector< DrawObj > m_SSQuadElementDO;
    vector< DrawObj > m_SSCapFeaElementDO;
    vector< DrawObj > m_SSFeaNodeDO;
    vector< DrawObj > m_SSElOrientationDO;
    vector< DrawObj > m_SSCapNormDO;


private:
    string m_ID;

};



#endif //VSP_TOP_FEAMESH_H
