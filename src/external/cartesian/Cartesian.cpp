// Cartesian.cpp,v 1.1
//
// Copyright 2000-2005 by Roman Kantor.
//
// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU Library General Public License
// version 2 as published by the Free Software Foundation.
//
// This library is distributed  WITHOUT ANY WARRANTY;
// WITHOUT even the implied warranty of MERCHANTABILITY
// or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// Library General Public License for more details.
// You should have received a copy of the GNU Library General Public
// License along with this library; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA.


#include "Cartesian.H"
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <stdlib.h>
#include <FL/fl_draw.H>
#include <FL/Fl.H>
#include <FL/Fl_Group.H>




static const int	CANVAS_BORDER = 0;                //gap between the graphics and surrounding"box"
static const int        AXIS_BORDER = 0;                  //gap between axis drawing(i.e.axis line) and its "box"
static const int        MINOR_INTERVAL = 0;               //0 stands for automatic choice in default_*_intervals array
static const int        MINOR_SEPARATION = 18;
static const int        MAJOR_STEP = 5;
static const int        LABEL_STEP = 10;
static const int        LABEL_SIZE = CA_DEFAULT_LABEL_SIZE;
static const Fl_Font    LABEL_FONT = FL_HELVETICA;


static const int MAX_LABEL_FORMAT = 16;
static const int MAX_LABEL_LENGTH = 32;


static const int NO_LIN_DEFAULTS=3;
static const double default_lin_intervals[NO_LIN_DEFAULTS] = {1, 2, 5};
static const int default_lin_major_steps[NO_LIN_DEFAULTS] = {5, 5, 2};
static const int default_lin_label_steps[NO_LIN_DEFAULTS] = {10, 5, 4};

static const int NO_LOG_DEFAULTS = 3;
static const double default_log_intervals[NO_LOG_DEFAULTS] = {1, 2, 5};
static const int default_log_major_steps[NO_LOG_DEFAULTS] = {5, 5, 2};
static const int default_log_label_steps[NO_LOG_DEFAULTS] = {10, 5, 2};



/// float drawings for more precise placement (especialy for PS output for Fl_Device!) /////

static inline void ca_rect(double x, double y, double w, double h){
  fl_begin_loop();
  fl_vertex(x,y);
  fl_vertex(x+w,y);
  fl_vertex(x+w,y+h);
  fl_vertex(x,y+h);
  fl_end_loop();
};

static inline void ca_rectf(double x, double y, double w, double h){
  fl_begin_polygon();
  fl_vertex(x,y);
  fl_vertex(x+w,y);
  fl_vertex(x+w,y+h);
  fl_vertex(x,y+h);
  fl_end_polygon();
};

static inline void ca_loop(double x1, double y1, double x2, double y2, double x3, double y3){
  fl_begin_loop();
  fl_vertex(x1,y1); fl_vertex(x2,y2); fl_vertex(x3,y3);
  fl_end_loop();
};

static inline void ca_polygon(double x1, double y1, double x2, double y2, double x3, double y3){
  fl_begin_polygon();
  fl_vertex(x1,y1); fl_vertex(x2,y2); fl_vertex(x3,y3);
  fl_end_polygon();
};

static inline void ca_loop(double x1, double y1, double x2, double y2, double x3, double y3, double x4, double y4){
  fl_begin_loop();
  fl_vertex(x1,y1); fl_vertex(x2,y2); fl_vertex(x3,y3); fl_vertex(x4,y4);
  fl_end_loop();
};

static inline void ca_polygon(double x1, double y1, double x2, double y2, double x3, double y3, double x4, double y4){
  fl_begin_polygon();
  fl_vertex(x1,y1); fl_vertex(x2,y2); fl_vertex(x3,y3); fl_vertex(x4,y4);
  fl_end_polygon();
};

static inline void ca_text(const char  *label, double x, double y){
  fl_draw(label,(int)(x+.5),(int)(y+.5));
};
static inline void ca_point(double x, double y){
  fl_point((int)(x+.5),(int)(y+.5));
};

/*
static inline void ca_pie(double x, double y, double w, double h, double a1, double a2){
fl_pie((int)(x+.5), (int)(y+.5),(int)(w+.5),(int)(h+.5),0,270.0);
};
*/

static inline void ca_filled_circle(double x, double y, double r){
  fl_begin_polygon();
  //fl_arc(x,y,r,0,360);
  fl_circle(x,y,r);
  fl_end_polygon();
};

static inline void ca_text(const char  *label, double x, double y, double w, double h, Fl_Align align){
  fl_draw(label, (int)(x+.5), (int)(y+.5), (int)(w+.5), (int)(h+.5), align);
};






////////////////////    Ca_Axis_    ////////////////////////////

void Ca_Axis_::minimum(double x){
  //if(min_ == x) return;
  min_=x;
  if(!valid_){
    max_=x;
    valid_=1;
  }
  damage(CA_DAMAGE_ALL);
  if(canvas_)
    canvas_->damage(CA_DAMAGE_ALL);
  update();

};

void Ca_Axis_::maximum(double x){
  //if(max_==x) return;
  max_=x;
  if(!valid_){
    min_=x;
    valid_=1;
  }
  damage(CA_DAMAGE_ALL);
  if(canvas_)
    canvas_->damage(CA_DAMAGE_ALL);
  update();

};


int Ca_Axis_::update(){

  double _k=k_;
  double _q=q_;
  min_pos_=min_pos();
  max_pos_=max_pos();
  if (min_==max_)
    k_=0;
  else
    if(scale_ & CA_LOG){
      k_=(max_pos_-min_pos_)/(log(max_)-log(min_));
      q_=min_pos_-k_*log(min_);
    }else{
      k_=(max_pos_-min_pos_)/(max_-min_);
      q_=min_pos_;
    }

    if((_k!=k_)||(_q!=q_))
      return 1;
    else
      return 0;
};

