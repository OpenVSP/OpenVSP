//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//

// FeaStructure.h
//
// Justin Gravett
//******************************************************************************
//   Derived from FEA Part Class
//   J.R. Gloudemans - 2/14/09
//******************************************************************************
//////////////////////////////////////////////////////////////////////

#ifndef FEASTRUCTURE_INCLUDED_
#define FEASTRUCTURE_INCLUDED_

#include "VspSurf.h"
#include "Vec3d.h"
#include "Parm.h"
#include "ParmContainer.h"
#include "DrawObj.h"
#include "APIDefines.h"
#include "SubSurface.h"
#include "MeshCommonSettings.h"
#include "GridDensity.h"
#include "BitMask.h"

#define FEA_PART_EXPANSION_FACTOR 1e-4

// Forward declaration
class FeaPart;
class FeaBC;

class FeaStructure : public ParmContainer
{
public:

    FeaStructure( const string& GeomID, int surf_index );
    virtual ~FeaStructure();

    virtual void Update();

    virtual void ParmChanged( Parm* parm_ptr, int type );

    virtual xmlNodePtr EncodeXml( xmlNodePtr & node );
    virtual xmlNodePtr DecodeXml( xmlNodePtr & node );

    virtual void AddLinkableParms( vector< string > & linkable_parm_vec, const string & link_container_id = string() );
    void SetDrawFlag( bool flag );

    void ReSuffixGroupNames();

    void AddFeaPart( FeaPart* fea_part )
    {
        m_FeaPartVec.push_back( fea_part );
    }
    FeaPart* AddFeaPart( int type );
    bool ValidFeaPartInd( int ind );
    void DelFeaPart( int ind );
    void ReorderFeaPart( int ind, int action );
    void UpdateFeaParts();
    FeaPart* GetFeaPart( int ind );

    int GetFeaPartIndex( const string &id );

    string GetFeaPartName( int ind );
    vector< FeaPart* > GetFeaPartVec()
    {
        return m_FeaPartVec;
    }
    int GetFeaPartIndex( FeaPart* fea_part );
    int NumFeaParts()
    {
        return m_FeaPartVec.size();
    }

    virtual void FetchAllTrimPlanes( vector < vector < vec3d > > &pt, vector < vector < vec3d > > &norm );

    vector< FeaPart* > GetFeaPartVecType( int type );

    bool FeaPartIsFixPoint( int ind );
    int GetNumFeaFixPoints();

    bool FeaPartIsArray( int ind );

    bool FeaPartIsSkin( int ind );
    bool FeaPartIsTrim( int ind );
    bool FeaPartIsType( int ind, int type );

    void IndividualizeRibArray( int rib_array_ind );
    void IndividualizeSliceArray( int slice_array_ind );
    void IndividualizeSSLineArray( int ssline_array_ind );

    vector< FeaPart* > InitFeaSkin( );
    FeaPart* GetFeaSkin();

    void UpdateFeaSubSurfs();
    void HighlightFeaParts( vector < int > active_ind_vec );
    void RecolorFeaSubSurfs( vector < int > active_ind_vec );
    SubSurface* AddFeaSubSurf( int type );
    bool ValidFeaSubSurfInd( int ind );
    void DelFeaSubSurf( int ind );
    SubSurface* GetFeaSubSurf( int ind );
    SubSurface* GetFeaSubSurf( const string &id );
    void ReorderFeaSubSurf( int ind, int action );
    int NumFeaSubSurfs()
    {
        return m_FeaSubSurfVec.size();
    }
    vector< SubSurface* > GetFeaSubSurfVec()
    {
        return m_FeaSubSurfVec;
    }
    void AddFeaSubSurf( SubSurface* sub_surf )
    {
        m_FeaSubSurfVec.push_back( sub_surf );
    }

    string GetParentGeomID()
    {
        return m_ParentGeomID;
    }

    int GetFeaStructMainSurfIndx()
    {
        return m_MainSurfIndx;
    }

    void ResetExportFileNames();

    StructSettings* GetStructSettingsPtr()
    {
        return &m_StructSettings;
    }
    GridDensity* GetFeaGridDensityPtr()
    {
        return &m_FeaGridDensity;
    }

    void BuildSuppressList();

