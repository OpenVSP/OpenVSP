# Parasite Drag on models whose Geoms do not line up one-for-one with their DegenGeoms.
#
# Reference length, fineness ratio or t/c, and form factor come from each Geom's DegenGeom.
# They depend only on that Geom's shape, so a Geom must get the same values whatever else is
# in the model.

import openvsp as vsp
import math
import os
import tempfile

from testhelp import ( drop_errors, pop_errors )


GEOM_RESULTS = ( "Comp_Lref", "Comp_FineRat", "Comp_FFOut" )


def fresh():
    vsp.VSPRenew()
    out = tempfile.mkdtemp()
    for t in ( vsp.COMP_GEOM_TXT_TYPE, vsp.COMP_GEOM_CSV_TYPE, vsp.DRAG_BUILD_CSV_TYPE, vsp.DEGEN_GEOM_CSV_TYPE ):
        vsp.SetComputationFileName( t, os.path.join( out, "f%d" % t ) )
    drop_errors()


def build_up():
    """Run the analysis; {label: {result name: value}} for each Geom row."""
    vsp.Update()
    vsp.SetAnalysisInputDefaults( "ParasiteDrag" )
    rid = vsp.ExecAnalysis( "ParasiteDrag" )
    labels = vsp.GetStringResults( rid, "Comp_Label" )
    rows = {}
    for n in GEOM_RESULTS + ( "Comp_Swet", ):
        vals = vsp.GetDoubleResults( rid, n )
        assert len( vals ) == len( labels ), "%s has %d values for %d rows" % ( n, len( vals ), len( labels ) )
        for lab, v in zip( labels, vals ):
            rows.setdefault( lab, {} )[n] = v
    drop_errors()
    return rows


def place( gid, x ):
    vsp.SetParmVal( gid, "X_Rel_Location", "XForm", x )


def alone( add ):
    """Rows for a model of just the Geoms add() makes."""
    fresh()
    add()
    return build_up()


def a_pod():
    vsp.SetGeomName( vsp.AddGeom( "POD" ), "ThePod" )


def a_wing():
    w = vsp.AddGeom( "WING" )
    vsp.SetGeomName( w, "TheWing" )
    place( w, 50.0 )


def assert_same( got, want, names ):
    for lab in names:
        assert lab in got, "no row for %s: %s" % ( lab, sorted( got ) )
        for n in GEOM_RESULTS:
            assert abs( got[lab][n] - want[lab][n] ) <= 1e-9 * max( 1.0, abs( want[lab][n] ) ), \
                   "%s %s is %g, alone it is %g" % ( lab, n, got[lab][n], want[lab][n] )


def testAnAuxiliaryGeomDoesNotShiftTheOthers():
    want = alone( a_pod )
    fresh()
    pod = vsp.AddGeom( "POD" )
    vsp.SetGeomName( pod, "ThePod" )
    vsp.AddGeom( "AUXILIARY", pod )
    assert_same( build_up(), want, [ "ThePod" ] )


def testADiskPropFirstInTheModel():
    """Used to loop forever: the disk has a DegenGeom but no row."""
    want = alone( a_pod )
    fresh()
    p = vsp.AddGeom( "PROP" )
    vsp.SetParmVal( p, "PropMode", "Design", vsp.PROP_DISK )
    pod = vsp.AddGeom( "POD" )
    vsp.SetGeomName( pod, "ThePod" )
    place( pod, 20.0 )
    assert_same( build_up(), want, [ "ThePod" ] )


def testADiskPropOnTheNoseOfASymmetricModel():
    """Used to overflow the DegenGeom vector."""
    want = alone( a_wing )[ "TheWing" ]
    fresh()
    f = vsp.AddGeom( "FUSELAGE" )
    p = vsp.AddGeom( "PROP", f )
    vsp.SetParmVal( p, "PropMode", "Design", vsp.PROP_DISK )
    names = [ "W", "H", "V" ]
    for i, n in enumerate( names ):
        w = vsp.AddGeom( "WING" )
        vsp.SetGeomName( w, n )
        place( w, 50.0 * ( i + 1 ) )
    vsp.SetParmVal( w, "Sym_Planar_Flag", "Sym", 0 )
    assert_same( build_up(), dict( ( n, want ) for n in names ), names )


def testAPropShowingBladesAndDisk():
    want = alone( a_wing )
    fresh()
    p = vsp.AddGeom( "PROP" )
    vsp.SetParmVal( p, "PropMode", "Design", vsp.PROP_BOTH )
    w = vsp.AddGeom( "WING" )
    vsp.SetGeomName( w, "TheWing" )
    place( w, 50.0 )
    assert_same( build_up(), want, [ "TheWing" ] )


def testAHumanWithManySurfaces():
    want = alone( a_wing )
    fresh()
    h = vsp.AddGeom( "HUMAN" )
    place( h, -50.0 )
    w = vsp.AddGeom( "WING" )
    vsp.SetGeomName( w, "TheWing" )
    place( w, 50.0 )
    assert_same( build_up(), want, [ "TheWing" ] )


def a_prop_with_a_subsurface():
    p = vsp.AddGeom( "PROP" )
    vsp.Update()
    vsp.AddSubSurf( p, vsp.SS_RECTANGLE, 0 )
    return p


def testAMultiBladePropWithASubsurface():
    """The row count took one row per symmetry copy for each subsurface, the rows made one per
    surface: the Geoms after a three-blade Prop lost their rows."""
    want = alone( a_pod )
    fresh()
    a_prop_with_a_subsurface()
    pod = vsp.AddGeom( "POD" )
    vsp.SetGeomName( pod, "ThePod" )
    place( pod, 20.0 )
    got = build_up()
    assert_same( got, want, [ "ThePod" ] )
    assert abs( got["ThePod"]["Comp_Swet"] - want["ThePod"]["Comp_Swet"] ) < 1e-6 * want["ThePod"]["Comp_Swet"]


def testACloneOfAPropWithASubsurface():
    want = alone( a_pod )
    fresh()
    p = a_prop_with_a_subsurface()
    vsp.Update()
    vsp.CloneGeomVec( [ p ] )
    pod = vsp.AddGeom( "POD" )
    vsp.SetGeomName( pod, "ThePod" )
    place( pod, 20.0 )
    assert_same( build_up(), want, [ "ThePod" ] )


def testAnAuxiliaryAlone():
    """CompGeom makes no results for an Auxiliary alone; the wetted areas were read from them."""
    fresh()
    vsp.AddGeom( "AUXILIARY" )
    build_up()


def testDeletingTheResultsBeforeReusingTheGeometry():
    """The wetted areas were read from the CompGeom results each time, so deleting the results
    and then reusing the geometry read freed memory."""
    want = alone( a_pod )
    vsp.DeleteAllResults()
    vsp.SetIntAnalysisInput( "ParasiteDrag", "RecomputeGeom", [ 0 ] )
    rid = vsp.ExecAnalysis( "ParasiteDrag" )
    swet = vsp.GetDoubleResults( rid, "Comp_Swet" )
    assert abs( swet[0] - want["ThePod"]["Comp_Swet"] ) < 1e-9 * want["ThePod"]["Comp_Swet"]


def testAnOlderCompGeomIsNotUsed():
    """A CompGeom that makes nothing must not pick up the last one that did."""
    fresh()
    w = vsp.AddGeom( "WING" )
    vsp.SetGeomName( w, "TheWing" )
    vsp.Update()
    assert build_up()["TheWing"]["Comp_Swet"] > 0.0
    vsp.SetParmVal( w, "Span", "XSec_1", 0.0 )
    vsp.Update()
    swet = build_up()["TheWing"]["Comp_Swet"]
    assert swet == 0.0, "a wing of no span has wetted area %g from the older CompGeom" % swet


def excres_inputs():
    vsp.SetAnalysisInputDefaults( "ParasiteDrag" )
    rid = vsp.ExecAnalysis( "ParasiteDrag" )
    drop_errors()
    return dict( zip( vsp.GetStringResults( rid, "Excres_Label" ), vsp.GetDoubleResults( rid, "Excres_Input" ) ) )


def testDeletingAnExcrescenceKeepsTheOthersValues():
    """The deleted one's value used to land on the one after it."""
    fresh()
    vsp.AddGeom( "WING" )
    vsp.Update()
    vsp.AddExcrescence( "A", vsp.EXCRESCENCE_COUNT, 5.0 )
    vsp.AddExcrescence( "B", vsp.EXCRESCENCE_CD, 0.001 )
    vsp.AddExcrescence( "C", vsp.EXCRESCENCE_COUNT, 20.0 )
    excres_inputs()
    vsp.DeleteExcrescence( 0 )
    assert excres_inputs() == { "B": 0.001, "C": 20.0 }


def testAnExcrescenceIsNotClampedByTheLastOnesType():
    fresh()
    vsp.AddGeom( "WING" )
    vsp.Update()
    vsp.AddExcrescence( "cd", vsp.EXCRESCENCE_CD, 0.001 )
    vsp.AddExcrescence( "cd2", vsp.EXCRESCENCE_CD, 0.002 )
    vsp.DeleteExcrescence( 1 )
    vsp.AddExcrescence( "count", vsp.EXCRESCENCE_COUNT, 300.0 )
    assert excres_inputs() == { "cd": 0.001, "count": 300.0 }


