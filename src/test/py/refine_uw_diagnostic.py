# Diagnostic for the seeded surface projection that Fit Model's Refine UW is built on.
#
# Refine UW calls TargetPt::RefineUW -> VspSurf::FindNearest01( u, w, pt, u0, w0 ) ->
# eli::geom::intersect::minimum_distance( ..., u0, v0 ), the seeded form.  Search UW calls the
# unseeded form, which sweeps the surface first.  The API exposes both directly as ProjPnt01Guess
# and ProjPnt01, so the two can be compared without going through Fit Model at all.
#
# This reports two things, and the second matters as much as the first:
#
#   Robustness -- how many seeds reached the global minimum, and how many merely settled: an
#   answer that is a genuine local minimum is not a failure for a seeded search.  What is left is
#   classified -- returned the seed, stuck at a degenerate pole, stopped on an ordinary edge,
#   stopped short of stationary, or converged to a saddle.
#
#   Cost -- the whole distribution, not a few hand picked seeds.  A seeded projection has a fast
#   path and one or more fallbacks, and an average says nothing useful about either: what matters
#   is how quick the fast path is, how dear the slow path is, and how often the slow path is taken.
#   So the median, the 90th and 99th percentiles and the maximum are all reported, along with an
#   estimate of how often the fallback runs.
#
# Seeds are a Latin hypercube with a fixed seed, so the sample covers the parameter space evenly
# and repeats exactly between runs.  A regular grid was what this used to do, and it lines seeds up
# with the symmetries of a body of revolution -- which both flatters the result and misses whole
# failure modes.  Field points are placed in several relations to the body, since a point off the
# nose, one beam on, and one beyond the local centre of curvature are different problems.

import math
import random
import time

import openvsp as vsp


def latin_hypercube( n, seed ):
    """n points in the unit square, one per row and per column, from a fixed seed."""
    rng = random.Random( seed )

    us = [ ( i + rng.random() ) / n for i in range( n ) ]
    ws = [ ( i + rng.random() ) / n for i in range( n ) ]

    rng.shuffle( us )
    rng.shuffle( ws )

    return list( zip( us, ws ) )


def field_points( pid ):
    """Target points in different relations to the body, each with a label.

    A pod runs along X from 0 to about 4, with a radius of about 0.6, so these are placed by hand
    against that rather than derived, and the labels say what each is meant to probe.
    """
    pts = []

    # Beam on, at three distances.  The classic case: one minimum on the near side, one maximum on
    # the far side, and the whole far side ridge in between.
    pts.append( ( "beam far",       vsp.vec3d(  2.0,  4.0,  0.0 ) ) )
    pts.append( ( "beam mid",       vsp.vec3d(  2.0,  1.5,  0.0 ) ) )
    pts.append( ( "beam near",      vsp.vec3d(  2.0,  0.8,  0.0 ) ) )

    # Off the ends, where the surface is nearly degenerate and the parameter directions collapse.
    pts.append( ( "off nose",       vsp.vec3d( -1.5,  0.0,  0.0 ) ) )
    pts.append( ( "off tail",       vsp.vec3d(  6.0,  0.0,  0.0 ) ) )
    pts.append( ( "past nose off",  vsp.vec3d( -0.5,  0.5,  0.3 ) ) )

    # Out of the symmetry plane, so the answer is not on a seam or a patch boundary.
    pts.append( ( "oblique",        vsp.vec3d(  1.0,  1.2, -1.4 ) ) )
    pts.append( ( "above",          vsp.vec3d(  2.5,  0.0,  2.0 ) ) )

    # Close in.  A point just off the surface is the case a mesher generates by the million, and
    # one inside the local radius of curvature behaves quite differently from one beyond it.
    pts.append( ( "just off",       vsp.vec3d(  2.0,  0.7,  0.0 ) ) )
    pts.append( ( "inside curv",    vsp.vec3d(  2.0,  0.3,  0.0 ) ) )

    # On the axis, where every W is equidistant and the problem is degenerate by symmetry.
    pts.append( ( "on axis",        vsp.vec3d(  2.0,  0.0,  0.0 ) ) )

    return pts


