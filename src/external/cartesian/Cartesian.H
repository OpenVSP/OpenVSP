// Cartesian.H,v 1.1
//
// Copyright 2000-2005 by Roman Kantor.
//
// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU Library General Public License
// version 2 as published by the Free Software Foundation.
//
// This library is distributed  WITHOUT ANY WARRANTY;
// without even the implied warranty of MERCHANTABILITY
// or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// Library General Public License for more details.
// You should have received a copy of the GNU Library General Public
// License along with this library; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA.


#ifndef _Cartesian_h_
#define _Cartesian_h_

#include <FL/Fl_Box.H>

#define DEFAULT_POINT_SIZE 4
#define CA_DEFAULT_LABEL_SIZE 12




enum Ca_Damage {CA_DAMAGE_ALL=1, CA_DAMAGE_ADD=2};

enum Ca_When{CA_WHEN_MIN=1,CA_WHEN_MAX=2};

enum Ca_AxisAlign{CA_LEFT=0, CA_RIGHT=1, CA_BOTTOM=CA_LEFT, CA_TOP=CA_RIGHT,
	CA_CENTER=3,CA_ALIGNMENT=3,CA_NO_LABELS=32, CA_NO_TICS=64, CA_LINE=128};

enum Ca_PointStyle{CA_NO_POINT=0, CA_ROUND=1,CA_SQUARE=2,CA_UP_TRIANGLE=3, CA_DOWN_TRIANGLE=4,
	CA_DIAMOND=5, CA_SIMPLE=6, CA_POINT_STYLE=127, CA_BORDER=128};

enum Ca_GridVisible{
	CA_MINOR_GRID=0xF,CA_LEFT_MINOR_TICK=0x1, CA_BOTTOM_MINOR_TICK=CA_LEFT_MINOR_TICK,CA_RIGHT_MINOR_TICK=0x2, CA_TOP_MINOR_TICK=CA_RIGHT_MINOR_TICK, CA_MINOR_TICK=0x3,
	CA_MAJOR_GRID=0xF0, CA_LEFT_MAJOR_TICK=0x10, CA_BOTTOM_MAJOR_TICK=CA_LEFT_MAJOR_TICK,CA_RIGHT_MAJOR_TICK=0x20, CA_TOP_MAJOR_TICK=CA_RIGHT_MAJOR_TICK,CA_MAJOR_TICK=0x30,
	CA_LABEL_GRID=0xF00,CA_LEFT_LABEL_TICK=0x100,CA_BOTTOM_LABEL_TICK=CA_LEFT_LABEL_TICK,CA_RIGHT_LABEL_TICK=0x200, CA_TOP_LABEL_TICK=CA_RIGHT_LABEL_TICK, CA_LABEL_TICK=0x300,
	CA_ALWAYS_VISIBLE=0x1000,CA_FRONT=0x2000};

enum Ca_AxisType{CA_LIN=0, CA_LOG=1, CA_REV=2};




class Ca_Canvas;
class Ca_Axis_;
class Ca_Object_;
class Ca_ObjectChain;
//class Cartesian;


///////////////////////////////////////////////////////////////////////////
class Ca_ObjectChain{
public:
    Ca_Object_ *object;
    Ca_ObjectChain *next;
};


///////////////////////////////////////////////////////////////////////////


class Ca_Canvas:public Fl_Box{

    friend class Ca_Axis_;
    friend class Ca_X_Axis;
    friend class Ca_Y_Axis;
    friend class Ca_Object_;

    static Ca_Canvas *current_;
    int replot_;
    Ca_Axis_ * last_axis_;
    int border_;
    Ca_Axis_ *current_x_;
    Ca_Axis_ *current_y_;
    Ca_ObjectChain * first_object_;
    Ca_ObjectChain * last_object_;
    Ca_ObjectChain * last_plotted_;
    void add_object(Ca_Object_ *object);

