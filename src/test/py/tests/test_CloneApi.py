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



def testClearingTheOriginalHandsTheNameToTheUser():
    """Clearing the original turns AutoName off, so a typed name survives an Update."""
    vsp.VSPRenew()
    drop_errors()
    pod = vsp.AddGeom( "POD" )
    vsp.Update()
    clone = vsp.CloneGeomVec( [ pod ] )[0]
    vsp.Update()
    autoname = vsp.FindParm( clone, "AutoName", "Behavior" )
    assert vsp.GetParmVal( autoname ) == 1, "automatic naming starts off, so nothing is measured"

    vsp.SetGeomCloneOriginal( clone, "" )
    vsp.Update()
    assert vsp.GetParmVal( autoname ) == 0

    vsp.SetGeomName( clone, "Hand Typed" )
    vsp.Update()
    assert vsp.GetGeomName( clone ) == "Hand Typed"
    assert_no_errors()


#==== How a Clone is displayed ====#

#==== Replacing a Clone with the real thing ====#

def testAReplacementIsTheOriginalsTypeInTheClonesPlace():
    """The replacement stands where the Clone stood, under the Clone's name, and the Clone is gone."""
    vsp.VSPRenew()
    drop_errors()
    fuse = vsp.AddGeom( "FUSELAGE" )
    vsp.SetGeomName( fuse, "Body" )
    pod = vsp.AddGeom( "POD", fuse )
    vsp.SetGeomName( pod, "Nacelle" )
    vsp.Update()

    clone = vsp.AddGeom( "CLONE", fuse )
    vsp.SetGeomCloneOriginal( clone, pod )
    vsp.Update()
    name = vsp.GetGeomName( clone )

    order_before = vsp.GetGeomChildren( fuse )
    slot = order_before.index( clone )
    count_before = len( vsp.FindGeoms() )

    real = vsp.ReplaceCloneGeom( clone )
    vsp.Update()

    assert real, "no replacement was made"

    # The replacement keeps the Clone's ID, so references to the Clone still resolve.
    assert real == clone, "the replacement did not take the Clone's ID"

    assert vsp.GetGeomTypeName( real ) == "Pod"
    assert vsp.GetGeomName( real ) == name
    # The Geom count is unchanged and no Clone remains.
    assert len( vsp.FindGeoms() ) == count_before, "a Geom was left behind"
    assert "Clone" not in [ vsp.GetGeomTypeName( g ) for g in vsp.FindGeoms() ], "the Clone survived"
    assert vsp.GetGeomParent( real ) == fuse, "the replacement is not where the Clone was"

    # Same slot among its siblings.
    order_after = vsp.GetGeomChildren( fuse )
    assert order_after.index( real ) == slot, ( order_before, order_after )
    assert_no_errors()


def testWhatHungOffTheCloneHangsOffTheReplacement():
    """Children are re-parented rather than deleted with the Clone."""
    vsp.VSPRenew()
    drop_errors()
    pod = vsp.AddGeom( "POD" )
    vsp.Update()
    clone = vsp.CloneGeomVec( [ pod ] )[0]
    child = vsp.AddGeom( "POD", clone )
    vsp.SetGeomName( child, "Rider" )
    vsp.Update()

    assert vsp.GetGeomParent( child ) == clone

    real = vsp.ReplaceCloneGeom( clone )
    vsp.Update()

    assert child in vsp.FindGeoms(), "the child went with the Clone"
    assert vsp.GetGeomParent( child ) == real, "the child did not follow the replacement"
    assert_no_errors()


def testASwitchLeftOnLeavesTheOriginalsValue():
    """With every switch on, the replacement takes the original's values.

    The original is changed after the Clone exists, so the Clone is shown tracking it first.
    The switch-off case is tested below.
    """
    vsp.VSPRenew()
    drop_errors()
    pod = vsp.AddGeom( "POD" )
    vsp.Update()
    clone = vsp.CloneGeomVec( [ pod ] )[0]
    switch( clone, "CloneXForm", True )

    vsp.SetParmVal( vsp.FindParm( pod, "Y_Rel_Location", "XForm" ), 3.0 )
    vsp.SetParmVal( vsp.FindParm( pod, "Density", "Mass_Props" ), 7.0 )
    vsp.Update()

    assert abs( vsp.GetParmVal( vsp.FindParm( clone, "Y_Rel_Location", "XForm" ) ) - 3.0 ) < 1e-6, \
           "the Clone was not tracking the original, so this proves nothing about the replacement"

    real = vsp.ReplaceCloneGeom( clone )
    vsp.Update()

    assert abs( vsp.GetParmVal( vsp.FindParm( real, "Y_Rel_Location", "XForm" ) ) - 3.0 ) < 1e-6
    assert abs( vsp.GetParmVal( vsp.FindParm( real, "Density", "Mass_Props" ) ) - 7.0 ) < 1e-6
    assert_no_errors()


