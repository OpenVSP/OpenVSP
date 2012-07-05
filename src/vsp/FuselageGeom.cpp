//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//

//******************************************************************************
//    
//		Fuselage Geometry Class
//  
//		J.R. Gloudemans - 8/18/2009
//
//******************************************************************************
#include <FL/Fl.H>

#include "FuselageGeom.h"
#include "defines.h"
#include "geom.h"
#include "aircraft.h"
#include "materialMgr.h"
#include "bezier_curve.h"
#include "bezier_surf.h"
#include "screenMgr.h"
#include "util.cpp"
#include "parmLinkMgr.h"

//==== Constructor =====//
FuselageGeom::FuselageGeom(Aircraft* aptr) : Geom(aptr)
{
  int i;

  redFlag = 0;
  activeControlPntID = 0;
    
  saved_xsec.set_fuse_ptr(this);

  numXsecs.deactivate();

  type = FUSELAGE_GEOM_TYPE;
  type_str = Stringc("fuselage2");

  char name[255];
  sprintf( name, "Fuselage_%d", geomCnt ); 
  geomCnt++;
  setName( Stringc(name) );

  spine.init(2);

  length.initialize(this, UPD_FUSE_BODY, "Length", 30.0 );
  length.set_lower_upper(0.00001, 1000000.0);
  length.set_script("fuselage_shape length", 0);

  //==== Load Initial Default Cross Sections ====//
  int num_init_xsecs = 5;
  for ( i = 0 ; i < num_init_xsecs ; i++)
  {
	FuselageXSec* xs = new FuselageXSec( this );
	xsecVec.push_back( xs );
	xs->set_loc_on_spine( (float)i/(float)(num_init_xsecs-1) );

	if ( i == 0 )
	{
		xs->set_top_sym( 1 );
		xs->getTopTanAng()->set( -45.0 );
		xs->getBotTanAng()->set( -45.0 );
		xs->getLeftTanAng()->set( -45.0 );
		xs->getRightTanAng()->set( -45.0 );
		xs->set_type(XSEC_POINT);
		xs->gen_parms();
	}
	if ( i == num_init_xsecs-1)
	{
		xs->set_top_sym( 1 );
		xs->getTopTanAng()->set( 45.0 );
		xs->getBotTanAng()->set( 45.0 );
		xs->getLeftTanAng()->set( 45.0 );
		xs->getRightTanAng()->set( 45.0 );
		xs->set_type(XSEC_POINT);
		xs->gen_parms();
	}
	xs->generate();
  }

  curr_xsec_num = 1;

  int defNumNoseAft = 5;
  xsecVec[0]->setNumSectInterp2(defNumNoseAft);
  xsecVec[1]->setNumSectInterp1(defNumNoseAft);
  xsecVec[xsecVec.size()-2]->setNumSectInterp2( defNumNoseAft );
  xsecVec[xsecVec.size()-1]->setNumSectInterp1( defNumNoseAft );

  set_num_pnts( numPnts.iget() );

  center.set_xyz( 0, 0, 0 );
  slice_parms.init(0,0);

  //==== Initialize IML Xsecs ====//
  generate();
  surfVec.push_back( &surf );
}

//==== Destructor =====//
FuselageGeom::~FuselageGeom()
{
	for ( int i = 0 ; i < (int)xsecVec.size() ; i++ )
		delete xsecVec[i];

}
void FuselageGeom::LoadLinkableParms( vector< Parm* > & parmVec )
{
	Geom::LoadLinkableParms( parmVec );
	Geom::AddLinkableParm( &length, parmVec, this, "Design" );

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
		Geom::AddLinkableParm( xsecVec[i]->get_y_offset(), parmVec, this, gname );
		Geom::AddLinkableParm( xsecVec[i]->get_z_offset(), parmVec, this, gname );
		Geom::AddLinkableParm( xsecVec[i]->get_location(), parmVec, this, gname );

		Geom::AddLinkableParm( xsecVec[i]->getTopTanAng(), parmVec, this, gname );
		Geom::AddLinkableParm( xsecVec[i]->getTopTanStr1(), parmVec, this, gname );
		Geom::AddLinkableParm( xsecVec[i]->getTopTanStr2(), parmVec, this, gname );
		Geom::AddLinkableParm( xsecVec[i]->getBotTanAng(), parmVec, this, gname );
		Geom::AddLinkableParm( xsecVec[i]->getBotTanStr1(), parmVec, this, gname );
		Geom::AddLinkableParm( xsecVec[i]->getBotTanStr2(), parmVec, this, gname );
		Geom::AddLinkableParm( xsecVec[i]->getLeftTanAng(), parmVec, this, gname );
		Geom::AddLinkableParm( xsecVec[i]->getLeftTanStr1(), parmVec, this, gname );
		Geom::AddLinkableParm( xsecVec[i]->getLeftTanStr2(), parmVec, this, gname );
		Geom::AddLinkableParm( xsecVec[i]->getRightTanAng(), parmVec, this, gname );
		Geom::AddLinkableParm( xsecVec[i]->getRightTanStr1(), parmVec, this, gname );
		Geom::AddLinkableParm( xsecVec[i]->getRightTanStr2(), parmVec, this, gname );
	}
}

