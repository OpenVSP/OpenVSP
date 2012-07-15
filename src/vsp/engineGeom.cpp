//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//

//******************************************************************************
//    
//    Engine Geometry Class
//  
// 
//   J.R. Gloudemans - 11/7/97
//   Sterling Software
//
//
//******************************************************************************

#include "engineGeom.h"
#include "aircraft.h"
#include "materialMgr.h"

#ifdef __APPLE__
#  include <OpenGL/gl.h>
#else
#  include <GL/gl.h>
#endif

#include "defines.h"


//==== Constructor =====//
EngineGeom::EngineGeom(Aircraft* aptr) : Geom(aptr)
{
	type = ENGINE_GEOM_TYPE;
	type_str = Stringc("engine");

	char name[255];
	sprintf( name, "Engine_%d", geomCnt ); 
	geomCnt++;
	name_str = Stringc(name);
	setSymCode(NO_SYM);

	define_eng_parms();
	define_inl_parms();
	define_noz_parms();

	numXsecs.deactivate();
	numPnts.deactivate();

	generate();

	surfVec.push_back( &inlet_surf );
	surfVec.push_back( &inl_duct_in );
	surfVec.push_back( &inl_duct_out );
	surfVec.push_back( &nozzle_surf );
	surfVec.push_back( &engine_surf );



}

//==== Destructor =====//
EngineGeom::~EngineGeom()
{
}

void EngineGeom::copy( Geom* fromGeom )
{
	Geom::copy( fromGeom );						// Copy Base Stuff

	if ( fromGeom->getType() != ENGINE_GEOM_TYPE )
		return;

	EngineGeom* g = (EngineGeom*)fromGeom;			// UpCast

    inl_duct_flag = g->get_inl_duct_flag();

	length.set( g->get_length()->get() );
	rad_tip.set( g->get_rad_tip()->get() );
	max_tip.set( g->get_max_tip()->get() );
	hub_tip.set( g->get_hub_tip()->get() );

	exit_area_ratio.set( g->get_exit_area_ratio()->get() );
	noz_length.set( g->get_noz_length()->get() );

	inl_duct_x_off.set( g->get_inl_duct_x_off()->get() );
	inl_duct_y_off.set( g->get_inl_duct_y_off()->get() );
	inl_duct_shape.set( g->get_inl_duct_shape()->get() );

	cowl_length.set( g->get_cowl_length()->get() );
	area_ratio_eng_thrt.set( g->get_area_ratio_eng_thrt()->get() );
	area_ratio_hl_thrt.set( g->get_area_ratio_hl_thrt()->get() );
	lip_fine.set( g->get_lip_fine()->get() );
	ht_wid_ratio.set( g->get_ht_wid_ratio()->get() );

	upper_shape.set( g->get_upper_shape()->get() );
	lower_shape.set( g->get_lower_shape()->get() );
	inl_x_rot.set( g->get_inl_x_rot()->get() );
	inl_scarf.set( g->get_inl_scarf()->get() );

	compose_model_matrix();
	generate();
}

void EngineGeom::LoadLinkableParms( vector< Parm* > & parmVec )
{
	Geom::LoadLinkableParms( parmVec );
	Geom::AddLinkableParm( &rad_tip, parmVec, this, "Engine" );
	Geom::AddLinkableParm( &max_tip, parmVec, this, "Engine" );
	Geom::AddLinkableParm( &hub_tip, parmVec, this, "Engine" );
	Geom::AddLinkableParm( &length, parmVec, this, "Engine" );

	Geom::AddLinkableParm( &noz_length, parmVec, this, "Nozzle" );
	Geom::AddLinkableParm( &exit_area_ratio, parmVec, this, "Nozzle" );
	Geom::AddLinkableParm( &noz_ht_wid_ratio, parmVec, this, "Nozzle" );
	Geom::AddLinkableParm( &exit_throat_ratio, parmVec, this, "Nozzle" );
	Geom::AddLinkableParm( &dive_flap_ratio, parmVec, this, "Nozzle" );

	Geom::AddLinkableParm( &noz_duct_x_off, parmVec, this, "Nozzle" );
	Geom::AddLinkableParm( &noz_duct_y_off, parmVec, this, "Nozzle" );
	Geom::AddLinkableParm( &noz_duct_shape, parmVec, this, "Nozzle" );

	Geom::AddLinkableParm( &cowl_length, parmVec, this, "Inlet" );
	Geom::AddLinkableParm( &area_ratio_eng_thrt, parmVec, this, "Inlet" );
	Geom::AddLinkableParm( &area_ratio_hl_thrt, parmVec, this, "Inlet" );
	Geom::AddLinkableParm( &lip_fine, parmVec, this, "Inlet" );
	Geom::AddLinkableParm( &ht_wid_ratio, parmVec, this, "Inlet" );
	Geom::AddLinkableParm( &upper_shape, parmVec, this, "Inlet" );
	Geom::AddLinkableParm( &lower_shape, parmVec, this, "Inlet" );
	Geom::AddLinkableParm( &inl_x_rot, parmVec, this, "Inlet" );
	Geom::AddLinkableParm( &inl_scarf, parmVec, this, "Inlet" );
	Geom::AddLinkableParm( &inl_duct_x_off, parmVec, this, "Inlet" );
	Geom::AddLinkableParm( &inl_duct_y_off, parmVec, this, "Inlet" );
	Geom::AddLinkableParm( &inl_duct_shape, parmVec, this, "Inlet" );
	Geom::AddLinkableParm( &divertor_height, parmVec, this, "Inlet" );
	Geom::AddLinkableParm( &divertor_length, parmVec, this, "Inlet" );

}


//==== Define Engine Parms ====//
void EngineGeom::define_eng_parms()
{
  rad_tip.initialize(this, UPD_ALL, "Radius_Tip", 1.5);
  rad_tip.set_lower_upper(0.00001, 1000000.0);
  rad_tip.set_script("engine_parm rtip", 0);

  max_tip.initialize(this, UPD_ALL, "Max_Tip_Ratio", 1.3);
  max_tip.set_lower_upper(1.0001, 1000000.0);
  max_tip.set_script("engine_parm max", 0);

  hub_tip.initialize(this, UPD_ALL, "Hub_Tip_Ratio", 0.5);
  hub_tip.set_lower_upper(0.01, 0.99);
  hub_tip.set_script("engine_parm hub", 0);

  length.initialize(this, UPD_ALL, "Length", 4.0);
  length.set_lower_upper(0.00001, 1000000.0);
  length.set_script("engine_parm len", 0);

  int total_num_xsecs = NUM_HUB_XSECS + NUM_ENG_XSECS;

  for ( int i = 0 ; i < total_num_xsecs ; i++ )
    {
      eng_xsecs[i].set_num_pnts(NUM_PNTS_XSEC);
      eng_xsecs[i].set_type(CIRCLE);
    }
  eng_xsecs[0].set_type(XSEC_POINT);
  eng_xsecs[total_num_xsecs-1].set_type(XSEC_POINT);

  engine_surf.set_num_xsecs( total_num_xsecs );
  engine_surf.set_num_pnts(  NUM_PNTS_XSEC );

}

//==== Define Inlet Parms ====//
void EngineGeom::define_inl_parms()
{
  //===== General Inlet Parms =====//
  inl_type = SUB_PITOT;
  inl_xy_sym_flag = FALSE;

  //==== Duct Parms ====//
  inl_duct_flag = FALSE;
  inl_duct_x_off.initialize(this, UPD_ALL, "Inlet_Duct_X_Offset", 3.0);
  inl_duct_y_off.initialize(this, UPD_ALL, "Inlet_Duct_Y_Offset", 1.0);
  inl_duct_shape.initialize(this, UPD_ALL, "Duct Shape Factor",   0.5);
  inl_duct_x_off.set_lower_upper(  0.0001, 1000000.0);
  inl_duct_y_off.set_lower_upper(  0.0,  1000000.0);
  inl_duct_shape.set_lower_upper(  0.01,   1.0);
  inl_duct_x_off.set_script("engine_duct xoff", 0);
  inl_duct_y_off.set_script("engine_duct yoff", 0);
  inl_duct_shape.set_script("engine_duct shape", 0);

  divertor_flag = FALSE;
  divertor_height.initialize(this, UPD_ALL, "Divertor_Heigth", 0.5);
  divertor_length.initialize(this, UPD_ALL, "Divertor_Length", 1.0);
  divertor_height.set_lower_upper(  0.0001, 1000000.0);
  divertor_length.set_lower_upper(  0.0001, 1000000.0);

  //===== Subsonic Pitot Inlet Parms =====// 
  inl_half_split_flag = FALSE;    
  cowl_length.initialize(this, UPD_ALL, "Cowl_Length", 2.0);
  area_ratio_eng_thrt.initialize(this, UPD_ALL, "Area_Ratio_Engine_Throat", 1.5);
  area_ratio_hl_thrt.initialize(this, UPD_ALL, "Area_Ratio_Highlight_Throat", 1.3);
  lip_fine.initialize(this,  UPD_ALL, "Lip_Finess_Ratio", 1.9);
  ht_wid_ratio.initialize(this, UPD_ALL, "Height_Width_Ratio", 1.0);
  upper_shape.initialize(this, UPD_ALL, "Upper_Shape_Factor",-1.3);
  lower_shape.initialize(this, UPD_INLET, "Lower_Shape_Factor",  0.7);

  cowl_length.set_lower_upper( 0.0001, 1000000.0);
  area_ratio_eng_thrt.set_lower_upper( 0.5, 10.0);
  area_ratio_hl_thrt.set_lower_upper( 1.01, 10.0);
  lip_fine.set_lower_upper( 0.01,   100.0);
  ht_wid_ratio.set_lower_upper( 0.05, 20.0);
  upper_shape.set_lower_upper(-2.0,    2.0);
  lower_shape.set_lower_upper(-1.0,    1.0);

  cowl_length.set_script("engine_inlet cowllen", 0);
  area_ratio_eng_thrt.set_script("engine_inlet engthrt", 0);
  area_ratio_hl_thrt.set_script("engine_inlet hlthrt", 0);
  lip_fine.set_script("engine_inlet lipfr", 0);
  ht_wid_ratio.set_script("engine_inlet hw", 0);
  upper_shape.set_script("engine_inlet upsurf", 0);
  lower_shape.set_script("engine_inlet lowsurf", 0);

  inl_x_rot.initialize(this, UPD_ALL, "Inlet_X_Rotation", 0.0);
  inl_scarf.initialize(this, UPD_ALL, "Inlet_Scarf", 0.0);
  inl_x_rot.set_lower_upper(-90.0,  90.0);
  inl_scarf.set_lower_upper(-60.0,  60.0);
  inl_x_rot.set_script("engine_inlet xrot", 0);
  inl_scarf.set_script("engine_inlet scarf", 0);

  inl_curve.init_num_sections(3);
  inl_xsec.set_num_pnts(NUM_PNTS_XSEC);
  inl_xsec.set_type(CIRCLE);

  inlet_surf.set_num_xsecs( NUM_INL_XSECS );
  inlet_surf.set_num_pnts(  NUM_PNTS_XSEC );

  inl_duct_curve.init_num_sections(1);

}

