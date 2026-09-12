# What the API answers when handed a Clone instead of the Geom it copies:
#
#   - a value that belongs to the shape: the same as the original;
#   - a value that depends on placement: from where the Clone is;
#   - a handle that can be written through: the original's ID, so writing changes the original
#     and every Clone of it.

import openvsp as vsp
import pytest

from clonehelp import ( box, switch, drop_errors, pop_errors, assert_refused,
                        assert_no_errors, a_route, a_polygon_mesh, scratch_output )
import os
import tempfile


def coords( pts ):
    return [ ( p.x(), p.y(), p.z() ) for p in pts ]


#==== Values that belong to the shape: the Clone answers the same ====#

def testACloneOfABodyOfRevolutionAnswersForItsCrossSection():
    """The section shape, and points and tangents computed on it, come with the shape."""
    vsp.VSPRenew()
    drop_errors()
    bor = vsp.AddGeom( "BODYOFREVOLUTION" )
    vsp.ChangeBORXSecShape( bor, vsp.XS_SUPER_ELLIPSE )
    vsp.Update()
    clone = vsp.CloneGeomVec( [ bor ] )[0]

    assert vsp.GetBORXSecShape( clone ) == vsp.XS_SUPER_ELLIPSE
    assert vsp.GetBORXSecShape( clone ) == vsp.GetBORXSecShape( bor )

    for fract in ( 0.0, 0.25, 0.6 ):
        on_bor = vsp.ComputeBORXSecPnt( bor, fract )
        on_clone = vsp.ComputeBORXSecPnt( clone, fract )
        assert ( on_clone.x(), on_clone.y(), on_clone.z() ) == \
               pytest.approx( ( on_bor.x(), on_bor.y(), on_bor.z() ) )

        tan_bor = vsp.ComputeBORXSecTan( bor, fract )
        tan_clone = vsp.ComputeBORXSecTan( clone, fract )
        assert ( tan_clone.x(), tan_clone.y(), tan_clone.z() ) == \
               pytest.approx( ( tan_bor.x(), tan_bor.y(), tan_bor.z() ) )
    assert_no_errors()


def testACloneOfABodyOfRevolutionAnswersForItsCstAirfoil():
    """CST degrees and coefficients describe the section, so they pass through."""
    vsp.VSPRenew()
    drop_errors()
    bor = vsp.AddGeom( "BODYOFREVOLUTION" )
    vsp.ChangeBORXSecShape( bor, vsp.XS_CST_AIRFOIL )
    vsp.Update()
    clone = vsp.CloneGeomVec( [ bor ] )[0]

    assert vsp.GetBORUpperCSTDegree( clone ) == vsp.GetBORUpperCSTDegree( bor )
    assert vsp.GetBORLowerCSTDegree( clone ) == vsp.GetBORLowerCSTDegree( bor )
    assert vsp.GetBORUpperCSTDegree( clone ) > 0, "no CST degree to compare"
    assert list( vsp.GetBORUpperCSTCoefs( clone ) ) == list( vsp.GetBORUpperCSTCoefs( bor ) )
    assert list( vsp.GetBORLowerCSTCoefs( clone ) ) == list( vsp.GetBORLowerCSTCoefs( bor ) )
    assert list( vsp.GetBORUpperCSTCoefs( clone ) ), "no CST coefficients to compare"
    assert_no_errors()


