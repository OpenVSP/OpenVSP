//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//

// MathText.h -- text with math in it, as LaTeX writes it, typeset in the embedded DejaVu fonts.
//
// Text between dollar signs is math: letters are oblique and digits upright, x_1 and x_{12} are subscripts,
// x^2 and x^{-1} superscripts, \alpha through \omega and \Gamma through \Omega Greek, \mathrm{max} upright and
// \text{at max} upright with its spaces.  Also \infty \pm \mp \times \cdot \deg \partial \nabla \approx \le \ge
// \ne \sim \to \ell \prime, and the spaces \, \; \quad and "\ ".  Text outside the dollar signs is set as it is.
//
//////////////////////////////////////////////////////////////////////

#ifndef MATHTEXT_H
#define MATHTEXT_H

#include <string>
#include <vector>

#include "VspFont.h"

using std::string;
using std::vector;

// A glyph placed by the typesetter: its origin and size in ems, y up from the baseline.
struct PlacedGlyph
{
    const VspFont* m_Font;
    int m_Codepoint;
    double m_X;
    double m_Y;
    double m_Size;
};

// Typeset glyphs, their advance and their extent above and below the baseline, in ems.
class MathLayout
{
public:
    MathLayout();

    vector < PlacedGlyph > m_Glyphs;
    double m_Width;
    double m_Ascent;
    double m_Descent;

    // The SVG document of the glyphs at size px, filled with the color, its baseline at baseline pixels from
    // its top.
    string ToSVG( double px, unsigned char r, unsigned char g, unsigned char b, int & w, int & h,
                  int & baseline ) const;

    // The glyphs' coverage at size px, w by h pixels row by row from the top, at least a line of the regular font
    // tall, its baseline at baseline pixels from its top.
    void Rasterize( double px, vector < unsigned char > & alpha, int & w, int & h, int & baseline ) const;
};

// Whether a string holds math.
bool HasMath( const string & str );

// Typesets a string with math in it, in the bold fonts when bold.
void LayoutMathText( const string & str, MathLayout & layout, bool bold = false );

// Typesets math, the contents of a pair of dollar signs, in the bold fonts when bold.
void LayoutMath( const string & tex, MathLayout & layout, bool bold = false );

// The string split into its text and its math, alternately, starting with text.
void SplitMathText( const string & str, vector < string > & parts );

#endif
