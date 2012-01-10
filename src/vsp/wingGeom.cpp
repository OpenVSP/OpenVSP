//******************************************************************************
//
//    Multi Section Wing (Strake/Aft Ext)  Geometry Class
//
//
//   J.R. Gloudemans - 2/1/95
//   Sterling Software
//
//******************************************************************************

#include "wingGeom.h"
#include "af.h"
#include "aircraft.h"
#include "materialMgr.h"

#ifdef __APPLE__
#  include <OpenGL/gl.h>
#else
#  include <GL/gl.h>
#endif

#include "defines.h"

//==== Constructor =====//
WingGeom::WingGeom(Aircraft* aptr) : Geom(aptr)
{
	deflect_flag = 0;
	deflect_curve.init( 2 );
	twist_curve.init( 2 );
	deflect_curve.load_pnt(  0, vec3d(0.0, 0.0, 0.0));
	deflect_curve.load_pnt(  1, vec3d(1.0, 0.0, 0.0));
	twist_curve.load_pnt(    0, vec3d(0.0, 0.0, 0.0));
	twist_curve.load_pnt(    1, vec3d(1.0, 0.0, 0.0));

	type = WING_GEOM_TYPE;
	type_str = Stringc("wing");

	char name[255];
	sprintf( name, "Wing_%d", geomCnt ); 
	geomCnt++;
	name_str = Stringc(name);
	id_str = name_str;				//jrg fix

	curr_af_id = MW_ROOT_AF;
	root_foil = new Af(this);
	strake_foil = new Af(this);
	aft_foil = new Af(this);
	tip_foil = new Af(this);

	set_num_pnts_all_afs();

	define_parms();
	setSymCode(XZ_SYM);

	strake_aft_flag = TRUE;

	//===== Aerodynamic Data =====
	trimmed_span = 1.0;

	generate();
}

//==== Destructor =====//
WingGeom::~WingGeom()
{
  delete root_foil;
  delete strake_foil;
  delete aft_foil;
  delete tip_foil;
}

void WingGeom::copy( Geom* fromGeom )
{
	Geom::copy( fromGeom );						// Copy Base Stuff

	if ( fromGeom->getType() != WING_GEOM_TYPE )
		return;

	WingGeom* g = (WingGeom*)fromGeom;

	*root_foil = *(g->get_root_af_ptr());
	*strake_foil = *(g->get_strake_af_ptr());
	*aft_foil = *(g->get_aft_af_ptr());
	*tip_foil = *(g->get_tip_af_ptr());

	root_foil->set_geom( this );
	strake_foil->set_geom( this );
	aft_foil->set_geom( this );
	tip_foil->set_geom( this );

	set_driver( g->get_driver() );
	aspect.set( g->get_aspect()->get() );
	taper.set(  g->get_taper()->get() );
	area.set(   g->get_mw_area()->get() );
	span.set(  g->get_mw_span()->get() );
	root_chord.set(  g->get_root_chord()->get() );
	tip_chord.set(  g->get_tip_chord()->get() );
	
	sweep.set( g->get_sweep()->get() );
	sweep_loc.set( g->get_sweep_loc()->get() );
	old_sweep_loc = sweep_loc();

	twist_loc.set( g->get_twist_loc()->get() );
	in_twist.set( g->get_in_twist()->get() );
	in_dihed.set( g->get_in_dihed()->get() );
	mid_twist.set( g->get_mid_twist()->get() );
	mid_dihed.set( g->get_mid_dihed()->get() );
	out_twist.set( g->get_out_twist()->get() );
	out_dihed.set( g->get_out_dihed()->get() );

	strake_sweep.set( g->get_strake_sweep()->get() );
	strake_span_per.set( g->get_strake_span_per()->get() );
	aft_ext_sweep.set( g->get_aft_ext_sweep()->get() );
	aft_ext_span_per.set( g->get_aft_ext_span_per()->get() );

	total_area.set( g->get_total_area()->get() );

	defl_scale.set( g->get_defl_scale()->get() );
	twist_scale.set( g->get_twist_scale()->get() );

	deflect_flag = g->get_deflect_flag();
	deflect_file_name = g->get_deflect_file_name();
    deflect_name = g->get_deflect_file_name();

    deflect_curve = g->deflect_curve;
    twist_curve = g->twist_curve;

	strake_aft_flag = g->get_strake_aft_flag();

	mwing_surf.set_num_pnts( numPnts.iget() );

	compose_model_matrix();
	generate();

}


//==== Define  Parms ====//
void WingGeom::define_parms()
{

  mwing_surf.set_num_pnts( numPnts.iget() );
  mwing_surf.set_num_xsecs( 6 );

  numXsecs.deactivate();

  set_driver(AR_TR_A);

  aspect.initialize(this, UPD_MW_ALL, "Aspect_Ratio", 2.0);
  aspect.set_lower_upper(0.05, 100.0);

  taper.initialize(this, UPD_MW_ALL, "Taper_Ratio", 0.15);
  taper.set_lower_upper(0.001, 5.0);

  area.initialize(this, UPD_MW_ALL, "Area", 75.0);
  area.set_lower_upper(0.001, 1000000.0);

  span.initialize(this, UPD_MW_ALL, "Span", 30.0);
  span.set_lower_upper(0.001, 10000.0);

  tip_chord.initialize(this, UPD_MW_ALL, "Tip_Chord", 5.0);
  tip_chord.set_lower_upper(0.001, 10000.0);

  root_chord.initialize(this, UPD_MW_ALL, "Root_Chord", 10.0);
  root_chord.set_lower_upper(0.001, 10000.0);

  sweep.initialize(this, UPD_MW_ALL, "Sweep", 55.0);
  sweep.set_lower_upper(-85.0, 85.0);

  sweep_loc.initialize(this, UPD_MW_SWEEP_LOC, "Sweep_Loc", 0.0);
  sweep_loc.set_lower_upper(0.0, 1.0);
  old_sweep_loc = sweep_loc();

  twist_loc.initialize(this, UPD_MW_ALL, "Twist_Location", 0.0);
  twist_loc.set_lower_upper(0.0, 1.0);

  in_twist.initialize(this, UPD_MW_ALL, "Inboard_Twist", 0.0);
  in_twist.set_lower_upper(-45.0, 45.0);
  in_dihed.initialize(this, UPD_MW_ALL, "Inboard_Dihedral", 0.0);
  in_dihed.set_lower_upper(-45.0, 45.0);

  mid_twist.initialize(this, UPD_MW_ALL, "Midsection_Twist", 0.0);
  mid_twist.set_lower_upper(-45.0, 45.0);
  mid_dihed.initialize(this, UPD_MW_ALL, "Midsection_Dihedral", 0.0);
  mid_dihed.set_lower_upper(-45.0, 45.0);

  out_twist.initialize(this, UPD_MW_ALL, "Outboard_Twist", 0.0);
  out_twist.set_lower_upper(-45.0, 45.0);
  out_dihed.initialize(this, UPD_MW_ALL, "Outboard_Dihedral", 0.0);
  out_dihed.set_lower_upper(-45.0, 45.0);

  strake_sweep.initialize(this, UPD_MW_ALL, "Strake_Sweep", 65.0);
  strake_sweep.set_lower_upper(-85.0, 85.0);
  strake_span_per.initialize(this, UPD_MW_ALL, "Strake_Sweep", 0.3);
  strake_span_per.set_lower_upper(0.01, 0.99);

  aft_ext_sweep.initialize(this, UPD_MW_ALL, "Aft_Ext_Sweep", -10.0);
  aft_ext_sweep.set_lower_upper(-85.0, 85.0);
  aft_ext_span_per.initialize(this, UPD_MW_ALL, "Aft_Ext_Sweep", 0.5);
  aft_ext_span_per.set_lower_upper(0.01, 0.99);

  total_area.initialize(this, UPD_MW_ALL, "Total_Area", 0.0);
  total_area.deactivate();

  //==== Control Surface Parms ====//
  all_move_flag = FALSE;
  in_flap_type  = MW_NO_FLAP;
  out_flap_type = MW_NO_FLAP;
  in_slat_type  = MW_NO_SLAT;
  out_slat_type = MW_NO_SLAT;

  in_flap_span_in.initialize(this, UPD_MW_CONT_SURF, "Inboard_Flap_Span_In", 0.2);
  in_flap_span_in.set_lower_upper(0.0, 1.0);
  in_flap_span_out.initialize(this, UPD_MW_CONT_SURF, "Inboard_Flap_Span_Out", 0.8);
  in_flap_span_out.set_lower_upper(0.0, 1.0);
  in_flap_chord.initialize(this, UPD_MW_CONT_SURF, "Inboard_Flap_Chord", 0.2);
  in_flap_chord.set_lower_upper(0.0001, 0.9999);
  out_flap_span_in.initialize(this, UPD_MW_CONT_SURF, "Outboard_Flap_Span_In", 0.2);
  out_flap_span_in.set_lower_upper(0.0, 1.0);
  out_flap_span_out.initialize(this, UPD_MW_CONT_SURF, "Outboard_Flap_Span_Out", 0.8);
  out_flap_span_out.set_lower_upper(0.0, 1.0);
  out_flap_chord.initialize(this, UPD_MW_CONT_SURF, "Outboard_Flap_Chord", 0.2);
  out_flap_chord.set_lower_upper(0.0001, 0.9999);

  in_slat_span_in.initialize(this, UPD_MW_CONT_SURF, "Inboard_Slat_Span_In", 0.2);
  in_slat_span_in.set_lower_upper(0.0, 1.0);
  in_slat_span_out.initialize(this, UPD_MW_CONT_SURF, "Inboard_Slat_Span_Out", 0.8);
  in_slat_span_out.set_lower_upper(0.0, 1.0);
  in_slat_chord.initialize(this, UPD_MW_CONT_SURF, "Inboard_Slat_Chord", 0.2);
  in_slat_chord.set_lower_upper(0.0001, 0.9999);
  out_slat_span_in.initialize(this, UPD_MW_CONT_SURF, "Outboard_Slat_Span_In", 0.2);
  out_slat_span_in.set_lower_upper(0.0, 1.0);
  out_slat_span_out.initialize(this, UPD_MW_CONT_SURF, "Outboard_Slat_Span_Out", 0.8);
  out_slat_span_out.set_lower_upper(0.0, 1.0);
  out_slat_chord.initialize(this, UPD_MW_CONT_SURF, "Outboard_Slat_Chord", 0.2);
  out_slat_chord.set_lower_upper(0.0001, 0.9999);

  defl_scale.initialize(this, UPD_MW_ALL, "Deflection Scale", 1.0);
  defl_scale.set_lower_upper(0.0001, 10.0);
  twist_scale.initialize(this, UPD_MW_ALL, "Twist Scale", 1.0);
  twist_scale.set_lower_upper(0.0001, 10.0);

  //==== Restrict Num of Xsec to 6 =====//
  numXsecs.set( 6 );

  center.set_xyz( 0, 0, 0 );

}



//==== Generate Multi Section Wing Component ====//
void WingGeom::generate()
{
  set_depend_parms();

  generate_surf();

  update_bbox();
}

void WingGeom::computeCenter()
{
	//==== Set Rotation Center ====//
	center.set_x( origin()*root_chord()*scaleFactor() ); 
}

//==== Parm Has Changed ReGenerate Multi Section Wing Component ====//
void WingGeom::parm_changed(Parm* p)
{
	int up_group = p->get_update_grp();

	//==== Set Rotation Center ====//
	center.set_x( origin()*root_chord()*scaleFactor() ); 

	switch (up_group)
    {
		case UPD_XFORM:
		{
			if ( p == &xLoc || p == &yLoc || p == &zLoc || 
				 p == &xRot || p == &yRot || p == &zRot )
				updateAttach(1);
			else
				updateAttach(0);

			generate();
		}
		break;

		case UPD_NUM_PNT_XSEC:
		{
			set_num_pnts_all_afs();
			generate();
		}
		break;

		case AF_UPDATE_GROUP:
        {
          get_af_ptr()->generate_airfoil();
          get_af_ptr()->load_name();
          generate();
		  updateAttach(0);
        }
		break;

		case UPD_MW_ALL:
        {
          generate();
		  updateAttach(0);
        }
		break;

		case UPD_MW_CONT_SURF:
        {
          generate_flap_slat_surfs();
        }
		break;

		case UPD_MW_SWEEP_LOC:
        {
          sweep = get_sweep_at( sweep_loc() )*RAD_2_DEG;
          old_sweep_loc = sweep_loc();
        }
		break;

    }

	if ( p == &scaleFactor )	
	{
		scale();
		generate();
		updateAttach(0);
	}

	compose_model_matrix();

	//==== Tell Aircraft You Have Changed ====//
	airPtr->geomMod( this );

}


vec3d WingGeom::getAttachUVPos(double u, double v)
{
	vec3d pos;

	vec3d uvpnt = mwing_surf.comp_uv_pnt(u,v);
	pos = uvpnt.transform( model_mat );

	return pos;
}



void WingGeom::scale()
{
	double current_factor = scaleFactor()*(1.0/lastScaleFactor);

    int original_driver_group = get_driver();
    set_driver(AR_TR_A);

	area.set( area()*current_factor );

    //update display
    generate();

    //return driver to original group
    set_driver(original_driver_group);

	lastScaleFactor = scaleFactor();

}

void WingGeom::acceptScaleFactor()
{
	lastScaleFactor = 1.0;
	scaleFactor.set(1.0);
}

void WingGeom::resetScaleFactor()
{
	scaleFactor.set( 1.0 );
    scale();
	lastScaleFactor = 1.0;
		
	//==== Tell Aircraft You Have Changed ====//
	airPtr->geomMod( this );


}


//==== Get x location of root chord's origin from aircrafts origin ====
double WingGeom::get_x_loc_root_chord()
{
    return ( get_x_le_root() );
}

//==== Get y location of root chord's origin from aircrafts origin ====
double WingGeom::get_y_loc_root_chord()
{
	vec3d v = getTotalTranVec();
    return ( v.y());
}

//==== Get z location of root chord's origin from aircrafts origin ====
double WingGeom::get_z_loc_root_chord()
{
	vec3d v = getTotalTranVec();
    return ( v.z() );
}

//==== Get x location of strake chord's origin from aircrafts origin ====
double WingGeom::get_x_loc_strake_chord()
{
  double sp =  ( span() * strake_span_per() );  
  if ( getSymCode() == XZ_SYM ) // Check Sym Code
    sp *= 0.5;   
  
  double x = get_x_loc_root_chord() + sp * tan(get_strake_sweep()->get()*DEG_2_RAD);

  return x;
}

//==== Get y location of strake chord's origin from aircrafts origin ====
double WingGeom::get_y_loc_strake_chord()
{  
  vec3d v = getTotalTranVec();
  double y = v.y();

  vec3d rot = getTotalRotVec();
  
  double sp =  span();  
  if ( getSymCode() == XZ_SYM ) // Check Sym Code
    sp *= 0.5; 
    
  if ( strake_span_per() < aft_ext_span_per() )
  {
    y += sp*strake_span_per() * cos((in_dihed() + rot.x())*DEG_2_RAD);
  }
  else
  {
    y += sp*aft_ext_span_per() * cos((in_dihed() + rot.x())*DEG_2_RAD);
    y += sp*(strake_span_per() - aft_ext_span_per()) * cos((mid_dihed()+ rot.x())*DEG_2_RAD);
  }  

  return y;
}