def testEverySwitchTurnedOffCarriesTheClonesOwnValue():
    """With each switch off, the replacement takes the Clone's own value.

    One check per switch, since each governs its own list of Parms.
    """
    vsp.VSPRenew()
    drop_errors()
    pod = vsp.AddGeom( "POD" )
    vsp.SetParmVal( vsp.FindParm( pod, "Y_Rel_Location", "XForm" ), 3.0 )
    vsp.SetParmVal( vsp.FindParm( pod, "Z_Rel_Rotation", "XForm" ), 10.0 )
    vsp.SetParmVal( vsp.FindParm( pod, "Density", "Mass_Props" ), 7.0 )
    vsp.SetParmVal( vsp.FindParm( pod, "Sym_Planar_Flag", "Sym" ), vsp.SYM_XZ )
    vsp.SetParmVal( vsp.FindParm( pod, "Negative_Volume_Flag", "Negative_Volume_Props" ), 0.0 )
    vsp.SetSetFlag( pod, 4, True )
    vsp.Update()

    clone = vsp.CloneGeomVec( [ pod ] )[0]
    for name in ( "CloneXForm", "CloneSym", "CloneMassProps", "CloneNegativeVolume",
                  "CloneSets", "CloneAppearance" ):
        switch( clone, name, False )

    vsp.SetParmVal( vsp.FindParm( clone, "Y_Rel_Location", "XForm" ), -5.0 )
    vsp.SetParmVal( vsp.FindParm( clone, "Z_Rel_Rotation", "XForm" ), 42.0 )
    vsp.SetParmVal( vsp.FindParm( clone, "Density", "Mass_Props" ), 2.0 )
    vsp.SetParmVal( vsp.FindParm( clone, "Sym_Planar_Flag", "Sym" ), 0.0 )
    vsp.SetParmVal( vsp.FindParm( clone, "Negative_Volume_Flag", "Negative_Volume_Props" ), 1.0 )
    vsp.SetSetFlag( clone, 4, False )
    vsp.SetSetFlag( clone, 5, True )
    vsp.SetGeomWireColor( clone, 11, 22, 33 )
    vsp.Update()

    real = vsp.ReplaceCloneGeom( clone )
    vsp.Update()

    assert abs( vsp.GetParmVal( vsp.FindParm( real, "Y_Rel_Location", "XForm" ) ) + 5.0 ) < 1e-6, "transform"
    assert abs( vsp.GetParmVal( vsp.FindParm( real, "Z_Rel_Rotation", "XForm" ) ) - 42.0 ) < 1e-6, "rotation"
    assert abs( vsp.GetParmVal( vsp.FindParm( real, "Density", "Mass_Props" ) ) - 2.0 ) < 1e-6, "mass properties"
    assert abs( vsp.GetParmVal( vsp.FindParm( real, "Sym_Planar_Flag", "Sym" ) ) ) < 1e-6, "symmetry"
    assert abs( vsp.GetParmVal( vsp.FindParm( real, "Negative_Volume_Flag", "Negative_Volume_Props" ) ) - 1.0 ) < 1e-6, "negative volume"
    assert vsp.GetSetFlag( real, 4 ) == False and vsp.GetSetFlag( real, 5 ) == True, "set membership"

    # Appearance is not stored in Parms, so check it directly.
    colour = vsp.GetGeomWireColor( real )
    assert ( colour.x(), colour.y(), colour.z() ) == ( 11.0, 22.0, 33.0 ), "appearance"
    assert_no_errors()


def testAnAttachmentTheCloneHeldOfItsOwnCarries():
    """Attachment is its own switch and its own list of Parms."""
    vsp.VSPRenew()
    drop_errors()
    fuse = vsp.AddGeom( "FUSELAGE" )
    pod = vsp.AddGeom( "POD", fuse )
    vsp.Update()
    clone = vsp.AddGeom( "CLONE", fuse )
    vsp.SetGeomCloneOriginal( clone, pod )
    switch( clone, "CloneAttach", False )
    vsp.SetParmVal( vsp.FindParm( clone, "Trans_Attach_Flag", "Attach" ), vsp.ATTACH_TRANS_UV )
    vsp.SetParmVal( vsp.FindParm( clone, "U_Attach_Location", "Attach" ), 0.35 )
    vsp.Update()

    real = vsp.ReplaceCloneGeom( clone )
    vsp.Update()

    assert vsp.GetParmVal( vsp.FindParm( real, "Trans_Attach_Flag", "Attach" ) ) == vsp.ATTACH_TRANS_UV
    assert abs( vsp.GetParmVal( vsp.FindParm( real, "U_Attach_Location", "Attach" ) ) - 0.35 ) < 1e-6
    assert_no_errors()


def testTheClonesOwnSubSurfacesCarryToTheReplacement():
    """With CloneSubSurfs off, the Clone's own subsurfaces move to the replacement with their IDs.

    The IDs matter: VSPAERO control surface groups name subsurfaces by ID.
    """
    vsp.VSPRenew()
    drop_errors()
    wing = vsp.AddGeom( "WING" )
    vsp.AddSubSurf( wing, vsp.SS_CONTROL )
    vsp.Update()
    clone = vsp.CloneGeomVec( [ wing ] )[0]
    switch( clone, "CloneSubSurfs", False )
    vsp.Update()

    # One copied before the switch went off, plus one added to the Clone.
    vsp.AddSubSurf( clone, vsp.SS_LINE )
    vsp.Update()
    assert vsp.GetNumSubSurf( clone ) == 2
    assert vsp.GetNumSubSurf( wing ) == 1

    was = sorted( vsp.GetSubSurfIDVec( clone ) )
    names_was = sorted( vsp.GetSubSurfName( ss ) for ss in was )

    real = vsp.ReplaceCloneGeom( clone )
    vsp.Update()

    assert vsp.GetNumSubSurf( real ) == 2, "the replacement did not take the Clone's own subsurfaces"
    types = sorted( vsp.GetSubSurfType( ss ) for ss in vsp.GetSubSurfIDVec( real ) )
    assert types == sorted( [ vsp.SS_CONTROL, vsp.SS_LINE ] ), types

    # The same subsurfaces, not re-made ones.
    assert sorted( vsp.GetSubSurfIDVec( real ) ) == was, "the subsurfaces were re-made, not handed over"
    assert sorted( vsp.GetSubSurfName( ss ) for ss in vsp.GetSubSurfIDVec( real ) ) == names_was

    # The original keeps its own.
    assert vsp.GetNumSubSurf( wing ) == 1
    assert_no_errors()