//==== Define Nozzle Parms ====//
void EngineGeom::define_noz_parms()
{
  noz_type = CONVER_AXI;

  //==== Nozzle Parms ====//
  noz_duct_flag = FALSE;
  noz_duct_x_off.initialize(this, UPD_NOZZLE, "Nozzle_Duct_X_Offset", 1.0);
  noz_duct_y_off.initialize(this, UPD_NOZZLE, "Nozzle_Duct_Y_Offset", 0.0);
  noz_duct_shape.initialize(this, UPD_NOZZLE, "Nozzle_Shape_Factor",   0.0);
  noz_duct_x_off.set_lower_upper(  0.0001, 1000000.0);
  noz_duct_y_off.set_lower_upper(  0.0,  1000000.0);
  noz_duct_shape.set_lower_upper( -1.0,    1.0);

  noz_length.initialize(this, UPD_NOZZLE, "Nozzle_Length", 2.0);
  exit_area_ratio.initialize(this, UPD_NOZZLE, "Area_Ratio_Exit_Eng", 2.0);
  noz_ht_wid_ratio.initialize(this, UPD_NOZZLE, "Nozzle_Height_Width_Ratio", 1.0);
  exit_throat_ratio.initialize(this, UPD_NOZZLE, "Area_Ratio_Exit_Throat", 1.5);
  dive_flap_ratio.initialize(this, UPD_NOZZLE, "Dive_Flap_Ratio", 1.5);
  noz_length.set_lower_upper(  0.0001, 1000000.0);
  exit_area_ratio.set_lower_upper(  0.0001,  1000000.0);
  noz_ht_wid_ratio.set_lower_upper( -1.0,    1.0);
  exit_throat_ratio.set_lower_upper(  0.0,  1000000.0);
  dive_flap_ratio.set_lower_upper( -1.0,    1.0);

  noz_length.set_script("engine_nozzle length", 0);
  exit_area_ratio.set_script("engine_nozzle exit", 0);

  noz_curve.init_num_sections(2);
  noz_xsec.set_num_pnts(NUM_PNTS_XSEC);
  noz_xsec.set_type(CIRCLE);

  nozzle_surf.set_num_xsecs( NUM_NOZ_XSECS );
  nozzle_surf.set_num_pnts(  NUM_PNTS_XSEC );

}


//==== Generate Cross Sections =====//
void EngineGeom::set_ducted_default()
{
	setSymCode(XZ_SYM);

	xLoc = 23.5;					// XForm Translation
	yLoc = 0.7;
	zLoc = 0.0;

	xRot = -9.0;					// XForm Rotation
	yRot = 0.0;
	zRot = 0.0;

  compose_model_matrix();

  rad_tip = 0.550;
  max_tip = 1.1;
  length = 6.6;
  hub_tip = 0.5;

  cowl_length = 3.0;
  area_ratio_eng_thrt = 1.05;
  area_ratio_hl_thrt = 1.075;
  lip_fine = 2.0;
  ht_wid_ratio = 0.6;
  upper_shape = -1.6;
  lower_shape = 0.7;
  inl_x_rot = 0.0;
  inl_scarf = 40.0;
  inl_duct_flag = 1;
  inl_duct_x_off = 11.3;
  inl_duct_y_off = 0.95;
  inl_duct_shape = 0.14;
  noz_type = CONVER_AXI;
  noz_length = 5.0;
  exit_area_ratio = 1.5;
  noz_ht_wid_ratio = 1.0;

}

//==== Generate Fuse Component ====//
void EngineGeom::generate()
{
  generate_engine_xsecs();
  generate_inlet_curves();
  generate_inl_duct_xsecs();
  generate_nozzle_curves();
  update_bbox();

}

//==== Parm Has Changed ReGenerate Fuse Component ====//
void EngineGeom::parm_changed(Parm* p)
{
	if ( p->get_update_grp() == UPD_XFORM )
	{
		if ( p == &xLoc || p == &yLoc || p == &zLoc || 
			 p == &xRot || p == &yRot || p == &zRot )
			updateAttach(1);
		else
			updateAttach(0);
	}
	else if ( p->get_update_grp() == UPD_NUM_PNT_XSEC )
	{
//		numPnts  = 4*(numPnts.iget()/4)+1;
//		numXsecs = 4*(numXsecs.iget()/4)+1;
	}
	else if ( p->get_update_grp() == UPD_ENGINE )
	{
        generate_engine_xsecs();
	}
	else if ( p->get_update_grp() == UPD_INLET )
	{
        generate_inlet_curves();
        generate_inl_duct_xsecs();
	}
	else if ( p->get_update_grp() == UPD_NOZZLE )
	{
		generate_nozzle_curves();
	}

	if ( p == &scaleFactor )	
	{
		scale();
	}


	generate();
	updateAttach(0);

	compose_model_matrix();

	//==== Tell Aircraft You Have Changed ====//
	airPtr->geomMod( this );

}

vec3d EngineGeom::getAttachUVPos(double u, double v)
{
	vec3d pos;
	vec3d uvpnt;

	if ( inl_duct_flag )
	{
		if ( u < 0.2 )
			uvpnt = (engine_surf.comp_uv_pnt(u*2.5,0.0) + 
					 engine_surf.comp_uv_pnt(u*2.5,0.5))*0.5;

		else if ( u < 0.4 )
			uvpnt = (inl_duct_in.comp_uv_pnt((u-0.2)*5,0.0)+
					 inl_duct_in.comp_uv_pnt((u-0.2)*5,0.5))*0.5;

		else if ( u < 0.6 )
			uvpnt = (inlet_surf.comp_uv_pnt((u-0.4)*5,0.0) + 
					 inlet_surf.comp_uv_pnt((u-0.4)*5,0.5))*0.5;
			
		else if ( u < 0.8 )
			uvpnt = inl_duct_out.comp_uv_pnt((u-0.6)*5,v);

		else
			uvpnt = (nozzle_surf.comp_uv_pnt((u-0.8)*5,0.0) + 
					 nozzle_surf.comp_uv_pnt((u-0.8)*5,0.5))*0.5;
	}
	else
	{
		if ( u < 0.3 )
			uvpnt = ( engine_surf.comp_uv_pnt(u*2.0,0.0) + 
			          engine_surf.comp_uv_pnt(u*2.0,0.5))*0.5;
		else if ( u < 0.5 )
			uvpnt =  ( inlet_surf.comp_uv_pnt((u-0.3)*2.5,0.0) + 
					   inlet_surf.comp_uv_pnt((u-0.3)*2.5,0.5))*0.5;
		else if ( u < 0.7 )
			uvpnt =  inlet_surf.comp_uv_pnt((u-0.5)*2.5+0.5,v);
		else if ( u < 0.9 )
			uvpnt = nozzle_surf.comp_uv_pnt((u-0.7)*2.5,v);
		else
			uvpnt = (nozzle_surf.comp_uv_pnt(0.5,0.0) + 
					 nozzle_surf.comp_uv_pnt(0.5,0.5))*0.5;
	}
		
	pos = uvpnt.transform( model_mat );

	return pos;
}

void EngineGeom::AddDefaultSources(double base_len)
{
	//==== Add Some Default Sources for CFD Mesh ====//
	PointSource* ps = new PointSource();
	ps->SetName("Cone");
	ps->SetLen( 0.2*base_len );
	ps->SetRad( 0.25*length()*rad_tip() );
	vec2d loc = vec2d(0.2,0);
	ps->SetUWLoc( loc );
	sourceVec.push_back( ps );

	ps = new PointSource();
	ps->SetName("Inlet");
	ps->SetLen( 0.2*base_len );
	ps->SetRad( 0.4*length()*rad_tip() );
	loc = vec2d(0.49,0);
	ps->SetUWLoc( loc );
	sourceVec.push_back( ps );
}


//==== Generate Engine Cross Sections ====//
void EngineGeom::generate_engine_xsecs()
{
  int i;
  double rad_hub = rad_tip() * hub_tip();
  double hub_length = HUB_LENGTH_RAD*rad_hub;

  for (  i = 0 ; i < NUM_HUB_XSECS ; i++)
    {
      double fract = (float)i/(float)(NUM_HUB_XSECS - 1);
      double x = -hub_length + (fract*fract)*hub_length; 
      vec3d pnt = vec3d(x, 0.0, 0.0); 
      eng_xsecs[i].set_pnt_on_spine(pnt);
      double rad = fract*rad_hub;
      eng_xsecs[i].set_height((float)(rad*2.0)); //??
      eng_xsecs[i].set_width((float)(rad*2.0)); //??
      eng_xsecs[i].generate();
    }
  double eng_length = length()*rad_tip(); 
  for ( i = 0 ; i < NUM_ENG_XSECS-1 ; i++)
    {
      double fract = (float)(i)/(float)(NUM_ENG_XSECS-2);
      double x = fract*eng_length; 
      vec3d pnt = vec3d(x, 0.0, 0.0); 
      eng_xsecs[i+NUM_HUB_XSECS].set_pnt_on_spine(pnt);
      eng_xsecs[i+NUM_HUB_XSECS].set_height((float)(rad_tip()*2.0));  //??
      eng_xsecs[i+NUM_HUB_XSECS].set_width((float)(rad_tip()*2.0)); //??
      eng_xsecs[i+NUM_HUB_XSECS].generate();
    }      

  vec3d pnt = vec3d(eng_length, 0.0, 0.0); 
  eng_xsecs[NUM_HUB_XSECS + NUM_ENG_XSECS -1].set_pnt_on_spine(pnt);
  eng_xsecs[NUM_HUB_XSECS + NUM_ENG_XSECS -1].set_height(0.0); //??
  eng_xsecs[NUM_HUB_XSECS + NUM_ENG_XSECS -1].set_width(0.0); //??
  eng_xsecs[NUM_HUB_XSECS + NUM_ENG_XSECS -1].generate();

  gen_surf();
 
}


//==== Generate Inlet Curves ====//
void EngineGeom::generate_inlet_curves()
{
  //==== Compute Defining Parmameters ====//
  double eng_rad  = rad_tip();
  double max_rad  = max_tip() * eng_rad;
  double cowl_len = -cowl_length() * eng_rad;
  
  //==== Check For Inlet Duct And Locate Last Xsec ====//
  double last_xsec_x = (length()*rad_tip())/3.0; 
  if ( inl_duct_flag )
    last_xsec_x = 0.0;

  double face_area = PI*eng_rad*eng_rad;
  double throat_area = face_area/area_ratio_eng_thrt();
  double hl_area = throat_area*area_ratio_hl_thrt();

  double throat_rad = sqrt(throat_area/PI); 
  double hl_rad     = sqrt(hl_area/PI); 

  double throat_len = cowl_len + lip_fine()*(hl_rad-throat_rad);

  //==== Throat Shape Factor - Lower Shape Factor ====//
  double throat_x1 = 0.0;
  double throat_x2 = throat_len;
  double throat_shape_factor = lower_shape(); 
  if ( throat_shape_factor < -0.5 )
    {
      throat_x1 = throat_len*(-0.5-throat_shape_factor);  
      throat_x2 = throat_len*0.5;
    }
  else if ( throat_shape_factor < 0.0 )
    throat_x2 = throat_len*(1.0+throat_shape_factor);

  else if ( throat_shape_factor < 0.5 )
    throat_x1 = throat_len*throat_shape_factor;

  else
    {
      throat_x1 = throat_len*0.5;  
      throat_x2 = throat_len*(1.0-(throat_shape_factor-0.5));
    }



  //==== Highlight to Throat Ellipse Appox ====//
  double ellipse_factor = 0.552284f;
  double ht_x_4 = ellipse_factor*(cowl_len - throat_len) + throat_len;
  double ht_z_5 = ellipse_factor*(throat_rad-hl_rad)+hl_rad;

  //==== External Shape Factor - Upper Shape Factor ====//
  double ext_z7 = hl_rad;
  double del_z7 = 0.5*(max_rad-hl_rad);
  double ext_x8 = last_xsec_x;
  double del_x8 = 0.5*(cowl_len-last_xsec_x);
  double ext_shape_factor = upper_shape();
 
  if ( ext_shape_factor < 0.0 )
    {
      ext_z7 += del_z7*ext_shape_factor*ext_shape_factor;  
      ext_x8 -= del_x8*ext_shape_factor;
    }
  else
    {
      ext_z7 += del_z7*ext_shape_factor;  
      ext_x8 += del_x8*ext_shape_factor*ext_shape_factor;
    }

  //==== Load Points For Curves ====//
  inl_curve.put_pnt(0, vec3d(0.0, 0.0, eng_rad) );
  inl_curve.put_pnt(1, vec3d(throat_x1, 0.0, eng_rad) );
  inl_curve.put_pnt(2, vec3d(throat_x2, 0.0, throat_rad) );
  inl_curve.put_pnt(3, vec3d(throat_len, 0.0, throat_rad) );

  inl_curve.put_pnt(4, vec3d(ht_x_4, 0.0, throat_rad) );
  inl_curve.put_pnt(5, vec3d(cowl_len, 0.0, ht_z_5) );
  inl_curve.put_pnt(6, vec3d(cowl_len, 0.0, hl_rad) );

  inl_curve.put_pnt(7, vec3d(cowl_len, 0.0, ext_z7) );
  inl_curve.put_pnt(8, vec3d(ext_x8,   0.0, max_rad) );
  inl_curve.put_pnt(9, vec3d(last_xsec_x, 0.0, max_rad) );

  gen_inl_surf();
}


