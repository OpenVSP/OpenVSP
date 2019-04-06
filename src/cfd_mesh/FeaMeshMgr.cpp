//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//

// FeaMeshMgr.cpp
//
//////////////////////////////////////////////////////////////////////

#include "FeaMeshMgr.h"
#include "Util.h"
#include "APIDefines.h"
#include "SubSurfaceMgr.h"
#include "StructureMgr.h"
#include "PntNodeMerge.h"
#include "main.h"

//=============================================================//
//=============================================================//

FeaMeshMgrSingleton::FeaMeshMgrSingleton() : CfdMeshMgrSingleton()
{
    m_TotalMass = 0.0;
    m_FeaMeshInProgress = false;
    m_NumFeaParts = 0;
    m_NumFeaSubSurfs = 0;
    m_FeaMeshStructIndex = -1;
    m_RemoveSkinTris = false;
    m_NumFeaFixPoints = 0;
    m_NumTris = 0;
    m_NumBeams = 0;
    m_MessageName = "FEAMessage";
}

FeaMeshMgrSingleton::~FeaMeshMgrSingleton()
{
    CleanUp();
}

void FeaMeshMgrSingleton::CleanUp()
{
    //==== Delete Old Elements ====//
    for ( unsigned int i = 0; i < m_FeaElementVec.size(); i++ )
    {
        m_FeaElementVec[i]->DeleteAllNodes();
        delete m_FeaElementVec[i];
    }
    m_FeaElementVec.clear();
    m_FeaNodeVec.clear();

    for ( unsigned int i = 0; i < m_AllPntVec.size(); i++ )
    {
        delete m_AllPntVec[i];
    }
    m_AllPntVec.clear();

    m_IndMap.clear();
    m_PntShift.clear();

    m_TotalMass = 0.0;
    m_NumFeaParts = 0;
    m_NumFeaSubSurfs = 0;
    m_NumFeaFixPoints = 0;
    m_NumTris = 0;
    m_NumBeams = 0;

    m_FeaPartNameVec.clear();
    m_FeaPartTypeVec.clear();
    m_FeaPartIncludedElementsVec.clear();
    m_FeaPartPropertyIndexVec.clear();
    m_FeaPartCapPropertyIndexVec.clear();

    m_SimplePropertyVec.clear();
    m_SimpleMaterialVec.clear();

    m_FixPntMap.clear();
    m_FixUWMap.clear();
    m_FixPntFeaPartIndexMap.clear();
    m_FixPntSurfIndMap.clear();
    m_FixPntBorderFlagMap.clear();
    m_FixPointMassFlagMap.clear();
    m_FixPointMassMap.clear();

    m_DrawBrowserNameVec.clear();
    m_DrawBrowserPartIndexVec.clear();
    m_DrawElementFlagVec.clear();
    m_FixPointFeaPartFlagVec.clear();
    m_DrawCapFlagVec.clear();

    CfdMeshMgrSingleton::CleanUp();
}

bool FeaMeshMgrSingleton::LoadSurfaces()
{
    CleanUp();

    // Identify the structure to mesh (m_FeaMeshStructIndex must be set) 
    FeaStructure* fea_struct = StructureMgr.GetFeaStruct( m_FeaMeshStructIndex );

    if ( !fea_struct )
    {
        addOutputText( "FeaMesh Failed: Invalid FeaStructure Selection\n" );
        m_FeaMeshInProgress = false;
        return false;
    }

    // Save structure name
    m_StructName = fea_struct->GetName();

    // Identify number of FeaParts
    m_NumFeaParts = fea_struct->NumFeaParts();

    // Identify number of FeaFixPoints
    m_NumFeaFixPoints = fea_struct->GetNumFeaFixPoints();

    LoadSkins();

    return true;
}

void FeaMeshMgrSingleton::LoadSkins()
{
    FeaStructure* fea_struct = StructureMgr.GetFeaStruct( m_FeaMeshStructIndex );

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

            m_RemoveSkinTris = skin->m_RemoveSkinTrisFlag();

            skin->FetchFeaXFerSurf( skinxfersurfs, 0, fea_struct->GetUSuppress(), fea_struct->GetWSuppress() );

            // Load Skin XFerSurf to m_SurfVec
            LoadSurfs( skinxfersurfs );

            for ( int j = 0; j < m_SurfVec.size(); j++ )
            {
                m_SurfVec[j]->SetFeaPartIndex( skin_index );
            }
        }
    }
}

void FeaMeshMgrSingleton::TransferMeshSettings()
{
    FeaStructure* fea_struct = StructureMgr.GetFeaStruct( m_FeaMeshStructIndex );

    if ( fea_struct )
    {
        m_StructSettings = SimpleFeaMeshSettings();
        m_StructSettings.CopyFrom( fea_struct->GetStructSettingsPtr() );

        m_FeaGridDensity = SimpleFeaGridDensity();
        m_FeaGridDensity.CopyFrom( fea_struct->GetFeaGridDensityPtr() );
    }
}

void FeaMeshMgrSingleton::GetMassUnit()
{
    switch ( m_Vehicle->m_StructUnit() )
    {
        case vsp::SI_UNIT:
            m_MassUnit = "kg";
            break;

        case vsp::CGS_UNIT:
            m_MassUnit = "g";
            break;

        case vsp::MPA_UNIT:
            m_MassUnit = "tonne"; // or Mg/
            break;

        case vsp::BFT_UNIT:
            m_MassUnit = "slug";
            break;

        case vsp::BIN_UNIT:
            m_MassUnit = "lbf*sec" + string( 1, 178 ) + "/in";
            break;
    }
}

void FeaMeshMgrSingleton::TransferFeaData()
{
    // Transfer FeaPart Data
    FeaStructure* fea_struct = StructureMgr.GetFeaStruct( m_FeaMeshStructIndex );

    if ( fea_struct )
    {
        vector < FeaPart* > fea_part_vec = fea_struct->GetFeaPartVec();
        m_FeaPartNameVec.resize( m_NumFeaParts );
        m_FeaPartTypeVec.resize( m_NumFeaParts );
        m_FeaPartIncludedElementsVec.resize( m_NumFeaParts );
        m_FeaPartPropertyIndexVec.resize( m_NumFeaParts );
        m_FeaPartCapPropertyIndexVec.resize( m_NumFeaParts );

        for ( size_t i = 0; i < fea_part_vec.size(); i++ )
        {
            m_FeaPartNameVec[i] = fea_part_vec[i]->GetName();
            m_FeaPartTypeVec[i] = fea_part_vec[i]->GetType();
            m_FeaPartIncludedElementsVec[i] = fea_part_vec[i]->m_IncludedElements.Get();
            m_FeaPartPropertyIndexVec[i] = fea_part_vec[i]->m_FeaPropertyIndex();
            m_FeaPartCapPropertyIndexVec[i] = fea_part_vec[i]->m_CapFeaPropertyIndex();
        }
    }

    // Transfer FeaProperty Data
    vector < FeaProperty* > fea_prop_vec = StructureMgr.GetFeaPropertyVec();
    m_SimplePropertyVec.resize( fea_prop_vec.size() );

    for ( size_t i = 0; i < fea_prop_vec.size(); i++ )
    {
        m_SimplePropertyVec[i] = SimpleFeaProperty();
        m_SimplePropertyVec[i].CopyFrom( fea_prop_vec[i] );
    }

    // Transfer FeaMaterial Data
    vector < FeaMaterial* > fea_mat_vec = StructureMgr.GetFeaMaterialVec();
    m_SimpleMaterialVec.resize( fea_mat_vec.size() );

    for ( size_t i = 0; i < fea_mat_vec.size(); i++ )
    {
        m_SimpleMaterialVec[i] = SimpleFeaMaterial();
        m_SimpleMaterialVec[i].CopyFrom( fea_mat_vec[i] );
    }
}

void FeaMeshMgrSingleton::TransferSubSurfData()
{
    FeaStructure* fea_struct = StructureMgr.GetFeaStruct( m_FeaMeshStructIndex );

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
    m_NumFeaSubSurfs = m_SimpleSubSurfaceVec.size();
}

void FeaMeshMgrSingleton::GenerateFeaMesh()
{
    m_FeaMeshInProgress = true;

    TransferMeshSettings();

    addOutputText( "Load Surfaces\n" );
    LoadSurfaces();

    if ( m_SurfVec.size() <= 0 )
    {
        addOutputText( "No Surfaces.  Done.\n" );
        m_FeaMeshInProgress = false;
        return;
    }

    // Hide all geoms after loading surfaces and settings
    m_Vehicle->HideAll();

    GetMassUnit();
    TransferFeaData();
    TransferSubSurfData();
    TransferDrawObjData();

    MergeCoplanarParts();

    CleanMergeSurfs();

    addOutputText( "Add Structure Parts\n" );
    AddStructureParts();

    // TODO: Update and Build Domain for Half Mesh?

    addOutputText( "Build Slice Planes\n" );
    BuildGrid();

    addOutputText( "Intersect\n" );
    Intersect();
    addOutputText( "Finished Intersect\n" );

    addOutputText( "Binary Adaptation Curve Approximation\n" );
    BinaryAdaptIntCurves();

    addOutputText( "Build Target Map\n" );
    BuildTargetMap( CfdMeshMgrSingleton::VOCAL_OUTPUT );

    addOutputText( "InitMesh\n" );
    InitMesh();

    SubTagTris();

    addOutputText( "Set Fixed Points\n" );
    SetFixPointSurfaceNodes();

    addOutputText( "Remesh\n" );
    Remesh( CfdMeshMgrSingleton::VOCAL_OUTPUT );

    SubSurfaceMgr.BuildSingleTagMap();

    CheckSubSurfBorderIntersect();

    CheckDuplicateSSIntersects();

    addOutputText( "Build Fea Mesh\n" );
    BuildFeaMesh();

    addOutputText( "Tag Fea Nodes\n" );
    TagFeaNodes();

    RemoveSubSurfFeaTris();

    RemoveSkinTris();

    addOutputText( "Exporting Files\n" );
    ExportFeaMesh();

    addOutputText( "Finished\n" );

    m_FeaMeshInProgress = false;
}

void FeaMeshMgrSingleton::ExportFeaMesh()
{
    if ( GetStructSettingsPtr()->GetExportFileFlag( vsp::FEA_NASTRAN_FILE_NAME ) )
    {
        WriteNASTRAN( GetStructSettingsPtr()->GetExportFileName( vsp::FEA_NASTRAN_FILE_NAME ) );
    }

    if ( GetStructSettingsPtr()->GetExportFileFlag( vsp::FEA_CALCULIX_FILE_NAME ) )
    {
        WriteCalculix();
    }

    if ( GetStructSettingsPtr()->GetExportFileFlag( vsp::FEA_STL_FILE_NAME ) )
    {
        WriteSTL( GetStructSettingsPtr()->GetExportFileName( vsp::FEA_STL_FILE_NAME ) );
    }

    if ( GetStructSettingsPtr()->GetExportFileFlag( vsp::FEA_GMSH_FILE_NAME ) )
    {
        WriteGmsh();
    }

    if ( GetStructSettingsPtr()->GetExportFileFlag( vsp::FEA_GMSH_FILE_NAME ) )
    {
        WriteGmsh();
    }

    if ( GetStructSettingsPtr()->GetExportFileFlag( vsp::FEA_MASS_FILE_NAME ) )
    {
        ComputeWriteMass();
        string mass_output = "Total Mass = " + std::to_string( m_TotalMass ) + "\n";
        addOutputText( mass_output );
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


}

void FeaMeshMgrSingleton::MergeCoplanarParts()
{
    FeaStructure* fea_struct = StructureMgr.GetFeaStruct( m_FeaMeshStructIndex );

    vector < VspSurf > all_surf_vec;
    vector < vec3d > all_norm_vec;
    vector < int > all_feaprt_ind_vec;
    vector < int > feaprt_surf_ind_vec;

    if ( fea_struct )
    {
        vector < FeaPart* > fea_part_vec = fea_struct->GetFeaPartVec();

        // Collect all surfaces and compute norms
        for ( size_t k = 1; k < m_NumFeaParts; k++ )
        {
            if ( fea_part_vec[k]->GetType() != vsp::FEA_DOME )
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
                if ( ( abs( abs( dot( all_norm_vec[i], all_norm_vec[j] ) ) - 1.0 ) <= FLT_EPSILON ) && ( all_norm_vec[j].mag() >= FLT_EPSILON ) && ( all_norm_vec[i].mag() >= FLT_EPSILON ) )
                {
                    vec3d pntA = all_surf_vec[i].CompPnt01( 0.5, 0.5 );
                    vec3d pntB = all_surf_vec[j].CompPnt01( 0.5, 0.5 );

                    BndBox bboxA, bboxB;
                    all_surf_vec[i].GetBoundingBox( bboxA );
                    all_surf_vec[j].GetBoundingBox( bboxB );

                    if ( ( dist_pnt_2_plane( pntA, all_norm_vec[i], pntB ) <= FLT_EPSILON ) && Compare( bboxA, bboxB ) )
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
                            // Coplanar surfaces are symmetric. Delete one surface and significantly oversize the other. 
                            // Note: Fixed points on the oversized surface will not be at the same UW coordinate
                            vec3d orgin, all_max, all_min;

                            if ( dist( maxA, orgin ) > dist( maxB, orgin ) )
                            {
                                all_max = maxA;
                            }
                            else
                            {
                                all_max = maxB;
                            }

                            if ( dist( minA, orgin ) > dist( minB, orgin ) )
                            {
                                all_min = minA;
                            }
                            else
                            {
                                all_min = minB;
                            }

                            // Move center of the surface to the orgin prior to expanding
                            vec3d centerA = ( maxA + minA ) / 2;
                            new_surf.Offset( -1 * centerA );

                            vec3d diag_vec = all_max - all_min;

                            // Note: The scaling may need adjustment. Errors can result if the surface becomes extremely large from merging multiple surfaces
                            new_surf.ScaleX( 1 + abs( diag_vec[0] ) );
                            new_surf.ScaleY( 1 + abs( diag_vec[1] ) );
                            new_surf.ScaleZ( 1 + abs( diag_vec[2] ) );

                            new_surf.Offset( centerA );
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
                        all_surf_vec[i] = new_surf;
                        all_surf_vec[j] = new_surf;
                        all_norm_vec[j] = vec3d();

                        // Output warning if FeaParts are different due to data loss (symmetric parts retain all FeaPart data)
                        if ( all_feaprt_ind_vec[i] != all_feaprt_ind_vec[j] )
                        {
                            string output = "WARNING: Coplanar Surfaces Merged: " + fea_part_vec[all_feaprt_ind_vec[i]]->GetName() +
                                ", " + fea_part_vec[all_feaprt_ind_vec[j]]->GetName() + "\n";

                            addOutputText( output.c_str() );
                        }
                    }
                }
            }
        }
    }
}