def testTheReplacementStandsWhereTheCloneStood():
    """The replacement, under a parent, stands where the Clone stood."""
    vsp.VSPRenew()
    drop_errors()
    fuse = vsp.AddGeom( "FUSELAGE" )
    vsp.SetParmVal( vsp.FindParm( fuse, "X_Rel_Location", "XForm" ), 2.0 )
    pod = vsp.AddGeom( "POD" )
    vsp.Update()

    clone = vsp.AddGeom( "CLONE", fuse )
    vsp.SetGeomCloneOriginal( clone, pod )
    vsp.SetParmVal( vsp.FindParm( clone, "X_Rel_Location", "XForm" ), 1.5 )
    vsp.SetParmVal( vsp.FindParm( clone, "Z_Rel_Location", "XForm" ), 0.75 )
    vsp.Update()

    before = box( clone )

    real = vsp.ReplaceCloneGeom( clone )
    vsp.Update()

    after = box( real )
    for i in range( len( before ) ):
        assert abs( after[i] - before[i] ) < 1e-6, ( before, after )
    assert_no_errors()


def testAFlippedCloneIsReplacedByAFlippedGeom():
    """The replacement takes the Clone's Flip_Flag."""
    vsp.VSPRenew()
    drop_errors()
    pod = vsp.AddGeom( "POD" )
    vsp.Update()
    clone = vsp.CloneGeomVec( [ pod ] )[0]
    vsp.SetParmVal( vsp.FindParm( clone, "Flip_Flag", "Sym" ), vsp.SYM_XZ )
    vsp.Update()

    real = vsp.ReplaceCloneGeom( clone )
    vsp.Update()

    assert real, "a flipped Clone was not replaced at all"
    assert vsp.GetGeomTypeName( real ) == "Pod"
    assert vsp.GetParmVal( vsp.FindParm( real, "Flip_Flag", "Sym" ) ) == vsp.SYM_XZ
    assert_no_errors()


def testReplaceRefusesWhatIsNotACloneOrHasNothingToCopy():
    vsp.VSPRenew()
    drop_errors()
    pod = vsp.AddGeom( "POD" )
    vsp.Update()

    vsp.ReplaceCloneGeom( pod )
    assert_refused( "is not a Clone" )

    drop_errors()
    bare = vsp.AddGeom( "CLONE" )
    vsp.SetGeomCloneOriginal( bare, "" )
    vsp.Update()
    drop_errors()
    vsp.ReplaceCloneGeom( bare )
    assert_refused( "no original" )


def testACloneOfTheReplacedCloneNowShowsARealGeom():
    """After replacing the inner Clone, an outer Clone of it shows a real Geom.

    The outer Clone's original ID does not change, so the check is on the type it resolves to.
    """
    vsp.VSPRenew()
    drop_errors()
    pod = vsp.AddGeom( "POD" )
    vsp.Update()
    inner = vsp.CloneGeomVec( [ pod ] )[0]
    outer = vsp.AddGeom( "CLONE" )
    vsp.SetGeomCloneOriginal( outer, inner )
    vsp.Update()

    assert vsp.GetGeomCloneOriginal( outer ) == inner
    assert vsp.GetGeomTypeName( inner ) == "Clone", "the middle Geom should be a Clone to start"

    real = vsp.ReplaceCloneGeom( inner )
    vsp.Update()

    assert vsp.GetGeomCloneOriginal( outer ) == inner, "the ID the outer Clone holds should not move"
    assert vsp.GetGeomTypeName( real ) == "Pod", "what the outer Clone shows is still a Clone"
    assert vsp.GetGeomTypeName( outer ) == "Clone", "the outer Clone should still be a Clone"

    # The outer Clone still shows a shape.
    assert box( outer ) != ( 0.0, 0.0, 0.0, 0.0, 0.0, 0.0 ), "the outer Clone shows nothing"
    assert_no_errors()



def testTheReplacementKeepsTheClonesIdentity():
    """The replacement keeps the Clone's Geom and Parm IDs, so a child and a design variable
    still point at it.
    """
    vsp.VSPRenew()
    drop_errors()
    pod = vsp.AddGeom( "POD" )
    vsp.Update()
    clone = vsp.CloneGeomVec( [ pod ] )[0]
    vsp.SetParmVal( vsp.FindParm( clone, "Y_Rel_Location", "XForm" ), 6.0 )
    vsp.Update()

    rider = vsp.AddGeom( "POD", clone )
    vsp.Update()

    zloc = vsp.FindParm( clone, "Z_Rel_Location", "XForm" )
    assert zloc, "could not find the Parm to build a design variable on"
    vsp.AddDesignVar( zloc, vsp.XDDM_VAR )
    assert vsp.GetNumDesignVars() == 1

    real = vsp.ReplaceCloneGeom( clone )
    vsp.Update()

    #==== The Geom ID ====#
    assert real == clone

    #==== The child ====#
    assert vsp.GetGeomParent( rider ) == real, "the attached Geom lost what it was attached to"

    #==== The Parm ID and the design variable ====#
    assert vsp.FindParm( real, "Z_Rel_Location", "XForm" ) == zloc, "the Parm ID did not carry across"
    assert vsp.GetNumDesignVars() == 1, "the design variable was lost"
    assert vsp.GetDesignVar( 0 ) == zloc, "the design variable no longer names the same Parm"

    #==== The Clone's own value ====#
    assert abs( vsp.GetParmVal( vsp.FindParm( real, "Y_Rel_Location", "XForm" ) ) - 6.0 ) < 1e-6
    assert_no_errors()


