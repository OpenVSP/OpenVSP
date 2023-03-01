//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//

// StructureMgr.cpp
//
// Justin Gravett
//////////////////////////////////////////////////////////////////////

#include "StructureMgr.h"
#include "Vehicle.h"
#include "UnitConversion.h"

StructureMgrSingleton::StructureMgrSingleton()
{
    InitFeaMaterials();
    InitFeaProperties();

    m_FeaAssemblyCount = 0;
    m_FeaPropertyCount = 0;
    m_FeaMatCount = 0;

    m_CurrStructIndex.Init( "CurrStructIndex", "Struct", this, -1, -1, 1e12 ); // -1 for none selected.

    m_CurrPartIndex = -1;
    m_CurrFeaMaterialIndex = -1;
    m_CurrFeaPropertyIndex = -1;

    m_FeaSliceOrientationIndex = 1; // Default to XY Plane
}

StructureMgrSingleton::~StructureMgrSingleton()
{
    Wype();
}

xmlNodePtr StructureMgrSingleton::EncodeXml( xmlNodePtr & node )
{
    xmlNodePtr structmgr_node = xmlNewChild( node, NULL, BAD_CAST"StructureMgr", NULL );

    ParmContainer::EncodeXml( structmgr_node );

    for ( int i = 0; i < (int)m_FeaMaterialVec.size(); i++ )
    {
        if ( m_FeaMaterialVec[i]->m_UserFeaMaterial )
        {
            m_FeaMaterialVec[i]->EncodeXml( structmgr_node );
        }
    }

    for ( int i = 0; i < (int)m_FeaPropertyVec.size(); i++ )
    {
        m_FeaPropertyVec[i]->EncodeXml( structmgr_node );
    }

    for ( int i = 0; i < (int)m_FeaAssemblyVec.size(); i++ )
    {
        m_FeaAssemblyVec[i]->EncodeXml( structmgr_node );
    }

    return structmgr_node;
}

xmlNodePtr StructureMgrSingleton::DecodeXml( xmlNodePtr & node )
{
    xmlNodePtr structmgr_node = XmlUtil::GetNode( node, "StructureMgr", 0 );

    if ( structmgr_node )
    {
        ParmContainer::DecodeXml( structmgr_node );

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

                    AddFeaProperty( fea_prop );
                }
            }
        }

        int num_assy = XmlUtil::GetNumNames( structmgr_node, "FeaAssembly" );

        for ( unsigned int i = 0; i < num_assy; i++ )
        {
            xmlNodePtr assy = XmlUtil::GetNode( structmgr_node, "FeaAssembly", i );

            if ( num_assy )
            {
                FeaAssembly* fea_assy = new FeaAssembly();

                if ( fea_assy )
                {
                    fea_assy->DecodeXml( assy );

                    AddFeaAssembly( fea_assy );
                }
            }
        }


    }

    InitFeaProperties();

    return node;
}

void StructureMgrSingleton::Renew()
{
    Wype();
    InitFeaMaterials();
}