    vector < double > GetUSuppress()
    {
        return m_Usuppress;
    }
    vector < double > GetWSuppress()
    {
        return m_Wsuppress;
    }

    bool PtsOnAnyPlanarPart( const vector < vec3d > &pnts );

    // Boundary Condition Stuff
    void AddFeaBC( FeaBC* fea_bc )
    {
        m_FeaBCVec.push_back( fea_bc );
    }

    FeaBC* AddFeaBC( int type = -1 );

    void DelFeaBC( int ind );

    bool ValidFeaBCInd( int ind );

    FeaBC* GetFeaBC( int ind );

    int GetFeaBCIndex( const string &id );

    int GetFeaBCIndex( FeaBC* fea_bc );

    vector< FeaBC* > GetFeaBCVec()
    {
        return m_FeaBCVec;
    }

    int NumFeaBCs()
    {
        return m_FeaBCVec.size();
    }
    void UpdateFeaBCs();

protected:

    string m_ParentGeomID;
    int m_MainSurfIndx;
    int m_FeaPartCount; // Counter used for creating unique name for parts
    int m_FeaSubSurfCount; // Counter used for creating unique name for subsurfaces

    vector < double > m_Usuppress;
    vector < double > m_Wsuppress;

    vector < FeaPart* > m_FeaPartVec;

    vector < SubSurface* > m_FeaSubSurfVec;

    vector < FeaBC* > m_FeaBCVec;

    StructSettings m_StructSettings;
    FeaGridDensity m_FeaGridDensity;

};

class FeaPart : public ParmContainer
{
public:

    FeaPart( const string &geomID, const string &structID, int type );
    virtual ~FeaPart();

    virtual void Update();
    virtual void UpdateFlags();
    virtual void UpdateSurface() = 0;
    virtual void UpdateOrientation();

    virtual void ParmChanged( Parm* parm_ptr, int type );

    virtual xmlNodePtr EncodeXml( xmlNodePtr & node );
    virtual xmlNodePtr DecodeXml( xmlNodePtr & node );

    virtual void SetDisplaySuffix( int num );

    virtual void UpdateSymmParts();

    static string GetTypeName( int type );

    virtual bool RefFrameIsBody( int orientation_plane );

    virtual void FetchFeaXFerSurf( vector< XferSurf > &xfersurfs, int compid, const vector < double > &usuppress = std::vector< double >(), const vector < double > &wsuppress = std::vector < double >() );

    virtual void LoadDrawObjs( std::vector< DrawObj* > & draw_obj_vec );
    virtual void UpdateDrawObjs();
    virtual void SetDrawObjHighlight ( bool highlight );

    virtual int GetType()
    {
        return m_FeaPartType;
    }

    virtual string GetParentGeomID()
    {
        return m_ParentGeomID;
    }

    virtual vector < VspSurf > GetFeaPartSurfVec()
    {
        return m_FeaPartSurfVec;
    }
    virtual void AddFeaPartSurf( VspSurf fea_surf )
    {
        m_FeaPartSurfVec.push_back( fea_surf );
    }
    virtual int NumFeaPartSurfs()
    {
        return m_FeaPartSurfVec.size();
    }
    virtual void DeleteFeaPartSurf( int ind );

    VspSurf* GetMainSurf();

    virtual bool PtsOnPlanarPart( const vector < vec3d > & pnts, double minlen, int surf_ind = 0 );

    int m_MainSurfIndx;
    IntParm m_IncludedElements; // Deprecated
    BoolParm m_CreateBeamElements;
    IntParm m_KeepDelShellElements;
    BoolParm m_DrawFeaPartFlag;
    IntParm m_AbsRelParmFlag;
    Parm m_AbsCenterLocation;
    Parm m_RelCenterLocation;
    IntParm m_FeaPropertyIndex;
    IntParm m_CapFeaPropertyIndex;
    IntParm m_OrientationType;

    string m_FeaPropertyID;
    string m_CapFeaPropertyID;

protected:

    int m_FeaPartType;

    string m_ParentGeomID;
    string m_StructID;

    vector < int > m_SymmIndexVec;

    vector < VspSurf > m_FeaPartSurfVec;
    vector < VspSurf > m_MainFeaPartSurfVec;

