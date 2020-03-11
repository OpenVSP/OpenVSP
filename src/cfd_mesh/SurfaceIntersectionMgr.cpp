//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//

// SurfaceIntersectionMgr
//
//////////////////////////////////////////////////////////////////////

#include "SurfaceIntersectionMgr.h"
#include "Util.h"
#include "SubSurfaceMgr.h"
#include "main.h"

#include <chrono>

#ifdef DEBUG_CFD_MESH
#include <direct.h>
#endif


//=============================================================//
//=============================================================//


SurfaceIntersectionSingleton::SurfaceIntersectionSingleton() : ParmContainer()
{
    m_Vehicle = VehicleMgr.GetVehicle();

    m_HighlightChainIndex = 0;

    // Array allocated to (m_NumComps + 6) later, so if this isn't reset by then, the
    // allocation will fail with a negative argument.
    m_NumComps = -10;

    m_MeshInProgress = false;

    m_MessageName = "SurfIntersectMessage";

#ifdef DEBUG_CFD_MESH
    m_DebugDir  = Stringc( "MeshDebug/" );
    _mkdir( m_DebugDir.get_char_star() );
    m_DebugFile = fopen( "MeshDebug/log.txt", "w" );
    m_DebugDraw = false;
#endif

}

SurfaceIntersectionSingleton::~SurfaceIntersectionSingleton()
{
    CleanUp();

#ifdef DEBUG_CFD_MESH
    if ( m_DebugFile )
    {
        fclose( m_DebugFile );
    }
#endif

}

//==== Parm Changed ====//
void SurfaceIntersectionSingleton::ParmChanged( Parm* parm_ptr, int type )
{
    if ( m_Vehicle )
    {
        m_Vehicle->ParmChanged( parm_ptr, Parm::SET );
    }
}

void SurfaceIntersectionSingleton::IntersectSurfaces()
{
    m_MeshInProgress = true;

    TransferMeshSettings();

    addOutputText( "Fetching Bezier Surfaces\n" );

    vector< XferSurf > xfersurfs;
    FetchSurfs( xfersurfs );

    // Hide all geoms after fetching their surfaces
    m_Vehicle->HideAll();

    CleanUp();
    addOutputText( "Loading Bezier Surfaces\n" );
    LoadSurfs( xfersurfs );

    TransferSubSurfData();

    CleanMergeSurfs();

    if ( m_SurfVec.size() == 0 )
    {
        addOutputText( "No Surfaces To Mesh\n" );
        m_MeshInProgress = false;
        return;
    }

    addOutputText( "Build Grid\n" );
    BuildGrid();

//    auto t1 = std::chrono::high_resolution_clock::now();

    addOutputText( "Intersect\n" );
    Intersect();
    addOutputText( "Finished Intersect\n" );

//    auto t2 = std::chrono::high_resolution_clock::now();
//    printf( "Intersect took %lld mus %f ms %f sec\n", std::chrono::duration_cast<std::chrono::microseconds>(t2-t1).count(), std::chrono::duration_cast<std::chrono::microseconds>(t2-t1).count()/1000.0, std::chrono::duration_cast<std::chrono::microseconds>(t2-t1).count()/1000000.0 );

    addOutputText( "Binary Adaptation Curve Approximation\n" );
    BinaryAdaptIntCurves();

    addOutputText( "Exporting Files\n" );
    ExportFiles();

    addOutputText( "Done\n" );

    m_MeshInProgress = false;
}

void SurfaceIntersectionSingleton::CleanUp()
{
    int i;
    //==== Delete Old Surfs ====//
    for ( i = 0 ; i < ( int )m_SurfVec.size() ; i++ )
    {
        delete m_SurfVec[i];
    }
    m_SurfVec.clear();

    //==== Delete ICurves =====//
    for ( i = 0 ; i < ( int )m_ICurveVec.size() ; i++ )
    {
        delete m_ICurveVec[i];
    }
    m_ICurveVec.clear();

    //==== Delete Seg Chains ====//
    list< ISegChain* >::iterator cl;
    for ( cl = m_ISegChainList.begin() ; cl != m_ISegChainList.end(); ++cl )
    {
        delete ( *cl );
    }
    m_ISegChainList.clear();

    //==== Delete Puws ====//
    for ( i = 0 ; i < ( int )m_DelPuwVec.size() ; i++ )
    {
        delete m_DelPuwVec[i];
    }
    m_DelPuwVec.clear();

    //==== Delete IPnts ====//
    for ( i = 0 ; i < ( int )m_DelIPntVec.size() ; i++ )
    {
        delete m_DelIPntVec[i];
    }
    m_DelIPntVec.clear();

    //==== Delete IPntGroups ====//
    for ( i = 0 ; i < ( int )m_DelIPntGroupVec.size() ; i++ )
    {
        delete m_DelIPntGroupVec[i];
    }
    m_DelIPntGroupVec.clear();

    //==== Delete IPntGroups ====//
    for ( i = 0 ; i < ( int )m_DelISegChainVec.size() ; i++ )
    {
        delete m_DelISegChainVec[i];
    }
    m_DelISegChainVec.clear();

    m_BinMap.clear();
    m_PossCoPlanarSurfMap.clear();

    debugPnts.clear();

    m_SimpleSubSurfaceVec.clear();
}

void SurfaceIntersectionSingleton::TransferMeshSettings()
{
    m_IntersectSettings = SimpleIntersectSettings();
    m_IntersectSettings.CopyFrom( m_Vehicle->GetISectSettingsPtr() );

    m_GeomNameMap.clear();

    // Copy over Geom IDs and names
    vector< Geom* > geom_vec = m_Vehicle->FindGeomVec( m_Vehicle->GetGeomVec() );

    for ( size_t i = 0; i < geom_vec.size(); i++ )
    {
        m_GeomNameMap[geom_vec[i]->GetID()] = geom_vec[i]->GetName();
    }
}

void SurfaceIntersectionSingleton::TransferSubSurfData()
{
    vector < SubSurface* > ss_vec = SubSurfaceMgr.GetSubSurfs();
    m_SimpleSubSurfaceVec.resize( ss_vec.size() );

    for ( size_t i = 0; i < ss_vec.size(); i++ )
    {
        m_SimpleSubSurfaceVec[i] = SimpleSubSurface();
        m_SimpleSubSurfaceVec[i].CopyFrom( ss_vec[i] );
    }
}

//==== Get vector of SimpleSubSurfaces from geom by ID and surf number ====//
vector< SimpleSubSurface > SurfaceIntersectionSingleton::GetSimpSubSurfs( string geom_id, int surfnum, int comp_id )
{
    vector< SimpleSubSurface > ret_vec;

    // m_CompID < 0 indicates an FeaPart surface. SubSurfaces on FeaParts is not supported at this time 
    if ( comp_id < 0 )
    {
        return ret_vec;
    }

    Geom* geom = m_Vehicle->FindGeom( geom_id );
    if ( !geom )
    {
        return ret_vec;
    }

    if ( geom->GetType().m_Type == MESH_GEOM_TYPE ||
         geom->GetType().m_Type == HUMAN_GEOM_TYPE )
    {
        return ret_vec;
    }

    int imain = geom->GetMainSurfID( surfnum );

    if ( imain < 0 )
    {
        return ret_vec;
    }

    for ( int i = 0; i < m_SimpleSubSurfaceVec.size(); i++ )
    {
        if ( imain == m_SimpleSubSurfaceVec[i].m_MainSurfIndx && strcmp( m_SimpleSubSurfaceVec[i].GetCompID().c_str(), geom_id.c_str() ) == 0 )
        {
            ret_vec.push_back( m_SimpleSubSurfaceVec[i] );
        }
    }

    return ret_vec;
}

void SurfaceIntersectionSingleton::addOutputText( const string &str, int output_type )
{
    if ( output_type != QUIET_OUTPUT )
    {

        MessageData data;
        data.m_String = m_MessageName;
        data.m_StringVec.push_back( str );
        MessageMgr::getInstance().Send( "ScreenMgr", NULL, data );
    }
}

void SurfaceIntersectionSingleton::FetchSurfs( vector< XferSurf > &xfersurfs )
{
    m_Vehicle->FetchXFerSurfs( GetSettingsPtr()->m_SelectedSetIndex, xfersurfs );
}

void SurfaceIntersectionSingleton::LoadSurfs( vector< XferSurf > &xfersurfs, int start_surf_id )
{
    int maxcompid = -1;
    for ( int i = 0; i < xfersurfs.size(); i++ )
    {
        Surf* surfPtr = new Surf();
        surfPtr->GetSurfCore()->SetSurf( xfersurfs[i].m_Surface );

        surfPtr->SetGeomID( xfersurfs[i].m_GeomID );

        surfPtr->SetMainSurfID( xfersurfs[i].m_SurfIndx );

        //Xfersurfs created each time and flips normal if geom is a negative volume
        if ( xfersurfs[i].m_SurfCfdType == vsp::CFD_NEGATIVE )
        {
            xfersurfs[i].m_FlipNormal = !xfersurfs[i].m_FlipNormal;
        }

        surfPtr->SetFlipFlag( xfersurfs[i].m_FlipNormal );

        //Sets whether NORMAL, NEGATIVE, TRANSPARENT
        surfPtr->SetSurfaceCfdType(xfersurfs[i].m_SurfCfdType);

        int cid = xfersurfs[i].m_CompIndx;

        if ( cid > maxcompid )
            maxcompid = cid;

        surfPtr->SetCompID( cid );
        surfPtr->SetUnmergedCompID( cid );
        surfPtr->SetSurfID( start_surf_id + i );
        surfPtr->GetSurfCore()->BuildPatches( surfPtr );
        m_SurfVec.push_back( surfPtr );
    }

    // Don't include FEA parts (with negative Comp index)
    if ( maxcompid >= 0 )
    {
        m_NumComps = maxcompid + 1;
    }
}

void SurfaceIntersectionSingleton::CleanMergeSurfs()
{

    vector < Surf* > surfs = m_SurfVec;
    m_SurfVec.clear();

    for ( int s = 0 ; s < ( int )surfs.size(); s++ )
    {

        Surf* surfPtr = surfs[s];

        bool addSurfFlag = true;

        if ( surfPtr->GetCompID() >= 0 )
        {
            if ( GetSettingsPtr()->m_HalfMeshFlag && surfPtr->GetSurfCore()->LessThanY( 1e-6 ) )
            {
                addSurfFlag = false;
            }
        }
        else if ( surfPtr->GetCompID() < 0 ) // Indicates FEA Part Surface (Increase Tolerance)
        {
            if ( GetSettingsPtr()->m_HalfMeshFlag && surfPtr->GetSurfCore()->LessThanY( 1e-3 ) )
            {
                addSurfFlag = false;
            }
        }

        if ( GetSettingsPtr()->m_HalfMeshFlag && surfPtr->GetSurfCore()->PlaneAtYZero() )
        {
            addSurfFlag = false;
        }

        if ( addSurfFlag )
        {
            m_SurfVec.push_back( surfPtr );
        }
        else
        {
            delete surfPtr;
        }
    }

    DeleteDuplicateSurfs();

    //==== Combine Components With Matching Surface Edges ====//
    map< int, int > mergeCompMap;
    for ( int s = 0 ; s < ( int )m_SurfVec.size() - 1 ; s++ )
    {
        for ( int t = s + 1 ; t < ( int )m_SurfVec.size() ; t++ )
        {
            int compA = m_SurfVec[s]->GetCompID();
            int compB = m_SurfVec[t]->GetCompID();
            if ( compA != compB && m_SurfVec[s]->BorderMatch( m_SurfVec[t] ) )
            {
                mergeCompMap[compB] = compA;
            }
        }
    }
    //==== Change Comp IDs ====//
    for ( int s = 0 ; s < ( int )m_SurfVec.size() ; s++ )
    {
        int compID = m_SurfVec[s]->GetCompID();
        if ( mergeCompMap.find( compID ) != mergeCompMap.end() )
        {
            int newCompID = mergeCompMap[compID];

            // Keep looking through the merge map until we get to the root component
            while ( mergeCompMap.find( newCompID ) != mergeCompMap.end() )
            {
                newCompID = mergeCompMap[newCompID];
            }

            m_SurfVec[s]->SetCompID( newCompID );
        }
    }
}

void SurfaceIntersectionSingleton::DeleteDuplicateSurfs()
{
    unsigned int nsurf = m_SurfVec.size();

    vector < bool > delflag( nsurf );
    for ( int i = 0 ; i < nsurf ; i++ )
    {
        delflag[i] = false;
    }

    for ( int s = 0 ; s < nsurf - 1 ; s++ )
    {
        for ( int t = s + 1 ; t < nsurf ; t++ )
        {
            if ( m_SurfVec[s]->GetSurfCore()->SurfMatch( m_SurfVec[t]->GetSurfCore() ) )
            {
                delflag[s] = true;
                delflag[t] = true;
            }
        }
    }

    vector < Surf* > keepSurf;
    for ( int i = 0 ; i < nsurf ; i++ )
    {
        if ( delflag[i] )
        {
            delete m_SurfVec[i];
        }
        else
        {
            keepSurf.push_back( m_SurfVec[i] );
        }
    }

    m_SurfVec = keepSurf;
}

