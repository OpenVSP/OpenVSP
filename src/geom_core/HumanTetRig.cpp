//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//
// HumanTetRig.cpp:
// The tet mesh that fills a Human, from HumanTetData.h (src/geom_core/humanshape/human_tets.py): one
// connectivity for every body, its vertices placed for the body from the skin, the joints and the
// anthropometric vector.  Posed by volumetric ARAP or by FAST (as libigl's arap and arap_dof).
//
//////////////////////////////////////////////////////////////////////

#include "HumanTetRig.h"
#include "HumanTetData.h"

#include <algorithm>
#include <cmath>

#include <Eigen/Dense>
#include <Eigen/Sparse>
#include <Eigen/Geometry>

#include <igl/cotmatrix_entries.h>
#include <igl/dqs.h>
#include <igl/group_sum_matrix.h>
#include <igl/parallel_for.h>
#include <igl/polar_svd.h>
#include <igl/polar_svd3x3.h>

// Iterations between ARAP's convergence checks.
#define ARAP_BLOCK 10

// What every body shares: the tets, and the handle weights and FAST's additional weights per tet
// vertex.
struct HumanTetShared
{
    HumanTetShared();

    Eigen::MatrixXi m_TT;
    Eigen::MatrixXd m_W;
    Eigen::MatrixXd m_We;
};

struct HumanTetRigData
{
    Eigen::MatrixXd m_TV;         // Tet vertices, rest.
    const HumanTetShared *m_Shared;

    Eigen::MatrixXd m_OffOffset;  // Each skin vertex no tet uses, less its boundary point, at rest.

    // ARAP, per tet (see PrecomputeARAP): P (row-major in 12 columns), each tet vertex's free index
    // (-1 if held), the free indices' tet vertices, the coupling Auk to the held vertices, and Auu's
    // factor P^T L L^T P, L lower and by columns.
    bool m_ARAPReady;
    bool m_ARAPFailed;      // The factorization failed; the skin is left unposed.
    Eigen::MatrixXd m_ArapP;
    vector < int > m_ArapFree;
    vector < int > m_ArapFreeTV;
    Eigen::SparseMatrix < double > m_ArapAuk;
    Eigen::PermutationMatrix < Eigen::Dynamic, Eigen::Dynamic, int > m_ArapPerm;
    Eigen::PermutationMatrix < Eigen::Dynamic, Eigen::Dynamic, int > m_ArapPermInv;
    vector < int > m_LStart;
    vector < int > m_LRow;
    vector < double > m_LVal;
    // Each tet's last rotation, the warm start of its next fit.
    vector < Eigen::Quaterniond, Eigen::aligned_allocator < Eigen::Quaterniond > > m_ArapQ;
    // The ( tet, corner ) pairs at each free vertex, 4 t + k, by free index.
    vector < int > m_ArapIncStart;
    vector < int > m_ArapInc;

    // The tet vertices of the last pose.
    Eigen::MatrixXd m_PosedTV;

    // FAST, condensed: the skinning matrix M1 = [ W .* x, W .* y, W .* z, W ] (tet vertices by 4 m, m
    // handles and additional weights), the rotation groups' covariance gather CSM (3 k by 4 m, k
    // groups), and the solve for the additional weights' columns, Lsep = Solve * R + Fix * Lhandles.
    bool m_FASTReady;
    bool m_FASTFailed;
    Eigen::SparseMatrix < double, Eigen::RowMajor > m_M1Rows;
    Eigen::MatrixXd m_CSM;
    Eigen::MatrixXd m_Solve;
    Eigen::MatrixXd m_Fix;
};

typedef vector < Eigen::Matrix3d, Eigen::aligned_allocator < Eigen::Matrix3d > > Matrix3dVec;

static_assert( HUMAN_TET_NUM_PARTIALS == HUMAN_NUM_PARTIALS, "HumanTetData.h and HumanTetPartial disagree" );

// Each handle's transform: its bone's, after its fraction of its partial rotation.
static void HandleTransforms( const vector < Matrix4d > &bone_mats, const vector < HumanPartialRot > &partials,
                              vector < Eigen::Matrix4d > &T )
{
    T.resize( HUMAN_TET_NUM_HANDLES );
    for ( int k = 0; k < HUMAN_TET_NUM_HANDLES; k++ )
    {
        Matrix4d mk = bone_mats[ human_tet_handle_bone[k] ];
        int ip = human_tet_handle_partial[k];
        if ( ip >= 0 )
        {
            const HumanPartialRot &pr = partials[ ip ];
            Matrix4d P;
            P.translatev( pr.m_Pivot );
            P.rotate( human_tet_handle_frac[k] * pr.m_Angle, pr.m_Axis );
            P.translatev( -pr.m_Pivot );
            mk.matMult( P );
        }
        const double *m = mk.const_data();
        for ( int c = 0; c < 4; c++ )
        {
            for ( int r = 0; r < 4; r++ )
            {
                T[k]( r, c ) = m[ c * 4 + r ];
            }
        }
    }
}

