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

//=============================================================//
//=============================================================//

FeaMeshMgrSingleton::FeaMeshMgrSingleton() : CfdMeshMgrSingleton()
{
    m_TotalMass = 0.0;
    m_FeaMeshInProgress = false;
    m_NumFeaParts = 0;
    m_NumFeaSubSurfs = 0;
}

FeaMeshMgrSingleton::~FeaMeshMgrSingleton()
{
    CleanUp();
    m_OutStream.clear();
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

    m_NumFeaParts = 0;
    m_NumFeaSubSurfs = 0;
    m_NumFeaFixPoints = 0;

    m_FeaPartNameVec.clear();
    m_FeaPartTypeVec.clear();
    m_FeaPartIntersectCapFlagVec.clear();
    m_FeaPartPropertyIndexVec.clear();
    m_FeaPartCapPropertyIndexVec.clear();

    m_SimplePropertyVec.clear();
    m_SimpleMaterialVec.clear();

    m_FixPntVec.clear();
    m_FixUWVec.clear();
    m_FixPntFeaPartIndexVec.clear();
    m_FixPntSurfIndVec.clear();
    m_FixPntBorderFlagVec.clear();

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
    m_StructName = fea_struct->GetFeaStructName();

    // Identify number of FeaParts
    m_NumFeaParts = fea_struct->NumFeaParts();

    // Identify number of FeaFixPoints
    m_NumFeaFixPoints = fea_struct->GetNumFeaFixPoints();

    LoadSkins();

    CleanMergeSurfs( );

    return true;
}

