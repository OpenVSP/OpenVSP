"""A skinning parameter that is switched off must not shape the surface.

Values whose Set flag is off are not the user's.  SetUnsetParms writes the achieved values
into them so the GUI can report what the loft did, which makes them outputs.  For a long time
they were inputs as well: a blending pass enforcing a quantity everywhere needed a value for
it at stations that never specified one, and read these.  An off value then steered the loft,
the value that built the surface was overwritten by the readback, and the surface stopped
being a function of the parms -- two different surfaces could carry identical parms, and only
a save and reload restored the correspondence.

Both halves are checked here, over random per side and spine configurations:

  - perturbing every off parm must not move the surface at all, and
  - saving and reloading must give back exactly what was on screen.

The second is the sharper of the two.  It fails for any value that reaches the surface
without being stored, however it got there.
"""

import math
import os
import random

import pytest

import openvsp as vsp

VALUES = [ "LAngle", "LSlew", "LStrength", "LCurve", "RAngle", "RSlew", "RStrength", "RCurve" ]
EQUALS = [ "LRAngleEq", "LRSlewEq", "LRStrengthEq", "LRCurveEq" ]
SIDES = [ "Top", "Bottom", "Left", "Right" ]


def sample( gid ):
    pts = []
    for i in range( 25 ):
        for j in range( 37 ):
            p = vsp.CompPnt01( gid, 0, i / 24.0, j / 36.0 )
            pts.append( ( p.x(), p.y(), p.z() ) )
    return pts


def deviation( a, b ):
    return max( math.dist( x, y ) for x, y in zip( a, b ) )


def off_parms( gid, xsurf ):
    """Every parm whose Set flag is off -- on the four sides and on any spine."""
    out = []
    for i in range( vsp.GetNumXSec( xsurf ) ):
        xsec = vsp.GetXSec( xsurf, i )
        for side in SIDES:
            for v in VALUES:
                if vsp.GetParmVal( vsp.GetXSecParm( xsec, side + v + "Set" ) ) < 0.5:
                    out.append( vsp.GetXSecParm( xsec, side + v ) )
        for k in range( vsp.GetNumSkinSpines( gid ) ):
            spine = vsp.GetSkinSpineID( xsec, k )
            for v in VALUES:
                if vsp.GetParmVal( vsp.GetSkinSpineParm( spine, v + "Set" ) ) < 0.5:
                    out.append( vsp.GetSkinSpineParm( spine, v ) )
    return out


def build_random( rng, geom_type ):
    """A Geom with random per side conditions and a few spines, symmetry off so the sides
    are free to disagree."""
    gid = vsp.AddGeom( geom_type )
    xsurf = vsp.GetXSecSurf( gid, 0 )

    for _ in range( rng.randint( 0, 3 ) ):
        vsp.AddSkinSpine( gid, rng.uniform( 0.0125, 0.9875 ) )
    vsp.Update()

    for i in range( vsp.GetNumXSec( xsurf ) ):
        xsec = vsp.GetXSec( xsurf, i )

        # Symmetry, continuity and the Equal flags all make ValidateParms cascade -- they turn
        # Set flags on that the user did not.  Randomize them: the station grouping keys on
        # those flags, and an ordering mistake anywhere in that chain shows up here.
        # Which frame the angles are measured from is per cross section, so a body may mix
        # them.  Randomized here rather than tested apart, because the properties below have
        # to hold whichever frame a section chose.
        vsp.SetParmVal( vsp.GetXSecParm( xsec, "CurveBasis" ), float( rng.randint( 0, 1 ) ) )

        for nm in [ "AllSym", "TBSym", "RLSym" ]:
            vsp.SetParmVal( vsp.GetXSecParm( xsec, nm ), float( rng.randint( 0, 1 ) ) )
        vsp.SetParmVal( vsp.GetXSecParm( xsec, "ContinuityTop" ), float( rng.randint( 0, 2 ) ) )

        for side in SIDES:
            for nm in EQUALS:
                vsp.SetParmVal( vsp.GetXSecParm( xsec, side + nm ), float( rng.randint( 0, 1 ) ) )
            for lr in [ "L", "R" ]:
                vsp.SetParmVal( vsp.GetXSecParm( xsec, side + lr + "AngleSet" ), float( rng.randint( 0, 1 ) ) )
                vsp.SetParmVal( vsp.GetXSecParm( xsec, side + lr + "CurveSet" ), float( rng.randint( 0, 1 ) ) )
                vsp.SetParmVal( vsp.GetXSecParm( xsec, side + lr + "Angle" ), rng.uniform( -40.0, 40.0 ) )
                vsp.SetParmVal( vsp.GetXSecParm( xsec, side + lr + "Strength" ), rng.uniform( 0.4, 1.6 ) )
                vsp.SetParmVal( vsp.GetXSecParm( xsec, side + lr + "Curve" ), rng.uniform( -1.0, 1.0 ) )

        for k in range( vsp.GetNumSkinSpines( gid ) ):
            spine = vsp.GetSkinSpineID( xsec, k )
            for nm in EQUALS:
                vsp.SetParmVal( vsp.GetSkinSpineParm( spine, nm ), float( rng.randint( 0, 1 ) ) )
            vsp.SetParmVal( vsp.GetSkinSpineParm( spine, "LRSym" ), float( rng.randint( 0, 1 ) ) )
            vsp.SetParmVal( vsp.GetSkinSpineParm( spine, "TBSym" ), float( rng.randint( 0, 1 ) ) )
            for lr in [ "L", "R" ]:
                vsp.SetParmVal( vsp.GetSkinSpineParm( spine, lr + "AngleSet" ), float( rng.randint( 0, 1 ) ) )
                vsp.SetParmVal( vsp.GetSkinSpineParm( spine, lr + "CurveSet" ), float( rng.randint( 0, 1 ) ) )

    vsp.Update()
    return gid, xsurf


