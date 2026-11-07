//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//

// MathText.cpp
//
//////////////////////////////////////////////////////////////////////

#include "MathText.h"

#include <algorithm>
#include <cctype>
#include <cmath>
#include <cstdio>
#include <cstring>

// Script sizes and placement, as fractions of the size of what they attach to.
static const double SCRIPT_SCALE = 0.7;
static const double MIN_SCRIPT_SIZE = 0.5;
static const double SUP_RAISE = 0.42;
static const double SUB_DROP = 0.2;
static const double SUB_DROP_WITH_SUP = 0.25;
static const double ITALIC_CORRECTION = 0.06;

// Spaces around binary operators and relations, in ems of the base size.
static const double BIN_SPACE = 4.0 / 18.0;
static const double REL_SPACE = 5.0 / 18.0;

MathLayout::MathLayout()
{
    m_Width = 0;
    m_Ascent = 0;
    m_Descent = 0;
}

// What a math atom is, for the spaces about it.
enum ATOM_CLASS
{
    ATOM_ORD,
    ATOM_BIN,
    ATOM_REL,
    ATOM_OPEN,
    ATOM_PUNCT
};

struct MathSymbol
{
    const char* m_Name;
    int m_Codepoint;
    int m_Class;
};

// Greek letters; the lower case are oblique in math, the upper case upright.
static const MathSymbol greek[] = {
    { "alpha", 0x3B1, ATOM_ORD }, { "beta", 0x3B2, ATOM_ORD }, { "gamma", 0x3B3, ATOM_ORD },
    { "delta", 0x3B4, ATOM_ORD }, { "epsilon", 0x3B5, ATOM_ORD }, { "varepsilon", 0x3B5, ATOM_ORD },
    { "zeta", 0x3B6, ATOM_ORD }, { "eta", 0x3B7, ATOM_ORD }, { "theta", 0x3B8, ATOM_ORD },
    { "vartheta", 0x3D1, ATOM_ORD }, { "iota", 0x3B9, ATOM_ORD }, { "kappa", 0x3BA, ATOM_ORD },
    { "lambda", 0x3BB, ATOM_ORD }, { "mu", 0x3BC, ATOM_ORD }, { "nu", 0x3BD, ATOM_ORD }, { "xi", 0x3BE, ATOM_ORD },
    { "pi", 0x3C0, ATOM_ORD }, { "rho", 0x3C1, ATOM_ORD }, { "sigma", 0x3C3, ATOM_ORD },
    { "tau", 0x3C4, ATOM_ORD }, { "upsilon", 0x3C5, ATOM_ORD }, { "phi", 0x3C6, ATOM_ORD },
    { "varphi", 0x3C6, ATOM_ORD }, { "chi", 0x3C7, ATOM_ORD }, { "psi", 0x3C8, ATOM_ORD },
    { "omega", 0x3C9, ATOM_ORD },
    { "Gamma", 0x393, ATOM_ORD }, { "Delta", 0x394, ATOM_ORD }, { "Theta", 0x398, ATOM_ORD },
    { "Lambda", 0x39B, ATOM_ORD }, { "Xi", 0x39E, ATOM_ORD }, { "Pi", 0x3A0, ATOM_ORD },
    { "Sigma", 0x3A3, ATOM_ORD }, { "Upsilon", 0x3A5, ATOM_ORD }, { "Phi", 0x3A6, ATOM_ORD },
    { "Psi", 0x3A8, ATOM_ORD }, { "Omega", 0x3A9, ATOM_ORD } };

// Other symbols, upright.
static const MathSymbol symbols[] = {
    { "infty", 0x221E, ATOM_ORD }, { "pm", 0xB1, ATOM_BIN }, { "mp", 0x2213, ATOM_BIN },
    { "times", 0xD7, ATOM_BIN }, { "cdot", 0x22C5, ATOM_BIN }, { "deg", 0xB0, ATOM_ORD },
    { "partial", 0x2202, ATOM_ORD }, { "nabla", 0x2207, ATOM_ORD }, { "approx", 0x2248, ATOM_REL },
    { "le", 0x2264, ATOM_REL }, { "ge", 0x2265, ATOM_REL }, { "ne", 0x2260, ATOM_REL },
    { "sim", 0x223C, ATOM_REL }, { "to", 0x2192, ATOM_REL }, { "ell", 0x2113, ATOM_ORD },
    { "prime", 0x2032, ATOM_ORD } };