//==== Generate Inlet Duct Curves ====//
void EngineGeom::generate_inl_duct_xsecs()
{
  if ( !inl_duct_flag )
    {
      inl_duct_in.set_num_xsecs(0);
      inl_duct_in.set_num_pnts( 0);
      inl_duct_out.set_num_xsecs(0);
      inl_duct_out.set_num_pnts( 0);
      return;
    }

  inl_duct_in.set_num_xsecs( NUM_INL_DUCT_XSECS );
  inl_duct_in.set_num_pnts(  NUM_PNTS_XSEC );
  inl_duct_out.set_num_xsecs( NUM_INL_DUCT_XSECS+1 );
  inl_duct_out.set_num_pnts(  NUM_PNTS_XSEC );
       
  double eng_rad  = rad_tip();
  double max_rad  = max_tip() * eng_rad;
  double eng_len = length()*rad_tip(); 
  double eng_len_1_3 = eng_len/3.0;
  double x_off = inl_duct_x_off()*eng_rad*2.0;  
  double y_off = inl_duct_y_off()*eng_rad*2.0; 

  inl_duct_curve.put_pnt(0, vec3d( 0.0, 0.0, 0.0) );
  inl_duct_curve.put_pnt(1, vec3d(-x_off*inl_duct_shape(), 0.0, 0.0) );
  inl_duct_curve.put_pnt(2, vec3d(-x_off*(1.0-inl_duct_shape()), y_off, 0.0) );
  inl_duct_curve.put_pnt(3, vec3d(-x_off, y_off, 0.0) );

  double dia = 2.0*eng_rad;
  double w = dia*sqrt(ht_wid_ratio());
  double h = w/ht_wid_ratio();
  vec3d neg_x = vec3d(-1.0, 0.0, 0.0);

  double u, ang;
  vec3d tan;
  vec3d pnt;
  int i,j;

  //==== Set Cross Section Parms And Loads X-Secs ====//
  for (  i = 0 ; i < NUM_INL_DUCT_XSECS ; i++ )
    {
      u = (double)i/(double)(NUM_INL_DUCT_XSECS - 1);

      //==== Generate Cross Section ====//
      inl_xsec.set_height((float)((h-dia)*u + dia) ); 
      inl_xsec.set_width((float)((w-dia)*u + dia) ); 
      inl_xsec.generate();

      for (  j = 0 ; j < NUM_PNTS_XSEC ; j++ )
        {
          inl_duct_in.set_pnt(i, j, inl_xsec.get_pnt(j));
        }
      //===== Add Rotation To Stay Tangent To Duct Curve =====//
      if ( inl_x_rot() > SMALL_POS_FLOAT ||  inl_x_rot() < SMALL_NEG_FLOAT)
        {
          inl_duct_in.rotate_xsec_x(i, inl_x_rot());           
        }

      //===== Add Rotation To Stay Tangent To Duct Curve =====//
      tan = inl_duct_curve.comp_tan(0, u);
      tan.normalize();
      ang = angle(neg_x, tan);
      inl_duct_in.rotate_xsec_z(i, ang*RAD_2_DEG);
         
      //===== Tranlate Xsec To Duct Curve =====//
      pnt = inl_duct_curve.comp_pnt(0, u);
      int out_ind = NUM_INL_DUCT_XSECS - i - 1;
      for (  j = 0 ; j < NUM_PNTS_XSEC ; j++ )
        {
          inl_duct_out.set_pnt(out_ind, j, inl_duct_in.get_pnt(i, j)*max_tip());
        }
      inl_duct_in.offset_xsec_x(i, pnt.x());
      inl_duct_in.offset_xsec_y(i, pnt.y());
      inl_duct_out.offset_xsec_x(out_ind, pnt.x());
      inl_duct_out.offset_xsec_y(out_ind, pnt.y());
    }

  //===== Add Last Xsec On Duct Curve =====//
  for (  j = 0 ; j < NUM_PNTS_XSEC ; j++ )
    {
      inl_duct_out.set_pnt(NUM_INL_DUCT_XSECS, j, inl_duct_in.get_pnt(0, j)*max_tip());
    }
  inl_duct_out.offset_xsec_x(NUM_INL_DUCT_XSECS, eng_len_1_3);


  inl_duct_in.load_refl_pnts_xsecs();
  inl_duct_out.load_refl_pnts_xsecs();

  inl_duct_in.load_hidden_surf();
  inl_duct_out.load_hidden_surf();

  inl_duct_in.load_normals();
  inl_duct_out.load_normals();

  inl_duct_in.load_uw();
  inl_duct_out.load_uw();

}


//==== Generate Nozzle Curves ====//
void EngineGeom::generate_nozzle_curves()
{
  //==== Compute Defining Parmameters ====//
  double eng_rad  = rad_tip();
  double max_rad  = max_tip() * eng_rad;
  double face_area = PI*eng_rad*eng_rad;
  double exit_area = face_area/exit_area_ratio();

  double eng_len = length()*rad_tip(); 
  double eng_len_1_3 = eng_len/3.0;
  double noz_len = noz_length()* eng_rad;

  double exit_rad = sqrt(exit_area/PI); 

  //==== Shape Control Points ====//
//   float out_x_1 = (noz_len+eng_len - eng_len_1_3)*.333 + eng_len_1_3;
//   float out_x_2 = (noz_len+eng_len - eng_len_1_3)*.667 + eng_len_1_3;
  
  double out_x_4 = (noz_len)*0.667 + eng_len;
  double out_x_5 = (noz_len)*0.333 + eng_len;

 
  //==== Load Points For Curves ====//
  noz_curve.put_pnt(0, vec3d(eng_len_1_3,     0.0, max_rad) );
  noz_curve.put_pnt(1, vec3d(out_x_5,         0.0, max_rad) );
  noz_curve.put_pnt(2, vec3d(out_x_4,         0.0, max_rad) );
  noz_curve.put_pnt(3, vec3d(noz_len+eng_len, 0.0, exit_rad) );

  noz_curve.put_pnt(4, vec3d(out_x_4,  0.0, eng_rad) );
  noz_curve.put_pnt(5, vec3d(out_x_5,  0.0, eng_rad) );
  noz_curve.put_pnt(6, vec3d(eng_len,  0.0, eng_rad) );

  gen_noz_surf();

}

//==== Write Engine File ====//
void EngineGeom::write(xmlNodePtr root)
{
  xmlAddStringNode( root, "Type", "Engine");

  //==== Write General Parms ====//
  xmlNodePtr gen_node = xmlNewChild( root, NULL, (const xmlChar *)"General_Parms", NULL );
  write_general_parms( gen_node );

  //==== Write Fuse Parms ====//
  xmlNodePtr eng_node = xmlNewChild( root, NULL, (const xmlChar *)"Engine_Parms", NULL );

  xmlAddIntNode( eng_node, "Engine_Type", 0 );
  xmlAddDoubleNode( eng_node, "Radius_Tip", rad_tip() );
  xmlAddDoubleNode( eng_node, "Max_Tip", max_tip() );
  xmlAddDoubleNode( eng_node, "Hub_Tip", hub_tip() );
  xmlAddDoubleNode( eng_node, "Eng_Length", length() );
  xmlAddIntNode( eng_node, "Inlet_Type", inl_type );
  xmlAddIntNode( eng_node, "Inl_Noz_Color", inl_noz_color );
  xmlAddIntNode( eng_node, "Inlet_XY_Sym_Flag", inl_xy_sym_flag );
  xmlAddIntNode( eng_node, "Inlet_Half_Split_Flag", inl_half_split_flag );
  xmlAddDoubleNode( eng_node, "Cowl_Length", cowl_length() );
  xmlAddDoubleNode( eng_node, "Eng_Thrt_Ratio", area_ratio_eng_thrt() );
  xmlAddDoubleNode( eng_node, "Hilight_Thrt_Ratio", area_ratio_hl_thrt() );
  xmlAddDoubleNode( eng_node, "Lip_Finess_Ratio", lip_fine() );
  xmlAddDoubleNode( eng_node, "Height_Width_Ratio", ht_wid_ratio() );
  xmlAddDoubleNode( eng_node, "Upper_Surf_Shape_Factor", upper_shape() );
  xmlAddDoubleNode( eng_node, "Lower_Surf_Shape_Factor", lower_shape() );
  xmlAddDoubleNode( eng_node, "Inlet_X_Axis_Rot", inl_x_rot() );
  xmlAddDoubleNode( eng_node, "Inlet_Scarf_Angle", inl_scarf() );
  xmlAddIntNode( eng_node, "Inlet_Duct_On_Off", inl_duct_flag );
  xmlAddDoubleNode( eng_node, "Inlet_Duct_X_Offset", inl_duct_x_off() );
  xmlAddDoubleNode( eng_node, "Inlet_Duct_Y_Offset", inl_duct_y_off() );
  xmlAddDoubleNode( eng_node, "Inlet_Duct_Shape_Factor", inl_duct_shape() );
  xmlAddIntNode( eng_node, "Divertor_On_Off", divertor_flag );
  xmlAddDoubleNode( eng_node, "Divertor_Height", divertor_height() );
  xmlAddDoubleNode( eng_node, "Divertor_Length", divertor_length() );
  xmlAddIntNode( eng_node, "Nozzle_Type", noz_type );
  xmlAddDoubleNode( eng_node, "Nozzle_Length", noz_length() );
  xmlAddDoubleNode( eng_node, "Exit_Area_Ratio", exit_area_ratio() );
  xmlAddDoubleNode( eng_node, "Nozzle_Height_Width_Ratio", noz_ht_wid_ratio() );
  xmlAddDoubleNode( eng_node, "Exit_Throat_Ratio", exit_throat_ratio() );
  xmlAddDoubleNode( eng_node, "Dive_Flap_Ratio", dive_flap_ratio() );
  xmlAddIntNode( eng_node, "Nozzle_Duct_On_Off", noz_duct_flag );
  xmlAddDoubleNode( eng_node, "Nozzle_Duct_X_Offset", noz_duct_x_off() );
  xmlAddDoubleNode( eng_node, "Nozzle_Duct_Y_Offset", noz_duct_y_off() );
  xmlAddDoubleNode( eng_node, "Nozzle_Duct_Shape_Factor", noz_duct_shape() );

}

//==== Write Engine File ====//
void EngineGeom::write(FILE* file_id)
{
}

