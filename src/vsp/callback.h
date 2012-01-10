//******************************************************************************
//    
//  Call Back Class
//   
//   J.R. Gloudemans - 4/21/03
//******************************************************************************

#ifndef CALLBACK_H
#define CALLBACK_H

#include <FL/Fl.H>
#include <FL/Fl_Widget.H>

#define CB_TEMPLATE T, CB_func_type

template < class T, class CB_func_type >

class Callback
{
  T* curr_screen;
  CB_func_type cb_ptr;

  int int_val;

public:
  Fl_Widget* widget;

  Callback( T* screen_in, CB_func_type cb_ptr_in);
  Callback( T* screen_in, CB_func_type cb_ptr_in, int int_val_in);
  Callback( T* screen_in, CB_func_type cb_ptr_in, int int_val_in, Fl_Widget* obj_in);
   ~Callback();

  void execute();

};


template < class T, class CB_func_type >
//===== Constructor  =====//
Callback<CB_TEMPLATE>::Callback( T* screen_in, CB_func_type cb_ptr_in)
{
  curr_screen = screen_in;
  cb_ptr = cb_ptr_in;
  widget = 0;
  int_val = 0;
}

template < class T, class CB_func_type >
//===== Constructor  =====//
Callback<CB_TEMPLATE>::Callback( T* screen_in, CB_func_type cb_ptr_in, int int_val_in)
{
  curr_screen = screen_in;
  cb_ptr = cb_ptr_in;
  widget = 0;
  int_val = int_val_in;
}

template < class T, class CB_func_type >
//===== Constructor  =====//
Callback<CB_TEMPLATE>::Callback( T* screen_in, CB_func_type cb_ptr_in, int int_val_in, Fl_Widget* obj_in)
{
  curr_screen = screen_in;
  cb_ptr = cb_ptr_in;
  widget = obj_in;
  int_val = int_val_in;
}

template < class T, class CB_func_type >
//===== Constructor  =====//
Callback<CB_TEMPLATE>::~Callback()
{

}

template < class T, class CB_func_type >
//===== Constructor  =====//
void Callback<CB_TEMPLATE>::execute()
{
  (curr_screen->*cb_ptr)(forms_obj, int_val);
}

#endif
