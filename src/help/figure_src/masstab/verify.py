"""Check what MassTab.md states beyond ../commontabs/verify.py against the openvsp module.

    <build>/venv/bin/python verify.py      # prints PASS or FAIL for each statement; always exits 0
"""

import masstab_figs as m
from masstab_figs import vsp, setp


def check( ok, what ):
    print( ( "PASS  " if ok else "FAIL  " ) + what )


def priority():
    skin = m.skin_alone()
    fuel = m.alone( m.tank, m.TANK[ "Density" ] )
    tol = 0.02 * fuel
    _, _, low = m.model( 0 )
    _, _, high = m.model( 1 )
    check( abs( low - skin ) < tol, "a Density 0 body first at equal Priority counts a tank inside it at Density 0" )
    check( abs( high - ( skin + fuel ) ) < tol, "the tank at the higher Priority counts at its own Density" )


def hidden_shell():
    vsp.VSPRenew()
    t = m.tank()
    setp( t, "Density", "Mass_Props", 0 )
    setp( t, "Shell_Flag", "Mass_Props", 1 )
    vsp.Update()
    alone = m.total_mass()
    vsp.VSPRenew()
    b = m.body()
    setp( b, "Shell_Flag", "Mass_Props", 0 )
    t = m.tank()
    setp( t, "Density", "Mass_Props", 0 )
    setp( t, "Shell_Flag", "Mass_Props", 1 )
    vsp.Update()
    inside = m.total_mass()
    check( alone > 1.0 and abs( inside ) < 1e-9, "a thin shell wholly inside another Geom adds no mass" )


def clone_greys():
    vsp.VSPRenew()
    p = vsp.AddGeom( "POD" )
    vsp.Update()
    c = vsp.AddGeom( "CLONE" )
    vsp.SetGeomCloneOriginal( c, p )
    vsp.Update()
    check( vsp.GetParmVal( c, "CloneMassProps", "Behavior" ) == 1, "a Clone copies its mass properties by default" )
    ids = [ vsp.FindParm( c, n, "Mass_Props" ) for n in ( "Density", "Mass_Prior", "Shell_Flag", "PointMass" ) ]
    check( all( i and not vsp.GetParmActiveFlag( i ) for i in ids ),
           "a Clone copying its original's mass properties greys the Mass tab" )


for fn in ( priority, hidden_shell, clone_greys ):
    fn()
