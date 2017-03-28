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
    m_DrawMeshFlag = false;
    m_FeaMeshInProgress = false;
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

    CfdMeshMgrSingleton::CleanUp();
}

bool FeaMeshMgrSingleton::LoadSurfaces()
{
    CleanUp();

    if ( !StructureMgr.ValidTotalFeaStructInd( m_FeaMeshStructIndex ) )
    {
        addOutputText( "FeaMesh Failed: Invalid FeaStructure Selection\n" );
        m_FeaMeshInProgress = false;
        return false;
    }

    // Identify the structure to mesh (m_FeaMeshStructIndex must be set) 
    vector < FeaStructure* > structvec = StructureMgr.GetAllFeaStructs();
    m_FeaMeshStruct = structvec[m_FeaMeshStructIndex];

    LoadSkins();

    CleanMergeSurfs( );

    return true;
}

void FeaMeshMgrSingleton::LoadSkins()
{
    FeaPart* FeaSkin = m_FeaMeshStruct->GetFeaSkin();

    if ( FeaSkin )
    {
        //===== Add FeaSkins ====//
        vector< XferSurf > skinxfersurfs;

        int skin_index = m_FeaMeshStruct->GetFeaPartIndex( FeaSkin );

        FeaSkin->FetchFeaXFerSurf( skinxfersurfs, 0 );

        // Load Skin XFerSurf to m_SurfVec
        LoadSurfs( skinxfersurfs );

        // Not sure this is needed, could possibly be done in Fetch call above.
        for ( int j = 0; j < m_SurfVec.size(); j++ )
        {
            m_SurfVec[j]->SetFeaPartIndex( skin_index );
        }
    }
}

void FeaMeshMgrSingleton::GenerateFeaMesh()
{
    m_OutStream.clear();
    m_FeaMeshInProgress = true;

    addOutputText( "Load Surfaces\n" );
    LoadSurfaces();

    if ( m_SurfVec.size() <= 0 )
    {
        m_FeaMeshInProgress = false;
        return;
    }

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
    vector < FeaPart* > FeaPartVec = m_FeaMeshStruct->GetFeaPartVec();

    //===== Add FeaParts ====//
    int num_parts = FeaPartVec.size();

    for ( int i = 1; i < num_parts; i++ ) // FeaSkin is index 0 and has been added already
    {
        int part_index = m_FeaMeshStruct->GetFeaPartIndex( FeaPartVec[i] );
        vector< XferSurf > partxfersurfs;

        FeaPartVec[i]->FetchFeaXFerSurf( partxfersurfs, -9999 );

        // Load Rib XFerSurf to m_SurfVec
        LoadSurfs( partxfersurfs );

        // Identify the FeaPart Type and ID. Add to m_FeaPartSurfVec
        int begin = m_SurfVec.size() - partxfersurfs.size();
        int end = m_SurfVec.size();

        for ( int j = begin; j < end; j++ )
        {
            m_SurfVec[j]->SetFeaPartIndex( part_index );
        }
    }
}

void FeaMeshMgrSingleton::BuildFeaMesh()
{
    for ( int s = 0; s < (int)m_SurfVec.size(); s++ )
    {
        vector < vec3d >pvec = m_SurfVec[s]->GetMesh()->GetSimpPntVec();
        vector < SimpTri > tvec = m_SurfVec[s]->GetMesh()->GetSimpTriVec();
        for ( int i = 0; i < (int)tvec.size(); i++ )
        {
            FeaTri* tri = new FeaTri;
            tri->Create( pvec[tvec[i].ind0], pvec[tvec[i].ind1], pvec[tvec[i].ind2] );
            tri->SetFeaPartIndex( m_SurfVec[s]->GetFeaPartIndex() );
            m_FeaElementVec.push_back( tri );
        }
    }
}

