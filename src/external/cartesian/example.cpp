// Cartesian v 1.1
//
// Copyright 2000-2008 by Roman Kantor.
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

#ifdef USE_ROTATED_TEXT
#include <Fl_Rotated_Text/Fl_Rotated_Text.cxx>
#endif


#include <math.h>
#include <FL/Fl.H>

#include <FL/Fl_Overlay_Window.H>
#include <FL/Fl_Light_Button.H>
#include "Cartesian.H"
#include <FL/fl_draw.H>






#ifdef FL_DEVICE // this is for printing using Fl_Device patch

#include <FL/Fl_Printer.H>
#include <FL/Fl_PostScript.H>
#include <FL/fl_file_chooser.H>


void print(Fl_Widget *, void *w) {
    Fl_Widget * g = (Fl_Widget *)w;
    char * filename = fl_file_chooser("Print to file...","*.ps","*.ps");
    if(!filename) return;
    FILE * f = fopen(filename,"w");
    if(!f) return;
    Fl_Printer * p = new Fl_PostScript(f, 3, Fl_Printer::A4, Fl_Printer::PORTRAIT);
    p->page();
    p->fit(g, FL_ALIGN_CENTER);
    p->draw(g);
    delete p;
};

void print2(Fl_Widget *, void *w) {
    Fl_Widget * g = (Fl_Widget *)w;
    Fl_Printer * p = fl_gdi_printer_chooser();
    if(!p) return;
    p->page();
    p->fit(g, FL_ALIGN_CENTER);
    p->draw(g);
    delete p;

};

#endif



const double PI=3.1416;
const double MIN_FREQ=90;
const double MAX_FREQ=25000;
const double FREQ_COEF=1.15;

Ca_X_Axis*  frequency;
Ca_Y_Axis* current;
Ca_Y_Axis* power;
Ca_Y_Axis* phase;
Fl_Light_Button *logarithmic;
Fl_Light_Button *reversed;

double R=600;
double L=0.16;
double C=1.6e-7;
double U=1;

Ca_Canvas *canvas;

void type_callback(Fl_Widget *, void *){
    power->scale((CA_LOG*logarithmic->value())|CA_REV*reversed->value());
}

void  next_freq(void *){
    static double f=MIN_FREQ;
    static Ca_LinePoint *P_I=0;
    static Ca_PolyLine *P_P=0;

    double XL=2*PI*f*L;
    double XC=1/(2*PI*f*C);
    double I,P,fi;

    I=U/sqrt(R*R+(XL-XC)*(XL-XC));
    P=I*I*R;
    fi=atan((XL-XC)/R);



    phase->current();                                //setting coordinate
    new Ca_Point(f,fi,FL_YELLOW,CA_DIAMOND|CA_BORDER);current->current();                                //setting coordinate

    current->rescale(CA_WHEN_MAX,I*1100);            //different rescalling for max and min just to get some extra gap 
    current->rescale(CA_WHEN_MIN,I*1000);            //above maximum
    P_I=new Ca_LinePoint(P_I,f,I*1000,0,FL_BLUE,CA_ROUND|CA_BORDER);

    power->current();                                //setting coordinate
    power->rescale(CA_WHEN_MIN|CA_WHEN_MAX,P*1000);
    P_P=new Ca_PolyLine(P_P,f,P*1000,FL_DASHDOT,2,FL_RED,CA_NO_POINT);


    f =f* FREQ_COEF;
    if(f<=MAX_FREQ)
        Fl::add_timeout(.1,next_freq);
    else{
        power->current();
        //new Ca_Text(1000,1,"Text\ntest!");
    }
};




