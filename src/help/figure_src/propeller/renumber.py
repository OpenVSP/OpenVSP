"""Renumber the figures of a help page in page order, and every reference to them.

    python renumber.py ../../Propeller.md

A figure is an image line followed by an italic caption:

    ![Figure 7. Tilt](images/Propeller_shape_kappa.png "Figure 7. Tilt")

    *Figure 7. A blade raked along a curve ...*

A new figure may be added without a number ( ![Tilt](images/...png "Tilt") and *A blade ...* ); it is numbered
here.  References in the text and in captions ("Figure 7", "Figures 13-14", "Figures 15-16 the lag hinge") follow
their figures to their new numbers.  A new figure has no reference yet: the script lists the figures the text never
refers to.
"""

import re
import sys

IMG = re.compile( r'^(\s*)!\[(?:Figure (\d+)\. )?([^\]]*)\]\(([^)\s]+) "(?:Figure \d+\. )?([^"]*)"\)\s*$' )
CAP = re.compile( r'^(\s*)\*(?:Figure \d+\. )?(.*)$', re.S )
REF = re.compile( r'\bFigure(s?) (\d+)(?:-(\d+))?' )


def main( path ):
    """Renumber the page at path in place."""
    blocks = open( path ).read().split( "\n\n" )
    old_to_new = {}
    figs = []
    n = 0
    for i, b in enumerate( blocks ):
        if isinstance( b, tuple ):
            continue
        m = IMG.match( b )
        if not m:
            continue
        n += 1
        ind, old, alt, src, title = m.groups()
        if old:
            old_to_new[ int( old ) ] = n
        blocks[ i ] = '%s![Figure %d. %s](%s "Figure %d. %s")' % ( ind, n, alt, src, n, title )
        c = CAP.match( blocks[ i + 1 ] )
        assert c and blocks[ i + 1 ].lstrip().startswith( "*" ), "figure %d has no caption" % n
        blocks[ i + 1 ] = ( c.group( 1 ), n, c.group( 2 ) )
        figs.append( ( n, alt ) )

    def fix( m ):
        s, a, b = m.groups()
        a = old_to_new[ int( a ) ]
        if b:
            return "Figures %d-%d" % ( a, old_to_new[ int( b ) ] )
        return "Figure%s %d" % ( s, a )

    referred = set()
    for i, b in enumerate( blocks ):
        if isinstance( b, tuple ):
            # A caption: its own number is set above, and the figures it refers to follow theirs
            ind, k, rest = b
            rest = REF.sub( fix, rest )
            blocks[ i ] = "%s*Figure %d. %s" % ( ind, k, rest )
            for m in REF.finditer( rest ):
                a = int( m.group( 2 ) )
                if a != k:
                    referred.update( range( a, int( m.group( 3 ) or a ) + 1 ) )
            continue
        if IMG.match( b ):
            continue
        blocks[ i ] = REF.sub( fix, b )
        for m in REF.finditer( blocks[ i ] ):
            a = int( m.group( 2 ) )
            referred.update( range( a, int( m.group( 3 ) or a ) + 1 ) )
    open( path, "w" ).write( "\n\n".join( blocks ) )
    print( "%d figures" % n )
    for k, alt in figs:
        if k not in referred:
            print( "not referred to: Figure %d. %s" % ( k, alt ) )


if __name__ == "__main__":
    main( sys.argv[ 1 ] )