void FeaMeshMgrSingleton::LoadSkins()
{
    FeaStructure* fea_struct = StructureMgr.GetFeaStruct( m_FeaMeshStructIndex );

    if ( fea_struct )
    {
        FeaPart* skin = fea_struct->GetFeaSkin();

        if ( skin )
        {
            //===== Add FeaSkins ====//
            vector< XferSurf > skinxfersurfs;

            int skin_index = fea_struct->GetFeaPartIndex( skin );

            skin->FetchFeaXFerSurf( skinxfersurfs, 0 );

            // Load Skin XFerSurf to m_SurfVec
            LoadSurfs( skinxfersurfs );

            // Not sure this is needed, could possibly be done in Fetch call above.
            for ( int j = 0; j < m_SurfVec.size(); j++ )
            {
                m_SurfVec[j]->SetFeaPartIndex( skin_index );
            }
        }
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
        m_FeaPartIntersectCapFlagVec.resize( m_NumFeaParts );
        m_FeaPartPropertyIndexVec.resize( m_NumFeaParts );
        m_FeaPartCapPropertyIndexVec.resize( m_NumFeaParts );

        for ( size_t i = 0; i < fea_part_vec.size(); i++ )
        {
            m_FeaPartNameVec[i] = fea_part_vec[i]->GetName();
            m_FeaPartTypeVec[i] = fea_part_vec[i]->GetType();
            m_FeaPartIntersectCapFlagVec[i] = fea_part_vec[i]->m_IntersectionCapFlag.Get();
            m_FeaPartPropertyIndexVec[i] = fea_part_vec[i]->GetFeaPropertyIndex();
            m_FeaPartCapPropertyIndexVec[i] = fea_part_vec[i]->GetCapFeaPropertyIndex();
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
    //m_OutStream.clear();
    m_FeaMeshInProgress = true;

    addOutputText( "Load Surfaces\n" );
    LoadSurfaces();

    if ( m_SurfVec.size() <= 0 )
    {
        m_FeaMeshInProgress = false;
        return;
    }

    // Hide all geoms and FeaParts after loading surfaces
    m_Vehicle->HideAll();
    GetStructSettingsPtr()->m_DrawFeaPartsFlag.Set( false );

    TransferFeaData();
    TransferSubSurfData();
    TransferDrawObjData();

    addOutputText( "Add Structure Parts\n" );
    AddStructureParts();

    // TODO: Update and Build Domain for Half Mesh?

    DeleteAllSources(); // TODO: Remove? No sources in FeaMesh

    addOutputText( "Build Slice Planes\n" );
    BuildGrid();

    addOutputText( "Intersect\n" );
    Intersect();

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

    addOutputText( "Build Fea Mesh\n" );
    BuildFeaMesh();

    addOutputText( "Tag Fea Nodes\n" );
    TagFeaNodes();

    addOutputText( "Exporting Files\n" );
    ExportFeaMesh();

    addOutputText( "Finished\n" );

    m_FeaMeshInProgress = false;
}

void FeaMeshMgrSingleton::ExportFeaMesh()
{
    WriteNASTRAN( GetStructSettingsPtr()->GetExportFileName( vsp::NASTRAN_FILE_NAME ) );
    WriteCalculix();
    WriteSTL( GetStructSettingsPtr()->GetExportFileName( vsp::STL_FEA_NAME ) );
    WriteGmsh();

    ComputeWriteMass();

    string mass_output = "Total Mass = " + std::to_string( m_TotalMass ) + "\n";
    addOutputText( mass_output );
}

void FeaMeshMgrSingleton::AddStructureParts()
{
    FeaStructure* fea_struct = StructureMgr.GetFeaStruct( m_FeaMeshStructIndex );

    if ( fea_struct )
    {
        vector < FeaPart* > fea_part_vec = fea_struct->GetFeaPartVec();

        //===== Add FeaParts ====//
        for ( int i = 1; i < m_NumFeaParts; i++ ) // FeaSkin is index 0 and has been added already
        {
            int part_index = fea_struct->GetFeaPartIndex( fea_part_vec[i] );
            vector< XferSurf > partxfersurfs;

            // Note: FeaFixPoint FeaParts are not surfaces, so FetchFeaXFerSurf will return an empty vector of XferSurf
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

        //===== Add FixedPoint Data ====//
        for ( int i = 0; i < m_NumFeaParts; i++ ) // Fixed Points are added after all surfaces have been added to m_SurfVec 
        {
            if ( fea_struct->FeaPartIsFixPoint( i ) )
            {
                FeaFixPoint* fixpnt = dynamic_cast<FeaFixPoint*>( fea_part_vec[i] );
                assert( fixpnt );

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
                        m_FixPntVec.push_back( pnt_vec[j] );
                        m_FixUWVec.push_back( uw );
                        m_FixPntSurfIndVec.push_back( surf_index );
                        m_FixPntFeaPartIndexVec.push_back( part_index );
                        m_FixPntMagicVVec.push_back( fixpnt->m_MagicVParent );

                        if ( fixpnt->m_BorderFlag )
                        {
                            m_FixPntBorderFlagVec.push_back( BORDER_FIX_POINT );
                        }
                        else
                        {
                            m_FixPntBorderFlagVec.push_back( SURFACE_FIX_POINT );
                        }
                    }
                }
            }
        }
    }
}

void FeaMeshMgrSingleton::BuildFeaMesh()
{
    // Build FeaTris
    for ( int s = 0; s < (int)m_SurfVec.size(); s++ )
    {
        vector < vec2d > uwvec = m_SurfVec[s]->GetMesh()->GetSimpUWPntVec();
        vector < vec3d >pvec = m_SurfVec[s]->GetMesh()->GetSimpPntVec();
        vector < SimpTri > tvec = m_SurfVec[s]->GetMesh()->GetSimpTriVec();

        for ( int i = 0; i < (int)tvec.size(); i++ )
        {
            // Determine tangent u-direction for orientation vector at tri midpoint
            vec2d uw0 = uwvec[tvec[i].ind0];
            vec2d uw1 = uwvec[tvec[i].ind1];
            vec2d uw2 = uwvec[tvec[i].ind2];

            vec2d avg_uw = ( uw0 + uw1 + uw2 ) / 3.0;

            vec3d pnt0 = pvec[tvec[i].ind0];
            vec3d pnt1 = pvec[tvec[i].ind1];
            vec3d pnt2 = pvec[tvec[i].ind2];

            vec3d avg_pnt = ( pnt0 + pnt1 + pnt2 ) / 3.0;

            vec2d closest_uw = m_SurfVec[s]->ClosestUW( avg_pnt, avg_uw[0], avg_uw[1] );

            vec3d orient_vec;

            if ( m_SurfVec[s]->ValidUW( closest_uw ) )
            {
                orient_vec = m_SurfVec[s]->GetSurfCore()->CompTanU( closest_uw[0], closest_uw[1] );
            }

            orient_vec.normalize();

            FeaTri* tri = new FeaTri;
            tri->Create( pvec[tvec[i].ind0], pvec[tvec[i].ind1], pvec[tvec[i].ind2], orient_vec );
            tri->SetFeaPartIndex( m_SurfVec[s]->GetFeaPartIndex() );

            // Check for subsurface:
            if ( tvec[i].m_Tags.size() == 2 )
            {
                // First index of m_Tags is the parent surface. Second index is subsurface index which begins 
                //  from the last FeaPart surface index (FeaFixPoints are not tagged)
                tri->SetFeaSSIndex( tvec[i].m_Tags[1] - ( m_NumFeaParts - m_NumFeaFixPoints ) - 1 );
            }
            else if ( tvec[i].m_Tags.size() > 2 )
            {
                //TODO: Develop Approach for Multiple SubSurfaces Overlap. Current approach is to give priority to first tagged subsurface
                tri->SetFeaSSIndex( tvec[i].m_Tags[1] - ( m_NumFeaParts - m_NumFeaFixPoints ) - 1 );
            }

            m_FeaElementVec.push_back( tri );
        }
    }

    // Build FeaBeam Intersections
    list< ISegChain* >::iterator c;

    for ( c = m_ISegChainList.begin(); c != m_ISegChainList.end(); c++ )
    {
        if ( !( *c )->m_BorderFlag ) // Only include intersection curves
        {
            // Check at least one surface intersection cap flag is true
            bool FeaPartCapA = m_FeaPartIntersectCapFlagVec[( *c )->m_SurfA->GetFeaPartIndex()];
            bool FeaPartCapB = m_FeaPartIntersectCapFlagVec[( *c )->m_SurfB->GetFeaPartIndex()];

            //// Check at least one surface intersection cap flag is true
            //if ( m_FeaMeshStruct->GetFeaPart( ( *c )->m_SurfA->GetFeaPartIndex() ) )
            //{
            //    FeaPartCapA = m_FeaMeshStruct->GetFeaPart( ( *c )->m_SurfA->GetFeaPartIndex() )->m_IntersectionCapFlag();
            //}
            //if ( m_FeaMeshStruct->GetFeaPart( ( *c )->m_SurfB->GetFeaPartIndex() ) )
            //{
            //    FeaPartCapB = m_FeaMeshStruct->GetFeaPart( ( *c )->m_SurfB->GetFeaPartIndex() )->m_IntersectionCapFlag();
            //}

            vector< vec3d > ipntVec;
            vector< vec3d > inormVec;
            vector < int > ssindexVec;
            Surf* NormSurf;
            int FeaPartIndex = -1;

            // Check if one surface is a skin and one is an FeaPart (m_CompID = -9999)
            if ( ( FeaPartCapA || FeaPartCapB ) && ( ( ( *c )->m_SurfA->GetCompID() < 0 && ( *c )->m_SurfB->GetCompID() >= 0 ) || ( ( *c )->m_SurfB->GetCompID() < 0 && ( *c )->m_SurfA->GetCompID() >= 0 ) ) )
            {
                vec3d center;

                if ( ( *c )->m_SurfA->GetCompID() < 0 && FeaPartCapA )
                {
                    FeaPartIndex = ( *c )->m_SurfA->GetFeaPartIndex();
                    center = ( *c )->m_SurfA->GetBBox().GetCenter();
                }
                else if ( ( *c )->m_SurfB->GetCompID() < 0 && FeaPartCapB )
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
                beam->Create( ipntVec[j - 1], ipntVec[j], inormVec[j - 1] );
                beam->SetFeaPartIndex( FeaPartIndex );
                beam->SetFeaSSIndex( ssindexVec[j] );
                m_FeaElementVec.push_back( beam );
            }
        }
    }
}

void FeaMeshMgrSingleton::ComputeWriteMass()
{
    m_TotalMass = 0.0;

    FILE* fp = fopen( GetStructSettingsPtr()->GetExportFileName( vsp::MASS_FILE_NAME ).c_str(), "w" );
    if ( fp )
    {
        fprintf( fp, "FeaStruct_Name: %s\n", m_StructName.c_str() );

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

                fprintf( fp, "\tFeaPartName: %s, Mass_Tris = %f, Mass_Beams = %f\n", name.c_str(), tri_mass, beam_mass );
                m_TotalMass += tri_mass + beam_mass;
            }
        }

        // Iterate over each FeaSubSurface index and calculate mass of each FeaElement if the subsurface indexes match
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

            fprintf( fp, "\tFeaPartName: %s, Mass_Tris = %f, Mass_Beams = %f\n", name.c_str(), tri_mass, beam_mass );
            m_TotalMass += tri_mass + beam_mass;
        }

        fprintf( fp, "Total Mass = %f\n", m_TotalMass );

        fclose( fp );
    }
}

