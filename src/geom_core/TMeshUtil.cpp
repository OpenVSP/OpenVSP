//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//

#ifdef WIN32
#define _HAS_STD_BYTE 0
#include <windows.h>
#endif

#define _USE_MATH_DEFINES
#include <cmath>

#include "Vehicle.h"
#include "VehicleMgr.h"

#include "TMesh.h"
#include "PGMesh.h"
#include "TMeshSweptVolume.h"

#include "tri_tri_intersect.h"
#include "predicates.h"
#include "Mathematics/ConvexHull3.h"
#include "Mathematics/DistRay3Triangle3.h"
#include "Mathematics/Ray.h"
#include "Mathematics/Triangle.h"

#include "Geom.h"
#include "SubSurfaceMgr.h"
#include "PntNodeMerge.h"
#include "VspCurve.h" // for #define TMAGIC

#include "triangle.h"
#include "triangle_api.h"

#include "VspUtil.h"

#include "delabella.h"
#include "StlHelper.h"


void CreateTMeshVecFromPts( const Geom * geom,
                            vector < TMesh* > & TMeshVec,
                            const vector< vector<vec3d> > & pnts,
                            const vector< vector<vec3d> > & uw_pnts,
                            int n_ref,
                            int indx, int platenum, int surftype, int cfdsurftype,
                            bool thicksurf, bool flipnormal, bool skipnegflipnormal,
                            int &iQuad, bool flatpatch )
{
    TMeshVec.push_back( new TMesh() );
    int itmesh = TMeshVec.size() - 1;
    TMeshVec[itmesh]->LoadGeomAttributes( geom );
    TMeshVec[itmesh]->m_SurfCfdType = cfdsurftype;
    TMeshVec[itmesh]->m_ThickSurf = thicksurf;
    TMeshVec[itmesh]->m_FlatPatch = flatpatch;
    TMeshVec[itmesh]->m_SurfType = surftype;
    TMeshVec[itmesh]->m_SurfNum = indx;
    TMeshVec[itmesh]->m_CopyIndex = geom->GetSurfCopyIndx( indx );
    TMeshVec[itmesh]->m_PlateNum = platenum;
    TMeshVec[itmesh]->m_UWPnts = uw_pnts;
    TMeshVec[itmesh]->m_XYZPnts = pnts;
    TMeshVec[itmesh]->m_Wmin = uw_pnts[0][0].y();
    TMeshVec[itmesh]->m_Uscale = geom->GetUMax( indx );
    TMeshVec[itmesh]->m_Wscale = geom->GetWMax( indx );;

    if ( !skipnegflipnormal && cfdsurftype == vsp::CFD_NEGATIVE )
    {
        flipnormal = !flipnormal;
    }

    BuildTMeshTris( TMeshVec[itmesh], flipnormal, geom->GetWMax( indx ), platenum, n_ref, iQuad );

}

void CreateTMeshVecFromPts( const Geom * geom,
                            vector < TMesh* > & TMeshVec,
                            const vector< vector<vec3d> > & pnts,
                            const vector< vector<vec3d> > & uw_pnts,
                            int n_ref,
                            int indx, int platenum, int surftype, int cfdsurftype,
                            bool thicksurf, bool flipnormal, bool skipnegflipnormal,
                            int iustart, int iuend,
                            int &iQuad, bool flatpatch )
{
    vector< vector < vec3d > > pnts_subset;
    vector< vector < vec3d > > uw_pnts_subset;

    int n = iuend - iustart + 1;

    pnts_subset.reserve( n );
    uw_pnts_subset.reserve( n );

    for ( int iu = iustart; iu <= iuend; iu++ )
    {
        pnts_subset.push_back( pnts[iu] );
        uw_pnts_subset.push_back( uw_pnts[iu] );
    }

    CreateTMeshVecFromPts( geom,
                           TMeshVec,
                           pnts_subset, uw_pnts_subset,
                           n_ref,
                           indx, platenum, surftype, cfdsurftype,
                           thicksurf, flipnormal, skipnegflipnormal,
                           iQuad, flatpatch );
}

void CreateTMeshVecFromPtsCheckFlat( const Geom * geom,
                                     vector < TMesh* > & TMeshVec,
                                     const vector< vector<vec3d> > & pnts,
                                     const vector< vector<vec3d> > & uw_pnts,
                                     int n_ref,
                                     int indx, int platenum, int surftype, int cfdsurftype,
                                     bool thicksurf, bool flipnormal, bool skipnegflipnormal, int &iQuad )
{
    // Comparing on distance squared between two normal vectors.
    double tol = 1e-12;

    int nu = pnts.size();
    if ( nu > 1 )
    {
        int nv = pnts[0].size();
        if ( nv > 1 )
        {
            // Build normal vectors.
            vector < vector < vec3d > > nvec;
            nvec.resize( nu - 1 );
            for ( int i = 0; i < nu - 1; i++ )
            {
                nvec[i].resize( nv - 1 );
                for ( int j = 0; j < nv - 1; j++ )
                {
                    vec3d u = pnts[ i + 1 ][ j + 1 ] - pnts[ i ][ j ];
                    vec3d v = pnts[ i + 1 ][ j ] - pnts[ i ][ j + 1 ];
                    vec3d n = cross( u, v );
                    n.normalize();

                    nvec[i][j] = n;
                }
            }

            // Check for planar strips.
            vector < bool > chordwise_flat( nu - 1, true );
            for ( int i = 0; i < nu - 1; i++ )
            {
                for ( int j = 1; j < nv - 1; j++ )
                {
                    if ( dist_squared( nvec[ i ][ 0 ],  nvec[ i ][ j ] ) > tol )
                    {
                        chordwise_flat[ i ] = false;
                        break;
                    }
                }
            }

            // Build spanwise extent of planar strips.
            vector < bool > spanwise_flat_match( nu - 1, false );
            spanwise_flat_match[ 0 ] = 0;
            for ( int i = 1; i < nu - 1; i++ )
            {
                if ( chordwise_flat[ i ] && chordwise_flat[ i - 1 ] &&
                     dist_squared( nvec[ i ][ 0 ], nvec[ i - 1 ][ 0 ] ) < tol )
                {
                    spanwise_flat_match[ i ] = true;
                }
            }


            int iustart = 0;
            while ( iustart < nu - 1 )
            {
                bool flatpatch = chordwise_flat[ iustart ];

                int iuend = iustart + 1;
                while ( spanwise_flat_match[ iuend ] == flatpatch && iuend < nu - 1 )
                {
                    iuend++;
                }

                CreateTMeshVecFromPts( geom,
                                       TMeshVec,
                                       pnts,
                                       uw_pnts,
                                       n_ref,
                                       indx, platenum, surftype, cfdsurftype,
                                       thicksurf, flipnormal, skipnegflipnormal,
                                       iustart, iuend,
                                       iQuad, flatpatch );

                // Over-ride some variable copies to full range rather than patch subset.
                TMeshVec.back()->m_UWPnts = uw_pnts;
                TMeshVec.back()->m_XYZPnts = pnts;
                TMeshVec.back()->m_Wmin = uw_pnts[0][0].y();

                iustart = iuend;
            }
        }
        else
        {
            CreateTMeshVecFromPts( geom,
                                   TMeshVec,
                                   pnts,
                                   uw_pnts,
                                   n_ref,
                                   indx, platenum, surftype, cfdsurftype,
                                   thicksurf, flipnormal, skipnegflipnormal,
                                   iQuad, false );
        }
    }
    else
    {
        CreateTMeshVecFromPts( geom,
                               TMeshVec,
                               pnts,
                               uw_pnts,
                               n_ref,
                               indx, platenum, surftype, cfdsurftype,
                               thicksurf, flipnormal, skipnegflipnormal,
                               iQuad, false );
    }
}


void BuildTMeshTris( TMesh *tmesh, bool flipnormal, double wmax, int platenum, int n_ref, int &iQuad  )
{
    double tol=1.0e-12;

    vector< vector<vec3d> > *pnts = &(tmesh->m_XYZPnts);
    vector< vector<vec3d> > *uw_pnts = &(tmesh->m_UWPnts);

    vec3d norm;
    vec3d v0, v1, v2, v3;
    vec3d uw0, uw1, uw2, uw3;
    vec3d d21, d01, d03, d23, d20, d31;

    int ref_start = 1 << ( n_ref + 2 ); // Offset n_ref by 2 to prevent collisions
    int ref_end = ref_start * 2;

    int nj = (*pnts).size();
    int nk = (*pnts)[0].size();

    // Find first non-degenerate j section.
    int firstj = -1;
    for ( int j = 0; j < nj - 1; j++ )
    {
        double areaj = 0.0;

        for ( int k = 0; k < nk - 1; k++ )
        {
            v0 = (*pnts)[j][k];
            v1 = (*pnts)[j + 1][k];
            v2 = (*pnts)[j + 1][k + 1];
            v3 = (*pnts)[j][k + 1];

            areaj += area( v0, v1, v2 ) + area( v0, v2, v3 );
        }
        if ( areaj > tol )
        {
            firstj = j;
            break;
        }
    }

    // Find last non-degenerate j section.
    int lastj = -1;
    for ( int j = nj - 2; j >= 0; j-- )
    {
        double areaj = 0.0;

        for ( int k = 0; k < nk - 1; k++ )
        {
            v0 = (*pnts)[j][k];
            v1 = (*pnts)[j + 1][k];
            v2 = (*pnts)[j + 1][k + 1];
            v3 = (*pnts)[j][k + 1];

            areaj += area( v0, v1, v2 ) + area( v0, v2, v3 );
        }
        if ( areaj > tol )
        {
            lastj = j;
            break;
        }
    }

    // Use degenerate j sections to find u to set condition
    double umin = (*uw_pnts)[ firstj ][ 0 ].x();
    double umax = (*uw_pnts)[ lastj + 1 ][ nk - 1 ].x();
    double umid = 0.5 * ( umin + umax );

    int jref = ref_start;
    for ( int j = 0; j < nj - 1; j++ )
    {
        int kref = ref_start;
        for ( int k = 0; k < nk - 1; k++ )
        {
            v0 = (*pnts)[j][k];
            v1 = (*pnts)[j + 1][k];
            v2 = (*pnts)[j + 1][k + 1];
            v3 = (*pnts)[j][k + 1];

            uw0 = (*uw_pnts)[j][k];
            uw1 = (*uw_pnts)[j + 1][k];
            uw2 = (*uw_pnts)[j + 1][k + 1];
            uw3 = (*uw_pnts)[j][k + 1];

            d21 = v2 - v1;
            d01 = v0 - v1;
            d03 = v0 - v3;
            d23 = v2 - v3;

            double quadrant = ( uw0.y() + uw1.y() + uw2.y() + uw3.y() ) / wmax; // * 4 * 0.25 canceled.
            double uave = ( uw0.x() + uw1.x() + uw2.x() + uw3.x() ) / 4.0;

            // Set up evencorners based on quadrants 0 and 2.
            bool evencorners = ( ( quadrant > 0 && quadrant < 1 ) || ( quadrant > 2 && quadrant < 3 ) );

            // Flip evencorners for second half of u.
            if ( uave > umid )
            {
                evencorners = !evencorners;
            }

            // Flip evencorners for every other degenerate plate (handles cruciform).
            if ( platenum % 2 )
            {
                evencorners = !evencorners;
            }

            if ( evencorners )
            {
                d20 = v2 - v0;
                if ( d21.mag() > tol && d01.mag() > tol && d20.mag() > tol )
                {
                    norm = cross( d21, d01 );
                    norm.normalize();
                    if ( flipnormal )
                    {
                        tmesh->AddTri( v0, v2, v1, norm * -1, uw0, uw2, uw1, iQuad, jref, kref );
                    }
                    else
                    {
                        tmesh->AddTri( v0, v1, v2, norm, uw0, uw1, uw2, iQuad, jref, kref );
                    }
                }

                if ( d03.mag() > tol && d23.mag() > tol && d20.mag() > tol )
                {
                    norm = cross( d03, d23 );
                    norm.normalize();
                    if ( flipnormal )
                    {
                        tmesh->AddTri( v0, v3, v2, norm * -1, uw0, uw3, uw2, iQuad, jref, kref );
                    }
                    else
                    {
                        tmesh->AddTri( v0, v2, v3, norm, uw0, uw2, uw3, iQuad, jref, kref );
                    }
                }
            }
            else
            {
                d31 = v3 - v1;
                if ( d01.mag() > tol && d31.mag() > tol && d03.mag() > tol )
                {
                    norm = cross( d01, d03 );
                    norm.normalize();
                    if ( flipnormal )
                    {
                        tmesh->AddTri( v0, v3, v1, norm * -1, uw0, uw3, uw1, iQuad, jref, kref );
                    }
                    else
                    {
                        tmesh->AddTri( v0, v1, v3, norm, uw0, uw1, uw3, iQuad, jref, kref );
                    }
                }

                if ( d21.mag() > tol && d23.mag() > tol && d31.mag() > tol )
                {
                    norm = cross( d23, d21 );
                    norm.normalize();
                    if ( flipnormal )
                    {
                        tmesh->AddTri( v1, v3, v2, norm * -1, uw1, uw3, uw2, iQuad, jref, kref );
                    }
                    else
                    {
                        tmesh->AddTri( v1, v2, v3, norm, uw1, uw2, uw3, iQuad, jref, kref );
                    }
                }
            }

            iQuad++;

            kref++;
            if ( kref >= ref_end )
            {
                kref = ref_start;
            }
        }

        jref++;
        if ( jref >= ref_end )
        {
            jref = ref_start;
        }
    }
}

