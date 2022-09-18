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

//=============================================================//
//=============================================================//

FeaMeshMgrSingleton::FeaMeshMgrSingleton() : CfdMeshMgrSingleton()
{
    m_FeaMeshInProgress = false;
    m_CADOnlyFlag = false;
    m_FeaStructID = string();
    m_MessageName = "FEAMessage";

    GetMeshPtr()->m_FeaGridDensityPtr = GetGridDensityPtr();
    GetMeshPtr()->m_StructSettingsPtr = GetStructSettingsPtr();
}

FeaMeshMgrSingleton::~FeaMeshMgrSingleton()
{
    CleanUp();
}

void FeaMeshMgrSingleton::CleanUp()
{
    GetMeshPtr()->Cleanup();

    CfdMeshMgrSingleton::CleanUp();
}

bool FeaMeshMgrSingleton::LoadSurfaces()
{
    CleanUp();

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

    if ( fea_struct )
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
            LoadSurfs( skinxfersurfs );

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
            }

        }
    }
}

void FeaMeshMgrSingleton::TransferMeshSettings()
{
    FeaStructure* fea_struct = StructureMgr.GetFeaStruct( m_FeaStructID );

    if ( fea_struct )
    {
        m_StructSettings = SimpleFeaMeshSettings();
        m_StructSettings.CopyFrom( fea_struct->GetStructSettingsPtr() );

        m_FeaGridDensity = SimpleFeaGridDensity();
        m_FeaGridDensity.CopyFrom( fea_struct->GetFeaGridDensityPtr() );

        if ( m_StructSettings.m_ConvertToQuadsFlag )
        {
            // Increase target edge length because tris are split into quads.
            // A tri with edge length 1.0 will result in an average quad edge of 0.349
            m_FeaGridDensity.ScaleMesh( 2.536 );
        }

        GetMeshPtr()->m_QuadMesh = m_StructSettings.m_ConvertToQuadsFlag;
        GetMeshPtr()->m_HighOrder = m_StructSettings.m_HighOrderElementFlag;

        GetMeshPtr()->m_NodeOffset = GetStructSettingsPtr()->m_NodeOffset;
        GetMeshPtr()->m_ElementOffset = GetStructSettingsPtr()->m_ElementOffset;
    }

}

void FeaMeshMgrSingleton::IdentifyCompIDNames()
{
    m_CompIDNameMap.clear();

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
    // Transfer FeaPart Data
    FeaStructure* fea_struct = StructureMgr.GetFeaStruct( m_FeaStructID );

    if ( fea_struct )
    {
        vector < FeaPart* > fea_part_vec = fea_struct->GetFeaPartVec();
        GetMeshPtr()->m_FeaPartNameVec.resize( GetMeshPtr()->m_NumFeaParts );
        GetMeshPtr()->m_FeaPartTypeVec.resize( GetMeshPtr()->m_NumFeaParts );
        GetMeshPtr()->m_FeaPartNumSurfVec.resize( GetMeshPtr()->m_NumFeaParts );
        GetMeshPtr()->m_FeaPartIncludedElementsVec.resize( GetMeshPtr()->m_NumFeaParts );
        GetMeshPtr()->m_FeaPartPropertyIndexVec.resize( GetMeshPtr()->m_NumFeaParts );
        GetMeshPtr()->m_FeaPartCapPropertyIndexVec.resize( GetMeshPtr()->m_NumFeaParts );

        for ( size_t i = 0; i < fea_part_vec.size(); i++ )
        {
            GetMeshPtr()->m_FeaPartNameVec[i] = fea_part_vec[i]->GetName();
            GetMeshPtr()->m_FeaPartTypeVec[i] = fea_part_vec[i]->GetType();
            GetMeshPtr()->m_FeaPartNumSurfVec[i] = fea_part_vec[i]->NumFeaPartSurfs();
            GetMeshPtr()->m_FeaPartIncludedElementsVec[i] = fea_part_vec[i]->m_IncludedElements.Get();
            GetMeshPtr()->m_FeaPartPropertyIndexVec[i] = fea_part_vec[i]->m_FeaPropertyIndex();
            GetMeshPtr()->m_FeaPartCapPropertyIndexVec[i] = fea_part_vec[i]->m_CapFeaPropertyIndex();
        }
    }

    // Transfer FeaProperty Data
    vector < FeaProperty* > fea_prop_vec = StructureMgr.GetFeaPropertyVec();
    GetMeshPtr()->m_SimplePropertyVec.resize( fea_prop_vec.size() );

    for ( size_t i = 0; i < fea_prop_vec.size(); i++ )
    {
        GetMeshPtr()->m_SimplePropertyVec[i] = SimpleFeaProperty();
        GetMeshPtr()->m_SimplePropertyVec[i].CopyFrom( fea_prop_vec[i] );
    }

    // Transfer FeaMaterial Data
    vector < FeaMaterial* > fea_mat_vec = StructureMgr.GetFeaMaterialVec();
    GetMeshPtr()->m_SimpleMaterialVec.resize( fea_mat_vec.size() );

    for ( size_t i = 0; i < fea_mat_vec.size(); i++ )
    {
        GetMeshPtr()->m_SimpleMaterialVec[i] = SimpleFeaMaterial();
        GetMeshPtr()->m_SimpleMaterialVec[i].CopyFrom( fea_mat_vec[i] );
    }
}

