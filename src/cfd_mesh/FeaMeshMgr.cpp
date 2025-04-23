//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//

// FeaMeshMgr.cpp
//
//////////////////////////////////////////////////////////////////////

#include "FeaMeshMgr.h"
#include "SubSurfaceMgr.h"
#include "StructureMgr.h"
#include "main.h"
#include "StringUtil.h"
#include "MeshAnalysis.h"
#include "StlHelper.h"
#include "MessageMgr.h"
#include "VspUtil.h"

//=============================================================//
//=============================================================//

FeaCount::FeaCount()
{
    m_NumNodes = 0;
    m_NumEls = 0;
    m_NumTris = 0;
    m_NumQuads = 0;
    m_NumBeams = 0;
}

//=============================================================//
//=============================================================//

FeaMeshMgrSingleton::FeaMeshMgrSingleton() : CfdMeshMgrSingleton()
{
    m_FeaMeshInProgress = false;
    m_CADOnlyFlag = false;

    m_MessageName = "FEAMessage";

    m_IntersectStructID = string();
    m_IntersectComplete = false;

    m_FeaStructID = string();
    m_ActiveMesh = NULL;
}

FeaMeshMgrSingleton::~FeaMeshMgrSingleton()
{
    CleanMeshMap();

    CleanUp();
}

// Cleanup done between mesh generation runs.
void FeaMeshMgrSingleton::CleanUp()
{
    CfdMeshMgrSingleton::CleanUp();

    m_SimplePropertyVec.clear();
    m_SimpleMaterialVec.clear();

    m_IntersectComplete = false;
}

// Cleanup done on file-load.
void FeaMeshMgrSingleton::CleanMeshMap()
{
    meshmaptype::iterator it = m_MeshPtrMap.begin();

    while ( it != m_MeshPtrMap.end() )
    {
        if ( m_ActiveMesh == it->second )
        {
            m_ActiveMesh = NULL;
        }
        delete it->second;
        it++;
    }
    m_MeshPtrMap.clear();
}

void FeaMeshMgrSingleton::SetActiveMesh( string struct_id )
{
    meshmaptype::iterator it = m_MeshPtrMap.find( struct_id );

    if ( it == m_MeshPtrMap.end() )
    {
        FeaMesh * fm = new FeaMesh( struct_id );
        m_MeshPtrMap[ struct_id ] = fm;
    }

    m_ActiveMesh = m_MeshPtrMap[ struct_id ];
}

FeaMesh* FeaMeshMgrSingleton::GetMeshPtr( string struct_id )
{
    meshmaptype::iterator it = m_MeshPtrMap.find( struct_id );

    if ( it == m_MeshPtrMap.end() )
    {
        return NULL;
    }
    return m_MeshPtrMap[ struct_id ];
}

bool FeaMeshMgrSingleton::LoadSurfaces()
{
    CleanUp();

    // Clean the active Mesh - leave others un-touched.
    if ( GetMeshPtr() )
    {
        GetMeshPtr()->Cleanup();
    }

    // Identify the structure to mesh (m_FeaMeshStructIndex must be set) 
    FeaStructure* fea_struct = StructureMgr.GetFeaStruct( m_FeaStructID );

    if ( !fea_struct )
    {
        addOutputText( "FeaMesh Failed: Invalid FeaStructure Selection\n" );
        m_FeaMeshInProgress = false;
        return false;
    }

    // Save structure name
    GetMeshPtr()->m_StructName = fea_struct->GetName();

    // Identify number of FeaParts
    GetMeshPtr()->m_NumFeaParts = fea_struct->NumFeaParts();

    // Identify number of FeaFixPoints
    GetMeshPtr()->m_NumFeaFixPoints = fea_struct->GetNumFeaFixPoints();

    LoadSkins();

    return true;
}

void FeaMeshMgrSingleton::LoadSkins()
{
    FeaStructure* fea_struct = StructureMgr.GetFeaStruct( m_FeaStructID );

    if ( fea_struct && GetMeshPtr() )
    {
        FeaPart* prt = fea_struct->GetFeaSkin();

        if ( prt )
        {
            FeaSkin* skin = dynamic_cast<FeaSkin*>( prt );
            assert( skin );

            fea_struct->BuildSuppressList();

            //===== Add FeaSkins ====//
            vector< XferSurf > skinxfersurfs;

            int skin_index = fea_struct->GetFeaPartIndex( prt );

            skin->FetchFeaXFerSurf( skinxfersurfs, 0, fea_struct->GetUSuppress(), fea_struct->GetWSuppress() );

            // Load Skin XFerSurf to m_SurfVec
            LoadSurfs( skinxfersurfs, GetMeshPtr()->m_LenScale );

            // begin should be zero here, but we copy the logic from AddStructureSurfParts for readability.
            int begin = m_SurfVec.size() - skinxfersurfs.size();
            int end = m_SurfVec.size();

            for ( int j = begin; j < end; j++ )
            {
                m_SurfVec[j]->SetFeaPartIndex( skin_index );

                if ( skin->m_RemoveSkinFlag() )
                {
                    m_SurfVec[j]->SetIgnoreSurfFlag( true );
                }

                if ( skin->m_CapUMinSuccess &&
                     skin->m_RemoveRootCapFlag() &&
                     m_SurfVec[j]->GetSurfCore()->GetMinU() < 1.0 )
                {
                    m_SurfVec[j]->SetIgnoreSurfFlag( true );
                }

                if ( skin->m_CapUMaxSuccess &&
                     skin->m_RemoveTipCapFlag() &&
                     m_SurfVec[j]->GetSurfCore()->GetMaxU() > ( skin->m_UMax - 1.0 ) )
                {
                    m_SurfVec[j]->SetIgnoreSurfFlag( true );
                }
            }

        }
    }
}

void FeaMeshMgrSingleton::TransferMeshSettings()
{
    FeaStructure* fea_struct = StructureMgr.GetFeaStruct( m_FeaStructID );

    if ( fea_struct && GetMeshPtr() )
    {
        GetMeshPtr()->m_StructSettings = SimpleFeaMeshSettings();
        GetMeshPtr()->m_StructSettings.CopyFrom( fea_struct->GetStructSettingsPtr() );

        GetMeshPtr()->m_FeaGridDensity = SimpleFeaGridDensity();
        GetMeshPtr()->m_FeaGridDensity.CopyFrom( fea_struct->GetFeaGridDensityPtr() );

        if ( GetMeshPtr()->m_StructSettings.m_ConvertToQuadsFlag )
        {
            // Increase target edge length because tris are split into quads.
            // A tri with edge length 1.0 will result in an average quad edge of 0.349
            GetMeshPtr()->m_FeaGridDensity.ScaleMesh( 2.536 );
        }
    }

    GetMeshPtr()->m_LenScale = m_Vehicle->ComputeStructuresScaleFactor();

    char buf[255];
    snprintf( buf, sizeof( buf ), "Scaling lengths by %f\n", GetMeshPtr()->m_LenScale );
    addOutputText( buf );
}

void FeaMeshMgrSingleton::IdentifyCompIDNames()
{
    m_CompIDNameMap.clear();

    if ( !GetMeshPtr() )
    {
        return;
    }

    for ( size_t i = 0; i < m_SurfVec.size(); i++ )
    {
        if ( m_CompIDNameMap.count( m_SurfVec[i]->GetFeaPartIndex() ) == 0 )
        {
            m_CompIDNameMap[m_SurfVec[i]->GetFeaPartIndex()] = GetMeshPtr()->m_StructName + "_" + GetMeshPtr()->m_FeaPartNameVec[m_SurfVec[i]->GetFeaPartIndex()];
        }
    }
}

void FeaMeshMgrSingleton::GetMassUnit()
{
    if ( !GetMeshPtr() )
    {
        return;
    }

    switch ( m_Vehicle->m_StructUnit() )
    {
        case vsp::SI_UNIT:
            GetMeshPtr()->m_MassUnit = "kg";
            break;

        case vsp::CGS_UNIT:
            GetMeshPtr()->m_MassUnit = "g";
            break;

        case vsp::MPA_UNIT:
            GetMeshPtr()->m_MassUnit = "tonne"; // or Mg/
            break;

        case vsp::BFT_UNIT:
            GetMeshPtr()->m_MassUnit = "slug";
            break;

        case vsp::BIN_UNIT:
            GetMeshPtr()->m_MassUnit = "lbf*sec" + string( 1, (char) 178 ) + "/in";
            break;
    }
}

void FeaMeshMgrSingleton::TransferFeaData()
{

    vector < string > prop_id_vec( m_SimplePropertyVec.size() );
    for ( int i = 0; i < m_SimplePropertyVec.size(); i++ )
    {
        prop_id_vec[i] = m_SimplePropertyVec[i].m_ID;
    }

    // Transfer FeaPart Data
    FeaStructure* fea_struct = StructureMgr.GetFeaStruct( m_FeaStructID );

    if ( fea_struct && GetMeshPtr() )
    {
        vector < FeaPart* > fea_part_vec = fea_struct->GetFeaPartVec();
        GetMeshPtr()->m_FeaPartNameVec.resize( GetMeshPtr()->m_NumFeaParts );
        GetMeshPtr()->m_FeaPartIDVec.resize( GetMeshPtr()->m_NumFeaParts );
        GetMeshPtr()->m_FeaPartTypeVec.resize( GetMeshPtr()->m_NumFeaParts );
        GetMeshPtr()->m_FeaPartNumSurfVec.resize( GetMeshPtr()->m_NumFeaParts );
        GetMeshPtr()->m_FeaPartKeepDelShellElementsVec.resize( GetMeshPtr()->m_NumFeaParts );
        GetMeshPtr()->m_FeaPartCreateBeamElementsVec.resize( GetMeshPtr()->m_NumFeaParts );
        GetMeshPtr()->m_FeaPartPropertyIndexVec.resize( GetMeshPtr()->m_NumFeaParts );
        GetMeshPtr()->m_FeaPartCapPropertyIndexVec.resize( GetMeshPtr()->m_NumFeaParts );
        GetMeshPtr()->m_FeaPartPropertyIDVec.resize( GetMeshPtr()->m_NumFeaParts );
        GetMeshPtr()->m_FeaPartCapPropertyIDVec.resize( GetMeshPtr()->m_NumFeaParts );
        GetMeshPtr()->m_FeaPartNumChainsVec.resize( GetMeshPtr()->m_NumFeaParts );

        for ( size_t i = 0; i < fea_part_vec.size(); i++ )
        {
            GetMeshPtr()->m_FeaPartNameVec[i] = fea_part_vec[i]->GetName();
            GetMeshPtr()->m_FeaPartIDVec[i] = fea_part_vec[i]->GetID();
            GetMeshPtr()->m_FeaPartTypeVec[i] = fea_part_vec[i]->GetType();
            GetMeshPtr()->m_FeaPartNumSurfVec[i] = fea_part_vec[i]->NumFeaPartSurfs();
            GetMeshPtr()->m_FeaPartKeepDelShellElementsVec[i] = fea_part_vec[i]->m_KeepDelShellElements();
            GetMeshPtr()->m_FeaPartCreateBeamElementsVec[i] = fea_part_vec[i]->m_CreateBeamElements();
            GetMeshPtr()->m_FeaPartPropertyIndexVec[i] = vector_find_val( prop_id_vec, fea_part_vec[i]->m_FeaPropertyID );
            GetMeshPtr()->m_FeaPartCapPropertyIndexVec[i] = vector_find_val( prop_id_vec, fea_part_vec[i]->m_CapFeaPropertyID );
            GetMeshPtr()->m_FeaPartPropertyIDVec[i] = fea_part_vec[i]->m_FeaPropertyID;
            GetMeshPtr()->m_FeaPartCapPropertyIDVec[i] = fea_part_vec[i]->m_CapFeaPropertyID;
            GetMeshPtr()->m_FeaPartNumChainsVec[i] = 0;
        }
    }
}

void FeaMeshMgrSingleton::TransferPropMatData()
{
    // Transfer FeaMaterial Data
    vector < FeaMaterial* > fea_mat_vec = StructureMgr.GetFeaMaterialVec();
    m_SimpleMaterialVec.resize( fea_mat_vec.size() );
    vector < string > mat_id_vec( fea_mat_vec.size() );

    for ( size_t i = 0; i < fea_mat_vec.size(); i++ )
    {
        fea_mat_vec[i]->Update();
        m_SimpleMaterialVec[i] = SimpleFeaMaterial();
        m_SimpleMaterialVec[i].CopyFrom( fea_mat_vec[i] );
        mat_id_vec[i] = fea_mat_vec[i]->GetID();
    }

    // Transfer FeaProperty Data
    vector < FeaProperty* > fea_prop_vec = StructureMgr.GetFeaPropertyVec();
    m_SimplePropertyVec.resize( fea_prop_vec.size() );

    for ( size_t i = 0; i < fea_prop_vec.size(); i++ )
    {
        fea_prop_vec[i]->Update();
        m_SimplePropertyVec[i] = SimpleFeaProperty();
        m_SimplePropertyVec[i].CopyFrom( fea_prop_vec[i], mat_id_vec );
    }

}

void FeaMeshMgrSingleton::TransferSubSurfData()
{
    FeaStructure* fea_struct = StructureMgr.GetFeaStruct( m_FeaStructID );

    vector < string > prop_id_vec( m_SimplePropertyVec.size() );
    for ( int i = 0; i < m_SimplePropertyVec.size(); i++ )
    {
        prop_id_vec[i] = m_SimplePropertyVec[i].m_ID;
    }

    if ( fea_struct )
    {
        vector < SubSurface* > fea_ss_vec = fea_struct->GetFeaSubSurfVec();
        m_SimpleSubSurfaceVec.resize( fea_ss_vec.size() );

        for ( size_t i = 0; i < fea_ss_vec.size(); i++ )
        {
            m_SimpleSubSurfaceVec[i] = SimpleSubSurface();
            m_SimpleSubSurfaceVec[ i ].CopyFrom( fea_ss_vec[ i ], prop_id_vec );
        }
    }

    if ( GetMeshPtr() )
    {
        // Identify number of FeaSubSurfaces
        GetMeshPtr()->m_NumFeaSubSurfs = m_SimpleSubSurfaceVec.size();
        // Duplicate subsurface data in mesh data structure so it will be available
        // after mesh generation is complete.
        GetMeshPtr()->m_SimpleSubSurfaceVec = m_SimpleSubSurfaceVec;
    }
}

void FeaMeshMgrSingleton::TransferBCData()
{
    FeaStructure* fea_struct = StructureMgr.GetFeaStruct( m_FeaStructID );

    if ( fea_struct && GetMeshPtr() )
    {
        vector< FeaBC* > bc_vec = fea_struct->GetFeaBCVec();

        int nbc = bc_vec.size();
        GetMeshPtr()->m_BCVec.resize( nbc );

        for ( int i = 0; i < nbc; i++ )
        {
            GetMeshPtr()->m_BCVec[i].CopyFrom( bc_vec[i] );
        }
    }
}

bool FeaMeshMgrSingleton::CheckPropMat()
{
    bool pass = true;

    char buf[512];

    for ( size_t i = 0; i < m_SimplePropertyVec.size(); i++ )
    {
        if ( m_SimplePropertyVec[ i ].GetSimpFeaMatIndex() == -1 )
        {
            snprintf( buf, sizeof( buf ), "Could not find material '%s' '%s'\n", m_SimplePropertyVec[ i ].m_FeaMatID.c_str(), m_SimplePropertyVec[ i ].m_MaterialName.c_str() );
            addOutputText( string( buf ) );
            pass = false;
        }
    }

    for ( size_t i = 0; i < m_SimpleSubSurfaceVec.size(); i++ )
    {
        if ( m_SimpleSubSurfaceVec[ i ].m_KeepDelShellElements == vsp::FEA_KEEP )
        {
            if ( m_SimpleSubSurfaceVec[ i ].GetFeaPropertyIndex() == -1 )
            {
                snprintf( buf, sizeof( buf ), "Could not find subsurface shell property '%s'\n", m_SimpleSubSurfaceVec[ i ].GetFeaPropertyID().c_str() );
                addOutputText( string( buf ) );
                pass = false;
            }
        }
        if ( m_SimpleSubSurfaceVec[ i ].m_CreateBeamElements )
        {
            if ( m_SimpleSubSurfaceVec[ i ].GetCapFeaPropertyIndex() == -1 )
            {
                snprintf( buf, sizeof( buf ), "Could not find subsurface cap property '%s'\n", m_SimpleSubSurfaceVec[ i ].GetCapFeaPropertyID().c_str() );
                addOutputText( string( buf ) );
                pass = false;
            }
        }
    }

    if ( GetMeshPtr() )
    {
        for ( size_t i = 0; i < GetMeshPtr()->m_FeaPartPropertyIndexVec.size(); i++ )
        {
            if ( GetMeshPtr()->m_FeaPartKeepDelShellElementsVec[i] == vsp::FEA_KEEP )
            {
                if ( GetMeshPtr()->m_FeaPartPropertyIndexVec[ i ] == -1 )
                {
                    snprintf( buf, sizeof( buf ), "Could not find part shell property '%s'\n", GetMeshPtr()->m_FeaPartPropertyIDVec[ i ].c_str() );
                    addOutputText( string( buf ) );
                    pass = false;
                }
            }
        }

        for ( size_t i = 0; i < GetMeshPtr()->m_FeaPartCapPropertyIndexVec.size(); i++ )
        {
            if ( GetMeshPtr()->m_FeaPartCreateBeamElementsVec[i] )
            {
                if ( GetMeshPtr()->m_FeaPartCapPropertyIndexVec[ i ] == -1 )
                {
                    snprintf( buf, sizeof( buf ), "Could not find part cap property '%s'\n", GetMeshPtr()->m_FeaPartCapPropertyIDVec[ i ].c_str() );
                    addOutputText( string( buf ) );
                    pass = false;
                }
            }
        }
    }

    return pass;
}