def testTheSubSurfaceIdentitiesCarryWhenCopyingWasOn():
    """With CloneSubSurfs on, the replacement's subsurfaces take the IDs of the Clone's copies,
    so VSPAERO control surface groups survive.
    """
    vsp.VSPRenew()
    drop_errors()
    wing = vsp.AddGeom( "WING" )
    vsp.AddSubSurf( wing, vsp.SS_CONTROL )
    vsp.Update()
    clone = vsp.CloneGeomVec( [ wing ] )[0]
    vsp.Update()

    assert vsp.GetNumSubSurf( clone ) == 1, "the Clone did not copy the subsurface"
    mirror = vsp.GetSubSurfIDVec( clone )[0]
    mirror_parms = sorted( vsp.GetSubSurfParmIDs( mirror ) )

    real = vsp.ReplaceCloneGeom( clone )
    vsp.Update()

    assert vsp.GetNumSubSurf( real ) == 1
    assert vsp.GetSubSurfIDVec( real )[0] == mirror, \
           "the replacement's subsurface did not take the mirror's ID"
    assert sorted( vsp.GetSubSurfParmIDs( vsp.GetSubSurfIDVec( real )[0] ) ) == mirror_parms, \
           "the subsurface's Parm IDs did not carry across"

    # The original's subsurface is untouched.
    assert vsp.GetNumSubSurf( wing ) == 1
    assert vsp.GetSubSurfIDVec( wing )[0] != mirror
    assert_no_errors()


def testNothingOfTheClonesOwnParmsLeaksIntoTheReplacement():
    """The ID swap matches Parms by group and name, so Clone-only Parms must be in groups no
    other Geom has.
    """
    vsp.VSPRenew()
    drop_errors()
    pod = vsp.AddGeom( "POD" )
    vsp.Update()
    clone = vsp.CloneGeomVec( [ pod ] )[0]
    vsp.Update()

    clone_groups = set()
    for p in vsp.GetGeomParmIDs( clone ):
        clone_groups.add( vsp.GetParmGroupName( p ) )

    pod_groups = set()
    for p in vsp.GetGeomParmIDs( pod ):
        pod_groups.add( vsp.GetParmGroupName( p ) )

    only_clone = clone_groups - pod_groups
    assert "Behavior" in only_clone, only_clone

    # Shared groups hold no Clone-only Parm names.
    shared = clone_groups & pod_groups
    for g in shared:
        cn = set( vsp.GetParmName( p ) for p in vsp.GetGeomParmIDs( clone )
                  if vsp.GetParmGroupName( p ) == g )
        pn = set( vsp.GetParmName( p ) for p in vsp.GetGeomParmIDs( pod )
                  if vsp.GetParmGroupName( p ) == g )
        extra = cn - pn
        assert extra == set(), ( g, extra )
    assert_no_errors()



def testTheJointDeflectionACloneHeldOfItsOwnCarries():
    """With CloneJoint off, the replacement hinge takes the Clone's own deflection."""
    vsp.VSPRenew()
    drop_errors()
    hinge = vsp.AddGeom( "HINGE" )

    # Translation is off by default; turn it on so it can be measured.
    vsp.SetParmVal( vsp.FindParm( hinge, "JointTranslateFlag", "Hinge" ), 1.0 )
    vsp.Update()

    clone = vsp.CloneGeomVec( [ hinge ] )[0]
    switch( clone, "CloneJoint", False )
    vsp.SetParmVal( vsp.FindParm( clone, "JointRotate", "Hinge" ), 17.0 )
    vsp.SetParmVal( vsp.FindParm( clone, "JointTranslate", "Hinge" ), 0.25 )
    vsp.Update()

    real = vsp.ReplaceCloneGeom( clone )
    vsp.Update()

    assert vsp.GetGeomTypeName( real ) == "Hinge", vsp.GetGeomTypeName( real )
    assert abs( vsp.GetParmVal( vsp.FindParm( real, "JointRotate", "Hinge" ) ) - 17.0 ) < 1e-6, \
           "the replacement hinge was not posed to the angle the Clone held"
    assert abs( vsp.GetParmVal( vsp.FindParm( real, "JointTranslate", "Hinge" ) ) - 0.25 ) < 1e-6
    assert_no_errors()


def testReplaceReturnsAnEmptyStringWhenItRefuses():
    """A refused replacement returns an empty string."""
    vsp.VSPRenew()
    drop_errors()
    pod = vsp.AddGeom( "POD" )
    vsp.Update()

    assert vsp.ReplaceCloneGeom( pod ) == "", "a refusal returned something"
    assert_refused( "is not a Clone" )

    drop_errors()
    bare = vsp.AddGeom( "CLONE" )
    vsp.SetGeomCloneOriginal( bare, "" )
    vsp.Update()
    drop_errors()
    assert vsp.ReplaceCloneGeom( bare ) == "", "a refusal returned something"
    assert_refused( "no original" )


