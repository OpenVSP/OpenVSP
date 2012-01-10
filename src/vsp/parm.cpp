//******************************************************************************
//
//   Parameter Class
//
//
//   J.R. Gloudemans - 10/16/93
//   Sterling Software
//
//
//******************************************************************************


#include "parm.h"
#include "geom.h"
#include "gui_device.h"
#include "scriptMgr.h"
#include "parmLinkMgr.h"

//==== Constructor =====//
Parm::Parm() :
  name( Stringc("Default") ),
  group_name( Stringc("Design") ),
  script_text( Stringc("") ),
  value_type( VALUE_DOUBLE ),
  value( 0.0 ),
  upper_bound( 1.0e12 ),
  lower_bound( -1.0e12 ),
  active_flag( TRUE ),
  update_group( 0 )
{
  geom_ptr = NULL;
  base_geom = NULL;
  link_update_flag = false;
  linked_flag = false;

}

//==== Destructor =====//
Parm::~Parm()
{
  clear_all_devs();
//  parmLinkMgrPtr->RemoveParmReferences( this );
}

////==== Copy Operator ====//
//Parm& Parm::operator = ( Parm const &other )
//{
//	if ( this != &other )
//	{
//		*this = other;
//	}
//	return *this;
//}


//==== Clear All Devices =====//
void Parm::clear_all_devs()
{
  for ( int i = 0 ; i < (int)devs.size() ; i++)
  {
    devs[i]->remove_parm_ptr();
  }
}

void Parm::clear_dev_list()
{
	devs.clear();
}

//==== Initialize Parm ====//
void Parm::initialize(GeomBase* geom_ptr_in, const Stringc& name_in, double val )
{
//  parmLinkMgrPtr->UnRegister( this );
  geom_ptr = geom_ptr_in;
  base_geom = geom_ptr_in;
  name = name_in;
  value = val;
//  parmLinkMgrPtr->Register( this );
}

//==== Initialize Parm ====//
void Parm::initialize(GeomBase* geom_ptr_in, int grp_in, const Stringc& name_in, double val )
{
  update_group = grp_in;
  initialize( geom_ptr_in, name_in, val );
}

//void Parm::initialize(GeomBase* geom_ptr_in, const Stringc& name_in, double val, const Stringc& group_name_in )
//{
//	initialize( geom_ptr_in, geom_ptr_in, name_in, val, group_name_in );
//  //parmLinkMgrPtr->UnRegister( this );
//  //geom_ptr = geom_ptr_in;
//  //base_geom = geom_ptr_in;
//  //name = name_in;
//  //value = val;
//  //group_name = group_name_in;
//  //parmLinkMgrPtr->Register( this );
//}
//
//void Parm::initialize(GeomBase* geom_ptr_in, int grp_in, const Stringc& name_in, double val, const Stringc& group_name_in )
//{
//  update_group = grp_in;
//  initialize( geom_ptr_in, name_in, val, group_name_in );
//}
//
//void Parm::initialize(GeomBase* geom_ptr_in, GeomBase* base_geom_in, const Stringc& name_in, double val, const Stringc& group_name_in )
//{
//  parmLinkMgrPtr->UnRegister( this );
//  geom_ptr = geom_ptr_in;
//  base_geom = base_geom_in;
//  name = name_in;
//  value = val;
//  group_name = group_name_in;
//  parmLinkMgrPtr->Register( this );
//}
//
//void Parm::initialize(GeomBase* geom_ptr_in, GeomBase* base_geom_in, int grp_in, const Stringc& name_in, double val, const Stringc& group_name_in )
//{
//  update_group = grp_in;
//  initialize( geom_ptr_in, base_geom_in, name_in, val, group_name_in );
//}


//==== Set Active Flag On ====//
void Parm::set_active_flag_on()
{
  int i;
  active_flag = TRUE;

  //==== Update Linked Parms ====//
  for ( i = 0 ; i < (int)devs.size() ; i++)
  {
    devs[i]->activate();
  }
}

//==== Set Active Flag On ====//
void Parm::set_active_flag_off()
{
  int i;
  active_flag = FALSE;

  //==== Update Linked Parms ====//
  for (  i = 0 ; i < (int)devs.size() ; i++)
  {
    devs[i]->deactivate();
  }
}