// Typeset glyphs being built, in ems, with the last glyph set so the next can kern against it.
struct Box
{
    Box()
    {
        m_Width = 0;
        m_Ascent = 0;
        m_Descent = 0;
        m_LastFont = nullptr;
        m_LastCodepoint = 0;
        m_LastSize = 0;
        m_EndsOblique = false;
    }

    vector < PlacedGlyph > m_Glyphs;
    double m_Width;
    double m_Ascent;
    double m_Descent;

    const VspFont* m_LastFont;
    int m_LastCodepoint;
    double m_LastSize;
    bool m_EndsOblique;
};

static void AddGlyph( Box & box, const VspFont & font, int codepoint, double size, bool oblique )
{
    if ( box.m_LastFont == &font && box.m_LastSize == size )
    {
        box.m_Width += font.GetKern( box.m_LastCodepoint, codepoint ) * size;
    }

    PlacedGlyph g;
    g.m_Font = &font;
    g.m_Codepoint = codepoint;
    g.m_X = box.m_Width;
    g.m_Y = 0;
    g.m_Size = size;
    box.m_Glyphs.push_back( g );

    double x0, y0, x1, y1;
    if ( font.GetBox( codepoint, x0, y0, x1, y1 ) )
    {
        box.m_Ascent = std::max( box.m_Ascent, y1 * size );
        box.m_Descent = std::max( box.m_Descent, -y0 * size );
    }
    box.m_Width += font.GetAdvance( codepoint ) * size;

    box.m_LastFont = &font;
    box.m_LastCodepoint = codepoint;
    box.m_LastSize = size;
    box.m_EndsOblique = oblique;
}

// Places a box's glyphs into another at ( dx, dy ).
static void AddBox( Box & box, const Box & part, double dx, double dy )
{
    for ( int i = 0; i < (int)part.m_Glyphs.size(); i++ )
    {
        PlacedGlyph g = part.m_Glyphs[i];
        g.m_X += dx;
        g.m_Y += dy;
        box.m_Glyphs.push_back( g );
    }
    if ( !part.m_Glyphs.empty() )
    {
        box.m_Ascent = std::max( box.m_Ascent, part.m_Ascent + dy );
        box.m_Descent = std::max( box.m_Descent, part.m_Descent - dy );
    }
}

static void AddSpace( Box & box, double width )
{
    box.m_Width += width;
    box.m_LastFont = nullptr;
}

// A typesetter of one string of math.
class MathParser
{
public:
    MathParser( const string & tex, bool bold )
    {
        DecodeUTF8( tex, m_Chars );
        m_Pos = 0;
        m_Upright = &VspFont::Regular();
        m_Oblique = &VspFont::Oblique();
        if ( bold )
        {
            m_Upright = &VspFont::Bold();
            m_Oblique = &VspFont::BoldOblique();
        }
    }

    // Typesets until the end, or the closing brace of a group.  Roman sets letters upright; text keeps spaces.
    Box ParseList( double size, bool roman, bool text, bool group );

protected:
    // One atom and its class, or false at the end of the list.
    bool ParseAtom( double size, bool roman, bool text, Box & atom, int & aclass );

    string ReadCommand();
    Box ParseArgument( double size, bool roman, bool text );

    vector < int > m_Chars;
    int m_Pos;
    const VspFont* m_Upright;
    const VspFont* m_Oblique;
};

string MathParser::ReadCommand()
{
    string name;
    while ( m_Pos < (int)m_Chars.size() && m_Chars[ m_Pos ] < 128 && isalpha( m_Chars[ m_Pos ] ) )
    {
        name += (char)m_Chars[ m_Pos ];
        m_Pos++;
    }
    if ( name.empty() && m_Pos < (int)m_Chars.size() )
    {
        name += (char)m_Chars[ m_Pos ];
        m_Pos++;
    }
    return name;
}