void FuselageGeom::RemoveXSecParmReferences( FuselageXSec* xsec )
{
	parmLinkMgrPtr->RemoveParmReferences( xsec->get_height() );
	parmLinkMgrPtr->RemoveParmReferences( xsec->get_width() );
	parmLinkMgrPtr->RemoveParmReferences( xsec->get_max_width_loc() );
	parmLinkMgrPtr->RemoveParmReferences( xsec->get_corner_rad() );
	parmLinkMgrPtr->RemoveParmReferences( xsec->get_top_tan_angle() );
	parmLinkMgrPtr->RemoveParmReferences( xsec->get_bot_tan_angle() );
	parmLinkMgrPtr->RemoveParmReferences( xsec->get_top_str() );
	parmLinkMgrPtr->RemoveParmReferences( xsec->get_upp_str() );
	parmLinkMgrPtr->RemoveParmReferences( xsec->get_bot_str() );
	parmLinkMgrPtr->RemoveParmReferences( xsec->get_low_str() );
	parmLinkMgrPtr->RemoveParmReferences( xsec->get_y_offset() );
	parmLinkMgrPtr->RemoveParmReferences( xsec->get_z_offset() );
	parmLinkMgrPtr->RemoveParmReferences( xsec->get_location() );

	parmLinkMgrPtr->RemoveParmReferences( xsec->getTopTanAng() );
	parmLinkMgrPtr->RemoveParmReferences( xsec->getTopTanStr1() );
	parmLinkMgrPtr->RemoveParmReferences( xsec->getTopTanStr2() );
	parmLinkMgrPtr->RemoveParmReferences( xsec->getBotTanAng() );
	parmLinkMgrPtr->RemoveParmReferences( xsec->getBotTanStr1() );
	parmLinkMgrPtr->RemoveParmReferences( xsec->getBotTanStr2() );
	parmLinkMgrPtr->RemoveParmReferences( xsec->getLeftTanAng() );
	parmLinkMgrPtr->RemoveParmReferences( xsec->getLeftTanStr1() );
	parmLinkMgrPtr->RemoveParmReferences( xsec->getLeftTanStr2() );
	parmLinkMgrPtr->RemoveParmReferences( xsec->getRightTanAng() );
	parmLinkMgrPtr->RemoveParmReferences( xsec->getRightTanStr1() );
	parmLinkMgrPtr->RemoveParmReferences( xsec->getRightTanStr2() );

}

void FuselageGeom::DeleteAllXSecs()
{
    //==== Load & Read Fuse Xsec Parms ====//
	for ( int i = 0 ; i < (int)xsecVec.size() ; i++ )
	{
		RemoveXSecParmReferences( xsecVec[i] );
		delete xsecVec[i];
	}
	xsecVec.clear();
}


void FuselageGeom::AddDefaultSources(double base_len)
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

void FuselageGeom::copy( Geom* fromGeom )
{
	int i;
	Geom::copy( fromGeom );						// Copy Base Stuff

	if ( fromGeom->getType() != FUSELAGE_GEOM_TYPE )
		return;

	FuselageGeom* g = (FuselageGeom*)fromGeom;

	length.set( g->get_length()->get() );

	DeleteAllXSecs();
	//for ( i = 0 ; i < (int)xsecVec.size() ; i++ )
	//{
	//	delete xsecVec[i];
	//}
	//xsecVec.clear();

	int numXsec = g->xsecVec.size();

	for ( i = 0 ; i < numXsec ; i++ )
	{
		FuselageXSec* new_xsec = new FuselageXSec(this);
		new_xsec->gen_parms();
		new_xsec->copy( *(g->xsecVec[i]) ); 
//		*new_xsec = *(g->xsecVec[i]);
		new_xsec->generate();
		new_xsec->set_loc_on_spine( g->xsecVec[i]->get_loc_on_spine() );
		new_xsec->set_fuse_ptr(this);

		xsecVec.push_back( new_xsec );
	}

	curr_xsec_num = g->get_curr_xsec_num();	

	//===== Set Flags =====
	int space_type = xsecVec[0]->get_pnt_space_type();
	set_pnt_space_type( space_type );

	generate();
	compose_model_matrix();

	set_num_pnts( numPnts.iget() );

	parmMgrPtr->RebuildAll();

}

void FuselageGeom::computeCenter()
{
	//==== Set Rotation Center ====//
	center.set_x( origin()*length()*scaleFactor() ); 
}