static void CSRToDense( const int *start, const int *col, const double *val, int nrow, int ncol, Eigen::MatrixXd &M )
{
    M = Eigen::MatrixXd::Zero( nrow, ncol );
    for ( int i = 0; i < nrow; i++ )
    {
        for ( int k = start[i]; k < start[ i + 1 ]; k++ )
        {
            M( i, col[k] ) = val[k];
        }
    }
}

HumanTetShared::HumanTetShared()
{
    m_TT.resize( HUMAN_TET_NTT, 4 );
    for ( int t = 0; t < HUMAN_TET_NTT; t++ )
    {
        for ( int j = 0; j < 4; j++ )
        {
            m_TT( t, j ) = human_tet_tets[t][j];
        }
    }

    CSRToDense( human_tet_w_start, human_tet_w_col, human_tet_w_val, HUMAN_TET_NTV, HUMAN_TET_NUM_HANDLES, m_W );
    CSRToDense( human_tet_we_start, human_tet_we_col, human_tet_we_val, HUMAN_TET_NTV, HUMAN_TET_NUM_EXTRA, m_We );
}

// The shared arrays, made once on first use.
static const HumanTetShared &GetTetShared()
{
    static const HumanTetShared shared;
    return shared;
}

// Free one method's precomputation when the other is used, so a Human keeps only one.
static void ReleaseARAP( HumanTetRigData &d )
{
    d.m_ARAPReady = false;
    d.m_ARAPFailed = false;
    d.m_ArapP = Eigen::MatrixXd();
    vector < int >().swap( d.m_ArapFree );
    vector < int >().swap( d.m_ArapFreeTV );
    d.m_ArapAuk = Eigen::SparseMatrix < double >();
    d.m_ArapPerm = Eigen::PermutationMatrix < Eigen::Dynamic, Eigen::Dynamic, int >();
    d.m_ArapPermInv = Eigen::PermutationMatrix < Eigen::Dynamic, Eigen::Dynamic, int >();
    vector < int >().swap( d.m_LStart );
    vector < int >().swap( d.m_LRow );
    vector < double >().swap( d.m_LVal );
    vector < Eigen::Quaterniond, Eigen::aligned_allocator < Eigen::Quaterniond > >().swap( d.m_ArapQ );
    vector < int >().swap( d.m_ArapIncStart );
    vector < int >().swap( d.m_ArapInc );
}

static void ReleaseFAST( HumanTetRigData &d )
{
    d.m_FASTReady = false;
    d.m_FASTFailed = false;
    d.m_M1Rows = Eigen::SparseMatrix < double, Eigen::RowMajor >();
    d.m_CSM = Eigen::MatrixXd();
    d.m_Solve = Eigen::MatrixXd();
    d.m_Fix = Eigen::MatrixXd();
}

HumanTetRig::HumanTetRig()
{
}

HumanTetRig::~HumanTetRig()
{
}

bool HumanTetRig::IsBuilt() const
{
    return m_Data != nullptr;
}

void HumanTetRig::Build( int gender, const vector < double > &vars, const vector < vec3d > &skin, const vector < vec3d > &joints )
{

    m_Data.reset( new HumanTetRigData() );
    HumanTetRigData &d = *m_Data;
    d.m_Shared = &GetTetShared();
    d.m_ARAPReady = false;
    d.m_ARAPFailed = false;
    d.m_FASTReady = false;
    d.m_FASTFailed = false;

    // Tet vertices: skin vertices, fractions of bone segments, and lattice points Pbar + Q X.
    d.m_TV.resize( HUMAN_TET_NTV, 3 );
    for ( int i = 0; i < HUMAN_TET_NSKIN; i++ )
    {
        int v = human_tet_skin_tv[i];
        if ( v >= 0 )
        {
            d.m_TV.row( v ) << skin[i].x(), skin[i].y(), skin[i].z();
        }
    }
    for ( int i = 0; i < HUMAN_TET_NBONE; i++ )
    {
        vec3d p = joints[ human_tet_bone_joints[i][0] ] * ( 1.0 - human_tet_bone_t[i] ) +
                  joints[ human_tet_bone_joints[i][1] ] * human_tet_bone_t[i];
        d.m_TV.row( human_tet_bone_tv[i] ) << p.x(), p.y(), p.z();
    }
    const double ( *pbar )[3] = human_tet_male_grid_pbar;
    const double ( *q )[18] = human_tet_male_grid_q;
    if ( gender != 0 )
    {
        pbar = human_tet_female_grid_pbar;
        q = human_tet_female_grid_q;
    }
    for ( int i = 0; i < HUMAN_TET_NGRID; i++ )
    {
        for ( int c = 0; c < 3; c++ )
        {
            double x = pbar[i][c];
            for ( int k = 0; k < 6; k++ )
            {
                x += q[i][ 6 * c + k ] * vars[k];
            }
            d.m_TV( human_tet_grid_tv[i], c ) = x;
        }
    }

    // A skin vertex no tet uses keeps its rest offset from the mean of its points on two boundary
    // triangles of the tet mesh.
    d.m_OffOffset.resize( HUMAN_TET_NOFF, 3 );
    for ( int i = 0; i < HUMAN_TET_NOFF; i++ )
    {
        Eigen::RowVector3d p = Eigen::RowVector3d::Zero();
        for ( int j = 0; j < 6; j++ )
        {
            p += 0.5 * human_tet_off_bary[i][ j % 3 ] * d.m_TV.row( human_tet_off_tri[i][j] );
        }
        const vec3d &s = skin[ human_tet_off_vert[i] ];
        d.m_OffOffset.row( i ) = Eigen::RowVector3d( s.x(), s.y(), s.z() ) - p;
    }

}

