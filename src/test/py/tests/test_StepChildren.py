# A Geom that depends on another one without hanging off it.
#
# A step child is how that is recorded: the Geom being depended on keeps a list of the Geoms
# that depend on it, and updates each of them when it changes.  A detached conformal, a routing
# point sitting on a surface and a landing gear's stow point are all step children.
#
# The list is saved with the model and re-added by whoever depends on the Geom, so adding is
# idempotent; and the registration is rebuilt when a hash of the Geoms depended on, together
# with the depending Geom's own ID, says it has gone stale.

import openvsp as vsp
import os
import re
import tempfile


def fresh():
    vsp.VSPRenew()
    out = tempfile.mkdtemp()
    vsp.SetComputationFileName( vsp.COMP_GEOM_TXT_TYPE, os.path.join( out, "comp_geom.txt" ) )
    pop_errors()
    return out


def pop_errors():
    mgr = vsp.ErrorMgrSingleton.getInstance()
    return [ mgr.PopLastError().m_ErrorString for _ in range( mgr.GetNumTotalErrors() ) ]



def step_child_lists( path ):
    """Every step child list in the file, as lists of IDs."""
    text = open( path ).read()
    out = []
    for listing in re.findall( r"<Step_Child_List>(.*?)</Step_Child_List>", text, re.S ):
        out.append( re.findall( r"<ID>(\w+)</ID>", listing ) )
    return [ ids for ids in out if ids ]




def a_pod_and_route():
    """A routing geom anchored to a pod, which makes the route a step child of the pod."""
    pod = vsp.AddGeom( "POD" )
    vsp.Update()
    route = vsp.AddGeom( "ROUTING" )
    for u in ( 0.25, 0.75 ):
        pt = vsp.AddRoutingPt( route, pod, 0 )
        vsp.SetParmVal( vsp.FindParm( pt, "U", "RoutePt" ), u )
    vsp.Update()
    return pod, route


def testAStepChildListDoesNotGrowOnEverySaveAndOpen():
    """The list is saved and re-added on every update, so a Geom is listed once."""
    out = fresh()
    pod = vsp.AddGeom( "POD" )
    vsp.Update()
    vsp.AddGeom( "CONFORMAL", pod )
    a_pod_and_route()
    vsp.Update()

    written = os.path.join( out, "step.vsp3" )
    sizes = []
    for cycle in range( 5 ):
        vsp.WriteVSPFile( written )
        vsp.VSPRenew()
        vsp.ReadVSPFile( written )
        vsp.Update()
        sizes.append( max( len( ids ) for ids in step_child_lists( written ) ) )

    assert sizes[0] > 0, "nothing in this model holds a step child, so nothing is measured"
    assert sizes == [ sizes[0] ] * len( sizes ), "the list grew: %s" % sizes
    pop_errors()


def testAStepChildListThatIsAlreadyTooLongComesBackPruned():
    """A list that already holds duplicates comes back with one entry per Geom."""
    out = fresh()
    pod = vsp.AddGeom( "POD" )
    vsp.Update()
    vsp.AddGeom( "CONFORMAL", pod )
    vsp.Update()

    written = os.path.join( out, "step.vsp3" )
    vsp.WriteVSPFile( written )
    text = open( written ).read()

    def triple( match ):
        return "<Step_Child_List>" + match.group( 1 ) * 3 + "</Step_Child_List>"

    bloated = re.sub( r"<Step_Child_List>(.*?)</Step_Child_List>", triple, text, flags = re.S )
    assert bloated != text, "this model wrote no step children, so nothing is measured"
    bad = os.path.join( out, "step_bloated.vsp3" )
    open( bad, "w" ).write( bloated )

    vsp.VSPRenew()
    vsp.ReadVSPFile( bad )
    vsp.Update()
    vsp.WriteVSPFile( written )

    for ids in step_child_lists( written ):
        assert len( ids ) == len( set( ids ) ), "duplicates survived the read: %s" % ids
    pop_errors()




if __name__ == "__main__":
    for name, fn in sorted( list( globals().items() ) ):
        if name.startswith( "test" ) and callable( fn ):
            print( name )
            fn()
