//
// Created by Rob McDonald on 9/13/22.
//

#ifndef VSP_TOP_FEAMESH_H
#define VSP_TOP_FEAMESH_H

#include "FeaElement.h"
#include "SimpleMeshSettings.h"
#include "SimpleSubSurface.h"
#include "SimpleBC.h"

class FixPoint
{
public:
    vector < vec3d > m_Pnt;              // Vector 3D coordinates for FeaFixPoints
    vec2d m_UW;                          // UW coordinates for FeaFixPoints
    int m_FeaPartIndex;                  // FixPoint FeaPart index
    vector < int > m_BorderFlag;         // Indicates if the FixPoint lies on a surface, border, or intersection
    vector < vector < int > > m_SurfInd; // Vector of FeaFixPoint parent surface index, corresponding to index in m_SurfVec (Note: not the surf ID)
    bool m_PtMassFlag;
    double m_PtMass;
    vector < int > m_NodeIndex;          // Index of node in mesh.
};

class FeaMesh
{
public:
    FeaMesh( string & struct_id );
    ~FeaMesh();

    void Cleanup();

    string GetID()   { return m_ID; }

    virtual SimpleGridDensity* GetGridDensityPtr()
    {
        return &m_FeaGridDensity;
    }

    virtual SimpleFeaMeshSettings* GetStructSettingsPtr()
    {
        return &m_StructSettings;
    }


    virtual bool FeaDataAvailable();
    virtual void SetAllDisplayFlags( bool flag );
    virtual void UpdateDrawObjs();
    virtual void LoadDrawObjs( vector< DrawObj* > & draw_obj_vec, SimpleFeaMeshSettings* st_settings );

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

    virtual void ExportFeaMesh();

    virtual void WriteCalculix();
    virtual void WriteCalculix( FILE* fp );
    virtual void WriteCalculixHeader( FILE* fp );
    virtual void WriteCalculixNodes( FILE* fp );
    virtual void WriteCalculixElements( FILE* fp );
    virtual void WriteCalculixBCs( FILE* fp );
    virtual void WriteCalculixProperties( FILE* fp );

    virtual void WriteNASTRAN();
    virtual void WriteNASTRAN( FILE* fp, FILE* temp, FILE* nkey_fp );
    virtual void WriteNASTRANHeader( FILE* fp );
    virtual void WriteNASTRANNodes( FILE* fp, FILE* temp, FILE* nkey_fp, int &set_cnt, int &max_grid_id );
    virtual void WriteNASTRANElements( FILE* fp, FILE* temp, FILE* nkey_fp, int &set_cnt, int &elem_id );
    virtual void CloseNASTRAN( FILE* fp, FILE* temp, FILE* nkey_fp );

    virtual void WriteGmsh();
    virtual void WriteSTL();

    // Was protected.
    virtual void WriteNASTRANSet( FILE* Nastran_fid, FILE* NKey_fid, int & set_num, vector < int > set_ids, const string &set_name, const int &offset );

    virtual void ComputeWriteMass();

    virtual FixPoint* GetFixPointByID( const string &id );

    // This is a duplicate of SurfaceIntersectionMgr::m_SimpleSubSurfaceVec
    // We need this to access the subsurface name and property index post-mesh generation.
    // Copying the full data structure is overkill, but it requires less maintenance.
    vector < SimpleSubSurface > m_SimpleSubSurfaceVec;

    SimpleFeaMeshSettings m_StructSettings;
    SimpleGridDensity m_FeaGridDensity;

    string m_StructName;

    double m_TotalMass;
    string m_MassUnit;

    bool m_MeshReady;

    unsigned int m_NumFeaParts;
    unsigned int m_NumFeaFixPoints;
    unsigned int m_NumFeaSubSurfs;

    vector < string > m_FeaPartNameVec;
    vector < string > m_FeaPartIDVec;
    vector < int > m_FeaPartTypeVec;
    vector < int > m_FeaPartNumSurfVec;
    vector < int > m_FeaPartIncludedElementsVec;
    vector < int > m_FeaPartPropertyIndexVec;
    vector < int > m_FeaPartCapPropertyIndexVec;

    unsigned int m_NumNodes;
    unsigned int m_NumEls;
    unsigned int m_NumTris;
    unsigned int m_NumQuads;
    unsigned int m_NumBeams;

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

    // Groups of trimming planes.
    vector < vector < vec3d > > m_TrimPt;
    vector < vector < vec3d > > m_TrimNorm;

    vector < SimpleBC > m_BCVec;

    // m_PartSurfOrientation[i][j] == GetFeaSurf(i,j)->GetFeaElementOrientation()
    vector < vector < vec3d > > m_PartSurfOrientation;


// Was private.
    bool m_MeshDOUpToDate;
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
