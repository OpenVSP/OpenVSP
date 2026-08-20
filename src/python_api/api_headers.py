# Read the API headers.
#
# The API is not one header.  The Doxygen configuration in src/Doxyfile.in lists six, and SWIG wraps
# a seventh, but the documentation and unit test generators were both written against
# VSP_Geom_API.h alone.  That left the vec3d, Matrix4d, APIError and Enumerations groups with no
# pages in the Python documentation, and left every example in Vec3d.h, Vec2d.h, Matrix4d.h and
# APIErrorMgr.h untested -- never compiled, never run.
#
# Both generators now parse through here, so a header added to HEADERS is picked up by the
# documentation and by the tests at the same time.

import os
import re

# Relative to the src directory.  Keep in step with the INPUT list in Doxyfile.in.
HEADERS = [
    'geom_api/VSP_Geom_API.h',
    'geom_api/APIErrorMgr.h',
    'util_api/Vec3d.h',
    'util_api/Vec2d.h',
    'util_api/Matrix4d.h',
    'geom_core/CustomGeom.h',
]

# Groups whose functions cannot be exercised from a standalone script, so their examples are
# illustrative rather than generated into the test suite.  The CustomGeom functions only do anything
# while a custom component is being built from a .vsppart file; called from an ordinary script they
# return an empty string and raise no error, so a generated test would assert against a silent
# no-op and be documenting the wrong thing entirely.
NO_TEST_GROUPS = { 'CustomGeom' }

# APIDefines.h holds the enums.  They are documented, but there is nothing to declare and nothing to
# test, so it is read separately by parse_enums rather than through parse.
ENUM_HEADER = 'geom_api/APIDefines.h'

# Not a declaration, whatever else the line may look like.
_SKIP = re.compile( r'^\s*(#|\}|\{|public:|private:|protected:|template|namespace|using|typedef|return|enum\b)' )

# A friend declaration inside a class names a free function, not a method.  Vec3d.h repeats each of
# them outside the class, where the documentation is, so the friend is a duplicate and is dropped.
# Vec2d.h declares them only as friends, so there the friend is the only place documentation can go
# -- which is why the test is for a doc block rather than for the keyword alone.
_FRIEND = re.compile( r'^\s*friend\b' )

# Types the API deals in.  A declaration has to start with one of these or be a constructor,
# destructor or operator, which keeps member variables and stray code out of the list.
_DECL = re.compile( r'(operator\s*(?:[-+*/=!<>\[\]]+|\s*\w+)|[~\w]+)\s*\(' )


class Entity:
    """One documented thing in a header: a free function, a class, or a method of one."""

    def __init__( self, name, cls, decl, doc, header ):
        self.name = name
        self.cls = cls              # enclosing class, or '' for a free function
        self.decl = decl
        self.doc = doc
        self.header = header

    @property
    def excluded( self ):
        r"""True when the declaration is marked \internal.

        Marks the handful of declarations that are deliberately left out of the API documentation --
        a layering violation kept for compatibility, or something the bindings %ignore because it
        cannot be expressed from the target language.  Doxygen already understands \internal and
        drops these from the generated output, and the audit honours it so a later pass does not put
        them back.  A plain // comment is not enough of a signal: section headers like
        "// Get Point Values" sit in front of perfectly ordinary declarations.
        """
        return '\\internal' in self.doc

    @property
    def group( self ):
        m = re.search( r'\\ingroup\s+(\w+)', self.doc )
        if m:
            return m.group( 1 )
        return ''

    @property
    def brief( self ):
        """The prose before the first command, as one line."""
        body = re.sub( r'\\ingroup\s+\w+', '', self.doc )
        body = re.split( r'\\(?:forcpponly|beginPythonOnly|code|param|return|sa|see)', body )[0]
        body = body.replace( '/*!', '' ).replace( '*/', '' )
        return ' '.join( l.strip() for l in body.split( '\n' ) if l.strip() )

    def code( self, lang ):
        """The example for a language, or '' when there is none."""
        m = re.search( r'\\code\{\.%s\}\n(.*?)\\endcode' % lang, self.doc, re.S )
        if m:
            return m.group( 1 )
        return ''

    @property
    def qualname( self ):
        if self.cls:
            return self.cls + '::' + self.name
        return self.name


