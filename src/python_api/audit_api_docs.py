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


# Which example a declaration needs follows from where it is actually reachable, and that is
# resolved per declaration against the owning type.  A bare name is not enough to decide it:
# vec2d::x and vec3d::x share a name, as do the free dist() over each, and Matrix4d has 28 methods
# in the Python bindings that AngelScript never registered.


def angelscript_names( srcdir ):
    """What ScriptMgr registers: ( set of globals, set of ( type, method ) )."""
    path = os.path.join( srcdir, 'geom_core/ScriptMgr.cpp' )
    if not os.path.exists( path ):
        return set(), set()

    src = open( path ).read()
    globals_, methods = set(), set()

    # A global registration can be bound to a singleton method -- that is how the whole CustomGeom
    # API is exposed -- so record the owning class too, or those read as unreachable.
    for m in re.finditer( r'RegisterGlobalFunction\(\s*"([^"]*)"\s*,\s*(as\w+)\(\s*(\w+)\s*,\s*(\w+)', src ):
        g = re.search( r'(\w+)\s*\(', m.group( 1 ) )
        if g:
            globals_.add( g.group( 1 ) )
        if m.group( 2 ).startswith( 'asMETHOD' ):
            methods.add( ( m.group( 3 ), m.group( 4 ) ) )

    for m in re.finditer( r'RegisterGlobalFunction\(\s*"([^"]*)"', src ):
        g = re.search( r'(\w+)\s*\(', m.group( 1 ) )
        if g:
            globals_.add( g.group( 1 ) )

    for m in re.finditer( r'RegisterObjectMethod\(\s*"(\w+)"\s*,\s*"([^"]*)"', src ):
        g = re.search( r'(\w+)\s*\(', m.group( 2 ) )
        if g:
            methods.add( ( m.group( 1 ), g.group( 1 ) ) )

    return globals_, methods


def python_module():
    try:
        import openvsp.vsp as v
        return v
    except ImportError:
        return None


def in_python( v, e ):
    """Is this declaration reachable from Python, as itself rather than as a name?"""
    if v is None:
        return False
    if e.cls:
        c = getattr( v, e.cls, None )
        return c is not None and hasattr( c, e.name )
    return hasattr( v, e.name )


def classify( e, py, ascript ):
    """'' when the entity is fully documented, otherwise what it is missing."""
    if e.excluded:
        # A plain // comment where the doxygen block would be says this was left out on purpose.
        return ''

    if not e.doc:
        return 'no doc block'

    # A class is documented by its description; the examples belong to its methods.
    if e.decl.startswith( 'class ' ):
        return ''

    if py and not e.code( 'py' ).strip():
        return 'no Python example'

    if ascript and not e.code( 'cpp' ).strip():
        return 'no AngelScript example'

    return ''


def reachable( v, as_globals, as_methods, e ):
    """( reachable from Python, reachable from AngelScript )."""
    py = in_python( v, e )
    if e.cls:
        ascript = ( e.cls, e.name ) in as_methods
    else:
        ascript = e.name in as_globals
    return py, ascript


def main( srcdir, verbose ):
    as_globals, as_methods = angelscript_names( srcdir )
    v = python_module()
    if v is None:
        print( 'audit_api_docs: openvsp not importable; the Python side cannot be checked' )

    total = 0
    for h in api_headers.HEADERS:
        p = os.path.join( srcdir, h )
        if not os.path.exists( p ):
            continue

        rows = []
        for e in api_headers.parse( p, h ):
            py, ascript = reachable( v, as_globals, as_methods, e )
            if py or ascript or e.decl.startswith( 'class ' ):
                rows.append( ( e, py, ascript ) )

        # A function can be declared more than once in a header -- Vec3d.h documents its free
        # functions outside the class and repeats them as bare friends, Vec2d.h does the reverse --
        # and the documentation belongs to the function, not to each declaration of it.  Count a
        # name once it is documented anywhere in the file.
        # \internal counts here too: marking one overload as not-part-of-the-API covers the set.
        documented = set( e.name for e, _, _ in rows if e.doc )

        gaps = {}
        for e, py, ascript in rows:
            if not e.doc and e.name in documented:
                continue
            c = classify( e, py, ascript )
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
