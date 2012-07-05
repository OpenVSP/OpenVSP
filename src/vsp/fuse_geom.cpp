//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//

//******************************************************************************
//    
//    Fuselage Geometry Class
//  
// 
//   J.R. Gloudemans - 10/2/93
//   Sterling Software
//
//   
//
//******************************************************************************
#include <FL/Fl.H>

#include "fuse_geom.h"
#include "defines.h"
#include "geom.h"
#include "aircraft.h"
#include "misc_func.h"
#include "materialMgr.h"
#include "bezier_curve.h"
#include "bezier_surf.h"
#include "screenMgr.h"
#include "util.h"
#include "parmLinkMgr.h"

//==== Constructor =====//
Fuse_geom::Fuse_geom(Aircraft* aptr) : Geom(aptr)
{

  int i;

  redFlag = 0;
  activeControlPntID = 0;
    
  saved_xsec.set_fuse_ptr(this);

  numXsecs.deactivate();

  type = FUSE_GEOM_TYPE;
	type_str = Stringc("fuse");

  char name[255];
  sprintf( name, "Fuse_%d", geomCnt ); 
  geomCnt++;
  setName( Stringc(name) );

  spine.init(3);

  length.initialize(this, UPD_FUSE_BODY, "Length", 30.0 );
  length.set_lower_upper(0.0001, 1000000.0);
  length.set_script("fuse_shape length", 0);

  camber.initialize(this, UPD_FUSE_BODY, "Camber", 0.0 );
  camber.set_lower_upper(-1.0, 1.0);
  camber.set_script("fuse_shape camber", 0);

  camber_loc.initialize(this, UPD_FUSE_BODY, "Camber_Loc", 0.5 );
  camber_loc.set_lower_upper(0.0, 1.0);
  camber_loc.set_script("fuse_shape camberloc", 0);

  aft_offset.initialize(this, UPD_FUSE_BODY, "Aft_Offset", 0.0 );
  aft_offset.set_lower_upper(-1.0, 1.0);
  aft_offset.set_script("fuse_shape aftoffset", 0);

  nose_angle.initialize(this, UPD_FUSE_BODY, "Nose_Angle", 0.0 );
  nose_angle.set_lower_upper(0.0, 90.0);
  nose_angle.set_script("fuse_shape noseangle", 0);

  nose_str.initialize(this, UPD_FUSE_BODY, "Nose_Str", 0.3 );
  nose_str.set_lower_upper(0.0, 1.0);
  nose_str.set_script("fuse_shape nosestr", 0);

  nose_rho.initialize(this, UPD_FUSE_BODY, "Nose_Rho", 0.5 );
  nose_rho.set_lower_upper(0.0, 2.0);
  nose_rho.set_script("fuse_shape noserho", 0);

  aft_rho.initialize(this, UPD_FUSE_BODY, "Aft_Rho", 0.5 );
  aft_rho.set_lower_upper(0.0, 2.0);
  aft_rho.set_script("fuse_shape aftrho", 0);

  nose_super_flag = 1;
  aft_super_flag  = 1;

  //==== Initialize IML Xsecs ====//
  imlFlag = 0;
     
  //==== Color ====//
  imlColor = vec3d( 0.0, 1.0, 0.0 );
  imlMaterialID = 0;

  //==== Load Initial Default Cross Sections ====//
  int num_init_xsecs = 5;
  for ( i = 0 ; i < num_init_xsecs ; i++)
  {
	fuse_xsec* xs = new fuse_xsec( this );
	xsecVec.push_back( xs );
	xs->set_loc_on_spine( (float)i/(float)(num_init_xsecs-1) );
	if ( i == 0 || i == num_init_xsecs-1)
	{
		xs->set_type(XSEC_POINT);
		xs->gen_parms();
	}
	
	xs->set_iml_flag( imlFlag );
	xs->generate();
  }

  curr_xsec_num = 1;

  int defNumNoseAft = 6;
  xsecVec[0]->setNumSectInterp2(defNumNoseAft);
  xsecVec[1]->setNumSectInterp1(defNumNoseAft);
  xsecVec[xsecVec.size()-2]->setNumSectInterp2( defNumNoseAft );
  xsecVec[xsecVec.size()-1]->setNumSectInterp1( defNumNoseAft );

  set_num_pnts( numPnts.iget() );

  center.set_xyz( 0, 0, 0 );
  slice_parms.init(0,0);

  //==== Initialize IML Xsecs ====//
  generate();

  surfVec.push_back( &oml_surf );

}

//==== Destructor =====//
Fuse_geom::~Fuse_geom()
{
	DeleteAllXSecs();


}
void Fuse_geom::LoadLinkableParms( vector< Parm* > & parmVec )
{
	Geom::LoadLinkableParms( parmVec );
	Geom::AddLinkableParm( &length, parmVec, this, "Design" );
	Geom::AddLinkableParm( &camber, parmVec, this, "Design" );
	Geom::AddLinkableParm( &camber_loc, parmVec, this, "Design" );
	Geom::AddLinkableParm( &aft_offset, parmVec, this, "Design" );
	Geom::AddLinkableParm( &nose_angle, parmVec, this, "Design" );
	Geom::AddLinkableParm( &nose_str, parmVec, this, "Design" );
	Geom::AddLinkableParm( &nose_rho, parmVec, this, "Design" );
	Geom::AddLinkableParm( &aft_rho, parmVec, this, "Design" );

	char gname[256];
	for ( int i = 0 ; i < (int)xsecVec.size() ; i++ )
	{
		sprintf( gname, "XSec_%2d", i );
		Geom::AddLinkableParm( xsecVec[i]->get_height(), parmVec, this, gname );
		Geom::AddLinkableParm( xsecVec[i]->get_width(), parmVec, this, gname );
		Geom::AddLinkableParm( xsecVec[i]->get_max_width_loc(), parmVec, this, gname );
		Geom::AddLinkableParm( xsecVec[i]->get_corner_rad(), parmVec, this, gname );
		Geom::AddLinkableParm( xsecVec[i]->get_top_tan_angle(), parmVec, this, gname );
		Geom::AddLinkableParm( xsecVec[i]->get_bot_tan_angle(), parmVec, this, gname );
		Geom::AddLinkableParm( xsecVec[i]->get_top_str(), parmVec, this, gname );
		Geom::AddLinkableParm( xsecVec[i]->get_upp_str(), parmVec, this, gname );
		Geom::AddLinkableParm( xsecVec[i]->get_bot_str(), parmVec, this, gname );
		Geom::AddLinkableParm( xsecVec[i]->get_low_str(), parmVec, this, gname );
		Geom::AddLinkableParm( xsecVec[i]->get_z_offset(), parmVec, this, gname );
		Geom::AddLinkableParm( xsecVec[i]->get_location(), parmVec, this, gname );
	}
}
void Fuse_geom::RemoveXSecParmReferences( fuse_xsec* xsec )
{
	parmMgrPtr->RemoveParmReferences( xsec->get_height() );
	parmMgrPtr->RemoveParmReferences( xsec->get_width() );
	parmMgrPtr->RemoveParmReferences( xsec->get_max_width_loc() );
	parmMgrPtr->RemoveParmReferences( xsec->get_corner_rad() );
	parmMgrPtr->RemoveParmReferences( xsec->get_top_tan_angle() );
	parmMgrPtr->RemoveParmReferences( xsec->get_bot_tan_angle() );
	parmMgrPtr->RemoveParmReferences( xsec->get_top_str() );
	parmMgrPtr->RemoveParmReferences( xsec->get_upp_str() );
	parmMgrPtr->RemoveParmReferences( xsec->get_bot_str() );
	parmMgrPtr->RemoveParmReferences( xsec->get_low_str() );
	parmMgrPtr->RemoveParmReferences( xsec->get_z_offset() );
	parmMgrPtr->RemoveParmReferences( xsec->get_location() );
}

void Fuse_geom::AddDefaultSources(double base_len)
{
	//==== Add Some Default Sources for CFD Mesh ====//
	PointSource* ps = new PointSource();
	ps->SetName("Nose");
	ps->SetLen( 0.2*base_len );
	ps->SetRad( 0.1*length() );
	vec2d loc = vec2d(0,0);
	ps->SetUWLoc( loc );
	sourceVec.push_back( ps );

	ps = new PointSource();
	ps->SetName("Aft");
	ps->SetLen( 0.2*base_len );
	ps->SetRad( 0.1*length() );
	loc = vec2d(1,0);
	ps->SetUWLoc( loc );
	sourceVec.push_back( ps );
}

void Fuse_geom::DeleteAllXSecs()
{
    //==== Load & Read Fuse Xsec Parms ====//
	for ( int i = 0 ; i < (int)xsecVec.size() ; i++ )
	{
		RemoveXSecParmReferences( xsecVec[i] );
		delete xsecVec[i];
	}
	xsecVec.clear();
}


void Fuse_geom::copy( Geom* fromGeom )
{
	int i;
	Geom::copy( fromGeom );						// Copy Base Stuff

	if ( fromGeom->getType() != FUSE_GEOM_TYPE )
		return;

	Fuse_geom* g = (Fuse_geom*)fromGeom;

	length.set( g->get_length()->get() );
	camber.set( g->get_camber()->get() );
	camber_loc.set( g->get_camber_loc()->get() );
	aft_offset.set( g->get_aft_offset()->get() );
	nose_angle.set( g->get_nose_angle()->get() );
	nose_str.set( g->get_nose_str()->get() );
	nose_rho.set( g->get_nose_rho()->get() );
	aft_rho.set( g->get_aft_rho()->get() );

	setNoseSuperFlag( g->getNoseSuperFlag() );
	setAftSuperFlag( g->getAftSuperFlag() );

	imlFlag = g->get_iml_flag();

	DeleteAllXSecs();
	//for ( i = 0 ; i < (int)xsecVec.size() ; i++ )
	//{
	//	RemoveXSecParmReferences( xsecVec[i] );
	//	delete xsecVec[i];
	//}
	//xsecVec.clear();

	int numXsec = g->xsecVec.size();

	for ( i = 0 ; i < numXsec ; i++ )
	{
		fuse_xsec* new_xsec = new fuse_xsec(this);
		new_xsec->gen_parms();
		*new_xsec = *(g->xsecVec[i]);
		new_xsec->set_iml_flag( imlFlag );
		new_xsec->generate();
		new_xsec->set_loc_on_spine( g->xsecVec[i]->get_loc_on_spine() );
		new_xsec->set_fuse_ptr(this);

		xsecVec.push_back( new_xsec );
	}

	curr_xsec_num = g->get_curr_xsec_num();	

	//===== Set Flags =====
	set_iml_flag( imlFlag );
	int space_type = xsecVec[0]->get_pnt_space_type();
	set_pnt_space_type( space_type );

	generate();
	compose_model_matrix();

	set_num_pnts( numPnts.iget() );

	parmMgrPtr->RebuildAll();

}

void Fuse_geom::computeCenter()
{
	//==== Set Rotation Center ====//
	center.set_x( origin()*length()*scaleFactor() ); 
}


void Fuse_geom::parm_changed(Parm* p)
{
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

		case UPD_FUSE_XSEC:
		{
			if ( p == get_curr_xsec()->get_top_thick() || 
				 p == get_curr_xsec()->get_bot_thick() ||
				 p == get_curr_xsec()->get_side_thick() )
			{
				get_curr_xsec()->thickChange();
			}

//			get_curr_xsec()->regenerate();
			generate();
			updateAttach(0);
		}
		break;

		case UPD_EDIT_CRV:
		{
//			generate();
			updateAttach(0);
		}

		case UPD_FUSE_BODY:
		{
			generate();
			updateAttach(0);
		}
		break;


		case UPD_NUM_PNT_XSEC:
		{
			set_num_pnts( numPnts.iget() );
			generate();
		}
		break;

    }

	if ( p == &scaleFactor )	
	{
		scale();
		generate();
	}

	compose_model_matrix();

}

void Fuse_geom::acceptScaleFactor()
{
	lastScaleFactor = 1.0;
	scaleFactor.set(1.0);
}

void Fuse_geom::resetScaleFactor()
{
	scaleFactor.set( 1.0 );
    scale();
	lastScaleFactor = 1.0;
		
	//==== Tell Aircraft You Have Changed ====//
	airPtr->geomMod( this );


}

void Fuse_geom::scale()
{
	double current_factor = scaleFactor()*(1.0/lastScaleFactor);

	length.set( length()*current_factor );

	for ( int i = 0 ; i < (int)xsecVec.size() ; i++ )
	{
		Parm* h = xsecVec[i]->get_height();
		Parm* w = xsecVec[i]->get_width();
		Parm* z = xsecVec[i]->get_z_offset();

		h->set( h->get()*current_factor );
		w->set( w->get()*current_factor );
		z->set( z->get()*current_factor );

		xsecVec[i]->generate();
	}


    //update display
    generate();

 
	lastScaleFactor = scaleFactor();
}