def test_off_parms_do_not_shape_the_surface( tmp_path ):
    rng = random.Random( 20260825 )

    for trial in range( 24 ):
        vsp.VSPRenew()
        geom_type = [ "STACK", "FUSELAGE" ][ trial % 2 ]
        gid, xsurf = build_random( rng, geom_type )

        before = sample( gid )

        for p in off_parms( gid, xsurf ):
            vsp.SetParmVal( p, vsp.GetParmVal( p ) + rng.uniform( 3.0, 9.0 ) )
        vsp.Update()

        moved = deviation( before, sample( gid ) )
        assert moved == 0.0, \
            "trial %d (%s): perturbing off parms moved the surface by %.3e" % ( trial, geom_type, moved )


def test_surface_survives_a_save_and_reload( tmp_path ):
    rng = random.Random( 987 )
    path = os.path.join( str( tmp_path ), "skin_round_trip.vsp3" )

    for trial in range( 24 ):
        vsp.VSPRenew()
        geom_type = [ "STACK", "FUSELAGE" ][ trial % 2 ]
        gid, xsurf = build_random( rng, geom_type )

        # Nudge the off parms first: that is what used to leave the surface and the parms
        # describing different things.
        for p in off_parms( gid, xsurf ):
            vsp.SetParmVal( p, vsp.GetParmVal( p ) + rng.uniform( 3.0, 9.0 ) )
        vsp.Update()
        onscreen = sample( gid )

        vsp.WriteVSPFile( path )
        vsp.VSPRenew()
        vsp.ReadVSPFile( path )
        vsp.Update()

        # Not bit equality: the blend sums one solution per pass, and floating point addition
        # is not associative, so a last bit can move without anything being wrong.  The
        # tolerance is far below anything geometric and far above one ulp, so a value
        # actually reaching the surface unstored still fails loudly.
        moved = deviation( onscreen, sample( vsp.FindGeoms()[0] ) )
        assert moved < 1.0e-12, \
            "trial %d (%s): save and reload moved the surface by %.3e" % ( trial, geom_type, moved )


def test_a_spine_enforcing_nothing_anywhere_changes_nothing():
    """A spine that enforces nothing on any XSec has no say, and is dropped from the station
    list -- not from the Geom, which is why the count below is still 1.  One that is inert on
    a single XSec is a different thing: it asks for a free loft there, and is left alone."""
    vsp.VSPRenew()
    gid = vsp.AddGeom( "STACK" )
    xsurf = vsp.GetXSecSurf( gid, 0 )

    xsec = vsp.GetXSec( xsurf, 2 )
    for nm in [ "AllSym", "TBSym", "RLSym" ]:
        vsp.SetParmVal( vsp.GetXSecParm( xsec, nm ), 0.0 )
    vsp.SetParmVal( vsp.GetXSecParm( xsec, "TopLAngleSet" ), 1.0 )
    vsp.SetParmVal( vsp.GetXSecParm( xsec, "TopLAngle" ), 30.0 )
    vsp.Update()

    before = sample( gid )

    vsp.AddSkinSpine( gid, 0.15 )
    vsp.Update()
    for i in range( vsp.GetNumXSec( xsurf ) ):
        spine = vsp.GetSkinSpineID( vsp.GetXSec( xsurf, i ), 0 )
        for nm in EQUALS:
            vsp.SetParmVal( vsp.GetSkinSpineParm( spine, nm ), 0.0 )
        for v in VALUES:
            vsp.SetParmVal( vsp.GetSkinSpineParm( spine, v + "Set" ), 0.0 )
    vsp.Update()

    assert vsp.GetNumSkinSpines( gid ) == 1
    assert deviation( before, sample( gid ) ) == 0.0


def test_a_spine_with_equal_set_keeps_one_value():
    """Equal means equal, on a spine as much as on a side.

    ValidateSpineParms settles the flags; the values have to be copied too.  A spine left
    carrying an Equal flag and two different numbers behaved differently depending on the
    continuity: at C0 each side of the joint used its own, but from C1 up
    connection_data::set_right_fp overwrites the left curve, so only the right value reached
    the surface and editing the left did nothing whatever.
    """
    for cont in [ 0.0, 1.0, 2.0 ]:
        vsp.VSPRenew()
        gid = vsp.AddGeom( "STACK" )
        xsurf = vsp.GetXSecSurf( gid, 0 )
        xsec = vsp.GetXSec( xsurf, 2 )

        for nm in [ "AllSym", "TBSym", "RLSym" ]:
            vsp.SetParmVal( vsp.GetXSecParm( xsec, nm ), 0.0 )
        vsp.SetParmVal( vsp.GetXSecParm( xsec, "ContinuityTop" ), cont )
        for side in SIDES:
            for lr in [ "L", "R" ]:
                vsp.SetParmVal( vsp.GetXSecParm( xsec, side + lr + "AngleSet" ), 1.0 )
                vsp.SetParmVal( vsp.GetXSecParm( xsec, side + lr + "Angle" ), 25.0 )

        vsp.AddSkinSpine( gid, 0.625 )
        vsp.Update()

        spine = vsp.GetSkinSpineID( xsec, 0 )
        vsp.SetParmVal( vsp.GetSkinSpineParm( spine, "LRAngleEq" ), 1.0 )
        vsp.SetParmVal( vsp.GetSkinSpineParm( spine, "LAngleSet" ), 1.0 )
        vsp.Update()

        before = sample( gid )

        # Edit only the left value, which is what the GUI slider drives.
        vsp.SetParmVal( vsp.GetSkinSpineParm( spine, "LAngle" ), -35.0 )
        vsp.Update()

        assert vsp.GetParmVal( vsp.GetSkinSpineParm( spine, "RAngle" ) ) == pytest.approx( -35.0 ), \
            "continuity %g: Equal is set but the right angle did not follow the left" % cont
        assert deviation( before, sample( gid ) ) > 1.0e-6, \
            "continuity %g: editing the spine angle did not move the surface" % cont


