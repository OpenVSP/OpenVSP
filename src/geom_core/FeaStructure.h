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
#include "TMesh.h"
#include "Vec2d.h"
#include "Vec3d.h"
#include "Parm.h"
#include "ParmContainer.h"
#include "DrawObj.h"
#include "APIDefines.h"
#include "SubSurface.h"
#include "Matrix.h"
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
    virtual void IndividualizeStiffenerArray( int stiffener_array_ind );

    virtual vector< FeaPart* > InitFeaSkin( );
    virtual FeaPart* GetFeaSkin( );
    virtual int GetNumFeaSkin();

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

protected:

    string m_ParentGeomID;
    int m_MainSurfIndx;
    int m_FeaPartCount; // Counter used for creating unique name for parts
    int m_FeaSubSurfCount; // Counter used for creating unique name for subsurfaces
    string m_FeaStructName;

    vector < FeaPart* > m_FeaPartVec;

    vector < SubSurface* > m_FeaSubSurfVec;

    StructSettings m_StructSettings;
    FeaGridDensity m_FeaGridDensity;

};

enum
{
    TRIS, BEAM, BOTH_ELEMENTS
};

enum
{
    PERCENT, LENGTH
};

enum
{
    XY_BODY, YZ_BODY, XZ_BODY, XY_ABS, YZ_ABS, XZ_ABS, CONST_U
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

    virtual double GetRibPerU( );
    virtual double GetRibTotalRotation( double initial_rotation, string perp_edge_ID );
    virtual VspSurf ComputeRibSurf( double rotation );

    virtual bool RefFrameIsBody( int orientation_plane );
    virtual VspSurf ComputeSliceSurf( int orientation_plane, double x_rot, double y_rot, double z_rot );

    virtual void FetchFeaXFerSurf( vector< XferSurf > &xfersurfs, int compid );

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

    virtual int GetFeaPropertyIndex()
    {
        return m_FeaPropertyIndex;
    }
    virtual void SetFeaPropertyIndex( int index )
    {
        m_FeaPropertyIndex = index;
    }

    virtual int GetCapFeaPropertyIndex()
    {
        return m_CapFeaPropertyIndex;
    }
    virtual void SetCapFeaPropertyIndex( int index )
    {
        m_CapFeaPropertyIndex = index;
    }

    virtual vector < VspSurf > GetFeaPartSurfVec()
    {
        return m_FeaPartSurfVec;
    }

    virtual int GetFeaMaterialIndex();
    virtual void SetFeaMaterialIndex( int index );

    virtual vector < DrawObj > GetDrawObjVec()
    {
        return m_FeaPartDO;
    }

    IntParm m_MainSurfIndx;
    IntParm m_IncludedElements;
    BoolParm m_DrawFeaPartFlag;
    IntParm m_LocationParmType;
    Parm m_CenterLocation;

protected:

    int m_FeaPartType;

    string m_ParentGeomID;

    int m_FeaPropertyIndex;
    int m_CapFeaPropertyIndex;

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

    virtual void UpdateDrawObjs( int id, bool highlight );

    IntParm m_OrientationPlane;
    IntParm m_RotationAxis;
    Parm m_XRot;
    Parm m_YRot;
    Parm m_ZRot;
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

    virtual void ComputePlanarSurf();
    virtual void UpdateDrawObjs( int id, bool highlight );

    Parm m_Theta;
    BoolParm m_LimitSparToSectionFlag;
    IntParm m_CurrWingSection;

protected:

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

protected:

    string m_PerpendicularEdgeID;

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

    BoolParm m_RemoveSkinTrisFlag;

protected:


};

class FeaBulkhead : public FeaPart
{
public:

    FeaBulkhead( string geomID, int type = vsp::FEA_BULKHEAD );
    virtual ~FeaBulkhead()    {};

    virtual void Update();

    virtual void BuildBulkheadSurf();

    virtual void UpdateDrawObjs( int id, bool highlight );

    Parm m_Aradius;
    Parm m_Bradius;
    Parm m_Cradius;
    Parm m_XLoc;
    Parm m_YLoc;
    Parm m_ZLoc;
    Parm m_XRot;
    Parm m_YRot;
    Parm m_ZRot;

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

    virtual void UpdateDrawObjs( int id, bool highlight );

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

    Parm m_RibSpacing;
    Parm m_StartLocation;
    Parm m_Theta;

protected:

    int m_NumRibs;

    string m_PerpendicularEdgeID;
};

class FeaStiffenerArray : public FeaPart
{
public:

    FeaStiffenerArray( string geomID, int type = vsp::FEA_STIFFENER_ARRAY );
    virtual ~FeaStiffenerArray()    {};

    virtual void Update();
    virtual void CreateFeaStiffenerArray();
    virtual void CalcNumStiffeners();

    virtual FeaSlice* AddFeaSlice( double center_location, int ind );

    virtual void UpdateDrawObjs( int id, bool highlight );

    virtual int GetNumStiffeners()
    {
        return m_NumStiffeners;
    }

    Parm m_StiffenerSpacing;
    Parm m_StartLocation;

protected:

    int m_NumStiffeners;

};

enum
{
    SHELL_PROPERTY, BEAM_PROPERTY,
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

    virtual void WriteNASTRAN( FILE* fp, int prop_id );
    virtual void WriteCalculix( FILE* fp, string ELSET );

    virtual int GetFeaMaterialIndex()
    {
        return m_FeaMaterialIndex;
    }
    virtual void SetFeaMaterialIndex( int index )
    {
        m_FeaMaterialIndex = index;
    }

    IntParm m_FeaPropertyType;
    Parm m_Thickness;
    Parm m_CrossSecArea;
    Parm m_Izz;
    Parm m_Iyy;
    Parm m_Izy;
    Parm m_Ixx;

    bool m_UserFeaProperty;

protected:

    int m_FeaMaterialIndex;
};

class FeaMaterial : public ParmContainer
{
public:
    FeaMaterial( );
    virtual ~FeaMaterial();

    virtual void ParmChanged( Parm* parm_ptr, int type );

    virtual xmlNodePtr EncodeXml( xmlNodePtr & node );
    virtual xmlNodePtr DecodeXml( xmlNodePtr & node );

    virtual void WriteNASTRAN( FILE* fp, int id );
    virtual void WriteCalculix( FILE* fp, int id );

    virtual double GetShearModulus();

    Parm m_MassDensity;
    Parm m_ElasticModulus;
    Parm m_PoissonRatio;
    Parm m_ThermalExpanCoeff;

    bool m_UserFeaMaterial;
};

#endif // !defined(FEASTRUCTURE_INCLUDED_)