def testExcrescencesSurviveAFile():
    fresh()
    vsp.AddGeom( "WING" )
    vsp.Update()
    vsp.AddExcrescence( "cd", vsp.EXCRESCENCE_CD, 0.001 )
    vsp.AddExcrescence( "count", vsp.EXCRESCENCE_COUNT, 50.0 )
    vsp.AddExcrescence( "area", vsp.EXCRESCENCE_DRAGAREA, 0.5 )
    want = excres_inputs()
    fname = os.path.join( tempfile.mkdtemp(), "excres.vsp3" )
    vsp.WriteVSPFile( fname )
    fresh()
    vsp.ReadVSPFile( fname )
    assert excres_inputs() == want


def testThePressureUnitIsSaved():
    """The unit was not a registered Parm, so a file kept the pressure but not its unit."""
    fresh()
    vsp.AddGeom( "WING" )
    vsp.Update()
    cid = vsp.FindContainer( "ParasiteDragSettings", 0 )
    unit = vsp.FindParm( cid, "PresUnit", "ParasiteDrag" )
    assert unit, "no PresUnit Parm"
    vsp.SetParmVal( cid, "FreestreamType", "ParasiteDrag", vsp.ATMOS_TYPE_MANUAL_P_T )
    vsp.SetParmVal( unit, vsp.PRES_UNIT_PA )
    vsp.SetParmVal( cid, "Pres", "ParasiteDrag", 90000.0 )
    vsp.Update()
    fname = os.path.join( tempfile.mkdtemp(), "pres.vsp3" )
    vsp.WriteVSPFile( fname )
    fresh()
    vsp.ReadVSPFile( fname )
    cid = vsp.FindContainer( "ParasiteDragSettings", 0 )
    assert vsp.GetParmVal( vsp.FindParm( cid, "PresUnit", "ParasiteDrag" ) ) == vsp.PRES_UNIT_PA
    assert vsp.GetParmVal( cid, "Pres", "ParasiteDrag" ) == 90000.0


def testEveryComponentResultHasOneValuePerComponent():
    for sub in ( 0, 1 ):
        fresh()
        vsp.AddGeom( "WING" )
        vsp.AddGeom( "POD" )
        vsp.Update()
        vsp.SetAnalysisInputDefaults( "ParasiteDrag" )
        vsp.SetIntAnalysisInput( "ParasiteDrag", "ExportSubCompFlag", [ sub ] )
        rid = vsp.ExecAnalysis( "ParasiteDrag" )
        n = vsp.GetIntResults( rid, "Num_Comp" )[0]
        for name in vsp.GetAllDataNames( rid ):
            if name.startswith( "Comp_" ):
                t = vsp.GetResultsType( rid, name )
                if t == vsp.DOUBLE_DATA:
                    vals = vsp.GetDoubleResults( rid, name )
                elif t == vsp.INT_DATA:
                    vals = vsp.GetIntResults( rid, name )
                else:
                    vals = vsp.GetStringResults( rid, name )
                assert len( vals ) == n, "%s has %d values for %d components" % ( name, len( vals ), n )
        drop_errors()


def testWhiteChristophHeatTransferIsReplacedByTheDefault():
    """It is a local skin friction equation, not a flat plate average, so it is DO_NOT_USE."""
    cfs = []
    errs = []
    for eq in ( vsp.DO_NOT_USE_CF_TURB_HEATTRANSFER_WHITE_CHRISTOPH, None ):
        fresh()
        vsp.AddGeom( "WING" )
        vsp.Update()
        vsp.SetAnalysisInputDefaults( "ParasiteDrag" )
        if eq is not None:
            vsp.SetIntAnalysisInput( "ParasiteDrag", "TurbCfEqnChoice", [ eq ] )
        rid = vsp.ExecAnalysis( "ParasiteDrag" )
        cfs.append( vsp.GetDoubleResults( rid, "Comp_Cf" )[0] )
        errs.append( pop_errors() )
    assert errs[0], "choosing the DO_NOT_USE equation raised no error"
    assert cfs[0] == cfs[1], "Cf %g, default equation %g" % tuple( cfs )


def testEveryTurbulentCfEquationHasAName():
    """The Schlichting roughness flow correction case set a member, so its name came back empty."""
    for eq in ( vsp.CF_TURB_ROUGHNESS_SCHLICHTING_AVG, vsp.CF_TURB_ROUGHNESS_SCHLICHTING_AVG_FLOW_CORRECTION ):
        fresh()
        vsp.AddGeom( "WING" )
        vsp.Update()
        vsp.SetAnalysisInputDefaults( "ParasiteDrag" )
        vsp.SetIntAnalysisInput( "ParasiteDrag", "TurbCfEqnChoice", [ eq ] )
        rid = vsp.ExecAnalysis( "ParasiteDrag" )
        name = vsp.GetStringResults( rid, "TurbCfEqnName" )[0]
        drop_errors()
        assert name != "", "equation %d has no name" % eq


def testASeparateSubsurfaceRowHoldsOnlyItsOwnArea():
    """A Separate subsurface's row also took in the Geom's other subsurfaces that were treated as parent."""
    def rows( separate ):
        fresh()
        p = vsp.AddGeom( "POD" )
        vsp.Update()
        ss = []
        for name, u in ( ( "A", 0.25 ), ( "B", 0.75 ) ):
            s = vsp.AddSubSurf( p, vsp.SS_RECTANGLE, 0 )
            vsp.SetSubSurfName( s, name )
            vsp.SetParmVal( vsp.GetParm( s, "Center_U", "SS_Rectangle" ), u )
            vsp.SetParmVal( vsp.GetParm( s, "U_Length", "SS_Rectangle" ), 0.2 )
            ss.append( s )
        for i in separate:
            vsp.SetParmVal( vsp.GetParm( ss[i], "IncludeFlag", "SubSurface" ), vsp.SS_INC_SEPARATE_TREATMENT )
        vsp.Update()
        vsp.SetAnalysisInputDefaults( "ParasiteDrag" )
        vsp.SetIntAnalysisInput( "ParasiteDrag", "ExportSubCompFlag", [ 1 ] )
        rid = vsp.ExecAnalysis( "ParasiteDrag" )
        drop_errors()
        return dict( zip( vsp.GetStringResults( rid, "Comp_Label" ), vsp.GetDoubleResults( rid, "Comp_Swet" ) ) )

    whole = rows( () )[ "PodGeom" ]
    both = rows( ( 0, 1 ) )
    one = rows( ( 0, ) )
    assert abs( one[ "[ss] A" ] - both[ "[ss] A" ] ) < 1e-6, "Separate row %g, alone it is %g" % ( one[ "[ss] A" ], both[ "[ss] A" ] )
    assert abs( one[ "PodGeom" ] + one[ "[ss] A" ] - whole ) < 1e-6, ( one, whole )


def testQuarterAndHalfChordSweepFollowTheTaper():
    """The chord change along the span was subtracted, so taper raised the c/4 and c/2 sweep."""
    fresh()
    w = vsp.AddGeom( "WING" )
    for name, val in ( ( "Sweep_Location", 0.0 ), ( "Sweep", 45.0 ), ( "Root_Chord", 6.0 ), ( "Tip_Chord", 1.5 ), ( "Span", 10.0 ) ):
        vsp.SetParmVal( w, name, "XSec_1", val )
    vsp.SetParmVal( w, "FFWingEqnType", "ParasiteDragProps", vsp.FF_W_JENKINSON_WING )
    vsp.Update()
    vsp.SetAnalysisInputDefaults( "ParasiteDrag" )
    rid = vsp.ExecAnalysis( "ParasiteDrag" )
    drop_errors()
    ff = vsp.GetDoubleResults( rid, "Comp_FFOut" )[0]
    t = vsp.GetDoubleResults( rid, "Comp_FineRat" )[0]

    # Jenkinson wing form factor with the half chord sweep of this planform
    sweep50 = math.atan( math.tan( math.radians( 45.0 ) ) + 0.5 * ( 1.5 - 6.0 ) / 10.0 )
    fstar = 1.0 + 3.3 * t - 0.008 * t * t + 27.0 * t ** 3
    expect = ( fstar - 1.0 ) * math.cos( sweep50 ) ** 2 + 1.0
    assert abs( ff - expect ) < 1e-3, "FF %g, expected %g" % ( ff, expect )


def testDatcomFormFactorFollowsTheMaximumThicknessLocation():
    """DATCOM's L was chosen from the percent laminar flow instead of where the section is thickest."""
    def datcom( thickloc, perclam ):
        fresh()
        w = vsp.AddGeom( "WING" )
        xs = vsp.GetXSecSurf( w, 0 )
        for i in range( vsp.GetNumXSec( xs ) ):
            vsp.ChangeXSecShape( xs, i, vsp.XS_FOUR_DIGIT_MOD )
            vsp.SetParmVal( vsp.GetXSecParm( vsp.GetXSec( xs, i ), "ThickLoc" ), thickloc )
        vsp.SetParmVal( w, "FFWingEqnType", "ParasiteDragProps", vsp.FF_W_DATCOM )
        vsp.SetParmVal( w, "PercLam", "ParasiteDragProps", perclam )
        vsp.Update()
        vsp.SetAnalysisInputDefaults( "ParasiteDrag" )
        rid = vsp.ExecAnalysis( "ParasiteDrag" )
        drop_errors()
        return vsp.GetDoubleResults( rid, "Comp_FFOut" )[0], vsp.GetDoubleResults( rid, "Comp_FineRat" )[0]

    aft, t = datcom( 0.5, 0.0 )
    assert abs( datcom( 0.5, 50.0 )[0] - aft ) < 1e-9, "laminar flow changed the DATCOM form factor"
    fwd, t2 = datcom( 0.2, 0.0 )
    assert abs( t - t2 ) < 2e-3

    # L is 1.2 when the maximum thickness is at or aft of 30% chord, 2.0 ahead of it
    expect = ( 1.0 + 1.2 * t + 100.0 * t ** 4 ) / ( 1.0 + 2.0 * t + 100.0 * t ** 4 )
    assert abs( aft / fwd - expect ) < 2e-3, "FF ratio %g, expected %g" % ( aft / fwd, expect )