//==== Set Update Geom Ptr ====//
void Parm::set_geom(GeomBase* geom_ptr_in)
{
//  parmLinkMgrPtr->UnRegister( this );
  geom_ptr = geom_ptr_in;
//  parmLinkMgrPtr->Register( this );
}

//==== Set Base Geom Ptr ====//
void Parm::set_base_geom(GeomBase* geom_ptr_in)
{
//  parmLinkMgrPtr->UnRegister( this );
  base_geom = geom_ptr_in;
//  parmLinkMgrPtr->Register( this );
}

//==== Set Value ====//
double Parm::set(double val_in)
{
  int i;

  //==== Check Bounds Violation ====//
  if ( val_in < lower_bound )
    value = lower_bound;
  else if ( val_in > upper_bound )
    value = upper_bound;
  else
    value = val_in;

  //==== Update All Devices ====//
  for ( i = 0 ; i < (int)devs.size() ; i++)
  {
    devs[i]->update();
  }
	
  parmLinkMgrPtr->ParmChanged( this, true );

  return(value);
}

double Parm::set_from_link( double val_in )
{
	int i;

	//==== Check Bounds Violation ====//
	if ( val_in < lower_bound )			value = lower_bound;
	else if ( val_in > upper_bound )	value = upper_bound;
	else								value = val_in;

	//==== Update All Devices ====//
	for ( i = 0 ; i < (int)devs.size() ; i++)
		devs[i]->update();
	
	parmLinkMgrPtr->ParmChanged( this, false);

	return(value);
}

//==== Set Value ====//
double Parm::set_from_device(Gui_device* dev_in, double val_in, int print)
{
  int i;

  //==== Check Bounds Violation ====//
  if ( val_in < lower_bound )
    value = lower_bound;
  else if ( val_in > upper_bound )
    value = upper_bound;
  else
    value = val_in;

  //==== Update All Devices ====//
  for ( i = 0 ; i < (int)devs.size() ; i++)
  {
      if ( devs[i] != dev_in )
        devs[i]->update_from_device();
  }

  //==== Trigger Update All Affected Parm_Groups ====//
  geom_ptr->parm_changed(this);

  //==== Tell Screen That Parm Changed ====//
  dev_in->trigger_screen(this);


  if (print && script_text.get_length())
  {
	  if (value_type == VALUE_INT)
		scriptMgr->addLineInt(script_text, value);
	  if (value_type == VALUE_DOUBLE)
		scriptMgr->addLineDouble(script_text, value);
  }

	parmLinkMgrPtr->ParmChanged( this, true );

  return(value);
}

//==== Set Upper Bounds ====//
void Parm::set_upper(double val_in)
{
  upper_bound = val_in;
  if (upper_bound < lower_bound)
    lower_bound = upper_bound;

  if (value > upper_bound)
    {
      value = set(upper_bound);

    }
}

//==== Set Lower Bounds ====//
void Parm::set_lower(double val_in)
{
  lower_bound = val_in;
  if (lower_bound > upper_bound)
    upper_bound = lower_bound;

  if (value < lower_bound)
    {
      value = set(lower_bound);

    }
}

//==== Set Upper and Lower Bounds ====//
void Parm::set_lower_upper(double low_val_in, double up_val_in )
{
  lower_bound = low_val_in;
  upper_bound = up_val_in;
  if (lower_bound > upper_bound)
    upper_bound = lower_bound;

  if (value < lower_bound)
    {
      value = set(lower_bound);

    }
  else if  ( value > upper_bound)
    {
      value = set(upper_bound);
    }
}

//==== Add Device To List ====//
void Parm::add_device(Gui_device* new_dev)
{
  devs.push_back(new_dev);
}

//==== Del Device From List ====//
void Parm::del_device(Gui_device* old_dev)
{
  int j;
 
  vector<Gui_device*> saveDevs;

  for ( j = 0 ; j < (int)devs.size() ; j++) 
  {
	if ( devs[j] != old_dev )
		saveDevs.push_back( devs[j] );
  }

  devs = saveDevs;

}

//==== Write To File ====//
void Parm::write(FILE* file_id)
{
  fprintf(file_id, "%f                  %s\n",value,name());
}

//==== Write To File ====//
void Parm::read(FILE* file_id)
{
  char buff[255];
  fscanf(file_id, "%lf",&value);                fgets(buff, 80, file_id);
}