//==== Get z location of strake chord's origin from aircrafts origin ====
double WingGeom::get_z_loc_strake_chord()
{
  vec3d v = getTotalTranVec();
  double z = v.z();

  vec3d rot = getTotalRotVec();

  double sp =   span();  
  if ( getSymCode() == XZ_SYM ) // Check Sym Code
    sp *= 0.5; 

  if ( strake_span_per() < aft_ext_span_per() )
  {
    z += sp*strake_span_per() * sin( (in_dihed()+ rot.x())*DEG_2_RAD );
  }
  else
  {
    z += sp*aft_ext_span_per() * sin( (in_dihed()+ rot.x())*DEG_2_RAD );
    z += sp*(strake_span_per() - aft_ext_span_per()) * sin( (mid_dihed()+ rot.x())*DEG_2_RAD );
  }  
    
  return z;
}

//==== Get x location of aft extension chord's origin from aircrafts origin ====
double WingGeom::get_x_loc_aft_chord()
{
  double sp =  ( span() * aft_ext_span_per() );  
  if ( getSymCode() == XZ_SYM ) // Check Sym Code
    sp *= 0.5;   

  vec3d v = getTotalTranVec();
  double x;
  if ( strake_span_per() < aft_ext_span_per() )
  {
    x = v.x() + sp * get_tan_sweep_at(0.0);
  }
  else
  {
    x = get_x_loc_root_chord() + sp * tan( get_strake_sweep()->get()*DEG_2_RAD );
  }
  
  return x;
}

//==== Get y location of aft extension chord's origin from aircrafts origin ====
double WingGeom::get_y_loc_aft_chord()
{
  vec3d v = getTotalTranVec();
  double y = v.y();
  vec3d rot = getTotalRotVec();
  
  double sp =  span();  
  if ( getSymCode() == XZ_SYM ) // Check Sym Code
    sp *= 0.5; 
    
  if ( strake_span_per() < aft_ext_span_per() )
  {
    y += sp*strake_span_per() * cos( (in_dihed() + rot.x())*DEG_2_RAD );
    y += sp*(aft_ext_span_per() - strake_span_per()) * cos( (mid_dihed() + rot.x()) *DEG_2_RAD);
  }
  else
  {
    y += sp*aft_ext_span_per() * cos( (in_dihed() + rot.x())*DEG_2_RAD );
  }  

  return y;
}

//==== Get z locations of aft extension chord's origin from aircrafts origin ====
double WingGeom::get_z_loc_aft_chord()
{
  vec3d v = getTotalTranVec();
  double z = v.z();
  vec3d rot = getTotalRotVec();
  
  double sp =  span();  
  if ( getSymCode() == XZ_SYM ) // Check Sym Code
    sp *= 0.5; 

  if ( strake_span_per() < aft_ext_span_per() )
  {
    z += sp*strake_span_per() * sin( (in_dihed() + rot.x())*DEG_2_RAD );
    z += sp*(aft_ext_span_per() - strake_span_per()) * sin( (mid_dihed() + rot.x())*DEG_2_RAD  );
  }
  else
  {
    z += sp*aft_ext_span_per() * sin( (in_dihed() + rot.x())*DEG_2_RAD );
  }  
    
  return z;
}

//==== Get x location of tip chord's origin from aircrafts origin ====
double WingGeom::get_x_loc_tip_chord()
{
  double sp =  span();  
  if ( getSymCode() == XZ_SYM ) // Check Sym Code
    sp *= 0.5;   
  
  vec3d v = getTotalTranVec();
  return (  v.x() + sp * get_tan_sweep_at(0.0)  );
}

//==== Get y location of tip chord's origin from aircrafts origin ====
double WingGeom::get_y_loc_tip_chord()
{
  vec3d v = getTotalTranVec();
  double y = v.y();
  vec3d rot = getTotalRotVec();
  
  double sp =  span();  
  if ( getSymCode() == XZ_SYM ) // Check Sym Code
    sp *= 0.5; 
    
  if ( strake_span_per() < aft_ext_span_per() )
  {
    y += sp*strake_span_per() * cos( (in_dihed() + rot.x())*DEG_2_RAD );
    y += sp*(aft_ext_span_per() - strake_span_per()) * cos( (mid_dihed() + rot.x())*DEG_2_RAD );
    y += sp*(1.0 - aft_ext_span_per()) * cos( (out_dihed() + rot.x())*DEG_2_RAD );
  }
  else
  {
    y += sp*aft_ext_span_per() * cos( (in_dihed() + rot.x())*DEG_2_RAD );
    y += sp*(strake_span_per() - aft_ext_span_per()) * cos( (mid_dihed() + rot.x())*DEG_2_RAD );
    y += sp*(1.0 - strake_span_per()) * cos( (out_dihed() + rot.x())*DEG_2_RAD );
  }  

  return y;
}

//==== Get z location of tip chord's origin from aircrafts origin ====
double WingGeom::get_z_loc_tip_chord()
{
  vec3d v = getTotalTranVec();
  double z = v.z();
  vec3d rot = getTotalRotVec();
  
  double sp =  span();  
  if ( getSymCode() == XZ_SYM ) // Check Sym Code
    sp *= 0.5; 

  if ( strake_span_per() < aft_ext_span_per() )
  {
    z += sp*strake_span_per() * sin( ( in_dihed() + rot.x() )*DEG_2_RAD );
    z += sp*(aft_ext_span_per() - strake_span_per()) * sin( ( mid_dihed() + rot.x() )*DEG_2_RAD );
    z += sp*(1.0 - aft_ext_span_per()) * sin( ( out_dihed() + rot.x() )*DEG_2_RAD );
    }
  else
  {
    z += sp*aft_ext_span_per() * sin( ( in_dihed() + rot.x() )*DEG_2_RAD );
    z += sp*(strake_span_per() - aft_ext_span_per()) * sin( ( mid_dihed() + rot.x() )*DEG_2_RAD );
    z += sp*(1.0 - strake_span_per()) * sin( ( out_dihed() + rot.x() )*DEG_2_RAD );
  }  
    
  return z;
}


//==== Set Num Pnts On All Airfoils and on Surf ====//
void WingGeom::set_num_pnts_all_afs()
{
  int npnts = numPnts.iget();

  if ( npnts%2 == 0 )
    numPnts.set( npnts + 1 );

  root_foil->set_num_pnts(numPnts.iget());
  strake_foil->set_num_pnts(numPnts.iget());
  aft_foil->set_num_pnts(numPnts.iget());
  tip_foil->set_num_pnts(numPnts.iget());

  mwing_surf.set_num_pnts( numPnts.iget() );
}

//==== Get Current Airfoil Ptr ====//
Af* WingGeom::get_af_ptr()
{
  Af* curr_af = root_foil;
  switch (curr_af_id)
    {
      case MW_ROOT_AF:
        curr_af = root_foil;
      break;

      case MW_STRAKE_AF:
        curr_af = strake_foil;
      break;

      case MW_AFT_AF:
        curr_af = aft_foil;
      break;

      case MW_TIP_AF:
        curr_af = tip_foil;
      break;
    }
  return(curr_af);
}

//==== Driver Group Has Changed ====//
void WingGeom::set_driver(int driver_in)
{
  int valid_flag = TRUE;

  switch (driver_in)
    {
      case AR_TR_A:
        aspect.activate();  taper.activate();         area.activate();
        span.deactivate();  root_chord.deactivate();  tip_chord.deactivate();
      break;

      case AR_TR_S:
        aspect.activate();  taper.activate();         span.activate();
        area.deactivate();  root_chord.deactivate();  tip_chord.deactivate();
      break;

      case AR_TR_TC:
        aspect.activate();  taper.activate();         tip_chord.activate();
        area.deactivate();  root_chord.deactivate();  span.deactivate();
      break;

      case AR_TR_RC:
        aspect.activate();  taper.activate();         root_chord.activate();
        area.deactivate();  tip_chord.deactivate();   span.deactivate();
      break;

      case S_TC_RC:
        span.activate();    tip_chord.activate();  root_chord.activate();
        taper.deactivate(); aspect.deactivate();   area.deactivate();
     break;

      case A_TC_RC:
        area.activate();    tip_chord.activate();  root_chord.activate();
        taper.deactivate(); aspect.deactivate();   span.deactivate();
      break;

      case TR_S_A:
        taper.activate();    span.activate();           area.activate();
        aspect.deactivate(); root_chord.deactivate();   tip_chord.deactivate();
      break;

      default:
        valid_flag = FALSE;
      break;
    }
  if (valid_flag)
    driver = driver_in;
}

//==== Calculate Dependant Parms ====//
void WingGeom::set_depend_parms()
{
  switch (driver)
    {
      case AR_TR_A:
        span       = sqrt( aspect()*area() );
        root_chord = (2.0*span()) / ( aspect()*(1.0+taper()) );
        tip_chord  = taper()*root_chord();
      break;

      case AR_TR_S:
        area       = (span() * span()) / aspect();
        root_chord = (2.0*span()) / ( aspect()*(1.0+taper()) );
        tip_chord  = taper()*root_chord();
      break;

      case AR_TR_TC:
        root_chord = tip_chord()/taper();
        span       = 0.5*aspect()*root_chord()*(1.0+taper());
        area       = (span() * span()) / aspect();
      break;

      case AR_TR_RC:
        tip_chord = taper()*root_chord();
        span      = 0.5*aspect()*root_chord()*(1.0+taper());
        area      = (span() * span()) / aspect();
      break;

      case S_TC_RC:
        taper     = tip_chord()/root_chord();
        aspect    = 2.0*span()/( root_chord()*(1.0+taper()) );
        area      = (span() * span()) / aspect();
     break;

      case A_TC_RC:
        taper     = tip_chord()/root_chord();
        aspect    = 2.0*span()/( root_chord()*(1.0+taper()) );
        span      = sqrt( aspect()*area() );
      break;

      case TR_S_A:
        aspect     = (span() * span()) / area();
        root_chord = (2.0*span()) / ( aspect()*(1.0+taper()) );
        tip_chord  = taper()*root_chord();
      break;
    }
}


//==== Write Multi Section Wing File ====//
void WingGeom::write(FILE* file_id)
{

  fprintf(file_id, "//***************** WING_STRAKE_AFT COMPONENT *****************//\n");
/*jrg - use xml???

  //==== Write General Parms ====//
  write_general_parms(file_id);

  fprintf(file_id, "%d                  Num_Pnts\n",num_pnts.iget());
  fprintf(file_id, "%d                  Driver\n",driver);
  fprintf(file_id, "%f                  Aspect_Ratio\n",aspect());
  fprintf(file_id, "%f                  Taper_Ratio\n",taper());
  fprintf(file_id, "%f                  Area (ft^2)\n",area());
  fprintf(file_id, "%f                  Span (ft)\n",span());
  fprintf(file_id, "%f                  Tip_Chord (ft)\n",tip_chord());
  fprintf(file_id, "%f                  Root_Chord (ft)\n",root_chord());

  fprintf(file_id, "%f                  Sweep (deg)\n",(double)((Degrees)sweep()));
  fprintf(file_id, "%f                  Sweep_Loc\n",sweep_loc());

  fprintf(file_id, "%f                  Twist_Loc\n",twist_loc());
  fprintf(file_id, "%f                  In_Twist (deg)\n",(float)((Degrees)in_twist()));
  fprintf(file_id, "%f                  In_Dihed (deg)\n",(float)((Degrees)in_dihed()));
  fprintf(file_id, "%f                  Mid_Twist (deg)\n",(float)((Degrees)mid_twist()));
  fprintf(file_id, "%f                  Mid_Dihed (deg)\n",(float)((Degrees)mid_dihed()));
  fprintf(file_id, "%f                  Out_Twist (deg)\n",(float)((Degrees)out_twist()));
  fprintf(file_id, "%f                  Out_Dihed (deg)\n",(float)((Degrees)out_dihed()));

  fprintf(file_id, "%f                  Strake_Sweep (deg)\n",(float)((Degrees)strake_sweep()));
  fprintf(file_id, "%f                  Strake_Span_Per\n",strake_span_per());
  fprintf(file_id, "%f                  Aft_Ext_Sweep (deg)\n",(float)((Degrees)aft_ext_sweep()));
  fprintf(file_id, "%f                  Aft_Ext_Span_Per\n",aft_ext_span_per());

  fprintf(file_id, "%d                  All_Move_Flag\n",all_move_flag);
  fprintf(file_id, "%d                  In_Flap_Type\n",in_flap_type);
  fprintf(file_id, "%d                  Out_Flap_Type\n",out_flap_type);
  fprintf(file_id, "%d                  In_Slat_Type\n",in_slat_type);
  fprintf(file_id, "%d                  Out_Slat_Type\n",out_slat_type);

  fprintf(file_id, "%f                  In_Flap_Span_In\n",in_flap_span_in());
  fprintf(file_id, "%f                  In_Flap_Span_Out\n",in_flap_span_out());
  fprintf(file_id, "%f                  In_Flap_Chord\n",in_flap_chord());
  fprintf(file_id, "%f                  Out_Flap_Span_In\n",out_flap_span_in());
  fprintf(file_id, "%f                  Out_Flap_Span_Out\n",out_flap_span_out());
  fprintf(file_id, "%f                  Out_Flap_Chord\n",out_flap_chord());

  fprintf(file_id, "%f                  In_Slat_Span_In\n",in_slat_span_in());
  fprintf(file_id, "%f                  In_Slat_Span_Out\n",in_slat_span_out());
  fprintf(file_id, "%f                  In_Slat_Chord\n",in_slat_chord());
  fprintf(file_id, "%f                  Out_Slat_Span_In\n",out_slat_span_in());
  fprintf(file_id, "%f                  Out_Slat_Span_Out\n",out_slat_span_out());
  fprintf(file_id, "%f                  Out_Slat_Chord\n",out_slat_chord());

  fprintf(file_id, "%d                  Deflect On_Off\n",deflect_flag );
  fprintf(file_id, "%s                  Deflect Name\n",(char *)deflect_name );
  fprintf(file_id, "%f                  Deflect Scale\n",defl_scale());
  fprintf(file_id, "%f                  Deflect Twist Scale\n",twist_scale());

  int num_defl_pnts = deflect_curve.get_num_pnts();
  fprintf(file_id, "%d                  Num Deflect Pnts\n",num_defl_pnts );

  for ( int i = 0 ; i < num_defl_pnts ; i++ )
    {
      vec3d dp = deflect_curve.get_pnt(i);
      vec3d tp = twist_curve.get_pnt(i);
      fprintf(file_id, "%f  %f   %f\n", dp.x(), dp.y(), tp.y() );
    }


  root_foil->write( file_id );
  strake_foil->write( file_id );
  aft_foil->write( file_id );
  tip_foil->write( file_id );
*/
}