void StructureMgrSingleton::Wype()
{
    // Delete FeaAssemblies
    for ( int i = 0; i < (int)m_FeaAssemblyVec.size(); i++ )
    {
        delete m_FeaAssemblyVec[i];
    }
    m_FeaAssemblyVec.clear();

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

void StructureMgrSingleton::Update()
{
    for ( int i = 0; i < (int)m_FeaMaterialVec.size(); i++ )
    {
        m_FeaMaterialVec[i]->Update();
    }

    for ( int i = 0; i < (int)m_FeaPropertyVec.size(); i++ )
    {
        m_FeaPropertyVec[i]->Update();
    }

    for ( int i = 0; i < (int)m_FeaAssemblyVec.size(); i++ )
    {
        m_FeaAssemblyVec[i]->Update();
    }
}

void StructureMgrSingleton::AddLinkableContainers( vector< string > & linkable_container_vec )
{
    vector< FeaStructure* > feastructvec = GetAllFeaStructs();

    for ( size_t i = 0; i < feastructvec.size(); i++ )
    {
        feastructvec[i]->AddLinkableContainers( linkable_container_vec );
    }

    for ( int i = 0; i < (int)m_FeaMaterialVec.size(); i++ )
    {
        m_FeaMaterialVec[i]->AddLinkableContainers( linkable_container_vec );
    }

    for ( int i = 0; i < (int)m_FeaPropertyVec.size(); i++ )
    {
        m_FeaPropertyVec[i]->AddLinkableContainers( linkable_container_vec );
    }

    for ( int i = 0; i < (int)m_FeaAssemblyVec.size(); i++ )
    {
        m_FeaAssemblyVec[i]->AddLinkableContainers( linkable_container_vec );
    }
}

//==== Get All FeaStructures ====//
vector < FeaStructure* > StructureMgrSingleton::GetAllFeaStructs()
{
    vector < FeaStructure* > feastructvec;

    Vehicle* veh = VehicleMgr.GetVehicle();

    if ( veh )
    {
        vector< Geom* > geom_vec = veh->FindGeomVec( veh->GetGeomVec() );

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

void StructureMgrSingleton::ResetAllExportFileNames()
{
    vector < FeaStructure* > structs = GetAllFeaStructs();

    for ( unsigned int i = 0; i < structs.size(); i++ )
    {
        structs[i]->ResetExportFileNames();
    }

    for ( unsigned int i = 0; i < m_FeaAssemblyVec.size(); i++ )
    {
        m_FeaAssemblyVec[i]->ResetExportFileNames();
    }
}

//==== Get FeaStructure from Total Structure Index ====//
FeaStructure* StructureMgrSingleton::GetFeaStruct( int total_struct_ind )
{
    FeaStructure* fea_struct = NULL;

    vector < FeaStructure* > struct_vec = GetAllFeaStructs();

    if ( ValidTotalFeaStructInd( total_struct_ind ) )
    {
        fea_struct = struct_vec[total_struct_ind];
    }
    return fea_struct;
}

//==== Get FeaStructure from Structure ID ====//
FeaStructure* StructureMgrSingleton::GetFeaStruct( const string & struct_id )
{
    FeaStructure* fea_struct = NULL;

    vector < FeaStructure* > struct_vec = GetAllFeaStructs();

    for ( size_t i = 0; i < struct_vec.size(); i++ )
    {
        if ( strcmp( struct_vec[i]->GetID().c_str(), struct_id.c_str() ) == 0 )
        {
            fea_struct = struct_vec[i];
        }
    }
    return fea_struct;
}

//==== Get FeaStructure Parent Geom ID from Structure ID ====//
string StructureMgrSingleton::GetFeaStructParentID( const string & struct_id )
{
    FeaStructure* fea_struct = GetFeaStruct( struct_id );
    if ( fea_struct )
    {
        return fea_struct->GetParentGeomID();
    }
    return string(); // indicates an error
}

//==== Validate FeaStructure Index ====//
bool StructureMgrSingleton::ValidTotalFeaStructInd( int index )
{
    vector < FeaStructure* > feastructvec = GetAllFeaStructs();

    if ( (int)feastructvec.size() > 0 && index >= 0 && index < (int)feastructvec.size() )
    {
        return true;
    }
    return false;
}

int StructureMgrSingleton::NumFeaStructures()
{
    return GetAllFeaStructs().size();
}

int StructureMgrSingleton::GetTotFeaStructIndex( FeaStructure* fea_struct )
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

//==== Get FeaStructure Geom Index Structure ID ====//
int StructureMgrSingleton::GetGeomFeaStructIndex( const string & struct_id )
{
    string parent_id = GetFeaStructParentID( struct_id );
    Vehicle* veh = VehicleMgr.GetVehicle();
    if ( veh )
    {
        Geom* parent_geom = veh->FindGeom( parent_id );
        if ( parent_geom )
        {
            return parent_geom->GetFeaStructIndex( struct_id );
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

//==== Get FeaPart pointer from FeaPart ID ====//
SubSurface* StructureMgrSingleton::GetFeaSubSurf( const string & feasubsurf_id )
{
    SubSurface* ret_ptr = NULL;

    vector < FeaStructure* > fea_struct_vec = GetAllFeaStructs();

    for ( unsigned int i = 0; i < fea_struct_vec.size(); i++ )
    {
        vector < SubSurface* > fea_subsurf_vec = fea_struct_vec[i]->GetFeaSubSurfVec();

        for ( unsigned int j = 0; j < fea_subsurf_vec.size(); j++ )
        {
            if ( fea_subsurf_vec[j]->GetID() == feasubsurf_id )
            {
                return fea_subsurf_vec[j];
            }
        }
    }

    return ret_ptr;
}


//==== Get FeaSubSurf index from SubSurf ID ====//
int StructureMgrSingleton::GetFeaSubSurfIndex( const string & ss_id )
{
    vector < FeaStructure* > fea_struct_vec = GetAllFeaStructs();

    for ( unsigned int i = 0; i < fea_struct_vec.size(); i++ )
    {
        vector < SubSurface* > ss_vec = fea_struct_vec[i]->GetFeaSubSurfVec();

        for ( unsigned int j = 0; j < ss_vec.size(); j++ )
        {
            if ( ss_vec[j]->GetID() == ss_id )
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

void StructureMgrSingleton::ShowAllParts()
{
    FeaStructure* fea_struct = GetFeaStruct( m_CurrStructIndex() );
    Vehicle* veh = VehicleMgr.GetVehicle();

    if ( fea_struct && veh )
    {
        string parent_id = fea_struct->GetParentGeomID();
        veh->HideAll();

        Geom* parent = veh->FindGeom( parent_id );
        if ( parent )
        {
            parent->SetSetFlag( vsp::SET_SHOWN, true ); //add to shown
            parent->SetSetFlag( vsp::SET_NOT_SHOWN, false ); //remove from show
        }

        vector < FeaPart* > part_vec = fea_struct->GetFeaPartVec();

        for ( size_t i = 0; i < part_vec.size(); i++ )
        {
            part_vec[i]->m_DrawFeaPartFlag.Set( true );
        }

        vector < SubSurface* > ss_vec = fea_struct->GetFeaSubSurfVec();

        for ( size_t i = 0; i < ss_vec.size(); i++ )
        {
            ss_vec[i]->m_DrawFeaPartFlag.Set( true );
        }
    }
}

void StructureMgrSingleton::HideAllParts()
{
    FeaStructure* fea_struct = GetFeaStruct( m_CurrStructIndex() );
    Vehicle* veh = VehicleMgr.GetVehicle();

    if ( fea_struct && veh )
    {
        string parent_id = fea_struct->GetParentGeomID();
        veh->HideAll();

        vector < FeaPart* > part_vec = fea_struct->GetFeaPartVec();

        for ( size_t i = 0; i < part_vec.size(); i++ )
        {
            part_vec[i]->m_DrawFeaPartFlag.Set( false );
        }

        vector < SubSurface* > ss_vec = fea_struct->GetFeaSubSurfVec();

        for ( size_t i = 0; i < ss_vec.size(); i++ )
        {
            ss_vec[i]->m_DrawFeaPartFlag.Set( false );
        }
    }
}

//==== Add FeaProperty =====//
FeaProperty* StructureMgrSingleton::AddFeaProperty( int property_type )
{
    FeaProperty* feaprop = new FeaProperty();

    if ( feaprop )
    {
        feaprop->SetName( string( "PROPERTY_" + std::to_string( m_FeaPropertyCount ) ) );
        feaprop->m_FeaPropertyType.Set( property_type );
        m_FeaPropertyVec.push_back( feaprop );
        m_FeaPropertyCount++;
    }

    return feaprop;
}

//==== Delete FeaProperty =====//
void StructureMgrSingleton::DeleteFeaProperty( string id )
{
    vector < FeaProperty* > newpropvec;

    for ( size_t i = 0; i < m_FeaPropertyVec.size(); i++ )
    {
        if ( m_FeaPropertyVec[i]->GetID() == id )
        {
            delete m_FeaPropertyVec[i];
        }
        else
        {
            newpropvec.push_back( m_FeaPropertyVec[i] );
        }
    }
    m_FeaPropertyVec = newpropvec;
}

//==== Validate FeaProperty Index ====//
bool StructureMgrSingleton::ValidFeaPropertyInd( int index )
{
    if ( (int)m_FeaPropertyVec.size() > 0 && index >= 0 && index < (int)m_FeaPropertyVec.size() )
    {
        return true;
    }
    return false;
}

FeaProperty* StructureMgrSingleton::GetFeaProperty( string id )
{
    if ( id == string( "NONE" ) )
    {
        return NULL;
    }
    for ( int i = 0 ; i < ( int )m_FeaPropertyVec.size() ; i++ )
    {
        if ( m_FeaPropertyVec[i]->GetID() == id )
        {
            return m_FeaPropertyVec[i];
        }
    }
    return NULL;
}

string StructureMgrSingleton::MakeDefaultShellProperty()
{
    FeaProperty* default_shell = new FeaProperty();

    default_shell->SetName( "DefaultShell" );
    default_shell->m_FeaMaterialID = "_Al6061T6";
    default_shell->m_FeaPropertyType.Set( vsp::FEA_SHELL );

    AddFeaProperty( default_shell );

    return default_shell->GetID();
}

string StructureMgrSingleton::MakeDefaultBeamProperty()
{
    FeaProperty* default_beam = new FeaProperty();

    default_beam->SetName( "DefaultBeam" );
    default_beam->m_FeaMaterialID = "_Al6061T6";
    default_beam->m_FeaPropertyType.Set( vsp::FEA_BEAM );

    AddFeaProperty( default_beam );

    return default_beam->GetID();
}

string StructureMgrSingleton::GetSomeShellProperty()
{
    int ifirstshell = -1;

    for ( size_t i = 0; i < m_FeaPropertyVec.size(); i++ )
    {
        if ( m_FeaPropertyVec[i]->m_FeaPropertyType() == vsp::FEA_SHELL )
        {
            if ( ifirstshell == -1 )
            {
                ifirstshell = i;
            }

            // If Default exists, return it.
            if ( m_FeaPropertyVec[i]->GetName() == "DefaultShell" )
            {
                return m_FeaPropertyVec[i]->GetID();
            }
        }
    }

    // No default exists, but a shell property does exist, return the first one encountered.
    if ( ifirstshell >= 0 )
    {
        return m_FeaPropertyVec[ ifirstshell ]->GetID();
    }

    // No shell property exists, make a default one and return it.
    return MakeDefaultShellProperty();
}

string StructureMgrSingleton::GetSomeBeamProperty()
{
    int ifirstbeam = -1;

    for ( size_t i = 0; i < m_FeaPropertyVec.size(); i++ )
    {
        if ( m_FeaPropertyVec[i]->m_FeaPropertyType() == vsp::FEA_BEAM )
        {
            if ( ifirstbeam == -1 )
            {
                ifirstbeam = i;
            }

            // If Default exists, return it.
            if ( m_FeaPropertyVec[i]->GetName() == "DefaultBeam" )
            {
                return m_FeaPropertyVec[i]->GetID();
            }
        }
    }

    // No default exists, but a beam property does exist, return the first one encountered.
    if ( ifirstbeam >= 0 )
    {
        return m_FeaPropertyVec[ ifirstbeam ]->GetID();
    }

    // No beam property exists, make a default one and return it.
    return MakeDefaultBeamProperty();
}

void StructureMgrSingleton::InitFeaProperties()
{
    // Check for existence of shell or beam property
    bool shell_prop = false;
    bool beam_prop = false;

    for ( size_t i = 0; i < m_FeaPropertyVec.size(); i++ )
    {
        if ( m_FeaPropertyVec[i]->m_FeaPropertyType() == vsp::FEA_SHELL )
        {
            shell_prop = true;
        }
        else if ( m_FeaPropertyVec[i]->m_FeaPropertyType() == vsp::FEA_BEAM )
        {
            beam_prop = true;
        }
    }

    if ( !shell_prop )
    {
        MakeDefaultShellProperty();
    }

    if ( !beam_prop )
    {
        MakeDefaultBeamProperty();
    }
}

//==== Add FeaMaterial =====//
FeaMaterial* StructureMgrSingleton::AddFeaMaterial()
{
    FeaMaterial* feamat = new FeaMaterial();

    if ( feamat )
    {
        feamat->SetName( string( "Material" + std::to_string( m_FeaMatCount ) ), false ); // false is for removeslashes
        feamat->m_UserFeaMaterial = true;
        m_FeaMaterialVec.push_back( feamat );
        m_FeaMatCount++;
    }

    return feamat;
}

//==== Delete FeaMaterial =====//
bool StructureMgrSingleton::DeleteFeaMaterial( string id )
{
    bool delsuccess = false;
    vector < FeaMaterial* > newmatvec;

    for ( size_t i = 0; i < m_FeaMaterialVec.size(); i++ )
    {
        // Only allow deletion of user materials.
        if ( m_FeaMaterialVec[i]->GetID() == id && m_FeaMaterialVec[i]->m_UserFeaMaterial == true )
        {
            delete m_FeaMaterialVec[i];
            delsuccess = true;
        }
        else
        {
            newmatvec.push_back( m_FeaMaterialVec[i] );
        }
    }
    m_FeaMaterialVec = newmatvec;
    return delsuccess;
}

//==== Validate FeaMaterial Index ====//
bool StructureMgrSingleton::ValidFeaMaterialInd( int index )
{
    if ( (int)m_FeaMaterialVec.size() > 0 && index >= 0 && index < (int)m_FeaMaterialVec.size() )
    {
        return true;
    }
    return false;
}

FeaMaterial* StructureMgrSingleton::GetFeaMaterial( string id )
{
    if ( id == string( "NONE" ) )
    {
        return NULL;
    }
    for ( int i = 0 ; i < ( int )m_FeaMaterialVec.size() ; i++ )
    {
        if ( m_FeaMaterialVec[i]->GetID() == id )
        {
            return m_FeaMaterialVec[i];
        }
    }
    return NULL;
}

void StructureMgrSingleton::InitFeaMaterials()
{
    int nmat = 16;
    const char *matids[] = {"_Al7075T6",
                            "_Al6061T6",
                            "_Al2024T3",
                            "_Ti6Al4V",
                            "_CrMo4130",
                            "_AS4-1",
                            "_AS4-2",
                            "_AS4-3",
                            "_AS4-4",
                            "_AS4-5",
                            "_AS4-6",
                            "_S2-1",
                            "_S2-2",
                            "_S2-3",
                            "_Balsa",
                            "_Spruce"
    };

    for ( int i = 0; i < nmat; i++ )
    {
        FeaMaterial* mat = new FeaMaterial();
        mat->MakeMaterial( matids[i] );
        mat->Update();
        AddFeaMaterial( mat );
    }
}

//==== Get FeaAssembly =====//
FeaAssembly* StructureMgrSingleton::GetFeaAssembly( const string & assyid )
{
    for ( unsigned int i = 0; i < m_FeaAssemblyVec.size(); i++ )
    {
        FeaAssembly* assy_ptr = m_FeaAssemblyVec[i];
        if ( assy_ptr )
        {
            if ( assy_ptr->GetID() == assyid )
            {
                return assy_ptr;
            }
        }
    }
    return NULL;
}

//==== Add FeaProperty =====//
FeaAssembly* StructureMgrSingleton::AddFeaAssembly( )
{
    FeaAssembly* feaassy = new FeaAssembly();

    if ( feaassy )
    {
        feaassy->SetName( string( "ASSEMBLY_" + std::to_string( m_FeaAssemblyCount ) ) );

        feaassy->ResetExportFileNames();

        m_FeaAssemblyVec.push_back( feaassy );
        m_FeaAssemblyCount++;
    }

    return feaassy;
}

//==== Delete FeaProperty =====//
void StructureMgrSingleton::DeleteFeaAssembly( int index )
{
    if ( !ValidFeaAssemblyInd( index ) )
        return;

    delete m_FeaAssemblyVec[index];
    m_FeaAssemblyVec.erase( m_FeaAssemblyVec.begin() + index );
}

//==== Validate FeaProperty Index ====//
bool StructureMgrSingleton::ValidFeaAssemblyInd( int index )
{
    if ( (int)m_FeaAssemblyVec.size() > 0 && index >= 0 && index < (int)m_FeaAssemblyVec.size() )
    {
        return true;
    }
    return false;
}

FeaAssembly* StructureMgrSingleton::GetFeaAssembly( int index )
{
    if ( ValidFeaAssemblyInd( index ) )
    {
        return m_FeaAssemblyVec[index];
    }
    return NULL;
}
