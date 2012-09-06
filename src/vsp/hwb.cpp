//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//

#include "hwb.h"
#include "aircraft.h"

#define LARGEST  2
#define MODERATE 1
#define SMALLEST 0

//******************************************************************************
//
//    HWB Geometry Class based on Multi Section Wing
//
//   Michael Link - 6/11/08
//   AVID LLC
//       Based on ms_wing
//
//******************************************************************************

//#include "ms_wing.h"
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

//==============================================================//
//==============================================================//
//==============================================================//

//==== Constructor =====//
HwbBaseGeom::HwbBaseGeom(Aircraft* aptr) : Geom(aptr)
{
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

	copySect.rootAf = new Af( this );
	copySect.rootAf->init_script("wing_foil");
	copySect.tipAf = new Af( this );
	copySect.tipAf->init_script("wing_foil");

	//==== Define Some Decent Default Parms ====//
	define_parms();
	setSymCode(XZ_SYM);
	generate();

	surfVec.push_back( &mwing_surf );

}

//==== Destructor =====//
HwbBaseGeom::~HwbBaseGeom()
{
	//==== Delete Foils ====//
	for ( int i = 0 ; i < foils.dimension() ; i++ )
		delete foils[i];

	delete copySect.rootAf;
	delete copySect.tipAf;

}

void HwbBaseGeom::copy( Geom* fromGeom )
{
	int i;
	Geom::copy( fromGeom );						// Copy Base Stuff

	if ( fromGeom->getType() != MS_WING_GEOM_TYPE && fromGeom->getType() != HWB_GEOM_TYPE)
		return;

	HwbBaseGeom* g = (HwbBaseGeom*)fromGeom;

	total_area.set( g->total_area() );
	total_span.set( g->total_span() );
	total_proj_span.set( g->total_proj_span() );
	avg_chord.set( g->avg_chord() );
	sect_proj_span.set( g->sect_proj_span() );

	sweep_off.set( g->sweep_off() );

	total_aspect.set( g->total_aspect() );

	sect_aspect.set( g->sect_aspect() );
	sect_taper.set( g->sect_taper() );
	sect_area.set( g->sect_area() );
	sect_span.set( g->sect_span() );
	sect_rc.set( g->sect_rc() );
	sect_tc.set( g->sect_tc() );
	sect_sweep.set( g->sect_sweep() );
	sect_sweep_loc.set( g->sect_sweep_loc() );
	sect_twist.set( g->sect_twist() );
	sect_twist_loc.set( g->sect_twist_loc() );

	sect_dihed1.set( g->sect_dihed1() );
	sect_dihed2.set( g->sect_dihed2() );
	sect_dihed_crv1.set( g->sect_dihed_crv1() );
	sect_dihed_crv2.set( g->sect_dihed_crv2() );
	sect_dihed_crv1_str.set( g->sect_dihed_crv1_str() );
	sect_dihed_crv2_str.set( g->sect_dihed_crv2_str() );

	deg_per_seg.set( g->deg_per_seg() );
	max_num_segs.set( g->max_num_segs() );

	driver = g->get_driver();
	trimmed_span = g->get_trimmed_span();

	rel_twist_flag    = g->get_rel_twist_flag();
	rel_dihedral_flag = g->get_rel_dihedral_flag();
	round_end_cap_flag = g->get_round_end_cap_flag();

	//==== Delete Foils ====//
	for ( i = 0 ; i < foils.dimension() ; i++ )
		delete foils[i];

	foils.init(0);

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


	set_num_pnts_all_afs();

}

//==== Define  Parms ====//
void HwbBaseGeom::define_parms()
{
  int i;

  //==== Load Up Some Default Sections ====//
  HwbSect ws;
  ws.span = 4.0;
  ws.tc = 12.0;
  ws.rc = 21.0;
  ws.sweep = 65.0;
  ws.dihed_crv1 = 0.0;
  ws.dihed_crv2 = 0.0;
  ws.fillDependData();
  sects.append( ws );
  ws.span = 7.0;
  ws.tc = 3.0;
  ws.rc = 12.0;
  ws.sweep = 40.0;
  ws.dihedral = 15.0;
  ws.dihed_crv1 = 0.1f;
  ws.dihed_crv2 = 0.5f;
  ws.fillDependData();
  sects.append( ws );
  ws.span = 2.0;
  ws.tc = 2.0;
  ws.rc = 3.0;
  ws.sweep = 33.0;
  ws.dihedral = 45.0;
  ws.dihed_crv1 = 0.0;
  ws.dihed_crv2 = 0.0;
  ws.fillDependData();
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
    ta += sects[i].area;
    ts += sects[i].span;
    tc += sects[i].tc + sects[i].rc;
//	tps += sects[i].span * cos( sects[i].dihedral * DEG_2_RAD );
	tps += sects[i].span * cos( get_sum_dihedral(i) * DEG_2_RAD );
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
  double sps =  sects[currSect].span * fabs(cos( get_sum_dihedral(currSect) *  DEG_2_RAD ));
  sect_proj_span.initialize(this, UPD_MSW_SECTS, "Sect_Aspect", sps );
  sect_proj_span.set_lower_upper(0.0001, 10000000.0);

  total_area.initialize(this, UPD_MSW_TOTAL_AREA, "Total_Area", ta);
  total_area.set_lower_upper(0.0001, 1000000.0);
  total_area.set_script("wing_plan area", 0);

  total_span.initialize(this, UPD_MSW_TOTAL_SPAN, "Total_Span", ts);
  total_span.set_lower_upper(0.0001, 1000000.0);
  total_span.set_script("wing_plan span", 0);

  total_proj_span.initialize(this, UPD_MSW_TOTAL_SPAN, "Total_Proj_Span", tps);
  total_proj_span.set_lower_upper(0.0001, 10000000.0);
  total_proj_span.set_script("wing_plan projspan", 0);

  avg_chord.initialize(this, UPD_MSW_AVG_CHORD, "Avg_Chord", ac);
  avg_chord.set_lower_upper(0.0001, 10000000.0);
  avg_chord.set_script("wing_plan chord", 0);

  sweep_off.initialize(this, UPD_MW_ALL, "Sweep_Offset", 0.0);
  sweep_off.set_lower_upper(-85.0, 85.0);
  sweep_off.set_script("wing_plan sweepoff", 0);

  total_aspect.initialize(this, UPD_MSW_ALL, "Total_Aspect", ar);
  total_aspect.deactivate();
  total_aspect.set_script("wing_plan ar", 0);

  sect_aspect.initialize(this, UPD_MSW_SECTS, "Sect_Aspect", sects[currSect].ar);
  sect_aspect.set_lower_upper(0.05, 100.0);
  sect_aspect.set_script("wing_sect ar", 0);

  sect_taper.initialize(this, UPD_MSW_SECTS, "Sect_Taper", sects[currSect].tr);
  sect_taper.set_lower_upper(0.01, 5.0);
  sect_taper.set_script("wing_sect tr", 0);

  sect_area.initialize(this, UPD_MSW_SECTS, "Sect_Area", sects[currSect].area);
  sect_area.set_lower_upper(0.0001, 1000000.0);
  sect_area.set_script("wing_sect area", 0);

  sect_span.initialize(this, UPD_MSW_SECTS, "Sect_Span", sects[currSect].span);
  sect_span.set_lower_upper(0.0001, 1000000.0);
  sect_span.set_script("wing_sect span", 0);

  sect_rc.initialize(this, UPD_MSW_SECTS, "Sect_RC", sects[currSect].rc);
  sect_rc.set_lower_upper(0.0001, 1000000.0);
  sect_rc.set_script("wing_sect rc", 0);

  sect_tc.initialize(this, UPD_MSW_SECTS, "Sect_TC", sects[currSect].tc);
  sect_tc.set_lower_upper(0.0001, 1000000.0);
  sect_tc.set_script("wing_sect tc", 0);

  sect_sweep.initialize(this, UPD_MSW_SECTS, "Sect_Sweep", sects[currSect].sweep);
  sect_sweep.set_lower_upper(-85.0, 85.0);
  sect_sweep.set_script("wing_sect sweep", 0);

  sect_sweep_loc.initialize(this, UPD_MSW_SECTS, "Sect_Sweep_Loc", sects[currSect].sweepLoc);
  sect_sweep_loc.set_lower_upper(0.0, 1.0);
  sect_sweep_loc.set_script("wing_sect sweeploc", 0);

  sect_twist.initialize(this, UPD_MSW_SECTS, "Sect_Twist", sects[currSect].twist);
  sect_twist.set_lower_upper(-45.0, 45.0);
  sect_twist.set_script("wing_sect twist", 0);

  sect_twist_loc.initialize(this, UPD_MSW_SECTS, "Sect_Twist_Loc", sects[currSect].twistLoc);
  sect_twist_loc.set_lower_upper(0.0, 1.0);
  sect_twist_loc.set_script("wing_sect twistloc", 0);

  sect_dihed1.initialize(this, UPD_MSW_SECTS, "Sect_Dihedral", sects[currSect].dihedral);
//  sect_dihed1.set_lower_upper(-180.0, 180.0);
  sect_dihed1.set_lower_upper(-360.0, 360.0);
  sect_dihed1.set_script("wing_dihed dihed1", 0);

  sect_dihed2.initialize(this, UPD_MSW_SECTS, "Sect_Dihedral", sects[nextSect].dihedral);
//  sect_dihed2.set_lower_upper(-180.0, 180.0);
  sect_dihed2.set_lower_upper(-360.0, 360.0);
  sect_dihed2.set_script("wing_dihed dihed2", 0);

  sect_dihed_crv1.initialize(this, UPD_MSW_SECTS, "Sect_Dihedral_Crv1", sects[currSect].dihed_crv1);
  sect_dihed_crv1.set_lower_upper( 0.0, 1.0 );
  sect_dihed_crv1.set_script("wing_dihed attach1", 0);

  sect_dihed_crv2.initialize(this, UPD_MSW_SECTS, "Sect_Dihedral_Crv2", sects[currSect].dihed_crv2);
  sect_dihed_crv2.set_lower_upper( 0.0, 1.0 );
  sect_dihed_crv2.set_script("wing_dihed attach2", 0);

  sect_dihed_crv1_str.initialize(this, UPD_MSW_SECTS, "Sect_Dihedral_Crv1_Str", sects[currSect].dihed_crv1_str);
  sect_dihed_crv1_str.set_lower_upper( 0.0, 2.0 );
  sect_dihed_crv1_str.set_script("wing_dihed tanstr1", 0);

  sect_dihed_crv2_str.initialize(this, UPD_MSW_SECTS, "Sect_Dihedral_Crv2_Str", sects[currSect].dihed_crv2_str);
  sect_dihed_crv2_str.set_lower_upper( 0.0, 2.0 );
  sect_dihed_crv2_str.set_script("wing_dihed tanstr2", 0);

  deg_per_seg.initialize(this, UPD_MSW_SECTS, "Degrees Per Segment in Blend", 9.0);
  deg_per_seg.set_lower_upper( 1.0, 30.0 );
  deg_per_seg.set_script("wing_dihed degperseg", 1);

  max_num_segs.initialize(this, UPD_MSW_SECTS, "Max Num Segments in Blend", 9 );
  max_num_segs.set_lower_upper( 2.0, 30.0 );
  max_num_segs.set_script("wing_dihed maxsegs", 1);

  driver =  sects[currSect].driver;
  set_driver( driver );
 
}

void HwbBaseGeom::LoadLinkableParms( vector< Parm* > & parmVec )
{
	Geom::LoadLinkableParms( parmVec );
	Geom::AddLinkableParm( &total_area, parmVec, this, "Design" );
	Geom::AddLinkableParm( &total_span, parmVec, this, "Design" );
	Geom::AddLinkableParm( &total_proj_span, parmVec, this, "Design" );
	Geom::AddLinkableParm( &avg_chord, parmVec, this, "Design" );

	Geom::AddLinkableParm( &sweep_off, parmVec, this, "Design" );
	Geom::AddLinkableParm( &total_aspect, parmVec, this, "Design" );
}

void HwbBaseGeom::GetInteriorPnts( vector< vec3d > & pVec )
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

//==== Generate Multi Section Wing Component ====//
void HwbBaseGeom::generate()
{
  generate_surf();

  update_bbox();
}


vec3d HwbBaseGeom::getAttachUVPos(double u, double v)
{
	vec3d pos;

	vec3d uvpnt = mwing_surf.comp_uv_pnt(u,v);
	pos = uvpnt.transform( model_mat );

	return pos;
}

void HwbBaseGeom::acceptScaleFactor()
{
	lastScaleFactor = 1.0;
	scaleFactor.set(1.0);
}

void HwbBaseGeom::resetScaleFactor()
{
	scaleFactor.set( 1.0 );
    scale();
	lastScaleFactor = 1.0;
		
	//==== Tell Aircraft You Have Changed ====//
	airPtr->geomMod( this );


}

void HwbBaseGeom::computeCenter()
{
	//==== Set Rotation Center ====//
	center.set_x( origin()*sects[0].rc*scaleFactor() ); 
}

//==== Parm Has Changed ReGenerate Multi Section Wing Component ====//
void HwbBaseGeom::parm_changed(Parm* p)
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

//			generate();
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
//		  for ( int i = 0 ; i < foils.dimension() ; i++ )
//		  {
//			foils[i]->generate_airfoil();
//			foils[i]->load_name();
//		  }
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

		case UPD_MSW_TOTAL_SPAN:
        {
  			//==== Compute Totals ====//
			double ts = 0.0;
  			for ( i = 0 ; i < sects.dimension() ; i++ )
				ts += sects[i].span;

  			//==== Compute Totals Proj Span ====//
			double tps = 0.0;
  			for ( i = 0 ; i < sects.dimension() ; i++ )
				tps += sects[i].span * cos( get_sum_dihedral(i) * DEG_2_RAD );
//				tps += sects[i].span * cos( sects[i].dihedral * DEG_2_RAD );

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
      				sects[i].span *= fract;
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
				ta += sects[i].area;

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
      				sects[i].area *= fract;
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
				tc += sects[i].tc + sects[i].rc;

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
      				sects[i].rc *= fract;
      				sects[i].tc *= fract;
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
			if ( currSect == nextSect )
			{
				if ( p == &sect_dihed1 )
					sect_dihed2 = sect_dihed1();
				else if ( p == &sect_dihed2 )
					sect_dihed1 = sect_dihed2();
			}
 
			set_depend_parms();
			generate();
		    updateAttach(0);
       }
      break;

    }

	if ( p == &scaleFactor )	
	{
		scale();
//		generate();
		updateAttach(0);
	}

	compose_model_matrix();

	//==== Tell Aircraft You Have Changed ====//
	airPtr->geomMod( this );

}


void HwbBaseGeom::computeTotals()
{
  //==== Compute Total Span ====//
	double ts = 0.0;
	double ta = 0.0;
	double tc = 0.0;
	double tps = 0.0;
	for (int i = 0 ; i < sects.dimension() ; i++ )
	{
		ts += sects[i].span;
		ta += sects[i].area;
		tc += sects[i].tc + sects[i].rc;
		tps += sects[i].span * cos( get_sum_dihedral(i) * DEG_2_RAD );
//		tps += sects[i].span * cos( sects[i].dihedral * DEG_2_RAD );
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

	sect_proj_span = sects[currSect].span * fabs(cos( get_sum_dihedral(currSect) *  DEG_2_RAD ));
//	sect_proj_span = sects[currSect].span * fabs(cos( sects[currSect].dihedral *  DEG_2_RAD ));
	sect_span = sects[currSect].span;
	sect_area = sects[currSect].area;
	sect_aspect = sects[currSect].ar;
	sect_taper = sects[currSect].tr;
	sect_tc = sects[currSect].tc;
	sect_rc = sects[currSect].rc;
}


//==== Match Root/Tip of Adjoining Sections ====//
void HwbBaseGeom::matchWingSects()
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
    sects[i-1].tc = sects[i].rc;
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
    sects[i+1].rc = sects[i].tc;
    sects[i+1].fillDependData();
    sects[i+1].driver = odriver;
  }
}

double HwbBaseGeom::get_cbar()
{
	double area_taper = 0.0;
	for ( int i = 0 ; i < sects.dimension() ; i++ )
		area_taper += sects[i].area*sects[i].tr;

	double avg_taper = area_taper/total_area();
	double root_chord = sects[0].rc;
	double m_a_c = 2.0/3.0*root_chord*(1.0+avg_taper+avg_taper*avg_taper)/(1.0+avg_taper);

	return( m_a_c );
}

//==== Get Current Airfoil Ptr ====//
void HwbBaseGeom::set_num_pnts_all_afs()
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
Af* HwbBaseGeom::get_af_ptr()
{
	if ( rootActiveFlag )
	{
//		if(!(sects[currSect].rootAf->get_thickness())) cout << "sending null rootaf" << endl;
  		return sects[currSect].rootAf;
	}

//	if(!(sects[currSect].tipAf->get_thickness())) cout << "sending null tipaf" << endl;
	return sects[currSect].tipAf;
}

//==== Add Section ====//
void HwbBaseGeom::add_sect()
{
  HwbSect ws1 = sects[currSect];
  HwbSect ws2 = sects[currSect];

  //==== Fill Wing Section 1 ====//
  int odriver = ws1.driver;
  ws1.driver = MS_S_TC_RC;
  ws1.span *= 0.5;
  ws1.tc = 0.5*(ws1.tc + ws1.rc);
  ws1.fillDependData();
  ws1.driver = odriver;

  //==== Fill Wing Section 1 ====//
  ws2.driver = MS_S_TC_RC;
  ws2.span *= 0.5;
  ws2.rc = 0.5*(ws2.tc + ws2.rc);
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
}

//==== Add Section ====//
void HwbBaseGeom::ins_sect()
{
  HwbSect ws = sects[currSect];

  sects.insert_after_index( ws, currSect );

  Af * foil = new Af(this);		
  foil->init_script("wing_foil");
	*foil = *ws.tipAf;
  foils.append( foil );

	sects[currSect].tipAf = foil;
	sects[currSect+1].rootAf =  foil;
	
  set_curr_sect( currSect );
}

//==== Delete Section ====//
void HwbBaseGeom::del_sect()
{
  if ( sects.dimension() < 2 )
    return;

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
			parmMgrPtr->RemoveParmReferences( foils[i]->get_camber() );
			delete foils[i];
		}
	}
	foils = tmpfoils;

  if ( currSect >= sects.dimension() )
    currSect = sects.dimension()-1;

  set_curr_sect( currSect );

  parmMgrPtr->RebuildAll();


}

//==== Copy Section ====//
void HwbBaseGeom::copy_sect()
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
void HwbBaseGeom::paste_sect()
{
	Af* raf = sects[currSect].rootAf;
	Af* taf = sects[currSect].tipAf;

	sects[currSect] = copySect;

	sects[currSect].rootAf = raf;
	sects[currSect].tipAf  = taf;

	*sects[currSect].rootAf = *(copySect.rootAf);
	*sects[currSect].tipAf  = *(copySect.tipAf);

	set_curr_sect( currSect );
}

void HwbBaseGeom::scale()
{
	double current_factor = scaleFactor()*(1.0/lastScaleFactor);

    for (  int i = 0 ; i < sects.dimension() ; i++ )
    {
    	int odriver = sects[i].driver;
		int ndriver = MS_S_TC_RC;
		sects[i].driver = ndriver;
		if ( i == 0 )
			sects[i].rc *= current_factor;

		sects[i].tc *= current_factor;		
		sects[i].span *= current_factor;
		sects[i].fillDependData();
		sects[i].driver = odriver;
    }

  	//==== Compute Totals ====//
	computeTotals();
    generate();

	lastScaleFactor = scaleFactor();

}



//==== Write Multi Section Wing File ====//
void HwbBaseGeom::write(FILE* file_id)
{

}

//==== Write Multi Section Wing File ====//
void HwbBaseGeom::write(xmlNodePtr root)
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
    xmlAddDoubleNode( sec_node, "AR",       sects[i].ar );
    xmlAddDoubleNode( sec_node, "TR",       sects[i].tr );
    xmlAddDoubleNode( sec_node, "Area",     sects[i].area );
    xmlAddDoubleNode( sec_node, "Span",     sects[i].span );
    xmlAddDoubleNode( sec_node, "TC",       sects[i].tc );
    xmlAddDoubleNode( sec_node, "RC",       sects[i].rc );
    xmlAddDoubleNode( sec_node, "Sweep",    sects[i].sweep );
    xmlAddDoubleNode( sec_node, "SweepLoc", sects[i].sweepLoc );
    xmlAddDoubleNode( sec_node, "Twist",    sects[i].twist );
    xmlAddDoubleNode( sec_node, "TwistLoc", sects[i].twistLoc );
    xmlAddDoubleNode( sec_node, "Dihedral", sects[i].dihedral );
    xmlAddDoubleNode( sec_node, "Dihed_Crv1", sects[i].dihed_crv1 );
    xmlAddDoubleNode( sec_node, "Dihed_Crv2", sects[i].dihed_crv2 );
    xmlAddDoubleNode( sec_node, "Dihed_Crv1_Str", sects[i].dihed_crv1_str );
    xmlAddDoubleNode( sec_node, "Dihed_Crv2_Str", sects[i].dihed_crv2_str );

    xmlAddIntNode( sec_node, "DihedRotFlag", sects[i].dihedRotFlag );
    xmlAddIntNode( sec_node, "SmoothBlendFlag", sects[i].smoothBlendFlag );
    xmlAddIntNode( sec_node, "NumInterpXsecs", sects[i].num_interp_xsecs );
  }

  feaMeshMgrPtr->WriteFeaStructData( this, root );

}
//==== Write Multi Section Wing File ====//
void HwbBaseGeom::read(xmlNodePtr root)
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
    for ( i = 0 ; i < foils.dimension() ; i++ )
    {			
      delete foils[i];			
	}

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

    dyn_array<HwbSect> tmpsects;

    for ( i = 0 ; i < num_sec ; i++ )
    {
      HwbSect ws;
      xmlNodePtr sec_node = xmlGetNode( sec_list_node, "Section", i );

      ws.driver   = xmlFindInt( sec_node, "Driver", ws.driver );
      ws.ar       = xmlFindDouble( sec_node, "AR", ws.ar );
      ws.tr       = xmlFindDouble( sec_node, "TR", ws.tr );
      ws.area     = xmlFindDouble( sec_node, "Area", ws.area );
      ws.span     = xmlFindDouble( sec_node, "Span", ws.span );
      ws.tc       = xmlFindDouble( sec_node, "TC", ws.tc );
      ws.rc       = xmlFindDouble( sec_node, "RC", ws.rc );
      ws.sweep    = xmlFindDouble( sec_node, "Sweep", ws.sweep );
      ws.sweepLoc = xmlFindDouble( sec_node, "SweepLoc", ws.sweepLoc );
      ws.twist    = xmlFindDouble( sec_node, "Twist", ws.twist );
      ws.twistLoc = xmlFindDouble( sec_node, "TwistLoc", ws.twistLoc );
      ws.dihedral = xmlFindDouble( sec_node, "Dihedral", ws.dihedral );
      ws.dihed_crv1 = xmlFindDouble( sec_node, "Dihed_Crv1", ws.dihed_crv1 );
      ws.dihed_crv2 = xmlFindDouble( sec_node, "Dihed_Crv2", ws.dihed_crv2 );
      ws.dihed_crv1_str = xmlFindDouble( sec_node, "Dihed_Crv1_Str", ws.dihed_crv1_str );
      ws.dihed_crv2_str = xmlFindDouble( sec_node, "Dihed_Crv2_Str", ws.dihed_crv2_str );

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

}