vector<TMesh*> CopyTMeshVec( const vector<TMesh*> &tmv )
{
    vector < TMesh* > tmv_out( tmv.size() );

    for ( int i = 0; i < tmv.size(); i++ )
    {
        tmv_out[i] = new TMesh();
        tmv_out[i]->CopyFlatten( tmv[i] );
    }

    return tmv_out;
}

void DeleteTMeshVec(  vector<TMesh*> &tmv )
{
    for ( int i = 0; i < tmv.size(); i++ )
    {
        delete tmv[i];
    }
    tmv.clear();
}

TMesh* MergeTMeshVec( const vector<TMesh*> &tmv )
{
    TMesh *tm = new TMesh();

    for ( int i = 0; i < tmv.size(); i++ )
    {
        tm->MergeTMeshes( tmv[i] );
    }

    return tm;
}

void LoadBndBox( vector< TMesh* > &tmv )
{
    for ( int i = 0; i < tmv.size(); i++ )
    {
        tmv[i]->LoadBndBox();
    }
}

void UpdateBBox( BndBox &bbox, vector<TMesh*> &tmv, const Matrix4d &transMat )
{
    bbox.Reset();
    if ( tmv.size() > 0 )
    {
        for ( int i = 0 ; i < ( int )tmv.size() ; i++ )
        {
            tmv[i]->UpdateBBox( bbox, transMat );
        }
    }
    else
    {
        bbox.Update( vec3d( 0.0, 0.0, 0.0 ));
    }
}

void ApplyScale( double scalefac, vector<TMesh*> &tmv )
{
    unordered_set < TNode* > nodeSet;
    for ( int i = 0 ; i < ( int )tmv.size() ; i++ )
    {
        for ( int j = 0 ; j < ( int )tmv[i]->m_NVec.size() ; j++ )
        {
            TNode* n = tmv[i]->m_NVec[j];
            nodeSet.insert( n );
        }
        //==== Split Tris ====//
        for ( int j = 0 ; j < ( int )tmv[i]->m_TVec.size() ; j++ )
        {
            TTri* t = tmv[i]->m_TVec[j];
            for ( int k = 0 ; k < ( int )t->m_NVec.size() ; k++ )
            {
                TNode* n = t->m_NVec[k];
                nodeSet.insert( n );
            }
        }

        for ( int j = 0 ; j < (int)tmv[i]->m_XYZPnts.size() ; j++ )
        {
            for ( int k = 0 ; k < (int)tmv[i]->m_XYZPnts[j].size() ; k++ )
            {
                tmv[i]->m_XYZPnts[j][k] = tmv[i]->m_XYZPnts[j][k] * ( scalefac );
            }
        }
    }
    for ( const auto& n : nodeSet )
    {
        n->m_Pnt = n->m_Pnt * ( scalefac );
    }
}

void MergeRemoveOpenMeshes( vector<TMesh*> &tmv, MeshInfo* info, bool deleteopen )
{
    int i, j;

    //==== Check If All Closed ====//
    for ( i = 0 ; i < ( int )tmv.size() ; i++ )
    {
        tmv[i]->CheckIfClosed();
    }

    //==== Try to Merge Non Closed Meshes ====//
    // Marks mesh un-used after merge for deletion
    for ( i = 0 ; i < ( int )tmv.size() ; i++ )
    {
        for ( j = i + 1 ; j < ( int )tmv.size() ; j++ )
        {
            tmv[i]->MergeNonClosed( tmv[j] );
        }
    }
    // Keep track of merged meshes in info.
    for ( i = 0 ; i < ( int )tmv.size() ; i++ )
    {
        if ( tmv[i]->m_DeleteMeFlag )
        {
            info->m_NumOpenMeshesMerged++;
            info->m_MergedMeshes.push_back( tmv[i]->m_NameStr );
        }
    }

    // Mark any still open meshes for deletion.  Perhaps make this optional.
    if ( deleteopen )
    {
        for ( i = 0 ; i < ( int )tmv.size() ; i++ )
        {
            if ( tmv[i]->m_NonClosedTriVec.size() )
            {
                if ( !tmv[i]->m_DeleteMeFlag )
                {
                    info->m_NumOpenMeshedDeleted++;
                    info->m_DeletedMeshes.push_back( tmv[i]->m_NameStr );
                }

                tmv[i]->m_DeleteMeFlag = true;
            }
        }
    }

    DeleteMarkedMeshes( tmv );

    //==== Remove Any Degenerate Tris ====//
    for ( i = 0 ; i < ( int )tmv.size() ; i++ )
    {
        info->m_NumDegenerateTriDeleted += tmv[i]->RemoveDegenerate();
    }

}

void DeleteMarkedMeshes( vector<TMesh*> &tmv )
{
    //==== Remove meshes marked for deletion ====//
    vector< TMesh* > newTMeshVec;
    for ( int i = 0 ; i < ( int )tmv.size() ; i++ )
    {
        if ( !tmv[i]->m_DeleteMeFlag )
        {
            newTMeshVec.push_back( tmv[i] );
        }
        else
        {
            delete tmv[i];
        }
    }
    tmv = newTMeshVec;
}

void FlattenTMeshVec( vector<TMesh*> &tmv )
{
    vector<TMesh*> flatTMeshVec;
    flatTMeshVec.reserve( tmv.size() );
    for ( int i = 0 ; i < ( int )tmv.size() ; i++ )
    {
        TMesh *tm = new TMesh();
        tm->CopyFlatten( tmv[ i ] );
        if ( tm->m_TVec.size() > 0 )
        {
            flatTMeshVec.push_back( tm );
        }
        else
        {
            delete tm;
        }
        delete tmv[i];
    }
    tmv.clear();
    tmv = flatTMeshVec;
}

void TransformMeshVec( vector<TMesh*> & meshVec, const Matrix4d & TransMat )
{
    // Build Map of nodes
    unordered_set < TNode* > nodeSet;
    for ( int i = 0 ; i < ( int )meshVec.size() ; i++ )
    {
        for ( int j = 0 ; j < ( int )meshVec[i]->m_NVec.size() ; j++ )
        {
            TNode* n = meshVec[i]->m_NVec[j];
            nodeSet.insert( n );
        }
        //==== Split Tris ====//
        for ( int j = 0 ; j < ( int )meshVec[i]->m_TVec.size() ; j++ )
        {
            TTri* t = meshVec[i]->m_TVec[j];
            for ( int k = 0 ; k < ( int )t->m_NVec.size() ; k++ )
            {
                TNode* n = t->m_NVec[k];
                nodeSet.insert( n );
            }
        }
    }

    // Apply Transformation to Nodes
    for ( const auto& n : nodeSet )
    {
        n->m_Pnt = TransMat.xform( n->m_Pnt );
    }

    vec3d zeroV = vec3d( 0.0, 0.0, 0.0 );
    zeroV = TransMat.xform( zeroV );

    // Apply Transformation to each triangle's normal vector
    for ( int i = 0 ; i < ( int )meshVec.size() ; i ++ )
    {
        for ( int j = 0 ; j < ( int )meshVec[i]->m_TVec.size() ; j++ )
        {
            if ( meshVec[i]->m_TVec[j]->m_SplitVec.size() )
            {
                for ( int t = 0 ; t < ( int ) meshVec[i]->m_TVec[j]->m_SplitVec.size() ; t++ )
                {
                    TTri* tri = meshVec[i]->m_TVec[j]->m_SplitVec[t];
                    tri->m_Norm = TransMat.xform( tri->m_Norm ) - zeroV;
                }
            }
            else
            {
                TTri* tri = meshVec[i]->m_TVec[j];
                tri->m_Norm = TransMat.xform( tri->m_Norm ) - zeroV;
            }
        }

        // Apply Transformation to Mesh's area center
        meshVec[i]->m_AreaCenter = TransMat.xform( meshVec[i]->m_AreaCenter );
    }
}

vector< string > GetTMeshNames( vector<TMesh*> &tmv )
{
    vector< string > names;
    for ( int i = 0 ; i < ( int )tmv.size() ; i++ )
    {
        string plate;
        if ( tmv[ i ]->m_PlateNum == -1 )
        {
            plate = "_S";
        }
        else
        {
            if ( tmv[i]->m_SurfType == vsp::NORMAL_SURF )
            {
                if ( tmv[ i ]->m_PlateNum == 0 )
                {
                    plate = "_V";
                }
                else if ( tmv[ i ]->m_PlateNum == 1 )
                {
                    plate = "_H";
                }
            }
            else // WING_SURF with m_TMeshVec[ i ]->m_PlateNum == 0
            {
                plate = "_C";
            }

        }

        names.push_back( tmv[i]->m_NameStr + plate + "_Surf" + to_string( ( long long )tmv[i]->m_SurfNum ) );
    }

    return names;
}

vector< string > GetTMeshIDs( vector<TMesh*> &tmv )
{
    vector< string > ids;
    for ( int i = 0; i < (int)tmv.size(); i++ )
    {
        string plate;
        if ( tmv[ i ]->m_PlateNum == -1 )
        {
            // plate; // empty string.
        }
        else
        {
            if ( tmv[i]->m_SurfType == vsp::NORMAL_SURF )
            {
                if ( tmv[ i ]->m_PlateNum == 0 )
                {
                    plate = "_V";
                }
                else if ( tmv[ i ]->m_PlateNum == 1 )
                {
                    plate = "_H";
                }
            }
            else // WING_SURF with m_TMeshVec[ i ]->m_PlateNum == 0
            {
                plate = "_C";
            }

        }

        ids.push_back( tmv[i]->m_OriginGeomID + plate + "_Surf" + to_string((long long)tmv[i]->m_SurfNum ) );
    }

    return ids;
}

vector< int > GetTMeshThicks( vector<TMesh*> &tmv )
{
    vector < int > thick;

    for ( int i = 0; i < (int)tmv.size(); i++ )
    {
        thick.push_back( tmv[i]->m_ThickSurf );
    }

    return thick;
}

vector< int > GetTMeshTypes( vector<TMesh*> &tmv )
{
    vector< int > type;
    for ( int i = 0; i < (int)tmv.size(); i++ )
    {
        type.push_back( tmv[i]->m_SurfType );
    }

    return type;
}

vector< int > GetTMeshPlateNum( vector<TMesh*> &tmv )
{
    vector< int > plate;
    for ( int i = 0; i < (int)tmv.size(); i++ )
    {
        plate.push_back( tmv[i]->m_PlateNum );
    }

    return plate;
}

vector< int > GetTMeshCopyIndex( vector<TMesh*> &tmv )
{
    vector< int > copy_indx;
    for ( int i = 0; i < (int)tmv.size(); i++ )
    {
        copy_indx.push_back( tmv[i]->m_CopyIndex );
    }

    return copy_indx;
}

vector< double > GetTMeshWmins( vector<TMesh*> &tmv )
{
    vector < double > wmin( tmv.size(), 0.0 );
    for ( int i = 0; i < (int)tmv.size(); i++ )
    {
        wmin[i] = tmv[i]->m_Wmin;
    }

    return wmin;
}

vector< double > GetTMeshUscale( vector<TMesh*> &tmv )
{
    vector< double > uscale( tmv.size(), 0.0 );
    for ( int i = 0; i < (int)tmv.size(); i++ )
    {
        uscale[i] = tmv[i]->m_Uscale;
    }

    return uscale;
}

vector< double > GetTMeshWscale( vector<TMesh*> &tmv )
{
    vector< double > wscale( tmv.size(), 0.0 );
    for ( int i = 0; i < (int)tmv.size(); i++ )
    {
        wscale[i] = tmv[i]->m_Wscale;
    }

    return wscale;
}

set< string > GetTMeshPtrIDs( const vector<TMesh*> &tmv )
{
    set< string > ids;
    for ( size_t i = 0; i < tmv.size(); i++ )
    {
        ids.insert( tmv[i]->m_OriginGeomID );
    }
    return ids;
}

void SubTagTris( bool tag_subs, vector<TMesh*> &tmv, const vector < string > & sub_vec )
{
    // Clear out the current Subtag Maps
    SubSurfaceMgr.ClearTagMaps();
    SubSurfaceMgr.m_CompNames = GetTMeshNames( tmv );
    SubSurfaceMgr.m_CompIDs = GetTMeshIDs( tmv );
    SubSurfaceMgr.m_CompTypes = GetTMeshTypes( tmv );
    SubSurfaceMgr.m_CompPlate = GetTMeshPlateNum( tmv );
    SubSurfaceMgr.m_CompCopyIndex = GetTMeshCopyIndex( tmv );
    SubSurfaceMgr.m_CompWmin = GetTMeshWmins( tmv );
    SubSurfaceMgr.m_CompUscale = GetTMeshUscale( tmv );
    SubSurfaceMgr.m_CompWscale = GetTMeshWscale( tmv );
    SubSurfaceMgr.m_CompThick = GetTMeshThicks( tmv );
    SubSurfaceMgr.SetSubSurfTags( tmv.size() );
    SubSurfaceMgr.BuildCompNameMap();
    SubSurfaceMgr.BuildCompIDMap();

    for ( int i = 0 ; i < ( int )tmv.size() ; i++ )
    {
        tmv[i]->SubTag( i + 1, tag_subs, sub_vec );
    }

    SubSurfaceMgr.BuildSingleTagMap();
}

void RefreshTagMaps( vector<TMesh*> &tmv )
{
    SubSurfaceMgr.PartialClearTagMaps();

    for ( int i = 0 ; i < ( int )tmv.size() ; i++ )
    {
        tmv[i]->RefreshTagMap();
    }

    SubSurfaceMgr.BuildSingleTagMap();
}