def testRoughnessHeightIsInTheModelLengthUnit():
    """The roughness height, labelled in the model unit like L_ref, was scaled by 12 in ft and 39.37 in m."""
    for unit in ( vsp.LEN_IN, vsp.LEN_FT, vsp.LEN_M ):
        fresh()
        p = vsp.AddGeom( "POD" )
        vsp.SetParmVal( p, "Roughness", "ParasiteDragProps", 0.001 )
        vsp.Update()
        vsp.SetAnalysisInputDefaults( "ParasiteDrag" )
        vsp.SetIntAnalysisInput( "ParasiteDrag", "LengthUnit", [ unit ] )
        vsp.SetIntAnalysisInput( "ParasiteDrag", "TurbCfEqnChoice", [ vsp.CF_TURB_ROUGHNESS_SCHLICHTING_AVG ] )
        rid = vsp.ExecAnalysis( "ParasiteDrag" )
        drop_errors()
        lref = vsp.GetDoubleResults( rid, "Comp_Lref" )[0]
        cf = vsp.GetDoubleResults( rid, "Comp_Cf" )[0]
        expect = ( 1.89 + 1.62 * math.log10( lref / 0.001 ) ) ** -2.5
        assert abs( cf - expect ) < 1e-9, "unit %d: Cf %g, expected %g" % ( unit, cf, expect )


def testRoughCfOverTheLaminarRunUsesItsLength():
    """The turbulent Cf removed over the laminar run used the whole length's L/k."""
    fresh()
    p = vsp.AddGeom( "POD" )
    vsp.SetParmVal( p, "Roughness", "ParasiteDragProps", 0.001 )
    vsp.SetParmVal( p, "PercLam", "ParasiteDragProps", 30.0 )
    vsp.Update()
    vsp.SetAnalysisInputDefaults( "ParasiteDrag" )
    vsp.SetIntAnalysisInput( "ParasiteDrag", "TurbCfEqnChoice", [ vsp.CF_TURB_ROUGHNESS_SCHLICHTING_AVG ] )
    rid = vsp.ExecAnalysis( "ParasiteDrag" )
    drop_errors()
    lref = vsp.GetDoubleResults( rid, "Comp_Lref" )[0]
    re = vsp.GetDoubleResults( rid, "Comp_Re" )[0]
    cf = vsp.GetDoubleResults( rid, "Comp_Cf" )[0]

    def rough( length ):
        return ( 1.89 + 1.62 * math.log10( length / 0.001 ) ) ** -2.5
    x = 0.3
    expect = rough( lref ) - x * rough( x * lref ) + x * 1.32824 / math.sqrt( x * re )
    assert abs( cf - expect ) < 1e-9, "Cf %g, expected %g" % ( cf, expect )


def testAShortLaminarRunBarelyChangesCf():
    """A laminar run of a few thousandths of a percent put the turbulent fits at Re near their poles."""
    eqns = ( vsp.CF_TURB_EXPLICIT_FIT_SPALDING, vsp.CF_TURB_EXPLICIT_FIT_SPALDING_CHI, vsp.CF_TURB_EXPLICIT_FIT_SCHOENHERR,
             vsp.CF_TURB_IMPLICIT_SCHOENHERR, vsp.CF_TURB_IMPLICIT_KARMAN_SCHOENHERR, vsp.CF_TURB_POWER_LAW_BLASIUS,
             vsp.CF_TURB_POWER_LAW_PRANDTL_LOW_RE, vsp.CF_TURB_POWER_LAW_PRANDTL_MEDIUM_RE,
             vsp.CF_TURB_POWER_LAW_PRANDTL_HIGH_RE, vsp.CF_TURB_SCHLICHTING_COMPRESSIBLE,
             vsp.CF_TURB_SCHULTZ_GRUNOW_SCHOENHERR )
    for eq in eqns:
        cfs = []
        for perclam in ( 0.0, 0.0001, 0.002, 0.005, 0.01, 100.0 ):
            fresh()
            p = vsp.AddGeom( "POD" )
            vsp.SetParmVal( p, "PercLam", "ParasiteDragProps", perclam )
            vsp.Update()
            vsp.SetAnalysisInputDefaults( "ParasiteDrag" )
            vsp.SetIntAnalysisInput( "ParasiteDrag", "TurbCfEqnChoice", [ eq ] )
            vsp.SetIntAnalysisInput( "ParasiteDrag", "FreestreamPropChoice", [ vsp.ATMOS_TYPE_MANUAL_RE_L ] )
            vsp.SetDoubleAnalysisInput( "ParasiteDrag", "Re_L", [ 1.0e5 ] )
            rid = vsp.ExecAnalysis( "ParasiteDrag" )
            drop_errors()
            cfs.append( vsp.GetDoubleResults( rid, "Comp_Cf" )[0] )
            re = vsp.GetDoubleResults( rid, "Comp_Re" )[0]
        for cf in cfs[ 1:-1 ]:
            assert math.isfinite( cf ) and abs( cf / cfs[0] - 1.0 ) < 1e-3, "equation %d: %s" % ( eq, cfs )
        assert abs( cfs[-1] - 1.32824 / math.sqrt( re ) ) < 1e-12, "equation %d all laminar: %g" % ( eq, cfs[-1] )


def testASeparateSubsurfaceUsesItsGeomsRoughnessUntilSet():
    """A subsurface's roughness starts at -1, unset, and went into the roughness Cf as is: NaN."""
    def cfs( ssrough ):
        fresh()
        p = vsp.AddGeom( "POD" )
        vsp.SetParmVal( p, "Roughness", "ParasiteDragProps", 0.001 )
        vsp.Update()
        s = vsp.AddSubSurf( p, vsp.SS_RECTANGLE, 0 )
        vsp.SetSubSurfName( s, "A" )
        vsp.SetParmVal( vsp.GetParm( s, "IncludeFlag", "SubSurface" ), vsp.SS_INC_SEPARATE_TREATMENT )
        if ssrough is not None:
            vsp.SetParmVal( vsp.GetParm( s, "Roughness", "ParasiteDragProps" ), ssrough )
        vsp.Update()
        vsp.SetAnalysisInputDefaults( "ParasiteDrag" )
        vsp.SetIntAnalysisInput( "ParasiteDrag", "TurbCfEqnChoice", [ vsp.CF_TURB_ROUGHNESS_SCHLICHTING_AVG ] )
        vsp.SetIntAnalysisInput( "ParasiteDrag", "ExportSubCompFlag", [ 1 ] )
        rid = vsp.ExecAnalysis( "ParasiteDrag" )
        drop_errors()
        return dict( zip( vsp.GetStringResults( rid, "Comp_Label" ), vsp.GetDoubleResults( rid, "Comp_Cf" ) ) )

    unset = cfs( None )
    assert math.isfinite( unset[ "[ss] A" ] ), unset
    assert abs( unset[ "[ss] A" ] - unset[ "PodGeom" ] ) < 1e-12, unset
    rougher = cfs( 0.01 )
    assert rougher[ "[ss] A" ] > rougher[ "PodGeom" ], rougher


def testARoughnessEquationOnASmoothPlateUsesTheDefault():
    """With no roughness height, L/k was infinite and the roughness equations gave Cf = 0; a nearly
    smooth plate got a third of the smooth friction."""
    def cf( eq, k ):
        fresh()
        p = vsp.AddGeom( "POD" )
        vsp.SetParmVal( p, "Roughness", "ParasiteDragProps", k )
        vsp.Update()
        vsp.SetAnalysisInputDefaults( "ParasiteDrag" )
        vsp.SetIntAnalysisInput( "ParasiteDrag", "TurbCfEqnChoice", [ eq ] )
        rid = vsp.ExecAnalysis( "ParasiteDrag" )
        drop_errors()
        mach = vsp.GetDoubleResults( rid, "FC_Mach" )[0]
        if eq == vsp.CF_TURB_ROUGHNESS_SCHLICHTING_AVG_FLOW_CORRECTION:
            return vsp.GetDoubleResults( rid, "Comp_Cf" )[0] * ( 1.0 + 0.2 * mach * mach ) ** 0.467
        return vsp.GetDoubleResults( rid, "Comp_Cf" )[0]

    # The flow corrected equation's Mach factor is taken out to compare
    smooth = cf( vsp.CF_TURB_SCHLICHTING_COMPRESSIBLE, 0.0 )
    for eq in ( vsp.CF_TURB_ROUGHNESS_SCHLICHTING_AVG, vsp.CF_TURB_ROUGHNESS_SCHLICHTING_AVG_FLOW_CORRECTION ):
        assert abs( cf( eq, 0.0 ) - smooth ) < 1e-12, "equation %d on a smooth plate" % eq
        assert abs( cf( eq, 1e-9 ) - smooth ) < 1e-12, "equation %d on a nearly smooth plate" % eq
        assert cf( eq, 0.01 ) > smooth, "equation %d on a rough plate" % eq