def testACloneOfABodyOfRevolutionAnswersForItsFileSection():
    """A section read from a file is still the section: the Clone hands back the same points."""
    vsp.VSPRenew()
    drop_errors()
    bor = vsp.AddGeom( "BODYOFREVOLUTION" )
    vsp.ChangeBORXSecShape( bor, vsp.XS_FILE_AIRFOIL )
    vsp.Update()
    clone = vsp.CloneGeomVec( [ bor ] )[0]

    upper = coords( vsp.GetBORAirfoilUpperPnts( bor ) )
    lower = coords( vsp.GetBORAirfoilLowerPnts( bor ) )
    assert upper and lower, "the file airfoil had no points"
    assert coords( vsp.GetBORAirfoilUpperPnts( clone ) ) == upper
    assert coords( vsp.GetBORAirfoilLowerPnts( clone ) ) == lower

    vsp.VSPRenew()
    drop_errors()
    bor = vsp.AddGeom( "BODYOFREVOLUTION" )
    vsp.ChangeBORXSecShape( bor, vsp.XS_FILE_FUSE )
    vsp.Update()
    clone = vsp.CloneGeomVec( [ bor ] )[0]

    pnts = coords( vsp.GetBORXSecPnts( bor ) )
    assert pnts, "the file section had no points"
    assert coords( vsp.GetBORXSecPnts( clone ) ) == pnts
    assert_no_errors()


def testACloneOfAPropellerAnswersForItsBladeCurves():
    """A blade curve is part of the propeller's shape, so a Clone reads the same one."""
    vsp.VSPRenew()
    drop_errors()
    prop = vsp.AddGeom( "PROP" )
    vsp.Update()
    clone = vsp.CloneGeomVec( [ prop ] )[0]

    for curve in ( vsp.PROP_CHORD, vsp.PROP_TWIST, vsp.PROP_THICK ):
        assert vsp.PCurveGetType( clone, curve ) == vsp.PCurveGetType( prop, curve )
        tvec = list( vsp.PCurveGetTVec( prop, curve ) )
        assert tvec, "the blade curve had no stations"
        assert list( vsp.PCurveGetTVec( clone, curve ) ) == tvec
        assert list( vsp.PCurveGetValVec( clone, curve ) ) == \
               list( vsp.PCurveGetValVec( prop, curve ) )
    assert_no_errors()


def testACloneOfAWingAnswersForItsSectionDrivers():
    """Which three of a wing section's dimensions drive it belongs to the wing."""
    vsp.VSPRenew()
    drop_errors()
    wing = vsp.AddGeom( "WING" )
    vsp.Update()
    clone = vsp.CloneGeomVec( [ wing ] )[0]

    drivers = list( vsp.GetDriverGroup( wing, 1 ) )
    assert drivers, "the wing section reported no drivers"
    assert list( vsp.GetDriverGroup( clone, 1 ) ) == drivers
    assert_no_errors()


def testACloneOfALandingGearAnswersForHowManyBogiesItHas():
    vsp.VSPRenew()
    drop_errors()
    gear = vsp.AddGeom( "GEAR" )
    vsp.Update()
    vsp.CreateAndAddBogie( gear )
    vsp.CreateAndAddBogie( gear )
    vsp.Update()
    clone = vsp.CloneGeomVec( [ gear ] )[0]

    assert vsp.GetNumBogies( gear ) == 2
    assert vsp.GetNumBogies( clone ) == 2
    assert_no_errors()


def testACloneOfARouteAnswersForHowManyPointsItHas():
    vsp.VSPRenew()
    drop_errors()
    route = a_route()
    clone = vsp.CloneGeomVec( [ route ] )[0]

    assert vsp.GetNumRoutingPts( route ) == 2
    assert vsp.GetNumRoutingPts( clone ) == 2
    assert_no_errors()


#==== Values that depend on placement: the Clone answers from where IT is ====#