void FuselageGeom::parm_changed(Parm* p)
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
			generate();
			updateAttach(0);
		}
		break;

		case UPD_EDIT_CRV:
		{
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

void FuselageGeom::acceptScaleFactor()
{
	lastScaleFactor = 1.0;
	scaleFactor.set(1.0);
}

void FuselageGeom::resetScaleFactor()
{
	scaleFactor.set( 1.0 );
    scale();
	lastScaleFactor = 1.0;
		
	//==== Tell Aircraft You Have Changed ====//
	airPtr->geomMod( this );
}

void FuselageGeom::scale()
{
	double current_factor = scaleFactor()*(1.0/lastScaleFactor);

	length.set( length()*current_factor );

	for ( int i = 0 ; i < (int)xsecVec.size() ; i++ )
	{
		Parm* h = xsecVec[i]->get_height();
		Parm* w = xsecVec[i]->get_width();
		Parm* y = xsecVec[i]->get_y_offset();
		Parm* z = xsecVec[i]->get_z_offset();

		h->set( h->get()*current_factor );
		w->set( w->get()*current_factor );
		y->set( y->get()*current_factor );
		z->set( z->get()*current_factor );

		xsecVec[i]->generate();
	}
    generate();
	lastScaleFactor = scaleFactor();
}

vec3d FuselageGeom::getAttachUVPos(double u, double v)
{
	vec3d pos;

	vec3d uvpnt = surf.comp_uv_pnt(u,v);
	pos = uvpnt.transform( model_mat );

	return pos;
}

//==== Update Bounding Box =====//
void FuselageGeom::update_bbox()
{
  vec3d pnt;
  bbox new_box;

  int num_pnts = surf.get_num_pnts();
  int num_xsec = surf.get_num_xsecs();
  int pnt_inc = max( num_pnts/16, 1);
 
  //==== Check at Least 16 Pnts ====//
  for ( int i = 0 ; i < num_xsec ; i++ )
  {
		for ( int j = 0 ; j < num_pnts ; j+=pnt_inc )
		{
		  pnt = surf.get_pnt(i,j);            new_box.update(pnt);
		}
  }

  bnd_box = new_box;
  update_xformed_bbox();			// Load Xform BBox
}

//==== Generate Fuse Component ====//
void FuselageGeom::generate()
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
}

//==== Write Fuse File ====//
void FuselageGeom::write(xmlNodePtr root)
{
  int i;
  xmlAddStringNode( root, "Type", "Fuselage2");

  //==== Write General Parms ====//
  xmlNodePtr gen_node = xmlNewChild( root, NULL, (const xmlChar *)"General_Parms", NULL );
  write_general_parms( gen_node );

  //==== Write Fuse Parms ====//
  xmlNodePtr fuse_node = xmlNewChild( root, NULL, (const xmlChar *)"Fuse_Parms", NULL );

  xmlAddDoubleNode( fuse_node, "Fuse_Length", length() );

  xmlAddIntNode( fuse_node, "Space_Type", xsecVec[0]->get_pnt_space_type() );

  //==== Write Fuse Xsec Parms ====//
  xmlNodePtr csl_node = xmlNewChild( root, NULL, (const xmlChar *)"Cross_Section_List", NULL );

  for ( i = 0 ; i < (int)xsecVec.size() ; i++ )
  {
	xmlNodePtr cs_node = xmlNewChild( csl_node, NULL, (const xmlChar *)"Cross_Section", NULL );
    xsecVec[i]->write( cs_node );
  }
}

