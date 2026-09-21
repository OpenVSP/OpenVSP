# Surface Intersection is the first half of CFD mesh: it loads the Bezier surfaces, builds the
# grid of border curves, matches them up, intersects every surface pair and hands back the chains.
# No triangles are made, so it runs in a fraction of a second on models the mesher takes minutes
# over, and everything it does can be read back out of the results entry it writes: each chain's
# raw points, and how many belong to each chain.
#
# What is worth pinning there is the shape of the chain network rather than any one coordinate.
# A chain that was never split where its neighbour breaks, a border pair cut to two different
# point counts, a pair of coincident chain ends that were not merged, a bounding box test that
# turned away a pair it should have let through -- each of those shows up as a chain end with
# nothing to meet, or as a chain count that moved.  Two runs of one model must also give the
# same chains: the intersection stage runs on several threads, and the point order it builds the
# chains from is the thing that makes a mesh reproducible at all.
#
# Note that IntersectSurfaces hides every Geom on its way through, so an analysis left on the
# default (shown) set finds nothing to do the second time it is run.  Every run here asks for
# SET_ALL.

import math

import openvsp as vsp
import pytest


TOL = 1e-6          # Coincident points, as a fraction of the 10 unit model.


def _errors():
    """How many errors are on the API stack.

    The stack is never emptied, so a count is only meaningful against an earlier count from
    the same process -- an absolute zero would be asserting about every test that ran before.
    """
    return vsp.ErrorMgrSingleton.getInstance().GetNumTotalErrors()


def _intersect( parallel = None ):
    """Run Surface Intersection over everything in the model and give back its chains."""
    if parallel is not None:
        settings = vsp.FindContainer( "SurfaceIntersectSettings", 0 )
        vsp.SetParmVal( vsp.FindParm( settings, "ParallelMeshFlag", "Global" ), float( parallel ) )

    vsp.SetIntAnalysisInput( "SurfaceIntersection", "SelectedSetIndex", [ vsp.SET_ALL ] )

    for flag in ( "IGESFileFlag", "STEPFileFlag" ):
        vsp.SetIntAnalysisInput( "SurfaceIntersection", flag, [ 0 ] )

    rid = vsp.ExecAnalysis( "SurfaceIntersection" )
    assert rid, "Surface Intersection returned no results"

    return _chains( rid )


def _chains( rid ):
    """The chains in a results entry, each a list of (x,y,z)."""
    counts = vsp.GetIntResults( rid, "Curve_Num_Pnts" )
    pnts = vsp.GetVec3dResults( rid, "Curve_Pnts" )

    out = []
    i = 0
    for n in counts:
        out.append( [ ( p.x(), p.y(), p.z() ) for p in pnts[i:i + n] ] )
        i += n

    return out


def _end_clusters( chains ):
    """Group the chain ends that land on the same point, and say how many ends each holds."""
    clusters = []

    for chain in chains:
        for end in ( chain[0], chain[-1] ):
            for c in clusters:
                if math.dist( c[0], end ) < TOL:
                    c[1] += 1
                    break
            else:
                clusters.append( [ end, 1 ] )

    return [ c[1] for c in clusters ]


def _pod_and_wing():
    """A wing run through the middle of a pod, so the two really do intersect."""
    vsp.VSPRenew()
    vsp.AddGeom( "POD", "" )
    wing = vsp.AddGeom( "WING", "" )
    vsp.SetParmVal( vsp.FindParm( wing, "X_Rel_Location", "XForm" ), 2.0 )
    vsp.SetParmVal( vsp.FindParm( wing, "TotalSpan", "WingGeom" ), 8.0 )
    vsp.Update()
    return wing


def testTheIntersectorIsRegisteredAsAnAnalysis():
    """It registers itself, so a script can reach it by name without a GUI."""
    vsp.VSPRenew()

    assert "SurfaceIntersection" in vsp.ListAnalysis(), "Surface Intersection is not registered"

    names = vsp.GetAnalysisInputNames( "SurfaceIntersection" )
    for want in ( "SelectedSetIndex", "IGESFileFlag", "STEPFileFlag" ):
        assert want in names, "Surface Intersection has no %s input" % want


def testAWingThroughAPodGivesTheChainsItShould():
    """The chain network a pod and a wing make: 20 junctions, 44 chains between them.

    12 of those chains bound the wing on its own and 12 bound the pod; the rest are what the
    intersection cut the two of them into.  A border curve that was not split where the patch
    across from it breaks, or a pair that was never matched, moves these counts.
    """
    n0 = _errors()
    _pod_and_wing()

    chains = _intersect()

    assert len( chains ) == 44, "expected 44 chains, got %d" % len( chains )
    assert sum( len( c ) for c in chains ) == 9264, \
           "expected 9264 chain points, got %d" % sum( len( c ) for c in chains )
    assert _errors() == n0, "intersecting a pod and a wing raised an error"


