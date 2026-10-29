"""Figures for MdcProtub.md."""

import os
import sys

HERE = os.path.dirname( os.path.abspath( __file__ ) )
sys.path.insert( 0, os.path.join( HERE, "..", "mdc_bod" ) )

import numpy as np

import datcomfig as df
import bodyfig as bf
from bodyfig import vsp, X, Y, Z, FS

TYPES = [ ( "VCYL", 1 ), ( "HCYL", 2 ), ( "LUG", 3 ), ( "SHOE", 4 ), ( "BLOCK", 5 ), ( "FAIRING", 6 ) ]


SRC = os.path.join( HERE, "..", "..", "..", "geom_core", "MdcProtubGeom.cpp" )


def default_members( ptype ):
    """The ( LPROT, WPROT, HPROT, OPROT ) of each member of ptype, for a body 6 in radius, as the Set Default button
    sets them.  The button is not in the API, so they are read from where it is written."""
    import re
    text = open( SRC ).read()
    body = text[ text.index( "void MdcProtubGeom::SetDefaults" ): text.index( "double MdcProtubGeom::Bottom" ) ]
    names = { 1: "VCYL", 2: "HCYL", 3: "LUG", 4: "SHOE", 5: "BLOCK" }
    if ptype in names:
        cond = "ptype == " + names[ ptype ]
        part = body[ body.index( cond ): ]
    else:
        part = body[ body.index( "else\n    {\n        mems" ): ]
    part = part[ part.index( "mems = {" ): part.index( ";" , part.index( "mems = {" ) ) ]
    rows = re.findall( r"\{\s*([-\d.]+),\s*([-\d.]+),\s*([-\d.]+),\s*([-\d.]+)\s*\}", part )
    return [ [ float( v ) for v in r ] for r in rows ]


def protub( ptype, rbody = 6.0, defaults = True ):
    """The protuberance of ptype on a body of radius rbody, with the sizes Set Default gives."""
    g = vsp.AddGeom( "MDC_PROTUB" )
    vsp.Update()
    vsp.SetParmVal( g, "RBODY", "Design", rbody )
    vsp.SetParmVal( g, "X_Rel_Location", "XForm", 0.0 )
    vsp.SetParmVal( g, "PTYPE", "Design", float( ptype ) )
    # The members' Parms are made by the update after PTYPE.
    vsp.Update()
    if defaults:
        k = rbody / 6.0
        for m, row in enumerate( default_members( ptype ) ):
            for nm, v in zip( ( "LPROT", "WPROT", "HPROT", "OPROT" ), row ):
                vsp.SetParmVal( g, "%s_%d" % ( nm, m + 1 ), "Design", k * v )
    vsp.Update()
    return g


def types():
    bf.renew()
    fig = bf.figure()
    cs = []
    for name, t in TYPES:
        bf.renew()
        body = df.cylinder( 6.0, -9.0, 9.0 )
        p = protub( t )
        vsp.SetParmVal( p, "NLOC", "Design", 1.0 )
        c = bf.canvas( fig, bf.ISO )
        df.draw( c, [ df.patch( body, -7.0, 7.0, 0.66, 0.84 ) ], df.BODY, edge = False )
        c.flush( 1.0 )
        df.draw( c, df.surfaces( p ), df.PART )
        c.flush( 5.0 )
        bf.title( c, name, pad = 2 )
        cs.append( c )
    bf.save( fig, "MdcProtub_types", [ cs[ 0:2 ], cs[ 2:4 ], cs[ 4:6 ] ] )