//==== Read Fuse File ====//
void EngineGeom::read(xmlNodePtr root)
{
  xmlNodePtr node;

  //===== Read General Parameters =====//
  node = xmlGetNode( root, "General_Parms", 0 );
  if ( node )
    read_general_parms( node );

  //===== Read Fuse Parameters =====//
  node = xmlGetNode( root, "Engine_Parms", 0 );
  if ( node )
  {
    rad_tip = xmlFindDouble( node, "Radius_Tip", rad_tip() );
    max_tip = xmlFindDouble( node, "Max_Tip",    max_tip() );
    hub_tip = xmlFindDouble( node, "Hub_Tip",    hub_tip() );
    length  = xmlFindDouble( node, "Eng_Length", length() );

    inl_type = xmlFindInt( node, "Inlet_Type", inl_type );
    inl_noz_color = xmlFindInt( node, "Inl_Noz_Color", inl_noz_color );
    inl_xy_sym_flag = xmlFindInt( node, "Inlet_XY_Sym_Flag", inl_xy_sym_flag );
    inl_half_split_flag = xmlFindInt( node, "Inlet_Half_Split_Flag", inl_half_split_flag );

    cowl_length  = xmlFindDouble( node, "Cowl_Length", cowl_length() );
    area_ratio_eng_thrt = xmlFindDouble( node, "Eng_Thrt_Ratio", area_ratio_eng_thrt() );
    area_ratio_hl_thrt  = xmlFindDouble( node, "Hilight_Thrt_Ratio", area_ratio_hl_thrt() );
    lip_fine = xmlFindDouble( node, "Lip_Finess_Ratio", lip_fine() );
    ht_wid_ratio = xmlFindDouble( node, "Height_Width_Ratio", ht_wid_ratio() );
    upper_shape = xmlFindDouble( node, "Upper_Surf_Shape_Factor", upper_shape() );
    lower_shape = xmlFindDouble( node, "Lower_Surf_Shape_Factor", lower_shape() );
    inl_x_rot = xmlFindDouble( node, "Inlet_X_Axis_Rot", inl_x_rot() );
    inl_scarf = xmlFindDouble( node, "Inlet_Scarf_Angle", inl_scarf() );

    inl_duct_flag   = xmlFindInt( node, "Inlet_Duct_On_Off", inl_duct_flag );
    inl_duct_x_off  = xmlFindDouble( node, "Inlet_Duct_X_Offset", inl_duct_x_off() );
    inl_duct_y_off  = xmlFindDouble( node, "Inlet_Duct_Y_Offset", inl_duct_y_off() );
    inl_duct_shape  = xmlFindDouble( node, "Inlet_Duct_Shape_Factor", inl_duct_shape() );
    divertor_flag   = xmlFindInt( node, "Divertor_On_Off", divertor_flag );
    divertor_height  = xmlFindDouble( node, "Divertor_Height", divertor_height() );
    divertor_length  = xmlFindDouble( node, "Divertor_Length", divertor_length() );

    noz_type   = xmlFindInt( node, "Nozzle_Type", noz_type );
    noz_length  = xmlFindDouble( node, "Nozzle_Length", noz_length() );
    exit_area_ratio  = xmlFindDouble( node, "Exit_Area_Ratio", exit_area_ratio() );
    noz_ht_wid_ratio  = xmlFindDouble( node, "Nozzle_Height_Width_Ratio", noz_ht_wid_ratio() );
    exit_throat_ratio  = xmlFindDouble( node, "Exit_Throat_Ratio", exit_throat_ratio() );
    dive_flap_ratio  = xmlFindDouble( node, "Dive_Flap_Ratio", dive_flap_ratio() );

    noz_duct_flag   = xmlFindInt( node, "Nozzle_Duct_On_Off", noz_duct_flag );
    noz_duct_x_off  = xmlFindDouble( node, "Nozzle_Duct_X_Offset", noz_duct_x_off() );
    noz_duct_y_off  = xmlFindDouble( node, "Nozzle_Duct_Y_Offset", noz_duct_y_off() );
    noz_duct_shape  = xmlFindDouble( node, "Nozzle_Duct_Shape_Factor", noz_duct_shape() );

  }
  generate();

}

//==== Read Fuse File ====//
void EngineGeom::read(FILE* file_id)
{
  char buff[255];
  int idum; 

  //==== Read General Parms ====// 
  read_general_parms(file_id);

  //==== Read Engine Parms ====//		
  fgets(buff, 80, file_id);
  fscanf(file_id, "%d",&idum); 			fgets(buff, 80, file_id);
  rad_tip.read(file_id);
  max_tip.read(file_id);
  hub_tip.read(file_id);
  length.read(file_id);

  //==== Read Inlet Parms ====// 		
  fgets(buff, 80, file_id);
  fscanf(file_id, "%d",&inl_type); 		fgets(buff, 80, file_id);
  //===== Fix For old Inlet Types =====
  if (inl_type == 0) inl_type = SUB_PITOT;

  fscanf(file_id, "%d",&inl_noz_color); 	fgets(buff, 80, file_id);
  fscanf(file_id, "%d",&inl_xy_sym_flag); 	fgets(buff, 80, file_id);

  //==== Read Subsonic Pitot Inlet Parms ====//	
  fgets(buff, 80, file_id);
  fscanf(file_id, "%d",&inl_half_split_flag); 	fgets(buff, 80, file_id);
  cowl_length.read(file_id);
  area_ratio_eng_thrt.read(file_id);
  area_ratio_hl_thrt.read(file_id);
  lip_fine.read(file_id);
  ht_wid_ratio.read(file_id);
  upper_shape.read(file_id);
  lower_shape.read(file_id);
  inl_x_rot.read(file_id);
  inl_scarf.read(file_id);

  //==== Read Inlet Duct Parms ====//		
  fgets(buff, 80, file_id);
  fscanf(file_id, "%d",&inl_duct_flag); 	fgets(buff, 80, file_id);
  inl_duct_x_off.read(file_id);
  inl_duct_y_off.read(file_id);
  inl_duct_shape.read(file_id);

  //==== Read Divertor Parms ====//		
  fgets(buff, 80, file_id);
  fscanf(file_id, "%d",&divertor_flag); 	fgets(buff, 80, file_id);
  divertor_height.read(file_id);
  divertor_length.read(file_id);

  //==== Read Nozzle Parms ====//		
  fgets(buff, 80, file_id);
  fscanf(file_id, "%d",&noz_type); 		fgets(buff, 80, file_id);

  //==== Read Converg and Diverge Parms ====//	
  fgets(buff, 80, file_id);
  noz_length.read(file_id);
  exit_area_ratio.read(file_id);
  noz_ht_wid_ratio.read(file_id);
  exit_throat_ratio.read(file_id);
  dive_flap_ratio.read(file_id);

  //==== Read Nozzle Duct Parms ====//		
  fgets(buff, 80, file_id);
  fscanf(file_id, "%d",&noz_duct_flag); 	fgets(buff, 80, file_id);
  noz_duct_x_off.read(file_id);
  noz_duct_y_off.read(file_id);
  noz_duct_shape.read(file_id);
 
  generate();

}

//==== Generate Cross Sections =====//
void EngineGeom::regenerate()
{
}


//==== Generate Cross Sections =====//
void EngineGeom::gen_surf()
{
  //==== Hub X-Secs ====//
  for ( int i = 0 ; i < NUM_HUB_XSECS + NUM_ENG_XSECS ; i++ )
    {
      for ( int j = 0 ; j < NUM_PNTS_XSEC ; j++ )
        {
			vec3d p = eng_xsecs[i].get_pnt(j) + eng_xsecs[i].get_pnt_on_spine();
			engine_surf.set_pnt(i, j, p);
        }
    }

  engine_surf.load_refl_pnts_xsecs();
  engine_surf.load_hidden_surf();
  engine_surf.load_normals();
  engine_surf.load_uw();
}

//==== Generate Cross Sections =====//
void EngineGeom::gen_inl_surf()
{
  double height = sqrt(ht_wid_ratio());
  double width  = height*ht_wid_ratio();
  inl_xsec.set_height((float)height); 
  inl_xsec.set_width((float)width); 
  inl_xsec.generate();

  double u, dia;
  vec3d def_pnt;
  vec3d temp_pnt;
  int i,j;

  //==== Set Cross Section Parms And Loads X-Secs ====//
  for (  i = 0 ; i < NXSEC_1 ; i++ )
    {
      u = (float)i/(float)(NXSEC_1 - 1);
      def_pnt = inl_curve.comp_pnt(0, u);
      dia = 2.0*def_pnt.z()/ht_wid_ratio();
      for (  j = 0 ; j < NUM_PNTS_XSEC ; j++ )
        {
          temp_pnt = inl_xsec.get_pnt(j)*dia;
          temp_pnt.set_x(def_pnt.x());      
          inlet_surf.set_pnt(i, j, temp_pnt);
        }
    }

  //==== Set Cross Section Parms And Loads X-Secs ====//
  for ( i = 0 ; i < NXSEC_2 ; i++ )
    {
      u = (float)(i+1)/(float)(NXSEC_2);
      def_pnt = inl_curve.comp_pnt(1, u);
      dia = 2.0*def_pnt.z()/ht_wid_ratio();
      for (  j = 0 ; j < NUM_PNTS_XSEC ; j++ )
        {
          temp_pnt = inl_xsec.get_pnt(j)*dia;
          temp_pnt.set_x(def_pnt.x());      
          inlet_surf.set_pnt(i+NXSEC_1, j, temp_pnt);
        }
    }

  //==== Set Cross Section Parms And Loads X-Secs ====//
  for ( i = 0 ; i < NXSEC_3 ; i++ )
    {
      u = (float)(i+1)/(float)(NXSEC_3);
      def_pnt = inl_curve.comp_pnt(2, u);
      dia = 2.0*def_pnt.z()/ht_wid_ratio();
      for (  j = 0 ; j < NUM_PNTS_XSEC ; j++ )
        {
          temp_pnt = inl_xsec.get_pnt(j)*dia;
          temp_pnt.set_x(def_pnt.x());      
          inlet_surf.set_pnt(i+NXSEC_1+NXSEC_2, j, temp_pnt);
        }
    }

  //==== Add Scarf Rotation ====//
  if ( inl_scarf() > SMALL_POS_FLOAT ||  inl_scarf() < SMALL_NEG_FLOAT)
    {
      double fract, x_loc, angle, one_over_cos_angle;
      double cowl_len = -cowl_length() * rad_tip();
      for (  i = 0 ; i < NUM_INL_XSECS ; i++ )
        {
          x_loc = inlet_surf.get_pnt(i, 0).x();
          fract = x_loc/cowl_len;
          if (fract > 0.0)
            {
              angle = inl_scarf()*fract;
              one_over_cos_angle = 1.0/cos(angle*DEG_2_RAD);
              inlet_surf.offset_xsec_x(i, -x_loc);
              inlet_surf.rotate_xsec_z(i, angle); 
              inlet_surf.scale_xsec_x(i, one_over_cos_angle); 
              inlet_surf.scale_xsec_y(i, one_over_cos_angle); 
              inlet_surf.offset_xsec_x(i, x_loc);
            }
        }
    }
  //==== Add X Rotation ====//
  if ( inl_x_rot() > SMALL_POS_FLOAT ||  inl_x_rot() < SMALL_NEG_FLOAT)
    {
      for (  i = 0 ; i < NUM_INL_XSECS ; i++ )
        {
          inlet_surf.rotate_xsec_x(i, inl_x_rot());           
        }
    }

  //==== Offset Because Of Duct ====//
  if ( inl_duct_flag )
    {
      double x_off = -inl_duct_x_off()*rad_tip()*2.0;  
      double y_off =  inl_duct_y_off()*rad_tip()*2.0;  
      for (  i = 0 ; i < NUM_INL_XSECS ; i++ )
        {
          inlet_surf.offset_xsec_x(i,  x_off);
          inlet_surf.offset_xsec_y(i,  y_off);
        }
    }

  inlet_surf.load_refl_pnts_xsecs();
  inlet_surf.load_hidden_surf();
  inlet_surf.load_normals();
  inlet_surf.load_uw();

}

//==== Generate Cross Sections =====//
void EngineGeom::gen_noz_surf()
{
  double height = sqrt(noz_ht_wid_ratio());
  double width  = height*noz_ht_wid_ratio();
  noz_xsec.set_height((float)height); 
  noz_xsec.set_width((float)width); 
  noz_xsec.generate();

  double u, dia;
  vec3d def_pnt;
  vec3d temp_pnt;
  int i,j;
  //==== Set Cross Section Parms And Loads X-Secs ====//
  for (  i = 0 ; i < NOZ_NX_1 ; i++ )
    {
      u = (float)i/(float)(NOZ_NX_1 - 1);
      def_pnt = noz_curve.comp_pnt(0, u);
      dia = 2.0*def_pnt.z()/noz_ht_wid_ratio();
      for (  j = 0 ; j < NUM_PNTS_XSEC ; j++ )
        {
          temp_pnt = noz_xsec.get_pnt(j)*dia;
          temp_pnt.set_x(def_pnt.x());      
          nozzle_surf.set_pnt(i, j, temp_pnt);
        }
    }

  //==== Set Cross Section Parms And Loads X-Secs ====//
  for ( i = 0 ; i < NOZ_NX_2 ; i++ )
    {
      u = (float)i/(float)(NOZ_NX_2 - 1);
      def_pnt = noz_curve.comp_pnt(1, u);
      dia = 2.0*def_pnt.z()/noz_ht_wid_ratio();
      for (  j = 0 ; j < NUM_PNTS_XSEC ; j++ )
        {
          temp_pnt = noz_xsec.get_pnt(j)*dia;
          temp_pnt.set_x(def_pnt.x());      
          nozzle_surf.set_pnt(i+NOZ_NX_1, j, temp_pnt);
        }
    }

  nozzle_surf.load_refl_pnts_xsecs();
  nozzle_surf.load_hidden_surf();
  nozzle_surf.load_normals();
  nozzle_surf.load_uw();


}