void Ca_Axis_::rescale_move(int when, double  x){


  if((when&CA_WHEN_MAX)&&(x>max_)){
    if(scale_ & CA_LOG)
      min_ *=x/max_;
    else
      min_ += x-max_;
    max_=x;
    damage(CA_DAMAGE_ALL);
    if(canvas_)
      canvas_->damage(CA_DAMAGE_ALL);

  }
  if((when&CA_WHEN_MIN)&&(x<min_)){
    if(scale_ & CA_LOG)
      max_ *=x/min_;
    else
      max_ -= min_-x;
    min_=x;
    damage(CA_DAMAGE_ALL);
    if(canvas_)
      canvas_->damage(CA_DAMAGE_ALL);
  }
  valid_=1;
};

double Ca_Axis_::position(double value){

  if (k_==0) return (min_pos_+max_pos_)/2;
  if(scale_ & CA_LOG)
    return (int)(q_+k_*log(value));
  else
    return min_pos_+k_*(value-min_);
};

double Ca_Axis_::value(double pos){
  if (max_==min_)
    return min_;
  if(scale_ & CA_LOG)
    return exp((pos-q_)/k_);
  else
    return (min_ +(pos-min_pos_)/k_);
};



int Ca_Axis_::next_tick(int &tick_index, double &tick_value, int &tick_order, double &interval ){


  ////////// I know snakes are evil creatures, but sometimes they work so there is such a serpent....
  ////////// How many if...else can be in in a function? this is going to be a record in the G. book of r.

  static int number_per_order;
  double _tick_interval;
  double minor_number_;


  if(scale_ & CA_LOG){   /////////////     begin logarithmic   /////////////////
    if (!interval){
      tick_order=(int)(floor(log10(min_)));
      if (tick_interval_!=0){
        interval=fabs(tick_interval_);
        number_per_order=(int)floor(10/interval+0.5);
      }else{
        number_per_order=(int)(abs(min_pos_-max_pos_)/(tick_separation_*log10(max_/min_)));
        if(number_per_order<=1){
          label_step_=major_step_=3;
          tick_order = 3*(tick_order/3);
          interval=1;
          number_per_order=0;
        }else{
          int _no_per_o=number_per_order;
          for(int i=NO_LOG_DEFAULTS-1;i>=0;i--){
            major_step_=default_log_major_steps[i];
            label_step_=default_log_label_steps[i];
            interval=default_log_intervals[i];
            number_per_order=(int)floor(10/interval+0.5);
            if((10/interval)>=_no_per_o)
              break;
          }
        }
      }
      tick_index=number_per_order;
      tick_order--;
      tick_value=pow(10.0f,tick_order);
      interval*=tick_value;

      if(!number_per_order){
        tick_order--;
        tick_value /=10;
        tick_index=1;
      }else
        tick_value *=10;
      return 1;
    }else{
      if (tick_value>(max_)){
        tick_index-=1;
        return 0;
      }else{
        if(number_per_order){
          if(tick_index==number_per_order){
            tick_order++;
            interval*=10;
            if(number_per_order<10){
              tick_index=1;
              tick_value=interval;
            }else{
              tick_index=2;
              tick_value=2*interval;
            }
          }else{
            tick_value +=interval;
            tick_index++;
          }
        }else{
          tick_order++;
          tick_index++;
          tick_value *=10;
        }
        return 1;
      }
    }

  }else{     ///////////////     begin linear       //////////////////////
    if (!interval){
      minor_number_= (double)abs(min_pos_-max_pos_)/(double)tick_separation_;
      _tick_interval=tick_interval_;
      if (_tick_interval<0){
        interval=_tick_interval=-_tick_interval;
        tick_order=(int)floor(log10(_tick_interval));
      }else{
        if(_tick_interval!=0){
          tick_order=(int)floor(log10(fabs(max_-min_)/minor_number_));
          interval= pow(10.0,tick_order) * _tick_interval;
        }else
          for(int i=NO_LIN_DEFAULTS-1;i>=0;i--){
            tick_order=(int)floor(log10(fabs(max_-min_)/minor_number_));
            interval= pow(10.0,tick_order)*(_tick_interval=default_lin_intervals[i]);
            major_step_=default_lin_major_steps[i];
            label_step_=default_lin_label_steps[i];
            if(((max_-min_)/interval)>=minor_number_)
              break;
          }
      }
      tick_value = floor(minimum()/interval);
      tick_value *= interval;
      tick_index=(int) floor((tick_value /interval)+0.5);
      return 1;
    }else{
      if (tick_value>(max_)){
        tick_index=-1;
        return 0;
      }else{
        tick_value +=interval;
        tick_index++;
        return 1;
      }
    }
  }		/////   Uf, this is the end of the leg-less beast!   //////
};



void Ca_Axis_::rescale(int when, double  x){
  if(!valid_){
    max_=x;
    min_=x;
    damage(CA_DAMAGE_ALL);
    if(canvas_)
      canvas_->damage(CA_DAMAGE_ALL);
    valid_=1;
    return;
  }

  if((when&CA_WHEN_MAX)&&(x>max_)){
    max_=x;
    damage(CA_DAMAGE_ALL);
    if(canvas_)
      canvas_->damage(CA_DAMAGE_ALL);
  }
  if((when&CA_WHEN_MIN)&&(x<min_)){
    min_=x;
    damage(CA_DAMAGE_ALL);
    if(canvas_)
      canvas_->damage(CA_DAMAGE_ALL);
  }
};

Ca_Axis_::Ca_Axis_(int x, int y, int w, int h, const char * label)
:Fl_Box(x,y,w,h,label),
scale_(CA_LIN), valid_(0), k_(0), q_(0), label_format_(0),
minor_grid_color_(FL_BLACK), major_grid_color_(FL_BLACK), label_grid_color_(FL_BLACK),
minor_grid_style_(FL_SOLID), major_grid_style_(FL_SOLID), label_grid_style_(FL_SOLID),
minor_grid_width_(0), major_grid_width_(0), label_grid_width_(0),
minor_grid_dashes_(0), major_grid_dashes_(0),label_grid_dashes_(0),
grid_visible_(0), tick_interval_(MINOR_INTERVAL), tick_separation_(MINOR_SEPARATION),
tick_length_(0), tick_width_(0), major_step_(MAJOR_STEP),label_step_(LABEL_STEP),
axis_align_(CA_BOTTOM),label_font_face_(FL_HELVETICA), label_font_size_(LABEL_SIZE),
min_(0),max_(0),min_pos_(0),max_pos_(0),border_(AXIS_BORDER),axis_color_(FL_BLACK)