// A group in braces or a single atom, as a command or script takes.
Box MathParser::ParseArgument( double size, bool roman, bool text )
{
    while ( m_Pos < (int)m_Chars.size() && m_Chars[ m_Pos ] == ' ' )
    {
        m_Pos++;
    }
    if ( m_Pos < (int)m_Chars.size() && m_Chars[ m_Pos ] == '{' )
    {
        m_Pos++;
        return ParseList( size, roman, text, true );
    }
    Box atom;
    int aclass;
    ParseAtom( size, roman, text, atom, aclass );
    return atom;
}

bool MathParser::ParseAtom( double size, bool roman, bool text, Box & atom, int & aclass )
{
    atom = Box();
    aclass = ATOM_ORD;

    if ( m_Pos >= (int)m_Chars.size() )
    {
        return false;
    }

    int c = m_Chars[ m_Pos ];
    m_Pos++;

    const VspFont & upright = *m_Upright;
    const VspFont & oblique = *m_Oblique;

    if ( c == '{' )
    {
        atom = ParseList( size, roman, text, true );
        return true;
    }

    if ( c == '\\' )
    {
        string name = ReadCommand();

        if ( name == "," )
        {
            AddSpace( atom, size / 6.0 );
            return true;
        }
        if ( name == ";" )
        {
            AddSpace( atom, size * 5.0 / 18.0 );
            return true;
        }
        if ( name == " " )
        {
            AddSpace( atom, upright.GetAdvance( ' ' ) * size );
            return true;
        }
        if ( name == "quad" )
        {
            AddSpace( atom, size );
            return true;
        }
        if ( name == "mathrm" )
        {
            atom = ParseArgument( size, true, false );
            return true;
        }
        if ( name == "text" )
        {
            atom = ParseArgument( size, true, true );
            return true;
        }

        for ( int i = 0; i < (int)( sizeof( greek ) / sizeof( greek[0] ) ); i++ )
        {
            if ( name == greek[i].m_Name )
            {
                if ( roman || greek[i].m_Codepoint < 0x3B1 )
                {
                    AddGlyph( atom, upright, greek[i].m_Codepoint, size, false );
                }
                else
                {
                    AddGlyph( atom, oblique, greek[i].m_Codepoint, size, true );
                }
                return true;
            }
        }
        for ( int i = 0; i < (int)( sizeof( symbols ) / sizeof( symbols[0] ) ); i++ )
        {
            if ( name == symbols[i].m_Name )
            {
                AddGlyph( atom, upright, symbols[i].m_Codepoint, size, false );
                aclass = symbols[i].m_Class;
                return true;
            }
        }

        // An escaped character, or an unknown command spelled out.
        for ( int i = 0; i < (int)name.size(); i++ )
        {
            AddGlyph( atom, upright, (unsigned char)name[i], size, false );
        }
        return true;
    }

    if ( c == ' ' )
    {
        if ( text )
        {
            AddSpace( atom, upright.GetAdvance( ' ' ) * size );
        }
        return true;
    }

    bool letter = ( c < 128 && isalpha( c ) ) || ( c >= 0x3B1 && c <= 0x3C9 );
    if ( letter && !roman )
    {
        AddGlyph( atom, oblique, c, size, true );
        return true;
    }

    if ( !text )
    {
        if ( c == '-' )
        {
            c = 0x2212;
        }
        if ( c == '+' || c == 0x2212 || c == 0xD7 || c == 0xB1 )
        {
            aclass = ATOM_BIN;
        }
        else if ( c == '=' || c == '<' || c == '>' )
        {
            aclass = ATOM_REL;
        }
        else if ( c == '(' || c == '[' )
        {
            aclass = ATOM_OPEN;
        }
        else if ( c == ',' || c == ';' )
        {
            aclass = ATOM_PUNCT;
        }
    }
    AddGlyph( atom, upright, c, size, false );
    return true;
}