def testAFlippedClonesReplacementIsTheFlippedShape():
    """The replacement has the Clone's flipped shape, where the Clone stood."""
    vsp.VSPRenew()
    drop_errors()
    # Symmetry off: a symmetric wing looks the same flipped about XZ.
    wing = vsp.AddGeom( "WING" )
    vsp.SetParmVal( vsp.FindParm( wing, "Sym_Planar_Flag", "Sym" ), 0.0 )
    vsp.Update()
    plain_box = box( wing )

    clone = vsp.CloneGeomVec( [ wing ] )[0]
    vsp.Update()
    assert box( clone ) == pytest.approx( plain_box ), "the Clone does not stand on the wing"

    vsp.SetParmVal( vsp.FindParm( clone, "Flip_Flag", "Sym" ), vsp.SYM_XZ )
    vsp.Update()
    assert box( clone ) != pytest.approx( plain_box ), "the flip did nothing, so this proves nothing"

    flipped_box = box( clone )

    real = vsp.ReplaceCloneGeom( clone )
    vsp.Update()
    assert_no_errors()

    assert box( real ) == pytest.approx( flipped_box )
    assert vsp.GetParmVal( vsp.FindParm( real, "Flip_Flag", "Sym" ) ) == vsp.SYM_XZ


def testAnAttributeOnACloneSurvivesItsReplacement():
    """An attribute on the Clone moves to the replacement with the same collection and IDs."""
    vsp.VSPRenew()
    drop_errors()
    wing = vsp.AddGeom( "WING" )
    clone = vsp.CloneGeomVec( [ wing ] )[0]
    vsp.Update()

    # The collection has its own ID.
    coll = vsp.GetChildCollection( clone )
    assert coll, "the Clone has no attribute collection to attach to"
    attr = vsp.AddAttributeString( coll, "MyAttr", "kept" )
    assert attr, "the attribute was not attached, so this proves nothing"
    assert "MyAttr" in vsp.FindAttributeNamesInCollection( coll )

    real = vsp.ReplaceCloneGeom( clone )
    vsp.Update()
    assert_no_errors()

    assert vsp.GetChildCollection( real ) == coll, "the collection took a new ID"
    assert list( vsp.FindAttributesInCollection( coll ) ) == [ attr ], \
           "the attribute took a new ID, or was duplicated rather than carried"
    assert list( vsp.GetAttributeStringVal( attr ) ) == [ "kept" ]


def testReplacingAFlippedCloneReportsACleanCall():
    """Nothing is lost, so a script reading the last-call flag sees a clean call."""
    vsp.VSPRenew()
    drop_errors()
    wing = vsp.AddGeom( "WING" )
    vsp.SetParmVal( vsp.FindParm( wing, "Sym_Planar_Flag", "Sym" ), 0.0 )
    clone = vsp.CloneGeomVec( [ wing ] )[0]
    vsp.SetParmVal( vsp.FindParm( clone, "Flip_Flag", "Sym" ), vsp.SYM_XZ )
    vsp.Update()

    real = vsp.ReplaceCloneGeom( clone )
    assert real, "the replacement was not made, so the flag is not what this measures"
    mgr = vsp.ErrorMgrSingleton.getInstance()
    assert not mgr.GetErrorLastCallFlag(), pop_errors()


def testACloneWhoseOriginalIsDeletedSaysSoWithItsOwnCode():
    """Deleting a Clone's original raises VSP_CLONE_ORIGINAL_LOST and leaves the Clone."""
    vsp.VSPRenew()
    drop_errors()
    wing = vsp.AddGeom( "WING" )
    clone = vsp.CloneGeomVec( [ wing ] )[0]
    vsp.Update()

    # Drain before the delete, which raises the error under test.
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


def testACloneOfACloneIsReplacedWhereItStood():
    """Replacing a Clone of a Clone uses the values the outer Clone shows, not the far end's.

    The middle link holds its own position and density.
    """
    vsp.VSPRenew()
    drop_errors()
    pod = vsp.AddGeom( "POD" )

    inner = vsp.CloneGeomVec( [ pod ] )[0]
    switch( inner, "CloneMassProps", False )
    vsp.SetParmVal( vsp.FindParm( inner, "X_Rel_Location", "XForm" ), 10.0 )
    vsp.SetParmVal( vsp.FindParm( inner, "Density", "Mass_Props" ), 7.0 )
    vsp.Update()

    # Both switches are off, or this measures nothing.
    assert vsp.GetParmVal( vsp.FindParm( inner, "X_Rel_Location", "XForm" ) ) == pytest.approx( 10.0 )
    assert vsp.GetParmVal( vsp.FindParm( inner, "Density", "Mass_Props" ) ) == pytest.approx( 7.0 )

    # The outer link copies the middle one.
    outer = vsp.CloneGeomVec( [ inner ] )[0]
    assert vsp.GetParmVal( vsp.FindParm( outer, "X_Rel_Location", "XForm" ) ) == pytest.approx( 10.0 )
    assert vsp.GetParmVal( vsp.FindParm( outer, "Density", "Mass_Props" ) ) == pytest.approx( 7.0 )

    real = vsp.ReplaceCloneGeom( outer )
    vsp.Update()
    assert_no_errors()

    assert vsp.GetParmVal( vsp.FindParm( real, "X_Rel_Location", "XForm" ) ) == pytest.approx( 10.0 ), \
           "the replacement took the far end of the chain's position, not the Clone's"
    assert vsp.GetParmVal( vsp.FindParm( real, "Density", "Mass_Props" ) ) == pytest.approx( 7.0 ), \
           "the replacement took the far end of the chain's density"