//==== Write Multi Section Wing File ====//
void WingGeom::write(xmlNodePtr root)
{

  xmlAddStringNode( root, "Type", "Mwing");

  //==== Write General Parms ====//
  xmlNodePtr gen_node = xmlNewChild( root, NULL, (const xmlChar *)"General_Parms", NULL );
  write_general_parms( gen_node );

  //==== Write Fuse Parms ====//
  xmlNodePtr mwing_node = xmlNewChild( root, NULL, (const xmlChar *)"Mwing_Parms", NULL );

  xmlAddIntNode( mwing_node, "Num_Pnts", numPnts.iget() );
  xmlAddIntNode( mwing_node, "Driver", driver );
  xmlAddDoubleNode( mwing_node, "Aspect_Ratio", aspect() );
  xmlAddDoubleNode( mwing_node, "Taper_Ratio", taper() );
  xmlAddDoubleNode( mwing_node, "Area", area() );
  xmlAddDoubleNode( mwing_node, "Span", span() );
  xmlAddDoubleNode( mwing_node, "Tip_Chord", tip_chord() );
  xmlAddDoubleNode( mwing_node, "Root_Chord", root_chord() );

  xmlAddDoubleNode( mwing_node, "Sweep", sweep()  );
  xmlAddDoubleNode( mwing_node, "Sweep_Loc", sweep_loc() );

  xmlAddDoubleNode( mwing_node, "Twist_Loc", twist_loc() );
  xmlAddDoubleNode( mwing_node, "In_Twist", in_twist() );
  xmlAddDoubleNode( mwing_node, "In_Dihed", in_dihed() );
  xmlAddDoubleNode( mwing_node, "Mid_Twist", mid_twist() );
  xmlAddDoubleNode( mwing_node, "Mid_Dihed", mid_dihed() );
  xmlAddDoubleNode( mwing_node, "Out_Twist", out_twist() );
  xmlAddDoubleNode( mwing_node, "Out_Dihed", out_dihed() );

  xmlAddDoubleNode( mwing_node, "Strake_Sweep",  strake_sweep()  );
  xmlAddDoubleNode( mwing_node, "Strake_Span_Per", strake_span_per() );
  xmlAddDoubleNode( mwing_node, "Aft_Ext_Sweep",  aft_ext_sweep()  );
  xmlAddDoubleNode( mwing_node, "Aft_Ext_Span_Per", aft_ext_span_per() );

  xmlAddIntNode( mwing_node, "All_Move_Flag", all_move_flag );
  xmlAddIntNode( mwing_node, "In_Flap_Type", in_flap_type );
  xmlAddIntNode( mwing_node, "Out_Flap_Type", out_flap_type );
  xmlAddIntNode( mwing_node, "In_Slat_Type", in_slat_type );
  xmlAddIntNode( mwing_node, "Out_Slat_Type", out_slat_type );

  xmlAddDoubleNode( mwing_node, "In_Flap_Span_In", in_flap_span_in() );
  xmlAddDoubleNode( mwing_node, "In_Flap_Span_Out", in_flap_span_out() );
  xmlAddDoubleNode( mwing_node, "In_Flap_Chord", in_flap_chord() );
  xmlAddDoubleNode( mwing_node, "Out_Flap_Span_In", out_flap_span_in() );
  xmlAddDoubleNode( mwing_node, "Out_Flap_Span_Out", out_flap_span_out() );
  xmlAddDoubleNode( mwing_node, "Out_Flap_Chord", out_flap_chord() );

  xmlAddDoubleNode( mwing_node, "In_Slat_Span_In", in_slat_span_in() );
  xmlAddDoubleNode( mwing_node, "In_Slat_Span_Out", in_slat_span_out() );
  xmlAddDoubleNode( mwing_node, "In_Slat_Chord", in_slat_chord() );
  xmlAddDoubleNode( mwing_node, "Out_Slat_Span_In", out_slat_span_in() );
  xmlAddDoubleNode( mwing_node, "Out_Slat_Span_Out", out_slat_span_out() );
  xmlAddDoubleNode( mwing_node, "Out_Slat_Chord", out_slat_chord() );

  xmlAddIntNode( mwing_node, "Deflect_On_Off", deflect_flag );
  xmlAddStringNode( mwing_node, "Deflect_Name", (char *)deflect_name );
  xmlAddDoubleNode( mwing_node, "Deflect_Scale", defl_scale() );
  xmlAddDoubleNode( mwing_node, "Deflect_Twist_Scale", twist_scale() );

  xmlAddIntNode( mwing_node, "Strake_Aft_Flag", strake_aft_flag );

  Stringc deflstr;
  char numstr[255];

  for ( int i = 0 ; i < deflect_curve.get_num_pnts() ; i++ )
  {
    vec3d dp = deflect_curve.get_pnt(i);
    vec3d tp = twist_curve.get_pnt(i);
    sprintf( numstr, "%lf, %lf, %lf,", dp.x(), dp.y(), tp.y() );
    deflstr.concatenate(numstr);
  }
  deflstr.concatenate("\0");
  xmlAddStringNode( mwing_node, "Deflect_Pnts", deflstr );

  //==== Write Airfoils ====//
  xmlNodePtr af_root_node = xmlNewChild( root, NULL, (const xmlChar *)"Root_Airfoil", NULL );
  root_foil->write( af_root_node );

  xmlNodePtr af_strake_node = xmlNewChild( root, NULL, (const xmlChar *)"Strake_Airfoil", NULL );
  strake_foil->write( af_strake_node );

  xmlNodePtr af_aft_node = xmlNewChild( root, NULL, (const xmlChar *)"Aft_Airfoil", NULL );
  aft_foil->write( af_aft_node );

  xmlNodePtr af_tip_node = xmlNewChild( root, NULL, (const xmlChar *)"Tip_Airfoil", NULL );
  tip_foil->write( af_tip_node );

}


//==== Read Multi Section Wing File ====//
void WingGeom::read(xmlNodePtr root)
{

  xmlNodePtr node;

  //===== Read General Parameters =====//
  node = xmlGetNode( root, "General_Parms", 0 );
  if ( node )
    read_general_parms( node );

  //===== Read Fuse Parameters =====//
  node = xmlGetNode( root, "Mwing_Parms", 0 );
  if ( node )
  {
    numPnts =  xmlFindInt( node, "Num_Pnts", numPnts.iget() );
    driver   =  xmlFindInt( node, "Driver", driver );

    aspect   =  xmlFindDouble( node, "Aspect_Ratio", aspect() );
    taper    =  xmlFindDouble( node, "Taper_Ratio", taper() );
    area     =  xmlFindDouble( node, "Area", area() );
    span     =  xmlFindDouble( node, "Span", span() );
    tip_chord  =  xmlFindDouble( node, "Tip_Chord", tip_chord() );
    root_chord   =  xmlFindDouble( node, "Root_Chord", root_chord() );

    sweep     =  xmlFindDouble( node, "Sweep",  sweep() ) ;
    sweep_loc =  xmlFindDouble( node, "Sweep_Loc", sweep_loc() );

    old_sweep_loc = sweep_loc();

    twist_loc =  xmlFindDouble( node, "Twist_Loc", twist_loc() );
    in_twist   =  xmlFindDouble( node, "In_Twist", in_twist() );
    in_dihed   =  xmlFindDouble( node, "In_Dihed", in_dihed() );
    mid_twist  =  xmlFindDouble( node, "Mid_Twist", mid_twist() );
    mid_dihed  =  xmlFindDouble( node, "Mid_Dihed", mid_dihed() );
    out_twist  =  xmlFindDouble( node, "Out_Twist", out_twist() );
    out_dihed  =  xmlFindDouble( node, "Out_Dihed", out_dihed() );

    strake_sweep =  xmlFindDouble( node, "Strake_Sweep",  strake_sweep() );
    strake_span_per   =  xmlFindDouble( node, "Strake_Span_Per", strake_span_per() );
    aft_ext_sweep =  xmlFindDouble( node, "Aft_Ext_Sweep", aft_ext_sweep() );
    aft_ext_span_per   =  xmlFindDouble( node, "Aft_Ext_Span_Per", aft_ext_span_per() );

    all_move_flag =  xmlFindInt( node, "All_Move_Flag", all_move_flag );
    in_flap_type  =  xmlFindInt( node, "In_Flap_Type", in_flap_type );
    out_flap_type =  xmlFindInt( node, "Out_Flap_Type", out_flap_type );
    in_slat_type  =  xmlFindInt( node, "In_Slat_Type", in_slat_type );
    out_slat_type =  xmlFindInt( node, "Out_Slat_Type", out_slat_type );

    in_flap_span_in   =  xmlFindDouble( node, "In_Flap_Span_In", in_flap_span_in() );
    in_flap_span_out  =  xmlFindDouble( node, "In_Flap_Span_Out", in_flap_span_out() );
    in_flap_chord     =  xmlFindDouble( node, "In_Flap_Chord", in_flap_chord() );
    out_flap_span_in  =  xmlFindDouble( node, "Out_Flap_Span_In", out_flap_span_in() );
    out_flap_span_out =  xmlFindDouble( node, "Out_Flap_Span_Out", out_flap_span_out() );
    out_flap_chord    =  xmlFindDouble( node, "Out_Flap_Chord", out_flap_chord() );

    in_slat_span_in   =  xmlFindDouble( node, "In_Slat_Span_In", in_slat_span_in() );
    in_slat_span_out  =  xmlFindDouble( node, "In_Slat_Span_Out", in_slat_span_out() );
    in_slat_chord     =  xmlFindDouble( node, "In_Slat_Chord", in_slat_chord() );
    out_slat_span_in  =  xmlFindDouble( node, "Out_Slat_Span_In", out_slat_span_in() );
    out_slat_span_out =  xmlFindDouble( node, "Out_Slat_Span_Out", out_slat_span_out() );
    out_slat_chord    =  xmlFindDouble( node, "Out_Slat_Chord", out_slat_chord() );

    deflect_flag =  xmlFindInt( node, "Deflect_On_Off", deflect_flag );
    deflect_name =  xmlFindString( node, "Deflect_Name", "Def_File" );
    defl_scale   =  xmlFindDouble( node, "Deflect_Scale", defl_scale() );
    twist_scale  =  xmlFindDouble( node, "Deflect_Twist_Scale", twist_scale() );

	strake_aft_flag = xmlFindInt( node, "Strake_Aft_Flag", strake_aft_flag );

    xmlNodePtr def_node = xmlGetNode( node, "Deflect_Pnts", 0 );

    if ( def_node )
    {
      int num_arr =  xmlGetNumArray( def_node, ',' );
      double* arr = (double*)malloc( num_arr*sizeof(double) );

      xmlExtractDoubleArray( def_node, ',', arr, num_arr );

      int num_defl = num_arr/3;

      deflect_curve.init( num_defl );
      twist_curve.init( num_defl );

      for ( int i = 0 ; i < num_arr ; i+=3 )
      {
        deflect_curve.load_pnt(  i/3, vec3d(arr[i], arr[i+1], 0.0));
        twist_curve.load_pnt(    i/3, vec3d(arr[i], arr[i+2], 0.0));
      }
      free(arr);
    }
  }
  xmlNodePtr af_root_node = xmlGetNode( root, "Root_Airfoil", 0 );
  if ( af_root_node )
    root_foil->read( af_root_node);

  xmlNodePtr af_strake_node = xmlGetNode( root, "Strake_Airfoil", 0 );
  if ( af_strake_node )
    strake_foil->read( af_strake_node);

  xmlNodePtr af_aft_node = xmlGetNode( root, "Aft_Airfoil", 0 );
  if ( af_aft_node )
    aft_foil->read( af_aft_node );

  xmlNodePtr af_tip_node = xmlGetNode( root, "Tip_Airfoil", 0 );
  if ( af_aft_node )
    tip_foil->read( af_tip_node );

  set_num_pnts_all_afs();
  generate();

  //===== Initialize Ref Area, Span and Cbar =====
//  ref_area = area();
//  ref_span = span();
//  ref_cbar = get_cbar();

}
//==== Read Simple File ====//
void WingGeom::readSimple(FILE* file_id)
{
  char buff[255];

  //==== Read General Parms ====// 
  read_general_parms(file_id);

  //==== Read Simple Wing Parms ====//		
  fgets(buff, 80, file_id);
  fscanf(file_id, "%d",&driver);             fgets(buff, 80, file_id);

  span.read(file_id);
  aspect.read(file_id);
  taper.read(file_id);
  area.read(file_id);
  root_chord.read(file_id);
  tip_chord.read(file_id);
 
  float tan_sweep;
  fscanf(file_id, "%f",&tan_sweep);              fgets(buff, 80, file_id);
  sweep = atan(tan_sweep)*RAD_2_DEG;
  sweep_loc.read(file_id);
  old_sweep_loc = sweep_loc();

  float tan_dihedral;
  fscanf(file_id, "%f",&tan_dihedral);             fgets(buff, 80, file_id);
  in_dihed = atan(tan_dihedral)*RAD_2_DEG;
  mid_dihed = atan(tan_dihedral)*RAD_2_DEG;
  out_dihed = atan(tan_dihedral)*RAD_2_DEG;

  twist_loc.read(file_id);

  float temp_twist;
  fscanf(file_id, "%f",&temp_twist);             fgets(buff, 80, file_id);

  in_twist =  temp_twist*RAD_2_DEG/3.0;
  mid_twist = temp_twist*RAD_2_DEG/3.0;
  out_twist = temp_twist*RAD_2_DEG/3.0;

  //==== Skip Flap Stuff ====//
  fgets(buff, 80, file_id);
  fgets(buff, 80, file_id);
  fgets(buff, 80, file_id);
  fgets(buff, 80, file_id);
  fgets(buff, 80, file_id);
  fgets(buff, 80, file_id);
  fgets(buff, 80, file_id);
  fgets(buff, 80, file_id);
  fgets(buff, 80, file_id);

  //==== Read Airfoils ====//
  //==== Root Airfoil ====/
  fgets(buff, 80, file_id);
  int numaf;
  fscanf(file_id, "%d",&numaf);             fgets(buff, 80, file_id);

  float rootcam, rootloc, rootthick;
  fscanf(file_id, "%f",&rootcam);           fgets(buff, 80, file_id);
  fscanf(file_id, "%f",&rootloc);           fgets(buff, 80, file_id);
  fscanf(file_id, "%f",&rootthick);         fgets(buff, 80, file_id);

  //==== Root Airfoil ====/
  fgets(buff, 80, file_id);
  fscanf(file_id, "%d",&numaf);             fgets(buff, 80, file_id);

  float tipcam, tiploc, tipthick;
  fscanf(file_id, "%f",&tipcam);           fgets(buff, 80, file_id);
  fscanf(file_id, "%f",&tiploc);           fgets(buff, 80, file_id);
  fscanf(file_id, "%f",&tipthick);         fgets(buff, 80, file_id);

  //==== Root Foil ====//
  root_foil->set_type( NACA_4_SERIES );
  root_foil->get_camber()->set( rootcam );
  root_foil->get_camber_loc()->set( rootloc );
  root_foil->get_thickness()->set( rootthick );
  root_foil->generate_airfoil();

  strake_foil->set_type( NACA_4_SERIES );
  strake_foil->get_camber()->set( rootcam + (tipcam-rootcam)/3.0 );
  strake_foil->get_camber_loc()->set( rootloc + (tiploc-rootloc)/3.0);
  strake_foil->get_thickness()->set( rootthick + (tipthick-rootthick)/3.0);
  strake_foil->generate_airfoil();

  aft_foil->set_type( NACA_4_SERIES );
  aft_foil->get_camber()->set( rootcam + 2.0*(tipcam-rootcam)/3.0 );
  aft_foil->get_camber_loc()->set( rootloc + 2.0*(tiploc-rootloc)/3.0);
  aft_foil->get_thickness()->set( rootthick + 2.0*(tipthick-rootthick)/3.0);
  aft_foil->generate_airfoil();

  //==== Tip Foil ====//
  tip_foil->set_type( NACA_4_SERIES );
  tip_foil->get_camber()->set( tipcam );
  tip_foil->get_camber_loc()->set( tiploc );
  tip_foil->get_thickness()->set( tipthick );
  tip_foil->generate_airfoil();


  //==== No Strake/Aft Ext ====//
  strake_span_per  = 1.0/3.0;
  aft_ext_span_per = 2.0/3.0;
  strake_aft_flag = 0;
  generate();

  //===== Initialize Ref Area, Span and Cbar =====
//  ref_area = area();
//  ref_span = span();
//  ref_cbar = get_cbar();

}

