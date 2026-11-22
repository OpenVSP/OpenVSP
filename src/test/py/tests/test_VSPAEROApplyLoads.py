# Apply Loads (vsploads -interp) moves a VSPAERO pressure solution onto the skin elements of a
# CalculiX FEA mesh and writes the static analysis input with a *DLOAD per skin element.  The
# skin elements are the FEA mesh's element sets named after the skin part, so a skin part named
# anything but "Skin" must still receive loads.

import os
import subprocess
import tempfile

import openvsp as vsp


def run_apply_loads( out, skin_name, rib_name = "" ):
    vsp.VSPRenew()
    w = vsp.AddGeom( "WING" )
    vsp.Update()
    si = vsp.AddFeaStruct( w, True, 0 )
    sid = vsp.GetFeaStructID( w, si )
    if skin_name:
        vsp.SetFeaPartName( vsp.GetFeaPartID( sid, 0 ), skin_name )
    rib = vsp.AddFeaPart( w, si, vsp.FEA_RIB )
    if rib_name:
        vsp.SetFeaPartName( rib, rib_name )
    vsp.Update()

    fem = os.path.join( out, "wing_fea.inp" )
    vsp.SetFeaMeshFileName( w, si, vsp.FEA_CALCULIX_FILE_NAME, fem )
    vsp.ComputeFeaMesh( w, si, vsp.FEA_CALCULIX_FILE_NAME )

    path = os.path.join( out, "wing.vsp3" )
    vsp.WriteVSPFile( path )
    vsp.SetVSP3FileName( path )
    for a in ( "VSPAEROComputeGeometry", "VSPAEROSweep" ):
        vsp.SetAnalysisInputDefaults( a )
        vsp.SetIntAnalysisInput( a, "GeomSet", [ vsp.SET_NONE ] )
        vsp.SetIntAnalysisInput( a, "ThinGeomSet", [ vsp.SET_ALL ] )
    vsp.ExecAnalysis( "VSPAEROComputeGeometry" )
    a = "VSPAEROSweep"
    vsp.SetIntAnalysisInput( a, "AlphaNpts", [ 1 ] )
    vsp.SetDoubleAnalysisInput( a, "AlphaStart", [ 5.0 ] )
    vsp.SetIntAnalysisInput( a, "WakeNumIter", [ 1 ] )
    vsp.SetIntAnalysisInput( a, "NCPU", [ 2 ] )
    vsp.SetStringAnalysisInput( a, "RedirectFile", [ os.path.join( out, "vspaero.log" ) ] )
    vsp.ExecAnalysis( a )

    loads = os.path.join( vsp.GetVSPAEROPath(), "vsploads" )
    with open( os.path.join( out, "vsploads.log" ), "w" ) as log:
        subprocess.run( [ loads, "-interp", "wing", "wing_fea", "-dynp", "1.0" ], cwd = out, stdout = log,
                        stderr = subprocess.STDOUT, timeout = 600 )

    # The pressure of every *DLOAD in the static analysis input.
    pressures = []
    in_dload = False
    with open( os.path.join( out, "wing_fea.static.inp" ) ) as f:
        for line in f:
            if line.startswith( "*" ):
                in_dload = line.upper().startswith( "*DLOAD" )
            elif in_dload:
                fields = [ x.strip() for x in line.split( "," ) ]
                if len( fields ) >= 3:
                    pressures.append( float( fields[2] ) )
    return pressures


def loaded( pressures ):
    return len( [ p for p in pressures if p != 0.0 ] )


def test_loads_reach_a_skin_named_skin():
    assert loaded( run_apply_loads( tempfile.mkdtemp(), "" ) ) > 0


def test_loads_reach_a_renamed_skin():
    assert loaded( run_apply_loads( tempfile.mkdtemp(), "WingCover" ) ) > 0


def test_only_the_skin_is_named_as_skin():
    # vsploads takes every element set named "ESkin..." as skin, so a rib named "SkinRib" must not
    # be named that way.
    out = tempfile.mkdtemp()
    run_apply_loads( out, "", "SkinRib" )
    skin_sets = []
    with open( os.path.join( out, "wing_fea.inp" ) ) as f:
        for line in f:
            if line.startswith( "*ELEMENT" ) and "ELSET=ESkin" in line:
                skin_sets.append( line.split( "ELSET=" )[1].strip() )
    assert len( skin_sets ) > 0
    assert all( "Rib" not in s for s in skin_sets )


if __name__ == "__main__":
    test_loads_reach_a_skin_named_skin()
    test_loads_reach_a_renamed_skin()
    test_only_the_skin_is_named_as_skin()