//===== Vectors of TMeshs with Bounding Boxes Already Set Up ====//
bool CheckIntersect( const vector<TMesh*> & tmesh_vec, const vector<TMesh*> & other_tmesh_vec )
{
    for ( int i = 0 ; i < (int)tmesh_vec.size() ; i++ )
    {
        for ( int j = 0 ; j < (int)other_tmesh_vec.size() ; j++ )
        {
            if ( tmesh_vec[i]->CheckIntersect( other_tmesh_vec[j] ) )
            {
                return true;
            }
        }
    }

    return false;
}

bool CheckIntersect( const vector<TMesh*> & tmesh_vec, const vec3d &org, const vec3d &norm )
{
    for ( int i = 0 ; i < (int)tmesh_vec.size() ; i++ )
    {
        if ( tmesh_vec[i]->CheckIntersect( org, norm ) )
        {
            return true;
        }
    }

    return false;
}

//===== Vectors of TMeshs with Bounding Boxes Already Set Up ====//
bool CheckIntersect( Geom* geom_ptr, const vector<TMesh*> & other_tmesh_vec )
{
    bool intsect_flag = false;

    vector< TMesh* > tmesh_vec = geom_ptr->CreateTMeshVec( false );
    for ( int i = 0 ; i < (int)tmesh_vec.size() ; i++ )
    {
        tmesh_vec[i]->LoadBndBox();
    }

    intsect_flag = CheckIntersect( tmesh_vec, other_tmesh_vec );

    for ( int i = 0 ; i < (int)tmesh_vec.size() ; i++ )
    {
        delete tmesh_vec[i];
    }

    return intsect_flag;
}

bool CheckSelfIntersect( const vector<TMesh*> & tmesh_vec )
{
    for ( int i = 0 ; i < ( int )tmesh_vec.size() ; i++ )
    {
        tmesh_vec[i]->LoadBndBox();
    }

    for ( int i = 0 ; i < (int)tmesh_vec.size() - 1 ; i++ )
    {
        for ( int j = i + 1 ; j < (int)tmesh_vec.size() ; j++ )
        {
            if ( tmesh_vec[i]->CheckIntersect( tmesh_vec[j] ) )
            {
                return true;
            }
        }
    }

    return false;
}

//==== Returns Large Neg Number If Error and 0.0 If Collision ====//
double FindMinDistance( const vector< TMesh* > & tmesh_vec, const vector< TMesh* > & other_tmesh_vec, bool & intersect_flag )
{
    intersect_flag = false;

    if ( CheckIntersect( tmesh_vec, other_tmesh_vec ) )
    {
        intersect_flag = true;
        return 0.0;
    }

    //==== Find Min Dist ====//
    double min_dist = 1.0e12;
    vec3d p1, p2;
    for ( int i = 0 ; i < (int)tmesh_vec.size() ; i++ )
    {
        for ( int j = 0 ; j < (int)other_tmesh_vec.size() ; j++ )
        {
            vec3d p1a, p2a;
            double d =  tmesh_vec[i]->MinDistance(  other_tmesh_vec[j], min_dist, p1a, p2a );
            if ( d < min_dist )
            {
                min_dist = d;
                p1 = p1a;
                p2 = p2a;
            }
        }
    }

    return min_dist;
}

//===== Find The Min Distance For Each Point And Returns Max =====//
double FindMaxMinDistance( const vector< TMesh* > & mesh_vec_1, const vector< TMesh* > & mesh_vec_2 )
{
    double max_dist = 0.0;

    if ( mesh_vec_1.size() != mesh_vec_2.size() )
        return max_dist;

    for ( int i = 0 ; i < (int)mesh_vec_1.size() ; i++ )
    {
        TMesh* tm1 = mesh_vec_1[i];
        TMesh* tm2 = mesh_vec_2[i];

        if ( tm1->m_NVec.size() == tm2->m_NVec.size() )
        {
            for ( int m = 0 ; m < tm1->m_NVec.size() ; m++ )
            {
                double d2 = dist_squared( tm1->m_NVec[m]->m_Pnt, tm2->m_NVec[m]->m_Pnt );
                max_dist = max( max_dist, d2 );
            }
        }
    }
    return sqrt( max_dist );
}

void DiscreteVisibility( vector < TMesh* > & primary_tmv, const vector < double > &azvec, const vector < double > & elvec, const vector < vec3d > & cen_vec, const string & resid, const vector<string> & cutout_vec  )
{
    bool intSubsFlag = !cutout_vec.empty();
    CSGMesh( primary_tmv, intSubsFlag, cutout_vec );

    for ( int i = 0; i < ( int )primary_tmv.size(); i++ )
    {
        primary_tmv[i]->SetIgnoreSubSurface();
    }
    FlattenTMeshVec( primary_tmv );

    for ( int i = 0; i < ( int )primary_tmv.size(); i++ )
    {
        primary_tmv[i]->LoadBndBox();
    }

    vector < vec3d > dir_vec;
    vector < double > dviz_vec, dmiss_vec;
    vector < int > viz_vec;
    vector < vec3d > pts;
    double dviz_sum = 0;
    for ( int icen = 0; icen < ( int )cen_vec.size(); icen++ )
    {
        const vec3d &cen = cen_vec[ icen ];
        for ( int i = 0; i < azvec.size(); ++i )
        {
            vec3d dir = -ToCartesian( vec3d( 1.0, -azvec[i] * M_PI / 180.0, -elvec[i] * M_PI / 180.0 ) );

            vector < double > tParmVec;
            vector < TTri* > triVec;

            for ( int j = 0; j < ( int )primary_tmv.size(); j++ )
            {
                primary_tmv[j]->m_TBox.RayCast( cen, dir, tParmVec, triVec );
            }

            int viz = 0;
            double dviz = 0;
            double dmiss = 1.0e12;
            if ( !tParmVec.empty() )
            {
                std::sort( tParmVec.begin(), tParmVec.end() );
                dviz = tParmVec[0];
                dmiss = 0;
                dviz_sum += dviz;

                pts.push_back( cen );
                pts.push_back( cen + dviz * dir );
            }
            else
            {
                viz = 1;
                vector < vec3d > closest_distpts(2);
                for ( int j = 0; j < ( int )primary_tmv.size(); j++ )
                {
                    vector < vec3d > distpts(2);
                    double dm = primary_tmv[j]->MinDistanceRay( cen, dir, 1e12, distpts[0], distpts[1] );

                    if ( dm < dmiss )
                    {
                        dmiss = dm;
                        closest_distpts[0] = distpts[0];
                        closest_distpts[1] = distpts[1];

                        dviz = -dist( cen, distpts[0] );
                    }
                }

                pts.push_back( closest_distpts[0] );
                pts.push_back( closest_distpts[1] );
            }

            dir_vec.push_back( dir );
            dviz_vec.push_back( dviz );
            dmiss_vec.push_back( dmiss );
            viz_vec.push_back( viz );
        }
    }

    //==== Create Results ====//
    Results *res = ResultsMgr.FindResultsPtr( resid );
    if ( res )
    {
        res->Add( new NameValData( "Center", cen_vec, "Visibility center point." ) );
        res->Add( new NameValData( "Pts", pts, "Visibility test endpoints." ) );

        res->Add( new NameValData( "Dirs", dir_vec, "Directions of visibility check." ) );
        res->Add( new NameValData( "DistVisible", dviz_vec, "Visibility distance." ) );
        res->Add( new NameValData( "DistMiss", dmiss_vec, "Miss distance of visibility." ) );
        res->Add( new NameValData( "Visible", viz_vec, "Visibility flag vector (0 occluded, 1 clear line of sight)." ) );

        res->Add( new NameValData( "Result", dviz_sum, "Point visibility result" ) );
    }

}

void LookAtVisibility( TMesh *primary_tm, const vec3d & dir, const double n2, const string & resid, vector< TMesh* > & result_tmv )
{
    Vehicle *veh = VehicleMgr.GetVehicle();
    if ( !veh )
    {
        return;
    }

    BndBox bbox;
    primary_tm->UpdateBBox( bbox );

    double dist = 1.1 * bbox.DiagDist();

    vec3d disp = dir;
    disp.normalize();
    disp *= dist;


    TMesh *sv = CreateTMeshPGMeshSweptVolumeTranslate( primary_tm, disp );
    delete primary_tm;

    // visible from
    sv->SetIgnoreShadow( disp, 1e-6 );
    sv->FlattenInPlace();

    sv->ComputeVisibleArea( dir, n2 );

    int ntags = -1;
    int nss = -1;
    int nsurf = -1;

    vector < string > surfNameVec;
    vector < string > surfIDVec;

    vector < string > tagNameVec;
    vector < string > tagIDVec;
    vector < vector < string > > vecTagIDVec;

    vector < string > ssNameVec;
    vector < string > ssIDVec;

    // Subtract off dummy tag.
    ntags = SubSurfaceMgr.GetNumTags() - 1;

    tagNameVec.resize( ntags );
    tagIDVec.resize( ntags );
    vecTagIDVec.resize( ntags );

    for ( int j = 0; j < ntags; j++ )
    {
        tagNameVec[j] = SubSurfaceMgr.GetTagNames( j );
        tagIDVec[j] = SubSurfaceMgr.GetTagIDs( j );
        vecTagIDVec[j] = SubSurfaceMgr.GetVecTagIDs( j );
    }

    // populate individual subsurfs
    for ( int j = 0; j < vecTagIDVec.size(); j++ )
    {
        string gtagid = vecTagIDVec[j][0];

        // add to surf ID vec if not already in there
        if ( find( surfIDVec.begin(), surfIDVec.end(), gtagid ) == surfIDVec.end() )
        {
            int pos = gtagid.find_first_of( '_' );

            string gid = gtagid.substr( 0, pos );
            string surf = gtagid.substr( pos + 1 );

            Geom *g = veh->FindGeom( gid );
            if ( g )
            {
                surfNameVec.push_back( g->GetName() + "," + surf );
                surfIDVec.push_back( gtagid );
            }
        }

        // start at iter 1, skip the geom id since all subsurf ids are unique anyways
        for ( int k = 1; k < vecTagIDVec[j].size(); k++ )
        {
            // add to subsurf ID vec if not already in there
            if ( find( ssIDVec.begin(), ssIDVec.end(), vecTagIDVec[j][k] ) == ssIDVec.end() )
            {
                SubSurface* ss = SubSurfaceMgr.GetSubSurf( vecTagIDVec[j][k] );
                if ( ss )
                {
                    ParmContainer* pc = ss->GetParentContainerPtr();
                    if ( pc )
                    {
                        string full_name = pc->GetName() + "," + ss->GetName();
                        ssIDVec.push_back( vecTagIDVec[j][k] );
                        ssNameVec.push_back( full_name );
                    }
                }
            }
        }
    }

    nss = ssIDVec.size();
    nsurf = surfIDVec.size();

    vector < double > surfWetAreaVec;
    vector < double > surfProjAreaVec;
    vector < double > surfSolarAreaVec;

    vector < double > tagWetAreaVec;
    vector < double > tagProjAreaVec;
    vector < double > tagSolarAreaVec;

    vector < double > ssWetAreaVec;
    vector < double > ssProjAreaVec;
    vector < double > ssSolarAreaVec;

    surfWetAreaVec.resize( nsurf, 0.0 );
    surfProjAreaVec.resize( nsurf, 0.0 );
    surfSolarAreaVec.resize( nsurf, 0.0 );

    tagWetAreaVec.resize( ntags, 0.0 );
    tagProjAreaVec.resize( ntags, 0.0 );
    tagSolarAreaVec.resize( ntags, 0.0 );

    ssWetAreaVec.resize( nss, 0.0 );
    ssProjAreaVec.resize( nss, 0.0 );
    ssSolarAreaVec.resize( nss, 0.0 );

    double wet;
    double proj;
    double solar;

    for ( int j = 0; j < ntags; j++ )
    {
        wet = sv->m_TagWetAreaVec[j];
        proj = sv->m_TagProjAreaVec[j];
        solar = sv->m_TagSolarAreaVec[j];

        tagWetAreaVec[j] += wet;
        tagProjAreaVec[j] += proj;
        tagSolarAreaVec[j] += solar;

        string gtagid = vecTagIDVec[j][0];

        vector < string >::iterator surf_iter = find( surfIDVec.begin(), surfIDVec.end(), gtagid );
        int surf_index = -1;
        if ( surf_iter != surfIDVec.end() )
        {
            surf_index = surf_iter - surfIDVec.begin();
        }
        if ( surf_index > -1 )
        {
            surfWetAreaVec[surf_index] += wet;
            surfProjAreaVec[surf_index] += proj;
            surfSolarAreaVec[surf_index] += solar;
        }

        for ( int k = 1; k < vecTagIDVec[j].size(); k++ )
        {
            vector < string >::iterator ssid_iter = find( ssIDVec.begin(), ssIDVec.end(), vecTagIDVec[j][k] );
            int ss_index = -1;
            if ( ssid_iter != ssIDVec.end() )
            {
                ss_index = ssid_iter - ssIDVec.begin();
            }
            if ( ss_index > -1 )
            {
                ssWetAreaVec[ss_index] += wet;
                ssProjAreaVec[ss_index] += proj;
                ssSolarAreaVec[ss_index] += solar;
            }
        }
    }


    result_tmv.push_back( sv );

    Results *res = ResultsMgr.FindResultsPtr( resid );
    if( res )
    {
        // Populate results.
        res->Add( new NameValData( "Num_Surfs", nsurf, "Number of surfs." ) );
        res->Add( new NameValData( "Surf_Name", surfNameVec, "Surf names." ) );
        res->Add( new NameValData( "Surf_ID", surfIDVec, "Surf IDs." ) );
        res->Add( new NameValData( "Surf_Wet_Area", surfWetAreaVec, "Visible surface area for surf." ) );
        res->Add( new NameValData( "Surf_Proj_Area", surfProjAreaVec, "Projected visible area for surf." ) );
        res->Add( new NameValData( "Surf_Solar_Area", surfSolarAreaVec, "Equivalent solar area for surf." ) );

        res->Add( new NameValData( "Num_Tags", ntags, "Number of tags." ) );
        res->Add( new NameValData( "Tag_Name", tagNameVec, "Tag names." ) );
        res->Add( new NameValData( "Tag_ID", tagIDVec, "Tag IDs." ) );
        res->Add( new NameValData( "Tag_Wet_Area", tagWetAreaVec, "Visible surface area for tag." ) );
        res->Add( new NameValData( "Tag_Proj_Area", tagProjAreaVec, "Projected visible area for tag." ) );
        res->Add( new NameValData( "Tag_Solar_Area", tagSolarAreaVec, "Equivalent solar area for tag." ) );

        res->Add( new NameValData( "Num_SubSurfs", nss, "Number of SubSurfs." ) );
        res->Add( new NameValData( "SubSurf_Name", ssNameVec, "SubSurf names." ) );
        res->Add( new NameValData( "SubSurf_ID", ssIDVec, "SubSurf IDs." ) );
        res->Add( new NameValData( "SubSurf_Wet_Area", ssWetAreaVec, "Visible surface area for SubSurf." ) );
        res->Add( new NameValData( "SubSurf_Proj_Area", ssProjAreaVec, "Projected visible area for SubSurf." ) );
        res->Add( new NameValData( "SubSurf_Solar_Area", ssSolarAreaVec, "Equivalent solar area for SubSurf." ) );

        res->Add( new NameValData( "Total_Wet_Area", sv->m_WetArea, "Visible surface area." ) );
        res->Add( new NameValData( "Total_Proj_Area", sv->m_ProjArea, "Projected visible area." ) );
        res->Add( new NameValData( "Total_Solar_Area", sv->m_SolarArea, "Equivalent solar area." ) );

        res->Add( new NameValData( "Result", sv->m_ProjArea, "Interference result" ) );
    }
}