vec3d Fuse_geom::getAttachUVPos(double u, double v)
{
	vec3d pos;

	vec3d uvpnt = oml_surf.comp_uv_pnt(u,v);
	pos = uvpnt.transform( model_mat );

	return pos;
}

void Fuse_geom::setImlColor( double r, double g, double b)
{
	imlColor.set_xyz( r, g, b );
	airPtr->geomMod( this );
}

//==== Update Bounding Box =====//
void Fuse_geom::update_bbox()
{
  vec3d pnt;
  bbox new_box;

  int num_pnts = oml_surf.get_num_pnts();
  int num_xsec = oml_surf.get_num_xsecs();
  int half = num_xsec/2;

  //==== Check Some Inlet And Nozzle Xsecs ====//
  for ( int j = 0 ; j < num_pnts ; j++ )
    {
      pnt = oml_surf.get_pnt(0,j);            new_box.update(pnt);
      pnt = oml_surf.get_pnt(half,j);         new_box.update(pnt);
      pnt = oml_surf.get_pnt(num_xsec-1,j);   new_box.update(pnt);
    }

  bnd_box = new_box;
  update_xformed_bbox();			// Load Xform BBox
}

//==== Generate Fuse Component ====//
void Fuse_geom::generate()
{
 
  //==== Update Location Limits on Spine ====//
  for ( int i = 0 ; i < (int)xsecVec.size() ; i++ )
  {
	  if ( i == 0 )
		xsecVec[i]->set_location_limits( 0.0, xsecVec[i+1]->get_loc_on_spine()-0.0001 );
	  else if ( i == xsecVec.size()-1 )
		xsecVec[i]->set_location_limits( xsecVec[i-1]->get_loc_on_spine()+0.0001, 1.0 );
	  else
		xsecVec[i]->set_location_limits( xsecVec[i-1]->get_loc_on_spine()+0.0001,
										 xsecVec[i+1]->get_loc_on_spine()-0.0001);
  }
	
  comp_spine();
  gen_surf();




  airPtr->geomMod( this );



//debug_print();

}

//==== Write Fuse File ====//
void Fuse_geom::write(FILE* file_id)
{
}

//==== Write Fuse File ====//
void Fuse_geom::write(xmlNodePtr root)
{
  int i;
  xmlAddStringNode( root, "Type", "Fuselage");

  //==== Write General Parms ====//
  xmlNodePtr gen_node = xmlNewChild( root, NULL, (const xmlChar *)"General_Parms", NULL );
  write_general_parms( gen_node );

  //==== Write Fuse Parms ====//
  xmlNodePtr fuse_node = xmlNewChild( root, NULL, (const xmlChar *)"Fuse_Parms", NULL );

  xmlAddDoubleNode( fuse_node, "Fuse_Length", length() );
  xmlAddDoubleNode( fuse_node, "Camber", camber() );
  xmlAddDoubleNode( fuse_node, "Camber_Location", camber_loc() );
  xmlAddDoubleNode( fuse_node, "Aft_Offset", aft_offset() );
  xmlAddDoubleNode( fuse_node, "Nose_Angle", nose_angle() );
  xmlAddDoubleNode( fuse_node, "Nose_Strength", nose_str() );
  xmlAddDoubleNode( fuse_node, "Nose_Rho", nose_rho() );
  xmlAddDoubleNode( fuse_node, "Aft_Rho", aft_rho() );

  xmlAddIntNode( fuse_node, "IML_Flag", imlFlag);
  xmlAddIntNode( fuse_node, "Space_Type", xsecVec[0]->get_pnt_space_type() );

  xmlAddIntNode( fuse_node, "Nose_Super_Flag", nose_super_flag);
  xmlAddIntNode( fuse_node, "Aft_Super_Flag",  aft_super_flag);

  //==== Write Fuse Xsec Parms ====//
  xmlNodePtr csl_node = xmlNewChild( root, NULL, (const xmlChar *)"Cross_Section_List", NULL );

  for ( i = 0 ; i < (int)xsecVec.size() ; i++ )
  {
	xmlNodePtr cs_node = xmlNewChild( csl_node, NULL, (const xmlChar *)"Cross_Section", NULL );
    xsecVec[i]->write( cs_node );
  }
}

//==== Read Fuse File ====//
void Fuse_geom::read(xmlNodePtr root)
{
  int i;
  xmlNodePtr node;

  //===== Read General Parameters =====//
  node = xmlGetNode( root, "General_Parms", 0 );
  if ( node )
  {
    read_general_parms( node );
  }

  imlFlag = 0;
  int num_xsecs;
  int space_type = PNT_SPACE_FIXED;

  //===== Read Fuse Parameters =====//
  node = xmlGetNode( root, "Fuse_Parms", 0 );
  if ( node )
  {
    length     = xmlFindDouble( node, "Fuse_Length", length() );
    camber     = xmlFindDouble( node, "Camber", camber() );
    camber_loc = xmlFindDouble( node, "Camber_Location", camber_loc() );
    aft_offset = xmlFindDouble( node, "Aft_Offset", aft_offset() );
    nose_angle = xmlFindDouble( node, "Nose_Angle", nose_angle());
    nose_str   = xmlFindDouble( node, "Nose_Strength", nose_str() );
    nose_rho   = xmlFindDouble( node, "Nose_Rho", nose_rho() );
    aft_rho    = xmlFindDouble( node, "Aft_Rho", aft_rho() );

    imlFlag      = xmlFindInt( node, "IML_Flag", imlFlag );
    space_type   = xmlFindInt( node, "Space_Type", space_type );

    nose_super_flag  = xmlFindInt( node, "Nose_Super_Flag", nose_super_flag );
    aft_super_flag  = xmlFindInt( node, "Aft_Super_Flag", aft_super_flag );

  }

  node = xmlGetNode( root, "Cross_Section_List", 0 );
  if ( node  )
  {
    xmlNodePtr xsec_list_node = node;
    num_xsecs = xmlGetNumNames( xsec_list_node, "Cross_Section" );

    //==== Load & Read Fuse Xsec Parms ====//
	DeleteAllXSecs();
	//for ( i = 0 ; i < (int)xsecVec.size() ; i++ )
	//{
	//	RemoveXSecParmReferences( curr_xsec );
	//	delete xsecVec[i];
	//}

	//xsecVec.clear();

    for ( i = 0 ; i < num_xsecs ; i++ )
    {
      xmlNodePtr xsec_node = xmlGetNode( xsec_list_node, "Cross_Section", i );
	  fuse_xsec* xs = new fuse_xsec( this );
	  xsecVec.push_back( xs );

	  //==== Set Num Nose/Aft Sections ====//
	  if ( i == 0 || i == num_xsecs-2 )
		  xs->setNumSectInterp2(6);
	  else if ( i == 1 || i == num_xsecs-1 )
		  xs->setNumSectInterp1(6);

      xs->read(xsec_node);
      xs->gen_parms();
      xs->generate();
    }
  }

  //===== Set Flag =====
  set_iml_flag( imlFlag );
  set_pnt_space_type( space_type );
  generate();
  parmMgrPtr->RebuildAll();

}


//==== Read Fuse File ====//
void Fuse_geom::read(FILE* file_id)
{
  int i;
  char buff[255];

  //==== Read General Parms ====// 
  read_general_parms(file_id);

  imlFlag = 0;

  fgets(buff, 80, file_id); 

  //==== Read Fuse Parms ====// 
  length.read( file_id );
  camber.read( file_id );
  camber_loc.read( file_id );
  aft_offset.read( file_id );
  nose_angle.read( file_id );
  nose_str.read( file_id );
  nose_rho.read( file_id );
  aft_rho.read( file_id );

  //==== Check for IML Stuff - else Old File ====//
  fgets(buff, 80, file_id );
  int num_xsecs;
  int space_type = PNT_SPACE_FIXED;

  Stringc parm_desc(buff);
  if ( parm_desc.search_for_substring("Number") >= 0 )
  {
    sscanf( buff, "%d",&num_xsecs);	
  }
  else
  {
    sscanf(buff,    "%d",&imlFlag);	
	int color;
    fscanf(file_id, "%d",&color);				fgets(buff, 80, file_id);

    fscanf(file_id, "%d",&num_xsecs);   		fgets(buff, 80, file_id );
    fscanf(file_id, "%d",&space_type);   		fgets(buff, 80, file_id );
  }

    //==== Load & Read Fuse Xsec Parms ====//
	DeleteAllXSecs();
	//for ( i = 0 ; i < (int)xsecVec.size() ; i++ )
	//	delete xsecVec[i];

	//xsecVec.clear();

    for ( i = 0 ; i < num_xsecs ; i++ )
    {
      fgets(buff, 80, file_id);
printf("%s \n", buff );
 	  fuse_xsec* xs = new fuse_xsec( this );
	  xsecVec.push_back( xs );
      xs->read(file_id);
      xs->gen_parms();
      xs->generate();
    }
/*
  //==== Load & Read Fuse Xsec Parms ====// 
  fuse_xsec* temp_xsec;
  xsecs.clear();
  for ( i = 0 ; i < num_xsecs ; i++)
    {
      fgets(buff, 80, file_id);
      temp_xsec = xsecs.insert_new();
      temp_xsec->read(file_id);
      temp_xsec->set_fuse_ptr(this);
      temp_xsec->gen_parms();
      temp_xsec->generate();
      xsecs.inc();
    }
*/
  set_iml_flag( imlFlag );
  set_pnt_space_type( space_type );

  generate();
  parmMgrPtr->RebuildAll();

}

//==== Compute Spine =====//
void Fuse_geom::comp_spine()
{
  int i;
  spine.load_pnt(0, vec3d(0.0, 0.0, 0.0));
  spine.load_tan(0, vec3d( nose_str()*length()*cos(DEG_2_RAD*nose_angle()), 0.0, 
                           nose_str()*length()*sin(DEG_2_RAD*nose_angle()) ));
  spine.load_pnt(1, vec3d(camber_loc()*length(), 0.0, camber()*length() ));
  spine.load_tan(1, vec3d(length(), 0.0, 0.0 ));

  spine.load_pnt(2, vec3d(length(), 0.0, aft_offset()*length()) );
  spine.load_tan(2, vec3d(((1.0-camber_loc())/2.0)*length(), 0.0, (aft_offset()-camber())*length()) );

  for ( i = 0 ; i < (int)xsecVec.size() ; i++ )
  {
      double loc = xsecVec[i]->get_loc_on_spine();
      vec3d sp_pnt = spine.comp_pnt_per_length( (float)loc );
      xsecVec[i]->set_pnt_on_spine(sp_pnt);
  } 
}

//==== Interpolate 2 Cross Sections =====//
void Fuse_geom::super_ellipse_interp_xsec( fuse_xsec* xsec0, fuse_xsec* xsec1, double fract,   
										   int ml_type, double rho, fuse_xsec* result )
{

	int save_ml_type0 = xsec0->get_ml_type();
	int save_ml_type1 = xsec1->get_ml_type();
	xsec0->set_ml_type( ml_type );               // Mold Line Type
	xsec1->set_ml_type( ml_type );


	int num_pnts_xsec = xsec0->get_num_pnts();
	double rh = xsec0->get_height()->get()/2.0;		double drh = xsec1->get_height()->get()/2.0 - rh;
	double rw = xsec0->get_width()->get()/2.0;		double drw = xsec1->get_width()->get()/2.0 - rw;
	double zo = xsec0->get_z_offset()->get();		double dzo = xsec1->get_z_offset()->get() - zo;
	double izo = xsec0->get_iml_z_offset();			double dizo = xsec1->get_iml_z_offset() - izo;
	double mwl = xsec0->get_max_width_loc()->get();
	double dmwl = xsec1->get_max_width_loc()->get() - mwl;
	double cr = xsec0->get_corner_rad()->get();		double dcr = xsec1->get_corner_rad()->get() - cr;
	double tta = xsec0->get_top_tan_angle()->get();
	double dtta = xsec1->get_top_tan_angle()->get() - tta; 
	double bta = xsec0->get_bot_tan_angle()->get();
	double dbta = xsec1->get_bot_tan_angle()->get() - bta;
	double ts = xsec0->get_top_str()->get();		double dts = xsec1->get_top_str()->get() - ts;
	double us = xsec0->get_upp_str()->get();		double dus = xsec1->get_upp_str()->get() - us;
	double ls = xsec0->get_low_str()->get();		double dls = xsec1->get_low_str()->get() - ls;
	double bs = xsec0->get_bot_str()->get();		double dbs = xsec1->get_bot_str()->get() - bs;
	double tt = xsec0->get_top_thick()->get();		double dtt =  xsec1->get_top_thick()->get() - tt;
	double bt = xsec0->get_bot_thick()->get();		double dbt =  xsec1->get_bot_thick()->get() - bt;
	double st = xsec0->get_side_thick()->get();		double dst =  xsec1->get_side_thick()->get() - st;

 //   fract = 1.0 - cos(fract*PI/2.0);		// Cos Spacing
    double sup_fract = 1.0f - super_ellipse( (float)fract, (float)rho );

    result->set_ml_type( ml_type );

	result->set_type( GENERAL );
	result->get_height()->set( (rh + sup_fract*drh)*2.0 );
    result->get_width()->set( (rw + sup_fract*drw)*2.0);
    result->get_z_offset()->set( zo + sup_fract*dzo);


    result->get_max_width_loc()->set(mwl + sup_fract*dmwl);
    result->get_corner_rad()->set(cr + sup_fract*dcr);
    result->get_top_tan_angle()->set(tta + sup_fract*dtta); 
    result->get_bot_tan_angle()->set(bta + sup_fract*dbta);
    result->get_top_str()->set(ts + sup_fract*dts);
    result->get_upp_str()->set(us + sup_fract*dus);
    result->get_low_str()->set(ls + sup_fract*dls);
    result->get_bot_str()->set(bs + sup_fract*dbs);

    result->set_num_pnts(num_pnts_xsec);

	if ( xsec0->get_type() == FROM_FILE ||  xsec1->get_type() == FROM_FILE )
	{
		interpolate_from_file( ml_type, (float)sup_fract, xsec0, xsec1, result );
    }
	else if ( xsec0->get_type() == EDIT_CRV ||  xsec1->get_type() == EDIT_CRV )
	{
		interpolate_from_edit_crv( ml_type, (float)sup_fract, xsec0, xsec1, result );
	}

	xsec0->set_ml_type( save_ml_type0 );               // Mold Line Type
	xsec1->set_ml_type( save_ml_type1 );

}