//==== Generate Cross Sections =====//
void EngineGeom::update_bbox()
{
  bbox new_box;

  //==== Check Some Inlet And Nozzle Xsecs ====//
  for ( int j = 0 ; j < NUM_PNTS_XSEC ; j++ )
    {
      vec3d pnt = inlet_surf.get_pnt(NXSEC_1+NXSEC_2-1,j);
      new_box.update( pnt );
      pnt = inlet_surf.get_pnt(NXSEC_1+NXSEC_2+NXSEC_3-1,j);
      new_box.update( pnt );
      pnt = nozzle_surf.get_pnt(NOZ_NX_1-1,j);
      new_box.update( pnt );
    }

  bnd_box = new_box;

  update_xformed_bbox();			// Load Xform BBox
}

//==== Draw Engine Geom ====//
void EngineGeom::draw()
{
	Geom::draw();
/*
	if ( fastDrawFlag )
	{
		inlet_surf.fast_draw_on();
		inl_duct_in.fast_draw_on();
		inl_duct_out.fast_draw_on();
		nozzle_surf.fast_draw_on();
		engine_surf.fast_draw_on();
	}
	else
	{
		inlet_surf.fast_draw_off();
		inl_duct_in.fast_draw_off();
		inl_duct_out.fast_draw_off();
		nozzle_surf.fast_draw_off();
		engine_surf.fast_draw_off();
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

		  inlet_surf.draw_wire();
		  inl_duct_in.draw_wire();
		  inl_duct_out.draw_wire();
		  nozzle_surf.draw_wire();
		  engine_surf.draw_wire();

		glPopMatrix();

		//==== Reflected Geom ====//
		glPushMatrix();
		glMultMatrixf((GLfloat*)reflect_mat); 

		  inlet_surf.draw_refl_wire(sym_code);
		  inl_duct_in.draw_refl_wire(sym_code);
		  inl_duct_out.draw_refl_wire(sym_code);
		  nozzle_surf.draw_refl_wire(sym_code);
		  engine_surf.draw_refl_wire(sym_code);

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
				inlet_surf.draw_shaded();
				inl_duct_in.draw_shaded();
				inl_duct_out.draw_shaded();
				nozzle_surf.draw_shaded();
				engine_surf.draw_shaded();
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
			{
				inlet_surf.draw_refl_shaded( sym_code);
				inl_duct_in.draw_refl_shaded( sym_code);
				inl_duct_out.draw_refl_shaded( sym_code);
				nozzle_surf.draw_refl_shaded( sym_code);
				engine_surf.draw_refl_shaded( sym_code);
			}
		}
		glPopMatrix();
	}
	else if ( displayFlag == GEOM_HIDDEN_FLAG )
	{
		//==== Draw Geom ====//
		glPushMatrix();
		glMultMatrixf((GLfloat*)model_mat); 
			glColor3ub( (int)color.x(), (int)color.y(), (int)color.z() );	
			inlet_surf.draw_hidden();
			glColor3ub( (int)color.x(), (int)color.y(), (int)color.z() );	
			inl_duct_in.draw_hidden();
			glColor3ub( (int)color.x(), (int)color.y(), (int)color.z() );	
			inl_duct_out.draw_hidden();
			glColor3ub( (int)color.x(), (int)color.y(), (int)color.z() );	
			nozzle_surf.draw_hidden();
			glColor3ub( (int)color.x(), (int)color.y(), (int)color.z() );	
			engine_surf.draw_hidden();
		glPopMatrix();

		//==== Reflected Geom ====//
		glPushMatrix();
		glMultMatrixf((GLfloat*)reflect_mat);
			glColor3ub( (int)color.x(), (int)color.y(), (int)color.z() );	
			inlet_surf.draw_refl_hidden( sym_code);
			glColor3ub( (int)color.x(), (int)color.y(), (int)color.z() );	
			inl_duct_in.draw_refl_hidden( sym_code);
			glColor3ub( (int)color.x(), (int)color.y(), (int)color.z() );	
			inl_duct_out.draw_refl_hidden( sym_code);
			glColor3ub( (int)color.x(), (int)color.y(), (int)color.z() );	
			nozzle_surf.draw_refl_hidden( sym_code);
			glColor3ub( (int)color.x(), (int)color.y(), (int)color.z() );	
			engine_surf.draw_refl_hidden( sym_code);
		glPopMatrix();		
	}
*/

}

//==== Draw If Alpha < 1 and Shaded ====//
void EngineGeom::drawAlpha()
{
	Geom::drawAlpha();
/*

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
			inlet_surf.draw_shaded();
			inl_duct_in.draw_shaded();
			inl_duct_out.draw_shaded();
			nozzle_surf.draw_shaded();
			engine_surf.draw_shaded();

		glPopMatrix();

		//==== Reflected Geom ====//
		glPushMatrix();
		glMultMatrixf((GLfloat*)reflect_mat); 

		mat->bind();
			inlet_surf.draw_refl_shaded(sym_code);
			inl_duct_in.draw_refl_shaded(sym_code);
			inl_duct_out.draw_refl_shaded(sym_code);
			nozzle_surf.draw_refl_shaded(sym_code);
			engine_surf.draw_refl_shaded(sym_code);

		glPopMatrix();
	}
*/

}

void EngineGeom::scale()
{
	double current_factor = scaleFactor()*(1.0/lastScaleFactor);

	rad_tip.set( rad_tip()*current_factor );

	lastScaleFactor = scaleFactor();

}


void EngineGeom::acceptScaleFactor()
{
	lastScaleFactor = 1.0;
	scaleFactor.set(1.0);

}

void EngineGeom::resetScaleFactor()
{
	scaleFactor.set( 1.0 );
    scale();
	lastScaleFactor = 1.0;

	generate();
		
	//==== Tell Aircraft You Have Changed ====//
	airPtr->geomMod( this );
}

//==== Compute And Load Normals ====//
void EngineGeom::load_normals()
{
  engine_surf.load_normals();
  nozzle_surf.load_normals();
  inlet_surf.load_normals();
  inl_duct_in.load_normals();
  inl_duct_out.load_normals();
}

//==== Draw Other Pnts XSecs====//
void EngineGeom::load_hidden_surf()
{
  engine_surf.load_hidden_surf();
  inlet_surf.load_hidden_surf();
  nozzle_surf.load_hidden_surf();
  inl_duct_in.load_hidden_surf();
  inl_duct_out.load_hidden_surf();

}

//==== Write Rhino File ====//
void EngineGeom::write_rhino_file(ON_BinaryFile* archive, ON_3dmObjectAttributes* attributes)
{
	load_simp_surf();

	simp_surf.clear_pnt_tan_flags();
	simp_surf.write_rhino_file( sym_code, model_mat, reflect_mat, archive, attributes );

}
void EngineGeom::load_simp_surf()
{
	vec3d pnt;
	int i, j;
	simp_surf.set_num_pnts( NUM_PNTS_XSEC );

	int tnum =  NUM_HUB_XSECS + NXSEC_1 + NXSEC_2 + NXSEC_3 + NUM_NOZ_XSECS -1 + 1;
	simp_surf.set_num_xsecs( tnum );

	if ( inl_duct_flag )
	{
		simp_surf.set_num_xsecs( tnum + 2*NUM_INL_DUCT_XSECS - 2 );
	}

	int xcnt = 0;
	for ( i = 0 ; i < NUM_HUB_XSECS + 1 ; i++ )
	{
		for ( j = 0 ; j < NUM_PNTS_XSEC ; j++ )
		{
			pnt = engine_surf.get_pnt( i, j );
			simp_surf.set_pnt( xcnt, j, pnt );
		}
		xcnt++;
	}
	if ( inl_duct_flag )
	{
		for ( i = 1 ; i < NUM_INL_DUCT_XSECS ; i++ )
		{
			for ( j = 0 ; j < NUM_PNTS_XSEC ; j++ )
			{
				pnt = inl_duct_in.get_pnt( i, j );
				simp_surf.set_pnt( xcnt, j, pnt );
			}
			xcnt++;
		}
	}

	for ( i = 1 ; i < NXSEC_1 ; i++ )
	{
		for ( j = 0 ; j < NUM_PNTS_XSEC ; j++ )
		{
			pnt = inlet_surf.get_pnt( i, j );
			simp_surf.set_pnt( xcnt, j, pnt );
		}
		xcnt++;
	}
	for ( i = NXSEC_1 ; i < NXSEC_1+NXSEC_2 ; i++ )
	{
		for ( j = 0 ; j < NUM_PNTS_XSEC ; j++ )
		{
			pnt = inlet_surf.get_pnt( i, j );
			simp_surf.set_pnt( xcnt, j, pnt );
		}
		xcnt++;
	}
	for ( i = NXSEC_1+NXSEC_2 ; i < NXSEC_1+NXSEC_2+NXSEC_3 ; i++ )
	{
		for ( j = 0 ; j < NUM_PNTS_XSEC ; j++ )
		{
			pnt = inlet_surf.get_pnt( i, j );
			simp_surf.set_pnt( xcnt, j, pnt );
		}
		xcnt++;
	}


	if ( inl_duct_flag )
	{
		for ( i = 1 ; i < NUM_INL_DUCT_XSECS ; i++ )
		{
			for ( j = 0 ; j < NUM_PNTS_XSEC ; j++ )
			{
				pnt = inl_duct_out.get_pnt( i, j );
				simp_surf.set_pnt( xcnt, j, pnt );
			}
			xcnt++;
		}
	}

	for ( i = 1 ; i < NUM_NOZ_XSECS ; i++ )
	{
		for ( j = 0 ; j < NUM_PNTS_XSEC ; j++ )
		{
			pnt = nozzle_surf.get_pnt( i, j );
			simp_surf.set_pnt( xcnt, j, pnt );
		}
		xcnt++;
	}
	for ( j = 0 ; j < NUM_PNTS_XSEC ; j++ )
	{
		pnt = engine_surf.get_pnt( NUM_HUB_XSECS + NUM_ENG_XSECS - 1, j );
		simp_surf.set_pnt( xcnt, j, pnt );
	}
	xcnt++;


}

//==== Convert To Tri Mesh ====//
vector< TMesh* > EngineGeom:: createTMeshVec()
{
	load_simp_surf();

	TMesh* tmPtr;
	vector< TMesh* > tMeshVec;

	tmPtr = simp_surf.createTMesh(model_mat);
	tMeshVec.push_back( tmPtr );

	if ( sym_code != NO_SYM )
	{
		simp_surf.set_sym_code( sym_code );
		simp_surf.load_refl_pnts_xsecs();
		tmPtr = simp_surf.createReflTMesh(sym_code, reflect_mat);
		tMeshVec.push_back( tmPtr );
	}
	return tMeshVec;
}