    vector < DrawObj > m_FeaPartDO;
    vector < DrawObj > m_FeaHighlightDO;
};

class FeaSlice : public FeaPart
{
public:

    FeaSlice( const string &geomID, const string &structID, int type = vsp::FEA_SLICE );
    virtual ~FeaSlice()    {};

    virtual void UpdateSurface();
    virtual void UpdateParmLimits();

    virtual VspSurf ComputeSliceSurf();

    virtual void SetSectionBBox( BndBox box )
    {
        m_SectBBox = box;
    }

    virtual void SetCenter( vec3d center )
    {
        m_Center = center;
    }

    IntParm m_OrientationPlane;
    IntParm m_RotationAxis;
    Parm m_XRot;
    Parm m_YRot;
    Parm m_ZRot;


protected:

    BndBox m_SectBBox;
    vec3d m_Center;
};

class FeaSpar : public FeaSlice
{
public:

    FeaSpar( const string &geomID, const string &structID, int type = vsp::FEA_SPAR );
    virtual ~FeaSpar()    {};

    virtual void UpdateSurface();
    virtual void UpdateParms();

    virtual void ComputePlanarSurf();

    Parm m_Theta;
    BoolParm m_LimitSparToSectionFlag;
    IntParm m_StartWingSection;
    IntParm m_EndWingSection;
    BoolParm m_BndBoxTrimFlag;
    BoolParm m_UsePercentChord;
    Parm m_PercentRootChord;
    Parm m_PercentTipChord;

private:

    double m_U_sec_min, m_U_sec_max;

};

class FeaRib : public FeaSlice
{
public:

    FeaRib( const string &geomID, const string &structID, int type = vsp::FEA_RIB );
    virtual ~FeaRib()    {};

    virtual void UpdateSurface();

    virtual void UpdateParmLimits();

    virtual xmlNodePtr EncodeXml( xmlNodePtr & node );
    virtual xmlNodePtr DecodeXml( xmlNodePtr & node );

    double GetRibPerU();
    double GetRibTotalRotation();
    VspSurf ComputeRibSurf();

    void SetPerpendicularEdgeID( const string & ID )
    {
        m_PerpendicularEdgeID = ID;
    }
    string GetPerpendicularEdgeID()
    {
        return m_PerpendicularEdgeID;
    }

    Parm m_Theta;
    BoolParm m_LimitRibToSectionFlag;
    IntParm m_StartWingSection;
    IntParm m_EndWingSection;
    BoolParm m_BndBoxTrimFlag;
    IntParm m_PerpendicularEdgeType;
    BoolParm m_MatchDihedralFlag;

protected:

    string m_PerpendicularEdgeID;
    double m_PerU;
    double m_TotRot;
    double m_U_sec_min, m_U_sec_max;
    vec3d m_WingNorm;
};

class FeaFixPoint : public FeaPart
{
public:

    FeaFixPoint( const string &geomID, const string &structID, const string &partID, int type = vsp::FEA_FIX_POINT );
    virtual ~FeaFixPoint()    {};

    virtual void UpdateSurface();
    vector < vec3d > GetPntVec(); // Returns the FeaFixPoint 3D coordinate on each parent surface
    vec2d GetUW(); // Returns the FeaFixPoint UW coordinate on main parent surface

    virtual xmlNodePtr EncodeXml( xmlNodePtr & node );
    virtual xmlNodePtr DecodeXml( xmlNodePtr & node );

    virtual void UpdateDrawObjs();
    virtual void SetDrawObjHighlight ( bool highlight );

    virtual bool PtsOnPlanarPart( const vector < vec3d > & pnts, double minlen, int surf_ind = 0 );

    virtual int NumFeaPartSurfs();

    Parm m_PosU;
    Parm m_PosW;
    BoolParm m_FixPointMassFlag;
    Parm m_FixPointMass;

    IntParm m_MassUnit;

    Parm m_FixPointMass_FEM;

    string m_ParentFeaPartID; // Parent FeaPart ID (FeaFixPoint is located on this surface)

protected:

    bool PlaneAtYZero( piecewise_surface_type & surface ) const;
    bool LessThanY( piecewise_surface_type & surface, double val ) const;

};

class FeaPartTrim : public FeaPart
{
public:

    FeaPartTrim( const string &geomID, const string &structID, int type = vsp::FEA_TRIM );
    virtual ~FeaPartTrim();

    virtual void Clear();

    virtual void UpdateSurface();

    virtual xmlNodePtr EncodeXml( xmlNodePtr & node );
    virtual xmlNodePtr DecodeXml( xmlNodePtr & node );

    virtual void UpdateDrawObjs();
    virtual void SetDrawObjHighlight ( bool highlight );

    virtual void FetchTrimPlanes( vector < vector < vec3d > > &pt, vector < vector < vec3d > > &norm );

    virtual bool PtsOnPlanarPart( const vector < vec3d > & pnts, double minlen, int surf_ind = 0 );

    virtual void AddTrimPart( string partID );
    virtual void DeleteTrimPart( int indx );
    virtual void RenameParms();

    vector < BoolParm* > m_FlipFlagVec;
    vector < string > m_TrimFeaPartIDVec;
};

class FeaSkin : public FeaPart
{
public:

    FeaSkin( const string &geomID, const string &structID, int type = vsp::FEA_SKIN );
    virtual ~FeaSkin()    {};

    virtual void UpdateSurface();

    void BuildSkinSurf();

    virtual void UpdateDrawObjs()    {}; // Do nothing for skins

    virtual bool PtsOnPlanarPart( const vector < vec3d > & pnts, double minlen, int surf_ind = 0 );

    BoolParm m_RemoveSkinFlag;

protected:


};

class FeaDome : public FeaPart
{
public:

    FeaDome( const string &geomID, const string &structID, int type = vsp::FEA_DOME );
    virtual ~FeaDome()    {};

    virtual void UpdateSurface();

    void BuildDomeSurf();

    virtual void UpdateDrawObjs();

    virtual bool PtsOnPlanarPart( const vector < vec3d > & pnts, double minlen, int surf_ind = 0 );

    Parm m_Aradius;
    Parm m_Bradius;
    Parm m_Cradius;
    Parm m_XLoc;
    Parm m_YLoc;
    Parm m_ZLoc;
    Parm m_XRot;
    Parm m_YRot;
    Parm m_ZRot;
    BoolParm m_SpineAttachFlag;
    Parm m_USpineLoc;
    BoolParm m_FlipDirectionFlag;

protected:


};

class FeaRibArray : public FeaPart
{
public:

    FeaRibArray( const string &geomID, const string &structID, int type = vsp::FEA_RIB_ARRAY );
    virtual ~FeaRibArray();

    virtual void UpdateSurface();
    void CalcNumRibs();
    void CreateFeaRibArray();

    virtual FeaRib* AddFeaRib( double center_location, int ind );

    virtual xmlNodePtr EncodeXml( xmlNodePtr & node );
    virtual xmlNodePtr DecodeXml( xmlNodePtr & node );

    virtual bool PtsOnPlanarPart( const vector < vec3d > & pnts, double minlen, int surf_ind = 0 );

    int GetNumRibs()
    {
        return m_NumRibs;
    }

    void SetPerpendicularEdgeID( const string & ID )
    {
        m_PerpendicularEdgeID = ID;
    }
    string GetPerpendicularEdgeID()
    {
        return m_PerpendicularEdgeID;
    }

    Parm m_RibAbsSpacing;
    Parm m_RibRelSpacing;
    BoolParm m_PositiveDirectionFlag;
    Parm m_AbsStartLocation;
    Parm m_RelStartLocation;
    Parm m_AbsEndLocation;
    Parm m_RelEndLocation;
    Parm m_Theta;
    BoolParm m_LimitArrayToSectionFlag;
    IntParm m_StartWingSection;
    IntParm m_EndWingSection;
    BoolParm m_BndBoxTrimFlag;
    IntParm m_PerpendicularEdgeType;
    BoolParm m_MatchDihedralFlag;

protected:

    int m_NumRibs;

    string m_PerpendicularEdgeID;
};

class FeaSliceArray : public FeaPart
{
public:

    FeaSliceArray( const string &geomID, const string &structID, int type = vsp::FEA_SLICE_ARRAY );
    virtual ~FeaSliceArray()    {};

    virtual void UpdateSurface();
    void CreateFeaSliceArray();
    void CalcNumSlices();

    virtual FeaSlice* AddFeaSlice( double center_location, int ind );

