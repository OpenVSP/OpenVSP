//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//

//******************************************************************************
//
//    Multi Section Wing Geometry Class
//
//   J.R. Gloudemans - 10/2/01
//       Modified to ms_wing for arbitrary sections
//
//   J.R. Gloudemans - 2/1/95
//   Sterling Software
//       Original strake/aft multi-section wing
//
//******************************************************************************
#include "ms_wing.h"
#include "af.h"
#include "aircraft.h"
#include "materialMgr.h"
#include "wingGeom.h"
#include "FeaMeshMgr.h"
#include "parmLinkMgr.h"

#ifdef __APPLE__
#  include <OpenGL/gl.h>
#else
#  include <GL/gl.h>
#endif

#include "defines.h"


//==== Constructor =====//
WingSect::WingSect()
{
  //==== Load Default Data Vals ====//
  SetDriver( MS_S_TC_RC );
  span = 30.0;
  tc = 10.0;		// Tip Chord
  rc = 20.0;		// Root Chord
  sweep = 10.0;
  sweepLoc = 0.0;	// Sweep Location
  refSweepLoc = sweepLoc();

  twist = 0.0;
  twistLoc = 0.0;	// Sweep Location

  dihedral = 0.0;

  dihed_crv1 = 0.0;
  dihed_crv2 = 0.0;

  dihed_crv1_str = 0.75;
  dihed_crv2_str = 0.75;

  dihedRotFlag = 0;
  smoothBlendFlag = 0;

  num_xsecs = 1;
  num_interp_xsecs = 1;
  num_actual_xsecs = 0;

  rootAf = 0;
  tipAf  = 0;

  fillDependData();

  ar.initialize( NULL, UPD_MSW_SECTS, "Aspect_Ratio", ar() );
  ar.set_lower_upper(0.001, 1000.0);
  tr.initialize( NULL, UPD_MSW_SECTS, "Taper_Ratio", tr() );
  tr.set_lower_upper(0.001, 1000.0);
  area.initialize( NULL, UPD_MSW_SECTS, "Area", area() );
  area.set_lower_upper(0.0001, 1000000.0);
  span.initialize( NULL, UPD_MSW_SECTS, "Span", span() );
  span.set_lower_upper(0.0001, 1000000.0);
  tc.initialize( NULL, UPD_MSW_SECTS, "Tip_Chord", tc() );
  tc.set_lower_upper(0.0001, 1000000.0);
  rc.initialize( NULL, UPD_MSW_SECTS, "Root_Chord", rc() );
  rc.set_lower_upper(0.0001, 1000000.0);
  sweep.initialize( NULL, UPD_MSW_SECTS, "Sweep", sweep() );
  sweep.set_lower_upper(-85.0, 85.0);
  sweepLoc.initialize( NULL, UPD_MSW_SECTS, "Sweep_Location", sweepLoc() );
  sweepLoc.set_lower_upper(0.0, 1.0);
  refSweepLoc = sweepLoc();
  twist.initialize( NULL, UPD_MSW_SECTS, "Twist", twist() );
  twist.set_lower_upper(-45.0, 45.0);
  twistLoc.initialize( NULL, UPD_MSW_SECTS, "Twist_Location", twistLoc() );
  twistLoc.set_lower_upper(0.0, 1.0);
  dihedral.initialize( NULL, UPD_MSW_SECTS, "Dihedral", dihedral() );
  dihedral.set_lower_upper(-360.0, 360.0);
  dihed_crv1.initialize( NULL, UPD_MSW_SECTS, "Dihedral_Curve_1", dihed_crv1() );
  dihed_crv1.set_lower_upper( 0.0, 0.99 );
  dihed_crv2.initialize( NULL, UPD_MSW_SECTS, "Dihedral_Curve_2", dihed_crv2() );
  dihed_crv2.set_lower_upper( 0.0, 0.99 );
  dihed_crv1_str.initialize( NULL, UPD_MSW_SECTS, "Dihedral_Curve_1_Str", dihed_crv1_str() );
  dihed_crv1_str.set_lower_upper( 0.0, 2.0 );
  dihed_crv2_str.initialize( NULL, UPD_MSW_SECTS, "Dihedral_Curve_2_Str", dihed_crv2_str() );
  dihed_crv2_str.set_lower_upper( 0.0, 2.0 );
}

void WingSect::draw()
{
}

void WingSect::SetGeomPtr( Geom* geomPtr )
{
	vector< Parm* > pVec = GetLinkableParms();
	for ( int i = 0 ; i < (int)pVec.size() ; i++ )
	{
		pVec[i]->set_geom( geomPtr );
		pVec[i]->set_base_geom( geomPtr );
	}
}

vector< Parm* > WingSect::GetLinkableParms()
{
	vector< Parm* > pVec;
	pVec.push_back( &ar );
	pVec.push_back( &tr );
	pVec.push_back( &area );
	pVec.push_back( &span );
	pVec.push_back( &tc );
	pVec.push_back( &rc );
	pVec.push_back( &sweep );
	pVec.push_back( &sweepLoc );
	pVec.push_back( &twist );
	pVec.push_back( &twistLoc );
	pVec.push_back( &dihedral );
	pVec.push_back( &dihed_crv1 );
	pVec.push_back( &dihed_crv2 );
	pVec.push_back( &dihed_crv1_str );
	pVec.push_back( &dihed_crv2_str );

	return pVec;
}

//==== Fill Wing Sect Dependant Data ====//
void WingSect::fillDependData()
{
  switch (driver)
    {
      case MS_AR_TR_A:
        span = sqrt( ar()*area() );
        rc   = (2.0*span()) / ( ar()*(1.0+tr()) );
        tc   = tr()*rc();
      break;

      case MS_AR_TR_S:
        area = (span() * span()) / ar();
        rc   = (2.0*span()) / ( ar()*(1.0+tr()) );
        tc   = tr()*rc();
      break;

      case MS_AR_TR_TC:
        rc   = tc()/tr();
        span = 0.5*ar()*rc()*(1.0+tr());
        area = (span() * span()) / ar();
      break;

      case MS_AR_TR_RC:
        tc   = tr()*rc();
        span = 0.5*ar()*rc()*(1.0+tr());
        area = (span() * span()) / ar();
      break;

      case MS_S_TC_RC:
        tr   = tc()/rc();
        ar   = 2.0*span()/( rc()*(1.0+tr()) );
        area = (span() * span()) / ar();
      break;

      case MS_A_TC_RC:
        tr    = tc()/rc();
        ar    = 2.0*span()/( rc()*(1.0+tr()) );
        span  = sqrt( ar()*area() );
      break;

      case MS_TR_S_A:
        ar = (span() * span()) / area();
        rc = (2.0*span()) / ( ar()*(1.0+tr()) );
        tc = tr()*rc();
      break;

      case MS_AR_A_RC:
		span = sqrt( ar() * area());
        tr = (2.0*span())/(ar()*rc()) - 1.0;
        tc = tr()*rc();
      break;

    }
}

//==== Calculate Tan Sweep at Specified Chord Location =====//
double WingSect::tan_sweep_at(double loc, int sym_code, double swp )
{
  double tan_sweep = tan(swp*DEG_2_RAD);
  double tan_sweep_at = tan_sweep - (2.0/ar())*
            ( (loc-sweepLoc()) * ((1.0-tr())/(1.0+tr())) );

  return ( tan_sweep_at );
}


double WingSect::tan_sweep_at( double loc, int sym_code)
{
  double tan_sweep = tan(sweep()*DEG_2_RAD);
  double tan_sweep_at = tan_sweep - (2.0/ar())*
            ( (loc-sweepLoc()) * ((1.0-tr())/(1.0+tr())) );

  return ( tan_sweep_at );
}

double WingSect::tan_sweep_at( double curr_loc, double new_loc, int sym_code )
{
  double tan_sweep = tan(sweep()*DEG_2_RAD);
  double tan_sweep_at = tan_sweep - (2.0/ar())*
            ( (new_loc-curr_loc) * ((1.0-tr())/(1.0+tr())) );

  return ( tan_sweep_at );
}
  
void WingSect::Build( double span_val, double tc_val, double rc_val, double sweep_val, 
					  double dihedral_val, double dihed_crv1_val, double dihed_crv2_val )
{
	span = span_val;
	tc = tc_val;
	rc = rc_val;
	sweep = sweep_val;
	dihedral = dihedral_val;
	dihed_crv1 = dihed_crv1_val;
	dihed_crv2 = dihed_crv2_val;

	fillDependData();

}

//==== Driver Group Has Changed ====//
void WingSect::SetDriver(int driver_in)
{
  int valid_flag = TRUE;

  switch (driver_in)
    {
      case AR_TR_A:
        ar.activate();		tr.activate();		area.activate();
        span.deactivate();  rc.deactivate();	tc.deactivate();
      break;

      case AR_TR_S:
        ar.activate();		tr.activate();			span.activate();
        area.deactivate();  rc.deactivate();		tc.deactivate();
      break;

      case AR_TR_TC:
        ar.activate();		tr.activate();			tc.activate();
        area.deactivate();  rc.deactivate();		span.deactivate();
      break;

      case AR_TR_RC:
        ar.activate();		tr.activate();			rc.activate();
        area.deactivate();  tc.deactivate();		span.deactivate();
      break;

      case S_TC_RC:
        span.activate();    tc.activate();			rc.activate();
        tr.deactivate();	ar.deactivate();		area.deactivate();
     break;

      case A_TC_RC:
        area.activate();    tc.activate();			rc.activate();
        tr.deactivate();	ar.deactivate();		span.deactivate();
      break;

      case TR_S_A:
        tr.activate();		span.activate();        area.activate();
        ar.deactivate();	rc.deactivate();		tc.deactivate();
      break;

      default:
        valid_flag = FALSE;
      break;
    }

	if (valid_flag)
	{
		driver = driver_in;
	}
}


//==============================================================//
//==============================================================//
//==============================================================//

//==== Constructor =====//
Ms_wing_geom::Ms_wing_geom(Aircraft* aptr) : Geom(aptr)
{
	sects.set_chunk_size( 256 );
	currSect = 0;
	nextSect = 0;
	highlightType = MSW_HIGHLIGHT_NONE;
	
	rel_twist_flag = false;
	rel_dihedral_flag = false;
	round_end_cap_flag = false;
 
	numXsecs.deactivate();

	rootActiveFlag = 1;

	type = MS_WING_GEOM_TYPE;
	type_str = Stringc("wing");

	char name[255];
	sprintf( name, "Ms_Wing_%d", geomCnt ); 
	geomCnt++;
	name_str = Stringc(name);
	id_str = name_str;				//jrg fix

	copySect.rootAf = new Af( this );
	copySect.rootAf->init_script("wing_foil");
	copySect.tipAf = new Af( this );
	copySect.tipAf->init_script("wing_foil");

	//==== Define Some Decent Default Parms ====//
	setSymCode(XZ_SYM);
	define_parms();
	generate();

	surfVec.push_back( &mwing_surf );

}

//==== Destructor =====//
Ms_wing_geom::~Ms_wing_geom()
{
	DeleteAllFoilsAndSects();
	////==== Delete Foils ====//
	//for ( int i = 0 ; i < foils.dimension() ; i++ )
	//	delete foils[i];

	delete copySect.rootAf;
	delete copySect.tipAf;

}

void Ms_wing_geom::copy( Geom* fromGeom )
{
	int i;
	Geom::copy( fromGeom );						// Copy Base Stuff

	if ( fromGeom->getType() != MS_WING_GEOM_TYPE && fromGeom->getType() != HWB_GEOM_TYPE)
		return;

	Ms_wing_geom* g = (Ms_wing_geom*)fromGeom;

	total_area.set( g->total_area() );
	total_span.set( g->total_span() );
	total_proj_span.set( g->total_proj_span() );
	avg_chord.set( g->avg_chord() );
	sect_proj_span.set( g->sect_proj_span() );

	sweep_off.set( g->sweep_off() );

	total_aspect.set( g->total_aspect() );

	deg_per_seg.set( g->deg_per_seg() );
	max_num_segs.set( g->max_num_segs() );

	trimmed_span = g->get_trimmed_span();

	rel_twist_flag    = g->get_rel_twist_flag();
	rel_dihedral_flag = g->get_rel_dihedral_flag();
	round_end_cap_flag = g->get_round_end_cap_flag();

	//==== Delete Foils ====//
	DeleteAllFoilsAndSects();

	//==== Copy Sections and Airfoils  ====//
	sects.init( g->sects.dimension() );

	for ( i = 0 ; i < sects.dimension() ; i++ )
	{
		sects[i] = g->sects[i];
	}

	//==== Load Airfoils ====//
	for (  i = 0 ; i < sects.dimension() ; i++ )
	{
		if ( i == 0 )
		{
			sects[i].rootAf = new Af( this );
			sects[i].rootAf->init_script("wing_foil");
			foils.append( sects[i].rootAf );
		}
		else
		{
			sects[i].rootAf = sects[i-1].tipAf;
		}

		sects[i].tipAf = new Af( this );
		sects[i].tipAf->init_script("wing_foil");
		foils.append( sects[i].tipAf );

	}

	for ( i = 0 ; i < sects.dimension() ; i++ )
	{
		*(sects[i].tipAf) = *(g->sects[i].tipAf);
		*(sects[i].rootAf) = *(g->sects[i].rootAf);

		sects[i].tipAf->set_geom( this );
		sects[i].rootAf->set_geom( this );
	}


	set_curr_sect(0);
	set_num_pnts_all_afs();
	generate();

	for ( i = 0 ; i < sects.dimension() ; i++ )
	{
		sects[i].SetGeomPtr( this );
	}

	parmLinkMgrPtr->RebuildAll();
}

WingSect* Ms_wing_geom::getCurrWingSect()
{
	return &sects[currSect];
}


void Ms_wing_geom::DeleteAllFoilsAndSects()
{
	int i;

	//==== Delete Foils ====//
	for ( i = 0 ; i < foils.dimension() ; i++ )
	{
		RemoveAirfoilParmReferences( foils[i] );
		delete foils[i];
	}
	foils.init(0);

	//==== Remove Parm Refs for Sects ====//
	for ( i = 0 ; i < sects.dimension() ; i++ )
	{
		RemoveWingSectParmReferences( i );
	}
}