void SurfaceIntersectionSingleton::BuildGrid()
{

    int i, j;
    vector< SCurve* > scurve_vec;
    for ( i = 0 ; i < ( int )m_SurfVec.size() ; i++ )
    {
        m_SurfVec[i]->FindBorderCurves();
        m_SurfVec[i]->LoadSCurves( scurve_vec );
    }

    for ( i = 0 ; i < ( int )scurve_vec.size() ; i++ )
    {
        for ( j = i + 1 ; j < ( int )scurve_vec.size() ; j++ )
        {
            ICurve* icrv = new ICurve;
            if ( icrv->Match( scurve_vec[i], scurve_vec[j] ) )
            {
                m_ICurveVec.push_back( icrv );
            }
            else
            {
                delete icrv;
            }
        }
    }


    //==== Check For SCurves Not Matched ====//
    int num_unmatched = 0;
    for ( i = 0 ; i < ( int )scurve_vec.size() ; i++ )
    {
        if ( !scurve_vec[i]->GetICurve() )
        {
            ICurve* icrv = new ICurve;
            icrv->SetACurve( scurve_vec[i] );
            m_ICurveVec.push_back( icrv );
            num_unmatched++;
        }
    }

#ifdef DEBUG_CFD_MESH
    fprintf( m_DebugFile, "CfdMeshMgr::BuildGrid \n" );
    fprintf( m_DebugFile, "  Num unmatched SCurves = %d \n", num_unmatched );

    for ( i = 0 ; i < ( int )m_ICurveVec.size() ; i++ )
    {
        m_ICurveVec[i]->DebugEdgeMatching( m_DebugFile );
    }

#endif
}

void SurfaceIntersectionSingleton::ExportFiles()
{
    if ( GetIntersectSettingsPtr()->GetExportFileFlag( vsp::INTERSECT_SRF_FILE_NAME ) )
    {
        WriteSurfsIntCurves( GetIntersectSettingsPtr()->GetExportFileName( vsp::INTERSECT_SRF_FILE_NAME ) );
    }

    if ( GetIntersectSettingsPtr()->GetExportFileFlag( vsp::INTERSECT_CURV_FILE_NAME ) )
    {
        WriteGridToolCurvFile( GetIntersectSettingsPtr()->GetExportFileName( vsp::INTERSECT_CURV_FILE_NAME ),
                               GetIntersectSettingsPtr()->m_ExportRawFlag );
    }

    if ( GetIntersectSettingsPtr()->GetExportFileFlag( vsp::INTERSECT_PLOT3D_FILE_NAME ) )
    {
        WritePlot3DFile( GetIntersectSettingsPtr()->GetExportFileName( vsp::INTERSECT_PLOT3D_FILE_NAME ),
                         GetIntersectSettingsPtr()->m_ExportRawFlag );
    }

}

void SurfaceIntersectionSingleton::WriteSurfsIntCurves( const string &filename )
{
    FILE* fp = fopen( filename.c_str(), "w" );
    if ( fp )
    {
        map< int, vector< int > > compMap;
        for ( int i = 0 ; i < ( int )m_SurfVec.size() ; i ++ )
        {
            int surfId = m_SurfVec[i]->GetSurfID();
            int compId = m_SurfVec[i]->GetCompID();

            compMap[compId].push_back( surfId );
        }

        fprintf( fp, "BEGIN Component_List\n" );

        fprintf( fp, "%d		// Number of Components \n", ( int )compMap.size() );

        map< int, vector< int > > :: iterator iter;

        for ( iter = compMap.begin() ; iter != compMap.end() ; ++iter )
        {
            int compId = iter->first;
            vector< int > idVec = iter->second;
            fprintf( fp, "BEGIN Component\n" );
            fprintf( fp, "%d		// Comp ID \n",       compId );
            fprintf( fp, "%d		// Num Surfs \n", ( int )idVec.size() );
            for ( int i = 0 ; i < ( int )idVec.size() ; i++ )
            {
                fprintf( fp, "%d		// Surf ID \n",   idVec[i] );
            }
            fprintf( fp, "END Component\n" );
        }
        fprintf( fp, "END Component_List\n" );

        fprintf( fp, "BEGIN Surface_List\n" );
        fprintf( fp, "%d		// Number of Cubic Bezier Surfaces \n", ( int )m_SurfVec.size() );
        for ( int s = 0 ; s < ( int )m_SurfVec.size() ; s++ )
        {
            fprintf( fp, "BEGIN Surface\n" );
            Surf* surfPtr = m_SurfVec[s];
            fprintf( fp, "%d		// Surface ID \n",    surfPtr->GetSurfID() );
            fprintf( fp, "%d		// Comp ID \n",       surfPtr->GetCompID() );

            surfPtr->GetSurfCore()->WriteSurf( fp );

            fprintf( fp, "END Surface\n" );
        }
        fprintf( fp, "END Surface_List\n" );


        vector< ISegChain* > border_curves;
        vector< ISegChain* > intersect_curves;
        list< ISegChain* >::iterator c;
        for ( c = m_ISegChainList.begin() ; c != m_ISegChainList.end(); ++c )
        {
            if ( ( *c )->m_BorderFlag )
            {
                border_curves.push_back( ( *c ) );
            }
            else
            {
                intersect_curves.push_back( ( *c ) );
            }
        }

        fprintf( fp, "BEGIN Border_Curve_List\n" );

        fprintf( fp, "%d		// Number of Border_Curves \n", ( int )border_curves.size() );

        for ( int i = 0 ; i < ( int )border_curves.size() ; i++ )
        {
            fprintf( fp, "BEGIN Border_Curve\n" );

            Surf* surfA =  border_curves[i]->m_SurfA;
            Surf* surfB =  border_curves[i]->m_SurfB;

            fprintf( fp, "%d		// Surface A ID \n", surfA->GetSurfID() );
            fprintf( fp, "%d		// Surface B ID \n", surfB->GetSurfID() );

            vector< IPnt* > ipntVec;

            for ( int j = 0 ; j < ( int )border_curves[i]->m_ISegDeque.size() ; j++ )
            {
                ipntVec.push_back( border_curves[i]->m_ISegDeque[j]->m_IPnt[0] );
            }
            ipntVec.push_back( border_curves[i]->m_ISegDeque.back()->m_IPnt[1] );

            if ( ! GetSettingsPtr()->m_XYZIntCurveFlag )
            {
                fprintf( fp, "%d		// Number of Border Points (Au, Aw, Bu, Bw) \n", ( int )ipntVec.size() );
            }
            else
            {
                fprintf( fp, "%d		// Number of Border Points (Au, Aw, Bu, Bw, X, Y, Z) \n", ( int )ipntVec.size() );
            }

            for ( int j = 0 ; j < ( int )ipntVec.size() ; j++ )
            {
                Puw* pwA = ipntVec[j]->GetPuw( surfA );
                Puw* pwB = ipntVec[j]->GetPuw( surfB );

                if ( ! GetSettingsPtr()->m_XYZIntCurveFlag )
                {
                    fprintf( fp, "%d    %16.16lf, %16.16lf, %16.16lf, %16.16lf \n", j,
                             pwA->m_UW.x(), pwA->m_UW.y(), pwB->m_UW.x(), pwB->m_UW.y() );
                }
                else
                {
                    vec3d pA = surfA->CompPnt( pwA->m_UW.x(), pwA->m_UW.y() );
                    vec3d pB = surfB->CompPnt( pwB->m_UW.x(), pwB->m_UW.y() );
                    vec3d p = ( pA + pB ) * 0.5;

                    fprintf( fp, "%d    %16.16lf, %16.16lf, %16.16lf, %16.16lf, %16.16lf, %16.16lf, %16.16lf \n", j,
                             pwA->m_UW.x(), pwA->m_UW.y(), pwB->m_UW.x(), pwB->m_UW.y(),
                             p.x(), p.y(), p.z() );
                }
            }
            fprintf( fp, "END Border_Curve\n" );
        }
        fprintf( fp, "END Border_Curve_List\n" );


        fprintf( fp, "BEGIN IntersectCurve_List\n" );

        fprintf( fp, "%d		// Number of Intersect_Curves \n", ( int )intersect_curves.size() );

        for ( int i = 0 ; i < ( int )intersect_curves.size() ; i++ )
        {
            fprintf( fp, "BEGIN Intersect_Curve\n" );

            Surf* surfA =  intersect_curves[i]->m_SurfA;
            Surf* surfB =  intersect_curves[i]->m_SurfB;

            fprintf( fp, "%d		// Surface A ID \n", surfA->GetSurfID() );
            fprintf( fp, "%d		// Surface B ID \n", surfB->GetSurfID() );

            vector< IPnt* > ipntVec;

            for ( int j = 0 ; j < ( int )intersect_curves[i]->m_ISegDeque.size() ; j++ )
            {
                ipntVec.push_back( intersect_curves[i]->m_ISegDeque[j]->m_IPnt[0] );
            }
            ipntVec.push_back( intersect_curves[i]->m_ISegDeque.back()->m_IPnt[1] );

            if ( ! GetSettingsPtr()->m_XYZIntCurveFlag )
            {
                fprintf( fp, "%d		// Number of Intersect Points (Au, Aw, Bu, Bw) \n", ( int )ipntVec.size() );
            }
            else
            {
                fprintf( fp, "%d		// Number of Intersect Points (Au, Aw, Bu, Bw, X, Y, Z) \n", ( int )ipntVec.size() );
            }

            for ( int j = 0 ; j < ( int )ipntVec.size() ; j++ )
            {
                Puw* pwA = ipntVec[j]->GetPuw( surfA );
                Puw* pwB = ipntVec[j]->GetPuw( surfB );

                if ( ! GetSettingsPtr()->m_XYZIntCurveFlag )
                {
                    fprintf( fp, "%d    %16.16lf, %16.16lf, %16.16lf, %16.16lf \n", j,
                             pwA->m_UW.x(), pwA->m_UW.y(), pwB->m_UW.x(), pwB->m_UW.y() );
                }
                else
                {
                    vec3d pA = surfA->CompPnt( pwA->m_UW.x(), pwA->m_UW.y() );
                    vec3d pB = surfB->CompPnt( pwB->m_UW.x(), pwB->m_UW.y() );
                    vec3d p = ( pA + pB ) * 0.5;

                    fprintf( fp, "%d    %16.16lf, %16.16lf, %16.16lf, %16.16lf, %16.16lf, %16.16lf, %16.16lf \n", j,
                             pwA->m_UW.x(), pwA->m_UW.y(), pwB->m_UW.x(), pwB->m_UW.y(),
                             p.x(), p.y(), p.z() );
                }
            }
            fprintf( fp, "END Intersect_Curve\n" );
        }


        fprintf( fp, "END IntersectCurve_List\n" );


        fclose( fp );
    }

}

void SurfaceIntersectionSingleton::WriteGridToolCurvFile( const string &filename, bool rawflag )
{
    FILE* fp = fopen( filename.c_str(), "w" );
    if ( fp )
    {
        int ncurve = 0;
        if ( rawflag )
        {
            ncurve = m_RawCurveAVec.size();
        }
        else
        {
            ncurve = m_BinAdaptCurveAVec.size();
        }

        int indx = 0;
        for ( indx = 0; indx < ncurve; indx++ )
        {
            // Assume A and B curves are coincident -- just print A curve.
            vector<vec3d> ptvec;

            if ( rawflag )
            {
                ptvec = m_RawCurveAVec[ indx ];
            }
            else
            {
                ptvec = m_BinAdaptCurveAVec[ indx ];
            }

            fprintf( fp, "%zu\n", ptvec.size() );

            for ( int i = 0; i < ptvec.size(); i++ )
            {
                vec3d pt = ptvec[i];
                fprintf( fp, "%21.15e %21.15e %21.15e\n", pt.x(), pt.y(), pt.z() );
            }

        }
        fclose( fp );
    }
}