    virtual bool PtsOnPlanarPart( const vector < vec3d > & pnts, double minlen, int surf_ind = 0 );

    int GetNumSlices()
    {
        return m_NumSlices;
    }

    Parm m_SliceAbsSpacing;
    Parm m_SliceRelSpacing;
    BoolParm m_PositiveDirectionFlag;
    Parm m_AbsStartLocation;
    Parm m_RelStartLocation;
    Parm m_AbsEndLocation;
    Parm m_RelEndLocation;
    IntParm m_OrientationPlane;
    IntParm m_RotationAxis;
    Parm m_XRot;
    Parm m_YRot;
    Parm m_ZRot;

protected:

    int m_NumSlices;

};

class FeaProperty : public ParmContainer
{
public:

    FeaProperty();
    virtual ~FeaProperty();

    virtual void Update();

    virtual xmlNodePtr EncodeXml( xmlNodePtr & node );
    virtual xmlNodePtr DecodeXml( xmlNodePtr & node );

    string GetTypeName( );
    string GetXSecName();

    IntParm m_FeaPropertyType;

    // Units used to specify materials.
    IntParm m_LengthUnit;

    Parm m_Thickness;
    Parm m_CrossSecArea;
    Parm m_Izz;
    Parm m_Iyy;
    Parm m_Izy;
    Parm m_Ixx;
    Parm m_Dim1;
    Parm m_Dim2;
    Parm m_Dim3;
    Parm m_Dim4;
    Parm m_Dim5;
    Parm m_Dim6;
    IntParm m_CrossSectType;
    IntParm m_FeaMaterialIndex;
    string m_FeaMaterialID;


    Parm m_Thickness_FEM;
    Parm m_CrossSecArea_FEM;
    Parm m_Izz_FEM;
    Parm m_Iyy_FEM;
    Parm m_Izy_FEM;
    Parm m_Ixx_FEM;
    Parm m_Dim1_FEM;
    Parm m_Dim2_FEM;
    Parm m_Dim3_FEM;
    Parm m_Dim4_FEM;
    Parm m_Dim5_FEM;
    Parm m_Dim6_FEM;

protected:

};

class FeaLayer : public ParmContainer
{
public:
    FeaLayer( );
    virtual ~FeaLayer();

    virtual void Update();

    virtual xmlNodePtr EncodeXml( xmlNodePtr & node );
    virtual xmlNodePtr DecodeXml( xmlNodePtr & node );

    IntParm m_LengthUnit;

    string m_FeaMaterialID;

    Parm m_Thickness;
    Parm m_Theta;

    Parm m_Thickness_FEM;
};

class FeaMaterial : public ParmContainer
{
public:
    FeaMaterial( );
    virtual ~FeaMaterial();

    virtual void Update();

    virtual void ParmChanged( Parm* parm_ptr, int type );

    virtual xmlNodePtr EncodeXml( xmlNodePtr & node );
    virtual xmlNodePtr DecodeXml( xmlNodePtr & node );

    virtual void MakeMaterial( string id );

    double GetShearModulus();
    double GetShearModulus_FEM();

    void AddLayer( FeaLayer *lay )
    {
        m_LayerVec.push_back( lay );
    }
    FeaLayer* AddLayer();
    FeaLayer* GetCurrLayer();
    bool DeleteLayer( const string &id );
    bool ValidLayerInd( int index );
    FeaLayer* GetFeaLayer( string id );
    int GetCurrLayerIndex();
    void SetCurrLayerIndex( int index );

    vector < FeaLayer* > GetLayerVec()
    {
        return m_LayerVec;
    }
    int NumLayers()
    {
        return m_LayerVec.size();
    }

    Parm m_MassDensity;
    Parm m_ElasticModulus;
    Parm m_PoissonRatio;
    Parm m_ThermalExpanCoeff;

    // Units used to specify materials.
    IntParm m_DensityUnit;
    IntParm m_ModulusUnit;
    IntParm m_TemperatureUnit;

    string m_Description;

    bool m_UserFeaMaterial;

    IntParm m_FeaMaterialType;

