# Build the reference pages for the Python API documentation.
#
# The API headers sort their contents into groups with \defgroup and \ingroup, and the C++
# documentation is built around them.  SWIG does not carry those tags into the Python docstrings, so
# Sphinx has no way to see the grouping and, left to itself, lists every function on one page in
# alphabetical order.  This reads the grouping back out of the headers and writes one page per
# group, each with a summary table over its contents.
#
# Reads every header the API is spread across, through api_headers, not only VSP_Geom_API.h.  The
# vec3d, Matrix4d and APIError groups live in other files entirely, and the enums live in
# APIDefines.h, so a single-header version of this left four of the groups advertised on the front
# page leading to nothing.
#
# Run at build time, so a function added to a header appears in the documentation without anyone
# having to remember a second place to add it.

import os
import sys

import api_headers


# Operators are wrapped under their Python names, and Sphinx will not document them without being
# asked.  vec3d and vec2d both overload arithmetic, which is most of what makes them usable.
SPECIAL = ( '__add__, __sub__, __mul__, __rmul__, __truediv__, __neg__, __eq__, __ne__, '
            '__getitem__, __setitem__, __len__, __repr__' )

# The header documents the free operator functions under their C++ spelling.  They reach Python as
# special methods of the class (bound in vsp_common.i), so the documentation has to name them the
# way a Python caller would write them.
PY_OPERATOR = {
    'operator+': '__add__',
    'operator-': '__sub__',
    'operator*': '__mul__',
    'operator/': '__truediv__',
    'operator==': '__eq__',
    'operator!=': '__ne__',
    'operator[]': '__getitem__',
}


def underline( text, char ):
    return text + '\n' + char * len( text )


def write_operators( f, cls, ops ):
    """The overloaded operators of a class, written out from the header.

    These are bound to Python special methods in the SWIG interface rather than declared in the
    header, so they carry no docstring for autodoc to pick up.  The description and the example are
    in the header, against the C++ spelling, so render them here under the Python spelling.
    """
    f.write( underline( 'Operators', '^' ) + '\n\n' )

    for py, brief, example in ops:
        f.write( '.. method:: %s.%s(other)\n\n' % ( cls, py ) )
        if brief:
            f.write( '    ' + brief + '\n\n' )
        if example:
            f.write( '    .. code-block:: python\n\n' )
            for line in example.rstrip().split( '\n' ):
                f.write( ( '        ' + line.strip() ).rstrip() + '\n' )
            f.write( '\n' )


def write_group_page( path, title, brief, classes, functions, operators ):
    with open( path, 'w' ) as f:
        f.write( underline( title, '=' ) + '\n\n' )

        if brief:
            f.write( brief + '\n\n' )

        f.write( '.. currentmodule:: openvsp\n\n' )

        for cls in sorted( set( classes ) | set( operators ) ):
            methods = classes.get( cls, [] )
            f.write( underline( cls, '-' ) + '\n\n' )

            if methods:
                f.write( '.. autosummary::\n\n' )
                for m in methods:
                    f.write( '    %s.%s\n' % ( cls, m ) )
                f.write( '\n' )

            # :undoc-members: keeps the class listing honest -- a method with no doxygen block still
            # shows up, with its signature, rather than silently vanishing from the documentation.
            f.write( '.. autoclass:: %s\n' % cls )
            f.write( '    :members:\n' )
            f.write( '    :undoc-members:\n' )
            f.write( '    :special-members: %s\n\n' % SPECIAL )

            if cls in operators:
                write_operators( f, cls, operators[ cls ] )

        if functions:
            if classes:
                f.write( underline( 'Functions', '-' ) + '\n\n' )

            f.write( '.. autosummary::\n\n' )
            for n in functions:
                f.write( '    %s\n' % n )
            f.write( '\n\n' )

            f.write( underline( 'Details', '-' ) + '\n\n' )
            for n in functions:
                f.write( '.. autofunction:: %s\n\n' % n )


def write_enum_page( path, title, brief, enums ):
    """The enums, written out from the header.

    An enum reaches Python as a plain int constant, which carries no docstring, so autodoc has
    nothing to find and none of this appears in the documentation unless it is written out here.
    """
    with open( path, 'w' ) as f:
        f.write( underline( title, '=' ) + '\n\n' )

        if brief:
            f.write( brief + '\n\n' )

        f.write( 'Each value below is available as an attribute of the ``openvsp`` module.\n\n' )

        for e in sorted( enums, key = lambda x: x.name ):
            f.write( underline( e.name, '-' ) + '\n\n' )

            if e.brief:
                f.write( e.brief + '\n\n' )

            f.write( '.. list-table::\n    :header-rows: 1\n    :widths: 40 60\n\n' )
            f.write( '    * - Value\n      - Description\n' )
            for vn, vb in e.values:
                f.write( '    * - ``%s``\n      - %s\n' % ( vn, vb ) )
            f.write( '\n' )