def classify_failure( pid, target, u0, w0, u, w, d, dbest ):
    """Why did this case not reach the minimum?

    Worked out from the answer alone, with a handful of surface queries -- no instrumentation
    inside the solver.  The point is to find diagnostics a solver could cheaply apply to itself,
    so anything needing more than the surface and its first derivatives is no use here.
    """
    if abs( u - u0 ) < 1.0e-12 and abs( w - w0 ) < 1.0e-12:
        return "returned the seed"

    # Is anything nearby better?  This comes first, because it is the question that decides whether
    # the solver did its job.  An answer with nothing lower around it is a local minimum, and a
    # seeded solve that converges to one has not failed -- a wing's root end cap is flat and has
    # its own nearest point, and a seed inside that basin is entitled to end there.  Probing is
    # clamped to the domain, so a constrained minimum on a boundary counts too; the Hessian would
    # not recognise one and neither would a test built on the gradient alone.
    h = 1.0e-3
    lower = False
    for du, dw in ( ( h, 0 ), ( -h, 0 ), ( 0, h ), ( 0, -h ), ( h, h ), ( -h, -h ), ( h, -h ), ( -h, h ) ):
        un = min( max( u + du, 0.0 ), 1.0 )
        wn = min( max( w + dw, 0.0 ), 1.0 )
        q = vsp.CompPnt01( pid, 0, un, wn )
        dn = ( ( q.x() - target.x() )**2 + ( q.y() - target.y() )**2 + ( q.z() - target.z() )**2 )**0.5
        if dn < d - 1.0e-12:
            lower = True
            break

    if not lower:
        return "a genuine second minimum"

    # Something nearby is better, so the solver stopped where it should not have.  Why?
    eps = 1.0e-9

    p = vsp.CompPnt01( pid, 0, u, w )
    r = vsp.vec3d( p.x() - target.x(), p.y() - target.y(), p.z() - target.z() )
    su = vsp.CompTanU01( pid, 0, u, w )
    sv = vsp.CompTanW01( pid, 0, u, w )

    rm, sum_, svm = r.mag(), su.mag(), sv.mag()

    on_edge = ( u < eps ) or ( u > 1 - eps ) or ( w < eps ) or ( w > 1 - eps )

    if on_edge:
        # Which edge, and is it degenerate?  A pod's nose and tail are poles: the whole W line
        # collapses to a point, so the W derivative vanishes and the U derivative points straight
        # into the singularity.  That is a different problem from running into an ordinary
        # boundary, and wants a different remedy.
        which = []
        if u < eps:
            which.append( "u=0" )
        if u > 1 - eps:
            which.append( "u=1" )
        if w < eps:
            which.append( "w=0" )
        if w > 1 - eps:
            which.append( "w=1" )

        degenerate = ( svm < 1.0e-6 * max( 1.0, sum_ ) ) or ( sum_ < 1.0e-6 * max( 1.0, svm ) )

        if degenerate:
            return "stuck at a degenerate pole (%s)" % ",".join( which )

        return "stopped on an ordinary edge (%s)" % ",".join( which )

    stationary = True
    if rm > 1.0e-9 and sum_ > 0 and svm > 0:
        cosu = abs( vsp.dot( su, r ) ) / ( sum_ * rm )
        cosv = abs( vsp.dot( sv, r ) ) / ( svm * rm )
        stationary = ( cosu < 1.0e-6 ) and ( cosv < 1.0e-6 )

    if not stationary:
        return "stopped short, not stationary"

    return "converged to a saddle or maximum"