void SurfaceIntersectionSingleton::WritePlot3DFile( const string &filename, bool rawflag )
{
    FILE* fp = fopen( filename.c_str(), "w" );
    if ( fp )
    {
        int nchain = 0;
        vector < vector < vec3d > > *allpts;

        // Assume A and B curves are coincident -- just print A curve.
        if ( rawflag )
        {
            nchain = m_RawCurveAVec.size();
            allpts = &m_RawCurveAVec;
        }
        else
        {
            nchain = m_BinAdaptCurveAVec.size();
            allpts = &m_BinAdaptCurveAVec;
        }

        fprintf( fp, " %d\n", nchain );

        int ichain = 0;
        for ( ichain = 0; ichain < nchain; ichain++ )
        {
            fprintf( fp, " %d 1 1\n", (*allpts)[ichain].size() );
        }

        for ( ichain = 0; ichain < nchain; ichain++ )
        {
            for ( int i = 0; i < (*allpts)[ichain].size(); i++ )
            {
                vec3d pt = (*allpts)[ichain][i];
                fprintf( fp, "%25.17e ", pt.x() );
            }
            fprintf( fp, "\n" );

            for ( int i = 0; i < (*allpts)[ichain].size(); i++ )
            {
                vec3d pt = (*allpts)[ichain][i];
                fprintf( fp, "%25.17e ", pt.y() );
            }
            fprintf( fp, "\n" );

            for ( int i = 0; i < (*allpts)[ichain].size(); i++ )
            {
                vec3d pt = (*allpts)[ichain][i];
                fprintf( fp, "%25.17e ", pt.z() );
            }
            fprintf( fp, "\n" );
        }

        fclose( fp );
    }
}

void SurfaceIntersectionSingleton::WriteIGESFile( const string& filename, int len_unit,
                                                  bool label_id, bool label_surf_num, bool label_split_num,
                                                  bool label_name, string label_delim )
{
    IGESutil iges( len_unit );

    BuildNURBSCurvesVec(); // Note: Must be called before BuildNURBSSurfMap

    BuildNURBSSurfMap();

    for ( size_t si = 0; si < m_NURBSSurfVec.size(); si++ )
    {
        if ( m_NURBSSurfVec[si].m_NURBSLoopVec.size() == 1 &&
                  m_NURBSSurfVec[si].m_NURBSLoopVec[0].m_BorderLoopFlag &&
                  m_NURBSSurfVec[si].m_NURBSLoopVec[0].m_InternalLoopFlag )
        {
            continue; // Indicates that the surface is completely enclosed
        }

        string label;

        if ( label_id )
        {
            label = m_SurfVec[m_NURBSSurfVec[si].m_SurfID]->GetGeomID();
        }

        if ( label_name )
        {
            if ( label.size() > 0 )
            {
                label.append( label_delim );
            }
            label.append( m_GeomNameMap[m_SurfVec[m_NURBSSurfVec[si].m_SurfID]->GetGeomID()] );
        }

        if ( label_surf_num )
        {
            if ( label.size() > 0 )
            {
                label.append( label_delim );
            }
            label.append( to_string( m_SurfVec[m_NURBSSurfVec[si].m_SurfID]->GetMainSurfID() ) );
        }

        if ( label_split_num )
        {
            if ( label.size() > 0 )
            {
                label.append( label_delim );
            }
            label.append( to_string( m_NURBSSurfVec[si].m_SurfID ) );
        }

        DLL_IGES_ENTITY_128 isurf = m_NURBSSurfVec[si].WriteIGESSurf( &iges, label.c_str() );
        
        m_NURBSSurfVec[si].WriteIGESLoops( &iges, isurf );
    }

    iges.WriteFile( filename, true );
}

void SurfaceIntersectionSingleton::WriteSTEPFile( const string& filename, int len_unit, double tol,
                                                  bool merge_pnts, bool label_id, bool label_surf_num, bool label_split_num,
                                                  bool label_name, string label_delim, int representation  )
{
    STEPutil step( len_unit, tol );

    // Identify the unique sets of intersected components
    vector < vector < int > > comp_id_group_vec = GetCompIDGroupVec();

    BuildNURBSCurvesVec(); // Note: Must be called before BuildNURBSSurfMap

    // Identify the SdaiB_spline_curve_with_knots
    for ( size_t i = 0; i < m_NURBSCurveVec.size(); i++ )
    {
        if ( !m_NURBSCurveVec[i].m_SubSurfFlag && !m_NURBSCurveVec[i].m_StructIntersectFlag )
        {
            m_NURBSCurveVec[i].WriteSTEPEdge( &step, merge_pnts );
        }
    }

    BuildNURBSSurfMap();

    vector < vector < SdaiAdvanced_face* > > adv_vec( comp_id_group_vec.size() );
    map < string, vector < SdaiSurface* > > geom_surf_label_map;

    for ( size_t si = 0; si < m_NURBSSurfVec.size(); si++ )
    {
        if ( m_NURBSSurfVec[si].m_NURBSLoopVec.size() == 1 &&
                  m_NURBSSurfVec[si].m_NURBSLoopVec[0].m_BorderLoopFlag &&
                  m_NURBSSurfVec[si].m_NURBSLoopVec[0].m_InternalLoopFlag )
        {
            continue; // Indicates that the surface is completely enclosed
        }

        string label;

        if ( label_id )
        {
            label = m_SurfVec[m_NURBSSurfVec[si].m_SurfID]->GetGeomID();
        }

        if ( label_name )
        {
            if ( label.size() > 0 )
            {
                label.append( label_delim );
            }
            label.append( m_GeomNameMap[m_SurfVec[m_NURBSSurfVec[si].m_SurfID]->GetGeomID()] );
        }

        if ( label_surf_num )
        {
            if ( label.size() > 0 )
            {
                label.append( label_delim );
            }
            label.append( to_string( m_SurfVec[m_NURBSSurfVec[si].m_SurfID]->GetMainSurfID() ) );
        }

        if ( label_split_num )
        {
            if ( label.size() > 0 )
            {
                label.append( label_delim );
            }
            label.append( to_string( m_NURBSSurfVec[si].m_SurfID ) );
        }

        SdaiSurface* surf = m_NURBSSurfVec[si].WriteSTEPSurf( &step, merge_pnts );
        geom_surf_label_map[label].push_back( surf );

        int comp_id = -1;
        for ( size_t j = 0; j < m_SurfVec.size(); j++ )
        {
            if ( m_SurfVec[j]->GetSurfID() == m_NURBSSurfVec[si].m_SurfID )
            {
                comp_id = m_SurfVec[j]->GetCompID();
                break;
            }
        }

        for ( size_t j = 0; j < comp_id_group_vec.size(); j++ )
        {
            if ( std::count( comp_id_group_vec[j].begin(), comp_id_group_vec[j].end(), comp_id ) )
            {
                vector < SdaiAdvanced_face* > adv = m_NURBSSurfVec[si].WriteSTEPLoops( &step, surf, merge_pnts );
                adv_vec[j].insert( adv_vec[j].end(), adv.begin(), adv.end() );
            }
        }
    }

    map < string, vector < SdaiSurface* > >::iterator it;

    for ( it = geom_surf_label_map.begin(); it != geom_surf_label_map.end(); ++it )
    {
        SdaiGeometric_set* gset = (SdaiGeometric_set*)step.registry->ObjCreate( "GEOMETRIC_SET" );
        step.instance_list->Append( (SDAI_Application_instance*)gset, completeSE );
        gset->name_( "'" + ( *it ).first + "'" );

        for ( size_t i = 0; i < ( *it ).second.size(); i++ )
        {
            gset->elements_()->AddNode( new EntityNode( (SDAI_Application_instance*)( *it ).second[i] ) );
        }
    }

    // TODO: Don't include transparent and structure surfaces in BREP?

    if ( representation == vsp::STEP_SHELL )
    {
        step.RepresentManifoldShell( adv_vec );
    }
    else
    {
        step.RepresentBREPSolid( adv_vec );
    }

    step.WriteFile( filename );
}

vector < vector < int > > SurfaceIntersectionSingleton::GetCompIDGroupVec()
{
    // Identify the unique sets of intersected components
    map < int, vector < int > > intersection_comp_id_map;
    list< ISegChain* >::iterator i_seg;

    for ( i_seg = m_ISegChainList.begin(); i_seg != m_ISegChainList.end(); ++i_seg )
    {
        int comp_A_id = ( *i_seg )->m_SurfA->GetCompID();
        int comp_B_id = ( *i_seg )->m_SurfB->GetCompID();

        if ( !std::count( intersection_comp_id_map[comp_A_id].begin(), intersection_comp_id_map[comp_A_id].end(), comp_B_id ) )
        {
            intersection_comp_id_map[comp_A_id].push_back( comp_B_id );
        }

        if ( !std::count( intersection_comp_id_map[comp_B_id].begin(), intersection_comp_id_map[comp_B_id].end(), comp_A_id ) )
        {
            intersection_comp_id_map[comp_B_id].push_back( comp_A_id );
        }
    }

    map< int, vector < int > >::iterator i_map;
    vector < vector < int > > comp_id_group_vec;

    for ( i_map = intersection_comp_id_map.begin(); i_map != intersection_comp_id_map.end(); ++i_map )
    {
        if ( comp_id_group_vec.size() == 0 )
        {
            comp_id_group_vec.push_back( i_map->second );
            continue;
        }

        bool matched = false;
        int group_index = -1;

        for ( size_t i = 0; i < i_map->second.size(); ++i )
        {
            if ( !matched )
            {
                for ( size_t j = 0; j < comp_id_group_vec.size(); ++j )
                {
                    if ( std::count( comp_id_group_vec[j].begin(), comp_id_group_vec[j].end(), i_map->second[i] ) )
                    {
                        matched = true;
                        group_index = j;
                        break;
                    }
                }
            }
            else if ( !std::count( comp_id_group_vec[group_index].begin(), comp_id_group_vec[group_index].end(), i_map->second[i] ) )
            {
                comp_id_group_vec[group_index].push_back( i_map->second[i] );
            }
        }

        if ( !matched )
        {
            comp_id_group_vec.push_back( i_map->second );
        }
    }

    return comp_id_group_vec;
}

void SurfaceIntersectionSingleton::BuildNURBSSurfMap()
{
    m_NURBSSurfVec.clear();
    m_NURBSSurfVec.resize( m_SurfVec.size() );

    for ( size_t si = 0; si < m_SurfVec.size(); si++ )
    {
        // Create NURBS Surface
        m_NURBSSurfVec[si].InitNURBSSurf( m_SurfVec[si] );

        m_NURBSSurfVec[si].m_SurfType = m_SurfVec[si]->GetSurfaceCfdType();

        // Identify all border and intersection NURBS curves on the surface
        vector < NURBS_Curve > nurbs_curve_vec = m_NURBSSurfVec[si].MatchNURBSCurves( m_NURBSCurveVec );
        m_NURBSSurfVec[si].SetNURBSCurveVec( nurbs_curve_vec );

        // Build NURBS loops
        m_NURBSSurfVec[si].BuildNURBSLoopMap();
    }
}