def testACloneOfARouteGivesItsPointsWhereTheCloneStands():
    """Route point coordinates are positions, so a Clone reports its own."""
    vsp.VSPRenew()
    drop_errors()
    route = a_route()
    dz = 5.0
    clone = vsp.CloneGeomVec( [ route ] )[0]
    vsp.SetParmVal( vsp.FindParm( clone, "Z_Rel_Location", "XForm" ), dz )
    vsp.Update()

    on_route = coords( vsp.GetAllRoutingPtCoords( route, 0 ) )
    on_clone = coords( vsp.GetAllRoutingPtCoords( clone, 0 ) )
    assert len( on_route ) == 2, on_route
    assert len( on_clone ) == len( on_route )

    for a, b in zip( on_route, on_clone ):
        assert b[0] == pytest.approx( a[0] )
        assert b[1] == pytest.approx( a[1] )
        assert b[2] == pytest.approx( a[2] + dz )

    for i in range( len( on_route ) ):
        one = vsp.GetRoutingPtCoord( clone, i, 0 )
        assert ( one.x(), one.y(), one.z() ) == pytest.approx( on_clone[i] )

    curve_route = coords( vsp.GetRoutingCurve( route, 0 ) )
    curve_clone = coords( vsp.GetRoutingCurve( clone, 0 ) )
    assert curve_route, "the route drew no curve"
    assert len( curve_clone ) == len( curve_route )
    for a, b in zip( curve_route, curve_clone ):
        assert b[2] == pytest.approx( a[2] + dz )
    assert_no_errors()


#==== Handles that can be written through: the original's IDs ====#

def testACloneOfARouteHandsBackTheOriginalsPointIds():
    """The points themselves belong to the route -- there is one set of them, not two."""
    vsp.VSPRenew()
    drop_errors()
    route = a_route()
    clone = vsp.CloneGeomVec( [ route ] )[0]

    ids = list( vsp.GetAllRoutingPtIds( route ) )
    assert len( ids ) == 2, ids
    assert list( vsp.GetAllRoutingPtIds( clone ) ) == ids
    for i, pt in enumerate( ids ):
        assert vsp.GetRoutingPtID( clone, i ) == pt
    assert_no_errors()


def testACloneOfALandingGearHandsBackTheOriginalsBogieIds():
    vsp.VSPRenew()
    drop_errors()
    gear = vsp.AddGeom( "GEAR" )
    vsp.Update()
    vsp.CreateAndAddBogie( gear )
    vsp.Update()
    clone = vsp.CloneGeomVec( [ gear ] )[0]

    ids = list( vsp.GetAllBogies( gear ) )
    assert len( ids ) == 1, ids
    assert list( vsp.GetAllBogies( clone ) ) == ids
    assert_no_errors()


#==== The other direction: a Clone of the wrong thing still gets refused ====#

def testACloneOfAPodIsStillNotABodyOfRevolution():
    """Type-specific accessors still refuse a Clone of the wrong type."""
    vsp.VSPRenew()
    drop_errors()
    pod = vsp.AddGeom( "POD" )
    vsp.Update()
    clone = vsp.CloneGeomVec( [ pod ] )[0]

    from clonehelp import assert_refused
    drop_errors()
    vsp.GetBORXSecShape( clone )
    assert_refused( "body of revolution" )

    drop_errors()
    vsp.GetNumBogies( clone )
    assert_refused( "GearGeom" )

    drop_errors()
    vsp.GetNumRoutingPts( clone )
    assert_refused( "RoutingGeom" )

    drop_errors()
    vsp.PCurveGetType( clone, vsp.PROP_CHORD )
    assert_refused( "PCurve" )


#==== The suffix automatic naming appends ====#

def testANewCloneIsNamedWithTheDefaultSuffix():
    """A Clone made any way at all starts out called the original's name plus _Clone."""
    vsp.VSPRenew()
    drop_errors()
    pod = vsp.AddGeom( "POD" )
    vsp.SetGeomName( pod, "Fuselage" )
    vsp.Update()

    clone = vsp.CloneGeomVec( [ pod ] )[0]

    assert vsp.GetGeomCloneNameSuffix( clone ) == "_Clone"
    assert vsp.GetGeomName( clone ) == "Fuselage_Clone"
    assert_no_errors()


def testSettingTheSuffixRenamesTheClone():
    """The name is rebuilt on the next update, the way a rename of the original rebuilds it."""
    vsp.VSPRenew()
    drop_errors()
    pod = vsp.AddGeom( "POD" )
    vsp.SetGeomName( pod, "Fuselage" )
    vsp.Update()
    clone = vsp.CloneGeomVec( [ pod ] )[0]

    vsp.SetGeomCloneNameSuffix( clone, "_Left" )
    vsp.Update()

    assert vsp.GetGeomCloneNameSuffix( clone ) == "_Left"
    assert vsp.GetGeomName( clone ) == "Fuselage_Left"

    # The name still follows the original.
    vsp.SetGeomName( pod, "Body" )
    vsp.Update()
    assert vsp.GetGeomName( clone ) == "Body_Left"
    assert_no_errors()


