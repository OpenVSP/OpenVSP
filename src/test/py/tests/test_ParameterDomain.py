# Parameters handed to a surface evaluation must lie on the surface.  Code-Eli finds the patch for
# a u,w with an assert as its only guard, so in a release build a parameter just off the end
# reads patches[-1]: usually a wrong point, sometimes a crash.  Each case here builds the model
# that formed such a parameter.

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
