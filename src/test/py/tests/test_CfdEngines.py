# Every engine representation CFDMesh is asked to close.
#
# Each Stack engine preset is meshed in every inlet and outlet representation the GUI offers
# for it, through a block that runs along the engine and cuts through it, the way a wing or
# a pylon would.  The negative representations cut the block and the engine's own body, so
# the faces where a positive and a negative body meet, the lips four surfaces share, and the
# flowpaths that pass from one body to the other all have to come out closed.
#
# The choices EngineModelScreen::Update greys out for a preset are meshed too, since files and
# the API can still set them: a file written before FLOWTHROUGH NEGATIVE ONLY was inserted among
# the modes loads its TO LIP inlet as that mode.  The cases where coincident faces of a positive
# and a negative body are paired are meshed again with the engine stretched along its axis,
# since the negative face is set off from the positive one by a fraction of the whole engine.
#
# Closure alone would pass a mesh that dropped a whole recess or kept both coincident faces, so
# the volume each mesh encloses is held to the volume CompGeom finds for the same model.
#
# The offered cases are also laid out together in one model, a row in Z for each preset and a
# column in Y for each pair of inlet and outlet representations, with the pairs a preset does not
# offer left empty.  The model is written to the working directory as EngineGrid.vsp3, to be
# meshed by hand; --engine-grid-mesh also meshes it here, which takes several minutes.

import os

import openvsp as vsp
import pytest


ENGINE_PRESETS = [ vsp.STACK_PRESET_FLOWTHRU_OUTLIP_ORIG, vsp.STACK_PRESET_FLOWTHRU_INLIP_ORIG,
                   vsp.STACK_PRESET_FLOWTHRU_MID_ORIG, vsp.STACK_PRESET_BOTHFACEFACE,
                   vsp.STACK_PRESET_BOTHLIPFACE, vsp.STACK_PRESET_BOTHFACELIP, vsp.STACK_PRESET_BOTHLIPLIP,
                   vsp.STACK_PRESET_OUTFACE, vsp.STACK_PRESET_OUTLIP, vsp.STACK_PRESET_INFACE,
                   vsp.STACK_PRESET_INLIP, vsp.STACK_PRESET_FLOWPATH_BOTH, vsp.STACK_PRESET_FLOWPATH_IN,
                   vsp.STACK_PRESET_FLOWPATH_OUT ]

FLOWTHROUGH_MODES = ( vsp.ENGINE_MODE_FLOWTHROUGH, vsp.ENGINE_MODE_FLOWTHROUGH_NEG,
                      vsp.ENGINE_MODE_FLOWTHROUGH_NEG_ONLY )

MODE_NAMES = { vsp.ENGINE_MODE_FLOWTHROUGH: "FLOWTHROUGH", vsp.ENGINE_MODE_FLOWTHROUGH_NEG: "FLOWTHROUGH_NEG",
               vsp.ENGINE_MODE_FLOWTHROUGH_NEG_ONLY: "FLOWTHROUGH_NEG_ONLY", vsp.ENGINE_MODE_TO_LIP: "TO_LIP",
               vsp.ENGINE_MODE_TO_FACE: "TO_FACE", vsp.ENGINE_MODE_TO_FACE_NEG: "TO_FACE_NEG",
               vsp.ENGINE_MODE_TO_FACE_NEG_ONLY: "TO_FACE_NEG_ONLY", vsp.ENGINE_MODE_EXTEND: "EXTEND", None: "-" }


def _engine_types( preset ):
    """The inlet/outlet description and the inlet and outlet geometry types a preset sets."""
    vsp.VSPRenew()
    eng = vsp.AddGeom( "STACK" )
    vsp.InitStackPreset( eng, preset )
    vsp.Update()
    return ( int( vsp.GetParmVal( eng, "GeomIOType", "EngineModel" ) ),
             int( vsp.GetParmVal( eng, "GeomInType", "EngineModel" ) ),
             int( vsp.GetParmVal( eng, "GeomOutType", "EngineModel" ) ) )