def testRoughnessFlowCorrectionUsesMachSquared():
    """The correction is built on the temperature ratio 1 + (gamma - 1)/2 M^2; the code used M."""
    def cf( eq ):
        fresh()
        p = vsp.AddGeom( "POD" )
        vsp.SetParmVal( p, "Roughness", "ParasiteDragProps", 0.001 )
        vsp.Update()
        vsp.SetAnalysisInputDefaults( "ParasiteDrag" )
        vsp.SetIntAnalysisInput( "ParasiteDrag", "TurbCfEqnChoice", [ eq ] )
        vsp.SetIntAnalysisInput( "ParasiteDrag", "FreestreamPropChoice", [ vsp.ATMOS_TYPE_MANUAL_RE_L ] )
        vsp.SetDoubleAnalysisInput( "ParasiteDrag", "Re_L", [ 1.0e6 ] )
        vsp.SetDoubleAnalysisInput( "ParasiteDrag", "Mach", [ 2.0 ] )
        rid = vsp.ExecAnalysis( "ParasiteDrag" )
        drop_errors()
        return vsp.GetDoubleResults( rid, "Comp_Cf" )[0]

    ratio = cf( vsp.CF_TURB_ROUGHNESS_SCHLICHTING_AVG_FLOW_CORRECTION ) / cf( vsp.CF_TURB_ROUGHNESS_SCHLICHTING_AVG )
    expect = ( 1.0 + 0.2 * 2.0 * 2.0 ) ** -0.467
    assert abs( ratio - expect ) < 1e-9, "ratio %g, expected %g" % ( ratio, expect )


def testATemperatureBelowZeroCelsiusIsKept():
    """The analysis set the unit but left the lower limit of the previous one, so -17.5 C became 0 C."""
    def re( unit, temp ):
        vsp.SetAnalysisInputDefaults( "ParasiteDrag" )
        vsp.SetIntAnalysisInput( "ParasiteDrag", "FreestreamPropChoice", [ vsp.ATMOS_TYPE_MANUAL_P_T ] )
        vsp.SetIntAnalysisInput( "ParasiteDrag", "TempUnit", [ unit ] )
        vsp.SetDoubleAnalysisInput( "ParasiteDrag", "Temperature", [ temp ] )
        rid = vsp.ExecAnalysis( "ParasiteDrag" )
        drop_errors()
        return vsp.GetDoubleResults( rid, "Comp_Re" )[0]

    fresh()
    vsp.AddGeom( "POD" )
    vsp.Update()
    kelvin = re( vsp.TEMP_UNIT_K, 273.15 - 17.5 )
    celsius = re( vsp.TEMP_UNIT_C, -17.5 )
    assert abs( celsius / kelvin - 1.0 ) < 1e-6, "Re %g in C, %g in K" % ( celsius, kelvin )


def testReLModeFormFactorUsesItsOwnMach():
    """In Re/L mode the form factors kept the Mach of the previous run."""
    def ff( mode, mach ):
        vsp.SetAnalysisInputDefaults( "ParasiteDrag" )
        vsp.SetIntAnalysisInput( "ParasiteDrag", "FreestreamPropChoice", [ mode ] )
        if mode == vsp.ATMOS_TYPE_MANUAL_RE_L:
            vsp.SetDoubleAnalysisInput( "ParasiteDrag", "Mach", [ mach ] )
        else:
            vsp.SetIntAnalysisInput( "ParasiteDrag", "VelocityUnit", [ vsp.V_UNIT_MACH ] )
            vsp.SetDoubleAnalysisInput( "ParasiteDrag", "Vinf", [ mach ] )
        rid = vsp.ExecAnalysis( "ParasiteDrag" )
        drop_errors()
        return vsp.GetDoubleResults( rid, "Comp_FFOut" )[0]

    fresh()
    w = vsp.AddGeom( "WING" )
    vsp.SetParmVal( w, "FFWingEqnType", "ParasiteDragProps", vsp.FF_W_KROO )
    vsp.Update()
    expect = ff( vsp.ATMOS_TYPE_US_STANDARD_1976, 0.2 )
    ff( vsp.ATMOS_TYPE_US_STANDARD_1976, 0.7 )
    got = ff( vsp.ATMOS_TYPE_MANUAL_RE_L, 0.2 )
    assert abs( got - expect ) < 1e-12, "FF %g, expected %g" % ( got, expect )


def totals( rid ):
    names = ( "Geom_f_Total", "Geom_CD_Total", "Geom_Perc_Total", "Total_f_Total", "Total_CD_Total", "Total_Perc_Total" )
    return dict( ( n, vsp.GetDoubleResults( rid, n )[0] ) for n in names )


def testTheGeometryTotalsAgree():
    """The f total and percentages summed the non-master rows, the CD total the master rows."""
    def run( build ):
        fresh()
        build()
        vsp.Update()
        vsp.SetAnalysisInputDefaults( "ParasiteDrag" )
        vsp.SetIntAnalysisInput( "ParasiteDrag", "RefFlag", [ vsp.MANUAL_REF ] )
        vsp.SetDoubleAnalysisInput( "ParasiteDrag", "Sref", [ 1.0 ] )
        rid = vsp.ExecAnalysis( "ParasiteDrag" )
        drop_errors()
        return totals( rid )

    def grouped():
        a = vsp.AddGeom( "POD" )
        b = vsp.AddGeom( "POD", a )
        vsp.SetParmVal( b, "Y_Rel_Location", "XForm", 5.0 )
        vsp.SetParmVal( b, "IncorporatedGen", "ParasiteDragProps", 1 )

    def separate_first():
        p = vsp.AddGeom( "POD" )
        vsp.Update()
        for name, u, inc in ( ( "A", 0.25, vsp.SS_INC_SEPARATE_TREATMENT ), ( "B", 0.75, vsp.SS_INC_TREAT_AS_PARENT ) ):
            s = vsp.AddSubSurf( p, vsp.SS_RECTANGLE, 0 )
            vsp.SetParmVal( vsp.GetParm( s, "Center_U", "SS_Rectangle" ), u )
            vsp.SetParmVal( vsp.GetParm( s, "IncludeFlag", "SubSurface" ), inc )
            vsp.SetParmVal( vsp.GetParm( s, "Q", "ParasiteDragProps" ), 3.0 )

    for build in ( grouped, separate_first ):
        t = run( build )
        assert abs( t[ "Geom_f_Total" ] - t[ "Geom_CD_Total" ] ) < 1e-12, ( build.__name__, t )
        assert abs( t[ "Total_f_Total" ] - t[ "Total_CD_Total" ] ) < 1e-12, ( build.__name__, t )
        assert abs( t[ "Geom_Perc_Total" ] - 1.0 ) < 1e-12, ( build.__name__, t )
        assert abs( t[ "Total_Perc_Total" ] - 1.0 ) < 1e-12, ( build.__name__, t )


def testASubsurfaceTreatedAsParentShowsItsParentsValues():
    """Its row copied the row before it, which could be a Separate subsurface with its own Q and Cf."""
    fresh()
    p = vsp.AddGeom( "POD" )
    vsp.SetParmVal( p, "FFBodyEqnType", "ParasiteDragProps", vsp.FF_B_TORENBEEK )
    vsp.Update()
    for name, u, inc in ( ( "A", 0.25, vsp.SS_INC_SEPARATE_TREATMENT ), ( "B", 0.75, vsp.SS_INC_TREAT_AS_PARENT ) ):
        s = vsp.AddSubSurf( p, vsp.SS_RECTANGLE, 0 )
        vsp.SetSubSurfName( s, name )
        vsp.SetParmVal( vsp.GetParm( s, "Center_U", "SS_Rectangle" ), u )
        vsp.SetParmVal( vsp.GetParm( s, "IncludeFlag", "SubSurface" ), inc )
        vsp.SetParmVal( vsp.GetParm( s, "Q", "ParasiteDragProps" ), 3.0 )
        vsp.SetParmVal( vsp.GetParm( s, "PercLam", "ParasiteDragProps" ), 50.0 )
    vsp.Update()
    vsp.SetAnalysisInputDefaults( "ParasiteDrag" )
    vsp.SetIntAnalysisInput( "ParasiteDrag", "ExportSubCompFlag", [ 1 ] )
    rid = vsp.ExecAnalysis( "ParasiteDrag" )
    drop_errors()
    labels = list( vsp.GetStringResults( rid, "Comp_Label" ) )
    pod = labels.index( "PodGeom" )
    b = labels.index( "[ss] B_0" )
    for name in ( "Comp_Q", "Comp_Cf", "Comp_FFOut", "Comp_PercLam" ):
        vals = vsp.GetDoubleResults( rid, name )
        assert vals[ b ] == vals[ pod ], "%s: %g, parent %g" % ( name, vals[ b ], vals[ pod ] )