void PlaneInterferenceCheck( TMesh *primary_tm, const vec3d & org, const vec3d & norm, const string & resid, vector< TMesh* > & result_tmv )
{
    bool intersect_flag = false;
    bool interference_flag = false;

    double vol_primary = primary_tm->ComputeTheoVol();

    double min_dist = 1.0e12;
    double max_dist = -1.0e12;
    double con_dist = 1.0e12;
    double con_vol = -1; // Not the true volume.
    double vol = 0;

    vector < vec3d > pts( 2 );

    double vref = vol_primary;

    vector < vec3d > threepts;
    MakeThreePts( org, norm, threepts );

    max_dist = primary_tm->MaxDistance( org, norm, max_dist, pts[0], pts[1] );

    if ( primary_tm->CheckIntersect( org, norm ) )
    {
        intersect_flag = true;
        interference_flag = true;

        // Add origin to primary mesh bounding box.
        BndBox bb = primary_tm->m_TBox.m_Box;
        bb.Update( org );
        double len = bb.DiagDist() * 2.1;

        TMesh * slice = MakeSlice( org, norm, len );

        result_tmv.push_back( slice );
        result_tmv.push_back( primary_tm );

        MeshCutAbovePlane( result_tmv, threepts );
        FlattenTMeshVec( result_tmv ); // Not required for volume calculations, do it for visualization and later use.

        min_dist = 0.0;
        con_dist = 1.0;

        for ( int i = 0; i < result_tmv.size(); i++ )
        {
            vol += result_tmv[i]->ComputeTrimVol();
        }
        con_vol = vol / vref;
    }
    else
    {
        pts.resize( 4 );
        min_dist = primary_tm->MinDistance( org, norm, min_dist, pts[2], pts[3] );
        con_dist = min_dist;

        if ( !primary_tm->m_TVec.empty() )
        {
            TTri *tri = primary_tm->m_TVec[0];
            vec3d cen = tri->ComputeCenter();


            if ( orient3d( (double*) threepts[0].v, (double*) threepts[1].v, (double*) threepts[2].v, (double*) cen.v ) > 0 )
            {
                interference_flag = true;
                result_tmv.push_back( primary_tm );
                con_vol = 1;
                con_dist += 1.0;
            }
        }
    }

    double gcon = con_dist * con_vol;

    Results *res = ResultsMgr.FindResultsPtr( resid );
    if( res )
    {
        // Populate results.
        res->Add( new NameValData( "Interference", interference_flag, "Flag indicating the primary and secondary interfere." ) );
        res->Add( new NameValData( "Intersection", intersect_flag, "Flag indicating the primary and secondary intersect." ) );
        res->Add( new NameValData( "Min_Dist", min_dist, "Minimum distance between primary and secondary." ) );
        res->Add( new NameValData( "Max_Dist", max_dist, "Maximum distance between primary and secondary." ) );
        res->Add( new NameValData( "Pts", pts, "Min/max distance line end points." ) );
        res->Add( new NameValData( "InterferenceVol", vol, "Volume of interference." ) );
        res->Add( new NameValData( "Con_Val", gcon, "Constraint value" ) );
        res->Add( new NameValData( "Result", gcon, "Interference result" ) );
    }
}

void CCEInterferenceCheck(  TMesh *primary_tm, TMesh *secondary_tm, const string & resid, vector< TMesh* > & result_tmv )
{
    bool intersect_flag = false;
    bool interference_flag = false;

    double vol_primary = primary_tm->ComputeTheoVol();

    double min_dist = 1.0e12;
    double con_dist = 1.0e12;
    double con_vol = -1; // Not the true volume.
    double vol = 0;

    bool primary_below_secondary = false;
    vector < vec3d > pts;

    if ( primary_tm->CheckIntersect( secondary_tm ) )
    {
        intersect_flag = true;
        interference_flag = true;

        result_tmv.push_back( secondary_tm );
        result_tmv.push_back( primary_tm );

        MeshCCEIntersect( result_tmv );
        FlattenTMeshVec( result_tmv ); // Not required for volume calculations, do it for visualization and later use.

        min_dist = 0.0;
        con_dist = 1.0;

        for ( int i = 0; i < result_tmv.size(); i++ )
        {
            vol += result_tmv[i]->ComputeTrimVol();
        }
        con_vol = vol / vol_primary;
    }
    else
    {
        pts.resize( 2 );
        min_dist = primary_tm->MinDistance( secondary_tm, min_dist, pts[0], pts[1] );
        con_dist = min_dist;

        if ( !primary_tm->m_TVec.empty() && !secondary_tm->m_TVec.empty() )
        {
            TTri *trip = primary_tm->m_TVec[0];
            vec3d upish( 0.000001, 0.000001, 1.0 );

            if ( DeterIntExtTri( trip, secondary_tm, upish ) ) // a inside b
            {
                primary_below_secondary = true;
                interference_flag = true;
                result_tmv.push_back( primary_tm );
                con_vol = 1;
                con_dist += 1.0;
                delete secondary_tm;
            }
            else
            {
                delete primary_tm;
                delete secondary_tm;
            }
        }
        else
        {
            delete primary_tm;
            delete secondary_tm;
        }
    }

    double gcon = con_dist * con_vol;

    Results *res = ResultsMgr.FindResultsPtr( resid );
    if( res )
    {
        // Populate results.
        res->Add( new NameValData( "Interference", interference_flag, "Flag indicating the primary and secondary interfere." ) );
        res->Add( new NameValData( "Intersection", intersect_flag, "Flag indicating the primary and secondary intersect." ) );
        res->Add( new NameValData( "Primary_Below_Secondary", primary_below_secondary, "Flag indicating the primary is contained below the secondary." ) );
        res->Add( new NameValData( "Min_Dist", min_dist, "Minimum distance between primary and secondary." ) );
        res->Add( new NameValData( "Pts", pts, "Min/max distance line end points." ) );
        res->Add( new NameValData( "InterferenceVol", vol, "Volume of interference." ) );
        res->Add( new NameValData( "Vol_Primary", vol_primary, "Volume of primary." ) );
        res->Add( new NameValData( "Con_Val", gcon, "Constraint value" ) );
        res->Add( new NameValData( "Result", gcon, "Interference result" ) );
    }

}

void SweptVolumeInterferenceCheck(  TMesh *primary_tm, TMesh *secondary_tm, const vector < vec3d > & dispvec, const string & resid, vector< TMesh* > & result_tmv )
{
    TMesh * swept_secondary_tm = CreateTMeshPGMeshSweptVolumeTranslate( secondary_tm, dispvec );
    delete secondary_tm;
    swept_secondary_tm->LoadBndBox();

    TMesh * swept_secondary_tm_copy = new TMesh();
    swept_secondary_tm_copy->CopyFlatten( swept_secondary_tm );

    ExteriorInterferenceCheck( primary_tm, swept_secondary_tm, resid, result_tmv );

    // Place swept volume copy at start of result_tmv
    result_tmv.insert( result_tmv.begin(), swept_secondary_tm_copy );
}

void ExteriorInterferenceCheck( TMesh *primary_tm, TMesh *secondary_tm, const string & resid, vector< TMesh* > & result_tmv )
{
    bool intersect_flag = false;
    bool interference_flag = false;

    double vol_primary = primary_tm->ComputeTheoVol();
    double vol_secondary = secondary_tm->ComputeTheoVol();

    double min_dist = 1.0e12;
    double con_dist = 1.0e12;
    double con_vol = -1; // Not the true volume.
    double vol = 0;

    bool primary_in_secondary = false;
    bool secondary_in_primary = true;
    vector < vec3d > pts;


    double vref = min( vol_primary, vol_secondary );

    if ( primary_tm->CheckIntersect( secondary_tm ) )
    {
        intersect_flag = true;
        interference_flag = true;

        result_tmv.push_back( primary_tm );
        result_tmv.push_back( secondary_tm );
        MeshIntersect( result_tmv );
        FlattenTMeshVec( result_tmv ); // Not required for volume calculations, do it for visualization and later use.

        TMesh *result_tm = MergeTMeshVec( result_tmv );
        DeleteTMeshVec( result_tmv );
        result_tmv.push_back( result_tm );

        min_dist = 0.0;
        con_dist = 1.0;

        for ( int i = 0; i < result_tmv.size(); i++ )
        {
            vol += result_tmv[i]->ComputeTrimVol();
        }
        con_vol = vol / vref;
    }
    else
    {
        pts.resize( 2 );
        min_dist = primary_tm->MinDistance( secondary_tm, min_dist, pts[0], pts[1] );
        con_dist = min_dist;

        if ( !primary_tm->m_TVec.empty() && !secondary_tm->m_TVec.empty() )
        {
            TTri *trip = primary_tm->m_TVec[0];
            TTri *tris = secondary_tm->m_TVec[0];


            if ( DeterIntExtTri( trip, secondary_tm ) ) // a inside b
            {
                primary_in_secondary = true;
                interference_flag = true;
                result_tmv.push_back( primary_tm );
                con_vol = 1;
                con_dist += 1.0;
                delete secondary_tm;
            }
            else if ( DeterIntExtTri( tris, primary_tm ) ) // b inside a
            {
                secondary_in_primary = true;
                interference_flag = true;
                result_tmv.push_back( secondary_tm );
                con_vol = 1;
                con_dist += 1.0;
                delete primary_tm;
            }
            else
            {
                delete primary_tm;
                delete secondary_tm;
            }
        }
        else
        {
            delete primary_tm;
            delete secondary_tm;
        }
    }

    double gcon = con_dist * con_vol;

    Results *res = ResultsMgr.FindResultsPtr( resid );
    if( res )
    {
        // Populate results.
        res->Add( new NameValData( "Interference", interference_flag, "Flag indicating the primary and secondary interfere." ) );
        res->Add( new NameValData( "Intersection", intersect_flag, "Flag indicating the primary and secondary intersect." ) );
        res->Add( new NameValData( "Primary_In_Secondary", primary_in_secondary, "Flag indicating the primary is contained within the secondary." ) );
        res->Add( new NameValData( "Secondary_In_Primary", secondary_in_primary, "Flag indicating the secondary is contained within the primary." ) );
        res->Add( new NameValData( "Min_Dist", min_dist, "Minimum distance between primary and secondary." ) );
        res->Add( new NameValData( "Pts", pts, "Minimum distance line end points." ) );
        res->Add( new NameValData( "InterferenceVol", vol, "Volume of interference." ) );
        res->Add( new NameValData( "Vol_Primary", vol_primary, "Volume of primary." ) );
        res->Add( new NameValData( "Vol_Secondary", vol_secondary, "Volume of secondary." ) );
        res->Add( new NameValData( "Con_Val", gcon, "Constraint value" ) );
        res->Add( new NameValData( "Result", gcon, "Interference result" ) );
    }
}

void ExteriorInterferenceCheck( vector< TMesh* > & primary_tmv, vector< TMesh* > & secondary_tmv, const string & resid, vector< TMesh* > & result_tmv )
{
    CSGMesh( primary_tmv );
    FlattenTMeshVec( primary_tmv );
    TMesh *primary_tm = MergeTMeshVec( primary_tmv );
    primary_tm->LoadBndBox();

    CSGMesh( secondary_tmv );
    FlattenTMeshVec( secondary_tmv );
    TMesh *secondary_tm = MergeTMeshVec( secondary_tmv );
    secondary_tm->LoadBndBox();

    ExteriorInterferenceCheck( primary_tm, secondary_tm, resid, result_tmv );
}

