# What the CFD mesher says about the mesh it just built.
#
# The watertight verdict used to be a line of console text and nothing else, so nothing could
# assert on it: a run that produced no triangles at all printed "Is Water Tight" and every
# file-size check in the suite passed.  Each run now writes a CFDMesh Results entry carrying
# the triangle count, the number of border edges (one triangle on them), the number of
# over-connected edges (more than two) and the verdict those two imply.
#
# That is the mesher's own account of its output, which is what makes the closure work
# testable at all.  A border edge where the model has no boundary means two patches did not
# meet; an over-connected edge means a surface was laid against itself or a face survived
# that should not have.
#
# Note that GenerateMesh hides every Geom on its way through, so a run left on the default
# (shown) set finds nothing the second time.  Every run here asks for SET_ALL.

import openvsp as vsp


def _errors():
    """How many errors are on the API stack.

    The stack is never emptied, so a count is only meaningful against an earlier count from
    the same process -- an absolute zero would be asserting about every test that ran before.
    """
    return vsp.ErrorMgrSingleton.getInstance().GetNumTotalErrors()


def _mesh( geom_set = vsp.SET_ALL ):
    """Mesh the model and hand back the CFDMesh results as a plain dict."""
    vsp.ComputeCFDMesh( geom_set, vsp.SET_NONE, vsp.CFD_STL_TYPE )

    rid = vsp.FindLatestResultsID( "CFDMesh" )
    assert rid, "the mesher wrote no CFDMesh results entry"

    out = {}
    for name in vsp.GetAllDataNames( rid ):
        # Run through the analysis rather than the API call, the entry also carries the
        # duration, which is not a count and is not the same twice.
        if vsp.GetResultsType( rid, name ) == vsp.INT_DATA:
            out[name] = vsp.GetIntResults( rid, name )[0]

    return out


def _coarse():
    """Edge lengths that mesh a ten unit model in a second or so."""
    vsp.SetCFDMeshVal( vsp.CFD_MAX_EDGE_LEN, 0.8 )
    vsp.SetCFDMeshVal( vsp.CFD_MIN_EDGE_LEN, 0.2 )


def _pod():
    vsp.VSPRenew()
    vsp.AddGeom( "POD", "" )
    vsp.Update()
    _coarse()


def _pod_and_wing():
    """A wing run through the middle of a pod, so the two really do intersect."""
    vsp.VSPRenew()
    vsp.AddGeom( "POD", "" )
    wing = vsp.AddGeom( "WING", "" )
    vsp.SetParmVal( vsp.FindParm( wing, "X_Rel_Location", "XForm" ), 2.0 )
    vsp.SetParmVal( vsp.FindParm( wing, "TotalSpan", "WingGeom" ), 8.0 )
    vsp.Update()
    _coarse()


def testThePodMeshCloses():
    """One closed body on its own: every edge has two triangles, so nothing is open."""
    n0 = _errors()
    _pod()

    res = _mesh()

    assert res["Num_Tris"] > 0, "the mesher produced no triangles"
    assert res["Num_Border_Edges"] == 0, \
           "%d edges have one triangle on them" % res["Num_Border_Edges"]
    assert res["Num_Over_Connected_Edges"] == 0, \
           "%d edges have more than two triangles on them" % res["Num_Over_Connected_Edges"]
    assert res["Water_Tight"] == 1, "a pod on its own did not close"
    assert _errors() == n0, "meshing a pod raised an error"


def testAWingThroughAPodCloses():
    """Two bodies cut against each other, which is where closure is actually at risk.

    The trimmed halves have to meet along the intersection curve to the last digit, on
    surfaces meshed independently and on different threads.
    """
    n0 = _errors()
    _pod_and_wing()

    res = _mesh()

    assert res["Num_Tris"] > 0, "the mesher produced no triangles"
    assert res["Water_Tight"] == 1, \
           "a wing through a pod left %d border and %d over-connected edges" % \
           ( res["Num_Border_Edges"], res["Num_Over_Connected_Edges"] )
    assert _errors() == n0, "meshing a wing through a pod raised an error"


def testTheVerdictFollowsTheEdgeCounts():
    """Water_Tight is not an independent opinion: it is exactly "neither count is nonzero"."""
    _pod_and_wing()

    res = _mesh()

    closed = res["Num_Border_Edges"] == 0 and res["Num_Over_Connected_Edges"] == 0
    assert res["Water_Tight"] == int( closed ), \
           "the verdict and the edge counts disagree: %s" % res


def testTwoRunsOfOneModelAgree():
    """One model meshed twice gives one answer.

    The surfaces are meshed on several threads, so this is the assertion that catches the
    thread that finished first changing the result.
    """
    _pod_and_wing()
    first = _mesh()

    _pod_and_wing()
    second = _mesh()

    assert first == second, "two runs of one model disagree: %s vs %s" % ( first, second )