def test_a_spine_holds_its_angle_wherever_it_is_put():
    """A station's condition must be enforced at the station, not near it.

    The controls are laid out at the stations, but the rib carries a curve of tangent and
    normal vectors built by evaluating them at sample parameters and fitting through the
    results.  Those samples used to be the XSec curve's own joints alone, so a station
    between two of them was never sampled and its value was smeared across the gap instead
    of imposed.  On a default Stack the joints fall on half integers: a spine at W=2.5 held
    its 40 degrees exactly while one at W=2.25 achieved 24 of them.  The control resolution
    was the cross section curve's segmentation rather than the user's station layout.
    """
    # Positions given in the cross section curve's own parameter, converted where they are
    # handed to the API, so the relationship to the XSec curve's joints stays legible.
    for w in [ 2.125, 2.25, 2.3346, 2.5, 2.625, 2.875 ]:
        vsp.VSPRenew()
        gid = vsp.AddGeom( "STACK" )
        xsurf = vsp.GetXSecSurf( gid, 0 )
        xsec = vsp.GetXSec( xsurf, 2 )
        for nm in [ "AllSym", "TBSym", "RLSym" ]:
            vsp.SetParmVal( vsp.GetXSecParm( xsec, nm ), 0.0 )

        vsp.AddSkinSpine( gid, w / 4.0 )
        vsp.Update()

        spine = vsp.GetSkinSpineID( xsec, 0 )
        vsp.SetParmVal( vsp.GetSkinSpineParm( spine, "LRAngleEq" ), 1.0 )
        vsp.SetParmVal( vsp.GetSkinSpineParm( spine, "LAngleSet" ), 1.0 )
        vsp.SetParmVal( vsp.GetSkinSpineParm( spine, "LSlew" ), 0.0 )
        vsp.SetParmVal( vsp.GetSkinSpineParm( spine, "LAngle" ), 40.0 )
        vsp.Update()

        # Tilt of dS/du away from the body axis, just after the XSec, at the spine's own W.
        a = vsp.CompPnt01( gid, 0, 0.5, w / 4.0 )
        b = vsp.CompPnt01( gid, 0, 0.5 + 1.0e-5, w / 4.0 )
        d = ( b.x() - a.x(), b.y() - a.y(), b.z() - a.z() )
        n = math.sqrt( sum( t * t for t in d ) )
        tilt = math.degrees( math.acos( max( -1.0, min( 1.0, d[0] / n ) ) ) )

        assert tilt == pytest.approx( 40.0, abs = 0.05 ), \
            "spine at W=%.4f asked for 40 deg and achieved %.2f" % ( w, tilt )


def test_neighbouring_spines_each_hold_their_own_angle():
    """Every station must be sampled, not just the first one.

    The sample parameters are the XSec curve's joints plus the stations.  Merging the
    stations in has to take the range bounds before inserting: reading the largest parameter
    off the back of the vector while pushing into it reads whatever was last pushed, and
    every station beyond that is dropped.  The first spine was then enforced and the rest
    were not -- visible as two neighbouring spines where one worked and the other barely
    moved the surface, and as a symmetric spine whose mirrors held but whose original did
    not.
    """
    for sep in [ 0.5, 0.25, 0.165, 0.05 ]:
        wa = 2.30
        wb = wa + sep

        vsp.VSPRenew()
        gid = vsp.AddGeom( "STACK" )
        xsurf = vsp.GetXSecSurf( gid, 0 )
        xsec = vsp.GetXSec( xsurf, 2 )
        for nm in [ "AllSym", "TBSym", "RLSym" ]:
            vsp.SetParmVal( vsp.GetXSecParm( xsec, nm ), 0.0 )

        vsp.AddSkinSpine( gid, wa / 4.0 )
        vsp.AddSkinSpine( gid, wb / 4.0 )
        vsp.Update()

        for k, ang in [ ( 0, 40.0 ), ( 1, 10.0 ) ]:
            spine = vsp.GetSkinSpineID( xsec, k )
            vsp.SetParmVal( vsp.GetSkinSpineParm( spine, "LRAngleEq" ), 1.0 )
            vsp.SetParmVal( vsp.GetSkinSpineParm( spine, "LAngleSet" ), 1.0 )
            vsp.SetParmVal( vsp.GetSkinSpineParm( spine, "LSlew" ), 0.0 )
            vsp.SetParmVal( vsp.GetSkinSpineParm( spine, "LAngle" ), ang )
        vsp.Update()

        for w, want in [ ( wa, 40.0 ), ( wb, 10.0 ) ]:
            a = vsp.CompPnt01( gid, 0, 0.5, w / 4.0 )
            b = vsp.CompPnt01( gid, 0, 0.5 + 1.0e-5, w / 4.0 )
            d = ( b.x() - a.x(), b.y() - a.y(), b.z() - a.z() )
            n = math.sqrt( sum( t * t for t in d ) )
            tilt = math.degrees( math.acos( max( -1.0, min( 1.0, d[0] / n ) ) ) )

            assert tilt == pytest.approx( want, abs = 0.05 ), \
                "separation %.3f: spine at W=%.4f asked for %.0f deg and achieved %.2f" % ( sep, w, want, tilt )