//==== Interpolate Xsecs That are From Files =====//
void Fuse_geom::interpolate_from_file
			( int mlt, float sup_fract, fuse_xsec* xsec0, fuse_xsec* xsec1, fuse_xsec* out )
{
  if ( xsec0->get_type() == XSEC_POINT )
  {
    out->set_type( FROM_FILE );
    out->set_file_crv( xsec1->get_file_crv(mlt) );
  }
  else if ( xsec0->get_type() == FROM_FILE &&  xsec1->get_type() == FROM_FILE )
  {
    out->set_type( FROM_FILE );
    out->interp_file_crv( sup_fract, xsec0->get_file_crv(mlt), xsec1->get_file_crv(mlt) );
  }

}

//==== Interpolate Xsecs That are From Edit Crvs =====//
void Fuse_geom::interpolate_from_edit_crv
			( int mlt, float sup_fract, fuse_xsec* xsec0, fuse_xsec* xsec1, fuse_xsec* out )
{
  if ( xsec0->get_type() == XSEC_POINT )
  {

    out->setEditCrv( xsec1->getEditCrv(mlt) );
	double w = xsec1->getEditCrv(mlt)->getMaxWidth()->get();
	double h = xsec1->getEditCrv(mlt)->getMaxHeight()->get();
	out->getEditCrv(mlt)->scaleWH( w*sup_fract, h*sup_fract ); 
    out->set_type( EDIT_CRV );
	out->getEditCrv(mlt)->generate();
  }
  else if ( xsec0->get_type() == EDIT_CRV &&  xsec1->get_type() == EDIT_CRV )
  {
	out->getEditCrv(mlt)->blend( sup_fract, xsec0->getEditCrv(mlt),xsec1->getEditCrv(mlt) );

    out->set_type( EDIT_CRV );
 	out->getEditCrv(mlt)->generate();
 }

}


//==== Set Flag all Xsecs =====//
void Fuse_geom::set_iml_flag( int flag )
{
  int i;
  imlFlag = flag;

  for ( i = 0 ; i < (int)xsecVec.size() ; i++ )
  {
	xsecVec[i]->set_iml_flag( imlFlag );
	xsecVec[i]->gen_parms();
	xsecVec[i]->generate();
  }
  generate();
}


//==== Cross Section Has Changed Regen Surf =====//
void Fuse_geom::xsec_trigger()
{
  generate();
}

//==== Generate Cross Sections =====//
void Fuse_geom::regenerate()
{
//jrg  if (screen)
//    screen->redraw();
}

//==== Generate Cross Sections =====//
void Fuse_geom::gen_surf()
{
	int i, j, n, s;
	vec3d p;
  				
	static fuse_xsec res_xsec;				// Interpolated Fuse Cross Sect
	res_xsec.set_fuse_ptr(this);

	vec3d spinePnt;
	vector< vec3d > spineVec;

	//==== Compute Number of Base Cross Sections ====//
	int num_base_pnts  = xsecVec[0]->get_num_pnts();
	int num_base_xsecs = xsecVec.size();

	if ( nose_super_flag )
		num_base_xsecs += xsecVec[0]->getNumSectInterp2();

	if ( aft_super_flag )
		num_base_xsecs += xsecVec.back()->getNumSectInterp1();

	//==== Load Stringer Point Vecs ====//
	static vector< vector< vec3d > > stringerVec;
	stringerVec.resize( num_base_pnts );
	for ( i = 0 ; i < (int)stringerVec.size() ; i++ )
		stringerVec[i].resize( num_base_xsecs );

	//==== Build Up Cross Sections to Interpolate ====//
	int xsecCnt = 0;
	for ( i = 0 ; i < (int)xsecVec.size() ; i++ )
	{
		spinePnt = xsecVec[i]->get_pnt_on_spine();
		spineVec.push_back( spinePnt );
		for (  j = 0 ; j < num_base_pnts ; j++ )		// Defined Fuse XSecs
		{
			p = xsecVec[i]->get_pnt(OML, j) + spinePnt;
			stringerVec[j][xsecCnt] = p;
		}
		xsecCnt++;

		//==== Interpolate Nose XSecs =====//
		if ( i == 0 && nose_super_flag )								
		{
			int nInterp = xsecVec[i]->getNumSectInterp2();
			for ( n = 0 ; n < 	nInterp ; n++ )
			{
				double fract = (double)(n+1)/(double)(nInterp + 1);
				fract = 1.0 - cos(fract*PI/2.0);		// Cos Spacing

				super_ellipse_interp_xsec( xsecVec[0], xsecVec[1], fract, OML, nose_rho(), &res_xsec );

				double loc = xsecVec[1]->get_loc_on_spine(); 
				spinePnt = spine.comp_pnt_per_length( (float)(fract*loc) );
				res_xsec.set_iml_flag( 0 );
				res_xsec.set_pnt_on_spine(spinePnt);
				res_xsec.set_pnt_space_type( xsecVec[0]->get_pnt_space_type() );
				res_xsec.generate();
				spinePnt = res_xsec.get_pnt_on_spine();
				spineVec.push_back( spinePnt );

				for (  j = 0 ; j < res_xsec.get_num_pnts() ; j++ )
				{
					vec3d p = res_xsec.get_pnt(OML, j) + spinePnt;
					stringerVec[j][xsecCnt] = p;

				}
				xsecCnt++;
			}
		}

		//==== Interpolate Aft XSecs =====//
		else if ( i == xsecVec.size() - 2 && aft_super_flag )
		{
			int nInterp = xsecVec[i+1]->getNumSectInterp1();
			for ( n = nInterp-1 ; n >= 	0 ; n-- )
			{
				double fract = (double)(n+1)/(double)(nInterp + 1);
				fract = 1.0 - cos(fract*PI/2.0);		// Cos Spacing

				super_ellipse_interp_xsec( xsecVec[i+1], xsecVec[i], fract, OML, aft_rho(), &res_xsec );

				double loc = xsecVec[i]->get_loc_on_spine(); 
				spinePnt = spine.comp_pnt_per_length( (float)(1.0f - (1.0f - loc)*fract) );
				res_xsec.set_iml_flag( 0 );
				res_xsec.set_pnt_on_spine(spinePnt);
				res_xsec.set_pnt_space_type( xsecVec[0]->get_pnt_space_type() );
				res_xsec.generate();
				spinePnt = res_xsec.get_pnt_on_spine();
				spineVec.push_back( spinePnt );

				for (  j = 0 ; j < res_xsec.get_num_pnts() ; j++ )
				{
					vec3d p = res_xsec.get_pnt(OML, j) + spinePnt;
					stringerVec[j][xsecCnt] = p;
				}
				xsecCnt++;
			}
		}
	}

	//==== Load Control Point Vecs ====//
	ControlPnt cp;
	cPntVec.resize( xsecVec.size()*3 );
	for ( i = 0 ; i < (int)xsecVec.size() ; i++ )
	{
		cp.xSecID = i;

		cp.pntID  = ControlPnt::TOP;				//Top Control Points
		cp.pnt3d  = xsecVec[i]->get_pnt_on_spine();
		cp.pnt3d.offset_z( xsecVec[i]->get_height()->get()/2.0 );
		cp.pnt2d = projectPoint( cp.pnt3d, 0 );
		cPntVec[i] = cp;

		cp.pntID  = ControlPnt::BOTTOM;				//Bottom Control Points
		cp.pnt3d  = xsecVec[i]->get_pnt_on_spine();
		cp.pnt3d.offset_z( -xsecVec[i]->get_height()->get()/2.0 );
		cp.pnt2d = projectPoint( cp.pnt3d, 0 );
		cPntVec[i + xsecVec.size()] = cp;

		cp.pntID  = ControlPnt::SIDE;				//Side Control Points
		cp.pnt3d  = xsecVec[i]->get_pnt_on_spine();
		cp.pnt3d.offset_y( xsecVec[i]->get_width()->get()/2.0 );
		cp.pnt2d = projectPoint( cp.pnt3d, 0 );
		cPntVec[i + xsecVec.size()*2] = cp;
	}


	//==== Build Stringers ====//
	int closeFlag = 0;
	static vector< Bezier_curve > sVec;
	sVec.resize( num_base_pnts );

	//==== Set Up Tangent Flags for Bezier Curves ====//
	static vector< int > tanFlagVec;
	tanFlagVec.resize( num_base_xsecs );

	for ( i = 0 ; i < (int)tanFlagVec.size() ; i++ )
	{
		tanFlagVec[i] = Bezier_curve::NADA;	
	}

	if ( nose_super_flag )
	{
		int off = xsecVec[0]->getNumSectInterp2() + 1;
		tanFlagVec[off] = Bezier_curve::ONLY_BACK;
	}
	else
	{
		if ( xsecVec[0]->get_type() == XSEC_POINT )
			tanFlagVec[0] = Bezier_curve::ZERO_X;
	}
	if ( aft_super_flag )
	{
		int off = num_base_xsecs - xsecVec.back()->getNumSectInterp1() - 2;
		tanFlagVec[off] = Bezier_curve::ONLY_FORWARD;
	}
	else
	{
		if ( xsecVec.back()->get_type() == XSEC_POINT )
			tanFlagVec[num_base_xsecs-1] = Bezier_curve::ZERO_X;
	}

	//==== Set Up Tangent Str Vec ====//
	static vector< double > tanAngVec;
	static vector< double > tanStr1Vec;
	static vector< double > tanStr2Vec;
	tanAngVec.resize( num_base_xsecs );
	tanStr1Vec.resize( num_base_xsecs );
	tanStr2Vec.resize( num_base_xsecs );

	int tInd = 0;
	for ( i = 0 ; i < (int)xsecVec.size() ; i++ )
	{
		tanAngVec[tInd] = xsecVec[i]->getProfileTanAng()->get();
		tanStr1Vec[tInd] = xsecVec[i]->getProfileTanStr1()->get();
		tanStr2Vec[tInd] = xsecVec[i]->getProfileTanStr2()->get();
		tInd++;

		if ( i == 0 && nose_super_flag )
			tInd += xsecVec[i]->getNumSectInterp2();

		if ( i == xsecVec.size()-2 && aft_super_flag )
			tInd += xsecVec[i]->getNumSectInterp2();
	}

	for ( s = 0 ; s < (int)sVec.size() ; s++ )
	{
		sVec[s].buildCurve( stringerVec[s], spineVec, tanFlagVec, tanAngVec, tanStr1Vec, tanStr2Vec, closeFlag);
	}

	//==== Compute Total Number of Interpolate Cross Sections =====//
	int num_inter_xsecs =  xsecVec.size();
	for ( i = 0 ; i < (int)xsecVec.size()-1 ; i++ )
		num_inter_xsecs += xsecVec[i]->getNumSectInterp2();

	vector< double > uVec;				// Vector of U vals 
	double uVal = 0.0;
	uVec.push_back(uVal);

	for ( int ix = 0 ; ix < (int)xsecVec.size()-1 ; ix++ )
	{
		for ( j = 0 ; j < (int)xsecVec[ix]->getNumSectInterp2()+1 ; j++ )		// Nose
		{
			if ( ix == 0 && nose_super_flag )
				uVal += 1.0;
			else if ( ix == xsecVec.size()-2 && aft_super_flag )
				uVal += 1.0;
			else
				uVal += 1.0/(double)(xsecVec[ix]->getNumSectInterp2()+1);

			uVec.push_back(uVal);
		}
	}

	//uVal = (int)uVal+1.0;				// Last Section
	//uVec.push_back(uVal);

	oml_surf.set_num_pnts( num_base_pnts );
	oml_surf.set_num_xsecs( uVec.size() );

	for ( s = 0 ; s < (int)sVec.size() ; s++ )
	{
		for ( int ix = 0 ; ix < oml_surf.get_num_xsecs() ; ix++ )
		{
			double u  = uVec[ix];
			int sect  = (int)u;
			vec3d p = sVec[s].comp_pnt(sect, u-(double)sect);
			oml_surf.set_pnt( ix, s, p );
		}
	}

	oml_surf.load_refl_pnts_xsecs();

	//=======================================//
	//==== Load IML Surf ====//
	if ( imlFlag )
	{
		//==== Build Up Cross Sections to Interpolate ====//
		int xsecCnt = 0;
		for ( i = 0 ; i < (int)xsecVec.size() ; i++ )
		{
			spinePnt = xsecVec[i]->get_pnt_on_spine();
			for (  j = 0 ; j < num_base_pnts ; j++ )		// Defined Fuse XSecs
			{
				p = xsecVec[i]->get_pnt(IML, j) + spinePnt;
				stringerVec[j][xsecCnt] = p;
			}
			xsecCnt++;

			//==== Interpolate Nose XSecs =====//
			if ( i == 0 && nose_super_flag )								
			{
				int nInterp = xsecVec[i]->getNumSectInterp2();
				for ( n = 0 ; n < 	nInterp ; n++ )
				{
					double fract = (double)(n+1)/(double)(nInterp + 1);
					fract = 1.0 - cos(fract*PI/2.0);		// Cos Spacing

					super_ellipse_interp_xsec( xsecVec[0], xsecVec[1], fract, IML, nose_rho(), &res_xsec );

					double loc = xsecVec[1]->get_loc_on_spine(); 
					spinePnt = spine.comp_pnt_per_length( (float)(fract*loc) );
					res_xsec.set_iml_flag( 1 );
					res_xsec.set_pnt_on_spine(spinePnt);
				    res_xsec.set_pnt_space_type( xsecVec[0]->get_pnt_space_type() );
					res_xsec.generate();
					spinePnt = res_xsec.get_pnt_on_spine();

					for (  j = 0 ; j < res_xsec.get_num_pnts() ; j++ )
					{
						vec3d p = res_xsec.get_pnt(IML, j) + spinePnt;
						stringerVec[j][xsecCnt] = p;

					}
					xsecCnt++;
				}
			}

			//==== Interpolate Aft XSecs =====//
			else if ( i == xsecVec.size() - 2 && aft_super_flag )
			{
				int nInterp = xsecVec[i+1]->getNumSectInterp1();
				for ( n = nInterp-1 ; n >= 	0 ; n-- )
				{
					double fract = (double)(n+1)/(double)(nInterp + 1);
					fract = 1.0 - cos(fract*PI/2.0);		// Cos Spacing

					super_ellipse_interp_xsec( xsecVec[i+1], xsecVec[i], fract, IML, aft_rho(), &res_xsec );

					double loc = xsecVec[i]->get_loc_on_spine(); 
					spinePnt = spine.comp_pnt_per_length( (float)(1.0 - (1.0 - loc)*fract) );
					res_xsec.set_iml_flag( 1 );
					res_xsec.set_pnt_on_spine(spinePnt);
				    res_xsec.set_pnt_space_type( xsecVec[0]->get_pnt_space_type() );
					res_xsec.generate();
					spinePnt = res_xsec.get_pnt_on_spine();

					for (  j = 0 ; j < res_xsec.get_num_pnts() ; j++ )
					{
						vec3d p = res_xsec.get_pnt(IML, j) + spinePnt;
						stringerVec[j][xsecCnt] = p;
					}
					xsecCnt++;
				}
			}
		}

		for ( s = 0 ; s < (int)sVec.size() ; s++ )
		{
			sVec[s].buildCurve( stringerVec[s], spineVec, tanFlagVec, tanAngVec, tanStr1Vec, tanStr2Vec, closeFlag);
		}

		iml_surf.set_num_pnts( num_base_pnts );
		iml_surf.set_num_xsecs( uVec.size() );

		for ( s = 0 ; s < (int)sVec.size() ; s++ )
		{
			for ( int ix = 0 ; ix < iml_surf.get_num_xsecs() ; ix++ )
			{
				double u  = uVec[ix];
				int sect  = (int)u;
				vec3d p = sVec[s].comp_pnt(sect, u-(double)sect);
				iml_surf.set_pnt( ix, s, p );
			}
		}

		iml_surf.load_refl_pnts_xsecs();
	}
    
  oml_surf.load_uw();
  load_hidden_surf();
  load_normals();
  update_bbox(); 

}

