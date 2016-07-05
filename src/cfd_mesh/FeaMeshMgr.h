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
#include "FeaPart.h"

class SectionEdge
{
public:
    SectionEdge()                   {}
    virtual ~SectionEdge()          {}

    vector< double > m_LineFractVec;
    vector< vec2d  > m_UWVec;
    vector< vec3d  > m_PntVec;
};

class WingSection
{
public:

    WingSection();
    virtual ~WingSection();

    void BuildClean();
    void Load( Surf* upper_surf, Surf* lower_surf );

    void WriteData( xmlNodePtr root );
    void ReadData(  xmlNodePtr root );


    vec2d GetUW( int edge_id, double fract );
    vec3d CompPnt( int edge_id, double fract );
    bool IntersectPlaneEdge( int edge_id, vec3d & orig, vec3d & norm, vec2d & result );
    void ComputePerSpanChord( vec3d & pnt, double* per_span, double* per_chord );
//  void Draw( bool highlight );

    Surf* m_UpperSurfPtr;
    Surf* m_LowerSurfPtr;

    vec3d m_Normal;
    vec3d m_ChordNormal;
    double m_SweepLE;

    enum { UW00, UW10, UW01, UW11, NUM_CORNER_PNTS };
    vec3d m_CornerPnts[NUM_CORNER_PNTS];

    enum { LE, TE, IN_CHORD, OUT_CHORD, NUM_EDGES };
    SectionEdge m_Edges[NUM_EDGES];

    vector< FeaRib* > m_RibVec;
    vector< FeaSpar* > m_SparVec;

    FeaSkin m_UpperSkin;
    FeaSkin m_LowerSkin;


};

//////////////////////////////////////////////////////////////////////
class FeaMeshMgrSingleton : public CfdMeshMgrSingleton
{
protected:
    FeaMeshMgrSingleton();
    FeaMeshMgrSingleton( FeaMeshMgrSingleton const& copy );          // Not Implemented
    FeaMeshMgrSingleton& operator=( FeaMeshMgrSingleton const& copy ); // Not Implemented

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

    StructSettings* GetStructSettingsPtr()
    {
        return m_Vehicle->GetStructSettingsPtr();
    }

    virtual bool LoadSurfaces();
    virtual void Build();
    virtual void Export();
    virtual void BuildClean();
    virtual void Intersect();
    virtual void IdentifyUpperLowerSurfaces();

    virtual void AddStructureParts();
    virtual void RemoveSliceSurfaces();

    virtual bool FetchWingSurf( vector< XferSurf > &xfersurfs );

    virtual vec3d ComputePoint( vec2d & uw, bool upperFlag );
    virtual vec3d GetNormal( int sectID );
    virtual WingSection* GetWingSection( int sectID );

    virtual int GetNumSections()
    {
        return m_UpperSurfVec.size();
    }

    virtual void LoadChains( Surf* sliceSurf, bool upperFlag, int sect_id,
                             list< ISegChain* > & chain_list );
    virtual void LoadCapChains( Surf* s0, Surf* s1, list< ISegChain* > & chain_list );
    virtual void LoadCapChains( Surf* s0, double w, list< ISegChain* > & chain_list );

    virtual void BuildSliceMesh();

    virtual void WriteCalculix( );
    virtual void WriteNASTRAN( const string &base_filename );

    virtual void ComputeWriteMass();

//  virtual void Draw();

    virtual double GetTotalMass()
    {
        return m_TotalMass;
    }

    virtual int GetCurrSectID()
    {
        return m_CurrSectID;
    }
    virtual void SetCurrSectID( int id );

    virtual int GetNumSpars();
    virtual int GetCurrSparID()
    {
        return m_CurrSparID;
    }
    virtual void SetCurrSparID( int id );

    virtual int GetNumRibs();
    virtual int GetCurrRibID()
    {
        return m_CurrRibID;
    }
    virtual void SetCurrRibID( int id );

    virtual int GetNumPointMasses();
    virtual int GetCurrPointMassID()
    {
        return m_CurrPointMassID;
    }
    virtual void SetCurrPointMassID( int id );