def testGroupedAreaReachesADragCarryingRow():
    """A grouped Geom's area was lost when its ancestor was grouped too, not in the set, or of
    another shape type."""
    def run( group, geomset = vsp.SET_ALL ):
        fresh()
        f = vsp.AddGeom( "FUSELAGE" )
        p = vsp.AddGeom( "POD", f )
        vsp.SetParmVal( p, "Z_Rel_Location", "XForm", 10.0 )
        g = vsp.AddGeom( "POD", p )
        vsp.SetParmVal( g, "Z_Rel_Location", "XForm", 10.0 )
        w = vsp.AddGeom( "WING", g )
        vsp.SetParmVal( w, "Z_Rel_Location", "XForm", 10.0 )
        group( f, p, g, w )
        vsp.Update()
        vsp.SetAnalysisInputDefaults( "ParasiteDrag" )
        vsp.SetIntAnalysisInput( "ParasiteDrag", "GeomSet", [ geomset ] )
        rid = vsp.ExecAnalysis( "ParasiteDrag" )
        drop_errors()
        swet = vsp.GetDoubleResults( rid, "Comp_Swet" )
        cd = vsp.GetDoubleResults( rid, "Comp_CD" )
        return sum( a for a, c in zip( swet, cd ) if c > 0 )

    def gen( gid, n ):
        vsp.SetParmVal( gid, "IncorporatedGen", "ParasiteDragProps", n )

    whole = run( lambda f, p, g, w: None )

    # Pod into its parent pod, which goes into the fuselage
    chained = run( lambda f, p, g, w: ( gen( p, 1 ), gen( g, 1 ) ) )
    assert abs( chained - whole ) < 1e-6, ( chained, whole )

    # A wing grouped into a pod keeps its own drag
    mixed = run( lambda f, p, g, w: gen( w, 1 ) )
    assert abs( mixed - whole ) < 1e-6, ( mixed, whole )

    # A pod grouped into a fuselage outside the set keeps its own drag
    def outside( f, p, g, w ):
        gen( p, 1 )
        for gid in ( p, g, w ):
            vsp.SetSetFlag( gid, vsp.SET_FIRST_USER, True )
    alone = run( lambda f, p, g, w: [ vsp.SetSetFlag( gid, vsp.SET_FIRST_USER, True ) for gid in ( p, g, w ) ], vsp.SET_FIRST_USER )
    assert abs( run( outside, vsp.SET_FIRST_USER ) - alone ) < 1e-6


def testAreaWhereSubsurfacesOverlapIsCountedOnce():
    """CompGeom tags a region inside two subsurfaces with both; no row read that tag, so it was lost."""
    def run( inca, incb ):
        fresh()
        p = vsp.AddGeom( "POD" )
        vsp.Update()
        ids = []
        for name, u, inc in ( ( "A", 0.4, inca ), ( "B", 0.5, incb ) ):
            s = vsp.AddSubSurf( p, vsp.SS_RECTANGLE, 0 )
            vsp.SetSubSurfName( s, name )
            vsp.SetParmVal( vsp.GetParm( s, "Center_U", "SS_Rectangle" ), u )
            vsp.SetParmVal( vsp.GetParm( s, "IncludeFlag", "SubSurface" ), inc )
            ids.append( s )
        vsp.Update()
        vsp.SetAnalysisInputDefaults( "ParasiteDrag" )
        vsp.SetIntAnalysisInput( "ParasiteDrag", "ExportSubCompFlag", [ 1 ] )
        rid = vsp.ExecAnalysis( "ParasiteDrag" )
        drop_errors()
        rows = dict( zip( vsp.GetStringResults( rid, "Comp_Label" ), vsp.GetDoubleResults( rid, "Comp_Swet" ) ) )
        cg = vsp.ExecAnalysis( "CompGeom" )
        tags = dict( zip( vsp.GetStringResults( cg, "Tag_ID" ), vsp.GetDoubleResults( cg, "Tag_Wet_Area" ) ) )
        total = sum( tags.values() )
        in_a = sum( a for t, a in tags.items() if ids[0] in t )
        return rows, total, in_a

    parent = vsp.SS_INC_TREAT_AS_PARENT
    separate = vsp.SS_INC_SEPARATE_TREATMENT

    rows, total, in_a = run( parent, parent )
    assert abs( rows[ "PodGeom" ] - total ) < 1e-6, ( rows, total )

    rows, total, in_a = run( separate, parent )
    assert abs( rows[ "PodGeom" ] + rows[ "[ss] A" ] - total ) < 1e-6, ( rows, total )
    assert abs( rows[ "[ss] A" ] - in_a ) < 1e-6, ( rows, in_a )

    rows, total, in_a = run( separate, separate )
    assert abs( rows[ "PodGeom" ] + rows[ "[ss] A" ] + rows[ "[ss] B" ] - total ) < 1e-6, ( rows, total )

    rows, total, in_a = run( vsp.SS_INC_ZERO_DRAG, parent )
    assert abs( rows[ "PodGeom" ] - ( total - in_a ) ) < 1e-6, ( rows, total, in_a )


def testTheMarginIsItsShareOfTheTotal():
    """A margin ahead of a % of CD_Geom or drag area excrescence was taken before they were updated."""
    for order in ( ( 0, 1, 2 ), ( 1, 2, 0 ) ):
        fresh()
        vsp.AddGeom( "POD" )
        vsp.Update()
        ex = ( ( "m", vsp.EXCRESCENCE_MARGIN, 5.0 ), ( "g", vsp.EXCRESCENCE_PERCENT_GEOM, 10.0 ),
               ( "d", vsp.EXCRESCENCE_DRAGAREA, 0.2 ) )
        for k in order:
            vsp.AddExcrescence( *ex[k] )
        vsp.SetAnalysisInputDefaults( "ParasiteDrag" )
        rid = vsp.ExecAnalysis( "ParasiteDrag" )
        drop_errors()
        labels = list( vsp.GetStringResults( rid, "Excres_Label" ) )
        perc = vsp.GetDoubleResults( rid, "Excres_PercTotalCD" )
        assert abs( perc[ labels.index( "m" ) ] - 0.05 ) < 1e-9, ( order, list( perc ) )
        t = totals( rid )
        sref = t[ "Total_f_Total" ] / t[ "Total_CD_Total" ]
        excf = vsp.GetDoubleResults( rid, "Excres_f_Total" )[0]
        excd = vsp.GetDoubleResults( rid, "Excres_CD_Total" )[0]
        assert abs( excf - excd * sref ) < 1e-9, ( order, excf, excd * sref )


def testAMarginCannotReachTheWholeTotal():
    """A margin of 100% made the total infinite, above it negative."""
    fresh()
    vsp.AddGeom( "POD" )
    vsp.Update()
    vsp.AddExcrescence( "m", vsp.EXCRESCENCE_MARGIN, 150.0 )
    vsp.SetAnalysisInputDefaults( "ParasiteDrag" )
    rid = vsp.ExecAnalysis( "ParasiteDrag" )
    drop_errors()
    total = vsp.GetDoubleResults( rid, "Total_CD_Total" )[0]
    assert math.isfinite( total ) and total > 0, total


def testASeparateSubsurfaceIsAComponentOfTheResults():
    """Without sub-components the results kept one row per Geom ID, dropping a Separate subsurface."""
    fresh()
    p = vsp.AddGeom( "POD" )
    vsp.Update()
    s = vsp.AddSubSurf( p, vsp.SS_RECTANGLE, 0 )
    vsp.SetSubSurfName( s, "A" )
    vsp.SetParmVal( vsp.GetParm( s, "IncludeFlag", "SubSurface" ), vsp.SS_INC_SEPARATE_TREATMENT )
    vsp.Update()
    vsp.SetAnalysisInputDefaults( "ParasiteDrag" )
    vsp.SetIntAnalysisInput( "ParasiteDrag", "ExportSubCompFlag", [ 0 ] )
    rid = vsp.ExecAnalysis( "ParasiteDrag" )
    drop_errors()
    assert list( vsp.GetStringResults( rid, "Comp_Label" ) ) == [ "PodGeom", "[ss] A" ]
    f = sum( vsp.GetDoubleResults( rid, "Comp_f" ) )
    assert abs( f - vsp.GetDoubleResults( rid, "Geom_f_Total" )[0] ) < 1e-12


def testWingReferenceLengthIsTheMeanAerodynamicChord():
    """The strips were weighted by their length along the leading edge, so a swept panel counted extra."""
    for sweep in ( 0.0, 60.0 ):
        fresh()
        w = vsp.AddGeom( "WING" )
        vsp.InsertXSec( w, 1, vsp.XS_FOUR_SERIES )
        vsp.Update()
        for sec, span, root, tip, swp in ( ( 1, 2.0, 4.0, 4.0, 0.0 ), ( 2, 8.0, 4.0, 1.0, sweep ) ):
            g = "XSec_%d" % sec
            for name, val in ( ( "Sweep_Location", 0.0 ), ( "Span", span ), ( "Root_Chord", root ),
                               ( "Tip_Chord", tip ), ( "Sweep", swp ) ):
                vsp.SetParmVal( w, name, g, val )
        vsp.Update()
        vsp.SetAnalysisInputDefaults( "ParasiteDrag" )
        rid = vsp.ExecAnalysis( "ParasiteDrag" )
        drop_errors()

        # integral( c^2 ) / integral( c ): inner 2 * 16 and 2 * 4, outer 8 * 21 / 3 and 8 * 2.5
        mac = ( 32.0 + 56.0 ) / ( 8.0 + 20.0 )
        lref = vsp.GetDoubleResults( rid, "Comp_Lref" )[0]
        assert abs( lref - mac ) < 1e-6, "sweep %g: L_ref %g, MAC %g" % ( sweep, lref, mac )


