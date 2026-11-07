//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//

// VspFont.h -- the embedded DejaVu fonts, read with stb_truetype: metrics and outlines in ems.
//
//////////////////////////////////////////////////////////////////////

#ifndef VSPFONT_H
#define VSPFONT_H

#include <string>
#include <vector>

#include "stb_truetype.h"

using std::string;
using std::vector;

class VspFont
{
public:
    VspFont( const unsigned char* data );

    // The TrueType data, for packing glyphs into a texture.
    const unsigned char* GetData() const        { return m_Data; }

    // Above and below the baseline, and the gap between lines, in ems; the descent is positive.
    double GetAscent() const                    { return m_Ascent; }
    double GetDescent() const                   { return m_Descent; }
    double GetLineGap() const                   { return m_LineGap; }

    bool HasGlyph( int codepoint ) const;

    // The distance to the next glyph, and the kerning between two, in ems.
    double GetAdvance( int codepoint ) const;
    double GetKern( int codepoint1, int codepoint2 ) const;

    // The glyph's extent from its origin in ems, y up.  False for a glyph with no outline, as a space.
    bool GetBox( int codepoint, double & x0, double & y0, double & x1, double & y1 ) const;

    // Appends the glyph's outline to SVG path data: the glyph at size px (pixels an em) with its origin at
    // ( x, y ) pixels, y down.
    void AppendSvgPath( int codepoint, double x, double y, double px, string & d ) const;

    // Adds the glyph's coverage to an image w pixels wide, row by row from the top: the glyph at size px with its
    // origin at ( x, y ) pixels, y down.
    void Rasterize( int codepoint, double x, double y, double px, vector < unsigned char > & alpha, int w,
                    int h ) const;

    static const VspFont & Regular();
    static const VspFont & Oblique();
    static const VspFont & Bold();
    static const VspFont & BoldOblique();

protected:
    const unsigned char* m_Data;
    stbtt_fontinfo m_Info;
    double m_Scale;             // ems a font unit

    double m_Ascent;
    double m_Descent;
    double m_LineGap;
};

// The code points of a UTF-8 string; an invalid byte stands for itself.
void DecodeUTF8( const string & str, vector < int > & codepoints );

#endif