string PackagingInterferenceCheck( vector< TMesh* > & primary_tmv, vector< TMesh* > & secondary_tmv, vector< TMesh* > & result_tmv )
{
    bool intersect_flag = false;
    bool interference_flag = false;

    CSGMesh( primary_tmv );
    FlattenTMeshVec( primary_tmv );
    TMesh *primary_tm = MergeTMeshVec( primary_tmv );
    primary_tm->LoadBndBox();
    double vol_primary = primary_tm->ComputeTheoVol();

    CSGMesh( secondary_tmv );
    FlattenTMeshVec( secondary_tmv );
    TMesh *secondary_tm = MergeTMeshVec( secondary_tmv );
    secondary_tm->LoadBndBox();
    double vol_secondary = secondary_tm->ComputeTheoVol();


    double min_dist = 1.0e12;
    double con_dist = 1.0e12;
    double con_vol = -1; // Not the true volume.
    double vol = 0;

    bool primary_in_secondary = false;
    bool secondary_in_primary = true;
    vector < vec3d > pts;


    double vref = vol_secondary; // min( vol_primary, vol_secondary );

    if ( primary_tm->CheckIntersect( secondary_tm ) )
    {
        intersect_flag = true;
        interference_flag = true;

        // Notice secondary first.
        result_tmv.push_back( secondary_tm );
        result_tmv.push_back( primary_tm );

        MeshSubtract( result_tmv ); // Secondary - Primary
        FlattenTMeshVec( result_tmv ); // Not required for volume calculations, do it for visualization and later use.

        min_dist = 0.0;
        con_dist = 1.0;

        for ( int i = 0; i < result_tmv.size(); i++ )
        {
            // vol += result_tmv[i]->ComputeTrimVol();
            vol += result_tmv[i]->ComputeTheoVol();
        }
        con_vol = vol / vref;
    }
    else
    {
        pts.resize( 2 );
        min_dist = primary_tm->MinDistance( secondary_tm, min_dist, pts[0], pts[1] );
        con_dist = min_dist;

        if ( !primary_tm->m_TVec.empty() && !secondary_tm->m_TVec.empty() )
        {
            TTri *tri = secondary_tm->m_TVec[0];
            if ( !DeterIntExtTri( tri, primary_tm ) ) // b not inside a
            {
                secondary_in_primary = true;
                interference_flag = true;
                result_tmv.push_back( secondary_tm );
                con_vol = 1;
                con_dist += 1.0;
                delete primary_tm;
            }
            else
            {
                delete primary_tm;
                delete secondary_tm;
            }
        }
        else
        {
            delete primary_tm;
            delete secondary_tm;
        }
    }

    double gcon = con_dist * con_vol;

    Results *res = ResultsMgr.CreateResults( "Packaging_Interference", "Packaging interference check." );
    if( res )
    {
        // Populate results.
        res->Add( new NameValData( "Interference", interference_flag, "Flag indicating the primary and secondary interfere." ) );
        res->Add( new NameValData( "Intersection", intersect_flag, "Flag indicating the primary and secondary intersect." ) );
        res->Add( new NameValData( "Primary_In_Secondary", primary_in_secondary, "Flag indicating the primary is contained within the secondary." ) );
        res->Add( new NameValData( "Secondary_In_Primary", secondary_in_primary, "Flag indicating the secondary is contained within the primary." ) );
        res->Add( new NameValData( "Min_Dist", min_dist, "Minimum distance between primary and secondary." ) );
        res->Add( new NameValData( "Pts", pts, "Minimum distance line end points." ) );
        res->Add( new NameValData( "InterferenceVol", vol, "Volume of interference." ) );
        res->Add( new NameValData( "Vol_Primary", vol_primary, "Volume of primary." ) );
        res->Add( new NameValData( "Vol_Secondary", vol_secondary, "Volume of secondary." ) );
        res->Add( new NameValData( "Con_Val", gcon, "Constraint value" ) );
        res->Add( new NameValData( "Result", gcon, "Interference result" ) );
    }

    return res->GetID();
}

string ExteriorSelfInterferenceCheck( vector< TMesh* > & primary_tmv, vector< TMesh* > & result_tmv )
{
    bool intersect_flag = false;
    bool interference_flag = false;

    double min_dist = 1.0e12;
    double con_dist = 1.0e12;
    double con_vol = -1; // Not the true volume.
    double vol = 0;

    vector < vec3d > pts;

    double vref = 1.0e12;
    for ( int i = 0 ; i < ( int )primary_tmv.size() ; i++ )
    {
        vref = min( vref, primary_tmv[i]->ComputeTheoVol() );
    }

    if ( CheckSelfIntersect( primary_tmv ) )
    {
        intersect_flag = true;
        interference_flag = true;

        result_tmv = CopyTMeshVec( primary_tmv );

        MeshIntersect( result_tmv );
        FlattenTMeshVec( result_tmv ); // Not required for volume calculations, do it for visualization and later use.

        min_dist = 0.0;
        con_dist = 1.0;

        for ( int i = 0; i < result_tmv.size(); i++ )
        {
            vol += result_tmv[i]->ComputeTrimVol();
        }
        con_vol = vol / vref;
    }
    else
    {
        pts.resize( 2 );
        for ( int i = 0 ; i < (int)primary_tmv.size() - 1 ; i++ )
        {
            for ( int j = i + 1 ; j < (int)primary_tmv.size() ; j++ )
            {
                min_dist = primary_tmv[i]->MinDistance( primary_tmv[j], min_dist, pts[0], pts[1] );
            }
        }
        con_dist = min_dist;
    }

    double gcon = con_dist * con_vol;

    Results *res = ResultsMgr.CreateResults( "External_Self_Interference", "External self interference check." );
    if( res )
    {
        // Populate results.
        res->Add( new NameValData( "Interference", interference_flag, "Flag indicating the primary and secondary interfere." ) );
        res->Add( new NameValData( "Intersection", intersect_flag, "Flag indicating the primary and secondary intersect." ) );
        res->Add( new NameValData( "Min_Dist", min_dist, "Minimum distance between primary and secondary." ) );
        res->Add( new NameValData( "Pts", pts, "Minimum distance line end points." ) );
        res->Add( new NameValData( "InterferenceVol", vol, "Volume of interference." ) );
        res->Add( new NameValData( "Con_Val", gcon, "Constraint value" ) );
        res->Add( new NameValData( "Result", gcon, "Interference result" ) );
    }

    return res->GetID();
}

bool DecideIgnoreTri( int aType, const vector < int > & bTypes, const vector < bool > & thicksurf, const vector < bool > & aInB )
{
    // Always delete Stiffener tris
    if ( aType == vsp::CFD_STIFFENER )
    {
        return true;
    }

    bool aInOneNormal = false;

    for ( int b = 0 ; b < ( int )aInB.size() ; b++ )
    {
        bool aInThisB = aInB[b];
        int bType = bTypes[b];
        bool bThick = thicksurf[b];

        // Can make absolute decisions about deleting a triangle or not in the cases below
        if ( aInThisB && bThick )
        {
            if( bType == vsp::CFD_NORMAL )
            {
                aInOneNormal = true;
            }

            // Normal(Positive) inside another Normal, or Negative inside another Negative
            if ( aType == bType && ( aType != vsp::CFD_TRANSPARENT && aType != vsp::CFD_STRUCTURE ) )
            {
                return true;
            }
            // Always delete Normal tris inside Negative surfaces
            else if ( aType == vsp::CFD_NORMAL && bType == vsp::CFD_NEGATIVE )
            {
                return true;
            }
            // Never delete Transparent tris inside Negative surfaces
            else if ( aType == vsp::CFD_TRANSPARENT && bType == vsp::CFD_NEGATIVE )
            {
                return false;
            }
            // Always delete Structure tris inside Negative surfaces
            else if ( aType == vsp::CFD_STRUCTURE && bType == vsp::CFD_NEGATIVE )
            {
                return true;
            }
        }
    }

    // Default condition for ignoretri.
    // The default value is applied for a triangle that is not inside
    // any other object.  I.e. an isolated thing in 'free space'.
    //
    // vsp::CFD_NORMAL, vsp::CFD_TRANSPARENT
    int ignoretri = false;

    // Flip sense of default value.  These things do not exist in 'free space'.
    if ( aType == vsp::CFD_NEGATIVE ||
         aType == vsp::CFD_STRUCTURE ||
         aType == vsp::CFD_STIFFENER ||  // Stiffener is special case -- always true previously.
         aType == vsp::CFD_MEASURE_DUCT )
    {
        ignoretri = true;
    }

    // Check non-absolute cases
    for ( int b = 0 ; b < ( int )aInB.size() ; b++ )
    {
        bool aInThisB = aInB[b];
        int bType = bTypes[b];
        bool bThick = thicksurf[b];

        if ( aInThisB && bThick )
        {
            if ( ( aType == vsp::CFD_NEGATIVE || aType == vsp::CFD_STRUCTURE ) && bType == vsp::CFD_NORMAL )
            {
                return false;
            }
            else if ( aType == vsp::CFD_TRANSPARENT && bType == vsp::CFD_NORMAL )
            {
                return true;
            }
            if ( aType == vsp::CFD_MEASURE_DUCT && aInOneNormal && bType == vsp::CFD_NEGATIVE )
            {
                return false;
            }
        }
    }

    return ignoretri;
}

double IntersectSplit( vector < TMesh * > &tmv, bool intSubsFlag, const vector < string > & sub_vec )
{
    //==== Scale To 1000 Units ====//
    BndBox bbox;
    Matrix4d mat;
    UpdateBBox( bbox, tmv, mat );

    double scalefac = 1000.0 / bbox.GetLargestDist();
    ApplyScale( scalefac, tmv );

    //==== Intersect Subsurfaces to make clean lines ====//
    if ( intSubsFlag )
    {
        for ( int i = 0 ; i < ( int )tmv.size() ; i++ )
        {
            vector < double > uvec;
            vector < double > vvec;
            if ( tmv[i]->m_UWPnts.size() > 0 && tmv[i]->m_UWPnts[0].size() > 0 )
            {
                uvec.resize( tmv[i]->m_UWPnts.size() );
                for ( int j = 0 ; j < ( int )tmv[i]->m_UWPnts.size(); j++ )
                {
                    uvec[j] = tmv[i]->m_UWPnts[j][0].x();
                }
                vvec.resize( tmv[i]->m_UWPnts[0].size() );
                for ( int j = 0; j < ( int ) tmv[ i ]->m_UWPnts[ 0 ].size(); j++ )
                {
                    vvec[j] = tmv[ i ]->m_UWPnts[ 0 ][ j ].y();
                }
            }

            vector< TMesh* > sub_surf_meshes;
            vector< SubSurface* > sub_surf_vec = SubSurfaceMgr.GetSubSurfs( tmv[i]->m_OriginGeomID, tmv[i]->m_SurfNum );
            int ss;
            for ( ss = 0 ; ss < ( int )sub_surf_vec.size() ; ss++ )
            {
                string subsurf_id = sub_surf_vec[ ss ]->GetID();

                if ( sub_vec.empty() || vector_contains_val( sub_vec, subsurf_id ) )
                {
                    vector< TMesh* > tmp_vec = sub_surf_vec[ss]->CreateTMeshVec( uvec, vvec );
                    sub_surf_meshes.insert( sub_surf_meshes.end(), tmp_vec.begin(), tmp_vec.end() );
                }
            }

            if ( !sub_surf_meshes.size() )
            {
                continue;    // Skip if no sub surface meshes
            }

            // Load All surf_mesh_bboxes
            for ( ss = 0 ; ss < ( int )sub_surf_meshes.size() ; ss++ )
            {
                // Build merge maps
                tmv[i]->BuildMergeMaps();

                sub_surf_meshes[ss]->LoadBndBox();
                // Swap the tmv[i]'s nodes to be UW instead of xyz
                tmv[i]->MakeNodePntUW();
                tmv[i]->LoadBndBox();

                // Intersect TMesh with sub_surface_meshes
                tmv[i]->Intersect( sub_surf_meshes[ss], true );

                // Split the triangles
                tmv[i]->Split();

                // Make current TMesh XYZ again and reset its octtree
                tmv[i]->MakeNodePntXYZ();
                tmv[i]->m_TBox.Reset();

                // Flatten Mesh
                TMesh* f_tmesh = new TMesh();
                f_tmesh->CopyFlatten( tmv[i] );
                delete tmv[i];
                tmv[i] = f_tmesh;
            }

            sub_surf_meshes.clear();
        }
    }
    // Tag meshes before regular intersection
    SubTagTris( (bool)intSubsFlag, tmv, sub_vec );

    //==== Check For Open Meshes and Merge or Delete Them ====//
    MeshInfo info;
    MergeRemoveOpenMeshes( tmv, &info, /* deleteopen */ false );

    //==== Create Bnd Box for  Mesh Geoms ====//
    for ( int i = 0 ; i < ( int )tmv.size() ; i++ )
    {
        tmv[i]->LoadBndBox();
    }

    //==== Intersect All Mesh Geoms ====//
    for ( int i = 0 ; i < ( int )tmv.size() ; i++ )
    {
        for ( int j = i + 1 ; j < ( int )tmv.size() ; j++ )
        {
            tmv[i]->Intersect( tmv[j] );
        }
    }

    //==== Split Intersected Tri in Mesh ====//
    for ( int i = 0 ; i < ( int )tmv.size() ; i++ )
    {
        tmv[i]->Split();
    }

    return scalefac;
}

