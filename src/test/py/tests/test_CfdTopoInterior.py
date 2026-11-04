# Which triangles of the initial CFD mesh lie inside other components.
#
# CFD Mesh can find them from the topology of the intersection curves, casting rays only to seed
# the walk and where the curves cannot tell, or by casting rays from every triangle.  On models
# whose intersections the rays read cleanly the two must give the same mesh, triangle for
# triangle.

import os

import openvsp as vsp

from testhelp import drop_errors

HERE = os.path.dirname( os.path.abspath( __file__ ) )
NEGATIVE_MODEL = os.path.join( HERE, "CfdNegativePodWing.vsp3" )
DISK_MODEL = os.path.join( HERE, "CfdActuatorDiskPodWing.vsp3" )


def _read_stl( path ):
    """The triangles of an ASCII STL file, each a sorted tuple of its vertices."""
    tris = []
    verts = []
    for line in open( path ):
        words = line.split()
        if words and words[0] == "vertex":
            verts.append( tuple( words[1:4] ) )
            if len( verts ) == 3:
                tris.append( tuple( sorted( verts ) ) )
                verts = []
    return sorted( tris )


def _mesh_stl( base, topo ):
    """The triangles of the CFD mesh of every Geom, made with the topology classification on or off."""
    vsp.SetCFDMeshVal( vsp.CFD_TOPO_INTERIOR_FLAG, topo )
    vsp.SetComputationFileName( vsp.CFD_STL_TYPE, str( base ) + ".stl" )
    vsp.ComputeCFDMesh( vsp.SET_ALL, vsp.SET_NONE, vsp.CFD_STL_TYPE )
    return _read_stl( str( base ) + ".stl" )


def _assert_same_both_ways( base ):
    on = _mesh_stl( str( base ) + "_on", 1 )
    off = _mesh_stl( str( base ) + "_off", 0 )
    assert len( on ) > 0
    assert on == off, "%d triangles by topology, %d by rays, %d differ" % \
        ( len( on ), len( off ), len( set( on ) ^ set( off ) ) )


def _wing_body_pod():
    """A wing through a fuselage, with a pod on each side lying through the wing."""
    fuse = vsp.AddGeom( "FUSELAGE" )
    vsp.SetParmVal( fuse, "Length", "Design", 10.0 )

    wing = vsp.AddGeom( "WING" )
    vsp.SetParmVal( wing, "X_Rel_Location", "XForm", 3.0 )
    vsp.SetParmVal( wing, "Z_Rel_Location", "XForm", -0.2 )
    vsp.SetParmVal( wing, "TotalSpan", "WingGeom", 12.0 )

    pod = vsp.AddGeom( "POD" )
    vsp.SetParmVal( pod, "Length", "Design", 3.0 )
    vsp.SetParmVal( pod, "FineRatio", "Design", 6.0 )
    vsp.SetParmVal( pod, "X_Rel_Location", "XForm", 2.8 )
    vsp.SetParmVal( pod, "Y_Rel_Location", "XForm", 3.0 )
    vsp.SetParmVal( pod, "Z_Rel_Location", "XForm", -0.3 )
    vsp.SetParmVal( pod, "Sym_Planar_Flag", "Sym", vsp.SYM_XZ )
    vsp.Update()

    vsp.SetCFDMeshVal( vsp.CFD_MAX_EDGE_LEN, 0.5 )
    vsp.SetCFDMeshVal( vsp.CFD_MIN_EDGE_LEN, 0.05 )


def test_setting( tmp_path, monkeypatch ):
    """On by default, and kept in the model file."""
    monkeypatch.chdir( tmp_path )
    vsp.VSPRenew()
    assert vsp.GetCFDMeshVal( vsp.CFD_TOPO_INTERIOR_FLAG ) == 1.0

    vsp.SetCFDMeshVal( vsp.CFD_TOPO_INTERIOR_FLAG, 0 )
    assert vsp.GetCFDMeshVal( vsp.CFD_TOPO_INTERIOR_FLAG ) == 0.0
    vsp.WriteVSPFile( str( tmp_path / "off.vsp3" ) )

    vsp.VSPRenew()
    assert vsp.GetCFDMeshVal( vsp.CFD_TOPO_INTERIOR_FLAG ) == 1.0
    vsp.ReadVSPFile( str( tmp_path / "off.vsp3" ) )
    assert vsp.GetCFDMeshVal( vsp.CFD_TOPO_INTERIOR_FLAG ) == 0.0

    vsp.VSPRenew()
    assert "TopoInterior" in vsp.GetAnalysisInputNames( "CfdMeshAnalysis" )