void SurfaceIntersectionSingleton::BuildNURBSCurvesVec()
{
    // Only define the NURBS curves once to help avoid tolerance errors
    m_NURBSCurveVec.clear();

    //==== Find Max Bound Box of All Components ====//
    BndBox big_box;
    for ( size_t s = 0; s < m_SurfVec.size(); ++s )
    {
        big_box.Update( m_SurfVec[s]->GetBBox() );
    }
    double x_dist = 1.0 + big_box.GetMax( 0 ) - big_box.GetMin( 0 );
    double y_dist = 1.0 + big_box.GetMax( 1 ) - big_box.GetMin( 1 );
    double z_dist = 1.0 + big_box.GetMax( 2 ) - big_box.GetMin( 2 );

    list< ISegChain* >::iterator i_seg;

    for ( i_seg = m_ISegChainList.begin(); i_seg != m_ISegChainList.end(); ++i_seg )
    {
        bool internal_flag = false, ss_flag = false, struct_intersect_flag = false;

        // Check if the curve is interenal or external
        // Identify test point
        vec3d cp;
        if ( ( *i_seg )->m_ISegDeque.size() <= 2 )
        {
            // Take midpoint of first segment
            cp = ( ( *i_seg )->m_ISegDeque[0]->m_IPnt[0]->m_Pnt + ( *i_seg )->m_ISegDeque[0]->m_IPnt[1]->m_Pnt ) / 2.0;
        }
        else
        {
            // Identify point approximately halfway on border curve
            cp = ( *i_seg )->m_ISegDeque[( *i_seg )->m_ISegDeque.size() / 2]->m_IPnt[0]->m_Pnt;
        }

        // Check 3 directions and take majority result
        vec3d xep = cp + vec3d( x_dist, 1.0e-4, 1.0e-4 );
        vec3d yep = cp + vec3d( 1.0e-4, y_dist, 1.0e-4 );
        vec3d zep = cp + vec3d( 1.0e-4, 1.0e-4, z_dist );

        vector< double > x_vec, y_vec, z_vec;

        // Check if the curve is inside any component by checking the number of intersections from 3 vectors
        // beginning at the midpoint of the curve. Checking 3 vectors prevents a false positive or negative
        // from a vector that exactly aligns with another curve
        for ( size_t j = 0; j < m_NumComps; j++ )
        {
            if ( j == ( *i_seg )->m_SurfA->GetCompID() || j == ( *i_seg )->m_SurfB->GetCompID() )
            {
                continue;
            }

            for ( size_t i = 0; i < m_SurfVec.size(); i++ )
            {
                if ( ( m_SurfVec[i]->GetCompID() == j ) &&
                     ( m_SurfVec[i]->GetSurfaceCfdType() == vsp::CFD_NORMAL ) )
                {
                    m_SurfVec[i]->IntersectLineSeg( cp, xep, x_vec );
                    m_SurfVec[i]->IntersectLineSeg( cp, yep, y_vec );
                    m_SurfVec[i]->IntersectLineSeg( cp, zep, z_vec );
                }
            }

            bool x_in = x_vec.size() % 2 == 1;
            bool y_in = y_vec.size() % 2 == 1;
            bool z_in = z_vec.size() % 2 == 1;

            if ( ( x_in && y_in ) || ( x_in && z_in ) || ( y_in && z_in ) )
            {
                // Odd -> curve is inside
                internal_flag = true;
                break;
            }
        }

        if ( !( *i_seg )->m_BorderFlag && ( ( *i_seg )->m_SurfA->GetCompID() == ( *i_seg )->m_SurfB->GetCompID() ) )
        {
            // Indicates a Sub-Surface
            ss_flag = true;
        }

        if ( !( *i_seg )->m_BorderFlag && ( *i_seg )->m_SurfA->GetSurfaceCfdType() == vsp::CFD_STRUCTURE && ( *i_seg )->m_SurfB->GetSurfaceCfdType() == vsp::CFD_STRUCTURE )
        {
            // Intersected structures
            struct_intersect_flag = true;
        }

        NURBS_Curve nurbs_curve;

        nurbs_curve.m_BorderFlag = ( *i_seg )->m_BorderFlag;
        nurbs_curve.m_InternalFlag = internal_flag;
        nurbs_curve.m_SubSurfFlag = ss_flag;
        nurbs_curve.m_StructIntersectFlag = struct_intersect_flag;
        nurbs_curve.m_SurfA_ID = ( *i_seg )->m_SurfA->GetSurfID();
        nurbs_curve.m_SurfB_ID = ( *i_seg )->m_SurfB->GetSurfID();
        nurbs_curve.InitNURBSCurve( ( *i_seg )->m_ACurve );

        if ( ( *i_seg )->m_SurfA->GetSurfaceCfdType() == vsp::CFD_TRANSPARENT || ( *i_seg )->m_SurfB->GetSurfaceCfdType() == vsp::CFD_TRANSPARENT )
        {
            nurbs_curve.m_SurfIntersectType = vsp::CFD_TRANSPARENT;
        }
        else if ( ( *i_seg )->m_SurfA->GetSurfaceCfdType() == vsp::CFD_STRUCTURE || ( *i_seg )->m_SurfB->GetSurfaceCfdType() == vsp::CFD_STRUCTURE )
        {
            nurbs_curve.m_SurfIntersectType = vsp::CFD_STRUCTURE;
        }

        m_NURBSCurveVec.push_back( nurbs_curve );
    }
}

void SurfaceIntersectionSingleton::BuildCurves()
{
    list< ISegChain* >::iterator c;
    for ( c = m_ISegChainList.begin() ; c != m_ISegChainList.end(); ++c )
    {
        ( *c )->BuildCurves();
    }
}
void SurfaceIntersectionSingleton::Intersect()
{

    if ( GetSettingsPtr()->m_IntersectSubSurfs ) BuildSubSurfIntChains();

    //==== Quad Tree Intersection - Intersection Segments Get Loaded at AddIntersectionSeg ===//
    for ( int i = 0 ; i < ( int )m_SurfVec.size() ; i++ )
    {
        for ( int j = i + 1; j < (int) m_SurfVec.size(); j++ )
        {
            m_SurfVec[i]->Intersect( m_SurfVec[j], this );
        }
    }

    BuildChains();

    MergeFeaPartSSEdgeOverlap(); // Only applicable to FEA Mesh

    LoadBorderCurves();

    MergeInteriorChainIPnts();

    SetFixPointBorderNodes(); // Only applicable to FEA Mesh
    CheckFixPointIntersects(); // Only applicable to FEA Mesh

    SplitBorderCurves();

    IntersectSplitChains();

    BuildCurves();
}

void SurfaceIntersectionSingleton::AddIntersectionSeg( const SurfPatch& pA, const SurfPatch& pB, const vec3d & ip0, const vec3d & ip1 )
{
    double d = dist_squared( ip0, ip1 );
    if ( d < DBL_EPSILON )
    {
        return;
    }

    vec2d proj_uwA0;
    pA.find_closest_uw( ip0, proj_uwA0.v );

    vec2d proj_uwB0;
    pB.find_closest_uw( ip0, proj_uwB0.v );

    vec2d proj_uwA1;
    pA.find_closest_uw( ip1, proj_uwA1.v );

    vec2d proj_uwB1;
    pB.find_closest_uw( ip1, proj_uwB1.v );

    // Intersections that lie exactly on a patch boundary will actually intersect both patches
    // that share that boundary.  So, detect intersections that lie on the patch minimum edge
    // and don't carry those forward.  Don't do this if the minimum parameter is zero.  I.e.
    // there is no prior patch.

    double tol = 1e-8; // Tolerance buildup due to SurfPatch::find_closest_uw and other inaccuracies

    if ( pA.get_u_min() > 0.0 ) // if Patch A is not the very beginning of u
    {
        double lim = pA.get_u_min() + tol;
        // if both points projected to A are on the starting edge of u
        if ( proj_uwA0.v[0] <= lim && proj_uwA1.v[0] <= lim )
        {
            return;
        }
    }

    if ( pB.get_u_min() > 0.0 ) // if Patch B is not the very beginning of u
    {
        double lim = pB.get_u_min() + tol;
        // if both points projected to B are on the starting edge of u
        if ( proj_uwB0.v[0] <= lim && proj_uwB1.v[0] <= lim )
        {
            return;
        }
    }

    if ( pA.get_w_min() > 0.0 ) // if Patch A is not the very beginning of w
    {
        double lim = pA.get_w_min() + tol;
        // if both points projected to A are on the starting edge of w
        if ( proj_uwA0.v[1] <= lim && proj_uwA1.v[1] <= lim )
        {
            return;
        }
    }

    if ( pB.get_w_min() > 0.0 ) // if Patch B is not the very beginning of w
    {
        double lim = pB.get_w_min() + tol;
        // if both points projected to B are on the starting edge of w
        if ( proj_uwB0.v[1] <= lim && proj_uwB1.v[1] <= lim )
        {
            return;
        }
    }

    Puw* puwA0 = new Puw( pA.get_surf_ptr(), proj_uwA0 );
    m_DelPuwVec.push_back( puwA0 );

    Puw* puwB0 = new Puw( pB.get_surf_ptr(), proj_uwB0 );
    m_DelPuwVec.push_back( puwB0 );

    IPnt* ipnt0 = new IPnt( puwA0, puwB0 );
    ipnt0->m_Pnt = ip0;
    m_DelIPntVec.push_back( ipnt0 );

    Puw* puwA1 = new Puw( pA.get_surf_ptr(), proj_uwA1 );
    m_DelPuwVec.push_back( puwA1 );

    Puw* puwB1 = new Puw( pB.get_surf_ptr(), proj_uwB1 );
    m_DelPuwVec.push_back( puwB1 );

    IPnt* ipnt1 = new IPnt( puwA1, puwB1 );
    ipnt1->m_Pnt = ip1;
    m_DelIPntVec.push_back( ipnt1 );

    long id0 = IPntBin::ComputeID( ipnt0->m_Pnt );
    long id1 = IPntBin::ComputeID( ipnt1->m_Pnt );

    //==== Determine if Segment has a Duplicate ====//
    bool match = false;

    vector< IPnt* > compareIPntVec0;
    m_BinMap[id0].AddCompareIPnts( ipnt0, compareIPntVec0 ); // Get all segments with matching end point

    for ( int i = 0; i < (int)compareIPntVec0.size(); i++ )
    {
        if ( compareIPntVec0[i]->m_Puws[0]->m_Surf == ipnt0->m_Puws[0]->m_Surf &&
             compareIPntVec0[i]->m_Puws[1]->m_Surf == ipnt0->m_Puws[1]->m_Surf )
        {
            if ( compareIPntVec0[i]->m_Segs.size() > 0 ) // Should always be true
            {
                double d0 = dist_squared( compareIPntVec0[i]->m_Segs[0]->m_IPnt[0]->m_Pnt, ip0 );
                double d1 = dist_squared( compareIPntVec0[i]->m_Segs[0]->m_IPnt[1]->m_Pnt, ip1 );

                double d2 = dist_squared( compareIPntVec0[i]->m_Segs[0]->m_IPnt[1]->m_Pnt, ip0 );
                double d3 = dist_squared( compareIPntVec0[i]->m_Segs[0]->m_IPnt[0]->m_Pnt, ip1 );

                if ( ( d0 <= DBL_EPSILON && d1 <= DBL_EPSILON ) || ( d2 <= DBL_EPSILON && d3 <= DBL_EPSILON ) )
                {
                    match = true;
                    break;
                }
            }
        }
    }

    if ( !match )
    {
        new ISeg( pA.get_surf_ptr(), pB.get_surf_ptr(), ipnt0, ipnt1 );

        m_BinMap[id0].m_ID = id0;
        m_BinMap[id0].m_IPnts.push_back( ipnt0 );

        m_BinMap[id1].m_ID = id1;
        m_BinMap[id1].m_IPnts.push_back( ipnt1 );
    }
    else
    {
        printf( "Duplicate Segment Skipped\n" );
    }

#ifdef DEBUG_CFD_MESH

    static bool onetime = true;
    static int ipntcnt = 0;
    static double max_dist = 0.0;
    if ( onetime )
    {
        fprintf( m_DebugFile, "CfdMeshMgr::AddIntersectionSeg \n" );
        onetime = false;
    }

    double dA0 = dist( ip0, puwA0->m_Surf->CompPnt( puwA0->m_UW.x(), puwA0->m_UW.y() ) );
    double dB0 = dist( ip0, puwB0->m_Surf->CompPnt( puwB0->m_UW.x(), puwB0->m_UW.y() ) );

    double dA1 = dist( ip1, puwA0->m_Surf->CompPnt( puwA1->m_UW.x(), puwA1->m_UW.y() ) );
    double dB1 = dist( ip1, puwB0->m_Surf->CompPnt( puwB1->m_UW.x(), puwB1->m_UW.y() ) );

    double total_d = dA0 + dB0 + dA1 + dB1;

    if ( total_d > max_dist )
    {
        max_dist = total_d;
        fprintf( m_DebugFile, "  Proj Pnt Dist = %f    %d \n", max_dist, ipntcnt );
    }
    ipntcnt++;

#endif


//if ( pA.get_surf_ptr() == m_SurfVec[0]  )
//{
//  debugUWs.push_back( proj_uwA0 );
//  debugUWs.push_back( proj_uwA1 );
//}


}


ISeg* SurfaceIntersectionSingleton::CreateSurfaceSeg(  Surf* surfA, vec2d & uwA0, vec2d & uwA1, Surf* surfB, vec2d & uwB0, vec2d & uwB1   )
{
    Puw* puwA0 = new Puw( surfA, uwA0 );
    m_DelPuwVec.push_back( puwA0 );

    Puw* puwB0 = new Puw( surfB, uwB0 );
    m_DelPuwVec.push_back( puwB0 );

    IPnt* ipnt0 = new IPnt( puwA0, puwB0 );
    ipnt0->m_Pnt = surfA->CompPnt( uwA0.x(), uwA0.y() );
    m_DelIPntVec.push_back( ipnt0 );

    Puw* puwA1 = new Puw( surfA, uwA1 );
    m_DelPuwVec.push_back( puwA1 );

    Puw* puwB1 = new Puw( surfB, uwB1 );
    m_DelPuwVec.push_back( puwB1 );

    IPnt* ipnt1 = new IPnt( puwA1, puwB1 );
    ipnt1->m_Pnt = surfA->CompPnt( uwA1.x(), uwA1.y() );
    m_DelIPntVec.push_back( ipnt1 );

    ISeg* iseg01 = new ISeg( surfA, surfB, ipnt0, ipnt1 );

    return iseg01;
}