def test_a_second_update_changes_nothing():
    """Update with nothing touched must not move the surface.

    ValidateParms cascades the Set flags -- continuity forces the right hand ones from the
    left, an Equal flag forces both of its own on, the symmetry flags copy whole sides.  The
    station grouping keys on exactly those flags, so it has to run after them.  When it ran
    first, the passes were laid out for the flags the user left and the ribs built from the
    flags validation produced, and the mismatched surface stood until something else was
    touched: reloading the file gave a different shape from the same parms.
    """
    rng = random.Random( 5150 )

    for trial in range( 12 ):
        vsp.VSPRenew()
        geom_type = [ "STACK", "FUSELAGE" ][ trial % 2 ]
        gid, xsurf = build_random( rng, geom_type )

        once = sample( gid )
        vsp.Update()
        assert deviation( once, sample( gid ) ) == 0.0, \
            "trial %d (%s): a second update moved the surface" % ( trial, geom_type )


def test_spines_survive_a_cross_section_shape_change():
    """Changing a cross section's shape must not discard what the XSec holds.

    XSec::CopyFrom used to drop to the non-virtual XSec::EncodeXml whenever the types
    differed, which threw away everything a derived XSec keeps outside its own Parms -- for a
    SkinXSec, the whole spine set.  ChangeXSecShape takes that path every time, since it
    builds the replacement from the same XSecSurf, so only the XSecCurve type differs.
    Changing a shape silently deleted that section's spines; doing it to the active section
    deleted them from the entire Geom, because SyncSkinSpines then truncated the rest to
    match the master.
    """
    for index in [ 0, 1 ]:
        vsp.VSPRenew()
        gid = vsp.AddGeom( "STACK" )
        xsurf = vsp.GetXSecSurf( gid, 0 )

        vsp.AddSkinSpine( gid, 0.35 )
        vsp.Update()
        for i in range( vsp.GetNumXSec( xsurf ) ):
            spine = vsp.GetSkinSpineID( vsp.GetXSec( xsurf, i ), 0 )
            vsp.SetParmVal( vsp.GetSkinSpineParm( spine, "LRAngleEq" ), 1.0 )
            vsp.SetParmVal( vsp.GetSkinSpineParm( spine, "LAngleSet" ), 1.0 )
            vsp.SetParmVal( vsp.GetSkinSpineParm( spine, "LAngle" ), 33.0 )
            vsp.SetParmVal( vsp.GetSkinSpineParm( spine, "LRSym" ), 1.0 )
        vsp.Update()

        vsp.ChangeXSecShape( xsurf, index, vsp.XS_ROUNDED_RECTANGLE )
        vsp.Update()

        assert vsp.GetNumSkinSpines( gid ) == 1, \
            "shape change on XSec %d left %d spines" % ( index, vsp.GetNumSkinSpines( gid ) )

        spine = vsp.GetSkinSpineID( vsp.GetXSec( xsurf, 2 ), 0 )
        assert vsp.GetParmVal( vsp.GetSkinSpineParm( spine, "W01" ) ) == pytest.approx( 0.35 )
        assert vsp.GetParmVal( vsp.GetSkinSpineParm( spine, "LAngle" ) ) == pytest.approx( 33.0 ), \
            "shape change on XSec %d reset the spine's angle" % index
        assert vsp.GetParmVal( vsp.GetSkinSpineParm( spine, "LRSym" ) ) == pytest.approx( 1.0 )


def test_a_shape_change_still_carries_width_and_height():
    """The one thing names cannot carry across a type change.

    Curve types spell width and height differently -- Diameter, Chord, Width -- so CopyFrom
    restates them explicitly when the curve types differ.
    """
    vsp.VSPRenew()
    gid = vsp.AddGeom( "STACK" )
    xsurf = vsp.GetXSecSurf( gid, 0 )
    xsec = vsp.GetXSec( xsurf, 2 )

    vsp.SetParmVal( vsp.GetXSecParm( xsec, "Ellipse_Width" ), 4.0 )
    vsp.SetParmVal( vsp.GetXSecParm( xsec, "Ellipse_Height" ), 2.0 )
    vsp.Update()

    vsp.ChangeXSecShape( xsurf, 2, vsp.XS_ROUNDED_RECTANGLE )
    vsp.Update()

    xsec = vsp.GetXSec( xsurf, 2 )
    assert vsp.GetParmVal( vsp.GetXSecParm( xsec, "RoundedRect_Width" ) ) == pytest.approx( 4.0 )
    assert vsp.GetParmVal( vsp.GetXSecParm( xsec, "RoundedRect_Height" ) ) == pytest.approx( 2.0 )