def bbox_field_points( pid ):
    """Field points derived from the bounding box, so any geometry can be swept.

    Placed relative to the box rather than by hand, since the point of sweeping a second and third
    geometry is to find out whether fixes worked out on a pod generalise -- and hand placing would
    smuggle the pod's proportions into every one of them.  The set covers the same relations the
    pod set does: broadside at three distances, off each end along the long axis, out of plane,
    and one inside the box.
    """
    lo = vsp.GetGeomBBoxMin( pid, 0, False )
    hi = vsp.GetGeomBBoxMax( pid, 0, False )

    cx, cy, cz = 0.5*( lo.x() + hi.x() ), 0.5*( lo.y() + hi.y() ), 0.5*( lo.z() + hi.z() )
    dx, dy, dz = hi.x() - lo.x(), hi.y() - lo.y(), hi.z() - lo.z()

    d = max( dx, dy, dz )

    pts = []

    pts.append( ( "broad far",  vsp.vec3d( cx, cy + 2.0*d,  cz ) ) )
    pts.append( ( "broad mid",  vsp.vec3d( cx, cy + 0.5*d,  cz ) ) )
    pts.append( ( "broad near", vsp.vec3d( cx, hi.y() + 0.02*d, cz ) ) )
    pts.append( ( "off lo x",   vsp.vec3d( lo.x() - 0.4*d, cy, cz ) ) )
    pts.append( ( "off hi x",   vsp.vec3d( hi.x() + 0.4*d, cy, cz ) ) )
    pts.append( ( "oblique",    vsp.vec3d( cx + 0.2*d, cy + 0.4*d, cz - 0.5*d ) ) )
    pts.append( ( "above",      vsp.vec3d( cx, cy, hi.z() + 0.5*d ) ) )
    pts.append( ( "inside",     vsp.vec3d( cx, cy, cz ) ) )

    return pts


def sweep( pid, pts, seeds ):
    """Run every seed against every field point.  Returns ( reached, settled, total, excesses ).

    'Reached' is the global minimum.  'Settled' counts those plus every answer that is a genuine
    local minimum, which for a seeded solve is not a failure at all: a wing's root end cap is flat
    and has its own nearest point, and a seed inside that basin is entitled to converge to it.
    Only what is left -- stopped on an edge it should have crossed, or stopped nowhere in
    particular -- is the solver failing to do its job.
    """
    reached = 0
    settled = 0
    total = 0
    excess = []

    for label, target in pts:
        dbest, ubest, wbest = vsp.ProjPnt01( pid, 0, target )

        for u0, w0 in seeds:
            d, u, w = vsp.ProjPnt01Guess( pid, 0, target, u0, w0 )

            total += 1

            if abs( d - dbest ) < 1.0e-6:
                reached += 1
                settled += 1
            else:
                if dbest > 0:
                    excess.append( ( d - dbest ) / dbest )
                else:
                    excess.append( d - dbest )

                if classify_failure( pid, target, u0, w0, u, w, d, dbest ) == "a genuine second minimum":
                    settled += 1

    return reached, settled, total, excess


