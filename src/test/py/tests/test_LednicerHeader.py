# A file that is neither Selig nor Lednicer is refused, promptly.
#
# FileAirfoil::ReadFile tries Selig, then Lednicer.  The Lednicer reader takes its point counts
# from the second line, which must hold two numbers.  Run in a child interpreter so a hang fails
# the test instead of stalling the suite.

import subprocess
import sys

SCRIPT = """
import sys
import openvsp as vsp
wing = vsp.AddGeom( "WING" )
xsurf = vsp.GetXSecSurf( wing, 0 )
vsp.ChangeXSecShape( xsurf, 0, vsp.XS_FILE_AIRFOIL )
e = vsp.ErrorMgrSingleton.getInstance()
n0 = e.GetNumTotalErrors()
vsp.ReadFileAirfoil( vsp.GetXSec( xsurf, 0 ), sys.argv[1] )
print( "ERRORS", e.GetNumTotalErrors() - n0 )
"""


def testAOneNumberHeaderIsNotLednicer( tmp_path ):
    af = tmp_path / "bad.dat"
    af.write_text( "Not an airfoil\n  17.\n  0.0  0.0\n  0.5  0.1\n  1.0  0.0\n" )
    run = subprocess.run( [ sys.executable, "-c", SCRIPT, str( af ) ], capture_output = True, text = True,
                          timeout = 60 )
    assert run.returncode == 0, run.stderr[ -2000: ]
    assert "ERRORS 1" in run.stdout, "a one-number header was read as an airfoil: %s" % run.stdout