def testTheThreadsDoNotChangeTheMesh():
    """And the answer is the one the single threaded run gives."""
    _pod_and_wing()
    vsp.SetCFDMeshVal( vsp.CFD_PARALLEL_MESH_FLAG, 1 )
    threaded = _mesh()

    _pod_and_wing()
    vsp.SetCFDMeshVal( vsp.CFD_PARALLEL_MESH_FLAG, 0 )
    serial = _mesh()

    vsp.SetCFDMeshVal( vsp.CFD_PARALLEL_MESH_FLAG, 1 )

    assert threaded == serial, \
           "meshing on several threads gave a different mesh: %s vs %s" % ( threaded, serial )


def testAnEmptySetProducesNoMesh():
    """Asked for nothing, the mesher says it built nothing rather than reporting success.

    This is the shape of the trap the default set falls into: a run hides every Geom once it
    has taken their surfaces, so a second run on the shown set has nothing to work with.
    """
    _pod()

    res = _mesh( vsp.SET_NONE )

    assert res["Num_Tris"] == 0, "an empty set produced %d triangles" % res["Num_Tris"]
    assert res["Water_Tight"] == 1, "an empty mesh has no edges to be open"


def testTheAnalysisReportsTheSameRun():
    """CfdMeshAnalysis hands back the entry the run wrote, rather than none at all."""
    _pod()

    vsp.SetIntAnalysisInput( "CfdMeshAnalysis", "SelectedSetIndex", [ vsp.SET_ALL ] )
    rid = vsp.ExecAnalysis( "CfdMeshAnalysis" )

    assert rid, "CfdMeshAnalysis returned no results"
    assert rid == vsp.FindLatestResultsID( "CFDMesh" ), \
           "CfdMeshAnalysis returned something other than the run's own results"
    assert vsp.GetIntResults( rid, "Num_Tris" )[0] > 0, "the analysis produced no triangles"


#==== The structures mesher ====#
#
# A structure's mesh is open by design -- a part's edge stops where the part does -- and every
# joint between parts puts more than two elements on one edge, so the FEAMesh entry carries no
# watertight verdict.  It counts the free edges instead: shell element edges with no other shell
# element on them, which is where a part that should meet another and does not shows up.


def _fea_pod( half = 0 ):
    """A pod with a bulkhead across it, meshed coarsely.  Hands back the result ID the analysis
    returned and the FEAMesh counts as a plain dict."""
    vsp.VSPRenew()
    pod = vsp.AddGeom( "POD", "" )
    ind = vsp.AddFeaStruct( pod )
    vsp.AddFeaPart( pod, ind, vsp.FEA_SLICE )
    vsp.SetFeaMeshVal( pod, ind, vsp.CFD_MAX_EDGE_LEN, 1.0 )
    vsp.SetFeaMeshVal( pod, ind, vsp.CFD_MIN_EDGE_LEN, 0.2 )
    vsp.SetFeaMeshStructIndex( ind )
    vsp.Update()

    an = "FeaMeshAnalysis"
    vsp.SetAnalysisInputDefaults( an )
    vsp.SetIntAnalysisInput( an, "HalfMeshFlag", [ half ] )
    rid = vsp.ExecAnalysis( an )

    out = {}
    for name in vsp.GetAllDataNames( rid ):
        if vsp.GetResultsType( rid, name ) == vsp.INT_DATA:
            out[name] = vsp.GetIntResults( rid, name )[0]

    return rid, out


def testAPodWithABulkheadHasNoFreeEdges():
    """The skin closes on itself and the bulkhead meets it all the way round."""
    n0 = _errors()

    rid, res = _fea_pod()

    assert res["Num_Els"] > 0, "the structure mesher produced no elements"
    assert res["Num_Free_Edges"] == 0, "%d element edges are free" % res["Num_Free_Edges"]
    assert _errors() == n0, "meshing a pod's structure raised an error"


def testAHalfMeshIsOpenAlongTheSymmetryPlane():
    """Cut in half, the skin and the bulkhead both stop at the symmetry plane."""
    rid, res = _fea_pod( half = 1 )

    assert res["Num_Els"] > 0, "the structure mesher produced no elements"
    assert res["Num_Free_Edges"] > 0, "a half mesh reported no free edges"


def testFeaMeshAnalysisHandsBackTheRunsEntry():
    rid, res = _fea_pod()

    assert rid == vsp.FindLatestResultsID( "FEAMesh" ), \
           "FeaMeshAnalysis returned something other than the run's FEAMesh entry"