//==== Define  Parms ====//
void Ms_wing_geom::define_parms()
{
  int i;

  //==== Load Up Some Default Sections ====//
  WingSect ws;
  ws.Build( 4.0, 12.0, 21.0, 65.0, 0.0, 0.0, 0.0 );
  sects.append( ws );

  ws.Build( 7.0, 3.0, 12.0, 40.0, 15.0, 0.1, 0.5 );
  sects.append( ws );

  ws.Build( 2.0, 2.0, 3.0, 33.0, 45.0, 0.0, 0.0 );
  sects.append( ws );

  set_curr_sect( currSect );
 
	//==== Load Airfoils ====//
	for (  i = 0 ; i < sects.dimension() ; i++ )
	{
		if ( i == 0 )
		{
			sects[i].rootAf = new Af( this );
			sects[i].rootAf->init_script("wing_foil");
			foils.append( sects[i].rootAf );
		}
		else
		{
			sects[i].rootAf = sects[i-1].tipAf;
		}
		sects[i].tipAf = new Af( this );
		sects[i].tipAf->init_script("wing_foil");
		foils.append( sects[i].tipAf );

	}

  //==== More Defaults ====//
  sects[0].rootAf->get_thickness()->set( 0.04 );
  sects[0].rootAf->generate_airfoil();
  sects[0].rootAf->load_name();
  sects[0].tipAf->get_thickness()->set( 0.06 );
  sects[0].tipAf->generate_airfoil();
  sects[0].tipAf->load_name();

  set_curr_sect(0);

  //==== Make Sure Root and Tip of Adjoining Sections Match ====//
  matchWingSects();	

  int num_sects = sects.dimension();

  set_num_pnts_all_afs();

  mwing_surf.set_num_pnts( numPnts.iget() );
  mwing_surf.set_num_xsecs( num_sects + 3 );

  compose_model_matrix();

  //==== Compute Totals ====//
  double ta = 0.0;
  double ts = 0.0;
  double tc = 0.0;
  double tps = 0.0;

  for ( i = 0 ; i < sects.dimension() ; i++ )
  {
    ta += sects[i].area_val();
    ts += sects[i].span_val();
    tc += sects[i].tc_val() + sects[i].rc_val();
//	tps += sects[i].span * cos( sects[i].dihedral * DEG_2_RAD );
	tps += sects[i].span_val() * cos( get_sum_dihedral(i) * DEG_2_RAD );
  }

	if ( sym_code == XZ_SYM )
	{
		ta *= 2.0;
		ts *= 2.0;
		tps *= 2.0;
	}

  double ac = tc/(sects.dimension() * 2 );
  double ar = (ts*ts)/ta;

//  double sps =  sects[currSect].span * fabs(cos( sects[currSect].dihedral *  DEG_2_RAD ));
  double sps =  sects[currSect].span_val() * fabs(cos( get_sum_dihedral(currSect) *  DEG_2_RAD ));
  sect_proj_span.initialize(this, UPD_MSW_SECTS, "Sect_Aspect", sps );
  sect_proj_span.set_lower_upper(0.0001, 1000000.0);

  total_area.initialize(this, UPD_MSW_TOTAL_AREA, "Total_Area", ta);
  total_area.set_lower_upper(0.0001, 1000000.0);
  total_area.set_script("wing_plan area", 0);

  total_span.initialize(this, UPD_MSW_TOTAL_SPAN, "Total_Span", ts);
  total_span.set_lower_upper(0.0001, 1000000.0);
  total_span.set_script("wing_plan span", 0);

  total_proj_span.initialize(this, UPD_MSW_TOTAL_SPAN, "Total_Proj_Span", tps);
  total_proj_span.set_lower_upper(0.0001, 1000000.0);
  total_proj_span.set_script("wing_plan projspan", 0);

  avg_chord.initialize(this, UPD_MSW_AVG_CHORD, "Avg_Chord", ac);
  avg_chord.set_lower_upper(0.0001, 1000000.0);
  avg_chord.set_script("wing_plan chord", 0);

  sweep_off.initialize(this, UPD_MW_ALL, "Sweep_Offset", 0.0);
  sweep_off.set_lower_upper(-85.0, 85.0);
  sweep_off.set_script("wing_plan sweepoff", 0);

  total_aspect.initialize(this, UPD_MSW_ALL, "Total_Aspect", ar);
  total_aspect.deactivate();
  total_aspect.set_script("wing_plan ar", 0);

  deg_per_seg.initialize(this, UPD_MSW_SECTS, "Degrees Per Segment in Blend", 9.0);
  deg_per_seg.set_lower_upper( 1.0, 30.0 );
  deg_per_seg.set_script("wing_dihed degperseg", 1);

  max_num_segs.initialize(this, UPD_MSW_SECTS, "Max Num Segments in Blend", 9 );
  max_num_segs.set_lower_upper( 2.0, 30.0 );
  max_num_segs.set_script("wing_dihed maxsegs", 1);

  for ( i = 0 ; i < sects.dimension() ; i++ )
  {
	sects[i].SetGeomPtr( this );
  }
 
}

void Ms_wing_geom::LoadLinkableParms( vector< Parm* > & parmVec )
{
	int i;
	char gname[256];

	Geom::LoadLinkableParms( parmVec );
	Geom::AddLinkableParm( &total_area, parmVec, this, "Design" );
	Geom::AddLinkableParm( &total_span, parmVec, this, "Design" );
	Geom::AddLinkableParm( &total_proj_span, parmVec, this, "Design" );
	Geom::AddLinkableParm( &avg_chord, parmVec, this, "Design" );

	Geom::AddLinkableParm( &sweep_off, parmVec, this, "Design" );
	Geom::AddLinkableParm( &total_aspect, parmVec, this, "Design" );

	//==== Sections ====//
	for ( i = 0 ; i < sects.dimension() ; i++ )
	{
		sprintf( gname, "Sect_%d", i );
		vector< Parm* > pVec = sects[i].GetLinkableParms();
		for ( int j = 0 ; j < (int)pVec.size() ; j++ )
		{
			Geom::AddLinkableParm( pVec[j], parmVec, this, gname );
		}
	}

	//==== Airfoils ====//
	vector< Af* > afVec;
	for (  i = 0 ; i < sects.dimension() ; i++ )
	{
		if ( i == 0 )	
			afVec.push_back( sects[i].rootAf );
		afVec.push_back( sects[i].tipAf );
	}

	for ( i = 0 ; i < (int)afVec.size() ; i++ )
	{
		sprintf( gname, "Airfoil_%d", i );
		vector< Parm* > pVec = afVec[i]->GetLinkableParms();
		for ( int j = 0 ; j < (int)pVec.size() ; j++ )
		{
			Geom::AddLinkableParm( pVec[j], parmVec, this, gname );
		}
	}
}

void Ms_wing_geom::RemoveWingSectParmReferences( int sect_id )
{
	if ( sect_id < 0 || sect_id >= sects.dimension() )
		return;

	vector< Parm* > pVec = sects[sect_id].GetLinkableParms();
	for ( int i = 0 ; i < (int)pVec.size() ; i++ )
	{
		parmLinkMgrPtr->RemoveParmReferences( pVec[i] );
	}
}

void Ms_wing_geom::RemoveAirfoilParmReferences( Af* afPtr )
{
	vector< Parm* > pVec = afPtr->GetLinkableParms();
	for ( int i = 0 ; i < (int)pVec.size() ; i++ )
	{
		parmLinkMgrPtr->RemoveParmReferences( pVec[i] );
	}
}


//==== Generate Multi Section Wing Component ====//
void Ms_wing_geom::generate()
{
  generate_surf();

  update_bbox();
}


vec3d Ms_wing_geom::getAttachUVPos(double u, double v)
{
	vec3d pos;

	vec3d uvpnt = mwing_surf.comp_uv_pnt(u,v);
	pos = uvpnt.transform( model_mat );

	return pos;
}

void Ms_wing_geom::acceptScaleFactor()
{
	lastScaleFactor = 1.0;
	scaleFactor.set(1.0);
}

void Ms_wing_geom::resetScaleFactor()
{
	scaleFactor.set( 1.0 );
    scale();
	lastScaleFactor = 1.0;
		
	//==== Tell Aircraft You Have Changed ====//
	airPtr->geomMod( this );
}

void Ms_wing_geom::computeCenter()
{
	if ( sects.dimension() >= 1 )
	{
		//==== Set Rotation Center ====//
		center.set_x( origin()*sects[0].rc_val()*scaleFactor() ); 
	}
}

