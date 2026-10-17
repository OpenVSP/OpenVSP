"""Screenshots of OpenVSP's GUI for the help pages.

Each screenshot is a crop of one screen or tab, taken from a model built for it with the openvsp module.  The model
is saved, `vsp -snap N model.vsp3` writes a PNG of screen N (an enum of GUI_VSP_SCREEN) or `vsp -snapgeom N
model.vsp3` one of each tab of Geom screen N (GUI_GEOM_SCREEN), and the crop is cut from one of those.  The vsp
program must have these options; name it with the environment variable VSP_SNAP_EXE.  The dump shows real windows,
so it needs a logged-in desktop session.

A crop box is given in screen units, (left, top, right, bottom) of the window's contents; the dump is taken at
scale 2 and the box is scaled to it.

    import snap
    def model():
        ...build with openvsp...
    snap.shots( model, "MS_WING_GEOM_SCREEN", { "Wing_sect_planform": ( "Wing_Sect", ( 4, 249, 456, 425 ) ) } )

Setting the environment variable SNAP_CACHE to a folder keeps each dump there, under the name given to shots(), and
a later run with the same name crops from the kept dump without running vsp again; delete the dump's folder to retake
it after changing the model.

The screen is named by its enum; each crop names the PNG it is cut from, which is the screen's title, or a Geom
screen's type and tab, as `Wing_Sect`.
"""
import os
import re
import shutil
import subprocess
import sys
import tempfile

import openvsp as vsp
from PIL import Image

SCALE = 2
HERE = os.path.dirname( os.path.abspath( __file__ ) )
APIDEFINES = os.path.join( HERE, "..", "..", "..", "geom_api", "APIDefines.h" )
IMAGES = os.path.normpath( os.path.join( os.path.dirname( os.path.abspath( __file__ ) ), "..", "..", "images" ) )


def enum_value( name ):
    """Value of a member of the enum that holds it in APIDefines.h; members are numbered from 0 in order."""
    text = open( APIDEFINES ).read()
    for body in re.findall( r"enum\s+\w+\s*\{(.*?)\}", text, re.S ):
        body = re.sub( r"/\*.*?\*/|//[^\n]*", "", body, flags = re.S )
        names = re.findall( r"^\s*\{?\s*(\w+)\s*(?:=\s*\w+\s*)?,", body, re.M )
        if name in names:
            return names.index( name )
    raise KeyError( name )


def dump( model, screen, keep = None ):
    """Build the model (a function that builds it from an empty model), dump the screen named by its enum (a member
    of VSP_SCREEN_TYPE or GUI_GEOM_SCREEN) and return the folder of the PNGs.  keep names a folder to dump into;
    otherwise a temporary one is used."""
    exe = os.environ.get( "VSP_SNAP_EXE" )
    if not exe:
        sys.exit( "set VSP_SNAP_EXE to a vsp program that has the -snap option" )

    folder = keep or tempfile.mkdtemp()
    os.makedirs( folder, exist_ok = True )
    home = tempfile.mkdtemp()

    vsp.VSPRenew()
    model()
    vsp.Update()
    vsp.WriteVSPFile( os.path.join( folder, "model.vsp3" ) )
    vsp.VSPRenew()

    # The model is named relative to the folder, so the path the main window shows is the same every run
    env = dict( os.environ, HOME = home )
    if screen.startswith( "VSP_" ):
        flag = "-snap"
    else:
        flag = "-snapgeom"
    r = subprocess.run( [ exe, flag, str( enum_value( screen ) ), "-snapscale", str( SCALE ), "model.vsp3" ],
                        cwd = folder, env = env, capture_output = True, text = True )
    shutil.rmtree( home, ignore_errors = True )
    if r.returncode != 0:
        sys.exit( "vsp -snap failed with code %d:\n%s" % ( r.returncode, r.stdout[ -2000: ] ) )
    return folder


def crop( folder, screen, box, name ):
    """Cut box (1x screen units) from the dump of screen and write images/<name>.png."""
    img = Image.open( os.path.join( folder, screen + ".png" ) )
    b = tuple( int( round( v * SCALE ) ) for v in box )
    assert b[ 2 ] <= img.width and b[ 3 ] <= img.height, ( screen, b, img.size )
    out = os.path.join( IMAGES, name + ".png" )
    img.crop( b ).save( out, optimize = True )
    print( "wrote", out, b[ 2 ] - b[ 0 ], "x", b[ 3 ] - b[ 1 ] )


def shots( model, screen, crops, name = None ):
    """Dump one screen of the model and cut every crop, given as { image name: ( png, box ) }.  With SNAP_CACHE set,
    the dump is kept in SNAP_CACHE/<name> (name defaults to the first image's) and reused while it is there."""
    cache = os.environ.get( "SNAP_CACHE" )
    if cache:
        folder = os.path.join( cache, name or list( crops )[ 0 ] )
        if not os.path.isdir( folder ) or not [ f for f in os.listdir( folder ) if f.endswith( ".png" ) ]:
            dump( model, screen, folder )
    else:
        folder = dump( model, screen )
    for image, ( png, box ) in crops.items():
        crop( folder, png, box, image )
    if not cache:
        shutil.rmtree( folder, ignore_errors = True )
