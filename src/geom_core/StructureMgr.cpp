//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//

// StructureMgr.cpp
//
// Justin Gravett
//////////////////////////////////////////////////////////////////////

#include "StructureMgr.h"
#include "VehicleMgr.h"
#include "Vehicle.h"


StructureMgrSingleton::StructureMgrSingleton()
{
    InitFeaMaterials();
    InitFeaProperties();

    m_FeaPropertyCount = 0;
    m_FeaMatCount = 0;
}

StructureMgrSingleton::~StructureMgrSingleton()
{

}

xmlNodePtr StructureMgrSingleton::EncodeXml( xmlNodePtr & node )
{
    xmlNodePtr structmgr_node = xmlNewChild( node, NULL, BAD_CAST"StructureMgr", NULL );

    for ( int i = 0; i < (int)m_FeaPropertyVec.size(); i++ )
    {
        if ( m_FeaPropertyVec[i]->m_UserFeaProperty )
        {
            m_FeaPropertyVec[i]->EncodeXml( structmgr_node );
        }
    }

    for ( int i = 0; i < (int)m_FeaMaterialVec.size(); i++ )
    {
        if ( m_FeaMaterialVec[i]->m_UserFeaMaterial )
        {
            m_FeaMaterialVec[i]->EncodeXml( structmgr_node );
        }
    }

    return structmgr_node;
}

xmlNodePtr StructureMgrSingleton::DecodeXml( xmlNodePtr & node )
{
    xmlNodePtr structmgr_node = XmlUtil::GetNode( node, "StructureMgr", 0 );

    if ( structmgr_node )
    {
        int num_prop = XmlUtil::GetNumNames( structmgr_node, "FeaPropertyInfo" );

        for ( unsigned int i = 0; i < num_prop; i++ )
        {
            xmlNodePtr prop_info = XmlUtil::GetNode( structmgr_node, "FeaPropertyInfo", i );

            if ( prop_info )
            {
                FeaProperty* fea_prop = new FeaProperty();

                if ( fea_prop )
                {
                    fea_prop->DecodeXml( prop_info );
                    fea_prop->m_UserFeaProperty = true;

                    AddFeaProperty( fea_prop );
                }
            }
        }

        int num_mat = XmlUtil::GetNumNames( structmgr_node, "FeaMaterialInfo" );

        for ( unsigned int i = 0; i < num_mat; i++ )
        {
            xmlNodePtr mat_info = XmlUtil::GetNode( structmgr_node, "FeaMaterialInfo", i );

            if ( mat_info )
            {
                FeaMaterial* fea_mat = new FeaMaterial();

                if ( fea_mat )
                {
                    fea_mat->DecodeXml( mat_info );
                    fea_mat->m_UserFeaMaterial = true;

                    AddFeaMaterial( fea_mat );
                }
            }
        }
    }

    return node;
}

void StructureMgrSingleton::Renew()
{
    Wype();
    InitFeaMaterials();
    InitFeaProperties();
}

void StructureMgrSingleton::Wype()
{
    // Delete FeaProperties
    for ( int i = 0; i < (int)m_FeaPropertyVec.size(); i++ )
    {
        delete m_FeaPropertyVec[i];
    }
    m_FeaPropertyVec.clear();

    // Delete FeaMaterials
    for ( int i = 0; i < (int)m_FeaMaterialVec.size(); i++ )
    {
        delete m_FeaMaterialVec[i];
    }
    m_FeaMaterialVec.clear();
}

//==== Get All FeaStructures ====//
vector < FeaStructure* > StructureMgrSingleton::GetAllFeaStructs()
{
    vector < FeaStructure* > feastructvec;

    Vehicle* veh = VehicleMgr.GetVehicle();

    if ( veh )
    {
        vector< Geom* > geom_vec = veh->FindGeomVec( veh->GetGeomVec( false ) );

        for ( unsigned int i = 0; i < geom_vec.size(); i++ )
        {
            vector < FeaStructure* > geomstructvec = geom_vec[i]->GetFeaStructVec();

            for ( unsigned int j = 0; j < geomstructvec.size(); j++ )
            {
                feastructvec.push_back( geomstructvec[j] );
            }
        }
    }

    return feastructvec;
}