def other_geometries( seeds ):
    """The same sweep on shapes that are not a body of revolution.

    A pod is closed and smooth in both directions and every one of its cross sections is the same
    circle, which makes it a poor sole witness: a fix tuned on it can be reading its symmetries
    rather than the problem.  A wing has a sharp trailing edge and a planform that varies, a
    fuselage has corners in its sections, and a stack is a run of joined segments.
    """
    print( "\n\nother geometries, %d seeds x 8 bounding box field points each\n" % len( seeds ) )

    print( "%-10s %6s %8s %10s %10s %10s"
           % ( "geom", "reach", "settled", "n adrift", "median off", "max off" ) )

    for gtype in ( "WING", "FUSELAGE", "STACK" ):
        vsp.VSPRenew()
        pid = vsp.AddGeom( gtype )
        vsp.Update()

        pts = bbox_field_points( pid )

        reached, settled, total, excess = sweep( pid, pts, seeds )

        if excess:
            excess.sort()
            med = percentile( excess, 0.50 )
            mx = excess[-1]
        else:
            med = 0.0
            mx = 0.0

        print( "%-10s %5d%% %7d%% %8d %10.2e %10.2e"
               % ( gtype, 100*reached//total, 100*settled//total, total - settled, med, mx ) )


def percentile( xs, q ):
    ys = sorted( xs )
    k = min( len( ys ) - 1, max( 0, int( round( q * ( len( ys ) - 1 ) ) ) ) )
    return ys[ k ]


def main():
    vsp.VSPRenew()
    pid = vsp.AddGeom( "POD" )
    vsp.Update()

    seeds = latin_hypercube( 200, seed = 20260820 )
    pts = field_points( pid )

    print( "POD, %d Latin hypercube seeds x %d field points = %d cases\n"
           % ( len( seeds ), len( pts ), len( seeds ) * len( pts ) ) )

    print( "%-14s %8s %7s %7s %7s   %s"
           % ( "field point", "best d", "reach", "stuck", "other", "notes" ) )

    all_times = []
    kinds = {}
    excess = []
    tot_reach = tot_stuck = tot_other = tot = 0

    for label, target in pts:
        dbest, ubest, wbest = vsp.ProjPnt01( pid, 0, target )

        reach = stuck = other = 0

        for u0, w0 in seeds:
            d, u, w = vsp.ProjPnt01Guess( pid, 0, target, u0, w0 )

            if abs( d - dbest ) < 1.0e-6:
                reach += 1
            else:
                excess.append( ( d - dbest ) / dbest )
                kind = classify_failure( pid, target, u0, w0, u, w, d, dbest )
                kinds[ kind ] = kinds.get( kind, 0 ) + 1
                if kind == "returned the seed":
                    stuck += 1
                else:
                    other += 1

        note = ""
        if stuck > 0:
            note = "%d seeds did not move" % stuck

        print( "%-14s %8.4f %6d%% %6d%% %6d%%   %s"
               % ( label, dbest, 100*reach//len(seeds), 100*stuck//len(seeds), 100*other//len(seeds), note ) )

        tot_reach += reach
        tot_stuck += stuck
        tot_other += other
        tot += len( seeds )

    print( "\noverall: %d%% reached the minimum, %d%% did not move, %d%% stopped elsewhere"
           % ( 100*tot_reach//tot, 100*tot_stuck//tot, 100*tot_other//tot ) )

    print( "\nfailures by kind, of %d that missed the minimum:" % ( tot - tot_reach ) )
    for k in sorted( kinds, key = lambda x: -kinds[x] ):
        print( "  %-34s %5d  %3d%%" % ( k, kinds[k], 100*kinds[k]//max(1,tot-tot_reach) ) )

    # How wrong is wrong?  A binary reached/missed hides the difference between an answer on the
    # far side of the body and one a few parts in ten thousand short, and on this model most of
    # what misses is the latter: the pod's circular section is four cubic arcs and ripples by
    # 3.9e-4 in radius, so each arc carries its own local minimum.  Landing on the neighbouring
    # one is not a solver failure and counting it as one sets a target no solver can meet.
    if excess:
        excess.sort()
        print( "\n how far short, relative, over the %d that missed:" % len( excess ) )
        print( "   median %.2e   p90 %.2e   max %.2e" % ( percentile( excess, 0.50 ),
                                                          percentile( excess, 0.90 ), excess[-1] ) )
        for thr in ( 1.0e-3, 1.0e-2, 1.0e-1 ):
            n = len( [ x for x in excess if x > thr ] )
            print( "   %5d worse than %.0e relative  (%d%% of all %d cases)"
                   % ( n, thr, 100*n//tot, tot ) )

    # --- cost, as a distribution -------------------------------------------------------------
    #
    # Timed one call at a time.  A loop over one seed measures a single path through the solver
    # and says nothing about the rest; this walks every case once so the percentiles describe what
    # a caller actually meets.
    print( "\ntiming over all %d cases, microseconds:" % ( len( seeds ) * len( pts ) ) )

    for label, target in pts:
        ts = []
        for u0, w0 in seeds:
            t0 = time.perf_counter()
            vsp.ProjPnt01Guess( pid, 0, target, u0, w0 )
            ts.append( ( time.perf_counter() - t0 ) * 1.0e6 )

        all_times += ts

        print( "  %-14s median %6.1f   p90 %6.1f   p99 %7.1f   max %7.1f"
               % ( label, percentile( ts, 0.50 ), percentile( ts, 0.90 ),
                   percentile( ts, 0.99 ), max( ts ) ) )

    total_ms = sum( all_times )/1000.0
    med = percentile( all_times, 0.50 )
    print( "\n  %-14s median %6.1f   p90 %6.1f   p99 %7.1f   max %7.1f"
           % ( "ALL", med, percentile( all_times, 0.90 ),
               percentile( all_times, 0.99 ), max( all_times ) ) )

    # How often the fallback runs.  Anything several times the median is not the fast path.
    slow = len( [ t for t in all_times if t > 3*med ] )
    print( "  %d%% of calls took more than 3x the median -- that is the fallback rate"
           % ( 100*slow//len( all_times ) ) )
    print( "\n  TOTAL to solve all %d cases: %.1f ms" % ( len( all_times ), total_ms ) )

    other_geometries( seeds )


if __name__ == '__main__':
    main()
