# The FEA structure API.

import openvsp as vsp

from testhelp import ( pop_errors, drop_errors, assert_no_errors )


def testDeletingAFeaSubSurfFromAnotherStructureIsRefused():
    """DeleteFeaSubSurf refuses a subsurface of another structure and leaves both structures alone."""
    vsp.VSPRenew()
    drop_errors()
    pod = vsp.AddGeom( "POD" )
    first = vsp.AddFeaStruct( pod )
    second = vsp.AddFeaStruct( pod )
    kept = vsp.AddFeaSubSurf( pod, first, vsp.SS_RECTANGLE )
    other = vsp.AddFeaSubSurf( pod, second, vsp.SS_RECTANGLE )
    vsp.Update()
    first_id = vsp.GetFeaStructID( pod, first )
    second_id = vsp.GetFeaStructID( pod, second )

    vsp.DeleteFeaSubSurf( pod, first, other )
    assert pop_errors(), "deleting another structure's subsurface raised no error"
    assert vsp.GetFeaSubSurfIDVec( first_id ) == ( kept, ), vsp.GetFeaSubSurfIDVec( first_id )
    assert vsp.GetFeaSubSurfIDVec( second_id ) == ( other, ), vsp.GetFeaSubSurfIDVec( second_id )

    vsp.DeleteFeaSubSurf( pod, second, other )
    assert_no_errors()
    assert vsp.GetFeaSubSurfIDVec( second_id ) == (), vsp.GetFeaSubSurfIDVec( second_id )


def two_assembly_structures():
    vsp.VSPRenew()
    drop_errors()
    pod = vsp.AddGeom( "POD" )
    s1 = vsp.GetFeaStructID( pod, vsp.AddFeaStruct( pod ) )
    s2 = vsp.GetFeaStructID( pod, vsp.AddFeaStruct( pod ) )
    return pod, s1, s2


def testAConnectionFromAFixedPointToItselfIsAnError():
    """A connection from a fixed point to itself is an error and adds nothing."""
    pod, s1, s2 = two_assembly_structures()
    fix = vsp.AddFeaPart( pod, 0, vsp.FEA_FIX_POINT )
    assy = vsp.AddFeaAssembly()
    vsp.AddFeaStructureToAssembly( assy, s1 )
    drop_errors()
    assert vsp.AddFeaAssemblyConnection( assy, fix, s1, 0, fix, s1, 0 ) == ""
    assert pop_errors(), "a connection from a fixed point to itself raised no error"
    assert vsp.NumFeaAssemblyConnections( assy ) == 0


def testAConnectionNeedsFixedPointsOfItsStructures():
    pod, s1, s2 = two_assembly_structures()
    fix1 = vsp.AddFeaPart( pod, 0, vsp.FEA_FIX_POINT )
    fix2 = vsp.AddFeaPart( pod, 1, vsp.FEA_FIX_POINT )
    slice1 = vsp.AddFeaPart( pod, 0, vsp.FEA_SLICE )
    assy = vsp.AddFeaAssembly()
    vsp.AddFeaStructureToAssembly( assy, s1 )
    drop_errors()
    vsp.AddFeaAssemblyConnection( assy, slice1, s1, 0, fix2, s2, 0 )
    assert pop_errors(), "a connection started at a part that is not a fixed point"
    vsp.AddFeaAssemblyConnection( assy, fix2, s1, 0, fix1, s2, 0 )
    assert pop_errors(), "a connection named the wrong Structure for its fixed points"
    vsp.AddFeaAssemblyConnection( assy, "NOT_A_FIXPT", s1, 0, fix2, s2, 0 )
    assert pop_errors(), "a connection started at a missing fixed point"
    assert vsp.NumFeaAssemblyConnections( assy ) == 0

    # s2 joins the Assembly after the connection that reaches it
    conn = vsp.AddFeaAssemblyConnection( assy, fix1, s1, 0, fix2, s2, 0 )
    assert_no_errors()
    assert vsp.GetFeaAssemblyConnectionID( assy, 0 ) == conn


def two_structures():
    vsp.VSPRenew()
    drop_errors()
    pod = vsp.AddGeom( "POD" )
    s1 = vsp.AddFeaStructure( pod )
    s2 = vsp.AddFeaStructure( pod )
    return pod, s1, s2


def testTheIndexFormRefusesAPartFromAnotherStructure():
    pod, s1, s2 = two_structures()
    slice2 = vsp.AddFeaPartByID( s2, vsp.FEA_SLICE )
    vsp.DeleteFeaPart( pod, 0, slice2 )
    assert pop_errors(), "DeleteFeaPart took a part from another structure"
    assert slice2 in vsp.GetFeaPartIDVec( s2 )
    vsp.DeleteFeaPartByID( slice2 )
    assert_no_errors()
    assert slice2 not in vsp.GetFeaPartIDVec( s2 )


def testABoundaryConditionTakesOnlyItsOwnStructuresParts():
    pod, s1, s2 = two_structures()
    slice2 = vsp.AddFeaPartByID( s2, vsp.FEA_SLICE )
    line2 = vsp.AddFeaSubSurfByID( s2, vsp.SS_LINE )
    bc = vsp.AddFeaBC( s1, vsp.FEA_BC_PART )
    vsp.SetFeaBCPartID( bc, slice2 )
    assert pop_errors(), "a boundary condition took another structure's part"
    vsp.SetFeaBCSubSurfID( bc, line2 )
    assert pop_errors(), "a boundary condition took another structure's subsurface"
    vsp.SetFeaBCSubSurfID( bc, vsp.GetFeaPartID( s1, 0 ) )
    assert pop_errors(), "a boundary condition took a part as a subsurface"


