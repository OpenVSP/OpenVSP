# Report API functions that are exposed but not fully documented.
#
# "Fully documented" means a doxygen block with an example in every language the thing is actually
# reachable from.  Both are load bearing: the examples are the test suite, so a function with no
# example is a function nothing checks.
#
# Two mistakes this is meant to stop repeating.  An audit that looks for blocks missing \param or
# \code never sees a function with *no* block at all, so absence and incompleteness are counted
# separately.  And an audit that reads VSP_Geom_API.h alone misses the four other headers the API is
# spread across -- which is how vec3d, vec2d, Matrix4d and the error manager went years with
# examples that had never once been run.
#
# Usage:  python audit_api_docs.py <src dir> [--verbose]

import os
import re
import sys

import api_headers


# vec2d is wrapped by SWIG but never registered with AngelScript, so its examples are Python only
# and the absence of an \forcpponly block is correct rather than a gap.
PYTHON_ONLY = { 'util_api/Vec2d.h' }

# CustomGeom.h is the other way round: it is reachable only from a custom component script, so it
# has no Python side at all.
ANGELSCRIPT_ONLY = { 'geom_core/CustomGeom.h' }


def angelscript_names( srcdir ):
    """Everything ScriptMgr registers, by name."""
    path = os.path.join( srcdir, 'geom_core/ScriptMgr.cpp' )
    if not os.path.exists( path ):
        return set()

    src = open( path ).read()
    out = set()
    for m in re.finditer( r'Register(?:GlobalFunction|ObjectMethod|ObjectBehaviour|ObjectType)\(\s*(?:"[^"]*"\s*,\s*)?"([^"]*)"', src ):
        g = re.search( r'(\w+)\s*\(', m.group( 1 ) )
        if g:
            out.add( g.group( 1 ) )
    return out


def python_names():
    """Everything the built module publishes, including the members of the wrapped classes."""
    try:
        import openvsp.vsp as v
    except ImportError:
        return None

    out = set( n for n in dir( v ) if not n.startswith( '_' ) )
    for cn in dir( v ):
        c = getattr( v, cn, None )
        if isinstance( c, type ):
            out |= set( n for n in dir( c ) if not n.startswith( '_' ) )
    return out


def classify( e, header ):
    """'' when the entity is fully documented, otherwise what it is missing."""
    if e.excluded:
        # A plain // comment where the doxygen block would be says this was left out on purpose.
        return ''

    if not e.doc:
        return 'no doc block'

    # A class is documented by its description; the examples belong to its methods.
    if e.decl.startswith( 'class ' ):
        return ''

    if header not in ANGELSCRIPT_ONLY and not e.code( 'py' ).strip():
        return 'no Python example'

    if header not in PYTHON_ONLY and not e.code( 'cpp' ).strip():
        return 'no AngelScript example'

    return ''


def main( srcdir, verbose ):
    as_names = angelscript_names( srcdir )
    py_names = python_names()
    if py_names is None:
        print( 'audit_api_docs: openvsp not importable; auditing every declaration' )
        py_names = set()
        exposed = None
    else:
        exposed = as_names | py_names

    total = 0
    for h in api_headers.HEADERS:
        p = os.path.join( srcdir, h )
        if not os.path.exists( p ):
            continue

        rows = api_headers.parse( p, h )
        if exposed is not None:
            rows = [ e for e in rows if e.name in exposed or e.decl.startswith( 'class ' ) ]

        gaps = {}
        for e in rows:
            c = classify( e, h )
            if c:
                gaps.setdefault( c, [] ).append( e.name )

        n = sum( len( set( v ) ) for v in gaps.values() )
        total += n
        print( '%-28s exposed %-4d  gaps %d' % ( h, len( rows ), n ) )

        for k in sorted( gaps ):
            names = sorted( set( gaps[ k ] ) )
            if verbose:
                print( '     %-22s %d' % ( k, len( names ) ) )
                for nm in names:
                    print( '         %s' % nm )
            else:
                shown = ' '.join( names[ :12 ] )
                if len( names ) > 12:
                    shown += ' ... (+%d)' % ( len( names ) - 12 )
                print( '     %-22s %-4d %s' % ( k, len( names ), shown ) )

    print()
    print( 'exposed but not fully documented: %d' % total )
    return total


if __name__ == '__main__':
    src = sys.argv[1]
    main( src, '--verbose' in sys.argv )
