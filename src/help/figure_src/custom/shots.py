"""Screenshots of the Cone custom component for Custom.md.  See ../screenshots/snap.py.

OpenVSP reads custom scripts from a CustomScripts folder in the working directory when it starts.  The Cone example
(examples/CustomScripts/Cone.vsppart) is put in a CustomScripts folder beside the dump, before openvsp is imported here
and before vsp is started there (vsp runs in the dump's folder)."""
import os
import shutil
import sys
import tempfile

HERE = os.path.dirname( os.path.abspath( __file__ ) )
CONE = os.path.join( HERE, "..", "..", "..", "..", "examples", "CustomScripts", "Cone.vsppart" )
NAME = "custom_cone"

cache = os.environ.get( "SNAP_CACHE" )
if cache:
    folder = os.path.join( cache, NAME )
else:
    folder = tempfile.mkdtemp()
scripts = os.path.join( folder, "CustomScripts" )
os.makedirs( scripts, exist_ok = True )
shutil.copy( CONE, scripts )
os.chdir( folder )

sys.path.insert( 0, os.path.join( HERE, "..", "screenshots" ) )
import snap
import openvsp as vsp


def model():
    vsp.AddGeom( "Cone" )
    vsp.Update()


SCREEN = "Custom_Design"
CROPS = { "Custom_gui_cone": ( SCREEN, ( 2, 0, 398, 118 ) ) }

if not [ f for f in os.listdir( folder ) if f.endswith( ".png" ) ]:
    snap.dump( model, "CUSTOM_GEOM_SCREEN", folder )
for image, ( png, box ) in CROPS.items():
    snap.crop( folder, png, box, image )
if not cache:
    shutil.rmtree( folder, ignore_errors = True )