//==== Read Multi Section Wing File ====//
void WingGeom::read(FILE* file_id)
{
  char buff[255];

  //==== Read General Parms ====//
  read_general_parms(file_id);

  //==== Read Wing - Strake - Aft Parms ====//
  int idum;
  fscanf(file_id, "%d",&idum);                  fgets(buff, 80, file_id);
  numPnts = idum;
  set_num_pnts_all_afs();

  fscanf(file_id, "%d",&driver);                fgets(buff, 80, file_id);
  aspect.read(file_id);
  taper.read(file_id);
  area.read(file_id);
  span.read(file_id);
  tip_chord.read(file_id);
  root_chord.read(file_id);

  sweep.read(file_id);
  sweep_loc.read(file_id);
  old_sweep_loc = sweep_loc();

  twist_loc.read(file_id);
  in_twist.read(file_id);
  in_dihed.read(file_id);
  mid_twist.read(file_id);
  mid_dihed.read(file_id);
  out_twist.read(file_id);
  out_dihed.read(file_id);

  strake_sweep.read(file_id);
  strake_span_per.read(file_id);

  aft_ext_sweep.read(file_id);
  aft_ext_span_per.read(file_id);

  fscanf(file_id, "%d",&all_move_flag);         fgets(buff, 80, file_id);
  fscanf(file_id, "%d",&in_flap_type);          fgets(buff, 80, file_id);
  fscanf(file_id, "%d",&out_flap_type);         fgets(buff, 80, file_id);
  fscanf(file_id, "%d",&in_slat_type);          fgets(buff, 80, file_id);
  fscanf(file_id, "%d",&out_slat_type);         fgets(buff, 80, file_id);

  in_flap_span_in.read(file_id);
  in_flap_span_out.read(file_id);
  in_flap_chord.read(file_id);
  out_flap_span_in.read(file_id);
  out_flap_span_out.read(file_id);
  out_flap_chord.read(file_id);

  in_slat_span_in.read(file_id);
  in_slat_span_out.read(file_id);
  in_slat_chord.read(file_id);
  out_slat_span_in.read(file_id);
  out_slat_span_out.read(file_id);
  out_slat_chord.read(file_id);

  int version = get_aircraft_ptr()->get_version();
  if ( version >= 6 )
    {
      fscanf(file_id, "%d",&deflect_flag);      fgets(buff, 80, file_id);
      char str[255];
      fscanf(file_id, "%s",&str);               fgets(buff, 80, file_id);
      deflect_name = str;
      defl_scale.read(file_id);         // Deflect
      twist_scale.read(file_id);        // Twist
      int num_defl;
      fscanf(file_id, "%d",&num_defl);          fgets(buff, 80, file_id);

      deflect_curve.init( num_defl );
      twist_curve.init( num_defl );

      for ( int i = 0 ; i < num_defl ; i++ )
        {
          float x, y, z;
          fscanf(file_id, "%f  %f  %f\n",&x, &y,  &z);
          deflect_curve.load_pnt(  i, vec3d(x, y, 0.0));
          twist_curve.load_pnt(    i, vec3d(x, z, 0.0));
        }

    }
  root_foil->read( file_id );
  strake_foil->read( file_id );
  aft_foil->read( file_id );
  tip_foil->read( file_id );
  set_num_pnts_all_afs();

  generate();

  //===== Initialize Ref Area, Span and Cbar =====
//  ref_area = area();
//  ref_span = span();
//  ref_cbar = get_cbar();


}


//==== Write Rhino File ====//
void WingGeom::write_rhino_file(ON_BinaryFile* archive, ON_3dmObjectAttributes* attributes)
{
	mwing_surf.clear_pnt_tan_flags();
	mwing_surf.clear_xsec_tan_flags();

	//==== Sharpen Trailing Edge of Wing ====//
	mwing_surf.set_pnt_tan_flag( 0, Bezier_curve::SHARP );
    int num_pnts  = mwing_surf.get_num_pnts();
	mwing_surf.set_pnt_tan_flag( num_pnts-1, Bezier_curve::SHARP );


	//==== Sharpen Wing Joints ====//
	int num_xsecs = mwing_surf.get_num_xsecs();

	for ( int j = 0 ; j < num_xsecs ; j++ )
		mwing_surf.set_xsec_tan_flag( j, Bezier_curve::SHARP );

	//==== Write File ====//
	mwing_surf.write_rhino_file( sym_code, model_mat, reflect_mat, archive, attributes );
}

//==== Generate Cross Sections =====//
void WingGeom::regenerate()
{
}

//==== Generate Cross Sections =====//
void WingGeom::generate_surf()
{
  int num_pnts = mwing_surf.get_num_pnts();

  //==== Check For Straight Taper Wing Flag ====//
  if ( !strake_aft_flag )
    {
      strake_sweep  = get_sweep_at(0.0)*RAD_2_DEG;
      aft_ext_sweep = get_sweep_at(1.0)*RAD_2_DEG;
    }

  //==== Main Wing Parms ====//
  double ty      = span();                       // Tip Y Offset
  if ( getSymCode() == XZ_SYM )  ty *= 0.5;   // Check Sym Code

  double tan_le  = get_tan_sweep_at(0.0);        // Tangent Leading Edge
  double toff    = tan_le*ty;                    // Tip X Offset
  double tc      = tip_chord();                  // Tip Chord
  double rc      = root_chord();                 // Root Chord

  //==== Strake Parms ====//
  double sy      = strake_span_per()*ty;                 // Strake Y Offset
  double sc      = (tc - rc)*strake_span_per() + rc;     // Strake Chord
  double sx      = tan(strake_sweep()*DEG_2_RAD)*sy;     // Strake X Length
  double strim   = toff*strake_span_per();               // Trim X Strake
  double sexp    = sx - strim;                           // Exposed X Strake

  //==== Aft Ext Parms ====//
  double ay      = aft_ext_span_per()*ty;                // Aft Y Offset
  double ax      = tan(-aft_ext_sweep()*DEG_2_RAD)*ay;   // Aft X Intercept
  double tan_te  = get_tan_sweep_at(1.0);                // Tangenet Trailing Edge
  double atrim   = -tan_te*ty*aft_ext_span_per();        // Aft Trimmed X
  double aexp    = ax - atrim;                           // Aft Exposed X
  double ac      = (tc - rc)*aft_ext_span_per() + rc;    // Aft Chord

  //==== Total Parms ====//
  double rct     = rc + sexp + aexp;                     // Total Root Chord
  double act, sct;                       // Total Aft and Strake Chords
  double aoff;                           // Aft X Offset
  double az, sz, tz;                     // Aft, Strake, Tip Z Offset
  double atwst, stwst;                   // Aft and Strake Twists
  int aid, sid;                         // Xsec Location IDs

  //==== Check If Aft is Inboard Of Strake ====//
  if ( ay < sy )
    {
      act  = ac + sexp*(1.0 - (aft_ext_span_per()/strake_span_per()));
      aoff = rc - atrim - act;
      sct  = sc;
      aid = 2;  sid = 3;
      az = tan(in_dihed()*DEG_2_RAD)*ay;
      sz = az + tan(mid_dihed()*DEG_2_RAD)*(sy - ay);
      tz = sz + tan(out_dihed()*DEG_2_RAD)*(ty - sy);
      atwst = in_twist();
      stwst = atwst + mid_twist();
    }
  else
    {
      act  = ac;
      aoff = toff*aft_ext_span_per();
      sct  = sc + aexp*(1.0 - (strake_span_per()/aft_ext_span_per()));;
      aid = 3;  sid = 2;
      sz = tan(in_dihed()*DEG_2_RAD)*sy;
      az = sz + tan(mid_dihed()*DEG_2_RAD)*(ay - sy);
      tz = az + tan(out_dihed()*DEG_2_RAD)*(ty - ay);
      stwst = in_twist();
      atwst = stwst + mid_twist();
    }

  //==== Load In Airfoils ====//
  for ( int i = 0 ; i < num_pnts ; i++ )
    {
      mwing_surf.set_pnt(0,  i, root_foil->get_end_cap(i));
      mwing_surf.set_pnt(1,  i, root_foil->get_pnt(i));
      mwing_surf.set_pnt(aid,i, aft_foil->get_pnt(i));
      mwing_surf.set_pnt(sid,i, strake_foil->get_pnt(i));
      mwing_surf.set_pnt(4,  i, tip_foil->get_pnt(i));
      mwing_surf.set_pnt(5,  i, tip_foil->get_end_cap(i));
    }

  //==== If Deflection File - Adjust Offsets and Twist ====//
  double total_tip_twist = in_twist() + mid_twist() + out_twist();
  if ( deflect_flag )
    {
      vec3d p = deflect_curve.comp_pnt_per_xyz( 0, aft_ext_span_per());
      az += p.y()*defl_scale();
      p = deflect_curve.comp_pnt_per_xyz( 0, strake_span_per());
      sz += p.y()*defl_scale();
      p = deflect_curve.comp_pnt_per_xyz( 0, 1.0 );
      tz += p.y()*defl_scale();

      p = twist_curve.comp_pnt_per_xyz( 0, aft_ext_span_per() );
      atwst += p.y()*twist_scale();
      p = twist_curve.comp_pnt_per_xyz( 0, strake_span_per() );
      stwst += p.y()*twist_scale();
      p = twist_curve.comp_pnt_per_xyz( 0, 1.0 );
      total_tip_twist += p.y()*twist_scale();
    }


  //==== Scale, Offset and Twist Cross Sections ====//
  mwing_surf.scale_xsec_x(  0, rct );
  mwing_surf.scale_xsec_z(  0, rct );
  mwing_surf.offset_xsec_x( 0, -sexp );

  mwing_surf.scale_xsec_x(  1, rct );
  mwing_surf.scale_xsec_z(  1, rct );
  mwing_surf.offset_xsec_x( 1, -sexp );

  mwing_surf.scale_xsec_x(  aid, act );
  mwing_surf.scale_xsec_z(  aid, act );
  mwing_surf.offset_xsec_x( aid, -twist_loc()*act);
  mwing_surf.rotate_xsec_y( aid, atwst);
  mwing_surf.offset_xsec_x( aid, aoff + twist_loc()*act );
  mwing_surf.offset_xsec_y( aid, ay );
  mwing_surf.offset_xsec_z( aid, az );

  mwing_surf.scale_xsec_x(  sid, sct );
  mwing_surf.scale_xsec_z(  sid, sct );
  mwing_surf.offset_xsec_x( sid, -twist_loc()*sct);
  mwing_surf.rotate_xsec_y( sid, stwst);
  mwing_surf.offset_xsec_x( sid, strim + twist_loc()*sct);
  mwing_surf.offset_xsec_y( sid, sy );
  mwing_surf.offset_xsec_z( sid, sz );

  mwing_surf.scale_xsec_x(  4, tc );
  mwing_surf.scale_xsec_z(  4, tc );
  mwing_surf.offset_xsec_x( 4, -twist_loc()*tc );
  mwing_surf.rotate_xsec_y( 4, total_tip_twist );
  mwing_surf.offset_xsec_x( 4, toff + twist_loc()*tc );
  mwing_surf.offset_xsec_y( 4, ty );
  mwing_surf.offset_xsec_z( 4, tz );

  mwing_surf.scale_xsec_x(  5, tc );
  mwing_surf.scale_xsec_z(  5, tc );
  mwing_surf.offset_xsec_x( 5, -twist_loc()*tc );
  mwing_surf.rotate_xsec_y( 5, total_tip_twist );
  mwing_surf.offset_xsec_x( 5, toff + twist_loc()*tc );
  mwing_surf.offset_xsec_y( 5, ty );
  mwing_surf.offset_xsec_z( 5, tz );


  mwing_surf.load_refl_pnts_xsecs();
//  if (main_window_ptr->get_display_type_flag() == DISPLAY_HIDDEN_LINE)
    mwing_surf.load_hidden_surf();
//  else if (main_window_ptr->get_display_type_flag() == DISPLAY_SHADED_SURF)
    mwing_surf.load_sharp_normals();

  //===== Compute Total Area ====//
  if ( getSymCode() == XZ_SYM )
    total_area = area() + sy*(sx-strim) + ay*(ax-atrim);
  else
    total_area = area() + 0.5*(sy*(sx-strim) + ay*(ax-atrim));

  generate_flap_slat_surfs();

}