void SurfaceIntersectionSingleton::BuildChains()
{
    //==== Load Adjoining Bins =====//
    map< long, IPntBin >::const_iterator iter;
    for ( iter = m_BinMap.begin() ; iter != m_BinMap.end() ; ++iter )
    {
        int id = ( *iter ).second.m_ID;
        for ( int i = -3 ; i < 4 ; i++ )        // Check All Nearby Bins
        {
            if ( i != 0 &&  m_BinMap.find( id + i ) != m_BinMap.end()  )
            {
                m_BinMap[id].m_AdjBins.push_back( id + i );
            }
        }
    }

    //==== Create Chains ====//
    for ( iter = m_BinMap.begin() ; iter != m_BinMap.end() ; ++iter )
    {
        int id = ( *iter ).second.m_ID;
        for ( int i = 0 ; i < ( int )m_BinMap[id].m_IPnts.size() ; i++ )
        {
            if ( !m_BinMap[id].m_IPnts[i]->m_UsedFlag && m_BinMap[id].m_IPnts[i]->m_Segs.size() > 0 )
            {
                ISeg* seg = m_BinMap[id].m_IPnts[i]->m_Segs[0];
                seg->m_IPnt[0]->m_UsedFlag = true;
                seg->m_IPnt[1]->m_UsedFlag = true;
                ISegChain* chain = new ISegChain;           // Create New Chain
                chain->m_SurfA = seg->m_SurfA;
                chain->m_SurfB = seg->m_SurfB;
                chain->m_ISegDeque.push_back( seg );
                ExpandChain( chain );
                if ( chain->Valid() )
                {
                    m_ISegChainList.push_back( chain );
                }
                else
                {
                    delete chain;
                    chain = NULL;
                }
            }
        }
    }

#ifdef DEBUG_CFD_MESH

    int num_bins = 0;
    int total_num_segs = 0;
    for ( iter = m_BinMap.begin() ; iter != m_BinMap.end() ; ++iter )
    {
        num_bins++;
        int id = ( *iter ).second.m_ID;
        total_num_segs += m_BinMap[id].m_IPnts.size();
    }

    double avg_num_segs = ( double )total_num_segs / ( double )num_bins;

    fprintf( m_DebugFile, "CfdMeshMgr::BuildChains \n" );
    fprintf( m_DebugFile, "   Num Bins = %d \n", num_bins );
    fprintf( m_DebugFile, "   Avg Num Segs per Bin = %f\n", avg_num_segs );

    fprintf( m_DebugFile, "   Num Chains %zu \n", m_ISegChainList.size() );
#endif

}

void SurfaceIntersectionSingleton::ExpandChain( ISegChain* chain )
{
    bool stillExpanding = true;
    bool expandFront = true;
    while ( stillExpanding )
    {
        IPnt* testIPnt;
        if ( expandFront )
        {
            testIPnt = chain->m_ISegDeque.front()->m_IPnt[0];
        }
        else
        {
            testIPnt = chain->m_ISegDeque.back()->m_IPnt[1];
        }

        long binID = IPntBin::ComputeID( testIPnt->m_Pnt );

        IPnt* matchIPnt = m_BinMap[binID].Match( testIPnt, m_BinMap );

        if ( !matchIPnt && !expandFront )   // No more matches in back of chain
        {
            stillExpanding = false;
        }
        else if ( !matchIPnt )                  // No more matches in front, try back
        {
            expandFront = false;
        }
        else
        {
            ISeg* seg = matchIPnt->m_Segs[0];
            chain->AddSeg( seg, expandFront );
            seg->m_IPnt[0]->m_UsedFlag = true;
            seg->m_IPnt[1]->m_UsedFlag = true;
        }
    }
}

void SurfaceIntersectionSingleton::WriteChains()
{
    FILE* fp;
    char str[256];
    sprintf( str, "RawInt_UW0.dat" );
    fp = fopen( str, "w" );

    for ( int i = 1 ; i < ( int )debugUWs.size() ; i += 2 )
    {
        fprintf( fp, "MOVE \n" );
        fprintf( fp, "%f %f\n", debugUWs[i - 1].x(), debugUWs[i - 1].y() );
        fprintf( fp, "%f %f\n", debugUWs[i].x(), debugUWs[i].y() );
    }

    fclose( fp );


    //==== Write Chains ====//
    list< ISegChain* >::iterator c;
    for ( int i = 0 ; i < ( int )m_SurfVec.size() ; i++ )
    {
        sprintf( str, "Intersection_UW%d.dat", i );
        fp = fopen( str, "w" );
        int c_ind = 0;
        for ( c = m_ISegChainList.begin() ; c != m_ISegChainList.end(); ++c )
        {
            if ( ( *c )->m_SurfA == m_SurfVec[i] || ( *c )->m_SurfB == m_SurfVec[i] )
            {
                int ncol = 10;
                if ( c_ind % ncol == 0 )
                {
                    fprintf( fp, "color WHITE\n" );
                }
                else if ( c_ind % ncol == 1 )
                {
                    fprintf( fp, "color RED\n" );
                }
                else if ( c_ind % ncol == 2 )
                {
                    fprintf( fp, "color PURPLE\n" );
                }
                else if ( c_ind % ncol == 3 )
                {
                    fprintf( fp, "color DARK_BLUE\n" );
                }
                else if ( c_ind % ncol == 4 )
                {
                    fprintf( fp, "color DARK_ORANGE\n" );
                }
                else if ( c_ind % ncol == 5 )
                {
                    fprintf( fp, "color AQUA\n" );
                }
                else if ( c_ind % ncol == 6 )
                {
                    fprintf( fp, "color GREEN\n" );
                }
                else if ( c_ind % ncol == 7 )
                {
                    fprintf( fp, "color DARK_RED\n" );
                }
                else if ( c_ind % ncol == 8 )
                {
                    fprintf( fp, "color DARK_PURPLE\n" );
                }
                else if ( c_ind % ncol == 9 )
                {
                    fprintf( fp, "color LIGHT_BLUE\n" );
                }
                c_ind++;
//BLACK,WHITE, RED, BLUE, GREEN, PURPLE, YELLOW, GREY or GRAY,
//AQUA,DARK_BLUE,DARK_RED, DARK_PURPLE, DARK_ORANGE, DARK_GREY or
//DARK_GRAY and LIGHT_BLUE
                for ( int s = 0 ; s < ( int )( *c )->m_ISegDeque.size() ; s++ )
                {
                    IPnt* ip0 = ( *c )->m_ISegDeque[s]->m_IPnt[0];
                    IPnt* ip1 = ( *c )->m_ISegDeque[s]->m_IPnt[1];

                    int puwind = -1;
                    for ( int p = 0 ; p < ( int )ip0->m_Puws.size() ; p++ )
                    {
                        if ( ip0->m_Puws[p]->m_Surf == m_SurfVec[i] )
                        {
                            puwind = p;
                        }
                    }

                    if ( puwind >= 0 )
                    {
                        Puw* puw0 = ip0->m_Puws[puwind];
                        Puw* puw1 = ip1->m_Puws[puwind];

                        fprintf( fp, "MOVE \n" );
                        fprintf( fp, "%f %f\n", puw0->m_UW.x(), puw0->m_UW.y() );
                        fprintf( fp, "%f %f\n", puw1->m_UW.x(), puw1->m_UW.y() );
                    }
                }

                //==== Draw Intersections ====//
                for ( int s = 0 ; s < ( int )( *c )->m_SplitVec.size() ; s++ )
                {
                    ISegSplit* split = ( *c )->m_SplitVec[s];
                    if ( !( *c )->m_BorderFlag && split->m_Surf == m_SurfVec[i] )
                    {
                        fprintf( fp, "color YELLOW\n" );
                        fprintf( fp, "MOVE \n" );
                        fprintf( fp, "%f %f\n", split->m_UW.x(), split->m_UW.y() );
                        fprintf( fp, "%f %f\n", split->m_UW.x() + 0.1, split->m_UW.y() + 0.1 );
                    }

                }
            }
        }
        fclose( fp );
    }
}

void SurfaceIntersectionSingleton::LoadBorderCurves()
{

    //==== Tesselate Border Curves ====//
    for ( int i = 0 ; i < ( int )m_ICurveVec.size() ; i++ )
    {
        if ( !m_ICurveVec[i]->m_SCurve_B )                              // Non Closed Solid
        {
            m_ICurveVec[i]->m_SCurve_B = m_ICurveVec[i]->m_SCurve_A;
        }

        if ( m_ICurveVec[i]->m_PlaneBorderIntersectFlag )
        {
            Surf* SurfA = m_ICurveVec[i]->m_SCurve_A->GetSurf();
            if ( !SurfA->GetSymPlaneFlag() )
            {
                m_ICurveVec[i]->PlaneBorderTesselate( m_ICurveVec[i]->m_SCurve_A, m_ICurveVec[i]->m_SCurve_B );
            }
            else
            {
                m_ICurveVec[i]->PlaneBorderTesselate( m_ICurveVec[i]->m_SCurve_B, m_ICurveVec[i]->m_SCurve_A );
            }
        }
        else
        {
            m_ICurveVec[i]->BorderTesselate( );
        }

        //==== Create New Chain ====//
        ISegChain* chain = new ISegChain;
        chain->m_BorderFlag = true;

        Surf* surfA = m_ICurveVec[i]->m_SCurve_A->GetSurf();
        Surf* surfB = m_ICurveVec[i]->m_SCurve_B->GetSurf();

        chain->m_SurfA = surfA;
        chain->m_SurfB = surfB;

        vector< vec3d > uwA = m_ICurveVec[i]->m_SCurve_A->GetUWTessPnts();
        vector< vec3d > uwB = m_ICurveVec[i]->m_SCurve_B->GetUWTessPnts();

        for ( int j = 1 ; j < ( int )uwA.size() ; j++ )
        {
            Puw* puwA0 = new Puw( surfA, vec2d( uwA[j - 1][0], uwA[j - 1][1] ) );
            Puw* puwA1 = new Puw( surfA, vec2d( uwA[j][0],   uwA[j][1] ) );
            Puw* puwB0 = new Puw( surfB, vec2d( uwB[j - 1][0], uwB[j - 1][1] ) );
            Puw* puwB1 = new Puw( surfB, vec2d( uwB[j][0],   uwB[j][1] ) );

            m_DelPuwVec.push_back( puwA0 );         // Save to delete later
            m_DelPuwVec.push_back( puwA1 );
            m_DelPuwVec.push_back( puwB0 );
            m_DelPuwVec.push_back( puwB1 );

            IPnt* p0 = new IPnt( puwA0, puwB0 );
            IPnt* p1 = new IPnt( puwA1, puwB1 );

            m_DelIPntVec.push_back( p0 );           // Save to delete later
            m_DelIPntVec.push_back( p1 );

            p0->CompPnt();
            p1->CompPnt();

            ISeg* seg = new ISeg( surfA, surfB, p0, p1 );
            chain->m_ISegDeque.push_back( seg );

        }

        if( chain->Valid() )
        {
            m_ISegChainList.push_front( chain );
        }
        else
        {
            delete chain;
            chain = NULL;
        }
    }

#ifdef DEBUG_CFD_MESH
    fprintf( m_DebugFile, "CfdMeshMgr::LoadBorderCurves \n" );
    fprintf( m_DebugFile, "   Total Num Chains = %zu \n", m_ISegChainList.size() );

    list< ISegChain* >::iterator c;
    for ( c = m_ISegChainList.begin() ; c != m_ISegChainList.end(); ++c )
    {
        fprintf( m_DebugFile, "   Chain Num Segs = %d Border %d \n",
                 ( *c )->m_ISegDeque.size(), ( int )( ( *c )->m_BorderFlag ) );
    }
#endif

}