void HumanTetRig::SkinFromTets( const vector < vec3d > &tv, vector < vec3d > &skin ) const
{
    const HumanTetRigData &d = *m_Data;
    skin.resize( HUMAN_TET_NSKIN );
    for ( int i = 0; i < HUMAN_TET_NSKIN; i++ )
    {
        if ( human_tet_skin_tv[i] >= 0 )
        {
            skin[i] = tv[ human_tet_skin_tv[i] ];
        }
    }
    for ( int i = 0; i < HUMAN_TET_NOFF; i++ )
    {
        vec3d p;
        for ( int j = 0; j < 6; j++ )
        {
            p = p + tv[ human_tet_off_tri[i][j] ] * ( 0.5 * human_tet_off_bary[i][ j % 3 ] );
        }
        skin[ human_tet_off_vert[i] ] = p + vec3d( d.m_OffOffset( i, 0 ), d.m_OffOffset( i, 1 ), d.m_OffOffset( i, 2 ) );
    }
}

// A tet's edges in the order of igl::cotmatrix_entries.
static const int TET_EDGES[6][2] = { { 1, 2 }, { 2, 0 }, { 0, 1 }, { 3, 0 }, { 3, 1 }, { 3, 2 } };

// Volumetric ARAP (libigl's arap with one rotation per tet), built per tet.  With the cotangent
// weights C, tet t's covariance is S = sum_k P( k, : )^T U( t_k, : ) for the constant P (4 by 3),
// P( k, : ) = sum over the edges ( a, b ) at k of +-C ( V_a - V_b ), and its rotation R adds
// P( k, : ) R to vertex t_k's right-hand side.  The free vertices solve Auu z = -Auk Y + those
// terms, Auu and Auk being -cotmatrix split into the free and held vertices.
void HumanTetRig::PrecomputeARAP()
{
    HumanTetRigData &d = *m_Data;
    int n = d.m_TV.rows();
    int nt = d.m_Shared->m_TT.rows();

    Eigen::MatrixXd C;
    igl::cotmatrix_entries( d.m_TV, d.m_Shared->m_TT, C );

    d.m_ArapP = Eigen::MatrixXd::Zero( nt, 12 );
    for ( int t = 0; t < nt; t++ )
    {
        for ( int e = 0; e < 6; e++ )
        {
            int ks = TET_EDGES[e][0];
            int kd = TET_EDGES[e][1];
            for ( int c = 0; c < 3; c++ )
            {
                double v = C( t, e ) * ( d.m_TV( d.m_Shared->m_TT( t, ks ), c ) - d.m_TV( d.m_Shared->m_TT( t, kd ), c ) );
                d.m_ArapP( t, 3 * ks + c ) += v;
                d.m_ArapP( t, 3 * kd + c ) -= v;
            }
        }
    }

    // Free and held vertices; the held ones in hold order.
    d.m_ArapFree.assign( n, 0 );
    vector < int > held( n, -1 );
    for ( int i = 0; i < HUMAN_TET_NHOLD; i++ )
    {
        d.m_ArapFree[ human_tet_hold_tv[i] ] = -1;
        held[ human_tet_hold_tv[i] ] = i;
    }
    d.m_ArapFreeTV.clear();
    for ( int v = 0; v < n; v++ )
    {
        if ( d.m_ArapFree[v] == 0 )
        {
            d.m_ArapFree[v] = d.m_ArapFreeTV.size();
            d.m_ArapFreeTV.push_back( v );
        }
    }
    int nf = d.m_ArapFreeTV.size();

    vector < Eigen::Triplet < double > > uu, uk;
    uu.reserve( nt * 24 );
    uk.reserve( nt * 4 );
    for ( int t = 0; t < nt; t++ )
    {
        for ( int e = 0; e < 6; e++ )
        {
            int va = d.m_Shared->m_TT( t, TET_EDGES[e][0] );
            int vb = d.m_Shared->m_TT( t, TET_EDGES[e][1] );
            double c = C( t, e );
            int fa = d.m_ArapFree[va];
            int fb = d.m_ArapFree[vb];
            if ( fa >= 0 )
            {
                uu.push_back( Eigen::Triplet < double >( fa, fa, c ) );
                if ( fb >= 0 )
                {
                    uu.push_back( Eigen::Triplet < double >( fa, fb, -c ) );
                }
                else
                {
                    uk.push_back( Eigen::Triplet < double >( fa, held[vb], -c ) );
                }
            }
            if ( fb >= 0 )
            {
                uu.push_back( Eigen::Triplet < double >( fb, fb, c ) );
                if ( fa >= 0 )
                {
                    uu.push_back( Eigen::Triplet < double >( fb, fa, -c ) );
                }
                else
                {
                    uk.push_back( Eigen::Triplet < double >( fb, held[va], -c ) );
                }
            }
        }
    }
    Eigen::SparseMatrix < double > Auu( nf, nf );
    Auu.setFromTriplets( uu.begin(), uu.end() );
    d.m_ArapAuk.resize( nf, HUMAN_TET_NHOLD );
    d.m_ArapAuk.setFromTriplets( uk.begin(), uk.end() );

    Eigen::SimplicialLLT < Eigen::SparseMatrix < double > > llt( Auu );
    if ( llt.info() != Eigen::Success )
    {
        d.m_ARAPFailed = true;
        d.m_ARAPReady = true;
        return;
    }
    d.m_ArapPerm = llt.permutationP();
    d.m_ArapPermInv = llt.permutationPinv();
    Eigen::SparseMatrix < double > L = llt.matrixL();
    L.makeCompressed();
    d.m_LStart.assign( L.outerIndexPtr(), L.outerIndexPtr() + nf + 1 );
    d.m_LRow.assign( L.innerIndexPtr(), L.innerIndexPtr() + L.nonZeros() );
    d.m_LVal.assign( L.valuePtr(), L.valuePtr() + L.nonZeros() );

    d.m_ArapIncStart.assign( nf + 1, 0 );
    for ( int t = 0; t < nt; t++ )
    {
        for ( int k = 0; k < 4; k++ )
        {
            int f = d.m_ArapFree[ d.m_Shared->m_TT( t, k ) ];
            if ( f >= 0 )
            {
                d.m_ArapIncStart[ f + 1 ]++;
            }
        }
    }
    for ( int f = 0; f < nf; f++ )
    {
        d.m_ArapIncStart[ f + 1 ] += d.m_ArapIncStart[f];
    }
    d.m_ArapInc.resize( d.m_ArapIncStart[nf] );
    vector < int > fill( d.m_ArapIncStart.begin(), d.m_ArapIncStart.end() - 1 );
    for ( int t = 0; t < nt; t++ )
    {
        for ( int k = 0; k < 4; k++ )
        {
            int f = d.m_ArapFree[ d.m_Shared->m_TT( t, k ) ];
            if ( f >= 0 )
            {
                d.m_ArapInc[ fill[f] ] = 4 * t + k;
                fill[f]++;
            }
        }
    }
    d.m_ArapQ.assign( nt, Eigen::Quaterniond::Identity() );
    d.m_ARAPReady = true;
}