//==== Generate Cross Sections =====//
void WingGeom::generate_flap_slat_surfs()
{
  vec3d pnt1, pnt2, pnt3, pnt4;
  int strake_id = 2;  int aft_id = 3;
  if ( strake_span_per() > aft_ext_span_per() )
    {
      strake_id = 3;
      aft_id = 2;
    }

  if ( in_flap_type == MW_NO_FLAP )
    {
      in_flap_surf.set_num_xsecs(0);
      in_flap_surf.set_num_pnts(0);
    }
  else
    {
      if ( in_flap_span_in() > in_flap_span_out() )
        {
          double split_val  = 0.5*(in_flap_span_in() + in_flap_span_out());
          in_flap_span_in  = split_val;
          in_flap_span_out = split_val;
        }
      in_flap_surf.set_num_xsecs(2);
      in_flap_surf.set_num_pnts(2);
      pnt1 = mwing_surf.get_pnt(1, 0);
      pnt2 = mwing_surf.linear_interpolate_xsec(1, in_flap_chord() );
      pnt3 = mwing_surf.get_pnt(aft_id, 0);
      pnt4 = mwing_surf.linear_interpolate_xsec(aft_id, in_flap_chord());
      in_flap_surf.set_pnt(0, 0, pnt1 + (pnt3 - pnt1)*in_flap_span_in());
      in_flap_surf.set_pnt(0, 1, pnt2 + (pnt4 - pnt2)*in_flap_span_in());
      in_flap_surf.set_pnt(1, 0, pnt1 + (pnt3 - pnt1)*in_flap_span_out());
      in_flap_surf.set_pnt(1, 1, pnt2 + (pnt4 - pnt2)*in_flap_span_out());
      in_flap_surf.load_refl_pnts_xsecs();
    }
  if ( out_flap_type == MW_NO_FLAP )
    {
      out_flap_surf.set_num_xsecs(0);
      out_flap_surf.set_num_pnts(0);
    }
  else
    {
      if ( out_flap_span_in() > out_flap_span_out() )
        {
          double split_val  = 0.5*(out_flap_span_in() + out_flap_span_out());
          out_flap_span_in  = split_val;
          out_flap_span_out = split_val;
        }
      out_flap_surf.set_num_xsecs(2);
      out_flap_surf.set_num_pnts(2);
      pnt1 = mwing_surf.get_pnt(aft_id, 0);
      pnt2 = mwing_surf.linear_interpolate_xsec(aft_id, out_flap_chord() );
      pnt3 = mwing_surf.get_pnt(4, 0);
      pnt4 = mwing_surf.linear_interpolate_xsec(4, out_flap_chord());
      out_flap_surf.set_pnt(0, 0, pnt1 + (pnt3 - pnt1)*out_flap_span_in());
      out_flap_surf.set_pnt(0, 1, pnt2 + (pnt4 - pnt2)*out_flap_span_in());
      out_flap_surf.set_pnt(1, 0, pnt1 + (pnt3 - pnt1)*out_flap_span_out());
      out_flap_surf.set_pnt(1, 1, pnt2 + (pnt4 - pnt2)*out_flap_span_out());
      out_flap_surf.load_refl_pnts_xsecs();
    }

  if ( in_slat_type == MW_NO_SLAT )
    {
      in_slat_surf.set_num_xsecs(0);
      in_slat_surf.set_num_pnts(0);
    }
  else
    {
      if ( in_slat_span_in() > in_slat_span_out() )
        {
          double split_val  = 0.5*(in_slat_span_in() + in_slat_span_out());
          in_slat_span_in  = split_val;
          in_slat_span_out = split_val;
        }
      in_slat_surf.set_num_xsecs(2);
      in_slat_surf.set_num_pnts(2);
      int num_pnts = mwing_surf.get_num_pnts();
      pnt1 = mwing_surf.get_pnt(1, num_pnts/2 );
      pnt2 = mwing_surf.linear_interpolate_xsec(1, 1.0 - in_slat_chord() );
      pnt3 = mwing_surf.get_pnt(strake_id, num_pnts/2);
      pnt4 = mwing_surf.linear_interpolate_xsec(strake_id, 1.0 - in_slat_chord());
      in_slat_surf.set_pnt(0, 0, pnt1 + (pnt3 - pnt1)*in_slat_span_in());
      in_slat_surf.set_pnt(0, 1, pnt2 + (pnt4 - pnt2)*in_slat_span_in());
      in_slat_surf.set_pnt(1, 0, pnt1 + (pnt3 - pnt1)*in_slat_span_out());
      in_slat_surf.set_pnt(1, 1, pnt2 + (pnt4 - pnt2)*in_slat_span_out());
      in_slat_surf.load_refl_pnts_xsecs();
    }
  if ( out_slat_type == MW_NO_SLAT )
    {
      out_slat_surf.set_num_xsecs(0);
      out_slat_surf.set_num_pnts(0);
    }
  else
    {
      if ( out_slat_span_in() > out_slat_span_out() )
        {
          double split_val  = 0.5*(out_slat_span_in() + out_slat_span_out());
          out_slat_span_in  = split_val;
          out_slat_span_out = split_val;
        }
      out_slat_surf.set_num_xsecs(2);
      out_slat_surf.set_num_pnts(2);
      int num_pnts = mwing_surf.get_num_pnts();
      pnt1 = mwing_surf.get_pnt(strake_id, num_pnts/2);
      pnt2 = mwing_surf.linear_interpolate_xsec(strake_id, 1.0 - out_slat_chord() );
      pnt3 = mwing_surf.get_pnt(4, num_pnts/2);
      pnt4 = mwing_surf.linear_interpolate_xsec(4, 1.0 - out_slat_chord());
      out_slat_surf.set_pnt(0, 0, pnt1 + (pnt3 - pnt1)*out_slat_span_in());
      out_slat_surf.set_pnt(0, 1, pnt2 + (pnt4 - pnt2)*out_slat_span_in());
      out_slat_surf.set_pnt(1, 0, pnt1 + (pnt3 - pnt1)*out_slat_span_out());
      out_slat_surf.set_pnt(1, 1, pnt2 + (pnt4 - pnt2)*out_slat_span_out());
      out_slat_surf.load_refl_pnts_xsecs();
    }


}

//==== Update Bounding Box =====//
void WingGeom::update_bbox()
{
  vec3d pnt;
  bbox new_box;

  int num_pnts = mwing_surf.get_num_pnts();

  //==== Check Some Inlet And Nozzle Xsecs ====//
  for ( int j = 0 ; j < num_pnts ; j++ )
    {
      pnt = mwing_surf.get_pnt(1,j);            new_box.update(pnt);
      pnt = mwing_surf.get_pnt(2,j);            new_box.update(pnt);
      pnt = mwing_surf.get_pnt(3,j);            new_box.update(pnt);
      pnt = mwing_surf.get_pnt(4,j);            new_box.update(pnt);
    }

  bnd_box = new_box;
  update_xformed_bbox();			// Load Xform BBox
}

void WingGeom::draw()
{
	if ( fastDrawFlag )
	{
		mwing_surf.fast_draw_on();
		in_flap_surf.fast_draw_on();
		out_flap_surf.fast_draw_on();
		in_slat_surf.fast_draw_on();
		out_slat_surf.fast_draw_on();
	}
	else
	{
		mwing_surf.fast_draw_off();
		in_flap_surf.fast_draw_off();
		out_flap_surf.fast_draw_off();
		in_slat_surf.fast_draw_off();
		out_slat_surf.fast_draw_off();
	}

	//==== Draw Highlighting Boxes ====//
	draw_highlight_boxes();

	//==== Check Noshow Flag ====//
	if ( noshowFlag ) return;	

	if ( displayFlag == GEOM_WIRE_FLAG )
	{
		glColor3ub( (int)color.x(), (int)color.y(), (int)color.z() );	

		//==== Draw Geom ====//
		glPushMatrix();
		glMultMatrixf((GLfloat*)model_mat); 
		mwing_surf.draw_wire();
		in_flap_surf.draw_wire();
		out_flap_surf.draw_wire();
		in_slat_surf.draw_wire();
		out_slat_surf.draw_wire();
		glPopMatrix();

		//==== Reflected Geom ====//
		glPushMatrix();
		glMultMatrixf((GLfloat*)reflect_mat); 
		mwing_surf.draw_refl_wire(sym_code);
		in_flap_surf.draw_refl_wire(sym_code);
		out_flap_surf.draw_refl_wire(sym_code);
		in_slat_surf.draw_refl_wire(sym_code);
		out_slat_surf.draw_refl_wire(sym_code);
		glPopMatrix();
	}
	else if ( displayFlag == GEOM_SHADE_FLAG )
	{
		//==== Draw Geom ====//
		glPushMatrix();
		glMultMatrixf((GLfloat*)model_mat); 

		Material* mat = matMgrPtr->getMaterial( materialID );
		if ( mat )
		{
			mat->bind();
			if ( mat->diff[3] > 0.99 )
			{
				mwing_surf.draw_shaded();
			}
		}
		glPopMatrix();

		//==== Reflected Geom ====//
		glPushMatrix();
		glMultMatrixf((GLfloat*)reflect_mat); 
		mat = matMgrPtr->getMaterial( materialID );
		if ( mat )
		{
			mat->bind();
			if  ( mat->diff[3] > 0.99 )
				mwing_surf.draw_refl_shaded( sym_code);
		}
		glPopMatrix();
	}
	else if ( displayFlag == GEOM_HIDDEN_FLAG )
	{
		//==== Draw Hidden Surface ====//
		glColor3ub( (int)color.x(), (int)color.y(), (int)color.z() );	

		//==== Draw Geom ====//
		glPushMatrix();
		glMultMatrixf((GLfloat*)model_mat); 
		mwing_surf.draw_hidden();
		glPopMatrix();

		//==== Reflected Geom ====//
		glColor3ub( (int)color.x(), (int)color.y(), (int)color.z() );	
		glPushMatrix();
		glMultMatrixf((GLfloat*)reflect_mat); 
		mwing_surf.draw_refl_hidden(sym_code);
		glPopMatrix();
			
	}
}

//==== Draw If Alpha < 1 and Shaded ====//
void WingGeom::drawAlpha()
{
	Material* mat = matMgrPtr->getMaterial( materialID );
	if ( !mat )
		return;

	if ( mat->diff[3] > 0.99 )
		return;
	
	if ( displayFlag == GEOM_SHADE_FLAG )
	{
		//==== Draw Geom ====//
		glPushMatrix();
		glMultMatrixf((GLfloat*)model_mat); 

		mat->bind();
		mwing_surf.draw_shaded();

		glPopMatrix();

		//==== Reflected Geom ====//
		glPushMatrix();
		glMultMatrixf((GLfloat*)reflect_mat); 

		mat->bind();
		mwing_surf.draw_refl_shaded( sym_code );

		glPopMatrix();
	}

}
//==== Set Strake and Aft On/Off Flag ====//
void WingGeom::set_strake_aft_flag(int flag_in)
{
  strake_aft_flag = flag_in;

  if ( strake_aft_flag )
    {
      strake_sweep.activate();
      aft_ext_sweep.activate();
    }
  else
    {
      strake_sweep.deactivate();
      aft_ext_sweep.deactivate();
      generate();
	
	  //==== Tell Aircraft You Have Changed ====//
	  airPtr->geomMod( this );
    }
}


//==== Compute And Load Normals ====//
void WingGeom::load_normals()
{
  mwing_surf.load_sharp_normals();
}

//==== Draw Other Pnts XSecs====//
void WingGeom::load_hidden_surf()
{
  mwing_surf.load_hidden_surf();
}

double WingGeom::get_area()
{
  return( area() );
}


double WingGeom::get_span()
{
  return( span() );

}

double WingGeom::get_cbar()
{
  double m_a_c = 2.0/3.0*root_chord()*(1.0+taper()+taper()*taper())/(1.0+taper());
  return( m_a_c );
}

//==== Calculate Absolute X Location of Quarter Mean Aerodynmaic Chord =====//
double WingGeom::get_x_quarter_cbar()
{
  vec3d v = getTotalTranVec();

  double x_q_cbar = 0.0;
  double tan_qc_sweep = get_tan_sweep_at(0.25);
  if ( getSymCode() == XZ_SYM &&  v.y() == 0.0 )
    x_q_cbar = 0.5*root_chord()*aspect()/span()*( (1.0+taper())*( v.x()+0.25*root_chord()) + (1.0+2.0*taper())*span()/6.0*tan_qc_sweep );
  else
    x_q_cbar = 0.5*root_chord()*aspect()/span()*( (1.0+taper())*( v.x()+0.25*root_chord()) + (1.0+2.0*taper())*span()/3.0*tan_qc_sweep );

  return ( x_q_cbar );

}

//==== Calculate Inboard Chord (Root Chord + Strake and Aft Extension Portions) =====//
double WingGeom::get_inboard_chord_length()
{
  double in_chord;

  //==== Main Wing Parms ====//
  double ty      = span();                       // Tip Y Offset
  if ( getSymCode() == XZ_SYM )  ty *= 0.5;   // Check Sym Code

  //==== Strake Parms ====//
  double sexp    = (tan(strake_sweep()*DEG_2_RAD) - get_tan_sweep_at(0.0))*ty*strake_span_per();                         // Exposed X Strake

  //==== Aft Ext Parms ====//
  double aexp    = (tan(-aft_ext_sweep()*DEG_2_RAD) + get_tan_sweep_at(1.0))*ty*aft_ext_span_per();                              // Aft Exposed X

  in_chord      = root_chord() + sexp + aexp;                   // Total Root Chord

  return(in_chord);
}

//==== Calculate X location of Strake Root Chord at the Leading Edge =====//
double WingGeom::get_x_le_root()
{
  //===== Determine Strake Break Location ====
  double ty      = span();                       // Tip Y Offset
  if ( getSymCode() == XZ_SYM )  ty *= 0.5;   // Check Sym Code

  double sexp    = (tan(strake_sweep()*DEG_2_RAD) - get_tan_sweep_at(0.0))*ty*strake_span_per();                         // Exposed X Strake

//  double x_le_root = xx_loc() - sexp;
  vec3d v = getTotalTranVec();
  double x_le_root = v.x() - sexp;

  return(x_le_root);
}

//==== Calculate Tan Sweep at Specified Chord Location =====//
double WingGeom::get_tan_sweep_at(double per_chord_loc)
{
  double tan_sweep = tan(sweep()*DEG_2_RAD);

  double asp     = aspect();                             // Aspect Ratio
  if ( getSymCode() != XZ_SYM )  asp *= 2.0;          // Check Sym Code

  double tan_sweep_at = tan_sweep - (4.0/asp)*
                ( (per_chord_loc-old_sweep_loc) * ((1.0-taper())/(1.0+taper())) );

  return ( tan_sweep_at );

}

//==== Calculate Sweep at Specified Chord Location (Radians) =====//
double WingGeom::get_sweep_at(double per_chord_loc)
{
  double sweep_at = atan( get_tan_sweep_at(per_chord_loc) );

  return(sweep_at);
}

//==== Put Sym Code ====//
void WingGeom::setSymCode(int in_sym)
{
  //jrg need this??
  if (in_sym != XZ_SYM && sym_code == XZ_SYM)
    {
      span = span()*0.5;
      area = area()*0.5;
      aspect = aspect()*0.5;
      total_area = total_area()*0.5;
    }
  else if ( in_sym == XZ_SYM && sym_code != XZ_SYM)
    {
      span = span()*2.0;
      area = area()*2.0;
      aspect = aspect()*2.0;
      total_area = total_area()*2.0;
    }
  if (in_sym == NO_SYM)
    {
      sym_code = in_sym;
      sym_vec = vec3d(1.0, 1.0, 1.0);
    }
  else if (in_sym == XY_SYM)
    {
      sym_code = in_sym;
      sym_vec = vec3d(1.0, 1.0, -1.0);
    }
  else if (in_sym == XZ_SYM)
    {
      sym_code = in_sym;
      sym_vec = vec3d(1.0,  -1.0, 1.0);
    }
  else if (in_sym == YZ_SYM)
    {
      sym_code = in_sym;
      sym_vec = vec3d(-1.0, 1.0, 1.0);
    }
  compose_model_matrix();

  //==== Tell Aircraft You Have Changed ====//
  airPtr->geomMod( this );


}

//===== Set Deflect Flag =====//
void WingGeom::set_deflect_flag(int flag_in)
{
  deflect_flag = flag_in;

  generate();

//main_window_ptr->draw_all_gl_windows();
	
  //==== Tell Aircraft You Have Changed ====//
  airPtr->geomMod( this );

}


//===== Read Deflection File =====//
int WingGeom::read_deflect_file( Stringc file_name )
{
  Stringc line;
  char buff[255];

  /* --- Open file --- */
  FILE* file_id = fopen(file_name, "r");

  if (file_id == (FILE *)NULL) return(0);

  fgets(buff, 80, file_id);  line = buff;

  if (line.search_for_substring("DEFLECTION FILE") < 0)
    {
      return(0);
    }
  deflect_file_name = file_name;

  fgets(buff, 80, file_id);
  deflect_name = buff;

  deflect_name.remove_trailing_blanks();
  deflect_name.remove_all_substrings('\n');

  int num_deflect_pnts;
  fscanf(file_id, "%d\n", &num_deflect_pnts);
  fgets(buff, 80, file_id);

  deflect_curve.init( num_deflect_pnts );
  twist_curve.init( num_deflect_pnts );

  float span, deflect, twist;
  for ( int i = 0 ; i < num_deflect_pnts ; i++ )
    {

      fscanf(file_id, "%f %f %f\n", &span, &deflect, &twist);
      deflect_curve.load_pnt(  i, vec3d(span, deflect, 0.0));
      twist_curve.load_pnt(    i, vec3d(span, twist, 0.0));

    }

  fclose(file_id);

  deflect_curve.comp_tans();
  twist_curve.comp_tans();

  generate();

  return 1;
}


//==== Convert To Tri Mesh ====//
vector< TMesh* > WingGeom:: createTMeshVec()
{
	vector< TMesh* > tMeshVec;

	TMesh* tmPtr = mwing_surf.createTMesh(model_mat);
	tMeshVec.push_back( tmPtr );

	if ( sym_code != NO_SYM )
	{
		tmPtr = mwing_surf.createReflTMesh(sym_code, reflect_mat);
		tMeshVec.push_back( tmPtr );
	}
	return tMeshVec;
}