def phi():
    """MdcProtub_phi: the roll angles PHIPRO, from the top, clockwise looking forward."""
    bf.renew()
    body = df.cylinder( 6.0, -9.0, 9.0 )
    p = protub( 5, defaults = False )
    vsp.SetParmVal( p, "NLOC", "Design", 3.0 )
    vsp.Update()
    for k, v in dict( LPROT_1 = 3.0, WPROT_1 = 1.2, HPROT_1 = 0.9, PHIPROFlag = 1, PHIPRO_1 = 0, PHIPRO_2 = 90, PHIPRO_3 = 225 ).items():
        vsp.SetParmVal( p, k, "Design", float( v ) )
    vsp.Update()
    fig = bf.figure()
    c = bf.canvas( fig, df.REAR )
    bf.fill( c, df.circle( body, 0.0 ), face = df.BODY, edge = bf.POS_EDGE )
    for G in df.surfaces( p, 31, 161 ):
        c.surface( G, color = df.PART, edge = False )
    c.flush( 5.0 )
    c.line( np.array( [ [ 0, 0, 0 ], [ 0, 0, 10.0 ] ] ), color = "#777777", lw = 0.6, ls = ( 0, ( 2, 2 ) ) )
    bf.label( c, np.array( [ 0, 0, 10.0 ] ), "top", dy = 7 )
    for ang, rho, nm in ( ( 90, 8.2, "PHIPRO 90" ), ( 225, 9.4, "PHIPRO 225" ) ):
        c.line( np.array( [ [ 0, 0, 0 ], [ 0, 8.0 * np.sin( np.radians( ang ) ) * 1.05, 8.0 * np.cos( np.radians( ang ) ) * 1.05 ] ] ),
                color = "#777777", lw = 0.6, ls = ( 0, ( 2, 2 ) ), z = 20 )
        P = df.arc( c, rho, 0, ang )
        if ang < 180:
            bf.label( c, P[ -1 ], nm, ha = "left", dx = 8, dy = -3 )
        else:
            bf.label( c, P[ -1 ], nm, ha = "right", dx = -4, dy = -9 )
    bf.label( c, np.array( [ 0, 0, 6.9 ] ), "PHIPRO 0", dy = 8 )
    bf.key( c, "rear", ( 1.0, 0.0 ), 20, 0 )
    bf.save( fig, "MdcProtub_phi", [ [ c ] ], hmax = 4.8 )


def dims():
    """MdcProtub_dims: the sizes of a BLOCK, from the side and from behind."""
    kw = dict( RBODY = 3.0, X = 5.5, LPROT_1 = 2.5, WPROT_1 = 2.0, HPROT_1 = 1.2, OPROT_1 = 0.8, PTYPE = 5 )
    bf.renew()
    body = df.cylinder( 3.0, 0.0, 9.0 )
    p = df.add( "MDC_PROTUB", **kw )
    G = df.surfaces( p, 31, 161 )[ 0 ]
    P = G.reshape( -1, 3 )
    assert abs( P[ :, 2 ].min() - 3.8 ) < 1e-6 and abs( P[ :, 2 ].max() - 5.0 ) < 1e-6 and abs( P[ :, 0 ].max() - 6.75 ) < 1e-6
    fig = bf.figure()
    cs = bf.canvas( fig, bf.SIDE )
    bf.fill( cs, bf.body_cut( body, 0, 401 ) )
    bf.axis_line( cs, -0.5, 9.5 )
    for G in df.surfaces( p, 31, 161 ):
        cs.surface( G, color = df.PART, edge = False )
    cs.flush( 5.0 )
    cs.line( np.array( [ [ 0, 0, -3.6 ], [ 0, 0, 5.6 ] ] ), color = "#777777", lw = 0.6, ls = ( 0, ( 2, 2 ) ) )
    cs.line( np.array( [ [ 5.5, 0, -3.6 ], [ 5.5, 0, 5.6 ] ] ), color = "#777777", lw = 0.6, ls = ( 0, ( 2, 2 ) ) )
    bf.dim( cs, np.array( [ 0, 0, -3.6 ] ), np.array( [ 5.5, 0, -3.6 ] ), "XPROT", dy = -8, ext = False )
    bf.label( cs, np.array( [ 0, 0, 5.6 ] ), "origin", dy = 7 )
    bf.dim( cs, np.array( [ 4.25, 0, 5.3 ] ), np.array( [ 6.75, 0, 5.3 ] ), "LPROT", dy = 8, ext = False )
    cs.line( np.array( [ [ 4.25, 0, 3.8 ], [ 4.25, 0, 5.5 ] ] ), color = "#777777", lw = 0.5, z = 40 )
    cs.line( np.array( [ [ 6.75, 0, 3.8 ], [ 6.75, 0, 5.5 ] ] ), color = "#777777", lw = 0.5, z = 40 )
    bf.dim( cs, np.array( [ 7.3, 0, 3.8 ] ), np.array( [ 7.3, 0, 5.0 ] ), "HPROT", dx = 26, ext = False )
    bf.dim( cs, np.array( [ 7.3, 0, 3.0 ] ), np.array( [ 7.3, 0, 3.8 ] ), "OPROT", dx = 26, ext = False )
    bf.dim( cs, np.array( [ 3.4, 0, 0.0 ] ), np.array( [ 3.4, 0, 3.0 ] ), "RBODY", dx = -26, ext = False )
    bf.key( cs, "side", ( 1.0, 0.0 ), 30, 0 )
    cr = bf.canvas( fig, df.REAR )
    bf.fill( cr, df.circle( body, 5.5 ), face = df.BODY, edge = bf.POS_EDGE )
    for G in df.surfaces( p, 31, 161 ):
        cr.surface( G, color = df.PART, edge = False )
    cr.flush( 5.0 )
    cr.line( np.array( [ [ 0, 0, 0 ], [ 0, 0, 5.6 ] ] ), color = "#777777", lw = 0.6, ls = ( 0, ( 2, 2 ) ) )
    bf.dim( cr, np.array( [ 0, -1.0, 5.3 ] ), np.array( [ 0, 1.0, 5.3 ] ), "WPROT", dy = 8, ext = False )
    cr.line( np.array( [ [ 0, -1.0, 4.9 ], [ 0, -1.0, 5.5 ] ] ), color = "#777777", lw = 0.5, z = 40 )
    cr.line( np.array( [ [ 0, 1.0, 4.9 ], [ 0, 1.0, 5.5 ] ] ), color = "#777777", lw = 0.5, z = 40 )
    bf.key( cr, "rear", ( 1.0, 0.0 ), 30, 0 )
    bf.save( fig, "MdcProtub_dims", [ [ cs ], [ cr ] ], hmax = 6.5 )


