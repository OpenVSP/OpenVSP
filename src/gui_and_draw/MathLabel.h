//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//

// MathLabel.h -- labels with math in them, as LaTeX writes it between dollar signs: the text drawn in the
// label's font, the math typeset in the embedded DejaVu fonts.
//
//////////////////////////////////////////////////////////////////////

#ifndef MATHLABEL_H
#define MATHLABEL_H

#include <string>

#include <FL/Fl_Widget.H>
#include <FL/Fl_Menu_Item.H>

using std::string;

// The label type that draws math, registered with FLTK when first asked for.
Fl_Labeltype MathLabelType();

// Copies the label to the widget, and draws it with math when it holds any.
void SetMathLabel( Fl_Widget* w, const char* label );

// Draws a menu item's label with math when it holds any.
void SetMathLabel( Fl_Menu_Item* item );

// The math typeset as an SVG document, in a color, at size pixels an em, in the bold fonts when bold, its baseline
// at baseline pixels from its top.
string MathSVG( const string & tex, int size, Fl_Color color, bool bold, int & w, int & h, int & baseline );

#endif