// Turns q towards the rotation nearest A (maximizing trace( R^T A )), one step of Mueller, Bender,
// Chentanez and Macklin, "A Robust Method to Extract the Rotational Part of Deformations" (2016), to
// first order in the step.  From a warm start one step per ARAP iteration suffices: its fixed point
// is the same.
static void TurnTowardsRotation( const Eigen::Matrix3d &A, Eigen::Quaterniond &q )
{
    Eigen::Matrix3d R = q.toRotationMatrix();
    Eigen::Vector3d w = R.col( 0 ).cross( A.col( 0 ) ) + R.col( 1 ).cross( A.col( 1 ) ) + R.col( 2 ).cross( A.col( 2 ) );
    w /= std::fabs( R.col( 0 ).dot( A.col( 0 ) ) + R.col( 1 ).dot( A.col( 1 ) ) + R.col( 2 ).dot( A.col( 2 ) ) ) + 1.0e-9;
    Eigen::Quaterniond dq( 1.0, 0.5 * w( 0 ), 0.5 * w( 1 ), 0.5 * w( 2 ) );
    q = dq * q;
    q.normalize();
}

// Solves Auu z = b by its factor, Auu = P^T L L^T P, sweeping each triangle once for all three
// columns.
static void FactorSolve( const HumanTetRigData &d, const Eigen::MatrixXd &b, Eigen::MatrixXd &z )
{
    int n = b.rows();
    Eigen::MatrixXd pb = d.m_ArapPerm * b;
    vector < double > y( 3 * n );
    for ( int i = 0; i < n; i++ )
    {
        y[ 3 * i ] = pb( i, 0 );
        y[ 3 * i + 1 ] = pb( i, 1 );
        y[ 3 * i + 2 ] = pb( i, 2 );
    }
    const int *st = d.m_LStart.data();
    const int *row = d.m_LRow.data();
    const double *val = d.m_LVal.data();
    // L y = P b; each column of L starts with its diagonal.
    for ( int j = 0; j < n; j++ )
    {
        double inv = 1.0 / val[ st[j] ];
        double y0 = y[ 3 * j ] * inv;
        double y1 = y[ 3 * j + 1 ] * inv;
        double y2 = y[ 3 * j + 2 ] * inv;
        y[ 3 * j ] = y0;
        y[ 3 * j + 1 ] = y1;
        y[ 3 * j + 2 ] = y2;
        for ( int k = st[j] + 1; k < st[ j + 1 ]; k++ )
        {
            double l = val[k];
            int i = 3 * row[k];
            y[i] -= l * y0;
            y[ i + 1 ] -= l * y1;
            y[ i + 2 ] -= l * y2;
        }
    }
    // L^T x = y.
    for ( int j = n - 1; j >= 0; j-- )
    {
        double s0 = y[ 3 * j ];
        double s1 = y[ 3 * j + 1 ];
        double s2 = y[ 3 * j + 2 ];
        for ( int k = st[j] + 1; k < st[ j + 1 ]; k++ )
        {
            double l = val[k];
            int i = 3 * row[k];
            s0 -= l * y[i];
            s1 -= l * y[ i + 1 ];
            s2 -= l * y[ i + 2 ];
        }
        double inv = 1.0 / val[ st[j] ];
        y[ 3 * j ] = s0 * inv;
        y[ 3 * j + 1 ] = s1 * inv;
        y[ 3 * j + 2 ] = s2 * inv;
    }
    Eigen::MatrixXd x( n, 3 );
    for ( int i = 0; i < n; i++ )
    {
        x( i, 0 ) = y[ 3 * i ];
        x( i, 1 ) = y[ 3 * i + 1 ];
        x( i, 2 ) = y[ 3 * i + 2 ];
    }
    z = d.m_ArapPermInv * x;
}

