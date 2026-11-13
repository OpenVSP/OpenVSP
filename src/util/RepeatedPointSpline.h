//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//

// RepeatedPointSpline.h
//
// A point listed twice in a row at the same parameter marks a corner.  A spline through
// such data is fit as separate runs between the repeated points, joined end to end, so
// it is continuous at a corner with independent slopes on either side.  Two points at
// the same parameter that are not the same point are a step, fit as a steep segment.
//
//////////////////////////////////////////////////////////////////////

#if !defined(REPEATEDPOINTSPLINE__INCLUDED_)
#define REPEATEDPOINTSPLINE__INCLUDED_

#include <vector>

#include "eli/code_eli.hpp"

using std::vector;

// Indices k of the points that repeat point k - 1 at the same parameter.
template < typename point__ >
vector< int > FindRepeatedPoints( const vector< point__ > & pts, const vector< double > & param )
{
    vector< int > rep;
    for ( int k = 1; k < ( int ) pts.size(); k++ )
    {
        if ( pts[k] == pts[k - 1] && param[k] == param[k - 1] )
        {
            rep.push_back( k );
        }
    }
    return rep;
}

// True when a point repeats the one before it.  A closed list wraps from its last point
// to its first at the final parameter.
template < typename point__ >
bool HasRepeatedPoint( const vector< point__ > & pts, const vector< double > & param, bool closed_flag )
{
    int n = ( int ) pts.size();
    for ( int k = 1; k < n; k++ )
    {
        if ( pts[k] == pts[k - 1] && param[k] == param[k - 1] )
        {
            return true;
        }
    }

    if ( closed_flag && n > 0 )
    {
        if ( pts[0] == pts[n - 1] && param[n] == param[n - 1] )
        {
            return true;
        }
    }
    return false;
}

// Point k of a list whose parameters may go one past its points, wrapping to its first.
template < typename point__ >
const point__ & WrappedPoint( const vector< point__ > & pts, int k )
{
    if ( k < ( int ) pts.size() )
    {
        return pts[k];
    }
    return pts[0];
}

// Moves a point that shares its parameter with the one before it, but is not the same
// point, a millionth of the parameter range away from it, so the step between them is a
// segment of its own.  The point moves forward, or the one before it back when the next
// parameter leaves no room.  The first and last parameters do not change.
template < typename point__ >
void SeparateSharedParameters( const vector< point__ > & pts, vector< double > & param )
{
    int n = ( int ) param.size();
    if ( n < 2 || pts.empty() )
    {
        return;
    }

    double dt = 1.0e-6 * ( param[n - 1] - param[0] );
    if ( dt <= 0.0 )
    {
        return;
    }

    for ( int k = 1; k < n; k++ )
    {
        if ( param[k] == param[k - 1] && !( WrappedPoint( pts, k ) == WrappedPoint( pts, k - 1 ) ) )
        {
            if ( k < n - 1 && param[k] + dt < param[k + 1] )
            {
                param[k] = param[k] + dt;
            }
            else if ( k - 1 > 0 && param[k - 2] < param[k - 1] - dt )
            {
                param[k - 1] = param[k - 1] - dt;
            }
        }
    }
}

// Removes each point that repeats the one before it at the same parameter.
template < typename point__ >
void DropRepeatedPoints( vector< point__ > & pts, vector< double > & param )
{
    vector< int > rep = FindRepeatedPoints( pts, param );
    for ( int i = ( int ) rep.size() - 1; i >= 0; i-- )
    {
        pts.erase( pts.begin() + rep[i] );
        param.erase( param.begin() + rep[i] );
    }
}

// Fits points istart to iend with an open ended PCHIP, or a line when there are only two.
template < typename creator__, typename point__, typename curve__ >
bool FitPCHIPRun( const vector< point__ > & pts, const vector< double > & param, int istart, int iend, curve__ & crv )
{
    int nseg = iend - istart;
    creator__ pcsc( nseg );

    pcsc.set_t0( param[istart] );
    for ( int i = 0; i < nseg; i++ )
    {
        pcsc.set_segment_dt( param[istart + i + 1] - param[istart + i], i );
    }

    if ( nseg == 1 )
    {
        point__ m = ( pts[iend] - pts[istart] ) / ( param[iend] - param[istart] );
        pcsc.set_segment_point_slope( pts[istart], m, pts[iend], m, 0 );
    }
    else
    {
        pcsc.set_chip( pts.begin() + istart, eli::geom::general::NOT_CONNECTED );
    }

    return pcsc.create( crv );
}