//==== Dump Xsec File ====//
void WingGeom::dump_xsec_file(int geom_no, FILE* dump_file)
{
  int i;

  fprintf(dump_file, "\n");
  fprintf(dump_file, "%s \n", (char*) getName() );
  fprintf(dump_file, " GROUP NUMBER      = %d \n",geom_no);
  fprintf(dump_file, " TYPE              = 0  \n");				// Lifting
  fprintf(dump_file, " CROSS SECTIONS    = %d \n",4);
  fprintf(dump_file, " PTS/CROSS SECTION = %d \n",mwing_surf.get_num_pnts());

  for ( i = 1 ; i < 5 ; i++ )
    {
      mwing_surf.write_xsec(i, model_mat, dump_file);
    }

  if ( sym_code == NO_SYM ) return;

  fprintf(dump_file, "\n");
  fprintf(dump_file, "%s \n", (char*) getName() );
  fprintf(dump_file, " GROUP NUMBER      = %d \n",geom_no);
  fprintf(dump_file, " TYPE              = 0  \n");
  fprintf(dump_file, " CROSS SECTIONS    = %d \n",4);
  fprintf(dump_file, " PTS/CROSS SECTION = %d \n",mwing_surf.get_num_pnts());

  for ( i = 1 ; i < 5 ; i++ )
    {
      mwing_surf.write_refl_xsec(sym_code, i, reflect_mat, dump_file );
    }
}

//==== Return Number Of Felisa Surface Written Out ====//
int WingGeom::get_num_felisa_comps()
{
  vec3d v = getTotalTranVec();

  if (  v.y() > 0.0 )
    return(8);
  else if ( sym_code == XZ_SYM )
    return(7);
  else
    return(5);
}

//==== Return Number Of Felisa Surface Written Out ====//
int WingGeom::get_num_felisa_wings()
{
  vec3d v = getTotalTranVec();

  if (  v.y() == 0.0 && sym_code == NO_SYM )
    return(0);
  else
    return(1);
}


//==== Write Out Felisa Surfaces To File ====//
int WingGeom::write_felisa_file(int geom_no, FILE* dump_file)
{
  int num_geoms = geom_no;
  int j;
  int num_pnts = mwing_surf.get_num_pnts()/2 + 1;
  int num_slices;
  vec3d temp;
  vec3d te_pnt1 = mwing_surf.get_pnt(1,0).transform(model_mat);
  vec3d te_pnt2 = mwing_surf.get_pnt(2,0).transform(model_mat);
  vec3d te_pnt3 = mwing_surf.get_pnt(3,0).transform(model_mat);
  vec3d te_pnt4 = mwing_surf.get_pnt(4,0).transform(model_mat);
  double span;
  double spandist[4];

  vec3d v = getTotalTranVec();

  if ( aft_ext_span_per() < strake_span_per() ) {

     spandist[1] =                          aft_ext_span_per();
     spandist[2] =      strake_span_per() - aft_ext_span_per();
     spandist[3] = 1. - strake_span_per();

  }
  else {

     spandist[1] =                          strake_span_per();
     spandist[2] =      aft_ext_span_per() - strake_span_per();
     spandist[3] = 1. - aft_ext_span_per();

  }

  span = 1.;
  int isec;
  for ( isec = 1 ; isec < 4 ; isec++ )
    {
      //==== Write Lower Wing Surfaces ====//
      fprintf(dump_file, "\n");
      fprintf(dump_file, "%d                    Surface_Number\n", num_geoms);
      num_geoms++;
      fprintf(dump_file, "%s-Lower-%d           Name\n", (char*)getName(),isec);
      num_slices = (int)(0.5*MAX(1.,get_aspect()->get())*num_pnts*(spandist[isec]/span));
      num_slices = MAX(num_slices,20);
      fprintf(dump_file, "%d                    U_Render\n",num_slices);
      fprintf(dump_file, "2                     W_Render\n");
      fprintf(dump_file, "2                     Num_xsecs\n");
      fprintf(dump_file, "%d                    Num_pnts\n", num_pnts);

      for ( j = num_pnts-1 ; j < mwing_surf.get_num_pnts() ; j++ )
        {
          temp = mwing_surf.get_pnt( isec, j).transform(model_mat);
          fprintf(dump_file, "%11.6f  %11.6f  %11.6f\n", temp.x(), temp.y(),temp.z());
        }
      for ( j = num_pnts-1 ; j < mwing_surf.get_num_pnts() ; j++ )
        {
          temp = mwing_surf.get_pnt( isec+1, j).transform(model_mat);
          fprintf(dump_file, "%11.6f  %11.6f  %11.6f\n", temp.x(), temp.y(),temp.z());
        }
    }

  //==== Check For Vertical Tail Config ====//
  if (  v.y() <= 0.0 && sym_code == NO_SYM )
    {
      //===== Write Top End Cap (Half) =====//
      fprintf(dump_file, "\n");
      fprintf(dump_file, "%d            Surface_Number\n", num_geoms);
      num_geoms++;
      fprintf(dump_file, "%s            Name\n", (char*) getName());
      num_slices = (int)(0.5*MAX(1.,get_aspect()->get())*num_pnts*(spandist[isec]/span));
      num_slices = MAX(num_slices,20);
      fprintf(dump_file, "%d                    U_Render\n",num_slices);
      fprintf(dump_file, "2                     W_Render\n");
      fprintf(dump_file, "2                     Num_xsecs\n");
      fprintf(dump_file, "%d            Num_pnts\n", num_pnts);

      for ( j = num_pnts-1 ; j < mwing_surf.get_num_pnts() ; j++ )
        {
          temp =  mwing_surf.get_pnt(1,j).transform(model_mat);
          fprintf(dump_file, "%11.6f  %11.6f  %11.6f\n", temp.x(), temp.y(), temp.z());
        }

      for ( j = num_pnts-1 ; j < mwing_surf.get_num_pnts() ; j++ )
        {
          temp =  mwing_surf.get_pnt(1,j).transform(model_mat);
          fprintf(dump_file, "%11.6f   0.0      %11.6f\n", temp.x(), temp.z());
        }

      //===== Write Bottom End Cap (Half) =====//
      fprintf(dump_file, "\n");
      fprintf(dump_file, "%d            Surface_Number\n", num_geoms);
      num_geoms++;
      fprintf(dump_file, "%s            Name\n", (char*) getName());
      fprintf(dump_file, "2                     U_Render\n");
      fprintf(dump_file, "2                     W_Render\n");
      fprintf(dump_file, "2                     Num_xsecs\n");
      fprintf(dump_file, "%d            Num_pnts\n", num_pnts);

      for ( j = num_pnts-1 ; j < mwing_surf.get_num_pnts() ; j++ )
        {
          temp = mwing_surf.get_pnt(4,j).transform(model_mat);
          fprintf(dump_file, "%11.6f  %11.6f  %11.6f\n", temp.x(), temp.y(), temp.z());
        }

      for ( j = num_pnts-1 ; j < mwing_surf.get_num_pnts() ; j++ )
        {
          temp = mwing_surf.get_pnt(4,j).transform(model_mat);
          fprintf(dump_file, "%11.6f   0.0      %11.6f\n", temp.x(), temp.z());
        }
    }
  else
    {
      for (  isec = 1 ; isec < 4 ; isec++ )
        {
          //==== Write Upper Wing Surfaces ====//
          fprintf(dump_file, "\n");
          fprintf(dump_file, "%d                        Surface_Number\n", num_geoms);
          num_geoms++;
          fprintf(dump_file, "%s-Upper-%d               Name\n", (char*) getName(),isec);
          num_slices = (int)(0.5*MAX(1.,get_aspect()->get())*num_pnts*(spandist[isec]/span));
          num_slices = MAX(num_slices,20);
          fprintf(dump_file, "%d                U_Render\n",num_slices);
          fprintf(dump_file, "2                 W_Render\n");
          fprintf(dump_file, "2                 Num_xsecs\n");
          fprintf(dump_file, "%d                        Num_pnts\n", num_pnts);

          for ( j = 0 ; j < num_pnts ; j++ )
            {
              temp = mwing_surf.get_pnt( isec, j).transform(model_mat);
              fprintf(dump_file, "%11.6f  %11.6f  %11.6f\n", temp.x(), temp.y(),temp.z());
            }
          for ( j = 0 ; j < num_pnts ; j++ )
            {
              temp = mwing_surf.get_pnt( isec+1, j).transform(model_mat);
              fprintf(dump_file, "%11.6f  %11.6f  %11.6f\n", temp.x(), temp.y(),temp.z());
            }
        }

      //==== Write Outer End Cap ====//
      fprintf(dump_file, "\n");
      fprintf(dump_file, "%d            Surface_Number\n", num_geoms);
      num_geoms++;
      fprintf(dump_file, "%s            Name\n", (char*) getName());
      fprintf(dump_file, "2             U_Render\n");
      fprintf(dump_file, "2             W_Render\n");
      fprintf(dump_file, "2             Num_xsecs\n");
      fprintf(dump_file, "%d            Num_pnts\n", num_pnts);

      for ( j = 0 ; j < num_pnts ; j++ )
        {
          temp = mwing_surf.get_pnt(4,j).transform(model_mat);
          fprintf(dump_file, "%11.6f  %11.6f  %11.6f\n", temp.x(), temp.y(), temp.z());
        }
      for ( j = mwing_surf.get_num_pnts()-1 ; j >= num_pnts-1 ; j-- )
        {
          temp = mwing_surf.get_pnt(4,j).transform(model_mat);
          fprintf(dump_file, "%11.6f  %11.6f  %11.6f\n", temp.x(), temp.y(), temp.z());
        }

      if (  v.y() > 0.0 )
        {
          //==== Write Inner End Cap ====//
          fprintf(dump_file, "\n");
          fprintf(dump_file, "%d                Surface_Number\n", num_geoms);
          num_geoms++;
          fprintf(dump_file, "%s                Name\n", (char*) getName());
          fprintf(dump_file, "2                 U_Render\n");
          fprintf(dump_file, "2                 W_Render\n");
          fprintf(dump_file, "2                 Num_xsecs\n");
          fprintf(dump_file, "%d                Num_pnts\n", num_pnts);

          for ( j = 0 ; j < num_pnts ; j++ )
            {
              temp = mwing_surf.get_pnt(1,j).transform(model_mat);
              fprintf(dump_file, "%11.6f  %11.6f  %11.6f\n", temp.x(), temp.y(), temp.z());
            }
          for ( j = mwing_surf.get_num_pnts()-1 ; j >= num_pnts-1 ; j-- )
            {
              temp = mwing_surf.get_pnt(1,j).transform(model_mat);
              fprintf(dump_file, "%11.6f  %11.6f  %11.6f\n", temp.x(), temp.y(), temp.z());
            }
        }


    }

 return(num_geoms);

}


//==== Write Out Felisa Surfaces To File ====//
void WingGeom::write_bco_info(FILE* file_ptr, int& surf_cnt, int& wing_cnt)
{
  vec3d v = getTotalTranVec();

  //==== Check For Vertical Tail Config ====//
  if (  v.y() == 0.0 && sym_code == NO_SYM )
    {
      fprintf( file_ptr, "%d        0      0     0.0     0.0     0.0     0.0   %s Surf 1\n", surf_cnt, (char*)getName() );
      surf_cnt++;
      fprintf( file_ptr, "%d        0      0     0.0     0.0     0.0     0.0   %s Surf 2\n", surf_cnt, (char*)getName() );
      surf_cnt++;
      fprintf( file_ptr, "%d        0      0     0.0     0.0     0.0     0.0   %s Surf 3\n", surf_cnt, (char*)getName() );
      surf_cnt++;
      fprintf( file_ptr, "%d        0      0     0.0     0.0     0.0     0.0   %s End Cap 1\n", surf_cnt, (char*)getName() );
      surf_cnt++;
      fprintf( file_ptr, "%d        0      0     0.0     0.0     0.0     0.0   %s End Cap 2\n", surf_cnt, (char*)getName() );
      surf_cnt++;
    }
  else if (  v.y() == 0.0 )
    {
      fprintf( file_ptr, "%d        0    -%d     0.0     0.0     0.0     0.0   %s Lower Surf 1\n", surf_cnt, wing_cnt, (char*)getName() );
      surf_cnt++;
      fprintf( file_ptr, "%d        0    -%d     0.0     0.0     0.0     0.0   %s Lower Surf 2\n", surf_cnt, wing_cnt, (char*)getName() );
      surf_cnt++;
      fprintf( file_ptr, "%d        0    -%d     0.0     0.0     0.0     0.0   %s Lower Surf 3\n", surf_cnt, wing_cnt, (char*)getName() );
      surf_cnt++;
      fprintf( file_ptr, "%d        0     %d     0.0     0.0     0.0     0.0   %s Upper Surf 1\n", surf_cnt, wing_cnt, (char*)getName() );
      surf_cnt++;
      fprintf( file_ptr, "%d        0     %d     0.0     0.0     0.0     0.0   %s Upper Surf 2\n", surf_cnt, wing_cnt, (char*)getName() );
      surf_cnt++;
      fprintf( file_ptr, "%d        0     %d     0.0     0.0     0.0     0.0   %s Upper Surf 3\n", surf_cnt, wing_cnt, (char*)getName() );
      surf_cnt++;
      fprintf( file_ptr, "%d        0      0     0.0     0.0     0.0     0.0   %s End Cap 1\n", surf_cnt, (char*)getName() );
      surf_cnt++;

      wing_cnt++;
    }
  else
    {
      fprintf( file_ptr, "%d        0    -%d     0.0     0.0     0.0     0.0   %s Lower Surf 1\n", surf_cnt, wing_cnt, (char*)getName() );
      surf_cnt++;
      fprintf( file_ptr, "%d        0    -%d     0.0     0.0     0.0     0.0   %s Lower Surf 2\n", surf_cnt, wing_cnt, (char*)getName() );
      surf_cnt++;
      fprintf( file_ptr, "%d        0    -%d     0.0     0.0     0.0     0.0   %s Lower Surf 3\n", surf_cnt, wing_cnt, (char*)getName() );
      surf_cnt++;
      fprintf( file_ptr, "%d        0     %d     0.0     0.0     0.0     0.0   %s Upper Surf 1\n", surf_cnt, wing_cnt, (char*)getName() );
      surf_cnt++;
      fprintf( file_ptr, "%d        0     %d     0.0     0.0     0.0     0.0   %s Upper Surf 2\n", surf_cnt, wing_cnt, (char*)getName() );
      surf_cnt++;
      fprintf( file_ptr, "%d        0     %d     0.0     0.0     0.0     0.0   %s Upper Surf 3\n", surf_cnt, wing_cnt, (char*)getName() );
      surf_cnt++;
      fprintf( file_ptr, "%d        0      0     0.0     0.0     0.0     0.0   %s End Cap 1\n", surf_cnt, (char*)getName() );
      surf_cnt++;
      fprintf( file_ptr, "%d        0      0     0.0     0.0     0.0     0.0   %s End Cap 1\n", surf_cnt, (char*)getName() );
      surf_cnt++;

      wing_cnt++;
    }

}


