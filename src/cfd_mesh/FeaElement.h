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
#include "FeaStructure.h"
#include "BitMask.h"

using namespace std;

class FeaNode
{
public:
    FeaNode()
    {
        m_Index = -1;
        m_FixedPointFlag = false;
    };
    FeaNode( const vec3d & p )
    {
        m_Pnt = p;
        m_Index = -1;
        m_FixedPointFlag = false;
    };
    virtual ~FeaNode()    {};

    long long int GetIndex();

    long long int m_Index;
    vec3d m_Pnt;
    bool m_FixedPointFlag;
    BitMask m_BCs;

    void AddTag( int ind );
    bool HasTag( int ind );
    bool HasOnlyTag( int ind );
    vector< int > m_Tags;

    void WriteNASTRAN( FILE* fp, long long int noffset, bool includeBC = false );
    void WriteNASTRAN_SPC1( FILE* fp, long long int noffset );
    void WriteCalculix( FILE* fp, long long int noffset );
    void WriteCalculixBCs( FILE* fp, long long int noffset );
    void WriteGmsh( FILE* fp, long long int noffset );
};

class FeaElement
{
public:

    FeaElement();
    virtual ~FeaElement()    {};

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
    virtual int GetFeaPartSurfNum()
    {
        return m_FeaPartSurfNum;
    }
    virtual void SetFeaPartSurfNum( int part_surf_num )
    {
        m_FeaPartSurfNum = part_surf_num;
    }
    virtual void SetReason( int r )
    {
        m_reason = r;
    }
    virtual int GetReason()
    {
        return m_reason;
    }
    virtual void SetChainIndex( int ichain )
    {
        m_ChainIndex = ichain;
    }
    virtual int GetChainIndex()
    {
        return m_ChainIndex;
    }

    virtual void WriteCalculix( FILE* fp, int id, long long int noffset, long long int eoffset ) = 0;
    virtual void WriteNASTRAN( FILE* fp, int id, int property_index, long long int noffset, long long int eoffset ) = 0;
    virtual void WriteGmsh( FILE* fp, int id, int fea_part_index, long long int noffset, long long int eoffset ) = 0;
    virtual void WriteSTL( FILE* fp ) = 0;
    virtual double ComputeMass( int property_index ) = 0;

    virtual int GetFeaSSIndex()
    {
        return m_FeaSSIndex;
    }
    virtual void SetFeaSSIndex( int fea_ss_index )
    {
        m_FeaSSIndex = fea_ss_index;
    }

    enum
    {
        FEA_TRI_3, FEA_TRI_6, FEA_BEAM, FEA_POINT_MASS, FEA_QUAD_4, FEA_QUAD_8
    };
    vector< FeaNode* > m_Corners;
    vector< FeaNode* > m_Mids;

    vec3d m_Orientation;
protected:

    int m_ElementType;
    int m_FeaPartIndex; // Corresponds to index in FeaStructure m_FeaPartVec
    int m_FeaSSIndex; // Corresponds to index in FeaStructure m_FeaSubSurfVec
    int m_FeaPartSurfNum; // corresponds to m_SurfVec->GetFeaPartSurfNum();
    int m_reason;
    int m_ChainIndex;
};

//==== 6 Point Triangle Element ====//
class FeaTri : public FeaElement
{
public:
    FeaTri()    {};
    virtual ~FeaTri()    {};

    virtual void Create( vec3d & p0, vec3d & p1, vec3d & p2, bool highorder );
    virtual void WriteCalculix( FILE* fp, int id, long long int noffset, long long int eoffset );
    virtual void WriteNASTRAN( FILE* fp, int id, int property_index, long long int noffset, long long int eoffset );
    virtual void WriteGmsh( FILE* fp, int id, int fea_part_index, long long int noffset, long long int eoffset );
    virtual void WriteSTL( FILE* fp );
    virtual double ComputeMass( int property_index );
};

//=== 8 Point Quad Element ====//
class FeaQuad : public FeaElement
{
public:
    FeaQuad()    {};
    virtual ~FeaQuad()    {};

    virtual void Create( vec3d & p0, vec3d & p1, vec3d & p2, vec3d & p3, bool highorder );
    virtual void WriteCalculix( FILE* fp, int id, long long int noffset, long long int eoffset );
    virtual void WriteNASTRAN( FILE* fp, int id, int property_index, long long int noffset, long long int eoffset );
    virtual void WriteGmsh( FILE* fp, int id, int fea_part_index, long long int noffset, long long int eoffset );
    virtual void WriteSTL( FILE* fp );
    virtual double ComputeMass( int property_index );
};

//=== Beam Element ====//
class FeaBeam : public FeaElement
{
public:
    FeaBeam()    { m_ElementIndex = -1; };
    virtual ~FeaBeam()    {};

