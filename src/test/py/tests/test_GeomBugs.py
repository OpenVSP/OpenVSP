# Bugs in the Geoms themselves, found while the Clone work went on.  Most of these are
# pre-existing faults it walked into, kept here so they stay fixed.  Two are not bug guards:
# the landing gear box inflated by rotation came from this branch moving the bounding boxes
# ahead of placement, and its test guards that fix; the Matrix4d case pins arithmetic this
# branch relies on and does not change.

import openvsp as vsp
import pytest

import os
import tempfile
import glob
import xml.etree.ElementTree

def testAParmCanBeFoundByNameAndGroupAfterAFileLoad():
    """Every container except the Geom lost name-and-group lookup when a model was reopened.

    ParmContainer::DecodeXml builds the group-to-Parm map before decoding the Parms, and
    decoding a Parm remaps its ID, so every entry in that map named an ID that no longer
    existed.  A container walked as part of the linkable set has its map rebuilt by
    LinkMgr::BuildLinkableParmData and never noticed; a subsurface, a cross section, a texture,
    a mesh source, a bogie or a routing point kept the stale map for the life of the model.

    The symptom was that a Parm readable from a model just built was unreadable from the same
    model reopened -- which is what drove people to walk FindContainerParmIDs by hand and match
    on GetParmName.
    """
    vsp.VSPRenew()
    out = tempfile.mkdtemp()

    pod = vsp.AddGeom( "POD" )
    wing = vsp.AddGeom( "WING" )
    fuse = vsp.AddGeom( "FUSELAGE" )
    gear = vsp.AddGeom( "GEAR" )
    route = vsp.AddGeom( "ROUTING" )
    vsp.AddSubSurf( wing, vsp.SS_LINE )
    vsp.AddCFDSource( vsp.POINT_SOURCE, pod, 0, 0.5, 1.0, 0.5, 0.5 )
    vsp.CreateAndAddBogie( gear )
    vsp.AddRoutingPt( route, pod, 0 )
    vsp.Update()

    written = os.path.join( out, "containers.vsp3" )
    vsp.WriteVSPFile( written )
    vsp.VSPRenew()
    vsp.ReadVSPFile( written )
    vsp.Update()

    def find( tag, gid_type ):
        gid = [ g for g in vsp.FindGeoms() if vsp.GetGeomTypeName( g ) == gid_type ][0]
        return gid

    pod = find( "pod", "Pod" )
    wing = find( "wing", "Wing" )
    fuse = find( "fuse", "Fuselage" )
    gear = find( "gear", "Gear" )
    route = find( "route", "Routing" )

    containers = {
        "Geom":         pod,
        "XSec":         vsp.GetXSec( vsp.GetXSecSurf( fuse, 0 ), 1 ),
        "SubSurface":   vsp.GetSubSurfIDVec( wing )[0],
        "BaseSource":   vsp.GetCFDSourceID( pod, 0 ),
        "Bogie":        list( vsp.GetAllBogies( gear ) )[0],
        "RoutingPoint": list( vsp.GetAllRoutingPtIds( route ) )[0],
    }

    for tag, cid in containers.items():
        assert cid, "%s: nothing came back from its getter" % tag
        parms = list( vsp.FindContainerParmIDs( cid ) )
        assert parms, "%s: the container reports no Parms at all" % tag

        # Every Parm the container reports has to be findable by the name and group it
        # reports, which is the whole contract of the three-argument lookup.
        name = vsp.GetParmName( parms[0] )
        group = vsp.GetParmGroupName( parms[0] )
        found = vsp.FindParm( cid, name, group )
        assert found, "%s: FindParm could not find %s / %s after a file load" % ( tag, name, group )
        assert found == parms[0], "%s: FindParm found a different Parm" % tag

    