void FeaMeshMgrSingleton::GenerateFeaMesh()
{
    m_FeaMeshInProgress = true;

#ifdef DEBUG_TIME_OUTPUT
    addOutputText( "Init Timer\n" );
#endif

    FeaStructure* fea_struct = StructureMgr.GetFeaStruct( m_FeaStructID );
    if ( fea_struct )
    {
        fea_struct->Update();
    }

    addOutputText( "Transfer Mesh Settings\n" );
    TransferMeshSettings();

    addOutputText( "Load Surfaces\n" );
    LoadSurfaces();

    if ( m_SurfVec.size() == 0 )
    {
        addOutputText( "No Surfaces.  Done.\n" );
        m_FeaMeshInProgress = false;
        MessageMgr::getInstance().Send( "ScreenMgr", "UpdateAllScreens" );
        return;
    }

    if ( !m_CADOnlyFlag )
    {
        // Hide all geoms after loading surfaces and settings
        m_Vehicle->HideAll();
    }

    GetMassUnit();

    // Transfer common property and material data to FeaMeshMgr.
    // This is repeated in ExportAssemblyMesh() and ExportFeaMesh().
    // Part-property associations can not change, but material thicknesses and dimensions can.
    // This allows some level of structural redesign without regenerating a mesh.
    TransferPropMatData();

    addOutputText( "Transfer FEA Data\n" );
    TransferFeaData();

    addOutputText( "Transfer Subsurf Data\n" );
    TransferSubSurfData();

    if ( !CheckPropMat() )
    {
        addOutputText( "Material or property not identified.\n" );
        m_FeaMeshInProgress = false;
        MessageMgr::getInstance().Send( "ScreenMgr", "UpdateAllScreens" );
        return;
    }

    // Needs to be after TransferSubSurfData so SubSurf BC's can be indexed.
    // Needs to be after TransferFeaData() so FeaParts can be indexed.
    TransferBCData();

    TransferDrawObjData();

    addOutputText( "Merge Co-Planar Parts\n" );
    MergeCoplanarParts();

    addOutputText( "Add Structure Parts\n" );
    AddStructureSurfParts();

    addOutputText( "Clean Merge Surfs\n" );
    CleanMergeSurfs( /* skip_duplicate_removal */ true ); // Must be called before AddStructureFixPoints to prevent FEA Fix Point surface misidentification

    // Must be after CleanMergeSurfs() as that is when m_SurfVec is last updated.
    BuildMeshOrientationLookup();

    addOutputText( "Add Structure Fix Points\n" );
    AddStructureFixPoints();

    addOutputText( "Add Structure Trim Planes\n" );
    AddStructureTrimPlanes();

    addOutputText( "Identify CompID Names\n" );
    IdentifyCompIDNames();

    // TODO: Update and Build Domain for Half Mesh?

    addOutputText( "Build Slice Planes\n" );
    BuildGrid();

    // addOutputText( "Intersect\n" ); // Output in intersect() itself.
    Intersect();

    if ( false )
    {
    addOutputText( "Degen Corners\n" );
    FindDegenCorners();

    addOutputText( "Add Degen Corner Chains\n" );
    AddDegenCornerChains();
    }

    addOutputText( "Binary Adaptation Curve Approximation\n" );
    BinaryAdaptIntCurves();

    m_IntersectComplete = true;

    if ( m_CADOnlyFlag )
    {
        UpdateDrawObjs();

        addOutputText( "Finished\n" );

        m_FeaMeshInProgress = false;
        m_CADOnlyFlag = false;
        MessageMgr::getInstance().Send( "ScreenMgr", "UpdateAllScreens" );
        return;
    }

    addOutputText( "Build Target Map\n" );
    BuildTargetMap( CfdMeshMgrSingleton::VOCAL_OUTPUT );

    // addOutputText( "InitMesh\n" ); // Output inside InitMesh
    InitMesh();

    addOutputText( "Sub Tag Tris\n" );
    SubTagTris();

    addOutputText( "Set Fixed Points\n" );
    SetFixPointSurfaceNodes();

    addOutputText( "Remesh\n" );
    Remesh( CfdMeshMgrSingleton::VOCAL_OUTPUT );

    if ( GetMeshPtr() && GetMeshPtr()->m_StructSettings.m_ConvertToQuadsFlag )
    {
        addOutputText( "ConvertToQuads\n" );
        ConvertToQuads();
    }

    addOutputText( "ConnectBorderNodes\n" );
    ConnectBorderNodes( false );        // No Wakes
    ConnectBorderNodes( true );         // Only Wakes

    addOutputText( "Post Mesh\n" );
    PostMesh();

    addOutputText( "Build Single Tag Map\n" );
    SubSurfaceMgr.BuildSingleTagMap();

    addOutputText( "Check Subsurf Border Intersect\n" );
    CheckSubSurfBorderIntersect();

    addOutputText( "Check Duplicate Subsurface Intersects\n" );
    CheckDuplicateSSIntersects();

    addOutputText( "Build Fea Mesh\n" );
    BuildFeaMesh();

    addOutputText( "Remove Subsurf FEA Tris\n" );
    RemoveSubSurfFeaTris();

    addOutputText( "Tag Fea Nodes\n" );
    TagFeaNodes();

    GetMeshPtr()->m_MeshReady = true;

    UpdateDrawObjs();

    addOutputText( "Finished\n" );

    m_FeaMeshInProgress = false;
    MessageMgr::getInstance().Send( "ScreenMgr", "UpdateAllScreens" );
}

void FeaMeshMgrSingleton::ExportFeaMesh( string structID )
{
    FeaStructure* fea_struct = StructureMgr.GetFeaStruct( structID );
    FeaMesh* mesh = GetMeshPtr( structID );

    if ( mesh && fea_struct )
    {
        mesh->m_StructSettings.CopyPostOpFrom( fea_struct->GetStructSettingsPtr() );

        TransferPropMatData();

        mesh->ExportFeaMesh();
    }
}

void FeaMeshMgrSingleton::ExportCADFiles()
{
    if ( !m_IntersectComplete )
    {
        return;
    }

    FeaStructure* fea_struct = StructureMgr.GetFeaStruct( m_FeaStructID );

    if ( fea_struct && GetMeshPtr() )
    {
        GetMeshPtr()->m_StructSettings.CopyPostOpFrom( fea_struct->GetStructSettingsPtr());
    }

    if ( GetSettingsPtr()->GetExportFileFlag( vsp::FEA_SRF_FILE_NAME ) )
    {
        WriteSurfsIntCurves( GetSettingsPtr()->GetExportFileName( vsp::FEA_SRF_FILE_NAME ) );
    }

    if ( GetSettingsPtr()->GetExportFileFlag( vsp::FEA_CURV_FILE_NAME ) )
    {
        WriteGridToolCurvFile( GetSettingsPtr()->GetExportFileName( vsp::FEA_CURV_FILE_NAME ),
                               GetSettingsPtr()->m_ExportRawFlag );
    }

    if ( GetSettingsPtr()->GetExportFileFlag( vsp::FEA_PLOT3D_FILE_NAME ) )
    {
        WritePlot3DFile( GetSettingsPtr()->GetExportFileName( vsp::FEA_PLOT3D_FILE_NAME ),
                         GetSettingsPtr()->m_ExportRawFlag );
    }

    if ( GetSettingsPtr()->GetExportFileFlag( vsp::FEA_IGES_FILE_NAME ) || GetSettingsPtr()->GetExportFileFlag( vsp::FEA_STEP_FILE_NAME ) )
    {
        BuildNURBSCurvesVec(); // Note: Must be called before BuildNURBSSurfMap

        BuildNURBSSurfMap();
    }

    if ( GetSettingsPtr()->GetExportFileFlag( vsp::FEA_IGES_FILE_NAME ) )
    {
        string delim = StringUtil::get_delim( GetSettingsPtr()->m_CADLabelDelim );

        WriteIGESFile( GetSettingsPtr()->GetExportFileName( vsp::FEA_IGES_FILE_NAME ), GetSettingsPtr()->m_CADLenUnit,
                       GetSettingsPtr()->m_CADLabelID, GetSettingsPtr()->m_CADLabelSurfNo, GetSettingsPtr()->m_CADLabelSplitNo,
                       GetSettingsPtr()->m_CADLabelName, delim );
    }

    if ( GetSettingsPtr()->GetExportFileFlag( vsp::FEA_STEP_FILE_NAME ) )
    {
        string delim = StringUtil::get_delim( GetSettingsPtr()->m_CADLabelDelim );

        WriteSTEPFile( GetSettingsPtr()->GetExportFileName( vsp::FEA_STEP_FILE_NAME ), GetSettingsPtr()->m_CADLenUnit,
                       GetSettingsPtr()->m_STEPTol, GetSettingsPtr()->m_STEPMergePoints,
                       GetSettingsPtr()->m_CADLabelID, GetSettingsPtr()->m_CADLabelSurfNo, GetSettingsPtr()->m_CADLabelSplitNo,
                       GetSettingsPtr()->m_CADLabelName, delim, GetSettingsPtr()->m_STEPRepresentation );
    }

}

void FeaMeshMgrSingleton::MergeCoplanarParts()
{
    FeaStructure* fea_struct = StructureMgr.GetFeaStruct( m_FeaStructID );

    vector < VspSurf > all_surf_vec;
    vector < vec3d > all_norm_vec;
    vector < int > all_feaprt_ind_vec;
    vector < int > feaprt_surf_ind_vec;

    if ( fea_struct && GetMeshPtr() )
    {
        vector < FeaPart* > fea_part_vec = fea_struct->GetFeaPartVec();

        // Collect all surfaces and compute norms
        for ( size_t k = 0; k < GetMeshPtr()->m_NumFeaParts; k++ )
        {
            if ( fea_part_vec[k]->GetType() != vsp::FEA_DOME && fea_part_vec[k]->GetType() != vsp::FEA_SKIN )
            {
                vector < VspSurf > surf_vec = fea_part_vec[k]->GetFeaPartSurfVec();

                for ( size_t j = 0; j < surf_vec.size(); j++ )
                {
                    all_surf_vec.push_back( surf_vec[j] );
                    all_feaprt_ind_vec.push_back( k );
                    all_norm_vec.push_back( surf_vec[j].CompNorm01( 0.5, 0.5 ) );
                    feaprt_surf_ind_vec.push_back( j );
                }
            }
        }

        for ( size_t i = 0; i < (int)all_norm_vec.size(); i++ )
        {
            for ( size_t j = i + 1; j < (int)all_norm_vec.size(); j++ )
            {
                if ( ( std::abs( std::abs( dot( all_norm_vec[i], all_norm_vec[j] ) ) - 1.0 ) <= FLT_EPSILON ) && ( all_norm_vec[j].mag() >= FLT_EPSILON ) && ( all_norm_vec[i].mag() >= FLT_EPSILON ) )
                {
                    vec3d pntA = all_surf_vec[i].CompPnt01( 0.5, 0.5 );
                    vec3d pntB = all_surf_vec[j].CompPnt01( 0.5, 0.5 );

                    BndBox bboxA, bboxB;
                    all_surf_vec[i].GetBoundingBox( bboxA );
                    all_surf_vec[j].GetBoundingBox( bboxB );

                    // Reverse FEA part expansion
                    BndBox temp_bboxA = bboxA;
                    BndBox temp_bboxB = bboxB;
                    
                    temp_bboxA.Expand( -FEA_PART_EXPANSION_FACTOR );
                    temp_bboxB.Expand( -FEA_PART_EXPANSION_FACTOR );

                    if ( ( dist_pnt_2_plane( pntA, all_norm_vec[i], pntB ) <= FLT_EPSILON ) && Compare( temp_bboxA, temp_bboxB ) )
                    {
                        VspSurf new_surf = all_surf_vec[i];

                        vec3d maxA = bboxA.GetMax();
                        vec3d maxB = bboxB.GetMax();
                        vec3d minA = bboxA.GetMin();
                        vec3d minB = bboxB.GetMin();

                        if ( ( all_feaprt_ind_vec[i] != all_feaprt_ind_vec[j] ) && dist( maxA, maxB ) <= FLT_EPSILON && dist( minA, minB ) <= FLT_EPSILON )
                        {
                            // Coplanar surfaces are same size and location. One surface kept, other is deleted
                            //  Note: Priority for which FeaPart data is kept is given to earlier index in fea_part_vec

                            // Update Parent Surface for Fixed Points. This is only supported for FeaParts placed in the 
                            //  same location, not for overlaps due to symmetry.
                            for ( size_t k = 0; k < fea_part_vec.size(); k++ )
                            {
                                if ( fea_struct->FeaPartIsFixPoint( k ) )
                                {
                                    FeaFixPoint* fixpnt = dynamic_cast<FeaFixPoint*>( fea_part_vec[k] );
                                    assert( fixpnt );

                                    string oldID = fea_part_vec[all_feaprt_ind_vec[j]]->GetID();
                                    string newID = fea_part_vec[all_feaprt_ind_vec[i]]->GetID();

                                    if ( strcmp( oldID.c_str(), fixpnt->m_ParentFeaPartID.c_str() ) == 0 )
                                    {
                                        fixpnt->m_ParentFeaPartID = newID;
                                    }
                                }
                            }
                        }
                        else 
                        {
                            // Coplanar surfaces are symmetric. Delete one surface resize the other. 
                            // Note: Fixed points on the oversized surface will not be at the same UW coordinate

                            // Identify how much larger to make the new surface (Note: May result in undesired surface intersections)
                            BndBox new_bbox = bboxA;
                            new_bbox.Update( bboxB );
                            double scale_factor = 1 + ( new_bbox.GetLargestDist() - bboxA.GetLargestDist() ) / bboxA.GetLargestDist();

                            // Move center of the surface to the orgin prior to expanding
                            vec3d centerA = ( maxA + minA ) / 2;
                            vec3d new_center = new_bbox.GetCenter();

                            new_surf.Offset( -1 * centerA );
                            new_surf.Scale( scale_factor );
                            new_surf.Offset( new_center );
                        }

                        fea_part_vec[all_feaprt_ind_vec[i]]->DeleteFeaPartSurf( feaprt_surf_ind_vec[i] );

                        // Adjust surf indexes for deleted surface
                        for ( size_t n = 0; n < feaprt_surf_ind_vec.size(); n++ )
                        {
                            if ( all_feaprt_ind_vec[i] == all_feaprt_ind_vec[n] && ( feaprt_surf_ind_vec[i] < feaprt_surf_ind_vec[n] ) )
                            {
                                feaprt_surf_ind_vec[n] -= 1;
                            }
                        }

                        fea_part_vec[all_feaprt_ind_vec[j]]->DeleteFeaPartSurf( feaprt_surf_ind_vec[j] );

                        // Adjust surf indexes for deleted surface
                        for ( size_t n = 0; n < feaprt_surf_ind_vec.size(); n++ )
                        {
                            if ( all_feaprt_ind_vec[j] == all_feaprt_ind_vec[n] && ( feaprt_surf_ind_vec[j] < feaprt_surf_ind_vec[n] ) )
                            {
                                feaprt_surf_ind_vec[n] -= 1;
                            }
                        }

                        // Add merged surface
                        fea_part_vec[all_feaprt_ind_vec[i]]->AddFeaPartSurf( new_surf );
                        // Update surface index
                        feaprt_surf_ind_vec[i] = fea_part_vec[all_feaprt_ind_vec[i]]->GetFeaPartSurfVec().size() - 1;
                        all_surf_vec[i] = new_surf;
                        all_surf_vec[j] = new_surf;
                        all_norm_vec[j] = vec3d(); // Only clear normal vector of deleted surface

                        // Output warning if FeaParts are different due to data loss (symmetric parts retain all FeaPart data)
                        if ( all_feaprt_ind_vec[i] != all_feaprt_ind_vec[j] )
                        {
                            string output = "WARNING: Coplanar Surfaces Merged: " + fea_part_vec[all_feaprt_ind_vec[i]]->GetName() +
                                ", " + fea_part_vec[all_feaprt_ind_vec[j]]->GetName() + "\n";

                            addOutputText( output );
                        }
                    }
                }
            }
        }
    }
}

void FeaMeshMgrSingleton::AddStructureSurfParts()
{
    FeaStructure* fea_struct = StructureMgr.GetFeaStruct( m_FeaStructID );

    if ( fea_struct && GetMeshPtr() )
    {
        // Identify the max Surf ID of structure's parent surfaces (not necessarily equal to m_SurfVec.size() due to CleanMergedSurfs)
        int start_surf_id = 0;
        for ( size_t i = 0; i < m_SurfVec.size(); i++ )
        {
            start_surf_id = max( start_surf_id, m_SurfVec[i]->GetSurfID() );
        }
        start_surf_id += 1;

        vector < FeaPart* > fea_part_vec = fea_struct->GetFeaPartVec();

        //===== Add FeaParts ====//
        for ( unsigned int i = 0; i < GetMeshPtr()->m_NumFeaParts; i++ ) // Do Not Assume Skin is Index 0
        {
            if ( !fea_struct->FeaPartIsFixPoint( i ) &&
                 !fea_struct->FeaPartIsSkin( i ) &&
                 !fea_struct->FeaPartIsTrim( i ) )
            {
                vector< XferSurf > partxfersurfs;

                fea_part_vec[i]->FetchFeaXFerSurf( partxfersurfs, -9999 + ( i - 1 ) );

                // Load FeaPart XFerSurf to m_SurfVec
                LoadSurfs( partxfersurfs, GetMeshPtr()->m_LenScale, start_surf_id );
                start_surf_id += partxfersurfs.size();

                // Identify the FeaPart index and add to m_FeaPartSurfVec
                int begin = m_SurfVec.size() - partxfersurfs.size();
                int end = m_SurfVec.size();

                for ( int j = begin; j < end; j++ )
                {
                    m_SurfVec[j]->SetFeaPartIndex( i );
                }
            }
        }
    }
}

void FeaMeshMgrSingleton::AddStructureFixPoints()
{
    FeaStructure* fea_struct = StructureMgr.GetFeaStruct( m_FeaStructID );

    if ( fea_struct && GetMeshPtr() )
    {
        vector < FeaPart* > fea_part_vec = fea_struct->GetFeaPartVec();

        //===== Add FeaParts ====//
        for ( unsigned int i = 0; i < fea_struct->NumFeaParts(); i++ ) // Do Not Assume Skin is Index 0
        {
            if ( fea_struct->FeaPartIsFixPoint( i ) )
            {
                //===== Add FixedPoint Data ====//
                FeaFixPoint* fixpnt = dynamic_cast<FeaFixPoint*>( fea_part_vec[i] );
                assert( fixpnt );
                FixPoint fxpt;

                fxpt.m_Pnt = fixpnt->GetPntVec( GetMeshPtr()->m_LenScale );

                int npt = fxpt.m_Pnt.size();

                fxpt.m_FeaPartIndex = i;
                fxpt.m_PtMassFlag = fixpnt->m_FixPointMassFlag.Get();
                if ( fxpt.m_PtMassFlag )
                {
                    fxpt.m_PtMass = fixpnt->m_FixPointMass_FEM.Get();
                }
                else
                {
                    // The flag should ensure this value is ignored, but there is no harm in zeroing
                    // it out just in case.
                    fxpt.m_PtMass = 0.0;
                }

                // Initialize node vector to -1.  Set in TagFeaNodes
                fxpt.m_NodeIndex.resize( npt, -1 );
                fxpt.m_SpiderIndex.resize( npt );

                if ( fixpnt->m_FixedPointType() == vsp::FEA_FIX_PT_ON_BODY )
                {
                    fxpt.m_FeaParentPartIndex = fea_struct->GetFeaPartIndex( fixpnt->m_ParentFeaPartID );
                    fxpt.m_OnBody = true;
                    vec2d uw = fixpnt->GetUW();
                    fxpt.m_UW = uw;

                    for ( size_t j = 0; j < npt; j++ )
                    {
                        bool onborder = false;
                        int iborder = -1;

                        // Identify the surface index and coordinate points for the fixed point
                        vector < int > surf_index;
                        for ( size_t k = 0; k < m_SurfVec.size(); k++ )
                        {
                            if ( m_SurfVec[k]->GetFeaPartIndex() == fea_struct->GetFeaPartIndex( fixpnt->m_ParentFeaPartID ) &&
                                 m_SurfVec[k]->GetFeaPartSurfNum() == j )
                            {
                                // Surfaces here are patches.  We're detecting points that fall on two patches because the border
                                // values of the parameter are valid on both patches.  This is not true when you reach the max/min
                                // limit of a patch.  I.e. W=0.0 and W=1.0 are the same point, but both do not get added by this
                                // logic.
                                if ( m_SurfVec[k]->ValidUW( fxpt.m_UW, 0.0 ) )
                                {
                                    surf_index.push_back( k );

                                    int border = m_SurfVec[k]->UWPointOnBorder( fxpt.m_UW.x(), fxpt.m_UW.y(), 1e-6 );
                                    if ( border != SurfCore::NOBNDY )
                                    {
                                        onborder = true;
                                        iborder = border;
                                    }
                                }
                            }
                        }


                        // Identify points on border where uw does not match up.
                        if ( onborder && surf_index.size() == 1 )  // Point is on border, but no second surface identified.
                        {
                            for ( size_t k = 0; k < m_SurfVec.size(); k++ )  // Loop over surface patches again.
                            {
                                if ( m_SurfVec[k]->GetFeaPartIndex() == fea_struct->GetFeaPartIndex( fixpnt->m_ParentFeaPartID ) &&
                                     m_SurfVec[k]->GetFeaPartSurfNum() == j )  // Apply same condition as before.
                                {
                                    if ( surf_index[0] != k ) // Skip already identified patch.
                                    {
                                        if ( m_SurfVec[surf_index[0]]->BorderMatch( iborder, m_SurfVec[k] ) )
                                        {
                                            surf_index.push_back( k );
                                        }
                                    }
                                }
                            }
                        }

                        if ( surf_index.empty() )
                        {
                            surf_index.push_back( -1 );
                        }

                        fxpt.m_SurfInd.push_back( surf_index );

                        if ( surf_index.size() > 1 )
                        {
                            fxpt.m_BorderFlag.push_back( BORDER_FIX_POINT );
                        }
                        else
                        {
                            fxpt.m_BorderFlag.push_back( SURFACE_FIX_POINT ); // Possibly re-set in CheckFixPointIntersects()
                        }

                    }
                }
                else
                {
                    fxpt.m_OnBody = false;
                    fxpt.m_FeaParentPartIndex = -1;
                }

                GetMeshPtr()->m_FixPntVec.push_back( fxpt );
            }
        }
    }
}