//==== Read Multi Section Wing File ====//
void HwbBaseGeom::read(FILE* file_id)
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
  for ( i = 0 ; i < foils.dimension() ; i++ )
  {			
    delete foils[i];			
  }

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
  dyn_array<HwbSect> tmpsects;
	for ( i = 0 ; i < nsects ; i++ )
  {
    HwbSect ws;
    fscanf(file_id, "%d",&(ws.driver) );				fgets(buff, 80, file_id);
    fscanf(file_id, "%lf",&(ws.ar) );						fgets(buff, 80, file_id);
    fscanf(file_id, "%lf",&(ws.tr) );						fgets(buff, 80, file_id);
    fscanf(file_id, "%lf",&(ws.area) );					fgets(buff, 80, file_id);
    fscanf(file_id, "%lf",&(ws.span) );					fgets(buff, 80, file_id);
    fscanf(file_id, "%lf",&(ws.tc) );						fgets(buff, 80, file_id);
    fscanf(file_id, "%lf",&(ws.rc) );						fgets(buff, 80, file_id);
    fscanf(file_id, "%lf",&(ws.sweep) );					fgets(buff, 80, file_id);
    fscanf(file_id, "%lf",&(ws.sweepLoc) );			fgets(buff, 80, file_id);
    fscanf(file_id, "%lf",&(ws.twist) );					fgets(buff, 80, file_id);
    fscanf(file_id, "%lf",&(ws.twistLoc) );			fgets(buff, 80, file_id);
    fscanf(file_id, "%lf",&(ws.dihedral) );			fgets(buff, 80, file_id);
    fscanf(file_id, "%lf",&(ws.dihed_crv1) );		fgets(buff, 80, file_id);
    fscanf(file_id, "%lf",&(ws.dihed_crv2) );		fgets(buff, 80, file_id);

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
//void HwbBaseGeom::write_rhino_file(ON_BinaryFile* archive, ON_3dmObjectAttributes* attributes)
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
//	dyn_array< HwbWingPnt > wingPnts;
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
void HwbBaseGeom::write_rhino_file(ON_BinaryFile* archive, ON_3dmObjectAttributes* attributes)
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


	dyn_array< HwbWingPnt > wingPnts;
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






bool HwbBaseGeom::merged_center_foil()
{
	if ( sym_code != XZ_SYM )
		return false;

	if ( fabs( yLoc() ) > 0.0000001 )
		return false;

	return true;
}

int HwbBaseGeom::get_num_bezier_comps()
{
	if ( sym_code == NO_SYM || merged_center_foil() )
		return 1;
	else 
		return 2;
}



void HwbBaseGeom::write_bezier_file( int id, FILE* file_id )
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

	dyn_array< HwbWingPnt > wingPnts;
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
vector< TMesh* > HwbBaseGeom:: createTMeshVec()
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
void HwbBaseGeom::dump_xsec_file(int geom_no, FILE* dump_file)
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
void HwbBaseGeom::set_depend_parms()
{
  sects[currSect].span = sect_span();
  sects[currSect].area = sect_area();
  sects[currSect].ar   = sect_aspect();
  sects[currSect].tr   = sect_taper();
  sects[currSect].tc   = sect_tc();
  sects[currSect].rc   = sect_rc();

  sects[currSect].sweep          = sect_sweep();
  sects[currSect].dihedral       = sect_dihed1();
  sects[nextSect].dihedral       = sect_dihed2();
  sects[currSect].dihed_crv1     = sect_dihed_crv1();
  sects[currSect].dihed_crv2	 = sect_dihed_crv2();
  sects[currSect].dihed_crv1_str = sect_dihed_crv1_str();
  sects[currSect].dihed_crv2_str = sect_dihed_crv2_str();
  sects[currSect].twist          = sect_twist();
  sects[currSect].twistLoc       = sect_twist_loc();


  //==== Check If Sweep Loc Changed ====//
  if ( fabs( sect_sweep_loc() - sects[currSect].sweepLoc) > 0.00001 )
  {
    double tan_swp = sects[currSect].tan_sweep_at( sect_sweep_loc(), getSymCode() );
    sects[currSect].sweep = atan( tan_swp )*RAD_2_DEG;
    sect_sweep = sects[currSect].sweep;

    sects[currSect].sweepLoc   = sect_sweep_loc();
  }

  sects[currSect].fillDependData();
  matchWingSects();

  switch (driver)
    {
      case AR_TR_A:
        sect_span = sqrt( sect_aspect()*sect_area() );
        sect_rc   = (2.0*sect_span()) / ( sect_aspect()*(1.0+sect_taper()) );
        sect_tc   = sect_taper()*sect_rc();
      break;

      case AR_TR_S:
        sect_area = (sect_span() * sect_span()) / sect_aspect();
        sect_rc   = (2.0*sect_span()) / ( sect_aspect()*(1.0+sect_taper()) );
        sect_tc   = sect_taper()*sect_rc();
      break;

      case AR_TR_TC:
        sect_rc   = sect_tc()/sect_taper();
        sect_span = 0.5*sect_aspect()*sect_rc()*(1.0+sect_taper());
        sect_area = (sect_span() * sect_span()) / sect_aspect();
      break;

      case AR_TR_RC:
        sect_tc   = sect_taper()*sect_rc();
        sect_span = 0.5*sect_aspect()*sect_rc()*(1.0+sect_taper());
        sect_area = (sect_span() * sect_span()) / sect_aspect();
      break;

      case S_TC_RC:
        sect_taper     = sect_tc()/sect_rc();
        sect_aspect    = 2.0*sect_span()/( sect_rc()*(1.0+sect_taper()) );
        sect_area      = (sect_span() * sect_span()) / sect_aspect();
     break;

      case A_TC_RC:
        sect_taper     = sect_tc()/sect_rc();
        sect_aspect    = 2.0*sect_span()/( sect_rc()*(1.0+sect_taper()) );
        sect_span      = sqrt( sect_aspect()*sect_area() );
      break;

      case TR_S_A:
        sect_aspect = (sect_span() * sect_span()) / sect_area();
        sect_rc     = (2.0*sect_span()) / ( sect_aspect()*(1.0+sect_taper()) );
        sect_tc     = sect_taper()*sect_rc();
      break;
    }

  //==== Compute Total Span ====//
  double ts = 0.0;
  double ta = 0.0;
  double tc = 0.0;
  double tps = 0.0;
  for (int i = 0 ; i < sects.dimension() ; i++ )
  {
		ts += sects[i].span;
		ta += sects[i].area;
		tc += sects[i].tc + sects[i].rc;
//		tps += sects[i].span * cos( sects[i].dihedral * DEG_2_RAD );
		tps += sects[i].span * cos( get_sum_dihedral(i) * DEG_2_RAD );
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

	sect_proj_span = sects[currSect].span * fabs(cos( get_sum_dihedral(currSect) *  DEG_2_RAD ));
//	sect_proj_span = sects[currSect].span * fabs(cos( sects[currSect].dihedral *  DEG_2_RAD ));


}

//==== Generate Cross Sections =====//
void HwbBaseGeom::regenerate()
{
}

//==== Set Dihed Rot Flag ====//
void HwbBaseGeom::set_dihed_rot_flag( int rf )
{
  sects[currSect].dihedRotFlag = rf;
  generate();
}

//==== Set Smooth Flag Flag ====//
void HwbBaseGeom::set_smooth_blend_flag( int sbf )
{
  sects[currSect].smoothBlendFlag = sbf;
  generate();
}

//==== Set Relative Twist Flag ====//
void HwbBaseGeom::set_rel_twist_flag(bool flag )
{
  rel_twist_flag = flag;
  generate();
}
//==== Set Relative Dihedral Flag ====//
void HwbBaseGeom::set_rel_dihedral_flag(bool flag )
{
  rel_dihedral_flag = flag;
  generate();
}
//==== Get Sum Dihedral ====//
double HwbBaseGeom::get_sum_dihedral( int sect_id )
{
	if ( sect_id < 0 || sect_id >= sects.dimension() )
		return 0.0;

	if ( rel_dihedral_flag == false )
		return sects[sect_id].dihedral;

	double sum_dihedral = 0.0;
	for ( int i = 0 ; i <= sect_id ; i++ )
		sum_dihedral += sects[i].dihedral;

	return sum_dihedral;
}

//==== Set Round End Cap Flag ====//
void HwbBaseGeom::set_round_end_cap_flag(bool flag )
{
  round_end_cap_flag = flag;
  generate();
}

//==== Set Num Interp Curr Sect ====//
void HwbBaseGeom::set_num_interp( int n )
{
	sects[currSect].num_interp_xsecs = n;
    generate();
}

//==== Inc Number Of Interp Cross Sections - All Sects ====//
void HwbBaseGeom::inc_all_interp()
{
	for ( int i = 0 ; i < sects.dimension() ; i++ )
	{
		sects[i].num_interp_xsecs++;
	}

	generate();
}

//==== Dec Number Of Interp Cross Sections - All Sects ====//
void HwbBaseGeom::dec_all_interp()
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
void HwbBaseGeom::set_curr_joint( int cj )
{
	if ( cj > 0 && cj == sects.dimension()-1 )
		set_curr_sect(cj-1);
	else
		set_curr_sect(cj);
}

//==== Get Joint Index ====//
int HwbBaseGeom::get_curr_joint()
{
	if ( currSect > 0 && currSect == sects.dimension()-1 )
		return currSect-1;

	return currSect;
}

//==== Set Foil Index ====//
void HwbBaseGeom::set_curr_foil( int cf )
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
int HwbBaseGeom::get_curr_foil()
{
	if ( !get_root_active() && currSect == sects.dimension()-1 )
		return currSect+1;
		
	return currSect;
}

//==== Sect Section Index ====//
void HwbBaseGeom::set_curr_sect(int cs)
{
  if ( cs >= 0 && cs < sects.dimension() )
    currSect = cs;

  if ( currSect < sects.dimension()-1 )
	  nextSect = currSect+1;
  else
	  nextSect = currSect;

//  sect_proj_span = sects[currSect].span * fabs(cos( sects[currSect].dihedral *  DEG_2_RAD ));
  sect_proj_span = sects[currSect].span * fabs(cos( get_sum_dihedral(currSect) *  DEG_2_RAD ));


  sect_span = sects[currSect].span;
  sect_area = sects[currSect].area;
  sect_aspect = sects[currSect].ar;
  sect_taper = sects[currSect].tr;
  sect_tc = sects[currSect].tc;
  sect_rc = sects[currSect].rc;
  sect_sweep = sects[currSect].sweep;
  sect_sweep_loc = sects[currSect].sweepLoc;
  sect_twist = sects[currSect].twist;
  sect_twist_loc = sects[currSect].twistLoc;
  sect_dihed1 = sects[currSect].dihedral;
  sect_dihed2 = sects[nextSect].dihedral;
  sect_dihed_crv1 = sects[currSect].dihed_crv1;
  sect_dihed_crv2 = sects[currSect].dihed_crv2;
  sect_dihed_crv1_str = sects[currSect].dihed_crv1_str;
  sect_dihed_crv2_str = sects[currSect].dihed_crv2_str;

  driver = sects[currSect].driver;

  set_driver( driver );

  airPtr->geomMod( this );


}

//==== Driver Group Has Changed ====//
void HwbBaseGeom::set_driver(int driver_in)
{
  int valid_flag = TRUE;

  switch (driver_in)
    {
      case AR_TR_A:
        sect_aspect.activate();  sect_taper.activate();   sect_area.activate();
        sect_span.deactivate();  sect_rc.deactivate();    sect_tc.deactivate();
      break;

      case AR_TR_S:
        sect_aspect.activate();  sect_taper.activate();         sect_span.activate();
        sect_area.deactivate();  sect_rc.deactivate();          sect_tc.deactivate();
      break;

      case AR_TR_TC:
        sect_aspect.activate();  sect_taper.activate();         sect_tc.activate();
        sect_area.deactivate();  sect_rc.deactivate();          sect_span.deactivate();
      break;

      case AR_TR_RC:
        sect_aspect.activate();  sect_taper.activate();         sect_rc.activate();
        sect_area.deactivate();  sect_tc.deactivate();          sect_span.deactivate();
      break;

      case S_TC_RC:
        sect_span.activate();    sect_tc.activate();         sect_rc.activate();
        sect_taper.deactivate(); sect_aspect.deactivate();   sect_area.deactivate();
     break;

      case A_TC_RC:
        sect_area.activate();    sect_tc.activate();         sect_rc.activate();
        sect_taper.deactivate(); sect_aspect.deactivate();   sect_span.deactivate();
      break;

      case TR_S_A:
        sect_taper.activate();    sect_span.activate();           sect_area.activate();
        sect_aspect.deactivate(); sect_rc.deactivate();           sect_tc.deactivate();
      break;

      default:
        valid_flag = FALSE;
      break;
    }
  if (valid_flag)
  {
    driver = driver_in;
    sects[currSect].driver = driver;
	}
}

//==== Generate Cross Sections =====//
void HwbBaseGeom::generate_flap_slat_surfs()
{
  //jrg check if used/needed
}

//==== Update Bounding Box =====//
void HwbBaseGeom::update_bbox()
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
void HwbBaseGeom::draw()
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
void HwbBaseGeom::drawAlpha()
{
	Geom::drawAlpha();
}


//==== Update Bounding Box =====//
void HwbBaseGeom::draw_sect_box()
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
void HwbBaseGeom::draw_refl_sect_box()
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
void HwbBaseGeom::draw_bbox( bbox & box )
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
void HwbBaseGeom::load_normals()
{
  mwing_surf.load_sharp_normals();
}

//==== Draw Hidden Surface====//
void HwbBaseGeom::load_hidden_surf()
{
  mwing_surf.load_hidden_surf();
}


void HwbBaseGeom::loadWingPnts( dyn_array< HwbWingPnt > & wingPnts )
{
	int i, j;
	HwbWingPnt wp;
	dyn_array< HwbWingPnt > tmpPnts;

	//==== First Point ====//
	wp.x = wp.y = wp.z = 0.0;
	wp.sectID = 0;
	wp.sectFract = 0.0;
	wp.chord = sects[0].rc;
	wp.twist = 0.0;
	wp.twistLoc = sects[0].twistLoc;
	wp.blendFlag = 0;

	if ( sects[0].dihedRotFlag )
		wp.dihedRot = -sects[0].dihedral;
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
	double rad = sects[i].span;
//    if ( get_sym_code() == XZ_SYM )  rad *= 0.5;   // Check Sym Code
	
	double ty = rad*cos(get_sum_dihedral(i)*DEG_2_RAD);
//	double ty = rad*cos(sects[i].dihedral*DEG_2_RAD);
	double tz = rad*sin(get_sum_dihedral(i)*DEG_2_RAD);	
//	double tz = rad*sin(sects[i].dihedral*DEG_2_RAD);	

    sects[i].sweep += sweep_off();
    double tan_le  = sects[i].tan_sweep_at( 0.0, getSymCode() );
	sects[i].sweep -= sweep_off();

    double toff    = tan_le*rad;                    // Tip X Offset

    total_dihed_offset += tz;
    total_span += ty;
    total_sweep_offset += toff;
	
	wp.x = total_sweep_offset;
	wp.y = total_span;
	wp.z = total_dihed_offset;

	wp.sectID = i;
	wp.sectFract = 1.0;
	wp.chord = sects[i].tc;

	total_twist += sects[i].twist;
	wp.twist = sects[i].twist;
	if ( rel_twist_flag )
		wp.twist = total_twist;

	wp.twistLoc = sects[i].twistLoc;
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
		
		if ( sects[sid].dihed_crv1 > 0.001 && sects[sid].dihed_crv2 > 0.001 )
		{
			double del_dihed = sects[sid].dihedral - sects[sid+1].dihedral;

			if ( fabs(del_dihed) > 0.001 )
			{
				//==== Dihedral Points ====//
				vec2d p0(tmpPnts[i-1].y, tmpPnts[i-1].z);
				vec2d p1(tmpPnts[i].y,   tmpPnts[i].z );
				vec2d p2(tmpPnts[i+1].y, tmpPnts[i+1].z );
		
				//==== Find Endpoints of Curve ====//
				double blendOff1 = sects[sid].dihed_crv1;
				double blendOff2 = sects[sid].dihed_crv2;
				double blendStr1 = sects[sid].dihed_crv1_str;
				double blendStr2 = sects[sid].dihed_crv2_str;
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

void HwbBaseGeom::generate_surf()
{
	int i, j;
	int num_pnts  = numPnts.iget();

	//==== Load Up Original Plan Points ====//
	dyn_array< HwbWingPnt > wingPnts;
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
    		base_surf.rotate_xsec_x( ixcnt, wingPnts[i].dihedRot );

    		base_surf.offset_xsec_x( ixcnt, -wingPnts[i].twistLoc * wingPnts[i].chord );
    		base_surf.rotate_xsec_y( ixcnt, wingPnts[i].twist);
    		base_surf.offset_xsec_x( ixcnt, wingPnts[i].twistLoc * wingPnts[i].chord);
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
void HwbBaseGeom::write_aero_file(FILE* aero_file, int aero_id_num)
{
  //jrg check if used/needed
}

//==== Return Number Of Felisa Surface Written Out ====//
int HwbBaseGeom::get_num_felisa_comps()
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
int HwbBaseGeom::get_num_felisa_wings()
{
  vec3d v = getTotalTranVec();

  if ( v.y() == 0.0 && sym_code == NO_SYM )
    return(0);
  else
    return(1);
}

//==== Write Out Felisa Surfaces To File ====//
int HwbBaseGeom::write_felisa_file(int geom_no, FILE* dump_file)
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
void HwbBaseGeom::write_bco_info(FILE* file_ptr, int& surf_cnt, int& wing_cnt)
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
void HwbBaseGeom::write_felisa_line_sources(FILE* dump_file)
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

int HwbBaseGeom::get_num_felisa_line_sources()
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
void HwbBaseGeom::write_felisa_tri_sources(FILE* dump_file)
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

int HwbBaseGeom::get_num_felisa_tri_sources()
{
  int nxsecs = mwing_surf.get_num_xsecs();
  return ( 2*(nxsecs - 3) );	

}

double HwbBaseGeom::getRefArea()
{
	if ( autoRefAreaFlag )
	{
		refArea = get_total_area()->get();
	}
	return refArea;
}
	
double HwbBaseGeom::getRefSpan()
{
	if ( autoRefSpanFlag )
	{
		refSpan = get_total_span()->get();
	}
	return refSpan;
}

double HwbBaseGeom::getRefCbar()
{
	if ( autoRefCbarFlag )
		computeCbarAC();

	return refCbar;
}

vec3d HwbBaseGeom::getAeroCenter()
{
	if ( autoAeroCenterFlag )
		computeCbarAC();

	return aeroCenter;
}

void HwbBaseGeom::computeCbarAC()
{

	dyn_array< HwbWingPnt > wingPnts;
	loadWingPnts( wingPnts );

	double area = 0.0;
	double mac  = 0.0;
	double xmac = 0.0;
	double ymac = 0.0;
	double zmac = 0.0;

	//==== Area Weighted Avg of Each Section ====//
	for ( int i = 1 ; i < wingPnts.dimension() ; i++ )
	{
		HwbWingPnt c1 = wingPnts[i-1];
		HwbWingPnt c2 = wingPnts[i];
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

void HwbBaseGeom::AddDefaultSources(double base_len)
{
	//==== Root Tip Chords ====//
	LineSource* ls = new LineSource();
	ls->SetName("Root_Chord");
	ls->SetLen( 0.1 );
	ls->SetLen2( 0.1 );
	ls->SetRad( 1.0 );
	ls->SetRad2( 1.0 );
	ls->SetUWLoc1( vec2d(0,0) );
	ls->SetUWLoc2( vec2d(0,0.5) );
	sourceVec.push_back( ls );

	ls = new LineSource();
	ls->SetName("Tip_Chord");
	ls->SetLen( 0.1 );
	ls->SetLen2( 0.1 );
	ls->SetRad( 1.0 );
	ls->SetRad2( 1.0 );
	ls->SetUWLoc1( vec2d(1,0) );
	ls->SetUWLoc2( vec2d(1,0.5) );
	sourceVec.push_back( ls );

	//==== Find U Vals For Joints ====//
	dyn_array< HwbWingPnt > wingPnts;
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
		ls->SetLen( 0.1 );
		ls->SetLen2( 0.1 );
		ls->SetRad( 1.0 );
		ls->SetRad2( 1.0 );
		ls->SetUWLoc1( vec2d(u0,0.5) );
		ls->SetUWLoc2( vec2d(u1,0.5) );
		sourceVec.push_back( ls );

		ls = new LineSource();
		ls->SetName("Trailing Edge");
		ls->SetLen( 0.1 );
		ls->SetLen2( 0.1 );
		ls->SetRad( 1.0 );
		ls->SetRad2( 1.0 );
		ls->SetUWLoc1( vec2d(u0,0.0) );
		ls->SetUWLoc2( vec2d(u1,0.0) );
		sourceVec.push_back( ls );
	}
}

void HwbBaseGeom::LoadDragFactors( DragFactors& drag_factors )
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
		double rc = sects[i].rc;
		double tc = sects[i].tc;
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

		double area = sects[i].area;
		double sweep = sects[i].sweep;

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

//==== Constructor =====//
HwbSect::HwbSect()
{
  //==== Load Default Data Vals ====//
  driver = MS_S_TC_RC;
  span = 30.0;
  tc = 10.0;		// Tip Chord
  rc = 20.0;		// Root Chord
  sweep = 10.0;
  sweepLoc = 0.0;	// Sweep Location
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
}

 

//==== Fill Wing Sect Dependant Data ====//
void HwbSect::fillDependData()
{
  switch (driver)
    {
      case MS_AR_TR_A:
        span = sqrt( ar*area );
        rc   = (2.0*span) / ( ar*(1.0+tr) );
        tc   = tr*rc;
      break;

      case MS_AR_TR_S:
        area = (span * span) / ar;
        rc   = (2.0*span) / ( ar*(1.0+tr) );
        tc   = tr*rc;
      break;

      case MS_AR_TR_TC:
        rc   = tc/tr;
        span = 0.5*ar*rc*(1.0+tr);
        area = (span * span) / ar;
      break;

      case MS_AR_TR_RC:
        tc   = tr*rc;
        span = 0.5*ar*rc*(1.0+tr);
        area = (span * span) / ar;
      break;

      case MS_S_TC_RC:
        tr   = tc/rc;
        ar   = 2.0*span/( rc*(1.0+tr) );
        area = (span * span) / ar;
      break;

      case MS_A_TC_RC:
        tr    = tc/rc;
        ar    = 2.0*span/( rc*(1.0+tr) );
        span  = sqrt( ar*area );
      break;

      case MS_TR_S_A:
        ar = (span * span) / area;
        rc = (2.0*span) / ( ar*(1.0+tr) );
        tc = tr*rc;
      break;

      case MS_AR_A_RC:
		span = sqrt( ar * area);
        tr = (2.0*span)/(ar*rc) - 1.0;
        tc = tr*rc;
      break;

    }
}

//==== Calculate Tan Sweep at Specified Chord Location =====//
double HwbSect::tan_sweep_at( double loc, int sym_code)
{
  double tan_sweep = tan(sweep*DEG_2_RAD);

  double tan_sweep_at = tan_sweep - (2.0/ar)*
            ( (loc-sweepLoc) * ((1.0-tr)/(1.0+tr)) );

  return ( tan_sweep_at );

}





//==========================================================//
//==========================================================//
//==========================================================//


Hwb_geom::Hwb_geom(Aircraft* aptr) : HwbBaseGeom(aptr)
{
	for(int i=0; i < sects.dimension(); i++)
	{
		currSect = i;
		get_sect_dihed_crv1()->set(0.0f);
  		get_sect_dihed_crv2()->set(0.0f);
	}

	currSect = 0;
	nextSect = 0;
	highlightType = MSW_HIGHLIGHT_NONE;	

	type = HWB_GEOM_TYPE;
	type_str = Stringc("wing");

	char name[255];
	sprintf( name, "Hwb_%d", geomCnt ); 
	geomCnt++;
	name_str = Stringc(name);
	
	//==== Define Some Decent Default Parms ====//
	define_parms();
	setSymCode(XZ_SYM);
	generate();
}

Hwb_geom::~Hwb_geom()
{
	int i = 0;

	int size = sweep_percent_edge_lengths.dimension();
	for( i = 0; i < size; i++)
	{	
		if(sweep_percent_edge_lengths[i])
			delete sweep_percent_edge_lengths[i];			
	}

	size = tesweep_percent_edge_lengths.dimension();
	for( i = 0; i < size; i++)
	{	
		if(tesweep_percent_edge_lengths[i])
			delete tesweep_percent_edge_lengths[i];			
	}

	size = dihedral_percent_edge_lengths.dimension();
	for( i = 0; i < size; i++)
	{
		if(dihedral_percent_edge_lengths[i])
			delete dihedral_percent_edge_lengths[i];			
	}

	size = sweep_degrees_perseg.dimension();
	for( i = 0; i < size; i++)
	{
		if(sweep_degrees_perseg[i])
			delete sweep_degrees_perseg[i];
	}

	size = tesweep_degrees_perseg.dimension();
	for( i = 0; i < size; i++)
	{	
		if(tesweep_degrees_perseg[i])
			delete tesweep_degrees_perseg[i];			
	}

	size = dihedral_degrees_perseg.dimension();
	for( i = 0; i < size; i++)
	{
		if(dihedral_degrees_perseg[i])
			delete dihedral_degrees_perseg[i];			
	}

	if(filleted_tip_chord)
		delete filleted_tip_chord;
	
	if(filleted_root_chord)
		delete filleted_root_chord;

	size = sub_sect_foils.dimension();
	for(int i = 0; i < size; i++)
	{
		if(sub_sect_foils[i])
		{
			delete sub_sect_foils[i];
			sub_sect_foils[i] = NULL;
		}
	}
}

void Hwb_geom::copy( Geom* fromGeom )
{
	int i = 0;

	HwbBaseGeom::copy( fromGeom );

	if ( fromGeom->getType() != HWB_GEOM_TYPE )
		return;

	Hwb_geom* g = (Hwb_geom*)fromGeom;

	highlightType = g->highlightType;

	sweep_percent_edge_length->set(g->sweep_percent_edge_length->get());
	tesweep_percent_edge_length->set(g->tesweep_percent_edge_length->get());
	dihedral_percent_edge_length->set(g->dihedral_percent_edge_length->get());
	sweep_degperseg->set(g->sweep_degperseg->get());
	tesweep_degperseg->set(g->tesweep_degperseg->get());
	dihedral_degperseg->set(g->sweep_degperseg->get());

	filleted_tip_chord->set(g->filleted_tip_chord->get());
	filleted_root_chord->set(g->filleted_root_chord->get());

	chord_lengths.init( g->chord_lengths.dimension() );

	for( i = 0; i < chord_lengths.dimension(); i++)
		chord_lengths[i] = g->chord_lengths[i];

	for( i = 0; i < g->sweep_percent_edge_lengths.dimension(); i++)
	{
		if(i > sweep_percent_edge_lengths.dimension()-1)
		{
			Parm* new_parm = new Parm();
			*(new_parm) = *(g->sweep_percent_edge_lengths[i]);
			sweep_percent_edge_lengths.append(new_parm);
		}
		else if( (i < sweep_percent_edge_lengths.dimension()-1) && (i > g->sweep_percent_edge_lengths.dimension()-1) )
		{	
			if(sweep_percent_edge_lengths[i])
				delete sweep_percent_edge_lengths[i];

			sweep_percent_edge_lengths.del_index(i);			
		}
		else
			*(sweep_percent_edge_lengths[i]) = *(g->sweep_percent_edge_lengths[i]);
	}

	for( i = 0; i < g->tesweep_percent_edge_lengths.dimension(); i++)
	{
		if(i > tesweep_percent_edge_lengths.dimension()-1)
		{
			Parm* new_parm = new Parm();
			*(new_parm) = *(g->tesweep_percent_edge_lengths[i]);
			tesweep_percent_edge_lengths.append(new_parm);
		}
		else if( (i < tesweep_percent_edge_lengths.dimension()-1) && (i > g->tesweep_percent_edge_lengths.dimension()-1) )
		{			
			if(tesweep_percent_edge_lengths[i])
				delete tesweep_percent_edge_lengths[i];
			
			tesweep_percent_edge_lengths.del_index(i);			
		}
		else
			*(tesweep_percent_edge_lengths[i]) = *(g->tesweep_percent_edge_lengths[i]);
	}

	for( i = 0; i < g->dihedral_percent_edge_lengths.dimension(); i++)
	{
		if(i > dihedral_percent_edge_lengths.dimension()-1)
		{
			Parm* new_parm = new Parm();
			*(new_parm) = *(g->dihedral_percent_edge_lengths[i]);
			dihedral_percent_edge_lengths.append(new_parm);
		}
		else if( (i < dihedral_percent_edge_lengths.dimension()-1) && (i > g->dihedral_percent_edge_lengths.dimension()-1) )
		{			
			if(dihedral_percent_edge_lengths[i])
				delete dihedral_percent_edge_lengths[i];
			
			dihedral_percent_edge_lengths.del_index(i);			
		}
		else
			*(dihedral_percent_edge_lengths[i]) = *(g->dihedral_percent_edge_lengths[i]);
	}

	for( i = 0; i < g->sweep_degrees_perseg.dimension(); i++)
	{
		if(i > sweep_degrees_perseg.dimension()-1)
		{
			Parm* new_parm = new Parm();
			*(new_parm) = *(g->sweep_degrees_perseg[i]);
			sweep_degrees_perseg.append(new_parm);
		}
		else if( (i < sweep_degrees_perseg.dimension()-1) && (i > g->sweep_degrees_perseg.dimension()-1) )
		{			
			if(sweep_degrees_perseg[i])
				delete sweep_degrees_perseg[i];
			
			sweep_degrees_perseg.del_index(i);			
		}
		else
			*(sweep_degrees_perseg[i]) = *(g->sweep_degrees_perseg[i]);
	}

	for( i = 0; i < g->tesweep_degrees_perseg.dimension(); i++)
	{
		if(i > tesweep_degrees_perseg.dimension()-1)
		{
			Parm* new_parm = new Parm();
			*(new_parm) = *(g->tesweep_degrees_perseg[i]);
			tesweep_degrees_perseg.append(new_parm);
		}
		else if( (i < tesweep_degrees_perseg.dimension()-1) && (i > g->tesweep_degrees_perseg.dimension()-1) )
		{			
			if(tesweep_degrees_perseg[i])
				delete tesweep_degrees_perseg[i];
			
			tesweep_degrees_perseg.del_index(i);			
		}
		else
			*(tesweep_degrees_perseg[i]) = *(g->tesweep_degrees_perseg[i]);
	}

	for( i = 0; i < g->dihedral_degrees_perseg.dimension(); i++)
	{
		if(i > dihedral_degrees_perseg.dimension()-1)
		{
			Parm* new_parm = new Parm();
			*(new_parm) = *(g->dihedral_degrees_perseg[i]);
			dihedral_degrees_perseg.append(new_parm);
		}
		else if( (i < dihedral_degrees_perseg.dimension()-1) && (i > g->dihedral_degrees_perseg.dimension()-1) )
		{			
			if(dihedral_degrees_perseg[i])
				delete dihedral_degrees_perseg[i];
			
			dihedral_degrees_perseg.del_index(i);			
		}
		else
			*(dihedral_degrees_perseg[i]) = *(g->dihedral_degrees_perseg[i]);
	}

//  There is no need to copy sub_sect_foils since it is generated by Hwb when generate() is called

	generate();
}

//==== Write Multi Section Wing File ====//
void Hwb_geom::write(xmlNodePtr root)
{
  int i;
  xmlAddStringNode( root, "Type", "Hwb");

  //==== Write General Parms ====//
  xmlNodePtr gen_node = xmlNewChild( root, NULL, (const xmlChar *)"General_Parms", NULL );
  write_general_parms( gen_node );

  //==== Write Fuse Parms ====//
  xmlNodePtr mswing_node = xmlNewChild( root, NULL, (const xmlChar *)"Hwb_Parms", NULL );

  xmlAddDoubleNode( mswing_node, "Total_Area", get_total_area()->get() );
  xmlAddDoubleNode( mswing_node, "Total_Span", get_total_span()->get() );
  xmlAddDoubleNode( mswing_node, "Total_Proj_Span", get_total_proj_span()->get() );
  xmlAddDoubleNode( mswing_node, "Avg_Chord", get_avg_chord()->get() );
  xmlAddDoubleNode( mswing_node, "Sweep_Off", get_sweep_off()->get() );

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

  xmlNodePtr sect_sizes_list_node = xmlNewChild( root, NULL, (const xmlChar *)"Section_Sizes_List", NULL );
  xmlAddIntNode( sect_sizes_list_node, "List_Size", section_sizes.dimension() );

  for( i = 0; i < section_sizes.dimension(); i++ )
  {
	char descrip[255];
	sprintf( descrip, "Section_Size%d", i );
	xmlAddIntNode( sect_sizes_list_node, descrip, section_sizes[i] );
  } 

  xmlNodePtr sweep_percent_lengths_list_node = xmlNewChild( root, NULL, (const xmlChar *)"Sweep_Percent_Lengths_List", NULL );
  xmlAddIntNode( sweep_percent_lengths_list_node, "List_Size", sweep_percent_edge_lengths.dimension() );

  for( i = 0; i < sweep_percent_edge_lengths.dimension(); i++ )
  {
	char descrip[255];
	sprintf( descrip, "Percent_Length%d", i );
	xmlAddDoubleNode( sweep_percent_lengths_list_node, descrip, sweep_percent_edge_lengths[i]->get() ); 
  }

  xmlNodePtr tesweep_percent_lengths_list_node = xmlNewChild( root, NULL, (const xmlChar *)"TESweep_Percent_Lengths_List", NULL );
  xmlAddIntNode( tesweep_percent_lengths_list_node, "List_Size", tesweep_percent_edge_lengths.dimension() );

  for( i = 0; i < tesweep_percent_edge_lengths.dimension(); i++ )
  {
	char descrip[255];
	sprintf( descrip, "Percent_Length%d", i );
	xmlAddDoubleNode( tesweep_percent_lengths_list_node, descrip, tesweep_percent_edge_lengths[i]->get() );
  }

  xmlNodePtr dihedral_percent_lengths_list_node = xmlNewChild( root, NULL, (const xmlChar *)"Dihedral_Percent_Lengths_List", NULL );
  xmlAddIntNode( dihedral_percent_lengths_list_node, "List_Size", dihedral_percent_edge_lengths.dimension() );

  for( i = 0; i < dihedral_percent_edge_lengths.dimension(); i++ )
  {
	char descrip[255];
	sprintf( descrip, "Percent_Length%d", i );
	xmlAddDoubleNode( dihedral_percent_lengths_list_node, descrip, dihedral_percent_edge_lengths[i]->get() );
  }

  xmlNodePtr sweep_degperseg_list_node = xmlNewChild( root, NULL, (const xmlChar *)"Sweep_DegPerSeg_List", NULL );
  xmlAddIntNode( sweep_degperseg_list_node, "List_Size", sweep_degrees_perseg.dimension() );

  for( i = 0; i < sweep_degrees_perseg.dimension(); i++ )
  {
	char descrip[255];
	sprintf( descrip, "DegPerSeg%d", i );
	xmlAddDoubleNode( sweep_degperseg_list_node, descrip, sweep_degrees_perseg[i]->get() ); 
  }

  xmlNodePtr tesweep_degperseg_list_node = xmlNewChild( root, NULL, (const xmlChar *)"TESweep_DegPerSeg_List", NULL );
  xmlAddIntNode( tesweep_degperseg_list_node, "List_Size", tesweep_degrees_perseg.dimension() );

  for( i = 0; i < tesweep_degrees_perseg.dimension(); i++ )
  {
	char descrip[255];
	sprintf( descrip, "DegPerSeg%d", i );
	xmlAddDoubleNode( tesweep_degperseg_list_node, descrip, tesweep_degrees_perseg[i]->get() );
  }

  xmlNodePtr dihedral_degperseg_list_node = xmlNewChild( root, NULL, (const xmlChar *)"Dihedral_DegPerSeg_List", NULL );
  xmlAddIntNode( dihedral_degperseg_list_node, "List_Size", dihedral_degrees_perseg.dimension() );

  for( i = 0; i < dihedral_degrees_perseg.dimension(); i++ )
  {
	char descrip[255];
	sprintf( descrip, "DegPerSeg%d", i );
	xmlAddDoubleNode( dihedral_degperseg_list_node, descrip, dihedral_degrees_perseg[i]->get() );
  }

  //===== Write Sections =====//
  xmlNodePtr sec_node;
  xmlNodePtr sec_list_node = xmlNewChild( root, NULL, (const xmlChar *)"Section_List", NULL );

  for ( i = 0 ; i < nsect ; i++ )
  {
    sec_node = xmlNewChild( sec_list_node, NULL, (const xmlChar *)"Section", NULL );

    xmlAddIntNode( sec_node, "Driver",      sects[i].driver );
    xmlAddDoubleNode( sec_node, "AR",       sects[i].ar );
    xmlAddDoubleNode( sec_node, "TR",       sects[i].tr );
    xmlAddDoubleNode( sec_node, "Area",     sects[i].area );
    xmlAddDoubleNode( sec_node, "Span",     sects[i].span );
    xmlAddDoubleNode( sec_node, "TC",       sects[i].tc );
    xmlAddDoubleNode( sec_node, "RC",       sects[i].rc );
    xmlAddDoubleNode( sec_node, "Sweep",    sects[i].sweep );
    xmlAddDoubleNode( sec_node, "SweepLoc", sects[i].sweepLoc );
    xmlAddDoubleNode( sec_node, "Twist",    sects[i].twist );
    xmlAddDoubleNode( sec_node, "TwistLoc", sects[i].twistLoc );
    xmlAddDoubleNode( sec_node, "Dihedral", sects[i].dihedral );

    xmlAddIntNode( sec_node, "DihedRotFlag", sects[i].dihedRotFlag );
    xmlAddIntNode( sec_node, "SmoothBlendFlag", sects[i].smoothBlendFlag );
    xmlAddIntNode( sec_node, "NumInterpXsecs", sects[i].num_interp_xsecs );
  }
}

//==== Write Multi Section Wing File ====//
void Hwb_geom::read(xmlNodePtr root)
{
  int i;
  xmlNodePtr node;

  //===== Read General Parameters =====//
  node = xmlGetNode( root, "General_Parms", 0 );
  if ( node )
    read_general_parms( node );

  //===== Read Fuse Parameters =====//
  node = xmlGetNode( root, "Hwb_Parms", 0 );
  if ( node )
  {
    double temp = 0.0;	

    temp = xmlFindDouble( node, "Total_Area", temp );
    get_total_area()->set(temp);
    temp = xmlFindDouble( node, "Total_Span", temp );
    get_total_span()->set(temp);
    temp = xmlFindDouble( node, "Total_Proj_Span", temp );
    get_total_proj_span()->set(temp);
    temp  = xmlFindDouble( node, "Avg_Chord", temp );
    get_avg_chord()->set(temp);
    temp  = xmlFindDouble( node, "Sweep_Off", temp );
    get_sweep_off()->set(temp);
  }

  //==== Read Airfoils ====//
  xmlNodePtr af_list_node = xmlGetNode( root, "Airfoil_List", 0 );
  if ( af_list_node )
  {
    dyn_array< Af* > tmpfoils;

    //==== Delete Foils ====//
    for ( i = 0 ; i < foils.dimension() ; i++ )
    {
		if(foils[i])
			delete foils[i];			
    }

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

  int list_size = 0;
  xmlNodePtr list_node = xmlGetNode( root, "Section_Sizes_List", 0 );
  list_size =  xmlFindInt( list_node, "List_Size", list_size );
  
  if ( list_node )
  {
	dyn_array<int> temp_section_sizes;
	int temp = 0;

	for( i = 0; i < list_size; i++ )
	{
		char descrip[255];
		sprintf( descrip, "Section_Size%d", i );
		//string label = descrip;
		temp = xmlFindInt( list_node, descrip, temp );
		temp_section_sizes.append( temp );
	}
	section_sizes = temp_section_sizes;
  }

  list_node = xmlGetNode( root, "Sweep_Percent_Lengths_List", 0 );
  list_size =  xmlFindInt( list_node, "List_Size", list_size );
  
  if ( list_node )
  {
	for( i = 0; i < sweep_percent_edge_lengths.dimension(); i++ )
	{
		if(sweep_percent_edge_lengths[i])
			delete sweep_percent_edge_lengths[i];
	}

	dyn_array<Parm*> temp_sweep_percent_lengths;
	double temp = 0;

	for( i = 0; i < list_size; i++ )
	{
		char descrip[255];
		sprintf( descrip, "Percent_Length%d", i );
		temp = xmlFindDouble( list_node, descrip, temp );
		Parm* sweep_init = new Parm();
		sweep_init->initialize(this, UPD_MSW_SECTS, "Sweep Percent Span of Fillet", temp);
		sweep_init->set_lower_upper(0.0, 0.5);
		sweep_init->set_script("wing_fillet percentspan", 0);
		temp_sweep_percent_lengths.append(sweep_init);
	}

	sweep_percent_edge_lengths = temp_sweep_percent_lengths;
  }

  list_node = xmlGetNode( root, "TESweep_Percent_Lengths_List", 0 );
  list_size =  xmlFindInt( list_node, "List_Size", list_size );
  
  if ( list_node )
  {
	for( i = 0; i < tesweep_percent_edge_lengths.dimension(); i++ )
	{
		if(tesweep_percent_edge_lengths[i])
			delete tesweep_percent_edge_lengths[i];
	}

	dyn_array<Parm*> temp_tesweep_percent_lengths;
	double temp = 0;

	for( i = 0; i < list_size; i++ )
	{
		char descrip[255];
		sprintf( descrip, "Percent_Length%d", i );
		temp = xmlFindDouble( list_node, descrip, temp );
		Parm* tesweep_init = new Parm();
		tesweep_init->initialize(this, UPD_MSW_SECTS, "TE Sweep Percent Span of Fillet", temp);
		tesweep_init->set_lower_upper(0.0, 0.5);
		tesweep_init->set_script("wing_fillet percentspan", 0);
		temp_tesweep_percent_lengths.append(tesweep_init);
	}

	tesweep_percent_edge_lengths = temp_tesweep_percent_lengths;
  }

  list_node = xmlGetNode( root, "Dihedral_Percent_Lengths_List", 0 );
  list_size =  xmlFindInt( list_node, "List_Size", list_size );
  
  if ( list_node )
  {
	for( i = 0; i < dihedral_percent_edge_lengths.dimension(); i++ )
	{
		if(dihedral_percent_edge_lengths[i])
			delete dihedral_percent_edge_lengths[i];
	}

	dyn_array<Parm*> temp_dihedral_percent_lengths;
	double temp = 0;

	for( i = 0; i < list_size; i++ )
	{
		char descrip[255];
		sprintf( descrip, "Percent_Length%d", i );
		temp = xmlFindDouble( list_node, descrip, temp );
		Parm* dihedral_init = new Parm();
		dihedral_init->initialize(this, UPD_MSW_SECTS, "Dihedral Percent Span of Fillet", temp);
		dihedral_init->set_lower_upper(0.0, 0.5);
		dihedral_init->set_script("wing_fillet percentspan", 0);
		temp_dihedral_percent_lengths.append(dihedral_init);
	}

	dihedral_percent_edge_lengths = temp_dihedral_percent_lengths;
  }

  list_node = xmlGetNode( root, "Sweep_DegPerSeg_List", 0 );
  list_size =  xmlFindInt( list_node, "List_Size", list_size );
  
  if ( list_node )
  {
	for( i = 0; i < sweep_degrees_perseg.dimension(); i++ )
	{
		if(sweep_degrees_perseg[i])
			delete sweep_degrees_perseg[i];
	}

	dyn_array<Parm*> temp_sweep_degrees_perseg;
	double temp = 0;

	for( i = 0; i < list_size; i++ )
	{
		char descrip[255];
		sprintf( descrip, "DegPerSeg%d", i );
		temp = xmlFindDouble( list_node, descrip, temp );
		Parm* sweep_init = new Parm();
		sweep_init->initialize(this, UPD_MSW_SECTS, "Sweep Degrees Per Section of Fillet", temp);
		sweep_init->set_lower_upper(1.0, 90.0);
		sweep_init->set_script("wing_fillet percentspan", 0);
		temp_sweep_degrees_perseg.append(sweep_init);
	}

	sweep_degrees_perseg = temp_sweep_degrees_perseg;
  }

  list_node = xmlGetNode( root, "TESweep_DegPerSeg_List", 0 );
  list_size =  xmlFindInt( list_node, "List_Size", list_size );

  if ( list_node )
  {
	for( i = 0; i < tesweep_degrees_perseg.dimension(); i++ )
	{
		if(tesweep_degrees_perseg[i])
			delete tesweep_degrees_perseg[i];
	}

	dyn_array<Parm*> temp_tesweep_degrees_perseg;
	double temp = 0;

	for( i = 0; i < list_size; i++ )
	{
		char descrip[255];
		sprintf( descrip, "DegPerSeg%d", i );
		temp = xmlFindDouble( list_node, descrip, temp );
		Parm* tesweep_init = new Parm();
		tesweep_init->initialize(this, UPD_MSW_SECTS, "TE Sweep Degrees Per Section of Fillet", temp);
		tesweep_init->set_lower_upper(1.0, 90.0);
		tesweep_init->set_script("wing_fillet percentspan", 0);
		temp_tesweep_degrees_perseg.append(tesweep_init);
	}

	tesweep_degrees_perseg = temp_tesweep_degrees_perseg;
  }

  list_node = xmlGetNode( root, "Dihedral_DegPerSeg_List", 0 );
  list_size =  xmlFindInt( list_node, "List_Size", list_size );
  
  if ( list_node )
  {
	for( i = 0; i < dihedral_degrees_perseg.dimension(); i++ )
	{
		if(dihedral_degrees_perseg[i])
			delete dihedral_degrees_perseg[i];
	}

	dyn_array<Parm*> temp_dihedral_degrees_perseg;
	double temp = 0;

	for( i = 0; i < list_size; i++ )
	{
		char descrip[255];
		sprintf( descrip, "DegPerSeg%d", i );
		temp = xmlFindDouble( list_node, descrip, temp );
		Parm* dihedral_init = new Parm();
		dihedral_init->initialize(this, UPD_MSW_SECTS, "Dihedral Degrees Per Section of Fillet", temp);
		dihedral_init->set_lower_upper(1.0, 90.0);
		dihedral_init->set_script("wing_fillet percentspan", 0);
		temp_dihedral_degrees_perseg.append(dihedral_init);
	}

	dihedral_degrees_perseg = temp_dihedral_degrees_perseg;
  }

  xmlNodePtr sec_list_node = xmlGetNode( root, "Section_List", 0 );
  if ( sec_list_node )
  {
    int num_sec =  xmlGetNumNames( sec_list_node, "Section" );
    assert( num_sec+1 == foils.dimension() );

    dyn_array<HwbSect> tmpsects;

    for ( i = 0 ; i < num_sec ; i++ )
    {
      HwbSect ws;
      xmlNodePtr sec_node = xmlGetNode( sec_list_node, "Section", i );

      ws.driver   = xmlFindInt( sec_node, "Driver", ws.driver );
      ws.ar       = xmlFindDouble( sec_node, "AR", ws.ar );
      ws.tr       = xmlFindDouble( sec_node, "TR", ws.tr );
      ws.area     = xmlFindDouble( sec_node, "Area", ws.area );
      ws.span     = xmlFindDouble( sec_node, "Span", ws.span );
      ws.tc       = xmlFindDouble( sec_node, "TC", ws.tc );
      ws.rc       = xmlFindDouble( sec_node, "RC", ws.rc );
      ws.sweep    = xmlFindDouble( sec_node, "Sweep", ws.sweep );
      ws.sweepLoc = xmlFindDouble( sec_node, "SweepLoc", ws.sweepLoc );
      ws.twist    = xmlFindDouble( sec_node, "Twist", ws.twist );
      ws.twistLoc = xmlFindDouble( sec_node, "TwistLoc", ws.twistLoc );
      ws.dihedral = xmlFindDouble( sec_node, "Dihedral", ws.dihedral );

      ws.dihedRotFlag     = xmlFindInt( sec_node, "DihedRotFlag", ws.dihedRotFlag );
      ws.smoothBlendFlag  = xmlFindInt( sec_node, "SmoothBlendFlag", ws.smoothBlendFlag );
      ws.num_interp_xsecs = xmlFindInt( sec_node, "NumInterpXsecs", ws.num_interp_xsecs );

      ws.rootAf = foils[i];
      ws.tipAf  = foils[i+1];

      tmpsects.append( ws );
    }
    sects = tmpsects;
  }

  int chord_lengths_size = chord_lengths.dimension();

  for(int i = 0; i < chord_lengths_size; i++)
  {
	  chord_lengths.del_index(0);
  }
 
  int num_sects = sects.dimension();

  for(int i = 0; i <= num_sects; i++)
  {
		chord_lengths.append(0.0);
  }

  set_num_pnts_all_afs();
  set_curr_sect( 0 );
  set_depend_parms();
  generate();
}

//==== Define  Parms ====//
void Hwb_geom::define_parms()
{
	int subsect_dim = sub_sects.dimension();
	int size_dim = section_sizes.dimension();
	int dihed_rot_dim = dihedral_rot_starting_indices.dimension();
	int sub_sect_foils_dim = sub_sect_foils.dimension();

	for(int i = 0; i < subsect_dim; i++)
	{
		sub_sects.del_index(0);
	}

	// = Destroy the section sizes = //
	for(int i = 0; i < size_dim; i++)
	{
		section_sizes.del_index(0);
	}

	// = Destroy the dihedral rotation indices = //
	for(int i = 0; i < dihed_rot_dim; i++)
	{
		dihedral_rot_starting_indices.del_index(0);
	}

	for(int i = 0; i < sub_sect_foils_dim; i++)
	{
		if(sub_sect_foils[i])
		{
			delete sub_sect_foils[i];
			sub_sect_foils[i] = NULL;
		}
	}

	for(int i = 0; i < sub_sect_foils_dim; i++)
	{
		sub_sect_foils.del_index(0);
	}

	for(int i = 0; i < sects.dimension(); i++)
	{
		sub_sects.append(sects[i]);

		if(i == 0)
		{
			Af* rootaf = new Af(this);
			sub_sects[i].rootAf = rootaf;
			*(sub_sects[i].rootAf) = *(sects[i].rootAf);
			sub_sects[i].rootAf->sterilize_parms();
			sub_sect_foils.append(rootaf);
		}
		else
		{
			sub_sects[i].rootAf = sub_sects[i-1].tipAf;
		}

		Af* tipaf = new Af(this);
		sub_sects[i].tipAf = tipaf;
		*(sub_sects[i].tipAf) = *(sects[i].tipAf);
		sub_sects[i].tipAf->sterilize_parms();
		sub_sect_foils.append(tipaf);
	}

	int num_sects = sub_sects.dimension();

	for(int i = 0; i < num_sects; i++)
		section_sizes.append(1);

	for(int i = 0; i < num_sects; i++)
		dihedral_rot_starting_indices.append(i);

	for(int i = 0; i < num_sects; i++)
	{
		chord_lengths.append(sects[i].rc);

		if(i == (num_sects-1))
			chord_lengths.append(sects[i].tc);
	}

	for(int i = 0; i < num_sects; i++)
	{
		Parm* sweep_init = new Parm();
		sweep_init->initialize(this, UPD_MSW_SECTS, "Sweep Percent Span of Fillet", 0.0);
		sweep_init->set_lower_upper(0.0, 0.5);
		sweep_init->set_script("wing_fillet percentspan", 0);
		sweep_percent_edge_lengths.append(sweep_init);
	}

	for(int i = 0; i < num_sects; i++)
	{
		Parm* tesweep_init = new Parm();
		tesweep_init->initialize(this, UPD_MSW_SECTS, "TE Sweep Percent Span of Fillet", 0.0);
		tesweep_init->set_lower_upper(0.0, 0.5);
		tesweep_init->set_script("wing_fillet percentspan", 0);
		tesweep_percent_edge_lengths.append(tesweep_init);
	}

	for(int i = 0; i < num_sects; i++)
	{
		Parm* dihedral_init = new Parm();
		dihedral_init->initialize(this, UPD_MSW_SECTS, "Dihedral Percent Span of Fillet", 0.0);
		dihedral_init->set_lower_upper(0.0, 0.5);
		dihedral_init->set_script("wing_fillet percentspan", 0);
		dihedral_percent_edge_lengths.append(dihedral_init);
	}

	for(int i = 0; i < num_sects; i++)
	{
		Parm* sweep_init = new Parm();
		sweep_init->initialize(this, UPD_MSW_SECTS, "Sweep Degrees Per Section of Fillet", 5.0);
		sweep_init->set_lower_upper(1.0, 90.0);
		sweep_init->set_script("wing_fillet degperseg", 0);
		sweep_degrees_perseg.append(sweep_init);
	}

	for(int i = 0; i < num_sects; i++)
	{
		Parm* tesweep_init = new Parm();
		tesweep_init->initialize(this, UPD_MSW_SECTS, "TE Sweep Degrees Per Section of Fillet", 5.0);
		tesweep_init->set_lower_upper(1.0, 90.0);
		tesweep_init->set_script("wing_fillet degperseg", 0);
		tesweep_degrees_perseg.append(tesweep_init);
	}

	for(int i = 0; i < num_sects; i++)
	{
		Parm* dihedral_init = new Parm();
		dihedral_init->initialize(this, UPD_MSW_SECTS, "Dihedral Degrees Per Section of Fillet", 5.0);
		dihedral_init->set_lower_upper(1.0, 90.0);
		dihedral_init->set_script("wing_fillet degperseg", 0);
		dihedral_degrees_perseg.append(dihedral_init);
	}

  	//==== More Defaults ====//
  	sub_sects[0].rootAf->get_thickness()->set( 0.04 );
  	sub_sects[0].rootAf->generate_airfoil();
  	sub_sects[0].rootAf->load_name();
  	sub_sects[0].tipAf->get_thickness()->set( 0.06 );
  	sub_sects[0].tipAf->generate_airfoil();
  	sub_sects[0].tipAf->load_name();

	//==== Make Sure Root and Tip of Adjoining Sections Match ====//
  	matchWingSects();

  	set_num_pnts_all_afs();

  	mwing_surf.set_num_pnts( numPnts.iget() );
  	mwing_surf.set_num_xsecs( num_sects + 3 );

  	compose_model_matrix();

  	//==== Compute Totals ====//
  	double ta = 0.0;
  	double ts = 0.0;
  	double tc = 0.0;
  	double tps = 0.0;

  	for (int i = 0 ; i < sub_sects.dimension() ; i++ )
  	{
  		ta += sects[i].area;
    		ts += sects[i].span;
    		tc += sects[i].tc + sects[i].rc;
		tps += sects[i].span * cos( sects[i].dihedral * DEG_2_RAD );
  	}

	if ( sym_code == XZ_SYM )
	{
		ta *= 2.0;
		ts *= 2.0;
		tps *= 2.0;
	}

  	double ac = tc/(sects.dimension() * 2 );
  	double ar = (ts*ts)/ta;
	
	double sps =  sects[currSect].span * fabs(cos( sects[currSect].dihedral *  DEG_2_RAD ));
  	get_sect_proj_span()->initialize(this, UPD_MSW_SECTS, "Sect_Aspect", sps );
  	get_sect_proj_span()->set_lower_upper(0.1, 10000.0);
	
  	get_total_area()->initialize(this, UPD_MSW_TOTAL_AREA, "Total_Area", ta);
  	get_total_area()->set_lower_upper(1.0, 1000000.0);
  	get_total_area()->set_script("wing_plan area", 0);
	
  	get_total_span()->initialize(this, UPD_MSW_TOTAL_SPAN, "Total_Span", ts);
  	get_total_span()->set_lower_upper(0.1, 10000.0);
  	get_total_span()->set_script("wing_plan span", 0);
	
  	get_total_proj_span()->initialize(this, UPD_MSW_TOTAL_SPAN, "Total_Proj_Span", tps);
  	get_total_proj_span()->set_lower_upper(0.1, 10000.0);
  	get_total_proj_span()->set_script("wing_plan projspan", 0);

  	get_avg_chord()->initialize(this, UPD_MSW_AVG_CHORD, "Avg_Chord", ac);
  	get_avg_chord()->set_lower_upper(0.1, 10000.0);
  	get_avg_chord()->set_script("wing_plan chord", 0);

  	get_sweep_off()->initialize(this, UPD_MW_ALL, "Sweep_Offset", 0.0);
  	get_sweep_off()->set_lower_upper(-85.0, 85.0);
  	get_sweep_off()->set_script("wing_plan sweepoff", 0);
	
  	get_total_aspect()->initialize(this, UPD_MSW_ALL, "Total_Aspect", ar);
  	get_total_aspect()->deactivate();
  	get_total_aspect()->set_script("wing_plan ar", 0);

  	get_sect_aspect()->initialize(this, UPD_MSW_SECTS, "Sect_Aspect", sects[currSect].ar);
  	get_sect_aspect()->set_lower_upper(0.05, 100.0);
  	get_sect_aspect()->set_script("wing_sect ar", 0);

  	get_sect_taper()->initialize(this, UPD_MSW_SECTS, "Sect_Taper", sects[currSect].tr);
  	get_sect_taper()->set_lower_upper(0.01, 5.0);
  	get_sect_taper()->set_script("wing_sect tr", 0);

  	get_sect_area()->initialize(this, UPD_MSW_SECTS, "Sect_Area", sects[currSect].area);
  	get_sect_area()->set_lower_upper(0.001, 1000000.0);
  	get_sect_area()->set_script("wing_sect area", 0);

  	get_sect_span()->initialize(this, UPD_MSW_SECTS, "Sect_Span", sects[currSect].span);
  	get_sect_span()->set_lower_upper(0.001, 1000000.0);
  	get_sect_span()->set_script("wing_sect span", 0);

  	get_sect_rc()->initialize(this, UPD_MSW_SECTS, "Sect_RC", sects[currSect].rc);
  	get_sect_rc()->set_lower_upper(0.001, 10000.0);
  	get_sect_rc()->set_script("wing_sect rc", 0);

  	get_sect_tc()->initialize(this, UPD_MSW_SECTS, "Sect_TC", sects[currSect].tc);
  	get_sect_tc()->set_lower_upper(0.001, 10000.0);
  	get_sect_tc()->set_script("wing_sect tc", 0);

  	get_sect_sweep()->initialize(this, UPD_MSW_SECTS, "Sect_Sweep", sects[currSect].sweep);
  	get_sect_sweep()->set_lower_upper(-85.0, 85.0);
  	get_sect_sweep()->set_script("wing_sect sweep", 0);

  	get_sect_sweep_loc()->initialize(this, UPD_MSW_SECTS, "Sect_Sweep_Loc", sects[currSect].sweepLoc);
  	get_sect_sweep_loc()->set_lower_upper(0.0, 1.0);
  	get_sect_sweep_loc()->set_script("wing_sect sweeploc", 0);

  	get_sect_twist()->initialize(this, UPD_MSW_SECTS, "Sect_Twist", sects[currSect].twist);
  	get_sect_twist()->set_lower_upper(-45.0, 45.0);
  	get_sect_twist()->set_script("wing_sect twist", 0);
	
  	get_sect_twist_loc()->initialize(this, UPD_MSW_SECTS, "Sect_Twist_Loc", sects[currSect].twistLoc);
  	get_sect_twist_loc()->set_lower_upper(0.0, 1.0);
  	get_sect_twist_loc()->set_script("wing_sect twistloc", 0);

  	get_sect_dihed1()->initialize(this, UPD_MSW_SECTS, "Sect_Dihedral", sects[currSect].dihedral);
  	get_sect_dihed1()->set_lower_upper(-360.0, 360.0);
  	get_sect_dihed1()->set_script("wing_dihed dihed1", 0);

	 get_sect_dihed2()->initialize(this, UPD_MSW_SECTS, "Sect_Dihedral", sects[currSect].dihedral);
  	 get_sect_dihed2()->set_lower_upper(-360.0, 360.0);
    	 get_sect_dihed2()->set_script("wing_dihed dihed2", 0);
	
	sweep_percent_edge_length = new Parm();
	sweep_percent_edge_length->initialize(this, UPD_MSW_SECTS, "Sweep Percent Span of Fillet", 0.0);
	sweep_percent_edge_length->set_lower_upper(0.0, 0.5);
	sweep_percent_edge_length->set_script("wing_fillet sweeppercentspan1", 0);

	tesweep_percent_edge_length = new Parm();
	tesweep_percent_edge_length->initialize(this, UPD_MSW_SECTS, "Trailing Edge Sweep Percent Span of Fillet", 0.0);
	tesweep_percent_edge_length->set_lower_upper(0.0, 0.5);
	tesweep_percent_edge_length->set_script("wing_fillet tesweeppercentspan1", 0);

	dihedral_percent_edge_length = new Parm();
	dihedral_percent_edge_length->initialize(this, UPD_MSW_SECTS, "Dihedral Percent Span of Fillet", 0.0);
	dihedral_percent_edge_length->set_lower_upper(0.0, 0.5);
	dihedral_percent_edge_length->set_script("wing_fillet dihedralpercentspan1", 0);

	sweep_degperseg = new Parm();
	sweep_degperseg->initialize(this, UPD_MSW_SECTS, "Degrees Per Segment in Blend for Sweep", 5.0);
  	sweep_degperseg->set_lower_upper( 1.0, 30.0 );
  	sweep_degperseg->set_script("wing_fillet sweepdegperseg", 1);

	tesweep_degperseg = new Parm();
	tesweep_degperseg->initialize(this, UPD_MSW_SECTS, "Degrees Per Segment in Blend for Sweep", 5.0);
  	tesweep_degperseg->set_lower_upper( 1.0, 30.0 );
  	tesweep_degperseg->set_script("wing_fillet sweepdegperseg", 1);

	dihedral_degperseg = new Parm();
	dihedral_degperseg->initialize(this, UPD_MSW_SECTS, "Degrees Per Segment in Blend for Dihedral", 5.0);
  	dihedral_degperseg->set_lower_upper( 1.0, 30.0 );
  	dihedral_degperseg->set_script("wing_fillet dihedraldegperseg", 1);

    filleted_tip_chord = new Parm();
	filleted_tip_chord->initialize(this, UPD_MSW_SECTS, "Length of Tip Chord After Fillet", 1.0);
	filleted_tip_chord->set_lower_upper( 0.0, 9000.00);
	filleted_tip_chord->set_script("wing_sect filletedtipchord", 1);

	filleted_root_chord = new Parm();
	filleted_root_chord->initialize(this, UPD_MSW_SECTS, "Length of Root Chord After Fillet", 1.0);
	filleted_root_chord->set_lower_upper( 0.0, 9000.00);
	filleted_root_chord->set_script("wing_sect filletedrootchord", 1);

	driver =  sects[currSect].driver;
  	set_driver( driver );
}

//==== Add Section ====//
void Hwb_geom::add_sect()
{
	HwbBaseGeom::add_sect();

	int num_sects = section_sizes[currSect];

 	section_sizes.insert_after_index( num_sects, currSect );
	chord_lengths.insert_after_index( (sects[currSect].rc + sects[currSect].tc)*0.5, currSect );

	Parm* new_parm = new Parm();
	new_parm->initialize(this, UPD_MSW_SECTS, "Sweep Degrees Per Section of Fillet", 5);
	new_parm->set_lower_upper(1.0, 90.0);
	new_parm->set_script("wing_fillet degperseg", 0);
	sweep_degrees_perseg.insert_after_index( new_parm, currSect);
	
	new_parm = new Parm();
	new_parm->initialize(this, UPD_MSW_SECTS, "TE Sweep Degrees Per Section of Fillet", 5);
	new_parm->set_lower_upper(1.0, 90.0);
	new_parm->set_script("wing_fillet degperseg", 0);
	tesweep_degrees_perseg.insert_after_index( new_parm, currSect );

	new_parm = new Parm();
	new_parm->initialize(this, UPD_MSW_SECTS, "Dihedral Degrees Per Section of Fillet", 5);
	new_parm->set_lower_upper(1.0, 90.0);
	new_parm->set_script("wing_fillet degperseg", 0);
	dihedral_degrees_perseg.insert_after_index( new_parm, currSect );

	new_parm = new Parm();
	new_parm->initialize(this, UPD_MSW_SECTS, "Sweep Percent Span of Fillet", 0.0);
	new_parm->set_lower_upper(0.0, 0.5);
	new_parm->set_script("wing_fillet percentspan", 0);
	sweep_percent_edge_lengths.insert_after_index( new_parm, currSect);
	
	new_parm = new Parm();
	new_parm->initialize(this, UPD_MSW_SECTS, "TE Sweep Percent Span of Fillet", 0.0);
	new_parm->set_lower_upper(0.0, 0.5);
	new_parm->set_script("wing_fillet percentspan", 0);
	tesweep_percent_edge_lengths.insert_after_index( new_parm, currSect );

	new_parm = new Parm();
	new_parm->initialize(this, UPD_MSW_SECTS, "Dihedral Percent Span of Fillet", 0.0);
	new_parm->set_lower_upper(0.0, 0.5);
	new_parm->set_script("wing_fillet percentspan", 0);
	dihedral_percent_edge_lengths.insert_after_index( new_parm, currSect );

	set_depend_parms();
	generate();

	airPtr->geomMod( this );

	filleted_tip_chord->set( chord_lengths[currSect+1] );
	filleted_root_chord->set( chord_lengths[currSect] );
}

//==== Update Bounding Box =====//
void Hwb_geom::draw_sect_box()
{
	if ( fastDrawFlag )
		return;

	if(type != HWB_GEOM_TYPE)
		return;

	vec3d pnt;
	bbox box;

	int num_pnts  = mwing_surf.get_num_pnts();

	int minSectID = 0;
	int maxSectID = 0;
	int starting_index = get_starting_index(currSect);

	for ( int i = 0; i < starting_index; i++ )
	{ 
		minSectID += sub_sects[i].num_actual_xsecs;
	}

	maxSectID = minSectID;
	int next_starting_index = starting_index + section_sizes[currSect];
	for(int i = starting_index; i < next_starting_index; i++)
	{	
		maxSectID += sub_sects[i].num_actual_xsecs;
	}

	if ( minSectID == 0 )
		minSectID = 1;

	if ( highlightType == MSW_HIGHLIGHT_JOINT )
		maxSectID = minSectID;

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
void Hwb_geom::draw_refl_sect_box()
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
	int starting_index = get_starting_index(currSect);
	for ( int i = 0; i < starting_index; i++ )
	{ 
		minSectID += sub_sects[i].num_actual_xsecs;
	}

	maxSectID = minSectID;
	int next_starting_index = starting_index + section_sizes[currSect];
	for(int i = starting_index; i < next_starting_index; i++)	
		maxSectID += sub_sects[i].num_actual_xsecs;

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

void Hwb_geom::generate()
{

  for(int i = 0; i < sects.dimension(); i++)
  {
	  sects[i].rootAf->generate_airfoil();
	  sects[i].tipAf->generate_airfoil();
  }

  smooth_wing_joints();

  generate_surf();

  update_bbox();
}

void Hwb_geom::DebugOutput()
{
	for(int i = 0; i < sects.dimension(); i++)
	{
		cout << "% sects rootaf address: " << sects[i].rootAf << endl;
		cout << "% sects rootaf address: " << sects[i].tipAf << endl;
	}

	for(int i = 0; i < sub_sect_foils.dimension(); i++)
	{
		cout << "% sub sect foils address: " << sub_sect_foils[i] << endl;
	}
}

//==== Draw Multi Section Wing Geom ====//
void Hwb_geom::draw()
{
	HwbBaseGeom::draw();	

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

//==== Parm Has Changed ReGenerate Multi Section Wing Component ====//
void Hwb_geom::parm_changed(Parm* p)
{
	filleted_tip_chord->set( chord_lengths[currSect+1] );
	filleted_root_chord->set( chord_lengths[currSect] );

	HwbBaseGeom::parm_changed(p);
}

void Hwb_geom::set_depend_parms()
{
	//get the fillet control variables
  	sweep_percent_edge_lengths[currSect]->set(sweep_percent_edge_length->get());
  	tesweep_percent_edge_lengths[currSect]->set(tesweep_percent_edge_length->get());
  	dihedral_percent_edge_lengths[currSect]->set(dihedral_percent_edge_length->get());

	sweep_degrees_perseg[currSect]->set(sweep_degperseg->get());
  	tesweep_degrees_perseg[currSect]->set(tesweep_degperseg->get());
  	dihedral_degrees_perseg[currSect]->set(dihedral_degperseg->get());

  sects[currSect].span = get_sect_span()->get();
  sects[currSect].area = get_sect_area()->get();
  sects[currSect].ar   = get_sect_aspect()->get();
  sects[currSect].tr   = get_sect_taper()->get();
  sects[currSect].tc   = get_sect_tc()->get();
  sects[currSect].rc   = get_sect_rc()->get();

  sects[currSect].sweep          = get_sect_sweep()->get();
  sects[currSect].dihedral       = get_sect_dihed1()->get();
  sects[currSect].dihedral       = get_sect_dihed2()->get();
  sects[currSect].twist          = get_sect_twist()->get();
  sects[currSect].twistLoc       = get_sect_twist_loc()->get();

  //==== Check If Sweep Loc Changed ====//
  if ( fabs( get_sect_sweep_loc()->get() - sects[currSect].sweepLoc) > 0.00001 )
  {
    double tan_swp = sects[currSect].tan_sweep_at( get_sect_sweep_loc()->get(), getSymCode() );
    sects[currSect].sweep = atan( tan_swp )*RAD_2_DEG;
    get_sect_sweep()->set( sects[currSect].sweep );

    sects[currSect].sweepLoc  = get_sect_sweep_loc()->get();
  }

  sects[currSect].fillDependData();
  matchWingSects();

  switch (driver)
    {
      case AR_TR_A:
        get_sect_span()->set( sqrt( get_sect_aspect()->get()*get_sect_area()->get() ) );
        get_sect_rc()->set( (2.0*get_sect_span()->get()) / ( get_sect_aspect()->get()*(1.0+get_sect_taper()->get()) ) );
        get_sect_tc()->set( get_sect_taper()->get()*get_sect_rc()->get() );
      break;

      case AR_TR_S:
        get_sect_area()->set( (get_sect_span()->get() * get_sect_span()->get()) / get_sect_aspect()->get() );
        get_sect_rc()->set( (2.0*get_sect_span()->get()) / ( get_sect_aspect()->get()*(1.0+get_sect_taper()->get()) ) );
        get_sect_tc()->set( get_sect_taper()->get()*get_sect_rc()->get() );
      break;

      case AR_TR_TC:
        get_sect_rc()->set( get_sect_tc()->get()/get_sect_taper()->get() );
        get_sect_span()->set( 0.5*get_sect_aspect()->get()*get_sect_rc()->get()*(1.0+get_sect_taper()->get() ) );
        get_sect_area()->set( (get_sect_span()->get() * get_sect_span()->get()) / get_sect_aspect()->get() );
      break;

      case AR_TR_RC:
        get_sect_tc()->set( get_sect_taper()->get()*get_sect_rc()->get() );
        get_sect_span()->set( 0.5*get_sect_aspect()->get()*get_sect_rc()->get()*(1.0+get_sect_taper()->get()) );
        get_sect_area()->set( (get_sect_span()->get() * get_sect_span()->get()) / get_sect_aspect()->get() );
      break;

      case S_TC_RC:
        get_sect_taper()->set( get_sect_tc()->get()/get_sect_rc()->get() );
        get_sect_aspect()->set( 2.0*get_sect_span()->get()/( get_sect_rc()->get()*(1.0+get_sect_taper()->get()) ) );
        get_sect_area()->set( (get_sect_span()->get() * get_sect_span()->get()) / get_sect_aspect()->get() );
     break;

      case A_TC_RC:
        get_sect_taper()->set( get_sect_tc()->get()/get_sect_rc()->get() );
       	get_sect_aspect()->set( 2.0*get_sect_span()->get()/( get_sect_rc()->get()*(1.0+get_sect_taper()->get()) ) );
        get_sect_span()->set( sqrt( get_sect_aspect()->get()*get_sect_area()->get() ) );
      break;

      case TR_S_A:
        get_sect_aspect()->set( (get_sect_span()->get() * get_sect_span()->get()) / get_sect_area()->get() );
        get_sect_rc()->set( (2.0*get_sect_span()->get()) / ( get_sect_aspect()->get()*(1.0+get_sect_taper()->get()) ) );
        get_sect_tc()->set( get_sect_taper()->get()*get_sect_rc()->get() );
      break;
    }

  //==== Compute Total Span ====//
  double ts = 0.0;
  double ta = 0.0;
  double tc = 0.0;
  double tps = 0.0;
  for (int i = 0 ; i < sects.dimension() ; i++ )
  {
			ts += sects[i].span;
			ta += sects[i].area;
      tc += sects[i].tc + sects[i].rc;
		  tps += sects[i].span * cos( sects[i].dihedral * DEG_2_RAD );
  }

	if ( sym_code == XZ_SYM )
	{
		ts *= 2.0;
		tps *= 2.0;
		ta *= 2.0;
	}

  get_total_span()->set( ts );
	get_total_proj_span()->set( tps );
  get_total_area()->set( ta );
	get_avg_chord()->set( tc/(double)(sects.dimension() * 2 ) );
	get_total_aspect()->set( (get_total_span()->get()*get_total_span()->get())/get_total_area()->get() );

	get_sect_proj_span()->set( sects[currSect].span * fabs(cos( sects[currSect].dihedral *  DEG_2_RAD )) );
}

//==== Add Section ====//
void Hwb_geom::ins_sect()
{
  HwbBaseGeom::ins_sect();

  int num_sects = section_sizes[currSect];

  section_sizes.insert_after_index( num_sects, currSect);
  sweep_percent_edge_lengths.insert_after_index( sweep_percent_edge_lengths[currSect], currSect);
  tesweep_percent_edge_lengths.insert_after_index( tesweep_percent_edge_lengths[currSect], currSect);
  dihedral_percent_edge_lengths.insert_after_index( dihedral_percent_edge_lengths[currSect], currSect);
  sweep_degrees_perseg.insert_after_index( sweep_degrees_perseg[currSect], currSect);
  tesweep_degrees_perseg.insert_after_index( tesweep_degrees_perseg[currSect], currSect);
  dihedral_degrees_perseg.insert_after_index( dihedral_degrees_perseg[currSect], currSect);

  chord_lengths.insert_after_index( sects[currSect].tc , currSect );

  set_depend_parms();
  generate();

  airPtr->geomMod( this );

  filleted_tip_chord->set( chord_lengths[currSect+1] );
  filleted_root_chord->set( chord_lengths[currSect] );
}

//==== Delete Section ====//
void Hwb_geom::del_sect()
{
  if ( sects.dimension() < 2 )
    return;
 
  if(currSect == sects.dimension()-1)
  	chord_lengths.del_index(currSect+1);
  else
  	chord_lengths.del_index(currSect);

  sweep_percent_edge_lengths.del_index(currSect);
  tesweep_percent_edge_lengths.del_index(currSect);
  dihedral_percent_edge_lengths.del_index(currSect);

  sweep_degrees_perseg.del_index(currSect);
  tesweep_degrees_perseg.del_index(currSect);
  dihedral_degrees_perseg.del_index(currSect);

  section_sizes.del_index(currSect);

  HwbBaseGeom::del_sect();

  set_depend_parms();
  generate();

  airPtr->geomMod( this );

  filleted_tip_chord->set( chord_lengths[currSect+1] );
  filleted_root_chord->set( chord_lengths[currSect] );
}

//==== Paste Section ====//
void Hwb_geom::paste_sect()
{
	HwbBaseGeom::paste_sect();

	set_depend_parms();
	generate();
  
	airPtr->geomMod( this );

}

void Hwb_geom::scale()
{
    double current_factor = scaleFactor()*(1.0/lastScaleFactor);

    for (  int i = 0 ; i < sects.dimension() ; i++ )
    {
    	int odriver = sects[i].driver;
		int ndriver = MS_S_TC_RC;
		sects[i].driver = ndriver;

		if ( i == 0 )
			sects[i].rc *= current_factor;
		else
			sects[i].rc = sects[i-1].tc;

		sects[i].tc *= current_factor;		
		sects[i].span *= current_factor;
		sects[i].fillDependData();
		sects[i].driver = odriver;
    }

  	//==== Compute Totals ====//
	computeTotals();
    generate();

	lastScaleFactor = scaleFactor();

	filleted_tip_chord->set( chord_lengths[currSect+1] );
	filleted_root_chord->set( chord_lengths[currSect] );
}

//==== Sect Section Index ====//
void Hwb_geom::set_curr_sect(int cs)
{	

  if ( cs >= 0 && cs < sects.dimension() )
    currSect = cs;

  nextSect = currSect;
  get_sect_proj_span()->set( sects[currSect].span * fabs(cos( sects[currSect].dihedral *  DEG_2_RAD )));	

  get_sect_span()->set( sects[currSect].span );
  get_sect_area()->set( sects[currSect].area );
  get_sect_aspect()->set( sects[currSect].ar );
  get_sect_taper()->set( sects[currSect].tr );
  get_sect_tc()->set( sects[currSect].tc );
  get_sect_rc()->set( sects[currSect].rc );
  get_sect_sweep()->set( sects[currSect].sweep );
  get_sect_sweep_loc()->set( sects[currSect].sweepLoc );
  get_sect_twist()->set( sects[currSect].twist );
  get_sect_twist_loc()->set( sects[currSect].twistLoc );
  get_sect_dihed1()->set( sects[currSect].dihedral );
  get_sect_dihed2()->set( sects[currSect].dihedral );

  sweep_percent_edge_length->set(sweep_percent_edge_lengths[currSect]->get());
  tesweep_percent_edge_length->set(tesweep_percent_edge_lengths[currSect]->get());
  dihedral_percent_edge_length->set(dihedral_percent_edge_lengths[currSect]->get());

  sweep_degperseg->set( (sweep_degrees_perseg[currSect])->get() );
  tesweep_degperseg->set( (tesweep_degrees_perseg[currSect])->get() );
  dihedral_degperseg->set( (dihedral_degrees_perseg[currSect])->get() );

  filleted_tip_chord->set( chord_lengths[currSect+1] );
  filleted_root_chord->set( chord_lengths[currSect] );

  driver = sects[currSect].driver;

  set_driver( driver );

  airPtr->geomMod( this );
}

//==== Set Joint Index ====//
void Hwb_geom::set_curr_joint( int cj )
{	
	set_curr_sect(cj);
}

//==== Set Dihed Rot Flag ====//
void Hwb_geom::set_dihed_rot_flag( int rf )
{
  sects[currSect].dihedRotFlag = rf;

  generate();
}

//==== Set Num Interp Curr Sect ====//
void Hwb_geom::set_num_interp( int n )
{
    	sects[currSect].num_interp_xsecs = n;

  	int starting_index = get_starting_index(currSect);
  	int next_starting_index = starting_index + section_sizes[currSect];
  	for(int i = starting_index; i < next_starting_index; i++)
  	{
		sub_sects[i].num_interp_xsecs = n;
	}

    	generate();
}

//==== Set Dependant Parms =====//
void Hwb_geom::dump_xsec_file(int geom_no, FILE* dump_file)
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

void Hwb_geom::loadWingPnts( dyn_array< HwbWingPnt > & wingPnts )
{
	int i;
	HwbWingPnt wp;
	dyn_array< HwbWingPnt > tmpPnts;

	//==== First Point ====//
	wp.x = wp.y = wp.z = 0.0;
	wp.sectID = 0;
	wp.sectFract = 0.0;
	wp.chord = sub_sects[0].rc;
	wp.twist = 0.0;
	wp.twistLoc = sub_sects[0].twistLoc;
	wp.blendFlag = 0;

	if ( sub_sects[0].dihedRotFlag )
		wp.dihedRot = -sub_sects[0].dihedral;
	else
		wp.dihedRot = 0.0;

	wingPnts.append( wp );

  //==== Compute Parameters For Each Section ====//
  double total_span = 0.0;
  double total_sweep_offset = 0.0;
  double total_dihed_offset = 0.0;

  //==== Load End Points for Each Section ====//
  for ( i = 0 ; i < sub_sects.dimension() ; i++ )
  {
	double rad = sub_sects[i].span;
//    if ( get_sym_code() == XZ_SYM )  rad *= 0.5;   // Check Sym Code
	
	double ty = rad*cos(sub_sects[i].dihedral*DEG_2_RAD);
	double tz = rad*sin(sub_sects[i].dihedral*DEG_2_RAD);	

    sub_sects[i].sweep += get_sweep_off()->get();
    double tan_le  = sub_sects[i].tan_sweep_at( 0.0, getSymCode() );
	sub_sects[i].sweep -= get_sweep_off()->get();

    double toff    = tan_le*rad;                    // Tip X Offset

    total_dihed_offset += tz;
    total_span += ty;
    total_sweep_offset += toff;

	wp.x = total_sweep_offset;
	wp.y = total_span;
	wp.z = total_dihed_offset;

	wp.sectID = i;
	wp.sectFract = 1.0;
	wp.chord = sub_sects[i].tc;
	wp.twist = sub_sects[i].twist;
	wp.twistLoc = sub_sects[i].twistLoc;
	wp.blendFlag = 0;

	wp.dihedRot = 0.0;
	//&& sub_sects[i].span > 0.03
	if ( sub_sects[i].dihedRotFlag)
	{
		if ( i < sub_sects.dimension()-1 )
			wp.dihedRot = -0.5*(sub_sects[i].dihedral + sub_sects[i+1].dihedral);
		else
			wp.dihedRot = -sub_sects[i].dihedral;		
	}
	wingPnts.append( wp );
  }

}

void Hwb_geom::generate_surf()
{
	int i, j;
	int num_pnts  = numPnts.iget();

	//==== Load Up Original Plan Points ====//
	dyn_array< HwbWingPnt > wingPnts;
	loadWingPnts( wingPnts );

	//==== Figure Out How Many Xsecs ====//
	int num_xs = wingPnts.dimension() + 2;				// End caps

	//==== Set Up Base Surf ====//
	base_surf.set_num_pnts( num_pnts );
	base_surf.set_num_xsecs( num_xs );

	//==== Clear Section Count ====//
	for ( i = 0 ; i < sub_sects.dimension() ; i++ )
	{
		sub_sects[i].num_xsecs = 0;
	}

	int ixcnt = 0;

	Xsec_surf primer_surf;
	primer_surf.set_num_pnts( num_pnts );
	primer_surf.set_num_xsecs( num_xs );

	int curr_wing_pnt = 0;

	for( i = 0; i < sects.dimension()+1; i++)
	{
		if(i == 0)
			curr_wing_pnt = 0;
		else
			curr_wing_pnt += section_sizes[i-1];

		int sid = wingPnts[curr_wing_pnt].sectID;
		Af* rfoil = sub_sects[sid].rootAf;
		Af* tfoil = sub_sects[sid].tipAf;

		for ( int ip = 0 ; ip < num_pnts ; ip++ )
		{
			vec3d pr = rfoil->get_pnt(ip);
			vec3d pt = tfoil->get_pnt(ip);
			vec3d pi = pr +  ( pt - pr )*wingPnts[curr_wing_pnt].sectFract;			
			primer_surf.set_pnt(ixcnt, ip, pi);
		}

		primer_surf.scale_xsec_z(  ixcnt, wingPnts[curr_wing_pnt].chord );

		ixcnt++;
	}

	//==== Load In Airfoils and Endcaps ====//
	ixcnt = 0;

	for ( i = 0 ; i < wingPnts.dimension() ; i++ )
	{
		int sid = wingPnts[i].sectID;
		Af* rfoil = sub_sects[sid].rootAf;
		Af* tfoil = sub_sects[sid].tipAf;

		Af* foil;									// jrg Should Interpolate Foils??
		if ( wingPnts[i].sectFract <  0.5 )
			foil = rfoil;
		else
			foil = tfoil;

		sub_sects[sid].num_xsecs++;

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
				{
					base_surf.set_pnt(ixcnt, ip, tfoil->get_rounded_end_cap(ip));
				}
				else
				{
    				base_surf.set_pnt(ixcnt, ip, tfoil->get_end_cap(ip));
				}
			}

			ixcnt++;
		}

	}

	for( i = 0; i < wingPnts.dimension()+2; i++)
	{
		for ( int ip = 0 ; ip < num_pnts ; ip++ )
		{
			vec3d out_pnt = base_surf.get_pnt( i, ip);
		}
	}

	int starting_index = 0;
	int next_starting_index = section_sizes[0];
	int sect_index = 0;
	double percentage_span = 0.0;

//	cout << "scale, translate, rotate" << endl;
	//==== Scale, Tranlate Rotate Airfoils ====//
	ixcnt = 0;
	for ( i = 0 ; i < wingPnts.dimension() ; i++ )
	{ 

		int numix = 1;
		if ( i == 0 || i == wingPnts.dimension()-1 )		//End caps
			numix = 2;

		for ( int j = 0 ; j < numix ; j++ )
		{
			if(ixcnt == starting_index || ixcnt == 1 || ixcnt == wingPnts.dimension()+1 || ixcnt == next_starting_index+1)
			{
				base_surf.scale_xsec_x(  ixcnt, wingPnts[i].chord );
				base_surf.scale_xsec_z(  ixcnt, wingPnts[i].chord );
			}
			else
			{
				for ( int ip = 0 ; ip < num_pnts ; ip++ )
				{
					vec3d interp_pnt = primer_surf.get_pnt(sect_index, ip);
					vec3d tip_pnt = primer_surf.get_pnt(sect_index+1, ip);
					vec3d root_pnt = primer_surf.get_pnt(sect_index, ip);
					interp_pnt.set_z( (tip_pnt.z() - root_pnt.z())*percentage_span + root_pnt.z());
					base_surf.set_pnt(ixcnt, ip, interp_pnt);
				}

				base_surf.scale_xsec_x(  ixcnt, wingPnts[i].chord );
			}

    		base_surf.rotate_xsec_x( ixcnt, wingPnts[i].dihedRot );

    		base_surf.offset_xsec_x( ixcnt, -wingPnts[i].twistLoc * wingPnts[i].chord );
    		base_surf.rotate_xsec_y( ixcnt, wingPnts[i].twist);
    		base_surf.offset_xsec_x( ixcnt, wingPnts[i].twistLoc * wingPnts[i].chord);
    		base_surf.offset_xsec_x( ixcnt, wingPnts[i].x );
    		base_surf.offset_xsec_y( ixcnt, wingPnts[i].y );
    		base_surf.offset_xsec_z( ixcnt, wingPnts[i].z );
			
			ixcnt++;
		}

		if(ixcnt > next_starting_index+1)
		{
			starting_index = next_starting_index;
			sect_index++;
			
			if(sect_index < section_sizes.dimension())
				next_starting_index += section_sizes[sect_index];
			
			if(ixcnt-2 < sub_sects.dimension() && sect_index < sects.dimension())
				percentage_span = sub_sects[ixcnt-2].span/sects[sect_index].span;
		}
		else if( ixcnt-2 >= 0 && ixcnt-2 < sub_sects.dimension() )
		{
			percentage_span += sub_sects[ixcnt-2].span/sects[sect_index].span;
		}
	}

	for( i = 0; i < wingPnts.dimension()+2; i++)
	{
		for ( int ip = 0 ; ip < num_pnts ; ip++ )
		{
			vec3d out_pnt = base_surf.get_pnt( i, ip);
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
/*
	for ( i = 2 ; i < (int)tanFlagVec.size()-1 ; i++ )
	{
		int sid = wingPnts[i-1].sectID;	
		if (  )
			tanFlagVec[i] = Bezier_curve::NADA;
	}
*/
	//==== Load Up Bezier Curves (Stringers) ====//
	int closeFlag = 0;
	static vector< Bezier_curve > sVec;
	sVec.resize( stringerVec.size() );

	for ( i = 0 ; i < (int)stringerVec.size() ; i++ )
	{
		sVec[i].buildCurve( stringerVec[i], tanFlagVec, 0.35, closeFlag);
	}

	for ( i = 0 ; i < sub_sects.dimension() ; i++ )
		sub_sects[i].num_actual_xsecs = sub_sects[i].num_xsecs;	

	//==== What u Vals to Interpolate ====//
	vector< double > uVec;				// Vector of U vals 
	double uVal = 0.0;
	uVec.push_back(uVal);
	uVal = 1.0;
	uVec.push_back(uVal);

	for ( int ix = 1 ; ix < wingPnts.dimension() ; ix++ )
	{
		int sid = wingPnts[ix].sectID;	

		int num_interp = sub_sects[sid].num_interp_xsecs;

		if ( wingPnts[ix-1].blendFlag && wingPnts[ix].blendFlag )
			num_interp = 0;

		sub_sects[sid].num_actual_xsecs += num_interp;

		for ( j = 0 ; j < num_interp+1 ; j++ )		
		{
			uVal += 1.0/(double)(num_interp+1);
			uVec.push_back(uVal);
		}

	}

	uVal = uVal+1.0;				// Last Section
	uVec.push_back(uVal);

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
//
//void Hwb_geom::loadWingStrakeGeom( WingGeom* wg )
//{
//	vector< HwbSect > wsVec;
//	double rootXOff = wg->loadMSWingSectVec( wsVec );
//
//	if ( sub_sects.dimension() != wsVec.size() || wsVec.size() != 3 )		//jrg should do better
//		return;
//
//	this->copy( wg );
//
//
//	*sub_sects[0].rootAf = *(wg->get_root_af_ptr());
//	*sub_sects[2].tipAf  = *(wg->get_tip_af_ptr());
//
//	if ( wg->get_aft_ext_span_per()->get() < wg->get_strake_span_per()->get() )
//	{
//		*sub_sects[0].tipAf    = *(wg->get_aft_af_ptr());
//		*sub_sects[1].rootAf   = *(wg->get_aft_af_ptr());
//		*sub_sects[1].tipAf    = *(wg->get_strake_af_ptr());
//		*sub_sects[2].rootAf   = *(wg->get_strake_af_ptr());
//	}
//	else
//	{
//		*sub_sects[0].tipAf   = *(wg->get_strake_af_ptr());
//		*sub_sects[1].rootAf  = *(wg->get_strake_af_ptr());
//		*sub_sects[1].tipAf   = *(wg->get_aft_af_ptr());
//		*sub_sects[2].rootAf  = *(wg->get_aft_af_ptr());
//	}
//
//	for ( int i = 0 ; i < (int)wsVec.size() ; i++ )
//	{
//		Af* raf  = sub_sects[i].rootAf;
//		Af* taf  = sub_sects[i].tipAf;
//		sub_sects[i] = wsVec[i];
//
//		sub_sects[i].dihedRotFlag = 0;
//		sub_sects[i].rootAf = raf;
//		sub_sects[i].tipAf  = taf;
//		sub_sects[i].rootAf->set_geom(this);
//		sub_sects[i].tipAf->set_geom(this);
//
//		sub_sects[i].fillDependData();
//	}
//
//	xLoc = xLoc.get() - rootXOff;
//
//	set_curr_sect( currSect );
//
//	generate();
//	updateAttach(0);
//
//}

// = Retieve the sub-section index that 
int Hwb_geom::get_starting_index(int index)
{
	int starting_index = 0;

	for(int i = 0; i < index; i++)
	{
		starting_index += section_sizes[i];
	}

	return starting_index;
}

//==== Write Rhino File ====//
void Hwb_geom::write_rhino_file(ON_BinaryFile* archive, ON_3dmObjectAttributes* attributes)
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


	dyn_array< HwbWingPnt > wingPnts;
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



void Hwb_geom::smooth_wing_joints()
{
	const double lower_dihedral_threshold = 10;
	const double avg_lower_length_threshold = 0.008;

	int num_sects = sects.dimension();
	int subsect_dim = sub_sects.dimension();
	int size_dim = section_sizes.dimension();
	int dihed_rot_dim = dihedral_rot_starting_indices.dimension();
	int sub_sect_foils_dim = sub_sect_foils.dimension();

	for(int i = 0; i < sub_sect_foils_dim; i++)
	{
		if(sub_sect_foils[i])
		{
			delete sub_sect_foils[i];
			sub_sect_foils[i] = NULL;
		}
	}

	for(int i = 0; i < sub_sect_foils_dim; i++)
	{
		sub_sect_foils.del_index(0);
	}

	// = Destroy the sub-sections = //
	for(int i = 0; i < subsect_dim; i++)
	{
		sub_sects.del_index(0);
	}

	// = Destroy the section sizes = //
	for(int i = 0; i < size_dim; i++)
	{
		section_sizes.del_index(0);
	}

	// = Destroy the dihedral rotation indices = //
	for(int i = 0; i < dihed_rot_dim; i++)
	{
		dihedral_rot_starting_indices.del_index(0);
	}

//	cout << "SUB SECT FOILD DIM: " << sub_sect_foils.dimension() << endl;

	double previous_te_sweep = 0.0;
	double previous_sweep = 0.0;	
	double previous_dihedral = 0.0;
	double previous_twist = 0.0;
	double previous_root_span = 0.0;
	double previous_sweep_offset = 0.0;
	double previous_te_sweep_offset = 0.0;
	double previous_dihedral_offset = 0.0;
	double curr_twist_total = 0.0;
	dyn_array< double > sweep_offsets;
	int num_subsections = 0;
	int total_num_subsections = 0;
	double total_span = 0.0;
	double total_sweep_offset = 0.0;
	double total_te_sweep_offset = 0.0;
	double total_dihedral_offset = 0.0;
	double total_dihedral_offset_x = 0.0;
	double total_twist = 0.0;
	
	double section_rc = sects[0].rc;
	double section_tc = sects[0].tc;	
	double section_sweep = atan( sects[0].tan_sweep_at(0.0,getSymCode()) )*RAD_2_DEG;
	double section_dihedral = sects[0].dihedral;
	double section_sweeploc = 0.0;
	double section_twist = sects[0].twist;
	double section_twistloc = sects[0].twistLoc;
	double section_span = sects[0].span;
	double section_sweep_offset = section_span*tan(section_sweep*DEG_2_RAD);
	double section_dihedral_offset = section_span*sin(section_dihedral*DEG_2_RAD);
	double section_dihedral_offset_x = section_span*cos(section_dihedral*DEG_2_RAD);
	double section_te_sweep = atan( (section_rc - section_sweep_offset - section_tc)/section_span )*RAD_2_DEG;
	double section_te_sweep_offset = section_span*tan(section_te_sweep*DEG_2_RAD);

	//set previous offsets to current initially
	previous_sweep_offset = section_sweep_offset;
	previous_te_sweep_offset = section_te_sweep_offset;
	previous_dihedral_offset = section_dihedral_offset;

	//set totals to current initially
	total_span += section_span;
	total_sweep_offset += section_sweep_offset;
	total_te_sweep_offset += section_te_sweep_offset;
	total_dihedral_offset += section_dihedral_offset;
	total_dihedral_offset_x += section_dihedral_offset_x;

	// = Find number of root subsections for sweep and dihedral = //
	int num_sweep_subsections = (int)ceil( fabs(section_sweep - previous_sweep) / sweep_degrees_perseg[0]->iget() );
	int num_te_sweep_subsections = (int)ceil( fabs(section_te_sweep - previous_te_sweep) / tesweep_degrees_perseg[0]->iget() );
	int num_dihedral_subsections = (int)ceil( fabs(section_dihedral - previous_dihedral) / dihedral_degrees_perseg[0]->iget() );

	// = Retrieve span percentage of root subsections for sweep and dihedral = //
	double sweep_edge_length_percentage = sweep_percent_edge_lengths[0]->get();
	double te_sweep_edge_length_percentage = tesweep_percent_edge_lengths[0]->get();
	double dihedral_edge_length_percentage = dihedral_percent_edge_lengths[0]->get();

	double root_thickness = sects[0].rootAf->get_thickness()->get()*sects[0].rc;
	double tip_thickness = sects[0].rootAf->get_thickness()->get()*sects[0].tc;

	//determine if sweep fillet inverts due to cross section dihedral rotation 
	if(num_sweep_subsections > 1)
	{
		double dihedral_subsection = 0;
		double span_subsection = section_span*sweep_edge_length_percentage/num_sweep_subsections;
		double root_thickness = sects[0].rootAf->get_thickness()->get()*10;
		double radius_dihed_rot = root_thickness - (root_thickness - tip_thickness)*(span_subsection*2/section_span);

		double ortho_deg1 = 90;
		double ortho_deg2 = 0;

		if(num_dihedral_subsections > 0)
		{
			double dihedral_span_subsection = section_span*dihedral_edge_length_percentage/num_dihedral_subsections;
			dihedral_subsection = section_dihedral/(num_dihedral_subsections + 1);

			if(span_subsection*2 < dihedral_span_subsection)
				ortho_deg2 = 90 - dihedral_subsection;
			else if(span_subsection < dihedral_span_subsection)
				ortho_deg2 = 90 - dihedral_subsection*3/2;
			else
				ortho_deg2 = 90 - dihedral_subsection*5/2;
		}
		else
		{
			dihedral_subsection = section_dihedral;
			ortho_deg2 = 90 - dihedral_subsection;
		}

		double x1 = 0;
		double x2 = span_subsection - radius_dihed_rot*cos(ortho_deg2*DEG_2_RAD);

		if(x2 < x1 && sects[0].dihedRotFlag)
			sweep_edge_length_percentage = 0.0;
	}

	//determine if te sweep fillet inverts due to cross section dihedral rotation 
	if(num_te_sweep_subsections > 1)
	{
		double dihedral_subsection = 0;
		double span_subsection = section_span*te_sweep_edge_length_percentage/num_te_sweep_subsections;
		double root_thickness = sects[0].rootAf->get_thickness()->get()*10;
		double radius_dihed_rot = root_thickness - (root_thickness - tip_thickness)*(span_subsection*2/section_span);

		double ortho_deg1 = 90;
		double ortho_deg2 = 0;

		if(num_dihedral_subsections > 0)
		{
			double dihedral_span_subsection = section_span*dihedral_edge_length_percentage/num_dihedral_subsections;
			dihedral_subsection = section_dihedral/(num_dihedral_subsections + 1);

			if(span_subsection*2 < dihedral_span_subsection)
				ortho_deg2 = 90 - dihedral_subsection;
			else if(span_subsection < dihedral_span_subsection)
				ortho_deg2 = 90 - dihedral_subsection*3/2;
			else
				ortho_deg2 = 90 - dihedral_subsection*5/2;
		}
		else
		{
			dihedral_subsection = section_dihedral;
			ortho_deg2 = 90 - dihedral_subsection;
		}

		double x1 = 0;
		double x2 = span_subsection - radius_dihed_rot*cos(ortho_deg2*DEG_2_RAD);

		if(x2 < x1 && sects[0].dihedRotFlag)
			te_sweep_edge_length_percentage = 0.0;
	}

	//determine if dihedral fillet inverts due to cross section dihedral rotation 
	if(num_dihedral_subsections > 1)
	{
		double dihedral_subsection = section_dihedral/(num_dihedral_subsections + 1);
		double span_subsection = section_span*dihedral_edge_length_percentage/num_dihedral_subsections;
		double radius_dihed_rot = root_thickness - (root_thickness - tip_thickness)*(span_subsection*2/section_span);

		double ortho_deg1 = 90;
		double ortho_deg2 = 90 - (dihedral_subsection*5)/2;

		double x1 = 0;
		double x2 = span_subsection - radius_dihed_rot*cos(ortho_deg2*DEG_2_RAD);

		if(x2 < x1 && sects[0].dihedRotFlag)
			dihedral_edge_length_percentage = 0.0;
	}

	//set the number of subsections to 0 if there is no fillet
	if(fabs(sweep_edge_length_percentage) < 0.0001)
		num_sweep_subsections = 0;

	if(fabs(te_sweep_edge_length_percentage) < 0.0001)
		num_te_sweep_subsections = 0;

	if(fabs(dihedral_edge_length_percentage) < 0.0001)
		num_dihedral_subsections = 0;

	double sweep_slope = 0;
	double te_sweep_slope = 0;
	double dihedral_slope = 0;
	double ortho_sweep_slope = 0;
	double ortho_te_sweep_slope = 0;
	double ortho_dihedral_slope = 0;

	double ortho_sweep_intercept = 0.0;
	double ortho_te_sweep_intercept = 0.0;
	double ortho_dihedral_intercept = 0.0;

	double sweep_adj_length = 0.0;
	double te_sweep_adj_length = 0.0;
	double dihedral_adj_length = 0.0;

	double sweep_circle_radius = 0.0;
	double te_sweep_circle_radius = 0.0;
	double dihedral_circle_radius = 0.0;

	double sweep_arc_angle = 0.0;
	double te_sweep_arc_angle = 0.0;
	double dihedral_arc_angle = 0.0;

	double sweep_subsection_arc_angle = 0.0;
	double te_sweep_subsection_arc_angle = 0.0;
	double dihedral_subsection_arc_angle = 0.0;

	double subsection_sweep = 0.0;
	double subsection_te_sweep = 0.0;
	double subsection_dihedral = 0.0;

	double first_sweep_subsection_span = 0.0;
	double first_te_sweep_subsection_span = 0.0;
	double first_dihedral_subsection_span = 0.0;

	//find the fillet boundary point on the leading edge, trailing edge and belly
	vec2d sweep_span_chord( section_sweep_offset*sweep_edge_length_percentage, section_span*sweep_edge_length_percentage );
	vec2d te_sweep_span_chord( section_rc - section_te_sweep_offset*te_sweep_edge_length_percentage, section_span*te_sweep_edge_length_percentage );
	vec2d dihedral_span_chord( section_dihedral_offset_x*dihedral_edge_length_percentage, section_dihedral_offset*dihedral_edge_length_percentage );	

	vec2d sweep_pivot_point, te_sweep_pivot_point, dihedral_pivot_point;
	
	if(num_sweep_subsections == 0)
	{
		sweep_span_chord.set_x(0.0);
		sweep_span_chord.set_y(0.0);
	}	
	
	if(num_te_sweep_subsections == 0)
	{
		te_sweep_span_chord.set_x(section_rc);
		te_sweep_span_chord.set_y(0.0);
	}
	
	if(num_dihedral_subsections == 0)
	{
		dihedral_span_chord.set_x(0.0);
		dihedral_span_chord.set_y(0.0);
	}

	//if even one of the subsection numbers is not 0
	if( num_sweep_subsections != 0 || num_te_sweep_subsections != 0 || num_dihedral_subsections != 0)
	{
		if( (section_sweep_offset - sweep_span_chord.x()) != 0 )
			sweep_slope = (section_span - sweep_span_chord.y()) / (section_sweep_offset - sweep_span_chord.x());
	
		if( (section_rc - section_te_sweep_offset - te_sweep_span_chord.x()) != 0 ) 
			te_sweep_slope = (section_span - te_sweep_span_chord.y()) / (section_rc - section_te_sweep_offset - te_sweep_span_chord.x());

		if( (section_span - dihedral_span_chord.x()) != 0 )
			dihedral_slope = (section_dihedral_offset - dihedral_span_chord.y()) / (section_dihedral_offset_x - dihedral_span_chord.x());

		if(sweep_slope != 0)
			ortho_sweep_slope = -1/sweep_slope;

		if(te_sweep_slope != 0)
			ortho_te_sweep_slope = -1/te_sweep_slope;

		if(dihedral_slope != 0)
			ortho_dihedral_slope = -1/dihedral_slope;

		ortho_sweep_intercept = sweep_span_chord.y() - ortho_sweep_slope*sweep_span_chord.x();
		ortho_te_sweep_intercept = te_sweep_span_chord.y() - ortho_te_sweep_slope*te_sweep_span_chord.x();
		ortho_dihedral_intercept = dihedral_span_chord.y() - ortho_dihedral_slope*dihedral_span_chord.x();
	
		if(sweep_slope != 0)
			sweep_pivot_point.set_xy( -1*ortho_sweep_intercept/ortho_sweep_slope, 0 );
		else
			sweep_pivot_point.set_xy( 0 , 0 );

		if(te_sweep_slope != 0)
			te_sweep_pivot_point.set_xy( -1*ortho_te_sweep_intercept/ortho_te_sweep_slope, 0 );
		else
			te_sweep_pivot_point.set_xy( 0 , 0 );

		if(dihedral_slope != 0)
			dihedral_pivot_point.set_xy( 0 , ortho_dihedral_intercept );
		else
			dihedral_pivot_point.set_xy( 0 , 0 );

		sweep_adj_length = fabs( sweep_pivot_point.x() - sweep_span_chord.x() );
		te_sweep_adj_length = fabs( te_sweep_pivot_point.x() - te_sweep_span_chord.x() );
		dihedral_adj_length = fabs( dihedral_pivot_point.x() - dihedral_span_chord.x() );

		sweep_circle_radius = sqrt( pow(sweep_adj_length, 2) + pow(sweep_span_chord.y(), 2) );
		te_sweep_circle_radius = sqrt( pow(te_sweep_adj_length, 2) + pow(te_sweep_span_chord.y(), 2) );
		dihedral_circle_radius = sqrt( pow(dihedral_adj_length, 2) + pow((dihedral_pivot_point.y() - dihedral_span_chord.y()), 2) );

		if( fabs(sweep_adj_length) > 0.0001)
			sweep_arc_angle = atan( sweep_span_chord.y()/sweep_adj_length )*RAD_2_DEG;

		if( fabs(te_sweep_adj_length) > 0.0001)
			te_sweep_arc_angle = atan( te_sweep_span_chord.y()/te_sweep_adj_length )*RAD_2_DEG;

		if( fabs(dihedral_adj_length) > 0.0001)
			dihedral_arc_angle = atan( dihedral_adj_length/(dihedral_pivot_point.y() - dihedral_span_chord.y()) )*RAD_2_DEG;

		if( num_sweep_subsections != 0 )
			sweep_subsection_arc_angle = sweep_arc_angle / num_sweep_subsections;
		else
			sweep_subsection_arc_angle = sweep_arc_angle;

		if( num_te_sweep_subsections != 0 )
			te_sweep_subsection_arc_angle = te_sweep_arc_angle / num_te_sweep_subsections;
		else
			te_sweep_subsection_arc_angle = te_sweep_arc_angle;

		if( num_dihedral_subsections != 0 )
			dihedral_subsection_arc_angle = dihedral_arc_angle / num_dihedral_subsections;
		else
			dihedral_subsection_arc_angle = dihedral_arc_angle;

		subsection_sweep = (section_sweep - previous_sweep) / ( num_sweep_subsections + 1 );
		subsection_te_sweep = (section_te_sweep - previous_te_sweep) / ( num_te_sweep_subsections + 1 );
		subsection_dihedral = (section_dihedral - previous_dihedral) / ( num_dihedral_subsections + 1 );
		
	}

	//find the span of the first sub-section for sweep, trailing edge sweep and dihedral 
	if( fabs(sweep_subsection_arc_angle) > 0.0001 )
		first_sweep_subsection_span = 2*sweep_circle_radius*sin( sweep_subsection_arc_angle/2 * DEG_2_RAD ) * cos( subsection_sweep * DEG_2_RAD );
	else
		first_sweep_subsection_span = section_span;

	if( fabs(te_sweep_subsection_arc_angle) > 0.0001 )
		first_te_sweep_subsection_span = 2*te_sweep_circle_radius*sin( te_sweep_subsection_arc_angle/2 * DEG_2_RAD ) * cos( subsection_te_sweep * DEG_2_RAD );
	else
		first_te_sweep_subsection_span = section_span;

	if( fabs(dihedral_subsection_arc_angle) > 0.0001 )	
		first_dihedral_subsection_span = 2*dihedral_circle_radius*sin( dihedral_subsection_arc_angle/2 * DEG_2_RAD );
	else
		first_dihedral_subsection_span = section_span;

	int num_largest_subsections = 0, num_moderate_subsections = 0, num_smallest_subsections = 0;
	double largest_subsection_arc_angle = 0.0, moderate_subsection_arc_angle = 0.0, smallest_subsection_arc_angle = 0.0;
	double largest_subsection_total_span = 0.0, moderate_subsection_total_span = 0.0, smallest_subsection_total_span = 0.0;
	int sweep_flag = SMALLEST, te_sweep_flag = SMALLEST, dihedral_flag = SMALLEST; 

	//determine the smallest, moderate and largest span-wise sub-sections
	if( (first_sweep_subsection_span < first_te_sweep_subsection_span) && (first_sweep_subsection_span < first_dihedral_subsection_span) )
	{
		num_smallest_subsections = num_sweep_subsections;
		smallest_subsection_arc_angle = sweep_subsection_arc_angle;
		smallest_subsection_total_span = sweep_span_chord.y();

		if(first_te_sweep_subsection_span < first_dihedral_subsection_span)
		{
			num_moderate_subsections = num_te_sweep_subsections;
			moderate_subsection_arc_angle = te_sweep_subsection_arc_angle;
			moderate_subsection_total_span = te_sweep_span_chord.y();
			te_sweep_flag = MODERATE;

			num_largest_subsections = num_dihedral_subsections;
			largest_subsection_arc_angle = dihedral_subsection_arc_angle;
			largest_subsection_total_span = sqrt( pow(dihedral_span_chord.y(), 2) + pow(dihedral_span_chord.x(), 2) );
			dihedral_flag = LARGEST;
		}
		else
		{
			num_moderate_subsections = num_dihedral_subsections;
			moderate_subsection_arc_angle = dihedral_subsection_arc_angle;
			moderate_subsection_total_span = sqrt( pow(dihedral_span_chord.y(), 2) + pow(dihedral_span_chord.x(), 2) );
			dihedral_flag = MODERATE;

			num_largest_subsections = num_te_sweep_subsections;
			largest_subsection_arc_angle = te_sweep_subsection_arc_angle;
			largest_subsection_total_span = te_sweep_span_chord.y();
			te_sweep_flag = LARGEST;
		}
	}
	else if( (first_te_sweep_subsection_span < first_sweep_subsection_span) && (first_te_sweep_subsection_span < first_dihedral_subsection_span) )
	{
		num_smallest_subsections = num_te_sweep_subsections;
		smallest_subsection_arc_angle = te_sweep_subsection_arc_angle;
		smallest_subsection_total_span = te_sweep_span_chord.y();

		if(first_sweep_subsection_span < first_dihedral_subsection_span)
		{
			num_moderate_subsections = num_sweep_subsections;
			moderate_subsection_arc_angle = sweep_subsection_arc_angle;
			moderate_subsection_total_span = sweep_span_chord.y();
			sweep_flag = MODERATE;

			num_largest_subsections = num_dihedral_subsections;
			largest_subsection_arc_angle = dihedral_subsection_arc_angle;
			largest_subsection_total_span = sqrt( pow(dihedral_span_chord.y(), 2) + pow(dihedral_span_chord.x(), 2) );
			dihedral_flag = LARGEST;
		}
		else
		{
			num_moderate_subsections = num_dihedral_subsections;
			moderate_subsection_arc_angle = dihedral_subsection_arc_angle;
			moderate_subsection_total_span = sqrt( pow(dihedral_span_chord.y(), 2) + pow(dihedral_span_chord.x(), 2) );
			dihedral_flag = MODERATE;

			num_largest_subsections = num_sweep_subsections;
			largest_subsection_arc_angle = sweep_subsection_arc_angle;
			largest_subsection_total_span = sweep_span_chord.y();
			sweep_flag = LARGEST;
		}
	}
	else
	{
		num_smallest_subsections = num_dihedral_subsections;
		smallest_subsection_arc_angle = dihedral_subsection_arc_angle;
		smallest_subsection_total_span = sqrt( pow(dihedral_span_chord.y(), 2) + pow(dihedral_span_chord.x(), 2) );

		if(first_sweep_subsection_span < first_te_sweep_subsection_span)
		{
			num_moderate_subsections = num_sweep_subsections;
			moderate_subsection_arc_angle = sweep_subsection_arc_angle;
			moderate_subsection_total_span = sweep_span_chord.y();
			sweep_flag = MODERATE;

			num_largest_subsections = num_te_sweep_subsections;
			largest_subsection_arc_angle = te_sweep_subsection_arc_angle;
			largest_subsection_total_span = te_sweep_span_chord.y();
			te_sweep_flag = LARGEST;
		}
		else
		{
			num_moderate_subsections = num_te_sweep_subsections;
			moderate_subsection_arc_angle = te_sweep_subsection_arc_angle;
			moderate_subsection_total_span = te_sweep_span_chord.y();
			te_sweep_flag = MODERATE;

			num_largest_subsections = num_sweep_subsections;
			largest_subsection_arc_angle = sweep_subsection_arc_angle;
			largest_subsection_total_span = sweep_span_chord.y();
			sweep_flag = LARGEST;
		}
	}

	//summing variables used to keep track of span as the subsections are added
	double total_smallest_subsection_span = 0, total_moderate_subsection_span = 0, total_largest_subsection_span = 0;

	//set total span values for all combinations of the number of moderate and largest subsections containing 0 sub-sections 
	if( num_smallest_subsections != 0  )
	{
		if( num_moderate_subsections == 0 && num_largest_subsections == 0 )
		{
			num_moderate_subsections = 1;
			moderate_subsection_total_span = smallest_subsection_total_span;
			total_moderate_subsection_span = moderate_subsection_total_span;

			num_largest_subsections = 1;
			largest_subsection_total_span = smallest_subsection_total_span;	
			total_largest_subsection_span = largest_subsection_total_span;
		}
		else if( num_moderate_subsections == 0 && num_largest_subsections != 0 )
		{
			num_moderate_subsections = 1;
			if(largest_subsection_total_span > smallest_subsection_total_span)
				moderate_subsection_total_span = largest_subsection_total_span;
			else
				moderate_subsection_total_span = smallest_subsection_total_span;

			total_moderate_subsection_span = moderate_subsection_total_span;
		}
		else if( num_moderate_subsections != 0 && num_largest_subsections == 0 )
		{
			num_largest_subsections = 1;

			if(moderate_subsection_total_span > smallest_subsection_total_span)
				largest_subsection_total_span = moderate_subsection_total_span;
			else
				largest_subsection_total_span = smallest_subsection_total_span;

			total_largest_subsection_span = largest_subsection_total_span;
		}

	}

	//find largest total span to mark where the root fillet ends (span-wise)
	if(smallest_subsection_total_span > moderate_subsection_total_span && smallest_subsection_total_span > largest_subsection_total_span)
		previous_root_span = smallest_subsection_total_span;
	else if(moderate_subsection_total_span > smallest_subsection_total_span && moderate_subsection_total_span > largest_subsection_total_span)
		previous_root_span = moderate_subsection_total_span;
	else
		previous_root_span = largest_subsection_total_span;

	int moderate_index = 0, largest_index = 0;	

	double front_offset = sweep_pivot_point.x() - sweep_circle_radius;
	double aft_offset = 0.0;

	//find the trailing edge offset
	if(section_sweep_offset + sects[0].tc > sects[0].rc)
		aft_offset = sects[0].rc - te_sweep_pivot_point.x() + te_sweep_circle_radius;
	else
		aft_offset = sects[0].rc - te_sweep_pivot_point.x() - te_sweep_circle_radius;

	double span_percentage_sum = 0.0;

	if(num_smallest_subsections != 0)
		dihedral_rot_starting_indices.append(1);
	else
		dihedral_rot_starting_indices.append(0);

	double smallest_subsection_span = 0.0, moderate_subsection_span = 0.0, largest_subsection_span = 0.0;
	smallest_subsection_span = (smallest_subsection_total_span/num_smallest_subsections);
	moderate_subsection_span = (moderate_subsection_total_span/num_moderate_subsections);
	largest_subsection_span = (largest_subsection_total_span/num_largest_subsections);

	double thickness_difference = sects[0].tipAf->get_thickness()->get() - sects[0].rootAf->get_thickness()->get(); 

	//generate the first root fillet sub-sections
	for(int i = 0; i < num_smallest_subsections; i++)
	{
		HwbSect wingsect;
		wingsect.driver = MS_S_TC_RC;
		double te_sweep = 0.0;

		wingsect.span = smallest_subsection_span;
		wingsect.twist = curr_twist_total + (section_twist - previous_twist) * (wingsect.span / section_span);
		curr_twist_total += (section_twist - previous_twist) * (wingsect.span / section_span);
		wingsect.twistLoc = section_twistloc;
		wingsect.sweepLoc = section_sweeploc;

		//keep track of percentage of span on current section
		span_percentage_sum += (wingsect.span/sects[0].span);

		//when a smallest sub-section passes a moderate sub-section the moderate sub-section parameter is changed accordingly
		if( (total_smallest_subsection_span + smallest_subsection_span) > (total_moderate_subsection_span + moderate_subsection_span) )
		{
			if(sweep_flag == MODERATE)
			{
				if( moderate_index >= num_moderate_subsections)
					wingsect.sweep = previous_sweep + subsection_sweep*(num_moderate_subsections+1);
				else
					wingsect.sweep = previous_sweep + (subsection_sweep*(moderate_index+1) + subsection_sweep*(moderate_index+2))/2;
			}	
			else if(te_sweep_flag == MODERATE)
			{
				if( moderate_index >= num_moderate_subsections)
					te_sweep = previous_te_sweep + subsection_te_sweep*(num_moderate_subsections+1);
				else
					te_sweep = previous_te_sweep + (subsection_te_sweep*(moderate_index+1) + subsection_te_sweep*(moderate_index+2))/2;
			}
			else
			{
				if( moderate_index >= num_moderate_subsections)
					wingsect.dihedral = previous_dihedral + subsection_dihedral*(num_moderate_subsections+1);
				else
					wingsect.dihedral = previous_dihedral + (subsection_dihedral*(moderate_index+1) + subsection_dihedral*(moderate_index+2))/2;
			}

				total_moderate_subsection_span += moderate_subsection_span;
				moderate_index++;
		}
		else
		{
			if(sweep_flag == MODERATE)
				wingsect.sweep = previous_sweep + subsection_sweep*(moderate_index+1);
			else if(te_sweep_flag == MODERATE)
				te_sweep = previous_te_sweep + subsection_te_sweep*(moderate_index+1);
			else
				wingsect.dihedral = previous_dihedral + subsection_dihedral*(moderate_index+1);
		}
		
		//when a smallest sub-section passes a largest sub-section the largest sub-section parameter is changed accordingly	
		if( (total_smallest_subsection_span + smallest_subsection_span) > (total_largest_subsection_span + largest_subsection_span) )
		{
			if(sweep_flag == LARGEST)
			{
				if( largest_index >= num_largest_subsections)
					wingsect.sweep = previous_sweep + subsection_sweep*(num_largest_subsections+1);
				else
					wingsect.sweep = previous_sweep + (subsection_sweep*(largest_index+1) + subsection_sweep*(largest_index+2))/2;
			}
			else if(te_sweep_flag == LARGEST)
			{
				if( largest_index >= num_largest_subsections)
					te_sweep = previous_te_sweep + subsection_te_sweep*(num_largest_subsections+1);
				else
					te_sweep = previous_te_sweep + (subsection_te_sweep*(largest_index+1) + subsection_te_sweep*(largest_index+2))/2;
			}
			else
			{
				if( largest_index >= num_largest_subsections)
					wingsect.dihedral = previous_dihedral + subsection_dihedral*(num_largest_subsections+1);
				else
					wingsect.dihedral = previous_dihedral + (subsection_dihedral*(largest_index+1) + subsection_dihedral*(largest_index+2))/2;
			}

			total_largest_subsection_span += largest_subsection_span;
			largest_index++;
		}
		else
		{
			if(sweep_flag == LARGEST)
				wingsect.sweep = previous_sweep + subsection_sweep*(largest_index+1);
			else if(te_sweep_flag == LARGEST)
				te_sweep = previous_te_sweep + subsection_te_sweep*(largest_index+1);
			else
				wingsect.dihedral = previous_dihedral + subsection_dihedral*(largest_index+1);
		}

		//set the smallest sub-section parameter
		if(sweep_flag == SMALLEST)	
			wingsect.sweep = previous_sweep + subsection_sweep*(i+1);	
		else if(te_sweep_flag == SMALLEST)
			te_sweep = previous_te_sweep + subsection_te_sweep*(i+1);
		else
			wingsect.dihedral = previous_dihedral + subsection_dihedral*(i+1);

		//sum the smallest sub-section span
		total_smallest_subsection_span += smallest_subsection_span;

		//determine root chord
		if(i == 0)
			wingsect.rc = section_rc - front_offset - aft_offset;
		else
			wingsect.rc = sub_sects[sub_sects.dimension()-1].tc;

		if(i == 0)
		{
			chord_lengths[0] = wingsect.rc;
		}

		double sub_sectional_sweep_offset = wingsect.span*tan(wingsect.sweep*DEG_2_RAD);
		double sub_sectional_te_sweep_offset = wingsect.span*tan(te_sweep*DEG_2_RAD);

		//set tip chord
		wingsect.tc = wingsect.rc - sub_sectional_sweep_offset - sub_sectional_te_sweep_offset;

		//set root airfoil
		if(i == 0)
		{
			Af* newrootaf = new Af(this);
			wingsect.rootAf = newrootaf;
			*(wingsect.rootAf) = *(sects[0].rootAf);
			wingsect.rootAf->sterilize_parms();
			sub_sect_foils.append(wingsect.rootAf);
		}
		else
			wingsect.rootAf = sub_sects[sub_sects.dimension() - 1].tipAf;

		double new_thickness = span_percentage_sum*thickness_difference + sects[0].rootAf->get_thickness()->get();

		//set tip airfoil
		Af* newaf = new Af( this );
		wingsect.tipAf = newaf;
		sub_sect_foils.append(newaf);
		wingsect.tipAf->init_script("wing_foil");
		*(wingsect.tipAf) = *(sects[0].rootAf);
		wingsect.tipAf->sterilize_parms();
		wingsect.tipAf->get_thickness()->set(new_thickness);
		wingsect.tipAf->generate_airfoil();
		wingsect.tipAf->load_name();

		wingsect.num_interp_xsecs = sects[0].num_interp_xsecs;

		wingsect.fillDependData();
		sub_sects.append( wingsect );

		num_subsections++;
		total_num_subsections++;
	}

	//check to see if there is any moderate sub-sections left
	if( (moderate_subsection_total_span > smallest_subsection_total_span) && (num_smallest_subsections > 0) )
	{
		// = Bridge subsection connects smallest subsections to moderate sub-sections = //
		HwbSect bridgesect;
		double te_sweep;
		bridgesect.driver = MS_S_TC_RC;

		if( ( total_moderate_subsection_span + moderate_subsection_span - smallest_subsection_total_span ) > 0.0001 )
		{
			bridgesect.span = total_moderate_subsection_span + moderate_subsection_span - smallest_subsection_total_span;
			bridgesect.twist = curr_twist_total + (section_twist - previous_twist) * (bridgesect.span / section_span);
			curr_twist_total += (section_twist - previous_twist) * (bridgesect.span / section_span);
			bridgesect.twistLoc = section_twistloc;
			bridgesect.sweepLoc = section_sweeploc;

			span_percentage_sum += (bridgesect.span/sects[0].span);

			// = Determine sweeps and dihedral for bridge subsection = //
			if(sweep_flag == SMALLEST)
				bridgesect.sweep = previous_sweep + subsection_sweep*num_sweep_subsections;
			else if(te_sweep_flag == SMALLEST)
				te_sweep = previous_te_sweep + subsection_te_sweep*num_te_sweep_subsections;
			else
				bridgesect.dihedral = previous_dihedral + subsection_dihedral*num_dihedral_subsections;

			if(sweep_flag == MODERATE)
				bridgesect.sweep = previous_sweep + subsection_sweep*(moderate_index+1);
			else if(te_sweep_flag == MODERATE)
				te_sweep = previous_te_sweep + subsection_te_sweep*(moderate_index+1);
			else
				bridgesect.dihedral = previous_dihedral + subsection_dihedral*(moderate_index+1);

			if( largest_subsection_total_span > smallest_subsection_total_span )
			{
				if( (smallest_subsection_total_span + bridgesect.span) > total_largest_subsection_span + largest_subsection_span )
				{
					if(sweep_flag == LARGEST)
						bridgesect.sweep = previous_sweep + (subsection_sweep*(largest_index+1) + subsection_sweep*(largest_index+2))/2;
					else if(te_sweep_flag == LARGEST)
						te_sweep = previous_te_sweep + (subsection_te_sweep*(largest_index+1) + subsection_te_sweep*(largest_index+2))/2;
					else
						bridgesect.dihedral = previous_dihedral + (subsection_dihedral*(largest_index+1) + subsection_dihedral*  									(largest_index+2))/2;

					total_largest_subsection_span += largest_subsection_span;
					largest_index++;	
				}
				else
				{
					if(sweep_flag == LARGEST)
						bridgesect.sweep = previous_sweep + subsection_sweep*(largest_index+1);
					else if(te_sweep_flag == LARGEST)
						te_sweep = previous_te_sweep + subsection_te_sweep*(largest_index+1);
					else
						bridgesect.dihedral = previous_dihedral + subsection_dihedral*(largest_index+1);
				}
			}

			//set bridge section root chord
			bridgesect.rc = sub_sects[sub_sects.dimension()-1].tc;

			double sub_sectional_sweep_offset = bridgesect.span*tan(bridgesect.sweep*DEG_2_RAD);
			double sub_sectional_te_sweep_offset = bridgesect.span*tan(te_sweep*DEG_2_RAD);

			//set bridge section tip chord
			bridgesect.tc = bridgesect.rc - sub_sectional_sweep_offset - sub_sectional_te_sweep_offset;

			double new_thickness = span_percentage_sum*thickness_difference + sects[0].rootAf->get_thickness()->get();

			// = Set airfoils for bridge subsection = //
			bridgesect.rootAf = sub_sects[sub_sects.dimension() - 1].tipAf;
			Af* newaf = new Af( this );
			bridgesect.tipAf = newaf;
			sub_sect_foils.append(newaf);
			bridgesect.tipAf->init_script("wing_foil");
			*(bridgesect.tipAf) = *(sects[0].rootAf);
			bridgesect.tipAf->sterilize_parms();
			bridgesect.tipAf->get_thickness()->set(new_thickness);
			bridgesect.tipAf->generate_airfoil();
			bridgesect.tipAf->load_name();

			bridgesect.num_interp_xsecs = sects[0].num_interp_xsecs;

			bridgesect.fillDependData();
			sub_sects.append( bridgesect );
		
			num_subsections++;
			total_num_subsections++;
			moderate_index++;	
			total_moderate_subsection_span += moderate_subsection_span;
		}	
		else
		{
			moderate_index++;	
			total_moderate_subsection_span += moderate_subsection_span;
		}

		//create remianing moderate sub-sections
		for(int k = moderate_index; k < num_moderate_subsections; k++)
		{
			HwbSect wingsect;
			wingsect.driver = MS_S_TC_RC;

			wingsect.span = moderate_subsection_span;
			wingsect.twist = curr_twist_total + (section_twist - previous_twist) * (wingsect.span / section_span);
			curr_twist_total += (section_twist - previous_twist) * (wingsect.span / section_span);
			wingsect.twistLoc = section_twistloc;
			wingsect.sweepLoc = section_sweeploc;
			span_percentage_sum += (wingsect.span/sects[0].span);

			// = Determine sweeps and dihedral for subsections = //
			if(sweep_flag == SMALLEST)
				wingsect.sweep = previous_sweep + subsection_sweep*num_sweep_subsections;
			else if(te_sweep_flag == SMALLEST)
				te_sweep = previous_te_sweep + subsection_te_sweep*num_te_sweep_subsections;
			else
				wingsect.dihedral = previous_dihedral + subsection_dihedral*num_dihedral_subsections;

			if(sweep_flag == MODERATE)
				wingsect.sweep = previous_sweep + subsection_sweep*(k+1);
			else if(te_sweep_flag == MODERATE)
				te_sweep = previous_te_sweep + subsection_te_sweep*(k+1);
			else
				wingsect.dihedral = previous_dihedral + subsection_dihedral*(k+1);

			
			if( largest_subsection_total_span > smallest_subsection_total_span )
			{
				if( total_moderate_subsection_span + moderate_subsection_span > total_largest_subsection_span + largest_subsection_span )
				{
					if(sweep_flag == LARGEST)
						wingsect.sweep = previous_sweep + (subsection_sweep*(largest_index+1) + subsection_sweep* 
									(largest_index+2))/2;
					else if(te_sweep_flag == LARGEST)
						te_sweep = previous_te_sweep + (subsection_te_sweep*(largest_index+1) + subsection_te_sweep* 								(largest_index+2))/2;
					else
						wingsect.dihedral = previous_dihedral + (subsection_dihedral*(largest_index+1) + subsection_dihedral* 								(largest_index+2))/2;

					total_largest_subsection_span += largest_subsection_span;
					largest_index++;
				}
				else
				{
					if(sweep_flag == LARGEST)
						wingsect.sweep = previous_sweep + subsection_sweep*(largest_index+1);
					else if(te_sweep_flag == LARGEST)
						te_sweep = previous_te_sweep + subsection_te_sweep*(largest_index+1);
					else
						wingsect.dihedral = previous_dihedral + subsection_dihedral*(largest_index+1);
				}
			}
			else
			{
				largest_index++;	
				total_largest_subsection_span += largest_subsection_span;
			}

			//set root chord
			wingsect.rc = sub_sects[sub_sects.dimension()-1].tc;

			double sub_sectional_sweep_offset = wingsect.span*tan(wingsect.sweep*DEG_2_RAD);
			double sub_sectional_te_sweep_offset = wingsect.span*tan(te_sweep*DEG_2_RAD);

			//set tip chord
			wingsect.tc = wingsect.rc - sub_sectional_sweep_offset - sub_sectional_te_sweep_offset;

			double new_thickness = span_percentage_sum*thickness_difference + sects[0].rootAf->get_thickness()->get();

			// = Set airfoils for moderate sub-sections = //
			wingsect.rootAf = sub_sects[sub_sects.dimension() - 1].tipAf;
			Af* newaf = new Af( this );
			wingsect.tipAf = newaf;
			sub_sect_foils.append( newaf );
			wingsect.tipAf->init_script("wing_foil");
			*(wingsect.tipAf) = *(sects[0].rootAf);
			wingsect.tipAf->sterilize_parms();
			wingsect.tipAf->get_thickness()->set(new_thickness);
			wingsect.tipAf->generate_airfoil();
			wingsect.tipAf->load_name();

			wingsect.num_interp_xsecs = sects[0].num_interp_xsecs;

			wingsect.fillDependData();
			sub_sects.append( wingsect );
	
			num_subsections++;
			total_num_subsections++;
		}

		//check for remaining largest sub-sections
		if( largest_subsection_total_span > moderate_subsection_total_span )
		{
			HwbSect bridgesect2;
			bridgesect2.driver = MS_S_TC_RC;

			if( ( total_largest_subsection_span + largest_subsection_span - moderate_subsection_total_span ) > 0.0001 )
			{
				bridgesect2.span = total_largest_subsection_span + largest_subsection_span - moderate_subsection_total_span;
				bridgesect2.twist = curr_twist_total + (section_twist - previous_twist) * (bridgesect2.span / section_span);
				curr_twist_total += (section_twist - previous_twist) * (bridgesect2.span / section_span);
				bridgesect2.twistLoc = section_twistloc;
				bridgesect2.sweepLoc = section_sweeploc;
		
				span_percentage_sum += (bridgesect2.span/sects[0].span);

				// = Determine sweeps and dihedral for bridge subsection = //
				if(sweep_flag == SMALLEST)
					bridgesect2.sweep = previous_sweep + subsection_sweep*num_sweep_subsections;
				else if(te_sweep_flag == SMALLEST)
					te_sweep = previous_te_sweep + subsection_te_sweep*num_te_sweep_subsections;
				else
					bridgesect2.dihedral = previous_dihedral + subsection_dihedral*num_dihedral_subsections;

				if(sweep_flag == MODERATE)
					bridgesect2.sweep = previous_sweep + subsection_sweep*num_sweep_subsections;
				else if(te_sweep_flag == MODERATE)
					te_sweep = previous_te_sweep + subsection_te_sweep*num_te_sweep_subsections;
				else
					bridgesect2.dihedral = previous_dihedral + subsection_dihedral*num_dihedral_subsections;

				if(sweep_flag == LARGEST)
					bridgesect2.sweep = previous_sweep + subsection_sweep*(largest_index+1);
				else if(te_sweep_flag == MODERATE)
					te_sweep = previous_te_sweep + subsection_te_sweep*(largest_index+1);
				else
					bridgesect2.dihedral = previous_dihedral + subsection_dihedral*(largest_index+1);

				//set bridge root chord
				bridgesect2.rc = sub_sects[sub_sects.dimension()-1].tc;

				double sub_sectional_sweep_offset = bridgesect2.span*tan(bridgesect2.sweep*DEG_2_RAD);
				double sub_sectional_te_sweep_offset = bridgesect2.span*tan(te_sweep*DEG_2_RAD);

				bridgesect2.tc = bridgesect2.rc - sub_sectional_sweep_offset - sub_sectional_te_sweep_offset;

				//set bridge tip chord
				double new_thickness = span_percentage_sum*thickness_difference + sects[0].rootAf->get_thickness()->get();

				// = Set airfoils for bridge subsection = //
				bridgesect2.rootAf = sub_sects[sub_sects.dimension() - 1].tipAf;
				Af* newaf = new Af( this );
				bridgesect2.tipAf = newaf;
				sub_sect_foils.append(newaf);
				bridgesect2.tipAf->init_script("wing_foil");
				*(bridgesect2.tipAf) = *(sects[0].rootAf);
				bridgesect2.tipAf->sterilize_parms();
				bridgesect2.tipAf->get_thickness()->set(new_thickness);
				bridgesect2.tipAf->generate_airfoil();
				bridgesect2.tipAf->load_name();

				bridgesect2.num_interp_xsecs = sects[0].num_interp_xsecs;

				bridgesect2.fillDependData();
				sub_sects.append( bridgesect2 );

				num_subsections++;
				total_num_subsections++;
				largest_index++;
				total_largest_subsection_span += largest_subsection_span;

			}
			else
			{
				largest_index++;	
				total_largest_subsection_span += largest_subsection_span;
			}

			//create remaining largest sub-sections
			for(int k = largest_index; k < num_largest_subsections; k++)
			{
				HwbSect wingsect;
				wingsect.driver = MS_S_TC_RC;

				wingsect.span = largest_subsection_span;
				wingsect.twist = curr_twist_total + (section_twist - previous_twist) * (wingsect.span / section_span);
				curr_twist_total += (section_twist - previous_twist) * (wingsect.span / section_span);
				wingsect.twistLoc = section_twistloc;
				wingsect.sweepLoc = section_sweeploc;

				span_percentage_sum += (wingsect.span/sects[0].span);

				// = Determine sweeps and dihedral for subsections = //
				if(sweep_flag == SMALLEST)
					wingsect.sweep = previous_sweep + subsection_sweep*num_sweep_subsections;
				else if(te_sweep_flag == SMALLEST)
					te_sweep = previous_te_sweep + subsection_te_sweep*num_te_sweep_subsections;
				else
					wingsect.dihedral = previous_dihedral + subsection_dihedral*num_dihedral_subsections;

				if(sweep_flag == MODERATE)
					wingsect.sweep = previous_sweep + subsection_sweep*num_sweep_subsections;
				else if(te_sweep_flag == MODERATE)
					te_sweep = previous_te_sweep + subsection_te_sweep*num_te_sweep_subsections;
				else
					wingsect.dihedral = previous_dihedral + subsection_dihedral*num_dihedral_subsections;

				if(sweep_flag == LARGEST)
					wingsect.sweep = previous_sweep + subsection_sweep*(k+1);
				else if(te_sweep_flag == LARGEST)
					te_sweep = previous_te_sweep + subsection_te_sweep*(k+1);
				else
					wingsect.dihedral = previous_dihedral + subsection_dihedral*(k+1);

				//set root chord
				wingsect.rc = sub_sects[sub_sects.dimension()-1].tc;

				double sub_sectional_sweep_offset = wingsect.span*tan(wingsect.sweep*DEG_2_RAD);
				double sub_sectional_te_sweep_offset = wingsect.span*tan(te_sweep*DEG_2_RAD);

				//set tip chord
				wingsect.tc = wingsect.rc - sub_sectional_sweep_offset - sub_sectional_te_sweep_offset;

				double new_thickness = span_percentage_sum*thickness_difference + sects[0].rootAf->get_thickness()->get();

				// = Set airfoils for subsections = //
				wingsect.rootAf = sub_sects[sub_sects.dimension() - 1].tipAf;
				Af* newaf = new Af( this );
				wingsect.tipAf = newaf;
				sub_sect_foils.append(newaf);
				wingsect.tipAf->init_script("wing_foil");
				*(wingsect.tipAf) = *(sects[0].rootAf);
				wingsect.tipAf->sterilize_parms();
				wingsect.tipAf->get_thickness()->set(new_thickness);
				wingsect.tipAf->generate_airfoil();
				wingsect.tipAf->load_name();

				wingsect.num_interp_xsecs = sects[0].num_interp_xsecs;

				wingsect.fillDependData();
				sub_sects.append( wingsect );
	
				num_subsections++;
				total_num_subsections++;
			}
		}			
	}
	else if( largest_subsection_total_span > smallest_subsection_total_span && (num_smallest_subsections > 0) )
	{	
		// = Determine sweeps and dihedral for bridge subsection = //
		// = Bridge subsection connects smallest and moderate subsections to largest subsections = //
		double te_sweep;
		HwbSect bridgesect;
		bridgesect.driver = MS_S_TC_RC;

		if( ( total_largest_subsection_span + largest_subsection_span - moderate_subsection_total_span ) > 0.0001 )
		{
			bridgesect.span = total_largest_subsection_span + largest_subsection_span - smallest_subsection_total_span;
			bridgesect.twist = curr_twist_total + (section_twist - previous_twist) * (bridgesect.span / section_span);
			curr_twist_total += (section_twist - previous_twist) * (bridgesect.span / section_span);
			bridgesect.twistLoc = section_twistloc;
			bridgesect.sweepLoc = section_sweeploc;

			span_percentage_sum += (bridgesect.span/sects[0].span);

			// = Determine sweeps and dihedral for bridge subsection = //
			if(sweep_flag == SMALLEST)
				bridgesect.sweep = previous_sweep + subsection_sweep*num_sweep_subsections;
			else if(te_sweep_flag == SMALLEST)
				te_sweep = previous_te_sweep + subsection_te_sweep*num_te_sweep_subsections;
			else
				bridgesect.dihedral = previous_dihedral + subsection_dihedral*num_dihedral_subsections;

			if(sweep_flag == MODERATE)
				bridgesect.sweep = previous_sweep + subsection_sweep*num_sweep_subsections;
			else if(te_sweep_flag == MODERATE)
				te_sweep = previous_te_sweep + subsection_te_sweep*num_te_sweep_subsections;
			else
				bridgesect.dihedral = previous_dihedral + subsection_dihedral*num_dihedral_subsections;

			if(sweep_flag == LARGEST)
				bridgesect.sweep = previous_sweep + subsection_sweep*(largest_index+1);
			else if(te_sweep_flag == LARGEST)
				te_sweep = previous_te_sweep + subsection_te_sweep*(largest_index+1);
			else
				bridgesect.dihedral = previous_dihedral + subsection_dihedral*(largest_index+1);

			//set the root chord
			bridgesect.rc = sub_sects[sub_sects.dimension()-1].tc;

			double sub_sectional_sweep_offset = bridgesect.span*tan(bridgesect.sweep*DEG_2_RAD);
			double sub_sectional_te_sweep_offset = bridgesect.span*tan(te_sweep*DEG_2_RAD);

			bridgesect.tc = bridgesect.rc - sub_sectional_sweep_offset - sub_sectional_te_sweep_offset;

			double new_thickness = span_percentage_sum*thickness_difference + sects[0].rootAf->get_thickness()->get();

			// = Set airfoils for bridge subsection = //
			bridgesect.rootAf = sub_sects[sub_sects.dimension() - 1].tipAf;
			Af* newaf = new Af( this );
			bridgesect.tipAf = new Af( this );
			sub_sect_foils.append(newaf);
			bridgesect.tipAf->init_script("wing_foil");
			*(bridgesect.tipAf) = *(sects[0].rootAf);
			bridgesect.tipAf->sterilize_parms();
			bridgesect.tipAf->get_thickness()->set(new_thickness);
			bridgesect.tipAf->generate_airfoil();
			bridgesect.tipAf->load_name();

			bridgesect.num_interp_xsecs = sects[0].num_interp_xsecs;

			bridgesect.fillDependData();
			sub_sects.append( bridgesect );
		
			num_subsections++;
			total_num_subsections++;
			largest_index++;
			total_largest_subsection_span += largest_subsection_span;
		}
		else
		{
			largest_index++;	
			total_largest_subsection_span += largest_subsection_span;
		}

		//create remaining largest sub-sections
		for(int k = largest_index; k < num_largest_subsections; k++)
		{
			HwbSect wingsect;
			wingsect.driver = MS_S_TC_RC;

			wingsect.span = largest_subsection_span;
			wingsect.twist = curr_twist_total + (section_twist - previous_twist) * (wingsect.span / section_span);
			curr_twist_total += (section_twist - previous_twist) * (wingsect.span / section_span);
			wingsect.twistLoc = section_twistloc;
			wingsect.sweepLoc = section_sweeploc;

			span_percentage_sum += (wingsect.span/sects[0].span);

			// = Determine sweep and dihedral for bridge subsection = //
			if(sweep_flag == SMALLEST)
				wingsect.sweep = previous_sweep + subsection_sweep*num_sweep_subsections;
			else if(te_sweep_flag == SMALLEST)
				te_sweep = previous_te_sweep + subsection_te_sweep*num_te_sweep_subsections;
			else
				wingsect.dihedral = previous_dihedral + subsection_dihedral*num_dihedral_subsections;

			if(sweep_flag == MODERATE)
				wingsect.sweep = previous_sweep + subsection_sweep*num_sweep_subsections;
			else if(te_sweep_flag == MODERATE)
				te_sweep = previous_te_sweep + subsection_te_sweep*num_te_sweep_subsections;
			else
				wingsect.dihedral = previous_dihedral + subsection_dihedral*num_dihedral_subsections;

			if(sweep_flag == LARGEST)
				wingsect.sweep = previous_sweep + subsection_sweep*(k+1);
			else if(te_sweep_flag == LARGEST)
				te_sweep = previous_te_sweep + subsection_te_sweep*(k+1);
			else
				wingsect.dihedral = previous_dihedral + subsection_dihedral*(k+1);
	
			wingsect.rc = sub_sects[sub_sects.dimension()-1].tc;

			double sub_sectional_sweep_offset = wingsect.span*tan(wingsect.sweep*DEG_2_RAD);
			double sub_sectional_te_sweep_offset = wingsect.span*tan(te_sweep*DEG_2_RAD);

			wingsect.tc = wingsect.rc - sub_sectional_sweep_offset - sub_sectional_te_sweep_offset;

			double new_thickness = span_percentage_sum*thickness_difference + sects[0].rootAf->get_thickness()->get();

			// = Set airfoils for largest sub-sections = //
			wingsect.rootAf = sub_sects[sub_sects.dimension() - 1].tipAf;
			Af* newaf = new Af( this );
			wingsect.tipAf = newaf;
			sub_sect_foils.append(newaf);
			wingsect.tipAf->init_script("wing_foil");
			*(wingsect.tipAf) = *(sects[0].rootAf);
			wingsect.tipAf->sterilize_parms();
			wingsect.tipAf->get_thickness()->set(new_thickness);
			wingsect.tipAf->generate_airfoil();
			wingsect.tipAf->load_name();

			wingsect.num_interp_xsecs = sects[0].num_interp_xsecs;

			wingsect.fillDependData();
			sub_sects.append( wingsect );
			num_subsections++;
			total_num_subsections++;
		}
	}

	previous_sweep = section_sweep;
	previous_te_sweep = section_te_sweep;
	previous_dihedral = section_dihedral;

	//main loop begin: main section -> tip fillet -> root fillet -> repeat
	for(int i = 1; i < num_sects; i++)
	{
		dyn_array< HwbSect > temp_subsects;
		dyn_array< double > te_sweep_list;

		double next_sweep_offset = sects[i].span*tan(atan(sects[i].tan_sweep_at(0.0,getSymCode())));
		double next_dihedral_offset = sects[i].span*sin(sects[i].dihedral*DEG_2_RAD);
		double next_dihedral_offset_x = sects[i].span*cos(sects[i].dihedral*DEG_2_RAD);
		double next_te_sweep_offset = (sects[i].rc - sects[i].tc - next_sweep_offset);
		double next_te_sweep = atan( next_te_sweep_offset/sects[i].span )*RAD_2_DEG;

		section_sweep = ( atan(sects[i-1].tan_sweep_at(0.0,getSymCode()))*RAD_2_DEG + 
						  atan(sects[i].tan_sweep_at(0.0,getSymCode()))*RAD_2_DEG ) / 2;
		section_dihedral = (sects[i-1].dihedral + sects[i].dihedral) / 2;
		section_te_sweep = (previous_te_sweep + next_te_sweep) / 2;

		// = Find number of subsections for sweeps and dihedral = //
		num_sweep_subsections = (int)ceil( fabs(section_sweep - previous_sweep) / sweep_degrees_perseg[i]->iget() );
		num_te_sweep_subsections = (int)ceil( fabs(section_te_sweep - previous_te_sweep) / tesweep_degrees_perseg[i]->iget() );
		num_dihedral_subsections = (int)ceil( fabs(section_dihedral - previous_dihedral) / dihedral_degrees_perseg[i]->iget() );
			
		// = Retrieve span percentage of subsections for sweeps and dihedral = //
		sweep_edge_length_percentage = sweep_percent_edge_lengths[i]->get();
		te_sweep_edge_length_percentage = tesweep_percent_edge_lengths[i]->get();
		dihedral_edge_length_percentage = dihedral_percent_edge_lengths[i]->get();

		if( fabs( section_sweep - previous_sweep ) < 0.0001 )
			sweep_edge_length_percentage = 0.0;
		
		if( fabs( section_te_sweep - previous_te_sweep ) < 0.0001 )
			te_sweep_edge_length_percentage = 0.0;

		if( fabs( section_dihedral - previous_dihedral ) < 0.0001 )
			dihedral_edge_length_percentage = 0.0;

		double other_sweep_slope = 0.0;
		double other_te_sweep_slope = 0.0;
		double other_dihedral_slope = 0.0;
		double other_ortho_sweep_slope = 0.0; 
		double other_ortho_te_sweep_slope = 0.0;
		double other_ortho_dihedral_slope = 0.0;
		double other_sweep_intercept = 0.0;
		double other_te_sweep_intercept = 0.0;
		double other_dihedral_intercept = 0.0;
		double other_ortho_sweep_intercept = 0.0; 
		double other_ortho_te_sweep_intercept = 0.0;
		double other_ortho_dihedral_intercept = 0.0;
		double tip_sweep_arc_angle = 0.0, tip_te_sweep_arc_angle = 0.0, tip_dihedral_arc_angle = 0.0; 
		double root_sweep_arc_angle = 0.0, root_te_sweep_arc_angle = 0.0, root_dihedral_arc_angle = 0.0;
		vec2d sweep_chord_fillet_intersection;		
		vec2d te_sweep_chord_fillet_intersection;
		vec2d dihedral_chord_fillet_intersection;		
		sweep_slope = 0;
		te_sweep_slope = 0;
		dihedral_slope = 0;
		ortho_sweep_slope = 0;
		ortho_te_sweep_slope = 0;
		ortho_dihedral_slope = 0;
		
		//find length of leading edges adjacent to current chord
		double sweep_edge_dist1 = sqrt( pow(sects[i-1].span, 2) + pow(section_sweep_offset, 2) );
		double sweep_edge_dist2 = sqrt( pow(sects[i].span, 2) + pow(next_sweep_offset, 2) );
		double te_sweep_edge_dist1 = sqrt( pow(sects[i-1].span, 2) + pow(section_te_sweep_offset, 2) );
		double te_sweep_edge_dist2 = sqrt( pow(sects[i].span, 2) + pow(next_te_sweep_offset, 2) );
		double dihedral_edge_dist1 = sects[i-1].span;
		double dihedral_edge_dist2 = sects[i].span;
		bool sweep_previous_edge = false, te_sweep_previous_edge = false, dihedral_previous_edge = false;
		double tip_starting_span = 0;

		vec2d other_sweep_span_chord, other_te_sweep_span_chord, other_dihedral_span_chord;

		double current_section_span = 0;

		if(sects[i-1].span < sects[i].span)
			current_section_span = sects[i-1].span;
		else
			current_section_span = sects[i].span;

		double root_thickness = sects[i].rootAf->get_thickness()->get()*sects[i].rc;
		double prev_root_thickness = sects[i-1].rootAf->get_thickness()->get()*sects[i-1].rc;
		double tip_thickness = sects[i].tipAf->get_thickness()->get()*sects[i].tc;

		if(num_sweep_subsections > 0 && sweep_edge_length_percentage > 0.00001)
		{
			double dihedral_subsection = 0;
			double span_subsection = current_section_span*sweep_edge_length_percentage/num_sweep_subsections;
			double radius_dihed_rot = root_thickness - (root_thickness - prev_root_thickness)*(span_subsection*2/current_section_span);

			double ortho_deg1 = 90 - section_dihedral;
			double ortho_deg2 = 0;
	
			if(num_dihedral_subsections > 0 && dihedral_edge_length_percentage > 0.00001)
			{
				double dihedral_span_subsection = current_section_span*dihedral_edge_length_percentage/num_dihedral_subsections;
				dihedral_subsection = (section_dihedral - previous_dihedral)/(num_dihedral_subsections + 1);
	
				if(span_subsection < dihedral_span_subsection)
					ortho_deg2 = 90 - (section_dihedral - dihedral_subsection);
				else
					ortho_deg2 = 90 - (section_dihedral - dihedral_subsection*3/2);
			}
			else
			{
				dihedral_subsection = previous_dihedral;
				ortho_deg2 = 90 - dihedral_subsection;
			}

			double x1 = root_thickness*cos(ortho_deg1*DEG_2_RAD);
			double x2 = span_subsection + radius_dihed_rot*cos(ortho_deg2*DEG_2_RAD);	

			if(x2 < x1 && sects[i].dihedRotFlag)
				sweep_edge_length_percentage = 0.0;
	
			radius_dihed_rot = root_thickness - (root_thickness - tip_thickness)*(span_subsection*2/current_section_span);

			if(num_dihedral_subsections > 0 && dihedral_edge_length_percentage > 0.00001)
			{
				double dihedral_span_subsection = current_section_span*dihedral_edge_length_percentage/num_dihedral_subsections;
				dihedral_subsection = (sects[i].dihedral - section_dihedral)/(num_dihedral_subsections + 1);
	
				if(span_subsection < dihedral_span_subsection)
					ortho_deg2 = 90 - (section_dihedral + dihedral_subsection);
				else
					ortho_deg2 = 90 - (section_dihedral + dihedral_subsection*3/2);
			}
			else
			{
				dihedral_subsection = sects[i].dihedral;
				ortho_deg2 = 90 - dihedral_subsection;
			}

			x1 = span_subsection + root_thickness*cos(ortho_deg1*DEG_2_RAD);
			x2 = radius_dihed_rot*cos(ortho_deg2*DEG_2_RAD);	

			if(x2 > x1 && sects[i].dihedRotFlag)
				sweep_edge_length_percentage = 0.0;
		}

		if(num_te_sweep_subsections > 0 && te_sweep_edge_length_percentage > 0.00001)
		{
			double dihedral_subsection = 0;
			double span_subsection = current_section_span*te_sweep_edge_length_percentage/num_te_sweep_subsections;
			double radius_dihed_rot = root_thickness - (root_thickness - prev_root_thickness)*(span_subsection*2/current_section_span);

			double ortho_deg1 = 90 - section_dihedral;
			double ortho_deg2 = 0;

			if(num_dihedral_subsections > 0 && dihedral_edge_length_percentage > 0.00001)
			{
				double dihedral_span_subsection = current_section_span*dihedral_edge_length_percentage/num_dihedral_subsections;
				dihedral_subsection = (section_dihedral - previous_dihedral)/(num_dihedral_subsections + 1);

				if(span_subsection < dihedral_span_subsection)
					ortho_deg2 = 90 - (section_dihedral - dihedral_subsection);
				else
					ortho_deg2 = 90 - (section_dihedral - dihedral_subsection*3/2);
			}
			else
			{
				dihedral_subsection = previous_dihedral;
				ortho_deg2 = 90 - dihedral_subsection;
			}

			double x1 = root_thickness*cos(ortho_deg1*DEG_2_RAD);
			double x2 = span_subsection + radius_dihed_rot*cos(ortho_deg2*DEG_2_RAD);

			if(x2 < x1 && sects[i].dihedRotFlag)
				te_sweep_edge_length_percentage = 0.0;

			radius_dihed_rot = root_thickness - (root_thickness - tip_thickness)*(span_subsection*2/current_section_span);

			if(num_dihedral_subsections > 0 && dihedral_edge_length_percentage > 0.00001)
			{
				double dihedral_span_subsection = current_section_span*dihedral_edge_length_percentage/num_dihedral_subsections;
				dihedral_subsection = (sects[i].dihedral - section_dihedral)/(num_dihedral_subsections + 1);

				if(span_subsection < dihedral_span_subsection)
					ortho_deg2 = 90 - (section_dihedral + dihedral_subsection);
				else
					ortho_deg2 = 90 - (section_dihedral + dihedral_subsection*3/2);
			}
			else
			{
				dihedral_subsection = sects[i].dihedral;
				ortho_deg2 = 90 - dihedral_subsection;
			}

			x1 = span_subsection + root_thickness*cos(ortho_deg1*DEG_2_RAD);
			x2 = radius_dihed_rot*cos(ortho_deg2*DEG_2_RAD);

			if(x2 > x1 && sects[i].dihedRotFlag)
				te_sweep_edge_length_percentage = 0.0;
		}

		if(num_dihedral_subsections > 0  && dihedral_edge_length_percentage > 0.00001)
		{
			double dihedral_subsection = (section_dihedral - previous_dihedral)/(num_dihedral_subsections + 1);
			double span_subsection = current_section_span*dihedral_edge_length_percentage/num_dihedral_subsections;
			double radius_dihed_rot = root_thickness - (root_thickness - prev_root_thickness)*(span_subsection/current_section_span);

			double ortho_deg1 = 90 - section_dihedral;
			double ortho_deg2 = 90 - (section_dihedral - dihedral_subsection*3/2);

			double x1 = root_thickness*cos(ortho_deg1*DEG_2_RAD);
			double x2 = span_subsection + radius_dihed_rot*cos(ortho_deg2*DEG_2_RAD);

			if(x2 < x1 && sects[i].dihedRotFlag)
				dihedral_edge_length_percentage = 0.0;

			dihedral_subsection = (sects[i].dihedral - section_dihedral)/(num_dihedral_subsections + 1);
			radius_dihed_rot = root_thickness - (root_thickness - tip_thickness)*(span_subsection/current_section_span);

			ortho_deg2 = 90 - (section_dihedral + dihedral_subsection*3/2);

			x1 = span_subsection + root_thickness*cos(ortho_deg1*DEG_2_RAD);
			x2 = radius_dihed_rot*cos(ortho_deg2*DEG_2_RAD);

			if(x2 > x1 && sects[i].dihedRotFlag)
				dihedral_edge_length_percentage = 0.0;
		}

		//initialize fillet boundary for each edge
		sweep_span_chord.set_xy(total_sweep_offset, total_span);
		te_sweep_span_chord.set_xy(total_sweep_offset+section_tc, total_span);
		dihedral_span_chord.set_xy(total_dihedral_offset_x, total_dihedral_offset);

		if(fabs(sweep_edge_length_percentage) < 0.0001)
			num_sweep_subsections = 0;
	
		if(fabs(te_sweep_edge_length_percentage) < 0.0001)
			num_te_sweep_subsections = 0;

		if(fabs(dihedral_edge_length_percentage) < 0.0001)
			num_dihedral_subsections = 0;

		if(num_sweep_subsections != 0 || num_te_sweep_subsections != 0 || num_dihedral_subsections != 0)
		{
			//determine longer leading edge
			if(sweep_edge_dist1 > sweep_edge_dist2)
			{
				sweep_span_chord.set_xy( next_sweep_offset*sweep_edge_length_percentage, sects[i].span*sweep_edge_length_percentage );
				sweep_span_chord.set_xy( total_sweep_offset + sweep_span_chord.x() , total_span + sweep_span_chord.y() );

				if(section_sweep_offset != 0)
					other_sweep_slope = sects[i-1].span/section_sweep_offset;
			
				if(next_sweep_offset != 0)
					sweep_slope = sects[i].span/next_sweep_offset;
			}
			else
			{
				sweep_span_chord.set_xy( section_sweep_offset*sweep_edge_length_percentage, sects[i-1].span*sweep_edge_length_percentage );
				sweep_span_chord.set_xy( total_sweep_offset - sweep_span_chord.x() , total_span - sweep_span_chord.y() );

				if(next_sweep_offset != 0)
					other_sweep_slope = sects[i].span/next_sweep_offset;
	
				if(section_sweep_offset != 0)
					sweep_slope = sects[i-1].span/section_sweep_offset;

				sweep_previous_edge = true;
			}

			//find length of trailing edges adjacent to current chord


			//determine longer trailing edge
			if(te_sweep_edge_dist1 > te_sweep_edge_dist2)
			{
				te_sweep_span_chord.set_xy( next_te_sweep_offset*te_sweep_edge_length_percentage, sects[i].span*te_sweep_edge_length_percentage );
				te_sweep_span_chord.set_xy( sects[0].rc - total_te_sweep_offset - te_sweep_span_chord.x() , total_span + te_sweep_span_chord.y() );

				if(section_te_sweep_offset != 0)
					other_te_sweep_slope = sects[i-1].span/(-1*section_te_sweep_offset);

				if(next_te_sweep_offset != 0)
					te_sweep_slope = sects[i].span/(-1*next_te_sweep_offset);
			}
			else
			{
				te_sweep_span_chord.set_xy( section_te_sweep_offset*te_sweep_edge_length_percentage, sects[i-1].span*te_sweep_edge_length_percentage );
				te_sweep_span_chord.set_xy( sects[0].rc - total_te_sweep_offset + te_sweep_span_chord.x() , total_span - te_sweep_span_chord.y() );

				if(next_te_sweep_offset != 0)
					other_te_sweep_slope = sects[i].span/(-1*next_te_sweep_offset);
			
				if(section_te_sweep_offset != 0)
					te_sweep_slope = sects[i-1].span/(-1*section_te_sweep_offset);

				te_sweep_previous_edge = true;
			}

			//determine longer belly edge
			if(dihedral_edge_dist1 > dihedral_edge_dist2)
			{
				dihedral_span_chord.set_xy( sects[i].span*cos(sects[i].dihedral*DEG_2_RAD)*dihedral_edge_length_percentage, 				next_dihedral_offset*dihedral_edge_length_percentage);
				dihedral_span_chord.set_xy( total_dihedral_offset_x + dihedral_span_chord.x(), total_dihedral_offset + dihedral_span_chord.y() );

				if(section_dihedral_offset_x != 0)
					other_dihedral_slope = section_dihedral_offset/section_dihedral_offset_x;

				if(next_dihedral_offset_x != 0)
					dihedral_slope = next_dihedral_offset/next_dihedral_offset_x;
			}
			else
			{
				dihedral_span_chord.set_xy( sects[i-1].span*cos(sects[i-1].dihedral*DEG_2_RAD)*dihedral_edge_length_percentage, section_dihedral_offset*dihedral_edge_length_percentage );
				dihedral_span_chord.set_xy( total_dihedral_offset_x - dihedral_span_chord.x() , total_dihedral_offset - dihedral_span_chord.y() );

				if(sects[i].span != 0)
					other_dihedral_slope = next_dihedral_offset/next_dihedral_offset_x;

				if(sects[i-1].span != 0)
					dihedral_slope = section_dihedral_offset/section_dihedral_offset_x;
	
				dihedral_previous_edge = true;
			}
			
			bool tan_sweep_undefined = false, tan_te_sweep_undefined = false, tan_dihedral_undefined = false;

			//find tangent line of leading edges
			double tangent_sweep = section_sweep;
			double tangent_sweep_slope = 0.0, tangent_sweep_intercept = 0.0;

			if( fabs(tangent_sweep) < 0.0001 )
				tan_sweep_undefined = true;

			if(!tan_sweep_undefined)
			{
				tangent_sweep_slope = tan((90 - tangent_sweep)*DEG_2_RAD);	
				tangent_sweep_intercept = total_span - tangent_sweep_slope*total_sweep_offset;
			}

			//find tangent line of trailing edges
			double tangent_te_sweep = (previous_te_sweep + next_te_sweep)/2;
			double tangent_te_sweep_slope = 0.0, tangent_te_sweep_intercept = 0.0;

			if( fabs(tangent_te_sweep) < 0.0001 )
				tan_te_sweep_undefined = true;

			if(!tan_te_sweep_undefined)
			{
				tangent_te_sweep_slope = -1*tan((90.0-tangent_te_sweep)*DEG_2_RAD);
				tangent_te_sweep_intercept = total_span - tangent_te_sweep_slope*total_te_sweep_offset;
			}

			//find tangent line of belly edges
			double tangent_dihedral = (sects[i-1].dihedral + sects[i].dihedral)/2;
			double tangent_dihedral_slope = 0.0, tangent_dihedral_intercept = 0.0;

			if( fabs(tangent_dihedral) < 0.0001 )
				tan_dihedral_undefined = true;

			if(!tan_dihedral_undefined)
			{
				tangent_dihedral_slope = tan(tangent_dihedral*DEG_2_RAD);
				tangent_dihedral_intercept = total_dihedral_offset - tangent_dihedral_slope*total_dihedral_offset_x;
			}

			//find center line of all edge pairings (line orthogonal to tangent line)
			double center_sweep_slope = 0;
			double center_te_sweep_slope = 0;
			double center_dihedral_slope = 0;
			double center_sweep_intercept = 0;
			double center_te_sweep_intercept = 0;
			double center_dihedral_intercept = 0;

			if(tangent_sweep_slope != 0.0)
				center_sweep_slope = -1/tangent_sweep_slope;
			else if(tan_sweep_undefined)
				center_sweep_slope = 0.0;
	
			if(tangent_te_sweep_slope != 0.0)
				center_te_sweep_slope = -1/tangent_te_sweep_slope;
			else if(tan_te_sweep_undefined)
				center_te_sweep_slope = 0.0;

			if(tangent_dihedral_slope != 0.0)
				center_dihedral_slope = -1/tangent_dihedral_slope;
			else if(tan_dihedral_undefined)
				center_dihedral_slope = 0.0;

			center_sweep_intercept = total_span - center_sweep_slope*total_sweep_offset;
			center_te_sweep_intercept = total_span - center_te_sweep_slope*(sects[0].rc - total_te_sweep_offset);
			center_dihedral_intercept = total_dihedral_offset - center_dihedral_slope*total_dihedral_offset_x;

			//find sweep fillet's pivot point by determining intersection of line orthogonal to one edge and center line
			//pivot point is center of fillet's circle
			
			if( fabs(sweep_slope) > 0.0001 )
				ortho_sweep_slope = -1/sweep_slope;
			else
				ortho_sweep_slope = 0.0;

			ortho_sweep_intercept = sweep_span_chord.y() - ortho_sweep_slope*sweep_span_chord.x();

			if( fabs(center_sweep_slope - ortho_sweep_slope) > 0.000001)
				sweep_pivot_point.set_x( (ortho_sweep_intercept - center_sweep_intercept) / (center_sweep_slope - ortho_sweep_slope) );
			else
				sweep_pivot_point.set_x( 0.0 );


			sweep_pivot_point.set_y( sweep_pivot_point.x()*ortho_sweep_slope + ortho_sweep_intercept );

			//find trailing edge sweep fillet's pivot point by determining intersection of line orthogonal to one edge and center line
			//pivot point is center of fillet's circle
			if( fabs(te_sweep_slope) > 0.0001 )
				ortho_te_sweep_slope = -1/te_sweep_slope;
			else
				ortho_te_sweep_slope = 0.0;

			ortho_te_sweep_intercept = te_sweep_span_chord.y() - ortho_te_sweep_slope*te_sweep_span_chord.x();

			if( fabs(center_te_sweep_slope - ortho_te_sweep_slope) > 0.000001 )
				te_sweep_pivot_point.set_x( (ortho_te_sweep_intercept - center_te_sweep_intercept) / (center_te_sweep_slope - ortho_te_sweep_slope) );
			else
				te_sweep_pivot_point.set_x( 0.0 );

			if( fabs(te_sweep_slope) > 0.0001 )
				te_sweep_pivot_point.set_y( te_sweep_pivot_point.x()*ortho_te_sweep_slope + ortho_te_sweep_intercept );
			else
				te_sweep_pivot_point.set_y( te_sweep_pivot_point.x()*center_te_sweep_slope + center_te_sweep_intercept );

			//find dihedral fillet's pivot point by determining intersection of line orthogonal to one edge and center line
			//pivot point is center of fillet's circle
			if( fabs(dihedral_slope) > 0.0001 )
			{
				ortho_dihedral_slope = -1/dihedral_slope;
				ortho_dihedral_intercept = dihedral_span_chord.y() - ortho_dihedral_slope*dihedral_span_chord.x();

				if( fabs(center_dihedral_slope - ortho_dihedral_slope) > 0.000001 )
					dihedral_pivot_point.set_x( (ortho_dihedral_intercept - center_dihedral_intercept) / (center_dihedral_slope - ortho_dihedral_slope) );
			}
			else
			{
				dihedral_pivot_point.set_x( dihedral_span_chord.x() );
			}
			
			if( fabs(dihedral_slope) > 0.0001 )
				dihedral_pivot_point.set_y( dihedral_pivot_point.x()*ortho_dihedral_slope + ortho_dihedral_intercept );
			else
				dihedral_pivot_point.set_y( dihedral_pivot_point.x()*center_dihedral_slope + center_dihedral_intercept );

			//find the all three of the second edge orthogonal lines
			if(other_sweep_slope != 0)
			{
				other_ortho_sweep_slope = -1/other_sweep_slope;
				other_ortho_sweep_intercept = sweep_pivot_point.y() - other_ortho_sweep_slope*sweep_pivot_point.x();
				other_sweep_intercept = total_span - other_sweep_slope*total_sweep_offset;
				other_sweep_span_chord.set_x( (other_sweep_intercept - other_ortho_sweep_intercept) / ( other_ortho_sweep_slope - other_sweep_slope ) );
				other_sweep_span_chord.set_y( other_sweep_span_chord.x()*other_sweep_slope + other_sweep_intercept );
			}
			else
			{
				other_ortho_sweep_intercept = 0.0;
				other_sweep_intercept = 0.0;
				
				if(sweep_edge_dist1 > sweep_edge_dist2)
					other_sweep_span_chord.set_x( total_sweep_offset );
				else
					other_sweep_span_chord.set_x( total_sweep_offset + next_sweep_offset );


				other_sweep_span_chord.set_y( sweep_pivot_point.y() );
			}
	
			if(other_te_sweep_slope != 0)
			{
				other_ortho_te_sweep_slope = -1/other_te_sweep_slope;
				other_ortho_te_sweep_intercept = te_sweep_pivot_point.y() - other_ortho_te_sweep_slope*te_sweep_pivot_point.x();
				other_te_sweep_intercept = total_span - other_te_sweep_slope*(sects[0].rc - total_te_sweep_offset);
				other_te_sweep_span_chord.set_x( (other_te_sweep_intercept - other_ortho_te_sweep_intercept) / ( other_ortho_te_sweep_slope - other_te_sweep_slope ) );
				other_te_sweep_span_chord.set_y( other_te_sweep_span_chord.x()*other_te_sweep_slope + other_te_sweep_intercept );
			}
			else
			{
				other_ortho_te_sweep_intercept = 0.0;
				other_te_sweep_intercept = 0.0;

				if(te_sweep_edge_dist1 > te_sweep_edge_dist2)
					other_te_sweep_span_chord.set_x( total_te_sweep_offset );
				else
					other_te_sweep_span_chord.set_x( total_te_sweep_offset + next_te_sweep_offset );

				other_te_sweep_span_chord.set_y( te_sweep_pivot_point.y() );
			}

			if(other_dihedral_slope != 0)
			{
				other_ortho_dihedral_slope = -1/other_dihedral_slope;
				other_ortho_dihedral_intercept = dihedral_pivot_point.y() - other_ortho_dihedral_slope*dihedral_pivot_point.x();
				other_dihedral_intercept = total_dihedral_offset - other_dihedral_slope*total_dihedral_offset_x;
				other_dihedral_span_chord.set_x( (other_dihedral_intercept - other_ortho_dihedral_intercept) / ( other_ortho_dihedral_slope - other_dihedral_slope ) ); 
				other_dihedral_span_chord.set_y( other_dihedral_span_chord.x()*other_dihedral_slope + other_dihedral_intercept );
			}
			else
			{
				other_ortho_dihedral_intercept = 0.0;
				other_dihedral_intercept = 0.0;
				other_dihedral_span_chord.set_x( dihedral_pivot_point.x() );
				other_dihedral_span_chord.set_y( total_dihedral_offset );
			}
					

			sweep_circle_radius = sqrt( pow(sweep_pivot_point.y() - sweep_span_chord.y(), 2) + pow(sweep_pivot_point.x() - sweep_span_chord.x(), 2) );
			te_sweep_circle_radius = sqrt( pow(te_sweep_pivot_point.y() - te_sweep_span_chord.y(), 2) + pow(te_sweep_pivot_point.x() - te_sweep_span_chord.x(), 2) );
			dihedral_circle_radius = sqrt( pow(dihedral_pivot_point.y() - dihedral_span_chord.y(), 2) + pow(dihedral_pivot_point.x() - dihedral_span_chord.x(), 2) );

			double sweep_pivot_point_sign = 0.0, te_sweep_pivot_point_sign = 0.0, dihedral_pivot_point_sign_y = 0.0;

			if(total_sweep_offset > sweep_pivot_point.x())
				sweep_pivot_point_sign = 1;
			else
				sweep_pivot_point_sign = -1;

			if( (sects[0].rc - total_te_sweep_offset ) > te_sweep_pivot_point.x())
				te_sweep_pivot_point_sign = 1;
			else
				te_sweep_pivot_point_sign = -1;

			if(total_dihedral_offset > dihedral_pivot_point.y())
				dihedral_pivot_point_sign_y = 1;
			else
				dihedral_pivot_point_sign_y = -1;

			double dihedral_radial_percentage = dihedral_circle_radius / (sqrt( pow(dihedral_pivot_point.y() - total_dihedral_offset, 2) + 
pow(total_dihedral_offset_x - dihedral_pivot_point.x(), 2) ) );

			sweep_chord_fillet_intersection.set_xy( sqrt( pow(sweep_circle_radius, 2) - pow(total_span - sweep_pivot_point.y(), 2) )*sweep_pivot_point_sign + sweep_pivot_point.x() , total_span );	
			te_sweep_chord_fillet_intersection.set_xy( sqrt( pow(te_sweep_circle_radius, 2) - pow(total_span - te_sweep_pivot_point.y(), 2) )*te_sweep_pivot_point_sign + te_sweep_pivot_point.x() , total_span );
			dihedral_chord_fillet_intersection.set_xy( (total_dihedral_offset_x - dihedral_pivot_point.x()) * dihedral_radial_percentage + dihedral_pivot_point.x(), (total_dihedral_offset - dihedral_pivot_point.y()) * dihedral_radial_percentage + dihedral_pivot_point.y());

			sweep_arc_angle = fabs(atan(sects[i-1].tan_sweep_at(0.0,getSymCode()))*RAD_2_DEG - 
								   atan(sects[i].tan_sweep_at(0.0,getSymCode()))*RAD_2_DEG);
			te_sweep_arc_angle = fabs(previous_te_sweep - next_te_sweep);
			dihedral_arc_angle = fabs(sects[i-1].dihedral - sects[i].dihedral);

			double third_side_sweep_tip = 0.0, third_side_te_sweep_tip = 0.0, third_side_dihedral_tip = 0.0;

			if(sweep_previous_edge)
				third_side_sweep_tip = sqrt( pow((sweep_span_chord.x() - sweep_chord_fillet_intersection.x()),2) + pow((sweep_span_chord.y() - sweep_chord_fillet_intersection.y()),2) );			
			else
				third_side_sweep_tip = sqrt( pow((sweep_chord_fillet_intersection.x() - other_sweep_span_chord.x()),2) + pow((sweep_chord_fillet_intersection.y() - other_sweep_span_chord.y()),2) );

			if(te_sweep_previous_edge)
				third_side_te_sweep_tip = sqrt( pow((te_sweep_span_chord.x() - te_sweep_chord_fillet_intersection.x()),2) + pow((te_sweep_span_chord.y() - te_sweep_chord_fillet_intersection.y()),2) );			
			else
				third_side_te_sweep_tip = sqrt( pow((te_sweep_chord_fillet_intersection.x() - other_te_sweep_span_chord.x()),2) + pow((te_sweep_chord_fillet_intersection.y() - other_te_sweep_span_chord.y()),2) );

			if(dihedral_previous_edge)
				third_side_dihedral_tip = sqrt( pow((dihedral_span_chord.x() - dihedral_chord_fillet_intersection.x()),2) + pow((dihedral_span_chord.y() - dihedral_chord_fillet_intersection.y()),2) );		
			else
				third_side_dihedral_tip = sqrt( pow((dihedral_chord_fillet_intersection.x() - other_dihedral_span_chord.x()),2) + pow((dihedral_chord_fillet_intersection.y() - other_dihedral_span_chord.y()),2) );

			double tip_fillet_span = 0.0, root_fillet_span = 0.0;

			if(dihedral_previous_edge)
				tip_fillet_span = sqrt( pow(total_sweep_offset - sweep_span_chord.x(),2) + pow(total_span - sweep_span_chord.y(),2) );
			else
				tip_fillet_span = sqrt( pow(total_sweep_offset - other_dihedral_span_chord.x(),2) + pow(total_span - other_dihedral_span_chord.y(),2) );

			if(dihedral_previous_edge)
				root_fillet_span = sqrt( pow(total_sweep_offset - other_sweep_span_chord.x(),2) + pow(total_span - other_sweep_span_chord.y(),2) );
			else
				root_fillet_span = sqrt( pow(total_sweep_offset - sweep_span_chord.x(),2) + pow(total_span - sweep_span_chord.y(),2) );
	
			
			double b = 0; 

			if( (pow(sweep_circle_radius,2) - pow(third_side_sweep_tip/2, 2)) > 0.0)
			{
				b = sqrt( pow(sweep_circle_radius,2) - pow(third_side_sweep_tip/2, 2) );
				tip_sweep_arc_angle = 2*atan(third_side_sweep_tip/(2*b))*RAD_2_DEG;
			}
			else
			{
				tip_sweep_arc_angle = 0.0;
			}

			if( (pow(te_sweep_circle_radius,2) - pow(third_side_te_sweep_tip/2, 2)) > 0.0)
			{
				b = sqrt( pow(te_sweep_circle_radius,2) - pow(third_side_te_sweep_tip/2, 2) );
				tip_te_sweep_arc_angle = 2*atan(third_side_te_sweep_tip/(2*b))*RAD_2_DEG;
			}
			else
			{
				tip_te_sweep_arc_angle = 0.0;
			}

			if( (pow(dihedral_circle_radius,2) - pow(third_side_dihedral_tip/2, 2)) > 0.0)
			{
				b = sqrt( pow(dihedral_circle_radius,2) - pow(third_side_dihedral_tip/2, 2) );
				tip_dihedral_arc_angle = 2*atan(third_side_dihedral_tip/(2*b))*RAD_2_DEG;
			}
			else
			{
				tip_dihedral_arc_angle = 0.0;
			}

			root_sweep_arc_angle = sweep_arc_angle - tip_sweep_arc_angle;
			root_te_sweep_arc_angle = te_sweep_arc_angle - tip_te_sweep_arc_angle;
			root_dihedral_arc_angle = dihedral_arc_angle - tip_dihedral_arc_angle;

			if(num_sweep_subsections != 0)
				sweep_subsection_arc_angle = tip_sweep_arc_angle / num_sweep_subsections;
			else
				sweep_subsection_arc_angle = tip_sweep_arc_angle;

			if(num_te_sweep_subsections != 0)
				te_sweep_subsection_arc_angle = tip_te_sweep_arc_angle / num_te_sweep_subsections;
			else
				te_sweep_subsection_arc_angle = tip_te_sweep_arc_angle;

			if(num_dihedral_subsections != 0)
				dihedral_subsection_arc_angle = tip_dihedral_arc_angle / num_dihedral_subsections;
			else
				dihedral_subsection_arc_angle = tip_dihedral_arc_angle;

			subsection_sweep = (section_sweep - previous_sweep) / ( num_sweep_subsections + 1 );
			subsection_te_sweep = (section_te_sweep - previous_te_sweep) / ( num_te_sweep_subsections + 1 );
			subsection_dihedral = (section_dihedral - previous_dihedral) / ( num_dihedral_subsections + 1 );
		}
		
		if(num_sweep_subsections == 0)
			other_sweep_span_chord = sweep_span_chord;

		if(num_te_sweep_subsections == 0)
			other_te_sweep_span_chord = te_sweep_span_chord;

		if(num_dihedral_subsections == 0)
			other_dihedral_span_chord = dihedral_span_chord;
		
		if(num_sweep_subsections > 0)
			first_sweep_subsection_span = 2*sweep_circle_radius*sin( sweep_subsection_arc_angle/2 * DEG_2_RAD ) * cos( (section_sweep - subsection_sweep) * DEG_2_RAD );
		else
			first_sweep_subsection_span = section_span;

		if(num_te_sweep_subsections > 0)
			first_te_sweep_subsection_span = 2*te_sweep_circle_radius*sin( te_sweep_subsection_arc_angle/2 * DEG_2_RAD ) * cos( (section_te_sweep - subsection_te_sweep) * DEG_2_RAD );
		else
			first_te_sweep_subsection_span = section_span;

		if(num_dihedral_subsections > 0)
			first_dihedral_subsection_span = 2*dihedral_circle_radius*sin( dihedral_subsection_arc_angle/2 * DEG_2_RAD );
		else
			first_dihedral_subsection_span = section_span;

		sweep_flag = SMALLEST; 
		te_sweep_flag = SMALLEST; 
		dihedral_flag = SMALLEST;

		if( (first_sweep_subsection_span < first_te_sweep_subsection_span) && (first_sweep_subsection_span < first_dihedral_subsection_span) )
		{
			num_smallest_subsections = num_sweep_subsections;
			smallest_subsection_arc_angle = sweep_subsection_arc_angle;
			
			if(sweep_previous_edge)
				smallest_subsection_total_span = sects[i-1].span * sweep_edge_length_percentage;
			else			
				smallest_subsection_total_span = sects[i-1].span * sweep_edge_dist2 * sweep_edge_length_percentage / sweep_edge_dist1;

			if(first_te_sweep_subsection_span < first_dihedral_subsection_span)
			{
				num_moderate_subsections = num_te_sweep_subsections;
				moderate_subsection_arc_angle = te_sweep_subsection_arc_angle;
				te_sweep_flag = MODERATE;
				
				if(te_sweep_previous_edge)
					moderate_subsection_total_span = sects[i-1].span * te_sweep_edge_length_percentage;
				else			
					moderate_subsection_total_span = sects[i-1].span * te_sweep_edge_dist2 * te_sweep_edge_length_percentage / te_sweep_edge_dist1;			
				
				num_largest_subsections = num_dihedral_subsections;
				largest_subsection_arc_angle = dihedral_subsection_arc_angle;
				dihedral_flag = LARGEST;

				if(dihedral_previous_edge)
					largest_subsection_total_span = sects[i-1].span * dihedral_edge_length_percentage;
				else			
					largest_subsection_total_span = sects[i].span * dihedral_edge_length_percentage;	
			}
			else
			{
				num_moderate_subsections = num_dihedral_subsections;
				moderate_subsection_arc_angle = dihedral_subsection_arc_angle;
				dihedral_flag = MODERATE;

				if(dihedral_previous_edge)
					moderate_subsection_total_span = sects[i-1].span * dihedral_edge_length_percentage;
				else			
					moderate_subsection_total_span = sects[i].span * dihedral_edge_length_percentage;		
	
				num_largest_subsections = num_te_sweep_subsections;
				largest_subsection_arc_angle = te_sweep_subsection_arc_angle;
				te_sweep_flag = LARGEST;	

				if(te_sweep_previous_edge)
					largest_subsection_total_span = sects[i-1].span * te_sweep_edge_length_percentage;
				else			
					largest_subsection_total_span = sects[i-1].span * te_sweep_edge_dist2 * te_sweep_edge_length_percentage / te_sweep_edge_dist1;	
			}

			
		}
		else if( (first_te_sweep_subsection_span < first_sweep_subsection_span) && (first_te_sweep_subsection_span < first_dihedral_subsection_span) )
		{
			num_smallest_subsections = num_te_sweep_subsections;
			smallest_subsection_arc_angle = te_sweep_subsection_arc_angle;	

			if(te_sweep_previous_edge)
				smallest_subsection_total_span = sects[i-1].span * te_sweep_edge_length_percentage;
			else			
				smallest_subsection_total_span = sects[i-1].span * te_sweep_edge_dist2 * te_sweep_edge_length_percentage / te_sweep_edge_dist1;	

			if(first_sweep_subsection_span < first_dihedral_subsection_span)
			{
				num_moderate_subsections = num_sweep_subsections;
				moderate_subsection_arc_angle = sweep_subsection_arc_angle;
				sweep_flag = MODERATE;			
	
				if(sweep_previous_edge)
					moderate_subsection_total_span = sects[i-1].span * sweep_edge_length_percentage;
				else			
					moderate_subsection_total_span = sects[i-1].span * sweep_edge_dist2 * sweep_edge_length_percentage / sweep_edge_dist1;	

				num_largest_subsections = num_dihedral_subsections;
				largest_subsection_arc_angle = dihedral_subsection_arc_angle;
				dihedral_flag = LARGEST;

				if(dihedral_previous_edge)
					largest_subsection_total_span = sects[i-1].span * dihedral_edge_length_percentage;
				else			
					largest_subsection_total_span = sects[i].span * dihedral_edge_length_percentage;
			
			}
			else
			{
				num_moderate_subsections = num_dihedral_subsections;
				moderate_subsection_arc_angle = dihedral_subsection_arc_angle;
				dihedral_flag = MODERATE;

				if(dihedral_previous_edge)
					moderate_subsection_total_span = sects[i-1].span * dihedral_edge_length_percentage;
				else			
					moderate_subsection_total_span = sects[i].span * dihedral_edge_length_percentage;		
	
				num_largest_subsections = num_sweep_subsections;
				largest_subsection_arc_angle = sweep_subsection_arc_angle;
				sweep_flag = LARGEST;

				if(sweep_previous_edge)
					largest_subsection_total_span = sects[i-1].span * sweep_edge_length_percentage;
				else			
					largest_subsection_total_span = sects[i-1].span * sweep_edge_dist2 * sweep_edge_length_percentage / sweep_edge_dist1;
			
			}
		}
		else
		{
			num_smallest_subsections = num_dihedral_subsections;
			smallest_subsection_arc_angle = dihedral_subsection_arc_angle;		

			if(dihedral_previous_edge)
				smallest_subsection_total_span = sects[i-1].span * dihedral_edge_length_percentage;
			else			
				smallest_subsection_total_span = sects[i].span * dihedral_edge_length_percentage;

			if(first_sweep_subsection_span < first_te_sweep_subsection_span)
			{
				num_moderate_subsections = num_sweep_subsections;
				moderate_subsection_arc_angle = sweep_subsection_arc_angle;
				sweep_flag = MODERATE;
	
				if(sweep_previous_edge)
					moderate_subsection_total_span = sects[i-1].span * sweep_edge_length_percentage;
				else			
					moderate_subsection_total_span = sects[i-1].span * sweep_edge_dist2 * sweep_edge_length_percentage / sweep_edge_dist1;			

				num_largest_subsections = num_te_sweep_subsections;
				largest_subsection_arc_angle = te_sweep_subsection_arc_angle;
				te_sweep_flag = LARGEST;

				if(te_sweep_previous_edge)
					largest_subsection_total_span = sects[i-1].span * te_sweep_edge_length_percentage;
				else			
					largest_subsection_total_span = sects[i-1].span * te_sweep_edge_dist2 * te_sweep_edge_length_percentage / te_sweep_edge_dist1;
			}
			else
			{
				num_moderate_subsections = num_te_sweep_subsections;
				moderate_subsection_arc_angle = te_sweep_subsection_arc_angle;
				te_sweep_flag = MODERATE;

				if(te_sweep_previous_edge)
					moderate_subsection_total_span = sects[i-1].span * te_sweep_edge_length_percentage;
				else			
					moderate_subsection_total_span = sects[i-1].span * te_sweep_edge_dist2 * te_sweep_edge_length_percentage / te_sweep_edge_dist1;

				num_largest_subsections = num_sweep_subsections;
				largest_subsection_arc_angle = sweep_subsection_arc_angle;
				sweep_flag = LARGEST;

				if(sweep_previous_edge)
					largest_subsection_total_span = sects[i-1].span * sweep_edge_length_percentage;
				else			
					largest_subsection_total_span = sects[i-1].span * sweep_edge_dist2 * sweep_edge_length_percentage / sweep_edge_dist1;			
			}
		}

		total_moderate_subsection_span = 0.0;
		total_largest_subsection_span = 0.0;

		if( num_smallest_subsections != 0  )
		{
			if( num_moderate_subsections == 0 && num_largest_subsections == 0 )
			{
				num_moderate_subsections = 1;
				moderate_subsection_total_span = smallest_subsection_total_span;
				total_moderate_subsection_span = moderate_subsection_total_span;

				num_largest_subsections = 1;
				largest_subsection_total_span = smallest_subsection_total_span;	
				total_largest_subsection_span = largest_subsection_total_span;
			}
			else if( num_moderate_subsections == 0 && num_largest_subsections != 0 )
			{
				num_moderate_subsections = 1;
				if(largest_subsection_total_span > smallest_subsection_total_span)
					moderate_subsection_total_span = largest_subsection_total_span;
				else
					moderate_subsection_total_span = smallest_subsection_total_span;

				total_moderate_subsection_span = moderate_subsection_total_span;
			}
			else if( num_moderate_subsections != 0 && num_largest_subsections == 0 )
			{
				num_largest_subsections = 1;

				if(moderate_subsection_total_span > smallest_subsection_total_span)
					largest_subsection_total_span = moderate_subsection_total_span;
				else
					largest_subsection_total_span = smallest_subsection_total_span;

				total_largest_subsection_span = largest_subsection_total_span;
			}

		}

		total_smallest_subsection_span = 0;

		double sweep_tss = 0.0, te_sweep_tss = 0.0, dihedral_tss = 0.0;
	
		if(sweep_previous_edge)
			sweep_tss = sects[i-1].span*sweep_edge_length_percentage;
		else
			sweep_tss = sects[i-1].span * sweep_edge_dist2 * sweep_edge_length_percentage / sweep_edge_dist1;

		if(te_sweep_previous_edge)
			te_sweep_tss = sects[i-1].span*te_sweep_edge_length_percentage;
		else
			te_sweep_tss = sects[i-1].span * te_sweep_edge_dist2 * te_sweep_edge_length_percentage / te_sweep_edge_dist1;

		if(dihedral_previous_edge)
			dihedral_tss = sects[i-1].span*dihedral_edge_length_percentage;
		else
			dihedral_tss = sects[i].span*dihedral_edge_length_percentage;

		if( fabs(section_sweep) < 0.0001 )
			sweep_tss = 0.0;

		if( fabs(section_te_sweep) < 0.0001 )
			te_sweep_tss = 0.0;

		if( fabs(section_dihedral) < 0.0001 )
			dihedral_tss = 0.0;

		if(sweep_tss > te_sweep_tss)
		{
			if(sweep_tss > dihedral_tss)
				tip_starting_span = sweep_tss;
			else
				tip_starting_span = dihedral_tss;
		}
		else if(te_sweep_tss > dihedral_tss)
			tip_starting_span = te_sweep_tss;
		else
			tip_starting_span = dihedral_tss;

		HwbSect mainsect;
		mainsect.driver = MS_S_TC_RC;
		mainsect.span = sects[i-1].span - tip_starting_span - previous_root_span;

		if( fabs(mainsect.span) > 0.00001)
		{
			span_percentage_sum += (mainsect.span/sects[i-1].span);

			mainsect.twist = total_twist + curr_twist_total + (section_twist - previous_twist) * (mainsect.span / sects[i-1].span);
			curr_twist_total += (section_twist - previous_twist) * (mainsect.span / sects[i-1].span);
			mainsect.twistLoc = sects[i-1].twistLoc;
			mainsect.sweepLoc = 0.0;
			mainsect.sweep = atan(sects[i-1].tan_sweep_at(0.0,getSymCode()))*RAD_2_DEG;
			mainsect.dihedral = sects[i-1].dihedral;

			if(sub_sects.dimension() == 0)
				mainsect.rc = section_rc;
			else
				mainsect.rc = sub_sects[sub_sects.dimension() - 1].tc;
				
			double span_percentage = (section_span - tip_starting_span)/section_span;
		
			mainsect.tc = ( section_sweep_offset - span_percentage * section_span*tan(mainsect.sweep*DEG_2_RAD) ) +
				( section_te_sweep_offset -  span_percentage * section_span*tan(previous_te_sweep*DEG_2_RAD) ) + sects[i].rc; 

			double span_at_root = 0.0, span_at_tip = 0.0;
			for(int j = 0; j < i; j++)
			{
				if(j != 0)
					span_at_root += sects[j-1].span;

				span_at_tip += sects[j].span;
			}

			double new_thickness = span_percentage_sum*thickness_difference + sects[i-1].rootAf->get_thickness()->get();

			if( fabs(previous_root_span) < 0.0001 )
			{
				mainsect.rootAf = new Af(this);
				*(mainsect.rootAf) = *(sects[i-1].rootAf);
				mainsect.rootAf->sterilize_parms();
				sub_sect_foils.append(mainsect.rootAf);
			}
			else
				mainsect.rootAf = sub_sects[sub_sects.dimension() - 1].tipAf;

			if(fabs(tip_starting_span) < 0.0001)
			{
				mainsect.tipAf = new Af( this );
				*(mainsect.tipAf) = *(sects[i-1].tipAf);
				mainsect.tipAf->sterilize_parms();
				sub_sect_foils.append(mainsect.tipAf);
			}
			else
			{
				Af* newaf = new Af( this );
				mainsect.tipAf = newaf;
				sub_sect_foils.append(newaf);
				mainsect.tipAf->init_script("wing_foil");
				*(mainsect.tipAf) = *(sects[i].rootAf);
				mainsect.tipAf->sterilize_parms();
				mainsect.tipAf->get_thickness()->set(new_thickness);
				mainsect.tipAf->generate_airfoil();
				mainsect.tipAf->load_name();
			}

			if(fabs(previous_root_span) < 0.0001)
					chord_lengths[i-1] = mainsect.rc;

			mainsect.num_interp_xsecs = sects[i-1].num_interp_xsecs;

			num_subsections++;
			total_num_subsections++;
			mainsect.fillDependData();
			sub_sects.append( mainsect );
		}	

		moderate_index = 0;
		largest_index = 0;
		curr_twist_total = 0.0;

		if( total_moderate_subsection_span != moderate_subsection_total_span )
			total_moderate_subsection_span = 0.0;

		if( total_largest_subsection_span != largest_subsection_total_span )		
			total_largest_subsection_span = 0.0;

		span_percentage_sum = 0.0;

		if(num_smallest_subsections != 0)
			dihedral_rot_starting_indices.append(sub_sects.dimension());
		else
			dihedral_rot_starting_indices.append(sub_sects.dimension()-1);

		double smallest_subsection_span = 0.0, moderate_subsection_span = 0.0, largest_subsection_span = 0.0;
		smallest_subsection_span = (smallest_subsection_total_span/num_smallest_subsections);
		moderate_subsection_span = (moderate_subsection_total_span/num_moderate_subsections);
		largest_subsection_span = (largest_subsection_total_span/num_largest_subsections);

		for(int j = 0; j < num_smallest_subsections; j++)
		{
			HwbSect wingsect;
			double te_sweep = 0.0;
			wingsect.driver = MS_S_TC_RC;
			
			wingsect.span = smallest_subsection_span;
			wingsect.twist = total_twist + section_twist - (section_twist - previous_twist) * (wingsect.span / section_span) - curr_twist_total;
			curr_twist_total += (section_twist - previous_twist) * (wingsect.span / section_span);
			wingsect.twistLoc = section_twistloc;
			wingsect.sweepLoc = section_sweeploc;

			span_percentage_sum += (wingsect.span/sects[i-1].span);

			double smallest_total = total_smallest_subsection_span + smallest_subsection_span;
			double moderate_total = total_moderate_subsection_span + moderate_subsection_span;


			if( (smallest_total > moderate_total) && fabs( smallest_total - moderate_total ) > 0.00001 && (moderate_total < moderate_subsection_total_span) )
			{
				if(sweep_flag == MODERATE)
				{
					if( (moderate_index+1) >= num_moderate_subsections)
						wingsect.sweep = section_sweep + -1*subsection_sweep*(num_moderate_subsections);
					else
						wingsect.sweep = section_sweep + -1*(subsection_sweep*(moderate_index+1) + subsection_sweep*(moderate_index+2))/2;
				}
				else if(te_sweep_flag == MODERATE)
				{
					if( (moderate_index+1) >= num_moderate_subsections)
						te_sweep = section_te_sweep + -1*subsection_te_sweep*(num_moderate_subsections);
					else
						te_sweep = section_te_sweep + -1*(subsection_te_sweep*(moderate_index+1) + subsection_te_sweep*(moderate_index+2))/2;

				}
				else
				{
					if( (moderate_index+1) >= num_moderate_subsections)
						wingsect.dihedral = section_dihedral + -1*subsection_dihedral*(num_moderate_subsections);
					else
						wingsect.dihedral = section_dihedral + -1*(subsection_dihedral*(moderate_index+1) + subsection_dihedral*(moderate_index+2))/2;
				}

				total_moderate_subsection_span += moderate_subsection_span;
				moderate_index++;
			}
			else
			{
				if(sweep_flag == MODERATE)
					wingsect.sweep = section_sweep + -1*subsection_sweep*(moderate_index+1);
				else if(te_sweep_flag == MODERATE)
					te_sweep = section_te_sweep + -1*subsection_te_sweep*(moderate_index+1);
				else
					wingsect.dihedral = section_dihedral + -1*subsection_dihedral*(moderate_index+1);
			}
			
			double largest_total = total_largest_subsection_span + largest_subsection_span;

			if( (smallest_total > largest_total) && fabs( smallest_total - largest_total ) > 0.00001 && (largest_total < largest_subsection_total_span) )
			{
				if(sweep_flag == LARGEST)
				{
					if( (largest_index+1) >= num_largest_subsections)
						wingsect.sweep = section_sweep + -1*subsection_sweep*num_largest_subsections;
					else
						wingsect.sweep = section_sweep + -1*(subsection_sweep*(largest_index+1) + subsection_sweep*(largest_index+2))/2;
				}
				else if(te_sweep_flag == LARGEST)
				{
					if( (largest_index+1) >= num_largest_subsections)
						te_sweep = section_te_sweep + -1*subsection_te_sweep*(num_largest_subsections);
					else
						te_sweep = section_te_sweep + -1*(subsection_te_sweep*(largest_index+1) + subsection_te_sweep*(largest_index+2))/2;
				}
				else
				{
					
					if( (largest_index+1) >= num_largest_subsections)
						wingsect.dihedral = section_dihedral + -1*subsection_dihedral*(num_largest_subsections);
					else
						wingsect.dihedral = section_dihedral + -1*(subsection_dihedral*(largest_index+1) + subsection_dihedral*(largest_index+2))/2;
				}

				total_largest_subsection_span += largest_subsection_span;
				largest_index++;
			}
			else
			{
				if(sweep_flag == LARGEST)
					wingsect.sweep = section_sweep + -1*subsection_sweep*(largest_index+1);
				else if(te_sweep_flag == LARGEST)
					te_sweep = section_te_sweep + -1*subsection_te_sweep*(largest_index+1);
				else
					wingsect.dihedral = section_dihedral + -1*subsection_dihedral*(largest_index+1);
			}

			if(sweep_flag == SMALLEST)
				wingsect.sweep = section_sweep + -1*subsection_sweep*(j+1);
			else if(te_sweep_flag == SMALLEST)
				te_sweep = section_te_sweep + -1*subsection_te_sweep*(j+1);
			else
				wingsect.dihedral = section_dihedral + -1*subsection_dihedral*(j+1);

			total_smallest_subsection_span += smallest_subsection_span;

			double new_thickness = sects[i].rootAf->get_thickness()->get() - span_percentage_sum*thickness_difference;

			if( j == (num_smallest_subsections - 1) && (largest_subsection_total_span <= smallest_subsection_total_span) && 
			  (moderate_subsection_total_span <= smallest_subsection_total_span) )
			{
				wingsect.rootAf = sub_sects[sub_sects.dimension() - 1].tipAf;
			}
			else
			{
				Af* newaf = new Af( this );
				wingsect.rootAf = newaf;
				sub_sect_foils.append(newaf);
				wingsect.rootAf->init_script("wing_foil");
				*(wingsect.rootAf) = *(sects[i].rootAf);
				wingsect.rootAf->sterilize_parms();
				wingsect.rootAf->get_thickness()->set(new_thickness);
				wingsect.rootAf->generate_airfoil();
				wingsect.rootAf->load_name();
			}			

			if(j == 0)
			{
				wingsect.tipAf = new Af(this);
				*(wingsect.tipAf) = *(sects[i].rootAf);
				wingsect.tipAf->sterilize_parms();
				sub_sect_foils.append(wingsect.tipAf);
			}
			else
				wingsect.tipAf = temp_subsects[temp_subsects.dimension()-1].rootAf;			

			wingsect.num_interp_xsecs = sects[i-1].num_interp_xsecs;

			wingsect.fillDependData();
			temp_subsects.append( wingsect );
			te_sweep_list.append( te_sweep );

			num_subsections++;
			total_num_subsections++;
		}

		if( moderate_subsection_total_span > smallest_subsection_total_span && (num_smallest_subsections > 0) )
		{
			// = Bridge subsection connects micro subsections to macro subsections = //
			HwbSect bridgesect;
			double te_sweep;
			bridgesect.driver = MS_S_TC_RC;

			if( ( total_moderate_subsection_span + moderate_subsection_span - smallest_subsection_total_span ) > 0.0001 )
			{
				bridgesect.span = total_moderate_subsection_span + moderate_subsection_span - smallest_subsection_total_span;	

				bridgesect.twist = total_twist + section_twist - (section_twist - previous_twist) * (bridgesect.span / section_span) - curr_twist_total;
				curr_twist_total += (section_twist - previous_twist) * (bridgesect.span / section_span);
				bridgesect.twistLoc = section_twistloc;
				bridgesect.sweepLoc = section_sweeploc;
	
				span_percentage_sum += (bridgesect.span/sects[i-1].span);

				// = Determine sweep and dihedral for bridge subsection = //
				if(sweep_flag == SMALLEST)
					bridgesect.sweep = section_sweep + -1*subsection_sweep*(num_sweep_subsections);
				else if(te_sweep_flag == SMALLEST)
					te_sweep = section_te_sweep + -1*subsection_te_sweep*(num_te_sweep_subsections);
				else
					bridgesect.dihedral = section_dihedral + -1*subsection_dihedral*(num_dihedral_subsections);

				if(sweep_flag == MODERATE)
					bridgesect.sweep = section_sweep + -1*subsection_sweep*(moderate_index+1);
				else if(te_sweep_flag == MODERATE)
					te_sweep = section_te_sweep + -1*subsection_te_sweep*(moderate_index+1);
				else
					bridgesect.dihedral = section_dihedral + -1*subsection_dihedral*(moderate_index+1);

				if( largest_subsection_total_span > smallest_subsection_total_span )
				{				
					if( (smallest_subsection_total_span + bridgesect.span) > (total_largest_subsection_span + largest_subsection_span) &&
					    fabs( (smallest_subsection_total_span + bridgesect.span) - (total_largest_subsection_span + largest_subsection_span) ) > 0.0001 )
					{
						if(sweep_flag == LARGEST)
							bridgesect.sweep = section_sweep + -1*(subsection_sweep*(largest_index+1) + subsection_sweep*(largest_index+2))/2;
						else if(te_sweep_flag == LARGEST)
							te_sweep = section_te_sweep + -1*(subsection_te_sweep*(largest_index+1) + subsection_te_sweep*(largest_index+2))/2;
						else
							bridgesect.dihedral = section_dihedral + -1*(subsection_dihedral*(largest_index+1) + subsection_dihedral*  											(largest_index+2))/2;

						total_largest_subsection_span += largest_subsection_span;
						largest_index++;
					}
					else
					{
						if(sweep_flag == LARGEST)
							bridgesect.sweep = section_sweep + -1*subsection_sweep*(largest_index+1);
						else if(te_sweep_flag == LARGEST)
							te_sweep = section_te_sweep + -1*subsection_te_sweep*(largest_index+1);
						else
							bridgesect.dihedral = section_dihedral + -1*subsection_dihedral*(largest_index+1);
					}
				}
				else
				{
					if(sweep_flag == LARGEST)
						bridgesect.sweep = section_sweep + -1*subsection_sweep*(largest_index+1);
					else if(te_sweep_flag == LARGEST)
						te_sweep = section_te_sweep + -1*subsection_te_sweep*(largest_index+1);
					else
						bridgesect.dihedral = section_dihedral + -1*subsection_dihedral*(largest_index+1);

				}

				double new_thickness = sects[i].rootAf->get_thickness()->get() - span_percentage_sum*thickness_difference;

				// = Set airfoil for bridge subsection = //
				if( ((moderate_index+1) - num_moderate_subsections == 0)  && (largest_subsection_total_span <= moderate_subsection_total_span) )
				{
					bridgesect.rootAf = sub_sects[sub_sects.dimension()-1].tipAf;
				}
				else
				{
					Af* newaf = new Af( this );
					bridgesect.rootAf = newaf;
					sub_sect_foils.append(newaf);
					bridgesect.rootAf->init_script("wing_foil");
					*(bridgesect.rootAf) = *(sects[i-1].tipAf);
					bridgesect.rootAf->sterilize_parms();
					bridgesect.rootAf->get_thickness()->set(new_thickness);
					bridgesect.rootAf->generate_airfoil();
					bridgesect.rootAf->load_name();
				}
				
				if(temp_subsects.dimension() == 0)
				{
					bridgesect.tipAf = new Af(this);
					*(bridgesect.tipAf) = *(sects[i-1].tipAf);
					bridgesect.tipAf->sterilize_parms();
					sub_sect_foils.append(bridgesect.tipAf);
				}
				else
					bridgesect.tipAf = temp_subsects[temp_subsects.dimension()-1].rootAf;

				bridgesect.num_interp_xsecs = sects[i-1].num_interp_xsecs;

				bridgesect.fillDependData();
				temp_subsects.append( bridgesect );
				te_sweep_list.append( te_sweep );
		
				num_subsections++;
				total_num_subsections++;
				moderate_index++;	
				total_moderate_subsection_span += moderate_subsection_span;
			}
			else
			{
				moderate_index++;	
				total_moderate_subsection_span += moderate_subsection_span;
			}

			for(int k = moderate_index; k < num_moderate_subsections; k++)
			{
				HwbSect wingsect;
				wingsect.driver = MS_S_TC_RC;

				wingsect.span = moderate_subsection_span;
				wingsect.twist = total_twist + section_twist - (section_twist - previous_twist) * (wingsect.span / section_span) - curr_twist_total;
				curr_twist_total += (section_twist - previous_twist) * (wingsect.span / section_span);
				wingsect.twistLoc = section_twistloc;
				wingsect.sweepLoc = section_sweeploc;

				span_percentage_sum += (wingsect.span/sects[i-1].span);

				// = Determine sweep and dihedral for bridge subsection = //
				if(sweep_flag == SMALLEST)
					wingsect.sweep = section_sweep + -1*subsection_sweep*(num_sweep_subsections);
				else if(te_sweep_flag == SMALLEST)
					te_sweep = section_te_sweep + -1*subsection_te_sweep*(num_te_sweep_subsections);
				else
					wingsect.dihedral = section_dihedral + -1*subsection_dihedral*(num_dihedral_subsections);

				if(sweep_flag == MODERATE)
					wingsect.sweep = section_sweep + -1*subsection_sweep*(k+1);
				else if(te_sweep_flag == MODERATE)
					te_sweep = section_te_sweep + -1*subsection_te_sweep*(k+1);
				else
					wingsect.dihedral = section_dihedral + -1*subsection_dihedral*(k+1);

				if( largest_subsection_total_span > smallest_subsection_total_span )
				{
					double moderate_total = total_moderate_subsection_span + moderate_subsection_span;
					double largest_total = total_largest_subsection_span + largest_subsection_span;

					if( moderate_total > largest_total && fabs( moderate_total - largest_total ) && (largest_total < largest_subsection_total_span) )
					{
						if(sweep_flag == LARGEST)
						{
							if( fabs(sweep_edge_length_percentage) > 0.0001 )
								wingsect.sweep = section_sweep + -1*(subsection_sweep*(largest_index+1) + subsection_sweep*(largest_index+2))/2;
							else
								wingsect.sweep = section_sweep + -1*subsection_sweep*(largest_index+1);
						}
						else if(te_sweep_flag == LARGEST)
						{
							if( fabs(te_sweep_edge_length_percentage) > 0.0001 )
								te_sweep = section_te_sweep + -1*(subsection_te_sweep*(largest_index+1) + subsection_te_sweep*(largest_index+2))/2;
							else
								te_sweep = section_te_sweep + -1*subsection_te_sweep*(largest_index+1);
						}
						else
						{
							if( fabs(sweep_edge_length_percentage) > 0.0001 )
								wingsect.dihedral = section_dihedral + -1*(subsection_dihedral*(largest_index+1) + subsection_dihedral*(largest_index+2))/2;
							else
								wingsect.dihedral = section_dihedral + -1*subsection_dihedral*(largest_index+1);
						}

						total_largest_subsection_span += largest_subsection_span;
						largest_index++;
					}
					else
					{
						if(sweep_flag == LARGEST)
							wingsect.sweep = section_sweep + -1*subsection_sweep*(largest_index+1);
						else if(te_sweep_flag == LARGEST)
							te_sweep = section_te_sweep +-1* subsection_te_sweep*(largest_index+1);
						else
							wingsect.dihedral = section_dihedral + -1*subsection_dihedral*(largest_index+1);
					}
				}
				else
				{
					if(sweep_flag == LARGEST)
						wingsect.sweep = section_sweep + -1*subsection_sweep*(largest_index+1);
					else if(te_sweep_flag == LARGEST)
						te_sweep = section_te_sweep +-1* subsection_te_sweep*(largest_index+1);
					else
						wingsect.dihedral = section_dihedral + -1*subsection_dihedral*(largest_index+1);
				}

				double new_thickness = sects[i].rootAf->get_thickness()->get() - span_percentage_sum*thickness_difference;

				// = Set airfoil for bridge subsection = //
				if( ((moderate_index+1) - num_moderate_subsections == 0)  && (largest_subsection_total_span <= moderate_subsection_total_span) )
				{
					wingsect.rootAf = sub_sects[sub_sects.dimension()-1].tipAf;
				}
				else
				{
					Af* newaf = new Af( this );
					wingsect.rootAf = newaf;
					sub_sect_foils.append(newaf);
					wingsect.rootAf->init_script("wing_foil");
					*(wingsect.rootAf) = *(sects[i-1].tipAf);
					wingsect.rootAf->sterilize_parms();
					wingsect.rootAf->get_thickness()->set(new_thickness);
					wingsect.rootAf->generate_airfoil();
					wingsect.rootAf->load_name();
				}	

				if(temp_subsects.dimension() == 0)
				{
					wingsect.tipAf = new Af(this);
					*(wingsect.tipAf) = *(sects[i-1].tipAf);
					wingsect.tipAf->sterilize_parms();
					sub_sect_foils.append(wingsect.tipAf);
				}
				else
					wingsect.tipAf = temp_subsects[temp_subsects.dimension()-1].rootAf;	

				wingsect.num_interp_xsecs = sects[i-1].num_interp_xsecs;

				wingsect.fillDependData();
				temp_subsects.append( wingsect );
				te_sweep_list.append( te_sweep );
				total_moderate_subsection_span += moderate_subsection_span;

				num_subsections++;
				total_num_subsections++;
			}

			if( largest_subsection_total_span > moderate_subsection_total_span )
			{
				HwbSect bridgesect2;
				bridgesect2.driver = MS_S_TC_RC;

				if( ( total_largest_subsection_span + largest_subsection_span - moderate_subsection_total_span ) > 0.0001 )
				{
					bridgesect2.span = total_largest_subsection_span + largest_subsection_span - moderate_subsection_total_span;
					
					bridgesect2.twist = total_twist + section_twist - (section_twist - previous_twist) * (bridgesect2.span / section_span) - curr_twist_total;
					curr_twist_total += (section_twist - previous_twist) * (bridgesect2.span / section_span);
					bridgesect2.twistLoc = section_twistloc;
					bridgesect2.sweepLoc = section_sweeploc;
	
					span_percentage_sum += (bridgesect2.span/sects[i-1].span);

					// = Determine sweep and dihedral for bridge subsection = //
					if(sweep_flag == SMALLEST)
						bridgesect2.sweep = section_sweep + -1*subsection_sweep*(num_sweep_subsections);
					else if(te_sweep_flag == SMALLEST)
						te_sweep = section_te_sweep + -1*subsection_te_sweep*(num_te_sweep_subsections);
					else
						bridgesect2.dihedral = section_dihedral + -1*subsection_dihedral*(num_dihedral_subsections);
			
					if(sweep_flag == MODERATE)
						bridgesect2.sweep = section_sweep + -1*subsection_sweep*(num_sweep_subsections);
					else if(te_sweep_flag == MODERATE)
						te_sweep = section_te_sweep + -1*subsection_te_sweep*(num_te_sweep_subsections);
					else
						bridgesect2.dihedral = section_dihedral + -1*subsection_dihedral*(num_dihedral_subsections);

					if(sweep_flag == LARGEST)
						bridgesect2.sweep = section_sweep + -1*subsection_sweep*(largest_index+1);
					else if(te_sweep_flag == LARGEST)
						te_sweep = section_te_sweep + -1*subsection_te_sweep*(largest_index+1);
					else
						bridgesect2.dihedral = section_dihedral + -1*subsection_dihedral*(largest_index+1);
	
					double new_thickness = sects[i].rootAf->get_thickness()->get() - span_percentage_sum*thickness_difference;
	
					// = Set airfoil for bridge subsection = //
					if( ((largest_index+1) - num_largest_subsections) == 0 )
					{
						bridgesect2.rootAf = sub_sects[sub_sects.dimension()-1].tipAf;
					}
					else
					{
						Af* newaf = new Af( this );
						bridgesect2.rootAf = newaf;
						sub_sect_foils.append(newaf);
						bridgesect2.rootAf->init_script("wing_foil");
						*(bridgesect2.rootAf) = *(sects[i-1].tipAf);
						bridgesect2.rootAf->sterilize_parms();
						bridgesect2.rootAf->get_thickness()->set(new_thickness);
						bridgesect2.rootAf->generate_airfoil();
						bridgesect2.rootAf->load_name();
					}
					
					if(temp_subsects.dimension() == 0)
					{
						bridgesect2.tipAf = new Af(this);
						*(bridgesect2.tipAf) = *(sects[i-1].tipAf);
						bridgesect2.tipAf->sterilize_parms();
						sub_sect_foils.append(bridgesect2.tipAf);
					}
					else
						bridgesect2.tipAf = temp_subsects[temp_subsects.dimension()-1].rootAf;			

					bridgesect2.num_interp_xsecs = sects[i-1].num_interp_xsecs;

					bridgesect2.fillDependData();
					temp_subsects.append( bridgesect2 );
					te_sweep_list.append( te_sweep );

					num_subsections++;
					total_num_subsections++;
					largest_index++;
					total_largest_subsection_span += largest_subsection_span;
				}
				else
				{
					largest_index++;	
					total_largest_subsection_span += largest_subsection_span;
				}

				for(int k = largest_index; k < num_largest_subsections; k++)
				{
					HwbSect wingsect;
					wingsect.driver = MS_S_TC_RC;

					wingsect.span = largest_subsection_span;
					wingsect.twist = total_twist + section_twist - (section_twist - previous_twist) * (wingsect.span / section_span) - curr_twist_total;
					curr_twist_total += (section_twist - previous_twist) * (wingsect.span / section_span);
					wingsect.twistLoc = section_twistloc;
					wingsect.sweepLoc = section_sweeploc;

					span_percentage_sum += (wingsect.span/sects[i-1].span);

					// = Determine sweep and dihedral for bridge subsection = //
					if(sweep_flag == SMALLEST)
						wingsect.sweep = section_sweep + -1*subsection_sweep*(num_sweep_subsections);
					else if(te_sweep_flag == SMALLEST)
						te_sweep = section_te_sweep + -1*subsection_te_sweep*(num_te_sweep_subsections);
					else
						wingsect.dihedral = section_dihedral + -1*subsection_dihedral*(num_dihedral_subsections);

					if(sweep_flag == MODERATE)
						wingsect.sweep = section_sweep + -1*subsection_sweep*(num_sweep_subsections);
					else if(te_sweep_flag == MODERATE)
						te_sweep = section_te_sweep + -1*subsection_te_sweep*(num_te_sweep_subsections);
					else
						wingsect.dihedral = section_dihedral + -1*subsection_dihedral*(num_dihedral_subsections);

					if(sweep_flag == LARGEST)
						wingsect.sweep = section_sweep + -1*subsection_sweep*(k+1);
					else if(te_sweep_flag == LARGEST)
						te_sweep = section_te_sweep + -1*subsection_te_sweep*(k+1);
					else
						wingsect.dihedral = section_dihedral + -1*subsection_dihedral*(k+1);

					double new_thickness = sects[i].rootAf->get_thickness()->get() - span_percentage_sum*thickness_difference;

					// = Set airfoil for bridge subsection = //
					if( k == num_largest_subsections-1 )
					{
						wingsect.rootAf = sub_sects[sub_sects.dimension()-1].tipAf;
					}
					else
					{
						Af* newaf = new Af( this );
						wingsect.rootAf = newaf;
						sub_sect_foils.append(newaf);
						wingsect.rootAf->init_script("wing_foil");
						*(wingsect.rootAf) = *(sects[i-1].tipAf);
						wingsect.rootAf->sterilize_parms();
						wingsect.rootAf->get_thickness()->set(new_thickness);
						wingsect.rootAf->generate_airfoil();
						wingsect.rootAf->load_name();
					}

					
					if(temp_subsects.dimension() == 0)
					{
						wingsect.tipAf = new Af(this);
						*(wingsect.tipAf) = *(sects[i-1].tipAf);
						wingsect.tipAf->sterilize_parms();
						sub_sect_foils.append(wingsect.tipAf);
					}
					else
						wingsect.tipAf = temp_subsects[temp_subsects.dimension()-1].rootAf;

					wingsect.num_interp_xsecs = sects[i-1].num_interp_xsecs;

					wingsect.fillDependData();
					temp_subsects.append( wingsect );
					te_sweep_list.append( te_sweep );

					num_subsections++;
					total_num_subsections++;
				}
			}			
		}
		else if( largest_subsection_total_span > smallest_subsection_total_span && (num_smallest_subsections > 0) )
		{
			// = Bridge subsection connects micro subsections to macro subsections = //
			double te_sweep;
			HwbSect bridgesect;
			bridgesect.driver = MS_S_TC_RC;

			if( ( total_largest_subsection_span + largest_subsection_span - smallest_subsection_total_span ) > 0.0001 )
			{
				bridgesect.span = total_largest_subsection_span + largest_subsection_span - smallest_subsection_total_span;

				bridgesect.twist = total_twist + section_twist - (section_twist - previous_twist) * (bridgesect.span / section_span) - curr_twist_total;
				curr_twist_total += (section_twist - previous_twist) * (bridgesect.span / section_span);
				bridgesect.twistLoc = section_twistloc;
				bridgesect.sweepLoc = section_sweeploc;

				span_percentage_sum += (bridgesect.span/sects[i-1].span);

				if(sweep_flag == SMALLEST)
					bridgesect.sweep = section_sweep + -1*subsection_sweep*num_sweep_subsections;
				else if(te_sweep_flag == SMALLEST)
					te_sweep = section_te_sweep + -1*subsection_te_sweep*num_te_sweep_subsections;
				else
					bridgesect.dihedral = section_dihedral + -1*subsection_dihedral*num_dihedral_subsections;

				if(sweep_flag == MODERATE)
					bridgesect.sweep = section_sweep + -1*subsection_sweep*num_sweep_subsections;
				else if(te_sweep_flag == MODERATE)
					te_sweep = section_te_sweep + -1*subsection_te_sweep*num_te_sweep_subsections;
				else
					bridgesect.dihedral = section_dihedral + -1*subsection_dihedral*num_dihedral_subsections;

				if(sweep_flag == LARGEST)
					bridgesect.sweep = section_sweep + -1*subsection_sweep*(largest_index+1);
				else if(te_sweep_flag == LARGEST)
					te_sweep = section_te_sweep + -1*subsection_te_sweep*(largest_index+1);
				else
					bridgesect.dihedral = section_dihedral + -1*subsection_dihedral*(largest_index+1);

				double new_thickness = sects[i].rootAf->get_thickness()->get() - span_percentage_sum*thickness_difference;
		
				// = Set airfoil for bridge subsection = //
				if( ((largest_index+1) - num_largest_subsections) == 0 )
				{
					bridgesect.rootAf = sub_sects[sub_sects.dimension()-1].tipAf;
				}
				else
				{
					Af* newaf = new Af( this );
					bridgesect.rootAf = newaf;
					sub_sect_foils.append(newaf);
					bridgesect.rootAf->init_script("wing_foil");
					*(bridgesect.rootAf) = *(sects[i-1].tipAf);
					bridgesect.rootAf->sterilize_parms();
					bridgesect.rootAf->get_thickness()->set(new_thickness);
					bridgesect.rootAf->generate_airfoil();					
					bridgesect.rootAf->load_name();
				}
			
				if(temp_subsects.dimension() == 0)
				{
					bridgesect.tipAf = new Af(this);
					*(bridgesect.tipAf) = *(sects[i-1].tipAf);
					bridgesect.tipAf->sterilize_parms();
					sub_sect_foils.append(bridgesect.tipAf);
				}
				else
					bridgesect.tipAf = temp_subsects[temp_subsects.dimension()-1].rootAf;

				bridgesect.num_interp_xsecs = sects[i-1].num_interp_xsecs;

				bridgesect.fillDependData();
				temp_subsects.append( bridgesect );
				te_sweep_list.append( te_sweep );

				num_subsections++;
				total_num_subsections++;
				largest_index++;
				total_largest_subsection_span += largest_subsection_span;
			}
			else
			{
				largest_index++;	
				total_largest_subsection_span += largest_subsection_span;
			}

			for(int k = largest_index; k < num_largest_subsections; k++)
			{
				HwbSect wingsect;
				wingsect.driver = MS_S_TC_RC;

				wingsect.span = largest_subsection_span;
				wingsect.twist = total_twist + section_twist - (section_twist - previous_twist) * (wingsect.span / section_span) - curr_twist_total;
				curr_twist_total += (section_twist - previous_twist) * (wingsect.span / section_span);
				wingsect.twistLoc = section_twistloc;
				wingsect.sweepLoc = section_sweeploc;

				span_percentage_sum += (wingsect.span/sects[i-1].span);

				// = Determine sweep and dihedral for bridge subsection = //
				if(sweep_flag == SMALLEST)
					wingsect.sweep = section_sweep + -1*subsection_sweep*(num_sweep_subsections);
				else if(te_sweep_flag == SMALLEST)
					te_sweep = section_te_sweep + -1*subsection_te_sweep*(num_te_sweep_subsections);
				else
					wingsect.dihedral = section_dihedral + -1*subsection_dihedral*(num_dihedral_subsections);

				if(sweep_flag == MODERATE)
					wingsect.sweep = section_sweep + -1*subsection_sweep*(num_sweep_subsections);
				else if(te_sweep_flag == MODERATE)
					te_sweep = section_te_sweep + -1*subsection_te_sweep*(num_te_sweep_subsections);
				else
					wingsect.dihedral = section_dihedral + -1*subsection_dihedral*(num_dihedral_subsections);
	
				if(sweep_flag == LARGEST)
					wingsect.sweep = section_sweep + -1*subsection_sweep*(k+1);
				else if(te_sweep_flag == LARGEST)
					te_sweep = section_te_sweep + -1*subsection_te_sweep*(k+1);
				else
					wingsect.dihedral = section_dihedral + -1*subsection_dihedral*(k+1);

				double new_thickness = sects[i].rootAf->get_thickness()->get() - span_percentage_sum*thickness_difference;

				// = Set airfoil for bridge subsection = //
		
				if( k == (num_largest_subsections-1) )
				{
					wingsect.rootAf = sub_sects[sub_sects.dimension()-1].tipAf;
				}
				else
				{
					Af* newaf = new Af( this );
					wingsect.rootAf = newaf;
					sub_sect_foils.append(newaf);
					wingsect.rootAf->init_script("wing_foil");
					*(wingsect.rootAf) = *(sects[i-1].tipAf);
					wingsect.rootAf->sterilize_parms();
					wingsect.rootAf->get_thickness()->set(new_thickness);
					wingsect.rootAf->generate_airfoil();
					wingsect.rootAf->load_name();
				}
			
				if(temp_subsects.dimension() == 0)
				{
					wingsect.tipAf = new Af(this);
					*(wingsect.tipAf) = *(sects[i-1].tipAf);
					wingsect.tipAf->sterilize_parms();
					sub_sect_foils.append(wingsect.tipAf);
				}
				else
					wingsect.tipAf = temp_subsects[temp_subsects.dimension()-1].rootAf;

				wingsect.num_interp_xsecs = sects[i-1].num_interp_xsecs;

				wingsect.fillDependData();
				temp_subsects.append( wingsect );
				te_sweep_list.append( te_sweep );

				num_subsections++;
				total_num_subsections++;
			}
		}

		double all_span = 0.0;
		double alt_chord = sub_sects[sub_sects.dimension()-1].tc;

		for(int k = temp_subsects.dimension()-1; k >= 0; k--)
		{
			temp_subsects[k].rc = sub_sects[sub_sects.dimension()-1].tc;			

			double sub_sectional_sweep_offset = temp_subsects[k].span*tan(temp_subsects[k].sweep*DEG_2_RAD);
			double sub_sectional_te_sweep_offset = temp_subsects[k].span*tan(te_sweep_list[k]*DEG_2_RAD);

			temp_subsects[k].tc = temp_subsects[k].rc - sub_sectional_sweep_offset - sub_sectional_te_sweep_offset;

			sub_sectional_sweep_offset = temp_subsects[k].span*tan(previous_sweep*DEG_2_RAD);
			sub_sectional_te_sweep_offset = temp_subsects[k].span*tan(previous_te_sweep*DEG_2_RAD);

			double new_value = alt_chord - sub_sectional_sweep_offset - sub_sectional_te_sweep_offset;

			alt_chord = new_value;
			all_span += temp_subsects[k].span;
			sub_sects.append(temp_subsects[k]);
		}

		section_sizes.append(num_subsections);
		num_subsections = 0;

		previous_sweep = section_sweep;
		previous_te_sweep = section_te_sweep;
		previous_dihedral = section_dihedral;
		previous_sweep_offset = section_sweep_offset;
		previous_te_sweep_offset = section_te_sweep_offset;
		previous_dihedral_offset = section_dihedral_offset;
		previous_twist = section_twist;

		section_rc = sects[i].rc;
		section_tc = sects[i].tc;	
		section_sweep = atan(sects[i].tan_sweep_at(0.0,getSymCode()))*RAD_2_DEG;
		section_dihedral = sects[i].dihedral;
		section_twist = sects[i].twist;
		section_twistloc = sects[i].twistLoc;
		section_span = sects[i].span;
		section_sweep_offset = section_span*tan(section_sweep*DEG_2_RAD);
		section_dihedral_offset = section_span*sin(section_dihedral*DEG_2_RAD);
		section_dihedral_offset_x = section_span*cos(section_dihedral*DEG_2_RAD);
		section_te_sweep = atan( (section_rc - section_sweep_offset - section_tc)/section_span )*RAD_2_DEG;
		section_te_sweep_offset = section_span*tan(section_te_sweep*DEG_2_RAD);
		total_twist += previous_twist;

		// = Find number of root subsections for sweep and dihedral = //
		num_sweep_subsections = (int)ceil( fabs(section_sweep - previous_sweep) / sweep_degrees_perseg[i]->iget() );
		num_te_sweep_subsections = (int)ceil( fabs(section_te_sweep - previous_te_sweep) / tesweep_degrees_perseg[i]->iget() );
		num_dihedral_subsections = (int)ceil( fabs(section_dihedral - previous_dihedral) / dihedral_degrees_perseg[i]->iget() );
	
		if(fabs(sweep_edge_length_percentage) < 0.0001)
			num_sweep_subsections = 0;
	
		if(fabs(te_sweep_edge_length_percentage) < 0.0001)
			num_te_sweep_subsections = 0;

		if(fabs(dihedral_edge_length_percentage) < 0.0001)
			num_dihedral_subsections = 0;

		if(num_sweep_subsections != 0 || num_te_sweep_subsections != 0 || num_dihedral_subsections != 0)
		{
			if(num_sweep_subsections != 0)
				sweep_subsection_arc_angle = root_sweep_arc_angle / num_sweep_subsections;
			else
				sweep_subsection_arc_angle = root_sweep_arc_angle;

			if(num_te_sweep_subsections != 0)
				te_sweep_subsection_arc_angle = root_te_sweep_arc_angle / num_te_sweep_subsections;
			else
				te_sweep_subsection_arc_angle = root_te_sweep_arc_angle;

			if(num_dihedral_subsections != 0)
				dihedral_subsection_arc_angle = root_dihedral_arc_angle / num_dihedral_subsections;
			else
				dihedral_subsection_arc_angle = root_dihedral_arc_angle;
		}

		subsection_sweep = (section_sweep - previous_sweep) / ( num_sweep_subsections + 1 );
		subsection_te_sweep = (section_te_sweep - previous_te_sweep) / ( num_te_sweep_subsections + 1 );
		subsection_dihedral = (section_dihedral - previous_dihedral) / ( num_dihedral_subsections + 1 );

		if(num_sweep_subsections > 0)
			first_sweep_subsection_span = 2*sweep_circle_radius*sin( sweep_subsection_arc_angle/2 * DEG_2_RAD ) * cos( (previous_sweep + subsection_sweep) * DEG_2_RAD );
		else
			first_sweep_subsection_span = section_span;

		if(num_te_sweep_subsections > 0)
			first_te_sweep_subsection_span = 2*te_sweep_circle_radius*sin( te_sweep_subsection_arc_angle/2 * DEG_2_RAD ) * cos( (previous_te_sweep + subsection_te_sweep) * DEG_2_RAD );
		else
			first_te_sweep_subsection_span = section_span;

		if(num_dihedral_subsections > 0)
			first_dihedral_subsection_span = 2*dihedral_circle_radius*sin( dihedral_subsection_arc_angle/2 * DEG_2_RAD );
		else
			first_dihedral_subsection_span = section_span;

		sweep_flag = SMALLEST; 
		te_sweep_flag = SMALLEST; 
		dihedral_flag = SMALLEST;

		if( (first_sweep_subsection_span < first_te_sweep_subsection_span) && (first_sweep_subsection_span < first_dihedral_subsection_span) )
		{
			num_smallest_subsections = num_sweep_subsections;
			smallest_subsection_arc_angle = sweep_subsection_arc_angle;

			if(sweep_previous_edge)
				smallest_subsection_total_span = sects[i-1].span * sweep_edge_length_percentage;			
			else
				smallest_subsection_total_span = sects[i].span * sweep_edge_length_percentage;	

			if(first_te_sweep_subsection_span < first_dihedral_subsection_span)
			{
				num_moderate_subsections = num_te_sweep_subsections;
				moderate_subsection_arc_angle = te_sweep_subsection_arc_angle;
				te_sweep_flag = MODERATE;
		
				if(te_sweep_previous_edge)
					moderate_subsection_total_span = sects[i-1].span * te_sweep_edge_length_percentage;			
				else
					moderate_subsection_total_span = sects[i].span * te_sweep_edge_length_percentage;
				
				num_largest_subsections = num_dihedral_subsections;
				largest_subsection_arc_angle = dihedral_subsection_arc_angle;
				dihedral_flag = LARGEST;

				if(dihedral_previous_edge)
					largest_subsection_total_span = sects[i-1].span*dihedral_edge_length_percentage;			
				else
					largest_subsection_total_span = sects[i].span*dihedral_edge_length_percentage;
			}
			else
			{
				num_moderate_subsections = num_dihedral_subsections;
				moderate_subsection_arc_angle = dihedral_subsection_arc_angle;
				dihedral_flag = MODERATE;

				if(dihedral_previous_edge)
					moderate_subsection_total_span = sects[i-1].span*dihedral_edge_length_percentage;			
				else
					moderate_subsection_total_span = sects[i].span*dihedral_edge_length_percentage;

				num_largest_subsections = num_te_sweep_subsections;
				largest_subsection_arc_angle = te_sweep_subsection_arc_angle;
				te_sweep_flag = LARGEST;

				if(te_sweep_previous_edge)
					largest_subsection_total_span = sects[i-1].span * te_sweep_edge_length_percentage;			
				else
					largest_subsection_total_span = sects[i].span * te_sweep_edge_length_percentage;
			}
		}
		else if( (first_te_sweep_subsection_span < first_sweep_subsection_span) && (first_te_sweep_subsection_span < first_dihedral_subsection_span) )
		{
			num_smallest_subsections = num_te_sweep_subsections;
			smallest_subsection_arc_angle = te_sweep_subsection_arc_angle;

			if(te_sweep_previous_edge)
				smallest_subsection_total_span = sects[i-1].span * te_sweep_edge_length_percentage;			
			else
				smallest_subsection_total_span = sects[i].span * te_sweep_edge_length_percentage;

			if(first_sweep_subsection_span < first_dihedral_subsection_span)
			{
				num_moderate_subsections = num_sweep_subsections;
				moderate_subsection_arc_angle = sweep_subsection_arc_angle;
				sweep_flag = MODERATE;

				if(sweep_previous_edge)
					moderate_subsection_total_span = sects[i-1].span * sweep_edge_length_percentage;			
				else
					moderate_subsection_total_span = sects[i].span * sweep_edge_length_percentage;
	
				num_largest_subsections = num_dihedral_subsections;
				largest_subsection_arc_angle = dihedral_subsection_arc_angle;
				dihedral_flag = LARGEST;

				if(dihedral_previous_edge)
					largest_subsection_total_span = sects[i-1].span*dihedral_edge_length_percentage;			
				else
					largest_subsection_total_span = sects[i].span*dihedral_edge_length_percentage;
			}
			else
			{
				num_moderate_subsections = num_dihedral_subsections;
				moderate_subsection_arc_angle = dihedral_subsection_arc_angle;
				dihedral_flag = MODERATE;

				if(dihedral_previous_edge)
					moderate_subsection_total_span = sects[i-1].span*dihedral_edge_length_percentage;			
				else
					moderate_subsection_total_span = sects[i].span*dihedral_edge_length_percentage;
	
				num_largest_subsections = num_sweep_subsections;
				largest_subsection_arc_angle = sweep_subsection_arc_angle;
				sweep_flag = LARGEST;

				if(sweep_previous_edge)
					largest_subsection_total_span = sects[i-1].span * sweep_edge_length_percentage;			
				else
					largest_subsection_total_span = sects[i].span * sweep_edge_length_percentage;
			}
		}
		else
		{
			num_smallest_subsections = num_dihedral_subsections;
			smallest_subsection_arc_angle = dihedral_subsection_arc_angle;

			if(dihedral_previous_edge)
				smallest_subsection_total_span = sects[i-1].span*dihedral_edge_length_percentage;			
			else
				smallest_subsection_total_span = sects[i].span*dihedral_edge_length_percentage;

			if(first_sweep_subsection_span < first_te_sweep_subsection_span)
			{
				num_moderate_subsections = num_sweep_subsections;
				moderate_subsection_arc_angle = sweep_subsection_arc_angle;
				sweep_flag = MODERATE;

				if(sweep_previous_edge)
					moderate_subsection_total_span = sects[i-1].span * sweep_edge_length_percentage;			
				else
					moderate_subsection_total_span = sects[i].span * sweep_edge_length_percentage;

				num_largest_subsections = num_te_sweep_subsections;
				largest_subsection_arc_angle = te_sweep_subsection_arc_angle;
				te_sweep_flag = LARGEST;

				if(te_sweep_previous_edge)
					largest_subsection_total_span = sects[i-1].span * te_sweep_edge_length_percentage;		
				else
					largest_subsection_total_span = sects[i].span * te_sweep_edge_length_percentage;
			}
			else
			{
				num_moderate_subsections = num_te_sweep_subsections;
				moderate_subsection_arc_angle = te_sweep_subsection_arc_angle;
				te_sweep_flag = MODERATE;

				if(te_sweep_previous_edge)
					moderate_subsection_total_span = sects[i-1].span * te_sweep_edge_length_percentage;			
				else
					moderate_subsection_total_span = sects[i].span * te_sweep_edge_length_percentage;

				num_largest_subsections = num_sweep_subsections;
				largest_subsection_arc_angle = sweep_subsection_arc_angle;
				sweep_flag = LARGEST;

				if(sweep_previous_edge)
					largest_subsection_total_span = sects[i-1].span * sweep_edge_length_percentage;			
				else
					largest_subsection_total_span = sects[i].span * sweep_edge_length_percentage;
			}
		}

		total_largest_subsection_span = 0.0;
		total_moderate_subsection_span = 0.0;
		total_smallest_subsection_span = 0.0;
		moderate_index = 0;
		largest_index = 0;

		if( num_smallest_subsections != 0  )
		{
			if( num_moderate_subsections == 0 && num_largest_subsections == 0 )
			{
				num_moderate_subsections = 1;
				moderate_subsection_total_span = smallest_subsection_total_span;
				total_moderate_subsection_span = moderate_subsection_total_span;

				num_largest_subsections = 1;
				largest_subsection_total_span = smallest_subsection_total_span;	
				total_largest_subsection_span = largest_subsection_total_span;
			}
			else if( num_moderate_subsections == 0 && num_largest_subsections != 0 )
			{
				num_moderate_subsections = 1;

				if(largest_subsection_total_span > smallest_subsection_total_span)
					moderate_subsection_total_span = largest_subsection_total_span;
				else
					moderate_subsection_total_span = smallest_subsection_total_span;

				total_moderate_subsection_span = moderate_subsection_total_span;
			}
			else if( num_moderate_subsections != 0 && num_largest_subsections == 0 )
			{
				num_largest_subsections = 1;

				if(moderate_subsection_total_span > smallest_subsection_total_span)
					largest_subsection_total_span = moderate_subsection_total_span;
				else
					largest_subsection_total_span = smallest_subsection_total_span;

				total_largest_subsection_span = largest_subsection_total_span;
			}

		}

		if(smallest_subsection_total_span > moderate_subsection_total_span && smallest_subsection_total_span > largest_subsection_total_span)
			previous_root_span = smallest_subsection_total_span;
		else if(moderate_subsection_total_span > smallest_subsection_total_span && moderate_subsection_total_span > largest_subsection_total_span)
			previous_root_span = moderate_subsection_total_span;		
		else	
			previous_root_span = largest_subsection_total_span;

		total_span += section_span;
		total_sweep_offset += section_sweep_offset;
		total_te_sweep_offset += section_te_sweep_offset;
		total_dihedral_offset += section_dihedral_offset;
		total_dihedral_offset_x += section_dihedral_offset_x;

		span_percentage_sum = 0.0;
		thickness_difference = sects[i].tipAf->get_thickness()->get() - sects[i].rootAf->get_thickness()->get();

		smallest_subsection_span = (smallest_subsection_total_span/num_smallest_subsections);
		moderate_subsection_span = (moderate_subsection_total_span/num_moderate_subsections);
		largest_subsection_span = (largest_subsection_total_span/num_largest_subsections);

		for(int j = 0; j < num_smallest_subsections; j++)
		{
			HwbSect wingsect;
			double te_sweep = 0.0;
			wingsect.driver = MS_S_TC_RC;	

			wingsect.span = smallest_subsection_span;
			wingsect.twist = total_twist + curr_twist_total + (section_twist - previous_twist) * (wingsect.span / section_span);
			curr_twist_total += (section_twist - previous_twist) * (wingsect.span / section_span);
			wingsect.twistLoc = section_twistloc;
			wingsect.sweepLoc = section_sweeploc;

			span_percentage_sum += (wingsect.span/sects[i].span);

			double smallest_total = total_smallest_subsection_span + smallest_subsection_span;
			double moderate_total = total_moderate_subsection_span + moderate_subsection_span;

			if( ( smallest_total > moderate_total ) && fabs( smallest_total - moderate_total ) > 0.00001 && (moderate_total < moderate_subsection_total_span) )
			{
				if(sweep_flag == MODERATE)
				{
					if( (moderate_index+1) >= num_moderate_subsections)
						wingsect.sweep = previous_sweep + subsection_sweep*(num_moderate_subsections);
					else
						wingsect.sweep = previous_sweep + (subsection_sweep*(moderate_index+1) + subsection_sweep*(moderate_index+2))/2;
				}
				else if(te_sweep_flag == MODERATE)
				{
					if( (moderate_index+1) >= num_moderate_subsections)
						te_sweep = previous_te_sweep + subsection_te_sweep*(num_moderate_subsections);
					else
						te_sweep = previous_te_sweep + (subsection_te_sweep*(moderate_index+1) + subsection_te_sweep*(moderate_index+2))/2;
				}
				else
				{
					if( (moderate_index+1) >= num_moderate_subsections)
						wingsect.dihedral = previous_dihedral + subsection_dihedral*(num_moderate_subsections);
					else
						wingsect.dihedral = previous_dihedral + (subsection_dihedral*(moderate_index+1) + subsection_dihedral*(moderate_index+2))/2;
				}

				total_moderate_subsection_span += moderate_subsection_span;
				moderate_index++;
			}
			else
			{
				if(sweep_flag == MODERATE)
					wingsect.sweep = previous_sweep + subsection_sweep*(moderate_index+1);
				else if(te_sweep_flag == MODERATE)
					te_sweep = previous_te_sweep + subsection_te_sweep*(moderate_index+1);
				else
					wingsect.dihedral = previous_dihedral + subsection_dihedral*(moderate_index+1);

			}
				
			smallest_total = total_smallest_subsection_span + smallest_subsection_span;
			double largest_total = total_largest_subsection_span + largest_subsection_span;

			if( (smallest_total > largest_total) &&  fabs(smallest_total - largest_total) > 0.00001 && (largest_total < largest_subsection_total_span) )
			{
				if(sweep_flag == LARGEST)
				{
					if( (largest_index+1) >= num_largest_subsections)
						wingsect.sweep = previous_sweep + subsection_sweep*(num_largest_subsections);
					else
						wingsect.sweep = previous_sweep + (subsection_sweep*(largest_index+1) + subsection_sweep*(largest_index+2))/2;
				}
				else if(te_sweep_flag == LARGEST)
				{
					if( (largest_index+1) >= num_largest_subsections)
						te_sweep = previous_te_sweep + subsection_te_sweep*(num_largest_subsections);
					else
						te_sweep = previous_te_sweep + (subsection_te_sweep*(largest_index+1) + subsection_te_sweep*(largest_index+2))/2;
				}
				else
				{
					if( (largest_index+1) >= num_largest_subsections)
						wingsect.dihedral = previous_dihedral + subsection_dihedral*(num_largest_subsections);
					else
						wingsect.dihedral = previous_dihedral + (subsection_dihedral*(largest_index+1) + subsection_dihedral*(largest_index+2))/2;
				}

				total_largest_subsection_span += largest_subsection_span;
				largest_index++;
			}
			else
			{
				if(sweep_flag == LARGEST)
					wingsect.sweep = previous_sweep + subsection_sweep*(largest_index+1);
				else if(te_sweep_flag == LARGEST)
					te_sweep = previous_te_sweep + subsection_te_sweep*(largest_index+1);
				else
					wingsect.dihedral = previous_dihedral + subsection_dihedral*(largest_index+1);
			}

			if(sweep_flag == SMALLEST)
				wingsect.sweep = previous_sweep + subsection_sweep*(j+1);
			else if(te_sweep_flag == SMALLEST)
				te_sweep = previous_te_sweep + subsection_te_sweep*(j+1);
			else
				wingsect.dihedral = previous_dihedral + subsection_dihedral*(j+1);

			wingsect.rc = sub_sects[sub_sects.dimension()-1].tc;

			if(j == 0)
			{
				chord_lengths[i] = wingsect.rc;
			}

			double sub_sectional_sweep_offset = wingsect.span*tan(wingsect.sweep*DEG_2_RAD);
			double sub_sectional_te_sweep_offset = wingsect.span*tan(te_sweep*DEG_2_RAD);

			wingsect.tc = wingsect.rc - sub_sectional_sweep_offset - sub_sectional_te_sweep_offset;

			double new_thickness = span_percentage_sum*thickness_difference + sects[i].rootAf->get_thickness()->get();

			if(j == 0)
			{
				wingsect.rootAf = new Af(this);
				*(wingsect.rootAf) = *(sects[i].rootAf);
				wingsect.rootAf->sterilize_parms();
				sub_sect_foils.append(wingsect.rootAf);
			}
			else
				wingsect.rootAf = sub_sects[total_num_subsections - 1].tipAf;
				
			Af* newaf = new Af( this );
			wingsect.tipAf = newaf;
			sub_sect_foils.append(newaf);
			wingsect.tipAf->init_script("wing_foil");
			*(wingsect.tipAf) = *(sects[i].rootAf);
			wingsect.tipAf->sterilize_parms();
			wingsect.tipAf->get_thickness()->set(new_thickness);
			wingsect.tipAf->generate_airfoil();
			wingsect.tipAf->load_name();

			wingsect.num_interp_xsecs = sects[i].num_interp_xsecs;		

			wingsect.fillDependData();
			sub_sects.append( wingsect );
			total_smallest_subsection_span += smallest_subsection_span;

			num_subsections++;
			total_num_subsections++;
		}

		if( moderate_subsection_total_span > smallest_subsection_total_span && (num_smallest_subsections > 0) )
		{
			// = Bridge subsection connects micro subsections to macro subsections = //
			HwbSect bridgesect;
			double te_sweep;
			bridgesect.driver = MS_S_TC_RC;

			if( ( total_moderate_subsection_span + moderate_subsection_span - smallest_subsection_total_span ) > 0.0001 )
			{
				bridgesect.span = total_moderate_subsection_span + moderate_subsection_span - smallest_subsection_total_span;
				
				bridgesect.twist = total_twist + curr_twist_total + (section_twist - previous_twist) * (bridgesect.span / section_span);
				curr_twist_total += (section_twist - previous_twist) * (bridgesect.span / section_span);
				bridgesect.twistLoc = section_twistloc;
				bridgesect.sweepLoc = section_sweeploc;

				span_percentage_sum += (bridgesect.span/sects[i].span);
	
				// = Determine sweep and dihedral for bridge subsection = //
				if(sweep_flag == SMALLEST)
					bridgesect.sweep = previous_sweep + subsection_sweep*(num_sweep_subsections);
				else if(te_sweep_flag == SMALLEST)
					te_sweep = previous_te_sweep + subsection_te_sweep*(num_te_sweep_subsections);
				else
					bridgesect.dihedral = previous_dihedral + subsection_dihedral*(num_dihedral_subsections);

				if(sweep_flag == MODERATE)
					bridgesect.sweep = previous_sweep + subsection_sweep*(moderate_index+1);
				else if(te_sweep_flag == MODERATE)
					te_sweep = previous_te_sweep + subsection_te_sweep*(moderate_index+1);
				else
					bridgesect.dihedral = previous_dihedral + subsection_dihedral*(moderate_index+1);

				if( largest_subsection_total_span > smallest_subsection_total_span )
				{
					if( (smallest_subsection_total_span + bridgesect.span) > (total_largest_subsection_span + largest_subsection_span) &&
					    fabs( (smallest_subsection_total_span + bridgesect.span) - (total_largest_subsection_span + largest_subsection_span)) > 0.0001  )
					{
						if(sweep_flag == LARGEST)
						{
							if( fabs(sweep_edge_length_percentage) > 0.0001)
								bridgesect.sweep = previous_sweep + (subsection_sweep*(largest_index+1) + subsection_sweep*(largest_index+2))/2;
							else
								bridgesect.sweep = previous_sweep + subsection_sweep*(largest_index+1);
						}
						else if(te_sweep_flag == LARGEST)
						{
							if( fabs(sweep_edge_length_percentage) > 0.0001)
								te_sweep = previous_te_sweep + (subsection_te_sweep*(largest_index+1) + subsection_te_sweep*(largest_index+2))/2;
							else
								te_sweep = previous_te_sweep + subsection_te_sweep*(largest_index+1);
						}
						else
						{
							if( fabs(sweep_edge_length_percentage) > 0.0001)
								bridgesect.dihedral = previous_dihedral + (subsection_dihedral*(largest_index+1) + subsection_dihedral*(largest_index+2))/2;
							else
								bridgesect.dihedral = previous_dihedral + subsection_dihedral*(largest_index+1);
						}

						total_largest_subsection_span += largest_subsection_span;
						largest_index++;	
					}
					else
					{
						if(sweep_flag == LARGEST)
							bridgesect.sweep = previous_sweep + subsection_sweep*(largest_index+1);
						else if(te_sweep_flag == LARGEST)
							te_sweep = previous_te_sweep + subsection_te_sweep*(largest_index+1);
						else
							bridgesect.dihedral = previous_dihedral + subsection_dihedral*(largest_index+1);
					}
				}
				else
				{
					if(sweep_flag == LARGEST)
						bridgesect.sweep = previous_sweep + subsection_sweep*(largest_index+1);
					else if(te_sweep_flag == LARGEST)
						te_sweep = previous_te_sweep + subsection_te_sweep*(largest_index+1);
					else
						bridgesect.dihedral = previous_dihedral + subsection_dihedral*(largest_index+1);
				}
	
				bridgesect.rc = sub_sects[sub_sects.dimension()-1].tc;
	
				double sub_sectional_sweep_offset = bridgesect.span*tan(bridgesect.sweep*DEG_2_RAD);
				double sub_sectional_te_sweep_offset = bridgesect.span*tan(te_sweep*DEG_2_RAD);

				bridgesect.tc = bridgesect.rc - sub_sectional_sweep_offset - sub_sectional_te_sweep_offset;

				double new_thickness = span_percentage_sum*thickness_difference + sects[i].rootAf->get_thickness()->get();

				// = Set airfoil for bridge subsection = //
				if(num_smallest_subsections == 0)
				{
					bridgesect.rootAf = new Af(this);
					*(bridgesect.rootAf) = *(sects[i].rootAf);
					bridgesect.rootAf->sterilize_parms();
					sub_sect_foils.append(bridgesect.rootAf);
				}
				else
					bridgesect.rootAf = sub_sects[total_num_subsections - 1].tipAf;
				
				Af* newaf = new Af( this );
				bridgesect.tipAf = newaf;
				sub_sect_foils.append(newaf);
				bridgesect.tipAf->init_script("wing_foil");
				*(bridgesect.tipAf) = *(sects[i].rootAf);
				bridgesect.tipAf->sterilize_parms();
				bridgesect.tipAf->get_thickness()->set(new_thickness);
				bridgesect.tipAf->generate_airfoil();
				bridgesect.tipAf->load_name();

				bridgesect.num_interp_xsecs = sects[i].num_interp_xsecs;

				bridgesect.fillDependData();
				sub_sects.append( bridgesect );
	
				num_subsections++;
				total_num_subsections++;
				moderate_index++;	
				total_moderate_subsection_span += moderate_subsection_span;	
			}
			else
			{
				moderate_index++;	
				total_moderate_subsection_span += moderate_subsection_span;
			}		

			for(int k = moderate_index; k < num_moderate_subsections; k++)
			{
				HwbSect wingsect;
				wingsect.driver = MS_S_TC_RC;

				wingsect.span = moderate_subsection_span;

				wingsect.twist = total_twist + curr_twist_total + (section_twist - previous_twist) * (wingsect.span / section_span);
				curr_twist_total += (section_twist - previous_twist) * (wingsect.span / section_span);
				wingsect.twistLoc = section_twistloc;
				wingsect.sweepLoc = section_sweeploc;

				span_percentage_sum += (wingsect.span/sects[i].span);

				// = Determine sweep and dihedral for bridge subsection = //
				if(sweep_flag == SMALLEST)
					wingsect.sweep = previous_sweep + subsection_sweep*(num_sweep_subsections);
				else if(te_sweep_flag == SMALLEST)
					te_sweep = previous_te_sweep + subsection_te_sweep*(num_te_sweep_subsections);
				else
					wingsect.dihedral = previous_dihedral + subsection_dihedral*(num_dihedral_subsections);

				if(sweep_flag == MODERATE)
					wingsect.sweep = previous_sweep + subsection_sweep*(k+1);
				else if(te_sweep_flag == MODERATE)
					te_sweep = previous_te_sweep + subsection_te_sweep*(k+1);
				else
					wingsect.dihedral = previous_dihedral + subsection_dihedral*(k+1);
	
				if( largest_subsection_total_span > smallest_subsection_total_span )
				{
					double moderate_total = total_moderate_subsection_span + moderate_subsection_span;
					double largest_total = total_largest_subsection_span + largest_subsection_span;

					if( (moderate_total > largest_total) && ( fabs( moderate_total - largest_total) > 0.00001 ) && 
(largest_total < largest_subsection_total_span) )
					{
						if(sweep_flag == LARGEST)
							wingsect.sweep = previous_sweep + (subsection_sweep*(largest_index+1) + subsection_sweep * (largest_index+2))/2;
						else if(te_sweep_flag == LARGEST)
							te_sweep = previous_te_sweep + (subsection_te_sweep*(largest_index+1) + subsection_te_sweep * (largest_index+2))/2;
						else
							wingsect.dihedral = previous_dihedral + (subsection_dihedral*(largest_index+1) + subsection_dihedral*(largest_index+2))/2;

						total_largest_subsection_span += largest_subsection_span;
						largest_index++;
					}
					else
					{
						if(sweep_flag == LARGEST)
							wingsect.sweep = previous_sweep + subsection_sweep*(largest_index+1);
						else if(te_sweep_flag == LARGEST)
							te_sweep = previous_te_sweep + subsection_te_sweep*(largest_index+1);
						else
							wingsect.dihedral = previous_dihedral + subsection_dihedral*(largest_index+1);
					}
				}
				else
				{
					if(sweep_flag == LARGEST)
						wingsect.sweep = previous_sweep + subsection_sweep*(largest_index+1);
					else if(te_sweep_flag == LARGEST)
						te_sweep = previous_te_sweep + subsection_te_sweep*(largest_index+1);
					else
						wingsect.dihedral = previous_dihedral + subsection_dihedral*(largest_index+1);
				}

				wingsect.rc = sub_sects[sub_sects.dimension()-1].tc;

				double sub_sectional_sweep_offset = wingsect.span*tan(wingsect.sweep*DEG_2_RAD);
				double sub_sectional_te_sweep_offset = wingsect.span*tan(te_sweep*DEG_2_RAD);

				wingsect.tc = wingsect.rc - sub_sectional_sweep_offset - sub_sectional_te_sweep_offset;

				double new_thickness = span_percentage_sum*thickness_difference + sects[i].rootAf->get_thickness()->get();

				// = Set airfoil for bridge subsection = //
				wingsect.rootAf = sub_sects[total_num_subsections - 1].tipAf;

				Af* newaf = new Af( this );
				wingsect.tipAf = newaf;
				sub_sect_foils.append(newaf);
				wingsect.tipAf->init_script("wing_foil");
				*(wingsect.tipAf) = *(sects[i].rootAf);
				wingsect.tipAf->sterilize_parms();
				wingsect.tipAf->get_thickness()->set(new_thickness);
				wingsect.tipAf->generate_airfoil();
				wingsect.tipAf->load_name();

				wingsect.num_interp_xsecs = sects[i].num_interp_xsecs;

				wingsect.fillDependData();
				sub_sects.append( wingsect );
	
				num_subsections++;
				total_num_subsections++;
				total_moderate_subsection_span += moderate_subsection_span;
			}

			if( largest_subsection_total_span > moderate_subsection_total_span )
			{
				HwbSect bridgesect2;
				bridgesect2.driver = MS_S_TC_RC;

				if( ( total_largest_subsection_span + largest_subsection_span - moderate_subsection_total_span ) > 0.0001 )
				{
					bridgesect2.span = total_largest_subsection_span + largest_subsection_span - moderate_subsection_total_span;
			
					bridgesect2.twist = total_twist + curr_twist_total + (section_twist - previous_twist) * (bridgesect2.span / section_span);
					curr_twist_total += (section_twist - previous_twist) * (bridgesect2.span / section_span);
					bridgesect2.twistLoc = section_twistloc;
					bridgesect2.sweepLoc = section_sweeploc;

					span_percentage_sum += (bridgesect2.span/sects[i].span);

					// = Determine sweep and dihedral for bridge subsection = //
					if(sweep_flag == SMALLEST)
						bridgesect2.sweep = previous_sweep + subsection_sweep*(num_sweep_subsections);
					else if(te_sweep_flag == SMALLEST)
						te_sweep = previous_te_sweep + subsection_te_sweep*(num_te_sweep_subsections);
					else
						bridgesect2.dihedral = previous_dihedral + subsection_dihedral*(num_dihedral_subsections);
	
					if(sweep_flag == MODERATE)
						bridgesect2.sweep = previous_sweep + subsection_sweep*(num_sweep_subsections);
					else if(te_sweep_flag == MODERATE)
						te_sweep = previous_te_sweep + subsection_te_sweep*(num_te_sweep_subsections);
					else
						bridgesect2.dihedral = previous_dihedral + subsection_dihedral*(num_dihedral_subsections);

					if(sweep_flag == LARGEST)
						bridgesect2.sweep = previous_sweep + subsection_sweep*(largest_index+1);
					else if(te_sweep_flag == LARGEST)
						te_sweep = previous_te_sweep + subsection_te_sweep*(largest_index+1);
					else
					bridgesect2.dihedral = previous_dihedral + subsection_dihedral*(largest_index+1);

					bridgesect2.rc = sub_sects[sub_sects.dimension()-1].tc;

					double sub_sectional_sweep_offset = bridgesect2.span*tan(bridgesect2.sweep*DEG_2_RAD);
					double sub_sectional_te_sweep_offset = bridgesect2.span*tan(te_sweep*DEG_2_RAD);

					bridgesect2.tc = bridgesect2.rc - sub_sectional_sweep_offset - sub_sectional_te_sweep_offset;
	
					double new_thickness = span_percentage_sum*thickness_difference + sects[i].rootAf->get_thickness()->get();

					// = Set airfoil for bridge subsection = //
					bridgesect2.rootAf = sub_sects[total_num_subsections - 1].tipAf;
					Af* newaf = new Af( this );
					bridgesect2.tipAf = newaf;
					sub_sect_foils.append(newaf);
					bridgesect2.tipAf->init_script("wing_foil");
					*(bridgesect2.tipAf) = *(sects[i].rootAf);
					bridgesect2.tipAf->sterilize_parms();
					bridgesect2.tipAf->get_thickness()->set(new_thickness);
					bridgesect2.tipAf->generate_airfoil();
					bridgesect2.tipAf->load_name();	

					bridgesect2.num_interp_xsecs = sects[i].num_interp_xsecs;	

					bridgesect2.fillDependData();
					sub_sects.append( bridgesect2 );
	
					num_subsections++;
					total_num_subsections++;
					largest_index++;
					total_largest_subsection_span += largest_subsection_span;
				}
				else
				{
					largest_index++;	
					total_largest_subsection_span += largest_subsection_span;
				}
				
				for(int k = largest_index; k < num_largest_subsections; k++)
				{
					HwbSect wingsect;
					wingsect.driver = MS_S_TC_RC;

					wingsect.span = largest_subsection_span;
					wingsect.twist = total_twist + curr_twist_total + (section_twist - previous_twist) * (wingsect.span / section_span);
					curr_twist_total += (section_twist - previous_twist) * (wingsect.span / section_span);
					wingsect.twistLoc = section_twistloc;
					wingsect.sweepLoc = section_sweeploc;

					span_percentage_sum += (wingsect.span/sects[i].span);

					// = Determine sweep and dihedral for bridge subsection = //
					if(sweep_flag == SMALLEST)
						wingsect.sweep = previous_sweep + subsection_sweep*(num_sweep_subsections);
					else if(te_sweep_flag == SMALLEST)
						te_sweep = previous_te_sweep + subsection_te_sweep*(num_te_sweep_subsections);
					else
						wingsect.dihedral = previous_dihedral + subsection_dihedral*(num_dihedral_subsections);

					if(sweep_flag == MODERATE)
						wingsect.sweep = previous_sweep + subsection_sweep*(num_sweep_subsections);
					else if(te_sweep_flag == MODERATE)
						te_sweep = previous_te_sweep + subsection_te_sweep*(num_te_sweep_subsections);
					else
						wingsect.dihedral = previous_dihedral + subsection_dihedral*(num_dihedral_subsections);

					if(sweep_flag == LARGEST)
						wingsect.sweep = previous_sweep + subsection_sweep*(k+1);
					else if(te_sweep_flag == LARGEST)
						te_sweep = previous_te_sweep + subsection_te_sweep*(k+1);
					else
						wingsect.dihedral = previous_dihedral + subsection_dihedral*(k+1);

					wingsect.rc = sub_sects[sub_sects.dimension()-1].tc;

					double sub_sectional_sweep_offset = wingsect.span*tan(wingsect.sweep*DEG_2_RAD);
					double sub_sectional_te_sweep_offset = wingsect.span*tan(te_sweep*DEG_2_RAD);

					wingsect.tc = wingsect.rc - sub_sectional_sweep_offset - sub_sectional_te_sweep_offset;

					double new_thickness = span_percentage_sum*thickness_difference + sects[i].rootAf->get_thickness()->get();

					// = Set airfoil for bridge subsection = //
					if(num_smallest_subsections == 0 && num_moderate_subsections == 0)
					{
						wingsect.rootAf = new Af(this);
						*(wingsect.rootAf) = *(sects[i].rootAf);
						wingsect.rootAf->sterilize_parms();
						sub_sect_foils.append(wingsect.rootAf);
					}
					else
						wingsect.rootAf = sub_sects[total_num_subsections - 1].tipAf;

					Af* newaf = new Af( this );
					wingsect.tipAf = newaf;
					sub_sect_foils.append(newaf);
					wingsect.tipAf->init_script("wing_foil");
					*(wingsect.tipAf) = *(sects[i].rootAf);
					wingsect.tipAf->sterilize_parms();
					wingsect.tipAf->get_thickness()->set(new_thickness);
					wingsect.tipAf->generate_airfoil();
					wingsect.tipAf->load_name();	

					wingsect.num_interp_xsecs = sects[i].num_interp_xsecs;

					wingsect.fillDependData();
					sub_sects.append( wingsect );
	
					num_subsections++;
					total_num_subsections++;
				}
			}			
		}
		else if( largest_subsection_total_span > smallest_subsection_total_span && (num_smallest_subsections > 0) )
		{
			// = Bridge subsection connects micro subsections to macro subsections = //
			double te_sweep;
			HwbSect bridgesect;
			bridgesect.driver = MS_S_TC_RC;

			if( ( total_largest_subsection_span + largest_subsection_span - smallest_subsection_total_span ) > 0.0001 )
			{
				bridgesect.span = total_largest_subsection_span + largest_subsection_span - smallest_subsection_total_span;

				bridgesect.twist = total_twist + curr_twist_total + (section_twist - previous_twist) * (bridgesect.span / section_span);
				curr_twist_total += (section_twist - previous_twist) * (bridgesect.span / section_span);
				bridgesect.twistLoc = section_twistloc;
				bridgesect.sweepLoc = section_sweeploc;

				span_percentage_sum += (bridgesect.span/sects[i].span);
	
				if(sweep_flag == SMALLEST)
					bridgesect.sweep = previous_sweep + subsection_sweep*num_sweep_subsections;
				else if(te_sweep_flag == SMALLEST)
					te_sweep = previous_te_sweep + subsection_te_sweep*num_te_sweep_subsections;
				else
				bridgesect.dihedral = previous_dihedral + subsection_dihedral*num_dihedral_subsections;

				if(sweep_flag == MODERATE)
					bridgesect.sweep = previous_sweep + subsection_sweep*num_sweep_subsections;
				else if(te_sweep_flag == MODERATE)
					te_sweep = previous_te_sweep + subsection_te_sweep*num_te_sweep_subsections;
				else
					bridgesect.dihedral = previous_dihedral + subsection_dihedral*num_dihedral_subsections;

				if(sweep_flag == LARGEST)
					bridgesect.sweep = previous_sweep + subsection_sweep*(largest_index+1);
				else if(te_sweep_flag == LARGEST)
					te_sweep = previous_te_sweep + subsection_te_sweep*(largest_index+1);
				else
					bridgesect.dihedral = previous_dihedral + subsection_dihedral*(largest_index+1);

				bridgesect.rc = sub_sects[sub_sects.dimension()-1].tc;
	
				double sub_sectional_sweep_offset = bridgesect.span*tan(bridgesect.sweep*DEG_2_RAD);
				double sub_sectional_te_sweep_offset = bridgesect.span*tan(te_sweep*DEG_2_RAD);

				bridgesect.tc = bridgesect.rc - sub_sectional_sweep_offset - sub_sectional_te_sweep_offset;

				double new_thickness = span_percentage_sum*thickness_difference + sects[i].rootAf->get_thickness()->get();

				// = Set airfoil for bridge subsection = //
				bridgesect.rootAf = sub_sects[total_num_subsections - 1].tipAf;
				Af* newaf = new Af( this ); 
				bridgesect.tipAf = newaf;
				sub_sect_foils.append(newaf);
				bridgesect.tipAf->init_script("wing_foil");
				*(bridgesect.tipAf) = *(sects[i].rootAf);
				bridgesect.tipAf->sterilize_parms();
				bridgesect.tipAf->get_thickness()->set(new_thickness);
				bridgesect.tipAf->generate_airfoil();
				bridgesect.tipAf->load_name();

				bridgesect.num_interp_xsecs = sects[i].num_interp_xsecs;

				bridgesect.fillDependData();
				sub_sects.append( bridgesect );
	
				num_subsections++;
				total_num_subsections++;
				largest_index++;
				total_largest_subsection_span += largest_subsection_span;
			}
			else
			{
				largest_index++;	
				total_largest_subsection_span += largest_subsection_span;
			}
			

			for(int k = largest_index; k < num_largest_subsections; k++)
			{
				HwbSect wingsect;
				wingsect.driver = MS_S_TC_RC;

				wingsect.span = largest_subsection_span;
				wingsect.twist = total_twist + section_twist - (section_twist - previous_twist) * (wingsect.span / section_span) - curr_twist_total;
				curr_twist_total += (section_twist - previous_twist) * (wingsect.span / section_span);
				wingsect.twistLoc = section_twistloc;
				wingsect.sweepLoc = section_sweeploc;

				span_percentage_sum += (wingsect.span/sects[i].span);

				// = Determine sweep and dihedral for bridge subsection = //
				if(sweep_flag == SMALLEST)
					wingsect.sweep = previous_sweep + subsection_sweep*(num_sweep_subsections);
				else if(te_sweep_flag == SMALLEST)
					te_sweep = previous_te_sweep + subsection_te_sweep*(num_te_sweep_subsections);
				else
					wingsect.dihedral = previous_dihedral + subsection_dihedral*(num_dihedral_subsections);

				if(sweep_flag == MODERATE)
					wingsect.sweep = previous_sweep + subsection_sweep*(num_sweep_subsections);
				else if(te_sweep_flag == MODERATE)
					te_sweep = previous_te_sweep + subsection_te_sweep*(num_te_sweep_subsections);
				else
					wingsect.dihedral = previous_dihedral + subsection_dihedral*(num_dihedral_subsections);
	
				if(sweep_flag == LARGEST)
					wingsect.sweep = previous_sweep + subsection_sweep*(k+1);
				else if(te_sweep_flag == LARGEST)
					te_sweep = previous_te_sweep + subsection_te_sweep*(k+1);
				else
					wingsect.dihedral = previous_dihedral + subsection_dihedral*(k+1);

				wingsect.rc = sub_sects[sub_sects.dimension()-1].tc;
	
				double sub_sectional_sweep_offset = wingsect.span*tan(wingsect.sweep*DEG_2_RAD);
				double sub_sectional_te_sweep_offset = wingsect.span*tan(te_sweep*DEG_2_RAD);

				wingsect.tc = wingsect.rc - sub_sectional_sweep_offset - sub_sectional_te_sweep_offset;

				// = Set airfoil for bridge subsection = //

				double new_thickness = span_percentage_sum*thickness_difference + sects[i].rootAf->get_thickness()->get();

				wingsect.rootAf = sub_sects[total_num_subsections - 1].tipAf;
				Af* newaf = new Af( this );
				wingsect.tipAf = newaf;
				sub_sect_foils.append(newaf);
				wingsect.tipAf->init_script("wing_foil");
				*(wingsect.tipAf) = *(sects[i].rootAf);
				wingsect.tipAf->sterilize_parms();
				wingsect.tipAf->get_thickness()->set(new_thickness);
				wingsect.tipAf->generate_airfoil();
				wingsect.tipAf->load_name();

				wingsect.num_interp_xsecs = sects[i].num_interp_xsecs;

				wingsect.fillDependData();
				sub_sects.append( wingsect );

				num_subsections++;
				total_num_subsections++;
			}
		}

		previous_sweep = section_sweep;
		previous_te_sweep = section_te_sweep;
		previous_dihedral = section_dihedral;
	}

	HwbSect mainsect;
	mainsect.driver = MS_S_TC_RC;
	mainsect.span = sects[sects.dimension()-1].span - previous_root_span;

	double span_at_root = 0.0;

	for(int i=0; i < sects.dimension()-1; i++)
	{
		span_at_root += sects[i].span;
	}

	span_percentage_sum += (mainsect.span/sects[sects.dimension()-1].span);

	if( fabs(mainsect.span) > 0.00001 )
	{
		mainsect.twist = total_twist + curr_twist_total + (section_twist - previous_twist) * (mainsect.span / sects[sects.dimension()-1].span);
		curr_twist_total += (section_twist - previous_twist) * (mainsect.span / sects[sects.dimension()-1].span);
		mainsect.twistLoc = sects[sects.dimension()-1].twistLoc;
		mainsect.sweepLoc = 0.0;
		mainsect.sweep = atan(sects[sects.dimension()-1].tan_sweep_at(0.0,getSymCode()))*RAD_2_DEG;
		mainsect.dihedral = sects[sects.dimension()-1].dihedral;

		if(sub_sects.dimension() == 0)
			mainsect.rc = section_rc;
		else
			mainsect.rc = sub_sects[sub_sects.dimension() - 1].tc;

		mainsect.tc = sects[sects.dimension()-1].tc;

		double new_thickness = span_percentage_sum*thickness_difference + sects[sects.dimension()-1].rootAf->get_thickness()->get();

		if( fabs(previous_root_span) < 0.0001 )
		{
			mainsect.rootAf = new Af(this);
			*(mainsect.rootAf) = *(sects[sects.dimension()-1].rootAf);
		}
		else
			mainsect.rootAf = sub_sects[sub_sects.dimension()-1].rootAf;

		mainsect.tipAf = new Af(this);
		*(mainsect.tipAf) = *(sects[sects.dimension()-1].tipAf);	

		num_subsections++;
		mainsect.fillDependData();
		section_sizes.append(num_subsections);
		sub_sects.append( mainsect );

		if(fabs(previous_root_span) < 0.0001)
					chord_lengths[chord_lengths.dimension()-2] = mainsect.rc;

		chord_lengths[chord_lengths.dimension()-1] = mainsect.tc;
	}
/*
	int starting_index = 0;
	int ending_index = 0;
	int main_index = 0;


    for(int j = 0; j < sects.dimension(); j++)
	{
		double percentage_span = 0.0;
		Af* root_af = sects[j].rootAf;
		Af* tip_af = sects[j].tipAf;

		if(j != 0)
			starting_index += section_sizes[j-1];

		ending_index += section_sizes[j];

		for(int k = starting_index; k < ending_index; k++)
		{
			if(k == 0)
			{
				sub_sects[k].rootAf = new Af(this);
				sub_sects[k].rootAf->set_type(6);
				sub_sects[k].rootAf->seed_interpolated_airfoil(root_af, tip_af, percentage_span);
			}
			else
			{
				sub_sects[k].rootAf = sub_sects[k-1].tipAf;
			}

			percentage_span += (sub_sects[k].span/sects[j].span);

			sub_sects[k].tipAf = new Af(this);
			sub_sects[k].tipAf->set_type(6);
			sub_sects[k].tipAf->seed_interpolated_airfoil(root_af, tip_af, percentage_span);
		}
	}
*/
	for(int j = 0; j < sects.dimension(); j++)
	{	
		int starting_index = dihedral_rot_starting_indices[j];
		int next_starting_index = 0;

		if(j != (sects.dimension() - 1) )
			next_starting_index = dihedral_rot_starting_indices[j+1];
		else
			next_starting_index = sub_sects.dimension();

		for(int i = starting_index; i < next_starting_index; i++)
 		{
			if(i != 0)
				sub_sects[i].dihedRotFlag = sects[j].dihedRotFlag;
  		}
	}
} 

//==== Append Wake Edges ====//
void HwbBaseGeom::AppendWakeEdges( vector< vector< vec3d > > & edges )
{
	if ( !m_WakeActiveFlag )
		return;

	int nxs = mwing_surf.get_num_xsecs();

	vector< vec3d > teVec;
	vector< vec3d > refTeVec;
	for ( int i = 1 ; i < nxs-1 ; i++ )
	{
		vec3d p = mwing_surf.get_pnt( i, 0 );
		teVec.push_back( p.transform( model_mat ) );

		if ( sym_code != NO_SYM )
			refTeVec.push_back( (p * sym_vec).transform(reflect_mat) );
	}

	edges.push_back( teVec );
	if (sym_code != NO_SYM)
		edges.push_back( refTeVec );
}