Box MathParser::ParseList( double size, bool roman, bool text, bool group )
{
    Box box;
    int prevclass = ATOM_OPEN;
    bool first = true;

    // Operator spaces are set only at the base size, as TeX sets none in scripts.
    bool spaced = !text && size >= 1.0;

    while ( m_Pos < (int)m_Chars.size() )
    {
        if ( group && m_Chars[ m_Pos ] == '}' )
        {
            m_Pos++;
            break;
        }

        Box nucleus;
        int aclass;
        if ( !ParseAtom( size, roman, text, nucleus, aclass ) )
        {
            break;
        }

        // Scripts attach to the atom before them.
        Box sub, sup;
        bool hassub = false;
        bool hassup = false;
        double ssize = std::max( size * SCRIPT_SCALE, MIN_SCRIPT_SIZE );
        while ( m_Pos < (int)m_Chars.size() && ( m_Chars[ m_Pos ] == '_' || m_Chars[ m_Pos ] == '^' ) )
        {
            bool issub = m_Chars[ m_Pos ] == '_';
            m_Pos++;
            if ( issub )
            {
                sub = ParseArgument( ssize, roman, text );
                hassub = true;
            }
            else
            {
                sup = ParseArgument( ssize, roman, text );
                hassup = true;
            }
        }

        // Spaces only move along; they are not atoms.
        if ( nucleus.m_Glyphs.empty() && !hassub && !hassup )
        {
            AddSpace( box, nucleus.m_Width );
            continue;
        }

        // A binary operator with nothing to its left, as a sign, is ordinary.
        if ( aclass == ATOM_BIN && ( first || prevclass == ATOM_BIN || prevclass == ATOM_REL || prevclass == ATOM_OPEN ||
                                     prevclass == ATOM_PUNCT ) )
        {
            aclass = ATOM_ORD;
        }

        if ( spaced && !first )
        {
            if ( aclass == ATOM_BIN || prevclass == ATOM_BIN )
            {
                AddSpace( box, BIN_SPACE * size );
            }
            else if ( aclass == ATOM_REL || prevclass == ATOM_REL )
            {
                AddSpace( box, REL_SPACE * size );
            }
        }

        double x = box.m_Width;
        if ( !hassub && !hassup )
        {
            // Kerning carries across ordinary atoms of single glyphs.
            if ( nucleus.m_Glyphs.size() == 1 && nucleus.m_LastFont )
            {
                AddGlyph( box, *nucleus.m_LastFont, nucleus.m_LastCodepoint, nucleus.m_LastSize, nucleus.m_EndsOblique );
            }
            else
            {
                AddBox( box, nucleus, x, 0 );
                box.m_Width = x + nucleus.m_Width;
                box.m_LastFont = nullptr;
                box.m_EndsOblique = nucleus.m_EndsOblique;
            }
        }
        else
        {
            AddBox( box, nucleus, x, 0 );
            double xs = x + nucleus.m_Width;
            double w = 0;
            if ( hassup )
            {
                double xi = 0;
                if ( nucleus.m_EndsOblique )
                {
                    xi = ITALIC_CORRECTION * size;
                }
                AddBox( box, sup, xs + xi, SUP_RAISE * size );
                w = std::max( w, xi + sup.m_Width );
            }
            if ( hassub )
            {
                double drop = SUB_DROP * size;
                if ( hassup )
                {
                    drop = SUB_DROP_WITH_SUP * size;
                }
                AddBox( box, sub, xs, -drop );
                w = std::max( w, sub.m_Width );
            }
            box.m_Width = xs + w;
            box.m_LastFont = nullptr;
            box.m_EndsOblique = false;
        }

        prevclass = aclass;
        first = false;
    }
    return box;
}

void LayoutMath( const string & tex, MathLayout & layout, bool bold )
{
    MathParser parser( tex, bold );
    Box box = parser.ParseList( 1.0, false, false, false );
    layout.m_Glyphs = box.m_Glyphs;
    layout.m_Width = box.m_Width;
    layout.m_Ascent = box.m_Ascent;
    layout.m_Descent = box.m_Descent;
}

