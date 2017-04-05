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
    m_NumFeaParts = 0;
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

    m_NumFeaParts = 0;

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

    m_NumFeaParts = m_FeaMeshStruct->NumFeaParts();

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
    //m_OutStream.clear();
    m_FeaMeshInProgress = true;

    addOutputText( "Load Surfaces\n" );
    LoadSurfaces();

    // Hide all geoms after loading surfaces
    m_Vehicle->HideAll();

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
    for ( int i = 1; i < m_NumFeaParts; i++ ) // FeaSkin is index 0 and has been added already
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
    // Build FeaTris
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

    // Build FeaBeam Intersections
    list< ISegChain* >::iterator c;

    for ( c = m_ISegChainList.begin(); c != m_ISegChainList.end(); c++ )
    {
        if ( !( *c )->m_BorderFlag ) // Only include intersection curves
        {
            bool FeaPartCapA = false;
            bool FeaPartCapB = false;

            // Check at least one surface intersection cap flag is true
            if ( m_FeaMeshStruct->GetFeaPart( ( *c )->m_SurfA->GetFeaPartIndex() ) )
            {
                FeaPartCapA = m_FeaMeshStruct->GetFeaPart( ( *c )->m_SurfA->GetFeaPartIndex() )->m_IntersectionCapFlag();
            }
            if ( m_FeaMeshStruct->GetFeaPart( ( *c )->m_SurfB->GetFeaPartIndex() ) )
            {
                FeaPartCapB = m_FeaMeshStruct->GetFeaPart( ( *c )->m_SurfB->GetFeaPartIndex() )->m_IntersectionCapFlag();
            }

            // Check if one surface is a skin and one is an FeaPart (m_CompID = -9999)
            if ( ( FeaPartCapA || FeaPartCapB ) && ( ( ( *c )->m_SurfA->GetCompID() < 0 && ( *c )->m_SurfB->GetCompID() >= 0 ) || ( ( *c )->m_SurfB->GetCompID() < 0 && ( *c )->m_SurfA->GetCompID() >= 0 ) ) )
            {
                int FeaPartIndex = -1;
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

                vector< vec3d > ipntVec;
                vector< vec3d > inormVec;
                Surf* NormSurf;

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
                }

                // Check if the direction of ipntVec. Reverse point and norm vec order if negative
                double theta = signed_angle( ipntVec[0] - center, ipntVec.back() - center, center );
                if ( theta < 0 )
                {
                    reverse( inormVec.begin(), inormVec.end() );
                    reverse( ipntVec.begin(), ipntVec.end() );
                }

                // Define FeaBeam elements
                for ( int j = 1; j < (int)ipntVec.size(); j++ )
                {
                    FeaBeam* beam = new FeaBeam;
                    beam->Create( ipntVec[j - 1], ipntVec[j], inormVec[j - 1] );
                    beam->SetFeaPartIndex( FeaPartIndex );
                    m_FeaElementVec.push_back( beam );
                }
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
        fprintf( fp, "FeaStruct_Name: %s\n", m_FeaMeshStruct->GetFeaStructName().c_str() );

        // Iterate over each FeaPart index and calculate mass of each FeaElement if the current indexes match
        for ( unsigned int i = 0; i < m_NumFeaParts; i++ )
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

    for ( unsigned int i = 0; i < m_NumFeaParts; i++ )
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

        for ( unsigned int i = 0; i < m_NumFeaParts; i++ )
        {
            fprintf( fp, "\n" );
            fprintf( fp, "$%s\n", m_FeaMeshStruct->GetFeaPartName( i ).c_str() );

            int property_id = m_FeaMeshStruct->GetFeaPropertyIndex( i );
            int cap_property_id = m_FeaMeshStruct->GetCapFeaPropertyIndex( i );

            for ( int j = 0; j < m_FeaElementVec.size(); j++ )
            {
                if ( m_FeaElementVec[j]->GetFeaPartIndex() == i )
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

        for ( unsigned int i = 0; i < m_NumFeaParts; i++ )
        {
            fprintf( fp, "**%s\n", m_FeaMeshStruct->GetFeaPartName( i ).c_str() );
            fprintf( fp, "*NODE, NSET=N%s\n", m_FeaMeshStruct->GetFeaPartName( i ).c_str() );

            int property_id = m_FeaMeshStruct->GetFeaPropertyIndex( i );
            int cap_property_id = m_FeaMeshStruct->GetCapFeaPropertyIndex( i );

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
                if ( m_FeaElementVec[j]->GetFeaPartIndex() == i && m_FeaElementVec[j]->GetElementType() == FeaElement::FEA_TRI_6 )
                {
                    elem_id++;
                    m_FeaElementVec[j]->WriteCalculix( fp, elem_id );
                }
            }

            fprintf( fp, "\n" );
            sprintf( str, "E%s", m_FeaMeshStruct->GetFeaPartName( i ).c_str() );

            StructureMgr.GetFeaProperty( property_id )->WriteCalculix( fp, str );

            if ( m_FeaMeshStruct->GetFeaPart( i )->m_IntersectionCapFlag() )
            {
                fprintf( fp, "\n" );
                fprintf( fp, "*ELEMENT, TYPE=B31, ELSET=E%s_CAP\n", m_FeaMeshStruct->GetFeaPartName( i ).c_str() );

                for ( int j = 0; j < m_FeaElementVec.size(); j++ )
                {
                    if ( m_FeaElementVec[j]->GetFeaPartIndex() == i && m_FeaElementVec[j]->GetElementType() == FeaElement::FEA_BEAM )
                    {
                        elem_id++;
                        m_FeaElementVec[j]->WriteCalculix( fp, elem_id );
                    }
                }

                fprintf( fp, "\n" );
                sprintf( str, "E%s_CAP", m_FeaMeshStruct->GetFeaPartName( i ).c_str() );

                StructureMgr.GetFeaProperty( cap_property_id )->WriteCalculix( fp, str );
                fprintf( fp, "\n" );
            }
        }

        // TODO: Identify and improve intersection elements and nodes

        fprintf( fp, "**Intersections\n" );
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
        fprintf( fp, "Materials\n" );

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
    if ( !GetFeaMeshInProgress() )
    {
        // Render Tag Colors
        m_FeaNodeDO.resize( m_NumFeaParts );
        m_FeaElementDO.resize( m_NumFeaParts );
        m_CapFeaElementDO.resize( m_NumFeaParts );
        m_ElementNormDO.clear();

        // Calculate constants for color sequence.
        const int ncgrp = 6; // Number of basic colors
        const int ncstep = (int)ceil( (double)( 2 * m_NumFeaParts ) / (double)ncgrp );
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
            m_FeaNodeDO[cnt].m_PointSize = 3.0;
            m_FeaElementDO[cnt].m_Type = DrawObj::VSP_SHADED_TRIS;
            m_FeaElementDO[cnt].m_Visible = false;
            m_CapFeaElementDO[cnt].m_Type = DrawObj::VSP_LINES;
            m_CapFeaElementDO[cnt].m_Visible = false;
            m_CapFeaElementDO[cnt].m_LineWidth = 2.0;

            if ( GetStructSettingsPtr()->m_DrawMeshFlag.Get() ||
                 GetStructSettingsPtr()->m_ColorTagsFlag.Get() )   // At least mesh or tags are visible.
            {
                m_FeaElementDO[cnt].m_Visible = true;

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

            if ( GetStructSettingsPtr()->m_ColorTagsFlag.Get() )
            {
                // Color sequence -- go around color wheel ncstep times with slight
                // offset from ncgrp basic colors.
                // Note, (cnt/ncgrp) uses integer division resulting in floor.
                double deg = ( ( cnt % ncgrp ) * ncstep + ( cnt / ncgrp ) ) * nctodeg;
                double deg2 = ( ( ( m_NumFeaParts + cnt ) % ncgrp ) * ncstep + ( ( m_NumFeaParts + cnt ) / ncgrp ) ) * nctodeg;
                vec3d rgb = m_FeaElementDO[cnt].ColorWheel( deg );
                rgb.normalize();

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
                m_ElementNormDO[cnt].m_LineColor = vec3d( 0.0, 0.0, 0.0 );
            }

            draw_obj_vec.push_back( &m_FeaNodeDO[cnt] );
            draw_obj_vec.push_back( &m_FeaElementDO[cnt] );
            draw_obj_vec.push_back( &m_CapFeaElementDO[cnt] );
        }

        for ( unsigned int i = 0; i < m_NumFeaParts; i++ )
        {
            if ( GetStructSettingsPtr()->m_DrawNodesFlag() )
            {
                m_FeaNodeDO[i].m_Visible = true;

                for ( unsigned int j = 0; j < (int)m_FeaNodeVec.size(); j++ )
                {
                    if ( m_PntShift[j] >= 0 )
                    {
                        if ( m_FeaNodeVec[j]->HasOnlyIndex( i ) )
                        {
                            m_FeaNodeDO[i].m_PntVec.push_back( m_FeaNodeVec[j]->m_Pnt );
                        }
                    }
                }
            }

            if ( m_FeaMeshStruct->GetFeaPart( i )->m_DrawElementsFlag() ) // TODO: LoadDrawObj should not rely on pointers
            {
                for ( int j = 0; j < m_FeaElementVec.size(); j++ )
                {
                    if ( m_FeaElementVec[j]->GetFeaPartIndex() == i && m_FeaElementVec[j]->GetElementType() == FeaElement::FEA_TRI_6 )
                    {
                        vec3d norm = cross( m_FeaElementVec[j]->m_Corners[1]->m_Pnt - m_FeaElementVec[j]->m_Corners[0]->m_Pnt, m_FeaElementVec[j]->m_Corners[2]->m_Pnt - m_FeaElementVec[j]->m_Corners[0]->m_Pnt );
                        norm.normalize();
                        m_FeaElementDO[i].m_PntVec.push_back( m_FeaElementVec[j]->m_Corners[0]->m_Pnt );
                        m_FeaElementDO[i].m_PntVec.push_back( m_FeaElementVec[j]->m_Corners[1]->m_Pnt );
                        m_FeaElementDO[i].m_PntVec.push_back( m_FeaElementVec[j]->m_Corners[2]->m_Pnt );
                        m_FeaElementDO[i].m_NormVec.push_back( norm );
                        m_FeaElementDO[i].m_NormVec.push_back( norm );
                        m_FeaElementDO[i].m_NormVec.push_back( norm );
                    }
                }
            }

            if ( m_FeaMeshStruct->GetFeaPart( i )->m_DrawCapElementsFlag() ) // TODO: LoadDrawObj should not rely on pointers
            {
                m_CapFeaElementDO[i].m_Visible = true;

                for ( int j = 0; j < m_FeaElementVec.size(); j++ )
                {
                    if ( m_FeaElementVec[j]->GetFeaPartIndex() == i && m_FeaElementVec[j]->GetElementType() == FeaElement::FEA_BEAM )
                    {
                        m_CapFeaElementDO[i].m_PntVec.push_back( m_FeaElementVec[j]->m_Corners[0]->m_Pnt );
                        m_CapFeaElementDO[i].m_PntVec.push_back( m_FeaElementVec[j]->m_Corners[1]->m_Pnt );

                        // Normal Vec is not required, load placeholder
                        m_CapFeaElementDO[i].m_NormVec.push_back( m_FeaElementVec[j]->m_Corners[0]->m_Pnt );
                        m_CapFeaElementDO[i].m_NormVec.push_back( m_FeaElementVec[j]->m_Corners[1]->m_Pnt );
                    }
                }
            }
        }

        int beam_cnt = 0;
        double line_length = 0.075;

        if ( GetStructSettingsPtr()->m_DrawElementNormsFlag() )
        {
            for ( unsigned int j = 0; j < m_FeaElementVec.size(); j++ )
            {
                if ( m_FeaElementVec[j]->GetElementType() == FeaElement::FEA_BEAM )
                {
                    DrawObj obj = DrawObj();

                    sprintf( str, "%s_Beam_Norm_%d", GetID().c_str(), beam_cnt );
                    obj.m_GeomID = string( str );

                    obj.m_Type = DrawObj::VSP_LINES;
                    obj.m_Visible = false;
                    obj.m_LineWidth = 1.0;

                    // Set line color and visibility the same as the FeaBeam cap DrawObj
                    obj.m_LineColor = m_CapFeaElementDO[m_FeaElementVec[j]->GetFeaPartIndex()].m_LineColor;
                    obj.m_Visible = m_CapFeaElementDO[m_FeaElementVec[j]->GetFeaPartIndex()].m_Visible;

                    FeaBeam* beam = dynamic_cast<FeaBeam*>( m_FeaElementVec[j] );
                    assert( beam );

                    vec3d norm_pnt = m_FeaElementVec[j]->m_Corners[0]->m_Pnt + line_length * beam->m_DispVec;

                    obj.m_PntVec.push_back( m_FeaElementVec[j]->m_Corners[0]->m_Pnt );
                    obj.m_PntVec.push_back( norm_pnt );

                    obj.m_NormVec.push_back( m_FeaElementVec[j]->m_Corners[0]->m_Pnt );
                    obj.m_NormVec.push_back( norm_pnt );

                    m_ElementNormDO.push_back( obj );

                    beam_cnt++;
                }
            }

            for ( unsigned int k = 0; k < m_ElementNormDO.size(); k++ )
            {
                draw_obj_vec.push_back( &m_ElementNormDO[k] );
            }
        }
    }
}