    /* No function body - prevents copy construction/assignment */
    Ca_Canvas(const Ca_Canvas &);
    const Ca_Canvas & operator=(const Ca_Canvas &);
    int dx_, dy_, dw_, dh_;

public:
    void draw();
    static Ca_Canvas * current(){return current_;};
    static void current(Ca_Canvas * _current){current_=_current;}
    void current_x(Ca_Axis_ * axis){current_x_=axis;};
    void current_y(Ca_Axis_ * axis){current_y_=axis;};
    Ca_Axis_ * current_x(){return current_x_;};
    Ca_Axis_ * current_y(){return current_y_;};
    void clear();
    int border(){return border_;};
    void border(int border);
    void clip_border(int dx, int dy, int dw, int dh){ dx_ = dx; dy_ = dy; dw_ = dw; dh_ = dh;}
    void clip_border(int * dx, int * dy, int *dw, int *dh){ *dx = dx_; *dy = dy_; *dw = dw_; *dh = dh_;}

    Ca_ObjectChain * objects(){return first_object_;};
    Ca_Canvas(int x, int y, int w, int h, const char *label=0);
    ~Ca_Canvas();

};


///////////////////////////////////////////////////////////////////////////

class Ca_Axis_:public Fl_Box{

    friend class Ca_Canvas;

    Ca_Axis_ *previous_axis_;




protected:
    Fl_Widget * widget_;
    int scale_;
    int next_tick(int &tick_index, double &tick_value, int &tick_order, double &interval_);
    bool valid_;
    double k_;
    double q_;
    const char *label_format_;
    Fl_Color minor_grid_color_;
    Fl_Color major_grid_color_;
    Fl_Color label_grid_color_;
    int minor_grid_style_;
    int major_grid_style_;
    int label_grid_style_;
    int minor_grid_width_;
    int major_grid_width_;
    int label_grid_width_;
    char * minor_grid_dashes_;
    char * major_grid_dashes_;
    char * label_grid_dashes_;
    int grid_visible_;
    double tick_interval_;
    int tick_separation_;
    int tick_length_;
    int tick_width_;
    int major_step_;
    int label_step_;
    Ca_Canvas * canvas_;
    unsigned char  axis_align_;
    Fl_Font label_font_face_;
    int label_font_size_;

    double min_;
    double max_;
    int min_pos_;
    int max_pos_;
    int border_;
    Fl_Color axis_color_;

    virtual int min_pos()=0;
    virtual int max_pos()=0;
    int update();
    virtual void draw_grid()=0;

public:
  Fl_Widget * widget() const {return widget_;}
  void widget(Fl_Widget * w) {widget_ = w;}
    virtual void current()=0;
    double position(double);
    double value(double);
    Ca_Canvas * canvas(){return canvas_;};
    int border(){return border_;};
    void border(int border){border_=border;damage(CA_DAMAGE_ALL);}
    double minimum() const {return min_;};
    double maximum() const {return max_;};
    void minimum(double x);
    void maximum(double x);
    void clear(){valid_=0;};
    int cleared(){return !valid_;};