void SurfaceIntersectionSingleton::BuildSubSurfIntChains()
{
    // Adds subsurface intersection chains
    vec2d uw_pnt0;
    vec2d uw_pnt1;
    const unsigned int num_sects = 100; // Number of segments to break subsurface segments up into

    // If there is an issue with having a watertight mesh between the intersection of two
    // components near a forced subsurface line, try increasing num_sects especially for highly
    // curved surfaces

    for ( int s = 0 ; s < ( int )m_SurfVec.size() ; s++ )
    {
        Surf* surf = m_SurfVec[s];

        // Get all SubSurfaces for the specified geom
        vector < SimpleSubSurface > ss_vec = GetSimpSubSurfs( surf->GetGeomID(), surf->GetMainSurfID(), surf->GetCompID() );

        // Split SubSurfs
        for ( int ss = 0 ; ss < ( int ) ss_vec.size(); ss++ )
        {
            ss_vec[ss].SplitSegsU( surf->GetSurfCore()->GetMinU() );
            ss_vec[ss].SplitSegsU( surf->GetSurfCore()->GetMaxU() );
            ss_vec[ss].SplitSegsW( surf->GetSurfCore()->GetMinW() );
            ss_vec[ss].SplitSegsW( surf->GetSurfCore()->GetMaxW() );

            vector < vector< SSLineSeg > >& segsvec = ss_vec[ss].GetSplitSegs();

            for ( int i = 0; i < segsvec.size(); i++ )
            {
                vector< SSLineSeg >& segs = segsvec[i];

                ISegChain* chain = NULL;

                bool new_chain = true;
                bool is_poly = ss_vec[ss].GetPolyFlag();

                // Build Intersection Chains
                for ( int ls = 0; ls < ( int )segs.size(); ls++ )
                {
                    if ( new_chain && chain )
                    {
                        if ( chain->Valid() )
                        {
                            if ( ss_vec[ss].m_IncludedElements != vsp::FEA_SHELL )
                            {
                                chain->m_SSIntersectIndex = ss; // Identify FeaSubSurfaceIndex
                            }

                            m_ISegChainList.push_back( chain );
                        }
                        else
                        {
                            delete chain;
                            chain = NULL;
                        }
                    }

                    if ( new_chain )
                    {
                        chain = new ISegChain;
                        chain->m_SurfA = surf;
                        chain->m_SurfB = surf;
                        if ( !is_poly )
                        {
                            new_chain = false;
                        }
                    }

                    SSLineSeg l_seg = segs[ls];
                    vec3d lp0, lp1;

                    lp0 = l_seg.GetP0();
                    lp1 = l_seg.GetP1();
                    uw_pnt0 = vec2d( lp0.x(), lp0.y() );
                    uw_pnt1 = vec2d( lp1.x(), lp1.y() );
                    double max_u, max_w, tol;
                    double min_u, min_w;
                    tol = 1e-6;
                    min_u = surf->GetSurfCore()->GetMinU();
                    min_w = surf->GetSurfCore()->GetMinW();
                    max_u = surf->GetSurfCore()->GetMaxU();
                    max_w = surf->GetSurfCore()->GetMaxW();

                    if ( uw_pnt0[0] < min_u - FLT_EPSILON || uw_pnt0[1] < min_w - FLT_EPSILON || uw_pnt1[0] < min_u - FLT_EPSILON || uw_pnt1[1] < min_w - FLT_EPSILON )
                    {
                        new_chain = true;
                        continue; // Skip if either point has a value not on this surface
                    }
                    if ( uw_pnt0[0] > max_u + FLT_EPSILON || uw_pnt0[1] > max_w + FLT_EPSILON || uw_pnt1[0] > max_u + FLT_EPSILON || uw_pnt1[1] > max_w + FLT_EPSILON )
                    {
                        new_chain = true;
                        continue; // Skip if either point has a value not on this surface
                    }
                    if ( ((std::abs( uw_pnt0[0]-max_u ) < tol && std::abs( uw_pnt1[0]-max_u ) < tol) ||
                            (std::abs( uw_pnt0[1]-max_w ) < tol && std::abs( uw_pnt1[1]-max_w ) < tol) ||
                            (std::abs( uw_pnt0[0]-min_u ) < tol && std::abs( uw_pnt1[0]-min_u ) < tol) ||
                            (std::abs( uw_pnt0[1]-min_w ) < tol && std::abs( uw_pnt1[1]-min_w ) < tol))
                            && is_poly  )
                    {
                        new_chain = true;
                        continue; // Skip if both end points are on the same edge of the surface
                    }

                    double delta_u = ( uw_pnt1[0] - uw_pnt0[0] ) / num_sects;
                    double delta_w = ( uw_pnt1[1] - uw_pnt0[1] ) / num_sects;

                    vector< vec2d > uw_pnts;
                    uw_pnts.resize( num_sects + 1 );
                    uw_pnts[0] = uw_pnt0;
                    uw_pnts[num_sects] = uw_pnt1;

                    // Add additional points between the segment endpoints to hopefully make the curve planar with the surface
                    for ( int p = 1 ; p < num_sects ; p++ )
                    {
                        uw_pnts[p] = vec2d( uw_pnt0[0] + delta_u * p, uw_pnt0[1] + delta_w * p );
                    }

                    for ( int p = 1 ; p < ( int ) uw_pnts.size() ; p++ )
                    {
                        Puw* puwA0 = new Puw( surf, uw_pnts[p - 1] );
                        Puw* puwA1 = new Puw( surf, uw_pnts[p] );
                        Puw* puwB0 = new Puw( surf, uw_pnts[p - 1] );
                        Puw* puwB1 = new Puw( surf, uw_pnts[p] );

                        m_DelPuwVec.push_back( puwA0 );         // Save to delete later
                        m_DelPuwVec.push_back( puwA1 );
                        m_DelPuwVec.push_back( puwB0 );
                        m_DelPuwVec.push_back( puwB1 );

                        IPnt* p0 = new IPnt( puwA0, puwB0 );
                        IPnt* p1 = new IPnt( puwA1, puwB1 );

                        m_DelIPntVec.push_back( p0 );           // Save to delete later
                        m_DelIPntVec.push_back( p1 );

                        p0->CompPnt();
                        p1->CompPnt();

                        ISeg* seg = new ISeg( surf, surf, p0, p1 );
                        chain->m_ISegDeque.push_back( seg );
                    }
                }
                if ( chain )
                {
                    if ( chain->Valid() )
                    {
                        if ( ss_vec[ss].m_IncludedElements != vsp::FEA_SHELL )
                        {
                            chain->m_SSIntersectIndex = ss; // Identify FeaSubSurfaceIndex
                        }

                        m_ISegChainList.push_back( chain );
                    }
                    else
                    {
                        delete chain;
                        chain = NULL;
                    }
                }
            }
        }
    }
}

void SurfaceIntersectionSingleton::SplitBorderCurves()
{
    vector< IPnt* > splitPnts;
    list< ISegChain* >::iterator c;
    for ( c = m_ISegChainList.begin() ; c != m_ISegChainList.end(); ++c )
    {
        if  ( !( *c )->m_BorderFlag ) // Non Border Chains
        {
            splitPnts.push_back( ( *c )->m_ISegDeque.front()->m_IPnt[0] );
            splitPnts.push_back( ( *c )->m_ISegDeque.back()->m_IPnt[1]  );
        }
    }

    for ( c = m_ISegChainList.begin() ; c != m_ISegChainList.end(); ++c )
    {
        if ( ( *c )->m_BorderFlag )
        {
            for ( int i = 0 ; i < ( int )splitPnts.size() ; i++ )
            {
                Puw* uwA = splitPnts[i]->GetPuw( ( *c )->m_SurfA );
                Puw* uwB = splitPnts[i]->GetPuw( ( *c )->m_SurfB );
                if ( uwA )
                {
                    ( *c )->AddBorderSplit( splitPnts[i], uwA );
                }
                else if ( uwB )
                {
                    ( *c )->AddBorderSplit( splitPnts[i], uwB );
                }
            }
        }
    }

    //==== Load Only Border Chains ====//
    vector< ISegChain* > chains;
    for ( c = m_ISegChainList.begin() ; c != m_ISegChainList.end(); ++c )
    {
        if ( ( *c )->m_BorderFlag )
        {
            chains.push_back( ( *c ) );
        }
    }

    //==== Split Chains ====//
    for ( int i = 0 ; i < ( int )chains.size() ; i++ )
    {
        chains[i]->MergeSplits();
        vector< ISegChain* > new_chains = chains[i]->SortAndSplit( this );
        for ( int j = 0 ; j < ( int )new_chains.size() ; j++ )
        {
            new_chains[j]->m_BorderFlag = true;
            if ( new_chains[j]->Valid() )
            {
                new_chains[j]->m_SSIntersectIndex = chains[i]->m_SSIntersectIndex; // Track SubSurface Index
                m_ISegChainList.push_back( new_chains[j] );
            }
        }
    }

    //==== Load Only Border Chains ====//
    chains.clear();
    for ( c = m_ISegChainList.begin() ; c != m_ISegChainList.end(); ++c )
    {
        if ( ( *c )->m_BorderFlag )
        {
            chains.push_back( ( *c ) );
        }
    }

    ////==== Check if Border Chains Lie On Another Surfaces ====//
    //for ( int i = 0 ; i < (int)chains.size() ; i++ )
    //{
    //  Surf* surfA = chains[i]->m_SurfA;
    //  vector< Surf* > surfBVec = GetPossCoPlanarSurfs( surfA );
    //  if ( surfBVec.size() )
    //  {
    //      for ( int s = 0 ; s < (int)surfBVec.size() ; s++ )
    //      {
    //          Surf* surfB = surfBVec[s];
    //          vector< ISegChain* > new_chains = chains[i]->FindCoPlanarChains( surfB, surfA );
    //          for ( int j = 0 ; j < (int)new_chains.size() ; j++ )
    //          {
    //              new_chains[j]->m_BorderFlag = true;
    //              m_ISegChainList.push_back( new_chains[j] );
    //          }
    //      }
    //  }
    //}

#ifdef DEBUG_CFD_MESH
    m_DebugDraw = true;
    for ( c = m_ISegChainList.begin() ; c != m_ISegChainList.end(); ++c )
    {
        vector< vec3d > crv;
        for ( int i = 0 ; i < ( int )( *c )->m_ISegDeque.size() ; i++ )
        {
            IPnt* ip = ( *c )->m_ISegDeque[i]->m_IPnt[0];
            crv.push_back( ip->m_Pnt );

            if ( i == ( int )( *c )->m_ISegDeque.size() - 1 )
            {
                ip = ( *c )->m_ISegDeque[i]->m_IPnt[1];
                crv.push_back( ip->m_Pnt );
            }
        }
        m_DebugCurves.push_back( crv );
        int crv_cnt = ( int )m_DebugCurves.size();
        int r = ( crv_cnt % 3 ) * 85;
        int g = ( ( crv_cnt + 2 ) % 5 ) * 55;
        int b = ( ( crv_cnt + 4 ) % 4 ) * 63;
        m_DebugColors.push_back( vec3d( r, g, b ) );
    }
#endif

}

void SurfaceIntersectionSingleton::IntersectSplitChains()
{
    //==== Intersect Intersection Curves (Not Border Curves) ====//
    list< ISegChain* >::iterator c;
    vector< ISegChain* > chains;
    for ( c = m_ISegChainList.begin() ; c != m_ISegChainList.end(); ++c )
    {
        if ( !( *c )->m_BorderFlag )
        {
            chains.push_back( ( *c ) );
        }
    }

    //==== Build Bounding Boxes Around Intersection Curves ====//
    for ( int i = 0 ; i < ( int )chains.size() ; i++ )
    {
        chains[i]->BuildBoxes();
    }

    //==== Do Intersection ====//
    for ( int i = 0 ; i < ( int )chains.size() ; i++ )
    {
        for ( int j = i + 1 ; j < ( int )chains.size() ; j++ )
        {
            if ( chains[i]->m_SurfA == chains[j]->m_SurfA || chains[i]->m_SurfA == chains[j]->m_SurfB )
            {
                chains[i]->Intersect( chains[i]->m_SurfA, chains[j] );
            }
            else if ( chains[i]->m_SurfB == chains[j]->m_SurfA || chains[i]->m_SurfB == chains[j]->m_SurfB )
            {
                chains[i]->Intersect( chains[i]->m_SurfB, chains[j] );
            }
        }
    }

    //==== Merge Splits ====//
    for ( int i = 0 ; i < ( int )chains.size() ; i++ )
    {
        chains[i]->MergeSplits();
    }

    //==== Remove Chain End Splits ====//
    for ( int i = 0 ; i < ( int )chains.size() ; i++ )
    {
        chains[i]->RemoveChainEndSplits();
    }

    //==== Split Chains ====//
    for ( int i = 0 ; i < ( int )chains.size() ; i++ )
    {
        vector< ISegChain* > new_chains = chains[i]->SortAndSplit( this );
        for ( int j = 0 ; j < ( int )new_chains.size() ; j++ )
        {
            if ( new_chains[j]->Valid() )
            {
                new_chains[j]->m_SSIntersectIndex = chains[i]->m_SSIntersectIndex; // Track SubSurface Index
                m_ISegChainList.push_back( new_chains[j] );
            }

        }
    }
}

void SurfaceIntersectionSingleton::MergeInteriorChainIPnts()
{
    //==== Merge Interior IPnts in Chains ====//
    list< ISegChain* >::iterator c;
    for ( c = m_ISegChainList.begin() ; c != m_ISegChainList.end(); ++c )
    {
        ( *c )->MergeInteriorIPnts();
    }
}



void SurfaceIntersectionSingleton::HighlightNextChain()
{
    m_HighlightChainIndex++;
    if ( m_HighlightChainIndex >= ( int )m_ISegChainList.size() )
    {
        m_HighlightChainIndex = 0;
    }
}

