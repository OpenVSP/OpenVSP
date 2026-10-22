# Parameters handed to a surface evaluation must lie on the surface.  Code-Eli finds the patch for
# a u,w with an assert as its only guard, so in a release build a parameter just off the end
# reads patches[-1]: usually a wrong point, sometimes a crash.  Each case here builds the model
# that formed such a parameter.

import math
import subprocess
import sys

import openvsp as vsp


def testRibsAtTheEndsOfAWingWithNoEndCaps():
    """A rib at either end of a wing with no end caps takes its rotation from a finite difference
    across its station, which must stay on the surface.  Run in a subprocess, since an
    evaluation off the surface can take the process down."""
    script = """
import openvsp as vsp
print( "READY", flush = True )
vsp.VSPRenew()
wid = vsp.AddGeom( "WING", "" )
vsp.SetParmVal( vsp.FindParm( wid, "CapUMinOption", "EndCap" ), vsp.NO_END_CAP )
vsp.SetParmVal( vsp.FindParm( wid, "CapUMaxOption", "EndCap" ), vsp.NO_END_CAP )
vsp.Update()
vsp.AddFeaStruct( wid )
for loc in ( 0.0, 1.0 ):
    for edge in ( vsp.TE_NORMAL, vsp.LE_NORMAL ):
        rib = vsp.AddFeaPart( wid, 0, vsp.FEA_RIB )
        vsp.SetParmVal( vsp.FindParm( rib, "RelCenterLocation", "FeaPart" ), loc )
        vsp.SetParmVal( vsp.FindParm( rib, "PerpendicularEdgeType", "FeaRib" ), edge )
vsp.Update()
print( "DONE", flush = True )
"""
    done = subprocess.run( [ sys.executable, "-c", script ], capture_output = True, text = True,
                           timeout = 300 )

    lines = done.stdout.split( "\n" )
    assert "READY" in lines, "the subprocess did not load OpenVSP:\n" + done.stderr
    assert done.returncode == 0 and "DONE" in lines, \
        "updating ribs at the ends of a wing with no end caps failed (exit %d)" % done.returncode


def testAirfoilExportAtTheTipOfAWingWithNoEndCaps( tmp_path ):
    """Airfoil export steps along the wing in fractions of its span.  For 93 sections the sum
    for the tip station rounds past 1, and with no tip cap that is past the end of the surface.
    Run in a subprocess, since an evaluation off the surface can take the process down."""
    meta = str( tmp_path / "af" )
    script = """
import openvsp as vsp
print( "READY", flush = True )
vsp.VSPRenew()
wid = vsp.AddGeom( "WING", "" )
vsp.SetParmVal( vsp.FindParm( wid, "CapUMinOption", "EndCap" ), vsp.NO_END_CAP )
vsp.SetParmVal( vsp.FindParm( wid, "CapUMaxOption", "EndCap" ), vsp.NO_END_CAP )
for i in range( 92 ):
    vsp.InsertXSec( wid, 1, vsp.XS_FOUR_SERIES )
vsp.Update()
vsp.ExportFile( %r, vsp.SET_ALL, vsp.EXPORT_SELIG_AIRFOIL )
print( "DONE", flush = True )
""" % meta
    done = subprocess.run( [ sys.executable, "-c", script ], capture_output = True, text = True,
                           timeout = 300 )

    lines = done.stdout.split( "\n" )
    assert "READY" in lines, "the subprocess did not load OpenVSP:\n" + done.stderr
    assert done.returncode == 0 and "DONE" in lines, \
        "exporting the airfoils of a 93 section wing failed (exit %d)" % done.returncode

    named = open( meta ).read().count( "Airfoil File Name" )
    written = len( list( tmp_path.glob( "*.dat" ) ) )
    assert named > 0 and written == named, "%d airfoils listed, %d written" % ( named, written )


def testCentroidOfAPolygonWithNoArea():
    """A sub-surface hanging off the edge of its surface can trim to a polygon that folds back on
    itself.  Its centroid orients the sub-surface's FEA elements, so it must be a point, not
    NaN."""
    folded = [ vsp.vec2d( 1, 4 ), vsp.vec2d( 1, 2 ), vsp.vec2d( -1, 2 ), vsp.vec2d( 1, 2 ),
               vsp.vec2d( 1, 0 ) ]

    assert vsp.poly_area( folded ) == 0.0

    c = vsp.poly_centroid( folded )
    assert math.isfinite( c.x() ) and math.isfinite( c.y() )
    assert abs( c.x() - 0.6 ) < 1e-12 and abs( c.y() - 2.0 ) < 1e-12

    # Scaled and moved, the fold's area rounds to a trace rather than to zero; the centroid is still the mean.
    for scale, shift in ( ( 0.37, 3.1 ), ( 1.9, -7.3 ), ( 0.013, 0.41 ) ):
        moved = [ vsp.vec2d( scale * p.x() + shift, scale * p.y() + shift ) for p in folded ]
        c = vsp.poly_centroid( moved )
        assert abs( c.x() - ( 0.6 * scale + shift ) ) < 1e-9 and abs( c.y() - ( 2.0 * scale + shift ) ) < 1e-9

    square = [ vsp.vec2d( 0, 0 ), vsp.vec2d( 2, 0 ), vsp.vec2d( 2, 2 ), vsp.vec2d( 0, 2 ) ]
    c = vsp.poly_centroid( square )
    assert abs( c.x() - 1.0 ) < 1e-12 and abs( c.y() - 1.0 ) < 1e-12