def testACloneOfACloneKeepsTheSubsurfacesItWasShowing():
    """Replacing a Clone of a Clone keeps the subsurfaces it shows, not the far end's."""
    vsp.VSPRenew()
    drop_errors()
    wing = vsp.AddGeom( "WING" )
    vsp.AddSubSurf( wing, vsp.SS_LINE )
    vsp.Update()

    # The middle link stops copying and deletes its subsurface.
    inner = vsp.CloneGeomVec( [ wing ] )[0]
    vsp.Update()
    assert vsp.GetNumSubSurf( inner ) == 1, "the Clone did not copy the subsurface"
    switch( inner, "CloneSubSurfs", False )
    vsp.Update()
    for ss in list( vsp.GetSubSurfIDVec( inner ) ):
        vsp.DeleteSubSurf( inner, ss )
    vsp.Update()
    assert vsp.GetNumSubSurf( inner ) == 0

    outer = vsp.AddGeom( "CLONE" )
    vsp.SetGeomCloneOriginal( outer, inner )
    vsp.Update()
    assert vsp.GetNumSubSurf( outer ) == 0, "the Clone of a Clone showed one that was not there"

    real = vsp.ReplaceCloneGeom( outer )
    vsp.Update()
    assert vsp.GetNumSubSurf( real ) == 0, \
           "the replacement gained a subsurface the Clone was not showing"


def testTheOriginalDoesNotKeepTheReplacementAsAStepChild():
    """The original does not list the replacement as a step child.

    The entry would hold the replacement's ID, which keeps resolving, so it would never be pruned.
    """
    vsp.VSPRenew()
    drop_errors()
    out = scratch_output()
    pod = vsp.AddGeom( "POD" )
    clone = vsp.CloneGeomVec( [ pod ] )[0]
    vsp.Update()

    real = vsp.ReplaceCloneGeom( clone )
    vsp.Update()
    assert_no_errors()

    written = os.path.join( out, "stepchild.vsp3" )
    vsp.WriteVSPFile( written )
    text = open( written ).read()

    assert real not in text.split( "<Step_Child_List>" )[-1].split( "</Step_Child_List>" )[0] \
           if "<Step_Child_List>" in text else True, \
           "the original still lists the replacement as a step child"

    # No step-child list anywhere names the replacement.
    for chunk in text.split( "<Step_Child_List>" )[1:]:
        assert real not in chunk.split( "</Step_Child_List>" )[0], \
               "the original still lists the replacement as a step child"


def testWhetherTheCloneWasShownSurvivesReplacement():
    """A hidden Clone gives a hidden replacement.  No switch governs the show flags."""
    vsp.VSPRenew()
    drop_errors()
    pod = vsp.AddGeom( "POD" )
    clone = vsp.CloneGeomVec( [ pod ] )[0]
    vsp.Update()

    vsp.SetSetFlag( clone, vsp.SET_SHOWN, False )
    vsp.SetSetFlag( clone, vsp.SET_NOT_SHOWN, True )
    vsp.Update()
    assert not vsp.GetSetFlag( clone, vsp.SET_SHOWN ), "the Clone would not hide"

    real = vsp.ReplaceCloneGeom( clone )
    vsp.Update()
    assert not vsp.GetSetFlag( real, vsp.SET_SHOWN ), "the replacement came back shown"
    assert vsp.GetSetFlag( real, vsp.SET_NOT_SHOWN )

    # The original stays shown.
    assert vsp.GetSetFlag( pod, vsp.SET_SHOWN )


def testReplacingACloneOfABlankLeavesNoFlip():
    """A Blank has no flip, so replacing a Clone of one that carries Flip_Flag gives a Blank
    whose Flip_Flag is 0, and a clean call."""
    vsp.VSPRenew()
    drop_errors()
    blank = vsp.AddGeom( "BLANK" )
    clone = vsp.CloneGeomVec( [ blank ] )[0]
    vsp.Update()

    vsp.SetParmVal( vsp.FindParm( clone, "Flip_Flag", "Sym" ), vsp.SYM_XZ )
    vsp.Update()
    assert vsp.GetParmVal( vsp.FindParm( clone, "Flip_Flag", "Sym" ) ) == vsp.SYM_XZ
    drop_errors()

    real = vsp.ReplaceCloneGeom( clone )
    assert real, "the replacement was not made"
    assert not vsp.ErrorMgrSingleton.getInstance().GetErrorLastCallFlag()
    vsp.Update()
    assert vsp.GetGeomTypeName( real ) == "Blank"
    assert vsp.GetParmVal( vsp.FindParm( real, "Flip_Flag", "Sym" ) ) == 0
    assert_no_errors()


def testMeshSourcesOnTheCloneSurviveReplacement():
    """The Clone's own mesh sources move to the replacement; the original's do not."""
    vsp.VSPRenew()
    drop_errors()
    pod = vsp.AddGeom( "POD" )
    vsp.AddCFDSource( vsp.POINT_SOURCE, pod, 0, 0.5, 1.0, 0.5, 0.5 )
    clone = vsp.CloneGeomVec( [ pod ] )[0]
    vsp.Update()

    # Different counts, so the count tells which set survived.
    vsp.AddCFDSource( vsp.LINE_SOURCE, clone, 0, 0.25, 2.0, 0.1, 0.1, 0.35, 3.0, 0.9, 0.9 )
    vsp.AddCFDSource( vsp.POINT_SOURCE, clone, 0, 0.4, 1.5, 0.2, 0.2 )
    vsp.Update()
    assert vsp.GetNumCFDSources( clone ) == 2, "the sources did not land on the Clone"
    assert vsp.GetNumCFDSources( pod ) == 1
    sources = [ vsp.GetCFDSourceID( clone, i ) for i in range( 2 ) ]

    real = vsp.ReplaceCloneGeom( clone )
    vsp.Update()
    assert vsp.GetNumCFDSources( real ) == 2, \
           "the Clone's mesh sources were lost, or the original's arrived in their place"
    assert [ vsp.GetCFDSourceID( real, i ) for i in range( 2 ) ] == sources, \
           "the Clone's mesh sources were made again rather than handed over"
    assert vsp.GetNumCFDSources( pod ) == 1, "the original's source moved"



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