{
  widget_ = 0;
  box(FL_NO_BOX);
  canvas_=Ca_Canvas::current();
  if(canvas_){
    previous_axis_=canvas_->last_axis_;
    canvas_->last_axis_=this;
  }
  labelsize(LABEL_SIZE);
};

Ca_Axis_::~Ca_Axis_(){

  if(!canvas_)return;
  if (canvas_->last_axis_==this)
    canvas_->last_axis_=previous_axis_;
  else{
    Ca_Axis_ *axis=canvas_->last_axis_;
    while(axis){
      if(axis->previous_axis_==this){
        axis->previous_axis_=previous_axis_;
        break;
      }
      axis=axis->previous_axis_;
    }
  };
};



///////////////////////  Ca_X_Axis  ///////////////////////////////////////////////

int Ca_X_Axis::min_pos(){
  int BD = 0;
  Fl_Widget * W_ = this;
  if(canvas_){
    BD = canvas_->border();
    W_ = canvas_;
  }else if(widget_){
    W_ = widget_;
  }

  if(scale_&CA_REV)
    return W_->x()+W_->w()-BD+Fl::box_dx(W_->box())-Fl::box_dw(W_->box());
  else
    return W_->x()+BD+Fl::box_dx(W_->box());

};

int Ca_X_Axis::max_pos(){
  int BD = 0;
  Fl_Widget * W_ = this;
  if(canvas_){
    BD = canvas_->border();
    W_ = canvas_;
  }else if(widget_){
    W_ = widget_;
  }
  if(scale_&CA_REV)
    return W_->x()+BD+Fl::box_dx(W_->box());
  else
    return W_->x()+W_->w()-BD+Fl::box_dx(W_->box())-Fl::box_dw(W_->box());

};


void Ca_X_Axis::draw(){
  if(min_==max_) return;
  int BD = 0;
  Fl_Widget * W_ = this;
  if(canvas_){
    BD = canvas_->border();
    W_ = canvas_;
  }else if(widget_){
    W_ = widget_;
  }
  int tick_index=-1;
  double tick_value;
  int tick_order;//, tick_number;
  double _interval=0;
  const char * label_format=label_format_;
  if(damage()|FL_DAMAGE_ALL)
    draw_label();
  if (damage()&(FL_DAMAGE_ALL|CA_DAMAGE_ALL)){
    update();
    if (box()==FL_NO_BOX){
      fl_color(parent()->color());
      fl_rectf(x(),y(),w(),h());
    }else
      draw_box();
    if(!valid_) return;
    fl_font(label_font_face_,label_font_size_);
    int l1=0;
    int l2=0;
    int m1=0;
    int m2=0;
    int l=0;
    int _y=0;
    int _w=0;
    int _h=0; //temporary coordinates for ticks
    double _pos,_x;
    //fl_clip(x()+Fl::box_dx(box()),y()+Fl::box_dy(box()),w()-Fl::box_dw(box()),h()-Fl::box_dh(box()));
    fl_color(axis_color_);
    int a=y()+Fl::box_dh(box())+border_;
    int b=a+h()-Fl::box_dh(box())-2*border_;

    switch(axis_align_ & CA_ALIGNMENT){
            case CA_BOTTOM:
              l=l1=m1=a;
              if(axis_align_&CA_NO_TICS)
                m2=m1;
              else
                if (tick_length_)
                  m2=m1+tick_length_;
                else
                  m2=m1+label_font_size_;
              l2=(m1+m2)/2;
              break;
            case CA_TOP:
              l=l2=m2=b-1;
              if(axis_align_&CA_NO_TICS)
                m1=m2;
              else
                if (tick_length_)
                  m1=m2-tick_length_;
                else
                  m1=m2-label_font_size_;
              l1=(m1+m2)/2;
              break;
            case CA_CENTER:
              m1=a;
              m2=b;
              l=(a+b)/2;
              l1=(a+l)/2;
              l2=(l+b)/2;
              break;
    }
    fl_line_style(FL_SOLID|FL_CAP_FLAT,tick_width_);
    double start_tick;
    double end_tick;
    bool tick_not_started = 1;

    while(next_tick(tick_index, tick_value, tick_order, _interval)){
      _pos=position(tick_value);
      if(scale_&CA_REV){
        if((_pos+1)<max_pos_-BD) break;
        if((_pos-1)>min_pos_+BD) continue;
      }else{
        if((_pos+1)<min_pos_-BD) continue;
        if((_pos-1)>max_pos_+BD) break;
      }
      if(!(axis_align_&CA_NO_TICS)){
        if(tick_index % major_step_){
          fl_begin_loop();
          fl_vertex(_pos,l1);
          fl_vertex(_pos,l2);
          fl_end_loop();
        }else{
          fl_begin_loop();
          fl_vertex(_pos,m1);
          fl_vertex(_pos,m2);
          fl_end_loop();
        }
        if(tick_not_started){
          tick_not_started = 0;
          start_tick = _pos;
        }
        end_tick = _pos;
      }

      if(!((tick_index % label_step_)|(axis_align_&CA_NO_LABELS))){
        char label[MAX_LABEL_LENGTH];
        char _label_format[MAX_LABEL_FORMAT];
        if(!label_format){
          int _tick_order;
          if (tick_order>=0)
            _tick_order=0;
          else
            _tick_order=-tick_order - 1;
          sprintf(_label_format,"%s.%if","%",_tick_order);
        }
        else
          strcpy(_label_format,label_format);
        sprintf(label, _label_format,tick_value);
        fl_measure(label,_w,_h);
        _x=_pos-_w/double(2);
        switch (axis_align_ & CA_ALIGNMENT){
                    case CA_TOP:
                      _y=m1-_h/3;
                      break;
                    case CA_BOTTOM:
                      _y=m2+_h;
                      break;
                    case CA_CENTER:
                      _y=l+_h/3;
                      Fl_Color _color=fl_color();
                      fl_color(color());
                      ca_rectf(_x-_h/6,l-_h/2,_w+_h/3,_h);
                      fl_color(_color);
                      break;
        }
        ca_text(label,_x,_y);
      }
    }
    if((axis_align_ & CA_LINE) && !tick_not_started){
      fl_begin_line();
      fl_vertex(start_tick,l);
      fl_vertex(end_tick,l);

      //fl_vertex(min_pos(),l);
      //fl_vertex(max_pos(),l);
      //fl_vertex(W_->x()+Fl::box_dx(W_->box()),l);
      //fl_vertex(W_->x()+W_->w()+Fl::box_dx(W_->box())-Fl::box_dw(W_->box()),l);

      fl_end_line();
    }
    fl_line_style(0,0);
    //  fl_pop_clip();
  }
};