void IntersectSplitClassify( vector < TMesh * > &tmv, bool intSubsFlag, const vector < string > & sub_vec )
{
    double scalefac = IntersectSplit( tmv, intSubsFlag, sub_vec );

    //==== Determine Which Triangle Are Interior/Exterior ====//
    for ( int i = 0 ; i < ( int )tmv.size() ; i++ )
    {
        tmv[i]->DeterIntExt( tmv );
    }

    //===== Reset Scale =====//
    ApplyScale( 1.0 / scalefac, tmv );
}

// Ensure that the first edge of the chain is oriented in increasing w order.
void NormalizeChain( vector < TEdge * > & chain )
{
    // Rotates chain such that segment with minimum w value is first.
    // A closed loop will have two such edges.  An open (in uw) will only have one.
    // It does not matter whether the wmin appears in the first or second node.
    double wmin = 1e6;
    int imin = -1;
    for ( int iedge = 0; iedge < chain.size(); iedge++ )
    {
        TEdge * e = chain[ iedge ];
        if ( e->m_N0->m_UWPnt.y() < wmin || e->m_N1->m_UWPnt.y() < wmin )
        {
            wmin = std::min( e->m_N0->m_UWPnt.y(), e->m_N1->m_UWPnt.y() );
            imin = iedge;
        }
    }
    std::rotate( chain.begin(), chain.begin() + imin, chain.end() );

    // Check to make sure that the first edge of the chain is oriented in increasing w order.
    bool flip = false;
    if ( chain.size() > 0 )
    {
        TEdge *e = chain[ 0 ];
        if ( e->m_N0->m_UWPnt.y() > e->m_N1->m_UWPnt.y() )
        {
            flip = true;
            std::reverse( chain.begin(), chain.end() );
            for ( int i = 0; i < ( int )chain.size(); i++ )
            {
                chain[i]->SwapEdgeDirection();
            }
        }
    }

    // If chain was flipped, then re-rotate to put wmin edge first
    if ( flip )
    {
        std::rotate( chain.begin(), chain.end() - 1, chain.end() );
    }
}

void BuildEdgeChains( vector< TEdge* > evec, vector < vector < TEdge* > > & echainvec )
{
    // Build chains of edges.

    // Search for first un-used edge, start new chain.
    int nedg = evec.size();
    vector < bool > usededge( nedg, false );
    for ( int iedge = 0; iedge < nedg; iedge++ )
    {
        if ( !usededge[ iedge ] )
        {
            int ichain = echainvec.size();
            echainvec.push_back( vector< TEdge* >() );
            echainvec[ ichain ].push_back( evec[iedge] );
            usededge[ iedge ] = true;

            // Start search for next edge after base edge.
            int jedge = iedge + 1;

            while ( jedge < nedg )
            {
                int jnearest = -1;
                double dmin = 1e10;
                bool flip = false;
                double l1 = 1, l2 = 1;

                // Check remaining edges.
                for ( ; jedge < nedg; jedge++ )
                {
                    TEdge *edge = echainvec[ ichain ].back();
                    l1 = dist( edge->m_N0->m_Pnt, edge->m_N1->m_Pnt );

                    if ( !usededge[ jedge ] )
                    {
                        TEdge *edge2 = evec[ jedge ];

                        double d = dist( edge->m_N1->m_Pnt, edge2->m_N0->m_Pnt );
                        if ( d < dmin )
                        {
                            jnearest = jedge;
                            dmin = d;
                            flip = false;
                            l2 = dist( edge2->m_N0->m_Pnt, edge2->m_N1->m_Pnt );
                        }

                        d = dist( edge->m_N1->m_Pnt, edge2->m_N1->m_Pnt );
                        if ( d < dmin )
                        {
                            jnearest = jedge;
                            dmin = d;
                            flip = true;
                            l2 = dist( edge2->m_N0->m_Pnt, edge2->m_N1->m_Pnt );
                        }
                    }
                }

                if ( jnearest != -1 && ( dmin / ( l1 + l2 ) ) < 1e-3 )
                {
                    TEdge *edge2 = evec[ jnearest ];

                    if ( flip )
                    {
                        edge2->SwapEdgeDirection();
                    }

                    echainvec[ ichain ].push_back( edge2 );
                    usededge[ jnearest ] = true;
                    // Reset search to base edge.
                    jedge = iedge + 1;
                }
            }
        }
    }

    for ( int ichain = 0; ichain < ( int )echainvec.size(); ichain++ )
    {
        NormalizeChain( echainvec[ ichain ] );
    }
}

void FindISectChains( TMesh *tmA, TMesh *tmB, vector < vector < TEdge* > > & echainvec )
{
    // Prep for intersection
    tmA->LoadBndBox();
    tmB->LoadBndBox();

    // Intersect meshes
    tmA->Intersect( tmB );

    // Compile list of all intersection edges in tmA
    vector< TEdge* > evec;
    for ( int itri = 0; itri < tmA->m_TVec.size(); itri++ )
    {
        TTri *tri = tmA->m_TVec[ itri ];
        evec.insert( evec.end(), tri->m_ISectEdgeVec.begin(), tri->m_ISectEdgeVec.end() );
    }

    BuildEdgeChains( evec, echainvec );

    // How to evaluate a UW point into XYZ on a mesh.
    // vec3d uw;
    // vec3d xyz = tmA->CompPnt( uw );
}

void CSGMesh( vector < TMesh * > &tmv, bool intSubsFlag, const vector < string > & sub_vec )
{
    IntersectSplitClassify( tmv, intSubsFlag, sub_vec );

    // Fill vector of cfdtypes so we don't have to pass TMeshVec all the way down.
    vector < int > bTypes( tmv.size() );
    vector < bool > thicksurf( tmv.size() );
    for ( int i = 0 ; i < ( int )tmv.size() ; i++ )
    {
        bTypes[i] = tmv[i]->m_SurfCfdType;
        thicksurf[i] = tmv[i]->m_ThickSurf;
    }

    //==== Mark which triangles to ignore ====//
    for ( int i = 0 ; i < ( int )tmv.size() ; i++ )
    {
        tmv[i]->SetIgnoreTriFlag( bTypes, thicksurf );
    }
}

void MeshUnion( vector < TMesh * > &tmv )
{
    IntersectSplitClassify( tmv, false );

    //==== Mark which triangles to ignore ====//
    for ( int i = 0 ; i < ( int )tmv.size() ; i++ )
    {
        tmv[i]->SetIgnoreInsideAny();
    }
}

void MeshCCEIntersect( vector < TMesh * > &tmv )
{
    double scalefac = IntersectSplit( tmv, false );

    vec3d upish( 0.000001, 0.000001, 1.0 );

    //==== Determine Which Triangle Are Interior/Exterior ====//
    for ( int i = 0 ; i < ( int )tmv.size() ; i++ )
    {
        tmv[i]->DeterIntExt( tmv, upish );
    }

    //===== Reset Scale =====//
    ApplyScale( 1.0 / scalefac, tmv );

    //==== Mark which triangles to ignore ====//
    for ( int i = 0 ; i < ( int )tmv.size() ; i++ )
    {
        tmv[i]->SetIgnoreInsideNotOne();
    }
}

void MeshIntersect( vector < TMesh * > &tmv )
{
    IntersectSplitClassify( tmv, false );

    //==== Mark which triangles to ignore ====//
    for ( int i = 0 ; i < ( int )tmv.size() ; i++ )
    {
        tmv[i]->SetIgnoreInsideNotOne();
    }
}

// Subtract meshes i != 0 from mesh j == 0.
void MeshSubtract( vector < TMesh* > & tmv )
{
    IntersectSplitClassify( tmv, false );

    vector < bool > mask( tmv.size(), false );
    mask[0] = true;

    tmv[0]->SetIgnoreInsideAny();;
    //==== Mark which triangles to ignore ====//
    for ( int i = 1 ; i < ( int )tmv.size() ; i++ )
    {
        tmv[i]->SetKeepMatchMask( mask );
        tmv[i]->FlipNormals();
    }
}

void MeshCutAbovePlane( vector < TMesh* > & tmv, const vector <vec3d> & threepts )
{
    IntersectSplitClassify( tmv, false );

    tmv[0]->SetIgnoreOutsideAll();
    tmv[1]->SetIgnoreAbovePlane( threepts );
}

TMesh* OctantSplitMesh( TMesh* tm )
{
    tm->LoadBndBox();
    BndBox bb = tm->m_TBox.m_Box;
    bb.Update( vec3d() ); // Make sure origin is in BBox
    double len = 2.1 * bb.DiagDist();

    // Slice along X, Y, Z = 0 planes to make sure all triangles lie in a single octant.
    for ( int idir = 0; idir < 3; idir++ )
    {
        vec3d org, norm;
        norm.v[ idir ] = 1;
        TMesh * slice = MakeSlice( org, norm, len );

        slice->LoadBndBox();

        tm->Intersect( slice );

        delete slice;
    }
    tm->Split();

    TMesh *flat_tm = new TMesh;
    flat_tm->CopyFlatten( tm );
    delete tm;

    return flat_tm;
}

TMesh* MakeConvexHull(const vector< TMesh* > & tmesh_vec )
{
    TMesh* tMesh = nullptr;

    int npts = 0;
    for ( int i = 0; i < ( int )tmesh_vec.size() ; i++ )
    {
        npts += tmesh_vec[ i ]->m_NVec.size();
    }
    gte::Vector3 < double > *pts = new gte::Vector3 < double > [ npts ];

    int k = 0;
    for ( int i = 0; i < ( int )tmesh_vec.size() ; i++ )
    {
        for ( int j = 0; j < tmesh_vec[ i ]->m_NVec.size(); j++ )
        {
            pts[ k ][ 0 ] = tmesh_vec[ i ]->m_NVec[ j ]->m_Pnt.x();
            pts[ k ][ 1 ] = tmesh_vec[ i ]->m_NVec[ j ]->m_Pnt.y();
            pts[ k ][ 2 ] = tmesh_vec[ i ]->m_NVec[ j ]->m_Pnt.z();
            k++;
        }
    }

    gte::ConvexHull3 < double > ch;

    ch( npts, pts, 0 );

    if ( ch.GetDimension() == 3 )
    {
        tMesh = new TMesh();

        if ( tMesh )
        {
            std::vector < size_t > hull = ch.GetHull();

            vector < bool > vused( npts, false );
            for ( int i = 0; i < hull.size(); i++ )
            {
                vused[ hull[ i ] ] = true;
            }

            vector < int > vxref( npts, -1 );
            int iused = 0;
            for ( int i = 0 ; i < ( int )npts ; i++ )
            {
                if ( vused[ i ] )
                {
                    TNode *n = new TNode();
                    n->m_Pnt = vec3d( pts[ i ][ 0 ], pts[ i ][ 1 ], pts[ i ][ 2 ] );
                    n->m_ID = i;
                    tMesh->m_NVec.push_back( n );
                    vxref[ i ] = iused;
                    iused++;
                }
            }

            int nFaces = hull.size() / 3;
            tMesh->m_TVec.reserve( nFaces );

            for ( int i = 0; i < nFaces; i++ )
            {
                TTri *t = new TTri( tMesh );

                t->m_N0 = tMesh->m_NVec[ vxref[ hull[ 3 * i + 0 ] ] ];
                t->m_N1 = tMesh->m_NVec[ vxref[ hull[ 3 * i + 1 ] ] ];
                t->m_N2 = tMesh->m_NVec[ vxref[ hull[ 3 * i + 2 ] ] ];

                t->CompNorm();
                tMesh->m_TVec.push_back( t );
            }
        }
    }

    delete[] pts;
    return tMesh;
}

void DeterIntExtTri( TTri* tri, const vector< TMesh* >& meshVec, const vec3d &dir )
{
    vec3d orig = ( tri->m_N0->m_Pnt + tri->m_N1->m_Pnt + tri->m_N2->m_Pnt ) / 3.0;
    tri->m_IgnoreTriFlag = false;
    int prior = -1;

    int nmesh = meshVec.size();
    tri->m_insideSurf.clear();
    tri->m_insideSurf.resize( nmesh, false );

    for ( int m = 0 ; m < ( int )meshVec.size() ; m++ )
    {
        if ( meshVec[m] != tri->GetTMeshPtr() && meshVec[m]->m_ThickSurf )
        {
            vector<double > tParmVec;
            vector <TTri*> triVec;
            meshVec[m]->m_TBox.RayCast( orig, dir, tParmVec, triVec );
            if ( tParmVec.size() % 2 )
            {
                tri->m_insideSurf[m] = true;

                // Priority assignment for wave drag.  Mass prop may need some adjustments.
                if ( meshVec[m]->m_MassPrior > prior ) // Should possibly check that priority is only for vsp::CFD_NORMAL
                {
                    // Assigns GeomID to slice triangles for later use by Wave Drag and Mass Properties.
                    tri->m_GeomID = meshVec[m]->m_OriginGeomID;
                    tri->m_Density = meshVec[m]->m_Density;
                    prior = meshVec[m]->m_MassPrior;
                }
            }
        }
    }
}

bool DeterIntExtTri( TTri* tri, TMesh* mesh, const vec3d &dir )
{
    if ( tri )
    {
        vector <TMesh*> tmv;
        tmv.push_back( mesh );
        DeterIntExtTri( tri, tmv, dir );

        if ( !tri->m_insideSurf.empty() )
        {
            return tri->m_insideSurf[0];
        }
    }
    return false;
}