def lref_of_the_first_component():
    vsp.SetAnalysisInputDefaults( "ParasiteDrag" )
    rid = vsp.ExecAnalysis( "ParasiteDrag" )
    drop_errors()
    return vsp.GetDoubleResults( rid, "Comp_Lref" )[0]


def testACurvedWingTakesItsOwnMeanAerodynamicChord():
    """Straight strips across a curved leading edge missed the MAC the wing integrates exactly; a Clone has its original's."""
    fresh()
    w = vsp.AddGeom( "WING" )
    vsp.InsertXSec( w, 1, vsp.XS_FOUR_SERIES )
    vsp.Update()
    for sec, span, root, tip, swp in ( ( 1, 2.0, 4.0, 4.0, 0.0 ), ( 2, 8.0, 4.0, 1.0, 40.0 ) ):
        g = "XSec_%d" % sec
        for name, val in ( ( "Span", span ), ( "Root_Chord", root ), ( "Tip_Chord", tip ), ( "Sweep", swp ) ):
            vsp.SetParmVal( w, name, g, val )
    vsp.SetParmVal( w, "InLEMode", "XSec_2", vsp.BLEND_MATCH_IN_LE_TRAP )
    vsp.SetParmVal( w, "OutLEMode", "XSec_1", vsp.BLEND_MATCH_IN_LE_TRAP )
    vsp.SetParmVal( w, "InLEStrength", "XSec_2", 1.0 )
    vsp.Update()
    vsp.CloneGeomVec( [ w ] )
    vsp.Update()
    mac = vsp.GetParmVal( w, "MAC", "WingGeom" )
    vsp.SetAnalysisInputDefaults( "ParasiteDrag" )
    rid = vsp.ExecAnalysis( "ParasiteDrag" )
    drop_errors()
    # The wing, then its Clone
    for lref in vsp.GetDoubleResults( rid, "Comp_Lref" ):
        assert abs( lref - mac ) < 1e-9, "L_ref %g, MAC %g" % ( lref, mac )


def testAPropBladeTakesItsMeanAerodynamicChord():
    """A blade is treated as a wing: integral( c^2 ) / integral( c ) over r/R from the root."""
    fresh()
    p = vsp.AddGeom( "PROP" )
    vsp.SetPCurve( p, vsp.PROP_CHORD, [ 0.2, 1.0 ], [ 0.2, 0.1 ], vsp.LINEAR )
    vsp.Update()
    radius = 0.5 * vsp.GetParmVal( p, "Diameter", "Design" )
    c0, c1 = 0.2, 0.1
    mac = radius * 2.0 * ( c0 * c0 + c0 * c1 + c1 * c1 ) / ( 3.0 * ( c0 + c1 ) )
    lref = lref_of_the_first_component()
    assert abs( lref - mac ) < 1e-6, "L_ref %g, MAC %g" % ( lref, mac )


def testABodyOfRevolutionTakesItsSectionChord():
    fresh()
    b = vsp.AddGeom( "BODYOFREVOLUTION" )
    vsp.SetParmVal( b, "Circle_Diameter", "XSecCurve", 2.5 )
    vsp.Update()
    lref = lref_of_the_first_component()
    assert abs( lref - 2.5 ) < 1e-9, "L_ref %g" % lref


def testAFormFactorOutsideItsRangeIsReported():
    """Shevell's form factor has sqrt( 1 - M^2 cos^2 ); above Mach 1 it is NaN and the wing silently had no drag."""
    fresh()
    w = vsp.AddGeom( "WING" )
    vsp.SetParmVal( w, "FFWingEqnType", "ParasiteDragProps", vsp.FF_W_SHEVELL )
    vsp.Update()
    vsp.SetAnalysisInputDefaults( "ParasiteDrag" )
    vsp.SetIntAnalysisInput( "ParasiteDrag", "VelocityUnit", [ vsp.V_UNIT_MACH ] )
    vsp.SetDoubleAnalysisInput( "ParasiteDrag", "Vinf", [ 1.5 ] )
    vsp.ExecAnalysis( "ParasiteDrag" )
    assert any( "No drag for" in e for e in pop_errors() )


def testTheCfTableHasEveryEquationInUse():
    """The table stopped at White-Christoph compressible, before the two roughness equations."""
    fname = os.path.join( tempfile.mkdtemp(), "cf.csv" )
    vsp.WriteCfEqnCSVFile( fname )
    text = open( fname ).read()
    for name in ( "Schlichting Avg Roughness", "Schlichting Avg Roughness w Flow Correction" ):
        assert ( name + "," ) in text, name


def testAPropDiskCarriesNoDrag():
    """A PROP_BOTH disk row read the blades' area, showed drag in no total, and went into an ancestor."""
    carried = []
    for gen in ( 0, 1 ):
        fresh()
        w = vsp.AddGeom( "WING" )
        p = vsp.AddGeom( "PROP", w )
        vsp.SetParmVal( p, "Z_Rel_Location", "XForm", 20.0 )
        vsp.SetParmVal( p, "PropMode", "Design", vsp.PROP_BOTH )
        vsp.SetParmVal( p, "IncorporatedGen", "ParasiteDragProps", gen )
        vsp.Update()
        vsp.SetAnalysisInputDefaults( "ParasiteDrag" )
        rid = vsp.ExecAnalysis( "ParasiteDrag" )
        drop_errors()
        swet = vsp.GetDoubleResults( rid, "Comp_Swet" )
        cd = vsp.GetDoubleResults( rid, "Comp_CD" )
        carried.append( sum( a for a, c in zip( swet, cd ) if c > 0 ) )
        t = totals( rid )
        assert abs( t[ "Geom_Perc_Total" ] - 1.0 ) < 1e-12, t

        vsp.SetIntAnalysisInput( "ParasiteDrag", "ExportSubCompFlag", [ 1 ] )
        rid = vsp.ExecAnalysis( "ParasiteDrag" )
        drop_errors()
        perc = vsp.GetDoubleResults( rid, "Comp_PercTotalCD" )
        master = [ l for l in vsp.GetStringResults( rid, "Comp_Label" ) ]
        rows = sum( pc for l, pc in zip( master, perc ) if l.startswith( "PropGeom_" ) or l.startswith( "WingGeom_" ) )
        assert rows <= 1.0 + 1e-9, "row shares sum to %g" % rows
    assert abs( carried[0] - carried[1] ) < 1e-6, carried


def testAnInfiniteFormFactorIsNoDragToo():
    """Calculate_CD zeroed a NaN f but kept an infinite one as infinite drag."""
    fresh()
    w = vsp.AddGeom( "WING" )
    vsp.SetParmVal( w, "FFWingEqnType", "ParasiteDragProps", vsp.FF_W_SHEVELL )
    vsp.SetParmVal( w, "Sweep", "XSec_1", 0.0 )
    vsp.SetParmVal( w, "Tip_Chord", "XSec_1", vsp.GetParmVal( w, "Root_Chord", "XSec_1" ) )
    vsp.Update()
    vsp.SetAnalysisInputDefaults( "ParasiteDrag" )
    vsp.SetIntAnalysisInput( "ParasiteDrag", "VelocityUnit", [ vsp.V_UNIT_MACH ] )
    vsp.SetDoubleAnalysisInput( "ParasiteDrag", "Vinf", [ 1.0 ] )
    rid = vsp.ExecAnalysis( "ParasiteDrag" )
    drop_errors()
    assert math.isfinite( vsp.GetDoubleResults( rid, "Total_CD_Total" )[0] )


def testCfIsContinuousAsTheLaminarRunGrows():
    """Below Re_lam 1e3 the laminar run was dropped; above it, it added friction: a 2-3% step."""
    for eq in ( vsp.CF_TURB_SCHLICHTING_COMPRESSIBLE, vsp.CF_TURB_POWER_LAW_BLASIUS, vsp.CF_TURB_EXPLICIT_FIT_SPALDING ):
        cfs = []
        turb = None
        for perclam in [ 0.0 ] + [ 0.5 + 0.1 * k for k in range( 20 ) ]:
            fresh()
            p = vsp.AddGeom( "POD" )
            vsp.SetParmVal( p, "PercLam", "ParasiteDragProps", perclam )
            vsp.Update()
            vsp.SetAnalysisInputDefaults( "ParasiteDrag" )
            vsp.SetIntAnalysisInput( "ParasiteDrag", "TurbCfEqnChoice", [ eq ] )
            vsp.SetIntAnalysisInput( "ParasiteDrag", "FreestreamPropChoice", [ vsp.ATMOS_TYPE_MANUAL_RE_L ] )
            vsp.SetDoubleAnalysisInput( "ParasiteDrag", "Re_L", [ 1.0e4 ] )
            rid = vsp.ExecAnalysis( "ParasiteDrag" )
            drop_errors()
            cf = vsp.GetDoubleResults( rid, "Comp_Cf" )[0]
            if turb is None:
                turb = cf
            else:
                cfs.append( cf )
        assert max( cfs ) <= turb * ( 1.0 + 1e-12 ), "equation %d: laminar flow raised Cf %s over %g" % ( eq, cfs, turb )
        for a, b in zip( cfs, cfs[ 1: ] ):
            assert abs( b - a ) < 2e-3 * turb, "equation %d: step %g" % ( eq, b - a )