void SurfaceIntersectionSingleton::DebugWriteChains( const char* name, bool tessFlag )
{
    if ( true )
    {
        for ( int i = 0 ; i < ( int )m_SurfVec.size() ; i++ )
        {
            char str[256];
            sprintf( str, "%s%d.m", name, i );
            FILE* fp = fopen( str, "w" );

            fprintf( fp, "clear all; format compact; close all;\n" );
            fprintf( fp, "figure(1)\n" );


            int cnt = 0;
            list< ISegChain* >::iterator c;
            for ( c = m_ISegChainList.begin() ; c != m_ISegChainList.end(); ++c )
            {
                if ( m_SurfVec[i] == ( *c )->m_SurfA || m_SurfVec[i] == ( *c )->m_SurfB )
                {

                    if ( tessFlag == false )
                    {
                        fprintf( fp, "x=[" );
                        int j;
                        vec2d uw1, uw2;
                        for ( j = 0 ; j < ( int )( *c )->m_ISegDeque.size() - 1 ; j++ )
                        {
                            uw1 = ( *c )->m_ISegDeque[j]->m_IPnt[0]->GetPuw( m_SurfVec[i] )->m_UW;
                            fprintf( fp, "%f,", uw1[0] );
                        }
                        uw1 = ( *c )->m_ISegDeque[j]->m_IPnt[0]->GetPuw( m_SurfVec[i] )->m_UW;
                        uw2 = ( *c )->m_ISegDeque[j]->m_IPnt[1]->GetPuw( m_SurfVec[i] )->m_UW;
                        fprintf( fp, "%f, %f];\n", uw1[0], uw2[0] );

                        fprintf( fp, "y=[" );
                        for ( j = 0 ; j < ( int )( *c )->m_ISegDeque.size() - 1 ; j++ )
                        {
                            uw1 = ( *c )->m_ISegDeque[j]->m_IPnt[0]->GetPuw( m_SurfVec[i] )->m_UW;
                            fprintf( fp, "%f,", uw1[1] );
                        }
                        uw1 = ( *c )->m_ISegDeque[j]->m_IPnt[0]->GetPuw( m_SurfVec[i] )->m_UW;
                        uw2 = ( *c )->m_ISegDeque[j]->m_IPnt[1]->GetPuw( m_SurfVec[i] )->m_UW;
                        fprintf( fp, "%f, %f];\n", uw1[1], uw2[1] );

                        fprintf( fp, "plot( x, y, 'x-'); hold on;\n" );
                    }
                    else
                    {
                        fprintf( fp, "u=[" );
                        int j;
                        vec2d uw1;
                        for ( j = 0 ; j < ( int )( *c )->m_TessVec.size() - 1 ; j++ )
                        {
                            uw1 = ( *c )->m_TessVec[j]->GetPuw( m_SurfVec[i] )->m_UW;
                            fprintf( fp, "%f,", uw1[0] );
                        }
                        uw1 = ( *c )->m_TessVec[j]->GetPuw( m_SurfVec[i] )->m_UW;
                        fprintf( fp, "%f];\n", uw1[0] );

                        fprintf( fp, "w=[" );
                        for ( j = 0 ; j < ( int )( *c )->m_TessVec.size() - 1 ; j++ )
                        {
                            uw1 = ( *c )->m_TessVec[j]->GetPuw( m_SurfVec[i] )->m_UW;
                            fprintf( fp, "%f,", uw1[1] );
                        }
                        uw1 = ( *c )->m_TessVec[j]->GetPuw( m_SurfVec[i] )->m_UW;
                        fprintf( fp, "%f];\n", uw1[1] );

                        fprintf( fp, "figure(1)\n");
                        fprintf( fp, "plot( u, w, 'x-'); hold on;\n" );


                        fprintf( fp, "x=[" );
                        vec3d pt;
                        for ( j = 0 ; j < ( int )( *c )->m_TessVec.size() - 1 ; j++ )
                        {
                            uw1 = ( *c )->m_TessVec[j]->GetPuw( m_SurfVec[i] )->m_UW;
                            pt = ( *c )->m_TessVec[j]->GetPuw( m_SurfVec[i] )->m_Surf->CompPnt( uw1[0], uw1[1] );
                            fprintf( fp, "%f,", pt.x() );
                        }
                        uw1 = ( *c )->m_TessVec[j]->GetPuw( m_SurfVec[i] )->m_UW;
                        pt = ( *c )->m_TessVec[j]->GetPuw( m_SurfVec[i] )->m_Surf->CompPnt( uw1[0], uw1[1] );
                        fprintf( fp, "%f];\n", pt.x() );

                        fprintf( fp, "y=[" );
                        for ( j = 0 ; j < ( int )( *c )->m_TessVec.size() - 1 ; j++ )
                        {
                            uw1 = ( *c )->m_TessVec[j]->GetPuw( m_SurfVec[i] )->m_UW;
                            pt = ( *c )->m_TessVec[j]->GetPuw( m_SurfVec[i] )->m_Surf->CompPnt( uw1[0], uw1[1] );
                            fprintf( fp, "%f,", pt.y() );
                        }
                        uw1 = ( *c )->m_TessVec[j]->GetPuw( m_SurfVec[i] )->m_UW;
                        pt = ( *c )->m_TessVec[j]->GetPuw( m_SurfVec[i] )->m_Surf->CompPnt( uw1[0], uw1[1] );
                        fprintf( fp, "%f];\n", pt.y() );

                        fprintf( fp, "z=[" );
                        for ( j = 0 ; j < ( int )( *c )->m_TessVec.size() - 1 ; j++ )
                        {
                            uw1 = ( *c )->m_TessVec[j]->GetPuw( m_SurfVec[i] )->m_UW;
                            pt = ( *c )->m_TessVec[j]->GetPuw( m_SurfVec[i] )->m_Surf->CompPnt( uw1[0], uw1[1] );
                            fprintf( fp, "%f,", pt.z() );
                        }
                        uw1 = ( *c )->m_TessVec[j]->GetPuw( m_SurfVec[i] )->m_UW;
                        pt = ( *c )->m_TessVec[j]->GetPuw( m_SurfVec[i] )->m_Surf->CompPnt( uw1[0], uw1[1] );
                        fprintf( fp, "%f];\n", pt.z() );

                        fprintf( fp, "figure(2)\n");
                        fprintf( fp, "plot3( x, y, z, 'x-'); hold on;\n" );

                    }
                    cnt++;
                }
            }
            fprintf( fp, "figure(1)\n");
            fprintf(fp, "axis off\n" );
            fprintf(fp, "axis equal\n" );

            fprintf( fp, "figure(2)\n");
            fprintf(fp, "axis off\n" );
            fprintf(fp, "axis equal\n" );

            fclose( fp );
        }
    }
    else
    {
        for ( int i = 0 ; i < ( int )m_SurfVec.size() ; i++ )
        {
            char str[256];
            sprintf( str, "%s%d.dat", name, i );
            FILE* fp = fopen( str, "w" );

            int cnt = 0;
            list< ISegChain* >::iterator c;
            for ( c = m_ISegChainList.begin() ; c != m_ISegChainList.end(); ++c )
            {
                if ( m_SurfVec[i] == ( *c )->m_SurfA || m_SurfVec[i] == ( *c )->m_SurfB )
                {
                    if ( cnt % 9 == 0 )
                    {
                        fprintf( fp, "COLOR RED\n" );
                    }
                    else if ( cnt % 9 == 1 )
                    {
                        fprintf( fp, "COLOR BLUE\n" );
                    }
                    else if ( cnt % 9 == 2 )
                    {
                        fprintf( fp, "COLOR GREEN\n" );
                    }
                    else if ( cnt % 9 == 3 )
                    {
                        fprintf( fp, "COLOR PURPLE\n" );
                    }
                    else if ( cnt % 9 == 4 )
                    {
                        fprintf( fp, "COLOR YELLOW\n" );
                    }
                    else if ( cnt % 9 == 5 )
                    {
                        fprintf( fp, "COLOR DARK_ORANGE\n" );
                    }
                    else if ( cnt % 9 == 6 )
                    {
                        fprintf( fp, "COLOR GREY\n" );
                    }
                    else if ( cnt % 9 == 7 )
                    {
                        fprintf( fp, "COLOR DARK_PURPLE\n" );
                    }
                    else if ( cnt % 9 == 8 )
                    {
                        fprintf( fp, "COLOR AQUA\n" );
                    }


                    if ( ! tessFlag )
                    {
                        for ( int j = 0 ; j < ( int )( *c )->m_ISegDeque.size() ; j++ )
                        {
                            fprintf( fp, "MOVE \n" );
                            vec2d uw0 = ( *c )->m_ISegDeque[j]->m_IPnt[0]->GetPuw( m_SurfVec[i] )->m_UW;
                            vec2d uw1 = ( *c )->m_ISegDeque[j]->m_IPnt[1]->GetPuw( m_SurfVec[i] )->m_UW;
                            vec2d tmp = uw0 + ( uw1 - uw0 ) * 0.1;
                            uw1 = uw1 + ( uw0 - uw1 ) * 0.1;
                            uw0 = tmp;
                            fprintf( fp, "%f %f\n", uw0[0], uw0[1] );
                            fprintf( fp, "%f %f\n", uw1[0], uw1[1] );
                        }
                    }
                    else
                    {
                        fprintf( fp, "MOVE \n" );
                        for ( int j = 1 ; j < ( int )( *c )->m_TessVec.size() ; j++ )
                        {
                            vec2d uw0 = ( *c )->m_TessVec[j - 1]->GetPuw( m_SurfVec[i] )->m_UW;
                            vec2d uw1 = ( *c )->m_TessVec[j]->GetPuw( m_SurfVec[i] )->m_UW;
                            vec2d tmp = uw0 + ( uw1 - uw0 ) * 0.1;
                            uw1 = uw1 + ( uw0 - uw1 ) * 0.1;
                            uw0 = tmp;
                            fprintf( fp, "%f %f\n", uw0[0], uw0[1] );
                            fprintf( fp, "%f %f\n", uw1[0], uw1[1] );
                        }
                    }
                    cnt++;
                }
            }
            fclose( fp );
        }
    }
}

void SurfaceIntersectionSingleton::AddPossCoPlanarSurf( Surf* surfA, Surf* surfB )
{
    vector< Surf* > surfVec = GetPossCoPlanarSurfs( surfA );

    //==== Check If SurfB Already Stored ====//
    for ( int i = 0 ; i < ( int )surfVec.size() ; i++ )
    {
        if ( surfVec[i] == surfB )
        {
            return;
        }
    }

    m_PossCoPlanarSurfMap[surfA].push_back( surfB );
}

vector< Surf* > SurfaceIntersectionSingleton::GetPossCoPlanarSurfs( Surf* surfPtr )
{
    if ( m_PossCoPlanarSurfMap.find( surfPtr ) != m_PossCoPlanarSurfMap.end() )
    {
        return m_PossCoPlanarSurfMap[surfPtr];
    }

    vector< Surf* > retSurfVec;
    return retSurfVec;
}

void SurfaceIntersectionSingleton::TestStuff()
{
    if ( !m_SurfVec.size() )
    {
        return;
    }

    Surf* sPtr = m_SurfVec[0];

    vector< SurfPatch* > pVec = sPtr->GetPatchVec();

    if ( !pVec.size() )
    {
        return;
    }

    SurfPatch sp0;
    SurfPatch sp1;
    SurfPatch sp2;
    SurfPatch sp3;

    pVec[0]->split_patch( sp0, sp1, sp2, sp3 );

    vec3d psurf = pVec[0]->comp_pnt_01( 0.3, 0.3 );
    vec3d ppatch = sp0.comp_pnt_01( 0.6, 0.6 );
    double d = dist( psurf, ppatch );

    psurf = pVec[0]->comp_pnt_01( 0.6, 0.6 );
    ppatch = sp3.comp_pnt_01( 0.2, 0.2 );
    d = dist( psurf, ppatch );

    psurf = pVec[0]->comp_pnt_01( 0.3, 0.6 );
    ppatch = sp2.comp_pnt_01( 0.6, 0.2 );
    d = dist( psurf, ppatch );

    psurf = pVec[0]->comp_pnt_01( 0.6, 0.3 );
    ppatch = sp1.comp_pnt_01( 0.2, 0.6 );
    d = dist( psurf, ppatch );

    sp3.split_patch( sp0, sp1, sp2, sp3 );
    psurf = pVec[0]->comp_pnt_01( 0.6, 0.6 );
    ppatch = sp0.comp_pnt_01( 0.4, 0.4 );
    d = dist( psurf, ppatch );
}

void SurfaceIntersectionSingleton::BinaryAdaptIntCurves()
{
    m_BinAdaptCurveAVec.clear();
    m_BinAdaptCurveBVec.clear();
    m_RawCurveAVec.clear();
    m_RawCurveBVec.clear();
    m_BorderCurveFlagVec.clear();

    list<ISegChain *>::iterator c;
    for ( c = m_ISegChainList.begin(); c != m_ISegChainList.end(); ++c )
    {
        m_BorderCurveFlagVec.push_back( (*c)->m_BorderFlag );

        vector<vec3d> ptvec;
        vector<vec3d> rawptvec;

        Bezier_curve xyzcrvA = (*c)->m_ACurve.GetUWCrv();
        xyzcrvA.TessAdaptXYZ( *(*c)->m_ACurve.GetSurf(), ptvec, GetSettingsPtr()->m_RelCurveTol, 16 );

        m_BinAdaptCurveAVec.push_back( ptvec );

        xyzcrvA.UWCurveToXYZCurve( (*c)->m_ACurve.GetSurf() );
        xyzcrvA.GetControlPoints( rawptvec );

        m_RawCurveAVec.push_back( rawptvec );


        ptvec.clear();
        rawptvec.clear();
        Bezier_curve xyzcrvB = (*c)->m_BCurve.GetUWCrv();
        xyzcrvB.TessAdaptXYZ( *(*c)->m_BCurve.GetSurf(), ptvec, GetSettingsPtr()->m_RelCurveTol, 16 );

        m_BinAdaptCurveBVec.push_back( ptvec );

        xyzcrvB.UWCurveToXYZCurve( (*c)->m_BCurve.GetSurf() );
        xyzcrvB.GetControlPoints( rawptvec );

        m_RawCurveBVec.push_back( rawptvec );

    }
}