def _inlet_modes( io, gin ):
    """The inlet representations the GUI offers, or None where there is no inlet."""
    if io == vsp.ENGINE_GEOM_OUTLET:
        return [ None ]
    if gin == vsp.ENGINE_GEOM_FLOWTHROUGH:
        return list( FLOWTHROUGH_MODES ) + [ vsp.ENGINE_MODE_TO_LIP, vsp.ENGINE_MODE_TO_FACE, vsp.ENGINE_MODE_TO_FACE_NEG,
                                             vsp.ENGINE_MODE_TO_FACE_NEG_ONLY, vsp.ENGINE_MODE_EXTEND ]
    if gin == vsp.ENGINE_GEOM_TO_LIP:
        return [ vsp.ENGINE_MODE_TO_LIP, vsp.ENGINE_MODE_EXTEND ]
    if gin == vsp.ENGINE_GEOM_TO_FACE:
        return [ vsp.ENGINE_MODE_TO_LIP, vsp.ENGINE_MODE_TO_FACE, vsp.ENGINE_MODE_TO_FACE_NEG,
                 vsp.ENGINE_MODE_TO_FACE_NEG_ONLY, vsp.ENGINE_MODE_EXTEND ]
    # A flowpath: no plain to-face representation, and a flowthrough one only when it has both ends.
    modes = [ vsp.ENGINE_MODE_TO_LIP, vsp.ENGINE_MODE_TO_FACE_NEG, vsp.ENGINE_MODE_TO_FACE_NEG_ONLY, vsp.ENGINE_MODE_EXTEND ]
    if io == vsp.ENGINE_GEOM_INLET_OUTLET:
        modes += [ vsp.ENGINE_MODE_FLOWTHROUGH_NEG, vsp.ENGINE_MODE_FLOWTHROUGH_NEG_ONLY ]
    return modes


def _outlet_modes( io, gin, gout, inmode ):
    """The outlet representations the GUI offers, or None where there is no outlet or it follows the inlet."""
    if io == vsp.ENGINE_GEOM_INLET:
        return [ None ]
    flowpath = gin == vsp.ENGINE_GEOM_FLOWPATH or gout == vsp.ENGINE_GEOM_FLOWPATH
    if io == vsp.ENGINE_GEOM_INLET_OUTLET and not flowpath and inmode in FLOWTHROUGH_MODES:
        return [ None ]

    to_face = not ( gout == vsp.ENGINE_GEOM_TO_LIP and
                    not ( io == vsp.ENGINE_GEOM_INLET_OUTLET and gin == vsp.ENGINE_GEOM_FLOWTHROUGH ) )
    plain_face = to_face and not ( gout == vsp.ENGINE_GEOM_FLOWPATH or
                                   ( io == vsp.ENGINE_GEOM_INLET_OUTLET and gin == vsp.ENGINE_GEOM_FLOWPATH ) )

    modes = [ vsp.ENGINE_MODE_TO_LIP ]
    if plain_face:
        modes.append( vsp.ENGINE_MODE_TO_FACE )
    if to_face or flowpath:
        modes += [ vsp.ENGINE_MODE_TO_FACE_NEG, vsp.ENGINE_MODE_TO_FACE_NEG_ONLY ]
    modes.append( vsp.ENGINE_MODE_EXTEND )
    return modes


def _cases():
    """Every preset in every representation the GUI offers, then every greyed one."""
    offered = []
    greyed = []
    for preset in ENGINE_PRESETS:
        io, gin, gout = _engine_types( preset )

        ins = _inlet_modes( io, gin )
        for inmode in ins:
            for outmode in _outlet_modes( io, gin, gout, inmode ):
                offered.append( ( preset, inmode, outmode ) )

        allin = [ None ]
        if io != vsp.ENGINE_GEOM_OUTLET:
            allin = list( range( vsp.ENGINE_MODE_NUM_TYPES ) )
        allout = [ None ]
        if io != vsp.ENGINE_GEOM_INLET:
            allout = list( range( vsp.ENGINE_MODE_TO_LIP, vsp.ENGINE_MODE_NUM_TYPES ) )
        for inmode in allin:
            for outmode in allout:
                if inmode in ins and outmode in _outlet_modes( io, gin, gout, inmode ):
                    continue
                if inmode in ins and None in _outlet_modes( io, gin, gout, inmode ):
                    continue
                greyed.append( ( preset, inmode, outmode ) )
    return offered, greyed


