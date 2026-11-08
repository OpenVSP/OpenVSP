//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//
//
//////////////////////////////////////////////////////////////////////

#include "HumanBezier.h"
#include "APIDefines.h"

#include "HumanBezierData.h"

//==== Cubic B-spline to Bezier ====//

// Insert the knot u once (Boehm).  The span k is the last one of the domain that starts at or
// before u; at the domain's end u closes span k, which gives the same points.
static void InsertKnot( vector < double > &knots, vector < vec3d > &cp, double u )
{
    int k = 3;
    while ( k + 1 < ( int )cp.size() && knots[ k + 1 ] <= u )
    {
        k++;
    }

    vector < vec3d > out( cp.size() + 1 );
    for ( int i = 0; i < ( int )out.size(); i++ )
    {
        if ( i <= k - 3 )
        {
            out[i] = cp[i];
        }
        else if ( i > k )
        {
            out[i] = cp[ i - 1 ];
        }
        else
        {
            double a = ( u - knots[i] ) / ( knots[ i + 3 ] - knots[i] );
            out[i] = cp[ i - 1 ] * ( 1.0 - a ) + cp[i] * a;
        }
    }
    cp.swap( out );
    knots.insert( knots.begin() + k + 1, u );
}

// The Bezier control points of a cubic B-spline over [ knots[3], knots[ ncp ] ], one curve
// segment after another, sharing their end points.
static void CubicBSplineToBezier( const vector < double > &knots_in, const vector < vec3d > &cp_in, vector < vec3d > &bez )
{
    vector < double > knots = knots_in;
    vector < vec3d > cp = cp_in;

    double u0 = knots[3];
    double u1 = knots[ cp.size() ];

    // Distinct knots of the domain, each raised to multiplicity three.
    vector < double > dist;
    for ( int i = 3; i <= ( int )cp.size(); i++ )
    {
        if ( dist.empty() || knots[i] > dist.back() )
        {
            dist.push_back( knots[i] );
        }
    }

    for ( int d = 0; d < ( int )dist.size(); d++ )
    {
        int mult = 0;
        for ( int i = 0; i < ( int )knots.size(); i++ )
        {
            if ( knots[i] == dist[d] )
            {
                mult++;
            }
        }
        for ( int m = mult; m < 3; m++ )
        {
            InsertKnot( knots, cp, dist[d] );
        }
    }

    // The curve passes through cp[ l - 3 ] at u0, l the last index of u0 among the knots.
    int l = 0;
    for ( int i = 0; i < ( int )knots.size(); i++ )
    {
        if ( knots[i] == u0 )
        {
            l = i;
        }
    }

    int nseg = ( int )dist.size() - 1;
    bez.assign( cp.begin() + l - 3, cp.begin() + l - 3 + 3 * nseg + 1 );
}

static const HumanBezPartData * GetHumanBezParts( int gender )
{
    if ( gender == vsp::FEMALE )
    {
        return hb_female_parts;
    }
    return hb_male_parts;
}

//==== Nets ====//

// One part's net in the data pose, P = Pbar + Q X.  A mirrored part is the mirror image of
// the stored right part with the around index reversed.
static void BuildHumanBezNet( const HumanBezPartData &part, const vector < double > &anthro, HumanBezNet &net )
{
    int nu = part.m_NU;
    int nv = part.m_NV;

    net.m_Pnts.resize( nu );
    for ( int i = 0; i < nu; i++ )
    {
        int iout = i;
        if ( net.m_Mirror )
        {
            // Control point i peaks at w = ( i - 1 ) / nu; this takes w to -w.
            iout = ( nu + 2 - i ) % nu;
        }
        net.m_Pnts[ iout ].resize( nv );

        for ( int j = 0; j < nv; j++ )
        {
            int c = i * nv + j;

            vec3d p;
            for ( int k = 0; k < 3; k++ )
            {
                int row = 3 * c + k;
                double v = part.m_PBar[ row ];
                for ( int m = 0; m < 6; m++ )
                {
                    v += part.m_Q[ 6 * row + m ] * anthro[m];
                }
                p[k] = v;
            }
            if ( net.m_Mirror )
            {
                p.scale_y( -1.0 );
            }
            net.m_Pnts[ iout ][j] = p;
        }
    }
}