def parse( path, header = '' ):
    """Every declaration in a header, each carrying the doxygen block in front of it.

    The blocks come in two pieces -- a lone "\\ingroup" block and then the description -- so they are
    accumulated until a declaration consumes them.  Only the first declaration after a block gets
    it; the overloads that follow are separate entries with no documentation of their own, which is
    what makes them show up in an audit.
    """
    if not header:
        header = os.path.basename( path )

    src = open( path ).read()

    out = []
    pending = ''
    depth = 0
    # Class we are inside, and the depth it was opened at, so a nested brace does not lose it.
    cls = ''
    cls_depth = -1

    # The lookbehind matters: these files head their sections with banners of the form
    # "//*********", where the second character of "//" and the "*" after it read as the start of a
    # block comment.  Without it the banner swallows everything up to the next "*/", which in
    # Vec3d.h is the \ingroup block of the first documented function.
    for chunk in re.split( r'((?<!/)/\*!.*?\*/|(?<!/)/\*.*?\*/)', src, flags = re.S ):
        if chunk.startswith( '/*!' ):
            pending += chunk
            continue
        if chunk.startswith( '/*' ):
            continue

        # The \ingroup block and the description are two comments with only a newline between them.
        # Nothing but whitespace does not end the run, or the group tag would be dropped every time.
        if not chunk.strip():
            continue

        doc = pending
        pending = ''

        stmt = ''
        for line in chunk.split( '\n' ):
            line = line.split( '//' )[0]

            m = re.match( r'\s*class\s+(\w+)', line )
            if m and '{' in line or ( m and not line.rstrip().endswith( ';' ) ):
                # A forward declaration ends in ; and opens nothing.
                if not line.rstrip().endswith( ';' ):
                    cls = m.group( 1 )
                    cls_depth = depth
                    out.append( Entity( cls, '', line.strip(), doc, header ) )
                    doc = ''

            for ch in line:
                if ch == '{':
                    depth += 1
                elif ch == '}':
                    depth -= 1
                    if cls and depth <= cls_depth:
                        cls = ''
                        cls_depth = -1

            s = line.strip()
            if not s:
                continue
            stmt = ( stmt + ' ' + s ).strip()
            if ';' not in stmt and '{' not in stmt:
                continue

            one = re.split( r'[;{]', stmt )[0].strip()
            stmt = ''

            if not one or _SKIP.match( one ) or '(' not in one:
                continue

            friend = _FRIEND.match( one )
            if friend and not doc:
                continue

            m = _DECL.search( one )
            if not m:
                continue

            name = re.sub( r'\s+', '', m.group( 1 ) )
            if name in ( 'if', 'for', 'while', 'switch', 'return', 'sizeof' ):
                continue

            # A documented friend is a free function that happens to be declared inside the class.
            out.append( Entity( name, '' if friend else cls, one, doc, header ) )
            doc = ''

    return out


def parse_all( srcdir ):
    """Every declaration across every API header."""
    out = []
    for h in HEADERS:
        p = os.path.join( srcdir, h )
        if os.path.exists( p ):
            out += parse( p, h )
    return out


def read_groups( srcdir ):
    """Group tag -> ( title, brief ), from the \\defgroup blocks in VSP_Geom_API.h."""
    src = open( os.path.join( srcdir, 'geom_api/VSP_Geom_API.h' ) ).read()

    groups = {}
    for m in re.finditer( r'\\defgroup\s+(\w+)\s+([^\n]+)\n((?:\s*\\brief[\s\S]*?)?)(?=\n\s*\\ref|\n\s*\\defgroup|\n\s*\*/)', src ):
        tag = m.group( 1 )
        title = m.group( 2 ).strip()
        brief = re.sub( r'\s*\\brief\s*', '', m.group( 3 ) ).strip()
        brief = ' '.join( l.strip() for l in brief.split( '\n' ) if l.strip() )
        # The briefs escape the wildcard for doxygen; Sphinx wants it plain.
        brief = brief.replace( '\\\\*', '*' ).replace( '\\*', '*' )
        groups[ tag ] = ( title, brief )

    return groups


class Enum:
    def __init__( self, name, brief, values ):
        self.name = name
        self.brief = brief
        self.values = values        # [ ( value name, brief ) ]


def parse_enums( srcdir ):
    """The enums, with the description of each value.

    None of this reaches Python through SWIG -- an int constant carries no docstring -- so the
    documentation has to be written out from the header directly.
    """
    src = open( os.path.join( srcdir, ENUM_HEADER ) ).read()

    out = []
    for m in re.finditer( r'/\*!\s*([^*]*?)\s*\*/\s*enum\s+(\w+)\s*\{(.*?)\}\s*;', src, re.S ):
        brief = ' '.join( l.strip() for l in m.group( 1 ).split( '\n' ) if l.strip() )
        brief = re.sub( r'\\ingroup\s+\w+', '', brief ).strip()
        name = m.group( 2 )

        values = []
        for v in re.finditer( r'(\w+)\s*(?:=\s*[^,/]+)?\s*,?\s*(?:/\*!<\s*(.*?)\s*\*/)?', m.group( 3 ) ):
            vn = v.group( 1 )
            if not vn:
                continue
            values.append( ( vn, ( v.group( 2 ) or '' ).strip() ) )

        out.append( Enum( name, brief, values ) )

    return out