def test_the_surfaces_classified_are_reported( tmp_path, monkeypatch ):
    """The results count the surfaces the chains' topology classified: some with it on, none off."""
    monkeypatch.chdir( tmp_path )
    vsp.VSPRenew()
    _wing_body_pod()
    counts = {}
    for topo in ( 1, 0 ):
        _mesh_stl( tmp_path / ( "m%d" % topo ), topo )
        rid = vsp.FindLatestResultsID( "CFDMesh" )
        counts[topo] = vsp.GetIntResults( rid, "Num_Topo_Surfs" )[0]
    assert counts[1] > 0
    assert counts[0] == 0


def test_a_wing_through_a_body_with_a_pod_on_it( tmp_path, monkeypatch ):
    monkeypatch.chdir( tmp_path )
    vsp.VSPRenew()
    _wing_body_pod()
    _assert_same_both_ways( tmp_path / "wb" )


def test_a_half_mesh_in_a_far_field( tmp_path, monkeypatch ):
    """The symmetry plane and the far field are left to the rays; the rest is not."""
    monkeypatch.chdir( tmp_path )
    vsp.VSPRenew()
    _wing_body_pod()
    vsp.SetCFDMeshVal( vsp.CFD_HALF_MESH_FLAG, 1 )
    vsp.SetCFDMeshVal( vsp.CFD_FAR_FIELD_FLAG, 1 )
    vsp.SetCFDMeshVal( vsp.CFD_FAR_MAX_EDGE_LEN, 4.0 )
    _assert_same_both_ways( tmp_path / "half" )


def test_rotors_in_a_row( tmp_path, monkeypatch ):
    """Three three-bladed rotors one behind another beside a fuselage, a blade or two through it."""
    monkeypatch.chdir( tmp_path )
    vsp.VSPRenew()

    fuse = vsp.AddGeom( "FUSELAGE" )
    vsp.SetParmVal( fuse, "Length", "Design", 10.0 )

    for k in range( 3 ):
        prop = vsp.AddGeom( "PROP" )
        vsp.SetParmVal( prop, "Diameter", "Design", 4.0 )
        vsp.SetParmVal( prop, "NumBlade", "Design", 3 )
        vsp.SetParmVal( prop, "X_Rel_Location", "XForm", 2.0 + 2.5 * k )
        vsp.SetParmVal( prop, "Y_Rel_Location", "XForm", 1.5 )
        vsp.SetParmVal( prop, "Z_Rel_Location", "XForm", 1.0 )
        vsp.SetParmVal( prop, "Y_Rel_Rotation", "XForm", 90.0 )
        vsp.SetParmVal( prop, "Rotate", "Design", 17.0 * k )
        vsp.SetParmVal( prop, "PropMode", "Design", vsp.PROP_BLADES )
    vsp.Update()

    vsp.SetCFDMeshVal( vsp.CFD_MAX_EDGE_LEN, 0.5 )
    vsp.SetCFDMeshVal( vsp.CFD_MIN_EDGE_LEN, 0.02 )
    _assert_same_both_ways( tmp_path / "rotors" )


def test_a_pod_and_wing_cut_by_negative_volumes( tmp_path, monkeypatch ):
    """A pod and wing with a hole through the pod and a divot in each wing, all negative volumes."""
    monkeypatch.chdir( tmp_path )
    vsp.VSPRenew()
    vsp.ReadVSPFile( NEGATIVE_MODEL )
    drop_errors()
    vsp.Update()
    _assert_same_both_ways( tmp_path / "negative" )


def test_actuator_disks_on_a_pod_and_wing( tmp_path, monkeypatch ):
    """Propellers in disk mode, one inside a duct, on a pod and wing: surfaces with no inside."""
    monkeypatch.chdir( tmp_path )
    vsp.VSPRenew()
    vsp.ReadVSPFile( DISK_MODEL )
    drop_errors()
    vsp.Update()
    _assert_same_both_ways( tmp_path / "disks" )