void SurfaceIntersectionSingleton::LoadDrawObjs( vector< DrawObj* > &draw_obj_vec )
{
    if ( m_MeshInProgress )
    {
        return;
    }

    // Draw ISegChains
    m_IsectCurveDO.m_GeomID = GetID() + "ISECTCURVE";
    m_IsectCurveDO.m_Type = DrawObj::VSP_LINES;
    m_IsectCurveDO.m_Visible = GetSettingsPtr()->m_DrawIsectFlag &&
                               GetSettingsPtr()->m_DrawCurveFlag &&
                               GetSettingsPtr()->m_DrawBinAdaptFlag;
    m_IsectCurveDO.m_LineColor = vec3d(0, 0, 1);
    m_IsectCurveDO.m_LineWidth = 2.0;

    m_IsectPtsDO.m_GeomID = GetID() + "ISECTPTS";
    m_IsectPtsDO.m_Type = DrawObj::VSP_POINTS;
    m_IsectPtsDO.m_Visible = GetSettingsPtr()->m_DrawIsectFlag &&
                               GetSettingsPtr()->m_DrawPntsFlag &&
                               GetSettingsPtr()->m_DrawBinAdaptFlag;
    m_IsectPtsDO.m_PointColor = vec3d(0, 0, 0);
    m_IsectPtsDO.m_PointSize = 10.0;

    m_BorderCurveDO.m_GeomID = GetID() + "BORDERCURVE";
    m_BorderCurveDO.m_Type = DrawObj::VSP_LINES;
    m_BorderCurveDO.m_Visible = GetSettingsPtr()->m_DrawBorderFlag &&
                               GetSettingsPtr()->m_DrawCurveFlag &&
                               GetSettingsPtr()->m_DrawBinAdaptFlag;
    m_BorderCurveDO.m_LineColor = vec3d(0, 1, 0);
    m_BorderCurveDO.m_LineWidth = 2.0;

    m_BorderPtsDO.m_GeomID = GetID() + "BORDERPTS";
    m_BorderPtsDO.m_Type = DrawObj::VSP_POINTS;
    m_BorderPtsDO.m_Visible = GetSettingsPtr()->m_DrawBorderFlag &&
                               GetSettingsPtr()->m_DrawPntsFlag &&
                               GetSettingsPtr()->m_DrawBinAdaptFlag;
    m_BorderPtsDO.m_PointColor = vec3d(0, 0, 0);
    m_BorderPtsDO.m_PointSize = 10.0;

    m_RawIsectCurveDO.m_GeomID = GetID() + "RAWISECTCURVE";
    m_RawIsectCurveDO.m_Type = DrawObj::VSP_LINES;
    m_RawIsectCurveDO.m_Visible = GetSettingsPtr()->m_DrawIsectFlag &&
                                  GetSettingsPtr()->m_DrawCurveFlag &&
                                  GetSettingsPtr()->m_DrawRawFlag;
    m_RawIsectCurveDO.m_LineColor = vec3d(1, 0, 1);
    m_RawIsectCurveDO.m_LineWidth = 2.0;

    m_RawIsectPtsDO.m_GeomID = GetID() + "RAWISECTPTS";
    m_RawIsectPtsDO.m_Type = DrawObj::VSP_POINTS;
    m_RawIsectPtsDO.m_Visible = GetSettingsPtr()->m_DrawIsectFlag &&
                                GetSettingsPtr()->m_DrawPntsFlag &&
                                GetSettingsPtr()->m_DrawRawFlag;
    m_RawIsectPtsDO.m_PointColor = vec3d(0.5, 0.5, 0.5);
    m_RawIsectPtsDO.m_PointSize = 10.0;

    m_RawBorderCurveDO.m_GeomID = GetID() + "RAWBORDERCURVE";
    m_RawBorderCurveDO.m_Type = DrawObj::VSP_LINES;
    m_RawBorderCurveDO.m_Visible = GetSettingsPtr()->m_DrawBorderFlag &&
                                   GetSettingsPtr()->m_DrawCurveFlag &&
                                   GetSettingsPtr()->m_DrawRawFlag;
    m_RawBorderCurveDO.m_LineColor = vec3d(1, 1, 0);
    m_RawBorderCurveDO.m_LineWidth = 2.0;

    m_RawBorderPtsDO.m_GeomID = GetID() + "RAWBORDERPTS";
    m_RawBorderPtsDO.m_Type = DrawObj::VSP_POINTS;
    m_RawBorderPtsDO.m_Visible = GetSettingsPtr()->m_DrawBorderFlag &&
                                 GetSettingsPtr()->m_DrawPntsFlag &&
                                 GetSettingsPtr()->m_DrawRawFlag;
    m_RawBorderPtsDO.m_PointColor = vec3d(0.5, 0.5, 0.5);
    m_RawBorderPtsDO.m_PointSize = 10.0;

    m_ApproxPlanesDO.m_GeomID = GetID() + "APPROXPLANES";
    m_ApproxPlanesDO.m_Type = DrawObj::VSP_LINES;
    m_ApproxPlanesDO.m_Visible = false;
    m_ApproxPlanesDO.m_LineColor = vec3d( .2, .2, .2 );
    m_ApproxPlanesDO.m_LineWidth = 1.0;

    m_IsectCurveDO.m_PntVec.clear();
    m_IsectPtsDO.m_PntVec.clear();
    m_BorderCurveDO.m_PntVec.clear();
    m_BorderPtsDO.m_PntVec.clear();
    m_RawIsectCurveDO.m_PntVec.clear();
    m_RawIsectPtsDO.m_PntVec.clear();
    m_RawBorderCurveDO.m_PntVec.clear();
    m_RawBorderPtsDO.m_PntVec.clear();
    m_ApproxPlanesDO.m_PntVec.clear();

    for ( int indx = 0; indx < m_RawCurveAVec.size(); indx++ )
    {
        DrawObj *curveDO;
        DrawObj *ptsDO;
        DrawObj *rawcurveDO;
        DrawObj *rawptsDO;

        if ( m_BorderCurveFlagVec[indx] )
        {
            curveDO = &m_BorderCurveDO;
            ptsDO = &m_BorderPtsDO;
            rawcurveDO = & m_RawBorderCurveDO;
            rawptsDO = &m_RawBorderPtsDO;
        }
        else
        {
            curveDO = &m_IsectCurveDO;
            ptsDO = &m_IsectPtsDO;
            rawcurveDO = &m_RawIsectCurveDO;
            rawptsDO = & m_RawIsectPtsDO;
        }

        {
            vector<vec3d> ptvec;
            vector<vec3d> rawptvec;

            ptvec = m_BinAdaptCurveAVec[ indx ];

            rawptvec = m_RawCurveAVec[ indx ];

            ptsDO->m_PntVec.insert( ptsDO->m_PntVec.end(), ptvec.begin(), ptvec.end() );
            rawptsDO->m_PntVec.insert( rawptsDO->m_PntVec.end(), rawptvec.begin(), rawptvec.end() );

            for ( int j = 1; j < ptvec.size(); j++ )
            {
                curveDO->m_PntVec.push_back( ptvec[j - 1] );
                curveDO->m_PntVec.push_back( ptvec[j] );
            }

            for ( int j = 1; j < rawptvec.size(); j++ )
            {
                rawcurveDO->m_PntVec.push_back( rawptvec[j - 1] );
                rawcurveDO->m_PntVec.push_back( rawptvec[j] );
            }


            ptvec.clear();
            rawptvec.clear();

            ptvec = m_BinAdaptCurveBVec[ indx ];

            rawptvec = m_RawCurveBVec[ indx ];

            ptsDO->m_PntVec.insert( ptsDO->m_PntVec.end(), ptvec.begin(), ptvec.end() );
            rawptsDO->m_PntVec.insert( rawptsDO->m_PntVec.end(), rawptvec.begin(), rawptvec.end() );

            for ( int j = 1; j < ptvec.size(); j++ )
            {
                curveDO->m_PntVec.push_back( ptvec[j - 1] );
                curveDO->m_PntVec.push_back( ptvec[j] );
            }

            for ( int j = 1; j < rawptvec.size(); j++ )
            {
                rawcurveDO->m_PntVec.push_back( rawptvec[j - 1] );
                rawcurveDO->m_PntVec.push_back( rawptvec[j] );
            }
        }
    }

    list<ISegChain *>::iterator c;
    for ( c = m_ISegChainList.begin() ; c != m_ISegChainList.end(); ++c )
    {
        if ( !(*c)->m_BorderFlag )
        {
            (*c)->m_ISegBoxA.AppendLineSegs( m_ApproxPlanesDO.m_PntVec );
            (*c)->m_ISegBoxB.AppendLineSegs( m_ApproxPlanesDO.m_PntVec );
        }
    }

    // Normal Vec is not required, load placeholder.
    m_IsectCurveDO.m_NormVec = m_IsectCurveDO.m_PntVec;
    m_IsectPtsDO.m_NormVec = m_IsectPtsDO.m_PntVec;
    m_BorderCurveDO.m_NormVec = m_BorderCurveDO.m_PntVec;
    m_BorderPtsDO.m_NormVec = m_BorderPtsDO.m_PntVec;

    m_RawIsectCurveDO.m_NormVec = m_RawIsectCurveDO.m_PntVec;
    m_RawIsectPtsDO.m_NormVec = m_RawIsectPtsDO.m_PntVec;
    m_RawBorderCurveDO.m_NormVec = m_RawBorderCurveDO.m_PntVec;
    m_RawBorderPtsDO.m_NormVec = m_RawBorderPtsDO.m_PntVec;

    m_ApproxPlanesDO.m_NormVec = m_ApproxPlanesDO.m_PntVec;

    draw_obj_vec.push_back( &m_IsectCurveDO );
    draw_obj_vec.push_back( &m_IsectPtsDO );
    draw_obj_vec.push_back( &m_BorderCurveDO );
    draw_obj_vec.push_back( &m_BorderPtsDO );

    draw_obj_vec.push_back( &m_RawIsectCurveDO );
    draw_obj_vec.push_back( &m_RawIsectPtsDO );
    draw_obj_vec.push_back( &m_RawBorderCurveDO );
    draw_obj_vec.push_back( &m_RawBorderPtsDO );

    draw_obj_vec.push_back( &m_ApproxPlanesDO );

}

void SurfaceIntersectionSingleton::SetICurveVec( ICurve* newcurve, int loc )
{
    m_ICurveVec[loc] = newcurve;
}


void SurfaceIntersectionSingleton::UpdateDisplaySettings()
{
    if ( GetIntersectSettingsPtr() )
    {
        GetIntersectSettingsPtr()->m_DrawBorderFlag = m_Vehicle->GetISectSettingsPtr()->m_DrawBorderFlag.Get();
        GetIntersectSettingsPtr()->m_DrawIsectFlag = m_Vehicle->GetISectSettingsPtr()->m_DrawIsectFlag.Get();
        GetIntersectSettingsPtr()->m_DrawRawFlag = m_Vehicle->GetISectSettingsPtr()->m_DrawRawFlag.Get();
        GetIntersectSettingsPtr()->m_DrawBinAdaptFlag = m_Vehicle->GetISectSettingsPtr()->m_DrawBinAdaptFlag.Get();
        GetIntersectSettingsPtr()->m_DrawCurveFlag = m_Vehicle->GetISectSettingsPtr()->m_DrawCurveFlag.Get();
        GetIntersectSettingsPtr()->m_DrawPntsFlag = m_Vehicle->GetISectSettingsPtr()->m_DrawPntsFlag.Get();

        GetIntersectSettingsPtr()->m_RelCurveTol = m_Vehicle->GetISectSettingsPtr()->m_RelCurveTol.Get();

        GetIntersectSettingsPtr()->m_IntersectSubSurfs = m_Vehicle->GetISectSettingsPtr()->m_IntersectSubSurfs.Get();
        GetIntersectSettingsPtr()->m_SelectedSetIndex = m_Vehicle->GetISectSettingsPtr()->m_SelectedSetIndex.Get();

        GetIntersectSettingsPtr()->m_XYZIntCurveFlag = m_Vehicle->GetISectSettingsPtr()->m_XYZIntCurveFlag.Get();
    }
}