//==== Write Out Felisa Background Lines File ====//
void WingGeom::write_felisa_line_sources(FILE* dump_file)
{
  //==== Find Leading Edge And Trailing Edge ====//
  vec3d te_pnt1 = mwing_surf.get_pnt(1,0).transform(model_mat);
  vec3d te_pnt2 = mwing_surf.get_pnt(2,0).transform(model_mat);
  vec3d te_pnt3 = mwing_surf.get_pnt(3,0).transform(model_mat);
  vec3d te_pnt4 = mwing_surf.get_pnt(4,0).transform(model_mat);
  int half_ind = mwing_surf.get_num_pnts()/2;
  vec3d le_pnt1 = mwing_surf.get_pnt(1,half_ind).transform(model_mat);
  vec3d le_pnt2 = mwing_surf.get_pnt(2,half_ind).transform(model_mat);
  vec3d le_pnt3 = mwing_surf.get_pnt(3,half_ind).transform(model_mat);
  vec3d le_pnt4 = mwing_surf.get_pnt(4,half_ind).transform(model_mat);
  double span =  get_span();
  double ds_root_le, ds_tip_le, ds_mid1_le, ds_mid2_le;
  double ds_root_te, ds_tip_te, ds_mid1_te, ds_mid2_te;
  double max_root_thick = dist(le_pnt1,te_pnt1)*(root_foil->get_thickness()->get());
  double max_tip_thick = dist(le_pnt4,te_pnt4)*(tip_foil->get_thickness()->get());
  double mid1_chord;
  double mid2_chord;
//   double max_mid1_thick;
//   double max_mid2_thick;

  //==== Find Max Foil Thickness  and chords ===//

  double root_chord = dist(le_pnt1,te_pnt1);
  double tip_chord  = dist(le_pnt4,te_pnt4);

  if ( aft_ext_span_per() < strake_span_per() ) {

     mid1_chord = dist(le_pnt2,te_pnt2);
     mid2_chord = dist(le_pnt3,te_pnt3);

//      max_mid1_thick = mid1_chord*(aft_foil->get_thickness()->get());
//      max_mid2_thick = mid2_chord*(strake_foil->get_thickness()->get());

  }
  else {

     mid1_chord = dist(le_pnt2,te_pnt2);
     mid2_chord = dist(le_pnt3,te_pnt3);

//      max_mid1_thick = mid1_chord*(strake_foil->get_thickness()->get());
//      max_mid2_thick = mid2_chord*(aft_foil->get_thickness()->get());

  }

  /* leading edge */

  ds_root_le = root_chord/40.;
  ds_tip_le  = MAX(tip_chord/root_chord,0.50)*ds_root_le;
  ds_mid1_le = MAX(mid1_chord/root_chord,0.50)*ds_root_le;
  ds_mid2_le = MAX(mid2_chord/root_chord,0.50)*ds_root_le;

  /* trailing edge */

  ds_root_te = root_chord/30.;
  ds_tip_te  = MAX(tip_chord/root_chord,0.50)*ds_root_te;
  ds_mid1_te = MAX(mid1_chord/root_chord,0.50)*ds_root_te;
  ds_mid2_te = MAX(mid2_chord/root_chord,0.50)*ds_root_te;

  /* limit spacing */

  ds_root_le = MAX(ds_root_le,span/1000.);
  ds_tip_le  = MAX(ds_tip_le, span/1000.);
  ds_mid1_le = MAX(ds_mid1_le,span/1000.);
  ds_mid2_le = MAX(ds_mid2_le,span/1000.);

  /* trailing edge */

  ds_root_te = MAX(ds_root_te,span/1000.);
  ds_tip_te  = MAX(ds_tip_te, span/1000.);
  ds_mid1_te = MAX(ds_mid1_te,span/1000.);
  ds_mid2_te = MAX(ds_mid2_te,span/1000.);

  span = MAX(MAX(0.5*span,root_chord),tip_chord);

  // Note, changing the spacing control from
  // ds_root_te, 2.*ds_root_te ,9.*ds_root_te); TO:
  // ds_root_te, 4.*ds_root_te ,9.*ds_root_te);
  //            ***
  // for the trailing edge line

  //==== Write Out Line Along Trailing Edge ====//
  fprintf(dump_file, " %s: Trailing Edge Line 1 \n", (char*) getName());
  fprintf(dump_file, "    %f    %f    %f    %f    %f    %f  \n",
      te_pnt1.x(), te_pnt1.y(), te_pnt1.z(),
      ds_root_te, 2.*ds_root_te ,9.*ds_root_te);
  fprintf(dump_file, "    %f    %f    %f    %f    %f    %f  \n",
      te_pnt2.x(), te_pnt2.y(), te_pnt2.z(),
      ds_mid1_te, 2.*ds_mid1_te, 9.*ds_mid1_te);

  fprintf(dump_file, " %s: Trailing Edge Line 2 \n", (char*) getName());
  fprintf(dump_file, "    %f    %f    %f    %f    %f    %f  \n",
      te_pnt2.x(), te_pnt2.y(), te_pnt2.z(),
      ds_mid1_te, 2.*ds_mid1_te, 9.*ds_mid1_te);
  fprintf(dump_file, "    %f    %f    %f    %f    %f    %f  \n",
      te_pnt3.x(), te_pnt3.y(), te_pnt3.z(),
      ds_mid2_te, 2.*ds_mid2_te, 9.*ds_mid2_te);

  fprintf(dump_file, " %s: Trailing Edge Line 3 \n", (char*) getName());
  fprintf(dump_file, "    %f    %f    %f    %f    %f    %f  \n",
      te_pnt3.x(), te_pnt3.y(), te_pnt3.z(),
      ds_mid2_te, 2.*ds_mid2_te, 9.*ds_mid2_te);
  fprintf(dump_file, "    %f    %f    %f    %f    %f    %f  \n",
      te_pnt4.x(), te_pnt4.y(), te_pnt4.z(),
      ds_tip_te,  2.*ds_tip_te,  9.*ds_tip_te );


  //==== Write Out Line Along Leading Edge ====//
  fprintf(dump_file, " %s: Leading Edge Line 1 \n", (char*) getName());
  fprintf(dump_file, "    %f    %f    %f    %f    %f    %f  \n",
      le_pnt1.x(), le_pnt1.y(), le_pnt1.z(),
      ds_root_le, 2.*ds_root_le, 9.*ds_root_le);
  fprintf(dump_file, "    %f    %f    %f    %f    %f    %f  \n",
      le_pnt2.x(), le_pnt2.y(), le_pnt2.z(),
      ds_mid1_le, 2.*ds_mid1_le, 9.*ds_mid1_le);

  fprintf(dump_file, " %s: Leading Edge Line 2 \n", (char*) getName());
  fprintf(dump_file, "    %f    %f    %f    %f    %f    %f  \n",
      le_pnt2.x(), le_pnt2.y(), le_pnt2.z(),
      ds_mid1_le, 2.*ds_mid1_le, 9.*ds_mid1_le);
  fprintf(dump_file, "    %f    %f    %f    %f    %f    %f  \n",
      le_pnt3.x(), le_pnt3.y(), le_pnt3.z(),
      ds_mid2_le, 2.*ds_mid2_le, 9.*ds_mid2_le);

  fprintf(dump_file, " %s: Leading Edge Line 3 \n", (char*) getName());
  fprintf(dump_file, "    %f    %f    %f    %f    %f    %f  \n",
      le_pnt3.x(), le_pnt3.y(), le_pnt3.z(),
      ds_mid2_le, 2.*ds_mid2_le, 9.*ds_mid2_le);
  fprintf(dump_file, "    %f    %f    %f    %f    %f    %f  \n",
      le_pnt4.x(), le_pnt4.y(), le_pnt4.z(),
      ds_tip_le,  2.*ds_tip_le,  9.*ds_tip_le);

  //==== Write Out Line Along Root Chord ====//
  fprintf(dump_file, " %s: Root Chord Edge Line \n", (char*) getName());
  fprintf(dump_file, "    %f    %f    %f    %f    %f    %f  \n",
      le_pnt1.x(), le_pnt1.y(), le_pnt1.z(),
      ds_root_te, 2.*ds_root_te, 9.*ds_root_te);
  fprintf(dump_file, "    %f    %f    %f    %f    %f    %f  \n",
      te_pnt1.x(), te_pnt1.y(), te_pnt1.z(),
      ds_root_te, 2.*ds_root_te, 9.*ds_root_te);

  //==== Write Out Line Along Tip Chord ====//
  fprintf(dump_file, " %s: Tip Chord Edge Line \n", (char*) getName());
  fprintf(dump_file, "    %f    %f    %f    %f    %f    %f  \n",
      le_pnt4.x(), le_pnt4.y(), le_pnt4.z(),
      ds_tip_te, 2.*ds_tip_te, 9.*ds_tip_te);
  fprintf(dump_file, "    %f    %f    %f    %f    %f    %f  \n",
      te_pnt4.x(), te_pnt4.y(), te_pnt4.z(),
      ds_tip_te, 2.*ds_tip_te, 9.*ds_tip_te);

  //==== Write Out 2nd Line Along Tip Chord ====//
  fprintf(dump_file, " %s: Second Tip Chord Edge Line \n", (char*) getName());
  fprintf(dump_file, "    %f    %f    %f    %f    %f    %f  \n",
      le_pnt4.x(), le_pnt4.y(), le_pnt4.z(),
      span/40., 2.*span/40., 9.*span/40.);
  fprintf(dump_file, "    %f    %f    %f    %f    %f    %f  \n",
      te_pnt4.x(), te_pnt4.y(), te_pnt4.z(),
      span/40., 2.*span/40., 9.*span/40.);

  //==== Write Out Line Down Centerline of Wing ====//
  fprintf(dump_file, " %s: Leading Edge Line 1 \n", (char*) getName());
  fprintf(dump_file, "    %f    %f    %f    %f    %f    %f  \n",
      0.5*(le_pnt1.x()+te_pnt1.x()),
      0.5*(le_pnt1.y()+te_pnt1.y()),
      0.5*(le_pnt1.z()+te_pnt1.z()),
      span/40., 2.*span/40., 9.*span/40.);
  fprintf(dump_file, "    %f    %f    %f    %f    %f    %f  \n",
      0.5*(le_pnt4.x()+te_pnt4.x()),
      0.5*(le_pnt4.y()+te_pnt4.y()),
      0.5*(le_pnt4.z()+te_pnt4.z()),
      span/40., 2.*span/40., 9.*span/40.);

}

//==== Write Out Felisa Background Triangle File ====//
void WingGeom::write_felisa_tri_sources(FILE* dump_file)
{
  //==== Find Leading Edge And Trailing Edge ====//
  vec3d te_pnt1 = mwing_surf.get_pnt(1,0).transform(model_mat);
  vec3d te_pnt2 = mwing_surf.get_pnt(2,0).transform(model_mat);
  vec3d te_pnt3 = mwing_surf.get_pnt(3,0).transform(model_mat);
  vec3d te_pnt4 = mwing_surf.get_pnt(4,0).transform(model_mat);
  int half_ind = mwing_surf.get_num_pnts()/2;
  vec3d le_pnt1 = mwing_surf.get_pnt(1,half_ind).transform(model_mat);
  vec3d le_pnt2 = mwing_surf.get_pnt(2,half_ind).transform(model_mat);
  vec3d le_pnt3 = mwing_surf.get_pnt(3,half_ind).transform(model_mat);
  vec3d le_pnt4 = mwing_surf.get_pnt(4,half_ind).transform(model_mat);
  double ds_root, ds_tip, ds_mid1, ds_mid2;

  double max_root_thick = dist(le_pnt1,te_pnt1)*(root_foil->get_thickness()->get());
  double max_tip_thick = dist(le_pnt4,te_pnt4)*(tip_foil->get_thickness()->get());
  double mid1_chord;
  double mid2_chord;
//   double max_mid1_thick;
//   double max_mid2_thick;

  //==== Find Max Foil Thickness  and chords ===//

  double root_chord = dist(le_pnt1,te_pnt1);
  double tip_chord  = dist(le_pnt4,te_pnt4);

  if ( aft_ext_span_per() < strake_span_per() ) {

     mid1_chord = dist(le_pnt2,te_pnt2);
     mid2_chord = dist(le_pnt3,te_pnt3);

//      max_mid1_thick = mid1_chord*(aft_foil->get_thickness()->get());
//      max_mid2_thick = mid2_chord*(strake_foil->get_thickness()->get());

  }
  else {

     mid1_chord = dist(le_pnt2,te_pnt2);
     mid2_chord = dist(le_pnt3,te_pnt3);

//      max_mid1_thick = mid1_chord*(strake_foil->get_thickness()->get());
//      max_mid2_thick = mid2_chord*(aft_foil->get_thickness()->get());

  }

  /* general wing spacing */

  ds_root = root_chord/30.;
  ds_tip = MAX(tip_chord/root_chord,0.25)*ds_root;
  ds_mid1 = MAX(mid1_chord/root_chord,0.25)*ds_root;
  ds_mid2 = MAX(mid2_chord/root_chord,0.25)*ds_root;


  //==== Write Out Tri Along Trailing Edges ====//
  fprintf(dump_file, " %s: Trailing Edge Tri 1 \n", (char*) getName());
  fprintf(dump_file, "    %f    %f    %f    %f    %f    %f  \n",
      te_pnt1.x(), te_pnt1.y(), te_pnt1.z(),
      ds_root, 3.*ds_root, 9.*ds_root);
  fprintf(dump_file, "    %f    %f    %f    %f    %f    %f  \n",
      te_pnt2.x(), te_pnt2.y(), te_pnt2.z(),
      ds_mid1, 3.*ds_mid1, 9.*ds_mid1);
  fprintf(dump_file, "    %f    %f    %f    %f    %f    %f  \n",
      le_pnt1.x(), le_pnt1.y(), le_pnt1.z(),
      ds_mid1, 3.*ds_mid1, 9.*ds_mid1);

  fprintf(dump_file, " %s: Trailing Edge Tri 2 \n", (char*) getName());
  fprintf(dump_file, "    %f    %f    %f    %f    %f    %f  \n",
      te_pnt2.x(), te_pnt2.y(), te_pnt2.z(),
      ds_mid1, 3.*ds_mid1, 9.*ds_mid1);
  fprintf(dump_file, "    %f    %f    %f    %f    %f    %f  \n",
      te_pnt3.x(), te_pnt3.y(), te_pnt3.z(),
      ds_mid2, 3.*ds_mid2, 9.*ds_mid2);
  fprintf(dump_file, "    %f    %f    %f    %f    %f    %f  \n",
      le_pnt2.x(), le_pnt2.y(), le_pnt2.z(),
      ds_mid1, 3.*ds_mid1, 9.*ds_mid1);

  fprintf(dump_file, " %s: Trailing Edge Tri 3 \n", (char*) getName());
  fprintf(dump_file, "    %f    %f    %f    %f    %f    %f  \n",
      te_pnt3.x(), te_pnt3.y(), te_pnt3.z(),
      ds_mid2, 3.*ds_mid2, 9.*ds_mid2);
  fprintf(dump_file, "    %f    %f    %f    %f    %f    %f  \n",
      te_pnt4.x(), te_pnt4.y(), te_pnt4.z(),
      ds_tip,  3.*ds_tip , 9.*ds_tip );
  fprintf(dump_file, "    %f    %f    %f    %f    %f    %f  \n",
      le_pnt3.x(), le_pnt3.y(), le_pnt3.z(),
      ds_mid2, 3.*ds_mid2, 9.*ds_mid2);

  //==== Write Out Tri Along Leading Edges ====//
  fprintf(dump_file, " %s: Leading Edge Tri 1 \n", (char*) getName());
  fprintf(dump_file, "    %f    %f    %f    %f    %f    %f  \n",
      le_pnt1.x(), le_pnt1.y(), le_pnt1.z(),
      ds_root, 3.*ds_root, 9.*ds_root);
  fprintf(dump_file, "    %f    %f    %f    %f    %f    %f  \n",
      le_pnt2.x(), le_pnt2.y(), le_pnt2.z(),
      ds_mid1, 3.*ds_mid1, 9.*ds_mid1);
  fprintf(dump_file, "    %f    %f    %f    %f    %f    %f  \n",
      te_pnt2.x(), te_pnt2.y(), te_pnt2.z(),
      ds_mid1, 3.*ds_mid1, 9.*ds_mid1);

 fprintf(dump_file, " %s: Trailing Edge Tri 2 \n", (char*) getName());
  fprintf(dump_file, "    %f    %f    %f    %f    %f    %f  \n",
      le_pnt2.x(), le_pnt2.y(), le_pnt2.z(),
      ds_mid1, 3.*ds_mid1, 9.*ds_mid1);
  fprintf(dump_file, "    %f    %f    %f    %f    %f    %f  \n",
      le_pnt3.x(), le_pnt3.y(), le_pnt3.z(),
      ds_mid2, 3.*ds_mid2, 9.*ds_mid2);
  fprintf(dump_file, "    %f    %f    %f    %f    %f    %f  \n",
      te_pnt3.x(), te_pnt3.y(), te_pnt3.z(),
      ds_mid2, 3.*ds_mid2, 9.*ds_mid2);

  fprintf(dump_file, " %s: Trailing Edge Tri 3 \n", (char*) getName());
  fprintf(dump_file, "    %f    %f    %f    %f    %f    %f  \n",
      le_pnt3.x(), le_pnt3.y(), le_pnt3.z(),
      ds_mid2, 3.*ds_mid2, 9.*ds_mid2);
  fprintf(dump_file, "    %f    %f    %f    %f    %f    %f  \n",
      le_pnt4.x(), le_pnt4.y(), le_pnt4.z(),
      ds_tip,  3.*ds_tip , 9.*ds_tip );
  fprintf(dump_file, "    %f    %f    %f    %f    %f    %f  \n",
      te_pnt4.x(), te_pnt4.y(), te_pnt4.z(),
      ds_tip,  3.*ds_tip , 9.*ds_tip );

}

