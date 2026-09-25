# Helpers the test files share.
#
# Not a test file itself -- pytest collects test_*.py, and this is imported by those.

import openvsp as vsp
import os
import tempfile


def pop_errors():
    """Take everything off the error stack and return what it said."""
    mgr = vsp.ErrorMgrSingleton.getInstance()
    return [ mgr.PopLastError().m_ErrorString for _ in range( mgr.GetNumTotalErrors() ) ]


def drop_errors():
    """Empty the error stack, for tests that ask for something on purpose refused."""
    pop_errors()


def assert_no_errors():
    assert pop_errors() == []


def a_wire_file():
    """The smallest plot3d file that makes a wireframe surface."""
    lines = [ "1", "7 5 1" ]
    for axis in range( 3 ):
        vals = []
        for j in range( 5 ):
            for i in range( 7 ):
                xyz = ( 4.0 * i / 6.0, 2.0 * j / 4.0, 0.4 * ( i % 2 ) )
                vals.append( "%.10g" % xyz[axis] )
        lines.append( " ".join( vals ) )
    path = os.path.join( tempfile.mkdtemp(), "wire.p3d" )
    open( path, "w" ).write( "\n".join( lines ) + "\n" )
    return path