// Tet t's covariance at U.
static void ArapCovariance( const HumanTetRigData &d, const Eigen::MatrixXd &U, int t, Eigen::Matrix3d &S )
{
    S.setZero();
    for ( int k = 0; k < 4; k++ )
    {
        int v = d.m_Shared->m_TT( t, k );
        for ( int i = 0; i < 3; i++ )
        {
            double p = d.m_ArapP( t, 3 * k + i );
            for ( int j = 0; j < 3; j++ )
            {
                S( i, j ) += p * U( v, j );
            }
        }
    }
}

// One ARAP iteration: fit each tet's rotation to its covariance at U, then solve for the free
// vertices of U.  The first iteration of a pose fits by SVD, as igl::arap_solve does, and seeds the
// warm starts; the rest turn each tet's last rotation.  AukY is Auk times the held vertices.  The
// tets' fits and the free vertices' right-hand sides are each independent, and run in parallel.
static void ArapIteration( HumanTetRigData &d, const Eigen::MatrixXd &AukY, bool first, Matrix3dVec &S, Matrix3dVec &R,
                           Eigen::MatrixXd &U )
{
    int nt = d.m_Shared->m_TT.rows();
    int nf = d.m_ArapFreeTV.size();
    if ( first )
    {
        igl::parallel_for( nt, [&]( int t )
        {
            ArapCovariance( d, U, t, S[t] );
        }, 1000 );
        double smax = 0.0;
        for ( int t = 0; t < nt; t++ )
        {
            smax = std::max( smax, S[t].cwiseAbs().maxCoeff() );
        }
        igl::parallel_for( nt, [&]( int t )
        {
            // Scaled as igl::arap_solve scales them for its single precision SVD.
            Eigen::Matrix3d Sn = S[t] / smax;
            igl::polar_svd3x3( Sn, R[t] );
            d.m_ArapQ[t] = Eigen::Quaterniond( R[t] );
        }, 1000 );
    }
    else
    {
        igl::parallel_for( nt, [&]( int t )
        {
            Eigen::Matrix3d St;
            ArapCovariance( d, U, t, St );
            TurnTowardsRotation( St, d.m_ArapQ[t] );
            R[t] = d.m_ArapQ[t].toRotationMatrix();
        }, 1000 );
    }

    Eigen::MatrixXd rhs( nf, 3 );
    igl::parallel_for( nf, [&]( int f )
    {
        Eigen::RowVector3d r = -AukY.row( f );
        for ( int i = d.m_ArapIncStart[f]; i < d.m_ArapIncStart[ f + 1 ]; i++ )
        {
            int t = d.m_ArapInc[i] / 4;
            int k = d.m_ArapInc[i] % 4;
            Eigen::RowVector3d p( d.m_ArapP( t, 3 * k ), d.m_ArapP( t, 3 * k + 1 ), d.m_ArapP( t, 3 * k + 2 ) );
            r += p * R[t];
        }
        rhs.row( f ) = r;
    }, 1000 );

    Eigen::MatrixXd z;
    FactorSolve( d, rhs, z );
    for ( int f = 0; f < nf; f++ )
    {
        U.row( d.m_ArapFreeTV[f] ) = z.row( f );
    }
}