//==== Compute And Load Normals ====//
void Fuse_geom::load_normals()
{
  oml_surf.load_normals();

  if ( imlFlag )
	  iml_surf.load_normals();
}

//==== Draw Hidden Surface====//
void Fuse_geom::load_hidden_surf()
{
  oml_surf.load_hidden_surf();	

  if ( imlFlag )
    iml_surf.load_hidden_surf();
}


//==== Get Cross_Section Number =====//
fuse_xsec* Fuse_geom::get_xsec(int number)
{
  if ( number >= 0 && number < (int)xsecVec.size() )
	  return xsecVec[number];
  return xsecVec[0];
}

//==== Add Cross Section =====//
fuse_xsec* Fuse_geom::add_xsec(fuse_xsec* curr_xsec)
{
	int i;

	//==== Dont add past last xsec ====//
	if ( curr_xsec_num >= (int)xsecVec.size()-1 )
		return xsecVec[curr_xsec_num];

    double loc1 = xsecVec[curr_xsec_num]->get_loc_on_spine();
    double loc2 = xsecVec[curr_xsec_num+1]->get_loc_on_spine();

	fuse_xsec* add_xsec = new fuse_xsec(this);
	add_xsec->gen_parms();
	*add_xsec = *(xsecVec[curr_xsec_num]);
	add_xsec->set_iml_flag( imlFlag );
	add_xsec->generate();
	add_xsec->set_fuse_ptr( this );

	add_xsec->set_loc_on_spine( 0.5f*(loc1+loc2) );

	vector< fuse_xsec* > tmpVec;

	for (  i = 0 ; i <= curr_xsec_num ; i++ )
		tmpVec.push_back( xsecVec[i] );

	tmpVec.push_back( add_xsec );

	for (  i = curr_xsec_num+1 ; i < (int)xsecVec.size() ; i++ )
		tmpVec.push_back( xsecVec[i] );

	xsecVec = tmpVec;

	curr_xsec_num += 1;

	add_xsec->setNumSectInterp1( curr_xsec->getNumSectInterp2() );
	add_xsec->setNumSectInterp2( curr_xsec->getNumSectInterp2() );

	curr_xsec = add_xsec;

	updateNumInter();

	this->generate();

	parmMgrPtr->RebuildAll();

	return curr_xsec;
}


//==== Delete Cross Section =====//
fuse_xsec* Fuse_geom::delete_xsec(fuse_xsec* curr_xsec)
{
	int i;
	//==== Keep At Least 3 Cross Sections ====//
	if ( xsecVec.size() < 4 )
		return (curr_xsec);

	//==== Dont Delete First or Last Cross Section ====//
	if ( curr_xsec_num == 0 || curr_xsec_num == xsecVec.size()-1 )
		return (curr_xsec);

	vector< fuse_xsec* > tmpVec;

	for (  i = 0 ; i < curr_xsec_num ; i++ )
		tmpVec.push_back( xsecVec[i] );

	for (  i = curr_xsec_num+1 ; i < (int)xsecVec.size() ; i++ )
		tmpVec.push_back( xsecVec[i] );

	xsecVec = tmpVec;

	//==== Save Cross Section Parameters ====//
//	saved_xsec = *curr_xsec;
	saved_xsec.copy( *curr_xsec );

	RemoveXSecParmReferences( curr_xsec );
//	delete curr_xsec;		//jrg Leak to Fix Crash - This comp is obsolete anyways

	curr_xsec = xsecVec[curr_xsec_num];

	updateNumInter();

	this->generate();

	parmMgrPtr->RebuildAll();

	return curr_xsec;
}

//==== Copy Cross Section =====//
void Fuse_geom::copy_xsec(fuse_xsec* curr_xsec)
{
//  saved_xsec = *curr_xsec;

  saved_xsec.copy( *curr_xsec );

}

//==== Paste Cross Section =====//
void Fuse_geom::paste_xsec(fuse_xsec* curr_xsec)
{
  double curr_loc = curr_xsec->get_loc_on_spine();

//  *curr_xsec = saved_xsec;
  //==== Save shape parms ====//
  double profileTanStr1 = curr_xsec->getProfileTanStr1()->get();
  double profileTanStr2 = curr_xsec->getProfileTanStr2()->get();
  double profileTanAng  = curr_xsec->getProfileTanAng()->get();
  int numSectInterp1 = curr_xsec->getNumSectInterp1();
  int numSectInterp2 = curr_xsec->getNumSectInterp2();

  curr_xsec->copy( saved_xsec );

  //==== Restore shape parms ====//
  curr_xsec->getProfileTanStr1()->set( profileTanStr1 );
  curr_xsec->getProfileTanStr2()->set( profileTanStr2 );
  curr_xsec->getProfileTanAng()->set( profileTanAng );
  curr_xsec->setNumSectInterp1( numSectInterp1 );
  curr_xsec->setNumSectInterp2( numSectInterp2 );

  curr_xsec->generate();

  curr_xsec->set_loc_on_spine( curr_loc );

  this->generate();
}


void Fuse_geom::set_pnt_space_type( int type )
{
  int i;
  for ( i = 0 ; i < (int)xsecVec.size() ; i++ )
  {
	xsecVec[i]->set_pnt_space_type( type );
	xsecVec[i]->generate();
  }
  this->generate();
}

void Fuse_geom::set_num_pnts( int num_in )
{
  int i;
  for ( i = 0 ; i < (int)xsecVec.size() ; i++ )
  {
	xsecVec[i]->set_num_pnts(num_in);
	xsecVec[i]->generate();
  }
  this->generate();
}

void Fuse_geom::setImlMaterialID( int m )
{
	if ( m >= 0 && m < matMgrPtr->getNumMaterial() )
	{
		imlMaterialID = m;
		airPtr->geomMod( this );
	}
}