def testDatcomFormFactorDoesNotDependOnTessellation():
    """The sampled thickest point of a NACA 0010, thickest at 30%, read 0.22 to 0.32 with Tess_W, so L flipped."""
    ffs = []
    for tess in ( 13, 21, 33, 41 ):
        fresh()
        w = vsp.AddGeom( "WING" )
        vsp.SetParmVal( w, "Tess_W", "Shape", tess )
        vsp.SetParmVal( w, "FFWingEqnType", "ParasiteDragProps", vsp.FF_W_DATCOM )
        vsp.Update()
        vsp.SetAnalysisInputDefaults( "ParasiteDrag" )
        rid = vsp.ExecAnalysis( "ParasiteDrag" )
        drop_errors()
        ff = vsp.GetDoubleResults( rid, "Comp_FFOut" )[0]
        t = vsp.GetDoubleResults( rid, "Comp_FineRat" )[0]
        ffs.append( ff / ( 1.0 + 1.2 * t + 100.0 * t ** 4 ) )
    assert max( ffs ) - min( ffs ) < 2e-3, ffs


def testRunningTheAnalysisLeavesWhatIsShown():
    """CompGeom hid the set and the analysis then showed it all, so a hidden Geom came back and one
    outside the set stayed hidden; a later run on the Shown set then left Geoms out."""
    fresh()
    f = vsp.AddGeom( "FUSELAGE" )
    w = vsp.AddGeom( "WING" )
    p = vsp.AddGeom( "POD" )
    vsp.SetParmVal( p, "Z_Rel_Location", "XForm", 10.0 )
    vsp.SetSetFlag( p, vsp.SET_SHOWN, False )
    vsp.SetSetFlag( p, vsp.SET_NOT_SHOWN, True )
    for gid in ( w, p ):
        vsp.SetSetFlag( gid, vsp.SET_FIRST_USER, True )
    vsp.Update()

    def shown():
        return [ vsp.GetSetFlag( g, vsp.SET_SHOWN ) for g in ( f, w, p ) ]

    before = shown()
    for geomset in ( vsp.SET_ALL, vsp.SET_FIRST_USER, vsp.SET_SHOWN, vsp.SET_NOT_SHOWN ):
        vsp.SetAnalysisInputDefaults( "ParasiteDrag" )
        vsp.SetIntAnalysisInput( "ParasiteDrag", "GeomSet", [ geomset ] )
        vsp.ExecAnalysis( "ParasiteDrag" )
        drop_errors()
        assert shown() == before, "set %d: %s became %s" % ( geomset, before, shown() )


def testTheReferenceWingDefaultsToTheModels():
    """The analysis defaulted WingID to empty, which left S_ref at whatever it last was, silently."""
    fresh()
    w = vsp.AddGeom( "WING" )
    vsp.Update()
    cid = vsp.FindContainer( "ParasiteDragSettings", 0 )
    vsp.SetParmVal( cid, "RefFlag", "ParasiteDrag", vsp.COMPONENT_REF )
    vsp.Update()
    vsp.SetAnalysisInputDefaults( "ParasiteDrag" )
    vsp.SetIntAnalysisInput( "ParasiteDrag", "RefFlag", [ vsp.COMPONENT_REF ] )
    rid = vsp.ExecAnalysis( "ParasiteDrag" )
    assert not pop_errors()
    sref = vsp.GetDoubleResults( rid, "FC_Sref" )[0]
    assert abs( sref - vsp.GetParmVal( w, "TotalArea", "WingGeom" ) ) < 1e-9, sref

    vsp.SetStringAnalysisInput( "ParasiteDrag", "WingID", [ "NOTAGEOMID" ] )
    vsp.ExecAnalysis( "ParasiteDrag" )
    assert len( [ e for e in pop_errors() if "reference wing" in e ] ) == 1

    # A Geom that is not a wing, reported once
    p = vsp.AddGeom( "POD" )
    vsp.Update()
    vsp.SetStringAnalysisInput( "ParasiteDrag", "WingID", [ p ] )
    vsp.ExecAnalysis( "ParasiteDrag" )
    assert len( [ e for e in pop_errors() if "reference wing" in e ] ) == 1

    # The default input on a fresh run, with no wing chosen yet, is not an error
    fresh()
    vsp.AddGeom( "WING" )
    vsp.Update()
    vsp.SetAnalysisInputDefaults( "ParasiteDrag" )
    vsp.SetIntAnalysisInput( "ParasiteDrag", "RefFlag", [ vsp.COMPONENT_REF ] )
    vsp.ExecAnalysis( "ParasiteDrag" )
    assert not pop_errors()


def testAGearGroundPlaneHasNoDrag():
    """The gear's ground plane, a surface CFD sees through, was 3700 of the gear's 3737 wetted area
    and set the tires' L_ref and form factor."""
    def run( plane ):
        fresh()
        vsp.AddGeom( "FUSELAGE" )
        g = vsp.AddGeom( "GEAR" )
        vsp.CreateAndAddBogie( g )
        vsp.SetParmVal( g, "ShowNominalGroundPlane", "GroundPlane", plane )
        vsp.Update()
        vsp.SetAnalysisInputDefaults( "ParasiteDrag" )
        rid = vsp.ExecAnalysis( "ParasiteDrag" )
        drop_errors()
        return [ vsp.GetDoubleResults( rid, n ) for n in ( "Comp_Swet", "Comp_Lref", "Comp_FFOut", "Comp_f" ) ]

    with_plane = run( 1 )
    without = run( 0 )
    for a, b in zip( with_plane, without ):
        assert max( abs( x - y ) for x, y in zip( a, b ) ) < 1e-9 * max( 1.0, max( abs( y ) for y in b ) ), ( with_plane, without )


def testWingThicknessDoesNotDependOnDihedral():
    """The sampled t/c is measured off the tessellated section, which dihedral stretches by 1/cos."""
    tocs = []
    for dihedral in ( 0.0, 30.0, 60.0 ):
        fresh()
        w = vsp.AddGeom( "WING" )
        vsp.SetParmVal( w, "Dihedral", "XSec_1", dihedral )
        vsp.Update()
        vsp.SetAnalysisInputDefaults( "ParasiteDrag" )
        rid = vsp.ExecAnalysis( "ParasiteDrag" )
        drop_errors()
        tocs.append( vsp.GetDoubleResults( rid, "Comp_FineRat" )[0] )
    assert max( tocs ) - min( tocs ) < 1e-6, tocs
    assert abs( tocs[0] - 0.1 ) < 1e-3, tocs


def testWingThicknessIsTheAreaWeightedMean():
    """The span maximum was used: a prop's elliptical root section made its blade t/c 0.5."""
    fresh()
    w = vsp.AddGeom( "WING" )
    for name, val in ( ( "Root_Chord", 4.0 ), ( "Tip_Chord", 2.0 ), ( "Span", 10.0 ), ( "Sweep", 0.0 ) ):
        vsp.SetParmVal( w, name, "XSec_1", val )
    xs = vsp.GetXSecSurf( w, 0 )
    for i, toc in ( ( 0, 0.18 ), ( 1, 0.08 ) ):
        vsp.SetParmVal( vsp.GetXSecParm( vsp.GetXSec( xs, i ), "ThickChord" ), toc )
    vsp.Update()
    vsp.SetAnalysisInputDefaults( "ParasiteDrag" )
    rid = vsp.ExecAnalysis( "ParasiteDrag" )
    drop_errors()

    # integral( c t/c ) / integral( c ) with c = 4 - 0.2 y and t/c = 0.18 - 0.01 y over y in [ 0, 10 ]
    expect = ( 0.72 * 10.0 - ( 0.04 + 0.036 ) * 50.0 + 0.002 * 1000.0 / 3.0 ) / 30.0
    toc = vsp.GetDoubleResults( rid, "Comp_FineRat" )[0]
    assert abs( toc - expect ) < 1e-3, "t/c %g, expected %g" % ( toc, expect )


def testASubsurfaceOnADiskHasNoArea():
    """A disk row showed its area, and a subsurface on the disk still added it to the prop."""
    def run( subsurf ):
        fresh()
        p = vsp.AddGeom( "PROP" )
        vsp.SetParmVal( p, "PropMode", "Design", vsp.PROP_BOTH )
        vsp.Update()
        if subsurf:
            s = vsp.AddSubSurf( p, vsp.SS_RECTANGLE, vsp.GetNumMainSurfs( p ) - 1 )
            vsp.SetParmVal( vsp.GetParm( s, "IncludeFlag", "SubSurface" ), subsurf )
        vsp.Update()
        vsp.SetAnalysisInputDefaults( "ParasiteDrag" )
        vsp.SetIntAnalysisInput( "ParasiteDrag", "ExportSubCompFlag", [ 1 ] )
        rid = vsp.ExecAnalysis( "ParasiteDrag" )
        drop_errors()
        return dict( zip( vsp.GetStringResults( rid, "Comp_Label" ), vsp.GetDoubleResults( rid, "Comp_Swet" ) ) ), \
            vsp.GetDoubleResults( rid, "Geom_f_Total" )[0]

    plain, f = run( None )
    assert plain[ "PropGeom_%d" % ( len( [ k for k in plain if k.startswith( "PropGeom_" ) ] ) - 1 ) ] == 0.0, plain
    for inc in ( vsp.SS_INC_TREAT_AS_PARENT, vsp.SS_INC_SEPARATE_TREATMENT ):
        rows, f2 = run( inc )
        assert abs( f2 - f ) < 1e-12, ( inc, f, f2 )


