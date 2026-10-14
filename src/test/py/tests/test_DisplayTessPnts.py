"""The points a display type shows are numbered by the Geom's surfaces in every display type."""

import openvsp as vsp


def centroid( pnts ):
    n = len( pnts )
    return vsp.vec3d( sum( p.x() for p in pnts ) / n, sum( p.y() for p in pnts ) / n, sum( p.z() for p in pnts ) / n )


def test_a_propellers_degen_points_follow_its_surfaces():
    # Blades and disk, mirrored: the degen preview has no disk, so it holds fewer surfaces.
    vsp.VSPRenew()
    pid = vsp.AddGeom( "PROP" )
    vsp.SetParmVal( pid, "PropMode", "Design", vsp.PROP_BOTH )
    vsp.SetParmVal( pid, "Sym_Planar_Flag", "Sym", vsp.SYM_XZ )
    vsp.Update()
    nmain = vsp.GetNumMainSurfs( pid )
    nsurf = 2 * nmain
    bezier = [ vsp.GetGeomDisplayTessPnts( pid, i ) for i in range( nsurf ) ]
    vsp.SetGeomDisplayType( pid, vsp.DISPLAY_DEGEN_SURF )
    vsp.Update()
    for i in range( nsurf ):
        degen = vsp.GetGeomDisplayTessPnts( pid, i )
        # The mirror copies follow the main surfaces in the same order.
        if vsp.GetGeomVSPSurfType( pid, i % nmain ) == vsp.DISK_SURF:
            assert len( degen ) == 0, i
        else:
            assert vsp.dist( centroid( degen ), centroid( bezier[ i ] ) ) < 0.1, i
