//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//

// StructureMgr.h
//
// Justin Gravett
//////////////////////////////////////////////////////////////////////

#if !defined(STRUCTUREMGR__INCLUDED_)
#define STRUCTUREMGR__INCLUDED_

#include "FeaStructure.h"

class StructureMgrSingleton
{
protected:
    StructureMgrSingleton();

public:

    static StructureMgrSingleton& getInstance()
    {
        static StructureMgrSingleton instance;
        return instance;
    }

    virtual ~StructureMgrSingleton();

    virtual xmlNodePtr EncodeXml( xmlNodePtr & node );
    virtual xmlNodePtr DecodeXml( xmlNodePtr & node );

    virtual void Renew();
    virtual void Wype();

    //==== FeaStructure Management ====//
    virtual vector < FeaStructure* > GetAllFeaStructs();
    virtual FeaStructure* GetFeaStruct( int struct_ind );
    virtual bool ValidTotalFeaStructInd( int index );
    virtual int NumFeaStructures();
    virtual int GetFeaStructIndex( FeaStructure* fea_struct );
    virtual FeaPart* GetFeaPart( const string & feapart_id );
    virtual int GetFeaPartIndex( const string & feapart_id );
    virtual string GetFeaPartName( const string & id );
    virtual int GetFeaPropertyIndex( const string & FeaPartID );
    virtual int GetFeaMaterialIndex( const string & FeaPartID );
    virtual void ResetExportFileNames( const string & VSP3FileName );

    virtual void UpdateLengthUnit( int new_unit );
    virtual void UpdateMassUnit( int new_unit );
    virtual void UpdateTempUnit( int new_unit );

    //==== FeaProperty Management ====//
    virtual void AddFeaProperty( FeaProperty* fea_prop )
    {
        m_FeaPropertyVec.push_back( fea_prop );
    }
    virtual FeaProperty* AddFeaProperty( int property_type );
    virtual void DeleteFeaProperty( int index );
    virtual bool ValidFeaPropertyInd( int index );
    virtual FeaProperty* GetFeaProperty( int index );

    virtual vector < FeaProperty* > GetFeaPropertyVec()
    {
        return m_FeaPropertyVec;
    }
    virtual int NumFeaProperties()
    {
        return m_FeaPropertyVec.size();
    }
    virtual void InitFeaProperties();

    //==== FeaMaterial Management ====//
    virtual void AddFeaMaterial( FeaMaterial* fea_mat )
    {
        m_FeaMaterialVec.push_back( fea_mat );
    }
    virtual FeaMaterial* AddFeaMaterial();
    virtual void DeleteFeaMaterial( int index );
    virtual bool ValidFeaMaterialInd( int index );
    virtual FeaMaterial* GetFeaMaterial( int index );
    virtual void InitFeaMaterials();

    virtual vector < FeaMaterial* > GetFeaMaterialVec()
    {
        return m_FeaMaterialVec;
    }
    virtual int NumFeaMaterials()
    {
        return m_FeaMaterialVec.size();
    }

    virtual void SetCurrStructIndex( int ind )
    {
        m_CurrStructIndex = ind;
    }
    virtual int GetCurrStructIndex()
    {
        return m_CurrStructIndex;
    }

    virtual void SetCurrPartIndex( int ind )
    {
        m_CurrPartIndex = ind;
    }
    virtual int GetCurrPartIndex()
    {
        return m_CurrPartIndex;
    }

protected:

    vector < FeaProperty* > m_FeaPropertyVec;
    int m_FeaPropertyCount;

    vector < FeaMaterial* > m_FeaMaterialVec;
    int m_FeaMatCount;

    int m_CurrStructIndex;
    int m_CurrPartIndex;
};

#define StructureMgr StructureMgrSingleton::getInstance()

#endif