OFFERED, GREYED = _cases()

# The meshes are coarse, and CompGeom's tessellation is not the mesh's.
VOLUME_RELTOL = 0.05

PAIRED_MODES = ( vsp.ENGINE_MODE_FLOWTHROUGH_NEG, vsp.ENGINE_MODE_TO_FACE_NEG )


# Known to fail once the engine is stretched, where one of the block's flat sides lands on a
# feature of the engine: along the paired faces at a flowpath's outlet face, where the block's
# sides cross both faces.
STRETCHED = { ( vsp.STACK_PRESET_FLOWPATH_BOTH, vsp.ENGINE_MODE_FLOWTHROUGH_NEG, vsp.ENGINE_MODE_TO_FACE_NEG, 4.0 ):
                  "over-connected edges where the block crosses the paired faces at the outlet face",
              ( vsp.STACK_PRESET_FLOWPATH_BOTH, vsp.ENGINE_MODE_FLOWTHROUGH_NEG_ONLY, vsp.ENGINE_MODE_TO_FACE_NEG, 4.0 ):
                  "over-connected edges where the block crosses the paired faces at the outlet face" }


def _params( cases, stretch, tag ):
    params = []
    for preset, inmode, outmode in cases:
        marks = []
        reason = STRETCHED.get( ( preset, inmode, outmode, stretch ) )
        if reason:
            marks = [ pytest.mark.xfail( strict = True, reason = reason ) ]
        params.append( pytest.param( preset, inmode, outmode, stretch, marks = marks,
                                     id = "%s-x%g-preset%d-%s-%s" % ( tag, stretch, preset, MODE_NAMES[inmode],
                                                                      MODE_NAMES[outmode] ) ) )
    return params


def _all_params():
    params = _params( OFFERED, 1.0, "offered" ) + _params( GREYED, 1.0, "greyed" )
    paired = [ c for c in OFFERED if c[1] in PAIRED_MODES or c[2] in PAIRED_MODES ]
    for stretch in ( 4.0, 10.0 ):
        params += _params( paired, stretch, "offered" )
    return params


def _block( eng ):
    """A flat block along the engine's axis, wider than the engine and three of its diameters long."""
    xmin, xmax, ymin, ymax, zmin, zmax = [], [], [], [], [], []
    for i in range( vsp.GetNumMainSurfs( eng ) ):
        bmax = vsp.GetGeomBBoxMax( eng, i, True )
        bmin = vsp.GetGeomBBoxMin( eng, i, True )
        xmin.append( bmin.x() )
        xmax.append( bmax.x() )
        ymin.append( bmin.y() )
        ymax.append( bmax.y() )
        zmin.append( bmin.z() )
        zmax.append( bmax.z() )
    d = max( ymax ) - min( ymin )

    blk = vsp.AddGeom( "STACK" )
    vsp.InitStackPreset( blk, vsp.STACK_PRESET_CYLINDER_ENDCAPS )
    xss = vsp.GetXSecSurf( blk, 0 )
    for i in range( 2 ):
        vsp.ChangeXSecShape( xss, i, vsp.XS_ROUNDED_RECTANGLE )
        xs = vsp.GetXSec( xss, i )
        vsp.SetParmVal( vsp.GetXSecParm( xs, "RoundedRect_Width" ), 1.2 * d )
        vsp.SetParmVal( vsp.GetXSecParm( xs, "RoundedRect_Height" ), 0.2 * d )
        vsp.SetParmVal( vsp.GetXSecParm( xs, "RoundRectXSec_Radius" ), 0.0 )
    vsp.SetParmVal( vsp.GetXSecParm( vsp.GetXSec( xss, 1 ), "XDelta" ), 3.0 * d )
    vsp.SetParmVal( blk, "X_Rel_Location", "XForm", 0.5 * ( max( xmax ) + min( xmin ) ) - 1.5 * d )
    vsp.SetParmVal( blk, "Y_Rel_Location", "XForm", 0.5 * ( max( ymax ) + min( ymin ) ) )
    vsp.SetParmVal( blk, "Z_Rel_Location", "XForm", 0.5 * ( max( zmax ) + min( zmin ) ) )
    return d