def testTheSmoothFloorIsCorrectedForMachToo():
    """The smooth floor had no Mach factor, so at M 4 it overrode the corrected rough Cf for k up to 1e-3."""
    def cf( eq, k ):
        fresh()
        p = vsp.AddGeom( "POD" )
        vsp.SetParmVal( p, "Roughness", "ParasiteDragProps", k )
        vsp.Update()
        vsp.SetAnalysisInputDefaults( "ParasiteDrag" )
        vsp.SetIntAnalysisInput( "ParasiteDrag", "TurbCfEqnChoice", [ eq ] )
        vsp.SetIntAnalysisInput( "ParasiteDrag", "FreestreamPropChoice", [ vsp.ATMOS_TYPE_MANUAL_RE_L ] )
        vsp.SetDoubleAnalysisInput( "ParasiteDrag", "Re_L", [ 1.0e6 ] )
        vsp.SetDoubleAnalysisInput( "ParasiteDrag", "Mach", [ 4.0 ] )
        rid = vsp.ExecAnalysis( "ParasiteDrag" )
        drop_errors()
        return vsp.GetDoubleResults( rid, "Comp_Cf" )[0]

    factor = ( 1.0 + 0.2 * 16.0 ) ** -0.467
    for k in ( 0.0, 1e-4, 1e-3 ):
        corrected = cf( vsp.CF_TURB_ROUGHNESS_SCHLICHTING_AVG_FLOW_CORRECTION, k )
        plain = cf( vsp.CF_TURB_ROUGHNESS_SCHLICHTING_AVG, k )
        assert abs( corrected / plain - factor ) < 1e-9, ( k, corrected, plain )


def testBelowTheCrossoverCfIsLaminar():
    """Where the turbulent fit falls under Blasius, % laminar was ignored up to 100%, then Cf stepped."""
    cfs = []
    for perclam in ( 0.0, 50.0, 99.9, 100.0 ):
        fresh()
        p = vsp.AddGeom( "POD" )
        vsp.SetParmVal( p, "PercLam", "ParasiteDragProps", perclam )
        vsp.Update()
        vsp.SetAnalysisInputDefaults( "ParasiteDrag" )
        vsp.SetIntAnalysisInput( "ParasiteDrag", "FreestreamPropChoice", [ vsp.ATMOS_TYPE_MANUAL_RE_L ] )
        vsp.SetDoubleAnalysisInput( "ParasiteDrag", "Re_L", [ 1.0e3 ] )
        rid = vsp.ExecAnalysis( "ParasiteDrag" )
        drop_errors()
        cfs.append( vsp.GetDoubleResults( rid, "Comp_Cf" )[0] )
        re = vsp.GetDoubleResults( rid, "Comp_Re" )[0]
    blasius = 1.32824 / math.sqrt( re )
    for cf in cfs:
        assert abs( cf - blasius ) < 1e-12, ( cfs, blasius )


def testKrooFormFactorMatchesKroo():
    """Kroo's second term has C^2 = 1.21, and no supervelocity once M cos(sweep) reaches 1."""
    def ff( mach ):
        vsp.SetAnalysisInputDefaults( "ParasiteDrag" )
        vsp.SetIntAnalysisInput( "ParasiteDrag", "VelocityUnit", [ vsp.V_UNIT_MACH ] )
        vsp.SetDoubleAnalysisInput( "ParasiteDrag", "Vinf", [ mach ] )
        rid = vsp.ExecAnalysis( "ParasiteDrag" )
        drop_errors()
        return vsp.GetDoubleResults( rid, "Comp_FFOut" )[0], vsp.GetDoubleResults( rid, "Comp_FineRat" )[0]

    fresh()
    w = vsp.AddGeom( "WING" )
    for name, val in ( ( "Sweep", 30.0 ), ( "Root_Chord", 2.0 ), ( "Tip_Chord", 2.0 ) ):
        vsp.SetParmVal( w, name, "XSec_1", val )
    vsp.SetParmVal( w, "FFWingEqnType", "ParasiteDragProps", vsp.FF_W_KROO )
    vsp.Update()

    # Kroo, AA241 "Form Factor for Lifting Surfaces", with C = 1.1
    got, tc = ff( 0.5 )
    cs2 = math.cos( math.radians( 30.0 ) ) ** 2
    beta2 = 1.0 - 0.25 * cs2
    expect = 1.0 + 2.0 * 1.1 * cs2 * tc / math.sqrt( beta2 ) + 1.1 * 1.1 * ( 1.0 + 5.0 * cs2 ) * cs2 * tc * tc / beta2 / 2.0
    assert abs( got - expect ) < 1e-9, "FF %g, expected %g" % ( got, expect )

    got, tc = ff( 1.5 )
    assert got == 1.0, "FF %g past M cos(sweep) = 1" % got


def testACloneReferenceWingHasItsOwnSymmetry():
    """A Clone reference wing took its original's area, symmetric even when the Clone is not."""
    for clone_sym in ( True, False ):
        fresh()
        w = vsp.AddGeom( "WING" )
        vsp.Update()
        k = vsp.CloneGeomVec( [ w ] )[0]
        vsp.SetParmVal( k, "X_Rel_Location", "XForm", 100.0 )
        if not clone_sym:
            vsp.SetParmVal( k, "CloneSym", "Behavior", 0 )
            vsp.Update()
            vsp.SetParmVal( k, "Sym_Planar_Flag", "Sym", 0 )
        vsp.Update()
        vsp.SetAnalysisInputDefaults( "ParasiteDrag" )
        vsp.SetIntAnalysisInput( "ParasiteDrag", "RefFlag", [ vsp.COMPONENT_REF ] )
        vsp.SetStringAnalysisInput( "ParasiteDrag", "WingID", [ k ] )
        rid = vsp.ExecAnalysis( "ParasiteDrag" )
        drop_errors()
        area = vsp.GetParmVal( w, "TotalArea", "WingGeom" )
        if not clone_sym:
            area *= 0.5
        sref = vsp.GetDoubleResults( rid, "FC_Sref" )[0]
        assert abs( sref - area ) < 1e-9, "CloneSym %s: S_ref %g, area %g" % ( clone_sym, sref, area )


def testACloneTakesItsOriginalsDragSettings():
    """A Clone used its own default Q, % laminar, roughness and form factor choices, not its original's."""
    names = ( "Comp_Swet", "Comp_Lref", "Comp_FineRat", "Comp_FFOut", "Comp_Re", "Comp_Cf", "Comp_Q", "Comp_PercLam", "Comp_f" )

    def rows( geom_type, clone_drag ):
        fresh()
        g = vsp.AddGeom( geom_type )
        if geom_type == "PROP":
            vsp.SetParmVal( g, "PropMode", "Design", vsp.PROP_BOTH )
        for name, val in ( ( "Q", 1.3 ), ( "PercLam", 20.0 ), ( "Roughness", 1e-5 ),
                           ( "FFBodyEqnType", vsp.FF_B_JENKINSON_FUSE ), ( "FFWingEqnType", vsp.FF_W_TORENBEEK ) ):
            vsp.SetParmVal( g, name, "ParasiteDragProps", val )
        s = vsp.AddSubSurf( g, vsp.SS_RECTANGLE, 0 )
        vsp.SetParmVal( vsp.GetParm( s, "IncludeFlag", "SubSurface" ), vsp.SS_INC_SEPARATE_TREATMENT )
        vsp.SetParmVal( vsp.GetParm( s, "Q", "ParasiteDragProps" ), 1.5 )
        vsp.Update()
        k = vsp.CloneGeomVec( [ g ] )[0]
        vsp.SetParmVal( k, "CloneParasiteDrag", "Behavior", clone_drag )
        vsp.SetParmVal( k, "X_Rel_Location", "XForm", 1000.0 )
        vsp.Update()
        vsp.SetAnalysisInputDefaults( "ParasiteDrag" )
        vsp.SetIntAnalysisInput( "ParasiteDrag", "ExportSubCompFlag", [ 1 ] )
        rid = vsp.ExecAnalysis( "ParasiteDrag" )
        drop_errors()
        table = [ vsp.GetDoubleResults( rid, n ) for n in names ]
        n = len( table[0] ) // 2
        return [ col[:n] for col in table ], [ col[n:] for col in table ]

    for geom_type in ( "POD", "WING", "PROP", "BODYOFREVOLUTION" ):
        original, clone = rows( geom_type, 1 )
        for name, a, b in zip( names, original, clone ):
            for x, y in zip( a, b ):
                assert abs( x - y ) <= 1e-9 * max( 1.0, abs( x ) ), "%s %s: original %g, Clone %g" % ( geom_type, name, x, y )

        original, clone = rows( geom_type, 0 )
        q = names.index( "Comp_Q" )
        assert clone[q][0] == 1.0 and original[q][0] == 1.3, ( geom_type, original[q], clone[q] )