def testAnEmptySuffixIsHonoured():
    """An empty suffix names the Clone exactly after the original."""
    vsp.VSPRenew()
    drop_errors()
    pod = vsp.AddGeom( "POD" )
    vsp.SetGeomName( pod, "Fuselage" )
    vsp.Update()
    clone = vsp.CloneGeomVec( [ pod ] )[0]

    vsp.SetGeomCloneNameSuffix( clone, "" )
    vsp.Update()

    assert vsp.GetGeomCloneNameSuffix( clone ) == ""
    assert vsp.GetGeomName( clone ) == "Fuselage"
    assert_no_errors()


def testTheSuffixSurvivesAFile():
    """The suffix is saved with the model."""
    vsp.VSPRenew()
    drop_errors()
    pod = vsp.AddGeom( "POD" )
    vsp.SetGeomName( pod, "Fuselage" )
    vsp.Update()
    clone = vsp.CloneGeomVec( [ pod ] )[0]
    vsp.SetGeomCloneNameSuffix( clone, "_Spare" )
    vsp.Update()

    path = os.path.join( tempfile.mkdtemp(), "suffix.vsp3" )
    vsp.WriteVSPFile( path, vsp.SET_ALL )
    vsp.VSPRenew()
    vsp.ReadVSPFile( path )
    vsp.Update()

    read = [ g for g in vsp.FindGeoms() if vsp.GetGeomTypeName( g ) == "Clone" ][0]
    assert vsp.GetGeomCloneNameSuffix( read ) == "_Spare"
    assert vsp.GetGeomName( read ) == "Fuselage_Spare"
    assert_no_errors()


def testAnEmptySuffixSurvivesAFileAsAnEmptySuffix():
    """An empty suffix reads back empty, not as the default."""
    vsp.VSPRenew()
    drop_errors()
    pod = vsp.AddGeom( "POD" )
    vsp.SetGeomName( pod, "Fuselage" )
    vsp.Update()
    clone = vsp.CloneGeomVec( [ pod ] )[0]
    vsp.SetGeomCloneNameSuffix( clone, "" )
    vsp.Update()

    path = os.path.join( tempfile.mkdtemp(), "emptysuffix.vsp3" )
    vsp.WriteVSPFile( path, vsp.SET_ALL )
    vsp.VSPRenew()
    vsp.ReadVSPFile( path )
    vsp.Update()

    read = [ g for g in vsp.FindGeoms() if vsp.GetGeomTypeName( g ) == "Clone" ][0]
    assert vsp.GetGeomCloneNameSuffix( read ) == ""
    assert vsp.GetGeomName( read ) == "Fuselage"
    assert_no_errors()


def testAFileWrittenBeforeTheSuffixExistedReadsAsTheOldDefault():
    """A file with no suffix node reads as _Clone."""
    vsp.VSPRenew()
    drop_errors()
    pod = vsp.AddGeom( "POD" )
    vsp.SetGeomName( pod, "Fuselage" )
    vsp.Update()
    clone = vsp.CloneGeomVec( [ pod ] )[0]
    vsp.SetGeomCloneNameSuffix( clone, "_Spare" )
    vsp.Update()

    path = os.path.join( tempfile.mkdtemp(), "old.vsp3" )
    vsp.WriteVSPFile( path, vsp.SET_ALL )

    # Remove the node to mimic an older file.
    text = open( path ).read()
    assert "NameSuffix" in text, "the suffix is not being written, so this test proves nothing"
    stripped = "\n".join( ln for ln in text.split( "\n" ) if "NameSuffix" not in ln )
    open( path, "w" ).write( stripped )

    vsp.VSPRenew()
    vsp.ReadVSPFile( path )
    vsp.Update()

    read = [ g for g in vsp.FindGeoms() if vsp.GetGeomTypeName( g ) == "Clone" ][0]
    assert vsp.GetGeomCloneNameSuffix( read ) == "_Clone"
    assert vsp.GetGeomName( read ) == "Fuselage_Clone"
    assert_no_errors()


