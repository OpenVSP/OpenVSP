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

// Forward declaration
class FeaPart;

class FeaStructure : public ParmContainer
{
public:

    FeaStructure( string GeomID, int surf_index );
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

    bool FeaPartIsFixPoint( int ind );
    int GetNumFeaFixPoints();

    bool FeaPartIsArray( int ind );

    void IndividualizeRibArray( int rib_array_ind );
    void IndividualizeSliceArray( int slice_array_ind );
    void IndividualizeSSLineArray( int ssline_array_ind );

    vector< FeaPart* > InitFeaSkin( );
    FeaPart* GetFeaSkin();

    void UpdateFeaSubSurfs();
    void RecolorFeaSubSurfs( vector < int > active_ind_vec );
    SubSurface* AddFeaSubSurf( int type );
    bool ValidFeaSubSurfInd( int ind );
    void DelFeaSubSurf( int ind );
    SubSurface* GetFeaSubSurf( int ind );
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

    int GetFeaPropertyIndex( int fea_part_ind );
    int GetCapFeaPropertyIndex( int fea_part_ind );

    string GetParentGeomID()
    {
        return m_ParentGeomID;
    }

    int GetFeaStructMainSurfIndx()
    {
        return m_MainSurfIndx;
    }

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

protected:

    string m_ParentGeomID;
    int m_MainSurfIndx;
    int m_FeaPartCount; // Counter used for creating unique name for parts
    int m_FeaSubSurfCount; // Counter used for creating unique name for subsurfaces

    vector < double > m_Usuppress;
    vector < double > m_Wsuppress;

    vector < FeaPart* > m_FeaPartVec;

    vector < SubSurface* > m_FeaSubSurfVec;

    StructSettings m_StructSettings;
    FeaGridDensity m_FeaGridDensity;

};

class FeaPart : public ParmContainer
{
public:

    FeaPart( string geomID, int type );
    virtual ~FeaPart();

    virtual void Update();

    virtual void ParmChanged( Parm* parm_ptr, int type );

    virtual xmlNodePtr EncodeXml( xmlNodePtr & node );
    virtual xmlNodePtr DecodeXml( xmlNodePtr & node );

    virtual void SetDisplaySuffix( int num );

    virtual void UpdateSymmParts();
    virtual void UpdateSymmIndex();

    static string GetTypeName( int type );

    virtual bool RefFrameIsBody( int orientation_plane );

    virtual void FetchFeaXFerSurf( vector< XferSurf > &xfersurfs, int compid, const vector < double > &usuppress = std::vector< double >(), const vector < double > &wsuppress = std::vector < double >() );

    virtual void LoadDrawObjs( std::vector< DrawObj* > & draw_obj_vec );
    virtual void UpdateDrawObjs( int id, bool highlight );

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
    virtual void DeleteFeaPartSurf( int ind );

    virtual int GetFeaMaterialIndex();
    virtual void SetFeaMaterialIndex( int index );

    VspSurf* GetMainSurf();

    virtual bool PtsOnPlanarPart( const vector < vec3d > & pnts, int surf_ind = 0 );

    IntParm m_MainSurfIndx;
    IntParm m_IncludedElements;
    BoolParm m_DrawFeaPartFlag;
    IntParm m_AbsRelParmFlag;
    Parm m_AbsCenterLocation;
    Parm m_RelCenterLocation;
    IntParm m_FeaPropertyIndex;
    IntParm m_CapFeaPropertyIndex;

protected:

    int m_FeaPartType;

    string m_ParentGeomID;

    vector < int > m_SymmIndexVec;

    vector < VspSurf > m_FeaPartSurfVec; 

    vector < DrawObj > m_FeaPartDO;
};

class FeaSlice : public FeaPart
{
public:

    FeaSlice( string geomID, int type = vsp::FEA_SLICE );
    virtual ~FeaSlice()    {};

    virtual void Update();
    virtual void UpdateParmLimits();

    virtual VspSurf ComputeSliceSurf();

    virtual void UpdateDrawObjs( int id, bool highlight );

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

enum
{
    PARALLEL_NONE, PARALLEL_LEAD_EDGE, PARALLEL_TRAIL_EDGE
};

class FeaSpar : public FeaSlice
{
public:

    FeaSpar( string geomID, int type = vsp::FEA_SPAR );
    virtual ~FeaSpar()    {};

    virtual void Update();
    virtual void UpdateParms();

    virtual void ComputePlanarSurf();
    virtual void UpdateDrawObjs( int id, bool highlight );

    Parm m_Theta;
    BoolParm m_LimitSparToSectionFlag;
    IntParm m_StartWingSection;
    IntParm m_EndWingSection;
    BoolParm m_BndBoxTrimFlag;

private:

    double m_U_sec_min, m_U_sec_max;

};

enum
{
    PERPENDICULAR_NONE, PERPENDICULAR_LEAD_EDGE, PERPENDICULAR_TRAIL_EDGE
};

class FeaRib : public FeaSlice
{
public:

    FeaRib( string geomID, int type = vsp::FEA_RIB );
    virtual ~FeaRib()    {};

    virtual void Update();

    virtual void UpdateParmLimits();

    virtual xmlNodePtr EncodeXml( xmlNodePtr & node );
    virtual xmlNodePtr DecodeXml( xmlNodePtr & node );

    double GetRibPerU();
    double GetRibTotalRotation();
    VspSurf ComputeRibSurf();

    virtual void UpdateDrawObjs( int id, bool highlight );

    void SetPerpendicularEdgeID( string ID )
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

    FeaFixPoint( string geomID, string partID, int type = vsp::FEA_FIX_POINT );
    virtual ~FeaFixPoint()    {};

    virtual void Update();
    void IdentifySplitSurfIndex( bool half_mesh_flag, const vector < double > &usuppress, const vector < double > &wsuppress );
    vector < vec3d > GetPntVec(); // Returns the FeaFixPoint 3D coordinate on each parent surface
    vec2d GetUW(); // Returns the FeaFixPoint UW coordinate on main parent surface

    virtual xmlNodePtr EncodeXml( xmlNodePtr & node );
    virtual xmlNodePtr DecodeXml( xmlNodePtr & node );

    virtual void UpdateDrawObjs( int id, bool highlight );

    virtual bool PtsOnPlanarPart( const vector < vec3d > & pnts, int surf_ind = 0 );

    Parm m_PosU;
    Parm m_PosW;
    BoolParm m_FixPointMassFlag;
    Parm m_FixPointMass;

    vector< vector < int > > m_SplitSurfIndex; // Identifies which surface FixPoint lies on after calling FetchFeaXFerSurf for each parent surface
    string m_ParentFeaPartID; // Parent FeaPart ID (FeaFixPoint is located on this surface)
    bool m_BorderFlag;

protected:

    bool PlaneAtYZero( piecewise_surface_type & surface ) const;
    bool LessThanY( piecewise_surface_type & surface, double val ) const;

};

class FeaSkin : public FeaPart
{
public:

    FeaSkin( string geomID, int type = vsp::FEA_SKIN );
    virtual ~FeaSkin()    {};

    virtual void Update();

    void BuildSkinSurf();

    virtual void UpdateDrawObjs( int id, bool highlight )    {}; // Do nothing for skins

    virtual bool PtsOnPlanarPart( const vector < vec3d > & pnts, int surf_ind = 0 );

    BoolParm m_RemoveSkinTrisFlag;

protected:


};

class FeaDome : public FeaPart
{
public:

    FeaDome( string geomID, int type = vsp::FEA_DOME );
    virtual ~FeaDome()    {};

    virtual void Update();

    void BuildDomeSurf();

    virtual void UpdateDrawObjs( int id, bool highlight );

    virtual bool PtsOnPlanarPart( const vector < vec3d > & pnts, int surf_ind = 0 );

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

    FeaRibArray( string geomID, int type = vsp::FEA_RIB_ARRAY );
    virtual ~FeaRibArray();

    virtual void Update();
    void CalcNumRibs();
    void CreateFeaRibArray();

    virtual FeaRib* AddFeaRib( double center_location, int ind );

    virtual xmlNodePtr EncodeXml( xmlNodePtr & node );
    virtual xmlNodePtr DecodeXml( xmlNodePtr & node );

    virtual void UpdateDrawObjs( int id, bool highlight );

    virtual bool PtsOnPlanarPart( const vector < vec3d > & pnts, int surf_ind = 0 );

    int GetNumRibs()
    {
        return m_NumRibs;
    }

    void SetPerpendicularEdgeID( string ID )
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

protected:

    int m_NumRibs;

    string m_PerpendicularEdgeID;
};

class FeaSliceArray : public FeaPart
{
public:

    FeaSliceArray( string geomID, int type = vsp::FEA_SLICE_ARRAY );
    virtual ~FeaSliceArray()    {};

    virtual void Update();
    void CreateFeaSliceArray();
    void CalcNumSlices();

    virtual FeaSlice* AddFeaSlice( double center_location, int ind );

    virtual void UpdateDrawObjs( int id, bool highlight );

    virtual bool PtsOnPlanarPart( const vector < vec3d > & pnts, int surf_ind = 0 );

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

    virtual void ParmChanged( Parm* parm_ptr, int type );

    virtual xmlNodePtr EncodeXml( xmlNodePtr & node );
    virtual xmlNodePtr DecodeXml( xmlNodePtr & node );

    string GetTypeName( );
    string GetXSecName();

    IntParm m_FeaPropertyType;
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

protected:

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

    double GetShearModulus();

    Parm m_MassDensity;
    Parm m_ElasticModulus;
    Parm m_PoissonRatio;
    Parm m_ThermalExpanCoeff;

    bool m_UserFeaMaterial;
};

#endif // !defined(FEASTRUCTURE_INCLUDED_)