//==== Validate FeaStructure Index ====//
bool StructureMgrSingleton::ValidTotalFeaStructInd( int index )
{
    vector < FeaStructure* > feastructvec = GetAllFeaStructs();

    if ( (int)feastructvec.size() > 0 && index >= 0 && index < (int)feastructvec.size() )
    {
        return true;
    }
    else
    {
        return false;
    }
}

int StructureMgrSingleton::NumFeaStructures()
{
    vector < FeaStructure* > struct_vec = GetAllFeaStructs();
    return struct_vec.size();
}

int StructureMgrSingleton::GetFeaStructIndex( FeaStructure* fea_struct )
{
    vector < FeaStructure* > fea_struct_vec = GetAllFeaStructs();

    for ( int i = 0; i < fea_struct_vec.size(); i++ )
    {
        if ( fea_struct_vec[i] == fea_struct )
        {
            return i;
        }
    }
    return -1; // indicates an error
}

//==== Get FeaPart pointer from FeaPart ID ====//
FeaPart* StructureMgrSingleton::GetFeaPart( const string & feapart_id )
{
    FeaPart* ret_ptr = NULL;

    vector < FeaStructure* > fea_struct_vec = GetAllFeaStructs();

    for ( unsigned int i = 0; i < fea_struct_vec.size(); i++ )
    {
        vector < FeaPart* > fea_part_vec = fea_struct_vec[i]->GetFeaPartVec();

        for ( unsigned int j = 0; j < fea_part_vec.size(); j++ )
        {
            if ( fea_part_vec[j]->GetID() == feapart_id )
            {
                return fea_part_vec[j];
            }
        }
    }

    return ret_ptr;
}

//==== Get FeaPart index from FeaPart ID ====//
int StructureMgrSingleton::GetFeaPartIndex( const string & feapart_id )
{
    vector < FeaStructure* > fea_struct_vec = GetAllFeaStructs();

    for ( unsigned int i = 0; i < fea_struct_vec.size(); i++ )
    {
        vector < FeaPart* > fea_part_vec = fea_struct_vec[i]->GetFeaPartVec();

        for ( unsigned int j = 0; j < fea_part_vec.size(); j++ )
        {
            if ( fea_part_vec[j]->GetID() == feapart_id )
            {
                return j;
            }
        }
    }
    return -1; // indicates an error
}

string StructureMgrSingleton::GetFeaPartName( const string & id )
{
    string name;
    FeaPart* fea_part = GetFeaPart( id );

    if ( fea_part )
    {
        name = fea_part->GetName();
    }
    return name;
}

//==== Get FeaProperty Index =====//
int StructureMgrSingleton::GetFeaPropertyIndex( const string & FeaPartID )
{
    FeaPart* fea_part = GetFeaPart( FeaPartID );

    if ( fea_part )
    {
        return fea_part->GetFeaPropertyIndex();
    }

    return -1; // indicates an error
}

//==== Get FeaMaterial Index =====//
int StructureMgrSingleton::GetFeaMaterialIndex( const string & FeaPartID )
{
    FeaPart* fea_part = GetFeaPart( FeaPartID );

    if ( fea_part )
    {
        return fea_part->GetFeaMaterialIndex();
    }

    return -1; // indicates an error
}

//==== Add FeaProperty =====//
FeaProperty* StructureMgrSingleton::AddFeaProperty( int property_type )
{
    FeaProperty* feaprop = new FeaProperty();

    if ( feaprop )
    {
        feaprop->SetName( string( "FEA_PROPERTY_" + std::to_string( m_FeaPropertyCount ) ) );
        feaprop->m_FeaPropertyType.Set( property_type );
        feaprop->m_UserFeaProperty = true;
        m_FeaPropertyVec.push_back( feaprop );
        m_FeaPropertyCount++;
    }

    return feaprop;
}

//==== Delete FeaProperty =====//
void StructureMgrSingleton::DeleteFeaProperty( int index )
{
    if ( !ValidFeaPropertyInd( index ) || !m_FeaPropertyVec[index]->m_UserFeaProperty )
        return;

    delete m_FeaPropertyVec[index];
    m_FeaPropertyVec.erase( m_FeaPropertyVec.begin() + index );
}

