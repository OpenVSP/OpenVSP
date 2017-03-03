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

#endif // !defined(FEASTRUCTURE_INCLUDED_)
