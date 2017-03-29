//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//

// FeaElement.h
//
// Justin Gravett
//******************************************************************************
//   Derived from FEA Part Class
//   J.R. Gloudemans - 2/14/09
//******************************************************************************
//////////////////////////////////////////////////////////////////////

#ifndef FEAELEMENT_INCLUDED_
#define FEAELEMENT_INCLUDED_

#include "Vec3d.h"

using namespace std;

class FeaNodeTag
{
    public:
    int m_FeaPartTagIndex;
};

class FeaNode
{
    public:
    FeaNode()
    {
        m_Index = -1;
    };
    FeaNode( vec3d & p )
    {
        m_Pnt = p;
        m_Index = -1;
    };
    virtual ~FeaNode()
    {
    };

    int GetIndex();

    int m_Index;
    vec3d m_Pnt;

    void AddTag( int ind );
    bool HasTag( int ind );
    bool HasOnlyIndex( int ind );
    vector< FeaNodeTag > m_Tags;

    void WriteNASTRAN( FILE* fp );
    void WriteCalculix( FILE* fp );
    void WriteGmsh( FILE* fp );
};

class FeaElement
{
    public:

    FeaElement()
    {
    };
    virtual ~FeaElement()
    {
    };

    virtual void DeleteAllNodes();
    virtual void LoadNodes( vector< FeaNode* > & node_vec );
    virtual int GetElementType()
    {
        return m_ElementType;
    }
    virtual int GetFeaPartIndex()
    {
        return m_FeaPartIndex;
    }
    virtual void SetFeaPartIndex( int fea_part_index )
    {
        m_FeaPartIndex = fea_part_index;
    }
    virtual int GetFeaPropertyIndex()
    {
        return m_FeaPropertyIndex;
    }
    virtual void SetFeaPropertyIndex( int index )
    {
        m_FeaPropertyIndex = index;
    }
    virtual void WriteCalculix( FILE* fp, int id ) = 0;
    virtual void WriteNASTRAN( FILE* fp, int id ) = 0;
    virtual void WriteGmsh( FILE* fp, int id , int fea_part_index ) = 0;
    virtual double ComputeMass() = 0;
    //  virtual void DrawPoly();

    enum
    {
        FEA_TRI_6, FEA_QUAD_8, FEA_BEAM
    };
    vector< FeaNode* > m_Corners;
    vector< FeaNode* > m_Mids;

    protected:
    int m_ElementType;
    int m_FeaPartIndex;
    int m_FeaPropertyIndex;
};

//==== 6 Point Triangle Element ====//
class FeaTri : public FeaElement
{
    public:
    FeaTri()
    {
    };
    virtual ~FeaTri()
    {
    };

    virtual void Create( vec3d & p0, vec3d & p1, vec3d & p2 );
    virtual void WriteCalculix( FILE* fp, int id );
    virtual void WriteNASTRAN( FILE* fp, int id );
    virtual void WriteGmsh( FILE* fp, int id, int fea_part_index );
    virtual double ComputeMass();
};

//=== 8 Point Quad Element ====//
class FeaQuad : public FeaElement
{
    public:
    FeaQuad()
    {
    };
    virtual ~FeaQuad()
    {
    };

    virtual void Create( vec3d & p0, vec3d & p1, vec3d & p2, vec3d & p3 );
    virtual void WriteCalculix( FILE* fp, int id );
    virtual void WriteNASTRAN( FILE* fp, int id );
    virtual void WriteGmsh( FILE* fp, int id, int fea_part_index );
    virtual double ComputeMass();
};

//=== Beam Element ====//
class FeaBeam : public FeaElement
{
    public:
    FeaBeam()
    {
    };
    virtual ~FeaBeam()
    {
    };

    virtual void Create( vec3d & p0, vec3d & p1, vec3d & p2 );
};

#endif // !defined(FEAELEMENT_INCLUDED_)