void FeaMeshMgrSingleton::ComputeWriteMass()
{
    m_TotalMass = 0.0;

    FILE* fp = fopen( GetStructSettingsPtr()->GetExportFileName( vsp::MASS_FILE_NAME ).c_str(), "w" );
    if ( fp )
    {
        fprintf( fp, "FeaStruct_Name: %s\n", m_FeaMeshStruct->GetFeaStructName().c_str() );

        int num_fea_parts = m_FeaMeshStruct->NumFeaParts();

        // Iterate over each FeaPart index and calculate mass of each FeaElement if the current indexes match
        for ( unsigned int i = 0; i < num_fea_parts; i++ )
        {
            double mass = 0;
            int property_id = m_FeaMeshStruct->GetFeaPropertyIndex( i );

            for ( int j = 0; j < m_FeaElementVec.size(); j++ )
            {
                if ( m_FeaElementVec[j]->GetFeaPartIndex() == i )
                {
                    mass += m_FeaElementVec[j]->ComputeMass( property_id );
                }
            }

            string name = m_FeaMeshStruct->GetFeaPartName( i );

            fprintf( fp, "\tFeaPartName: %s, Mass = %f\n", name.c_str(), mass );
            m_TotalMass += mass;
        }

        fprintf( fp, "Total Mass = %f\n", m_TotalMass );

        fclose( fp );
    }
}

