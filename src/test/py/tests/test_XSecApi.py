# Cross section calls through the API.

import openvsp as vsp

from testhelp import ( pop_errors, drop_errors, assert_no_errors )


def testClearingSkinningOnAPropIsRefused():
    """ClearSkinning refuses a Prop, whose sections are not skinned."""
    vsp.VSPRenew()
    drop_errors()
    pid = vsp.AddGeom( "PROP" )
    vsp.Update()
    drop_errors()
    vsp.ClearSkinning( pid )
    assert pop_errors(), "ClearSkinning accepted a Prop"


def testClearingSkinningPastTheLastSectionIsRefused():
    vsp.VSPRenew()
    drop_errors()
    sid = vsp.AddGeom( "STACK" )
    vsp.Update()
    drop_errors()
    vsp.ClearSkinning( sid, 1 )
    assert_no_errors()
    vsp.ClearSkinning( sid, 99 )
    assert pop_errors(), "ClearSkinning accepted a section past the last"


def testAnAliasForAMissingXSecIsAnError():
    """SetXSecAlias and SetXSecCurveAlias report a missing XSec and stop."""
    vsp.VSPRenew()
    drop_errors()
    vsp.SetXSecAlias( "no such xsec", "A" )
    assert pop_errors(), "SetXSecAlias accepted a missing XSec"
    vsp.SetXSecCurveAlias( "no such xsec", "A" )
    assert pop_errors(), "SetXSecCurveAlias accepted a missing XSec"


def testADriverGroupPastTheLastSectionIsAnError():
    """SetDriverGroup reports a fuselage section index past the last."""
    vsp.VSPRenew()
    fid = vsp.AddGeom( "FUSELAGE" )
    vsp.Update()
    drop_errors()
    vsp.SetDriverGroup( fid, 99, vsp.WIDTH_XSEC_DRIVER, vsp.HEIGHT_XSEC_DRIVER )
    assert pop_errors(), "SetDriverGroup accepted a section past the last"


def testCutCopyPasteInsertAndSplitCheckTheIndex():
    vsp.VSPRenew()
    wid = vsp.AddGeom( "WING" )
    fid = vsp.AddGeom( "FUSELAGE" )
    vsp.Update()
    wsurf = vsp.GetXSecSurf( wid, 0 )
    n = vsp.GetNumXSec( wsurf )
    drop_errors()
    for call in ( lambda: vsp.CutXSec( fid, 99 ), lambda: vsp.CopyXSec( fid, 99 ), lambda: vsp.PasteXSec( fid, 99 ),
                  lambda: vsp.InsertXSec( fid, 99, vsp.XS_ELLIPSE ), lambda: vsp.SplitWingXSec( wid, 99 ) ):
        call()
        assert pop_errors(), "an index past the last XSec raised no error"
    # A wing's root has no section to change
    vsp.SplitWingXSec( wid, 0 )
    assert pop_errors(), "SplitWingXSec split the root"
    vsp.CutXSec( wid, 0 )
    assert pop_errors(), "CutXSec cut the root"
    assert vsp.GetNumXSec( wsurf ) == n
    # Nor can a wing give up its only section
    vsp.CutXSec( wid, 1 )
    assert pop_errors(), "CutXSec cut a wing's only section"
    assert vsp.GetNumXSec( wsurf ) == n


def testASparsWingSectionsCanPassEachOther():
    vsp.VSPRenew()
    wid = vsp.AddGeom( "WING" )
    vsp.InsertXSec( wid, 1, vsp.XS_FOUR_SERIES )
    surf = vsp.GetXSecSurf( wid, 0 )
    sid = vsp.AddFeaStructure( wid )
    spar = vsp.AddFeaPartByID( sid, vsp.FEA_SPAR )
    vsp.Update()
    drop_errors()
    vsp.SetFeaPartStartWingSectionID( spar, vsp.GetXSec( surf, 2 ) )
    vsp.SetFeaPartEndWingSectionID( spar, vsp.GetXSec( surf, 2 ) )
    assert_no_errors()
    assert vsp.GetFeaPartStartWingSectionID( spar ) == vsp.GetXSec( surf, 2 )
    assert vsp.GetFeaPartEndWingSectionID( spar ) == vsp.GetXSec( surf, 2 )


def testAnAncestorSetterTakesOnlyAnAncestor():
    vsp.VSPRenew()
    parent = vsp.AddGeom( "POD" )
    child = vsp.AddGeom( "POD", parent )
    other = vsp.AddGeom( "POD" )
    drop_errors()
    vsp.SetGeomSymAncestorID( child, other )
    assert pop_errors(), "a Geom that is not an ancestor was taken"
    vsp.SetGeomIncorporatedAncestorID( child, other )
    assert pop_errors(), "a Geom that is not an ancestor was taken"
    vsp.SetGeomSymAncestorID( child, "" )
    assert_no_errors()
    assert vsp.GetGeomSymAncestorID( child ) == ""


def testTheFlapAbsRelHasItsOwnName( tmp_path ):
    """The flap's Abs/Rel is TE_Flap_AbsRel; a file that calls it TE_Trim_AbsRel still reads it."""
    vsp.VSPRenew()
    drop_errors()
    wid = vsp.AddGeom( "WING" )
    vsp.Update()
    xs = vsp.GetXSec( vsp.GetXSecSurf( wid, 0 ), 1 )

    flap = vsp.GetXSecParm( xs, "TE_Flap_AbsRel" )
    trim = vsp.GetXSecParm( xs, "TE_Trim_AbsRel" )
    assert flap and trim and flap != trim
    assert vsp.GetParmGroupName( flap ) == "Flap"
    assert vsp.GetParmGroupName( trim ) == "Trim"

    vsp.SetParmVal( flap, vsp.ABS )
    vsp.SetParmVal( trim, vsp.REL )
    vsp.Update()

    fname = str( tmp_path / "flap_absrel.vsp3" )
    vsp.WriteVSPFile( fname )

    # Write the file as an older version did, with the flap's Parm under the trim's name.
    with open( fname ) as f:
        text = f.read()
    assert "TE_Flap_AbsRel" in text
    with open( fname, "w" ) as f:
        f.write( text.replace( "TE_Flap_AbsRel", "TE_Trim_AbsRel" ) )

    vsp.VSPRenew()
    vsp.ReadVSPFile( fname )
    vsp.Update()
    wid = vsp.FindGeoms()[0]
    xs = vsp.GetXSec( vsp.GetXSecSurf( wid, 0 ), 1 )
    assert vsp.GetParmVal( vsp.GetXSecParm( xs, "TE_Flap_AbsRel" ) ) == vsp.ABS
    assert vsp.GetParmVal( vsp.GetXSecParm( xs, "TE_Trim_AbsRel" ) ) == vsp.REL


def testSixSeriesTakesOnlyTheSeriesThereAre():
    """The Six Series Series Parm stops at 65A, the last series."""
    vsp.VSPRenew()
    drop_errors()
    wid = vsp.AddGeom( "WING" )
    surf = vsp.GetXSecSurf( wid, 0 )
    vsp.ChangeXSecShape( surf, 0, vsp.XS_SIX_SERIES )
    xs = vsp.GetXSec( surf, 0 )
    series = vsp.GetXSecParm( xs, "Series" )
    vsp.SetParmVal( series, 8 )
    assert vsp.GetParmVal( series ) == 7