void Ca_X_Axis::current(){
  if(canvas_)
    canvas_->current_x(this);
};

void Ca_X_Axis::draw_grid(){
  if(!valid_)return;
  if(max_==min_)return;
  int tick_index=-1;
  double tick_value;
  int tick_order;
  double _interval=0;
  int l1,l2;
  double _pos;

  int BD = 0;
  Fl_Widget * W_ = this;
  if(canvas_){
    BD = canvas_->border();
    W_ = canvas_;
  }else if(widget_){
    W_ = widget_;
  }

  l1=W_->y()+Fl::box_dy(W_->box());
  l2=W_->y()+W_->h()+Fl::box_dy(W_->box())-Fl::box_dh(W_->box());
  int tcl;
  if(!(tcl=tick_length_))
    tcl=label_font_size_;
  while(next_tick(tick_index, tick_value, tick_order, _interval)){
    _pos=position(tick_value);
    if(scale_&CA_REV){
      if(_pos<max_pos_-BD) break;
      if(_pos>min_pos_+BD) continue;
    }else{
      if(_pos<min_pos_-BD) continue;
      if(_pos>max_pos_+BD) break;
    }
    int grt;
    if((grt=grid_visible_ & CA_LABEL_GRID) && !(tick_index % label_step_)){
      fl_color(label_grid_color_);
      fl_line_style(label_grid_style_,label_grid_width_);
      if(grt==CA_LABEL_GRID){
        fl_begin_loop();
        fl_vertex(_pos,l1);
        fl_vertex(_pos,l2);
        fl_end_loop();

      }else{
        if(grt&CA_LEFT_LABEL_TICK){
          fl_begin_loop();
          fl_vertex(_pos,l1);
          fl_vertex(_pos,l1+tcl);
          fl_end_loop();
        }
        if(grt&CA_RIGHT_LABEL_TICK){
          fl_begin_loop();
          fl_vertex(_pos,l2-tcl);
          fl_vertex(_pos,l2);
          fl_end_loop();
        }
      }
    }else if((grt=grid_visible_ & CA_MAJOR_GRID) && !(tick_index % major_step_)){
      fl_color(major_grid_color_);
      fl_line_style(major_grid_style_,major_grid_width_);
      if(grt==CA_MAJOR_GRID){
        fl_begin_loop();
        fl_vertex(_pos,l1);
        fl_vertex(_pos,l2);
        fl_end_loop();

      }else{
        if(grt&CA_LEFT_MAJOR_TICK){
          fl_begin_loop();
          fl_vertex(_pos,l1);
          fl_vertex(_pos,l1+tcl);
          fl_end_loop();
        }
        if(grt&CA_RIGHT_MAJOR_TICK){
          fl_begin_loop();
          fl_vertex(_pos,l2-tcl);
          fl_vertex(_pos,l2);
          fl_end_loop();
        }
      }
    }else if((grt=grid_visible_&CA_MINOR_GRID)){
      fl_color(minor_grid_color_);
      fl_line_style(minor_grid_style_,minor_grid_width_);
      if(grt==CA_MINOR_GRID){
        fl_begin_loop();
        fl_vertex(_pos,l1);
        fl_vertex(_pos,l2);
        fl_end_loop();

      }else{
        if(grt&CA_LEFT_MINOR_TICK){
          fl_begin_loop();
          fl_vertex(_pos,l1);
          fl_vertex(_pos,l1+tcl/2);
          fl_end_loop();
        }
        if(grt&CA_RIGHT_MINOR_TICK){
          fl_begin_loop();
          fl_vertex(_pos,l2-tcl/2);
          fl_vertex(_pos,l2);
          fl_end_loop();
        }
      }
    }
  }
  fl_line_style(0,0);
  fl_color(FL_BLACK);
};



Ca_X_Axis::Ca_X_Axis(int x, int y, int w, int h, const char *label):Ca_Axis_(x, y, w,  h,  label){
  if(canvas_ && !(canvas_->current_x()))
    current();
};


Ca_X_Axis::~Ca_X_Axis(){
  if(canvas_){
    Ca_ObjectChain *ochain=canvas_->first_object_;
    Ca_ObjectChain *next;
    Ca_ObjectChain *previous=0;
    while (ochain){
      next=ochain->next;
      if(ochain->object->x_axis_==this){
        delete ochain->object;
        if(previous)
          previous->next=next;
        else
          canvas_->first_object_=next;
        delete ochain;
      }
      ochain=next;
    }
  }
}



////////////////////////////   Ca_Y_Axis  //////////////////////////////////////////////////////

int Ca_Y_Axis::min_pos(){
  int BD = 0;
  Fl_Widget * W_ = this;
  if(canvas_){
    BD = canvas_->border();
    W_ = canvas_;
  }else if(widget_){
    W_ = widget_;
  }
  if (scale_&CA_REV)
    return W_->y()+BD+Fl::box_dy(W_->box());
  else
    return W_->y()+W_->h()-BD+Fl::box_dy(W_->box())-Fl::box_dh(W_->box());

};

int Ca_Y_Axis::max_pos(){
  int BD = 0;
  Fl_Widget * W_ = this;
  if(canvas_){
    BD = canvas_->border();
    W_ = canvas_;
  }else if(widget_){
    W_ = widget_;
  }
  if (scale_&CA_REV)
    return W_->y()+W_->h()-BD+Fl::box_dy(W_->box())-Fl::box_dh(W_->box());
  else
    return W_->y()+BD+Fl::box_dy(W_->box());


};

