//
// Created by Rob McDonald on 9/13/22.
//

#ifndef VSP_TOP_FEAMESH_H
#define VSP_TOP_FEAMESH_H

#include "FeaElement.h"
#include "SimpleMeshSettings.h"
#include "SimpleSubSurface.h"
#include "SimpleBC.h"

void CloseNASTRAN( FILE *dat_fp, FILE *bdf_header_fp, FILE *bdf_fp, FILE *nkey_fp );

class PartTrim
{
public:
    bool CullPtByTrimGroup( const vec3d &pt, int isymm, double tol );


    // Groups of trimming planes.
    vector < vec3d > m_TrimPt;
    vector < vec3d > m_TrimNorm;
    int m_TrimSymm;
};

class FixPoint
{
public:
    vector < vec3d > m_Pnt;              // Vector 3D coordinates for FeaFixPoints
    bool m_OnBody;
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
    FeaMesh( const string & struct_id );
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
    virtual void WriteNASTRAN( FILE *dat_fp, FILE *bdf_header_fp, FILE *bdf_fp, FILE *nkey_fp );
    virtual void WriteNASTRANHeader( FILE* dat_fp );
    virtual void WriteNASTRANNodes( FILE* dat_fp, FILE* bdf_fp, FILE* nkey_fp, int &set_cnt );
    virtual void WriteNASTRANSPC1( FILE *bdf_fp );
    virtual void WriteNASTRANElements( FILE* dat_fp, FILE* bdf_fp, FILE* nkey_fp, int &set_cnt );

    virtual void WriteGmsh();
    virtual void WriteGmsh( FILE* fp );
    static void WriteGmshHeader( FILE* fp );
    virtual void WriteGmshNames( FILE* fp );
    virtual void WriteGmshNodes( FILE* fp );
    virtual void WriteGmshElements( FILE* fp, int & ele_cnt );

    virtual void WriteSTL();
    virtual void WriteSTL( FILE* fp );

    // Was protected.
    virtual void WriteNASTRANSet( FILE* dat_fp, FILE* nkey_fp, int & set_num, vector < long long int > set_ids, const string &set_name, const long long int &offset );

    virtual void ComputeWriteMass();
    virtual void ComputeWriteMass( FILE* fp );

    virtual FixPoint* GetFixPointByID( const string &id );

    // This is a duplicate of SurfaceIntersectionMgr::m_SimpleSubSurfaceVec
    // We need this to access the subsurface name and property index post-mesh generation.
    // Copying the full data structure is overkill, but it requires less maintenance.
    vector < SimpleSubSurface > m_SimpleSubSurfaceVec;

    SimpleFeaMeshSettings m_StructSettings;
    SimpleFeaGridDensity m_FeaGridDensity;

    string m_StructName;

    double m_TotalMass;
    string m_MassUnit;

    // Scale factor for OpenVSP model to FEA Output length conversion.
    double m_LenScale;

    bool m_MeshReady;

    unsigned long long int m_NumFeaParts;
    unsigned long long int m_NumFeaFixPoints;
    unsigned long long int m_NumFeaTrimParts;
    unsigned long long int m_NumFeaSubSurfs;

    vector < string > m_FeaPartNameVec;
    vector < string > m_FeaPartIDVec;
    vector < int > m_FeaPartTypeVec;
    vector < int > m_FeaPartNumSurfVec;
    vector < int > m_FeaPartKeepDelShellElementsVec;
    vector < bool > m_FeaPartCreateBeamElementsVec;
    vector < int > m_FeaPartPropertyIndexVec;
    vector < int > m_FeaPartCapPropertyIndexVec;
    vector < int > m_FeaPartNumChainsVec;
    vector < string > m_FeaPartPropertyIDVec;
    vector < string > m_FeaPartCapPropertyIDVec;

    unsigned long long int m_NumNodes;
    unsigned long long int m_NumEls;
    unsigned long long int m_NumTris;
    unsigned long long int m_NumQuads;
    unsigned long long int m_NumBeams;

    vector < string > m_DrawBrowserNameVec;
    vector < int > m_DrawBrowserPartIndexVec;
    vector < bool > m_DrawElementFlagVec;
    vector < bool > m_FixPointFeaPartFlagVec;
    vector < bool > m_DrawCapFlagVec;

    vector< FeaElement* > m_FeaElementVec;

    vector < FeaNode* > m_FeaOffBodyFixPointNodeVec;

    vector < FeaNode* > m_FeaNodeVec;
    vector < bool > m_FeaNodeVecUsed;

    vector < FixPoint > m_FixPntVec; // Fix point data map.

    // Groups of trimming planes.
    vector < PartTrim > m_TrimVec;

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
    DrawObj m_BCNodeDO;


private:
    string m_ID;

};



#endif //VSP_TOP_FEAMESH_H
