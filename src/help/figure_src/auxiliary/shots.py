"""Screenshots of the Auxiliary's Design tab for Auxiliary.md.  See ../screenshots/snap.py."""
import os
import sys

HERE = os.path.dirname( os.path.abspath( __file__ ) )
sys.path.insert( 0, os.path.join( HERE, "..", "screenshots" ) )
sys.path.insert( 0, os.path.join( HERE, "..", "propeller" ) )
sys.path.insert( 0, os.path.join( HERE, "..", "gear" ) )
import snap
import openvsp as vsp
import gear_figs as gf


def model():
    """The tricycle gear of the page's figures with a 3pt Ground Plane under its nose bogie and both sides of its main
    bogie, the nose tire flat and its strut compressed."""
    g = gf.tricycle_gear()
    vsp.SetParmVal( g, "ShowNominalGroundPlane", "GroundPlane", 0 )
    nose, main = vsp.GetAllBogies( g )
    vsp.SetParmVal( vsp.FindParm( nose, "TravelCompressed", "Bogie" ), 0.4 )
    a = vsp.AddGeom( "AUXILIARY", g )
    vsp.SetParmVal( a, "AuxiliaryGeomType", "Design", vsp.AUX_GEOM_THREE_PT_GROUND )
    vsp.Update()
    vsp.SetAuxiliaryGeomContactPtID( a, 0, nose )
    vsp.SetAuxiliaryGeomContactPtID( a, 1, main )
    vsp.SetAuxiliaryGeomContactPtID( a, 2, main )
    vsp.SetParmVal( a, "ContactPt3_Isymm", "Design", 1 )
    vsp.SetParmVal( a, "ContactPt1_SuspensionMode", "Design", vsp.GEAR_SUSPENSION_COMPRESSED )
    vsp.SetParmVal( a, "ContactPt1_TireMode", "Design", vsp.TIRE_FLAT_CONTACT )
    vsp.Update()


snap.shots( model, "AUXILIARY_GEOM_SCREEN",
            { "Auxiliary_ground_plane_tab": ( "Auxiliary_Design", ( 4, 54, 396, 429 ) ) },
            name = "Auxiliary_ground" )