// Fits points istart to iend as runs split at the repeated points rep, joined end to end.
// A run of a single point adds nothing, so crv may be left empty.
template < typename creator__, typename point__, typename curve__ >
bool FitPCHIPRuns( const vector< point__ > & pts, const vector< double > & param, const vector< int > & rep,
                   int istart, int iend, curve__ & crv )
{
    vector< int > ends;
    for ( size_t i = 0; i < rep.size(); i++ )
    {
        if ( rep[i] > istart && rep[i] <= iend )
        {
            ends.push_back( rep[i] - 1 );
        }
    }
    ends.push_back( iend );

    crv.clear();
    crv.set_t0( param[istart] );

    bool first = true;
    int irun = istart;
    for ( size_t i = 0; i < ends.size(); i++ )
    {
        if ( ends[i] > irun )
        {
            curve__ piece;
            if ( !FitPCHIPRun< creator__ >( pts, param, irun, ends[i], piece ) )
            {
                return false;
            }

            if ( first )
            {
                crv = piece;
                first = false;
            }
            else
            {
                crv.push_back( piece );
            }
        }
        irun = ends[i] + 1;
    }

    return true;
}

// Copies segments ifirst to ilast of crv into piece, starting it at parameter t0.
template < typename curve__ >
void CopySegments( const curve__ & crv, int ifirst, int ilast, double t0, curve__ & piece )
{
    piece.clear();
    piece.set_t0( t0 );
    for ( int i = ifirst; i <= ilast; i++ )
    {
        typename curve__::curve_type seg;
        double dt;
        crv.get( seg, dt, i );
        piece.push_back( seg, dt );
    }
}

// PCHIP through points with repeated points as corners.  A closed list has one more
// parameter than points and wraps to its first point at the last one.  Unless the wrap
// is itself a corner, the run through it is fit in one piece, so the curve stays smooth
// there, and then split at the wrap to start and end the curve.
template < typename creator__, typename point__, typename curve__ >
bool InterpolatePCHIPCorners( const vector< point__ > & pts, const vector< double > & param, bool closed_flag, curve__ & crv )
{
    vector< point__ > ext( pts );
    if ( closed_flag )
    {
        ext.push_back( pts[0] );
    }

    vector< int > rep = FindRepeatedPoints( ext, param );
    int n = ( int ) ext.size() - 1;

    if ( !closed_flag || rep.empty() || rep.front() == 1 || rep.back() == n )
    {
        if ( !FitPCHIPRuns< creator__ >( ext, param, rep, 0, n, crv ) )
        {
            return false;
        }
        return crv.number_segments() > 0;
    }

    // Run through the wrap, from the last repeated point round to the first.
    double period = param[n] - param[0];
    vector< point__ > wpts;
    vector< double > wparam;
    for ( int k = rep.back(); k <= n; k++ )
    {
        wpts.push_back( ext[k] );
        wparam.push_back( param[k] );
    }
    for ( int k = 1; k < rep.front(); k++ )
    {
        wpts.push_back( ext[k] );
        wparam.push_back( param[k] + period );
    }

    curve__ wrap;
    if ( !FitPCHIPRun< creator__ >( wpts, wparam, 0, ( int ) wpts.size() - 1, wrap ) )
    {
        return false;
    }

    int nbefore = n - rep.back();
    curve__ before, after;
    CopySegments( wrap, 0, nbefore - 1, param[rep.back()], before );
    CopySegments( wrap, nbefore, wrap.number_segments() - 1, param[0], after );

    curve__ middle;
    if ( !FitPCHIPRuns< creator__ >( ext, param, rep, rep.front(), rep.back() - 1, middle ) )
    {
        return false;
    }

    crv = after;
    crv.push_back( middle );
    crv.push_back( before );
    return true;
}

#endif // !defined(REPEATEDPOINTSPLINE__INCLUDED_)