bool HasMath( const string & str )
{
    size_t a = str.find( '$' );
    return a != string::npos && str.find( '$', a + 1 ) != string::npos;
}

void SplitMathText( const string & str, vector < string > & parts )
{
    parts.clear();
    string cur;
    for ( int i = 0; i < (int)str.size(); i++ )
    {
        if ( str[i] == '\\' && i + 1 < (int)str.size() && str[ i + 1 ] == '$' )
        {
            cur += '$';
            i++;
        }
        else if ( str[i] == '$' )
        {
            parts.push_back( cur );
            cur.clear();
        }
        else
        {
            cur += str[i];
        }
    }
    parts.push_back( cur );
}

void LayoutMathText( const string & str, MathLayout & layout, bool bold )
{
    vector < string > parts;
    SplitMathText( str, parts );

    Box box;
    const VspFont* upright = &VspFont::Regular();
    if ( bold )
    {
        upright = &VspFont::Bold();
    }
    for ( int i = 0; i < (int)parts.size(); i++ )
    {
        if ( i % 2 == 0 )
        {
            vector < int > cps;
            DecodeUTF8( parts[i], cps );
            for ( int k = 0; k < (int)cps.size(); k++ )
            {
                AddGlyph( box, *upright, cps[k], 1.0, false );
            }
        }
        else
        {
            MathLayout m;
            LayoutMath( parts[i], m, bold );
            Box part;
            part.m_Glyphs = m.m_Glyphs;
            part.m_Ascent = m.m_Ascent;
            part.m_Descent = m.m_Descent;
            AddBox( box, part, box.m_Width, 0 );
            box.m_Width += m.m_Width;
            box.m_LastFont = nullptr;
        }
    }
    layout.m_Glyphs = box.m_Glyphs;
    layout.m_Width = box.m_Width;
    layout.m_Ascent = box.m_Ascent;
    layout.m_Descent = box.m_Descent;
}

void MathLayout::Rasterize( double px, vector < unsigned char > & alpha, int & w, int & h, int & baseline ) const
{
    const VspFont & font = VspFont::Regular();
    baseline = (int)ceil( std::max( m_Ascent, font.GetAscent() ) * px ) + 1;
    w = (int)ceil( m_Width * px ) + 2;
    h = baseline + (int)ceil( std::max( m_Descent, font.GetDescent() ) * px ) + 1;

    alpha.assign( w * h, 0 );
    for ( int i = 0; i < (int)m_Glyphs.size(); i++ )
    {
        const PlacedGlyph & pg = m_Glyphs[i];
        pg.m_Font->Rasterize( pg.m_Codepoint, 1.0 + pg.m_X * px, baseline - pg.m_Y * px, pg.m_Size * px, alpha, w, h );
    }
}

string MathLayout::ToSVG( double px, unsigned char r, unsigned char g, unsigned char b, int & w, int & h,
                          int & baseline ) const
{
    // A pixel of room on each side, so antialiased edges are not cut.
    baseline = (int)ceil( m_Ascent * px ) + 1;
    w = (int)ceil( m_Width * px ) + 2;
    h = baseline + (int)ceil( m_Descent * px ) + 1;

    string d;
    for ( int i = 0; i < (int)m_Glyphs.size(); i++ )
    {
        const PlacedGlyph & pg = m_Glyphs[i];
        pg.m_Font->AppendSvgPath( pg.m_Codepoint, 1.0 + pg.m_X * px, baseline - pg.m_Y * px, pg.m_Size * px, d );
    }

    char head[256];
    snprintf( head, sizeof( head ), "<svg width=\"%d\" height=\"%d\" viewBox=\"0 0 %d %d\" "
              "xmlns=\"http://www.w3.org/2000/svg\"><path fill=\"#%02x%02x%02x\" d=\"", w, h, w, h, r, g, b );
    return string( head ) + d + "\"/></svg>";
}
