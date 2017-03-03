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

// Forward declaration
class FeaPart;

string GenerateID();

class FeaStructure
{
public:

    FeaStructure( string GeomID, int surf_index );
    virtual ~FeaStructure();

    virtual void Update();

    virtual xmlNodePtr EncodeXml( xmlNodePtr & node );
    virtual xmlNodePtr DecodeXml( xmlNodePtr & node );

    virtual void AddFeaPart( FeaPart* fea_struct )
    {
        m_FeaPartVec.push_back( fea_struct );
    }
    virtual FeaPart* AddFeaPart( int type );
    virtual bool ValidFeaPartInd( int ind );
    virtual void DelFeaPart( int ind );
    virtual void UpdateFeaParts();
    virtual FeaPart* GetFeaPart( int ind );
    virtual FeaPart* GetFeaPart( const string & id );
    virtual vector< FeaPart* > GetFeaPartVec()
    {
        return m_FeaPartVec;
    }
    virtual int GetFeaPartIndex( FeaPart* fea_prt );
    virtual int NumFeaParts()
    {
        return m_FeaPartVec.size();
    }

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

    virtual string GetParentGeomID()
    {
        return m_ParentGeomID;
    }

    virtual string GetFeaStructID()
    {
        return m_FeaStructID;
    }
    virtual void SetFeaStructID( string structID )
    {
        m_FeaStructID = structID;
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
    string m_FeaStructID;
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

    static string GetTypeName( int type );

    virtual void FetchFeaXFerSurf( vector< XferSurf > &xfersurfs );

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

    virtual int GetFeaMaterialIndex();
    virtual void SetFeaMaterialIndex( int index );

    IntParm m_MainSurfIndx;

protected:

    int m_FeaPartType;

    string m_ParentGeomID;

    int m_FeaPropertyIndex;

    vector < int > m_SymmIndexVec;

    vector < VspSurf > m_FeaPartSurfVec; 

    vector < DrawObj > m_FeaPartDO;
};

class FeaSpar : public FeaPart
{
public:

    FeaSpar( string geomID, int type = vsp::FEA_SPAR );
    virtual ~FeaSpar()    {};

    virtual void Update();

    virtual void ComputePlanarSurf();
    virtual void LoadDrawObjs( vector< DrawObj* > & draw_obj_vec, int id, bool highlight );

    Parm m_PerU; // Percent U Location
    Parm m_Alpha;
    Parm m_Theta;
    //BoolParm m_TrimFlag;
    //Parm m_PlaneSurfLengthScale;
    //Parm m_PlaneSurfWidthScale;

protected:

};

class FeaRib : public FeaPart
{
public:

    FeaRib( string geomID, int type = vsp::FEA_RIB );
    virtual ~FeaRib()    {};

    virtual void Update();

    virtual void ComputePlanarSurf();
    virtual void LoadDrawObjs( vector< DrawObj* > & draw_obj_vec, int id, bool highlight );

    Parm m_PerW; // Percent W Location
    Parm m_Alpha;
    Parm m_Theta;
    //BoolParm m_TrimFlag;
    //Parm m_PlaneSurfLengthScale;
    //Parm m_PlaneSurfWidthScale;

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
    CONST_U, CONST_W
};

class FeaStiffener : public FeaPart
{
public:

    FeaStiffener( string geomID, int type = vsp::FEA_STIFFENER );
    virtual ~FeaStiffener()    {};

    virtual void Update();

    virtual void ComputeEndPoints();
    virtual void LoadDrawObjs( vector< DrawObj* > & draw_obj_vec, int id, bool highlight );

    IntParm m_StiffenerConstType; // Either constant u or constant w line
    Parm m_StiffenerConstVal; // Either the const u value or const w value of line

protected:

    SSLine* m_FeaStiffenerSubSurf;
    vector < SSLineSeg > m_LVec;
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

#endif // !defined(FEASTRUCTURE_INCLUDED_)