void Ca_Y_Axis::draw(){
  if(min_==max_) return;
  int BD = 0;
  Fl_Widget * W_ = this;
  if(canvas_){
    BD = canvas_->border();
    W_ = canvas_;
  }else if(widget_){
    W_ = widget_;
  }
  int tick_index=-1;
  double tick_value;
  int tick_order;//,tick_number;
  double _interval=0;
  const char * label_format=label_format_;
  //    if(damage()|FL_DAMAGE_ALL)
  //        draw_label();
  if (damage()&(FL_DAMAGE_ALL|CA_DAMAGE_ALL)){
    update();
    if (box()==FL_NO_BOX){
      fl_color(parent()->color());
      fl_rectf(x(),y(),w(),h());
    }else
      draw_box();
    if(!valid_) return;
    fl_font(label_font_face_,label_font_size_);
    int l1=0; int l2=0; int m1=0; int m2=0; int l=0; int _x=0; int _w,_h; //temporary coordinates for ticks
    double _pos,_y;
    fl_clip(x()+Fl::box_dx(box()),y()+Fl::box_dy(box()),w()-Fl::box_dw(box()),h()-Fl::box_dh(box()));
    fl_color(axis_color_);
    int a=x()+Fl::box_dx(box())+border_;
    int b=a+w()-Fl::box_dw(box())-2*border_;
    switch(axis_align_ & CA_ALIGNMENT){
            case CA_RIGHT:
              l=l1=m1=a;
              if(axis_align_&CA_NO_TICS)
                m2=m1;
              else
                if (tick_length_)
                  m2=m1+tick_length_;
                else
                  m2=m1+label_font_size_;
              l2=(l1+m2)/2;
              break;
            case CA_LEFT:
              l=l2=m2=b-1;
              if(axis_align_&CA_NO_TICS)
                m1=m2;
              else
                if (tick_length_)
                  m1=m2-tick_length_;
                else
                  m1=m2-label_font_size_;
              l1=(m1+m2)/2;
              break;
            case CA_CENTER:
              m1=a;
              m2=b;
              l=(a+b)/2;
              l1=(a+l)/2;
              l2=(l+b)/2;
              break;
    }
    fl_line_style(FL_SOLID|FL_CAP_FLAT,tick_width_);
    //		double minp,maxp;
    double start_tick;
    double end_tick;
    bool tick_not_started = 1;


    while(next_tick(tick_index, tick_value, tick_order, _interval)){
      _pos=position(tick_value);
      if(scale_&CA_REV){
        if((_pos+1)<min_pos_-BD) continue;
        if((_pos-1)>max_pos_+BD) break;
      }else{
        if((_pos+1)<max_pos_-BD) break;
        if((_pos-1)>min_pos_+BD) continue;
      }
      if(!(axis_align_&CA_NO_TICS)){
        fl_begin_loop();
        if(tick_index % major_step_){
          fl_vertex(l1,_pos);
          fl_vertex(l2,_pos);
        }else{
          fl_vertex(m1,_pos);
          fl_vertex(m2,_pos);
        }
        fl_end_loop();
        if(tick_not_started){
          tick_not_started = 0;
          start_tick = _pos;
        }
        end_tick = _pos;
      }
      if(!((tick_index % label_step_)|(axis_align_&CA_NO_LABELS))){
        char label[MAX_LABEL_LENGTH];
        char _label_format[MAX_LABEL_FORMAT];
        if(!label_format){
          int _tick_order;
          if (tick_order>=0)
            _tick_order=0;
          else
            _tick_order=-tick_order - 1;
          sprintf(_label_format,"%s.%if","%",_tick_order);
        }
        else
          strcpy(_label_format,label_format);
        sprintf(label, _label_format,tick_value);
        fl_measure(label,_w,_h);
        _y=_pos+_h/3;
        switch (axis_align_ & CA_ALIGNMENT){
                    case CA_LEFT:
                      _x=m1-_h/3-_w;
                      break;
                    case CA_RIGHT:
                      _x=m2+_h/3;
                      break;
                    case CA_CENTER:
                      _x=(m1+m2)/2-_w/2;
                      Fl_Color _color=fl_color();
                      fl_color(color());
                      ca_rectf(_x-_h/6,_pos-_h/2,_w+_h/3,_h);
                      fl_color(_color);
                      break;
        }
        ca_text(label,_x,_y);
      }
    }
    if((axis_align_ & CA_LINE) && !tick_not_started){
      fl_begin_line();
      fl_vertex(l,start_tick);
      fl_vertex(l,end_tick);
      //fl_vertex(l,W_->y()+Fl::box_dy(W_->box()));
      //fl_vertex(l,W_->y()+W_->h()+Fl::box_dy(W_->box())-Fl::box_dh(W_->box()));

      fl_end_line();
    }
    fl_line_style(0);
    fl_pop_clip();
  }
};


void Ca_Y_Axis::current(){
  if(canvas_)
    canvas_->current_y(this);
};



