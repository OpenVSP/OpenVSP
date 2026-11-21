"""The model of the POGS guide's Figures 3 and 4: the small aircraft of the CFD Mesh page with a blunt trailing edge on
its wing, so that the faces Split and Join Surfaces joins on a body, a blunt trailing edge and a tip cap all show."""

import os
import sys

HERE = os.path.dirname( os.path.abspath( __file__ ) )
sys.path.insert( 0, os.path.join( HERE, "..", "cfdmesh" ) )
import openvsp as vsp
import cfdmodel as cm

TE_THICK = 0.04   # the wing's trailing edge thickness


def blunt_aircraft():
    """The small aircraft, its Wing given a blunt trailing edge TE_THICK thick.  Returns the IDs of the Fuselage, the
    Wing and the tail."""
    f, w, h = cm.aircraft()
    xss = vsp.GetXSecSurf( w, 0 )
    for i in range( vsp.GetNumXSec( xss ) ):
        x = vsp.GetXSec( xss, i )
        vsp.SetParmVal( vsp.GetXSecParm( x, "TE_Close_Type" ), vsp.CLOSE_SKEWBOTH )
        vsp.SetParmVal( vsp.GetXSecParm( x, "TE_Close_AbsRel" ), vsp.ABS )
        vsp.SetParmVal( vsp.GetXSecParm( x, "TE_Close_Thick" ), TE_THICK )
    vsp.Update()
    cm.coarse()
    return f, w, h
