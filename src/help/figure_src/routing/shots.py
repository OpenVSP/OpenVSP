"""Screenshots of the Routing component's Design tab for Routing.md.  See ../screenshots/snap.py."""
import os
import sys

HERE = os.path.dirname( os.path.abspath( __file__ ) )
sys.path.insert( 0, os.path.join( HERE, "..", "screenshots" ) )
import snap

import routing_figs


def model():
    """The route of Figure 1; the dump selects its first point."""
    routing_figs.model()


snap.shots( model, "ROUTING_GEOM_SCREEN", { "Routing_gui_points": ( "Routing_Design", ( 4, 55, 396, 582 ) ) },
            name = "routing" )
