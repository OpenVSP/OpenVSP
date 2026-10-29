# The DATCOM conversion script, examples/scripts/python/datcom2vsp.py, run on its example input,
# examples/data/manual_fig7.mdc, the protuberance example of the Missile DATCOM manual's Figure 7: a body, four
# protuberance sets and a fin set.

import importlib.util
import os
import sys
import tempfile

import openvsp as vsp
import pytest

HERE = os.path.dirname( os.path.abspath( __file__ ) )
EXAMPLES = os.path.join( HERE, "..", "..", "..", "..", "examples" )


def script():
    # No bytecode is left beside the script: the scripts folder is installed as it is.
    spec = importlib.util.spec_from_file_location( "datcom2vsp",
                                                   os.path.join( EXAMPLES, "scripts", "python", "datcom2vsp.py" ) )
    module = importlib.util.module_from_spec( spec )
    keep = sys.dont_write_bytecode
    sys.dont_write_bytecode = True
    try:
        spec.loader.exec_module( module )
    finally:
        sys.dont_write_bytecode = keep
    return module


def build():
    d2v = script()
    cases = d2v.read_cases( os.path.join( EXAMPLES, "data", "manual_fig7.mdc" ) )
    assert len( cases ) >= 1
    vsp.VSPRenew()
    d2v.Builder( vsp, cases[0] ).build()
    return d2v


def testExampleBuildsItsComponentsOnTheBody():
    build()
    names = sorted( vsp.GetGeomTypeName( g ) for g in vsp.FindGeoms() )
    assert names == [ "MDC_BOD", "MDC_FIN", "MDC_PROTUB", "MDC_PROTUB", "MDC_PROTUB", "MDC_PROTUB" ]

    body = [ g for g in vsp.FindGeoms() if vsp.GetGeomTypeName( g ) == "MDC_BOD" ][0]
    for g in vsp.FindGeoms():
        if g != body:
            assert vsp.GetGeomParent( g ) == body

    # The body is 78 inches long and 12 across; the fins are at XLE 64.
    lo = vsp.GetGeomBBoxMin( body, 0, True )
    hi = vsp.GetGeomBBoxMax( body, 0, True )
    assert hi.x() - lo.x() == pytest.approx( 78.0, abs=1e-6 )
    assert hi.y() - lo.y() == pytest.approx( 12.0, abs=1e-6 )
    fin = [ g for g in vsp.FindGeoms() if vsp.GetGeomTypeName( g ) == "MDC_FIN" ][0]
    assert vsp.GetParmVal( fin, "X_Rel_Location", "XForm" ) == pytest.approx( 64.0 )


def testComparingWithItsOwnStlFindsNoDifference():
    d2v = build()
    path = os.path.join( tempfile.mkdtemp(), "self.stl" )
    vsp.ExportFile( path, vsp.SET_ALL, vsp.EXPORT_STL )
    vsp.DeleteGeomVec( [ g for g in vsp.FindGeoms() if vsp.GetGeomTypeName( g ) == "Mesh" ] )
    far = d2v.compare_to_stl( vsp, path, 1.0 )
    assert far is not None
    assert far < 1.0e-3