void FeaMeshMgrSingleton::ForceSurfaceFixPoints( int surf_indx, vector < vec2d > &adduw )
{
    if ( !GetMeshPtr() )
    {
        return;
    }

    for ( size_t n = 0; n < GetMeshPtr()->m_NumFeaFixPoints; n++ )  // Loop over all fix points.
    {
        FixPoint fxpt = GetMeshPtr()->m_FixPntVec[ n ];       // This fix point

        if ( fxpt.m_OnBody )
        {
            for ( size_t j = 0; j < fxpt.m_SurfInd.size(); j++ )  // For all multiplicity of points (symmetry, arrays, etc)
            {
                if ( fxpt.m_SurfInd[j].size() == 1 )      // On one surface only (not a boundary or intersection)
                {
                    if ( fxpt.m_SurfInd[ j ][0] == surf_indx )  // It is _this_ surface
                    {
                        if ( fxpt.m_BorderFlag[ j ] == SURFACE_FIX_POINT )  // Should be redundant by now, but to be safe.
                        {
                            adduw.push_back( fxpt.m_UW );
                        }
                    }
                }
            }
        }
    }
}

void FeaMeshMgrSingleton::AddStructureTrimPlanes()
{
    FeaStructure* fea_struct = StructureMgr.GetFeaStruct( m_FeaStructID );

    if ( fea_struct && GetMeshPtr() )
    {
        vector < FeaPartTrim * > trims;
        fea_struct->FetchAllTrimPlanes( trims );


        int ntrimpart = trims.size();
        GetMeshPtr()->m_NumFeaTrimParts = ntrimpart;

        int ntrimgroup = 0;
        for ( int i = 0; i < ntrimpart; i++ )
        {
            FeaPartTrim * trim = trims[i];

            if ( trim )
            {
                ntrimgroup += trim->CountTrimPlanes();
            }
        }


        GetMeshPtr()->m_TrimVec.reserve( ntrimgroup );

        for ( int i = 0; i < ntrimpart; i++ )
        {
            FeaPartTrim * trim = trims[i];

            if ( trim )
            {
                vector < vector < vec3d > > pti;
                vector < vector < vec3d > > normi;
                vector < vector < VspSurf > > surfi;

                trim->FetchTrimPlanes( pti, normi, surfi, GetMeshPtr()->m_LenScale );

                int nadd = pti.size();

                for ( int j = 0; j < nadd; j++ )
                {
                    PartTrim t;
                    t.m_TrimPt = pti[j];
                    t.m_TrimNorm = normi[j];
                    t.m_TrimSurf = surfi[j];
                    t.m_TrimSymm = j;

                    GetMeshPtr()->m_TrimVec.push_back( t );
                }
            }
        }
    }
}

void FeaMeshMgrSingleton::BuildMeshOrientationLookup()
{
    if ( !GetMeshPtr() )
    {
        return;
    }

    GetMeshPtr()->m_PartSurfOrientation.resize( GetMeshPtr()->m_NumFeaParts );

    //==== FeaProperties ====//
    for ( unsigned int i = 0; i < GetMeshPtr()->m_NumFeaParts; i++ )
    {
        int surf_num = GetMeshPtr()->m_FeaPartNumSurfVec[i];
        GetMeshPtr()->m_PartSurfOrientation[i].resize( surf_num );
        for ( int isurf = 0; isurf < surf_num; isurf++ )
        {
            Surf *srf = GetFeaSurf( i, isurf );  // i is partID, isurf is surf_number
            if ( srf )
            {
                vec3d orient = srf->GetFeaElementOrientation();
                GetMeshPtr()->m_PartSurfOrientation[i][isurf] = orient;
            }
        }
    }
}

void FeaMeshMgrSingleton::RemoveTrimTris()
{
    if ( !GetMeshPtr() )
    {
        return;
    }

    double trimtol = 0.01 * GetGridDensityPtr()->m_MinLen;
    if ( GetMeshPtr()->m_TrimVec.size() > 0 ) // Skip if there are no trim groups.
    {
        for ( int s = 0; s < ( int ) m_SurfVec.size(); ++s ) // every surface
        {
            bool delSomeTris = false;

            list < Face * > faceList = m_SurfVec[ s ]->GetMesh()->GetFaceList();
            for ( list < Face * >::iterator t = faceList.begin(); t != faceList.end(); ++t ) // every triangle
            {
                vec3d cp = ( *t )->ComputeCenterPnt( m_SurfVec[ s ] );

                for ( int i = 0; i < GetMeshPtr()->m_TrimVec.size(); i++ )
                {
                    // This seems convoluted, but it needs to be cumulative.
                    if ( GetMeshPtr()->m_TrimVec[i].CullPtByTrimGroup( cp, m_SurfVec[s]->GetFeaSymmIndex(), trimtol ) )
                    {
                        ( *t )->deleteFlag = true;
                        delSomeTris = true;
                        break; // Once flagged for deletion, don't check further trim groups, go to next tri.
                    }
                }
            }

            if ( delSomeTris ) // Skip removal if no tris in this m_SurfVec were flagged for deletion.
            {
                m_SurfVec[ s ]->GetMesh()->RemoveInteriorFacesEdgesNodes();
            }
        }
    }
}

void FeaMeshMgrSingleton::CheckDuplicateSSIntersects()
{
    // Check for SubSurface intersection chains that overlap, which are identified by the same end points.
    //  This is done to ensure no duplicate beam elements are created along the intersection curve. At this
    //  time, priority is given to the earlier SubSurface (lower index).

    list< ISegChain* >::iterator c1;
    list< ISegChain* >::iterator c2;
    int c1_index = -1;

    for ( c1 = m_ISegChainList.begin(); c1 != m_ISegChainList.end(); ++c1 )
    {
        c1_index++;
        int c2_index = -1;

        if ( !( *c1 )->m_BorderFlag && ( *c1 )->m_SSIntersectIndex >= 0 )
        {
            for ( c2 = m_ISegChainList.begin(); c2 != m_ISegChainList.end(); ++c2 )
            {
                c2_index++;

                if ( !( *c2 )->m_BorderFlag && ( *c2 )->m_SSIntersectIndex >= 0 && c1_index != c2_index )
                {
                    if ( ( *c1 )->m_TessVec.size() == ( *c2 )->m_TessVec.size() && ( *c1 )->m_SSIntersectIndex != ( *c2 )->m_SSIntersectIndex )
                    {
                        bool match = false;

                        if ( dist( ( *c1 )->m_TessVec[0]->m_Pnt, ( *c2 )->m_TessVec[0]->m_Pnt ) <= FLT_EPSILON
                             && dist( ( *c1 )->m_TessVec.back()->m_Pnt, ( *c2 )->m_TessVec.back()->m_Pnt ) <= FLT_EPSILON )
                        {
                            match = true;
                        }
                        else if ( dist( ( *c1 )->m_TessVec[0]->m_Pnt, ( *c2 )->m_TessVec.back()->m_Pnt ) <= FLT_EPSILON
                                  && dist( ( *c1 )->m_TessVec.back()->m_Pnt, ( *c2 )->m_TessVec[0]->m_Pnt ) <= FLT_EPSILON )
                        {
                            match = true;
                        }

                        if ( match )
                        {
                            if ( ( *c1 )->m_SSIntersectIndex < ( *c2 )->m_SSIntersectIndex ) // Priority given to lower SubSurface index
                            {
                                ( *c2 )->m_BorderFlag = false;
                                ( *c2 )->m_SSIntersectIndex = -1;
                            }
                            else
                            {
                                ( *c1 )->m_BorderFlag = false;
                                ( *c1 )->m_SSIntersectIndex = -1;
                            }
                        }
                    }
                }
            }
        }
    }
}

void FeaMeshMgrSingleton::BuildFeaMesh()
{
    if ( !GetMeshPtr() )
    {
        return;
    }

    bool highorder = GetMeshPtr()->m_StructSettings.m_HighOrderElementFlag;
    double trimtol = 0.01 * GetGridDensityPtr()->m_MinLen;

    //==== Collect All Nodes and Tris ====//
    vector < vec2d > all_uw_vec;
    vector < int > uw_surf_ind_vec; // Vector of surface index for each UW point
    vector < vec3d > all_pnt_vec;
    vector < SimpFace > all_face_vec;
    vector < int > tri_surf_ind_vec; // Vector of surface index for each SimpTri

    // BuildFeaMesh() is called after ConvertToQuads().  So, we need to skip every other point for a tri
    // mesh, but use every point for a quad mesh.
    int tessIncrement = 2;
    if ( GetMeshPtr()->m_StructSettings.m_ConvertToQuadsFlag )
    {
        tessIncrement = 1;
    }

    // Build FeaBeam Intersections
    list< ISegChain* >::iterator c;

    for ( c = m_ISegChainList.begin(); c != m_ISegChainList.end(); ++c )
    {
        if ( !( *c )->m_BorderFlag ) // Only include intersection curves
        {
            // Check at least one surface intersection cap flag is true
            bool BeamElementsA = GetMeshPtr()->m_FeaPartCreateBeamElementsVec[( *c )->m_SurfA->GetFeaPartIndex()];
            bool BeamElementsB = GetMeshPtr()->m_FeaPartCreateBeamElementsVec[( *c )->m_SurfB->GetFeaPartIndex()];

            vector < vec3d > ipntVec, inormVec;
            vector < vec2d > iuwVec;
            vector < int > ssindexVec;
            Surf* NormSurf = NULL;
            int FeaPartIndex = -1;

            // Check if one surface is a skin and one is an FeaPart (m_CompID = -9999)
            if ( ( BeamElementsA || BeamElementsB ) &&
                ( ( ( *c )->m_SurfA->GetCompID() < 0 && ( *c )->m_SurfB->GetCompID() >= 0 ) || ( ( *c )->m_SurfB->GetCompID() < 0 && ( *c )->m_SurfA->GetCompID() >= 0 ) ) )
            {
                vec3d center;

                if ( ( *c )->m_SurfA->GetCompID() < 0 && BeamElementsA )
                {
                    FeaPartIndex = ( *c )->m_SurfA->GetFeaPartIndex();
                    center = ( *c )->m_SurfA->GetBBox().GetCenter();
                }
                else if ( ( *c )->m_SurfB->GetCompID() < 0 && BeamElementsB )
                {
                    FeaPartIndex = ( *c )->m_SurfB->GetFeaPartIndex();
                    center = ( *c )->m_SurfB->GetBBox().GetCenter();
                }

                // Identify the normal surface as the skin surface
                if ( ( *c )->m_SurfA->GetCompID() >= 0 )
                {
                    NormSurf = ( *c )->m_SurfA;
                }
                else if ( ( *c )->m_SurfB->GetCompID() >= 0 )
                {
                    NormSurf = ( *c )->m_SurfB;
                }

                // Get points and compute normals
                for ( int j = 0; j < (int)( *c )->m_TessVec.size(); j += tessIncrement )  // Increment by tessIncrement
                {
                    Puw* Puw = ( *c )->m_TessVec[j]->GetPuw( NormSurf );
                    iuwVec.push_back( vec2d( Puw->m_UW[0], Puw->m_UW[1] ) );
                    vec3d norm = NormSurf->GetSurfCore()->CompNorm( Puw->m_UW[0], Puw->m_UW[1] );
                    norm.normalize();

                    if ( NormSurf->GetFlipFlag() )
                    {
                        norm = -1 * norm;
                    }

                    inormVec.push_back( norm );
                    ipntVec.push_back( ( *c )->m_TessVec[j]->m_Pnt );
                    ssindexVec.push_back( -1 ); // Indicates not a subsurface intersection
                }

                // Check if the direction of ipntVec. Reverse point and norm vec order if negative
                double theta = signed_angle( ipntVec[0] - center, ipntVec.back() - center, center );
                if ( theta < 0 )
                {
                    reverse( inormVec.begin(), inormVec.end() );
                    reverse( ipntVec.begin(), ipntVec.end() );
                }
            }
            // Check for an intersection with the same component ID -> indicates a subsurface intersection
            else if ( ( *c )->m_SurfA->GetCompID() == ( *c )->m_SurfB->GetCompID() && ( *c )->m_SurfA->GetCompID() >= 0 )
            {
                if ( ( *c )->m_SSIntersectIndex >= 0 )
                {
                    FeaPartIndex = ( *c )->m_SurfA->GetFeaPartIndex();
                    NormSurf = ( *c )->m_SurfA;

                    // Get points and compute normals
                    for ( int j = 0; j < (int)( *c )->m_TessVec.size(); j += tessIncrement )  // Increment by tessIncrement
                    {
                        Puw* Puw = ( *c )->m_TessVec[j]->GetPuw( NormSurf );
                        iuwVec.push_back( vec2d( Puw->m_UW[0], Puw->m_UW[1] ) );
                        vec3d norm = NormSurf->GetSurfCore()->CompNorm( Puw->m_UW[0], Puw->m_UW[1] );
                        norm.normalize();

                        if ( NormSurf->GetFlipFlag() )
                        {
                            norm = -1 * norm;
                        }

                        inormVec.push_back( norm );
                        ipntVec.push_back( ( *c )->m_TessVec[j]->m_Pnt );
                        ssindexVec.push_back( ( *c )->m_SSIntersectIndex );
                    }
                }
            }

            int ichain = GetMeshPtr()->m_FeaPartNumChainsVec[ FeaPartIndex ];
            GetMeshPtr()->m_FeaPartNumChainsVec[ FeaPartIndex ]++;

            int normsurfindx = vector_find_val( m_SurfVec, NormSurf );
            // Define FeaBeam elements
            for ( int j = 1; j < (int)ipntVec.size(); j++ )
            {
                vec3d start_pnt = ipntVec[j - 1];
                vec3d end_pnt = ipntVec[j];

                // Check for collapsed beam elements (caused by bug in Intersect where invalid intersection points are added to m_BinMap)
                if ( dist( start_pnt, end_pnt ) < FLT_EPSILON )
                {
                    printf( "Warning: Collapsed Beam Element Skipped\n" );
                    break;
                }

                if ( GetMeshPtr()->m_TrimVec.size() > 0 ) // Skip if there are no trim groups.
                {
                    bool skipElement = false;
                    vec3d mid_pnt = 0.5 * ( start_pnt + end_pnt );

                    for ( int i = 0; i < GetMeshPtr()->m_TrimVec.size(); i++ )
                    {
                        // This seems convoluted, but it needs to be cumulative.
                        if ( GetMeshPtr()->m_TrimVec[i].CullPtByTrimGroup( mid_pnt, NormSurf->GetFeaSymmIndex(), trimtol ) )
                        {
                            skipElement = true;
                            break; // Once flagged for deletion, don't check further trim groups, go to next beam segment.
                        }
                    }

                    if ( skipElement )
                    {
                        break;
                    }
                }

                //// Use node point if close to beam endpoints (avoids tolerance errors in BuildIndMap and FindPntInd)
                //for ( size_t k = 0; k < node_vec.size(); k++ )
                //{
                //    if ( dist( node_vec[k], start_pnt ) <= FLT_EPSILON )
                //    {
                //        start_pnt = node_vec[k];
                //    }
                //    else if ( dist( node_vec[k], end_pnt ) <= FLT_EPSILON )
                //    {
                //        end_pnt = node_vec[k];
                //    }
                //}

                all_pnt_vec.push_back( start_pnt );
                all_pnt_vec.push_back( end_pnt );
                uw_surf_ind_vec.push_back( NormSurf->GetSurfID() );
                uw_surf_ind_vec.push_back( NormSurf->GetSurfID() );
                all_uw_vec.push_back( iuwVec[j - 1] );
                all_uw_vec.push_back( iuwVec[j] );

                FeaBeam* beam = new FeaBeam;
                beam->Create( start_pnt, end_pnt, inormVec[ j - 1 ],  inormVec[ j ] );
                beam->SetFeaPartIndex( FeaPartIndex );
                beam->SetFeaSSIndex( ssindexVec[j] );
                beam->SetFeaPartSurfNum( m_SurfVec[normsurfindx]->GetFeaPartSurfNum() );
                beam->SetChainIndex( ichain );
                GetMeshPtr()->m_FeaElementVec.push_back( beam );
                GetMeshPtr()->m_NumBeams++;
            }
        }
    }

    for ( int s = 0; s < (int)m_SurfVec.size(); s++ )
    {
        vector < vec2d > uw_vec_curr = m_SurfVec[s]->GetMesh()->GetSimpUWPntVec();
        vector < vec3d > pnt_vec_curr = m_SurfVec[s]->GetMesh()->GetSimpPntVec();
        vector < SimpFace > face_vec_curr = m_SurfVec[ s ]->GetMesh()->GetSimpFaceVec();

        for ( size_t i = 0; i < face_vec_curr.size(); i++ )
        {
            // Offset SimpTri indexes 
            face_vec_curr[i].ind0 += all_pnt_vec.size();
            face_vec_curr[i].ind1 += all_pnt_vec.size();
            face_vec_curr[i].ind2 += all_pnt_vec.size();
            if( face_vec_curr[i].m_isQuad )
            {
                face_vec_curr[i].ind3 += all_pnt_vec.size();
            }

            all_face_vec.push_back( face_vec_curr[i] );
            tri_surf_ind_vec.push_back( s );
        }

        for ( size_t i = 0; i < pnt_vec_curr.size(); i++ )
        {
            all_pnt_vec.push_back( pnt_vec_curr[i] );
            all_uw_vec.push_back( uw_vec_curr[i] );
            uw_surf_ind_vec.push_back( s );
        }
    }

    if ( all_pnt_vec.size() == 0 )
    {
        m_FeaMeshInProgress = false;
        addOutputText( "Error: No Nodes in Mesh\n" );
        return;
    }

    //==== Build Map ====//
    PntNodeCloud pnCloud;
    pnCloud.AddPntNodes( all_pnt_vec );

    //==== Compute Tol ====//
    BndBox bb = m_Vehicle->GetBndBox();
    double tol = bb.GetLargestDist() * 1.0e-10;

    //==== Use NanoFlann to Find Close Points and Group ====//
    IndexPntNodes( pnCloud, tol );

    //==== Load Used Nodes ====//
    vector < vec3d > node_vec;
    vector < vec2d > new_uw_vec;
    vector < int > index_vec;
    vector < int > new_uw_surf_ind_vec;
    node_vec.reserve( pnCloud.m_NumUsedPts );
    new_uw_vec.reserve( pnCloud.m_NumUsedPts );
    index_vec.reserve( pnCloud.m_NumUsedPts );
    new_uw_surf_ind_vec.reserve( pnCloud.m_NumUsedPts );

    for ( size_t i = 0; i < (int)all_pnt_vec.size(); i++ )
    {
        if ( pnCloud.UsedNode( i ) )
        {
            node_vec.push_back( all_pnt_vec[i] );
            new_uw_vec.push_back( all_uw_vec[i] );
            new_uw_surf_ind_vec.push_back( uw_surf_ind_vec[i] );
        }
    }

    //==== Set Adjusted Node IDs ====//
    for ( size_t i = 0; i < (int)all_pnt_vec.size(); i++ )
    {
        index_vec.push_back( pnCloud.GetNodeUsedIndex( i ) );
    }

    for ( size_t j = 0; j < all_face_vec.size(); j++ )
    {
        all_face_vec[j].ind0 = index_vec[ all_face_vec[j].ind0 ];
        all_face_vec[j].ind1 = index_vec[ all_face_vec[j].ind1 ];
        all_face_vec[j].ind2 = index_vec[ all_face_vec[j].ind2 ];

        if ( all_face_vec[j].m_isQuad )
        {
            all_face_vec[j].ind3 = index_vec[ all_face_vec[j].ind3 ];
        }
    }

    GetMeshPtr()->m_FeaElementVec.reserve( GetMeshPtr()->m_FeaElementVec.size() + all_face_vec.size() );
    // Build FeaTris
    for ( int i = 0; i < (int)all_face_vec.size(); i++ )
    {
        vec3d pnt0 = node_vec[all_face_vec[i].ind0];
        vec3d pnt1 = node_vec[all_face_vec[i].ind1];
        vec3d pnt2 = node_vec[all_face_vec[i].ind2];

        vec3d avg_pnt;
        if ( all_face_vec[i].m_isQuad )
        {
            vec3d pnt3 = node_vec[all_face_vec[i].ind3];
            avg_pnt = ( pnt0 + pnt1 + pnt2 + pnt3 ) / 4.0;
        }
        else
        {
            avg_pnt = ( pnt0 + pnt1 + pnt2 ) / 3.0;
        }


        // Determine initial guess for uw near SimpTri center (uw points defining SimpTri can be from different surfaces after adjustments) 
        vec2d uw_guess;

        if ( tri_surf_ind_vec[i] == new_uw_surf_ind_vec[all_face_vec[i].ind0] )
        {
            uw_guess = new_uw_vec[all_face_vec[i].ind0];
        }
        else if ( tri_surf_ind_vec[i] == new_uw_surf_ind_vec[all_face_vec[i].ind1] )
        {
            uw_guess = new_uw_vec[all_face_vec[i].ind1];
        }
        else if ( tri_surf_ind_vec[i] == new_uw_surf_ind_vec[all_face_vec[i].ind2] )
        {
            uw_guess = new_uw_vec[all_face_vec[i].ind2];
        }
        else if ( all_face_vec[i].m_isQuad && tri_surf_ind_vec[i] == new_uw_surf_ind_vec[all_face_vec[i].ind3] )
        {
            uw_guess = new_uw_vec[all_face_vec[i].ind3];
        }
        else
        {
            uw_guess.set_x( m_SurfVec[tri_surf_ind_vec[i]]->GetSurfCore()->GetMidU() );
            uw_guess.set_y( m_SurfVec[tri_surf_ind_vec[i]]->GetSurfCore()->GetMidW() );
        }

        FeaElement* elem = NULL;
        if( all_face_vec[i].m_isQuad )
        {
            elem = new FeaQuad;
            ((FeaQuad*)elem)->Create( node_vec[all_face_vec[i].ind0], node_vec[all_face_vec[i].ind1], node_vec[all_face_vec[i].ind2], node_vec[all_face_vec[i].ind3], highorder );
            GetMeshPtr()->m_NumQuads++;
        }
        else
        {
            elem = new FeaTri;
            ((FeaTri*)elem)->Create( node_vec[all_face_vec[i].ind0], node_vec[all_face_vec[i].ind1], node_vec[all_face_vec[i].ind2], highorder );
            GetMeshPtr()->m_NumTris++;
        }
        elem->SetFeaPartIndex( m_SurfVec[tri_surf_ind_vec[i]]->GetFeaPartIndex() );
        elem->SetFeaPartSurfNum( m_SurfVec[tri_surf_ind_vec[i]]->GetFeaPartSurfNum() );
        elem->SetReason( all_face_vec[i].m_reason );

        vec2d closest_uw = m_SurfVec[tri_surf_ind_vec[i]]->ClosestUW( avg_pnt, uw_guess[0], uw_guess[1] );

        // Determine tangent u-direction for orientation vector at tri midpoint
        vec3d orient_vec = m_SurfVec[tri_surf_ind_vec[i]]->GetFeaElementOrientation( closest_uw[0], closest_uw[1] );

        // Check for subsurface:
        if ( all_face_vec[i].m_Tags.size() == 2 )
        {
            // First index of m_Tags is the parent surface. Second index is subsurface index which begins 
            //  from the last FeaPart surface index (FeaFixPoints are not tagged; they are not surfaces)
            // Tag numbering offset is established in CfdMeshMgrSingleton::SetSimpSubSurfTags() which is called from
            // CfdMeshMgrSingleton::SubTagTris().  The offset is based on the number of surfaces encountered, typically
            // one per FeaPart.  However, FixPoints and Trim Parts don't have surfaces associated with them.
            int ssindex = all_face_vec[i].m_Tags[1] - ( GetMeshPtr()->m_NumFeaParts - GetMeshPtr()->m_NumFeaFixPoints - GetMeshPtr()->m_NumFeaTrimParts ) - 1;

            if ( ssindex <= m_SimpleSubSurfaceVec.size() )
            {
                elem->SetFeaSSIndex( ssindex );

                int type = m_SimpleSubSurfaceVec[ssindex].GetFeaOrientationType();
                vector < vec3d > ovec = m_SimpleSubSurfaceVec[ssindex].GetFeaOrientationVec();
                int surf_num = m_SurfVec[tri_surf_ind_vec[i]]->GetFeaPartSurfNum();
                vec3d defaultorientation = ovec[ surf_num ];
                orient_vec = m_SurfVec[tri_surf_ind_vec[i]]->GetFeaElementOrientation( closest_uw[0], closest_uw[1], type, defaultorientation );
            }
            if ( false ) // else
            {
                printf( "Error:  Failed to match subsurface for element properties.\n" );
            }
        }
        else if ( all_face_vec[i].m_Tags.size() > 2 )
        {
            // Give priority to first tagged subsurface in the event of overlap.  Potential to do something more sophisticated later.
            int ssindex = all_face_vec[i].m_Tags[1] - ( GetMeshPtr()->m_NumFeaParts - GetMeshPtr()->m_NumFeaFixPoints - GetMeshPtr()->m_NumFeaTrimParts ) - 1;

            if ( ssindex <= m_SimpleSubSurfaceVec.size() )
            {
                elem->SetFeaSSIndex( ssindex );

                int type = m_SimpleSubSurfaceVec[ssindex].GetFeaOrientationType();
                vector < vec3d > ovec = m_SimpleSubSurfaceVec[ssindex].GetFeaOrientationVec();
                int surf_num = m_SurfVec[tri_surf_ind_vec[i]]->GetFeaPartSurfNum();
                vec3d defaultorientation = ovec[ surf_num ];
                orient_vec = m_SurfVec[tri_surf_ind_vec[i]]->GetFeaElementOrientation( closest_uw[0], closest_uw[1], type, defaultorientation );
            }
            if ( false ) // else
            {
                printf( "Error:  Failed to match subsurface for element properties.\n" );
            }
        }

        // Project orientation vector into plane of element.  Will be trivial for u-direction orientation, but
        // should allow NASTRAN and CalculiX equivalence for other cases.
        vec3d norm = m_SurfVec[tri_surf_ind_vec[i]]->GetSurfCore()->CompNorm( closest_uw[0], closest_uw[1] );
        orient_vec = proj_vec_to_plane( orient_vec, norm );

        orient_vec.normalize();

        elem->m_Orientation = orient_vec;

        GetMeshPtr()->m_FeaElementVec.push_back( elem );
        GetMeshPtr()->m_NumEls++;
    }

    // Add all off-body fixed points to m_FeaOffBodyFixPointNodeVec
    for ( size_t n = 0; n < GetMeshPtr()->m_NumFeaFixPoints; n++ )
    {
        FixPoint fxpt = GetMeshPtr()->m_FixPntVec[n];

        if ( !fxpt.m_OnBody )
        {
            for ( size_t j = 0; j < fxpt.m_Pnt.size(); j++ )
            {
                FeaNode *nod = new FeaNode( fxpt.m_Pnt[j] );

                // Should be redundant.
                //n->m_FixedPointFlag = true;
                //n->AddTag( fxpt.m_FeaPartIndex );
                GetMeshPtr()->m_FeaOffBodyFixPointNodeVec.push_back( nod );
            }
        }
    }
}