void Ca_Y_Axis::draw_grid(){
  if(!valid_)return;
  if(max_==min_)return;
  int tick_index=-1;
  double tick_value;
  int tick_order;
  double _interval=0;
  int l1,l2;

  int BD = 0;
  Fl_Widget * W_ = this;
  if(canvas_){
    BD = canvas_->border();
    W_ = canvas_;
  }else if(widget_){
    W_ = widget_;
  }
  l1=W_->x()+Fl::box_dx(W_->box());
  l2=W_->x()+W_->w()+Fl::box_dx(W_->box())-Fl::box_dw(W_->box());
  int tcl;
  if(!(tcl=tick_length_))
    tcl=label_font_size_;
  while(next_tick(tick_index, tick_value, tick_order,_interval)){
    double _pos=position(tick_value);
    if(scale_&CA_REV){
      if(_pos<min_pos_-BD) continue;
      if(_pos>max_pos_+BD) break;
    }else{
      if(_pos<max_pos_-BD) break;
      if(_pos>min_pos_+BD) continue;
    }
    int grt;
    if((grt=grid_visible_&CA_LABEL_GRID) && !(tick_index % label_step_)){
      fl_color(label_grid_color_);
      fl_line_style(label_grid_style_,label_grid_width_);
      if(grt==CA_LABEL_GRID){
        fl_begin_loop();
        fl_vertex(l1,_pos);
        fl_vertex(l2,_pos);
        fl_end_loop();

      }else{
        if(grt&CA_LEFT_LABEL_TICK){
          fl_begin_loop();
          fl_vertex(l1,_pos);
          fl_vertex(l1+tcl,_pos);
          fl_end_loop();
        }
        if(grt&CA_RIGHT_LABEL_TICK){
          fl_begin_loop();
          fl_vertex(l2-tcl,_pos);
          fl_vertex(l2,_pos);
          fl_end_loop();
        }
      }
    }else if((grt=grid_visible_&CA_MAJOR_GRID) && !(tick_index % major_step_)){
      fl_color(major_grid_color_);
      fl_line_style(major_grid_style_,major_grid_width_);
      if(grt==CA_MAJOR_GRID){
        fl_begin_loop();
        fl_vertex(l1,_pos);
        fl_vertex(l2,_pos);
        fl_end_loop();

      }else{
        if(grt&CA_LEFT_MAJOR_TICK){
          fl_begin_loop();
          fl_vertex(l1,_pos);
          fl_vertex(l1+tcl,_pos);
          fl_end_loop();
        }
        if(grt&CA_RIGHT_MAJOR_TICK){
          fl_begin_loop();
          fl_vertex(l2-tcl,_pos);
          fl_vertex(l2,_pos);
          fl_end_loop();
        }
      }
    }else if((grt=(grid_visible_&CA_MINOR_GRID))){
      fl_color(minor_grid_color_);
      fl_line_style(minor_grid_style_,minor_grid_width_);
      if(grt==CA_MINOR_GRID){
        fl_begin_loop();
        fl_vertex(l1,_pos);
        fl_vertex(l2,_pos);
        fl_end_loop();

      }else{
        if(grt&CA_LEFT_MINOR_TICK){
          fl_begin_loop();
          fl_vertex(l1,_pos);
          fl_vertex(l1+tcl/2,_pos);
          fl_end_loop();
        }
        if(grt&CA_RIGHT_MINOR_TICK){
          fl_begin_loop();
          fl_vertex(l2-tcl/2,_pos);
          fl_vertex(l2,_pos);
          fl_end_loop();
        }
      }
    }
  }

  fl_line_style(0,0);
  fl_color(FL_BLACK);
};


Ca_Y_Axis::Ca_Y_Axis(int x, int y, int w, int h, const char * label):Ca_Axis_(x, y, w,  h,  label){
  if(canvas_ && !(canvas_->current_y()))
    current();
  axis_align(CA_LEFT);
};

Ca_Y_Axis::~Ca_Y_Axis(){
  if(canvas_){
    Ca_ObjectChain *ochain=canvas_->first_object_;
    Ca_ObjectChain *next;
    Ca_ObjectChain *previous=0;
    while (ochain){
      next=ochain->next;
      if(ochain->object->y_axis_==this){
        delete ochain->object;
        if(previous)
          previous->next=next;
        else
          canvas_->first_object_=next;
        delete ochain;
      }
      ochain=next;
    }
  }
}

/////////////////////////////// Ca_Canvas  ////////////////////////////////////////////

Ca_Canvas *Ca_Canvas::current_=0;

void Ca_Canvas::draw(){

  uchar damage_= damage();
  // int _b=border_/2;
  // int _x=x()+_b;
  // int _y=y()+_b;
  // int _w=w()-2*_b;
  // int _h=h()-2*_b;
  int replot=0;

  Ca_Axis_ *axis = last_axis_;

  /// something similar will go in the future into the lauout layer...
  while (axis){
    replot |= axis->update();
    axis=axis->previous_axis_;
  }
  ///
  if(damage_!=CA_DAMAGE_ADD)
    draw_box();

  if((damage_!=CA_DAMAGE_ADD)||replot){
    last_plotted_=0;
    axis=last_axis_;
    while(axis){
      if(!(axis->grid_visible()&CA_FRONT)&&(axis->visible()||(axis->grid_visible()&CA_ALWAYS_VISIBLE)))
        axis->draw_grid();
      axis=axis->previous_axis_;
    }
  }

  fl_clip(x()+Fl::box_dx(box())+dx_, y()+Fl::box_dy(box())+border_+dy_, w()-Fl::box_dw(box()) - dw_, h()-Fl::box_dh(box()) - dh_);
  if (last_plotted_)
    last_plotted_=last_plotted_->next;
  else
    last_plotted_=first_object_;
  while(last_plotted_){
    last_plotted_->object->draw();
    last_plotted_=last_plotted_->next;
  }
  last_plotted_=last_object_;
  fl_pop_clip();

  axis=last_axis_;
  while(axis){
    if((axis->grid_visible()&CA_FRONT)&&(axis->visible()||(axis->grid_visible()&CA_ALWAYS_VISIBLE)))
      axis->draw_grid();
    axis=axis->previous_axis_;
  }


  if (damage_&FL_DAMAGE_ALL)
    draw_label();
};



void Ca_Canvas::add_object(Ca_Object_ * object){
  last_object_=last_object_->next=new Ca_ObjectChain();
  last_object_->object=object;
};

void Ca_Canvas::clear(){
  while(first_object_)
    delete first_object_->object;
  damage(CA_DAMAGE_ALL);
};

Ca_Canvas::Ca_Canvas(int x, int y, int w, int h, const char *label)
:Fl_Box(x,y,w,h,label),
last_axis_(0),border_(CANVAS_BORDER), current_x_(0), current_y_(0),
first_object_(0),last_object_(0),last_plotted_(0), dx_(0), dy_(0), dw_(0), dh_(0)
{

  current(this);
};

void Ca_Canvas::border(int border){
  border_=border;
  damage(CA_DAMAGE_ALL);
  Ca_Axis_ *axis=last_axis_;
  while(axis){
    axis->damage(CA_DAMAGE_ALL);
    axis=axis->previous_axis_;
  }
};

Ca_Canvas::~Ca_Canvas(){
  clear();
  Ca_Axis_ *axis=last_axis_;
  while(axis){
    last_axis_=axis->previous_axis_;
    axis->canvas_=0;
    axis = last_axis_;
  }
};





////////////////////////  Ca_Object //////////////////////