void FeaMeshMgrSingleton::AddStructureParts()
{
    FeaStructure* fea_struct = StructureMgr.GetFeaStruct( m_FeaMeshStructIndex );

    if ( fea_struct )
    {
        int fix_pnt_cnt = 0;
        vector < FeaPart* > fea_part_vec = fea_struct->GetFeaPartVec();

        //===== Add FeaParts ====//
        for ( int i = 1; i < m_NumFeaParts; i++ ) // FeaSkin is index 0 and has been added already
        {
            if ( !fea_struct->FeaPartIsFixPoint( i ) )
            {
                int part_index = fea_struct->GetFeaPartIndex( fea_part_vec[i] );
                vector< XferSurf > partxfersurfs;

                fea_part_vec[i]->FetchFeaXFerSurf( partxfersurfs, -9999 + ( i - 1 ) );

                // Load Rib XFerSurf to m_SurfVec
                LoadSurfs( partxfersurfs );

                // Identify the FeaPart index and add to m_FeaPartSurfVec
                int begin = m_SurfVec.size() - partxfersurfs.size();
                int end = m_SurfVec.size();

                for ( int j = begin; j < end; j++ )
                {
                    m_SurfVec[j]->SetFeaPartIndex( part_index );
                }
            }
            else if ( fea_struct->FeaPartIsFixPoint( i ) ) 
            {
                //===== Add FixedPoint Data ====//
                FeaFixPoint* fixpnt = dynamic_cast<FeaFixPoint*>( fea_part_vec[i] );
                assert( fixpnt );

                // Identify which surface(s) the fixed point lies on while taking into consideration surface split suppression
                fixpnt->IdentifySplitSurfIndex( m_StructSettings.m_HalfMeshFlag, fea_struct->GetUSuppress(), fea_struct->GetWSuppress() );

                int part_index = fea_struct->GetFeaPartIndex( fea_part_vec[i] );
                vector < vec3d > pnt_vec = fixpnt->GetPntVec();
                vec2d uw = fixpnt->GetUW();

                for ( size_t j = 0; j < pnt_vec.size(); j++ )
                {
                    // Identify the surface index and coordinate points for the fixed point
                    vector < int > surf_index;
                    surf_index.resize( fixpnt->m_SplitSurfIndex[j].size() );

                    for ( size_t m = 0; m < fixpnt->m_SplitSurfIndex[j].size(); m++ )
                    {
                        for ( size_t k = 0; k < m_SurfVec.size(); k++ )
                        {
                            if ( m_SurfVec[k]->GetSurfID() == fixpnt->m_SplitSurfIndex[j][m] && m_SurfVec[k]->GetFeaPartIndex() == StructureMgr.GetFeaPartIndex( fixpnt->m_ParentFeaPartID ) )
                            {
                                surf_index[m] = k;
                            }
                        }
                    }

                    if ( surf_index.size() > 0 )
                    {
                        m_FixPntMap[fix_pnt_cnt].push_back( pnt_vec[j] );
                        m_FixUWMap[fix_pnt_cnt].push_back( uw );
                        m_FixPntSurfIndMap[fix_pnt_cnt].push_back( surf_index );
                        m_FixPntFeaPartIndexMap[fix_pnt_cnt].push_back( part_index );
                        m_FixPointMassFlagMap[fix_pnt_cnt].push_back( fixpnt->m_FixPointMassFlag.Get() );
                        m_FixPointMassMap[fix_pnt_cnt].push_back( fixpnt->m_FixPointMass.Get() );

                        if ( fixpnt->m_BorderFlag )
                        {
                            m_FixPntBorderFlagMap[fix_pnt_cnt].push_back( BORDER_FIX_POINT );
                        }
                        else
                        {
                            m_FixPntBorderFlagMap[fix_pnt_cnt].push_back( SURFACE_FIX_POINT );
                        }
                    }
                }
                fix_pnt_cnt++;
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

    for ( c1 = m_ISegChainList.begin(); c1 != m_ISegChainList.end(); c1++ )
    {
        c1_index++;
        int c2_index = -1;

        if ( !( *c1 )->m_BorderFlag && ( *c1 )->m_SSIntersectIndex >= 0 )
        {
            for ( c2 = m_ISegChainList.begin(); c2 != m_ISegChainList.end(); c2++ )
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
    //==== Collect All Nodes and Tris ====//
    vector < vec2d > all_uw_vec;
    vector < int > uw_surf_ind_vec; // Vector of surface index for each UW point
    vector < vec3d > all_pnt_vec;
    vector < SimpTri > all_tri_vec;
    vector < int > tri_surf_ind_vec; // Vector of surface index for each SimpTri

    for ( int s = 0; s < (int)m_SurfVec.size(); s++ )
    {
        vector < vec2d > uw_vec_curr = m_SurfVec[s]->GetMesh()->GetSimpUWPntVec();
        vector < vec3d > pnt_vec_curr = m_SurfVec[s]->GetMesh()->GetSimpPntVec();
        vector < SimpTri > tri_vec_curr = m_SurfVec[s]->GetMesh()->GetSimpTriVec();

        for ( size_t i = 0; i < tri_vec_curr.size(); i++ )
        {
            // Offset SimpTri indexes 
            tri_vec_curr[i].ind0 += all_pnt_vec.size();
            tri_vec_curr[i].ind1 += all_pnt_vec.size();
            tri_vec_curr[i].ind2 += all_pnt_vec.size();

            all_tri_vec.push_back( tri_vec_curr[i] );
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
        for ( size_t j = 0; j < all_tri_vec.size(); j++ )
        {
            if ( all_tri_vec[j].ind0 == i && all_tri_vec[j].ind0 != index_vec[i] )
            {
                all_tri_vec[j].ind0 = index_vec[i];
            }

            if ( all_tri_vec[j].ind1 == i && all_tri_vec[j].ind1 != index_vec[i] )
            {
                all_tri_vec[j].ind1 = index_vec[i];
            }

            if ( all_tri_vec[j].ind2 == i && all_tri_vec[j].ind2 != index_vec[i] )
            {
                all_tri_vec[j].ind2 = index_vec[i];
            }
        }
    }

    // Build FeaTris
    for ( int i = 0; i < (int)all_tri_vec.size(); i++ )
    {
        vec3d pnt0 = node_vec[all_tri_vec[i].ind0];
        vec3d pnt1 = node_vec[all_tri_vec[i].ind1];
        vec3d pnt2 = node_vec[all_tri_vec[i].ind2];

        vec3d avg_pnt = ( pnt0 + pnt1 + pnt2 ) / 3.0;

        // Determine initial guess for uw near SimpTri center (uw points defining SimpTri can be from different surfaces after adjustments) 
        vec2d uw_guess;

        if ( tri_surf_ind_vec[i] == new_uw_surf_ind_vec[all_tri_vec[i].ind0] )
        {
            uw_guess = new_uw_vec[all_tri_vec[i].ind0];
        }
        else if ( tri_surf_ind_vec[i] == new_uw_surf_ind_vec[all_tri_vec[i].ind1] )
        {
            uw_guess = new_uw_vec[all_tri_vec[i].ind1];
        }
        else if ( tri_surf_ind_vec[i] == new_uw_surf_ind_vec[all_tri_vec[i].ind2] )
        {
            uw_guess = new_uw_vec[all_tri_vec[i].ind2];
        }
        else
        {
            uw_guess.set_x( m_SurfVec[tri_surf_ind_vec[i]]->GetSurfCore()->GetMidU() );
            uw_guess.set_y( m_SurfVec[tri_surf_ind_vec[i]]->GetSurfCore()->GetMidW() );
        }

        vec2d closest_uw = m_SurfVec[tri_surf_ind_vec[i]]->ClosestUW( avg_pnt, uw_guess[0], uw_guess[1] );

        // Determine tangent u-direction for orientation vector at tri midpoint
        vec3d orient_vec;

        if ( m_SurfVec[tri_surf_ind_vec[i]]->ValidUW( closest_uw ) )
        {
            orient_vec = m_SurfVec[tri_surf_ind_vec[i]]->GetSurfCore()->CompTanU( closest_uw[0], closest_uw[1] );
        }

        orient_vec.normalize();

        FeaTri* tri = new FeaTri;
        tri->Create( node_vec[all_tri_vec[i].ind0], node_vec[all_tri_vec[i].ind1], node_vec[all_tri_vec[i].ind2], orient_vec );
        tri->SetFeaPartIndex( m_SurfVec[tri_surf_ind_vec[i]]->GetFeaPartIndex() );

        // Check for subsurface:
        if ( all_tri_vec[i].m_Tags.size() == 2 )
        {
            // First index of m_Tags is the parent surface. Second index is subsurface index which begins 
            //  from the last FeaPart surface index (FeaFixPoints are not tagged; they are not surfaces)
            tri->SetFeaSSIndex( all_tri_vec[i].m_Tags[1] - ( m_NumFeaParts - m_NumFeaFixPoints ) - 1 );
        }
        else if ( all_tri_vec[i].m_Tags.size() > 2 )
        {
            //Give priority to first tagged subsurface in the event of overlap
            tri->SetFeaSSIndex( all_tri_vec[i].m_Tags[1] - ( m_NumFeaParts - m_NumFeaFixPoints ) - 1 );
        }

        m_FeaElementVec.push_back( tri );
        m_NumTris++;
    }

    // Build FeaBeam Intersections
    list< ISegChain* >::iterator c;

    for ( c = m_ISegChainList.begin(); c != m_ISegChainList.end(); c++ )
    {
        if ( !( *c )->m_BorderFlag ) // Only include intersection curves
        {
            // Check at least one surface intersection cap flag is true
            int FeaPartCapA = m_FeaPartIncludedElementsVec[( *c )->m_SurfA->GetFeaPartIndex()];
            int FeaPartCapB = m_FeaPartIncludedElementsVec[( *c )->m_SurfB->GetFeaPartIndex()];

            vector< vec3d > ipntVec, inormVec;
            vector < int > ssindexVec;
            Surf* NormSurf;
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

                // Use node point if close to beam endpoints (avoids tolerance errors in BuildIndMap and FindPntInd)
                for ( size_t k = 0; k < node_vec.size(); k++ )
                {
                    if ( dist( node_vec[k], start_pnt ) <= FLT_EPSILON )
                    {
                        start_pnt = node_vec[k];
                    }
                    else if ( dist( node_vec[k], end_pnt ) <= FLT_EPSILON )
                    {
                        end_pnt = node_vec[k];
                    }
                }

                beam->Create( start_pnt, end_pnt, inormVec[j - 1] );
                beam->SetFeaPartIndex( FeaPartIndex );
                beam->SetFeaSSIndex( ssindexVec[j] );
                m_FeaElementVec.push_back( beam );
                m_NumBeams++;
            }
        }
    }
}

void FeaMeshMgrSingleton::ComputeWriteMass()
{
    m_TotalMass = 0.0;

    FILE* fp = fopen( GetStructSettingsPtr()->GetExportFileName( vsp::FEA_MASS_FILE_NAME ).c_str(), "w" );
    if ( fp )
    {
        fprintf( fp, "...FEA Mesh...\n" );
        fprintf( fp, "Mass_Unit: %s\n", m_MassUnit.c_str() );
        fprintf( fp, "Num_Tris: %d\n", m_NumTris );
        fprintf( fp, "Num_Beams: %d\n", m_NumBeams );
        fprintf( fp, "\n" );

        if ( m_NumFeaParts > 0 )
        {
            fprintf( fp, "FeaPart_Name         Mass_Tris   Mass_Beams\n" );
        }

        // Iterate over each FeaPart index and calculate mass of each FeaElement if the current indexes match
        for ( unsigned int i = 0; i < m_NumFeaParts; i++ )
        {
            if ( m_FeaPartTypeVec[i] != vsp::FEA_FIX_POINT )
            {
                double tri_mass = 0;
                double beam_mass = 0;
                int property_id = m_FeaPartPropertyIndexVec[i];
                int cap_property_id = m_FeaPartCapPropertyIndexVec[i];

                for ( int j = 0; j < m_FeaElementVec.size(); j++ )
                {
                    if ( m_FeaElementVec[j]->GetFeaPartIndex() == i && m_FeaElementVec[j]->GetFeaSSIndex() < 0 && m_FeaElementVec[j]->GetElementType() == FeaElement::FEA_TRI_6 )
                    {
                        tri_mass += m_FeaElementVec[j]->ComputeMass( property_id );
                    }
                    else if ( m_FeaElementVec[j]->GetFeaPartIndex() == i && m_FeaElementVec[j]->GetFeaSSIndex() < 0 && m_FeaElementVec[j]->GetElementType() == FeaElement::FEA_BEAM )
                    {
                        beam_mass += m_FeaElementVec[j]->ComputeMass( cap_property_id );
                    }
                }

                string name = m_FeaPartNameVec[i];

                fprintf( fp, "%-20s% -12.4f% -12.4f\n", name.c_str(), tri_mass, beam_mass );

                m_TotalMass += tri_mass + beam_mass;
            }
        }

        // Add point masses
        if ( m_NumFeaFixPoints > 0 )
        {
            fprintf( fp, "\n" );
            fprintf( fp, "PointMass_Name       Mass        X_loc       Y_loc       Z_loc\n" );
        }

        for ( unsigned int i = 0; i < m_NumFeaFixPoints; i++ )
        {
            if ( m_FixPointMassFlagMap[i][0] )
            {
                double pnt_mass = 0;
                string name = m_FeaPartNameVec[m_FixPntFeaPartIndexMap[i][0]];
                vec3d pnt;

                for ( int j = 0; j < m_FeaElementVec.size(); j++ )
                {
                    if ( m_FeaElementVec[j]->GetElementType() == FeaElement::FEA_POINT_MASS && m_FeaElementVec[j]->GetFeaPartIndex() == m_FixPntFeaPartIndexMap[i][0] && m_FeaElementVec[j]->GetFeaSSIndex() < 0 )
                    {
                        pnt_mass += m_FeaElementVec[j]->ComputeMass( -1 ); // property ID ignored for point masses
                        
                        vector < FeaNode* > node_vec;
                        m_FeaElementVec[j]->LoadNodes( node_vec );

                        if ( node_vec.size() > 0 )
                        {
                            pnt = node_vec[0]->m_Pnt;
                        }
                    }
                }

                fprintf( fp, "%-20s% -12.4f% -12.4f% -12.4f% -12.4f\n", name.c_str(), pnt_mass, pnt[0], pnt[1], pnt[2] );

                m_TotalMass += pnt_mass;
            }
        }

        // Iterate over each FeaSubSurface index and calculate mass of each FeaElement if the subsurface indexes match
        if ( m_NumFeaSubSurfs > 0 )
        {
            fprintf( fp, "\n" );
            fprintf( fp, "FeaSubSurf_Name      Mass_Tris   Mass_Beams\n" );
        }
        
        for ( unsigned int i = 0; i < m_NumFeaSubSurfs; i++ )
        {
            double tri_mass = 0;
            double beam_mass = 0;
            int property_id = m_SimpleSubSurfaceVec[i].GetFeaPropertyIndex();
            int cap_property_id = m_SimpleSubSurfaceVec[i].GetCapFeaPropertyIndex();

            for ( int j = 0; j < m_FeaElementVec.size(); j++ )
            {
                if ( m_FeaElementVec[j]->GetFeaSSIndex() == i && m_FeaElementVec[j]->GetElementType() == FeaElement::FEA_TRI_6 )
                {
                    tri_mass += m_FeaElementVec[j]->ComputeMass( property_id );
                }
                else if ( m_FeaElementVec[j]->GetFeaSSIndex() == i && m_FeaElementVec[j]->GetElementType() == FeaElement::FEA_BEAM )
                {
                    beam_mass += m_FeaElementVec[j]->ComputeMass( cap_property_id );
                }
            }

            string name = m_SimpleSubSurfaceVec[i].GetName();

            fprintf( fp, "%-20s% -12.4f% -12.4f\n", name.c_str(), tri_mass, beam_mass );

            m_TotalMass += tri_mass + beam_mass;
        }

        fprintf( fp, "\n" );
        fprintf( fp, "FeaStruct_Name       Total_Mass\n" );
        fprintf( fp, "%-20s% -9.4f\n", m_StructName.c_str(), m_TotalMass );

        fclose( fp );
    }
}

void FeaMeshMgrSingleton::SetFixPointSurfaceNodes()
{
    for ( size_t n = 0; n < m_NumFeaFixPoints; n++ )
    {
        for ( size_t j = 0; j < m_FixPntSurfIndMap[n].size(); j++ )
        {
            if ( m_FixPntBorderFlagMap[n][j] == SURFACE_FIX_POINT && m_FixPntSurfIndMap[n][j].size() == 1 )
            {
                for ( size_t k = 0; k < m_FixPntSurfIndMap[n][j].size(); k++ )
                {
                    for ( size_t i = 0; i < m_SurfVec.size(); i++ )
                    {
                        if ( m_FixPntSurfIndMap[n][j][k] == i )
                        {
                            string fix_point_name = m_FeaPartNameVec[m_FixPntFeaPartIndexMap[n][j]];

                            if ( m_SurfVec[i]->GetMesh()->SetFixPoint( m_FixPntMap[n][j], m_FixUWMap[n][j] ) )
                            {
                                string count = std::to_string( m_SurfVec[i]->GetMesh()->GetNumFixPointIter() );
                                string message = "\t" + fix_point_name + " Number of Iterations:" + count + "\n";
                                addOutputText( message );
                                m_SurfVec[i]->GetMesh()->ResetNumFixPointIter();
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
    for ( size_t n = 0; n < m_NumFeaFixPoints; n++ )
    {
        // Idenitfy and set FeaFixPoints on border curves
        for ( size_t j = 0; j < m_FixPntSurfIndMap[n].size(); j++ )
        {
            // Only check for FeaFixPoints on two surfaces. Nodes are automatically set for more than two surface intersections
            if ( m_FixPntBorderFlagMap[n][j] == BORDER_FIX_POINT && m_FixPntSurfIndMap[n][j].size() == 2 )
            {
                bool split = false;
                list< ISegChain* >::iterator c;
                for ( c = m_ISegChainList.begin(); c != m_ISegChainList.end(); c++ )
                {
                    if ( ( ( *c )->m_SurfA == m_SurfVec[m_FixPntSurfIndMap[n][j][1]] && ( *c )->m_SurfB == m_SurfVec[m_FixPntSurfIndMap[n][j][0]] ) ||
                        ( ( *c )->m_SurfA == m_SurfVec[m_FixPntSurfIndMap[n][j][0]] && ( *c )->m_SurfB == m_SurfVec[m_FixPntSurfIndMap[n][j][1]] ) )
                    {
                        vec2d closest_uwA, closest_uwB;

                        if ( ( *c )->m_SurfA->ValidUW( m_FixUWMap[n][j] ) )
                        {
                            closest_uwA = ( *c )->m_SurfA->ClosestUW( m_FixPntMap[n][j], m_FixUWMap[n][j][0], m_FixUWMap[n][j][1] );
                        }
                        else
                        {
                            closest_uwA = ( *c )->m_SurfA->ClosestUW( m_FixPntMap[n][j] );
                        }

                        if ( ( *c )->m_SurfB->ValidUW( m_FixUWMap[n][j] ) )
                        {
                            closest_uwB = ( *c )->m_SurfB->ClosestUW( m_FixPntMap[n][j], m_FixUWMap[n][j][0], m_FixUWMap[n][j][1] );
                        }
                        else
                        {
                            closest_uwB = ( *c )->m_SurfB->ClosestUW( m_FixPntMap[n][j] );
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

                        IPnt* split_pnt = new IPnt( p0, p1 );
                        bool success = false;

                        if ( p0 )
                        {
                            success = ( *c )->AddBorderSplit( split_pnt, p0 );
                        }
                        if ( p1 && !success )
                        {
                            success = ( *c )->AddBorderSplit( split_pnt, p1 );
                        }

                        if ( success )
                        {
                            string fix_point_name = m_FeaPartNameVec[m_FixPntFeaPartIndexMap[n][j]];
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
                            if ( split_pnt )
                            {
                                delete split_pnt;
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
    // Idenitfy and set FeaFixPoints on intersection curves

    for ( size_t n = 0; n < m_NumFeaFixPoints; n++ )
    {
        for ( size_t j = 0; j < m_FixPntSurfIndMap[n].size(); j++ )
        {
            bool split = false;
            list< ISegChain* >::iterator c;
            for ( c = m_ISegChainList.begin(); c != m_ISegChainList.end(); c++ )
            {
                Puw* p0 = NULL;
                Puw* p1 = NULL;
                IPnt* split_pnt = NULL;
                bool success = false;
                double tol = 1e-3;

                if ( !( *c )->m_BorderFlag && m_FixPntSurfIndMap[n][j].size() == 1 )
                {
                    if ( ( *c )->m_SurfA != ( *c )->m_SurfB ) // Check for intersection between two FeaPart surfaces
                    {
                        if ( ( *c )->m_SurfA == m_SurfVec[m_FixPntSurfIndMap[n][j][0]] )
                        {
                            vec2d closest_uw = ( *c )->m_SurfB->ClosestUW( m_FixPntMap[n][j] );
                            vec3d closest_pnt = ( *c )->m_SurfB->CompPnt( closest_uw[0], closest_uw[1] );

                            if ( ( *c )->m_SurfA->GetCompID() < 0 ) // Looser tolerance for FeaParts
                            {
                                tol = 1e-2;
                            }

                            // Compare FeaFixPoint to closest point on other surface
                            if ( dist( closest_pnt, m_FixPntMap[n][j] ) <= tol )
                            {
                                vec2d closest_uwA = ( *c )->m_SurfA->ClosestUW( m_FixPntMap[n][j], m_FixUWMap[n][j][0], m_FixUWMap[n][j][1] );
                                vec2d closest_uwB = closest_uw;

                                if ( ( *c )->m_SurfA->ValidUW( closest_uwA ) )
                                {
                                    p0 = new Puw( ( *c )->m_SurfA, closest_uwA );
                                }
                                if ( ( *c )->m_SurfB->ValidUW( closest_uwB ) )
                                {
                                    p1 = new Puw( ( *c )->m_SurfB, closest_uwB );
                                }

                                split_pnt = new IPnt( p0, p1 );

                                if ( p0 )
                                {
                                    success = ( *c )->AddBorderSplit( split_pnt, p0 );

                                    if ( success )
                                    {
                                        m_FixPntMap[n][j] = ( *c )->m_SurfA->CompPnt( closest_uwA.x(), closest_uwA.y() );
                                    }
                                }
                                if ( p1 && !success )
                                {
                                    success = ( *c )->AddBorderSplit( split_pnt, p1 );

                                    if ( success )
                                    {
                                        m_FixPntMap[n][j] = closest_pnt;
                                    }
                                }
                            }
                        }
                        else if ( ( *c )->m_SurfB == m_SurfVec[m_FixPntSurfIndMap[n][j][0]] )
                        {
                            vec2d closest_uw = ( *c )->m_SurfA->ClosestUW( m_FixPntMap[n][j] );
                            vec3d closest_pnt = ( *c )->m_SurfA->CompPnt( closest_uw[0], closest_uw[1] );

                            if ( ( *c )->m_SurfB->GetCompID() < 0 ) // Looser tolerance for FeaParts
                            {
                                tol = 1e-2;
                            }

                            // Compare FeaFixPoint to closest point on other surface
                            if ( dist( closest_pnt, m_FixPntMap[n][j] ) <= tol )
                            {
                                vec2d closest_uwA = closest_uw;
                                vec2d closest_uwB = ( *c )->m_SurfB->ClosestUW( m_FixPntMap[n][j], m_FixUWMap[n][j][0], m_FixUWMap[n][j][1] );

                                if ( ( *c )->m_SurfA->ValidUW( closest_uwA ) )
                                {
                                    p0 = new Puw( ( *c )->m_SurfA, closest_uwA );
                                }
                                if ( ( *c )->m_SurfB->ValidUW( closest_uwB ) )
                                {
                                    p1 = new Puw( ( *c )->m_SurfB, closest_uwB );
                                }

                                IPnt* split_pnt = new IPnt( p0, p1 );

                                if ( p0 )
                                {
                                    success = ( *c )->AddBorderSplit( split_pnt, p0 );

                                    if ( success )
                                    {
                                        m_FixPntMap[n][j] = closest_pnt;
                                    }
                                }
                                if ( p1 && !success )
                                {
                                    success = ( *c )->AddBorderSplit( split_pnt, p1 );

                                    if ( success )
                                    {
                                        m_FixPntMap[n][j] = ( *c )->m_SurfB->CompPnt( closest_uwB[0], closest_uwB[1] );
                                    }
                                }
                            }
                        }
                    }
                    else if ( ( *c )->m_SurfA == ( *c )->m_SurfB && ( *c )->m_SurfA->GetSurfID() == m_FixPntSurfIndMap[n][j][0] ) // Indicates SubSurface Edge
                    {
                        double closest_dist = FLT_MAX;

                        for ( size_t m = 0; m < ( *c )->m_ISegDeque.size(); m++ )
                        {
                            vec3d ipnt0 = ( *c )->m_ISegDeque[m]->m_IPnt[0]->m_Pnt;
                            vec3d ipnt1 = ( *c )->m_ISegDeque[m]->m_IPnt[1]->m_Pnt;

                            // Find perpendicular distance from FeaFixPoint to ISeg
                            double distance = ( cross( ( m_FixPntMap[n][j] - ipnt0 ), ( m_FixPntMap[n][j] - ipnt1 ) ).mag() ) / ( ( ipnt1 - ipnt0 ).mag() );

                            if ( distance <= closest_dist )
                            {
                                closest_dist = distance;
                            }
                        }

                        if ( closest_dist < tol )
                        {
                            vec2d closest_uwA = ( *c )->m_SurfA->ClosestUW( m_FixPntMap[n][j], m_FixUWMap[n][j][0], m_FixUWMap[n][j][1] );
                            vec2d closest_uwB = ( *c )->m_SurfB->ClosestUW( m_FixPntMap[n][j], m_FixUWMap[n][j][0], m_FixUWMap[n][j][1] );

                            if ( ( *c )->m_SurfA->ValidUW( closest_uwA ) )
                            {
                                p0 = new Puw( ( *c )->m_SurfA, closest_uwA );
                            }
                            if ( ( *c )->m_SurfB->ValidUW( closest_uwB ) )
                            {
                                p1 = new Puw( ( *c )->m_SurfB, closest_uwB );
                            }

                            IPnt* split_pnt = new IPnt( p0, p1 );

                            if ( p0 )
                            {
                                success = ( *c )->AddBorderSplit( split_pnt, p0 );
                            }
                            if ( p1 && !success )
                            {
                                success = ( *c )->AddBorderSplit( split_pnt, p1 );
                            }
                        }
                    }

                    if ( success )
                    {
                        m_FixPntBorderFlagMap[n][j] = INTERSECT_FIX_POINT;

                        string fix_point_name = m_FeaPartNameVec[m_FixPntFeaPartIndexMap[n][j]];
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
                        if ( split_pnt )
                        {
                            delete split_pnt;
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

    for ( c = m_ISegChainList.begin(); c != m_ISegChainList.end(); c++ )
    {
        for ( size_t i = 0; i < ( *c )->m_SplitVec.size(); i++ )
        {
            all_split_pnt_vec.push_back( ( *c )->m_SplitVec[i]->m_Pnt );
            all_split_uw_vec.push_back( ( *c )->m_SplitVec[i]->m_UW );
        }
    }

    for ( c = m_ISegChainList.begin(); c != m_ISegChainList.end(); c++ )
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

            if ( ( ( abs( abs( ( *c )->m_SurfA->GetSurfCore()->GetMaxW() - ( *c )->m_SurfB->GetSurfCore()->GetMinW() ) - 2 * TMAGIC ) <= FLT_EPSILON ) )
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
                                    if ( abs( uw_curve[uw][0] - uw_curve.back()[0] ) > FLT_EPSILON )
                                    {
                                        const_u_curve = false;
                                    }
                                    if ( abs( uw_curve[uw][1] - uw_curve.back()[1] ) > FLT_EPSILON )
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
                                            ( ( const_u_curve && abs( uw_pnt0[0] - uw_pnt1[0] ) <= FLT_EPSILON ) ||
                                            ( const_w_curve && abs( uw_pnt0[1] - uw_pnt1[1] ) <= FLT_EPSILON ) ) )
                                        {
                                            // Check if a split point lies on subsurface edge
                                            int k = 0;

                                            while ( k < (int)split_pnt_vec.size() && ( *c )->m_BorderFlag && ( *c )->m_SSIntersectIndex < 0 )
                                            {
                                                bool split_on_ss_edge = false;

                                                // Check if the split point lies on the subsurface edge 
                                                if ( const_w_curve && ( ( abs( split_uw_vec[k][1] - uw_pnt0[1] ) <= FLT_EPSILON ) || ( closedW && ( abs( 0.0 - uw_pnt0[1] ) <= FLT_EPSILON ) ) ) &&
                                                    ( ( split_uw_vec[k][0] >= uw_pnt0[0] && split_uw_vec[k][0] <= uw_pnt1[0] ) ||
                                                        ( split_uw_vec[k][0] >= uw_pnt1[0] && split_uw_vec[k][0] <= uw_pnt0[0] ) ) )
                                                {
                                                    split_on_ss_edge = true;
                                                }
                                                else if ( const_u_curve && ( ( abs( split_uw_vec[k][0] - uw_pnt0[0] ) <= FLT_EPSILON ) || ( closedU && ( abs( 0.0 - uw_pnt0[0] ) <= FLT_EPSILON ) ) ) &&
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

                                                    if ( const_w_curve && ( ( abs( split_uw_vec[n][1] - uw_pnt0[1] ) <= FLT_EPSILON ) || ( closedW && ( abs( 0.0 - uw_pnt0[1] ) <= FLT_EPSILON ) ) ) &&
                                                        ( ( ( split_uw_vec[k][0] >= uw_pnt0[0] && split_uw_vec[k][0] <= uw_pnt1[0] ) ||
                                                            ( split_uw_vec[k][0] >= uw_pnt1[0] && split_uw_vec[k][0] <= uw_pnt0[0] ) ) &&
                                                            ( ( split_uw_vec[n][0] >= uw_pnt0[0] && split_uw_vec[n][0] <= uw_pnt1[0] ) ||
                                                            ( split_uw_vec[n][0] >= uw_pnt1[0] && split_uw_vec[n][0] <= uw_pnt0[0] ) ) ) )
                                                    {
                                                        split_on_ss_edge = true;
                                                    }
                                                    else if ( const_u_curve && ( ( abs( split_uw_vec[n][0] - uw_pnt0[0] ) <= FLT_EPSILON ) || ( closedU && ( abs( 0.0 - uw_pnt0[0] ) <= FLT_EPSILON ) ) ) &&
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

            if ( ( surfA->GetCompID() < 0 ) && ( surfB->GetCompID() >= 0 ) && ( i != j ) && ( m_FeaPartTypeVec[surfA->GetFeaPartIndex()] != vsp::FEA_DOME ) )
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

                    for ( int i = 0; i < segsvec.size(); i++ )
                    {
                        vector< SSLineSeg >& segs = segsvec[i];
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
                                for ( c1 = m_ISegChainList.begin(); c1 != m_ISegChainList.end(); c1++ )
                                {
                                    if ( ( *c1 )->m_SurfA == ( *c1 )->m_SurfB ) // Indicates SubSurface ISegChain
                                    {
                                        if ( ( dist_squared( part_pnt0, ( *c1 )->m_ISegDeque[0]->m_IPnt[0]->m_Pnt ) <= FLT_EPSILON && dist_squared( part_pnt1, ( *c1 )->m_ISegDeque.back()->m_IPnt[1]->m_Pnt ) <= FLT_EPSILON ) ||
                                            ( dist_squared( part_pnt0, ( *c1 )->m_ISegDeque.back()->m_IPnt[1]->m_Pnt ) <= FLT_EPSILON && dist_squared( part_pnt1, ( *c1 )->m_ISegDeque[0]->m_IPnt[0]->m_Pnt ) <= FLT_EPSILON ) )
                                        {
                                            // Check if this ISegChain has already been added to remove_chain_list
                                            if ( std::find( remove_chain_list.begin(), remove_chain_list.end(), ( *c1 ) ) == remove_chain_list.end() )
                                            {
                                                string part = m_FeaPartNameVec[surfA->GetFeaPartIndex()];
                                                string message = "Merged Intersection Curve: " + part + " and " + ss_vec[ss].GetName() + "\n";
                                                addOutputText( message.c_str() );

                                                remove_chain_list.push_back( *c1 );

                                                // Split FeaPart ISegChain at SubSurface edge points
                                                for ( c2 = m_ISegChainList.begin(); c2 != m_ISegChainList.end(); c2++ )
                                                {
                                                    if ( ( *c1 ) != ( *c2 ) && ( ( ( *c2 )->m_SurfA == surfA && ( *c2 )->m_SurfB == surfB ) || ( ( *c2 )->m_SurfB == surfA && ( *c2 )->m_SurfA == surfB ) ) )
                                                    {
                                                        vec2d part_UW0 = vec2d( part_U0, part_W0 );
                                                        vec2d part_UW1 = vec2d( part_U1, part_W1 );

                                                        Puw* part_p0 = new Puw( surfA, part_UW0 );
                                                        Puw* skin_p0 = new Puw( surfB, uw_pnt0 );

                                                        Puw* part_p1 = new Puw( surfA, part_UW1 );
                                                        Puw* skin_p1 = new Puw( surfB, uw_pnt1 );

                                                        IPnt* split_pnt0 = new IPnt( part_p0, skin_p0 );
                                                        IPnt* split_pnt1 = new IPnt( part_p1, skin_p1 );

                                                        bool success0 = false;
                                                        bool success1 = false;

                                                        if ( part_p0 )
                                                        {
                                                            success0 = ( *c2 )->AddBorderSplit( split_pnt0, part_p0 );
                                                        }
                                                        if ( skin_p0 && !success0 )
                                                        {
                                                            success0 = ( *c2 )->AddBorderSplit( split_pnt0, skin_p0 );
                                                        }

                                                        if ( part_p1 )
                                                        {
                                                            success1 = ( *c2 )->AddBorderSplit( split_pnt1, part_p1 );
                                                        }
                                                        if ( skin_p1 && !success1 )
                                                        {
                                                            success1 = ( *c2 )->AddBorderSplit( split_pnt1, skin_p1 );
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
                                                            if ( split_pnt0 )
                                                            {
                                                                delete split_pnt0;
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
                                                            if ( split_pnt1 )
                                                            {
                                                                delete split_pnt1;
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

    for ( c = old_chain_list.begin(); c != old_chain_list.end(); c++ )
    {
        if ( std::find( remove_chain_list.begin(), remove_chain_list.end(), ( *c ) ) == remove_chain_list.end() )
        {
            m_ISegChainList.push_back( ( *c ) );
        }
    }
}

void FeaMeshMgrSingleton::RemoveSubSurfFeaTris()
{
    for ( unsigned int i = 0; i < m_NumFeaSubSurfs; i++ )
    {
        for ( int j = 0; j < m_FeaElementVec.size(); j++ )
        {
            if ( m_FeaElementVec[j]->GetFeaSSIndex() == i && m_FeaElementVec[j]->GetElementType() == FeaElement::FEA_TRI_6 )
            {
                if ( m_SimpleSubSurfaceVec[i].m_IncludedElements == vsp::FEA_BEAM )
                {
                    delete m_FeaElementVec[j];
                    m_FeaElementVec.erase( m_FeaElementVec.begin() + j );
                    j--;
                }
            }
        }
    }
}

void FeaMeshMgrSingleton::RemoveSkinTris()
{
    if ( m_RemoveSkinTris )
    {
        for ( size_t i = 0; i < m_NumFeaParts; i++ )
        {
            if ( m_FeaPartTypeVec[i] == vsp::FEA_SKIN )
            {
                for ( int j = 0; j < m_FeaElementVec.size(); j++ )
                {
                    if ( m_FeaElementVec[j]->GetFeaPartIndex() == i && m_FeaElementVec[j]->GetElementType() == FeaElement::FEA_TRI_6 && m_FeaElementVec[j]->GetFeaSSIndex() < 0 )
                    {
                        delete m_FeaElementVec[j];
                        m_FeaElementVec.erase( m_FeaElementVec.begin() + j );
                        j--;
                    }
                }
            }
        }
    }
}

void FeaMeshMgrSingleton::TagFeaNodes()
{
    //==== Collect All FeaNodes ====//
    m_FeaNodeVec.clear();

    for ( int i = 0; i < (int)m_FeaElementVec.size(); i++ )
    {
        m_FeaElementVec[i]->LoadNodes( m_FeaNodeVec );
    }

    vector< vec3d* > m_AllPntVec;
    for ( int i = 0; i < (int)m_FeaNodeVec.size(); i++ )
    {
        m_AllPntVec.push_back( &m_FeaNodeVec[i]->m_Pnt );
    }

    //==== Build Node Map ====//
    m_IndMap.clear();
    m_PntShift.clear();
    int numPnts = BuildIndMap( m_AllPntVec, m_IndMap, m_PntShift );

    //==== Assign Index Numbers to Nodes ====//
    for ( int i = 0; i < (int)m_FeaNodeVec.size(); i++ )
    {
        m_FeaNodeVec[i]->m_Tags.clear();
        int ind = FindPntIndex( m_FeaNodeVec[i]->m_Pnt, m_AllPntVec, m_IndMap );
        m_FeaNodeVec[i]->m_Index = m_PntShift[ind] + 1;
    }

    // Tag FeaPart Nodes with FeaPart Index
    for ( unsigned int i = 0; i < m_NumFeaParts; i++ )
    {
        vector< FeaNode* > temp_nVec;

        for ( int j = 0; j < m_FeaElementVec.size(); j++ )
        {
            if ( m_FeaElementVec[j]->GetFeaPartIndex() == i && m_FeaElementVec[j]->GetFeaSSIndex() < 0 )
            {
                m_FeaElementVec[j]->LoadNodes( temp_nVec );
            }
        }

        for ( int j = 0; j < (int)temp_nVec.size(); j++ )
        {
            int ind = FindPntIndex( temp_nVec[j]->m_Pnt, m_AllPntVec, m_IndMap );
            m_FeaNodeVec[ind]->AddTag( i );
        }
    }

    // Tag FeaSubSurface Nodes with FeaSubSurface Index, beginning at the last FeaPart index (m_NumFeaParts)
    for ( unsigned int i = 0; i < m_NumFeaSubSurfs; i++ )
    {
        vector< FeaNode* > temp_nVec;

        for ( int j = 0; j < m_FeaElementVec.size(); j++ )
        {
            if ( m_FeaElementVec[j]->GetFeaSSIndex() == i && m_FeaElementVec[j]->GetFeaSSIndex() >= 0 && m_SimpleSubSurfaceVec[i].m_IncludedElements != vsp::FEA_BEAM )
            {
                m_FeaElementVec[j]->LoadNodes( temp_nVec );
            }
        }

        for ( int j = 0; j < (int)temp_nVec.size(); j++ )
        {
            int ind = FindPntIndex( temp_nVec[j]->m_Pnt, m_AllPntVec, m_IndMap );
            m_FeaNodeVec[ind]->AddTag( i + m_NumFeaParts );
        }
    }

    //==== Tag FeaFixPoints ====//
    for ( size_t j = 0; j < m_NumFeaFixPoints; j++ )
    {
        for ( size_t k = 0; k < m_FixPntMap[j].size(); k++ )
        {
            for ( int i = 0; i < (int)m_FeaNodeVec.size(); i++ )
            {
                // Compare the distance between node and fixed point, but only use nodes that have been tagged to an FeaPart
                if ( ( dist( m_FeaNodeVec[i]->m_Pnt, m_FixPntMap[j][k] ) <= FLT_EPSILON ) && ( m_FeaNodeVec[i]->m_Tags.size() > 0 ) )
                {
                    m_FeaNodeVec[i]->AddTag( m_FixPntFeaPartIndexMap[j][k] );
                    m_FeaNodeVec[i]->m_FixedPointFlag = true;

                    // Create mass element if mass flag is true
                    if ( m_FixPointMassFlagMap[j][k] )
                    {
                        FeaPointMass* mass = new FeaPointMass;
                        mass->Create( m_FeaNodeVec[i]->m_Pnt, m_FixPointMassMap[j][k] );
                        mass->SetFeaPartIndex( m_FixPntFeaPartIndexMap[j][k] );

                        int ind = FindPntIndex( m_FeaNodeVec[i]->m_Pnt, m_AllPntVec, m_IndMap );
                        mass->m_Corners[0]->m_Index = m_PntShift[ind] + 1;

                        m_FeaElementVec.push_back( mass );
                    }
                    break;
                }
            }
        }
    }
}

void FeaMeshMgrSingleton::WriteNASTRAN( const string &filename )
{
    // Create temporary file to store NASTRAN bulk data. Case control information (SETs) will be 
    //  defined in the *_NASTRAN.dat file prior to the bulk data (elements, gridpoints, etc.)
    FILE* temp = std::tmpfile();

    FILE* fp = fopen( filename.c_str(), "w" ); // Open *_NASTRAN.dat
    if ( fp && temp )
    {
        FILE* nkey_fp = NULL;
        if ( GetStructSettingsPtr()->GetExportFileFlag( vsp::FEA_NKEY_FILE_NAME ) )
        {
            string nkey_fname = GetStructSettingsPtr()->GetExportFileName( vsp::FEA_NKEY_FILE_NAME );
            nkey_fp = fopen( nkey_fname.c_str(), "w" ); // Open *_NASTRAN.nkey
            if ( nkey_fp )
            {
                fprintf( nkey_fp, "$ NASTRAN Tag Key File Generated from %s\n", VSPVERSION4 );
                fprintf( nkey_fp, "%s\n\n", nkey_fname.c_str() );
            }
        }

        // Comments can be at top of NASTRAN file before case control section
        fprintf( fp, "$NASTRAN Data File Generated from %s\n", VSPVERSION4 );
        fprintf( fp, "$Num_Tris: %d\n", m_NumTris );
        fprintf( fp, "$Num_Beams %d\n", m_NumBeams );

        // Write bulk data to temp file
        fprintf( temp, "\nBEGIN BULK\n" );

        int set_cnt = 1;
        int max_grid_id = 0;
        vector < int > grid_id_vec;

        // FeaPart Nodes
        for ( unsigned int i = 0; i < m_NumFeaParts; i++ )
        {
            grid_id_vec.clear();

            fprintf( temp, "\n" );
            fprintf( temp, "$%s Gridpoints\n", m_FeaPartNameVec[i].c_str() );

            if ( m_FeaPartTypeVec[i] != vsp::FEA_FIX_POINT )
            {
                for ( unsigned int j = 0; j < (int)m_FeaNodeVec.size(); j++ )
                {
                    if ( m_PntShift[j] >= 0 )
                    {
                        if ( m_FeaNodeVec[j]->HasOnlyIndex( i ) )
                        {
                            m_FeaNodeVec[j]->WriteNASTRAN( temp );
                            grid_id_vec.push_back( m_FeaNodeVec[j]->m_Index );
                            max_grid_id = max( max_grid_id, m_FeaNodeVec[j]->m_Index );
                        }
                    }
                }
            }
            else if ( m_FeaPartTypeVec[i] == vsp::FEA_FIX_POINT ) // FixedPoint Nodes
            {
                for ( unsigned int j = 0; j < (int)m_FeaNodeVec.size(); j++ )
                {
                    if ( m_PntShift[j] >= 0 )
                    {
                        if ( m_FeaNodeVec[j]->m_Tags.size() > 1 && m_FeaNodeVec[j]->m_FixedPointFlag && m_FeaNodeVec[j]->HasTag( i ) )
                        {
                            m_FeaNodeVec[j]->WriteNASTRAN( temp );
                            grid_id_vec.push_back( m_FeaNodeVec[j]->m_Index );
                            max_grid_id = max( max_grid_id, m_FeaNodeVec[j]->m_Index );
                        }
                    }
                }
            }

            // Write FEA part node set
            string name = m_FeaPartNameVec[i] + "_Gridpoints";
            WriteNASTRANSet( fp, nkey_fp, set_cnt, grid_id_vec, name );
        }

        // SubSurface Nodes
        for ( unsigned int i = 0; i < m_NumFeaSubSurfs; i++ )
        {
            fprintf( temp, "\n" );
            fprintf( temp, "$%s Gridpoints\n", m_SimpleSubSurfaceVec[i].GetName().c_str() );

            grid_id_vec.clear();

            for ( unsigned int j = 0; j < (int)m_FeaNodeVec.size(); j++ )
            {
                if ( m_PntShift[j] >= 0 )
                {
                    if ( m_FeaNodeVec[j]->HasOnlyIndex( i + m_NumFeaParts ) )
                    {
                        m_FeaNodeVec[j]->WriteNASTRAN( temp );
                        grid_id_vec.push_back( m_FeaNodeVec[j]->m_Index );
                        max_grid_id = max( max_grid_id, m_FeaNodeVec[j]->m_Index );
                    }
                }
            }

            // Write subsurface node set
            string name = m_SimpleSubSurfaceVec[i].GetName() + "_Gridpoints";
            WriteNASTRANSet( fp, nkey_fp, set_cnt, grid_id_vec, name );
        }

        // Intersection Nodes
        fprintf( temp, "\n" );
        fprintf( temp, "$Intersections\n" );

        grid_id_vec.clear();

        for ( unsigned int j = 0; j < (int)m_FeaNodeVec.size(); j++ )
        {
            if ( m_PntShift[j] >= 0 )
            {
                if ( m_FeaNodeVec[j]->m_Tags.size() > 1 && !m_FeaNodeVec[j]->m_FixedPointFlag )
                {
                    m_FeaNodeVec[j]->WriteNASTRAN( temp );
                    grid_id_vec.push_back( m_FeaNodeVec[j]->m_Index );
                    max_grid_id = max( max_grid_id, m_FeaNodeVec[j]->m_Index );
                }
            }
        }

        // Write intersection node set
        string name = "Intersection_Gridpoints";
        WriteNASTRANSet( fp, nkey_fp, set_cnt, grid_id_vec, name );

        //==== Remaining Nodes ====//
        fprintf( temp, "\n" );
        fprintf( temp, "$Remainingnodes\n" );
        for ( int i = 0; i < (int)m_FeaNodeVec.size(); i++ )
        {
            grid_id_vec.clear();

            if ( m_PntShift[i] >= 0 && m_FeaNodeVec[i]->m_Tags.size() == 0 )
            {
                m_FeaNodeVec[i]->WriteNASTRAN( temp );
            }

            // Write remaining node set
            string name = "Remaining_Gridpoints";
            WriteNASTRANSet( fp, nkey_fp, set_cnt, grid_id_vec, name );
        }

        int elem_id = max_grid_id + 1; // First element ID begins after last gridpoint ID
        vector < int > shell_elem_id_vec, beam_elem_id_vec;

        // Write FeaParts
        for ( unsigned int i = 0; i < m_NumFeaParts; i++ )
        {
            if ( m_FeaPartTypeVec[i] != vsp::FEA_FIX_POINT )
            {
                fprintf( temp, "\n" );
                fprintf( temp, "$%s\n", m_FeaPartNameVec[i].c_str() );

                shell_elem_id_vec.clear();
                beam_elem_id_vec.clear();

                int property_id = m_FeaPartPropertyIndexVec[i];
                int cap_property_id = m_FeaPartCapPropertyIndexVec[i];

                for ( int j = 0; j < m_FeaElementVec.size(); j++ )
                {
                    if ( m_FeaElementVec[j]->GetFeaPartIndex() == i && m_FeaElementVec[j]->GetFeaSSIndex() < 0 )
                    {
                        if ( m_FeaElementVec[j]->GetElementType() != FeaElement::FEA_BEAM )
                        {
                            m_FeaElementVec[j]->WriteNASTRAN( temp, elem_id, property_id );
                            shell_elem_id_vec.push_back( elem_id );
                        }
                        else
                        {
                            m_FeaElementVec[j]->WriteNASTRAN( temp, elem_id, cap_property_id );
                            beam_elem_id_vec.push_back( elem_id );
                        }

                        elem_id++;
                    }
                }

                // Write shell element set
                string name = m_FeaPartNameVec[i] + "_ShellElements";
                WriteNASTRANSet( fp, nkey_fp, set_cnt, shell_elem_id_vec, name );

                // Write beam element set
                name = m_FeaPartNameVec[i] + "_BeamElements";
                WriteNASTRANSet( fp, nkey_fp, set_cnt, beam_elem_id_vec, name );
            }
        }

        // Write FeaFixPoints
        for ( unsigned int i = 0; i < m_NumFeaFixPoints; i++ )
        {
            if ( m_FixPointMassFlagMap[i][0] )
            {
                fprintf( temp, "\n" );
                fprintf( temp, "$%s\n", m_FeaPartNameVec[m_FixPntFeaPartIndexMap[i][0]].c_str() );

                vector < int > mass_elem_id_vec;

                for ( int j = 0; j < m_FeaElementVec.size(); j++ )
                {
                    if ( m_FeaElementVec[j]->GetElementType() == FeaElement::FEA_POINT_MASS && m_FeaElementVec[j]->GetFeaPartIndex() == m_FixPntFeaPartIndexMap[i][0] && m_FeaElementVec[j]->GetFeaSSIndex() < 0 )
                    {
                        m_FeaElementVec[j]->WriteNASTRAN( temp, elem_id, -1 ); // property ID ignored for Point Masses
                        mass_elem_id_vec.push_back( elem_id );
                        elem_id++;
                    }
                }

                // Write mass element set
                string name = m_FeaPartNameVec[m_FixPntFeaPartIndexMap[i][0]] + "_MassElements";
                WriteNASTRANSet( fp, nkey_fp, set_cnt, mass_elem_id_vec, name );
            }
        }

        // Write FeaSubSurfaces
        for ( unsigned int i = 0; i < m_NumFeaSubSurfs; i++ )
        {
            fprintf( temp, "\n" );
            fprintf( temp, "$%s\n", m_SimpleSubSurfaceVec[i].GetName().c_str() );

            int property_id = m_SimpleSubSurfaceVec[i].GetFeaPropertyIndex();
            int cap_property_id = m_SimpleSubSurfaceVec[i].GetCapFeaPropertyIndex();

            shell_elem_id_vec.clear();
            beam_elem_id_vec.clear();

            for ( int j = 0; j < m_FeaElementVec.size(); j++ )
            {
                if ( m_FeaElementVec[j]->GetFeaSSIndex() == i )
                {
                    if ( m_FeaElementVec[j]->GetElementType() != FeaElement::FEA_BEAM )
                    {
                        m_FeaElementVec[j]->WriteNASTRAN( temp, elem_id, property_id );
                        shell_elem_id_vec.push_back( elem_id );
                    }
                    else
                    {
                        m_FeaElementVec[j]->WriteNASTRAN( temp, elem_id, cap_property_id );
                        beam_elem_id_vec.push_back( elem_id );
                    }

                    elem_id++;
                }
            }

            // Write shell element set
            string name = m_SimpleSubSurfaceVec[i].GetName() + "_ShellElements";
            WriteNASTRANSet( fp, nkey_fp, set_cnt, shell_elem_id_vec, name );

            // Write beam element set
            name = m_SimpleSubSurfaceVec[i].GetName() + "_BeamElements";
            WriteNASTRANSet( fp, nkey_fp, set_cnt, beam_elem_id_vec, name );
        }

        //==== Properties ====//
        fprintf( temp, "\n" );
        fprintf( temp, "$Properties\n" );

        for ( unsigned int i = 0; i < m_SimplePropertyVec.size(); i++ )
        {
            m_SimplePropertyVec[i].WriteNASTRAN( temp, i + 1 );
        }

        //==== Materials ====//
        fprintf( temp, "\n" );
        fprintf( temp, "$Materials\n" );

        for ( unsigned int i = 0; i < m_SimpleMaterialVec.size(); i++ )
        {
            m_SimpleMaterialVec[i].WriteNASTRAN( temp, i + 1 );
        }

        fprintf( temp, "\nEND DATA\n" );

        // Obtain file size:
        fseek( temp, 0, SEEK_END );
        long lSize = ftell( temp );
        rewind( temp );

        // Allocate memory to contain the whole file:
        char * buffer = (char*)malloc( sizeof( char )*lSize );
        if ( buffer == NULL )
        {
            addOutputText( "WriteNASTRAN memory error\n" );
        }

        // Copy the file into the buffer:
        size_t result = fread( buffer, 1, lSize, temp );
        if ( result != lSize )
        {
            addOutputText( "WriteNASTRAN reading error\n" );
        }

        // The whole file is now loaded in the memory buffer. Write to NASTRAN file
        fprintf( fp, buffer );

        // Close open files and free memmory
        fclose( fp );
        fclose( temp );
        free( buffer );

        if ( nkey_fp )
        {
            fclose( nkey_fp );
        }
    }
}

void FeaMeshMgrSingleton::WriteCalculix()
{
    string fn = GetStructSettingsPtr()->GetExportFileName( vsp::FEA_CALCULIX_FILE_NAME );
    FILE* fp = fopen( fn.c_str(), "w" );
    if ( fp )
    {
        fprintf( fp, "**Calculix Data File Generated from %s\n", VSPVERSION4 );
        fprintf( fp, "**Num_Tris: %d\n", m_NumTris );
        fprintf( fp, "**Num_Beams %d\n\n", m_NumBeams );

        int elem_id = 0;
        char str[256];

        //==== Write FeaParts ====//
        for ( unsigned int i = 0; i < m_NumFeaParts; i++ )
        {
            if ( m_FeaPartTypeVec[i] != vsp::FEA_FIX_POINT )
            {
                fprintf( fp, "**%s\n", m_FeaPartNameVec[i].c_str() );
                fprintf( fp, "*NODE, NSET=N%s\n", m_FeaPartNameVec[i].c_str() );

                for ( unsigned int j = 0; j < (int)m_FeaNodeVec.size(); j++ )
                {
                    if ( m_PntShift[j] >= 0 )
                    {
                        if ( m_FeaNodeVec[j]->HasOnlyIndex( i ) )
                        {
                            m_FeaNodeVec[j]->WriteCalculix( fp );
                        }
                    }
                }

                fprintf( fp, "\n" );

                if ( m_FeaPartIncludedElementsVec[i] == vsp::FEA_SHELL || m_FeaPartIncludedElementsVec[i] == vsp::FEA_SHELL_AND_BEAM )
                {
                    fprintf( fp, "*ELEMENT, TYPE=S6, ELSET=E%s\n", m_FeaPartNameVec[i].c_str() );

                    for ( int j = 0; j < m_FeaElementVec.size(); j++ )
                    {
                        if ( m_FeaElementVec[j]->GetFeaPartIndex() == i && m_FeaElementVec[j]->GetElementType() == FeaElement::FEA_TRI_6 && m_FeaElementVec[j]->GetFeaSSIndex() < 0 )
                        {
                            elem_id++;
                            m_FeaElementVec[j]->WriteCalculix( fp, elem_id );
                        }
                    }
                    fprintf( fp, "\n" );
                }

                if ( m_FeaPartIncludedElementsVec[i] == vsp::FEA_BEAM || m_FeaPartIncludedElementsVec[i] == vsp::FEA_SHELL_AND_BEAM )
                {
                    fprintf( fp, "*ELEMENT, TYPE=B32, ELSET=E%s_CAP\n", m_FeaPartNameVec[i].c_str() );

                    for ( int j = 0; j < m_FeaElementVec.size(); j++ )
                    {
                        if ( m_FeaElementVec[j]->GetFeaPartIndex() == i && m_FeaElementVec[j]->GetElementType() == FeaElement::FEA_BEAM && m_FeaElementVec[j]->GetFeaSSIndex() < 0 )
                        {
                            elem_id++;
                            m_FeaElementVec[j]->WriteCalculix( fp, elem_id );
                        }
                    }

                    // Write Normal Vectors
                    fprintf( fp, "\n" );
                    fprintf( fp, "*NORMAL\n" );

                    for ( int j = 0; j < m_FeaElementVec.size(); j++ )
                    {
                        if ( m_FeaElementVec[j]->GetFeaPartIndex() == i && m_FeaElementVec[j]->GetElementType() == FeaElement::FEA_BEAM && m_FeaElementVec[j]->GetFeaSSIndex() < 0 )
                        {
                            FeaBeam* beam = dynamic_cast<FeaBeam*>( m_FeaElementVec[j] );
                            assert( beam );
                            beam->WriteCalculixNormal( fp );
                        }
                    }

                    fprintf( fp, "\n" );
                }
            }
        }

        //==== Write Fixed Points ====//
        for ( size_t i = 0; i < m_NumFeaFixPoints; i++ )
        {
            fprintf( fp, "**%s\n", m_FeaPartNameVec[m_FixPntFeaPartIndexMap[i][0]].c_str() );
            fprintf( fp, "*NODE, NSET=N%s\n", m_FeaPartNameVec[m_FixPntFeaPartIndexMap[i][0]].c_str() );

            for ( unsigned int j = 0; j < (int)m_FeaNodeVec.size(); j++ )
            {
                if ( m_PntShift[j] >= 0 )
                {
                    if ( m_FeaNodeVec[j]->m_Tags.size() > 1 && m_FeaNodeVec[j]->m_FixedPointFlag && m_FeaNodeVec[j]->HasTag( m_FixPntFeaPartIndexMap[i][0] ) )
                    {
                        m_FeaNodeVec[j]->WriteCalculix( fp );
                    }
                }
            }

            if ( m_FixPointMassFlagMap[i][0] )
            {
                fprintf( fp, "\n" );
                fprintf( fp, "*ELEMENT, TYPE=MASS, ELSET=E%s\n", m_FeaPartNameVec[m_FixPntFeaPartIndexMap[i][0]].c_str() );

                for ( int j = 0; j < m_FeaElementVec.size(); j++ )
                {
                    if ( m_FeaElementVec[j]->GetFeaPartIndex() == m_FixPntFeaPartIndexMap[i][0] && m_FeaElementVec[j]->GetElementType() == FeaElement::FEA_POINT_MASS && m_FeaElementVec[j]->GetFeaSSIndex() < 0 )
                    {
                        elem_id++;
                        m_FeaElementVec[j]->WriteCalculix( fp, elem_id );
                    }
                }

                fprintf( fp, "\n" );

                fprintf( fp, "*MASS, ELSET=E%s\n", m_FeaPartNameVec[m_FixPntFeaPartIndexMap[i][0]].c_str() );
                fprintf( fp, "%f\n", m_FixPointMassMap[i][0] );
            }

            fprintf( fp, "\n" );
        }

        //==== Write SubSurfaces ====//
        for ( unsigned int i = 0; i < m_NumFeaSubSurfs; i++ )
        {
            fprintf( fp, "**%s\n", m_SimpleSubSurfaceVec[i].GetName().c_str() );
            fprintf( fp, "*NODE, NSET=N%s\n", m_SimpleSubSurfaceVec[i].GetName().c_str() );

            for ( unsigned int j = 0; j < (int)m_FeaNodeVec.size(); j++ )
            {
                if ( m_PntShift[j] >= 0 )
                {
                    if ( m_FeaNodeVec[j]->HasOnlyIndex( i + m_NumFeaParts ) )
                    {
                        m_FeaNodeVec[j]->WriteCalculix( fp );
                    }
                }
            }

            if ( m_SimpleSubSurfaceVec[i].m_IncludedElements == vsp::FEA_SHELL || m_SimpleSubSurfaceVec[i].m_IncludedElements == vsp::FEA_SHELL_AND_BEAM )
            {
                fprintf( fp, "\n" );
                fprintf( fp, "*ELEMENT, TYPE=S6, ELSET=E%s\n", m_SimpleSubSurfaceVec[i].GetName().c_str() );

                for ( int j = 0; j < m_FeaElementVec.size(); j++ )
                {
                    if ( m_FeaElementVec[j]->GetFeaSSIndex() == i && m_FeaElementVec[j]->GetElementType() == FeaElement::FEA_TRI_6 )
                    {
                        elem_id++;
                        m_FeaElementVec[j]->WriteCalculix( fp, elem_id );
                    }
                }
                fprintf( fp, "\n" );
            }

            if ( m_SimpleSubSurfaceVec[i].m_IncludedElements == vsp::FEA_BEAM || m_SimpleSubSurfaceVec[i].m_IncludedElements == vsp::FEA_SHELL_AND_BEAM )
            {
                fprintf( fp, "\n" );
                fprintf( fp, "*ELEMENT, TYPE=B32, ELSET=E%s_CAP\n", m_SimpleSubSurfaceVec[i].GetName().c_str() );

                for ( int j = 0; j < m_FeaElementVec.size(); j++ )
                {
                    if ( m_FeaElementVec[j]->GetFeaSSIndex() == i && m_FeaElementVec[j]->GetElementType() == FeaElement::FEA_BEAM )
                    {
                        elem_id++;
                        m_FeaElementVec[j]->WriteCalculix( fp, elem_id );
                    }
                }

                // Write Normal Vectors
                fprintf( fp, "\n" );
                fprintf( fp, "*NORMAL\n" );

                for ( int j = 0; j < m_FeaElementVec.size(); j++ )
                {
                    if ( m_FeaElementVec[j]->GetFeaSSIndex() == i && m_FeaElementVec[j]->GetElementType() == FeaElement::FEA_BEAM )
                    {
                        FeaBeam* beam = dynamic_cast<FeaBeam*>( m_FeaElementVec[j] );
                        assert( beam );
                        beam->WriteCalculixNormal( fp );
                    }
                }

                fprintf( fp, "\n" );
            }
        }

        //==== Intersection Nodes ====//
        fprintf( fp, "**Intersections\n" );
        fprintf( fp, "*NODE, NSET=Nintersections\n" );

        for ( unsigned int j = 0; j < (int)m_FeaNodeVec.size(); j++ )
        {
            if ( m_PntShift[j] >= 0 )
            {
                if ( m_FeaNodeVec[j]->m_Tags.size() > 1 && !m_FeaNodeVec[j]->m_FixedPointFlag )
                {
                    m_FeaNodeVec[j]->WriteCalculix( fp );
                }
            }
        }
        fprintf( fp, "\n" );

        //==== Remaining Nodes ====//
        fprintf( fp, "**Remaining Nodes\n" );
        fprintf( fp, "*NODE, NSET=RemainingNodes\n" );
        for ( int i = 0; i < (int)m_FeaNodeVec.size(); i++ )
        {
            if ( m_PntShift[i] >= 0 && m_FeaNodeVec[i]->m_Tags.size() == 0 )
            {
                m_FeaNodeVec[i]->WriteCalculix( fp );
            }
        }

        //==== FeaProperties ====//
        for ( unsigned int i = 0; i < m_NumFeaParts; i++ )
        {
            if ( m_FeaPartTypeVec[i] != vsp::FEA_FIX_POINT )
            {
                int property_id = m_FeaPartPropertyIndexVec[i];
                int cap_property_id = m_FeaPartCapPropertyIndexVec[i];

                if ( m_FeaPartIncludedElementsVec[i] == vsp::FEA_SHELL || m_FeaPartIncludedElementsVec[i] == vsp::FEA_SHELL_AND_BEAM )
                {
                    fprintf( fp, "\n" );
                    sprintf( str, "E%s", m_FeaPartNameVec[i].c_str() );
                    m_SimplePropertyVec[property_id].WriteCalculix( fp, str );
                }

                if ( m_FeaPartIncludedElementsVec[i] == vsp::FEA_BEAM || m_FeaPartIncludedElementsVec[i] == vsp::FEA_SHELL_AND_BEAM )
                {
                    fprintf( fp, "\n" );
                    sprintf( str, "E%s_CAP", m_FeaPartNameVec[i].c_str() );
                    m_SimplePropertyVec[cap_property_id].WriteCalculix( fp, str );
                }
            }
        }

        for ( unsigned int i = 0; i < m_NumFeaSubSurfs; i++ )
        {
            int property_id = m_SimpleSubSurfaceVec[i].GetFeaPropertyIndex();
            int cap_property_id = m_SimpleSubSurfaceVec[i].GetCapFeaPropertyIndex();

            if ( m_SimpleSubSurfaceVec[i].m_IncludedElements == vsp::FEA_SHELL || m_SimpleSubSurfaceVec[i].m_IncludedElements == vsp::FEA_SHELL_AND_BEAM )
            {
                fprintf( fp, "\n" );
                sprintf( str, "E%s", m_SimpleSubSurfaceVec[i].GetName().c_str() );
                m_SimplePropertyVec[property_id].WriteCalculix( fp, str );
            }

            if ( m_SimpleSubSurfaceVec[i].m_IncludedElements == vsp::FEA_BEAM || m_SimpleSubSurfaceVec[i].m_IncludedElements == vsp::FEA_SHELL_AND_BEAM )
            {
                fprintf( fp, "\n" );
                sprintf( str, "E%s_CAP", m_SimpleSubSurfaceVec[i].GetName().c_str() );
                m_SimplePropertyVec[cap_property_id].WriteCalculix( fp, str );
            }
        }


        //==== Materials ====//
        fprintf( fp, "\n" );
        fprintf( fp, "**Materials\n" );
        for ( unsigned int i = 0; i < m_SimpleMaterialVec.size(); i++ )
        {
            m_SimpleMaterialVec[i].WriteCalculix( fp, i );
            fprintf( fp, "\n" );
        }

        fclose( fp );
    }
}

void FeaMeshMgrSingleton::WriteGmsh()
{
    string fn = GetStructSettingsPtr()->GetExportFileName( vsp::FEA_GMSH_FILE_NAME );
    FILE* fp = fopen( fn.c_str(), "w" );
    if ( fp )
    {
        //=====================================================================================//
        //============== Write Gmsh File ======================================================//
        //=====================================================================================//
        fprintf( fp, "$MeshFormat\n" );
        fprintf( fp, "2.2 0 %d\n", ( int )sizeof( double ) );
        fprintf( fp, "$EndMeshFormat\n" );

        // Count FeaNodes
        int node_count = 0;
        for ( unsigned int j = 0; j < (int)m_FeaNodeVec.size(); j++ )
        {
            if ( m_PntShift[j] >= 0 )
            {
                node_count++;
            }
        }

        //==== Group and Name FeaParts ====//
        fprintf( fp, "$PhysicalNames\n" );
        fprintf( fp, "%d\n", m_NumFeaParts - m_NumFeaFixPoints );
        for ( unsigned int i = 0; i < m_NumFeaParts; i++ )
        {
            if ( m_FeaPartTypeVec[i] != vsp::FEA_FIX_POINT )
            {
                fprintf( fp, "9 %d \"%s\"\n", i + 1, m_FeaPartNameVec[i].c_str() );
            }
        }
        fprintf( fp, "$EndPhysicalNames\n" );

        //==== Write Nodes ====//
        fprintf( fp, "$Nodes\n" );
        fprintf( fp, "%d\n", node_count );

        for ( unsigned int j = 0; j < (int)m_FeaNodeVec.size(); j++ )
        {
            if ( m_PntShift[j] >= 0 )
            {
                m_FeaNodeVec[j]->WriteGmsh( fp );
            }
        }

        fprintf( fp, "$EndNodes\n" );

        //==== Write FeaElements ====//
        fprintf( fp, "$Elements\n" );
        fprintf( fp, "%d\n", (int)m_FeaElementVec.size() );

        int ele_cnt = 1;

        for ( unsigned int j = 0; j < m_NumFeaParts; j++ )
        {
            for ( int i = 0; i < (int)m_FeaElementVec.size(); i++ )
            {
                if ( m_FeaElementVec[i]->GetFeaPartIndex() == j )
                {
                    m_FeaElementVec[i]->WriteGmsh( fp, ele_cnt, j + 1 );
                    ele_cnt++;
                }
            }
        }

        fprintf( fp, "$EndElements\n" );
        fclose( fp );

        // Note: Material properties are not supported in *.msh file
    }
}

void FeaMeshMgrSingleton::TransferDrawObjData()
{
    FeaStructure* fea_struct = StructureMgr.GetFeaStruct( m_FeaMeshStructIndex );

    if ( fea_struct )
    {
        vector < FeaPart* > fea_part_vec = fea_struct->GetFeaPartVec();

        // FeaParts:
        for ( unsigned int i = 0; i < m_NumFeaParts; i++ )
        {
            string name = m_StructName + ":  " + m_FeaPartNameVec[i];

            if ( fea_part_vec[i]->m_IncludedElements() == vsp::FEA_SHELL || fea_part_vec[i]->m_IncludedElements() == vsp::FEA_SHELL_AND_BEAM )
            {
                m_DrawBrowserNameVec.push_back( name );
                m_DrawBrowserPartIndexVec.push_back( i );
            }

            if ( m_FeaPartTypeVec[i] == vsp::FEA_FIX_POINT )
            {
                m_FixPointFeaPartFlagVec.push_back( true );
            }
            else
            {
                m_FixPointFeaPartFlagVec.push_back( false );
            }

            m_DrawElementFlagVec.push_back( true );

            if ( m_FeaPartIncludedElementsVec[i] == vsp::FEA_BEAM || m_FeaPartIncludedElementsVec[i] == vsp::FEA_SHELL_AND_BEAM )
            {
                name += "_CAP";
                m_DrawBrowserNameVec.push_back( name );
                m_DrawBrowserPartIndexVec.push_back( i );
                m_DrawCapFlagVec.push_back( true );
            }
            else
            { 
                m_DrawCapFlagVec.push_back( false );
            }
        }

        // FeaSubSurfaces:
        for ( unsigned int i = 0; i < m_NumFeaSubSurfs; i++ )
        {
            string name = m_StructName + ":  " + m_SimpleSubSurfaceVec[i].GetName();

            if ( m_SimpleSubSurfaceVec[i].m_TestType != vsp::NONE && ( m_SimpleSubSurfaceVec[i].m_IncludedElements == vsp::FEA_SHELL || m_SimpleSubSurfaceVec[i].m_IncludedElements == vsp::FEA_SHELL_AND_BEAM ) )
            {
                m_DrawBrowserNameVec.push_back( name );
                m_DrawBrowserPartIndexVec.push_back( m_NumFeaParts + i );
            }

            m_DrawElementFlagVec.push_back( true );

            if ( m_SimpleSubSurfaceVec[i].m_IncludedElements == vsp::FEA_BEAM || m_SimpleSubSurfaceVec[i].m_IncludedElements == vsp::FEA_SHELL_AND_BEAM )
            {
                name += "_CAP";
                m_DrawBrowserNameVec.push_back( name );
                m_DrawBrowserPartIndexVec.push_back( m_NumFeaParts + i );
                m_DrawCapFlagVec.push_back( true );
            }
            else
            {
                m_DrawCapFlagVec.push_back( false );
            }
        }
    }
}

void FeaMeshMgrSingleton::SetDrawElementFlag( int index, bool flag )
{
    if ( index >= 0 && index < m_DrawElementFlagVec.size() && m_DrawElementFlagVec.size() > 0 )
    {
        m_DrawElementFlagVec[index] = flag;
    }
}

void FeaMeshMgrSingleton::SetDrawCapFlag( int index, bool flag )
{
    if ( index >= 0 && index < m_DrawCapFlagVec.size() && m_DrawCapFlagVec.size() > 0 )
    {
        m_DrawCapFlagVec[index] = flag;
    }
}

bool FeaMeshMgrSingleton::FeaDataAvailable()
{
    if ( m_FeaElementVec.size() > 0 )
    {
        return true;
    }
    return false;
}

void FeaMeshMgrSingleton::SetAllDisplayFlags( bool flag )
{
    for ( unsigned int i = 0; i < m_DrawElementFlagVec.size(); i++ )
    {
        SetDrawElementFlag( i, flag );
    }

    for ( unsigned int i = 0; i < m_DrawCapFlagVec.size(); i++ )
    {
        SetDrawCapFlag( i, flag );
    }
}

void FeaMeshMgrSingleton::LoadDrawObjs( vector< DrawObj* > &draw_obj_vec )
{
    if ( !GetGridDensityPtr() || !GetStructSettingsPtr() )
    {
        return;
    }

    if ( !GetFeaMeshInProgress() && ( m_DrawElementFlagVec.size() == m_NumFeaParts + m_NumFeaSubSurfs ) )
    {
        SurfaceIntersectionSingleton::LoadDrawObjs( draw_obj_vec );

        // FeaParts:
        m_FeaNodeDO.resize( m_NumFeaParts );
        m_FeaElementDO.resize( m_NumFeaParts );
        m_CapFeaElementDO.resize( m_NumFeaParts );
        m_TriOrientationDO.resize( m_NumFeaParts );
        m_CapNormDO.resize( m_NumFeaParts );

        double line_length = GetGridDensityPtr()->m_MinLen / 3.0; // Length of orientation vectors

        // Calculate constants for color sequence.
        const int ncgrp = 6; // Number of basic colors
        const double ncstep = (int)ceil( (double)( 4 * ( m_NumFeaParts + m_NumFeaSubSurfs ) ) / (double)ncgrp );
        const double nctodeg = 360.0 / ( ncgrp*ncstep );

        char str[256];
        for ( int cnt = 0; cnt <  m_NumFeaParts; cnt++ )
        {
            m_FeaNodeDO[cnt] = DrawObj();
            m_FeaElementDO[cnt] = DrawObj();
            m_CapFeaElementDO[cnt] = DrawObj();

            sprintf( str, "%s_Node_Tag_%d", GetID().c_str(), cnt );
            m_FeaNodeDO[cnt].m_GeomID = string( str );
            sprintf( str, "%s_Element_Tag_%d", GetID().c_str(), cnt );
            m_FeaElementDO[cnt].m_GeomID = string( str );
            sprintf( str, "%s_Cap_Element_Tag_%d", GetID().c_str(), cnt );
            m_CapFeaElementDO[cnt].m_GeomID = string( str );

            m_FeaNodeDO[cnt].m_Type = DrawObj::VSP_POINTS;
            m_FeaNodeDO[cnt].m_Visible = false;
            m_FeaNodeDO[cnt].m_PointSize = 4.0;

            if ( m_FixPointFeaPartFlagVec[cnt] )
            {
                m_FeaElementDO[cnt].m_Type = DrawObj::VSP_POINTS;
                m_FeaElementDO[cnt].m_PointSize = 7.0;
            }
            else
            {
                m_FeaElementDO[cnt].m_Type = DrawObj::VSP_SHADED_TRIS;
            }
            m_FeaElementDO[cnt].m_Visible = false;
            m_CapFeaElementDO[cnt].m_Type = DrawObj::VSP_LINES;
            m_CapFeaElementDO[cnt].m_Visible = false;
            m_CapFeaElementDO[cnt].m_LineWidth = 3.0;

            if ( GetStructSettingsPtr()->m_DrawMeshFlag ||
                 GetStructSettingsPtr()->m_ColorTagsFlag )   // At least mesh or tags are visible.
            {
                m_FeaElementDO[cnt].m_Visible = true;

                if ( !m_FixPointFeaPartFlagVec[cnt] )
                {
                    if ( GetStructSettingsPtr()->m_DrawMeshFlag &&
                         GetStructSettingsPtr()->m_ColorTagsFlag ) // Both are visible.
                    {
                        m_FeaElementDO[cnt].m_Type = DrawObj::VSP_WIRE_SHADED_TRIS;
                        m_FeaElementDO[cnt].m_LineColor = vec3d( 0.4, 0.4, 0.4 );
                    }
                    else if ( GetStructSettingsPtr()->m_DrawMeshFlag ) // Mesh only
                    {
                        m_FeaElementDO[cnt].m_Type = DrawObj::VSP_HIDDEN_TRIS;
                        m_FeaElementDO[cnt].m_LineColor = vec3d( 0.4, 0.4, 0.4 );
                    }
                    else // Tags only
                    {
                        m_FeaElementDO[cnt].m_Type = DrawObj::VSP_SHADED_TRIS;
                    }
                }
            }

            if ( GetStructSettingsPtr()->m_ColorTagsFlag )
            {
                // Color sequence -- go around color wheel ncstep times with slight
                // offset from ncgrp basic colors.
                // Note, (cnt/ncgrp) uses integer division resulting in floor.
                double deg = ( ( cnt % ncgrp ) * ncstep + ( cnt / ncgrp ) ) * nctodeg;
                double deg2 = ( ( ( m_NumFeaParts + cnt ) % ncgrp ) * ncstep + ( ( m_NumFeaParts + cnt ) / ncgrp ) ) * nctodeg;
                vec3d rgb = m_FeaElementDO[cnt].ColorWheel( deg );
                rgb.normalize();

                if ( m_FixPointFeaPartFlagVec[cnt] )
                {
                    m_FeaElementDO[cnt].m_PointColor = rgb;
                }

                m_FeaNodeDO[cnt].m_PointColor = rgb;
                m_CapFeaElementDO[cnt].m_LineColor = m_CapFeaElementDO[cnt].ColorWheel( deg2 );

                for ( int i = 0; i < 3; i++ )
                {
                    m_FeaElementDO[cnt].m_MaterialInfo.Ambient[i] = (float)rgb.v[i] / 5.0f;
                    m_FeaElementDO[cnt].m_MaterialInfo.Diffuse[i] = 0.4f + (float)rgb.v[i] / 10.0f;
                    m_FeaElementDO[cnt].m_MaterialInfo.Specular[i] = 0.04f + 0.7f * (float)rgb.v[i];
                    m_FeaElementDO[cnt].m_MaterialInfo.Emission[i] = (float)rgb.v[i] / 20.0f;
                }
                m_FeaElementDO[cnt].m_MaterialInfo.Ambient[3] = 1.0f;
                m_FeaElementDO[cnt].m_MaterialInfo.Diffuse[3] = 1.0f;
                m_FeaElementDO[cnt].m_MaterialInfo.Specular[3] = 1.0f;
                m_FeaElementDO[cnt].m_MaterialInfo.Emission[3] = 1.0f;

                m_FeaElementDO[cnt].m_MaterialInfo.Shininess = 32.0f;
            }
            else
            {
                // No color needed for mesh only.
                m_FeaNodeDO[cnt].m_PointColor = vec3d( 0.0, 0.0, 0.0 );
                m_CapFeaElementDO[cnt].m_LineColor = vec3d( 0.0, 0.0, 0.0 );
            }

            draw_obj_vec.push_back( &m_FeaNodeDO[cnt] );
            draw_obj_vec.push_back( &m_FeaElementDO[cnt] );

            if ( GetStructSettingsPtr()->m_DrawNodesFlag && m_DrawElementFlagVec[cnt] )
            {
                m_FeaNodeDO[cnt].m_Visible = true;

                for ( unsigned int j = 0; j < (int)m_FeaNodeVec.size(); j++ )
                {
                    if ( m_PntShift[j] >= 0 )
                    {
                        if ( m_FeaNodeVec[j]->HasTag( cnt ) )
                        {
                            m_FeaNodeDO[cnt].m_PntVec.push_back( m_FeaNodeVec[j]->m_Pnt );
                        }
                    }
                }
            }

            if ( m_DrawElementFlagVec[cnt] )
            {
                if ( m_FixPointFeaPartFlagVec[cnt] )
                {
                    for ( unsigned int j = 0; j < (int)m_FeaNodeVec.size(); j++ )
                    {
                        if ( m_PntShift[j] >= 0 )
                        {
                            if ( m_FeaNodeVec[j]->HasTag( cnt ) && m_FeaNodeVec[j]->m_FixedPointFlag )
                            {
                                m_FeaElementDO[cnt].m_PntVec.push_back( m_FeaNodeVec[j]->m_Pnt );
                            }
                        }
                    }
                }
                else
                {
                    for ( int j = 0; j < m_FeaElementVec.size(); j++ )
                    {
                        if ( m_FeaElementVec[j]->GetFeaPartIndex() == cnt && m_FeaElementVec[j]->GetElementType() == FeaElement::FEA_TRI_6 && m_FeaElementVec[j]->GetFeaSSIndex() < 0 )
                        {
                            vec3d norm = cross( m_FeaElementVec[j]->m_Corners[1]->m_Pnt - m_FeaElementVec[j]->m_Corners[0]->m_Pnt, m_FeaElementVec[j]->m_Corners[2]->m_Pnt - m_FeaElementVec[j]->m_Corners[0]->m_Pnt );
                            norm.normalize();
                            m_FeaElementDO[cnt].m_PntVec.push_back( m_FeaElementVec[j]->m_Corners[0]->m_Pnt );
                            m_FeaElementDO[cnt].m_PntVec.push_back( m_FeaElementVec[j]->m_Corners[1]->m_Pnt );
                            m_FeaElementDO[cnt].m_PntVec.push_back( m_FeaElementVec[j]->m_Corners[2]->m_Pnt );
                            m_FeaElementDO[cnt].m_NormVec.push_back( norm );
                            m_FeaElementDO[cnt].m_NormVec.push_back( norm );
                            m_FeaElementDO[cnt].m_NormVec.push_back( norm );
                        }
                    }
                }
            }

            if ( m_DrawCapFlagVec[cnt] )
            {
                m_CapFeaElementDO[cnt].m_Visible = true;

                for ( int j = 0; j < m_FeaElementVec.size(); j++ )
                {
                    if ( m_FeaElementVec[j]->GetFeaPartIndex() == cnt && m_FeaElementVec[j]->GetElementType() == FeaElement::FEA_BEAM && m_FeaElementVec[j]->GetFeaSSIndex() < 0 )
                    {
                        m_CapFeaElementDO[cnt].m_PntVec.push_back( m_FeaElementVec[j]->m_Corners[0]->m_Pnt );
                        m_CapFeaElementDO[cnt].m_PntVec.push_back( m_FeaElementVec[j]->m_Corners[1]->m_Pnt );

                        // Normal Vec is not required, load placeholder
                        m_CapFeaElementDO[cnt].m_NormVec.push_back( m_FeaElementVec[j]->m_Corners[0]->m_Pnt );
                        m_CapFeaElementDO[cnt].m_NormVec.push_back( m_FeaElementVec[j]->m_Corners[1]->m_Pnt );
                    }
                }
            }

            sprintf( str, "%s_Tri_Norm_%d", GetID().c_str(), cnt );
            m_TriOrientationDO[cnt].m_GeomID = string( str );
            sprintf( str, "%s_Cap_Norm_%d", GetID().c_str(), cnt );
            m_CapNormDO[cnt].m_GeomID = string( str );

            m_TriOrientationDO[cnt].m_Type = DrawObj::VSP_LINES;
            m_TriOrientationDO[cnt].m_LineWidth = 1.0;
            m_CapNormDO[cnt].m_Type = DrawObj::VSP_LINES;
            m_CapNormDO[cnt].m_LineWidth = 1.0;

            if ( GetStructSettingsPtr()->m_DrawElementOrientVecFlag && m_DrawElementFlagVec[cnt] )
            {
                m_TriOrientationDO[cnt].m_Visible = true;
            }
            else
            {
                m_TriOrientationDO[cnt].m_Visible = false;
            }

            if ( GetStructSettingsPtr()->m_DrawElementOrientVecFlag && m_DrawCapFlagVec[cnt] )
            {
                m_CapNormDO[cnt].m_Visible = true;
            }
            else
            {
                m_CapNormDO[cnt].m_Visible = false;
            }

            m_TriOrientationDO[cnt].m_LineColor = vec3d( 0.0, 0.0, 0.0 );

            m_CapNormDO[cnt].m_LineColor = vec3d( 0.0, 0.0, 0.0 );

            vector < vec3d > tri_orient_pnt_vec, cap_norm_pnt_vec;

            for ( int j = 0; j < m_FeaElementVec.size(); j++ )
            {
                if ( m_FeaElementVec[j]->GetFeaPartIndex() == cnt && m_FeaElementVec[j]->GetElementType() == FeaElement::FEA_TRI_6 && m_FeaElementVec[j]->GetFeaSSIndex() < 0 )
                {
                    vec3d center = ( m_FeaElementVec[j]->m_Corners[0]->m_Pnt + m_FeaElementVec[j]->m_Corners[1]->m_Pnt + m_FeaElementVec[j]->m_Corners[2]->m_Pnt ) / 3.0;

                    // Define orientation vec:
                    FeaTri* tri = dynamic_cast<FeaTri*>( m_FeaElementVec[j] );
                    assert( tri );

                    vec3d orient_pnt = center + line_length * tri->m_Orientation;

                    tri_orient_pnt_vec.push_back( center );
                    tri_orient_pnt_vec.push_back( orient_pnt );
                }
                else if ( m_FeaElementVec[j]->GetFeaPartIndex() == cnt && m_FeaElementVec[j]->GetElementType() == FeaElement::FEA_BEAM && m_FeaElementVec[j]->GetFeaSSIndex() < 0 )
                {
                    FeaBeam* beam = dynamic_cast<FeaBeam*>( m_FeaElementVec[j] );
                    assert( beam );

                    vec3d norm_pnt = m_FeaElementVec[j]->m_Mids[0]->m_Pnt + line_length * beam->m_DispVec;

                    cap_norm_pnt_vec.push_back( m_FeaElementVec[j]->m_Mids[0]->m_Pnt );
                    cap_norm_pnt_vec.push_back( norm_pnt );
                }
            }

            m_TriOrientationDO[cnt].m_PntVec = tri_orient_pnt_vec;
            m_CapNormDO[cnt].m_PntVec = cap_norm_pnt_vec;
        }

        // FeaSubSurfaces:
        m_SSFeaNodeDO.resize( m_NumFeaSubSurfs );
        m_SSFeaElementDO.resize( m_NumFeaSubSurfs );
        m_SSCapFeaElementDO.resize( m_NumFeaSubSurfs );
        m_SSTriOrientationDO.resize( m_NumFeaSubSurfs );
        m_SSCapNormDO.resize( m_NumFeaSubSurfs );

        for ( int cnt = 0; cnt < m_NumFeaSubSurfs; cnt++ )
        {
            m_SSFeaNodeDO[cnt] = DrawObj();
            m_SSFeaElementDO[cnt] = DrawObj();
            m_SSCapFeaElementDO[cnt] = DrawObj();

            sprintf( str, "%s_SSNode_Tag_%d", GetID().c_str(), cnt );
            m_SSFeaNodeDO[cnt].m_GeomID = string( str );
            sprintf( str, "%s_SSElement_Tag_%d", GetID().c_str(), cnt );
            m_SSFeaElementDO[cnt].m_GeomID = string( str );
            sprintf( str, "%s_SSCap_Element_Tag_%d", GetID().c_str(), cnt );
            m_SSCapFeaElementDO[cnt].m_GeomID = string( str );

            m_SSFeaNodeDO[cnt].m_Type = DrawObj::VSP_POINTS;
            m_SSFeaNodeDO[cnt].m_Visible = false;
            m_SSFeaNodeDO[cnt].m_PointSize = 3.0;
            m_SSFeaElementDO[cnt].m_Type = DrawObj::VSP_SHADED_TRIS;
            m_SSFeaElementDO[cnt].m_Visible = false;
            m_SSCapFeaElementDO[cnt].m_Type = DrawObj::VSP_LINES;
            m_SSCapFeaElementDO[cnt].m_Visible = false;
            m_SSCapFeaElementDO[cnt].m_LineWidth = 3.0;

            if ( GetStructSettingsPtr()->m_DrawMeshFlag ||
                 GetStructSettingsPtr()->m_ColorTagsFlag )   // At least mesh or tags are visible.
            {
                m_SSFeaElementDO[cnt].m_Visible = true;

                if ( GetStructSettingsPtr()->m_DrawMeshFlag &&
                     GetStructSettingsPtr()->m_ColorTagsFlag ) // Both are visible.
                {
                    m_SSFeaElementDO[cnt].m_Type = DrawObj::VSP_WIRE_SHADED_TRIS;
                    m_SSFeaElementDO[cnt].m_LineColor = vec3d( 0.4, 0.4, 0.4 );
                }
                else if ( GetStructSettingsPtr()->m_DrawMeshFlag ) // Mesh only
                {
                    m_SSFeaElementDO[cnt].m_Type = DrawObj::VSP_HIDDEN_TRIS;
                    m_SSFeaElementDO[cnt].m_LineColor = vec3d( 0.4, 0.4, 0.4 );
                }
                else // Tags only
                {
                    m_SSFeaElementDO[cnt].m_Type = DrawObj::VSP_SHADED_TRIS;
                }
            }

            if ( GetStructSettingsPtr()->m_ColorTagsFlag )
            {
                // Color sequence -- go around color wheel ncstep times with slight
                // offset from ncgrp basic colors.
                // Note, (cnt/ncgrp) uses integer division resulting in floor.
                double deg = ( ( ( m_NumFeaParts + cnt ) % ncgrp ) * ncstep + ( ( m_NumFeaParts + cnt ) / ncgrp ) ) * nctodeg;
                double deg2 = ( ( ( 1 + m_NumFeaParts + m_NumFeaSubSurfs + cnt ) % ncgrp ) * ncstep + ( ( 1 + m_NumFeaParts + m_NumFeaSubSurfs + cnt ) / ncgrp ) ) * nctodeg; 
                vec3d rgb = m_SSFeaElementDO[cnt].ColorWheel( deg );
                rgb.normalize();

                m_SSFeaNodeDO[cnt].m_PointColor = rgb;
                m_SSCapFeaElementDO[cnt].m_LineColor = m_SSCapFeaElementDO[cnt].ColorWheel( deg2 );

                for ( int i = 0; i < 3; i++ )
                {
                    m_SSFeaElementDO[cnt].m_MaterialInfo.Ambient[i] = (float)rgb.v[i] / 5.0f;
                    m_SSFeaElementDO[cnt].m_MaterialInfo.Diffuse[i] = 0.4f + (float)rgb.v[i] / 10.0f;
                    m_SSFeaElementDO[cnt].m_MaterialInfo.Specular[i] = 0.04f + 0.7f * (float)rgb.v[i];
                    m_SSFeaElementDO[cnt].m_MaterialInfo.Emission[i] = (float)rgb.v[i] / 20.0f;
                }
                m_SSFeaElementDO[cnt].m_MaterialInfo.Ambient[3] = 1.0f;
                m_SSFeaElementDO[cnt].m_MaterialInfo.Diffuse[3] = 1.0f;
                m_SSFeaElementDO[cnt].m_MaterialInfo.Specular[3] = 1.0f;
                m_SSFeaElementDO[cnt].m_MaterialInfo.Emission[3] = 1.0f;

                m_SSFeaElementDO[cnt].m_MaterialInfo.Shininess = 32.0f;
            }
            else
            {
                // No color needed for mesh only.
                m_SSFeaNodeDO[cnt].m_PointColor = vec3d( 0.0, 0.0, 0.0 );
                m_SSCapFeaElementDO[cnt].m_LineColor = vec3d( 0.0, 0.0, 0.0 );
            }

            draw_obj_vec.push_back( &m_SSFeaNodeDO[cnt] );
            draw_obj_vec.push_back( &m_SSFeaElementDO[cnt] );
        }

        for ( unsigned int i = 0; i < m_NumFeaSubSurfs; i++ )
        {
            if ( GetStructSettingsPtr()->m_DrawNodesFlag && m_DrawElementFlagVec[i + m_NumFeaParts] )
            {
                m_SSFeaNodeDO[i].m_Visible = true;

                for ( unsigned int j = 0; j < (int)m_FeaNodeVec.size(); j++ )
                {
                    if ( m_PntShift[j] >= 0 )
                    {
                        if ( m_FeaNodeVec[j]->HasTag( i + m_NumFeaParts ) )
                        {
                            m_SSFeaNodeDO[i].m_PntVec.push_back( m_FeaNodeVec[j]->m_Pnt );
                        }
                    }
                }
            }

            if ( m_DrawElementFlagVec[i + m_NumFeaParts] )
            {
                for ( int j = 0; j < m_FeaElementVec.size(); j++ )
                {
                    if ( ( m_FeaElementVec[j]->GetFeaSSIndex() == i ) && m_FeaElementVec[j]->GetElementType() == FeaElement::FEA_TRI_6 )
                    {
                        vec3d norm = cross( m_FeaElementVec[j]->m_Corners[1]->m_Pnt - m_FeaElementVec[j]->m_Corners[0]->m_Pnt, m_FeaElementVec[j]->m_Corners[2]->m_Pnt - m_FeaElementVec[j]->m_Corners[0]->m_Pnt );
                        norm.normalize();
                        m_SSFeaElementDO[i].m_PntVec.push_back( m_FeaElementVec[j]->m_Corners[0]->m_Pnt );
                        m_SSFeaElementDO[i].m_PntVec.push_back( m_FeaElementVec[j]->m_Corners[1]->m_Pnt );
                        m_SSFeaElementDO[i].m_PntVec.push_back( m_FeaElementVec[j]->m_Corners[2]->m_Pnt );
                        m_SSFeaElementDO[i].m_NormVec.push_back( norm );
                        m_SSFeaElementDO[i].m_NormVec.push_back( norm );
                        m_SSFeaElementDO[i].m_NormVec.push_back( norm );
                    }
                }
            }

            if ( m_DrawCapFlagVec[i + m_NumFeaParts] )
            {
                m_SSCapFeaElementDO[i].m_Visible = true;

                for ( int j = 0; j < m_FeaElementVec.size(); j++ )
                {
                    if ( ( m_FeaElementVec[j]->GetFeaSSIndex() == i ) && m_FeaElementVec[j]->GetElementType() == FeaElement::FEA_BEAM )
                    {
                        m_SSCapFeaElementDO[i].m_PntVec.push_back( m_FeaElementVec[j]->m_Corners[0]->m_Pnt );
                        m_SSCapFeaElementDO[i].m_PntVec.push_back( m_FeaElementVec[j]->m_Corners[1]->m_Pnt );

                        // Normal Vec is not required, load placeholder
                        m_SSCapFeaElementDO[i].m_NormVec.push_back( m_FeaElementVec[j]->m_Corners[0]->m_Pnt );
                        m_SSCapFeaElementDO[i].m_NormVec.push_back( m_FeaElementVec[j]->m_Corners[1]->m_Pnt );
                    }
                }
            }

            sprintf( str, "%s_SSTri_Norm_%d", GetID().c_str(), i );
            m_SSTriOrientationDO[i].m_GeomID = string( str );
            sprintf( str, "%s_SSCap_Norm_%d", GetID().c_str(), i );
            m_SSCapNormDO[i].m_GeomID = string( str );

            m_SSTriOrientationDO[i].m_Type = DrawObj::VSP_LINES;
            m_SSTriOrientationDO[i].m_LineWidth = 1.0;
            m_SSCapNormDO[i].m_Type = DrawObj::VSP_LINES;
            m_SSCapNormDO[i].m_LineWidth = 1.0;

            if ( GetStructSettingsPtr()->m_DrawElementOrientVecFlag && m_DrawElementFlagVec[i +m_NumFeaParts] )
            {
                m_SSTriOrientationDO[i].m_Visible = true;
            }
            else
            {
                m_SSTriOrientationDO[i].m_Visible = false;
            }

            if ( GetStructSettingsPtr()->m_DrawElementOrientVecFlag && m_DrawCapFlagVec[i + m_NumFeaParts] )
            {
                m_SSCapNormDO[i].m_Visible = true;
            }
            else
            {
                m_SSCapNormDO[i].m_Visible = false;
            }

            m_SSTriOrientationDO[i].m_LineColor = vec3d( 0.0, 0.0, 0.0 );

            m_SSCapNormDO[i].m_LineColor = vec3d( 0.0, 0.0, 0.0 );

            vector < vec3d > ss_tri_orient_pnt_vec, ss_cap_norm_pnt_vec;

            for ( int j = 0; j < m_FeaElementVec.size(); j++ )
            {
                if ( ( m_FeaElementVec[j]->GetFeaSSIndex() == i ) && m_FeaElementVec[j]->GetElementType() == FeaElement::FEA_TRI_6 )
                {
                    vec3d center = ( m_FeaElementVec[j]->m_Corners[0]->m_Pnt + m_FeaElementVec[j]->m_Corners[1]->m_Pnt + m_FeaElementVec[j]->m_Corners[2]->m_Pnt ) / 3.0;

                    // Define orientation vec:
                    FeaTri* tri = dynamic_cast<FeaTri*>( m_FeaElementVec[j] );
                    assert( tri );

                    vec3d orient_pnt = center + line_length * tri->m_Orientation;

                    ss_tri_orient_pnt_vec.push_back( center );
                    ss_tri_orient_pnt_vec.push_back( orient_pnt );
                }
                else if ( ( m_FeaElementVec[j]->GetFeaSSIndex() == i ) && m_FeaElementVec[j]->GetElementType() == FeaElement::FEA_BEAM )
                {
                    FeaBeam* beam = dynamic_cast<FeaBeam*>( m_FeaElementVec[j] );
                    assert( beam );

                    vec3d norm_pnt = m_FeaElementVec[j]->m_Mids[0]->m_Pnt + line_length * beam->m_DispVec;

                    ss_cap_norm_pnt_vec.push_back( m_FeaElementVec[j]->m_Mids[0]->m_Pnt );
                    ss_cap_norm_pnt_vec.push_back( norm_pnt );
                }
            }

            m_SSTriOrientationDO[i].m_PntVec = ss_tri_orient_pnt_vec;
            m_SSCapNormDO[i].m_PntVec = ss_cap_norm_pnt_vec;
        }

        // Add cap and orientation DrawObjs last so they are drawn over surfaces
        for ( unsigned int i = 0; i < m_NumFeaParts; i++ )
        {
            draw_obj_vec.push_back( &m_CapFeaElementDO[i] );
            draw_obj_vec.push_back( &m_TriOrientationDO[i] );
            draw_obj_vec.push_back( &m_CapNormDO[i] );
        }
        for ( unsigned int i = 0; i < m_NumFeaSubSurfs; i++ )
        {
            draw_obj_vec.push_back( &m_SSCapFeaElementDO[i] );
            draw_obj_vec.push_back( &m_SSTriOrientationDO[i] );
            draw_obj_vec.push_back( &m_SSCapNormDO[i] );
        }
    }
}

void FeaMeshMgrSingleton::UpdateDisplaySettings()
{
    if ( GetStructSettingsPtr() && StructureMgr.GetFeaStruct( m_FeaMeshStructIndex ) )
    {
        GetStructSettingsPtr()->m_DrawMeshFlag = StructureMgr.GetFeaStruct( m_FeaMeshStructIndex )->GetStructSettingsPtr()->m_DrawMeshFlag.Get();
        GetStructSettingsPtr()->m_DrawBadFlag = StructureMgr.GetFeaStruct( m_FeaMeshStructIndex )->GetStructSettingsPtr()->m_DrawBadFlag.Get();
        GetStructSettingsPtr()->m_ColorTagsFlag = StructureMgr.GetFeaStruct( m_FeaMeshStructIndex )->GetStructSettingsPtr()->m_ColorTagsFlag.Get();

        GetStructSettingsPtr()->m_DrawNodesFlag = StructureMgr.GetFeaStruct( m_FeaMeshStructIndex )->GetStructSettingsPtr()->m_DrawNodesFlag.Get();
        GetStructSettingsPtr()->m_DrawElementOrientVecFlag = StructureMgr.GetFeaStruct( m_FeaMeshStructIndex )->GetStructSettingsPtr()->m_DrawElementOrientVecFlag.Get();

        GetStructSettingsPtr()->m_DrawBorderFlag = StructureMgr.GetFeaStruct( m_FeaMeshStructIndex )->GetStructSettingsPtr()->m_DrawBorderFlag.Get();
        GetStructSettingsPtr()->m_DrawIsectFlag = StructureMgr.GetFeaStruct( m_FeaMeshStructIndex )->GetStructSettingsPtr()->m_DrawIsectFlag.Get();
        GetStructSettingsPtr()->m_DrawRawFlag = StructureMgr.GetFeaStruct( m_FeaMeshStructIndex )->GetStructSettingsPtr()->m_DrawRawFlag.Get();
        GetStructSettingsPtr()->m_DrawBinAdaptFlag = StructureMgr.GetFeaStruct( m_FeaMeshStructIndex )->GetStructSettingsPtr()->m_DrawBinAdaptFlag.Get();
        GetStructSettingsPtr()->m_DrawCurveFlag = StructureMgr.GetFeaStruct( m_FeaMeshStructIndex )->GetStructSettingsPtr()->m_DrawCurveFlag.Get();
        GetStructSettingsPtr()->m_DrawPntsFlag = StructureMgr.GetFeaStruct( m_FeaMeshStructIndex )->GetStructSettingsPtr()->m_DrawPntsFlag.Get();

        GetStructSettingsPtr()->m_RelCurveTol = StructureMgr.GetFeaStruct( m_FeaMeshStructIndex )->GetStructSettingsPtr()->m_RelCurveTol.Get();
    }
}

void FeaMeshMgrSingleton::WriteNASTRANSet( FILE* Nastran_fid, FILE* NKey_fid, int & set_num, vector < int > set_ids, const string set_name )
{
    if ( set_ids.size() > 0 && Nastran_fid )
    {
        fprintf( Nastran_fid, "\nSET %d = ", set_num );

        for ( size_t i = 0; i < set_ids.size(); i++ )
        {
            fprintf( Nastran_fid, "%d", set_ids[i] );

            if ( i != set_ids.size() - 1 )
            {
                fprintf( Nastran_fid, "," );

                if ( ( i + 1 ) % 9 == 0 ) // 9 IDs per line
                {
                    fprintf( Nastran_fid, "\n" );
                }
            }
        }

        fprintf( Nastran_fid, "\n" );

        if ( NKey_fid ) // Write to NASTRAN key file if defined
        {
            fprintf( NKey_fid, "%d,%s\n", set_num, set_name.c_str() );
        }

        set_num++; // Increment set identification number
    }
}