def test_the_validator_settles_in_one_pass():
    """ValidateParms must reach its answer in a single pass.

    The surface is built from the flags one pass leaves behind.  If a second pass would
    produce different flags, the surface on screen belongs to neither state, and reloading
    the file gives a different shape from the same parms.

    The way this went wrong: an Equal flag forces its own pair of Set flags on, and that was
    done at the end -- after slew and strength had been derived from the angle flag's earlier
    value.  Setting LRAngleEq with LAngleSet off left LAngleSet on and LStrengthSet off, a
    state the derivation says cannot exist.

    Every combination of the flags that interact, at each continuity.
    """
    interacting = [ "LAngleSet", "RAngleSet", "LCurveSet", "RCurveSet",
                    "LRAngleEq", "LRSlewEq", "LRStrengthEq", "LRCurveEq" ]
    derived = [ "LSlewSet", "RSlewSet", "LStrengthSet", "RStrengthSet" ]

    vsp.VSPRenew()
    gid = vsp.AddGeom( "STACK" )
    xsurf = vsp.GetXSecSurf( gid, 0 )
    vsp.AddSkinSpine( gid, 0.35 )
    vsp.Update()
    xsec = vsp.GetXSec( xsurf, 2 )

    def read( spine ):
        return tuple( vsp.GetParmVal( vsp.GetSkinSpineParm( spine, n ) ) for n in interacting + derived )

    for cont in [ 0.0, 1.0, 2.0 ]:
        for bits in range( 1 << len( interacting ) ):
            spine = vsp.GetSkinSpineID( xsec, 0 )
            vsp.SetParmVal( vsp.GetXSecParm( xsec, "ContinuityTop" ), cont )
            for k, nm in enumerate( interacting ):
                vsp.SetParmVal( vsp.GetSkinSpineParm( spine, nm ), float( ( bits >> k ) & 1 ) )
            vsp.Update()
            first = read( spine )

            # Dirty the surface without touching a flag, so validation runs again.
            angle = vsp.GetSkinSpineParm( spine, "LAngle" )
            vsp.SetParmVal( angle, vsp.GetParmVal( angle ) + 1.0 )
            vsp.Update()
            second = read( spine )

            assert first == second, \
                "continuity %g, flags %s: a second pass changed them to %s" % (
                    cont,
                    dict( zip( interacting + derived, [ int( x ) for x in first ] ) ),
                    dict( zip( interacting + derived, [ int( x ) for x in second ] ) ) )


