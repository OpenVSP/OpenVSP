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

    virtual void AddFeaPart( FeaPart* fea_part )
    {
        m_FeaPartVec.push_back( fea_part );
    }
    virtual FeaPart* AddFeaPart( int type );
    virtual bool ValidFeaPartInd( int ind );
    virtual void DelFeaPart( int ind );
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

    virtual vector< FeaPart* > AddEvenlySpacedRibs( const int num_rib );

    virtual vector< FeaPart* > InitFeaSkin( );
    virtual FeaPart* GetFeaSkin( );
    virtual int GetNumFeaSkin();

    virtual void RecolorFeaSubSurfs( int active_ind );
    virtual SubSurface* AddFeaSubSurf( int type );
    virtual bool ValidFeaSubSurfInd( int ind );
    virtual void DelFeaSubSurf( int ind );
    virtual SubSurface* GetFeaSubSurf( int ind );
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

protected:

    string m_ParentGeomID;
    int m_MainSurfIndx;
    int m_FeaPartCount; // Counter used for creating unique name for parts
    int m_FeaSubSurfCount; // Counter used for creating unique name for subsurfaces
    string m_FeaStructName;

    vector < FeaPart* > m_FeaPartVec;

    vector < SubSurface* > m_FeaSubSurfVec;

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

    virtual void UpdateSymmetricSurfs();

    vector < Matrix4d > CalculateSymmetricTransform();

    static string GetTypeName( int type );

    virtual void FetchFeaXFerSurf( vector< XferSurf > &xfersurfs, int compid );

    virtual void LoadDrawObjs( vector< DrawObj* > & draw_obj_vec, int id, bool highlight );

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

    virtual int GetFeaMaterialIndex();
    virtual void SetFeaMaterialIndex( int index );

    IntParm m_MainSurfIndx;
    BoolParm m_IntersectionCapFlag;

protected:

    int m_FeaPartType;

    string m_ParentGeomID;

    int m_FeaPropertyIndex;
    int m_CapFeaPropertyIndex;

    vector < int > m_SymmIndexVec;

    vector < VspSurf > m_FeaPartSurfVec; 

    vector < DrawObj > m_FeaPartDO;
};

enum
{
    XY_PLANE, YZ_PLANE, XZ_PLANE
};

class FeaFullDepth : public FeaPart
{
public:

    FeaFullDepth( string geomID, int type = vsp::FEA_FULL_DEPTH );
    virtual ~FeaFullDepth()    {};

    virtual void Update();

    virtual void ComputePlanarSurf();
    virtual void LoadDrawObjs( vector< DrawObj* > & draw_obj_vec, int id, bool highlight );

    IntParm m_OrientationPlane;
    Parm m_CenterPerBBoxLocation;
    Parm m_Theta;
};

enum
{
    PARALLEL_NONE, PARALLEL_LEAD_EDGE, PARALLEL_TRAIL_EDGE
};

class FeaSpar : public FeaPart
{
public:

    FeaSpar( string geomID, int type = vsp::FEA_SPAR );
    virtual ~FeaSpar()    {};

    virtual void Update();

    virtual void ComputePlanarSurf();
    virtual void LoadDrawObjs( vector< DrawObj* > & draw_obj_vec, int id, bool highlight );

    Parm m_PerV; // Percent V Location
    Parm m_Theta;

protected:

};

enum
{
    PERPENDICULAR_NONE, PERPENDICULAR_LEAD_EDGE, PERPENDICULAR_TRAIL_EDGE
};

class FeaRib : public FeaPart
{
public:

    FeaRib( string geomID, int type = vsp::FEA_RIB );
    virtual ~FeaRib()    {};

    virtual void Update();

    virtual void ComputePlanarSurf();
    virtual void LoadDrawObjs( vector< DrawObj* > & draw_obj_vec, int id, bool highlight );

    Parm m_PerU; // Percent U Location
    Parm m_Theta;

    IntParm m_PerpendicularEdgeFlag;

protected:


};

class FeaFixPoint : public FeaPart
{
public:

    FeaFixPoint( string geomID, int type = vsp::FEA_FIX_POINT );
    virtual ~FeaFixPoint()    {};

    virtual void Update();

    virtual void LoadDrawObjs( vector< DrawObj* > & draw_obj_vec, int id, bool highlight );

    Parm m_PosU;
    Parm m_PosW;

protected:


};

enum
{
    CONST_U, CONST_V
};

enum
{
    STIFFENER_PLANE, STIFFENER_SUBSURF
};

class FeaStiffenerPlane : public FeaPart
{
public:

    FeaStiffenerPlane( string geomID, int type = vsp::FEA_STIFFENER_PLANE );
    virtual ~FeaStiffenerPlane()    {};

    virtual void Update();

    virtual void ComputePlanarSurf();

    virtual void LoadDrawObjs( vector< DrawObj* > & draw_obj_vec, int id, bool highlight );

    IntParm m_OrientationPlane;
    Parm m_CenterPerBBoxLocation;
    Parm m_Theta;
};

class FeaStiffenerSubSurf : public FeaPart
{
public:

    FeaStiffenerSubSurf( string geomID, int type = vsp::FEA_STIFFENER_SUB_SURF );
    virtual ~FeaStiffenerSubSurf()    {};

    virtual void Update();

    virtual void LoadDrawObjs( vector< DrawObj* > & draw_obj_vec, int id, bool highlight );

    Parm m_StiffenerConstVal;
    IntParm m_StiffenerConstType; // Either constant u or constant w line

protected:

    SSLine* m_FeaStiffenerSubSurf;
};

class FeaSkin : public FeaPart
{
public:

    FeaSkin( string geomID, int type = vsp::FEA_SKIN );
    virtual ~FeaSkin()    {};

    virtual void Update();

    virtual void BuildSkinSurf();

protected:


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