//==== Parm Has Changed ReGenerate Multi Section Wing Component ====//
void Ms_wing_geom::parm_changed(Parm* p)
{
	int i;
	int up_group = p->get_update_grp();

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
		  for ( int i = 0 ; i < foils.dimension() ; i++ )
		  {
			foils[i]->generate_airfoil();
			foils[i]->load_name();
		  }
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

		case UPD_MSW_TOTAL_SPAN:
        {
  			//==== Compute Totals ====//
			double ts = 0.0;
  			for ( i = 0 ; i < sects.dimension() ; i++ )
				ts += sects[i].span_val();

  			//==== Compute Totals Proj Span ====//
			double tps = 0.0;
  			for ( i = 0 ; i < sects.dimension() ; i++ )
				tps += sects[i].span_val()* cos( get_sum_dihedral(i) * DEG_2_RAD );

			if ( sym_code == XZ_SYM )
			{
				ts *= 2.0;
				tps *= 2.0;
			}

			double fract = -1.0;
			if ( fabs( ts - total_span()) > 0.001 && ts > 0.1 )
				fract = total_span()/ts;

			else if ( fabs( tps - total_proj_span()) > 0.001 && tps > 0.1 )
				fract =  total_proj_span()/tps;

  			//==== Adjust Sections Total Span ====//
			if ( fract > 0.0 )
			{
    			for (  i = 0 ; i < sects.dimension() ; i++ )
    			{
      				int odriver = sects[i].driver;
      				int ndriver = MS_S_TC_RC;
      				sects[i].driver = ndriver;
      				sects[i].span_set( sects[i].span_val() * fract );
      				sects[i].fillDependData();
      				sects[i].driver = odriver;
    			}
  			}

  			//==== Compute Totals ====//
			computeTotals();
          generate();
		  updateAttach(0);

        }
      break;

      case UPD_MSW_TOTAL_AREA:
        {
  			//==== Compute Totals ====//
			double ta = 0.0;

  			for ( i = 0 ; i < sects.dimension() ; i++ )
				ta += sects[i].area_val();

			if ( sym_code == XZ_SYM )
			{
				ta *= 2.0;
			}


  			//==== Adjust Sections Total Area ====//
 			if ( fabs( ta - total_area()) > 0.001 && ta > 0.1 )
  			{
    			double fract = total_area()/ta;

    			for (  i = 0 ; i < sects.dimension() ; i++ )
    			{
      				int odriver = sects[i].driver;
					int ndriver = MS_AR_TR_A;
      				sects[i].driver = ndriver;
      				sects[i].area_set( sects[i].area_val() * fract );
      				sects[i].fillDependData();
      				sects[i].driver = odriver;
    			}
  			}
  			//==== Compute Totals ====//
			computeTotals();
          generate();
		  updateAttach(0);
        }
      break;

      case UPD_MSW_AVG_CHORD:
        {
  			//==== Compute Totals ====//
			double tc = 0.0;

  			for ( i = 0 ; i < sects.dimension() ; i++ )
				tc += sects[i].tc_val() + sects[i].rc_val();

			double ac = tc/(double)(sects.dimension() * 2 );

  			//==== Adjust Sections Total Area ====//
 		    if ( fabs( ac - avg_chord()) > 0.001  )
  			{
    			double fract = avg_chord()/ac;

				if ( fract < 0.001f )
				  fract = 0.001f;

    			for (  i = 0 ; i < sects.dimension() ; i++ )
    			{
      				int odriver = sects[i].driver;
					int ndriver = MS_S_TC_RC;
      				sects[i].driver = ndriver;
      				sects[i].rc_set(  sects[i].rc_val() * fract );
      				sects[i].tc_set(  sects[i].tc_val() * fract );
      				sects[i].fillDependData();
      				sects[i].driver = odriver;
    			}
  			}
  			//==== Compute Totals ====//
			computeTotals();
          generate();
		  updateAttach(0);
        }
      break;

      case UPD_MSW_SECTS:
        { 
			set_depend_parms();
			generate();
		    updateAttach(0);
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


void Ms_wing_geom::computeTotals()
{
  //==== Compute Total Span ====//
	double ts = 0.0;
	double ta = 0.0;
	double tc = 0.0;
	double tps = 0.0;
	for (int i = 0 ; i < sects.dimension() ; i++ )
	{
		ts += sects[i].span_val();
		ta += sects[i].area_val();
		tc += sects[i].tc_val() + sects[i].rc_val();
		tps += sects[i].span_val() * cos( get_sum_dihedral(i) * DEG_2_RAD );
	}

	if ( sym_code == XZ_SYM )
	{
		ts *= 2.0;
		tps *= 2.0;
		ta *= 2.0;
	}

	if ( fabs(total_span() - ts) > 0.001 )
		total_span = ts;

	if ( fabs(total_proj_span() - tps) > 0.001 )
		total_proj_span = tps;

  if ( fabs( total_area() - ta ) > 0.001 )
		total_area = ta;

	double ac = tc/(double)(sects.dimension() * 2 );

	if ( fabs( avg_chord() - ac ) > 0.001 )
		avg_chord = ac;

	total_aspect =  (total_span()*total_span())/total_area();

	sect_proj_span = sects[currSect].span_val() * fabs(cos( get_sum_dihedral(currSect) *  DEG_2_RAD ));
}


//==== Match Root/Tip of Adjoining Sections ====//
void Ms_wing_geom::matchWingSects()
{
  int i;

  //==== Start From Current Section and Work In ====//
  for ( i = currSect ; i >= 1 ; i-- )
  {
    int ndriver = MS_S_TC_RC;
    int odriver = sects[i-1].driver;
    if ( odriver == MS_AR_TR_A || 
         odriver == MS_AR_TR_S || 
         odriver == MS_AR_TR_RC ) 
    {
      ndriver = MS_AR_TR_TC; 
    }

    sects[i-1].driver = ndriver;
    sects[i-1].tc_set( sects[i].rc_val() );
    sects[i-1].fillDependData();
    sects[i-1].driver = odriver;
  }
  //==== Start From Current Section and Work Out ====//
  for ( i = currSect ; i < sects.dimension() - 1 ; i++ )
  {
    int ndriver = MS_S_TC_RC;
    int odriver = sects[i+1].driver;
    if ( odriver == MS_AR_TR_A || 
         odriver == MS_AR_TR_S || 
         odriver == MS_AR_TR_TC ) 
    {
      ndriver = MS_AR_TR_RC; 
    }
    sects[i+1].driver = ndriver;
    sects[i+1].rc_set( sects[i].tc_val() );
    sects[i+1].fillDependData();
    sects[i+1].driver = odriver;
  }
}

double Ms_wing_geom::get_cbar()
{
	double area_taper = 0.0;
	for ( int i = 0 ; i < sects.dimension() ; i++ )
		area_taper += sects[i].area_val()*sects[i].tr_val();

	double avg_taper = area_taper/total_area();
	double root_chord = sects[0].rc_val();
	double m_a_c = 2.0/3.0*root_chord*(1.0+avg_taper+avg_taper*avg_taper)/(1.0+avg_taper);

	return( m_a_c );
}

//==== Get Current Airfoil Ptr ====//
void Ms_wing_geom::set_num_pnts_all_afs()
{
  int npnts = numPnts.iget();

  if ( npnts%2 == 0 )
    numPnts.set( npnts + 1 );

  for ( int i = 0 ; i < foils.dimension() ; i++ )
  {
    foils[i]->set_num_pnts(numPnts.iget());
  }

  mwing_surf.set_num_pnts( numPnts.iget() );
}

//==== Get Current Airfoil Ptr ====//
Af* Ms_wing_geom::get_af_ptr()
{
	if ( rootActiveFlag )
  		return sects[currSect].rootAf;
	
	return sects[currSect].tipAf;
}

//==== Add Section ====//
void Ms_wing_geom::add_sect()
{
  WingSect ws1 = sects[currSect];
  WingSect ws2 = sects[currSect];

  //==== Fill Wing Section 1 ====//
  double tc1 = ws1.tc_val();
  double rc1 = ws1.rc_val();
  int odriver = ws1.driver;
  ws1.driver = MS_S_TC_RC;
  ws1.span_set( ws1.span_val() * 0.5 );
  ws1.tc_set( 0.5*(tc1 + rc1) );
  ws1.rc_set( rc1 );
  ws1.fillDependData();
  ws1.driver = odriver;

  //==== Fill Wing Section 2 ====//
  double tc2 = ws2.tc_val();
  double rc2 = ws2.rc_val();
  ws2.driver = MS_S_TC_RC;
  ws2.span_set( ws2.span_val() * 0.5 );
  ws2.rc_set( 0.5*(tc2 + rc2));
  ws2.tc_set( tc2 );
  ws2.fillDependData();
  ws2.driver = odriver;

	Af* midaf = new Af( this );
	midaf->init_script("wing_foil");
	foils.append( midaf );

	*midaf = *ws1.rootAf;
	ws1.tipAf = midaf;
	ws2.rootAf = midaf;

  sects[currSect] = ws1;
  sects.insert_after_index( ws2, currSect );

  set_curr_sect( currSect );

  set_depend_parms();
  generate();

  airPtr->geomMod( this );
  parmLinkMgrPtr->RebuildAll();

}

//==== Add Section ====//
void Ms_wing_geom::ins_sect()
{
  WingSect ws = sects[currSect];

  sects.insert_after_index( ws, currSect );

  Af * foil = new Af(this);		
  foil->init_script("wing_foil");
	*foil = *ws.tipAf;
  foils.append( foil );

	sects[currSect].tipAf = foil;
	sects[currSect+1].rootAf =  foil;
	
  set_curr_sect( currSect );
  set_depend_parms();
  generate();

  airPtr->geomMod( this );
  parmLinkMgrPtr->RebuildAll();

}

//==== Delete Section ====//
void Ms_wing_geom::del_sect()
{
  if ( sects.dimension() < 2 )
    return;

  RemoveWingSectParmReferences( currSect );
  sects.del_index( currSect );

	//==== Check If All Airfoils Are Still Referenced ====//
	dyn_array< Af* > tmpfoils;
	for ( int i = 0 ; i < foils.dimension() ; i++ )
	{
		int ref = 0;
		for ( int j = 0 ; j < sects.dimension() ; j++ )
		{
			if ( sects[j].rootAf == foils[i] || sects[j].tipAf == foils[i] )
				ref = 1;
		}
		if ( ref )
		{
			tmpfoils.append( foils[i] );
		}
		else
		{
			RemoveAirfoilParmReferences( foils[i] );
			delete foils[i];
		}
	}
	foils = tmpfoils;

  if ( currSect >= sects.dimension() )
    currSect = sects.dimension()-1;

  set_curr_sect( currSect );
  set_depend_parms();
  generate();

  airPtr->geomMod( this );

  parmLinkMgrPtr->RebuildAll();


}

//==== Copy Section ====//
void Ms_wing_geom::copy_sect()
{
	Af* raf = copySect.rootAf;
	Af* taf = copySect.tipAf;
	copySect = sects[currSect];
	*raf = *sects[currSect].rootAf;
	*taf = *sects[currSect].tipAf;

	copySect.rootAf = raf;
	copySect.tipAf  = taf;

}

//==== Paste Section ====//
void Ms_wing_geom::paste_sect()
{
	Af* raf = sects[currSect].rootAf;
	Af* taf = sects[currSect].tipAf;

	sects[currSect] = copySect;

	sects[currSect].rootAf = raf;
	sects[currSect].tipAf  = taf;

	*sects[currSect].rootAf = *(copySect.rootAf);
	*sects[currSect].tipAf  = *(copySect.tipAf);

	set_curr_sect( currSect );
	set_depend_parms();
	generate();
  
	airPtr->geomMod( this );
    parmLinkMgrPtr->RebuildAll();

}

void Ms_wing_geom::scale()
{
	//double current_factor = scaleFactor()*(1.0/lastScaleFactor);

 //   for (  int i = 0 ; i < sects.dimension() ; i++ )
 //   {
 //   	int odriver = sects[i].driver;
	//	int ndriver = MS_S_TC_RC;
	//	sects[i].driver = ndriver;
	//	if ( i == 0 )
	//		sects[i].rc_set(sects[i].rc_val() * current_factor);

	//	sects[i].tc_set( sects[i].tc_val() * current_factor );		
	//	sects[i].span_set( sects[i].span_val() * current_factor );
	//	sects[i].fillDependData();
	//	sects[i].driver = odriver;
 //   }

//jrg scale area not rc/tc/span
	double current_factor = scaleFactor()*(1.0/lastScaleFactor);
	for (  int i = 0 ; i < sects.dimension() ; i++ )
	{
		int odriver = sects[i].driver;
		int ndriver = MS_AR_TR_A;
		sects[i].driver = ndriver;
		sects[i].area_set( sects[i].area_val() * current_factor );
		sects[i].fillDependData();
		sects[i].driver = odriver;
	}


  	//==== Compute Totals ====//
	computeTotals();
    generate();

	lastScaleFactor = scaleFactor();

}



//==== Write Multi Section Wing File ====//
void Ms_wing_geom::write(FILE* file_id)
{

}

//==== Write Multi Section Wing File ====//
void Ms_wing_geom::write(xmlNodePtr root)
{
  int i;
  xmlAddStringNode( root, "Type", "Mswing");

  //==== Write General Parms ====//
  xmlNodePtr gen_node = xmlNewChild( root, NULL, (const xmlChar *)"General_Parms", NULL );
  write_general_parms( gen_node );

  //==== Write Fuse Parms ====//
  xmlNodePtr mswing_node = xmlNewChild( root, NULL, (const xmlChar *)"Mswing_Parms", NULL );

  xmlAddDoubleNode( mswing_node, "Total_Area", total_area() );
  xmlAddDoubleNode( mswing_node, "Total_Span", total_span() );
  xmlAddDoubleNode( mswing_node, "Total_Proj_Span", total_proj_span() );
  xmlAddDoubleNode( mswing_node, "Avg_Chord", avg_chord() );
  xmlAddDoubleNode( mswing_node, "Sweep_Off", sweep_off() );
  xmlAddIntNode( mswing_node, "Deg_Per_Seg", (int)(deg_per_seg()+0.5) );
  xmlAddIntNode( mswing_node, "Max_Num_Seg", (int)(max_num_segs()+0.5) );
  xmlAddIntNode( mswing_node, "Rel_Dihedral_Flag", get_rel_dihedral_flag() );
  xmlAddIntNode( mswing_node, "Rel_Twist_Flag", get_rel_twist_flag() );
  xmlAddIntNode( mswing_node, "Round_End_Cap_Flag", get_round_end_cap_flag() );

  int nsect = sects.dimension();

  //===== Write Airfoils =====//
  xmlNodePtr af_node;
  xmlNodePtr af_list_node = xmlNewChild( root, NULL, (const xmlChar *)"Airfoil_List", NULL );

  for ( i = 0 ; i < nsect ; i++ )
  {
    if ( i == 0 )
    {
      af_node = xmlNewChild( af_list_node, NULL, (const xmlChar *)"Airfoil", NULL );
      sects[i].rootAf->write( af_node );
    }

    af_node = xmlNewChild( af_list_node, NULL, (const xmlChar *)"Airfoil", NULL );
    sects[i].tipAf->write( af_node );
  }

  //===== Write Sections =====//
  xmlNodePtr sec_node;
  xmlNodePtr sec_list_node = xmlNewChild( root, NULL, (const xmlChar *)"Section_List", NULL );

  for ( i = 0 ; i < nsect ; i++ )
  {
    sec_node = xmlNewChild( sec_list_node, NULL, (const xmlChar *)"Section", NULL );

    xmlAddIntNode( sec_node, "Driver",      sects[i].driver );
    xmlAddDoubleNode( sec_node, "AR",       sects[i].ar_val() );
    xmlAddDoubleNode( sec_node, "TR",       sects[i].tr_val() );
    xmlAddDoubleNode( sec_node, "Area",     sects[i].area_val() );
    xmlAddDoubleNode( sec_node, "Span",     sects[i].span_val() );
    xmlAddDoubleNode( sec_node, "TC",       sects[i].tc_val() );
    xmlAddDoubleNode( sec_node, "RC",       sects[i].rc_val() );
    xmlAddDoubleNode( sec_node, "Sweep",    sects[i].sweep_val() );
    xmlAddDoubleNode( sec_node, "SweepLoc", sects[i].sweepLoc_val() );
    xmlAddDoubleNode( sec_node, "Twist",    sects[i].twist_val() );
    xmlAddDoubleNode( sec_node, "TwistLoc", sects[i].twistLoc_val() );
    xmlAddDoubleNode( sec_node, "Dihedral", sects[i].dihedral_val() );
    xmlAddDoubleNode( sec_node, "Dihed_Crv1", sects[i].dihed_crv1_val() );
    xmlAddDoubleNode( sec_node, "Dihed_Crv2", sects[i].dihed_crv2_val() );
    xmlAddDoubleNode( sec_node, "Dihed_Crv1_Str", sects[i].dihed_crv1_str_val() );
    xmlAddDoubleNode( sec_node, "Dihed_Crv2_Str", sects[i].dihed_crv2_str_val() );

    xmlAddIntNode( sec_node, "DihedRotFlag", sects[i].dihedRotFlag );
    xmlAddIntNode( sec_node, "SmoothBlendFlag", sects[i].smoothBlendFlag );
    xmlAddIntNode( sec_node, "NumInterpXsecs", sects[i].num_interp_xsecs );
  }

  feaMeshMgrPtr->WriteFeaStructData( this, root );

}
//==== Write Multi Section Wing File ====//
void Ms_wing_geom::read(xmlNodePtr root)
{
  int i;
  xmlNodePtr node;

  //===== Read General Parameters =====//
  node = xmlGetNode( root, "General_Parms", 0 );
  if ( node )
    read_general_parms( node );

  //===== Read Fuse Parameters =====//
  node = xmlGetNode( root, "Mswing_Parms", 0 );
  if ( node )
  {
    total_area = xmlFindDouble( node, "Total_Area", total_area() );
    total_span = xmlFindDouble( node, "Total_Span", total_span() );
    total_proj_span = xmlFindDouble( node, "Total_Proj_Span", total_proj_span() );
    avg_chord  = xmlFindDouble( node, "Avg_Chord", avg_chord() );
    sweep_off  = xmlFindDouble( node, "Sweep_Off", sweep_off() );
	deg_per_seg = xmlFindInt( node, "Deg_Per_Seg", (int)(deg_per_seg()+0.5) );
	max_num_segs = xmlFindInt( node, "Max_Num_Seg", (int)(max_num_segs()+0.5) );
	rel_dihedral_flag = xmlFindInt( node, "Rel_Dihedral_Flag", rel_dihedral_flag )!= 0;
	rel_twist_flag = xmlFindInt( node, "Rel_Twist_Flag", rel_twist_flag )!= 0;
	round_end_cap_flag = xmlFindInt( node, "Round_End_Cap_Flag", round_end_cap_flag )!= 0;
  }

  //==== Read Airfoils ====//
  xmlNodePtr af_list_node = xmlGetNode( root, "Airfoil_List", 0 );
  if ( af_list_node )
  {
    dyn_array< Af* > tmpfoils;

    //==== Delete Foils ====//
	DeleteAllFoilsAndSects();
 //   for ( i = 0 ; i < foils.dimension() ; i++ )
 //   {			
 //     delete foils[i];			
	//}

    int num_af = xmlGetNumNames( af_list_node, "Airfoil" );

    for ( i = 0 ; i < num_af ; i++ )
    {
      xmlNodePtr af_node = xmlGetNode( af_list_node, "Airfoil", i );

      Af* afptr = new Af( this );
	  afptr->init_script("wing_foil");
	  afptr->read( af_node );
      tmpfoils.append( afptr );
    }
    foils = tmpfoils;
  }

  xmlNodePtr sec_list_node = xmlGetNode( root, "Section_List", 0 );
  if ( sec_list_node )
  {
    int num_sec =  xmlGetNumNames( sec_list_node, "Section" );
    assert( num_sec+1 == foils.dimension() );

    dyn_array<WingSect> tmpsects;
	tmpsects.set_chunk_size( 256 );

    for ( i = 0 ; i < num_sec ; i++ )
    {
      WingSect ws;
      xmlNodePtr sec_node = xmlGetNode( sec_list_node, "Section", i );

      ws.driver   = xmlFindInt( sec_node, "Driver", ws.driver );
	  ws.SetDriver( ws.driver );

      ws.ar_set( xmlFindDouble( sec_node, "AR", ws.ar_val() ) );
      ws.tr_set( xmlFindDouble( sec_node, "TR", ws.tr_val() ) );
      ws.area_set( xmlFindDouble( sec_node, "Area", ws.area_val() ) );
      ws.span_set( xmlFindDouble( sec_node, "Span", ws.span_val() ) );
      ws.tc_set( xmlFindDouble( sec_node, "TC", ws.tc_val() ) );
      ws.rc_set( xmlFindDouble( sec_node, "RC", ws.rc_val() ) );
      ws.sweep_set( xmlFindDouble( sec_node, "Sweep", ws.sweep_val() ) );
      ws.sweepLoc_set( xmlFindDouble( sec_node, "SweepLoc", ws.sweepLoc_val() ) );
	  ws.refSweepLoc = ws.sweepLoc_val();
      ws.twist_set( xmlFindDouble( sec_node, "Twist", ws.twist_val() ) );
      ws.twistLoc_set( xmlFindDouble( sec_node, "TwistLoc", ws.twistLoc_val() ) );
      ws.dihedral_set( xmlFindDouble( sec_node, "Dihedral", ws.dihedral_val() ) );
      ws.dihed_crv1_set( xmlFindDouble( sec_node, "Dihed_Crv1", ws.dihed_crv1_val() ) );
      ws.dihed_crv2_set( xmlFindDouble( sec_node, "Dihed_Crv2", ws.dihed_crv2_val() ) );
      ws.dihed_crv1_str_set( xmlFindDouble( sec_node, "Dihed_Crv1_Str", ws.dihed_crv1_str_val() ) );
      ws.dihed_crv2_str_set( xmlFindDouble( sec_node, "Dihed_Crv2_Str", ws.dihed_crv2_str_val() ) );

	  ws.dihedRotFlag     = xmlFindInt( sec_node, "DihedRotFlag", ws.dihedRotFlag );
	  ws.smoothBlendFlag  = xmlFindInt( sec_node, "SmoothBlendFlag", ws.smoothBlendFlag );
	  ws.num_interp_xsecs = xmlFindInt( sec_node, "NumInterpXsecs", ws.num_interp_xsecs );

      ws.rootAf = foils[i];
      ws.tipAf  = foils[i+1];

      tmpsects.append( ws );
    }
    sects = tmpsects;
  }

  set_num_pnts_all_afs();
  set_curr_sect( 0 );
  set_depend_parms();
  generate();
  parmLinkMgrPtr->RebuildAll();

  feaMeshMgrPtr->SetFeaStructData( this, root );

}

//==== Read Multi Section Wing File ====//
void Ms_wing_geom::read(FILE* file_id)
{

  int i;
  char buff[255];

  //==== Read General Parms ====//
  read_general_parms(file_id);

  //==== Read Total Wing Parms ====//
  int idum;
  fscanf(file_id, "%d",&idum);                  fgets(buff, 80, file_id);
  numPnts = idum;
  set_num_pnts_all_afs();

  total_area.read( file_id );
  total_span.read( file_id );
  total_proj_span.read( file_id );
  avg_chord.read( file_id );
  sweep_off.read( file_id );

  //==== Number Sects ====//
  int nsects;
  fscanf(file_id, "%d",&nsects);                  fgets(buff, 80, file_id);

  //==== Delete Foils and Sections ====//
  DeleteAllFoilsAndSects();
  //for ( i = 0 ; i < foils.dimension() ; i++ )
  //{			
  //  delete foils[i];			
  //}

	//==== Load Airfoils ====//
  dyn_array< Af* > tmpfoils;
	for ( i = 0 ; i < nsects+1 ; i++ )
  {
    Af* afptr = new Af( this );
    afptr->init_script("wing_foil");
    afptr->read( file_id );
    tmpfoils.append( afptr );
  }
  foils = tmpfoils;

	//==== Load Sections ====//
  dyn_array<WingSect> tmpsects;
  tmpsects.set_chunk_size( 256 );

	for ( i = 0 ; i < nsects ; i++ )
  {
    WingSect ws;
	double val;
    fscanf(file_id, "%d",&(ws.driver) );								fgets(buff, 80, file_id);
	ws.SetDriver( ws.driver );
	
	fscanf(file_id, "%lf",&val );		ws.ar_set(val);			fgets(buff, 80, file_id);
    fscanf(file_id, "%lf",&val );		ws.tr_set(val);			fgets(buff, 80, file_id);
    fscanf(file_id, "%lf",&val );		ws.area_set(val);		fgets(buff, 80, file_id);
    fscanf(file_id, "%lf",&val );		ws.span_set(val);		fgets(buff, 80, file_id);
    fscanf(file_id, "%lf",&val );		ws.tc_set(val);			fgets(buff, 80, file_id);
    fscanf(file_id, "%lf",&val );		ws.rc_set(val);			fgets(buff, 80, file_id);
    fscanf(file_id, "%lf",&val );		ws.sweep_set(val);		fgets(buff, 80, file_id);
    fscanf(file_id, "%lf",&val );		ws.sweepLoc_set(val);	fgets(buff, 80, file_id);
	ws.refSweepLoc = val;
    fscanf(file_id, "%lf",&val );		ws.twist_set(val);		fgets(buff, 80, file_id);
    fscanf(file_id, "%lf",&val );		ws.twistLoc_set(val);	fgets(buff, 80, file_id);
    fscanf(file_id, "%lf",&val );		ws.dihedral_set(val);	fgets(buff, 80, file_id);
    fscanf(file_id, "%lf",&val );		ws.dihed_crv1_set(val);	fgets(buff, 80, file_id);
    fscanf(file_id, "%lf",&val );		ws.dihed_crv2_set(val);	fgets(buff, 80, file_id);

    ws.rootAf = foils[i];
    ws.tipAf  = foils[i+1];



    tmpsects.append( ws );
  }
  sects = tmpsects;

  set_curr_sect( 0 );
	set_depend_parms();

  set_num_pnts_all_afs();
  generate();


}

////==== Write Rhino File ====//
//void Ms_wing_geom::write_rhino_file(ON_BinaryFile* archive, ON_3dmObjectAttributes* attributes)
//{
//	base_surf.load_refl_pnts_xsecs();
//
//	base_surf.clear_pnt_tan_flags();
//	base_surf.clear_xsec_tan_flags();
//
//	//==== Sharpen Trailing Edge of Wing ====//
//	base_surf.set_pnt_tan_flag( 0, Bezier_curve::SHARP );
//    int num_pnts  = base_surf.get_num_pnts();
//	base_surf.set_pnt_tan_flag( num_pnts-1, Bezier_curve::SHARP );
//
//
//	//==== Sharpen Wing Joints ====//
//	int num_xsecs = base_surf.get_num_xsecs();
//
//	base_surf.set_xsec_tan_flag( 0, Bezier_curve::SHARP );
//	base_surf.set_xsec_tan_flag( 1, Bezier_curve::SHARP );
//
//	if ( round_end_cap_flag )
//	{
//		base_surf.set_xsec_tan_flag( num_xsecs-2, Bezier_curve::ONLY_BACK );
//		base_surf.set_xsec_tan_flag( num_xsecs-1, Bezier_curve::MIRROR_END_CAP );
//	}
//	else
//	{
//		base_surf.set_xsec_tan_flag( num_xsecs-2, Bezier_curve::SHARP );
//		base_surf.set_xsec_tan_flag( num_xsecs-1, Bezier_curve::SHARP );
//	}
//
//
//	dyn_array< WingPnt > wingPnts;
//	loadWingPnts( wingPnts );
//
//
//	for ( int i = 1 ; i < wingPnts.dimension()-1 ; i++ )
//	{		
//
//		if ( !wingPnts[i].blendFlag || !wingPnts[i-1].blendFlag || !wingPnts[i+1].blendFlag )
//			base_surf.set_xsec_tan_flag( i+1, Bezier_curve::SHARP );		
//
//	}
//
//	//==== Write File ====//
//	base_surf.write_rhino_file( sym_code, model_mat, reflect_mat, archive, attributes );
//}

//==== Write Rhino File ====//
void Ms_wing_geom::write_rhino_file(ON_BinaryFile* archive, ON_3dmObjectAttributes* attributes)
{
	base_surf.load_refl_pnts_xsecs();

	base_surf.clear_pnt_tan_flags();
	base_surf.clear_xsec_tan_flags();

	//==== Sharpen Trailing Edge of Wing ====//
	base_surf.set_pnt_tan_flag( 0, Bezier_curve::SHARP );
    int num_pnts  = base_surf.get_num_pnts();
	base_surf.set_pnt_tan_flag( num_pnts-1, Bezier_curve::SHARP );


	//==== Sharpen Wing Joints ====//
	int num_xsecs = base_surf.get_num_xsecs();

	base_surf.set_xsec_tan_flag( 0, Bezier_curve::SHARP );
	base_surf.set_xsec_tan_flag( 1, Bezier_curve::SHARP );

	if ( round_end_cap_flag )
	{
		base_surf.set_xsec_tan_flag( num_xsecs-2, Bezier_curve::ONLY_BACK );
		base_surf.set_xsec_tan_flag( num_xsecs-1, Bezier_curve::MIRROR_END_CAP );
	}
	else
	{
		base_surf.set_xsec_tan_flag( num_xsecs-2, Bezier_curve::SHARP );
		base_surf.set_xsec_tan_flag( num_xsecs-1, Bezier_curve::SHARP );
	}


	dyn_array< WingPnt > wingPnts;
	loadWingPnts( wingPnts );


	for ( int i = 1 ; i < wingPnts.dimension()-1 ; i++ )
	{		

		if ( !wingPnts[i].blendFlag || !wingPnts[i-1].blendFlag || !wingPnts[i+1].blendFlag )
			base_surf.set_xsec_tan_flag( i+1, Bezier_curve::SHARP );		

	}

	//==== Set Up Split Values ====//
	bool single_comp = merged_center_foil();
	vector <int> u_split;

	if ( !single_comp )
		u_split.push_back( 0 );
		
	u_split.push_back( 3 );

	u_split.push_back( 3*(base_surf.get_num_xsecs() - 2) );
	u_split.push_back( 3*(base_surf.get_num_xsecs() - 1) );


	vector <int> w_split;
	w_split.push_back( 0 );
	w_split.push_back( 3*(base_surf.get_num_pnts()/2)  );
	w_split.push_back( 3*(base_surf.get_num_pnts() - 1)  );

	//==== Write File ====//
	base_surf.write_split_rhino_file( sym_code, model_mat, reflect_mat, 
		u_split, w_split, single_comp, archive, attributes );
}






bool Ms_wing_geom::merged_center_foil()
{
	if ( sym_code != XZ_SYM )
		return false;

	if ( fabs( yLoc() ) > 0.0000001 )
		return false;

	return true;
}

int Ms_wing_geom::get_num_bezier_comps()
{
	if ( sym_code == NO_SYM || merged_center_foil() )
		return 1;
	else 
		return 2;
}



void Ms_wing_geom::write_bezier_file( int id, FILE* file_id )
{
	int i;

	base_surf.load_refl_pnts_xsecs();
	base_surf.clear_pnt_tan_flags();
	base_surf.clear_xsec_tan_flags();

	//==== Sharpen Trailing Edge of Wing ====//
	base_surf.set_pnt_tan_flag( 0, Bezier_curve::SHARP );
    int num_pnts  = base_surf.get_num_pnts();
	base_surf.set_pnt_tan_flag( num_pnts-1, Bezier_curve::SHARP );

	//==== Sharpen Wing Joints ====//
	int num_xsecs = base_surf.get_num_xsecs();

	base_surf.set_xsec_tan_flag( 0, Bezier_curve::SHARP );
	base_surf.set_xsec_tan_flag( 1, Bezier_curve::SHARP );

	if ( round_end_cap_flag )
	{
		base_surf.set_xsec_tan_flag( num_xsecs-2, Bezier_curve::ONLY_BACK );
		base_surf.set_xsec_tan_flag( num_xsecs-1, Bezier_curve::MIRROR_END_CAP );
	}
	else
	{
		base_surf.set_xsec_tan_flag( num_xsecs-2, Bezier_curve::SHARP );
		base_surf.set_xsec_tan_flag( num_xsecs-1, Bezier_curve::SHARP );
	}

	dyn_array< WingPnt > wingPnts;
	loadWingPnts( wingPnts );

	for (  i = 1 ; i < wingPnts.dimension()-1 ; i++ )
	{		
		if ( !wingPnts[i].blendFlag || !wingPnts[i-1].blendFlag || !wingPnts[i+1].blendFlag )
			base_surf.set_xsec_tan_flag( i+1, Bezier_curve::SHARP );		
	}

	bool single_comp = merged_center_foil();

	//==== Set Up Split Values ====//
	vector <int> u_split;

	if ( !single_comp )
		u_split.push_back( 0 );
	u_split.push_back( 3 );

	for ( i = 2 ; i < base_surf.get_num_xsecs()-2 ; i++ )
	{
		if ( base_surf.get_xsec_tan_flag( i ) )
			u_split.push_back( 3*i );
	}

	u_split.push_back( 3*(base_surf.get_num_xsecs() - 2) );
	u_split.push_back( 3*(base_surf.get_num_xsecs() - 1) );


	vector <int> w_split;
	w_split.push_back( 0 );
	w_split.push_back( 3*(base_surf.get_num_pnts()/2)  );
	w_split.push_back( 3*(base_surf.get_num_pnts() - 1)  );

	base_surf.write_bezier_file( file_id, sym_code, model_mat, reflect_mat, u_split, w_split, single_comp );
}




//==== Convert To Tri Mesh ====//
vector< TMesh* > Ms_wing_geom:: createTMeshVec()
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



//==== Set Dependant Parms =====//
void Ms_wing_geom::dump_xsec_file(int geom_no, FILE* dump_file)
{

  int i;

  int nxsec = mwing_surf.get_num_xsecs();

  fprintf(dump_file, "\n");
  fprintf(dump_file, "%s \n", (char*) getName());
  fprintf(dump_file, " GROUP NUMBER      = %d \n",geom_no);
  fprintf(dump_file, " TYPE              = 0  \n");
  fprintf(dump_file, " CROSS SECTIONS    = %d \n", nxsec-2 );
  fprintf(dump_file, " PTS/CROSS SECTION = %d \n",mwing_surf.get_num_pnts());

  for ( i = 1 ; i < nxsec-1 ; i++ )
    {
      mwing_surf.write_xsec(i, model_mat, dump_file);
    }

  if ( sym_code == NO_SYM ) return;

  fprintf(dump_file, "\n");
  fprintf(dump_file, "%s \n", (char*) getName());
  fprintf(dump_file, " GROUP NUMBER      = %d \n",geom_no);
  fprintf(dump_file, " TYPE              = 0  \n");
  fprintf(dump_file, " CROSS SECTIONS    = %d \n",nxsec-2);
  fprintf(dump_file, " PTS/CROSS SECTION = %d \n",mwing_surf.get_num_pnts());

  for ( i = 1 ; i < nxsec-1 ; i++ )
    {
      mwing_surf.write_refl_xsec(sym_code, i, reflect_mat, dump_file);
    }

}


//==== Set Dependant Parms =====//
void Ms_wing_geom::set_depend_parms()
{

  //==== Check If Sweep Loc Changed ====//
  double new_sweep_loc = get_sect_sweep_loc()->get();
  double old_sweep_loc = get_sect_ref_sweep_loc();
  if ( fabs( new_sweep_loc - old_sweep_loc ) > 0.00001 )
{
	double tan_swp = sects[currSect].tan_sweep_at( old_sweep_loc, new_sweep_loc, getSymCode() ); 
    //double tan_swp = sects[currSect].tan_sweep_at( get_sect_sweep_loc()->get(), getSymCode() );
	double new_sweep = atan( tan_swp )*RAD_2_DEG;
    sects[currSect].sweep_set( new_sweep );

	sects[currSect].refSweepLoc = get_sect_sweep_loc()->get();

  }

  sects[currSect].fillDependData();
  matchWingSects();

  //==== Compute Total Span ====//
  double ts = 0.0;
  double ta = 0.0;
  double tc = 0.0;
  double tps = 0.0;
  for (int i = 0 ; i < sects.dimension() ; i++ )
  {
		ts += sects[i].span_val();
		ta += sects[i].area_val();
		tc += sects[i].tc_val() + sects[i].rc_val();
//		tps += sects[i].span * cos( sects[i].dihedral * DEG_2_RAD );
		tps += sects[i].span_val() * cos( get_sum_dihedral(i) * DEG_2_RAD );
  }

	if ( sym_code == XZ_SYM )
	{
		ts *= 2.0;
		tps *= 2.0;
		ta *= 2.0;
	}

  total_span = ts;
	total_proj_span = tps;
  total_area = ta;
	avg_chord = tc/(double)(sects.dimension() * 2 );
	total_aspect =  (total_span()*total_span())/total_area();

	sect_proj_span = sects[currSect].span_val() * fabs(cos( get_sum_dihedral(currSect) *  DEG_2_RAD ));
//	sect_proj_span = sects[currSect].span * fabs(cos( sects[currSect].dihedral *  DEG_2_RAD ));


}

//==== Generate Cross Sections =====//
void Ms_wing_geom::regenerate()
{
}

//==== Set Dihed Rot Flag ====//
void Ms_wing_geom::set_dihed_rot_flag( int rf )
{
  sects[currSect].dihedRotFlag = rf;
  generate();
}

//==== Set Smooth Flag Flag ====//
void Ms_wing_geom::set_smooth_blend_flag( int sbf )
{
  sects[currSect].smoothBlendFlag = sbf;
  generate();
}

//==== Set Relative Twist Flag ====//
void Ms_wing_geom::set_rel_twist_flag(bool flag )
{
  rel_twist_flag = flag;
  generate();
}
//==== Set Relative Dihedral Flag ====//
void Ms_wing_geom::set_rel_dihedral_flag(bool flag )
{
  rel_dihedral_flag = flag;
  generate();
}
//==== Get Sum Dihedral ====//
double Ms_wing_geom::get_sum_dihedral( int sect_id )
{
	if ( sect_id < 0 || sect_id >= sects.dimension() )
		return 0.0;

	if ( rel_dihedral_flag == false )
		return sects[sect_id].dihedral_val();

	double sum_dihedral = 0.0;
	for ( int i = 0 ; i <= sect_id ; i++ )
		sum_dihedral += sects[i].dihedral_val();

	return sum_dihedral;
}

//==== Set Round End Cap Flag ====//
void Ms_wing_geom::set_round_end_cap_flag(bool flag )
{
  round_end_cap_flag = flag;
  generate();
}

//==== Set Num Interp Curr Sect ====//
void Ms_wing_geom::set_num_interp( int n )
{
	sects[currSect].num_interp_xsecs = n;
    generate();
}

//==== Inc Number Of Interp Cross Sections - All Sects ====//
void Ms_wing_geom::inc_all_interp()
{
	for ( int i = 0 ; i < sects.dimension() ; i++ )
	{
		sects[i].num_interp_xsecs++;
	}
	generate();
}

//==== Dec Number Of Interp Cross Sections - All Sects ====//
void Ms_wing_geom::dec_all_interp()
{
	int i;
	for ( i = 0 ; i < sects.dimension() ; i++ )
	{
		if ( sects[i].num_interp_xsecs <= 0 )
			return;
	}

	for ( i = 0 ; i < sects.dimension() ; i++ )
		sects[i].num_interp_xsecs--;

	generate();
}

//==== Set Joint Index ====//
void Ms_wing_geom::set_curr_joint( int cj )
{
	if ( cj > 0 && cj == sects.dimension()-1 )
		set_curr_sect(cj-1);
	else
		set_curr_sect(cj);
}

//==== Get Joint Index ====//
int Ms_wing_geom::get_curr_joint()
{
	if ( currSect > 0 && currSect == sects.dimension()-1 )
		return currSect-1;

	return currSect;
}

//==== Set Foil Index ====//
void Ms_wing_geom::set_curr_foil( int cf )
{
	if ( cf < sects.dimension() )
	{
		set_curr_sect(cf);
		set_root_active();
	}
	else
	{
		set_curr_sect(sects.dimension()-1);
		set_tip_active();
	}
}

//==== Get Foil Index  ====//
int Ms_wing_geom::get_curr_foil()
{
	if ( !get_root_active() && currSect == sects.dimension()-1 )
		return currSect+1;
		
	return currSect;
}

//==== Sect Section Index ====//
void Ms_wing_geom::set_curr_sect(int cs)
{
  if ( cs >= 0 && cs < sects.dimension() )
    currSect = cs;

  if ( currSect < sects.dimension()-1 )
	  nextSect = currSect+1;
  else
	  nextSect = currSect;

//  sect_proj_span = sects[currSect].span * fabs(cos( sects[currSect].dihedral *  DEG_2_RAD ));
  sect_proj_span = sects[currSect].span_val() * fabs(cos( get_sum_dihedral(currSect) *  DEG_2_RAD ));


  airPtr->geomMod( this );

}

void Ms_wing_geom::set_driver( int d )
{
	sects[currSect].SetDriver( d );
}


//==== Generate Cross Sections =====//
void Ms_wing_geom::generate_flap_slat_surfs()
{
  //jrg check if used/needed
}

//==== Update Bounding Box =====//
void Ms_wing_geom::update_bbox()
{
  vec3d pnt;
  bbox new_box;

  int num_pnts  = mwing_surf.get_num_pnts();
  int num_xsecs = mwing_surf.get_num_xsecs();

  //==== Check Some Inlet And Nozzle Xsecs ====//
  for ( int j = 0 ; j < num_pnts ; j++ )
  {
    for ( int i = 1 ; i < num_xsecs-1 ; i++ )
    {
    	pnt = mwing_surf.get_pnt(i,j);
		new_box.update(pnt);
	}
  }

  bnd_box = new_box;
  update_xformed_bbox();
}



//==== Draw Multi Section Wing Geom ====//
void Ms_wing_geom::draw()
{
	Geom::draw();

	if ( displayFlag == GEOM_WIRE_FLAG )
	{
		//==== Draw Hightlighted Stuff ====//
		if ( redFlag )
		{
			glLineWidth(2);
			glColor3f(0.75, 0, 0);
			if ( highlightType != MSW_HIGHLIGHT_NONE )
				draw_sect_box();
		}

	}

}

//==== Draw If Alpha < 1 and Shaded ====//
void Ms_wing_geom::drawAlpha()
{
	Geom::drawAlpha();
}


//==== Update Bounding Box =====//
void Ms_wing_geom::draw_sect_box()
{
	if ( fastDrawFlag )
		return;

	int i;
	vec3d pnt;
	bbox box;

	int num_pnts  = mwing_surf.get_num_pnts();

	int minSectID = 0;
	int maxSectID = 0;
	for ( i = 0 ; i < currSect ; i++ )
	{
		minSectID += sects[i].num_actual_xsecs;
	}

	maxSectID = minSectID + sects[currSect].num_actual_xsecs;

	if ( minSectID == 0 )
		minSectID = 1;

	if ( highlightType == MSW_HIGHLIGHT_JOINT )
		minSectID = maxSectID;

	if ( highlightType == MSW_HIGHLIGHT_FOIL && get_root_active() )
		maxSectID = minSectID;

	if ( highlightType == MSW_HIGHLIGHT_FOIL && !get_root_active() )
		minSectID = maxSectID;


	//==== Check Some Inlet And Nozzle Xsecs ====//
	for ( int j = 0 ; j < num_pnts ; j++ )
	{
		vec3d p = mwing_surf.get_pnt(minSectID,j);
		box.update(p.transform(model_mat));

		if ( maxSectID != minSectID )
		{
			vec3d p = mwing_surf.get_pnt(maxSectID,j);
			box.update(p.transform(model_mat));
		}
	}

	draw_bbox( box );
}

//==== Update Bounding Box =====//
void Ms_wing_geom::draw_refl_sect_box()
{
	if ( fastDrawFlag )
		return;

	if (sym_code == NO_SYM)
		return;


	vec3d pnt;
	bbox box;

	int num_pnts  = mwing_surf.get_num_pnts();

	int minSectID = 0;
	int maxSectID = 0;
	for ( int i = 0 ; i < currSect ; i++ )
		minSectID += sects[i].num_actual_xsecs;

	maxSectID = minSectID + sects[currSect].num_actual_xsecs;

	if ( minSectID == 0 )
		minSectID = 1;

	if ( highlightType == MSW_HIGHLIGHT_JOINT )
		minSectID = maxSectID;

	if ( highlightType == MSW_HIGHLIGHT_FOIL && get_root_active() )
		maxSectID = minSectID;

	if ( highlightType == MSW_HIGHLIGHT_FOIL && !get_root_active() )
		minSectID = maxSectID;


  //==== Check Some Inlet And Nozzle Xsecs ====//
  for ( int j = 0 ; j < num_pnts ; j++ )
  {
	if ( highlightType == MSW_HIGHLIGHT_SECT )
	{
		vec3d p = mwing_surf.get_pnt(minSectID,j)*sym_vec;
		box.update(p.transform(reflect_mat));
	}

	vec3d p = mwing_surf.get_pnt(maxSectID,j)*sym_vec;
    box.update(p.transform(reflect_mat));
  }

  draw_bbox( box );
}

//==== Draw Box ====//
void Ms_wing_geom::draw_bbox( bbox & box )
{
  double temp[3];
  temp[0] = box.get_min(0);
  temp[1] = box.get_min(1);
  temp[2] = box.get_min(2);

  glBegin( GL_LINE_STRIP );
    glVertex3dv(temp);
    temp[0] = box.get_max(0);
    glVertex3dv(temp);
    temp[1] = box.get_max(1);
    glVertex3dv(temp);
    temp[2] = box.get_max(2);
    glVertex3dv(temp);
    temp[0] = box.get_min(0);
    glVertex3dv(temp);
    temp[2] = box.get_min(2);
    glVertex3dv(temp);
    temp[1] = box.get_min(1);
    glVertex3dv(temp);
    temp[2] = box.get_max(2);
    glVertex3dv(temp);
    temp[0] = box.get_max(0);
    glVertex3dv(temp);
    temp[2] = box.get_min(2);
    glVertex3dv(temp);
  glEnd();

  glBegin( GL_LINE_STRIP );
    temp[2] = box.get_max(2);
    glVertex3dv(temp);
    temp[1] = box.get_max(1);
    glVertex3dv(temp);
  glEnd();

  glBegin( GL_LINE_STRIP );
    temp[2] = box.get_min(2);
    glVertex3dv(temp);
    temp[0] = box.get_min(0);
    glVertex3dv(temp);
  glEnd();

  glBegin( GL_LINE_STRIP );
    temp[2] = box.get_max(2);
    glVertex3dv(temp);
    temp[1] = box.get_min(1);
    glVertex3dv(temp);
  glEnd();


}

//==== Compute And Load Normals ====//
void Ms_wing_geom::load_normals()
{
  mwing_surf.load_sharp_normals();
}

//==== Draw Hidden Surface====//
void Ms_wing_geom::load_hidden_surf()
{
  mwing_surf.load_hidden_surf();
}


void Ms_wing_geom::loadWingPnts( dyn_array< WingPnt > & wingPnts )
{
	int i, j;
	WingPnt wp;
	dyn_array< WingPnt > tmpPnts;

	//==== First Point ====//
	wp.x = wp.y = wp.z = 0.0;
	wp.sectID = 0;
	wp.sectFract = 0.0;
	wp.chord = sects[0].rc_val();
	wp.twist = 0.0;
	wp.twistLoc = sects[0].twistLoc_val();
	wp.blendFlag = 0;

	if ( sects[0].dihedRotFlag )
		wp.dihedRot = -sects[0].dihedral_val();
	else
		wp.dihedRot = 0.0;

	tmpPnts.append( wp );

  //==== Compute Parameters For Each Section ====//
  double total_span = 0.0;
  double total_sweep_offset = 0.0;
  double total_dihed_offset = 0.0;

  //==== Load End Points for Each Section ====//
  double total_twist = 0.0;
  for ( i = 0 ; i < sects.dimension() ; i++ )
  {
	double rad = sects[i].span_val();
//    if ( get_sym_code() == XZ_SYM )  rad *= 0.5;   // Check Sym Code
	
	double ty = rad*cos(get_sum_dihedral(i)*DEG_2_RAD);
//	double ty = rad*cos(sects[i].dihedral*DEG_2_RAD);
	double tz = rad*sin(get_sum_dihedral(i)*DEG_2_RAD);	
//	double tz = rad*sin(sects[i].dihedral*DEG_2_RAD);	

//   sects[i].sweep_set(  sects[i].sweep_val() + sweep_off() );
//   double tan_le  = sects[i].tan_sweep_at( 0.0, getSymCode() );
//   sects[i].sweep_set(  sects[i].sweep_val() - sweep_off() );
	double tan_le  = sects[i].tan_sweep_at( 0.0, getSymCode(), sects[i].sweep_val()+sweep_off() );

    double toff    = tan_le*rad;                    // Tip X Offset

    total_dihed_offset += tz;
    total_span += ty;
    total_sweep_offset += toff;
	
	wp.x = total_sweep_offset;
	wp.y = total_span;
	wp.z = total_dihed_offset;

	wp.sectID = i;
	wp.sectFract = 1.0;
	wp.chord = sects[i].tc_val();

	total_twist += sects[i].twist_val();
	wp.twist = sects[i].twist_val();
	if ( rel_twist_flag )
		wp.twist = total_twist;

	wp.twistLoc = sects[i].twistLoc_val();
	wp.blendFlag = 0;

	wp.dihedRot = 0.0;
	if ( sects[i].dihedRotFlag )
	{
		if ( i < sects.dimension()-1 )
			wp.dihedRot = -0.5*(get_sum_dihedral(i) + get_sum_dihedral(i+1));
//			wp.dihedRot = -0.5*(sects[i].dihedral + sects[i+1].dihedral);
		else
			wp.dihedRot = -get_sum_dihedral(i);
//			wp.dihedRot = -sects[i].dihedral;
	}
	tmpPnts.append( wp );
  }

  //==== Compute Dihedral Blends ====//
  wingPnts.append( tmpPnts[0] );

  for ( i = 1 ; i < tmpPnts.dimension()-1 ; i++ )
	{
		int addFlag = 1;
		int sid = tmpPnts[i].sectID;
		
		if ( sects[sid].dihed_crv1_val() > 0.001 && sects[sid].dihed_crv2_val() > 0.001 )
		{
			double del_dihed = sects[sid].dihedral_val() - sects[sid+1].dihedral_val();

			if ( fabs(del_dihed) > 0.001 )
			{
				//==== Dihedral Points ====//
				vec2d p0(tmpPnts[i-1].y, tmpPnts[i-1].z);
				vec2d p1(tmpPnts[i].y,   tmpPnts[i].z );
				vec2d p2(tmpPnts[i+1].y, tmpPnts[i+1].z );
		
				//==== Find Endpoints of Curve ====//
				double blendOff1 = sects[sid].dihed_crv1_val();
				double blendOff2 = sects[sid].dihed_crv2_val();
				double blendStr1 = sects[sid].dihed_crv1_str_val();
				double blendStr2 = sects[sid].dihed_crv2_str_val();
				vec2d ep01 = p1 + ( p0 - p1 )*blendOff1;
				vec2d ep12 = p1 + ( p2 - p1 )*blendOff2;
				vec2d tep01 = ep01 + ( p1 - ep01 )*blendStr1;
				vec2d tep12 = ep12 + ( p1 - ep12 )*blendStr2;
				vec3d bp0( ep01.x(),  ep01.y(), 0.0 );
				vec3d bp1( tep01.x(), tep01.y(), 0.0 );
				vec3d bp2( tep12.x(), tep12.y(), 0.0 );
				vec3d bp3( ep12.x(),  ep12.y(), 0.0 );

				Bezier_curve bc;
				bc.init_num_sections( 1 );
				bc.put_pnt( 0, bp0 );
				bc.put_pnt( 1, bp1 );
				bc.put_pnt( 2, bp2 );
				bc.put_pnt( 3, bp3 );

				addFlag = 0;

				double fr1 = 1.0 - blendOff1;
				double fr2 = blendOff2;


				//==== Compute Number of Intermediate Sections ====// 	
      			int dxs = (int)(fabs(del_dihed)/(double)deg_per_seg()) + 1;
      			if ( dxs > max_num_segs() )
					dxs = max_num_segs.iget();
				if ( dxs < 2 )
					dxs = 2;

				//==== Force Even Number ====//
				if ( dxs%2 == 1 )
				dxs++;

				//==== Planform (Sweep) ====//
				double x0 = tmpPnts[i-1].x + fr1*( tmpPnts[i].x   - tmpPnts[i-1].x );
				double x1 = tmpPnts[i].x;
				double x2 = tmpPnts[i].x   + fr2*( tmpPnts[i+1].x - tmpPnts[i].x );

				//==== Chord ====//		
				double c0 = tmpPnts[i-1].chord + fr1*( tmpPnts[i].chord   - tmpPnts[i-1].chord );
				double c1 = tmpPnts[i].chord;
				double c2 = tmpPnts[i].chord + fr2*( tmpPnts[i+1].chord - tmpPnts[i].chord );

				//==== Twist ====//		
				double t0 = tmpPnts[i-1].twist + fr1*( tmpPnts[i].twist   - tmpPnts[i-1].twist );
				double t1 = tmpPnts[i].twist;
				double t2 = tmpPnts[i].twist + fr2*( tmpPnts[i+1].twist - tmpPnts[i].twist );

				//==== Twist Loc ====//		
				double tl0 = tmpPnts[i-1].twistLoc + fr1*( tmpPnts[i].twistLoc   - tmpPnts[i-1].twistLoc );
				double tl1 = tmpPnts[i].twistLoc;
				double tl2 = tmpPnts[i].twistLoc + fr2*( tmpPnts[i+1].twistLoc - tmpPnts[i].twistLoc );

				//==== Dihedral Rotation ====//		
				double dr1 = -get_sum_dihedral(sid);
//				double dr1 = -sects[sid].dihedral;
				double dr2 = -get_sum_dihedral(sid+1);
//				double dr2 = -sects[sid+1].dihedral;
					
				tmpPnts[i+1].dihedRot = dr2;

				//==== Find Fraction Cut Sect 1 - 2 ====//
				double len01 = dist( ep01, p1 );
				double len12 = dist( p1, ep12 );
				double tlen  = len01 + len12;
				double dcut  = 0.5;
				if ( tlen )
					dcut  = len01/tlen;

				//==== Array of u values ====//
				dyn_array< double > us;
				for ( j = 0 ; j <= dxs ; j++ )
				{
					double fract = (double)(j)/(double)dxs;
					us.append( fract );
				}		
   	
				//==== Find Total Dist Along Bezier ====//
				dyn_array< double > dists;
				double total_dist = 0.0;
				dists.append( total_dist );
				for ( j = 1 ; j < us.dimension() ; j++ )
				{
					vec3d p0 = bc.comp_pnt( 0, us[j-1] );
					vec3d p1 = bc.comp_pnt( 0, us[j] );
					total_dist += dist( p0, p1 );
					dists.append( total_dist );
				}
				if ( total_dist <= 0.0 )
					total_dist = 0.001f;
	
				//==== Find U value at dcut fract ====//
				double ucut = dcut;
				int numu1 = dxs/2+1;
				for ( j = 0 ; j < dists.dimension()-1 ; j++ )
				{
					double dfract0 = dists[j]/total_dist;
					double dfract1 = dists[j+1]/total_dist;
					if ( dcut >= dfract0 && dcut <= dfract1 )
					{
						double denom = dfract1-dfract0;
						double dfract = 0.0;
						if ( denom )
							dfract = (dcut - dfract0)/denom;
						ucut = us[j] + dfract*(us[j+1] - us[j]);

						if ( dfract < 0.5 )
							numu1 = j;
						else
							numu1 = j+1;
						if ( numu1 >= dxs )
							numu1 = dxs-1;
						if ( numu1 <= 0 )
							numu1 = 0;

						break;
					}
				}
 				//==== Blend along First Section ====//
				vec3d lastpnt;
				double sum_dist = 0.0;										
				double tdist = total_dist*dcut;
				for ( j = 0 ; j <= numu1 ; j++ )
				{
					double u = 0.0;
					if ( numu1 )
						u = ((double)j/(double)numu1)*ucut;

					vec3d bzpnt = bc.comp_pnt( 0, u );

					if ( j > 0 )
						sum_dist += dist( lastpnt, bzpnt );
					lastpnt = bzpnt;					

					double fract = 0.0;
					if ( tdist )
						fract = sum_dist/tdist;

					wp.y = bzpnt.x();
					wp.z = bzpnt.y();
					wp.dihedRot = dr1 + u*( dr2 - dr1 );

					wp.x        = x0  + fract*( x1  - x0 );
					wp.chord    = c0  + fract*( c1  - c0 );		
					wp.twist    = t0  + fract*( t1  - t0 );		
					wp.twistLoc = tl0 + fract*( tl1 - tl0 );
					wp.sectID = sid;
					wp.sectFract = (1.0 - blendOff1) + fract*(blendOff1);
					wp.blendFlag = 1;


					wingPnts.append( wp );
				}

 				//==== Blend along Second Section ====//
				sum_dist = 0.0;
				tdist = total_dist - total_dist*dcut;
				for ( j = numu1+1 ; j <= dxs ; j++ )
				{
					double u = ucut;
					double denom =  (double)(dxs-numu1);
					if ( denom )
						u = ucut + ((double)(j - numu1)/denom)*(1.0 - ucut);

					vec3d bzpnt = bc.comp_pnt( 0, u );
					sum_dist += dist( lastpnt, bzpnt );
					lastpnt = bzpnt;					

					double fract = 0.0;

					if ( tdist )
						fract = sum_dist/tdist;

					wp.y = bzpnt.x();
					wp.z = bzpnt.y();
					wp.dihedRot = dr1 + u*( dr2 - dr1 );

					wp.x        = x1  + fract*(x2  - x1);
					wp.chord    = c1  + fract*(c2  - c1);
					wp.twist    = t1  + fract*(t2  - t1);
					wp.twistLoc = tl1 + fract*(tl2 - tl1);
					wp.sectID = sid+1;
					wp.sectFract =  fract*blendOff2;						
					wp.blendFlag = 1;

					wingPnts.append( wp );
				}
 			}
		}
		if ( addFlag )
		{
			wingPnts.append( tmpPnts[i] );
		}
	}
	//==== Add Last Point ====//
	wingPnts.append( tmpPnts[tmpPnts.dimension()-1] );


}

void Ms_wing_geom::generate_surf()
{
	int i, j;
	int num_pnts  = numPnts.iget();

	//==== Load Up Original Plan Points ====//
	dyn_array< WingPnt > wingPnts;
	loadWingPnts( wingPnts );


	//==== Figure Out How Many Xsecs ====//
	int num_xs = wingPnts.dimension() + 2;				// End caps

	//==== Set Up Base Surf ====//
	base_surf.set_num_pnts( num_pnts );
	base_surf.set_num_xsecs( num_xs );

	//==== Clear Section Count ====//
	for ( i = 0 ; i < sects.dimension() ; i++ )
		sects[i].num_xsecs = 0;

	//==== Load In Airfoils and Endcaps ====//
	int ixcnt = 0;
	for ( i = 0 ; i < wingPnts.dimension() ; i++ )
	{
		int sid = wingPnts[i].sectID;
		Af* rfoil = sects[sid].rootAf;
		Af* tfoil = sects[sid].tipAf;

		Af* foil;									// jrg Should Interpolate Foils??
		if ( wingPnts[i].sectFract <  0.5 )
			foil = rfoil;
		else
			foil = tfoil;

		sects[sid].num_xsecs++;

		if ( i == 0 )	
		{
  			for ( int ip = 0 ; ip < num_pnts ; ip++ )
    			base_surf.set_pnt(ixcnt, ip, rfoil->get_end_cap(ip));	
 
			ixcnt++;
		}
			
  		for ( int ip = 0 ; ip < num_pnts ; ip++ )
		{
			vec3d pr = rfoil->get_pnt(ip);
			vec3d pt = tfoil->get_pnt(ip);
			vec3d pi = pr +  ( pt - pr )*wingPnts[i].sectFract;			
    		base_surf.set_pnt(ixcnt,ip, pi );
		}

		ixcnt++;

		if ( i == wingPnts.dimension()-1 )	
		{
  			for ( int ip = 0 ; ip < num_pnts ; ip++ )
			{
				if ( round_end_cap_flag )
					base_surf.set_pnt(ixcnt, ip, tfoil->get_rounded_end_cap(ip));
				else
					base_surf.set_pnt(ixcnt, ip, tfoil->get_end_cap(ip));
  				
			}

			ixcnt++;
		}
	}

	//==== Scale, Tranlate Rotate Airfoils ====//
	ixcnt = 0;
	for ( i = 0 ; i < wingPnts.dimension() ; i++ )
	{
		int numix = 1;
		if ( i == 0 || i == wingPnts.dimension()-1 )		//End caps
			numix = 2;

		for ( int j = 0 ; j < numix ; j++ )
		{
    		base_surf.scale_xsec_x(  ixcnt, wingPnts[i].chord );
//7/5/09
    		base_surf.scale_xsec_y(  ixcnt, wingPnts[i].chord );

    		base_surf.scale_xsec_z(  ixcnt, wingPnts[i].chord );
    		//base_surf.rotate_xsec_x( ixcnt, wingPnts[i].dihedRot );

    		base_surf.offset_xsec_x( ixcnt, -wingPnts[i].twistLoc * wingPnts[i].chord );
    		base_surf.rotate_xsec_y( ixcnt, wingPnts[i].twist);
    		base_surf.offset_xsec_x( ixcnt, wingPnts[i].twistLoc * wingPnts[i].chord);

    		base_surf.rotate_xsec_x( ixcnt, wingPnts[i].dihedRot );

    		base_surf.offset_xsec_x( ixcnt, wingPnts[i].x );
    		base_surf.offset_xsec_y( ixcnt, wingPnts[i].y );
    		base_surf.offset_xsec_z( ixcnt, wingPnts[i].z );
			
			ixcnt++;
		}
	}

	//==== Load Stringer Point Vecs ====//
	static vector< vector< vec3d > > stringerVec;
	stringerVec.resize( base_surf.get_num_pnts() );
	for ( i = 0 ; i < (int)stringerVec.size() ; i++ )
		stringerVec[i].resize( base_surf.get_num_xsecs() );

	for ( i = 0 ; i < (int)stringerVec.size() ; i++ )
	{
		for ( j = 0 ; j < (int)stringerVec[i].size() ; j++ )
		{
			stringerVec[i][j] = base_surf.get_pnt(j, i);
		}
	}

	//==== Set Up Tangent Flags for Bezier Curves ====//
	static vector< int > tanFlagVec;
	tanFlagVec.resize( num_xs );

	for ( i = 0 ; i < (int)tanFlagVec.size() ; i++ )
		tanFlagVec[i] = Bezier_curve::SHARP;

	for ( i = 2 ; i < (int)tanFlagVec.size()-1 ; i++ )
	{
		int sid = wingPnts[i-1].sectID;	
		if ( sects[sid].smoothBlendFlag )
			tanFlagVec[i] = Bezier_curve::NADA;
	}
	if ( round_end_cap_flag )
	{
		int ind = (int)tanFlagVec.size()-2;
		tanFlagVec[ind] = Bezier_curve::ONLY_BACK;
		ind = (int)tanFlagVec.size()-1;
		tanFlagVec[ind] = Bezier_curve::ZERO;
	}

	//==== Load Up Bezier Curves (Stringers) ====//
	int closeFlag = 0;
	static vector< Bezier_curve > sVec;
	sVec.resize( stringerVec.size() );

	for ( i = 0 ; i < (int)stringerVec.size() ; i++ )
	{
		sVec[i].buildCurve( stringerVec[i], tanFlagVec, 0.35, closeFlag);
	}

	if ( round_end_cap_flag )
	{
		for ( i = 0 ; i < (int)stringerVec.size() ; i++ )
		{
			//==== Find Tanget At Tip ====//
			int mirror = (int)stringerVec.size()-1-i;
			int num_ctl_pnts = sVec[i].get_num_control_pnts();
			vec3d ptop = sVec[i].get_pnt( num_ctl_pnts-4 );
			vec3d pmirror = sVec[mirror].get_pnt( num_ctl_pnts-4 );
			
			vec3d dir = ptop - pmirror;
			double mag = dir.mag();

			if ( mag )
			{
				dir.normalize();
				vec3d ep = sVec[i].get_pnt( num_ctl_pnts-1 );
				vec3d tp = ep + dir*0.35*mag;
				sVec[i].put_pnt( num_ctl_pnts-2, tp );
			}
		}
	}

	for ( i = 0 ; i < sects.dimension() ; i++ )
		sects[i].num_actual_xsecs = sects[i].num_xsecs;

	//==== What u Vals to Interpolate ====//
	vector< double > uVec;				// Vector of U vals 
	double uVal = 0.0;
	uVec.push_back(uVal);
	uVal = 1.0;
	uVec.push_back(uVal);

	for ( int ix = 1 ; ix < wingPnts.dimension() ; ix++ )
	{
		int sid = wingPnts[ix].sectID;	

		int num_interp = sects[sid].num_interp_xsecs;

		if ( wingPnts[ix-1].blendFlag && wingPnts[ix].blendFlag )
			num_interp = 0;

		sects[sid].num_actual_xsecs += num_interp;

		for ( j = 0 ; j < num_interp+1 ; j++ )		
		{
			uVal += 1.0/(double)(num_interp+1);
			uVec.push_back(uVal);
		}
	}
	if ( round_end_cap_flag )
	{
		int num_interp = 3;
		for ( j = 0 ; j < num_interp+1 ; j++ )		
		{
			uVal += 1.0/(double)(num_interp+1);
			uVec.push_back(uVal);
		}
	}
	else
	{
		uVal = uVal+1.0;				// Last Section
		uVec.push_back(uVal);
	}

	//==== Load up Interpolated Surface ====//
	mwing_surf.set_num_pnts( base_surf.get_num_pnts()  );
	mwing_surf.set_num_xsecs( uVec.size() );
	for ( int s = 0 ; s < (int)sVec.size() ; s++ )
	{
		for ( int ix = 0 ; ix < (int)mwing_surf.get_num_xsecs() ; ix++ )
		{
			double u  = uVec[ix];
			int sect  = (int)u;
			vec3d p = sVec[s].comp_pnt(sect, u-(double)sect);
			mwing_surf.set_pnt( ix, s, p );
		}
	}

	mwing_surf.load_refl_pnts_xsecs();
	mwing_surf.load_hidden_surf();
	mwing_surf.load_sharp_normals();

	mwing_surf.load_uw();

	generate_flap_slat_surfs();
}

//==== Write Aero File =====//
void Ms_wing_geom::write_aero_file(FILE* aero_file, int aero_id_num)
{
  //jrg check if used/needed
}

//==== Return Number Of Felisa Surface Written Out ====//
int Ms_wing_geom::get_num_felisa_comps()
{
  //==== Write out one side top/bot + endcap ====//
  int nxs = mwing_surf.get_num_xsecs();

  vec3d v = getTotalTranVec();

  if ( v.y() > 0.0 )
    return( 2*(nxs-3)+2 );
  else if ( sym_code == XZ_SYM )
    return( 2*(nxs-3)+1 );
  else
    return( nxs-3 + 2);
}

//==== Return Number Of Felisa Surface Written Out ====//
int Ms_wing_geom::get_num_felisa_wings()
{
  vec3d v = getTotalTranVec();

  if ( v.y() == 0.0 && sym_code == NO_SYM )
    return(0);
  else
    return(1);
}

//==== Write Out Felisa Surfaces To File ====//
int Ms_wing_geom::write_felisa_file(int geom_no, FILE* dump_file)
{
  int isec, j;
  vec3d temp;
  int num_geoms = geom_no;

  int nxsecs = mwing_surf.get_num_xsecs();
  int npnts  = mwing_surf.get_num_pnts()/2 + 1;

  vec3d v = getTotalTranVec();

  //==== Write Lower Wing Surfaces ====//
  for ( isec = 1 ; isec < nxsecs-2 ; isec++ )
	{
      fprintf(dump_file, "\n");
      fprintf(dump_file, "%d                    Surface_Number\n", num_geoms);
      num_geoms++;
      fprintf(dump_file, "%s-Lower-%d           Name\n", (char*) this->getName(),isec);

	  double sdist = dist( mwing_surf.get_pnt(isec,0), mwing_surf.get_pnt(isec+1,0) );
	  double cdist = dist( mwing_surf.get_pnt(isec,0), mwing_surf.get_pnt(isec,npnts) );
      int num_slices = (int)(npnts*( sdist/cdist ) );
      num_slices = MAX(num_slices,20);
      fprintf(dump_file, "%d                    U_Render\n",num_slices);
      fprintf(dump_file, "2                     W_Render\n");
      fprintf(dump_file, "2                     Num_xsecs\n");
      fprintf(dump_file, "%d                    Num_pnts\n", npnts);

      for ( j = npnts-1 ; j < mwing_surf.get_num_pnts() ; j++ )
        {
          temp = mwing_surf.get_pnt( isec, j).transform(model_mat);
          fprintf(dump_file, "%11.6f  %11.6f  %11.6f\n", temp.x(), temp.y(),temp.z());
        }
      for ( j = npnts-1 ; j < mwing_surf.get_num_pnts() ; j++ )
        {
          temp = mwing_surf.get_pnt( isec+1, j).transform(model_mat);
          fprintf(dump_file, "%11.6f  %11.6f  %11.6f\n", temp.x(), temp.y(),temp.z());
        }
  }
  //==== Check For Vertical Tail Config ====//
  if ( v.y() <= 0.0 && sym_code == NO_SYM )
    {
      //===== Write Top End Cap (Half) =====//
      fprintf(dump_file, "\n");
      fprintf(dump_file, "%d            Surface_Number\n", num_geoms);
      num_geoms++;
      fprintf(dump_file, "%s            Name\n", (char*) this->getName());
      fprintf(dump_file, "2                     U_Render\n");
      fprintf(dump_file, "2                     W_Render\n");
      fprintf(dump_file, "2                     Num_xsecs\n");
      fprintf(dump_file, "%d            Num_pnts\n", npnts);

      for ( j = npnts-1 ; j < mwing_surf.get_num_pnts() ; j++ )
        {
          temp =  mwing_surf.get_pnt(1,j).transform(model_mat);
          fprintf(dump_file, "%11.6f  %11.6f  %11.6f\n", temp.x(), temp.y(), temp.z());
        }

      for ( j = npnts-1 ; j < mwing_surf.get_num_pnts() ; j++ )
        {
          temp =  mwing_surf.get_pnt(1,j).transform(model_mat);
          fprintf(dump_file, "%11.6f   0.0      %11.6f\n", temp.x(), temp.z());
        }

      //===== Write Bottom End Cap (Half) =====//
      fprintf(dump_file, "\n");
      fprintf(dump_file, "%d            Surface_Number\n", num_geoms);
      num_geoms++;
      fprintf(dump_file, "%s            Name\n", (char*) this->getName());
      fprintf(dump_file, "2                     U_Render\n");
      fprintf(dump_file, "2                     W_Render\n");
      fprintf(dump_file, "2                     Num_xsecs\n");
      fprintf(dump_file, "%d            Num_pnts\n", npnts);

      for ( j = npnts-1 ; j < mwing_surf.get_num_pnts() ; j++ )
        {
          temp = mwing_surf.get_pnt(nxsecs-2,j).transform(model_mat);
          fprintf(dump_file, "%11.6f  %11.6f  %11.6f\n", temp.x(), temp.y(), temp.z());
        }

      for ( j = npnts-1 ; j < mwing_surf.get_num_pnts() ; j++ )
        {
          temp = mwing_surf.get_pnt(nxsecs-2,j).transform(model_mat);
          fprintf(dump_file, "%11.6f   0.0      %11.6f\n", temp.x(), temp.z());
        }
    }
  else
    {
      for (  isec = 1 ; isec < nxsecs-2 ; isec++ )
        {
          //==== Write Upper Wing Surfaces ====//
          fprintf(dump_file, "\n");
          fprintf(dump_file, "%d                        Surface_Number\n", num_geoms);
          num_geoms++;
          fprintf(dump_file, "%s-Upper-%d               Name\n", (char*) this->getName(),isec);
					double sdist = dist( mwing_surf.get_pnt(isec,0), mwing_surf.get_pnt(isec+1,0) );
					double cdist = dist( mwing_surf.get_pnt(isec,0), mwing_surf.get_pnt(isec,npnts) );
      		int num_slices = (int)(npnts*( sdist/cdist ) );
      		num_slices = MAX(num_slices,20);
          fprintf(dump_file, "%d                U_Render\n",num_slices);
          fprintf(dump_file, "2                 W_Render\n");
          fprintf(dump_file, "2                 Num_xsecs\n");
          fprintf(dump_file, "%d                        Num_pnts\n", npnts);

          for ( j = 0 ; j < npnts ; j++ )
            {
              temp = mwing_surf.get_pnt( isec, j).transform(model_mat);
              fprintf(dump_file, "%11.6f  %11.6f  %11.6f\n", temp.x(), temp.y(),temp.z());
            }
          for ( j = 0 ; j < npnts ; j++ )
            {
              temp = mwing_surf.get_pnt( isec+1, j).transform(model_mat);
              fprintf(dump_file, "%11.6f  %11.6f  %11.6f\n", temp.x(), temp.y(),temp.z());
            }
        }

      //==== Write Outer End Cap ====//
      fprintf(dump_file, "\n");
      fprintf(dump_file, "%d            Surface_Number\n", num_geoms);
      num_geoms++;
      fprintf(dump_file, "%s            Name\n", (char*) this->getName());
      fprintf(dump_file, "2             U_Render\n");
      fprintf(dump_file, "2             W_Render\n");
      fprintf(dump_file, "2             Num_xsecs\n");
      fprintf(dump_file, "%d            Num_pnts\n", npnts);

      for ( j = 0 ; j < npnts ; j++ )
        {
          temp = mwing_surf.get_pnt(nxsecs-2,j).transform(model_mat);
          fprintf(dump_file, "%11.6f  %11.6f  %11.6f\n", temp.x(), temp.y(), temp.z());
        }
      for ( j = mwing_surf.get_num_pnts()-1 ; j >= npnts-1 ; j-- )
        {
          temp = mwing_surf.get_pnt(nxsecs-2,j).transform(model_mat);
          fprintf(dump_file, "%11.6f  %11.6f  %11.6f\n", temp.x(), temp.y(), temp.z());
        }

      if ( v.y() > 0.0 )
        {
          //==== Write Inner End Cap ====//
          fprintf(dump_file, "\n");
          fprintf(dump_file, "%d                Surface_Number\n", num_geoms);
          num_geoms++;
          fprintf(dump_file, "%s                Name\n", (char*) this->getName());
          fprintf(dump_file, "2                 U_Render\n");
          fprintf(dump_file, "2                 W_Render\n");
          fprintf(dump_file, "2                 Num_xsecs\n");
          fprintf(dump_file, "%d                Num_pnts\n", npnts);

          for ( j = 0 ; j < npnts ; j++ )
            {
              temp = mwing_surf.get_pnt(1,j).transform(model_mat);
              fprintf(dump_file, "%11.6f  %11.6f  %11.6f\n", temp.x(), temp.y(), temp.z());
            }
          for ( j = mwing_surf.get_num_pnts()-1 ; j >= npnts-1 ; j-- )
            {
              temp = mwing_surf.get_pnt(1,j).transform(model_mat);
              fprintf(dump_file, "%11.6f  %11.6f  %11.6f\n", temp.x(), temp.y(), temp.z());
            }
        }

    }

 return(num_geoms);
}

//==== Write Out Felisa Surfaces To File ====//
void Ms_wing_geom::write_bco_info(FILE* file_ptr, int& surf_cnt, int& wing_cnt)
{
  int ixs;
  int nxsecs = mwing_surf.get_num_xsecs();

  vec3d v = getTotalTranVec();

  //==== Check For Vertical Tail Config ====//
  if ( v.y() == 0.0 && sym_code == NO_SYM )
    {
      for ( ixs = 1 ; ixs < nxsecs-2 ; ixs++ )
      {
      	fprintf( file_ptr, "%d        0      0     0.0     0.0     0.0     0.0   %s Surf %d\n", surf_cnt, (char*)getName(), ixs );
      	surf_cnt++;
      }
      fprintf( file_ptr, "%d        0      0     0.0     0.0     0.0     0.0   %s End Cap 1\n", surf_cnt, (char*)getName() );
      surf_cnt++;
      fprintf( file_ptr, "%d        0      0     0.0     0.0     0.0     0.0   %s End Cap 2\n", surf_cnt, (char*)getName() );
      surf_cnt++;
    }
  else if ( v.y() == 0.0 )
    {
      for ( ixs = 1 ; ixs < nxsecs-2 ; ixs++ )
      {
      	fprintf( file_ptr, "%d        0    -%d     0.0     0.0     0.0     0.0   %s Lower Surf %d\n", surf_cnt, wing_cnt, (char*)getName(), ixs );
      	surf_cnt++;
      }
      for ( int ixs = 1 ; ixs < nxsecs-2 ; ixs++ )
      {
      	fprintf( file_ptr, "%d        0     %d     0.0     0.0     0.0     0.0   %s Upper Surf %d\n", surf_cnt, wing_cnt, (char*)getName(), ixs );
      	surf_cnt++;
			}
      fprintf( file_ptr, "%d        0      0     0.0     0.0     0.0     0.0   %s End Cap 1\n", surf_cnt, (char*)getName() );
      surf_cnt++;

      wing_cnt++;
    }
  else
    {
      for ( ixs = 1 ; ixs < nxsecs-2 ; ixs++ )
      {
      	fprintf( file_ptr, "%d        0    -%d     0.0     0.0     0.0     0.0   %s Lower Surf %d\n", surf_cnt, wing_cnt, (char*)getName(), ixs );
      	surf_cnt++;
      }
      for ( ixs = 1 ; ixs < nxsecs-2 ; ixs++ )
      {
      	fprintf( file_ptr, "%d        0     %d     0.0     0.0     0.0     0.0   %s Upper Surf %d\n", surf_cnt, wing_cnt, (char*)getName(), ixs );
      	surf_cnt++;
			}
      fprintf( file_ptr, "%d        0      0     0.0     0.0     0.0     0.0   %s End Cap 1\n", surf_cnt, (char*)getName() );
      surf_cnt++;
      fprintf( file_ptr, "%d        0      0     0.0     0.0     0.0     0.0   %s End Cap 1\n", surf_cnt, (char*)getName() );
      surf_cnt++;

      wing_cnt++;
    }
}


//==== Write Out Felisa Background Lines File ====//
void Ms_wing_geom::write_felisa_line_sources(FILE* dump_file)
{
  int ix;
  int nxsecs = mwing_surf.get_num_xsecs();
  int npnts  = mwing_surf.get_num_pnts()/2 + 1;

  //==== Find Root Chord Pnts ====//
  vec3d te_rpnt = mwing_surf.get_pnt(1,0).transform(model_mat);
  int half_ind = mwing_surf.get_num_pnts()/2;
  vec3d le_rpnt = mwing_surf.get_pnt(1,half_ind).transform(model_mat);
  double root_chord =  dist(le_rpnt,te_rpnt);

  //==== Find Tip Chord Pnts ====//
  vec3d te_tpnt = mwing_surf.get_pnt(nxsecs-2,0).transform(model_mat);
  vec3d le_tpnt = mwing_surf.get_pnt(nxsecs-2,half_ind).transform(model_mat);
  double tip_chord = dist( te_tpnt, le_tpnt );

  double span = total_span();
  span = MAX(MAX(0.5*span,root_chord),tip_chord);

  //==== Write Out Line Along Trailing Edge ====//
	for ( ix = 1 ; ix < nxsecs-2 ;  ix++ )
  {
		vec3d te_pnt1 = mwing_surf.get_pnt(ix,0).transform(model_mat);
		vec3d te_pnt2 = mwing_surf.get_pnt(ix+1,0).transform(model_mat);

		vec3d le_pnt1 = mwing_surf.get_pnt(ix,half_ind).transform(model_mat);
		vec3d le_pnt2 = mwing_surf.get_pnt(ix+1,half_ind).transform(model_mat);

		double chord1 = dist( te_pnt1, le_pnt1 );
		double chord2 = dist( te_pnt2, le_pnt2 );

  	double ds_root_te = root_chord/30.;
  	double ds1 = MAX(chord1/root_chord,0.50)*ds_root_te;
  	double ds2 = MAX(chord2/root_chord,0.50)*ds_root_te;

  	ds1 = MAX(ds1, total_span()/1000.);
		ds2 = MAX(ds2, total_span()/1000.);

  	//==== Write Out Line Along Trailing Edge ====//
  	fprintf(dump_file, " %s: Trailing Edge Line %d \n", (char*) getName(), ix);
  	fprintf(dump_file, "    %f    %f    %f    %f    %f    %f  \n",
      te_pnt1.x(), te_pnt1.y(), te_pnt1.z(), ds1, 2.*ds1 ,9.*ds1);
  	fprintf(dump_file, "    %f    %f    %f    %f    %f    %f  \n",
      te_pnt2.x(), te_pnt2.y(), te_pnt2.z(), ds2, 2.*ds2 ,9.*ds2);
	}

  //==== Write Out Line Along Leading Edge ====//
	for ( ix = 1 ; ix < nxsecs-2 ;  ix++ )
  {
		vec3d le_pnt1 = mwing_surf.get_pnt(ix,half_ind).transform(model_mat);
		vec3d le_pnt2 = mwing_surf.get_pnt(ix+1,half_ind).transform(model_mat);

		vec3d te_pnt1 = mwing_surf.get_pnt(ix,0).transform(model_mat);
		vec3d te_pnt2 = mwing_surf.get_pnt(ix+1,0).transform(model_mat);

		double chord1 = dist( te_pnt1, le_pnt1 );
		double chord2 = dist( te_pnt2, le_pnt2 );

  	double ds_root_le = root_chord/40.;
  	double ds1 = MAX(chord1/root_chord,0.50)*ds_root_le;
  	double ds2 = MAX(chord2/root_chord,0.50)*ds_root_le;

  	ds1 = MAX(ds1, total_span()/1000.);
		ds2 = MAX(ds2, total_span()/1000.);

  	//==== Write Out Line Along Leading Edge ====//
  	fprintf(dump_file, " %s: Leading Edge Line %d \n", (char*)getName(), ix);
  	fprintf(dump_file, "    %f    %f    %f    %f    %f    %f  \n",
      le_pnt1.x(), le_pnt1.y(), le_pnt1.z(), ds1, 2.*ds1 ,9.*ds1);
  	fprintf(dump_file, "    %f    %f    %f    %f    %f    %f  \n",
      le_pnt2.x(), le_pnt2.y(), le_pnt2.z(), ds2, 2.*ds2 ,9.*ds2);

  	//==== Write Out Line Along Center Line ====//
  	fprintf(dump_file, " %s: Center Edge Line %d \n", (char*) getName(), ix);
  	fprintf(dump_file, "    %f    %f    %f    %f    %f    %f  \n",
      0.5*(le_pnt1.x()+te_pnt1.x()), 0.5*(le_pnt1.y()+te_pnt1.y()), 0.5*(le_pnt1.z()+te_pnt1.z()),
			span/40., 2.*span/40., 9.*span/40.);
  	fprintf(dump_file, "    %f    %f    %f    %f    %f    %f  \n",
      0.5*(le_pnt2.x()+te_pnt2.x()), 0.5*(le_pnt2.y()+te_pnt2.y()), 0.5*(le_pnt2.z()+te_pnt2.z()),
      span/40., 2.*span/40., 9.*span/40.);
	}

  //==== Write Out Line Along Root Chord ====//
  double ds_root_te = root_chord/30.;
  ds_root_te = MAX(ds_root_te, total_span()/1000.);
  fprintf(dump_file, " %s: Root Chord Edge Line \n", (char*) getName());
  fprintf(dump_file, "    %f    %f    %f    %f    %f    %f  \n",
      le_rpnt.x(), le_rpnt.y(), le_rpnt.z(), ds_root_te, 2.*ds_root_te, 9.*ds_root_te);
  fprintf(dump_file, "    %f    %f    %f    %f    %f    %f  \n",
      te_rpnt.x(), te_rpnt.y(), te_rpnt.z(), ds_root_te, 2.*ds_root_te, 9.*ds_root_te);

  //==== Write Out Line Along Tip Chord ====//
  double ds_tip_te  = MAX(tip_chord/root_chord,0.50)*ds_root_te;
  ds_tip_te  = MAX(ds_tip_te, total_span()/1000.);
  fprintf(dump_file, " %s: Tip Chord Edge Line \n", (char*) getName());
  fprintf(dump_file, "    %f    %f    %f    %f    %f    %f  \n",
      le_tpnt.x(), le_tpnt.y(), le_tpnt.z(), ds_tip_te, 2.*ds_tip_te, 9.*ds_tip_te);
  fprintf(dump_file, "    %f    %f    %f    %f    %f    %f  \n",
      te_tpnt.x(), te_tpnt.y(), te_tpnt.z(), ds_tip_te, 2.*ds_tip_te, 9.*ds_tip_te);

  //==== Write Out 2nd Line Along Tip Chord ====//
  fprintf(dump_file, " %s: Second Tip Chord Edge Line \n", (char*) getName());
  fprintf(dump_file, "    %f    %f    %f    %f    %f    %f  \n",
      le_tpnt.x(), le_tpnt.y(), le_tpnt.z(), span/40., 2.*span/40., 9.*span/40.);
  fprintf(dump_file, "    %f    %f    %f    %f    %f    %f  \n",
      te_tpnt.x(), te_tpnt.y(), te_tpnt.z(), span/40., 2.*span/40., 9.*span/40.);

}

int Ms_wing_geom::get_num_felisa_line_sources()
{
  int nls = 0;
  int nxsecs = mwing_surf.get_num_xsecs();

  nls += nxsecs-3;    // Trailing Edge
	nls += nxsecs-3;		// Leading
  nls += nxsecs-3;		// Center
  nls += 3;						// Root, Tip Tip

  return nls;

}

//==== Write Out Felisa Background Triangle File ====//
void Ms_wing_geom::write_felisa_tri_sources(FILE* dump_file)
{
  int ix;
  int nxsecs = mwing_surf.get_num_xsecs();
  int npnts  = mwing_surf.get_num_pnts()/2 + 1;

  //==== Find Root Chord Pnts ====//
  vec3d te_rpnt = mwing_surf.get_pnt(1,0).transform(model_mat);
  int half_ind = mwing_surf.get_num_pnts()/2;
  vec3d le_rpnt = mwing_surf.get_pnt(1,half_ind).transform(model_mat);
  double root_chord =  dist(le_rpnt,te_rpnt);

  //==== Find Tip Chord Pnts ====//
  vec3d te_tpnt = mwing_surf.get_pnt(nxsecs-2,0).transform(model_mat);
  vec3d le_tpnt = mwing_surf.get_pnt(nxsecs-2,half_ind).transform(model_mat);
  double tip_chord = dist( te_tpnt, le_tpnt );

  double span = total_span();
  span = MAX(MAX(0.5*span,root_chord),tip_chord);

  //====  Write Out Tris ====//
	for ( ix = 1 ; ix < nxsecs-2 ;  ix++ )
  {
		vec3d te_pnt1 = mwing_surf.get_pnt(ix,0).transform(model_mat);
		vec3d te_pnt2 = mwing_surf.get_pnt(ix+1,0).transform(model_mat);

		vec3d le_pnt1 = mwing_surf.get_pnt(ix,half_ind).transform(model_mat);
		vec3d le_pnt2 = mwing_surf.get_pnt(ix+1,half_ind).transform(model_mat);

		double chord1 = dist( te_pnt1, le_pnt1 );
		double chord2 = dist( te_pnt2, le_pnt2 );

    double ds_root = root_chord/30.;
  	double ds1 = MAX(chord1/root_chord,0.25)*ds_root;
  	double ds2 = MAX(chord2/root_chord,0.25)*ds_root;

  	fprintf(dump_file, " %s: Trailing Edge Tri %d \n", (char*) getName(), ix);
  	fprintf(dump_file, "    %f    %f    %f    %f    %f    %f  \n",
      te_pnt1.x(), te_pnt1.y(), te_pnt1.z(), ds1, 3.*ds1, 9.*ds1);
  	fprintf(dump_file, "    %f    %f    %f    %f    %f    %f  \n",
      te_pnt2.x(), te_pnt2.y(), te_pnt2.z(), ds2, 3.*ds2, 9.*ds2);
  	fprintf(dump_file, "    %f    %f    %f    %f    %f    %f  \n",
      le_pnt1.x(), le_pnt1.y(), le_pnt1.z(), ds1, 3.*ds1, 9.*ds1);

  	fprintf(dump_file, " %s: Leading Edge Tri %d \n", (char*) getName(), ix);
  	fprintf(dump_file, "    %f    %f    %f    %f    %f    %f  \n",
      le_pnt1.x(), le_pnt1.y(), le_pnt1.z(), ds1, 3.*ds1, 9.*ds1);
  	fprintf(dump_file, "    %f    %f    %f    %f    %f    %f  \n",
      le_pnt2.x(), le_pnt2.y(), le_pnt2.z(), ds2, 3.*ds2, 9.*ds2);
  	fprintf(dump_file, "    %f    %f    %f    %f    %f    %f  \n",
      te_pnt2.x(), te_pnt2.y(), te_pnt2.z(), ds2, 3.*ds2, 9.*ds2);
  }



}

int Ms_wing_geom::get_num_felisa_tri_sources()
{
  int nxsecs = mwing_surf.get_num_xsecs();
  return ( 2*(nxsecs - 3) );	

}

void Ms_wing_geom::loadWingStrakeGeom( WingGeom* wg )
{
	vector< WingSect > wsVec;
	double rootXOff = wg->loadMSWingSectVec( wsVec );

	if ( sects.dimension() != wsVec.size() || wsVec.size() != 3 )		//jrg should do better
		return;

	this->copy( wg );


	*sects[0].rootAf = *(wg->get_root_af_ptr());
	*sects[2].tipAf  = *(wg->get_tip_af_ptr());

	if ( wg->get_aft_ext_span_per()->get() < wg->get_strake_span_per()->get() )
	{
		*sects[0].tipAf    = *(wg->get_aft_af_ptr());
		*sects[1].rootAf   = *(wg->get_aft_af_ptr());
		*sects[1].tipAf    = *(wg->get_strake_af_ptr());
		*sects[2].rootAf   = *(wg->get_strake_af_ptr());
	}
	else
	{
		*sects[0].tipAf   = *(wg->get_strake_af_ptr());
		*sects[1].rootAf  = *(wg->get_strake_af_ptr());
		*sects[1].tipAf   = *(wg->get_aft_af_ptr());
		*sects[2].rootAf  = *(wg->get_aft_af_ptr());
	}

	for ( int i = 0 ; i < (int)wsVec.size() ; i++ )
	{
		Af* raf  = sects[i].rootAf;
		Af* taf  = sects[i].tipAf;
		sects[i] = wsVec[i];

		sects[i].dihedRotFlag = 0;
		sects[i].rootAf = raf;
		sects[i].tipAf  = taf;
		sects[i].rootAf->set_geom(this);
		sects[i].tipAf->set_geom(this);

		sects[i].fillDependData();
	}

	xLoc = xLoc.get() - rootXOff;

	set_curr_sect( currSect );

	generate();
	updateAttach(0);

}

double Ms_wing_geom::getRefArea()
{
	if ( autoRefAreaFlag )
	{
		refArea = get_total_area()->get();
	}
	return refArea;
}
	
double Ms_wing_geom::getRefSpan()
{
	if ( autoRefSpanFlag )
	{
		refSpan = get_total_span()->get();
	}
	return refSpan;
}

double Ms_wing_geom::getRefCbar()
{
	if ( autoRefCbarFlag )
		computeCbarAC();

	return refCbar;
}

vec3d Ms_wing_geom::getAeroCenter()
{
	if ( autoAeroCenterFlag )
		computeCbarAC();

	return aeroCenter;
}

void Ms_wing_geom::computeCbarAC()
{

	dyn_array< WingPnt > wingPnts;
	loadWingPnts( wingPnts );

	double area = 0.0;
	double mac  = 0.0;
	double xmac = 0.0;
	double ymac = 0.0;
	double zmac = 0.0;

	//==== Area Weighted Avg of Each Section ====//
	for ( int i = 1 ; i < wingPnts.dimension() ; i++ )
	{
		WingPnt c1 = wingPnts[i-1];
		WingPnt c2 = wingPnts[i];
		double span   = c2.y - c1.y;
		double taper  = c2.chord/c1.chord;
		double taper1 = taper + 1.0;
		double fract  = (taper+taper1)/(3.0*taper1);
 
		double aseg   = 0.5*(c1.chord +c2.chord)*span;
		double macseg = c1.chord*(taper*taper + taper + 1.0)/(1.5*taper1);
		double xseg   = c1.x + fract*(c2.x - c1.x);
		double yseg   = c1.y + fract*(c2.y - c1.y);
		double zseg   = c1.z + fract*(c2.z - c1.z);

		area += aseg;
		mac  += aseg*macseg;
		xmac += aseg*xseg;
		ymac += aseg*yseg;
		zmac += aseg*zseg;
	}

	if ( area )
	{
		mac  = mac/area;
		xmac = xmac/area;
		ymac = ymac/area;
		zmac = zmac/area;
	}

	if ( sym_code == XZ_SYM )
	{
		ymac = 0.0;
	}

	if ( autoRefCbarFlag )
		refCbar = mac;

	if ( autoAeroCenterFlag )
	{
		if ( getSymCode() == XZ_SYM )
			aeroCenter.set_xyz( xmac + 0.25*mac + get_tran_x(), ymac, zmac + get_tran_z() );
		else
			aeroCenter.set_xyz( xmac + 0.25*mac + get_tran_x(), ymac + get_tran_y(), zmac + get_tran_z() );
	}


}

void Ms_wing_geom::AddDefaultSources(double base_len)
{
	double def_len = 0.2*base_len;
	double def_rad = 0.1*total_span();

	//==== Root Tip Chords ====//
	LineSource* ls = new LineSource();
	ls->SetName("Root_Chord");
	ls->SetLen( def_len );
	ls->SetLen2( def_len );
	ls->SetRad( def_rad );
	ls->SetRad2( def_rad );
	ls->SetUWLoc1( vec2d(0,0) );
	ls->SetUWLoc2( vec2d(0,0.5) );
	sourceVec.push_back( ls );

	ls = new LineSource();
	ls->SetName("Tip_Chord");
	ls->SetLen( def_len );
	ls->SetLen2( def_len );
	ls->SetRad( def_rad );
	ls->SetRad2( def_rad );
	ls->SetUWLoc1( vec2d(1,0) );
	ls->SetUWLoc2( vec2d(1,0.5) );
	sourceVec.push_back( ls );

	//==== Find U Vals For Joints ====//
	dyn_array< WingPnt > wingPnts;
	loadWingPnts( wingPnts );

	vector< double > jointVec;
	vector< double > uVec;				// Vector of U vals 
	double uVal = 0.0;	uVec.push_back(uVal);
	uVal = 1.0;			uVec.push_back(uVal);	jointVec.push_back(uVal);

	for ( int ix = 1 ; ix < wingPnts.dimension() ; ix++ )
	{
		int sid = wingPnts[ix].sectID;	
		int num_interp = sects[sid].num_interp_xsecs;

		if ( wingPnts[ix-1].blendFlag && wingPnts[ix].blendFlag )
			num_interp = 0;

		for ( int j = 0 ; j < num_interp+1 ; j++ )		
		{
			uVal += 1.0;
//			uVal += 1.0/(double)(num_interp+1);
			uVec.push_back(uVal);
		}
		jointVec.push_back(uVal);
	}

	uVal = uVal+1.0;				// Last Section
	uVec.push_back(uVal);

	for ( int i = 0 ; i < (int)jointVec.size()-1 ; i++ )
	{
		double u0 = jointVec[i]/uVal;
		double u1 = jointVec[i+1]/uVal;
		ls = new LineSource();
		ls->SetName("Leading Edge");
		ls->SetLen( def_len );
		ls->SetLen2( def_len );
		ls->SetRad( def_rad );
		ls->SetRad2( def_rad );
		ls->SetUWLoc1( vec2d(u0,0.5) );
		ls->SetUWLoc2( vec2d(u1,0.5) );
		sourceVec.push_back( ls );

		ls = new LineSource();
		ls->SetName("Trailing Edge");
		ls->SetLen( def_len );
		ls->SetLen2( def_len );
		ls->SetRad( def_rad );
		ls->SetRad2( def_rad );
		ls->SetUWLoc1( vec2d(u0,0.0) );
		ls->SetUWLoc2( vec2d(u1,0.0) );
		sourceVec.push_back( ls );
	}
}

void Ms_wing_geom::LoadDragFactors( DragFactors& drag_factors )
{
	double min_chord = 1.0e12;
	double max_chord = 0.0;
	double avg_chord = 0.0;

	double min_toverc = 1.0e12;
	double max_toverc = 0.0;
	double avg_toverc = 0.0;

	double avg_sweep = 0.0;

	double total_area = 1.0e-12;
	for ( int i = 0 ; i < sects.dimension() ; i++ )
	{
		double rc = sects[i].get_rc()->get();
		double tc = sects[i].get_tc()->get();
		double ac = 0.5*(rc+tc);

		if ( rc < min_chord ) min_chord = rc;
		if ( rc > max_chord ) max_chord = rc;
		if ( tc < min_chord ) min_chord = tc;
		if ( tc > max_chord ) max_chord = tc;

		double rtoverc = sects[i].rootAf->get_thickness()->get();
		double ttoverc = sects[i].tipAf->get_thickness()->get();
		double atoverc = 0.5*(rtoverc+ttoverc);

		if ( rtoverc < min_toverc ) min_toverc = rtoverc;
		if ( rtoverc > max_toverc ) max_toverc = rtoverc;
		if ( ttoverc < min_toverc ) min_toverc = ttoverc;
		if ( ttoverc > max_toverc ) max_toverc = ttoverc;

		double area = sects[i].get_area()->get();
		double sweep = sects[i].get_sweep()->get();

		avg_chord += ac*area;
		avg_toverc += atoverc*area;
		avg_sweep += sweep*area;

		total_area += area;
	}

	drag_factors.m_MinChord = min_chord;
	drag_factors.m_AvgChord = avg_chord/total_area;
	drag_factors.m_MaxChord = max_chord;

	drag_factors.m_MinThickToChord = min_toverc;
	drag_factors.m_AvgThickToChord = avg_toverc/total_area;
	drag_factors.m_MaxThickToChord = max_toverc;

	drag_factors.m_AvgSweep = avg_sweep/total_area;

}

void Ms_wing_geom::GetInteriorPnts( vector< vec3d > & pVec )
{
	int np = mwing_surf.get_num_pnts();
	int nxs = mwing_surf.get_num_xsecs();
	vec3d p0 = mwing_surf.get_pnt( nxs/2, np/4 );
	vec3d p1 = mwing_surf.get_pnt( nxs/2, 3*np/4 );
	vec3d p  = (p0 + p1)*0.5;
	vec3d tp = p.transform( model_mat );
	pVec.push_back( tp );


	if (sym_code != NO_SYM)
	{
		tp = (p * sym_vec).transform(reflect_mat);
		pVec.push_back( tp );
    }
}