def testEveryChainEndMeetsAnotherChainEnd():
    """A chain end with nothing to meet is a hole in the intersection topology.

    Ends come in pairs at worst -- two chains running into one junction -- so a junction
    holding one end means a chain was not split, not matched, or not merged with its partner.
    """
    _pod_and_wing()

    chains = _intersect()
    counts = _end_clusters( chains )

    assert counts, "no chains at all"
    lonely = [ n for n in counts if n < 2 ]
    assert not lonely, "%d chain ends meet nothing: %s" % ( len( lonely ), counts )

    odd = [ n for n in counts if n % 2 ]
    assert not odd, "%d junctions join an odd number of chain ends: %s" % ( len( odd ), counts )


def testTwoRunsOfOneModelAgreeExactly():
    """One model intersected twice gives one answer, to the last digit.

    The chains are built from work shared out over threads and from points ordered by where
    they were allocated, so this is the assertion that catches either of those leaking into
    the result.
    """
    _pod_and_wing()
    first = _intersect()

    _pod_and_wing()
    second = _intersect()

    assert first == second, "two runs of one model gave different chains"


def testTheThreadsDoNotChangeTheAnswer():
    """And the answer is the same one the single threaded run gives."""
    _pod_and_wing()
    threaded = _intersect( parallel = True )

    _pod_and_wing()
    serial = _intersect( parallel = False )

    vsp.SetParmVal( vsp.FindParm( vsp.FindContainer( "SurfaceIntersectSettings", 0 ),
                                  "ParallelMeshFlag", "Global" ), 1.0 )

    assert threaded == serial, "meshing on several threads gave a different answer"


def testAWingOnItsOwnClosesOnItsBorders():
    """With nothing to intersect, what is left is the patches a wing is carved into.

    Six junctions, each joining four chain ends, is a wing whose caps and trailing edge each
    arrive as one patch.  Cutting a feature down the middle adds patches and junctions.
    """
    vsp.VSPRenew()
    vsp.AddGeom( "WING", "" )
    vsp.Update()

    chains = _intersect()
    counts = _end_clusters( chains )

    assert len( chains ) == 12, "expected 12 border chains on a wing, got %d" % len( chains )
    assert sorted( counts ) == [ 4 ] * 6, "wing border junctions are %s" % sorted( counts )


def testMovingTheWingClearOfThePodLeavesOnlyTheBorders():
    """Nothing crosses, so no chain is cut and every junction is a plain patch corner."""
    wing = _pod_and_wing()
    vsp.SetParmVal( vsp.FindParm( wing, "Z_Rel_Location", "XForm" ), 5.0 )
    vsp.Update()

    chains = _intersect()
    counts = _end_clusters( chains )

    assert len( chains ) == 24, "expected the 24 border chains alone, got %d" % len( chains )
    assert max( counts ) == 4, "something still crosses: junctions are %s" % sorted( counts )


def _results():
    """The SurfaceIntersection results entry the last run wrote, as a plain dict."""
    rid = vsp.FindLatestResultsID( "SurfaceIntersection" )
    assert rid, "the intersector wrote no results entry"

    out = {}
    for name in vsp.GetAllDataNames( rid ):
        # Run through the analysis rather than the API call, the entry also carries the
        # duration, which is not a count and is not the same twice.
        if vsp.GetResultsType( rid, name ) == vsp.INT_DATA:
            out[name] = vsp.GetIntResults( rid, name )[0]

    return out


def testTheRunReportsWhatItFound():
    """The chain counts agree with the chains the entry carries."""
    _pod_and_wing()

    chains = _intersect()
    res = _results()

    assert res["Num_Chains"] == len( chains ), \
           "results say %d chains, but carry %d" % ( res["Num_Chains"], len( chains ) )
    assert res["Num_Curve_Pnts"] == sum( len( c ) for c in chains ), \
           "results say %d points, but carry %d" % \
           ( res["Num_Curve_Pnts"], sum( len( c ) for c in chains ) )
    assert res["Num_Surfs"] > 0, "the intersector reported no surfaces"


def testTheAnalysisReportsTheSameRun():
    """SurfaceIntersectionAnalysis hands back the entry the run wrote, rather than none."""
    _pod_and_wing()

    vsp.SetIntAnalysisInput( "SurfaceIntersection", "SelectedSetIndex", [ vsp.SET_ALL ] )
    rid = vsp.ExecAnalysis( "SurfaceIntersection" )

    assert rid, "SurfaceIntersection returned no results"
    assert rid == vsp.FindLatestResultsID( "SurfaceIntersection" ), \
           "the analysis returned something other than the run's own results"
