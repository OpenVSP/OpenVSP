# The API's enums in AngelScript.
#
# Every enum APIDefines.h declares is registered with AngelScript under its own name, with every one of its
# values, as SWIG gives them all to Python.  An advanced link's script that declares each value, named by its
# enum, compiles only if the enum and the value are both registered and the value is the enum's.

import os
import re

import openvsp as vsp
import pytest

from testhelp import drop_errors

HEADER = os.path.join( os.path.dirname( os.path.abspath( __file__ ) ), "..", "..", "..", "geom_api", "APIDefines.h" )


def header_enums():
    """{ enum name: [ value names ] } for every enum in APIDefines.h, in its order."""
    with open( HEADER ) as f:
        text = f.read()
    text = re.sub( r"/\*.*?\*/", "", text, flags=re.S )
    text = re.sub( r"//[^\n]*", "", text )
    enums = {}
    for m in re.finditer( r"\benum\s+(\w+)\s*\{(.*?)\}", text, re.S ):
        vals = []
        for item in m.group( 2 ).split( "," ):
            item = item.strip()
            if item:
                vals.append( re.match( r"(\w+)", item ).group( 1 ) )
        enums[ m.group( 1 ) ] = vals
    return enums


ENUMS = header_enums()


def testTheHeaderIsRead():
    assert len( ENUMS ) > 100
    assert "POINT_END_CAP" in ENUMS[ "CAP_TYPE" ]


@pytest.mark.parametrize( "name", sorted( ENUMS ) )
def testAScriptKnowsEachValueOfTheEnum( name ):
    vsp.VSPRenew()
    vsp.DelAllAdvLinks()
    drop_errors()
    vsp.AddAdvLink( "Enum" )
    code = "".join( "{ %s e = %s::%s; }\n" % ( name, name, v ) for v in ENUMS[ name ] )
    vsp.SetAdvLinkCode( 0, code )
    assert vsp.BuildAdvLinkScript( 0 ), name
    vsp.DelAllAdvLinks()
    drop_errors()