//==== Dump Xsec File =====//
void EngineGeom::dump_xsec_file(int geom_no, FILE* dump_file)
{  
  int i, num_xsecs;
  if ( inl_duct_flag )
    num_xsecs = NXSEC_3 + NUM_INL_DUCT_XSECS + NOZ_NX_1;
  else 
    num_xsecs = NXSEC_3 + NOZ_NX_1;
  
  fprintf(dump_file, "\n");
  fprintf(dump_file, "%s \n",(char*) getName());
  fprintf(dump_file, " GROUP NUMBER      = %d \n",geom_no);
  fprintf(dump_file, " TYPE              = 1\n");
  fprintf(dump_file, " CROSS SECTIONS    = %d \n",num_xsecs);
  fprintf(dump_file, " PTS/CROSS SECTION = %d \n",NUM_PNTS_XSEC);

  for ( i = NXSEC_1+NXSEC_2 ; i < NXSEC_1+NXSEC_2+NXSEC_3 ; i++ )
    {
      inlet_surf.write_xsec(i, model_mat, dump_file);    
    }
  if ( inl_duct_flag )
    {
      for ( i = 0 ; i < NUM_INL_DUCT_XSECS ; i++ )
        {
          inl_duct_out.write_xsec(i, model_mat, dump_file);    
        }
    }
  for ( i = 0 ; i < NOZ_NX_1 ; i++ )
    {
      nozzle_surf.write_xsec(i, model_mat, dump_file);    
    }

  if ( sym_code == NO_SYM ) return;

  fprintf(dump_file, "\n");
  fprintf(dump_file, "%s \n",(char*) getName());
  fprintf(dump_file, " GROUP NUMBER      = %d \n",geom_no);
  fprintf(dump_file, " TYPE              = 1 \n");
  fprintf(dump_file, " CROSS SECTIONS    = %d \n",num_xsecs);
  fprintf(dump_file, " PTS/CROSS SECTION = %d \n",NUM_PNTS_XSEC);

  for ( i = NXSEC_1+NXSEC_2 ; i < NXSEC_1+NXSEC_2+NXSEC_3 ; i++ )
    {
      inlet_surf.write_refl_xsec(sym_code, i, reflect_mat, dump_file);    
    }
  if ( inl_duct_flag )
    {
      for ( i = 0 ; i < NUM_INL_DUCT_XSECS ; i++ )
        {
          inl_duct_out.write_refl_xsec(sym_code, i, reflect_mat, dump_file);    
        }
    }
  for ( i = 0 ; i < NOZ_NX_1 ; i++ )
    {
      nozzle_surf.write_refl_xsec(sym_code, i, reflect_mat, dump_file);    
    }
}




//==== Return Number Of Felisa Surface Written Out ====//
int EngineGeom::get_num_felisa_comps()
{
    return(4);
}

/*****************************************************************
******************************************************************
******************************************************************
//==== Write Out Felisa Surfaces To File ====//
int EngineGeom::write_felisa_file(int geom_no, FILE* dump_file)
{
  int num_pnts = NUM_PNTS_XSEC/2 + 1;
  int quart = (NUM_PNTS_XSEC-1)/4;

  int num_duct = 0;
  if ( inl_duct_flag ) num_duct = NUM_INL_DUCT_XSECS;

  int num_geoms = geom_no;
  int i, j;
  vec3d temp; 

  //==== Write Outer Inlet Surface (Half) ====//
  int total_num_xsecs = NXSEC_3 + NOZ_NX_1 - 1;
  if ( inl_duct_flag ) total_num_xsecs += num_duct - 1;
 
  fprintf(dump_file, "\n");
  fprintf(dump_file, "%d		Surface_Number\n", num_geoms);
  num_geoms++;
  fprintf(dump_file, "%s		Name\n", name());
  fprintf(dump_file, "2			U_Render\n");
  fprintf(dump_file, "2			W_Render\n");
  fprintf(dump_file, "%d		Num_xsecs\n",total_num_xsecs);
  fprintf(dump_file, "%d		Num_pnts\n", num_pnts);

  //==== Inlet Outer Xsecs ====//
  for (  i = 0 ; i < NXSEC_3 ; i++ )
    {
      for (  j = quart ; j < 3*quart+1 ; j++ )
        {
          temp = inlet_surf.get_pnt(i+NXSEC_1+NXSEC_2, j).transform(model_mat);
          fprintf(dump_file, "%11.6f  %11.6f  %11.6f\n", temp.x(), temp.y(),temp.z());
        } 
    }
  
  //==== Duct Outer Xsecs ====//
  for (  i = 1 ; i < num_duct ; i++ )
    {
      for (  j = quart ; j < 3*quart+1 ; j++ )
        {
          temp = inl_duct_out.get_pnt(i, j).transform(model_mat);
          fprintf(dump_file, "%11.6f  %11.6f  %11.6f\n", temp.x(), temp.y(),temp.z());
        } 
    }
  
  //==== Nozzle Outer Xsecs ====//
  for (  i = 1 ; i < NOZ_NX_1 ; i++ )
    {
      for (  j = quart ; j < 3*quart+1 ; j++ )
        {
          temp = nozzle_surf.get_pnt(i, j).transform(model_mat);
          fprintf(dump_file, "%11.6f  %11.6f  %11.6f\n", temp.x(), temp.y(),temp.z());
        } 
    }


  //==== Write Outer Inlet Surface (Half) ====//
  fprintf(dump_file, "\n");
  fprintf(dump_file, "%d		Surface_Number\n", num_geoms);
  num_geoms++;
  fprintf(dump_file, "%s		Name\n", name());
  fprintf(dump_file, "2			U_Render\n");
  fprintf(dump_file, "2			W_Render\n");
  fprintf(dump_file, "%d		Num_xsecs\n",total_num_xsecs);
  fprintf(dump_file, "%d		Num_pnts\n", num_pnts);

  //==== Inlet Outer Xsecs ====//
  for (  i = 0 ; i < NXSEC_3 ; i++ )
    {
      for (  j = 3*quart ; j < NUM_PNTS_XSEC ; j++ )
        {
          temp = inlet_surf.get_pnt(i+NXSEC_1+NXSEC_2, j).transform(model_mat);
          fprintf(dump_file, "%11.6f  %11.6f  %11.6f\n", temp.x(), temp.y(),temp.z());
        } 
      for (  j = 1 ; j < quart+1 ; j++ )
        {
          temp = inlet_surf.get_pnt(i+NXSEC_1+NXSEC_2, j).transform(model_mat);
          fprintf(dump_file, "%11.6f  %11.6f  %11.6f\n", temp.x(), temp.y(),temp.z());
        } 
    }
  
  //==== Duct Outer Xsecs ====//
  for (  i = 1 ; i < num_duct ; i++ )
    {
      for (  j = 3*quart ; j < NUM_PNTS_XSEC ; j++ )
        {
          temp = inl_duct_out.get_pnt(i, j).transform(model_mat);
          fprintf(dump_file, "%11.6f  %11.6f  %11.6f\n", temp.x(), temp.y(),temp.z());
        } 
      for (  j = 1 ; j < quart+1 ; j++ )
        {
          temp = inl_duct_out.get_pnt(i, j).transform(model_mat);
          fprintf(dump_file, "%11.6f  %11.6f  %11.6f\n", temp.x(), temp.y(),temp.z());
        } 
    }
  
  //==== Nozzle Outer Xsecs ====//
  for (  i = 1 ; i < NOZ_NX_1 ; i++ )
    {
      for (  j = 3*quart ; j < NUM_PNTS_XSEC ; j++ )
        {
          temp = nozzle_surf.get_pnt(i, j).transform(model_mat);
          fprintf(dump_file, "%11.6f  %11.6f  %11.6f\n", temp.x(), temp.y(),temp.z());
        } 
      for (  j = 1 ; j < quart+1 ; j++ )
        {
          temp = nozzle_surf.get_pnt(i, j).transform(model_mat);
          fprintf(dump_file, "%11.6f  %11.6f  %11.6f\n", temp.x(), temp.y(),temp.z());
        } 
    }

  //==== Write Inner Inlet Surface (Half) ====//
  total_num_xsecs = ( NXSEC_1 + NXSEC_2) + NOZ_NX_2 + (NUM_ENG_XSECS -3);
  if ( inl_duct_flag ) total_num_xsecs += num_duct;
 
  fprintf(dump_file, "\n");
  fprintf(dump_file, "%d		Surface_Number\n", num_geoms);
  num_geoms++;
  fprintf(dump_file, "%s		Name\n", name());
  fprintf(dump_file, "2			U_Render\n");
  fprintf(dump_file, "2			W_Render\n");
  fprintf(dump_file, "%d		Num_xsecs\n",total_num_xsecs);
  fprintf(dump_file, "%d		Num_pnts\n", num_pnts);

  //==== Nozzle Inner Xsecs ====//
  for (  i = 0 ; i < NOZ_NX_2 ; i++ )
    {
      for (  j = quart ; j < 3*quart+1 ; j++ )
        {
          temp = nozzle_surf.get_pnt(i + NOZ_NX_1, j).transform(model_mat);
          fprintf(dump_file, "%11.6f  %11.6f  %11.6f\n", temp.x(), temp.y(),temp.z());
        } 
    }

  //==== Engine Inner Xsecs ====//
  for (  i = NUM_ENG_XSECS-3 ; i > 0 ; i-- )
    {
      for (  j = quart ; j < 3*quart+1 ; j++ )
        {
          temp = pnt_xsec(i + NUM_HUB_XSECS, j).transform(model_mat);
          fprintf(dump_file, "%11.6f  %11.6f  %11.6f\n", temp.x(), temp.y(),temp.z());
        } 
    }

  //==== Duct Inner Xsecs ====//
  for (  i = 0 ; i < num_duct ; i++ )
    {
      for (  j = quart ; j < 3*quart+1 ; j++ )
        {
          temp = inl_duct_in.get_pnt(i, j).transform(model_mat);
          fprintf(dump_file, "%11.6f  %11.6f  %11.6f\n", temp.x(), temp.y(),temp.z());
        } 
    }

  //==== Inlet Inner Xsecs ====//
  for (  i = 1 ; i <= NXSEC_1+NXSEC_2 ; i++ )
    {
      for (  j = quart ; j < 3*quart+1 ; j++ )
        {
          temp = inlet_surf.get_pnt(i, j).transform(model_mat);
          fprintf(dump_file, "%11.6f  %11.6f  %11.6f\n", temp.x(), temp.y(),temp.z());
        } 
    }
  

  //==== Write Outer Inlet Surface (Half) ====//
  fprintf(dump_file, "\n");
  fprintf(dump_file, "%d		Surface_Number\n", num_geoms);
  num_geoms++;
  fprintf(dump_file, "%s		Name\n", name());
  fprintf(dump_file, "2			U_Render\n");
  fprintf(dump_file, "2			W_Render\n");
  fprintf(dump_file, "%d		Num_xsecs\n",total_num_xsecs);
  fprintf(dump_file, "%d		Num_pnts\n", num_pnts);

  //==== Nozzle Inner Xsecs ====//
  for (  i = 0 ; i < NOZ_NX_2 ; i++ )
    {
      for (  j = 3*quart ; j < NUM_PNTS_XSEC ; j++ )
        {
          temp = nozzle_surf.get_pnt(i + NOZ_NX_1, j).transform(model_mat);
          fprintf(dump_file, "%11.6f  %11.6f  %11.6f\n", temp.x(), temp.y(),temp.z());
        } 
      for (  j = 1 ; j < quart+1 ; j++ )
        {
          temp = nozzle_surf.get_pnt(i + NOZ_NX_1, j).transform(model_mat);
          fprintf(dump_file, "%11.6f  %11.6f  %11.6f\n", temp.x(), temp.y(),temp.z());
        } 
    }

  //==== Engine Inner Xsecs ====//
  for (  i = NUM_ENG_XSECS-3 ; i > 0 ; i-- )
    {
      for (  j = 3*quart ; j < NUM_PNTS_XSEC ; j++ )
        {
          temp = pnt_xsec(i + NUM_HUB_XSECS, j).transform(model_mat);
          fprintf(dump_file, "%11.6f  %11.6f  %11.6f\n", temp.x(), temp.y(),temp.z());
        } 
      for (  j = 1 ; j < quart+1 ; j++ )
        {
          temp = pnt_xsec(i + NUM_HUB_XSECS, j).transform(model_mat);
          fprintf(dump_file, "%11.6f  %11.6f  %11.6f\n", temp.x(), temp.y(),temp.z());
        } 
    }

  //==== Duct Inner Xsecs ====//
  for (  i = 0 ; i < num_duct ; i++ )
    {
      for (  j = 3*quart ; j < NUM_PNTS_XSEC ; j++ )
        {
          temp = inl_duct_in.get_pnt(i, j).transform(model_mat);
          fprintf(dump_file, "%11.6f  %11.6f  %11.6f\n", temp.x(), temp.y(),temp.z());
        } 
      for (  j = 1 ; j < quart+1 ; j++ )
        {
          temp = inl_duct_in.get_pnt(i, j).transform(model_mat);
          fprintf(dump_file, "%11.6f  %11.6f  %11.6f\n", temp.x(), temp.y(),temp.z());
        } 
    }

  //==== Inlet Inner Xsecs ====//
  for (  i = 1 ; i <= NXSEC_1+NXSEC_2 ; i++ )
    {
      for (  j = 3*quart ; j < NUM_PNTS_XSEC ; j++ )
        {
          temp = inlet_surf.get_pnt(i, j).transform(model_mat);
          fprintf(dump_file, "%11.6f  %11.6f  %11.6f\n", temp.x(), temp.y(),temp.z());
        } 
      for (  j = 1 ; j < quart+1 ; j++ )
        {
          temp = inlet_surf.get_pnt(i, j).transform(model_mat);
          fprintf(dump_file, "%11.6f  %11.6f  %11.6f\n", temp.x(), temp.y(),temp.z());
        } 
    }

  return(num_geoms);

}
******************************************************************
******************************************************************
*****************************************************************/

