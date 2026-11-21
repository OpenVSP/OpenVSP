"""Pictures of meshes as CFD Mesh draws them in OpenVSP, for CFDMesh.md.

    <build>/venv/bin/python meshshots.py            # every picture (see SHOTS at the end)
    <build>/venv/bin/python meshshots.py wake       # or only some

Each picture is taken in a process of its own, through the openvsp module's GUI facade: the model is built and meshed
first, then the GUI is started, the CFD Mesh screen opened (it loads the mesh's draw objects), the display set, and
the main window grabbed at high resolution with ScreenGrab.  The grab is cropped and scaled to the page's width
here.
"""

import os
import sys
import json
import subprocess

HERE = os.path.dirname( os.path.abspath( __file__ ) )
OUT = os.path.join( HERE, "..", "..", "images" )
WIDTH = 1400     # pixels: 7 in at 200 dpi, as the other figures
HMAX = 920       # pixels: the tallest a picture is let grow, 4.6 in
GRAB = ( 4800, 3600 )
STARTED = []     # the GUI is started once in each process, after the model is meshed


def grab( name, view, crop = None, color = "PLAIN", size = GRAB, svg = False, keep = False, center = None, zoom = 1.0,
          rot = None, preview = False, zoom_abs = None, turn = False ):
    """In this process: start the GUI on the meshed model, show the mesh and save the grab as name.png in OUT.
    crop is ( x0, y0, x1, y1 ) as fractions of the autocropped grab, from its top left."""
    import openvsp as vsp
    from PIL import Image

    if not STARTED:
        vsp.StartGUI()
        vsp.ShowScreen( vsp.VSP_CFD_MESH_SCREEN )
        STARTED.append( True )
    s = vsp.FindContainer( "CFDMeshSettings", 0 )
    # PLAIN draws the mesh uncolored, so its edges show; TAG and REASON color it as the Display tab's Color By.
    if color == "PLAIN":
        vsp.SetParmVal( vsp.FindParm( s, "ColorTagsFlag", "DrawMesh" ), 0.0 )
    else:
        vsp.SetParmVal( vsp.FindParm( s, "ColorTagsFlag", "DrawMesh" ), 1.0 )
        vsp.SetParmVal( vsp.FindParm( s, "ColorReasonFlag", "DrawMesh" ), getattr( vsp, color ) )
    vsp.SetParmVal( vsp.FindParm( s, "DrawSourceWake", "DrawMesh" ), float( preview ) )
    vsp.SetParmVal( vsp.FindParm( s, "DrawFarFieldPreview", "DrawMesh" ), 0.0 )
    vsp.SetViewAxis( False )
    vsp.SetShowBorders( False )
    # Set twice: the first view set after the GUI starts is sometimes lost.
    for k in range( 2 ):
        vsp.SetAllViews( getattr( vsp, view ) )
        vsp.FitAllViews()
        vsp.Update()
        vsp.UpdateGUI()
    print( "fit zoom", name, vsp.GetParmVal( vsp.FindParm( vsp.GetVehicleID(), "Zoom", "AdjustView" ) ) )
    # Zoom in on center through the Vehicle's view Parms (the Adjust View screen's): the fitted zoom divided by zoom,
    # about center, turned to rot ( X, Y, Z rotation, degrees ) if given.
    if center is not None:
        v = vsp.GetVehicleID()
        z = vsp.GetParmVal( vsp.FindParm( v, "Zoom", "AdjustView" ) )
        if zoom_abs is not None:
            z = zoom_abs
        for n, val in ( ( "CORX", center[ 0 ] ), ( "CORY", center[ 1 ] ), ( "CORZ", center[ 2 ] ), ( "PanX", 0.0 ),
                        ( "PanY", 0.0 ), ( "Zoom", z / zoom ) ):
            vsp.SetParmVal( vsp.FindParm( v, n, "AdjustView" ), val )
        if rot is not None:
            for n, val in zip( ( "RotationX", "RotationY", "RotationZ" ), rot ):
                vsp.SetParmVal( vsp.FindParm( v, n, "AdjustView" ), val )
        vsp.Update()
        vsp.UpdateGUI()
    if svg:
        out = os.path.join( OUT, name + ".svg" )
        vsp.VectorScreenGrab( out, vsp.VEC_SVG, True, True, 7.0, 4.6, vsp.LEN_IN )
        fit_svg( out )
        print( "made", name, "svg", os.path.getsize( out ) )
        return
    raw = os.path.join( HERE, "_grab_" + name + ".png" )
    # A zoomed view fills the window, so it is not cropped to what is drawn: the two of a pair keep one frame.
    vsp.ScreenGrab( raw, size[ 0 ], size[ 1 ], True, center is None )

    im = Image.open( raw ).convert( "RGBA" )
    print( "grab", name, im.size, im.getbbox() )
    if keep or os.environ.get( "KEEP_RAW" ):
        print( "kept", raw )
    else:
        os.remove( raw )
    if crop is not None:
        w, h = im.size
        im = im.crop( ( int( crop[ 0 ] * w ), int( crop[ 1 ] * h ), int( crop[ 2 ] * w ), int( crop[ 3 ] * h ) ) )
    if turn:
        # Turned a quarter clockwise, so a long, narrow view fills the page's width.
        im = im.rotate( -90, expand = True )
    bg = Image.new( "RGBA", im.size, ( 255, 255, 255, 255 ) )
    bg.alpha_composite( im )
    im = bg.convert( "RGB" )
    scale = min( WIDTH / im.size[ 0 ], HMAX / im.size[ 1 ], 1.0 )
    im = im.resize( ( int( round( im.size[ 0 ] * scale ) ), int( round( im.size[ 1 ] * scale ) ) ), Image.LANCZOS )
    if im.size[ 0 ] < WIDTH and not name.startswith( "_" ):
        page = Image.new( "RGB", ( WIDTH, im.size[ 1 ] ), ( 255, 255, 255 ) )
        page.paste( im, ( ( WIDTH - im.size[ 0 ] ) // 2, 0 ) )
        im = page
    im.save( os.path.join( OUT, name + ".png" ), dpi = ( 200, 200 ) )
    print( "made", name, im.size )


def fit_svg( fn ):
    """Give an SVG the page's width, 7 in, or less where it would be taller than 4.6 in, keeping its viewBox."""
    import re
    txt = open( fn ).read()
    vb = [ float( v ) for v in re.search( r'viewBox="([^"]*)"', txt ).group( 1 ).split() ]
    w = 7.0
    h = w * vb[ 3 ] / vb[ 2 ]
    if h > 4.6:
        h = 4.6
        w = h * vb[ 2 ] / vb[ 3 ]
    txt = re.sub( r'width="[^"]*" height="[^"]*"', 'width="%.3fin" height="%.3fin"' % ( w, h ), txt, count = 1 )
    open( fn, "w" ).write( txt )


def window( V, view, lo, hi ):
    """The crop, as fractions of a grab of the whole mesh V seen in view, that shows the model window lo..hi (two
    corners in the view's right and up axes)."""
    import numpy as np
    right, up = VIEW_AXES[ view ]
    u = V @ np.array( right )
    v = V @ np.array( up )
    du = u.max() - u.min()
    dv = v.max() - v.min()
    return ( ( lo[ 0 ] - u.min() ) / du, ( v.max() - hi[ 1 ] ) / dv, ( hi[ 0 ] - u.min() ) / du,
             ( v.max() - lo[ 1 ] ) / dv )


# The screen's right and up directions of each 2D view, in model axes.
VIEW_AXES = {
    "CAM_TOP": ( ( 1.0, 0.0, 0.0 ), ( 0.0, 1.0, 0.0 ) ),
    "CAM_LEFT": ( ( 1.0, 0.0, 0.0 ), ( 0.0, 0.0, 1.0 ) ),
    "CAM_REAR": ( ( 0.0, 1.0, 0.0 ), ( 0.0, 0.0, 1.0 ) ),
    "CAM_RIGHT": ( ( -1.0, 0.0, 0.0 ), ( 0.0, 0.0, 1.0 ) ),
}


def run( name ):
    """Make one picture in a process of its own."""
    r = subprocess.run( [ sys.executable, os.path.abspath( __file__ ), "--one", name ], capture_output = True,
                        text = True )
    lines = [ l for l in r.stdout.splitlines() if l.startswith( "made" ) ]
    print( "\n".join( lines ) if lines else r.stdout[ -2000: ] + r.stderr[ -2000: ] )


def one( name ):
    import openvsp_config
    openvsp_config.LOAD_GRAPHICS = True
    openvsp_config.LOAD_FACADE = True
    sys.path.insert( 0, HERE )
    SHOTS[ name ]()


# ---------------------------------------------------------------- the pictures


def compose( name, parts, labels, ncol ):
    """Lay the grabs parts (names of images in OUT, removed afterwards) out in ncol columns at the page's width, with
    a label in the top left corner of each."""
    import matplotlib
    matplotlib.use( "Agg" )
    import matplotlib.pyplot as plt
    from PIL import Image
    sys.path.insert( 0, os.path.join( HERE, "..", "propeller" ) )
    import figlib as fl
    fl.style()
    ims = [ Image.open( os.path.join( OUT, p + ".png" ) ) for p in parts ]
    nrow = ( len( ims ) + ncol - 1 ) // ncol
    gap = 0.04
    w = ( fl.PAGE_W - gap * ( ncol - 1 ) ) / ncol
    h = max( w * im.size[ 1 ] / im.size[ 0 ] for im in ims )
    if nrow * h > fl.HMAX:
        h = fl.HMAX / nrow
        w = min( w, min( h * im.size[ 0 ] / im.size[ 1 ] for im in ims ) )
    H = nrow * h + gap * ( nrow - 1 )
    fig = plt.figure( figsize = ( fl.PAGE_W, H ) )
    for k, ( im, lab ) in enumerate( zip( ims, labels ) ):
        r, c = divmod( k, ncol )
        cw = fl.PAGE_W / ncol
        x0 = c * cw + 0.5 * ( cw - w )
        ax = fig.add_axes( [ x0 / fl.PAGE_W, 1.0 - ( r + 1 ) * h / H - r * gap / H, w / fl.PAGE_W, h / H ] )
        ax.imshow( im, interpolation = "lanczos" )
        ax.set_axis_off()
        if lab:
            ax.text( 0.01, 0.99, lab, transform = ax.transAxes, ha = "left", va = "top",
                     bbox = dict( boxstyle = "square,pad=0.2", fc = "white", ec = "none" ) )
    fig.savefig( os.path.join( OUT, name + ".png" ), dpi = 200, metadata = { "Software": None } )
    plt.close( fig )
    for p in parts:
        os.remove( os.path.join( OUT, p + ".png" ) )
    print( "made", name )


def figs():
    sys.path.insert( 0, os.path.join( HERE, "..", "propeller" ) )
    import cfdmesh_figs as F
    return F


def shot_aircraft():
    import cfdmodel as cm
    cm.aircraft()
    cm.coarse()
    cm.mesh( "aircraft" )
    grab( "CFDMesh_aircraft", "CAM_LEFT_ISO", svg = True )


def shot_reason():
    import openvsp as vsp
    import cfdmodel as cm
    f, w, h = cm.aircraft()
    cm.coarse()
    vsp.DeleteAllCFDSources()
    vsp.AddDefaultSources()
    vsp.SetCFDWakeFlag( w, True )
    vsp.Update()
    cm.mesh( "reason" )
    grab( "CFDMesh_reason", "CAM_LEFT_ISO", color = "REASON" )


def shot_negative():
    import openvsp as vsp
    import cfdmodel as cm
    cm.negative_volume()
    cm.mesh( "negative" )
    grab( "CFDMesh_negative", "CAM_LEFT_ISO", color = "TAG", svg = True )


def shot_disk():
    import cfdmodel as cm
    cm.actuator_disk()
    cm.mesh( "disk" )
    grab( "CFDMesh_disk", "CAM_LEFT_ISO", color = "TAG", svg = True )


def growth_one( g, name ):
    import openvsp as vsp
    import cfdmodel as cm
    cm.aircraft()
    cm.coarse( growth = g )
    vsp.DeleteAllCFDSources()
    vsp.AddDefaultSources()
    vsp.Update()
    V, T, tag = cm.mesh( "growth" )
    grab( name, "CAM_TOP", crop = window( V, "CAM_TOP", ( 3.0, 1.0 ), ( 6.2, 4.2 ) ), size = ( 8000, 4500 ) )


def shot_growth_11():
    growth_one( 1.1, "_growth_11" )


def shot_growth_13():
    growth_one( 1.3, "_growth_13" )


def shot_growth_20():
    growth_one( 2.0, "_growth_20" )


def shot_growth():
    for k in ( "growth_11", "growth_13", "growth_20" ):
        run( k )
    compose( "CFDMesh_growth", [ "_growth_11", "_growth_13", "_growth_20" ], [ "1.1", "1.3", "2.0" ], 3 )


def shot_degen():
    import openvsp as vsp
    import cfdmodel as cm
    cm.aircraft()
    cm.coarse()
    cm.mesh( "degen", set = vsp.SET_NONE, degen = vsp.SET_ALL )
    grab( "CFDMesh_degen", "CAM_LEFT_ISO", color = "TAG", svg = True )


def shot_source_types():
    F = figs()
    import cfdmodel as cm
    F.source_types_model()
    cm.mesh( "srctypes" )
    grab( "CFDMesh_source_types", "CAM_TOP", turn = True )


def wingsrc_one( sources, name ):
    import openvsp as vsp
    import cfdmodel as cm
    F = figs()
    F.wing_alone()
    cm.coarse()
    if sources:
        vsp.DeleteAllCFDSources()
        vsp.AddDefaultSources()
    cm.mesh( "wing" )
    grab( name, "CAM_TOP" )


def shot_wing_bare():
    wingsrc_one( False, "_wing_bare" )


def shot_wing_src():
    wingsrc_one( True, "_wing_src" )


def shot_wing_sources():
    run( "wing_bare" )
    run( "wing_src" )
    compose( "CFDMesh_wing_sources", [ "_wing_bare", "_wing_src" ], [ "", "" ], 2 )


def shot_wake():
    import openvsp as vsp
    import cfdmodel as cm
    f, w, h = cm.aircraft()
    cm.coarse()
    vsp.SetCFDWakeFlag( w, True )
    vsp.Update()
    cm.mesh( "wake" )
    grab( "CFDMesh_wake", "CAM_LEFT_ISO", svg = True )


def ball_crop( view, size, frac ):
    """The crop, as fractions of the grab, of a window frac of the grab's width wide about the ball.  The ball's dense
    mesh draws as a solid disk of edge-line gray, so it is where gray pixels are densest."""
    import numpy as np
    from PIL import Image
    grab( "_ball_find", view, color = "TAG", size = size, keep = True )
    im = np.asarray( Image.open( os.path.join( HERE, "_grab__ball_find.png" ) ).convert( "RGBA" ) ).astype( float )
    os.remove( os.path.join( HERE, "_grab__ball_find.png" ) )
    os.remove( os.path.join( OUT, "_ball_find.png" ) )
    rgb = im[ :, :, :3 ]
    gray = ( im[ :, :, 3 ] > 200 ) & ( rgb.max( 2 ) - rgb.min( 2 ) < 40 )
    h, w = gray.shape
    n = max( 4, w // 80 )
    c = np.pad( np.cumsum( np.cumsum( gray, 0 ), 1 ), ( ( 1, 0 ), ( 1, 0 ) ) )
    dens = c[ n:, n: ] - c[ :-n, n: ] - c[ n:, :-n ] + c[ :-n, :-n ]
    cy, cx = np.unravel_index( np.argmax( dens ), dens.shape )
    cx += n / 2.0
    cy += n / 2.0
    r = 0.5 * frac * w
    return ( ( cx - r ) / w, ( cy - r ) / h, ( cx + r ) / w, ( cy + r ) / h )


def puck_one( rig, color, name ):
    """The box's top face under the ball, from above and to the side, with Rigorous 3D Growth Limiting off or on."""
    import openvsp as vsp
    import cfdmodel as cm
    cm.puck_ball()
    vsp.SetParmVal( cm.density( "RigorLimit" ), rig )
    vsp.Update()
    cm.mesh( "puck" )
    size = ( 7000, 3500 )
    grab( name, "CAM_LEFT_ISO", crop = ball_crop( "CAM_LEFT_ISO", size, 0.22 ), color = color, size = size )


def shot_puck_off():
    puck_one( 0.0, "PLAIN", "_puck_off" )


def shot_puck_on():
    puck_one( 1.0, "PLAIN", "_puck_on" )


def shot_puck_reason_off():
    puck_one( 0.0, "REASON", "_puck_reason_off" )


def shot_puck_reason_on():
    puck_one( 1.0, "REASON", "_puck_reason_on" )


def shot_puck():
    for k in ( "puck_off", "puck_on", "puck_reason_off", "puck_reason_on" ):
        run( k )
    compose( "CFDMesh_rigorous", [ "_puck_off", "_puck_on" ], [ "off", "on" ], 2 )
    compose( "CFDMesh_rigorous_reason", [ "_puck_reason_off", "_puck_reason_on" ], [ "off", "on" ], 2 )


def sym_one( split, name ):
    """The symmetry plane near the fuselage in the 50x box, zoomed in through the view Parms."""
    import openvsp as vsp
    F = figs()
    F.sym_mesh( split )
    grab( name, "CAM_RIGHT", center = ( 5.0, 0.0, 0.0 ), zoom = 60.0, size = ( 4000, 2280 ), crop = ( 0.0, 0.2, 1.0, 0.8 ) )


def shot_sym_off():
    sym_one( 0.0, "_sym_off" )


def shot_sym_on():
    sym_one( 1.0, "_sym_on" )


def shot_symmetry():
    run( "sym_off" )
    run( "sym_on" )
    compose( "CFDMesh_symmetry", [ "_sym_off", "_sym_on" ], [ "off", "on" ], 1 )


def shot_top():
    """cfdmodel.top() colored by Reason."""
    import openvsp as vsp
    import cfdmodel as cm
    cm.top()
    cm.mesh( "top" )
    grab( "CFDMesh_top_reason", "CAM_LEFT_ISO", color = "REASON" )


FAR_SPHERE_R = 20.0


def farfield_one( comp, half, name ):
    import openvsp as vsp
    import cfdmodel as cm
    cm.aircraft()
    cm.coarse()
    vsp.SetCFDMeshVal( vsp.CFD_FAR_FIELD_FLAG, 1.0 )
    vsp.SetCFDMeshVal( vsp.CFD_HALF_MESH_FLAG, float( half ) )
    if comp:
        e, c = cm.centered_sphere( FAR_SPHERE_R )
        vsp.SetSetFlag( e, vsp.SET_SHOWN, False )
        vsp.SetSetFlag( e, vsp.SET_FIRST_USER, True )
        vsp.Update()
        vsp.SetParmVal( cm.setting( "FarComp", "FarField" ), 1.0 )
        vsp.SetCFDFarFieldGeomID( e )
    vsp.SetCFDMeshVal( vsp.CFD_FAR_MAX_EDGE_LEN, 4.0 )
    vsp.Update()
    cm.mesh( "far", set = vsp.SET_ALL )
    grab( name, "CAM_LEFT_ISO", size = ( 4000, 3000 ) )


def shot_far_box_full():
    farfield_one( False, False, "_far_box_full" )


def shot_far_box_half():
    farfield_one( False, True, "_far_box_half" )


def shot_far_sphere_full():
    farfield_one( True, False, "_far_sphere_full" )


def shot_far_sphere_half():
    farfield_one( True, True, "_far_sphere_half" )


def shot_farfield():
    for k in ( "far_box_full", "far_box_half", "far_sphere_full", "far_sphere_half" ):
        run( k )
    compose( "CFDMesh_farfield", [ "_far_box_full", "_far_box_half", "_far_sphere_full", "_far_sphere_half" ],
             [ "box, full", "box, half", "sphere, full", "sphere, half" ], 2 )


def shot_duct():
    """The flow-through nacelle with a disk in its duct, from above and to the side and from upstream, both at one
    zoom about the nacelle's middle, uncropped, so they share a scale."""
    import cfdmodel as cm
    cm.duct_disk()
    cm.mesh( "duct" )
    for name, view in ( ( "_duct_iso", "CAM_LEFT_ISO" ), ( "_duct_front", "CAM_FRONT" ) ):
        grab( name, view, color = "TAG", size = ( 3000, 2250 ), center = ( 2.0, 0.0, 0.0 ), zoom = 1.0,
              zoom_abs = DUCT_ZOOM, crop = ( 0.18, 0.08, 0.82, 0.92 ) )
    compose( "CFDMesh_duct", [ "_duct_iso", "_duct_front" ], [ "", "" ], 2 )


DUCT_ZOOM = 0.0021     # the view Parm Zoom of both pictures


def shot_source_preview():
    import openvsp as vsp
    import cfdmodel as cm
    F = figs()
    F.wing_alone()
    cm.coarse()
    vsp.DeleteAllCFDSources()
    vsp.AddDefaultSources()
    cm.mesh( "srcprev" )
    grab( "CFDMesh_source_preview", "CAM_LEFT_ISO", preview = True )


SHOTS = {
    "aircraft": shot_aircraft,
    "reason": shot_reason,
    "negative": shot_negative,
    "disk": shot_disk,
    "growth_11": shot_growth_11,
    "growth_13": shot_growth_13,
    "growth_20": shot_growth_20,
    "growth": shot_growth,
    "degen": shot_degen,
    "sourcetypes": shot_source_types,
    "wing_bare": shot_wing_bare,
    "wing_src": shot_wing_src,
    "wingsources": shot_wing_sources,
    "wake": shot_wake,
    "puck_off": shot_puck_off,
    "puck_on": shot_puck_on,
    "puck_reason_off": shot_puck_reason_off,
    "puck_reason_on": shot_puck_reason_on,
    "puck": shot_puck,
    "sym_off": shot_sym_off,
    "sym_on": shot_sym_on,
    "symmetry": shot_symmetry,
    "top": shot_top,
    "far_box_full": shot_far_box_full,
    "far_box_half": shot_far_box_half,
    "far_sphere_full": shot_far_sphere_full,
    "far_sphere_half": shot_far_sphere_half,
    "farfield": shot_farfield,
    "duct": shot_duct,
    "sourcepreview": shot_source_preview,
}

# The pictures made by default; the others are parts of these.
MAIN = [ "aircraft", "reason", "negative", "disk", "growth", "degen", "sourcetypes",
         "wingsources", "wake", "puck", "symmetry", "top", "farfield", "duct", "sourcepreview" ]

if __name__ == "__main__":
    if len( sys.argv ) > 2 and sys.argv[ 1 ] == "--one":
        one( sys.argv[ 2 ] )
    else:
        for k in ( sys.argv[ 1: ] or MAIN ):
            if k in ( "growth", "wingsources", "puck", "symmetry", "farfield" ):
                SHOTS[ k ]()
            else:
                run( k )