def testCloningASelectionGivesEveryCloneTheOneSuffix():
    """Every Clone made from one selection gets the same suffix."""
    vsp.VSPRenew()
    drop_errors()
    pod = vsp.AddGeom( "POD" )
    vsp.SetGeomName( pod, "Fuselage" )
    wing = vsp.AddGeom( "WING", pod )
    vsp.SetGeomName( wing, "Main" )
    vsp.Update()

    clones = vsp.CloneGeomVec( [ pod, wing ], "_Left" )
    vsp.Update()

    assert len( clones ) == 2
    names = sorted( vsp.GetGeomName( c ) for c in clones )
    assert names == [ "Fuselage_Left", "Main_Left" ], names
    for c in clones:
        assert vsp.GetGeomCloneNameSuffix( c ) == "_Left"
    assert_no_errors()


def testCloneGeomVecStillDefaultsToTheOldSuffix():
    """With no suffix argument, CloneGeomVec uses _Clone."""
    vsp.VSPRenew()
    drop_errors()
    pod = vsp.AddGeom( "POD" )
    vsp.SetGeomName( pod, "Fuselage" )
    vsp.Update()

    clones = vsp.CloneGeomVec( [ pod ] )
    vsp.Update()

    assert vsp.GetGeomName( clones[0] ) == "Fuselage_Clone"
    assert_no_errors()


def testTheSuffixIsOnlyAskedOfAClone():
    """Get and Set both refuse a Geom that is not a Clone."""
    vsp.VSPRenew()
    drop_errors()
    pod = vsp.AddGeom( "POD" )
    vsp.Update()

    vsp.GetGeomCloneNameSuffix( pod )
    assert_refused( "is not a Clone" )

    drop_errors()
    vsp.SetGeomCloneNameSuffix( pod, "_Nope" )
    assert_refused( "is not a Clone" )


def testTurningAutomaticNamingOffLeavesTheNameAlone():
    """With AutoName off, setting the suffix does not rename the Clone."""
    vsp.VSPRenew()
    drop_errors()
    pod = vsp.AddGeom( "POD" )
    vsp.SetGeomName( pod, "Fuselage" )
    vsp.Update()
    clone = vsp.CloneGeomVec( [ pod ] )[0]

    switch( clone, "AutoName", False )
    vsp.SetGeomName( clone, "Hand Typed" )
    vsp.Update()

    vsp.SetGeomCloneNameSuffix( clone, "_Left" )
    vsp.Update()

    assert vsp.GetGeomName( clone ) == "Hand Typed"
    assert vsp.GetGeomCloneNameSuffix( clone ) == "_Left"
    assert_no_errors()


def testACloneWhoseOriginalIsDeletedSaysSoWithItsOwnCode():
    """Deleting a Clone's original raises VSP_CLONE_ORIGINAL_LOST and leaves the Clone."""
    vsp.VSPRenew()
    drop_errors()
    wing = vsp.AddGeom( "WING" )
    clone = vsp.CloneGeomVec( [ wing ] )[0]
    vsp.Update()

    # Drained before the delete, not after: the complaint is raised by the very update that
    # finds the original gone, so draining afterwards would throw away the thing being tested.
    drop_errors()
    vsp.DeleteGeom( wing )
    vsp.Update()

    mgr = vsp.ErrorMgrSingleton.getInstance()
    codes = []
    strings = []
    for _ in range( mgr.GetNumTotalErrors() ):
        err = mgr.PopLastError()
        codes.append( err.m_ErrorCode )
        strings.append( err.m_ErrorString )

    assert vsp.VSP_CLONE_ORIGINAL_LOST in codes, ( codes, strings )
    assert clone in vsp.FindGeoms(), "the Clone was removed rather than left to be repointed"
    assert vsp.GetGeomCloneOriginal( clone ) == "", "the Clone still claims an original"

