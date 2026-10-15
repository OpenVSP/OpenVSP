"""Wireframe (Hermite) files."""

import openvsp as vsp


def test_a_component_name_with_spaces_is_kept_whole( tmp_path ):
    fname = str( tmp_path / "wire.hrm" )
    with open( fname, "w" ) as f:
        f.write( " HERMITE INPUT FILE\n\n NUMBER OF COMPONENTS = 1\n" )
        f.write( "Left Wing Panel\n GROUP NUMBER      = 0\n TYPE              = 0\n" )
        f.write( " CROSS SECTIONS    = 2\n PTS/CROSS SECTION = 3\n" )
        for x in ( 0.0, 1.0 ):
            for z in ( 0.0, 0.5, 1.0 ):
                f.write( "%f %f %f\n" % ( x, 0.0, z ) )
    vsp.VSPRenew()
    vsp.ImportFile( fname, vsp.IMPORT_XSEC_WIRE, "" )
    vsp.Update()
    names = [ vsp.GetGeomName( g ) for g in vsp.FindGeoms() ]
    assert "Left_Wing_Panel" in names, names