void BuildHumanBezierNets( int gender, const vector < double > &anthro, vector < HumanBezNet > &nets )
{
    const HumanBezPartData *parts = GetHumanBezParts( gender );

    nets.clear();
    for ( int ip = 0; ip < HUMAN_BEZ_NUM_PART; ip++ )
    {
        int nside = 2;
        if ( parts[ip].m_Centre )
        {
            nside = 1;
        }

        for ( int side = 0; side < nside; side++ )
        {
            nets.push_back( HumanBezNet() );
            nets.back().m_Part = ip;
            nets.back().m_Mirror = ( side == 1 );
            BuildHumanBezNet( parts[ip], anthro, nets.back() );
        }
    }
}

//==== Surfaces ====//

// The Bezier patches of a closed part's B-spline net, as a VspSurf: u along, w around.
static void BuildHumanBezSurf( const HumanBezPartData &part, const vector < vector < vec3d > > &net, VspSurf &surf )
{
    int nu = part.m_NU;
    int nv = part.m_NV;

    vector < double > vknots( part.m_VKnots, part.m_VKnots + nv + 4 );
    vector < double > uknots( part.m_UKnots, part.m_UKnots + nu + 7 );

    // Along each around index.
    vector < vector < vec3d > > along( nu );
    for ( int i = 0; i < nu; i++ )
    {
        CubicBSplineToBezier( vknots, net[i], along[i] );
    }

    // Around each along Bezier index, the periodic net unrolled.
    int na = ( int )along[0].size();
    vector < vector < vec3d > > cpts( na );
    for ( int a = 0; a < na; a++ )
    {
        vector < vec3d > ring( nu + 3 );
        for ( int i = 0; i < nu + 3; i++ )
        {
            ring[i] = along[ i % nu ][a];
        }
        CubicBSplineToBezier( uknots, ring, cpts[a] );

        // Closed around exactly.
        cpts[a].back() = cpts[a][0];
    }

    // Poles exactly.
    for ( int b = 0; b < ( int )cpts[0].size(); b++ )
    {
        cpts[0][b] = net[0][0];
        cpts[ na - 1 ][b] = net[0][ nv - 1 ];
    }

    surf.CreateBicubic( cpts );
    surf.SetSurfType( vsp::NORMAL_SURF );
    surf.SetMagicVParm( false );
}

void BuildHumanBezierSurfs( int gender, const vector < HumanBezNet > &nets, const Matrix4d &place,
                            vector < VspSurf > &surfs )
{
    const HumanBezPartData *parts = GetHumanBezParts( gender );

    surfs.resize( nets.size() );
    for ( int n = 0; n < ( int )nets.size(); n++ )
    {
        vector < vector < vec3d > > pnts = nets[n].m_Pnts;
        for ( int i = 0; i < ( int )pnts.size(); i++ )
        {
            place.xformvec( pnts[i] );
        }

        BuildHumanBezSurf( parts[ nets[n].m_Part ], pnts, surfs[n] );
    }
}

//==== Eyes ====//

void GetHumanBezierEyes( int gender, const vector < HumanBezNet > &nets, const vector < VspSurf > &surfs,
                         vec3d &reye, vec3d &leye )
{
    const double *uw = hb_male_eye_uw;
    if ( gender == vsp::FEMALE )
    {
        uw = hb_female_eye_uw;
    }

    for ( int n = 0; n < ( int )nets.size() && n < ( int )surfs.size(); n++ )
    {
        if ( nets[n].m_Part == HUMAN_BEZ_EYE_PART && !nets[n].m_Mirror )
        {
            reye = surfs[n].CompPnt( uw[0], uw[1] );
            leye = surfs[n].CompPnt( uw[2], uw[3] );
            return;
        }
    }
}