def members():
    """MdcProtub_members: the members of a LUG and a SHOE, from behind."""
    fig = bf.figure()
    rows = []
    for ptype, names, tag in ( ( 3, [ "base plate", "post", "post", "top plate" ], "LUG" ),
                               ( 4, [ "base plate", "web", "top plate" ], "SHOE" ) ):
        bf.renew()
        body = df.cylinder( 6.0, -9.0, 9.0 )
        p = protub( ptype )
        c = bf.canvas( fig, df.REAR )
        C = df.circle( body, 0.0, n = 721 )
        C = C[ ( np.abs( C[ :, 1 ] ) < 2.6 ) & ( C[ :, 2 ] > 0 ) ]
        C = C[ np.argsort( C[ :, 1 ] ) ]
        poly = np.vstack( [ C, [ 0.0, C[ -1, 1 ], 4.6 ], [ 0.0, C[ 0, 1 ], 4.6 ] ] )
        bf.fill( c, poly, face = df.BODY, edge = bf.POS_EDGE )
        for G in df.surfaces( p, 41, 161 ):
            c.surface( G, color = df.PART, edge = False )
        c.flush( 5.0 )
        bf.title( c, tag, pad = 4 )
        rows.append( [ c ] )
        # Member m, numbered, with a leader from a point of it to a label beside the drawing: ( y, z ) of each.
        if ptype == 3:
            spots = [ ( ( -0.75, 6.05 ), ( -1.7, 6.5 ), "l" ), ( ( -0.375, 6.45 ), ( -1.7, 7.0 ), "l" ),
                      ( ( 0.375, 6.45 ), ( 1.7, 7.0 ), "r" ), ( ( 0.0, 6.675 ), ( 1.7, 7.5 ), "r" ) ]
        else:
            spots = [ ( ( -1.6, 6.05 ), ( -2.4, 6.6 ), "l" ), ( ( 0.0, 6.5 ), ( 1.4, 6.5 ), "r" ),
                      ( ( -0.4, 6.975 ), ( -2.4, 7.2 ), "l" ) ]
        for m, ( nm, ( q, e, side ) ) in enumerate( zip( names, spots ) ):
            c.line( np.array( [ [ 0.0, q[ 0 ], q[ 1 ] ], [ 0.0, e[ 0 ], e[ 1 ] ] ] ), color = "#444444", lw = 0.6, z = 60 )
            bf.label( c, np.array( [ 0.0, e[ 0 ], e[ 1 ] ] ), "%d  %s" % ( m + 1, nm ), ha = "right" if side == "l" else "left",
                      dx = -3 if side == "l" else 3 )
    bf.save( fig, "MdcProtub_members", rows, hmax = 6.0 )


FIGS = { "types": types, "dims": dims, "members": members, "phi": phi }

if __name__ == "__main__":
    for name in ( sys.argv[ 1: ] or FIGS ):
        FIGS[ name ]()