FeaNode* FeaMeshMgrSingleton::FindNode( vector< FeaNode* > nodeVec, int id )
{
    //==== jrg Brute Force for Now ====//
    for ( int i = 0 ; i < ( int )nodeVec.size() ; i++ )
    {
        if ( nodeVec[i]->m_Index == id )
        {
            return nodeVec[i];
        }
    }

    return NULL;
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

    int num_fea_parts = m_FeaMeshStruct->NumFeaParts();

    for ( unsigned int i = 0; i < num_fea_parts; i++ )
    {
        vector< FeaNode* > temp_nVec;

        for ( int j = 0; j < m_FeaElementVec.size(); j++ )
        {
            if ( m_FeaElementVec[j]->GetFeaPartIndex() == i )
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
}

void FeaMeshMgrSingleton::WriteNASTRAN( const string &filename )
{
    FILE* fp = fopen( filename.c_str(), "w" );
    if ( fp )
    {
        fprintf( fp, "BEGIN BULK\n" );

        int elem_id = 0;
        int num_fea_parts = m_FeaMeshStruct->NumFeaParts();

        for ( unsigned int i = 0; i < num_fea_parts; i++ )
        {
            fprintf( fp, "\n" );
            fprintf( fp, "$%s\n", m_FeaMeshStruct->GetFeaPartName( i ).c_str() );

            int property_id = m_FeaMeshStruct->GetFeaPropertyIndex( i );

            for ( int j = 0; j < m_FeaElementVec.size(); j++ )
            {
                if ( m_FeaElementVec[j]->GetFeaPartIndex() == i )
                {
                    elem_id++;
                    m_FeaElementVec[j]->WriteNASTRAN( fp, elem_id, property_id );
                }
            }
        }

        for ( unsigned int i = 0; i < num_fea_parts; i++ )
        {
            fprintf( fp, "\n" );
            fprintf( fp, "$%s Gridpoints\n", m_FeaMeshStruct->GetFeaPartName( i ).c_str() );

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

        // TODO: Write and improve intersection elements/nodes

        fprintf( fp, "\n" );
        fprintf( fp, "$Intersections\n" );

        for ( unsigned int j = 0; j < (int)m_FeaNodeVec.size(); j++ )
        {
            if ( m_PntShift[j] >= 0 )
            {
                if ( m_FeaNodeVec[j]->m_Tags.size() > 1 )
                {
                    m_FeaNodeVec[j]->WriteNASTRAN( fp );
                }
            }
        }



        ////==== Write Rib Spar Intersections =====//
        //for ( int r = 0 ; r < m_SurfVec.size(); r++ )
        //{
        //    if ( m_SurfVec[r]->GetFeaPartType() == vsp::FEA_RIB )
        //    {
        //        for ( int s = 0; s < m_SurfVec.size(); s++ )
        //        {
        //            if ( m_SurfVec[s]->GetFeaPartType() == vsp::FEA_SPAR )
        //            {
        //                fprintf( fp, "\n" );
        //                fprintf( fp, "$Intersection,%d,%d\n", r, s );

        //                for ( int i = 0; i < (int)m_FeaNodeVec.size(); i++ )
        //                {
        //                    if ( m_PntShift[i] >= 0 )
        //                    {
        //                        if ( m_FeaNodeVec[i]->HasTag( vsp::FEA_RIB, r ) && m_FeaNodeVec[i]->HasTag( vsp::FEA_SPAR, s ) )
        //                        {
        //                            m_FeaNodeVec[i]->WriteNASTRAN( fp );
        //                        }
        //                    }
        //                }
        //                fprintf( fp, "\n" );
        //            }
        //        }
        //    }
        //}

        ////==== Write Out Rib LE/TE ====//
        //for ( int r = 0 ; r < rib_cnt ; r++ )
        //{
        //    vector< FeaNode* > letenodes;
        //    for ( int i = 0 ; i < ( int )nodeVec.size() ; i++ )
        //    {
        //        if ( nodeVec[i]->m_Tags.size() == 2 )
        //        {
        //            if ( nodeVec[i]->HasTag( RIB_LOWER, r + 1 ) && nodeVec[i]->HasTag( RIB_UPPER, r + 1 ) )
        //            {
        //                letenodes.push_back( nodeVec[i] );
        //            }
        //        }
        //    }
        //    if ( letenodes.size() == 2 )
        //    {
        //        if ( letenodes[1]->m_Pnt.x() < letenodes[0]->m_Pnt.x() )
        //        {
        //            FeaNode* temp = letenodes[0];
        //            letenodes[0]  = letenodes[1];
        //            letenodes[1]  = temp;
        //        }
        //        fprintf( fp, "\n" );
        //        fprintf( fp, "$RibLE,%d\n", r + 1 );
        //        letenodes[0]->WriteNASTRAN( fp );

        //        fprintf( fp, "\n" );
        //        fprintf( fp, "$RibTE,%d\n", r + 1 );
        //        letenodes[1]->WriteNASTRAN( fp );
        //    }
        //}

        ////==== Write Rib Upper Boundary =====//
        //for ( int r = 0 ; r < rib_cnt ; r++ )
        //{
        //    fprintf( fp, "\n" );
        //    fprintf( fp, "$RibUpperBoundary,%d\n", r + 1 );
        //    for ( int i = 0 ; i < ( int )nodeVec.size() ; i++ )
        //    {
        //        if ( nodeVec[i]->HasTag( RIB_UPPER, r + 1 ) && nodeVec[i]->m_Tags.size() == 1 )
        //        {
        //            nodeVec[i]->WriteNASTRAN( fp );
        //        }
        //    }
        //}
        ////==== Write Spar Upper Boundary =====//
        //for ( int s = 0 ; s < spar_cnt ; s++ )
        //{
        //    fprintf( fp, "\n" );
        //    fprintf( fp, "$SparUpperBoundary,%d\n", s + 1 );
        //    for ( int i = 0 ; i < ( int )nodeVec.size() ; i++ )
        //    {
        //        if ( nodeVec[i]->HasTag( SPAR_UPPER, s + 1 ) && nodeVec[i]->m_Tags.size() == 1 )
        //        {
        //            nodeVec[i]->WriteNASTRAN( fp );
        //        }
        //    }
        //}
        ////==== Write Rib Lower Boundary  =====//
        //for ( int r = 0 ; r < rib_cnt ; r++ )
        //{
        //    fprintf( fp, "\n" );
        //    fprintf( fp, "$RibLowerBoundary,%d\n", r + 1 );
        //    for ( int i = 0 ; i < ( int )nodeVec.size() ; i++ )
        //    {
        //        if ( nodeVec[i]->HasTag( RIB_LOWER, r + 1 ) && nodeVec[i]->m_Tags.size() == 1 )
        //        {
        //            nodeVec[i]->WriteNASTRAN( fp );
        //        }
        //    }
        //}
        ////==== Write Spar Lower Boundary =====//
        //for ( int s = 0 ; s < spar_cnt ; s++ )
        //{
        //    fprintf( fp, "\n" );
        //    fprintf( fp, "$SparLowerBoundary,%d\n", s + 1 );
        //    for ( int i = 0 ; i < ( int )nodeVec.size() ; i++ )
        //    {
        //        if ( nodeVec[i]->HasTag( SPAR_LOWER, s + 1 ) && nodeVec[i]->m_Tags.size() == 1 )
        //        {
        //            nodeVec[i]->WriteNASTRAN( fp );
        //        }
        //    }
        //}
        ////==== Write Point Masses =====//
        //for ( int p = 0 ; p < ( int )m_PointMassVec.size() ; p++ )
        //{
        //    //==== Snap To Nearest Attach Point ====//
        //    int close_ind  = 0;
        //    double close_d = 1.0e12;

        //    FeaNode node;
        //    node.m_Pnt = vec3d( m_PointMassVec[p]->m_PosX(), m_PointMassVec[p]->m_PosY(), m_PointMassVec[p]->m_PosZ() );
        //    node.m_Index = numPnts + p + 1;
        //    fprintf( fp, "\n" );
        //    fprintf( fp, "$Pointmass,%d\n", p + 1 );
        //    node.WriteNASTRAN( fp );

        //    //==== Find Attach Point Index ====//
        //    int ind = FindPntIndex( m_PointMassVec[p]->m_AttachPos, allPntVec, indMap );
        //    fprintf( fp, "$Connects,%d\n", pntShift[ind] + 1 );
        //}

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

        vector < FeaProperty* > property_vec = StructureMgr.GetFeaPropertyVec();

        for ( unsigned int i = 0; i < property_vec.size(); i++ )
        {
            property_vec[i]->WriteNASTRAN( fp, i + 1 );
        }

        //==== Materials ====//
        fprintf( fp, "\n" );
        fprintf( fp, "$Materials\n" );

        vector < FeaMaterial* > material_vec = StructureMgr.GetFeaMaterialVec();

        for ( unsigned int i = 0; i < material_vec.size(); i++ )
        {
            material_vec[i]->WriteNASTRAN( fp, i + 1 );
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

        int num_fea_parts = m_FeaMeshStruct->NumFeaParts();

        for ( unsigned int i = 0; i < num_fea_parts; i++ )
        {
            fprintf( fp, "$**%%%s\n", m_FeaMeshStruct->GetFeaPartName( i ).c_str() );
            fprintf( fp, "*NODE, NSET=N%s\n", m_FeaMeshStruct->GetFeaPartName( i ).c_str() );

            int property_id = m_FeaMeshStruct->GetFeaPropertyIndex( i );

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
            fprintf( fp, "*ELEMENT, TYPE=S6, ELSET=E%s\n", m_FeaMeshStruct->GetFeaPartName( i ).c_str() );

            for ( int j = 0; j < m_FeaElementVec.size(); j++ )
            {
                if ( m_FeaElementVec[j]->GetFeaPartIndex() == i )
                {
                    elem_id++;
                    m_FeaElementVec[j]->WriteCalculix( fp, elem_id );
                }
            }

            fprintf( fp, "\n" );
            sprintf( str, "E%s", m_FeaMeshStruct->GetFeaPartName( i ).c_str() );

            StructureMgr.GetFeaProperty( property_id )->WriteCalculix( fp, str );
            fprintf( fp, "\n" );
        }

        // TODO: Identify and improve intersection elements and nodes

        fprintf( fp, "**%%Intersections\n" );
        fprintf( fp, "*NODE, NSET=Nintersections\n" );

        for ( unsigned int j = 0; j < (int)m_FeaNodeVec.size(); j++ )
        {
            if ( m_PntShift[j] >= 0 )
            {
                if ( m_FeaNodeVec[j]->m_Tags.size() > 1 )
                {
                    m_FeaNodeVec[j]->WriteCalculix( fp );
                }
            }
        }







        ////==== Rib Spar Intersections ====//
        //for ( int r = 0 ; r < ( int )m_SurfVec.size(); r++ )
        //{
        //    if ( m_SurfVec[r]->GetFeaPartType() == vsp::FEA_RIB )
        //    {
        //        rib_cnt++;
        //        spar_cnt = 0;

        //        for ( int s = 0; s < (int)m_SurfVec.size(); s++ )
        //        {
        //            if ( m_SurfVec[s]->GetFeaPartType() == vsp::FEA_SPAR )
        //            {
        //                spar_cnt++;
        //                fprintf( fp, "\n" );
        //                fprintf( fp, "**%%Rib-Spar connections %d %d\n", rib_cnt, spar_cnt );
        //                fprintf( fp, "*NODE, NSET=Nconnections%d%d\n", rib_cnt, spar_cnt );
        //                for ( int i = 0; i < (int)m_FeaNodeVec.size(); i++ )
        //                {
        //                    if ( m_PntShift[i] >= 0 )
        //                    {
        //                        if ( m_FeaNodeVec[i]->HasTag( vsp::FEA_RIB, r ) && m_FeaNodeVec[i]->HasTag( vsp::FEA_SPAR, s ) )
        //                        {
        //                            //nodeVec[i]->m_Thick = 0.5 * ( ribs[r]->m_Thickness + spars[s]->m_Thickness );
        //                            m_FeaNodeVec[i]->WriteCalculix( fp );
        //                        }
        //                    }
        //                }
        //            }
        //        }
        //    }
        //}

        //==== Materials ====//
        fprintf( fp, "\n" );
        fprintf( fp, "$Materials\n" );

        vector < FeaMaterial* > material_vec = StructureMgr.GetFeaMaterialVec();

        for ( unsigned int i = 0; i < material_vec.size(); i++ )
        {
            material_vec[i]->WriteCalculix( fp, i );
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
        int num_fea_parts = m_FeaMeshStruct->NumFeaParts();

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
        fprintf( fp, "%d\n", num_fea_parts );
        for ( unsigned int i = 0; i < num_fea_parts; i++ )
        {
            fprintf( fp, "9 %d \"%s\"\n", i + 1, m_FeaMeshStruct->GetFeaPartName( i ).c_str() );
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

        for ( unsigned int j = 0; j < num_fea_parts; j++ )
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

void FeaMeshMgrSingleton::LoadDrawObjs( vector< DrawObj* > &draw_obj_vec )
{
    if ( GetStructSettingsPtr()->m_DrawMeshFlag() && !GetFeaMeshInProgress() )
    {
        // Render Tag Colors
        int num_tags = SubSurfaceMgr.GetNumTags();
        m_SSTagDO.resize( num_tags );
        map<int, DrawObj*> tag_dobj_map;
        map< std::vector<int>, int >::const_iterator mit;
        map< int, DrawObj* >::const_iterator dmit;
        map< std::vector<int>, int > tagMap = SubSurfaceMgr.GetSingleTagMap();
        int cnt = 0;

        // Calculate constants for color sequence.
        const int ncgrp = 6; // Number of basic colors
        const int ncstep = (int)ceil( (double)num_tags / (double)ncgrp );
        const double nctodeg = 360.0 / ( ncgrp*ncstep );

        char str[256];
        for ( mit = tagMap.begin(); mit != tagMap.end(); mit++ )
        {
            m_SSTagDO[cnt] = DrawObj();
            tag_dobj_map[mit->second] = &m_SSTagDO[cnt];
            sprintf( str, "%s_TAG_%d", GetID().c_str(), cnt );
            m_SSTagDO[cnt].m_GeomID = string( str );

            m_SSTagDO[cnt].m_Type = DrawObj::VSP_SHADED_TRIS;
            m_SSTagDO[cnt].m_Visible = false;
            if ( GetStructSettingsPtr()->m_DrawMeshFlag.Get() ||
                 GetStructSettingsPtr()->m_ColorTagsFlag.Get() )   // At least mesh or tags are visible.
            {
                m_SSTagDO[cnt].m_Visible = true;

                if ( GetStructSettingsPtr()->m_DrawMeshFlag.Get() &&
                     GetStructSettingsPtr()->m_ColorTagsFlag.Get() ) // Both are visible.
                {
                    m_SSTagDO[cnt].m_Type = DrawObj::VSP_HIDDEN_TRIS_CFD;
                    m_SSTagDO[cnt].m_LineColor = vec3d( 0.4, 0.4, 0.4 );
                }
                else if ( GetStructSettingsPtr()->m_DrawMeshFlag.Get() ) // Mesh only
                {
                    m_SSTagDO[cnt].m_Type = DrawObj::VSP_HIDDEN_TRIS_CFD;
                    m_SSTagDO[cnt].m_LineColor = vec3d( 0.4, 0.4, 0.4 );
                }
                else // Tags only
                {
                    m_SSTagDO[cnt].m_Type = DrawObj::VSP_SHADED_TRIS;
                }
            }

            if ( GetStructSettingsPtr()->m_ColorTagsFlag.Get() )
            {
                // Color sequence -- go around color wheel ncstep times with slight
                // offset from ncgrp basic colors.
                // Note, (cnt/ncgrp) uses integer division resulting in floor.
                double deg = ( ( cnt % ncgrp ) * ncstep + ( cnt / ncgrp ) ) * nctodeg;
                vec3d rgb = m_SSTagDO[cnt].ColorWheel( deg );
                rgb.normalize();

                for ( int i = 0; i < 3; i++ )
                {
                    m_SSTagDO[cnt].m_MaterialInfo.Ambient[i] = (float)rgb.v[i] / 5.0f;
                    m_SSTagDO[cnt].m_MaterialInfo.Diffuse[i] = 0.4f + (float)rgb.v[i] / 10.0f;
                    m_SSTagDO[cnt].m_MaterialInfo.Specular[i] = 0.04f + 0.7f * (float)rgb.v[i];
                    m_SSTagDO[cnt].m_MaterialInfo.Emission[i] = (float)rgb.v[i] / 20.0f;
                }
                m_SSTagDO[cnt].m_MaterialInfo.Ambient[3] = 1.0f;
                m_SSTagDO[cnt].m_MaterialInfo.Diffuse[3] = 1.0f;
                m_SSTagDO[cnt].m_MaterialInfo.Specular[3] = 1.0f;
                m_SSTagDO[cnt].m_MaterialInfo.Emission[3] = 1.0f;

                m_SSTagDO[cnt].m_MaterialInfo.Shininess = 32.0f;
            }
            else
            {
                // No color needed for mesh only.
            }

            draw_obj_vec.push_back( &m_SSTagDO[cnt] );
            cnt++;
        }

        for ( int i = 0; i < (int)m_SurfVec.size(); i++ )
        {
            vector < vec3d > pVec = m_SurfVec[i]->GetMesh()->GetSimpPntVec();
            vector < SimpTri > tVec = m_SurfVec[i]->GetMesh()->GetSimpTriVec();

            for ( int t = 0; t < (int)tVec.size(); t++ )
            {
                if ( ( !m_SurfVec[i]->GetWakeFlag() ) && ( !m_SurfVec[i]->GetFarFlag() ) && ( !m_SurfVec[i]->GetSymPlaneFlag() ) )
                {
                    SimpTri* stri = &tVec[t];
                    dmit = tag_dobj_map.find( SubSurfaceMgr.GetTag( stri->m_Tags ) );
                    if ( dmit == tag_dobj_map.end() )
                    {
                        continue;
                    }

                    DrawObj* obj = dmit->second;
                    vec3d norm = cross( pVec[stri->ind1] - pVec[stri->ind0], pVec[stri->ind2] - pVec[stri->ind0] );
                    norm.normalize();
                    obj->m_PntVec.push_back( pVec[stri->ind0] );
                    obj->m_PntVec.push_back( pVec[stri->ind1] );
                    obj->m_PntVec.push_back( pVec[stri->ind2] );
                    obj->m_NormVec.push_back( norm );
                    obj->m_NormVec.push_back( norm );
                    obj->m_NormVec.push_back( norm );
                }
            }
        }
    }
}