int HumanTetRig::PoseARAP( const vector < Matrix4d > &bone_mats, const vector < HumanPartialRot > &partials, int max_iter,
                           double tol, vector < vec3d > &skin )
{
    if ( m_Data->m_FASTReady )
    {
        ReleaseFAST( *m_Data );
    }
    if ( !m_Data->m_ARAPReady )
    {
        PrecomputeARAP();
    }
    if ( m_Data->m_ARAPFailed )
    {
        return 0;
    }
    HumanTetRigData &d = *m_Data;

    vector < Eigen::Matrix4d > T;
    HandleTransforms( bone_mats, partials, T );

    // Dual quaternion skinning of the tet mesh, the holds placed by their bones.
    vector < Eigen::Quaterniond, Eigen::aligned_allocator < Eigen::Quaterniond > > vQ( HUMAN_TET_NUM_HANDLES );
    vector < Eigen::Vector3d > vT( HUMAN_TET_NUM_HANDLES );
    for ( int k = 0; k < HUMAN_TET_NUM_HANDLES; k++ )
    {
        Eigen::Matrix3d R = T[k].block < 3, 3 >( 0, 0 );
        vQ[k] = Eigen::Quaterniond( R );
        vT[k] = T[k].block < 3, 1 >( 0, 3 );
    }
    Eigen::MatrixXd U;
    igl::dqs( d.m_TV, d.m_Shared->m_W, vQ, vT, U );
    Eigen::MatrixXd Y( HUMAN_TET_NHOLD, 3 );
    for ( int i = 0; i < HUMAN_TET_NHOLD; i++ )
    {
        int v = human_tet_hold_tv[i];
        Eigen::Vector4d p( d.m_TV( v, 0 ), d.m_TV( v, 1 ), d.m_TV( v, 2 ), 1.0 );
        Eigen::Vector4d q = T[ human_tet_hold_handle[i] ] * p;
        Y.row( i ) = q.head < 3 >();
        U.row( v ) = Y.row( i );
    }
    Eigen::MatrixXd AukY = d.m_ArapAuk * Y;

    // Iterate in blocks until the skin settles, with each iteration's covariances and rotations.
    Matrix3dVec S( d.m_Shared->m_TT.rows() );
    Matrix3dVec R( d.m_Shared->m_TT.rows() );
    int it = 0;
    while ( it < max_iter )
    {
        Eigen::MatrixXd Uold = U;
        for ( int k = 0; k < ARAP_BLOCK; k++ )
        {
            ArapIteration( d, AukY, it + k == 0, S, R, U );
        }
        it += ARAP_BLOCK;
        double mv = 0.0;
        for ( int i = 0; i < HUMAN_TET_NSKIN; i++ )
        {
            int v = human_tet_skin_tv[i];
            if ( v >= 0 )
            {
                mv = std::max( mv, ( U.row( v ) - Uold.row( v ) ).cwiseAbs().maxCoeff() );
            }
        }
        if ( mv < tol )
        {
            break;
        }
    }

    d.m_PosedTV = U;
    vector < vec3d > tv( U.rows() );
    for ( int v = 0; v < U.rows(); v++ )
    {
        tv[v].set_xyz( U( v, 0 ), U( v, 1 ), U( v, 2 ) );
    }
    SkinFromTets( tv, skin );
    return it;
}

// FAST's rotation groups of the tets (human_tet_group), as the sum matrix of the groups over the tets.
struct FASTGroups
{
    FASTGroups()
    {
        Eigen::Matrix < int, Eigen::Dynamic, 1 > G( HUMAN_TET_NTT );
        for ( int t = 0; t < HUMAN_TET_NTT; t++ )
        {
            G( t ) = human_tet_group[t];
        }
        igl::group_sum_matrix( G, m_Sum );
        m_K = m_Sum.rows();
    }

    int m_K;
    Eigen::SparseMatrix < double > m_Sum;
};

// The groups, made once on first use.
static const FASTGroups &GetFASTGroups()
{
    static const FASTGroups groups;
    return groups;
}

// The cotangent Laplacian, -2 igl::cotmatrix, from the cotangent entries C.
static void CotLaplacian( const Eigen::MatrixXd &C, const Eigen::MatrixXi &TT, int n, Eigen::SparseMatrix < double > &L )
{
    vector < Eigen::Triplet < double > > ijv;
    ijv.reserve( TT.rows() * 24 );
    for ( int t = 0; t < TT.rows(); t++ )
    {
        for ( int e = 0; e < 6; e++ )
        {
            int a = TT( t, TET_EDGES[e][0] );
            int b = TT( t, TET_EDGES[e][1] );
            double c = -2.0 * C( t, e );
            ijv.push_back( Eigen::Triplet < double >( a, b, c ) );
            ijv.push_back( Eigen::Triplet < double >( b, a, c ) );
            ijv.push_back( Eigen::Triplet < double >( a, a, -c ) );
            ijv.push_back( Eigen::Triplet < double >( b, b, -c ) );
        }
    }
    L.resize( n, n );
    L.setFromTriplets( ijv.begin(), ijv.end() );
}

// Coordinate dim of igl::arap_linear_block for per-tet rotations (tet vertices by tets).
static void EdgeBlock( const Eigen::MatrixXd &C, const Eigen::MatrixXd &V, const Eigen::MatrixXi &TT, int dim,
                       Eigen::SparseMatrix < double > &K )
{
    vector < Eigen::Triplet < double > > ijv;
    ijv.reserve( TT.rows() * 12 );
    for ( int t = 0; t < TT.rows(); t++ )
    {
        for ( int e = 0; e < 6; e++ )
        {
            int a = TT( t, TET_EDGES[e][0] );
            int b = TT( t, TET_EDGES[e][1] );
            double v = C( t, e ) * ( V( a, dim ) - V( b, dim ) );
            ijv.push_back( Eigen::Triplet < double >( a, t, v ) );
            ijv.push_back( Eigen::Triplet < double >( b, t, -v ) );
        }
    }
    K.resize( V.rows(), TT.rows() );
    K.setFromTriplets( ijv.begin(), ijv.end() );
}