def testAFixedPointLiesOnlyOnASurfaceOfItsOwnStructure():
    pod, s1, s2 = two_structures()
    fix1 = vsp.AddFeaPartByID( s1, vsp.FEA_FIX_POINT )
    fix1b = vsp.AddFeaPartByID( s1, vsp.FEA_FIX_POINT )
    slice2 = vsp.AddFeaPartByID( s2, vsp.FEA_SLICE )
    drop_errors()
    vsp.SetFeaFixPointParentID( fix1, slice2 )
    assert pop_errors(), "a fixed point took another structure's part"
    vsp.SetFeaFixPointParentID( fix1, fix1b )
    assert pop_errors(), "a fixed point took another fixed point as its surface"
    vsp.SetFeaFixPointOtherGeomID( fix1, "no such geom" )
    assert pop_errors(), "a fixed point took a missing Geom"


def testABeamPropertyNeedsAnIsotropicMaterial():
    vsp.VSPRenew()
    drop_errors()
    beam = vsp.AddFeaProperty( vsp.FEA_BEAM )
    before = vsp.GetFeaPropertyMaterialID( beam )
    mat = vsp.AddFeaMaterial()
    vsp.SetParmVal( vsp.FindParm( mat, "FeaMaterialType", "FeaMaterial" ), vsp.FEA_ENG_ORTHO )
    vsp.Update()
    drop_errors()
    vsp.SetFeaPropertyMaterialID( beam, mat )
    assert pop_errors(), "a beam property took an orthotropic material"
    assert vsp.GetFeaPropertyMaterialID( beam ) == before


def testSettingAPropertyByIDOutlastsAPendingLegacyIndex():
    """A property set by ID survives the next Update, even with a legacy index pending."""
    pod, s1, s2 = two_structures()
    sl = vsp.AddFeaPartByID( s1, vsp.FEA_SLICE )
    vsp.SetParmVal( vsp.FindParm( sl, "FeaPropertyIndex", "FeaPart" ), 0 )
    prop = vsp.AddFeaProperty( vsp.FEA_SHELL )
    vsp.SetFeaPartPropertyID( sl, prop )
    vsp.Update()
    assert vsp.GetFeaPartPropertyID( sl ) == prop


def testReadingAPropertyKeepsAPendingLegacyIndex():
    pod, s1, s2 = two_structures()
    sl = vsp.AddFeaPartByID( s1, vsp.FEA_SLICE )
    props = vsp.GetFeaPropertyIDVec()
    vsp.SetParmVal( vsp.FindParm( sl, "FeaPropertyIndex", "FeaPart" ), 1 )
    assert vsp.GetFeaPartPropertyID( sl ) == props[1]
    vsp.AddFeaPartByID( s1, vsp.FEA_RIB )
    vsp.Update()
    assert vsp.GetFeaPartPropertyID( sl ) == props[1]
    assert_no_errors()


def testAPropertySlotTakesOnlyItsOwnKindOfProperty():
    pod, s1, s2 = two_structures()
    sl = vsp.AddFeaPartByID( s1, vsp.FEA_SLICE )
    shell = vsp.AddFeaProperty( vsp.FEA_SHELL )
    beam = vsp.AddFeaProperty( vsp.FEA_BEAM )
    drop_errors()
    vsp.SetFeaPartPropertyID( sl, beam )
    assert pop_errors(), "the shell slot took a beam property"
    vsp.SetFeaPartCapPropertyID( sl, shell )
    assert pop_errors(), "the cap slot took a shell property"


def testReadingAMaterialKeepsAPendingLegacyIndex():
    vsp.VSPRenew()
    prop = vsp.AddFeaProperty( vsp.FEA_SHELL )
    vsp.SetParmVal( vsp.FindParm( prop, "FeaMaterialIndex", "FeaProperty" ), 3 )
    mat = vsp.GetFeaPropertyMaterialID( prop )
    vsp.Update()
    assert vsp.GetFeaPropertyMaterialID( prop ) == mat
    assert mat != ""


def testABoundaryConditionTakesOnlyTheReferenceItsTypeUses():
    pod, s1, s2 = two_structures()
    sl = vsp.AddFeaPartByID( s1, vsp.FEA_SLICE )
    line = vsp.AddFeaSubSurfByID( s1, vsp.SS_LINE )
    part_bc = vsp.AddFeaBC( s1, vsp.FEA_BC_PART )
    ss_bc = vsp.AddFeaBC( s1, vsp.FEA_BC_SUBSURF )
    drop_errors()
    vsp.SetFeaBCSubSurfID( part_bc, line )
    assert pop_errors(), "a part boundary condition took a subsurface"
    vsp.SetFeaBCPartID( ss_bc, sl )
    assert pop_errors(), "a subsurface boundary condition took a part"


def testAFixedPointRefusesAGeomWithNoSurface():
    pod, s1, s2 = two_structures()
    fix = vsp.AddFeaPartByID( s1, vsp.FEA_FIX_POINT )
    blank = vsp.AddGeom( "BLANK" )
    drop_errors()
    vsp.SetFeaFixPointOtherGeomID( fix, blank )
    assert pop_errors(), "a fixed point took a Blank Geom"
