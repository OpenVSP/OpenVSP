# Turn the examples in the API headers into test suites, one Python and one AngelScript.
#
# Every documented API function carries two examples, and they are the only thing keeping the
# documentation honest -- an example that no longer works is a test failure rather than something a
# user discovers.
#
# Both suites are generated from the headers through api_headers, so the two languages are always
# exercising the same set of examples.  The Python half used to read the docstrings back out of
# vsp.py instead, which quietly dropped two whole categories: anything with "(self" in its
# signature, meaning every method of vec3d, vec2d, Matrix4d, ErrorObj and ErrorMgrSingleton, and
# every overloaded function, because SWIG replaces the docstring of an overload set with a bare list
# of signatures.  The AngelScript half only ever read VSP_Geom_API.h.  Between them, the examples in
# Vec3d.h, Vec2d.h, Matrix4d.h and APIErrorMgr.h had never been run at all.

import os
import sys

import api_headers


PY_HEADER = '''
import unittest
import os, sys

curr_path = os.path.dirname(os.path.realpath(__file__))
vsp_path = os.path.join(curr_path, '../..')
sys.path.insert(1, vsp_path)

from openvsp import *

class TestOpenVSP(unittest.TestCase):
\tdef setUp(self):
\t\tVSPRenew()
\t\t# Start from a clean error queue so each example is only judged on the
\t\t# errors it raised itself.
\t\tapi_err_mgr = ErrorMgrSingleton.getInstance()
\t\twhile api_err_mgr.GetNumTotalErrors() > 0:
\t\t\tapi_err_mgr.PopLastError()
\tdef tearDown(self):
\t\t# An example that leaves an API error behind has not worked, whether or
\t\t# not it bothered to check anything itself.  An example that raises one
\t\t# on purpose is expected to take it back off the queue.
\t\tapi_err_mgr = ErrorMgrSingleton.getInstance()
\t\tapi_err_msgs = []
\t\twhile api_err_mgr.GetNumTotalErrors() > 0:
\t\t\tapi_err_msgs.append( api_err_mgr.PopLastError().GetErrorString() )
\t\tassert len( api_err_msgs ) == 0, "API errors: " + "; ".join( api_err_msgs )
'''


# An operator is documented under its C++ spelling, which is not an identifier in either language.
_SYMBOL = {
    '+': 'add', '-': 'sub', '*': 'mul', '/': 'div', '=': 'assign',
    '[]': 'index', '==': 'eq', '!=': 'ne', '<': 'lt', '>': 'gt',
    '+=': 'add_assign', '-=': 'sub_assign', '*=': 'mul_assign', '/=': 'div_assign',
    '<=': 'le', '>=': 'ge', '()': 'call',
}


def safe_name( name ):
    """A test function name that both languages will accept."""
    if name.startswith( 'operator' ):
        sym = name[ len( 'operator' ) : ].strip()
        return 'operator_' + _SYMBOL.get( sym, 'op' )
    return name.replace( '~', 'dtor_' )


def unique( name, seen ):
    """Overloads share a name; give each example its own test."""
    out = name
    while out in seen:
        out += '1'
    seen.add( out )
    return out


def reindent( code, tabs ):
    """Put an example at a fixed indent, keeping the block structure it had.

    The indent of each line is turned into a level and re-emitted as tabs, rather than the original
    whitespace being kept as-is.  Two reasons.  The common leading whitespace is what has to be
    stripped, not the indent of the first line -- several examples indent their first line further
    than the rest, which a first-line version turns into code that will not parse.  And the examples
    are hand written, so a line here and there sits a space off from its neighbours; keeping that
    verbatim under a tab indent mixes tabs and spaces and Python rejects it.  Rounding to the
    nearest level absorbs the jitter, and emitting tabs throughout means nothing can mix.
    """
    lines = code.split( '\n' )
    body = [ l for l in lines if l.strip() ]
    if not body:
        return ''

    indents = [ len( l ) - len( l.lstrip() ) for l in body ]
    base = min( indents )

    out = ''
    for l in lines:
        if not l.strip():
            out += '\n'
            continue

        level = int( round( ( len( l ) - len( l.lstrip() ) - base ) / 4.0 ) )
        out += '\t' * ( tabs + max( 0, level ) ) + l.strip() + '\n'
    return out


