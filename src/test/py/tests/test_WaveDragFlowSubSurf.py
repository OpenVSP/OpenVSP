# A Wave Drag flow-through sub-surface list read from a file holds each sub-surface of the model
# once, so the next save writes it once.

import re

import openvsp as vsp

from testhelp import drop_errors


def write_with_flow_list( path ):
    vsp.VSPRenew()
    drop_errors()

    pod = vsp.AddGeom( "POD" )
    ss = vsp.AddSubSurf( pod, vsp.SS_RECTANGLE )
    vsp.Update()
    vsp.AddWaveDragFlowSubSurf( ss )
    vsp.WriteVSPFile( str( path ) )
    return ss


def count_flow_entries( path ):
    return len( re.findall( r"<FlowSS>", path.read_text() ) )


def testRepeatedAndMissingFlowSubSurfsAreReadOnce( tmp_path ):
    path = tmp_path / "flow.vsp3"
    ss = write_with_flow_list( path )
    assert count_flow_entries( path ) == 1

    # The list as a file left by repeated opens: the one entry 300 times, and one naming no
    # sub-surface of the model.
    text = path.read_text()
    entry = re.search( r"\s*<FlowSS>.*?</FlowSS>", text, re.S ).group( 0 )
    missing = entry.replace( ss, "NOSUCHSSID" )
    path.write_text( text.replace( entry, entry * 300 + missing, 1 ) )
    assert count_flow_entries( path ) == 301

    vsp.VSPRenew()
    vsp.ReadVSPFile( str( path ) )
    drop_errors()
    flow = vsp.GetWaveDragFlowSubSurfVec()
    assert len( flow ) == 1
    assert flow[0] in vsp.GetSubSurfIDVec( vsp.FindGeoms()[0] )

    again = tmp_path / "again.vsp3"
    vsp.WriteVSPFile( str( again ) )
    assert count_flow_entries( again ) == 1
