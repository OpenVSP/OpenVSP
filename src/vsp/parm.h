//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//

//******************************************************************************
//
//   Parameter Base Class
//
//   J.R. Gloudemans - 6/26/93
//
//******************************************************************************

#ifndef PARM_H
#define PARM_H

#pragma warning(disable:4786)	//jrg windows??
#pragma warning(disable:4251)   // dll warning
#include <math.h>
#include <stdio.h>

#include "stringc.h"

#include <vector>				//jrg windows??	
using namespace std;			//jrg windows??

#define XDDM_VAR 0
#define XDDM_CONST 1

class Gui_device;
class GeomBase;

class VSPDLL Parm
{
protected:
     Stringc name;
	 Stringc script_text;
	 Stringc group_name;

	 int value_type;

     double value;
     double upper_bound;
     double lower_bound;

     int active_flag;

	 bool link_update_flag;
	 bool linked_flag;

     int update_group;
     GeomBase* geom_ptr;
	 GeomBase* base_geom;

     vector<Gui_device*> devs;

public:
 
  	 enum { VALUE_DOUBLE = 0, VALUE_INT };

	 Parm();
     virtual ~Parm();

	 //Parm& operator=( Parm const &other );

     void clear_all_devs();

     void initialize(GeomBase* geom_ptr_in, const Stringc& name_in, double val );
     void initialize(GeomBase* geom_ptr_in, int update_grp, const Stringc& name_in, double val );

	 //void initialize(GeomBase* geom_ptr_in, const Stringc& name_in, double val, const Stringc& group_name_in );
  //   void initialize(GeomBase* geom_ptr_in, int update_grp, const Stringc& name_in, double val, const Stringc& group_name_in  );

	 //void initialize(GeomBase* geom_ptr_in, GeomBase* base_geom_in, const Stringc& name_in, double val, const Stringc& group_name_in );
  //   void initialize(GeomBase* geom_ptr_in, GeomBase* base_geom_in, int update_grp, const Stringc& name_in, double val, const Stringc& group_name_in  );

     void set_active_flag_on();
     void set_active_flag_off();
     int  get_active_flag()                     { return(active_flag); }
     void activate()                            { set_active_flag_on(); }
     void deactivate()                          { set_active_flag_off(); }

     void set_geom(GeomBase* pg_ptr_in);
     GeomBase* get_geom()                           { return(geom_ptr); }
 
	 void set_base_geom(GeomBase* base_geom_in );
	 GeomBase* get_geom_base()						{ return base_geom; }
	 
	 void set_update_grp(int update_grp)        { update_group = update_grp; }
     int get_update_grp()                       { return(update_group); }

//     void set_name(const Stringc& name_in)      { name = name_in; }
     Stringc get_name()	const						{ return(name); }
	 void set_group_name( const Stringc& n )		{ group_name = n; }
	 Stringc get_group_name()						{ return(group_name); }
     void set_script(const Stringc& script_in, int type)  { script_text = script_in; value_type = type;}
     Stringc get_script()						{ return(script_text); }

     void init_val(double val_in)               { value = val_in; }

     double set(double val_in);
	 double set_from_link(double val_in);

     double operator= (double val_in)           { return( set(val_in) ); }
//     double set_from_device(Gui_device* dev_in, double val_in);
     double set_from_device(Gui_device* dev_in, double val_in, int print);
     double operator () ()                      { return(value); }
//	 operator const double()					{ return(value); }		// Implict double cast

	 double operator*=(double rhs)				{ value *= rhs;  return value; }
	 double operator+=(double rhs)				{ value += rhs;  return value; }
	 double operator-=(double rhs)				{ value -= rhs;  return value; }

     double get()                               { return(value); }
     int iget()                                 { return((int)(value + 0.5)); }

     void   set_upper(double val_in);
     double get_upper()                         { return(upper_bound); }
     void   set_lower(double val_in);
     double get_lower()                         { return(lower_bound); }
     void   set_lower_upper(double v1, double v2);

	 void set_link_update_flag( bool f )		{ link_update_flag = f; }
	 bool get_link_update_flag()				{ return link_update_flag; }
	 void set_linked_flag( bool f )				{ linked_flag = f; }
	 bool get_linked_flag()						{ return linked_flag; }

     void add_device(Gui_device* dev_in);
     void del_device(Gui_device* dev_in);
	 void clear_dev_list();

     void write(FILE* file_id);
     void read(FILE* file_id);


};


class VSPDLL ParmHolder
{
protected:
	Parm* m_ParmPtr;
	int m_DesType;

public:

	ParmHolder()								{ m_ParmPtr = NULL; m_DesType = XDDM_VAR; }
	~ParmHolder()								{ }

	void setParm( Parm* p )						{ m_ParmPtr = p; }
	Parm* getParm()								{ return m_ParmPtr; }

	void setDesType( int type )					{ m_DesType = type; }
	int getDesType()							{ return m_DesType; }
};


#endif