// FAST (libigl's arap_dof with per-tet rotations in groups, no dynamics) with every handle's
// transform fixed, built sparse and condensed.  The three output coordinates share one system, so
// the unknowns are the 4 m entries of one coordinate's transforms: the 4 m - 4 h of the additional
// weights are free.
void HumanTetRig::PrecomputeFAST()
{
    HumanTetRigData &d = *m_Data;
    int n = d.m_TV.rows();
    int h = HUMAN_TET_NUM_HANDLES;
    int e = HUMAN_TET_NUM_EXTRA;
    int m = h + e;

    Eigen::MatrixXd Wall( n, m );
    Wall << d.m_Shared->m_W, d.m_Shared->m_We;
    int k = GetFASTGroups().m_K;

    vector < Eigen::Triplet < double > > ijv;
    ijv.reserve( n * 40 );
    for ( int v = 0; v < n; v++ )
    {
        for ( int j = 0; j < m; j++ )
        {
            double w = Wall( v, j );
            if ( w != 0.0 )
            {
                for ( int c = 0; c < 3; c++ )
                {
                    ijv.push_back( Eigen::Triplet < double >( v, c * m + j, w * d.m_TV( v, c ) ) );
                }
                ijv.push_back( Eigen::Triplet < double >( v, 3 * m + j, w ) );
            }
        }
    }
    Eigen::SparseMatrix < double > M1( n, 4 * m );
    M1.setFromTriplets( ijv.begin(), ijv.end() );
    d.m_M1Rows = M1;

    Eigen::MatrixXd C;
    igl::cotmatrix_entries( d.m_TV, d.m_Shared->m_TT, C );
    Eigen::SparseMatrix < double > L;
    CotLaplacian( C, d.m_Shared->m_TT, n, L );

    // The quadratic form M1^T L M1 and, per coordinate j, the covariance gather Gsum K_j^T M1: four
    // independent products, made in parallel.
    Eigen::MatrixXd Q;
    d.m_CSM.resize( 3 * k, 4 * m );
    igl::parallel_for( 4, [&]( int task )
    {
        if ( task == 3 )
        {
            Eigen::SparseMatrix < double > M1T = M1.transpose();
            Eigen::SparseMatrix < double > LM = L * M1;
            Q = Eigen::MatrixXd( M1T * LM );
        }
        else
        {
            Eigen::SparseMatrix < double > K;
            EdgeBlock( C, d.m_TV, d.m_Shared->m_TT, task, K );
            Eigen::SparseMatrix < double > KT = K.transpose();
            Eigen::SparseMatrix < double > GK = GetFASTGroups().m_Sum * KT;
            Eigen::SparseMatrix < double > GKM = GK * M1;
            d.m_CSM.block( task * k, 0, k, 4 * m ) = Eigen::MatrixXd( GKM );
        }
    }, 1 );

    // Free entries f (the additional weights') and fixed entries x (the handles'), per row of a
    // transform: minimizing over f gives f = Qff^-1 ( 2 CSM_f^T R - Qfx x ).
    int nf = 4 * e;
    int nx = 4 * h;
    vector < int > fr( nf );
    vector < int > fx( nx );
    for ( int c = 0; c < 4; c++ )
    {
        for ( int j = 0; j < e; j++ )
        {
            fr[ c * e + j ] = c * m + h + j;
        }
        for ( int i = 0; i < h; i++ )
        {
            fx[ c * h + i ] = c * m + i;
        }
    }
    Eigen::MatrixXd Qff( nf, nf );
    Eigen::MatrixXd Qfx( nf, nx );
    Eigen::MatrixXd Cf( nf, 3 * k );
    for ( int a = 0; a < nf; a++ )
    {
        for ( int b = 0; b < nf; b++ )
        {
            Qff( a, b ) = Q( fr[a], fr[b] );
        }
        for ( int b = 0; b < nx; b++ )
        {
            Qfx( a, b ) = Q( fr[a], fx[b] );
        }
        Cf.row( a ) = d.m_CSM.col( fr[a] ).transpose();
    }
    Eigen::PartialPivLU < Eigen::MatrixXd > lu( Qff );
    Eigen::MatrixXd X1 = lu.solve( Cf );
    Eigen::MatrixXd X2 = lu.solve( Qfx );
    if ( !X1.allFinite() || !X2.allFinite() )
    {
        d.m_FASTFailed = true;
        d.m_FASTReady = true;
        return;
    }
    d.m_Solve = Eigen::MatrixXd::Zero( 4 * m, 3 * k );
    d.m_Fix = Eigen::MatrixXd::Zero( 4 * m, nx );
    for ( int a = 0; a < nf; a++ )
    {
        d.m_Solve.row( fr[a] ) = 2.0 * X1.row( a );
        d.m_Fix.row( fr[a] ) = -X2.row( a );
    }
    for ( int b = 0; b < nx; b++ )
    {
        d.m_Fix( fx[b], b ) = 1.0;
    }
    d.m_FASTReady = true;
}