//==== Write Out Felisa Surfaces To File ====//
int EngineGeom::write_felisa_file(int geom_no, FILE* dump_file)
{
  int num_pnts = NUM_PNTS_XSEC/2 + 1;
  int quart = (NUM_PNTS_XSEC-1)/4;

  int num_duct = 0;
  if ( inl_duct_flag ) num_duct = NUM_INL_DUCT_XSECS;

  int num_geoms = geom_no;
  int i, j;
  vec3d temp; 

  //==== Write Outer Inlet Surface (Half) ====//
  int total_num_xsecs = NXSEC_2 + NXSEC_3 + NOZ_NX_1 - 1;
  if ( inl_duct_flag ) total_num_xsecs += num_duct - 1;
 
  fprintf(dump_file, "\n");
  fprintf(dump_file, "%d		Surface_Number\n", num_geoms);
  num_geoms++;
  fprintf(dump_file, "%s_1		Name\n", (char*) getName());
  fprintf(dump_file, "2			U_Render\n");
  fprintf(dump_file, "2			W_Render\n");
  fprintf(dump_file, "%d		Num_xsecs\n",total_num_xsecs);
  fprintf(dump_file, "%d		Num_pnts\n", num_pnts);

  //==== Inlet Mid Xsecs ====//
  for (  i = 0 ; i < NXSEC_2 ; i++ )
    {
      for (  j = quart ; j < 3*quart+1 ; j++ )
        {
          temp = inlet_surf.get_pnt(i+NXSEC_1, j).transform(model_mat);
          fprintf(dump_file, "%11.6f  %11.6f  %11.6f\n", temp.x(), temp.y(),temp.z());
        } 
    }

  //==== Inlet Outer Xsecs ====//
  for (  i = 0 ; i < NXSEC_3 ; i++ )
    {
      for (  j = quart ; j < 3*quart+1 ; j++ )
        {
          temp = inlet_surf.get_pnt(i+NXSEC_1+NXSEC_2, j).transform(model_mat);
          fprintf(dump_file, "%11.6f  %11.6f  %11.6f\n", temp.x(), temp.y(),temp.z());
        } 
    }
  
  //==== Duct Outer Xsecs ====//
  for (  i = 1 ; i < num_duct ; i++ )
    {
      for (  j = quart ; j < 3*quart+1 ; j++ )
        {
          temp = inl_duct_out.get_pnt(i, j).transform(model_mat);
          fprintf(dump_file, "%11.6f  %11.6f  %11.6f\n", temp.x(), temp.y(),temp.z());
        } 
    }
  
  //==== Nozzle Outer Xsecs ====//
  for (  i = 1 ; i < NOZ_NX_1 ; i++ )
    {
      for (  j = quart ; j < 3*quart+1 ; j++ )
        {
          temp = nozzle_surf.get_pnt(i, j).transform(model_mat);
          fprintf(dump_file, "%11.6f  %11.6f  %11.6f\n", temp.x(), temp.y(),temp.z());
        } 
    }


  //==== Write Outer Inlet Surface (Half) ====//
  fprintf(dump_file, "\n");
  fprintf(dump_file, "%d		Surface_Number\n", num_geoms);
  num_geoms++;
  fprintf(dump_file, "%s_2		Name\n", (char*) getName());
  fprintf(dump_file, "2			U_Render\n");
  fprintf(dump_file, "2			W_Render\n");
  fprintf(dump_file, "%d		Num_xsecs\n",total_num_xsecs);
  fprintf(dump_file, "%d		Num_pnts\n", num_pnts);

  //==== Inlet Mid Xsecs ====//
  for (  i = 0 ; i < NXSEC_2 ; i++ )
    {
      for (  j = 3*quart ; j < NUM_PNTS_XSEC ; j++ )
        {
          temp = inlet_surf.get_pnt(i+NXSEC_1, j).transform(model_mat);
          fprintf(dump_file, "%11.6f  %11.6f  %11.6f\n", temp.x(), temp.y(),temp.z());
        } 
      for (  j = 1 ; j < quart+1 ; j++ )
        {
          temp = inlet_surf.get_pnt(i+NXSEC_1, j).transform(model_mat);
          fprintf(dump_file, "%11.6f  %11.6f  %11.6f\n", temp.x(), temp.y(),temp.z());
        } 
    }

  //==== Inlet Outer Xsecs ====//
  for (  i = 0 ; i < NXSEC_3 ; i++ )
    {
      for (  j = 3*quart ; j < NUM_PNTS_XSEC ; j++ )
        {
          temp = inlet_surf.get_pnt(i+NXSEC_1+NXSEC_2, j).transform(model_mat);
          fprintf(dump_file, "%11.6f  %11.6f  %11.6f\n", temp.x(), temp.y(),temp.z());
        } 
      for (  j = 1 ; j < quart+1 ; j++ )
        {
          temp = inlet_surf.get_pnt(i+NXSEC_1+NXSEC_2, j).transform(model_mat);
          fprintf(dump_file, "%11.6f  %11.6f  %11.6f\n", temp.x(), temp.y(),temp.z());
        } 
    }
  
  //==== Duct Outer Xsecs ====//
  for (  i = 1 ; i < num_duct ; i++ )
    {
      for (  j = 3*quart ; j < NUM_PNTS_XSEC ; j++ )
        {
          temp = inl_duct_out.get_pnt(i, j).transform(model_mat);
          fprintf(dump_file, "%11.6f  %11.6f  %11.6f\n", temp.x(), temp.y(),temp.z());
        } 
      for (  j = 1 ; j < quart+1 ; j++ )
        {
          temp = inl_duct_out.get_pnt(i, j).transform(model_mat);
          fprintf(dump_file, "%11.6f  %11.6f  %11.6f\n", temp.x(), temp.y(),temp.z());
        } 
    }
  
  //==== Nozzle Outer Xsecs ====//
  for (  i = 1 ; i < NOZ_NX_1 ; i++ )
    {
      for (  j = 3*quart ; j < NUM_PNTS_XSEC ; j++ )
        {
          temp = nozzle_surf.get_pnt(i, j).transform(model_mat);
          fprintf(dump_file, "%11.6f  %11.6f  %11.6f\n", temp.x(), temp.y(),temp.z());
        } 
      for (  j = 1 ; j < quart+1 ; j++ )
        {
          temp = nozzle_surf.get_pnt(i, j).transform(model_mat);
          fprintf(dump_file, "%11.6f  %11.6f  %11.6f\n", temp.x(), temp.y(),temp.z());
        } 
    }

  //==== Write Engine Face ====//
  fprintf(dump_file, "\n");
  fprintf(dump_file, "%d		Surface_Number\n", num_geoms);
  num_geoms++;
  fprintf(dump_file, "%s_Inlet_Face	Name\n", (char*) getName());
  fprintf(dump_file, "2			U_Render\n");
  fprintf(dump_file, "2			W_Render\n");
  fprintf(dump_file, "%d		Num_xsecs\n",2);
  fprintf(dump_file, "%d		Num_pnts\n", num_pnts);

  //==== Inlet Mid Xsecs ====//
  for (  j = quart ; j < 3*quart+1 ; j++ )
    {
      temp = inlet_surf.get_pnt( NXSEC_1, j).transform(model_mat);
      fprintf(dump_file, "%11.6f  %11.6f  %11.6f\n", temp.x(), temp.y(),temp.z());
    } 

   for (  j = quart ; j >= 1 ; j-- )
     {
       temp = inlet_surf.get_pnt(NXSEC_1, j).transform(model_mat);
       fprintf(dump_file, "%11.6f  %11.6f  %11.6f\n", temp.x(), temp.y(),temp.z());
     } 
   for (  j = NUM_PNTS_XSEC-1 ; j >= 3*quart ; j-- )
     {
       temp = inlet_surf.get_pnt(NXSEC_1, j).transform(model_mat);
       fprintf(dump_file, "%11.6f  %11.6f  %11.6f\n", temp.x(), temp.y(),temp.z());
     } 


  //==== Write Nozzle Face ====//
 
  fprintf(dump_file, "\n");
  fprintf(dump_file, "%d		Surface_Number\n", num_geoms);
  num_geoms++;
  fprintf(dump_file, "%s_Exit_Face	Name\n", (char*) getName());
  fprintf(dump_file, "2			U_Render\n");
  fprintf(dump_file, "2			W_Render\n");
  fprintf(dump_file, "%d		Num_xsecs\n",2);
  fprintf(dump_file, "%d		Num_pnts\n", num_pnts);

  //==== Nozzle Inner Xsecs ====//
  for (  j = quart ; j < 3*quart+1 ; j++ )
    {
      temp = nozzle_surf.get_pnt(NOZ_NX_1, j).transform(model_mat);
      fprintf(dump_file, "%11.6f  %11.6f  %11.6f\n", temp.x(), temp.y(),temp.z());
    } 

   for (  j = quart ; j >= 1 ; j-- )
    {
      temp = nozzle_surf.get_pnt(NOZ_NX_1, j).transform(model_mat);
      fprintf(dump_file, "%11.6f  %11.6f  %11.6f\n", temp.x(), temp.y(),temp.z());
    } 
   for (  j = NUM_PNTS_XSEC-1 ; j >= 3*quart ; j-- )
    {
      temp = nozzle_surf.get_pnt(NOZ_NX_1, j).transform(model_mat);
      fprintf(dump_file, "%11.6f  %11.6f  %11.6f\n", temp.x(), temp.y(),temp.z());
    } 

  return(num_geoms);

}


//==== Write Out Felisa BCO Info To File ====//
void EngineGeom::write_bco_info(FILE* file_ptr, int& surf_cnt, int&)
{
  fprintf( file_ptr, "%d        0      0     0.0     0.0     0.0     0.0   %s 1\n", surf_cnt, (char*) getName() );
  surf_cnt++;
  fprintf( file_ptr, "%d        0      0     0.0     0.0     0.0     0.0   %s 2\n", surf_cnt, (char*) getName() );
  surf_cnt++;
  fprintf( file_ptr, "%d        1      0     0.4     0.0     0.0     0.0   %s Inlet Face\n", surf_cnt, (char*) getName() );
  surf_cnt++;
  fprintf( file_ptr, "%d        1      0    -1.0     0.0     0.0     0.0   %s Exit Face\n", surf_cnt, (char*) getName() );
  surf_cnt++;
}

        
//==== Return Number Of FELISA Background Lines ====//
int EngineGeom::get_num_felisa_line_sources()
{
  int total_num_xsecs = NXSEC_3 + NOZ_NX_1 - 1;
  if ( inl_duct_flag ) total_num_xsecs += NUM_INL_DUCT_XSECS - 1;

  return(total_num_xsecs - 1);
}