def generate_unit_test( srcdir, unit_file ):
    ents = api_headers.parse_all( srcdir )

    unit_test = PY_HEADER
    seen = set()

    for e in ents:
        if e.group in api_headers.NO_TEST_GROUPS:
            continue

        code = e.code( 'py' )
        if not code.strip():
            continue

        name = unique( 'test_' + safe_name( e.name ), seen )

        unit_test += '\tdef %s(self):\n' % name
        unit_test += reindent( code, 2 )
        unit_test += '\n'

    unit_test += '''
if __name__ == '__main__':
    unittest.main()
'''

    with open( unit_file, 'w' ) as unit:
        unit.write( unit_test )

    return len( seen )


def generate_vspscript_unit_test( srcdir, vspscript_unittest_filepath ):
    ents = api_headers.parse_all( srcdir )

    script = ''
    end_script = 'int main()\n{\n    int int_ret = 0;\n'
    seen = set()

    for e in ents:
        if e.group in api_headers.NO_TEST_GROUPS:
            continue

        code = e.code( 'cpp' )
        if not code.strip():
            continue

        # AngelScript spells the containers differently from C++.
        code = code.replace( 'vector', 'array' ).replace( 'std::', '' )

        name = unique( 'test_' + safe_name( e.name ), seen )

        script += 'int %s()\n{\n' % name
        script += '    VSPRenew();\n'
        script += '    int __failure = 0;\n'
        # Start from a clean error queue so this example is only judged on errors it raised itself.
        script += '    while ( GetNumTotalErrors() > 0 ) { PopLastError(); }\n'
        script += '    Print("//==== %s ====//");\n' % name
        script += code
        # An example that leaves an API error behind has not worked, whether or not it bothered to
        # check anything itself.  Nothing in the documentation raises an error on purpose.
        script += '\n    while ( GetNumTotalErrors() > 0 )\n'
        script += '    {\n'
        script += '        ErrorObj err = PopLastError();\n'
        script += '        Print( "    API error: " + err.GetErrorString() );\n'
        script += '        __failure++;\n'
        script += '    }\n'
        script += '\n    return __failure;\n'
        script += '}\n'

        # Report which example failed rather than only how many.
        end_script += '    {\n'
        end_script += '        int f = %s();\n' % name
        end_script += '        if ( f > 0 )\n'
        end_script += '        {\n'
        end_script += '            Print( "    FAILED: %s" );\n' % name
        end_script += '        }\n'
        end_script += '        int_ret += f;\n'
        end_script += '    }\n'

    end_script += '    Print( "\\n//==== ALL TEST SCRIPTS COMPLETED ====//" );\n'
    end_script += '    if ( int_ret == 0 )\n'
    end_script += '    {\n'
    end_script += '        Print("    All Scripts Run Successfully");\n'
    end_script += '    }\n'
    end_script += '    else\n'
    end_script += '    {\n'
    end_script += '        string fail_message = "    Number of failed scripts : " + int_ret;\n'
    end_script += '        Print( fail_message );\n'
    end_script += '    }\n'
    # Report the accumulated failures to the caller.  This used to return zero unconditionally, so
    # the generated test could never fail no matter what the examples reported through __failure.
    end_script += '    return int_ret;\n'
    end_script += '}\n'

    script += end_script

    with open( vspscript_unittest_filepath, 'w' ) as f:
        f.write( script )

    return len( seen )


if __name__ == '__main__':
    base_dir = sys.argv[1]
    srcdir = sys.argv[2]
    openvsp_dir = os.path.join( base_dir, 'openvsp' )
    unit_file = os.path.join( openvsp_dir, 'tests', 'test_vsp_api.py' )
    unit_file_vspscript = os.path.join( openvsp_dir, 'tests', 'test_vsp_api.vspscript' )

    npy = generate_unit_test( srcdir, unit_file )
    nas = generate_vspscript_unit_test( srcdir, unit_file_vspscript )

    print( 'gen_unit_test: %d Python tests, %d AngelScript tests' % ( npy, nas ) )