//==== Validate FeaProperty Index ====//
bool StructureMgrSingleton::ValidFeaPropertyInd( int index )
{
    if ( (int)m_FeaPropertyVec.size() > 0 && index >= 0 && index < (int)m_FeaPropertyVec.size() )
    {
        return true;
    }
    else
    {
        return false;
    }
}

FeaProperty* StructureMgrSingleton::GetFeaProperty( int index )
{
    if ( ValidFeaPropertyInd( index ) )
    {
        return m_FeaPropertyVec[index];
    }
    else
    {
        return NULL;
    }
}

void StructureMgrSingleton::InitFeaProperties()
{
    FeaProperty* default_shell = new FeaProperty();

    if ( default_shell )
    {
        default_shell->SetName( "Default_Shell" );
        default_shell->SetFeaMaterialIndex( 0 ); // aluminum
        default_shell->m_FeaPropertyType.Set( SHELL_PROPERTY );
        default_shell->m_UserFeaProperty = false;

        AddFeaProperty( default_shell );
    }

    FeaProperty* default_beam = new FeaProperty();

    if ( default_beam )
    {
        default_beam->SetName( "Default_Beam" );
        default_beam->SetFeaMaterialIndex( 0 ); // aluminum
        default_beam->m_FeaPropertyType.Set( BEAM_PROPERTY );
        default_beam->m_UserFeaProperty = false;

        AddFeaProperty( default_beam );
    }
}

//==== Add FeaMaterial =====//
FeaMaterial* StructureMgrSingleton::AddFeaMaterial()
{
    FeaMaterial* feamat = new FeaMaterial();

    if ( feamat )
    {
        feamat->SetName( string( "FEA_MATERIAL_" + std::to_string( m_FeaMatCount ) ) );
        feamat->m_UserFeaMaterial = true;
        m_FeaMaterialVec.push_back( feamat );
        m_FeaMatCount++;
    }

    return feamat;
}

//==== Delete FeaMaterial =====//
void StructureMgrSingleton::DeleteFeaMaterial( int index )
{
    if ( !ValidFeaMaterialInd( index ) || !m_FeaMaterialVec[index]->m_UserFeaMaterial ) 
        return;

    delete m_FeaMaterialVec[index];
    m_FeaMaterialVec.erase( m_FeaMaterialVec.begin() + index );
}

//==== Validate FeaMaterial Index ====//
bool StructureMgrSingleton::ValidFeaMaterialInd( int index )
{
    if ( (int)m_FeaMaterialVec.size() > 0 && index >= 0 && index < (int)m_FeaMaterialVec.size() )
    {
        return true;
    }
    else
    {
        return false;
    }
}

FeaMaterial* StructureMgrSingleton::GetFeaMaterial( int index )
{
    if ( ValidFeaMaterialInd( index ) )
    {
        return m_FeaMaterialVec[index];
    }
    else
    {
        return NULL;
    }
}

void StructureMgrSingleton::InitFeaMaterials()
{
    // Reference: http://www.matweb.com/search/datasheet.aspx?bassnum=MA0001

    FeaMaterial* aluminum = new FeaMaterial();

    if ( aluminum )
    {
        aluminum->SetName( "Aluminum" );
        aluminum->m_MassDensity.Set( 0.0975 ); // lb/in^3
        aluminum->m_ElasticModulus.Set( 10.2e6 ); // psi
        aluminum->m_PoissonRatio.Set( 0.33 );
        aluminum->m_ThermalExpanCoeff.Set( 1.33e-5 ); // in/(in-°F)
        aluminum->m_UserFeaMaterial = false;
        AddFeaMaterial( aluminum );
    }

    FeaMaterial* steel = new FeaMaterial();

    if ( steel )
    {
        steel->SetName( "Steel" );
        steel->m_MassDensity.Set( 0.284 ); // lb/in^3
        steel->m_ElasticModulus.Set( 29.0e6 ); // psi
        steel->m_PoissonRatio.Set( 0.30 );
        steel->m_ThermalExpanCoeff.Set( 6.67e-6 ); // in/(in-°F)
        steel->m_UserFeaMaterial = false;
        AddFeaMaterial( steel );
    }
}