def testAnAttributeOnAClonesParmSurvivesItsReplacement():
    """An attribute on one of the Clone's Parms moves to the replacement's Parm."""
    vsp.VSPRenew()
    drop_errors()
    pod = vsp.AddGeom( "POD" )
    clone = vsp.CloneGeomVec( [ pod ] )[0]
    vsp.Update()

    pid = vsp.FindParm( clone, "X_Rel_Location", "XForm" )
    coll = vsp.GetChildCollection( pid )
    attr = vsp.AddAttributeString( coll, "ParmNote", "mine" )
    vsp.Update()
    assert list( vsp.FindAttributesInCollection( coll ) ) == [ attr ]

    real = vsp.ReplaceCloneGeom( clone )
    vsp.Update()
    assert_no_errors()

    same = vsp.FindParm( real, "X_Rel_Location", "XForm" )
    assert same == pid, "the Parm did not keep its ID, so this measures nothing"
    assert vsp.GetChildCollection( same ) == coll, "the Parm's collection took a new ID"
    assert list( vsp.FindAttributesInCollection( coll ) ) == [ attr ], \
           "the attribute on the Clone's Parm was lost, or duplicated rather than moved"


def testHowTheCloneWasBeingShownSurvivesItsReplacement():
    """The replacement keeps the Clone's draw type.

    These view properties are not Parms, and a paste alone would give a wireframe Geom.
    """
    vsp.VSPRenew()
    drop_errors()
    pod = vsp.AddGeom( "POD" )
    clone = vsp.CloneGeomVec( [ pod ] )[0]
    vsp.Update()

    vsp.SetGeomDrawType( clone, vsp.GEOM_DRAW_SHADE )
    vsp.Update()
    assert vsp.GetGeomDrawType( clone ) == vsp.GEOM_DRAW_SHADE, "the Clone would not change"

    real = vsp.ReplaceCloneGeom( clone )
    vsp.Update()
    assert_no_errors()

    assert vsp.GetGeomDrawType( real ) == vsp.GEOM_DRAW_SHADE, \
           "the replacement came back wireframe"

    # The original is untouched.
    assert vsp.GetGeomDrawType( pod ) == vsp.GEOM_DRAW_WIRE

    # Display type is not tested: a Clone always follows the original's, so carried or not
    # would read the same.


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


def testReplacingACloneLeavesTheAttributeClipboardAlone():
    """Whatever the user last copied is still there to paste after a replacement."""
    vsp.VSPRenew()
    drop_errors()
    pod = vsp.AddGeom( "POD" )
    clone = vsp.CloneGeomVec( [ pod ] )[0]
    vsp.AddAttributeString( vsp.GetChildCollection( clone ), "ClonesNote", "clone" )
    mine = vsp.AddAttributeString( vsp.GetChildCollection( pod ), "MyNote", "mine" )
    assert vsp.CopyAttribute( mine ) == 0
    vsp.Update()

    vsp.ReplaceCloneGeom( clone )
    vsp.Update()

    pasted = vsp.PasteAttribute( vsp.GetChildCollection( pod ) )
    assert [ vsp.GetAttributeName( a ) for a in pasted ] == [ "MyNote" ]
    assert_no_errors()


def testAReplacementCarriesWhatTheCloneHeldAndCopiesTheRest():
    """Attributes on the Clone and its common Parms come from the Clone; shape attributes come
    from the original.  One on a Clone-only Parm moves to the Geom."""
    vsp.VSPRenew()
    drop_errors()
    fuse = vsp.AddGeom( "FUSELAGE" )
    vsp.Update()
    on_fuse = { "Geom": fuse,
                "X_Rel_Location": vsp.FindParm( fuse, "X_Rel_Location", "XForm" ),
                "Length": vsp.FindParm( fuse, "Length", "Design" ),
                "XSec": vsp.GetXSec( vsp.GetXSecSurf( fuse, 0 ), 1 ) }
    for name, obj in on_fuse.items():
        vsp.AddAttributeString( vsp.GetChildCollection( obj ), "Orig_" + name, name )

    clone = vsp.CloneGeomVec( [ fuse ] )[0]
    note = vsp.AddAttributeString( vsp.GetChildCollection( clone ), "ClonesNote", "clone" )
    switch_parm = vsp.FindParm( clone, "CloneXForm", "Behavior" )
    behavior = vsp.AddAttributeString( vsp.GetChildCollection( switch_parm ), "SwitchNote", "b" )
    vsp.Update()

    real = vsp.ReplaceCloneGeom( clone )
    vsp.Update()
    assert_no_errors()

    counts = [ len( vsp.FindAttributesByName( "Orig_" + name ) ) for name in on_fuse ]
    assert counts == [ 1, 1, 2, 2 ], counts
    assert sorted( vsp.FindAttributesInCollection( vsp.GetChildCollection( real ) ) ) == \
           sorted( [ note, behavior ] )


def testARouteThroughACloneFollowsItsReplacement():
    """A route through a Clone follows the replacement when it moves."""
    vsp.VSPRenew()
    drop_errors()
    pod = vsp.AddGeom( "POD" )
    vsp.Update()
    clone = vsp.CloneGeomVec( [ pod ] )[0]
    vsp.SetParmVal( vsp.FindParm( clone, "Y_Rel_Location", "XForm" ), 5.0 )
    vsp.Update()
    route = vsp.AddGeom( "ROUTING" )
    for u in ( 0.1, 0.9 ):
        pt = vsp.AddRoutingPt( route, clone, 0 )
        vsp.SetParmVal( vsp.FindParm( pt, "U", "RoutePt" ), u )
    vsp.Update()

    real = vsp.ReplaceCloneGeom( clone )
    vsp.Update()
    vsp.SetParmVal( vsp.FindParm( real, "Y_Rel_Location", "XForm" ), 9.0 )
    vsp.Update()

    assert [ p.y() for p in vsp.GetAllRoutingPtCoords( route, 0 ) ] == pytest.approx( [ 9.0, 9.0 ] )
    assert_no_errors()