    virtual void Create( vec3d &p0, vec3d &p1, vec3d &norm0, vec3d &norm1 );
    virtual void WriteCalculix( FILE* fp, int id, long long int noffset, long long int eoffset );
    virtual void WriteCalculixNormal( FILE* fp, long long int noffset, long long int eoffset );
    virtual void WriteCalculixNormal( FILE* fp );
    virtual void WriteNASTRAN( FILE* fp, int id, int property_index, long long int noffset, long long int eoffset );
    virtual void WriteGmsh( FILE* fp, int id, int fea_part_index, long long int noffset, long long int eoffset );
    virtual void WriteSTL( FILE* fp ) {};
    virtual double ComputeMass( int property_index );

    vec3d m_Norm0; // Vector from end point in the displacement coordinate system at the end point
    vec3d m_Norm1;

private:

    int m_ElementIndex;
};

//=== Point Mass Element ====//
class FeaPointMass : public FeaElement
{
public:
    FeaPointMass()    { m_Mass = 0; };
    virtual ~FeaPointMass()    {};

    virtual void Create( vec3d & p0, double mass );
    virtual void WriteCalculix( FILE* fp, int id, long long int noffset, long long int eoffset );
    virtual void WriteNASTRAN( FILE* fp, int id, int property_index, long long int noffset, long long int eoffset );
    virtual void WriteGmsh( FILE* fp, int id, int fea_part_index, long long int noffset, long long int eoffset )    {};
    virtual void WriteSTL( FILE* fp ) {};
    virtual double ComputeMass( int property_index )
    {
        return m_Mass;
    };

private:

    double m_Mass;

};

//=== SimpleFeaProperty ====//
class SimpleFeaProperty
{
    public:
    SimpleFeaProperty()    {
        m_Used = false;
        m_FeaPropertyType = vsp::FEA_SHELL;
        m_Thickness = 0;
        m_CrossSecArea = 0;
        m_Izz = 0;
        m_Iyy = 0;
        m_Izy = 0;
        m_Ixx = 0;
        m_Dim1 = 0;
        m_Dim2 = 0;
        m_Dim3 = 0;
        m_Dim4 = 0;
        m_Dim5 = 0;
        m_Dim6 = 0;
        m_CrossSectType = vsp::FEA_XSEC_GENERAL;
        m_SimpleFeaMatIndex = 0;
    };
    ~SimpleFeaProperty()    {};

    void CopyFrom( FeaProperty* fea_prop, const vector < string > &mat_id_vec );

    void WriteNASTRAN( FILE* fp, int id ) const;
    void WriteCalculix( FILE* fp, const string &ELSET, const string &ORIENTATION ) const;

    int GetSimpFeaMatIndex() const
    {
        return m_SimpleFeaMatIndex;
    }
    string GetFeaMatID()
    {
        return m_FeaMatID;
    }

    bool m_Used;

    int m_FeaPropertyType;
    double m_Thickness;
    double m_CrossSecArea;
    double m_Izz;
    double m_Iyy;
    double m_Izy;
    double m_Ixx;
    double m_Dim1;
    double m_Dim2;
    double m_Dim3;
    double m_Dim4;
    double m_Dim5;
    double m_Dim6;
    int m_CrossSectType;

    string m_ID;

    int m_SimpleFeaMatIndex;
    string m_FeaMatID;
    string m_MaterialName;
    string m_Name;
};

//=== SimpleFeaMaterial ====//
class SimpleFeaMaterial
{
public:
    SimpleFeaMaterial()    {
        m_Used = false;
        m_FeaMaterialType = vsp::FEA_ISOTROPIC;
        m_MassDensity = 0;
        m_ElasticModulus = 0;
        m_PoissonRatio = 0;
        m_ThermalExpanCoeff = 0;
        m_E1 = 0;
        m_E2 = 0;
        m_E3 = 0;
        m_nu12 = 0;
        m_nu13 = 0;
        m_nu23 = 0;
        m_G12 = 0;
        m_G13 = 0;
        m_G23 = 0;
        m_A1 = 0;
        m_A2 = 0;
        m_A3 = 0;
    };
    ~SimpleFeaMaterial()    {};

    void CopyFrom( FeaMaterial* fea_mat );

    void WriteNASTRAN( FILE* fp, int id ) const;
    void WriteCalculix( FILE* fp, int id ) const;

    double GetShearModulus() const;

    bool m_Used;

    double m_MassDensity;
    double m_ElasticModulus;
    double m_PoissonRatio;
    double m_ThermalExpanCoeff;

    int m_FeaMaterialType;

    // Orthotropic material properties
    double m_E1;
    double m_E2;
    double m_E3;
    double m_nu12;
    double m_nu13;
    double m_nu23;
    double m_G12;
    double m_G13;
    double m_G23;
    double m_A1;
    double m_A2;
    double m_A3;

    string m_ID;

protected:

    string m_Name;
};


#endif // !defined(FEAELEMENT_INCLUDED_)