Ca_Object_::Ca_Object_(Ca_Canvas * canvas)
:canvas_(canvas)
{
  if(!canvas_)
    canvas_=Ca_Canvas::current();
  Ca_ObjectChain *objectchain=new(Ca_ObjectChain);
  objectchain->object=this;
  objectchain->next=0;
  if(canvas_->last_object_)
    canvas_->last_object_=canvas_->last_object_->next=objectchain;
  else
    canvas_->last_object_=canvas_->first_object_=objectchain;
  x_axis_=canvas_->current_x();
  y_axis_=canvas_->current_y();
  canvas_->damage(CA_DAMAGE_ADD);
};


Ca_Object_::~Ca_Object_(){
  Ca_ObjectChain * objectchain=canvas_->first_object_;
  Ca_ObjectChain * previouschain=0;
  while(objectchain->object!=this){
    previouschain=objectchain;
    objectchain=objectchain->next;
  }
  if(previouschain)
    previouschain->next=objectchain->next;
  else
    canvas_->first_object_=objectchain->next;
  if(canvas_->last_object_==objectchain)
    canvas_->last_object_=previouschain;
  delete objectchain;
  canvas_->last_plotted_=0;
  canvas_->damage(CA_DAMAGE_ALL);
};



/////////////////////////   Ca_Point    //////////////////////////////////////////////////////

void Ca_Point::draw(){
  fl_color(color);
  double s,t;
  double _x=x_axis_->position(x);
  double _y=y_axis_->position(y);
  switch(style & CA_POINT_STYLE){
  case CA_NO_POINT:
    break;
  case CA_SIMPLE:
    ca_point(_x,_y);
    break;
  case CA_ROUND:
    ca_filled_circle(_x,_y,size);
    //ca_pie(_x-size,_y-size,2*size,2*size,0,360);
    if (style & CA_BORDER){
      fl_color(border_color);
      fl_line_style(0,border_width);
      fl_begin_loop();
      fl_circle(_x,_y,size);
      fl_end_loop();
      fl_line_style(0,0);
    }
    break;
  case CA_SQUARE:
    ca_rectf(_x-size,_y-size,2*size,2*size);
    if (style & CA_BORDER){
      fl_color(border_color);
      fl_line_style(0,border_width);
      ca_rect(_x-size,_y-size,2*size,2*size);
      fl_line_style(0,0);
    }
    break;
  case CA_UP_TRIANGLE:
    s=(int)(1.12*size+0.5);
    t=_y+size/3;
    ca_polygon(_x,t-2*size,_x-s,t,_x+s,t);
    if (style & CA_BORDER){
      fl_color(border_color);
      fl_line_style(0,border_width);
      ca_loop(_x,t-2*size,_x-s,t,_x+s,t);
      fl_line_style(0,0);
    }
    break;
  case CA_DOWN_TRIANGLE:
    s=(int)(1.12*size+0.5);
    t=_y-size/3;
    ca_polygon(_x,t+2*size,_x-s,t,_x+s,t);
    if (style & CA_BORDER){
      fl_color(border_color);
      fl_line_style(0,border_width);
      ca_loop(_x,t+2*size,_x-s,t,_x+s,t);
      fl_line_style(0,0);
    }
    break;
  case CA_DIAMOND:
    s=(int)(1.3*size+0.5);
    ca_polygon(_x,_y-s,_x-s,_y,_x,_y+s,_x+s,_y);
    if (style & CA_BORDER){
      fl_color(border_color);
      fl_line_style(0,border_width);
      ca_loop(_x,_y-s,_x-s,_y,_x,_y+s,_x+s,_y);
      fl_line_style(0,0);
    }
    break;
  }
};

Ca_Point::Ca_Point(double _x, double _y, Fl_Color _color, int _style, int _size, Fl_Color _border_color, int _border_width)
:Ca_Object_(0),
x(_x),
y(_y),
style(_style),
size(_size),
color(_color),
border_color(_border_color),
border_width(_border_width)

{
};

////////////////////////////  Ca_LinePoint  ////////////////////////////////////////////////////////

void Ca_LinePoint::draw(){
  Ca_Point::draw();
  if(previous){
    fl_color(color);
    fl_line_style(0,line_width);
    fl_begin_line();
    fl_vertex(previous->x_axis_->position(previous->x),previous->y_axis_->position(previous->y));
    fl_vertex(x_axis_->position(x),y_axis_->position(y));
    fl_end_line();
    fl_line_style(0,0);
  }

};

Ca_LinePoint::Ca_LinePoint( Ca_LinePoint *_previous, double _x, double _y,int _line_width, Fl_Color color, int style, int size, Fl_Color border_color, int _border_width):Ca_Point(_x, _y, color, style, size, border_color, _border_width){
  previous=_previous;
  line_width=_line_width;
};

Ca_LinePoint::Ca_LinePoint(Ca_LinePoint *_previous, double _x, double _y)
:Ca_Point(_x, _y, previous->color, previous->style, previous->size,previous->border_color, previous->border_width),
previous(_previous)
{
  if(_previous)
    line_width=_previous->line_width;
  else
    line_width=0;
};


////////////////////////////  Ca_PolyLine  ////////////////////////////////////////////////////////


Ca_PolyLine::Ca_PolyLine(Ca_PolyLine *_previous, double _x, double _y,int _line_style, int _line_width, Fl_Color color, int style, int size, Fl_Color border_color,int _border_width)
:Ca_LinePoint(_previous, _x, _y, _line_width, color,  style, size, border_color, _border_width),
line_style(_line_style)
{
  next=0;
  if(_previous) _previous->next=this;
  canvas_->damage(CA_DAMAGE_ALL);
};

Ca_PolyLine::Ca_PolyLine(Ca_PolyLine *_previous, double _x, double _y):Ca_LinePoint(_previous,x,y){
  next=0;
  if(_previous){
    line_style=_previous->line_style;
    _previous->next=this;
  }
  canvas_->damage(CA_DAMAGE_ALL);
};

void Ca_PolyLine::draw(){
  Ca_Point::draw();
  if(next) return;
  Ca_PolyLine * temp;
  int c=color;
  int style=line_style;
  int size=line_width;
  fl_color(c);
  fl_line_style(style,size);
  fl_begin_line();
  fl_vertex(x_axis_->position(x),y_axis_->position(y));
  temp=(Ca_PolyLine *)previous;
  while(temp){
    fl_vertex(temp->x_axis_->position(temp->x),temp->y_axis_->position(temp->y));
    if((temp->line_style != style)||(temp->color!=c)||(temp->line_width!=size)){
      fl_end_line();
      c=temp->color;
      style=temp->line_style;
      size=temp->line_width;
      fl_color(c);
      fl_line_style(style,size);
      fl_begin_line();
      fl_vertex(temp->x_axis_->position(x),temp->y_axis_->position(y));
    }
    temp=(Ca_PolyLine *)(temp->previous);
  }
  fl_end_line();
  fl_line_style(0,0);
};