def testAReplacementThatCannotStandWhereTheCloneDidSaysSo():
    """A Clone of a route moved off the original is replaced at the original's position, with an
    error and the last-call flag set."""
    vsp.VSPRenew()
    drop_errors()
    original = a_route()
    clone = vsp.CloneGeomVec( [ original ] )[0]
    vsp.SetParmVal( vsp.FindParm( clone, "X_Rel_Location", "XForm" ), 1.0 )
    vsp.SetParmVal( vsp.FindParm( clone, "Y_Rel_Location", "XForm" ), 2.0 )
    vsp.SetParmVal( vsp.FindParm( clone, "Z_Rel_Location", "XForm" ), 3.0 )
    vsp.Update()
    drop_errors()

    real = vsp.ReplaceCloneGeom( clone )
    assert real == clone
    assert vsp.ErrorMgrSingleton.getInstance().GetErrorLastCallFlag(), \
           "the replacement reported a problem and the call reported clean"
    assert_refused( "places its own shape" )


def testACloneOfAPolygonMeshIsNotReplaced():
    """A polygon mesh cannot be copied, so replacing a Clone of one is refused and the Clone stays."""
    vsp.VSPRenew()
    drop_errors()
    original = a_polygon_mesh()
    clone = vsp.CloneGeomVec( [ original ] )[0]
    vsp.Update()
    drop_errors()

    assert vsp.ReplaceCloneGeom( clone ) == ""
    assert vsp.GetGeomTypeName( clone ) == "Clone"
    assert vsp.GetGeomCloneOriginal( clone ) == original
    assert_refused( "polygon mesh" )


@pytest.mark.parametrize( "make", [ a_route, lambda: vsp.AddGeom( "POD" ) ] )
def testAReplacementThatStandsWhereTheCloneDidReportsACleanCall( make ):
    """A Clone standing on its original is replaced cleanly."""
    vsp.VSPRenew()
    drop_errors()
    original = make()
    vsp.Update()
    clone = vsp.CloneGeomVec( [ original ] )[0]
    vsp.Update()
    drop_errors()

    real = vsp.ReplaceCloneGeom( clone )
    assert real == clone
    assert not vsp.ErrorMgrSingleton.getInstance().GetErrorLastCallFlag(), pop_errors()
    vsp.Update()
    assert_no_errors()


def testATextureOnACloneShowingItsOriginalsIsKept():
    """With CloneAppearance on, a texture attached to the Clone is not drawn but is kept on the
    replacement, after the original's."""
    vsp.VSPRenew()
    drop_errors()
    pod = vsp.AddGeom( "POD" )
    vsp.AttachGeomTexture( pod, "orig.png" )
    clone = vsp.CloneGeomVec( [ pod ] )[0]
    switch( clone, "CloneAppearance", True )
    mine = vsp.AttachGeomTexture( clone, "mine.png" )
    vsp.Update()

    real = vsp.ReplaceCloneGeom( clone )
    vsp.Update()

    textures = list( vsp.GetGeomTextureIDVec( real ) )
    assert len( textures ) == 2 and textures[1] == mine, textures
    drop_errors()


def testTheDisplayTypeOfACloneWithAnOriginalIsRefused():
    """A Clone with an original is displayed the way its original is, so setting its display
    type is refused with VSP_WRONG_GEOM_TYPE and leaves it unchanged."""
    vsp.VSPRenew()
    drop_errors()
    pod = vsp.AddGeom( "POD" )
    clone = vsp.CloneGeomVec( [ pod ] )[0]
    vsp.Update()
    before = vsp.GetGeomDisplayType( clone )
    assert before != vsp.DISPLAY_DEGEN_SURF, "the type asked for is the one it has"

    vsp.SetGeomDisplayType( clone, vsp.DISPLAY_DEGEN_SURF )
    vsp.Update()
    assert vsp.GetGeomDisplayType( clone ) == before

    mgr = vsp.ErrorMgrSingleton.getInstance()
    codes = [ mgr.PopLastError().m_ErrorCode for _ in range( mgr.GetNumTotalErrors() ) ]
    assert codes == [ vsp.VSP_WRONG_GEOM_TYPE ]


def testTheDisplayTypeOfACloneWithNoOriginalCanBeSet():
    """A Clone with no original has no display type to follow, so its own can be set."""
    vsp.VSPRenew()
    drop_errors()
    clone = vsp.AddGeom( "CLONE" )
    vsp.SetGeomCloneOriginal( clone, "" )
    vsp.Update()
    drop_errors()
    assert vsp.GetGeomDisplayType( clone ) != vsp.DISPLAY_DEGEN_SURF, \
           "the type asked for is the one it has"

    vsp.SetGeomDisplayType( clone, vsp.DISPLAY_DEGEN_SURF )
    vsp.Update()
    assert vsp.GetGeomDisplayType( clone ) == vsp.DISPLAY_DEGEN_SURF
    assert_no_errors()


if __name__ == "__main__":
    for name, fn in sorted( list( globals().items() ) ):
        if name.startswith( "test" ) and callable( fn ):
            print( name )
            fn()
