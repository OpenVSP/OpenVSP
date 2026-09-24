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

    


@pytest.mark.parametrize( "preset", [ vsp.STACK_PRESET_FLOWTHRU_OUTLIP_ORIG,
                                      vsp.STACK_PRESET_FLOWTHRU_INLIP_ORIG,
                                      vsp.STACK_PRESET_FLOWTHRU_MID_ORIG ] )
def testAFlowThroughStackPresetClosesItsLoop( preset ):
    """A loop Stack's last cross section closes onto its first.

    The flow-through presets list their sections starting from wherever the loop begins, so the
    section that lands first carried a delta meant for its place further round -- 3.0, -2.0,
    -0.75.  The first section was held at the origin, but only once the rest of the chain had
    already been placed from it, so a freshly made preset stayed open by exactly that delta
    until the model was saved and reopened.
    """
    vsp.VSPRenew()
    sid = vsp.AddGeom( "STACK" )
    vsp.InitStackPreset( sid, preset )
    vsp.Update()

    xss = vsp.GetXSecSurf( sid, 0 )
    last = vsp.GetXSec( xss, vsp.GetNumXSec( xss ) - 1 )
    for name in ( "XAbs", "YAbs", "ZAbs" ):
        assert vsp.GetParmVal( vsp.GetXSecParm( last, name ) ) == pytest.approx( 0.0, abs=1e-12 ), name


def testAParmCanBeFoundByNameAndGroupAfterItsXSecChangesShape():
    """Changing a cross section's shape keeps its Parm IDs by swapping them onto the new cross
    section.  The swap moved the IDs but not the map each container finds its Parms by, so a
    lookup by name and group answered an ID that no longer belonged to it, while a lookup by
    name alone still worked."""
    vsp.VSPRenew()
    fid = vsp.AddGeom( "FUSELAGE" )
    vsp.Update()
    xss = vsp.GetXSecSurf( fid, 0 )
    xs = vsp.GetXSec( xss, 2 )
    before = { key: vsp.FindParm( xs, *key ) for key in ( ( "TopLStrength", "XSec" ), ( "SectTess_U", "XSec" ) ) }

    vsp.ChangeXSecShape( xss, 2, vsp.XS_SUPER_ELLIPSE )
    vsp.Update()
    xs = vsp.GetXSec( xss, 2 )

    for key, pid in before.items():
        assert vsp.ValidParm( pid ), key
        assert vsp.FindParm( xs, *key ) == pid, key


def testAttributesThatShareANameAreEachFound():
    """A collection may hold two attributes with the same name.  The search paired a list with
    one entry per attribute against a list with one entry per distinct name, so past the first
    repeat every attribute was matched against another's name -- and the last read ran off the
    end of the shorter list."""
    vsp.VSPRenew()
    pod = vsp.AddGeom( "POD" )
    coll = vsp.GetChildCollection( pod )
    first = vsp.AddAttributeString( coll, "Note", "first" )
    second = vsp.AddAttributeString( coll, "Note", "second" )
    other = vsp.AddAttributeString( coll, "Other", "third" )

    notes = [ vsp.FindAttributeInCollection( pod, "Note", i ) for i in range( 2 ) ]
    assert sorted( notes ) == sorted( [ first, second ] )
    assert vsp.FindAttributeInCollection( pod, "Other", 0 ) == other