//==== Read Fuse File ====//
void FuselageGeom::read(xmlNodePtr root)
{
  int i;
  xmlNodePtr node;

  //===== Read General Parameters =====//
  node = xmlGetNode( root, "General_Parms", 0 );
  if ( node )
  {
    read_general_parms( node );
  }

  int num_xsecs;
  int space_type = PNT_SPACE_FIXED;

  //===== Read Fuse Parameters =====//
  node = xmlGetNode( root, "Fuse_Parms", 0 );
  if ( node )
  {
    length     = xmlFindDouble( node, "Fuse_Length", length() );

    space_type   = xmlFindInt( node, "Space_Type", space_type );
  }

  node = xmlGetNode( root, "Cross_Section_List", 0 );
  if ( node  )
  {
    xmlNodePtr xsec_list_node = node;
    num_xsecs = xmlGetNumNames( xsec_list_node, "Cross_Section" );

    //==== Load & Read Fuse Xsec Parms ====//
	DeleteAllXSecs();
	//for ( i = 0 ; i < (int)xsecVec.size() ; i++ )
	//	delete xsecVec[i];

	//xsecVec.clear();

    for ( i = 0 ; i < num_xsecs ; i++ )
    {
      xmlNodePtr xsec_node = xmlGetNode( xsec_list_node, "Cross_Section", i );
	  FuselageXSec* xs = new FuselageXSec( this );
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
  set_pnt_space_type( space_type );
  generate();

  parmMgrPtr->RebuildAll();

}

//==== Compute Spine =====//
void FuselageGeom::comp_spine()
{
  int i;
  spine.load_pnt(0, vec3d(0.0, 0.0, 0.0));
  spine.load_tan(0, vec3d(0.0, 0.0, 0.0));
  spine.load_pnt(1, vec3d(length(), 0.0, 0.0 ));
  spine.load_tan(1, vec3d(0.0, 0.0, 0.0));

  for ( i = 0 ; i < (int)xsecVec.size() ; i++ )
  {
      double loc = xsecVec[i]->get_loc_on_spine();
      vec3d sp_pnt = spine.comp_pnt_per_length( (float)loc );
      xsecVec[i]->set_pnt_on_spine(sp_pnt);
  } 
}

//==== Interpolate Xsecs That are From Files =====//
void FuselageGeom::interpolate_from_file
			( float sup_fract, FuselageXSec* xsec0, FuselageXSec* xsec1, FuselageXSec* out )
{
  if ( xsec0->get_type() == XSEC_POINT )
  {
    out->set_type( FROM_FILE );
    out->set_file_crv( xsec1->get_file_crv() );
  }
  else if ( xsec0->get_type() == FROM_FILE &&  xsec1->get_type() == FROM_FILE )
  {
    out->set_type( FROM_FILE );
    out->interp_file_crv( sup_fract, xsec0->get_file_crv(), xsec1->get_file_crv() );
  }
}

//==== Interpolate Xsecs That are From Edit Crvs =====//
void FuselageGeom::interpolate_from_edit_crv
			( float sup_fract, FuselageXSec* xsec0, FuselageXSec* xsec1, FuselageXSec* out )
{
  if ( xsec0->get_type() == XSEC_POINT )
  {

    out->setEditCrv( xsec1->getEditCrv() );
	double w = xsec1->getEditCrv()->getMaxWidth()->get();
	double h = xsec1->getEditCrv()->getMaxHeight()->get();
	out->getEditCrv()->scaleWH( w*sup_fract, h*sup_fract ); 
    out->set_type( EDIT_CRV );
	out->getEditCrv()->generate();
  }
  else if ( xsec0->get_type() == EDIT_CRV &&  xsec1->get_type() == EDIT_CRV )
  {
	out->getEditCrv()->blend( sup_fract, xsec0->getEditCrv(),xsec1->getEditCrv() );

    out->set_type( EDIT_CRV );
 	out->getEditCrv()->generate();
 }
}

//==== Cross Section Has Changed Regen Surf =====//
void FuselageGeom::xsec_trigger()
{
  generate();
}

//==== Generate Cross Sections =====//
void FuselageGeom::regenerate()
{
}

//==== Generate Cross Sections =====//
void FuselageGeom::gen_surf()
{
	int i, j, s;
	vec3d p;
  				
	//==== Compute Number of Base Cross Sections ====//
	int num_base_pnts  = xsecVec[0]->get_num_pnts();
	int num_base_xsecs = xsecVec.size();

	//==== Load Stringer Point Vecs ====//
	static vector< vector< vec3d > > stringerVec;
	stringerVec.resize( num_base_pnts );
	for ( i = 0 ; i < (int)stringerVec.size() ; i++ )
		stringerVec[i].resize( num_base_xsecs );

	//==== Build Up Cross Sections to Interpolate ====//
	int xsecCnt = 0;
	for ( i = 0 ; i < (int)xsecVec.size() ; i++ )
	{
		vec3d spinePnt = xsecVec[i]->get_pnt_on_spine();			
		for (  j = 0 ; j < num_base_pnts ; j++ )		// Defined Fuse XSecs
		{
			p = xsecVec[i]->get_pnt(j) + spinePnt;
			stringerVec[j][xsecCnt] = p;
		}
		xsecCnt++;
	}

	//==== Load Control Points (Handles) Vecs ====//
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

	//==== Create Stringers ====//
	static vector< Bezier_curve > sVec;
	sVec.resize( num_base_pnts );

	//==== Build Stringer Curves ====//
	for ( s = 0 ; s < (int)sVec.size() ; s++ )
	{
		//==== Set Up Tangent Str Vec ====//
		static vector< double > tanAngVec;
		static vector< double > tanStr1Vec;
		static vector< double > tanStr2Vec;
		tanAngVec.resize( num_base_xsecs );
		tanStr1Vec.resize( num_base_xsecs );
		tanStr2Vec.resize( num_base_xsecs );

		for ( i = 0 ; i < (int)xsecVec.size() ; i++ )
		{
			xsecVec[i]->setShapeSymmetry();
			tanAngVec[i]  = DEG_2_RAD*xsecVec[i]->get_tan_ang( s );
			tanStr1Vec[i] = xsecVec[i]->get_tan_str1( s );
			tanStr2Vec[i] = xsecVec[i]->get_tan_str2( s );
		}
		//==== Compute Rotation Axis ====//
		double fract = (double)s/(double)(sVec.size()-1);
		double ang = fract*PI*2.0;
		vec3d axis( 0.0, 1.0, 0.0 );
		axis.rotate_x( cos(ang), sin(ang) );

		sVec[s].buildCurve( stringerVec[s], axis, tanAngVec, tanStr1Vec, tanStr2Vec );
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
			uVal += 1.0/(double)(xsecVec[ix]->getNumSectInterp2()+1);
			uVec.push_back(uVal);
		}
	}
//jrg Extra Section???
	//uVal = (int)uVal+1.0;				// Last Section
	//uVec.push_back(uVal);

	surf.set_num_pnts( num_base_pnts );
	surf.set_num_xsecs( uVec.size() );

	for ( s = 0 ; s < (int)sVec.size() ; s++ )
	{
		for ( int ix = 0 ; ix < surf.get_num_xsecs() ; ix++ )
		{
			double u  = uVec[ix];
			int sect  = (int)u;
			vec3d p = sVec[s].comp_pnt(sect, u-(double)sect);
			surf.set_pnt( ix, s, p );
		}
	}

  surf.load_refl_pnts_xsecs();
  surf.load_uw();
  load_hidden_surf();
  load_normals();
  update_bbox(); 

}
//==== Compute And Load Normals ====//
void FuselageGeom::load_normals()
{
  surf.load_normals();
}

//==== Draw Hidden Surface====//
void FuselageGeom::load_hidden_surf()
{
  surf.load_hidden_surf();	
}

//==== Get Cross_Section Number =====//
FuselageXSec* FuselageGeom::get_xsec(int number)
{
  if ( number >= 0 && number < (int)xsecVec.size() )
	  return xsecVec[number];
  return xsecVec[0];
}

//==== Add Cross Section =====//
FuselageXSec* FuselageGeom::add_xsec(FuselageXSec* curr_xsec)
{
	int i;

	//==== Dont add past last xsec ====//
	if ( curr_xsec_num >= (int)xsecVec.size()-1 )
		return xsecVec[curr_xsec_num];

    double loc1 = xsecVec[curr_xsec_num]->get_loc_on_spine();
    double loc2 = xsecVec[curr_xsec_num+1]->get_loc_on_spine();

	FuselageXSec* add_xsec = new FuselageXSec(this);
	add_xsec->gen_parms();
	*add_xsec = *(xsecVec[curr_xsec_num]);
	add_xsec->generate();
	add_xsec->set_fuse_ptr( this );

	add_xsec->set_loc_on_spine( 0.5f*(loc1+loc2) );

	vector< FuselageXSec* > tmpVec;

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
FuselageXSec* FuselageGeom::delete_xsec(FuselageXSec* curr_xsec)
{
	int i;
	//==== Keep At Least 3 Cross Sections ====//
	if ( xsecVec.size() < 4 )
		return (curr_xsec);

	//==== Dont Delete First or Last Cross Section ====//
	if ( curr_xsec_num == 0 || curr_xsec_num == xsecVec.size()-1 )
		return (curr_xsec);

	vector< FuselageXSec* > tmpVec;

	for (  i = 0 ; i < curr_xsec_num ; i++ )
		tmpVec.push_back( xsecVec[i] );

	for (  i = curr_xsec_num+1 ; i < (int)xsecVec.size() ; i++ )
		tmpVec.push_back( xsecVec[i] );

	xsecVec = tmpVec;

	//==== Save Cross Section Parameters ====//
	saved_xsec.copy( *curr_xsec );
	
	RemoveXSecParmReferences( curr_xsec );
	delete curr_xsec;

	curr_xsec = xsecVec[curr_xsec_num];

	updateNumInter();

	this->generate();

	parmMgrPtr->RebuildAll();

	return curr_xsec;
}

//==== Copy Cross Section =====//
void FuselageGeom::copy_xsec(FuselageXSec* curr_xsec)
{
  saved_xsec.copy( *curr_xsec );
}

//==== Paste Cross Section =====//
void FuselageGeom::paste_xsec(FuselageXSec* curr_xsec)
{
  double curr_loc = curr_xsec->get_loc_on_spine();

  //==== Save shape parms ====//
  double topTanStr1 = curr_xsec->getTopTanStr1()->get();
  double topTanStr2 = curr_xsec->getTopTanStr2()->get();
  double topTanAng  = curr_xsec->getTopTanAng()->get();
  double botTanStr1 = curr_xsec->getBotTanStr1()->get();
  double botTanStr2 = curr_xsec->getBotTanStr2()->get();
  double botTanAng  = curr_xsec->getBotTanAng()->get();
  double leftTanStr1 = curr_xsec->getLeftTanStr1()->get();
  double leftTanStr2 = curr_xsec->getLeftTanStr2()->get();
  double leftTanAng  = curr_xsec->getLeftTanAng()->get();
  double rightTanStr1 = curr_xsec->getRightTanStr1()->get();
  double rightTanStr2 = curr_xsec->getRightTanStr2()->get();
  double rightTanAng  = curr_xsec->getRightTanAng()->get();
  int numSectInterp1 = curr_xsec->getNumSectInterp1();
  int numSectInterp2 = curr_xsec->getNumSectInterp2();

  curr_xsec->copy( saved_xsec );

  //==== Restore shape parms ====//
  curr_xsec->getTopTanStr1()->set( topTanStr1 );
  curr_xsec->getTopTanStr2()->set( topTanStr2 );
  curr_xsec->getTopTanAng()->set( topTanAng );
  curr_xsec->getBotTanStr1()->set( botTanStr1 );
  curr_xsec->getBotTanStr2()->set( botTanStr2 );
  curr_xsec->getBotTanAng()->set( botTanAng );
  curr_xsec->getLeftTanStr1()->set( leftTanStr1 );
  curr_xsec->getLeftTanStr2()->set( leftTanStr2 );
  curr_xsec->getLeftTanAng()->set( leftTanAng );
  curr_xsec->getRightTanStr1()->set( rightTanStr1 );
  curr_xsec->getRightTanStr2()->set( rightTanStr2 );
  curr_xsec->getRightTanAng()->set( rightTanAng );
  curr_xsec->setNumSectInterp1( numSectInterp1 );
  curr_xsec->setNumSectInterp2( numSectInterp2 );

  curr_xsec->generate();

  curr_xsec->set_loc_on_spine( curr_loc );

  this->generate();
}

void FuselageGeom::set_pnt_space_type( int type )
{
  int i;
  for ( i = 0 ; i < (int)xsecVec.size() ; i++ )
  {
	xsecVec[i]->set_pnt_space_type( type );
	xsecVec[i]->generate();
  }
  this->generate();
}

void FuselageGeom::set_num_pnts( int num_in )
{
  int i;
  for ( i = 0 ; i < (int)xsecVec.size() ; i++ )
  {
	xsecVec[i]->set_num_pnts(num_in);
	xsecVec[i]->generate();
  }
  this->generate();
}

void FuselageGeom::draw()
{
	surf.fast_draw_off();

	//==== Draw Highlighting Boxes ====//
	draw_highlight_boxes();

	//==== Set Up Xsec Hightlighting ====//
	if ( redFlag )
	{
		surf.set_highlight_xsec_flag(1);
		int id = 0;
		for ( int i = 0 ; i < curr_xsec_num ; i++ )
		{
			id += 1 + xsecVec[i]->getNumSectInterp2();
		}
		surf.set_highlight_xsec_id( id );		
	}
	else
	{
		surf.set_highlight_xsec_flag(0);
	}

	//==== Check Noshow Flag ====//
	if ( noshowFlag ) return;	

	if ( displayFlag == GEOM_WIRE_FLAG )
	{
		glColor3ub( (int)color.x(), (int)color.y(), (int)color.z() );	

		//==== Draw Geom ====//
		glPushMatrix();
		glMultMatrixf((GLfloat*)model_mat); 

		if ( !redFlag )
			surf.draw_wire();
		else
			draw_wire_fuselage();

		if ( redFlag )
			drawControlPoints();

		glPopMatrix();

		//==== Reflected Geom ====//
		glPushMatrix();
		glMultMatrixf((GLfloat*)reflect_mat); 
		glColor3ub( (int)color.x(), (int)color.y(), (int)color.z() );	
		surf.draw_refl_wire(sym_code);
		glPopMatrix();
	}
	else if ( displayFlag == GEOM_SHADE_FLAG || displayFlag == GEOM_TEXTURE_FLAG )
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
				surf.draw_shaded();

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
		mat = matMgrPtr->getMaterial( materialID );
		if ( mat )
		{
			mat->bind();
			if  ( mat->diff[3] > 0.99 )
			{
				surf.draw_refl_shaded( sym_code);
				
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
		surf.draw_hidden();

		if ( redFlag )
			drawControlPoints();

		glPopMatrix();

		//==== Reflected Geom ====//
		glColor3ub( (int)color.x(), (int)color.y(), (int)color.z() );	
		glPushMatrix();
		glMultMatrixf((GLfloat*)reflect_mat); 
		surf.draw_refl_hidden(sym_code);
		glPopMatrix();
			
	}
}

void FuselageGeom::draw_wire_fuselage()
{
	//==== Draw Cross Sections ====// 
	glPushAttrib(GL_CURRENT_BIT);
	glLineWidth(1.0);
	glColor3ub( (int)color.x(), (int)color.y(), (int)color.z() );	

	int id_cnt = 0;
	int curr_edit_id;
	vector< int > edit_ids;
	for ( int i = 0 ; i < (int)xsecVec.size() ; i++ )
	{
		if ( i == curr_xsec_num )
			curr_edit_id = id_cnt;
		edit_ids.push_back( id_cnt );

		id_cnt += 1 + xsecVec[i]->getNumSectInterp2();
	}

	int num_xsecs = surf.get_num_xsecs();
	for ( int ixs = 0 ; ixs < num_xsecs ; ixs++ )
	{
		if ( ixs == curr_edit_id )
		{
			glColor3ub( 255, 0, 0 );	
			glLineWidth(4.0);
		}
		else if ( ContainsVal<int> ( edit_ids, ixs ) )
		{
			glColor3ub( (int)color.x(), (int)color.y(), (int)color.z() );
			glLineWidth(2.0);
		}
		else
		{
			glColor3ub( 100, 100, 100 );
			glLineWidth(1.0);
		}

		vector< vec3d > pnt_vec;
		surf.get_xsec( ixs, pnt_vec );
		glBegin( GL_LINE_STRIP );
		for ( int j = 0 ; j < (int)pnt_vec.size() ; j++ )
		{
			glVertex3dv( pnt_vec[j].data() );
		}
		glEnd();
	}

	glLineWidth(1.0);
	glColor3ub( 100, 100, 100 );
	int num_pnts = surf.get_num_pnts();
	for ( int ipnt = 0 ; ipnt < num_pnts ; ipnt++ )
	{
		vector< vec3d > pnt_vec;
		surf.get_stringer( ipnt, pnt_vec );
		glBegin( GL_LINE_STRIP );
		for ( int j = 0 ; j < (int)pnt_vec.size() ; j++ )
		{
			glVertex3dv( pnt_vec[j].data() );
		}
		glEnd();
	}
	glPopAttrib();
}


//==== Draw If Alpha < 1 and Shaded ====//
void FuselageGeom::drawAlpha()
{
	if ( displayFlag != GEOM_SHADE_FLAG &&  displayFlag == !GEOM_TEXTURE_FLAG)
		return;

	//==== Check Noshow Flag ====//
	if ( noshowFlag ) return;	

	Material* mat = matMgrPtr->getMaterial( materialID );
	if ( mat && mat->diff[3] <= 0.99 )
	{
		//==== Draw Geom ====//
		glPushMatrix();
		glMultMatrixf((GLfloat*)model_mat); 

		mat->bind();
		surf.draw_shaded();
		
		if ( displayFlag == GEOM_TEXTURE_FLAG )
			drawTextures(false);

		glPopMatrix();

		//==== Reflected Geom ====//
		glPushMatrix();
		glMultMatrixf((GLfloat*)reflect_mat); 

		surf.draw_refl_shaded( sym_code);

		if ( displayFlag == GEOM_TEXTURE_FLAG )
			drawTextures(true);

		glPopMatrix();
	}
}


float FuselageGeom::get_area()
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


float FuselageGeom::get_span()
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
float FuselageGeom::get_cbar()
{
  return( (float)length() );
}


//==== If IML, Then Write Out Inner Surface ====//
void FuselageGeom::dump_xsec_file(int geom_no, FILE* dump_file)
{
	int i;
	vec3d p;

	//==== Only Write Out OML not IML ====//
	fprintf(dump_file, "\n");
	fprintf(dump_file, "%s \n",(char*) getName());
	fprintf(dump_file, " GROUP NUMBER      = %d \n",geom_no);
	fprintf(dump_file, " TYPE              = 1\n");
	fprintf(dump_file, " CROSS SECTIONS    = %d \n",surf.get_num_xsecs() );
	fprintf(dump_file, " PTS/CROSS SECTION = %d \n",surf.get_num_pnts() );

	// Write out cross sections 
	for ( i = 0 ; i < surf.get_num_xsecs() ; i++ ) 
	{
		surf.write_xsec(i, model_mat, dump_file);      
	}

	if ( sym_code == NO_SYM ) return;

	fprintf(dump_file, "\n");
	fprintf(dump_file, "%s \n", (char*) getName());
	fprintf(dump_file, " GROUP NUMBER      = %d \n",geom_no);
	fprintf(dump_file, " TYPE              = 1  \n");						// Non Lifting
	fprintf(dump_file, " CROSS SECTIONS    = %d \n",surf.get_num_xsecs());
	fprintf(dump_file, " PTS/CROSS SECTION = %d \n",surf.get_num_pnts());

	for ( i = 0 ; i < surf.get_num_xsecs() ; i++ )
    {
      surf.write_refl_xsec(sym_code, i, reflect_mat, dump_file);
    }
}

//==== Return Number Of Felisa Surfaces Written Out ====//
int FuselageGeom::get_num_felisa_comps()
{
  int i, j;

  //==== Load Oml into pnt_xsec array ====//
  int num_xsecs = surf.get_num_xsecs();
  int num_pnts  = surf.get_num_pnts();
  pnt_xsec.init( num_xsecs, num_pnts);
  for ( i = 0 ; i < num_xsecs ; i++ )
    for ( j = 0 ; j < num_pnts ; j++ )
      pnt_xsec( i, j ) = surf.get_pnt( i, j );
 
  //==== Number of Comps Outer ====//
  int num_felisa = get_num_felisa_comps_single_surf();

  return num_felisa;
}

//==== Return Number Of Felisa Surfaces Written Out ====//
int FuselageGeom::get_num_felisa_comps_single_surf()
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
int FuselageGeom::write_felisa_file(int geom_no, FILE* dump_file)
{
  int i, j;

  //==== Load Oml into pnt_xsec array ====//
  int num_xsecs = surf.get_num_xsecs();
  int num_pnts  = surf.get_num_pnts();
  pnt_xsec.init( num_xsecs, num_pnts);
  for ( i = 0 ; i < num_xsecs ; i++ )
    for ( j = 0 ; j < num_pnts ; j++ )
      pnt_xsec( i, j ) = surf.get_pnt( i, j );

  //==== Write Outer ====//
  int geom_id = write_felisa_file_single_surf(geom_no, dump_file);

  return geom_id;
}

//==== Write Out Felisa Surfaces To File ====//
int FuselageGeom::write_felisa_file_single_surf(int geom_no, FILE* dump_file)
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
void FuselageGeom::write_bco_info(FILE* file_ptr, int& surf_cnt, int& dum)
{
  int i, j;

  //==== Load Oml into pnt_xsec array ====//
  int num_xsecs = surf.get_num_xsecs();
  int num_pnts  = surf.get_num_pnts();
  pnt_xsec.init( num_xsecs, num_pnts);
  for ( i = 0 ; i < num_xsecs ; i++ )
    for ( j = 0 ; j < num_pnts ; j++ )
      pnt_xsec( i, j ) = surf.get_pnt( i, j );


  //==== Write Outer ====//
  write_bco_info_single_surf(file_ptr, surf_cnt, dum);
}

//==== Write Out Felisa Surfaces To File ====//
void FuselageGeom::write_bco_info_single_surf(FILE* file_ptr, int& surf_cnt, int&)
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
int FuselageGeom::get_num_felisa_line_sources()
{
  //==== Load Oml into pnt_xsec array ====//
  int num_xsecs = surf.get_num_xsecs();
  int num_pnts  = surf.get_num_pnts();
  pnt_xsec.init( num_xsecs, num_pnts);
  for ( int i = 0 ; i < num_xsecs ; i++ )
    for ( int j = 0 ; j < num_pnts ; j++ )
      pnt_xsec( i, j ) = surf.get_pnt( i, j );

  return( 2*( pnt_xsec.dimen_1()-1 ) );
}

//==== Write Out Felisa Background Lines File ====//
void FuselageGeom::write_felisa_line_sources(FILE* dump_file)
{
  int i, j;

  //==== Load Oml into pnt_xsec array ====//
  int num_xsecs = surf.get_num_xsecs();
  int num_pnts  = surf.get_num_pnts();
  pnt_xsec.init( num_xsecs, num_pnts);
  for ( i = 0 ; i < num_xsecs ; i++ )
    for ( j = 0 ; j < num_pnts ; j++ )
      pnt_xsec( i, j ) = surf.get_pnt( i, j );

  //==== Write Outer ====//
  write_felisa_line_sources_single_surf(dump_file);
}

//==== Write Out Felisa Background Lines File ====//
void FuselageGeom::write_felisa_line_sources_single_surf(FILE* dump_file)
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
void FuselageGeom::write_rhino_file(ON_BinaryFile* archive, ON_3dmObjectAttributes* attributes)
{
	surf.clear_xsec_tan_flags();

	surf.set_xsec_tan_flag( 0, Bezier_curve::ZERO );
    int num_xsecs  = surf.get_num_xsecs();
	surf.set_xsec_tan_flag( num_xsecs-1, Bezier_curve::ZERO );


  surf.write_rhino_file( sym_code, model_mat, reflect_mat, archive, attributes );
}

int  FuselageGeom::get_num_bezier_comps()
{
	if ( sym_code == NO_SYM )
		return 1;
	else 
		return 2;
}

void FuselageGeom::write_bezier_file( int id, FILE* file_id )
{
	surf.clear_xsec_tan_flags();

	surf.set_xsec_tan_flag( 0, Bezier_curve::PREDICT );
    int num_xsecs  = surf.get_num_xsecs();
	surf.set_xsec_tan_flag( num_xsecs-1, Bezier_curve::PREDICT );

	//==== Look For Sharp Tangents ====//
	vector <int> uSharpVec;
	for ( int i = 1 ; i < surf.get_num_xsecs()-1 ; i++ )
	{
		vec3d p0 = surf.get_pnt( i-1, 0 );
		vec3d p1 = surf.get_pnt( i,   0 );
		vec3d p2 = surf.get_pnt( i+1, 0 );

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

	u_split.push_back( 3*(surf.get_num_xsecs() - 1)  );
	vector <int> w_split;
	w_split.push_back( 0 );
	w_split.push_back( 3*(surf.get_num_pnts()/4)  );
	w_split.push_back( 3*(surf.get_num_pnts()/2)  );
	w_split.push_back( 3*3*(surf.get_num_pnts()/4)  );
	w_split.push_back( 3*(surf.get_num_pnts() - 1)  );

	surf.write_bezier_file( file_id, sym_code, model_mat, reflect_mat, u_split, w_split  );
}


//==== Convert To Tri Mesh ====//
vector< TMesh* > FuselageGeom:: createTMeshVec()
{
	vector< TMesh* > tMeshVec;

	TMesh* tmPtr = surf.createTMesh(model_mat);
	tMeshVec.push_back( tmPtr );

	if ( sym_code != NO_SYM )
	{
		tmPtr = surf.createReflTMesh(sym_code, reflect_mat);
		tMeshVec.push_back( tmPtr );
	}
	return tMeshVec;
}

void FuselageGeom::debug_print()
{
}

vec3d FuselageGeom::getVertex3d(int id, double x, double p, int r)			
{ 
	return surf.get_vertex(x, p, r);
}

void FuselageGeom::getVertexVec(vector< VertexID > *vertVec)
{ 
	buildVertexVec(&surf, 0, vertVec); 
}

void FuselageGeom::updateNumInter()
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

void FuselageGeom::incNumInterpAll( int off )
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


void FuselageGeom::drawControlPoints()
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
			glColor3ub( 255, 0, 255 );
			glPointSize( 8.0 );
		}
		glBegin( GL_POINTS );
		cPntVec[i].pnt2d = projectPoint( cPntVec[i].pnt3d, 0 );
		glVertex3dv( cPntVec[i].pnt3d.data() );
		glEnd();
	}

}

void FuselageGeom::draw2D(vec2d & cursor)
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
double FuselageGeom::getRefArea()
{
	if ( autoRefAreaFlag )
	{
		refArea = get_area();
	}
	return refArea;
}
	
double FuselageGeom::getRefSpan()
{
	if ( autoRefSpanFlag )
	{
		refSpan = get_span();
	}
	return refSpan;
}

double FuselageGeom::getRefCbar()
{
	if ( autoRefCbarFlag )
	{
		refCbar = length();
	}
	return refCbar;
}

vec3d FuselageGeom::getAeroCenter()
{
	if ( autoAeroCenterFlag )
	{
		aeroCenter.set_x( length()*0.25 + get_tran_x() );
		aeroCenter.set_y( get_tran_y() );
		aeroCenter.set_z( get_tran_z() );
	}
	return aeroCenter;
}

void FuselageGeom::LoadDragFactors( DragFactors& drag_factors )
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

void FuselageGeom::GetInteriorPnts( vector< vec3d > & pVec )
{
	int np = surf.get_num_pnts();
	int nxs = surf.get_num_xsecs();
	vec3d p0 = surf.get_pnt( nxs/2, np/4 );
	vec3d p1 = surf.get_pnt( nxs/2, 3*np/4 );
	vec3d p  = (p0 + p1)*0.5;
	vec3d tp = p.transform( model_mat );
	pVec.push_back( tp ); 

	if (sym_code != NO_SYM)
	{
		tp = (p * sym_vec).transform(reflect_mat);
		pVec.push_back( tp );
    }
}