void MakeThreePts( const vec3d & org, const vec3d & norm, vector <vec3d> &threepts )
{
    threepts.clear();
    threepts.reserve( 3 );

    threepts.emplace_back( 0, 1.0, 1.0 );
    threepts.emplace_back( 0, -1.0, 1.0 );
    threepts.emplace_back( 0, -1.0, -1.0 );

    Matrix4d rot, trans;
    rot.rotatealongX( norm );
    rot.affineInverse();

    trans.translatev( org );
    rot.postMult( trans );

    rot.xformvec( threepts );
}

TMesh* MakeSlice( const vec3d & org, const vec3d & norm, const double & len )
{
    TMesh* tm = MakeSlice( vsp::X_DIR, len );

    Matrix4d rot, trans;
    rot.rotatealongX( norm );
    rot.affineInverse();

    trans.translatev( org );
    rot.postMult( trans );

    tm->Transform( rot );

    return tm;
}

TMesh* MakeSlice( const int &swdir, const double & len )
{
    bool tesselate = true;
    int ntess = 10;

    int dir1, dir2;
    if ( swdir == vsp::X_DIR )
    {
        dir1 = vsp::Y_DIR;
        dir2 = vsp::Z_DIR;
    }
    else if ( swdir == vsp::Y_DIR )
    {
        dir1 = vsp::Z_DIR;
        dir2 = vsp::X_DIR;
    }
    else
    {
        dir1 = vsp::X_DIR;
        dir2 = vsp::Y_DIR;
    }

    double del1 = len;
    double s1   = -0.5 * len;
    double del2 = len;
    double s2   = -0.5 * len;

    vec3d n;
    n[ swdir ] = 1;

    TMesh* tm = new TMesh();

    tm->m_ThickSurf = false;
    tm->m_SurfCfdType = vsp::CFD_STRUCTURE;

    if ( tesselate )
    {

        double ds = 1.0 / (double) ntess;
        int iQuad = 0;
        for ( int i = 0; i < ntess; i++ )
        {
            double d10 = s1 + del1 * ds * ( double )i;
            double d11 = s1 + del1 * ds * ( double )( i + 1 );

            for ( int j = 0; j < ntess; j++ )
            {
                double d20 = s2 + del2 * ds * ( double )j;
                double d21 = s2 + del2 * ds * ( double )( j + 1 );

                vec3d p1, p2, p3, p4;
                p1[swdir] = 0;
                p1[dir1] = d10;
                p1[dir2] = d20;

                p2[swdir] = 0;
                p2[dir1] = d11;
                p2[dir2] = d20;

                p3[swdir] = 0;
                p3[dir1] = d11;
                p3[dir2] = d21;

                p4[swdir] = 0;
                p4[dir1] = d10;
                p4[dir2] = d21;

                tm->AddTri( p1, p2, p3, n, iQuad );
                tm->AddTri( p1, p3, p4, n, iQuad );
                iQuad++;
            }
        }
    }
    else
    {
        vec3d p1, p2, p3, p4;
        p1[swdir] = 0;
        p1[dir1] = s1;
        p1[dir2] = s2;

        p2[swdir] = 0;
        p2[dir1] = s1 + del1;
        p2[dir2] = s2;

        p3[swdir] = 0;
        p3[dir1] = s1 + del1;
        p3[dir2] = s2 + del2;

        p4[swdir] = 0;
        p4[dir1] = s1;
        p4[dir2] = s2 + del2;

        tm->AddTri( p1, p2, p3, n, 1 );
        tm->AddTri( p1, p3, p4, n, 1 );
    }

    return tm;
}

double MakeSlices( vector<TMesh*> &tmv, const BndBox & bbox, int numSlices, int swdir, vector < double > &slicevec, bool mpslice, bool tesselate, bool autoBounds, double start, double end, int slctype )
{
    int s, i, j;
    double offset = 0.0001; // Amount to extend slicing bounds.

    int dir1, dir2;
    if ( swdir == vsp::X_DIR )
    {
        dir1 = vsp::Y_DIR;
        dir2 = vsp::Z_DIR;
    }
    else if ( swdir == vsp::Y_DIR )
    {
        dir1 = vsp::Z_DIR;
        dir2 = vsp::X_DIR;
    }
    else
    {
        dir1 = vsp::X_DIR;
        dir2 = vsp::Y_DIR;
    }

    double swMin;
    double swMax;
    if ( autoBounds )
    {
        swMin = bbox.GetMin( swdir ) - offset;
        swMax = bbox.GetMax( swdir ) + offset;
    }
    else
    {
        swMin = start - offset;
        swMax = end + offset;
    }

    // MassProp slice always uses autobounds.  Does not need offset because slices are later shifted by width/2.
    if ( mpslice )
    {
        swMin = bbox.GetMin( swdir );
        swMax = bbox.GetMax( swdir );
    }

    double sliceW;
    if ( mpslice )
    {
        sliceW = ( swMax - swMin ) / ( double )( numSlices );
    }
    else
    {
        if ( numSlices > 1 )
        {
            sliceW = ( swMax - swMin ) / ( double )( numSlices - 1 );
        }
        else
        {
            sliceW = 0.0;
        }
    }
    slicevec.resize( numSlices );

    double del1 = 1.02 * ( bbox.GetMax( dir1 ) - bbox.GetMin( dir1 ) );
    double s1   = bbox.GetMin( dir1 ) - 0.01 * del1;
    double del2 = 1.02 * ( bbox.GetMax( dir2 ) - bbox.GetMin( dir2 ) );
    double s2   = bbox.GetMin( dir2 ) - 0.01 * del2;

    vec3d n;
    n[ swdir ] = 1;

    for ( s = 0 ; s < numSlices ; s++ )
    {
        TMesh* tm = new TMesh();

        tm->m_ThickSurf = false;
        tm->m_SurfCfdType = slctype;

        tmv.push_back( tm );

        double sw;
        if ( mpslice )
        {
            sw = swMin + ( double )s * sliceW + 0.5 * sliceW;
        }
        else
        {
            sw = swMin + ( double )s * sliceW;
        }
        slicevec[s] = sw;

        if ( tesselate )
        {
            int ntess = numSlices;
            double ds = 1.0 / (double) ntess;
            int iQuad = 0;
            for ( i = 0 ; i < ntess ; i++ )
            {
                double d10 = s1 + del1 * ds * ( double )i;
                double d11 = s1 + del1 * ds * ( double )( i + 1 );

                for ( j = 0 ; j < ntess ; j++ )
                {
                    double d20 = s2 + del2 * ds * ( double )j;
                    double d21 = s2 + del2 * ds * ( double )( j + 1 );

                    vec3d p1, p2, p3, p4;
                    p1[swdir] = sw;
                    p1[dir1] = d10;
                    p1[dir2] = d20;

                    p2[swdir] = sw;
                    p2[dir1] = d11;
                    p2[dir2] = d20;

                    p3[swdir] = sw;
                    p3[dir1] = d11;
                    p3[dir2] = d21;

                    p4[swdir] = sw;
                    p4[dir1] = d10;
                    p4[dir2] = d21;

                    tm->AddTri( p1, p2, p3, n, iQuad );
                    tm->AddTri( p1, p3, p4, n, iQuad );
                    iQuad++;
                }
            }
        }
        else
        {
            vec3d p1, p2, p3, p4;
            p1[swdir] = sw;
            p1[dir1] = s1;
            p1[dir2] = s2;

            p2[swdir] = sw;
            p2[dir1] = s1 + del1;
            p2[dir2] = s2;

            p3[swdir] = sw;
            p3[dir1] = s1 + del1;
            p3[dir2] = s2 + del2;

            p4[swdir] = sw;
            p4[dir1] = s1;
            p4[dir2] = s2 + del2;

            tm->AddTri( p1, p2, p3, n, 1 );
            tm->AddTri( p1, p3, p4, n, 1 );
        }
    }
    return sliceW;
}

double CalcMeshDeviation( TMesh *tm, const vec3d &cen, const vec3d &norm )
{
    Matrix4d mat;
    mat.rotatealongX( norm );
    mat.translatev( -cen );

    int n = tm->m_NVec.size();
    double maxx = -1.0;
    for ( int i = 0; i < n; i++ )
    {
        vec3d pt = mat.xform( tm->m_NVec[i]->m_Pnt );

        double ax = std::abs( pt.x() );
        if ( ax > maxx )
        {
            maxx = ax;
        }
    }

    return maxx;
}

void FitPlaneToMesh( TMesh *tm, vec3d &cen, vec3d &norm )
{
    int n = tm->m_NVec.size();
    vector < vec3d > pts;
    pts.reserve( n );
    for ( int i = 0; i < n; i++ )
    {
        pts.push_back( tm->m_NVec[i]->m_Pnt );
    }

    FitPlane( pts, cen, norm );
}

// Prioritize TMeshes by:
// 1 Wing surface type.  Wings dominate all other types.
// 2 Mass priority.  User can manually change priority.
// 3 GeomID alphabetically.  Arbitrary, but repeatable within one file.
// 4 Surface number.  Break ties repeatably from symmetry.
static bool CutterTMeshCompare( TMesh* a, TMesh* b )
{
    if ( a->m_SurfType == vsp::WING_SURF && b->m_SurfType != vsp::WING_SURF )
        return true;

    if ( b->m_SurfType == vsp::WING_SURF && a->m_SurfType != vsp::WING_SURF )
        return false;

    if ( a->m_MassPrior > b->m_MassPrior )
        return true;

    if ( b->m_MassPrior > a->m_MassPrior )
        return false;

    if ( a->m_OriginGeomID < b->m_OriginGeomID )
        return true;

    if ( b->m_OriginGeomID < b->m_OriginGeomID )
        return false;

    return ( a->m_SurfNum < b->m_SurfNum );
}

TMesh* MakeCutter( TMesh *tm, const vec3d &norm )
{
    // Convert input TMesh into PGMesh to make isolating the boundary easier.
    PGMulti pgmulti;
    PGMesh *pgm = pgmulti.GetActiveMesh();

    vector < PGNode* > nod( tm->m_NVec.size() );
    for ( int i = 0; i < tm->m_NVec.size(); i++ )
    {
        tm->m_NVec[i]->m_ID = i;
        PGPoint *pnt = pgmulti.AddPoint( tm->m_NVec[i]->m_Pnt );
        nod[i] = pgm->AddNode( pnt );
    }

    for ( int i = 0; i < tm->m_TVec.size(); i++ )
    {
        TTri *t = tm->m_TVec[i];
        pgm->AddFace( nod[t->m_N0->m_ID], nod[t->m_N1->m_ID], nod[t->m_N2->m_ID],
                     t->m_N0->m_UWPnt.as_vec2d_xy(), t->m_N1->m_UWPnt.as_vec2d_xy(), t->m_N2->m_UWPnt.as_vec2d_xy(),
                     t->m_Norm, t->m_iQuad, 0, t->m_jref, t->m_kref );
    }

    // Merge Nodes and Edges to make topologically correct.
    pgm->MergeCoincidentNodes();

    // Build vector of boundary edges.
    vector < PGEdge* > boundary_edges;
    list< PGEdge* >::iterator e;
    for ( e = pgm->m_EdgeList.begin(); e != pgm->m_EdgeList.end(); ++e )
    {
        if ( ( *e )->m_FaceVec.size() < 2 )
        {
            boundary_edges.push_back( (*e) );
        }
    }

    // Construct new TMesh cutter volume from surfaces.
    TMesh *tm_cutter = new TMesh();

    int numnode = pgm->m_NodeList.size();
    tm_cutter->m_NVec.resize( numnode * 2 );

    int inode = 0;
    list< PGNode* >::iterator n;
    for ( n = pgm->m_NodeList.begin() ; n != pgm->m_NodeList.end(); ++n )
    {
        ( *n )->m_Pt->m_ID = inode;

        TNode *n1 = new TNode();
        n1->m_Pnt = ( *n )->m_Pt->m_Pnt - 10 * norm;
        n1->m_ID = inode;
        tm_cutter->m_NVec[ inode ] = n1;

        TNode *n2 = new TNode();
        n2->m_Pnt = ( *n )->m_Pt->m_Pnt + 10 * norm;
        n2->m_ID = inode + numnode;
        tm_cutter->m_NVec[ inode + numnode ] = n2;

        inode++;
    }

    list< PGFace* >::iterator f;
    for ( f = pgm->m_FaceList.begin() ; f != pgm->m_FaceList.end(); ++f )
    {
        vector < PGNode* > nodVec;
        ( *f )->GetNodesAsTris( nodVec );

        int npt = nodVec.size();
        int ntri = npt / 3;

        for ( int i = 0; i < ntri; i++ )
        {
            TTri *t = new TTri( tm_cutter );

            t->m_N0 = tm_cutter->m_NVec[ nodVec[ i * 3 + 0 ]->m_Pt->m_ID ];
            t->m_N1 = tm_cutter->m_NVec[ nodVec[ i * 3 + 1 ]->m_Pt->m_ID ];
            t->m_N2 = tm_cutter->m_NVec[ nodVec[ i * 3 + 2 ]->m_Pt->m_ID ];

            t->CompNorm();
            tm_cutter->m_TVec.push_back( t );

            t = new TTri( tm_cutter );

            t->m_N0 = tm_cutter->m_NVec[ nodVec[ i * 3 + 0 ]->m_Pt->m_ID + numnode ];
            t->m_N2 = tm_cutter->m_NVec[ nodVec[ i * 3 + 1 ]->m_Pt->m_ID + numnode ];
            t->m_N1 = tm_cutter->m_NVec[ nodVec[ i * 3 + 2 ]->m_Pt->m_ID + numnode ];

            t->CompNorm();
            tm_cutter->m_TVec.push_back( t );
        }
    }

    // Boundary faces are not guaranteed to be consistently oriented.  However,
    // it doesn't matter for use as a cutting volume.
    for ( int i = 0; i < boundary_edges.size(); i++ )
    {
        PGEdge* e = boundary_edges[i];

        TTri *t = new TTri( tm_cutter );

        t->m_N0 = tm_cutter->m_NVec[ e->m_N0->m_Pt->m_ID ];
        t->m_N1 = tm_cutter->m_NVec[ e->m_N0->m_Pt->m_ID + numnode ];
        t->m_N2 = tm_cutter->m_NVec[ e->m_N1->m_Pt->m_ID ];
        t->CompNorm();
        tm_cutter->m_TVec.push_back( t );

        t = new TTri( tm_cutter );

        t->m_N0 = tm_cutter->m_NVec[ e->m_N0->m_Pt->m_ID + numnode ];
        t->m_N1 = tm_cutter->m_NVec[ e->m_N1->m_Pt->m_ID + numnode ];
        t->m_N2 = tm_cutter->m_NVec[ e->m_N1->m_Pt->m_ID ];
        t->CompNorm();
        tm_cutter->m_TVec.push_back( t );
    }

    return tm_cutter;
}