def _stl_volume( path ):
    """The volume a closed triangulation in an ASCII STL file encloses."""
    vol = 0.0
    tri = []
    for line in open( path ):
        f = line.split()
        if f and f[0] == "vertex":
            tri.append( [ float( x ) for x in f[1:4] ] )
            if len( tri ) == 3:
                a, b, c = tri
                vol += ( a[0] * ( b[1] * c[2] - b[2] * c[1] ) - a[1] * ( b[0] * c[2] - b[2] * c[0] ) +
                         a[2] * ( b[0] * c[1] - b[1] * c[0] ) ) / 6.0
                tri = []
    return abs( vol )


@pytest.mark.parametrize( "preset, inmode, outmode, stretch", _all_params() )
def testAnEngineThroughABlockIsClosed( preset, inmode, outmode, stretch, tmp_path ):
    vsp.VSPRenew()
    eng = vsp.AddGeom( "STACK" )
    vsp.InitStackPreset( eng, preset )
    vsp.Update()

    xss = vsp.GetXSecSurf( eng, 0 )
    for i in range( 1, vsp.GetNumXSec( xss ) ):
        p = vsp.GetXSecParm( vsp.GetXSec( xss, i ), "XDelta" )
        vsp.SetParmVal( p, stretch * vsp.GetParmVal( p ) )
    vsp.Update()

    # The block is sized and placed by the preset as it comes, since some representations
    # leave no surface at all, and an extension would carry the block off downstream.
    d = _block( eng )

    if inmode is not None:
        vsp.SetParmVal( eng, "InletModeType", "EngineModel", inmode )
    if outmode is not None:
        vsp.SetParmVal( eng, "OutletModeType", "EngineModel", outmode )
    vsp.Update()

    what = "Stack preset %d stretched %g, inlet %s, outlet %s" % ( preset, stretch, MODE_NAMES[inmode], MODE_NAMES[outmode] )

    mesh = vsp.ComputeCompGeom( vsp.SET_ALL, False, 0 )
    cgvol = vsp.GetDoubleResults( vsp.FindLatestResultsID( "Comp_Geom" ), "Total_Wet_Vol" )[0]
    vsp.DeleteGeom( mesh )
    vsp.Update()

    stl = str( tmp_path / "engine.stl" )
    vsp.SetComputationFileName( vsp.CFD_STL_TYPE, stl )
    vsp.SetCFDMeshVal( vsp.CFD_MAX_EDGE_LEN, 0.15 * d )
    vsp.SetCFDMeshVal( vsp.CFD_MIN_EDGE_LEN, 0.02 * d )
    vsp.SetCFDMeshVal( vsp.CFD_SPLIT_JOIN_SURFS_FLAG, 1 )
    vsp.SetCFDMeshVal( vsp.CFD_HALF_MESH_FLAG, 0 )
    vsp.ComputeCFDMesh( vsp.SET_ALL, vsp.SET_NONE, vsp.CFD_STL_TYPE )

    rid = vsp.FindLatestResultsID( "CFDMesh" )
    assert rid, "%s: the mesher wrote no CFDMesh results entry" % what

    ntri = vsp.GetIntResults( rid, "Num_Tris" )[0]
    nborder = vsp.GetIntResults( rid, "Num_Border_Edges" )[0]
    nover = vsp.GetIntResults( rid, "Num_Over_Connected_Edges" )[0]

    assert ntri > 0, "%s meshed nothing" % what

    assert nborder == 0 and nover == 0, \
           "%s left %d border and %d over-connected edges" % ( what, nborder, nover )

    vol = _stl_volume( stl )
    assert vol == pytest.approx( cgvol, rel = VOLUME_RELTOL ), \
           "%s encloses %g, CompGeom finds %g" % ( what, vol, cgvol )