    void rescale(int when, double x);
    void rescale_move(int when, double  x);
    void tick_interval(double interval) {tick_interval_=interval; damage(CA_DAMAGE_ALL);};
    double tick_interval(){return tick_interval_;};
    void tick_separation(int separation) {tick_separation_=separation; damage(CA_DAMAGE_ALL);};
    int tick_separation(){return tick_separation_;};
    void tick_length(int length){tick_length_=length; damage(CA_DAMAGE_ALL);};
    int tick_length(){return tick_length_;};
    void tick_width(int width){tick_width_=width; damage(CA_DAMAGE_ALL);};
    int tick_width(){return tick_width_;};
    void major_step(int step){major_step_=step;damage(CA_DAMAGE_ALL);};
    int major_step(){return major_step_;};
    void label_step(int step){label_step_=step;damage(CA_DAMAGE_ALL);};
    int label_step(){return label_step_;};
    void label_format(const char *format){label_format_=format; damage(CA_DAMAGE_ALL);};
    const char* label_format(){return label_format_;};
    void label_font(Fl_Font face){label_font_face_=face; damage(CA_DAMAGE_ALL);};
    Fl_Font label_font(){return label_font_face_;};
    void label_size(int size){label_font_size_=size; damage(CA_DAMAGE_ALL);};
    int label_size(){return label_font_size_;};
    void axis_color(Fl_Color _axis_color){axis_color_=_axis_color; damage(CA_DAMAGE_ALL);};
    Fl_Color axis_color(){ return axis_color_;};
    void minor_grid_color(Fl_Color color){minor_grid_color_=color; canvas_->damage(CA_DAMAGE_ALL);};
    Fl_Color minor_grid_color(){return minor_grid_color_;};
    void minor_grid_style(int style, int width=0, char * dashes=0){minor_grid_style_=style;minor_grid_width_=width;minor_grid_dashes_=dashes; canvas_->damage(CA_DAMAGE_ALL);};
    void minor_grid_style(int * style, int * width=0, char ** dashes=0){
	if (style) *style=minor_grid_style_;
	    if (width) *width=minor_grid_width_;
	    if (dashes) *dashes=minor_grid_dashes_;
	};
    void major_grid_color(Fl_Color color){major_grid_color_=color;  canvas_->damage(CA_DAMAGE_ALL);};
    Fl_Color major_grid_color(){return major_grid_color_;};
    void major_grid_style(int style, int width=0, char * dashes=0){major_grid_style_=style; major_grid_width_=width; major_grid_dashes_=dashes; canvas_->damage(CA_DAMAGE_ALL);};
    void major_grid_style(int * style, int * width=0, char ** dashes=0){
        if (style) *style=major_grid_style_;
        if (width) *width=major_grid_width_;
        if (dashes) *dashes=major_grid_dashes_;
    };
    void label_grid_color(Fl_Color color){label_grid_color_=color;  canvas_->damage(CA_DAMAGE_ALL);};
    Fl_Color label_grid_color(){return label_grid_color_;};
    void label_grid_style(int style, int width=0, char * dashes=0){label_grid_style_=style; label_grid_width_=width; label_grid_dashes_=dashes; canvas_->damage(CA_DAMAGE_ALL);};
    void label_grid_style(int * style, int * width=0, char ** dashes=0){
        if (style) *style=label_grid_style_;
        if (width) *width=label_grid_width_;
        if (dashes) *dashes=label_grid_dashes_;
    };
    void grid_visible(int visible){grid_visible_=visible;canvas_->damage(CA_DAMAGE_ALL);};
    int grid_visible(){return grid_visible_;};
    void axis_align(unsigned char align){axis_align_=align; damage(CA_DAMAGE_ALL);};
    unsigned char axis_align(){return axis_align_;};
    void scale(int s){if(s!=scale_){redraw(); canvas_->redraw();} scale_=s;};
    int scale(){return scale_;};

    Ca_Axis_(int x, int y, int w, int h, const char * label=0);
    ~Ca_Axis_();
};


//////////////////////////////////////////////////////////////////////
class Ca_X_Axis: public Ca_Axis_{

protected:
    int min_pos();
    int max_pos();
    void draw();
    void draw_grid();
public:
    void current();
    Ca_X_Axis(int x, int y, int w, int h, const char * label=0);
    ~Ca_X_Axis();

};

///////////////////////////////////////////////////////////////////////////

class Ca_Y_Axis: public Ca_Axis_{
    int min_pos();
    int max_pos();
    void draw();
    void draw_grid();
public:
    void current();
    Ca_Y_Axis(int x, int y, int w, int h, const char * label=0);
    ~Ca_Y_Axis();

};

///////////////////////////////////////////////////////////////////////////

class Ca_Object_{

    friend class Ca_Canvas;
    friend class Ca_Axis_;
    friend class Ca_Y_Axis;
    friend class Ca_X_Axis;

    // No function body - prevents copy construction/assignment
    Ca_Object_(const Ca_Object_ &);
    const Ca_Object_ & operator=(const Ca_Object_ &);

protected:
    Ca_Canvas *canvas_;
    Ca_Axis_ *x_axis_;
    Ca_Axis_ *y_axis_;
    virtual void draw()=0;

public:
  Ca_Canvas * canvas() const {return canvas_;}
    Ca_Object_(Ca_Canvas * canvas=0);
    virtual ~Ca_Object_();

};