def test_the_surface_does_not_tear_between_stations():
    """The blend pairs each group's patches by index, so every group is cut at every station.

    A rib gains a joint at a station either by carrying condition regions, whose breaks are
    merged into the joint set, or by carrying a tangent or normal curve, which is sampled at
    the stations.  A group that enforces nothing has neither, so if its regions were dropped
    it would keep no joint where a station falls between two cross section curve joints, while
    the group that does enforce there would.  The two skins would then have different patches
    and the blend would multiply a weight into the wrong interval, opening the surface up.

    Checked directly rather than against a reference: walk the section and look for a step
    much larger than its neighbours.
    """
    for w01 in [ 0.125, 0.15, 0.40, 0.61 ]:
        vsp.VSPRenew()
        gid = vsp.AddGeom( "STACK" )
        xsurf = vsp.GetXSecSurf( gid, 0 )

        # Every side free, so the sides form a group that enforces nothing at all.
        for i in range( vsp.GetNumXSec( xsurf ) ):
            xsec = vsp.GetXSec( xsurf, i )
            for nm in [ "AllSym", "TBSym", "RLSym" ]:
                vsp.SetParmVal( vsp.GetXSecParm( xsec, nm ), 0.0 )
            for side in SIDES:
                for nm in EQUALS:
                    vsp.SetParmVal( vsp.GetXSecParm( xsec, side + nm ), 0.0 )
                for lr in [ "L", "R" ]:
                    vsp.SetParmVal( vsp.GetXSecParm( xsec, side + lr + "AngleSet" ), 0.0 )
                    vsp.SetParmVal( vsp.GetXSecParm( xsec, side + lr + "CurveSet" ), 0.0 )

        vsp.AddSkinSpine( gid, w01 )
        vsp.Update()

        for i in range( vsp.GetNumXSec( xsurf ) ):
            spine = vsp.GetSkinSpineID( vsp.GetXSec( xsurf, i ), 0 )
            for nm in EQUALS:
                vsp.SetParmVal( vsp.GetSkinSpineParm( spine, nm ), 0.0 )
            for lr in [ "L", "R" ]:
                vsp.SetParmVal( vsp.GetSkinSpineParm( spine, lr + "AngleSet" ), 1.0 )
                vsp.SetParmVal( vsp.GetSkinSpineParm( spine, lr + "Angle" ), 35.0 )
        vsp.Update()

        steps = []
        prev = None
        for j in range( 161 ):
            p = vsp.CompPnt01( gid, 0, 0.5, j / 160.0 )
            cur = ( p.x(), p.y(), p.z() )
            if prev is not None:
                steps.append( math.dist( prev, cur ) )
            prev = cur

        biggest = max( steps )
        typical = sorted( steps )[ len( steps ) // 2 ]

        # A circle sampled evenly in w steps a little unevenly on its own: the ratio sits at
        # 1.16 when the surface is whole, and reaches 1.44 when it is torn.
        assert biggest < 1.3 * typical, \
            "spine at W01=%.3f: the section steps by %.4f where it usually steps %.4f" % (
                w01, biggest, typical )


def test_a_conformal_child_reproduces_a_blended_parent():
    """A Conformal geom rebuilds its parent's skin from what the surface kept.

    A blended surface is one skin per group of stations enforcing alike, summed with weights,
    and no single rib set describes it.  The surface used to keep only the first group's ribs,
    so the child applied that group's conditions right around the cross section and the sides
    that asked for something else silently lost them -- and once a group's conditions are
    confined to its own spans, that rib is not one the uniform creator will even accept, so it
    failed outright.

    At zero offset the child should sit exactly on its parent, which is the sharpest statement
    of "rebuilt the same surface".
    """
    for spine in [ False, True ]:
        vsp.VSPRenew()
        gid = vsp.AddGeom( "STACK" )
        xsurf = vsp.GetXSecSurf( gid, 0 )

        if spine:
            vsp.AddSkinSpine( gid, 0.35 )
        vsp.Update()

        for i in range( vsp.GetNumXSec( xsurf ) ):
            xsec = vsp.GetXSec( xsurf, i )
            for nm in [ "AllSym", "TBSym", "RLSym" ]:
                vsp.SetParmVal( vsp.GetXSecParm( xsec, nm ), 0.0 )

            # Top and Left hold a tangent, Bottom and Right run free: several groups, so the
            # parent is blended.
            for side in SIDES:
                on = 0.0
                if side in ( "Top", "Left" ):
                    on = 1.0
                for nm in EQUALS:
                    vsp.SetParmVal( vsp.GetXSecParm( xsec, side + nm ), 0.0 )
                for lr in [ "L", "R" ]:
                    vsp.SetParmVal( vsp.GetXSecParm( xsec, side + lr + "AngleSet" ), on )
                    vsp.SetParmVal( vsp.GetXSecParm( xsec, side + lr + "Angle" ), 25.0 )

            if spine:
                spn = vsp.GetSkinSpineID( xsec, 0 )
                for nm in EQUALS:
                    vsp.SetParmVal( vsp.GetSkinSpineParm( spn, nm ), 0.0 )
                for lr in [ "L", "R" ]:
                    vsp.SetParmVal( vsp.GetSkinSpineParm( spn, lr + "AngleSet" ), 1.0 )
                    vsp.SetParmVal( vsp.GetSkinSpineParm( spn, lr + "Angle" ), -30.0 )
        vsp.Update()

        cid = vsp.AddGeom( "CONFORMAL", gid )
        vsp.SetParmVal( vsp.GetParm( cid, "Offset", "Design" ), 0.0 )
        vsp.Update()

        worst = 0.0
        for i in range( 21 ):
            for j in range( 31 ):
                a = vsp.CompPnt01( gid, 0, i / 20.0, j / 30.0 )
                b = vsp.CompPnt01( cid, 0, i / 20.0, j / 30.0 )
                worst = max( worst, math.dist( ( a.x(), a.y(), a.z() ), ( b.x(), b.y(), b.z() ) ) )

        assert worst < 1.0e-9, \
            "spine=%s: conformal child at zero offset is %.3e from its parent" % ( spine, worst )


def _sample_surface( gid, nu = 17, nw = 49 ):
    pts = []
    for i in range( nu ):
        for j in range( nw ):
            p = vsp.CompPnt01( gid, 0, i / ( nu - 1.0 ), j / ( nw - 1.0 ) )
            pts.append( ( p.x(), p.y(), p.z() ) )
    return pts


def test_the_curve_frame_matches_the_circle_frame_on_a_circle():
    """The two skinning frames must agree wherever the assumption behind the old one holds.

    Angle and slew are rotations of a frame.  The old one is built by assuming the section is
    a circle traversed uniformly in the curve parameter; the new one takes the frame from the
    curve itself.  On a circle the assumption is true, so the two have to give the same
    surface -- and if the tangent's sign or the handedness were wrong, they would not be close
    at all.  This is what pins those down.

    A rounded rectangle is the other half of the statement: it agrees at the four sides, where
    a symmetric section's tangent runs along the face and the circle is right anyway, and
    differs in between, which is the whole point of offering the choice.
    """
    def build( shape, curvebasis ):
        vsp.VSPRenew()
        gid = vsp.AddGeom( "STACK" )
        xsurf = vsp.GetXSecSurf( gid, 0 )

        for i in range( vsp.GetNumXSec( xsurf ) ):
            vsp.ChangeXSecShape( xsurf, i, shape )
        vsp.Update()

        for i in range( vsp.GetNumXSec( xsurf ) ):
            xsec = vsp.GetXSec( xsurf, i )
            vsp.SetParmVal( vsp.GetXSecParm( xsec, "CurveBasis" ), curvebasis )
            for nm in [ "AllSym", "TBSym", "RLSym" ]:
                vsp.SetParmVal( vsp.GetXSecParm( xsec, nm ), 0.0 )
            for side in SIDES:
                for lr in [ "L", "R" ]:
                    vsp.SetParmVal( vsp.GetXSecParm( xsec, side + lr + "AngleSet" ), 1.0 )
                    vsp.SetParmVal( vsp.GetXSecParm( xsec, side + lr + "Angle" ), 25.0 )
        vsp.Update()
        return gid

    def walk( gid, ws ):
        return [ ( lambda p: ( p.x(), p.y(), p.z() ) )( vsp.CompPnt01( gid, 0, i / 40.0, w ) )
                 for i in range( 41 ) for w in ws ]

    everywhere = [ j / 120.0 for j in range( 121 ) ]
    sides = [ 0.0, 0.25, 0.5, 0.75 ]

    circle_ref = walk( build( vsp.XS_CIRCLE, 0.0 ), everywhere )
    circle_new = walk( build( vsp.XS_CIRCLE, 1.0 ), everywhere )
    # A circle section is four cubic Beziers, so it is exactly circular only at the segment
    # joints.  The skinning controls are sampled between the joints too, where the circle
    # assumption is off by the approximation error, so the two frames agree to that rather
    # than exactly.  The rounded rectangle below moves ninety times further, so a wrong
    # tangent sign or handedness still cannot pass this.
    assert deviation( circle_ref, circle_new ) < 1.0e-3, \
        "on a circle the two frames differ by %.3e" % deviation( circle_ref, circle_new )

    rr_ref = walk( build( vsp.XS_ROUNDED_RECTANGLE, 0.0 ), sides )
    rr_new = walk( build( vsp.XS_ROUNDED_RECTANGLE, 1.0 ), sides )
    assert deviation( rr_ref, rr_new ) < 1.0e-12, \
        "at the four sides the two frames differ by %.3e" % deviation( rr_ref, rr_new )

    rr_ref = walk( build( vsp.XS_ROUNDED_RECTANGLE, 0.0 ), everywhere )
    rr_new = walk( build( vsp.XS_ROUNDED_RECTANGLE, 1.0 ), everywhere )
    # Measured 7.46e-2.  The floor is set well below that but far above nothing, so a
    # regression that merely halved the disagreement still fails rather than sliding under a
    # tolerance chosen to be easy to pass.
    assert deviation( rr_ref, rr_new ) > 3.0e-2, \
        "the curve frame moved a rounded rectangle only %.3e, against 7.46e-2 measured" % (
            deviation( rr_ref, rr_new ) )


def test_a_point_section_keeps_the_circle_frame():
    """A point section has no extent, so no tangent at any parameter and no frame to build.

    It keeps the circle whatever the flag says, so setting the flag on one does nothing at
    all.  The nose and tail of a default Stack are point sections, so this is not a corner
    case a user has to go looking for.
    """
    def build( curvebasis ):
        vsp.VSPRenew()
        gid = vsp.AddGeom( "STACK" )
        xsurf = vsp.GetXSecSurf( gid, 0 )

        vsp.ChangeXSecShape( xsurf, 2, vsp.XS_POINT )
        vsp.Update()

        for i in range( vsp.GetNumXSec( xsurf ) ):
            xsec = vsp.GetXSec( xsurf, i )
            for nm in [ "AllSym", "TBSym", "RLSym" ]:
                vsp.SetParmVal( vsp.GetXSecParm( xsec, nm ), 0.0 )
            for side in SIDES:
                for lr in [ "L", "R" ]:
                    vsp.SetParmVal( vsp.GetXSecParm( xsec, side + lr + "AngleSet" ), 1.0 )
                    vsp.SetParmVal( vsp.GetXSecParm( xsec, side + lr + "Angle" ), 25.0 )

        # Only the point section, so it is the one thing that differs between runs.
        vsp.SetParmVal( vsp.GetXSecParm( vsp.GetXSec( xsurf, 2 ), "CurveBasis" ), curvebasis )
        vsp.Update()
        return gid

    moved = deviation( sample( build( 0.0 ) ), sample( build( 1.0 ) ) )
    assert moved == 0.0, \
        "asking a point section for a curve frame moved the surface by %.3e" % moved


def test_a_slit_section_takes_its_frame_from_the_curve():
    """A section with no width or no height still has a tangent, and it is worth having.

    A slit is not a closed loop: the curve runs out along it and back.  The tangent reverses
    at each end, so the two sides get frames a half turn apart -- which is right, since those
    really are the two sides of the surface and each one's width axis points out of its own.
    Only the fold itself has nothing to say, and there the two one sided tangents are exactly
    opposed and the circle stands in.

    So the flag has to do something here, and what it does has to stay finite and bounded.
    """
    def build( shape, w, h, curvebasis ):
        vsp.VSPRenew()
        gid = vsp.AddGeom( "STACK" )
        xsurf = vsp.GetXSecSurf( gid, 0 )

        vsp.ChangeXSecShape( xsurf, 2, shape )
        vsp.Update()
        vsp.SetXSecWidthHeight( vsp.GetXSec( xsurf, 2 ), w, h )
        vsp.Update()

        for i in range( vsp.GetNumXSec( xsurf ) ):
            xsec = vsp.GetXSec( xsurf, i )
            for nm in [ "AllSym", "TBSym", "RLSym" ]:
                vsp.SetParmVal( vsp.GetXSecParm( xsec, nm ), 0.0 )
            for side in SIDES:
                for lr in [ "L", "R" ]:
                    vsp.SetParmVal( vsp.GetXSecParm( xsec, side + lr + "AngleSet" ), 1.0 )
                    vsp.SetParmVal( vsp.GetXSecParm( xsec, side + lr + "Angle" ), 25.0 )

        vsp.SetParmVal( vsp.GetXSecParm( vsp.GetXSec( xsurf, 2 ), "CurveBasis" ), curvebasis )
        vsp.Update()
        return gid

    for name, w, h in [ ( "no width", 0.0, 2.0 ), ( "no height", 2.0, 0.0 ) ]:
        ref = sample( build( vsp.XS_ELLIPSE, w, h, 0.0 ) )
        new = sample( build( vsp.XS_ELLIPSE, w, h, 1.0 ) )

        # Measured 7.6e-2 with no height and 8.6e-2 with no width.
        assert deviation( ref, new ) > 3.0e-2, \
            "%s: the curve frame moved the slit only %.3e, against 7.6e-2 measured" % (
                name, deviation( ref, new ) )

        # Finite, and nowhere near escaping: the fold is where a frame built on a reversing
        # tangent would go wrong, and it would go wrong by a lot rather than a little.
        reach = max( abs( c ) for p in ref for c in p )
        for p in new:
            for c in p:
                assert math.isfinite( c ), "%s: the surface came back with %r" % ( name, c )
                assert abs( c ) < 10.0 * reach + 1.0, \
                    "%s: the surface reaches %.3e where the circle frame reaches %.3e" % (
                        name, abs( c ), reach )


def test_the_readback_uses_the_frame_the_surface_was_built_in():
    """Both directions go through GetBasis, so both have to follow the flag.

    GetTanNormCrv turns angle and slew into the derivatives the loft is built from.
    GetAngStrCrv goes the other way, turning an achieved derivative back into angle and slew,
    and SetUnsetParms writes those into the parms that are switched off so the GUI can report
    what the loft did.  If the readback measured from a different frame than the surface was
    built in, the numbers on screen would describe some other surface.

    With nothing enforced the flag cannot touch the geometry -- there are no conditions for a
    frame to orient -- so the surface is identical either way and any change in the reported
    values is the readback, and only the readback, changing frame.  Were GetAngStrCrv still
    measuring from the circle, the reported values would be identical too.
    """
    VALUES = [ "LAngle", "LSlew", "RAngle", "RSlew" ]
    SETS = [ v + "Set" for v in [ "LAngle", "LSlew", "LStrength", "LCurve",
                                  "RAngle", "RSlew", "RStrength", "RCurve" ] ]

    def build( curvebasis ):
        vsp.VSPRenew()
        gid = vsp.AddGeom( "STACK" )
        xsurf = vsp.GetXSecSurf( gid, 0 )

        for i in range( vsp.GetNumXSec( xsurf ) ):
            vsp.ChangeXSecShape( xsurf, i, vsp.XS_ROUNDED_RECTANGLE )
        vsp.Update()

        # Off the four sides, so the frames do not happen to coincide there.
        vsp.AddSkinSpine( gid, 0.10 )
        vsp.Update()

        for i in range( vsp.GetNumXSec( xsurf ) ):
            xsec = vsp.GetXSec( xsurf, i )
            vsp.SetParmVal( vsp.GetXSecParm( xsec, "CurveBasis" ), curvebasis )
            for nm in [ "AllSym", "TBSym", "RLSym" ]:
                vsp.SetParmVal( vsp.GetXSecParm( xsec, nm ), 0.0 )
            for side in SIDES:
                for nm in EQUALS + SETS:
                    vsp.SetParmVal( vsp.GetXSecParm( xsec, side + nm ), 0.0 )
            spine = vsp.GetSkinSpineID( xsec, 0 )
            for nm in EQUALS + SETS:
                vsp.SetParmVal( vsp.GetSkinSpineParm( spine, nm ), 0.0 )
        vsp.Update()

        reported = {}
        for i in range( vsp.GetNumXSec( xsurf ) ):
            xsec = vsp.GetXSec( xsurf, i )
            for side in SIDES:
                for nm in VALUES:
                    reported[ ( i, side, nm ) ] = vsp.GetParmVal( vsp.GetXSecParm( xsec, side + nm ) )
            spine = vsp.GetSkinSpineID( xsec, 0 )
            for nm in VALUES:
                reported[ ( i, "spine", nm ) ] = vsp.GetParmVal( vsp.GetSkinSpineParm( spine, nm ) )

        return sample( gid ), reported

    circle_surf, circle_says = build( 0.0 )
    curve_surf, curve_says = build( 1.0 )

    assert deviation( circle_surf, curve_surf ) == 0.0, \
        "with nothing enforced the flag moved the surface by %.3e" % deviation( circle_surf, curve_surf )

    sides = max( abs( circle_says[k] - curve_says[k] ) for k in circle_says if k[1] != "spine" )
    assert sides < 1.0e-9, \
        "the two frames disagree by %.3e at the four sides of a symmetric section" % sides

    spine = max( abs( circle_says[k] - curve_says[k] ) for k in circle_says if k[1] == "spine" )
    # Measured 34.5 degrees.
    assert spine > 20.0, \
        "the readback differs by only %.4f degrees between frames, against 34.5 measured -- " \
        "is it still measuring from the circle?" % spine


def test_the_curve_frame_reaches_a_shape_that_keeps_no_size_in_its_parms():
    """The frame comes from the curve, so a shape that does not record its size cannot hide.

    AC25_773 keeps dummy Width and Height parms pinned to zero while its curve spans about
    140 by 62.  Deciding "this section is a point" from those parms classified it as one, so
    the curve frame was refused on it and the toggle did nothing at all -- ticked, with no
    warning and nothing greyed out.  The extent is measured from the curve now.
    """
    def build( curvebasis ):
        vsp.VSPRenew()
        gid = vsp.AddGeom( "STACK" )
        xsurf = vsp.GetXSecSurf( gid, 0 )

        for i in range( vsp.GetNumXSec( xsurf ) ):
            vsp.ChangeXSecShape( xsurf, i, vsp.XS_AC25_773 )
        vsp.Update()

        for i in range( vsp.GetNumXSec( xsurf ) ):
            xsec = vsp.GetXSec( xsurf, i )
            vsp.SetParmVal( vsp.GetXSecParm( xsec, "CurveBasis" ), curvebasis )
            for nm in [ "AllSym", "TBSym", "RLSym" ]:
                vsp.SetParmVal( vsp.GetXSecParm( xsec, nm ), 0.0 )
            for side in SIDES:
                for lr in [ "L", "R" ]:
                    vsp.SetParmVal( vsp.GetXSecParm( xsec, side + lr + "AngleSet" ), 1.0 )
                    vsp.SetParmVal( vsp.GetXSecParm( xsec, side + lr + "Angle" ), 25.0 )
        vsp.Update()
        return gid

    moved = deviation( sample( build( 0.0 ) ), sample( build( 1.0 ) ) )
    # Measured 2.99e-2.
    assert moved > 1.0e-2, \
        "the curve frame moved a section whose size parms are dummies only %.3e, against " \
        "2.99e-2 measured" % moved
