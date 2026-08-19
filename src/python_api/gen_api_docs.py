# Build the reference pages for the Python API documentation.
#
# The API header sorts its functions into groups with \defgroup and \ingroup, and the C++
# documentation is built around them.  SWIG does not carry those tags into the Python docstrings,
# so Sphinx has no way to see the grouping and, left to itself, lists every function on one page in
# alphabetical order.  This reads the grouping back out of the header and writes one page per group,
# each with a summary table over the functions in it.
#
# Run at build time, so a function added to the header appears in the documentation without anyone
# having to remember a second place to add it.

import os
import re
import sys


def read_groups( header ):
    """Group tag -> ( title, brief ), from the \\defgroup blocks."""
    src = open( header ).read()

    groups = {}
    for m in re.finditer( r'\\defgroup\s+(\w+)\s+([^\n]+)\n((?:\s*\\brief[\s\S]*?)?)(?=\n\s*\\ref|\n\s*\\defgroup|\n\s*\*/)', src ):
        tag = m.group( 1 )
        title = m.group( 2 ).strip()
        brief = re.sub( r'\s*\\brief\s*', '', m.group( 3 ) ).strip()
        brief = ' '.join( l.strip() for l in brief.split( '\n' ) if l.strip() )
        groups[ tag ] = ( title, brief )

    return groups


def read_members( header ):
    """Group tag -> [ function names ], in the order the header declares them."""
    src = open( header ).read()

    members = {}
    for tag, name in re.findall( r"\\ingroup\s+(\w+)[\s\S]*?\*/\s*extern[^;]*?\b(\w+)\s*\(", src ):
        lst = members.setdefault( tag, [] )
        if name not in lst:
            lst.append( name )

    return members


def underline( text, char ):
    return text + '\n' + char * len( text )


def write_group_page( path, tag, title, brief, names ):
    with open( path, 'w' ) as f:
        f.write( underline( title, '=' ) + '\n\n' )

        if brief:
            f.write( brief + '\n\n' )

        # The table gives the one line summary of each function; the details follow below it.
        f.write( '.. currentmodule:: openvsp\n\n' )
        f.write( '.. autosummary::\n\n' )
        for n in names:
            f.write( '    %s\n' % n )
        f.write( '\n\n' )

        f.write( underline( 'Details', '-' ) + '\n\n' )
        for n in names:
            f.write( '.. autofunction:: %s\n\n' % n )


def write_index( path, entries ):
    """The page that lists the groups, which is what the front page links to."""
    with open( path, 'w' ) as f:
        f.write( underline( 'API Functions by Group', '=' ) + '\n\n' )
        f.write( 'The OpenVSP API is sorted into the groups below.  Each page lists the functions of\n'
                 'one group with a short summary, followed by the full description of each.\n\n' )
        f.write( '.. toctree::\n    :maxdepth: 1\n\n' )
        for tag, title in entries:
            f.write( '    groups/%s\n' % tag )
        f.write( '\n' )


def main( header, outdir ):
    groups = read_groups( header )
    members = read_members( header )

    gdir = os.path.join( outdir, 'groups' )
    os.makedirs( gdir, exist_ok = True )

    entries = []
    for tag in sorted( members.keys(), key = lambda t: groups.get( t, ( t, '' ) )[0].lower() ):
        names = members[ tag ]
        if not names:
            continue

        title, brief = groups.get( tag, ( tag, '' ) )
        write_group_page( os.path.join( gdir, tag + '.rst' ), tag, title, brief, names )
        entries.append( ( tag, title ) )

    write_index( os.path.join( outdir, 'api_groups.rst' ), entries )

    print( 'gen_api_docs: %d groups, %d functions' % ( len( entries ), sum( len( members[t] ) for t, _ in entries ) ) )

    # A tag used on a function but never defined would silently make a page with no title.
    undefined = sorted( t for t in members if t not in groups )
    if undefined:
        print( 'gen_api_docs: WARNING ingroup tags with no defgroup: %s' % ', '.join( undefined ) )


if __name__ == '__main__':
    main( sys.argv[1], sys.argv[2] )
