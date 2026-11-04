# Files written and read under a locale with a comma decimal point.
#
# A program that hosts OpenVSP may set the user's locale -- Qt and Tk do, and so does
# locale.setlocale( locale.LC_ALL, "" ).  printf, strtod and sscanf follow it, so without care
# every number in a .vsp3, mesh or CAD file would be written with a comma and every number read
# would stop at the first '.'.  Each case writes the same model in the "C" locale and in a comma
# locale, each in its own interpreter, and the files must agree byte for byte once the random
# IDs and time stamps are set aside.  The comma locale must still be in place afterwards.

import locale
import os
import re
import subprocess
import sys
import tempfile

import pytest

SRC = os.path.abspath( os.path.join( os.path.dirname( __file__ ), "..", "..", "..", ".." ) )
AIRFOIL = os.path.join( SRC, "examples", "airfoil", "N0012_VSP.af" )
FUSE_XSEC = os.path.join( SRC, "TestXSec.fxs" )

SCRIPT = """
import locale
import os
import sys

import openvsp as vsp

out, loc, airfoil, fuse_xsec, read_from = sys.argv[1:6]
if loc != "C":
    locale.setlocale( locale.LC_ALL, loc )
os.chdir( out )

pod = vsp.AddGeom( "POD" )
vsp.SetParmVal( pod, "Length", "Design", 4.25 )
wing = vsp.AddGeom( "WING", pod )
vsp.SetParmVal( wing, "X_Rel_Location", "XForm", 1.5 )
vsp.SetParmVal( wing, "TotalSpan", "WingGeom", 6.125 )
vsp.SetParmVal( wing, "Sweep", "XSec_1", 12.5 )
xsurf = vsp.GetXSecSurf( wing, 0 )
vsp.ChangeXSecShape( xsurf, 0, vsp.XS_FILE_AIRFOIL )
vsp.ReadFileAirfoil( vsp.GetXSec( xsurf, 0 ), airfoil )
fuse = vsp.AddGeom( "FUSELAGE" )
vsp.SetParmVal( fuse, "Y_Rel_Location", "XForm", 6.0 )
fsurf = vsp.GetXSecSurf( fuse, 0 )
vsp.ChangeXSecShape( fsurf, 2, vsp.XS_FILE_FUSE )
vsp.ReadFileXSec( vsp.GetXSec( fsurf, 2 ), fuse_xsec )
vsp.Update()

vsp.WriteVSPFile( "model.vsp3", vsp.SET_ALL )
vsp.ExportFile( "exp.stl", vsp.SET_ALL, vsp.EXPORT_STL )
vsp.ExportFile( "exp.stp", vsp.SET_ALL, vsp.EXPORT_STEP )
vsp.ExportFile( "exp.igs", vsp.SET_ALL, vsp.EXPORT_IGES )
vsp.SetComputationFileName( vsp.DEGEN_GEOM_CSV_TYPE, "degen.csv" )
vsp.ComputeDegenGeom( vsp.SET_ALL, vsp.DEGEN_GEOM_CSV_TYPE )
vsp.SetComputationFileName( vsp.COMP_GEOM_CSV_TYPE, "cg.csv" )
vsp.ComputeCompGeom( vsp.SET_ALL, False, vsp.COMP_GEOM_CSV_TYPE )

# Fine enough that the mesh writers format on several threads.
vsp.SetCFDMeshVal( vsp.CFD_MAX_EDGE_LEN, 0.5 )
vsp.SetCFDMeshVal( vsp.CFD_MIN_EDGE_LEN, 0.05 )
vsp.SetComputationFileName( vsp.CFD_STL_TYPE, "cfd.stl" )
vsp.SetComputationFileName( vsp.CFD_TRI_TYPE, "cfd.tri" )
vsp.SetComputationFileName( vsp.CFD_VSPGEOM_TYPE, "cfd.vspgeom" )
vsp.ComputeCFDMesh( vsp.SET_ALL, vsp.SET_NONE, vsp.CFD_STL_TYPE | vsp.CFD_TRI_TYPE | vsp.CFD_VSPGEOM_TYPE )

vsp.ClearVSPModel()
vsp.ReadVSPFile( read_from )
vsp.Update()
vsp.WriteVSPFile( "reread.vsp3", vsp.SET_ALL )
wid = [ g for g in vsp.FindGeoms() if vsp.GetGeomTypeName( g ) == "Wing" ][0]
print( "SPAN %r" % vsp.GetParmVal( wid, "TotalSpan", "WingGeom" ) )
print( "SWEEP %r" % vsp.GetParmVal( wid, "Sweep", "XSec_1" ) )
print( "RADIX %s" % locale.localeconv()[ "decimal_point" ] )
"""