void CutMesh( TMesh *target_tm, TMesh *cutter_tm )
{
    target_tm->LoadBndBox();

    target_tm->Intersect( cutter_tm );
    cutter_tm->RemoveIsectEdges();
    target_tm->Split();

    vector < TMesh* > cutter_tmvec;
    cutter_tmvec.push_back( cutter_tm );
    target_tm->DeterIntExt( cutter_tmvec );

    vector < int > bTypes;
    bTypes.push_back( vsp::CFD_NORMAL );
    vector < bool > thicksurf;
    thicksurf.push_back( true );

    target_tm->SetIgnoreTriFlag( bTypes, thicksurf );
}

void TrimTMeshSequence( vector < TMesh* > tmvec )
{
    int nMesh = tmvec.size();

    int npt = 0;
    for ( int i = 0 ; i < nMesh; i++ )
    {
        npt += tmvec[ i ]->m_NVec.size();
    }

    vector < vec3d > pts;
    pts.reserve( npt );
    for ( int i = 0 ; i < nMesh; i++ )
    {
        for ( int j = 0; j < tmvec[ i ]->m_NVec.size(); j++ )
        {
            pts.push_back( tmvec[ i ]->m_NVec[ j ]->m_Pnt );
        }
    }

    // Best fit plane to all points across all meshes.
    vec3d cen, norm;
    FitPlane( pts, cen, norm );

    for ( int i = nMesh - 2 ; i >= 0; i-- ) // Iterate over cutters backwards.
    {
        TMesh *cutter_tm = MakeCutter( tmvec[i], norm );
        cutter_tm->LoadBndBox();
        for ( int j = i + 1 ; j < nMesh; j++ ) // Apply to all later meshes.
        {
            CutMesh( tmvec[j], cutter_tm );
            tmvec[j]->FlattenInPlace();
        }
        delete cutter_tm;
    }
}

void TrimCoplanarPatches( vector < TMesh* > &tmv )
{
    double tol = 1e-6;

    int nMesh = tmv.size();
    vector < int > root_coplanar( nMesh, -1 );

    for ( int i = 0 ; i < nMesh - 1; i++ )
    {
        TMesh *tmi = tmv[ i ];

        if ( tmi->m_FlatPatch )
        {
            vec3d cen, norm;
            FitPlaneToMesh( tmi, cen, norm );

            for ( int j = i + 1; j < nMesh; j++ )
            {
                TMesh *tmj = tmv[ j ];

                if ( tmj->m_FlatPatch &&
                     ( root_coplanar[j] == -1 ) &&
                     !( tmi->m_OriginGeomID == tmj->m_OriginGeomID &&
                        tmi->m_SurfNum == tmj->m_SurfNum &&
                        tmi->m_PlateNum == tmj->m_PlateNum )
                   )
                {
                    if ( CalcMeshDeviation( tmj, cen, norm ) < tol )
                    {
                        root_coplanar[i] = i;
                        root_coplanar[j] = i;
                    }
                }
            }
        }
    }

    for ( int i = 0 ; i < nMesh - 1; i++ )
    {
        vector < TMesh* > tmgroup;
        if ( root_coplanar[i] == i )
        {
            tmgroup.push_back( tmv[ i ] );
            for ( int j = i + 1; j < nMesh; j++ )
            {
                if ( root_coplanar[j] == i )
                {
                    tmgroup.push_back( tmv[ j ] );
                }
            }
        }

        // Sort in order of priority.  First should cut all later etc.
        std::sort( tmgroup.begin(), tmgroup.end(), CutterTMeshCompare );

        TrimTMeshSequence( tmgroup );

        for ( int j = 0; j < tmgroup.size(); j++ )
        {
            tmgroup[j]->m_InGroup.push_back( i );
        }
    }
}

// When Degen plate and camber surfaces are made into TMeshs, they can be split into patches if some areas
// are planar.  This stitches those back together.
void MergeCoplanarSplitPatches( vector < TMesh* > &tmv )
{
    for ( int i = 0 ; i < ( int )tmv.size() - 1; i++ )
    {
        TMesh *tmi = tmv[ i ];
        if ( tmi->m_DeleteMeFlag == false )
        {
            for ( int j = i + 1 ; j < ( int )tmv.size(); j++ )
            {
                TMesh *tmj = tmv[ j ];
                if ( tmi->m_OriginGeomID == tmj->m_OriginGeomID &&
                     tmi->m_SurfNum == tmj->m_SurfNum &&
                     tmi->m_PlateNum == tmj->m_PlateNum &&
                     tmj->m_DeleteMeFlag == false )
                {
                    tmi->MergeTMeshes( tmj );
                    tmj->m_DeleteMeFlag = true;
                }
            }
        }
    }

    DeleteMarkedMeshes( tmv );
}

// TMeshes with coplanar patches (which have been trimmed) are stitched together here without
// running Intersect on them.  This hopefully prevents sliver hell.  However, it runs the risk
// of missing intersections if non-coplanar patches of these surfaces also intersect in a
// non-coplanar way.
void MergeCoplanarTrimGroups( vector < TMesh* > &tmv )
{
    vector < int > allGroups;
    for ( int i = 0 ; i < ( int )tmv.size(); i++ )
    {
        TMesh *tmi = tmv[ i ];
        allGroups.insert( allGroups.end(), tmi->m_InGroup.begin(), tmi->m_InGroup.end() );
    }
    std::sort( allGroups.begin(), allGroups.end() );
    allGroups.erase( std::unique( allGroups.begin(), allGroups.end() ), allGroups.end() );

    for ( int ig = 0 ; ig < ( int )allGroups.size(); ig++ )
    {
        int grp = allGroups[ig];

        for ( int i = 0 ; i < ( int )tmv.size() - 1; i++ )
        {
            TMesh *tmi = tmv[ i ];
            if ( tmi->m_DeleteMeFlag == false &&
                 vector_contains_val( tmi->m_InGroup, grp ) )
            {
                for ( int j = i + 1; j < ( int ) tmv.size(); j++ )
                {
                    TMesh *tmj = tmv[ j ];

                    if ( tmj->m_DeleteMeFlag == false &&
                         vector_contains_val( tmj->m_InGroup, grp ) )
                    {
                        tmi->MergeTMeshes( tmj );
                        tmj->m_DeleteMeFlag = true;
                    }
                }
            }
        }
    }

    DeleteMarkedMeshes( tmv );
}

// tmi->m_PlateNum == tmj->m_PlateNum &&
void MergeDegenCruciformTMeshes( vector < TMesh* > &tmv )
{
    for ( int i = 0 ; i < ( int )tmv.size() - 1; i++ )
    {
        TMesh *tmi = tmv[ i ];
        if ( tmi->m_DeleteMeFlag == false )
        {
            for ( int j = i + 1 ; j < ( int )tmv.size(); j++ )
            {
                TMesh *tmj = tmv[ j ];
                if ( tmi->m_OriginGeomID == tmj->m_OriginGeomID &&
                     tmi->m_ThickSurf == false && // Degen only
                     tmj->m_ThickSurf == false &&
                     tmi->m_SurfType == vsp::NORMAL_SURF && // bodies, not wings.
                     tmj->m_SurfType == vsp::NORMAL_SURF &&
                     tmi->m_CopyIndex == tmj->m_CopyIndex && // Same symmetry copy
                     tmi->m_SurfNum == tmj->m_SurfNum && // Same surface number
                     tmj->m_DeleteMeFlag == false )
                {
                    tmi->MergeTMeshes( tmj );
                    tmj->m_DeleteMeFlag = true;
                }
            }
        }
    }

    DeleteMarkedMeshes( tmv );
}

// Look for TMesh's that correspond to symmetrical copies of a Geom, where the points along the
// center plane should be at Y==0, but are slightly off.
void ForceSymmSmallYZero( vector < TMesh* > &tmv )
{
    for ( int i = 0 ; i < ( int )tmv.size() - 1; i++ )
    {
        TMesh *tmi = tmv[ i ];
        Matrix4d mat;
        BndBox bbi;
        tmi->UpdateBBox( bbi, mat );
        double ylimi = bbi.GetMax( 1 );
        if ( bbi.GetCenter().y() > 0 )
        {
            ylimi = bbi.GetMin( 1 );
        }

        for ( int j = i + 1 ; j < ( int )tmv.size(); j++ )
        {
            TMesh *tmj = tmv[ j ];
            if ( tmi->m_OriginGeomID == tmj->m_OriginGeomID &&
                 tmi->m_PlateNum == tmj->m_PlateNum )
            {
                BndBox bbj;
                tmj->UpdateBBox( bbj, mat );
                double ylimj = bbj.GetMax( 1 );
                if ( bbj.GetCenter().y() > 0 )
                {
                    ylimj = bbj.GetMin( 1 );
                }

                if ( std::abs( ylimj - ylimi ) < 1e-6 )
                {
                    tmi->ForceSmallYZero();
                    tmj->ForceSmallYZero();
                }
            }
        }
    }
}

TMesh* AddHalfBox( const vector < TMesh* > &tmv, const string &id )
{
    BndBox box;
    for ( int m = 0 ; m < ( int )tmv.size() ; m++ )
    {
        for ( int t = 0 ; t < ( int )tmv[m]->m_TVec.size() ; t++ )
        {
            box.Update( tmv[m]->m_TVec[t]->m_N0->m_Pnt );
            box.Update( tmv[m]->m_TVec[t]->m_N1->m_Pnt );
            box.Update( tmv[m]->m_TVec[t]->m_N2->m_Pnt );
        }
    }

    vec3d bscale = vec3d( 2, 2, 2 );
    box.Scale( bscale );

    TMesh* tm = new TMesh();
    tm->m_SurfCfdType = vsp::CFD_NEGATIVE;
    tm->m_OriginGeomID = id;

    double xmin = box.GetMin( 0 );
    double xmax = box.GetMax( 0 );

    double zmin = box.GetMin( 2 );
    double zmax = box.GetMax( 2 );

    double ymin = box.GetMin( 1 );

    vec3d A = vec3d( xmin, 0, zmin );
    vec3d B = vec3d( xmax, 0, zmin );
    vec3d C = vec3d( xmin, 0, zmax );
    vec3d D = vec3d( xmax, 0, zmax );
    vec3d E = vec3d( xmin, ymin, zmin );
    vec3d F = vec3d( xmax, ymin, zmin );
    vec3d G = vec3d( xmin, ymin, zmax );
    vec3d H = vec3d( xmax, ymin, zmax );

    tm->AddTri( G, E, H, vec3d( 0, -1, 0 ), 1 );
    tm->AddTri( H, E, F, vec3d( 0, -1, 0 ), 1 );

    tm->AddTri( B, A, D, vec3d( 0, 1, 0 ), 2 );
    tm->AddTri( D, A, C, vec3d( 0, 1, 0 ), 2 );

    tm->AddTri( C, A, E, vec3d( -1, 0, 0 ), 3 );
    tm->AddTri( G, C, E, vec3d( -1, 0, 0 ), 3 );

    tm->AddTri( F, B, D, vec3d( 1, 0, 0 ), 4 );
    tm->AddTri( F, D, H, vec3d( 1, 0, 0 ), 4 );

    tm->AddTri( D, C, G, vec3d( 0, 0, 1 ), 5 );
    tm->AddTri( H, D, G, vec3d( 0, 0, 1 ), 5 );

    tm->AddTri( E, A, B, vec3d( 0, 0, -1 ), 6 );
    tm->AddTri( E, B, F, vec3d( 0, 0, -1 ), 6 );

    return tm;
}

void IgnoreYLessThan( vector < TMesh* > &tmv, const double &ytol )
{
    for ( int i = 0; i < tmv.size(); i++ )
    {
        tmv[i]->IgnoreYLessThan( ytol );
    }
}

TMesh* GetMeshByID( const vector < TMesh* > &tmv, const string &id )
{
    for ( int i = 0; i < tmv.size(); i++ )
    {
        if ( tmv[i]->m_OriginGeomID == id )
        {
            return tmv[i];
        }
    }
    return nullptr;
}

