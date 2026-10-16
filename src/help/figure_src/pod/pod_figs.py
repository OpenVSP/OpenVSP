"""Figure for Pod.md: the Pod's profile.

    <build>/venv/bin/python pod_figs.py

The drawing helpers are shared with the other body pages, in ../fuselagestack/bodyfig.py.
"""

import os
import sys

sys.path.insert( 0, os.path.join( os.path.dirname( os.path.abspath( __file__ ) ), "..", "fuselagestack" ) )

import numpy as np
import matplotlib.pyplot as plt

import bodyfig as bf
from bodyfig import vsp, FS
import figlib as fl


def profile_of( length, fine ):
    """x / L and r / R along the default Pod's profile."""
    bf.renew()
    p = vsp.AddGeom( "POD" )
    vsp.SetParmVal( p, "Length", "Design", length )
    vsp.SetParmVal( p, "FineRatio", "Design", fine )
    vsp.Update()
    G = bf.grid( p, 0, nu = 801, w = np.linspace( 0.0, 1.0, 9 ) )
    x = G[ :, 0, 0 ]
    r = np.hypot( G[ ..., 1 ], G[ ..., 2 ] ).max( 1 )
    R = length / fine
    assert abs( r.max() - R ) < 1e-9 * length
    return x / length, r / R


def stations( xl, rr ):
    """Where the radius reaches half and nine tenths of its greatest, ahead of and behind mid-length."""
    i = np.argmax( rr )
    out = {}
    for frac in ( 0.5, 0.9 ):
        out[ ( frac, "fwd" ) ] = np.interp( frac, rr[ :i + 1 ], xl[ :i + 1 ] )
        out[ ( frac, "aft" ) ] = np.interp( frac, rr[ i: ][ ::-1 ], xl[ i: ][ ::-1 ] )
    return out


def profile():
    """Pod_profile: radius along the length, the same for every Pod."""
    xl, rr = profile_of( 10.0, 15.0 )
    xl2, rr2 = profile_of( 3.0, 4.0 )
    assert np.max( np.abs( np.interp( xl, xl2, rr2 ) - rr ) ) < 1e-6        # one profile at every size
    st = stations( xl, rr )
    for k, v in sorted( st.items() ):
        print( "r/R = %.1f %s at x/L = %.3f" % ( k[0], k[1], v ) )
    bf.style()
    fig = plt.figure( figsize = ( fl.PAGE_W, 2.6 ) )
    ax = fig.add_axes( [ 0.09, 0.18, 0.88, 0.76 ] )
    ax.plot( xl, rr, color = "#333333", lw = 1.8 )
    ax.fill_between( xl, 0, rr, color = bf.POS )
    i = np.argmax( rr )
    ax.plot( [ xl[i], xl[i] ], [ 0, 1 ], color = "#999999", lw = 0.7, ls = ( 0, ( 2, 2 ) ) )
    ax.annotate( "greatest radius $R = L/\\mathrm{FineRatio}$", ( xl[i], 1.0 ), ( 0.62, 1.07 ), fontsize = FS,
                 arrowprops = dict( arrowstyle = "-|>", lw = 0.7, color = "k", mutation_scale = 6 ) )
    ax.set_xlim( 0, 1 )
    ax.set_ylim( 0, 1.2 )
    ax.set_xlabel( "$x/L$", fontsize = FS )
    ax.set_ylabel( "$r/R$", fontsize = FS )
    ax.set_xticks( np.linspace( 0, 1, 11 ) )
    ax.tick_params( labelsize = FS )
    for sp in ( "top", "right" ):
        ax.spines[ sp ].set_visible( False )
    bf.save_plot( fig, "Pod_profile" )


if __name__ == "__main__":
    profile()