///////////////////////////////////////////////////////////////////////////

class Ca_Point:public Ca_Object_{

protected:
    void draw();

public:
    double x;
    double y;
    int style;
    int size;
    Fl_Color color;
    Fl_Color border_color;
    int border_width;
    Ca_Point(double _x, double _y, Fl_Color color=FL_BLACK, int style=CA_SIMPLE, int size=DEFAULT_POINT_SIZE, Fl_Color border_color=FL_BLACK, int border_width=0);
};

///////////////////////////////////////////////////////////////////////////

class Ca_LinePoint:public Ca_Point{

protected:
    void draw();

public:
    Ca_LinePoint *previous;
    int line_width;
    Ca_LinePoint(Ca_LinePoint *_previous, double _x, double _y, int line_width, Fl_Color color=FL_BLACK,  int style=CA_SIMPLE, int size=DEFAULT_POINT_SIZE, Fl_Color border_color=FL_BLACK, int border_width=0);
    Ca_LinePoint(Ca_LinePoint *_previous, double _x, double _y);
};


///////////////////////////////////////////////////////////////////////////

class Ca_PolyLine:public Ca_LinePoint{

protected:
    void draw();
public:
    int line_style;
    Ca_PolyLine * next;
    Ca_PolyLine(Ca_PolyLine *_previous, double _x, double _y,int line_style, int line_width=0, Fl_Color color=FL_BLACK, int style=CA_SIMPLE, int size=DEFAULT_POINT_SIZE,  Fl_Color border_color=FL_BLACK, int border_width=0);
    Ca_PolyLine(Ca_PolyLine *_previous, double _x, double _y);
};

///////////////////////////////////////////////////////////////////////////

class Ca_Line:protected Ca_Point{

protected:
    void draw();

public:
    using Ca_Point::style; //just making public usefull data
    using Ca_Point::size;
    using Ca_Point::color;
    using Ca_Point::border_color;
    using Ca_Point::border_width;

    int line_style;
    int line_width;
    int n;
    double * data;
    double * data_2;

    Ca_Line(int _n, double *_data, double *_data_2, int _line_style, int _line_width=0, Fl_Color color=FL_BLACK, int style=CA_SIMPLE, int size=DEFAULT_POINT_SIZE, Fl_Color border_color=FL_BLACK, int border_width=0);
    Ca_Line(int _n, double *_data, int _line_style, int _line_width=0, Fl_Color _color=FL_BLACK, int style=CA_SIMPLE, int size=DEFAULT_POINT_SIZE, Fl_Color border_color=FL_BLACK, int border_width=0);


};

///////////////////////////////////////////////////////////////////////////

class Ca_Text:public Ca_Object_{

protected:
    void draw();

public:
    double x1,x2,y1,y2;
    const char * label;
    uchar align;
    Fl_Color label_color;
    Fl_Font label_font;
    int label_size;
    Ca_Text(double _x1, double _x2, double _y1, double _y2, const char *_label, uchar _align=FL_ALIGN_CENTER, Fl_Font _label_font=FL_HELVETICA, int _label_size=CA_DEFAULT_LABEL_SIZE, Fl_Color _label_color=FL_BLACK);
    Ca_Text(double x, double y, char *_label=0, uchar _align=FL_ALIGN_CENTER, Fl_Font _label_font=FL_HELVETICA, int _label_size=CA_DEFAULT_LABEL_SIZE, Fl_Color _label_color=FL_BLACK);
};

///////////////////////////////////////////////////////////////////////////

class Ca_Bar:public Ca_Text{
protected:
    void draw();
public:
    Fl_Color color;
    Fl_Color border_color;
    int border_width;
    Ca_Bar(double _x1, double _x2, double _y1, double _y2, Fl_Color _color=FL_RED,  Fl_Color _border_color=FL_BLACK, int _border_width=0,  const char *_label=0, uchar _align=FL_ALIGN_CENTER, Fl_Font _label_font=FL_HELVETICA, int _label_size=CA_DEFAULT_LABEL_SIZE, Fl_Color _label_color=FL_BLACK);
};


#endif