def write_index( path, entries ):
    """The page that lists the groups, which is what the front page links to."""
    with open( path, 'w' ) as f:
        f.write( underline( 'API Functions by Group', '=' ) + '\n\n' )
        f.write( 'The OpenVSP API is sorted into the groups below.  Each page lists the contents of\n'
                 'one group with a short summary, followed by the full description of each.\n\n' )
        f.write( '.. toctree::\n    :maxdepth: 1\n\n' )
        for tag, title in entries:
            f.write( '    groups/%s\n' % tag )
        f.write( '\n' )


def main( srcdir, outdir ):
    groups = api_headers.read_groups( srcdir )
    ents = api_headers.parse_all( srcdir )

    # A class carries a group tag; its methods are documented individually but almost never repeat
    # the tag, so a method with no tag of its own belongs to the group of the class it is in.
    class_group = {}
    for e in ents:
        if e.decl.startswith( 'class ' ) and e.group:
            class_group[ e.name ] = e.group

    # Group tag -> { class: [ methods ] }, [ free functions ], { class: [ operators ] }.
    classes = {}
    functions = {}
    operators = {}
    for e in ents:
        tag = e.group
        if not tag and e.cls:
            tag = class_group.get( e.cls, '' )
        if not tag:
            continue

        if e.name.startswith( 'operator' ):
            # Free operator functions are declared outside the class; find the class from the
            # operand type so they are documented with it rather than as loose functions.
            owner = e.cls
            if not owner:
                for c in class_group:
                    if c in e.decl:
                        owner = c
                        break
            if not owner:
                continue

            py = PY_OPERATOR.get( e.name )
            if not py:
                continue

            lst = operators.setdefault( tag, {} ).setdefault( owner, [] )
            if not any( p == py for p, _, _ in lst ):
                lst.append( ( py, e.brief, e.code( 'py' ) ) )
            continue

        if e.decl.startswith( 'class ' ):
            classes.setdefault( tag, {} ).setdefault( e.name, [] )
            continue

        if e.cls:
            # A destructor is not reachable from Python, and a constructor arrives as __init__,
            # which autoclass already documents from the class itself.  Naming either in the
            # summary table only produces a Sphinx warning and a blank row.
            if e.name.startswith( '~' ) or e.name == e.cls:
                continue

            byclass = classes.setdefault( tag, {} )
            lst = byclass.setdefault( e.cls, [] )
            if e.name not in lst:
                lst.append( e.name )
        else:
            lst = functions.setdefault( tag, [] )
            if e.name not in lst:
                lst.append( e.name )

    gdir = os.path.join( outdir, 'groups' )
    os.makedirs( gdir, exist_ok = True )

    enums = api_headers.parse_enums( srcdir )

    tags = set( classes ) | set( functions )
    if enums:
        tags.add( 'Enumerations' )

    entries = []
    for tag in sorted( tags, key = lambda t: groups.get( t, ( t, '' ) )[0].lower() ):
        title, brief = groups.get( tag, ( tag, '' ) )
        path = os.path.join( gdir, tag + '.rst' )

        if tag == 'Enumerations':
            write_enum_page( path, title, brief, enums )
        else:
            write_group_page( path, title, brief, classes.get( tag, {} ),
                              functions.get( tag, [] ), operators.get( tag, {} ) )

        entries.append( ( tag, title ) )

    write_index( os.path.join( outdir, 'api_groups.rst' ), entries )

    nfunc = sum( len( v ) for v in functions.values() )
    nmeth = sum( len( m ) for v in classes.values() for m in v.values() )
    print( 'gen_api_docs: %d groups, %d functions, %d methods, %d enums'
           % ( len( entries ), nfunc, nmeth, len( enums ) ) )

    # A group named on the front page with nothing behind it is a dead link.
    empty = sorted( t for t in groups if t not in tags )
    if empty:
        print( 'gen_api_docs: WARNING groups with no documented members: %s' % ', '.join( empty ) )


if __name__ == '__main__':
    main( sys.argv[1], sys.argv[2] )