void FeaMeshMgrSingleton::SetFixPointSurfaceNodes()
{
    if ( !GetMeshPtr() )
    {
        return;
    }

    for ( size_t n = 0; n < GetMeshPtr()->m_NumFeaFixPoints; n++ )
    {
        FixPoint fxpt = GetMeshPtr()->m_FixPntVec[n];

        if ( fxpt.m_OnBody )
        {
            for ( size_t j = 0; j < fxpt.m_SurfInd.size(); j++ )
            {
                if ( fxpt.m_BorderFlag[j] == SURFACE_FIX_POINT && fxpt.m_SurfInd[j].size() == 1 )
                {
                    if ( fxpt.m_SurfInd[j][0] >= 0 )
                    {
                        string fix_point_name = GetMeshPtr()->m_FeaPartNameVec[ fxpt.m_FeaPartIndex ];

                        if ( m_SurfVec[ fxpt.m_SurfInd[j][0] ]->GetMesh()->SetFixPoint( fxpt.m_Pnt[j], fxpt.m_UW ) )
                        {
                            // No message on success.
                        }
                        else
                        {
                            string message = "\tNo node found for " + fix_point_name + ". Adjust GridDensity.\n";
                            addOutputText( message );
                        }
                    }
                }
            }
        }
    }
}

// Called first from SurfaceIntersectionMgr::Intersect()
void FeaMeshMgrSingleton::SetFixPointBorderNodes()
{
    if ( !GetMeshPtr() )
    {
        return;
    }

    addOutputText( "SetFixPointBorderNodes\n" );

    for ( size_t n = 0; n < GetMeshPtr()->m_NumFeaFixPoints; n++ )
    {
        FixPoint fxpt = GetMeshPtr()->m_FixPntVec[n];

        if ( fxpt.m_OnBody )
        {
            // Identify and set FeaFixPoints on border curves
            for ( size_t j = 0; j < fxpt.m_SurfInd.size(); j++ )
            {
                // Only check for FeaFixPoints on two surfaces. Nodes are automatically set for more than two surface intersections
                if ( fxpt.m_BorderFlag[j] == BORDER_FIX_POINT && fxpt.m_SurfInd[j].size() == 2 )
                {
                    bool split = false;
                    list< ISegChain* >::iterator c;
                    for ( c = m_ISegChainList.begin(); c != m_ISegChainList.end(); ++c )
                    {
                        if (( ( *c )->m_SurfA == m_SurfVec[fxpt.m_SurfInd[j][1]] && ( *c )->m_SurfB == m_SurfVec[fxpt.m_SurfInd[j][0]] ) ||
                            ( ( *c )->m_SurfA == m_SurfVec[fxpt.m_SurfInd[j][0]] && ( *c )->m_SurfB == m_SurfVec[fxpt.m_SurfInd[j][1]] ) )
                        {
                            vec2d closest_uwA, closest_uwB;

                            if ( ( *c )->m_SurfA->ValidUW( fxpt.m_UW ) )
                            {
                                closest_uwA = ( *c )->m_SurfA->ClosestUW( fxpt.m_Pnt[j], fxpt.m_UW[0], fxpt.m_UW[1] );
                            }
                            else
                            {
                                closest_uwA = ( *c )->m_SurfA->ClosestUW( fxpt.m_Pnt[j] );
                            }

                            if ( ( *c )->m_SurfB->ValidUW( fxpt.m_UW ) )
                            {
                                closest_uwB = ( *c )->m_SurfB->ClosestUW( fxpt.m_Pnt[j], fxpt.m_UW[0], fxpt.m_UW[1] );
                            }
                            else
                            {
                                closest_uwB = ( *c )->m_SurfB->ClosestUW( fxpt.m_Pnt[j] );
                            }

                            Puw* p0 = NULL;
                            Puw* p1 = NULL;

                            if ( ( *c )->m_SurfA->ValidUW( closest_uwA ) )
                            {
                                p0 = new Puw( ( *c )->m_SurfA, closest_uwA );
                            }
                            if ( ( *c )->m_SurfB->ValidUW( closest_uwB ) )
                            {
                                p1 = new Puw( ( *c )->m_SurfB, closest_uwB );
                            }

                            bool success = false;

                            if ( p0 )
                            {
                                success = ( *c )->AddBorderSplit( p0 );
                            }
                            if ( p1 && !success )
                            {
                                success = ( *c )->AddBorderSplit( p1 );
                            }

                            if ( success )
                            {
                                string fix_point_name = GetMeshPtr()->m_FeaPartNameVec[fxpt.m_FeaPartIndex];
                                string message = "\tBorder Intersect Point Set for " + fix_point_name + "\n";
                                addOutputText( message );
                                split = true;
                            }
                            else // Free memory
                            {
                                if ( p0 )
                                {
                                    delete p0;
                                }
                                if ( p1 )
                                {
                                    delete p1;
                                }
                            }
                        }

                        if ( split )
                        {
                            break;
                        }
                    }
                }
            }
        }
    }
}

// Called immediately next from SurfaceIntersectionMgr::Intersect()
void FeaMeshMgrSingleton::CheckFixPointIntersects()
{
    if ( !GetMeshPtr() )
    {
        return;
    }

    addOutputText( "CheckFixPointIntersects\n" );
    // Identify and set FeaFixPoints on intersection curves

    for ( size_t n = 0; n < GetMeshPtr()->m_NumFeaFixPoints; n++ )
    {
        FixPoint fxpt = GetMeshPtr()->m_FixPntVec[n];

        if ( fxpt.m_OnBody )
        {
            for ( size_t j = 0; j < fxpt.m_SurfInd.size(); j++ )
            {
                bool split = false;
                list< ISegChain* >::iterator c;
                for ( c = m_ISegChainList.begin(); c != m_ISegChainList.end(); ++c )
                {
                    Puw* p0 = NULL;
                    Puw* p1 = NULL;
                    bool success = false;
                    double tol = 1e-3;

                    if ( !( *c )->m_BorderFlag && fxpt.m_SurfInd[j].size() == 1 )
                    {
                        if ( fxpt.m_SurfInd[j][0] >= 0 )
                        {
                            if ( ( *c )->m_SurfA != ( *c )->m_SurfB ) // Check for intersection between two FeaPart surfaces
                            {
                                if ( ( *c )->m_SurfA == m_SurfVec[fxpt.m_SurfInd[j][0]] )
                                {
                                    vec2d closest_uw = ( *c )->m_SurfB->ClosestUW( fxpt.m_Pnt[j] );
                                    vec3d closest_pnt = ( *c )->m_SurfB->CompPnt( closest_uw[0], closest_uw[1] );

                                    if ( ( *c )->m_SurfA->GetCompID() < 0 ) // Looser tolerance for FeaParts
                                    {
                                        tol = 1e-2;
                                    }

                                    // Compare FeaFixPoint to closest point on other surface
                                    if ( dist( closest_pnt, fxpt.m_Pnt[j] ) <= tol )
                                    {
                                        vec2d closest_uwA = ( *c )->m_SurfA->ClosestUW( fxpt.m_Pnt[j], fxpt.m_UW[0], fxpt.m_UW[1] );
                                        vec2d closest_uwB = closest_uw;

                                        if ( ( *c )->m_SurfA->ValidUW( closest_uwA ) )
                                        {
                                            p0 = new Puw( ( *c )->m_SurfA, closest_uwA );
                                        }
                                        if ( ( *c )->m_SurfB->ValidUW( closest_uwB ) )
                                        {
                                            p1 = new Puw( ( *c )->m_SurfB, closest_uwB );
                                        }

                                        if ( p0 )
                                        {
                                            success = ( *c )->AddBorderSplit( p0 );
                                        }
                                        if ( p1 && !success )
                                        {
                                            success = ( *c )->AddBorderSplit( p1 );
                                        }
                                    }
                                }
                                else if ( ( *c )->m_SurfB == m_SurfVec[fxpt.m_SurfInd[j][0]] )
                                {
                                    vec2d closest_uw = ( *c )->m_SurfA->ClosestUW( fxpt.m_Pnt[j] );
                                    vec3d closest_pnt = ( *c )->m_SurfA->CompPnt( closest_uw[0], closest_uw[1] );

                                    if ( ( *c )->m_SurfB->GetCompID() < 0 ) // Looser tolerance for FeaParts
                                    {
                                        tol = 1e-2;
                                    }

                                    // Compare FeaFixPoint to closest point on other surface
                                    if ( dist( closest_pnt, fxpt.m_Pnt[j] ) <= tol )
                                    {
                                        vec2d closest_uwA = closest_uw;
                                        vec2d closest_uwB = ( *c )->m_SurfB->ClosestUW( fxpt.m_Pnt[j], fxpt.m_UW[0], fxpt.m_UW[1] );

                                        if ( ( *c )->m_SurfA->ValidUW( closest_uwA ) )
                                        {
                                            p0 = new Puw( ( *c )->m_SurfA, closest_uwA );
                                        }
                                        if ( ( *c )->m_SurfB->ValidUW( closest_uwB ) )
                                        {
                                            p1 = new Puw( ( *c )->m_SurfB, closest_uwB );
                                        }

                                        if ( p0 )
                                        {
                                            success = ( *c )->AddBorderSplit( p0 );
                                        }
                                        if ( p1 && !success )
                                        {
                                            success = ( *c )->AddBorderSplit( p1 );
                                        }
                                    }
                                }
                            }
                            else if ( ( *c )->m_SurfA == ( *c )->m_SurfB && ( *c )->m_SurfA->GetSurfID() == fxpt.m_SurfInd[j][0] ) // Indicates SubSurface Edge
                            {
                                double closest_dist = FLT_MAX;

                                for ( size_t m = 0; m < ( *c )->m_ISegDeque.size(); m++ )
                                {
                                    vec3d ipnt0 = ( *c )->m_ISegDeque[m]->m_IPnt[0]->m_Pnt;
                                    vec3d ipnt1 = ( *c )->m_ISegDeque[m]->m_IPnt[1]->m_Pnt;

                                    // Find perpendicular distance from FeaFixPoint to ISeg
                                    double distance = ( cross(( fxpt.m_Pnt[j] - ipnt0 ), ( fxpt.m_Pnt[j] - ipnt1 ) ).mag() ) / ( ( ipnt1 - ipnt0 ).mag() );

                                    if ( distance <= closest_dist )
                                    {
                                        closest_dist = distance;
                                    }
                                }

                                if ( closest_dist < tol )
                                {
                                    vec2d closest_uwA = ( *c )->m_SurfA->ClosestUW( fxpt.m_Pnt[j], fxpt.m_UW[0], fxpt.m_UW[1] );
                                    vec2d closest_uwB = ( *c )->m_SurfB->ClosestUW( fxpt.m_Pnt[j], fxpt.m_UW[0], fxpt.m_UW[1] );

                                    if ( ( *c )->m_SurfA->ValidUW( closest_uwA ) )
                                    {
                                        p0 = new Puw( ( *c )->m_SurfA, closest_uwA );
                                    }
                                    if ( ( *c )->m_SurfB->ValidUW( closest_uwB ) )
                                    {
                                        p1 = new Puw( ( *c )->m_SurfB, closest_uwB );
                                    }

                                    if ( p0 )
                                    {
                                        success = ( *c )->AddBorderSplit( p0 );
                                    }
                                    if ( p1 && !success )
                                    {
                                        success = ( *c )->AddBorderSplit( p1 );
                                    }
                                }
                            }

                            if ( success )
                            {
                                GetMeshPtr()->m_FixPntVec[n].m_BorderFlag[j] = INTERSECT_FIX_POINT;

                                string fix_point_name = GetMeshPtr()->m_FeaPartNameVec[fxpt.m_FeaPartIndex];
                                string message = "\tIntersection Found for " + fix_point_name + "\n";
                                addOutputText( message );
                                split = true;
                            }
                            else // Free memory
                            {
                                if ( p0 )
                                {
                                    delete p0;
                                }
                                if ( p1 )
                                {
                                    delete p1;
                                }
                            }
                        }

                        if ( split )
                        {
                            break;
                        }
                    }
                }
            }
        }
    }
}

