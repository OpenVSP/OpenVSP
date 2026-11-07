//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//

// VspFont.cpp
//
//////////////////////////////////////////////////////////////////////

#include "VspFont.h"

#include <algorithm>
#include <cmath>
#include <cstdio>

#include "DejaVuFonts.h"

VspFont::VspFont( const unsigned char* data )
{
    m_Data = data;
    stbtt_InitFont( &m_Info, data, stbtt_GetFontOffsetForIndex( data, 0 ) );
    m_Scale = stbtt_ScaleForMappingEmToPixels( &m_Info, 1.0f );

    int ascent, descent, linegap;
    stbtt_GetFontVMetrics( &m_Info, &ascent, &descent, &linegap );
    m_Ascent = ascent * m_Scale;
    m_Descent = -descent * m_Scale;
    m_LineGap = linegap * m_Scale;
}

bool VspFont::HasGlyph( int codepoint ) const
{
    return stbtt_FindGlyphIndex( &m_Info, codepoint ) != 0;
}

double VspFont::GetAdvance( int codepoint ) const
{
    int advance, lsb;
    stbtt_GetCodepointHMetrics( &m_Info, codepoint, &advance, &lsb );
    return advance * m_Scale;
}

double VspFont::GetKern( int codepoint1, int codepoint2 ) const
{
    return stbtt_GetCodepointKernAdvance( &m_Info, codepoint1, codepoint2 ) * m_Scale;
}

bool VspFont::GetBox( int codepoint, double & x0, double & y0, double & x1, double & y1 ) const
{
    int ix0, iy0, ix1, iy1;
    if ( !stbtt_GetCodepointBox( &m_Info, codepoint, &ix0, &iy0, &ix1, &iy1 ) )
    {
        x0 = y0 = x1 = y1 = 0;
        return false;
    }
    x0 = ix0 * m_Scale;
    y0 = iy0 * m_Scale;
    x1 = ix1 * m_Scale;
    y1 = iy1 * m_Scale;
    return true;
}

static void AppendPoint( double x, double y, string & d )
{
    char buf[64];
    snprintf( buf, sizeof( buf ), "%.2f,%.2f ", x, y );
    d += buf;
}

void VspFont::AppendSvgPath( int codepoint, double x, double y, double px, string & d ) const
{
    stbtt_vertex* verts = nullptr;
    int n = stbtt_GetCodepointShape( &m_Info, codepoint, &verts );
    double s = m_Scale * px;
    for ( int i = 0; i < n; i++ )
    {
        const stbtt_vertex & v = verts[i];
        switch ( v.type )
        {
            case STBTT_vmove:
                if ( i > 0 )
                {
                    d += "Z ";
                }
                d += "M";
                AppendPoint( x + v.x * s, y - v.y * s, d );
                break;
            case STBTT_vline:
                d += "L";
                AppendPoint( x + v.x * s, y - v.y * s, d );
                break;
            case STBTT_vcurve:
                d += "Q";
                AppendPoint( x + v.cx * s, y - v.cy * s, d );
                AppendPoint( x + v.x * s, y - v.y * s, d );
                break;
            case STBTT_vcubic:
                d += "C";
                AppendPoint( x + v.cx * s, y - v.cy * s, d );
                AppendPoint( x + v.cx1 * s, y - v.cy1 * s, d );
                AppendPoint( x + v.x * s, y - v.y * s, d );
                break;
            default:
                break;
        }
    }
    if ( n > 0 )
    {
        d += "Z ";
    }
    stbtt_FreeShape( &m_Info, verts );
}

void VspFont::Rasterize( int codepoint, double x, double y, double px, vector < unsigned char > & alpha, int w,
                         int h ) const
{
    float s = m_Scale * px;
    int ix = (int)floor( x );
    int iy = (int)floor( y );
    float dx = x - ix;
    float dy = y - iy;

    int x0, y0, x1, y1;
    stbtt_GetCodepointBitmapBoxSubpixel( &m_Info, codepoint, s, s, dx, dy, &x0, &y0, &x1, &y1 );
    int gw = x1 - x0;
    int gh = y1 - y0;
    if ( gw <= 0 || gh <= 0 )
    {
        return;
    }

    vector < unsigned char > glyph( gw * gh, 0 );
    stbtt_MakeCodepointBitmapSubpixel( &m_Info, glyph.data(), gw, gh, gw, s, s, dx, dy, codepoint );

    // Glyphs that overlap keep the greater coverage.
    for ( int j = 0; j < gh; j++ )
    {
        int row = iy + y0 + j;
        if ( row < 0 || row >= h )
        {
            continue;
        }
        for ( int i = 0; i < gw; i++ )
        {
            int col = ix + x0 + i;
            if ( col < 0 || col >= w )
            {
                continue;
            }
            unsigned char & a = alpha[ row * w + col ];
            a = std::max( a, glyph[ j * gw + i ] );
        }
    }
}

const VspFont & VspFont::Regular()
{
    static VspFont font( dejavu_sans_ttf );
    return font;
}

const VspFont & VspFont::Oblique()
{
    static VspFont font( dejavu_sans_oblique_ttf );
    return font;
}

const VspFont & VspFont::Bold()
{
    static VspFont font( dejavu_sans_bold_ttf );
    return font;
}

const VspFont & VspFont::BoldOblique()
{
    static VspFont font( dejavu_sans_bold_oblique_ttf );
    return font;
}

void DecodeUTF8( const string & str, vector < int > & codepoints )
{
    codepoints.clear();
    int n = str.size();
    int i = 0;
    while ( i < n )
    {
        unsigned char c = str[i];
        int len = 1;
        int cp = c;
        if ( c >= 0xF0 )
        {
            len = 4;
            cp = c & 0x07;
        }
        else if ( c >= 0xE0 )
        {
            len = 3;
            cp = c & 0x0F;
        }
        else if ( c >= 0xC0 )
        {
            len = 2;
            cp = c & 0x1F;
        }

        bool valid = len > 1 && i + len <= n;
        for ( int k = 1; k < len && valid; k++ )
        {
            unsigned char cc = str[ i + k ];
            if ( ( cc & 0xC0 ) != 0x80 )
            {
                valid = false;
            }
            cp = ( cp << 6 ) | ( cc & 0x3F );
        }

        if ( valid )
        {
            codepoints.push_back( cp );
            i += len;
        }
        else
        {
            codepoints.push_back( c );
            i++;
        }
    }
}