Ca_Line::Ca_Line(int _n, double *_data, double *_data_2, int _line_style, int _line_width, Fl_Color color,  int style, int size, Fl_Color border_color, int border_width)
:Ca_Point(0, 0, color, style, size, border_color, border_width),
line_style(_line_style),
line_width(_line_width),
n(_n),
data(_data),
data_2(_data_2)
{};

Ca_Line::Ca_Line(int _n, double *_data, int _line_style, int _line_width, Fl_Color color, int style, int size, Fl_Color border_color, int border_width)
:Ca_Point(0, 0, color, style, size, border_color, border_width),
line_style(_line_style),
line_width(_line_width),
n(_n),
data(_data),
data_2(0)
{};

void Ca_Line::draw(){
  fl_color(color);
  fl_line_style(line_style,line_width);
  fl_begin_line();
  int i;
  if(data_2){
    for(i=0;i<n;i++)
      fl_vertex(x_axis_->position(data[i]),y_axis_->position(data_2[i]));
    fl_end_line();
    fl_line_style(0,0);
    for(i=0;i<n;i++){
      x=data[i];
      y=data_2[i];
      Ca_Point::draw();
    }
  }else{
    for(i=0;i<n;i++)
      fl_vertex(x_axis_->position(data[2*i]),y_axis_->position(data[2*i+1]));
    fl_end_line();
    for(i=0;i<n;i++){
      x=data[2*i];
      y=data[2*i+1];
      Ca_Point::draw();
    }
  }
  fl_line_style(0,0);

};







void Ca_Text::draw(){
  uchar align_=align;
  double X,Y,W,H;
  double X1,Y1;

  X = x_axis_->position(x1);
  X1 = x_axis_->position(x2);
  if(X1>X)
    W=X1-X;
  else{
    W=X-X1;
    X=X1;
  }
  Y = y_axis_->position(y1);
  Y1 = y_axis_->position(y2);
  if(Y1>Y)
    H = Y1-Y;
  else{
    H = Y-Y1;
    Y = Y1;
  }
  fl_color(label_color);
  fl_font(label_font,label_size);
  ca_text(label,X,Y,W,H,(Fl_Align)align_);
};


Ca_Text::Ca_Text(double _x1, double _x2, double _y1, double _y2, const char *_label, uchar _align, Fl_Font _label_font, int _label_size, Fl_Color _label_color)
:Ca_Object_(0),
x1(_x1), x2(_x2), y1(_y1), y2(_y2),
label(_label),
align(_align),
label_color(_label_color),
label_font(_label_font),
label_size(_label_size)
{};

Ca_Text::Ca_Text(double x, double y,char *_label, uchar _align, Fl_Font _label_font, int _label_size, Fl_Color _label_color)
:Ca_Object_(0),
x1(x), x2(x), y1(y), y2(y),
label(_label),
align(_align),
label_color(_label_color),
label_font(_label_font),
label_size(_label_size)
{};




void Ca_Bar::draw(){
  uchar align_=align;
  double X,Y,W,H;
  double X1,Y1;

  X = x_axis_->position(x1);
  X1 = x_axis_->position(x2);
  if(X1>X)
    W=X1-X;
  else{
    W=X-X1;
    X=X1;
  }
  Y = y_axis_->position(y1);
  Y1 = y_axis_->position(y2);
  if(Y1>Y)
    H = Y1-Y;
  else{
    H = Y-Y1;
    Y = Y1;
  }

  fl_color(color);
  ca_rectf(X,Y,W,H);
  if(border_width>=0){
    fl_color(border_color);
    fl_line_style(FL_SOLID|FL_CAP_SQUARE, border_width);
    ca_rect(X,Y,W,H);
  }
  fl_line_style(0,0);

  if(!(align_&15)||(align_&FL_ALIGN_INSIDE)){
    X += border_width/2;
    W -= border_width;
    Y += border_width/2;
    H -= border_width;
  }else{
    X -= border_width/2;
    W += border_width;
    Y -= border_width/2;
    H += border_width;
    if (align_ & FL_ALIGN_TOP){
      align_ ^= (FL_ALIGN_BOTTOM|FL_ALIGN_TOP);
      H=Y;
      Y=canvas_->y()+Fl::box_dy(canvas_->box());
      H-=Y;
    }else if(align_ & FL_ALIGN_BOTTOM){
      align_^=(FL_ALIGN_BOTTOM|FL_ALIGN_TOP);
      Y=Y+H;
      H=canvas_->y()+canvas_->h()+Fl::box_dy(canvas_->box())-Fl::box_dh(canvas_->box())-Y;
    }else if(align_ & FL_ALIGN_LEFT){
      align_^=(FL_ALIGN_LEFT|FL_ALIGN_RIGHT);
      W=X;
      X=canvas_->x()+Fl::box_dx(canvas_->box());
      W=W-X-3;
    }else if(align_ & FL_ALIGN_RIGHT){
      align_ ^=(FL_ALIGN_LEFT|FL_ALIGN_RIGHT);
      X=X+W+3;
      W=canvas_->x()+Fl::box_dx(canvas_->box())+canvas_->w()-Fl::box_dw(canvas_->box())-X;
    }
  }
  fl_color(label_color);
  fl_font(label_font,label_size);
  ca_text(label,X,Y,W,H,(Fl_Align)align_);
};







Ca_Bar::Ca_Bar(double _x1, double _x2, double _y1, double _y2, Fl_Color _color,  Fl_Color _border_color, int _border_width,  const char *_label, uchar _align, Fl_Font _label_font, int _label_size, Fl_Color _label_color)
:Ca_Text(_x1, _x2, _y1, _y2, _label, _align, _label_font, _label_size, _label_color),
color(_color),
border_color(_border_color),
border_width(_border_width)
{
};
