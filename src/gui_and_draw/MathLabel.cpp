//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//

// MathLabel.cpp
//
//////////////////////////////////////////////////////////////////////

#include "MathLabel.h"

#include <algorithm>
#include <map>
#include <vector>

#include <FL/Fl.H>
#include <FL/fl_draw.H>
#include <FL/Fl_SVG_Image.H>

#include "MathText.h"

using std::vector;

string MathSVG( const string & tex, int size, Fl_Color color, bool bold, int & w, int & h, int & baseline )
{
    MathLayout layout;
    LayoutMath( tex, layout, bold );
    unsigned char r, g, b;
    Fl::get_color( color, r, g, b );
    return layout.ToSVG( size, r, g, b, w, h, baseline );
}

// Typeset math, kept for each string, size, color and weight it is drawn at.
struct MathImage
{
    Fl_SVG_Image* m_Image;
    int m_Baseline;
};

static const MathImage & GetMathImage( const string & tex, int size, Fl_Color color, bool bold )
{
    static std::map < string, MathImage > cache;

    unsigned char r, g, b;
    Fl::get_color( color, r, g, b );
    char key[64];
    snprintf( key, sizeof( key ), "|%d|%02x%02x%02x|%d", size, r, g, b, (int)bold );

    std::map < string, MathImage >::iterator it = cache.find( tex + key );
    if ( it != cache.end() )
    {
        return it->second;
    }

    int w, h, baseline;
    string svg = MathSVG( tex, size, color, bold, w, h, baseline );
    MathImage & mi = cache[ tex + key ];
    mi.m_Image = new Fl_SVG_Image( nullptr, svg.c_str() );
    mi.m_Baseline = baseline;
    return mi;
}

// The label's pieces in order: text drawn in the label's font, math as images overlapping their neighbors by the
// pixel of room each has on either side; and how far the whole reaches above and below its baseline.
struct LabelPiece
{
    string m_Text;
    const MathImage* m_Math;
    int m_Width;
};

static int LayoutLabel( const Fl_Label* o, vector < LabelPiece > & pieces, int & ascent, int & descent )
{
    vector < string > parts;
    SplitMathText( o->value, parts );

    fl_font( o->font, o->size );
    ascent = fl_height() - fl_descent();
    descent = fl_descent();

    int width = 0;
    pieces.clear();
    for ( int i = 0; i < (int)parts.size(); i++ )
    {
        if ( parts[i].empty() )
        {
            continue;
        }
        LabelPiece p;
        p.m_Math = nullptr;
        if ( i % 2 == 0 )
        {
            p.m_Text = parts[i];
            p.m_Width = (int)( fl_width( parts[i].c_str() ) + 0.5 );
        }
        else
        {
            p.m_Math = &GetMathImage( parts[i], o->size, o->color, ( o->font & FL_BOLD ) != 0 );
            p.m_Width = p.m_Math->m_Image->w() - 2;
            ascent = std::max( ascent, p.m_Math->m_Baseline );
            descent = std::max( descent, p.m_Math->m_Image->h() - p.m_Math->m_Baseline );
        }
        width += p.m_Width;
        pieces.push_back( p );
    }
    return width;
}

static void MathLabelDraw( const Fl_Label* o, int X, int Y, int W, int H, Fl_Align align )
{
    vector < LabelPiece > pieces;
    int ascent, descent;
    int width = LayoutLabel( o, pieces, ascent, descent );

    int x = X + ( W - width ) / 2;
    if ( align & FL_ALIGN_LEFT )
    {
        x = X;
    }
    else if ( align & FL_ALIGN_RIGHT )
    {
        x = X + W - width;
    }

    int base = Y + ( H - ascent - descent ) / 2 + ascent;
    if ( align & FL_ALIGN_TOP )
    {
        base = Y + ascent;
    }
    else if ( align & FL_ALIGN_BOTTOM )
    {
        base = Y + H - descent;
    }

    fl_font( o->font, o->size );
    fl_color( o->color );
    for ( int i = 0; i < (int)pieces.size(); i++ )
    {
        if ( pieces[i].m_Math )
        {
            pieces[i].m_Math->m_Image->draw( x - 1, base - pieces[i].m_Math->m_Baseline );
        }
        else
        {
            fl_draw( pieces[i].m_Text.c_str(), x, base );
        }
        x += pieces[i].m_Width;
    }
}

static void MathLabelMeasure( const Fl_Label* o, int & W, int & H )
{
    vector < LabelPiece > pieces;
    int ascent, descent;
    W = LayoutLabel( o, pieces, ascent, descent );
    H = ascent + descent;
}

Fl_Labeltype MathLabelType()
{
    static bool registered = false;
    if ( !registered )
    {
        Fl::set_labeltype( FL_FREE_LABELTYPE, MathLabelDraw, MathLabelMeasure );
        registered = true;
    }
    return FL_FREE_LABELTYPE;
}

void SetMathLabel( Fl_Widget* w, const char* label )
{
    w->copy_label( label );
    if ( label && HasMath( label ) )
    {
        w->labeltype( MathLabelType() );
    }
}

void SetMathLabel( Fl_Menu_Item* item )
{
    if ( item && item->label() && HasMath( item->label() ) )
    {
        item->labeltype( MathLabelType() );
    }
}