// FAST's tet vertices for the handle transforms T.
static void FASTTets( HumanTetRigData &d, const vector < Eigen::Matrix4d > &T, int iters, Eigen::MatrixXd &U )
{
    int h = HUMAN_TET_NUM_HANDLES;
    int m = h + HUMAN_TET_NUM_EXTRA;
    int k = GetFASTGroups().m_K;

    // Row c m + j, column r: entry ( r, c ) of transform j.  The additional weights start at zero.
    Eigen::MatrixXd Lh( 4 * h, 3 );
    Eigen::MatrixXd Lsep = Eigen::MatrixXd::Zero( 4 * m, 3 );
    for ( int i = 0; i < h; i++ )
    {
        for ( int c = 0; c < 4; c++ )
        {
            for ( int r = 0; r < 3; r++ )
            {
                Lh( c * h + i, r ) = T[i]( r, c );
                Lsep( c * m + i, r ) = T[i]( r, c );
            }
        }
    }
    Eigen::MatrixXd Lfix = d.m_Fix * Lh;

    // Local step: each group's best rotation, as igl::fit_rotations( S, false ) fits it; row c k + g of
    // Rxyz holds row c of group g's rotation.  Global step: the additional weights' transforms.
    Eigen::MatrixXd S( 3 * k, 3 );
    Eigen::MatrixXd Rxyz( 3 * k, 3 );
    for ( int it = 0; it < iters; it++ )
    {
        S = d.m_CSM * Lsep;
        for ( int g = 0; g < k; g++ )
        {
            Eigen::Matrix3d si, ri, ti, ui, vi;
            Eigen::Vector3d sv;
            for ( int i = 0; i < 3; i++ )
            {
                for ( int j = 0; j < 3; j++ )
                {
                    si( i, j ) = S( i * k + g, j );
                }
            }
            igl::polar_svd( si, ri, ti, ui, sv, vi );
            for ( int c = 0; c < 3; c++ )
            {
                for ( int j = 0; j < 3; j++ )
                {
                    Rxyz( c * k + g, j ) = ri( c, j );
                }
            }
        }
        Lsep = d.m_Solve * Rxyz + Lfix;
    }

    // U = M1 Lsep, by rows in parallel.
    U.resize( d.m_M1Rows.rows(), 3 );
    igl::parallel_for( d.m_M1Rows.rows(), [&]( int v )
    {
        Eigen::RowVector3d u = Eigen::RowVector3d::Zero();
        for ( Eigen::SparseMatrix < double, Eigen::RowMajor >::InnerIterator itr( d.m_M1Rows, v ); itr; ++itr )
        {
            u += itr.value() * Lsep.row( itr.index() );
        }
        U.row( v ) = u;
    }, 1000 );
}

void HumanTetRig::PoseFAST( const vector < Matrix4d > &bone_mats, const vector < HumanPartialRot > &partials, int iters,
                           vector < vec3d > &skin )
{
    if ( m_Data->m_ARAPReady )
    {
        ReleaseARAP( *m_Data );
    }
    if ( !m_Data->m_FASTReady )
    {
        PrecomputeFAST();
    }
    if ( m_Data->m_FASTFailed )
    {
        return;
    }

    vector < Eigen::Matrix4d > T;
    HandleTransforms( bone_mats, partials, T );
    Eigen::MatrixXd U;
    FASTTets( *m_Data, T, iters, U );
    m_Data->m_PosedTV = U;

    int ntv = U.rows();
    vector < vec3d > tv( ntv );
    for ( int v = 0; v < ntv; v++ )
    {
        tv[v].set_xyz( U( v, 0 ), U( v, 1 ), U( v, 2 ) );
    }
    SkinFromTets( tv, skin );
}

// A point's position in the posed tet t, from its affine coordinates in the rest one.
static Eigen::Vector3d TetImage( const HumanTetRigData &d, int t, const vec3d &rest )
{
    Eigen::Vector3d p0 = d.m_TV.row( d.m_Shared->m_TT( t, 0 ) ).transpose();
    Eigen::Matrix3d A;
    for ( int k = 0; k < 3; k++ )
    {
        A.col( k ) = d.m_TV.row( d.m_Shared->m_TT( t, k + 1 ) ).transpose() - p0;
    }
    Eigen::Vector3d b = A.partialPivLu().solve( Eigen::Vector3d( rest.x(), rest.y(), rest.z() ) - p0 );
    Eigen::Vector3d q = ( 1.0 - b.sum() ) * d.m_PosedTV.row( d.m_Shared->m_TT( t, 0 ) ).transpose();
    for ( int k = 0; k < 3; k++ )
    {
        q += b( k ) * d.m_PosedTV.row( d.m_Shared->m_TT( t, k + 1 ) ).transpose();
    }
    return q;
}

void HumanTetRig::PosePoints( const vector < vec3d > &rest, vector < vec3d > &posed ) const
{
    const HumanTetRigData &d = *m_Data;
    // The tet data embed the nets of HumanBezierData.h, point for point.
    assert( ( int ) rest.size() == HUMAN_TET_NCP );
    int n = std::min( ( int ) rest.size(), HUMAN_TET_NCP );
    posed = rest;
    for ( int i = 0; i < n; i++ )
    {
        Eigen::Vector3d q = 0.5 * ( TetImage( d, human_tet_cp_tet[i][0], rest[i] ) + TetImage( d, human_tet_cp_tet[i][1], rest[i] ) );
        posed[i].set_xyz( q( 0 ), q( 1 ), q( 2 ) );
    }
}