void FeaMeshMgrSingleton::CheckSubSurfBorderIntersect()
{
    // Collect all split points
    vector < vec3d > all_split_pnt_vec;
    vector < vec2d > all_split_uw_vec;
    list< ISegChain* >::iterator c;

    for ( c = m_ISegChainList.begin(); c != m_ISegChainList.end(); ++c )
    {
        for ( size_t i = 0; i < ( *c )->m_SplitVec.size(); i++ )
        {
            all_split_pnt_vec.push_back( ( *c )->m_SplitVec[i]->m_Pnt );
            all_split_uw_vec.push_back( ( *c )->m_SplitVec[i]->m_UW );
        }
    }

    for ( c = m_ISegChainList.begin(); c != m_ISegChainList.end(); ++c )
    {
        if ( ( *c )->m_BorderFlag && ( *c )->m_SurfA->GetCompID() == ( *c )->m_SurfB->GetCompID() && ( *c )->m_SSIntersectIndex < 0 )
        {
            // Match for split points for current ISegChain
            vector < vec3d > split_pnt_vec;
            vector < vec2d > split_uw_vec;

            for ( size_t i = 0; i < all_split_pnt_vec.size(); i++ )
            {
                if ( ( dist( ( *c )->m_ISegDeque[0]->m_IPnt[0]->m_Pnt, all_split_pnt_vec[i] ) <= FLT_EPSILON ) ||
                    ( dist( ( *c )->m_ISegDeque.back()->m_IPnt[1]->m_Pnt, all_split_pnt_vec[i] ) <= FLT_EPSILON ) )
                {
                    split_pnt_vec.push_back( all_split_pnt_vec[i] );
                    split_uw_vec.push_back( all_split_uw_vec[i] );
                }
            }

            vector < Surf* > surf_vec;
            surf_vec.resize( 2 );
            surf_vec[0] = ( *c )->m_SurfA;
            surf_vec[1] = ( *c )->m_SurfB;

            // Get the total min and max U/W for Surf A and B
            double tot_max_u = max( ( *c )->m_SurfA->GetSurfCore()->GetMaxU(), ( *c )->m_SurfB->GetSurfCore()->GetMaxU() );
            double tot_max_w = max( ( *c )->m_SurfA->GetSurfCore()->GetMaxW(), ( *c )->m_SurfB->GetSurfCore()->GetMaxW() );
            double tot_min_u = min( ( *c )->m_SurfA->GetSurfCore()->GetMinU(), ( *c )->m_SurfB->GetSurfCore()->GetMinU() );
            double tot_min_w = min( ( *c )->m_SurfA->GetSurfCore()->GetMinW(), ( *c )->m_SurfB->GetSurfCore()->GetMinW() );

            // Check for closedU and closedW surface
            bool closedW = false;
            bool closedU = false;
            
            vec3d min_min_A = ( *c )->m_SurfA->CompPnt( ( *c )->m_SurfA->GetSurfCore()->GetMinU(), ( *c )->m_SurfA->GetSurfCore()->GetMinW() );
            vec3d min_min_B = ( *c )->m_SurfB->CompPnt( ( *c )->m_SurfB->GetSurfCore()->GetMinU(), ( *c )->m_SurfB->GetSurfCore()->GetMinW() );
            vec3d min_max_A = ( *c )->m_SurfA->CompPnt( ( *c )->m_SurfA->GetSurfCore()->GetMinU(), ( *c )->m_SurfA->GetSurfCore()->GetMaxW() );
            vec3d min_max_B = ( *c )->m_SurfB->CompPnt( ( *c )->m_SurfB->GetSurfCore()->GetMinU(), ( *c )->m_SurfB->GetSurfCore()->GetMaxW() );
            vec3d max_min_A = ( *c )->m_SurfA->CompPnt( ( *c )->m_SurfA->GetSurfCore()->GetMaxU(), ( *c )->m_SurfA->GetSurfCore()->GetMinW() );
            vec3d max_min_B = ( *c )->m_SurfB->CompPnt( ( *c )->m_SurfB->GetSurfCore()->GetMaxU(), ( *c )->m_SurfB->GetSurfCore()->GetMinW() );
            vec3d max_max_A = ( *c )->m_SurfA->CompPnt( ( *c )->m_SurfA->GetSurfCore()->GetMaxU(), ( *c )->m_SurfA->GetSurfCore()->GetMaxW() );
            vec3d max_max_B = ( *c )->m_SurfB->CompPnt( ( *c )->m_SurfB->GetSurfCore()->GetMaxU(), ( *c )->m_SurfB->GetSurfCore()->GetMaxW() );

            if ( dist( min_min_A, min_max_B ) <= FLT_EPSILON && dist( max_min_A, max_max_B ) <= FLT_EPSILON )
            {
                closedW = true;
            }
            if ( dist( min_min_A, max_min_B ) <= FLT_EPSILON && dist( max_max_B, min_max_A ) <= FLT_EPSILON )
            {
                closedU = true;
            }

            // Check for magicV parm
            bool magicV = false;

            if ( ( ( std::abs( std::abs( ( *c )->m_SurfA->GetSurfCore()->GetMaxW() - ( *c )->m_SurfB->GetSurfCore()->GetMinW() ) - 2 * TMAGIC ) <= FLT_EPSILON ) )
                 && ( ( dist( min_max_A, min_min_B ) <= FLT_EPSILON ) && ( dist( max_max_A, max_min_B ) <= FLT_EPSILON ) ) )
            {
                magicV = true;
                tot_min_w -= TMAGIC;
                tot_max_w += TMAGIC;
            }

            int i = 0;

            while ( i < (int)surf_vec.size() && ( *c )->m_BorderFlag && ( *c )->m_SSIntersectIndex < 0 )
            {
                // Get all SubSurfaces for the specified geom
                vector < SimpleSubSurface > ss_vec = GetSimpSubSurfs( surf_vec[i]->GetGeomID(), surf_vec[i]->GetMainSurfID(), surf_vec[i]->GetCompID() );

                int ss = 0;

                while ( ss < (int)ss_vec.size() && ( *c )->m_BorderFlag && ( *c )->m_SSIntersectIndex < 0 )
                {
                    if ( ss_vec[ss].m_CreateBeamElements ) // Only consider SubSurface if cap intersections is flagged
                    {
                        // Split SubSurfs
                        ss_vec[ss].SplitSegsU( surf_vec[i]->GetSurfCore()->GetMinU() );
                        ss_vec[ss].SplitSegsU( surf_vec[i]->GetSurfCore()->GetMaxU() );
                        ss_vec[ss].SplitSegsW( surf_vec[i]->GetSurfCore()->GetMinW() );
                        ss_vec[ss].SplitSegsW( surf_vec[i]->GetSurfCore()->GetMaxW() );

                        vector < vector< SSLineSeg > >& segsvec = ss_vec[ss].GetSplitSegs();

                        // Build Intersection Chains
                        int j = 0;

                        while ( j < (int)segsvec.size() && ( *c )->m_BorderFlag && ( *c )->m_SSIntersectIndex < 0 )
                        {
                            vector< SSLineSeg >& segs = segsvec[j];
                            int ls = 0;

                            while ( ls < (int)segs.size() && ( *c )->m_BorderFlag && ( *c )->m_SSIntersectIndex < 0 )
                            {
                                vec3d lp0 = segs[ls].GetP0();
                                vec3d lp1 = segs[ls].GetP1();
                                vec2d uw_pnt0 = vec2d( lp0.x(), lp0.y() );
                                vec2d uw_pnt1 = vec2d( lp1.x(), lp1.y() );

                                // Cap subsurface edge points to within min and max U/W.
                                if ( uw_pnt0[0] < tot_min_u && closedU )
                                {
                                    uw_pnt0[0] = tot_min_u;
                                }
                                else if ( uw_pnt0[0] > tot_max_u && closedU )
                                {
                                    uw_pnt0[0] = tot_max_u;
                                }

                                if ( uw_pnt1[0] < tot_min_u && closedU )
                                {
                                    uw_pnt1[0] = tot_min_u;
                                }
                                else if ( uw_pnt1[0] > tot_max_u && closedU )
                                {
                                    uw_pnt1[0] = tot_max_u;
                                }

                                if ( uw_pnt0[1] < tot_min_w && closedW )
                                {
                                    uw_pnt0[1] = tot_min_w;
                                }
                                else if ( uw_pnt0[1] > tot_max_w && closedW )
                                {
                                    uw_pnt0[1] = tot_max_w;
                                }

                                if ( uw_pnt1[1] < tot_min_w && closedW )
                                {
                                    uw_pnt1[1] = tot_min_w;
                                }
                                else if ( uw_pnt1[1] > tot_max_w && closedW )
                                {
                                    uw_pnt1[1] = tot_max_w;
                                }

                                // Check for constant u/w curve
                                vector < vec3d > uw_curve = ( *c )->m_ACurve.GetUWTessPnts();
                                bool const_u_curve = true;
                                bool const_w_curve = true;

                                for ( size_t uw = 0; uw < uw_curve.size() - 1; uw++ )
                                {
                                    if ( std::abs( uw_curve[uw][0] - uw_curve.back()[0] ) > FLT_EPSILON )
                                    {
                                        const_u_curve = false;
                                    }
                                    if ( std::abs( uw_curve[uw][1] - uw_curve.back()[1] ) > FLT_EPSILON )
                                    {
                                        const_w_curve = false;
                                    }
                                }

                                // Get limits of surface edges
                                double tol = 1e-6;
                                double min_u = surf_vec[i]->GetSurfCore()->GetMinU();
                                double min_w = surf_vec[i]->GetSurfCore()->GetMinW();
                                double max_u = surf_vec[i]->GetSurfCore()->GetMaxU();
                                double max_w = surf_vec[i]->GetSurfCore()->GetMaxW();

                                if ( magicV )
                                {
                                    min_w -= TMAGIC;
                                    max_w += TMAGIC;
                                }

                                if ( ( ( std::abs( uw_pnt0[0] - max_u ) < tol && std::abs( uw_pnt1[0] - max_u ) < tol ) ||
                                    ( std::abs( uw_pnt0[1] - max_w ) < tol && std::abs( uw_pnt1[1] - max_w ) < tol ) ||
                                     ( std::abs( uw_pnt0[0] - min_u ) < tol && std::abs( uw_pnt1[0] - min_u ) < tol ) ||
                                     ( std::abs( uw_pnt0[1] - min_w ) < tol && std::abs( uw_pnt1[1] - min_w ) < tol ) )
                                        && ss_vec[ss].GetPolyFlag() )
                                {
                                    if ( ( dist( ( *c )->m_ISegDeque[0]->m_IPnt[0]->m_Puws[0]->m_UW, uw_pnt0 ) <= FLT_EPSILON
                                         && dist( ( *c )->m_ISegDeque.back()->m_IPnt[1]->m_Puws.back()->m_UW, uw_pnt1 ) <= FLT_EPSILON )
                                         || ( dist( ( *c )->m_ISegDeque[0]->m_IPnt[0]->m_Puws[0]->m_UW, uw_pnt1 ) <= FLT_EPSILON
                                         && dist( ( *c )->m_ISegDeque.back()->m_IPnt[1]->m_Puws.back()->m_UW, uw_pnt0 ) <= FLT_EPSILON ) )
                                    {
                                        // The border curve is also a SubSurface intersection curve, and has not been split by other curves
                                        ( *c )->m_SSIntersectIndex = ss;
                                        ( *c )->m_BorderFlag = false;
                                        break;
                                    }
                                    else if ( uw_pnt0[0] > min_u - FLT_EPSILON && uw_pnt1[0] > min_u - FLT_EPSILON && uw_pnt0[0] < max_u + FLT_EPSILON && uw_pnt1[0] < max_u + FLT_EPSILON
                                              && uw_pnt0[1] > min_w - FLT_EPSILON && uw_pnt1[1] > min_w - FLT_EPSILON && uw_pnt0[1] < max_w + FLT_EPSILON && uw_pnt1[1] < max_w + FLT_EPSILON )
                                    {
                                        // Reset SSLineSeg points and check again for valid UW
                                        uw_pnt0 = vec2d( lp0.x(), lp0.y() );
                                        uw_pnt1 = vec2d( lp1.x(), lp1.y() );

                                        if ( uw_pnt0[0] < surf_vec[i]->GetSurfCore()->GetMinU() )
                                        {
                                            uw_pnt0[0] = surf_vec[i]->GetSurfCore()->GetMinU();
                                        }
                                        else if ( uw_pnt0[0] > surf_vec[i]->GetSurfCore()->GetMaxU() )
                                        {
                                            uw_pnt0[0] = surf_vec[i]->GetSurfCore()->GetMaxU();
                                        }

                                        if ( uw_pnt1[0] < surf_vec[i]->GetSurfCore()->GetMinU() )
                                        {
                                            uw_pnt1[0] = surf_vec[i]->GetSurfCore()->GetMinU();
                                        }
                                        else if ( uw_pnt1[0] > surf_vec[i]->GetSurfCore()->GetMaxU() )
                                        {
                                            uw_pnt1[0] = surf_vec[i]->GetSurfCore()->GetMaxU();
                                        }

                                        if ( uw_pnt0[1] < surf_vec[i]->GetSurfCore()->GetMinW() )
                                        {
                                            uw_pnt0[1] = surf_vec[i]->GetSurfCore()->GetMinW();
                                        }
                                        else if ( uw_pnt0[1] > surf_vec[i]->GetSurfCore()->GetMaxW() )
                                        {
                                            uw_pnt0[1] = surf_vec[i]->GetSurfCore()->GetMaxW();
                                        }

                                        if ( uw_pnt1[1] < surf_vec[i]->GetSurfCore()->GetMinW() )
                                        {
                                            uw_pnt1[1] = surf_vec[i]->GetSurfCore()->GetMinW();
                                        }
                                        else if ( uw_pnt1[1] > surf_vec[i]->GetSurfCore()->GetMaxW() )
                                        {
                                            uw_pnt1[1] = surf_vec[i]->GetSurfCore()->GetMaxW();
                                        }

                                        vec3d corner0 = surf_vec[i]->CompPnt( uw_pnt0[0], uw_pnt0[1] );
                                        vec3d corner1 = surf_vec[i]->CompPnt( uw_pnt1[0], uw_pnt1[1] );

                                        // Check if the subsurface edge matched the ISegChain
                                        if ( ( dist( ( *c )->m_TessVec[0]->m_Pnt, corner0 ) <= FLT_EPSILON
                                             && dist( ( *c )->m_TessVec.back()->m_Pnt, corner1 ) <= FLT_EPSILON )
                                                || ( dist( ( *c )->m_TessVec.back()->m_Pnt, corner0 ) <= FLT_EPSILON
                                             && dist( ( *c )->m_TessVec[0]->m_Pnt, corner1 ) <= FLT_EPSILON ) )
                                        {
                                            // The border curve is also a SubSurface intersection curve. The SubSurface edge may exceed the U/W limits of the parent surface
                                            ( *c )->m_SSIntersectIndex = ss;
                                            ( *c )->m_BorderFlag = false;
                                            break;
                                        }

                                        // Check that the endpoints of the subsurface edge are not equal and the edge is a constant U or W curve
                                        if ( (dist( corner0, corner1 ) > FLT_EPSILON ) && 
                                            ( ( const_u_curve && std::abs( uw_pnt0[0] - uw_pnt1[0] ) <= FLT_EPSILON ) ||
                                            ( const_w_curve && std::abs( uw_pnt0[1] - uw_pnt1[1] ) <= FLT_EPSILON ) ) )
                                        {
                                            // Check if a split point lies on subsurface edge
                                            int k = 0;

                                            while ( k < (int)split_pnt_vec.size() && ( *c )->m_BorderFlag && ( *c )->m_SSIntersectIndex < 0 )
                                            {
                                                bool split_on_ss_edge = false;

                                                // Check if the split point lies on the subsurface edge 
                                                if ( const_w_curve && ( ( std::abs( split_uw_vec[k][1] - uw_pnt0[1] ) <= FLT_EPSILON ) || ( closedW && ( std::abs( 0.0 - uw_pnt0[1] ) <= FLT_EPSILON ) ) ) &&
                                                    ( ( split_uw_vec[k][0] >= uw_pnt0[0] && split_uw_vec[k][0] <= uw_pnt1[0] ) ||
                                                        ( split_uw_vec[k][0] >= uw_pnt1[0] && split_uw_vec[k][0] <= uw_pnt0[0] ) ) )
                                                {
                                                    split_on_ss_edge = true;
                                                }
                                                else if ( const_u_curve && ( ( std::abs( split_uw_vec[k][0] - uw_pnt0[0] ) <= FLT_EPSILON ) || ( closedU && ( std::abs( 0.0 - uw_pnt0[0] ) <= FLT_EPSILON ) ) ) &&
                                                    ( ( split_uw_vec[k][1] >= uw_pnt0[1] && split_uw_vec[k][1] <= uw_pnt1[1] ) ||
                                                        ( split_uw_vec[k][1] >= uw_pnt1[1] && split_uw_vec[k][1] <= uw_pnt0[1] ) ) )
                                                {
                                                    split_on_ss_edge = true;
                                                }

                                                if ( split_on_ss_edge )
                                                {
                                                    if ( ( dist( ( *c )->m_ISegDeque[0]->m_IPnt[0]->m_Pnt, split_pnt_vec[k] ) <= FLT_EPSILON
                                                            && dist( ( *c )->m_TessVec.back()->m_Pnt, corner1 ) <= FLT_EPSILON
                                                            && dist( ( *c )->m_TessVec[0]->m_Pnt, corner0 ) > FLT_EPSILON )
                                                            || ( dist( ( *c )->m_ISegDeque.back()->m_IPnt[1]->m_Pnt, split_pnt_vec[k] ) <= FLT_EPSILON
                                                            && dist( ( *c )->m_TessVec[0]->m_Pnt, corner1 ) <= FLT_EPSILON
                                                            && dist( ( *c )->m_TessVec.back()->m_Pnt, corner0 ) > FLT_EPSILON ) )
                                                    {
                                                        // The border curve is also a SubSurface intersection curve and has been split
                                                        ( *c )->m_SSIntersectIndex = ss;
                                                        ( *c )->m_BorderFlag = false;
                                                        break;
                                                    }
                                                    else if ( ( dist( ( *c )->m_ISegDeque[0]->m_IPnt[0]->m_Pnt, split_pnt_vec[k] ) <= FLT_EPSILON
                                                                && dist( ( *c )->m_TessVec.back()->m_Pnt, corner0 ) <= FLT_EPSILON
                                                                && dist( ( *c )->m_TessVec[0]->m_Pnt, corner1 ) > FLT_EPSILON )
                                                            || ( dist( ( *c )->m_ISegDeque.back()->m_IPnt[1]->m_Pnt, split_pnt_vec[k] ) <= FLT_EPSILON
                                                                && dist( ( *c )->m_TessVec[0]->m_Pnt, corner0 ) <= FLT_EPSILON
                                                                && dist( ( *c )->m_TessVec.back()->m_Pnt, corner1 ) > FLT_EPSILON ) )
                                                    {
                                                        // The border curve is also a SubSurface intersection curve and has been split
                                                        ( *c )->m_SSIntersectIndex = ss;
                                                        ( *c )->m_BorderFlag = false;
                                                        break;
                                                    }
                                                }

                                                // Check if the subsurface edge is split by more than one point
                                                int n = 0;

                                                while ( n < (int)split_pnt_vec.size() && ( *c )->m_BorderFlag && ( *c )->m_SSIntersectIndex < 0 )
                                                {
                                                    split_on_ss_edge = false;

                                                    if ( const_w_curve && ( ( std::abs( split_uw_vec[n][1] - uw_pnt0[1] ) <= FLT_EPSILON ) || ( closedW && ( std::abs( 0.0 - uw_pnt0[1] ) <= FLT_EPSILON ) ) ) &&
                                                        ( ( ( split_uw_vec[k][0] >= uw_pnt0[0] && split_uw_vec[k][0] <= uw_pnt1[0] ) ||
                                                            ( split_uw_vec[k][0] >= uw_pnt1[0] && split_uw_vec[k][0] <= uw_pnt0[0] ) ) &&
                                                            ( ( split_uw_vec[n][0] >= uw_pnt0[0] && split_uw_vec[n][0] <= uw_pnt1[0] ) ||
                                                            ( split_uw_vec[n][0] >= uw_pnt1[0] && split_uw_vec[n][0] <= uw_pnt0[0] ) ) ) )
                                                    {
                                                        split_on_ss_edge = true;
                                                    }
                                                    else if ( const_u_curve && ( ( std::abs( split_uw_vec[n][0] - uw_pnt0[0] ) <= FLT_EPSILON ) || ( closedU && ( std::abs( 0.0 - uw_pnt0[0] ) <= FLT_EPSILON ) ) ) &&
                                                        ( ( ( split_uw_vec[k][1] >= uw_pnt0[1] && split_uw_vec[k][1] <= uw_pnt1[1] ) ||
                                                            ( split_uw_vec[k][1] >= uw_pnt1[1] && split_uw_vec[k][1] <= uw_pnt0[1] ) ) &&
                                                                ( ( split_uw_vec[n][1] >= uw_pnt0[1] && split_uw_vec[n][1] <= uw_pnt1[1] ) ||
                                                                ( split_uw_vec[n][1] >= uw_pnt1[1] && split_uw_vec[n][1] <= uw_pnt0[1] ) ) ) )
                                                    {
                                                        split_on_ss_edge = true;
                                                    }

                                                    if ( split_on_ss_edge && n != k )
                                                    {
                                                        if ( ( dist( ( *c )->m_ISegDeque[0]->m_IPnt[0]->m_Pnt, split_pnt_vec[k] ) <= FLT_EPSILON
                                                                && dist( ( *c )->m_ISegDeque.back()->m_IPnt[1]->m_Pnt, split_pnt_vec[n] ) <= FLT_EPSILON )
                                                                || ( dist( ( *c )->m_ISegDeque.back()->m_IPnt[1]->m_Pnt, split_pnt_vec[k] ) <= FLT_EPSILON
                                                                && dist( ( *c )->m_ISegDeque[0]->m_IPnt[0]->m_Pnt, split_pnt_vec[n] ) <= 2 * FLT_EPSILON ) )
                                                        {
                                                            // The border curve is also a SubSurface intersection curve and has been split
                                                            ( *c )->m_SSIntersectIndex = ss;
                                                            ( *c )->m_BorderFlag = false;
                                                            break;
                                                        }
                                                        else if ( ( dist( ( *c )->m_ISegDeque[0]->m_IPnt[0]->m_Pnt, split_pnt_vec[n] ) <= FLT_EPSILON
                                                                    && dist( ( *c )->m_ISegDeque.back()->m_IPnt[1]->m_Pnt, split_pnt_vec[k] ) <= FLT_EPSILON )
                                                                || ( dist( ( *c )->m_ISegDeque.back()->m_IPnt[1]->m_Pnt, split_pnt_vec[n] ) <= FLT_EPSILON
                                                                    && dist( ( *c )->m_ISegDeque[0]->m_IPnt[0]->m_Pnt, split_pnt_vec[k] ) <= FLT_EPSILON ) )
                                                        {
                                                            // The border curve is also a SubSurface intersection curve and has been split
                                                            ( *c )->m_SSIntersectIndex = ss;
                                                            ( *c )->m_BorderFlag = false;
                                                            break;
                                                        }
                                                    }
                                                    n++; // increase splitpnt index
                                                }
                                                k++; // increase splitpnt index
                                            }
                                        }
                                    }
                                }
                                ls++; // increase seg index
                            }
                            j++; // increase segvec index
                        }
                    }
                    ss++; // increase subsurface index
                }
                i++; // increase surf index
            }
        }
    }
}

