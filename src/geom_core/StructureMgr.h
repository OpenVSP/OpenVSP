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

    void Renew();
    void Wype();

    //==== FeaStructure Management ====//
    vector < FeaStructure* > GetAllFeaStructs();
    FeaStructure* GetFeaStruct( int struct_ind );
    bool ValidTotalFeaStructInd( int index );
    int NumFeaStructures();
    int GetTotFeaStructIndex( FeaStructure* fea_struct );
    FeaPart* GetFeaPart( const string & feapart_id );
    int GetFeaPartIndex( const string & feapart_id );
    int GetFeaSubSurfIndex( const string & ss_id );
    string GetFeaPartName( const string & id );
    int GetFeaPropertyIndex( const string & FeaPartID );
    int GetFeaMaterialIndex( const string & FeaPartID );
    void ResetExportFileNames( const string & VSP3FileName );
    void ShowAllParts();

    void UpdateStructUnit( int new_unit );

    //==== FeaProperty Management ====//
    void AddFeaProperty( FeaProperty* fea_prop )
    {
        m_FeaPropertyVec.push_back( fea_prop );
    }
    FeaProperty* AddFeaProperty( int property_type );
    void DeleteFeaProperty( int index );
    bool ValidFeaPropertyInd( int index );
    FeaProperty* GetFeaProperty( int index );

    vector < FeaProperty* > GetFeaPropertyVec()
    {
        return m_FeaPropertyVec;
    }
    int NumFeaProperties()
    {
        return m_FeaPropertyVec.size();
    }
    void InitFeaProperties();

    //==== FeaMaterial Management ====//
    void AddFeaMaterial( FeaMaterial* fea_mat )
    {
        m_FeaMaterialVec.push_back( fea_mat );
    }
    FeaMaterial* AddFeaMaterial();
    void DeleteFeaMaterial( int index );
    bool ValidFeaMaterialInd( int index );
    FeaMaterial* GetFeaMaterial( int index );
    void InitFeaMaterials();

    vector < FeaMaterial* > GetFeaMaterialVec()
    {
        return m_FeaMaterialVec;
    }
    int NumFeaMaterials()
    {
        return m_FeaMaterialVec.size();
    }

    void SetCurrStructIndex( int ind )
    {
        m_CurrStructIndex = ind;
    }
    int GetCurrStructIndex()
    {
        return m_CurrStructIndex;
    }

    void SetCurrPartIndex( int ind )
    {
        m_CurrPartIndex = ind;
    }
    int GetCurrPartIndex()
    {
        return m_CurrPartIndex;
    }

    void SetCurrMaterialIndex( int ind )
    {
        m_CurrFeaMaterialIndex = ind;
    }
    int GetCurrMaterialIndex()
    {
        return m_CurrFeaMaterialIndex;
    }

    void SetCurrPropertyIndex( int ind )
    {
        m_CurrFeaPropertyIndex = ind;
    }
    int GetCurrPropertyIndex()
    {
        return m_CurrFeaPropertyIndex;
    }

    void SetFeaSliceOrientIndex( int ind )
    {
        m_FeaSliceOrientationIndex = ind;
    }
    int GetFeaSliceOrientIndex()
    {
        return m_FeaSliceOrientationIndex;
    }

protected:

    vector < FeaProperty* > m_FeaPropertyVec;
    int m_FeaPropertyCount;

    vector < FeaMaterial* > m_FeaMaterialVec;
    int m_FeaMatCount;

    int m_CurrStructIndex;
    int m_CurrPartIndex;
    int m_CurrFeaMaterialIndex;
    int m_CurrFeaPropertyIndex;
    int m_FeaSliceOrientationIndex; // Identifies the default orientation for FeaSlices; 0: XY_PLANE, 1: YZ_PLANE, 2: XZ_PLANE
};

#define StructureMgr StructureMgrSingleton::getInstance()

#endif