int main(int argc, char ** argv) {

#ifdef USE_ROTATED_TEXT
    Fl_Rotated_Text current_label("Current [mA]",FL_HELVETICA,14,0, 1);
    Fl_Rotated_Text phase_label("Phase [rad]",FL_HELVETICA,14,0, 3);
#endif


    Fl_Double_Window *w= new Fl_Double_Window(580, 380, "Cartesian graphics example");
    w->size_range(450,250);


    Fl_Group *c =new Fl_Group(0, 35, 580, 345 );

    c->box(FL_DOWN_BOX);
    c->align(FL_ALIGN_TOP|FL_ALIGN_INSIDE);

    canvas = new Ca_Canvas(180, 75, 300, 225, "RLC resonance circuit");
    canvas->box(FL_DOWN_BOX);
    canvas->color(7);
    canvas->align(FL_ALIGN_TOP);
    Fl_Group::current()->resizable(canvas);
    // w->resizable(canvas);
    canvas->border(15);

    frequency = new Ca_X_Axis(180, 305, 300, 30, "Frequency [Hz]");
    frequency->labelsize(14);
    frequency->align(FL_ALIGN_BOTTOM);
    frequency->scale(CA_LOG);
    frequency->minimum(MIN_FREQ);
    frequency->maximum(MAX_FREQ);
    frequency->label_format("%g");
    frequency->minor_grid_color(fl_gray_ramp(20));
    frequency->major_grid_color(fl_gray_ramp(15));
    frequency->label_grid_color(fl_gray_ramp(10));
    frequency->grid_visible(CA_MINOR_GRID|CA_MAJOR_GRID|CA_LABEL_GRID);
    frequency->major_step(10);
    frequency->label_step(10);
    frequency->axis_color(FL_BLACK);
    frequency->axis_align(CA_BOTTOM|CA_LINE);


    current = new Ca_Y_Axis(137, 70, 43, 235 /*, "I [mA]" */);
#ifdef USE_ROTATED_TEXT
    current->image(&current_label);
    current->align(FL_ALIGN_LEFT);
#else
    current->label("Current [mA]");
    current->align(FL_ALIGN_RIGHT|FL_ALIGN_TOP);
#endif
    //current->align(FL_ALIGN_TOP_RIGHT);

    current->minor_grid_style(FL_DASH);
    current->axis_align(CA_LEFT);
    current->axis_color(FL_BLACK);




    power = new Ca_Y_Axis(10, 70, 75, 235, "P [mW]");
    power->box(FL_DOWN_BOX);
    power->align(FL_ALIGN_TOP);
    power->grid_visible(CA_MINOR_TICK|CA_MAJOR_TICK|CA_LABEL_GRID|CA_ALWAYS_VISIBLE);

    power->minor_grid_color(FL_RED);
    power->major_grid_color(FL_RED);
    power->label_grid_color(FL_RED);
    power->minimum(0.01);                    //setting beginning range
    power->maximum(1);


    phase = new Ca_Y_Axis(480, 70, 45, 235);
#ifdef USE_ROTATED_TEXT
    phase->image(&phase_label);
    phase->align(FL_ALIGN_RIGHT);
#else
    phase->label("Phase [rad]");
    phase->align(FL_ALIGN_LEFT|FL_ALIGN_TOP);
#endif

    phase->minimum(-PI);
    phase->maximum(PI);
    phase->axis_align(CA_RIGHT);
    phase->tick_interval(-PI/4);        //fixed ticks setting
    phase->major_step(2);
    phase->label_step(4);
    phase->label_format("%.2f");

    power->current();

    new Ca_Bar(7000, 12000, 0.0011, 0.9, FL_RED,  FL_BLACK, 4,  "Bar", FL_ALIGN_TOP, FL_HELVETICA);
    new Ca_Bar(5000, 10000, 0.0011, 0.4, FL_GREEN,  FL_BLACK, 4,  "Sec.\nbar", FL_ALIGN_TOP|FL_ALIGN_INSIDE, FL_HELVETICA);



    logarithmic=new Fl_Light_Button(10,310, 75, 25, "Log");
    logarithmic->callback(&type_callback);


    reversed= new Fl_Light_Button(10,340, 75, 25, "Rev");
    reversed->callback(&type_callback);
    c->end();

#ifdef FL_DEVICE
    Fl_Button *b2 = new Fl_Button(5,5, 90, 25, "Print to file");
    b2->callback(print,c);

    Fl_Button *b3 = new Fl_Button(105,5, 90, 25, "Print");
    b3->callback(print2,c);

#endif

    Fl_Group::current()->resizable(c);
    w->end();
    w->show(argc, argv);
    Fl::add_timeout(0,next_freq);
    Fl::run();
    return 0;
};