void FeaMeshMgrSingleton::MergeFeaPartSSEdgeOverlap()
{
    if ( !GetMeshPtr() )
    {
        return;
    }

    // Check for SubSurface Edges on FeaPart Surfaces. If found, remove the SubSurface intersectionchain and 
    //  allow it to be replaced by the FeaPart intersection curve. 
    vec2d uw_pnt0;
    vec2d uw_pnt1;
    vector < ISegChain* > remove_chain_list;

    for ( size_t i = 0; i < (int)m_SurfVec.size(); i++ )
    {
        for ( size_t j = 0; j < (int)m_SurfVec.size(); j++ )
        {
            Surf* surfA = m_SurfVec[i]; // FeaPart Surface
            Surf* surfB = m_SurfVec[j]; // Skin Surface

            if ( ( surfA->GetCompID() < 0 ) && ( surfB->GetCompID() >= 0 ) && ( i != j ) && ( GetMeshPtr()->m_FeaPartTypeVec[surfA->GetFeaPartIndex()] != vsp::FEA_DOME ) )
            {
                vector < SimpleSubSurface > ss_vec = GetSimpSubSurfs( surfB->GetGeomID(), surfB->GetMainSurfID(), surfB->GetCompID() );

                // Split SubSurfs
                for ( int ss = 0; ss < (int)ss_vec.size(); ss++ )
                {
                    ss_vec[ss].SplitSegsU( surfB->GetSurfCore()->GetMinU() );
                    ss_vec[ss].SplitSegsU( surfB->GetSurfCore()->GetMaxU() );
                    ss_vec[ss].SplitSegsW( surfB->GetSurfCore()->GetMinW() );
                    ss_vec[ss].SplitSegsW( surfB->GetSurfCore()->GetMaxW() );

                    vector < vector< SSLineSeg > >& segsvec = ss_vec[ss].GetSplitSegs();

                    for ( int k = 0; k < segsvec.size(); k++ )
                    {
                        vector< SSLineSeg >& segs = segsvec[k];
                        bool is_poly = ss_vec[ss].GetPolyFlag();

                        // Build Intersection Chains
                        for ( int ls = 0; ls < (int)segs.size(); ls++ )
                        {
                            SSLineSeg l_seg = segs[ls];
                            vec3d lp0, lp1;

                            lp0 = l_seg.GetP0();
                            lp1 = l_seg.GetP1();
                            uw_pnt0 = vec2d( lp0.x(), lp0.y() );
                            uw_pnt1 = vec2d( lp1.x(), lp1.y() );
                            double max_u, max_w, tol;
                            double min_u, min_w;
                            tol = 1e-6;
                            min_u = surfB->GetSurfCore()->GetMinU();
                            min_w = surfB->GetSurfCore()->GetMinW();
                            max_u = surfB->GetSurfCore()->GetMaxU();
                            max_w = surfB->GetSurfCore()->GetMaxW();

                            if ( uw_pnt0[0] < min_u - FLT_EPSILON || uw_pnt0[1] < min_w - FLT_EPSILON || uw_pnt1[0] < min_u - FLT_EPSILON || uw_pnt1[1] < min_w - FLT_EPSILON )
                            {
                                continue; // Skip if either point has a value not on this surface
                            }
                            if ( uw_pnt0[0] > max_u + FLT_EPSILON || uw_pnt0[1] > max_w + FLT_EPSILON || uw_pnt1[0] > max_u + FLT_EPSILON || uw_pnt1[1] > max_w + FLT_EPSILON )
                            {
                                continue; // Skip if either point has a value not on this surface
                            }
                            if ( ( ( std::abs( uw_pnt0[0] - max_u ) < tol && std::abs( uw_pnt1[0] - max_u ) < tol ) ||
                                ( std::abs( uw_pnt0[1] - max_w ) < tol && std::abs( uw_pnt1[1] - max_w ) < tol ) ||
                                 ( std::abs( uw_pnt0[0] - min_u ) < tol && std::abs( uw_pnt1[0] - min_u ) < tol ) ||
                                 ( std::abs( uw_pnt0[1] - min_w ) < tol && std::abs( uw_pnt1[1] - min_w ) < tol ) )
                                 && is_poly )
                            {
                                continue; // Skip if both end points are on the same edge of the surface
                            }

                            // Project SubSurface edge point on FeaPart surface
                            vec3d skin_pnt0 = surfB->CompPnt( uw_pnt0[0], uw_pnt0[1] );
                            vec3d skin_pnt1 = surfB->CompPnt( uw_pnt1[0], uw_pnt1[1] );

                            double part_U0, part_W0, part_U1, part_W1;
                            surfA->GetSurfCore()->FindNearest( part_U0, part_W0, skin_pnt0 );
                            surfA->GetSurfCore()->FindNearest( part_U1, part_W1, skin_pnt1 );

                            vec3d part_pnt0 = surfA->GetSurfCore()->CompPnt( part_U0, part_W0 );
                            vec3d part_pnt1 = surfA->GetSurfCore()->CompPnt( part_U1, part_W1 );

                            if ( dist_squared( part_pnt0, skin_pnt0 ) <= FLT_EPSILON && dist_squared( part_pnt1, skin_pnt1 ) <= FLT_EPSILON )
                            {
                                list< ISegChain* >::iterator c1, c2;
                                for ( c1 = m_ISegChainList.begin(); c1 != m_ISegChainList.end(); ++c1 )
                                {
                                    if ( ( *c1 )->m_SurfA == ( *c1 )->m_SurfB ) // Indicates SubSurface ISegChain
                                    {
                                        if ( ( dist_squared( part_pnt0, ( *c1 )->m_ISegDeque[0]->m_IPnt[0]->m_Pnt ) <= FLT_EPSILON && dist_squared( part_pnt1, ( *c1 )->m_ISegDeque.back()->m_IPnt[1]->m_Pnt ) <= FLT_EPSILON ) ||
                                            ( dist_squared( part_pnt0, ( *c1 )->m_ISegDeque.back()->m_IPnt[1]->m_Pnt ) <= FLT_EPSILON && dist_squared( part_pnt1, ( *c1 )->m_ISegDeque[0]->m_IPnt[0]->m_Pnt ) <= FLT_EPSILON ) )
                                        {
                                            // Check if this ISegChain has already been added to remove_chain_list
                                            if ( std::find( remove_chain_list.begin(), remove_chain_list.end(), ( *c1 ) ) == remove_chain_list.end() )
                                            {
                                                string part = GetMeshPtr()->m_FeaPartNameVec[surfA->GetFeaPartIndex()];
                                                string message = "Merged Intersection Curve: " + part + " and " + ss_vec[ss].GetName() + "\n";
                                                addOutputText( message );

                                                remove_chain_list.push_back( *c1 );

                                                // Split FeaPart ISegChain at SubSurface edge points
                                                for ( c2 = m_ISegChainList.begin(); c2 != m_ISegChainList.end(); ++c2 )
                                                {
                                                    if ( ( *c1 ) != ( *c2 ) && ( ( ( *c2 )->m_SurfA == surfA && ( *c2 )->m_SurfB == surfB ) || ( ( *c2 )->m_SurfB == surfA && ( *c2 )->m_SurfA == surfB ) ) )
                                                    {
                                                        vec2d part_UW0 = vec2d( part_U0, part_W0 );
                                                        vec2d part_UW1 = vec2d( part_U1, part_W1 );

                                                        Puw* part_p0 = new Puw( surfA, part_UW0 );
                                                        Puw* skin_p0 = new Puw( surfB, uw_pnt0 );

                                                        Puw* part_p1 = new Puw( surfA, part_UW1 );
                                                        Puw* skin_p1 = new Puw( surfB, uw_pnt1 );

                                                        bool success0 = false;
                                                        bool success1 = false;

                                                        if ( part_p0 )
                                                        {
                                                            success0 = ( *c2 )->AddBorderSplit( part_p0 );
                                                        }
                                                        if ( skin_p0 && !success0 )
                                                        {
                                                            success0 = ( *c2 )->AddBorderSplit( skin_p0 );
                                                        }

                                                        if ( part_p1 )
                                                        {
                                                            success1 = ( *c2 )->AddBorderSplit( part_p1 );
                                                        }
                                                        if ( skin_p1 && !success1 )
                                                        {
                                                            success1 = ( *c2 )->AddBorderSplit( skin_p1 );
                                                        }

                                                        // Free memory
                                                        if ( !success0 )
                                                        {
                                                            if ( part_p0 )
                                                            {
                                                                delete part_p0;
                                                            }
                                                            if ( skin_p0 )
                                                            {
                                                                delete skin_p0;
                                                            }
                                                        }
                                                        if ( !success1 )
                                                        {
                                                            if ( part_p1 )
                                                            {
                                                                delete part_p1;
                                                            }
                                                            if ( skin_p1 )
                                                            {
                                                                delete skin_p1;
                                                            }
                                                        }
                                                    }
                                                }
                                            }
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }

    // Only keep ISegChains not added to remove_chain_list
    list< ISegChain* >::iterator c;
    list < ISegChain* > old_chain_list = m_ISegChainList;
    m_ISegChainList.clear();

    for ( c = old_chain_list.begin(); c != old_chain_list.end(); ++c )
    {
        if ( std::find( remove_chain_list.begin(), remove_chain_list.end(), ( *c ) ) == remove_chain_list.end() )
        {
            m_ISegChainList.push_back( ( *c ) );
        }
    }
}

void FeaMeshMgrSingleton::RemoveSubSurfFeaTris()
{
    if ( !GetMeshPtr() )
    {
        return;
    }

    vector< FeaElement* > newFeaElementVec;
    newFeaElementVec.reserve( GetMeshPtr()->m_FeaElementVec.size() );

    for ( int j = 0; j < GetMeshPtr()->m_FeaElementVec.size(); j++ )
    {
        bool delj = false;

        if ( GetMeshPtr()->m_FeaElementVec[ j ]->GetElementType() == FeaElement::FEA_TRI_3 ||
             GetMeshPtr()->m_FeaElementVec[ j ]->GetElementType() == FeaElement::FEA_TRI_6 ||
             GetMeshPtr()->m_FeaElementVec[ j ]->GetElementType() == FeaElement::FEA_QUAD_4 ||
             GetMeshPtr()->m_FeaElementVec[ j ]->GetElementType() == FeaElement::FEA_QUAD_8 )
        {
            for ( unsigned int i = 0; i < GetMeshPtr()->m_NumFeaSubSurfs; i++ )
            {
                if ( GetMeshPtr()->m_FeaElementVec[ j ]->GetFeaSSIndex() == i )
                {
                    if ( m_SimpleSubSurfaceVec[ i ].m_KeepDelShellElements == vsp::FEA_DELETE )
                    {
                        delj = true;
                        break;
                    }
                }
            }
        }

        if ( delj )
        {
            delete GetMeshPtr()->m_FeaElementVec[ j ];
        }
        else
        {
            newFeaElementVec.push_back( GetMeshPtr()->m_FeaElementVec[ j ] );
        }
    }

    GetMeshPtr()->m_FeaElementVec = newFeaElementVec;
}

void FeaMeshMgrSingleton::TagFeaNodes()
{
    if ( !GetMeshPtr() )
    {
        return;
    }

    // Number of spider points to find for on-body fixed points.
    int nspider = 6;

    //==== Collect All FeaNodes ====//
    GetMeshPtr()->m_FeaNodeVec.clear();

    // Load off body fixed point nodes.
    for ( size_t j = 0; j < GetMeshPtr()->m_FeaOffBodyFixPointNodeVec.size(); j++ )
    {
        GetMeshPtr()->m_FeaNodeVec.push_back( GetMeshPtr()->m_FeaOffBodyFixPointNodeVec[j] );
    }

    for ( int i = 0; i < (int)GetMeshPtr()->m_FeaElementVec.size(); i++ )
    {
        GetMeshPtr()->m_FeaElementVec[i]->LoadNodes( GetMeshPtr()->m_FeaNodeVec );
    }

    vector< vec3d > allPntVec;
    allPntVec.reserve( GetMeshPtr()->m_FeaNodeVec.size() );

    for ( int i = 0; i < (int)GetMeshPtr()->m_FeaNodeVec.size(); i++ )
    {
        allPntVec.push_back( GetMeshPtr()->m_FeaNodeVec[i]->m_Pnt );
    }

    PntNodeCloud pnCloud;
    pnCloud.AddPntNodes( allPntVec );

    double tol = 1e-6;
    //==== Use NanoFlann to Find Close Points and Group ====//
    IndexPntNodes( pnCloud, tol );

    GetMeshPtr()->m_FeaNodeVecUsed.resize( GetMeshPtr()->m_FeaNodeVec.size() );
    for ( int i = 0; i < (int)GetMeshPtr()->m_FeaNodeVec.size(); i++ )
    {
        GetMeshPtr()->m_FeaNodeVecUsed[i] = pnCloud.UsedNode( i );
    }

    // Count fixed points for additional offset.
    long long int fixptoffset = 0;
    for ( size_t j = 0; j < GetMeshPtr()->m_NumFeaFixPoints; j++ )
    {
        int spidermult = 1;
        if ( GetMeshPtr()->m_FixPntVec[j].m_OnBody )
        {
            spidermult += nspider;
        }

        fixptoffset += spidermult * GetMeshPtr()->m_FixPntVec[j].m_Pnt.size();
    }
    GetMeshPtr()->m_FixPtOffset = fixptoffset;

    //==== Assign Index Numbers to Nodes ====//
    for ( int i = 0; i < (int)GetMeshPtr()->m_FeaNodeVec.size(); i++ )
    {
        GetMeshPtr()->m_FeaNodeVec[i]->m_Tags.clear();
        GetMeshPtr()->m_FeaNodeVec[i]->m_Index = pnCloud.GetNodeUsedIndex( i ) + 1 + fixptoffset;
    }

    // Override index numbers for fixed points.  Also set other fixed point settings and add mass elements.
    long long int ifixpt = 0;
    for ( size_t j = 0; j < GetMeshPtr()->m_NumFeaFixPoints; j++ )
    {
        FixPoint fxpt = GetMeshPtr()->m_FixPntVec[j];
        for ( size_t k = 0; k < fxpt.m_Pnt.size(); k++ )
        {
            int ind = pnCloud.LookupPntBase( fxpt.m_Pnt[k] );

            if ( ind >= 0 )
            {
                vector < long long int > matches = pnCloud.GetMatches( ind );
                for ( size_t i = 0; i < matches.size(); i++ )
                {
                    // Override index numbers for fixed points.
                    GetMeshPtr()->m_FeaNodeVec[ matches[i] ]->m_Index = ifixpt + 1;

                    GetMeshPtr()->m_FeaNodeVec[ matches[i] ]->AddTag( fxpt.m_FeaPartIndex );
                    GetMeshPtr()->m_FeaNodeVec[ matches[i] ]->m_FixedPointFlag = true;
                }
            }
            else
            {
                printf( "Point not found.\n" );
            }

            // Set fix point node index here.
            GetMeshPtr()->m_FixPntVec[ j ].m_NodeIndex[ k ] = ifixpt + 1;

            // Create mass element if mass flag is true
            if ( fxpt.m_PtMassFlag )
            {
                FeaPointMass *mass = new FeaPointMass;
                mass->Create( GetMeshPtr()->m_FeaNodeVec[ ind ]->m_Pnt, fxpt.m_PtMass );
                mass->SetFeaPartIndex( fxpt.m_FeaPartIndex );

                mass->m_Corners[ 0 ]->m_Index = ifixpt + 1;

                GetMeshPtr()->m_FeaElementVec.push_back( mass );
            }


            ifixpt++;
        }
    }

    // Tag FeaPart Nodes with FeaPart Index
    for ( unsigned int i = 0; i < GetMeshPtr()->m_NumFeaParts; i++ )
    {
        vector< FeaNode* > temp_nVec;

        for ( int j = 0; j < GetMeshPtr()->m_FeaElementVec.size(); j++ )
        {
            if ( GetMeshPtr()->m_FeaElementVec[j]->GetFeaPartIndex() == i && GetMeshPtr()->m_FeaElementVec[j]->GetFeaSSIndex() < 0 )
            {
                GetMeshPtr()->m_FeaElementVec[j]->LoadNodes( temp_nVec );
            }
        }

        for ( int j = 0; j < (int)temp_nVec.size(); j++ )
        {
            int ind = pnCloud.LookupPntBase( temp_nVec[j]->m_Pnt );

            if ( ind >= 0 )
            {
                GetMeshPtr()->m_FeaNodeVec[ ind ]->AddTag( i );
            }
            else
            {
                printf( "Point not found.\n" );
            }
        }
    }

    // Tag FeaSubSurface Nodes with FeaSubSurface Index, beginning at the last FeaPart index (GetMeshPtr()->m_NumFeaParts)
    for ( unsigned int i = 0; i < GetMeshPtr()->m_NumFeaSubSurfs; i++ )
    {
        vector< FeaNode* > temp_nVec;

        for ( int j = 0; j < GetMeshPtr()->m_FeaElementVec.size(); j++ )
        {
            if ( GetMeshPtr()->m_FeaElementVec[j]->GetFeaSSIndex() == i )
            {
                GetMeshPtr()->m_FeaElementVec[j]->LoadNodes( temp_nVec );
            }
        }

        for ( int j = 0; j < (int)temp_nVec.size(); j++ )
        {
            int ind = pnCloud.LookupPntBase( temp_nVec[j]->m_Pnt );

            if ( ind >= 0 )
            {
                GetMeshPtr()->m_FeaNodeVec[ ind ]->AddTag( i + GetMeshPtr()->m_NumFeaParts );
            }
            else
            {
                printf( "Point not found.\n" );
            }
        }
    }

    // Find 'spider' points
    // Needs to be after tags are applied for classification.
    for ( size_t j = 0; j < GetMeshPtr()->m_NumFeaFixPoints; j++ )
    {
        FixPoint fxpt = GetMeshPtr()->m_FixPntVec[j];
        if ( fxpt.m_OnBody )
        {
            for ( size_t k = 0; k < fxpt.m_Pnt.size(); k++ )
            {
                int ind = pnCloud.LookupPntBase( fxpt.m_Pnt[k] );

                // Find nearest N points.
                // N is over-sized to deal with duplicates.
                vector < long long int > results_vec;
                pnCloud.LookupPntBase( fxpt.m_Pnt[k], 6*6*6, results_vec );

                // Eliminate duplicates
                // Points are in distance order
                vector_remove_duplicates_preserve_order( results_vec );

                // Remove center point.
                vector_remove_val( results_vec, (long long int ) ind );

                // Filter to only used points on parent point
                vector < long long int > keep_vec;
                for ( int i = 0; i < (int)results_vec.size(); i++ )
                {
                    int ires = results_vec[i];
                    if (  GetMeshPtr()->m_FeaNodeVecUsed[ ires ] &&
                          GetMeshPtr()->m_FeaNodeVec[ ires ]->HasTag( fxpt.m_FeaParentPartIndex ) )
                    {
                        keep_vec.push_back( ires );
                    }
                }
                results_vec = keep_vec;

                // Trim to six nearest points
                results_vec.resize( 6 );

                // Store spider points.
                GetMeshPtr()->m_FixPntVec[ j ].m_SpiderIndex[ k ] = results_vec;

                // Renumber spider nodes
                for ( int i = 0; i < results_vec.size(); i++ )
                {
                    int ires = results_vec[i];
                    if ( GetMeshPtr()->m_FeaNodeVec[ ires ]->m_Index > fixptoffset )
                    {
                        // printf("Renumbering spider point %d to %d\n", (int) (GetMeshPtr()->m_FeaNodeVec[ ires ]->m_Index), (int) (ifixpt + 1) );

                        vector < long long int > matches = pnCloud.GetMatches( ires );
                        for ( size_t imatch = 0; imatch < matches.size(); imatch++ )
                        {
                            // Override index numbers for spider points.
                            GetMeshPtr()->m_FeaNodeVec[ matches[imatch] ]->m_Index = ifixpt + 1;
                        }
                        ifixpt++;
                    }
                }
            }
        }
    }

    // Apply BC's to nodes.
    for ( unsigned int i = 0; i < GetMeshPtr()->m_BCVec.size(); i++ )
    {
        for ( unsigned int j = 0; j < (int)GetMeshPtr()->m_FeaNodeVec.size(); j++ )
        {
            if ( GetMeshPtr()->m_FeaNodeVecUsed[ j ] )
            {
                GetMeshPtr()->m_BCVec[i].ApplyTo( GetMeshPtr()->m_FeaNodeVec[j] );
            }
        }
    }

    long long int maxid = 0;
    for ( int i = 0; i < (int)GetMeshPtr()->m_FeaNodeVec.size(); i++ )
    {
        if ( GetMeshPtr()->m_FeaNodeVecUsed[ i ] )
        {
            maxid = std::max( maxid, GetMeshPtr()->m_FeaNodeVec[ i ]->m_Index );
        }
    }
    GetMeshPtr()->m_NumNodes = maxid;
}

void FeaMeshMgrSingleton::TransferDrawObjData()
{
    FeaStructure* fea_struct = StructureMgr.GetFeaStruct( m_FeaStructID );

    if ( fea_struct && GetMeshPtr() )
    {
        // FeaParts:
        for ( unsigned int i = 0; i < GetMeshPtr()->m_NumFeaParts; i++ )
        {
            string name = GetMeshPtr()->m_StructName + ":  " + GetMeshPtr()->m_FeaPartNameVec[i];

            if ( GetMeshPtr()->m_FeaPartKeepDelShellElementsVec[i] == vsp::FEA_KEEP )
            {
                GetMeshPtr()->m_DrawBrowserNameVec.push_back( name );
                GetMeshPtr()->m_DrawBrowserPartIndexVec.push_back( i );
            }

            GetMeshPtr()->m_FixPointFeaPartFlagVec.push_back( GetMeshPtr()->m_FeaPartTypeVec[i] == vsp::FEA_FIX_POINT );

            GetMeshPtr()->m_DrawElementFlagVec.push_back( true );

            if ( GetMeshPtr()->m_FeaPartCreateBeamElementsVec[i] )
            {
                name += "_CAP";
                GetMeshPtr()->m_DrawBrowserNameVec.push_back( name );
                GetMeshPtr()->m_DrawBrowserPartIndexVec.push_back( i );
                GetMeshPtr()->m_DrawCapFlagVec.push_back( true );
            }
            else
            {
                GetMeshPtr()->m_DrawCapFlagVec.push_back( false );
            }
        }

        // FeaSubSurfaces:
        for ( unsigned int i = 0; i < GetMeshPtr()->m_NumFeaSubSurfs; i++ )
        {
            string name = GetMeshPtr()->m_StructName + ":  " + m_SimpleSubSurfaceVec[i].GetName();

            if ( m_SimpleSubSurfaceVec[i].m_TestType != vsp::NONE && m_SimpleSubSurfaceVec[i].m_KeepDelShellElements == vsp::FEA_KEEP )
            {
                GetMeshPtr()->m_DrawBrowserNameVec.push_back( name );
                GetMeshPtr()->m_DrawBrowserPartIndexVec.push_back( GetMeshPtr()->m_NumFeaParts + i );
            }

            GetMeshPtr()->m_DrawElementFlagVec.push_back( true );

            if ( m_SimpleSubSurfaceVec[i].m_CreateBeamElements )
            {
                name += "_CAP";
                GetMeshPtr()->m_DrawBrowserNameVec.push_back( name );
                GetMeshPtr()->m_DrawBrowserPartIndexVec.push_back( GetMeshPtr()->m_NumFeaParts + i );
                GetMeshPtr()->m_DrawCapFlagVec.push_back( true );
            }
            else
            {
                GetMeshPtr()->m_DrawCapFlagVec.push_back( false );
            }
        }
    }
}

void FeaMeshMgrSingleton::UpdateDrawObjs()
{
    SurfaceIntersectionSingleton::UpdateDrawObjs();

    if ( GetMeshPtr() )
    {
        GetMeshPtr()->UpdateDrawObjs();
    }
}

void FeaMeshMgrSingleton::LoadDrawObjs( vector< DrawObj* > & draw_obj_vec )
{
    if ( !GetFeaMeshInProgress() )
    {
        if ( m_IntersectComplete )
        {
            SurfaceIntersectionSingleton::LoadDrawObjs( draw_obj_vec );
        }

        if ( GetMeshPtr() )
        {
            GetMeshPtr()->LoadDrawObjs( draw_obj_vec, GetMeshPtr()->GetStructSettingsPtr() );
        }
    }
}

void FeaMeshMgrSingleton::UpdateDisplaySettings()
{
    if ( GetMeshPtr() )
    {
        GetMeshPtr()->UpdateDisplaySettings();
    }
}

void FeaMeshMgrSingleton::UpdateAssemblyDisplaySettings( const string &assembly_id )
{
    FeaAssembly* fea_assembly = StructureMgr.GetFeaAssembly( assembly_id );

    if ( fea_assembly )
    {
        m_AssemblySettings.m_DrawAsMeshFlag = fea_assembly->m_AssemblySettings.m_DrawAsMeshFlag.Get();
        m_AssemblySettings.m_DrawMeshFlag = fea_assembly->m_AssemblySettings.m_DrawMeshFlag.Get();
        m_AssemblySettings.m_ColorFacesFlag = fea_assembly->m_AssemblySettings.m_ColorFacesFlag.Get();
        m_AssemblySettings.m_ColorTagReason = fea_assembly->m_AssemblySettings.m_ColorTagReason.Get();

        m_AssemblySettings.m_DrawNodesFlag = fea_assembly->m_AssemblySettings.m_DrawNodesFlag.Get();
        m_AssemblySettings.m_DrawBCNodesFlag = fea_assembly->m_AssemblySettings.m_DrawBCNodesFlag.Get();
        m_AssemblySettings.m_DrawElementOrientVecFlag = fea_assembly->m_AssemblySettings.m_DrawElementOrientVecFlag.Get();
    }
}

void FeaMeshMgrSingleton::RegisterAnalysis()
{
    if (!AnalysisMgr.FindAnalysis( "FeaMeshAnalysis" ))
    {
        FeaMeshAnalysis* sia = new FeaMeshAnalysis();

        if ( sia && !AnalysisMgr.RegisterAnalysis( sia ) )
        {
            delete sia;
        }
    }
}

Surf* FeaMeshMgrSingleton::GetFeaSurf( int FeaPartID, int surf_num )
{
    for ( int i = 0; i < m_SurfVec.size(); i++ )
    {
        if ( m_SurfVec[i]->GetFeaPartIndex() == FeaPartID && m_SurfVec[i]->GetFeaPartSurfNum() == surf_num )
        {
            return m_SurfVec[i];
        }
    }
    return NULL;
}

void FeaMeshMgrSingleton::MeshUnMeshed( const vector < string > & idvec )
{
    for ( int i = 0; i < idvec.size(); i++ )
    {
        SetFeaMeshStructID( idvec[i] );

        if ( GetMeshPtr() )
        {
            if ( !GetMeshPtr()->m_MeshReady )
            {
                GenerateFeaMesh();

                // Trigger screen update after each mesh generation.
                MessageMgr::getInstance().Send( "ScreenMgr", "UpdateAllScreens" );
            }
        }
    }
}

void FeaMeshMgrSingleton::CleanupMeshes( const vector < string > & idvec )
{
    for ( int i = 0; i < idvec.size(); i++ )
    {
        FeaMesh* mesh = GetMeshPtr( idvec[i] );
        if ( mesh )
        {
            mesh->Cleanup();
        }
    }
}

void FeaMeshMgrSingleton::ExportAssemblyMesh( const string &assembly_id )
{
    FeaAssembly* fea_assembly = StructureMgr.GetFeaAssembly( assembly_id );

    if ( !fea_assembly )
    {
        return;
    }

    vector < string > idvec = fea_assembly->m_StructIDVec;

    m_AssemblySettings.CopyPostOpFrom( &(fea_assembly->m_AssemblySettings) );

    addOutputText( "Exporting Assembly Mesh.\n" );

    // Transfer common property and material data to FeaMeshMgr.
    TransferPropMatData();

    // Compute and store offsets.
    int noffset = 0;
    int eoffset = 0;

    int connoffset = 0;

    FeaCount feacount;

    for ( int i = 0; i < idvec.size(); i++ )
    {
        FeaMesh* mesh = GetMeshPtr( idvec[i] );
        if ( mesh )
        {
            FeaStructure* fea_struct = StructureMgr.GetFeaStruct( idvec[i] );

            if ( fea_struct )
            {
                // Store in Parm.
                fea_struct->GetStructSettingsPtr()->m_NodeOffset.Set( noffset );
            }

            feacount.m_NumNodes += mesh->m_NumNodes;
            feacount.m_NumEls += mesh->m_NumEls;
            feacount.m_NumTris += mesh->m_NumTris;
            feacount.m_NumQuads += mesh->m_NumQuads;
            feacount.m_NumBeams += mesh->m_NumBeams;

            int maxn = noffset + mesh->m_NumNodes;

            // Round up at magnitude of number.  Consider ceil2scale( n, 1000 ); instead.
            // eoffset = magroundup( maxn );
            eoffset = ceil2scale( maxn, 1000 );

            if ( fea_struct )
            {
                // Store in Parm.
                fea_struct->GetStructSettingsPtr()->m_ElementOffset.Set( eoffset );

                // Transfer to mesh.
                mesh->m_StructSettings.CopyPostOpFrom( fea_struct->GetStructSettingsPtr() );
            }

            int maxe = eoffset + mesh->m_NumEls + feacount.m_NumBeams;
            // noffset = magroundup( maxe );
            noffset = ceil2scale( maxe, 1000 );
        }
    }
    connoffset = noffset;

    if ( m_AssemblySettings.GetExportFileFlag( vsp::FEA_CALCULIX_FILE_NAME ) )
    {
        WriteAssemblyCalculix( assembly_id, feacount );
    }

    if ( m_AssemblySettings.GetExportFileFlag( vsp::FEA_NASTRAN_FILE_NAME ) )
    {
        WriteAssemblyNASTRAN( assembly_id, feacount, connoffset );
    }

    if ( m_AssemblySettings.GetExportFileFlag( vsp::FEA_STL_FILE_NAME ) )
    {
        WriteAssemblySTL( assembly_id );
    }

    if ( m_AssemblySettings.GetExportFileFlag( vsp::FEA_GMSH_FILE_NAME ) )
    {
        WriteAssemblyGmsh( assembly_id );
    }

    if ( m_AssemblySettings.GetExportFileFlag( vsp::FEA_MASS_FILE_NAME ) )
    {
        double assy_mass = ComputeWriteAssemblyMass( assembly_id );
        string mass_output = "Total Assembly Mass = " + std::to_string( assy_mass ) + "\n";
        FeaMeshMgr.addOutputText( mass_output );
    }
}

void FeaMeshMgrSingleton::WriteAssemblyCalculix( const string &assembly_id, const FeaCount &feacount )
{
    string fn = m_AssemblySettings.GetExportFileName( vsp::FEA_CALCULIX_FILE_NAME );
    FILE* fp = fopen( fn.c_str(), "w" );

    if ( fp )
    {
        WriteAssemblyCalculix( fp, assembly_id, feacount );
        fclose( fp );
    }
}

void FeaMeshMgrSingleton::WriteAssemblyCalculix( FILE* fp, const string &assembly_id, const FeaCount &feacount  )
{
    FeaAssembly* fea_assembly = StructureMgr.GetFeaAssembly( assembly_id );

    if ( !fea_assembly )
    {
        return;
    }

    FeaMeshMgr.ResetPropMatUse();

    vector < string > & idvec = fea_assembly->m_StructIDVec;

    if ( fp )
    {
        fprintf( fp, "** Calculix assembly data file generated from %s\n", VSPVERSION4 );
        fprintf( fp, "\n" );
        fprintf( fp, "** Num_Structures:  %lu\n", idvec.size() );
        fprintf( fp, "** Num_Nodes:       %llu\n", feacount.m_NumNodes );
        fprintf( fp, "** Num_Els:         %llu\n", feacount.m_NumEls );
        fprintf( fp, "** Num_Tris:        %llu\n", feacount.m_NumTris );
        fprintf( fp, "** Num_Quads:       %llu\n", feacount.m_NumQuads );
        fprintf( fp, "** Num_Beams:       %llu\n", feacount.m_NumBeams );
        fprintf( fp, "** Num_Connections: %lu\n", fea_assembly->m_ConnectionVec.size() );
        fprintf( fp, "\n" );

        for ( int i = 0; i < idvec.size(); i++ )
        {
            FeaMesh* mesh = GetMeshPtr( idvec[i] );
            if ( mesh )
            {
                mesh->WriteCalculixHeader( fp );
            }
        }

        for ( int i = 0; i < idvec.size(); i++ )
        {
            FeaMesh* mesh = GetMeshPtr( idvec[i] );
            if ( mesh )
            {
                mesh->WriteCalculixNodes( fp );
            }
        }

        for ( int i = 0; i < idvec.size(); i++ )
        {
            FeaMesh* mesh = GetMeshPtr( idvec[i] );
            if ( mesh )
            {
                mesh->WriteCalculixElements( fp );
            }
        }

        for ( int i = 0; i < idvec.size(); i++ )
        {
            FeaMesh* mesh = GetMeshPtr( idvec[i] );
            if ( mesh )
            {
                mesh->WriteCalculixBCs( fp );
            }
        }

        for ( int i = 0; i < fea_assembly->m_ConnectionVec.size(); i++ )
        {
            FeaConnection* conn = fea_assembly->m_ConnectionVec[i];
            if ( conn )
            {
                WriteConnectionCalculix( fp, conn );
            }
        }

        for ( int i = 0; i < idvec.size(); i++ )
        {
            FeaMesh* mesh = GetMeshPtr( idvec[i] );
            if ( mesh )
            {
                mesh->WriteCalculixProperties( fp );
            }
        }

        WriteCalculixMaterials( fp );
    }
}

void FeaMeshMgrSingleton::WriteAssemblyGmsh( const string &assembly_id )
{
    string fn = m_AssemblySettings.GetExportFileName( vsp::FEA_GMSH_FILE_NAME );
    FILE* fp = fopen( fn.c_str(), "w" );

    if ( fp )
    {
        WriteAssemblyGmsh( fp, assembly_id );
        fclose( fp );
    }
}

void FeaMeshMgrSingleton::WriteAssemblyGmsh( FILE* fp, const string &assembly_id )
{
    FeaAssembly* fea_assembly = StructureMgr.GetFeaAssembly( assembly_id );

    if ( !fea_assembly )
    {
        return;
    }

    vector < string > & idvec = fea_assembly->m_StructIDVec;

    if ( fp )
    {
        FeaMesh::WriteGmshHeader( fp );

        fprintf( fp, "$PhysicalNames\n" );
        for ( int i = 0; i < idvec.size(); i++ )
        {
            FeaMesh* mesh = GetMeshPtr( idvec[i] );
            if ( mesh )
            {
                mesh->WriteGmshNames( fp );
            }
        }
        fprintf( fp, "$EndPhysicalNames\n" );


        // Count FeaNodes
        int node_count = 0;
        for ( int i = 0; i < idvec.size(); i++ )
        {
            FeaMesh* mesh = GetMeshPtr( idvec[i] );
            if ( mesh )
            {
                for ( unsigned int j = 0; j < (int)mesh->m_FeaNodeVec.size(); j++ )
                {
                    if ( mesh->m_FeaNodeVecUsed[ j ] )
                    {
                        node_count++;
                    }
                }
            }
        }

        //==== Write Nodes ====//
        fprintf( fp, "$Nodes\n" );
        fprintf( fp, "%u\n", node_count );
        for ( int i = 0; i < idvec.size(); i++ )
        {
            FeaMesh* mesh = GetMeshPtr( idvec[i] );
            if ( mesh )
            {
                mesh->WriteGmshNodes( fp );
            }
        }
        fprintf( fp, "$EndNodes\n" );


        //==== Write FeaElements ====//
        fprintf( fp, "$Elements\n" );

        int ele_cnt = 0;
        for ( int i = 0; i < idvec.size(); i++ )
        {
            FeaMesh* mesh = GetMeshPtr( idvec[i] );
            if ( mesh )
            {
                ele_cnt += mesh->m_FeaElementVec.size();
            }
        }
        fprintf( fp, "%d\n", ele_cnt );

        ele_cnt = 1;
        for ( int i = 0; i < idvec.size(); i++ )
        {
            FeaMesh* mesh = GetMeshPtr( idvec[i] );
            if ( mesh )
            {
                mesh->WriteGmshElements( fp, ele_cnt );
            }
        }
        fprintf( fp, "$EndElements\n" );
    }
}

void FeaMeshMgrSingleton::WriteAssemblySTL( const string &assembly_id )
{
    string fn = m_AssemblySettings.GetExportFileName( vsp::FEA_STL_FILE_NAME );
    FILE* fp = fopen( fn.c_str(), "w" );

    if ( fp )
    {
        WriteAssemblySTL( fp, assembly_id );
        fclose( fp );
    }
}

void FeaMeshMgrSingleton::WriteAssemblySTL( FILE* fp, const string &assembly_id )
{
    FeaAssembly* fea_assembly = StructureMgr.GetFeaAssembly( assembly_id );

    if ( !fea_assembly )
    {
        return;
    }

    vector < string > & idvec = fea_assembly->m_StructIDVec;

    if ( fp )
    {
        for ( int i = 0; i < idvec.size(); i++ )
        {
            FeaMesh* mesh = GetMeshPtr( idvec[i] );
            if ( mesh )
            {
                mesh->WriteSTL( fp );
            }
        }
    }
}

double FeaMeshMgrSingleton::ComputeWriteAssemblyMass( const string &assembly_id )
{
    double assy_mass = 0;

    string fn = m_AssemblySettings.GetExportFileName( vsp::FEA_MASS_FILE_NAME );
    FILE* fp = fopen( fn.c_str(), "w" );

    if ( fp )
    {
        assy_mass = ComputeWriteAssemblyMass( fp, assembly_id );
        fclose( fp );
    }
    return assy_mass;
}

double FeaMeshMgrSingleton::ComputeWriteAssemblyMass( FILE* fp, const string &assembly_id )
{
    double assy_mass = 0;
    FeaAssembly* fea_assembly = StructureMgr.GetFeaAssembly( assembly_id );

    if ( !fea_assembly )
    {
        return assy_mass;
    }

    vector < string > & idvec = fea_assembly->m_StructIDVec;

    if ( fp )
    {
        for ( int i = 0; i < idvec.size(); i++ )
        {
            FeaMesh* mesh = GetMeshPtr( idvec[i] );
            if ( mesh )
            {
                mesh->ComputeWriteMass( fp );
                assy_mass += mesh->m_TotalMass;
            }
        }

        fprintf( fp, "\n" );
        fprintf( fp, "Assembly_Name       Total_Mass\n" );
        fprintf( fp, "%-20s% -9.4f\n", fea_assembly->GetName().c_str(), assy_mass );

    }

    return assy_mass;
}

void FeaMeshMgrSingleton::DetermineConnectionNodes( FeaConnection* conn, int &startnod, int &endnod )
{
    startnod = - 1;
    endnod = -1;

    if ( conn )
    {

        FeaMesh *startmesh = GetMeshPtr( conn->m_StartStructID );
        if ( startmesh )
        {
            FixPoint *fxpt = startmesh->GetFixPointByID( conn->m_StartFixPtID );
            unsigned long long int noffset = startmesh->m_StructSettings.m_NodeOffset;

            if ( fxpt )
            {
                int npt = fxpt->m_NodeIndex.size();
                int indx = conn->m_StartFixPtSurfIndex();

                if ( indx >=0 && indx < npt )
                {
                    startnod = fxpt->m_NodeIndex[ indx ] + noffset;
                }
            }
        }

        FeaMesh *endmesh = GetMeshPtr( conn->m_EndStructID );
        if ( endmesh )
        {
            FixPoint *fxpt = endmesh->GetFixPointByID( conn->m_EndFixPtID );
            unsigned long long int noffset = endmesh->m_StructSettings.m_NodeOffset;

            if ( fxpt )
            {
                int npt = fxpt->m_NodeIndex.size();
                int indx = conn->m_EndFixPtSurfIndex();

                if ( indx >=0 && indx < npt )
                {
                    endnod = fxpt->m_NodeIndex[ indx ] + noffset;
                }
            }
        }
    }
}

// Connection assumed between 0th entry of fxpt.  Needs specification in situations of part arrays and symmetry.
void FeaMeshMgrSingleton::WriteConnectionCalculix( FILE* fp, FeaConnection* conn )
{
    if ( fp && conn )
    {
        int startnod, endnod;
        DetermineConnectionNodes( conn, startnod, endnod );

        if ( startnod >= 0 && endnod >= 0 )
        {
            fprintf( fp, "** CONNECTION %s\n", conn->MakeName().c_str() );

            BitMask dof = conn->GetAsBitMask();

            for ( int i = 1; i <= 6; i++ )
            {
                if ( dof.CheckBit( i - 1) )
                {
                    fprintf( fp, "*EQUATION\n" );
                    fprintf( fp, "2\n" );                // Two terms in equation.
                    fprintf( fp, "%d,%d,%f,%d,%d,%f\n", endnod, i, 1.0, startnod, i, -1.0 ); // LHS of equation = 0
                }
            }
            fprintf( fp, "\n" );
        }
    }
}

void FeaMeshMgrSingleton::WriteCalculixMaterials( FILE* fp )
{
    if ( fp )
    {
        //==== Materials ====//
        fprintf( fp, "\n" );
        fprintf( fp, "** Materials\n" );
        for ( unsigned int i = 0; i < m_SimpleMaterialVec.size(); i++ )
        {
            m_SimpleMaterialVec[i].WriteCalculix( fp, i );
        }
    }
}

void FeaMeshMgrSingleton::WriteAssemblyNASTRAN( const string &assembly_id, const FeaCount &feacount, long long int connoffset )
{
    string dat_fn = m_AssemblySettings.GetExportFileName( vsp::FEA_NASTRAN_FILE_NAME );

    string bdf_fn = dat_fn;
    int pos = bdf_fn.find( ".dat" );
    if ( pos >= 0 )
    {
        bdf_fn.erase( pos, bdf_fn.length() - 1 );
    }
    bdf_fn.append( ".bdf" );

    FILE* dat_fp = fopen( dat_fn.c_str(), "w" );

    // Create temporary file to store NASTRAN bulk data. Case control information (SETs) will be
    //  defined in the *_NASTRAN.dat file prior to the bulk data (elements, gridpoints, etc.)
    FILE* bdf_fp = std::tmpfile();

    FILE* bdf_header_fp = fopen( bdf_fn.c_str(), "w" );

    if ( dat_fp && bdf_header_fp && bdf_fp )
    {
        FILE* nkey_fp = NULL;
        if ( m_AssemblySettings.GetExportFileFlag( vsp::FEA_NKEY_FILE_NAME ) )
        {
            string nkey_fname = m_AssemblySettings.GetExportFileName( vsp::FEA_NKEY_FILE_NAME );
            nkey_fp = fopen( nkey_fname.c_str(), "w" ); // Open *_NASTRAN.nkey
            if ( nkey_fp )
            {
                fprintf( nkey_fp, "$ NASTRAN Tag Key File Generated from %s\n", VSPVERSION4 );
                fprintf( nkey_fp, "%s\n\n", nkey_fname.c_str() );
            }
        }

        WriteAssemblyNASTRAN( dat_fp, bdf_header_fp, bdf_fp, nkey_fp, assembly_id, feacount, connoffset );

        CloseNASTRAN( dat_fp, bdf_header_fp, bdf_fp, nkey_fp );
    }
}

void FeaMeshMgrSingleton::WriteAssemblyNASTRAN( FILE *dat_fp, FILE *bdf_header_fp, FILE *bdf_fp, FILE *nkey_fp, const string &assembly_id, const FeaCount &feacount, long long int connoffset )
{
    FeaAssembly* fea_assembly = StructureMgr.GetFeaAssembly( assembly_id );

    if ( !fea_assembly )
    {
        return;
    }

    FeaMeshMgr.ResetPropMatUse();

    vector < string > & idvec = fea_assembly->m_StructIDVec;

    if ( dat_fp )
    {
        fprintf( dat_fp, "$ NASTRAN assembly data file generated from %s\n", VSPVERSION4 );
        fprintf( dat_fp, "\n" );
        fprintf( dat_fp, "$ Num_Structures:     %lu\n", idvec.size() );
        fprintf( dat_fp, "$ Num_Nodes:          %llu\n", feacount.m_NumNodes );
        fprintf( dat_fp, "$ Num_Els:            %llu\n", feacount.m_NumEls );
        fprintf( dat_fp, "$ Num_Tris:           %llu\n", feacount.m_NumTris );
        fprintf( dat_fp, "$ Num_Quads:          %llu\n", feacount.m_NumQuads );
        fprintf( dat_fp, "$ Num_Beams:          %llu\n", feacount.m_NumBeams );
        fprintf( dat_fp, "$ Num_Connections:    %lu\n", fea_assembly->m_ConnectionVec.size() );
        fprintf( dat_fp, "$ Connection_Offset:  %llu\n", connoffset );
        fprintf( dat_fp, "\n" );

        for ( int i = 0; i < idvec.size(); i++ )
        {
            FeaMesh* mesh = GetMeshPtr( idvec[i] );
            if ( mesh )
            {
                mesh->WriteNASTRANHeader( dat_fp );
            }
        }

        // Write bulk data to temp file
        fprintf(bdf_header_fp, "$EXECUTIVE CONTROL DECK\n");
        fprintf(bdf_header_fp, "ID TEMP\n");
        fprintf(bdf_header_fp, "SOL 1\n");
        fprintf(bdf_header_fp, "CEND\n");
        fprintf(bdf_header_fp, "$CASE CONTROL DECK\n");
        fprintf(bdf_header_fp, "DISPLACEMENT(PRINT,PLOT) = ALL\n");
        fprintf(bdf_header_fp, "ECHO = UNSORT\n");
        fprintf(bdf_header_fp, "ELDATA(5,PRINT) = ALL\n");
        fprintf(bdf_header_fp, "FORCE(PRINT,PLOT) = ALL\n");
        fprintf(bdf_header_fp, "GPFORCE = ALL\n");
        fprintf(bdf_header_fp, "MPCFORCES(PRINT,PLOT) = ALL\n");
        fprintf(bdf_header_fp, "OLOAD(PRINT,PLOT) = ALL\n");
        fprintf(bdf_header_fp, "SPC = 1\n");
        fprintf(bdf_header_fp, "SPCFORCES(PRINT,PLOT) = ALL\n");
        fprintf(bdf_header_fp, "STRESS(PRINT,PLOT) = ALL\n");
        fprintf(bdf_header_fp, "STRAIN(PRINT,PLOT) = ALL\n");
        fprintf(bdf_header_fp, "SUBTITLE = TEMP\n");
        fprintf(bdf_header_fp, "TITLE = TEMP LOAD CASES\n");
        fprintf(bdf_header_fp, "SUBCASE 1\n");
        fprintf(bdf_header_fp, "    LABEL = TEMPSUB\n");
        fprintf(bdf_header_fp, "    LOAD = 1\n");
        fprintf(bdf_header_fp, "BEGIN BULK\n");

        int set_cnt = 1;

        for ( int i = 0; i < idvec.size(); i++ )
        {
            FeaMesh* mesh = GetMeshPtr( idvec[i] );
            if ( mesh )
            {
                mesh->WriteNASTRANSPC1( bdf_fp );
            }
        }


        for ( int i = 0; i < idvec.size(); i++ )
        {
            FeaMesh* mesh = GetMeshPtr( idvec[i] );
            if ( mesh )
            {
                mesh->WriteNASTRANNodes( dat_fp, bdf_fp, nkey_fp, set_cnt );
            }
        }

        for ( int i = 0; i < idvec.size(); i++ )
        {
            FeaMesh* mesh = GetMeshPtr( idvec[i] );
            if ( mesh )
            {
                mesh->WriteNASTRANElements( dat_fp, bdf_fp, nkey_fp, set_cnt );
            }
        }

        int connid = connoffset + 1;
        for ( int i = 0; i < fea_assembly->m_ConnectionVec.size(); i++ )
        {
            FeaConnection* conn = fea_assembly->m_ConnectionVec[i];
            if ( conn )
            {
                WriteConnectionNASTRAN( bdf_fp, conn, connid );
            }
        }

        WriteNASTRANProperties( bdf_header_fp );

        WriteNASTRANMaterials( bdf_header_fp );

        fprintf( bdf_fp, "\nENDDATA\n" );
    }
}

void FeaMeshMgrSingleton::WriteConnectionNASTRAN( FILE* bdf_fp, FeaConnection* conn, int &connid )
{
    if ( bdf_fp && conn )
    {
        int startnod, endnod;
        DetermineConnectionNodes( conn, startnod, endnod );

        if ( startnod >= 0 && endnod >= 0 )
        {
            BitMask dof = conn->GetAsBitMask();
            string bcstr = dof.AsNASTRAN();

            fprintf( bdf_fp, "$ Connection %s\n", conn->MakeName().c_str() );
            fprintf( bdf_fp, "RBE2    ,%8d,%8d,%s,%8d\n", connid, startnod, bcstr.c_str(), endnod);
            fprintf( bdf_fp, "\n" );
            connid++;
        }
    }
}

void FeaMeshMgrSingleton::WriteNASTRANProperties( FILE* bdf_fp )
{
    if ( bdf_fp )
    {
        //==== Properties ====//
        fprintf( bdf_fp, "\n" );
        fprintf( bdf_fp, "$Properties\n" );

        for ( unsigned int i = 0; i < FeaMeshMgr.GetSimplePropertyVec().size(); i++ )
        {
            FeaMeshMgr.GetSimplePropertyVec()[i].WriteNASTRAN( bdf_fp, i + 1 );
        }
    }
}

void FeaMeshMgrSingleton::WriteNASTRANMaterials( FILE* bdf_fp )
{
    if ( bdf_fp )
    {
        //==== Materials ====//
        fprintf( bdf_fp, "\n" );
        fprintf( bdf_fp, "$Materials\n" );

        for ( unsigned int i = 0; i < FeaMeshMgr.GetSimpleMaterialVec().size(); i++ )
        {
            FeaMeshMgr.GetSimpleMaterialVec()[i].WriteNASTRAN( bdf_fp, i + 1 );
        }
    }
}

void FeaMeshMgrSingleton::ModifyConnDO( FeaConnection* conn, vector < DrawObj* > connDO )
{
    if ( conn && connDO.size() == 2 && connDO[0]->m_PntVec.size() == 2 && connDO[1]->m_PntVec.size() == 2 )
    {
        FeaMesh *startmesh = GetMeshPtr( conn->m_StartStructID );

        if ( startmesh )
        {
            FixPoint *fxpt = startmesh->GetFixPointByID( conn->m_StartFixPtID );

            if ( fxpt )
            {
                int npt = fxpt->m_Pnt.size();
                int indx = conn->m_StartFixPtSurfIndex();

                if ( indx >= 0 && indx < npt && npt > 0 )
                {
                    connDO[ 0 ]->m_PntVec[ 0 ] = fxpt->m_Pnt[ indx ];
                    connDO[ 1 ]->m_PntVec[ 0 ] = fxpt->m_Pnt[ indx ];
                    connDO[ 0 ]->m_GeomChanged = true;
                    connDO[ 1 ]->m_GeomChanged = true;
                }
            }
        }

        FeaMesh *endmesh = GetMeshPtr( conn->m_EndStructID );

        if ( endmesh )
        {
            FixPoint *fxpt = endmesh->GetFixPointByID( conn->m_EndFixPtID );

            if ( fxpt )
            {
                int npt = fxpt->m_Pnt.size();
                int indx = conn->m_EndFixPtSurfIndex();

                if ( indx >= 0 && indx < npt )
                {
                    connDO[0]->m_PntVec[1] = fxpt->m_Pnt[ indx ];
                    connDO[1]->m_PntVec[1] = fxpt->m_Pnt[ indx ];
                    connDO[ 0 ]->m_GeomChanged = true;
                    connDO[ 1 ]->m_GeomChanged = true;
                }
            }
        }
    }
}