void FeaMeshMgrSingleton::SetFixPointSurfaceNodes()
{
    for ( size_t j = 0; j < m_FixPntSurfIndVec.size(); j++ )
    {
        if ( m_FixPntBorderFlagVec[j] == SURFACE_FIX_POINT && m_FixPntSurfIndVec[j].size() == 1 )
        {
            for ( size_t k = 0; k < m_FixPntSurfIndVec[j].size(); k++ )
            {
                for ( size_t i = 0; i < m_SurfVec.size(); i++ )
                {
                    if ( m_FixPntSurfIndVec[j][k] == i )
                    {
                        string fix_point_name = m_FeaPartNameVec[m_FixPntFeaPartIndexVec[j]];

                        if ( !m_SurfVec[i]->GetMesh()->SetFixPoint( m_FixPntVec[j], m_FixUWVec[j] ) )
                        {
                            string message = "Error: No node found for " + fix_point_name + ". Adjust GridDensity.\n";
                            addOutputText( message );
                        }
                        else
                        {
                            string count = std::to_string( m_SurfVec[i]->GetMesh()->GetNumFixPointIter() );
                            string message = "\t" + fix_point_name + " Number of Iterations:" + count + "\n";
                            addOutputText( message );
                            m_SurfVec[i]->GetMesh()->ResetNumFixPointIter();
                        }
                    }
                }
            }
        }
    }
}

void FeaMeshMgrSingleton::SetFixPointBorderNodes()
{
    // Idenitfy and set FeaFixPoints on border curves
    for ( size_t j = 0; j < m_FixPntSurfIndVec.size(); j++ )
    {
        // Only check for FeaFixPoints on two surfaces. Nodes are automatically set for more than two surface intersections
        if ( m_FixPntBorderFlagVec[j] == BORDER_FIX_POINT && m_FixPntSurfIndVec[j].size() == 2 ) 
        {
            list< ISegChain* >::iterator c;
            for ( c = m_ISegChainList.begin(); c != m_ISegChainList.end(); c++ )
            {
                if ( ( ( *c )->m_SurfA == m_SurfVec[m_FixPntSurfIndVec[j][1]] && ( *c )->m_SurfB == m_SurfVec[m_FixPntSurfIndVec[j][0]] ) ||
                    ( ( *c )->m_SurfA == m_SurfVec[m_FixPntSurfIndVec[j][0]] && ( *c )->m_SurfB == m_SurfVec[m_FixPntSurfIndVec[j][1]] ) )
                {
                    if ( m_FixPntMagicVVec[j] )
                    {
                        if ( ( m_FixUWVec[j].y() == ( *c )->m_SurfA->GetSurfCore()->GetMaxW() + TMAGIC ) ||
                            ( m_FixUWVec[j].y() == ( *c )->m_SurfB->GetSurfCore()->GetMaxW() + TMAGIC ) )
                        {
                            m_FixUWVec[j].set_y( m_FixUWVec[j].y() - TMAGIC );
                        }
                        else if ( ( m_FixUWVec[j].y() == ( *c )->m_SurfA->GetSurfCore()->GetMinW() - TMAGIC ) ||
                            ( m_FixUWVec[j].y() == ( *c )->m_SurfB->GetSurfCore()->GetMinW() - TMAGIC ) )
                        {
                            m_FixUWVec[j].set_y( m_FixUWVec[j].y() + TMAGIC );
                        }
                    }

                    Puw* p0 = NULL;
                    Puw* p1 = NULL;

                    if ( ( *c )->m_SurfA->ValidUW( m_FixUWVec[j] ) )
                    {
                        p0 = new Puw( ( *c )->m_SurfA, m_FixUWVec[j] );
                    }
                    if ( ( *c )->m_SurfB->ValidUW( m_FixUWVec[j] ) )
                    {
                        p1 = new Puw( ( *c )->m_SurfB, m_FixUWVec[j] );
                    }

                    IPnt* split_pnt = new IPnt( p0, p1 );

                    if ( p0 )
                    {
                        ( *c )->AddBorderSplit( split_pnt, p0 );
                    }
                    else if ( p1 )
                    {
                        ( *c )->AddBorderSplit( split_pnt, p1 );
                    }
                }
            }
        }
    }
}