//==== Write Out Cross Sections to Aero File ====//
void WingGeom::write_aero_xsec(FILE* dump_file)
{
  //===== Determine Number of Cross Sections =====
  int num_sec;
  if ( sym_code == NO_SYM )
  {
    num_sec = 4;
  }
  else
  {
    num_sec = 7;
  }

  fprintf(dump_file, "========= CROSS SECTIONS =================================================================\n");
  fprintf(dump_file, "%d                        Number of Cross Sections\n",num_sec);
  fprintf(dump_file, "%d                        Points Per Cross Sections\n",mwing_surf.get_num_pnts());

  if ( sym_code != NO_SYM )
  {
    for ( int i = 4 ; i > 1 ; i-- )
      {
        mwing_surf.write_refl_xsec(sym_code, i, reflect_mat, dump_file);
      }
  }

  for ( int i = 1 ; i < 5 ; i++ )
    {
      mwing_surf.write_xsec(i, model_mat, dump_file);
    }

}

//==== Write Aero File =====//
void WingGeom::write_aero_file(FILE* aero_file, int aero_id_num)
{
//jrg fix

}





// Determines whether or not the strake chord should be considered for geometric
// dependent analysis routines such as aero. The strake chord has higher priority
// than the aft chord. In other words if both occupy the same location the aft chord
// will be disregarded
bool WingGeom::strake_chord_exists()
{
  bool strake_exists = false;

  if ( get_strake_span_per()->get() >= 0.02   // not on top of root chord
    && get_strake_span_per()->get() <= .98 )  // not on top of tip chord
  {
    // check that aft chord is not on top of strake chord
    if( fabs(get_strake_span_per()->get() - get_aft_ext_span_per()->get()) > .02 )
    {
        strake_exists = true;
    }
    else
    {
        // check if strake sweep matches le sweep or aft sweep matches te
        if ( fabs(get_strake_sweep()->get() - get_sweep_at(0.)*RAD_2_DEG ) >
            fabs(get_aft_ext_sweep()->get() - get_sweep_at(1.)*RAD_2_DEG ) )
        {
            strake_exists = true;
        }
    }
  }  
  return strake_exists;
}

// Determines whether or not the aft chord should be considered for geometric
// dependent analysis routines such as aero. 
//
// No longer true - The strake chord has higher priority
// than the aft chord. In other words if both occupy the same location the aft chord
// will be disregarded
bool WingGeom::aft_ext_chord_exists()
{
  bool aft_exists = false;

  if ( get_aft_ext_span_per()->get() >= 0.02   // not on top of root chord
    && get_aft_ext_span_per()->get() <= .98 )  // not on top of tip chord
  {
    // check that aft chord is not on top of strake chord
    if( fabs(get_strake_span_per()->get() - get_aft_ext_span_per()->get()) > .02 )
    {
        aft_exists = true;
    }
    else
    {
        // check if strake sweep matches le sweep or aft sweep matches te
        if ( fabs(get_strake_sweep()->get() - get_sweep_at(0.)*RAD_2_DEG ) <
            fabs(get_aft_ext_sweep()->get() - get_sweep_at(1.)*RAD_2_DEG ) )
        {
            aft_exists = true;
        }
    }
  }
  return aft_exists;
}

double WingGeom::get_strake_chord_length()
{
    // strake chord for main wing
    double sc = (tip_chord() - root_chord())*strake_span_per() + root_chord(); 
    
    if ( strake_span_per() < aft_ext_span_per() && aft_ext_span_per() > .02 )
    {
      double ty = span();                            // Tip Y Offset
      if ( getSymCode() == XZ_SYM )  ty *= 0.5;   // Check Sym Code

      // aft extension of strake chord
      double root_aft_ext = (tan(-aft_ext_sweep()*DEG_2_RAD) + get_tan_sweep_at(1.0))*ty*aft_ext_span_per();
      sc += root_aft_ext*(1.0 - strake_span_per() / aft_ext_span_per() );
    }
    
    return   sc;
}

double WingGeom::get_aft_ext_chord_length()
{
    // strake chord for main wing
    double ac = (tip_chord() - root_chord())*aft_ext_span_per() + root_chord();
 
    if ( aft_ext_span_per() < strake_span_per() && strake_span_per() > .02 )
    {
      double ty = span();                            // Tip Y Offset
      if ( getSymCode() == XZ_SYM )  ty *= 0.5;   // Check Sym Code
      
      // forward extension of aft chord
      double root_fore_ext = (tan(strake_sweep()*DEG_2_RAD) - get_tan_sweep_at(0.0))*ty*strake_span_per();
      ac += root_fore_ext*(1.0 - aft_ext_span_per() / strake_span_per() );
    } 
    return   ac;
}

vec3d WingGeom::getVertex3d(int surfid, double x, double p, int r)			
{ 
	switch (surfid)
	{
		case WING_SURF:
			return mwing_surf.get_vertex(x, p, r);
		case IN_FLAP_SURF:
			return in_flap_surf.get_vertex(x, p, r);
		case OUT_FLAP_SURF:
			return out_flap_surf.get_vertex(x, p, r);
		case IN_SLAT_SURF:
			return in_slat_surf.get_vertex(x, p, r);
		case OUT_SLAT_SURF:
			return out_slat_surf.get_vertex(x, p, r);
		default:
			return vec3d(0,0,0);
	}
}

void WingGeom::getVertexVec(vector< VertexID > *vertVec)
{ 
	buildVertexVec(&mwing_surf, WING_SURF, vertVec); 
	buildVertexVec(&in_flap_surf, IN_FLAP_SURF, vertVec); 
	buildVertexVec(&out_flap_surf, OUT_FLAP_SURF, vertVec); 
	buildVertexVec(&in_slat_surf, IN_SLAT_SURF, vertVec); 
	buildVertexVec(&out_slat_surf, OUT_SLAT_SURF, vertVec); 
}

//==== Load Wing Sections and Return Root X Offset =====//
double WingGeom::loadMSWingSectVec( vector< WingSect >& sectVec )
{
  //==== Check For Straight Taper Wing Flag ====//
  if ( !strake_aft_flag )
    {
      strake_sweep  = get_sweep_at(0.0)*RAD_2_DEG;
      aft_ext_sweep = get_sweep_at(1.0)*RAD_2_DEG;
    }

  //==== Main Wing Parms ====//
  double ty      = span();                       // Tip Y Offset
  if ( getSymCode() == XZ_SYM )  ty *= 0.5;   // Check Sym Code

  double tan_le  = get_tan_sweep_at(0.0);        // Tangent Leading Edge
  double toff    = tan_le*ty;                    // Tip X Offset
  double tc      = tip_chord();                  // Tip Chord
  double rc      = root_chord();                 // Root Chord

  //==== Strake Parms ====//
  double sy      = strake_span_per()*ty;                 // Strake Y Offset
  double sc      = (tc - rc)*strake_span_per() + rc;     // Strake Chord
  double sx      = tan(strake_sweep()*DEG_2_RAD)*sy;     // Strake X Length
  double strim   = toff*strake_span_per();               // Trim X Strake
  double sexp    = sx - strim;                           // Exposed X Strake

  //==== Aft Ext Parms ====//
  double ay      = aft_ext_span_per()*ty;                // Aft Y Offset
  double ax      = tan(-aft_ext_sweep()*DEG_2_RAD)*ay;   // Aft X Intercept
  double tan_te  = get_tan_sweep_at(1.0);                // Tangenet Trailing Edge
  double atrim   = -tan_te*ty*aft_ext_span_per();        // Aft Trimmed X
  double aexp    = ax - atrim;                           // Aft Exposed X
  double ac      = (tc - rc)*aft_ext_span_per() + rc;    // Aft Chord

  //==== Total Parms ====//
  double rct     = rc + sexp + aexp;                     // Total Root Chord
  double act, sct;                       // Total Aft and Strake Chords
  double aoff;                           // Aft X Offset
  double az, sz, tz;                     // Aft, Strake, Tip Z Offset
  double atwst, stwst;                   // Aft and Strake Twists
  int aid, sid;                         // Xsec Location IDs

  //==== Check If Aft is Inboard Of Strake ====//
  if ( ay < sy )
    {
      act  = ac + sexp*(1.0 - (aft_ext_span_per()/strake_span_per()));
      aoff = rc - atrim - act;
      sct  = sc;
      aid = 2;  sid = 3;
      az = tan(in_dihed()*DEG_2_RAD)*ay;
      sz = az + tan(mid_dihed()*DEG_2_RAD)*(sy - ay);
      tz = sz + tan(out_dihed()*DEG_2_RAD)*(ty - sy);
      atwst = in_twist();
      stwst = atwst + mid_twist();
    }
  else
    {
      act  = ac;
      aoff = toff*aft_ext_span_per();
      sct  = sc + aexp*(1.0 - (strake_span_per()/aft_ext_span_per()));;
      aid = 3;  sid = 2;
      sz = tan(in_dihed()*DEG_2_RAD)*sy;
      az = sz + tan(mid_dihed()*DEG_2_RAD)*(ay - sy);
      tz = az + tan(out_dihed()*DEG_2_RAD)*(ty - ay);
      stwst = in_twist();
      atwst = stwst + mid_twist();
    }

  //==== If Deflection File - Adjust Offsets and Twist ====//
  double total_tip_twist = in_twist() + mid_twist() + out_twist();
  if ( deflect_flag )
    {
      vec3d p = deflect_curve.comp_pnt_per_xyz( 0, aft_ext_span_per());
      az += p.y()*defl_scale();
      p = deflect_curve.comp_pnt_per_xyz( 0, strake_span_per());
      sz += p.y()*defl_scale();
      p = deflect_curve.comp_pnt_per_xyz( 0, 1.0 );
      tz += p.y()*defl_scale();

      p = twist_curve.comp_pnt_per_xyz( 0, aft_ext_span_per() );
      atwst += p.y()*twist_scale();
      p = twist_curve.comp_pnt_per_xyz( 0, strake_span_per() );
      stwst += p.y()*twist_scale();
      p = twist_curve.comp_pnt_per_xyz( 0, 1.0 );
      total_tip_twist += p.y()*twist_scale();
    }

  WingSect ws;
  if ( fabs( ay - sy ) < 0.001 )
	  ay += 0.001f;

  if ( ay < sy )
  {
	ws.get_span()->set( sqrt(ay*ay + az*az) );
//	if ( getSymCode() != XZ_SYM )  ws.span *= 0.5;
	ws.get_rc()->set( rct );
	ws.get_tc()->set( act );
	ws.get_twist()->set( in_twist() );
	ws.get_twistLoc()->set( twist_loc() );
	ws.get_sweep()->set( strake_sweep() );
	ws.get_dihedral()->set( in_dihed() );
	sectVec.push_back( ws );

	ws.get_span()->set( sqrt((sy-ay)*(sy-ay) + (sz-az)*(sz-az)) );
//	if ( getSymCode() != XZ_SYM )  ws.span *= 0.5;
	ws.get_rc()->set(  act);
	ws.get_tc()->set( sct);
	ws.get_twist()->set( in_twist() + mid_twist());
	ws.get_twistLoc()->set( twist_loc());
	ws.get_sweep()->set( strake_sweep());
	ws.get_dihedral()->set( mid_dihed());
	sectVec.push_back( ws );

	ws.get_span()->set(  sqrt((ty-sy)*(ty-sy) + (tz-sz)*(tz-sz)) );
//	if ( getSymCode() != XZ_SYM )  ws.span *= 0.5;
	ws.get_rc()->set(  sct );
	ws.get_tc()->set(  tc );
	ws.get_twist()->set(  in_twist() + mid_twist() + out_twist() );
	ws.get_twistLoc()->set(  twist_loc() );
	ws.get_dihedral()->set(  out_dihed() );
	ws.get_sweep()->set(  get_sweep_at(0.0)*RAD_2_DEG );
	sectVec.push_back( ws );
  }
  else
  {
	ws.get_span()->set(  sqrt(sy*sy + sz*sz));
//	if ( getSymCode() != XZ_SYM )  ws.span *= 0.5;
	ws.get_rc()->set(  rct);
	ws.get_tc()->set(  sct);
	ws.get_twist()->set(  in_twist());
	ws.get_twistLoc()->set(  twist_loc());
	ws.get_dihedral()->set(  in_dihed());
	ws.get_sweep()->set(  strake_sweep());
	sectVec.push_back( ws );

	ws.get_span()->set( sqrt((ay-sy)*(ay-sy) + (az-sz)*(az-sz)));
//	if ( getSymCode() != XZ_SYM )  ws.span *= 0.5;
	ws.get_rc()->set( rct);
	ws.get_tc()->set( act);
	ws.get_twist()->set(in_twist() + mid_twist());
	ws.get_twistLoc()->set( twist_loc());
	ws.get_dihedral()->set( mid_dihed());
	ws.get_sweep()->set( get_sweep_at(0.0)*RAD_2_DEG);
	sectVec.push_back( ws );

	ws.get_span()->set(sqrt((ty-ay)*(ty-ay) + (tz-az)*(tz-az)));
//	if ( getSymCode() != XZ_SYM )  ws.span *= 0.5;
	ws.get_rc()->set( act );
	ws.get_tc()->set( tc );
	ws.get_twist()->set( in_twist() + mid_twist() + out_twist() );
	ws.get_twistLoc()->set( twist_loc() );
	ws.get_dihedral()->set( out_dihed() );
	ws.get_sweep()->set( get_sweep_at(0.0)*RAD_2_DEG );
	sectVec.push_back( ws );
  }

  return sexp;
}