void FeaMeshMgrSingleton::TransferSubSurfData()
{
    FeaStructure* fea_struct = StructureMgr.GetFeaStruct( m_FeaStructID );

    if ( fea_struct )
    {
        vector < SubSurface* > fea_ss_vec = fea_struct->GetFeaSubSurfVec();
        m_SimpleSubSurfaceVec.resize( fea_ss_vec.size() );

        for ( size_t i = 0; i < fea_ss_vec.size(); i++ )
        {
            m_SimpleSubSurfaceVec[i] = SimpleSubSurface();
            m_SimpleSubSurfaceVec[i].CopyFrom( fea_ss_vec[i] );
        }
    }

    // Identify number of FeaSubSurfaces
    GetMeshPtr()->m_NumFeaSubSurfs = m_SimpleSubSurfaceVec.size();
    // Duplicate subsurface data in mesh data structure so it will be available
    // after mesh generation is complete.
    GetMeshPtr()->m_SimpleSubSurfaceVec = m_SimpleSubSurfaceVec;
}

void FeaMeshMgrSingleton::GenerateFeaMesh()
{
    m_FeaMeshInProgress = true;

#ifdef DEBUG_TIME_OUTPUT
    addOutputText( "Init Timer\n" );
#endif

    addOutputText( "Transfer Mesh Settings\n" );
    TransferMeshSettings();

    addOutputText( "Load Surfaces\n" );
    LoadSurfaces();

    if ( m_SurfVec.size() == 0 )
    {
        addOutputText( "No Surfaces.  Done.\n" );
        m_FeaMeshInProgress = false;
        return;
    }

    if ( ( !GetStructSettingsPtr()->GetExportFileFlag( vsp::FEA_CALCULIX_FILE_NAME ) && !GetStructSettingsPtr()->GetExportFileFlag( vsp::FEA_GMSH_FILE_NAME ) &&
           !GetStructSettingsPtr()->GetExportFileFlag( vsp::FEA_NASTRAN_FILE_NAME ) && !GetStructSettingsPtr()->GetExportFileFlag( vsp::FEA_MASS_FILE_NAME ) &&
           !GetStructSettingsPtr()->GetExportFileFlag( vsp::FEA_STL_FILE_NAME ) ) )
    {
        m_CADOnlyFlag = true;
    }

    if ( !m_CADOnlyFlag )
    {
        // Hide all geoms after loading surfaces and settings
        m_Vehicle->HideAll();
    }

    GetMassUnit();

    addOutputText( "Transfer FEA Data\n" );
    TransferFeaData();

    addOutputText( "Transfer Subsurf Data\n" );
    TransferSubSurfData();
    TransferDrawObjData();

    addOutputText( "Merge Co-Planar Parts\n" );
    MergeCoplanarParts();

    addOutputText( "Add Structure Parts\n" );
    AddStructureSurfParts();

    addOutputText( "Clean Merge Surfs\n" );
    CleanMergeSurfs(); // Must be called before AddStructureFixPoints to prevent FEA Fix Point surface misidentification

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

    addOutputText( "Binary Adaptation Curve Approximation\n" );
    BinaryAdaptIntCurves();

    if ( m_CADOnlyFlag )
    {
        // No need to generate mesh
        addOutputText( "Exporting Files\n" );
        ExportFeaMesh();

        addOutputText( "Finished\n" );

        m_FeaMeshInProgress = false;
        m_CADOnlyFlag = false;
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

    if ( GetSettingsPtr()->m_ConvertToQuadsFlag )
    {
        addOutputText( "ConvertToQuads\n" );
        ConvertToQuads();
    }

    addOutputText( "ConnectBorderEdges\n" );
    ConnectBorderEdges( false );        // No Wakes
    ConnectBorderEdges( true );         // Only Wakes

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

    addOutputText( "Tag Fea Nodes\n" );
    TagFeaNodes();

    addOutputText( "Remove Subsurf FEA Tris\n" );
    RemoveSubSurfFeaTris();

    addOutputText( "Exporting Files\n" );
    ExportFeaMesh();

    UpdateDrawObjs();

    addOutputText( "Finished\n" );

    m_FeaMeshInProgress = false;
}

void FeaMeshMgrSingleton::ExportFeaMesh()
{
    if ( !m_CADOnlyFlag )
    {
        if ( GetStructSettingsPtr()->GetExportFileFlag( vsp::FEA_NASTRAN_FILE_NAME ) )
        {
            GetMeshPtr()->WriteNASTRAN( GetStructSettingsPtr()->GetExportFileName( vsp::FEA_NASTRAN_FILE_NAME ) );
        }

        if ( GetStructSettingsPtr()->GetExportFileFlag( vsp::FEA_CALCULIX_FILE_NAME ) )
        {
            GetMeshPtr()->WriteCalculix();
        }

        if ( GetStructSettingsPtr()->GetExportFileFlag( vsp::FEA_STL_FILE_NAME ) )
        {
            WriteSTL( GetStructSettingsPtr()->GetExportFileName( vsp::FEA_STL_FILE_NAME ) );
        }

        if ( GetStructSettingsPtr()->GetExportFileFlag( vsp::FEA_GMSH_FILE_NAME ) )
        {
            GetMeshPtr()->WriteGmsh();
        }

        if ( GetStructSettingsPtr()->GetExportFileFlag( vsp::FEA_MASS_FILE_NAME ) )
        {
            GetMeshPtr()->ComputeWriteMass();
            string mass_output = "Total Mass = " + std::to_string( GetMeshPtr()->m_TotalMass ) + "\n";
            addOutputText( mass_output );
        }
    }

    if ( GetStructSettingsPtr()->GetExportFileFlag( vsp::FEA_SRF_FILE_NAME ) )
    {
        WriteSurfsIntCurves( GetStructSettingsPtr()->GetExportFileName( vsp::FEA_SRF_FILE_NAME ) );
    }

    if ( GetStructSettingsPtr()->GetExportFileFlag( vsp::FEA_CURV_FILE_NAME ) )
    {
        WriteGridToolCurvFile( GetStructSettingsPtr()->GetExportFileName( vsp::FEA_CURV_FILE_NAME ),
                               GetStructSettingsPtr()->m_ExportRawFlag );
    }

    if ( GetStructSettingsPtr()->GetExportFileFlag( vsp::FEA_PLOT3D_FILE_NAME ) )
    {
        WritePlot3DFile( GetStructSettingsPtr()->GetExportFileName( vsp::FEA_PLOT3D_FILE_NAME ),
                         GetStructSettingsPtr()->m_ExportRawFlag );
    }

    if ( GetStructSettingsPtr()->GetExportFileFlag( vsp::FEA_IGES_FILE_NAME ) || GetStructSettingsPtr()->GetExportFileFlag( vsp::FEA_STEP_FILE_NAME ) )
    {
        BuildNURBSCurvesVec(); // Note: Must be called before BuildNURBSSurfMap

        BuildNURBSSurfMap();
    }

    if ( GetStructSettingsPtr()->GetExportFileFlag( vsp::FEA_IGES_FILE_NAME ) )
    {
        string delim = StringUtil::get_delim( GetStructSettingsPtr()->m_CADLabelDelim );

        WriteIGESFile( GetStructSettingsPtr()->GetExportFileName( vsp::FEA_IGES_FILE_NAME ), GetStructSettingsPtr()->m_CADLenUnit,
                       GetStructSettingsPtr()->m_CADLabelID, GetStructSettingsPtr()->m_CADLabelSurfNo, GetStructSettingsPtr()->m_CADLabelSplitNo,
                       GetStructSettingsPtr()->m_CADLabelName, delim );
    }

    if ( GetStructSettingsPtr()->GetExportFileFlag( vsp::FEA_STEP_FILE_NAME ) )
    {
        string delim = StringUtil::get_delim( GetStructSettingsPtr()->m_CADLabelDelim );

        WriteSTEPFile( GetStructSettingsPtr()->GetExportFileName( vsp::FEA_STEP_FILE_NAME ), GetStructSettingsPtr()->m_CADLenUnit,
                       GetStructSettingsPtr()->m_STEPTol, GetStructSettingsPtr()->m_STEPMergePoints,
                       GetStructSettingsPtr()->m_CADLabelID, GetStructSettingsPtr()->m_CADLabelSurfNo, GetStructSettingsPtr()->m_CADLabelSplitNo,
                       GetStructSettingsPtr()->m_CADLabelName, delim, GetStructSettingsPtr()->m_STEPRepresentation );
    }

}

void FeaMeshMgrSingleton::MergeCoplanarParts()
{
    FeaStructure* fea_struct = StructureMgr.GetFeaStruct( m_FeaStructID );

    vector < VspSurf > all_surf_vec;
    vector < vec3d > all_norm_vec;
    vector < int > all_feaprt_ind_vec;
    vector < int > feaprt_surf_ind_vec;

    if ( fea_struct )
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

    if ( fea_struct )
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
                LoadSurfs( partxfersurfs, start_surf_id );
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

    if ( fea_struct )
    {
        vector < FeaPart* > fea_part_vec = fea_struct->GetFeaPartVec();

        //===== Add FeaParts ====//
        for ( unsigned int i = 0; i < GetMeshPtr()->m_NumFeaParts; i++ ) // Do Not Assume Skin is Index 0
        {
            if ( fea_struct->FeaPartIsFixPoint( i ) )
            {
                //===== Add FixedPoint Data ====//
                FeaFixPoint* fixpnt = dynamic_cast<FeaFixPoint*>( fea_part_vec[i] );
                assert( fixpnt );
                FixPoint fxpt;

                int part_index = fea_struct->GetFeaPartIndex( fea_part_vec[i] );
                vector < vec3d > pnt_vec = fixpnt->GetPntVec();
                vec2d uw = fixpnt->GetUW();

                for ( size_t j = 0; j < pnt_vec.size(); j++ )
                {
                    // Identify the surface index and coordinate points for the fixed point
                    vector < int > surf_index;
                    for ( size_t k = 0; k < m_SurfVec.size(); k++ )
                    {
                        if ( m_SurfVec[k]->GetFeaPartIndex() == StructureMgr.GetFeaPartIndex( fixpnt->m_ParentFeaPartID ) &&
                             m_SurfVec[k]->GetFeaPartSurfNum() == j )
                        {
                            if ( m_SurfVec[k]->ValidUW( uw, 0.0 ) )
                            {
                                surf_index.push_back( k );
                            }
                        }
                    }

                    if ( surf_index.size() > 0 )
                    {
                        fxpt.m_Pnt.push_back( pnt_vec[j] );
                        fxpt.m_UW.push_back( uw );
                        fxpt.m_SurfInd.push_back( surf_index );
                        fxpt.m_FeaPartIndex.push_back( part_index );
                        fxpt.m_PtMassFlag.push_back( fixpnt->m_FixPointMassFlag.Get() );
                        fxpt.m_PtMass.push_back( fixpnt->m_FixPointMass.Get() );

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

                GetMeshPtr()->m_FixPntVec.push_back( fxpt );
            }
        }
    }
}

void FeaMeshMgrSingleton::AddStructureTrimPlanes()
{
    FeaStructure* fea_struct = StructureMgr.GetFeaStruct( m_FeaStructID );

    if ( fea_struct )
    {
        fea_struct->FetchAllTrimPlanes( m_TrimPt, m_TrimNorm );
    }
}

void FeaMeshMgrSingleton::BuildMeshOrientationLookup()
{

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

bool FeaMeshMgrSingleton::CullPtByTrimGroup( const vec3d &pt, const vector < vec3d > & pplane, const vector < vec3d > & nplane )
{
    double tol = 0.01 * GetGridDensityPtr()->m_MinLen;
    // Number of planes in this trim group.
    int numplane = pplane.size();
    for ( int iplane = 0; iplane < numplane; iplane++ )
    {
        vec3d u = pt - pplane[ iplane ];
        double dp = dot( u, nplane[ iplane ] );
        if ( dp < tol )
        {
            return false;
        }
    }
    return true;
}

void FeaMeshMgrSingleton::RemoveTrimTris()
{
    if ( m_TrimPt.size() > 0 ) // Skip if there are no trim groups.
    {
        for ( int s = 0; s < ( int ) m_SurfVec.size(); ++s ) // every surface
        {
            bool delSomeTris = false;

            list < Face * > faceList = m_SurfVec[ s ]->GetMesh()->GetFaceList();
            for ( list < Face * >::iterator t = faceList.begin(); t != faceList.end(); ++t ) // every triangle
            {
                vec3d cp = ( *t )->ComputeCenterPnt( m_SurfVec[ s ] );

                for ( int i = 0; i < m_TrimPt.size(); i++ )
                {
                    // This seems convoluted, but it needs to be cumulative.
                    if ( CullPtByTrimGroup( cp, m_TrimPt[ i ], m_TrimNorm[ i ] ) )
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
    bool highorder = GetSettingsPtr()->m_HighOrderElementFlag;

    //==== Collect All Nodes and Tris ====//
    vector < vec2d > all_uw_vec;
    vector < int > uw_surf_ind_vec; // Vector of surface index for each UW point
    vector < vec3d > all_pnt_vec;
    vector < SimpFace > all_face_vec;
    vector < int > tri_surf_ind_vec; // Vector of surface index for each SimpTri

    // Build FeaBeam Intersections
    list< ISegChain* >::iterator c;

    for ( c = m_ISegChainList.begin(); c != m_ISegChainList.end(); ++c )
    {
        if ( !( *c )->m_BorderFlag ) // Only include intersection curves
        {
            // Check at least one surface intersection cap flag is true
            int FeaPartCapA = GetMeshPtr()->m_FeaPartIncludedElementsVec[( *c )->m_SurfA->GetFeaPartIndex()];
            int FeaPartCapB = GetMeshPtr()->m_FeaPartIncludedElementsVec[( *c )->m_SurfB->GetFeaPartIndex()];

            vector < vec3d > ipntVec, inormVec;
            vector < vec2d > iuwVec;
            vector < int > ssindexVec;
            Surf* NormSurf = NULL;
            int FeaPartIndex = -1;

            // Check if one surface is a skin and one is an FeaPart (m_CompID = -9999)
            if ( ( ( FeaPartCapA == vsp::FEA_BEAM || FeaPartCapA == vsp::FEA_SHELL_AND_BEAM ) || ( FeaPartCapB == vsp::FEA_BEAM || FeaPartCapB == vsp::FEA_SHELL_AND_BEAM ) ) &&
                ( ( ( *c )->m_SurfA->GetCompID() < 0 && ( *c )->m_SurfB->GetCompID() >= 0 ) || ( ( *c )->m_SurfB->GetCompID() < 0 && ( *c )->m_SurfA->GetCompID() >= 0 ) ) )
            {
                vec3d center;

                if ( ( *c )->m_SurfA->GetCompID() < 0 && ( FeaPartCapA == vsp::FEA_BEAM || FeaPartCapA == vsp::FEA_SHELL_AND_BEAM ) )
                {
                    FeaPartIndex = ( *c )->m_SurfA->GetFeaPartIndex();
                    center = ( *c )->m_SurfA->GetBBox().GetCenter();
                }
                else if ( ( *c )->m_SurfB->GetCompID() < 0 && ( FeaPartCapB == vsp::FEA_BEAM || FeaPartCapB == vsp::FEA_SHELL_AND_BEAM ) )
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
                for ( int j = 0; j < (int)( *c )->m_TessVec.size(); j++ )
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
                    for ( int j = 0; j < (int)( *c )->m_TessVec.size(); j++ )
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

            int normsurfindx = vector_find_val( m_SurfVec, NormSurf );
            // Define FeaBeam elements
            for ( int j = 1; j < (int)ipntVec.size(); j++ )
            {
                FeaBeam* beam = new FeaBeam;

                vec3d start_pnt = ipntVec[j - 1];
                vec3d end_pnt = ipntVec[j];

                // Check for collapsed beam elements (caused by bug in Intersect where invalid intersection points are added to m_BinMap)
                if ( dist( start_pnt, end_pnt ) < FLT_EPSILON )
                {
                    printf( "Warning: Collapsed Beam Element Skipped\n" );
                    break;
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

                beam->Create( start_pnt, end_pnt, inormVec[j - 1] );
                beam->SetFeaPartIndex( FeaPartIndex );
                beam->SetFeaSSIndex( ssindexVec[j] );
                beam->SetFeaPartSurfNum( m_SurfVec[normsurfindx]->GetFeaPartSurfNum() );
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

    for ( size_t i = 0; i < index_vec.size(); i++ )
    {
        for ( size_t j = 0; j < all_face_vec.size(); j++ )
        {
            if ( all_face_vec[j].ind0 == i && all_face_vec[j].ind0 != index_vec[i] )
            {
                all_face_vec[j].ind0 = index_vec[i];
            }

            if ( all_face_vec[j].ind1 == i && all_face_vec[j].ind1 != index_vec[i] )
            {
                all_face_vec[j].ind1 = index_vec[i];
            }

            if ( all_face_vec[j].ind2 == i && all_face_vec[j].ind2 != index_vec[i] )
            {
                all_face_vec[j].ind2 = index_vec[i];
            }

            if ( all_face_vec[j].m_isQuad )
            {
                if ( all_face_vec[j].ind3 == i && all_face_vec[j].ind3 != index_vec[i] )
                {
                    all_face_vec[j].ind3 = index_vec[i];
                }
            }
        }
    }

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

        vec2d closest_uw = m_SurfVec[tri_surf_ind_vec[i]]->ClosestUW( avg_pnt, uw_guess[0], uw_guess[1] );

        // Determine tangent u-direction for orientation vector at tri midpoint
        vec3d orient_vec = m_SurfVec[tri_surf_ind_vec[i]]->GetFeaElementOrientation( closest_uw[0], closest_uw[1] );

        // Check for subsurface:
        if ( all_face_vec[i].m_Tags.size() == 2 )
        {
            // First index of m_Tags is the parent surface. Second index is subsurface index which begins 
            //  from the last FeaPart surface index (FeaFixPoints are not tagged; they are not surfaces)
            int ssindex = all_face_vec[i].m_Tags[1] - ( GetMeshPtr()->m_NumFeaParts - GetMeshPtr()->m_NumFeaFixPoints ) - 1;
            elem->SetFeaSSIndex( ssindex );

            int type = m_SimpleSubSurfaceVec[ssindex].GetFeaOrientationType();
            vector < vec3d > ovec = m_SimpleSubSurfaceVec[ssindex].GetFeaOrientationVec();
            int surf_num = m_SurfVec[tri_surf_ind_vec[i]]->GetFeaPartSurfNum();
            vec3d defaultorientation = ovec[ surf_num ];
            orient_vec = m_SurfVec[tri_surf_ind_vec[i]]->GetFeaElementOrientation( closest_uw[0], closest_uw[1], type, defaultorientation );
        }
        else if ( all_face_vec[i].m_Tags.size() > 2 )
        {
            //Give priority to first tagged subsurface in the event of overlap
            int ssindex = all_face_vec[i].m_Tags[1] - ( GetMeshPtr()->m_NumFeaParts - GetMeshPtr()->m_NumFeaFixPoints ) - 1;
            elem->SetFeaSSIndex( ssindex );

            int type = m_SimpleSubSurfaceVec[ssindex].GetFeaOrientationType();
            vector < vec3d > ovec = m_SimpleSubSurfaceVec[ssindex].GetFeaOrientationVec();
            int surf_num = m_SurfVec[tri_surf_ind_vec[i]]->GetFeaPartSurfNum();
            vec3d defaultorientation = ovec[ surf_num ];
            orient_vec = m_SurfVec[tri_surf_ind_vec[i]]->GetFeaElementOrientation( closest_uw[0], closest_uw[1], type, defaultorientation );
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

 
}

void FeaMeshMgrSingleton::SetFixPointSurfaceNodes()
{
    for ( size_t n = 0; n < GetMeshPtr()->m_NumFeaFixPoints; n++ )
    {
        FixPoint fxpt = GetMeshPtr()->m_FixPntVec[n];
        for ( size_t j = 0; j < fxpt.m_SurfInd.size(); j++ )
        {
            if ( fxpt.m_BorderFlag[j] == SURFACE_FIX_POINT && fxpt.m_SurfInd[j].size() == 1 )
            {
                for ( size_t k = 0; k < fxpt.m_SurfInd[j].size(); k++ )
                {
                    for ( size_t i = 0; i < m_SurfVec.size(); i++ )
                    {
                        if ( fxpt.m_SurfInd[j][k] == i )
                        {
                            string fix_point_name = GetMeshPtr()->m_FeaPartNameVec[fxpt.m_FeaPartIndex[j]];

                            if ( m_SurfVec[i]->GetMesh()->SetFixPoint( fxpt.m_Pnt[j], fxpt.m_UW[j] ) )
                            {
                                // No message on success.
                            }
                            else
                            {
                                string message = "Error: No node found for " + fix_point_name + ". Adjust GridDensity.\n";
                                addOutputText( message );
                            }
                            break;
                        }
                    }
                }
            }
        }
    }
}

void FeaMeshMgrSingleton::SetFixPointBorderNodes()
{
    for ( size_t n = 0; n < GetMeshPtr()->m_NumFeaFixPoints; n++ )
    {
        FixPoint fxpt = GetMeshPtr()->m_FixPntVec[n];
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

                        if ( ( *c )->m_SurfA->ValidUW( fxpt.m_UW[j] ) )
                        {
                            closest_uwA = ( *c )->m_SurfA->ClosestUW( fxpt.m_Pnt[j], fxpt.m_UW[j][0], fxpt.m_UW[j][1] );
                        }
                        else
                        {
                            closest_uwA = ( *c )->m_SurfA->ClosestUW( fxpt.m_Pnt[j] );
                        }

                        if ( ( *c )->m_SurfB->ValidUW( fxpt.m_UW[j] ) )
                        {
                            closest_uwB = ( *c )->m_SurfB->ClosestUW( fxpt.m_Pnt[j], fxpt.m_UW[j][0], fxpt.m_UW[j][1] );
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
                            string fix_point_name = GetMeshPtr()->m_FeaPartNameVec[fxpt.m_FeaPartIndex[j]];
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

void FeaMeshMgrSingleton::CheckFixPointIntersects()
{
    // Identify and set FeaFixPoints on intersection curves

    for ( size_t n = 0; n < GetMeshPtr()->m_NumFeaFixPoints; n++ )
    {
        FixPoint fxpt = GetMeshPtr()->m_FixPntVec[n];
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
                                vec2d closest_uwA = ( *c )->m_SurfA->ClosestUW( fxpt.m_Pnt[j], fxpt.m_UW[j][0], fxpt.m_UW[j][1] );
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

                                    if ( success )
                                    {
                                        fxpt.m_Pnt[j] = ( *c )->m_SurfA->CompPnt( closest_uwA.x(), closest_uwA.y() );
                                    }
                                }
                                if ( p1 && !success )
                                {
                                    success = ( *c )->AddBorderSplit( p1 );

                                    if ( success )
                                    {
                                        fxpt.m_Pnt[j] = closest_pnt;
                                    }
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
                                vec2d closest_uwB = ( *c )->m_SurfB->ClosestUW( fxpt.m_Pnt[j], fxpt.m_UW[j][0], fxpt.m_UW[j][1] );

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

                                    if ( success )
                                    {
                                        fxpt.m_Pnt[j] = closest_pnt;
                                    }
                                }
                                if ( p1 && !success )
                                {
                                    success = ( *c )->AddBorderSplit( p1 );

                                    if ( success )
                                    {
                                        fxpt.m_Pnt[j] = ( *c )->m_SurfB->CompPnt( closest_uwB[0], closest_uwB[1] );
                                    }
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
                            vec2d closest_uwA = ( *c )->m_SurfA->ClosestUW( fxpt.m_Pnt[j], fxpt.m_UW[j][0], fxpt.m_UW[j][1] );
                            vec2d closest_uwB = ( *c )->m_SurfB->ClosestUW( fxpt.m_Pnt[j], fxpt.m_UW[j][0], fxpt.m_UW[j][1] );

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
                        fxpt.m_BorderFlag[j] = INTERSECT_FIX_POINT;

                        string fix_point_name = GetMeshPtr()->m_FeaPartNameVec[fxpt.m_FeaPartIndex[j]];
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
                    if ( ss_vec[ss].m_IncludedElements == vsp::FEA_BEAM || ss_vec[ss].m_IncludedElements == vsp::FEA_SHELL_AND_BEAM ) // Only consider SubSurface if cap intersections is flagged
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
    for ( unsigned int i = 0; i < GetMeshPtr()->m_NumFeaSubSurfs; i++ )
    {
        for ( int j = 0; j < GetMeshPtr()->m_FeaElementVec.size(); j++ )
        {
            if ( GetMeshPtr()->m_FeaElementVec[j]->GetFeaSSIndex() == i &&
               ( GetMeshPtr()->m_FeaElementVec[j]->GetElementType() == FeaElement::FEA_TRI_3 ||
                 GetMeshPtr()->m_FeaElementVec[j]->GetElementType() == FeaElement::FEA_TRI_6 ||
                 GetMeshPtr()->m_FeaElementVec[j]->GetElementType() == FeaElement::FEA_QUAD_4 ||
                 GetMeshPtr()->m_FeaElementVec[j]->GetElementType() == FeaElement::FEA_QUAD_8 ) )
            {
                if ( m_SimpleSubSurfaceVec[i].m_IncludedElements == vsp::FEA_BEAM )
                {
                    delete GetMeshPtr()->m_FeaElementVec[j];
                    GetMeshPtr()->m_FeaElementVec.erase( GetMeshPtr()->m_FeaElementVec.begin() + j );
                    j--;
                }
            }
        }
    }
}

void FeaMeshMgrSingleton::TagFeaNodes()
{
    //==== Collect All FeaNodes ====//
    GetMeshPtr()->m_FeaNodeVec.clear();

    for ( int i = 0; i < (int)GetMeshPtr()->m_FeaElementVec.size(); i++ )
    {
        GetMeshPtr()->m_FeaElementVec[i]->LoadNodes( GetMeshPtr()->m_FeaNodeVec );
    }

    vector< vec3d* > m_AllPntVec;
    for ( int i = 0; i < (int)GetMeshPtr()->m_FeaNodeVec.size(); i++ )
    {
        m_AllPntVec.push_back( &(GetMeshPtr()->m_FeaNodeVec[i]->m_Pnt) );
    }

    //==== Build Node Map ====//
    GetMeshPtr()->m_IndMap.clear();
    GetMeshPtr()->m_PntShift.clear();
    BuildIndMap( m_AllPntVec, GetMeshPtr()->m_IndMap, GetMeshPtr()->m_PntShift );

    //==== Assign Index Numbers to Nodes ====//
    for ( int i = 0; i < (int)GetMeshPtr()->m_FeaNodeVec.size(); i++ )
    {
        GetMeshPtr()->m_FeaNodeVec[i]->m_Tags.clear();
        int ind = FindPntIndex( GetMeshPtr()->m_FeaNodeVec[i]->m_Pnt, m_AllPntVec, GetMeshPtr()->m_IndMap );
        GetMeshPtr()->m_FeaNodeVec[i]->m_Index = GetMeshPtr()->m_PntShift[ind] + 1;
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
            int ind = FindPntIndex( temp_nVec[j]->m_Pnt, m_AllPntVec, GetMeshPtr()->m_IndMap );
            GetMeshPtr()->m_FeaNodeVec[ind]->AddTag( i );
        }
    }

    // Tag FeaSubSurface Nodes with FeaSubSurface Index, beginning at the last FeaPart index (GetMeshPtr()->m_NumFeaParts)
    for ( unsigned int i = 0; i < GetMeshPtr()->m_NumFeaSubSurfs; i++ )
    {
        vector< FeaNode* > temp_nVec;

        for ( int j = 0; j < GetMeshPtr()->m_FeaElementVec.size(); j++ )
        {
            if ( GetMeshPtr()->m_FeaElementVec[j]->GetFeaSSIndex() == i && GetMeshPtr()->m_FeaElementVec[j]->GetFeaSSIndex() >= 0 && m_SimpleSubSurfaceVec[i].m_IncludedElements != vsp::FEA_BEAM )
            {
                GetMeshPtr()->m_FeaElementVec[j]->LoadNodes( temp_nVec );
            }
        }

        for ( int j = 0; j < (int)temp_nVec.size(); j++ )
        {
            int ind = FindPntIndex( temp_nVec[j]->m_Pnt, m_AllPntVec, GetMeshPtr()->m_IndMap );
            GetMeshPtr()->m_FeaNodeVec[ind]->AddTag( i + GetMeshPtr()->m_NumFeaParts );
        }
    }

    //==== Tag FeaFixPoints ====//
    for ( size_t j = 0; j < GetMeshPtr()->m_NumFeaFixPoints; j++ )
    {
        FixPoint fxpt = GetMeshPtr()->m_FixPntVec[j];
        for ( size_t k = 0; k < fxpt.m_Pnt.size(); k++ )
        {
            for ( int i = 0; i < (int)GetMeshPtr()->m_FeaNodeVec.size(); i++ )
            {
                // Compare the distance between node and fixed point, but only use nodes that have been tagged to an FeaPart
                if (( dist( GetMeshPtr()->m_FeaNodeVec[i]->m_Pnt, fxpt.m_Pnt[k] ) <= FLT_EPSILON ) && ( GetMeshPtr()->m_FeaNodeVec[i]->m_Tags.size() > 0 ) )
                {
                    GetMeshPtr()->m_FeaNodeVec[i]->AddTag( fxpt.m_FeaPartIndex[k] );
                    GetMeshPtr()->m_FeaNodeVec[i]->m_FixedPointFlag = true;

                    // Create mass element if mass flag is true
                    if ( fxpt.m_PtMassFlag[k] )
                    {
                        FeaPointMass* mass = new FeaPointMass;
                        mass->Create( GetMeshPtr()->m_FeaNodeVec[i]->m_Pnt, fxpt.m_PtMass[k] );
                        mass->SetFeaPartIndex( fxpt.m_FeaPartIndex[k] );

                        int ind = FindPntIndex( GetMeshPtr()->m_FeaNodeVec[i]->m_Pnt, m_AllPntVec, GetMeshPtr()->m_IndMap );
                        mass->m_Corners[0]->m_Index = GetMeshPtr()->m_PntShift[ind] + 1;

                        GetMeshPtr()->m_FeaElementVec.push_back( mass );
                    }
                    break;
                }
            }
        }
    }
}

void FeaMeshMgrSingleton::TransferDrawObjData()
{
    FeaStructure* fea_struct = StructureMgr.GetFeaStruct( m_FeaStructID );

    if ( fea_struct )
    {
        vector < FeaPart* > fea_part_vec = fea_struct->GetFeaPartVec();

        // FeaParts:
        for ( unsigned int i = 0; i < GetMeshPtr()->m_NumFeaParts; i++ )
        {
            string name = GetMeshPtr()->m_StructName + ":  " + GetMeshPtr()->m_FeaPartNameVec[i];

            if ( fea_part_vec[i]->m_IncludedElements() == vsp::FEA_SHELL || fea_part_vec[i]->m_IncludedElements() == vsp::FEA_SHELL_AND_BEAM )
            {
                GetMeshPtr()->m_DrawBrowserNameVec.push_back( name );
                GetMeshPtr()->m_DrawBrowserPartIndexVec.push_back( i );
            }

            GetMeshPtr()->m_FixPointFeaPartFlagVec.push_back( GetMeshPtr()->m_FeaPartTypeVec[i] == vsp::FEA_FIX_POINT );

            GetMeshPtr()->m_DrawElementFlagVec.push_back( true );

            if ( GetMeshPtr()->m_FeaPartIncludedElementsVec[i] == vsp::FEA_BEAM || GetMeshPtr()->m_FeaPartIncludedElementsVec[i] == vsp::FEA_SHELL_AND_BEAM )
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

            if ( m_SimpleSubSurfaceVec[i].m_TestType != vsp::NONE && ( m_SimpleSubSurfaceVec[i].m_IncludedElements == vsp::FEA_SHELL || m_SimpleSubSurfaceVec[i].m_IncludedElements == vsp::FEA_SHELL_AND_BEAM ) )
            {
                GetMeshPtr()->m_DrawBrowserNameVec.push_back( name );
                GetMeshPtr()->m_DrawBrowserPartIndexVec.push_back( GetMeshPtr()->m_NumFeaParts + i );
            }

            GetMeshPtr()->m_DrawElementFlagVec.push_back( true );

            if ( m_SimpleSubSurfaceVec[i].m_IncludedElements == vsp::FEA_BEAM || m_SimpleSubSurfaceVec[i].m_IncludedElements == vsp::FEA_SHELL_AND_BEAM )
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

    GetMeshPtr()->UpdateDrawObjs();
}

void FeaMeshMgrSingleton::LoadDrawObjs( vector< DrawObj* > & draw_obj_vec )
{
    if ( !GetGridDensityPtr() || !GetStructSettingsPtr() )
    {
        return;
    }

    if ( !GetFeaMeshInProgress() )
    {
        SurfaceIntersectionSingleton::LoadDrawObjs( draw_obj_vec );
        GetMeshPtr()->LoadDrawObjs( draw_obj_vec );
    }
}

void FeaMeshMgrSingleton::UpdateDisplaySettings()
{
    if ( GetStructSettingsPtr() && StructureMgr.GetFeaStruct( m_FeaStructID ) )
    {
        GetStructSettingsPtr()->m_DrawMeshFlag = StructureMgr.GetFeaStruct( m_FeaStructID )->GetStructSettingsPtr()->m_DrawMeshFlag.Get();
        GetStructSettingsPtr()->m_ColorTagsFlag = StructureMgr.GetFeaStruct( m_FeaStructID )->GetStructSettingsPtr()->m_ColorTagsFlag.Get();

        GetStructSettingsPtr()->m_DrawNodesFlag = StructureMgr.GetFeaStruct( m_FeaStructID )->GetStructSettingsPtr()->m_DrawNodesFlag.Get();
        GetStructSettingsPtr()->m_DrawElementOrientVecFlag = StructureMgr.GetFeaStruct( m_FeaStructID )->GetStructSettingsPtr()->m_DrawElementOrientVecFlag.Get();

        GetStructSettingsPtr()->m_DrawBorderFlag = StructureMgr.GetFeaStruct( m_FeaStructID )->GetStructSettingsPtr()->m_DrawBorderFlag.Get();
        GetStructSettingsPtr()->m_DrawIsectFlag = StructureMgr.GetFeaStruct( m_FeaStructID )->GetStructSettingsPtr()->m_DrawIsectFlag.Get();
        GetStructSettingsPtr()->m_DrawRawFlag = StructureMgr.GetFeaStruct( m_FeaStructID )->GetStructSettingsPtr()->m_DrawRawFlag.Get();
        GetStructSettingsPtr()->m_DrawBinAdaptFlag = StructureMgr.GetFeaStruct( m_FeaStructID )->GetStructSettingsPtr()->m_DrawBinAdaptFlag.Get();
        GetStructSettingsPtr()->m_DrawCurveFlag = StructureMgr.GetFeaStruct( m_FeaStructID )->GetStructSettingsPtr()->m_DrawCurveFlag.Get();
        GetStructSettingsPtr()->m_DrawPntsFlag = StructureMgr.GetFeaStruct( m_FeaStructID )->GetStructSettingsPtr()->m_DrawPntsFlag.Get();
    }
}

void FeaMeshMgrSingleton::RegisterAnalysis()
{
    string analysis_name = "FeaMeshAnalysis";

    if (!AnalysisMgr.FindAnalysis(analysis_name))
    {
        FeaMeshAnalysis* sia = new FeaMeshAnalysis();

        if ( sia && !AnalysisMgr.RegisterAnalysis( analysis_name, sia ) )
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