@pytest.mark.parametrize( "splitjoin", [ 0, 1 ] )
def testALipCapAlongABlockPatchBorderIsClosed( splitjoin, tmp_path ):
    """The nacelle from its inlet lip to its outlet face, represented to both lips, where it sits
    in the grid and meshed at the default sizes.  The flat cap at the inlet lip meets a flat side of
    the block along a line where two of the block's patches meet, so every intersection segment on
    the cap lies along an edge of a block triangle."""
    vsp.VSPRenew()
    eng = vsp.AddGeom( "STACK" )
    vsp.InitStackPreset( eng, vsp.STACK_PRESET_BOTHLIPFACE )
    vsp.SetParmVal( eng, "Y_Rel_Location", "XForm", 136.0 )
    vsp.SetParmVal( eng, "Z_Rel_Location", "XForm", 32.0 )
    vsp.Update()

    _block( eng )

    vsp.SetParmVal( eng, "InletModeType", "EngineModel", vsp.ENGINE_MODE_TO_LIP )
    vsp.SetParmVal( eng, "OutletModeType", "EngineModel", vsp.ENGINE_MODE_TO_LIP )
    vsp.Update()

    stl = str( tmp_path / "engine.stl" )
    vsp.SetComputationFileName( vsp.CFD_STL_TYPE, stl )
    vsp.SetCFDMeshVal( vsp.CFD_SPLIT_JOIN_SURFS_FLAG, splitjoin )
    vsp.SetCFDMeshVal( vsp.CFD_HALF_MESH_FLAG, 0 )
    vsp.ComputeCFDMesh( vsp.SET_ALL, vsp.SET_NONE, vsp.CFD_STL_TYPE )

    rid = vsp.FindLatestResultsID( "CFDMesh" )
    assert rid, "the mesher wrote no CFDMesh results entry"

    ntri = vsp.GetIntResults( rid, "Num_Tris" )[0]
    nborder = vsp.GetIntResults( rid, "Num_Border_Edges" )[0]
    nover = vsp.GetIntResults( rid, "Num_Over_Connected_Edges" )[0]

    assert ntri > 0
    assert nborder == 0 and nover == 0, "split and join %d left %d border and %d over-connected edges" % \
                                        ( splitjoin, nborder, nover )


def _mode_key( mode ):
    """A representation mode for sorting, with no mode first."""
    if mode is None:
        return -1
    return mode


def _build_grid():
    """Every offered case in one model: a row in Z for each preset, a column in Y for each pair of
    inlet and outlet representations, each engine through its own block.  A pair a preset does not
    offer is left empty.  Returns the rows, the columns, the largest engine diameter and the
    pitch of the grid."""
    rows = list( ENGINE_PRESETS )
    cols = sorted( set( ( c[1], c[2] ) for c in OFFERED ), key = lambda c: ( _mode_key( c[0] ), _mode_key( c[1] ) ) )

    vsp.VSPRenew()

    # The pitch of the grid, from the largest preset.
    pitch = 0.0
    for preset in rows:
        eng = vsp.AddGeom( "STACK" )
        vsp.InitStackPreset( eng, preset )
        vsp.Update()
        bmin = vsp.GetGeomBBoxMin( eng, 0, True )
        bmax = vsp.GetGeomBBoxMax( eng, 0, True )
        pitch = max( pitch, bmax.y() - bmin.y(), bmax.z() - bmin.z() )
        vsp.DeleteGeom( eng )
    dmax = pitch
    pitch = 2.0 * pitch

    for irow, preset in enumerate( rows ):
        for icol, ( inmode, outmode ) in enumerate( cols ):
            if ( preset, inmode, outmode ) not in OFFERED:
                continue

            eng = vsp.AddGeom( "STACK" )
            vsp.InitStackPreset( eng, preset )
            vsp.SetGeomName( eng, "Preset%d_%s_%s" % ( preset, MODE_NAMES[inmode], MODE_NAMES[outmode] ) )
            vsp.SetParmVal( eng, "Y_Rel_Location", "XForm", icol * pitch )
            vsp.SetParmVal( eng, "Z_Rel_Location", "XForm", irow * pitch )
            vsp.Update()

            _block( eng )

            if inmode is not None:
                vsp.SetParmVal( eng, "InletModeType", "EngineModel", inmode )
            if outmode is not None:
                vsp.SetParmVal( eng, "OutletModeType", "EngineModel", outmode )
            vsp.Update()

    return rows, cols, dmax, pitch


