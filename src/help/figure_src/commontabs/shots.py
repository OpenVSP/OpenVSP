"""Screenshots of the common tabs for SubTab.md and XFormTab.md.  See ../screenshots/snap.py."""
import os
import sys

sys.path.insert( 0, os.path.join( os.path.dirname( os.path.abspath( __file__ ) ), "..", "screenshots" ) )
import snap
import openvsp as vsp


def wing_control():
    """The default Wing with the default Control_Surf."""
    wid = vsp.AddGeom( "WING" )
    vsp.AddSubSurf( wid, vsp.SS_CONTROL )
    vsp.Update()


def prop_surfaces():
    """The default Propeller with a Line on every blade and a Rectangle on Surf_1."""
    pid = vsp.AddGeom( "PROP" )
    vsp.AddSubSurf( pid, vsp.SS_LINE, -1 )
    vsp.AddSubSurf( pid, vsp.SS_RECTANGLE, 1 )
    vsp.Update()


def pod_on_wing():
    """A Pod child of the default Wing, attached by eta MN and Comp, mirrored in the wing's XZ plane."""
    wid = vsp.AddGeom( "WING" )
    pid = vsp.AddGeom( "POD", wid )
    vsp.SetParmVal( pid, "Trans_Attach_Flag", "Attach", vsp.ATTACH_TRANS_EtaMN )
    vsp.SetParmVal( pid, "Rots_Attach_Flag", "Attach", vsp.ATTACH_ROT_COMP )
    vsp.SetParmVal( pid, "Eta_Attach_Location", "Attach", 0.4 )
    vsp.SetParmVal( pid, "M_Attach_Location", "Attach", 1.0 )
    vsp.SetParmVal( pid, "N_Attach_Location", "Attach", 0.0 )
    vsp.SetParmVal( pid, "Sym_Ancestor", "Sym", 2 )
    vsp.SetParmVal( pid, "Sym_Ancestor_Origin_Flag", "Sym", 1 )
    vsp.SetParmVal( pid, "Sym_Planar_Flag", "Sym", vsp.SYM_XZ )
    vsp.Update()


def battery():
    """A battery Pod inside the default Fuselage."""
    vsp.AddGeom( "FUSELAGE" )
    pid = vsp.AddGeom( "POD" )
    vsp.SetGeomName( pid, "Battery" )
    vsp.SetParmVal( pid, "Length", "Design", 3.0 )
    vsp.SetParmVal( pid, "FineRatio", "Design", 2.0 )
    vsp.SetParmVal( pid, "X_Rel_Location", "XForm", 12.0 )
    vsp.Update()


snap.shots( wing_control, "MS_WING_GEOM_SCREEN", { "SubTab_gui_control": ( "Wing_Sub", ( 4, 222, 456, 603 ) ) },
            name = "commontabs_wing_control" )
snap.shots( prop_surfaces, "PROP_GEOM_SCREEN", { "SubTab_gui_list": ( "Propeller_Sub", ( 4, 54, 506, 219 ) ) },
            name = "commontabs_prop_surfaces" )
snap.shots( pod_on_wing, "POD_GEOM_SCREEN", { "XFormTab_gui_attach": ( "Pod_XForm", ( 4, 448, 396, 730 ) ),
                                              "XFormTab_gui_sym": ( "Pod_XForm", ( 4, 294, 396, 404 ) ) },
            name = "commontabs_pod_on_wing" )
snap.shots( battery, "VSP_SNAP_TO_SCREEN", { "XFormTab_gui_snapto": ( "Snap_To", ( 0, 0, 300, 366 ) ) },
            name = "commontabs_battery" )