void FeaMeshMgrSingleton::CheckFixPointIntersects()
{
    // Idenitfy and set FeaFixPoints on intersection curves
    double tol = 0.01;

    for ( size_t j = 0; j < m_FixPntSurfIndVec.size(); j++ )
    {
        list< ISegChain* >::iterator c;
        for ( c = m_ISegChainList.begin(); c != m_ISegChainList.end(); c++ )
        {
            if ( !( *c )->m_BorderFlag && m_FixPntSurfIndVec[j].size() == 1 && ( ( *c )->m_SurfA != ( *c )->m_SurfB ) )
            {
                if ( ( *c )->m_SurfA == m_SurfVec[m_FixPntSurfIndVec[j][0]] )
                {
                    vec2d closest_uw = ( *c )->m_SurfB->ClosestUW( m_FixPntVec[j] );
                    vec3d closest_pnt = ( *c )->m_SurfB->CompPnt( closest_uw[0], closest_uw[1] );

                    // Compare FeaFixPoint to closest point on other surface
                    if ( dist( closest_pnt, m_FixPntVec[j] ) <= tol )
                    {
                        Puw* p0 = NULL;
                        Puw* p1 = NULL;
                        
                        if ( ( *c )->m_SurfB->ValidUW( closest_uw ) )
                        {
                            p0 = new Puw( ( *c )->m_SurfB, closest_uw );
                        }
                        if ( ( *c )->m_SurfA->ValidUW( m_FixUWVec[j] ) )
                        {
                            p1 = new Puw( ( *c )->m_SurfA, m_FixUWVec[j] );
                        }

                        IPnt* split_pnt = new IPnt( p0, p1 );

                        if ( p0 )
                        {
                            ( *c )->AddBorderSplit( split_pnt, p0 );
                        }
                        else if ( p1 )
                        {
                            ( *c )->AddBorderSplit( split_pnt, p1 );
                        }

                        m_FixPntBorderFlagVec[j] = INTERSECT_FIX_POINT;
                        m_FixPntVec[j] = closest_pnt; // Set FeaFixPoint to closest point (for node tagging and identification)
                    }
                }
                else if ( ( *c )->m_SurfB == m_SurfVec[m_FixPntSurfIndVec[j][0]] )
                {
                    vec2d closest_uw = ( *c )->m_SurfA->ClosestUW( m_FixPntVec[j] );
                    vec3d closest_pnt = ( *c )->m_SurfA->CompPnt( closest_uw[0], closest_uw[1] );

                    // Compare FeaFixPoint to closest point on other surface
                    if ( dist( closest_pnt, m_FixPntVec[j] ) <= tol )
                    {
                        Puw* p0 = NULL;
                        Puw* p1 = NULL;

                        if ( ( *c )->m_SurfA->ValidUW( closest_uw ) )
                        {
                            p0 = new Puw( ( *c )->m_SurfA, closest_uw );
                        }
                        if ( ( *c )->m_SurfB->ValidUW( m_FixUWVec[j] ) )
                        {
                            p1 = new Puw( ( *c )->m_SurfB, m_FixUWVec[j] );
                        }

                        IPnt* split_pnt = new IPnt( p0, p1 );

                        if ( p0 )
                        {
                            ( *c )->AddBorderSplit( split_pnt, p0 );
                        }
                        else if ( p1 )
                        {
                            ( *c )->AddBorderSplit( split_pnt, p1 );
                        }

                        m_FixPntBorderFlagVec[j] = INTERSECT_FIX_POINT;
                        m_FixPntVec[j] = closest_pnt; // Set FeaFixPoint to closest point (for node tagging and identification)
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

        for ( size_t j = 0; j < m_FixPntVec.size(); j++ )
        {
            if ( dist( m_FeaNodeVec[i]->m_Pnt, m_FixPntVec[j] ) <= FLT_EPSILON )
            {
                m_FeaNodeVec[i]->AddTag( m_FixPntFeaPartIndexVec[j] );
                m_FeaNodeVec[i]->m_FixedPointFlag = true;
            }
        }
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
            if ( ( m_FeaElementVec[j]->GetFeaSSIndex() == i )  && m_FeaElementVec[j]->GetFeaSSIndex() >= 0 )
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
}

void FeaMeshMgrSingleton::WriteNASTRAN( const string &filename )
{
    FILE* fp = fopen( filename.c_str(), "w" );
    if ( fp )
    {
        fprintf( fp, "BEGIN BULK\n" );

        int elem_id = 0;

        // Write FeaParts
        for ( unsigned int i = 0; i < m_NumFeaParts; i++ )
        {
            if ( m_FeaPartTypeVec[i] != vsp::FEA_FIX_POINT )
            {
                fprintf( fp, "\n" );
                fprintf( fp, "$%s\n", m_FeaPartNameVec[i].c_str() );

                int property_id = m_FeaPartPropertyIndexVec[i];
                int cap_property_id = m_FeaPartCapPropertyIndexVec[i];

                for ( int j = 0; j < m_FeaElementVec.size(); j++ )
                {
                    if ( m_FeaElementVec[j]->GetFeaPartIndex() == i && m_FeaElementVec[j]->GetFeaSSIndex() < 0 )
                    {
                        elem_id++;

                        if ( m_FeaElementVec[j]->GetElementType() != FeaElement::FEA_BEAM )
                        {
                            m_FeaElementVec[j]->WriteNASTRAN( fp, elem_id, property_id );
                        }
                        else
                        {
                            m_FeaElementVec[j]->WriteNASTRAN( fp, elem_id, cap_property_id );
                        }
                    }
                }
            }
        }

        // Write FeaSubSurfaces
        for ( unsigned int i = 0; i < m_NumFeaSubSurfs; i++ )
        {
            fprintf( fp, "\n" );
            fprintf( fp, "$%s\n", m_SimpleSubSurfaceVec[i].GetName().c_str() );

            int property_id = m_SimpleSubSurfaceVec[i].GetFeaPropertyIndex();
            int cap_property_id = m_SimpleSubSurfaceVec[i].GetCapFeaPropertyIndex();

            for ( int j = 0; j < m_FeaElementVec.size(); j++ )
            {
                if ( m_FeaElementVec[j]->GetFeaSSIndex() == i )
                {
                    elem_id++;

                    if ( m_FeaElementVec[j]->GetElementType() != FeaElement::FEA_BEAM )
                    {
                        m_FeaElementVec[j]->WriteNASTRAN( fp, elem_id, property_id );
                    }
                    else
                    {
                        m_FeaElementVec[j]->WriteNASTRAN( fp, elem_id, cap_property_id );
                    }
                }
            }
        }

        for ( unsigned int i = 0; i < m_NumFeaParts; i++ )
        {
            if ( m_FeaPartTypeVec[i] != vsp::FEA_FIX_POINT )
            {
                fprintf( fp, "\n" );
                fprintf( fp, "$%s Gridpoints\n", m_FeaPartNameVec[i].c_str() );

                for ( unsigned int j = 0; j < (int)m_FeaNodeVec.size(); j++ )
                {
                    if ( m_PntShift[j] >= 0 )
                    {
                        if ( m_FeaNodeVec[j]->HasOnlyIndex( i ) )
                        {
                            m_FeaNodeVec[j]->WriteNASTRAN( fp );
                        }
                    }
                }
            }
        }

        for ( unsigned int i = 0; i < m_NumFeaSubSurfs; i++ )
        {
            fprintf( fp, "\n" );
            fprintf( fp, "$%s Gridpoints\n", m_SimpleSubSurfaceVec[i].GetName().c_str() );

            for ( unsigned int j = 0; j < (int)m_FeaNodeVec.size(); j++ )
            {
                if ( m_PntShift[j] >= 0 )
                {
                    if ( m_FeaNodeVec[j]->HasOnlyIndex( i + m_NumFeaParts ) )
                    {
                        m_FeaNodeVec[j]->WriteNASTRAN( fp );
                    }
                }
            }
        }

        fprintf( fp, "\n" );
        fprintf( fp, "$Intersections\n" );

        for ( unsigned int j = 0; j < (int)m_FeaNodeVec.size(); j++ )
        {
            if ( m_PntShift[j] >= 0 )
            {
                if ( m_FeaNodeVec[j]->m_Tags.size() > 1 && !m_FeaNodeVec[j]->m_FixedPointFlag )
                {
                    m_FeaNodeVec[j]->WriteNASTRAN( fp );
                }
            }
        }

        fprintf( fp, "\n" );
        fprintf( fp, "$FixedPoints\n" );

        for ( unsigned int j = 0; j < (int)m_FeaNodeVec.size(); j++ )
        {
            if ( m_PntShift[j] >= 0 )
            {
                if ( m_FeaNodeVec[j]->m_Tags.size() > 1 && m_FeaNodeVec[j]->m_FixedPointFlag )
                {
                    m_FeaNodeVec[j]->WriteNASTRAN( fp );
                }
            }
        }

        //==== Remaining Nodes ====//
        fprintf( fp, "\n" );
        fprintf( fp, "$Remainingnodes\n" );
        for ( int i = 0 ; i < ( int )m_FeaNodeVec.size() ; i++ )
        {
            if ( m_PntShift[i] >= 0 && m_FeaNodeVec[i]->m_Tags.size() == 0 )
            {
                m_FeaNodeVec[i]->WriteNASTRAN( fp );
            }
        }

        //==== Properties ====//
        fprintf( fp, "\n" );
        fprintf( fp, "$Properties\n" );

        for ( unsigned int i = 0; i < m_SimplePropertyVec.size(); i++ )
        {
            m_SimplePropertyVec[i].WriteNASTRAN( fp, i + 1 );
        }

        //==== Materials ====//
        fprintf( fp, "\n" );
        fprintf( fp, "$Materials\n" );

        for ( unsigned int i = 0; i < m_SimpleMaterialVec.size(); i++ )
        {
            m_SimpleMaterialVec[i].WriteNASTRAN( fp, i + 1 );
        }

        fprintf( fp, "END DATA\n" );

        fclose( fp );
    }
}

void FeaMeshMgrSingleton::WriteCalculix()
{
    string fn = GetStructSettingsPtr()->GetExportFileName( vsp::CALCULIX_FILE_NAME );
    FILE* fp = fopen( fn.c_str(), "w" );
    if ( fp )
    {
        int elem_id = 0;
        char str[256];

        //==== Write FeaParts ====//
        for ( unsigned int i = 0; i < m_NumFeaParts; i++ )
        {
            if ( m_FeaPartTypeVec[i] != vsp::FEA_FIX_POINT )
            {
                fprintf( fp, "**%s\n", m_FeaPartNameVec[i].c_str() );
                fprintf( fp, "*NODE, NSET=N%s\n", m_FeaPartNameVec[i].c_str() );

                int property_id = m_FeaPartPropertyIndexVec[i];
                int cap_property_id = m_FeaPartCapPropertyIndexVec[i];

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
                sprintf( str, "E%s", m_FeaPartNameVec[i].c_str() );

                m_SimplePropertyVec[property_id].WriteCalculix( fp, str );
                fprintf( fp, "\n" );

                if ( m_FeaPartIntersectCapFlagVec[i] )
                {
                    fprintf( fp, "*ELEMENT, TYPE=B31, ELSET=E%s_CAP\n", m_FeaPartNameVec[i].c_str() );

                    for ( int j = 0; j < m_FeaElementVec.size(); j++ )
                    {
                        if ( m_FeaElementVec[j]->GetFeaPartIndex() == i && m_FeaElementVec[j]->GetElementType() == FeaElement::FEA_BEAM && m_FeaElementVec[j]->GetFeaSSIndex() < 0 )
                        {
                            elem_id++;
                            m_FeaElementVec[j]->WriteCalculix( fp, elem_id );
                        }
                    }

                    fprintf( fp, "\n" );
                    sprintf( str, "E%s_CAP", m_FeaPartNameVec[i].c_str() );

                    m_SimplePropertyVec[cap_property_id].WriteCalculix( fp, str );
                    fprintf( fp, "\n" );

                    // Write Normal Vectors
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

        //==== Write SubSurfaces ====//
        for ( unsigned int i = 0; i < m_NumFeaSubSurfs; i++ )
        {
            fprintf( fp, "**%s\n", m_SimpleSubSurfaceVec[i].GetName().c_str() );
            fprintf( fp, "*NODE, NSET=N%s\n", m_SimpleSubSurfaceVec[i].GetName().c_str() );

            int property_id = m_SimpleSubSurfaceVec[i].GetFeaPropertyIndex();
            int cap_property_id = m_SimpleSubSurfaceVec[i].GetCapFeaPropertyIndex();

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
            sprintf( str, "E%s", m_SimpleSubSurfaceVec[i].GetName().c_str() );

            m_SimplePropertyVec[property_id].WriteCalculix( fp, str );

            if ( m_SimpleSubSurfaceVec[i].m_IntersectionCapFlag )
            {
                fprintf( fp, "\n" );
                fprintf( fp, "*ELEMENT, TYPE=B31, ELSET=E%s_CAP\n", m_SimpleSubSurfaceVec[i].GetName().c_str() );

                for ( int j = 0; j < m_FeaElementVec.size(); j++ )
                {
                    if ( m_FeaElementVec[j]->GetFeaSSIndex() == i && m_FeaElementVec[j]->GetElementType() == FeaElement::FEA_BEAM )
                    {
                        elem_id++;
                        m_FeaElementVec[j]->WriteCalculix( fp, elem_id );
                    }
                }

                fprintf( fp, "\n" );
                sprintf( str, "E%s_CAP", m_SimpleSubSurfaceVec[i].GetName().c_str() );

                m_SimplePropertyVec[cap_property_id].WriteCalculix( fp, str );
                fprintf( fp, "\n" );

                // Write Normal Vectors
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

        //==== Fixed Point Nodes ====//
        fprintf( fp, "**Fixed Points\n" );
        fprintf( fp, "*NODE, NSET=FixedPoints\n" );

        for ( unsigned int j = 0; j < (int)m_FeaNodeVec.size(); j++ )
        {
            if ( m_PntShift[j] >= 0 )
            {
                if ( m_FeaNodeVec[j]->m_Tags.size() > 1 && m_FeaNodeVec[j]->m_FixedPointFlag )
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
    string fn = GetStructSettingsPtr()->GetExportFileName( vsp::GMSH_FEA_NAME );
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

            if ( m_FeaPartTypeVec[i] == vsp::FEA_SLICE )
            {
                FeaSlice* slice = dynamic_cast<FeaSlice*>( fea_part_vec[i] );
                assert( slice );

                if ( slice->m_IncludeTrisFlag() )
                {
                    m_DrawBrowserNameVec.push_back( name );
                    m_DrawBrowserPartIndexVec.push_back( i );
                }
            }
            else
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

            if ( m_FeaPartIntersectCapFlagVec[i] )
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

            if ( m_SimpleSubSurfaceVec[i].m_TestType != vsp::NONE )
            {
                m_DrawBrowserNameVec.push_back( name );
                m_DrawBrowserPartIndexVec.push_back( m_NumFeaParts + i );
            }

            m_DrawElementFlagVec.push_back( true );

            if ( m_SimpleSubSurfaceVec[i].m_IntersectionCapFlag )
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

void FeaMeshMgrSingleton::LoadDrawObjs( vector< DrawObj* > &draw_obj_vec )
{
    if ( !GetFeaMeshInProgress() && ( m_DrawElementFlagVec.size() == m_NumFeaParts + m_NumFeaSubSurfs ) )
    {
        // FeaParts:
        m_FeaNodeDO.resize( m_NumFeaParts );
        m_FeaElementDO.resize( m_NumFeaParts );
        m_CapFeaElementDO.resize( m_NumFeaParts );
        m_TriOrientationDO.resize( m_NumFeaParts );
        m_CapNormDO.resize( m_NumFeaParts );

        double line_length = GetGridDensityPtr()->m_MinLen() / 3.0; // Length of orientation vectors

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

            if ( GetStructSettingsPtr()->m_DrawMeshFlag.Get() ||
                 GetStructSettingsPtr()->m_ColorTagsFlag.Get() )   // At least mesh or tags are visible.
            {
                m_FeaElementDO[cnt].m_Visible = true;

                if ( !m_FixPointFeaPartFlagVec[cnt] )
                {
                    if ( GetStructSettingsPtr()->m_DrawMeshFlag.Get() &&
                         GetStructSettingsPtr()->m_ColorTagsFlag.Get() ) // Both are visible.
                    {
                        m_FeaElementDO[cnt].m_Type = DrawObj::VSP_HIDDEN_TRIS_CFD;
                        m_FeaElementDO[cnt].m_LineColor = vec3d( 0.4, 0.4, 0.4 );
                    }
                    else if ( GetStructSettingsPtr()->m_DrawMeshFlag.Get() ) // Mesh only
                    {
                        m_FeaElementDO[cnt].m_Type = DrawObj::VSP_HIDDEN_TRIS_CFD;
                        m_FeaElementDO[cnt].m_LineColor = vec3d( 0.4, 0.4, 0.4 );
                    }
                    else // Tags only
                    {
                        m_FeaElementDO[cnt].m_Type = DrawObj::VSP_SHADED_TRIS;
                    }
                }
            }

            if ( GetStructSettingsPtr()->m_ColorTagsFlag.Get() )
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

            if ( GetStructSettingsPtr()->m_DrawNodesFlag() )
            {
                m_FeaNodeDO[cnt].m_Visible = true;

                for ( unsigned int j = 0; j < (int)m_FeaNodeVec.size(); j++ )
                {
                    if ( m_PntShift[j] >= 0 )
                    {
                        if ( m_FeaNodeVec[j]->HasOnlyIndex( cnt ) )
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

            if ( GetStructSettingsPtr()->m_DrawElementOrientVecFlag() )
            {
                m_TriOrientationDO[cnt].m_Visible = true;
            }
            else
            {
                m_TriOrientationDO[cnt].m_Visible = false;
            }

            if ( GetStructSettingsPtr()->m_DrawElementOrientVecFlag() )
            {
                m_CapNormDO[cnt].m_Visible = true;
            }
            else
            {
                m_CapNormDO[cnt].m_Visible = false;
            }

            m_TriOrientationDO[cnt].m_LineColor = m_FeaNodeDO[cnt].m_PointColor;

            m_CapNormDO[cnt].m_LineColor = m_CapFeaElementDO[cnt].m_LineColor;

            vector < vec3d > tri_orient_pnt_vec, cap_norm_pnt_vec;

            for ( int j = 0; j < m_FeaElementVec.size(); j++ )
            {
                if ( m_FeaElementVec[j]->GetFeaPartIndex() == cnt && m_FeaElementVec[j]->GetElementType() == FeaElement::FEA_TRI_6 && m_FeaElementVec[j]->GetFeaSSIndex() < 0 )
                {
                    // Define normal vec:
                    vec3d norm = cross( m_FeaElementVec[j]->m_Corners[1]->m_Pnt - m_FeaElementVec[j]->m_Corners[0]->m_Pnt, m_FeaElementVec[j]->m_Corners[2]->m_Pnt - m_FeaElementVec[j]->m_Corners[0]->m_Pnt );
                    norm.normalize();
                    vec3d center = ( m_FeaElementVec[j]->m_Corners[0]->m_Pnt + m_FeaElementVec[j]->m_Corners[1]->m_Pnt + m_FeaElementVec[j]->m_Corners[2]->m_Pnt ) / 3.0;
                    vec3d norm_pnt = center + line_length * norm;

                    tri_orient_pnt_vec.push_back( center );
                    tri_orient_pnt_vec.push_back( norm_pnt );

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

            if ( GetStructSettingsPtr()->m_DrawMeshFlag.Get() ||
                 GetStructSettingsPtr()->m_ColorTagsFlag.Get() )   // At least mesh or tags are visible.
            {
                m_SSFeaElementDO[cnt].m_Visible = true;

                if ( GetStructSettingsPtr()->m_DrawMeshFlag.Get() &&
                     GetStructSettingsPtr()->m_ColorTagsFlag.Get() ) // Both are visible.
                {
                    m_SSFeaElementDO[cnt].m_Type = DrawObj::VSP_HIDDEN_TRIS_CFD;
                    m_SSFeaElementDO[cnt].m_LineColor = vec3d( 0.4, 0.4, 0.4 );
                }
                else if ( GetStructSettingsPtr()->m_DrawMeshFlag.Get() ) // Mesh only
                {
                    m_SSFeaElementDO[cnt].m_Type = DrawObj::VSP_HIDDEN_TRIS_CFD;
                    m_SSFeaElementDO[cnt].m_LineColor = vec3d( 0.4, 0.4, 0.4 );
                }
                else // Tags only
                {
                    m_SSFeaElementDO[cnt].m_Type = DrawObj::VSP_SHADED_TRIS;
                }
            }

            if ( GetStructSettingsPtr()->m_ColorTagsFlag.Get() )
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
            if ( GetStructSettingsPtr()->m_DrawNodesFlag() )
            {
                m_SSFeaNodeDO[i].m_Visible = true;

                for ( unsigned int j = 0; j < (int)m_FeaNodeVec.size(); j++ )
                {
                    if ( m_PntShift[j] >= 0 )
                    {
                        if ( m_FeaNodeVec[j]->HasOnlyIndex( i + m_NumFeaParts ) )
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

            if ( GetStructSettingsPtr()->m_DrawElementOrientVecFlag() )
            {
                m_SSTriOrientationDO[i].m_Visible = true;
            }
            else
            {
                m_SSTriOrientationDO[i].m_Visible = false;
            }

            if ( GetStructSettingsPtr()->m_DrawElementOrientVecFlag() )
            {
                m_SSCapNormDO[i].m_Visible = true;
            }
            else
            {
                m_SSCapNormDO[i].m_Visible = false;
            }

            m_SSTriOrientationDO[i].m_LineColor = m_SSFeaNodeDO[i].m_PointColor;

            m_SSCapNormDO[i].m_LineColor = m_SSCapFeaElementDO[i].m_LineColor;

            vector < vec3d > ss_tri_orient_pnt_vec, ss_cap_norm_pnt_vec;

            for ( int j = 0; j < m_FeaElementVec.size(); j++ )
            {
                if ( ( m_FeaElementVec[j]->GetFeaSSIndex() == i ) && m_FeaElementVec[j]->GetElementType() == FeaElement::FEA_TRI_6 )
                {
                    // Define normal vec:
                    vec3d norm = cross( m_FeaElementVec[j]->m_Corners[1]->m_Pnt - m_FeaElementVec[j]->m_Corners[0]->m_Pnt, m_FeaElementVec[j]->m_Corners[2]->m_Pnt - m_FeaElementVec[j]->m_Corners[0]->m_Pnt );
                    norm.normalize();
                    vec3d center = ( m_FeaElementVec[j]->m_Corners[0]->m_Pnt + m_FeaElementVec[j]->m_Corners[1]->m_Pnt + m_FeaElementVec[j]->m_Corners[2]->m_Pnt ) / 3.0;
                    vec3d norm_pnt = center + line_length * norm;

                    ss_tri_orient_pnt_vec.push_back( center );
                    ss_tri_orient_pnt_vec.push_back( norm_pnt );

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

                    vec3d norm_pnt = m_FeaElementVec[j]->m_Corners[0]->m_Pnt + line_length * beam->m_DispVec;

                    ss_cap_norm_pnt_vec.push_back( m_FeaElementVec[j]->m_Corners[0]->m_Pnt );
                    ss_cap_norm_pnt_vec.push_back( norm_pnt );
                }
            }

            m_SSTriOrientationDO[i].m_PntVec = ss_tri_orient_pnt_vec;
            m_SSCapNormDO[i].m_PntVec = ss_cap_norm_pnt_vec;

            draw_obj_vec.push_back( &m_SSTriOrientationDO[i] );
            draw_obj_vec.push_back( &m_SSCapNormDO[i] );
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

        // Render bad edges
        m_MeshBadEdgeDO.m_GeomID = GetID() + "BADEDGE";
        m_MeshBadEdgeDO.m_Type = DrawObj::VSP_LINES;
        m_MeshBadEdgeDO.m_Visible = GetStructSettingsPtr()->m_DrawBadFlag.Get();
        m_MeshBadEdgeDO.m_LineColor = vec3d( 1, 0, 0 );
        m_MeshBadEdgeDO.m_LineWidth = 3.0;

        vector< vec3d > badEdgeData;

        vector< Edge* >::iterator e;
        for ( e = m_BadEdges.begin(); e != m_BadEdges.end(); e++ )
        {
            badEdgeData.push_back( ( *e )->n0->pnt );
            badEdgeData.push_back( ( *e )->n1->pnt );
        }
        m_MeshBadEdgeDO.m_PntVec = badEdgeData;
        // Normal Vec is not required, load placeholder.
        m_MeshBadEdgeDO.m_NormVec = badEdgeData;

        draw_obj_vec.push_back( &m_MeshBadEdgeDO );

        m_MeshBadTriDO.m_GeomID = GetID() + "BADTRI";
        m_MeshBadTriDO.m_Type = DrawObj::VSP_HIDDEN_TRIS_CFD;
        m_MeshBadTriDO.m_Visible = GetStructSettingsPtr()->m_DrawBadFlag.Get();
        m_MeshBadTriDO.m_LineColor = vec3d( 1, 0, 0 );
        m_MeshBadTriDO.m_LineWidth = 3.0;

        vector< vec3d > badTriData;
        vector< Tri* >::iterator t;
        for ( t = m_BadTris.begin(); t != m_BadTris.end(); t++ )
        {
            badTriData.push_back( ( *t )->n0->pnt );
            badTriData.push_back( ( *t )->n1->pnt );
            badTriData.push_back( ( *t )->n2->pnt );
        }
        m_MeshBadTriDO.m_PntVec = badTriData;
        // Normal Vec is not required, load placeholder.
        m_MeshBadTriDO.m_NormVec = badTriData;

        draw_obj_vec.push_back( &m_MeshBadTriDO );
    }
}