def _bad_edge_cells( path, pitch ):
    """The grid cells, as ( row, column ), holding edges of a triangulation in an ASCII STL file
    used by one triangle or by more than two."""
    ids = {}
    count = {}
    tri = []
    where = {}
    for line in open( path ):
        f = line.split()
        if f and f[0] == "vertex":
            key = tuple( round( float( x ), 6 ) for x in f[1:4] )
            if key not in ids:
                ids[key] = len( ids )
                where[ ids[key] ] = key
            tri.append( ids[key] )
            if len( tri ) == 3:
                for a, b in ( ( tri[0], tri[1] ), ( tri[1], tri[2] ), ( tri[2], tri[0] ) ):
                    e = ( min( a, b ), max( a, b ) )
                    count[e] = count.get( e, 0 ) + 1
                tri = []
    cells = set()
    for e, n in count.items():
        if n != 2:
            y = 0.5 * ( where[ e[0] ][1] + where[ e[1] ][1] )
            z = 0.5 * ( where[ e[0] ][2] + where[ e[1] ][2] )
            cells.add( ( int( round( z / pitch ) ), int( round( y / pitch ) ) ) )
    return cells


def testEveryEngineCaseInOneGrid():
    """The offered cases laid out as a matrix in one model, written to EngineGrid.vsp3 in the
    working directory and read back whole."""
    _build_grid()

    fname = os.path.abspath( "EngineGrid.vsp3" )
    vsp.WriteVSPFile( fname, vsp.SET_ALL )

    vsp.VSPRenew()
    vsp.ReadVSPFile( fname )
    vsp.Update()

    nset = 0
    for gid in vsp.FindGeoms():
        if vsp.GetGeomName( gid ).startswith( "Preset" ):
            nset += 1
    assert nset == len( OFFERED )


def testEveryEngineCaseInOneGridIsClosed( tmp_path, request ):
    """The grid meshed as a whole, closed in every cell."""
    if not request.config.getoption( "--engine-grid-mesh" ):
        pytest.skip( "meshing the engine grid takes several minutes; ask for it with --engine-grid-mesh" )

    rows, cols, d, pitch = _build_grid()

    mesh = vsp.ComputeCompGeom( vsp.SET_ALL, False, 0 )
    cgvol = vsp.GetDoubleResults( vsp.FindLatestResultsID( "Comp_Geom" ), "Total_Wet_Vol" )[0]
    vsp.DeleteGeom( mesh )
    vsp.Update()

    stl = str( tmp_path / "EngineGrid.stl" )
    vsp.SetComputationFileName( vsp.CFD_STL_TYPE, stl )
    vsp.SetCFDMeshVal( vsp.CFD_MAX_EDGE_LEN, 0.15 * d )
    vsp.SetCFDMeshVal( vsp.CFD_MIN_EDGE_LEN, 0.02 * d )
    vsp.SetCFDMeshVal( vsp.CFD_SPLIT_JOIN_SURFS_FLAG, 1 )
    vsp.SetCFDMeshVal( vsp.CFD_HALF_MESH_FLAG, 0 )
    vsp.ComputeCFDMesh( vsp.SET_ALL, vsp.SET_NONE, vsp.CFD_STL_TYPE )

    rid = vsp.FindLatestResultsID( "CFDMesh" )
    assert rid, "the mesher wrote no CFDMesh results entry"

    ntri = vsp.GetIntResults( rid, "Num_Tris" )[0]
    nborder = vsp.GetIntResults( rid, "Num_Border_Edges" )[0]
    nover = vsp.GetIntResults( rid, "Num_Over_Connected_Edges" )[0]
    print( "EngineGrid: %d rows, %d columns, %d engines, %d tris, %d border, %d over-connected edges" %
           ( len( rows ), len( cols ), len( OFFERED ), ntri, nborder, nover ) )

    assert ntri > 0

    bad = _bad_edge_cells( stl, pitch )
    assert not bad, "the grid left %d border and %d over-connected edges, in cells %s" % ( nborder, nover, sorted( bad ) )

    vol = _stl_volume( stl )
    assert vol == pytest.approx( cgvol, rel = VOLUME_RELTOL ), "the grid encloses %g, CompGeom finds %g" % ( vol, cgvol )