def testChoosingAnOriginalThatHangsOffTheCloneIsRefused():
    """A Clone cannot take one of its own descendants as its original."""
    vsp.VSPRenew()
    drop_errors()
    pod = vsp.AddGeom( "POD" )
    clone = vsp.CloneGeomVec( [ pod ] )[0]
    vsp.Update()

    below = vsp.AddGeom( "POD", clone )
    vsp.Update()
    assert vsp.GetGeomParent( below ) == clone

    vsp.SetGeomCloneOriginal( clone, below )
    assert_refused( "hangs off" )
    assert vsp.GetGeomCloneOriginal( clone ) == pod, "the Clone took an original below itself"

def testAskingAGeomThatIsNotACloneWhatItCopiesIsRefused():
    """Get and Set of the original both refuse a Geom that is not a Clone."""
    vsp.VSPRenew()
    drop_errors()
    pod = vsp.AddGeom( "POD" )
    vsp.Update()

    assert vsp.GetGeomCloneOriginal( pod ) == "", "a Pod answered what it copies"
    assert_refused( "not a Clone" )

    vsp.SetGeomCloneOriginal( pod, pod )
    assert_refused( "not a Clone" )

def testCloningAListWithAnIdInItThatNamesNothingIsRefusedWhole():
    """One bad ID refuses the whole list; nothing is cloned."""
    vsp.VSPRenew()
    drop_errors()
    pod = vsp.AddGeom( "POD" )
    vsp.Update()
    before = len( vsp.FindGeoms() )

    made = vsp.CloneGeomVec( [ pod, "NOSUCHGEOM" ] )
    assert list( made ) == [], "a partial answer came back"
    assert_refused( "Can't Find Geom" )
    vsp.Update()
    assert len( vsp.FindGeoms() ) == before, "something was cloned anyway"

def testTheCloneButtonGivesACloneTheOriginalsViewProperties():
    """A Clone made by CloneGeomVec starts with the original's draw type."""
    vsp.VSPRenew()
    drop_errors()
    pod = vsp.AddGeom( "POD" )
    vsp.SetGeomDrawType( pod, vsp.GEOM_DRAW_TEXTURE )
    vsp.Update()

    made = vsp.CloneGeomVec( [ pod ] )
    vsp.Update()
    assert len( made ) == 1
    assert vsp.GetGeomDrawType( made[0] ) == vsp.GEOM_DRAW_TEXTURE, \
           "the Clone arrived wireframe beside a textured original"

    # Changing the Clone's does not change the original's.
    vsp.SetGeomDrawType( made[0], vsp.GEOM_DRAW_WIRE )
    vsp.Update()
    assert vsp.GetGeomDrawType( pod ) == vsp.GEOM_DRAW_TEXTURE, "the original followed the Clone"
    assert_no_errors()

def testAddingACloneFromTheMenuKeepsTheDefaults():
    """A Clone added with AddGeom takes its parent as original but keeps the default draw type."""
    vsp.VSPRenew()
    drop_errors()
    pod = vsp.AddGeom( "POD" )
    vsp.SetGeomDrawType( pod, vsp.GEOM_DRAW_TEXTURE )
    vsp.Update()

    added = vsp.AddGeom( "CLONE", pod )
    vsp.Update()
    assert vsp.GetGeomCloneOriginal( added ) == pod, "it did not take the Pod as its original"
    assert vsp.GetGeomDrawType( added ) == vsp.GEOM_DRAW_WIRE, \
           "adding a Clone took the original's view properties"
    assert_no_errors()



if __name__ == "__main__":
    for name, fn in sorted( list( globals().items() ) ):
        if name.startswith( "test" ) and callable( fn ):
            print( name )
            fn()