void Fuse_geom::draw()
{
	if ( fastDrawFlag )
	{
		oml_surf.fast_draw_on();
		if ( imlFlag )
			iml_surf.fast_draw_on();
	}
	else
	{
		oml_surf.fast_draw_off();
		if ( imlFlag )
			iml_surf.fast_draw_off();
	}
	//==== Draw Highlighting Boxes ====//
	draw_highlight_boxes();

	//==== Set Up Xsec Hightlighting ====//
	if ( redFlag )
	{
		oml_surf.set_highlight_xsec_flag(1);
		int id = 0;
		for ( int i = 0 ; i < curr_xsec_num ; i++ )
		{
			id += 1 + xsecVec[i]->getNumSectInterp2();
		}
		oml_surf.set_highlight_xsec_id( id );		//jrg iml??
	}
	else
	{
		oml_surf.set_highlight_xsec_flag(0);
	}

	//==== Check Noshow Flag ====//
	if ( noshowFlag ) return;	

	if ( displayFlag == GEOM_WIRE_FLAG )
	{
		glColor3ub( (int)color.x(), (int)color.y(), (int)color.z() );	

		//==== Draw Geom ====//
		glPushMatrix();
		glMultMatrixf((GLfloat*)model_mat); 
		oml_surf.draw_wire();
		if ( imlFlag )
		{
			glColor3ub( (int)imlColor.x(), (int)imlColor.y(), (int)imlColor.z() );	
			iml_surf.draw_wire();
			glColor3ub( (int)color.x(), (int)color.y(), (int)color.z() );
		}

		if ( redFlag )
			drawControlPoints();

		glPopMatrix();

		//==== Reflected Geom ====//
		glPushMatrix();
		glMultMatrixf((GLfloat*)reflect_mat); 
		glColor3ub( (int)color.x(), (int)color.y(), (int)color.z() );	
		oml_surf.draw_refl_wire(sym_code);
		if ( imlFlag )
		{
			glColor3ub( (int)imlColor.x(), (int)imlColor.y(), (int)imlColor.z() );	
			iml_surf.draw_refl_wire(sym_code);
			glColor3ub( (int)color.x(), (int)color.y(), (int)color.z() );
		}
		glPopMatrix();
	}
	else if ( displayFlag == GEOM_SHADE_FLAG || displayFlag == GEOM_TEXTURE_FLAG )
	{
		//==== Draw Geom ====//
		glPushMatrix();
		glMultMatrixf((GLfloat*)model_mat); 

		//==== Draw Iml ====//
		if ( imlFlag )
		{
			Material* mat = matMgrPtr->getMaterial( imlMaterialID );
			if ( mat )
			{
				mat->bind();
				if ( mat->diff[3] > 0.99 )
				{
					iml_surf.draw_shaded();
				}
			}
		}
		Material* mat = matMgrPtr->getMaterial( materialID );
		if ( mat )
		{
			mat->bind();
			if ( mat->diff[3] > 0.99 )
			{
				oml_surf.draw_shaded();

				if ( displayFlag == GEOM_TEXTURE_FLAG )
					drawTextures(false);
			}
		}
		if ( redFlag )
			drawControlPoints();

		glPopMatrix();

		//==== Reflected Geom ====//
		glPushMatrix();
		glMultMatrixf((GLfloat*)reflect_mat); 
		if ( imlFlag )
		{
			mat = matMgrPtr->getMaterial( imlMaterialID );
			if ( mat )
			{
				mat->bind();
				if  ( mat->diff[3] > 0.99 )
				{
					iml_surf.draw_refl_shaded( sym_code);
				}
			}
		}
		mat = matMgrPtr->getMaterial( materialID );
		if ( mat )
		{
			mat->bind();
			if  ( mat->diff[3] > 0.99 )
			{
				oml_surf.draw_refl_shaded( sym_code);
				
				if ( displayFlag == GEOM_TEXTURE_FLAG )
					drawTextures(true);
			}
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
		oml_surf.draw_hidden();
		if ( imlFlag )
			iml_surf.draw_hidden();

		if ( redFlag )
			drawControlPoints();

		glPopMatrix();

		//==== Reflected Geom ====//
		glColor3ub( (int)color.x(), (int)color.y(), (int)color.z() );	
		glPushMatrix();
		glMultMatrixf((GLfloat*)reflect_mat); 
		oml_surf.draw_refl_hidden(sym_code);
		if ( imlFlag )
			iml_surf.draw_refl_hidden(sym_code);
		glPopMatrix();
			
	}
}

//==== Draw If Alpha < 1 and Shaded ====//
void Fuse_geom::drawAlpha()
{
	if ( displayFlag != GEOM_SHADE_FLAG &&  displayFlag == !GEOM_TEXTURE_FLAG)
		return;

	//==== Check Noshow Flag ====//
	if ( noshowFlag ) return;	

	if ( imlFlag )
	{
		Material* mat = matMgrPtr->getMaterial( imlMaterialID );
		if ( mat && mat->diff[3] <= 0.99 )
		{
			//==== Draw Geom ====//
			glPushMatrix();
			glMultMatrixf((GLfloat*)model_mat); 

			mat->bind();
			iml_surf.draw_shaded();

			glPopMatrix();

			//==== Reflected Geom ====//
			glPushMatrix();
			glMultMatrixf((GLfloat*)reflect_mat); 

			mat->bind();
			iml_surf.draw_refl_shaded( sym_code);

			glPopMatrix();
		}
	}


	Material* mat = matMgrPtr->getMaterial( materialID );
	if ( mat && mat->diff[3] <= 0.99 )
	{
		//==== Draw Geom ====//
		glPushMatrix();
		glMultMatrixf((GLfloat*)model_mat); 

		mat->bind();
		oml_surf.draw_shaded();
		
		if ( displayFlag == GEOM_TEXTURE_FLAG )
			drawTextures(false);

		glPopMatrix();

		//==== Reflected Geom ====//
		glPushMatrix();
		glMultMatrixf((GLfloat*)reflect_mat); 

		oml_surf.draw_refl_shaded( sym_code);

		if ( displayFlag == GEOM_TEXTURE_FLAG )
			drawTextures(true);


		glPopMatrix();
	}


}


float Fuse_geom::get_area()
{
  int i;
  double loc, old_loc, width, old_width;
  double sec_area, plan_area = 0.0;

  old_loc = old_width = 0.0;

  for ( i = 0 ; i < (int)xsecVec.size() ; i++ )
  {
    loc = xsecVec[i]->get_loc_on_spine();
    width = xsecVec[i]->get_width()->get(); 

    if ( i > 0 )
    {
      sec_area = (loc-old_loc)*(float)length()*0.5*(old_width+width);
      plan_area += sec_area;
    }
    old_loc = loc;
    old_width = width;
  }

  return( (float)plan_area);

}


float Fuse_geom::get_span()
{
  double width, span = 0.0;
  for ( int i = 0 ; i < (int)xsecVec.size() ; i++ )
  {

    width = xsecVec[i]->get_width()->get(); 
    if ( width > span )
    {
      span = width;
    }
  }
  return( (float)span );
}

//==== Get Cbar ====//
float Fuse_geom::get_cbar()
{
  return( (float)length() );
}



//==== If IML, Then Write Out Inner Surface ====//
void Fuse_geom::dump_xsec_file(int geom_no, FILE* dump_file)
{
	int i;
	vec3d p;

	//==== Only Write Out OML not IML ====//
	fprintf(dump_file, "\n");
	fprintf(dump_file, "%s \n",(char*) getName());
	fprintf(dump_file, " GROUP NUMBER      = %d \n",geom_no);
	fprintf(dump_file, " TYPE              = 1\n");
	fprintf(dump_file, " CROSS SECTIONS    = %d \n",oml_surf.get_num_xsecs() );
	fprintf(dump_file, " PTS/CROSS SECTION = %d \n",oml_surf.get_num_pnts() );

	// Write out cross sections 
	for ( i = 0 ; i < oml_surf.get_num_xsecs() ; i++ ) 
	{
		oml_surf.write_xsec(i, model_mat, dump_file);      
	}

	if ( sym_code == NO_SYM ) return;

	fprintf(dump_file, "\n");
	fprintf(dump_file, "%s \n", (char*) getName());
	fprintf(dump_file, " GROUP NUMBER      = %d \n",geom_no);
	fprintf(dump_file, " TYPE              = 1  \n");						// Non Lifting
	fprintf(dump_file, " CROSS SECTIONS    = %d \n",oml_surf.get_num_xsecs());
	fprintf(dump_file, " PTS/CROSS SECTION = %d \n",oml_surf.get_num_pnts());

	for ( i = 0 ; i < oml_surf.get_num_xsecs() ; i++ )
    {
      oml_surf.write_refl_xsec(sym_code, i, reflect_mat, dump_file);
    }

/*
  if ( !imlFlag )
  {
	  fprintf(dump_file, "\n");
	  fprintf(dump_file, "%s \n",(char*) getName());
	  fprintf(dump_file, " GROUP NUMBER      = %d \n",geom_no);
	  fprintf(dump_file, " TYPE              = 1\n");
	  fprintf(dump_file, " CROSS SECTIONS    = %d \n",oml_surf.get_num_xsecs() );
	  fprintf(dump_file, " PTS/CROSS SECTION = %d \n",oml_surf.get_num_pnts() );

	  // Write out cross sections 
	  for ( i = 0 ; i < oml_surf.get_num_xsecs() ; i++ ) 
	  {
		  oml_surf.write_xsec(i, model_mat, dump_file);      
	  }
  }
  else
  {
	  fprintf(dump_file, "\n");
	  fprintf(dump_file, "%s \n",(char*) getName());
	  fprintf(dump_file, " GROUP NUMBER      = %d \n",geom_no);
	  fprintf(dump_file, " TYPE              = 1\n");
	  fprintf(dump_file, " CROSS SECTIONS    = %d \n",oml_surf.get_num_xsecs()+iml_surf.get_num_xsecs() );
	  fprintf(dump_file, " PTS/CROSS SECTION = %d \n",oml_surf.get_num_pnts() );

	  // Write out oml cross sections 
	  for ( i = 0 ; i < oml_surf.get_num_xsecs() ; i++ ) 
	  {
		  oml_surf.write_xsec(i, model_mat, dump_file);      
	  }

	  // Write out iml cross sections 
	  for ( i = iml_surf.get_num_xsecs()-1 ; i >= 0 ; i-- ) 
	  {
		  iml_surf.write_xsec(i, model_mat, dump_file);      
	  }
  }
*/
}


//==== Return Number Of Felisa Surfaces Written Out ====//
int Fuse_geom::get_num_felisa_comps()
{
  int i, j;

  //==== Load Oml into pnt_xsec array ====//
  int num_xsecs = oml_surf.get_num_xsecs();
  int num_pnts  = oml_surf.get_num_pnts();
  pnt_xsec.init( num_xsecs, num_pnts);
  for ( i = 0 ; i < num_xsecs ; i++ )
    for ( j = 0 ; j < num_pnts ; j++ )
      pnt_xsec( i, j ) = oml_surf.get_pnt( i, j );
 
  //==== Number of Comps Outer ====//
  int num_felisa = get_num_felisa_comps_single_surf();

  //==== Check for Inner Surface ====//
  if ( imlFlag )
  {
    //==== Load Iml into pnt_xsec array ====//
    int num_xsecs = iml_surf.get_num_xsecs();
    int num_pnts  = iml_surf.get_num_pnts();
    pnt_xsec.init( num_xsecs, num_pnts);
    for ( i = 0 ; i < num_xsecs ; i++ )
      for ( j = 0 ; j < num_pnts ; j++ )
        pnt_xsec( i, j ) = iml_surf.get_pnt( i, j );

    //==== Dump Inner ====//
    num_felisa += get_num_felisa_comps_single_surf();
  }
  return num_felisa;
}


//==== Return Number Of Felisa Surfaces Written Out ====//
int Fuse_geom::get_num_felisa_comps_single_surf()
{

  int   num_surf, j;
  double dist, eps = 0.1f;
  vec3d test1, test2; 

  /* kludge for forcing fuselage splitting in certain ways */
  
  num_surf = 0;

  test1 = pnt_xsec(0,0).transform(model_mat);
  test2 = pnt_xsec(pnt_xsec.dimen_1()-1,pnt_xsec.dimen_2()-1).transform(model_mat);

  /* body lies on symmetry plane, write out semi-pie surface */

  if ( test1.y() <= 0. && test2.y() <= 0. && (strstr(getName(),"split_tb") == NULL) ) {

     num_surf += 1;

  }

  /* body lies on symmetry plane, split body into top and bottom parts */

  else if ( test1.y() <= 0. && test2.y() <= 0. && (strstr(getName(),"split_tb") != NULL) ) { 

     num_surf += 2;

  }

  /* body lies off symmetry plane, split body into top and bottom parts */

  else if ( test1.y() > 0. && test2.y() > 0. && (strstr(getName(),"split_tb") != NULL) ) { 

     num_surf += 2;

  }

  /* body lies off symmetry plane, split body into left and right parts */

  else { 

     num_surf += 2;

  }

  /* check for open nose section */

  dist = 0.;

  for ( j = 0 ; j < pnt_xsec.dimen_2() - 1 ; j++ ) {

    test1 = pnt_xsec(0,j  ).transform(model_mat);
    test2 = pnt_xsec(0,j+1).transform(model_mat);

    dist += sqrt( pow(test2.x() - test1.x(),2.)
                + pow(test2.y() - test1.y(),2.)
                + pow(test2.z() - test1.z(),2.) );

  }

  if ( dist >= eps ) {

     num_surf += 1;

  }

  /* check for open aft section */

  dist = 0.;

  for ( j = 0 ; j < pnt_xsec.dimen_2() - 1 ; j++ ) {

    test1 = pnt_xsec(pnt_xsec.dimen_1()-1,j  ).transform(model_mat);
    test2 = pnt_xsec(pnt_xsec.dimen_1()-1,j+1).transform(model_mat);

    dist += sqrt( pow(test2.x() - test1.x(),2.)
                + pow(test2.y() - test1.y(),2.)
                + pow(test2.z() - test1.z(),2.) );

  }

  if ( dist >= eps ) {

     num_surf += 1;

  }

  return(num_surf);

}

//==== Write Out Felisa Surfaces To File ====//
int Fuse_geom::write_felisa_file(int geom_no, FILE* dump_file)
{
  int i, j;

  //==== Load Oml into pnt_xsec array ====//
  int num_xsecs = oml_surf.get_num_xsecs();
  int num_pnts  = oml_surf.get_num_pnts();
  pnt_xsec.init( num_xsecs, num_pnts);
  for ( i = 0 ; i < num_xsecs ; i++ )
    for ( j = 0 ; j < num_pnts ; j++ )
      pnt_xsec( i, j ) = oml_surf.get_pnt( i, j );

  //==== Write Outer ====//
  int geom_id = write_felisa_file_single_surf(geom_no, dump_file);

  //==== Check for Inner ====//
  if ( imlFlag )
  {
     //==== Load Iml into pnt_xsec array ====//
    int num_xsecs = iml_surf.get_num_xsecs();
    int num_pnts  = iml_surf.get_num_pnts();
    pnt_xsec.init( num_xsecs, num_pnts);
    for ( i = 0 ; i < num_xsecs ; i++ )
      for ( j = 0 ; j < num_pnts ; j++ )
        pnt_xsec( i, j ) = iml_surf.get_pnt( i, j );


    //==== Dump Inner ====//
    geom_id = write_felisa_file_single_surf(geom_id, dump_file);

  }
  return geom_id;

}

//==== Write Out Felisa Surfaces To File ====//
int Fuse_geom::write_felisa_file_single_surf(int geom_no, FILE* dump_file)
{
 
   int num_geoms = geom_no;
  int i, j, num_pnts;
  vec3d temp, test1, test2; 
  int quart;
  double dist, eps = 0.1f;

  test1 = pnt_xsec(0,0).transform(model_mat);
  test2 = pnt_xsec(pnt_xsec.dimen_1()-1,pnt_xsec.dimen_2()-1).transform(model_mat);

  /* body lies on symmetry plane, write out semi-pie surface */
  if ( test1.y() <= 0. && test2.y() <= 0. && (strstr(getName(),"split_tb") == NULL) ) {

     fprintf(dump_file, "\n");
     fprintf(dump_file, "%d		Surface_Number\n", num_geoms);
     num_geoms++;
     fprintf(dump_file, "%s		Name\n", (char*) getName());
     num_pnts = pnt_xsec.dimen_2() - 4;
     quart = (pnt_xsec.dimen_2()-1)/4;

     fprintf(dump_file, "2			U_Render\n");
     fprintf(dump_file, "2			W_Render\n");
     fprintf(dump_file, "%d		Num_xsecs\n",pnt_xsec.dimen_1());
     fprintf(dump_file, "%d		Num_pnts\n", num_pnts);

     for ( i = 0 ; i < pnt_xsec.dimen_1() ; i++ ) {

        for ( j = 3*quart+2 ; j <= pnt_xsec.dimen_2()-2 ; j++ ) {

           temp = pnt_xsec(i,j).transform(model_mat);

           fprintf(dump_file, "%11.6f  %11.6f  %11.6f\n", temp.x(), temp.y(),temp.z());

        } 

        for ( j = 0 ; j <= 3*quart-2 ; j++ ) {
           
           temp = pnt_xsec(i,j).transform(model_mat);

           fprintf(dump_file, "%11.6f  %11.6f  %11.6f\n", temp.x(), temp.y(),temp.z());

        } 
     }
  }

  /* body lies on symmetry plane, split body into top and bottom parts */
  /* OR body lies off symmetry plane, split body into top and bottom parts */

  else if ( ( test1.y() <= 0. && test2.y() <= 0. && (strstr(getName(),"split_tb") != NULL) ) ||
            ( test1.y() >  0. && test2.y() >  0. && (strstr(getName(),"split_tb") != NULL) ) ) {

     fprintf(dump_file, "\n");
     fprintf(dump_file, "%d		Surface_Number\n", num_geoms);
     num_geoms++;
     fprintf(dump_file, "%s		Name\n", (char*) getName());
     num_pnts = pnt_xsec.dimen_2()/2 + 1;
     quart = (pnt_xsec.dimen_2()-1)/4;

     fprintf(dump_file, "2			U_Render\n");
     fprintf(dump_file, "2			W_Render\n");
     fprintf(dump_file, "%d		Num_xsecs\n",pnt_xsec.dimen_1());
     fprintf(dump_file, "%d		Num_pnts\n", num_pnts);

     for (  i = 0 ; i < pnt_xsec.dimen_1() ; i++ ) {
        
        for (  j = quart ; j < 3*quart+1 ; j++ ) {
            
           temp = pnt_xsec(i,j).transform(model_mat);

           fprintf(dump_file, "%11.6f  %11.6f  %11.6f\n", temp.x(), temp.y(),temp.z());
        }
     }

     fprintf(dump_file, "\n");
     fprintf(dump_file, "%d		Surface_Number\n", num_geoms);
     num_geoms++;
     fprintf(dump_file, "%s		Name\n", (char*) getName());

     fprintf(dump_file, "2			U_Render\n");
     fprintf(dump_file, "2			W_Render\n");
     fprintf(dump_file, "%d		Num_xsecs\n",pnt_xsec.dimen_1());
     fprintf(dump_file, "%d		Num_pnts\n", num_pnts);

     for (  i = 0 ; i < pnt_xsec.dimen_1() ; i++ ) {
        
        for (  j = 3*quart ; j < pnt_xsec.dimen_2() ; j++ ) {
            
           temp = pnt_xsec(i,j).transform(model_mat);

           fprintf(dump_file, "%11.6f  %11.6f  %11.6f\n", temp.x(), temp.y(),temp.z());
        } 

        for ( j = 1 ; j < quart+1 ; j++ ) {
            
           temp = pnt_xsec(i,j).transform(model_mat);

           fprintf(dump_file, "%11.6f  %11.6f  %11.6f\n", temp.x(), temp.y(),temp.z());

        }
     }
  }

  /* body lies off symmetry plane, split body into left and right parts */
  else { 

     fprintf(dump_file, "\n");
     fprintf(dump_file, "%d		Surface_Number\n", num_geoms);
     num_geoms++;
     fprintf(dump_file, "%s		Name\n", (char*) getName());
     num_pnts = pnt_xsec.dimen_2()/2 + 1;
     quart = (pnt_xsec.dimen_2()-1)/4;

     fprintf(dump_file, "2			U_Render\n");
     fprintf(dump_file, "2			W_Render\n");
     fprintf(dump_file, "%d		Num_xsecs\n",pnt_xsec.dimen_1());
     fprintf(dump_file, "%d		Num_pnts\n", num_pnts);

     for (  i = 0 ; i < pnt_xsec.dimen_1() ; i++ ) {
        
        for ( j = 0 ; j <= 2*quart ; j++ ) {
            
           temp = pnt_xsec(i,j).transform(model_mat);

           fprintf(dump_file, "%11.6f  %11.6f  %11.6f\n", temp.x(), temp.y(),temp.z());
        }
     }

     fprintf(dump_file, "\n");
     fprintf(dump_file, "%d		Surface_Number\n", num_geoms);
     num_geoms++;
     fprintf(dump_file, "%s		Name\n", (char*) getName());

     fprintf(dump_file, "2			U_Render\n");
     fprintf(dump_file, "2			W_Render\n");
     fprintf(dump_file, "%d		Num_xsecs\n",pnt_xsec.dimen_1());
     fprintf(dump_file, "%d		Num_pnts\n", num_pnts);

     for (  i = 0 ; i < pnt_xsec.dimen_1() ; i++ ) {
        
        for ( j = 2*quart ; j < pnt_xsec.dimen_2() ; j++ ) {
            
           temp = pnt_xsec(i,j).transform(model_mat);

           fprintf(dump_file, "%11.6f  %11.6f  %11.6f\n", temp.x(), temp.y(),temp.z());

        }
     }
  }

  /* check for open nose section */
  dist = 0.;

  for ( j = 0 ; j < pnt_xsec.dimen_2() - 1 ; j++ ) {

    test1 = pnt_xsec(0,j  ).transform(model_mat);
    test2 = pnt_xsec(0,j+1).transform(model_mat);

    dist += sqrt( pow(test2.x() - test1.x(),2.)
                + pow(test2.y() - test1.y(),2.)
                + pow(test2.z() - test1.z(),2.) );

  }

  if ( dist >= eps ) {

     test1 = pnt_xsec(0,0).transform(model_mat);
     test2 = pnt_xsec(pnt_xsec.dimen_1()-1,pnt_xsec.dimen_2()-1).transform(model_mat);

     /* body lies on symmetry plane, split body into top and bottom parts */
     /* OR body lies off symmetry plane, split body into top and bottom parts */

     if ( ( test1.y() <= 0. && test2.y() <= 0. && (strstr(getName(),"split_tb") != NULL) ) ||
          ( test1.y() >  0. && test2.y() >  0. && (strstr(getName(),"split_tb") != NULL) ) ) {

        fprintf(dump_file, "\n");
        fprintf(dump_file, "%d		Surface_Number\n", num_geoms);
        num_geoms++;
        fprintf(dump_file, "%s_Nose_Face_TB	Name\n", (char*) getName());

        fprintf(dump_file, "2			U_Render\n");
        fprintf(dump_file, "2			W_Render\n");
        fprintf(dump_file, "%d		Num_xsecs\n",2);
        fprintf(dump_file, "%d		Num_pnts\n", pnt_xsec.dimen_2()/2 + 1);

        quart = (pnt_xsec.dimen_2()-1)/4;
	
        for (  j = quart ; j <= 3*quart ; j++ ) {

           temp = pnt_xsec(0,j).transform(model_mat);

           fprintf(dump_file, "%11.6f  %11.6f  %11.6f\n", temp.x(), temp.y(),temp.z());

        } 

//        printf("Beep! \n");

        for ( j = quart ; j >= 0 ; j-- ) {
            
           temp = pnt_xsec(0,j).transform(model_mat);

           fprintf(dump_file, "%11.6f  %11.6f  %11.6f\n", temp.x(), temp.y(),temp.z());

        } 

        for ( j = pnt_xsec.dimen_2()-2 ; j >= 3*quart ; j-- ) {
            
           temp = pnt_xsec(0,j).transform(model_mat);

           fprintf(dump_file, "%11.6f  %11.6f  %11.6f\n", temp.x(), temp.y(),temp.z());

        } 

     }

     /* body on symmetry plane, so write out left and right halfs */
     /* OR body off symmetry plane, so write out left and right halfs */

     else {

        fprintf(dump_file, "\n");
        fprintf(dump_file, "%d		Surface_Number\n", num_geoms);
        num_geoms++;
        fprintf(dump_file, "%s_Nose_Face_LR	Name\n", (char*) getName());

        fprintf(dump_file, "2			U_Render\n");
        fprintf(dump_file, "2			W_Render\n");
        fprintf(dump_file, "%d		Num_xsecs\n",2);
        fprintf(dump_file, "%d		Num_pnts\n", pnt_xsec.dimen_2()/2 + 1);

        quart = (pnt_xsec.dimen_2()-1)/4;

        for ( j = 0 ; j <= 2*quart ; j++ ) {

           temp = pnt_xsec(0,j).transform(model_mat);

           fprintf(dump_file, "%11.6f  %11.6f  %11.6f\n", temp.x(), temp.y(),temp.z());

        } 

        for ( j = pnt_xsec.dimen_2()-1 ; j >= 2*quart ; j-- ) {

           temp = pnt_xsec(0,j).transform(model_mat);

           fprintf(dump_file, "%11.6f  %11.6f  %11.6f\n", temp.x(), temp.y(),temp.z());
        } 

     }

  }

  /* check for open aft section */

  dist = 0.;

  for ( j = 0 ; j < pnt_xsec.dimen_2() - 1 ; j++ ) {

    test1 = pnt_xsec(pnt_xsec.dimen_1()-1,j  ).transform(model_mat);
    test2 = pnt_xsec(pnt_xsec.dimen_1()-1,j+1).transform(model_mat);

    dist += sqrt( pow(test2.x() - test1.x(),2.)
                + pow(test2.y() - test1.y(),2.)
                + pow(test2.z() - test1.z(),2.) );

  }

  if ( dist >= eps ) {

     test1 = pnt_xsec(0,0).transform(model_mat);
     test2 = pnt_xsec(pnt_xsec.dimen_1()-1,pnt_xsec.dimen_2()-1).transform(model_mat);

     /* body lies on symmetry plane, split body into top and bottom parts */
     /* OR body lies off symmetry plane, split body into top and bottom parts */

     if ( ( test1.y() <= 0. && test2.y() <= 0. && (strstr(getName(),"split_tb") != NULL) ) ||
          ( test1.y() >  0. && test2.y() >  0. && (strstr(getName(),"split_tb") != NULL) ) ) {


        fprintf(dump_file, "\n");
        fprintf(dump_file, "%d		Surface_Number\n", num_geoms);
        num_geoms++;
        fprintf(dump_file, "%s_Aft_Face_TR	Name\n", (char*) getName());

        fprintf(dump_file, "2			U_Render\n");
        fprintf(dump_file, "2			W_Render\n");
        fprintf(dump_file, "%d		Num_xsecs\n",2);
        fprintf(dump_file, "%d		Num_pnts\n", pnt_xsec.dimen_2()/2 + 1);

        quart = (pnt_xsec.dimen_2()-1)/4;

        for (  j = quart ; j < 3*quart+1 ; j++ ) {

           temp = pnt_xsec(pnt_xsec.dimen_1()-1,j).transform(model_mat);

            fprintf(dump_file, "%11.6f  %11.6f  %11.6f\n", temp.x(), temp.y(),temp.z());

        } 
            
        for ( j = quart ; j >= 1 ; j-- ) {

           temp = pnt_xsec(pnt_xsec.dimen_1()-1,j).transform(model_mat);

           fprintf(dump_file, "%11.6f  %11.6f  %11.6f\n", temp.x(), temp.y(),temp.z());

        } 
            
        for (  j = pnt_xsec.dimen_2()-1 ; j >= 3*quart ; j-- ) {

           temp = pnt_xsec(pnt_xsec.dimen_1()-1,j).transform(model_mat);

           fprintf(dump_file, "%11.6f  %11.6f  %11.6f\n", temp.x(), temp.y(),temp.z());

        } 

     }

     /* body on symmetry plane, so write out left and right halfs */
     /* OR body off symmetry plane, so write out left and right halfs */

     else {

        fprintf(dump_file, "\n");
        fprintf(dump_file, "%d		Surface_Number\n", num_geoms);
        num_geoms++;
        fprintf(dump_file, "%s_Aft_Face_LR	Name\n", (char*) getName());

        fprintf(dump_file, "2			U_Render\n");
        fprintf(dump_file, "2			W_Render\n");
        fprintf(dump_file, "%d		Num_xsecs\n",2);
        fprintf(dump_file, "%d		Num_pnts\n", pnt_xsec.dimen_2()/2 + 1);

        quart = (pnt_xsec.dimen_2()-1)/4;

        for ( j = 0 ; j < 2*quart+1 ; j++ ) {

           temp = pnt_xsec(pnt_xsec.dimen_1()-1,j).transform(model_mat);

           fprintf(dump_file, "%11.6f  %11.6f  %11.6f\n", temp.x(), temp.y(),temp.z());

        } 

        for ( j = pnt_xsec.dimen_2()-1 ; j >= 2*quart ; j-- ) {

           temp = pnt_xsec(pnt_xsec.dimen_1()-1,j).transform(model_mat);

           fprintf(dump_file, "%11.6f  %11.6f  %11.6f\n", temp.x(), temp.y(),temp.z());
        } 

     }

  }

  return(num_geoms);

}  

//==== Write Out Felisa Surfaces To File ====//
void Fuse_geom::write_bco_info(FILE* file_ptr, int& surf_cnt, int& dum)
{
  int i, j;

  //==== Load Oml into pnt_xsec array ====//
  int num_xsecs = oml_surf.get_num_xsecs();
  int num_pnts  = oml_surf.get_num_pnts();
  pnt_xsec.init( num_xsecs, num_pnts);
  for ( i = 0 ; i < num_xsecs ; i++ )
    for ( j = 0 ; j < num_pnts ; j++ )
      pnt_xsec( i, j ) = oml_surf.get_pnt( i, j );


  //==== Write Outer ====//
  write_bco_info_single_surf(file_ptr, surf_cnt, dum);

  //==== Check for Inner ====//
  if ( imlFlag )
  {
     //==== Load Iml into pnt_xsec array ====//
    int num_xsecs = iml_surf.get_num_xsecs();
    int num_pnts  = iml_surf.get_num_pnts();
    pnt_xsec.init( num_xsecs, num_pnts);
    for ( i = 0 ; i < num_xsecs ; i++ )
      for ( j = 0 ; j < num_pnts ; j++ )
        pnt_xsec( i, j ) = iml_surf.get_pnt( i, j );

    //==== Dump Inner ====//
    write_bco_info_single_surf(file_ptr, surf_cnt, dum);

  }
}

//==== Write Out Felisa Surfaces To File ====//
void Fuse_geom::write_bco_info_single_surf(FILE* file_ptr, int& surf_cnt, int&)
{

  vec3d test1, test2; 
  double dist, eps = 0.1f;
  int   j;

  test1 = pnt_xsec(0,0).transform(model_mat);
  test2 = pnt_xsec(pnt_xsec.dimen_1()-1,pnt_xsec.dimen_2()-1).transform(model_mat);

  if ( test1.y() <= 0. && test2.y() <= 0. && (strstr(getName(),"split_tb") == NULL) ) {

     fprintf( file_ptr, "%d        0      0     0.0     0.0     0.0     0.0   %s 1\n", surf_cnt, (char*)getName() );
     surf_cnt++;
  }
  else {

     fprintf( file_ptr, "%d        0      0     0.0     0.0     0.0     0.0   %s 1\n", surf_cnt, (char*)getName() );
     surf_cnt++;
     fprintf( file_ptr, "%d        0      0     0.0     0.0     0.0     0.0   %s 2\n", surf_cnt, (char*)getName() );
     surf_cnt++;
  }

  /* check for open nose section */
  dist = 0.;

  for ( j = 0 ; j < pnt_xsec.dimen_2() - 1 ; j++ ) {

    test1 = pnt_xsec(0,j  ).transform(model_mat);
    test2 = pnt_xsec(0,j+1).transform(model_mat);

    dist += sqrt( pow(test2.x() - test1.x(),2.)
                + pow(test2.y() - test1.y(),2.)
                + pow(test2.z() - test1.z(),2.) );

  }

  if ( dist >= eps ) {

     fprintf( file_ptr, "%d        1      0     0.4     0.0     0.0     0.0   %s_Nose_Face 1\n", surf_cnt, (char*)getName() );
     surf_cnt++;

  }

  /* check for open aft section */
  dist = 0.;

  for ( j = 0 ; j < pnt_xsec.dimen_2() - 1 ; j++ ) {

    test1 = pnt_xsec(pnt_xsec.dimen_1()-1,j  ).transform(model_mat);
    test2 = pnt_xsec(pnt_xsec.dimen_1()-1,j+1).transform(model_mat);

    dist += sqrt( pow(test2.x() - test1.x(),2.)
                + pow(test2.y() - test1.y(),2.)
                + pow(test2.z() - test1.z(),2.) );
  }

  if ( dist >= eps ) {

     fprintf( file_ptr, "%d        1      0    -1.0     0.0     0.0     0.0   %s_Aft_Face 1\n", surf_cnt, (char*)getName() );
     surf_cnt++;
  }

}
       
//==== Return Number Of FELISA Background Lines ====//
int Fuse_geom::get_num_felisa_line_sources()
{
  //==== Load Oml into pnt_xsec array ====//
  int num_xsecs = oml_surf.get_num_xsecs();
  int num_pnts  = oml_surf.get_num_pnts();
  pnt_xsec.init( num_xsecs, num_pnts);
  for ( int i = 0 ; i < num_xsecs ; i++ )
    for ( int j = 0 ; j < num_pnts ; j++ )
      pnt_xsec( i, j ) = oml_surf.get_pnt( i, j );

  if ( imlFlag )
    return ( 4*( pnt_xsec.dimen_1()-1 ) );

  return( 2*( pnt_xsec.dimen_1()-1 ) );
}

//==== Write Out Felisa Background Lines File ====//
void Fuse_geom::write_felisa_line_sources(FILE* dump_file)
{
  int i, j;

  //==== Load Oml into pnt_xsec array ====//
  int num_xsecs = oml_surf.get_num_xsecs();
  int num_pnts  = oml_surf.get_num_pnts();
  pnt_xsec.init( num_xsecs, num_pnts);
  for ( i = 0 ; i < num_xsecs ; i++ )
    for ( j = 0 ; j < num_pnts ; j++ )
      pnt_xsec( i, j ) = oml_surf.get_pnt( i, j );

  //==== Write Outer ====//
  write_felisa_line_sources_single_surf(dump_file);

  //==== Check for Inner ====//
  if ( imlFlag )
  {

     //==== Load Iml into pnt_xsec array ====//
    int num_xsecs = iml_surf.get_num_xsecs();
    int num_pnts  = iml_surf.get_num_pnts();
    pnt_xsec.init( num_xsecs, num_pnts);
    for ( i = 0 ; i < num_xsecs ; i++ )
      for ( j = 0 ; j < num_pnts ; j++ )
        pnt_xsec( i, j ) = iml_surf.get_pnt( i, j );


    //==== Dump Inner ====//
    write_felisa_line_sources_single_surf(dump_file);

  }
}

//==== Write Out Felisa Background Lines File ====//
void Fuse_geom::write_felisa_line_sources_single_surf(FILE* dump_file)
{

  int i, j;
  vec3d temp;
  vec3d zero(0.0, 0.0, 0.0);

  //==== Find Xsec Center Pnts ====/
  dyn_array<vec3d> cent_pnts;
  cent_pnts.init(pnt_xsec.dimen_1());
  for (  i = 0 ; i < pnt_xsec.dimen_1() ; i++ )
    {
      cent_pnts[i] = zero;
      for (  j = 0 ; j < pnt_xsec.dimen_2() ; j++ )
        {
          cent_pnts[i] = cent_pnts[i] + pnt_xsec(i, j);
        }
      cent_pnts[i] = cent_pnts[i]/(double) pnt_xsec.dimen_2();
    } 

  //==== Find Max Dist From Center Pnts ====/
  dyn_array<double> max_dists;
  max_dists.init(pnt_xsec.dimen_1());
  for (  i = 0 ; i < pnt_xsec.dimen_1() ; i++ )
    {
      max_dists[i] = 0.0;
      for (  j = 0 ; j < pnt_xsec.dimen_2() ; j++ )
        {
          double tmp_dist = dist(cent_pnts[i], pnt_xsec(i, j));
          if ( tmp_dist >  max_dists[i] )
            max_dists[i] = tmp_dist;
        }
    } 

  //==== Set First And Last Max Dists ====/
  int last_index = pnt_xsec.dimen_1()-1;
  max_dists[0] = max_dists[2];
  max_dists[1] = max_dists[2];

  max_dists[last_index]   = max_dists[last_index-2];
  max_dists[last_index-1] = max_dists[last_index-2];


  //==== Limit minimum spacing ====/
  for (  i = 0 ; i < last_index ; i++ ) {

     max_dists[i] = MAX(length()/75.,max_dists[i]);

  }

  //==== Dump Lines ====/
  for (  i = 0 ; i < last_index ; i++ )
    {
      fprintf(dump_file, " %s - Fuse Line \n", (char*) getName());

      temp = cent_pnts[i].transform(model_mat);
      
  
      fprintf(dump_file, "    %f    %f    %f   %f    %f    %f  \n",
         temp.x(), temp.y(), temp.z(), max_dists[i]*0.25, 1.125*max_dists[i], 3.0*max_dists[i]);


      temp = cent_pnts[i+1].transform(model_mat);
      fprintf(dump_file, "    %f    %f    %f    %f    %f    %f  \n",
         temp.x(), temp.y(), temp.z(), max_dists[i+1]*0.25, 1.125*max_dists[i+1], 3.0*max_dists[i+1]);
    }

  //==== Dump Lines ====/
  for (  i = 0 ; i < last_index ; i++ )
    {
      fprintf(dump_file, " %s - Far Fuse Line \n", (char*) getName());

      temp = cent_pnts[i].transform(model_mat);
      
  
      fprintf(dump_file, "    %f    %f    %f   %f    %f    %f  \n",
         temp.x(), temp.y(), temp.z(), 4.*max_dists[i], 12.0*max_dists[i], 36.0*max_dists[i]);


      temp = cent_pnts[i+1].transform(model_mat);
      fprintf(dump_file, "    %f    %f    %f    %f    %f    %f  \n",
         temp.x(), temp.y(), temp.z(), 4.*max_dists[i+1], 12.0*max_dists[i+1], 36.0*max_dists[i+1]);
    }
            
}

//==== Write Rhino File ====//
void Fuse_geom::write_rhino_file(ON_BinaryFile* archive, ON_3dmObjectAttributes* attributes)
{
	oml_surf.clear_xsec_tan_flags();

	oml_surf.set_xsec_tan_flag( 0, Bezier_curve::ZERO );
    int num_xsecs  = oml_surf.get_num_xsecs();
	oml_surf.set_xsec_tan_flag( num_xsecs-1, Bezier_curve::ZERO );


  oml_surf.write_rhino_file( sym_code, model_mat, reflect_mat, archive, attributes );
}

int  Fuse_geom::get_num_bezier_comps()
{
	if ( sym_code == NO_SYM )
		return 1;
	else 
		return 2;
}

void Fuse_geom::write_bezier_file( int id, FILE* file_id )
{
	oml_surf.clear_xsec_tan_flags();

	oml_surf.set_xsec_tan_flag( 0, Bezier_curve::PREDICT );
    int num_xsecs  = oml_surf.get_num_xsecs();
	oml_surf.set_xsec_tan_flag( num_xsecs-1, Bezier_curve::PREDICT );

	//==== Look For Sharp Tangents ====//
	vector <int> uSharpVec;
	for ( int i = 1 ; i < oml_surf.get_num_xsecs()-1 ; i++ )
	{
		vec3d p0 = oml_surf.get_pnt( i-1, 0 );
		vec3d p1 = oml_surf.get_pnt( i,   0 );
		vec3d p2 = oml_surf.get_pnt( i+1, 0 );

		vec3d p01 = p0 - p1;
		vec3d p21 = p2 - p1;

		double ang = 180.0*DEG_2_RAD;

		if ( p01.mag() > 0.0000001 && p21.mag() > 0.0000001 )
			ang = angle( p01, p21 );

		if ( ang < 100.0*DEG_2_RAD )
			uSharpVec.push_back( i );
	}


	vector <int> u_split;
	u_split.push_back( 0 );

	for ( int i = 0 ; i < (int)uSharpVec.size() ; i++ )
		u_split.push_back( 3*uSharpVec[i] );

	u_split.push_back( 3*(oml_surf.get_num_xsecs() - 1)  );
	vector <int> w_split;
	w_split.push_back( 0 );
	w_split.push_back( 3*(oml_surf.get_num_pnts()/4)  );
	w_split.push_back( 3*(oml_surf.get_num_pnts()/2)  );
	w_split.push_back( 3*3*(oml_surf.get_num_pnts()/4)  );
	w_split.push_back( 3*(oml_surf.get_num_pnts() - 1)  );

	oml_surf.write_bezier_file( file_id, sym_code, model_mat, reflect_mat, u_split, w_split  );
}


//==== Convert To Tri Mesh ====//
vector< TMesh* > Fuse_geom:: createTMeshVec()
{
	vector< TMesh* > tMeshVec;

	TMesh* tmPtr = oml_surf.createTMesh(model_mat);
	tMeshVec.push_back( tmPtr );

	if ( sym_code != NO_SYM )
	{
		tmPtr = oml_surf.createReflTMesh(sym_code, reflect_mat);
		tMeshVec.push_back( tmPtr );
	}
	return tMeshVec;
}

void Fuse_geom::debug_print()
{
	int i;
	printf("\n\n");
	printf("Fuse_geom:: %d \n", (long)this);

	printf("Fuse_geom: curr_xsec_num %d \n", curr_xsec_num);
	printf("Fuse_geom: curr_xsec_num %d \n", curr_xsec_num);

	for ( i = 0 ; i < (int)xsecVec.size() ; i++ )
	{
		printf("  Fuse_geom: xsecVec[%d] = %d \n", i, (long)xsecVec[i] );
	}
	
	printf("Fuse_geom: imlFlag %d \n", imlFlag);
	printf("Fuse_geom: imlColor %f %f %f \n", imlColor.x(), imlColor.y(), imlColor.z());

	printf("Fuse_geom: oml_surf num_pnts %d \n", oml_surf.get_num_pnts() );
	printf("Fuse_geom: oml_surf num_secs %d \n", oml_surf.get_num_xsecs() );

	printf("Fuse_geom: iml_surf num_pnts %d \n", iml_surf.get_num_pnts() );
	printf("Fuse_geom: iml_surf num_secs %d \n", iml_surf.get_num_xsecs() );

	
	printf("Fuse_geom: spine num_pnts %d \n", spine.get_num_pnts() );
	printf("Fuse_geom: spine length %f \n", spine.get_length() );

	printf("Fuse_geom: length %f \n", length() );
	printf("Fuse_geom: camber %f \n", camber() );
	printf("Fuse_geom: camber_loc %f \n", camber_loc() );
	printf("Fuse_geom: aft_offset %f \n", aft_offset() );
	printf("Fuse_geom: nose_angle %f \n", nose_angle() );
	printf("Fuse_geom: nose_str %f \n", nose_str() );
	printf("Fuse_geom: nose_rho %f \n", nose_rho() );
	printf("Fuse_geom: aft_rho %f \n", aft_rho() ); 
}

vec3d Fuse_geom::getVertex3d(int surfid, double x, double p, int r)			
{ 
	switch (surfid)
	{
		case IML_SURF:
			return iml_surf.get_vertex(x, p, r);
		case OML_SURF:
			return oml_surf.get_vertex(x, p, r);
		default:
			return vec3d(0,0,0);
	}
}

void Fuse_geom::getVertexVec(vector< VertexID > *vertVec)
{ 
	buildVertexVec(&iml_surf, IML_SURF, vertVec); 
	buildVertexVec(&oml_surf, OML_SURF, vertVec); 
}

void Fuse_geom::updateNumInter()
{
	if ( curr_xsec_num > 0 )
	{
		int n = xsecVec[curr_xsec_num]->getNumSectInterp1();
		xsecVec[curr_xsec_num-1]->setNumSectInterp2(n);
	}
	if ( curr_xsec_num < (int)xsecVec.size()-1 )
	{
		int n = xsecVec[curr_xsec_num]->getNumSectInterp2();
		xsecVec[curr_xsec_num+1]->setNumSectInterp1(n);
	}
}

void Fuse_geom::incNumInterpAll( int off )
{
	int i;
	int minNum = xsecVec[0]->getNumSectInterp2();
	for ( i = 0 ; i < (int)xsecVec.size()-1 ; i++ )
	{
		if ( xsecVec[i]->getNumSectInterp2() < minNum )
			minNum = xsecVec[i]->getNumSectInterp2();
	}
	if ( off < 0 && abs(off) > minNum )
		return;

	for ( i = 0 ; i < (int)xsecVec.size()-1 ; i++ )
	{
		int num = xsecVec[i]->getNumSectInterp2();
		xsecVec[i]->setNumSectInterp2( num+off );
		xsecVec[i+1]->setNumSectInterp1( num+off );
	}
}


void Fuse_geom::drawControlPoints()
{
	int i;
	for ( i = 0 ; i < (int)cPntVec.size() ; i++ )
	{
		if ( activeControlPntID == i )
		{
			if ( Fl::event_state(FL_SHIFT) )
				glColor3ub( 255, 255, 0 );
			else
				glColor3ub( 255, 0, 0 );

			glBegin( GL_LINES );

			vec3d pnt = cPntVec[i].pnt3d;
			glVertex3dv( pnt.data() );
			if ( cPntVec[i].pntID == ControlPnt::TOP )
				pnt.offset_z( length.get()/20.0 );
			else if ( cPntVec[i].pntID == ControlPnt::BOTTOM )
				pnt.offset_z( -length.get()/20.0 );
			else if ( cPntVec[i].pntID == ControlPnt::SIDE )
				pnt.offset_y( length.get()/20.0 );

			glVertex3dv( pnt.data() );

			glEnd();


			glPointSize( 16.0 );
		}
		else
		{
			glColor3ub( 255, 0, 0 );
			glPointSize( 8.0 );
		}
		glBegin( GL_POINTS );
		cPntVec[i].pnt2d = projectPoint( cPntVec[i].pnt3d, 0 );
		glVertex3dv( cPntVec[i].pnt3d.data() );
		glEnd();
	}

}


void Fuse_geom::draw2D(vec2d & cursor)
{
	if ( !redFlag )			//Only On Edit
		return;


	int i;
	int nearCPnt = -1;
	double nearDist = 0.05;

	//==== Find Distance to Control Points ====//
	for ( i = 0 ; i < (int)cPntVec.size() ; i++ )
	{
		double dist = dist_squared( cursor, cPntVec[i].pnt2d );
		if ( dist < nearDist )
		{
			nearDist = dist;
			nearCPnt = i;
		}
	}

	//==== Change Close Point if NOT Holding Shift ====//
	static ControlPnt* currCPnt;
	if ( nearCPnt >= 0 && !Fl::event_state(FL_SHIFT) )
	{
		currCPnt = &cPntVec[nearCPnt];
		activeControlPntID = nearCPnt;
	}
	else if ( nearCPnt < 0 )
	{
		activeControlPntID = -1;
		currCPnt = 0;
	}

	//==== If Shift Key Then Move Control Point ====//
	if ( currCPnt && Fl::event_state(FL_SHIFT))
	{
		if ( curr_xsec_num != currCPnt->xSecID )
		{
			set_curr_xsec_num(currCPnt->xSecID);
			airPtr->getScreenMgr()->getScreen(this)->show(this);
		}

		vec2d currPnt2 = currCPnt->pnt2d;
		vec3d offPnt3 = currCPnt->pnt3d;

		//==== Offset Height Point ====//
		if ( currCPnt->pntID == ControlPnt::TOP )
		{
			offPnt3.offset_z(1.0);
			vec2d offPnt2 = projectPoint( offPnt3, 0 );

			double moveZ = proj_pnt_on_line_u(currPnt2, offPnt2, cursor);
			cPntVec[curr_xsec_num].pnt3d.offset_z( moveZ );				
			cPntVec[curr_xsec_num].pnt2d = projectPoint( offPnt3, 0 );

			double h = xsecVec[curr_xsec_num]->get_height()->get();
			xsecVec[curr_xsec_num]->get_height()->set((h+moveZ));

			if ( xsecVec[curr_xsec_num]->get_type() == EDIT_CRV )
			{
				EditCurve* ec =  xsecVec[curr_xsec_num]->getEditCrv();
				double mh = ec->getMaxHeight()->get();
				double sy = ec->getScaleY()->get();
				ec->getScaleY()->set( sy + moveZ/mh );
				ec->parm_changed(ec->getScaleY());
			}

			double zoff = xsecVec[curr_xsec_num]->get_z_offset()->get();
			xsecVec[curr_xsec_num]->get_z_offset()->set((zoff+moveZ/2.0));

			xsecVec[curr_xsec_num]->parm_changed( xsecVec[curr_xsec_num]->get_height() );
			xsecVec[curr_xsec_num]->parm_changed( xsecVec[curr_xsec_num]->get_z_offset() );
		}
		else if ( currCPnt->pntID == ControlPnt::BOTTOM )
		{
			offPnt3.offset_z(1.0);
			vec2d offPnt2 = projectPoint( offPnt3, 0 );

			double moveZ = proj_pnt_on_line_u(currPnt2, offPnt2, cursor);
			cPntVec[curr_xsec_num].pnt3d.offset_z( -moveZ );				
			cPntVec[curr_xsec_num].pnt2d = projectPoint( offPnt3, 0 );

			double h = xsecVec[curr_xsec_num]->get_height()->get();
			xsecVec[curr_xsec_num]->get_height()->set((h-moveZ));

			if ( xsecVec[curr_xsec_num]->get_type() == EDIT_CRV )
			{
				EditCurve* ec =  xsecVec[curr_xsec_num]->getEditCrv();
				double mh = ec->getMaxHeight()->get();
				double sy = ec->getScaleY()->get();
				ec->getScaleY()->set( sy - moveZ/mh );
				ec->parm_changed(ec->getScaleY());
			}


			double zoff = xsecVec[curr_xsec_num]->get_z_offset()->get();
			xsecVec[curr_xsec_num]->get_z_offset()->set((zoff+moveZ/2.0));

			xsecVec[curr_xsec_num]->parm_changed( xsecVec[curr_xsec_num]->get_height() );
			xsecVec[curr_xsec_num]->parm_changed( xsecVec[curr_xsec_num]->get_z_offset() );
		}
		else if ( currCPnt->pntID == ControlPnt::SIDE )
		{
			offPnt3.offset_y(1.0);
			vec2d offPnt2 = projectPoint( offPnt3, 0 );

			double moveY = proj_pnt_on_line_u(currPnt2, offPnt2, cursor);
			cPntVec[curr_xsec_num].pnt3d.offset_y( moveY );				
			cPntVec[curr_xsec_num].pnt2d = projectPoint( offPnt3, 0 );

			double w = xsecVec[curr_xsec_num]->get_width()->get();
			xsecVec[curr_xsec_num]->get_width()->set((w+moveY));

			if ( xsecVec[curr_xsec_num]->get_type() == EDIT_CRV )
			{
				EditCurve* ec =  xsecVec[curr_xsec_num]->getEditCrv();
				double mw = ec->getMaxWidth()->get();
				double sx = ec->getScaleX()->get();
				ec->getScaleX()->set( sx + moveY/mw );
				ec->parm_changed(ec->getScaleX());
			}


			xsecVec[curr_xsec_num]->parm_changed( xsecVec[curr_xsec_num]->get_width() );
		}

	}

}

//==== Aero Ref Parameters ====//
double Fuse_geom::getRefArea()
{
	if ( autoRefAreaFlag )
	{
		refArea = get_area();
	}
	return refArea;
}
	
double Fuse_geom::getRefSpan()
{
	if ( autoRefSpanFlag )
	{
		refSpan = get_span();
	}
	return refSpan;
}

double Fuse_geom::getRefCbar()
{
	if ( autoRefCbarFlag )
	{
		refCbar = length();
	}
	return refCbar;
}

vec3d Fuse_geom::getAeroCenter()
{
	if ( autoAeroCenterFlag )
	{
		aeroCenter.set_x( length()*0.25 + get_tran_x() );
		aeroCenter.set_y( get_tran_y() );
		aeroCenter.set_z( get_tran_z() );
	}
	return aeroCenter;
}

void Fuse_geom::LoadDragFactors( DragFactors& drag_factors )
{
	double max_xsec_area = 0.000000000001;
	for ( int i = 0 ; i < (int)xsecVec.size() ; i++ )
	{
		double a = xsecVec[i]->computeArea();
		if ( a > max_xsec_area )
			max_xsec_area = a;
	}

	double dia = 2.0*sqrt( max_xsec_area/PI );

	drag_factors.m_Length = length();
	drag_factors.m_MaxXSecArea = max_xsec_area;
	drag_factors.m_LengthToDia = length()/dia;
}

void Fuse_geom::GetInteriorPnts( vector< vec3d > & pVec )
{
	vec3d p = spine.comp_pnt( 0.5 );
	vec3d tp = p.transform( model_mat );
	pVec.push_back( tp ); 

	if (sym_code != NO_SYM)
	{
		tp = (p * sym_vec).transform(reflect_mat);
		pVec.push_back( tp );
    }
}