    //virtual int GetNumUpSkinSpliceLines();
    //virtual int GetCurrUpSkinSliceLineID()        { return m_CurrUpSkinSpliceLineID; }
    //virtual void SetCurrUpSkinSpliceLineID( int id );

    virtual FeaRib* GetCurrRib();
    virtual FeaSpar* GetCurrSpar();
    virtual FeaSkin* GetCurrUpperSkin();
    virtual FeaSkin* GetCurrLowerSkin();
    virtual FeaPointMass* GetCurrPointMass();

    virtual FeaNode* FindNode( vector< FeaNode* > nodeVec, int id );

    virtual void AddRib();
    virtual void DelCurrRib();
    virtual void AddSpar();
    virtual void DelCurrSpar();
    virtual void AddPointMass();
    virtual void DelCurrPointMass();

    enum { UP_SKIN_EDIT, LOW_SKIN_EDIT, SPAR_EDIT, RIB_EDIT, POINT_MASS_EDIT };
    virtual void SetCurrEditType( int t )
    {
        m_CurrEditType = t;
    }
    virtual int  GetCurrEditType()
    {
        return m_CurrEditType;
    }

    virtual void SetDrawMeshFlag( bool f )
    {
        m_DrawMeshFlag = f;
    }
    virtual void SetDrawFlag( bool f )
    {
        m_DrawFlag = f;
    }
    virtual void SetDrawAttachPointsFlag( bool f )
    {
        m_DrawAttachPoints = f;
    }
    virtual bool GetDrawAttachPointsFlag()
    {
        return m_DrawAttachPoints;
    }

    virtual void SaveData();
    virtual void WriteFeaStructData( Geom* geom_ptr, xmlNodePtr root );
    virtual void SetFeaStructData( Geom* geom_ptr, xmlNodePtr root );
    virtual void ReadFeaStructData( );
    virtual void CopyGeomPtr( Geom* from_geom, Geom* to_geom );
    virtual void LoadAttachPoints();

    virtual void CursorPos( vec2d & cursor );
    virtual void MouseClick( vec2d & cursor );

    enum { MASS_FILE_NAME, NASTRAN_FILE_NAME, GEOM_FILE_NAME, THICK_FILE_NAME, STL_FEA_NAME, NUM_FEA_FILE_NAMES };
    string GetFeaExportFileName( int type );
    void SetFeaExportFileName( const string &fn, int type );
    bool GetFeaExportFileFlag( int type );
    void SetFeaExportFileFlag( bool flag, int type );
    void ResetFeaExportFileNames();

    Parm m_ThickScale;

protected:

    Geom* m_WingGeom;

    bool m_DrawFlag;
    bool m_DrawMeshFlag;

    xmlNodePtr m_XmlDataNode;
    vector< Geom* > m_DataGeomVec;

    int m_CurrSectID;

    double m_TotalMass;

    int m_CurrSparID;
    int m_CurrRibID;
    int m_CurrPointMassID;

    int m_CurrEditType;

    vector< Surf* > m_UpperSurfVec;
    vector< Surf* > m_LowerSurfVec;

    vector< WingSection > m_WingSections;

    vector< FeaSlice* > m_SliceVec;
    vector< FeaSkin* > m_SkinVec;

    bool m_DrawAttachPoints;
    int m_ClosestAttachPoint;
    vector< vec3d > m_AttachPoints;

    vector< FeaPointMass* > m_PointMassVec;

    bool m_ExportFeaFileFlags[NUM_FEA_FILE_NAMES];
    string m_ExportFeaFileNames[NUM_FEA_FILE_NAMES];


    vector< vec3d >debugPnts;

private:
    DrawObj m_SkinElemDO;
    DrawObj m_SliceElemDO;

    CfdMeshSettings* GetCfdSettingsPtr()         { return NULL; }

};
//FEAMesh:
//feamass.dat        -> [modelname]_mass.dat
//feaNASTRAN.dat -> [modelname]_NASTRAN.dat
//feageom.dat        -> [modelname]_calculix_geom.dat
//feanodethick.dat ->  [modelname]_calculix_thick.dat

#define FeaMeshMgr FeaMeshMgrSingleton::getInstance()

#endif