//==== Write Out Felisa Background Lines File ====//
void EngineGeom::write_felisa_line_sources(FILE* dump_file)
{
  int i;
  vec3d temp;

  //==== Check For Duct ====/
  int num_duct = 0;
  if ( inl_duct_flag ) num_duct = NUM_INL_DUCT_XSECS;

  int total_num_xsecs = NXSEC_3 + NOZ_NX_1 - 1;
  if ( inl_duct_flag ) total_num_xsecs += num_duct - 1;

  //==== Find Xsec Center Pnts Outer Inlet ====/
  //sdyn_array<vec3d> cent_pnts;
  //cent_pnts.init( total_num_xsecs );
  vector<vec3d> cent_pnts;
  cent_pnts.resize( total_num_xsecs );
  dyn_array<double> max_dists;
  max_dists.init(total_num_xsecs);

  int cnt = 0;
  //==== Inlet Outer Xsecs ====//
  for (  i = 0 ; i < NXSEC_3; i++ )
    {
      cent_pnts[cnt] = inlet_surf.get_xsec_center( i+NXSEC_1+NXSEC_2 );
      max_dists[cnt] = inlet_surf.get_max_dist_from_xsec
                                   (i+NXSEC_1+NXSEC_2,cent_pnts[cnt]); 
      cnt++;
    }

  //==== Duct Outer Xsecs ====//
  for (  i = 1 ; i < num_duct ; i++ )
    {
      cent_pnts[cnt] = inl_duct_out.get_xsec_center( i );
      max_dists[cnt] = inl_duct_out.get_max_dist_from_xsec(i,cent_pnts[cnt]); 
      cnt++;
    }

  //==== Nozzle Outer Xsecs ====//
  for (  i = 1 ; i < NOZ_NX_1 ; i++ )
    {
      cent_pnts[cnt] = nozzle_surf.get_xsec_center( i );
      max_dists[cnt] = nozzle_surf.get_max_dist_from_xsec(i,cent_pnts[cnt]); 
      cnt++;
    }

  //==== Dump Lines ====/
  for (  i = 0 ; i < cnt-1 ; i++ )
    {
      fprintf(dump_file, "  Engine Line %d \n", i);

      temp = cent_pnts[i].transform(model_mat);
      
      fprintf(dump_file, "    %f    %f    %f    %f    %f    %f  \n",
          temp.x(), temp.y(), temp.z(), max_dists[i]*0.25, 1.125*max_dists[i], 3.0*max_dists[i]);

      temp = cent_pnts[i+1].transform(model_mat);
      fprintf(dump_file, "    %f    %f    %f    %f    %f    %f  \n",
          temp.x(), temp.y(), temp.z(), max_dists[i+1]*0.25, 1.125*max_dists[i+1], 3.0*max_dists[i+1]);
     }

}


//====Return Number Of FELISA Background Tris ====//
int EngineGeom::get_num_felisa_tri_sources()
{
  int num_xsecs = NUM_INL_XSECS + NUM_NOZ_XSECS - 1;
  if ( inl_duct_flag ) 
    num_xsecs += NUM_INL_DUCT_XSECS + NUM_INL_DUCT_XSECS - 2;

  int num_tris = (num_xsecs - 1)*NUM_PNTS_XSEC*2;

  return( num_tris ); 

  return( (NUM_PNTS_XSEC-1)*2 );
//  return(0);
}

//==== Write Out Felisa Background Tris File ====//
void EngineGeom::write_felisa_tri_sources(FILE* dump_file)
{
  int j;
  vec3d pnt1, pnt2, pnt3, pnt4;

  float space = 0.4f;
  float dist1 = 1.2f;
  float dist2 = 2.4f;

  int inl_id = NXSEC_1 + NXSEC_2 -1;
  int noz_id = NOZ_NX_1 - 1; 

  //==== Inlet Outer Xsecs ====//
  for (  j = 0 ; j < NUM_PNTS_XSEC-1 ; j++ )
    {
      pnt1 = inlet_surf.get_pnt(inl_id,   j).transform(model_mat);        
      pnt2 = nozzle_surf.get_pnt(noz_id,  j).transform(model_mat);        
      pnt3 = nozzle_surf.get_pnt(noz_id,j+1).transform(model_mat);        
      pnt4 = inlet_surf.get_pnt(inl_id, j+1).transform(model_mat);        

          fprintf(dump_file, " %s: Inlet Tri %d \n",getName().get_char_star(), j);
          fprintf(dump_file, "    %f    %f    %f    %f    %f    %f  \n",
            pnt1.x(), pnt1.y(), pnt1.z(), space, dist1, dist2);
          fprintf(dump_file, "    %f    %f    %f    %f    %f    %f  \n",
            pnt2.x(), pnt2.y(), pnt2.z(), space, dist1, dist2);
          fprintf(dump_file, "    %f    %f    %f    %f    %f    %f  \n",
            pnt3.x(), pnt3.y(), pnt3.z(), space, dist1, dist2);

          fprintf(dump_file, " %s: Inlet Tri %d \n",getName().get_char_star(), j);
          fprintf(dump_file, "    %f    %f    %f    %f    %f    %f  \n",
            pnt1.x(), pnt1.y(), pnt1.z(), space, dist1, dist2);
          fprintf(dump_file, "    %f    %f    %f    %f    %f    %f  \n",
            pnt3.x(), pnt3.y(), pnt3.z(), space, dist1, dist2);
          fprintf(dump_file, "    %f    %f    %f    %f    %f    %f  \n",
            pnt4.x(), pnt4.y(), pnt4.z(), space, dist1, dist2);
    }
}

vec3d EngineGeom::getVertex3d(int surfid, double x, double p, int r)			
{ 
	switch (surfid)
	{
		case INLET_SURF:
			return inlet_surf.get_vertex(x, p, r);
		case INL_DUCT_IN_SURF:
			return inl_duct_in.get_vertex(x, p, r);
		case INL_DUCT_OUT_SURF:
			return inl_duct_out.get_vertex(x, p, r);
		case NOZZLE_SURF:
			return nozzle_surf.get_vertex(x, p, r);
		case ENGINE_SURF:
			return engine_surf.get_vertex(x, p, r);
		default:
			return vec3d(0,0,0);
	}
}

void  EngineGeom::getVertexVec(vector< VertexID > *vertVec)
{ 
	buildVertexVec(&inlet_surf, INLET_SURF, vertVec); 
	buildVertexVec(&inl_duct_in, INL_DUCT_IN_SURF, vertVec); 
	buildVertexVec(&inl_duct_out, INL_DUCT_OUT_SURF, vertVec); 
	buildVertexVec(&nozzle_surf, NOZZLE_SURF, vertVec); 
	buildVertexVec(&engine_surf, ENGINE_SURF, vertVec); 
}

int  EngineGeom::get_num_bezier_comps()
{
	if ( sym_code == NO_SYM )
		return 1;
	else 
		return 2;
}

void EngineGeom::write_bezier_file( int id, FILE* file_id )
{
	Xsec_surf srf;

	srf.set_num_pnts( engine_surf.get_num_pnts() );

	int nxs = NUM_HUB_XSECS +  NUM_INL_XSECS + NOZ_NX_1;
	if ( inl_duct_flag )
		nxs += 2*NUM_INL_DUCT_XSECS - 1;

	srf.set_num_xsecs( nxs );

	int cnt = 0;
	for ( int i = 0 ; i < NUM_HUB_XSECS ; i++ )
	{
		srf.copy_xsec( &engine_surf, i, cnt );			cnt++;
	}

	if ( inl_duct_flag )
	{
		for ( int i = 0 ; i <NUM_INL_DUCT_XSECS-1 ; i++ )
		{
			srf.copy_xsec( &inl_duct_in, i, cnt  );		cnt++;
		}
	}

	for ( int i = 0 ; i < NUM_INL_XSECS-1 ; i++ )
	{
		srf.copy_xsec( &inlet_surf, i, cnt  );			cnt++;
	}

	if ( inl_duct_flag )
	{
		for ( int i = 0 ; i <NUM_INL_DUCT_XSECS ; i++ )
		{
			srf.copy_xsec( &inl_duct_out, i, cnt  );		cnt++;
		}
	}

	for ( int i = 0 ; i < NOZ_NX_1 ; i++ )
	{
		srf.copy_xsec( &nozzle_surf, i, cnt );			cnt++;
	}

	srf.copy_xsec( &nozzle_surf, NOZ_NX_1-1, cnt  );	cnt++;

	//==== Collapse To Point ====//
	vec3d center = srf.get_xsec_center( srf.get_num_xsecs()-1 );
	for ( int i = 0 ; i < srf.get_num_pnts() ; i++ )
	{
		srf.set_pnt( srf.get_num_xsecs()-1, i, center );
	}

//for ( int i = 0 ; i < srf.get_num_xsecs() ; i++ )
//{
//	printf( "%d: %f \n", i, srf.get_pnt( i, 0 ).x() );
//}

	srf.clear_xsec_tan_flags();

	srf.set_xsec_tan_flag( 0, Bezier_curve::NADA );
	srf.set_xsec_tan_flag(NUM_HUB_XSECS-1, Bezier_curve::SHARP );
	srf.set_xsec_tan_flag(NUM_HUB_XSECS, Bezier_curve::SHARP );
	srf.set_xsec_tan_flag( srf.get_num_xsecs()-2, Bezier_curve::SHARP );
	srf.set_xsec_tan_flag( srf.get_num_xsecs()-1, Bezier_curve::NADA );

	vector <int> u_split;
	u_split.push_back( 0 );
	u_split.push_back( 3*(NUM_HUB_XSECS-1) );
	u_split.push_back( 3*(NUM_HUB_XSECS) );
	if ( inl_duct_flag )
		u_split.push_back( 3*(NUM_HUB_XSECS + NXSEC_1 + NXSEC_2/2 + NUM_INL_DUCT_XSECS ) );
	else
		u_split.push_back( 3*(NUM_HUB_XSECS + NXSEC_1 + NXSEC_2/2 + 1) );
	u_split.push_back( 3*(srf.get_num_xsecs() - 2)  );
	u_split.push_back( 3*(srf.get_num_xsecs() - 1)  );

	vector <int> w_split;
	w_split.push_back( 0 );
	w_split.push_back( 3*(srf.get_num_pnts()/4)  );
	w_split.push_back( 3*(srf.get_num_pnts()/2)  );
	w_split.push_back( 3*3*(srf.get_num_pnts()/4)  );
	w_split.push_back( 3*(srf.get_num_pnts() - 1)  );

	srf.write_bezier_file( file_id, sym_code, model_mat, reflect_mat, u_split, w_split  );
}


//==== Aero Ref Parameters ====//
double EngineGeom::getRefArea()
{
	if ( autoRefAreaFlag )
	{
		double eng_length = length()*rad_tip(); 
		refArea = 2.0*rad_tip()*eng_length;
	}
	return refArea;
}
	
double EngineGeom::getRefSpan()
{
	if ( autoRefSpanFlag )
	{
		refSpan = 2.0*rad_tip();
	}
	return refSpan;
}

double EngineGeom::getRefCbar()
{
	if ( autoRefCbarFlag )
	{
		double eng_length = length()*rad_tip(); 
		refCbar = eng_length;
	}
	return refCbar;
}

vec3d EngineGeom::getAeroCenter()
{
	if ( autoAeroCenterFlag )
	{
		double eng_length = length()*rad_tip(); 
		aeroCenter.set_x( eng_length*0.25 + get_tran_x() );
		aeroCenter.set_y( get_tran_y() );
		aeroCenter.set_z( get_tran_z() );
	}
	return aeroCenter;
}

void EngineGeom::GetInteriorPnts( vector< vec3d > & pVec )
{
	double eng_length = length()*rad_tip(); 
	vec3d p( eng_length*0.5, 0.0, 0.0 );
	vec3d tp = p.transform( model_mat );
	pVec.push_back( tp ); 

	if (sym_code != NO_SYM)
	{
		tp = (p * sym_vec).transform(reflect_mat);
		pVec.push_back( tp );
    }
}



