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
#include "StructSettings.h"
#include "GridDensity.h"

// Forward declaration
class FeaPart;

class FeaStructure
{
public:

    FeaStructure( string GeomID, int surf_index );
    virtual ~FeaStructure();

    virtual void Update();

    virtual xmlNodePtr EncodeXml( xmlNodePtr & node );
    virtual xmlNodePtr DecodeXml( xmlNodePtr & node );

    virtual void SetDrawFlag( bool flag );

    virtual void AddFeaPart( FeaPart* fea_part )
    {
        m_FeaPartVec.push_back( fea_part );
    }
    virtual FeaPart* AddFeaPart( int type );
    virtual bool ValidFeaPartInd( int ind );
    virtual void DelFeaPart( int ind );
    virtual void ReorderFeaPart( int ind, int action );
    virtual void UpdateFeaParts();
    virtual FeaPart* GetFeaPart( int ind );
    virtual string GetFeaPartName( int ind );
    virtual vector< FeaPart* > GetFeaPartVec()
    {
        return m_FeaPartVec;
    }
    virtual int GetFeaPartIndex( FeaPart* fea_part );
    virtual int NumFeaParts()
    {
        return m_FeaPartVec.size();
    }

    virtual bool FeaPartIsFixPoint( int ind );
    virtual int GetNumFeaFixPoints();

    virtual bool FeaPartIsArray( int ind );

    virtual void IndividualizeRibArray( int rib_array_ind );
    virtual void IndividualizeSliceArray( int slice_array_ind );
    virtual void IndividualizeSSLineArray( int ssline_array_ind );

    virtual vector< FeaPart* > InitFeaSkin( );
    virtual FeaPart* GetFeaSkin();

    virtual void UpdateFeaSubSurfs();
    virtual void RecolorFeaSubSurfs( vector < int > active_ind_vec );
    virtual SubSurface* AddFeaSubSurf( int type );
    virtual bool ValidFeaSubSurfInd( int ind );
    virtual void DelFeaSubSurf( int ind );
    virtual SubSurface* GetFeaSubSurf( int ind );
    virtual void ReorderFeaSubSurf( int ind, int action );
    virtual int NumFeaSubSurfs()
    {
        return m_FeaSubSurfVec.size();
    }
    virtual vector< SubSurface* > GetFeaSubSurfVec()
    {
        return m_FeaSubSurfVec;
    }
    virtual void AddFeaSubSurf( SubSurface* sub_surf )
    {
        m_FeaSubSurfVec.push_back( sub_surf );
    }

    virtual int GetFeaPropertyIndex( int fea_part_ind );
    virtual int GetCapFeaPropertyIndex( int fea_part_ind );

    virtual string GetParentGeomID()
    {
        return m_ParentGeomID;
    }

    virtual int GetFeaStructMainSurfIndx()
    {
        return m_MainSurfIndx;
    }

    virtual string GetFeaStructName()
    {
        return m_FeaStructName;
    }
    virtual void SetFeaStructName( string struct_name )
    {
        m_FeaStructName = struct_name;
    }

    virtual StructSettings* GetStructSettingsPtr()
    {
        return &m_StructSettings;
    }
    virtual GridDensity* GetFeaGridDensityPtr()
    {
        return &m_FeaGridDensity;
    }

    virtual void BuildSuppressList();

    virtual vector < double > GetUSuppress()
    {
        return m_Usuppress;
    }
    virtual vector < double > GetWSuppress()
    {
        return m_Wsuppress;
    }

    virtual bool PtsOnAnyPlanarPart( const vector < vec3d > &pnts );

protected:

    string m_ParentGeomID;
    int m_MainSurfIndx;
    int m_FeaPartCount; // Counter used for creating unique name for parts
    int m_FeaSubSurfCount; // Counter used for creating unique name for subsurfaces
    string m_FeaStructName;

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

    virtual vector < DrawObj > GetDrawObjVec()
    {
        return m_FeaPartDO;
    }

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

    virtual double GetRibPerU();
    virtual double GetRibTotalRotation();
    virtual VspSurf ComputeRibSurf();

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
    virtual void IdentifySplitSurfIndex();
    virtual vector < vec3d > GetPntVec(); // Returns the FeaFixPoint 3D coordinate on each parent surface
    virtual vec2d GetUW(); // Returns the FeaFixPoint UW coordinate on main parent surface

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
    bool m_HalfMeshFlag;

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

    virtual void BuildSkinSurf();

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

    virtual void BuildDomeSurf();

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
    virtual void CalcNumRibs();
    virtual void CreateFeaRibArray();

    virtual FeaRib* AddFeaRib( double center_location, int ind );

    virtual xmlNodePtr EncodeXml( xmlNodePtr & node );
    virtual xmlNodePtr DecodeXml( xmlNodePtr & node );

    virtual void UpdateDrawObjs( int id, bool highlight );

    virtual bool PtsOnPlanarPart( const vector < vec3d > & pnts, int surf_ind = 0 );

    virtual int GetNumRibs()
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
    virtual void CreateFeaSliceArray();
    virtual void CalcNumSlices();

    virtual FeaSlice* AddFeaSlice( double center_location, int ind );

    virtual void UpdateDrawObjs( int id, bool highlight );

    virtual bool PtsOnPlanarPart( const vector < vec3d > & pnts, int surf_ind = 0 );

    virtual int GetNumSlices()
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

    virtual string GetTypeName( );
    virtual string GetXSecName();

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

    virtual void ParmChanged( Parm* parm_ptr, int type );

    virtual xmlNodePtr EncodeXml( xmlNodePtr & node );
    virtual xmlNodePtr DecodeXml( xmlNodePtr & node );

    virtual double GetShearModulus();

    Parm m_MassDensity;
    Parm m_ElasticModulus;
    Parm m_PoissonRatio;
    Parm m_ThermalExpanCoeff;

    bool m_UserFeaMaterial;
};

#endif // !defined(FEASTRUCTURE_INCLUDED_)
