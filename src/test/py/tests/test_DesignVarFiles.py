"""Reading design variable files that are missing or short."""

import os

import openvsp as vsp


def a_pod_with_two_vars():
    vsp.VSPRenew()
    pod = vsp.AddGeom( "POD" )
    vsp.Update()
    length = vsp.FindParm( pod, "Length", "Design" )
    fine = vsp.FindParm( pod, "FineRatio", "Design" )
    vsp.AddDesignVar( length, vsp.XDDM_VAR )
    vsp.AddDesignVar( fine, vsp.XDDM_VAR )
    return length, fine


def test_a_missing_des_file_leaves_the_variables( tmp_path ):
    a_pod_with_two_vars()
    vsp.ReadApplyDESFile( str( tmp_path / "missing.des" ) )
    assert vsp.GetNumDesignVars() == 2


def test_a_missing_xddm_file_leaves_the_variables( tmp_path ):
    a_pod_with_two_vars()
    vsp.ReadApplyXDDMFile( str( tmp_path / "missing.xddm" ) )
    assert vsp.GetNumDesignVars() == 2


def test_a_des_file_shorter_than_its_count_reads_what_it_has( tmp_path ):
    length, fine = a_pod_with_two_vars()
    path = str( tmp_path / "short.des" )
    vsp.WriteDESFile( path )
    lines = open( path ).read().splitlines()
    # The count, then one line per variable; keep the count and the first variable only.
    assert int( lines[0] ) == 2
    first = lines[1]
    name = first.split( ":" )[0]
    open( path, "w" ).write( lines[0] + "\n" + first.rsplit( " ", 1 )[0] + " 7.5\n" )

    vsp.DeleteAllDesignVars()
    vsp.ReadApplyDESFile( path )
    assert vsp.GetNumDesignVars() == 1
    assert vsp.GetDesignVar( 0 ) == name
    assert vsp.GetParmVal( name ) == 7.5