FILES = [ "model.vsp3", "exp.stl", "exp.stp", "exp.igs", "degen.csv", "cg.csv", "cfd.stl", "cfd.tri",
          "cfd.vspgeom", "reread.vsp3" ]

# Geom and Parm IDs are random, and STEP and IGES files carry the time they were written.  A
# .vsp3 file lists some of its entries in ID order, so its lines are compared as a set.
ID = re.compile( rb"(?<![A-Za-z])[A-Z]{9,11}(?![A-Za-z])" )
STAMPS = [ re.compile( rb"FILE_NAME\('[^']*','[^']*'" ), re.compile( rb"LOCAL_TIME\([^)]*\)" ),
           re.compile( rb"(?m)^.{72}G {0,6}\d+\r?$" ) ]


def _comma_locale():
    saved = locale.setlocale( locale.LC_NUMERIC )
    try:
        for name in [ "de_DE.UTF-8", "de_DE.utf8", "fr_FR.UTF-8", "fr_FR.utf8", "German_Germany.1252" ]:
            try:
                locale.setlocale( locale.LC_NUMERIC, name )
            except locale.Error:
                continue
            if locale.localeconv()[ "decimal_point" ] == ",":
                return name
    finally:
        locale.setlocale( locale.LC_NUMERIC, saved )
    return None


def _normal( data, name ):
    if name.endswith( ".vsp3" ):
        return sorted( ID.sub( b"ID", data ).split( b"\n" ) )
    ids = {}
    data = ID.sub( lambda m: b"ID%d" % ids.setdefault( m.group( 0 ), len( ids ) ), data )
    for s in STAMPS:
        data = s.sub( b"STAMP", data )
    return data


def _run( loc, read_from ):
    out = tempfile.mkdtemp( prefix = "vsp_locale_" )
    run = subprocess.run( [ sys.executable, "-c", SCRIPT, out, loc, AIRFOIL, FUSE_XSEC, read_from ],
                          capture_output = True, text = True, timeout = 600 )
    assert run.returncode == 0, run.stderr[ -2000: ]
    vals = dict( line.split( " ", 1 ) for line in run.stdout.splitlines() if " " in line )
    return out, vals


def testCommaLocaleWritesAndReadsLikeC():
    loc = _comma_locale()
    if not loc:
        pytest.skip( "no locale with a comma decimal point is installed" )

    c_out, c_vals = _run( "C", "model.vsp3" )
    t_out, t_vals = _run( loc, os.path.join( c_out, "model.vsp3" ) )

    assert c_vals[ "SPAN" ] == "6.125"
    assert c_vals[ "SWEEP" ] == "12.5"
    assert t_vals[ "SPAN" ] == "6.125"
    assert t_vals[ "SWEEP" ] == "12.5"
    # Each call puts the host's locale back as it found it.
    assert t_vals[ "RADIX" ] == ","

    for f in FILES:
        a = open( os.path.join( c_out, f ), "rb" ).read()
        b = open( os.path.join( t_out, f ), "rb" ).read()
        assert _normal( a, f ) == _normal( b, f ), f + " differs between the C locale and " + loc