    // Orthotropic material properties
    Parm m_E1;
    Parm m_E2;
    Parm m_E3;
    Parm m_nu12;
    Parm m_nu13;
    Parm m_nu23;
    Parm m_G12;
    Parm m_G13;
    Parm m_G23;
    Parm m_A1;
    Parm m_A2;
    Parm m_A3;

    // In FEM output units.
    Parm m_MassDensity_FEM;
    Parm m_ElasticModulus_FEM;
    Parm m_ThermalExpanCoeff_FEM;

    Parm m_E1_FEM;
    Parm m_E2_FEM;
    Parm m_E3_FEM;
    Parm m_G12_FEM;
    Parm m_G13_FEM;
    Parm m_G23_FEM;
    Parm m_A1_FEM;
    Parm m_A2_FEM;
    Parm m_A3_FEM;

    vector < FeaLayer* > m_LayerVec;
    int m_CurrentLayerIndex;
    int m_FeaLayerCount;
};

class FeaConnection : public ParmContainer
{
public:

    FeaConnection();
    virtual ~FeaConnection()    {};

    virtual void Update();

    virtual BitMask GetAsBitMask();

    virtual xmlNodePtr EncodeXml( xmlNodePtr & node );
    virtual xmlNodePtr DecodeXml( xmlNodePtr & node );

    virtual string MakeLabel();
    virtual string MakeName();

    virtual void LoadDrawObjs( std::vector< DrawObj* > & draw_obj_vec );
    virtual void UpdateDrawObjs();
    virtual void SetDrawObjHighlight ( bool highlight );

    string m_StartStructID;
    string m_StartFixPtID;
    IntParm m_StartFixPtSurfIndex;

    string m_EndStructID;
    string m_EndFixPtID;
    IntParm m_EndFixPtSurfIndex;

    IntParm m_ConMode;
    IntParm m_Constraints;

protected:

    DrawObj m_ConnLineDO;
    DrawObj m_ConnPtsDO;

};

class FeaAssembly : public ParmContainer
{
public:

    FeaAssembly();
    ~FeaAssembly();

    virtual void Update();

    virtual xmlNodePtr EncodeXml( xmlNodePtr & node );
    virtual xmlNodePtr DecodeXml( xmlNodePtr & node );

    virtual void AddStructure( const string &id );
    virtual void DelStructure( const string &id );

    virtual void GetAllFixPts( vector< FeaPart* > & fixpts, vector <string> &structids );
    virtual void AddConnection( const string &startid, const string &startstructid, int startindx,
                                const string &endid, const string &endstructid, int endindx );
    virtual void DelConnection( int index );

    virtual void ResetExportFileNames();
    virtual void AddLinkableParms( vector< string > & linkable_parm_vec, const string & link_container_id = string() );


    AssemblySettings m_AssemblySettings;

    vector < string > m_StructIDVec;

    vector < FeaConnection* > m_ConnectionVec;

};

class FeaBC : public ParmContainer
{
public:
    FeaBC( const string &m_StructID );
    ~FeaBC()       {};

    virtual void ParmChanged( Parm* parm_ptr, int type );

    virtual xmlNodePtr EncodeXml( xmlNodePtr & node );
    virtual xmlNodePtr DecodeXml( xmlNodePtr & node );

    virtual string GetDescription();
    virtual string GetDescriptionDOF();

    virtual BitMask GetAsBitMask();

    virtual void SetPartID( const string &id ) { m_PartID = id; }
    virtual string GetPartID()  { return m_PartID; }

    virtual void SetSubSurfID( const string &id ) { m_SubSurfID = id; }
    virtual string GetSubSurfID()  { return m_SubSurfID; }

    virtual void Update();

    IntParm m_FeaBCType;

    IntParm m_ConMode;
    IntParm m_Constraints;

    BoolParm m_XLTFlag;
    BoolParm m_XGTFlag;

    BoolParm m_YLTFlag;
    BoolParm m_YGTFlag;

    BoolParm m_ZLTFlag;
    BoolParm m_ZGTFlag;

    Parm m_XLTVal;
    Parm m_XGTVal;

    Parm m_YLTVal;
    Parm m_YGTVal;

    Parm m_ZLTVal;
    Parm m_ZGTVal;

protected:

    string m_StructID;
    string m_PartID;
    string m_SubSurfID;

};

#endif // !defined(FEASTRUCTURE_INCLUDED_)
